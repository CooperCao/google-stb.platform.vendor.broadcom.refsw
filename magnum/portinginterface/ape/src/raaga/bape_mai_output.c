/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#include "bape_buffer.h"

#if defined BCHP_DVP_CFG_REG_START
#include "bape_dma_output_priv.h"
#endif

BDBG_MODULE(bape_mai_output);
BDBG_FILE_MODULE(bape_loudness);
BDBG_FILE_MODULE(bape_fci);


#if BAPE_CHIP_MAX_MAI_OUTPUTS > 0   /* If no MAI outputs, then skip all of this and just put in stub funcs at bottom of file. */

BDBG_OBJECT_ID(BAPE_MaiOutput);

typedef enum BAPE_MaiOutputDataPath
{
    BAPE_MaiOutputDataPath_eNone,
    BAPE_MaiOutputDataPath_eStereo,
    BAPE_MaiOutputDataPath_eMaiMulti,
    BAPE_MaiOutputDataPath_eHbr,
    BAPE_MaiOutputDataPath_eMax
} BAPE_MaiOutputDataPath;

typedef struct BAPE_MaiOutput
{
    BDBG_OBJECT(BAPE_MaiOutput)
    BAPE_Handle deviceHandle;
    BAPE_MaiOutputSettings settings;
    unsigned index;
    BAPE_OutputPortObject outputPort;
    unsigned offset;
    unsigned sampleRate;
    struct
    {
        BAPE_MclkSource mclkSource;
        unsigned pllChannel;    /* only applies if mclkSource refers to a PLL */
        unsigned mclkFreqToFsRatio;
    } mclkInfo;
    bool enabled;
    char name[7];   /* MAI %d */
    bool muted;
    BAPE_MaiOutputInterruptHandlers interrupts;

    /* The following are used to generate a pauseburst compressed mute on legacy chips */
    BMMA_Block_Handle muteBufferBlock;
    BMMA_DeviceOffset muteBufferOffset;
    void *pMuteBuffer;
    BAPE_SfifoGroupHandle hSfifo;
    BAPE_MixerGroupHandle hMixer;
    BAPE_MaiOutputDataPath dataPath;
    bool lowLatencyMode;
    bool honorHwMute;

#if defined BCHP_DVP_CFG_REG_START
    /* necessary for SPDIF encoding */
    struct {
        unsigned subFrames;
        unsigned clk;
        uint32_t channelStatus[6];
        bool cpuEncode;
    } spdif;

    BAPE_DmaOutputHandle dma;
#endif
} BAPE_MaiOutput;

#define BAPE_MAI_OUTPUT_MAX_CHANNEL_PAIRS 4

/* Currently all chips support only one -- TODO: HBR? */
#define GET_MAI_MULTI_STREAM_ID(idx, chPair) ((chPair)+3)
#define GET_MAI_STEREO_STREAM_ID(idx) (1)

#if (defined BCHP_AUD_FMM_OP_CTRL_ENABLE_SET_STREAM10_ENA_MASK)
#define GET_MAI_HBR_STREAM_ID(idx) (BCHP_AUD_FMM_OP_CTRL_ENABLE_SET_STREAM10_ENA_SHIFT)
#elif (defined BCHP_AUD_FMM_OP_CTRL_ENABLE_SET_STREAM9_ENA_MASK)
#define GET_MAI_HBR_STREAM_ID(idx) (BCHP_AUD_FMM_OP_CTRL_ENABLE_SET_STREAM9_ENA_SHIFT)
#endif

#define GET_MAI_CBIT_STREAM_ID(idx) (1)

/* Static function prototypes */
static void      BAPE_MaiOutput_P_SetCbits_isr(BAPE_MaiOutputHandle handle);
static uint32_t  BAPE_MaiOutput_P_SampleRateToMaiFormat_isrsafe(unsigned sampleRate);
static BERR_Code BAPE_MaiOutput_P_OpenHw(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_CloseHw(BAPE_MaiOutputHandle handle);
static BERR_Code BAPE_MaiOutput_P_SetBurstConfig(BAPE_MaiOutputHandle handle);
static BERR_Code BAPE_MaiOutput_P_SetCrossbar(BAPE_MaiOutputHandle handle, BAPE_StereoMode stereoMode);
static BERR_Code BAPE_MaiOutput_P_ApplySettings(BAPE_MaiOutputHandle handle, const BAPE_MaiOutputSettings *pSettings, bool force);
static void      BAPE_MaiOutput_P_SetLoudnessEquivlanceVolume(BAPE_OutputPort output);

/* Output port callbacks */
static void      BAPE_MaiOutput_P_SetTimingParams_isr(BAPE_OutputPort output, unsigned sampleRate, BAVC_Timebase timebase);
static BERR_Code BAPE_MaiOutput_P_Enable(BAPE_OutputPort output);
static void      BAPE_MaiOutput_P_Disable(BAPE_OutputPort output);
static void      BAPE_MaiOutput_P_SetMclk_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio);
static void      BAPE_MaiOutput_P_SetMute(BAPE_OutputPort output, bool muted, bool sync);

/* Implementations for Legacy vs. New [7429] Hardware */
#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
/* New */
#define Impl IopOut

#include "bchp_aud_fmm_iop_out_mai_0.h"

#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_1_REG_START
#include "bchp_aud_fmm_iop_out_mai_1.h"
#endif

/* Define MAI version */
#if (defined BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL_DITHER_VALUE_MASK)
#define BAPE_MAI_IOPOUT_VERSION     1
#else
#define BAPE_MAI_IOPOUT_VERSION     0
#endif

#if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_NCO_0 || defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen0
#if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen0
    #define BAPE_MAI_MCLKCFG_NCO_CONSTRUCT_PARAM(idx) Mclk_gen##idx
#else
    #define BAPE_MAI_MCLKCFG_NCO_CONSTRUCT_PARAM(idx) NCO_##idx
#endif
#endif

