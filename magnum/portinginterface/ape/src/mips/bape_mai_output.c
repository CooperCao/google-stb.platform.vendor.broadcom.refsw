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
#include "bchp_aud_fmm_op_ctrl.h"

BDBG_MODULE(bape_mai_output);

BDBG_OBJECT_ID(BAPE_MaiOutput);

typedef struct BAPE_MaiOutput
{
    BDBG_OBJECT(BAPE_MaiOutput)
    BAPE_Handle deviceHandle;
    BAPE_MaiOutputSettings settings;
    unsigned index;
    BAPE_MixerOutputObject connector;
    uint32_t offset;
    BAVC_AudioSamplingRate sampleRate;
    bool enabled;
    char name[7];   /* MAI %d */
    BAPE_MaiOutputInterruptHandlers interrupts;
} BAPE_MaiOutput;

#if defined BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE_MAI_PAYLOAD_SEL_MAI_Multi
#define MAI_MULTI 1
#elif defined BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE_MAI_PAYLOAD_SEL_I2S_Multi0
#else
#error Must use either I2S multi or MAI multi
#endif

static void BAPE_MaiOutput_P_SetTimingParams_isr(void *pHandle, BAVC_AudioSamplingRate sampleRate, BAVC_Timebase timebase);
static void BAPE_MaiOutput_P_Enable(void *pHandle);
static void BAPE_MaiOutput_P_Disable(void *pHandle);
static void BAPE_MaiOutput_P_SetCbits_isr(BAPE_MaiOutputHandle handle);
static void BAPE_MaiOutput_P_SetMclk_isr(void *pHandle, BAPE_MclkSource mclkSource, unsigned multiplier);
static uint32_t BAPE_MaiOutput_P_SampleRateToMaiFormat(BAVC_AudioSamplingRate sampleRate);

