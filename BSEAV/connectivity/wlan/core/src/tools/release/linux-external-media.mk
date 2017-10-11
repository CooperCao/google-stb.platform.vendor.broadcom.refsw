#
# This brand makefile is used to build the Host Drivers, Firmwares, Apps and HSL
# for the BRAND linux-external-media
#
# $Id$
#

PARENT_MAKEFILE := linux-media.mk

BRAND       := linux-external-media

## -----------------------------------------------------------------------
## The Brand specific filelist DEFS and UNDEFS
## -----------------------------------------------------------------------
MEDIA_BRAND_SRCFILELIST_DEFS   :=
MEDIA_BRAND_SRCFILELIST_UNDEFS := BCMINTERNAL BCMCCX BCMEXTCCX WLTEST

## -----------------------------------------------------------------------
## The Brand specific Mogrification DEFS and UNDEFS
## -----------------------------------------------------------------------
DRIVER_MOGRIFY_DEFS    := linux
# DRIVER_MOGRIFY_DEFS    := BCMDONGLEHOST DHD_GPL linux LINUX OSLREGOPS BCMNVRAMR

DRIVER_MOGRIFY_UNDEFS  := \
	_WIN32 _WIN64 WIN7 MACOSX _MACOSX_ __ECOS __NetBSD__ vxworks __vxworks \
	TARGETOS_nucleus DOS PCBIOS NDIS _CFE_ _MINOSL_ UNDER_CE \
	BCMDBG BCMDBG_ERR BCMDBG_ASSERT BCMASSERT_LOG BCMDBG_DUMP BCMDBG_TRAP BCMDBG_PKT BCMDBG_MEM \
	CONFIG_PCMCIA_MODULE BCMINTERNAL _HNDRTE_ EFI \
	_MSC_VER MFGTEST ILSIM BCMPERFSTATS BCMTSTAMPEDLOGS DSLCPE_DELAY \
	DONGLEBUILD BCMROMOFFLOAD BCMROMBUILD WLLMAC OLYMPIC_RWL \
	__BOB__ BCMWAPI_WPI BCMWAPI_WAI CONFIG_XIP NDIS60 NDISVER WLTEST DHD_SPROM \
	BCMJTAG NOTYET BCMECICOEX BINOSL USER_MODE BCMHND74K \
	APSTA_PINGTEST SI_SPROM_PROBE SI_ENUM_BASE_VARIABLE \
	BCMASSERT_SUPPORT EXT_STA BCMSPI USBAP WL_NVRAM_FILE \
	BCM_BOOTLOADER BCMNVRAMW WLNOKIA BCMSDIO SDTEST \
	USE_ETHER_BPF_MTAP
DRIVER_MOGRIFY_UNDEFS  += \
	_RTE_ WL_LOCATOR ATE_BUILD PATCH_SDPCMDEV_STRUCT WLC_PATCH \
	SERDOWNLOAD CTFMAP PKTCTFMAP DHD_AWDL WLAWDL

DRIVER_MOGRIFY_UNDEFS  += \
	CUSTOMER_HW4 WLMEDIA_CUSTOMER_1 UNRELEASEDCHIP

DHD_MOGRIFY_DEFS       :=
DHD_MOGRIFY_UNDEFS     := BCMCCX BCMEXTCCX
HSL_MOGRIFY_DEFS       := BCMCRYPTO LINUX SUPP BCMWPS DHCPD_APPS P2P BWL WFI PRE_SECFRW
HSL_MOGRIFY_UNDEFS     := ROUTER
# HSL_MOGRIFY_UNDEFS  += BCMWAPI_WAI BCMWAPI_WPI

MEDIA_BRAND_MOGRIFY_DEFS     := $(DRIVER_MOGRIFY_DEFS) $(DHD_MOGRIFY_DEFS) $(HSL_MOGRIFY_DEFS)
MEDIA_BRAND_MOGRIFY_UNDEFS   := $(DRIVER_MOGRIFY_UNDEFS) $(DHD_MOGRIFY_UNDEFS) $(HSL_MOGRIFY_UNDEFS)

