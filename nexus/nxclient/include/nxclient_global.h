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
 *
 * Module Description:
 *
 *****************************************************************************/
#ifndef NXCLIENT_GLOBAL_H__
#define NXCLIENT_GLOBAL_H__

#include "nexus_types.h"
#include "nexus_display_types.h"
#include "nexus_vbi.h"
#include "nexus_hdmi_types.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#include "nexus_hdmi_output_hdcp.h"
#else
typedef void *NEXUS_HdmiOutputStatus;
typedef void *NEXUS_HdmiOutputHdcpStatus;
typedef unsigned NEXUS_HdmiOutputCrcData;
#endif
#if NEXUS_HAS_AUDIO
#include "nexus_audio_processing_types.h"
#include "nexus_audio_mixer.h"
#else
typedef unsigned NEXUS_AutoVolumeLevelSettings;
typedef unsigned NEXUS_TruVolumeSettings;
typedef unsigned NEXUS_DolbyDigitalReencodeSettings;
typedef unsigned NEXUS_AudioMixerDolbySettings;
typedef unsigned NEXUS_DolbyVolume258Settings;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
The nxclient calls in this header file are global because they affect all clients.
**/

#define NXCLIENT_BAD_SEQUENCE_NUMBER   NEXUS_MAKE_ERR_CODE(0x200, 0)

/* 3 displays covers HD, SD and encode */
#define NXCLIENT_MAX_DISPLAYS 3

/**
Summary:
Output selection for local audio outputs.

Description:
eAuto is recommended because the server can automatically switch outputs for each codec based on capabilties.
Typical switches the server will make include:
* Downcovert AC3+ to AC3 if the device only supports the latter codec (using ePassthrough)
* Transcode AAC to AC3 if the device only supports the latter codec
* Decode MPEG and output as PCM if he device does not support the codec
* Passthrough AC3 and AAC as compressed if the device supports it
etc.

If you set another mode, the app will likely have to adjust as each audio codec is started, and one
client may not know what is being decoded on another client.

If you need to override one mode and not use eAuto, consider switching back to eAuto after that special mode
is satisfied.

This setting does not apply to SimpleEncoder-based transcode audio paths.
**/
typedef enum NxClient_AudioOutputMode
{
    NxClient_AudioOutputMode_eNone, /* no audio */
    NxClient_AudioOutputMode_eAuto, /* Use server defaults and/or HDMI EDID capabilities. */
    NxClient_AudioOutputMode_ePcm, /* stereo or mono PCM. See channelMode for left/right selection. */
    NxClient_AudioOutputMode_eMultichannelPcm,
    NxClient_AudioOutputMode_ePassthrough, /* compressed output.
        Passthrough of AC3+ to SPDIF automatically does a downconvert to AC3. */
    NxClient_AudioOutputMode_eTranscode, /* transcode to compressed output using 'transcodeCodec'.
        The encoder does not support all codecs as the decoder. */
    NxClient_AudioOutputMode_eMax
} NxClient_AudioOutputMode;

typedef struct NxClient_AudioOutputSettings
{
    NxClient_AudioOutputMode outputMode; /* Preferred format. Reverts to ePcm if mode not possible. */
    NEXUS_AudioChannelMode channelMode;  /* stereo/left/right selection for NxClient_AudioOutputMode_ePcm */
    NEXUS_AudioCodec transcodeCodec;     /* for NxClient_AudioOutputMode_eTranscode. System default is AC3. */
    unsigned additionalDelay; /* additional delay per output, in milliseconds. requires init-time allocation of memory. see -audio_output_delay nxserver cmdline option. */

    NEXUS_AudioVolumeType volumeType; /* Per-output volume.  This will be combined with the global volume in NxClient_Audiosettings if volumeType matches.
                                         If volume types are different and both global settings and these values are modified, these will take precedence.  */
    int32_t leftVolume;
    int32_t rightVolume;
    bool muted;

    NEXUS_AudioChannelStatusInfo channelStatusInfo; /* only applies to hdmi and spdif */
    NEXUS_AudioLoudnessDeviceMode loudnessDeviceMode; /* only applies to hdmi */
} NxClient_AudioOutputSettings;

/**
Summary:
Global audio settings

Description:
They apply to decode and compressed decoders (in as much as possible).
**/
typedef struct NxClient_AudioSettings {
    unsigned sequenceNumber;

    NEXUS_AudioVolumeType volumeType; /* global volume. specifies the units for leftVolume and rightVolume. */
    int32_t leftVolume; /* the units depend on the value of volumeType. See docs for NEXUS_AudioVolumeType. */
    int32_t rightVolume; /* the units depend on the value of volumeType. See docs for NEXUS_AudioVolumeType. */
    bool muted;

    NxClient_AudioOutputSettings hdmi, spdif, dac;
} NxClient_AudioSettings;

