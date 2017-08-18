#
# Common makefile to build android DHD Dongle Image(on Ubuntu).
# Package DHD sources with all sdio/pcie dongle images
#
# NOTE: This makefile should not be used by itself, it is included into
# other brands like android-external-dongle.mk
#
# Author: Kim Lo
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
OEM_LIST           ?= android
WPA_SUPP_TAG       ?= WPA_SUPPLICANT-0-8_00_56
WARN_FILE          := _WARNING_PARTIAL_BUILD_DO_NOT_USE
INPROGRESS_DIR     := $(CURDIR)/BLDTMP/in-progress

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

SRCFILELIST_ORG    := $(SRCDIR)/tools/release/linux-dhd-filelist.txt
SRCFILELIST_GPL_ORG:= $(SRCDIR)/tools/release/linux-dhd-gpl-filelist.txt
SRCFILELIST        := linux-dhd-filelist.txt
SRCFILELIST_GPL    := linux-dhd-gpl-filelist.txt

# OEM flags are per OEM specific, hence derived in OEM specific
# targets
GCCFILESDEFS_OEM =
GCCFILESDEFS_GPL = $(DEFS:%=-D%) -DESTA_OPENSRC_CLEANUP_LIST $(UNDEFS:%=-U%)


VALID_BRANDS := android-external-dongle

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

# Kernels for full z-images
zimage-ics-mr1   := 3.0.8-g49a0ff3-maguro-ics-mr1-4.0.3
zimage-maguro-jb := 3.0.8-maguro-JB
zimage-shamu-ll    := 5.1.1-nexus6-LL
zimage-shamu-interposer    := 5.1.1-nexus6-interposer
zimage-angler-interposer    := 7.0-nexus6p-interposer

# From above list, choose os variants for each oem for host driver
DRIVERS_android      ?= panda-ics panda-jb x86-3-10-20 x86-3-10-20-vendorext x86-kk-intel TPCI-kk jf-kk ja-kk h-kk v1-kk k-kk

# List of OEMS needing Android zImage like builds
# WARN: zImage builds are very lengthy, ensure that only minimum number
# WARN: of targets are selected
OEM_LIST_ZIMAGE     ?= android

# Since we are doing local build, add a unique string to ensure build don't
# steps on each other

DRIVERS_ZIMAGE1_android ?= $(zimage-shamu-ll)
DRIVERS_ZIMAGE2_android ?= $(zimage-shamu-interposer)
DRIVERS_ZIMAGE3_android ?= $(zimage-angler-interposer)

# From above list, choose os variants for each oem for apps
# WARNING:
# WARNING: Only one app variant per CPU arch type per OEM
# WARNING:
APPS_android         ?= panda x86android armandroid

# Generic hardware-less DHD driver make targets
DHD_DRIVERS_android  := $(DRIVERS_android:%=%-dhd-android)

# Define android nexus 6 GIT location
DRIVERS_ZIMAGE1_android_GIT ?= /projects/hnd_swgit_ext2/gerrit_repos/android/nexus/lollipop/kernel/msm.git
DRIVERS_ZIMAGE2_android_GIT ?= /projects/hnd_swgit_ext2/gerrit_repos/android/nexus/lollipop/kernel/msm.git
DRIVERS_ZIMAGE3_android_GIT ?= /projects/hnd_swgit_ext2/gerrit_repos/android/nexus/nyc/kernel/6p-interposer.git

android_GIT_TAG1 ?= android-msm-shamu-3.10-lollipop-mr1
android_GIT_TAG2 ?= android-msm-shamu-3.10-lollipop-mr1
android_GIT_TAG3 ?= android-msm-angler-3.10

# Define android katkit GIT location
intel_kernel ?= kernel_intel-uefi
DRIVERS_module_android_GIT ?= ssh://hnd-swgit:29418/android/x86-kk-local/$(intel_kernel)

# Use bcm-master branch for TOT and TOB build. Use hard-coded tag for now for
# tagged build. Need to change to use config file value for future tagged build
ifdef TAG
android_GIT_TAG ?= 475224e53e446f1f636b37204b38bf3e79004696
else
android_GIT_TAG ?= bcm-master
endif

# App build targets for each oem
APP_DRIVERS_android  := $(APPS_android:%=%-app-android)

#---------------------------------------------------------------
# Options to cross-compile host-side for 3.0.8-maguro-jellybean for android ICS/Galaxy nexus phone
magurojb-args    := LINUXDIR=/tools/linux/src/linux-3.0.8-maguro-JB
magurojb-args    += CROSS_COMPILE=arm-eabi-
magurojb-args    += ARCH=arm
magurojb-args    += PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.4.3/bin:$(PATH)"
magurojb-dhd	 += OEM_ANDROID=1
magurojb-dhd     += $(magurojb-args) dhd-cdc-sdmmc-android-prime-jellybean-cfg80211-oob-gpl
magurojb-dhd     += dhd-cdc-sdmmc-android-prime-jellybean-cfg80211-oob-gpl-debug
#---------------------------------------------------------------
# Options to cross-compile host-side for 5.1.1 -shamu-lollipop
shamull-args           := LINUXDIR=$(INPROGRESS_DIR)/android_$(DRIVERS_ZIMAGE1_android)
shamull-args            += CROSS_COMPILE=arm-eabi-
shamull-args            += ARCH=arm
shamull-args            += PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.8/bin/:$(PATH)"
shamull-dhd             += $(shamull-args) dhd-cdc-sdmmc-android-shamu-lollipop-cfg80211-oob-gpl
shamull-dhd             += dhd-cdc-sdmmc-android-shamu-lollipop-cfg80211-oob-gpl-debug

#---------------------------------------------------------------
# Options to cross-compile host-side for 5.1.1 -shamu-interposer
shamu-interposer-args   := LINUXDIR=$(INPROGRESS_DIR)/android_$(DRIVERS_ZIMAGE2_android)
shamu-interposer-args   += CROSS_COMPILE=arm-eabi-
shamu-interposer-args   += ARCH=arm
shamu-interposer-args   += PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.8/bin/:$(PATH)"
shamu-interposer-dhd    += $(shamu-interposer-args) dhd-cdc-sdmmc-android-shamu-lollipop-cfg80211-oob-gpl
shamu-interposer-dhd    += dhd-cdc-sdmmc-android-shamu-lollipop-cfg80211-oob-gpl-debug

#---------------------------------------------------------------
# Options to cross-compile host-side for 7.0-angler-interposer
angler-interposer-args   := LINUXDIR=$(INPROGRESS_DIR)/android_$(DRIVERS_ZIMAGE3_android)
angler-interposer-args   += CROSS_COMPILE=aarch64-linux-android-
angler-interposer-args   += ARCH=arm
angler-interposer-args   += PATH="/projects/hnd/tools/linux/hndtools-aarch64-linux-android-4.9/bin/:$(PATH)"
angler-interposer-dhd    += $(angler-interposer-args) dhd-cdc-sdmmc-android-cfg80211-oob-gpl
angler-interposer-dhd    += dhd-cdc-sdmmc-android-cfg80211-oob-gpl-debug

