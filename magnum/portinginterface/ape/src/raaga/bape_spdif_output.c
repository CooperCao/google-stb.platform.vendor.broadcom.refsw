/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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

#include "bape.h"
#include "bape_priv.h"

BDBG_MODULE(bape_spdif_output);

#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0

BDBG_OBJECT_ID(BAPE_SpdifOutput);

typedef struct BAPE_SpdifOutput
{
    BDBG_OBJECT(BAPE_SpdifOutput)
    BAPE_Handle deviceHandle;
    BAPE_SpdifOutputSettings settings;
    unsigned index;
    BAPE_OutputPortObject outputPort;
    uint32_t offset;
    unsigned sampleRate;
    struct
    {
        BAPE_MclkSource mclkSource;
        unsigned pllChannel;    /* only applies if mclkSource refers to a PLL */
        unsigned mclkFreqToFsRatio;
    } mclkInfo;
    bool enabled;
    bool muted;
    char name[9];   /* SPDIF %d */
    /* The following are used to generate a pauseburst compressed mute on legacy chips */
    uint32_t *pMuteBuffer;
    BAPE_SfifoGroupHandle hSfifo;
    BAPE_MixerGroupHandle hMixer;
    BAPE_SpdifBurstType currentMuteBufferType;
} BAPE_SpdifOutput;

/* Static function prototypes */
static void      BAPE_SpdifOutput_P_SetCbits_isr(BAPE_SpdifOutputHandle handle);
static BERR_Code BAPE_SpdifOutput_P_OpenHw(BAPE_SpdifOutputHandle handle);
static BERR_Code BAPE_SpdifOutput_P_CloseHw(BAPE_SpdifOutputHandle handle);
static BERR_Code BAPE_SpdifOutput_P_SetBurstConfig(BAPE_SpdifOutputHandle handle, bool pcm);
static BERR_Code BAPE_SpdifOutput_P_ApplySettings(BAPE_SpdifOutputHandle handle, 
                                                  const BAPE_SpdifOutputSettings *pSettings, bool force );

/* Output port callbacks */
static void      BAPE_SpdifOutput_P_SetTimingParams_isr(BAPE_OutputPort output, unsigned sampleRate, BAVC_Timebase timebase);
static void      BAPE_SpdifOutput_P_SetMute(BAPE_OutputPort output, bool muted, bool sync);
static BERR_Code BAPE_SpdifOutput_P_Enable(BAPE_OutputPort output);
static void      BAPE_SpdifOutput_P_Disable(BAPE_OutputPort output);
static void      BAPE_SpdifOutput_P_SetMclk_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio);

#if BAPE_CHIP_MAX_SPDIF_OUTPUTS  > 1
    #define  GET_SPDIF_REG_ADDR2(prefix,idx)         (prefix##0         + (prefix##1         - prefix##0        ) * idx )
    #define  GET_SPDIF_REG_ADDR3(prefix,idx,suffix)  (prefix##0##suffix + (prefix##1##suffix - prefix##0##suffix) * idx )
    #define  GET_SPDIF_STREAM_ID(idx) ((idx == 1)?0:(BDBG_ASSERT(0),0))
#else
    #define  GET_SPDIF_REG_ADDR2(prefix,idx       )  (prefix##0         )
    #define  GET_SPDIF_REG_ADDR3(prefix,idx,suffix)  (prefix##0##suffix )
    #define  GET_SPDIF_STREAM_ID(idx) (0)
#endif

/* Implementations for Legacy vs. New [7429] Hardware */
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
#include "bchp_aud_fmm_iop_out_spdif_0.h"

#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_1_REG_START
#include "bchp_aud_fmm_iop_out_spdif_1.h"
#endif

static BERR_Code BAPE_SpdifOutput_P_Open_IopOut(BAPE_SpdifOutputHandle handle);
static void      BAPE_SpdifOutput_P_Close_IopOut(BAPE_SpdifOutputHandle handle);
static void      BAPE_SpdifOutput_P_SetCbits_IopOut_isr(BAPE_SpdifOutputHandle handle);
static void      BAPE_SpdifOutput_P_SetMclk_IopOut_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio);
static void      BAPE_SpdifOutput_P_SetMute_IopOut(BAPE_OutputPort output, bool muted, bool sync);
static BERR_Code BAPE_SpdifOutput_P_Enable_IopOut(BAPE_OutputPort output);
static void      BAPE_SpdifOutput_P_Disable_IopOut(BAPE_OutputPort output);
static BERR_Code BAPE_SpdifOutput_P_OpenHw_IopOut(BAPE_SpdifOutputHandle handle);
static BERR_Code BAPE_SpdifOutput_P_CloseHw_IopOut(BAPE_SpdifOutputHandle handle);
static BERR_Code BAPE_SpdifOutput_P_ApplySettings_IopOut(BAPE_SpdifOutputHandle handle, 
                                                         const BAPE_SpdifOutputSettings *pSettings, bool force);
static BERR_Code BAPE_SpdifOutput_P_SetBurstConfig_IopOut(BAPE_SpdifOutputHandle handle, bool pcm);


#define  BAPE_SPDIF_0_START BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
#define  BAPE_SPDIF_0_END   BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_END

#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 1
    #define  BAPE_SPDIF_1_START BCHP_AUD_FMM_IOP_OUT_SPDIF_1_REG_START
    #define  BAPE_SPDIF_1_END   BCHP_AUD_FMM_IOP_OUT_SPDIF_1_REG_END
    #if ((BAPE_SPDIF_0_END-BAPE_SPDIF_0_START) != (BAPE_SPDIF_1_END-BAPE_SPDIF_1_START))
        #error "SPDIF interfaces present with different sizes"
    #endif
    #define  BAPE_SPDIF_START(idx) ((idx==1)?BAPE_SPDIF_1_START:BAPE_SPDIF_0_START)
#else
    #define  BAPE_SPDIF_START(idx) BAPE_SPDIF_0_START
#endif

#define BAPE_SPDIF_Reg_P_GetAddress(Register, idx) BAPE_Reg_P_GetAddress(Register, BAPE_SPDIF_0_START, BAPE_SPDIF_START, idx)


#else
static const uint16_t g_pauseburst[6] = {0xf872, 0x4e1f, 0x0003, 0x0020, 0x0000, 0x0000};
static const uint16_t g_nullburst[4] = {0xf872, 0x4e1f, 0xe000, 0x0000};

#define BAPE_P_MUTE_BUFFER_SIZE         sizeof(g_pauseburst)*64

static BERR_Code BAPE_SpdifOutput_P_Open_Legacy(BAPE_SpdifOutputHandle handle);
static void      BAPE_SpdifOutput_P_Close_Legacy(BAPE_SpdifOutputHandle handle);
static void      BAPE_SpdifOutput_P_SetCbits_Legacy_isr(BAPE_SpdifOutputHandle handle);
static BERR_Code BAPE_SpdifOutput_P_Enable_Legacy(BAPE_OutputPort output);
static void      BAPE_SpdifOutput_P_Disable_Legacy(BAPE_OutputPort output);
static void      BAPE_SpdifOutput_P_SetMclk_Legacy_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio);
static BERR_Code BAPE_SpdifOutput_P_OpenHw_Legacy(BAPE_SpdifOutputHandle handle);
static BERR_Code BAPE_SpdifOutput_P_CloseHw_Legacy(BAPE_SpdifOutputHandle handle);
static BERR_Code BAPE_SpdifOutput_P_ApplySettings_Legacy(BAPE_SpdifOutputHandle handle, 
                                                         const BAPE_SpdifOutputSettings *pSettings, bool force);
static void      BAPE_SpdifOutput_P_SetMute_Legacy(BAPE_OutputPort output, bool muted, bool sync);
static BERR_Code BAPE_SpdifOutput_P_SetBurstConfig_Legacy(BAPE_SpdifOutputHandle handle, bool pcm);

#define ALWAYS_COMP_FOR_FW_CBITS        0
/* Use the second output of the last mixer.  It will never be used.  */
#define BAPE_SPDIF_INVALID_FCI_SOURCE (0x100|((BAPE_CHIP_MAX_MIXERS*2)+1))
#endif

/***************************************************************************
        Public APIs: From bape_output.h
***************************************************************************/
void BAPE_SpdifOutput_GetDefaultSettings(
    BAPE_SpdifOutputSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->stereoMode = BAPE_StereoMode_eLeftRight;
    pSettings->ditherEnabled = true;
    pSettings->limitTo16Bits = false;
    pSettings->underflowBurst = BAPE_SpdifBurstType_ePause;
    pSettings->useRawChannelStatus = false;
    /* channel status is initialized to zero for all fields */
}

/**************************************************************************/

