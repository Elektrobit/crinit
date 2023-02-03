Summary: EB BaseOS Configurable Rootfs Init
Name: crinit
Group: System/Base
Version: 0.8.1
%global soversion_ 0
Release: 1
Source0: %{name}.tar.gz
License: Closed
%if "%{_vendor}" == "debbuild"
# Needed to set Maintainer in output debs
Packager: Rainer Müller <rainer.mueller@emlix.com>
%endif
BuildRequires: cmake
BuildRequires: re2c
BuildRequires:  libcmocka-dev

%description
The EB BaseOS Configurable Rootfs Init Daemon including the client API shared library and the crinit-ctl CLI interface.

%package shutdown
Summary: EB BaseOS poweroff/reboot
Group: System/Base

%description shutdown
Reboot and poweroff binaries using Crinit's client API.

%package machine-id-gen
Summary: Machine-ID generator example application
Group: System/Base

%description machine-id-gen
Example application setting /etc/machine-id either from Kernel command line or from S32G OTP memory.

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

%package tests-reports
Summary: The unit tests report package
Group: System/Base

%description tests-reports
tests reports and coverage reports for unit tests run on build.

%prep
%setup -n %{name}

%build
cmake . \
    -DCMAKE_BUILD_TYPE=Release \
    -DUNIT_TESTS=ON \
    -DMACHINE_ID_EXAMPLE=ON \
    -DENABLE_ASAN=OFF \
    -DENABLE_ANALYZER=OFF \
    -DENABLE_WERROR=ON
make
# in case of no unit tests yet
touch crinit-test-report.xml
ctest --output-on-failure --output-junit crinit-test-report.xml

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

# machine-id-gen
install -m 0755 src/machine-id-gen %{buildroot}/%{_bindir}
mkdir -p %{buildroot}/etc
ln -sf /run/machine-id %{buildroot}/etc/machine-id

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

# tests reports
mkdir -p %{buildroot}/opt/testing
install -m 0644 crinit-test-report.xml %{buildroot}/opt/testing

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

%files machine-id-gen
%{_bindir}/machine-id-gen
/etc/machine-id

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

%files tests-reports
%defattr (-, root, root)
%dir /opt/testing
/opt/testing/*

