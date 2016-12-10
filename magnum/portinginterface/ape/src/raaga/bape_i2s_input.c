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

BDBG_MODULE(bape_i2s_input);

BDBG_OBJECT_ID(BAPE_I2sInput);

#if defined BCHP_AUD_FMM_IOP_IN_I2S_STEREO_0_REG_START
    #include "bchp_aud_fmm_iop_in_i2s_stereo_0.h"
    #ifdef BCHP_AUD_FMM_IOP_IN_I2S_STEREO_1_REG_START
        #include "bchp_aud_fmm_iop_in_i2s_stereo_1.h"
        #define GET_I2S_INPUT_OFFSET(idx) (idx*(BCHP_AUD_FMM_IOP_IN_I2S_STEREO_1_REG_START-BCHP_AUD_FMM_IOP_IN_I2S_STEREO_0_REG_START))
    #else
        #define GET_I2S_INPUT_OFFSET(idx) (0)
    #endif
    #define BAPE_I2S_IOPIN_CONSTRUCT(prefix,suffix) prefix##_STEREO_0_##suffix
    #define BAPE_I2S_IOPIN_VERSION      1
#elif defined BCHP_AUD_FMM_IOP_IN_I2S_0_REG_START
    #include "bchp_aud_fmm_iop_in_i2s_0.h"
    #ifdef BCHP_AUD_FMM_IOP_IN_I2S_1_REG_START
        #include "bchp_aud_fmm_iop_in_i2s_1.h"
        #define GET_I2S_INPUT_OFFSET(idx) (idx*(BCHP_AUD_FMM_IOP_IN_I2S_1_REG_START-BCHP_AUD_FMM_IOP_IN_I2S_0_REG_START))
    #else
        #define GET_I2S_INPUT_OFFSET(idx) (0)
    #endif
    #define BAPE_I2S_IOPIN_CONSTRUCT(prefix,suffix) prefix##_0_##suffix
    #define BAPE_I2S_IOPIN_VERSION      2
#else
    #ifdef BCHP_AUD_FMM_IOP_CTRL_I2SIN_CFG1
        #define GET_I2S_INPUT_OFFSET(idx) (idx*(BCHP_AUD_FMM_IOP_CTRL_I2SIN_CFG1-BCHP_AUD_FMM_IOP_CTRL_I2SIN_CFG0))
    #else
        #define GET_I2S_INPUT_OFFSET(idx) (0)
    #endif
#endif

#if BAPE_CHIP_MAX_I2S_INPUTS > 0
/* Static function prototypes */
#if defined BAPE_I2S_IOPIN_VERSION
/* 7429 style of registers */
static void BAPE_I2sInput_P_Enable_IopOut(BAPE_InputPort inputPort);
static void BAPE_I2sInput_P_Disable_IopOut(BAPE_InputPort inputPort);
static BERR_Code BAPE_I2sInput_P_ApplySettings_IopOut(BAPE_I2sInputHandle handle, const BAPE_I2sInputSettings *pSettings);

#elif defined BCHP_AUD_FMM_IOP_CTRL_I2SIN_CFG0
/* Legacy style of registers */
static void BAPE_I2sInput_P_Enable_Legacy(BAPE_InputPort inputPort);
static void BAPE_I2sInput_P_Disable_Legacy(BAPE_InputPort inputPort);
static BERR_Code BAPE_I2sInput_P_ApplySettings_Legacy(BAPE_I2sInputHandle handle, const BAPE_I2sInputSettings *pSettings);

#endif

/* Input port callbacks */
static void BAPE_I2sInput_P_Enable(BAPE_InputPort inputPort);
static void BAPE_I2sInput_P_Disable(BAPE_InputPort inputPort);
#endif

/****  #define SETUP_PINMUX_FOR_I2S_IN_ON_7422   Only defined for testing... changes pinmux settings. */
#ifdef  SETUP_PINMUX_FOR_I2S_IN_ON_7422
    static void BAPE_I2sInput_P_SetupPinmuxForI2sTesting( BAPE_Handle deviceHandle );
    #warning "Compiling with special pinmux code to enable I2S input"
