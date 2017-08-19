#
# Common makefile to build Linux CDC DHD and SDIO Dongle Image(on linux).
# Package DHD sources with all sdio dongle images
#
# NOTE: This makefile should not be used by itself, it is included into
# other brands like linux-external-dongle-sdio.mk
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
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
OEM_LIST           ?= bcm
WPA_SUPP_TAG       ?= WPA_SUPPLICANT-0-8_00_56
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
SRCFILELISTS_COMPONENTS += src/tools/release/components/wlolpc-filelist.txt
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

SRCFILELIST_ORG    := $(SRCDIR)/tools/release/linux-dhd-sdio-filelist.txt
SRCFILELIST_GPL_ORG:= $(SRCDIR)/tools/release/linux-dhd-sdio-gpl-filelist.txt
SRCFILELIST        := linux-dhd-sdio-filelist.txt
SRCFILELIST_GPL    := linux-dhd-sdio-gpl-filelist.txt
SRCFILELIST_HSL_ORG:=$(SRCDIR)/tools/release/linux-p2plib-filelist.txt
SRCFILELIST_HSL    := hsl_master_filelist


# OEM and HSL flags are per OEM specific, hence derived in OEM specific
# targets
GCCFILESDEFS_OEM =
GCCFILESDEFS_HSL =
GCCFILESDEFS_GPL = $(DEFS:%=-D%) -DESTA_OPENSRC_CLEANUP_LIST $(UNDEFS:%=-U%)

VALID_BRANDS := \
	linux-external-dongle-sdio \
	linux-internal-dongle-sdio \
	linux-internal-dongle \
	linux-mfgtest-dongle-sdio

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

## Some of these macros can be overridden by calling brand makefiles

# Other images (in addition to EMBED_IMAGE_<oem>) to be released to customer
# come from RLS_IMAGE_><oem> in brand makefile.
# WARN: Do not add any dongle images to following list. Instead, update
#       appropriate <brand>_ release makefiles.

RLS_IMAGES_bcm      += $(EMBED_IMAGE_bcm)

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

#---------------------------------------------------------------
# fc15 i686 (32bit)
fc15-dhd-opts    := LINUXVER=2.6.38.6-26.rc1.fc15.i686.PAE
fc15-dhd-opts    += CROSS_COMPILE=/tools/bin/
fc15-dhd         := $(fc15-dhd-opts) dhd-cdc-sdstd dhd-cdc-sdstd-debug \
			 dhd-cdc-sdstd-apsta dhd-cdc-sdmmc-gpl dhd-cdc-sdstd-hc3
fc15-app         := NATIVEBLD=1 $(32GCC64)
# ---------------------------------------------------------------
# fc19 i686 (32bit)
fc19-dhd-opts    := LINUXVER=3.11.1-200.fc19.i686.PAE
fc19-dhd-opts    += CROSS_COMPILE=/tools/bin/
fc19-dhd         := $(fc19-dhd-opts) dhd-cdc-sdstd dhd-cdc-sdstd-debug \
			 dhd-cdc-sdstd-apsta dhd-cdc-sdmmc-gpl dhd-cdc-sdstd-hc3
fc19-app         := NATIVEBLD=1 $(32GCC64)
# ---------------------------------------------------------------
# fc18 i686 (32bit)
# Options to host-side for updated Fedora Core 18
fc18-dhd-opts    := LINUXVER=3.6.10-4.fc18.i686
fc18-dhd-opts    += CROSS_COMPILE=/tools/bin/
fc18-dhd         := $(fc18-dhd-opts) dhd-cdc-sdstd dhd-cdc-sdstd-debug \
			 dhd-cdc-sdstd-apsta dhd-cdc-sdmmc-gpl dhd-cdc-sdstd-hc3
