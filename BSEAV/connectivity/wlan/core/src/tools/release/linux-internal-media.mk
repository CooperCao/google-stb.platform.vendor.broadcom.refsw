#
# This brand makefile is used to build the Host Drivers, Firmwares, Apps and HSL
# for the BRAND linux-internal-media
#
# $Id$
#

PARENT_MAKEFILE := linux-media.mk

BRAND       := linux-internal-media

## -----------------------------------------------------------------------
## The Brand specific filelist DEFS and UNDEFS
## -----------------------------------------------------------------------
MEDIA_BRAND_SRCFILELIST_DEFS   := BCMINTERNAL
MEDIA_BRAND_SRCFILELIST_UNDEFS := WLTEST

## -----------------------------------------------------------------------
## The Brand specific Mogrification DEFS and UNDEFS
## -----------------------------------------------------------------------
MEDIA_BRAND_MOGRIFY_DEFS     :=
MEDIA_BRAND_MOGRIFY_UNDEFS   := DHD_AWDL WLAWDL

## -----------------------------------------------------------------------
## List of Host Driver images to build for each <oem>
## -----------------------------------------------------------------------
# The Operating System of the Host Driver Builds
HOST_DRV_OS_bcm    := fc19x64 fc22x64 \
	stb7445armlx31419 stb7252armlx314111 stb7252armlx4110 stb7425mipslx33

HOST_DRV_TYPE_bcm  := dhd wl

RLS_HOST_DRIVERS_bcm_dhd_fc19x64 := \
	dhd-cdc-usb-gpl-media-debug \
	dhd-cdc-usb-comp-gpl-media-debug \
	dhd-cdc-usb-reqfw-comp-gpl-media-debug \
	dhd-msgbuf-pciefd-media-debug

RLS_HOST_DRIVERS_bcm_dhd_fc22x64   := \
	dhd-msgbuf-pciefd-debug

RLS_HOST_DRIVERS_bcm_dhd_stb7425mipslx33 := \
	dhd-cdc-usb-comp-gpl-mips-debug \
	dhd-cdc-usb-reqfw-comp-gpl-mips-debug \
	dhd-cdc-usb-android-media-cfg80211-mips-debug \
	dhd-cdc-usb-android-media-cfg80211-comp-mfp-mips-debug \
	dhd-cdc-usb-android-media-cfg80211-noproptxstatus-mips-debug \
	dhd-cdc-usb-reqfw-android-media-cfg80211-comp-mfp-mips-debug \
	dhd-msgbuf-pciefd-mfp-mips-debug \
	dhd-msgbuf-pciefd-android-media-cfg80211-mfp-mips-debug

RLS_HOST_DRIVERS_bcm_dhd_stb7252armlx314111 := \
	dhd-cdc-usb-comp-gpl-armv7l-debug \
	dhd-cdc-usb-reqfw-comp-gpl-armv7l-debug \
	dhd-cdc-usb-android-media-cfg80211-comp-mfp-armv7l-debug \
	dhd-cdc-usb-android-media-cfg80211-noproptxstatus-armv7l-debug \
	dhd-cdc-usb-reqfw-android-media-cfg80211-comp-mfp-armv7l-debug \
	dhd-msgbuf-pciefd-mfp-armv7l-debug \
	dhd-msgbuf-pciefd-mfp-secdma-armv7l-debug \
	dhd-msgbuf-pciefd-mfp-stbap-armv7l-debug \
	dhd-msgbuf-pciefd-mfp-secdma-stbap-armv7l-debug \
	dhd-msgbuf-pciefd-android-media-cfg80211-mfp-armv7l-debug \
	dhd-msgbuf-pciefd-android-media-cfg80211-mfp-secdma-armv7l-debug

