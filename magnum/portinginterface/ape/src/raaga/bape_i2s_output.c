/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

BDBG_MODULE(bape_i2s_output);
BDBG_FILE_MODULE(bape_fci);

#if BAPE_CHIP_MAX_I2S_OUTPUTS > 0   /* If no I2S outputs, then skip all of this and just put in stub funcs at bottom of file. */

BDBG_OBJECT_ID(BAPE_I2sOutput);

typedef struct BAPE_I2sOutput
{
    BDBG_OBJECT(BAPE_I2sOutput)
    BAPE_Handle deviceHandle;
    BAPE_I2sOutputSettings settings;
    unsigned index;
    BAPE_OutputPortObject outputPort;
    unsigned sampleRate;
    uint32_t offset;
    struct
    {
        BAPE_MclkSource mclkSource;
        unsigned pllChannel;    /* only applies if mclkSource refers to a PLL */
        unsigned mclkFreqToFsRatio;
    } mclkInfo;
    bool enabled;
    char name[14];   /* I2s Output %d */
} BAPE_I2sOutput;


/* Static function prototypes */
static void         BAPE_I2sOutput_P_UpdateMclkReg_isr(BAPE_I2sOutputHandle hI2sOutput );
static BERR_Code    BAPE_I2sOutput_P_OpenHw(BAPE_I2sOutputHandle handle);
static BERR_Code    BAPE_I2sOutput_P_CloseHw(BAPE_I2sOutputHandle handle);
static BERR_Code    BAPE_I2sOutput_P_ApplySettings(BAPE_I2sOutputHandle handle,const BAPE_I2sOutputSettings *pSettings,bool force);

/* Output port callbacks */
static void BAPE_I2sOutput_P_SetTimingParams_isr(BAPE_OutputPort output, unsigned sampleRate, BAVC_Timebase timebase);
static BERR_Code BAPE_I2sOutput_P_Enable(BAPE_OutputPort output);
static void BAPE_I2sOutput_P_Disable(BAPE_OutputPort output);
static void BAPE_I2sOutput_P_SetMclk_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio);

#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_REG_START || defined BCHP_AUD_FMM_IOP_OUT_I2S_0_REG_START
#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_REG_START
    #include "bchp_aud_fmm_iop_out_i2s_stereo_0.h"
    #define BAPE_I2S_IOPOUT_CONSTRUCT(prefix,suffix) prefix##_STEREO_0_##suffix
    #define BAPE_I2S_IOPOUT_VERSION      1
#elif defined BCHP_AUD_FMM_IOP_OUT_I2S_0_REG_START
    #include "bchp_aud_fmm_iop_out_i2s_0.h"
    #define BAPE_I2S_IOPOUT_CONSTRUCT(prefix,suffix) prefix##_0_##suffix
    #define BAPE_I2S_IOPOUT_VERSION      2
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_1_REG_START || defined BCHP_AUD_FMM_IOP_OUT_I2S_1_REG_START
    #if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_1_REG_START
        #include "bchp_aud_fmm_iop_out_i2s_stereo_1.h"
        #define GET_I2S_OFFSET(idx) ((idx)*(BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_1_REG_START-BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_REG_START))
    #elif defined BCHP_AUD_FMM_IOP_OUT_I2S_1_REG_START
        #include "bchp_aud_fmm_iop_out_i2s_1.h"
        #define GET_I2S_OFFSET(idx) ((idx)*(BCHP_AUD_FMM_IOP_OUT_I2S_1_REG_START-BCHP_AUD_FMM_IOP_OUT_I2S_0_REG_START))
    #endif
#endif
#endif

#if defined BAPE_I2S_IOPOUT_VERSION
/* 7249-style chips */
static void         BAPE_I2sOutput_P_UpdateMclkReg_IopOut_isr(BAPE_I2sOutputHandle hI2sOutput );
static BERR_Code    BAPE_I2sOutput_P_OpenHw_IopOut(BAPE_I2sOutputHandle handle);
static BERR_Code    BAPE_I2sOutput_P_CloseHw_IopOut(BAPE_I2sOutputHandle handle);
static BERR_Code    BAPE_I2sOutput_P_ApplySettings_IopOut(BAPE_I2sOutputHandle handle,const BAPE_I2sOutputSettings *pSettings,bool force);
static BERR_Code    BAPE_I2sOutput_P_Enable_IopOut(BAPE_OutputPort output);
static void         BAPE_I2sOutput_P_Disable_IopOut(BAPE_OutputPort output);

#else
/* Legacy STB chips */
static void         BAPE_I2sOutput_P_UpdateMclkReg_Legacy_isr(BAPE_I2sOutputHandle hI2sOutput );
static BERR_Code    BAPE_I2sOutput_P_OpenHw_Legacy(BAPE_I2sOutputHandle handle);
static BERR_Code    BAPE_I2sOutput_P_CloseHw_Legacy(BAPE_I2sOutputHandle handle);
static BERR_Code    BAPE_I2sOutput_P_ApplySettings_Legacy(BAPE_I2sOutputHandle handle,const BAPE_I2sOutputSettings *pSettings,bool force);
static BERR_Code    BAPE_I2sOutput_P_Enable_Legacy(BAPE_OutputPort output);
static void         BAPE_I2sOutput_P_Disable_Legacy(BAPE_OutputPort output);

#if BAPE_CHIP_MAX_I2S_OUTPUTS > 1
    #define  GET_I2S_REG_ADDR2(prefix,idx)         (prefix##0         + (prefix##1         - prefix##0        ) * idx )
    #define  GET_I2S_REG_ADDR3(prefix,idx,suffix)  (prefix##0##suffix + (prefix##1##suffix - prefix##0##suffix) * idx )

    /* STB Chips */
    #define  GET_I2S_OP_STREAM_ID(idx) ((idx) == 0 ? 7 : (idx) == 1 ? 8 : (BDBG_ASSERT(0), 0))
    #define  GET_I2S_IOP_STREAM_ID(idx) GET_I2S_OP_STREAM_ID(idx) /* STB chips have this privilege */

    #define  GET_I2S_CROSSBAR_ADDR(idx) (((idx) * (BCHP_AUD_FMM_OP_CTRL_I2SS1_CROSSBAR - BCHP_AUD_FMM_OP_CTRL_I2SS0_CROSSBAR)) + BCHP_AUD_FMM_OP_CTRL_I2SS0_CROSSBAR);
#else
    #define  GET_I2S_REG_ADDR2(prefix,idx       )  (prefix##0         )
    #define  GET_I2S_REG_ADDR3(prefix,idx,suffix)  (prefix##0##suffix )
    #define  GET_I2S_OP_STREAM_ID(idx) (7)     /* Holds true across all chips today */
    #define  GET_I2S_IOP_STREAM_ID(idx) GET_I2S_OP_STREAM_ID(idx)
    #define  GET_I2S_CROSSBAR_ADDR(idx) (BCHP_AUD_FMM_OP_CTRL_I2SS0_CROSSBAR);
#endif
#endif

#ifndef GET_I2S_OFFSET
    #define GET_I2S_OFFSET(idx) (0)
#endif

/****  #define SETUP_PINMUX_FOR_I2S_OUT_ON_7425 ****/   /* Only defined for testing... changes pinmux settings. */
#ifdef  SETUP_PINMUX_FOR_I2S_OUT_ON_7425
    static void BAPE_I2sOutput_P_SetupPinmuxForI2sTesting( BAPE_Handle deviceHandle );
    #warning "Compiling with special pinmux code to enable I2S outputs"
