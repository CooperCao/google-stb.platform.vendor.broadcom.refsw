#
# Linux wl Makefile
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

PWD          := $(shell pwd)
BUILD_BASE   := $(PWD)
UNAME        := $(shell uname -a)
NULL         := /dev/null

VALIDATE_GPL := /projects/hnd_software/gallery/src/tools/release/validate_gpl.sh

export MOGRIFY_RULES     = mogrify_common_rules.mk
export BRAND_RULES       = brand_common_rules.mk

# Enable verbosity on wl/config details and linux dir/version
export SHOWWLCONF=1

# Redirect hybrid builds to E.C to match precommit usage
ifeq ($(findstring linux-external-wl-portsrc-hybrid,$(BRAND)),)
     export EMAKE_COMPATIBLE=no
endif

ifneq ($(findstring x86_64,$(UNAME)),)
	32BIT := -32 ARCH=i386
	32GCC64 := CC='gcc -m32'
	64GCC64 := CC='gcc -m64'
	32BITON64LIB := CROSS_LD_PATH=-L/projects/hnd/tools/linux/lib
	64BITON64LIB := CROSS_LD_PATH=-L/projects/hnd/tools/linux/lib_x86_64
	DRIVERS += $(DRIVERS_64)
	HYBRID_DRIVERS += $(HYBRID_DRIVERS_64)
	APP_DRIVERS += $(APP_DRIVERS_64)
	export 32ON64=1
endif # UNAME

SRCFILELISTS_COMPONENTS += src/tools/release/components/wps-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/secfrw-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/pre_secfrw-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/p2p-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/dhcpd-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/supp-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/wlolpc-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/wlphy-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/wlppr-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/bwl-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/bcmcrypto-filelist.txt
SRCFILELISTS_COMPONENTS += src/tools/release/components/wfi-filelist.txt

SRCFILELIST      := generic-filelist.txt
# prep'd filelist after processing above SRCFILELIST
SRCFILELIST_PREP := generic-filelist-prep.txt
HYBRID_FILELIST  := hybrid-filelist.txt
SRCFILTER        := src/tools/build/srcfilter.pl
WLEXEFILELIST    := src/tools/release/linux-wlexe-filelist.txt
SHIPPED_FILELIST := src/tools/release/hybrid-shipped-filelist.txt

# add BCMUSBDL for generating that list
BCMDLFILELIST    := src/tools/release/linux-usbbcmdl-filelist.txt

PREPROCESSOR_INCLUDES := src/tools/build/preprocessor_includes.pl
RELEASENOTES    ?= ReleaseNotesWl.html
RELPKG_DIR      ?= release/packages

# This causes CLM generation step to stage generated wlc_clm_data.c
# built as there is one wlc_clm_data.c per driver target. The
# central WLAN_Common.mk subsequently fetches them via makefile
# vpath search path
export WLAN_COPY_GEN=1

# Generated modules that need to be packaged
GENERATED_MODULES   := src/$(WLAN_GEN_BASEDIR)

# These are module names and directories that will be checked out of CVS.
ifneq ($(findstring media,$(BRAND)),)
  MEDIA_BLD       := 1
  SRCFILELIST_ORG := src/tools/release/linux-media-filelist.txt
  # SVN bom specification
  HNDSVN_BOM      := media.sparse
else
  HNDSVN_BOM      := wl-src.sparse
  SRCFILELIST_ORG := src/tools/release/linux-wl-filelist.txt
endif # media

include src-features-master-list.mk

NON_REMOVED_SRC_FEATURES := $(sort $(SELECTED_SRC_FEATURES) $(COMPILE_TIME_SRC_FEATURES))

INVALID_SRC_FEATURES := $(sort $(filter-out $(SRC_FEATURES_MASTER_LIST),$(NON_REMOVED_SRC_FEATURES)))

REMOVED_SRC_FEATURES := $(sort $(filter-out $(NON_REMOVED_SRC_FEATURES),$(SRC_FEATURES_MASTER_LIST)))

# Also remove unreleased chips ... backward compatible with non linux wl builds usage of unreleased-chiplist.mk
UNDEFS := WLAWDL AWDL_FAMILY DHD_AWDL WLAWDL_LATENCY_SUPPORT
UNDEFS += WLAIBSS IBSS_PEER_DISCOVERY_EVENT IBSS_PEER_GROUP_KEY IBSS_PEER_MGMT CREATE_IBSS_ON_QUIET_WITH_11H \
          IBSS_RMC NAN_ASP_IBSS OPENSSL_BUILD_SHLIBSSL WLAIBSS_MCHAN WLAIBSS_PS

DEFS :=

ifneq ($(MEDIA_BLD),)
# P2P WIFI BWL DEFS/UNDEFS
#DEFS += BCM47XX BCM47XX_CHOPS BCMSDIO LINUX BCMWPS P2P DHCPD_APPS
#DEFS += LINUX BCMWPS P2P DHCPD_APPS BWL WFI BCMSDIO
DEFS += LINUX BCMWPS P2P DHCPD_APPS WFI

SRCFILELISTS_COMPONENTS += src/tools/release/components/wps-filelist.txt

UNDEFS += CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ \
          CONFIG_BCM93725 BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
          CONFIG_BCM93725 BCM93725B BCM94100 BCM_USB NICMODE ETSIM \
          INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS \
          __klsi__ VX_BSD4_3 ROUTER BCMENET BCM4710A0 \
          CONFIG_BCM4710 CONFIG_MIPS_BRCM DSLCPE LINUXSIM BCMSIM \
          BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 BCMUSBDEV \
          BCMSDIODEV WLFIPS BCMWAPI_WPI BCMWAPI_WAI \
          DEBUG_LOST_INTERRUPTS BCMQT BCMSLTGT

endif # MEDIA_BLD

UNDEFS += BCM_DNGL_EMBEDIMAGE

# Unreleased-chiplist is included only for non-internal builds
ifeq ($(findstring BCMINTERNAL,$(DEFS))$(findstring internal,$(BRAND)),)
include unreleased-chiplist.mk
BLDTYPE := release
else
BLDTYPE := debug
endif # BCMINTERNAL
REMOVED_SRC_FEATURES := $(sort $(UNDEFS) $(REMOVED_SRC_FEATURES))

ifdef BCM_MFGTEST
ifneq ($(findstring WLTEST,$(REMOVED_SRC_FEATURES)),)
   $(error WLTEST can not be mogrified out for mfgtest builds)
endif
# All submakes get WLTEST setting inherited from this parent
export WLTEST=1
endif # BCM_MFGTEST

# for file list pre-processing
FILESDEFS = $(NON_REMOVED_SRC_FEATURES) $(DEFS)
FILESUNDEFS += $(patsubst %,NO_%,$(REMOVED_SRC_FEATURES) $(UNDEFS))
FILESDEFS += FILESUNDEFS
GCCFILESDEFS = $(patsubst %,-D%_SRC,$(FILESDEFS))


# Mogrifier step pre-requisites
MOGRIFY_FLAGS     = $(REMOVED_SRC_FEATURES:%=-U%) $(SELECTED_SRC_FEATURES:%=-D%)
MOGRIFY_FILETYPES =
MOGRIFY_EXCLUDE   =

# Add mogrify comment removal option if a hybrid driver is requested
ifneq ($(HYBRID_DRIVERS),)
	# Looks like WLPHY feature is forcefully removed as a define.
	HYBRID_FILESDEFS = $(filter-out -DWLPHY_SRC,  $(GCCFILESDEFS))
	MOGRIFY_FLAGS += -strip_comments
endif

# Don't use this feature for the moment. portsrc hybrid doesn't have any
# wlc common code in the tar.
ifeq (0,1)
	# Add mogify WL_xxxx debug message removal option if requested
	# (: separated)
	ifneq ($(STRIP_WLMSG_SYMBOLS),)
		MOGRIFY_FLAGS += -strip_wlmsgs "$(strip $(STRIP_WLMSG_SYMBOLS))"
	endif
endif

export MOGRIFY_FLAGS
export MOGRIFY_FILETYPES
export MOGRIFY_EXCLUDE

