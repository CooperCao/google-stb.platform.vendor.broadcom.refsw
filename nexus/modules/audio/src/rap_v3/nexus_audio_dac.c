/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
#include "nexus_audio_module.h"
#include "brap_hifidac.h"

#if BCHP_CHIP == 35230 || BCHP_CHIP == 35125
#include "nexus_gpio.h"
#endif


BDBG_MODULE(nexus_audio_dac);

typedef struct NEXUS_AudioDac
{
    NEXUS_OBJECT(NEXUS_AudioDac);
    bool opened;
    NEXUS_AudioDacSettings settings;
    NEXUS_AudioOutputObject connector;
} NEXUS_AudioDac;

static NEXUS_AudioDac g_dacs[NEXUS_NUM_AUDIO_DACS];

/* Convert an integer to a DAC enum tag */
static BRAP_OutputPort NEXUS_DAC_OUTPUT_PORT(int i)
{
    switch ( i )
    {
    case 0:
        return BRAP_OutputPort_eDac0;
    #if NEXUS_NUM_AUDIO_DACS > 1
    case 1:
        return BRAP_OutputPort_eDac1;
    #if NEXUS_NUM_AUDIO_DACS > 2
    case 2:
        return BRAP_OutputPort_eDac2;
    #endif
    #endif
    default:
        BDBG_ERR(("Unsupported audio DAC id requested (%d)", i));
        BDBG_ASSERT(i < NEXUS_NUM_AUDIO_DACS);
        return BRAP_OutputPort_eMax;
    }
}


#if BCHP_CHIP == 35230 || BCHP_CHIP == 35125
static void NEXUS_AudioDac_P_DisableDacMute(void)
{
    NEXUS_GpioHandle   gpioHandle = NULL;
    NEXUS_GpioSettings gpioSettings;
#if BCHP_CHIP == 35125
    unsigned mutePin = 4;  /* GPIO_4 for DAC_A */
#else
    unsigned mutePin = 3;  /* GPIO_3 for DAC_A */
#endif

    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
    gpioSettings.value = NEXUS_GpioValue_eLow;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
    gpioSettings.interrupt.callback = NULL;
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, mutePin, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandle);
    NEXUS_Gpio_Close(gpioHandle);

#if BCHP_CHIP == 35230
    mutePin = 2;   /* GPIO_2 for DAC_D */
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, mutePin, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandle);
    NEXUS_Gpio_Close(gpioHandle);
#endif
}
#endif


/***************************************************************************
Summary:
    Get default settings for an audio DAC
See Also:

 ***************************************************************************/
void NEXUS_AudioDac_GetDefaultSettings(
    NEXUS_AudioDacSettings *pSettings   /* [out] default settings */
    )
{
    BERR_Code errCode;
    BRAP_OutputPortConfig outputSettings;

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    /* Use DAC 0 */
    errCode = BRAP_GetDefaultOutputConfig(
        g_NEXUS_audioModuleData.hRap,
        BRAP_OutputPort_eDac0,
        &outputSettings);

    if ( errCode )
    {
        errCode=BERR_TRACE(errCode);
    }

    switch ( outputSettings.uOutputPortSettings.sDacSettings.eMuteType )
    {
    case BRAP_OP_DacMuteType_Drive_aaaa:
        pSettings->muteType = NEXUS_AudioDacMuteType_eAaaa;
        break;
    case BRAP_OP_DacMuteType_Drive_5555:
        pSettings->muteType = NEXUS_AudioDacMuteType_e5555;
        break;
    case BRAP_OP_DacMuteType_Use_reg_val:
        pSettings->muteType = NEXUS_AudioDacMuteType_eCustomValue;
        pSettings->muteValueLeft = outputSettings.uOutputPortSettings.sDacSettings.ui32LeftDacOutputVal;
        pSettings->muteValueRight = outputSettings.uOutputPortSettings.sDacSettings.ui32RightDacOutputVal;
        break;
    default:
        BDBG_ERR(("Invalid DAC mute type"));
        BDBG_ASSERT(0);
        return;
    }

    pSettings->testTone.sharedSamples = true;
    pSettings->testTone.numSamplesLeft = 64;
    pSettings->testTone.numSamplesRight = 64;
    pSettings->testTone.sampleRate = 48000;
}

/***************************************************************************
Summary:
    Init the DAC interface
See Also:

 ***************************************************************************/
