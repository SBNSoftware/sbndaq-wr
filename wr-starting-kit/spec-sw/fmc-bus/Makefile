
# We have a problem here: this package may be a submodule of something
# else (for example: fine-delay). If this is the case, fmc-bus is on ../
# This external script returns the pathname of the "kernel" subdir of fmc-bus
FMC_DRV ?= $(shell ./check-fmc-bus)
export FMC_DRV

RUNME := $(shell test -d $(FMC_DRV) || git submodule update --init)

DIRS = $(FMC_DRV) kernel tools

all clean modules install modules_install:
	for d in $(DIRS); do $(MAKE) -C $$d $@ || exit 1; done