BERR_Code BAPE_SpdifOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_SpdifOutputSettings *pSettings,
    BAPE_SpdifOutputHandle *pHandle             /* [out] */
    )
{
    BERR_Code errCode;
    BAPE_SpdifOutputHandle handle;
    BAPE_SpdifOutputSettings defaultSettings;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);
    
    BDBG_MSG(("%s: Opening SPDIF Output: %u", __FUNCTION__, index));

    *pHandle = NULL;

    if ( index >= BAPE_CHIP_MAX_SPDIF_OUTPUTS )
    {
        BDBG_ERR(("Request to open SPDIF output %d but chip only has %u SPDIF outputs", index, BAPE_CHIP_MAX_SPDIF_OUTPUTS));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->spdifOutputs[index] )
    {
        BDBG_ERR(("SPDIF output %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Allocate the device structure, then fill in all the fields. */
    handle = BKNI_Malloc(sizeof(BAPE_SpdifOutput));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Init to specified settings */
    if ( NULL == pSettings )
    {
        BAPE_SpdifOutput_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_SpdifOutput));
    BDBG_OBJECT_SET(handle, BAPE_SpdifOutput);
    handle->deviceHandle = deviceHandle;
    handle->index = index;
    BAPE_P_InitOutputPort(&handle->outputPort, BAPE_OutputPortType_eSpdifOutput, index, handle);
    BAPE_FMT_P_EnableSource(&handle->outputPort.capabilities, BAPE_DataSource_eFci);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_ePcmStereo);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_eIec61937);
    if ( pSettings->allow4xCompressed )
    {
        BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_eIec61937x4);
    }
    handle->outputPort.pllRequired = true;
    handle->outputPort.muteInMixer = true;
    handle->outputPort.setTimingParams_isr = BAPE_SpdifOutput_P_SetTimingParams_isr;
    handle->outputPort.enable = BAPE_SpdifOutput_P_Enable;
    handle->outputPort.disable = BAPE_SpdifOutput_P_Disable;
    handle->outputPort.setMclk_isr = BAPE_SpdifOutput_P_SetMclk_isr;
    handle->outputPort.setMute = BAPE_SpdifOutput_P_SetMute;
    handle->muted = true;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "SPDIF %u", index);
    handle->outputPort.pName = handle->name;
    handle->offset = 0;

    /* Setup to 48k, muted by default */
    BKNI_EnterCriticalSection();
    BAPE_SpdifOutput_P_SetTimingParams_isr(&handle->outputPort, 48000, BAVC_Timebase_e0);
    BKNI_LeaveCriticalSection();

    /* store open settings before open calls. */
    handle->settings = *pSettings;

#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
    errCode = BAPE_SpdifOutput_P_Open_IopOut(handle);
#else
    errCode = BAPE_SpdifOutput_P_Open_Legacy(handle);
#endif

    if ( errCode )
    {
        BAPE_SpdifOutput_Close(handle);
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_SpdifOutput_P_OpenHw(handle);
    if ( errCode )
    {
        BAPE_SpdifOutput_Close(handle);
        return BERR_TRACE(errCode);
    }

    /* mute */
    BAPE_SpdifOutput_P_SetMute(&handle->outputPort, handle->muted, false);

    /* Initialize hardware before applying settings */
    BKNI_EnterCriticalSection();
    BAPE_SpdifOutput_P_SetMclk_isr(&handle->outputPort, BAPE_MclkSource_ePll0, 0, BAPE_BASE_PLL_TO_FS_RATIO);
    BKNI_LeaveCriticalSection();  

    errCode = BAPE_SpdifOutput_P_ApplySettings(handle, pSettings, true);   /* true => force update of HW */
    if ( errCode )
    {
        BAPE_SpdifOutput_Close(handle);
        return BERR_TRACE(errCode);
    }

    *pHandle = handle;
    handle->deviceHandle->spdifOutputs[index] = handle;
    return BERR_SUCCESS;
}

/**************************************************************************/

void BAPE_SpdifOutput_Close(
    BAPE_SpdifOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    if ( handle->index >= BAPE_CHIP_MAX_SPDIF_OUTPUTS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(handle->index < BAPE_CHIP_MAX_SPDIF_OUTPUTS);
        return;
    }

    /* Make sure we're not still connected to anything */
    if ( handle->outputPort.mixer )
    {
        BDBG_ERR(("Cannot close SPDIF output %p (%d), still connected to mixer %p", (void *)handle, handle->index, (void *)handle->outputPort.mixer));
        BDBG_ASSERT(NULL == handle->outputPort.mixer);
        return;
    }

    BAPE_SpdifOutput_P_CloseHw(handle);

#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
    BAPE_SpdifOutput_P_Close_IopOut(handle);
#else
    BAPE_SpdifOutput_P_Close_Legacy(handle);
#endif

    handle->deviceHandle->spdifOutputs[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_SpdifOutput);
    BKNI_Free(handle);    
}

/**************************************************************************/

void BAPE_SpdifOutput_GetSettings(
    BAPE_SpdifOutputHandle handle,
    BAPE_SpdifOutputSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

/**************************************************************************/

BERR_Code BAPE_SpdifOutput_SetSettings(
    BAPE_SpdifOutputHandle handle,
    const BAPE_SpdifOutputSettings *pSettings
    )
{
    BERR_Code   errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(NULL != pSettings);

    errCode = BAPE_SpdifOutput_P_ApplySettings(handle, pSettings, false); /* false => don't force (only update HW for changes) */
    return errCode;
}

/**************************************************************************/

void BAPE_SpdifOutput_GetOutputPort(
    BAPE_SpdifOutputHandle handle,
    BAPE_OutputPort *pOutputPort        /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(NULL != pOutputPort);
    *pOutputPort = &handle->outputPort;
}

/***************************************************************************
        BAPE Internal APIs: From bape_fmm_priv.h
***************************************************************************/

BERR_Code BAPE_SpdifOutput_P_PrepareForStandby(
    BAPE_Handle bapeHandle
    )
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    spdifOutputIndex;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each opened SpdifOutput, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( spdifOutputIndex=0 ; spdifOutputIndex<BAPE_CHIP_MAX_SPDIF_OUTPUTS ; spdifOutputIndex++ )
    {
        if ( bapeHandle->spdifOutputs[spdifOutputIndex] )       /* If this SpdifOutput is open... */
        {
            BAPE_SpdifOutputHandle hSpdifOutput = bapeHandle->spdifOutputs[spdifOutputIndex];

            /* Put the HW into the generic closed state. */
            BAPE_SpdifOutput_P_CloseHw(hSpdifOutput);
        }
    }
    return errCode;
}


BERR_Code BAPE_SpdifOutput_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    spdifOutputIndex;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each opened SpdifOutput, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( spdifOutputIndex=0 ; spdifOutputIndex<BAPE_CHIP_MAX_SPDIF_OUTPUTS ; spdifOutputIndex++ )
    {
        if ( bapeHandle->spdifOutputs[spdifOutputIndex] )       /* If this SpdifOutput is open... */
        {
            BAPE_SpdifOutputHandle hSpdifOutput = bapeHandle->spdifOutputs[spdifOutputIndex];

            /* Put the HW into the generic open state. */
            errCode = BAPE_SpdifOutput_P_OpenHw(hSpdifOutput);
            if ( errCode ) return BERR_TRACE(errCode);
            
            /* Now apply changes for the settings struct. */
            errCode = BAPE_SpdifOutput_P_ApplySettings(hSpdifOutput, &hSpdifOutput->settings, true);   /* true => force update of HW */
            if ( errCode ) return BERR_TRACE(errCode);

            /* Now restore the dynamic stuff from the values saved in the device struct. */
            BKNI_EnterCriticalSection();

                BAPE_SpdifOutput_P_SetTimingParams_isr(&hSpdifOutput->outputPort, 
                                                     hSpdifOutput->sampleRate, 
                                                     0);    /* timebase is unused, 0 is dummy value */
                BAPE_SpdifOutput_P_SetMclk_isr(&hSpdifOutput->outputPort,
                                             hSpdifOutput->mclkInfo.mclkSource,
                                             hSpdifOutput->mclkInfo.pllChannel,
                                             hSpdifOutput->mclkInfo.mclkFreqToFsRatio );
            BKNI_LeaveCriticalSection();

        }
    }
    return errCode;
}

/***************************************************************************
        Private callbacks: Protyped above
***************************************************************************/

static void BAPE_SpdifOutput_P_SetTimingParams_isr(BAPE_OutputPort output, unsigned sampleRate, BAVC_Timebase timebase)
{
    BAPE_SpdifOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    BSTD_UNUSED(timebase);  /* SPDIF doesn't care */

    handle->sampleRate = sampleRate;

    BAPE_SpdifOutput_P_SetCbits_isr(handle);
}

/**************************************************************************/

static BERR_Code BAPE_SpdifOutput_P_Enable(BAPE_OutputPort output)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
    return BAPE_SpdifOutput_P_Enable_IopOut(output);
