/*
 * Mac OS X Configuration File for Release driver
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

#include "Prefix_Common.h"

// Broadcom configurables for Release build configuration

// Configurables applicable to OSX 10_9 and later
#define WLP2P
#define WLP2P_UCODE
#define WLMCHAN
#define WL_BSSCFG_TX_SUPR
#define P2P_IE_OVRD
#define WLMCNX
#define BCMCCMP
#define OPPORTUNISTIC_ROAM
#define WLROAMPROF
#define WL_DYNAMIC_TEMPSENSE
#define WL_ASSOC_RECREATE
#define WLPKTENG
#define WAR4360_UCODE
#define WLTINYDUMP


#define BCMDBG_ASSERT

// Configurables only applicable to OSX 10_9
#if VERSION_MAJOR == 13
#define BCMDBG_DUMP
#define BCMDBG_FATAL
#endif

// Configurables applicable to OSX 10_10
#if VERSION_MAJOR == 14
#define BCMDBG_DUMP
#define BCM_BACKPLANE_TIMEOUT
#endif

// Configurables applicable to OSX 10_10 and later
#if VERSION_MAJOR >= 14
#define MFP
#endif

// Configurables applicable to OSX 10_11 and later
#if VERSION_MAJOR >= 15
#define STATS_REG_RET_FFFFFFFF
#define BCMCONDITIONAL_LOGGING
#define BCMDBG_ERR
#define WLMSG_ASSOC
#define WLMSG_SCAN
#define WLMSG_DFS
#define WLMSG_PS
#define WLMSG_INFORM
#define WL_EXPORT_GET_PHYREG

#define WOWL_OS_OFFLOADS
#define ENABLE_MBUF_PANICPARANOIDCHECK
#define RXIV_DYNAMIC
#define VALIDATE_IE_SIZE
#define WL_CHANSWITCH_REASON
#endif
