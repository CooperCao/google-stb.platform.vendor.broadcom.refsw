/***************************************************************************
 *     (c)2007-2010 Broadcom Corporation
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
#ifndef NEXUS_SYNC_CHANNEL_INIT_H__
#define NEXUS_SYNC_CHANNEL_INIT_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_SyncChannelModuleSettings 
{
    struct 
    {
        NEXUS_ModuleHandle audio;
        NEXUS_ModuleHandle display;
    } modules;
    struct {
        bool requireFullScreen;
        unsigned tsmLockTimeout; /* time to wait after receiving a video TSM callback before accepting the PTSSTCDiff value */
        unsigned unmuteTimeout; /* time to wait before unmuting video after adjusting the PTS offset */
        unsigned unconditionalUnmuteTimeout; /* time to wait before unconditionally unmuting video (in case of error with audio sample rate callback or failure to start audio) */
        struct
        {
            unsigned timeout; /* time to wait before checking the number of callbacks received vs the acceptable callback count for rate mismatch */
            unsigned acceptableMtbcLower; /* acceptable mean time between callbacks lower bound */
            unsigned acceptableMtbcUpper; /* acceptable mean time between callbacks upper bound */
            unsigned acceptableTtlc; /* acceptable time to last callback for mismatches */
        } rateMismatchDetection;
        struct
        {
            unsigned timeout; /* time to wait before checking the number of callbacks received vs the acceptable callback count for rate rematch */
            unsigned acceptableTtlc; /* acceptable time to last callback for rematches */
        } rateRematchDetection;
    } video;
    struct
    {
        unsigned unmuteTimeout; /* time to wait before unmuting audio after accepting video TSM callback */
        unsigned receiverDelayCompensation; /* compensate for external audio receiver delay */
        unsigned unconditionalUnmuteTimeout; /* time to wait before unconditionally unmuting audio (in case of error with video TSM callback or failure to start video) */
    } audio;
} NEXUS_SyncChannelModuleSettings;

void NEXUS_SyncChannelModule_GetDefaultSettings(
    NEXUS_SyncChannelModuleSettings *pSettings /* [out] */
    );
    
NEXUS_ModuleHandle NEXUS_SyncChannelModule_Init(
    const NEXUS_SyncChannelModuleSettings *pSettings
    );
    
void NEXUS_SyncChannelModule_Uninit(void);

#ifdef __cplusplus
}
#endif

#endif
