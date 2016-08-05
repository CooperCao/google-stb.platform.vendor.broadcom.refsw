/******************************************************************************
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
 *****************************************************************************/
#ifndef NEXUS_CORE_AUDIO_H__
#define NEXUS_CORE_AUDIO_H__

#include "bstd.h"

/* Do not include Magnum audio header files here. */

typedef enum NEXUS_AudioInputType
{
    NEXUS_AudioInputType_eDecoder,
    NEXUS_AudioInputType_ePlayback,
    NEXUS_AudioInputType_eI2s,
    NEXUS_AudioInputType_eHdmi,
    NEXUS_AudioInputType_eSpdif,
    NEXUS_AudioInputType_eRf,
    NEXUS_AudioInputType_eRfDecoder,
    NEXUS_AudioInputType_eMixer,
    NEXUS_AudioInputType_eIntermediateMixer, /* APE Raaga platforms only - intermediate HW mixer*/
    NEXUS_AudioInputType_eDspMixer, /* Used only on APE platforms */
    NEXUS_AudioInputType_eEncoder,
    NEXUS_AudioInputType_eRfEncoder,
    NEXUS_AudioInputType_eBbe,
    NEXUS_AudioInputType_eXen,
    NEXUS_AudioInputType_eDolbyDigitalBass,
    NEXUS_AudioInputType_eDtsNeo,
    NEXUS_AudioInputType_eAutoVolumeLevel,
    NEXUS_AudioInputType_eProLogic2,
    NEXUS_AudioInputType_eTruSurroundXt,
    NEXUS_AudioInputType_eTruSurroundHd,
    NEXUS_AudioInputType_eTruVolume,
    NEXUS_AudioInputType_eCustomSurround,
    NEXUS_AudioInputType_eCustomBass,
    NEXUS_AudioInputType_eCustomVoice,
    NEXUS_AudioInputType_eAnalogDecoder,
    NEXUS_AudioInputType_eAnalogInput,
    NEXUS_AudioInputType_eDtsEncode,
    NEXUS_AudioInputType_eAc3Encode,
    NEXUS_AudioInputType_eDolbyVolume,
    NEXUS_AudioInputType_eDolbyVolume258,
    NEXUS_AudioInputType_e3dSurround,
    NEXUS_AudioInputType_eMonoDownmix,
    NEXUS_AudioInputType_eSbcEncode,
    NEXUS_AudioInputType_eAudysseyAbx,
    NEXUS_AudioInputType_eAudysseyAdv,
    NEXUS_AudioInputType_eDolbyDigitalReencode,
    NEXUS_AudioInputType_eStudioSound,
    NEXUS_AudioInputType_eCustomAvl,
    NEXUS_AudioInputType_eCustomDbe,
    NEXUS_AudioInputType_eCustomAcc,
    NEXUS_AudioInputType_eEchoCanceller,
    NEXUS_AudioInputType_eInputCapture,
    NEXUS_AudioInputType_eAudioProcessor,
    NEXUS_AudioInputType_eMax
} NEXUS_AudioInputType;

typedef enum NEXUS_AudioInputFormat
{
    NEXUS_AudioInputFormat_eNone,
    NEXUS_AudioInputFormat_ePcmStereo,
    NEXUS_AudioInputFormat_ePcm5_1,
    NEXUS_AudioInputFormat_ePcm7_1,
    NEXUS_AudioInputFormat_ePcmMono,
    NEXUS_AudioInputFormat_eCompressed,
    NEXUS_AudioInputFormat_eMax
} NEXUS_AudioInputFormat;

typedef enum NEXUS_AudioOutputType
{
    NEXUS_AudioOutputType_eDac = 0,
    NEXUS_AudioOutputType_eI2s,
    NEXUS_AudioOutputType_eI2sMulti,
    NEXUS_AudioOutputType_eSpdif,
    NEXUS_AudioOutputType_eHdmi,
    NEXUS_AudioOutputType_eRfm,
    NEXUS_AudioOutputType_eCapture,
    NEXUS_AudioOutputType_eArc,
    NEXUS_AudioOutputType_eMux,
    NEXUS_AudioOutputType_eGroup,
    NEXUS_AudioOutputType_eDummy,
    NEXUS_AudioOutputType_eMax
} NEXUS_AudioOutputType;

typedef struct NEXUS_AudioInputFormatData
{
    BAVC_AudioSamplingRate samplingRate;
} NEXUS_AudioInputFormatData;

typedef NEXUS_Error (*NEXUS_AudioInputConnectCb)(void *, NEXUS_AudioInputHandle);
typedef NEXUS_Error (*NEXUS_AudioInputDisconnectCb)(void *, NEXUS_AudioInputHandle);

