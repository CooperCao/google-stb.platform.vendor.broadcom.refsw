#
# Common makefile to build Linux USB DHD and its supporting applications
# Package usb dhd sources and needed usb dongle images
#
# NOTE: This makefile should not be used by itself, it is included into
# other brands like linux-external-dongle-usb.mk
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#

NULL               := /dev/null
FIND               := /usr/bin/find
BUILD_BASE         := $(shell pwd)
SRCDIR             := $(BUILD_BASE)/src
RELEASEDIR         := $(BUILD_BASE)/release


export BRAND_RULES       = brand_common_rules.mk
export MOGRIFY_RULES     = mogrify_common_rules.mk

# Mogrifier step pre-requisites
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
export MOGRIFY_FILETYPES =
export MOGRIFY_EXCLUDE   =

MOGRIFY             = perl $(SRCDIR)/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS)) -skip_copyright_open
SRCFILTER          := perl $(SRCDIR)/tools/build/srcfilter.pl
SRCFILELIST        := $(SRCDIR)/tools/release/linux-dhd-usb-filelist.txt
SRCGPLFILELIST     := $(SRCDIR)/tools/release/linux-dhd-usb-gpl-filelist.txt
UNAME              := $(shell uname -a)
WARN_FILE          := _WARNING_PARTIAL_BUILD_DO_NOT_USE
DOXYGEN            := /tools/oss/bin/doxygen

# For usb-media we only build for bcm for now.
ifneq ($(findstring media, $(BRAND)),)
	OEM_LIST           := bcm
else
	OEM_LIST           ?= bcm
endif # BRAND

# Place a warning file to indicate build failed
define WARN_PARTIAL_BUILD
	touch $1/$(WARN_FILE)
endef # WARN_PARTIAL_BUILD

# Remove warning file to indicate build step completed successfully
define REMOVE_WARN_PARTIAL_BUILD
	rm -f $1/$(WARN_FILE)
endef # REMOVE_WARN_PARTIAL_BUILD

VALID_BRANDS:=linux-external-dongle-usb-media linux-external-dongle-usb linux-internal-dongle-usb linux-mfgtest-dongle-usb

ifeq ($(filter $(VALID_BRANDS),$(BRAND)),)
        $(error ERROR This $(BRAND) build is not a standalone build brand. Use one of the $(VALID_BRANDS) brands)
endif # BRAND

ifneq ($(findstring x86_64,$(UNAME)),)
	32BIT = -32 ARCH=i386
	32BITCFLAG = -m32
	32BITON64 = CC='gcc -m32'
	64BITON64 = CC='gcc -m64'
	32BITON64LIB = CROSS_LD_PATH=-L/projects/hnd/tools/linux/lib
	64BITON64LIB = CROSS_LD_PATH=-L/projects/hnd/tools/linux/lib_x86_64
	export 32ON64=1
endif # UNAME

ifneq ($(findstring media, $(BRAND)),)
	MEDIA_BLD  := 1
	SRCFILELIST := $(SRCDIR)/tools/release/linux-dhd-usb-media-filelist.txt
endif # BRAND

## Some of these macros can be overriden by calling brand makefiles

# Package up embeddable images as well for each oem
RLS_IMAGES_bcm      += $(EMBED_IMAGE_bcm)

ALL_DNGL_IMAGES     += $(foreach oem,$(OEM_LIST),$(RLS_IMAGES_$(oem)))

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

