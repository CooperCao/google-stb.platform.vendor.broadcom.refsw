#
# Common makefile to build LINUX CDC DHD on linux from trunk.
#
# NOTE: This makefile should not be used by itself, it is included into
# other brands like linux-external-dhd.mk
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

NULL               := /dev/null
FIND               := /usr/bin/find
BUILD_BASE         := $(shell pwd)
SRCDIR             := $(BUILD_BASE)/src
RELEASEDIR         := $(BUILD_BASE)/release
MOGRIFY             = perl $(SRCDIR)/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS)) -skip_copyright_open
SRCFILTER          := perl $(SRCDIR)/tools/build/srcfilter.pl
DOXYGEN            := /tools/oss/bin/doxygen
UNAME              := $(shell uname -a)
OEM_LIST           ?= bcm
WARN_FILE          := _WARNING_PARTIAL_BUILD_DO_NOT_USE

export BRAND_RULES       = brand_common_rules.mk
export MOGRIFY_RULES     = mogrify_common_rules.mk

# Mogrifier step pre-requisites
# MOGRIFY_FLAGS gets DEFS and UNDEFS and any mogrify.pl cmd line options
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%) -skip_copyright_open
# Addition file types to mogrify in addition to what is in $(MOGRIFY_RULES)
export MOGRIFY_FILETYPES =
# Addition folders to skip in addition to what is in $(MOGRIFY_RULES)
export MOGRIFY_EXCLUDE   =

# These are brand specific component level central filelists
SRCFILELIST_ORG    := $(SRCDIR)/tools/release/linux-dhd-filelist.txt
SRCFILELIST_GPL_ORG:= $(SRCDIR)/tools/release/linux-dhd-gpl-filelist.txt
SRCFILELIST        := linux-dhd-filelist.txt
SRCFILELIST_GPL    := linux-dhd-gpl-filelist.txt

# OEM and HSL flags are per OEM specific, hence derived in OEM specific
# targets
GCCFILESDEFS_OEM =
GCCFILESDEFS_GPL = $(DEFS:%=-D%) -DESTA_OPENSRC_CLEANUP_LIST $(UNDEFS:%=-U%)

VALID_BRANDS := linux-external-dhd

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

ifneq ($(findstring x86_64,$(UNAME)),)
	32BIT = -32 ARCH=i386
	32GCC64 = CC='gcc -m32'
	# following one is only for WPS
	32GCC64PATH = PATH=/tools/oss/packages/i686-rhel4/gcc/default/bin:$(PATH)
	# following is for /tools/linux/src/linux-2.6.9-intc1/Makefile
	export 32ON64=1
endif # UNAME

# Dongle related variables and their references come from global makefile
# linux-dongle-image-launch.mk
HNDRTE_DEPS         := checkout
HNDRTE_IMGFN        := rtecdc.h

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

#---------------------------------------------------------------
# fc15 i686 (32bit)
fc15-dhd-opts    := LINUXVER=2.6.38.6-26.rc1.fc15.i686.PAE
fc15-dhd-opts    += CROSS_COMPILE=/tools/bin/
fc15-dhd-opts    += TARGET_OS=fc15 TARGET_ARCH=x86
fc15-dhd         := $(fc15-dhd-opts) dhd-cdc-sdstd \
                    dhd-cdc-sdstd-debug dhd-cdc-sdstd-apsta \
                    dhd-cdc-sdmmc-gpl dhd-cdc-sdstd-hc3 \
                    dhd-cdc-usb-gpl \
                    dhd-cdc-usb-comp-gpl
fc15-app         := NATIVEBLD=1 $(32GCC64)
# ---------------------------------------------------------------
# fc19 i686 (32bit)
fc19-dhd-opts    := LINUXVER=3.11.1-200.fc19.i686.PAE
fc19-dhd-opts    += CROSS_COMPILE=/tools/bin/
fc19-dhd-opts    += TARGET_OS=fc19 TARGET_ARCH=x86
fc19-dhd         := $(fc19-dhd-opts) dhd-cdc-sdstd \
                    dhd-cdc-sdstd-debug dhd-cdc-sdstd-apsta \
                    dhd-cdc-sdmmc-gpl dhd-cdc-sdstd-hc3 \
                    dhd-cdc-usb-gpl \
                    dhd-cdc-usb-comp-gpl
