/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/

#include "bape.h"
#include "bape_priv.h"

#if !B_REFSW_MINIMAL
BDBG_MODULE(bape_i2s_multi_output);

#if BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS > 0   /* If no I2S multi outputs, then skip all of this and just put in stub funcs at bottom of file. */

BDBG_OBJECT_ID(BAPE_I2sMultiOutput);

typedef struct BAPE_I2sMultiOutput
{
    BDBG_OBJECT(BAPE_I2sMultiOutput)
    BAPE_Handle deviceHandle;
    BAPE_I2sMultiOutputSettings settings;
    unsigned index;
    unsigned numPorts;
    BAPE_OutputPortObject outputPort[BAPE_ChannelPair_eMax];
    unsigned sampleRate;
    struct
    {
        BAPE_MclkSource mclkSource;
        unsigned pllChannel;    /* only applies if mclkSource refers to a PLL */
        unsigned mclkFreqToFsRatio;
    } mclkInfo;
    bool enabled[BAPE_ChannelPair_eMax];
    char name[21];   /* I2sMulti Output %d */
} BAPE_I2sMultiOutput;

/* Static function prototypes */
static void BAPE_I2sMultiOutput_P_UpdateMclkReg_isr(BAPE_I2sMultiOutputHandle handle );
static BERR_Code BAPE_I2sMultiOutput_P_OpenHw(BAPE_I2sMultiOutputHandle handle);
static BERR_Code BAPE_I2sMultiOutput_P_CloseHw(BAPE_I2sMultiOutputHandle handle);
static BERR_Code BAPE_I2sMultiOutput_P_ApplySettings(BAPE_I2sMultiOutputHandle handle, const BAPE_I2sMultiOutputSettings *pSettings, bool force);

/* Output port callbacks */
static void BAPE_I2sMultiOutput_P_SetTimingParams_isr(BAPE_OutputPort output, unsigned sampleRate, BAVC_Timebase timebase);
static BERR_Code BAPE_I2sMultiOutput_P_Enable(BAPE_OutputPort output);
static void BAPE_I2sMultiOutput_P_Disable(BAPE_OutputPort output);
static void BAPE_I2sMultiOutput_P_SetMclk_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio);


#ifdef BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP10
#define  GET_I2S_MULTI_OP_STREAM_ID(idx, chpair)  (3+(chpair))  /* Holds true across all DTV chips today */
#define  GET_I2S_MULTI_IOP_STREAM_ID(idx, chpair) (3+(chpair))  /* Holds true across all DTV chips today */
#define  GET_I2S_MULTI_NUM_CHANNEL_PAIRS(idx)     (3)           /* Holds true across all DTV chips today */
#else
#error Unknown I2S Multi Stream IDs
#endif

/***************************************************************************
        Public APIs: From bape_output.h
***************************************************************************/

void BAPE_I2sMultiOutput_GetDefaultSettings(
    BAPE_I2sMultiOutputSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->mode           = BAPE_I2sMultiMode_eMultichannel;

    BAPE_I2sOutput_GetDefaultSettings(&pSettings->i2sSettings);
}

/**************************************************************************/