#else
    return BAPE_SpdifOutput_P_Enable_Legacy(output);
#endif
}

/**************************************************************************/

static void BAPE_SpdifOutput_P_Disable(BAPE_OutputPort output)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
    BAPE_SpdifOutput_P_Disable_IopOut(output);
#else
    BAPE_SpdifOutput_P_Disable_Legacy(output);
#endif
}

/**************************************************************************/

static void BAPE_SpdifOutput_P_SetMclk_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
    BAPE_SpdifOutput_P_SetMclk_IopOut_isr(output, mclkSource, pllChannel, mclkFreqToFsRatio);
#else
    BAPE_SpdifOutput_P_SetMclk_Legacy_isr(output, mclkSource, pllChannel, mclkFreqToFsRatio);
#endif
}

/**************************************************************************/

static void BAPE_SpdifOutput_P_SetMute(BAPE_OutputPort output, bool muted, bool sync)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
    BAPE_SpdifOutput_P_SetMute_IopOut(output, muted, sync);
#else
    BAPE_SpdifOutput_P_SetMute_Legacy(output, muted, sync);
#endif
}

/***************************************************************************
        Private functions: Protyped above
***************************************************************************/

static void BAPE_SpdifOutput_P_SetCbits_isr(BAPE_SpdifOutputHandle handle)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
    BAPE_SpdifOutput_P_SetCbits_IopOut_isr(handle);
#else
    BAPE_SpdifOutput_P_SetCbits_Legacy_isr(handle);
#endif
}

/**************************************************************************/

static BERR_Code BAPE_SpdifOutput_P_OpenHw(BAPE_SpdifOutputHandle handle)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
    return BAPE_SpdifOutput_P_OpenHw_IopOut(handle);
#else
    return BAPE_SpdifOutput_P_OpenHw_Legacy(handle);
#endif
}

/**************************************************************************/

static BERR_Code BAPE_SpdifOutput_P_CloseHw(BAPE_SpdifOutputHandle handle)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
    return BAPE_SpdifOutput_P_CloseHw_IopOut(handle);
#else
    return BAPE_SpdifOutput_P_CloseHw_Legacy(handle);
#endif
}

static BERR_Code BAPE_SpdifOutput_P_SetBurstConfig(BAPE_SpdifOutputHandle handle, bool pcm)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
    return BAPE_SpdifOutput_P_SetBurstConfig_IopOut(handle, pcm);
#else
    return BAPE_SpdifOutput_P_SetBurstConfig_Legacy(handle, pcm);
#endif
}

/**************************************************************************/

static BERR_Code BAPE_SpdifOutput_P_ApplySettings(
    BAPE_SpdifOutputHandle handle,
    const BAPE_SpdifOutputSettings *pSettings,
    bool force
    )
{
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
    return BAPE_SpdifOutput_P_ApplySettings_IopOut(handle, pSettings, force);
#else
    return BAPE_SpdifOutput_P_ApplySettings_Legacy(handle, pSettings, force);
#endif
}

#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
/**************************************************************************
7429-style RDB
**************************************************************************/

/**************************************************************************/

static BERR_Code BAPE_SpdifOutput_P_Open_IopOut(BAPE_SpdifOutputHandle handle)
{
    BKNI_EnterCriticalSection();
    BAPE_SpdifOutput_P_SetMclk_isr(&handle->outputPort, BAPE_MclkSource_ePll0, 0, 256);
    BKNI_LeaveCriticalSection();

    BAPE_SpdifOutput_P_SetBurstConfig(handle, true);
    return BERR_SUCCESS;
}

static void BAPE_SpdifOutput_P_Close_IopOut(BAPE_SpdifOutputHandle handle)
{
    BSTD_UNUSED(handle);
    return;
}

static void BAPE_SpdifOutput_P_SetCbits_IopOut_isr(BAPE_SpdifOutputHandle handle)
{
    BAPE_Reg_P_FieldList regFieldList;
    uint32_t regAddr, regVal;
    unsigned validity = 0;
    unsigned dither = 0;
    bool compressed = false;
    bool compressedAsPcm = false;

    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    if ( handle->outputPort.mixer )
    {
        const BAPE_FMT_Descriptor     *pBfd = BAPE_Mixer_P_GetOutputFormat(handle->outputPort.mixer);

        compressed = BAPE_FMT_P_IsCompressed_isrsafe(pBfd);
        compressedAsPcm = BAPE_FMT_P_IsDtsCdCompressed_isrsafe(pBfd);
    }

    BDBG_MSG(("Set SPDIF CBITS SR %u", handle->sampleRate));

    /* Only set validity if we're outputting compressed in a non-high-bitrate mode. */
    if ( compressed && !compressedAsPcm && handle->sampleRate != 192000 && handle->sampleRate != 176400 )
    {
        validity = 1;
    }
    else if ( !compressed && !compressedAsPcm )
    {
        if ( handle->settings.ditherEnabled )
        {
            dither = 1;
        }
    }

    /* Set Cbit transmission type */
    if ( compressed && !compressedAsPcm )
    {
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, COMP_OR_LINEAR, Compressed);
    }
    else
    {
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, COMP_OR_LINEAR, Linear);
    }

    /* Hold while we update the other bits */
    BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, HOLD_CSTAT, Hold);

    /* Tell the HW to hold the current CBITS and set dither correctly */
    regAddr = BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, handle->index);
    BAPE_Reg_P_InitFieldList_isr(handle->deviceHandle, &regFieldList);
    BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, HOLD_CSTAT, Hold);
    BAPE_Reg_P_AddToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, DITHER_ENA, dither);
    BAPE_Reg_P_AddToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, VALIDITY, validity);
    BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, regAddr);

    /* Program channel status */
    if ( handle->settings.useRawChannelStatus )
    {
        regVal =
            (uint32_t)handle->settings.rawChannelStatus[0] |
            (((uint32_t)handle->settings.rawChannelStatus[1])<<8) |
            (((uint32_t)handle->settings.rawChannelStatus[2])<<16) |
            (((uint32_t)handle->settings.rawChannelStatus[3])<<24);
        BAPE_Reg_P_Write_isr(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CHANSTAT_0, handle->index), regVal);
        regVal = (uint32_t)handle->settings.rawChannelStatus[4];
        BAPE_Reg_P_Write_isr(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CHANSTAT_1, handle->index), regVal);
    }
    else
    {
        BAPE_Spdif_P_ChannelStatusBits cbits;

        BAPE_P_MapSpdifChannelStatusToBits_isr(&handle->outputPort, &handle->settings.channelStatus, &cbits);
        BAPE_Reg_P_Write_isr(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CHANSTAT_0, handle->index), cbits.bits[0]);
        BAPE_Reg_P_Write_isr(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CHANSTAT_1, handle->index), cbits.bits[1]);
    }

    /* Reload the new channel status in HW */
    BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, HOLD_CSTAT, Update);

    return;
}

/**************************************************************************/

static void BAPE_SpdifOutput_P_SetMclk_IopOut_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
    BAPE_Reg_P_FieldList regFieldList;

    BAPE_SpdifOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(handle->offset == 0);

    /* Save the settings in case we need to re-apply them later. */
    handle->mclkInfo.mclkSource         = mclkSource;
    handle->mclkInfo.pllChannel         = pllChannel;
    handle->mclkInfo.mclkFreqToFsRatio  = mclkFreqToFsRatio;

    BAPE_Reg_P_InitFieldList_isr(handle->deviceHandle, &regFieldList);

    switch ( mclkSource )
    {
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0_PLLCLKSEL_PLL0_ch1
    case BAPE_MclkSource_ePll0:
        switch ( pllChannel )
        {
        case 0: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, PLL0_ch1); break;
        case 1: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, PLL0_ch2); break;
        case 2: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, PLL0_ch3); break;
        default: (void) BERR_TRACE(BERR_NOT_SUPPORTED); break;
        }
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0_PLLCLKSEL_PLL1_ch1
    case BAPE_MclkSource_ePll1:
        switch ( pllChannel )
        {
        case 0: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, PLL1_ch1); break;
        case 1: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, PLL1_ch2); break;
        case 2: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, PLL1_ch3); break;
        default: (void) BERR_TRACE(BERR_NOT_SUPPORTED); break;
        }
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0_PLLCLKSEL_PLL2_ch1
    case BAPE_MclkSource_ePll2:
        switch ( pllChannel )
        {
        case 0: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, PLL2_ch1); break;
        case 1: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, PLL2_ch2); break;
        case 2: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, PLL2_ch3); break;
        default: (void) BERR_TRACE(BERR_NOT_SUPPORTED); break;
        }
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen0
    case BAPE_MclkSource_eNco0:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen0);
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen1
    case BAPE_MclkSource_eNco1:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen1);
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen2
    case BAPE_MclkSource_eNco2:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen2);
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen3
    case BAPE_MclkSource_eNco3:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen3);
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen4
    case BAPE_MclkSource_eNco4:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen4);
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen5
    case BAPE_MclkSource_eNco5:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen5);
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen6
    case BAPE_MclkSource_eNco6:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen6);
        break;
