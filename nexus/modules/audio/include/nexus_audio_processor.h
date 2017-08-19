/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
* API Description:
*   API name: AudioProcessor
*    Specific APIs related to Audio Post Processing
*
***************************************************************************/

#ifndef NEXUS_AUDIO_PROCESSOR_H__
#define NEXUS_AUDIO_PROCESSOR_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "nexus_audio_processing_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: AudioProcessor

Header file: nexus_audio_processor.h

Module: Audio

Description: Audio Post Processing stage

**************************************/

typedef struct NEXUS_AudioProcessor *NEXUS_AudioProcessorHandle;


/***************************************************************************
Summary:
Advanced Audio Tsm processing mode
***************************************************************************/
typedef enum NEXUS_AudioAdvancedTsmMode
{
    NEXUS_AudioAdvancedTsmMode_eOff,
    NEXUS_AudioAdvancedTsmMode_eDsola, /* stretches or shrinks audio, using pitch correction */
    NEXUS_AudioAdvancedTsmMode_ePpm,   /* repeat or drop audio samples, smoothing with neighboring samples */
    NEXUS_AudioAdvancedTsmMode_eMax
} NEXUS_AudioAdvancedTsmMode;

/***************************************************************************
Summary:
Advanced Audio Tsm Settings
***************************************************************************/
typedef struct NEXUS_AudioAdvancedTsmSettings
{
    NEXUS_AudioAdvancedTsmMode mode;
} NEXUS_AudioAdvancedTsmSettings;

/***************************************************************************
Summary:
Advanced Audio Tsm Status
***************************************************************************/
typedef struct NEXUS_AudioAdvancedTsmStatus
{
    NEXUS_AudioAdvancedTsmMode mode;    /* current mode of Advanced TSM processor */
    uint32_t pts;                       /* current PTS in 45kHz units */
    NEXUS_PtsType ptsType;              /* PTS Type */
    int correction;                     /* Correction to this PTS in milliseconds */
} NEXUS_AudioAdvancedTsmStatus;

/***************************************************************************
Summary:
Audio Processor Settings
***************************************************************************/
typedef struct NEXUS_AudioProcessorSettings
{
    NEXUS_AudioPostProcessing type;
    union
    {
        NEXUS_AudioFadeSettings fade;
        NEXUS_KaraokeVocalSettings karaokeVocal;
        NEXUS_AudioAdvancedTsmSettings advancedTsm;
        NEXUS_AmbisonicSettings ambisonic;
    } settings;
} NEXUS_AudioProcessorSettings;

/***************************************************************************
Summary:
Audio Processor Open Settings
***************************************************************************/
typedef struct NEXUS_AudioProcessorOpenSettings
{
    NEXUS_AudioPostProcessing type;
} NEXUS_AudioProcessorOpenSettings;

/* Define separate OpenSettings, or allow type to be in the general settings,
   but have no effect (print warning) during SetSettings */

/***************************************************************************
Summary:
Audio Processor Status
***************************************************************************/
typedef struct NEXUS_AudioProcessorStatus
{
    NEXUS_AudioPostProcessing type;
    union
    {
        NEXUS_AudioAdvancedTsmStatus advancedTsm;
        NEXUS_AudioFadeStatus fade;
    } status;
} NEXUS_AudioProcessorStatus;

/***************************************************************************
Summary:
    Get default settings for a AudioProcessor stage
***************************************************************************/
void NEXUS_AudioProcessor_GetDefaultOpenSettings(
    NEXUS_AudioProcessorOpenSettings *pSettings   /* [out] default open settings */
    );

/***************************************************************************
Summary:
    Open a AudioProcessor stage
***************************************************************************/
NEXUS_AudioProcessorHandle NEXUS_AudioProcessor_Open( /* attr{destructor=NEXUS_AudioProcessor_Close}  */
    const NEXUS_AudioProcessorOpenSettings *pSettings     /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
    Close a AudioProcessor stage

Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void NEXUS_AudioProcessor_Close(
    NEXUS_AudioProcessorHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for a AudioProcessor stage
***************************************************************************/
void NEXUS_AudioProcessor_GetSettings(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioProcessorSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for a AudioProcessor stage
***************************************************************************/
NEXUS_Error NEXUS_AudioProcessor_SetSettings(
    NEXUS_AudioProcessorHandle handle,
    const NEXUS_AudioProcessorSettings *pSettings
    );

/***************************************************************************
Summary:
    Get Status for a AudioProcessor stage
***************************************************************************/
void NEXUS_AudioProcessor_GetStatus(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioProcessorStatus *pStatus    /* [out] Status */
    );

/***************************************************************************
Summary:
    Get the audio connector for a AudioProcessor stage
***************************************************************************/
NEXUS_AudioInputHandle NEXUS_AudioProcessor_GetConnector(
    NEXUS_AudioProcessorHandle handle
    );

/***************************************************************************
Summary:
    Get the audio connector for a AudioProcessor stage
***************************************************************************/
NEXUS_AudioInputHandle NEXUS_AudioProcessor_GetConnectorByType(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioConnectorType type
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AudioProcessor_AddInput(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AudioProcessor_RemoveInput(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AudioProcessor_RemoveAllInputs(
    NEXUS_AudioProcessorHandle handle
    );
#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_PROCESSOR_H__ */