## -----------------------------------------------------------------------
## The Driver Types and Platform/OS specific definitions for each <oem>
## -----------------------------------------------------------------------
# Release Source Packages for each <oem> (eg: dhd, wl, bcmdbus)
RLS_SRC_PKG_bcm   := dhd wl dhd-opensource
# RLS_SRC_PKG_bcm += bcmdbus

# Build sanity to be verified for the following Release Source Packages
RLS_SRC_PKG_VERIFY_DRV_bcm   := dhd wl
RLS_SRC_PKG_VERIFY_APPS_bcm  := dhd wl

# Mogrify flags for each Release Source Package for each <oem>

RLS_SRC_PKG_MOGRIFY_DEFS_bcm_wl    :=
RLS_SRC_PKG_MOGRIFY_UNDEFS_bcm_wl  := OEM_ANDROID
RLS_SRC_PKG_MOGRIFY_FLAGS_bcm_wl   :=

RLS_SRC_PKG_MOGRIFY_DEFS_bcm_dhd    := BCMDONGLEHOST DHD_GPL
RLS_SRC_PKG_MOGRIFY_UNDEFS_bcm_dhd  := WLC_LOW WLC_HIGH BCM_DNGL_EMBEDIMAGE CONFIG_PCMCIA BCMPCMCIA SERDOWNLOAD
RLS_SRC_PKG_MOGRIFY_FLAGS_bcm_dhd   :=

# Mogrify flags for the BRCM generic DHD Open Source Package -->'dhd-opensource' (for oem = bcm)
RLS_SRC_PKG_MOGRIFY_DEFS_bcm_dhd-opensource    := LINUX linux \
	LINUX_POSTMOGRIFY_REMOVAL BCMDONGLEHOST NO_BCMDBG_ASSERT
RLS_SRC_PKG_MOGRIFY_UNDEFS_bcm_dhd-opensource  := \
	BCMDBG_CTRACE BCMDBG_POOL BCM4329C0 EHCI_FASTPATH_TX EHCI_FASTPATH_RX \
	BCM_DNGL_EMBEDIMAGE BCMASSERT_LOG BCM47XX_CA9 CTFPOOL UNRELEASEDCHIP \
	CTFMAP CTFPOOL_SPINLOCK BCM_GMAC3 DBUS_LINUX_RXDPC DBUS_LINUX_HIST \
	USB_TRIGGER_DEBUG BCMLFRAG CONFIG_USBRNDIS_RETAIL NDIS_MINIPORT_DRIVER \
	DHD_DEBUG WLC_LOW BCM_MWBMAP_DEBUG PKTQ_LOG BCMFRAGPOOL ATE_BUILD \
	BCMLFRAG BCMSPLITRX BCM_SPLITBUF DHD_DEBUG_WAKE_LOCK DHD_DEBUG \
	DEBUG_COUNTER DHDTCPACK_SUP_DBG DEBUG_CPU_FREQ DEBUGGER \
	PROP_TXSTATUS_DEBUG OOO_DEBUG SR_DEBUG DEBUGFS_CFG80211 \
	QMONITOR SIMPLE_ISCAN BCM_BUZZZ RWL_WIFI RWL_DONGLE RWL_SOCKET \
	RWL_SERIAL DHDTCPACK_SUPPRESS BCMPCIE_OOB_HOST_WAKE CONFIG_MACH_UNIVERSAL5433 \
	BCM_FD_AGGR SHOW_LOGTRACE SHOW_EVENTS MCAST_LIST_ACCUMULATION WLFBT WLAIBSS \
	CUSTOM_PLATFORM_NV_TEGRA WLC_HIGH WLC_HIGH_ONLY CONFIG_PCMCIA BCMPCMCIA \
	ARGOS_CPU_SCHEDULER ARGOS_RPS_CPU_CTL WL_ACSD DHD_BUZZZ_LOG_ENABLED DHD_ULP \
	BCM_OBJECT_TRACE CONFIG_MACH_UNIVERSAL7420 CONFIG_SOC_EXYNOS8890 DHD_DHCP_DUMP \
	DETAIL_DEBUG_LOG_FOR_IOCTL DHD_DEBUG_PAGEALLOC DHD_DEBUG_SCAN_WAKELOCK DHD_IFDEBUG \
	DHD_FW_COREDUMP DHD_LB DHD_LB_RXC DHD_LB_RXP DHD_LB_TXC DHD_LB_STATS WL_NAN_DEBUG \
	USE_EXYNOS_PCIE_RC_PMPATCH NOT_YET notdef CUSTOMER_HW4_DEBUG
