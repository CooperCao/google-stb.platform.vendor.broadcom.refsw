/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef NXCLIENT_GLOBAL_H__
#define NXCLIENT_GLOBAL_H__

#include "nexus_types.h"
#include "nexus_display_types.h"
#include "nexus_vbi.h"
#include "nexus_hdmi_types.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#include "nexus_hdmi_output_hdcp.h"
#endif
#if NEXUS_HAS_AUDIO
#include "nexus_audio_processing_types.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_equalizer.h"
#endif
#include "nexus_core_compat.h"
#include "nexus_base_os.h"

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

typedef enum NxClientAudioCodecSupport
{
    NxClientAudioCodecSupport_eDefault, /* Follows the edid/default behavior for compressed */
    NxClientAudioCodecSupport_eEnabled, /* Forces codec to output compressed even if edid reports it as not supported */
    NxClientAudioCodecSupport_eDisabled, /* Forces codec to not output compressed even if edid reports it as not supported */
    NxClientAudioCodecSupport_eMax
} NxClientAudioCodecSupport;

typedef struct NxClient_AudioEqualizer
{
    unsigned numStages; /* Number of equalizer stages to be applied to output */
    NEXUS_AudioEqualizerStageSettings stageSettings[NEXUS_MAX_AUDIO_STAGES_PER_EQUALIZER]; /* Stage Settings each stage uses an SRC.
                                                                                  Best case senario NUM_MAX stages could be set */
} NxClient_AudioEqualizer;

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
    bool ditherEnabled;              /* Enable dither signal when no pcm audio data is present.
                                        Only applies to HDMI and spdif. */

    NEXUS_AudioChannelStatusInfo channelStatusInfo; /* only applies to hdmi and spdif */
    NEXUS_AudioLoudnessDeviceMode loudnessDeviceMode; /* only applies to hdmi */
    NxClientAudioCodecSupport compressedOverride[NEXUS_AudioCodec_eMax]; /* Overrides the expected behavior for compressed output based on the codec.
                                                                            Will affect eAuto, ePassthrough, eTranscode as well as how we fall back if things are not supported.
                                                                            As long as the transcode codec is supported (via edid or through this) transcode will could still be possible.
                                                                            Only applies to HDMI and spdif. */
    NxClient_AudioEqualizer equalizer; /* Equalizer settings */
    NEXUS_AudioPresentation presentation; /* Specifies which presentation output should get. Currently only supported for DAC and I2S outputs,
                                                other outputs always get main presentation. */
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
    bool dolbyMsAllowed; /* If false and we are running MS11/12 we will reconfigure the back end to remove the DSP Mixer and DDRE.
                             Need to started the server with MS11/12 enabled. You will lose AC3/DDP encode and audio description */
    int32_t loopbackVolumeMatrix[NEXUS_AudioChannel_eMax][NEXUS_AudioChannel_eMax]; /* For a multichannel mixer, if we have one
                                           or more PcmPlayback/FMM (loopback) inputs, apply these mixing coefficients.
                                           This can be used to mix across channel pairs, including subtraction.
                                           Common use case would be to map stereo into multichannel in a variety
                                           of different ways. For example, if content is mono, choose where that should
                                           be routed into the multichannel domain.
                                           Some example matrices:
                                           1. L->L, R->R (default)
                                              coeff[i][i] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL, zero all others
                                           2. L->C
                                              coeff[NEXUS_AudioChannel_eCenter][NEXUS_AudioChannel_eLeft] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL
                                           3. (L/2+R/2)->C, L-3(L+R)/8, R-3(L+R)/8 (upmix L and R to center, subtract 3/8 from L and R)
                                              coeff[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eLeft] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - 3*NEXUS_AUDIO_VOLUME_LINEAR_NORMAL/8
                                              coeff[NEXUS_AudioChannel_eLeft][NEXUS_AudioChannel_eRight] = -3*NEXUS_AUDIO_VOLUME_LINEAR_NORMAL/8
                                              coeff[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eLeft] = -3*NEXUS_AUDIO_VOLUME_LINEAR_NORMAL/8
                                              coeff[NEXUS_AudioChannel_eRight][NEXUS_AudioChannel_eRight] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - 3*NEXUS_AUDIO_VOLUME_LINEAR_NORMAL/8
                                              coeff[NEXUS_AudioChannel_eCenter][NEXUS_AudioChannel_eLeft] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL/2
                                              coeff[NEXUS_AudioChannel_eCenter][NEXUS_AudioChannel_eRight] = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL/2
                                              */
    NxClient_AudioOutputSettings hdmi;
    NxClient_AudioOutputSettings spdif;
    NxClient_AudioOutputSettings dac; /* DAC settings and I2S0 unless -i2s0 specified at runtime */
    NxClient_AudioOutputSettings i2s[NEXUS_MAX_AUDIO_I2S_OUTPUTS];
    NxClient_AudioOutputSettings rfm;
    NEXUS_AudioLoudnessSettings loudnessSettings;
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
    NEXUS_AudioAdvancedTsmSettings advancedTsm; /* used when ms11 or ms12 is enabled */
} NxClient_AudioProcessingSettings;

