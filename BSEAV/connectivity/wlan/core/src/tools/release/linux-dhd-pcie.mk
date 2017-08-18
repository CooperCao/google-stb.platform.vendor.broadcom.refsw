#
# Common makefile to build Linux CDC DHD and PCIE Dongle Image(on linux).
# Package DHD sources with all pcie dongle images
#
# NOTE: This makefile should not be used by itself, it is included into
# other brands like linux-external-dongle-pcie.mk
#
# Contact: hnd-software-scm-list
#
# $Id$:
#

NULL               := /dev/null
FIND               := /usr/bin/find
BUILD_BASE         := $(shell pwd)
SRCDIR             := $(BUILD_BASE)/src
RELEASEDIR         := $(BUILD_BASE)/release
MOGRIFY             = perl $(SRCDIR)/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS)) -skip_copyright_open
SRCFILTER          := perl $(SRCDIR)/tools/build/srcfilter.pl
DOXYGEN            := /tools/oss/bin/doxygen
WPSMAKEFILE        := src/tools/release/linux-wps-enrollee.mk
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
SRCFILELISTS_COMPONENTS += src/tools/release/components/wlphy-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/bcmcrypto-filelist.txt

SRCFILELISTS_HSL_COMPONENTS += src/tools/release/components/wps-filelist.txt
SRCFILELISTS_HSL_COMPONENTS += src/tools/release/components/secfrw-filelist.txt
SRCFILELISTS_HSL_COMPONENTS += src/tools/release/components/pre_secfrw-filelist.txt
SRCFILELISTS_HSL_COMPONENTS += src/tools/release/components/p2p-filelist.txt
SRCFILELISTS_HSL_COMPONENTS += src/tools/release/components/dhcpd-filelist.txt
SRCFILELISTS_HSL_COMPONENTS += src/tools/release/components/supp-filelist.txt
SRCFILELISTS_HSL_COMPONENTS += src/tools/release/components/bwl-filelist.txt
SRCFILELISTS_HSL_COMPONENTS += src/tools/release/components/bcmcrypto-filelist.txt
SRCFILELISTS_HSL_COMPONENTS += src/tools/release/components/wfi-filelist.txt

SRCFILELIST_ORG    := $(SRCDIR)/tools/release/linux-dhd-pcie-filelist.txt
SRCFILELIST_GPL_ORG    := $(SRCDIR)/tools/release/linux-dhd-pcie-gpl-filelist.txt
SRCFILELIST        := linux-dhd-pcie-filelist.txt
SRCFILELIST_GPL        := linux-dhd-pcie-gpl-filelist.txt
SRCFILELIST_HSL_android:=$(SRCDIR)/tools/release/android-apps-filelist.txt
SRCFILELIST_HSL    := hsl_master_filelist

# OEM and HSL flags are per OEM specific, hence derived in OEM specific
# targets
GCCFILESDEFS_OEM =
GCCFILESDEFS_HSL =

LINUX_KERNEL_SRC_BCMDHD := linux-2.6.39.4-xoom linux-3.0.8-g49a0ff3-maguro-ics-mr1-4.0.3 linux-3.0.8-maguro-JB linux-3.4.5-manta-JB

VALID_BRANDS := \
	linux-external-dongle-pcie \
	linux-olympic-external-dhd-pcie \
	linux-olympic-internal-dhd-pcie \
	linux-external-dongle-pcie-media \
	linux-internal-dongle-pcie \
	linux-mfgtest-dongle-pcie

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

ifneq ($(findstring media, $(BRAND)),)
	MEDIA_BLD  := 1
endif # BRAND

## Some of these macros can be overridden by calling brand makefiles

# Other images (in addition to EMBED_IMAGE_<oem>) to be released to customer
# come from RLS_IMAGE_><oem> in brand makefile.
# WARN: Do not add any dongle images to following list. Instead, update
#       appropriate <brand>_ release makefiles.

RLS_IMAGES_bcm      += $(EMBED_IMAGE_bcm)
RLS_IMAGES_voipbcm  += $(EMBED_IMAGE_voipbcm)
RLS_IMAGES_android  += $(EMBED_IMAGE_android)

# Big list of default dongle images that are pre-built and exist under 'src',
# but need not be copied over to 'release/<oem>' folders

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

ifeq ($(findstring BCMINTERNAL,$(COMMONUNDEFS)),BCMINTERNAL)
  BLDTYPE ?= release
else
  BLDTYPE ?= debug
endif

# ---------------------------------------------------------------
# fc15 i686 (32bit)
fc15-dhd-opts    := LINUXVER=2.6.38.6-26.rc1.fc15.i686.PAE
fc15-dhd-opts    += CROSS_COMPILE=/tools/bin/
fc15-dhd         := $(fc15-dhd-opts) dhd-msgbuf-pciefd-debug
fc15-app         := NATIVEBLD=1 $(32GCC64)
# ---------------------------------------------------------------
# fc19 i686 (32bit)
fc19-dhd-opts    := LINUXVER=3.11.1-200.fc19.i686.PAE
fc19-dhd-opts    += CROSS_COMPILE=/tools/bin/
fc19-dhd         := $(fc19-dhd-opts) dhd-msgbuf-pciefd-debug
fc19-app         := NATIVEBLD=1 $(32GCC64)
# ---------------------------------------------------------------
# fc19 x64 (64bit)
fc19x64-dhd-opts    := LINUXVER=3.11.1-200.fc19.x86_64
fc19x64-dhd-opts    += CROSS_COMPILE=/tools/bin/
fc19x64-dhd     := $(fc19x64-dhd-opts) dhd-msgbuf-pciefd-debug dhd-msgbuf-pciefd-cfg80211-debug dhd-msgbuf-pciefd-pcieoob-debug
fc19x64-app         := NATIVEBLD=1 TARGETARCH=x86_64

#---------------------------------------------------------------
# fc19 x64 (64bit) wapi builds
fc19x64wapi-dhd-opts    := LINUXVER=3.11.1-200.fc19.x86_64.wapi
fc19x64wapi-dhd-opts    += CROSS_COMPILE=/tools/bin/
fc19x64wapi-dhd         := $(fc19x64wapi-dhd-opts)  dhd-msgbuf-pciefd-cfg80211-wapi-debug
fc19x64wapi-app         := NATIVEBLD=1 TARGETARCH=64

