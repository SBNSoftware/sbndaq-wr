########################################################################
## Makefile that check submodules and run their Makefile
##
##
## Authors: 
##      - Benoit Rat (Seven Solutions, www.sevensols.com)
##
## GNU Lesser General Public License Usage
## This file may be used under the terms of the GNU Lesser
## General Public License version 2.1 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL included in the
## packaging of this file.  Please review the following information to
## ensure the GNU Lesser General Public License version 2.1 requirements
## will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
########################################################################

SUBMOD_DIRS=spec-sw etherbone
MAKE_DIRS = spec-sw etherbone/api #tools

#RUNME := $(shell test -d $(FMC_DRV) || git submodule update --init)


## Call the sub folder Makefiles
all clean install: init
	@for d in $(MAKE_DIRS); do $(MAKE) -C $$d $@ || exit 1; done
	@if [ "x$@" = "xinstall" ]; then ./scripts/wr-ssk-get -i; fi

## Init repo and create the .INIT file to not do it again
init: .INIT

.INIT: 
	git submodule init 
	@ $(MAKE) update
	@touch .INIT; \

## Force updating the submodules and fetch new gateware
update:
	./scripts/wr-ssk-get -f
	git submodule update
	cd spec-sw
	git submodule update


