#
# BCM947xx VxWorks Router build/release Makefile
#
# This Makefile is intended to be run either in a Linux environment or
# in a cygwin environment (after executing
# src/tools/build/build_vx.sh). Make sure that both CFE and VxWorks
# can always be built using Linux, cygwin, or a Tornado command line
# shell.
#
# Copyright (C) 2002 Broadcom Corporation
#
# $Id$
#

BUILD_BASE := $(PWD)

BUILDDIR := $(BUILD_BASE)/build
RELEASEDIR := $(BUILD_BASE)/release

PATH := :/projects/hnd/tools/linux/hndtools-mipsel-linux/bin:$(PATH)

# These symbols will be UNDEFINED in the source code by the transmogrifier
UNDEFS += \
	CONFIG_BCM93725 CONFIG_BCM93725_VJ CONFIG_BCM93730 CONFIG_BCM933XX \
	BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
	BCM93725B BCM94100 BCM_USB NICMODE ETSIM \
	INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS __klsi__ VX_BSD4_3 \
	MICROSOFT MSFT BSPSRC BCMSDIO BCMPCMCIA POCKET_PC WL_PCMCIA

# These symbols will be DEFINED in the source code by the transmogrifier
DEFS += \
	BCM47XX_CHOPS BCM4710A0 BCM47XX BCMENET __CONFIG_SES__

# Source mogrification command
MOGRIFY = perl src/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS))

# These are module names and directories that will be checked out of CVS.
#
CVSMODULES = linuxsim

all: $(CVSMODULES)
# 	Build the linuxsim and 4710sim (cheeseball) images
	@date +"%c -- MARK PREBUILD --"
	$(MAKE) -C src/include
# 	MOGRIFY	the src
	@date +"%c -- MARK mogrify --"
	find src components -type f | file -f - | grep text | sed -ne 's/^\([^:]*\):.*/\1/p' > $@
	xargs $(MOGRIFY) < $@
	date +"%c -- MARK build and release linuxsim --"
#	Build the linuxsim and install the binaries
	$(MAKE) -j BCMSIM_DEBUG=0 -C src/tools/linuxsim BUILDDIR="$(BUILDDIR)" INSTALLDIR="$(RELEASEDIR)" install
	date +"%c -- MARK build 4710sim --"
#	Build 4710sim and install the binaries
	$(MAKE) -C src/tools/4710sim
	date +"%c -- MARK release 4710sim --"
	install -d $(RELEASEDIR)/
	install -d $(RELEASEDIR)/bin
	cp src/tools/4710sim/4318 $(RELEASEDIR)/bin
	cp src/tools/4710sim/4704 $(RELEASEDIR)/bin
	cp src/tools/4710sim/d11server $(RELEASEDIR)/bin
	date +"%c -- MARK archive src --"
#       Keep src directory as it is for future debugging
	tar -czf --exclude=*/.svn src.tar.gz src
#	Remove source and intermediate files
	rm -rf build
	rm -rf src

# Check out modules using export -kv
# This removes the CVS keyword escape syntax around CVS keyword expansions
# This messes up binary files, so follow up with a normal checkout of files
# used in the VxWorks build

# How to filter the cvs diff for override patch
OVPATCH = perl src/tools/build/ovpatch.pl $(OVERRIDE)

$(CVSMODULES):
ifneq ($(TAG),)
	cvs -Q export -kv $(if $(TAG),-r $(TAG)) $@
else
	cvs -Q co -P $@
endif
ifneq ($(OVERRIDE),)
#	Apply any changes from override files
	cvs status -l $(OVERRIDE) > /dev/null
	-cp $(OVERRIDE)/tools/build/ovpatch.pl src/tools/build/ovpatch.pl
	cvs diff -Nu $(OVERRIDE) | $(OVPATCH) > patch.cvs
	patch -E -p0 < patch.cvs
#	rm -f patch.cvs
endif

.PHONY: all