BERR_Code BAPE_I2sMultiOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_I2sMultiOutputSettings *pSettings,
    BAPE_I2sMultiOutputHandle *pHandle             /* [out] */
    )
{
    BERR_Code errCode;
    BAPE_I2sMultiOutputHandle handle;
    BAPE_I2sMultiOutputSettings defaultSettings;
    unsigned i, numPorts, numChannelPairs;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    BDBG_MSG(("%s: Opening I2S Multi Output: %u", __FUNCTION__, index));

    *pHandle = NULL;    /* Set up to return null handle in case of error. */

    if ( index >= BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS )
    {
        BDBG_ERR(("Request to open I2S Multi output %d but chip only has %u I2S Multi outputs", index, BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->i2sMultiOutputs[index] )
    {
        BDBG_ERR(("I2S Multi output %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Init to specified settings */
    if ( NULL == pSettings )
    {
        BAPE_I2sMultiOutput_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    switch ( pSettings->mode )
    {
    default:
        BDBG_ERR(("Invalid I2S Multi Mode %u", pSettings->mode));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    case BAPE_I2sMultiMode_eMultichannel:
        numPorts = 1;
        numChannelPairs = GET_I2S_MULTI_NUM_CHANNEL_PAIRS(index);
        break;
    case BAPE_I2sMultiMode_eStereo:
        numPorts = GET_I2S_MULTI_NUM_CHANNEL_PAIRS(index);
        numChannelPairs = 1;
        break;
    }

    /* Allocate the device structure, then fill in all the fields. */
    handle = BKNI_Malloc(sizeof(BAPE_I2sMultiOutput));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_I2sMultiOutput));
    BDBG_OBJECT_SET(handle, BAPE_I2sMultiOutput);
    handle->deviceHandle = deviceHandle;
    handle->index = index;
    handle->settings.mode = pSettings->mode;
    handle->numPorts = numPorts;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "I2S Multi Output %u", index);
    BDBG_ASSERT(numPorts < BAPE_ChannelPair_eMax);
    for ( i = 0; i < handle->numPorts; i++ )
    {
        BAPE_P_InitOutputPort(&handle->outputPort[i], BAPE_OutputPortType_eI2sMultiOutput, index, handle);
        handle->outputPort[i].maxChannelPairs = numChannelPairs;
        handle->outputPort[i].mclkOutput = BAPE_MclkSource_eNone;
        handle->outputPort[i].compressedSupported = false;
        handle->outputPort[i].muteInMixer = true;
        handle->outputPort[i].fsTiming = false;
        handle->outputPort[i].pllRequired = true;
    /*  handle->outputPort[i].mixerId[]  leave at default */
        handle->outputPort[i].setTimingParams_isr = BAPE_I2sMultiOutput_P_SetTimingParams_isr;
        handle->outputPort[i].setMclk_isr = BAPE_I2sMultiOutput_P_SetMclk_isr;
        handle->outputPort[i].setFs = NULL;         /* not used unless fsTiming == true */
        handle->outputPort[i].setMute = NULL;
        handle->outputPort[i].enable = BAPE_I2sMultiOutput_P_Enable;
        handle->outputPort[i].disable = BAPE_I2sMultiOutput_P_Disable;
    /*  handle->outputPort[i].volume - leave at default */
        handle->outputPort[i].pName = handle->name;

        /* Setup to 48k, muted by default */
        BKNI_EnterCriticalSection();
        BAPE_I2sMultiOutput_P_SetTimingParams_isr(&handle->outputPort[i], 48000, BAVC_Timebase_e0);
        BKNI_LeaveCriticalSection();
    }

    errCode = BAPE_I2sMultiOutput_P_OpenHw(handle);
    if ( errCode )
    {
        BAPE_I2sMultiOutput_Close(handle);
        return BERR_TRACE(errCode);
    }

    /* Initialize hardware before applying settings */
    for ( i = 0; i < handle->numPorts; i++ )
    {
        BKNI_EnterCriticalSection();
        BAPE_I2sMultiOutput_P_SetMclk_isr(&handle->outputPort[i], BAPE_MclkSource_ePll0, 0, BAPE_BASE_PLL_TO_FS_RATIO);
        BKNI_LeaveCriticalSection();    
    }

    errCode = BAPE_I2sMultiOutput_P_ApplySettings(handle, pSettings, true);   /* true => force update of HW */
    if ( errCode )
    {
        BAPE_I2sMultiOutput_Close(handle);
        return BERR_TRACE(errCode);
    }
                        
    *pHandle = handle;
    handle->deviceHandle->i2sMultiOutputs[index] = handle;
    return BERR_SUCCESS;
}
/**************************************************************************/

void BAPE_I2sMultiOutput_Close(
    BAPE_I2sMultiOutputHandle handle
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sMultiOutput);

    for ( i = 0; i < handle->numPorts; i++ )
    {
        /* Make sure we're not still connected to anything */
        if ( handle->outputPort[i].mixer )
        {
            BDBG_ERR(("Cannot close I2S Multi output %p (%s), still connected to mixer %p on connector %u", (void *)handle, handle->name, (void *)handle->outputPort[i].mixer, i));
            BDBG_ASSERT(NULL == handle->outputPort[i].mixer);
            return;
        }
    }

    BAPE_I2sMultiOutput_P_CloseHw(handle);

    BDBG_ASSERT(handle->index < BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS);
    handle->deviceHandle->i2sMultiOutputs[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_I2sMultiOutput);
    BKNI_Free(handle);    
}

/**************************************************************************/

void BAPE_I2sMultiOutput_GetSettings(
    BAPE_I2sMultiOutputHandle handle,
    BAPE_I2sMultiOutputSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sMultiOutput);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

/**************************************************************************/

BERR_Code BAPE_I2sMultiOutput_SetSettings(
    BAPE_I2sMultiOutputHandle handle,
    const BAPE_I2sMultiOutputSettings *pSettings
    )
{
    BERR_Code   errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sMultiOutput);
    BDBG_ASSERT(NULL != pSettings);

    errCode = BAPE_I2sMultiOutput_P_ApplySettings(handle, pSettings, false); /* false => don't force (only update HW for changes) */
    return errCode;
}

/**************************************************************************/

void BAPE_I2sMultiOutput_GetOutputPort(
    BAPE_I2sMultiOutputHandle handle,
    BAPE_OutputPort *pOutputPort        /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sMultiOutput);
    BDBG_ASSERT(NULL != pOutputPort);
    *pOutputPort = &handle->outputPort[0];
}

