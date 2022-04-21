Summary: Spec file for ncplot
Name: ncplot
Version: 4.9
Release: 14%{?dist}
License: GPL
Group: System Environment/Daemons
Url: http://www.eol.ucar.edu/
Packager: Chris Webster <cjw@ucar.edu>
# becomes RPM_BUILD_ROOT
BuildRoot:  %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Vendor: UCAR
BuildArch: x86_64
Requires: netcdf libpng gsl

%if 0%{?rhel} <= 7
Requires: openmotif
%else
Requires: motif
%endif

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

echo
echo Optionally \'yum install GMT \"gshhg*\"\' for geo-political boundaries database.
echo


%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/%{name}

%changelog
* Thu Apr 30 2022 Chris Webster <cjw@ucar.edu> - 4.9-13
- Fix wind barb bug instroduced in 2017.  Fixes core dump when engaging.
* Sun May 30 2021 Chris Webster <cjw@ucar.edu> - 4.9-12
- Improve box-zoom behavior when going off right edge of time-series.
* Sat Mar 13 2021 Chris Webster <cjw@ucar.edu> - 4.9-11
- Fix Help Menu.  Use system "opener" instead of forking firefox.
* Thu Jul 13 2011 Chris Webster <cjw@ucar.edu> - 1.0-2
- updates for 4.9.1
* Thu Sep 3 2009 Chris Webster <cjw@ucar.edu> - 1.0-1
- initial version
