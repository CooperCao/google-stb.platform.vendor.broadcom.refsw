/*
 * Header file used by Visual Studio Win8.x dhd projects
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

/*
 * For all drivers
 */

#if NTDDI_VERSION <= 0x06010000
#define NDIS620_MINIPORT 1
#define NDIS620
#else
#define D0_COALESCING
#define KEEP_ALIVE
#define PNO_SUPPORT
#define TRACE_LOGGING
#define WLP2P
#if NTDDI_VERSION >= 0x0A000000
#define NDIS650_MINIPORT 1
#define NDIS650
#else
#define NDIS640_MINIPORT 1
#define NDIS640
#endif
#endif

#define MEMORY_TAG 'DWMB'
#define EXT_STA
#define WLNOEIND
#define WL_MONITOR
#define SHOW_EVENTS
#define BCMPERFSTATS
#define BDC
#define BCMDONGLEHOST
#define DELTASTATS
#define STA
#define WME
#define WL11H
#define BCMDMA32
#define BCMCCISSR3
#define WLTINYDUMP
#define NDIS
#define NDIS_MINIPORT_DRIVER
#define NDIS_WDM 1
#define WDM
#define BCMDRIVER
#define EMBED_IMAGE_GENERIC
#define NDIS_DMAWAR
#define BINARY_COMPATIBLE 0
#define DBAND
#define WL11AC
#define WL11N
#define BCMNDIS6
#define MFP
#define AP
#define WLCNT
#define DHD_WD_COALESCABLE
#define BCMWAPI_WAI
#define BCMWAPI_WPI
#define WL_WLC_SHIM
#define WL_WLC_SHIM_EVENTS
#define DHD_FW_COREDUMP
#define WLVIF
#define WLWSEC
/* #define BISON_SHIM_PATCH */
/* #define CARIBOU_SHIM_PATCH */

#if defined(WL_NDWDI)
#define WL11U
#endif /* WL_NDWDI */

/*
 * For SDIO full dongle only
 */

/*
 * For PCIE full dongle only
 */
#if defined(BCMPCIE)
#define PCIE_FULL_DONGLE
#define BCMDMA64OSL
#define CACHE_FW_IMAGES

#if !defined(WL_NDWDI)
/*
 * Define LBFC_STATS to turn on the detailed counting of flow controlled datapath
 * This feature is independent of the actual flow control code
 * which allows the comparison of the baseline briver and the flow controlled path
 *
 * #define LBFC_STATS
 */

/*
 * Flow control feature sets only enabled for PCIE full dogle.
 * Use outside of this scope is depending on available testing for regression 
 */
#define LBFC /* Enable flow control when lbufs are exhausted */
#define CHAINED_COMPLETE /* Return completed pkts to NDIS as a chain, WDI excepted */

#endif /* ! WL_NDWDI */

#endif /* #if defined(BCMPCIE) */

/*
 * For internal/debug builds
 */