#endif /* SETUP_PINMUX_FOR_I2S_OUT_ON_7425 */


/***************************************************************************
        Public APIs: From bape_output.h
***************************************************************************/

void BAPE_I2sOutput_GetDefaultSettings(
    BAPE_I2sOutputSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->stereoMode     = BAPE_StereoMode_eLeftRight;
    pSettings->justification  = BAPE_I2sJustification_eMsbFirst;
    pSettings->dataAlignment  = BAPE_I2sDataAlignment_eDelayed;
    pSettings->lrPolarity     = BAPE_I2sLRClockPolarity_eLeftLow;
    pSettings->sclkPolarity   = BAPE_I2sSclkPolarity_eFalling;
    pSettings->sclkRate       = BAPE_SclkRate_e64Fs;
}

/**************************************************************************/

BERR_Code BAPE_I2sOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_I2sOutputSettings *pSettings,
    BAPE_I2sOutputHandle *pHandle             /* [out] */
    )
{
    BERR_Code errCode;
    BAPE_I2sOutputHandle handle;
    BAPE_I2sOutputSettings defaultSettings;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    BDBG_MSG(("%s: Opening I2S Output: %u", __FUNCTION__, index));

    *pHandle = NULL;    /* Set up to return null handle in case of error. */

    if ( index >= BAPE_CHIP_MAX_I2S_OUTPUTS )
    {
        BDBG_ERR(("Request to open I2S output %d but chip only has %u I2S outputs", index, BAPE_CHIP_MAX_I2S_OUTPUTS));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->i2sOutputs[index] )
    {
        BDBG_ERR(("I2S output %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    #ifdef  SETUP_PINMUX_FOR_I2S_OUT_ON_7425
        BAPE_I2sOutput_P_SetupPinmuxForI2sTesting(deviceHandle);
    #endif /* SETUP_PINMUX_FOR_I2S_OUT_ON_7425 */

    /* Allocate the device structure, then fill in all the fields. */
    handle = BKNI_Malloc(sizeof(BAPE_I2sOutput));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Init to specified settings */
    if ( NULL == pSettings )
    {
        BAPE_I2sOutput_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_I2sOutput));
    BDBG_OBJECT_SET(handle, BAPE_I2sOutput);
    handle->deviceHandle = deviceHandle;
    handle->index = index;
    handle->offset = GET_I2S_OFFSET(index);
    BAPE_P_InitOutputPort(&handle->outputPort, BAPE_OutputPortType_eI2sOutput, index, handle);
    handle->outputPort.mclkOutput = BAPE_MclkSource_eNone;
    handle->outputPort.muteInMixer = true;
    handle->outputPort.fsTiming = false;
    handle->outputPort.pllRequired = true;

    /* Setup source capabilities */
    BAPE_FMT_P_EnableSource(&handle->outputPort.capabilities, BAPE_DataSource_eFci);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_ePcmStereo);
    if ( pSettings->allowCompressed )
    {
        BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_eIec61937);
        BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_eIec61937x4);
    }

/*  handle->connector.mixerId[]  leave at default */
    handle->outputPort.setTimingParams_isr = BAPE_I2sOutput_P_SetTimingParams_isr;
    handle->outputPort.setMclk_isr = BAPE_I2sOutput_P_SetMclk_isr;
    handle->outputPort.setFs = NULL;         /* not used unless fsTiming == true */
    handle->outputPort.setMute = NULL;
    handle->outputPort.enable = BAPE_I2sOutput_P_Enable;
    handle->outputPort.disable = BAPE_I2sOutput_P_Disable;
/*  handle->connector.volume - leave at default */
    BKNI_Snprintf(handle->name, sizeof(handle->name), "I2S Output %u", index);
    handle->outputPort.pName = handle->name;

    /* Setup to 48k, muted by default */
    BKNI_EnterCriticalSection();
    BAPE_I2sOutput_P_SetTimingParams_isr(&handle->outputPort, 48000, BAVC_Timebase_e0);
    BKNI_LeaveCriticalSection();
                    
    errCode = BAPE_I2sOutput_P_OpenHw(handle);
    if ( errCode )
    {
        BAPE_I2sOutput_Close(handle);
        return BERR_TRACE(errCode);
    }

    /* Initialize hardware before applying settings */
    BKNI_EnterCriticalSection();
    BAPE_I2sOutput_P_SetMclk_isr(&handle->outputPort, BAPE_MclkSource_ePll0, 0, BAPE_BASE_PLL_TO_FS_RATIO);
    BKNI_LeaveCriticalSection();    
                    
    errCode = BAPE_I2sOutput_P_ApplySettings(handle, pSettings, true);   /* true => force update of HW */
    if ( errCode )
    {
        BAPE_I2sOutput_Close(handle);
        return BERR_TRACE(errCode);
    }
                        
    *pHandle = handle;
    handle->deviceHandle->i2sOutputs[index] = handle;
    return BERR_SUCCESS;
}

/**************************************************************************/

void BAPE_I2sOutput_Close(
    BAPE_I2sOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);

    if ( handle->index >= BAPE_CHIP_MAX_I2S_OUTPUTS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(handle->index < BAPE_CHIP_MAX_I2S_OUTPUTS);
        return;
    }

    /* Make sure we're not still connected to anything */
    if ( handle->outputPort.mixer )
    {
        BDBG_ERR(("Cannot close I2S output %p (%s), still connected to mixer %p", (void *)handle, handle->name, (void *)handle->outputPort.mixer));
        BDBG_ASSERT(NULL == handle->outputPort.mixer);
        return;
    }

    BAPE_I2sOutput_P_CloseHw(handle);

    handle->deviceHandle->i2sOutputs[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_I2sOutput);
    BKNI_Free(handle);    
}

/**************************************************************************/

void BAPE_I2sOutput_GetSettings(
    BAPE_I2sOutputHandle handle,
    BAPE_I2sOutputSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

/**************************************************************************/

BERR_Code BAPE_I2sOutput_SetSettings(
    BAPE_I2sOutputHandle handle,
    const BAPE_I2sOutputSettings *pSettings
    )
{
    BERR_Code   errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);
    BDBG_ASSERT(NULL != pSettings);

    errCode = BAPE_I2sOutput_P_ApplySettings(handle, pSettings, false); /* false => don't force (only update HW for changes) */
    return errCode;
}

/**************************************************************************/

void BAPE_I2sOutput_GetOutputPort(
    BAPE_I2sOutputHandle handle,
    BAPE_OutputPort *pOutputPort        /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);
    BDBG_ASSERT(NULL != pOutputPort);
    *pOutputPort = &handle->outputPort;
}

/***************************************************************************
        BAPE Internal APIs: From bape_fmm_priv.h
***************************************************************************/

BERR_Code BAPE_I2sOutput_P_PrepareForStandby(BAPE_Handle bapeHandle)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    i2sOutputIndex;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each opened I2sOutput, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( i2sOutputIndex=0 ; i2sOutputIndex<BAPE_CHIP_MAX_I2S_OUTPUTS ; i2sOutputIndex++ )
    {
        if ( bapeHandle->i2sOutputs[i2sOutputIndex] )       /* If this I2sOutput is open... */
        {
            BAPE_I2sOutputHandle hI2sOutput = bapeHandle->i2sOutputs[i2sOutputIndex];

            /* Put the HW into the generic open state. */
            BAPE_I2sOutput_P_CloseHw(hI2sOutput);
        }
    }
    return errCode;
}

