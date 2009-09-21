### anoubis RPM spec file ##################################

### package definition #####################################
Summary:	The Anoubis Security Suite
Name:		anoubis
Version:	@@VERSION@@
Release:	2
License:	BSD
Group:		System Environment/Base
URL:		http://www.genua.de
Source0:	%{name}-%{version}.tar.gz
Source1:	anoubisd.init
Source10:	policy.admin.0
Source11:	policy.user.0
Source12:	policy.admin.default
Source20:	policy.profiles.admin
Source21:	policy.profiles.medium
Source22:	policy.profiles.high
Vendor:		GeNUA mbH
BuildRoot:	%(mktemp -d %{_tmppath}/%{name}-%{version}-build.XXXX)

### dependencies ###########################################
BuildRequires:	autoconf
BuildRequires:	gcc
BuildRequires:	gcc-c++
BuildRequires:	make
BuildRequires:	check-devel >= 0.9.4
BuildRequires:	openssl-devel
BuildRequires:	libnotify-devel
BuildRequires:	libevent
BuildRequires:	wxGTK-devel >= 2.8
BuildRequires:	libattr-devel
BuildRequires:	libstdc++-devel
BuildRequires:	flex
BuildRequires:	bison

%define rcdir %{_sysconfdir}/init.d

# long descpritive part of the packaged software goes here
%description
The Anoubis Security Suite.

### subpackage anoubisd ####################################
%package -n anoubisd
Summary:	daemon of the Anoubis Security Suite
Group:		System Environment/Daemons
Requires:	libevent
Requires:	openssl
Requires:	cvs

%description -n anoubisd
Central daemon of the Anoubis Security Suite.

### subpackage xanoubis ####################################
%package -n xanoubis
Summary:	GUI of the Anoubis Security Suite
Group:		User Interface/X
Requires:	anoubisd
Requires:	wxGTK >= 2.8
Requires:	libevent
Requires:	libnotify
Requires:	xorg-x11-fonts
Requires:	cvs

%description -n xanoubis
GUI of the Anoubis Security Suite.


### build stages ###########################################
%prep
%setup -q

%build
# Update config.{guess,sub}
%{?suse_update_config}
export CFLAGS="$RPM_OPT_FLAGS"
./configure --prefix=%{_prefix} \
 --enable-static-anoubisd \
 --infodir=%{_infodir} \
 --mandir=%{_mandir} \
 --sbindir=/sbin \
 --sysconfdir=/etc \
 --disable-tests

%install
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] \
  && rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{rcdir}
cp $RPM_SOURCE_DIR/anoubisd.init $RPM_BUILD_ROOT%{rcdir}/anoubisd
chmod 755 $RPM_BUILD_ROOT%{rcdir}/anoubisd