void BAPE_MaiOutput_GetDefaultSettings(
    BAPE_MaiOutputSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->stereoMode = BAPE_StereoMode_eLeftRight;
    pSettings->ditherEnabled = true;
    pSettings->underflowBurst = BAPE_SpdifBurstType_ePause;
    pSettings->useRawChannelStatus = false;
    /* channel status is initialized to zero for all fields */
}

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
    uint32_t regVal;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);
    
    *pHandle = NULL;

    if ( index >= BAPE_CHIP_MAX_MAI_OUTPUTS )
    {
        BDBG_ERR(("Request to open MAI %d but chip only has %u SPDIF outputs", index, BAPE_CHIP_MAX_SPDIF_OUTPUTS));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->maiOutputs[index] )
    {
        BDBG_ERR(("SPDIF output %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle = BKNI_Malloc(sizeof(BAPE_MaiOutput));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_MaiOutput));
    BDBG_OBJECT_SET(handle, BAPE_MaiOutput);
    BAPE_P_InitMixerOutput(&handle->connector, handle);
    handle->deviceHandle = deviceHandle;
    handle->index = index;
    handle->connector.maxChannelPairs = 4;  /* MaiOutput is stero, compressed, 5.1, or 7.1 */
    handle->connector.compressedSupported = true;
    handle->connector.muteInMixer = true;
    handle->connector.setTimingParams_isr = BAPE_MaiOutput_P_SetTimingParams_isr;
    /* These are the same for MAI multi and I2S multi */
    handle->connector.streamId[BAPE_ChannelPair_eLeftRight] = 3;
    handle->connector.streamId[BAPE_ChannelPair_eLeftRightSurround] = 4;
    handle->connector.streamId[BAPE_ChannelPair_eCenterLfe] = 5;
    handle->connector.streamId[BAPE_ChannelPair_eLeftRightRear] = 6;
    /* MAI also needs to be active for STREAM1 to output Channel Status and allow for null/pause burst overwriting */
    handle->connector.dummyStreamId = 1;
    handle->connector.enable = BAPE_MaiOutput_P_Enable;
    handle->connector.disable = BAPE_MaiOutput_P_Disable;
    handle->connector.setMclk_isr = BAPE_MaiOutput_P_SetMclk_isr;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "MAI %u", index);
    handle->connector.pName = handle->name;
    /* Currently we only support one HDMI, this can be expanded later if we support more */
    handle->offset = 0;
    handle->sampleRate = BAVC_AudioSamplingRate_e48k;

    BDBG_ASSERT(handle->offset == 0);

    /* Init to specified settings */
    if ( NULL == pSettings )
    {
        BAPE_MaiOutput_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* Always source MAI timing from I2S Multi or MAI Multi.  CSTAT comes from the second formatter.  */
    /* In Stereo/Compressed, data comes from MAI - Multichannel sources from I2S/MAI Multi */
    regVal = BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_STREAM_ROUTE, STRM1_TMG_SRC_SEL)|
                BCHP_MASK(AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_PAYLOAD_SEL)|
                BCHP_MASK(AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_CSTAT_SEL));
    #ifdef BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE_HBR_ENABLE_MASK
    regVal &= ~BCHP_MASK(AUD_FMM_OP_CTRL_STREAM_ROUTE, HBR_ENABLE);
    #endif
    #if MAI_MULTI
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_STREAM_ROUTE, STRM1_TMG_SRC_SEL, MAI_Multi);
    #else
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_STREAM_ROUTE, STRM1_TMG_SRC_SEL, I2S_Multi0);
    #endif
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_PAYLOAD_SEL, Stereo);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_CSTAT_SEL, STRM1);
    BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE, regVal);

    /* Setup the MAI output crossbar to properly assign the channel order for HDMI. */
    /* HDMI expects L R LFE C Ls Rs Lr Rr, we output L R Ls Rs C LFE Lr Rr */
    #if MAI_MULTI
    regVal = BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MAI_CROSSBAR);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT2)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT3)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT4)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT5));
    regVal |= (BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT2, In2_r)|
               BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT3, In2_l)|
               BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT4, In1_l)|
               BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CROSSBAR, OUT5, In1_r));
    BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MAI_CROSSBAR, regVal);
    #else
    regVal = BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_I2SM0_CROSSBAR);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2SM0_CROSSBAR, OUT1_L)|
                BCHP_MASK(AUD_FMM_OP_CTRL_I2SM0_CROSSBAR, OUT1_R)|
                BCHP_MASK(AUD_FMM_OP_CTRL_I2SM0_CROSSBAR, OUT2_L)|
                BCHP_MASK(AUD_FMM_OP_CTRL_I2SM0_CROSSBAR, OUT2_R));
    regVal |= (BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM0_CROSSBAR, OUT1_L, In2_r)|
               BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM0_CROSSBAR, OUT1_R, In2_l)|
               BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM0_CROSSBAR, OUT2_L, In1_l)|
               BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2SM0_CROSSBAR, OUT2_R, In1_r));
    BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_I2SM0_CROSSBAR, regVal);
    #endif

    /* Initialize hardware before applying settings */
    BKNI_EnterCriticalSection();
    BAPE_MaiOutput_P_SetMclk_isr(handle, BAPE_MclkSource_ePll0, 1);
    BKNI_LeaveCriticalSection();    

    errCode = BAPE_MaiOutput_SetSettings(handle, pSettings);
    if ( errCode )
    {
        BAPE_MaiOutput_Close(handle);
        return BERR_TRACE(errCode);
    }

    *pHandle = handle;
    handle->deviceHandle->maiOutputs[index] = handle;
    return BERR_SUCCESS;
}

void BAPE_MaiOutput_Close(
    BAPE_MaiOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    /* Make sure we're not still connected to anything */
    if ( handle->connector.mixer )
    {
        BDBG_ERR(("Cannot close spdif output %p (%d), still connected to mixer %p", handle, handle->index, handle->connector.mixer));
        BDBG_ASSERT(NULL == handle->connector.mixer);
        return;
    }

    handle->deviceHandle->maiOutputs[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_MaiOutput);
    BKNI_Free(handle);    
}

