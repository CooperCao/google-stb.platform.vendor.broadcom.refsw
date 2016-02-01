/***************************************************************************
 *     Copyright (c) 2006-2011, Broadcom Corporation
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

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bchp_hifidac_ctrl0.h"
#include "bchp_hifidac_rm0.h"
#include "bchp_aud_fmm_op_ctrl.h"
/* Currently we only support one DAC, this can be expanded later if we support more */

BDBG_MODULE(bape_dac);

BDBG_OBJECT_ID(BAPE_Dac);

typedef struct BAPE_Dac
{
    BDBG_OBJECT(BAPE_Dac)
    BAPE_Handle deviceHandle;
    BAPE_DacSettings settings;
    unsigned index;
    BAPE_MixerOutputObject connector;
    uint32_t offset;
    BAVC_AudioSamplingRate sampleRate;
    bool enabled;
    char name[7];   /* DAC %d */
} BAPE_Dac;

static void BAPE_Dac_P_SetTimingParams_isr(void *pHandle, BAVC_AudioSamplingRate sampleRate, BAVC_Timebase timebase);
static void BAPE_Dac_P_SetMute(void *pHandle, bool muted, bool sync);
static void BAPE_Dac_P_SetRateManager_isr(BAPE_DacHandle handle, BAVC_AudioSamplingRate sampleRate);
static void BAPE_Dac_P_SyncVolume(BAPE_DacHandle handle);
static void BAPE_Dac_P_Enable(void *pHandle);
static void BAPE_Dac_P_Disable(void *pHandle);

void BAPE_Dac_GetDefaultSettings(
    BAPE_DacSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->stereoMode = BAPE_StereoMode_eLeftRight;
    pSettings->muteType = BAPE_DacMuteType_eAaaa;
    pSettings->customLeftValue = 0x3333;
    pSettings->customRightValue = 0xcccc;
    pSettings->testTone.sharedSamples = true;
    pSettings->testTone.sampleRate = BAVC_AudioSamplingRate_e48k;
}