RLS_SRC_PKG_MOGRIFY_UNDEFS_bcm_dhd-opensource  += \
	CUSTOMER_HW_31_1 CUSTOMER_HW_32_1 CUSTOMER_HW_33_1 CUSTOMER_HW_31_2
RLS_SRC_PKG_MOGRIFY_FLAGS_bcm_dhd-opensource   := -strip_bcmromfn

RLS_SRC_PKG_MOGRIFY_DEFS_bcm_bcmdbus   :=
RLS_SRC_PKG_MOGRIFY_UNDEFS_bcm_bcmdbus :=
RLS_SRC_PKG_MOGRIFY_FLAGS_bcm_bcmdbus  := -translate_open_to_dual_copyright

RLS_SRC_PKG_MOGRIFY_DEFS_bcm_wlexe    := LINUX_POSTMOGRIFY_REMOVAL
RLS_SRC_PKG_MOGRIFY_UNDEFS_bcm_wlexe  := NL80211 WLAWDL WLAWDL_LATENCY_SUPPORT
RLS_SRC_PKG_MOGRIFY_FLAGS_bcm_wlexe   :=

# Filelist for each Release Source Package for each <oem>
RLS_SRC_PKG_FILELIST_bcm_dhd        := src/tools/release/linux-dhd-media-filelist.txt
RLS_SRC_PKG_FILELIST_DEFS_bcm_dhd   := P2P_SRC BCMWPS_SRC DHCPD_APPS_SRC
RLS_SRC_PKG_FILELIST_UNDEFS_bcm_dhd :=

# Filelist for the BRCM generic DHD Open Source Package -->'dhd-opensource' (for oem = bcm)
RLS_SRC_PKG_FILELIST_bcm_dhd-opensource        := src/tools/release/linux-dhd-media-opensource-filelist.txt
RLS_SRC_PKG_FILELIST_DEFS_bcm_dhd-opensource   :=
RLS_SRC_PKG_FILELIST_UNDEFS_bcm_dhd-opensource :=

RLS_SRC_PKG_FILELIST_bcm_wl         := src/tools/release/linux-media-filelist.txt \
		src/tools/release/components/wlphy-filelist.txt \
		src/tools/release/components/p2p-filelist.txt \
		src/tools/release/linux-p2plib-filelist.txt \
		src/tools/release/components/wps-filelist.txt \
		src/tools/release/linux-wlexe-filelist.txt
RLS_SRC_PKG_FILELIST_DEFS_bcm_wl    := \
	AP_SRC BCMASSERT_LOG_SRC BCMAUTH_PSK_SRC BCMDBUS_SRC BCMSUP_PSK_SRC \
	BCMUTILS_SRC OSLLX_SRC WL_SRC WLAMPDU_SRC WLAMSDU_SRC WLC_SPLIT_SRC \
	WLDPT_SRC WLEXTLOG_SRC WLLED_SRC WLLX_SRC WLP2P_SRC WOWL_SRC WLSRC_SRC \
	LINUX_SRC BCMWPS_SRC WL_FW_DECOMP_SRC P2P_SRC WL11U_SRC \
	L2_FILTER_SRC WLPROBRESP_SW_SRC WET_SRC
RLS_SRC_PKG_FILELIST_UNDEFS_bcm_wl  :=

RLS_SRC_PKG_FILELIST_bcm_bcmdbus         := src/tools/release/linux-media-dbusmod-filelist.txt
RLS_SRC_PKG_FILELIST_DEFS_bcm_bcmdbus    :=
RLS_SRC_PKG_FILELIST_UNDEFS_bcm_bcmdbus  :=

