/*
 * Mac OS X Configuration File for driver in common.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#include <libkern/version.h>

// Broadcom configurables
/////////////////////////////////////////////////////////////////////////////////////////
// For all the driver build configurations: Debug, Release, Debug_Mfg, Release_Mfg

// Configurables applicable to OSX 10_9 and later
#define SPINWAIT_POLL_PERIOD 20
#define MACOSX
#define BCMBUILD
#define BCMDRIVER
#define DMA
#define BCMDMA64
#define BCMDMA64OSL
#define BCMDMASGLISTOSL
#define BCMPMU
#define STA
#define AP
#define WLCNT
#define WL_MONITOR
#define WL11AC
#define WLATF
#define WL11N
#define WL11H
#define WLTPC
#define WLCSA
#define WLQUIET
#define WL11D
#define WLCNTRY
#define DBAND
#define BCMSUP_PSK
#define BCMINTSUP
#define WLAFTERBURNER
#define CRAM
#define WME
#define WLWSEC
#define WLAMPDU
#define WLAMPDU_MAC
#define WLAMPDU_HW
#define WLAMSDU
#define WLOLPC
#define WLAMSDU_SWDEAGG
#define __MBUF_TRANSITION_
#define WOWL
#define WLSCANCACHE
#define WLEXTLOG
#define WL_BSSLISTSORT
#define BCMNVRAMW
#define APSTA
#define PHYCAL_CACHING
#define SAMPLE_COLLECT
#define WIFI_ACT_FRAME
#define APPLEIOCIE
#define WLTXMONITOR
#define IO80211P2P
#define WLP2P_NEW_WFA_OUI
#define BCMECICOEX
#define PPR_API
#define ENABLE_ACPHY
#define SCBFREELIST
#define WME_PER_AC_TUNING
#define WME_PER_AC_TX_PARAMS
#define SURVIVE_PERST_ENAB
#define WL_BEAMFORMING
#define WL_SARLIMIT
#define WL_LTR
#define WL_ANTGAIN
#define WL_EXPORT_CURPOWER
#define PWRTHROTTLE_GPIO
#define SR_ESSENTIALS
#define SAVERESTORE
#define SRFAST
#define DMA_MRENAB
#define DISABLE_INVALIDCC_ADOPT

// Configurables only applicable to OSX 10_9
#if VERSION_MAJOR == 13
#define SCAN_JOIN_TIMEOUT
#endif

// Configurables only applicable to OSX 10_10
#if VERSION_MAJOR == 14
#define AWDL_FAMILY
#define SCB_MEMDBG
#endif

// Configurables applicable to OSX 10_10 and later
#if VERSION_MAJOR >= 14
#define BCMDBG_FATAL
#define WLBTCPROF
#define WLCNTSCB
#endif

// Configurables applicable to OSX 10_11 and later
#if VERSION_MAJOR >= 15
#define BCMDBG_DUMP
#define DISABLE_PCIE2_AXI_TIMEOUT
#define CCA_STATS
#define CCA_STATS_EXT
#define CCA_STATS_WITH_ROAM
#define PCIE_AER_EVT
#define ENABLE_CORECAPTURE
#define ENABLE_LOADDRIVER_CHECKBOARDID
#define DISABLE_MCS32_IN_40MHZ
#define WL_EXCESS_PMWAKE
#define WL_CONNECTION_STATS
#define WL_PWRSTATS
#define WLC_BCMDMA_ERRORS
#define WLC_MQERROR_DBG
#define WL_EVDATA_BSSINFO
#define CNTRY_DEFAULT
#define LOCALE_PRIORITIZATION_2G
#define WL_EIRP_OFF
#define P2PO_DISABLED
#define NEED_HARD_RESET
#define BCM_BACKPLANE_TIMEOUT
#define P2PO
#define ANQPO_DISABLED
#define WL_EVENTQ
#define BCM_DECODE_NO_ANQP
#define BCM_DECODE_NO_HOTSPOT_ANQP
#define WLWFDS
#define WLTXPWR_CACHE
#define WLTXPWR_CACHE_PHY_ONLY
#define SROM12
#define WLPM_BCNRX
#define NEW_PCB_FN_REGISTER
#define ACMAJORREV2_THROUGHPUT_OPT
#define WL_SRENABLE_OVERRIDE
#define SCAN_LINKDOWN_RECOV
#define NO_ROM_COMPAT
#define ECOUNTERS
#define WL_DATAPATH_HC
#define WL_RX_DMA_STALL_CHECK
#define WL_TX_STALL
#define WLC_TXSTALL_FULL_STATS
#define WL_TXQ_STALL
#define DWL_RX_STALL
#define WLC_DEBUG_CRASH
#define WLC_DUMP_ALL_MAC_REGS
#define PCI_ACCESS_HISTORY
#define WL_OLPC_IOVARS_ENAB 1
#endif /* VERSION_MAJOR >= 15 */