BERR_Code BAPE_Dac_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_DacSettings *pSettings,
    BAPE_DacHandle *pHandle             /* [out] */
    )
{
    BERR_Code errCode;
    BAPE_DacHandle handle;
    BAPE_DacSettings defaultSettings;
    uint32_t regVal, regAddr, i;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);
    
    *pHandle = NULL;

    if ( index >= BAPE_CHIP_MAX_DACS )
    {
        BDBG_ERR(("Request to open DAC %d but chip only has %u DACs", index, BAPE_CHIP_MAX_DACS));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->dacs[index] )
    {
        BDBG_ERR(("DAC %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle = BKNI_Malloc(sizeof(BAPE_Dac));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_Dac));
    BDBG_OBJECT_SET(handle, BAPE_Dac);
    BAPE_P_InitMixerOutput(&handle->connector, handle);
    handle->deviceHandle = deviceHandle;
    handle->index = index;
    handle->connector.mclkOutput = BAPE_MclkSource_eHifidac0 + index;
    handle->connector.maxChannelPairs = 1;  /* DAC is stero only */
    handle->connector.setTimingParams_isr = BAPE_Dac_P_SetTimingParams_isr;
    handle->connector.setMute = BAPE_Dac_P_SetMute;
    handle->connector.streamId[BAPE_ChannelPair_eLeftRight] = 2;
    handle->connector.enable = BAPE_Dac_P_Enable;
    handle->connector.disable = BAPE_Dac_P_Disable;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "DAC %u", index);
    handle->connector.pName = handle->name;
    /* Currently we only support one DAC, this can be expanded later if we support more */
    handle->offset = 0;
    /* Setup to 48k, muted by default */
    BKNI_EnterCriticalSection();
    BAPE_Dac_P_SetTimingParams_isr(handle, BAVC_AudioSamplingRate_e48k, BAVC_Timebase_e0);
    BKNI_LeaveCriticalSection();
    BAPE_Dac_P_SetMute(handle, false, false);

    /* Init to specified settings */
    if ( NULL == pSettings )
    {
        BAPE_Dac_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* Initialize hardware before applying settings */
    regAddr = BCHP_HIFIDAC_CTRL0_RESET + handle->offset;
    BREG_Write32(deviceHandle->regHandle, regAddr, 1);
    (void)BREG_Read32(deviceHandle->regHandle, regAddr);    /* Sync the register write */
    BREG_Write32(deviceHandle->regHandle, regAddr, 0);
    /* Init test RAM */
    regAddr = BCHP_HIFIDAC_CTRL0_TEST + handle->offset;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(HIFIDAC_CTRL0_TEST, RAM_BUF_SEL);
    regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_TEST, RAM_BUF_SEL, Samples);
    /* Enable the test tone */
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
    regVal &= ~BCHP_MASK(HIFIDAC_CTRL0_TEST, TESTTONE_ENABLE);
    regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_TEST, TESTTONE_ENABLE, Enable);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
    /* Set samples to zero */
    for ( i = 0; i <= BCHP_HIFIDAC_CTRL0_TESTTONE_SAMPLE_i_ARRAY_END; i++ )
    {
        BREG_Write32(deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_TESTTONE_SAMPLE_i_ARRAY_BASE + handle->offset + (i*4), 0);
    }
    regAddr = BCHP_HIFIDAC_CTRL0_TEST + handle->offset;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(HIFIDAC_CTRL0_TEST, RAM_BUF_SEL);
    regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_TEST, RAM_BUF_SEL, Pingpong);
    for ( i = 0; i <= BCHP_HIFIDAC_CTRL0_TESTTONE_SAMPLE_i_ARRAY_END; i++ )
    {
        BREG_Write32(deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_TESTTONE_SAMPLE_i_ARRAY_BASE + handle->offset + (i*4), 0);
    }
    regVal &= ~BCHP_MASK(HIFIDAC_CTRL0_TEST, TESTTONE_ENABLE);
    regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_TEST, TESTTONE_ENABLE, Normal_operation);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
    /* Unmute RFmod by default */
    regAddr = BCHP_HIFIDAC_CTRL0_CONFIG + handle->offset;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(HIFIDAC_CTRL0_CONFIG, RFMOD_MUTE);
    regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, RFMOD_MUTE, Normal_operation);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
    /* Program MAPPER_SOFTMUTE to normal */
    regVal &= ~BCHP_MASK(HIFIDAC_CTRL0_CONFIG, MAPPER_SOFTMUTE);
    regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, MAPPER_SOFTMUTE, Normal_operation);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
    /* Write full volume by default */
    regAddr = BCHP_HIFIDAC_CTRL0_DAC_VOLUME + handle->offset;
    regVal = BCHP_FIELD_DATA(HIFIDAC_CTRL0_DAC_VOLUME, DAC_VOL, 0x1ffff);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

#if BCHP_CHIP == 7408
    /* Adjust DAC polarity correction */
    regAddr = BCHP_HIFIDAC_CTRL0_PEAK_CONFIG + handle->offset;
    regVal = BCHP_FIELD_ENUM(HIFIDAC_CTRL0_PEAK_CONFIG, PEAK_ENABLE, Enable) |
             BCHP_FIELD_ENUM(HIFIDAC_CTRL0_PEAK_CONFIG, PEAK_ROUND_TYPE, Magnitude_truncate);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
    regAddr = BCHP_HIFIDAC_CTRL0_PEAK_GAIN + handle->offset;
    regVal = BCHP_FIELD_DATA(HIFIDAC_CTRL0_PEAK_GAIN, PEAK_GAIN, 0x3e2eb);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
    regAddr = BCHP_HIFIDAC_CTRL0_PEAK_A1 + handle->offset;
    regVal = BCHP_FIELD_DATA(HIFIDAC_CTRL0_PEAK_A1, PEAK_A1, 0x1904f);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
    regAddr = BCHP_HIFIDAC_CTRL0_PEAK_A2 + handle->offset;
    regVal = BCHP_FIELD_DATA(HIFIDAC_CTRL0_PEAK_A2, PEAK_A2, 0x35088);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
#endif

    /* mute */
    BAPE_Dac_P_SetMute(handle, true, false);

    /* unmute the DACALL Control port */
    regAddr = BCHP_HIFIDAC_CTRL0_MUTECTRL + handle->offset;
    regVal = BREG_Read32 (deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL, MUTEALL);
    regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_MUTECTRL, MUTEALL, Ramp_unmute);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);


    errCode = BAPE_Dac_SetSettings(handle, pSettings);
    if ( errCode )
    {
        BAPE_Dac_Close(handle);
        return BERR_TRACE(errCode);
    }

    *pHandle = handle;
    handle->deviceHandle->dacs[index] = handle;
    return BERR_SUCCESS;
}

