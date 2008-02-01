### anoubis RPM spec file ##################################

### package definition #####################################
Summary:	The Anoubis Security Suite
Name:		anoubis
Version:	@@VERSION@@
Release:	1
License:	BSD
Group:		System Environment/Base
URL:		http://www.genua.de
Source:		%{name}-%{version}.tar.gz
Vendor:		GeNUA mbH
BuildRoot:	%(mktemp -d %{_tmppath}/%{name}-%{version}-build.XXXX)

### dependencies ###########################################
BuildRequires:	autoconf
BuildRequires:	check-devel >= 0.9.4

# long descpritive part of the packaged software goes here
%description
The Anoubis Security Suite.

### subpackage anoubisd ####################################
%package anoubisd
Summary:	daemon of the Anoubis Security Suite
Group:		System Environment/Daemons

%description anoubisd
Central daemon of the Anoubis Security Suite.

### subpackage xanoubis ####################################
%package xanoubis
Summary:	GUI of the Anoubis Security Suite
Group:		User Interface/X
BuildRequires:	wxGTK-devel >= 2.8
BuildRequires:	libstdc++-devel

%description xanoubis
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
 --mandir=%{_mandir}

%install
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] \
  && rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

# remove files that should not be part of the rpm
rm -f $RPM_BUILD_ROOT%{_includedir}/anoubischat.h
rm -f $RPM_BUILD_ROOT%{_libdir}/libanoubischat.a

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] \
  && rm -rf $RPM_BUILD_ROOT


### files of subpackage anoubisd ###########################
%files anoubisd
%defattr(-,root,root)
%{_prefix}/sbin/anoubisd
%{_mandir}/man8/anoubisd.8.gz

### files of subpackage xanoubis ###########################
%files xanoubis
%defattr(-,root,root)
%doc AUTHORS INSTALL NEWS README ChangeLog
%{_prefix}/bin/xanoubis
%{_mandir}/man1/xanoubis.1.gz
%{_mandir}/man3/anoubischat.3.gz
%{_prefix}/share/xanoubis/icons/*.png
%{_prefix}/share/xanoubis/icons/*.gif


### changelog ##############################################
%changelog
* Fri Feb 01 2008 Sebastian Trahm
- first packaging for Fedora Core 8 (werewolf)