NEXUS_Error NEXUS_AudioDac_P_Init(void)
{
    int i;
    BERR_Code errCode;

    BDBG_MSG(("Initializing %d DACs", NEXUS_NUM_AUDIO_DACS));

    for ( i = 0; i < NEXUS_NUM_AUDIO_DACS; i++ )
    {
        BRAP_OutputPortConfig outputSettings;

        BRAP_OutputPort port = NEXUS_DAC_OUTPUT_PORT(i);

        BDBG_MSG(("Initializing DAC %d (RAP port %d)", i, port));

        /* setup default config */
        errCode = BRAP_GetDefaultOutputConfig(
            g_NEXUS_audioModuleData.hRap,
            port,
            &outputSettings);

        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        /*
            This "opens" the output port in raptor.
            It must be done for each output you wish to use
        */

        errCode = BRAP_SetOutputConfig(
            g_NEXUS_audioModuleData.hRap,
            &outputSettings);

        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        /* Populate settings with defaults */
        NEXUS_AudioDac_GetDefaultSettings(&g_dacs[i].settings);
    }

#if BCHP_CHIP == 35230 || BCHP_CHIP == 35125
    /* Disable DAC mute */
    NEXUS_AudioDac_P_DisableDacMute();
#endif

    BDBG_MSG(("Successfully initialized DACs"));
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Open an audio DAC
See Also:
    NEXUS_AudioDac_Close
 ***************************************************************************/
NEXUS_AudioDacHandle NEXUS_AudioDac_Open(
    unsigned index,
    const NEXUS_AudioDacSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_AudioDacHandle handle;
    
    /* Sanity check */
    if ( index >= NEXUS_NUM_AUDIO_DACS )
    {
        BDBG_ERR(("Dac Index %u out of range", index));
        return NULL;
    }

    handle = &g_dacs[index];
    if ( handle->opened )
    {
        BDBG_ERR(("Dac %d already open", index));
        return NULL;
    }

    handle->opened = true;
    NEXUS_AUDIO_OUTPUT_INIT(&handle->connector, NEXUS_AudioOutputType_eDac, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioOutput, &handle->connector, Open);
    handle->connector.port = NEXUS_DAC_OUTPUT_PORT(index);
    NEXUS_OBJECT_SET(NEXUS_AudioDac, handle);
    NEXUS_AudioDac_SetSettings(handle, pSettings);

    /* Success */
    return handle;
}


/***************************************************************************
Summary:
    Close an audio DAC
Description:
    Input to the DAC must be removed prior to closing.
See Also:
    NEXUS_AudioDac_Open
    NEXUS_AudioOutput_RemoveInput
    NEXUS_AudioOutput_RemoveAllInputs
 ***************************************************************************/
static void NEXUS_AudioDac_P_Finalizer(
    NEXUS_AudioDacHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDac, handle);
    BDBG_ASSERT(handle->opened);

    NEXUS_AudioOutput_Shutdown(&handle->connector);
    NEXUS_OBJECT_CLEAR(NEXUS_AudioDac, handle);
}

static void NEXUS_AudioDac_P_Release( NEXUS_AudioDacHandle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioOutput, &handle->connector, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioDac, NEXUS_AudioDac_Close);

/***************************************************************************
Summary:
    Get Settings for an audio DAC
See Also:
    NEXUS_AudioDac_SetSettings
 ***************************************************************************/
void NEXUS_AudioDac_GetSettings(
    NEXUS_AudioDacHandle handle,
    NEXUS_AudioDacSettings *pSettings    /* [out] Settings */
    )
{
    /* Sanity Check */
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDac);
    BDBG_ASSERT(NULL != pSettings);

    /* Copy settings */
    *pSettings = handle->settings;
}

