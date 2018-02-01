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
****************************************************************************/

#ifndef BIP_STREAMER_H
#define BIP_STREAMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_types.h"
#include "nexus_recpump.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"
#if NEXUS_HAS_HDMI_INPUT
#include "nexus_hdmi_input.h"
#endif
#include "nexus_file.h"
#include "bip.h"

/**
BIP_Streamer a base class that implements a generic streamer interface.

Protocol specific classes such as BIP_HttpStreamer and BIP_UdpStreamer are derived from this class.

Applications using BIP_HttpServer Interface does *NOT* directly use BIP_Streamer's APIs.
Instead, apps will fill-in the BIP_Streamer structures as defined via the BIP_HttpServer_HttpStreamer
or BIP_UdpStreamer Interfaces.

 **/
typedef struct BIP_Streamer *BIP_StreamerHandle;

typedef struct BIP_StreamerTrackInfo BIP_StreamerTrackInfo;
typedef struct BIP_StreamerStreamInfo BIP_StreamerStreamInfo;

/**
Summary:
API to Create Streamer context.
*/
typedef struct BIP_StreamerCreateSettings
{
    BIP_SETTINGS(BIP_StreamerCreateSettings)       /* Internal use... for init verification. */

    /* NOTE: it's ok to define these callbacks in the createSettings as these callbacks are not invoked until Streamer is started! */
    BIP_CallbackDesc    endOfStreamingCallback;     /* Callback to indicate that streaming has finished for this context, */
                                                    /* can be issued when streaming is completed or streaming fails due to client channel change, */
                                                    /* or any network errors during streaming. */
    BIP_CallbackDesc    softErrorCallback;          /* Callback to let caller know about soft errors *not* due to streaming, */
                                                    /* e.g. no data to stream from live source, failure to read from the file, etc. */
} BIP_StreamerCreateSettings;
BIP_SETTINGS_ID_DECLARE(BIP_StreamerCreateSettings);

#define BIP_Streamer_GetDefaultCreateSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_StreamerCreateSettings) \
        /* Set non-zero defaults explicitly. */                               \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_StreamerHandle BIP_Streamer_Create(
    const BIP_StreamerCreateSettings *pSettings
    );

/**
Summary:
API to Get/Set Streamer runtime settings
*/
typedef struct BIP_StreamerSettings
{
    BIP_TranscodeProfile        *pTranscodeProfile;      /*!< If set, this profile will be used to set/get the transcode settings. */
    bool                        seekPositionValid;       /*!< if set, seekPositionInMs is valid. */
    unsigned                    seekPositionInMs;        /*!< if seekPositionValid is set, file input will seek & start from to this time offset. */
} BIP_StreamerSettings;

void BIP_Streamer_GetSettings(
    BIP_StreamerHandle      hStreamer,
    BIP_StreamerSettings    *pSettings                   /*!< Note: caller must provide pointer to a TranscodeProfile structure if it wants the current TranscodeProfile settings. */
    );

BIP_Status BIP_Streamer_SetSettings(
    BIP_StreamerHandle      hStreamer,
    BIP_StreamerSettings    *pSettings
    );

/**
Summary:
Input sources from where to stream out.
*/
typedef enum BIP_StreamerInputType
{
    BIP_StreamerInputType_eFile,                   /* File input */
    BIP_StreamerInputType_eTuner,                  /* Input from traditional tuners like QAM, SAT, OFDM, etc. including Streamer input */
    BIP_StreamerInputType_eBfile,                  /* Bfile input (bfile_io_read_t interface) */
    BIP_StreamerInputType_eIp,                     /* Input from IP Tuner */
    BIP_StreamerInputType_eRecpump,                /* Input from Recpump directly, where media to stream out will be available (app responsible for this setup!) */
#if NEXUS_HAS_HDMI_INPUT
    BIP_StreamerInputType_eHdmi,                   /* Input from HDMI Input, streamer will encode the content to app specified transcoded profile. */
#endif
    BIP_StreamerInputType_eMax
} BIP_StreamerInputType;

/**
Summary:
API to Set File Input specific settings to the Streamer.
*/
typedef struct BIP_StreamerFileInputSettings
{
    BIP_SETTINGS(BIP_StreamerFileInputSettings)                     /* Internal use... for init verification. */
    off64_t                 beginByteOffset;                        /* Initial byte offset into the file from where to start streaming. */
    off64_t                 endByteOffset;                          /* End byte offset into the file where to end the streaming. if 0, whole file is sent from the beginByteOffset. */
    unsigned                beginTimeOffsetInMs;                    /* Initial time offset into the file from where to start streaming */
    unsigned                endTimeOffsetInMs;                      /* End time offset into the file where to end the streaming. if 0, whole file is sent. */
    const char              *playSpeed;                             /* PlaySpeed string, default value of NULL means "1" for normal play, "2" means 2X, 1/2 means slow forward at 1/2 rate, etc. */
                                                                    /* Requires NAV index to be specified in the BIP_MediaInfo_Track.info.video.n */
    bool                    enableHwPacing;                         /* If true, BIP will use File -> playpump -> recpump -> network path to enable h/w based pacing. */
                                                                    /* BIP will also internally choose this path for streaming a single program out of MPTS stream, network decryption, etc. scenarios. */
                                                                    /* NOTE: app must only set this flag for MPEG2 TS & PES type streams as Playback h/w only supports these formats. */
                                                                    /* NOTE2: BIP will configure transport h/w to pace using transport timestamps (TTS) if */
                                                                    /* BIP_StreamerStreamInfo.transportTimeStampEnabled is set or BIP_StreamerOutputSettings.enableTransportTimestamp is set. */
                                                                    /* Otherwise, BIP will program transport h/w to pace using PCRs. */
                                                                    /* Flag is ignored for any other such formats and stream is directly sent over the network path. */
    NEXUS_PlaypumpHandle    hPlaypump;                              /* Playpump handle: used for File -> playpump -> recpump -> network case, BIP will internally open one if not specified. */
    NEXUS_RecpumpHandle     hRecpump;                               /* Recpump handle: BIP will internally open one if not specified & is required for streaming! */
    NEXUS_RecpumpSettings   recpumpSettings;                        /* Recpump settings: to allow app to provide any specific settings */
    bool                    enableContinousPlay;                    /* Auto rewind when file reaches the end */
    bool                    enableAllPass;                          /* Allows app to send everything in the file. */
    bool                    dropNullPackets;                        /* If true and enableAllPass is true, NULL packets will be dropped. */
    unsigned                maxDataRate;                            /* Maximum data rate for the playback parser band in units of bits per second. */
                                                                    /* Default is typically 108000000 (i.e. 108 Mbps). If you increase this, */
                                                                    /* you need to analyze total transport bandwidth and overall system bandwidth. */
} BIP_StreamerFileInputSettings;
BIP_SETTINGS_ID_DECLARE(BIP_StreamerFileInputSettings);


typedef struct BIP_StreamerAudioTrackInfo
{
    NEXUS_AudioCodec codec;                     /*!< Audio codec */
}BIP_StreamerAudioTrackInfo;

typedef struct BIP_StreamerVideoTrackInfo
{
    NEXUS_VideoCodec codec;                         /*!< Video codec */
    uint16_t    width;                         /*!< Coded video width, or 0 if unknown */
    uint16_t    height;                        /*!< Coded video height, or 0 if unknown  */
    unsigned    colorDepth;                    /*!< For H265/HEVC video code, color depth: 8 --> 8 bits, 10 --> 10 bits, 0 for other Video Codecs. */
    const char  *pMediaNavFileAbsolutePathName;/*!< Media NAV file: set if NAV file is available for this Video track. */
}BIP_StreamerVideoTrackInfo;

/**
Summary:
This structure defines a track specific information for BIP_Streamer.
*/
struct BIP_StreamerTrackInfo
{
    BIP_MediaInfoTrackType type;                /*!< Type of track: Audio/Video/Pcr */
    unsigned               trackId;             /*!< Unique track ID (PID for MPEG2-TS, track_ID for ISOBMFF) */

    union {
        BIP_StreamerAudioTrackInfo audio;          /*!< Information for audio track */
        BIP_StreamerVideoTrackInfo video;          /*!< Information for video track */
    } info;
} ;

/**
Summary:
Populate a BIP_StreamerTrackInfo structure using data from a BIP_MediaInfoTrack structure
*/
void BIP_Streamer_GetStreamerTrackInfoFromMediaInfo(
    const BIP_MediaInfoTrack  *pMediaInfoTrack,
    BIP_StreamerTrackInfo     *pStreamerTrack
    );

/**
Summary:
This structure provides top level information about a stream (such as its container format type, number of trackGroups (programs) , number of tracks, and
other common info that applies across tracks) for BIP_Streamer.
*/
struct BIP_StreamerStreamInfo
{
    NEXUS_TransportType transportType;             /*!< Container format type */
    unsigned            numberOfTrackGroups;       /*!< Total number of trackGroups in the stream */
    unsigned            numberOfTracks;            /*!< Total number of tracks in the stream */
    unsigned            durationInMs;               /*!< Duration of stream in milliseconds or 0 if unknown */
    int64_t             contentLength;              /*!< Content length of the File, 0 if not known or for Live Channel */
    bool                transportTimeStampEnabled;  /*!< Indicates if MPEG2 TS content contains additional 4 byte timpstamp (192 byte Transport Packet) */
} ;

/**
Summary:
Populate a BIP_StreamerStreamInfo structure using data from a BIP_MediaInfoStream structure
*/
void BIP_Streamer_GetStreamerStreamInfoFromMediaInfo(
    const BIP_MediaInfoStream *pMediaInfoStream,
    BIP_StreamerStreamInfo    *pStreamerStreamInfo
    );


#define BIP_Streamer_GetDefaultFileInputSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_StreamerFileInputSettings) \
        /* Set non-zero defaults explicitly. */                                  \
        (pSettings)->maxDataRate = 108*1000*1000;                                \
        BIP_SETTINGS_GET_DEFAULT_END


BIP_Status BIP_Streamer_SetFileInputSettings(
    BIP_StreamerHandle              hStreamer,
    const char                      *pMediaFileAbsolutePathName,            /* Media input source: Name of media file */
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerFileInputSettings   *pFileInputSettings
    );

/**
Summary:
API to Set Tuner Input specific settings to the Streamer.
*/
typedef struct BIP_StreamerTunerInputSettings
{
    BIP_SETTINGS(BIP_StreamerTunerInputSettings)        /* Internal use... for init verification. */

    bool                            enableAllPass;      /* Allows app to send everything in the tuner input. */

    NEXUS_RecpumpHandle             hRecpump;          /* Recpump handle: BIP will internally open one if not specified & is required for streaming! */
    NEXUS_RecpumpSettings           recpumpSettings;   /* Recpump settings: to allow app to provide any specific settings */
} BIP_StreamerTunerInputSettings;
BIP_SETTINGS_ID_DECLARE(BIP_StreamerTunerInputSettings);

#define BIP_Streamer_GetDefaultTunerInputSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_StreamerTunerInputSettings) \
        /* Set non-zero defaults explicitly. */                                   \
        BIP_SETTINGS_GET_DEFAULT_END


BIP_Status BIP_Streamer_SetTunerInputSettings(
    BIP_StreamerHandle              hStreamer,
    NEXUS_ParserBand                hParserBand,                    /* ParserBand being used for the live channel */
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerTunerInputSettings  *pTunerInputSettings
    );

/**
Summary:
API to Set Recpump Input specific settings to the Streamer.

Description:
BIP Streamer will use the Recpump buffer as media input (instead of File or Tuner input)
and stream it out.
In this input mode, app responsible for setting up the how stream gets recorded to the recpump.

Usage:
Apps can use this input type to setup usages that are not handled via currently supported
BIP Streamer inputs. E.g. If app wants to have a custom way to setup the AV pipe
(say for special transcode usage), then BIP will then simply stream out the the data
available in the Recpump fifo.

*/
typedef struct BIP_StreamerRecpumpInputSettings
{
    BIP_SETTINGS(BIP_StreamerRecpumpInputSettings)   /* Internal use... for init verification. */

} BIP_StreamerRecpumpInputSettings;
BIP_SETTINGS_ID_DECLARE(BIP_StreamerRecpumpInputSettings);

#define BIP_Streamer_GetDefaultRecpumpInputSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_StreamerRecpumpInputSettings) \
        /* Set non-zero defaults explicitly. */                                     \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_Status BIP_Streamer_SetRecpumpInputSettings(
    BIP_StreamerHandle                  hStreamer,
    NEXUS_RecpumpHandle                 hRecpump,                   /* Required Recpump handle: App responsible for Opening it, Adding Pids to it. */
                                                                    /* BIP will start & stop the Recpump. */
    BIP_StreamerRecpumpInputSettings    *pRecpumpInputSettings
    );

/**
Summary:
API to Set Ip Input specific settings to the Streamer.
*/
typedef struct BIP_StreamerIpInputSettings
{
    BIP_SETTINGS(BIP_StreamerIpInputSettings)           /* Internal use... for init verification. */

    NEXUS_RecpumpHandle             hRecpump;           /* Recpump handle: BIP will internally open one if not specified & is required for streaming! */
    NEXUS_RecpumpSettings           recpumpSettings;    /* Recpump settings: to allow app to provide any specific settings */

    bool                            enableAllPass;      /* Allows app to send everything in the file. */
    bool                            dropNullPackets;    /* If true and enableAllPass is true, NULL packets will be dropped. */

    BIP_PlayerPrepareSettings       *pPrepareSettings;
    BIP_PlayerSettings              *pPlayerSettings;

} BIP_StreamerIpInputSettings;
BIP_SETTINGS_ID_DECLARE(BIP_StreamerIpInputSettings);

#define BIP_Streamer_GetDefaultIpInputSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_StreamerIpInputSettings) \
        /* Set non-zero defaults explicitly. */                                \
        /* TODO: get defaults for Player related settings. */                  \
        BIP_SETTINGS_GET_DEFAULT_END


BIP_Status BIP_Streamer_SetIpInputSettings(
    BIP_StreamerHandle              hStreamer,
    BIP_PlayerHandle                hPlayer,                    /* BIP Player instance that is associated w/ IP Input. */
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerIpInputSettings     *pIpInputSettings
    );

#if NEXUS_HAS_HDMI_INPUT
/**
Summary:
Input Settings when streaming from HDMI Input: App must setup the BIP_StreamerTranscodeProfile to provide the transcode settings.
*/
typedef struct BIP_StreamerHdmiInputSettings
{
    BIP_SETTINGS(BIP_StreamerHdmiInputSettings)   /* Internal use... for init verification. */
} BIP_StreamerHdmiInputSettings;
BIP_SETTINGS_ID_DECLARE(BIP_StreamerHdmiInputSettings);

#define BIP_Streamer_GetDefaultHdmiInputSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_StreamerHdmiInputSettings) \
        /* Set non-zero defaults explicitly. */                                \
        /* TODO: get defaults for Player related settings. */                  \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_Status BIP_Streamer_SetHdmiInputSettings(
    BIP_StreamerHandle              hStreamer,
    NEXUS_HdmiInputHandle           hHdmiInput,
    BIP_StreamerHdmiInputSettings   *pHdmiInputSettings
    );
#endif

/**
Summary:
API to Add one Track to the Streamer.

Description:
BIP_HttpStreamer_AddTrack API is used to add a track number that needs to be streamed out.
Caller should invoke it multiple times once for each track.

It is useful to either specify individual tracks (within a program) of a MPTS fle stream
or in the Tuner case to choose specific set of tracks (within a program) among the multiple programs.
In addition, in Single Program case, it allows BIP Streamer to include track related info
(such as PIDS, Codecs, etc.) in the HTTP Response itself.

*/
typedef struct BIP_StreamerTrackSettings
{
    BIP_SETTINGS(BIP_StreamerTrackSettings)   /* Internal use... for init verification. */

    NEXUS_PidChannelSettings            pidChannelSettings;
    NEXUS_RecpumpAddPidChannelSettings  recpumpPidChannelSettings;
} BIP_StreamerTrackSettings;
BIP_SETTINGS_ID_DECLARE(BIP_StreamerTrackSettings );

