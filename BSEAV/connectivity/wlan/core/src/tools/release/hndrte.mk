#
# Master makefile to build all misc hndrte stuff
# (except for dongle images and dhd)
#
# Copyright (C) 2002 Broadcom Corporation
#
# $Id$
#

PARENT_MAKEFILE :=
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

export BRAND_RULES       = brand_common_rules.mk
include $(BRAND_RULES)

export VCTOOL      := svn
BUILD_BASE         := $(PWD)

UNAME       := $(shell uname -a)

HNDSVN_BOM  = hndrte.sparse

default: all build_end

.PHONY: all FORCE

FORCE:

all: build_start
	@$(MARKSTART)
	$(MAKE) -C src/include
##########################################################
## WARN: BUILD ONLY SELECT FOLDERS
## WARN: OTHERS ARE ANYWAY BUILT IN hndrte-dongle-wl brand
##########################################################
	$(MAKE) $(MAKEJOBS) -C src/dongle/make/test -k
#	$(MAKE) $(MAKEJOBS) -C src/dongle/make/sim -k
	$(MAKE) $(MAKEJOBS) -C src/dongle/make/usbrdl -k
##########################################################
## WARN: DO NOT ADD ENTIRE FOLDER HERE
## WARN: BUILD ONLY SELECTIVE BRINGUP and ROMLSIM IMAGES
##########################################################
	#
	# NOMAXSIZE=1 ignores errors when the required amount of ROM memory exceeds the size of
	# the ROM. This is useful for nightly builds of ROM targets. It allows the ROM build
	# infrastructure to be validated without the the need to constantly tweak the
	# ROM configuration in order to deal with increased memory requirements over time.
	# The command line option should be excluded (defaults to 0) when building real ROM
	# candidates for a chip tape-out.
	#
	$(MAKE) -C components/chips/images/roml/4365c0 NOMAXSIZE=1
	@$(MARKEND)

build_start:
	@$(MARKSTART_BRAND)

build_clean:
	@$(MARKSTART)
	-@find src components -type f -name "*.o" -print | xargs rm -f
	@$(MARKEND)

build_end: build_clean
	@$(MARKEND_BRAND)