void NxClient_GetAudioSettings(
    NxClient_AudioSettings *pSettings
    );

NEXUS_Error NxClient_SetAudioSettings(
    const NxClient_AudioSettings *pSettings
    );

typedef struct NxClient_AudioProcessingSettings
{
    NEXUS_AutoVolumeLevelSettings avl; /* set avl.enabled = true to turn on Auto Volume Level for all PCM outputs */
    NEXUS_TruVolumeSettings truVolume;
    struct {
        NEXUS_DolbyDigitalReencodeSettings ddre; /* used when ms11 or ms12 is enabled */
        NEXUS_AudioMixerDolbySettings dolbySettings; /* used when ms11 or ms12 is enabled */
        NEXUS_DolbyVolume258Settings dolbyVolume258; /* used when ms11 is enabled */
    } dolby;
} NxClient_AudioProcessingSettings;

void NxClient_GetAudioProcessingSettings(
    NxClient_AudioProcessingSettings *pSettings
    );

NEXUS_Error NxClient_SetAudioProcessingSettings(
    const NxClient_AudioProcessingSettings *pSettings
    );

/**
Summary:
Actual audio status.

Description:
Status is based on client and/or server settings as well as system capabilities.
**/
typedef struct NxClient_AudioStatus {
    struct {
        NxClient_AudioOutputMode outputMode; /* Actual format. Never eAuto. */
        NEXUS_AudioChannelMode channelMode;  /* for NxClient_AudioOutputMode_ePcm */
        NEXUS_AudioCodec outputCodec;        /* if compressed output, which codec is being sent. if pcm, ePcm. */
    } hdmi, spdif, dac;
    struct {
        bool ddre;
        bool mixer; /* applies to dolbySettings */
        bool dolbyVolume258;
    } dolbySupport;
} NxClient_AudioStatus;

NEXUS_Error NxClient_GetAudioStatus(
    NxClient_AudioStatus *pStatus
    );

typedef enum NxClient_HdcpLevel
{
    NxClient_HdcpLevel_eNone,      /* No HDCP requested by this client. */
    NxClient_HdcpLevel_eOptional,  /* Authentication failure will only result in a callback. Video and audio will not be muted. */
    NxClient_HdcpLevel_eMandatory, /* Authentication failure will cause server to mute video and audio in addition to firing a callback. */
    NxClient_HdcpLevel_eMax
} NxClient_HdcpLevel;

typedef enum NxClient_HdcpVersion
{
    NxClient_HdcpVersion_eAuto,    /* Always authenticate using the highest version supported by HDMI receiver */
    NxClient_HdcpVersion_eFollow,  /* If HDMI receiver is a Repeater, the HDCP_version depends on the Repeater downstream topology */
                                   /* If Repeater downstream topology contains one or more HDCP 1.x device, then authenticate with Repeater using the HDCP 1.x */
                                   /* If Repeater downstream topology contains only HDCP 2.2 devices, then authenticate with Repeater using HDCP 2.2 */
                                   /* If HDMI Receiver is not a Repeater, then default to 'auto' selection */
    NxClient_HdcpVersion_eHdcp1x,  /* Always authenticate using HDCP 1.x mode (regardless of HDMI Receiver capabilities) */
    NxClient_HdcpVersion_eHdcp22,  /* Always authenticate using HDCP 2.2 mode (regardless of HDMI Receiver capabilities) */
    NxClient_HdcpVersion_eMax
} NxClient_HdcpVersion;

/* subset of NEXUS_GraphicsSettings for NxClient */
typedef struct NxClient_GraphicsSettings
{
    NEXUS_GraphicsFilterCoeffs horizontalFilter;   /* GFD horizontal upscaler coefficients */
    NEXUS_GraphicsFilterCoeffs verticalFilter;     /* GFD vertical  upscaler coefficients */
    unsigned horizontalCoeffIndex;                 /* if horizontalFilter == eSharp, then this index is used for table-driven coefficients for horizontal upscale. */
    unsigned verticalCoeffIndex;                   /* if verticalFilter == eSharp, then this index is used for table-driven coefficients for vertical upscale. */
    uint8_t alpha;                                 /* GFD alpha, from 0 (transparent) to 0xFF (opaque). Applied in addition to per-pixel alpha. */
} NxClient_GraphicsSettings;