fc19-app         := NATIVEBLD=1 $(32GCC64)
# ---------------------------------------------------------------
# fc19 x84 (64bit)
# DISABLED due to compile errors
fc19x64-dhd-opts    := LINUXVER=3.11.1-200.fc19.x86_64
fc19x64-dhd-opts    += CROSS_COMPILE=/tools/bin/
fc19x64-dhd-opts    += TARGET_OS=fc19 TARGET_ARCH=x86_64
fc19x64-dhd         := $(fc19x64-dhd-opts) dhd-cdc-sdstd \
                    dhd-cdc-sdstd-debug dhd-cdc-sdstd-apsta \
                    dhd-cdc-sdmmc-gpl dhd-cdc-sdstd-hc3 \
                    dhd-cdc-usb-gpl \
                    dhd-cdc-usb-comp-gpl
fc19x64-app         := NATIVEBLD=1
#---------------------------------------------------------------
# Options to cross-compile host-side (icsmr2 build) for Panda ICS
panda-ics-dhd-opts   := LINUXVER=3.0.8-panda
panda-ics-dhd-opts   += CROSS_COMPILE=arm-eabi-
panda-ics-dhd-opts   += ARCH=arm TARGET_ARCH=armeabi
panda-ics-dhd-opts   += PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.4.0/bin:$(PATH)"
panda-ics-dhd-opts   += OEM_ANDROID=1
panda-ics-dhd        := $(panda-ics-dhd-opts)
panda-ics-dhd        += dhd-cdc-sdmmc-android-panda-cfg80211-oob
panda-ics-dhd        += dhd-cdc-sdmmc-android-panda-cfg80211-oob-debug
panda-ics-dhd        += dhd-cdc-sdmmc-android-panda-icsmr1-cfg80211-oob
panda-ics-dhd        += dhd-cdc-sdmmc-android-panda-icsmr1-cfg80211-oob-debug
#---------------------------------------------------------------
# Build app wl/dhd for this os variant
panda-app        := TARGETENV=android_ndk_r6b TARGETARCH=arm_android_ndk_r6b TARGETOS=unix
panda-app        += TARGET_PREFIX=/projects/hnd/tools/linux/hndtools-arm-eabi-4.4.0/bin/arm-eabi-
panda-app        += TARGET_NDK=/projects/hnd_tools/linux/android-ndk-r6b LINUXDIR=/tools/linux/src/linux-3.0.3-panda
#---------------------------------------------------------------

# From above list, choose os variants for each oem for host driver
DRIVERS_bcm          ?= fc15 fc19
DRIVERS_android      ?= panda-ics

# Generic hardware-less DHD driver make targets
DHD_DRIVERS_bcm      := $(DRIVERS_bcm:%=%-dhd-bcm)
DHD_DRIVERS_android  := $(DRIVERS_android:%=%-dhd-android)

# From above list, choose os variants for each oem for apps
# WARNING:
# WARNING: Only one app variant per CPU arch type per OEM
# WARNING:
APPS_bcm             ?= fc15
APPS_android         ?= fc15 panda

# App build targets for each oem
APP_DRIVERS_bcm      := $(APPS_bcm:%=%-app-bcm)
APP_DRIVERS_android  := $(APPS_android:%=%-app-android)

# SVN bom specification KLO: new sparse file here
HNDSVN_BOM            := dhd.sparse

# These symbols will be UNDEFINED in the source code by the mogrifier
#
# NOTE: Since this is a global makefile, include only global DEFS and UNDEFS
# here. Device and Build type specific flags should go to individual (calling)
# makefile
#

