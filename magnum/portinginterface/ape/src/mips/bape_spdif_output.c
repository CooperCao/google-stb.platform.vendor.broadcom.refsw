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

BDBG_MODULE(bape_spdif_output);

BDBG_OBJECT_ID(BAPE_SpdifOutput);

typedef struct BAPE_SpdifOutput
{
    BDBG_OBJECT(BAPE_SpdifOutput)
    BAPE_Handle deviceHandle;
    BAPE_SpdifOutputSettings settings;
    unsigned index;
    BAPE_MixerOutputObject connector;
    uint32_t offset;
    BAVC_AudioSamplingRate sampleRate;
    bool enabled;
    char name[9];   /* SPDIF %d */
} BAPE_SpdifOutput;

static void BAPE_SpdifOutput_P_SetTimingParams_isr(void *pHandle, BAVC_AudioSamplingRate sampleRate, BAVC_Timebase timebase);
static void BAPE_SpdifOutput_P_SetMute(void *pHandle, bool muted, bool sync);
static void BAPE_SpdifOutput_P_Enable(void *pHandle);
static void BAPE_SpdifOutput_P_Disable(void *pHandle);
static void BAPE_SpdifOutput_P_SetCbits_isr(BAPE_SpdifOutputHandle handle);
static void BAPE_SpdifOutput_P_SetMclk_isr(void *pHandle, BAPE_MclkSource mclkSource, unsigned multiplier);

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
    uint32_t regVal, regAddr;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);
    
    *pHandle = NULL;

    if ( index >= BAPE_CHIP_MAX_SPDIF_OUTPUTS )
    {
        BDBG_ERR(("Request to open SPDIF %d but chip only has %u SPDIF outputs", index, BAPE_CHIP_MAX_SPDIF_OUTPUTS));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->spdifOutputs[index] )
    {
        BDBG_ERR(("SPDIF output %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle = BKNI_Malloc(sizeof(BAPE_SpdifOutput));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_SpdifOutput));
    BDBG_OBJECT_SET(handle, BAPE_SpdifOutput);
    BAPE_P_InitMixerOutput(&handle->connector, handle);
    handle->deviceHandle = deviceHandle;
    handle->index = index;
    handle->connector.maxChannelPairs = 1;  /* SpdifOutput is stero or compressed only */
    handle->connector.compressedSupported = true;
    handle->connector.pllRequired = true;
    handle->connector.setTimingParams_isr = BAPE_SpdifOutput_P_SetTimingParams_isr;
    handle->connector.setMute = BAPE_SpdifOutput_P_SetMute;
    handle->connector.streamId[BAPE_ChannelPair_eLeftRight] = 0;
    handle->connector.enable = BAPE_SpdifOutput_P_Enable;
    handle->connector.disable = BAPE_SpdifOutput_P_Disable;
    handle->connector.setMclk_isr = BAPE_SpdifOutput_P_SetMclk_isr;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "SPDIF %u", index);
    handle->connector.pName = handle->name;
    /* Currently we only support one SPDIF, this can be expanded later if we support more */
    handle->offset = 0;
    handle->sampleRate = BAVC_AudioSamplingRate_e48k;

    BDBG_ASSERT(handle->offset == 0);

    /* Init to specified settings */
    if ( NULL == pSettings )
    {
        BAPE_SpdifOutput_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* Enable the clock and data while opening the output port. Never disable it */
    regAddr = BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 + handle->offset;
    regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0, CLOCK_ENABLE)|
                BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0,DATA_ENABLE));
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_SPDIF_CFG_0, CLOCK_ENABLE, Enable);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_SPDIF_CFG_0, DATA_ENABLE, Enable);
    BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

    /* mute */
    BAPE_SpdifOutput_P_SetMute(handle, true, false);

    /* Initialize hardware before applying settings */
    BKNI_EnterCriticalSection();
    BAPE_SpdifOutput_P_SetMclk_isr(handle, BAPE_MclkSource_ePll0, 1);
    BKNI_LeaveCriticalSection();    

    errCode = BAPE_SpdifOutput_SetSettings(handle, pSettings);
    if ( errCode )
    {
        BAPE_SpdifOutput_Close(handle);
        return BERR_TRACE(errCode);
    }

    *pHandle = handle;
    handle->deviceHandle->spdifOutputs[index] = handle;
    return BERR_SUCCESS;
}

