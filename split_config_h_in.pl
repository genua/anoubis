#!/usr/bin/perl

##########################################################################
# Copyright (c) 2008 GeNUA mbH <info@genua.de>
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
use Fatal qw/rename open close unlink/;

my $confhname = 'src/config.h.in';
my $vershname = 'src/version.h.in';
my $confhbakname = "$confhname.bak";
my $found_version;

rename($confhname, $confhbakname);
unlink($vershname) if -e $vershname;

open(my $in, '<', $confhbakname);
open(my $confout, '>', $confhname);
open(my $versout, '>', $vershname);
print $versout "/* src/version.h.in. */\n\n";

{
	# read paragraphs
	local $/="\n\n";

	my $par;

	while ( defined ($par = <$in>) ) {
		if ($par =~ /\b(?:VERSION|PACKAGE_(?:STRING|VERSION|BUILD))\b/)
		{
			print $versout $par;
			$found_version = 1;
		}
		else {
			print $confout $par;
		}
	}
}
close($confout);
close($versout);
close($in);

if (!$found_version) {
	unlink($vershname);
	rename($confhbakname, $confhname);
	die "FATAL: did not find VERSION in config.h.in. Rerun autoheader?\n";
}
unlink($confhbakname);