fc18-app         := NATIVEBLD=1 $(32GCC64)
#---------------------------------------------------------------
# fc19 x64 (64bit)
fc19x64-dhd-opts    := LINUXVER=3.11.1-200.fc19.x86_64
fc19x64-dhd-opts    += CROSS_COMPILE=/tools/bin/
fc19x64-dhd         := $(fc19x64-dhd-opts) dhd-cdc-sdstd dhd-cdc-sdstd-debug \
			 dhd-cdc-sdstd-apsta dhd-cdc-sdmmc-gpl dhd-cdc-sdstd-hc3
fc19x64-app         := NATIVEBLD=1 TARGETARCH=x86_64

# Cmd line args/options for wps subbuild (centralize references to tools used)
WPS_ARGS_bcm     := $(32GCC64PATH)
WPS_ARGS_bcmsglx := CC="arm-linux-gcc"
WPS_ARGS_bcmsglx += PATH="/projects/hnd/tools/linux/hndtools-arm-linux-3.4.3/bin:$(PATH)"

# From above list, choose os variants for each oem for host driver
DRIVERS_bcm          ?= fc15 fc18 fc19 fc19x64

# From above list, choose os variants for each oem for apps
# WARNING:
# WARNING: Only one app variant per CPU arch type per OEM
# WARNING:
APPS_bcm             ?= fc15 fc19x64

# From above list, choose os variants for each oem for hsl
HSL_bcm              ?=

# Generic hardware-less DHD driver make targets
DHD_DRIVERS_bcm      := $(DRIVERS_bcm:%=%-dhd-bcm)

# App build targets for each oem
APP_DRIVERS_bcm      := $(APPS_bcm:%=%-app-bcm)

# HSL build targets for each oem
APP_HSL_bcm          := $(HSL_bcm:%=%-hsl-bcm)

# SVN bom specification
HNDSVN_BOM            := hndrte_WLM_WPS.sparse

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
COMMONUNDEFS += DHD_AWDL
COMMONUNDEFS += WLAWDL

# additional UNDEFS for DHD build
#DHD_UNDEFS +=

# additional UNDEFS for HSL build
#P2P_UNDEFS += BCMWAPI_WAI
#P2P_UNDEFS += BCMWAPI_WPI

# These symbols will be DEFINED in the source code by the mogrifier
#

# DEFS common to both DHD & HSL build
COMMONDEFS += BCM47XX
COMMONDEFS += BCM47XX_CHOPS
COMMONDEFS += BCMSDIO

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
DEFS_bcm      += -DOEM_BCM -UWLNOKIA
DEFS_bcm      += -UOEM_ANDROID
DEFS_nokia    += -UOEM_BCM -DWLNOKIA
DEFS_nokia    += -UOEM_ANDROID


# These are extensions of source files that should be mogrified
# (using the parameters specified in DEFS and UNDEFS above.)
#
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