#endif
    default:
        BDBG_ERR(("Unsupported clock source %u for SPDIF %u", mclkSource, handle->index));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    switch ( mclkFreqToFsRatio )
    {
    case 128: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, MCLK_RATE, MCLK_128fs_SCLK_64fs); break;
    case 256: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, MCLK_RATE, MCLK_256fs_SCLK_64fs); break;
    case 384: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, MCLK_RATE, MCLK_384fs_SCLK_64fs); break;
    case 512: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, MCLK_RATE, MCLK_512fs_SCLK_64fs); break;
    default:
        BDBG_ERR(("Unsupported MCLK Rate of %uFs", mclkFreqToFsRatio));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        break;
    }
    BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_MCLK_CFG_0, handle->index));
}

/**************************************************************************/

static void BAPE_SpdifOutput_P_SetMute_IopOut(BAPE_OutputPort output, bool muted, bool sync)
{
    BAPE_SpdifOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    /* Only support one now */
    BDBG_ASSERT(handle->offset == 0);

    /* Must manipulate CSTAT registers in critical section */
    BKNI_EnterCriticalSection();

    if ( muted )
    {
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, OVERWRITE_DATA, Enable);
    }
    else
    {
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, OVERWRITE_DATA, Disable);
    }

    BKNI_LeaveCriticalSection();

    handle->muted = muted;
    BDBG_MSG(("SPDIF output (IopOut) mute %u", muted));
    BSTD_UNUSED(sync);
}

/**************************************************************************/

static BERR_Code BAPE_SpdifOutput_P_Enable_IopOut(BAPE_OutputPort output)
{
    BAPE_Reg_P_FieldList regFieldList;
    BAPE_SpdifOutputHandle handle;
    BAPE_MixerHandle mixer;
    unsigned numChannelPairs = 0;
    uint32_t regAddr;
    BAPE_OutputVolume volume;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(false == handle->enabled);
    BDBG_ASSERT(NULL != output->mixer);

    mixer = output->mixer;

    BDBG_MSG(("Enabling %s", handle->name));

    /* apply loudness equivalance levels*/
    BAPE_GetOutputVolume(output, &volume);
    switch (handle->deviceHandle->settings.loudnessMode)
    {
    default:
    case BAPE_LoudnessEquivalenceMode_eNone:
        output->additionalGain = 0;
        break;
    case BAPE_LoudnessEquivalenceMode_eAtscA85:
        output->additionalGain = -7; /* 24dB->31dB*/
        break;
    case BAPE_LoudnessEquivalenceMode_eEbuR128:
        output->additionalGain = -8; /* 23dB->31dB*/
        break;
    }
    BAPE_SetOutputVolume(output, &volume);
    BAPE_SpdifOutput_SetSettings(handle, &handle->settings);

    numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(BAPE_Mixer_P_GetOutputFormat(mixer));

    BDBG_ASSERT(numChannelPairs <= 1);

    regAddr = BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_STREAM_CFG_0, handle->index);
    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
    BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_STREAM_CFG_0, GROUP_ID, 0);
    BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_STREAM_CFG_0, CHANNEL_GROUPING, 1);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_STREAM_CFG_0, STREAM_BIT_RESOLUTION, Res_24_Bit);
    BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_STREAM_CFG_0, FCI_ID, output->sourceMixerFci.ids[0]);
    BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);

    /* Update CBITS */
    BKNI_EnterCriticalSection();
    BAPE_SpdifOutput_P_SetCbits_isr(handle);
    BKNI_LeaveCriticalSection();

    /* Enable the interface. */
    BAPE_Reg_P_UpdateField(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_STREAM_CFG_0, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_STREAM_CFG_0, ENA, 1);

    handle->enabled = true;
    return BERR_SUCCESS;
}

/**************************************************************************/

static void BAPE_SpdifOutput_P_Disable_IopOut(BAPE_OutputPort output)
{
    BAPE_SpdifOutputHandle handle;
    const BAPE_FMT_Descriptor     *pFormat;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    BDBG_ASSERT(true == handle->enabled);

    pFormat = BAPE_Mixer_P_GetOutputFormat(output->mixer);
    BDBG_ASSERT(NULL != pFormat);

    /* If we are PCM we need to disable the stream config so that we hold the channel status */
    /* If we were to disable this for compressed we would loose pause bursts */
    if (BAPE_FMT_P_IsLinearPcm_isrsafe(pFormat))
    {
        BAPE_Reg_P_UpdateField(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_STREAM_CFG_0, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_STREAM_CFG_0, ENA, 0);
    }

    /* Reset source FCI to Invalid */
    BAPE_Reg_P_UpdateField(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_STREAM_CFG_0, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_STREAM_CFG_0, FCI_ID, BAPE_FCI_ID_INVALID);

    handle->enabled = false;
}

/**************************************************************************/

static BERR_Code BAPE_SpdifOutput_P_OpenHw_IopOut(BAPE_SpdifOutputHandle handle)
{
    BERR_Code            errCode = BERR_SUCCESS;
    BAPE_Handle          deviceHandle;
    BAPE_Reg_P_FieldList regFieldList;

    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    /* Enable the clock and data while opening the output. */

    BAPE_Reg_P_InitFieldList(deviceHandle, &regFieldList);

    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG, CLOCK_ENABLE, Enable);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG, DATA_ENABLE, Enable);

    BDBG_MSG(("ENABLE clock and data"));

    BAPE_Reg_P_ApplyFieldList(&regFieldList, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG, handle->index));

#if !(defined BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL_DITHER_VALUE_MASK)
    /* if we don't have a DITHER_VALUE register, then the default dither value is '1'.  
       make sure the default width is 16 bit in that case. */
    BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, DITHER_WIDTH, RES_16);
#endif

#ifdef BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL_WAIT_PCM_TO_COMP_MASK
    BAPE_Reg_P_UpdateField(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, WAIT_PCM_TO_COMP, 2);
#endif

    /* Default cbit Sample Rate to 48k */
    BAPE_Reg_P_Write_isr(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CHANSTAT_0, handle->index), 0x02000000);

    return errCode;
}

static BERR_Code BAPE_SpdifOutput_P_CloseHw_IopOut(BAPE_SpdifOutputHandle handle)
{
    BERR_Code            errCode = BERR_SUCCESS;
    BAPE_Handle          deviceHandle;
    BAPE_Reg_P_FieldList regFieldList;

    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    /* Disable the interface */
    BAPE_Reg_P_UpdateField(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_STREAM_CFG_0, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_STREAM_CFG_0, ENA, 0);

    /* Enable the clock and data while opening the output. */

    BAPE_Reg_P_InitFieldList(deviceHandle, &regFieldList);

    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG, CLOCK_ENABLE, Disable);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG, DATA_ENABLE, Disable);

    BDBG_MSG(("DISABLE clock and data"));

    BAPE_Reg_P_ApplyFieldList(&regFieldList, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG, handle->index));

    return errCode;
}

static BERR_Code BAPE_SpdifOutput_P_SetBurstConfig_IopOut(BAPE_SpdifOutputHandle handle, bool pcm)
{
    BAPE_Reg_P_FieldList regFieldList;

    BSTD_UNUSED( pcm );

    /* Set Burst type */
    BAPE_Reg_P_InitFieldList_isr(handle->deviceHandle, &regFieldList);
    if ( handle->settings.underflowBurst == BAPE_SpdifBurstType_ePause )
    {
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_RAMP_BURST, TYPE, Pause);
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_RAMP_BURST, REP_PERIOD, PER_3);
    }
    else if ( handle->settings.underflowBurst == BAPE_SpdifBurstType_eNull )
    {
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_RAMP_BURST, TYPE, Null);
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_RAMP_BURST, REP_PERIOD, PER_3);
    }
    else
    {
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_RAMP_BURST, REP_PERIOD, None);
    }
    BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_RAMP_BURST, handle->index));

    return BERR_SUCCESS;
}