void NxClient_GetAudioProcessingSettings(
    NxClient_AudioProcessingSettings *pSettings
    );

NEXUS_Error NxClient_SetAudioProcessingSettings(
    const NxClient_AudioProcessingSettings *pSettings
    );


/**
Summary:
Actual audio output status.

Description:
Status is based on client and/or server settings as well as system capabilities.
**/

typedef struct NxClient_AudioOutputStatus
{
    NxClient_AudioOutputMode outputMode; /* Actual format. Never eAuto. */
    NEXUS_AudioChannelMode channelMode;  /* for NxClient_AudioOutputMode_ePcm */
    NEXUS_AudioCodec outputCodec;        /* if compressed output, which codec is being sent. if pcm, ePcm. */
} NxClient_AudioOutputStatus;

/**
Summary:
Actual audio status.

Description:
Status is based on client and/or server settings as well as system capabilities.
**/
typedef struct NxClient_AudioStatus {
    NxClient_AudioOutputStatus hdmi;
    NxClient_AudioOutputStatus spdif;
    NxClient_AudioOutputStatus dac; /* DAC status and I2S0 unless -i2s0 specified on at runtime */
    NxClient_AudioOutputStatus i2s[NEXUS_MAX_AUDIO_I2S_OUTPUTS];
    NxClient_AudioOutputStatus rfm;

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
    NxClient_HdcpLevel_eOptional,  /* Enable HDCP but do not mute video, even on authentication failure. */
    NxClient_HdcpLevel_eMandatory, /* Enable HDCP and mute video until authentication success. */
    NxClient_HdcpLevel_eMax
} NxClient_HdcpLevel;

typedef enum NxClient_HdcpVersion
{
    NxClient_HdcpVersion_eAuto,  /* Authenticate with either HDCP 1.x or HDCP 2.2, depending on directly connected device.
                                      If connected to HDCP 2.2 repeater, select Content Stream Type 0 or 1 depending on connected TV.
                                      Scenarios:
                                        STB -> repeater(2.2) -> TV(2.2)   use HDCP 2.2 content stream type 1
                                        STB -> repeater(2.2) -> TV(1.x)   use HDCP 2.2 content stream type 0
                                        STB -> repeater(1.x) -> TV(1.x)   use HDCP 1.x
                                      Requires extra authentication step for HDCP 2.2 content stream type 0.
                                      This mode cannot be used for HDCP 2.2 compliance testing. */
    NxClient_HdcpVersion_eFollow = NxClient_HdcpVersion_eAuto, /* deprecated */
    NxClient_HdcpVersion_eHdcp1x,  /* Always authenticate using HDCP 1.x mode (regardless of HDMI Receiver capabilities) */
    NxClient_HdcpVersion_eAutoHdcp22Type0, /* Authenticate with either HDCP 1.x or HDCP 2.2 Content Stream Type 0, depending only on directly connected device.
                                      Scenarios:
                                        STB -> repeater(2.2) -> TV(2.2 or 1.x)   use HDCP 2.2 content stream type 0
                                        STB -> repeater(1.x) -> TV(1.x)          use HDCP 1.x
                                      Unlike eFollow, this does not require an extra authentication step. */
    NxClient_HdcpVersion_eHdcp22,  /* Always authenticate using HDCP 2.2 mode, Content Stream Type 1 */
    NxClient_HdcpVersion_eMax
} NxClient_HdcpVersion;