#endif /* SETUP_PINMUX_FOR_I2S_IN_ON_7422 */


/***************************************************************************
        Public APIs: From bape_input.h
***************************************************************************/
void BAPE_I2sInput_GetDefaultSettings(
    BAPE_I2sInputSettings *pSettings        /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->sampleRate = 48000;
    pSettings->bitsPerSample = 0;
    pSettings->lrPolarity = BAPE_I2sLRClockPolarity_eLeftLow;
    pSettings->sclkPolarity = BAPE_I2sSclkPolarity_eFalling;
    pSettings->dataAlignment = BAPE_I2sDataAlignment_eDelayed;
    pSettings->justification = BAPE_I2sJustification_eMsbFirst;
}

/**************************************************************************/

BERR_Code BAPE_I2sInput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_I2sInputSettings *pSettings,
    BAPE_I2sInputHandle *pHandle             /* [out] */
    )
{
#if BAPE_CHIP_MAX_I2S_INPUTS > 0
    BERR_Code errCode;
    BAPE_I2sInputHandle handle;
    BAPE_I2sInputSettings defaultSettings;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);
    
    BDBG_MSG(("%s: Opening I2S Input: %u", __FUNCTION__, index));

    *pHandle = NULL;

    if ( index >= BAPE_CHIP_MAX_I2S_INPUTS )
    {
        BDBG_ERR(("Request to open I2S input %d but chip only has %u I2S inputs", index, BAPE_CHIP_MAX_I2S_INPUTS));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->i2sInputs[index] )
    {
        BDBG_ERR(("I2S input %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    #ifdef  SETUP_PINMUX_FOR_I2S_IN_ON_7422
        BAPE_I2sInput_P_SetupPinmuxForI2sTesting(deviceHandle);
    #endif /* SETUP_PINMUX_FOR_I2S_IN_ON_7422 */

    /* Allocate the device structure, then fill in all the fields. */
    handle = BKNI_Malloc(sizeof(BAPE_I2sInput));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_I2sInput));
    BDBG_OBJECT_SET(handle, BAPE_I2sInput);
    handle->deviceHandle = deviceHandle;
    handle->index = index;
    handle->offset = GET_I2S_INPUT_OFFSET(index);
    BAPE_P_InitInputPort(&handle->inputPort, BAPE_InputPortType_eI2s, index, handle);
#if defined BAPE_I2S_IOPIN_VERSION
    {
        uint32_t regVal;
        regVal = BAPE_Reg_P_Read(deviceHandle, handle->offset + BAPE_I2S_IOPIN_CONSTRUCT(BCHP_AUD_FMM_IOP_IN_I2S,CAPTURE_FCI_ID_TABLE));
        handle->inputPort.streamId[BAPE_ChannelPair_eLeftRight] = BCHP_GET_FIELD_DATA(regVal, BAPE_I2S_IOPIN_CONSTRUCT(AUD_FMM_IOP_IN_I2S,CAPTURE_FCI_ID_TABLE), START_FCI_ID);
    }
#else
    handle->inputPort.streamId[BAPE_ChannelPair_eLeftRight] = 4+index;
#endif
    handle->inputPort.enable = BAPE_I2sInput_P_Enable;
    handle->inputPort.disable = BAPE_I2sInput_P_Disable;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "I2S Input %u", index);
    handle->inputPort.pName = handle->name;

    /* Init to specified settings */
    if ( NULL == pSettings )
    {
        BAPE_I2sInput_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    errCode = BAPE_I2sInput_SetSettings(handle, pSettings);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_settings;
    }

    *pHandle = handle;
    handle->deviceHandle->i2sInputs[index] = handle;
    return BERR_SUCCESS;

err_settings:
    BAPE_I2sInput_Close(handle);
    return errCode;
#else
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    BDBG_ASSERT(NULL != pHandle);
    *pHandle = NULL;
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif    
}

/**************************************************************************/

