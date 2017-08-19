#
# This brand makefile is used to build the Host Drivers, Firmwares, Apps and HSL
# for the BRAND linux-external-stbsoc
#
# $Id: $
#

PARENT_MAKEFILE := linux-stbsoc.mk

BRAND       := linux-external-stbsoc

## -----------------------------------------------------------------------
## The Brand specific filelist DEFS and UNDEFS
## -----------------------------------------------------------------------
STBSOC_BRAND_SRCFILELIST_DEFS   :=
STBSOC_BRAND_SRCFILELIST_UNDEFS := BCMINTERNAL BCMEXTCCX WLTEST

## -----------------------------------------------------------------------
## The Brand specific Mogrification DEFS and UNDEFS
## -----------------------------------------------------------------------
DRIVER_MOGRIFY_DEFS    := linux
# DRIVER_MOGRIFY_DEFS    := BCMDONGLEHOST DHD_GPL linux LINUX OSLREGOPS BCMNVRAMR

DRIVER_MOGRIFY_UNDEFS  := \
	_WIN32 _WIN64 WIN7 MACOSX _MACOSX_ __ECOS __NetBSD__ vxworks __vxworks \
	TARGETOS_nucleus DOS PCBIOS NDIS _CFE_ _MINOSL_ UNDER_CE \
	BCMDBG_TRAP BCMDBG_PKT BCMDBG_MEM \
	CONFIG_PCMCIA_MODULE BCMINTERNAL _HNDRTE_ EFI \
	_MSC_VER MFGTEST ILSIM BCMPERFSTATS BCMTSTAMPEDLOGS DSLCPE_DELAY \
	DONGLEBUILD BCMROMOFFLOAD BCMROMBUILD WLLMAC OLYMPIC_RWL \
	__BOB__ BCMWAPI_WPI BCMWAPI_WAI CONFIG_XIP NDIS60 NDISVER WLTEST DHD_SPROM \
	BCMJTAG NOTYET BINOSL USER_MODE BCMHND74K \
	APSTA_PINGTEST SI_SPROM_PROBE SI_ENUM_BASE_VARIABLE \
	EXT_STA BCMSPI USBAP WL_NVRAM_FILE \
	BCM_BOOTLOADER BCMNVRAMW WLNOKIA BCMSDIO SDTEST \
	USE_ETHER_BPF_MTAP
DRIVER_MOGRIFY_UNDEFS  += \
	_RTE_ WL_LOCATOR ATE_BUILD PATCH_SDPCMDEV_STRUCT WLC_PATCH \
	SERDOWNLOAD CTFMAP PKTCTFMAP DHD_AWDL WLAWDL

DRIVER_MOGRIFY_UNDEFS  += \
	CUSTOMER_HW4 WLMEDIA_CUSTOMER_1 UNRELEASEDCHIP

DHD_MOGRIFY_DEFS       :=
DHD_MOGRIFY_UNDEFS     := BCMEXTCCX
HSL_MOGRIFY_DEFS       := BCMCRYPTO LINUX SUPP BCMWPS DHCPD_APPS P2P BWL WFI PRE_SECFRW
HSL_MOGRIFY_UNDEFS     := ROUTER
# HSL_MOGRIFY_UNDEFS  += BCMWAPI_WAI BCMWAPI_WPI

STBSOC_BRAND_MOGRIFY_DEFS     := $(DRIVER_MOGRIFY_DEFS) $(DHD_MOGRIFY_DEFS) $(HSL_MOGRIFY_DEFS)
STBSOC_BRAND_MOGRIFY_UNDEFS   := $(DRIVER_MOGRIFY_UNDEFS) $(DHD_MOGRIFY_UNDEFS) $(HSL_MOGRIFY_UNDEFS)

## -----------------------------------------------------------------------
## The Driver Types and Platform/OS specific definitions for each <oem>
## -----------------------------------------------------------------------
# Release Source Packages for each <oem> (eg: dhd, wl, bcmdbus)
RLS_SRC_PKG_bcm   := wl
# RLS_SRC_PKG_bcm += bcmdbus

# Build sanity to be verified for the following Release Source Packages
RLS_SRC_PKG_VERIFY_DRV_bcm   := wl
RLS_SRC_PKG_VERIFY_APPS_bcm  := wl

# Mogrify flags for each Release Source Package for each <oem>

RLS_SRC_PKG_MOGRIFY_DEFS_bcm_wl    :=
RLS_SRC_PKG_MOGRIFY_UNDEFS_bcm_wl  := WLNOKIA
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
	WLBTAMP QMONITOR SIMPLE_ISCAN BCM_BUZZZ RWL_WIFI RWL_DONGLE RWL_SOCKET \
	RWL_SERIAL DHDTCPACK_SUPPRESS BCMPCIE_OOB_HOST_WAKE CONFIG_MACH_UNIVERSAL5433 \
	BCM_FD_AGGR SHOW_LOGTRACE SHOW_EVENTS MCAST_LIST_ACCUMULATION WLFBT WLAIBSS \
	CUSTOM_PLATFORM_NV_TEGRA WLC_HIGH WLC_HIGH_ONLY CONFIG_PCMCIA BCMPCMCIA \
	ARGOS_CPU_SCHEDULER ARGOS_RPS_CPU_CTL WL_ACSD DHD_BUZZZ_LOG_ENABLED DHD_ULP \
	BCM_OBJECT_TRACE CONFIG_MACH_UNIVERSAL7420 CONFIG_SOC_EXYNOS8890 DHD_DHCP_DUMP \
	DETAIL_DEBUG_LOG_FOR_IOCTL DHD_DEBUG_PAGEALLOC DHD_DEBUG_SCAN_WAKELOCK DHD_IFDEBUG \
	DHD_FW_COREDUMP DHD_LB DHD_LB_RXC DHD_LB_RXP DHD_LB_TXC DHD_LB_STATS WL_NAN_DEBUG \
	WL11ULB USE_EXYNOS_PCIE_RC_PMPATCH NOT_YET notdef CUSTOMER_HW4_DEBUG
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
RLS_SRC_PKG_FILELIST_bcm_dhd        := src/tools/release/linux-dhd-stbsoc-filelist.txt
RLS_SRC_PKG_FILELIST_DEFS_bcm_dhd   := P2P_SRC BCMWPS_SRC DHCPD_APPS_SRC
RLS_SRC_PKG_FILELIST_UNDEFS_bcm_dhd :=

