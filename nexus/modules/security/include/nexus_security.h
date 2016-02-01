/***************************************************************************
 *     (c)2013-2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#ifndef NEXUS_SECURITY_H__
#define NEXUS_SECURITY_H__

#include "nexus_core_utils.h"
#include "nexus_security_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/* the following functions are only callable from the server, not from clients. */

/**
Summary:
Get Security Capabilities
**/
void NEXUS_GetSecurityCapabilities(
    NEXUS_SecurityCapabilities *pCaps /* [out] */
    );

/**
Summary:
Deprecated. The application should perform its own keyslot/pidchannel bookkeeping.
**/
NEXUS_KeySlotHandle NEXUS_Security_LocateCaKeySlotAssigned(
    unsigned long pidchannel
    );

/**
Summary:
Deprecated. Use NEXUS_KeySlot_AddPidChannel instead.
**/
NEXUS_Error NEXUS_Security_AddPidChannelToKeySlot(
    NEXUS_KeySlotHandle keyHandle,
    unsigned int pidChannel
    );

/**
Summary:
Deprecated. Use NEXUS_KeySlot_RemovePidChannel instead.
**/
NEXUS_Error NEXUS_Security_RemovePidChannelFromKeySlot(
    NEXUS_KeySlotHandle keyHandle,
    unsigned int pidChannel
    );

/**
Summary:
Get default settings for NEXUS_Security_ConfigMulti2
**/
void NEXUS_Security_GetDefaultMulti2Settings(
    NEXUS_SecurityMulti2Settings *pSettings /* [out] */
    );

/**
Summary:
Configure multi2 on a keyslot.
**/
NEXUS_Error NEXUS_Security_ConfigMulti2(
    NEXUS_KeySlotHandle keyslot,
    const NEXUS_SecurityMulti2Settings *pSettings
    );


/**
Summary:
Get default Global Control Word setting for a key slot. - Zeus 4.1+.
**/
void NEXUS_KeySlot_GetDefaultGlobalControlWordSettings(
	NEXUS_KeySlotGlobalControlWordSettings  *pSettings  /* [out] */
    );

/**
Summary:
Set new Global Control Word for specified key slot. - Zeus 4.1+.
**/
NEXUS_Error NEXUS_KeySlot_SetGlobalControlWordSettings(
    NEXUS_KeySlotHandle                            keyHandle,
    const NEXUS_KeySlotGlobalControlWordSettings  *pSettings
    );



#ifdef __cplusplus
}
#endif

#endif
