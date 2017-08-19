#
# This brand makefile is used to build the Host Drivers, Firmwares, Apps and HSL
# for the BRAND linux-mfgtest-media
#
# $Id$
#

PARENT_MAKEFILE := linux-media.mk

BRAND       := linux-mfgtest-media

## -----------------------------------------------------------------------
## The Brand specific filelist DEFS and UNDEFS
## -----------------------------------------------------------------------
MEDIA_BRAND_SRCFILELIST_DEFS   := BCMINTERNAL WLTEST
MEDIA_BRAND_SRCFILELIST_UNDEFS :=

## -----------------------------------------------------------------------
## The Brand specific Mogrification DEFS and UNDEFS
## -----------------------------------------------------------------------
MEDIA_BRAND_MOGRIFY_DEFS     :=
MEDIA_BRAND_MOGRIFY_UNDEFS   := DHD_AWDL WLAWDL

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
RLS_SRC_PKG_MOGRIFY_UNDEFS_bcm_wl  := OEM_ANDROID DHD_AWDL WLAWDL
RLS_SRC_PKG_MOGRIFY_FLAGS_bcm_wl   :=

# Filelist for each Release Source Package for each <oem>
RLS_SRC_PKG_FILELIST_bcm_wl        := src/tools/release/linux-media-filelist.txt \
		src/tools/release/components/wlphy-filelist.txt \
		src/tools/release/linux-wlexe-filelist.txt
RLS_SRC_PKG_FILELIST_DEFS_bcm_wl   := \
	AP_SRC BCMASSERT_LOG_SRC BCMAUTH_PSK_SRC BCMCCX_SRC BCMNVRAMR_SRC \
	BCMSUP_PSK_SRC OSLLX_SRC WL_SRC WLAMPDU_SRC WLAMSDU_SRC WLAPSDSTD_SRC \
	WLEXTLOG_SRC WLLED_SRC WLLX_SRC WLSRC_SRC WL_FW_DECOMP_SRC L2_FILTER_SRC
RLS_SRC_PKG_FILELIST_UNDEFS_bcm_wl :=

## -----------------------------------------------------------------------
## List of Host Driver images to build for each <oem>
## -----------------------------------------------------------------------
# The Operating System of the Host Driver Builds
# HOST_DRV_OS_bcm    := fc15 fc19x64 stb7346le2637 stb7346le336
HOST_DRV_OS_bcm  := fc19x64 fc22x64 \
	stb7252armlx314111 stb7252armlx4110 stb7425mipslx33

HOST_DRV_TYPE_bcm  := dhd wl

# Build sanity of Source Packages to be verified on following OS/platform
HOST_OS_PKG_VERIFY_bcm    ?= fc19x64

RLS_HOST_DRIVERS_bcm_dhd_fc19x64 := \
	dhd-cdc-usb-gpl-media-debug \
	dhd-cdc-usb-comp-gpl-media-debug \
	dhd-cdc-usb-reqfw-comp-gpl-media-debug \
	dhd-msgbuf-pciefd-media-debug

RLS_HOST_DRIVERS_bcm_dhd_fc22x64   := \
	dhd-msgbuf-pciefd-debug

RLS_HOST_DRIVERS_bcm_dhd_stb7425mipslx33 := \
	dhd-cdc-usb-comp-gpl-mips-debug \
	dhd-msgbuf-pciefd-mfp-mips-debug

RLS_HOST_DRIVERS_bcm_dhd_stb7252armlx314111 := \
	dhd-msgbuf-pciefd-mfp-armv7l-debug \
	dhd-cdc-usb-comp-gpl-armv7l-debug \
	dhd-msgbuf-pciefd-mfp-secdma-armv7l-debug

RLS_HOST_DRIVERS_bcm_dhd_stb7252armlx4110 := \
	dhd-msgbuf-pciefd-mfp-armv7l-debug \
	dhd-cdc-usb-comp-gpl-armv7l-debug \
	dhd-msgbuf-pciefd-mfp-secdma-armv7l-debug

RLS_HOST_DRIVERS_bcm_wl_fc19x64 := \
	debug-apdef-stadef-mfgtest \
	debug-apdef-stadef-slvradar-mfgtest

RLS_HOST_DRIVERS_bcm_wl_fc22x64 := \
	debug-apdef-stadef-mfgtest \
	debug-apdef-stadef-slvradar-mfgtest

RLS_HOST_DRIVERS_bcm_wl_stb7252armlx314111 := \
	debug-apdef-stadef-mfgtest-stb-armv7l \
	debug-apdef-stadef-mfgtest-stb-slvradar-armv7l \
	debug-apdef-stadef-mfgtest-extnvm-stb-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7252armlx4110 := \
	debug-apdef-stadef-mfgtest-stb-armv7l \
	debug-apdef-stadef-mfgtest-stb-slvradar-armv7l \
	debug-apdef-stadef-mfgtest-extnvm-stb-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7425mipslx33 := \
	debug-apdef-stadef-mfgtest-stb-mips \
	debug-apdef-stadef-mfgtest-extnvm-stb-mips \
	debug-apdef-stadef-p2p-mchan-tdls-wowl-mfgtest-extnvm-stb-mips

RLS_HOST_DRIVERS_PKG_bcm_wl_fc19x64 := \
	debug-apdef-stadef-mfgtest

## -----------------------------------------------------------------------
## List of Dongle Firmware builds needed for each <oem>
## -----------------------------------------------------------------------
RLS_FIRMWARES_USB_FD_bcm  :=

RLS_FIRMWARES_PCIE_FD_bcm  := \
	43602a1-ram/pcie-ag-splitrx-mfgtest

RLS_FIRMWARES_bcm  := $(RLS_FIRMWARES_USB_FD_bcm) $(RLS_FIRMWARES_PCIE_FD_bcm)

# List FW mages to embed into the Host driver for each <oem> (if BCMEMBEDIMAGE is ON)
EMBED_FIRMWARES_bcm  :=

# Define ALL_DNGL_IMAGES here so that 'hndrte-dongle-wl' build will pickup from here
ALL_DNGL_IMAGES    := $(RLS_FIRMWARES_bcm) $(EMBED_FIRMWARES_bcm)


## -----------------------------------------------------------------------
## Include the Parent Makefile
## -----------------------------------------------------------------------
include $(PARENT_MAKEFILE)
