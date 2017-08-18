#
# This makefile is used for the following
#    - Common definitions used for -stbsoc- brands
#    - Includes the parent makefile 'linux.mk' for the -stbsoc- brands
# This makefile is included from the following brand makefiles
#    - linux-internal-stbsoc.mk
#    - linux-external-stbsoc.mk
#    - linux-mfgtest-stbsoc.mk
#
# $Id: $
#

#PARENT_MAKEFILE := linux.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

## -----------------------------------------------------------------------
## The filelists and component level central filelists for -stbsoc- brands
## -----------------------------------------------------------------------
SRCFILELIST  := \
	src/tools/release/linux-dhd-stbsoc-filelist.txt \
	src/tools/release/linux-stbsoc-filelist.txt \
	src/tools/release/linux-wlexe-filelist.txt \
	src/tools/release/linux-usbbcmdl-filelist.txt
SRCFILELIST_GPL  := \
	src/tools/release/linux-dhd-gpl-filelist.txt
SRCFILELIST_HSL  := \
	src/tools/release/linux-p2plib-filelist.txt
SRCFILELISTS_COMPONENTS := \
	src/tools/release/components/wlphy-filelist.txt \
	src/tools/release/components/bcmcrypto-filelist.txt
SRCFILELISTS_HSL_COMPONENTS  := \
	src/tools/release/components/wps-filelist.txt \
	src/tools/release/components/secfrw-filelist.txt \
	src/tools/release/components/pre_secfrw-filelist.txt \
	src/tools/release/components/p2p-filelist.txt \
	src/tools/release/components/dhcpd-filelist.txt \
	src/tools/release/components/supp-filelist.txt \
	src/tools/release/components/bwl-filelist.txt \
	src/tools/release/components/bcmcrypto-filelist.txt \
	src/tools/release/components/wfi-filelist.txt

# Final Filelist for Parent makefile
BRAND_SRCFILELIST  := $(SRCFILELIST) $(SRCFILELIST_GPL) $(SRCFILELIST_HSL) \
	$(SRCFILELISTS_COMPONENTS) $(SRCFILELISTS_HSL_COMPONENTS)

## -----------------------------------------------------------------------
## The filelist DEFS and UNDEFS for -stbsoc- brands
## -----------------------------------------------------------------------
COMMON_SRCFILELIST_DEFS := \
	WLPHY BCMCRYPTO BCMWPS LINUX BCMWFI P2P BWL WFI DHCPD_APPS BCMDBUS WLLX \
	ESTA_OPENSRC_CLEANUP_LIST

COMMON_SRCFILELIST_UNDEFS := \
	BCMSDIO POCKET_PC WLNINTENDO WLNINTENDO2 WME_PER_AC_TUNING WLNOKIA \
	WLPLT WLBTAMP BCMWAPI_WPI ROUTER LINUX_CRYPTO \
	vxworks __ECOS DOS PCBIOS __IOPOS__ NDIS _CFE_ _HNDRTE_ _MINOSL_ MACOSX __NetBSD__ EFI \
	LINUX_ROUTER ECOS_ROUTER WINCE PRE_SECFRW DHCPD_APPS \
	SUPP WLBTAMP OEM_CHROMIUMOS NO_WLBTAMP NO_WLEXTLOG
# COMMON_SRCFILELIST_UNDEFS += OEM_ANDROID

# stbsoc Specific DEFS and UNDEFS for filelist (from Brand Makefile)
STBSOC_BRAND_SRCFILELIST_DEFS   ?=
STBSOC_BRAND_SRCFILELIST_UNDEFS ?=

# Final Filelist DEFS & UNDEFS for Parent makefile
BRAND_SRCFILELIST_DEFS   := $(COMMON_SRCFILELIST_DEFS) $(STBSOC_BRAND_SRCFILELIST_DEFS)
BRAND_SRCFILELIST_UNDEFS := $(COMMON_SRCFILELIST_UNDEFS) $(STBSOC_BRAND_SRCFILELIST_UNDEFS)

## -----------------------------------------------------------------------
## The Mogrification DEFS and UNDEFS for -stbsoc- brands
## -----------------------------------------------------------------------
COMMON_MOGRIFY_DEFS := BCM47XX_CHOPS

COMMON_MOGRIFY_UNDEFS   := \
        AWDL BCMINTERNAL CCX_SDK INTERNAL MACOSX NOT_FOR_RELEASE OLYMPIC_RWL PALM RIM \
        WAR_4324_PR116249 WAR_HWJIRA_CRWLPCIEGEN2_160 WAR1_HWJIRA_CRWLPCIEGEN2_162 WAR2_HWJIRA_CRWLPCIEGEN2_162 \
        WAR4360_UCODE WLMOTOROLALJ WLNIN WLNINTENDO WLNINTENDO_ENABLED WLNINTENDO2 WLNINTENDO2DBG \
        WLNOKIA WLNOKIA_ENABLED WLNOKIA_NVMEM WLNOKIA_WAPI DHD_AWDL WLAWDL BCMSDIO POCKET_PC \
        vxworks _vxworks  __ECOS DOS PCBIOS __IOPOS__ NDIS _CFE_ _HNDRTE_ _MINOSL_ MACOSX EFI \
        ECOS_ROUTER WINCE OEM_CHROMIUMOS _WIN32 _WIN64 WIN7 MACOSX _MACOSX_ __ECOS __NetBSD__ \
        TARGETOS_nucleus DOS PCBIOS NDIS _CFE_ _MINOSL_ UNDER_CE CONFIG_PCMCIA_MODULE BCMINTERNAL _HNDRTE_ \
        _MSC_VER ILSIM BCMPERFSTATS BCMTSTAMPEDLOGS DSLCPE_DELAY \
        DONGLEBUILD BCMROMOFFLOAD BCMROMBUILD  OLYMPIC_RWL \
        __BOB__  CONFIG_XIP NDIS60 NDISVER  DHD_SPROM NOTYET BINOSL USER_MODE BCMHND74K \
        BCMSPI USBAP BCM_BOOTLOADER  WLNOKIA BCMSDIO SDTEST USE_ETHER_BPF_MTAP \
        _RTE_ WL_LOCATOR ATE_BUILD PATCH_SDPCMDEV_STRUCT SERDOWNLOAD \
        CUSTOMER_HW4 WLMEDIA_CUSTOMER_1 UNRELEASEDCHIP BCM33XX_CHOPS BCM42XX_CHOPS BCM4413_CHOPS \
        BCM4710A0 BCM93310 BCM93350 BCM93352 BCM93725B BCM94100 \
        BCMSDIO BCMSDIODEV BCMSIMUNIFIED BCMSLTGT BCM_USB BCMUSBDEV COMS \
        CONFIG_BCM4710 CONFIG_BCM93725 CONFIG_BCM93725_VJ CONFIG_BCRM_93725 CONFIG_BRCM_VJ CONFIG_MIPS_BRCM \
        DSLCPE ETSIM INTEL __IOPOS__ __klsi__ LINUXSIM NETGEAR \
        NICMODE PSOS ROUTER VX_BSD4_3 WLBTAMP BCM_GMAC3

# stbsoc Specific Mogrification DEFS and UNDEFS (from Brand Makefile)
STBSOC_BRAND_MOGRIFY_DEFS      ?=
STBSOC_BRAND_MOGRIFY_UNDEFS    ?=

# Final Mogrification DEFS & UNDEFS for Parent makefile
BRAND_MOGRIFY_DEFS     := $(COMMON_MOGRIFY_DEFS) $(STBSOC_BRAND_MOGRIFY_DEFS)
BRAND_MOGRIFY_UNDEFS   := $(COMMON_MOGRIFY_UNDEFS) $(STBSOC_BRAND_MOGRIFY_UNDEFS)

## -----------------------------------------------------------------------
## The Driver Types and Platform/OS specific definitions for each <oem>
## -----------------------------------------------------------------------
# OEM list
OEM_LIST   ?= bcm

# The Operating System of the Host Driver Builds

HOST_DRV_OS_bcm       ?= stb7271armlx4117
HOST_DRV_OS_android   ?=

# The Driver Types to build for each <oem>
#    dhd --> Full Dongle Host driver
#    wl -->  BMAC / NIC Driver
HOST_DRV_TYPE_bcm     ?= wl
HOST_DRV_TYPE_android ?=

# The Operating System of the Apps Builds
APP_DRV_OS_bcm        ?= stb7271armlx41110 stb7271armlx4111064bit
APP_DRV_OS_android    ?=