# Available toolchains
GCC33X    := /tools/gcc/3.3.1/i686-2.4/bin/
GCC43X    := /tools/gcc/4.3.0/i686-2.6/bin/
GCC64_43X := /tools/gcc/4.3.0/x86_64-2.6/bin
GCCXC     := /tools/bin/
GCCXC-5.1.1     :=/tools/linux/local/cross/rhel6-64/fc22/bin/x86_64-fc22.4.0.4.301-linux-

# Linux Kernel for validating wl src
LINUX_KERNEL_DIRS_TO_VALIDATE_WL_SRC  := \
	linux-3.11.1-200.fc19.i686.PAE \
	linux-3.11.1-200.fc19.x86_64 \
	linux-2.6.38.6-26.rc1.fc15-x86_64 \
	linux-2.6.38.6-26.rc1.fc15.i686.PAE

# WARN /tools/oss/bin/ will be obsoleted and replaced with /tools/bin/
GCCX      := /tools/oss/bin/

PATH      := $(PATH):/projects/hnd/tools/linux/hndtools-mipsel-linux/bin
PATH      := $(PATH):/projects/hnd/tools/linux-arm/armeb9-linux-3.4.6/bin

# Defaults for x86, arm and mips architectures

# vanilla 2.6.35
v35-LINUXVER          := 2.6.35-9-generic
v35-CROSS_COMPILE     := $(GCCX)
v35-GCCVER            := 4.8.2$(32BIT)
v35-opt-xcc           := TARGETARCH=x86 BLDTYPE=$(BLDTYPE)
ifneq ($(32ON64),)
v35-opt-gcc           := $(v35-opt-xcc) $(32GCC64) $(32BITON64LIB)
v35-opt-xcc           += CROSS_COMPILE=$(v35-CROSS_COMPILE)
v35-opt-xcc           += GCCVER=$(v35-GCCVER)
# for linking 32 bit hybrid driver
v35-32ELF_EMU         := -melf_i386
endif
v35-opt-xcc           += OBJDIR=obj/$(BLDTYPE)/x86/
# STBLinux-FC10 (mips-little-endian)
bdfc10-LINUXVER       := 2.6.28.9
bdfc10-LINUXDIR       := /tools/linux/src/stblinux-2.6.28
bdfc10-CROSS_COMPILE  := /projects/hnd/tools/media/crosstools_sf-linux-2.6.18.0_gcc-4.2-10ts_uclibc-nptl-0.9.29-20070423_20080721/bin/mipsel-linux-
bdfc10-opt            := PATH="/projects/hnd/tools/media/crosstools_sf-linux-2.6.18.0_gcc-4.2-10ts_uclibc-nptl-0.9.29-20070423_20080721/bin:$(PATH)"
bdfc10-opt            += TARGETARCH=mips TARGETENV=linuxmips TARGETOS=unix
bdfc10-opt-gcc        := $(bdfc10-opt) CC=mipsel-linux-gcc LIBUSB_PATH=/projects/hnd/tools/linux/lib/mips
bdfc10-opt-gcc        += OBJDIR=mips/
bdfc10-opt-xcc        := $(bdfc10-opt) CROSS_COMPILE=mipsel-linux-
bdfc10-opt-xcc        += BLDTYPE=$(BLDTYPE) OBJDIR=obj/$(BLDTYPE)/mips/
# STBLinux-FC15 (mips-little-endian)
stbfc15-LINUXVER       := 2.6.37-3.0
stbfc15-LINUXDIR       := /tools/linux/src/stblinux-2.6.37-3.0
stbfc15-CROSS_COMPILE  := /projects/hnd/tools/media/stbgcc-4.5.3-2.1/bin/mipsel-linux-
stbfc15-opt            := PATH="/projects/hnd/tools/media/stbgcc-4.5.3-2.1/bin:$(PATH)"
stbfc15-opt            += TARGETARCH=mips TARGETENV=linuxmips TARGETOS=unix
stbfc15-opt-gcc        := $(stbfc15-opt) CC=mipsel-linux-gcc LIBUSB_PATH=/projects/hnd/tools/linux/lib/mips
stbfc15-opt-gcc        += OBJDIR=mips/
stbfc15-opt-xcc        := $(stbfc15-opt) CROSS_COMPILE=mipsel-linux-
stbfc15-opt-xcc        += BLDTYPE=$(BLDTYPE) OBJDIR=obj/$(BLDTYPE)/mips/
# fc15 i686 (32bit)
fc15-LINUXVER         := 2.6.38.6-26.rc1.fc15.i686.PAE
fc15-CROSS_COMPILE    := $(GCCXC)
fc15-opt-xcc          := TARGETARCH=x86 BLDTYPE=$(BLDTYPE)
ifneq ($(32ON64),)
  fc15-opt-gcc        := $(fc15-opt-xcc) $(32GCC64) $(32BITON64LIB)
  fc15-opt-xcc        += CROSS_COMPILE=$(fc15-CROSS_COMPILE)
endif
fc15-opt-xcc          += OBJDIR=obj/$(BLDTYPE)/x86/

# fc19 i686 (32bit)
fc19-LINUXVER       := 3.11.1-200.fc19.i686.PAE
fc19-CROSS_COMPILE  := $(GCCXC)
fc19-opt-xcc        := TARGETARCH=x86 BLDTYPE=$(BLDTYPE)
ifneq ($(32ON64),)
  fc19-opt-gcc      := $(fc19-opt-xcc) $(32GCC64) $(32BITON64LIB)
  fc19-opt-xcc      += CROSS_COMPILE=$(fc19-CROSS_COMPILE)
endif
fc19-opt-xcc        += OBJDIR=obj/$(BLDTYPE)/x86/

# Linux/MIPS
mipsel-LINUXDIR       := /projects/hnd/swbuild/build_linux/SPURS_REL_3_60_RC9/linux-external-router/2004.4.2.0/build/src/linux/linux
mipsel-ARCH           := mipsel
mipselexe-TARGETENV   := linuxmips
mipselexe-TARGETARCH  := mips
mipselexe-TARGETOS    := unix

# Linux/ARM
arm-LINUXDIR          := /projects/hnd/tools/linux-arm/linux-2.6.17.14-bcm1161
arm-ARCH              := arm
arm-CROSS_COMPILE     := /projects/hnd/tools/linux-arm/armeb9-linux-3.4.6/bin/armeb-linux-
armexe-TARGETENV      := linuxarm
armexe-TARGETARCH     := arm
armexe-TARGETOS       := unix

# Defaults for x86_64 architecture
# vanilla 2.6.35
v35_64-LINUXVER       := 2.6.35-9-generic-x86_64
v35_64-CROSS_COMPILE  := $(GCCX)
v35_64-GCCVER         := 4.8.2
v35_64-opt-xcc        := TARGETARCH=x86_64
v35_64-opt-gcc        := $(v35_64-opt-xcc)
v35_64-opt-gcc        += OBJDIR=x86_64
v35_64-opt-xcc        += CROSS_COMPILE=$(v35_64-CROSS_COMPILE)
v35_64-opt-xcc        += GCCVER=$(v35_64-GCCVER)
v35_64-opt-xcc        += BLDTYPE=$(BLDTYPE) OBJDIR=obj/$(BLDTYPE)/$(TARGETARCH)
# fc15 x64 (64bit)
fc15_64-LINUXVER      := 2.6.38.6-26.rc1.fc15-x86_64
fc15_64-CROSS_COMPILE := $(GCCXC)
fc15_64-opt-xcc       := TARGETARCH=x86_64 BLDTYPE=$(BLDTYPE) $(64GCC64) $(64BITON64LIB)
fc15_64-opt-gcc       := $(fc15_64-opt-xcc)
fc15_64-opt-gcc       += OBJDIR=x64_64
fc15_64-opt-xcc       += CROSS_COMPILE=$(fc15_64-CROSS_COMPILE)
fc15_64-opt-xcc       += OBJDIR=obj/$(BLDTYPE)/x86_64
# fc19 x64 (64bit)
fc19_64-LINUXVER      := 3.11.1-200.fc19.x86_64
fc19_64-CROSS_COMPILE := $(GCCXC)
fc19_64-opt-xcc       := TARGETARCH=x86_64
fc19_64-opt-gcc       := $(fc19_64-opt-xcc)
fc19_64-opt-gcc       += OBJDIR=x64_64
fc19_64-opt-xcc       += CROSS_COMPILE=$(fc19_64-CROSS_COMPILE)
fc19_64-opt-xcc       += BLDTYPE=$(BLDTYPE) OBJDIR=obj/$(BLDTYPE)/$(TARGETARCH)
# fc22 x64 (64bit)
fc22_64-LINUXVER      := 4.0.4-301.fc22.x86_64
fc22_64-CROSS_COMPILE := $(GCCXC-5.1.1)
fc22_64-opt-xcc       := TARGETARCH=x86_64
fc22_64-GCCVER        := 5.1.1
fc22_64-opt-gcc       := $(fc22_64-opt-xcc)
fc22_64-opt-gcc       += OBJDIR=x64_64
fc22_64-opt-xcc       += CROSS_COMPILE=$(fc22_64-CROSS_COMPILE)
fc22_64-opt-xcc       += GCCVER=$(fc22_64-GCCVER)
fc22_64-opt-xcc       += BLDTYPE=$(BLDTYPE) OBJDIR=obj/$(BLDTYPE)/$(TARGETARCH)

