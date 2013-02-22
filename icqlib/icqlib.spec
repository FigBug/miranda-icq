%define	name	icqlib
%define	version	1.0.0
%define	release	1
%define	prefix	/usr
%define	serial	1

Summary:	ICQ library implementation of Mirabilis' ICQ protocol.
Name:		%{name}
Version:	%{version}
Release:	%{release}
Serial:		%{serial}
Prefix:		%{prefix}
Copyright:	GPL
Group:		System Environment/Libraries
Vendor:		Denis V. Dmitrienko <denis@null.net>
Url:		http://kicq.sourceforge.net/
Source:		%{name}-%{version}.tar.gz
Buildroot:	%{_tmppath}/%{name}-%{version}-root

%description
Icqlib is the most feature complete, open source, library implementation
of Mirabilis' ICQ protocol available on the Internet. icqlib currently
supports approximately 90% of the ICQ UDP v5 protocol and 80% of the
ICQ TCP v2 protocol, including new UIN registration, chat, and file
transfer.

Core Developers
---------------
Denis V. Dmitrienko <denis@null.net>
Bill Soudan <soudan@kde.org>

Secondary Developers
--------------------
Vadim Zaliva <lord@crocodile.org>
Nikita I. Makeev <whale@nichego.net> (retired ;)

%prep
%setup -q

%build
export CFLAGS="${RPM_OPT_FLAGS}" CXXFLAGS="${RPM_OPT_FLAGS}";
./configure --prefix=%{prefix}
make

%install
[ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT;
mkdir -p $RPM_BUILD_ROOT%{prefix}
make prefix=$RPM_BUILD_ROOT%{prefix} install

%clean
[ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT;

%post	-p /sbin/ldconfig
%postun	-p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog DEVEL README* TODO
%{prefix}/include/icq.h
%{prefix}/lib/libicq*

%changelog