# UNDEFS common to both DHD & HSL build
COMMONUNDEFS += BCM33XX_CHOPS
COMMONUNDEFS += BCM42XX_CHOPS
COMMONUNDEFS += BCM4413_CHOPS
COMMONUNDEFS += BCM4710A0
COMMONUNDEFS += BCM93310
COMMONUNDEFS += BCM93350
COMMONUNDEFS += BCM93352
COMMONUNDEFS += BCM93725B
COMMONUNDEFS += BCM94100
COMMONUNDEFS += BCMENET
COMMONUNDEFS += BCMP2P
COMMONUNDEFS += BCMQT
COMMONUNDEFS += BCMSDIODEV
COMMONUNDEFS += BCMSIM
COMMONUNDEFS += BCMSIMUNIFIED
COMMONUNDEFS += BCMSLTGT
COMMONUNDEFS += BCM_USB
COMMONUNDEFS += BCMUSBDEV
COMMONUNDEFS += COMS
COMMONUNDEFS += CONFIG_BCM4710
COMMONUNDEFS += CONFIG_BCM93725
COMMONUNDEFS += CONFIG_BCM93725_VJ
COMMONUNDEFS += CONFIG_BCRM_93725
COMMONUNDEFS += CONFIG_BRCM_VJ
COMMONUNDEFS += CONFIG_MIPS_BRCM
COMMONUNDEFS += DEBUG_LOST_INTERRUPTS
COMMONUNDEFS += DSLCPE
COMMONUNDEFS += ETSIM
COMMONUNDEFS += INTEL
COMMONUNDEFS += __klsi__
COMMONUNDEFS += LINUXSIM
COMMONUNDEFS += NETGEAR
COMMONUNDEFS += NICMODE
COMMONUNDEFS += PSOS
COMMONUNDEFS += ROUTER
COMMONUNDEFS += VX_BSD4_3
COMMONUNDEFS += WLFIPS
COMMONUNDEFS += WLNINTENDO
COMMONUNDEFS += WLNINTENDO2
COMMONUNDEFS += SERDOWNLOAD

# additional UNDEFS for DHD build
DHD_UNDEFS += BCMCCX
DHD_UNDEFS += BCMEXTCCX

# These symbols will be DEFINED in the source code by the mogrifier
#

# DEFS common to both DHD & HSL build
COMMONDEFS += BCM47XX
COMMONDEFS += BCM47XX_CHOPS
COMMONDEFS += BCMSDIO

UNDEFS = $(COMMONUNDEFS) $(DHD_UNDEFS)
DEFS = $(COMMONDEFS) $(DHD_DEFS)

ALLDHD_DEFS =  $(COMMONDEFS) $(DHD_DEFS)
ALLDHD_UNDEFS = $(COMMONUNDEFS) $(DHD_UNDEFS)

# defines for filtering master filelist
# for file list pre-processing
#FILESDEFS = $(DEFS)
#FILESUNDEFS =$(patsubst %,NO_%,$(UNDEFS))
#FILESDEFS += FILESUNDEFS
#GCCFILESDEFS = $(patsubst %,-D%_SRC,$(FILESDEFS))

# Combined xmog in and out flags for specific oem needs
# TODO:  BCMWAPI_WPI should not be defined here (it came from a merge)
DEFS_bcm      += -DOEM_BCM -UWLNOKIA
DEFS_bcm      += -UOEM_ANDROID
DEFS_nokia    += -UOEM_BCM -DWLNOKIA
DEFS_nokia    += -UOEM_ANDROID
DEFS_android  += -UOEM_BCM -UWLNOKIA
DEFS_android  += -DOEM_ANDROID -UBCMSPI -USDHOST3
DEFS_android  += -UBCMNVRAM -URWL_DONGLE -UUART_REFLECTOR -UDHDARCH_MIPS
# Can't mogrify this out, breaks wl utility: DEFS_android  += -UD11AC_IOTYPES


# These are extensions of source files that should be mogrified
# (using the parameters specified in DEFS and UNDEFS above.)
#
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

all: build_start checkout filelists mogrify prebuild_prep build_dhd build_x86_apps build_apps release build_end