/**************************************************************************/

void BAPE_I2sMultiOutput_GetStereoOutputPort(
    BAPE_I2sMultiOutputHandle handle,
    BAPE_ChannelPair channelPair,
    BAPE_OutputPort *pOutputPort        /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sMultiOutput);
    BDBG_ASSERT(NULL != pOutputPort);
    if ( channelPair < handle->numPorts )
    {
        *pOutputPort = &handle->outputPort[channelPair];
    }
    else
    {
        BDBG_ERR(("Channel Pair %u not available for I2S Multi Output %u", channelPair, handle->index));
        *pOutputPort = NULL;
    }
}

/***************************************************************************
        BAPE Internal APIs: From bape_fmm_priv.h
***************************************************************************/

BERR_Code BAPE_I2sMultiOutput_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    index,portIndex;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each opened I2sMultiOutput, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( index=0 ; index<BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS ; index++ )
    {
        if ( bapeHandle->i2sMultiOutputs[index] )       /* If this I2sMultiOutput is open... */
        {
            BAPE_I2sMultiOutputHandle hI2sMultiOutput = bapeHandle->i2sMultiOutputs[index];

            /* Put the HW into the generic open state. */
            errCode = BAPE_I2sMultiOutput_P_OpenHw(hI2sMultiOutput);
            if ( errCode ) return BERR_TRACE(errCode);
            
            /* Now apply changes for the settings struct. */
            errCode = BAPE_I2sMultiOutput_P_ApplySettings(hI2sMultiOutput, &hI2sMultiOutput->settings, true);   /* true => force update of HW */
            if ( errCode ) return BERR_TRACE(errCode);

            /* Now restore the dynamic stuff from the values saved in the device struct. */

            for ( portIndex = 0; portIndex < hI2sMultiOutput->numPorts; portIndex++ )
            {
                BKNI_EnterCriticalSection();
                    BAPE_I2sMultiOutput_P_SetTimingParams_isr(&hI2sMultiOutput->outputPort[portIndex], 
                                                         hI2sMultiOutput->sampleRate, 
                                                         0);    /* timebase is unused, 0 is dummy value */
                BKNI_LeaveCriticalSection();
            }

            BKNI_EnterCriticalSection();
                BAPE_I2sMultiOutput_P_UpdateMclkReg_isr(hI2sMultiOutput);
            BKNI_LeaveCriticalSection();
        }
    }
    return errCode;
}

