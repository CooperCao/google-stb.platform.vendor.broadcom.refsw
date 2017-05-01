/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include "atlas.h"
#include "platform.h"
#include "nexus_platform.h"
#include "convert.h"

BDBG_MODULE(atlas_platform);

CPlatform::CPlatform() :
    _bInitialized(false),
    _bMemSettingsValid(false)
{
    memset(&_platformSettings, 0, sizeof(NEXUS_PlatformSettings));
    memset(&_platformConfig, 0, sizeof(NEXUS_PlatformConfiguration));
    memset(&_memSettings, 0, sizeof(NEXUS_MemoryConfigurationSettings));
#if HAS_VID_NL_LUMA_RANGE_ADJ
    for (int i = 0; i < (eWindowType_Max - eWindowType_Mosaic1); i++)
    {
        _lumaRangeAdj[i] = 0;
    }
#endif
}

CPlatform::~CPlatform()
{
}

void CPlatform::initialize()
{
    NEXUS_Platform_GetSettings(&_platformSettings);

    NEXUS_Platform_GetConfiguration(&_platformConfig);
#ifdef NXCLIENT_SUPPORT
    _bMemSettingsValid = false;
#else
    NEXUS_GetDefaultMemoryConfigurationSettings(&_memSettings);

    _bMemSettingsValid = (NEXUS_VideoFormat_eUnknown != _memSettings.display[0].maxFormat);
#endif /* ifdef NXCLIENT_SUPPORT */
    _bInitialized = true;
}

NEXUS_PlatformSettings * CPlatform::getPlatformSettings()
{
    NEXUS_Platform_GetSettings(&_platformSettings);
    return(&_platformSettings);
}

bool CPlatform::isSupportedDisplay(uint16_t number)
{
    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_HAS_DISPLAY
#if NEXUS_MAX_DISPLAYS
    BDBG_ASSERT(number < NEXUS_MAX_DISPLAYS);
    return(_platformConfig.supportedDisplay[number]);

#endif
#endif /* if NEXUS_HAS_DISPLAY */
    return(NULL);
}

bool CPlatform::isSupportedVideoWindow(
        uint16_t numDisplay,
        uint16_t number
        )
{
    bool isSupported = false;

    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_HAS_DISPLAY
#if NEXUS_MAX_DISPLAYS
    BDBG_ASSERT(numDisplay < NEXUS_MAX_DISPLAYS);
    isSupported = _platformConfig.supportedDisplay[numDisplay];

    if (true == _bMemSettingsValid)
    {
        isSupported &= _memSettings.display[numDisplay].window[number].used;
    }
#endif /* if NEXUS_MAX_DISPLAYS */
#endif /* if NEXUS_HAS_DISPLAY */

    return(isSupported);
} /* isSupportedVideoWindow */

bool CPlatform::isSupportedDecoder(uint16_t number)
{
    NEXUS_Error          nerror      = NEXUS_SUCCESS;
    eRet                 ret         = eRet_Ok;
    bool                 isSupported = false;
    NEXUS_PlatformStatus status;

    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

    nerror = NEXUS_Platform_GetStatus(&status);
    CHECK_NEXUS_ERROR_GOTO("unable to get nexus platform status", ret, nerror, error);

#if NEXUS_HAS_VIDEO_DECODER
#if NEXUS_MAX_VIDEO_DECODERS
    BDBG_ASSERT(number < NEXUS_MAX_VIDEO_DECODERS);

    /* 97445 box mode 15 - video decoder 1 is linked to video decoder 0
     * and should not be used.  unfortunately there is no programmatic
     * way to detect this right now. */
    if ((0x7445 == status.chipId) &&
        (15 == status.boxMode) &&
        (1 == number))
    {
        isSupported = false;
        goto done;
    }

    isSupported = _platformConfig.supportedDecoder[number];

    if (true == _bMemSettingsValid)
    {
        isSupported &= _memSettings.videoDecoder[number].used;
    }
#endif /* if NEXUS_MAX_VIDEO_DECODERS */
#endif /* if NEXUS_HAS_VIDEO_DECODER */

error:
done:
    return(isSupported);
} /* isSupportedDecoder */