/**
Typecast to BRAP_OutputPort in audio module.
**/
typedef enum NEXUS_AudioOutputPort
{
    NEXUS_AudioOutputPort_eSpdif = 0,
    NEXUS_AudioOutputPort_eDac0,
    NEXUS_AudioOutputPort_eI2s0,
    NEXUS_AudioOutputPort_eI2s1,
    NEXUS_AudioOutputPort_eI2s2,
    NEXUS_AudioOutputPort_eDac1,
    NEXUS_AudioOutputPort_eMai,
    NEXUS_AudioOutputPort_eFlex,
    NEXUS_AudioOutputPort_eRfMod,
#if BRAP_VER >= 4
    NEXUS_AudioOutputPort_eArc,
#endif
    NEXUS_AudioOutputPort_eI2s3,
    NEXUS_AudioOutputPort_eI2s4,
    NEXUS_AudioOutputPort_eSpdif1,
    NEXUS_AudioOutputPort_eI2s5,
    NEXUS_AudioOutputPort_eI2s6,
    NEXUS_AudioOutputPort_eI2s7,
    NEXUS_AudioOutputPort_eI2s8,
    NEXUS_AudioOutputPort_eDac2,
#if BRAP_VER >= 4
    NEXUS_AudioOutputPort_eI2s9,
#endif
    NEXUS_AudioOutputPort_eMaiMulti0,
    NEXUS_AudioOutputPort_eMaiMulti1,
    NEXUS_AudioOutputPort_eMaiMulti2,
    NEXUS_AudioOutputPort_eMaiMulti3,
#if BRAP_VER >= 4
    NEXUS_AudioOutputPort_eDummySink0,
    NEXUS_AudioOutputPort_eDummySink1,
    NEXUS_AudioOutputPort_eDummySink2,
    NEXUS_AudioOutputPort_eDummySink3,
#endif
    NEXUS_AudioOutputPort_eMax
} NEXUS_AudioOutputPort;

struct NEXUS_AudioInput
{
    NEXUS_OBJECT(NEXUS_AudioInput);
    NEXUS_AudioInputFormat format;      /* Data Format */
    NEXUS_AudioInputType objectType;    /* Object type */
    void *pObjectHandle;                /* Handle of actual "input" object */
    void *pMixerData;                   /* Blind mixer data container */
    void *pCrc;                         /* BAPE_CrcHandle - NULL (disabled) by default */
    NEXUS_ModuleHandle module;                      /* Module handle, used to serialize callbacks if necessary */
    NEXUS_AudioInputConnectCb connectCb;            /* Connect Callback - Called prior to starting in a new connection */
    NEXUS_AudioInputDisconnectCb disconnectCb;      /* Disconnect Callback - Called when connections change */
    size_t port;                        /* Connector port.  Typecast to BAPE_Connector accordingly. */
    size_t inputPort;                   /* Input port.  Typecast to BAPE_InputPort accordingly. */
    const char *pName;
};

typedef struct NEXUS_AudioInput NEXUS_AudioInputObject;

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioInput);

#define NEXUS_AUDIO_INPUT_INIT(input, inputType, handle) \
do \
{ \
    BKNI_Memset((input), 0, sizeof(NEXUS_AudioInputObject)); \
    NEXUS_OBJECT_SET(NEXUS_AudioInput, (input)); \
    (input)->objectType = (inputType); \
    (input)->pObjectHandle = (handle); \
    (input)->pName = ""; \
} while(0)

struct NEXUS_AudioOutput
{
    NEXUS_OBJECT(NEXUS_AudioOutput);
    NEXUS_AudioOutputType objectType;   /* Object type */
    void *pObjectHandle;                /* Handle of actual "input" object */
    void *pMixerData;                   /* Blind mixer data container */
    void *pCrc;                         /* BAPE_CrcHandle - NULL (disabled) by default */
    size_t port;                        /* Actual raptor output port, typecast to BRAP_OutputPort or BAPE_MixerOutput */
    const char *pName;                  /* Name of object */
    NEXUS_Error (*setChannelMode)(void *pHandle, NEXUS_AudioChannelMode channelMode);
};

typedef struct NEXUS_AudioOutput NEXUS_AudioOutputObject;

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioOutput);

#define NEXUS_AUDIO_OUTPUT_INIT(output, outputType, handle) \
do \
{ \
    BKNI_Memset((output), 0, sizeof(NEXUS_AudioOutputObject)); \
    NEXUS_OBJECT_SET(NEXUS_AudioOutput, (output)); \
    (output)->objectType = (outputType); \
    (output)->pObjectHandle = (handle); \
    (output)->pName = ""; \
} while(0)

#define NEXUS_AUDIO_MAX_OUTPUTS (NEXUS_AudioOutputPort_eMax)
typedef struct NEXUS_AudioOutputList
{
    NEXUS_AudioOutput outputs[NEXUS_AUDIO_MAX_OUTPUTS];
} NEXUS_AudioOutputList;

#endif /* #ifndef NEXUS_CORE_AUDIO_H__ */