RLS_HOST_DRIVERS_bcm_dhd_stb7252armlx4110 := \
	dhd-cdc-usb-comp-gpl-armv7l-debug \
	dhd-cdc-usb-reqfw-comp-gpl-armv7l-debug \
	dhd-cdc-usb-android-media-cfg80211-comp-mfp-armv7l-debug \
	dhd-cdc-usb-android-media-cfg80211-noproptxstatus-armv7l-debug \
	dhd-cdc-usb-reqfw-android-media-cfg80211-comp-mfp-armv7l-debug \
	dhd-msgbuf-pciefd-mfp-armv7l-debug \
	dhd-msgbuf-pciefd-mfp-secdma-armv7l-debug \
	dhd-msgbuf-pciefd-mfp-stbap-armv7l-debug \
	dhd-msgbuf-pciefd-mfp-secdma-stbap-armv7l-debug \
	dhd-msgbuf-pciefd-android-media-cfg80211-mfp-armv7l-debug \
	dhd-msgbuf-pciefd-android-media-cfg80211-mfp-secdma-armv7l-debug

RLS_HOST_DRIVERS_bcm_wl_fc19x64 := \
	debug-apdef-stadef \
	debug-apdef-stadef-slvradar

RLS_HOST_DRIVERS_bcm_wl_fc22x64 := \
	debug-apdef-stadef-p2p-mchan-tdls-wowl-mfp

RLS_HOST_DRIVERS_bcm_wl_stb7445armlx31419 := \
	debug-apdef-stadef-stb-armv7l \
	debug-apdef-stadef-extnvm-stb-armv7l \
	debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-armv7l \
	debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-slvradar-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7252armlx314111 := \
	debug-apdef-stadef-stb-armv7l \
	debug-apdef-stadef-extnvm-stb-armv7l \
	debug-apdef-stadef-extnvm-mfp-stb-stbap-armv7l \
	debug-apdef-stadef-p2p-mchan-tdls-wowl-extnvm-stb-armv7l \
	debug-apdef-stadef-p2p-mchan-tdls-wowl-extnvm-mfp-stb-armv7l \
	debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-wet-slvradar-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7252armlx4110 := \
	debug-apdef-stadef-stb-armv7l \
	debug-apdef-stadef-extnvm-stb-armv7l \
	debug-apdef-stadef-extnvm-mfp-stb-stbap-armv7l \
	debug-apdef-stadef-p2p-mchan-tdls-wowl-extnvm-stb-armv7l \
	debug-apdef-stadef-p2p-mchan-tdls-wowl-extnvm-mfp-stb-armv7l \
	debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-armv7l \
	debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-slvradar-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7425mipslx33 := \
	debug-apdef-stadef-stb-mips \
	debug-apdef-stadef-extnvm-stb-mips \
	debug-apdef-stadef-p2p-mchan-tdls-wowl-extnvm-stb-mips \
	debug-apdef-stadef-p2p-mchan-tdls-wowl-extnvm-mfp-stb-mips

## -----------------------------------------------------------------------
## List of Dongle Firmware builds needed for each <oem>
## -----------------------------------------------------------------------
RLS_FIRMWARES_USB_FD_bcm  := \
	43569a2-ram/usb-ag-assert-pool \
        43569a2-ram/usb-ag-assert-pool-tdls \
	43569a2-ram/usb-ag-assert-pool-p2p-mchan

RLS_FIRMWARES_PCIE_FD_bcm  := \
	43602a1-ram/pcie-ag-splitrx \
	43602a1-ram/pcie-ag-splitrx-clm_min \
	43602a1-ram/pcie-ag-err-assert-splitrx \
	43602a1-ram/pcie-ag-err-assert-splitrx-clm_min \
	43602a1-ram/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-err-assert-dbgam

RLS_FIRMWARES_bcm  := $(RLS_FIRMWARES_USB_FD_bcm) $(RLS_FIRMWARES_PCIE_FD_bcm)

# List FW mages to embed into the Host driver for each <oem> (if BCMEMBEDIMAGE is ON)
EMBED_FIRMWARES_bcm  :=

# Define ALL_DNGL_IMAGES here so that 'hndrte-dongle-wl' build will pickup from here
ALL_DNGL_IMAGES    := $(RLS_FIRMWARES_bcm) $(EMBED_FIRMWARES_bcm)


## -----------------------------------------------------------------------
## Include the Parent Makefile
## -----------------------------------------------------------------------
include $(PARENT_MAKEFILE)