# Unreleased-chiplist is included only for non-internal builds

ifeq ($(findstring BCMINTERNAL,$(DEFS))$(findstring internal,$(BRAND)),)
include unreleased-chiplist.mk
endif # BCMINTERNAL

include $(MOGRIFY_RULES)
include $(BRAND_RULES)

# check out files
checkout: $(CHECKOUT_TGT)

build_start:
	@$(MARKSTART_BRAND)

filelists:
	@$(MARKSTART)
	cat $(SRCFILELIST_ORG) | \
		src/tools/release/rem_comments.awk > master_filelist.h
	cat $(SRCFILELIST_GPL_ORG) | \
		src/tools/release/rem_comments.awk > master_filelist_gpl.h
	gcc -E -undef  $(DEFS:%=-D%_SRC) -DESTA_OPENSRC_CLEANUP_LIST_SRC \
			$(UNDEFS:%=-DNO_%_SRC) \
			-Ulinux -o $(SRCFILELIST_GPL) master_filelist_gpl.h
	@$(MARKEND)

mogrify:
	@$(MARKSTART)
	$(MAKE) -f $(MOGRIFY_RULES)
	@$(MARKEND)

## ------------------------------------------------------------------
## Build oem brand specific DHD modules
## For each oem brand, a separate set of sources are filtered, mogrified and then
## dhd is built with specific embeddable dongle image
## ------------------------------------------------------------------
prebuild_prep: $(foreach oem,$(OEM_LIST),prebuild_$(oem)_prep)

