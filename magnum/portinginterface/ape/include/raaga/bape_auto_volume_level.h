/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*   API name: AutoVolumeLevel
*    Specific APIs related to Broadcom Auto Volume Levelling (AVL) processing
*
***************************************************************************/

#ifndef BAPE_AUTO_VOLUME_LEVEL_H__
#define BAPE_AUTO_VOLUME_LEVEL_H__

#include "bape.h"

/***************************************************************************
Summary: 
AutoVolumeLevel Handle 
***************************************************************************/
typedef struct BAPE_AutoVolumeLevel *BAPE_AutoVolumeLevelHandle;

/***************************************************************************
Summary:
AutoVolumeLevel Settings
***************************************************************************/
typedef struct BAPE_AutoVolumeLevelSettings
{
    bool enabled;                   /* If true, processing is enabled.  Otherwise this stage is bypassed */
    bool loudnessEqualization;      /* If true, loudness level equivalence processing is enabled. */

    /* The settings below are typically used at defaults, but may be modified if desired.
       Careful study and analysis is recommended before adjusting these values.  */

    int target;                     /* Target level of output signal in terms of dBFs */
    int lowerBound;                 /* Lower bound for primary range of volume control */
    unsigned fixedBoost;            /* Amount of fixed boost (in dB) if level < LOWERBOUND */
    unsigned ref;                   /* Reference logarithmic gain value of 0 dBFs */
    unsigned alpha;                 /* Attenuation factor for Level Maximum */
    unsigned beta;                  /* Attenuation factor for Level Mean */
    unsigned threshold;             /* Threshold for detecting active portion of signal . Input value should be multiplied by factor of 10 */
    unsigned dtfPercent;            /* Decay Time for DTF Percent delay (in seconds) */
    unsigned alpha2;                /* Attenuation factor for Level Minimum */
    unsigned fastGainRamp;          /* Fast Gain Ramp in milliseconds. Input value should be multiplied by factor of 10 */
    unsigned dtf;                   /* Decay Time Fraction. Input value should be multiplied by factor of 10 */
} BAPE_AutoVolumeLevelSettings;

/***************************************************************************
Summary:
AutoVolumeLevel Settings
***************************************************************************/
typedef struct BAPE_AutoVolumeLevelStatus
{
    unsigned numChannels;               /* number of channels AVL is currently processing */
    int peakLevel[BAPE_Channel_eMax];   /* describes the level for each channel in -dB below unity */
    int rmsLevel[BAPE_Channel_eMax];    /* describes the level for each channel in -dB below unity */
    uint32_t pts;                       /* 45kHz based PTS indicating the PTS at the time the peak and RMS values were sampled */
} BAPE_AutoVolumeLevelStatus;

/***************************************************************************
Summary:
    Get default settings for an AutoVolumeLevel stage
***************************************************************************/
void BAPE_AutoVolumeLevel_GetDefaultSettings(
    BAPE_AutoVolumeLevelSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open an AutoVolumeLevel stage
***************************************************************************/
BERR_Code BAPE_AutoVolumeLevel_Create(
    BAPE_Handle deviceHandle,
    const BAPE_AutoVolumeLevelSettings *pSettings,
    BAPE_AutoVolumeLevelHandle *pHandle
    );

/***************************************************************************
Summary:
    Close an AutoVolumeLevel stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void BAPE_AutoVolumeLevel_Destroy(
    BAPE_AutoVolumeLevelHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for an AutoVolumeLevel stage
***************************************************************************/
void BAPE_AutoVolumeLevel_GetSettings(
    BAPE_AutoVolumeLevelHandle handle,
    BAPE_AutoVolumeLevelSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for an AutoVolumeLevel stage
***************************************************************************/
BERR_Code BAPE_AutoVolumeLevel_SetSettings(
    BAPE_AutoVolumeLevelHandle handle,
    const BAPE_AutoVolumeLevelSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for an AutoVolumeLevel stage
***************************************************************************/
void BAPE_AutoVolumeLevel_GetConnector(
    BAPE_AutoVolumeLevelHandle handle,
    BAPE_Connector *pConnector
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_AutoVolumeLevel_AddInput(
    BAPE_AutoVolumeLevelHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_AutoVolumeLevel_RemoveInput(
    BAPE_AutoVolumeLevelHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_AutoVolumeLevel_RemoveAllInputs(
    BAPE_AutoVolumeLevelHandle handle
    );

/***************************************************************************
Summary:
Get Status of the AVL processor
  an error return code indicates that status is not available/valid
***************************************************************************/
BERR_Code BAPE_AutoVolumeLevel_GetStatus(
    BAPE_AutoVolumeLevelHandle handle,
    BAPE_AutoVolumeLevelStatus *pStatus    /* [out] Status */
    );

#endif /* #ifndef BAPE_AUTO_VOLUME_LEVEL_H__ */