/* subset of NEXUS_GraphicsSettings for NxClient */
typedef struct NxClient_GraphicsSettings
{
    /* Control blend of final graphics framebuffer and video using per-pixel or constant alpha.
    See nexus_display_types.h for values of NEXUS_CompositorBlendFactor. */
    NEXUS_CompositorBlendFactor sourceBlendFactor; /* Source is the graphics to be blended with video. */
    NEXUS_CompositorBlendFactor destBlendFactor; /* Dest is video to be blended with graphics. */
    uint8_t constantAlpha; /* used if sourceBlendFactor or destBlendFactor specify it. */

    NEXUS_GraphicsFilterCoeffs horizontalFilter;   /* GFD horizontal upscaler coefficients */
    NEXUS_GraphicsFilterCoeffs verticalFilter;     /* GFD vertical  upscaler coefficients */
    unsigned horizontalCoeffIndex;                 /* if horizontalFilter == eSharp, then this index is used for table-driven coefficients for horizontal upscale. */
    unsigned verticalCoeffIndex;                   /* if verticalFilter == eSharp, then this index is used for table-driven coefficients for vertical upscale. */
    uint8_t alpha;                                 /* GFD alpha, from 0 (transparent) to 0xFF (opaque). Applied in addition to per-pixel alpha. */
    bool enabled;                                  /* if set to false graphics is disabled */
    /*
     * With hdr display, sdr gfx pixel values are always adjusted lower to avoid being too bright / too saturated.
     * The following settings allow linear adjustment of gfx pixel values to approximate an sdr to hdr conversion.
     * These settings have no effect on PLM-capable platforms.
     */
    struct {
        int16_t y, cb, cr; /* valid range: 32767 to -32768. default is 0. The smaller this number, the dimmer / less saturated */
    } sdrToHdr;
} NxClient_GraphicsSettings;