# The Operating System of the HSL Builds
APP_HSL_OS_bcm        ?= stb7271armlx41110
APP_HSL_OS_android    ?=

## -----------------------------------------------------------------------
## Check Validity of Brands
## -----------------------------------------------------------------------
VALID_BRANDS := \
	linux-external-stbsoc \
	linux-internal-stbsoc \
	linux-mfgtest-stbsoc

ifeq ($(filter $(VALID_BRANDS),$(BRAND)),)
	$(error ERROR This $(BRAND) build is not a standalone build brand. Valid brands are $(VALID_BRANDS))
endif

#include $(PARENT_MAKEFILE)

# Originally Copy of linux.mk.We want customize what is packaged. Right now is used to build the following
# TBD: review this and take out areas we dont' need. ie DHD related.
#    - Full dongle (DHD) Host driver for USB/PCIE devices
#    - BMAC Host driver for USB devices
#    - NIC driver for PCIE devices
#    - Dongle Firmware for DHD/BMAC Drivers for USB/PCIE devices
#    - Apps and HSL for DHD/BMAC/NIC drivers (for USB/PCIE devices)
#    - Software Packages for DHD/BMAC drivers
#
# Modifications:
#    Remove -d from src filter. Which allows src/generated to be recursively be included in package
#
#
#

NULL         := /dev/null
FIND         := /usr/bin/find
BUILD_BASE   := $(shell pwd)
SRCDIR       := $(BUILD_BASE)/src
RELEASEDIR   := $(BUILD_BASE)/release
# MOGRIFY      = perl $(SRCDIR)/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS))
SRCFILTER    := srcfilter.py
DOXYGEN      := /tools/oss/bin/doxygen
UNAME        := $(shell uname -a)
WARN_FILE    := _WARNING_PARTIAL_BUILD_DO_NOT_USE
WPA_SUPP_TAG ?= WPA_SUPPLICANT-0-8_00_56


# SVN bom specification KLO: new sparse file here
HNDSVN_BOM   := dhd.sparse

empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

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

ifdef TAG
  vinfo := $(subst _,$(space),$(TAG))
else
  vinfo := $(shell date '+D11 REL %Y %m %d')
endif

# Place a warning file to indicate build failed
define WARN_PARTIAL_BUILD
	touch $1/$(WARN_FILE)
endef

# Remove warning file to indicate build step completed successfully
define REMOVE_WARN_PARTIAL_BUILD
	rm -f $1/$(WARN_FILE)
endef

ifneq ($(findstring x86_64,$(UNAME)),)
	32BIT = -32 ARCH=i386
	32GCC64 = CC='gcc -m32'
	64GCC64 = CC='gcc -m64'
	# following one is only for WPS
	32GCC64PATH = PATH=/tools/oss/packages/i686-rhel4/gcc/default/bin:$(PATH)
	# following is for /tools/linux/src/linux-2.6.9-intc1/Makefile
	export 32ON64=1
	32LIB64 = CROSS_LD_PATH=-L/projects/hnd/tools/linux/lib
	64LIB64 = CROSS_LD_PATH=-L/projects/hnd/tools/linux/lib_x86_64
endif # UNAME

## -------------------------------------------------------------------------
## Fedora 15, i686, 32bit
fc15-host-drv-opts   := LINUXVER=2.6.38.6-26.rc1.fc15.i686.PAE \
			LINUXDIR=/tools/linux/src/linux-2.6.38.6-26.rc1.fc15.i686.PAE \
			CROSS_COMPILE=/tools/bin/ GCCVER=4.6.0$(32BIT) \
			TARGET_OS=fc15 TARGET_ARCH=x86
fc15-app-opts        := NATIVEBLD=1 TARGETARCH=x86 $(32GCC64) $(32LIB64)
fc15-hsl-opts        := $(fc15-app-opts)
## -------------------------------------------------------------------------
## Fedora 19, i686, 32bit
fc19-host-drv-opts   := LINUXVER=3.11.1-200.fc19.i686.PAE \
			LINUXDIR=/tools/linux/src/linux-3.11.1-200.fc19.i686.PAE \
			CROSS_COMPILE=/tools/bin/ GCCVER=4.8.1$(32BIT) \
			TARGET_OS=fc19 TARGET_ARCH=x86
fc19-app-opts        := NATIVEBLD=1 TARGETARCH=x86 $(32GCC64) $(32LIB64)
fc19-hsl-opts        := $(fc19-app-opts)
## -------------------------------------------------------------------------
## Fedora 19, x86, 64bit
fc19x64-host-drv-opts := LINUXVER=3.11.1-200.fc19.x86_64 \
			LINUXDIR=/tools/linux/src/linux-3.11.1-200.fc19.x86_64 \
			CROSS_COMPILE=/tools/bin/ GCCVER=4.8.1 \
			TARGET_OS=fc19 TARGET_ARCH=x86_64
fc19x64-app-opts      := NATIVEBLD=1 TARGETARCH=x86_64 $(64GCC64) $(64LIB64)
fc19x64-hsl-opts      := $(fc19x64-app-opts)
## -------------------------------------------------------------------------
## Fedora 22, x86, 64bit
fc22x64-host-drv-opts := LINUXVER=4.0.4-301.fc22.x86_64 \
			LINUXDIR=/tools/linux/src/linux-4.0.4-301.fc22.x86_64 \
			CROSS_COMPILE=/tools/linux/local/cross/rhel6-64/fc22/bin/x86_64-fc22.4.0.4.301-linux- GCCVER=5.1.1 \
			TARGET_OS=fc22 TARGET_ARCH=x86_64
fc22x64-app-opts      := NATIVEBLD=1 TARGETARCH=x86_64 $(64GCC64) $(64LIB64)
fc22x64-hsl-opts      := $(fc22x64-app-opts)
## -------------------------------------------------------------------------
## BRIX-android 4.2.2 x86 64bit
brix-and-422-host-drv-opts := LINUXVER=3.10.11-aia1+-androidx86 \
			LINUXDIR=/tools/linux/src/linux-3.10.11-aia1+-androidx86 \
			PATH="/projects/hnd/tools/linux/hndtools-x86_64-linux-glibc2.7-4.6/bin:$(PATH)" \
			CROSS_COMPILE=x86_64-linux-
## -------------------------------------------------------------------------
## STBLinux-7425 (mips-little-endian)
stb7425mipslx33-host-drv-opts := LINUXVER=3.3.8-3.3 \
			LINUXDIR=/projects/hnd/tools/Linux/BCG/stblinux-3.3-3.3/linux \
			CROSS_COMPILE=/projects/hnd/tools/Linux/BCG/stbgcc-4.5.4-2.8/bin/mipsel-linux-
stb7425mipslx33-app-opts := TARGETARCH=mips TARGETENV=linuxmips TARGETOS=unix \
			PATH="/projects/hnd/tools/Linux/BCG/stbgcc-4.5.4-2.8/bin:$(PATH)" \
			CROSS_COMPILE=/projects/hnd/tools/Linux/BCG/stbgcc-4.5.4-2.8/bin/mipsel-linux- \
			CC=mipsel-linux-gcc STBLINUX=1 STB=1 LIBUSB_PATH=/projects/hnd/tools/linux/BCG/rootfs-3.3-3.3/uclinux-rootfs/lib/libusb \
			AR=mipsel-linux-ar
stb7425mipslx33-hsl-opts := $(stb7425mipslx33-app-opts)
## -------------------------------------------------------------------------
## STBLinux-7445c (arm-little-endian)
stb7445armlx31412-host-drv-opts := LINUXVER=3.14.13-1.2pre \
			LINUXDIR=/projects/hnd/tools/linux/BCG/stblinux-3.14-1.2/linux \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.0/bin/arm-linux-
stb7445armlx31412-app-opts := TARGETARCH=armv7l TARGETENV=linuxarm TARGETOS=unix \
			PATH="/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.0/bin:$(PATH)" \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.0/bin/arm-linux- \
			CC=arm-linux-gcc STBLINUX=1 STB=1 LIBUSB_PATH=/projects/hnd/tools/linux/lib/arm31412 \
			AR=arm-linux-ar
stb7445armlx31412-hsl-opts   := $(stb7445armlx31412-app-opts)
## -------------------------------------------------------------------------
## STBLinux-7271 (arm-little-endian)
stb7271armlx4110-host-drv-opts := LINUXVER=4.1.19-1.0 \
                        LINUXDIR=/projects/hnd/tools/linux/BCG/stblinux-4.1-1.0/linux \
                        CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.5/bin/arm-linux-
