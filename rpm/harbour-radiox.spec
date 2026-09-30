Name:       harbour-radiox
Summary:    radio x — Frankfurter Stadtradio schedule, recordings and livestream
Version:    1.0.0
Release:    1
Group:      Utility
License:    MIT
URL:        https://github.com/gobuki/harbour-radiox
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  gcc-c++

%description
harbour-radiox displays the weekly program schedule of radio x
(Frankfurter Stadtradio, FM 91,8), browse the +7 mediathek
recordings, read sendetipps, and listen to the livestream.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

for SIZE in 86 108 128 172; do
  mkdir -p %{buildroot}%{_datadir}/icons/hicolor/${SIZE}x${SIZE}/apps
  install -m 644 rpm/icons/${SIZE}x${SIZE}/%{name}.png \
    %{buildroot}%{_datadir}/icons/hicolor/${SIZE}x${SIZE}/apps/%{name}.png
done

mkdir -p %{buildroot}%{_datadir}/metainfo
install -m 644 rpm/%{name}.appdata.xml \
  %{buildroot}%{_datadir}/metainfo/%{name}.metainfo.xml

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
/usr/share/metainfo/%{name}.metainfo.xml