#---------------------------------------------------------------
# Options to host-side for updated Fedora Core 15
#---------------------------------------------------------------
# fc15 i686 (32bit)
fc15-dhd          := LINUXVER=2.6.38.6-26.rc1.fc15.i686.PAE
fc15-dhd          += CROSS_COMPILE=/tools/bin/
fc15-dhd          += dhd-cdc-usb-gpl
fc15-dhd          += dhd-cdc-usb-comp-gpl
fc15-dhd          += dhd-cdc-usb-urblim-comp-gpl
fc15-app          := NATIVEBLD=1 TARGETARCH=x86 $(32BITON64) $(32BITON64LIB)
#---------------------------------------------------------------
# Options to host-side for updated Fedora Core 15
#---------------------------------------------------------------
# fc15 (64bit)
fc15_64-dhd       := LINUXVER=2.6.38.6-26.rc1.fc15-x86_64
fc15_64-dhd       += CROSS_COMPILE=/tools/bin/
fc15_64-dhd       += dhd-cdc-usb-gpl
fc15_64-dhd       += dhd-cdc-usb-comp-gpl
fc15_64-dhd       += dhd-cdc-usb-urblim-comp-gpl
fc15_64-app       := NATIVEBLD=1 TARGETARCH=x86_64 $(64BITON64) $(64BITON64LIB)
#---------------------------------------------------------------
# Options to host-side for updated Fedora Core 19
#---------------------------------------------------------------
# fc19 i686 (32bit)
fc19-dhd          := LINUXVER=3.11.1-200.fc19.i686.PAE
fc19-dhd          += CROSS_COMPILE=/tools/bin/
fc19-dhd          += dhd-cdc-usb-gpl
fc19-dhd          += dhd-cdc-usb-comp-gpl
fc19-dhd          += dhd-cdc-usb-urblim-comp-gpl
fc19-app          := NATIVEBLD=1 TARGETARCH=x86 $(32BITON64) $(32BITON64LIB)
#---------------------------------------------------------------
# Options to host-side for updated Fedora Core 19
#---------------------------------------------------------------
# fc19 (64bit)
fc19_64-dhd       := LINUXVER=3.11.1-200.fc19.x86_64
fc19_64-dhd       += CROSS_COMPILE=/tools/bin/
fc19_64-dhd       += dhd-cdc-usb-gpl
fc19_64-dhd       += dhd-cdc-usb-comp-gpl
fc19_64-dhd       += dhd-cdc-usb-urblim-comp-gpl
fc19_64-app       := NATIVEBLD=1 TARGETARCH=x86_64 $(64BITON64) $(64BITON64LIB)
#---------------------------------------------------------------
# Options for host-side for MIPS on 7346 platform (Media)
stbfc15-dhd          := CROSS_COMPILE=mipsel-linux-
stbfc15-dhd          += LINUXDIR=/tools/linux/src/stblinux-2.6.37-3.0
stbfc15-dhd          += ARCH=mips
stbfc15-dhd          += PATH="/projects/hnd/tools/media/stbgcc-4.5.3-2.1/bin/:$(PATH)"
stbfc15-dhd          += dhd-cdc-usb-gpl
stbfc15-dhd          += dhd-cdc-usb-comp-gpl
# Options to build wl and bcmdl executables
stbfc15-app          := TARGETENV=linuxmips TARGETARCH=mips TARGETOS=unix CC=mipsel-linux-gcc LIBUSB_PATH=/projects/hnd/tools/linux/lib/mips
stbfc15-app          += PATH="/projects/hnd/tools/media/stbgcc-4.5.3-2.1/bin/:$(PATH)"
#---------------------------------------------------------------
# Options to cross-compile host-side for 3.4.5 nexus10
nexus10-args       := LINUXDIR=/tools/linux/src/linux-3.4.5-nexus10-JB-MR1-OPEN
nexus10-args       += CROSS_COMPILE=arm-eabi-
nexus10-args       += ARCH=arm
nexus10-args       += PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.4.3/bin/:$(PATH)"
nexus10-dhd        += $(nexus10-args)
nexus10-dhd        += dhd-cdc-usb-android-cfg80211-jellybean-reqfw
nexus10-dhd        += dhd-cdc-usb-android-cfg80211-jellybean-reqfw-debug
#---------------------------------------------------------------
# Options to cross-compile host-side (icsmr2 build) for Panda ICS
panda-ics-dhd-opts   := LINUXVER=3.0.8-panda
panda-ics-dhd-opts   += CROSS_COMPILE=arm-eabi-
panda-ics-dhd-opts   += ARCH=arm
panda-ics-dhd-opts   += PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.4.0/bin:$(PATH)"
panda-ics-dhd-opts   += OEM_ANDROID=1
panda-ics-dhd        := $(panda-ics-dhd-opts)
panda-ics-dhd        += dhd-cdc-usb-android-cfg80211-reqfw
panda-ics-dhd        += dhd-cdc-usb-android-cfg80211-reqfw-debug
#---------------------------------------------------------------
# Options to cross-compile host-side for Panda JB
panda-jb-dhd-opts   := LINUXVER=3.2.0-panda
panda-jb-dhd-opts   += CROSS_COMPILE=arm-eabi-
panda-jb-dhd-opts   += ARCH=arm
panda-jb-dhd-opts   += PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.4.0/bin:$(PATH)"
panda-jb-dhd-opts   += OEM_ANDROID=1
panda-jb-dhd        := $(panda-jb-dhd-opts)
panda-jb-dhd        += dhd-cdc-usb-android-cfg80211-jellybean-reqfw
panda-jb-dhd        += dhd-cdc-usb-android-cfg80211-jellybean-reqfw-debug
#---------------------------------------------------------------
# Build app wl/dhd for this os variant
panda-app        := TARGETENV=android_ndk_r6b TARGETARCH=arm_android_ndk_r6b TARGETOS=unix
panda-app        += TARGET_PREFIX=/projects/hnd/tools/linux/hndtools-arm-eabi-4.4.0/bin/arm-eabi-
panda-app        += TARGET_NDK=/projects/hnd_tools/linux/android-ndk-r6b LINUXDIR=/tools/linux/src/linux-3.0.3-panda
#---------------------------------------------------------------

# From above list, choose os variants for each oem
DRIVERS_bcm       ?= fc15 fc19 fc19_64 stbfc15
DRIVERS_android   ?= panda-jb panda-ics nexus10