stb7271armlx4110-app-opts := TARGETARCH=armv7l TARGETENV=linuxarm TARGETOS=unix \
                        PATH="/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.5/bin:$(PATH)" \
                        CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.5/bin/arm-linux- \
                        CC=arm-linux-gcc STBLINUX=1 STB=1 LIBUSB_PATH=/projects/hnd/tools/linux/lib/arm4110 \
                        AR=arm-linux-ar
stb7271armlx4110-hsl-opts   := $(stb7271armlx4110-app-opts)

## -------------------------------------------------------------------------
## STBLinux-7271 4.1-1.10(arm-little-endian)
stb7271armlx41110-host-drv-opts := LINUXVER=4.1.20-1.10 \
                        LINUXDIR=/opt/brcm/linux-4.1-1.10/arm \
                        CROSS_COMPILE=/opt/toolchains/stbgcc-4.8-1.6/bin/arm-linux-
stb7271armlx41110-app-opts := TARGETARCH=armv7l TARGETENV=linuxarm TARGETOS=unix \
                        PATH="/opt/toolchains/stbgcc-4.8-1.6/bin:$(PATH)" \
                        CROSS_COMPILE=/opt/toolchains/stbgcc-4.8-1.6/bin/arm-linux- \
                        CC=arm-linux-gcc STBLINUX=1 STB=1 \
                        AR=arm-linux-ar
stb7271armlx41110-hsl-opts   := $(stb7271armlx41110-app-opts)
## -------------------------------------------------------------------------
## STBLinux-7271 4.1-1.10 (arm-little-endian 64 bit)
stb7271armlx4111064bit-host-drv-opts := LINUXVER=4.1.20-1.10 \
                        LINUXDIR=/opt/brcm/linux-4.1-1.10/arm64 \
                        CROSS_COMPILE=/opt/toolchains/stbgcc-4.8-1.6/bin/aarch64-linux-gnu-
stb7271armlx4111064bit-app-opts := TARGETARCH=aarch64 TARGETENV=linuxarm TARGETOS=unix \
                        PATH="/opt/toolchains/stbgcc-4.8-1.6/bin:$(PATH)" \
                        CROSS_COMPILE=/opt/toolchains/stbgcc-4.8-1.6/bin/aarch64-linux-gnu- \
                        CC=aarch64-linux-gnu-gcc STBLINUX=1 STB=1 \
                        AR=aarch64-linux-gnu-ar
stb7271armlx4111064bit-hsl-opts   := $(stb7271armlx4111064bit-app-opts)
## -------------------------------------------------------------------------
## STBLinux-7271 4.1-1.8 (arm-little-endian)
stb7271armlx4118-host-drv-opts := LINUXVER=4.1.20-1.8 \
                        LINUXDIR=/opt/brcm/linux-4.1-1.8/arm \
                        CROSS_COMPILE=/opt/toolchains/stbgcc-4.8-1.5/bin/arm-linux-
stb7271armlx4118-app-opts := TARGETARCH=armv7l TARGETENV=linuxarm TARGETOS=unix \
                        PATH="/opt/toolchains/stbgcc-4.8-1.5/bin:$(PATH)" \
                        CROSS_COMPILE=/opt/toolchains/stbgcc-4.8-1.5/bin/arm-linux- \
                        CC=arm-linux-gcc STBLINUX=1 STB=1 \
                        AR=arm-linux-ar
stb7271armlx4118-hsl-opts   := $(stb7271armlx4118-app-opts)
## -------------------------------------------------------------------------
## STBLinux-7271 4.1-1.8 (arm-little-endian 64 bit)
stb7271armlx411864bit-host-drv-opts := LINUXVER=4.1.20-1.8 \
                        LINUXDIR=/opt/brcm/linux-4.1-1.8/arm64 \
                        CROSS_COMPILE=/opt/toolchains/stbgcc-4.8-1.5/bin/aarch64-linux-gnu-
stb7271armlx411864bit-app-opts := TARGETARCH=aarch64 TARGETENV=linuxarm TARGETOS=unix \
                        PATH="/opt/toolchains/stbgcc-4.8-1.5/bin:$(PATH)" \
                        CROSS_COMPILE=/opt/toolchains/stbgcc-4.8-1.5/bin/aarch64-linux-gnu- \
                        CC=aarch64-linux-gnu-gcc STBLINUX=1 STB=1 \
                        AR=aarch64-linux-gnu-ar
stb7271armlx411864bit-hsl-opts   := $(stb7271armlx411864bit-app-opts)
## -------------------------------------------------------------------------
## STBLinux-7445 (3.14.28-1.9pre)(arm-little-endian)
stb7445armlx31419-host-drv-opts := LINUXVER=3.14.28-1.9pre \
			LINUXDIR=/projects/hnd/tools/linux/BCG/stblinux-3.14.28-1.9pre/linux \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.3/bin/arm-linux-
stb7445armlx31419-app-opts := TARGETARCH=armv7l TARGETENV=linuxarm TARGETOS=unix \
			PATH="/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.3/bin:$(PATH)" \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.3/bin/arm-linux- \
			CC=arm-linux-gcc STBLINUX=1 STB=1 LIBUSB_PATH=/projects/hnd/tools/linux/lib/arm31416 \
			AR=arm-linux-ar
stb7445armlx31419-hsl-opts   := $(stb7445armlx31419-app-opts)
## -------------------------------------------------------------------------
## STBLinux-7439 (arm-little-endian)
stb7439armlx31413-host-drv-opts := LINUXVER=3.14.28-1.3pre \
			LINUXDIR=/projects/hnd/tools/linux/BCG/stblinux-3.14.28-1.3pre/linux \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.0/bin/arm-linux-
stb7439armlx31413-app-opts := TARGETARCH=armv7l TARGETENV=linuxarm TARGETOS=unix \
			PATH="/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.0/bin:$(PATH)" \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.0/bin/arm-linux- \
			CC=arm-linux-gcc STBLINUX=1 STB=1 LIBUSB_PATH=/projects/hnd/tools/linux/lib/arm31413 \
			AR=arm-linux-ar
stb7439armlx31413-hsl-opts   := $(stb7439armlx31413-app-opts)
## -------------------------------------------------------------------------
## STBLinux-7439 (3.14.28-1.5pre)(arm-little-endian)
stb7439armlx31415-host-drv-opts := LINUXVER=3.14.28-1.5pre \
			LINUXDIR=/projects/hnd/tools/linux/BCG/stblinux-3.14.28-1.5pre/linux \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.1/bin/arm-linux-
stb7439armlx31415-app-opts := TARGETARCH=armv7l TARGETENV=linuxarm TARGETOS=unix \
			PATH="/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.1/bin:$(PATH)" \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.1/bin/arm-linux- \
			CC=arm-linux-gcc STBLINUX=1 STB=1 LIBUSB_PATH=/projects/hnd/tools/linux/lib/arm31415 \
			AR=arm-linux-ar
stb7439armlx31415-hsl-opts   := $(stb7439armlx31415-app-opts)
## -------------------------------------------------------------------------
## STBLinux-97252 (3.14.28-1.4)(arm-little-endian)
stb97252armlx31414-host-drv-opts := LINUXVER=3.14.28-1.4 \
			LINUXDIR=/projects/hnd/tools/linux/BCG/stblinux-3.14.28-1.4-freeze/linux \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.1/bin/arm-linux-
stb97252armlx31414-app-opts := TARGETARCH=armv7l TARGETENV=linuxarm TARGETOS=unix \
			PATH="/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.1/bin:$(PATH)" \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.1/bin/arm-linux- \
			CC=arm-linux-gcc STBLINUX=1 STB=1 LIBUSB_PATH=/projects/hnd/tools/linux/lib/arm31414 \
			AR=arm-linux-ar
stb97252armlx31414-hsl-opts   := $(stb7439armlx31414-app-opts)

## -------------------------------------------------------------------------
## STBLinux-7439 (3.14.28-1.6pre)(arm-little-endian)
stb7439armlx31416-host-drv-opts := LINUXVER=3.14.28-1.6pre \
			LINUXDIR=/projects/hnd/tools/linux/BCG/stblinux-3.14.28-1.6pre/linux \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.2/bin/arm-linux-
stb7439armlx31416-app-opts := TARGETARCH=armv7l TARGETENV=linuxarm TARGETOS=unix \
			PATH="/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.2/bin:$(PATH)" \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.2/bin/arm-linux- \
			CC=arm-linux-gcc STBLINUX=1 STB=1 LIBUSB_PATH=/projects/hnd/tools/linux/lib/arm31416 \
			AR=arm-linux-ar