typedef enum NxClient_SlaveDisplayMode
{
    NxClient_SlaveDisplayMode_eReplicated, /* replicate graphics and video from main display */
    NxClient_SlaveDisplayMode_eGraphics, /* show special graphic on slave display. see NxClient_SetSlaveDisplayGraphics. */
    NxClient_SlaveDisplayMode_eMax
} NxClient_SlaveDisplayMode;

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
    NEXUS_VideoFormat defaultSdFormat; /* This defaults to an SD format (NTSC, PAL, etc.) if the system is not HD/SD simul and does not have component.
                                          If set, then when HDMI is disconnected, automatically switch the main display to this SD format.
                                          This allows SD outputs (composite, 656, etc.) to work automatically.
                                          When HDMI is reconnected, switch back to the TV's preferred format.
                                          When nxserver starts with HDMI disconnected, 'defaultSdFormat' is used instead of 'format'.
                                          If eUnknown, this feature is disabled. */
    NEXUS_DisplayAspectRatio aspectRatio;
    struct {
        unsigned x, y;
    } sampleAspectRatio;         /* Valid if aspectRatio is NEXUS_DisplayAspectRatio_eSar */
    NEXUS_Pixel backgroundColor; /* surface compositor background color. fills graphics plane where no client surface is visible. */
    NxClient_GraphicsSettings graphicsSettings;
    bool secure;
    NEXUS_TristateEnable dropFrame;
    NEXUS_DisplayPriority priority; /* see NEXUS_DisplaySettings.priority */

    struct {
        NxClient_SlaveDisplayMode mode;
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
        bool followPreferredFormat; /* For video only, audio does not have a preferred format */
        bool preventUnsupportedFormat; /* If disabled audio will ignore EDID and attempt compressed and may result in no audio. Enabled by default */
        NxClient_HdcpLevel hdcp; /* Client sets its desired level. Server aggregates requests from all clients and drives
                      HDCP authentication. Check NxClient_DisplayStatus.hdmi.hdcp for status and hdmiOutputHdcpChanged for callback. */
        NxClient_HdcpVersion version;
        NEXUS_ColorSpace colorSpace;
        unsigned colorDepth;

        /* Sets the dynamic range mode of the video at the HDMI output. See NEXUS_DisplaySettings.dynamicRangeMode comments. */
        NEXUS_VideoDynamicRangeMode dynamicRangeMode;
        /*
         * DEPRECATED. The dynamicRangeMode member will take precedence over
         * this setting, unless dynamicRangeMode is set to NEXUS_HdmiDynamicRangeMode_eAuto
         * and drmInfoFrame.eotf is not set to NEXUS_VideoEotf_eMax.
         *
         * New dynamic range modes are only available via the use of the NEXUS_HdmiDynamicRangeMode
         * enum member dynamicRangeMode below. In addition, the metadata contained in
         * NEXUS_HdmiDynamicRangeMasteringInfoFrame should only be set for debug
         * purposes, as setting the metadata using the NxClient_setDisplaySettings
         * call is not frame-synchronous.
         *
         * The following table explains how the deprecated and new APIs work together.
         *
         * "Forced" means that the output mode will be forced even if the TV doesn't support it.
         * NEXUS_VideoEotf is set via the NEXUS_HdmiDynamicRangeMasteringInfoFrame.eotf field below (the DEPRECATED API).
         * NEXUS_HdmiDynamicRangeMode is set via the dynamicRangeMode field above (the new API).
         * DRMIF - Yes means the DRMIF is transmitted; No means it is not.
         * "xxx" means that it doesn't matter what the given column is set to
         * "Auto" will try to select the best mode based on Broadcom usage rules
         *
         * Usage        DRMIF  Deprecated  Forced  NEXUS_VideoEotf            NEXUS_HdmiDynamicRangeMode
         * ------------ ------ ----------- ------- -------------------------- ---------------------------------------
         * Legacy SDR   No     Yes         Yes     NEXUS_VideoEotf_eInvalid   NEXUS_HdmiDynamicRangeMode_eAuto
         * Legacy SDR   No     No          No      xxx                        NEXUS_HdmiDynamicRangeMode_eLegacy
         * SDR          Yes    Yes         Yes     NEXUS_VideoEotf_eSdr       NEXUS_HdmiDynamicRangeMode_eAuto
         * SDR          Yes    No          No      xxx                        NEXUS_HdmiDynamicRangeMode_eSdr
         * HDR10/PQ     Yes    Yes         Yes     NEXUS_VideoEotf_eHdr10/Pq  NEXUS_HdmiDynamicRangeMode_eAuto
         * HDR10/PQ     Yes    No          No      xxx                        NEXUS_HdmiDynamicRangeMode_eHdr10
         * HLG          Yes    Yes         Yes     NEXUS_VideoEotf_eHlg       NEXUS_HdmiDynamicRangeMode_eAuto
         * HLG          Yes    No          No      xxx                        NEXUS_HdmiDynamicRangeMode_eHlg
         * Dolby Vision No     No          No      xxx                        NEXUS_HdmiDynamicRangeMode_eDolbyVision
         * HDR10+       Yes    No          No      xxx                        NEXUS_HdmiDynamicRangeMode_eHdr10Plus
         * Track Input  *      No          No      xxx                        NEXUS_HdmiDynamicRangeMode_eTrackInput
         * Auto         *      No          No      NEXUS_VideoEotf_eMax       NEXUS_HdmiDynamicRangeMode_eAuto
         *
         * Also see NEXUS_HdmiOutputExtraSettings.overrideDynamicRangeMasteringInfoFrame
         */
        NEXUS_HdmiDynamicRangeMasteringInfoFrame drmInfoFrame;
        NEXUS_MatrixCoefficients matrixCoefficients;
    } hdmiPreferences;
    struct {
        bool enabled;
        bool mpaaDecimationEnabled;
        bool sdDisplay; /* Connect component to SD display instead of HD display. If HD display is 4K, this allows component to still carry video. */
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

/**
If NxClient_DisplaySettings.slaveDisplay[].mode == NxClient_SlaveDisplayMode_eGraphics, this surface is displayed
instead of replicating from the main display. Set the mode first, then the surface.
**/
NEXUS_Error NxClient_SetSlaveDisplayGraphics(
    unsigned slaveDisplay, /* 0 = SD, 1 = miracast */
    NEXUS_SurfaceHandle surface /* will be copied */
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
        NEXUS_HdmiOutputStatus status;
        NEXUS_HdmiOutputHdcpStatus hdcp;
        NEXUS_HdmiOutputHdcpError lastHdcpError; /* NxClient does an automatic retry. If authentication fails,
            hdcp.hdcpError may return 0, but lastHdcpError will record the last error. lastHdcpError will
            be 0 if HDCP is authenticated or disabled. */
        NEXUS_VideoDynamicRangeMode dynamicRangeMode;
    } hdmi;
} NxClient_DisplayStatus;