static BERR_Code BAPE_MaiOutput_P_Open_IopOut(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_Close_IopOut(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_SetCbits_IopOut_isr(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_SetMclk_IopOut_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio);
static void      BAPE_MaiOutput_P_SetMute_IopOut(BAPE_OutputPort output, bool muted);
static BERR_Code BAPE_MaiOutput_P_Enable_IopOut(BAPE_OutputPort output);
static void      BAPE_MaiOutput_P_Disable_IopOut(BAPE_OutputPort output);
static BERR_Code BAPE_MaiOutput_P_OpenHw_IopOut(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_CloseHw_IopOut(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_SetCrossbar_IopOut(BAPE_MaiOutputHandle handle, BAPE_StereoMode stereoMode);
static BERR_Code BAPE_MaiOutput_P_SetBurstConfig_IopOut(BAPE_MaiOutputHandle handle);

#define  BAPE_MAI_0_START BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
#define  BAPE_MAI_0_END   BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_END

#if BAPE_CHIP_MAX_MAI_OUTPUTS > 1
    #define  BAPE_MAI_1_START BCHP_AUD_FMM_IOP_OUT_MAI_1_REG_START
    #define  BAPE_MAI_1_END   BCHP_AUD_FMM_IOP_OUT_MAI_1_REG_END
    #if ((BAPE_MAI_0_END-BAPE_MAI_0_START) != (BAPE_MAI_1_END-BAPE_MAI_1_START))
        #error "MAI interfaces present with different sizes"
    #endif
    #define  BAPE_MAI_START(idx) ((idx==1)?BAPE_MAI_1_START:BAPE_MAI_0_START)
#else
    #define  BAPE_MAI_START(idx) BAPE_MAI_0_START
#endif

#define BAPE_MAI_Reg_P_GetAddress(Register, idx) BAPE_Reg_P_GetAddress(Register, BAPE_MAI_0_START, BAPE_MAI_START, idx)
#define BAPE_MAI_Reg_P_GetArrayAddress(Register, i, idx) BAPE_MAI_Reg_P_GetAddress(BAPE_Reg_P_GetArrayAddress(Register, i), idx)

#elif defined BCHP_DVP_CFG_REG_START
/* OTT */
#define Impl Ott

#include "bchp_dvp_cfg.h"
static BERR_Code BAPE_MaiOutput_P_Open_Ott(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_Close_Ott(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_SetCbits_Ott_isr(BAPE_MaiOutputHandle handle);
static BERR_Code BAPE_MaiOutput_P_Enable_Ott(BAPE_OutputPort output);
static void      BAPE_MaiOutput_P_Disable_Ott(BAPE_OutputPort output);
static void      BAPE_MaiOutput_P_SetMclk_Ott_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio);
static BERR_Code BAPE_MaiOutput_P_OpenHw_Ott(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_CloseHw_Ott(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_SetCrossbar_Ott(BAPE_MaiOutputHandle handle, BAPE_StereoMode stereoMode);
static void      BAPE_MaiOutput_P_SetMute_Ott(BAPE_OutputPort output, bool muted);
static BERR_Code BAPE_MaiOutput_P_SetBurstConfig_Ott(BAPE_MaiOutputHandle handle);

#define  BAPE_MAI_0_START BCHP_DVP_CFG_MAI0_CTL
#define  BAPE_MAI_0_END   BCHP_DVP_CFG_MAI0_SMP

#if BAPE_CHIP_MAX_MAI_OUTPUTS > 1
    #define  BAPE_MAI_1_START BCHP_DVP_CFG_MAI1_CTL
    #define  BAPE_MAI_1_END   BCHP_DVP_CFG_MAI1_SMP
    #if ((BAPE_MAI_0_END-BAPE_MAI_0_START) != (BAPE_MAI_1_END-BAPE_MAI_1_START))
        #error "MAI interfaces present with different sizes"
    #endif
    #define  BAPE_MAI_START(idx) ((idx==1)?BAPE_MAI_1_START:BAPE_MAI_0_START)
#else
    #define  BAPE_MAI_START(idx) BAPE_MAI_0_START
#endif

#define BAPE_MAI_Reg_P_GetAddress(Register, idx) BAPE_Reg_P_GetAddress(Register, BAPE_MAI_0_START, BAPE_MAI_START, idx)
#define BAPE_MAI_Reg_P_GetArrayAddress(Register, i, idx) BAPE_MAI_Reg_P_GetAddress(BAPE_Reg_P_GetArrayAddress(Register, i), idx)

#else
/* Legacy */
#define Impl Legacy

static const uint16_t g_pauseburst[6] = {0xf872, 0x4e1f, 0x0003, 0x0020, 0x0000, 0x0000};
static const uint16_t g_nullburst[4] = {0xf872, 0x4e1f, 0xe000, 0x0000};

#define BAPE_P_MUTE_BUFFER_SIZE         384 /* Divisable by both 6 and 8 */

static BERR_Code BAPE_MaiOutput_P_Open_Legacy(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_Close_Legacy(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_SetCbits_Legacy_isr(BAPE_MaiOutputHandle handle);
static BERR_Code BAPE_MaiOutput_P_Enable_Legacy(BAPE_OutputPort output);
static void      BAPE_MaiOutput_P_Disable_Legacy(BAPE_OutputPort output);
static void      BAPE_MaiOutput_P_SetMclk_Legacy_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio);
static BERR_Code BAPE_MaiOutput_P_OpenHw_Legacy(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_CloseHw_Legacy(BAPE_MaiOutputHandle handle);
static void      BAPE_MaiOutput_P_SetCrossbar_Legacy(BAPE_MaiOutputHandle handle, BAPE_StereoMode stereoMode);
static void      BAPE_MaiOutput_P_SetMute_Legacy(BAPE_OutputPort output, bool muted);
static BAPE_MaiOutputDataPath BAPE_MaiOutput_P_FormatToDataPath(const BAPE_FMT_Descriptor *pFormat);
static BERR_Code BAPE_MaiOutput_P_SetBurstConfig_Legacy(BAPE_MaiOutputHandle handle);

#endif

/***************************************************************************
        Public APIs: From bape_output.h
***************************************************************************/

void BAPE_MaiOutput_GetDefaultSettings(
    BAPE_MaiOutputSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->stereoMode = BAPE_StereoMode_eLeftRight;
    pSettings->ditherEnabled = true;
    pSettings->underflowBurst = BAPE_SpdifBurstType_ePause;
    pSettings->burstPadding = 0;
    #if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_LOW_LATENCY_PASSTHROUGH_CFG
    #if 0
    pSettings->lowLatencyPassthroughEnabled = true;
    pSettings->hardwareMutingEnabled = true;
    #endif
    #endif
    pSettings->useRawChannelStatus = false;
    /* channel status is initialized to zero for all fields */
}

/**************************************************************************/

BERR_Code BAPE_MaiOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_MaiOutputSettings *pSettings,
    BAPE_MaiOutputHandle *pHandle             /* [out] */
    )
{
    BERR_Code errCode;
    BAPE_MaiOutputHandle handle;
    BAPE_MaiOutputSettings defaultSettings;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    BDBG_MSG(("%s: Opening MAI Output: %u", BSTD_FUNCTION, index));

    *pHandle = NULL;    /* Set up to return null handle in case of error. */

    if ( index >= BAPE_CHIP_MAX_MAI_OUTPUTS )
    {
        BDBG_ERR(("Request to open MAI %d but chip only has %u MAI outputs", index, BAPE_CHIP_MAX_MAI_OUTPUTS));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->maiOutputs[index] )
    {
        BDBG_ERR(("MAI output %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Allocate the device structure, then fill in all the fields. */
    handle = BKNI_Malloc(sizeof(BAPE_MaiOutput));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_MaiOutput));
    BDBG_OBJECT_SET(handle, BAPE_MaiOutput);
    handle->deviceHandle = deviceHandle;
    handle->index = index;
    BAPE_P_InitOutputPort(&handle->outputPort, BAPE_OutputPortType_eMaiOutput, index, handle);
#if defined BCHP_DVP_CFG_REG_START
    BAPE_FMT_P_EnableSource(&handle->outputPort.capabilities, BAPE_DataSource_eHostBuffer);
#else
    BAPE_FMT_P_EnableSource(&handle->outputPort.capabilities, BAPE_DataSource_eFci);
#endif
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_ePcmStereo);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_ePcm5_1);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_ePcm7_1);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_eIec61937);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_eIec61937x4);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_eIec61937x16);
    handle->outputPort.muteInMixer = true;
    handle->outputPort.setTimingParams_isr = BAPE_MaiOutput_P_SetTimingParams_isr;
    handle->outputPort.enable = BAPE_MaiOutput_P_Enable;
    handle->outputPort.disable = BAPE_MaiOutput_P_Disable;
    handle->outputPort.setMclk_isr = BAPE_MaiOutput_P_SetMclk_isr;
    handle->outputPort.setMute = BAPE_MaiOutput_P_SetMute;
    handle->outputPort.additionalGain = 0;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "MAI %u", index);
    handle->outputPort.pName = handle->name;
    handle->offset = 0;
    handle->sampleRate = 48000;

    BDBG_ASSERT(handle->offset == 0);

    BKNI_EnterCriticalSection();
    BAPE_MaiOutput_P_SetTimingParams_isr(&handle->outputPort, 48000, BAVC_Timebase_e0);
    BKNI_LeaveCriticalSection();

    /* Init to specified settings */
    if ( NULL == pSettings )
    {
        BAPE_MaiOutput_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    #if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_LOW_LATENCY_PASSTHROUGH_CFG
    handle->lowLatencyMode = pSettings->lowLatencyPassthroughEnabled;
    handle->honorHwMute = pSettings->hardwareMutingEnabled;
    #endif

    /* store open settings before open calls. */
    handle->settings = *pSettings;

#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
    errCode = BAPE_MaiOutput_P_Open_IopOut(handle);
#elif defined BCHP_DVP_CFG_REG_START
    errCode = BAPE_MaiOutput_P_Open_Ott(handle);
#else
    errCode = BAPE_MaiOutput_P_Open_Legacy(handle);
#endif
    if ( errCode )
    {
        BAPE_MaiOutput_Close(handle);
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_MaiOutput_P_OpenHw(handle);
    if ( errCode )
    {
        BAPE_MaiOutput_Close(handle);
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_MaiOutput_P_ApplySettings(handle, pSettings, true);   /* true => force update of HW */
    if ( errCode )
    {
        BAPE_MaiOutput_Close(handle);
        return BERR_TRACE(errCode);
    }

    *pHandle = handle;
    handle->deviceHandle->maiOutputs[index] = handle;
    return BERR_SUCCESS;
}

/**************************************************************************/

void BAPE_MaiOutput_Close(
    BAPE_MaiOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    if ( handle->index >= BAPE_CHIP_MAX_MAI_OUTPUTS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(handle->index < BAPE_CHIP_MAX_MAI_OUTPUTS);
        return;
    }

    /* Make sure we're not still connected to anything */
    if ( handle->outputPort.mixer )
    {
        BDBG_ERR(("Cannot close MAI output %p (%d), still connected to mixer %p", (void *)handle, handle->index, (void *)handle->outputPort.mixer));
        BDBG_ASSERT(NULL == handle->outputPort.mixer);
        return;
    }

    BAPE_MaiOutput_P_CloseHw(handle);

#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
    BAPE_MaiOutput_P_Close_IopOut(handle);
#elif defined BCHP_DVP_CFG_REG_START
    BAPE_MaiOutput_P_Close_Ott(handle);
#else
    BAPE_MaiOutput_P_Close_Legacy(handle);
#endif

    handle->deviceHandle->maiOutputs[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_MaiOutput);
    BKNI_Free(handle);
}

/**************************************************************************/

void BAPE_MaiOutput_GetSettings(
    BAPE_MaiOutputHandle handle,
    BAPE_MaiOutputSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

/**************************************************************************/

BERR_Code BAPE_MaiOutput_SetSettings(
    BAPE_MaiOutputHandle handle,
    const BAPE_MaiOutputSettings *pSettings
    )
{
    BERR_Code   errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(NULL != pSettings);

    errCode = BAPE_MaiOutput_P_ApplySettings(handle, pSettings, false); /* false => don't force (only update HW for changes) */

    return errCode;
}

/**************************************************************************/

void BAPE_MaiOutput_P_DeterminePauseBurstEnabled(
    BAPE_MaiOutputHandle handle,
    bool *compressed,
    bool *burstsEnabled)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    if ( handle->outputPort.mixer )
    {
        const BAPE_FMT_Descriptor *pBfd = BAPE_Mixer_P_GetOutputFormat_isrsafe(handle->outputPort.mixer);
        *compressed = BAPE_FMT_P_IsCompressed_isrsafe(pBfd);
    }
    else {
        *compressed = false;
    }
    *burstsEnabled = handle->settings.underflowBurst != BAPE_SpdifBurstType_eNone;
    return;
}

/**************************************************************************/

void BAPE_MaiOutput_GetOutputPort(
    BAPE_MaiOutputHandle handle,
    BAPE_OutputPort *pOutputPort        /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(NULL != pOutputPort);
    *pOutputPort = &handle->outputPort;
}

/**************************************************************************/

void BAPE_MaiOutput_GetInterruptHandlers(
    BAPE_MaiOutputHandle handle,
    BAPE_MaiOutputInterruptHandlers *pInterrupts    /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(NULL != pInterrupts);
    BKNI_EnterCriticalSection();
    *pInterrupts = handle->interrupts;
    BKNI_LeaveCriticalSection();
}

/**************************************************************************/

BERR_Code BAPE_MaiOutput_SetInterruptHandlers(
    BAPE_MaiOutputHandle handle,
    const BAPE_MaiOutputInterruptHandlers *pInterrupts
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(NULL != pInterrupts);
    BKNI_EnterCriticalSection();
    handle->interrupts = *pInterrupts;
    BKNI_LeaveCriticalSection();
    return BERR_SUCCESS;
}

/***************************************************************************
        BAPE Internal APIs: From bape_fmm_priv.h
***************************************************************************/

BERR_Code BAPE_MaiOutput_P_PrepareForStandby(
    BAPE_Handle bapeHandle
    )
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    maiOutputIndex;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each opened MaiOutput, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( maiOutputIndex=0 ; maiOutputIndex<BAPE_CHIP_MAX_MAI_OUTPUTS ; maiOutputIndex++ )
    {
        if ( bapeHandle->maiOutputs[maiOutputIndex] )       /* If this MaiOutput is open... */
        {
            BAPE_MaiOutputHandle hMaiOutput = bapeHandle->maiOutputs[maiOutputIndex];

            /* Put the HW into the generic closed state. */
            BAPE_MaiOutput_P_CloseHw(hMaiOutput);
        }
    }
    return errCode;
}

BERR_Code BAPE_MaiOutput_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    maiOutputIndex;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each opened MaiOutput, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( maiOutputIndex=0 ; maiOutputIndex<BAPE_CHIP_MAX_MAI_OUTPUTS ; maiOutputIndex++ )
    {
        if ( bapeHandle->maiOutputs[maiOutputIndex] )       /* If this MaiOutput is open... */
        {
            BAPE_MaiOutputHandle hMaiOutput = bapeHandle->maiOutputs[maiOutputIndex];

            /* Put the HW into the generic open state. */
            errCode = BAPE_MaiOutput_P_OpenHw(hMaiOutput);
            if ( errCode ) return BERR_TRACE(errCode);

            /* Now apply changes for the settings struct. */
            errCode = BAPE_MaiOutput_P_ApplySettings(hMaiOutput, &hMaiOutput->settings, true);   /* true => force update of HW */
            if ( errCode ) return BERR_TRACE(errCode);

            /* Now restore the dynamic stuff from the values saved in the device struct. */
            BKNI_EnterCriticalSection();

            BAPE_MaiOutput_P_SetTimingParams_isr(&hMaiOutput->outputPort,
                                                 hMaiOutput->sampleRate,
                                                 0);    /* timebase is unused, 0 is dummy value */

            BAPE_MaiOutput_P_SetMclk_isr(&hMaiOutput->outputPort,
                                         hMaiOutput->mclkInfo.mclkSource,
                                         hMaiOutput->mclkInfo.pllChannel,
                                         hMaiOutput->mclkInfo.mclkFreqToFsRatio );

            BKNI_LeaveCriticalSection();
        }
    }
    return errCode;
}

/***************************************************************************
        Private callbacks: Protyped above
***************************************************************************/

static void BAPE_MaiOutput_P_SetTimingParams_isr(BAPE_OutputPort output, unsigned sampleRate, BAVC_Timebase timebase)
{
    BAPE_MaiOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BSTD_UNUSED(timebase);  /* MAI doesn't care */

    handle->sampleRate = sampleRate;

    BAPE_MaiOutput_P_SetCbits_isr(handle);

    if ( handle->interrupts.sampleRate.pCallback_isr )
    {
        BDBG_MSG(("Sending sample rate callback - new rate %u", sampleRate));
        handle->interrupts.sampleRate.pCallback_isr(handle->interrupts.sampleRate.pParam1, handle->interrupts.sampleRate.param2, sampleRate);
    }
}

/**************************************************************************/
static BERR_Code BAPE_MaiOutput_P_Enable(BAPE_OutputPort output)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
    return BAPE_MaiOutput_P_Enable_IopOut(output);
#elif defined BCHP_DVP_CFG_REG_START
    return BAPE_MaiOutput_P_Enable_Ott(output);
#else
    return BAPE_MaiOutput_P_Enable_Legacy(output);
#endif
}

/**************************************************************************/

static void BAPE_MaiOutput_P_Disable(BAPE_OutputPort output)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
    BAPE_MaiOutput_P_Disable_IopOut(output);
#elif defined BCHP_DVP_CFG_REG_START
    BAPE_MaiOutput_P_Disable_Ott(output);
#else
    BAPE_MaiOutput_P_Disable_Legacy(output);
#endif
}

/**************************************************************************/

static void BAPE_MaiOutput_P_SetMclk_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
    BAPE_MaiOutput_P_SetMclk_IopOut_isr(output, mclkSource, pllChannel, mclkFreqToFsRatio);
#elif defined BCHP_DVP_CFG_REG_START
    BAPE_MaiOutput_P_SetMclk_Ott_isr(output, mclkSource, pllChannel, mclkFreqToFsRatio);
#else
    BAPE_MaiOutput_P_SetMclk_Legacy_isr(output, mclkSource, pllChannel, mclkFreqToFsRatio);
#endif
}

/**************************************************************************/

static void      BAPE_MaiOutput_P_SetMute(BAPE_OutputPort output, bool muted, bool sync)
{
    BSTD_UNUSED(sync);
#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
    BAPE_MaiOutput_P_SetMute_IopOut(output, muted);
#elif defined BCHP_DVP_CFG_REG_START
    BAPE_MaiOutput_P_SetMute_Ott(output, muted);
#else
    BAPE_MaiOutput_P_SetMute_Legacy(output, muted);
#endif
}

/***************************************************************************
        Private functions: Protyped above
***************************************************************************/

static void BAPE_MaiOutput_P_SetCbits_isr(BAPE_MaiOutputHandle handle)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
    BAPE_MaiOutput_P_SetCbits_IopOut_isr(handle);
#elif defined BCHP_DVP_CFG_REG_START
    BAPE_MaiOutput_P_SetCbits_Ott_isr(handle);
#else
    BAPE_MaiOutput_P_SetCbits_Legacy_isr(handle);
#endif
}

/**************************************************************************/

static BERR_Code BAPE_MaiOutput_P_OpenHw(BAPE_MaiOutputHandle handle)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
    return BAPE_MaiOutput_P_OpenHw_IopOut(handle);
#elif defined BCHP_DVP_CFG_REG_START
    return BAPE_MaiOutput_P_OpenHw_Ott(handle);
#else
    return BAPE_MaiOutput_P_OpenHw_Legacy(handle);
#endif
}

/**************************************************************************/

static void BAPE_MaiOutput_P_CloseHw(BAPE_MaiOutputHandle handle)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
    BAPE_MaiOutput_P_CloseHw_IopOut(handle);
#elif defined BCHP_DVP_CFG_REG_START
    BAPE_MaiOutput_P_CloseHw_Ott(handle);
#else
    BAPE_MaiOutput_P_CloseHw_Legacy(handle);
#endif
}

static BERR_Code BAPE_MaiOutput_P_SetBurstConfig(BAPE_MaiOutputHandle handle)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
    return BAPE_MaiOutput_P_SetBurstConfig_IopOut(handle);
#elif defined BCHP_DVP_CFG_REG_START
    return BAPE_MaiOutput_P_SetBurstConfig_Ott(handle);
#else
    return BAPE_MaiOutput_P_SetBurstConfig_Legacy(handle);
#endif
}

