/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
* API Description:
*   API name: AudioOutput
*    Generic APIs for audio outputs from an audio mixer.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_AUDIO_DECODER_PRIV_H__
#define NEXUS_AUDIO_DECODER_PRIV_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "priv/nexus_audio_input_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioDecoder);

#if NEXUS_HAS_ASTM
typedef struct NEXUS_AudioDecoderAstmSettings
{
    bool enableAstm; /* Astm will enable/disable astm mode */
    unsigned int syncLimit; /* limit outside of which decoder will not apply astm mode */
    bool enableTsm; /* Astm will enable/disable tsm. */
    bool enablePlayback; /* Astm will enable/disable playback. */
    int32_t ptsOffset; /* Astm will use this while in TSM-disabled mode */
    NEXUS_Callback firstPts_isr; /* Notify Astm when first PTS happens. */
    NEXUS_Callback tsmPass_isr; /* Notify Astm when TSM passes. */
    NEXUS_Callback tsmFail_isr; /* Notify Astm when TSM fails. */
    NEXUS_Callback tsmLog_isr; /* Notify Astm when TSM log buffer fills */
    NEXUS_Callback lifecycle_isr; /* Notify Astm when decoder is started/stopped */
    NEXUS_Callback watchdog_isr; /* Notify Astm when a watchdog occurs */
    void * callbackContext; /* user context passed in isr callbacks */
} NEXUS_AudioDecoderAstmSettings;

typedef struct NEXUS_AudioDecoderAstmStatus 
{
    bool started;
    uint32_t pts;
    int32_t ptsStcDiff;
    unsigned decodedCount;
    struct
    {
        unsigned int address;
        unsigned int size;
    } tsmLog;
} NEXUS_AudioDecoderAstmStatus;
#endif

#if NEXUS_HAS_SIMPLE_DECODER
typedef struct NEXUS_AudioDecoderSimpleSettings
{
    unsigned gaThreshold;
} NEXUS_AudioDecoderSimpleSettings;
#endif

NEXUS_Error NEXUS_AudioDecoder_ApplySettings_priv(
    NEXUS_AudioDecoderHandle handle
    );
        
#if NEXUS_HAS_ASTM    
void NEXUS_AudioDecoder_GetAstmSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder, 
    NEXUS_AudioDecoderAstmSettings * pAstmSettings  /* [out] */
    );

NEXUS_Error NEXUS_AudioDecoder_SetAstmSettings_priv( 
    NEXUS_AudioDecoderHandle audioDecoder, 
    const NEXUS_AudioDecoderAstmSettings * pAstmSettings 
    );
    
NEXUS_Error NEXUS_AudioDecoder_GetAstmStatus_isr( 
    NEXUS_AudioDecoderHandle audioDecoder, 
    NEXUS_AudioDecoderAstmStatus * pAstmStatus  /* [out] */
    );
#endif

void NEXUS_AudioDecoder_GetSyncSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder, 
    NEXUS_AudioInputSyncSettings *pSyncSettings  /* [out] */
    );
    
NEXUS_Error NEXUS_AudioDecoder_SetSyncSettings_priv( 
    NEXUS_AudioDecoderHandle audioDecoder, 
    const NEXUS_AudioInputSyncSettings *pSyncSettings 
    );
    
NEXUS_Error NEXUS_AudioDecoder_GetSyncStatus_isr( 
    NEXUS_AudioDecoderHandle audioDecoder, 
    NEXUS_AudioInputSyncStatus *pSyncStatus  /* [out] */
    );

#if NEXUS_HAS_SIMPLE_DECODER
void NEXUS_AudioDecoder_GetSimpleSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    NEXUS_AudioDecoderSimpleSettings *pSimpleSettings  /* [out] */
    );

NEXUS_Error NEXUS_AudioDecoder_SetSimpleSettings_priv(
    NEXUS_AudioDecoderHandle audioDecoder,
    const NEXUS_AudioDecoderSimpleSettings *pSimpleSettings
    );
#endif
        
#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_DECODER_PRIV_H__ */