BERR_Code BAPE_I2sOutput_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    i2sOutputIndex;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each opened I2sOutput, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( i2sOutputIndex=0 ; i2sOutputIndex<BAPE_CHIP_MAX_I2S_OUTPUTS ; i2sOutputIndex++ )
    {
        if ( bapeHandle->i2sOutputs[i2sOutputIndex] )       /* If this I2sOutput is open... */
        {
            BAPE_I2sOutputHandle hI2sOutput = bapeHandle->i2sOutputs[i2sOutputIndex];

            /* Put the HW into the generic open state. */
            errCode = BAPE_I2sOutput_P_OpenHw(hI2sOutput);
            if ( errCode ) return BERR_TRACE(errCode);
            
            /* Now apply changes for the settings struct. */
            errCode = BAPE_I2sOutput_P_ApplySettings(hI2sOutput, &hI2sOutput->settings, true);   /* true => force update of HW */
            if ( errCode ) return BERR_TRACE(errCode);

            /* Now restore the dynamic stuff from the values saved in the device struct. */
            BKNI_EnterCriticalSection();

                BAPE_I2sOutput_P_SetTimingParams_isr(&hI2sOutput->outputPort, 
                                                     hI2sOutput->sampleRate, 
                                                     0);    /* timebase is unused, 0 is dummy value */
                BAPE_I2sOutput_P_UpdateMclkReg_isr(hI2sOutput);
            BKNI_LeaveCriticalSection();
        }
    }
    return errCode;
}


/***************************************************************************
        Private callbacks: Protyped above
***************************************************************************/

static void BAPE_I2sOutput_P_SetTimingParams_isr(BAPE_OutputPort output, unsigned sampleRate, BAVC_Timebase timebase)
{
    BAPE_I2sOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);
    BSTD_UNUSED(timebase);  /* I2s doesn't care */

    handle->sampleRate = sampleRate;
}

/**************************************************************************/

static BERR_Code BAPE_I2sOutput_P_Enable(BAPE_OutputPort output)
{
#ifdef BAPE_I2S_IOPOUT_VERSION
    return BAPE_I2sOutput_P_Enable_IopOut(output);
#else
    return BAPE_I2sOutput_P_Enable_Legacy(output);
#endif
}

/**************************************************************************/

static void BAPE_I2sOutput_P_Disable(BAPE_OutputPort output)
{
#ifdef BAPE_I2S_IOPOUT_VERSION
    BAPE_I2sOutput_P_Disable_IopOut(output);
#else
    BAPE_I2sOutput_P_Disable_Legacy(output);
#endif
}

/**************************************************************************/

static void BAPE_I2sOutput_P_SetMclk_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
    BAPE_I2sOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);

    /* Just do some validation, then set the new values in the handle's mclkInfo struct.
     * Then ...UpdateMclkReg_isr() will take it from  there.
     */
    if (BAPE_MCLKSOURCE_IS_PLL(mclkSource) )
    {
        handle->mclkInfo.mclkSource = mclkSource;
        handle->mclkInfo.pllChannel = pllChannel;
    }
    else  if ( BAPE_MCLKSOURCE_IS_DAC(mclkSource) )
    {
        handle->mclkInfo.mclkSource = mclkSource;
        handle->mclkInfo.pllChannel = 0;

        BDBG_WRN(("DAC timing source has been selected for I2S output %u.", handle->index));
        BDBG_WRN(("It is strongly recommended to place I2S and DAC outputs on separate mixers."));
    }
    else  if ( BAPE_MCLKSOURCE_IS_NCO(mclkSource) )
    {
        handle->mclkInfo.mclkSource = mclkSource;
        handle->mclkInfo.pllChannel = 0;

        BDBG_WRN(("NCO timing source has been selected for I2S output %u.", handle->index));
        BDBG_WRN(("It is strongly recommended to use PLL timing for I2S."));
    }
    /* mclkSource is not a valid PLL and not a valid DAC... give up. */
    else
    {
        BDBG_ERR(("mclkSource (%u) doesn't refer to a valid PLL or DAC", mclkSource));
        BDBG_ASSERT(false);     /* something went wrong somewhere! */
        return;
    }

    handle->mclkInfo.mclkFreqToFsRatio = mclkFreqToFsRatio;

    BAPE_I2sOutput_P_UpdateMclkReg_isr( handle );

    return;
}

/***************************************************************************
        Private functions: Protyped above
***************************************************************************/
static void         BAPE_I2sOutput_P_UpdateMclkReg_isr(BAPE_I2sOutputHandle hI2sOutput )
{
#ifdef BAPE_I2S_IOPOUT_VERSION
    BAPE_I2sOutput_P_UpdateMclkReg_IopOut_isr(hI2sOutput);
#else
    BAPE_I2sOutput_P_UpdateMclkReg_Legacy_isr(hI2sOutput);
#endif
}

static BERR_Code    BAPE_I2sOutput_P_OpenHw(BAPE_I2sOutputHandle handle)
{
#ifdef BAPE_I2S_IOPOUT_VERSION
    return BAPE_I2sOutput_P_OpenHw_IopOut(handle);
#else
    return BAPE_I2sOutput_P_OpenHw_Legacy(handle);
#endif    
}

static BERR_Code    BAPE_I2sOutput_P_CloseHw(BAPE_I2sOutputHandle handle)
{
#ifdef BAPE_I2S_IOPOUT_VERSION
    return BAPE_I2sOutput_P_CloseHw_IopOut(handle);
#else
    return BAPE_I2sOutput_P_CloseHw_Legacy(handle);
#endif    
}

static BERR_Code    BAPE_I2sOutput_P_ApplySettings(BAPE_I2sOutputHandle handle,const BAPE_I2sOutputSettings *pSettings,bool force)
{
#ifdef BAPE_I2S_IOPOUT_VERSION
    return BAPE_I2sOutput_P_ApplySettings_IopOut(handle, pSettings, force);
#else
    return BAPE_I2sOutput_P_ApplySettings_Legacy(handle, pSettings, force);
#endif    
}

#ifdef BAPE_I2S_IOPOUT_VERSION