void BAPE_Dac_Close(
    BAPE_DacHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Dac);

    /* Make sure we're not still connected to anything */
    if ( handle->connector.mixer )
    {
        BDBG_ERR(("Cannot close dac %p (%d), still connected to mixer %p", handle, handle->index, handle->connector.mixer));
        BDBG_ASSERT(NULL == handle->connector.mixer);
        return;
    }

    handle->deviceHandle->dacs[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_Dac);
    BKNI_Free(handle);    
}

void BAPE_Dac_GetSettings(
    BAPE_DacHandle handle,
    BAPE_DacSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Dac);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

BERR_Code BAPE_Dac_SetSettings(
    BAPE_DacHandle handle,
    const BAPE_DacSettings *pSettings
    )
{
    uint32_t regVal, origRegVal, tmp;
    unsigned i;
    BDBG_OBJECT_ASSERT(handle, BAPE_Dac);
    BDBG_ASSERT(NULL != pSettings);
    
    /* Sanity checks before changing any settings */
    if ( pSettings->testTone.enabled && 
         (pSettings->testTone.numSamplesLeft < 1 || pSettings->testTone.numSamplesLeft > 64 ||
          pSettings->testTone.numSamplesRight < 1 || pSettings->testTone.numSamplesRight > 64) )
    {
        BDBG_ERR(("Test tone samples must be between 1 and 64 for both left and right."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Mute before changing this setting */
    origRegVal = regVal = BREG_Read32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_CONFIG + handle->offset);
    regVal &= ~(BCHP_MASK(HIFIDAC_CTRL0_CONFIG, MAPPER_MUTETYPE) |
                BCHP_MASK(HIFIDAC_CTRL0_CONFIG, MAPPER_SELECT_LEFT) |
                BCHP_MASK(HIFIDAC_CTRL0_CONFIG, MAPPER_SELECT_RIGHT));
    switch ( pSettings->muteType )
    {
    case BAPE_DacMuteType_eAaaa:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, MAPPER_MUTETYPE, Drive_aaaa);
        break;
    case BAPE_DacMuteType_e5555:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, MAPPER_MUTETYPE, Drive_5555);
        break;
    case BAPE_DacMuteType_eCustom:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, MAPPER_MUTETYPE, Use_reg_val);

        tmp = BREG_Read32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_DAC_CONST_VAL + handle->offset);
        tmp &= ~(BCHP_MASK(HIFIDAC_CTRL0_DAC_CONST_VAL, LEFT)|BCHP_MASK(HIFIDAC_CTRL0_DAC_CONST_VAL, RIGHT));
        tmp |= BCHP_FIELD_DATA(HIFIDAC_CTRL0_DAC_CONST_VAL, LEFT, pSettings->customLeftValue);
        tmp |= BCHP_FIELD_DATA(HIFIDAC_CTRL0_DAC_CONST_VAL, LEFT, pSettings->customRightValue);
        BREG_Write32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_DAC_CONST_VAL + handle->offset, tmp);
        break;
    default:
        BDBG_ERR(("Invalid DAC mute type %d", pSettings->muteType));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    switch ( pSettings->stereoMode )
    {
    case BAPE_StereoMode_eLeftRight:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, MAPPER_SELECT_LEFT, Left) | 
                  BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, MAPPER_SELECT_RIGHT, Right);
        break;
    case BAPE_StereoMode_eLeftLeft:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, MAPPER_SELECT_LEFT, Left) | 
                  BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, MAPPER_SELECT_RIGHT, Left);
        break;
    case BAPE_StereoMode_eRightLeft:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, MAPPER_SELECT_LEFT, Right) | 
                  BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, MAPPER_SELECT_RIGHT, Left);
        break;
    case BAPE_StereoMode_eRightRight:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, MAPPER_SELECT_LEFT, Right) | 
                  BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, MAPPER_SELECT_RIGHT, Right);
        break;
    }

    if ( regVal != origRegVal )
    {
        /* If the register is going to change, we need to mute first to avoid a pop */
        tmp = BREG_Read32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_DAC_VOLUME + handle->offset);
        BREG_Write32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_DAC_VOLUME + handle->offset, 0);
        BAPE_Dac_P_SyncVolume(handle);

        /* write the new register and unmute */
        BREG_Write32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_CONFIG + handle->offset, regVal);
        BREG_Write32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_DAC_VOLUME + handle->offset, tmp);
    }

    /* Only do this if test tone settings have changed */
    if ( BKNI_Memcmp(&handle->settings.testTone, &pSettings->testTone, sizeof(pSettings->testTone)) )
    {
        /* Setup the test tone if enabled */
        regVal = BREG_Read32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_TEST + handle->offset);
        if ( pSettings->testTone.enabled )
        {
            regVal &= ~(BCHP_MASK(HIFIDAC_CTRL0_TEST, RAM_BUF_SEL)|
                        BCHP_MASK(HIFIDAC_CTRL0_TEST, TESTTONE_LEFT)|
                        BCHP_MASK(HIFIDAC_CTRL0_TEST, TESTTONE_RIGHT)|
                        BCHP_MASK(HIFIDAC_CTRL0_TEST, TESTTONE_SHARE)|
                        BCHP_MASK(HIFIDAC_CTRL0_TEST, TESTTONE_REPEAT_LEFT)|
                        BCHP_MASK(HIFIDAC_CTRL0_TEST, TESTTONE_REPEAT_RIGHT)|
                        BCHP_MASK(HIFIDAC_CTRL0_TEST, TESTTONE_ENABLE));
    
            /* Disable the test tone first if it's enabled */
            BREG_Write32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_TEST + handle->offset, regVal);
    
            regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_TEST, RAM_BUF_SEL, Samples);
            regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_TEST, TESTTONE_ENABLE, Enable);
    
            if ( pSettings->testTone.zeroOnLeft )
                regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_TEST, TESTTONE_LEFT, Output_zero);
            else
                regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_TEST, TESTTONE_LEFT, Output_testtone);
    
            if ( pSettings->testTone.zeroOnRight )
                regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_TEST, TESTTONE_RIGHT, Output_zero);
            else
                regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_TEST, TESTTONE_RIGHT, Output_testtone);
    
            if ( pSettings->testTone.sharedSamples )
                regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_TEST, TESTTONE_SHARE, Share);
            else
                regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_TEST, TESTTONE_SHARE, Dont_share);
    
            regVal |= BCHP_FIELD_DATA(HIFIDAC_CTRL0_TEST, TESTTONE_REPEAT_LEFT, pSettings->testTone.numSamplesLeft-1);
            regVal |= BCHP_FIELD_DATA(HIFIDAC_CTRL0_TEST, TESTTONE_REPEAT_RIGHT, pSettings->testTone.numSamplesRight-1);
    
            /* Enable the tone */
            BREG_Write32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_TEST + handle->offset, regVal);
    
            /* Load samples afterward */
            for ( i = 0; i < 64; i++ )
            {
                BREG_Write32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_TESTTONE_SAMPLE_i_ARRAY_BASE + handle->offset + (i*4),
                             pSettings->testTone.samples[i] & BCHP_MASK(HIFIDAC_CTRL0_TESTTONE_SAMPLE_i, TESTTONE_SAMPLE));
            }

            /* Un-Mute */
            BAPE_Dac_P_SetMute(handle, false, false);
        }
        else
        {
            if ( BCHP_GET_FIELD_DATA(regVal, HIFIDAC_CTRL0_TEST, TESTTONE_ENABLE) )
            {
                if ( !handle->enabled )
                {
                    /* Mute */
                    BAPE_Dac_P_SetMute(handle, true, true);
                }

                /* Clear sample buffer */
                for ( i = 0; i < 64; i++ )
                {
                    BREG_Write32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_TESTTONE_SAMPLE_i_ARRAY_BASE + handle->offset + (i*4), 0);
                }
    
                regVal &= ~BCHP_MASK(HIFIDAC_CTRL0_TEST, TESTTONE_ENABLE);
                BREG_Write32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_TEST + handle->offset, regVal);
                /* Set the rate manager back to the desired rate */
                BKNI_EnterCriticalSection();
                BAPE_Dac_P_SetRateManager_isr(handle, handle->sampleRate);
                BKNI_LeaveCriticalSection();
            }
        }
    }

    handle->settings = *pSettings;

    return BERR_SUCCESS;
}

