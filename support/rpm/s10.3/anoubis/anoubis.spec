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
BuildRequires:	libevent
BuildRequires:	wxGTK-devel >= 2.8
BuildRequires:	libattr-devel
BuildRequires:	libstdc++-devel


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
Requires:	xorg-x11-fonts

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
mkdir -p $RPM_BUILD_ROOT/etc/init.d
cp $RPM_SOURCE_DIR/anoubisd.init $RPM_BUILD_ROOT/etc/init.d/anoubisd
chmod 755 $RPM_BUILD_ROOT/etc/init.d/anoubisd
DEF_POLICY_DIR=$RPM_BUILD_ROOT/usr/share/anoubis/default_policy
mkdir -p $DEF_POLICY_DIR/admin $DEF_POLICY_DIR/user
for a in admin user ; do
    for f in $RPM_SOURCE_DIR//policy.$a.* ; do
	cp $f $DEF_POLICY_DIR/$a/${f##*/policy.$a.}
    done ;
done

mkdir -p $RPM_BUILD_ROOT/etc/udev/rules.d
cp $RPM_SOURCE_DIR/anoubisd.udev \
  $RPM_BUILD_ROOT/etc/udev/rules.d/06-anoubis.rules

# remove files that should not be part of the rpm
rm -fr $RPM_BUILD_ROOT%{_includedir}
rm -f  $RPM_BUILD_ROOT%{_libdir}/*.a
rm -fr $RPM_BUILD_ROOT%{_mandir}/man3

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] \
  && rm -rf $RPM_BUILD_ROOT

### package scripts ########################################
%pre -n anoubisd
if ! getent passwd _anoubisd >/dev/null; then
	groupadd -f -r _anoubisd
	useradd -M -r -s /sbin/nologin -d /var/run/anoubisd \
	    -g _anoubisd _anoubisd
fi

%post -n anoubisd
chkconfig --add anoubisd
chkconfig anoubisd on
mkdir -p /etc/anoubis/policy/admin
mkdir -p /etc/anoubis/policy/user
mkdir -p /etc/anoubis/policy/pubkeys
chmod 700 /etc/anoubis/policy
chmod 700 /etc/anoubis/policy/admin /etc/anoubis/policy/user
chmod 700 /etc/anoubis/policy/pubkeys
for p in admin/0 user/0 admin/default; do
    if [ ! -e /etc/anoubis/policy/$p ] ; then
	cp /usr/share/anoubis/default_policy/$p /etc/anoubis/policy/$p
	chmod 600 /etc/anoubis/policy/$p
	chown _anoubisd: /etc/anoubis/policy/$p
    fi
done
chown _anoubisd: /etc/anoubis/policy
chown _anoubisd: /etc/anoubis/policy/admin /etc/anoubis/policy/user
chown _anoubisd: /etc/anoubis/policy/pubkeys
if [ ! -e /dev/eventdev ] ; then
	mknod /dev/eventdev c 10 62
fi


%preun -n anoubisd
chkconfig --del anoubisd
rmdir /etc/anoubis/policy/admin /etc/anoubis/policy/user 2>/dev/null || true
rmdir /etc/anoubis/policy/pubkeys 2>/dev/null || true
rmdir /etc/anoubis/policy /etc/anoubis 2>/dev/null || true

### files of subpackage anoubisd ###########################
%files -n anoubisd
%defattr(-,root,root)
/etc/init.d/*
/etc/udev/*
/sbin/*
/usr/share/anoubis/*
%{_mandir}/man8/*

### files of subpackage xanoubis ###########################
%files -n xanoubis
%defattr(-,root,root)
%doc AUTHORS INSTALL NEWS README ChangeLog
%{_prefix}/bin/xanoubis
%{_mandir}/man1/xanoubis*.1.gz
%{_prefix}/share/xanoubis/*


### changelog ##############################################
%changelog
* Fri Feb 01 2008 Sebastian Trahm
- first packaging for Fedora Core 8 (werewolf)