void BAPE_I2sInput_Close(
    BAPE_I2sInputHandle handle
    )
{
#if BAPE_CHIP_MAX_I2S_INPUTS > 0
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sInput);

    /* Make sure we're not still connected to anything */
    if ( BAPE_InputPort_P_HasConsumersAttached(&handle->inputPort) )
    {
        BDBG_ERR(("Cannot close I2S input %p (%d)", (void *)handle, handle->index));
        #if BDBG_DEBUG_BUILD
        {
            BAPE_PathNode * pConsumer;
            for ( pConsumer = BLST_S_FIRST(&handle->inputPort.consumerList);
                pConsumer != NULL;
                pConsumer = BLST_S_NEXT(pConsumer, consumerNode) )
            {
                BDBG_ERR(("  still connected to %s", pConsumer->pName));
            }

        }
        #endif
        BDBG_ASSERT(!BAPE_InputPort_P_HasConsumersAttached(&handle->inputPort));
        return;
    }

    handle->deviceHandle->i2sInputs[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_I2sInput);
    BKNI_Free(handle);    
#else
    BSTD_UNUSED(handle);
#endif
}

/**************************************************************************/

void BAPE_I2sInput_GetSettings(
    BAPE_I2sInputHandle handle,
    BAPE_I2sInputSettings *pSettings        /* [out] */
    )
{
#if BAPE_CHIP_MAX_I2S_INPUTS > 0
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sInput);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
#endif
}

/**************************************************************************/

BERR_Code BAPE_I2sInput_SetSettings(
    BAPE_I2sInputHandle handle,
    const BAPE_I2sInputSettings *pSettings
    )
{
#if BAPE_CHIP_MAX_I2S_INPUTS > 0   
    BERR_Code errCode;
    BAPE_FMT_Descriptor format;

    #ifdef BAPE_I2S_IOPIN_VERSION
    errCode = BAPE_I2sInput_P_ApplySettings_IopOut(handle, pSettings);
    #else
    errCode = BAPE_I2sInput_P_ApplySettings_Legacy(handle, pSettings);
    #endif
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->settings = *pSettings;

    /* Update Format */
    BKNI_EnterCriticalSection();
    BAPE_InputPort_P_GetFormat_isr(&handle->inputPort, &format);
    format.source = BAPE_DataSource_eFci;
    format.type = BAPE_DataType_ePcmStereo;
    format.sampleRate = handle->settings.sampleRate;
    errCode = BAPE_InputPort_P_SetFormat_isr(&handle->inputPort, &format);
    BKNI_LeaveCriticalSection();
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/**************************************************************************/

void BAPE_I2sInput_GetInputPort(
    BAPE_I2sInputHandle handle,
    BAPE_InputPort *pInputPort
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sInput);
    BDBG_ASSERT(NULL != pInputPort);
    *pInputPort = &handle->inputPort;
}

/***************************************************************************
        BAPE Internal APIs: From bape_fmm_priv.h
***************************************************************************/
#if !B_REFSW_MINIMAL
BERR_Code BAPE_I2sInput_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BERR_Code   errCode = BERR_SUCCESS;
#if BAPE_CHIP_MAX_I2S_INPUTS > 0
    unsigned    i2sInputIndex;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each opened I2sInput, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( i2sInputIndex=0 ; i2sInputIndex<BAPE_CHIP_MAX_I2S_INPUTS ; i2sInputIndex++ )
    {
        if ( bapeHandle->i2sInputs[i2sInputIndex] )       /* If this I2sInput is open... */
        {
            BAPE_I2sInputHandle hI2sInput = bapeHandle->i2sInputs[i2sInputIndex];

            /* Put the HW into the generic open state. */
                /* Nothing to do here for I2sInput. */
            
            /* Now apply changes for the settings struct. */
            errCode = BAPE_I2sInput_SetSettings(hI2sInput, &hI2sInput->settings);
            if ( errCode ) return BERR_TRACE(errCode);

            /* Now restore the dynamic stuff from the values saved in the device struct. */
                /* And nothing to do here either. */
        }
    }
