Summary: The crinit init system
Name: crinit
Group: System/Base
Version: 0.12.5
%global soversion_ 0
Release: 1
Source0: %{name}.tar.gz
License: Closed
%if "%{_vendor}" == "debbuild"
# Needed to set Maintainer in output debs
Packager: Andreas Zdziarstek <andreas.zdziarstek@emlix.com>
%endif
BuildRequires: cmake
BuildRequires: re2c
BuildRequires: libcmocka-dev

%description
The crinit daemon including the client API shared library and the crinit-ctl CLI interface.

%package shutdown
Summary: Symlinks to crinit-ctl for poweroff/reboot
Group: System/Base

%description shutdown
Reboot and poweroff binaries using Crinit's client API.

%package machine-id-gen
Summary: Machine-ID generator example application
Group: System/Base

%description machine-id-gen
Example application setting /etc/machine-id either from Kernel command line or from NXP S32G SoC OTP memory.

%package conf-example
Summary: Crinit example/test configuration files
Group: System/Base

%description conf-example
Example and test configuration files for Crinit.

%package devel
Summary: Crinit - Client development files
Group:  Development/Languages/C and C++

%description devel
Development files for client programs willing to use the client API of crinit.

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
install -D -m 0644 config/test/*.crincl %{buildroot}/%{_sysconfdir}/crinit/test
mkdir -p %{buildroot}/%{_sysconfdir}/crinit/example
install -D -m 0644 config/example/*.crinit %{buildroot}/%{_sysconfdir}/crinit/example
install -D -m 0644 config/example/*.series %{buildroot}/%{_sysconfdir}/crinit/example

# devel
mkdir -p %{buildroot}/%{_includedir}
mkdir -p %{buildroot}/%{_libdir}
install -m 0644 inc/crinit-client.h %{buildroot}/%{_includedir}
install -m 0644 inc/crinit-sdefs.h %{buildroot}/%{_includedir}
ln -sf libcrinit-client.so.%{soversion_} %{buildroot}/%{_libdir}/libcrinit-client.so

# tests reports
mkdir -p %{buildroot}/opt/testing
install -m 0644 crinit-test-report.xml %{buildroot}/opt/testing

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
%{_sysconfdir}/crinit/test/*.crincl
%{_sysconfdir}/crinit/example/*.crinit
%{_sysconfdir}/crinit/example/*.series

%files devel
%{_includedir}/crinit-client.h
%{_includedir}/crinit-sdefs.h
%{_libdir}/libcrinit-client.so

%files tests-reports
%defattr (-, root, root)
%dir /opt/testing
/opt/testing/*