#define BIP_Streamer_GetDefaultTrackSettings(pSettings)                            \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_StreamerTrackSettings )      \
        /* Set non-zero defaults explicitly. */                                    \
        NEXUS_PidChannel_GetDefaultSettings( &(pSettings)->pidChannelSettings );   \
        NEXUS_Recpump_GetDefaultAddPidChannelSettings( &(pSettings)->recpumpPidChannelSettings ); \
        BIP_SETTINGS_GET_DEFAULT_END


BIP_Status BIP_Streamer_AddTrack(
    BIP_StreamerHandle          hStreamer,
    BIP_StreamerTrackInfo       *pStreamerTrackInfo,
    BIP_StreamerTrackSettings   *pTrackSettings
    );

/**
Summary:
Streamer Transport Layer Protocols.
*/
typedef enum BIP_StreamerProtocol
{
    BIP_StreamerProtocol_eTcp,
    BIP_StreamerProtocol_ePlainUdp,
    BIP_StreamerProtocol_eRtp,
    BIP_StreamerProtocol_eMax
} BIP_StreamerProtocol;

/**
Summary:
Streamer Output MPEG Transport Stream PAT/PMT modes.
*/
typedef enum BIP_StreamerMpeg2TsPatPmtMode
{
    BIP_StreamerMpeg2TsPatPmtMode_eAuto,                /* BIP will construct and insert PAT/PMT. */
    BIP_StreamerMpeg2TsPatPmtMode_ePassThruAsTracks,    /* App adds PAT/PMT tracks with BIP_<type>Streamer_AddTrack() */
    BIP_StreamerMpeg2TsPatPmtMode_eInsertCustom,        /* App provides custom PAT/PMT in BIP_StreamerOutputSettings */
    BIP_StreamerMpeg2TsPatPmtMode_eDisable,             /* Streamer output will have no PAT or PMT */
    BIP_StreamerMpeg2TsPatPmtMode_eMax
} BIP_StreamerMpeg2TsPatPmtMode;


/**
Summary:
Streamer Output specific settings.
*/
typedef struct BIP_StreamerOutputSettings
{
    BIP_SETTINGS(BIP_StreamerOutputSettings)    /* Internal use... for init verification. */

    struct   /* These fields are only used when output container type is NEXUS_TransportType_eTs */
    {
        bool            enableTransportTimestamp;   /* Optional: indicates if 4 byte timestamp should be inserted for MPEG2 TS output (making it 192 byte TS packet) */
                                                    /* if input stream has transportTimestampEnabled, */
                                                    /* BIP_StreamerFileInputSettings.enableHwPacing is NOT set, and */
                                                    /* output settings doesn't set enableTransportTimestamp, then 4 byte timestamp is removed from the output */

        BIP_StreamerMpeg2TsPatPmtMode   patPmtMode; /* Selects handling of PAT/PMT for streamer output */

        unsigned        patPmtIntervalInMs; /* Interval (in ms) between PAT/PMT transmissions. */

        const void     *customPatAddr;      /* Pointer to PAT to send when patPmtMode==eInsertCustom */
        size_t          customPatLength;    /* Length (in bytes) of customPat */

        const void     *customPmtAddr;      /* Pointer to PMT to send when patPmtMode==eInsertCustom */
        size_t          customPmtLength;    /* Length (in bytes) of customPmt */
    } mpeg2Ts;

    bool                enableHwOffload;    /* Optional: enables offload to h/w like ASP if available on a platform & doable for a particular mediaInput stream format */
    bool                enableStreamingUsingPlaybackCh;    /* Optional: enables streaming out using Nexus Playback -> Recpump -> PBIP Streaming Path. */
    unsigned            maxDataRate;        /* Maximum data rate for streaming out. */
} BIP_StreamerOutputSettings;
BIP_SETTINGS_ID_DECLARE(BIP_StreamerOutputSettings );

#define BIP_Streamer_GetDefaultOutputSettings(pSettings)                       \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_StreamerOutputSettings ) \
        /* Set non-zero defaults explicitly. */                                \
        (pSettings)->mpeg2Ts.patPmtMode = BIP_StreamerMpeg2TsPatPmtMode_eAuto; \
        (pSettings)->mpeg2Ts.patPmtIntervalInMs = 100;                         \
        (pSettings)->maxDataRate = 108*1000*1000;                              \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_Status BIP_Streamer_SetOutputSettings(
    BIP_StreamerHandle          hStreamer,
    BIP_StreamerProtocol        streamerProtocol,
    BIP_StreamerOutputSettings  *pOutputSettings
    );