NEXUS_Error NxClient_GetDisplayStatus(
    NxClient_DisplayStatus *pStatus
    );

typedef enum NxClientExternalAppState
{
    NxClientExternalAppState_eNone, /* Not running in external app mode. NxClient_Connect can work. */
    NxClientExternalAppState_eGraphicsOnly, /* External app enabled and decode disabled, Graphics support only.
                                       NxClient_Connect will fail. Server can switch to eDecode. */
    NxClientExternalAppState_eDecode, /* External app enabled and decode enabled.
                                       NxClient_Connect can work. Server can switch to eGraphicsOnly. */
    NxClientExternalAppState_eMax
} NxClientExternalAppState;

typedef struct NxClient_Status
{
    NxClientExternalAppState externalAppState;
} NxClient_Status;

NEXUS_Error NxClient_GetStatus(
    NxClient_Status *pStatus
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
    unsigned audioSettingsChanged;
    unsigned thermalConfigChanged;
    unsigned coolingAgentChanged;
    unsigned standbyStateChanged; /* incremented with NxClient_StandbySettings.settings.mode changes */
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
    NEXUS_CallbackDesc displaySettingsChanged; /* called if NxClient_CallbackStatus.displaySettingsChanged increments */
    NEXUS_CallbackDesc audioSettingsChanged; /* called if NxClient_CallbackStatus.audioSettingsChanged increments */
    NEXUS_CallbackDesc thermalConfigChanged;  /* called when the thermal configuration changes */
    NEXUS_CallbackDesc coolingAgentChanged;  /* called when the cooling agent changes */
    NEXUS_CallbackDesc standbyStateChanged;
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
    unsigned blockOffset, /* offset into preceding block for start of keys */
    unsigned size /* Size of keys in bytes */
    );

/* Set this HDMI input as a repeater to the HDMI output, and trigger a new hdcp authentication attempt
(after hdcp authentication has already been enabled). */
NEXUS_Error NxClient_SetHdmiInputRepeater(
    NEXUS_HdmiInputHandle hdmiInput
    );

#define NXCLIENT_MAX_THERMAL_PRIORITIES 32
#define NXCLIENT_MAX_THERMAL_CONFIGS    16

typedef enum NxClient_CoolingAgent
{
    NxClient_CoolingAgent_eUnknown,
    NxClient_CoolingAgent_eCpuPstate,  /* Throttle CPU frequency by limiting max P state */
    NxClient_CoolingAgent_eCpuIdle,    /* CPU idle injection */
    NxClient_CoolingAgent_eGraphics3D, /* Throttle Graphics 3D frequency */
    NxClient_CoolingAgent_eGraphics2D, /* Throttle Graphics 2D frequency */
    NxClient_CoolingAgent_eDisplay,    /* Set Display Resolution */
    NxClient_CoolingAgent_eStopPip,    /* Stop Pip Decode */
    NxClient_CoolingAgent_eStopMain,   /* Stop Main Decode */
    NxClient_CoolingAgent_eUser,       /* User Defined Cooling Agent */
    NxClient_CoolingAgent_eMax
} NxClient_CoolingAgent;