/**************************************************************************/

static BERR_Code BAPE_MaiOutput_P_SetCrossbar(BAPE_MaiOutputHandle handle, BAPE_StereoMode stereoMode)
{
#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
    BAPE_MaiOutput_P_SetCrossbar_IopOut(handle, stereoMode);
#elif defined BCHP_DVP_CFG_REG_START
    BAPE_MaiOutput_P_SetCrossbar_Ott(handle, stereoMode);
#else
    BAPE_MaiOutput_P_SetCrossbar_Legacy(handle, stereoMode);
#endif
    return BERR_SUCCESS;
}

/**************************************************************************/

static BERR_Code BAPE_MaiOutput_P_ApplySettings(
    BAPE_MaiOutputHandle handle,
    const BAPE_MaiOutputSettings *pSettings,
    bool force
    )
{
    bool burstTypeChanged;
    bool pcm = true;
    BSTD_UNUSED(force); /* We don't need this because all settings are always written to HW. */

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(NULL != pSettings);

    burstTypeChanged = handle->settings.underflowBurst != pSettings->underflowBurst;

    handle->settings = *pSettings;

    /* Remaining fields are handled by MS regs.  Must modify those in critical section. */
    BKNI_EnterCriticalSection();
    BAPE_MaiOutput_P_SetCbits_isr(handle);
    BKNI_LeaveCriticalSection();


    if ( handle->outputPort.mixer )
    {
        const BAPE_FMT_Descriptor     *pBfd = BAPE_Mixer_P_GetOutputFormat_isrsafe(handle->outputPort.mixer);
        pcm = BAPE_FMT_P_IsLinearPcm_isrsafe(pBfd);
    }

    if (pcm)
    {
        BAPE_MaiOutput_P_SetCrossbar(handle, handle->settings.stereoMode);
    }
    else
    {
        BAPE_MaiOutput_P_SetCrossbar(handle, BAPE_StereoMode_eLeftRight);
    }

    /* check/modify burst configuration */
    if ( burstTypeChanged )
    {
        BAPE_MaiOutput_P_SetBurstConfig(handle);
    }

    /* apply loudness equivalance levels*/
    if (handle->outputPort.mixer)
    {
        BAPE_MaiOutput_P_SetLoudnessEquivlanceVolume(&handle->outputPort);
    }
    return BERR_SUCCESS;
}

static uint32_t BAPE_MaiOutput_P_SampleRateToMaiFormat_isrsafe(unsigned sampleRate)
{
    switch ( sampleRate )
    {
    case 32000:    /* 32K Sample rate */
        return 7;
    case 44100:    /* 44.1K Sample rate */
        return 8;
    case 48000:      /* 48K Sample rate */
        return 9;
    case 96000:      /* 96K Sample rate */
        return 12;
    case 128000:     /* 128K Sample rate */
        return 13;
    case 176400:   /* 176.4K Sample rate */
        return 14;
    case 192000:     /* 192K Sample rate */
        return 15;
    default:
        return 0;   /* Not Indicated */
    }
}

void BAPE_MaiOutput_P_SetLoudnessEquivlanceVolume(BAPE_OutputPort output)
{
    BAPE_OutputVolume volume;
    const BAPE_FMT_Descriptor *pFormat;
    BAPE_MaiOutputHandle handle;

    handle = output->pHandle;
    pFormat = BAPE_Mixer_P_GetOutputFormat_isrsafe(output->mixer);
    BAPE_GetOutputVolume(output, &volume);

    if (handle->settings.loudnessType == BAPE_OutputLoudnessType_Passive &&
        pFormat->type != BAPE_DataType_ePcm5_1 &&
        pFormat->type != BAPE_DataType_ePcm7_1)
    {
        switch (handle->deviceHandle->settings.loudnessSettings.loudnessMode)
        {
        default:
        case BAPE_LoudnessEquivalenceMode_eNone:
            handle->outputPort.additionalGain = 0;
            break;
        case BAPE_LoudnessEquivalenceMode_eAtscA85:
            handle->outputPort.additionalGain = -7; /* 24dB->31dB*/
            break;
        case BAPE_LoudnessEquivalenceMode_eEbuR128:
            handle->outputPort.additionalGain = -8; /* 23dB->31dB*/
            break;
        }
    }
    else
    {
        handle->outputPort.additionalGain = 0;
    }
    BDBG_MODULE_MSG(bape_loudness,("HDMI is configured as %s and will output at %ddB for %s mode",
         ((pFormat->type == BAPE_DataType_ePcm5_1 || pFormat->type == BAPE_DataType_ePcm7_1) ? "MULITICHANNEL":
          (BAPE_FMT_P_IsCompressed_isrsafe(pFormat)) ? "COMPRESSED" :
          (handle->settings.loudnessType == BAPE_OutputLoudnessType_Passive) ? "PASSIVE" : "ACTIVE"),
         handle->outputPort.additionalGain,
         (handle->deviceHandle->settings.loudnessSettings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? "ATSC" :
          handle->deviceHandle->settings.loudnessSettings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? "EBU" : "DISABLED")));
    BAPE_SetOutputVolume(&handle->outputPort, &volume);
}

#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
/**************************************************************************
7429-style RDB
**************************************************************************/
static BERR_Code BAPE_MaiOutput_P_Open_IopOut(BAPE_MaiOutputHandle handle)
{
    BKNI_EnterCriticalSection();
#if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen0 || defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_NCO_0
    BAPE_MaiOutput_P_SetMclk_isr(&handle->outputPort, BAPE_MclkSource_eNco0, 0, 256);
#else
    BAPE_MaiOutput_P_SetMclk_isr(&handle->outputPort, BAPE_MclkSource_ePll0, 0, 256);
#endif
    BKNI_LeaveCriticalSection();

    BAPE_MaiOutput_P_SetBurstConfig_IopOut(handle);

    return BERR_SUCCESS;
}

static void BAPE_MaiOutput_P_Close_IopOut(BAPE_MaiOutputHandle handle)
{
    BSTD_UNUSED(handle);
    return;
}

static void BAPE_MaiOutput_P_SetCbits_IopOut_isr(BAPE_MaiOutputHandle handle)
{
    BAPE_Reg_P_FieldList regFieldList;
    uint32_t regAddr, regVal;
    unsigned validity = 0;
    unsigned dither = 0;
    unsigned hbr = 0;
    BAPE_DataType dataType = BAPE_DataType_ePcmStereo;
    bool compressed = false;
    bool compressedAsPcm = false;

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    if ( handle->outputPort.mixer )
    {
        const BAPE_FMT_Descriptor     *pBfd = BAPE_Mixer_P_GetOutputFormat_isrsafe(handle->outputPort.mixer);

        dataType = pBfd->type;
        hbr = (unsigned)BAPE_FMT_P_IsHBR_isrsafe(pBfd);
        compressed = BAPE_FMT_P_IsCompressed_isrsafe(pBfd);
        compressedAsPcm = BAPE_FMT_P_IsDtsCdCompressed_isrsafe(pBfd);
    }

    BDBG_MSG(("Set MAI CBITS SR %u compressed %u, compressedAsPcm %u", handle->sampleRate, compressed, compressedAsPcm));

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

    /* Program MAI format correctly */
    if ( handle->outputPort.mixer )
    {
        regAddr = BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_WORD, handle->index);
        BAPE_Reg_P_InitFieldList_isr(handle->deviceHandle, &regFieldList);

        if ( compressed && !compressedAsPcm )
        {
            if ( hbr )
            {
                BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_WORD, AUDIO_FORMAT, HBR_compressed_8_channel);
            }
            else
            {
                BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_WORD, AUDIO_FORMAT, Compressed_audio_2_channel);
            }
        }
        else
        {
            switch ( dataType )
            {
            case BAPE_DataType_ePcm5_1:
                BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_WORD, AUDIO_FORMAT, SPDIF_linearPCM_6_channel);
                break;
            case BAPE_DataType_ePcm7_1:
                BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_WORD, AUDIO_FORMAT, SPDIF_linearPCM_8_channel);
                break;
            default:
            case BAPE_DataType_ePcmStereo:
                BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_WORD, AUDIO_FORMAT, SPDIF_linearPCM_stereo);
                break;
            }
            BAPE_Reg_P_AddToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_WORD, SAMPLE_WIDTH, 0); /* 32 bits per sample */
        }

        if ( compressed || compressedAsPcm )
        {
            BAPE_Reg_P_AddToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_WORD, SAMPLE_WIDTH, 16);
        }

        if ( hbr )
        {
            BAPE_Reg_P_AddToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_WORD,
                                          SAMPLE_RATE, BAPE_MaiOutput_P_SampleRateToMaiFormat_isrsafe(192000));
        }
        else
        {
            BAPE_Reg_P_AddToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_WORD,
                                          SAMPLE_RATE, BAPE_MaiOutput_P_SampleRateToMaiFormat_isrsafe(handle->sampleRate));
        }

        BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, regAddr);
    }

    /* Set Cbit transmission type */
    if ( compressed && !compressedAsPcm )
    {
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, COMP_OR_LINEAR, Compressed);
    }
    else
    {
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, COMP_OR_LINEAR, Linear);
    }

    /* hold cbits before changing dither or validity */
    BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, HOLD_CSTAT, Hold);

    /* Tell the HW to hold the current CBITS and set dither correctly */
    regAddr = BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, handle->index);
    BAPE_Reg_P_InitFieldList_isr(handle->deviceHandle, &regFieldList);
    BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, HOLD_CSTAT, Hold);
    BAPE_Reg_P_AddToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, DITHER_ENA, dither);
    BAPE_Reg_P_AddToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, VALIDITY, validity);
    BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, regAddr);

    /* Program channel status */
    if ( handle->settings.useRawChannelStatus )
    {
        regVal =
            (uint32_t)handle->settings.rawChannelStatus[0] |
            (((uint32_t)handle->settings.rawChannelStatus[1])<<8) |
            (((uint32_t)handle->settings.rawChannelStatus[2])<<16) |
            (((uint32_t)handle->settings.rawChannelStatus[3])<<24);
        BAPE_Reg_P_Write_isr(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CHANSTAT_0, handle->index), regVal);
        regVal = (uint32_t)handle->settings.rawChannelStatus[4];
        BAPE_Reg_P_Write_isr(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CHANSTAT_1, handle->index), regVal);
    }
    else
    {
        BAPE_Spdif_P_ChannelStatusBits cbits;

        BAPE_P_MapSpdifChannelStatusToBits_isr(&handle->outputPort, &handle->settings.channelStatus, &cbits);
        BAPE_Reg_P_Write_isr(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CHANSTAT_0, handle->index), cbits.bits[0]);
        BAPE_Reg_P_Write_isr(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CHANSTAT_1, handle->index), cbits.bits[1]);
    }

    /* Reload the new channel status in HW */
    BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, HOLD_CSTAT, Update);

    return;
}

static void BAPE_MaiOutput_P_SetMclk_IopOut_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
    BAPE_Reg_P_FieldList regFieldList;

    BAPE_MaiOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(handle->offset == 0);

    /* Save the settings in case we need to re-apply them later. */
    handle->mclkInfo.mclkSource         = mclkSource;
    handle->mclkInfo.pllChannel         = pllChannel;
    handle->mclkInfo.mclkFreqToFsRatio  = mclkFreqToFsRatio;

    BAPE_Reg_P_InitFieldList_isr(handle->deviceHandle, &regFieldList);

    switch ( mclkSource )
    {
#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_PLL0_ch1
    case BAPE_MclkSource_ePll0:
        switch ( pllChannel )
        {
        case 0: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, PLL0_ch1); break;
        case 1: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, PLL0_ch2); break;
        case 2: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, PLL0_ch3); break;
        default: (void) BERR_TRACE(BERR_NOT_SUPPORTED); break;
        }
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_PLL1_ch1
    case BAPE_MclkSource_ePll1:
        switch ( pllChannel )
        {
        case 0: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, PLL1_ch1); break;
        case 1: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, PLL1_ch2); break;
        case 2: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, PLL1_ch3); break;
        default: (void) BERR_TRACE(BERR_NOT_SUPPORTED); break;
        }
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_PLL2_ch1
    case BAPE_MclkSource_ePll2:
        switch ( pllChannel )
        {
        case 0: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, PLL2_ch1); break;
        case 1: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, PLL2_ch2); break;
        case 2: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, PLL2_ch3); break;
        default: (void) BERR_TRACE(BERR_NOT_SUPPORTED); break;
        }
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen0 || defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_NCO_0
    case BAPE_MclkSource_eNco0:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, BAPE_MAI_MCLKCFG_NCO_CONSTRUCT_PARAM(0));
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen1 || defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_NCO_1
    case BAPE_MclkSource_eNco1:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, BAPE_MAI_MCLKCFG_NCO_CONSTRUCT_PARAM(1));
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen2 || defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_NCO_2
    case BAPE_MclkSource_eNco2:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, BAPE_MAI_MCLKCFG_NCO_CONSTRUCT_PARAM(2));
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen3 || defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_NCO_3
    case BAPE_MclkSource_eNco3:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, BAPE_MAI_MCLKCFG_NCO_CONSTRUCT_PARAM(3));
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen4 || defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_NCO_4
    case BAPE_MclkSource_eNco4:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, BAPE_MAI_MCLKCFG_NCO_CONSTRUCT_PARAM(4));
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen5 || defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_NCO_5
    case BAPE_MclkSource_eNco5:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, BAPE_MAI_MCLKCFG_NCO_CONSTRUCT_PARAM(5));
        break;
