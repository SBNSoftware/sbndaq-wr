%define _libversion 1.0.0
%define _kernel_release %(uname -r)

Name: spec
Version: %{_libversion}
Release: 1
Summary: Install White Rabbit SPEC and WR-NIC drivers
Packager: William Badgett
Source: %{name}.tar
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
ExclusiveOs: linux
Provides: spec.ko
Prefix: %{_prefix}
License: GPL
%undefine _debugsource_packages
%{?systemd_requires}


%description
Load the White Rabbit SPEC and WR-NIC drivers


%prep
%setup -c


%build


%install

[ $RPM_BUILD_ROOT != / ] && rm -rf $RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/lib/firmware/fmc
mkdir -p $RPM_BUILD_ROOT/lib/modules/%{_kernel_release}/extra
mkdir -p $RPM_BUILD_ROOT/lib/modules/%{_kernel_release}/weak-updates
mkdir -p $RPM_BUILD_ROOT/usr/local/sbin
mkdir -p $RPM_BUILD_ROOT/usr/lib/systemd/system

# Kernel modules
cp wr-starting-kit/spec-sw/fmc-bus/kernel/*.ko \
   $RPM_BUILD_ROOT/lib/modules/%{_kernel_release}/extra
cp wr-starting-kit/spec-sw/kernel/spec.ko \
   $RPM_BUILD_ROOT/lib/modules/%{_kernel_release}/extra
cp wr-starting-kit/spec-sw/kernel/wr-nic.ko \
   $RPM_BUILD_ROOT/lib/modules/%{_kernel_release}/extra

# User-space tools
cp wr-starting-kit/spec-sw/tools/* \
   $RPM_BUILD_ROOT/usr/bin

# Firmware
cp wr-starting-kit/firmware/*.bin \
   $RPM_BUILD_ROOT/usr/lib/firmware/fmc/

# Systemd service and helpers
install -m 644 etc/spec.service  $RPM_BUILD_ROOT/usr/lib/systemd/system/


%clean
[ $RPM_BUILD_ROOT != / ] && rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
/lib/modules/%{_kernel_release}/extra/*.ko
/usr/bin/spec-cl
/usr/bin/spec-fwloader
/usr/bin/specmem
/usr/bin/spec-vuart
/usr/bin/stamp-frame
/usr/bin/wr-dio-agent
/usr/bin/wr-dio-cmd
/usr/bin/wr-dio-pps
/usr/bin/wr-dio-ruler
/usr/lib/firmware/fmc/spec-init.bin
/usr/lib/firmware/fmc/spec_sbnd.bin
/usr/lib/firmware/fmc/wr_nic_dio.bin
/usr/lib/systemd/system/spec.service


%post
# Create weak-updates symlinks
ln -sf /lib/modules/%{_kernel_release}/extra/fmc.ko                /lib/modules/%{_kernel_release}/weak-updates/
ln -sf /lib/modules/%{_kernel_release}/extra/fmc-fakedev.ko        /lib/modules/%{_kernel_release}/weak-updates/
ln -sf /lib/modules/%{_kernel_release}/extra/fmc-chardev.ko        /lib/modules/%{_kernel_release}/weak-updates/
ln -sf /lib/modules/%{_kernel_release}/extra/fmc-trivial.ko        /lib/modules/%{_kernel_release}/weak-updates/
ln -sf /lib/modules/%{_kernel_release}/extra/fmc-write-eeprom.ko   /lib/modules/%{_kernel_release}/weak-updates/
ln -sf /lib/modules/%{_kernel_release}/extra/spec.ko               /lib/modules/%{_kernel_release}/weak-updates/
ln -sf /lib/modules/%{_kernel_release}/extra/wr-nic.ko             /lib/modules/%{_kernel_release}/weak-updates/

## Enable and reload systemd unit
#%systemd_post spec.service

# Setuid on tools
chmod +s /usr/bin/spec-cl
chmod +s /usr/bin/spec-fwloader
chmod +s /usr/bin/specmem
chmod +s /usr/bin/spec-vuart
chmod +s /usr/bin/stamp-frame
chmod +s /usr/bin/wr-dio-agent
chmod +s /usr/bin/wr-dio-cmd
chmod +s /usr/bin/wr-dio-pps
chmod +s /usr/bin/wr-dio-ruler


#%preun
#%systemd_preun spec.service

#%postun
#%systemd_postun_with_restart spec.service