# ---------------------------------------------------------------
# fc22 x64 (64bit)
fc22x64-dhd-opts    := LINUXVER=4.0.4-301.fc22.x86_64
fc22x64-dhd-opts    += LINUXDIR=/tools/linux/src/linux-4.0.4-301.fc22.x86_64 \
			CROSS_COMPILE=/tools/linux/local/cross/rhel6-64/fc22/bin/x86_64-fc22.4.0.4.301-linux- GCCVER=5.1.1 \
			TARGET_OS=fc22 TARGET_ARCH=x86_64
fc22x64-dhd         := $(fc22x64-dhd-opts) dhd-msgbuf-pciefd-debug dhd-msgbuf-pciefd-pcieoob-debug
fc22x64-app         := NATIVEBLD=1 TARGETARCH=x86_64

# ---------------------------------------------------------------
# android x86 3.10 (64bit)
x86-3-10-args       := LINUXVER=3.10.11-aia1+-androidx86
x86-3-10-args       += CROSS_COMPILE=x86_64-linux-
x86-3-10-args       += ARCH=x86
x86-3-10-args       += PATH="/projects/hnd/tools/linux/hndtools-x86_64-linux-glibc2.7-4.6/bin/:$(PATH)"
x86-3-10-dhd        := $(x86-3-10-args)
x86-3-10-dhd        += dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-debug
x86android-app      := TARGETENV=android_ndk_r6b TARGETARCH=x86_android_ndk_r6b TARGETOS=unix
x86android-app      += TARGET_PREFIX=/projects/hnd_tools/linux/android-ndk-r6b/toolchains/x86-4.4.3/prebuilt/linux-x86/bin/i686-android-linux-
x86android-app      += TARGET_NDK=/projects/hnd_tools/linux/android-ndk-r6b LINUXDIR=/tools/linux/src/linux-3.0.3-panda

# FTDI Driver
# ---------------------------------------------------------------
# fc19 x64 (64bit)
fc19x64-ftdi-opts    := LINUXVER=3.11.1-200.fc19.x86_64
fc19x64-ftdi-opts    += CROSS_COMPILE=/tools/bin/
# ---------------------------------------------------------------
# fc22 x64 (64bit)
fc22x64-ftdi-opts    := LINUXVER=4.0.4-301.fc22.x86_64
fc22x64-ftdi-opts    += LINUXDIR=/tools/linux/src/linux-4.0.4-301.fc22.x86_64 \
                        CROSS_COMPILE=/tools/linux/local/cross/rhel6-64/fc22/bin/x86_64-fc22.4.0.4.301-linux- GCCVER=5.1.1 \
                        TARGET_OS=fc22 TARGET_ARCH=x86_64

FTDI_BULD_LIST  := fc19x64 fc22x64

# Cmd line args/options for wps subbuild (centralize references to tools used)
WPS_ARGS_bcm     := $(32GCC64PATH)
WPS_ARGS_bcmsglx := CC="arm-linux-gcc"
WPS_ARGS_bcmsglx += PATH="/projects/hnd/tools/linux/hndtools-arm-linux-3.4.3/bin:$(PATH)"

# From above list, choose os variants for each oem for host driver
DRIVERS_bcm          ?= fc15 fc19 fc19x64 fc19x64wapi fc22x64
DRIVERS_android      ?= x86-3-10

# From above list, choose os variants for each oem for apps
# WARNING:
# WARNING: Only one app variant per CPU arch type per OEM
# WARNING:
APPS_bcm             ?= fc15 fc19x64
APPS_android         ?= x86android

# From above list, choose os variants for each oem for hsl
HSL_bcm              ?=
HSL_android          ?= nexusgb

# Generic hardware-less DHD driver make targets
DHD_DRIVERS_bcm      := $(DRIVERS_bcm:%=%-dhd-bcm)
DHD_DRIVERS_voipbcm  := $(DRIVERS_voipbcm:%=%-dhd-voipbcm)
DHD_DRIVERS_android  := $(DRIVERS_android:%=%-dhd-android)

# App build targets for each oem
APP_DRIVERS_bcm      := $(APPS_bcm:%=%-app-bcm)
APP_DRIVERS_voipbcm  := $(APPS_voipbcm:%=%-app-voipbcm)
APP_DRIVERS_android  := $(APPS_android:%=%-app-android)

# HSL build targets for each oem
APP_HSL_bcm          := $(HSL_bcm:%=%-hsl-bcm)
APP_HSL_android      := $(HSL_android:%=%-hsl-android)

# FTDI OOB driver
OOB_DRV_PATH         := $(SRCDIR)/../components/opensource/ftdi_sio/

# SVN bom specification
ifneq ($(findstring media, $(BRAND)),)
	HNDSVN_BOM	:= dhd-usb-media.sparse
else
	HNDSVN_BOM	:= hndrte_WLM_WPS.sparse
endif # BRAND

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
#COMMONUNDEFS += SERDOWNLOAD	/* remove to enable the router donwload function: "rwl_download" */
COMMONUNDEFS += BCMSDIO
COMMONUNDEFS += DHD_AWDL
COMMONUNDEFS += WLAWDL

# additional UNDEFS for DHD build
DHD_UNDEFS += BCMCCX
DHD_UNDEFS += BCMEXTCCX

# additional UNDEFS for HSL build
#P2P_UNDEFS += BCMWAPI_WAI
#P2P_UNDEFS += BCMWAPI_WPI

# These symbols will be DEFINED in the source code by the mogrifier
#

# DEFS common to both DHD & HSL build
COMMONDEFS += BCM47XX
COMMONDEFS += BCM47XX_CHOPS

# additional DEFS for DHD build
#DEFS += BCMDONGLEHOST conflict with HSL build
#DHD_DEFS += DHDTHREAD

# additional DEFS for HSL build
P2P_DEFS += BCMCRYPTO
P2P_DEFS += SUPP
P2P_DEFS += BCMWPS
P2P_DEFS += DHCPD_APPS
P2P_DEFS += P2P
P2P_DEFS += PRE_SECFRW