RLS_SRC_PKG_FILELIST_bcm_wlexe         := src/tools/release/linux-wlexe-filelist.txt
RLS_SRC_PKG_FILELIST_UNDEFS_bcm_wlexe  :=

## -----------------------------------------------------------------------
## List of Host Driver images to build for each <oem>
## -----------------------------------------------------------------------
# The Operating System of the Host Driver Builds
HOST_DRV_OS_bcm  := fc19x64 fc22x64 brix-and-422 stb7425mipslx33 \
		stb7445armlx31419 stb7252armlx314111 stb7252armlx4110

HOST_DRV_TYPE_bcm  := dhd wl

# Build sanity of Source Packages to be verified on following OS/platform
HOST_OS_PKG_VERIFY_bcm    ?= fc19x64

RLS_HOST_DRIVERS_bcm_dhd_fc19x64   := \
	dhd-cdc-usb-gpl-media \
	dhd-cdc-usb-comp-gpl-media \
	dhd-cdc-usb-reqfw-comp-gpl-media \
	dhd-msgbuf-pciefd-media

RLS_HOST_DRIVERS_bcm_dhd_fc22x64   := \
	dhd-msgbuf-pciefd

RLS_HOST_DRIVERS_bcm_dhd_brix-and-422   := \
	dhd-cdc-usb-android-media-cfg80211-noproptxstatus \
	dhd-cdc-usb-android-media-cfg80211-comp \
	dhd-cdc-usb-android-media-cfg80211-comp-mfp \
	dhd-msgbuf-pciefd-android-media-cfg80211-mfp \
	dhd-cdc-usb-android-media-cfg80211

RLS_HOST_DRIVERS_bcm_dhd_stb7425mipslx33 := \
	dhd-cdc-usb-comp-gpl-mips \
	dhd-cdc-usb-reqfw-comp-gpl-mips \
	dhd-cdc-usb-android-media-cfg80211-comp-mips \
	dhd-cdc-usb-android-media-cfg80211-comp-mfp-mips \
	dhd-cdc-usb-android-media-cfg80211-noproptxstatus-mips \
	dhd-cdc-usb-reqfw-android-media-cfg80211-comp-mips \
	dhd-cdc-usb-reqfw-android-media-cfg80211-comp-mfp-mips \
	dhd-msgbuf-pciefd-mfp-mips \
	dhd-msgbuf-pciefd-android-media-cfg80211-mfp-mips

RLS_HOST_DRIVERS_bcm_dhd_stb7252armlx314111 := \
	dhd-cdc-usb-comp-gpl-armv7l \
	dhd-cdc-usb-reqfw-comp-gpl-armv7l \
	dhd-cdc-usb-android-media-cfg80211-comp-mfp-armv7l \
	dhd-cdc-usb-android-media-cfg80211-noproptxstatus-armv7l \
	dhd-cdc-usb-reqfw-android-media-cfg80211-comp-mfp-armv7l \
	dhd-msgbuf-pciefd-mfp-armv7l \
	dhd-msgbuf-pciefd-mfp-secdma-armv7l \
	dhd-msgbuf-pciefd-mfp-stbap-armv7l \
	dhd-msgbuf-pciefd-mfp-secdma-stbap-armv7l \
	dhd-msgbuf-pciefd-android-media-cfg80211-mfp-armv7l \
	dhd-msgbuf-pciefd-android-media-cfg80211-mfp-secdma-armv7l

RLS_HOST_DRIVERS_bcm_dhd_stb7252armlx4110 := \
	dhd-cdc-usb-comp-gpl-armv7l \
	dhd-cdc-usb-reqfw-comp-gpl-armv7l \
	dhd-cdc-usb-android-media-cfg80211-comp-mfp-armv7l \
	dhd-cdc-usb-android-media-cfg80211-noproptxstatus-armv7l \
	dhd-cdc-usb-reqfw-android-media-cfg80211-comp-mfp-armv7l \
	dhd-msgbuf-pciefd-mfp-armv7l \
	dhd-msgbuf-pciefd-mfp-secdma-armv7l \
	dhd-msgbuf-pciefd-mfp-stbap-armv7l \
	dhd-msgbuf-pciefd-mfp-secdma-stbap-armv7l \
	dhd-msgbuf-pciefd-android-media-cfg80211-mfp-armv7l \
	dhd-msgbuf-pciefd-android-media-cfg80211-mfp-secdma-armv7l