bool CPlatform::isSupportedStillDecoder(uint16_t number)
{
    bool isSupported = false;

    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if (NEXUS_NUM_STILL_DECODES > 0)
    BDBG_ASSERT(number < NEXUS_NUM_STILL_DECODES);

    isSupported = true;

    if (true == _bMemSettingsValid)
    {
        isSupported &= _memSettings.stillDecoder[number].used;
    }
#endif /* if (NEXUS_NUM_STILL_DECODES > 0) */

    return(isSupported);
} /* isSupportedStillDecoder */

bool CPlatform::isSupportedEncoder(uint16_t number)
{
    bool isSupported = false;

    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_HAS_VIDEO_ENCODER
#if NEXUS_MAX_VIDEO_ENCODERS
    BDBG_ASSERT(number < NEXUS_MAX_VIDEO_ENCODERS);
    isSupported = true;

    if (true == _bMemSettingsValid)
    {
        isSupported &= _memSettings.videoEncoder[number].used;
    }
#endif /* if NEXUS_MAX_VIDEO_ENCODERS */
#endif /* if NEXUS_HAS_VIDEO_ENCODER */

    return(isSupported);
} /* isSupportedEncoder */

NEXUS_VideoFormat CPlatform::getDisplayMaxVideoFormat(int numDisplay)
{
    NEXUS_VideoFormat maxFormat = NEXUS_VideoFormat_eUnknown;

    if (true == _bMemSettingsValid)
    {
        maxFormat = _memSettings.display[numDisplay].maxFormat;
    }
    else
    {
        int format = 0;
        NEXUS_DisplayCapabilities capabilities;

        NEXUS_GetDisplayCapabilities(&capabilities);

        for (format = 0; format < NEXUS_VideoFormat_eMax; format++)
        {
            if (true == capabilities.displayFormatSupported[format])
            {
                if (videoFormatToVertRes(maxFormat).toInt() < videoFormatToVertRes((NEXUS_VideoFormat)format).toInt())
                {   /* Simple fix hard-codes SD display 1 to fix 7125 and any other 65nm platform that doesn't have
                     * _bMemSettingsValid.  Can be removed when those platforms are no longer supported in Nexus. */
                    if ((numDisplay != 1) || (videoFormatToVertRes((NEXUS_VideoFormat)format).toInt() <= 576))
                    {
                        maxFormat = (NEXUS_VideoFormat)format;
                    }
                }
            }
        }
    }

    return(maxFormat);
} /* getDisplayMaxVideoFormat */

NEXUS_VideoFormat CPlatform::getDecoderMaxVideoFormat(int numDecoder)
{
    NEXUS_VideoFormat maxFormat = NEXUS_VideoFormat_eUnknown;

    if (true == _bMemSettingsValid)
    {
        maxFormat = _memSettings.videoDecoder[numDecoder].maxFormat;
    }
    else
    {
        NEXUS_VideoDecoderCapabilities capabilities;

        NEXUS_GetVideoDecoderCapabilities(&capabilities);
        maxFormat = capabilities.memory[numDecoder].maxFormat;
    }

    return(maxFormat);
} /* getDecoderMaxVideoFormat */

uint32_t CPlatform::getNumWindowsPerDisplay()
{
    BDBG_ASSERT(isInitialized());

    return(_platformConfig.numWindowsPerDisplay);
}

NEXUS_HdmiOutputHandle CPlatform::getOutputHdmi(uint16_t number)
{
    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_HAS_DISPLAY
#if NEXUS_NUM_HDMI_OUTPUTS
    BDBG_ASSERT(number < NEXUS_NUM_HDMI_OUTPUTS);
    return(_platformConfig.outputs.hdmi[number]);

#endif
#endif /* if NEXUS_HAS_DISPLAY */
    return(NULL);
}