static BERR_Code BAPE_SpdifOutput_P_ApplySettings_IopOut(
    BAPE_SpdifOutputHandle handle,
    const BAPE_SpdifOutputSettings *pSettings,
    bool force
    )
{
    BAPE_Reg_P_FieldList regFieldList;
    BAPE_Handle          deviceHandle;
    bool burstTypeChanged;
    const BAPE_FMT_Descriptor     *pFormat;
    bool pcm = true;

    BSTD_UNUSED(force); /* Not used because HW gets updated unconditionally */

    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(NULL != pSettings);
    
    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    if ( handle->outputPort.mixer )
    {
        pFormat = BAPE_Mixer_P_GetOutputFormat(handle->outputPort.mixer);
        pcm = BAPE_FMT_P_IsLinearPcm_isrsafe(pFormat);
    }

    BAPE_Reg_P_InitFieldList(deviceHandle, &regFieldList);

    BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG, LIMIT_TO_16_BITS, (pSettings->limitTo16Bits) ? 1 : 0);

    if (pcm)
    {
        switch (pSettings->stereoMode)
        {
        default:
        case BAPE_StereoMode_eLeftRight:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG, LR_SELECT, Normal);
            break;
        case BAPE_StereoMode_eLeftLeft:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG, LR_SELECT, Both_Get_Left);
            break;
        case BAPE_StereoMode_eRightRight:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG, LR_SELECT, Both_Get_Right);
            break;
        case BAPE_StereoMode_eRightLeft:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG, LR_SELECT, Swap);
            break;
        }
    }
    else
    {
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG, LR_SELECT, Normal);
    }

    BAPE_Reg_P_ApplyFieldList(&regFieldList, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG, handle->index));

#if BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL_DITHER_VALUE_MASK
    /* On 28nm DITHER_VALUE defaults to '-1'. Allow 24 bit dither as the default in that case.
       Make sure dither width is compatible with bit resolution.
       Note - diter ENABLE/DISABLE is controlled by cbit ISR */
    if ( pSettings->limitTo16Bits )
    {
        BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, DITHER_WIDTH, RES_16);
    }
    else
    {
        BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BAPE_SPDIF_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_CTRL, DITHER_WIDTH, RES_24);
    }
#endif

    burstTypeChanged = handle->settings.underflowBurst != pSettings->underflowBurst;

    handle->settings = *pSettings;

    /* Remaining fields are handled by MS regs.  Must modify those in critical section. */    
    BKNI_EnterCriticalSection();
    BAPE_SpdifOutput_P_SetCbits_isr(handle);
    BKNI_LeaveCriticalSection();

    /* check/modify burst configuration */
    if ( burstTypeChanged )
    {
        BAPE_SpdifOutput_P_SetBurstConfig(handle, pcm);
    }

    return BERR_SUCCESS;
}
#else
/**************************************************************************
Legacy 7425 style AIO
**************************************************************************/

static BERR_Code BAPE_SpdifOutput_P_Open_Legacy(BAPE_SpdifOutputHandle handle)
{
    handle->pMuteBuffer = BMEM_Heap_AllocAligned(handle->deviceHandle->memHandle, BAPE_P_MUTE_BUFFER_SIZE, 8, 0);
    if ( NULL == handle->pMuteBuffer )
    {
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }

    handle->currentMuteBufferType = BAPE_SpdifBurstType_eMax;
    BAPE_SpdifOutput_P_SetBurstConfig(handle, true);

    return BERR_SUCCESS;
}

static void BAPE_SpdifOutput_P_Close_Legacy(BAPE_SpdifOutputHandle handle)
{
    if ( handle->pMuteBuffer )
    {
        BMEM_Heap_Free(handle->deviceHandle->memHandle, handle->pMuteBuffer);
    }
}

static void BAPE_SpdifOutput_P_SetCbits_Legacy_isr(BAPE_SpdifOutputHandle handle)
{
    uint32_t regAddr, regVal;
    unsigned validity = 0;
    BAPE_DataType dataType = BAPE_DataType_ePcmStereo;
    unsigned compressed = 0;
    unsigned dither = 0;
    bool compressedAsPcm = false;
    const BAPE_FMT_Descriptor     *pFormat;

    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    if ( handle->outputPort.mixer )
    {
        pFormat = BAPE_Mixer_P_GetOutputFormat(handle->outputPort.mixer);

        dataType = pFormat->type;
        compressed = (unsigned)BAPE_FMT_P_IsCompressed_isrsafe(pFormat);
        compressedAsPcm = BAPE_FMT_P_IsDtsCdCompressed_isrsafe(pFormat);
    }
    else
    {
        /* No update required, maintain previous settings until connected.  Update will be applied in Enable(). */
        return;
    }

    /* Only set validity if we're outputting compressed */
    if ( compressed && !compressedAsPcm )
    {
        validity = 1;
    }
    else if ( !compressedAsPcm )
    {
        if ( handle->settings.ditherEnabled )
        {
            dither = 1;
        }
    }

    BDBG_MSG(("Set SPDIF HW CBITS SR %u compressed %u validity %u dither %u", handle->sampleRate, compressed, validity, dither));
    /* Tell the HW to hold the current CBITS and set dither correctly */
    regAddr = BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0;
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, HOLD_CSTAT, Hold);
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);

    regVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, DITHER_ENA)|
                BCHP_MASK(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, VALIDITY));
    regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, DITHER_ENA, dither);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, VALIDITY, validity);
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);

    /* Program channel status */
    if ( handle->settings.useRawChannelStatus )
    {
        regVal =
            (uint32_t)handle->settings.rawChannelStatus[0] |
            (((uint32_t)handle->settings.rawChannelStatus[1])<<8) |
            (((uint32_t)handle->settings.rawChannelStatus[2])<<16) |
            (((uint32_t)handle->settings.rawChannelStatus[3])<<24);
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, regVal);
        regVal = (uint32_t)handle->settings.rawChannelStatus[4];
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, regVal);
    }
    else if ( compressed && !compressedAsPcm )
    {
        regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0);
        regVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_PRO_CONS)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_COMP_LIN)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_CP)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_EMPH)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_CMODE)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_CATEGORY)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_SOURCE)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_FREQ)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_ACCURACY)|
#ifdef BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0_Compressed_BITS_31_to_30_MASK
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_BITS_31_to_30)
#else
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_FREQ_EXTN)
#endif
                    );
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_PRO_CONS, handle->settings.channelStatus.professional);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_COMP_LIN, 1);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_CP, (handle->settings.channelStatus.copyright)?0:1);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_CATEGORY, handle->settings.channelStatus.categoryCode);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_SOURCE, handle->settings.channelStatus.sourceNumber);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_FREQ, BAPE_P_GetSampleRateCstatCode_isr(handle->sampleRate));
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, Compressed_ACCURACY, handle->settings.channelStatus.clockAccuracy);
        BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, regVal);
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, 0);
    }
    else
    {
        regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0);
        regVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_PRO_CONS)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_COMP_LIN)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_CP)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_EMPH)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_CMODE)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_CATEGORY)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_SOURCE)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_FREQ)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_ACCURACY)|
#ifdef BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0_Compressed_BITS_31_to_30_MASK
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_BITS_31_to_30)
#else
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_FREQ_EXTN)
#endif
                    );
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_PRO_CONS, handle->settings.channelStatus.professional);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_CP, (handle->settings.channelStatus.copyright)?0:1);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_CATEGORY, handle->settings.channelStatus.categoryCode);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_SOURCE, handle->settings.channelStatus.sourceNumber);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_FREQ, BAPE_P_GetSampleRateCstatCode_isr(handle->sampleRate));
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_ACCURACY, handle->settings.channelStatus.clockAccuracy);
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, regVal);
        regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0);
        regVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, PCM_MAX_LEN)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, PCM_LENGTH)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, PCM_ORIG_FREQ));
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, PCM_MAX_LEN, 1);    /* 24-bits */
        #ifdef BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0_PCM_CGMS_A_MASK
        regVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, PCM_CGMS_A));
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, PCM_CGMS_A, handle->settings.channelStatus.cgmsA);
        #endif
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_0, regVal);
    }

    /* Begin using new bits */
    regAddr = BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0;
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, HOLD_CSTAT);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, HOLD_CSTAT, Update);
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);

    return;
}

/**************************************************************************/

