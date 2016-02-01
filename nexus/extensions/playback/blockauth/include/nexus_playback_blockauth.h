/***************************************************************************
 *     (c)2015 Broadcom Corporation
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
 **************************************************************************/
#ifndef NEXUS_PLAYBACK_BLOCKAUTH_H__
#define NEXUS_PLAYBACK_BLOCKAUTH_H__

#include "nexus_playback.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
If enabled is set to true then this mode is switched on.

ca_state is the key management state
and will be passed when calling authorize_block and
load_key.

The authorize_block callback function must return, in the
"authorized" variable, the number of bytes starting from
"position" that can be injected given the currently loaded
key(s). "position" is a byte offset. If "authorized" is
returned as 0, then "position" cannot be authorized and a
key needs to be loaded. At that point the load_key function
will be called by NEXUS_Playback. If authorize_block
succeeds, the function return will be 0, otherwise non-zero.
The function should only fail if the ca_state is invalid.

The load_key callback function must load the key applicable
to "position". If the CA system uses odd and even keys, then
that must be managed by the load_key callback function and
saved in the ca_state - it is entirely responsible for
tracking the full range of data that can be played with the
current keys. The authorize_block accesses the ca_state to
actually authorize the block. If the function succeeds, the
function return value will return 0, otherwise it will
return non-zero. The function should fail if the ca_state is
invalid or the key could not be loaded.
**/
typedef struct NEXUS_PlaybackBlockAuthorizationSettings
{
    bool enabled;
    void *ca_state;
    int (*authorize_block)(void *ca_state, off_t position, size_t size, size_t *authorized);
    int (*load_key)(void *ca_state, off_t position);
} NEXUS_PlaybackBlockAuthorizationSettings;

void NEXUS_Playback_GetBlockAuthorizationSettings(
    NEXUS_PlaybackHandle playback,
    NEXUS_PlaybackBlockAuthorizationSettings *pSettings
    );

/**
Must be called when playback is stopped.
**/
NEXUS_Error NEXUS_Playback_SetBlockAuthorizationSettings(
    NEXUS_PlaybackHandle playback,
    const NEXUS_PlaybackBlockAuthorizationSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif
