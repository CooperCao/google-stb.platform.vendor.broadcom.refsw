/***************************************************************************
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

#ifndef BIP_TRANSCODE_H
#define BIP_TRANSCODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_types.h"
#include "nexus_playback.h"
#include "nexus_recpump.h"
#include "bip.h"
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#include "nexus_audio_encoder.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_simple_encoder.h"
#if NEXUS_HAS_HDMI_INPUT
#include "nexus_hdmi_input.h"
#endif

/**
Summary:
This class provides the Data types and APIs used for Encoding an outgoing stream for HTTP, and UDP Transcode classes.
**/
typedef struct BIP_Transcode *BIP_TranscodeHandle;

/**
Summary
This structure defines the configuration associated with a encoding profile of a stream.

Description
Apps can define one or more of these profiles for streaming media using any of the streaming protocols (HTTP, HLS or MPEG DASH, UDP, etc.).

See Also:
BIP_Transcode_GetDefaultProfile(profile)
BIP_Transcode_GetDefaultProfileFor_720p30_AVC_AAC_MPEG2_TS(profile)
 **/
typedef struct BIP_TranscodeProfile
{
    BIP_SETTINGS(BIP_TranscodeProfile)                          /* Internal use... for init verification. */

    NEXUS_TransportType                     containerType;      /* Container type of the encoded stream */
    /* Container specific profile. */
    struct  /* These fields only apply when containerType==NEXUS_TransportType_eTs */
    {
        unsigned                            pmtPid;             /* If !=0, build & insert PAT & PMT. */
        unsigned                            pmtIntervalInMs;    /* How often to insert PAT & PMT. */
        unsigned                            pcrPid;             /* If !=0, use this PCR PID. */
    } mpeg2Ts;

    /* Video specific profile. */
    bool                                    disableVideo;       /* Set if Video should be disabled in encoded stream */
    struct
    {
        unsigned                            trackId;            /* Unique track ID (PID for MPEG2-TS, track_ID for ISOBMFF) */
        unsigned                            width;              /* Video width of a profile */
        unsigned                            height;             /* Video height of a profile */
        NEXUS_DisplayAspectRatio            aspectRatio;
        unsigned                            refreshRate;        /* Units of 1/1000 Hz. 59940 = 59.94Hz, 60000 = 60Hz; 0 uses default 59.94 or 50hz based on Encoder framerate settings */
        NEXUS_VideoEncoderStartSettings     startSettings;      /* Set codec, level, profile, etc. here. */
                                                                /* Display/window/stc handles are acquired internally. */
        NEXUS_VideoEncoderSettings          settings;           /* Set bitRate, frameRate, GOP settings, etc. here. */
    } video;

    /* Audio specific profile. */
    bool                                    disableAudio;       /* Set if Audio should be disabled in encoded stream */
    struct
    {
        unsigned                            trackId;            /* Unique track ID (PID for MPEG2-TS, track_ID for ISOBMFF) */
        bool                                passthrough;        /* If true, pass the original audio through without re-encoding */
        NEXUS_AudioCodec                    audioCodec;         /* Output codec (ignored if passthrough = true) */
        NEXUS_AudioEncoderCodecSettings     settings;           /* Ignored if passthrough = true */
    } audio;
} BIP_TranscodeProfile;
BIP_SETTINGS_ID_DECLARE(BIP_TranscodeProfile);

typedef struct BIP_TranscodeAudioTrackInfo
{
    NEXUS_AudioCodec codec;                     /*!< Audio codec */
}BIP_TranscodeAudioTrackInfo;

typedef struct BIP_TranscodeVideoTrackInfo
{
    NEXUS_VideoCodec codec;                         /*!< Video codec */
    uint16_t    width;                         /*!< Coded video width, or 0 if unknown */
    uint16_t    height;                        /*!< Coded video height, or 0 if unknown  */
    unsigned    colorDepth;                    /*!< For H265/HEVC video code, color depth: 8 --> 8 bits, 10 --> 10 bits, 0 for other Video Codecs. */
    const char  *pMediaNavFileAbsolutePathName;/*!< Media NAV file: set if NAV file is available for this Video track. */
}BIP_TranscodeVideoTrackInfo;