/**
Summary:
Global display settings

Description:
They apply direction to the main display.
They will be transformed and applied to any secondary (e.g. SD) displays.
The server adjusts graphics automatically on format change.
**/
typedef struct NxClient_DisplaySettings
{
    unsigned sequenceNumber;
    NEXUS_VideoFormat format;
    NEXUS_DisplayAspectRatio aspectRatio;
    struct {
        unsigned x, y;
    } sampleAspectRatio;         /* Valid if aspectRatio is NEXUS_DisplayAspectRatio_eSar */
    NEXUS_Pixel backgroundColor; /* surface compositor background color. fills graphics plane where no client surface is visible. */
    NxClient_GraphicsSettings graphicsSettings;
    bool secure;

    struct {
        NEXUS_VideoFormat format;
        NEXUS_DisplayAspectRatio aspectRatio;
        NEXUS_Pixel backgroundColor; /* surface compositor background color. fills graphics plane where no client surface is visible. */
        struct {
            unsigned x, y;
        } sampleAspectRatio;         /* Valid if aspectRatio is NEXUS_DisplayAspectRatio_eSar */
        NxClient_GraphicsSettings graphicsSettings;
    } slaveDisplay[NXCLIENT_MAX_DISPLAYS-1]; /* where slaveDisplay[0] is SD display, slaveDisplay[1] is encode display. */

    struct {
       NEXUS_VideoOrientation orientation; /* override orientation of the 3D display. If 2D, then 'format' will determine if 2D or 3D. */
    } display3DSettings;
    struct {
        bool enabled;
        bool followPreferredFormat;
        bool preventUnsupportedFormat;
        NxClient_HdcpLevel hdcp; /* Client sets its desired level. Server aggregates requests from all clients and drives
                      HDCP authentication. Check NxClient_DisplayStatus.hdmi.hdcp for status and hdmiOutputHdcpChanged for callback. */
        NxClient_HdcpVersion version;
        NEXUS_ColorSpace colorSpace;
        unsigned colorDepth;
        NEXUS_HdmiDynamicRangeMasteringInfoFrame drmInfoFrame;
        NEXUS_MatrixCoefficients matrixCoefficients;
    } hdmiPreferences;
    struct {
        bool enabled;
        bool mpaaDecimationEnabled;
    } componentPreferences;
    struct {
        bool enabled;
        unsigned rfmChannel; /* 3 or 4 */
    } compositePreferences; /* applies to composite and rfm */
} NxClient_DisplaySettings;

void NxClient_GetDisplaySettings(
    NxClient_DisplaySettings *pSettings
    );

NEXUS_Error NxClient_SetDisplaySettings(
    const NxClient_DisplaySettings *pSettings
    );

typedef struct NxClient_DisplayStatus
{
    struct {
        unsigned number;
    } framebuffer;
    struct {
        unsigned total;
        unsigned used;
    } transcodeDisplays;

    /* NxClient_DisplayStatus.hdmi is deprecated. Use NEXUS_HdmiOutput_Open(NEXUS_ALIAS_ID + 0) to get status
    directly from a read-only alias. */
    struct {
        NEXUS_HdmiOutputStatus status; /* does not provide 4Kp60 preferred format status */
        NEXUS_HdmiOutputHdcpStatus hdcp;
    } hdmi;
} NxClient_DisplayStatus;

NEXUS_Error NxClient_GetDisplayStatus(
    NxClient_DisplayStatus *pStatus
    );

/* these are only for display-based PQ settings.
for video window PQ settings, see SimpleVideoDecoder. */
typedef struct NxClient_PictureQualitySettings
{
    NEXUS_GraphicsColorSettings graphicsColor;
} NxClient_PictureQualitySettings;

void NxClient_GetPictureQualitySettings(
    NxClient_PictureQualitySettings *pSettings
    );

NEXUS_Error NxClient_SetPictureQualitySettings(
    const NxClient_PictureQualitySettings *pSettings
    );

typedef struct NxClient_CallbackStatus
{
    /* incrementing number. if different from previous read, there was a change. */
    unsigned hdmiOutputHotplug;
    unsigned hdmiOutputHdcpChanged;
    unsigned displaySettingsChanged;
} NxClient_CallbackStatus;

/**
Poll for change in status.
**/
NEXUS_Error NxClient_GetCallbackStatus(
    NxClient_CallbackStatus *pStatus
    );

typedef struct NxClient_CallbackThreadSettings
{
    NEXUS_CallbackDesc hdmiOutputHotplug; /* called if NxClient_CallbackStatus.hdmiOutputHotplug increments */
    NEXUS_CallbackDesc hdmiOutputHdcpChanged;
    NEXUS_CallbackDesc displaySettingsChanged; /* called if NxClient_CallbackStatus.displayStatusChanged increments */
    unsigned interval; /* polling interval in milliseconds */
} NxClient_CallbackThreadSettings;