RLS_HOST_DRIVERS_bcm_wl_fc19x64 := \
	nodebug-apdef-stadef \
	nodebug-apdef-stadef-slvradar

RLS_HOST_DRIVERS_bcm_wl_fc22x64 := \
	nodebug-apdef-stadef-p2p-mchan-tdls-wowl-mfp

RLS_HOST_DRIVERS_bcm_wl_stb7445armlx31419 := \
	nodebug-apdef-stadef-stb-armv7l \
	nodebug-apdef-stadef-extnvm-stb-armv7l \
	nodebug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-armv7l \
	nodebug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-slvradar-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7252armlx314111 := \
	nodebug-apdef-stadef-stb-armv7l \
	nodebug-apdef-stadef-extnvm-stb-armv7l \
	nodebug-apdef-stadef-extnvm-mfp-stb-stbap-armv7l \
	nodebug-apdef-stadef-p2p-mchan-tdls-wowl-extnvm-stb-armv7l \
	nodebug-apdef-stadef-p2p-mchan-tdls-wowl-extnvm-mfp-stb-armv7l \
	nodebug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-wet-slvradar-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7252armlx4110 := \
	nodebug-apdef-stadef-stb-armv7l \
	nodebug-apdef-stadef-extnvm-stb-armv7l \
	nodebug-apdef-stadef-extnvm-mfp-stb-stbap-armv7l \
	nodebug-apdef-stadef-p2p-mchan-tdls-wowl-extnvm-stb-armv7l \
	nodebug-apdef-stadef-p2p-mchan-tdls-wowl-extnvm-mfp-stb-armv7l \
	nodebug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-armv7l \
	nodebug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-slvradar-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7425mipslx33 := \
	nodebug-apdef-stadef-stb-mips \
	nodebug-apdef-stadef-extnvm-stb-mips \
	nodebug-apdef-stadef-p2p-mchan-tdls-wowl-extnvm-stb-mips \
	nodebug-apdef-stadef-p2p-mchan-tdls-wowl-extnvm-mfp-stb-mips

RLS_HOST_DRIVERS_PKG_bcm_wl_fc19x64 := \
	nodebug-apdef-stadef

RLS_HOST_DRIVERS_PKG_bcm_dhd_fc19x64 := \
	dhd-cdc-usb-reqfw-comp-gpl \
	dhd-msgbuf-pciefd

## -----------------------------------------------------------------------
## List of Dongle Firmware builds needed for each <oem>
## -----------------------------------------------------------------------
RLS_FIRMWARES_USB_FD_bcm  := \
	43569a2-ram/usb-ag-pool \
	43569a2-ram/usb-ag-pool-tdls \
	43569a2-ram/usb-ag-pool-p2p-mchan

RLS_FIRMWARES_PCIE_FD_bcm  := \
	43602a1-ram/pcie-ag-splitrx \
	43602a1-ram/pcie-ag-splitrx-clm_min \
	43602a1-ram/pcie-ag-err-splitrx \
	43602a1-ram/pcie-ag-err-splitrx-clm_min \
	43602a1-ram/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop

RLS_FIRMWARES_bcm  := $(RLS_FIRMWARES_USB_FD_bcm) $(RLS_FIRMWARES_PCIE_FD_bcm)

# List FW mages to embed into the Host driver for each <oem> (if BCMEMBEDIMAGE is ON)
EMBED_FIRMWARES_bcm  :=

# Define ALL_DNGL_IMAGES here so that 'hndrte-dongle-wl' build will pickup from here
ALL_DNGL_IMAGES    := $(RLS_FIRMWARES_bcm) $(EMBED_FIRMWARES_bcm)


## -----------------------------------------------------------------------
## Include the Parent Makefile
## -----------------------------------------------------------------------
include $(PARENT_MAKEFILE)