/***************************************************************************
        Private callbacks: Protyped above
***************************************************************************/

static void BAPE_I2sMultiOutput_P_SetTimingParams_isr(BAPE_OutputPort output, unsigned sampleRate, BAVC_Timebase timebase)
{
    BAPE_I2sMultiOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sMultiOutput);
    BSTD_UNUSED(timebase);  /* I2s doesn't care */

    handle->sampleRate = sampleRate;
}

/**************************************************************************/

static BERR_Code BAPE_I2sMultiOutput_P_Enable(BAPE_OutputPort output)
{
    BAPE_I2sMultiOutputHandle handle;
    BAPE_IopStreamSettings streamSettings;
    BERR_Code errCode;
    unsigned streamId;
    unsigned startPair, numPairs, i;
    
    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sMultiOutput);

    for ( startPair = 0; startPair < handle->numPorts; startPair++ )
    {
        if ( output == &handle->outputPort[startPair] )
        {
            break;
        }
    }
    BDBG_ASSERT(startPair < handle->numPorts);
    numPairs = output->mixer->numChannelPairs;
    
    for ( i = startPair; i < (startPair+numPairs); i++ )
    {
        BDBG_ASSERT(false == handle->enabled[i]);

        streamId = GET_I2S_MULTI_IOP_STREAM_ID(handle->index, i);
        BDBG_MSG(("Enabling %s [IOP stream %u]", handle->name, streamId));
        
        /* Write source FCI to IOP */
        BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
        streamSettings.resolution = 24;
        streamSettings.input = output->sourceMixerFci.ids[BAPE_ChannelPair_eLeftRight];         /* Take source FCI provided from mixer */
        errCode = BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        
        /* Write the enable bit in the OP (only stereo) */
        streamId = GET_I2S_MULTI_OP_STREAM_ID(handle->index, i);
        BDBG_MSG(("Writing %x to enable set", (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(streamId)));
        BREG_Write32(handle->deviceHandle->regHandle, 
                     BCHP_AUD_FMM_OP_CTRL_ENABLE_SET,
                     (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(streamId));
    
        handle->enabled[i] = true;
    }

    return BERR_SUCCESS;
}

/**************************************************************************/

static void BAPE_I2sMultiOutput_P_Disable(BAPE_OutputPort output)
{
    BAPE_I2sMultiOutputHandle handle;
    BAPE_IopStreamSettings streamSettings;
    BERR_Code errCode;
    unsigned streamId;
    unsigned startPair, numPairs, i;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sMultiOutput);

    for ( startPair = 0; startPair < handle->numPorts; startPair++ )
    {
        if ( output == &handle->outputPort[startPair] )
        {
            break;
        }
    }
    BDBG_ASSERT(startPair < handle->numPorts);
    numPairs = output->mixer->numChannelPairs;

    for ( i = startPair; i < (startPair+numPairs); i++ )
    {
        streamId = GET_I2S_MULTI_OP_STREAM_ID(handle->index, i);
        BDBG_MSG(("Disabling %s [stream %u]", handle->name, streamId));
    
        /* Clear the enable bit in the OP */
        BDBG_MSG(("Writing %x to enable clear", (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(streamId)));
        BREG_Write32(handle->deviceHandle->regHandle, 
                     BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR,
                     BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_CLEAR, STREAM0_ENA)<<streamId);
    
        /* Reset source FCI to Invalid */
        streamId = GET_I2S_MULTI_IOP_STREAM_ID(handle->index, i);
        BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
        streamSettings.input = BAPE_FCI_ID_INVALID;
        errCode = BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
        BDBG_ASSERT(BERR_SUCCESS == errCode);

        handle->enabled[i] = false;
    }
}

