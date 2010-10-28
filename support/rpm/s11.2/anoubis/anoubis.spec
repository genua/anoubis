### anoubis RPM spec file ##################################
##
## http://en.opensuse.org/Packaging/RPM_Macros
## http://www.rpm.org/wiki/Problems/Distributions
##

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
Source2:	anoubisd.conf
Vendor:		GeNUA mbH
BuildRoot:	%(mktemp -d %{_tmppath}/%{name}-%{version}-build.XXXX)

### dependencies ###########################################
BuildRequires:	autoconf
BuildRequires:	automake
BuildRequires:	bison
BuildRequires:	check-devel >= 0.9.4
BuildRequires:	flex
BuildRequires:	gcc
BuildRequires:	gcc-c++
BuildRequires:	gettext
BuildRequires:	libattr-devel
BuildRequires:	libevent-devel
BuildRequires:	libnotify-devel
BuildRequires:	libopenssl-devel
BuildRequires:	libstdc++-devel
BuildRequires:	make
BuildRequires:	wxGTK-devel >= 2.8

%define daemon	%{name}d
%define gui	x%{name}
%define user	_%{name}d
%define sbindir	/sbin
%define policydir	/var/lib/%{name}
%define rundir	/var/run/%{daemon}
%define scanuser	_%{name}scan
%define scanhome	/var/spool/%{name}

# long descpritive part of the packaged software goes here
%description
The Anoubis Security Suite.

### subpackage anoubisd ####################################
%package -n %{daemon}
Summary:	Daemon of the Anoubis Security Suite
Group:		System Environment/Daemons
Requires:	openssl
Requires:	cvs

%description -n %{daemon}
Central daemon of the Anoubis Security Suite.

### subpackage xanoubis ####################################
%package -n %{gui}
Summary:	GUI of the Anoubis Security Suite
Group:		User Interface/X
Requires:	%{daemon} >= 0.9.3.26.216
Requires:	wxGTK >= 2.8
Requires:	xorg-x11-fonts
Requires:	openssl
Requires:	cvs

%description -n %{gui}
Graphical user interface of the Anoubis Security Suite.


### build stages ###########################################
%prep
%setup -q

%build
# Update config.{guess,sub}
%{?suse_update_config}
export CFLAGS="$RPM_OPT_FLAGS"
./configure --prefix=%{_prefix} \
 --enable-static-%{daemon} \
 --infodir=%{_infodir} \
 --mandir=%{_mandir} \
 --sbindir=%{sbindir} \
 --libdir=%{_libdir} \
 --sysconfdir=%{_sysconfdir} \
 --disable-tests

%install
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] \
  && rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install-strip
install -D -m 755 %{SOURCE1} %{buildroot}%{_initddir}/%{daemon}
install -D -m 644 %{SOURCE2} %{buildroot}%{_sysconfdir}/%{name}/%{daemon}.conf