UNDEFS = $(COMMONUNDEFS) $(DHD_UNDEFS) $(P2P_UNDEFS)
DEFS = $(COMMONDEFS) $(DHD_DEFS) $(P2P_DEFS)

ALLP2P_DEFS =  $(COMMONDEFS) $(P2P_DEFS)
ALLP2P_UNDEFS = $(COMMONUNDEFS) $(P2P_UNDEFS)
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
DEFS_bcm      += -DOEM_BCM -UOEM_MOTOROLA -UWLMOTOROLALJ -UWLNOKIA
ifndef MEDIA_BLD
	DEFS_bcm      += -UOEM_ANDROID
endif
DEFS_voipbcm  += -DOEM_BCM -UOEM_MOTOROLA -UWLMOTOROLALJ -UWLNOKIA
DEFS_voipbcm  += -UOEM_ANDROID
DEFS_nokia    += -UOEM_BCM -UOEM_MOTOROLA -UWLMOTOROLALJ -DWLNOKIA
DEFS_nokia    += -UOEM_ANDROID
DEFS_android  += -UOEM_BCM -UOEM_MOTOROLA -UWLMOTOROLALJ -UWLNOKIA
DEFS_android  += -DOEM_ANDROID -UBCMSPI -USDHOST3
DEFS_android  += -UBCMNVRAM -URWL_DONGLE -UUART_REFLECTOR -UBGBRD -UDHDARCH_MIPS
# Can't mogrify this out, breaks wl utility: DEFS_android  += -UD11AC_IOTYPES


# These are extensions of source files that should be mogrified
# (using the parameters specified in DEFS and UNDEFS above.)
#
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

all: build_start checkout mogrify filelists build_dongle_images copy_dongle_images prebuild_prep build_dhd build_ftdi_driver build_apps release build_end
#all: build_start checkout mogrify filelists prebuild_prep build_dhd build_apps release build_end

include linux-dongle-image-launch.mk

# Unreleased-chiplist is included only for non-internal builds

ifeq ($(findstring BCMINTERNAL,$(DEFS))$(findstring internal,$(BRAND)),)
include unreleased-chiplist.mk
endif # BCMINTERNAL

include $(MOGRIFY_RULES)
include $(BRAND_RULES)

DNGL_IMG_FILES += rtecdc.romlsim.trx

# check out files
checkout: $(CHECKOUT_TGT)

build_start:
	@$(MARKSTART_BRAND)

filelists:
	@$(MARKSTART)
	cat $(SRCFILELIST_ORG) $(SRCFILELISTS_COMPONENTS) | \
		src/tools/release/rem_comments.awk > master_filelist.h
	cat $(SRCFILELIST_HSL_ORG) $(SRCFILELISTS_HSL_COMPONENTS) | \
		src/tools/release/rem_comments.awk > master_filelist_hsl.h
	@$(MARKEND)

mogrify: checkout
	@$(MARKSTART)
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_DIRS=components
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
	mkdir -p build/$(oem)/components
	set -o pipefail; \
	    tar -C components --exclude=README --exclude=apps --exclude=opensource \
	        --exclude=tools --exclude=ucode --exclude=unittest --exclude-vcs -cf - . | \
	    tar -C build/$(oem)/components -xf -
	gcc -E -undef $(ALLDHD_DEFS:%=-D%_SRC) $(OEM_DEFS:%=-D%_SRC) \
		$(ALLDHD_UNDEFS:%=-DNO_%_SRC) $(OEM_UNDEFS:%=-DNO_%_SRC) \
		-Ulinux -o $(SRCFILELIST)_$(oem) master_filelist.h
	if [ "$(oem)" == "bcm" ]; then \
	    gcc -E -undef $(ALLP2P_DEFS:%=-D%_SRC) $(OEM_DEFS:%=-D%_SRC) \
		$(ALLP2P_UNDEFS:%=-DNO_%_SRC) $(OEM_UNDEFS:%=-DNO_%_SRC) \
		-Ulinux -o $(SRCFILELIST_HSL)_$(oem) master_filelist_hsl.h; \
	fi
	set -o pipefail; \
	if [ "$(oem)" == "bcm" ]; then \
	    sort -u $(SRCFILELIST)_$(oem) $(SRCFILELIST_HSL)_$(oem) > $(SRCFILELIST)_$(oem)_combo && \
	    $(FIND) src components $(FIND_SKIPCMD) -type f -print | $(SRCFILTER) -v $(SRCFILELIST)_$(oem)_combo | col -b | \
		$(GREP_SKIPCMD) | \
		pax -rw $(PAX_SKIPCMD) build/$(oem) && \
	    cd build/$(oem) && \
		$(FIND) src components $(FIND_SKIPCMD) -type f -print | $(SRCFILTER) -v ../../src/tools/release/$(SRCFILELIST_GPL) | col -b | \
		xargs -t -l1 $(MOGRIFY) -strip_comments; \
	else \
	    $(FIND) src components $(FIND_SKIPCMD) -type f -print | $(SRCFILTER) -v $(SRCFILELIST)_$(oem) | col -b | \
		$(GREP_SKIPCMD) | \
		pax -rw $(PAX_SKIPCMD) build/$(oem) && \
		if [ "$(oem)" == "android" ]; then \
		    $(FIND) src components $(FIND_SKIPCMD) -type f -print | \
			$(SRCFILTER) -v $(SRCFILELIST_HSL_android) | col -b | \
			$(GREP_SKIPCMD) | \
			pax -rw $(PAX_SKIPCMD) build/$(oem); \
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
build_dhd: $(foreach oem,$(OEM_LIST),build_$(oem)_dhd)

build_bcm_dhd: oem=bcm
build_bcm_dhd: $(DHD_DRIVERS_bcm)

build_voipbcm_dhd: oem=voipbcm
build_voipbcm_dhd: $(DHD_DRIVERS_voipbcm)

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
		$(opts) \
		$(BUILD_$(oem)_DHD_TARGETS)
	@$(MARKEND)

## ------------------------------------------------------------------
## Build oem brand specific apps
## ------------------------------------------------------------------
build_apps: $(foreach oem,$(OEM_LIST),build_$(oem)_apps)