void BAPE_SpdifOutput_Close(
    BAPE_SpdifOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    /* Make sure we're not still connected to anything */
    if ( handle->connector.mixer )
    {
        BDBG_ERR(("Cannot close spdif output %p (%d), still connected to mixer %p", handle, handle->index, handle->connector.mixer));
        BDBG_ASSERT(NULL == handle->connector.mixer);
        return;
    }

    handle->deviceHandle->spdifOutputs[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_SpdifOutput);
    BKNI_Free(handle);    
}

void BAPE_SpdifOutput_GetSettings(
    BAPE_SpdifOutputHandle handle,
    BAPE_SpdifOutputSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

BERR_Code BAPE_SpdifOutput_SetSettings(
    BAPE_SpdifOutputHandle handle,
    const BAPE_SpdifOutputSettings *pSettings
    )
{
    uint32_t regAddr, regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(NULL != pSettings);
    
    regAddr = BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 + handle->offset;
    regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0, LR_SELECT)|
                BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0, LIMIT_TO_16_BITS));
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_SPDIF_CFG_0, LR_SELECT, pSettings->stereoMode);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_SPDIF_CFG_0, LIMIT_TO_16_BITS, (pSettings->limitTo16Bits?1:0));
    BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
    
    handle->settings = *pSettings;
    /* Remaining fields are handled by MS regs.  Must modify those in critical section. */    
    BKNI_EnterCriticalSection();
    BAPE_SpdifOutput_P_SetCbits_isr(handle);
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

void BAPE_SpdifOutput_GetConnector(
    BAPE_SpdifOutputHandle handle,
    BAPE_MixerOutput *pConnector        /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->connector;
}

static void BAPE_SpdifOutput_P_SetTimingParams_isr(void *pHandle, BAVC_AudioSamplingRate sampleRate, BAVC_Timebase timebase)
{
    BAPE_SpdifOutputHandle handle = pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BSTD_UNUSED(timebase);  /* SPDIF doesn't care */

    handle->sampleRate = sampleRate;

    BAPE_SpdifOutput_P_SetCbits_isr(handle);
}

static void BAPE_SpdifOutput_P_SetMute(void *pHandle, bool muted, bool sync)
{
    BAPE_SpdifOutputHandle handle = pHandle;
    uint32_t regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    /* Only support one now */
    BDBG_ASSERT(handle->offset == 0);

    /* Must manipulate MS registers in critical section */
    BKNI_EnterCriticalSection();
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0);

    /* We always use FW channel status, so set override only to mute */
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, OVERWRITE_DATA);
    if ( muted )
    {
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, OVERWRITE_DATA, Enable);
    }
    else
    {
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, OVERWRITE_DATA, Disable);
    }
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, regVal);

    BKNI_LeaveCriticalSection();

    BSTD_UNUSED(sync);
}

static void BAPE_SpdifOutput_P_Enable(void *pHandle)
{
    BAPE_SpdifOutputHandle handle;
    uint32_t regVal;
    
    handle = pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(false == handle->enabled);

    BDBG_MSG(("Enabling %s", handle->name));

    /* Write the enable bit in the OP (only stereo) */
    BDBG_MSG(("Writing %x to enable set", (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(handle->connector.streamId[0])));
    BREG_Write32(handle->deviceHandle->regHandle, 
                 BCHP_AUD_FMM_OP_CTRL_ENABLE_SET,
                 (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(handle->connector.streamId[0]));

    BAPE_SpdifOutput_P_SetMute(pHandle, false, false);

    /* Enable this stream in the MS FW */
    BKNI_EnterCriticalSection();
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA, 1);
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, regVal);
    BKNI_LeaveCriticalSection();

    /* Enable microsequencer */
    regVal = BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_STRM_ENA, STREAM0_ENA);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_STRM_ENA, STREAM0_ENA, 1);
    BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA, regVal);


    handle->enabled = true;
}

