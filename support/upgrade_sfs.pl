#!/usr/bin/perl

##########################################################################
# Copyright (c) 2009 GeNUA mbH <info@genua.de>
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##########################################################################

use strict;
use warnings FATAL => "all";
use Getopt::Std;

use File::Find;

my $errors = 0;

# We collect all files before we start to convert them.
# If we find something we can't handle, we can still abort savely.
my @siglist;
my @cslist;

my $opt = {};
getopts("dh",$opt) || usage();

my $Debug = $opt->{d};
$opt->{h} and usage();

umask 0027;

my $sfsdir = "/var/lib/anoubis/sfs";
my $sfsversionfile = "/var/lib/anoubis/sfs.version";
my $sfsversion = read_sfsversion($sfsversionfile);

if ($sfsversion > 0) {
	# nothing to do
	print STDERR "version $sfsversion is new enough, exiting.\n" if $Debug;
	exit(0)
}
# the version we convert to
$sfsversion = 1;

my (undef, undef, $uid, $gid) = getpwnam('_anoubisd');
if (! defined $gid) {
	die "User _anoubisd not found\n";
}

my @dirs = ($sfsdir);
find(\&wanted, @dirs);

write_sfsversion($sfsversionfile, $sfsversion);
print STDERR "Set sfs.version to $sfsversion.\n" if $Debug;

foreach my $file (@cslist) {
	upgrade($file, 1);
}

print STDERR "Upgraded " . scalar @cslist . " unsigned checksums.\n" if $Debug;

foreach my $file (@siglist) {
	upgrade($file, 2);
}

print STDERR "Upgraded " . scalar @siglist . " signed checksums.\n" if $Debug;

exit(0);

##########################################################################
# Subroutines
##########################################################################

sub read_sfsversion {
	my $name = shift;
	open(my $fh, "<", $name) or return 0;
	my $line = <$fh>;
	close $fh;
	if ($line && $line =~ m/^(\d+)\n$/) {
		return $1;
	}
	die "Could not parse $name\n";
}

sub write_sfsversion {
	my ($name, $version) = @_;
	open(my $fh, ">", $name) or die "Cannot write $name\n";
	print $fh "$version\n";
	close $fh or die "Cannot write $name\n";
}



sub wanted {
	my $name = $_;
	my $fullname = $File::Find::name;

	return unless -f $fullname;

	if ($name =~ /^k.*/) {
		push @siglist, $fullname;
	} elsif ($name =~ /^\d+$/) {
		push @cslist, $fullname;
	} else {
		die "Don't know how to handle $fullname\n";
	}
}

sub upgrade {
	my ($name, $type) = @_;

	# slurp mode
	local $/;

	# disable unicode
	use bytes;

	my $fh;
	my $data;
	open($fh, "<", $name) or die "can't read $name\n";
	$data = <$fh>;
	close($fh);

	if (length($data) < 32) {
		die "$name is too short\n";
	}

	open($fh, ">", "$name.new") or die "can't write $name.new\n";

	# perl pack's 'L' is always 32bit
	print $fh pack('L*',
	    $sfsversion,
	    # type, offset 7*4=28, length 256/8=32,
	    $type, 28, length($data),
	    # type EOT, offset 0, length 0
	    0, 0, 0), $data;

	close($fh) or die "can't write $name.new\n";
	chown(0, $gid, "$name.new") or die "can't chgrp $name.new\n";
	rename("$name.new", $name) or die "can't move $name.new over $name\n";
}

sub usage {
	print STDERR <<"EOF";
usage: $0 [-hd]
	Converts files in $sfsdir from format version 0 to version 1.
EOF
	exit(1);
}