build_ftdi_driver:
	@$(MARKSTART)
	set -e; $(foreach oem, $(FTDI_BULD_LIST), $(MAKE) -C $(OOB_DRV_PATH) $($(oem)-ftdi-opts);)
	@$(MARKEND)

build_bcm_apps: oem=bcm
build_bcm_apps: $(APP_DRIVERS_bcm)

build_voipbcm_apps: oem=voipbcm
build_voipbcm_apps: $(APP_DRIVERS_voipbcm)

build_android_apps: oem=android
build_android_apps: $(APP_DRIVERS_android)

#- Common app build steps for all oems
$(foreach oem,$(OEM_LIST),$(APP_DRIVERS_$(oem))):: opts=$($(subst -$(oem),,$@))
$(foreach oem,$(OEM_LIST),$(APP_DRIVERS_$(oem)))::
	@$(MARKSTART)
	$(MAKE) -C build/$(oem)/src/wl/exe \
		ASD=1 \
		$(if $(BCM_MFGTEST), WLTEST=1) $(opts)
	$(MAKE) -C build/$(oem)/src/dhd/exe \
		REPORT_FATAL_TIMEOUTS=1 \
		$(if $(BCM_MFGTEST), WLTEST=1) $(opts)
	# wlm fails to cross compile on arm, forcing x86 only
	# $(MAKE) -C build/$(oem)/src/wl/exe wlm $($@)
	$(MAKE) -C build/$(oem)/src/wl/exe wlm $(32GCC64)
	@$(MARKEND)

# Build HSL apps
build_hsl: $(foreach oem,$(OEM_LIST),build_$(oem)_hsl)

# First create dummy targets per oem type for hsl release
$(foreach oem,$(OEM_LIST),build_$(oem)_hsl):

# Then secondary build_<oem>_hsl for selective oem do build
build_bcm_hsl: oem=bcm
build_bcm_hsl: $(APP_HSL_bcm)

build_android_hsl:
build_android_hsl:
## ------------------------------------------------------------------
## Build HSL for all oems
## ------------------------------------------------------------------

#- Common hsl build steps for all oems
$(foreach oem,$(OEM_LIST),$(APP_HSL_$(oem))): opts=$($(subst -$(oem),,$@))
$(foreach oem,$(OEM_LIST),$(APP_HSL_$(oem))):
	@$(MARKSTART)
	@if [ "$(opts)" == "" ]; then \
	    echo "ERROR: $@ hsl options missing"; \
	    echo "ERROR: Check APP_HSL_$(oem) specification above"; \
	    exit 1; \
	else \
		pushd build/$(oem)/src/wps/linux/enr; \
		$(MAKE) -f wps_enr_app.mk $($@) $(BUILD_$(oem)_P2P_TARGETS) ; \
		popd; \
	fi
	@$(MARKEND)

#- bcm specific app build
$(APP_DRIVERS_bcm)::
	@$(MARKSTART)
	@$(MARKEND)

# For external brands, build android zImage
# TO-DO: Need some routine cleanup, like defining macros for
# TO-DO: long paths. Replace hardcoded android references with $(oem)

ifeq ($(BRAND),linux-external-dongle-pcie)
build_bcmdhd_all: oem=android
build_bcmdhd_all: copy_linux_kernel_code link_bcmdhd_src build_bcmdhd release_bcmdhd_src delete_linux_kernel

# TO-DO: This needs to be replaced with git sync command
copy_linux_kernel_code: checkout mogrify
	@$(MARKSTART)
	@for kernel_src in $(LINUX_KERNEL_SRC_BCMDHD); \
	do \
		echo "cp -dPR /tools/linux/src/$$kernel_src $(BUILD_BASE)"; \
		cp -dPR /tools/linux/src/$$kernel_src $(BUILD_BASE); \
	done
	@$(MARKEND)

# TO-DO: Maintenance of svn-external-filelist.txt may be a challenge
# TO-DO: to keep in sync with src/dhd/linux/Makefile SOURCES list
# TO-DO: Rename the file and see if it can be generated dynamically
# TO-DO: From CFILES and VPATH
link_bcmdhd_src: copy_linux_kernel_code
	@$(MARKSTART)
	@for kernel_src in $(LINUX_KERNEL_SRC_BCMDHD); \
	do \
		rm -rfv $(BUILD_BASE)/$$kernel_src/drivers/net/wireless/bcmdhd; \
		mkdir -pv $(BUILD_BASE)/$$kernel_src/drivers/net/wireless/bcmdhd/include/proto; \
		install -pv $(SRCDIR)/bcmdhd/Kconfig $(BUILD_BASE)/$$kernel_src/drivers/net/wireless/bcmdhd; \
		if [ "$$kernel_src" == "linux-3.0.8-maguro-JB" ]; then \
	        install -pv $(SRCDIR)/bcmdhd/Makefile_jb $(BUILD_BASE)/$$kernel_src/drivers/net/wireless/bcmdhd/Makefile; \
		else \
            install -pv $(SRCDIR)/bcmdhd/Makefile_ics $(BUILD_BASE)/$$kernel_src/drivers/net/wireless/bcmdhd/Makefile; \
	    fi;\
		for original_file_path in `cat $(SRCDIR)/bcmdhd/svn-external-filelist.txt | awk '{print $$1}'`; \
		do \
			link_file_path=`cat $(SRCDIR)/bcmdhd/svn-external-filelist.txt | grep $$original_file_path | awk '{print $$2}'`; \
			cp -dPus $(BUILD_BASE)/build/android/$$original_file_path $(BUILD_BASE)/$$kernel_src/drivers/net/wireless/bcmdhd/$$link_file_path; \
		done; \
	done
	@$(MARKEND)

# TO-DO: Replace build.sh with actual $(MAKE) commands
build_bcmdhd: link_bcmdhd_src
	@$(MARKSTART)
	@for kernel_src in $(LINUX_KERNEL_SRC_BCMDHD); \
	do \
		cd $(BUILD_BASE)/$$kernel_src/drivers/net/wireless/bcmdhd/include; \
		$(MAKE); \
		cd $(BUILD_BASE)/$$kernel_src; \
		./build.sh; \
		mkdir -pv $(RELEASEDIR)/android/host/zImage-$$kernel_src; \
		cp -fpuv $(BUILD_BASE)/$$kernel_src/arch/arm/boot/zImage $(RELEASEDIR)/android/host/zImage-$$kernel_src/; \
	done
	@$(MARKEND)