/**************************************************************************/

static void BAPE_I2sMultiOutput_P_SetMclk_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
    BAPE_I2sMultiOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sMultiOutput);

    /* Just do some validation, then set the new values in the handle's mclkInfo struct.
     * Then ...UpdateMclkReg_isr() will take it from  there.
     */
    if ( mclkSource >= BAPE_MclkSource_ePll0 && mclkSource <= BAPE_MclkSource_ePll2 )
    {
        handle->mclkInfo.mclkSource = mclkSource;
        handle->mclkInfo.pllChannel = pllChannel;
    }
    else  if ( mclkSource >= BAPE_MclkSource_eHifidac0 && mclkSource <= BAPE_MclkSource_eHifidac2 )
    {
        handle->mclkInfo.mclkSource = mclkSource;
        handle->mclkInfo.pllChannel = 0;

        BDBG_WRN(("DAC timing source has been selected for I2S output %u.", handle->index));
        BDBG_WRN(("It is strongly recommended to place I2S and DAC outputs on separate mixers."));
    }
    /* mclkSource is not a valid PLL and not a valid DAC... give up. */
    else
    {
        BDBG_ERR(("mclkSource (%u) doesn't refer to a valid PLL or DAC", mclkSource));
        BDBG_ASSERT(false);     /* something went wrong somewhere! */
        return;
    }

    handle->mclkInfo.mclkFreqToFsRatio = mclkFreqToFsRatio;

    BAPE_I2sMultiOutput_P_UpdateMclkReg_isr( handle );

    return;
}

/***************************************************************************
        Private functions: Protyped above
***************************************************************************/

/* Updates the MCLK_CFG based on values in the handle's mclkInfo struct and 
 * the handle's sclkRate setting. 
 */ 
static void BAPE_I2sMultiOutput_P_UpdateMclkReg_isr(BAPE_I2sMultiOutputHandle handle )
{
    /* Use the values in the handle's mclkInfo struct to update the
     * MCLK_CFG register.  */

    BAPE_MclkSource     mclkSource          = handle->mclkInfo.mclkSource;
    unsigned            pllChannel          = handle->mclkInfo.pllChannel;
    unsigned            mclkFreqToFsRatio   = handle->mclkInfo.mclkFreqToFsRatio;

    uint32_t regAddr;
    uint32_t regVal;
    uint32_t mclkRate;
    uint32_t pllclksel;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sMultiOutput);

    /* We need to determine new values for the MCLK_RATE and PLLCLKSEL fields of the
     * AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO<n> register.
     */ 
    switch ( mclkSource )
    {
    /* PLL Timing */
    #if BAPE_CHIP_MAX_PLLS > 0
    case BAPE_MclkSource_ePll0:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI_PLLCLKSEL_PLL0_ch1 + pllChannel;
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 1
    case BAPE_MclkSource_ePll1:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI_PLLCLKSEL_PLL1_ch1 + pllChannel;
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 2
    case BAPE_MclkSource_ePll2:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI_PLLCLKSEL_PLL2_ch1 + pllChannel;
        break;
    #endif
    
    /* DAC Timing */
    #if BAPE_CHIP_MAX_DACS > 0
    case BAPE_MclkSource_eHifidac0:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI_PLLCLKSEL_Hifidac0;
        break;
    #endif
    #if BAPE_CHIP_MAX_DACS > 1
    case BAPE_MclkSource_eHifidac1:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI_PLLCLKSEL_Hifidac1;
        break;
    #endif
    #if BAPE_CHIP_MAX_DACS > 2
    case BAPE_MclkSource_eHifidac2:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI_PLLCLKSEL_Hifidac2;
        break;
    #endif
    
    /* Should never get here */
    default:
        BDBG_ERR(("mclkSource (%u) doesn't refer to a valid PLL or DAC", mclkSource));
        BDBG_ASSERT(false);     /* something went wrong somewhere! */
        return;
    }

    /* Compute the value for the MCLK_RATE */
    switch ( handle->settings.i2sSettings.sclkRate )
    {
    case BAPE_SclkRate_e64Fs:
        mclkRate = mclkFreqToFsRatio / ( 2 * 64 );
        break;

        /* Add code for this after we increase the base PLL rates. */
    case BAPE_SclkRate_e128Fs:
        mclkRate = mclkFreqToFsRatio / ( 2 * 128 );
        break;

    default:
        BDBG_ASSERT(false); /* Should never get here */
        return;
    }

    BDBG_ASSERT(mclkRate > 0); /* If this happens, might need to increase BAPE_BASE_PLL_TO_FS_RATIO */

    /* Choose the register for the appropriate output. */
    regAddr = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI;

    /* Read the register and clear the fields that we're going to fill in. */
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI, PLLCLKSEL)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI, MCLK_RATE));

    /* Fill in the MCLK_RATE and PLLCLKSEL fields. */
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI, MCLK_RATE, mclkRate);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI, PLLCLKSEL, pllclksel);

    /* Then write it to the reg. */
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);
}

