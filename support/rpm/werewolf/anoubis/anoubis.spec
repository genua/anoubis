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
Source2:	anoubisd.udev
Source10:	policy.admin.0
Source11:	policy.user.0
Source12:	policy.admin.default
Vendor:		GeNUA mbH
BuildRoot:	%(mktemp -d %{_tmppath}/%{name}-%{version}-build.XXXX)

### dependencies ###########################################
BuildRequires:	autoconf
BuildRequires:	check-devel >= 0.9.4
BuildRequires:	openssl-devel
BuildRequires:	libnotify-devel
BuildRequires:	libevent-devel
BuildRequires:	wxGTK-devel >= 2.8
BuildRequires:	libattr-devel
BuildRequires:	libstdc++-devel
BuildRequires:	flex
BuildRequires:	bison
BuildRequires:	desktop-file-utils

%define rcdir %{_sysconfdir}/rc.d/init.d

# long descpritive part of the packaged software goes here
%description
The Anoubis Security Suite.

### subpackage anoubisd ####################################
%package -n anoubisd
Summary:	daemon of the Anoubis Security Suite
Group:		System Environment/Daemons
Requires:	libevent
Requires:	openssl

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
 --sbindir=/sbin

%install
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] \
  && rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{rcdir}
cp $RPM_SOURCE_DIR/anoubisd.init $RPM_BUILD_ROOT%{rcdir}/anoubisd
chmod 755 $RPM_BUILD_ROOT%{rcdir}/anoubisd
DEF_POLICY_DIR=$RPM_BUILD_ROOT/usr/share/anoubis/default_policy
mkdir -p $DEF_POLICY_DIR/admin $DEF_POLICY_DIR/user
for a in admin user ; do
    for f in $RPM_SOURCE_DIR//policy.$a.* ; do
	cp $f $DEF_POLICY_DIR/$a/${f##*/policy.$a.}
    done ;
done

# install symlink in /etc/anoubis
mkdir -p $RPM_BUILD_ROOT/etc/anoubis
mkdir -p $RPM_BUILD_ROOT/var/lib/anoubis/policy
ln -s /var/lib/anoubis/policy $RPM_BUILD_ROOT/etc/anoubis/policy

# install udev rules of anoubis devices
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/udev/rules.d
install -m 0644 $RPM_SOURCE_DIR/anoubisd.udev \
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
POLDIR=/etc/anoubis/policy
exit 0

%post -n anoubisd
chkconfig --add anoubisd
chkconfig anoubisd on
mkdir -p /var/lib/anoubis/policy/admin
mkdir -p /var/lib/anoubis/policy/user
mkdir -p /var/lib/anoubis/policy/pubkeys
chmod 700 /var/lib/anoubis/policy
chmod 700 /var/lib/anoubis/policy/admin /var/lib/anoubis/policy/user
chmod 700 /var/lib/anoubis/policy/pubkeys
for p in admin/0 user/0 admin/default; do
    if [ ! -e /var/lib/anoubis/policy/$p ] ; then
	cp /usr/share/anoubis/default_policy/$p /var/lib/anoubis/policy/$p
	chmod 600 /var/lib/anoubis/policy/$p
	chown _anoubisd: /var/lib/anoubis/policy/$p
    fi
done
chown _anoubisd: /var/lib/anoubis/policy
chown _anoubisd: /var/lib/anoubis/policy/admin /var/lib/anoubis/policy/user
chown _anoubisd: /var/lib/anoubis/policy/pubkeys
if [ ! -e /dev/eventdev ] ; then
	mknod /dev/eventdev c 10 62
fi
# execute only on new installs (i.e. exactly least 1 version installed)
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
/sbin/*
/usr/share/anoubis/*
%{_sysconfdir}/udev/rules.d/06-anoubis.rules
%{_sysconfdir}/anoubis/policy
/var/lib/anoubis/*
%{_mandir}/man8/*

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
* Tue Nov 25 2008 Stefan Fritsch
- fix install scripts deleting startlinks on upgrade

* Wed Oct 15 2008 Sebastian Trahm
- add installation of desktop icon and file

* Mon Oct 13 2008 Sebastian Trahm
- fix udev permission for /dev/anoubis

* Fri Feb 01 2008 Sebastian Trahm
- first packaging for Fedora Core 8 (werewolf)
