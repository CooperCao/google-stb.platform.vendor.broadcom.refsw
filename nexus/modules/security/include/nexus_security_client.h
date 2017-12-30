/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
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
All pid channels are associated with a keyslot, either by default or by the application.

To handle encrypted data, the application must call NEXUS_KeySlot_AddPidChannel to associate a client-managed keyslot.

To move clear data from GLR to GLR (global region), or from GLR to CRR (compressed restricted region), the system defaults
to a "bypass keyslot" called NEXUS_BypassKeySlot_eG2GR.

To move clear data from CRR to CRR, the application must call NEXUS_SetPidChannelBypassKeyslot with NEXUS_BypassKeySlot_eGR2R.
Before closing the pid channel in this configuration, the app should call NEXUS_SetPidChannelBypassKeyslot to restore the pid channel
back to NEXUS_BypassKeySlot_eG2GR. This cannot be done automatically by Nexus because of required synchronization.
Instead, Nexus will detect it as a "leak" and print a warning to fix the application.

Pid channels cannot move data from CRR to GLR.
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