release_bcmdhd_src: build_bcmdhd
	@$(MARKSTART)
	mkdir -pv $(RELEASEDIR)/android/dongle-host-driver-source-open-kernel-build/bcmdhd/include/proto
	@for original_file_path in `cat $(SRCDIR)/bcmdhd/svn-external-filelist.txt | awk '{print $$1}'`; \
	do \
		link_file_path=`cat $(SRCDIR)/bcmdhd/svn-external-filelist.txt | grep $$original_file_path | awk '{print $$2}'`; \
		cp -fpuv $(BUILD_BASE)/$$original_file_path $(RELEASEDIR)/android/dongle-host-driver-source-open-kernel-build/bcmdhd/$$link_file_path; \
	done
	cp -fv $(SRCDIR)/include/epivers.h $(RELEASEDIR)/android/dongle-host-driver-source-open-kernel-build/bcmdhd/include/epivers.h; \
	cd  $(RELEASEDIR)/android/dongle-host-driver-source-open-kernel-build/; \
	$(FIND) bcmdhd -type d |  xargs rmdir -p --ignore-fail-on-non-empty
	cd $(RELEASEDIR)/android/dongle-host-driver-source-open-kernel-build/; \
	   $(FIND) bcmdhd | $(SRCFILTER) -v android-gpl-filelist.txt | col -b | \
		xargs -t -l1 $(MOGRIFY) $(DEFS_$(oem)) \
		-DESTA_POSTMOGRIFY_REMOVAL -DLINUX_POSTMOGRIFY_REMOVAL -strip_comments
	install -pv $(SRCDIR)/bcmdhd/Kconfig $(RELEASEDIR)/android/dongle-host-driver-source-open-kernel-build/bcmdhd
	install -pv $(SRCDIR)/bcmdhd/Makefile $(RELEASEDIR)/android/dongle-host-driver-source-open-kernel-build/bcmdhd
	tar cpf $(RELEASEDIR)/android/dongle-host-driver-source-open-kernel-build.tar -C $(RELEASEDIR)/android dongle-host-driver-source-open-kernel-build
	gzip -9 $(RELEASEDIR)/android/dongle-host-driver-source-open-kernel-build.tar
	rm -rfv $(RELEASEDIR)/android/dongle-host-driver-source-open-kernel-build
	@$(MARKEND)

delete_linux_kernel: release_bcmdhd_src
	@$(MARKSTART)
	@for kernel_src in $(LINUX_KERNEL_SRC_BCMDHD); \
	do \
		echo "rm -rf $(BUILD_BASE)/$$kernel_src"; \
		rm -rf $(BUILD_BASE)/$$kernel_src; \
	done
	@date +"END:   $@, %D %T"  | tee -a profile.log

else # BRAND

# For non-external brands, don't build android zImage
build_bcmdhd_all:

endif # BRAND

release_hsl_android:

release_hsl_bcm:

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
	install -d -m 755 $(RELDIR)/firmware
	install -d -m 755 $(RELDIR)/host
	install -d -m 755 $(RELDIR)/apps
	install -d -m 755 $(RELDIR)/nvram
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	# Copy README and release notes (oem specific ones override generic)
#	touch $(RELDIR)/README.txt
#	-install src/doc/BrandReadmes/$(BRAND).txt $(RELDIR)/README.txt
#	-install src/doc/BrandReadmes/$(BRAND)_oem_$(oem).txt $(RELDIR)/README.txt
	# Copy over host kernel modules
	@cd build/$(oem); \
	shopt -s nullglob; for mdir in src/dhd/linux/dhd-msgbuf-*; do \
	    mkdir -p -v $(RELEASEDIR)/$(oem)/host/$$(basename $${mdir}); \
            for mod in dhd.o dhd.ko bcmdhd.ko; do \
                if [ -s "$$mdir/$${mod}.stripped" ]; then \
                   install -pv $$mdir/$${mod}.stripped \
                        $(RELEASEDIR)/$(oem)/host/$$(basename $${mdir})/$${mod}; \
                fi; \
            done; \
	done; \
	# install FTDI drivers
	shopt -s nullglob; for mdir in $(OOB_DRV_PATH)/ftdi-*; do \
		install -Dpv $${mdir}/ftdi_sio_brcm.ko $(RELEASEDIR)/$(oem)/host/$$(basename $${mdir})/ftdi_sio_brcm.ko;\
	done; \
	@echo "Copying nvram files to nvram folder now"; \
                install -pv $(SRCDIR)/shared/nvram/bcm94345*.txt $(RELDIR)/nvram; \
                install -pv $(SRCDIR)/shared/nvram/bcm94350*.txt $(RELDIR)/nvram; \
                install -pv $(SRCDIR)/shared/nvram/BRCM_IP_TAG.txt $(RELDIR)/nvram;

	# Copying dongle images to both build/<oem> and release/<oem> folders
	@echo "Copying $(RLS_IMAGES_$(oem)) to firmware folder now"; \
	failedimgs=""; \
	echo "Copying $(RLS_IMAGES_$(oem)) to firmware folder now"; \
	for img in $(RLS_IMAGES_$(oem)); do \
	   if [ ! -s "$(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin" -o \
	        ! -s "$(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).h" ]; \
	   then \
	      echo "WARN: Dongle image $${img} NOT BUILT"; \
	      failedimgs="  $${failedimgs} $${img}\n"; \
	   else \
	      echo "---------------------------------------------------"; \
	      echo "Copying $${img} image files:"; \
	      echo "---------------------------------------------------"; \
	      install -dv  build/$(oem)/$(DNGL_IMGDIR)/$${img}; \
	      install -pv  $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin \
	                   $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).h \
	                   build/$(oem)/$(DNGL_IMGDIR)/$${img}/; \
	      install -pDv $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin \
	                   $(RELDIR)/firmware/$${img}.bin; \
	      install -pDv $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).h \
	                   $(RELDIR)/firmware/$${img}.h; \
	      if [ -s "$(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).romlsim.trx" ]; then \
	         install -pDv $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).romlsim.trx \
		    	   $(RELDIR)/firmware/$${img}.romlsim.trx; \
	      fi; \
	   fi; \
	done; \
	if [ "$${failedimgs}" != "" ]; then \
	   echo "ERROR: Following dongle images failed to build"; \
	   echo -e "\n$${failedimgs}"; \
	   grep -B 3 "Error: image exceeds size limit" \
		$(HNDRTE_RLSLOG) || true; \
	   echo "ERROR: Verify buildlog file:  $(HNDRTE_RLSLOG)"; \
	   exit 1; \
	fi
	# Copying native and other cross-compiled wl and dhd exe apps
	@echo "Copying native and cross compiled apps for $(foreach app,$(APP_DRIVERS_$(oem)),$(subst -$(oem),,$(app))) variants"
	for arch in $(foreach app,$(APP_DRIVERS_$(oem)),$(if $(filter TARGETARCH%, $($(patsubst %-$(oem),%,$(app)))),$(patsubst TARGETARCH=%,%,$(filter TARGETARCH%, $($(patsubst %-$(oem),%,$(app))))),x86)); do \
		if [ -d build/$(oem)/src/wl/exe/obj/wl/$$arch ]; then \
			if [ "$$arch" == "x86" ]; then \
				$(MAKE) -C build/$(oem)/src/wl/exe ASD=1 TARGETARCH=$$arch INSTALL_DIR=$(RELEASEDIR)/$(oem)/apps release_bins; \
				install -pv build/$(oem)/src/dhd/exe/dhd $(RELDIR)/apps; \
			else \
				mkdir -pv $(RELDIR)/apps/$$arch; \
				install -pv build/$(oem)/src/wl/exe/wl$$arch $(RELDIR)/apps/$$arch/wl; \
				install -pv build/$(oem)/src/wl/exe/socket/$$arch/wl_server_socket$$arch $(RELDIR)/apps/$$arch/wl_server_socket; \
				install -pv build/$(oem)/src/wl/exe/dongle/$$arch/wl_server_dongle$$arch $(RELDIR)/apps/$$arch/wl_server_dongle; \
				install -pv build/$(oem)/src/wl/exe/serial/$$arch/wl_server_serial$$arch $(RELDIR)/apps/$$arch/wl_server_serial; \
				install -pv build/$(oem)/src/wl/exe/wifi/$$arch/wl_server_wifi$$arch $(RELDIR)/apps/$$arch/wl_server_wifi; \
				install -pv build/$(oem)/src/dhd/exe/dhd$$arch $(RELDIR)/apps/$$arch/dhd; \
			fi; \
		fi; \
	done
	# wlm fails to cross compile on arm, forcing x86 only
	install -p build/$(oem)/src/wl/exe/wlm/x86/wlm.so $(RELDIR)/apps
	install src/doc/BCMLogo.gif $(RELEASEDIR)/$(oem)
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
# 	#End of release_$(oem)_bins
	@$(MARKEND)

