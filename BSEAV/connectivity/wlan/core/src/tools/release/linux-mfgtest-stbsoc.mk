#
# This brand makefile is used to build the Host Drivers, Firmwares, Apps and HSL
# for the BRAND linux-mfgtest-stbsoc
#
# $Id: $
#

PARENT_MAKEFILE := linux-stbsoc.mk

BRAND       := linux-mfgtest-stbsoc

## -----------------------------------------------------------------------
## The Brand specific filelist DEFS and UNDEFS
## -----------------------------------------------------------------------
STBSOC_BRAND_SRCFILELIST_DEFS   := WLTEST
STBSOC_BRAND_SRCFILELIST_UNDEFS := BCMINTERNAL

## -----------------------------------------------------------------------
## The Brand specific Mogrification DEFS and UNDEFS
## -----------------------------------------------------------------------
STBSOC_BRAND_MOGRIFY_DEFS     :=
STBSOC_BRAND_MOGRIFY_UNDEFS   := DHD_AWDL WLAWDL BCMINTERNAL

## -----------------------------------------------------------------------
## The Driver Types and Platform/OS specific definitions for each <oem>
## -----------------------------------------------------------------------
# Release Source Packages for each <oem> (eg: dhd, wl, bcmdbus)
RLS_SRC_PKG_bcm   := wl

# Build sanity to be verified for the following Release Source Packages
RLS_SRC_PKG_VERIFY_DRV_bcm   := wl
RLS_SRC_PKG_VERIFY_APPS_bcm  := wl

# Mogrify flags for each Release Source Package for each <oem>

RLS_SRC_PKG_MOGRIFY_DEFS_bcm_wl    :=
RLS_SRC_PKG_MOGRIFY_UNDEFS_bcm_wl  := OEM_ANDROID
RLS_SRC_PKG_MOGRIFY_FLAGS_bcm_wl   :=

# Filelist for each Release Source Package for each <oem>
RLS_SRC_PKG_FILELIST_bcm_wl        := src/tools/release/linux-stbsoc-filelist.txt \
		src/tools/release/components/wlphy-filelist.txt \
		src/tools/release/linux-wlexe-filelist.txt
RLS_SRC_PKG_FILELIST_DEFS_bcm_wl   := \
	AP_SRC BCMASSERT_LOG_SRC BCMAUTH_PSK_SRC BCMCCX_SRC BCMNVRAMR_SRC \
	BCMSUP_PSK_SRC OSLLX_SRC WL_SRC WLAMPDU_SRC WLAMSDU_SRC WLAPSDSTD_SRC \
	WLEXTLOG_SRC WLLED_SRC WLLX_SRC WLSRC_SRC WL_FW_DECOMP_SRC L2_FILTER_SRC \
	WET_SRC WLP2P_SRC WOWL_SRC
RLS_SRC_PKG_FILELIST_UNDEFS_bcm_wl :=

## -----------------------------------------------------------------------
## List of Host Driver images to build for each <oem>
## -----------------------------------------------------------------------
# The Operating System of the Host Driver Builds
HOST_DRV_OS_bcm  := stb7271armlx4118 stb7271armlx411864bit stb7271armlx41110 stb7271armlx4111064bit

HOST_DRV_TYPE_bcm  := wl

# Build sanity of Source Packages to be verified on following OS/platform
HOST_OS_PKG_VERIFY_bcm    ?= stb7271armlx41110

RLS_HOST_DRIVERS_bcm_wl_stb7271armlx41110 := \
	debug-apdef-stadef-mfgtest-extnvm-stbsoc-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7271armlx4111064bit := \
	debug-apdef-stadef-mfgtest-extnvm-stbsoc-armv8

RLS_HOST_DRIVERS_bcm_wl_stb7271armlx4118 := \
	debug-apdef-stadef-mfgtest-extnvm-stbsoc-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7271armlx411864bit := \
	debug-apdef-stadef-mfgtest-extnvm-stbsoc-armv8

RLS_HOST_DRIVERS_PKG_bcm_wl_stb7271armlx41110 := \
	debug-apdef-stadef-mfgtest-extnvm-stbsoc-armv7l

RLS_HOST_DRIVERS_PKG_bcm_wl_stb7271armlx4111064bit := \
	debug-apdef-stadef-mfgtest-extnvm-stbsoc-armv8

## -----------------------------------------------------------------------
## List of Dongle Firmware builds needed for each <oem>
## -----------------------------------------------------------------------
#RLS_FIRMWARES_USB_FD_bcm  :=

# RLS_FIRMWARES_USB_BMAC_bcm  :=

# RLS_FIRMWARES_PCIE_FD_bcm  :=

# RLS_FIRMWARES_bcm  := $(RLS_FIRMWARES_USB_FD_bcm) $(RLS_FIRMWARES_PCIE_FD_bcm) $(RLS_FIRMWARES_USB_BMAC_bcm)

# List FW mages to embed into the Host driver for each <oem> (if BCMEMBEDIMAGE is ON)
# EMBED_FIRMWARES_bcm  :=

# Define ALL_DNGL_IMAGES here so that 'hndrte-dongle-wl' build will pickup from here
# ALL_DNGL_IMAGES    := $(RLS_FIRMWARES_bcm) $(EMBED_FIRMWARES_bcm)


## -----------------------------------------------------------------------
## Include the Parent Makefile
## -----------------------------------------------------------------------
include $(PARENT_MAKEFILE)