# Filelist for the BRCM generic DHD Open Source Package -->'dhd-opensource' (for oem = bcm)
RLS_SRC_PKG_FILELIST_bcm_dhd-opensource        := src/tools/release/linux-dhd-stbsoc-opensource-filelist.txt
RLS_SRC_PKG_FILELIST_DEFS_bcm_dhd-opensource   :=
RLS_SRC_PKG_FILELIST_UNDEFS_bcm_dhd-opensource :=

RLS_SRC_PKG_FILELIST_bcm_wl         := src/tools/release/linux-stbsoc-filelist.txt \
		src/tools/release/components/wlphy-filelist.txt \
		src/tools/release/linux-wlexe-filelist.txt
RLS_SRC_PKG_FILELIST_DEFS_bcm_wl    := \
	AP_SRC BCMASSERT_LOG_SRC BCMAUTH_PSK_SRC BCMSUP_PSK_SRC \
	BCMUTILS_SRC OSLLX_SRC WL_SRC WLAMPDU_SRC WLAMSDU_SRC WLC_SPLIT_SRC \
	WLDPT_SRC WLEXTLOG_SRC WLLED_SRC WLLX_SRC WLP2P_SRC WOWL_SRC WLSRC_SRC \
	LINUX_SRC BCMWPS_SRC P2P_SRC WL11U_SRC BCMCCX_SRC \
	L2_FILTER_SRC WLPROBRESP_SW_SRC WET_SRC BCMNVRAMR_SRC DHCPD_APPS_SRC
RLS_SRC_PKG_FILELIST_UNDEFS_bcm_wl  :=

RLS_SRC_PKG_FILELIST_bcm_bcmdbus         := src/tools/release/linux-stbsoc-dbusmod-filelist.txt
RLS_SRC_PKG_FILELIST_DEFS_bcm_bcmdbus    :=
RLS_SRC_PKG_FILELIST_UNDEFS_bcm_bcmdbus  :=

RLS_SRC_PKG_FILELIST_bcm_wlexe         := src/tools/release/linux-wlexe-filelist.txt
RLS_SRC_PKG_FILELIST_DEFS_bcm_wlexe    := WLBTAMP
RLS_SRC_PKG_FILELIST_UNDEFS_bcm_wlexe  :=

## -----------------------------------------------------------------------
## List of Host Driver images to build for each <oem>
## -----------------------------------------------------------------------
# The Operating System of the Host Driver Builds
HOST_DRV_OS_bcm  := stb7271armlx4118 stb7271armlx411864bit stb7271armlx41110 stb7271armlx4111064bit

HOST_DRV_TYPE_bcm  := wl

# Build sanity of Source Packages to be verified on following OS/platform
HOST_OS_PKG_VERIFY_bcm   ?= stb7271armlx41110

RLS_HOST_DRIVERS_bcm_wl_stb7271armlx41110 := \
	nodebug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv7l \
	debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv7l \
	nodebug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv7l \
	debug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7271armlx4111064bit := \
	nodebug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv8 \
	debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv8 \
	nodebug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv8 \
	debug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv8
	
RLS_HOST_DRIVERS_bcm_wl_stb7271armlx4118 := \
	nodebug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv7l \
	debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv7l \
	nodebug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv7l \
	debug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv7l \
	debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-android-stbsoc-armv7l

RLS_HOST_DRIVERS_bcm_wl_stb7271armlx411864bit := \
	nodebug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv8 \
	debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv8 \
	nodebug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv8 \
	debug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv8 \
	debug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-android-stbsoc-armv8

RLS_HOST_DRIVERS_PKG_bcm_wl_stb7271armlx4112 := \
	nodebug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv7l \
	nodebug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-stbsoc-armv7l \
	nodebug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-stbsoc-armv7l \
	nodebug-apdef-stadef-extnvm-stbsoc-armv7l

RLS_HOST_DRIVERS_PKG_bcm_wl_stb7271armlx411264bit := \
	nodebug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv8 \
	nodebug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-wowl-stbsoc-armv8 \
	nodebug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv8 \
	nodebug-apdef-stadef-extnvm-stbsoc-armv8

RLS_HOST_DRIVERS_PKG_bcm_wl_stb7271armlx41110 := \
	nodebug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv7l \
	nodebug-apdef-stadef-extnvm-stbsoc-armv7l

RLS_HOST_DRIVERS_PKG_bcm_wl_stb7271armlx4111064bit := \
	nodebug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv8 \
	nodebug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-wowl-stbsoc-armv8 \
	nodebug-apdef-stadef-extnvm-p2p-mchan-tdls-mfp-cfg80211-wowl-stbsoc-armv8 \
	nodebug-apdef-stadef-extnvm-stbsoc-armv8


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
