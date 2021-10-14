Summary: EB BaseOS Configurable Rootfs Init
Name: crinit
Version: 0.1git%{gitrev_}
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
mkdir -p %{buildroot}/etc/crinit/test
install -m 0755 crinit %{buildroot}/%{_bindir}
install -D -m 0644 config/s32g/*.crinit %{buildroot}/etc/crinit
install -D -m 0644 config/s32g/*.series %{buildroot}/etc/crinit
install -D -m 0644 config/test/*.crinit %{buildroot}/etc/crinit/test
install -D -m 0644 config/test/*.series %{buildroot}/etc/crinit/test

%files
%doc README.md
%{_bindir}/%{name}
/etc/crinit/*