#- Bcm specific extra release files needed
release_bcm_extra_bins: RELDIR=release/bcm
release_bcm_extra_bins:
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
		src/doc/ReleaseNotesDongleSdio.html > \
		$(RELDIR)/ReleaseNotes.htm
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
# 	#End of release_bcm_extra_bins
	@$(MARKEND)

#- VoIPBcm specific extra release files needed
release_voipbcm_extra_bins:
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,release/voipbcm)
	sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
		src/doc/ReleaseNotesDongleSdio.html > \
		release/voipbcm/ReleaseNotes.htm
	$(call REMOVE_WARN_PARTIAL_BUILD,release/voipbcm)
# 	#End of release_voipbcm_extra_bins
	@$(MARKEND)

#- Nokia specific extra release files needed
release_nokia_extra_bins:
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,release/nokia)
	$(call REMOVE_WARN_PARTIAL_BUILD,release/nokia)
# 	#End of release_nokia_extra_bins
	@$(MARKEND)

#- Android specific extra release files needed
release_android_extra_bins:
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,release/android)
	shopt -s nullglob; \
	shopt -s extglob; \
	$(call REMOVE_WARN_PARTIAL_BUILD,release/android)
	@$(MARKEND)

## ------------------------------------------------------------------
## Create oem brand specific source releases
## ------------------------------------------------------------------

# This is a convinience target for open source pacakge testing, not called by anyone directly
release_open_src: $(foreach oem,$(OEM_LIST_OPEN_SRC),release_$(oem)_open_src_package)

release_src: $(foreach oem,$(OEM_LIST),release_$(oem)_src_package) \
             $(foreach oem,$(OEM_LIST_OPEN_SRC),release_$(oem)_open_src_package) \
             $(foreach oem,$(OEM_LIST),release_$(oem)_hsl_src_package)

$(foreach oem,$(OEM_LIST),release_$(oem)_src_package): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),release_$(oem)_src_package): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),release_$(oem)_src_package):
	@$(MARKSTART)
	mkdir -p $(RELDIR)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	@echo "Generating release source + binaries package now in $(RELDIR) ...."
	# First create release/<oem>/src from SRCFILTER_<oem> src filter
ifdef MEDIA_BLD
	tar cpf - $(TAR_SKIPCMD) -C build/$(oem) \
	         `$(FIND) src components $(FIND_SKIPCMD) -print | $(SRCFILTER) -v $(SRCFILELIST)_$(oem)_combo` \
	         `cd build/$(oem); $(FIND) $(DNGL_IMGDIR)/* -type f \
	        -name "*$(DNGL_IMG_PFX).h" -o -name "*$(DNGL_IMG_PFX).bin"` | \
		 tar xpf - $(TAR_SKIPCMD) -C $(RELDIR)
else
	tar cpf - $(TAR_SKIPCMD) -C build/$(oem) \
	         `$(FIND) src components $(FIND_SKIPCMD) -print | $(SRCFILTER) -v $(SRCFILELIST)_$(oem)` \
	         `cd build/$(oem); $(FIND) $(DNGL_IMGDIR)/* -type f \
			-name "*$(DNGL_IMG_PFX).h" -o -name "*$(DNGL_IMG_PFX).bin"` | \
		 tar xpf - $(TAR_SKIPCMD) -C $(RELDIR)