#---------------------------------------------------------------
# Options to cross-compile host-side for Galaxy S4(JF_ATT) KK
jf-kk-dhd-opts		:= LINUXDIR=/tools/linux/src/linux-3.4.0-JF_ATT-KK-140109
jf-kk-dhd-opts		+= CROSS_COMPILE=arm-eabi-
jf-kk-dhd-opts		+= ARCH=arm
jf-kk-dhd-opts		+= PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.7/bin/:$(PATH)"
jf-kk-dhd-opts		+= OEM_ANDROID=1
jf-kk-dhd			+= $(jf-kk-dhd-opts)
jf-kk-dhd			+= dhd-Makefile_hw4-android_kk-customer_hw4
#---------------------------------------------------------------
# Options to cross-compile host-side for Galaxy S4(JA) KK
ja-kk-dhd-opts		:= LINUXDIR=/tools/linux/src/linux-3.4.5-JA_3G-KK-140304
ja-kk-dhd-opts		+= CROSS_COMPILE=arm-eabi-
ja-kk-dhd-opts		+= ARCH=arm
ja-kk-dhd-opts		+= PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.7/bin/:$(PATH)"
ja-kk-dhd-opts		+= OEM_ANDROID=1
ja-kk-dhd			+= $(ja-kk-dhd-opts)
ja-kk-dhd			+= dhd-Makefile_hw4-android_kk-customer_hw4
#---------------------------------------------------------------
# Options to cross-compile host-side for Galaxy Note3(H_ATT) KK
h-kk-dhd-opts		:= LINUXDIR=/tools/linux/src/linux-3.4.0-H_ATT-KK-140304
h-kk-dhd-opts		+= CROSS_COMPILE=arm-eabi-
h-kk-dhd-opts		+= ARCH=arm
h-kk-dhd-opts		+= PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.7/bin/:$(PATH)"
h-kk-dhd-opts		+= OEM_ANDROID=1
h-kk-dhd			+= $(h-kk-dhd-opts)
h-kk-dhd			+= dhd-Makefile_hw4-android_kk-customer_hw4
#---------------------------------------------------------------
# Options to cross-compile host-side for Galaxy Note Pro(V1_WIFI) KK
v1-kk-dhd-opts		:= LINUXDIR=/tools/linux/src/linux-3.4.0-V1-WiFi-KK-140203
v1-kk-dhd-opts		+= CROSS_COMPILE=arm-eabi-
v1-kk-dhd-opts		+= ARCH=arm
v1-kk-dhd-opts		+= PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.7/bin/:$(PATH)"
v1-kk-dhd-opts		+= OEM_ANDROID=1
v1-kk-dhd			+= $(v1-kk-dhd-opts)
v1-kk-dhd			+= dhd-Makefile_hw4-android_kk-customer_hw4
#---------------------------------------------------------------
# Options to cross-compile host-side for Galaxy S5(K_EU) KK
k-kk-dhd-opts		:= LINUXDIR=/tools/linux/src/linux-3.4.0-K_LTE_Module-KK-140304
k-kk-dhd-opts		+= CROSS_COMPILE=arm-eabi-
k-kk-dhd-opts		+= ARCH=arm
k-kk-dhd-opts		+= PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.7/bin/:$(PATH)"
k-kk-dhd-opts		+= OEM_ANDROID=1
k-kk-dhd			+= $(k-kk-dhd-opts)
k-kk-dhd			+= dhd-Makefile_hw4-android_kk-customer_hw4
#---------------------------------------------------------------
# Options to cross-compile host-side for Galaxy Note4(T PCIe) KK
TPCI-kk-dhd-opts		:= LINUXDIR=/tools/linux/src/linux-3.10.0-T-PCIE-140306
TPCI-kk-dhd-opts		+= CROSS_COMPILE=arm-eabi-
TPCI-kk-dhd-opts		+= ARCH=arm
TPCI-kk-dhd-opts		+= PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.7/bin/:$(PATH)"
TPCI-kk-dhd-opts		+= OEM_ANDROID=1
TPCI-kk-dhd				+= $(TPCI-kk-dhd-opts)
TPCI-kk-dhd				+= dhd-Makefile_hw4-android_kk-customer_hw4

