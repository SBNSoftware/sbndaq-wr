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

%description
Load the White Rabbit SPEC and WR-NIC drivers

%prep
%setup -c

%build

%install
[ $RPM_BUILD_ROOT != / ] && rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT{/usr/bin,/usr/lib/firmware/fmc,/etc/init.d,/etc/rc.d,/etc/rc.d/rc0.d,/etc/rc.d/rc1.d,/etc/rc.d/rc2.d,/etc/rc.d/rc3.d,/etc/rc.d/rc4.d,/etc/rc.d/rc5.d,/etc/rc.d/rc6.d,/lib/modules/%{_kernel_release}/extra,/lib/modules/%{_kernel_release}/weak-updates}

cp wr-starting-kit/spec-sw/fmc-bus/kernel/fmc.ko \
  $RPM_BUILD_ROOT/lib/modules/%{_kernel_release}/extra
cp wr-starting-kit/spec-sw/fmc-bus/kernel/fmc-fakedev.ko \
  $RPM_BUILD_ROOT/lib/modules/%{_kernel_release}/extra
cp wr-starting-kit/spec-sw/fmc-bus/kernel/fmc-chardev.ko \
  $RPM_BUILD_ROOT/lib/modules/%{_kernel_release}/extra
cp wr-starting-kit/spec-sw/fmc-bus/kernel/fmc-trivial.ko \
  $RPM_BUILD_ROOT/lib/modules/%{_kernel_release}/extra
cp wr-starting-kit/spec-sw/fmc-bus/kernel/fmc-write-eeprom.ko \
  $RPM_BUILD_ROOT/lib/modules/%{_kernel_release}/extra
cp wr-starting-kit/spec-sw/kernel/spec.ko \
  $RPM_BUILD_ROOT/lib/modules/%{_kernel_release}/extra
cp wr-starting-kit/spec-sw/kernel/wr-nic.ko \
  $RPM_BUILD_ROOT/lib/modules/%{_kernel_release}/extra

cp wr-starting-kit/spec-sw/tools/spec-cl       \
  $RPM_BUILD_ROOT/usr/bin
cp wr-starting-kit/spec-sw/tools/spec-fwloader \
  $RPM_BUILD_ROOT/usr/bin
cp wr-starting-kit/spec-sw/tools/specmem       \
  $RPM_BUILD_ROOT/usr/bin
cp wr-starting-kit/spec-sw/tools/spec-vuart    \
  $RPM_BUILD_ROOT/usr/bin
cp wr-starting-kit/spec-sw/tools/stamp-frame   \
  $RPM_BUILD_ROOT/usr/bin
cp wr-starting-kit/spec-sw/tools/wr-dio-agent  \
  $RPM_BUILD_ROOT/usr/bin
cp wr-starting-kit/spec-sw/tools/wr-dio-cmd    \
  $RPM_BUILD_ROOT/usr/bin
cp wr-starting-kit/spec-sw/tools/wr-dio-pps    \
  $RPM_BUILD_ROOT/usr/bin
cp wr-starting-kit/spec-sw/tools/wr-dio-ruler  \
  $RPM_BUILD_ROOT/usr/bin

cp wr-starting-kit/firmware/spec-init.bin  \
  $RPM_BUILD_ROOT/usr/lib/firmware/fmc/
cp wr-starting-kit/firmware/spec_sbnd.bin  \
  $RPM_BUILD_ROOT/usr/lib/firmware/fmc/
cp wr-starting-kit/firmware/wr_nic_dio.bin  \
  $RPM_BUILD_ROOT/usr/lib/firmware/fmc/

install -m 755 etc/spec $RPM_BUILD_ROOT/etc/init.d/.
ln -s /etc/rc.d/init.d/spec $RPM_BUILD_ROOT/etc/rc.d/rc0.d/K50spec
ln -s /etc/rc.d/init.d/spec $RPM_BUILD_ROOT/etc/rc.d/rc1.d/K50spec
ln -s /etc/rc.d/init.d/spec $RPM_BUILD_ROOT/etc/rc.d/rc2.d/K50spec
ln -s /etc/rc.d/init.d/spec $RPM_BUILD_ROOT/etc/rc.d/rc3.d/S50spec
ln -s /etc/rc.d/init.d/spec $RPM_BUILD_ROOT/etc/rc.d/rc4.d/S50spec
ln -s /etc/rc.d/init.d/spec $RPM_BUILD_ROOT/etc/rc.d/rc5.d/S50spec
ln -s /etc/rc.d/init.d/spec $RPM_BUILD_ROOT/etc/rc.d/rc6.d/K50spec

%clean
[ $RPM_BUILD_ROOT != / ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
/lib/modules/%{_kernel_release}/extra/spec.ko
/lib/modules/%{_kernel_release}/extra/wr-nic.ko
/lib/modules/%{_kernel_release}/extra/fmc.ko
/lib/modules/%{_kernel_release}/extra/fmc-chardev.ko
/lib/modules/%{_kernel_release}/extra/fmc-fakedev.ko
/lib/modules/%{_kernel_release}/extra/fmc-trivial.ko
/lib/modules/%{_kernel_release}/extra/fmc-write-eeprom.ko
/etc/init.d/spec
/etc/rc.d/rc0.d/K50spec
/etc/rc.d/rc1.d/K50spec
/etc/rc.d/rc2.d/K50spec
/etc/rc.d/rc3.d/S50spec
/etc/rc.d/rc4.d/S50spec
/etc/rc.d/rc5.d/S50spec
/etc/rc.d/rc6.d/K50spec
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

%post
ln -sf  /lib/modules/%{_kernel_release}/extra/fmc.ko \
  /lib/modules/%{_kernel_release}/weak-updates/.
ln -sf  /lib/modules/%{_kernel_release}/extra/fmc-fakedev.ko \
  /lib/modules/%{_kernel_release}/weak-updates/.
ln -sf  /lib/modules/%{_kernel_release}/extra/fmc-chardev.ko \
  /lib/modules/%{_kernel_release}/weak-updates/.
ln -sf  /lib/modules/%{_kernel_release}/extra/fmc-trivial.ko \
  /lib/modules/%{_kernel_release}/weak-updates/.
ln -sf  /lib/modules/%{_kernel_release}/extra/fmc-write-eeprom.ko \
  /lib/modules/%{_kernel_release}/weak-updates/.
ln -sf  /lib/modules/%{_kernel_release}/extra/spec.ko \
  /lib/modules/%{_kernel_release}/weak-updates/.
ln -sf  /lib/modules/%{_kernel_release}/extra/wr-nic.ko \
  /lib/modules/%{_kernel_release}/weak-updates/.

/usr/bin/systemctl daemon-reload
/sbin/service spec start
chmod +s /usr/bin/spec-cl       
chmod +s /usr/bin/spec-fwloader 
chmod +s /usr/bin/specmem       
chmod +s /usr/bin/spec-vuart    
chmod +s /usr/bin/stamp-frame   
chmod +s /usr/bin/wr-dio-agent  
chmod +s /usr/bin/wr-dio-cmd    
chmod +s /usr/bin/wr-dio-pps    
chmod +s /usr/bin/wr-dio-ruler  

%preun
/usr/bin/systemctl daemon-reload
/sbin/service spec stop