static void BAPE_SpdifOutput_P_SetMclk_Legacy_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
    uint32_t regAddr, regVal;
    uint32_t pllclksel, mclkRate;

    BAPE_SpdifOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(handle->offset == 0);

    /* Save the settings in case we need to re-apply them later. */
    handle->mclkInfo.mclkSource         = mclkSource;
    handle->mclkInfo.pllChannel         = pllChannel;
    handle->mclkInfo.mclkFreqToFsRatio  = mclkFreqToFsRatio;

    switch ( mclkSource )
    {
    /* SPDIF Timing */
    #if BAPE_CHIP_MAX_PLLS > 0
    case BAPE_MclkSource_ePll0:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0_PLLCLKSEL_PLL0_ch1 + pllChannel;
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 1
    case BAPE_MclkSource_ePll1:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0_PLLCLKSEL_PLL1_ch1 + pllChannel;
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 2
    case BAPE_MclkSource_ePll2:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0_PLLCLKSEL_PLL2_ch1 + pllChannel;
        break;
    #endif
    
    /* DAC Timing */
    #if BAPE_CHIP_MAX_DACS > 0
    case BAPE_MclkSource_eHifidac0:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0_PLLCLKSEL_Hifidac0;
        break;
    #endif
    #if BAPE_CHIP_MAX_DACS > 1
    case BAPE_MclkSource_eHifidac1:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0_PLLCLKSEL_Hifidac1;
        break;
    #endif
    #if BAPE_CHIP_MAX_DACS > 2
    case BAPE_MclkSource_eHifidac2:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0_PLLCLKSEL_Hifidac2;
        break;
    #endif
    
    /* NCO Timing */
    #if BAPE_CHIP_MAX_NCOS > 0
    case BAPE_MclkSource_eNco0:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0_PLLCLKSEL_Mclk_gen0;
        break;
    #endif
    #if BAPE_CHIP_MAX_NCOS > 1
    case BAPE_MclkSource_eNco1:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0_PLLCLKSEL_Mclk_gen1;
        break;
    #endif
    #if BAPE_CHIP_MAX_NCOS > 2
    case BAPE_MclkSource_eNco2:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0_PLLCLKSEL_Mclk_gen2;
        break;
    #endif
    
    default:
        BDBG_ERR(("mclkSource (%u) doesn't refer to a valid PLL or DAC", mclkSource));
        BDBG_ASSERT(false);     /* something went wrong somewhere! */
        return;
    }

    /*  PLLs are the recommended timing source for SPDIF, so print a warning if they're using a DAC rate manager. */
    if ( BAPE_MCLKSOURCE_IS_DAC(mclkSource) )
    {
        BDBG_WRN(("DAC timing source has been selected for SPDIF output %u.", handle->index));
        BDBG_WRN(("It is strongly recommended to place SPDIF and DAC outputs on separate mixers."));
    }
    else if ( BAPE_MCLKSOURCE_IS_NCO(mclkSource) )
    {
        BDBG_WRN(("NCO timing source has been selected for SPDIF output %u.", handle->index));
        BDBG_WRN(("It is strongly recommended to use PLL timing for SPDIF."));
    }


    /* Tell the output formatter how fast our mclk is. */
    mclkRate = mclkFreqToFsRatio / ( 128 );  /* mclkRate (for SPDIF) is in multiples of 128Fs */

    /* Choose the register for the appropriate output. */
    regAddr = GET_SPDIF_REG_ADDR2(BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF, handle->index);

    /* Read the register and clear the fields that we're going to fill in. */
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, PLLCLKSEL)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, MCLK_RATE));

    /* Fill in the MCLK_RATE and PLLCLKSEL fields. */
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, MCLK_RATE, mclkRate);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, PLLCLKSEL, pllclksel);

    /* Then write it to the reg. */
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);
}

static void BAPE_SpdifOutput_P_SetMute_Legacy(BAPE_OutputPort output, bool muted, bool sync)
{
    BAPE_SpdifOutputHandle handle;
    BAPE_MixerGroupInputSettings dataInputSettings, burstInputSettings;
    bool compressed = false;
    const BAPE_FMT_Descriptor *pFormat = NULL;

    BSTD_UNUSED(sync);
    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    if ( output->mixer )
    {
        pFormat = BAPE_Mixer_P_GetOutputFormat(output->mixer);
        BDBG_ASSERT(NULL != pFormat);
        compressed = BAPE_FMT_P_IsCompressed_isrsafe(pFormat);
    }

    if ( compressed )
    {
        BAPE_MixerGroup_P_GetInputSettings(handle->hMixer, 0, &dataInputSettings);
        BAPE_MixerGroup_P_GetInputSettings(handle->hMixer, 1, &burstInputSettings);
        dataInputSettings.rampStep = 0x800000;
        burstInputSettings.rampStep = 0x800000;

        if ( muted )
        {
            dataInputSettings.coefficients[0][0][0] = 0;
            dataInputSettings.coefficients[0][1][1] = 0;
            burstInputSettings.coefficients[0][0][0] = 0x800000;
            burstInputSettings.coefficients[0][1][1] = 0x800000;
        }
        else
        {
            dataInputSettings.coefficients[0][0][0] = 0x800000;
            dataInputSettings.coefficients[0][1][1] = 0x800000;
            burstInputSettings.coefficients[0][0][0] = 0;
            burstInputSettings.coefficients[0][1][1] = 0;
        }
        (void)BAPE_MixerGroup_P_SetInputSettings(handle->hMixer, 0, &dataInputSettings);
        (void)BAPE_MixerGroup_P_SetInputSettings(handle->hMixer, 1, &burstInputSettings);
    }
    handle->muted = muted;
}

/**************************************************************************/

static BERR_Code BAPE_SpdifOutput_P_Enable_Legacy(BAPE_OutputPort output)
{
    BAPE_SpdifOutputHandle handle;
    uint32_t regVal;
    unsigned streamId;
    const BAPE_FMT_Descriptor *pFormat;
    BAPE_MixerGroupInputSettings dataInputSettings, burstInputSettings;
    BAPE_OutputVolume volume;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(false == handle->enabled);
    BDBG_ASSERT(NULL != output->mixer);

    /* apply loudness equivalance levels*/
    BAPE_GetOutputVolume(output, &volume);
    switch (handle->deviceHandle->settings.loudnessMode)
    {
    default:
    case BAPE_LoudnessEquivalenceMode_eNone:
        output->additionalGain = 0;
        break;
    case BAPE_LoudnessEquivalenceMode_eAtscA85:
        output->additionalGain = -7; /* 24dB->31dB*/
        break;
    case BAPE_LoudnessEquivalenceMode_eEbuR128:
        output->additionalGain = -8; /* 23dB->31dB*/
        break;
    }
    BAPE_SetOutputVolume(output, &volume);
    BAPE_SpdifOutput_SetSettings(handle, &handle->settings);

    pFormat = BAPE_Mixer_P_GetOutputFormat(output->mixer);
    BDBG_ASSERT(NULL != pFormat);
    streamId = GET_SPDIF_STREAM_ID(handle->index);
    BDBG_MSG(("Enabling %s [stream %u]", handle->name, streamId));

    BAPE_MixerGroup_P_GetInputSettings(handle->hMixer, 0, &dataInputSettings);
    BAPE_MixerGroup_P_GetInputSettings(handle->hMixer, 1, &burstInputSettings);
    dataInputSettings.input = handle->outputPort.sourceMixerFci;
    if ( handle->muted && BAPE_FMT_P_IsCompressed_isrsafe(pFormat) )
    {
        /* If switching between compressed<->pcm we need to ensure pausebursts are only sent in compressed mode */
        dataInputSettings.coefficients[0][0][0] = 0;   /* Mute data path */
        dataInputSettings.coefficients[0][1][1] = 0;
        burstInputSettings.coefficients[0][0][0] = 0x800000;    /* Enable pauseburst */
        burstInputSettings.coefficients[0][1][1] = 0x800000;
    }
    else
    {
        dataInputSettings.coefficients[0][0][0] = 0x800000;    /* Enable data path */
        dataInputSettings.coefficients[0][1][1] = 0x800000;
        burstInputSettings.coefficients[0][0][0] = 0;   /* Mute pauseburst */
        burstInputSettings.coefficients[0][1][1] = 0;
    }
    (void)BAPE_MixerGroup_P_SetInputSettings(handle->hMixer, 0, &dataInputSettings);
    (void)BAPE_MixerGroup_P_SetInputSettings(handle->hMixer, 1, &burstInputSettings);

    BAPE_SpdifOutput_P_SetBurstConfig(handle, BAPE_FMT_P_IsLinearPcm_isrsafe(pFormat));

    if ( BAPE_FMT_P_IsLinearPcm_isrsafe(pFormat) )
    {
        BAPE_SpdifOutput_P_SetMute(output, handle->muted, false);

        BKNI_EnterCriticalSection();
        /* Make sure CBITS are correct for current format */
        BAPE_SpdifOutput_P_SetCbits_isr(handle);
        BKNI_LeaveCriticalSection();
    }
    else
    {
        BKNI_EnterCriticalSection();
        /* Make sure CBITS are correct for current format */
        BAPE_SpdifOutput_P_SetCbits_isr(handle);
        BKNI_LeaveCriticalSection();

        BAPE_SpdifOutput_P_SetMute(output, handle->muted, false);
    }

    BKNI_EnterCriticalSection();
    /* enable stream 0 */
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA);
    regVal |= (BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA, Enable));
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, regVal);
    BKNI_LeaveCriticalSection();

    (void)BAPE_MixerGroup_P_StartInput(handle->hMixer, 0);  /* Input for data path */

    handle->enabled = true;
    return BERR_SUCCESS;
}

/**************************************************************************/

