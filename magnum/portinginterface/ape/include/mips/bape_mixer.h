/***************************************************************************
 *     Copyright (c) 2006-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_MIXER_H_
#define BAPE_MIXER_H_

#include "bavc.h"

/***************************************************************************
Summary:
Mixer Handle
***************************************************************************/
typedef struct BAPE_Mixer *BAPE_MixerHandle;

/***************************************************************************
Summary:
Mixer Settings
***************************************************************************/
typedef struct BAPE_MixerSettings
{
    BAVC_AudioSamplingRate defaultSampleRate;   /* If there is no master input or the
                                                   master input is not started, the 
                                                   sample rate specified here will be 
                                                   applied to all outputs. */

    BAPE_Pll outputPll;                 /* What output PLL will be used with
                                           this mixer.  Required for I2S and SPDIF 
                                           outputs.  Independent PLLs should be used 
                                           if their input clock (VCXO) is different or 
                                           if their sample rate is not an integer multiple of
                                           other rates driven by the PLL. This cannot be 
                                           changed while inputs are running. */
    BAVC_Timebase outputTimebase;       /* What timebase will be used to drive 
                                           any DAC outputs connected to this mixer.  
                                           Timebase input to the VCXO Rate Manager 
                                           must be programmed externally by the 
                                           application.  This cannot be changed while 
                                           inputs are running. */
    unsigned outputDelay;               /* Delay for connected outputs (in ms).  Currently, applies to decoder
                                           inputs only. */
} BAPE_MixerSettings;

/***************************************************************************
Summary:
Get Default Mixer Settings
***************************************************************************/
void BAPE_Mixer_GetDefaultSettings(
    BAPE_MixerSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Open a mixer
***************************************************************************/
BERR_Code BAPE_Mixer_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_MixerSettings *pSettings,
    BAPE_MixerHandle *pHandle               /* [out] */
    );

/***************************************************************************
Summary:
Close a mixer
***************************************************************************/
void BAPE_Mixer_Close(
    BAPE_MixerHandle handle
    );

/***************************************************************************
Summary:
Add Mixer Input 
 
Description: 
Inputs to a mixer can not be modified while any previously connected 
inputs are running. 
***************************************************************************/
BERR_Code BAPE_Mixer_AddInput(
    BAPE_MixerHandle handle,
    BAPE_MixerInput input,
    bool makeMaster                /* If true, this will be the master input for sample rate purposes */
    );

/***************************************************************************
Summary:
Remove Mixer Input 
 
Description: 
Inputs to a mixer can not be modified while any previously connected 
inputs are running. 
***************************************************************************/
BERR_Code BAPE_Mixer_RemoveInput(
    BAPE_MixerHandle handle,
    BAPE_MixerInput input
    );

/***************************************************************************
Summary:
Remove All Mixer Inputs
 
Description: 
Inputs to a mixer can not be modified while any previously connected 
inputs are running. 
***************************************************************************/
BERR_Code BAPE_Mixer_RemoveAllInputs(
    BAPE_MixerHandle handle
    );

/***************************************************************************
Summary:
Add Mixer Output
 
Description: 
Outputs from a mixer can not be modified while any previously connected 
inputs are running unless BAPE_MixerSettings.useDummySink is set to true.
***************************************************************************/
BERR_Code BAPE_Mixer_AddOutput(
    BAPE_MixerHandle handle,
    BAPE_MixerOutput output
    );

/***************************************************************************
Summary:
Remove Mixer Output
 
Description: 
Outputs from a mixer can not be modified while any previously connected 
inputs are running unless BAPE_MixerSettings.useDummySink is set to true.
***************************************************************************/
BERR_Code BAPE_Mixer_RemoveOutput(
    BAPE_MixerHandle handle,
    BAPE_MixerOutput output
    );

/***************************************************************************
Summary:
Remove All Mixer Outputs
 
Description: 
Outputs from a mixer can not be modified while any previously connected 
inputs are running unless BAPE_MixerSettings.useDummySink is set to true. 
***************************************************************************/
BERR_Code BAPE_Mixer_RemoveAllOutputs(
    BAPE_MixerHandle handle
    );