stb7439armlx31416-hsl-opts   := $(stb7439armlx31416-app-opts)

## -------------------------------------------------------------------------
## STBLinux-7252 (3.14.28-1.11pre)(arm-little-endian)
stb7252armlx314111-host-drv-opts := LINUXVER=3.14.28-1.11pre \
			LINUXDIR=/projects/hnd/tools/linux/BCG/stblinux-3.14.28-1.11pre/linux \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.4/bin/arm-linux-
stb7252armlx314111-app-opts := TARGETARCH=armv7l TARGETENV=linuxarm TARGETOS=unix \
			PATH="/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.4/bin:$(PATH)" \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.4/bin/arm-linux- \
			CC=arm-linux-gcc STBLINUX=1 STB=1 LIBUSB_PATH=/projects/hnd/tools/linux/lib/arm314111 \
			AR=arm-linux-ar
stb7252armlx314111-hsl-opts   := $(stb7252armlx314111-app-opts)
## -------------------------------------------------------------------------
## STBLinux-7252 (4.1-1.0)(arm-little-endian)
stb7252armlx4110-host-drv-opts := LINUXVER=4.1.19-1.0 \
			LINUXDIR=/projects/hnd/tools/linux/BCG/stblinux-4.1-1.0/linux \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.5/bin/arm-linux-
stb7252armlx4110-app-opts := TARGETARCH=armv7l TARGETENV=linuxarm TARGETOS=unix \
			PATH="/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.5/bin:$(PATH)" \
			CROSS_COMPILE=/projects/hnd/tools/linux/BCG/stbgcc-4.8-1.5/bin/arm-linux- \
			CC=arm-linux-gcc STBLINUX=1 STB=1 LIBUSB_PATH=/projects/hnd/tools/linux/lib/arm4110 \
			AR=arm-linux-ar
stb7252armlx4110-hsl-opts   := $(stb7252armlx4110-app-opts)
## -------------------------------------------------------------------------
## For Filelist manipulationcd
## --------------------------------------------------------------------------------
# Brand Specific file list, DEFS, UNDEFS (from Brand Makefile)
BRAND_SRCFILELIST         ?=
BRAND_SRCFILELIST_DEFS    ?=
BRAND_SRCFILELIST_UNDEFS  ?=
SRCFILELIST_FINAL         := src_file_list

# To mogrify source file lists
include src-features-master-list.mk
SRC_FEATURES_FINAL := $(sort $(filter-out $(BRAND_SRCFILELIST_UNDEFS),$(SRC_FEATURES_MASTER_LIST)) $(BRAND_SRCFILELIST_DEFS))
FILES_DEFS         := $(patsubst %,-D%_SRC,$(SRC_FEATURES_FINAL))

## --------------------------------------------------------------------------------
## For Mogrification
## --------------------------------------------------------------------------------
export BRAND_RULES       = brand_common_rules.mk
export MOGRIFY_RULES     = mogrify_common_rules.mk

# Mogrifier step pre-requisites
# MOGRIFY_FLAGS gets DEFS and UNDEFS and any mogrify.pl cmd line options
export MOGRIFY_FLAGS     =
# Addition file types to mogrify in addition to what is in $(MOGRIFY_RULES)
export MOGRIFY_FILETYPES =
# Addition folders to skip in addition to what is in $(MOGRIFY_RULES)
export MOGRIFY_EXCLUDE   =

# Brand Specific Mogrification DEFS and UNDEFS (from Brand Makefile)
BRAND_MOGRIFY_DEFS      ?=
BRAND_MOGRIFY_UNDEFS    ?=

DEFS := $(BRAND_MOGRIFY_DEFS)
UNDEFS := $(BRAND_MOGRIFY_UNDEFS)

MOGRIFY_FLAGS_COMMON    := $(UNDEFS:%=-U%) $(DEFS:%=-D%)

# Unreleased-chiplist is included only for non-internal builds
ifeq ($(findstring BCMINTERNAL,$(DEFS))$(findstring internal,$(BRAND)),)
include unreleased-chiplist.mk
endif # BCMINTERNAL

# Combined xmog in and out flags for specific oem needs
DEFS_bcm      += -DOEM_BCM -UWLNOKIA
DEFS_nokia    += -UOEM_BCM -DWLNOKIA -UOEM_ANDROID
DEFS_android  += -UOEM_BCM -UWLNOKIA -DOEM_ANDROID \
	-UBCMSPI -USDHOST3 -UBCMNVRAM -URWL_DONGLE -UUART_REFLECTOR -UDHDARCH_MIPS

# These are extensions of source files that should be mogrified
# (using the parameters specified in DEFS and UNDEFS above.)
#
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

include $(BRAND_RULES)


## --------------------------------------------------------------------------------
## Generic definitions for Drivers, Apps & HSL
## --------------------------------------------------------------------------------
ifneq ($(findstring internal,$(BRAND)),)
    BLD_TYPE ?= debug
else
    BLD_TYPE ?= release
endif
ifneq ($(findstring mfgtest,$(BRAND)),)
    BCM_MFGTEST ?= 1
endif

# Default <oem> list
OEM_LIST     ?= bcm

## --------------------------------------------------------------------------------
## For Release Host Drivers and Firmwares
## --------------------------------------------------------------------------------
# The Operating System of the Host Driver Builds (default)
HOST_DRV_OS_bcm         ?=
HOST_DRV_OS_android     ?=

# The Driver Type of the Host driver (default). eg: dhd / bmac
HOST_DRV_TYPE_bcm       ?=
HOST_DRV_TYPE_android   ?=

# The Host driver make targets (for each oem)
# HOST_DRIVERS_bcm     := $(foreach host_os,$(HOST_DRV_OS_bcm:%=%-host-drv-bcm),$(host_os))
# HOST_DRIVERS_android := $(foreach host_os,$(HOST_DRV_OS_android:%=%-host-drv-android),$(host_os))

# Host Drivers to build & Release for each <oem> (default none)
# RLS_HOST_DRIVERS_dhd_bcm      ?=
# RLS_HOST_DRIVERS_dhd_android  ?=
# RLS_HOST_DRIVERS_wl_bcm       ?=
# RLS_HOST_DRIVERS_wl_android   ?=

# List FW mages to Release and Embedded into the Host driver for each <oem> (if BCMEMBEDIMAGE is ON)
RLS_FIRMWARES_bcm    ?=
EMBED_FIRMWARES_bcm  ?=

# Package up embeddable images as well for each oem
RLS_FIRMWARES_bcm    += $(EMBED_FIRMWARES_bcm)

# ALL_DNGL_IMAGES      += $(foreach oem,$(OEM_LIST),$(RLS_FIRMWARES_$(oem)))

# Dongle related variables and their references come from global makefile
# linux-dongle-image-launch.mk
HNDRTE_DEPS         := checkout
HNDRTE_IMGFN        := rtecdc.h

# include linux-dongle-image-launch.mk
# DNGL_IMG_FILES   += rtecdc.bin.trx


## --------------------------------------------------------------------------------
## For Release Apps and HSL
## --------------------------------------------------------------------------------
# The Operating System of the Apps Builds (default)
APP_DRV_OS_bcm       ?=
APP_DRV_OS_android   ?=

# App make targets for each oem (for each oem)
APP_DRIVERS_bcm      := $(APP_DRV_OS_bcm:%=%-app-bcm)
APP_DRIVERS_android  := $(APP_DRV_OS_bcm:%=%-app-android)

# The Operating System of the HSL Builds (default)
APP_HSL_OS_bcm       ?=
APP_HSL_OS_android   ?=

# HSL make targets for each oem (for each oem)
APP_HSL_bcm          := $(APP_HSL_OS_bcm:%=%-hsl-bcm)
APP_HSL_android      := $(APP_HSL_OS_bcm:%=%-hsl-android)

## --------------------------------------------------------------------------------
## For Release Source Package
## --------------------------------------------------------------------------------
# Release Source Packages for each <oem> (eg: dhd, wl, bcmdbus)
RLS_SRC_PKG_bcm       ?=
RLS_SRC_PKG_android   ?=

# Mogrify flags for each Release Source Package for each <oem>
# RLS_SRC_PKG_MOGRIFY_FLAGS_bcm_dhd      ?=
# RLS_SRC_PKG_MOGRIFY_FLAGS_bcm_wl       ?=
# RLS_SRC_PKG_MOGRIFY_FLAGS_bcm_bcmdbus  ?=

