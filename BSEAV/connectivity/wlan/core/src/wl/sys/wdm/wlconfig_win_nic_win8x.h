/*
 * Header file used by Visual Studio Windows8.x projects
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
#define NDIS
#define NDIS_MINIPORT_DRIVER
#define NDIS_WDM 1
#define WDM
#define WL_PM2_RCV_DUR_LIMIT
#define TRACE_LOGGING
#define DELTASTATS
#define BCMDRIVER
#define NDIS_DMAWAR
#define BCMDBG_TRAP
#define UNRELEASEDCHIP
#define PKTQ_LOG
#define WLWSEC
#define WL11H
#define WLCSA
#define WLQUIET
#define WLTPC
#define WL11D
#define WLCNTRY
#define WLEXTLOG
#define WLSCANCACHE
#define EXT_STA
#define WL_MONITOR
#define WL_NEW_RXSTS
#define IBSS_PEER_GROUP_KEY
#define IBSS_PEER_DISCOVERY_EVENT
#define IBSS_PEER_MGMT
#define AP
#define WLVIF
#define WDS
#define APCS
#define WME_PER_AC_TX_PARAMS
#define WME_PER_AC_TUNING
#define STA
#define WLOVERTHRUSTER
#define WME
#define WL11N
#define DBAND
#define WLRM
#define WLCNT
#define WLCNTSCB
#define WLCOEX
#define BCMSUP_PSK
#define BCMINTSUP
#define WLCAC
#define BCMAUTH_PSK
#define BCMCCMP
#define WIFI_ACT_FRAME
#define BCMDMA32
#define WLAMSDU
#define WLAMSDU_SWDEAGG
#define WLAMPDU
#define WLAMPDU_HW
#define WLAMPDU_MAC
#define WLAMPDU_PRECEDENCE
#define WL_ASSOC_RECREATE
#define WLP2P
#define WL_BSSCFG_TX_SUPR
#define WLP2P_UCODE
#define WLMCNX
#define WLMCHAN
#define BCMNVRAMR
#define BCMNVRAMW
#define WLDIAG
#define WLTINYDUMP
#define BCM_DCS
#define WLPFN
#define NLO
#define MFP
#define IO80211P2P
#define WL11AC
#define PPR_API
#define BCMECICOEX
#define WLATF
#define WPP_TRACING
#define WL_WLC_SHIM
#define WL_WLC_SHIM_EVENTS
/* #define BISON_SHIM_PATCH */
/* #define CARIBOU_SHIM_PATCH */
#ifdef WL_NDWDI
#define WLFBT_STA_OVERRIDES
#define WL_ASSOC_MGR
#define MAXIEREGS 8
#define WL_OSL_ACTION_FRAME_SUPPORT
#define D0_COALESCING
#define WL11U
#define WLFBT
#define WLFBT_OVER_DS_DISABLED
#endif /* WL_NDWDI */

/*
 * For NIC drivers only
 */
#define MEMORY_TAG 'NWMB'
#define WLPIO
#define BCMDMA64OSL
/* These 3 should go together */
#define PKTC
#define PKTC_DONGLE
#define PKTC_TX_DONGLE

#define WLOLPC
#define WOWL
#define TDLS_TESTBED
#define WLTDLS
#define CCA_STATS
#define ISID_STATS
#define TRAFFIC_MGMT
#define TRAFFIC_SHAPING
#define WL_LPC
#define WL_BEAMFORMING
#define GTK_RESET
#define SURVIVE_PERST_ENAB
#define WL_LTR
#define WL_PRE_AC_DELAY_NOISE_INT
#define PHYCAL_CACHING
#define WLWNM
#define WL_EXPORT_AMPDU_RETRY
#define BCMWAPI_WPI
#define BCMWAPI_WAI
#define SAVERESTORE
#define SR_ESSENTIALS