#---------------------------------------------------------------
# Options to cross-compile host-side (icsmr2 build) for Panda ICS
panda-ics-dhd-opts   := LINUXVER=3.0.8-panda
panda-ics-dhd-opts   += CROSS_COMPILE=arm-eabi-
panda-ics-dhd-opts   += ARCH=arm
panda-ics-dhd-opts   += PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.4.0/bin:$(PATH)"
panda-ics-dhd-opts   += OEM_ANDROID=1
panda-ics-dhd        := $(panda-ics-dhd-opts)
panda-ics-dhd        += dhd-cdc-sdmmc-android-panda-cfg80211-oob
panda-ics-dhd        += dhd-cdc-sdmmc-android-panda-cfg80211-oob-debug
panda-ics-dhd        += dhd-cdc-sdmmc-android-panda-icsmr1-cfg80211-oob
panda-ics-dhd        += dhd-cdc-sdmmc-android-panda-icsmr1-cfg80211-oob-debug
#---------------------------------------------------------------
# Options to cross-compile host-side for Panda JB
panda-jb-dhd-opts   := LINUXVER=3.2.0-panda
panda-jb-dhd-opts   += CROSS_COMPILE=arm-eabi-
panda-jb-dhd-opts   += ARCH=arm
panda-jb-dhd-opts   += PATH="/projects/hnd/tools/linux/hndtools-arm-eabi-4.4.0/bin:$(PATH)"
panda-jb-dhd-opts   += OEM_ANDROID=1
panda-jb-dhd        := $(panda-jb-dhd-opts)
panda-jb-dhd        += dhd-cdc-sdmmc-android-panda-cfg80211-oob
panda-jb-dhd        += dhd-cdc-sdmmc-android-panda-cfg80211-oob-debug
#---------------------------------------------------------------
# Options to compile host-side for 3.10 android x86
x86-3-10-args       := LINUXVER=3.10.11-aia1+-androidx86
x86-3-10-args       += CROSS_COMPILE=x86_64-linux-
x86-3-10-args       += ARCH=x86
x86-3-10-args       += PATH="/projects/hnd/tools/linux/hndtools-x86_64-linux-glibc2.7-4.6/bin/:$(PATH)"
x86-3-10-dhd        := $(x86-3-10-args)
x86-3-10-dhd        += dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-debug
#---------------------------------------------------------------
# Options to compile host-side for 3.10.20 64bit Kitkat android x86 With VENDOR EXTN. Support
x86-3-10-20-vendorext-args       := LINUXVER=3.10.20-vendorext-gcc264a6-androidx86
x86-3-10-20-vendorext-args       += CROSS_COMPILE=x86_64-linux-
x86-3-10-20-vendorext-args       += ARCH=x86
x86-3-10-20-vendorext-args       += PATH="/projects/hnd/tools/linux/hndtools-x86_64-linux-glibc2.7-4.6/bin/:$(PATH)"
x86-3-10-20-vendorext-dhd        := $(x86-3-10-20-vendorext-args)
x86-3-10-20-vendorext-dhd        += dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-debug
x86-3-10-20-vendorext-dhd        += dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-customer4 dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-customer4-debug
x86-3-10-20-vendorext-dhd        += dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-hal-debug dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-hal-customer4-debug
x86-3-10-20-vendorext-dhd        += dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-hal-nan dhd-msgbuf-pciefd-android-cfg80211-hal-nan
x86-3-10-20-vendorext-dhd        += dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-hal-nan-debug dhd-msgbuf-pciefd-android-cfg80211-hal-nan-debug
#---------------------------------------------------------------
# Options to compile host-side for 3.10.20 64bit Kitkat android x86
x86-3-10-20-args       := LINUXVER=3.10.20-gcc264a6-androidx86
x86-3-10-20-args       += CROSS_COMPILE=x86_64-linux-
x86-3-10-20-args       += ARCH=x86
x86-3-10-20-args       += PATH="/projects/hnd/tools/linux/hndtools-x86_64-linux-glibc2.7-4.6/bin/:$(PATH)"
x86-3-10-20-dhd        := $(x86-3-10-20-args)
x86-3-10-20-dhd        += dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-debug
x86-3-10-20-dhd        += dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-customer4 dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-customer4-debug
#---------------------------------------------------------------
# Options to compile Intel uefi 64bit Kitkat android x86
x86-kk-intel-args       := LINUXDIR=$(BUILD_BASE)/kernel_intel-uefi
x86-kk-intel-args       += CROSS_COMPILE=/projects/hnd/tools/linux/hndtools-x86_64-linux-glibc2.7-4.6/bin/x86_64-linux-
x86-kk-intel-args       += ARCH=x86
x86-kk-intel-args       += PATH="/projects/hnd/tools/linux/hndtools-x86_64-linux-glibc2.7-4.6/bin/:$(PATH)"
x86-kk-intel-dhd        := $(x86-kk-intel-args)
x86-kk-intel-dhd        += dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-debug
#---------------------------------------------------------------
# Build app wl/dhd for this os variant
panda-app        := TARGETENV=android_ndk_r6b TARGETARCH=arm_android_ndk_r6b TARGETOS=unix
panda-app        += TARGET_PREFIX=/projects/hnd_tools/linux/android-ndk-r9/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86_64/bin/arm-linux-androideabi-
panda-app        += TARGET_NDK=/projects/hnd_tools/linux/android-ndk-r6b LINUXDIR=/tools/linux/src/linux-3.0.3-panda
#---------------------------------------------------------------
# Build android x86 nl80211 version wl/dhd
x86android-app   := TARGETENV=android_ndk_r6b TARGETARCH=x86_android_ndk_r6b TARGETOS=unix
x86android-app   += TARGET_PREFIX=/projects/hnd_tools/linux/android-ndk-r6b/toolchains/x86-4.4.3/prebuilt/linux-x86/bin/i686-android-linux-
x86android-app   += TARGET_NDK=/projects/hnd_tools/linux/android-ndk-r6b LINUXDIR=/tools/linux/src/linux-4.0.4-301.fc22.x86_64.full
x86android-app   += NL80211=1 NLHEADERS_PATH=/projects/hnd_tools/linux/android-libnl/libnl-headers-4.2 NLSTATICLIB_PATH=/projects/hnd_tools/linux/android-libnl/libnl_2-4.2-x86.a
#---------------------------------------------------------------
# Build android arm nl80211 version wl
armandroid-app   := TARGETOS=unix TARGETARCH=arm_android TARGETENV=android
armandroid-app   += TARGET_PREFIX=/projects/hnd/tools/linux/arm-linux-androideabi-4.6/bin/arm-linux-androideabi-
armandroid-app   += NL80211=1
armandroid-app   += NLHEADERS_PATH=/projects/hnd_tools/linux/android-libnl/libnl-headers-4.2-nokernel
armandroid-app   += NLSTATICLIB_PATH=/projects/hnd_tools/linux/android-libnl/libnl_2-4.2-arm.a
#---------------------------------------------------------------

# Cmd line args/options for wps subbuild (centralize references to tools used)
WPS_ARGS_bcm     := $(32GCC64PATH)
WPS_ARGS_bcmsglx := CC="arm-linux-gcc"
WPS_ARGS_bcmsglx += PATH="/projects/hnd/tools/linux/hndtools-arm-linux-3.4.3/bin:$(PATH)"

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
COMMONUNDEFS += BCMDBUS
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
#DHD_UNDEFS +=

# These symbols will be DEFINED in the source code by the mogrifier
#

# DEFS common to both DHD & HSL build
COMMONDEFS += BCM47XX
COMMONDEFS += BCM47XX_CHOPS

# additional DEFS for HSL build
P2P_DEFS += BCMCRYPTO
P2P_DEFS += LINUX
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
DEFS_voipbcm  += -DOEM_BCM -UWLNOKIA
DEFS_voipbcm  += -UOEM_ANDROID
DEFS_nokia    += -UOEM_BCM -DWLNOKIA
DEFS_nokia    += -UOEM_ANDROID
DEFS_android  += -DOEM_ANDROID -DESTA_POSTMOGRIFY_REMOVAL
DEFS_android  += -UOEM_BCM -UWLNOKIA
DEFS_android  += -UOEM_SAMSUNG -UBCMSPI -USDHOST3
DEFS_android  += -UBCMNVRAM -URWL_DONGLE -UUART_REFLECTOR -UDHDARCH_MIPS
DEFS_android  += -UBCMINTERNAL -UCUSTOMER_HW4 -UCUSTOMER_HW4_RELEASE -UMCAST_LIST_ACCUMULATION
DEFS_android  += -UROAM_CHANNEL_CACHE -UUSE_INITIAL_2G_SCAN -UDUAL_ESCAN_RESULT_BUFFER
DEFS_android  += -UNOT_YET -UOKC_SUPPORT -UFULL_ROAMING_SCAN_PERIOD_60
DEFS_android  += -UBLOCK_IPV6_PACKET -UPASS_IPV4_SUSPEND -USUPPORT_DEEP_SLEEP
DEFS_android  += -UWL_CFG80211_GON_COLLISION -UWL_CFG80211_USE_PRB_REQ_FOR_AF_TX
DEFS_android  += -UDHD_PM_CONTROL_FROM_FILE -UROAM_API -USUPPORT_AUTO_CHANNEL
DEFS_android  +=  -USUPPORT_HIDDEN_AP -UUSE_CID_CHECK -USUPPORT_AMPDU_MPDU_CMD
DEFS_android  +=  -USUPPORT_SOFTAP_SINGL_DISASSOC
DEFS_android  +=  -USIMPLE_ISCAN -USUPPORT_MULTIPLE_REVISION
DEFS_android  +=  -UCUSTOMER_SET_COUNTRY -UDHD_DEBUG_WAKE_LOCK -UBCMSPI_ANDROID
DEFS_android  +=  -UUSE_WEP_AUTH_SHARED_OPEN -UROAM_AP_ENV_DETECTION -strip_bcmromfn
DEFS_android  +=  -UCONFIG_ARCH_RHEA -UCONFIG_ARCH_CAPRI
DEFS_android_special := $(DEFS_android)
DEFS_samsung  += -UOEM_BCM -UWLNOKIA
DEFS_samsung  += -DOEM_ANDROID -DOEM_SAMSUNG -USDHOST3
DEFS_samsung  += -UBCMNVRAM -URWL_DONGLE -UUART_REFLECTOR -UDHDARCH_MIPS
DEFS_samsung  += -UCCX_SDK -strip_bcmromfn
# Can't mogrify this out, breaks wl utility: DEFS_android  += -UD11AC_IOTYPES


