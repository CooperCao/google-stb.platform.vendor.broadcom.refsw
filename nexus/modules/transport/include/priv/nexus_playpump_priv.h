/***************************************************************************
 *     (c)2010-2013 Broadcom Corporation
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
#ifndef NEXUS_PLAYPUMP_PRIV_H__
#define NEXUS_PLAYPUMP_PRIV_H__

#include "nexus_types.h"
#include "nexus_playpump.h"

#include "bpvrlib_feed.h"

#ifdef __cplusplus
extern "C" {
#endif

NEXUS_Error NEXUS_Playpump_AddExtendedOffsetEntries_priv(
        NEXUS_PlaypumpHandle playpump, /* instance of NEXUS_Playpump */
		const BPVRlib_Feed_ExtendedOffsetEntry *entries, /* pointer to array of BPVRlib_Feed_ExtendedOffsetEntries */ 
		size_t count, /* number of elements in the array */ 
		size_t *nconsumed/* out pointer to return number of consumed elements */
		);

NEXUS_Error NEXUS_Playpump_GetCompleted_priv(
        NEXUS_PlaypumpHandle playpump, /* instance of NEXUS_Playpump */
		size_t *ncompleted	/* out pointer to return number of completed elements */
		);

NEXUS_Error NEXUS_Playpump_StartMuxInput_priv(
        NEXUS_PlaypumpHandle playpump /* instance of NEXUS_Playpump */
        );

typedef struct NEXUS_Playpump_OpenPidChannelSettings_priv {
    uint16_t tsPid;
#if BXPT_SW7425_4528_WORKAROUND
    bool remapping;
    uint16_t remappedPesId;
#endif
    bool preserveCC;
} NEXUS_Playpump_OpenPidChannelSettings_priv;

void NEXUS_Playpump_GetDefaultOpenPidChannelSettings_priv(NEXUS_Playpump_OpenPidChannelSettings_priv *pSettings);

NEXUS_PidChannelHandle NEXUS_Playpump_OpenPidChannel_priv(
    NEXUS_PlaypumpHandle playpump,
    unsigned src_pid,                   /* destination substream */
    const NEXUS_PlaypumpOpenPidChannelSettings *pSettings,
    const NEXUS_Playpump_OpenPidChannelSettings_priv *pSettings_priv
    );

#if defined(B_HAS_ASF)
void* NEXUS_Playpump_GetAsfHandle_priv(NEXUS_PlaypumpHandle handle);
#endif

#ifdef __cplusplus
}
#endif

#endif