typedef struct BIP_TranscodeStreamInfo
{
    NEXUS_TransportType transportType;             /*!< Container format type */
    unsigned            numberOfTrackGroups;       /*!< Total number of trackGroups in the stream */
    unsigned            numberOfTracks;            /*!< Total number of tracks in the stream */
    unsigned            durationInMs;               /*!< Duration of stream in milliseconds or 0 if unknown */
    int64_t             contentLength;              /*!< Content length of the File, 0 if not known or for Live Channel */
    bool                transportTimeStampEnabled;  /*!< Indicates if MPEG2 TS content contains additional 4 byte timpstamp (192 byte Transport Packet) */
}BIP_TranscodeStreamInfo;

typedef struct BIP_TranscodeTrackInfo
{
    BIP_MediaInfoTrackType type;                /*!< Type of track: Audio/Video/Pcr */
    unsigned               trackId;             /*!< Unique track ID (PID for MPEG2-TS, track_ID for ISOBMFF) */

    union {
        BIP_TranscodeAudioTrackInfo audio;          /*!< Information for audio track */
        BIP_TranscodeVideoTrackInfo video;          /*!< Information for video track */
    } info;
}BIP_TranscodeTrackInfo;

/**
Summary:
API to allow app to get the default values for common profiles.

Apps can then change/adjust any of these encoding parameters for their custom encoding requirements.
See Also:
**/
#define BIP_Transcode_GetDefaultProfileFor_720p30_AVC_AAC_MPEG2_TS(profile)                         \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(profile, BIP_TranscodeProfile)                               \
        /* Set non-zero defaults explicitly. */                                                     \
        NEXUS_VideoEncoder_GetDefaultSettings(&(profile)->video.settings);                          \
        NEXUS_VideoEncoder_GetDefaultStartSettings(&(profile)->video.startSettings);                \
        (profile)->containerType                            = NEXUS_TransportType_eTs;              \
        (profile)->mpeg2Ts.pcrPid                           = 0x102;                                \
        (profile)->mpeg2Ts.pmtPid                           = 0x50;                                 \
        (profile)->mpeg2Ts.pmtIntervalInMs                  = 0x100;                                \
        (profile)->video.width                              = 1280;                                 \
        (profile)->video.height                             = 720;                                  \
        (profile)->video.refreshRate                        = 60000;                                \
        (profile)->video.trackId                            = 0x101;                                \
        (profile)->video.startSettings.codec                = NEXUS_VideoCodec_eH264;               \
        (profile)->video.startSettings.level                = NEXUS_VideoCodecLevel_e31;            \
        (profile)->video.startSettings.interlaced           = false;                                \
        (profile)->video.startSettings.adaptiveLowDelayMode = true;                                 \
        (profile)->video.startSettings.rateBufferDelay      = 1500;                                 \
        (profile)->video.settings.frameRate                 = NEXUS_VideoFrameRate_e30;             \
        (profile)->video.settings.bitrateMax                = 6000000;                              \
        (profile)->video.settings.streamStructure.duration  = 1000; /* 1 sec GOP, 1 Segment/GOP */  \
        (profile)->video.settings.streamStructure.adaptiveDuration  = true;                         \
        (profile)->audio.audioCodec                         = NEXUS_AudioCodec_eAac;                \
        (profile)->audio.trackId                            = 0x104;                                \
        (profile)->disableAudio                             = false;                                \
        BIP_SETTINGS_GET_DEFAULT_END