# From above list, choose os variants for each oem for apps
# WARNING:
# WARNING: Only one app variant per CPU arch type per OEM
# WARNING:
# for usb media, we don't build any android apps till they are ported to
# android
ifdef MEDIA_BLD
    APPS_bcm      := fc15 stbfc15
    APPS_android  :=
else
    APPS_bcm      ?= fc15 fc15_64
    APPS_android  ?= panda
endif


# Generic hardware-less DHD driver make targets
DHD_DRIVERS_bcm      := $(foreach driver,$(DRIVERS_bcm),$(driver)-dhd)
DHD_DRIVERS_android  := $(DRIVERS_android:%=%-dhd)

# OPENSRC driver make targets (if any)
OPENSRC_DRIVERS_bcm  := fc15-opensrc

# App build targets for each oem
APP_DRIVERS_bcm      := $(APPS_bcm:%=%-app-bcm)
APP_DRIVERS_android  := $(APPS_android:%=%-app-android)

# For oem=bcm, if no specific app requests are specified, use lowest kernel to
# produce apps
APP_DRIVERS_bcm      := $(if $(APP_DRIVERS_bcm),$(APP_DRIVERS_bcm),$(if $(rh90-app),rh90-app,))

# SVN bom specification
ifneq ($(findstring media, $(BRAND)),)
	HNDSVN_BOM := dhd-usb-media.sparse
else
	HNDSVN_BOM := hndrte.sparse
endif # BRAND

# These symbols will be UNDEFINED in the source code by the transmogirifier
#
# NOTE: Since this is a global makefile, include only gloabl DEFS and UNDEFS
# here. Device and Build type specific flags should go to individual (calling)
# makefile
#
UNDEFS += CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ \
          CONFIG_BCM93725 BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
          CONFIG_BCM93725 BCM93725B BCM94100 BCM_USB NICMODE ETSIM \
          INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS \
          __klsi__ VX_BSD4_3 ROUTER BCMENET BCM4710A0 \
          CONFIG_BCM4710 CONFIG_MIPS_BRCM DSLCPE LINUXSIM BCMSIM \
          BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 BCMUSBDEV BCMCCX BCMEXTCCX  \
          BCMSDIODEV WLNOKIA WLFIPS BCMWAPI_WPI BCMP2P BCMWAPI_WAI

UNDEFS += DEBUG_LOST_INTERRUPTS BCMQT BCMSLTGT
UNDEFS += BCMSDIO SDTEST BCMPERFSTATS

# These symbols will be DEFINED in the source code by the transmogirifier
#
DEFS += BCM47XX BCM47XX_CHOPS BCMUSB

# Combined xmog in and out flags for specific oem needs
#SAMPLE# DEFS_bcm      += -DOEM_BCM -UOEM_<OEM1> -UOEM_<OEM2>
ifdef MEDIA_BLD
	DEFS_bcm      += -DOEM_BCM
else
	DEFS_bcm      += -DOEM_BCM -UOEM_ANDROID
endif
DEFS_android  += -UOEM_BCM -DOEM_ANDROID
ifeq ($(findstring internal,$(BRAND)),)
  DEFS_bcm     += -UBCMEMBEDIMAGE -UBCM_DNGL_EMBEDIMAGE
  DEFS_android += -UBCMEMBEDIMAGE -UBCM_DNGL_EMBEDIMAGE
endif # BRAND

# These are extensions of source files that should be transmogrified
# (using the parameters specified in DEFS and UNDEFS above.)
#
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

all: build_start checkout mogrify build_dongle_images copy_dongle_images prebuild_prep build_dhd build_apps release build_end

include linux-dongle-image-launch.mk
ifneq ($(ALLCHIPS),1)
# Unreleased-chiplist is included only for non-internal builds
ifeq ($(findstring BCMINTERNAL,$(DEFS))$(findstring internal,$(BRAND)),)
include unreleased-chiplist.mk
endif # BCMINTERNAL
endif # ALLCHIPS

include $(MOGRIFY_RULES)
include $(BRAND_RULES)

DNGL_IMG_FILES    += rtecdc.bin.trx
DNGL_IMG_FILES    += rtecdc.trx

# check out files
checkout: $(CHECKOUT_TGT)

build_start:
	@$(MARKSTART_BRAND)

mogrify: checkout
	@$(MARKSTART)
	$(MAKE) -f $(MOGRIFY_RULES)
	@$(MARKEND)	

## ------------------------------------------------------------------
## Mogrify sources. Only common non-oem specific flags are mogrified
## in or out. Oem specific flag mogrification happens in build_<oem>_dhd
## step
## ------------------------------------------------------------------

define MOGRIFY_LIST
	$(FIND) src components $(MOGRIFY_EXCLUDE) -type f -print  |  \
		perl -ne 'print if /($(subst $(space),|,$(foreach ext,$(MOGRIFY_EXT),$(ext))))$$/i' > \
		src/.mogrified
endef

build_include: checkout
	@$(MARKSTART)
#disabled#	$(MAKE) -C src/include
	@$(MARKEND)	