void BAPE_Dac_GetConnector(
    BAPE_DacHandle handle,
    BAPE_MixerOutput *pConnector        /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Dac);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->connector;
}

static void BAPE_Dac_P_SetRateManager_isr(BAPE_DacHandle handle, BAVC_AudioSamplingRate sampleRate)
{
    uint32_t regAddr, regVal, numerator, denominator, sampleInc, phaseInc;
    unsigned rateNum;

    rateNum = BAPE_P_GetSampleRateValue_isrsafe(sampleRate);

    BDBG_MSG(("Setting DAC %u to %s", handle->index, BAPE_P_GetSampleRateString_isrsafe(sampleRate)));

    regAddr = BCHP_HIFIDAC_CTRL0_CONFIG + handle->offset;
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(HIFIDAC_CTRL0_CONFIG, FIR_MODE);
    if ( rateNum > 96000 )
    {
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, FIR_MODE, Audio_over100kHz);
    }
    else
    {
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, FIR_MODE, Audio_under100kHz);
    }
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);

    /* Program the Oversampling frequency */
    regAddr = BCHP_HIFIDAC_CTRL0_RATEMGRCFG + handle->offset;
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(HIFIDAC_CTRL0_RATEMGRCFG, OVERSAMPLE);
    if ( rateNum <= 48000 )
    {
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_RATEMGRCFG, OVERSAMPLE, Fs256);
    }
    else
    {
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_RATEMGRCFG, OVERSAMPLE, Fs64);
    }
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);

    switch ( sampleRate )
    {
    case BAVC_AudioSamplingRate_e32k:
        numerator = 303;
        denominator = 1024;
        sampleInc = 3;
        phaseInc = 0x0009b583;
        break;
    case BAVC_AudioSamplingRate_e44_1k:
        numerator = 307;
        denominator = 784;
        sampleInc = 2;
        phaseInc = 0x000d6159;
        break;
    default:
        BDBG_WRN(("Unsupported DAC sampling rate (%d).  Defaulting to 48k.", sampleRate));
        /* Fall through */
    case BAVC_AudioSamplingRate_e48k:
        numerator = 101;
        denominator = 512;
        sampleInc = 2;
        phaseInc = 0x000e9045;
        break;
    case BAVC_AudioSamplingRate_e96k:
        numerator = 101;
        denominator = 256;
        sampleInc = 4;
        phaseInc = 0x00074823;
        break;
    }

    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_HIFIDAC_RM0_RATE_RATIO + handle->offset);
    regVal &= ~BCHP_MASK(HIFIDAC_RM0_RATE_RATIO, DENOMINATOR);
    regVal |= BCHP_FIELD_DATA(HIFIDAC_RM0_RATE_RATIO, DENOMINATOR, denominator);
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_HIFIDAC_RM0_RATE_RATIO + handle->offset, regVal);

    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_HIFIDAC_RM0_SAMPLE_INC + handle->offset);
    regVal &= ~(BCHP_MASK(HIFIDAC_RM0_SAMPLE_INC, SAMPLE_INC) | BCHP_MASK(HIFIDAC_RM0_SAMPLE_INC, NUMERATOR));
    regVal |= BCHP_FIELD_DATA(HIFIDAC_RM0_SAMPLE_INC, NUMERATOR, numerator);
    regVal |= BCHP_FIELD_DATA(HIFIDAC_RM0_SAMPLE_INC, SAMPLE_INC, sampleInc);
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_HIFIDAC_RM0_SAMPLE_INC + handle->offset, regVal);

    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_HIFIDAC_RM0_PHASE_INC + handle->offset);
    regVal &= ~BCHP_MASK(HIFIDAC_RM0_PHASE_INC, PHASE_INC);
    regVal |= BCHP_FIELD_DATA(HIFIDAC_RM0_PHASE_INC, PHASE_INC, phaseInc);
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_HIFIDAC_RM0_PHASE_INC + handle->offset, regVal);
}