# Filelist for each Release Source Package for each <oem>
# RLS_SRC_PKG_FILELIST_bcm_dhd      ?=
# RLS_SRC_PKG_FILELIST_bcm_bcmdbus  ?=
# RLS_SRC_PKG_FILELIST_bcm_wl       ?=

## --------------------------------------------------------------------------------
## List of targets to build for specific brands (or all: by default)
## --------------------------------------------------------------------------------
all: build_start checkout filelists prebuild_prep build_dongle_images copy_dongle_images build_host_drv build_apps build_hsl release build_end
release: release_host_drv release_apps release_hsl release_fw release_src verify_pkg_apps verify_pkg_src release_cleanup

## --------------------------------------------------------------------------------
## Common Targets used for all Brands or <oem>
## --------------------------------------------------------------------------------
build_start:
	@$(MARKSTART_BRAND)

# check out files
checkout: $(CHECKOUT_TGT) | build_start

filelists: | checkout
	@$(MARKSTART)
	cat $(BRAND_SRCFILELIST) | src/tools/release/rem_comments.awk > master_filelist.h
	gcc -E -undef $(FILES_DEFS) -o $(SRCFILELIST_FINAL) master_filelist.h
	@$(MARKEND)

build_end: | release
	@$(MARKEND_BRAND)

## --------------------------------------------------------------------------------
## Pre-Build preparation
## For each <oem> brand, a separate set of sources are filtered & mogrified
## --------------------------------------------------------------------------------
prebuild_prep: $(foreach oem,$(OEM_LIST),prebuild_$(oem)_prep) | filelists

$(foreach oem,$(OEM_LIST),prebuild_$(oem)_prep): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),prebuild_$(oem)_prep): | filelists
	@$(MARKSTART)
	@echo "Copy over sources to $(oem) build workspace"
	sort -u -o $(SRCFILELIST_FINAL)_$(oem) $(SRCFILELIST_FINAL)
	mkdir -p build/$(oem)
	$(FIND) src components router $(FIND_SKIPCMD) -print | $(SRCFILTER) -v $(SRCFILELIST_FINAL)_$(oem) | pax -rw build/$(oem)
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_FLAGS="$(MOGRIFY_FLAGS_COMMON) $(DEFS_$(oem))" MOGRIFY_DIRS=build/$(oem)/src
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_FLAGS="$(MOGRIFY_FLAGS_COMMON) $(DEFS_$(oem))" MOGRIFY_DIRS=build/$(oem)/router
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_FLAGS="$(MOGRIFY_FLAGS_COMMON) $(DEFS_$(oem))" MOGRIFY_DIRS=build/$(oem)/components/shared/
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_FLAGS="$(MOGRIFY_FLAGS_COMMON) $(DEFS_$(oem))" MOGRIFY_DIRS=build/$(oem)/components/router/
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_FLAGS="$(MOGRIFY_FLAGS_COMMON) $(DEFS_$(oem))" MOGRIFY_DIRS=build/$(oem)/components/apps/wps/
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_FLAGS="$(MOGRIFY_FLAGS_COMMON) $(DEFS_$(oem))" MOGRIFY_DIRS=build/$(oem)/components/phy/
	$(MAKE) -C build/$(oem)/src/include


## --------------------------------------------------------------------------------
## Build Host Drivers for all <oem>
## --------------------------------------------------------------------------------
build_host_drv: $(foreach oem,$(OEM_LIST),$(foreach drv_os,$(HOST_DRV_OS_$(oem)),$(foreach drv_type,$(HOST_DRV_TYPE_$(oem)),build_host_drv_$(oem)_$(drv_type)_$(drv_os)))) | prebuild_prep

$(foreach oem,$(OEM_LIST),$(foreach drv_os,$(HOST_DRV_OS_$(oem)),$(foreach drv_type,$(HOST_DRV_TYPE_$(oem)),build_host_drv_$(oem)_$(drv_type)_$(drv_os)))):: oem=$(shell echo $(@F) | cut -d_ -f4)
$(foreach oem,$(OEM_LIST),$(foreach drv_os,$(HOST_DRV_OS_$(oem)),$(foreach drv_type,$(HOST_DRV_TYPE_$(oem)),build_host_drv_$(oem)_$(drv_type)_$(drv_os)))):: drv_type=$(shell echo $(@F) | cut -d_ -f5)
$(foreach oem,$(OEM_LIST),$(foreach drv_os,$(HOST_DRV_OS_$(oem)),$(foreach drv_type,$(HOST_DRV_TYPE_$(oem)),build_host_drv_$(oem)_$(drv_type)_$(drv_os)))):: drv_os=$(shell echo $(@F) | cut -d_ -f6)
$(foreach oem,$(OEM_LIST),$(foreach drv_os,$(HOST_DRV_OS_$(oem)),$(foreach drv_type,$(HOST_DRV_TYPE_$(oem)),build_host_drv_$(oem)_$(drv_type)_$(drv_os)))):: opts=$(drv_os)-host-drv-opts
$(foreach oem,$(OEM_LIST),$(foreach drv_os,$(HOST_DRV_OS_$(oem)),$(foreach drv_type,$(HOST_DRV_TYPE_$(oem)),build_host_drv_$(oem)_$(drv_type)_$(drv_os)))):: | prebuild_prep
	@$(MARKSTART)
	@if [ "$(opts)" == "" ]; then \
	    echo "ERROR: 'opts' missing..."; \
	    exit 1; \
	fi
	@echo "opts ---> $(opts) ---> $($(opts))"
	set -e; for host_drv in $(RLS_HOST_DRIVERS_$(oem)_$(drv_type)_$(drv_os)); do \
		echo "Building the Host Driver: $$host_drv"; \
		if [ "$$drv_type" == "wl" ]; then \
			$(MAKE) -C build/$(oem)/src/wl/linux SHOWWLCONF=1 $(if $(BCM_MFGTEST), WLTEST=1) $($(opts)) $$host_drv; \
		elif [ "$$drv_type" == "dhd" ]; then \
			$(MAKE) -C build/$(oem)/src/dhd/linux $(if $(BCM_MFGTEST), WLTEST=1) BUILD_TAG=TRUNK $($(opts)) $$host_drv;  \
		else \
			echo "ERROR: Unknown 'drv_type'... "; \
			exit 1; \
		fi \
	done
	@$(MARKEND)


## --------------------------------------------------------------------------------
## Build APPS for all <oem>
## --------------------------------------------------------------------------------
.EXPORT_ALL_VARIABLES:
build_apps: $(foreach oem,$(OEM_LIST),$(foreach apps_os,$(APP_DRV_OS_$(oem)),build_apps_$(oem)_$(apps_os))) | prebuild_prep

$(foreach oem,$(OEM_LIST),$(foreach apps_os,$(APP_DRV_OS_$(oem)),build_apps_$(oem)_$(apps_os))):: oem=$(shell echo $(@F) | cut -d_ -f3)
$(foreach oem,$(OEM_LIST),$(foreach apps_os,$(APP_DRV_OS_$(oem)),build_apps_$(oem)_$(apps_os))):: apps_os=$(shell echo $(@F) | cut -d_ -f4)
$(foreach oem,$(OEM_LIST),$(foreach apps_os,$(APP_DRV_OS_$(oem)),build_apps_$(oem)_$(apps_os))):: opts=$(apps_os)-app-opts
$(foreach oem,$(OEM_LIST),$(foreach apps_os,$(APP_DRV_OS_$(oem)),build_apps_$(oem)_$(apps_os))):: | prebuild_prep
	@$(MARKSTART)
	@echo "opts ---> $(opts) ---> $($(opts))"
	$(MAKE) -C build/$(oem)/src/dhd/exe $(if $(BCM_MFGTEST), WLTEST=1) BUILD_TAG=TRUNK $($(opts))
	if [ "$(subst TARGETARCH=,,$(filter TARGETARCH%, $($(opts))))" == "x86_64" ]; then \
		$(MAKE) -C build/$(oem)/components/router/acsd WL_MEDIA_ACSD=1 acsd $($(opts)); \
		$(MAKE) -C build/$(oem)/components/router/acsd WL_MEDIA_ACSD=1 acs_cli $($(opts)); \
	fi
	if [ "$$oem" != "android" ]; then \
		$(MAKE) -C build/$(oem)/src/wl/exe $(if $(BCM_MFGTEST), WLTEST=1) $($(opts)); \
	fi
	if [ "$(subst TARGETARCH=,,$(filter TARGETARCH%, $($(opts))))" == "x86" ]; then \
		$(MAKE) -C src/usbdev/usbdl; \
		$(MAKE) -C src/tools/misc trx; \
	fi
	@$(MARKEND)


