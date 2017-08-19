/*
 * Mac OS X Configuration File for Release MFG driver
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

// Broadcom configurables for Release_Mfg build configuration

// Configurables applicable to OSX 10_9 and later
#define DBG
#define WL_LOCK_CHECK
#define WLOTA_EN
#define WLTEST

// Configurables only applicable to OSX 10_9
#if VERSION_MAJOR == 13
#define OPPORTUNISTIC_ROAM
#define P2PO
#define ANQPO_DISABLED
#define WL_EVENTQ
#define BCM_DECODE_NO_ANQP
#define BCM_DECODE_NO_HOTSPOT_ANQP
#define WLWFDS
#endif

// Configurables applicable to OSX 10_10
#if VERSION_MAJOR == 14
#define BCMDBG_DUMP
#endif

// Configurables applicable to OSX 10_11 and later
#if VERSION_MAJOR >= 15
#define WOWL_OS_OFFLOADS
#define BCMCONDITIONAL_LOGGING
#define BCMDBG_ERR
#define WLMSG_PS
#define WLMSG_ASSOC
#define WLMSG_SCAN
#define WLMSG_INFORM
#define WLMSG_DFS
#endif