static void BAPE_Dac_P_SetTimingParams_isr(void *pHandle, BAVC_AudioSamplingRate sampleRate, BAVC_Timebase timebase)
{
    BAPE_DacHandle handle = pHandle;
    uint32_t regVal;
    BDBG_OBJECT_ASSERT(handle, BAPE_Dac);

    /* Update timebase */
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_HIFIDAC_RM0_CONTROL + handle->offset);
    regVal &= ~BCHP_MASK(HIFIDAC_RM0_CONTROL, TIMEBASE);
    switch ( timebase )
    {
    case BAVC_Timebase_e0:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_0);
        break;
    case BAVC_Timebase_e1:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_1);
        break;
    case BAVC_Timebase_e2:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_2);
        break;
    case BAVC_Timebase_e3:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_3);
        break;
    case BAVC_Timebase_e4:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_4);
        break;
    case BAVC_Timebase_e5:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_5);
        break;
    case BAVC_Timebase_e6:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_6);
        break;
    case BAVC_Timebase_e7:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_7);
        break;
    case BAVC_Timebase_e8:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_8);
        break;
    case BAVC_Timebase_e9:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_9);
        break;
    case BAVC_Timebase_e10:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_10);
        break;
    case BAVC_Timebase_e11:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_11);
        break;
    case BAVC_Timebase_e12:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_12);
        break;
    case BAVC_Timebase_e13:
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_RM0_CONTROL, TIMEBASE, TIMEBASE_13);
        break;
    default:
    case BAVC_Timebase_eMax:
        BDBG_ERR(("%s: invalid timebase (%d)", __FUNCTION__, timebase));
        BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_HIFIDAC_RM0_CONTROL + handle->offset, regVal);

    /* Update Sample Rate */
    handle->sampleRate = sampleRate;
    BAPE_Dac_P_SetRateManager_isr(handle, sampleRate);
}

