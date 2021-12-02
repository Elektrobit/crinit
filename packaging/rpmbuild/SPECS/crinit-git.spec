Summary: EB BaseOS Configurable Rootfs Init
Name: crinit
Group: System/Base
Version: 0.2git%{gitrev_}
Release: 1%{?dist}
Source0: crinit-git-%{gitrev_}.tar.gz
License: Closed

%description
The EB BaseOS Configurable Rootfs Init Daemon including the client API shared library and the crinit-ctl CLI interface.

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
Summary: EB BaseOS Configurable Rootfs Init - Client development headers
Group:  Development/Languages/C and C++

%description devel
Development headers for client programs willing to use the client API of the Configurable Rootfs Init Daemon.

%prep
%setup -q -c

%build
make

%install
mkdir -p %{buildroot}/%{_bindir}
mkdir -p %{buildroot}/%{_libdir}
install -m 0755 crinit %{buildroot}/%{_bindir}
install -m 0755 crinit-ctl  %{buildroot}/%{_bindir}
install -m 0755 lib/*.so  %{buildroot}/%{_libdir}

# conf-example
mkdir -p %{buildroot}/%{_sysconfdir}/crinit/test
install -D -m 0644 config/test/*.crinit %{buildroot}/%{_sysconfdir}/crinit/test
install -D -m 0644 config/test/*.series %{buildroot}/%{_sysconfdir}/crinit/test

# devel
mkdir -p %{buildroot}/%{_includedir}
install -m 0644 inc/crinit-client.h %{buildroot}/%{_includedir}
install -m 0644 inc/crinit-sdefs.h %{buildroot}/%{_includedir}

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
%{_libdir}/*.so

%files conf-example
%{_sysconfdir}/crinit/test/*.crinit
%{_sysconfdir}/crinit/test/*.series

%files devel
%{_includedir}/crinit-client.h
%{_includedir}/crinit-sdefs.h

%ifarch aarch64
%files conf-s32g
%{_sysconfdir}/crinit/*.crinit
%{_sysconfdir}/crinit/*.series
%endif