/* 7429 style chips */
static void BAPE_I2sOutput_P_UpdateMclkReg_IopOut_isr(BAPE_I2sOutputHandle hI2sOutput )
{
    /* Use the values in the handle's mclkInfo struct to update the
     * MCLK_CFG register.  */

    BAPE_MclkSource      mclkSource          = hI2sOutput->mclkInfo.mclkSource;
    unsigned             pllChannel          = hI2sOutput->mclkInfo.pllChannel;
    unsigned             mclkFreqToFsRatio   = hI2sOutput->mclkInfo.mclkFreqToFsRatio;
    BAPE_Reg_P_FieldList regFieldList;

    BDBG_OBJECT_ASSERT(hI2sOutput, BAPE_I2sOutput);

    BAPE_Reg_P_InitFieldList(hI2sOutput->deviceHandle, &regFieldList);

    switch ( mclkSource )
    {
#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0_PLLCLKSEL_PLL0_ch1 || defined BCHP_AUD_FMM_IOP_OUT_I2S_0_MCLK_CFG_0_PLLCLKSEL_PLL0_ch1
    case BAPE_MclkSource_ePll0:
        switch ( pllChannel )
        {
        /*case 0: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0, PLLCLKSEL, PLL0_ch1); break;*/
        case 0: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, PLL0_ch1); break;
        case 1: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, PLL0_ch2); break;
        case 2: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, PLL0_ch3); break;
        default: (void) BERR_TRACE(BERR_NOT_SUPPORTED); break;
        }
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0_PLLCLKSEL_PLL1_ch1 || defined BCHP_AUD_FMM_IOP_OUT_I2S_0_MCLK_CFG_0_PLLCLKSEL_PLL1_ch1
    case BAPE_MclkSource_ePll1:
        switch ( pllChannel )
        {
        case 0: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, PLL1_ch1); break;
        case 1: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, PLL1_ch2); break;
        case 2: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, PLL1_ch3); break;
        default: (void) BERR_TRACE(BERR_NOT_SUPPORTED); break;
        }
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0_PLLCLKSEL_PLL2_ch1 || defined BCHP_AUD_FMM_IOP_OUT_I2S_0_MCLK_CFG_0_PLLCLKSEL_PLL2_ch1
    case BAPE_MclkSource_ePll2:
        switch ( pllChannel )
        {
        case 0: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, PLL2_ch1); break;
        case 1: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, PLL2_ch2); break;
        case 2: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, PLL2_ch3); break;
        default: (void) BERR_TRACE(BERR_NOT_SUPPORTED); break;
        }
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen0 || defined BCHP_AUD_FMM_IOP_OUT_I2S_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen0
    case BAPE_MclkSource_eNco0:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, Mclk_gen0);
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen1 || defined BCHP_AUD_FMM_IOP_OUT_I2S_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen1
    case BAPE_MclkSource_eNco1:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, Mclk_gen1);
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen2 || defined BCHP_AUD_FMM_IOP_OUT_I2S_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen2
    case BAPE_MclkSource_eNco2:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, Mclk_gen2);
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen3 || defined BCHP_AUD_FMM_IOP_OUT_I2S_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen3
    case BAPE_MclkSource_eNco3:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, Mclk_gen3);
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen4 || defined BCHP_AUD_FMM_IOP_OUT_I2S_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen4
    case BAPE_MclkSource_eNco4:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, Mclk_gen4);
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen5 || defined BCHP_AUD_FMM_IOP_OUT_I2S_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen5
    case BAPE_MclkSource_eNco5:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, Mclk_gen5);
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen6 || defined BCHP_AUD_FMM_IOP_OUT_I2S_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen6
    case BAPE_MclkSource_eNco6:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), PLLCLKSEL, Mclk_gen6);
        break;
#endif
    default:
        BDBG_ERR(("Unsupported clock source %u for MAI %u", mclkSource, hI2sOutput->index));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }

#if defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0_MCLK_RATE_MCLK_128fs_SCLK_64fs
    switch ( mclkFreqToFsRatio )
    {
    case 128: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), MCLK_RATE, MCLK_128fs_SCLK_64fs); break;
    case 256: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), MCLK_RATE, MCLK_256fs_SCLK_64fs); break;
    case 384: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), MCLK_RATE, MCLK_384fs_SCLK_64fs); break;
    case 512: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0), MCLK_RATE, MCLK_512fs_SCLK_64fs); break;
    default:
        BDBG_ERR(("Unsupported MCLK Rate of %uFs", mclkFreqToFsRatio));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        break;
    }
#elif defined BCHP_AUD_FMM_IOP_OUT_I2S_0_MCLK_CFG_0_MCLK_RATE_Divide_By_4
    {
        unsigned mclk_rate = 0;
        /* MCLK_RATE = mclkFreqToFsRatio / (2*32*SCLKS_PER_1FS_DIV32) */
        switch ( hI2sOutput->settings.sclkRate )
        {
        /*case BAPE_SclkRate_e32Fs:
            mclk_rate = mclkFreqToFsRatio / (2*32*1);
            break;*/
        case BAPE_SclkRate_e64Fs:
            mclk_rate = mclkFreqToFsRatio / (2*32*2);
            break;
        case BAPE_SclkRate_e128Fs:
            mclk_rate = mclkFreqToFsRatio / (2*32*4);
            break;
        default:
            BDBG_ERR(("Invalid value for BAPE_I2sOutputSettings.sclkRate: %u", hI2sOutput->settings.sclkRate ));
            BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        if ( mclk_rate != 0 )
        {
            BAPE_Reg_P_AddToFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S, MCLK_CFG_0), MCLK_RATE, mclk_rate);
        }
    }
#else
    #warning "UNSUPPORTED CHIP - update this code"
#endif
    BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(BCHP_AUD_FMM_IOP_OUT_I2S,MCLK_CFG_0) + hI2sOutput->offset);
}

static BERR_Code BAPE_I2sOutput_P_OpenHw_IopOut(BAPE_I2sOutputHandle handle)
{
    BAPE_Reg_P_FieldList regFieldList;
    BERR_Code            errCode = BERR_SUCCESS;
    BAPE_Handle          deviceHandle;
    unsigned             index;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    index = handle->index;

    /* Enable the clock and data while opening the output port. Never disable it */
    BAPE_Reg_P_InitFieldList(deviceHandle, &regFieldList);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CFG), CLOCK_ENABLE, Enable);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CFG), DATA_ENABLE, Enable);
    BAPE_Reg_P_ApplyFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(BCHP_AUD_FMM_IOP_OUT_I2S,I2S_CFG) + handle->offset);

    /* Turn on the I2S outputs  */
    BAPE_Reg_P_InitFieldList(deviceHandle, &regFieldList);    

    if (index == 0)
    {
        #if defined BCHP_AUD_MISC_SEROUT_OE_SDATS0_OE_Drive
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, SDATS0_OE, Drive);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, SCLKS0_OE, Drive);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, LRCKS0_OE, Drive);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, MCLKS0_OE, Drive);
        #elif defined BCHP_AUD_MISC_SEROUT_OE_OUT_I2S0_SDAT_OE_Drive
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S0_SDAT_OE, Drive);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S0_MT_SCLK_OE, Drive);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S0_MT_LRCK_OE, Drive);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S0_MT_MCLK_OE, Drive);
        #else
        #warning "UNSUPPORTED CHIP - update this code"
        #endif
    }
    #if BAPE_CHIP_MAX_I2S_OUTPUTS > 1
    else if(index == 1)
    {
        #if defined BCHP_AUD_MISC_SEROUT_OE_SDATS1_OE_Drive
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, SDATS1_OE, Drive);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, SCLKS1_OE, Drive);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, LRCKS1_OE, Drive);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, MCLKS1_OE, Drive);
        #elif defined BCHP_AUD_MISC_SEROUT_OE_OUT_I2S1_SDAT_OE_Drive
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S1_SDAT_OE, Drive);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S1_MT_SCLK_OE, Drive);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S1_MT_LRCK_OE, Drive);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S1_MT_MCLK_OE, Drive);
        #else
        #warning "UNSUPPORTED CHIP - update this code"
        #endif
    }
    #endif 
    #if BAPE_CHIP_MAX_I2S_OUTPUTS > 2
        #error "Need to support more I2S outputs"
    #endif 

    BAPE_Reg_P_ApplyFieldList(&regFieldList, BCHP_AUD_MISC_SEROUT_OE);

    return errCode;
}