endif # MEDIA_BLD
	@if [ "$(oem)" == "android" ]; then \
		install -pv src/Android.mk $(RELDIR)/src/Android.mk; \
	fi
#	tar cpf $(RELDIR)/dongle-host-driver-source.tar -C $(RELDIR) $(TAR_SKIPCMD) \
	         src components firmware host apps README.txt BCMLogo.gif
	tar cpf $(RELDIR)/dongle-host-driver-source.tar -C $(RELDIR) $(TAR_SKIPCMD) \
	         src components firmware host apps BCMLogo.gif
#	@if [ -s "$(RELDIR)/$(notdir $(PXA270-kernelPKG))" ]; then \
#	    echo "Including $(notdir $(PXA270-kernelPKG)) in $(oem) package"; \
#	    tar uvpf $(RELDIR)/dongle-host-driver-source.tar -C $(RELDIR) $(TAR_SKIPCMD) \
#	         $(notdir $(PXA270-kernelPKG)); \
#	fi
	gzip -9 $(RELDIR)/dongle-host-driver-source.tar
ifdef BCM_MFGTEST
	@echo "Generating release binaries package now in $(RELDIR) ...."
	tar cpf $(RELDIR)/dongle-host-driver-binary.tar -C $(RELDIR) $(TAR_SKIPCMD) \
	         firmware host apps  BCMLogo.gif
	gzip -9 $(RELDIR)/dongle-host-driver-binary.tar
endif # BCM_MFGTEST
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)
# 	#End of release_$(oem)_src_package


# Validate open-src source package again after cleanup
define VALIDATE_OPEN_SRC_PACKAGE_AFTER_CLEANUP
        echo "$(MAKE) -C $(RELDIR)/open-src/src/dhd/linux \
        $(if $(BCM_MFGTEST), WLTEST=1) $1"; \
                $(MAKE) -C $(RELDIR)/open-src/src/dhd/linux \
        $(if $(BCM_MFGTEST), WLTEST=1) \
                $1; makerc2=$$?; \
        echo "dhd.ko exit code=$$makerc2"; \
        if [ "$$makerc2" != "0" ]; then \
                echo "Open Source post build validation failed"; \
        exit 1; \
        fi
endef # VALIDATE_OPEN_SRC_PACKAGE_AFTER_CLEANUP


# Additional steps to create a open gpl package for certain oem brands
# Currently bcm and android need this special open-src source package
$(OEM_LIST_OPEN_SRC:%=release_%_open_src_package): oem=$(shell echo $(@F) | cut -d_ -f2)
$(OEM_LIST_OPEN_SRC:%=release_%_open_src_package): RELDIR=release/$(oem)
$(OEM_LIST_OPEN_SRC:%=release_%_open_src_package): OEM_DEFS= $(patsubst -D%,%,$(filter -D%,$(DEFS_$(oem))))
$(OEM_LIST_OPEN_SRC:%=release_%_open_src_package): OEM_UNDEFS= $(patsubst -U%,%,$(filter -U%,$(DEFS_$(oem)))) ESTA_OPENSRC_CLEANUP_LIST
$(OEM_LIST_OPEN_SRC:%=release_%_open_src_package):
ifeq ($(BRAND),linux-external-dongle-pcie)
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	@echo "Generating open source + binaries package now in $(RELDIR)"
	rm -rf $(RELDIR)/open-src
	mkdir -p $(RELDIR)/open-src
	# Copy all previous sources used for dhd/apps builds
	cp -rp $(RELDIR)/src $(RELDIR)/open-src/
	# Remove selected files by filelist
	cp $(SRCFILELIST)_$(oem) $(RELDIR)/open-src/$(SRCFILELIST)_$(oem)_open-src
	cd $(RELDIR)/open-src; \
	   rm -rfv `$(FIND) src components | $(SRCFILTER) -v $(SRCFILELIST)_$(oem)_open-src`
	# Cleanup any other additional non-opensrc list of files
	# We do not release wl.exe binary or its sources
	# we also exclude p2p related file.
	gcc -E -undef $(ALLDHD_DEFS:%=-D%_SRC) $(OEM_DEFS:%=-D%_SRC) \
		$(ALLDHD_UNDEFS:%=-DNO_%_SRC) $(OEM_UNDEFS:%=-DNO_%_SRC) \
		-Ulinux -o $(SRCFILELIST)_$(oem)_open-src master_filelist.h
	# $(MOGRIFY) -UESTA_OPENSRC_CLEANUP_LIST $(SRCFILELIST)_$(oem)
	# On selected files, copy fresh sources for translating
	# open to dual copyright license
	cp $(SRCFILELIST)_$(oem)_open-src $(RELDIR)/open-src/
	$(FIND) src components | $(SRCFILTER) -v $(SRCFILELIST)_$(oem)_open-src | col -b | \
		cpio -pd $(RELDIR)/open-src
	rm -f $(SRCFILELIST)_$(oem)_open-src
	cd  $(RELDIR)/open-src; \
	$(FIND) src components -type d |  xargs rmdir -p --ignore-fail-on-non-empty

	# Translating copyright text 'open' to 'dual' and remove BCMROMFN, etc.
	$(MAKE) -f $(MOGRIFY_RULES) \
		MOGRIFY_DIRS=$(RELDIR)/open-src/src \
		MOGRIFY_FLAGS="$(DEFS_$(oem)) -translate_open_to_dual_copyright -strip_bcmromfn"
	# Stripping comments some selective open-src files
	cp $(SRCFILELIST_GPL) $(RELDIR)/open-src
	cd $(RELDIR)/open-src; \
	   $(FIND) src components | $(SRCFILTER) -v $(SRCFILELIST_GPL) | col -b | \
		xargs -t -l1 $(MOGRIFY) $(DEFS_$(oem)) \
		-DESTA_POSTMOGRIFY_REMOVAL -DLINUX_POSTMOGRIFY_REMOVAL -strip_comments
	# Copy android specific config files
	@if [ "$(oem)" == "android" ]; then \
		install -dv $(RELDIR)/open-src/config; \
		install -pv src/dhd/android/Android.mk $(RELDIR)/open-src; \
	fi

	# Validate android open-src source package again after cleanup
	@if [ "$(oem)" == "android" ]; then \
		$(call VALIDATE_OPEN_SRC_PACKAGE_AFTER_CLEANUP, $(cupcake-dhd)); \
	fi; \

	# Cleanup dhd bits after validation open-src source package
	@if [ "$(oem)" == "android" -o "$(oem)" == "bcm" ]; then \
		echo "$(MAKE) -C $(RELDIR)/open-src/src/dhd/exe clean"; \
		$(MAKE) -C $(RELDIR)/open-src/src/dhd/exe clean; \
		echo "$(MAKE) -C $(RELDIR)/open-src/src/dhd/linux clean"; \
		$(MAKE) -C $(RELDIR)/open-src/src/dhd/linux clean; \
	fi
	# Built dhd bits are already picked in previous target
	# Exclude apps/wl*, we do not release wl.exe binary or its sources
	tar cpf $(RELDIR)/dongle-host-driver-source-open.tar $(TAR_SKIPCMD) \
		-C $(RELDIR) --exclude=apps/wl* open-src/src firmware \
		host apps BCMLogo.gif
	gzip -9 $(RELDIR)/dongle-host-driver-source-open.tar
	# Remove non open-src packages for android
	@if [ "$(oem)" == "android" ]; then \
	    rm -fv $(RELDIR)/dongle-host-driver-source.tar.gz; \
	    rm -rf $(RELDIR)/src; \
	fi
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)
endif # BRAND
# 	#End of release_$(oem)_open_src_package