#endif
#if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen6 || defined BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0_PLLCLKSEL_NCO_6
    case BAPE_MclkSource_eNco6:
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, PLLCLKSEL, BAPE_MAI_MCLKCFG_NCO_CONSTRUCT_PARAM(6));
        break;
#endif
    default:
        BDBG_ERR(("Unsupported clock source %u for MAI %u", mclkSource, handle->index));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    switch ( mclkFreqToFsRatio )
    {
    case 128: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, MCLK_RATE, MCLK_128fs_SCLK_64fs); break;
    case 256: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, MCLK_RATE, MCLK_256fs_SCLK_64fs); break;
    case 384: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, MCLK_RATE, MCLK_384fs_SCLK_64fs); break;
    case 512: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, MCLK_RATE, MCLK_512fs_SCLK_64fs); break;
    default:
        BDBG_ERR(("Unsupported MCLK Rate of %uFs", mclkFreqToFsRatio));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        break;
    }
    BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_MCLK_CFG_0, handle->index));
}

static void BAPE_MaiOutput_P_SetMute_IopOut(BAPE_OutputPort output, bool muted)
{
    BAPE_MaiOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    /* Must manipulate CSTAT registers in critical section */
    BKNI_EnterCriticalSection();

    if ( muted )
    {
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, OVERWRITE_DATA, Enable);
    }
    else
    {
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, OVERWRITE_DATA, Disable);
    }

    BKNI_LeaveCriticalSection();


    handle->muted = muted;
    BDBG_MSG(("MAI output %d (IopOut) mute %u", handle->index, muted));
}

/**************************************************************************/

static BERR_Code BAPE_MaiOutput_P_Enable_IopOut(BAPE_OutputPort output)
{
    BAPE_Reg_P_FieldList regFieldList;
    BAPE_MaiOutputHandle handle;
    const BAPE_FMT_Descriptor *pFormat;
    uint32_t regAddr;
    unsigned numChannelPairs = 0;
    unsigned i;
    bool hbr;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(false == handle->enabled);
    BDBG_ASSERT(NULL != output->mixer);

    BDBG_MSG(("Enabling %s", handle->name));

    /* apply loudness equivalance levels*/
    BAPE_MaiOutput_P_SetLoudnessEquivlanceVolume(output);
    BAPE_MaiOutput_SetSettings(handle, &handle->settings);

    pFormat = BAPE_Mixer_P_GetOutputFormat_isrsafe(output->mixer);
    BDBG_ASSERT(NULL != pFormat);
    numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(pFormat);
    if ( numChannelPairs > 4 )
    {
        BDBG_ASSERT(numChannelPairs <= 4);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    hbr = (pFormat->type == BAPE_DataType_eIec61937x16) ? true : false;

    BDBG_MODULE_MSG(bape_fci, ("Linking MAI OUTPUT FCI Source"));
    for ( i = 0; i < numChannelPairs; i++ )
    {
        regAddr = BAPE_MAI_Reg_P_GetArrayAddress(AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, i, handle->index);
        BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
        #if (BAPE_MAI_IOPOUT_VERSION == 1)
         /* Grouping is required for newer 28nm chips, however there is still a bug which causes the
            MAI path not to start properly IF we didn't wait for it to drain on the previous stop sequence. See workaround in _Disable*/
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, GROUP_ID, 0);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, CHANNEL_GROUPING, 0xf>>(4-numChannelPairs));
        #else
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, GROUP_ID, i);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, CHANNEL_GROUPING, 1);
        #endif
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, STREAM_BIT_RESOLUTION, Res_24_Bit);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, FCI_ID, output->sourceMixerFci.ids[i]);
        BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);
        BDBG_MSG(("Mixer %d FCI %d -> MAI pair %d", output->mixer->index, output->sourceMixerFci.ids[i], i));
        BDBG_MODULE_MSG(bape_fci, ("  fci %x -> MAI %d [%d]", output->sourceMixerFci.ids[i], handle->index, i));
    }
    for ( ; i <= BCHP_AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i_ARRAY_END; i++ )
    {

        regAddr = BAPE_MAI_Reg_P_GetArrayAddress(AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, i, handle->index);
        BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, GROUP_ID, i);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, CHANNEL_GROUPING, 1);
        BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);
    }

    /* Set MAI source */
    regAddr = BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_CFG, handle->index);
    if ( hbr )
    {
        BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_CFG, SPDIF_MODE, SPDIF_Format);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_CFG, MAI_PAYLOAD_SEL, MAI_HBR);
    }
    else
    {
        BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_CFG, SPDIF_MODE, SPDIF_Format);
        if ( numChannelPairs > 1 )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_CFG, MAI_PAYLOAD_SEL, MAI_Multi);
        }
        else
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_CFG, MAI_PAYLOAD_SEL, Stereo);
        }
    }
    BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);

    /* Update CBITS */
    BKNI_EnterCriticalSection();
    BAPE_MaiOutput_P_SetCbits_isr(handle);
    BKNI_LeaveCriticalSection();

    /* Reset the interface to avoid grouping lockup */
    for ( i = 0; i < numChannelPairs; i++ )
    {
        regAddr = BAPE_MAI_Reg_P_GetArrayAddress(AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, i, handle->index);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, INIT_SM, 1);
    }
    for ( i = 0; i < numChannelPairs; i++ )
    {
        regAddr = BAPE_MAI_Reg_P_GetArrayAddress(AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, i, handle->index);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, INIT_SM, 0);
    }

    /* Enable the interface. Disable channels pairs not in use */
    for (i = numChannelPairs; i < BAPE_MAI_OUTPUT_MAX_CHANNEL_PAIRS; i++ )
    {
        regAddr = BAPE_MAI_Reg_P_GetArrayAddress(AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, i, handle->index);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, ENA, 0);
    }
    for ( i = 0; i < numChannelPairs; i++ )
    {
        regAddr = BAPE_MAI_Reg_P_GetArrayAddress(AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, i, handle->index);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, ENA, 1);
    }

    /* Enable the MAI bus - This must be done last to make older Onkyo receivers happy switching from DTS->PCM */
    /* TBD: How is this handled on 7429? */

    handle->enabled = true;
    return BERR_SUCCESS;
}

static void BAPE_MaiOutput_P_Disable_IopOut(BAPE_OutputPort output)
{
    BAPE_MaiOutputHandle handle;
    /* BAPE_Reg_P_FieldList regFieldList; */
    uint32_t regAddr;
    const BAPE_FMT_Descriptor *pFormat;
    unsigned i, numChannelPairs = 0;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(NULL != output->mixer);

    BDBG_MSG(("Disabling %s", handle->name));
    pFormat = BAPE_Mixer_P_GetOutputFormat_isrsafe(output->mixer);
    BDBG_ASSERT(NULL != pFormat);
    numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(pFormat);
    BDBG_ASSERT(numChannelPairs <= 4);

    /* If we are PCM (Stereo only) we need to disable the stream config so that we hold the channel status */
    /* If we were to disable this for compressed we would loose pause bursts */
    if (pFormat->type == BAPE_DataType_ePcmMono ||
        pFormat->type == BAPE_DataType_ePcmStereo)
    {
        regAddr = BAPE_MAI_Reg_P_GetArrayAddress(AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, 0, handle->index);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, ENA, 0);
    }

    /* Clear out FCI ID's */
    for ( i = 0; i < numChannelPairs; i++ )
    {
        regAddr = BAPE_MAI_Reg_P_GetArrayAddress(AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, i, handle->index);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, FCI_ID, BAPE_FCI_ID_INVALID);
    }

    handle->enabled = false;
}

static BERR_Code BAPE_MaiOutput_P_OpenHw_IopOut(BAPE_MaiOutputHandle handle)
{
    BERR_Code       errCode = BERR_SUCCESS;
    BAPE_Handle     deviceHandle;

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

#if BAPE_CHIP_MAX_MAI_OUTPUTS == 1
    BDBG_ASSERT(handle->index == 0);
#endif

    /* Always source MAI timing from I2S Multi or MAI Multi.  */
    BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_CFG, handle->index), AUD_FMM_IOP_OUT_MAI_0_MAI_FORMAT_CFG, MAI_PAYLOAD_SEL, MAI_Multi);

#if !(defined BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL_DITHER_VALUE_MASK)
    /* if we don't have a DITHER_VALUE register, then the default dither value is '1'.
       make sure the default width is 16 bit in that case. */
    BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, DITHER_WIDTH, RES_16);
#endif

#ifdef BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL_WAIT_PCM_TO_COMP_MASK
    BAPE_Reg_P_UpdateField(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, WAIT_PCM_TO_COMP, 2);
#endif

    BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, INSERT_ON_UFLOW , Insert);
    BAPE_Reg_P_UpdateField(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_RAMP_BURST, handle->index), AUD_FMM_IOP_OUT_MAI_0_SPDIF_RAMP_BURST, STEPSIZE, 0);

    #if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_LOW_LATENCY_PASSTHROUGH_CFG
    if ( handle->lowLatencyMode )
    {
        BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, handle->index), AUD_FMM_IOP_OUT_MAI_0_SPDIF_CTRL, SPDIF_BYPASS, Disable);
        BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_LOW_LATENCY_PASSTHROUGH_CFG, handle->index), AUD_FMM_IOP_OUT_MAI_0_LOW_LATENCY_PASSTHROUGH_CFG, LOW_LATENCY_PASSTHROUGH_ENABLE, Enable);
    }
    if ( handle->honorHwMute )
    {
        BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_HONOR_HW_MUTE, handle->index), AUD_FMM_IOP_OUT_MAI_0_HONOR_HW_MUTE, HONOR_HW_MUTE, Enable);
    }
    #endif

    /* Default cbit Sample Rate to 48k */
    BAPE_Reg_P_Write_isr(handle->deviceHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_CHANSTAT_0, handle->index), 0x02000000);

    return errCode;
}

static void BAPE_MaiOutput_P_CloseHw_IopOut(BAPE_MaiOutputHandle handle)
{
    uint32_t regAddr;
    unsigned i;
    /* Disable the MAI streams */
    for ( i = 0; i < 4; i++ )
    {
        regAddr = BAPE_MAI_Reg_P_GetArrayAddress(AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, i, handle->index);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_OUT_MAI_0_STREAM_CFG_i, ENA, 0);
    }
    return;
}

static BERR_Code BAPE_MaiOutput_P_SetBurstConfig_IopOut(BAPE_MaiOutputHandle handle)
{
    BAPE_Reg_P_FieldList regFieldList;
    uint32_t regAddr;

    /* Set Burst type */
    regAddr = BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_RAMP_BURST, handle->index);
    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
    if ( handle->settings.underflowBurst == BAPE_SpdifBurstType_ePause )
    {
        #if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_RAMP_BURST_BURST_TYPE_MASK
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_SPDIF_RAMP_BURST, BURST_TYPE, Pause);
        #else
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_SPDIF_RAMP_BURST, TYPE, Pause);
        #endif
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_SPDIF_RAMP_BURST, REP_PERIOD, PER_32);
    }
    else if ( handle->settings.underflowBurst == BAPE_SpdifBurstType_eNull )
    {
        #if defined BCHP_AUD_FMM_IOP_OUT_MAI_0_SPDIF_RAMP_BURST_BURST_TYPE_MASK
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_SPDIF_RAMP_BURST, BURST_TYPE, Null);
        #else
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_SPDIF_RAMP_BURST, TYPE, Null);
        #endif
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_SPDIF_RAMP_BURST, REP_PERIOD, PER_32);
    }
    else
    {
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_SPDIF_RAMP_BURST, REP_PERIOD, None);
    }
    BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);

    return BERR_SUCCESS;
}


static void      BAPE_MaiOutput_P_SetCrossbar_IopOut(BAPE_MaiOutputHandle handle, BAPE_StereoMode stereoMode)
{
    #if 0 /* IOP platforms copy data post channel status resulting in issues with some devices */
    BAPE_Reg_P_FieldList regFieldList;

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    /* Setup the MAI output crossbar to properly assign the channel order for HDMI. */
    /* HDMI expects L R LFE C Ls Rs Lr Rr, we output L R Ls Rs C LFE Lr Rr */
    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
    switch ( stereoMode )
    {
    default:
    case BAPE_StereoMode_eLeftRight:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT0_L, In0_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT0_R, In0_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT1_L, In1_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT1_R, In1_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT2_L, In2_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT2_R, In2_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT3_L, In3_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT3_R, In3_r);
        break;
    case BAPE_StereoMode_eLeftLeft:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT0_L, In0_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT0_R, In0_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT1_L, In1_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT1_R, In1_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT2_L, In2_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT2_R, In2_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT3_L, In3_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT3_R, In3_l);
        break;
    case BAPE_StereoMode_eRightRight:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT0_L, In0_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT0_R, In0_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT1_L, In1_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT1_R, In1_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT2_L, In2_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT2_R, In2_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT3_L, In3_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT3_R, In3_r);
        break;
    case BAPE_StereoMode_eRightLeft:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT0_L, In0_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT0_R, In0_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT1_L, In1_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT1_R, In1_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT2_L, In2_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT2_R, In2_l);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT3_L, In3_r);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, OUT3_R, In3_l);
        break;
    }
    BAPE_Reg_P_ApplyFieldList(&regFieldList, BAPE_MAI_Reg_P_GetAddress(BCHP_AUD_FMM_IOP_OUT_MAI_0_MAI_CROSSBAR, handle->index));
    #else
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    if ( handle->outputPort.mixer )
    {
        BAPE_Mixer_P_ApplyStereoMode(handle->outputPort.mixer, stereoMode);
    }
    #endif

}