## ------------------------------------------------------------------
## Build oem brand specific DHD modules
## For each oem brand, a separate set of sources are filtered, mogrified and then
## dhd is built with specific embeddable dongle image
## ------------------------------------------------------------------
prebuild_prep: $(foreach oem,$(OEM_LIST),prebuild_$(oem)_prep)

$(foreach oem,$(OEM_LIST),prebuild_$(oem)_prep): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),prebuild_$(oem)_prep):
	@$(MARKSTART)
	@echo "Copy over dhd sources to $(oem) build workspace"
	mkdir -p build/$(oem)
	cp $(SRCFILELIST) $(SRCFILELIST)_$(oem)
	$(MOGRIFY) $(DEFS_$(oem)) $(SRCFILELIST)_$(oem)
	$(FIND) src components | $(SRCFILTER) -v $(SRCFILELIST)_$(oem) | col -b | \
		$(GREP_SKIPCMD) | \
		pax -rw $(PAX_SKIPCMD)  build/$(oem)
	rm -f $(SRCFILELIST)_$(oem)
	@if [ "$(oem)" == "bcm" ]; then \
	    $(MOGRIFY) -DESTA_OPENSRC_CLEANUP_LIST $(SRCGPLFILELIST); \
	    cd build/$(oem); \
	       $(FIND) src components | $(SRCFILTER) -v $(SRCGPLFILELIST) | col -b | \
	               xargs -t -l1 $(MOGRIFY) -strip_comments; \
	fi
	@sed -e "s%DNGL_IMAGE_NAME[[:space:]]*?=[[:space:]]*.*$$%DNGL_IMAGE_NAME?=$(EMBED_IMAGE_$(oem))%g" \
	    -e "s%OEM[[:space:]]*?=[[:space:]]*.*$$%OEM?=$(shell echo $(oem)| tr 'a-z' 'A-Z')%g" \
	    src/dhd/linux/Makefile > build/$(oem)/src/dhd/linux/Makefile
	@echo "New dongle image name and oem values in build/$(oem)/src/dhd/linux/Makefile"
	@grep "DNGL_IMAGE_NAME.*=\|OEM.*=" build/$(oem)/src/dhd/linux/Makefile
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_DIRS="build/$(oem)/src" MOGRIFY_FLAGS="$(DEFS_$(oem))"

	$(MAKE) -C build/$(oem)/src/include
	@$(MARKEND)
# 	#End of prebuild_$(oem)_prep


## ------------------------------------------------------------------
## Build DHD (generic) and OPENSRC (if any) for all oems
## ------------------------------------------------------------------
build_dhd: $(foreach oem,$(OEM_LIST),build_$(oem)_dhd) \
           $(foreach oem,$(OEM_LIST),$(if $(OPENSRC_DRIVERS_$(oem)),build_$(oem)_opensrc))

build_bcm_dhd: oem=bcm
build_bcm_dhd: $(DHD_DRIVERS_bcm)

build_android_dhd: oem=android
build_android_dhd: $(DHD_DRIVERS_android)

#- Common dhd build steps for all oems
$(foreach oem,$(OEM_LIST),$(DHD_DRIVERS_$(oem))):
	@$(MARKSTART)
	@if [ "$($@)" == "" ]; then \
	    echo "ERROR: $@ dhd options missing"; \
	    echo "ERROR: Check DHD_DRIVERS_$(oem) specification above"; \
	    exit 1; \
	fi
	$(MAKE) -C build/$(oem)/src/dhd/linux $(if $(BCM_MFGTEST), WLTEST=1) $($@) $(BUILD_$(oem)_DHD_TARGETS)
	@$(MARKEND)

build_bcm_opensrc: oem=bcm
build_bcm_opensrc: $(OPENSRC_DRIVERS_bcm)

# TODO: If we need to generate any opensrc/gpl drivers for usb dhd needs, use following rule
$(foreach oem,$(OEM_LIST),$(if $(OPENSRC_DRIVERS_$(oem)),$(OPENSRC_DRIVERS_$(oem)))):

## ------------------------------------------------------------------
## Build oem brand specific apps
## ------------------------------------------------------------------
build_apps: $(foreach oem,$(OEM_LIST),build_$(oem)_apps)

build_bcm_apps: oem=bcm
build_bcm_apps: $(APP_DRIVERS_bcm)

build_android_apps: oem=android
build_android_apps: $(APP_DRIVERS_android)

#- Common app build steps for all oems
$(foreach oem,$(OEM_LIST),$(APP_DRIVERS_$(oem))):: opts=$(subst -$(oem),,$@)
$(foreach oem,$(OEM_LIST),$(APP_DRIVERS_$(oem)))::
	@$(MARKSTART)
	@if [ "$($(opts))" == "" ]; then \
	    echo "ERROR: $(opts)-app options missing"; \
	    echo "ERROR: Check APP_DRIVERS_$(oem) specification above"; \
	    exit 1; \
	fi
	$(MAKE) -C build/$(oem)/src/wl/exe $(if $(BCM_MFGTEST), WLTEST=1) $($(opts))