#else
    BSTD_UNUSED(bapeHandle);
#endif
    return errCode;
}
#endif

/***************************************************************************
        Private callbacks: Protyped above
***************************************************************************/
#if BAPE_CHIP_MAX_I2S_INPUTS > 0
static void BAPE_I2sInput_P_Enable(BAPE_InputPort inputPort)
{
#ifdef BAPE_I2S_IOPIN_VERSION
    BAPE_I2sInput_P_Enable_IopOut(inputPort);
#else
    BAPE_I2sInput_P_Enable_Legacy(inputPort);
#endif
}

/**************************************************************************/

static void BAPE_I2sInput_P_Disable(BAPE_InputPort inputPort)
{
#ifdef BAPE_I2S_IOPIN_VERSION
    BAPE_I2sInput_P_Disable_IopOut(inputPort);
#else
    BAPE_I2sInput_P_Disable_Legacy(inputPort);
#endif
}

#ifdef BAPE_I2S_IOPIN_VERSION
/* 7429 style of registers */
static void BAPE_I2sInput_P_Enable_IopOut(BAPE_InputPort inputPort)
{
    BAPE_I2sInputHandle handle;

    handle = inputPort->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sInput);
    BDBG_ASSERT(false == handle->enable);

    BDBG_MSG(("Enabling %s", handle->name));

    BAPE_Reg_P_UpdateField(handle->deviceHandle, 
                           BAPE_I2S_IOPIN_CONSTRUCT(BCHP_AUD_FMM_IOP_IN_I2S,CAP_STREAM_CFG_0) + handle->offset,
                           BAPE_I2S_IOPIN_CONSTRUCT(AUD_FMM_IOP_IN_I2S,CAP_STREAM_CFG_0),
                           CAP_ENA,
                           1);

    handle->enable = true;
}

static void BAPE_I2sInput_P_Disable_IopOut(BAPE_InputPort inputPort)
{
    BAPE_I2sInputHandle handle;

    handle = inputPort->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sInput);
    BDBG_ASSERT(true == handle->enable);

    BDBG_MSG(("Disabling %s", handle->name));

    BAPE_Reg_P_UpdateField(handle->deviceHandle, 
                           BAPE_I2S_IOPIN_CONSTRUCT(BCHP_AUD_FMM_IOP_IN_I2S,CAP_STREAM_CFG_0) + handle->offset,
                           BAPE_I2S_IOPIN_CONSTRUCT(AUD_FMM_IOP_IN_I2S,CAP_STREAM_CFG_0),
                           CAP_ENA,
                           0);

    handle->enable = false;
}