static BERR_Code BAPE_I2sOutput_P_CloseHw_IopOut(BAPE_I2sOutputHandle handle)
{
    BAPE_Reg_P_FieldList regFieldList;
    BERR_Code            errCode = BERR_SUCCESS;
    BAPE_Handle          deviceHandle;
    unsigned             index;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    index = handle->index;

    /* Turn off the I2S outputs  */
    BAPE_Reg_P_InitFieldList(deviceHandle, &regFieldList);    

    if (index == 0)
    {
        #if defined BCHP_AUD_MISC_SEROUT_OE_SDATS0_OE_Drive
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, SDATS0_OE, Tristate);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, SCLKS0_OE, Tristate);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, LRCKS0_OE, Tristate);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, MCLKS0_OE, Tristate);
        #elif defined BCHP_AUD_MISC_SEROUT_OE_OUT_I2S0_SDAT_OE_Drive
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S0_SDAT_OE, Tristate);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S0_MT_SCLK_OE, Tristate);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S0_MT_LRCK_OE, Tristate);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S0_MT_MCLK_OE, Tristate);
        #else
        #warning "UNSUPPORTED CHIP - update this code"
        #endif
    }
    #if BAPE_CHIP_MAX_I2S_OUTPUTS > 1
    else if(index == 1)
    {
        #if defined BCHP_AUD_MISC_SEROUT_OE_SDATS1_OE_Drive
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, SDATS1_OE, Tristate);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, SCLKS1_OE, Tristate);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, LRCKS1_OE, Tristate);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, MCLKS1_OE, Tristate);
        #elif defined BCHP_AUD_MISC_SEROUT_OE_OUT_I2S1_SDAT_OE_Drive
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S1_SDAT_OE, Tristate);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S1_MT_SCLK_OE, Tristate);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S1_MT_LRCK_OE, Tristate);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_MISC_SEROUT_OE, OUT_I2S1_MT_MCLK_OE, Tristate);
        #else
        #warning "UNSUPPORTED CHIP - update this code"
        #endif
    }
    #endif 
    #if BAPE_CHIP_MAX_I2S_OUTPUTS > 2
        #error "Need to support more I2S outputs"
    #endif 

    BAPE_Reg_P_ApplyFieldList(&regFieldList, BCHP_AUD_MISC_SEROUT_OE);

    /* Enable the clock and data while opening the output port. Never disable it */
    BAPE_Reg_P_InitFieldList(deviceHandle, &regFieldList);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CFG), CLOCK_ENABLE, Disable);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CFG), DATA_ENABLE, Disable);
    BAPE_Reg_P_ApplyFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(BCHP_AUD_FMM_IOP_OUT_I2S,I2S_CFG) + handle->offset);

    return errCode;
}

static BERR_Code BAPE_I2sOutput_P_ApplySettings_IopOut(
    BAPE_I2sOutputHandle handle,
    const BAPE_I2sOutputSettings *pSettings,
    bool force
    )
{
    BAPE_Reg_P_FieldList regFieldList;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);
    BDBG_ASSERT(NULL != pSettings);

    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);

    switch ( pSettings->justification )
    {
    case BAPE_I2sJustification_eMsbFirst:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CFG), DATA_JUSTIFICATION, MSB);
        break;
    case BAPE_I2sJustification_eLsbFirst:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CFG), DATA_JUSTIFICATION, LSB);
        break;
    default:
        BDBG_ERR(("Invalid value for BAPE_I2sOutputSettings.justification: %u", pSettings->justification ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    switch ( pSettings->dataAlignment )
    {
    case BAPE_I2sDataAlignment_eDelayed:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CFG), DATA_ALIGNMENT, Delayed);
        break;
    case BAPE_I2sDataAlignment_eAligned:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CFG), DATA_ALIGNMENT, Aligned);
        break;
    default:
        BDBG_ERR(("Invalid value for BAPE_I2sOutputSettings.dataAlignment: %u", pSettings->dataAlignment ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    switch ( pSettings->lrPolarity )
    {
    case BAPE_I2sLRClockPolarity_eLeftLow:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CFG), LRCK_POLARITY, Low_for_left);
        break;
    case BAPE_I2sLRClockPolarity_eLeftHigh:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CFG), LRCK_POLARITY, High_for_left);
        break;
    default:
        BDBG_ERR(("Invalid value for BAPE_I2sOutputSettings.lrPolarity: %u", pSettings->lrPolarity ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /*  Since setting the sclkRate involves updating two registers, only bother with this
     *  if somebody is actually changing it.
     */
    if (pSettings->sclkRate != handle->settings.sclkRate || force)
    {
        switch ( pSettings->sclkRate )
        {
        case BAPE_SclkRate_e64Fs:
            BAPE_Reg_P_AddToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CFG), SCLKS_PER_1FS_DIV32, 2);
            break;
        case BAPE_SclkRate_e128Fs:
            BAPE_Reg_P_AddToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CFG), SCLKS_PER_1FS_DIV32, 4);
            break;
        default:
            BDBG_ERR(("Invalid value for BAPE_I2sOutputSettings.sclkRate: %u", pSettings->sclkRate ));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        /* Set the new sclkRate into the settings struct so that ..._UpdateMclkReg_isr()
         * will be able to use it.
         */
        handle->settings.sclkRate = pSettings->sclkRate;

        /* Now update the MCLK_CONFIG register.  */
        BKNI_EnterCriticalSection();
        BAPE_I2sOutput_P_UpdateMclkReg_isr( handle );
        BKNI_LeaveCriticalSection();
    }

    /* We only support 24 bits per sample. */
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CFG), BITS_PER_SAMPLE, Bitwidth24);

    /* Apply register changes */
    BAPE_Reg_P_ApplyFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(BCHP_AUD_FMM_IOP_OUT_I2S,I2S_CFG) + handle->offset);

    if ( pSettings->stereoMode != handle->settings.stereoMode || force )
    {
        BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
        switch ( pSettings->stereoMode )
        {
        default:
        case BAPE_StereoMode_eLeftRight:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CROSSBAR), OUT_L, In_l);
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CROSSBAR), OUT_R, In_r);
            break;
        case BAPE_StereoMode_eLeftLeft:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CROSSBAR), OUT_L, In_l);
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CROSSBAR), OUT_R, In_l);
            break;
        case BAPE_StereoMode_eRightRight:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CROSSBAR), OUT_L, In_r);
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CROSSBAR), OUT_R, In_r);
            break;
        case BAPE_StereoMode_eRightLeft:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CROSSBAR), OUT_L, In_r);
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,I2S_CROSSBAR), OUT_R, In_l);
            break;            
        }

        /* Apply register changes */
        BAPE_Reg_P_ApplyFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(BCHP_AUD_FMM_IOP_OUT_I2S,I2S_CROSSBAR) + handle->offset);

        handle->settings.stereoMode = pSettings->stereoMode;
    }

    handle->settings = *pSettings;

    return BERR_SUCCESS;
}

static BERR_Code BAPE_I2sOutput_P_Enable_IopOut(BAPE_OutputPort output)
{
    BAPE_I2sOutputHandle handle;
    BAPE_Reg_P_FieldList regFieldList;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);

    BDBG_ASSERT(false == handle->enabled);

    BDBG_MODULE_MSG(bape_fci, ("Linking I2S OUTPUT FCI Source"));
    BDBG_MODULE_MSG(bape_fci, ("  fci %x -> I2S %d [%d]", output->sourceMixerFci.ids[0], handle->index, 0));

    /* Setup Interface */
    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
    BAPE_Reg_P_AddToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,STREAM_CFG_0), CHANNEL_GROUPING, 1);
    BAPE_Reg_P_AddToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,STREAM_CFG_0), GROUP_ID, 0);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,STREAM_CFG_0), STREAM_BIT_RESOLUTION, Res_24_Bit);
    BAPE_Reg_P_AddToFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,STREAM_CFG_0), FCI_ID, output->sourceMixerFci.ids[BAPE_ChannelPair_eLeftRight]);
    BAPE_Reg_P_ApplyFieldList(&regFieldList, BAPE_I2S_IOPOUT_CONSTRUCT(BCHP_AUD_FMM_IOP_OUT_I2S,STREAM_CFG_0) + handle->offset);

    /* Enable the interface */
    BAPE_Reg_P_UpdateField(handle->deviceHandle, 
                           BAPE_I2S_IOPOUT_CONSTRUCT(BCHP_AUD_FMM_IOP_OUT_I2S,STREAM_CFG_0) + handle->offset,
                           BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,STREAM_CFG_0),
                           ENA,
                           1);

    handle->enabled = true;

    return BERR_SUCCESS;
}