ifdef MEDIA_BLD
	$(MAKE) -C build/$(oem)/src/usbdev/usbdl $(if $(BCM_MFGTEST), WLTEST=1) $($(opts));
	$(MAKE) -C build/$(oem)/src/wps/wpscli/linux/  $(if $(BCM_MFGTEST), WLTEST=1) $($(opts))
#	$(MAKE) -C build/$(oem)/src/apps/bwl/linux/  $(if $(BCM_MFGTEST), WLTEST=1) $($(opts))
	$(MAKE) -C build/$(oem)/src/apps/wfi/linux/wfi_api/ $(if $(BCM_MFGTEST), WLTEST=1) $($(opts))
	$(MAKE) -C build/$(oem)/src/wps/wpsapi/linux/ $(if $(BCM_MFGTEST), WLTEST=1) $($(opts)) BCM_WPS_IOTYPECOMPAT=1
#	#cd build/$(oem)/src/wps/linux/enr; $(MAKE) -f wps_enr_app.mk libs wpsenr wpsreg $(if $(BCM_MFGTEST), WLTEST=1) $($(opts))
else # Media Build
	if [ "$(oem)" != "android" ]; then \
		$(MAKE) -C build/$(oem)/src/usbdev/usbdl $(if $(BCM_MFGTEST), WLTEST=1) $($(opts)) ObjPfx=$(subst TARGETARCH=,,$(filter TARGETARCH%, $($(opts))))/; \
		$(MAKE) -C build/$(oem)/src/wl/exe wlm $(if $(BCM_MFGTEST), WLTEST=1) $($(opts)); \
	fi
endif # Media Build


## ------------------------------------------------------------------
## Copy over built binaries to release/<oem> folders for each oem
## ------------------------------------------------------------------
release_bins: $(foreach oem,$(OEM_LIST),release_$(oem)_bins release_$(oem)_extra_bins)

## ------------------------------------------------------------------
## First copy over *common* stuff to release/<oem> folders for each oem
## oem specific copy steps are in release_<oem>_extra_bins targets below
## ------------------------------------------------------------------
$(foreach oem,$(OEM_LIST),release_$(oem)_bins): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),release_$(oem)_bins): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),release_$(oem)_bins):
	@$(MARKSTART)
	install -d -m 755 $(RELDIR)/firmware
	install -d -m 755 $(RELDIR)/host
	install -d -m 755 $(RELDIR)/apps
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	# Copy README and release notes (oem specific ones override generic)
	touch $(RELDIR)/README.txt
	-install src/doc/BrandReadmes/$(BRAND).txt $(RELDIR)/README.txt
	-install src/doc/BrandReadmes/$(BRAND)_oem_$(oem).txt $(RELDIR)/README.txt
#comment# Copying native wl and dhd exe apps - if any
#comment#-install -p build/$(oem)/src/wl/exe/wl $(RELDIR)/apps
#comment#-install -p build/$(oem)/src/usbdev/usbdl/bcmdl $(RELDIR)/apps
#comment#-install -p build/$(oem)/src/wl/exe/socket/x86/wl_server_socket $(RELDIR)/apps
#comment#-install -p build/$(oem)/src/wl/exe/dongle/x86/wl_server_dongle $(RELDIR)/apps
#comment#-install -p build/$(oem)/src/wl/exe/serial/x86/wl_server_serial $(RELDIR)/apps
#comment#-install -p build/$(oem)/src/wl/exe/wifi/x86/wl_server_wifi $(RELDIR)/apps
#comment#-install -p build/$(oem)/src/wl/exe/wlm/x86/wlm.so $(RELDIR)/src/wl/exe/wlm/x86
	#Add media apps
ifdef MEDIA_BLD
	@install -pv build/$(oem)/src/wps/wpscli/linux/obj/x86-release/wpscli_test_cmd $(RELDIR)/apps