static BERR_Code BAPE_I2sInput_P_ApplySettings_IopOut(BAPE_I2sInputHandle handle, const BAPE_I2sInputSettings *pSettings)
{
    BAPE_Reg_P_FieldList regFieldList;
    uint32_t regAddr;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sInput);
    BDBG_ASSERT(NULL != pSettings);

    regAddr = BAPE_I2S_IOPIN_CONSTRUCT(BCHP_AUD_FMM_IOP_IN_I2S,I2S_IN_CFG) + handle->offset;
    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
    BAPE_Reg_P_AddToFieldList(&regFieldList, BAPE_I2S_IOPIN_CONSTRUCT(AUD_FMM_IOP_IN_I2S,I2S_IN_CFG), BITS_PER_SAMPLE, pSettings->bitsPerSample);

    switch ( pSettings->dataAlignment )
    {
    case BAPE_I2sDataAlignment_eAligned:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPIN_CONSTRUCT(AUD_FMM_IOP_IN_I2S,I2S_IN_CFG), DATA_ALIGNMENT, Aligned);
        break;
    case BAPE_I2sDataAlignment_eDelayed:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPIN_CONSTRUCT(AUD_FMM_IOP_IN_I2S,I2S_IN_CFG), DATA_ALIGNMENT, Delayed);
        break;
    default:
        BDBG_ERR(("Invalid Data Alignment type %d", pSettings->dataAlignment));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    switch ( pSettings->justification )
    {
    case BAPE_I2sJustification_eLsbFirst:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPIN_CONSTRUCT(AUD_FMM_IOP_IN_I2S,I2S_IN_CFG), DATA_JUSTIFICATION, LSB);
        break;
    case BAPE_I2sJustification_eMsbFirst:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPIN_CONSTRUCT(AUD_FMM_IOP_IN_I2S,I2S_IN_CFG), DATA_JUSTIFICATION, MSB);
        break;
    default:
        BDBG_ERR(("Invalid Data Justification type %d", pSettings->justification));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    switch ( pSettings->lrPolarity )
    {
    case BAPE_I2sLRClockPolarity_eLeftHigh:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPIN_CONSTRUCT(AUD_FMM_IOP_IN_I2S,I2S_IN_CFG), LRCK_POLARITY, High_for_left);
        break;
    case BAPE_I2sLRClockPolarity_eLeftLow:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPIN_CONSTRUCT(AUD_FMM_IOP_IN_I2S,I2S_IN_CFG), LRCK_POLARITY, Low_for_left);
        break;
    default:
        BDBG_ERR(("Invalid polarity of the left/right clock %d", pSettings->lrPolarity));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    switch ( pSettings->sclkPolarity )
    {
    case BAPE_I2sSclkPolarity_eRising:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPIN_CONSTRUCT(AUD_FMM_IOP_IN_I2S,I2S_IN_CFG), SCLK_POLARITY, Rising_aligned_with_sdata);
        break;
    case BAPE_I2sSclkPolarity_eFalling:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_I2S_IOPIN_CONSTRUCT(AUD_FMM_IOP_IN_I2S,I2S_IN_CFG), SCLK_POLARITY, Falling_aligned_with_sdata);
        break;
    default:
        BDBG_ERR(("Invalid polarity of serial bit clock %d", pSettings->lrPolarity));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);

    return BERR_SUCCESS;
}

#elif defined BCHP_AUD_FMM_IOP_CTRL_I2SIN_CFG0
/* Legacy style of registers */
static void BAPE_I2sInput_P_Enable_Legacy(BAPE_InputPort inputPort)
{
    BAPE_I2sInputHandle handle;
    BERR_Code errCode;

    handle = inputPort->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sInput);
    BDBG_ASSERT(false == handle->enable);

    BDBG_MSG(("Enabling %s", handle->name));

    errCode = BAPE_Iop_P_EnableCapture(handle->deviceHandle, inputPort->streamId[0], 1);
    BDBG_ASSERT(BERR_SUCCESS == errCode);

    handle->enable = true;
}

static void BAPE_I2sInput_P_Disable_Legacy(BAPE_InputPort inputPort)
{
    BAPE_I2sInputHandle handle;

    handle = inputPort->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_I2sInput);
    BDBG_ASSERT(true == handle->enable);

    BDBG_MSG(("Disabling %s", handle->name));

    BAPE_Iop_P_DisableCapture(handle->deviceHandle, inputPort->streamId[0], 1);

    handle->enable = false;
}