# These are extensions of source files that should be mogrified
# (using the parameters specified in DEFS and UNDEFS above.)
#
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

all: build_start checkout mogrify filelists prebuild_prep prepare_kernel build_dhd_zimage build_android_dhd build_android_apps release build_end

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
	cat $(SRCFILELIST_ORG) $(SRCFILELISTS_COMPONENTS) | \
		src/tools/release/rem_comments.awk > master_filelist.h
	cat $(SRCFILELIST_GPL_ORG) $(SRCFILELISTS_GPL_COMPONENTS) | \
		src/tools/release/rem_comments.awk > master_filelist_gpl.h
	gcc -E -undef  $(DEFS:%=-D%_SRC) -DESTA_OPENSRC_CLEANUP_LIST_SRC \
			$(UNDEFS:%=-DNO_%_SRC) \
			-Ulinux -o $(SRCFILELIST_GPL) master_filelist_gpl.h
	@$(MARKEND)

mogrify: checkout
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

	$(FIND) src components $(FIND_SKIPCMD) -print | $(SRCFILTER) -v $(SRCFILELIST)_$(oem) | col -b | \
	$(GREP_SKIPCMD)  | \
	pax -rw $(PAX_SKIPCMD)  build/$(oem); \
	if [ "$(oem)" == "android" ]; then \
	        $(FIND) src components $(FIND_SKIPCMD) -print | $(SRCFILTER) -v $(SRCFILELIST_HSL_android) | col -b | \
		$(GREP_SKIPCMD) | \
		pax -rw $(PAX_SKIPCMD) build/$(oem); \
		cd build/$(oem); \
	fi

	@$(MARKEND)
#disable#End of prebuild_$(oem)_prep

prepare_kernel:
	git clone $(DRIVERS_module_android_GIT)
	cd $(intel_kernel) && \
	make ARCH=x86  CROSS_COMPILE=/projects/hnd/tools/linux/hndtools-x86_64-linux-glibc2.7-4.6/bin/x86_64-linux-  defconfig  x86_64_bigcore_android_defconfig && \
	make ARCH=x86  CROSS_COMPILE=/projects/hnd/tools/linux/hndtools-x86_64-linux-glibc2.7-4.6/bin/x86_64-linux- $(or $(MAKEJOBS),-j16)


## ------------------------------------------------------------------
## Build DHD (generic) for all oems
## ------------------------------------------------------------------
build_dhd: $(foreach oem,$(OEM_LIST),build_$(oem)_dhd)

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
		$(if $(findstring android,$(oem)),OEM_ANDROID=1) \
		$(if $(findstring samsung,$(oem)),OEM_ANDROID=1 OEM_SAMSUNG=1) \
		$(opts) \
		$(BUILD_$(oem)_DHD_TARGETS) $(or $(MAKEJOBS),-j16)
	@$(MARKEND)

## ------------------------------------------------------------------
## Build oem brand specific apps
## ------------------------------------------------------------------
build_apps: $(foreach oem,$(OEM_LIST),build_$(oem)_apps)

build_android_apps: oem=android
build_android_apps: $(APP_DRIVERS_android)

#- Common app build steps for all oems
$(foreach oem,$(OEM_LIST),$(APP_DRIVERS_$(oem))):: opts=$($(subst -$(oem),,$@))
$(foreach oem,$(OEM_LIST),$(APP_DRIVERS_$(oem)))::
	@$(MARKSTART)
	$(MAKE) -C build/$(oem)/src/wl/exe \
		$(if $(BCM_MFGTEST), WLTEST=1) $(opts) $(or $(MAKEJOBS),-j16)
	$(MAKE) -C build/$(oem)/src/dhd/exe \
		$(if $(BCM_MFGTEST), WLTEST=1) $(opts) $(or $(MAKEJOBS),-j16)
	# wlm fails to cross compile on arm, forcing x86 only
	# $(MAKE) -C build/$(oem)/src/wl/exe wlm $($@)
	$(MAKE) -C build/$(oem)/src/wl/exe wlm $(32GCC64) $(or $(MAKEJOBS),-j16)
	@$(MARKEND)

#- Android specific app builds only for fc9g
$(filter fc9g%,$(APP_DRIVERS_android)):: opts=$($(subst -android,,$@))
$(filter fc9g%,$(APP_DRIVERS_android))::
#disabled#	$(MAKE) -C src/wps/android $(opts)


ifeq ($(BRAND),android-external-dongle)

build_dhd_zimage: checkout mogrify
build_dhd_zimage: $(OEM_LIST_ZIMAGE:%=fetch-kernel-%)
build_dhd_zimage: $(OEM_LIST_ZIMAGE:%=setup-bcmdhdws-%)
build_dhd_zimage: $(OEM_LIST_ZIMAGE:%=build-bcmdhd-%)
build_dhd_zimage: $(OEM_LIST_ZIMAGE:%=release-bcmdhd-%)
##build_dhd_zimage: $(OEM_LIST_ZIMAGE:%=clean-kernel-%)

