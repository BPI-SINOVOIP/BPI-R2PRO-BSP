Name: cgicc			
Version: 3.2.9	
Release:	1%{?dist}
Summary: GNU cgicc is an ANSI C++ compliant class library that greatly simplifies the creation of CGI applications for the World Wide Web. cgicc performs the following functions.Parses both GET and POST form data transparently.Provides string, integer, floating-point and single- and multiple-choice retrieval methods for form data.Provides methods for saving and restoring CGI environments to aid in application debugging.Provides full on-the-fly HTML generation capabilities, with support for cookies.Supports HTTP file upload.Compatible with FastCGI.

Group:	cgicc	
License:GPL v 3	
URL:	 http://www.gnu.org/cgicc	
Source0:http://ftp.gnu.org/gnu/cgicc/%{name}-%{version}.tar.gz
	
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: gettext
BuildRequires: automake
BuildRequires: autoconf
BuildRequires: libtool
BuildRequires: doxygen

%description
Introduction to GNU Cgicc

GNU cgicc is an ANSI C++ compliant class library that greatly simplifies the creation of CGI applications for the World Wide Web. cgicc performs the following functions:

    * Parses both GET and POST form data transparently.
    * Provides string, integer, floating-point and single- and multiple-choice retrieval methods for form data.
    * Provides methods for saving and restoring CGI environments to aid in application debugging.
    * Provides full on-the-fly HTML generation capabilities, with support for cookies.
    * Supports HTTP file upload.
    * Compatible with FastCGI. 
%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%doc /usr/doc/* 
%doc /usr/share/*
%{_bindir}/*
%{_libdir}/*
%{_includedir}/*


%changelog