$(foreach oem,$(OEM_LIST),release_$(oem)_hsl_src_package): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),release_$(oem)_hsl_src_package): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),release_$(oem)_hsl_src_package):

release_bcm_hsl_src_package:

#        $(call WARN_PARTIAL_BUILD,$(RELDIR))
#        @echo "Generating hsl release source + binaries package now in $(RELDIR)/hsl_src ...."
#        rm -rf $(RELDIR)/hsl_pkg_list $(RELDIR)/hsl_src
#        mkdir $(RELDIR)/hsl_src
#
#        # copy all p2p sources and dongle binaries and headers
#        tar cpf - $(TAR_SKIPCMD) -C build/$(oem) \
#        	`$(FIND) src | $(SRCFILTER) -v $(SRCFILELIST_HSL)_$(oem)` \
#        	`cd build/$(oem); $(FIND) $(DNGL_IMGDIR)/* -type f -name \
#        		"*$(DNGL_IMG_PFX).h" -o -name "*$(DNGL_IMG_PFX).bin"` | \
#        		tar xpf - $(TAR_SKIPCMD) -C $(RELDIR)/hsl_src
#        # copy all p2p binaries
#        tar cpf - $(TAR_SKIPCMD) -C $(RELDIR) `cd $(RELDIR); $(FIND) apps/hsl-* -print` | tar xpf - $(TAR_SKIPCMD) -C $(RELDIR)/hsl_src
#
#        # Create 'hsl_pkg_list' list of files. Exclude intermediate bits
#        -cd $(RELDIR)/hsl_src; \
#        	find src apps $(FIND_SKIPCMD) -type f ! -name "*.o" -a ! -name "*.d" -print  >> \
#        	../hsl_pkg_list
#        tar cpf $(RELDIR)/linux-hsl-source-$(RELNUM).tar -C $(RELDIR)/hsl_src $(TAR_SKIPCMD) \
#                 -T $(RELDIR)/hsl_pkg_list
#        gzip -9v $(RELDIR)/linux-hsl-source-$(RELNUM).tar
#        $(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
## 	#End of release_$(oem)_hsl_src_package

## ------------------------------------------------------------------
## Generate documentation and package it
## ------------------------------------------------------------------
release_doc: $(OEM_LIST:%=release_%_doc_package)

$(OEM_LIST:%=release_%_doc_package): oem=$(shell echo $(@F) | cut -d_ -f2)
$(OEM_LIST:%=release_%_doc_package): RELDIR=release/$(oem)
$(OEM_LIST:%=release_%_doc_package):

release_build_clean: $(foreach oem,$(OEM_LIST),release_$(oem)_clean)

$(foreach oem,$(OEM_LIST),release_$(oem)_clean): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),release_$(oem)_clean):
	@$(MARKSTART)
	@if [ -f "$(HNDRTE_FLAG)"   ]; then rm -fv  $(HNDRTE_FLAG);   fi
	@if [ -f "wps_enrollee.tgz" ]; then rm -fv  wps_enrollee.tgz; fi
	@if [ -d "build/$(oem)"     ]; then \
	    echo "rm -rf build/$(oem)"; \
	    rm -rf build/$(oem); \
	    if [ "$(wildcard build/*)" == "build/$(oem)" ]; then \
	       rmdir -v build; \
	    fi; \
	fi
	@$(MARKEND)

release: release_bins integrate_firmware release_src release_build_clean
#release: release_bins integrate_firmware release_doc release_build_clean

# Integrate firmwares into release directory
integrate_firmware:
	@$(MARKSTART)
	@echo "Copying images from firmware release"
	src/tools/misc/combine_release.sh $(BRAND) AARDVARK_TWIG_6_30_235 $(TAG)
	@$(MARKEND)

build_end:
	@rm -fv $(HNDRTE_FLAG)
	@$(MARKEND_BRAND)

is32on64: # for build_config consumption
	@echo "$(BRAND) can be build on a 64 bit linux cluster"

.PHONY: FORCE checkout build_include build_end build_dongle_images prebuild_prep build_dhd build_apps build_ftdi_driver_fc19_64 build_hsl $(foreach oem,$(OEM_LIST),build_$(oem)_dhd) $(foreach oem,$(OEM_LIST),build_$(oem)_apps) release release_bins release_src $(foreach oem,$(OEM_LIST),release_$(oem)_src_package) $(foreach oem,$(OEM_LIST_OPEN_SRC),release_$(oem)_open_src_package) $(foreach oem,$(OEM_LIST),release_$(oem)_bins release_$(oem)_extra_bins) $(foreach oem,$(OEM_LIST),release_$(oem)_hsl_src_package) is32on64 integrate_firmware