static void BAPE_SpdifOutput_P_Disable(void *pHandle)
{
    BAPE_SpdifOutputHandle handle;
    uint32_t regVal;

    handle = pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    /*BDBG_ASSERT(true == handle->enabled);*/

    BDBG_MSG(("Disabling %s", handle->name));

    BAPE_SpdifOutput_P_SetMute(pHandle, true, true);

    /* Clear the enable bit in the OP */
    BDBG_MSG(("Writing %x to enable clear", (BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_SET, STREAM0_ENA))<<(handle->connector.streamId[0])));
    BREG_Write32(handle->deviceHandle->regHandle, 
                 BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR,
                 BCHP_MASK(AUD_FMM_OP_CTRL_ENABLE_CLEAR, STREAM0_ENA)<<handle->connector.streamId[0]);

    /* Disable microsequencer */
    regVal = BREG_Read32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_STRM_ENA, STREAM0_ENA);
    BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_STRM_ENA, regVal);

    /* Disable this stream in the MS FW */
    BKNI_EnterCriticalSection();
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA);
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, regVal);
    BKNI_LeaveCriticalSection();

    handle->enabled = false;
}

static void BAPE_SpdifOutput_P_SetCbits_isr(BAPE_SpdifOutputHandle handle)
{
    uint32_t regAddr, regVal, cbitAddr, cbitVal, cbitBufAddr;
    unsigned i;
    bool compressed;
    uint16_t cbits[3];

    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);

    compressed = (handle->connector.mixer && handle->connector.mixer->compressed)?true:false;

    regAddr = BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0;
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~( (BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, CHAN_OVERRIDE)) 
                |(BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, VALIDITY)) 
                |(BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, DITHER_ENA)) 
                |(BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, COMP_OR_LINEAR))
                |(BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, SPDIF_OR_PCM))
                |(BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, FLUSH_ON_UFLOW))
                |(BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, INSERT_ON_UFLOW))
                |(BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, INSERT_WHEN_DISA))      
                |(BCHP_MASK(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA))
                );

    regVal |= (BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, SPDIF_OR_PCM, 1));
    if ( handle->settings.channelStatus.separateLeftRight )
    {
        regVal |= (BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, CHAN_OVERRIDE, 1));
    }
    if ( compressed )
    {
        regVal |= (BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, VALIDITY, 1));
        regVal |= (BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, COMP_OR_LINEAR, 1));
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, DITHER_ENA, Disable);
    } 
    else
    {
		if (handle->settings.ditherEnabled)
		{
	        regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, DITHER_ENA, Enable);
		}
    }
    
    /* RAP programs channel status bits here. */
    if ( handle->settings.useRawChannelStatus )
    {
        cbits[0] = (uint16_t)handle->settings.rawChannelStatus[0] |
                (((uint16_t)handle->settings.rawChannelStatus[1])<<8);
        cbits[1] = ((uint32_t)handle->settings.rawChannelStatus[2]) |
            (((uint32_t)handle->settings.rawChannelStatus[3])<<8);
        cbits[2] = 0;
    }
    else if ( handle->settings.channelStatus.professional )
    {
        cbits[0] = 0xc000 | (BAPE_P_GetSampleRateCstatCode_isrsafe(handle->sampleRate)<<8);
        cbits[1] = cbits[2] = 0;
    }
    else
    {
        /* Refer to IEC 60958-3 (5.2.2) */
        cbits[0] = (((uint16_t)handle->settings.channelStatus.categoryCode)<<8) |
                   (((uint16_t)handle->settings.channelStatus.formatInformation)<<3);
        if ( !handle->settings.channelStatus.copyright )
        {
            cbits[0] |= 1<<2;   /* Copyright = 0, no copyright = 1 in bit 2 */
        }
        if ( compressed )
        {
            cbits[0] |= 1<<1;   /* Linear/PCM */
        }
        cbits[1] = (((uint16_t)handle->settings.channelStatus.clockAccuracy)<<12) |
                    (BAPE_P_GetSampleRateCstatCode_isrsafe(handle->sampleRate)<<8) |
                    ((uint16_t)handle->settings.channelStatus.sourceNumber) |
                    (((uint16_t)handle->settings.channelStatus.separateLeftRight)<<4);
        cbits[2] = (((uint16_t)handle->settings.channelStatus.cgmsA)<<8);
    }
    
    /* Load channel status bits into FW registers */
    cbitAddr = BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0;
    cbitVal = BREG_Read32_isr(handle->deviceHandle->regHandle, cbitAddr);
    if ( 0x50 == BCHP_GET_FIELD_DATA(cbitVal, AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0, OFFSET) )
    {
        /* Using ping buffer - Load pong */
        cbitBufAddr = BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 48;
    }
    else
    {
        /* Using pong buffer - Load ping */
        cbitBufAddr = BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE;
    }
    BREG_Write32_isr(handle->deviceHandle->regHandle, cbitBufAddr, cbits[0]);
    BREG_Write32_isr(handle->deviceHandle->regHandle, cbitBufAddr+4, cbits[1]);
    BREG_Write32_isr(handle->deviceHandle->regHandle, cbitBufAddr+8, cbits[2]);
    for ( i = 0; i < 9; i++ )
    {
        BREG_Write32_isr(handle->deviceHandle->regHandle, cbitBufAddr+12+(4*i), 0);
    }
    cbitVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0, OFFSET);
    if ( cbitBufAddr == BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE )
    {
        cbitVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0, OFFSET, 0x50);
    }
    else
    {
        cbitVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0, OFFSET, 0x5c);
    }
    BREG_Write32_isr(handle->deviceHandle->regHandle, cbitAddr, cbitVal);
                
    regVal |= (BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, INSERT_ON_UFLOW, 1));
    regVal |= (BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, FLUSH_ON_UFLOW, 1));
    regVal |= (BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, INSERT_WHEN_DISA, 1));

    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_FW_RAMP_AMOUNT_0, 0);

    /* Enable formatter */
    regVal |= (BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, STREAM_ENA, Enable));
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);

    /* Disable bypass */
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM0);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_MS_CTRL_USEQ_BYPASS, STREAM0, 0);
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS, regVal);

    return;
}