typedef struct NxClient_ThermalStatus
{
    unsigned temperature;                /* Current Temperature in millidegrees C*/
    unsigned activeTempThreshold;        /* Temp threshold of active NxClient_ThermalConfiguration in millidegrees C. 0 if none active */
    struct {
        bool inUse;                      /* Indicates whether cooling agent is applied or not */
        NEXUS_Timestamp lastAppliedTime; /* Timestamp in ms when cooling agent was last applied */
        NEXUS_Timestamp lastRemovedTime; /* Timestamp in ms when cooling agent was last removed */
    } priorityTable[NXCLIENT_MAX_THERMAL_PRIORITIES];          /* index is priority from thermal.cfg file */
} NxClient_ThermalStatus;

NEXUS_Error NxClient_GetThermalStatus(
    NxClient_ThermalStatus *pStatus
    );

typedef struct NxClient_ThermalConfigurationList
{
    unsigned tempThreshold[NXCLIENT_MAX_THERMAL_CONFIGS]; /* array of keys for NxClient_ThermalConfiguration. 0 is unused slot. */
} NxClient_ThermalConfigurationList;

/**
Summary:
Load all available thermal configs
**/
void NxClient_GetThermalConfigurationList(
    NxClient_ThermalConfigurationList *pConfigList
    );

typedef struct NxClient_ThermalConfiguration
{
    unsigned overTempThreshold; /* Over Temp Threshold in millidegrees C at which thermal throttling is applied*/
    unsigned hysteresis;        /* Hysteresis in millidegrees C */
    unsigned overTempReset;     /* Over temperature reset threshold in millidegrees C*/
    unsigned overPowerThreshold;/* Power Threshold in mW at which throttling is applied */
    unsigned pollInterval;      /* Thermal polling interval in milliseconds */
    unsigned tempDelay;         /* Delay in milliseconds before applying next cooling agent */
    unsigned thetaJC;           /* Thermal Resistance of the Box */
    struct {
        NxClient_CoolingAgent agent;  /* Cooling Agent type */
    } priorityTable[NXCLIENT_MAX_THERMAL_PRIORITIES];
} NxClient_ThermalConfiguration;

/**
Summary:
Get the themal configuration for the specified threshold.
Returns error if no matching config is loaded
**/
NEXUS_Error NxClient_GetThermalConfiguration(
    unsigned tempThreshold, /* Unique key, which is the also the threshold for desired configuration. Specified in millidegrees celcius */
    NxClient_ThermalConfiguration *pConfig
    );

/**
Summary:
Enable/Disable or Modify a thermal configuration. using the threshold as a unique id.
Pass in NULL to disable a configuration.
A configuration can be enabled/disabled/modified only if its threshold is higher than all existing configurations.
**/
NEXUS_Error NxClient_SetThermalConfiguration(
    unsigned tempThreshold, /* Unique key, which is lower threshold to activate configuration, in millidegrees celcius */
    const NxClient_ThermalConfiguration *pConfig /* attr{null_allowed=y} Set NULL to unload a config. */
    );

/**
Summary:
If a client calls NxClient_SetWatchdogTimeout, it must call NxClient_SetWatchdogTimeout again
before that timeout expires, otherwise the box will be reset. Call with timeout 0 to clear a
client's watchdog. nxserver aggregates all client watchdog requests.

See NEXUS_WATCHDOG_MAX_TIMEOUT in nexus_watchdog.h for the max value.

Because NxClient_SetWatchdogTimeout is per client, if a client crashes with a pending watchdog
there is no way it can be cleared. Therefore the system will watchdog immediately.
**/
NEXUS_Error NxClient_SetWatchdogTimeout(
    unsigned timeout    /* timeout value in seconds for this client */
    );

#ifdef __cplusplus
}
#endif

#endif /* NXCLIENT_GLOBAL_H__ */
