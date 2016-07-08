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
***************************************************************************/
#ifndef NEXUS_AUDIO_OUTPUT_H__
#define NEXUS_AUDIO_OUTPUT_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: AudioOutput

Header file: nexus_audio_output.h

Module: Audio

Description: Abstract connector for routing audio to an output

**************************************/

/***************************************************************************
Summary:
Generic audio output settings
***************************************************************************/
typedef struct NEXUS_AudioOutputSettings
{
    NEXUS_Timebase timebase;    /* Timebase for this output.  This directly affects DAC outputs and outputs
                                   using the nco clock below.  If using a PLL, autoConfigureVcxo determines
                                   if the upstream VCXO-PLL is set to this timebase as well. */

	NEXUS_AudioChannelMode channelMode;

    NEXUS_AudioVolumeType volumeType; /* specifies the units for leftVolume and rightVolume. */
    int32_t leftVolume; /* the units depend on the value of volumeType. See docs for NEXUS_AudioVolumeType. */
    int32_t rightVolume; /* the units depend on the value of volumeType. See docs for NEXUS_AudioVolumeType. */

    bool muted;
    
    unsigned additionalDelay; /* Delay to add to this path. If you are using SyncChannel, this additionalDelay will 
    							 be added to whatever delay SyncChannel assigns. In milliseconds. */

    NEXUS_AudioOutputPll pll; /* PLL Selection.  May not apply to all outputs, see NEXUS_AudioOutputPll for details. */
    bool autoConfigureVcxo;   /* If true (default), the timebase above will be automatically programmed to the VCXO-PLL 
                                 feeding the PLL above. */  

    NEXUS_AudioOutputNco nco; /* NCO Selection.  May not apply to all outputs, see NEXUS_AudioOutputNco for details. */

    unsigned defaultSampleRate; /* This is the default sample rate for an output in Hz (default=48000). 
                                   Outputs connected to an AudioDecoder will automatically switch to the 
                                   decoded sample rate.  Outputs connected to other inputs such as AudioPlayback
                                   or I2sInput will sample rate convert the input data to this rate.  Adjusting
                                   this rate to match the source rate can avoid a sample rate conversion. 
                                   Supported values are 48000, 44100, and 32000. */
} NEXUS_AudioOutputSettings;

/***************************************************************************
Summary:
Get settings of an audio output
***************************************************************************/
void NEXUS_AudioOutput_GetSettings(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioOutputSettings *pSettings    /* [out] Current Settings */
    );

/***************************************************************************
Summary:
Set settings of an audio output
***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_SetSettings(
    NEXUS_AudioOutputHandle output,
    const NEXUS_AudioOutputSettings *pSettings
    );

/***************************************************************************
Summary:
Status of the audio output
***************************************************************************/
typedef struct NEXUS_AudioOutputStatus 
{
    unsigned delay; /* Delay on this path, in milliseconds. This reports the total delay including SyncChannel's delay and NEXUS_AudioOutputSettings.additionalDelay. */
} NEXUS_AudioOutputStatus;

/***************************************************************************
Summary:
Get status of an audio output
***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_GetStatus(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioOutputStatus *pStatus    /* [out] */
    );

/***************************************************************************
Summary:
Outputs that require unique timebases
***************************************************************************/
typedef struct NEXUS_AudioOutputEnabledOutputs
{
#if defined(NEXUS_NUM_AUDIO_DUMMY_OUTPUTS) && NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
    unsigned audioDummy[NEXUS_NUM_AUDIO_DUMMY_OUTPUTS];
#endif
#if defined(NEXUS_NUM_SPDIF_OUTPUTS) && NEXUS_NUM_SPDIF_OUTPUTS
    unsigned spdif[NEXUS_NUM_SPDIF_OUTPUTS];
#endif
#if defined(NEXUS_NUM_DAC_OUTPUTS) && NEXUS_NUM_DAC_OUTPUTS
    unsigned dac[NEXUS_NUM_DAC_OUTPUTS];
#endif
#if defined(NEXUS_NUM_I2S_OUTPUTS) && NEXUS_NUM_I2S_OUTPUTS
    unsigned i2s[NEXUS_NUM_I2S_OUTPUTS];
#endif
#if defined(NEXUS_NUM_I2S_MULTI_OUTPUTS) && NEXUS_NUM_I2S_MULTI_OUTPUTS
    unsigned i2sMulti[NEXUS_NUM_I2S_MULTI_OUTPUTS];
#endif
#if defined(NEXUS_NUM_HDMI_OUTPUTS) && NEXUS_NUM_HDMI_OUTPUTS
    unsigned hdmi[NEXUS_NUM_HDMI_OUTPUTS];
#endif
    int unused; /*prevents compiler warnings*/
} NEXUS_AudioOutputEnabledOutputs;