## --------------------------------------------------------------------------------
## Build HSL for all <oem>
## --------------------------------------------------------------------------------
build_hsl: $(foreach oem,$(OEM_LIST),$(foreach hsl_os,$(APP_HSL_OS_$(oem)),build_hsl_$(oem)_$(hsl_os))) | prebuild_prep

$(foreach oem,$(OEM_LIST),$(foreach hsl_os,$(APP_HSL_OS_$(oem)),build_hsl_$(oem)_$(hsl_os))):: oem=$(shell echo $(@F) | cut -d_ -f3)
$(foreach oem,$(OEM_LIST),$(foreach hsl_os,$(APP_HSL_OS_$(oem)),build_hsl_$(oem)_$(hsl_os))):: hsl_os=$(shell echo $(@F) | cut -d_ -f4)
$(foreach oem,$(OEM_LIST),$(foreach hsl_os,$(APP_HSL_OS_$(oem)),build_hsl_$(oem)_$(hsl_os))):: opts=$(hsl_os)-app-opts
$(foreach oem,$(OEM_LIST),$(foreach hsl_os,$(APP_HSL_OS_$(oem)),build_hsl_$(oem)_$(hsl_os))):: | prebuild_prep
	@$(MARKSTART)
	@echo "opts ---> $(opts) ---> $($(opts))"
	# $(MAKE) -C build/$(oem)/src/apps/bwl/linux/  $(if $(BCM_MFGTEST), WLTEST=1) $($(opts))
	$(MAKE) -C build/$(oem)/src/apps/wfi/linux/wfi_api/ $(if $(BCM_MFGTEST), WLTEST=1) $($(opts))
	$(MAKE) -C build/$(oem)/components/apps/wps/wpsapi/linux/ $(if $(BCM_MFGTEST), WLTEST=1) $($(opts)) BCM_WPS_IOTYPECOMPAT=1
	@$(MARKEND)


## --------------------------------------------------------------------------------
## Copy over built Host drivers to release/<oem> folders for each <oem>
## --------------------------------------------------------------------------------
release_host_drv: $(foreach oem,$(OEM_LIST),release_$(oem)_host_drv)