#	@install -pv build/$(oem)/src/apps/bwl/linux/bwl $(RELDIR)/apps
	@install -pv build/$(oem)/src/apps/wfi/linux/wfi_api/obj/x86-release/wfiapitester $(RELDIR)/apps
	@install -pv build/$(oem)/src/usbdev/usbdl/bcmdl $(RELDIR)/apps
	@install -pv build/$(oem)/src/wl/exe/wl $(RELDIR)/apps
	@install -pv build/$(oem)/src/wl/exe/socket_noasd/x86/wl_server_socket $(RELDIR)/apps
	@install -pv build/$(oem)/src/wl/exe/dongle_noasd/x86/wl_server_dongle $(RELDIR)/apps
	@install -pv build/$(oem)/src/wl/exe/serial_noasd/x86/wl_server_serial $(RELDIR)/apps
	@install -pv build/$(oem)/src/wl/exe/wifi_noasd/x86/wl_server_wifi $(RELDIR)/apps
	@install -pv build/$(oem)/src/wps/wpsapi/linux/release/x86/wpsenr $(RELDIR)/apps
	@install -pv build/$(oem)/src/wps/wpsapi/linux/release/x86/wpsreg $(RELDIR)/apps
	@install -pv build/$(oem)/src/wps/wpsapi/linux/release/x86/wpsapitester $(RELDIR)/apps
	@install -pv build/$(oem)/src/usbdev/usbdl/bcmdl $(RELDIR)/apps

	# Add media mips apps
	install -d -m 755 $(RELDIR)/apps/mips
	@install -pv build/$(oem)/src/wps/wpscli/linux/obj/mips-release/wpscli_test_cmd $(RELDIR)/apps/mips
	@install -pv build/$(oem)/src/apps/wfi/linux/wfi_api/obj/mips-release/wfiapitester $(RELDIR)/apps/mips
	@install -pv build/$(oem)/src/wl/exe/wlmips $(RELDIR)/apps/mips/wl
	@install -pv build/$(oem)/src/wl/exe/socket_noasd/mips/wl_server_socketmips $(RELDIR)/apps/mips/wl_server_socket
	@install -pv build/$(oem)/src/wl/exe/dongle_noasd/mips/wl_server_donglemips $(RELDIR)/apps/mips/wl_server_dongle
	@install -pv build/$(oem)/src/wl/exe/serial_noasd/mips/wl_server_serialmips $(RELDIR)/apps/mips/wl_server_serial
	@install -pv build/$(oem)/src/wl/exe/wifi_noasd/mips/wl_server_wifimips $(RELDIR)/apps/mips/wl_server_wifi
	@install -pv build/$(oem)/src/wps/wpsapi/linux/release/mips/wpsenr $(RELDIR)/apps/mips
	@install -pv build/$(oem)/src/wps/wpsapi/linux/release/mips/wpsreg $(RELDIR)/apps/mips
	@install -pv build/$(oem)/src/wps/wpsapi/linux/release/mips/wpsapitester $(RELDIR)/apps/mips
	@install -pv build/$(oem)/src/usbdev/usbdl/mips/bcmdl $(RELDIR)/apps/mips
else # Media Build
	# Copying native and other cross-compiled wl and dhd exe apps
	@echo "Copying native and cross compiled apps for $(foreach app,$(APP_DRIVERS_$(oem)),$(subst -$(oem),,$(app))) variants"
	for arch in $(foreach app,$(APP_DRIVERS_$(oem)),$(if $(filter TARGETARCH%, $($(patsubst %-$(oem),%,$(app)))),$(patsubst TARGETARCH=%,%,$(filter TARGETARCH%, $($(patsubst %-$(oem),%,$(app))))),x86)); do \
		if [ "$$arch" != "x86" ]; then \
			sarch=$$arch; \
		else \
			sarch=""; \
		fi; \
		echo "TARGETARCH=$$arch"; \
		mkdir -pv $(RELDIR)/apps/$$sarch; \
		if [ "$(oem)" != "android" ]; then \
			install -pv build/$(oem)/src/usbdev/usbdl/$$arch/bcmdl $(RELDIR)/apps/$$sarch; \
		fi; \
		install -pv build/$(oem)/src/wl/exe/wl$$sarch $(RELDIR)/apps/$$sarch/wl; \
		install -pv build/$(oem)/src/wl/exe/socket_noasd/$$arch/wl_server_socket$$sarch $(RELDIR)/apps/$$sarch/wl_server_socket; \
		install -pv build/$(oem)/src/wl/exe/dongle_noasd/$$arch/wl_server_dongle$$sarch $(RELDIR)/apps/$$sarch/wl_server_dongle; \
		install -pv build/$(oem)/src/wl/exe/serial_noasd/$$arch/wl_server_serial$$sarch $(RELDIR)/apps/$$sarch/wl_server_serial; \
		install -pv build/$(oem)/src/wl/exe/wifi_noasd/$$arch/wl_server_wifi$$sarch $(RELDIR)/apps/$$sarch/wl_server_wifi; \
	done
	# wlm fails to cross compile on arm, forcing x86 only
	if [ "$(oem)" != "android" ]; then \
		install -p build/$(oem)/src/wl/exe/wlm/x86/wlm.so $(RELDIR)/apps; \
	fi
endif
	# Copy over host kernel modules
	@cd build/$(oem); \
	for mdir in src/dhd/linux/dhd-cdc-*; do \
		mkdir -p -v $(RELEASEDIR)/$(oem)/host/$$(basename $${mdir}); \
		if [ -s "$$mdir/dhd.ko.stripped" ]; then \
			install -pv $$mdir/dhd.ko.stripped \
			$(RELEASEDIR)/$(oem)/host/$$(basename $${mdir})/dhd.ko; \
		elif [ -s "$$mdir/dhd.o.stripped" ]; then \
			install -pv $$mdir/dhd.o.stripped \
			$(RELEASEDIR)/$(oem)/host/$$(basename $${mdir})/dhd.o; \
		elif [ -s "$$mdir/bcmdhd.ko.stripped" ]; then \
			install -pv $$mdir/bcmdhd.ko.stripped \
			$(RELEASEDIR)/$(oem)/host/$$(basename $${mdir})/bcmdhd.ko; \
		fi; \
	done