void BAPE_MaiOutput_GetSettings(
    BAPE_MaiOutputHandle handle,
    BAPE_MaiOutputSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

BERR_Code BAPE_MaiOutput_SetSettings(
    BAPE_MaiOutputHandle handle,
    const BAPE_MaiOutputSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(NULL != pSettings);
    
    handle->settings = *pSettings;

    /* Remaining fields are handled by MS regs.  Must modify those in critical section. */    
    BKNI_EnterCriticalSection();
    BAPE_MaiOutput_P_SetCbits_isr(handle);
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

void BAPE_MaiOutput_GetConnector(
    BAPE_MaiOutputHandle handle,
    BAPE_MixerOutput *pConnector        /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->connector;
}

static void BAPE_MaiOutput_P_SetTimingParams_isr(void *pHandle, BAVC_AudioSamplingRate sampleRate, BAVC_Timebase timebase)
{
    BAPE_MaiOutputHandle handle = pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BSTD_UNUSED(timebase);  /* SPDIF doesn't care */

    handle->sampleRate = sampleRate;

    BAPE_MaiOutput_P_SetCbits_isr(handle);

    if ( handle->interrupts.sampleRate.pCallback_isr )
    {
        BDBG_MSG(("Sending sample rate callback - new rate %s", BAPE_P_GetSampleRateString_isrsafe(sampleRate)));
        handle->interrupts.sampleRate.pCallback_isr(handle->interrupts.sampleRate.pParam1, handle->interrupts.sampleRate.param2, sampleRate);
    }
}

static void BAPE_MaiOutput_P_Enable(void *pHandle)
{
    BAPE_MaiOutputHandle handle;
    BAPE_MixerHandle mixer;
    BREG_Handle regHandle;
    uint32_t regVal, regAddr;
    unsigned i;
    
    handle = pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(false == handle->enabled);
    BDBG_ASSERT(NULL != handle->connector.mixer);

    mixer = handle->connector.mixer;
    regHandle = handle->deviceHandle->regHandle;

    BDBG_MSG(("Enabling %s", handle->name));

    BDBG_ASSERT(mixer->numChannelPairs <= 4);

    /* Set MAI/I2S Multi Grouping based on number of channel pairs */
    #if MAI_MULTI
    regAddr = BCHP_AUD_FMM_OP_CTRL_MAI_MULTI_GROUPING;
    #else
    regAddr = BCHP_AUD_FMM_OP_CTRL_I2SM0_GROUPING;
    #endif
    #if 0 /* This grouping does not seem to be stable in HW.  If programmed, once in a handful of tries the HW doesn't start */
    /* We need to write 1's in the fields that are multichannel.  For all stereo, write 0 */
    if ( mixer->numChannelPairs == 1 )
    {
        BREG_Write32(regHandle, regAddr, 0);
    }
    else
    {
        BREG_Write32(regHandle, regAddr, 0xf>>(4-mixer->numChannelPairs));
    }
    #else
    BREG_Write32(regHandle, regAddr, 0);
    #endif

    /* Setup correct routing */
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE);
    regVal &= ~BCHP_MASK(AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_PAYLOAD_SEL);
    if ( mixer->numChannelPairs == 1 )
    {
        /* For stereo/compressed, fetch data from the MAI path */
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_PAYLOAD_SEL, Stereo);
    }
    else
    {
        /* For multichannel PCM, fetch data from the I2S/MAI multi path */
        #if MAI_MULTI
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_PAYLOAD_SEL, MAI_Multi);
        #else
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_PAYLOAD_SEL, I2S_Multi0);
        #endif
    }
    BREG_Write32(regHandle, BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE, regVal);

    /* Enable the MAI bus */
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_OP_CTRL_MAI_CFG);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CFG, SPDIF_MODE)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CFG, ENABLE_MAI));
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CFG, SPDIF_MODE, SPDIF_Format);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CFG, ENABLE_MAI, Enable);
    BREG_Write32(regHandle, BCHP_AUD_FMM_OP_CTRL_MAI_CFG, regVal);


    /* Enable MAI stream to activate MS/CSTAT generation */
    regVal = BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM1_ENA, 1);

    /* Enable MAI/I2S Multi for the proper number of channels */
    for ( i = 0; i < mixer->numChannelPairs; i++ )
    {
        BDBG_MSG(("Writing %x to enable set", (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(handle->connector.streamId[i])));
        regVal |= (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(handle->connector.streamId[i]);
    }

    /* Enable both MAI and I2S/MAI Multi simultaneously to make sure they're synchronized. */
    BREG_Write32(regHandle, BCHP_AUD_FMM_OP_CTRL_ENABLE_SET, regVal);

    /* Enable the formatter using HW channel status */
    BKNI_EnterCriticalSection();
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1);

    /* We always use HW channel status */
    regVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, OVERWRITE_DATA)|
                BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, STREAM_ENA));
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, OVERWRITE_DATA, Disable);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, STREAM_ENA, Enable);
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, regVal);
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM1);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM1, Bypass);
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS, regVal);
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, ENABLE);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, ENABLE, Enable);
    BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, regVal);
    BKNI_LeaveCriticalSection();

    /* Enable microsequencer */
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_STRM_ENA, STREAM1_ENA);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_STRM_ENA, STREAM1_ENA, 1);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA, regVal);

    handle->enabled = true;
}