# install policy profiles
DEF_POLICY_DIR=$RPM_BUILD_ROOT/usr/share/anoubisd/policy_templates
mkdir -p $DEF_POLICY_DIR/admin $DEF_POLICY_DIR/user $DEF_POLICY_DIR/profiles
for a in admin user profiles ; do
    for f in $RPM_SOURCE_DIR//policy.$a.* ; do
	tgt=$DEF_POLICY_DIR/$a/${f##*/policy.$a.}
	cp $f $tgt
	perl -p -i -e s,/usr/lib/kde3,/opt/kde3/lib/kde3,g $tgt
    done ;
done

# install wizard templates of xanoubis
DEF_WIZARD_DIR=$RPM_BUILD_ROOT/usr/share/xanoubis/policy_templates/wizard
mkdir -p $DEF_WIZARD_DIR
install -p $RPM_BUILD_ROOT/%{_datadir}/xanoubis/profiles/wizard/{alf,sandbox} \
	$DEF_WIZARD_DIR

rm -rf $RPM_BUILD_ROOT/usr/share/xanoubis/profiles
mkdir -p $RPM_BUILD_ROOT/etc/anoubis/profiles/wizard
# Must put xanoubis's checksum into the anoubisd package because
# xanoubis is not yet installed when anoubisd's postinst runs.
SUM_XANOUBIS=`sha256sum $RPM_BUILD_ROOT/usr/bin/xanoubis|cut -d' '  -f 1`
for f in `grep -rl xanoubis $DEF_POLICY_DIR` ; do
    perl -p -i -e 's{/usr/bin/xanoubis \@\@}{/usr/bin/xanoubis '$SUM_XANOUBIS'}g' $f
done

# install symlink in /etc/anoubis
mkdir -p $RPM_BUILD_ROOT/etc/anoubis
mkdir -p $RPM_BUILD_ROOT/var/lib/anoubis/policy
ln -s /var/lib/anoubis/policy $RPM_BUILD_ROOT/etc/anoubis/policy

# install udev rules of anoubis devices
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/udev/rules.d
install -m 0644 support/udev.rules \
	$RPM_BUILD_ROOT%{_sysconfdir}/udev/rules.d/06-anoubis.rules

# remove files that should not be part of the rpm
rm -fr $RPM_BUILD_ROOT%{_includedir}
rm -f  $RPM_BUILD_ROOT%{_libdir}/*.a
rm -fr $RPM_BUILD_ROOT%{_mandir}/man3

# install and register desktop icon and file
mkdir -p $RPM_BUILD_ROOT%{_datadir}/{applications,pixmaps,xanoubis}
install -p $RPM_BUILD_ROOT%{_datadir}/xanoubis/icons/xanoubis.png \
	$RPM_BUILD_ROOT%{_datadir}/pixmaps
install -p $RPM_BUILD_ROOT%{_datadir}/xanoubis/xanoubis.desktop \
	$RPM_BUILD_ROOT%{_datadir}/applications

# add the anoubisd configuration files
cp $RPM_SOURCE_DIR/anoubisd.conf $RPM_BUILD_ROOT/etc/anoubis

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] \
  && rm -rf $RPM_BUILD_ROOT

### package scripts ########################################
%pre -n anoubisd
if [ "$1" -gt 1 ]; then
    # when upgrading, stop old anoubisd
    %{rcdir}/anoubisd stop
fi
if ! getent passwd _anoubisd >/dev/null; then
	groupadd -f -r _anoubisd
	useradd -M -r -s /sbin/nologin -d /var/run/anoubisd \
	    -g _anoubisd _anoubisd
fi
exit 0

%posttrans -n xanoubis
# remove profile directory from old rpms, and add symlink manually.
# rpm will not replace a directory with a symlink on upgrades
PROFDIR=/usr/share/xanoubis/profiles
if [ ! -L $PROFDIR ] ; then
	if [ -e $PROFDIR ] ; then
		mv $PROFDIR $PROFDIR.rpm-bak.$$
	fi
	ln -s /etc/anoubis/profiles $PROFDIR
fi
exit 0

%post -n xanoubis
# update xanoubis wizard profiles
# we just overwrite the old files until we have a better mechanism
# for updates
rm -f /etc/anoubis/profiles/wizard/alf
rm -f /etc/anoubis/profiles/wizard/sandbox
cp /usr/share/xanoubis/policy_templates/wizard/* \
	/etc/anoubis/profiles/wizard
chmod 644 /etc/anoubis/profiles/wizard/*

%post -n anoubisd
chkconfig --add anoubisd
chkconfig anoubisd on
mkdir -p /var/lib/anoubis/policy/admin
mkdir -p /var/lib/anoubis/policy/user
mkdir -p /var/lib/anoubis/policy/pubkeys
chmod 700 /var/lib/anoubis/policy
chmod 700 /var/lib/anoubis/policy/admin /var/lib/anoubis/policy/user
chmod 700 /var/lib/anoubis/policy/pubkeys
# copy new default policy
export PATH=$PATH:/opt/kde3/bin
/usr/share/anoubisd/install_policy -q\
	/usr/share/anoubisd/policy_templates/admin \
	/var/lib/anoubis/policy/admin

/usr/share/anoubisd/install_policy -q\
	/usr/share/anoubisd/policy_templates/user \
	/var/lib/anoubis/policy/user

# update xanoubis profiles
# we just overwrite the old files until we have a better mechanism
# for updates
rm -f /etc/anoubis/profiles/admin
rm -f /etc/anoubis/profiles/medium
rm -f /etc/anoubis/profiles/high
/usr/share/anoubisd/install_policy -q -n -o \
	/usr/share/anoubisd/policy_templates/profiles \
	/etc/anoubis/profiles

chown _anoubisd: /var/lib/anoubis/policy
chown _anoubisd: /var/lib/anoubis/policy/admin /var/lib/anoubis/policy/user
chown _anoubisd: /var/lib/anoubis/policy/pubkeys
if [ ! -e /dev/eventdev ] ; then
	mknod /dev/eventdev c 10 62
fi
# our udev rules file is only effective starting with the next reboot.
# Therefore update permissions manually now.
if [ -e /dev/anoubis ] ; then
       chmod 644 /dev/anoubis
fi
# execute only on new installs (i.e. exactly 1 version installed)
if [ "$1" = 1 ]; then
    %{rcdir}/anoubisd start
fi
exit 0

%preun -n anoubisd
# execute only on package removal,
# i.e. if last version is removed (0 versions left)
if [ "$1" = 0 ] ; then
    %{rcdir}/anoubisd stop
    chkconfig --del anoubisd
    rmdir /var/lib/anoubis/policy/admin \
	/var/lib/anoubis/policy/user 2>/dev/null || true
    rmdir /var/lib/anoubis/policy/pubkeys 2>/dev/null || true
    rmdir /var/lib/anoubis/policy /var/lib/anoubis 2>/dev/null || true
fi
exit 0

%postun -n anoubisd
# execute only on upgrades (i.e. at least 1 version left)
if [ "$1" -ge 1 ]; then
    %{rcdir}/anoubisd start
fi
exit 0


### files of subpackage anoubisd ###########################
%files -n anoubisd
%defattr(-,root,root)
%{rcdir}/*
/etc/anoubis/anoubisd.conf
/sbin/*
%{_prefix}/bin/anoubis-keygen
/usr/share/anoubisd/*
%{_sysconfdir}/udev/rules.d/06-anoubis.rules
%{_sysconfdir}/anoubis
/var/lib/anoubis/*
%{_mandir}/man1/anoubis-keygen.1.gz
%{_mandir}/man4/*
%{_mandir}/man5/*
%{_mandir}/man8/*
%{_mandir}/man9/*

### files of subpackage xanoubis ###########################
%files -n xanoubis
%defattr(-,root,root)
%doc AUTHORS INSTALL NEWS README ChangeLog
%{_prefix}/bin/xanoubis
%{_mandir}/man1/xanoubis*.1.gz
%{_prefix}/share/xanoubis/*
%attr(0644,root,root) %{_datadir}/xanoubis/xanoubis.desktop
%attr(0644,root,root) %{_datadir}/applications/xanoubis.desktop
%attr(0644,root,root) %{_datadir}/pixmaps/xanoubis.png


### changelog ##############################################
%changelog
* Mon Sep 21 2009 Sten Spans
- add anoubis-keygen

* Thu Jul 03 2009 Sebastian Trahm
- add configuration file anoubisd.conf to subpackage anoubisd

* Thu Jul 02 2009 Sebastian Trahm
- add anoubisd.conf(5) to subpackage anoubisd

* Thu Mar 26 2009 Sebastian Trahm
- add install and update of wizard template files

* Tue Nov 25 2008 Stefan Fritsch
- fix install scripts deleting startlinks on upgrade

* Wed Oct 15 2008 Sebastian Trahm
- add installation of desktop icon and file

* Mon Oct 13 2008 Sebastian Trahm
- fix udev permission for /dev/anoubis

* Fri Feb 01 2008 Sebastian Trahm
- first packaging for Fedora Core 8 (werewolf)