# Check out kernel from GIT repository
$(OEM_LIST_ZIMAGE:%=fetch-kernel-%): oem=$(subst fetch-kernel-,,$@)
$(OEM_LIST_ZIMAGE:%=fetch-kernel-%): blddir=build/$(oem)
$(OEM_LIST_ZIMAGE:%=fetch-kernel-%): checkout mogrify
	@$(MARKSTART)
	@for ksrc in $(DRIVERS_ZIMAGE1_$(oem)); \
	do \
		if [ -d "$(DRIVERS_ZIMAGE1_$(oem)_GIT)" ]; then \
		   mkdir -p  $(INPROGRESS_DIR)/$(oem)_$$ksrc; \
		   cd $(INPROGRESS_DIR)/$(oem)_$$ksrc; \
		   date +"[%D %T] Start check out kernel"; \
		   echo "git clone $(DRIVERS_ZIMAGE1_$(oem)_GIT) ."; \
		   git clone $(DRIVERS_ZIMAGE1_$(oem)_GIT) . ; \
		   echo "git checkout $($(oem)_GIT_TAG1)"; \
		   git checkout $($(oem)_GIT_TAG1); \
		   date +"[%D %T] End check out kernel"; \
		else \
		   echo "ERROR: Missing GIT repository $(DRIVERS_ZIMAGE1_$(oem)_GIT)"; \
		   exit 1; \
		fi; \
	done

	@for ksrc in $(DRIVERS_ZIMAGE2_$(oem)); \
	do \
		if [ -d "$(DRIVERS_ZIMAGE2_$(oem)_GIT)" ]; then \
		   mkdir -p  $(INPROGRESS_DIR)/$(oem)_$$ksrc; \
		   cd $(INPROGRESS_DIR)/$(oem)_$$ksrc; \
		   date +"[%D %T] Start check out kernel"; \
		   echo "git clone $(DRIVERS_ZIMAGE2_$(oem)_GIT) ."; \
		   git clone $(DRIVERS_ZIMAGE2_$(oem)_GIT) . ; \
		   echo "git checkout $($(oem)_GIT_TAG2)"; \
		   git checkout $($(oem)_GIT_TAG2); \
		   date +"[%D %T] End check out kernel"; \
		else \
		   echo "ERROR: Missing GIT repository $(DRIVERS_ZIMAGE2_$(oem)_GIT)"; \
		   exit 1; \
		fi; \
	done

	@for ksrc in $(DRIVERS_ZIMAGE3_$(oem)); \
	do \
		if [ -d "$(DRIVERS_ZIMAGE3_$(oem)_GIT)" ]; then \
		   mkdir -p  $(INPROGRESS_DIR)/$(oem)_$$ksrc; \
		   cd $(INPROGRESS_DIR)/$(oem)_$$ksrc; \
		   date +"[%D %T] Start check out kernel"; \
		   echo "git clone $(DRIVERS_ZIMAGE3_$(oem)_GIT) ."; \
		   git clone $(DRIVERS_ZIMAGE3_$(oem)_GIT) . ; \
		   echo "git checkout $($(oem)_GIT_TAG3)"; \
		   git checkout $($(oem)_GIT_TAG3); \
		   date +"[%D %T] End check out kernel"; \
		else \
		   echo "ERROR: Missing GIT repository $(DRIVERS_ZIMAGE3_$(oem)_GIT)"; \
		   exit 1; \
		fi; \
	done
	@$(MARKEND)

# TO-DO: Maintenance of svn-external-filelist.txt may be a challenge
# TO-DO: to keep in sync with src/dhd/linux/Makefile SOURCES list
# TO-DO: Rename the file and see if it can be generated dynamically
# TO-DO: From CFILES and VPATH
$(OEM_LIST_ZIMAGE:%=setup-bcmdhdws-%): oem=$(subst setup-bcmdhdws-,,$@)
$(OEM_LIST_ZIMAGE:%=setup-bcmdhdws-%): blddir=build/$(oem)
$(OEM_LIST_ZIMAGE:%=setup-bcmdhdws-%): checkout mogrify
	@$(MARKSTART)
	@for ksrc in $(DRIVERS_ZIMAGE1_$(oem)); \
	do \
		rm -rfv $(INPROGRESS_DIR)/$(oem)_$$ksrc/drivers/net/wireless/bcmdhd; \
		cd $(SRCDIR)/bcmdhd/mk; \
		make -f bcmdhd.mk BCMDHD_OEM=google BCMDHD_WORK_DIR=$(INPROGRESS_DIR)/test mogrify_only ; \
		mv -fv $(INPROGRESS_DIR)/test/google $(INPROGRESS_DIR)/$(oem)_$$ksrc/drivers/net/wireless/bcmdhd; \
		rm -rfv $(INPROGRESS_DIR)/test; \
	done
	@for ksrc in $(DRIVERS_ZIMAGE2_$(oem)); \
	do \
		rm -rfv $(INPROGRESS_DIR)/$(oem)_$$ksrc/drivers/net/wireless/bcmdhd; \
		cd $(SRCDIR)/bcmdhd/mk; \
		make -f bcmdhd.mk BCMDHD_OEM=interposer_customer4 BCMDHD_WORK_DIR=$(INPROGRESS_DIR)/test mogrify_only ; \
		mv -fv $(INPROGRESS_DIR)/test/interposer_customer4 $(INPROGRESS_DIR)/$(oem)_$$ksrc/drivers/net/wireless/bcmdhd; \
		rm -rfv $(INPROGRESS_DIR)/test; \
	done
	@for ksrc in $(DRIVERS_ZIMAGE3_$(oem)); \
	do \
		rm -rfv $(INPROGRESS_DIR)/$(oem)_$$ksrc/drivers/net/wireless/bcmdhd; \
		cd $(SRCDIR)/bcmdhd/mk; \
		make -f bcmdhd.mk BCMDHD_OEM=interposer_customer4 BCMDHD_WORK_DIR=$(INPROGRESS_DIR)/test mogrify_only ; \
		mv -fv $(INPROGRESS_DIR)/test/interposer_customer4 $(INPROGRESS_DIR)/$(oem)_$$ksrc/drivers/net/wireless/bcmdhd; \
		rm -rfv $(INPROGRESS_DIR)/test; \
	done
	@$(MARKEND)