NEXUS_ComponentOutputHandle CPlatform::getOutputComponent(uint16_t number)
{
    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_HAS_DISPLAY
#if NEXUS_NUM_COMPONENT_OUTPUTS
    BDBG_ASSERT(number < NEXUS_NUM_COMPONENT_OUTPUTS);
    return(_platformConfig.outputs.component[number]);

#endif
#endif /* if NEXUS_HAS_DISPLAY */
    return(NULL);
}

NEXUS_SvideoOutputHandle CPlatform::getOutputSvideo(uint16_t number)
{
    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_HAS_DISPLAY
#if NEXUS_NUM_SVIDEO_OUTPUTS
    BDBG_ASSERT(number < NEXUS_NUM_SVIDEO_OUTPUTS);
    return(_platformConfig.outputs.svideo[number]);

#endif
#endif /* if NEXUS_HAS_DISPLAY */
    return(NULL);
}

NEXUS_CompositeOutputHandle CPlatform::getOutputComposite(uint16_t number)
{
    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_HAS_DISPLAY
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    BDBG_ASSERT(number < NEXUS_NUM_COMPOSITE_OUTPUTS);
    return(_platformConfig.outputs.composite[number]);

#endif
#endif /* if NEXUS_HAS_DISPLAY */
    return(NULL);
}

#if NEXUS_HAS_RFM
NEXUS_RfmHandle CPlatform::getOutputRFM(uint16_t number)
{
    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_HAS_DISPLAY
#if NEXUS_HAS_RFM
    BDBG_ASSERT(number < NEXUS_NUM_RFM_OUTPUTS);
    return(_platformConfig.outputs.rfm[number]);

#endif
#endif /* if NEXUS_HAS_DISPLAY */
    return(NULL);
}

#endif /* if NEXUS_HAS_RFM */

NEXUS_AudioDacHandle CPlatform::getOutputAudioDac(uint16_t number)
{
    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_HAS_AUDIO
#if NEXUS_NUM_AUDIO_DACS
    BDBG_ASSERT(number < NEXUS_NUM_AUDIO_DACS);
    return(_platformConfig.outputs.audioDacs[number]);

#endif
#endif /* if NEXUS_HAS_AUDIO */
    return(NULL);
}

NEXUS_I2sOutputHandle CPlatform::getOutputI2sDac(uint16_t number)
{
    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_NUM_I2S_OUTPUTS
    if (number < NEXUS_NUM_I2S_OUTPUTS)
    {
        return(_platformConfig.outputs.i2s[number]);
    }
#endif /* if NEXUS_NUM_I2S_OUTPUTS */
    return(NULL);
}

NEXUS_SpdifOutputHandle CPlatform::getOutputSpdif(uint16_t number)
{
    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_HAS_AUDIO
#if NEXUS_NUM_SPDIF_OUTPUTS
    BDBG_ASSERT(number < NEXUS_NUM_SPDIF_OUTPUTS);
    return(_platformConfig.outputs.spdif[number]);

#endif
#endif /* if NEXUS_HAS_AUDIO */
    return(NULL);
}

NEXUS_AudioDummyOutputHandle CPlatform::getOutputAudioDummy(uint16_t number)
{
    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_HAS_AUDIO
#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
    BDBG_ASSERT(number < NEXUS_NUM_AUDIO_DUMMY_OUTPUTS);
    return(_platformConfig.outputs.audioDummy[number]);

#endif
#endif /* if NEXUS_HAS_AUDIO */
    return(NULL);
}

NEXUS_FrontendHandle CPlatform::getFrontend(uint16_t number)
{
    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_HAS_FRONTEND
#if NEXUS_MAX_FRONTENDS
    BDBG_ASSERT(number < NEXUS_MAX_FRONTENDS);
    return(_platformConfig.frontend[number]);

#endif
#endif /* if NEXUS_HAS_FRONTEND */
    return(NULL);
}

