Summary: network performance monitoring probe
Name: freenote-probe
Version: 1.0.0rc9
Release: 1
Copyright: GPL
Group: Applications/Internet 
Source: http://freenote.petta-tech.com/downloads/freenote-probe-1.0.0rc9.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot

%description
The FreeNote Probe is a program designed to simulate user
agent actions in order to automatically test the performance
and reliability of network services.

%prep
%setup -q

%build
./configure
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/man/man1

install -s -m 755 src/fnprobe $RPM_BUILD_ROOT/usr/bin/fnprobe
install -m 644 doc/fnprobe.1 $RPM_BUILD_ROOT/usr/man/man1/fnprobe.1

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README COPYING

/usr/bin/fnprobe
/usr/man/man1/fnprobe.1.gz
