% WR SPEC Starting Kit
% Benoit RAT, Javier Diaz (Seven Solutions) & Miguel Jimenez (UGR)


### Copyright

This document is copyrighted (under the Berne Convention) by Seven
Solutions company and is formally licensed to the public under **GPL v2.0** license.
Report content can be copied, modified, and redistributed.

The Seven Solutions Logo can not be modified in any form, or by any means without prior
written permission by Seven Solutions.

### Licenses

~~~~~~~
The "WR SPEC Starting Kit" (as defined above) is provided under the terms of GPL v2.0
Copyright (C) 2013 - Seven Solutions

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
~~~~~~~~~


The [SPEC] and [FMC-DIO] have been released under the **CERN OHL** licence.

~~~~~~~
Copyright CERN 2011.
This documentation describes Open Hardware and is licensed under the CERN OHL v. 1.1.

You may redistribute and modify this documentation under the terms of the
CERN OHL v.1.1. (http://ohwr.org/cernohl). This documentation is distributed
WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING OF
MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A
PARTICULAR PURPOSE. Please see the CERN OHL v.1.1 for applicable
conditions
~~~~~~~~~~~~~~~~~


\clearpage



### Revision table


------------------------------------------------------------------------
 Rev      Date          Author          Comments
-----  ----------- -------------------  --------------------------------
 1.0   23/05/2013   Benoit Rat\         First release
                    [Seven Solutions]

 1.1   26/07/2013   Benoit Rat\         Correct errors in submodule init
                    [Seven Solutions]

 2.0   15/05/2014   Benoit Rat\         Updating for v2.0 release
                    [Seven Solutions]
------------------------------------------------------------------------

You can also check the [Changelog section](#changelog) for more information.

\clearpage

Introduction
=============


About the White Rabbit technology
----------------------------------

White Rabbit ([WR]) is an extension to Ethernet network with **accurate
synchronization** and Gigabit data transfer capability. It has been
conceived to fulfill the following goals:\


Time Precision
:	The [WR] technology provides a common clock for physical layer in the entire network, allowing synchronization at sub-nanosecond accuracy with  picoseconds precision.

Scalability
:	The [WR] network is designed to be highly scalable up to thousands of nodes. It also intends to be as modular as possible and compatible with non-[WR] devices. Finally, all products related to this technology available at the Open Hardware Repository ([OHWR]) represent “open developments”, i.e. all the hardware designs, firmware and software and documentation are fully available. This shall facilitate the new incremental developments and customization of the available designs and products.

Distance Range
:	Taking into account the size and ranges of the majority of industrial and scientific facilities, the [WR] network specifications have been chosen to support distance ranges of 10 km between nodes using fiber cables.\


The **Starting Kit** has been designed to test and check if the White
Rabbit technology meets your requirements. It shall also facilitate the
understanding on the technology capabilities and possibilities.
WR starting kit can also be used to evaluate what can be done with it
and how to integrate it on your own project.

You can find more information on other components and use cases on:

* Our webpage <http://www.sevensols.com/whiterabbitsolution/>
* The official wiki page <http://www.ohwr.org/projects/white-rabbit/wiki>


About the Starting Kit
----------------------------------

This starting kit uses two nodes, each one composed of a [SPEC] and one
[FMC-DIO] card.

A node makes basic operations such as input timestamping or programmable output pulse generation.
Additionally, specific software and gateware layers allow to use it as a standard network
interface card implementing the White Rabbit technology functionalities.
Network packages with accurate time-stamping information are generated/timestamped at
the hardware level to achieve the highest accuracy.

It is based on different projects:

* [spec-sw]: driver to communicate to the [SPEC] card through PCIe. It
also includes a set of tools to experiments.
* [wr-nic]:  gateware that includes the NIC & DIO capabilities.
* [wrpc-sw]: white rabbit PTP firmware for the synchronization.


About this document:
--------------------

This document is intended to be a step by step user friendly tutorial
to start with the White Rabbit technology. It includes the description
of some simple experiments to illustrate [WR] capabilities. Some
concepts are deliberately avoided to ease the comprehension of this document.
If you want to know more about these concepts, please access to the
related documents in the [Reference section](#references)
and especially to the following ones:

* [spec-sw.pdf]
* [wrpc.pdf]

The rest of the document provides an explanation about the system setup, software driver, FPGA configuration and
some application examples.

This will allow easily validating the main capabilities of White Rabbit
technology.

We have also included a chapter called [Quick Start Guide for
Developers](#quick-start-guide-for-developers) in order to facilitate
the starting stages of development processes, indicating where to find
the different source projects and how to compile them.


Changelog
-----------------

In this section, we resume the main changes that happens at each release of the starting-kit

### wr-starting-kit-v2.0

* The driver now support kernel from 2.8.x to 3.5.x
* A valid EEPROM data is now **mandatory** in order to start the kernel
* The gateware is now accesible remotely/standalone through ethernet using etherbone core.
* First DIO channel output is now reserved for outputing PPS
* PPSi has been introduced to improve compatibility with other PTP devices.
* GrandMaster locking has been improved
* FMC bus has been updated to the stable version which is officialy included in linux kernel (>3.5)
* Calibration has been improved using t42p procedure


A detailed changelog can be found on the wiki page of each submodules:

* [wr-nic-v2.0](http://www.ohwr.org/projects/wr-nic/wiki/Release_v2)
* [spec-sw-v2014-02](http://www.ohwr.org/projects/spec-sw/wiki#version-2014-02)
* [fmc-bus-v2014-02](http://www.ohwr.org/news/352)
* [wrpc-v2.1](http://www.ohwr.org/projects/wr-cores/wiki/Wrpc_release-v21)
* [ppsi-v2013.11](http://www.ohwr.org/news/345)



System Setup
===========

This section describes the main elements required to start with the White-Rabbit based
on the *"White Rabbit (SPEC-based) starting kit"*. The different
hardware elements and theirs connections are described in this section.

What do you need?
----------------

In order to use the white rabbit starting kit and setup the different
experiments you will need:

* An oscillocope with at least 150Mhz bandwitdh (500Mhz is recommanded).
* A PC with at least two `PCIe x4` ports (`x8` & `x16` are also compatible)
* The Operative System **Ubuntu LTS 32bit (Long Term Support)** installed.
* Two mini-USB (B) cables (not provided with the kit).

This tutorial has been tested and verified with Ubuntu LTS 12.04 and Ubuntu LTS 14.04,
this mean that standard support will only be given for these releases.
However, the `spec-sw` driver should work with other releases, distributions and architectu
that run kernel `2.8.x` to `3.5.x`.

Finally, [Seven Solutions] also provides a fully setup PC or USB-live images to skip
the instalation process and ease the introduction to the *"White Rabbit World"*.

> ***Note:***  This tutorial follow a configuration with two [SPEC+FMCDIO] boards connected in the two **PCIe x16** interfaces of the same computer. However, you can also use two different computers (if you do not have two **PCIe x4**) and follow this tutorial; you just need to replace all the commands with `wr1` or `0x0300` by the `wr0` and its corresponding bus_id on the second PC. The configuration with two separated PCs is more *"natural"* to understand how to communicate two different nodes using the starting kit,
but it requires more physical space to implement it.

![Configuration with one or two PCs](img/ssk_configs.png)

Starting kit components
-----------------

The starting kit is composed of various elements that you should find
in the package[^standardssk] :

* 2x [SPEC]s boards
* 2x [FMC-DIO]s 5CH TTL A

* 2x SFPs LC
	* AXGE-1254-0531 (blue)
	* AXGE-3454-0531 (violet)
* 1x LC-LC cable (2m)
* 3x LEMO cable (2m)
* 3x LEMO-BNC Adaptor
* 1x Pre-installed live-USB (Ubuntu 14.04)

![The components of the starting kit](img/ssk_components.jpg)

[^standardssk]: Only with the standard version of starting kit.


Physical setup
-------------------


* Plug the [SPEC+FMCDIO] into your `PCIe x{4,8,16}` port.
* You should also connect a mini-USB cable to the UART of the board.
(A Virtual UART is also available, but it is safer to use the physical
one).
* Connect the two [SPEC+FMCDIO] boards using the SFPs and the optical fiber cable (LC-LC)
	* In the demo we have put the violet SFP on the top SPEC board (wr0)
	* ... and the blue SFP below (wr1)
* Start Ubuntu LTS.
* Prepare an oscilloscope with at least two input channels to access
the [FMCDIO] outputs.


![Mounting two SPECs in the same PC](img/ssk_inside.jpg)

![Connecting SFPs and fiber](img/ssk_sfp.jpg)

![The UART use a mini-USB cable to communicate](img/ssk_usb.jpg)

Pre-Installed Live-USB
========================

In our latest WR package we also provide a Live-USB (Ubuntu 14.04) pendrive with
the drivers & tools pre-installed.

In order to run the Live-USB from the pendrive you need to modify the
BIOS of your PC to enable booting from USB.
In the most recent BIOS you will find a `Boot options menu` by pressing
tipically `F10`, `F11` or `F12` during the startup of the PC.

Otherwise you might need to modify the Boot priority in the BIOS Configuration. To do so restart your computer, and watch for a message telling you which key to press to enter the BIOS setup. It will usually be one of F1, F2, DEL, ESC or F10. Press this key while your computer is booting to edit your BIOS settings. Once you are in the BIOS configuration, look for something similar as `Boot` > `Boot Order`. You should see an entry for `removable drive` or `USB media`. Move this to the top of the list to make the computer attempt to boot from the USB device before booting from the hard disk. If you still having trouble do perform this step please search on the web how to `Boot from USB` using your BIOS manufacturer as keyword in the research.

Once Ubuntu has started you should select your own language, and click
the `Try Ubuntu Live` options, then you can go directly to the
[Setting Master & Slave using White Rabbit Core Section](#setting-master-slave-using-white-rabbit-core)

If you prefer to use the WR Starting Kit with your own PC you need to
follow the installation steps in the [next section](#installation).

Installation
=======================

> Notes: If you are running the Live-USB pendrive you should skip this
section.

Once you have your system running with the boards plugged, you need to:

* Check that the boards has been detected
* Install the tools to build drivers, etc...
* The drivers : `spec-sw`
* The HDL bitstream of [SPEC+FMCDIO]: `wr-nic`


SPEC detection
----------------

First you should check that the boards has been correctly detected on your PCIe bus by doing the following:

~~~~~{.sh}
>:$ lspci | grep CERN
~~~~~~~~~~

You should obtain something similar as:

~~~~~{.sh}
05:00.0 Non-VGA unclassified device: CERN/ECP/EDU Device 018d (rev 03)
0b:00.0 Non-VGA unclassified device: CERN/ECP/EDU Device 018d (rev 03)
~~~~~~~~~~

Where `05` and `0b` represent the ID of the PCIe slot given by your motherboard.
> **Note**: The PCIe slot numbering are normally ordered from top to bottom on the motherboard,
this mean that the board with ID `05` will be above the one with index `0b`.


Tools
-----------------

Then you need to install all the tools that you will need:

* **git**: A powerful & distributed version control system
* **build-essentials**: Contains various binaries to build source code
* **linux-source**: Might be useful to compile drivers & kernel modules
* **minicom**: Hyperterminal for linux
* **texinfo, texlive, emacs**: Tools to build documentation


You can also run this command[^debian] for minimal setup:

~~~~{.sh}
sudo apt-get install git build-essential linux-headers-$(uname -r)  minicom
~~~~~~~~~

and this one if you also want to generate documentation (not mandatory):

~~~~{.sh}
sudo apt-get install texinfo emacs texlive pandoc
~~~~~~~~~


[^debian]: This sample command is for debian's like distributions.
However similar packages exist for other distributions.

Install the project
-------------------

The first step is to install the driver to communicate with the card using
the PCIe port. To do so you need to get the latest release[^commitgit] of [wr-starting-kit] project which
includes the [spec-sw] project.

~~~~{.sh}
## Create the "root" directory
>:$ mkdir -p ~/wr/
>:$ cd ~/wr/

## Clone the repository
>:$ git clone git://ohwr.org/white-rabbit/wr-starting-kit.git
>:$ cd wr-starting-kit

## Checkout the stable release
>:$ git checkout -b wr-starting-kit-v2.0 wr-starting-kit-v2.0
~~~~~~~~~~~~

Then you can get the project submodule by running

~~~~{.sh}
## Obtain the spec-sw project using submodules
>:$ git submodule init
>:$ git submodule update

## Update submodule of spec-sw
>:$ cd spec-sw
>:$ git submodule init
>:$ git submodule update
>:$ cd -
~~~~~~~~~~~~

Or you can try our new Makefile that should perform everything!

~~~~{.sh}
>:$ make
>:$ sudo make install
~~~~~~~~~~~~

[^commitgit]: You should only use our package or proper release (tagged commit).
The **master** branch might have the latest source but support is only offered for tagged release. The other branches are normally used for development and are not stable.



Structure of the spec-sw project
---------------------------------

Most of the information on how the project is structured can be found in the [spec-sw.pdf] in the `/doc` folder,
however in the following paragraphs we briefly summarize it:

### Folders layout

Each of the following folders contain:

* **doc**: documentation of the project
* **gateware**: downloaded HDL binaries also called gateware.\
 (This folder is created while downloading gateware)
* **kernel**: Kernel modules (drivers) to connect the [SPEC] to the PC through PCIe.
* **tools**: Set of tools used for the experiments in the Starting Kit


### Structure of the driver and HDL

The Starting Kit contains 3 different drivers:

* **fmc.ko**: Define a generic FMC-bus[^fmc-bus] used to identify and connect to FMC boards independently
of the carrier being used (e.g. [SPEC], SVEC, ...)
* **spec.ko**: It is the device driver that connects to the [SPEC] through PCI. It also loads the spec-init.bin “golden” gateware file.
* **wr-nic.ko**: The wr-nic driver is basically an Ethernet driver with
support for hardware time stamping. It also loads the following:
	* `fmc/wr_nic_dio.bin` gateware for the NIC and DIO properties.
	* `fmc/wr_nic_dio-wrc.bin` software for the LM32[^lm32inc].

[^fmc-bus]: More information about the [FMC]-bus can be found within [fmc-bus.pdf] in the `doc/` folder.
[^lm32inc]: In future version, the program file for the LM32 will be included into the gateware.


Compile & install
------------------------

A Makefile in the [spec-sw] project has been written to compile and install easily the drivers and the tools used below.

~~~~{.sh}
## Go to spec-sw project
>:$ cd spec-sw

## In the root folder (spec-sw), run
>:$ make

## then, install the driver in your system so that they load automatically
>:$ sudo make install
~~~~~~~~~~~

If everything works well you should see the driver in

~~~~{.sh}
>:$ ls -l /lib/modules/$(uname -r)/extra
~~~~~~~~~~~

> ***Notes:*** The procedure below is specific to ubuntu distribution so you
might want to find a similar way to perform it if you use another
distribution.

You should add the `extra` folder to the search of module so that at
next reboot the kenel modules are easily founded.

~~~~{.sh}
## Open the configuration file
>:$ sudo nano /etc/depmod.d/ubuntu.conf
~~~~~~~~~~~

Check if you already have the extra folder or otherwise add the `extra` keyword
before built-in.
The first line of this file should now be similar as:

	search updates ubuntu extra built-in

Finally, you must regenerate the map of dependencies with this new path
by calling:

~~~~{.sh}
>:$ sudo depmod -a
~~~~~~~~~~~


Download, install & load the gateware
--------------------------------------

You need to install the gateware[^version] to `/lib/firmware/fmc`


### Automatic procedure

You can also use the script `wr-ssk-get` in `wr-starting-kit/scripts` folder
to ease the installation. `sudo` is required

~~~~{.sh}
## Fetch and install the firmware
>:$ sudo scripts/wr-ssk-get --all
~~~~~~~~~~~~~~


### Manual procedure

First you need to download the gateware/firmware files from our website:

<http://www.sevensols.com/dl/wr-starting-kit/bin/latest_stable.tar.gz>

~~~~{.sh}
## Create the gateware folder
>:$ mkdir firmware

## Extract to the gateware folder
>:$ tar -xzf wr-starting-kit-v2.0_gw.tar.gz -C ./firmware
~~~~~~~~~~~

Once you have the file you need to install them to your system in order to make them load automatically

~~~~{.sh}
## Install the HDL binaries in /lib/firmware/fmc (require sudo)
>:$ sudo cp -v firmware/*.bin /lib/firmware/fmc
~~~~~~~~~~~~~

[^version]: The HDL binaries (gateware) came from other project, you can find in
the [Developers Section](#quick-start-guide-for-developers) where to obtain them and
how to compile them.



Loading the driver
------------------------

To enable the wr-nic you should execute the modprobe[^errmodprobe] command to load everything

~~~~{.sh}
>:$ sudo modprobe spec
>:$ sudo modprobe wr-nic
~~~~~~~~

[^errmodprobe]: We have found a problem in some distribution with `modprobe` command.
You should use `insmod` instead and  look at the [Frequently Added Questions sections](#frequently-added-questions).


You should expect to obtain two (or one) new interface(s):

~~~~{.sh}
>:$ ifconfig -a | grep wr
wr0       Link encap:Ethernet  HWaddr XX:XX:XX:XX:XX:XX
~~~~~~~~~~~~

If it is not the case you might have an empty EEPROM that need to be
written with proper value as explained in the next section.


Creating Valid EEPROM
----------------------

> ***Notes***: If you already have the new interfaces wrX while doing
`ifconfig`, this mean that you can **skip** this section.

On older manufactured [FMC-DIO] (before 2014), the EEPROM was not
configured with the proper information
which is now needed by the fmc-bus to identify properly which [FMC] has been plugged
into the [SPEC].

Therefore if you try to load the kernel driver

~~~~~{.sh}
>:$ sudo modprobe spec
~~~~~~~~~~

you might obtain the following warning on `dmesg`:

~~~~~{.sh}
>:$ dmesg
...
spec 0000:05:00.0: FPGA programming successful
spec 0000:05:00.0: mezzanine 0
EEPROM has no FRU information
~~~~~~~~~~

This information, called the FRU, contains the type of FMC board, its serial number, etc.
You can generate in two different way:

### Automatic

A small script has been created for this procedure and thus you just need to call it as below:

	scripts/wr-ssk-get -w

Then the script will detect if you have one or more [FMC-DIO] with empty
eeprom. If it is the case it will ask to enter a S/N.

> ***Tip***: The S/N is labeled on the back side of the
[FMC-DIO] board and the index of PCIe slots on the bus "normally" respect
the physical plug order from top to bottom on the motherboard.

If for instance you have one with the label `7S-FMCDIO5ch-v1.0-S3-058` and
you can't read the label on the second one, we suggest you to fill the input as below:

	Found 2 FMC board(s) with empty EEPROM
	Enter S/N for board fmc-0500 in the format xx-XXX (or 0) : 03-058
	...
	Enter S/N for board fmc-0b00 in the format xx-XXX (or 0) : 0


### Manual

If you prefer to perform these steps manually you can try the
following procedure:

~~~~~{.sh}
##First you need to go in the fru-generator folder
>:$ cd spec-sw/fmc-bus/tools/

## And compile the library
>:$ make

## Then, find out on which bus id you have the boards
>:$ ls /sys/bus/fmc/devices/
fmc-0500  fmc-0b00

### Enter root mode in the terminal
>:$ sudo bash

## Write the eeprom for fmc-0500: the first/the top one fmc board.
>:# FRU_VENDOR="CERN" FRU_NAME="FmcDio5cha" FRU_PART="EDA-02408-V2-0" \
./fru-generator -s  7S-DIO-v2-S03-058 > /sys/bus/fmc/devices/fmc-0500/eeprom

## Write the eeprom for fmc-0b00: the 2nd/the bottom one fmc board.
>:# FRU_VENDOR="CERN" FRU_NAME="FmcDio5cha" FRU_PART="EDA-02408-V2-0" \
./fru-generator -s 7S-DIO-v2-S00-000 > /sys/bus/fmc/devices/fmc-0b00/eeprom

## Finally, Go back to user and starting-kit root directory
>:# exit
>:$ cd -
~~~~~~~~~~

Don't forget to modify the bus id: `fmc-XXXX` and the S/N
`7S-DIO-v2-Sxx-XXX` with your specific devices setup.

### Reload the kernel driver

Once you have correctly written (Auto or Manual) the EEPROM of the [FMC-DIO], you need
to reload the kernel driver by doing this

~~~~{.sh}
>:$ sudo rmmod spec
>:$ sudo modprobe spec
~~~~~~~~~~

you should now obtain something like this on `dmesg`:

~~~~~{.sh}
...
[269290.520027] spec 0000:05:00.0: mezzanine 0
[269290.520035]       Manufacturer: CERN
[269290.520038]       Product name: FmcDio5cha
...
[269291.086221] spec 0000:0b:00.0: mezzanine 0
[269291.086228]       Manufacturer: CERN
[269291.086230]       Product name: FmcDio5cha
...
~~~~~~~~



Setting Master & Slave using White Rabbit Core
=============================================

A "[SPEC+FMCDIO]" board is considered as a slave node, however in a standard
 White Rabbit network the synchronization is done using a master such
 as a White Rabbit Switch [WRS].

In order to enable the synchronization capabilities we need to set up one board in master mode
(faking to be a [WRS]) and the other one in slave mode (default mode).  This is done through the
shell of White Rabbit Core that allows this functionality among others.


Connect to the UARTs
-----------------------

To configure the [SPEC] board in master/slave mode you need to access to the White Rabbit Core (WRC) through the UART (virtual or physical).

### Physical UART

To access to the physical, you first need to connect the mini-USB of the [SPEC] to your PC. Then, you need to run
 a terminal emulator[^sudomc] (such as minicom) on the device created while plug-in the USB cable.

~~~~~{.sh}
>:$ sudo minicom --device=/dev/ttyUSB0 -b 115200
>:$ sudo minicom --device=/dev/ttyUSB1 -b 115200
~~~~~~~~~~

> ***Notes:*** ttyUSB0 might not correspond to your first board, and ttyUSB1 to the second one.
This depends on how you have plugged the USB cable to your USB of your machine.

### Virtual UART

An easier way to perform this operation is to access to the virtual
UART of the [SPEC] board by using the spec-vuart tool.
You first need to know the `bus_id` of your board

> ***Notes:*** The VUART can not work if the USB is already connected as the Physical UART
has the priority.

~~~~~{.sh}
>:$ lspci | grep CERN
05:00.0 Non-VGA unclassified device: CERN/ECP/EDU Device 018d (rev 03)
0b:00.0 Non-VGA unclassified device: CERN/ECP/EDU Device 018d (rev 03)
~~~~~~~~~~

 This means that your first board (wr0) is at **05**:00.0 and the second one (wr1) is at **0b**:00.0
 Thus, for each board you can open a terminal and run the following command:

~~~~~{.sh}
>:$ sudo ./tools/spec-vuart -b 0x05	#For the first SPEC board
>:$ sudo ./tools/spec-vuart -b 0x0b	#For the second SPEC board
~~~~~~~~~~

> ***Warning:***  Be aware that if you try to remove the spec kernel modules with the virtual UART
activated, your PC will hang and thus you will need to reboot. You must close
 the virtual UART before doing this kind of operation.


[^sudomc]: On Ubuntu LTS, you need sudo permission to access to a
ttyUSB*X* device. It might be a good idea to add your user to the dialout group to
obtain appropriate permisssion on the device: `sudo usermod -a -G dialout $USER`


The White Rabbit Core Shell
----------------------------

Once you are in the UART you should obtain the White Rabit Core console (`wrc#`).
A complete reference of the shell commands is included in the [wrpc.pdf] manual or
you can read them in the [Wiki](http://www.ohwr.org/projects/wr-cores/wiki/Wrpc_shell)

The most useful commands are repeated here for your convenience

* `gui`
* `mode master`
* `mode slave`
* `ptp stop`
* `ptp start`

For the tutorial we will use the following names:

* `wrc1#` for `wrc#` console of the main board (wr0/busID=0x0500)
* `wrc2#` for `wrc#` console of the second board  (wr1/busID=0x0b00)


Configure in slave & master mode
--------------------------------

Now we can perform the following operation:

* Check if you have the correct SFP and its corresponding value[^sfpdetectbug]

~~~~~{.sh}
wrc1# sfp detect
AXGE-3454-0531 #purple
wrc1# sfp match
SFP matched, dTx=46407, dRx=167843, alpha=-73622176
~~~~~~~

~~~~~{.sh}
wrc2# sfp detect
AXGE-1254-0531 #blue
wrc1# sfp match
SFP matched, dTx=46407, dRx=167843, alpha=73622176
~~~~~~~

[^sfpdetectbug]: If you obtain an error such as **Could not match to DB** you
might add your own SFP parameters. List your actual sfp database with `sfp show`
and check the [Calibration Section](#calibration) that explain how to add SFPs parameters.

* Setup one board in master (the one with the purple SFP)

~~~~~{.sh}
wrc1# mode master
SPLL_Init: running as Free-running Master, 1 ref channels, 2 out channels
Locking PLL...
~~~~~~~

* And let the other one in slave mode (since slave mode is the default
mode, this should not be necessary but you might have set it before in master mode)

~~~~~{.sh}
wrc2# mode slave
slave
~~~~~~~

### Start the synchronization with PTP daemon


You should run the PTP daemon on both cards

~~~~~{.sh}
wrc1# ptp start
wrc2# ptp start
~~~~~~~

And you should obtain the following message on the slave board:

	SPLL_Init: running as Slave, 1 ref channels, 2 out channels
	Enabling ptracker channel: 0
	Enabling ptracker channel: 0
	servo:pre: -1885143040/2ps
	servo:Deltas: Master: Tx=46407ps, Tx=175043ps, Slave: tx=46407ps, Rx=167843ps.
	Adjust: counter = seconds [+0]
	Adjust: counter = nanoseconds [+343914800]
	servo:busy

Finally you can run the WRC shell in `gui` mode to obtain more information

~~~~~{.sh}
wrc2# gui
~~~~~~~~~~

~~~~~{.sh}
WR PTP Core Sync Monitor v 1.0
Esc = exit

TAI Time:                  Thu, Jan 1, 1970, 0:30:15

wru1: Link up   (RX: 766, TX: 340), mode: WR Slave   Locked  Calibrated

Synchronization status:

Servo state:               TRACK_PHASE
Phase tracking:            ON
Synchronization source:    wru1
Aux clock status:

Timing parameters:

Round-trip time (mu):      698557 ps
Master-slave delay:        345660 ps
Master PHY delays:         TX: 46407 ps, RX: 175043 ps
Slave PHY delays:          TX: 46407 ps, RX: 167843 ps
Total link asymmetry:      7237 ps
Cable rtt delay:           262857 ps
Clock offset:              2 ps
Phase setpoint:            7268 ps
Skew:                      2 ps
Manual phase adjustment:   0 ps
Update counter:            117
--
~~~~~~~~~~~~~~~~~~

You can see that the slave node is locked, calibrated and that the phase tracking is enabled.


> ***Notes:*** We also recommand you to set up an init script if you do not want to repeat these operations at each
reboot. You can look at the [wrpc.pdf] for more information or use the `init show` command to check the
one you have running.




Bring up the network interface
-------------------------------

Once the drivers are loaded and the PTP has started you might need to bring the new network interfaces up.
In order to do this you should just execute:

~~~~{.sh}
>:$ sudo ifconfig wr0
>:$ sudo ifconfig wr1
~~~~~~~~~~~~

And then check if they are mounted by doing

~~~~{.sh}
>:$ sudo ifconfig | grep wr
wr0       Link encap:Ethernet  HWaddr 08:00:30:0d:e8:6b
wr1       Link encap:Ethernet  HWaddr 08:00:30:0d:e4:cd
~~~~~~~~~~~~


> ***Notes:*** On the distributions that use gnome desktop (i.e, Ubuntu), the interfaces are automatically mounted by *gnome
network manager*, so you might not need to perform the last step.


Some Illustrative Experiments
==================

To perform the experiments we recommend you to plug the LEMO cable to your
oscilloscope by using the LEMO-BNC adapter provided in your package.

It is also recommended to follow the order of the tutorial since the
concepts that are intended to be described and tested are explained in
the same order as the experiments.

> **Notes:** Before each experiment you should make sure that you set
up your boards in *master/slave* mode and that the PTP daemons are running on both.
 You cand find how to do it in subsection [4.3](#configure-in-slave-master-mode).
A basic explanation is included in [Frequently Added Questions](#frequently-added-questions) section (first one).


Playing with the DIO channels
----------------------------------------------------------------------------------------

The first step to perform is to check if the [FMCDIO] channels are accessible using the installed driver.
In the `tools` subdirectory you will you find the `wr-dio-cmd` program,
that let you quickly test the I/O features of the [FMCDIO] boards

This is the general syntax of the command[^sudo]: `wr-dio-cmd <ifname> <cmd> [<arg> ...]`

The main commands correspond to: `mode, pulse & stamp`, and you can
find an extended description of
 them in the [spec-sw.pdf:6.7][spec-sw.pdf] document.


[^sudo]: As the application use `ioctl` you should call with root privilege by using sudo.

### Setting the mode

The first thing is to set the correct mode for each channel; you can use a command that set the
mode for a specific channel[^dionum] or for all of them at the same
time.

	mode <channel> <mode> [<channel> <mode> ...]
	mode <modech0><modech1><modech2><modech3><modech4>

The available modes are:

* `I/i`: Input
* `0/1`: Output, steady state fixed at `0` low or `1` high.
* `D/d`: DIO core output
* `P/p` (Channel 0): Output PPS (Pulse Per Second) Generator from PTP core.
* `C/c` (Channel 4): Clock Input to PTP core: allow to input a high frequency signal without interruption throwing

>> ***Note:*** If the Letter is capitalized the I/O channel is connected to 50-ohm termination resistor, otherwise it is not. i.e, The `I` is input with 50-ohm termination resistor and `i` is without it.

> ***Note:*** The first channel (channel 0) has been modified and now support only the P/p as output
mode. You will not be able to use D,d,1,0 modes.


For example you can set it up like this

~~~~~{.sh}
## Configure channel 0 as input with termination, 1 as input, 4 as fixed low
>:$ sudo ./tools/wr-dio-cmd wr0 mode Ii--0
~~~~~~~~~~~~~~~~



### Default Mode

After reprogramming/rebooting the FGPA, the channels are set up by default as:

#. Channel 0 (connector #1) output a PPS signal: `p`
#. Channel 1 (connector #2) low state: `0`.
#. Channel 2 (connector #3) same as ch1: low state: `0`
#. Channel 3 (connector #4) is configured as input with termination impedance: `i`.
#. Channel 4 (connector #5) is in Clock Mode with termination: `C`.

To reset to the default mode you can reset/reprogram the FPGA or set it back with:

~~~~~{.sh}
## Revert back to default mode
>:$ sudo ./tools/wr-dio-cmd wr0 mode p00ic.
~~~~~~~~~~~~~~~~


### Generating pulse

The command is used in the following formats:

	pulse <channel> <duration> <when> [<period> <count>]

You should plug the LEMO cable in the connector #5 (channel 4), and connect the BNC adapter to your oscilloscope.

![Connecting LEMO on connector #5 (channel 4)](img/ssk_lemo.jpg)

Finally you need to active a trigger pulse in your oscilloscope. Then you can try the following commands:

~~~~~{.sh}
## Set channel 4 as Input
>:$ sudo ./tools/wr-dio-cmd wr0 mode 4 D

## Pulse channel 4 for 0.1 seconds now
>:$ sudo ./tools/wr-dio-cmd wr0 pulse 4 .1 now

## Pulse for 10 microseconds in the middle of the next second
>:$ sudo ./tools/wr-dio-cmd wr0 pulse 4 .00001 +1.5

## Pulse for 1ms at 17:00 today
## Set the datetime of the next event (60 seconds from now)
## NOTE: this only work if the date is correctly set in the master SPEC,
>:$ sudo ./tools/wr-dio-cmd wr0 pulse 4 .001 $(date +%s --date 17:00)
~~~~~~~~

> ***Note:*** please note that the `pulse` command activates the [FMCDIO] mode (without changing the termination)


### Time stamping output

Once you have generated the pulse you can retrieve its timestamp by executing:

~~~~~{.sh}
>:$ sudo ./tools/wr-dio-cmd wr0 stamp 4
ch 4,       378.788588536
ch 4,       381.268701864
ch 4,       387.284885816
ch 4,       469.500000000
ch 4,       500.500000000
ch 4,       504.500000000
ch 4,       530.500000000
ch 4,       534.500000000
ch 4,       542.500000000
~~~~~~~~

> ***Notes:*** If you see other timestampings messages coming from channel 2 this mean you don't
have disabled the PPS mode. Check [Disabling PPS](#disabling-pps-on-channel-2) section.


### Time stamping input & output.

The idea here, is to create a pulse from one [FMCDIO] and to timestamp it as local
and remote input. Then we can compare both time stamps and measure the
interval between generation and reception of the pulse.

The configuration is done as indicated in the figure below:

![Time-stamping configuration](img/ssk_playdio.png)

~~~~~{.sh}
## Configure #2 & #5 (ch1 & ch4) as ouput and #3 (ch2) as input with termination on wr0.
>:$ sudo ./tools/wr-dio-cmd wr0 mode -DI-D
## Configure #4 (ch3) as input on wr1
>:$ sudo ./tools/wr-dio-cmd wr1 mode 3 I

## Then flush the previous timestamp
>:$ sudo ./tools/wr-dio-cmd wr0 stamp &> /dev/null
>:$ sudo ./tools/wr-dio-cmd wr1 stamp &> /dev/null


## Schedule the pulse to the next common event on two outputs
>:$ sudo ./tools/wr-dio-cmd wr0 pulse 1 .00001 +2
>:$ sudo ./tools/wr-dio-cmd wr0 pulse 4 .00001 +2

## Then (after 60s), you should run stamp on the wr0
>:$ sudo ./tools/wr-dio-cmd wr0 stamp
ch 1,      2267.500000000
ch 2,      2267.500000008
ch 4,      2270.500000000
>:$ sudo ./tools/wr-dio-cmd wr1 stamp
ch 3,      2270.500000008
~~~~~~~~~~~~~~~~~


We observe that locally (ouput=>ch1, input=>ch2) and remotely (output=>ch4, input=>ch2),
the timestamp between sending a pulse and receiving is of 8 nanoseconds[^8ns] for both.
This is a very simple example to show how the two [SPEC]s are synchronized.


[^dionum]: The connector on the [FMC-DIO] panel are enumerated from #1 to #5 and correspond to the channel 0 to 4 on the `wr-dio-cmd`.
[^8ns]: The DIO core work at a frequency of 125Mhz, therefore the minimum difference between two timestamped events is of 8ns.
However with the next experiment you will find out that the synchronization is better.

Synchronization of PPS
----------------------------------------------------------------------------------------

This corresponds to the experiment from [spec-sw.pdf:6.8][spec-sw.pdf] document.
Here we want to synchronize the PPS of the master and slave boards using WR protocol on a fiber.

### Setup

In order to perform a more illustrative experiment, this example should
be done with two [SPEC+FMCDIO] boards separated by a 5km fiber
on two different PCs.
However to simplify the experiments we are using 2m fiber cable and the two boards connected to the same PC.
The first connector (ch 0) of each board output the PPS and is connected to the oscilloscope using LEMO cable & BNC adapter.

![Setup for the PPS synchronization](img/ssk_pps-setup.png)


### Run PPS command

Once the synchronization is enabled, you can see the PPS on channel 0
of both boards.

> ***Notes:*** To generate a PPS signal the PLL has to be lock. See "I can't generate a PPS signal" on [Frequently Added Questions](#frequently-added-questions) .

If you don't see this pulse, you might want to force the pulse mode:

~~~~~~{.sh}
## run pps on channel 0 of the "first" card
>:$ sudo ./tools/wr-dio-cmd wr0 mode 0 p

## run pps on channel 0 of the "second" card
>:$ sudo ./tools/wr-dio-cmd wr1 mode 0 p
~~~~~~~~~~


### Result

The following two figures show the *pulse-per-second* signals
retrieved from two different *simple-DIO* cards.

![Synchronization of the two PPS (5ns/div)](img/ssk_pps-res.jpg)

To avoid the unstable signal after the pps go high you can
add a 50-ohm resistor termination if the oscilloscope allows it.


The Network Interface Card (NIC) Synchronization
---------------------------------------

### Introduction

The *wr-nic* driver registers a Linux network interface card for
each [SPEC] device that it drives.  The cards are called `wr%d` (i.e.,
*wr0*, *wr1*, ...).

The [SPEC] can carry normal data traffic in
addition to the PTP frames of *White Rabbit*, that remain
invisible to the host computer.

> ***Notes:*** If a user wants to use the WR-NIC on a real network to transfer
data he should assign an Interface address. However the *White Rabbit* synchronization happens at Ethernet
layer therefore no IP address is needed. Moreover if you want to communicate two WR-NIC boards that are on the same
computer you should not use an IP protocol because the data will never
be put on the fiber since the computer knows
that the two IPs are on the same machine.

### Timestamp Mechanism

The [SPEC] Ethernet interface
supports hardware timestamping for user frames through the
standard Linux mechanisms.  Time stamps are currently reported with
a resolution of 8ns only (*White Rabbit* does much better, but the code
to illustrate this is not developed for this simple demo, yet).

Unfortunately the Linux mechanisms are not trivial: the application must
enable timestamping on both the hardware interface and the specific
socket it is using, and it must issue several *ioctl* and *setsockopt*
commands. Moreover, timestamps are returned to user space using the
*recvmsg* system call, which is more difficult to deal with than the
normal *send* or *recv* commands.

To simplify use of timestamps for Ethernet frames,
this package includes the `stamp-frame` program in the
`tools` directory.  The program is a minimal implementation of the basic
time-synchronization protocols (like NTP and PTP), but excluding the
synchronization itself. The idea is sending a frame from one host to
another, and receiving a second frame back; the departure and arrival
times are recorded and collected at a single place, so they can
be reported to
the user.

### Synchronization And Timestamping

The `stamp-frame` example supports two modes of operations. In
*listen* mode, it binds to an Ethernet interface and listens forever:
it waits for the forward frames and replies to them; in normal mode it
sends the forward frame and reports data as soon as it gets a reply.

~~~~~{.sh}
## On a terminal run
>:$ sudo ./tools/stamp-frame wr0 listen
tools/stamp-frame: Using interface wr0, with all timestamp options active

## On another terminal (maybe on another host) run
>:$ sudo ./tools/stamp-frame wr1
tools/stamp-frame: Using interface wr1, with all timestamp options active
timestamp    T1:      1891.948736656
timestamp    T2:      1892.038390176
timestamp    T3:      1892.048625152
timestamp    T4:      1891.959080320
round trip time:         0.000108688
forward    time:         0.089653520
backward   time:        -0.089544832
~~~~~~~~~~~~

This is done four times: departure and arrival of the forward frame, followed
by departure and arrival of the backward frame. Thus,
time stamps T1 and T4 are collected at the
original sender (here: *wr1*) while T2 and T3 are collected at the
remote host (here: *wr0*).  The times above are all consistent
because the two [SPEC] cards are synchronized with *White Rabbit*. The
reported forward and backward times match the fact that we used
a 10km fiber to connect the two cards; the difference between them
is due to the different speed of light in the two directions,
because the two SFP transceivers we plugged to use different wave lengths.

The following example shows the output for two forcibly-unsynchronized
cards.  The difference between the two clocks is clearly a
few seconds; the
*round trip time* is correct nonetheless, because it is a difference of
differences:

~~~~~{.sh}
   timestamp    T1:        13.225249168
   timestamp    T2:         9.130237600
   timestamp    T3:         9.140438816
   timestamp    T4:        13.235559016
   round trip time:         0.000108632
   forward    time:        -5.904988432
   backward   time:         4.095120200
~~~~~~~~~~~~

The code in `stamp-frame`
is designed to be simple to be reused, but there is one non-obvious detail
that is worth describing here.  Whereas the receive timestamp is
returned to user-space together with the frame it refers to, the
transmit timestamp is only known after the relevant frame left the
computer.  For this reason, in order
to communicate the TX timestamp of a frame to your peer, you will need to
send another message which carries the departure time of the previous
frame.  This further message is usually called *follow-up*,
and `stamp-frame` respects this tradition.




Transmitting external frequency
-----------------------------------

This experiment merges the concepts seen before with the [FMCDIO] channels and the NIC synchronization.

### Summary

A typical application for *White Rabbit* (or any time
synchronization system) is being able to generate output signals at the
same time in different output boards; (in the framework of a
distributed instrumentation facility); another typical application is
time stamping input events.

Thus in these experiments we are going to transmit an external frequency in
the 100Hz range using the starting kit:

* The user supplies a ~100Hz square wave on the channel 0 of the master card.
* The **dio-ruler** on master host reads the UTC time of the rising edge of the external pulse upon IRQ.
* Then the **dio-ruler** forwards it (as an **ioctl** structure) to its local and remote selected outputs.
* At the slave workstation the **dio-agent** waits to receive
timestamped **ioctl** order and passes it to the *DIO* driver.
* The DIO will generate a *1ms* pulse at the programmed time.
* On the screen of the oscilloscope we should see a constant time offset between the two pulses.

The figure below illustrates how the connection must be done between the different components:

![Setup to transmit a 100Hz signal](img/ssk_100Hz.png)


### How does it work?


The example is made up of two programs: `wr-dio-agent` and `wr-dio-ruler`: the ruler rules and the agent acts only.

These programs transmit using raw Ethernet frames to force the optical fiber transmission when two boards are on the same machine.
It also transmits using broadcast addresses to avoid complication on
selecting to who it should be transmitted.
The simplification adopted above, however, most likely prevents the programs from working within a more complex
network topology.

> ***Notes:*** By using an IP protocol (i.e, UDP) the data will never enter the optical fiber between the two boards
because the OS see that the destination is local and therefore routes them directly to the receiver without using the sender.

The *agent* is a "dumb" agent in charge of forwarding *WR* packet to the *DIO* core.

The *ruler* waits for timestamps to appear on a specific input channel;
when a positive-going edge occurs the *ruler* notifies it or replicates the edge on one or more outputs.
Each output can be local or remote, and can use a different delay from the input pulse.

For further details (deeper studies) the user is recommended to read the [spec-sw.pdf:6.8][spec-sw.pdf] document
and to also take a look at the source code of `wr-dio-agent` and `wr-dio-ruler`


> ***Notes:***  All pulses generation are driven by
host software, after a hardware interrupt reports the input event.
For this reason, you will not be able to reliably replicate pulses with
delays smaller than a few hundred microseconds, depending on the
processing power of your computer and the load introduced by other
processes.  For remote connections, you must also count the overhead
of network communication as well as transmission delays over
the fiber (a 10km fiber introduces a
delay of 50 microseconds).

[^loopback]:

### The setup

Then we should run the "dumb" agent on the slave board in charge of forwarding *ioctl* packet to the *DIO* core:

~~~~~~{.sh}
>:$ sudo ./tool/wr-dio-agent wr1 &
~~~~~~~~~~

Then you need to connect the output of the generator[^fake100hz] to channel 1 of the master [SPEC+FMCDIO] board.
The generated waveform should be a 0-5V pulse at ~100Hz (5ms at 5V then 5ms at 0V)


[^fake100hz]: If you lack a wave form generator, you can make an output pulse with
`wr-dio-cmd <if> mode <ch> P` or other means and use it as a trigger. Check the [Two PCs example](#two-pcs-example).



You should setup the channel 0 as input and check if the 100Hz signal is correctly timestamped:

~~~~~~{.sh}
## Set channel 0 of master board as Input and disable PPS signal (if the FPGA is reflashed).
>:$ sudo ./tools/wr-dio-cmd  wr0 mode I-0--

## Retrieved the time stamped value
>:$ sudo ./tools/wr-dio-cmd wr0 stamp
ch 0,      3573.281851462
ch 0,      3573.291851460
ch 0,      3573.301851460
ch 0,      3573.311851456
ch 0,      3573.321851454
ch 0,      3573.331851458
ch 0,      3573.341851454
...
~~~~~~~~~~~~~~~~~~~~~~~~~~

Here we can see that we have correctly timestamped each 10ms (+/- 10ns)[^10ns].

[^10ns]: The 10 nanoseconds jitter is due to our waveform generators used in the experiments.
This should disappear by using a good quality waveform generator.


Once you know that your input is correct, you need to capture each
event on channel 0 and replicate them with a delay of 1ms
on both the local (channel 3) and the remote card (channel 1)

~~~~~~{.sh}
## Set local3 as Output
>:$ sudo ./tools/wr-dio-cmd wr0 mode 3 D

## Set remote1 as Output (and disable 2)
>:$ sudo ./tools/wr-dio-cmd wr1 mode -D0--

## Creating the ruler to forward from input 0 to local3 and remote1
>:$ sudo ./tools/wr-dio-ruler wr0 IN0 L3+0.001 R1+0.001
wr-dio-ruler: configured for  local channel 3, delay 0.001000000
wr-dio-ruler: configured for remote channel 1, delay 0.001000000
~~~~~~~~~~~~~

### Result

To check if the experiment works well you can compare the timestamp of the two outputs

~~~~~~~~{.sh}
## Checking the timestamp on local ouput
>:$ sudo ./tools/wr-dio-cmd wr0 stamp 3
ch 3,      5340.004864960
ch 3,      5340.014864960
ch 3,      5340.024864968
ch 3,      5340.034864964
...

## Checking the timestamp on remote ouput
>:$ sudo ./tools/wr-dio-cmd wr1 stamp 1
ch 1,      5340.004864962
ch 1,      5340.014864960
ch 1,      5340.024864968
ch 1,      5340.034864960
...
~~~~~~~~~~~~~~~

Or compare the signals[^3cables] on the oscilloscope as in the next figure:

![Result with a 100Hz train](img/ssk_100Hz-train.jpg)

[^3cables]: To do this you need to connect from the waveform generator to the input with a LEMO cable that is not provided.

> ***Notes:*** This code was developed as a demo sample therefore: the delays should be no
more than the interval between input pulses, because the tools
reprograms all output triggers at each input event. The output pulse is fixed at 1ms


### Two PCs example

This experiment is taken from the spec-sw document;
we verified the tools and we think this is a good explanation of how to use them.

It shows how to use the *ruler* and *agent* on
two hosts, called `spusa` and `tornado`. It is pretty the same thing as the previous
experiment only that the board are now both named `wr0`.

![Transmit PPS between two PCs](img/ssk_txpps.png)

The input events on `spusa` are replicated to one local channel and two remote channels,
with a delay of 1ms. The input events in this case are from a *pulse-per-second* signal from
channel 0.

~~~~~{.sh}

   >spusa:# sudo ./tools/wr-dio-cmd wr0 mode 0 P

   >tornado:# sudo ./tools/wr-dio-agent wr0 &

   >spusa:# ./tools/wr-dio-ruler wr0 IN4 L3+.001 R4+.001 R2+.001
   wr-dio-ruler: configured for  local channel 3, delay 0.001000000
   wr-dio-ruler: configured for remote channel 4, delay 0.001000000
   wr-dio-ruler: configured for remote channel 2, delay 0.001000000

   [... wait a few seconds ...]

   >spusa:# ./tools/wr-dio-cmd wr0 stamp 3
   ch 3,       385.001000000
   ch 3,       386.001000000
   ch 3,       387.001000000
   ch 3,       388.001000000
   >tornado:# ./tools/wr-dio-cmd wr0 stamp 2
   ch 2,       385.001000000
   ch 2,       386.001000000
   ch 2,       387.001000000
   ch 2,       388.001000000
   >tornado:# ./tools/wr-dio-cmd wr0 stamp 4
   ch 4,       385.001000000
   ch 4,       386.001000000
   ch 4,       387.001000000
   ch 4,       388.001000000
~~~~~~~~~~~~

Advanced used
==================

<!--TODO: Heat & Auto-calibration chapter -->


Connect to the WRS
----------------------

If you have a White Rabbit Switch ([WRS]), you should try to see
the synchronization between the [WRS] and the two [SPEC+FMCDIO] boards.

First you need to set up the two [SPEC+FMCDIO]s as `slave` by changing the
mode in the UART of each board.

~~~~~{.sh}
wrc# ptp stop
wrc# mode slave
slave
wrc# ptp start
~~~~~~~

Then you should run the [WRS] in master or grandmaster mode as explained
 in [wr-switch-guide.pdf].

Finally you need to connect the slave SFPs (blue) to the [SPEC]s and the
 master SFP (violet) to the switch.

The result of this experiment is illustrated by the next figure where
you can see the 3 PPS signals: yellow for [WRS], blue and violet for
[SPEC+FMCDIO].

![WRS PPS (yellow) vs Starting Kit PPS (blue & violet)](img/ssk_wrspps.jpg)

The distance between each PPS might be reduced by the improving the
calibration which is the subject of the next section.


Calibration
-----------------------

When you compare the PPS of two [SPEC+FMCDIO] board you will find that they
are almost always synchronized without the need to calibrate the
gateware. However if you compare these PPS with the one of the switch
and you do not obtain a delay similar as the one in the WRS figure above
you might need to calibrate the [WRS] and the [SPEC+FMCDIO].

The value that you need to use can be found at the wiki page:
<http://www.ohwr.org/projects/white-rabbit/wiki/Calibration>

If you use a custom gateware or SFPs that are not listed in the wiki page
you should run yourself the calibration in order to obtain the correct values
for your setup.

> **Warning**: Cable length/type can modify around ~5ns/meter the delay
between the moment when the PPS was out of the board and when it was
captured by the oscilloscope.

> **Notes**: Please check that you are using the values that correspond
to your gateware release. When this document was generated we refer as
wr-nic-v2.0 (starting kit). You can also check using the `ver` command
if the corresponding embedded LM32 corresponds to your gateware version
(i.e, wrpc-v2.1-for-wrnic lm32 is embedded in wr-nic-v2.0). 

~~~~~{.sh}
wrc# ver                                                                        
WR Core build: wrpc-v2.1-for-wrnic
...

## List your actual sfp database
wrc# sfp show

## Erase the actual database if it was corrupted
wrc# sfp erase

## Add the parameters for the SFP provided in the starting kit
wrc# sfp add AXGE-1254-0531 121751 95827  64481496
wrc# sfp add AXGE-3454-0531 121751 95827 -66584558
~~~~~~~~~~~

Do not forget to also add the proper calibration values to your other
WR equipments (i.e, [WRS]) as explained in the wiki page.

You might also check the [wrpc.pdf] for extended explanations on how to run
yourself the calibration procedure.

> **Notes**: Please check that the master board has a valid t42p written
in its EEPROM. This value must be obtained by first connecting this
board in slave mode to a working WR master. For more information read the
[wrpc-v2.1.pdf] page 7.
 

SPEC+DIO as grandmaster
------------------------

In basic configuration your Master [SPEC] can use its internal
free-running oscillator as a time reference. However, you can also
discipline your Master [SPEC] with external 10 MHz and 1-PPS signal by
connecting them to the appropriate LEMO connectors of [FMC-DIO] board:

![Grandmaster setup](img/ssk_grandmaster.png)

The requirement for the applied signals are:

* ~[1.5V to 3V][^aroundval] without termination
* ~[2.5V to 4V] with 50Ohm termination.
* PPS pulse width must be at least one 10MHz period (>100ns).

[^aroundval]: These values are given on an indicative basis because they depend on the source and
connection you are using.

### Using a GPS

The common way to use grandmaster mode is with a GPS source where the
input 10MHz signal can be square or sinusoidal.

When the GrandMaster is a [SPEC+FMCDIO] `PPS_in` pulse serves to *mark* which rising edge of the 10MHz signal is considered the first one valid for a new second: the `PPS_out` will be fixed on the `10MHz_in` and not the `PPS_in`. `PPS_in` should therefore arrives at least 8ns before the `10MHz_in` clock.

As *1m is around ~5ns of delay*, you could use a cable with 4m to connect to `PPS_in` and a cable with 1m to connect to `10MHz`.

The following figure shows what kind of signals need to be provided to plug to the GM [SPEC+FMCDIO], and with which fixed delay the `PPS_out` is produced (~8ns).

![Grandmaster signals from GPS with different cables length (PPS in=>Yellow, 10MHz CMOS=> Blue, 10MHz Sin => Green, PPS out=>)](img/ssk_gm-4m1m.jpg)

### Using the fine-delay.

If you have a [FMC fine-delay] board you can generate a precise PPS/10MHz by
executing the following commands (using release v2014.04):

~~~~~~{.sh}
>:$ sudo tools/fmc-fdelay-pulse -o 1 -r 1s-10n -T 1s       ## PPS on #1  (10ns before 10MHz).
>:$ sudo tools/fmc-fdelay-pulse -o 2 -1                    ## 10MhZ on #2
~~~~~~~~~~~~~


![Grandmaster signals from a FMC fine-delay (PPS=>Yellow, 10MHz=> Blue)](img/ssk_gm_fdelay.jpg)

### Locking GM PLL to external 10MHz/PPS

Then in the wrc console of the [SPEC+FMCDIO] just execute the following commands:

~~~~~~{.sh}
wrc# ptp stop
wrc# mode gm
wrc# ptp start
~~~~~~~~~~~~~

And you should obtain:

~~~~~~{.sh}
PLL locking .................. LOCKED
~~~~~~~~~~~~~

For more information about grandmaster mode you can take a look at: [wr_external_reference.pdf].
This document has been written for the WR switch, but timing/accuracy/stability requirements are
similar for the [SPEC+FMCDIO].

By default the [SPEC+FMCDIO] should be configured to be run in GM setup, but if you are not sure please
 reset the default value. See [Default Mode](#default-mode) Section. If you
want to run the [SPEC+FMCDIO] in standalone mode as grandmaster we also
recommand you to modify the startup script as explained in the [section below](#run-in-standalone)


Run in standalone
----------------------------

> **Note**: You need to have the Xilinx JTAG USB platform cable or similar
 to perform the next step.

You can also run GM in a full standalone mode in order to transmit WR clock without the need of a PC.

First you need to flash the SPEC with the latest bitstream you can find the package:
<http://www.sevensols.com/dl/wr-nic/bin/latest_stable.tar.gz>

To do so you need to generate a MSC file as follows:

	Create PROM Files > SPI Flash > Single FPGA > 32M > MCS

And then, when asking for SPI/BPI, just add the `.msc` file	and select
the corresponding SPI flash memory: `M25P32`.

Then:

* The setup of the cable is exactly the same as above, and you do not need to set up the mode because it is
correctly configured for GM by default at power up.
* Then you need to run grandmaster mode from wrc console, as above.
* (Optional) Finally, if you need to keep this configuration at power up, you can write a small script
on the EEPROM to boot in grandmaster mode.

### EEPROM boot script

~~~~~~{.sh}
wrc# init erase
wrc# init add ptp stop
wrc# init add sfp detect
wrc# init add sfp match
wrc# init add mode gm
wrc# init add ptp start
~~~~~~

Please refer to *Writing EEPROM and calibration* Section of the [wrpc.pdf] document.




Manage standalone node using etherbone
----------------------------------------

You can also use SPEC card in standalone mode as we have seen before
but... how can you configure spec card if you do not use drivers?

In wr-nic project, Etherbone core has been added to the design and it allows
a direct access to the memory map[^specmem] (wishbone registers) using
UDP/TCP packets.

In order to connect you to the [SPEC+FMCDIO] in standlone you must
first configure it with a valid IP. This can be done automatically (complex) using bootp protocol,
or manually (easy) through the WRC mini-USB UART.

[^specmem]: If you are still connected through the PCIe you can also use the tools
`spec-sw/tools/specmem` to directly read/write on the WB registers as you will
do with the etherbone tool.


### Set IP using BootP

This is the more complex way to do it because you need to create a BootP server
on your LAN, but then the IP of each standalone board can be automatically
asigned.

On Ubuntu the `dnsmasq` package can be used as BootP deamon. The configuration
file should be something similar as:

~~~~~~~{.sh}
## Specified interfaces (10.10.10.1 is your IP)
interface=eth1
## DHCP setup
dhcp-range=10.10.10.1,10.10.1.15,12h
## BootP setup
dhcp-boot=pxe.0,tfpservname,10.10.10.1
## Set a specific IP for a MAC address (optional)
dhcp-host=08:00:30:0d:f8:23,targetsysname,10.10.10.10
~~~~~~~~~~~~

### Set IP through WRC UART

An easier way to set the IP of your standalone board is through the WRC Shell by
connecting the mini-USB UART to your PC.

Then you should just write in the terminal:

~~~~~~~{.sh}
wrc# ip set 10.10.10.10
~~~~~~~~~~~~

If you want this IP to be kept after a power cycle you must add this command in the
init EEPROM script as explained [previously](#eeprom-boot-script).


### Access to the wishbone device

At this step, you need to connect the standalone device with your PC. If you
do not have any SFP interface on your PC you can use an SFP-RJ45 adapter
to connect to it.

You need to check that your interface is on the same LAN as the standalone
[SPEC+FMCDIO] board or you should set it manually if it is not the case:

~~~~~~~{.sh}
>:$ sudo ifconfig eth1 10.10.10.1
~~~~~~~~~~~~

Finally you can try the etherbone library to connect to your standalone
[SPEC+FMCDIO] board. Please check that you have compiled the library for
your platform by doing `make && sudo make install` in the main folder or by going to
`etherbone/api` and perform a `make clean && make && sudo make install`.

To ease the communication using etherbone, we have added the `eb-mem.sh` tool in the `scripts/`
folder that access through the etherbone library to the device and perform
read/write operations.

Below we display a quick example on how to use it.

~~~~~~{.sh}
## It shows you Device memory map
>:$ scripts/eb-mem.sh --scan --ip 10.10.10.10

## Read a memory address (DIO I/O mode register <=> 62000 + 300 + 3C)
>:$ scripts/eb-mem.sh --read --ip 10.10.10.10 --address 0x6233C

## Write a memory address (DIO I/O mode register <=> 62000 + 300 + 3C)
## Force only ch2 (#3) into P mode (forbidden operation while using wr-dio-cmd)
>:$ scripts/eb-mem.sh --write --ip 10.10.10.10 --address 0x6233C --value 0x00A00

## Show help
>:$ scripts/eb-mem.sh --help
~~~~~~~~~~~~~

<!--TODO: Caloe -->

A library called CALoE (Conﬁguration Abstraction Layer over Etherbone)
can also be used to ease the task of accesing to various register through
etherbone.

You can find more information on the following page
<https://github.com/klyone/caloe/wiki>


Quick Start Guide For Developers
=================================


Introduction
--------------


This section has the purpose to give the developers a quick start guide to install
the tools and compile the different projects to obtain the binaries used previously.

If you need a more complete guide on how to create your own HDL for the [SPEC],
or you want to improve the structure of a project using the [OHWR] platform we recommend you
to look at the [spec-getting-started.pdf] guide.

The starting kit is based on various project:

[spec-sw]
:	The project that contains the software (application + driver) which you have already compiled above.
[spec-golden]
:	A simple gateware in order to access to the EEPROM of the [FMC] (WB-I2C)
[wr-nic]
:	gateware (FPGA HDL) that includes the NIC & DIO capabilities.
[wrpc-sw]
:	LM32 software in the white rabbit PTP core for the synchronization.


Tools
----------------

You must have installed the following tools:

* Xilinx ISE (>14.x)
* Git
* Build-essentials
* Autoconf/Automake
* Kernel source
* Lm32 cross compiler
* Hdl make
* Texinfo



Golden SPEC gateware
---------------------

> *Dep*: hdlmake, Xilinx ISE 14

This is a really simple gateware that allows the PC to read the FMC-EEPROM in
order to know what type of gateware it should load.
The gateware is a wishbone PCIe bridge connected to a WB I2C module.

To synthetize it, the user needs to follow the next steps:

~~~~~~{.sh}
## Checkout the code
svn checkout http://svn.ohwr.org/spec/trunk/hdl/golden@53 spec-golden

## Go to the main directory
cd spec-golden/syn/

## Synthetize using hdlmake
hdlmake --fetch
hdlmake
make
~~~~~~~~~~~


You will therefore obtain your golden firmware called as `spec-init.bin`


WRPC-SW (LM32 firmware)
-----------------------

> *Dep*: lm32 compiler

> ***Notes***: The steps in this section are not needed as we already provide the `wrc.ram` in the [wr-nic] repository.
However we provide here a simple resume of the steps required to compile the firmware specifically for the wr-nic.
You should also look at the [wrpc.pdf] to understand how to use it and
how to compile new firmware for other configurations.

You can download it from [wr-nic-v2.0.tar.gz](http://www.ohwr.org/projects/wr-nic/files) file or you can try to compile it following the instructions below:

You first need to install the **lm32** compiler as suggested in
[wrpc.pdf], and then you need to compile it using the specific configuration as bellow:

~~~~~~{.sh}
## Set up CROSS_COMPILE variable for this terminal
export CROSS_COMPILE="<your_path_to_lm32>/lm32/bin/lm32-elf-";

## Clone the repository
git clone git://ohwr.org/hdl-core-lib/wr-cores/wrpc-sw.git
cd wrpc-sw

## Checkout the stable release
git checkout -b wrpc-v2.1-for-wrnic wrpc-v2.1-for-wrnic

~~~~~~~~~~

And finally configure and compile it

~~~~~~{.sh}
## Configuring the project for SPEC
make wrnic_defconfig

## Compile
make
~~~~~~~~~~

You should obtain various files named wrc.bin, wrc.elf, wrc.vhd, wrc.ram

You can therefore use them to override the one in [wr-nic](#wr-nic-hdl-gateware) project.

~~~~~{.sh}
## Override the default embeded wrpc-sw
cp wrc.ram <wr_root_folder>/wr-nic/syn/spec
~~~~~~~~~~~


WR-NIC (HDL-gateware)
----------------------

> *Dep*: hdlmake, Xilinx ISE 14

This step shows us how to prepare the WR-NIC bitstream ([SPEC+FMCDIO]) with
the wrpc-sw (`wrc.ram` file) embedded inside.

~~~~~~{.bash}
## Checkout the code
git clone git://ohwr.org/white-rabbit/wr-nic.git
cd wr-nic
git checkout -b wr-nic-v2.0 wr-nic-v2.0

## Create and update the submodules
git submodule init
git submodule update

## Go to the main directory
cd wr-nic/syn/spec/

## Synthetize using hdlmake
hdlmake --make-ise --ise-proj
make
~~~~~~~~~~~

You should finally obtain the bitstream to import in your [FMC] driver folder.


Frequently Added Questions
===========================

Online & Updated
-----------------

The FAQ is updated on a regular basis on the ohwr.org website so
we strongly recommand you to visit the webpage:

* <http://www.ohwr.org/projects/wr-starting-kit/wiki/FAQ>

and you might also find information there:

* <http://www.ohwr.org/projects/spec/wiki/FAQ>
* <http://www.ohwr.org/projects/wr-cores/wiki/Wrpc_faq>


Resume
------------------


However, you will find a resume of the most asked questions below:

### I can't generate a PPS signal or any point in section 5 is not working

In all of those cases the PLL has to be locked. That means your card has to be either in Master Mode, Grand Master or Slave Mode.

When you have only one card (or some but unconnected) it should be working as Free Running Master. If you have two of them (or more), one of them has to work as master, the others as Slaves and all of them have to be synced via PTP as explained en section [4.3](#configure-in-slave-master-mode). To say that is running Grand Master Mode is to make sure the PLL has already been locked to the external reference (see [Appendix](#appendix)).

### I don't see the interface using ifconfig

Please check if you see them using `ifconfig -a`, if you see them by using the `-a` option you just need
to bring them up. This is explained in the [Bring up the network interface](#bring-up-the-network-interface) section.

### Compilation of spec-sw is impossible

To compile you must have at least the 2.6.36 kernel (errors occurs with
2.6.32, 2.6.34). We recommend you to use the latest version of your distribution.

If you think you have the correct setup you can also use our automatic build
script that also generate a report with your system information.

	sudo scripts/wr-ssk-get --report

After a few minutes your project should have been installed correctly, if you
still have problems send us the generated `report.log` files in the main folder.


### The starting kit is based on Ubuntu. How can I use it with Scientific Linux?

The commands to execute in order to update the kernel using scientific linux (you need at least the 2.6.36 to compile the starting kit) are:

~~~~~{.bash}
rpm --import http://elrepo.org/RPM-GPG-KEY-elrepo.org
rpm -Uvh http://elrepo.org/elrepo-release-6-5.el6.elrepo.noarch.rpm
sudo yum --enablerepo=elrepo-kernel --skip-broken install kernel-lt kernel-lt-devel kernel-lt-headers
~~~~~~~~~~

You can have more information on how to do this by browsing the (http://elrepo.org/tiki/tiki-index.php "ELRepo Project")
After this you should reboot your PC with the new kernel source and start from [section compile & install](#compile-install)


### I can't detect any level on my inputs.

You should first try to desactivate the termination resistance that can
consume most of the current if this one is really low.

Otherwise you should configure the DAC to set up a different threshold, however this option is not actually implemented.


### The link is down, but everything is connected

*The SFPs and the optical fiber are plugged but the error message appears `Link down --`
and you can not see any green LED on the [SPEC].*

Please check that the SFP connector is totally plugged into the [SPEC]. The SFP cage of the [SPEC]
can be blocked sometimes by the PC back-panel. You must push the SFP
until hearing a "clicking noise".

### What is the latency in the WR-NIC

Many people ask us about the latency in the WR-NIC.

The answer depends really on what you want to measure: The only "relevant" delay might be the time between the SFP and the PCIe which is less than 20 cycles.
However the delay introduced by your nondeterministic OS is much more important.

### Why am I getting "Warning: tx timestamp never became available" message form the UART?

This warning seems to appear after unnistalling wr-nic driver if PTP has already been started. It can be easily solved by reinstalling the driver.

After that the device keeps working as was previously specified.

### No SFP in EEPROM database

When you try to match your SFPs and you obtain an error such as **Could not match to DB** you
need to add your own SFP parameters. Please refer to the [Calibration Section](#calibration)
to understand how to add correct SFPs parameters.


### Modprobe is not working

If you get and error like the following:

~~~~~~~{.sh}
>:$ sudo modprobe wr-nic
FATAL: Module wr_nic not found.
~~~~~~~~~

Be sure that you have run the `sudo depmod -a` command.
Otherwise you can still load the kernel modules with insmod using the
following commands:

~~~~~~~{.sh}
>:$ sudo insmod kernel/fmc.ko
>:$ sudo insmod kernel/spec.ko
>:$ sudo insmod kernel/wr-nic.ko
~~~~~~~~~~~~~

### The system hangs-up

Unfortunately, in some circumstances the system might hang during or
 slightly after firmware loading.
Please first check that you were not using the virtual UART while
 removing/inserting kernel modules.

Please help us to solve this error by sending us a log with the following information:

* What is your system & distribution  (`uname -a`)
* All the commands executed from login to the crash
* The serial log starting from the login (dmesg)


Glossary
===========

NIC
:	Network Interface Card
DIO
:	Digital Input/Output
PTP
:	Precise Time Protocol, a time synchronization protocol
EEPROM
:	Electrically Erasable Programmable Read-Only Memory, a non-volatile memory
FMC
:	FPGA Mezzanine Card, an ANSI standard for mezzanine card form factor.
LEMO
:	LEMO is the name of a push-pull connectors made by the LEMO company.
SFP
:	Small form-factor pluggable transceiver, a hot-pluggable transceiver for optical fiber
PCIe
:	Peripheral Component Interconnect Express, a high-speed serial computer expansion bus standard
LM32
:	LatticeMico32 is a 32-bit microprocessor soft core optimized for field-programmable gate arrays (FPGAs).
HDL
:	Hardware description language
SPEC
:	Simple PCIe FMC carrier
UART
:	Universal Asynchronous Receiver/Transmitter
WB
: 	Whishbone Bus, an open system bus interconnect architecture designed for reuse
WRC / WRPC
:	 White Rabbit (PTP) Core, the main funcionality shared by all WR elements.
[WR]
: 	White Rabbit
[WRS]
: 	White Rabbit Switch


[WR]:http://www.sevensols.com/whiterabbitsolution
[WRS]: http://www.sevensols.com/en/products/wr-switch.html
[WRSSK]: http://www.sevensols.com/en/products/wr-starting-kit.html
[SPEC+FMCDIO]: http://www.sevensols.com/en/products/wr-starting-kit.html
[SPEC]: www.sevensols.com/en/products/spec.html
[FMC]: http://www.ohwr.org/projects/fmc-projects
[FMC-DIO]: http://www.sevensols.com/en/products/fmc-dio.html
[FMCDIO]: http://www.sevensols.com/en/products/fmc-dio.html
[FMC fine-delay]: http://www.sevensols.com/en/products/fmc-del.html
[OHWR]: http://www.ohwr.org/projects/white-rabbit



References
============

* [spec-sw.pdf] main documentation of the spec-sw
* [wrpc.pdf] White Rabbit Core documentation
* [fmc-bus.pdf] How the FMC bus driver works and how to write one for your own FMC board.
* SFPs information <http://www.ohwr.org/projects/white-rabbit/wiki/SFP>
* White Rabbit Calibration: <http://www.ohwr.org/documents/213>
* [spec-2-spec]: SPEC-2-SPEC demo
* [wr_external_reference.pdf]: How to use the grandmaster mode in the switch.
* [spec-getting-started.pdf]:A tutorial to get ready to work with the (SPEC), including hardware deployment instructions, full required toolchain setup and a collection of step-by-step demonstrative tutorials.

<!-- List of links -->



[wr-switch-guide.pdf]: http://www.ohwr.org/attachments/download/2263/wr-switch-sw-v3.3-20130725_userguide.pdf
[wr-nic]: http://www.ohwr.org/projects/wr-nic/
[wr-starting-kit]: http://www.ohwr.org/projects/wr-starting-kit/
[spec-sw]: http://www.ohwr.org/projects/spec-sw/
[wrpc-sw]: http://www.ohwr.org/projects/wrpc-sw/
[spec-golden]: http://www.ohwr.org/projects/spec/repository/show/trunk/hdl/golden
[spec-2-spec]: http://www.ohwr.org/projects/wr-cores/wiki/spec-to-spec
[spec-getting-started.pdf]:http://www.ohwr.org/projects/spec-getting-started/wiki
[wr-nic.pdf]: http://www.sevensols.com/dl/wr-nic/latest_stable.pdf
[fmc-bus.pdf]: http://www.ohwr.org/attachments/download/2685/fmc-bus-2014-02-release.pdf
[spec-sw.pdf]:http://www.sevensols.com/dl/spec-sw/latest_stable.pdf
[wr_external_reference.pdf]: http://www.ohwr.org/attachments/1647/wr_external_reference.pdf
[wrpc.pdf]: http://www.ohwr.org/attachments/2559/wrpc-v2.1.pdf
[wrpc-v2.1.pdf]: http://www.ohwr.org/attachments/2559/wrpc-v2.1.pdf
[Seven Solutions]: http://www.sevensols.com
