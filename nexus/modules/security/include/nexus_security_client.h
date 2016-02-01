/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
#ifndef NEXUS_SECURITY_CLIENT_H__
#define NEXUS_SECURITY_CLIENT_H__

#include "nexus_security_datatypes.h"
#include "nexus_pid_channel.h"

/* security API's callable by nexus clients */

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Get default settings for NEXUS_Security_LoadClearKey
**/
void NEXUS_Security_GetDefaultClearKey(
    NEXUS_SecurityClearKey *pClearKey /* [out] */
    );

/**
Summary:
Load a clear key to a keyslot.
**/
NEXUS_Error NEXUS_Security_LoadClearKey(
    NEXUS_KeySlotHandle keyslot,
    const NEXUS_SecurityClearKey *pClearKey
    );

/**
Summary:
Get default settings for NEXUS_Security_AllocateKeySlot
**/
void NEXUS_Security_GetDefaultKeySlotSettings(
    NEXUS_SecurityKeySlotSettings *pSettings /* [out] */
    );

/**
Summary:
Allocate a keyslot for use in the security module.
**/
NEXUS_KeySlotHandle NEXUS_Security_AllocateKeySlot(
    const NEXUS_SecurityKeySlotSettings *pSettings
    );

/**
Summary:
Destroy a keyslot created by NEXUS_Security_AllocateKeySlot.
**/
void NEXUS_Security_FreeKeySlot(
    NEXUS_KeySlotHandle keyslot
    );

/**
Summary:
Add a PID channel to the CA or CACP keyslot.

Description:
This function shall add a PID channel to the CA or CACP keyslot. The newly
added PID channel will use the alogirthm and key value of the CA or CACP
keyslot for descrambling or descrambling followed by CPS.
**/
NEXUS_Error NEXUS_KeySlot_AddPidChannel(
    NEXUS_KeySlotHandle keyslot,
    NEXUS_PidChannelHandle pidChannel
    );


/**
Summary:
Remove a PID channel from the CA or CACP keyslot.

Description:
This function shall remove a PID channel from the CA or CACP keyslot. The
removed PID channel will NOT use the algorithm and key value of the CA or CACP
keyslot anymore.
**/
void NEXUS_KeySlot_RemovePidChannel(
    NEXUS_KeySlotHandle keyslot,
    NEXUS_PidChannelHandle pidChannel
    );
    
    
/**
Summary:
Get default settings for NEXUS_Security_InvalidateKey
**/
void NEXUS_Security_GetDefaultInvalidateKeySettings(
    NEXUS_SecurityInvalidateKeySettings *pSettings /* [out] */
    );


/**
Summary:
Invalidate the current settings for the key slot
**/
NEXUS_Error NEXUS_Security_InvalidateKey(
    NEXUS_KeySlotHandle keyslot,
    const NEXUS_SecurityInvalidateKeySettings *pSettings
    );


/**
Summary:
Get default settings for NEXUS_Security_ConfigAlgorithm
**/
void NEXUS_Security_GetDefaultAlgorithmSettings(
    NEXUS_SecurityAlgorithmSettings *pSettings /* [out] */
    );

/**
Summary:
Configure algorithm on a keyslot.
**/
NEXUS_Error NEXUS_Security_ConfigAlgorithm(
    NEXUS_KeySlotHandle keyslot,
    const NEXUS_SecurityAlgorithmSettings *pSettings
    );

/* backward compat */
#define NEXUS_Security_GetKeySlotInfo NEXUS_KeySlot_GetInfo

/**
Summary:
    Retrieve the external key data associated with a keySlot.
    NEXUS_Security_ConfigAlgorithm must be called before this function, and enableExtKey and/or enableExtIv must have been set.
**/
NEXUS_Error NEXUS_KeySlot_GetExternalKeyData(
    NEXUS_KeySlotHandle keyslot,
    NEXUS_SecurityAlgorithmConfigDestination dest, /* Identify block,  cpd, ca, or cps          */
    NEXUS_SecurityKeyType  keyDestEntryType,       /* Identify polarity  odd, even, or clear  */
    NEXUS_KeySlotExternalKeyData *pKeyData         /*[out] */
    );


/**
Summary:

All pid channels must be associated with a keyslot. Pid channels handing encrtpted data will have a
regular client managed keyslot assocaiated with it.

For pid channels handeling clear data the system will default to a Bypass keyslot for capable of
transferring data from GLR to GLR.

If a pid channel is handling clear data and has a destination in CRR (with source CRR or GLR), this function
must be called to associated it with the NEXUS_BypassKeySlot_eGR2R Bypass Keyslot.

**/
NEXUS_Error NEXUS_SetPidChannelBypassKeyslot(
    NEXUS_PidChannelHandle pidChannel,
    NEXUS_BypassKeySlot bypassKeySlot
    );

void NEXUS_GetPidChannelBypassKeyslot(
    NEXUS_PidChannelHandle pidChannel,
    NEXUS_BypassKeySlot *pBypassKeySlot
    );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