validate_src-opt-xcc   := TARGETARCH=x86 BLDTYPE=$(BLDTYPE)
ifneq ($(32ON64),)
validate_src-opt-gcc   := $(validate_src-opt-xcc) $(32GCC64) $(32BITON64LIB)
validate_src-opt-xcc   += CROSS_COMPILE=$(validate_src-CROSS_COMPILE)
validate_src-opt-xcc   += GCCVER=$(validate_src-GCCVER)
endif
validate-opt-xcc       += OBJDIR=obj/$(BLDTYPE)/x86/

WL_APP_TARGETS := $(APP_DRIVERS:%=%-app)

empty :=
space := $(empty) $(empty)
comma := $(empty),$(empty)

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
RELNUM_ := $(shell echo $(RELNUM) | tr '.' '_')
RELDATE := $(shell date '+%c')

# If driver types aren't specified, set either debug or nodebug internal/external brands
# if release/brand makefile doesn't set DEBUGS value
ifneq ($(findstring external,$(BRAND)),)
  DEBUGS ?= nodebug
else
  DEBUGS ?= debug
endif

# If a given <kernel>-DEFBASICS doesn't mark a target as either debug|nodebug
# set to brand specific DEBUGS default list
define set_debugs
$(foreach tgt,$(1),$(if $(findstring debug,$(tgt)),$(tgt),$(DEBUGS:%=%-$(tgt))))
endef

# DRIVER_TARGETS e.g. driver-fc6-<debug|nodebug>-stadef-apdef
DRIVER_TARGETS := $(foreach driver_i, $(DRIVERS),\
                  $(patsubst %,driver-$(driver_i)-%,$(call set_debugs,$($(driver_i)-DEFBASICS))))


# HYBRID_DRIVER_TARGETS e.g. hybrid-fc6-<debug|nodebug>-stadef-apdef
HYBRID_DRIVER_TARGETS := $(foreach driver_i, $(HYBRID_DRIVERS),\
                         $(patsubst %,hybrid-$(driver_i)-%,$(call set_debugs,$($(driver_i)-DEFBASICS))))


# WLSRC_TARGETS e.g. srcwldriver-fc6-<debug|nodebug>-stadef-apdef
RELEASE_WLSRC_TARGETS := $(strip $(foreach driver_i, $(RELEASE_WLSRC_DRIVERS),\
                     $(patsubst %,validate_wl_src-$(driver_i)-%,$(call set_debugs,$($(driver_i)-DEFBASICS)))))

# Generate list of wl driver source files given a wl target (DEFBASICS)
CUR_WLFILES = $(shell $(MAKE) --no-print-directory -C $(PWD)/src/wl/linux INOBJDIR=true WLCONFFILE=$(PWD)/src/wl/config/wlconfig_lx_wl_$(word $(words $(subst -, ,$@)),$(subst -, ,$@)) TARGET=$@ TARGETS=$@ showwlfiles_src 2> $(NULL))

ifdef HYBRID_DRIVERS
  PARTIAL_WL_PFX   := wlc_hybrid
endif # HYBRID_DRIVERS

ifneq ($(ALL_DNGL_IMAGES),)
  HNDRTE_DEPS  := mogrify
  HNDRTE_IMGFN := rtecdc.h
endif # ALL_DNGL_IMAGES

define arch_word
$(filter mipsel mipseb arm,$(subst -, ,$1))
endef

define driver_word
$(filter $(RELEASE_WLSRC_DRIVERS) $(HYBRID_DRIVERS) $(DRIVERS),$(subst -, ,$1))
endef

define non_config_words
$(filter driver hybrid srcwldriver $(call driver_word,$1) $(call arch_word,$1),$(subst -, ,$1))
endef

define config
$(patsubst %-,%,$(subst $(space),,$(foreach w,$(filter-out $(call non_config_words,$1),$(subst -, ,$1)),$(w)-)))
endef

all: build_start showenv release build_end

ifneq ($(ALL_DNGL_IMAGES),)
include linux-dongle-image-launch.mk
# Additionally copy these from dongle image build for usb targets
DNGL_IMG_FILES += rtecdc.trx
DNGL_IMG_FILES += rtecdc.bin.trx
DNGL_IMG_FILES += bcm94319wlusbnp4l.nvm

  # If any defbasics target contains embeddedable firmware,
  # build those firmware images; generate list of rtecdc_<chip>.h and append
  # them to filelist
  ifneq ($(findstring dnglimage,$(ALL-DEFBASICS)),)
    # If a calling release makefile doesn't specify embeddable firmware images, only then try to derive
    ifndef EMBED_DONGLE_IMAGES
      $(warning Parent $(BRAND).mk does not list EMBED_DONGLE_IMAGES)
      EMBED_DONGLE_IMAGES=$(shell $(MAKE) -s TARGETS=dummy $(if $(BCM_MFGTEST), WLTEST=1) -C src/wl/linux show_dongle_images 2> $(NULL))
    endif

    ALL_DNGL_IMAGES       += $(EMBED_DONGLE_IMAGES)
    EMBED_DONGLE_CHIPREVS += $(foreach img,$(EMBED_DONGLE_IMAGES),$(subst -bmac,,$(word 1,$(subst /, ,$(img)))))
    DNGL_IMG_FILES        += $(patsubst %,rtecdc_%.h,$(EMBED_DONGLE_CHIPREVS))
    ifneq ($(SRCFILES),)
       SRCFILES           += $(EMBED_DONGLE_IMAGES:%=build/dongle/%/rtecdc*.h)
    endif
    # For media source package, include embeddable as well as other releasable
    # firmware images in media-src.tar.gz package
    ifneq ($(MEDIA_BLD),)
       ifneq ($(SRCFILES),)
         SRCFILES         += $(ALL_DNGL_IMAGES:%=build/dongle/%/rtecdc*.h)
         SRCFILES         += $(ALL_DNGL_IMAGES:%=build/dongle/%/rtecdc*.trx)
       endif
    endif
  endif # dnglimage
endif # ALL_DNGL_IMAGES

include $(MOGRIFY_RULES)
include $(BRAND_RULES)

PREPROCESSOR_INCLUDE_PATH := $(addprefix -I ,$(addsuffix /include,src src/common $(WLAN_COMPONENT_PATHS)) ./components/shared ../components/shared/devctrl_if)

ifneq ($(ALL_DNGL_IMAGES),)
release: build_dongle_images copy_dongle_images
endif # ALL_DNGL_IMAGES
release: $(if $(DRIVERS),$(DRIVER_TARGETS))
release: $(if $(DRIVERS),package_generic_wl_src validate_generic_wl_src)
release: $(if $(HYBRID_DRIVERS),hybrid-prep)
release: $(if $(HYBRID_DRIVERS),$(HYBRID_DRIVER_TARGETS))
release: $(if $(WL_APP_TARGETS),$(WL_APP_TARGETS))
release: $(if $(RELEASE_WLEXESRC),validate_wlexe_src package_wlexe_src) \
         $(if $(RELEASE_WLSRC),validate_wl_src package_wl_src) \
         $(if $(RELEASE_BCMDLSRC),validate_bcmdl_src package_bcmdl_src) \
         $(if $(RELEASE_MEDIASRC),package_media_src)