#define BIP_Transcode_GetDefaultProfileFor_480p30_AVC_AAC_MPEG2_TS(profile)                         \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(profile, BIP_TranscodeProfile)                               \
        /* Set non-zero defaults explicitly. */                                                     \
        NEXUS_VideoEncoder_GetDefaultSettings(&(profile)->video.settings);                          \
        NEXUS_VideoEncoder_GetDefaultStartSettings(&(profile)->video.startSettings);                \
        (profile)->containerType                            = NEXUS_TransportType_eTs;              \
        (profile)->mpeg2Ts.pcrPid                           = 0x102;                                \
        (profile)->mpeg2Ts.pmtPid                           = 0x50;                                 \
        (profile)->mpeg2Ts.pmtIntervalInMs                  = 0x100;                                \
        (profile)->video.width                              = 640;                                  \
        (profile)->video.height                             = 480;                                  \
        (profile)->video.refreshRate                        = 60000;                                \
        (profile)->video.trackId                            = 0x101;                                \
        (profile)->video.startSettings.codec                = NEXUS_VideoCodec_eH264;               \
        (profile)->video.startSettings.level                = NEXUS_VideoCodecLevel_e31;            \
        (profile)->video.startSettings.interlaced           = false;                                \
        (profile)->video.startSettings.adaptiveLowDelayMode = true;                                 \
        (profile)->video.startSettings.rateBufferDelay      = 1500;                                 \
        (profile)->video.settings.frameRate                 = NEXUS_VideoFrameRate_e30;             \
        (profile)->video.settings.bitrateMax                = 1000000;                              \
        (profile)->video.settings.streamStructure.duration  = 1000; /* 1 sec GOP, 1 Segment/GOP */  \
        (profile)->video.settings.streamStructure.adaptiveDuration  = true;                         \
        (profile)->audio.audioCodec                         = NEXUS_AudioCodec_eAac;                \
        (profile)->audio.trackId                            = 0x104;                                \
        BIP_SETTINGS_GET_DEFAULT_END

#define BIP_Transcode_GetDefaultProfile(profile)                                                    \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(profile, BIP_TranscodeProfile)                               \
        /* Set non-zero defaults explicitly. */                                                     \
        NEXUS_VideoEncoder_GetDefaultSettings(&(profile)->video.settings);                        \
        NEXUS_VideoEncoder_GetDefaultStartSettings(&(profile)->video.startSettings);              \
        (profile)->containerType                            = NEXUS_TransportType_eTs;              \
        (profile)->video.width                              = 720;                                  \
        (profile)->video.height                             = 1280;                                 \
        (profile)->video.startSettings.codec                = NEXUS_VideoCodec_eH264;               \
        (profile)->video.startSettings.level                = NEXUS_VideoCodecLevel_e31;            \
        (profile)->video.startSettings.interlaced           = false;                                \
        (profile)->video.settings.bitrateMax                = 1000000;                              \
        (profile)->video.settings.frameRate                 = NEXUS_VideoFrameRate_e30;             \
        (profile)->video.settings.streamStructure.duration  = 2000; /* 2 sec GOP, 1 Segment/GOP */  \
        (profile)->audio.audioCodec                         = NEXUS_AudioCodec_eAac;                \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary
Handles required for encoding.

See Also:
BIP_Transcode_GetDefaultTranscodeNexusHandless
**/
typedef struct BIP_TranscodeNexusHandles
{
    BIP_SETTINGS(BIP_TranscodeNexusHandles)       /* Internal use... for init verification. */

    bool                                    useSimpleHandles;
    struct
    {
        NEXUS_SimpleEncoderHandle           hEncoder;
        NEXUS_SimpleVideoDecoderHandle      hVideo;
        NEXUS_SimpleAudioDecoderHandle      hAudio;
        NEXUS_SimpleStcChannelHandle        hStcChannel;
    } simple;
    /* Note: Will add a structure for non-simple handles once that is supported. */
} BIP_TranscodeNexusHandles;
BIP_SETTINGS_ID_DECLARE(BIP_TranscodeNexusHandles);

#define BIP_Transcode_GetDefaultNexusHandles(pHandles)                          \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pHandles, BIP_TranscodeNexusHandles)     \
        /* Set non-zero defaults explicitly. */                                 \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