static void BAPE_SpdifOutput_P_Disable_Legacy(BAPE_OutputPort output)
{
    BAPE_SpdifOutputHandle handle;
    uint32_t regVal;
    BAPE_MixerGroupInputSettings mixerInputSettings;
    unsigned streamId;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    if ( !handle->enabled )
    {
        return;
    }

    streamId = GET_SPDIF_STREAM_ID(handle->index);
    BDBG_MSG(("Disabling %s [stream %u]", handle->name, streamId));

    /* Disable this stream in the MS FW */
    BKNI_EnterCriticalSection();
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA);
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, regVal);
    BKNI_LeaveCriticalSection();

    /* Stop the data path to our local mixer */
    BAPE_MixerGroup_P_StopInput(handle->hMixer, 0);

    /* Reset source FCI to Invalid */
    BAPE_MixerGroup_P_GetInputSettings(handle->hMixer, 0, &mixerInputSettings);
    BAPE_FciIdGroup_Init(&mixerInputSettings.input);
    (void)BAPE_MixerGroup_P_SetInputSettings(handle->hMixer, 0, &mixerInputSettings);

    handle->enabled = false;
}

/**************************************************************************/

static BERR_Code BAPE_SpdifOutput_P_OpenHw_Legacy(BAPE_SpdifOutputHandle handle)
{
    BERR_Code       errCode = BERR_SUCCESS;
    BAPE_Handle     deviceHandle;
    uint32_t        regAddr, regVal;
    unsigned        streamId;
    BAPE_SfifoGroupCreateSettings sfifoCreateSettings;
    BAPE_MixerGroupCreateSettings mixerCreateSettings;
    BAPE_SfifoGroupSettings sfifoSettings;
    BAPE_MixerGroupSettings mixerSettings;
    BAPE_MixerGroupInputSettings mixerInputSettings;
    BAPE_FciIdGroup fciGroup;
    BAPE_Reg_P_FieldList regFieldList;
    BAPE_IopStreamSettings streamSettings;

    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    /* Enable the clock and data while opening the output port. Never disable it */
    regAddr = BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 + handle->offset;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0, CLOCK_ENABLE)|
                BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0,DATA_ENABLE));
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_SPDIF_CFG_0, CLOCK_ENABLE, Enable);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_SPDIF_CFG_0, DATA_ENABLE, Enable);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

    streamId = GET_SPDIF_STREAM_ID(handle->index);

    /* Reset source FCI to Invalid */
    BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
    streamSettings.input = BAPE_SPDIF_INVALID_FCI_SOURCE;
    errCode = BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
    BDBG_ASSERT(BERR_SUCCESS == errCode);

    BAPE_SfifoGroup_P_GetDefaultCreateSettings(&sfifoCreateSettings);
    sfifoCreateSettings.numChannelPairs = 1;
    sfifoCreateSettings.ppmCorrection = false;
    errCode = BAPE_SfifoGroup_P_Create(handle->deviceHandle, &sfifoCreateSettings, &handle->hSfifo);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    BAPE_MixerGroup_P_GetDefaultCreateSettings(&mixerCreateSettings);
    mixerCreateSettings.numChannelPairs = 1;
    errCode = BAPE_MixerGroup_P_Create(handle->deviceHandle, &mixerCreateSettings, &handle->hMixer);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BAPE_SfifoGroup_P_GetSettings(handle->hSfifo, &sfifoSettings);
    sfifoSettings.dataWidth = 16;
    sfifoSettings.sampleRepeatEnabled = false;
    sfifoSettings.interleaveData = true;
    sfifoSettings.loopAround = true;
    BMEM_Heap_ConvertAddressToOffset(handle->deviceHandle->memHandle, handle->pMuteBuffer, &sfifoSettings.bufferInfo[0].base);
    sfifoSettings.bufferInfo[0].length = sizeof(g_pauseburst)*64;
    sfifoSettings.bufferInfo[0].wrpoint = sfifoSettings.bufferInfo[0].base;
    errCode = BAPE_SfifoGroup_P_SetSettings(handle->hSfifo, &sfifoSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BAPE_MixerGroup_P_GetSettings(handle->hMixer, &mixerSettings);
    mixerSettings.volumeControlEnabled = true;
    errCode = BAPE_MixerGroup_P_SetSettings(handle->hMixer, &mixerSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Link pauseburst SFIFO to the mixer's second input (actual data will be first input) */
    BAPE_MixerGroup_P_GetInputSettings(handle->hMixer, 1, &mixerInputSettings);
    BAPE_SfifoGroup_P_GetOutputFciIds(handle->hSfifo, &mixerInputSettings.input);
    mixerInputSettings.coefficients[0][0][0] = 0;
    mixerInputSettings.coefficients[0][0][1] = 0;
    mixerInputSettings.coefficients[0][1][0] = 0;
    mixerInputSettings.coefficients[0][1][1] = 0;
    mixerInputSettings.rampStep = 0x800000;
    errCode = BAPE_MixerGroup_P_SetInputSettings(handle->hMixer, 1, &mixerInputSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Setup stereo data path as mixer output 0 */
    BAPE_MixerGroup_P_GetOutputFciIds(handle->hMixer, 0, &fciGroup);
    BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
    streamSettings.resolution = 24;
    streamSettings.input = fciGroup.ids[0];
    (void)BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
    
    /* Write the enable bit in the OP (only stereo) */
    BREG_Write32(handle->deviceHandle->regHandle, 
                 BCHP_AUD_FMM_OP_CTRL_ENABLE_SET,
                 (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(streamId));

    /* Setup Channel Status Formatter and enable always.  This solves a lot of receiver compatibility issues. */

    /* Enable HW cbit formatter */
    BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS, AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM0, Bypass);
    BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0, ENABLE, Enable);

    /* Disable FW cbit formatter */
    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
    BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, OVERWRITE_DATA, Disable);
    BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA, Disable);
    BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0);

    /* Default cbit Sample Rate to 48k */    
    BAPE_Reg_P_UpdateField(handle->deviceHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0, PCM_FREQ, 2);

    /* Set Default Cbits */
    BKNI_EnterCriticalSection();
    BAPE_SpdifOutput_P_SetCbits_isr(handle);
    BKNI_LeaveCriticalSection();

    /* Enable microsequencer */
    BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA, AUD_FMM_MS_CTRL_STRM_ENA, STREAM0_ENA, Enable);

    /* Enable data source and pauseburst ringbuffer */
    (void)BAPE_MixerGroup_P_StartOutput(handle->hMixer, 0);
    (void)BAPE_MixerGroup_P_StartInput(handle->hMixer, 1);  /* Input for pauseburst */
    (void)BAPE_SfifoGroup_P_Start(handle->hSfifo, false);   /* Sfifo for pauseburst */

    return errCode;
}

/**************************************************************************/

static BERR_Code BAPE_SpdifOutput_P_CloseHw_Legacy(BAPE_SpdifOutputHandle handle)
{
    BERR_Code       errCode = BERR_SUCCESS;
    BAPE_Handle     deviceHandle;
    uint32_t        regAddr, regVal;
    unsigned        streamId;

    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    BAPE_SfifoGroup_P_Stop(handle->hSfifo);
    BAPE_MixerGroup_P_StopInput(handle->hMixer, 1);
    BAPE_MixerGroup_P_StopOutput(handle->hMixer, 0);

    /* Disable microsequencer */
    BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA, AUD_FMM_MS_CTRL_STRM_ENA, STREAM0_ENA, Disable);

    /* Clear the enable bit in the OP */
    streamId = GET_SPDIF_STREAM_ID(handle->index);
    BREG_Write32(handle->deviceHandle->regHandle, 
                 BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR,
                 BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_CLEAR, STREAM0_ENA)<<streamId);

    /* Disable the Spdif clock and data. */
    regAddr = BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 + handle->offset;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0, CLOCK_ENABLE)|
                BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0,DATA_ENABLE));
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_SPDIF_CFG_0, CLOCK_ENABLE, Disable);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_SPDIF_CFG_0, DATA_ENABLE, Disable);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

    if ( handle->hSfifo )
    {
        BAPE_SfifoGroup_P_Destroy(handle->hSfifo);
        handle->hSfifo = NULL;
    }

    if ( handle->hMixer )
    {
        BAPE_MixerGroup_P_Destroy(handle->hMixer);
        handle->hMixer = NULL;
    }

    return errCode;
}