/***************************************************************************
Summary:
    Set Settings for an audio DAC
See Also:
    NEXUS_AudioDac_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDac_SetSettings(
    NEXUS_AudioDacHandle handle,
    const NEXUS_AudioDacSettings *pSettings    /* [in] Settings */
    )
{
    BRAP_OutputPortConfig outputSettings, oldSettings;
    NEXUS_AudioDacSettings settings;
    BRAP_OP_DacToneInfo toneInfo;
    NEXUS_Error errCode;

    /* Sanity Check */
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDac);

    if ( NULL == pSettings )
    {
        NEXUS_AudioDac_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    if ( pSettings->volume != handle->settings.volume )
    {
        if ( pSettings->volume > NEXUS_AUDIO_VOLUME_DB_NORMAL || pSettings->volume < NEXUS_AUDIO_VOLUME_DB_MIN )
        {
            BDBG_ERR(("DAC volume out of range.  Must be between %d and %d", NEXUS_AUDIO_VOLUME_DB_NORMAL, NEXUS_AUDIO_VOLUME_DB_MIN));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        /* RAP takes this value in positive 1/100 dB as opposed to the nexus -1/100dB. */
        errCode = BRAP_OP_SetDacVolume(g_NEXUS_audioModuleData.hRap, handle->connector.port, -pSettings->volume);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    errCode = BRAP_GetCurrentOutputConfig(g_NEXUS_audioModuleData.hRap,
                                          handle->connector.port,
                                          &outputSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    oldSettings = outputSettings;
    /* Adjust RAP parameters to match mute type */
    switch ( pSettings->muteType )
    {
    case NEXUS_AudioDacMuteType_eConstantLow:
    case NEXUS_AudioDacMuteType_eConstantHigh:
    case NEXUS_AudioDacMuteType_eSquareWaveOpp:
    case NEXUS_AudioDacMuteType_eSquareWaveSame:
        BDBG_ERR(("Mute mode %d not supported on this chipset", pSettings->muteType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case NEXUS_AudioDacMuteType_eAaaa:
        outputSettings.uOutputPortSettings.sDacSettings.eMuteType = BRAP_OP_DacMuteType_Drive_aaaa;
        break;
    case NEXUS_AudioDacMuteType_e5555:
        outputSettings.uOutputPortSettings.sDacSettings.eMuteType = BRAP_OP_DacMuteType_Drive_5555;
        break;
    case NEXUS_AudioDacMuteType_eCustomValue:
        outputSettings.uOutputPortSettings.sDacSettings.eMuteType = BRAP_OP_DacMuteType_Use_reg_val;
        outputSettings.uOutputPortSettings.sDacSettings.ui32LeftDacOutputVal = pSettings->muteValueLeft;
        outputSettings.uOutputPortSettings.sDacSettings.ui32RightDacOutputVal = pSettings->muteValueRight;
        break;
    default:
        BDBG_ERR(("Invalid DAC mute type"));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Only call SetOutputConfig if something actually changed */
    if ( BKNI_Memcmp(&oldSettings, &outputSettings, sizeof(oldSettings)) )
    {
        errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap,
                                       &outputSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Only set test tone settings if something changed */
    if ( BKNI_Memcmp(&pSettings->testTone, &handle->settings.testTone, sizeof(handle->settings.testTone)) )
    {
        errCode = BRAP_OP_DacGetDefaultTestTone(g_NEXUS_audioModuleData.hRap, handle->connector.port, &toneInfo);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        BDBG_CASSERT(sizeof(toneInfo.aSample) == sizeof(pSettings->testTone.samples));
        BKNI_Memcpy(toneInfo.aSample, pSettings->testTone.samples, sizeof(pSettings->testTone.samples));
        if ( pSettings->testTone.zeroOnLeft )
        {
            if ( pSettings->testTone.zeroOnRight )
            {
                toneInfo.eLROutputControl = BRAP_OP_DacLROutputCtrl_eBothLRZeros;
            }
            else
            {
                toneInfo.eLROutputControl = BRAP_OP_DacLROutputCtrl_eOnlyLZeros;
            }
        }
        else if ( pSettings->testTone.zeroOnRight )
        {
            toneInfo.eLROutputControl = BRAP_OP_DacLROutputCtrl_eOnlyRZeros;
        }
        else
        {
            toneInfo.eLROutputControl = BRAP_OP_DacLROutputCtrl_eNoZeros;
        }
        toneInfo.bSampleBufferMode = pSettings->testTone.sharedSamples;
        toneInfo.uiRightRepeat = pSettings->testTone.numSamplesRight;
        toneInfo.uiLeftRepeat = pSettings->testTone.numSamplesLeft;
        toneInfo.eSamplingRate = NEXUS_AudioModule_P_SampleRate2Avc(pSettings->testTone.sampleRate);
        errCode = BRAP_OP_DacEnableTestTone(g_NEXUS_audioModuleData.hRap, handle->connector.port, pSettings->testTone.enabled, &toneInfo);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    
    handle->settings = *pSettings;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Get the audio connector for an audio DAC
See Also:

 ***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioDac_GetConnector(
    NEXUS_AudioDacHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDac);
    return &handle->connector;
}

