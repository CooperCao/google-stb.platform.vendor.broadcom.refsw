#
# This brand makefile is used to build the Host Drivers, Firmwares, Apps and HSL
# for the BRAND linux-internal-stbsoc
#
# $Id: $
#

PARENT_MAKEFILE := linux-stbsoc.mk

BRAND       := linux-internal-stbsoc

## -----------------------------------------------------------------------
## The Brand specific filelist DEFS and UNDEFS
## -----------------------------------------------------------------------
STBSOC_BRAND_SRCFILELIST_DEFS   := BCMINTERNAL WLTEST
STBSOC_BRAND_SRCFILELIST_UNDEFS :=

## -----------------------------------------------------------------------
## The Brand specific Mogrification DEFS and UNDEFS
## -----------------------------------------------------------------------
STBSOC_BRAND_MOGRIFY_DEFS     :=
STBSOC_BRAND_MOGRIFY_UNDEFS   := DHD_AWDL WLAWDL

## -----------------------------------------------------------------------
## List of Host Driver images to build for each <oem>
## -----------------------------------------------------------------------
# The Operating System of the Host Driver Builds

HOST_DRV_OS_bcm  := stb7271armlx41110

HOST_DRV_TYPE_bcm  := wl

# Build sanity of Source Packages to be verified on following OS/platform
HOST_OS_PKG_VERIFY_bcm    ?=


RLS_HOST_DRIVERS_bcm_wl_stb7271armlx41110 := \
	debug-bcmintdbg-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7271armlx4111064bit := \
	debug-bcmintdbg-apdef-stadef-extnvm-stbsoc-armv8 \
	debug-bcmintdbg-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv8 \
	debug-bcmintdbg-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv8

RLS_HOST_DRIVERS_bcm_wl_stb7271armlx4118 := \
	debug-bcmintdbg-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7271armlx411864bit := \
	debug-bcmintdbg-apdef-stadef-extnvm-stbsoc-armv8 \
	debug-bcmintdbg-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv8 \
	debug-bcmintdbg-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv8

RLS_HOST_DRIVERS_PKG_bcm_wl_stb7271armlx4112 := \
	debug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv7l \
	debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-wowl-stbsoc-armv7l \
	debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv7l \
	debug-apdef-stadef-extnvm-stbsoc-armv7l

RLS_HOST_DRIVERS_PKG_bcm_wl_stb7271armlx411264bit := \
	debug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv8 \
	debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-wowl-stbsoc-armv8 \
	debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv8 \
	debug-apdef-stadef-extnvm-stbsoc-armv8

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