static BERR_Code BAPE_SpdifOutput_P_SetBurstConfig_Legacy(BAPE_SpdifOutputHandle handle, bool pcm)
{
    unsigned i;
    uint16_t *pCached;

    if (pcm)
    {
        if(handle->currentMuteBufferType == BAPE_SpdifBurstType_eNone)
        {
            return BERR_SUCCESS;
        }
    }
    else
    {
        if (handle->currentMuteBufferType == handle->settings.underflowBurst)
        {
            return BERR_SUCCESS;
        }
    }

    if (handle->hSfifo) 
    {
        BAPE_SfifoGroup_P_Stop(handle->hSfifo); /* stop the fifo for mute buffer to clear it */
    }

    (void)BMEM_Heap_ConvertAddressToCached(handle->deviceHandle->memHandle, handle->pMuteBuffer, (void **)&pCached);

    BDBG_MSG(("filling Burst RBUF addr %p/%p, size %u with underflowBurst=%d(%s)", 
        (void *)handle->pMuteBuffer, (void *)pCached, BAPE_P_MUTE_BUFFER_SIZE, handle->settings.underflowBurst,
        (handle->settings.underflowBurst==BAPE_SpdifBurstType_ePause)?"Pause Bursts" : (handle->settings.underflowBurst==BAPE_SpdifBurstType_eNull)?"NULL Bursts" : "Zeros"));

    BKNI_Memset( pCached, 0, BAPE_P_MUTE_BUFFER_SIZE );

    if ( handle->settings.underflowBurst == BAPE_SpdifBurstType_ePause )
    {
        for ( i = 0; i < (BAPE_P_MUTE_BUFFER_SIZE/sizeof(g_pauseburst)); i++ )
        {
            pCached[6*i] = g_pauseburst[0];
            pCached[(6*i)+1] = g_pauseburst[1];
            pCached[(6*i)+2] = g_pauseburst[2];
            pCached[(6*i)+3] = g_pauseburst[3];
            pCached[(6*i)+4] = g_pauseburst[4];
            pCached[(6*i)+5] = g_pauseburst[5];
        }
    }
    else if ( handle->settings.underflowBurst == BAPE_SpdifBurstType_eNull )
    {
        for ( i = 0; i < (BAPE_P_MUTE_BUFFER_SIZE/sizeof(g_nullburst)); i++ )
        {
            pCached[4*i] = g_nullburst[0];
            pCached[(4*i)+1] = g_nullburst[1];
            pCached[(4*i)+2] = g_nullburst[2];
            pCached[(4*i)+3] = g_nullburst[3];
        }
    }

    BMEM_Heap_FlushCache(handle->deviceHandle->memHandle, pCached, sizeof(g_pauseburst)*64);

    if (handle->hSfifo)
    {
        (void)BAPE_SfifoGroup_P_Start(handle->hSfifo, false);   /* Sfifo for pauseburst */
    }

    if (!pcm)
    {
        handle->currentMuteBufferType = handle->settings.underflowBurst;
    }
    else
    {
        handle->currentMuteBufferType = BAPE_SpdifBurstType_eNone;
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_SpdifOutput_P_ApplySettings_Legacy(
    BAPE_SpdifOutputHandle handle,
    const BAPE_SpdifOutputSettings *pSettings,
    bool force
    )
{
    uint32_t regAddr, regVal;
    BAPE_Reg_P_FieldList regFieldList;
    bool burstTypeChanged;
    const BAPE_FMT_Descriptor     *pFormat;
    bool pcm = true;

    BSTD_UNUSED(force); /* Not used because HW gets updated unconditionally */

    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(NULL != pSettings);
    
    if ( handle->outputPort.mixer )
    {
        pFormat = BAPE_Mixer_P_GetOutputFormat(handle->outputPort.mixer);
        pcm = BAPE_FMT_P_IsLinearPcm_isrsafe(pFormat);
    }

    regAddr = BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 + handle->offset;
    regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0, LR_SELECT)|
                BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0, LIMIT_TO_16_BITS));
    if (pcm)
    {
        regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_SPDIF_CFG_0, LR_SELECT, pSettings->stereoMode);
    }
    else
    {
        regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_SPDIF_CFG_0, LR_SELECT, BAPE_StereoMode_eLeftRight);
    }
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_SPDIF_CFG_0, LIMIT_TO_16_BITS, (pSettings->limitTo16Bits?1:0));
    BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);

    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
    if ( pSettings->underflowBurst == BAPE_SpdifBurstType_ePause )
    {
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_MS_CTRL_FW_BURST_0, TYPE, Pause);
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_MS_CTRL_FW_BURST_0, REP_PERIOD, PER_3);
    }
    else
    {
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_MS_CTRL_FW_BURST_0, TYPE, Null);
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_MS_CTRL_FW_BURST_0, REP_PERIOD, None);
    }
    BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, BCHP_AUD_FMM_MS_CTRL_FW_BURST_0);

    burstTypeChanged = handle->settings.underflowBurst != pSettings->underflowBurst;

    handle->settings = *pSettings;

    /* Remaining fields are handled by MS regs.  Must modify those in critical section. */    
    BKNI_EnterCriticalSection();
    BAPE_SpdifOutput_P_SetCbits_isr(handle);
    BKNI_LeaveCriticalSection();

    /* check/modify burst configuration */
    if ( burstTypeChanged )
    {
        BAPE_SpdifOutput_P_SetBurstConfig(handle, pcm);
    }

    return BERR_SUCCESS;
}
#endif

#else

void BAPE_SpdifOutput_GetDefaultSettings(
    BAPE_SpdifOutputSettings *pSettings
    )
{
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_SpdifOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_SpdifOutputSettings *pSettings,
    BAPE_SpdifOutputHandle *pHandle             /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(index);
    BSTD_UNUSED(pHandle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


void BAPE_SpdifOutput_Close(
    BAPE_SpdifOutputHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void BAPE_SpdifOutput_GetSettings(
    BAPE_SpdifOutputHandle handle,
    BAPE_SpdifOutputSettings *pSettings     /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}


BERR_Code BAPE_SpdifOutput_SetSettings(
    BAPE_SpdifOutputHandle handle,
    const BAPE_SpdifOutputSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_SpdifOutput_GetOutputPort(
    BAPE_SpdifOutputHandle handle,
    BAPE_OutputPort *pOutputPort        /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pOutputPort);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
}

#endif

#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0 || BAPE_CHIP_MAX_MAI_OUTPUTS > 0
/***************************************************************************
Summary:
Setup Channel Status Bits for SPDIF or HDMI outputs
***************************************************************************/
void BAPE_P_MapSpdifChannelStatusToBits_isr(
    BAPE_OutputPort output,
    const BAPE_SpdifChannelStatus *pChannelStatus,
    BAPE_Spdif_P_ChannelStatusBits *pBits           /* [out] */
    )
{
    bool compressed;
    bool compressedAsPcm = false;
    unsigned sampleRate;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);
    BDBG_ASSERT(NULL != pChannelStatus);
    BDBG_ASSERT(NULL != pBits);

    pBits->bits[0] = 0;
    pBits->bits[1] = 0;
    pBits->bits[2] = 0;

    if ( output->mixer )
    {
        const BAPE_FMT_Descriptor     *pBfd = BAPE_Mixer_P_GetOutputFormat(output->mixer);

        compressed = BAPE_FMT_P_IsCompressed_isrsafe(pBfd);
        compressedAsPcm = BAPE_FMT_P_IsDtsCdCompressed_isrsafe(pBfd);
        sampleRate = (pBfd->type == BAPE_DataType_eIec61937x16) ?  768000 :  pBfd->sampleRate;
    }
    else
    {
        compressed = false;
        sampleRate = 48000;
    }
    
    if ( pChannelStatus->professional )
    {
        pBits->bits[0] |= 0x01 <<  0;   /* PRO_CONS */
    }
    if ( compressed && !compressedAsPcm )
    {
        pBits->bits[0] |= 0x01 <<  1;   /* COMP_LIN */
    }
    if ( !pChannelStatus->copyright )
    {
        pBits->bits[0] |= 0x01 <<  2;   /* CP */
    }
    /* EMPH = 0 */
    /* CMODE = 0 */
    pBits->bits[0] |= ((uint32_t)pChannelStatus->categoryCode&0xff) <<  8;   /* CATEGORY */
    pBits->bits[0] |= ((uint32_t)pChannelStatus->sourceNumber&0x0f) << 16;   /* SOURCE */
    pBits->bits[0] |= ((uint32_t)BAPE_P_GetSampleRateCstatCode_isr(sampleRate)) << 24;   /* FREQ */
    pBits->bits[0] |= ((uint32_t)pChannelStatus->clockAccuracy&0x03) << 28;   /* ACCURACY */
    /* FREQ_EXTN = 0 */

    if ( compressed && !compressedAsPcm )  
    {
        /* Compressed leaves word1 at 0 */
    }
    else
    {
        pBits->bits[1] |= 0x01 << 0;   /* MAX_LEN = 24 bits */
        /* LENGTH = 0 [not indicated] */
        /* ORIG_FREQ = 0 [not indicated] */
        pBits->bits[1] |= ((uint32_t)pChannelStatus->cgmsA&0x03) << 8;   /* CGMS_A */
    }

    /* Done */
}

#endif

