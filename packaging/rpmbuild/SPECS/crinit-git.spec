Summary: EB BaseOS Configurable Rootfs Init
Name: crinit
Group: System/Base
Version: 0.3.4
%global soversion_ 0
Release: 1
Source0: crinit-%{version}.tar.gz
License: Closed
BuildRequires: cmake

%description
The EB BaseOS Configurable Rootfs Init Daemon including the client API shared library and the crinit-ctl CLI interface.

%package shutdown
Summary: EB BaseOS poweroff/reboot
Group: System/Base

%description shutdown
Reboot and poweroff binaries using Crinit's client API.

%package conf-example
Summary: EB BaseOS example/test configuration files
Group: System/Base

%description conf-example
Example configuration files for the EB BaseOS Configurable Rootfs Init Daemon.

%ifarch aarch64
%package conf-s32g
Summary: EB BaseOS configuration files for NXP S32G
Group: System/Base

%description conf-s32g
The configuration files for the EB BaseOS Configurable Rootfs Init Daemon used on S32G.

%endif

%package devel
Summary: EB BaseOS Configurable Rootfs Init - Client development files
Group:  Development/Languages/C and C++

%description devel
Development files for client programs willing to use the client API of the Configurable Rootfs Init Daemon.

%prep
%setup

%build
cmake . -DUNIT_TESTS=Off
make

%install
mkdir -p %{buildroot}/%{_bindir}
mkdir -p %{buildroot}/%{_libdir}
install -m 0755 src/crinit %{buildroot}/%{_bindir}
install -m 0755 src/crinit-ctl %{buildroot}/%{_bindir}
install -m 0755 src/libcrinit-client.so.%{version} %{buildroot}/%{_libdir}
ln -sf libcrinit-client.so.%{version} %{buildroot}/%{_libdir}/libcrinit-client.so.%{soversion_}

# shutdown
mkdir -p %{buildroot}/sbin
ln -sf /%{_bindir}/crinit-ctl %{buildroot}/sbin/poweroff
ln -sf /%{_bindir}/crinit-ctl %{buildroot}/sbin/reboot

# conf-example
mkdir -p %{buildroot}/%{_sysconfdir}/crinit/test
install -D -m 0644 config/test/*.crinit %{buildroot}/%{_sysconfdir}/crinit/test
install -D -m 0644 config/test/*.series %{buildroot}/%{_sysconfdir}/crinit/test

# devel
mkdir -p %{buildroot}/%{_includedir}
mkdir -p %{buildroot}/%{_libdir}
install -m 0644 inc/crinit-client.h %{buildroot}/%{_includedir}
install -m 0644 inc/crinit-sdefs.h %{buildroot}/%{_includedir}
ln -sf libcrinit-client.so.%{soversion_} %{buildroot}/%{_libdir}/libcrinit-client.so

%ifarch aarch64
# conf-s32g
mkdir -p %{buildroot}/%{_sysconfdir}/crinit
install -D -m 0644 config/s32g/*.crinit %{buildroot}/%{_sysconfdir}/crinit
install -D -m 0644 config/s32g/*.series %{buildroot}/%{_sysconfdir}/crinit
%endif

%files
%doc README.md
%{_bindir}/crinit
%{_bindir}/crinit-ctl
%{_libdir}/libcrinit-client.so.%{version}
%{_libdir}/libcrinit-client.so.%{soversion_}

%files shutdown
/sbin/poweroff
/sbin/reboot

%files conf-example
%{_sysconfdir}/crinit/test/*.crinit
%{_sysconfdir}/crinit/test/*.series

%files devel
%{_includedir}/crinit-client.h
%{_includedir}/crinit-sdefs.h
%{_libdir}/libcrinit-client.so

%ifarch aarch64
%files conf-s32g
%{_sysconfdir}/crinit/*.crinit
%{_sysconfdir}/crinit/*.series
%endif