static void BAPE_MaiOutput_P_Disable(void *pHandle)
{
    BAPE_MaiOutputHandle handle;
    BAPE_MixerHandle mixer;
    BREG_Handle regHandle;
    uint32_t regVal;
    unsigned i;

    handle = pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    /*BDBG_ASSERT(true == handle->enabled);*/
    BDBG_ASSERT(NULL != handle->connector.mixer);

    mixer = handle->connector.mixer;
    regHandle = handle->deviceHandle->regHandle;

    BDBG_MSG(("Disabling %s", handle->name));

    /* Disable the MAI bus */
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_OP_CTRL_MAI_CFG);
    regVal &= ~BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CFG, ENABLE_MAI);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_CFG, ENABLE_MAI, Disable);
    BREG_Write32(regHandle, BCHP_AUD_FMM_OP_CTRL_MAI_CFG, regVal);

    /* Disable the MAI and MAI/I2S Multi streams */
    regVal = BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_ENABLE_CLEAR, STREAM1_ENA, 1);
    for ( i = 0; i < mixer->numChannelPairs; i++ )
    {
        regVal |= (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_CLEAR, STREAM0_ENA))<<(handle->connector.streamId[i]);
    }
    BREG_Write32(regHandle, BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR, regVal);

    /* Revert to stereo mode */
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE);
    regVal &= ~BCHP_MASK(AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_PAYLOAD_SEL);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_STREAM_ROUTE, MAI_PAYLOAD_SEL, Stereo);
    BREG_Write32(regHandle, BCHP_AUD_FMM_OP_CTRL_STREAM_ROUTE, regVal);

    /* Disable microsequencer */
    BKNI_EnterCriticalSection();
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1);

    regVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, OVERWRITE_DATA)|
                BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, STREAM_ENA));
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, OVERWRITE_DATA, Enable);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, STREAM_ENA, Disable);
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, regVal);
    BKNI_LeaveCriticalSection();

    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_STRM_ENA, STREAM1_ENA);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA, regVal);

    handle->enabled = false;
}

static void BAPE_MaiOutput_P_SetCbits_isr(BAPE_MaiOutputHandle handle)
{
    uint32_t regAddr, regVal;
    bool compressed;
    unsigned validity;

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);

    compressed = (handle->connector.mixer && handle->connector.mixer->compressed)?true:false;

    BDBG_MSG(("Set MAI CBITS SR %u (%s)", handle->sampleRate, BAPE_P_GetSampleRateString_isrsafe(handle->sampleRate)));

    /* Only set validity if we're outputting compressed in a non-high-bitrate mode. */
    if ( compressed && handle->sampleRate != BAVC_AudioSamplingRate_e192k && handle->sampleRate != BAVC_AudioSamplingRate_e176_4k )
    {
        validity = 1;
    }
    else
    {
        validity = 0;
    }

    /* Program MAI format correctly */
    if ( handle->connector.mixer )
    {
        regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MAI_FORMAT);
        regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_FORMAT, AUDIO_FORMAT)|
                    BCHP_MASK(AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_WIDTH)|
                    BCHP_MASK(AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_RATE));
        if ( handle->connector.mixer->compressed )
        {
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_FORMAT, AUDIO_FORMAT, SPDIF_Comp_2_Channel);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_WIDTH, 16);    /* TODO: */
        }
        else
        {
            switch ( handle->connector.mixer->numChannelPairs )
            {
            case 3:
                regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_FORMAT, AUDIO_FORMAT, SPDIF_PCM_6_Channel);
                break;
            case 4:
                regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_FORMAT, AUDIO_FORMAT, SPDIF_PCM_8_Channel);
                break;
            default:
                regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MAI_FORMAT, AUDIO_FORMAT, SPDIF_PCM_2_Channel);
                break;
            }
            regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_WIDTH, 0); /* 32 bits per sample */
        }
        regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_RATE, 
                                  BAPE_MaiOutput_P_SampleRateToMaiFormat(handle->connector.mixer->sampleRate));
        BDBG_MSG(("Set MAI format to 0x%x", regVal));
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MAI_FORMAT, regVal);
    }

    /* Tell the HW to hold the current CBITS and set dither correctly */
    regAddr = BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1;
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, HOLD_CSTAT)|
                BCHP_MASK(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, DITHER_ENA)|
                BCHP_MASK(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, VALIDITY));
    if ( handle->settings.ditherEnabled && !compressed )
    {
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, DITHER_ENA, Enable);
    }
    else
    {
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, DITHER_ENA, Disable);
    }
    regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, VALIDITY, validity);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, HOLD_CSTAT, Hold);
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);

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
    else if ( handle->connector.mixer && handle->connector.mixer->compressed )
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
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_BITS_31_to_30));
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_PRO_CONS, handle->settings.channelStatus.professional);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_COMP_LIN, 1);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_CP, (handle->settings.channelStatus.copyright)?0:1);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_CATEGORY, handle->settings.channelStatus.categoryCode);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_SOURCE, handle->settings.channelStatus.sourceNumber);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, Compressed_FREQ, BAPE_P_GetSampleRateCstatCode_isrsafe(handle->sampleRate));
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
                    BCHP_MASK(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_BITS_31_to_30));
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_PRO_CONS, handle->settings.channelStatus.professional);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_CP, (handle->settings.channelStatus.copyright)?0:1);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_CATEGORY, handle->settings.channelStatus.categoryCode);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_SOURCE, handle->settings.channelStatus.sourceNumber);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1, PCM_FREQ, BAPE_P_GetSampleRateCstatCode_isrsafe(handle->sampleRate));
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

    regAddr = BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1;
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, HOLD_CSTAT);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1, HOLD_CSTAT, Update);
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);

    return;
}