static void BAPE_SpdifOutput_P_SetMclk_isr(void *pHandle, BAPE_MclkSource mclkSource, unsigned multiplier)
{
    uint32_t regVal;

    BAPE_SpdifOutputHandle handle;

    handle = pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_SpdifOutput);
    BDBG_ASSERT(handle->offset == 0);

    BDBG_ASSERT(multiplier == 1 || multiplier == 2 || multiplier == 4);

    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, PLLCLKSEL)|
                BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, MCLK_RATE));
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, MCLK_RATE, multiplier);
    if ( mclkSource >= BAPE_MclkSource_ePll0 && mclkSource <= BAPE_MclkSource_ePll2 )
    {
        unsigned pll = mclkSource - BAPE_MclkSource_ePll0;
        BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);
        switch ( multiplier )
        {
        case 1:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, PLLCLKSEL, (3*pll));
            break;
        case 2:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, PLLCLKSEL, 1+(3*pll));
            break;
        case 4:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, PLLCLKSEL, 2+(3*pll));
            break;
        default:
            BDBG_ASSERT(false); /* Should never get here */
            break;
        }
    }
    else
    {
        /* TODO: Expand this if we support > 1 DAC */
        BDBG_ASSERT(mclkSource == BAPE_MclkSource_eHifidac0);
        regVal |= BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, PLLCLKSEL, Hifidac0);
        BDBG_WRN(("DAC timing source has been selected for SPDIF outupt %u.", handle->index));
        BDBG_WRN(("It is strongly recommended to place SPDIF and DAC outputs on separate mixers."));
    }
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, regVal);
}