static BERR_Code BAPE_I2sInput_P_ApplySettings_Legacy(BAPE_I2sInputHandle handle, const BAPE_I2sInputSettings *pSettings)
{
    uint32_t regVal, regAddr;

    BDBG_OBJECT_ASSERT(handle, BAPE_I2sInput);
    BDBG_ASSERT(NULL != pSettings);

    regAddr = BCHP_AUD_FMM_IOP_CTRL_I2SIN_CFG0 + handle->offset;
    regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_IOP_CTRL_I2SIN_CFG0, DATA_JUSTIFICATION) |
                BCHP_MASK(AUD_FMM_IOP_CTRL_I2SIN_CFG0, DATA_ALIGNMENT) |
                BCHP_MASK(AUD_FMM_IOP_CTRL_I2SIN_CFG0, SCLK_POLARITY) |
                BCHP_MASK(AUD_FMM_IOP_CTRL_I2SIN_CFG0, LRCK_POLARITY) |
                BCHP_MASK(AUD_FMM_IOP_CTRL_I2SIN_CFG0, BITS_PER_SAMPLE));

    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_I2SIN_CFG0, BITS_PER_SAMPLE, pSettings->bitsPerSample);

    switch ( pSettings->dataAlignment )
    {
    case BAPE_I2sDataAlignment_eAligned:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, DATA_ALIGNMENT, Aligned);
        break;
    case BAPE_I2sDataAlignment_eDelayed:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, DATA_ALIGNMENT, Delayed);
        break;
    default:
        BDBG_ERR(("Invalid Data Alignment type %d", pSettings->dataAlignment));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    switch ( pSettings->justification )
    {
    case BAPE_I2sJustification_eLsbFirst:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, DATA_JUSTIFICATION, LSB);
        break;
    case BAPE_I2sJustification_eMsbFirst:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, DATA_JUSTIFICATION, MSB);
        break;
    default:
        BDBG_ERR(("Invalid Data Justification type %d", pSettings->justification));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    switch ( pSettings->lrPolarity )
    {
    case BAPE_I2sLRClockPolarity_eLeftHigh:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, LRCK_POLARITY, High_for_left);
        break;
    case BAPE_I2sLRClockPolarity_eLeftLow:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, LRCK_POLARITY, Low_for_left);
        break;
    default:
        BDBG_ERR(("Invalid polarity of the left/right clock %d", pSettings->lrPolarity));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    switch ( pSettings->sclkPolarity )
    {
    case BAPE_I2sSclkPolarity_eRising:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, SCLK_POLARITY, Rising_aligned_with_sdata);
        break;
    case BAPE_I2sSclkPolarity_eFalling:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_I2SIN_CFG0, SCLK_POLARITY, Falling_aligned_with_sdata);
        break;
    default:
        BDBG_ERR(("Invalid polarity of serial bit clock %d", pSettings->lrPolarity));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);

    return BERR_SUCCESS;
}

#endif

#endif

/***************************************************************************
        Test code to support I2S testing
***************************************************************************/
#ifdef  SETUP_PINMUX_FOR_I2S_IN_ON_7422

#include "bchp_aon_pin_ctrl.h"
#include "bchp_sun_top_ctrl.h"

static void BAPE_I2sInput_P_SetupPinmuxForI2sTesting( BAPE_Handle deviceHandle )
{
    uint32_t    reg;

    /* First, route the I2S0 inputs from somewhere that we can attach to them. It seems that we should be able to get
     * them from either aon_gpio pins 01,02,03, or aon_gpio pins 10,11,12. 
     */
    #if 1
        reg = BREG_Read32(deviceHandle->regHandle,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
    
        reg &=~(    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_10 ) |
                    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_11 ) |
                    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12 ) );
    
        reg |=(    BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_10, 4 ) |  /* I2S_CLK0_IN  on J2303/14 (Front panel i/f connector) */
                   BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_11, 4 ) |  /* I2S_DATA0_IN on J2303/15 (Front panel i/f connector) */
                   BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, 4 ) ); /* I2S_LR0_IN   on J2303/16 (Front panel i/f connector) */
    
        BREG_Write32 (deviceHandle->regHandle, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);
    #else
        reg = BREG_Read32(deviceHandle->regHandle,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
    
        reg &=~(    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01 ) |
                    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_02 ) |
                    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03 ) );
    
        reg |=(    BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01, 2 ) |  /* I2S_CLK0_IN  on J2303/10 (Front panel i/f connector) */
                   BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_02, 2 ) |  /* I2S_DATA0_IN on J2303/11 (Front panel i/f connector) */
                   BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03, 2 ) ); /* I2S_LR0_IN   on J2303/12 (Front panel i/f connector) */
    
        BREG_Write32 (deviceHandle->regHandle, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);
    #endif
}
#endif /* SETUP_PINMUX_FOR_I2S_IN_ON_7422 */