# TO-DO: Replace build.sh with actual $(MAKE) commands
$(OEM_LIST_ZIMAGE:%=build-bcmdhd-%): oem=$(subst build-bcmdhd-,,$@)
$(OEM_LIST_ZIMAGE:%=build-bcmdhd-%): blddir=build/$(oem)
$(OEM_LIST_ZIMAGE:%=build-bcmdhd-%): checkout mogrify
	@$(MARKSTART)
	$(FIND) $(INPROGRESS_DIR)/$(oem)_$(DRIVERS_ZIMAGE1_$(oem))/drivers/net/wireless/bcmdhd $(MOGRIFY_EXCLUDE) -type f -print  | \
		xargs $(MOGRIFY) $(DEFS_$(oem)) -DESTA_POSTMOGRIFY_REMOVAL | \
		xargs $(MOGRIFY_SCRIPT) -translate_open_to_dual_copyright
	mkdir -pv $(RELEASEDIR)/$(oem)/dongle-host-driver-source-open-kernel-build/
	cp -rfv $(INPROGRESS_DIR)/$(oem)_$(DRIVERS_ZIMAGE1_$(oem))/drivers/net/wireless/bcmdhd $(RELEASEDIR)/$(oem)/dongle-host-driver-source-open-kernel-build/
	@for ksrc in $(DRIVERS_ZIMAGE1_$(oem));\
	do \
		$(MAKE) -C $(INPROGRESS_DIR)/$(oem)_$$ksrc/drivers/net/wireless/bcmdhd/include; \
		cd $(INPROGRESS_DIR)/$(oem)_$$ksrc; \
		PATH=/bin:$(PATH)  && \
		make ARCH=arm CROSS_COMPILE=/projects/hnd/tools/linux/hndtools-arm-eabi-4.8/bin/arm-eabi- shamu_defconfig && \
		make ARCH=arm CROSS_COMPILE=/projects/hnd/tools/linux/hndtools-arm-eabi-4.8/bin/arm-eabi- $(or $(MAKEJOBS),-j4) ; \
		mkdir -pv $(RELEASEDIR)/$(oem)/host/bootimage-$$ksrc; \
		cp -fpuv $(INPROGRESS_DIR)/$(oem)_$$ksrc/arch/arm/boot/zImage-dtb $(RELEASEDIR)/$(oem)/host/bootimage-$$ksrc/; \
		/projects/hnd/tools/bin/mkbootimg --kernel $(INPROGRESS_DIR)/$(oem)_$$ksrc/arch/arm/boot/zImage-dtb --ramdisk /projects/hnd/tools/ramdisk_shamu/ramdisk.img --cmdline "console=ttyHSL0,115200,n8 androidboot.console=ttyHSL0 androidboot.hardware=shamu msm_rtb.filter=0x37 ehci-hcd.park=3 utags.blkdev=/dev/block/platform/msm_sdcc.1/by-name/utags utags.backup=/dev/block/platform/msm_sdcc.1/by-name/utagsBackup coherent_pool=8M" --base 0x00000000 --pagesize 2048 --ramdisk_offset BOARD_RAMDISK_OFFSET --tags_offset BOARD_KERNEL_TAGS_OFFSET --output $(RELEASEDIR)/$(oem)/host/bootimage-$$ksrc/boot.img ; \
	done

	$(FIND) $(INPROGRESS_DIR)/$(oem)_$(DRIVERS_ZIMAGE2_$(oem))/drivers/net/wireless/bcmdhd $(MOGRIFY_EXCLUDE) -type f -print  | \
		xargs $(MOGRIFY) $(DEFS_$(oem)) -DESTA_POSTMOGRIFY_REMOVAL | \
		xargs $(MOGRIFY_SCRIPT) -translate_open_to_dual_copyright
	mkdir -pv $(RELEASEDIR)/$(oem)/dongle-host-driver-source-open-kernel-build/
	cp -rfv $(INPROGRESS_DIR)/$(oem)_$(DRIVERS_ZIMAGE2_$(oem))/drivers/net/wireless/bcmdhd $(RELEASEDIR)/$(oem)/dongle-host-driver-source-open-kernel-build/
	@for ksrc in $(DRIVERS_ZIMAGE2_$(oem));\
	do \
		$(MAKE) -C $(INPROGRESS_DIR)/$(oem)_$$ksrc/drivers/net/wireless/bcmdhd/include; \
		cd $(INPROGRESS_DIR)/$(oem)_$$ksrc; \
		PATH=/bin:$(PATH)  && \
		make ARCH=arm CROSS_COMPILE=/projects/hnd/tools/linux/hndtools-arm-eabi-4.8/bin/arm-eabi- shamu_interposer_defconfig && \
		make ARCH=arm CROSS_COMPILE=/projects/hnd/tools/linux/hndtools-arm-eabi-4.8/bin/arm-eabi- $(or $(MAKEJOBS),-j4) ; \
		mkdir -pv $(RELEASEDIR)/$(oem)/host/bootimage-$$ksrc; \
		cp -fpuv $(INPROGRESS_DIR)/$(oem)_$$ksrc/arch/arm/boot/zImage-dtb $(RELEASEDIR)/$(oem)/host/bootimage-$$ksrc/; \
		/projects/hnd/tools/bin/mkbootimg --kernel $(INPROGRESS_DIR)/$(oem)_$$ksrc/arch/arm/boot/zImage-dtb --ramdisk /projects/hnd/tools/ramdisk_shamu/ramdisk.img --cmdline "console=ttyHSL0,115200,n8 androidboot.console=ttyHSL0 androidboot.hardware=shamu msm_rtb.filter=0x37 ehci-hcd.park=3 utags.blkdev=/dev/block/platform/msm_sdcc.1/by-name/utags utags.backup=/dev/block/platform/msm_sdcc.1/by-name/utagsBackup coherent_pool=8M" --base 0x00000000 --pagesize 2048 --ramdisk_offset BOARD_RAMDISK_OFFSET --tags_offset BOARD_KERNEL_TAGS_OFFSET --output $(RELEASEDIR)/$(oem)/host/bootimage-$$ksrc/boot.img ; \
	done

	$(FIND) $(INPROGRESS_DIR)/$(oem)_$(DRIVERS_ZIMAGE3_$(oem))/drivers/net/wireless/bcmdhd $(MOGRIFY_EXCLUDE) -type f -print  | \
		xargs $(MOGRIFY) $(DEFS_$(oem)) -DESTA_POSTMOGRIFY_REMOVAL | \
		xargs $(MOGRIFY_SCRIPT) -translate_open_to_dual_copyright
	mkdir -pv $(RELEASEDIR)/$(oem)/dongle-host-driver-source-open-kernel-build/
	cp -rfv $(INPROGRESS_DIR)/$(oem)_$(DRIVERS_ZIMAGE3_$(oem))/drivers/net/wireless/bcmdhd $(RELEASEDIR)/$(oem)/dongle-host-driver-source-open-kernel-build/
	@for ksrc in $(DRIVERS_ZIMAGE3_$(oem));\
	do \
		$(MAKE) -C $(INPROGRESS_DIR)/$(oem)_$$ksrc/drivers/net/wireless/bcmdhd/include; \
		cd $(INPROGRESS_DIR)/$(oem)_$$ksrc; \
		PATH=/bin:$(PATH)  && \
		make ARCH=arm64 CROSS_COMPILE=/projects/hnd/tools/linux/hndtools-aarch64-linux-android-4.9/bin/aarch64-linux-android- angler_interposer_defconfig && \
		make ARCH=arm64 CROSS_COMPILE=/projects/hnd/tools/linux/hndtools-aarch64-linux-android-4.9/bin/aarch64-linux-android- $(or $(MAKEJOBS),-j4) ; \
		mkdir -pv $(RELEASEDIR)/$(oem)/host/bootimage-$$ksrc; \
		cp -fpuv $(INPROGRESS_DIR)/$(oem)_$$ksrc/arch/arm64/boot/Image.gz-dtb $(RELEASEDIR)/$(oem)/host/bootimage-$$ksrc/; \
	done
	@$(MARKEND)