/**************************************************************************/

static void BAPE_I2sOutput_P_Disable_IopOut(BAPE_OutputPort output)
{
    BAPE_I2sOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);

    /* Disable the interface */
    BAPE_Reg_P_UpdateField(handle->deviceHandle, 
                           BAPE_I2S_IOPOUT_CONSTRUCT(BCHP_AUD_FMM_IOP_OUT_I2S,STREAM_CFG_0) + handle->offset,
                           BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,STREAM_CFG_0),
                           ENA,
                           0);

    /* Clear the input value */
    BAPE_Reg_P_UpdateField(handle->deviceHandle, 
                           BAPE_I2S_IOPOUT_CONSTRUCT(BCHP_AUD_FMM_IOP_OUT_I2S,STREAM_CFG_0) + handle->offset,
                           BAPE_I2S_IOPOUT_CONSTRUCT(AUD_FMM_IOP_OUT_I2S,STREAM_CFG_0),
                           FCI_ID,
                           BAPE_FCI_ID_INVALID);


    handle->enabled = false;
}

#else
/* Legacy STB chips */
static void BAPE_I2sOutput_P_UpdateMclkReg_Legacy_isr(BAPE_I2sOutputHandle hI2sOutput )
{
    /* Use the values in the handle's mclkInfo struct to update the
     * MCLK_CFG register.  */

    BAPE_MclkSource     mclkSource          = hI2sOutput->mclkInfo.mclkSource;
    unsigned            pllChannel          = hI2sOutput->mclkInfo.pllChannel;
    unsigned            mclkFreqToFsRatio   = hI2sOutput->mclkInfo.mclkFreqToFsRatio;

    uint32_t regAddr;
    uint32_t regVal;
    uint32_t mclkRate;
    uint32_t pllclksel;

    BDBG_OBJECT_ASSERT(hI2sOutput, BAPE_I2sOutput);

    /* We need to determine new values for the MCLK_RATE and PLLCLKSEL fields of the
     * AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO<n> register.
     */ 
    switch ( mclkSource )
    {
    /* PLL Timing */
    #if BAPE_CHIP_MAX_PLLS > 0
    case BAPE_MclkSource_ePll0:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0_PLLCLKSEL_PLL0_ch1 + pllChannel;
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 1
    case BAPE_MclkSource_ePll1:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0_PLLCLKSEL_PLL1_ch1 + pllChannel;
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 2
    case BAPE_MclkSource_ePll2:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0_PLLCLKSEL_PLL2_ch1 + pllChannel;
        break;
    #endif
    
    /* DAC Timing */
    #if BAPE_CHIP_MAX_DACS > 0
    case BAPE_MclkSource_eHifidac0:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0_PLLCLKSEL_Hifidac0;
        break;
    #endif
    #if BAPE_CHIP_MAX_DACS > 1
    case BAPE_MclkSource_eHifidac1:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0_PLLCLKSEL_Hifidac1;
        break;
    #endif
    #if BAPE_CHIP_MAX_DACS > 2
    case BAPE_MclkSource_eHifidac2:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0_PLLCLKSEL_Hifidac2;
        break;
    #endif
    
    /* NCO Timing */
    #if BAPE_CHIP_MAX_NCOS > 0
    case BAPE_MclkSource_eNco0:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0_PLLCLKSEL_Mclk_gen0;
        break;
    #endif
    #if BAPE_CHIP_MAX_NCOS > 1
    case BAPE_MclkSource_eNco1:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0_PLLCLKSEL_Mclk_gen1;
        break;
    #endif
    #if BAPE_CHIP_MAX_NCOS > 2
    case BAPE_MclkSource_eNco2:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0_PLLCLKSEL_Mclk_gen2;
        break;
    #endif
    
    /* Should never get here */
    default:
        BDBG_ERR(("mclkSource (%u) doesn't refer to a valid PLL or DAC", mclkSource));
        BDBG_ASSERT(false);     /* something went wrong somewhere! */
        return;
    }

    /* Compute the value for the MCLK_RATE */
    switch ( hI2sOutput->settings.sclkRate )
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

    /* We have the values in "pllclksel" and "mclkRate", now write to the registers. */

    /* Choose the register for the appropriate output. */
    regAddr = GET_I2S_REG_ADDR2(BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO, hI2sOutput->index);

    /* Read the register and clear the fields that we're going to fill in. */
    regVal = BREG_Read32_isr(hI2sOutput->deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0, PLLCLKSEL)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0, MCLK_RATE));

    /* Fill in the MCLK_RATE and PLLCLKSEL fields. */
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0, MCLK_RATE, mclkRate);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0, PLLCLKSEL, pllclksel);

    /* Then write it to the reg. */
    BREG_Write32_isr(hI2sOutput->deviceHandle->regHandle, regAddr, regVal);
}

/**************************************************************************/

static BERR_Code BAPE_I2sOutput_P_OpenHw_Legacy(BAPE_I2sOutputHandle handle)
{
    BERR_Code       errCode = BERR_SUCCESS;
    BAPE_Handle     deviceHandle;
    uint32_t        regAddr, regVal;
    unsigned        index;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    index = handle->index;

    /* Enable the clock and data while opening the output port. Never disable it */
    regAddr = GET_I2S_REG_ADDR3(BCHP_AUD_FMM_OP_CTRL_I2SS, index, _CFG);
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2SS0_CFG, CLOCK_ENABLE)|
                BCHP_MASK(AUD_FMM_OP_CTRL_I2SS0_CFG, DATA_ENABLE));
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CFG, CLOCK_ENABLE, Enable);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CFG, DATA_ENABLE, Enable);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

    /* Turn on the I2S outputs  */
    regAddr = BCHP_AUD_FMM_MISC_SEROUT_OE;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);

    if (index == 0)
    {
        regVal &= ~(
            #ifdef BCHP_AUD_FMM_MISC_SEROUT_OE_EXTI2SS0_MCLK_OE_MASK
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,EXTI2SS0_MCLK_OE) |
            #endif
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,LRCKS0_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SCLKS0_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SDATS0_OE));

        #ifdef BCHP_AUD_FMM_MISC_SEROUT_OE_EXTI2SS0_MCLK_OE_MASK
            regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,EXTI2SS0_MCLK_OE, Drive));
        #endif
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,LRCKS0_OE, Drive));            
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SCLKS0_OE, Drive));            
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SDATS0_OE, Drive));
    }
    #if BAPE_CHIP_MAX_I2S_OUTPUTS > 1
    else if(index == 1)
    {
        regVal &= ~(
            #ifdef BCHP_AUD_FMM_MISC_SEROUT_OE_EXTI2SS1_MCLK_OE_MASK
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,EXTI2SS1_MCLK_OE) |
            #endif
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,LRCKS1_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SCLKS1_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SDATS1_OE));

        #ifdef BCHP_AUD_FMM_MISC_SEROUT_OE_EXTI2SS1_MCLK_OE_MASK
            regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,EXTI2SS1_MCLK_OE, Drive));
        #endif
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,LRCKS1_OE, Drive));            
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SCLKS1_OE, Drive));            
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SDATS1_OE, Drive));
    }
    #endif 
    #if BAPE_CHIP_MAX_I2S_OUTPUTS > 2
        #error "Need to support more I2S outputs"
    #endif 
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

    return errCode;
}