/***************************************************************************
Summary:
Input Volume Settings
***************************************************************************/
typedef struct BAPE_MixerInputVolume
{
    int32_t coefficients[BAPE_Channel_eMax][BAPE_Channel_eMax];     /* Entries in this table reflect scaling from the input channel to the output channel.
                                                                       The first index is the input channel and the second index is the output channel.
                                                                       Default is to have BAPE_VOLUME_NORMAL for each [n][n] coefficient and 
                                                                       BAPE_VOLUME_MIN for all others.  This maps input channels to the same output channel
                                                                       with no scaling.  You can achieve effects such as a mono mix with these coefficients
                                                                       if desired by setting [Left][Left] to BAPE_VOLUME_NORMAL/2 and [Left][Right] to 
                                                                       BAPE_VOLUME_NORMAL/2, etc.  Mixing is only permitted between channels in the 
                                                                       same channel pair, so for example you can blend left and right, but not left and 
                                                                       center.  Values are specified in 5.23 2' complement integers.  These settings 
                                                                       are ignored for compressed inputs. */
    bool muted;                   /* Mute input data if true */
} BAPE_MixerInputVolume;

/***************************************************************************
Summary:
Get Input Volume Settings
***************************************************************************/
BERR_Code BAPE_Mixer_GetInputVolume(
    BAPE_MixerHandle mixer,
    BAPE_MixerInput input,
    BAPE_MixerInputVolume *pVolume      /* [out] */
    );

/***************************************************************************
Summary:
Set Input Volume Settings 
 
Description: 
Because inputs can be connected to more than one mixer, input volume is not 
persistent after an input is removed from a mixer. 
***************************************************************************/
BERR_Code BAPE_Mixer_SetInputVolume(
    BAPE_MixerHandle mixer,
    BAPE_MixerInput input,
    const BAPE_MixerInputVolume *pVolume
    );

/***************************************************************************
Summary:
Output Volume Settings
***************************************************************************/
typedef struct BAPE_OutputVolume
{
    uint32_t volume[BAPE_Channel_eMax];         /* Output volume scaling per output channel.  Default is BAPE_VOLUME_NORMAL for all channels.  
                                                   Ignored for compressed data.  Values are specified in 5.23 integers, so 0x800000 corresponds
                                                   to unity (BAPE_VOLUME_NORMAL). */
    bool muted;                                 /* Mute all output channels if true. */
} BAPE_OutputVolume;

/***************************************************************************
Summary:
Get Output Volume Settings
***************************************************************************/
BERR_Code BAPE_GetOutputVolume(
    BAPE_MixerOutput output,
    BAPE_OutputVolume *pVolume      /* [out] */
    );

/***************************************************************************
Summary:
Set Output Volume Settings 
 
Description: 
Output volume settings are persistent and will remain set even when 
outputs are added/removed from mixers. 
***************************************************************************/
BERR_Code BAPE_SetOutputVolume(
    BAPE_MixerOutput output,
    const BAPE_OutputVolume *pVolume
    );
   
/***************************************************************************
Summary:
Output Delay Status
***************************************************************************/
typedef struct BAPE_OutputDelayStatus
{
    unsigned pathDelay;         /* Path delay of all inputs to this output (in ms) */
    unsigned additionalDelay;   /* Delay added in BAPE_MixerSettings (in ms) */
} BAPE_OutputDelayStatus;

/***************************************************************************
Summary:
Get Output Delay Status for lipsync purposes
***************************************************************************/
void BAPE_GetOutputDelayStatus(
    BAPE_MixerOutput output,
    BAPE_OutputDelayStatus *pStatus     /* [out] */
    );
      
/***************************************************************************
Summary:
Get Output Volume Ramp Step
***************************************************************************/
void BAPE_GetOutputVolumeRampStep(
    BAPE_Handle deviceHandle,
    uint32_t *pRampStep                 /* All mixers output output volume is changed by this amount
                                           every Fs while ramping.  Specified in 4.23 format. 
                                           Ignored for compressed data. */
    );
   
/***************************************************************************
Summary:
Set Output Volume Ramp Step
***************************************************************************/
void BAPE_SetOutputVolumeRampStep(
    BAPE_Handle deviceHandle,
    const uint32_t rampStep             /* All mixers output output volume is changed by this amount
                                           every Fs while ramping.  Specified in 4.23 format. 
                                           Ignored for compressed data. */
    );
   
#endif /* #ifndef BAPE_MIXER_H_ */