/**
Summary:
API to optionally encode the streamer output.

See Also:
BIP_Transcode_GetDefaultProfile(profile)
BIP_Transcode_GetDefaultProfileFor_720p30_AVC_AAC_MPEG2_TS(profile)
**/
BIP_Status BIP_Streamer_AddTranscodeProfile(
    BIP_StreamerHandle          hStreamer,
    BIP_TranscodeProfile        *pTranscodeProfile
    );

/**
Summary:
API to optionally set the Nexus Handles required for Transcode operation.

Description:
This is an optional API. If app doesn't call it, then BIP will internally open/acquire Nexus
Handles needed for the Transcode.

See Also:
BIP_Transcode_GetDefaultNexusHandles
**/
BIP_Status BIP_Streamer_SetTranscodeHandles(
    BIP_StreamerHandle          hStreamer,
    BIP_TranscodeNexusHandles   *pTranscodeNexusHandles
    );


typedef enum BIP_StreamingMethod
{
    BIP_StreamingMethod_eRaveInterruptBased,  /*!< This indicates streaming will be driven by rave interrupts.*/
    BIP_StreamingMethod_eSystemTimerBased,    /*!< This indicates streaming will be driven by system timer at a periodic interval.*/
    BIP_LiveStreaminType_eMax
}BIP_StreamingMethod;

typedef struct BIP_StreamingMethodRaveInterruptBasedSettings
{
      unsigned dataReadyScaleFactor;            /*!< This will determine the recpump dataReadyThreshold.
                                                     For example if this is 16 , then dataReadyThreshold will be 16*recpumpOpenSettings.data.atomSize.
                                                     so if atomsize is 4096 then, 65Kbytes */

      unsigned timeOutIntervalInMs;             /*!< This specifies the timeOutInterval in milliseconds.
                                                     This specifies the worst case time to call the Data read
                                                     function when there is no callback from rave over this duration.
                                                     For http case we would like to set this to a higher value if we want to read bigger
                                                     chunk(which eventually will reduce the interrupt).
                                                     For example if read chunk size is 65Kbytes then for a 20Mbitsps stream we won't get any interrupt for approx 26.3 msec.
                                                     So this should be set at higher value than that.*/
} BIP_StreamingMethodRaveInterruptBasedSettings;


typedef struct BIP_StreamingMethodSystemTimerBasedSettings
{
    unsigned  timeOutIntervalInMs;              /*!< This specifies the duration after which data read call will be initiated periodically.
                                                     For udp case we found that data read at an interval of 5msec provides the best output.*/
}BIP_StreamingMethodSystemTimerBasedSettings;

/**
Summary:
API to prepare the streamer for streaming.

Description:
This API is used to prepare the streaming session. Streamer will internally
acquire/open/start the resources required for streaming and provide a
BIP_SUCCESS or BIP_ERROR_* status back to the caller. A HttpStreamer
type caller can use it to send proper HTTP Response to the client.

Note: this API will setup and get the whole streaming pipe ready but
*NOT* start the streaming.  Caller must call BIP_Streamer_Start to start the actual streaming.

*/
typedef struct BIP_StreamerPrepareSettings
{
    BIP_SETTINGS(BIP_StreamerPrepareSettings)           /*!< Internal use... for init verification. */

    BIP_TranscodeProfile        *pTranscodeProfile;     /*!< Optional Transcode profile that Streamer should use. */
                                                        /*!< If Null (default value), then Streamer will use the profile added via the AddTranscodeProfile. */
                                                        /*!< Otherwise, caller like HttpStreamer can select a profile based on the client request (Adaptive Streaming case. */
    bool                        enableRaiIndex;         /*!< If set, RAI index will be enabled (aids in identifying Segment boundaries for Adaptive Streaming). */
    NEXUS_RecpumpOpenSettings   recpumpOpenSettings;    /*!< Recpump Open Settings: allows callers to tune the dataReadyThreasholds! */
    bool                        seekPositionValid;      /*!< if set, seekPositionInMs is valid. */
    unsigned                    seekPositionInMs;       /*!< if seekPositionValid is set, file input will seek & start from to this time offset. */
} BIP_StreamerPrepareSettings;
BIP_SETTINGS_ID_DECLARE(BIP_StreamerPrepareSettings );