#elif defined BCHP_DVP_CFG_REG_START
/**************************************************************************
OTT style MAI
**************************************************************************/

static BERR_Code
BAPE_MaiOutput_P_Open_Ott(BAPE_MaiOutputHandle handle)
{
    BSTD_UNUSED(handle);
    handle->dma = BAPE_DmaOutput_P_Open(handle->deviceHandle);
    return BERR_SUCCESS;
}

static void
BAPE_MaiOutput_P_Close_Ott(BAPE_MaiOutputHandle handle)
{
    BSTD_UNUSED(handle);
    if (handle->dma) {
        BAPE_DmaOutput_P_Close(handle->dma);
        handle->dma = NULL;
    }
}

static bool
BAPE_MaiOutput_P_SampleRateToPeriod_isrsafe(unsigned sampleRate, unsigned *_n, unsigned *_m)
{
    uint64_t srcf = 108000000;
    unsigned m;

    for (m = 0; m < 256; m++, srcf += 108000000) {
        if (srcf % sampleRate == 0) {
            break;
        }
    }
    if (m == 256) {
        return false;
    }
    *_n = srcf / sampleRate;
    *_m = m;
    return true;
}

static void
BAPE_MaiOutput_P_SetCbits_Ott_isr(BAPE_MaiOutputHandle handle)
{
    BAPE_Reg_P_FieldList regFieldList;
    uint32_t regAddr;
    unsigned hbr = 0;
    BAPE_DataType dataType = BAPE_DataType_ePcmStereo;
    bool compressed = false;
    bool compressedAsPcm = false;
    unsigned n, m;

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    handle->spdif.subFrames = 2;
    if ( handle->outputPort.mixer )
    {
        const BAPE_FMT_Descriptor     *pBfd = BAPE_Mixer_P_GetOutputFormat_isrsafe(handle->outputPort.mixer);

        dataType = pBfd->type;
        hbr = (unsigned)BAPE_FMT_P_IsHBR_isrsafe(pBfd);
        compressed = BAPE_FMT_P_IsCompressed_isrsafe(pBfd);
        compressedAsPcm = BAPE_FMT_P_IsDtsCdCompressed_isrsafe(pBfd);
        handle->spdif.subFrames = hbr ? 8 : BAPE_FMT_P_GetNumChannels_isrsafe(pBfd);
    }

    BDBG_MSG(("Set MAI CBITS SR %u, hbr %d, compressed %u, compressedAsPcm %u, channels %d",
              handle->sampleRate, hbr, compressed, compressedAsPcm, handle->spdif.subFrames));

    /* Program MAI format correctly */
    if ( handle->outputPort.mixer )
    {
        regAddr = BAPE_MAI_Reg_P_GetAddress(BCHP_DVP_CFG_MAI0_FMT, handle->index);
        BAPE_Reg_P_InitFieldList_isr(handle->deviceHandle, &regFieldList);

        if ( compressed && !compressedAsPcm )
        {
            if ( hbr )
            {
                BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, HDMI_MAI_FORMAT, AUDIO_FORMAT, HBR_compressed_8_channel);
            }
            else
            {
                BAPE_Reg_P_AddToFieldList_isr(&regFieldList, HDMI_MAI_FORMAT, AUDIO_FORMAT, 0xc2);
            }
        }
        else
        {
            switch ( dataType )
            {
            case BAPE_DataType_ePcm5_1:
                BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, HDMI_MAI_FORMAT, AUDIO_FORMAT, SPDIF_linearPCM_6_channel);
                break;
            case BAPE_DataType_ePcm7_1:
                BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, HDMI_MAI_FORMAT, AUDIO_FORMAT, SPDIF_linearPCM_8_channel);
                break;
            default:
            case BAPE_DataType_ePcmStereo:
                BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, HDMI_MAI_FORMAT, AUDIO_FORMAT, SPDIF_linearPCM_stereo);
                break;
            }
            BAPE_Reg_P_AddToFieldList_isr(&regFieldList, HDMI_MAI_FORMAT, SAMPLE_WIDTH, 0); /* 32 bits per sample */
        }

        if ( compressed || compressedAsPcm )
        {
            BAPE_Reg_P_AddToFieldList_isr(&regFieldList, HDMI_MAI_FORMAT, SAMPLE_WIDTH, 16);
        }

        if ( hbr )
        {
            BAPE_Reg_P_AddToFieldList_isr(&regFieldList, HDMI_MAI_FORMAT,
                                          SAMPLE_RATE, BAPE_MaiOutput_P_SampleRateToMaiFormat_isrsafe(192000));
        }
        else
        {
            BAPE_Reg_P_AddToFieldList_isr(&regFieldList, HDMI_MAI_FORMAT,
                                          SAMPLE_RATE, BAPE_MaiOutput_P_SampleRateToMaiFormat_isrsafe(handle->sampleRate));
        }

        BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, regAddr);

        /* Set sampling period */

        regAddr = BAPE_MAI_Reg_P_GetAddress(BCHP_DVP_CFG_MAI0_SMP, handle->index);
        BAPE_Reg_P_InitFieldList_isr(handle->deviceHandle, &regFieldList);
        n = m = 0;
        if (!BAPE_MaiOutput_P_SampleRateToPeriod_isrsafe(hbr ? 192000 : handle->sampleRate, &n, &m)) {
            BDBG_ERR(("can't accurately set MAI sample period for %d", hbr ? 192000 : handle->sampleRate));
        }
        BAPE_Reg_P_AddToFieldList_isr(&regFieldList, DVP_CFG_MAI0_SMP, SMP_N, n);
        BAPE_Reg_P_AddToFieldList_isr(&regFieldList, DVP_CFG_MAI0_SMP, SMP_M, m);
        BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, regAddr);

        /* Set #channels */

        regAddr = BAPE_MAI_Reg_P_GetAddress(BCHP_DVP_CFG_MAI0_CTL, handle->index);
        BAPE_Reg_P_InitFieldList_isr(handle->deviceHandle, &regFieldList);
        BAPE_Reg_P_AddToFieldList_isr(&regFieldList, DVP_CFG_MAI0_CTL, CHNUM, handle->spdif.subFrames);
        BAPE_Reg_P_AddToFieldList_isr(&regFieldList, DVP_CFG_MAI0_CTL, PAREN, 1);
        BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, regAddr);
        if (!handle->settings.useRawChannelStatus)
        {
            BAPE_Spdif_P_ChannelStatusBits cbits;
            BAPE_P_MapSpdifChannelStatusToBits_isr(&handle->outputPort, &handle->settings.channelStatus, &cbits);
            handle->spdif.channelStatus[0] = cbits.bits[0];
            handle->spdif.channelStatus[1] = cbits.bits[1];
            handle->spdif.channelStatus[2] = cbits.bits[2];
            handle->spdif.channelStatus[3] = 0;
            handle->spdif.channelStatus[4] = 0;
            handle->spdif.channelStatus[5] = 0;
            BDBG_MSG(("%s: %.8x %.8x %.8x", BSTD_FUNCTION, handle->spdif.channelStatus[0], handle->spdif.channelStatus[1], handle->spdif.channelStatus[2]));
        }
        handle->spdif.clk = 0;
    }

    return;
}

#ifdef SOFTWARE_PARITY
bool
parityodd(uint32_t v)
{
    v = (v >> 16) ^ (v & 0xffff);
    v = (v >> 8) ^ (v & 0xff);
    v = (v >> 4) ^ (v & 0xf);
    v = (v >> 2) ^ (v & 0x3);
    v = (v >> 1) ^ (v & 0x1);
    return v;
}
#endif

static void
BAPE_MaiOutput_P_SpdifEncode_Ott_isr(void *ctx, BAPE_BufferDescriptor *pDesc, uint8_t *pIn, unsigned inCount, uint8_t *pOut, unsigned outCount, unsigned *inSize, unsigned *outSize)
{
    unsigned bytesPerSample;
    unsigned framesToCopy;
    unsigned totalBytesRead, totalBytesWritten;
    BAPE_MaiOutputHandle handle = (BAPE_MaiOutputHandle)ctx;
    switch (pDesc->bitsPerSample) {
    case 8:
        bytesPerSample = 1;
        break;
    case 16:
        bytesPerSample = 2;
        break;
    case 24:
        bytesPerSample = 3;
        break;
    case 32:
        bytesPerSample = 4;
        break;
    default:
        BDBG_ASSERT(0);
    }
    framesToCopy = inCount / (bytesPerSample * handle->spdif.subFrames);
    if (framesToCopy > outCount / (4 * handle->spdif.subFrames)) {
        framesToCopy = outCount / (4 * handle->spdif.subFrames);
    }
    totalBytesRead = framesToCopy * bytesPerSample * handle->spdif.subFrames;
    totalBytesWritten = framesToCopy * 4 * handle->spdif.subFrames;
    /*
     * assume host order, 24 bit, signed, HDMI_MAI_CONFIG.MAI_BIT_REVERSE == 1, DVP_CFG_MAI0_CTL.PAREN == 1
     * PCUV LLLL LLLL LLLL LLLL 0000 0000 'M or B' PCUV RRRR RRRR RRRR RRRR 'W'
     * 'M' 4 (or 0)
     * 'B' 1 (or f)
     * 'W' 2 (or same as subframe 0)
     */
    while (framesToCopy) {
        uint32_t flags;
        bool cb;
        unsigned c;
        if (handle->settings.useRawChannelStatus) {
            cb = (handle->settings.rawChannelStatus[handle->spdif.clk >> 3] & (1 << (handle->spdif.clk & 7))) != 0;
        }
        else {
            cb = (handle->spdif.channelStatus[handle->spdif.clk >> 5] & (1 << (handle->spdif.clk & 31))) != 0;
        }
        flags = 0x00000000; /* V */
        if (cb) {
            flags |= 0x40000000;
        }
        if (handle->spdif.clk == 0) {
            flags |= 0x1; /* B */
        }
        else {
            flags |= 0x4; /* M */
        }
        if (++handle->spdif.clk >= 192) {
            handle->spdif.clk = 0;
        }
        /* assume signed, littleendian */
        for (c = 0; c < handle->spdif.subFrames; c++) {
            uint32_t samp;
            switch (bytesPerSample) {
            case 1:
                samp = *pIn++;
                samp = (samp << 20) | flags;
                break;
            case 2:
                samp = *pIn++;
                samp |= (*pIn++) << 8;
                samp = (samp << 12) | flags;
                break;
            case 3:
                samp = *pIn++;
                samp |= (*pIn++) << 8;
                samp |= (*pIn++) << 16;
                samp = (samp << 8) | flags;
                break;
            case 4:
                pIn++; /* skip LSB */
                samp = *pIn++;
                samp |= (*pIn++) << 8;
                samp |= (*pIn++) << 16;
                samp = (samp << 8) | flags;
                break;
            }

#ifdef SOFTWARE_PARITY
            if (parityodd(samp)) {
                samp |= 0x80000000;
            }
#endif
            *(uint32_t *)pOut = samp;
            flags = (flags & 0xfffffff0) | 0x2; /* W */
            pOut += 4;
        }
        framesToCopy--;
    }
    *inSize = totalBytesRead;
    *outSize = totalBytesWritten;
    BDBG_MSG(("BAPE_MaiOutput_P_SpdifEncode_Ott_isr: %d/%d", totalBytesRead, totalBytesWritten));
}

static BERR_Code
BAPE_MaiOutput_P_Enable_Ott(BAPE_OutputPort output)
{
    BAPE_MaiOutputHandle handle;
    BERR_Code rc;
    const BAPE_FMT_Descriptor *pFormat;

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    if (output->mixer == NULL) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    pFormat = BAPE_Mixer_P_GetOutputFormat_isrsafe(output->mixer);
    handle->spdif.cpuEncode = pFormat->type != BAPE_DataType_eIec60958Raw;

    /*
     * set MAI_BIT_REVERSE if CPU doing encoding
     * softFMM requirements TBD
     */
    BREG_AtomicUpdate32(handle->deviceHandle->regHandle,
                        BCHP_HDMI_MAI_CONFIG,
                        BCHP_MASK(HDMI_MAI_CONFIG, MAI_BIT_REVERSE)
                        | BCHP_MASK(HDMI_MAI_CONFIG, MAI_FORMAT_REVERSE),
                        BCHP_FIELD_DATA(HDMI_MAI_CONFIG, MAI_BIT_REVERSE, handle->spdif.cpuEncode)
                        | BCHP_FIELD_DATA(HDMI_MAI_CONFIG, MAI_FORMAT_REVERSE, 1));

    rc = BAPE_DmaOutput_P_Bind(handle->dma, output->mixer->bufferGroupHandle, handle->spdif.cpuEncode ? BAPE_MaiOutput_P_SpdifEncode_Ott_isr : 0, handle);
    if (rc != BERR_SUCCESS) {
        return BERR_TRACE(rc);
    }
    rc = BAPE_DmaOutput_P_Enable(handle->dma, true);
    if (rc != BERR_SUCCESS) {
        BAPE_DmaOutput_P_UnBind(handle->dma);
        return BERR_TRACE(rc);
    }

    /* Update CBITS */
    BKNI_EnterCriticalSection();
    BAPE_MaiOutput_P_SetCbits_isr(handle);
    BKNI_LeaveCriticalSection();

    /* enable MAI */
    {
        uint32_t reg = BAPE_MAI_Reg_P_GetAddress(BCHP_DVP_CFG_MAI0_CTL, handle->index);
        uint32_t ctl = BREG_Read32(handle->deviceHandle->regHandle, reg);
        if ((ctl & BCHP_MASK(DVP_CFG_MAI0_CTL, ENABLE)) == 0) {
            ctl |= BCHP_FIELD_DATA(DVP_CFG_MAI0_CTL, ENABLE, 1);
            BREG_Write32(handle->deviceHandle->regHandle, reg, ctl);
            BREG_Write32(handle->deviceHandle->regHandle, reg, ctl | BCHP_FIELD_DATA(DVP_CFG_MAI0_CTL, FLUSH, 1));
            BREG_Write32(handle->deviceHandle->regHandle, reg, ctl);
        }
    }

    return BERR_SUCCESS;
}