API to Create Transcode context.
*/
typedef struct BIP_TranscodeCreateSettings
{
    BIP_SETTINGS(BIP_TranscodeCreateSettings)       /* Internal use... for init verification. */
} BIP_TranscodeCreateSettings;
BIP_SETTINGS_ID_DECLARE(BIP_TranscodeCreateSettings);

#define BIP_Transcode_GetDefaultCreateSettings(pSettings)                       \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_TranscodeCreateSettings)  \
        /* Set non-zero defaults explicitly. */                                 \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_TranscodeHandle BIP_Transcode_Create(
    const BIP_TranscodeCreateSettings   *pSettings
    );

/**
Summary:
API to Get/Set Transcode runtime settings
*/
typedef struct BIP_TranscodeSettings
{
    BIP_TranscodeProfile        *pTranscodeProfile;     /*!< If set, this profile will be used to modify the Transcode settings. */
    bool                        flush;                  /*!< if set, Transcode pipeline will be flushed (e.g. set during seek). */
} BIP_TranscodeSettings;

void BIP_Transcode_GetSettings(
    BIP_TranscodeHandle      hTranscode,
    BIP_TranscodeSettings    *pSettings
    );

BIP_Status BIP_Transcode_SetSettings(
    BIP_TranscodeHandle      hTranscode,
    BIP_TranscodeSettings    *pSettings
    );

/**
Summary:
API to prepare Transcode.

Description:
*/
typedef struct BIP_TranscodePrepareSettings
{
    BIP_SETTINGS(BIP_TranscodePrepareSettings)      /* Internal use... for init verification. */

    BIP_TranscodeNexusHandles   *pNexusHandles;     /* Caller/App can optionally provide the Transcode related Nexus handles. Otherwise, BIP Transcode will internally open/acquire them. */
    NEXUS_PidChannelHandle      hVideoPidChannel;   /* Must be set if Video needs to be enabled in the output encoded stream. */
    BIP_TranscodeTrackInfo      videoTrack;
    NEXUS_PidChannelHandle      hAudioPidChannel;   /* Must be set if Audio needs to be enabled in the output encoded stream. */
    BIP_TranscodeTrackInfo      audioTrack;
    NEXUS_PidChannelHandle      hPcrPidChannel;     /* Must be set if nonRealTime mode is not set in encoded stream. */
    BIP_TranscodeTrackInfo      pcrTrack;           /* Must be specified for the Live Channels, NULL otherwise. */
    NEXUS_PlaybackHandle        hPlayback;          /* Must be specified if input is being fed via the Playback Channel. */
#if NEXUS_HAS_HDMI_INPUT
    NEXUS_HdmiInputHandle       hHdmiInput;         /* Must be specified if input source is the HDMI input. */
#endif
    bool                        enableRaiIndex;     /* If set, RAI index will be enabled (aids in identifying Segment boundaries for Adaptive Streaming). */
} BIP_TranscodePrepareSettings;
BIP_SETTINGS_ID_DECLARE(BIP_TranscodePrepareSettings );

#define BIP_Transcode_GetDefaultPrepareSettings(pSettings)                          \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_TranscodePrepareSettings )    \
        /* Set non-zero defaults explicitly. */                                     \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_Status BIP_Transcode_Prepare(
    BIP_TranscodeHandle             hTranscode,
    BIP_TranscodeStreamInfo         *pTranscodeStreamInfo,
    BIP_TranscodeProfile            *pTranscodeProfile,
    NEXUS_RecpumpHandle             hRecpump,
    bool                            nonRealTime,
    BIP_TranscodePrepareSettings    *pSettings
    );

/**
Summary:
API to start Transcode.

Description:
This API is used to start Transcode.

*/
typedef struct BIP_TranscodeStartSettings
{
    BIP_SETTINGS(BIP_TranscodeStartSettings)   /* Internal use... for init verification. */

    unsigned unused;

} BIP_TranscodeStartSettings;
BIP_SETTINGS_ID_DECLARE(BIP_TranscodeStartSettings );