#comment# Copying native and other cross-compiled wl and dhd exe apps
#comment#@echo "Copying native and cross compiled apps for $(foreach app,$(APP_DRIVERS_$(oem)),$(subst -$(oem),,$(app))) variants"
#comment#for arch in $(foreach app,$(APP_DRIVERS_$(oem)),$(if $(filter TARGETARCH%, $($(patsubst %-$(oem),%,$(app)))),$(patsubst TARGETARCH=%,%,$(filter TARGETARCH%, $($(patsubst %-$(oem),%,$(app))))),x86)); do \
#comment#	if [ -d build/$(oem)/src/wl/exe/obj/wl_noasd/$$arch ]; then \
#comment#		$(MAKE) -C build/$(oem)/src/wl/exe TARGETARCH=$$arch INSTALL_DIR=$(RELEASEDIR)/$(oem)/apps release_bins; \
#comment#	fi; \
#comment#	if [ "$$arch" != "x86" ]; then \
#comment#		sarch=$$arch; \
#comment#	fi; \
#comment#	#if [ "$(oem)" != "android" ]; then \
#comment#	#	install -pv build/$(oem)/src/usbdev/usbdl/$$sarch/bcmdl $(RELDIR)/apps/$$sarch; \
#comment#	#fi; \
#comment#done
#comment# wlm fails to cross compile on arm, forcing x86 only
#comment#if [ "$(oem)" != "android" ]; then \
#comment#	install -p build/$(oem)/src/wl/exe/wlm/x86/wlm.so $(RELDIR)/apps; \
#comment#fi

#comment# Copying any other cross-compiled wl and dhd exe apps - if any
#comment#@for arch in $(shell echo $(foreach app,$(APP_DRIVERS_$(oem)),$($(subt -$(oem),,$(app)))) | fmt -1 | grep TARGETARCH | sed -e 's/TARGETARCH=//g'); do \
#comment#     install -pv build/$(oem)/src/wl/exe/wl$$arch $(RELDIR)/apps; \
#comment#     install -pv build/$(oem)/src/wl/exe/socket/$$arch/wl_server_socket$$arch $(RELDIR)/apps; \
#comment#     install -pv build/$(oem)/src/wl/exe/dongle/$$arch/wl_server_dongle$$arch $(RELDIR)/apps; \
#comment#     install -pv build/$(oem)/src/wl/exe/serial/$$arch/wl_server_serial$$arch $(RELDIR)/apps; \
#comment#     install -pv build/$(oem)/src/wl/exe/wifi/$$arch/wl_server_wifi$$arch $(RELDIR)/apps; \
#	#done
	install src/doc/BCMLogo.gif $(RELEASEDIR)/$(oem)
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)
# 	#End of release_$(oem)_bins

# TODO: If we need to additional built/binary bits or steps for each oem
$(foreach oem,$(OEM_LIST),release_$(oem)_extra_bins):

## ------------------------------------------------------------------
## Create oem brand specific source releases
## ------------------------------------------------------------------

release_src: $(foreach oem,$(OEM_LIST),release_$(oem)_src_package) \
             $(foreach oem,$(OEM_LIST),release_$(oem)_src_extra_package)

$(foreach oem,$(OEM_LIST),release_$(oem)_src_package): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),release_$(oem)_src_package): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),release_$(oem)_src_package):
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	@echo "Generating release source + binaries package now in $(RELDIR) ...."
	cp $(SRCFILELIST) $(SRCFILELIST)_$(oem)
	$(MOGRIFY) $(DEFS_$(oem)) $(SRCFILELIST)_$(oem)
	# First create release/<oem>/src from SRCFILTER_<oem> src filter
	tar cpf - $(TAR_SKIPCMD) -C build/$(oem) \
	         `$(FIND) src components $(FIND_SKIPCMD) -print | $(SRCFILTER) -v $(SRCFILELIST)_$(oem)` | \
		 tar xpf - $(TAR_SKIPCMD) -C $(RELDIR)
	rm -f $(SRCFILELIST)_$(oem)
	tar cpf $(RELDIR)/dongle-host-driver-source.tar -C $(RELDIR) $(TAR_SKIPCMD) \
	         src firmware host apps README.txt BCMLogo.gif
	gzip -9 $(RELDIR)/dongle-host-driver-source.tar
ifdef BCM_MFGTEST
	@echo "Generating release binaries package now in $(RELDIR) ...."
	tar cpf $(RELDIR)/dongle-host-driver-binary.tar -C $(RELDIR) $(TAR_SKIPCMD) \
	         firmware host apps README.txt BCMLogo.gif
	gzip -9 $(RELDIR)/dongle-host-driver-binary.tar
endif # BCM_MFGTEST
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)
#comment#End of release_$(oem)_src_package