void NxClient_GetDefaultCallbackThreadSettings(
    NxClient_CallbackThreadSettings *pSettings
    );

/**
Start thread for asynchronous notification of NxClient_CallbackStatus change.
**/
NEXUS_Error NxClient_StartCallbackThread(
    const NxClient_CallbackThreadSettings *pSettings
    );

void NxClient_StopCallbackThread(void);

/**
Write VBI to the display
**/
NEXUS_Error NxClient_Display_WriteTeletext(
    const NEXUS_TeletextLine *pLines,   /* array of NEXUS_TeletextLine entries to output */
    size_t numLines,                    /* number of NEXUS_TeletextLine entries pointed to by pLines */
    size_t *pNumLinesWritten            /* [out] number of NEXUS_TeletextLine entries written */
    );

NEXUS_Error NxClient_Display_WriteClosedCaption(
    const NEXUS_ClosedCaptionData *pEntries,    /* array of NEXUS_ClosedCaptionData entries to output */
    size_t numEntries,                          /* number of NEXUS_ClosedCaptionData entries pointed to by pEntries */
    size_t *pNumEntriesWritten                  /* [out] number of NEXUS_ClosedCaptionData entries written */
    );

NEXUS_Error NxClient_Display_SetWss(
    uint16_t wssData
    );

NEXUS_Error NxClient_Display_SetCgms(
    uint32_t cgmsData
    );

NEXUS_Error NxClient_Display_SetCgmsB(
    const uint32_t *pCgmsData, /* array of 'size' uint32_t's */
    unsigned size
    );

/**
Enable macrovision on all displays where it is supported.
If a display format is changed and macrovision now becomes supportable, the app
must call NxClient_Display_SetMacrovision again.
**/
NEXUS_Error NxClient_Display_SetMacrovision(
    NEXUS_DisplayMacrovisionType type,
    const NEXUS_DisplayMacrovisionTables *pTable  /* attr{null_allowed=y} Optional macrovision tables if type == NEXUS_DisplayMacrovisionType_eCustom. */
    );

typedef struct NxClient_DisplayCrcData
{
    NEXUS_DisplayCrcData data[8];
    unsigned numEntries; /* numEntries of data[] that are populated */
} NxClient_DisplayCrcData;

/**
Will set NEXUS_DisplaySettings.crcQueueSize to 32 if zero.
**/
NEXUS_Error NxClient_Display_GetCrcData(
    unsigned displayIndex, /* 0 is main, 1 is first slave, etc. */
    NxClient_DisplayCrcData *pData
    );

typedef struct NxClient_HdmiOutputCrcData
{
    NEXUS_HdmiOutputCrcData data[8];
    unsigned numEntries; /* numEntries of data[] that are populated */
} NxClient_HdmiOutputCrcData;

/**
Will set NEXUS_HdmiOutputSettings.crcQueueSize to 32 if zero.
**/
NEXUS_Error NxClient_HdmiOutput_GetCrcData(
    NxClient_HdmiOutputCrcData *pData
    );

/**
Capture screenshot of graphics and video on the display

Video is taken directly from the decoder, so does not include picture quality
processing (like deinterlacing) and lacks aspect ratio control. So it is
not an exact replica of what is on the display. Non-decoder inputs (like HDMI)
are not supported.
**/
typedef struct NxClient_ScreenshotSettings
{
    unsigned tbd;
} NxClient_ScreenshotSettings;

void NxClient_GetDefaultScreenshotSettings(
    NxClient_ScreenshotSettings *pSettings
    );

NEXUS_Error NxClient_Screenshot(
    const NxClient_ScreenshotSettings *pSettings,
    NEXUS_SurfaceHandle surface
    );

NEXUS_Error NxClient_GrowHeap(
    unsigned heapIndex /* client heap index. see NEXUS_ClientConfiguration.heap[]. for now, should be NXCLIENT_DYNAMIC_HEAP. */
    );

void NxClient_ShrinkHeap(
    unsigned heapIndex
    );

typedef enum NxClient_HdcpType
{
    NxClient_HdcpType_1x,
    NxClient_HdcpType_2x,
    NxClient_HdcpType_eMax
} NxClient_HdcpType;

NEXUS_Error NxClient_LoadHdcpKeys(
    NxClient_HdcpType hdcpType,
    NEXUS_MemoryBlockHandle block, /* Keys are copied. Memory can be freed after call returns.
                                      Memory must be CPU accessible in server. */
    unsigned blockOffset, /* offset into preceeding block for start of keys */
    unsigned size /* Size of keys in bytes */
    );

#ifdef __cplusplus
}
#endif

#endif /* NXCLIENT_GLOBAL_H__ */
