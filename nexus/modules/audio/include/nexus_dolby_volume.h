/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
*   API name: DolbyVolume
*    Specific APIs related to Dolby Volume Audio Processing
*
***************************************************************************/

#ifndef NEXUS_DOLBY_VOLUME_H__
#define NEXUS_DOLBY_VOLUME_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "nexus_audio_processing_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: DolbyVolume

Header file: nexus_dolby_volume.h

Module: Audio

Description: Dolby Volume stage

**************************************/

typedef struct NEXUS_DolbyVolume *NEXUS_DolbyVolumeHandle;

/***************************************************************************
Summary:
DolbyVolume Settings
***************************************************************************/
typedef struct NEXUS_DolbyVolumeSettings
{
    bool enabled;       /* If true, processing is enabled.  Otherwise, the stage is bypassed. */

    int	blockSize;      /*Size of processing block in samples.  Ranges from 256 to 512. */
    unsigned numBands;  /* Number of critical bands to use.  Values of 20 or 40 are legal. */

    /* System Settings*/
    unsigned inputReferenceLevel;   /* Input reference level in dBSPL.  Ranges from 0 to 100. */
    unsigned outputReferenceLevel;  /* Output reference level in dBSPL.  Ranges from 0 to 100. */
    int calibrationOffset;          /* Calibration offset in dB.  Ranges from -100 to +30. */
    bool reset;                     /* Forces a reset if true. */

    /*Volume Modeler Settings*/
    bool volumeModelerEnabled;      /* If true, the volume modeler is enabled. */
    int	digitalVolumeLevel;         /* Volume level gain in dB.  Ranges from -100 to 30. */
    int analogVolumeLevel;          /* Volume level gain in dB.  Ranges from -100 to 30. */

    /*Volume Leveler Settings */
    bool volumeLevelerEnabled;      /* If true, the volume leveler is enabled */
    bool midsideProcessingEnabled;  /* If true, midside processing is enabled */
    bool halfModeEnabled;           /* If true, half mode will be enabled */
    unsigned volumeLevelerAmount;   /* Ranges from 0 to 10 */

} NEXUS_DolbyVolumeSettings;

/***************************************************************************
Summary:
    Get default settings for a DolbyVolume stage
***************************************************************************/
void NEXUS_DolbyVolume_GetDefaultSettings(
    NEXUS_DolbyVolumeSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open a DolbyVolume stage
***************************************************************************/
NEXUS_DolbyVolumeHandle NEXUS_DolbyVolume_Open( /* attr{destructor=NEXUS_DolbyVolume_Close}  */
    const NEXUS_DolbyVolumeSettings *pSettings     /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
    Close a DolbyVolume stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void NEXUS_DolbyVolume_Close(
    NEXUS_DolbyVolumeHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for a DolbyVolume stage
***************************************************************************/
void NEXUS_DolbyVolume_GetSettings(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_DolbyVolumeSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for a DolbyVolume stage
***************************************************************************/
NEXUS_Error NEXUS_DolbyVolume_SetSettings(
    NEXUS_DolbyVolumeHandle handle,
    const NEXUS_DolbyVolumeSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for a DolbyVolume stage
***************************************************************************/
NEXUS_AudioInputHandle NEXUS_DolbyVolume_GetConnector(
    NEXUS_DolbyVolumeHandle handle
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
NEXUS_Error NEXUS_DolbyVolume_AddInput(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_DolbyVolume_RemoveInput(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_DolbyVolume_RemoveAllInputs(
    NEXUS_DolbyVolumeHandle handle
    );

/***************************************************************************
Summary:
Dolby Volume 258 (MS11) Handle
***************************************************************************/
typedef struct NEXUS_DolbyVolume258 *NEXUS_DolbyVolume258Handle;

/***************************************************************************
Summary:
    Get default settings for a DolbyVolume258 stage
***************************************************************************/
void NEXUS_DolbyVolume258_GetDefaultSettings(
    NEXUS_DolbyVolume258Settings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open a DolbyVolume258 stage
***************************************************************************/
NEXUS_DolbyVolume258Handle NEXUS_DolbyVolume258_Open( /* attr{destructor=NEXUS_DolbyVolume258_Close}  */
    const NEXUS_DolbyVolume258Settings *pSettings     /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
    Close a DolbyVolume258 stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void NEXUS_DolbyVolume258_Close(
    NEXUS_DolbyVolume258Handle handle
    );

/***************************************************************************
Summary:
    Get Settings for a DolbyVolume258 stage
***************************************************************************/
void NEXUS_DolbyVolume258_GetSettings(
    NEXUS_DolbyVolume258Handle handle,
    NEXUS_DolbyVolume258Settings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for a DolbyVolume258 stage
***************************************************************************/
NEXUS_Error NEXUS_DolbyVolume258_SetSettings(
    NEXUS_DolbyVolume258Handle handle,
    const NEXUS_DolbyVolume258Settings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for a DolbyVolume258 stage
***************************************************************************/
NEXUS_AudioInputHandle NEXUS_DolbyVolume258_GetConnector(
    NEXUS_DolbyVolume258Handle handle
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
NEXUS_Error NEXUS_DolbyVolume258_AddInput(
    NEXUS_DolbyVolume258Handle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_DolbyVolume258_RemoveInput(
    NEXUS_DolbyVolume258Handle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_DolbyVolume258_RemoveAllInputs(
    NEXUS_DolbyVolume258Handle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_DOLBY_VOLUME_H__ */
