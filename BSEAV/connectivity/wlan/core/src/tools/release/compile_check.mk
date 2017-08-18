#
# Compile Everything Linux Makefile
#
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$
#
# This Makefile tries to compile everything that can be compiled on
# Linux, as a very basic pre check-in source code verifier.  It does
# not perform transmogrification, copying, or any other non-compile
# parts of the regular build process, so don't expect the resulting
# executables to actually be usable.  The compilation is actually
# preformed by the Linux server farm, in 64bit where possible.  Use
# -j[n] for parallel makes.  Save the results to a file and scan
# carefully for Error messages.
#
# 	cd src
# 	make -j8 -f tools/release/compile_check.mk | tee results.txt
# 	...
# 	grep -C 3 Error results.txt
#
# Note that you will need to have the whole of tag-src checked out to
# be sure of having everything needed for a full compile.

B32=/tools/bin/bsub -q sj-Ihnd -I -R r32bit
B64=/tools/bin/bsub -q sj-Ihnd -I -R r64bit
UNAME=$(shell uname -s)

# On linux environments:
# Force path for MAKE.  This is needed because OSS make embeds the
# platform-specific bin dir in MAKE, which will break if passed on to
# the wrong platform via bsub.  Can't just remove the dirname since
# OSS make also puts its platform-specific bin dir before the generic
# one in PATH, and lsf propagates PATH.

ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)

MAKE := gmake

all: wl-win dhd-win dhd-ce exe-win exe-ce

else # linux

MAKE := /tools/oss/bin/make

all: wl-linux wl-bsd hndrte linuxsim hostdrv usbdl \
	linux-router

endif # UNAME

p: build_64 build_32

build_64:
	$(B64) $(MAKE) -f tools/release/compile_check.mk -j8 wl-bsd hostdrv usbdl \
	linux-router

build_32:
	$(B32) $(MAKE) -f tools/release/compile_check.mk -j8 wl-linux hndrte linuxsim

include/epivers.h: include/Makefile include/epivers.sh
	$(MAKE) -C include

ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)

# Dummy dongle image for many dhd builds
include/rtecdc.h:
	@echo "#- $0"
	@echo "/* Dummy rtecdc.h for dhd build */"      > include/rtecdc.h
	@echo "/* dhd name = $1 */"                    >> include/rtecdc.h
	@echo "#ifdef MEMBLOCK"                        >> include/rtecdc.h
	@echo "#define MYMEMBLOCK MEMBLOCK"            >> include/rtecdc.h
	@echo "#else"                                  >> include/rtecdc.h
	@echo "#define MYMEMBLOCK 2048"                >> include/rtecdc.h
	@echo "#endif"                                 >> include/rtecdc.h
	@echo "unsigned char dlarray[MYMEMBLOCK + 1];" >> include/rtecdc.h
	@echo "#undef MYMEMBLOCK"                      >> include/rtecdc.h
#	@echo -e "\nDummy rtecdc.h contents:\n"
#	@cat include/rtecdc.h

wl-win: include/epivers.h include/rtecdc.h
	$(MAKE) -C wl/sys/wdm

dhd-win: include/epivers.h include/rtecdc.h
	$(MAKE) -C dhd/wdm -f sdio.mk

dhd-ce: include/epivers.h include/rtecdc.h
	$(MAKE) -C dhd/ce ce_dhd

exe-win: include/epivers.h
	$(MAKE) -C wl/exe

exe-ce: include/epivers.h
	$(MAKE) -C wl/exe/ce

else # linux

hndrte: include/epivers.h
	$(MAKE) -C dongle/make

wl-linux: include/epivers.h
	$(MAKE) -C wl/linux

linuxsim: include/epivers.h
	$(MAKE) -C tools/linuxsim all

hostdrv: include/epivers.h
	$(MAKE) -C usbdev/hostdrv

usbdl: include/epivers.h
	$(MAKE) -C usbdev/usbdl

wl-bsd: include/epivers.h
	$(MAKE) -C wl/bsd

linux-router: include/epivers.h
	cp router/config/defconfig-router router/.config
	cp linux/linux/arch/mips/defconfig-bcm947xx-nfsrouter \
		linux/linux/.config
	$(MAKE) -j1 -C router oldconfig all install

endif # UNAME

clean:
	$(MAKE) -C dongle/make clean
	$(MAKE) -C wl/linux clean
	$(MAKE) -C usbdev/hostdrv clean
	$(MAKE) -C usbdev/usbdl clean
	$(MAKE) -C tools/linuxsim clean
	$(MAKE) -C wl/bsd clean
	-$(MAKE) -C wl/sys/wdm clean
	-$(MAKE) -C dhd/wdm -f sdio.mk clean
	-$(MAKE) -C dhd/ce clean
	-$(MAKE) -C wl/exe clean
	-$(MAKE) -C wl/exe/ce clean

# Need the whole of tag-src to be sure of compiling everything.
# Use checkout not update so that we don't add more dirs than we need.
# Limit accidents by checking that src is already there.
update:
	cd .. && test -d src && cvs co -P tag-src
