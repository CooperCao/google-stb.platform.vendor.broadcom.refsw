/*
 * Mac OS X Configuration File for Debug MFG driver
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

// Broadcom configurables for Debug_Mfg build configuration

// Configurables applicable to OSX 10_9 and later
#define DBG
#define BCMDBG
#define DBG_BCN_LOSS
#define SR_DEBUG
#define WL_LOCK_CHECK
#define WLOTA_EN
#define WLTEST

// Configurables applicable to OSX 10_9
#if VERSION_MAJOR == 13
#define P2PO
#define ANQPO_DISABLED
#define WL_EVENTQ
#define BCM_DECODE_NO_ANQP
#define BCM_DECODE_NO_HOTSPOT_ANQP
#define WLWFDS
#endif

// Configurables applicable to OSX 10_11 and later
#if VERSION_MAJOR >= 15
#define ASSERT_WARN_ENABLE
#define WOWL_OS_OFFLOADS
#endif