$(foreach oem,$(OEM_LIST),release_$(oem)_host_drv): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),release_$(oem)_host_drv): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),release_$(oem)_host_drv): |build_host_drv
	@$(MARKSTART)
	install -d -m 755 $(RELDIR)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))

	# Copy README and release notes (oem specific ones override generic)
	# touch $(RELDIR)/README.txt
	# install src/doc/BrandReadmes/$(BRAND).txt $(RELDIR)/README.txt
	# -install src/doc/BrandReadmes/$(BRAND)_oem_$(oem).txt $(RELDIR)/README.txt

	# Copying all the Host Drivers
	@echo "The List of Host Driver types: ---> $(HOST_DRV_TYPE_$(oem))"
	for drv_type in $(HOST_DRV_TYPE_$(oem)); do \
		dest_dir=$$drv_type"_driver"; \
		if [ "$$drv_type" == "dhd" ]; then \
			for drv_image_dir in $(shell find build/${oem}/src/dhd/linux/host/* -type d -name 'dhd-*'); do \
				install -d -m 755 $(RELDIR)/host/$$dest_dir/$$(basename $$drv_image_dir); \
				install -pv $$drv_image_dir/*.ko $(RELDIR)/host/$$dest_dir/$$(basename $$drv_image_dir); \
			done ; \
			for drv_image_dir in $(shell find build/${oem}/src/dhd/linux/host/* -type d -name 'dhd-*-armv7l-*'); do \
				install -d -m 755 $(RELDIR)/BCG_STB/$$dest_dir/$$(basename $$drv_image_dir); \
				install -pv $$drv_image_dir/*.ko $(RELDIR)/BCG_STB/$$dest_dir/$$(basename $$drv_image_dir); \
			done ; \
		elif [ "$$drv_type" == "wl" ]; then \
			for drv_image_dir in $(foreach image_dir, build/${oem}/src/wl/linux/obj-*, $(image_dir)); do \
				install -d -m 755 $(RELDIR)/host/$$dest_dir/$$(basename $$drv_image_dir); \
				install -pv $$drv_image_dir/*.ko $(RELDIR)/host/$$dest_dir/$$(basename $$drv_image_dir); \
			done ; \
##			for drv_image_dir in $(shell find build/${oem}/src/wl/linux/obj-* -type d -name 'obj-*-armv7l-*'); do \
##				install -d -m 755 $(RELDIR)/BCG_STB/$$dest_dir/$$(basename $$drv_image_dir); \
##				install -pv $$drv_image_dir/*.ko $(RELDIR)/BCG_STB/$$dest_dir/$$(basename $$drv_image_dir); \
##			done ; \
		else \
			echo "Unknown driver type..."; \
			exit 1; \
		fi; \
	done

#comment#	# Copying WPA Supplicant
#comment#	@if [ "$(oem)" == "bcm" ]; then \
#comment#		mkdir -p -v $(RELDIR)/BCG_STB/wpa_supp ; \
#comment# android-panda-wpa-supp is not ready for use, switch from \
#comment# ubuntu-external-wpa-sup later when it is ready \
#comment#	LATEST_DIR=`ls -1d /projects/hnd/swbuild/build_linux/$(WPA_SUPP_TAG)/android-panda-wpa-supp/* | sort -t. -n -k1,1 -k2,2 -k3,3 -k4,4 | tail -1` ; \
#comment#		LATEST_DIR=`ls -1d /projects/hnd/swbuild/build_linux/$(WPA_SUPP_TAG)/ubuntu-external-wpa-supp/* | sort -t. -n -k1,1 -k2,2 -k3,3 -k4,4 | tail -1` ; \
#comment#		LATEST_BUILD=`basename $${LATEST_DIR}` ; \
#comment#	for mdir in /projects/hnd/swbuild/build_linux/$(WPA_SUPP_TAG)/android-panda-wpa-supp/$${LATEST_BUILD}/release/external/base* ; do \
#comment#		for mdir in /projects/hnd/swbuild/build_linux/$(WPA_SUPP_TAG)/ubuntu-external-wpa-supp/$${LATEST_BUILD}/release/base* ; do \
#comment#			cp -a $${mdir} $(RELDIR)/BCG_STB/wpa_supp ; \
#comment#		done ; \
#comment#	fi
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)


## --------------------------------------------------------------------------------
## Copy over built APPS to release/<oem> folders for each <oem>
## --------------------------------------------------------------------------------
release_apps: $(foreach oem,$(OEM_LIST),release_$(oem)_apps)

$(foreach oem,$(OEM_LIST),release_$(oem)_apps): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),release_$(oem)_apps): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),release_$(oem)_apps): |build_apps
	@$(MARKSTART)
	install -d -m 755 $(RELDIR)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	# Copying native and other cross-compiled apps
	@echo "Copying native and cross compiled apps for $(foreach app, $(APP_DRIVERS_$(oem)), $(subst -$(oem),,$(app))) variants"
	for arch in $(foreach app, $(APP_DRIVERS_$(oem)), $(patsubst TARGETARCH=%,%,$(filter TARGETARCH%, $($(patsubst %-$(oem),%-opts,$(app)))))); do \
		if [ "$$arch" != "x86" ]; then \
			sarch=$$arch; \
		else \
			sarch=""; \
		fi; \
		echo "TARGETARCH=$$arch"; \
		mkdir -pv $(RELDIR)/apps/$$sarch; \
		if [ -f "build/$(oem)/src/dhd/exe/$$arch/dhd" ]; then \
			install -pv build/$(oem)/src/dhd/exe/$$arch/dhd \
				$(RELDIR)/apps/$$sarch/dhd; fi; \
		if [ -f "build/$(oem)/src/wl/exe/wl$$sarch" ]; then \
			install -pv build/$(oem)/src/wl/exe/wl$$sarch \
				$(RELDIR)/apps/$$sarch/wl; fi; \
		if [ "$$arch" == "x86_64" ]; then \
			if [ -f "build/$(oem)/components/router/acsd/acsd" ]; then \
				install -pv build/$(oem)/components/router/acsd/acsd \
					$(RELDIR)/apps/$$sarch/acsd;\
				install -pv build/$(oem)/components/router/acsd/acs_cli \
					$(RELDIR)/apps/$$sarch/acs_cli;\
				install -pv build/$(oem)/components/router/acsd/acsd_config.txt \
					$(RELDIR)/apps/$$sarch/acsd_config.txt; fi; fi; \
		if [ -f "build/$(oem)/src/wl/exe/socket_noasd/$$arch/wl_server_socket$$sarch" ]; then \
			install -pv build/$(oem)/src/wl/exe/socket_noasd/$$arch/wl_server_socket$$sarch \
				$(RELDIR)/apps/$$sarch/wl_server_socket; fi; \
		if [ -f "build/$(oem)/src/wl/exe/dongle_noasd/$$arch/wl_server_dongle$$sarch" ]; then \
			install -pv build/$(oem)/src/wl/exe/dongle_noasd/$$arch/wl_server_dongle$$sarch \
				$(RELDIR)/apps/$$sarch/wl_server_dongle; fi; \
		if [ -f "build/$(oem)/src/wl/exe/serial_noasd/$$arch/wl_server_serial$$sarch" ]; then \
			install -pv build/$(oem)/src/wl/exe/serial_noasd/$$arch/wl_server_serial$$sarch \
				$(RELDIR)/apps/$$sarch/wl_server_serial; fi; \
		if [ -f "build/$(oem)/src/wl/exe/wifi_noasd/$$arch/wl_server_wifi$$sarch" ]; then \
			install -pv build/$(oem)/src/wl/exe/wifi_noasd/$$arch/wl_server_wifi$$sarch \
				$(RELDIR)/apps/$$sarch/wl_server_wifi; fi; \
		if [ -f "build/$(oem)/src/usbdev/usbdl/$$arch/bcmdl" ]; then \
			install -pv build/$(oem)/src/usbdev/usbdl/$$arch/bcmdl $(RELDIR)/apps/$$sarch; fi; \
	done
##	install -d -m 755 $(RELDIR)/BCG_STB/apps
##	if [ -d "$(RELDIR)/apps/armv7l" ]; then \
##		cp -rp $(RELDIR)/apps/armv7l $(RELDIR)/BCG_STB/apps ; \
##	fi
##	if [ -d "$(RELDIR)/BCG_STB" ]; then \
##		install -d -m 755 $(RELDIR)/packages ; \
##		tar -cpzf $(RELDIR)/packages/BCG-STB-pkg.tar.gz \
##			$(RELDIR)/BCG_STB ; \
##	fi
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)


## --------------------------------------------------------------------------------
## Copy over built HSL binaries to release/<oem> folders for each <oem>
## --------------------------------------------------------------------------------
release_hsl: $(foreach oem,$(OEM_LIST),release_$(oem)_hsl)

$(foreach oem,$(OEM_LIST),release_$(oem)_hsl): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),release_$(oem)_hsl): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),release_$(oem)_hsl): |build_hsl
	@$(MARKSTART)
	install -d -m 755 $(RELDIR)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	# Copying native and other cross-compiled wl and dhd exe apps
	@echo "Copying native and cross compiled apps for $(foreach app, $(APP_DRIVERS_$(oem)), $(subst -$(oem),,$(app))) variants"
	for arch in $(foreach app, $(APP_DRIVERS_$(oem)), $(patsubst TARGETARCH=%,%,$(filter TARGETARCH%, $($(patsubst %-$(oem),%-opts,$(app)))))); do \
		if [ "$$arch" != "x86" ]; then \
			sarch=$$arch; \
		else \
			sarch=""; \
		fi; \
		echo "TARGETARCH=$$arch"; \
		mkdir -pv $(RELDIR)/apps/$$sarch; \
		if [ -f "build/$(oem)/components/apps/wps/wpscli/linux/obj/$$arch-release/wpscli_test_cmd" ]; then \
			install -pv build/$(oem)/components/apps/wps/wpscli/linux/obj/$$arch-release/wpscli_test_cmd \
				$(RELDIR)/apps/$$sarch; fi; \
		if [ -f "build/$(oem)/src/apps/wfi/linux/wfi_api/obj/$$arch-release/wfiapitester" ]; then \
			install -pv build/$(oem)/src/apps/wfi/linux/wfi_api/obj/$$arch-release/wfiapitester \
				$(RELDIR)/apps/$$sarch; fi; \
		if [ -f "build/$(oem)/components/apps/wps/wpsapi/linux/release/$$arch/wpsenr" ]; then \
			install -pv build/$(oem)/components/apps/wps/wpsapi/linux/release/$$arch/wpsenr \
				$(RELDIR)/apps/$$sarch; fi; \
		if [ -f "build/$(oem)/components/apps/wps/wpsapi/linux/release/$$arch/wpsreg" ]; then \
			install -pv build/$(oem)/components/apps/wps/wpsapi/linux/release/$$arch/wpsreg \
				$(RELDIR)/apps/$$sarch; fi; \
		if [ -f "build/$(oem)/components/apps/wps/wpsapi/linux/release/$$arch/wpsapitester" ]; then \
			install -pv build/$(oem)/components/apps/wps/wpsapi/linux/release/$$arch/wpsapitester \
				$(RELDIR)/apps/$$sarch; fi; \
	done
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)


## --------------------------------------------------------------------------------
## Copy over built firmwares to release/<oem> folders for each <oem>
## --------------------------------------------------------------------------------
release_fw: $(foreach oem,$(OEM_LIST),release_$(oem)_fw)

$(foreach oem,$(OEM_LIST),release_$(oem)_fw): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),release_$(oem)_fw): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),release_$(oem)_fw): |build_dongle_images copy_dongle_images
	@$(MARKSTART)
	install -d -m 755 $(RELDIR)/firmware
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	# START: Copying dongle images to both build/<oem> and release/<oem> folders
	@echo "Copying $(RLS_FIRMWARES_$(oem)) to firmware folder now"; \
	@failedimgs=""; \
	for img in $(RLS_FIRMWARES_$(oem)); do \
	    if [ ! -s "$(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin" -o \
	         ! -s "$(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).h" ]; then \
	        echo "WARN: Dongle image $${img} NOT BUILT"; \
	        failedimgs="  $${failedimgs} $${img}\n"; \
	    else \
	        if [ -s "$(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin.trx" ] && [[ $$img =~ usb ]]; then \
	            install -pDv $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin.trx $(RELDIR)/firmware/$${img}.bin.trx; \
	        elif [ -s "$(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin" ] && [[ $$img =~ pcie ]]; then \
	            install -pDv $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin $(RELDIR)/firmware/$${img}.bin; \
	        elif [ -s "$(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin.trx" ]; then \
	            install -pDv $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin.trx $(RELDIR)/firmware/$${img}.bin.trx; \
	        fi; \
	        if [ -s "$(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).map" ] && [ "$(BLD_TYPE)" != "release" ]; then \
	            install -pDv $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).map $(RELDIR)/firmware/$${img}.map; \
	        fi; \
	    fi; \
	done; \
	if [ "$${failedimgs}" != "" ]; then \
	    echo "ERROR: Following dongle images failed to build"; \
	    echo -e "\n$${failedimgs}"; \
	    exit 1; \
	fi; \
	# END: Copying dongle images to both build/<oem> and release/<oem> folders
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)

## --------------------------------------------------------------------------------
## Create brand specific source packages for each <oem>
## --------------------------------------------------------------------------------
release_src: $(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_$(oem)_src_pkg_$(pkg)))

$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_$(oem)_src_pkg_$(pkg))): oem=$(shell echo $(@F) | cut -d_ -f2)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_$(oem)_src_pkg_$(pkg))): src_pkg=$(shell echo $(@F) | cut -d_ -f5)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_$(oem)_src_pkg_$(pkg))): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_$(oem)_src_pkg_$(pkg))): src_pkg_dir=$(src_pkg)_src
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_$(oem)_src_pkg_$(pkg))): src_pkg_name=linux-src-pkg-$(src_pkg).tar.gz
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_$(oem)_src_pkg_$(pkg))): src_pkg_filelist=linux-src-pkg-$(src_pkg)-filelist
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_$(oem)_src_pkg_$(pkg))): release_host_drv release_apps release_hsl
	@$(MARKSTART)
	mkdir -p $(RELDIR)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	@echo "Generating release source package for '$(src_pkg)' in '$(RELDIR)' ...."
	@echo "src_pkg_dir = $(src_pkg_dir)"
	@echo "src_pkg_name = $(src_pkg_name)"
	@echo "src_pkg_filelist = $(src_pkg_filelist)"
	install -d -m 755 $(RELDIR)/packages
	install -d $(RELDIR)/packages/$(src_pkg_dir)
	cat $(RLS_SRC_PKG_FILELIST_$(oem)_$(src_pkg)) | \
		 src/tools/release/rem_comments.awk > $(src_pkg_filelist)-temp.h
	gcc -E -undef \
		$(RLS_SRC_PKG_FILELIST_DEFS_$(oem)_$(src_pkg):%=-D%) \
		$(RLS_SRC_PKG_FILELIST_UNDEFS_$(oem)_$(src_pkg):%=-U%) \
		-o $(src_pkg_filelist)-temp.txt $(src_pkg_filelist)-temp.h
	cat $(src_pkg_filelist)-temp.txt | src/tools/release/rem_comments.awk | \
		sort -u | perl -lane "print if /\S/" > $(RELDIR)/packages/$(src_pkg_filelist).txt
	cd build/$(oem) && \
		find src components $(FIND_SKIPCMD) -print | \
		$(SRCFILTER) -v -e ../../$(RELDIR)/packages/$(src_pkg_filelist).txt | \
		pax -rw ../../$(RELDIR)/packages/$(src_pkg_dir)
	$(MAKE) -f $(MOGRIFY_RULES) \
		MOGRIFY_DIRS=$(RELDIR)/packages/$(src_pkg_dir) \
		MOGRIFY_FLAGS="$(RLS_SRC_PKG_MOGRIFY_FLAGS_$(oem)_$(src_pkg)) \
			$(patsubst %,-U%,$(RLS_SRC_PKG_MOGRIFY_UNDEFS_$(oem)_$(src_pkg))) \
			$(patsubst %,-D%,$(RLS_SRC_PKG_MOGRIFY_DEFS_$(oem)_$(src_pkg)))"
	tar -cpzf $(RELDIR)/packages/$(src_pkg_name) \
		-C $(RELDIR)/packages/$(src_pkg_dir) \
		-T $(RELDIR)/packages/$(src_pkg_filelist).txt
	rm -f $(src_pkg_filelist)-temp.h $(src_pkg_filelist)-temp.txt
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)

## --------------------------------------------------------------------------------
## Verify the package source for few host targets
## --------------------------------------------------------------------------------
verify_pkg_src: $(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_DRV_$(oem)),$(foreach drv_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_host_drv_$(oem)_$(pkg)_pkg_$(drv_os))))

$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_DRV_$(oem)),$(foreach drv_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_host_drv_$(oem)_$(pkg)_pkg_$(drv_os)))): oem=$(shell echo $(@F) | cut -d_ -f4)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_DRV_$(oem)),$(foreach drv_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_host_drv_$(oem)_$(pkg)_pkg_$(drv_os)))): pkg_type=$(shell echo $(@F) | cut -d_ -f5)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_DRV_$(oem)),$(foreach drv_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_host_drv_$(oem)_$(pkg)_pkg_$(drv_os)))): drv_os=$(shell echo $(@F) | cut -d_ -f7)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_DRV_$(oem)),$(foreach drv_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_host_drv_$(oem)_$(pkg)_pkg_$(drv_os)))): opts=$(drv_os)-host-drv-opts
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_DRV_$(oem)),$(foreach drv_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_host_drv_$(oem)_$(pkg)_pkg_$(drv_os)))): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_DRV_$(oem)),$(foreach drv_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_host_drv_$(oem)_$(pkg)_pkg_$(drv_os)))): src_pkg_dir=$(pkg_type)_src
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_DRV_$(oem)),$(foreach drv_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_host_drv_$(oem)_$(pkg)_pkg_$(drv_os)))): release_src
	@$(MARKSTART)
	mkdir -p $(RELDIR)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	@if [ "$($(opts))" == "" ]; then \
	    echo "ERROR: 'opts' missing..." >&2; \
	    exit 1; \
	fi
	@echo "opts ---> $(opts) ---> $($(opts))"
	set -e; for host_drv in $(RLS_HOST_DRIVERS_PKG_$(oem)_$(pkg_type)_$(drv_os)); do \
		echo "Building the Host Driver: $$host_drv"; \
		if [ "$$pkg_type" == "wl" ]; then \
			$(MAKE) -C $(RELDIR)/packages/$(src_pkg_dir)/src/wl/linux SHOWWLCONF=1 $(if $(BCM_MFGTEST), WLTEST=1) $($(opts)) $$host_drv; \
		elif [ "$$pkg_type" == "dhd" ]; then \
			$(MAKE) -C $(RELDIR)/packages/$(src_pkg_dir)/src/dhd/linux $(if $(BCM_MFGTEST), WLTEST=1) BUILD_TAG=TRUNK $($(opts)) $$host_drv;  \
		else \
			echo "ERROR: Unknown 'pkg_type': $$pkg_type " >&2; \
			exit 1; \
		fi \
	done
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)

## --------------------------------------------------------------------------------
## Verify wl and dhd apps from respective packages
## --------------------------------------------------------------------------------
verify_pkg_apps: $(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_APPS_$(oem)),$(foreach apps_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_apps_$(oem)_$(pkg)_pkg_$(apps_os))))

$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_APPS_$(oem)),$(foreach apps_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_apps_$(oem)_$(pkg)_pkg_$(apps_os)))): oem=$(shell echo $(@F) | cut -d_ -f3)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_APPS_$(oem)),$(foreach apps_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_apps_$(oem)_$(pkg)_pkg_$(apps_os)))): pkg=$(shell echo $(@F) | cut -d_ -f4)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_APPS_$(oem)),$(foreach apps_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_apps_$(oem)_$(pkg)_pkg_$(apps_os)))): apps_os=$(shell echo $(@F) | cut -d_ -f6)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_APPS_$(oem)),$(foreach apps_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_apps_$(oem)_$(pkg)_pkg_$(apps_os)))): opts=$(apps_os)-app-opts
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_APPS_$(oem)),$(foreach apps_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_apps_$(oem)_$(pkg)_pkg_$(apps_os)))): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_VERIFY_APPS_$(oem)),$(foreach apps_os,$(HOST_OS_PKG_VERIFY_$(oem)),verify_apps_$(oem)_$(pkg)_pkg_$(apps_os)))): release_apps
	@$(MARKSTART)
	@echo "opts ---> $(opts) ---> $($(opts))"
	if [ "$$pkg" == "dhd" ]; then \
		$(MAKE) -C $(RELDIR)/packages/dhd_src/src/dhd/exe $(if $(BCM_MFGTEST), WLTEST=1) $($(opts)); \
	elif [ "$$pkg" == "wl" ]; then \
		$(MAKE) -C $(RELDIR)/packages/wl_src/src/wl/exe $(if $(BCM_MFGTEST), WLTEST=1) $($(opts)); \
	else \
		echo "Currently verification of this 'pkg_type': $$pkg is not supported"; \
	fi
	@$(MARKEND)

## -------------------------------------------------------------------------------------------
## Remove / delete all source package folders created under 'release_src'
## Note: 'release_cleanup' must be invoked only after 'release_src'
## Note: 'verify_pkg_src' and 'verify_pkg_apps' must be invoked only before 'release_cleanup'
## -------------------------------------------------------------------------------------------
release_cleanup: $(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_cleanup_$(oem)_src_pkg_$(pkg)))

$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_cleanup_$(oem)_src_pkg_$(pkg))): oem=$(shell echo $(@F) | cut -d_ -f3)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_cleanup_$(oem)_src_pkg_$(pkg))): src_pkg=$(shell echo $(@F) | cut -d_ -f6)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_cleanup_$(oem)_src_pkg_$(pkg))): RELDIR=release/$(oem)
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_cleanup_$(oem)_src_pkg_$(pkg))): src_pkg_dir=$(src_pkg)_src
$(foreach oem,$(OEM_LIST),$(foreach pkg,$(RLS_SRC_PKG_$(oem)),release_cleanup_$(oem)_src_pkg_$(pkg))):
	@$(MARKSTART)
	mkdir -p $(RELDIR)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	@echo "Deleting release source folder for '$(src_pkg)' in '$(RELDIR)' "
	rm -rf $(RELDIR)/packages/$(src_pkg_dir)
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	@$(MARKEND)

.PHONY: FORCE checkout build_include prebuild_prep build_host_drv $(foreach oem,$(OEM_LIST),build_host_drv_$(oem)) build_dongle_images copy_dongle_images release_build_clean build_apps $(foreach oem,$(OEM_LIST),$(APP_DRIVERS_$(oem))) build_hsl release release_bins release_apps release_hsl release_fw release_src
