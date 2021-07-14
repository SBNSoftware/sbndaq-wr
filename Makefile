#
# Make and install SPEC and WR-NIC device drivers, wrapped around starting kit
#

SPEC_VERSION=$(shell grep 'define _libversion' spec.spec|cut -d ' ' -f 3)
SPEC_RELEASE=$(shell grep 'Release:' spec.spec | cut -d ' ' -f 2)


default:
	cd wr-starting-kit; make
	tar cvf spec.tar \
		etc/spec \
		wr-starting-kit/firmware/spec-init.bin \
		wr-starting-kit/firmware/spec_sbnd.bin \
		wr-starting-kit/firmware/wr_nic_dio.bin \
		wr-starting-kit/spec-sw/fmc-bus/kernel/fmc.ko \
		wr-starting-kit/spec-sw/fmc-bus/kernel/fmc-chardev.ko \
		wr-starting-kit/spec-sw/fmc-bus/kernel/fmc-fakedev.ko \
		wr-starting-kit/spec-sw/fmc-bus/kernel/fmc-trivial.ko \
		wr-starting-kit/spec-sw/fmc-bus/kernel/fmc-write-eeprom.ko \
		wr-starting-kit/spec-sw/kernel/spec.ko \
		wr-starting-kit/spec-sw/kernel/wr-nic.ko \
		wr-starting-kit/spec-sw/tools/spec-cl       \
		wr-starting-kit/spec-sw/tools/spec-fwloader \
		wr-starting-kit/spec-sw/tools/specmem       \
		wr-starting-kit/spec-sw/tools/spec-vuart    \
		wr-starting-kit/spec-sw/tools/stamp-frame   \
		wr-starting-kit/spec-sw/tools/wr-dio-agent  \
		wr-starting-kit/spec-sw/tools/wr-dio-cmd    \
		wr-starting-kit/spec-sw/tools/wr-dio-pps    \
		wr-starting-kit/spec-sw/tools/wr-dio-ruler  

RPM_STATUS := $(shell rpm -q spec)
ifeq ($(RPM_STATUS),package spec is not installed)
  UNINSTALL_CMD = echo $(RPM_STATUS)
else
  UNINSTALL_CMD = ksu root -e /bin/rpm -e spec
endif

install:
	$(UNINSTALL_CMD)
	mkdir -p $(HOME)/rpmbuild/SOURCES
	cp spec.tar $(HOME)/rpmbuild/SOURCES/.
	rpmbuild -bb spec.spec
	ls $(HOME)/rpmbuild/RPMS/`uname -i`
	ksu root -e /bin/rpm -i $(HOME)/rpmbuild/RPMS/`uname -i`/spec-$(SPEC_VERSION)-$(SPEC_RELEASE).`uname -i`.rpm

uninstall:
	$(UNINSTALL_CMD)

clean:
	rm -f spec.tar
	cd wr-starting-kit; make clean