all: build_start checkout mogrify filelists build_dongle_images copy_dongle_images prebuild_prep build_dhd build_apps release build_end

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
	cat $(SRCFILELIST_GPL_ORG) $(SRCFILELISTS_GPL_COMPONENTS) | \
		src/tools/release/rem_comments.awk > master_filelist_gpl.h
	gcc -E -undef  $(DEFS:%=-D%_SRC) -DESTA_OPENSRC_CLEANUP_LIST_SRC \
			$(UNDEFS:%=-DNO_%_SRC) \
			-Ulinux -o $(SRCFILELIST_GPL) master_filelist_gpl.h
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
	mkdir -p build/$(oem)
	mkdir -p build/$(oem)/components/shared/
	cp -R components/shared/* build/$(oem)/components/shared/
	gcc -E -undef $(ALLDHD_DEFS:%=-D%_SRC) $(OEM_DEFS:%=-D%_SRC) \
		$(ALLDHD_UNDEFS:%=-DNO_%_SRC) $(OEM_UNDEFS:%=-DNO_%_SRC) \
		-Ulinux -o $(SRCFILELIST)_$(oem) master_filelist.h
	if [ "$(oem)" == "bcm" ]; then \
		gcc -E -undef $(ALLP2P_DEFS:%=-D%_SRC) $(OEM_DEFS:%=-D%_SRC) \
		$(ALLP2P_UNDEFS:%=-DNO_%_SRC) $(OEM_UNDEFS:%=-DNO_%_SRC) \
		-Ulinux -o $(SRCFILELIST_HSL)_$(oem) master_filelist_hsl.h; \
	fi
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

build_bcm_apps: oem=bcm
build_bcm_apps: $(APP_DRIVERS_bcm)


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
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	# Copy README if exist
	touch $(RELDIR)/README.txt
	if test -e src/doc/BrandReadmes/$(BRAND).txt; then \
		install -v src/doc/BrandReadmes/$(BRAND).txt $(RELDIR)/README.txt 2>$(NULL) ||:; \
	fi
	if test -e src/doc/BrandReadmes/$(BRAND)_oem_$(oem).txt; then \
		install -v src/doc/BrandReadmes/$(BRAND)_oem_$(oem).txt $(RELDIR)/README.txt 2>$(NULL) ||:; \
	fi
	# Copy over host kernel modules
	@cd build/$(oem); \
	shopt -s nullglob; for mdir in src/dhd/linux/dhd-*; do \
	    mkdir -p -v $(RELEASEDIR)/$(oem)/host/$$(basename $${mdir}); \
            for mod in dhd.o dhd.ko bcmdhd.ko; do \
                if [ -s "$$mdir/$${mod}.stripped" ]; then \
                   install -pv $$mdir/$${mod}.stripped \
                        $(RELEASEDIR)/$(oem)/host/$$(basename $${mdir})/$${mod}; \
                fi; \
            done; \
	done
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
	#wlm fails to cross compile on arm, forcing x86 only
	@if [ "$(oem)" == "bcm" ]; then \
		install -p build/$(oem)/src/wl/exe/wlm/x86/wlm.so $(RELDIR)/apps; \
	fi
	install src/doc/BCMLogo.gif $(RELEASEDIR)/$(oem)
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
# 	#End of release_$(oem)_bins
	@$(MARKEND)

#- Bcm specific extra release files needed
release_bcm_extra_bins: RELDIR=release/bcm
release_bcm_extra_bins:
	@$(MARKSTART)
# 	#End of release_bcm_extra_bins
	@$(MARKEND)

#- Nokia specific extra release files needed
release_nokia_extra_bins:
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,release/nokia)
	$(call REMOVE_WARN_PARTIAL_BUILD,release/nokia)
# 	#End of release_nokia_extra_bins
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
	tar cpf - $(TAR_SKIPCMD) -C build/$(oem) \
		`$(FIND) src components $(FIND_SKIPCMD) -print | $(SRCFILTER) -v $(SRCFILELIST)_$(oem)` \
		`cd build/$(oem) && $(FIND) $(DNGL_IMGDIR)/* -type f \
			-name "*$(DNGL_IMG_PFX).h" -o -name "*$(DNGL_IMG_PFX).bin"` | \
		tar xpf - $(TAR_SKIPCMD) -C $(RELDIR); \
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
	         firmware host apps README.txt BCMLogo.gif
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
# Currently bcm need this special open-src source package
$(OEM_LIST_OPEN_SRC:%=release_%_open_src_package): oem=$(shell echo $(@F) | cut -d_ -f2)
$(OEM_LIST_OPEN_SRC:%=release_%_open_src_package): RELDIR=release/$(oem)
$(OEM_LIST_OPEN_SRC:%=release_%_open_src_package): OEM_DEFS= $(patsubst -D%,%,$(filter -D%,$(DEFS_$(oem))))
$(OEM_LIST_OPEN_SRC:%=release_%_open_src_package): OEM_UNDEFS= $(patsubst -U%,%,$(filter -U%,$(DEFS_$(oem)))) ESTA_OPENSRC_CLEANUP_LIST
$(OEM_LIST_OPEN_SRC:%=release_%_open_src_package):
ifeq ($(BRAND),linux-external-dongle-sdio)
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	@echo "Generating open source + binaries package now in $(RELDIR)"
	rm -rf $(RELDIR)/open-src
	mkdir -p $(RELDIR)/open-src
	# Copy all previous sources used for dhd/apps builds
	cp -rp $(RELDIR)/src $(RELDIR)/components $(RELDIR)/open-src/
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
	rm -f $(RELDIR)/open-src/$(SRCFILELIST)_$(oem)_open-src
	cd  $(RELDIR)/open-src; \
	$(FIND) src components -type d |  xargs rmdir -p --ignore-fail-on-non-empty

	# Translating copyright text 'open' to 'dual' and remove BCMROMFN, etc.
	$(MAKE) -f $(MOGRIFY_RULES) \
		MOGRIFY_DIRS=$(RELDIR)/open-src \
		MOGRIFY_FLAGS="$(DEFS_$(oem)) -translate_open_to_dual_copyright -strip_bcmromfn"
	rm -f $(RELDIR)/open-src/.mogrified
	# Stripping comments some selective open-src files
	cp $(SRCFILELIST_GPL) $(RELDIR)/open-src
	cd $(RELDIR)/open-src; \
	   $(FIND) src components | $(SRCFILTER) -v $(SRCFILELIST_GPL) | col -b | \
		xargs -t -l1 $(MOGRIFY) $(DEFS_$(oem)) \
		-DESTA_POSTMOGRIFY_REMOVAL -DLINUX_POSTMOGRIFY_REMOVAL -strip_comments
	rm -f $(RELDIR)/open-src/$(SRCFILELIST_GPL)
	# Cleanup dhd bits after validation open-src source package
	@if [ "$(oem)" == "bcm" ]; then \
		echo "$(MAKE) -C $(RELDIR)/open-src/src/dhd/exe clean"; \
		$(MAKE) -C $(RELDIR)/open-src/src/dhd/exe clean; \
		echo "$(MAKE) -C $(RELDIR)/open-src/src/dhd/linux clean"; \
		$(MAKE) -C $(RELDIR)/open-src/src/dhd/linux clean; \
	fi
	# Built dhd bits are already picked in previous target
	# Exclude apps/wl*, we do not release wl.exe binary or its sources
	tar cpf $(RELDIR)/dongle-host-driver-source-open.tar $(TAR_SKIPCMD) \
		-C $(RELDIR) --exclude=apps/wl* open-src/src open-src/components \
		firmware host apps README.txt BCMLogo.gif
	gzip -9 $(RELDIR)/dongle-host-driver-source-open.tar
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
release_doc: release_bcm_doc_package

release_bcm_doc_package: oem=bcm
release_bcm_doc_package: RELDIR=release/$(oem)
release_bcm_doc_package:

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

# Integrate firmwares into release directory
integrate_firmware:
	@$(MARKSTART)
	@echo "Copying images from firmware release"
	src/tools/misc/combine_release.sh $(BRAND) PHOENIX2_REL_6_10_148 $(TAG)
	src/tools/misc/combine_release.sh $(BRAND) AARDVARK_REL_6_30_69_14 $(TAG)
	@$(MARKEND)

build_end:
	@rm -fv $(HNDRTE_FLAG)
	@$(MARKEND_BRAND)

is32on64: # for build_config consumption
	@echo "$(BRAND) can be build on a 64 bit linux cluster"

.PHONY: FORCE checkout build_include build_end build_dongle_images prebuild_prep build_dhd build_apps build_hsl $(foreach oem,$(OEM_LIST),build_$(oem)_dhd) $(foreach oem,$(OEM_LIST),build_$(oem)_apps) release release_bins release_src $(foreach oem,$(OEM_LIST),release_$(oem)_src_package) $(foreach oem,$(OEM_LIST_OPEN_SRC),release_$(oem)_open_src_package) $(foreach oem,$(OEM_LIST),release_$(oem)_bins release_$(oem)_extra_bins) $(foreach oem,$(OEM_LIST),release_$(oem)_hsl_src_package) is32on64 integrate_firmware