release: release_driver
ifneq ($(WLNINTENDO),true)
release: release_apps
endif

release:
	@$(MARKEND)

showenv:
	@$(MARKSTART)
	@echo "DRIVERS           = $(DRIVERS)"
	@echo "DRIVER TARGETS    = $(DRIVER_TARGETS)"
	@echo "HYBRID_TARGETS    = $(HYBRID_DRIVER_TARGETS)"
	@echo "APPS              = $(WL_APP_TARGETS)"
	@echo "WLSRC_TARGETS     = $(RELEASE_WLSRC_TARGETS)"
ifneq ($(ALL_DNGL_IMAGES),)
	@echo "ALL_DNGL_IMAGES   = $(ALL_DNGL_IMAGES)"
endif # ALL_DNGL_IMAGES
	@echo "SRC_FEATURES      = $(COMPILE_TIME_SRC_FEATURES)"
	@echo "DEFS              = $(DEFS)"
	@echo "UNDEFS            = $(UNDEFS)"
	@$(MARKEND)

clean:
	@$(MARKSTART)
	rm -rf src build release hybrid-*
	@$(MARKEND)

filelists: checkout
	@$(MARKSTART)
	# Temporary filelist generation for SVN build test
	cat $(SRCFILELIST_ORG) $(SRCFILELISTS_COMPONENTS) | \
		src/tools/release/rem_comments.awk > master_filelist.h
	gcc -E -undef $(GCCFILESDEFS) -Ulinux -o $(SRCFILELIST) master_filelist.h
	# By processing on SELECTED_SRC_FEATURES generate HYBRID_FILELIST
	# SELECTED_SRC_FEATURES go as DEFS and others as UNDEFS in processing
	# master_filelist
	gcc -E -undef $(HYBRID_FILESDEFS) -Ulinux -o $(HYBRID_FILELIST) master_filelist.h
	@$(MARKEND)

# Checkout sources
checkout: $(CHECKOUT_TGT)

build_start:
	@$(MARKSTART_BRAND)

mogrify: checkout filelists
	@$(MARKSTART)
ifneq ($(SELECTED_SRC_FEATURES)$(COMPILE_TIME_SRC_FEATURES),)
	@echo Source features selected $(SELECTED_SRC_FEATURES)
	@echo Source features available at compile time $(COMPILE_TIME_SRC_FEATURES)
	@echo Source features implicitly removed $(REMOVED_SRC_FEATURES)
	@if [ -n "$(INVALID_SRC_FEATURES)" ] ; then \
		echo The following SOURCE FEATURES are INVALID; \
		echo $(INVALID_SRC_FEATURES); \
		exit 1; \
	  fi
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_DIRS=src
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_DIRS=components
else # SELECTED_SRC_FEATURES or COMPILE_TIME_SRC_FEATURES
	@echo Source feature removal, i.e. mogrify, skipped
endif # SELECTED_SRC_FEATURES or COMPILE_TIME_SRC_FEATURES
	@$(MARKEND)

prep: mogrify
	@$(MARKSTART)
	mkdir -p build/components
	set -o pipefail; \
	    tar -C components --exclude=README --exclude=unittest --exclude-vcs \
		-cf - phy shared/devctrl_if shared/proto shared/d11shm | \
	    tar -C build/components -xf -

# TODO: Remove this SRCFILES based conditional that does different steps
# TODO: if it is defined. Use some other macro to decide this step
ifneq ($(SRCFILES),)
	@echo Copying src to build/src based on files in $(SRCFILELIST)
	grep "\.c$$" $(SRCFILELIST) >cfiles-$@
	perl $(PREPROCESSOR_INCLUDES) $(PREPROCESSOR_INCLUDE_PATH) `cat cfiles-$@` | sort -u >hfiles-$@
	cat $(SRCFILELIST) hfiles-$@ >list-$@
	find src components $(FIND_SKIPCMD) -type f -print | perl $(SRCFILTER) -v -e list-$@ | \
		pax -rw $(PAX_SKIPCMD) build
	@echo Also copying these additional files: $(SRCFILES)
	perl -le 'while (@ARGV) { print $$ARGV[0]; shift @ARGV; }' $(SRCFILES) |\
		pax -rw $(PAX_SKIPCMD) build
	# Record the filtered contents in $(SRCFILELIST_PREP) for later use
	cd build; find . -depth -type f > ../$(SRCFILELIST_PREP)
else # SRCFILES
	@echo Copying src to build/src ...  no file filter
	find src components $(FIND_SKIPCMD) -type f -print | pax -rw $(PAX_SKIPCMD) build
endif # SRCFILES
	@rm -fv list-$@ cfiles-$@ cfiles1-$@ hfiles-$@
	# Copy files for building apps also.
	# media build already include them
	# do so for wl builds
ifneq ($(MEDIA_BLD),)
	# package media src
ifeq ($(RELEASE_MEDIASRC),1)
	cd build && find src components $(FIND_SKIPCMD) -type f -print > ../media_src_pkg-contents.txt
endif # RELEASE_MEDIASRC
else # MEDIA_BLD
	find src components $(FIND_SKIPCMD) -type f -print | \
		perl $(SRCFILTER) -v -e $(WLEXEFILELIST) | \
		pax -rw $(PAX_SKIPCMD) build
	find src components $(FIND_SKIPCMD) -type f -print | \
		perl $(SRCFILTER) -v -e $(BCMDLFILELIST) | \
		pax -rw $(PAX_SKIPCMD) build
endif # MEDIA_BLD
	@$(MARKEND)

package_generic_wl_src: prep $(DRIVER_TARGETS)
	# Append list of generated modules to processed filelist filter
	@for mod in $(GENERATED_MODULES); do \
	    echo "$$mod" >> $(SRCFILELIST_PREP); \
	done
ifndef RELEASE_WLSRC
#	# Temporary Debugging random preco issues, that show in production only
ifeq ($(EMAKE_COMPATIBLE),no)
	tar -cpzf build/generic-src.tar.gz $(TAR_SKIPCMD) -C build -T $(SRCFILELIST_PREP)
else  # EMAKE_COMPATIBLE
	-tar -cpzf build/generic-src.tar.gz $(TAR_SKIPCMD) -C build -T $(SRCFILELIST_PREP)
endif # EMAKE_COMPATIBLE
endif # RELEASE_WLSRC

ifneq ($(filter $(BRAND),linux-external-wl linux-mfgtest-wl),)
validate_generic_wl_src: prep $(DRIVER_TARGETS)
	@$(MARKSTART)
	set -o pipefail
	for kernel_src in $(LINUX_KERNEL_DIRS_TO_VALIDATE_WL_SRC); do \
		tgtbase=$(BUILD_BASE)/VGSW && \
		rm -rf $$tgtbase && \
		mkdir -pv $$tgtbase && \
		tar -xzvf $(BUILD_BASE)/build/generic-src.tar.gz  $(TAR_SKIPCMD) -C $$tgtbase && \
		$(MAKE) -C $$tgtbase/src/wl/linux debug-native-apdef-stadef \
		LINUXDIR=/tools/linux/src/$$kernel_src 2>&1 | tee -a $$tgtbase/,wl_validate_build.log && \
		if (grep "\[debug-native-apdef-stadef\] Error" $$tgtbase/,wl_validate_build.log) ; then \
			echo "ERROR: wl src validation failed" ; \
			exit 1; \
		fi ; \
		rm -rf $$tgtbase ; \
	done
	@$(MARKEND)
else

validate_generic_wl_src:

endif

# Build tool(s)
# Module makefile should produce TARGETARCH specific executable
# (e.g. wlarm_le for ARM or wlmips for MIPS)
$(WL_APP_TARGETS): opt=$(subst -app,-opt,$(@))
$(WL_APP_TARGETS): prep
	@$(MARKSTART)
	# Create arch-specific src/wl/exe directory for non-native arch
	-@$(if $(filter-out exe,$@) cp -ra src/wl/exe src/wl/$@,echo "Build native $@")
	$(MAKE) -C build/src/wl/exe $($(opt)-gcc)