#if HAS_VID_NL_LUMA_RANGE_ADJ
bool CPlatform::getPlmLumaRangeAdjVideo(unsigned inputIndex, unsigned rectIndex)
{
    uint32_t reg;
    unsigned lRangeAdj;
    unsigned rectDelta;

#if BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LRANGE_ADJ_MASK
    rectDelta = rectIndex;
#else
    rectDelta = (rectIndex>>1);
#endif
    NEXUS_Platform_ReadRegister(BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE + 4*rectDelta + (inputIndex*VID_NLCONFIG_INDEX_DELTA), &reg);
#if BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LRANGE_ADJ_MASK
    lRangeAdj = BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_LRANGE_ADJ);
#else
    lRangeAdj = rectIndex&1?BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT1_SEL_LRANGE_ADJ):BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_LRANGE_ADJ);
#endif

    return(lRangeAdj != LRANGE_ADJ_DISABLE);
}
#endif

#if HAS_VID_NL_LUMA_RANGE_ADJ
void CPlatform::setPlmLumaRangeAdjVideo(unsigned inputIndex, unsigned rectIndex, bool enable)
{
    uint32_t reg;
    unsigned lRangeAdj;
    unsigned curLRangeAdj;
    unsigned rectDelta;

#if BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LRANGE_ADJ_MASK
    rectDelta = rectIndex;
#else
    rectDelta = (rectIndex>>1);
#endif
    NEXUS_Platform_ReadRegister(BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE + 4*rectDelta + (inputIndex*VID_NLCONFIG_INDEX_DELTA), &reg);
#if BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LRANGE_ADJ_MASK
    curLRangeAdj = BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_LRANGE_ADJ);
#else
    curLRangeAdj = rectIndex&1?BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT1_SEL_LRANGE_ADJ):BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_LRANGE_ADJ);
#endif
    if(true == enable)
    {
        lRangeAdj = _lumaRangeAdj[rectIndex];
    }
    else
    {
        lRangeAdj = LRANGE_ADJ_DISABLE;
    }

    if(lRangeAdj != curLRangeAdj) {
#if BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LRANGE_ADJ_MASK
        BCHP_SET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_LRANGE_ADJ, lRangeAdj);
#else
        if(rectIndex&1)
            BCHP_SET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT1_SEL_LRANGE_ADJ, lRangeAdj);
        else
            BCHP_SET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_LRANGE_ADJ, lRangeAdj);
#endif
        NEXUS_Platform_WriteRegister(BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE + 4*rectDelta + (inputIndex*VID_NLCONFIG_INDEX_DELTA), reg);
    }
    if(curLRangeAdj != LRANGE_ADJ_DISABLE)
        _lumaRangeAdj[rectIndex] = curLRangeAdj;
}
#endif

#if HAS_GFX_NL_LUMA_RANGE_ADJ
bool CPlatform::getPlmLumaRangeAdjGraphics()
{
    uint32_t reg;
    unsigned lRangeAdj;

    NEXUS_Platform_ReadRegister(BCHP_GFD_0_NL_CSC_CTRL, &reg);
    lRangeAdj = BCHP_GET_FIELD_DATA(reg, GFD_0_NL_CSC_CTRL, LRANGE_ADJ_EN);

    return(lRangeAdj == BCHP_GFD_0_NL_CSC_CTRL_LRANGE_ADJ_EN_ENABLE);
}
#endif

#if HAS_GFX_NL_LUMA_RANGE_ADJ
void CPlatform::setPlmLumaRangeAdjGraphics(bool enable)
{
    uint32_t reg;
    unsigned lRangeAdj;
    unsigned curLRangeAdj;

    NEXUS_Platform_ReadRegister(BCHP_GFD_0_NL_CSC_CTRL, &reg);
    curLRangeAdj = BCHP_GET_FIELD_DATA(reg, GFD_0_NL_CSC_CTRL, LRANGE_ADJ_EN);
    if(true == enable)
        lRangeAdj = 1;
    else
        lRangeAdj = 0;
    if(lRangeAdj != curLRangeAdj) {
        BCHP_SET_FIELD_DATA(reg, GFD_0_NL_CSC_CTRL, LRANGE_ADJ_EN, lRangeAdj);
        NEXUS_Platform_WriteRegister(BCHP_GFD_0_NL_CSC_CTRL, reg);
    }
}
#endif