static void
BAPE_MaiOutput_P_Disable_Ott(BAPE_OutputPort output)
{
    BAPE_MaiOutputHandle handle;

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    /* disable MAI */
    BAPE_DmaOutput_P_UnBind(handle->dma);
}

static void
BAPE_MaiOutput_P_SetMclk_Ott_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
    BSTD_UNUSED(output);
    BSTD_UNUSED(mclkSource);
    BSTD_UNUSED(pllChannel);
    BSTD_UNUSED(mclkFreqToFsRatio);
}

static BERR_Code
BAPE_MaiOutput_P_OpenHw_Ott(BAPE_MaiOutputHandle handle)
{
    uint32_t reg = BAPE_MAI_Reg_P_GetAddress(BCHP_DVP_CFG_MAI0_CTL, handle->index);
    uint32_t ctl =
        BCHP_FIELD_DATA(DVP_CFG_MAI0_CTL, CHALIGN, 1)
        | BCHP_FIELD_DATA(DVP_CFG_MAI0_CTL, WHOLSMP, 1)
        | BCHP_FIELD_DATA(DVP_CFG_MAI0_CTL, CHNUM, 2);
    uint32_t clr =
        BCHP_FIELD_DATA(DVP_CFG_MAI0_CTL, DLATE, 1)
        | BCHP_FIELD_DATA(DVP_CFG_MAI0_CTL, ERRORE, 1)
        | BCHP_FIELD_DATA(DVP_CFG_MAI0_CTL, ERRORF, 1);
    BREG_Write32(handle->deviceHandle->regHandle, reg, BCHP_FIELD_DATA(DVP_CFG_MAI0_CTL, RST_MAI, 1));
    BREG_Write32(handle->deviceHandle->regHandle, reg, clr);
    BREG_Write32(handle->deviceHandle->regHandle, reg, ctl);
    /* DMA thresholds (from Patrick, ??units??)
     * panic high: 0x32
     * panic low: 0x08
     * dreq high: 0x01
     * dreq low: 0x01
     */
    BREG_Write32(handle->deviceHandle->regHandle, BAPE_MAI_Reg_P_GetAddress(BCHP_DVP_CFG_MAI0_THR, handle->index), 0x32080108);

    return BERR_SUCCESS;
}

static void
BAPE_MaiOutput_P_CloseHw_Ott(BAPE_MaiOutputHandle output)
{
    BSTD_UNUSED(output);
}

static void
BAPE_MaiOutput_P_SetCrossbar_Ott(BAPE_MaiOutputHandle output, BAPE_StereoMode stereoMode)
{
    BSTD_UNUSED(output);
    BSTD_UNUSED(stereoMode);
}

static void
BAPE_MaiOutput_P_SetMute_Ott(BAPE_OutputPort output, bool muted)
{
    BSTD_UNUSED(output);
    BSTD_UNUSED(muted);
}

static BERR_Code
BAPE_MaiOutput_P_SetBurstConfig_Ott(BAPE_MaiOutputHandle output)
{
    BSTD_UNUSED(output);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

#else
/**************************************************************************
Legacy 7425 style AIO
**************************************************************************/

static BAPE_MaiOutputDataPath BAPE_MaiOutput_P_FormatToDataPath(const BAPE_FMT_Descriptor *pFormat)
{
    if ( pFormat )
    {
        switch ( pFormat->type )
        {
        case BAPE_DataType_ePcmStereo:
        case BAPE_DataType_eIec61937:
        case BAPE_DataType_eIec61937x4:
            return BAPE_MaiOutputDataPath_eStereo;
        case BAPE_DataType_ePcm5_1:
        case BAPE_DataType_ePcm7_1:
            return BAPE_MaiOutputDataPath_eMaiMulti;
        case BAPE_DataType_eIec61937x16:
            return BAPE_MaiOutputDataPath_eHbr;
        default:
            break;
        }
    }
    return BAPE_MaiOutputDataPath_eNone;
}

static BERR_Code BAPE_MaiOutput_P_Open_Legacy(BAPE_MaiOutputHandle handle)
{
    handle->muteBufferBlock = BMMA_Alloc(handle->deviceHandle->memHandle, BAPE_P_MUTE_BUFFER_SIZE, 32, NULL);
    if ( NULL == handle->muteBufferBlock )
    {
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    handle->pMuteBuffer = BMMA_Lock(handle->muteBufferBlock);
    if ( NULL == handle->pMuteBuffer )
    {
        BMMA_Free(handle->muteBufferBlock);
        handle->muteBufferBlock = NULL;
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    handle->muteBufferOffset = BMMA_LockOffset(handle->muteBufferBlock);
    if ( 0 == handle->muteBufferOffset )
    {
        BMMA_Unlock(handle->muteBufferBlock, handle->pMuteBuffer);
        handle->pMuteBuffer = NULL;
        BMMA_Free(handle->muteBufferBlock);
        handle->muteBufferBlock = NULL;
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }

    BAPE_MaiOutput_P_SetBurstConfig_Legacy(handle);

    return BERR_SUCCESS;
}

static void BAPE_MaiOutput_P_Close_Legacy(BAPE_MaiOutputHandle handle)
{
    if ( handle->muteBufferBlock )
    {
        if ( handle->muteBufferOffset )
        {
            BMMA_UnlockOffset(handle->muteBufferBlock, handle->muteBufferOffset);
            handle->muteBufferOffset = 0;
        }
        if ( handle->pMuteBuffer )
        {
            BMMA_Unlock(handle->muteBufferBlock, handle->pMuteBuffer);
            handle->pMuteBuffer = NULL;
        }
        BMMA_Free(handle->muteBufferBlock);
        handle->muteBufferBlock = NULL;
    }
}

static void BAPE_MaiOutput_P_SetCbits_Legacy_isr(BAPE_MaiOutputHandle handle)
{
    uint32_t regAddr, regVal;
    unsigned validity = 0;
    BAPE_DataType dataType = BAPE_DataType_ePcmStereo;
    unsigned compressed = 0;
    unsigned hbr = 0;
    unsigned dither = 0;
    bool compressedAsPcm = false;
    const BAPE_FMT_Descriptor     *pFormat;

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    if ( handle->outputPort.mixer )
    {
        pFormat = BAPE_Mixer_P_GetOutputFormat_isrsafe(handle->outputPort.mixer);

        dataType = pFormat->type;
        compressed = (unsigned)BAPE_FMT_P_IsCompressed_isrsafe(pFormat);
        hbr = (unsigned)BAPE_FMT_P_IsHBR_isrsafe(pFormat);
        compressedAsPcm = BAPE_FMT_P_IsDtsCdCompressed_isrsafe(pFormat);
    }
    else
    {
        /* No update required, maintain previous settings until connected.  Update will be applied in Enable(). */
        return;
    }

    BDBG_MSG(("Set MAI CBITS SR %u compressed %u, compressedAsPcm %u", handle->sampleRate, compressed, compressedAsPcm));

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

    /* Program MAI format correctly */
    if ( handle->outputPort.mixer )
    {
        regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MAI_FORMAT);
        regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_FORMAT, AUDIO_FORMAT)|
                    BCHP_MASK(AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_WIDTH)|
                    BCHP_MASK(AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_RATE));
        if ( compressed && !compressedAsPcm )
        {
            if ( hbr )
            {
                regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_FORMAT, AUDIO_FORMAT, SPDIF_Comp_8_Channel);
            }
            else
            {
                regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_FORMAT, AUDIO_FORMAT, SPDIF_Comp_2_Channel);
            }
        }
        else
        {
            switch ( dataType )
            {
            case BAPE_DataType_ePcm5_1:
                regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_FORMAT, AUDIO_FORMAT, SPDIF_PCM_6_Channel);
                break;
            case BAPE_DataType_ePcm7_1:
                regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_FORMAT, AUDIO_FORMAT, SPDIF_PCM_8_Channel);
                break;
            default:
            case BAPE_DataType_ePcmStereo:
                regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_FORMAT, AUDIO_FORMAT, SPDIF_PCM_2_Channel);
                break;
            }
            regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_WIDTH, 0); /* 32 bits per sample */
        }

        if ( compressed || compressedAsPcm )
        {
            regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_WIDTH, 16);    /* TODO: */
        }

        if ( hbr )
        {
            regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_RATE,
                                      BAPE_MaiOutput_P_SampleRateToMaiFormat_isrsafe(192000));
        }
        else
        {
            regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_RATE,
                                      BAPE_MaiOutput_P_SampleRateToMaiFormat_isrsafe(handle->sampleRate));
        }
        BDBG_MSG(("Set MAI format to 0x%x", regVal));
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MAI_FORMAT, regVal);
    }

    BDBG_MSG(("Set MAI HW CBITS SR %u compressed %u hbr %u validity %u dither %u", handle->sampleRate, compressed, hbr, validity, dither));
    /* Tell the HW to hold the current CBITS and set dither correctly */
    regAddr = BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1;
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, HOLD_CSTAT, Hold);
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);

    regVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, DITHER_ENA)|
                BCHP_MASK(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, VALIDITY));
    regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, DITHER_ENA, dither);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, VALIDITY, validity);
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);

    /* JDG - Do not change the value of AUD_FMM_MS_CTRL_STRM_ENA.STREAM1_ENA here.
             This causes upstream requests to stop if you are using the Stereo datapath.  That can cause
             mixer rate errors and FMM lockups.  So, we need to leave this enabled always.  For MAI_Multi and HBR
             paths, this is disabled already so this has no effect. */

    /* Program channel status */
    if ( handle->settings.useRawChannelStatus )
    {
        regVal =
            (uint32_t)handle->settings.rawChannelStatus[0] |
            (((uint32_t)handle->settings.rawChannelStatus[1])<<8) |
            (((uint32_t)handle->settings.rawChannelStatus[2])<<16) |
            (((uint32_t)handle->settings.rawChannelStatus[3])<<24);
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, regVal);
        regVal = (uint32_t)handle->settings.rawChannelStatus[4];
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, regVal);
    }
    else if ( compressed && !compressedAsPcm )
    {
        regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1);
        regVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_PRO_CONS)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_COMP_LIN)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_CP)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_EMPH)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_CMODE)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_CATEGORY)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_SOURCE)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_FREQ)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_ACCURACY)|
#ifdef BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1_Compressed_BITS_31_to_30_MASK
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_BITS_31_to_30)
#else
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_FREQ_EXTN)
#endif
                    );
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_PRO_CONS, handle->settings.channelStatus.professional);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_COMP_LIN, 1);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_CP, (handle->settings.channelStatus.copyright)?0:1);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_CATEGORY, handle->settings.channelStatus.categoryCode);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_SOURCE, handle->settings.channelStatus.sourceNumber);
        if ( dataType == BAPE_DataType_eIec61937x16 )
        {
            regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_FREQ, BAPE_P_GetConsumerSampleRateCstatCode_isr(768000));
        }
        else
        {
            regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_FREQ, BAPE_P_GetConsumerSampleRateCstatCode_isr(handle->sampleRate));
        }
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_ACCURACY, handle->settings.channelStatus.clockAccuracy);
        BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, regVal);
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, 0);
    }
    else
    {
        regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1);
        regVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_PRO_CONS)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_COMP_LIN)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_CP)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_EMPH)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_CMODE)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_CATEGORY)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_SOURCE)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_FREQ)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_ACCURACY)|
#ifdef BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1_Compressed_BITS_31_to_30_MASK
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_BITS_31_to_30)
#else
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_FREQ_EXTN)
#endif
                    );
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_PRO_CONS, handle->settings.channelStatus.professional);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_CP, (handle->settings.channelStatus.copyright)?0:1);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_CATEGORY, handle->settings.channelStatus.categoryCode);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_SOURCE, handle->settings.channelStatus.sourceNumber);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_FREQ, BAPE_P_GetConsumerSampleRateCstatCode_isr(handle->sampleRate));
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_ACCURACY, handle->settings.channelStatus.clockAccuracy);
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, regVal);
        regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1);
        regVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, PCM_MAX_LEN)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, PCM_LENGTH)|
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, PCM_ORIG_FREQ));
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, PCM_MAX_LEN, 1);    /* 24-bits */
        #ifdef BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1_PCM_CGMS_A_MASK
        regVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, PCM_CGMS_A));
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, PCM_CGMS_A, handle->settings.channelStatus.cgmsA);
        #endif
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_HI_1, regVal);
    }

    /* Begin using new bits */
    regAddr = BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1;
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, HOLD_CSTAT);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, HOLD_CSTAT, Update);
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);

    return;
}

