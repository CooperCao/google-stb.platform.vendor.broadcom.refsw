/***************************************************************************
 * (c) 2002-2016 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
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
    bool isSupported = false;

    BSTD_UNUSED(number);
    BDBG_ASSERT(isInitialized());

#if NEXUS_HAS_VIDEO_DECODER
#if NEXUS_MAX_VIDEO_DECODERS
    BDBG_ASSERT(number < NEXUS_MAX_VIDEO_DECODERS);
    isSupported = _platformConfig.supportedDecoder[number];

    if (true == _bMemSettingsValid)
    {
        isSupported &= _memSettings.videoDecoder[number].used;
    }
#endif /* if NEXUS_MAX_VIDEO_DECODERS */
#endif /* if NEXUS_HAS_VIDEO_DECODER */

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
        int                       format       = 0;
        NEXUS_DisplayCapabilities capabilities;

        NEXUS_GetDisplayCapabilities(&capabilities);

        for (format = 0; format < NEXUS_VideoFormat_eMax; format++)
        {
            if (true == capabilities.displayFormatSupported[format])
            {
                if (videoFormatToVertRes(maxFormat).toInt() < videoFormatToVertRes((NEXUS_VideoFormat)format).toInt())
                {   /* Simple fix hard-codes SD display 1 to fix 7125 and any other 65nm platform that doesn't have
                     _bMemSettingsValid.  Can be removed when those platforms are no longer supported in Nexus. */
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
}

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