/**************************************************************************/

static BERR_Code BAPE_I2sMultiOutput_P_OpenHw(BAPE_I2sMultiOutputHandle handle)
{
    BERR_Code       errCode = BERR_SUCCESS;
    BAPE_Handle     deviceHandle;
    uint32_t        regAddr, regVal;
    unsigned        index;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sMultiOutput);

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    index = handle->index;

    /* Enable the clock and data while opening the output port. */
    regAddr = BCHP_AUD_FMM_OP_CTRL_I2S_CFG_MULTI;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, CLOCK_ENABLE)|
                BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, DATA_ENABLE));
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, CLOCK_ENABLE, Enable);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, DATA_ENABLE, Enable);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

    /* Disable HW grouping */
    regAddr = BCHP_AUD_FMM_OP_CTRL_I2SM_GROUPING;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2SM_GROUPING, GROUP));
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

    /* Turn on the I2S outputs  */
    regAddr = BCHP_AUD_FMM_MISC_SEROUT_OE;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);

    #if BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS == 1
    if (index == 0)
    {
        regVal &= ~(BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,EXTI2SM0_MCLK_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,LRCKM0_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SCLKM0_OE) |                    
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SDATM0_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SDATM1_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SDATM2_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SDATM3_OE));

        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,EXTI2SM0_MCLK_OE, Drive));            
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,LRCKM0_OE, Drive));            
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SCLKM0_OE, Drive));            
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SDATM0_OE, Drive));
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SDATM1_OE, Drive));
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SDATM2_OE, Drive));
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SDATM3_OE, Drive));
    }
    #else
        #error "Need to support more I2S Multi outputs"
    #endif
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);    

    return errCode;
}

/**************************************************************************/

static BERR_Code BAPE_I2sMultiOutput_P_CloseHw(BAPE_I2sMultiOutputHandle handle)
{
    BERR_Code       errCode = BERR_SUCCESS;
    BAPE_Handle     deviceHandle;
    uint32_t        regAddr, regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sMultiOutput);

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    /* Put the I2S outputs into a high-Z state */
    regAddr = BCHP_AUD_FMM_MISC_SEROUT_OE;
    regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
    if (handle->index == 0)
    {
        regVal &= ~(BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,EXTI2SM0_MCLK_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,LRCKM0_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SCLKM0_OE) |                    
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SDATM0_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SDATM1_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SDATM2_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SDATM3_OE));

        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,EXTI2SM0_MCLK_OE, Tristate));            
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,LRCKM0_OE, Tristate));            
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SCLKM0_OE, Tristate));            
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SDATM0_OE, Tristate));
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SDATM1_OE, Tristate));
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SDATM2_OE, Tristate));
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SDATM3_OE, Tristate));
    }
    #if BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS > 1
        #error "Need to support more I2S Multi outputs"
    #endif
    BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);

    /* Disable the I2S Multi clock and data. */
    regAddr = BCHP_AUD_FMM_OP_CTRL_I2S_CFG_MULTI;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, CLOCK_ENABLE)|
                BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, DATA_ENABLE));
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, CLOCK_ENABLE, Disable);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, DATA_ENABLE, Disable);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

    return errCode;
}