/**************************************************************************/

static void BAPE_MaiOutput_P_SetMclk_Legacy_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
    uint32_t regVal, pllclksel=0;
    uint32_t mclkRate;

    BAPE_MaiOutputHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(handle->offset == 0);

    /* Save the settings in case we need to re-apply them later. */
    handle->mclkInfo.mclkSource         = mclkSource;
    handle->mclkInfo.pllChannel         = pllChannel;
    handle->mclkInfo.mclkFreqToFsRatio  = mclkFreqToFsRatio;

    switch ( mclkSource )
    {
    /* PLL Timing */
    #if BAPE_CHIP_MAX_PLLS > 0
    case BAPE_MclkSource_ePll0:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI_PLLCLKSEL_PLL0_ch1 + pllChannel;
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 1
    case BAPE_MclkSource_ePll1:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI_PLLCLKSEL_PLL1_ch1 + pllChannel;
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 2
    case BAPE_MclkSource_ePll2:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI_PLLCLKSEL_PLL2_ch1 + pllChannel;
        break;
    #endif

    /* DAC Timing */
    #if BAPE_CHIP_MAX_DACS > 0
    case BAPE_MclkSource_eHifidac0:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI_PLLCLKSEL_Hifidac0;
        break;
    #endif
    #if BAPE_CHIP_MAX_DACS > 1
    case BAPE_MclkSource_eHifidac1:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI_PLLCLKSEL_Hifidac1;
        break;
    #endif
    #if BAPE_CHIP_MAX_DACS > 2
    case BAPE_MclkSource_eHifidac2:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI_PLLCLKSEL_Hifidac2;
        break;
    #endif

    /* NCO (Mclkgen) Timing */
    #if BAPE_CHIP_MAX_NCOS > 0
    case BAPE_MclkSource_eNco0:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI_PLLCLKSEL_Mclk_gen0;
        break;
    #endif
    #if BAPE_CHIP_MAX_NCOS > 1
    case BAPE_MclkSource_eNco1:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI_PLLCLKSEL_Mclk_gen1;
        break;
    #endif
    #if BAPE_CHIP_MAX_NCOS > 2
    case BAPE_MclkSource_eNco2:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI_PLLCLKSEL_Mclk_gen2;
        break;
    #endif

    /* Should never get here */
    default:
        BDBG_ERR(("mclkSource (%u) doesn't refer to a valid PLL DAC, or NCO", mclkSource));
        BDBG_ASSERT(false);     /* something went wrong somewhere! */
        return;
    }

    /* Tell the output formatter how fast our mclk is. */
    mclkRate = mclkFreqToFsRatio / ( 2 * 64 );  /* MAI has sclk = 64Fs */

    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI, PLLCLKSEL)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI, MCLK_RATE));
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI, MCLK_RATE, mclkRate);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI, PLLCLKSEL, pllclksel);
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI, regVal);
}

static void BAPE_MaiOutput_P_SetMute_Legacy(BAPE_OutputPort output, bool muted)
{
    BAPE_MaiOutputHandle handle;
    BAPE_MixerGroupInputSettings dataInputSettings, burstInputSettings;
    const BAPE_FMT_Descriptor *pFormat;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    pFormat = BAPE_Mixer_P_GetOutputFormat_isrsafe(output->mixer);
    BDBG_ASSERT(NULL != pFormat);

    if ( BAPE_FMT_P_IsCompressed_isrsafe(pFormat) )
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
            (void)BAPE_MixerGroup_P_SetInputSettings(handle->hMixer, 0, &dataInputSettings);
            (void)BAPE_MixerGroup_P_SetInputSettings(handle->hMixer, 1, &burstInputSettings);
        }
        else
        {
            dataInputSettings.coefficients[0][0][0] = 0x800000;
            dataInputSettings.coefficients[0][1][1] = 0x800000;
            burstInputSettings.coefficients[0][0][0] = 0;
            burstInputSettings.coefficients[0][1][1] = 0;
            (void)BAPE_MixerGroup_P_SetInputSettings(handle->hMixer, 1, &burstInputSettings);
            (void)BAPE_MixerGroup_P_SetInputSettings(handle->hMixer, 0, &dataInputSettings);
        }
    }
    handle->muted = muted;
}

static BERR_Code BAPE_MaiOutput_P_Enable_Legacy(BAPE_OutputPort output)
{
    BAPE_IopStreamSettings streamSettings;
    unsigned streamId;

    BAPE_MaiOutputHandle handle;
    BREG_Handle regHandle;
    BAPE_Reg_P_FieldList regFieldList;
    const BAPE_FMT_Descriptor *pFormat;
    uint32_t streamEnables, regAddr;
    unsigned i, numChannelPairs;
    bool enableNewPath=false;
    BAPE_MaiOutputDataPath newDataPath;
    BAPE_MixerGroupInputSettings dataInputSettings, burstInputSettings;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(false == handle->enabled);
    BDBG_ASSERT(NULL != output->mixer);

    regHandle = handle->deviceHandle->regHandle;

    BDBG_MSG(("Enabling %s", handle->name));

    /* apply loudness equivalance levels*/
    BAPE_MaiOutput_P_SetLoudnessEquivlanceVolume(output);
    BAPE_MaiOutput_SetSettings(handle, &handle->settings);

    pFormat = BAPE_Mixer_P_GetOutputFormat_isrsafe(output->mixer);
    BDBG_ASSERT(NULL != pFormat);
    numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(pFormat);
    BDBG_ASSERT(numChannelPairs <= 4);

    BDBG_MSG(("Starting MAI output %u using data type %s", handle->index, BAPE_FMT_P_GetTypeName_isrsafe(pFormat)));

    /* Set MAI/I2S Multi Grouping based on number of channel pairs */
    regAddr = BCHP_AUD_FMM_OP_CTRL_MAI_MULTI_GROUPING;
    #if 0 /* This grouping does not seem to be stable in HW.  If programmed, once in a handful of tries the HW doesn't start */
    /* We need to write 1's in the fields that are multichannel.  For all stereo, write 0 */
    if ( numChannelPairs == 1 )
    {
        BREG_Write32(regHandle, regAddr, 0);
    }
    else
    {
        BREG_Write32(regHandle, regAddr, 0xf>>(4-numChannelPairs));
    }
    #else
    BREG_Write32(regHandle, regAddr, 0);
    #endif

    newDataPath = BAPE_MaiOutput_P_FormatToDataPath(pFormat);

    /* Setup new data layout if required */
    if ( newDataPath != handle->dataPath )
    {
        enableNewPath = true;

        if ( handle->dataPath != BAPE_MaiOutputDataPath_eNone )
        {
            /* Stop stream 1 path */
            BAPE_SfifoGroup_P_Stop(handle->hSfifo);
            BAPE_MixerGroup_P_StopInput(handle->hMixer, 1);
            BAPE_MixerGroup_P_StopOutput(handle->hMixer, 0);
            BREG_Write32(regHandle, BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR, 1<<GET_MAI_STEREO_STREAM_ID(handle->index));
            /* If HBR, stop the HBR output path */
            if ( handle->dataPath == BAPE_MaiOutputDataPath_eHbr )
            {
                BAPE_MixerGroup_P_StopOutput(handle->hMixer, 1);
                BREG_Write32(regHandle, BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR, 1<<GET_MAI_HBR_STREAM_ID(handle->index));
            }
            handle->dataPath = BAPE_MaiOutputDataPath_eNone;
        }
    }

    for ( i = 0; i < numChannelPairs; i++ )
    {
        switch ( newDataPath )
        {
        case BAPE_MaiOutputDataPath_eStereo:
        case BAPE_MaiOutputDataPath_eHbr:
            streamId = GET_MAI_STEREO_STREAM_ID(handle->index);
            BDBG_MSG(("Stereo/HBR Payload source FCI 0x%x", output->sourceMixerFci.ids[i]));
            /* Setup actual data source as primary mixer input */
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
            break;
        case BAPE_MaiOutputDataPath_eMaiMulti:
            streamId = GET_MAI_MULTI_STREAM_ID(handle->index, i);
            BDBG_MSG(("MAI Multi Payload [stream %u] source FCI 0x%x", streamId, output->sourceMixerFci.ids[i]));
            /* Write source FCI to IOP */
            BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
            streamSettings.resolution = 24;
            streamSettings.input = output->sourceMixerFci.ids[i];   /* Take source FCI provided from mixer */
            (void)BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
            break;
        default:
            break;
        }
    }

    /* Setup for HBR vs. non-HBR output */
    #ifdef BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE_HBR_ENABLE_MASK
    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
    if ( newDataPath == BAPE_MaiOutputDataPath_eHbr )
    {
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_PAYLOAD_SEL, HBR_channel);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, HBR_ENABLE, 1);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, HBR_HW_CH_STATUS_EN, 1);
    }
    else
    {
        if ( newDataPath == BAPE_MaiOutputDataPath_eMaiMulti )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_PAYLOAD_SEL, MAI_Multi);
        }
        else
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_PAYLOAD_SEL, Stereo);
        }
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, HBR_ENABLE, 0);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, HBR_HW_CH_STATUS_EN, 0);
    }
    BAPE_Reg_P_ApplyFieldList(&regFieldList, BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE);
    #endif

    /* Enable the formatter using HW channel status */
    BKNI_EnterCriticalSection();
    BAPE_MaiOutput_P_SetCbits_isr(handle);
    if ( newDataPath == BAPE_MaiOutputDataPath_eStereo )
    {
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA, AUD_FMM_MS_CTRL_STRM_ENA, STREAM1_ENA, Enable);
    }
    else /* HBR/Multichannel */
    {
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA, AUD_FMM_MS_CTRL_STRM_ENA, STREAM1_ENA, Disable);
    }
    BKNI_LeaveCriticalSection();

    /* Enable output streams always */
    streamEnables = (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(GET_MAI_STEREO_STREAM_ID(handle->index));
    switch ( newDataPath )
    {
    case BAPE_MaiOutputDataPath_eMaiMulti:
        for ( i = 0; i < numChannelPairs; i++ )
        {
            streamEnables |= (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(GET_MAI_MULTI_STREAM_ID(handle->index, i));
        }
        break;
    case BAPE_MaiOutputDataPath_eHbr:
        streamEnables |= (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(GET_MAI_HBR_STREAM_ID(handle->index));
        break;
    default:
        break;
    }
    BREG_Write32(regHandle, BCHP_AUD_FMM_OP_CTRL_ENABLE_SET, streamEnables);

    if ( enableNewPath )
    {
        /* Enable remaining path consumer..producer */
        if ( newDataPath == BAPE_MaiOutputDataPath_eHbr )
        {
            /* Enable the HBR output from mixer output 1 only in HBR mode */
            (void)BAPE_MixerGroup_P_StartOutput(handle->hMixer, 1);
        }
        else
        {
            (void)BAPE_MixerGroup_P_StartOutput(handle->hMixer, 0);
        }
        (void)BAPE_MixerGroup_P_StartInput(handle->hMixer, 1);  /* Input for pauseburst */
        (void)BAPE_SfifoGroup_P_Start(handle->hSfifo, false);   /* Sfifo for pauseburst */
        handle->dataPath = newDataPath;
    }

    /* Last, enable the actual data input for stereo/hbr mode */
    switch ( newDataPath )
    {
    case BAPE_MaiOutputDataPath_eStereo:
    case BAPE_MaiOutputDataPath_eHbr:
        (void)BAPE_MixerGroup_P_StartInput(handle->hMixer, 0);  /* Input for data */
        break;
    default:
        break;
    }

    handle->enabled = true;
    return BERR_SUCCESS;
}

/**************************************************************************/

static void BAPE_MaiOutput_P_Disable_Legacy(BAPE_OutputPort output)
{
    BAPE_MaiOutputHandle handle;
    const BAPE_FMT_Descriptor *pFormat;
    unsigned i, numChannelPairs;

    BERR_Code errCode;
    uint32_t regVal;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(NULL != output->mixer);

    BDBG_MSG(("Disabling %s", handle->name));
    pFormat = BAPE_Mixer_P_GetOutputFormat_isrsafe(output->mixer);
    BDBG_ASSERT(NULL != pFormat);
    numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(pFormat);
    BDBG_ASSERT(numChannelPairs <= 4);

    /* We need to stop producer..consumer.  First stop consuming from the real data source */
    switch ( handle->dataPath )
    {
    case BAPE_MaiOutputDataPath_eStereo:
    case BAPE_MaiOutputDataPath_eHbr:
        BAPE_MixerGroup_P_StopInput(handle->hMixer, 0);
        break;
    case BAPE_MaiOutputDataPath_eMaiMulti:
        regVal = 0;
        for ( i = 0; i < numChannelPairs; i++ )
        {
            regVal |= 1<<GET_MAI_MULTI_STREAM_ID(handle->index, i);
        }
        BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR, regVal);
        break;
    default:
        break;
    }

    /* Now data consumption has stopped, clear out FCI ID's */
    if ( handle->dataPath == BAPE_MaiOutputDataPath_eMaiMulti )
    {
        unsigned streamId;
        for ( i = 0; i < numChannelPairs; i++ )
        {
            BAPE_IopStreamSettings streamSettings;
            streamId = GET_MAI_MULTI_STREAM_ID(handle->index, i);
            /* Write source FCI to IOP */
            BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
            streamSettings.input = BAPE_FCI_ID_INVALID;
            errCode = BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
            BDBG_ASSERT(BERR_SUCCESS == errCode);
        }
    }
    else
    {
        BAPE_MixerGroupInputSettings mixerInputSettings;
        BAPE_MixerGroup_P_GetInputSettings(handle->hMixer, 0, &mixerInputSettings);
        BAPE_FciIdGroup_Init(&mixerInputSettings.input);
        (void)BAPE_MixerGroup_P_SetInputSettings(handle->hMixer, 0, &mixerInputSettings);
    }

    handle->enabled = false;
}

/**************************************************************************/

static BERR_Code BAPE_MaiOutput_P_OpenHw_Legacy(BAPE_MaiOutputHandle handle)
{
    BERR_Code       errCode = BERR_SUCCESS;
    BAPE_Handle     deviceHandle;
    uint32_t        streamId;
    BAPE_SfifoGroupCreateSettings sfifoCreateSettings;
    BAPE_MixerGroupCreateSettings mixerCreateSettings;
    BAPE_SfifoGroupSettings sfifoSettings;
    BAPE_MixerGroupSettings mixerSettings;
    BAPE_MixerGroupInputSettings mixerInputSettings;
    BAPE_FciIdGroup fciGroup;
    BAPE_Reg_P_FieldList regFieldList;
    BAPE_IopStreamSettings streamSettings;

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    BDBG_ASSERT(handle->index == 0);

    /* Setup Channel Status Formatter and enable always.  This solves a lot of receiver compatibility issues. */
    BKNI_EnterCriticalSection();
    {
        /* Enable HW cbit formatter */
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS, AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM1, Bypass);
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, ENABLE, Enable);
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA, AUD_FMM_MS_CTRL_STRM_ENA, STREAM1_ENA, Enable);

        /* Disable FW cbit formatter */
        BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, OVERWRITE_DATA, Disable);
        BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, STREAM_ENA, Disable);
        BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1);

        /* Default cbit Sample Rate to 48k */
        BAPE_Reg_P_UpdateField(handle->deviceHandle, BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_FREQ, 2);

        BAPE_MaiOutput_P_SetCbits_isr(handle);
    }
    BKNI_LeaveCriticalSection();

    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, STRM1_TMG_SRC_SEL, MAI_Multi);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_PAYLOAD_SEL, Stereo);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_CSTAT_SEL, STRM1);
    #ifdef BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE_HBR_ENABLE_MASK
    BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, HBR_ENABLE, 0);
    BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, HBR_HW_CH_STATUS_EN, 0);
    #endif
    BAPE_Reg_P_ApplyFieldList(&regFieldList, BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE);

    /* The data going to the MS doesn't matter.  Set it to an invalid FCI ID */
    streamId = GET_MAI_CBIT_STREAM_ID(handle->index);
    BDBG_MSG(("MAI Channel Status [stream %u] source FCI 0x%x", streamId, BAPE_FCI_ID_INVALID));

    /* Write dummy source FCI to IOP */
    BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
    streamSettings.resolution = 24;
    streamSettings.input = BAPE_FCI_ID_INVALID;
    errCode = BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Enable MAI Bus */
    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_OP_CTRL_MAI_CFG, SPDIF_MODE, SPDIF_Format);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_OP_CTRL_MAI_CFG, ENABLE_MAI, Enable);
    BAPE_Reg_P_ApplyFieldList(&regFieldList, BCHP_AUD_FMM_OP_CTRL_MAI_CFG);

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
    sfifoSettings.bufferInfo[0].block = handle->muteBufferBlock;
    sfifoSettings.bufferInfo[0].pBuffer = handle->pMuteBuffer;
    sfifoSettings.bufferInfo[0].base = handle->muteBufferOffset;
    sfifoSettings.bufferInfo[0].length = BAPE_P_MUTE_BUFFER_SIZE;
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
    BAPE_SfifoGroup_P_GetOutputFciIds_isrsafe(handle->hSfifo, &mixerInputSettings.input);
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

    BKNI_EnterCriticalSection();
    BAPE_MaiOutput_P_SetMclk_isr(&handle->outputPort, BAPE_MclkSource_eHifidac0, 0, 256);
    BKNI_LeaveCriticalSection();

    /* Setup stereo data path as mixer output 0 */
    BAPE_MixerGroup_P_GetOutputFciIds(handle->hMixer, 0, &fciGroup);
    BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, GET_MAI_STEREO_STREAM_ID(0), &streamSettings);
    streamSettings.resolution = 24;
    streamSettings.input = fciGroup.ids[0];
    (void)BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, GET_MAI_STEREO_STREAM_ID(0), &streamSettings);
    /* Setup HBR data path as mixer output 1 */
    BAPE_MixerGroup_P_GetOutputFciIds(handle->hMixer, 1, &fciGroup);
    BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, GET_MAI_HBR_STREAM_ID(0), &streamSettings);
    streamSettings.resolution = 24;
    streamSettings.input = fciGroup.ids[0];
    (void)BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, GET_MAI_HBR_STREAM_ID(0), &streamSettings);
    /* Enable stereo data path consumer..producer */
    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_PAYLOAD_SEL, Stereo);
    BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, HBR_ENABLE, 0);
    BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_OP_CTRL_STREAM_ROUTE, HBR_HW_CH_STATUS_EN, 0);
    BAPE_Reg_P_ApplyFieldList(&regFieldList, BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE);
    BKNI_EnterCriticalSection();
    BAPE_MaiOutput_P_SetCbits_Legacy_isr(handle);
    BKNI_LeaveCriticalSection();
    BAPE_Reg_P_Write(handle->deviceHandle, BCHP_AUD_FMM_OP_CTRL_ENABLE_SET, 1<<GET_MAI_STEREO_STREAM_ID(0));
    /* Enable data source and pauseburst ringbuffer */
    (void)BAPE_MixerGroup_P_StartOutput(handle->hMixer, 0);
    (void)BAPE_MixerGroup_P_StartInput(handle->hMixer, 1);  /* Input for pauseburst */
    (void)BAPE_SfifoGroup_P_Start(handle->hSfifo, false);   /* Sfifo for pauseburst */
    /* Save current state */
    handle->dataPath = BAPE_MaiOutputDataPath_eStereo;

    return errCode;
}