$(OEM_LIST_ZIMAGE:%=release-bcmdhd-%): oem=$(subst release-bcmdhd-,,$@)
$(OEM_LIST_ZIMAGE:%=release-bcmdhd-%): blddir=build/$(oem)
$(OEM_LIST_ZIMAGE:%=release-bcmdhd-%): checkout mogrify
	@$(MARKSTART)
	tar cpf $(RELEASEDIR)/$(oem)/dongle-host-driver-source-open-kernel-build.tar -C $(RELEASEDIR)/$(oem) dongle-host-driver-source-open-kernel-build
	gzip -9fv $(RELEASEDIR)/$(oem)/dongle-host-driver-source-open-kernel-build.tar
	rm -rfv $(RELEASEDIR)/$(oem)/dongle-host-driver-source-open-kernel-build
	@$(MARKEND)

$(OEM_LIST_ZIMAGE:%=clean-kernel-%): oem=$(subst clean-kernel-,,$@)
$(OEM_LIST_ZIMAGE:%=clean-kernel-%): blddir=build/$(oem)
$(OEM_LIST_ZIMAGE:%=clean-kernel-%): checkout mogrify
	@$(MARKSTART)
	@for ksrc in $(DRIVERS_ZIMAGE1_$(oem)); \
	do \
		echo "rm -rf $(INPROGRESS_DIR)/$(oem)_$$ksrc"; \
		rm -rf $(INPROGRESS_DIR)/$(oem)_$$ksrc; \
	done
	@date +"END:   $@, %D %T"  | tee -a profile.log

	@for ksrc in $(DRIVERS_ZIMAGE2_$(oem)); \
	do \
		echo "rm -rf $(INPROGRESS_DIR)/$(oem)_$$ksrc"; \
		rm -rf $(INPROGRESS_DIR)/$(oem)_$$ksrc; \
	done
	@date +"END:   $@, %D %T"  | tee -a profile.log

	@for ksrc in $(DRIVERS_ZIMAGE3_$(oem)); \
	do \
		echo "rm -rf $(INPROGRESS_DIR)/$(oem)_$$ksrc"; \
		rm -rf $(INPROGRESS_DIR)/$(oem)_$$ksrc; \
	done
	@date +"END:   $@, %D %T"  | tee -a profile.log
else # BRAND

# For non-external brands, don't build android zImage
build_dhd_zimage:

endif # BRAND

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
		-install -v src/doc/BrandReadmes/$(BRAND).txt $(RELDIR)/README.txt 2>$(NULL); \
	fi
	if test -e src/doc/BrandReadmes/$(BRAND)_oem_$(oem).txt; then \
		-install -v src/doc/BrandReadmes/$(BRAND)_oem_$(oem).txt $(RELDIR)/README.txt 2>$(NULL); \
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
	# Copying native and other cross-compiled wl and dhd exe apps
	@echo "Copying native and cross compiled apps for $(foreach app,$(APP_DRIVERS_$(oem)),$(subst -$(oem),,$(app))) variants"
	for arch in $(foreach app,$(APP_DRIVERS_$(oem)),$(if $(filter TARGETARCH%, $($(patsubst %-$(oem),%,$(app)))),$(patsubst TARGETARCH=%,%,$(filter TARGETARCH%, $($(patsubst %-$(oem),%,$(app))))),x86)); do \
		if [ -d build/$(oem)/src/wl/exe/obj/wl/$$arch ]; then \
			$(MAKE) -C build/$(oem)/src/wl/exe TARGETARCH=$$arch INSTALL_DIR=$(RELEASEDIR)/$(oem)/apps release_bins $(or $(MAKEJOBS),-j16); \
		fi; \
		if [ "$$arch" != "x86" ]; then \
			sarch=$$arch; \
		fi; \
		install -pv build/$(oem)/src/dhd/exe/dhd$$sarch $(RELDIR)/apps; \
		install -pv build/$(oem)/src/wl/exe/wl$$sarch $(RELDIR)/apps; \
	done
	#wlm fails to cross compile on arm, forcing x86 only
	@if [ "$(oem)" == "bcm" ]; then \
		install -p build/$(oem)/src/wl/exe/wlm/x86/wlm.so $(RELDIR)/apps; \
	fi
	install src/doc/BCMLogo.gif $(RELEASEDIR)/$(oem)