/**************************************************************************/

static BERR_Code BAPE_I2sMultiOutput_P_ApplySettings(
    BAPE_I2sMultiOutputHandle handle,
    const BAPE_I2sMultiOutputSettings *pSettings,
    bool force
    )
{
    uint32_t regAddr, regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sMultiOutput);
    BDBG_ASSERT(NULL != pSettings);

    if ( handle->settings.mode != pSettings->mode )
    {
        BDBG_ERR(("Cannot change I2S Multi Mode after opening the output."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Start by reading the appropriate BCHP_AUD_FMM_OP_CTRL_I2S_CFG_MULTI register  for this device. */
    regAddr = BCHP_AUD_FMM_OP_CTRL_I2S_CFG_MULTI;   
    regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, DATA_JUSTIFICATION));
    switch ( pSettings->i2sSettings.justification )
    {
    case BAPE_I2sJustification_eMsbFirst:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, DATA_JUSTIFICATION,MSB);
        break;
    case BAPE_I2sJustification_eLsbFirst:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, DATA_JUSTIFICATION,LSB);
        break;
    default:
        BDBG_ERR(("Invalid value for BAPE_I2sMultiOutputSettings.justification: %u", pSettings->i2sSettings.justification ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, DATA_ALIGNMENT));
    switch ( pSettings->i2sSettings.dataAlignment )
    {
    case BAPE_I2sDataAlignment_eDelayed:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, DATA_ALIGNMENT, Delayed);
        break;
    case BAPE_I2sDataAlignment_eAligned:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, DATA_ALIGNMENT, Aligned);
        break;
    default:
        BDBG_ERR(("Invalid value for BAPE_I2sMultiOutputSettings.dataAlignment: %u", pSettings->i2sSettings.dataAlignment ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, LRCK_POLARITY));
    switch ( pSettings->i2sSettings.lrPolarity )
    {
    case BAPE_I2sLRClockPolarity_eLeftLow:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, LRCK_POLARITY, Low_for_left);
        break;
    case BAPE_I2sLRClockPolarity_eLeftHigh:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, LRCK_POLARITY, High_for_left);
        break;
    default:
        BDBG_ERR(("Invalid value for BAPE_I2sMultiOutputSettings.lrPolarity: %u", pSettings->i2sSettings.lrPolarity ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /*  Since setting the sclkRate involves updating two registers, only bother with this
     *  if somebody is actually changing it.
     */
    if (pSettings->i2sSettings.sclkRate != handle->settings.i2sSettings.sclkRate || force)
    {
        regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, SCLKS_PER_1FS_DIV32));
        switch ( pSettings->i2sSettings.sclkRate )
        {
        case BAPE_SclkRate_e64Fs:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, SCLKS_PER_1FS_DIV32, 2 );
            break;

        case BAPE_SclkRate_e128Fs:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, SCLKS_PER_1FS_DIV32, 4 );
            break;

        default:
            BDBG_ERR(("Invalid value for BAPE_I2sMultiOutputSettings.sclkRate: %u", pSettings->i2sSettings.sclkRate ));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        /* Set the new sclkRate into the settings struct so that ..._UpdateMclkReg_isr()
         * will be able to use it.
         */
        handle->settings.i2sSettings.sclkRate = pSettings->i2sSettings.sclkRate;

        /* Now update the MCLK_CONFIG register.  */
        BKNI_EnterCriticalSection();
        BAPE_I2sMultiOutput_P_UpdateMclkReg_isr( handle );
        BKNI_LeaveCriticalSection();
    }

    /* We only support 24 bits per sample. */
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, BITS_PER_SAMPLE ));
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_MULTI, BITS_PER_SAMPLE, Bitwidth24);

    BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);

    if ( pSettings->i2sSettings.stereoMode != handle->settings.i2sSettings.stereoMode || force )
    {
        regVal = BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_I2SM_CROSSBAR);
        regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT0_L)|
                    BCHP_MASK(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT0_R)|
                    BCHP_MASK(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT1_L)|
                    BCHP_MASK(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT1_R)|
                    BCHP_MASK(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT2_L)|
                    BCHP_MASK(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT2_R));
        switch ( handle->settings.i2sSettings.stereoMode )
        {
        default:
        case BAPE_StereoMode_eLeftRight:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT0_L, In0_l);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT0_R, In0_r);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT1_L, In1_l);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT1_R, In1_r);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT2_L, In2_l);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT2_R, In2_r);
            break;
        case BAPE_StereoMode_eLeftLeft:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT0_L, In0_l);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT0_R, In0_l);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT1_L, In1_l);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT1_R, In1_l);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT2_L, In2_l);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT2_R, In2_l);
            break;
        case BAPE_StereoMode_eRightRight:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT0_L, In0_r);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT0_R, In0_r);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT1_L, In1_r);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT1_R, In1_r);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT2_L, In2_r);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT2_R, In2_r);
            break;
        case BAPE_StereoMode_eRightLeft:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT0_L, In0_r);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT0_R, In0_l);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT1_L, In1_r);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT1_R, In1_l);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT2_L, In2_r);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM_CROSSBAR, OUT2_R, In2_l);
            break;
        }
        BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_I2SM_CROSSBAR, regVal);
        handle->settings.i2sSettings.stereoMode = pSettings->i2sSettings.stereoMode;
    }

    handle->settings = *pSettings;

    return BERR_SUCCESS;
}