ifneq ($(MEDIA_BLD),)
#	$(MAKE) -C build/src/apps/bwl/linux $($(opt)-gcc)
	$(MAKE) -C build/src/apps/wfi/linux/wfi_api $($(opt)-xcc)
	$(MAKE) -C build/src/wps/wpsapi/linux $($(opt)-xcc)
	$(MAKE) -C build/src/usbdev/usbdl $($(opt)-gcc) ObjPfx=$(subst TARGETARCH=,,$(filter TARGETARCH%, $($(opt)-gcc)))/
else
ifneq ($(ALL_DNGL_IMAGES)$(RELEASE_BCMDLSRC),)
	$(MAKE) -C build/src/usbdev/usbdl $($(opt)-gcc)
endif # ALL_DNGL_IMAGES
endif # MEDIA_BLD
	@$(MARKEND)

# Release and archive (only build tools if not a hybrid because BCMDRIVER is mogrified in)
release_driver: prep $(DRIVER_TARGETS)
	@$(MARKSTART)
	install -d release
	install -d release/exe
	-install -p src/doc/BrandReadmes/$(BRAND).txt release/README.txt
# Only release source if features have been selected (SELECTED or COMPILE_TIME)
# and brand specific source files are provided. An external brand must provide
# these by definition.
ifneq ($(SELECTED_SRC_FEATURES)$(COMPILE_TIME_SRC_FEATURES),)
ifneq ($(SRCFILES),)
        # Release source tarball
        #cd build/src/wl/linux && find . -path '*.tar.gz' | \
	#cpio -L -p -v -d $(PWD)/release
ifndef RELEASE_WLSRC
	mv build/generic-src.tar.gz release/generic-src.tar.gz
endif  # RELEASE_WLSRC
endif # SRCFILES
endif # SELECTED_SRC_FEATURES or COMPILE_TIME_SRC_FEATURES
        # Release modules
	cd build/src/wl/linux && \
	find .	-path '*/wl.ko' -o \
		-path '*/wl*.ko' -o \
		-path '*/bcm_dbus*' -o \
		-path '*/wlumdrv' | \
		grep -v "\-high-" | \
		pax -rw -v $(PAX_SKIPCMD) $(PWD)/release
        # Release tool
ifeq ($(WLNINTENDO),true)
        # linux command line tool for nintendo
	cd build/src/wl && find . \( -path '*exe/brcmwd' -o -path '*exe/brcmwd_listen' \) -type f | \
	pax -rw -v $(PAX_SKIPCMD) $(PWD)/release
	cp src/doc/WL_Spec.pdf release/
	cp src/include/nintendowm.h release/
        # temporary setting to detect internal build
ifeq ($(filter BCMINTERNAL,$(DEFS)),BCMINTERNAL)
	cd build/src/tools/misc && find . -path './nitro' -type f | \
		pax -rw -v $(PAX_SKIPCMD) $(PWD)/release/exe/
endif
ifeq ($(filter BCMINTERNAL,$(DEFS)),BCMINTERNAL)
	cd build/src/tools/misc && find . -path './nitro' -type f | \
		pax -rw -v $(PAX_SKIPCMD) $(PWD)/release/exe/
	cd build/src/wl && find . -path '*exe/wl' -type f | \
		pax -rw -v $(PAX_SKIPCMD) $(PWD)/release/
endif
endif #WLNINTENDO
ifneq ($(ALL_DNGL_IMAGES),)
	# Copy dongle images first
	mkdir -p release/firmware
	-@for img in $(ALL_DNGL_IMAGES); do \
	   echo "Copying $${img}"; \
	   install -dv release/firmware/$${img}; \
	   install -pv $(DNGL_IMG_FILES:%=$(DNGL_IMGDIR)/$${img}/%) \
			release/firmware/$${img}; \
	done
	# Release bmac/high modules only if dongle images are copied over
	cd build/src/wl/linux && \
	find .	-path '*/wl.ko' -o \
		-path '*/wl*.ko' -o \
		-path '*/bcm_dbus*' -o \
		-path '*/wlumdrv' | \
		grep "\-high-" | \
		pax -rw -v $(PAX_SKIPCMD) $(PWD)/release
ifneq ($(RELEASE_MEDIASRC),)
	@echo Copying the CLM Generated file needed for building package
	wlc_clm_data_c=$$(find build/src/wl/linux -name wlc_clm_data.c | tail -n 1) && \
	cp -p $${wlc_clm_data_c?} build/components/clm-api/src/ && \
	cp -p $${wlc_clm_data_c?} components/clm-api/src/
endif # RELEASE_MEDIASRC
endif # ALL_DNGL_IMAGES
	cp src/doc/$(RELEASENOTES) release/ReleaseNotes.html
	cp src/doc/BCMLogo.gif release/
	@$(MARKEND)

release_apps: release_driver
	@$(MARKSTART)
	for arch in $(foreach app, $(APP_DRIVERS), $(patsubst TARGETARCH=%,%,$(filter TARGETARCH%, $($(patsubst %,%-opt-xcc,$(app)))))); do \
		if [ "$$arch" != "x86" ]; then \
			sarch=$$arch; \
		else \
			sarch=""; \
		fi; \
		echo "TARGETARCH=$$arch"; \
		mkdir -pv release/exe/$$sarch; \
		if [ -f "build/src/wl/exe/wl$$sarch" ]; then \
			install -pv build/src/wl/exe/wl$$sarch $(BUILD_BASE)/release/exe/$$sarch/wl; fi; \
		if [ -f "build/src/usbdev/usbdl/$$sarch/bcmdl" ]; then \
			install -pv build/src/usbdev/usbdl/$$sarch/bcmdl $(BUILD_BASE)/release/exe/$$sarch; fi; \
		echo "Copying RWL bits if they exist for $$arch"; \
		if [ -f "build/src/wl/exe/dongle_noasd/$$arch/wl_server_dongle$$sarch" ]; then \
			install -pv build/src/wl/exe/dongle_noasd/$$arch/wl_server_dongle$$sarch $(BUILD_BASE)/release/exe/$$sarch; fi; \
		if [ -f "build/src/wl/exe/serial_noasd/$$arch/wl_server_serial$$sarch" ]; then \
			install -pv build/src/wl/exe/serial_noasd/$$arch/wl_server_serial$$sarch $(BUILD_BASE)/release/exe/$$sarch; fi; \
		if [ -f "build/src/wl/exe/socket_noasd/$$arch/wl_server_socket$$sarch" ]; then \
			install -pv build/src/wl/exe/socket_noasd/$$arch/wl_server_socket$$sarch $(BUILD_BASE)/release/exe/$$sarch; fi; \
		if [ -f "build/src/wl/exe/wifi_noasd/$$arch/wl_server_wifi$$sarch" ]; then \
			install -pv build/src/wl/exe/wifi_noasd/$$arch/wl_server_wifi$$sarch $(BUILD_BASE)/release/exe/$$sarch; fi; \
		echo "Copied RWL bits for $$arch"; \
	done
