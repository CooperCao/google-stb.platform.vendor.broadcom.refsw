/***************************************************************************
*     (c)2004-2011 Broadcom Corporation
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
*   API name: AudioDac
*    Specific APIs related to audio DAC outputs.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_AUDIO_DAC_H__
#define NEXUS_AUDIO_DAC_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: AudioDac

Header file: nexus_audio_dac.h

Module: Audio

Description: Route PCM audio data to a set of L/R DAC outputs

**************************************/

/**
Summary:
Handle for audio DAC
**/
typedef struct NEXUS_AudioDac *NEXUS_AudioDacHandle;

/***************************************************************************
Summary:
Audio DAC Mute Modes
***************************************************************************/
typedef enum NEXUS_AudioDacMuteType
{
    /* These options are supported only on the 7400/7401 chipsets. */
    NEXUS_AudioDacMuteType_eConstantLow,
    NEXUS_AudioDacMuteType_eConstantHigh,
    NEXUS_AudioDacMuteType_eSquareWaveOpp,
    NEXUS_AudioDacMuteType_eSquareWaveSame,    
    NEXUS_AudioDacMuteType_eAaaa,               /* Supported on 7405, 3548, and 7420 classes of 65nm chips */
    NEXUS_AudioDacMuteType_e5555,               /* Supported on 7405, 3548, and 7420 classes of 65nm chips */
    NEXUS_AudioDacMuteType_eCustomValue,        /* Supported on 7405, 3548, and 7420 classes of 65nm chips as well as 40nm chips (7422, 35230, etc.) */
    NEXUS_AudioDacMuteType_eDrive0,             /* Supported on 40nm chips (7422, 35230, etc.) */
    NEXUS_AudioDacMuteType_eDriveNegative1,     /* Supported on 40nm chips (7422, 35230, etc.) */    
    NEXUS_AudioDacMuteType_eMax
} NEXUS_AudioDacMuteType;

/***************************************************************************
Summary:
Audio DAC Settings
***************************************************************************/
typedef struct NEXUS_AudioDacSettings
{
    NEXUS_AudioDacMuteType muteType;
    uint16_t muteValueLeft;     /* 16-bit sample, used only if mute type == NEXUS_AudioDacMuteType_eCustomValue */
    uint16_t muteValueRight;    /* 16-bit sample, used only if mute type == NEXUS_AudioDacMuteType_eCustomValue */

    struct
    {
        bool enabled;           /* If true, test tone output is enabled with the settings below.  If false, the tone output is disabled. */
        int32_t samples[64];    /* Test tone samples.  The samples are 20-bit signed data. */
        bool zeroOnLeft;        /* If true, the left channel will output zeroes */
        bool zeroOnRight;       /* If true, the right channel will output zeroes */
        bool sharedSamples;     /* If true (default), samples 0..63 will be output on both Left and Right.
                                   If false, samples 0..31 will output on Left and 32..63 on Right. */
        unsigned numSamplesLeft;    /* Number of samples to play on the left channel before repeating (1..64) */
        unsigned numSamplesRight;   /* Number of samples to play on the right channel before repeating (1..64) */
        unsigned sampleRate;        /* Sampling frequency of the samples */
    } testTone;

	uint32_t peakGain;      	/* PEAK_GAIN - Peaking filter gain */
    uint32_t scale;             /* Affects the setting of the HIFIDAC_CTRLn_SCALE register directly.  Applicable to DTV systems only. */
    int32_t volume;             /* Attenuation in 1/100 dB.  Ranges from NEXUS_AUDIO_VOLUME_DB_NORMAL to NEXUS_AUDIO_VOLUME_DB_MIN.  Default=0. */
} NEXUS_AudioDacSettings;

/***************************************************************************
Summary:
Get default settings for an audio DAC
***************************************************************************/
void NEXUS_AudioDac_GetDefaultSettings(
    NEXUS_AudioDacSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
Open an audio DAC
***************************************************************************/
NEXUS_AudioDacHandle NEXUS_AudioDac_Open( /* attr{destructor=NEXUS_AudioDac_Close}  */
    unsigned index,
    const NEXUS_AudioDacSettings *pSettings     /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Close an audio DAC
    
Description:
Input to the DAC must be removed prior to closing.
***************************************************************************/
void NEXUS_AudioDac_Close(
    NEXUS_AudioDacHandle handle
    );

/***************************************************************************
Summary:
Get Settings for an audio DAC
***************************************************************************/
void NEXUS_AudioDac_GetSettings(
    NEXUS_AudioDacHandle handle,
    NEXUS_AudioDacSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
Set Settings for an audio DAC
***************************************************************************/
NEXUS_Error NEXUS_AudioDac_SetSettings(
    NEXUS_AudioDacHandle handle,
    const NEXUS_AudioDacSettings *pSettings    /* [in] Settings */
    );

/***************************************************************************
Summary:
Get the audio connector for an audio DAC
***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioDac_GetConnector(
    NEXUS_AudioDacHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_DAC_H__ */