static void BAPE_Dac_P_SetMute(void *pHandle, bool muted, bool sync)
{
    BAPE_DacHandle handle = pHandle;
    uint32_t regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_Dac);

    regVal = BREG_Read32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY + handle->offset);
    regVal &= ~BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL_DACONLY, MUTEDAC);
    if ( muted )
    {
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_MUTECTRL_DACONLY, MUTEDAC, Ramp_mute);
    }
    BREG_Write32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY + handle->offset, regVal);
    if ( sync )
    {
        BAPE_Dac_P_SyncVolume(handle);
    }
}

static void BAPE_Dac_P_SyncVolume(BAPE_DacHandle handle)
{
    int i;
    BDBG_OBJECT_ASSERT(handle, BAPE_Dac);
    for ( i = 0; i < 1000; i++ )
    {
        uint32_t status = BREG_Read32(handle->deviceHandle->regHandle, BCHP_HIFIDAC_CTRL0_STATUS + handle->offset);
        if ( BCHP_GET_FIELD_DATA(status, HIFIDAC_CTRL0_STATUS, VOLUME_AT_TARGET) )
        {
            break;
        }
        BKNI_Sleep(1);
    }
}

static void BAPE_Dac_P_Enable(void *pHandle)
{
    BAPE_DacHandle handle;
    
    handle = pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Dac);
    BDBG_ASSERT(false == handle->enabled);

    BDBG_MSG(("Enabling DAC"));

    /* Write the enable bit in the OP (only stereo) */
    BDBG_MSG(("Writing %x to enable set", (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(handle->connector.streamId[0])));
    BREG_Write32(handle->deviceHandle->regHandle, 
                 BCHP_AUD_FMM_OP_CTRL_ENABLE_SET,
                 (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(handle->connector.streamId[0]));

    BAPE_Dac_P_SetMute(pHandle, false, false);

    handle->enabled = true;
}
static void BAPE_Dac_P_Disable(void *pHandle)
{
    BAPE_DacHandle handle;

    handle = pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Dac);
    /*BDBG_ASSERT(true == handle->enabled);*/

    BDBG_MSG(("Disabling DAC"));

    if ( !handle->settings.testTone.enabled )
    {
        BAPE_Dac_P_SetMute(pHandle, true, true);
    }

    /* Clear the enable bit in the OP */
    BDBG_MSG(("Writing %x to enable clear", (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(handle->connector.streamId[0])));
    BREG_Write32(handle->deviceHandle->regHandle, 
                 BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR,
                 BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_CLEAR, STREAM0_ENA)<<handle->connector.streamId[0]);

    handle->enabled = false;
}