ifneq ($(MEDIA_BLD),)
	for arch in $(foreach app, $(APP_DRIVERS), $(patsubst TARGETARCH=%,%,$(filter TARGETARCH%, $($(patsubst %,%-opt-xcc,$(app)))))); do \
		if [ "$$arch" != "x86" ]; then \
			sarch=$$arch; \
		else \
			sarch=""; \
		fi; \
		if [ -f "build/src/usbdev/usbdl/$$arch/bcmdl" ]; then \
			install -pv build/src/usbdev/usbdl/$$arch/bcmdl $(BUILD_BASE)/release/exe/$$sarch; fi; \
		if [ -f "build/src/apps/wfi/linux/wfi_api/obj/$(BLDTYPE)/$$arch/libwfiapi.a" ]; then \
			install -pv build/src/apps/wfi/linux/wfi_api/obj/$(BLDTYPE)/$$arch/libwfiapi.a $(BUILD_BASE)/release/exe/$$sarch; fi; \
		if [ -f "build/src/apps/wfi/linux/wfi_api/obj/$(BLDTYPE)/$$arch/wfiapitester" ]; then \
			install -pv build/src/apps/wfi/linux/wfi_api/obj/$(BLDTYPE)/$$arch/wfiapitester $(BUILD_BASE)/release/exe/$$sarch; fi; \
		if [ -f "build/src/wps/wpscli/linux/obj/$(BLDTYPE)/$$arch/wpscli_test_cmd" ]; then \
			install -pv build/src/wps/wpscli/linux/obj/$(BLDTYPE)/$$arch/wpscli_test_cmd $(BUILD_BASE)/release/exe/$$sarch; fi; \
		if [ -f "build/src/wps/wpscli/linux/obj/$(BLDTYPE)/$$arch/*.a" ]; then \
			install -pv build/src/wps/wpscli/linux/obj/$(BLDTYPE)/$$arch/*.a $(BUILD_BASE)/release/exe/$$sarch; fi; \
		if [ -f "build/src/wps/wpsapi/linux/$(BLDTYPE)/$$arch/wpsenr" ]; then \
			install -pv build/src/wps/wpsapi/linux/$(BLDTYPE)/$$arch/wpsenr $(BUILD_BASE)/release/exe/$$sarch; fi; \
		if [ -f "build/src/wps/wpsapi/linux/$(BLDTYPE)/$$arch/wpsreg" ]; then \
			install -pv build/src/wps/wpsapi/linux/$(BLDTYPE)/$$arch/wpsreg $(BUILD_BASE)/release/exe/$$sarch; fi; \
	done
endif # MEDIA_BLD
	@$(MARKEND)

# Parse the shipped file list, and select files for the given target.
.shipped = \
  $(sort \
    $(foreach f,$(shell sed -e 's,\#.*$$,,' ${SHIPPED_FILELIST} 2>/dev/null), \
      $(or $(filter $(strip $1):%,$f), \
        $(if $(filter 1,$(words $(subst :, ,$f))),$f) \
       ) \
     ) \
   )

# Generate the correct name of the linker and its default flags, per target.
.crossld = \
  $(strip ${$(strip $(call driver_word,$1))-CROSS_COMPILE})ld \
    ${$(strip $(call driver_word,$1))-32ELF_EMU}

# Generate the name of the object output directory, per target.
.objpath = \
  build/src/wl/linux/obj-$(call config,$1)-${$(call driver_word,$1)-LINUXVER}

# Join words from $2 together around a string from $1.
.compose = $(subst ${space},$(strip $1),$(strip $2))

# DRIVER_TARGETS e.g. driver-fc6-debug-stadef-apdef
driver-%: prep
	@$(MARKSTART)
	$(MAKE) -C build/src/wl/linux $(if $(SHOWWLCONF),SHOWWLCONF=1) \
	  LINUXVER=$($(call driver_word,$@)-LINUXVER) \
	  $(if $($(call driver_word,$@)-LINUXDIR),LINUXDIR=$($(call driver_word,$@)-LINUXDIR)) \
	  $(if $($(call driver_word,$@)-GCCVER) ,GCCVER=$($(call driver_word,$@)-GCCVER)) \
	  $(if $($(call driver_word,$@)-TARGETARCH),ARCH=$($(call driver_word,$@)-TARGETARCH)) \
	  CROSS_COMPILE=$($(call driver_word,$@)-CROSS_COMPILE) \
	  $(subst $(filter $(call driver_word,$*),$(subst -, ,$*))-,,$*)
	sync ; sync ; sync
	chmod 444 $(call .objpath,$*)/*.o
	@echo "DRIVER $* OBJECTS" ; ls -l $(call .objpath,$*)/*.o
	@$(MARKEND)

# Common step to do second level mogrification of hybrid sources
hybrid-prep: prep
	@$(MARKSTART)
	@echo "Making additional mogrification to remove hybrid stuff"
	# See note in src/tools/release/src-features-master-list.mk before making
	# changes to this mogrification logic
	@echo "Removing LINUX_POSTMOGRIFY_REMOVAL"
	$(MAKE) MOGRIFY_FLAGS=-DLINUX_POSTMOGRIFY_REMOVAL -f $(MOGRIFY_RULES)
	@$(MARKEND)

#
# Delayed-eval, for use in the recipe, dependent on target-specific values.
# These are delayed-eval, because during the early parts of the build, the
# hybrid shipped filelist is not present.  These macros, therefore, are
# expanded just before the commands to build the rule below are issued.
#
# This is inefficient as heck, but luckily it doesn't get called a whole lot.
#
.CSOURCE = $(filter %.c,${.SHIPPED})
.HSOURCE = $(filter %.h,${.SHIPPED})
.THEREST = $(filter-out %.c %.h,${.SHIPPED})
.MODOBJS = wl.o wl.mod.o wlc_hybrid.o $(patsubst %.c,%.o,$(notdir ${.CSOURCE}))
.EXCLUDE = '^($(call .compose,|,${.MODOBJS}))$$'

# HYBRID_DRIVER_TARGETS e.g. hybrid-fc6-debug-stadef-apdef
hybrid-%: .SHIPPED  = $(call .shipped,$*)
hybrid-%: .CROSSLD  = $(call .crossld,$*)
hybrid-%: .OBJPATH  = $(call .objpath,$*)
hybrid-%: .THEREST += ${HYBRID_NON_CH_FILES}
hybrid-%: hybrid-prep driver-%
	@$(MARKSTART)
	@echo "Eliminate comments and empty lines from $(HYBRID_FILELIST)"
	egrep -v "^#|^ *$$" $(HYBRID_FILELIST) >list-$@
	#
	@echo "Cleaned up $(HYBRID_FILELIST)"
	cat list-$@
	#
	perl -e 'print(join(qq{\n},qw{${.CSOURCE}}).qq{\n})' > cfiles-$@
	#
	# Now cfiles-$@ represents only .c files that we
	# want to ship in hybrid release in the source form (tarball)
	@echo "C files after direct generation"
	cat cfiles-$@
	#
	# Now search through cfiles-$@ list for #included header files
	perl $(PREPROCESSOR_INCLUDES) $(PREPROCESSOR_INCLUDE_PATH) `cat cfiles-$@` | sort -u >hfiles-$@
	@echo "Header files referenced in C files"
	cat hfiles-$@
	#
	@echo "Now concatenate both list-$@ and headers to get chfiles-$@"
	cat list-$@ hfiles-$@ > chlist-$@
	( \
	  grep -v '\.c$$' chlist-$@ ; \
	  perl -e 'print(join(qq{\n},qw{${.CSOURCE}}).qq{\n})' \
	) > list-$@
	rm -f chlist-$@
	#
	# List of .c and .h to be shipped in tarball in source form
	@echo "Combined list of files to copy after removal of hybrid files"
	cat list-$@
	@[ -d $@ ] || mkdir -pv $@
	#
	# Copy those .c and .h files from src to $@ (temporary build workspace
	# for build validation). Basically build a filtered hybrid build tree
	find src components $(FIND_SKIPCMD) -type f -print | perl $(SRCFILTER) -v -e list-$@ | \
		pax -rw $(PAX_SKIPCMD) $@
	#
	# Copy any additional makefiles or config files needed (hybrid specific, if any)
	@echo "Also copying these additional files: ${.THEREST} (if any)"
	perl -le 'while (@ARGV) { print $$ARGV[0]; shift @ARGV; }' ${.THEREST} | pax -rw -v $(PAX_SKIPCMD) $@
	#
	# Next generate a relocatable $(PARTIAL_WL_PFX).o from non-releaseable .o files
	@echo "HYBRID $* OBJECTS" ; ls -l $(call .objpath,$*)/*.o
	cd build/src/wl/linux/obj-$(call config,$@)-$($(call driver_word,$@)-LINUXVER); \
	  $($(call driver_word,$@)-CROSS_COMPILE)ld $($(call driver_word,$@)-32ELF_EMU) -r -o $(PARTIAL_WL_PFX).o `ls *.o | grep -vE ${.EXCLUDE}`
	#
	# Copy PARTIAL_WL_PFX.o to $@/lib
	@[ -d $@/lib ] || mkdir -pv $@/lib
	cp build/src/wl/linux/obj-$(call config,$@)-$($(call driver_word,$@)-LINUXVER)/$(PARTIAL_WL_PFX).o \
	  $@/lib/$(PARTIAL_WL_PFX).o_shipped
	#
	# Copy Hybrid license file
	cp src/doc/HybridBinaryLicense.txt $@/lib/LICENSE.txt
	#
	# Strip local symbols from PARTIAL_WL_PFX.o
	cd $@/lib; strip -x $(PARTIAL_WL_PFX).o_shipped
	#
	# Convert undesired globals to local first and rename to numbered names wlc_<nnnn>
	cd $@/lib; \
	  nm -g $(PARTIAL_WL_PFX).o_shipped | grep -v ' U ' | \
		cut -d ' ' -f 3 >gsyms ; \
	  for g in $(HYBRID_GLOBALS) ; do echo "$$g"; done >keep_gsyms; \
	  grep -v -x -f keep_gsyms gsyms >rename_syms; \
	  objcopy --localize-symbols=rename_syms $(PARTIAL_WL_PFX).o_shipped; \
	  i=0; \
	  rm -fv redefine-syms-list; \
	  for f in `cat rename_syms`; do \
		echo $$f=wlc_$$((i++)) >> redefine-syms-list; \
	  done; \
	  echo "One massive command to rename un-needed globals as wlc_<nnnn> "; \
	  echo "The build contains what symbol was renamed to what. "; \
	  echo "Rename memset and memcpy which might be an "; \
	  echo "undefined symbol because of GCC usage"; \
		echo memset=osl_memset >> redefine-syms-list; \
		echo memcpy=osl_memcpy >> redefine-syms-list; \
		objcopy --redefine-syms redefine-syms-list \
		$(PARTIAL_WL_PFX).o_shipped; \
		echo "List of redefined symbols:"; \
		awk -F= '{printf "%12.12s = %s\n",$$2, $$1}' \
		    redefine-syms-list; \
	  rm -fv gsyms keep_gsyms rename_syms redefine-syms-list
	#
	# Add Linux 2.6 Kbuild makefile
	if [ "$(findstring nodebug,$@)" != "" ]; then \
		cp $(HYBRID_KBUILD_MAKEFILE_SOURCE) $@/$(HYBRID_KBUILD_MAKEFILE_DESTINATION); \
	else \
		cat $(HYBRID_KBUILD_MAKEFILE_SOURCE) | sed 's/#EXTRA_CFLAGS/EXTRA_CFLAGS/' > $@/$(HYBRID_KBUILD_MAKEFILE_DESTINATION); \
	fi
	#
	# Remove extra blank lines which probably came from mogrification from all text files
	#  Make sure we consider whitespace only lines as "blank"
	cd $@; find * $(FIND_SKIPCMD) -print | perl -n -e 'chomp; print "$$_\n" if -f && -T' \
	  | xargs perl -n -i -e 's/^\s+\n$$/\n/; if ($$_ ne "\n"){ print; $$b=0 } elsif (++$$b <= 1) { print } '
	#
	# By now hybrid tree in $@ is all cleaned and pruned. tar it up now
ifeq ($(EMAKE_COMPATIBLE),no)
	cd $@; tar -czvf hybrid-portsrc.tar.gz $(TAR_SKIPCMD) *
else  # EMAKE_COMPATIBLE
	-cd $@; tar -czvf hybrid-portsrc.tar.gz $(TAR_SKIPCMD) *
endif # EMAKE_COMPATIBLE
	# ===== HYBRID PACKAGING STEP IS COMPLETE NOW ==========================
	#
	# ===== NEXT VERIFY IF PACKAGED BITS BUILD CLEAN =======================
	# Create a temp folder to do verification build
	# TO-DO: In this verification step, one can add additional checks like
	# TO-DO: copyright search
	rm -rf $@-tmpd
	mkdir $@-tmpd
	#
	# First Extract the just packaged hybrid-postsrc.tar.gz into $@-tmpd
	#
	tar xzf $@/hybrid-portsrc.tar.gz  $(TAR_SKIPCMD) -C $@-tmpd

	# Check all files in a GPL license tree for Broadcom header comment
	# The header comment should not have line:
	#    "UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation"
	$(VALIDATE_GPL) $@-tmpd

#
#	# WARN: Electric emake will not be able to tee to a log file,
#	# WARN: so do not update make to be $(MAKE) for kernelbuilds.
	if [ "$(findstring 64,$@)" != "" ]; then \
		linuxdir=latest_generic64; \
	else \
		linuxdir=latest_generic32; \
	fi; \
	export KBUILD_DIR=/tools/linux/src/$${linuxdir}; \
	set -o pipefail && make -C $@-tmpd $(CROSS_COMPILE) KBUILD_DIR=$${KBUILD_DIR} KERNELSRC=$${KBUILD_DIR} 2>&1 | tee -a $@/,kbuild.log
	rm -rf $@-tmpd
	@if (grep -i "Warning\|: error:" $@/,kbuild.log | grep -v "WARNING: modpost: missing MODULE_LICENSE() in" | grep -v "WARNING: Symbol version dump") ; then \
		echo >&2 "ERROR: "; \
		echo >&2 "ERROR: Unexpected warnings were found"; \
		echo >&2 "ERROR: and may be fatal."; \
		echo >&2 "======================================="; \
		grep >&2 "Warning:" $@/,kbuild.log; \
		echo >&2 "======================================="; \
		exit 1; \
	fi
	# ===== PACKAGE BUILD VALIDATION IS COMPLETE NOW =======================
	# Now rename the hybrid-portsrc.tar.gz to contain the current target name
	# We will have one for each kernel and target variant
	# WARN: Before renaming this final $@-postsrc.tar.gz consult test teams
	[ -d $@/release ] || mkdir $@/release
	#
	# Move package to the release folder and rename it using a version number convention.
	mv $@/hybrid-portsrc.tar.gz $@/release/$@-$(RELNUM_).tar.gz
	#
	# Replace keyword with actual release number in README.txt
	sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
	 	-e "s/rel_date/$(RELDATE)/g" \
		src/doc/ReadmeReleaseHybridBuild.txt > \
		$@/release/README_$(RELNUM).txt
	#
	# Cleanup after hybrid packaging
	cd release; rm -fv generic-src.tar.gz
	@rm -fv list-$@ cfiles-$@ cfiles1-$@ hfiles-$@
	@$(MARKEND)

# WL source packaging targets. It is a separate as it may require additional
# mogrification, source filtering etc., For now wlsrc*.tar.gz generated is
# same as generic-src.tar.gz generated above, but it may change in future

validate_wl_src: $(RELEASE_WLSRC_TARGETS)

.SECONDEXPANSION:
$(RELEASE_WLSRC_TARGETS): \
  .SHIPPED  = $(call .shipped,$(patsubst validate_wl_src-%,%,$@))
$(RELEASE_WLSRC_TARGETS): \
  .THEREST += ${WLSRC_NON_CH_FILES}
$(RELEASE_WLSRC_TARGETS): $$(patsubst validate_wl_src-%,driver-%,$$@)
ifneq ($(RELEASE_WLSRC),)
	@$(MARKSTART)
	@echo Making additional mogrification to for wlsrc stuff
	@echo Eliminate comments and empty lines from $(SRCFILELIST)
	grep -v "^#" $(SRCFILELIST) >list-$@ ; mv list-$@ $(SRCFILELIST)
	grep -v "^ *$$" $(SRCFILELIST) >list-$@ ; mv list-$@ $(SRCFILELIST)
	@echo Cleaned up $(SRCFILELIST)
	cat $(SRCFILELIST)
	perl -e 'print(join(qq{\n},qw{${.CSOURCE}}).qq{\n})' > cfiles-$@
	@echo "C files after direct generation"
	cat cfiles-$@
	# Compute headers used by cfiles
	perl $(PREPROCESSOR_INCLUDES) $(PREPROCESSOR_INCLUDE_PATH) `cat cfiles-$@` | sort -u >hfiles-$@
	@echo Header files referenced in C files
	cat hfiles-$@
	cat $(SRCFILELIST) hfiles-$@ >list-$@
	@echo Combined list before removal of hybrid files
	cat list-$@
	( \
	  grep -v '\.c$$' list-$@ ; \
	  perl -e 'print(join(qq{\n},qw{${.CSOURCE}}).qq{\n})' \
	) > list-$@
	@echo Combined list of files to copy after removal of hybrid files
	cat list-$@
	install -d $@
	find src components $(FIND_SKIPCMD) -type f -print | perl $(SRCFILTER) -v -e list-$@ | \
		pax -rw $(PAX_SKIPCMD) $@
	@echo Also copying these additional files: ${.THEREST}
	perl -le 'while (@ARGV) { print $$ARGV[0]; shift @ARGV; }' ${.THEREST} | pax -rw -v $(PAX_SKIPCMD) $@
	# Remove extra blank lines which probably came from mogrification from all text files
	#  Make sure we consider whitespace only lines as "blank"
	cd $@; find * $(FIND_SKIPCMD) -print | perl -n -e 'chomp; print "$$_\n" if -f && -T' \
	  | xargs perl -n -i -e 's/^\s+\n$$/\n/; if ($$_ ne "\n"){ print; $$b=0 } elsif (++$$b <= 1) { print } '
	#  Copy non-src files
	perl -le 'while (@ARGV) { print $$ARGV[0]; shift @ARGV; }' $(SRCFILES) | pax -rw $(PAX_SKIPCMD) $@
	cd $@; tree * > $(subst validate_,,$@)-contents.txt
	cd $@; tar -czf $(subst validate_,,$@).tar.gz $(TAR_SKIPCMD) *
	@rm -fv list-$@ cfiles-$@ cfiles1-$@ hfiles-$@
	@$(MARKEND)
endif # RELEASE_WLSRC

validate_wlexe_src: checkout mogrify
ifneq ($(RELEASE_WLEXESRC),)
	@$(MARKSTART)
	install -d $@
	mkdir -p $@/components/shared/
	cp -R components/shared/* $@/components/shared/
	find src components $(FIND_SKIPCMD) -type f -print | \
		perl $(SRCFILTER) -v -e $(WLEXEFILELIST) | \
		pax -rw $(PAX_SKIPCMD) $@
	# Only native compilation of wl.exe needs to be proof tested
	$(MAKE) -C $@/src/wl/exe $(validate_src-opt-gcc)
	$(RM) -r $@/src/wl/exe/obj
	cd $@; tree src > $(subst validate_,,$@)-contents.txt
	cd $@; tar -czf $(subst validate_,,$@).tar.gz $(TAR_SKIPCMD) src
	@$(MARKEND)
endif # RELEASE_WLEXESRC

validate_bcmdl_src: checkout mogrify
ifneq ($(RELEASE_BCMDLSRC),)
	@$(MARKSTART)
	install -d $@
	mkdir -p $@/components/shared/
	cp -R components/shared/* $@/components/shared/
	find src components $(FIND_SKIPCMD) -type f -print | \
		perl $(SRCFILTER) -v -e $(BCMDLFILELIST) | \
		pax -rw $(PAX_SKIPCMD) $@
	$(MAKE) -C $@/src/usbdev/usbdl $(validate_src-opt-gcc)
	rm -f $@/src/usbdev/usbdl/*.o
	cd $@; tree * > $(subst validate_,,$@)-contents.txt
	cd $@; tar -czf $(subst validate_,,$@).tar.gz  $(TAR_SKIPCMD) *
	@$(MARKEND)
endif # RELEASE_BCMDLSRC

package_wl_src: validate_wl_src $(DRIVER_TARGETS)
	@$(MARKSTART)
ifneq ($(RELEASE_WLSRC),)
	cd release; rm -fv generic-src.tar.gz
	# Package wl driver sources and driver modules
	mkdir -p $(RELPKG_DIR)
	-install -p src/doc/WlSrcBinaryLicense.txt $(RELPKG_DIR)/LICENSE.txt
	install -p src/doc/BrandReadmes/$(BRAND).txt $(RELPKG_DIR)/README.txt
	@cp -v validate_wl_src*/*.tar.gz $(RELPKG_DIR)
	mkdir -p $(RELPKG_DIR)/driver
	cd build/src/wl/linux; tar cvpf - $(TAR_SKIPCMD)  \
	 $(foreach d,$(RELEASE_WLSRC_DRIVERS),obj-*-$($(d)-LINUXVER)/wl.ko) | \
	 tar xvpf - $(TAR_SKIPCMD) -C $(PWD)/$(RELPKG_DIR)/driver
	cd release; rm -rfv \
	 $(foreach d,$(RELEASE_WLSRC_DRIVERS),obj-*-$($(d)-LINUXVER))
	# Cleanup after all packaging is completed
	rm -rf validate_wl_src*