/**************************************************************************/

static BERR_Code BAPE_I2sOutput_P_CloseHw_Legacy(BAPE_I2sOutputHandle handle)
{
    BERR_Code       errCode = BERR_SUCCESS;
    BAPE_Handle     deviceHandle;
    uint32_t        regAddr, regVal;
    unsigned        index;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    index = handle->index;

    /* Put the I2S outputs into a high-Z state */
    regAddr = BCHP_AUD_FMM_MISC_SEROUT_OE;
    regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
    if (handle->index == 0)
    {
        regVal &= ~(
            #ifdef BCHP_AUD_FMM_MISC_SEROUT_OE_EXTI2SS0_MCLK_OE_MASK
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,EXTI2SS0_MCLK_OE) |
            #endif
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,LRCKS0_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SCLKS0_OE) |
                    BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SDATS0_OE));

        #ifdef BCHP_AUD_FMM_MISC_SEROUT_OE_EXTI2SS0_MCLK_OE_MASK
            regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,EXTI2SS0_MCLK_OE, Tristate));
        #endif
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,LRCKS0_OE, Tristate));
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SCLKS0_OE, Tristate));
        regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SDATS0_OE, Tristate));
    }
    #if BAPE_CHIP_MAX_I2S_OUTPUTS > 1
        else if(handle->index == 1)
        {
            regVal &= ~(
                #ifdef BCHP_AUD_FMM_MISC_SEROUT_OE_EXTI2SS1_MCLK_OE_MASK
                        BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,EXTI2SS1_MCLK_OE) |
                #endif
                        BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,LRCKS1_OE) |
                        BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SCLKS1_OE) |
                        BCHP_MASK(AUD_FMM_MISC_SEROUT_OE,SDATS1_OE));

            #ifdef BCHP_AUD_FMM_MISC_SEROUT_OE_EXTI2SS1_MCLK_OE_MASK
                regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,EXTI2SS1_MCLK_OE, Tristate));
            #endif
            regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,LRCKS1_OE, Tristate));
            regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SCLKS1_OE, Tristate));
            regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE,SDATS1_OE, Tristate));
        }
    #endif 
    #if BAPE_CHIP_MAX_I2S_OUTPUTS > 2
        #error "Need to support more I2S outputs"
    #endif 
    BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);

    /* Disable the I2S clock and data. */
    regAddr = GET_I2S_REG_ADDR3(BCHP_AUD_FMM_OP_CTRL_I2SS, index, _CFG);
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2SS0_CFG, CLOCK_ENABLE)|
                BCHP_MASK(AUD_FMM_OP_CTRL_I2SS0_CFG, DATA_ENABLE));
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CFG, CLOCK_ENABLE, Disable);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CFG, DATA_ENABLE, Disable);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

    return errCode;
}

/**************************************************************************/

static BERR_Code BAPE_I2sOutput_P_ApplySettings_Legacy(
    BAPE_I2sOutputHandle handle,
    const BAPE_I2sOutputSettings *pSettings,
    bool force
    )
{
    uint32_t regAddr, regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);
    BDBG_ASSERT(NULL != pSettings);

    /* Start by reading the appropriate BCHP_AUD_FMM_OP_CTRL_I2SS<n>_CFG register  for this device. */
    regAddr = GET_I2S_REG_ADDR3(BCHP_AUD_FMM_OP_CTRL_I2SS, handle->index, _CFG);
    regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);

    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2SS0_CFG, DATA_JUSTIFICATION));
    switch ( pSettings->justification )
    {
    case BAPE_I2sJustification_eMsbFirst:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CFG, DATA_JUSTIFICATION,MSB);
        break;
    case BAPE_I2sJustification_eLsbFirst:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CFG, DATA_JUSTIFICATION,LSB);
        break;
    default:
        BDBG_ERR(("Invalid value for BAPE_I2sOutputSettings.justification: %u", pSettings->justification ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2SS0_CFG, DATA_ALIGNMENT));
    switch ( pSettings->dataAlignment )
    {
    case BAPE_I2sDataAlignment_eDelayed:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CFG, DATA_ALIGNMENT, Delayed);
        break;
    case BAPE_I2sDataAlignment_eAligned:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CFG, DATA_ALIGNMENT, Aligned);
        break;
    default:
        BDBG_ERR(("Invalid value for BAPE_I2sOutputSettings.dataAlignment: %u", pSettings->dataAlignment ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2SS0_CFG, LRCK_POLARITY));
    switch ( pSettings->lrPolarity )
    {
    case BAPE_I2sLRClockPolarity_eLeftLow:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CFG, LRCK_POLARITY, Low_for_left);
        break;
    case BAPE_I2sLRClockPolarity_eLeftHigh:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CFG, LRCK_POLARITY, High_for_left);
        break;
    default:
        BDBG_ERR(("Invalid value for BAPE_I2sOutputSettings.lrPolarity: %u", pSettings->lrPolarity ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /*  Since setting the sclkRate involves updating two registers, only bother with this
     *  if somebody is actually changing it.
     */
    if (pSettings->sclkRate != handle->settings.sclkRate || force)
    {
        regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2SS0_CFG, SCLKS_PER_1FS_DIV32));
        switch ( pSettings->sclkRate )
        {
        case BAPE_SclkRate_e64Fs:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_I2SS0_CFG, SCLKS_PER_1FS_DIV32, 2 );
            break;

        case BAPE_SclkRate_e128Fs:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_I2SS0_CFG, SCLKS_PER_1FS_DIV32, 4 );
            break;

        default:
            BDBG_ERR(("Invalid value for BAPE_I2sOutputSettings.sclkRate: %u", pSettings->sclkRate ));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        /* Set the new sclkRate into the settings struct so that ..._UpdateMclkReg_isr()
         * will be able to use it.
         */
        handle->settings.sclkRate = pSettings->sclkRate;

        /* Now update the MCLK_CONFIG register.  */
        BKNI_EnterCriticalSection();
        BAPE_I2sOutput_P_UpdateMclkReg_isr( handle );
        BKNI_LeaveCriticalSection();
    }

    /* We only support 24 bits per sample. */
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2SS0_CFG, BITS_PER_SAMPLE ));
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CFG, BITS_PER_SAMPLE, Bitwidth24);

    BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);

    if ( pSettings->stereoMode != handle->settings.stereoMode || force )
    {
        regAddr = GET_I2S_CROSSBAR_ADDR(handle->index);
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2SS0_CROSSBAR, OUT_L)|
                    BCHP_MASK(AUD_FMM_OP_CTRL_I2SS0_CROSSBAR, OUT_R));
        switch ( pSettings->stereoMode )
        {
        default:
        case BAPE_StereoMode_eLeftRight:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CROSSBAR, OUT_L, In_l);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CROSSBAR, OUT_R, In_r);
            break;
        case BAPE_StereoMode_eLeftLeft:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CROSSBAR, OUT_L, In_l);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CROSSBAR, OUT_R, In_l);
            break;
        case BAPE_StereoMode_eRightRight:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CROSSBAR, OUT_L, In_r);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CROSSBAR, OUT_R, In_r);
            break;
        case BAPE_StereoMode_eRightLeft:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CROSSBAR, OUT_L, In_r);
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SS0_CROSSBAR, OUT_R, In_l);
            break;            
        }
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
        handle->settings.stereoMode = pSettings->stereoMode;
    }

    handle->settings = *pSettings;

    return BERR_SUCCESS;
}