static void BAPE_MaiOutput_P_SetMclk_isr(void *pHandle, BAPE_MclkSource mclkSource, unsigned multiplier)
{
    uint32_t regVal, pllclksel=0;
    uint32_t rate;

    BAPE_MaiOutputHandle handle;

    handle = pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiOutput);
    BDBG_ASSERT(handle->offset == 0);

    if ( mclkSource >= BAPE_MclkSource_ePll0 && mclkSource <= BAPE_MclkSource_ePll2 )
    {
        unsigned pll = mclkSource - BAPE_MclkSource_ePll0;

        BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);

        switch ( multiplier )
        {
        case 1:
            pllclksel = 3*pll;
            break;
        case 2:
            pllclksel = 1+(3*pll);
            break;
        case 4:
            pllclksel = 2+(3*pll);
            break;
        default:
            BDBG_ASSERT(multiplier == 1 || multiplier == 2 || multiplier == 4);
            return;
        }
        rate = 1;	/* Always use 128fs for PLL */
    }
    else
    {
        unsigned dac;

        /* DAC */
        dac = mclkSource - BAPE_MclkSource_eHifidac0;
        /* TODO: Expand below when more DACs are available */
        BDBG_ASSERT(dac == 0);
#if MAI_MULTI
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI_PLLCLKSEL_Hifidac0;
#else
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0_PLLCLKSEL_Hifidac0;
#endif
        rate = multiplier;
    }

#if MAI_MULTI
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI, PLLCLKSEL)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI, MCLK_RATE));
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI, MCLK_RATE, rate);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI, PLLCLKSEL, pllclksel);
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI, regVal);
#else
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0, PLLCLKSEL)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0, MCLK_RATE));
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0, MCLK_RATE, rate);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0, PLLCLKSEL, pllclksel);
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0, regVal);
#endif
}

static uint32_t BAPE_MaiOutput_P_SampleRateToMaiFormat(BAVC_AudioSamplingRate sampleRate)
{
    switch ( sampleRate )
    {
    case BAVC_AudioSamplingRate_e32k:    /* 32K Sample rate */
        return 7;
    case BAVC_AudioSamplingRate_e44_1k:    /* 44.1K Sample rate */
        return 8;
    case BAVC_AudioSamplingRate_e48k:      /* 48K Sample rate */
        return 9;
    case BAVC_AudioSamplingRate_e96k:      /* 96K Sample rate */
        return 12;
    case BAVC_AudioSamplingRate_e128k:     /* 128K Sample rate */
        return 13;
    case BAVC_AudioSamplingRate_e176_4k:   /* 176.4K Sample rate */
        return 14;
    case BAVC_AudioSamplingRate_e192k:     /* 192K Sample rate */
        return 15;
    default:
        BDBG_ASSERT(false); /* Should never get here */
        return 0;
    }
}

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