#define BIP_Streamer_GetDefaultPrepareSettings(pSettings)                         \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_StreamerPrepareSettings )   \
        /* Set non-zero defaults explicitly. */                                   \
        NEXUS_Recpump_GetDefaultOpenSettings( &(pSettings)->recpumpOpenSettings); \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_Status BIP_Streamer_Prepare(
    BIP_StreamerHandle hStreamer,
    BIP_StreamerPrepareSettings *pSettings
    );

/**
Summary:
API to start Streamer.

Description:
This API is used to start a streaming session.

Note: User of BIP_Streamer must call the BIP_Streamer_Set[File|Tuner|Ip|Bfile]InputSettings()
and BIP_Streamer_SetOutputSettings(), BIP_Streamer_Prepare() APIs before calling this BIP_Streamer_Start.

*/
typedef struct BIP_StreamerStartSettings
{
    BIP_SETTINGS(BIP_StreamerStartSettings)   /* Internal use... for init verification. */

    /* TODO: Add Function pointer to send function */
    unsigned unused;

    BIP_StreamingMethod     streamingMethod;
    unsigned                timeOutIntervalInMs;    /*!< This specifies the timeOutInterval in milliseconds.
                                                       For RaveInterruptBasedStreaming : this specifies the worst case time to call the Data read
                                                       function when there is no callback from rave over this duration.
                                                       For http case we would like to set this to a higher value if we want to read bigger
                                                       chunk(which eventually will reduce the interrupt).
                                                       For example if read chunk size is 65Kbytes then for a 20Mbitsps stream we won't get any interrupt for approx 26.3 msec.
                                                       So this should be set at higher value than that.

                                                       For SystemTimerBasedStreaming : this specifies the duration after which data read call will be initiated periodically.
                                                       For udp case we found that data read at an interval of 5msec provides the best output.*/
} BIP_StreamerStartSettings;

BIP_SETTINGS_ID_DECLARE(BIP_StreamerStartSettings );

#define BIP_Streamer_GetDefaultStartSettings(pSettings)                         \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_StreamerStartSettings )   \
        /* Set non-zero defaults explicitly. */                                 \
        (pSettings)->streamingMethod = BIP_StreamingMethod_eRaveInterruptBased; \
        (pSettings)->timeOutIntervalInMs = 5;                                   \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_Status BIP_Streamer_Start(
    BIP_StreamerHandle hStreamer,
    BIP_StreamerStartSettings *pSettings
    );

/**
Summary:
API to Stop Streamer

Description:

This API allows caller to stop & reset streamer to its original state as if streamer was just created.
If caller runs into an error after having successfully called some BIP_Streamer APIs before BIP_Steamer_Start(),
it must call BIP_Streamer_Stop to reset the streamer state.

*/
BIP_Status BIP_Streamer_Stop(
    BIP_StreamerHandle hStreamer
    );

/**
Summary:
API to return Streamer status

Description:
*/
typedef struct BIP_StreamerStats
{
    unsigned    numTracksAdded;             /* total of tracks currently added. */
} BIP_StreamerStats;

typedef struct BIP_StreamerStatus
{
    BIP_StreamerStats   stats;
} BIP_StreamerStatus;

BIP_Status BIP_Streamer_GetStatus(
    BIP_StreamerHandle hStreamer,
    BIP_StreamerStatus *pStatus
    );

/**
Summary:
API to Destroy a Streamer context
*/
void BIP_Streamer_Destroy(
    BIP_StreamerHandle streamer
    );

/**
Summary:
API to Print Steamer Status
*/
void BIP_Streamer_PrintStatus(
    BIP_StreamerHandle hStreamer
    );

#ifdef __cplusplus
}
#endif

#endif /* BIP_STREAMER_H */