/***************************************************************************
    Define stub functions for when there are no I2S outputs. 
***************************************************************************/
#else /* BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS <= 0 */
    /* No I2S outputs, just use stubbed out functions. */


/**************************************************************************/

void BAPE_I2sMultiOutput_GetDefaultSettings(
    BAPE_I2sMultiOutputSettings *pSettings
    )
{
    BSTD_UNUSED(pSettings);
}

/**************************************************************************/

BERR_Code BAPE_I2sMultiOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_I2sMultiOutputSettings *pSettings,
    BAPE_I2sMultiOutputHandle *pHandle             /* [out] */
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);

    *pHandle = NULL;

    return BERR_NOT_SUPPORTED;
}

/**************************************************************************/

void BAPE_I2sMultiOutput_Close(
    BAPE_I2sMultiOutputHandle handle
    )
{
    BSTD_UNUSED(handle);
}

/**************************************************************************/

void BAPE_I2sMultiOutput_GetSettings(
    BAPE_I2sMultiOutputHandle handle,
    BAPE_I2sMultiOutputSettings *pSettings     /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

/**************************************************************************/

BERR_Code BAPE_I2sMultiOutput_SetSettings(
    BAPE_I2sMultiOutputHandle handle,
    const BAPE_I2sMultiOutputSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);

    return BERR_NOT_SUPPORTED;
}

/**************************************************************************/

void BAPE_I2sMultiOutput_GetOutputPort(
    BAPE_I2sMultiOutputHandle handle,
    BAPE_OutputPort *pOutputPort        /* [out] */
    )
{
    BSTD_UNUSED(handle);

    *pOutputPort = NULL;
}

/**************************************************************************/

BERR_Code BAPE_I2sMultiOutput_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BSTD_UNUSED(bapeHandle);
    return BERR_SUCCESS;
}
#endif /* BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS > 0 */
#endif /* !B_REFSW_MINIMAL */