/***************************************************************************
Summary:
Recommended setting of pll and nco
***************************************************************************/
typedef struct NEXUS_AudioOutputClockSource
{
    NEXUS_AudioOutputPll pll;
    NEXUS_AudioOutputNco nco;
} NEXUS_AudioOutputClockSource;

/***************************************************************************
Summary:
Recommmend setting of pll and nco for each output
***************************************************************************/
typedef struct NEXUS_AudioOutputClockConfig
{
#if defined(NEXUS_NUM_AUDIO_DUMMY_OUTPUTS) && NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
    NEXUS_AudioOutputClockSource audioDummy[NEXUS_NUM_AUDIO_DUMMY_OUTPUTS];
#endif
#if defined(NEXUS_NUM_SPDIF_OUTPUTS) && NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutputClockSource spdif[NEXUS_NUM_SPDIF_OUTPUTS];
#endif
#if defined(NEXUS_NUM_I2S_OUTPUTS) && NEXUS_NUM_I2S_OUTPUTS
    NEXUS_AudioOutputClockSource i2s[NEXUS_NUM_I2S_OUTPUTS];
#endif
#if defined(NEXUS_NUM_I2S_MULTI_OUTPUTS) && NEXUS_NUM_I2S_MULTI_OUTPUTS
    NEXUS_AudioOutputClockSource i2sMulti[NEXUS_NUM_I2S_MULTI_OUTPUTS];
#endif
#if defined(NEXUS_NUM_HDMI_OUTPUTS) && NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutputClockSource hdmi[NEXUS_NUM_HDMI_OUTPUTS];
#endif
    int unused; /*prevents compiler warnings*/
} NEXUS_AudioOutputClockConfig;


/***************************************************************************
Summary:
Initialize NEXUS_AudioOutputEnabledOutputs
***************************************************************************/
void NEXUS_AudioOutput_GetDefaultEnabledOutputs(
    NEXUS_AudioOutputEnabledOutputs *pOutputs);

/***************************************************************************
Summary:
Determines the a proper configuration of timebases for outputs

For the NEXUS_AudioOutputEnabledOutputs set the output to the timebase id for
all outputs that will share a time base.  We recommend 1,2,3,4.., though 
the timebase id is arbitrary but 0 is consider invalid.

Note: 
DAC has its own clock and will not be taken into consideration when 
determing unique clock sources.

Example:
If HDMI and SPDIF will share a clock source, no I2S outputs are enabled 
and 2 Dummy Outputs required.  Configure the structure such as:

audioDummy[2,3]
spdif[1]
hdmi[1]

***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_CreateClockConfig(
    const NEXUS_AudioOutputEnabledOutputs *outputs,
    NEXUS_AudioOutputClockConfig *config);

/***************************************************************************
Summary:
Add an input to this output
***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_AddInput(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
Add an input to this output
***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_RemoveInput(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
Add an input to this output
***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_RemoveAllInputs(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
Shutdown this output handle

Description:
    This routine should be called before the specific output object
    (e.g. AudioDac) is closed.
***************************************************************************/
void NEXUS_AudioOutput_Shutdown(
    NEXUS_AudioOutputHandle output
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_OUTPUT_H__ */