endif # RELEASE_WLSRC
ifneq ($(ALL_DNGL_IMAGES),)
	# Package dongle images
	mkdir -p $(RELPKG_DIR)/firmware
	-@for img in $(ALL_DNGL_IMAGES); do \
	   install -dv $(RELPKG_DIR)/firmware/$${img}; \
	   install -pv $(DNGL_IMG_FILES:%=$(DNGL_IMGDIR)/$${img}/%) \
		$(RELPKG_DIR)/firmware/$${img}; \
	done
endif # ALL_DNGL_IMAGES
	@$(MARKEND)

package_wlexe_src: validate_wlexe_src
ifneq ($(RELEASE_WLEXESRC),)
	@$(MARKSTART)
	# Package wl.exe sources and wl.exe binary
	mkdir -p $(RELPKG_DIR)/apps
	install -p validate_wlexe_src/src/wl/exe/wl $(RELPKG_DIR)/apps
	install -pv validate_wlexe_src/*.tar.gz $(RELPKG_DIR)
	cd release; rm -rfv exe
	# Cleanup after all packaging is completed
	rm -rf validate_wlexe_src*
	@$(MARKEND)
endif # RELEASE_WLEXESRC

package_bcmdl_src: validate_bcmdl_src
ifneq ($(RELEASE_BCMDLSRC),)
	@$(MARKSTART)
	# Package usb downloader sources and bcmdl binary
	mkdir -p $(RELPKG_DIR)/apps
	install -p validate_bcmdl_src/src/usbdev/usbdl/bcmdl $(RELPKG_DIR)/apps
	install -pv validate_bcmdl_src/*.tar.gz $(RELPKG_DIR)
	install -pv validate_bcmdl_src/*-contents.txt $(RELPKG_DIR)
	# Cleanup after all packaging is completed
	rm -rf validate_bcmdl_src*
	@$(MARKEND)
endif # RELEASE_BCMDLSRC

package_media_src: release_apps
ifneq ($(RELEASE_MEDIASRC),)
	@$(MARKSTART)
	rm -f release/generic-src.tar.gz
#	find release -type f -print >> media_src_pkg-contents.txt
	@echo Update the file list with the CLM Generated file
	echo "components/clm-api/src/wlc_clm_data.c" >> ./media_src_pkg-contents.txt
	mkdir -p release/packages
	tar -czf release/packages/media_src_pkg.tar.gz  $(TAR_SKIPCMD) -T media_src_pkg-contents.txt
	mv media_src_pkg-contents.txt release/packages
	@$(MARKEND)
endif # RELEASE_MEDIASRC

build_clean: release
	@$(MARKSTART)
	-@rm -rf build
	-@find src components -type f -name "*.o" -exec rm -rf {} +
	@$(MARKEND)

build_end: build_clean
	@$(MARKEND_BRAND)

is32on64: # for build_config consumption
	@echo "$(BRAND) can be build on a 64 bit linux cluster"

isEmakeCompatible:
ifeq ($(EMAKE_COMPATIBLE),no)
	@exit 1
else  # EMAKE_COMPATIBLE
	@exit 0
endif # EMAKE_COMPATIBLE

.PHONY: all clean mogrify prep release validate_generic_wl_src release_driver release_apps $(WL_APP_TARGETS) validate_wl_src validate_wlexe_src validate_bcmdl_src package_wl_src package_wlexe_src package_bcmdl_src package_media_src $(RELEASE_WLSRC_TARGETS) is32on64