$(foreach oem,$(OEM_LIST),prebuild_$(oem)_prep): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),prebuild_$(oem)_prep): OEM_DEFS= $(patsubst -D%,%, $(filter -D%,$(DEFS_$(oem)))) ESTA_OPENSRC_CLEANUP_LIST
$(foreach oem,$(OEM_LIST),prebuild_$(oem)_prep): OEM_UNDEFS= $(patsubst -U%,%,$(filter -U%,$(DEFS_$(oem))))
$(foreach oem,$(OEM_LIST),prebuild_$(oem)_prep):
	@$(MARKSTART)
	@echo "Copy over dhd sources to $(oem) build workspace"
	mkdir -p build/$(oem)
	mkdir -p build/$(oem)/components/shared/
	cp -R components/shared/* build/$(oem)/components/shared/
	gcc -E -undef $(ALLDHD_DEFS:%=-D%_SRC) $(OEM_DEFS:%=-D%_SRC) \
		$(ALLDHD_UNDEFS:%=-DNO_%_SRC) $(OEM_UNDEFS:%=-DNO_%_SRC) \
		-Ulinux -o $(SRCFILELIST)_$(oem) master_filelist.h
	if [ "$(oem)" == "bcm" ]; then \
	cat $(SRCFILELIST)_$(oem) $(SRCFILELIST_HSL)_$(oem) | sort -u > $(SRCFILELIST)_$(oem)_combo; \
	$(FIND) src components $(FIND_SKIPCMD) -print | $(SRCFILTER) -v $(SRCFILELIST)_$(oem)_combo | col -b | \
		$(GREP_SKIPCMD) | \
		pax -rw $(PAX_SKIPCMD) build/$(oem); \
	cd build/$(oem); \
	       $(FIND) src components $(FIND_SKIPCMD) -print | $(SRCFILTER) -v ../../$(SRCFILELIST_GPL) | col -b | \
	    xargs -t -l1 $(MOGRIFY) -strip_comments; \
	else \
	$(FIND) src components $(FIND_SKIPCMD) -print | $(SRCFILTER) -v $(SRCFILELIST)_$(oem) | col -b | \
		$(GREP_SKIPCMD)  | \
		pax -rw $(PAX_SKIPCMD)  build/$(oem); \
				if [ "$(oem)" == "android" ]; then \
	        $(FIND) src components $(FIND_SKIPCMD) -print | $(SRCFILTER) -v $(SRCFILELIST_HSL_android) | col -b | \
			$(GREP_SKIPCMD) | \
			pax -rw $(PAX_SKIPCMD) build/$(oem); \
		cd build/$(oem); \
		fi \
	fi

	sed -e "s%DNGL_IMAGE_NAME[[:space:]]*?=[[:space:]]*.*$$%DNGL_IMAGE_NAME?=$(EMBED_IMAGE_$(oem))%g" \
	    -e "s%OEM[[:space:]]*?=[[:space:]]*.*$$%OEM?=$(shell echo $(oem)| tr 'a-z' 'A-Z')%g" \
	    src/dhd/linux/Makefile > build/$(oem)/src/dhd/linux/Makefile
	@echo "New dongle image name and oem values in build/$(oem)/src/dhd/linux/Makefile"
	@grep "DNGL_IMAGE_NAME.*=\|OEM.*=" build/$(oem)/src/dhd/linux/Makefile
	$(FIND) build/$(oem)/src -type d -print  | xargs chmod ugo+rx
	$(MAKE) -f $(MOGRIFY_RULES) \
		MOGRIFY_DIRS=build/$(oem)/src \
		MOGRIFY_FLAGS="$(DEFS_$(oem)) -skip_copyright_open"
	$(MAKE) -C build/$(oem)/src/include
	# WARN: Do not exit for missing EMBED_IMAGE_$(oem) too soon.
	# WARN: release_$(oem)_bins checks this for all RLS_IMAGES_$(oem)
	# If EMBED_IMAGE_$(oem) is built, copy it over. It is not needed
	# for dhd build, as we don't embed image into host driver
	-@if [ -n "$(EMBED_IMAGE_$(oem))" ]; then \
	     if [ -s "$(DNGL_IMGDIR)/$(EMBED_IMAGE_$(oem))/$(DNGL_IMG_PFX).h" ]; then \
	        install -d build/$(oem)/$(DNGL_IMGDIR)/$(EMBED_IMAGE_$(oem)); \
	        install -p $(DNGL_IMGDIR)/$(EMBED_IMAGE_$(oem))/$(DNGL_IMG_PFX).h build/$(oem)/$(DNGL_IMGDIR)/$(EMBED_IMAGE_$(oem))/; \
	     else \
	        echo "ERROR: $(oem) dongle image $(EMBED_IMAGE_$(oem)) NOT BUILT"; \
	        echo "ERROR: Verify buildlog file: $(HNDRTE_RLSLOG)"; \
	     fi; \
	fi
	@$(MARKEND)
# 	#End of prebuild_$(oem)_prep

## ------------------------------------------------------------------
## Build DHD (generic) for all oems
## ------------------------------------------------------------------
build_dhd: $(foreach oem,$(OEM_LIST),build_$(oem)_dhd) \
	   $(foreach oem,$(OEM_LIST),$(if $(OPENSRC_DRIVERS_$(oem)),build_$(oem)_opensrc))

build_bcm_dhd: oem=bcm
build_bcm_dhd: $(DHD_DRIVERS_bcm)

build_android_dhd: oem=android
build_android_dhd: $(DHD_DRIVERS_android)

#- Common dhd build steps for all oems
$(foreach oem,$(OEM_LIST),$(DHD_DRIVERS_$(oem))):: opts=$($(subst -$(oem),,$@))
$(foreach oem,$(OEM_LIST),$(DHD_DRIVERS_$(oem)))::
	@$(MARKSTART)
	@if [ "$(opts)" == "" ]; then \
	    echo "ERROR: $(opts) dhd options missing"; \
	    echo "ERROR: Check DHD_DRIVERS_$(oem) specification above"; \
	    exit 1; \
	fi
	$(MAKE) -C build/$(oem)/src/dhd/linux \
		$(if $(BCM_MFGTEST), WLTEST=1) \
		BUILD_TAG=TRUNK $(opts) $(BUILD_$(oem)_DHD_TARGETS)
	@$(MARKEND)

build_bcm_opensrc: oem=bcm
build_bcm_opensrc: $(OPENSRC_DRIVERS_bcm)


## ------------------------------------------------------------------
## Build oem brand specific apps
## ------------------------------------------------------------------
.EXPORT_ALL_VARIABLES:
build_apps: $(foreach oem,$(OEM_LIST),build_$(oem)_apps)

build_bcm_apps: oem=bcm
build_bcm_apps: $(APP_DRIVERS_bcm)

build_android_apps: oem=android
build_android_apps: $(APP_DRIVERS_android)

#- Common app build steps for all oems
$(foreach oem,$(OEM_LIST),$(APP_DRIVERS_$(oem))):: opts=$($(subst -$(oem),,$@))
$(foreach oem,$(OEM_LIST),$(APP_DRIVERS_$(oem)))::
	@$(MARKSTART)
	$(MAKE) -C build/$(oem)/src/dhd/exe \
		$(if $(BCM_MFGTEST), WLTEST=1) BUILD_TAG=TRUNK $(opts)
	@$(MARKEND)


build_x86_apps:
	@$(MARKSTART)
	$(MAKE) -C src/usbdev/usbdl
	$(MAKE) -C src/tools/misc trx
	$(MAKE) -C src/wl/exe
	@$(MARKEND)



## ------------------------------------------------------------------
## Copy over built binaries to release/<oem> folders for each oem
## ------------------------------------------------------------------
release_bins: $(foreach oem,$(OEM_LIST),release_$(oem)_bins)

## ------------------------------------------------------------------
## First copy over *common* stuff to release/<oem> folders for each oem
## oem specific copy steps are in release_<oem>_extra_bins targets below
## ------------------------------------------------------------------
$(foreach oem,$(OEM_LIST),release_$(oem)_bins): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),release_$(oem)_bins): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),release_$(oem)_bins):
	@$(MARKSTART)
	install -d -m 755 $(RELDIR)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
#comment# Copy README and release notes (oem specific ones override generic)
	# touch $(RELDIR)/README.txt
#comment#comment out for now. will come back to fix this
#disable#-install src/doc/BrandReadmes/$(BRAND).txt $(RELDIR)/README.txt
#disable#-install src/doc/BrandReadmes/$(BRAND)_oem_$(oem).txt $(RELDIR)/README.txt
#comment# Copy over host kernel modules
	chmod 755 src/tools/build/SDE/sde_release
	src/tools/build/SDE/sde_release release-bins $@
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)

## ------------------------------------------------------------------
## Create oem brand specific source releases
## ------------------------------------------------------------------

release_src: $(foreach oem,$(OEM_LIST),release_$(oem)_src_package)

$(foreach oem,$(OEM_LIST),release_$(oem)_src_package): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),release_$(oem)_src_package):
	@$(MARKSTART)
	mkdir -p $(RELDIR)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	@echo "Generating release source + binaries package now in $(RELDIR) ...."
#comment# Call perl script to copy files in the filelist to release directories
	chmod 755 src/tools/build/SDE/sde_release
	src/tools/build/SDE/sde_release release-src $@ linux-dhd-filelist.txt
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)

## ------------------------------------------------------------------
## Create firmware release
## ------------------------------------------------------------------
integrate_firmware: $(foreach oem,$(OEM_LIST),release_$(oem)_firmware)

$(foreach oem,$(OEM_LIST),release_$(oem)_firmware): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),release_$(oem)_firmware):
	@$(MARKSTART)
	chmod 755 src/tools/build/SDE/sde_release
	src/tools/build/SDE/sde_release integrate-fw $@
	@$(MARKEND)

release: release_bins release_src

is32on64: # for build_config consumption
	@echo "$(BRAND) can be build on a 64 bit linux cluster"

build_end:
	@$(MARKEND_BRAND)

.PHONY: FORCE checkout build_include prebuild_prep build_dhd $(foreach oem,$(OEM_LIST),build_$(oem)_dhd) is32on64 release_build_clean build_apps $(foreach oem,$(OEM_LIST),$(APP_DRIVERS_$(oem))) build_x86_apps release release_bins release_src