# Additional steps to create a open gpl package for certain oem brands
$(foreach oem,$(OEM_LIST),release_$(oem)_src_extra_package): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),release_$(oem)_src_extra_package): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),release_$(oem)_src_extra_package):
ifeq ($(BRAND),linux-external-dongle-usb)
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	@echo "Generating GPL release source + binaries package now in $(RELDIR) ...."
	rm -rf $(RELDIR)/open-src
	mkdir -p $(RELDIR)/open-src
	cp $(SRCFILELIST) $(SRCFILELIST)_$(oem)
	$(MOGRIFY) $(DEFS_$(oem)) $(SRCFILELIST)_$(oem)
	# Copy previous sources used for dhd/apps builds
	cp -rp $(RELDIR)/src $(RELDIR)/open-src/
	# Copy fresh orig sources for translating open to dual copyright license
	cd $(RELDIR)/open-src; \
	   rm -fv `$(FIND) src components | $(SRCFILTER) -v $(SRCFILELIST)_$(oem)`
	# Cleanip any other additional non-opensrc list of files
	# We do not release wl.exe binary or its sources
	$(MOGRIFY) -UESTA_OPENSRC_CLEANUP_LIST $(SRCFILELIST)_$(oem)
	$(FIND) src components | $(SRCFILTER) -v $(SRCFILELIST)_$(oem) | col -b | \
		$(GREP_SKIPCMD) | \
		pax -rw $(PAX_SKIPCMD) $(RELDIR)/open-src
	rm -f $(SRCFILELIST)_$(oem)
	# Translating copyright text 'open' to 'dual'
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_DIRS=$(RELDIR)/open-src/src MOGRIFY_FLAGS="-translate_open_to_dual_copyright"

	# Stripping comments some selective open-src files
	cd $(RELDIR)/open-src; \
	   $(FIND) src components | $(SRCFILTER) -v $(SRCGPLFILELIST) | col -b | \
		xargs -t -l1 $(MOGRIFY) -DLINUX_POSTMOGRIFY_REMOVAL -strip_comments
	# Built dhd bits are already picked in previous target
	# Exclude apps/wl*, we do not release wl.exe binary or its sources
	tar cpf $(RELDIR)/dongle-host-driver-source-open.tar -C $(RELDIR)  $(TAR_SKIPCMD) \
	         open-src/src firmware host apps README.txt BCMLogo.gif
	gzip -9 $(RELDIR)/dongle-host-driver-source-open.tar
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)
endif # BRAND
#comment#End of release_$(oem)_src_extra_package

release_doc: $(OEM_LIST:%=release_%_doc_package)
$(OEM_LIST:%=release_%_doc_package): oem=$(shell echo $(@F) | cut -d_ -f2)
$(OEM_LIST:%=release_%_doc_package): RELDIR=release/$(oem)
$(OEM_LIST:%=release_%_doc_package):
release_build_clean: $(foreach oem,$(OEM_LIST),release_$(oem)_clean)

$(foreach oem,$(OEM_LIST),release_$(oem)_clean): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),release_$(oem)_clean):
	@$(MARKSTART)
	@if [ -f "$(HNDRTE_FLAG)"   ]; then rm -fv  $(HNDRTE_FLAG);   fi
	@if [ -d "build/$(oem)"     ]; then \
	    echo "rm -rf build/$(oem)"; \
	    rm -rf build/$(oem); \
	    if [ "$(wildcard build/*)" == "build/$(oem)" ]; then \
	       rmdir -v build; \
	    fi; \
	fi
	@$(MARKEND)

release: release_bins integrate_firmware release_src release_build_clean

# Integrate firmwares into release directory
integrate_firmware:
	@$(MARKSTART)
	@echo "Copying images from firmware release"
ifneq ($(findstring media, $(BRAND)),)
#	src/tools/misc/combine_release_usb.sh $(BRAND) PHOENIX2_REL_6_10_114 43242a0min-roml $(TAG) linux-external-dongle-usb
	src/tools/misc/combine_release_usb.sh $(BRAND) FALCON_REL_5_90_188_51 43236b-roml $(TAG) linux-external-dongle-usb
else
	src/tools/misc/combine_release_usb.sh $(BRAND) PHOENIX2_REL_6_10_123_6 43242a0min-roml $(TAG)
endif # BRAND
	@$(MARKEND)

build_end:
	@rm -fv $(HNDRTE_FLAG)
	@$(MARKEND_BRAND)	

is32on64: # for build_config consumption
	@echo "$(BRAND) can be build on a 64 bit linux cluster"

.PHONY: FORCE checkout build_include build_end build_dongle_images prebuild_prep build_dhd build_apps $(foreach oem,$(OEM_LIST),build_$(oem)_dhd) $(foreach oem,$(OEM_LIST),build_$(oem)_apps) release release_bins release_src $(foreach oem,$(OEM_LIST),release_$(oem)_src_package) $(foreach oem,$(OEM_LIST),release_$(oem)_bins release_$(oem)_extra_bins) is32on64 integrate_firmware
