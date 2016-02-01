/***************************************************************************
*     (c)2004-2010 Broadcom Corporation
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
*   API name: DolbyVolume
*    Specific APIs related to Dolby Volume Audio Processing
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef BAPE_DOLBY_VOLUME_H__
#define BAPE_DOLBY_VOLUME_H__

#include "bape.h"

/***************************************************************************
Summary:
DolbyVolume Handle
***************************************************************************/
typedef struct BAPE_DolbyVolume *BAPE_DolbyVolumeHandle;

/***************************************************************************
Summary:
DolbyVolume Settings
***************************************************************************/
typedef struct BAPE_DolbyVolumeSettings
{
    bool enabled;       /* If true, processing is enabled.  Otherwise, the stage is bypassed. */

    /* System Settings*/
    int preGain;                    /* Gain applied to the signal prior to entering DV258.  Ranges from -480 to 480 (-30..30 in steps of 0.0625 dB) (MS11 only) */
    unsigned inputReferenceLevel;   /* Input reference level.  Ranges from 0 to 2080 (0..130 in steps of 0.0625 dB) */
    unsigned outputReferenceLevel;  /* Input reference level.  Ranges from 0 to 2080 (0..130 in steps of 0.0625 dB) */
    int calibrationOffset;          /* Calibration offset.  Ranges from -480 to 480 (-30..30 in steps of 0.0625 dB) */
    bool reset;                     /* Forces a reset if true. */

    /* Volume Modeler Settings*/
    bool volumeModelerEnabled;      /* If true, the volume modeler is enabled. */
    int digitalVolumeLevel;         /* Volume level gain applied by Dolby Volume.  Ranges from -1536 to 480 (-96..30 in steps of 0.0625 dB) */
    int analogVolumeLevel;          /* Volume level gain applied after Dolby Volume.  Ranges from -1536 to 480 (-96..30 in steps of 0.0625 dB) */

    /* Volume Leveler Settings */
    bool volumeLevelerEnabled;      /* If true, the volume leveler is enabled */
    bool midsideProcessingEnabled;  /* If true, midside processing is enabled */
    bool halfModeEnabled;           /* If true, half mode will be enabled */
    unsigned volumeLevelerAmount;   /* Ranges from 0 to 10 */

    /* Limiter Settings */
    bool limiterEnabled;            /* If true, the limiter is enabled (MS11 only) */
} BAPE_DolbyVolumeSettings;

/***************************************************************************
Summary:
    Get default settings for a DolbyVolume stage
***************************************************************************/
void BAPE_DolbyVolume_GetDefaultSettings(
    BAPE_DolbyVolumeSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open a DolbyVolume stage
***************************************************************************/
BERR_Code BAPE_DolbyVolume_Create(
    BAPE_Handle deviceHandle,
    const BAPE_DolbyVolumeSettings *pSettings,
    BAPE_DolbyVolumeHandle *pHandle             /* [out] */
    );

/***************************************************************************
Summary:
    Close a DolbyVolume stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void BAPE_DolbyVolume_Destroy(
    BAPE_DolbyVolumeHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for a DolbyVolume stage
***************************************************************************/
void BAPE_DolbyVolume_GetSettings(
    BAPE_DolbyVolumeHandle handle,
    BAPE_DolbyVolumeSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for a DolbyVolume stage
***************************************************************************/
BERR_Code BAPE_DolbyVolume_SetSettings(
    BAPE_DolbyVolumeHandle handle,
    const BAPE_DolbyVolumeSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for a DolbyVolume stage
***************************************************************************/
void BAPE_DolbyVolume_GetConnector(
    BAPE_DolbyVolumeHandle handle,
    BAPE_Connector *pConnector       /* [out] */
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_DolbyVolume_AddInput(
    BAPE_DolbyVolumeHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_DolbyVolume_RemoveInput(
    BAPE_DolbyVolumeHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_DolbyVolume_RemoveAllInputs(
    BAPE_DolbyVolumeHandle handle
    );

#endif /* #ifndef BAPE_DOLBY_VOLUME_H__ */