# install policy profiles
DEF_POLICY_DIR=$RPM_BUILD_ROOT/%{_datadir}/%{daemon}/policy_templates
mkdir -p $DEF_POLICY_DIR/{admin,user,profiles}
for a in admin user profiles ; do
    for f in $RPM_SOURCE_DIR/policy.$a.* ; do
	tgt=$DEF_POLICY_DIR/$a/${f##*/policy.$a.}
	cp $f $tgt
	perl -p -i -e s,/usr/lib/kde3,/opt/kde3/lib/kde3,g $tgt
    done ;
done

# install wizard templates of xanoubis
GUI_POLICY_DIR=$RPM_BUILD_ROOT/%{_datadir}/%{gui}/policy_templates
GUI_PROFILES_DIR=$RPM_BUILD_ROOT/%{_datadir}/%{gui}/profiles
mkdir -p $GUI_POLICY_DIR/wizard
install -m 644 $GUI_PROFILES_DIR/wizard/{alf,sandbox} $GUI_POLICY_DIR/wizard
rm -rf $GUI_PROFILES_DIR
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/%{name}/profiles/wizard

# install symlink in /etc/anoubis
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/%{name}
mkdir -p $RPM_BUILD_ROOT/%{policydir}/policy
mkdir -p $RPM_BUILD_ROOT/%{scanhome}
ln -s %{policydir}/policy $RPM_BUILD_ROOT/%{_sysconfdir}/%{name}/policy

# install udev rules of anoubis devices
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/udev/rules.d
install -m 0644 support/udev.rules \
	$RPM_BUILD_ROOT%{_sysconfdir}/udev/rules.d/06-%{name}.rules

# remove files that should not be part of the rpm
rm -fr $RPM_BUILD_ROOT%{_includedir}
rm -f  $RPM_BUILD_ROOT%{_libdir}/*.a
rm -fr $RPM_BUILD_ROOT%{_mandir}/man3

# install and register desktop icon and file
mkdir -p $RPM_BUILD_ROOT%{_datadir}/{applications,pixmaps,%{gui}}
install -p $RPM_BUILD_ROOT%{_datadir}/%{gui}/icons/%{gui}.png \
	$RPM_BUILD_ROOT%{_datadir}/pixmaps
install -p $RPM_BUILD_ROOT%{_datadir}/%{gui}/%{gui}.desktop \
	$RPM_BUILD_ROOT%{_datadir}/applications

# locate translations
%find_lang %{name}

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] \
  && rm -rf $RPM_BUILD_ROOT

### package scripts ########################################
%pre -n %{daemon}
# Do not stop anoubisd during an upgrade,
# instead restart it after the upgrade
if ! getent passwd %{user} >/dev/null; then
	groupadd -r %{user} 2>/dev/null || :
	useradd -M -r -s /sbin/nologin -d %{rundir} \
	    -g %{user} %{user}
fi
if ! getent passwd %{scanuser} >/dev/null; then
	groupadd -r %{scanuser} 2>/dev/null || :
	useradd -M -r -s /sbin/nologin -d %{scanhome} \
	    -g %{scanuser} %{scanuser}
fi
if ! getent group _nosfs >/dev/null; then
	groupadd -r _nosfs 2>/dev/null || :
fi
exit 0

%pre -n %{gui}
if ! getent group _nosfs >/dev/null; then
	groupadd -f -r _nosfs
fi
exit 0

%posttrans -n %{gui}
# remove profile directory from old rpms, and add symlink manually.
# rpm will not replace a directory with a symlink on upgrades
PROFDIR=%{_datadir}/%{gui}/profiles
if [ ! -L $PROFDIR ] ; then
	if [ -e $PROFDIR ] ; then
		mv $PROFDIR $PROFDIR.rpm-bak.$$
	fi
	ln -s %{_sysconfdir}/%{name}/profiles $PROFDIR
fi
exit 0

%post -n %{gui}
# update xanoubis wizard profiles
# we just overwrite the old files until we have a better mechanism
# for updates
rm -f %{_sysconfdir}/%{name}/profiles/wizard/alf
rm -f %{_sysconfdir}/%{name}/profiles/wizard/sandbox
cp %{_datadir}/%{gui}/policy_templates/wizard/* \
	%{_sysconfdir}/%{name}/profiles/wizard
chmod 644 %{_sysconfdir}/%{name}/profiles/wizard/*

if getent group _nosfs >/dev/null; then
	chown root:_nosfs %{_bindir}/%{gui} && \
	chmod 2755 %{_bindir}/%{gui}
fi

%post -n %{daemon}
chkconfig --add %{daemon}
chkconfig %{daemon} on
mkdir -p %{policydir}/policy/{admin,user,pubkeys}
chmod -R 700 %{policydir}/policy

if getent group _nosfs >/dev/null; then
	chown root:_nosfs %{sbindir}/sfssig %{sbindir}/anoubisctl && \
	chmod 2755 %{sbindir}/sfssig %{sbindir}/anoubisctl
fi

# update run dir permissions from old versions
if [ -d %{rundir} ] ; then
	chown root:%{user} %{rundir}
	chmod 0770 %{rundir}
fi

# copy new default policy
export PATH=$PATH:/opt/kde3/bin
%{_datadir}/%{daemon}/install_policy -q\
	%{_datadir}/%{daemon}/policy_templates/admin \
	%{policydir}/policy/admin

%{_datadir}/%{daemon}/install_policy -q\
	%{_datadir}/%{daemon}/policy_templates/user \
	%{policydir}/policy/user

# update xanoubis profiles
# we just overwrite the old files until we have a better mechanism
# for updates
rm -f %{_sysconfdir}/%{name}/profiles/admin
rm -f %{_sysconfdir}/%{name}/profiles/medium
rm -f %{_sysconfdir}/%{name}/profiles/high
%{_datadir}/%{daemon}/install_policy -q -n -o \
	%{_datadir}/%{daemon}/policy_templates/profiles \
	%{_sysconfdir}/%{name}/profiles

chown -R %{user}: %{policydir}/policy

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
    %{_initddir}/%{daemon} start
fi
exit 0

%preun -n %{daemon}
# execute only on package removal,
# i.e. if last version is removed (0 versions left)
if [ "$1" = 0 ] ; then
    %{_initddir}/%{daemon} stop
    chkconfig --del %{daemon}
    rmdir %{policydir}/policy/{admin,user,pubkeys} 2>/dev/null || :
    rmdir %{policydir}/policy %{policydir} 2>/dev/null || :
fi
exit 0

%postun -n %{daemon}
# execute only on upgrades (i.e. at least 1 version left)
if [ "$1" -ge 1 ]; then
    %{_initddir}/%{daemon} restart
fi
exit 0


### files of subpackage anoubisd ###########################
%files -n %{daemon} -f %{name}.lang
%defattr(-,root,root)
%{_initddir}/%{daemon}
%config(noreplace) %{_sysconfdir}/%{name}
%config(noreplace) %{_sysconfdir}/udev/rules.d/06-%{name}.rules
%{sbindir}/%{daemon}
%{sbindir}/anoubisctl
%{sbindir}/sfssig
%{sbindir}/anoubis-keyinstall
%{_bindir}/anoubis-keygen
%{_bindir}/playground
%{_datadir}/%{daemon}/*
%attr(0750,root,%{user}) %dir %{policydir}
%attr(0700,%{scanuser},%{scanuser}) %dir %{scanhome}
%{policydir}/*
%{_mandir}/man1/anoubis-keygen.1.gz
%{_mandir}/man1/anoubis-keyinstall.1.gz
%{_mandir}/man1/playground.1.gz
%{_mandir}/man4/*
%{_mandir}/man5/*
%{_mandir}/man8/*
%{_mandir}/man9/*

### files of subpackage xanoubis ###########################
%files -n %{gui}
%defattr(-,root,root)
%doc ChangeLog
%{_bindir}/%{gui}
%{_mandir}/man1/%{gui}*.1.gz
%{_datadir}/%{gui}
%{_datadir}/%{gui}/icons
%{_datadir}/%{gui}/policy_templates
%{_datadir}/%{gui}/%{gui}.desktop
%attr(0755,root,root) %{_datadir}/%{gui}/utils
%attr(0644,root,root) %{_datadir}/applications/%{gui}.desktop
%attr(0644,root,root) %{_datadir}/pixmaps/%{gui}.png


### changelog ##############################################
%changelog
* Tue Jul 27 2010 Sten Spans
- fixed most rpmlint errors
- fixed groupadd errors
- introduced more variables

* Mon Mar 01 2010 Sten Spans
- add new gettext translations
- complete spring cleaning

* Wed Jan 27 2010 Sebastian Trahm
- add default policy file for user

* Tue Dec 01 2009 Stefan Fritsch
- adjust init script LSB dependencies

* Fri Nov 27 2009 Christian Ehrhardt
- add anoubis-keyinstall

* Thu Nov 05 2009 Sten Spans
- add setgid group and permissions to the anoubis client utilities

* Wed Oct 14 2009 Sebastian Trahm
- xanoubis package has to depend on a minimal version of anoubisd

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
