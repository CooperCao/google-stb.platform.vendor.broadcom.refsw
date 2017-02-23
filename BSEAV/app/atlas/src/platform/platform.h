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

#include "nexus_platform.h"
#include "nexus_audio_dummy_output.h"
#include "nexus_i2s_output.h"

#ifndef PLATFORM_H__
#define PLATFORM_H__

#ifndef NEXUS_NUM_VIDEO_ENCODERS
#define NEXUS_NUM_VIDEO_ENCODERS        0
#endif

#define ATLAS_NUM_PCM_PLAYBACKS         2
#define ATLAS_NUM_PCM_CAPTURES          2
#define ATLAS_NUM_ES_DECODES            1
#define ATLAS_NUM_STREAMERS             1
#define ATLAS_NUM_LINEIN                0
#define ATLAS_NUM_IR_REMOTES            1
#define ATLAS_NUM_RF4CE_REMOTES         1
#define ATLAS_NUM_BLUETOOTH_REMOTES     1
#define ATLAS_MAX_PROGRAMS              8
#define ATLAS_NUM_SIMPLE_AUDIO_DECODES  NEXUS_NUM_AUDIO_DECODERS
#define ATLAS_NUM_SIMPLE_VIDEO_DECODES  NEXUS_NUM_VIDEO_DECODERS
#define ATLAS_NUM_GRAPHICS              1
#define ATLAS_NUM_TIMEBASES             NEXUS_NUM_TIMEBASES - (NEXUS_NUM_VIDEO_DECODERS + NEXUS_NUM_VIDEO_ENCODERS)

class CPlatform
{
public:
    CPlatform();
    ~CPlatform();

    void initialize(void);

    NEXUS_PlatformSettings *    getPlatformSettings(void);
    uint32_t                    getNumWindowsPerDisplay(void);
    NEXUS_HdmiOutputHandle      getOutputHdmi(uint16_t number);
    NEXUS_ComponentOutputHandle getOutputComponent(uint16_t number);
    NEXUS_SvideoOutputHandle    getOutputSvideo(uint16_t number);
    NEXUS_CompositeOutputHandle getOutputComposite(uint16_t number);
#if NEXUS_HAS_RFM
    NEXUS_RfmHandle getOutputRFM(uint16_t number);
#endif
    NEXUS_I2sOutputHandle               getOutputI2sDac(uint16_t number);
    NEXUS_AudioDacHandle                getOutputAudioDac(uint16_t number);
    NEXUS_SpdifOutputHandle             getOutputSpdif(uint16_t number);
    NEXUS_AudioDummyOutputHandle        getOutputAudioDummy(uint16_t number);
    NEXUS_FrontendHandle                getFrontend(uint16_t number);
    NEXUS_MemoryConfigurationSettings * getMemSettings(void) { return(&_memSettings); }
    NEXUS_VideoFormat                   getDisplayMaxVideoFormat(int numDisplay = 0);
    NEXUS_VideoFormat                   getDecoderMaxVideoFormat(int numDecoder = 0);

    bool isInitialized(void) { return(_bInitialized); }
    bool isSupportedDisplay(uint16_t number);
    bool isSupportedVideoWindow(uint16_t numDisplay, uint16_t number);
    bool isSupportedDecoder(uint16_t number);
    bool isSupportedStillDecoder(uint16_t number);
    bool isSupportedEncoder(uint16_t number);
    bool isMemSettingsValid(void) { return(_bMemSettingsValid); }

protected:
    bool                              _bInitialized;
    NEXUS_PlatformSettings            _platformSettings;
    NEXUS_PlatformConfiguration       _platformConfig;
    NEXUS_MemoryConfigurationSettings _memSettings;
    bool                              _bMemSettingsValid;
}; /* CPlatform */

#endif /* PLATFORM_H__ */