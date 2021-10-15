Summary: EB BaseOS Configurable Rootfs Init
Name: crinit
Version: 0.2git%{gitrev_}
Release: 1%{?dist}
Source0: crinit-git-%{gitrev_}.tar.gz
License: Closed

%description
The EB BaseOS Configurable Rootfs Init Daemon

%prep
%setup -q -c

%build
make

%install
mkdir -p %{buildroot}/%{_bindir}
mkdir -p %{buildroot}/%{_sysconfdir}/crinit/test
mkdir -p %{buildroot}/%{_libdir}/crinit
install -m 0755 crinit %{buildroot}/%{_bindir}
install -m 0755 crinit-ctl  %{buildroot}/%{_bindir}
install -m 0755 lib/*.so  %{buildroot}/%{_libdir}/crinit
install -D -m 0644 config/s32g/*.crinit %{buildroot}/%{_sysconfdir}/crinit
install -D -m 0644 config/s32g/*.series %{buildroot}/%{_sysconfdir}/crinit
install -D -m 0644 config/test/*.crinit %{buildroot}/%{_sysconfdir}/crinit/test
install -D -m 0644 config/test/*.series %{buildroot}/%{_sysconfdir}/crinit/test

%files
%doc README.md
%{_bindir}/crinit
%{_bindir}/crinit-ctl
%{_sysconfdir}/crinit/*
%{_libdir}/crinit/*.so