static BERR_Code BAPE_I2sOutput_P_Enable_Legacy(BAPE_OutputPort output)
{
    BAPE_I2sOutputHandle handle;
    BAPE_IopStreamSettings streamSettings;
    BERR_Code errCode;
    unsigned streamId;
    
    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);

    BDBG_ASSERT(false == handle->enabled);

    streamId = GET_I2S_IOP_STREAM_ID(handle->index);
    
    /* Write source FCI to IOP */
    BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
    streamSettings.resolution = 24;
    streamSettings.input = output->sourceMixerFci.ids[BAPE_ChannelPair_eLeftRight];         /* Take source FCI provided from mixer */
    errCode = BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    streamId = GET_I2S_OP_STREAM_ID(handle->index);
    BDBG_MSG(("Enabling %s [stream %u]", handle->name, streamId));
    
    /* Write the enable bit in the OP (only stereo) */
    BDBG_MSG(("Writing %x to enable set", (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(streamId)));
    BREG_Write32(handle->deviceHandle->regHandle, 
                 BCHP_AUD_FMM_OP_CTRL_ENABLE_SET,
                 (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(streamId));

    handle->enabled = true;
    return BERR_SUCCESS;
}

/**************************************************************************/

static void BAPE_I2sOutput_P_Disable_Legacy(BAPE_OutputPort output)
{
    BAPE_I2sOutputHandle handle;
    BAPE_IopStreamSettings streamSettings;
    BERR_Code errCode;
    unsigned streamId;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sOutput);

    streamId = GET_I2S_OP_STREAM_ID(handle->index);
    BDBG_MSG(("Disabling %s [stream %u]", handle->name, streamId));

    /* Clear the enable bit in the OP */
    BDBG_MSG(("Writing %x to enable clear", (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(streamId)));
    BREG_Write32(handle->deviceHandle->regHandle, 
                 BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR,
                 BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_CLEAR, STREAM0_ENA)<<streamId);

    /* Reset source FCI to Invalid */
    streamId = GET_I2S_IOP_STREAM_ID(handle->index);    
    BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
    streamSettings.input = BAPE_FCI_ID_INVALID;
    errCode = BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
    BDBG_ASSERT(BERR_SUCCESS == errCode);

    handle->enabled = false;
}

#endif
/***************************************************************************
Summary:
Test code to support I2S testing
***************************************************************************/
#ifdef  SETUP_PINMUX_FOR_I2S_OUT_ON_7425

#include "bchp_aon_pin_ctrl.h"
#include "bchp_sun_top_ctrl.h"

static void BAPE_I2sOutput_P_SetupPinmuxForI2sTesting( BAPE_Handle deviceHandle )
{
    uint32_t    reg;

    /* First, route the I2S0 outputs to somewhere that we can attach to them.
     * It seems that we should be able to send them to either aon_gpio 
     * pins 01,02,03, or aon_gpio pins 10,11,12.  But it doesn't seem like
     * AON_GPIO_03 works.
     */
    #if 1
        reg = BREG_Read32(deviceHandle->regHandle,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
    
        reg &=~(    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_10 ) |
                    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_11 ) |
                    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12 ) );
    
        reg |=(    BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_10, 3 ) |  /* I2S_CLK0_OUT  on J2303/14 (Front panel i/f connector) */
                   BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_11, 3 ) |  /* I2S_DATA0_OUT on J2303/15 (Front panel i/f connector) */
                   BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, 3 ) ); /* I2S_LR0_OUT   on J2303/16 (Front panel i/f connector) */
    
        BREG_Write32 (deviceHandle->regHandle, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);
    #else
        reg = BREG_Read32(deviceHandle->regHandle,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
    
        reg &=~(    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01 ) |
                    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_02 ) |
                    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03 ) );
    
        reg |=(    BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01, 3 ) |  /* I2S_CLK0_OUT  on J2303/10 (Front panel i/f connector) */
                   BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_02, 3 ) |  /* I2S_DATA0_OUT on J2303/11 (Front panel i/f connector) */
                   BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03, 3 ) ); /* I2S_LR0_OUT   on J2303/12 (Front panel i/f connector) */
    
        BREG_Write32 (deviceHandle->regHandle, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);
    #endif

    /* Now, route the I2S1 outputs to somewhere that we can attach to them. 
     * These can only come out on gpio pins 55,56,57.
     */
    reg = BREG_Read32(deviceHandle->regHandle,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);

    reg &=~(    BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_057 ) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_055 ) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_056 ) );

    reg |=(    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_057, 5 ) |  /* I2S_CLK1 on  J2902/3  (RMXP_OUT connector)  */
               BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_055, 5 ) |  /* I2S_DATA1 on J2902/21 (RMXP_OUT connector)  */
               BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_056, 5 ) ); /* I2S_LR1 on   J2902/23 (RMXP_OUT connector)  */

    BREG_Write32 (deviceHandle->regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);

    /* Route each "External MCLK" (also called AUD_FS_CLKx)  */
    reg = BREG_Read32(deviceHandle->regHandle,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2);
    reg &=~ ( BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, gpio_099) |
              BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, gpio_100) );
    reg |=  ( BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, gpio_099, 2 ) |  /* External MCLK0 (AUD_FS_CLK0) on J3002/1 */
              BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, gpio_100, 2 ) ); /* External MCLK1 (AUD_FS_CLK1) on J3002/5 */
    BREG_Write32 (deviceHandle->regHandle, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);
}
#endif /* SETUP_PINMUX_FOR_I2S_OUT_ON_7425 */


/***************************************************************************
    Define stub functions for when there are no I2S outputs. 
***************************************************************************/
#else /* BAPE_CHIP_MAX_I2S_OUTPUTS <= 0 */
    /* No I2S outputs, just use stubbed out functions. */

/**************************************************************************/

void BAPE_I2sOutput_GetDefaultSettings(
    BAPE_I2sOutputSettings *pSettings
    )
{
    BSTD_UNUSED(pSettings);
}

/**************************************************************************/

BERR_Code BAPE_I2sOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_I2sOutputSettings *pSettings,
    BAPE_I2sOutputHandle *pHandle             /* [out] */
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);

    *pHandle = NULL;

    return BERR_NOT_SUPPORTED;
}

/**************************************************************************/

void BAPE_I2sOutput_Close(
    BAPE_I2sOutputHandle handle
    )
{
    BSTD_UNUSED(handle);
}

/**************************************************************************/

void BAPE_I2sOutput_GetSettings(
    BAPE_I2sOutputHandle handle,
    BAPE_I2sOutputSettings *pSettings     /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

/**************************************************************************/

BERR_Code BAPE_I2sOutput_SetSettings(
    BAPE_I2sOutputHandle handle,
    const BAPE_I2sOutputSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);

    return BERR_NOT_SUPPORTED;
}

/**************************************************************************/

void BAPE_I2sOutput_GetOutputPort(
    BAPE_I2sOutputHandle handle,
    BAPE_OutputPort *pOutputPort        /* [out] */
    )
{
    BSTD_UNUSED(handle);

    *pOutputPort = NULL;
}

/**************************************************************************/

BERR_Code BAPE_I2sOutput_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BSTD_UNUSED(bapeHandle);
    return BERR_SUCCESS;
}

#endif /* BAPE_CHIP_MAX_I2S_OUTPUTS > */
