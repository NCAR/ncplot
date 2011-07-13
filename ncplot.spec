Summary: Spec file for ncplot
Name: ncplot
Version: 4.9
Release: 1
License: GPL
Group: System Environment/Daemons
Url: http://www.eol.ucar.edu/
Packager: Chris Webster <cjw@ucar.edu>
# becomes RPM_BUILD_ROOT
BuildRoot:  %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Vendor: UCAR
BuildArch: i386
Requires: openmotif netcdf libpng
Source: ftp://ftp.eol.ucar.edu/pub/archive/RAF-src/%{name}.tar.gz

%description
Configuration for NCAR-EOL ncplot aircraft netCDF plotting tool.

%prep
%setup -n %{name}

%build
make

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_bindir}
cp %{name} %{buildroot}%{_bindir}

%post


%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/%{name}

%changelog
* Thu Jul 13 2011 Chris Webster <cjw@ucar.edu> - 1.0-2
- updates for 4.9.1
* Thu Sep 3 2009 Chris Webster <cjw@ucar.edu> - 1.0-1
- initial version
