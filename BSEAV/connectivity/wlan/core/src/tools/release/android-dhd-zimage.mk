#
# Common makefile to build android DHD Dongle Image
#
# NOTE: This makefile should not be used by itself, it is included into
# other brands like android-external-dongle.mk
#
# Author: Kim Lo
# Contact: hnd-software-scm-list
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

export SHELL       := /bin/bash
NULL               := /dev/null
FIND               := /usr/bin/find
BUILD_BASE         := $(shell pwd)
SRCDIR             := $(BUILD_BASE)/src
RELEASEDIR         := $(BUILD_BASE)/release
MOGRIFY_SCRIPT     := perl $(SRCDIR)/tools/build/mogrify.pl
MOGRIFY             = perl $(SRCDIR)/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS)) -skip_copyright_open
SRCFILTER          := perl $(SRCDIR)/tools/build/srcfilter.pl
DOXYGEN            := /tools/oss/bin/doxygen
WPSMAKEFILE        := src/tools/release/linux-wps-enrollee.mk
UNAME              := $(shell uname -a)
OEM_LIST           ?= samsung
WARN_FILE          := _WARNING_PARTIAL_BUILD_DO_NOT_USE

MOGRIFIED          := mogrified_src
MOGRIFY_DIR        := $(BUILD_BASE)/src/bcmdhd/mk/$(MOGRIFIED)
PUBLIC_KERNELS     := /projects/hnd/tools/linux/SAMSUNG
KERNEL             := linux-3.10.0-TR_LTE-140905-BUILTIN
TOOLCHAIN_DIR      := /tools/linux/local/hndtools-arm-eabi-4.7/arm-eabi/bin

UNIQ_STR           := $(strip $(shell mktemp -u XXXXXXXX))
INPROGRESS_DIR     := /tmp/wlanswbuild/in-progress
INPROGRESS_UNIQ    := $(INPROGRESS_DIR)/$(UNIQ_STR)

export BRAND_RULES       = brand_common_rules.mk

VALID_BRANDS := android-dongle-zimage

ifeq ($(filter $(VALID_BRANDS),$(BRAND)),)
        $(error ERROR This $(BRAND) build is not a standalone build brand. Valid brands are $(VALID_BRANDS))
endif # BRAND

# Place a warning file to indicate build failed
define WARN_PARTIAL_BUILD
	touch $1/$(WARN_FILE)
endef # WARN_PARTIAL_BUILD

# Remove warning file to indicate build step completed successfully
define REMOVE_WARN_PARTIAL_BUILD
	rm -f $1/$(WARN_FILE)
endef # REMOVE_WARN_PARTIAL_BUILD

# following one is only for WPS
ifneq ($(findstring x86_64,$(UNAME)),)
	32GCC64PATH = PATH=/tools/oss/packages/i686-rhel4/gcc/default/bin:$(PATH)
endif # UNAME

empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

ifdef TAG
  vinfo := $(subst _,$(space),$(TAG))
else
  vinfo := $(shell date '+D11 REL %Y %m %d')
endif

maj     := $(word 3,$(vinfo))
min     := $(word 4,$(vinfo))
rcnum   := $(word 5,$(vinfo))
rcnum   := $(patsubst RC%,%,$(rcnum))
ifeq ($(rcnum),)
  rcnum := 0
endif
incr    := $(word 6,$(vinfo))
ifeq ($(incr),)
  incr  := 0
endif
RELNUM  := $(maj).$(min).$(rcnum).$(incr)

ifeq ($(findstring BCMINTERNAL,$(COMMONUNDEFS)),BCMINTERNAL)
  BLDTYPE ?= release
else
  BLDTYPE ?= debug
endif

include $(BRAND_RULES)

# check out files
checkout: $(CHECKOUT_TGT)


all: copy_kernel build_zimage release clean_kernel

copy_kernel:
	@$(MARKSTART)
	mkdir -p -v $(INPROGRESS_UNIQ) && \
	cd $(INPROGRESS_UNIQ) && \
	cp -r $(PUBLIC_KERNELS)/$(KERNEL).7z . && \
	7z x $(KERNEL).7z
	@$(MARKEND)

build_zimage:
	@$(MARKSTART)
	cd src/bcmdhd/mk && \
	$(MAKE) -f bcmdhd.mk BCMDHD_OEM=samsung BCMDHD_ANDROID_KERNEL=$(INPROGRESS_UNIQ)/$(KERNEL) BCMDHD_WORK_DIR=$(INPROGRESS_UNIQ) KERNEL_BCMDHD_DIR=drivers/net/wireless/bcmdhd4358 PROJECT=TR_LTE KERNEL_DATE=140905 bootimage
	@$(MARKEND)

release:
	@$(MARKSTART)
	mkdir -p release/external/samsung_TR_LTE/host && \
	cp $(INPROGRESS_UNIQ)/$(KERNEL)/boot.img.tar.md5 release/external/samsung_TR_LTE/host && \
	cp $(INPROGRESS_UNIQ)/$(KERNEL)/arch/arm/boot/zImage release/external/samsung_TR_LTE/host
	@$(MARKEND)

clean_kernel:
	@$(MARKSTART)
#comment# do a rm test first to make sure we keep rm -rf under control
	rm $(INPROGRESS_UNIQ)/$(KERNEL)/Makefile && \
	rm -rf $(INPROGRESS_UNIQ)
	@$(MARKEND)

.PHONY: FORCE checkout copy_kernel build_zimage clean_kernel release