#comment# Copy zImage from 3.0.8 or 3.2.0 kernel tree to release/android/host tree
#comment# Need to come up with a better way to integrate kernel image
#comment# Copy wpa_supplicant binaries to release/android/apps tree
	@if [ "$(oem)" == "android" ]; then \
		for mdir in release/android/host/*3.0.8-panda; do \
			install -p /tools/linux/src/linux-3.0.8-panda/arch/arm/boot/zImage $${mdir}; \
		done; \
		for mdir in release/android/host/*3.2.0-panda; do \
			install -p /tools/linux/src/linux-3.2.0-panda/arch/arm/boot/zImage $${mdir}; \
		done; \
		cp -a /projects/hnd_embedded/Android/BRCM_WPA_SUPPLICANT_HOSTAPD/wpa_supplicant_8.56/* release/android/apps; \
		mkdir -p -v release/android/apps/wpa_supp ; \
		LATEST_DIR=`ls -1d /projects/hnd/swbuild/build_linux/$(WPA_SUPP_TAG)/ubuntu-external-wpa-supp/* | sort -t. -n -k1,1 -k2,2 -k3,3 -k4,4 | tail -1`; \
		LATEST_BUILD=`basename $${LATEST_DIR}` ; \
		for mdir in /projects/hnd/swbuild/build_linux/$(WPA_SUPP_TAG)/ubuntu-external-wpa-supp/$${LATEST_BUILD}/release/base* ; do \
			cp -a $${mdir} release/android/apps/wpa_supp/ ; \
		done ; \
	fi
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
#disable#End of release_$(oem)_bins
	@$(MARKEND)


#- Android specific extra release files needed
release_android_extra_bins:
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,release/android)
	install -p $(fc9g-dir)/epi_ttcp \
		release/android/apps; \
	shopt -s nullglob; \
	shopt -s extglob; \
#disabled#	install -p src/wps/android/wpsenrarm_android
#disabled#		release/android/apps
#disabled#End of release_android_extra_bins
	$(call REMOVE_WARN_PARTIAL_BUILD,release/android)
	@$(MARKEND)

## ------------------------------------------------------------------
## Create oem brand specific source releases
## ------------------------------------------------------------------

# This is a convinience target for open source pacakge testing, not called by anyone directly
release_open_src: $(foreach oem,$(OEM_LIST_OPEN_SRC),release_$(oem)_open_src_package)

release_src: $(foreach oem,$(OEM_LIST),release_$(oem)_src_package) \
             $(foreach oem,$(OEM_LIST_OPEN_SRC),release_$(oem)_open_src_package)

$(foreach oem,$(OEM_LIST),release_$(oem)_src_package): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),release_$(oem)_src_package): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),release_$(oem)_src_package):
	@$(MARKSTART)
	mkdir -p $(RELDIR)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	@echo "Generating release source + binaries package now in $(RELDIR) ...."
	# First create release/<oem>/src from SRCFILTER_<oem> src filter
	# For Android, use src/bcmdhd/google_filelist.txt, create semi flat "src" release folder
	# Copy Makefile and Kconfig from ROOT/src, cp .c/.h files from ROOT/build/src
	# For Samsung, using src/bcmdhd/samsung_filelist.txt, create semi flat "src" release folder
	# Copy Makefile and Kconfig from ROOT/src, cp .c/.h files from ROOT/build/src
	if [ "$(oem)" == "android" ]; then \
	        mkdir -pv $(RELDIR)/src/include/proto; \
	        cp src/bcmdhd/Makefile_jb $(RELDIR)/src/Makefile; \
	        cp src/bcmdhd/Kconfig $(RELDIR)/src/Kconfig; \
		cp build/$(oem)/src/include/Makefile $(RELDIR)/src/include/Makefile; \
		for i in `egrep '^components/shared/proto/.*\.h$$' src/tools/release/google_filelist.txt`; do \
		   cp build/$(oem)/$$i $(RELDIR)/src/include/proto; \
		done; \
		for i in `egrep '^src/include/.*\.h$$' src/tools/release/google_filelist.txt | grep -v '/proto/'`; do \
		   cp build/$(oem)/$$i $(RELDIR)/src/include; \
		done; \
		for i in `egrep '^src/shared/bcmwifi/include/.*\.h$$' src/tools/release/google_filelist.txt`; do \
		   cp build/$(oem)/$$i $(RELDIR)/src/include; \
		done; \
		for i in `egrep  '\.[ch]$$' src/tools/release/google_filelist.txt | grep -v 'src/include/' | grep -v 'src/shared/bcmwifi/include/' `; do \
		   cp build/$(oem)/$$i $(RELDIR)/src; \
		done; \
		cd $(RELDIR) && $(FIND) src components | xargs -t -L1 $(MOGRIFY_SCRIPT)  -translate_open_to_dual_copyright ; \
	elif [ "$(oem)" == "samsung" ]; then \
	        mkdir -pv $(RELDIR)/src/include/proto; \
	        cp src/bcmdhd/Makefile_hw4_jb $(RELDIR)/src/Makefile; \
	        cp src/bcmdhd/Kconfig_hw4_jb $(RELDIR)/src/Kconfig; \
		cp build/$(oem)/src/include/Makefile $(RELDIR)/src/include/Makefile; \
		for i in `egrep '^components/shared/proto/.*\.h$$' src/tools/release/samsung_filelist.txt`; do \
		   cp build/$(oem)/$$i $(RELDIR)/src/include/proto; \
		done; \
		for i in `egrep '^src/include/.*\.h$$' src/tools/release/samsung_filelist.txt | grep -v '/proto/'`; do \
		   cp build/$(oem)/$$i $(RELDIR)/src/include; \
		done; \
		for i in `egrep  '\.[ch]$$' src/tools/release/samsung_filelist.txt | grep -v 'src/include/'`; do \
		   cp build/$(oem)/$$i $(RELDIR)/src; \
		done; \
		cd $(RELDIR) && $(FIND) src components | xargs -t -L1 $(MOGRIFY_SCRIPT)  -translate_open_to_dual_copyright ; \
	else \
		tar cpf - $(TAR_SKIPCMD) -C build/$(oem) \
	                 `$(FIND) src components $(FIND_SKIPCMD) -print | $(SRCFILTER) -v $(SRCFILELIST)_$(oem)` \
	                 `cd build/$(oem) && $(FIND) $(DNGL_IMGDIR)/* -type f \
				-name "*$(DNGL_IMG_PFX).h" -o -name "*$(DNGL_IMG_PFX).bin"` | \
		         tar xpf - $(TAR_SKIPCMD) -C $(RELDIR); \
	fi
	tar cpf $(RELDIR)/dongle-host-driver-source.tar -C $(RELDIR) $(TAR_SKIPCMD) \
	         src firmware host apps README.txt BCMLogo.gif
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
#disable#End of release_$(oem)_src_package


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
ifeq ($(BRAND),android-external-dongle)
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
		-DESTA_POSTMOGRIFY_REMOVAL -DLINUX_POSTMOGRIFY_REMOVAL -strip_comments -translate_open_to_dual_copyright
	# Copy android specific config files
	@if [ "$(oem)" == "android" ]; then \
		install -dv $(RELDIR)/open-src/config; \
		install -pv src/dhd/android/Android.mk $(RELDIR)/open-src; \
	fi

	# Validate Bcm and android open-src source package again after cleanup
	@if [ "$(oem)" == "bcm" ]; then \
		$(call VALIDATE_OPEN_SRC_PACKAGE_AFTER_CLEANUP, $(fc9-dhd)); \
	elif [ "$(oem)" == "android" ]; then \
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
		host apps README.txt BCMLogo.gif
	gzip -9 $(RELDIR)/dongle-host-driver-source-open.tar
	# Remove non open-src packages for android
	@if [ "$(oem)" == "android" ]; then \
	    rm -fv $(RELDIR)/dongle-host-driver-source.tar.gz; \
	    rm -rf $(RELDIR)/src; \
	fi
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)
endif # BRAND
#disable#End of release_$(oem)_open_src_package

release: release_bins release_src

build_end:
	@rm -fv $(HNDRTE_FLAG)
	@$(MARKEND_BRAND)

is32on64: # for build_config consumption
	@echo "$(BRAND) can be build on a 64 bit linux cluster"

.PHONY: FORCE checkout build_include build_end prebuild_prep prepare_kernel build_dhd build_apps $(foreach oem,$(OEM_LIST),build_$(oem)_dhd) $(foreach oem,$(OEM_LIST),build_$(oem)_apps) release release_bins release_src $(foreach oem,$(OEM_LIST),release_$(oem)_src_package) $(foreach oem,$(OEM_LIST_OPEN_SRC),release_$(oem)_open_src_package) $(foreach oem,$(OEM_LIST),release_$(oem)_bins release_$(oem)_extra_bins) is32on64 integrate_firmware