/**************************************************************************/

static void BAPE_MaiOutput_P_CloseHw_Legacy(BAPE_MaiOutputHandle handle)
{
    if ( handle->dataPath != BAPE_MaiOutputDataPath_eNone )
    {
        /* Stop stream 1 path */
        BAPE_SfifoGroup_P_Stop(handle->hSfifo);
        BAPE_MixerGroup_P_StopInput(handle->hMixer, 1);
        BAPE_MixerGroup_P_StopOutput(handle->hMixer, 0);
        BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR, 1<<GET_MAI_STEREO_STREAM_ID(handle->index));
        /* If HBR, stop the HBR output path */
        if ( handle->dataPath == BAPE_MaiOutputDataPath_eHbr )
        {
            BAPE_MixerGroup_P_StopOutput(handle->hMixer, 1);
            BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR, 1<<GET_MAI_HBR_STREAM_ID(handle->index));
        }
        handle->dataPath = BAPE_MaiOutputDataPath_eNone;
    }

    /* Disable microsequencer */
    BKNI_EnterCriticalSection();
    {
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA, AUD_FMM_MS_CTRL_STRM_ENA, STREAM1_ENA, Disable);
    }
    BKNI_LeaveCriticalSection();

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
}

static void BAPE_MaiOutput_P_SetCrossbar_Legacy(BAPE_MaiOutputHandle handle, BAPE_StereoMode stereoMode)
{
    uint32_t regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    /* Setup the MAI output crossbar to properly assign the channel order for HDMI. */
    /* HDMI expects L R LFE C Ls Rs Lr Rr, we output L R Ls Rs C LFE Lr Rr */
    regVal = BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MAI_CROSSBAR);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT0)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT1)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT2)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT3)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT4)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT5)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT6)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT7));
    switch ( stereoMode )
    {
    default:
    case BAPE_StereoMode_eLeftRight:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT0, In0_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT1, In0_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT2, In1_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT3, In1_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT4, In2_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT5, In2_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT6, In3_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT7, In3_r);
        break;
    case BAPE_StereoMode_eLeftLeft:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT0, In0_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT1, In0_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT2, In1_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT3, In1_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT4, In2_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT5, In2_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT6, In3_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT7, In3_l);
        break;
    case BAPE_StereoMode_eRightRight:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT0, In0_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT1, In0_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT2, In1_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT3, In1_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT4, In2_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT5, In2_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT6, In3_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT7, In3_r);
        break;
    case BAPE_StereoMode_eRightLeft:
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT0, In0_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT1, In0_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT2, In1_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT3, In1_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT4, In2_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT5, In2_l);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT6, In3_r);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT7, In3_l);
        break;
    }
    BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MAI_CROSSBAR, regVal);
}

static BERR_Code BAPE_MaiOutput_P_SetBurstConfig_Legacy(BAPE_MaiOutputHandle handle)
{
    unsigned i;
    uint16_t *pCached;

    if (handle->hSfifo)
    {
        BAPE_SfifoGroup_P_Stop(handle->hSfifo); /* stop the fifo for mute buffer to clear it */
    }

    pCached = (uint16_t*)handle->pMuteBuffer;
    BDBG_MSG(("filling Burst RBUF addr %p/%p, size %u with underflowBurst=%d(%s)",
        (void *)handle->pMuteBuffer, (void *)pCached, BAPE_P_MUTE_BUFFER_SIZE, handle->settings.underflowBurst,
        (handle->settings.underflowBurst==BAPE_SpdifBurstType_ePause)?"Pause Bursts" : (handle->settings.underflowBurst==BAPE_SpdifBurstType_eNull)?"NULL Bursts" : "Zeros"));

    BKNI_Memset( pCached, 0, BAPE_P_MUTE_BUFFER_SIZE );

    if ( handle->settings.underflowBurst == BAPE_SpdifBurstType_ePause )
    {
        pCached[0] = g_pauseburst[0];
        pCached[1] = g_pauseburst[1];
        pCached[2] = g_pauseburst[2];
        pCached[3] = g_pauseburst[3]; /* size of burst */
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

    BAPE_FLUSHCACHE_ISRSAFE(handle->muteBufferBlock, handle->pMuteBuffer, BAPE_P_MUTE_BUFFER_SIZE);

    if (handle->hSfifo)
    {
        (void)BAPE_SfifoGroup_P_Start(handle->hSfifo, false);   /* Sfifo for pauseburst */
    }

    return BERR_SUCCESS;
}
#endif

/***************************************************************************
    Define stub functions for when there are no I2S outputs.
***************************************************************************/
#else
/* No MAI output interface.  Use Stubs. */

/**************************************************************************/

void BAPE_MaiOutput_GetDefaultSettings(
    BAPE_MaiOutputSettings *pSettings
    )
{
    BSTD_UNUSED(pSettings);
}

/**************************************************************************/

BERR_Code BAPE_MaiOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_MaiOutputSettings *pSettings,
    BAPE_MaiOutputHandle *pHandle             /* [out] */
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pHandle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/**************************************************************************/

void BAPE_MaiOutput_Close(
    BAPE_MaiOutputHandle handle
    )
{
    BSTD_UNUSED(handle);
}

/**************************************************************************/

void BAPE_MaiOutput_GetSettings(
    BAPE_MaiOutputHandle handle,
    BAPE_MaiOutputSettings *pSettings     /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

/**************************************************************************/

BERR_Code BAPE_MaiOutput_SetSettings(
    BAPE_MaiOutputHandle handle,
    const BAPE_MaiOutputSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/**************************************************************************/

void BAPE_MaiOutput_GetOutputPort(
    BAPE_MaiOutputHandle handle,
    BAPE_OutputPort *pOutputPort        /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pOutputPort);
}

/**************************************************************************/

void BAPE_MaiOutput_GetInterruptHandlers(
    BAPE_MaiOutputHandle handle,
    BAPE_MaiOutputInterruptHandlers *pInterrupts    /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pInterrupts);
}

/**************************************************************************/

BERR_Code BAPE_MaiOutput_SetInterruptHandlers(
    BAPE_MaiOutputHandle handle,
    const BAPE_MaiOutputInterruptHandlers *pInterrupts
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pInterrupts);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/**************************************************************************/


BERR_Code BAPE_MaiOutput_P_PrepareForStandby(
    BAPE_Handle bapeHandle
    )
{
    BSTD_UNUSED(bapeHandle);
    return BERR_SUCCESS;
}

BERR_Code BAPE_MaiOutput_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BSTD_UNUSED(bapeHandle);
    return BERR_SUCCESS;
}

void BAPE_MaiOutput_P_DeterminePauseBurstEnabled(
    BAPE_MaiOutputHandle handle,
    bool *compressed,
    bool *burstsEnabled)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(compressed);
    BSTD_UNUSED(burstsEnabled);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
}

#endif