#define BIP_Transcode_GetDefaultStartSettings(pSettings)                       \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_TranscodeStartSettings ) \
        /* Set non-zero defaults explicitly. */                               \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_Status BIP_Transcode_Start(
    BIP_TranscodeHandle         hTranscode,
    BIP_TranscodeStartSettings  *pSettings
    );

/**
Summary:
API to Stop Transcode

Description:

This API allows caller to stop & reset Transcode to its original state as if Transcode was just created.
*/
BIP_Status BIP_Transcode_Stop(
    BIP_TranscodeHandle     hTranscode
    );

/**
Summary:
API to return Transcode status

Description:
*/
typedef struct BIP_TranscodeStatus
{
    bool                TranscodeActive;    /* Set to true if streaming is currently in progress on this Transcode. */
} BIP_TranscodeStatus;

BIP_Status BIP_Transcode_GetStatus(
    BIP_TranscodeHandle     hTranscode,
    BIP_TranscodeStatus     *pStatus
    );

/**
Summary:
API to Destroy a Transcode context
*/
void BIP_Transcode_Destroy(
    BIP_TranscodeHandle     Transcode
    );

/**
Summary:
API to Print Steamer Status
*/
void BIP_Transcode_PrintStatus(
    BIP_TranscodeHandle     hTranscode
    );

#else /* !NEXUS_HAS_VIDEO_TRANSCODE */

/* Dummy stub definition for non-encode compilation. */
typedef struct BIP_Transcode *BIP_TranscodeHandle;

typedef struct BIP_TranscodeProfile
{
    BIP_SETTINGS(BIP_TranscodeProfile)                          /* Internal use... for init verification. */
    NEXUS_TransportType                 containerType;      /* Container type of the encoded stream */
} BIP_TranscodeProfile;

typedef struct BIP_TranscodeNexusHandles
{
    BIP_SETTINGS(BIP_TranscodeNexusHandles)                          /* Internal use... for init verification. */
    bool useSimpleHandles;
} BIP_TranscodeNexusHandles;

typedef struct BIP_TranscodeCreateSettings
{
    unsigned dummy;
} BIP_TranscodeCreateSettings;
typedef struct BIP_TranscodeSettings
{
    unsigned dummy;
} BIP_TranscodeSettings;

typedef struct BIP_TranscodePrepareSettings
{
    unsigned dummy;
} BIP_TranscodePrepareSettings;

typedef struct BIP_TranscodeStartSettings
{
    unsigned dummy;
} BIP_TranscodeStartSettings;

typedef struct BIP_TranscodeStatus
{
    unsigned dummy;
} BIP_TranscodeStatus;

typedef struct BIP_TranscodeStreamInfo
{
    unsigned dummy;
}BIP_TranscodeStreamInfo;

BIP_TranscodeHandle BIP_Transcode_Create( const BIP_TranscodeCreateSettings *pCreateSettings);
void BIP_Transcode_Destroy( BIP_TranscodeHandle hTranscode);
void BIP_Transcode_GetSettings( BIP_TranscodeHandle hTranscode, BIP_TranscodeSettings *pSettings);
BIP_Status BIP_Transcode_SetSettings( BIP_TranscodeHandle hTranscode, BIP_TranscodeSettings *pSettings);
BIP_Status BIP_Transcode_Prepare( BIP_TranscodeHandle hTranscode, BIP_TranscodeStreamInfo *pTranscodeStreamInfo, BIP_TranscodeProfile *pTranscodeProfile, NEXUS_RecpumpHandle hRecpump, bool nonRealTime, BIP_TranscodePrepareSettings *pSettings);
BIP_Status BIP_Transcode_Start( BIP_TranscodeHandle hTranscode, BIP_TranscodeStartSettings *pSettings);
BIP_Status BIP_Transcode_Stop( BIP_TranscodeHandle hTranscode );
void BIP_Transcode_PrintStatus( BIP_TranscodeHandle hTranscode );
#endif

#ifdef __cplusplus
}
#endif

#endif /* BIP_TRANSCODE_H */
