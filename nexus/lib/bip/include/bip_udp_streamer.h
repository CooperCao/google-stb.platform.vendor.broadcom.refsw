/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef BIP_UDP_STREAMER_H
#define BIP_UDP_STREAMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip.h"

typedef struct BIP_UdpStreamer *BIP_UdpStreamerHandle;

/**

Summary:
UDP Streamer Interface for streaming AV Media from an input such as File, Tuner, etc.

Description:
UDP Streamer Class is responsible for streaming out a media from a file, live tuner or buffered input.
Before BIP_UdpStreamer_Start() can be called, caller must provide detailed information about the input source &
how to output this media.

Streamer Input Settings are set via via BIP_UdpStreamer_Set[File|Tuner]InputSettings(). Caller must specifiy
Tracks to stream out using the BIP_UdpStreamer_AddTrack().

Streamer Output Settings (e.g. Streaming Protocol, Encryption Settings, etc.) are set via the BIP_UdpStreamer_SetOutputSettings().

Only after input & output settings are set, BIP_UdpStreamer_Start() can be called.

Caller can either call BIP_UdpStreamer_Stop() when it gets the endOfStreamingCallback or if the server is being Stopped.

*/

/**
Summary:
API to Create a UdpStreamer context.
*/
typedef struct BIP_UdpStreamerCreateSettings
{
    BIP_SETTINGS(BIP_UdpStreamerCreateSettings)   /* Internal use... for init verification. */

    /* NOTE: it's ok to define these callbacks in the createSettings as these callbacks are not invoked until Streamer is started! */
    BIP_CallbackDesc    endOfStreamingCallback;     /* Callback to indicate that streaming has finished for this context, */
                                                    /* can be issued when streaming is completed or streaming fails due to client channel change, */
                                                    /* or any network errors during streaming. */
    BIP_CallbackDesc    softErrorCallback;          /* Callback to let caller know about soft errors *not* due to streaming, */
                                                    /* e.g. no data to stream from live source, failure to read from the file, etc. */
} BIP_UdpStreamerCreateSettings;
BIP_SETTINGS_ID_DECLARE(BIP_UdpStreamerCreateSettings);

#define BIP_UdpStreamer_GetDefaultCreateSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_UdpStreamerCreateSettings) \
        /* Set non-zero defaults explicitly. */                                      \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_UdpStreamerHandle BIP_UdpStreamer_Create(
    const BIP_UdpStreamerCreateSettings *pSettings
    );

/**
Summary:
API to Get/Set Streamer runtime settings
*/
typedef struct BIP_UdpStreamerSettings
{
    BIP_StreamerSettings streamerSettings;
} BIP_UdpStreamerSettings;

void BIP_UdpStreamer_GetSettings(
    BIP_UdpStreamerHandle   hUdpStreamer,
    BIP_UdpStreamerSettings *pSettings
    );

BIP_Status BIP_UdpStreamer_SetSettings(
    BIP_UdpStreamerHandle   hUdpStreamer,
    BIP_UdpStreamerSettings *pSettings
    );

/**
Summary:
API to Set File Input specific settings to the Streamer.

Description:

See Also:
BIP_MediaInfoStream in bip_media_info.h
BIP_StreamerFileInputSettings in bip_streamer.h
BIP_Streamer_GetDefaultFileInputSettings
*/
BIP_Status BIP_UdpStreamer_SetFileInputSettings(
    BIP_UdpStreamerHandle           hUdpStreamer,
    const char                      *pMediaFileAbsolutePathName,        /* Media input source: Name of media file */
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerFileInputSettings   *pFileInputSettings
    );

/**
Summary:
API to Set Tuner Input specific settings to the Streamer.

Description:

See Also:
BIP_MediaInfoStream in bip_media_info.h
BIP_StreamerTunerInputSettings in bip_streamer.h
BIP_Streamer_GetDefaultTunerInputSettings
*/
BIP_Status BIP_UdpStreamer_SetTunerInputSettings(
    BIP_UdpStreamerHandle           hUdpStreamer,
    NEXUS_ParserBand                hParserBand,                /* ParserBand being used for the live channel */
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerTunerInputSettings  *pTunerInputSettings
    );

/**
Summary:
API to Set Recpump Input specific settings to the Streamer.

Description:
BIP Streamer will use the Recpump buffer as media input (instead of File or Tuner input) and stream it out.

Usage:
Apps can use this input type to setup usages that are not handled via currently supported
BIP Streamer inputs. E.g. If app wants to have a custom way to setup the AV pipe
(say for special transcode usage), then BIP will then simply stream out the the data
available in the Recpump fifo.

See Also:
BIP_StreamerRecpumpInputSettings in bip_streamer.h
BIP_Streamer_GetDefaultRecpumpInputSettings
*/
BIP_Status BIP_UdpStreamer_SetRecpumpInputSettings(
    BIP_UdpStreamerHandle               hUdpStreamer,
    NEXUS_RecpumpHandle                 hRecpump,
    BIP_StreamerRecpumpInputSettings    *pRecpumpInputSettings
    );

/**
Summary:
API to Add one Track to the Streamer.
Description:
BIP_UdpStreamer_AddTrack API is used to add a track number that needs to be streamed out.
Caller should invoke it multiple times once for each track.

It is useful to either specify individual tracks (within a program) of a MPTS fle stream
or in the Tuner case to choose specific set of tracks (within a program) among the multiple programs.
In addition, in Single Program case, it allows BIP Streamer to include track related info
(such as PIDS, Codecs, etc.) in the UDP Response itself.

See Also:
BIP_MediaInfoTrack in bip_media_info.h
BIP_StreamerTrackSettings in bip_streamer.h
BIP_Streamer_GetDefaultTrackSettings
*/

BIP_Status BIP_UdpStreamer_AddTrack(
    BIP_UdpStreamerHandle       hUdpStreamer,
    BIP_StreamerTrackInfo *pStreamerTrackInfo,
    BIP_StreamerTrackSettings   *pTrackSettings
    );

/**
Summary:
API to Set Streamer Output Settings.

See Also:
BIP_UdpStreamer_GetDefaultOutputSettings
**/
typedef enum BIP_UdpStreamerProtocol
{
    BIP_UdpStreamerProtocol_ePlainUdp,             /* Stream media directly over UDP w/o using any additional protocols on top of UDP */
    BIP_UdpStreamerProtocol_eRtp,                  /* Stream media using RTP over UDP */
    BIP_UdpStreamerProtocol_eMax
} BIP_UdpStreamerProtocol;

typedef struct BIP_UdpStreamerOutputSettings
{
    BIP_SETTINGS(BIP_UdpStreamerOutputSettings)              /* Internal use... for init verification. */

    struct BIP_StreamerOutputSettings   streamerSettings;
    BIP_AppInitialPayload               appInitialPayload;   /* Optional: app specific private header */
    NEXUS_HeapHandle                    heapHandle;          /* Optional: heap for fifo allocation */
    bool                                enableHwOffload;     /* Optional: enables offload to h/w like ASP if available on a platform & doable for a particular mediaInput stream format */
    bool                                enableDtcpIp;        /* Optional: To stream media using DTCP/IP, this should be set to true. */
    BIP_DtcpIpOutputSettings            dtcpIpOutput;        /* if enableDtcpIp is true, this structure should be filled-in with DTCP/IP output settings. */
} BIP_UdpStreamerOutputSettings;
BIP_SETTINGS_ID_DECLARE(BIP_UdpStreamerOutputSettings );

#define BIP_UdpStreamer_GetDefaultOutputSettings(pSettings)                       \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_UdpStreamerOutputSettings ) \
        /* Set non-zero defaults explicitly. */                                   \
        BIP_Streamer_GetDefaultOutputSettings(&(pSettings)->streamerSettings);    \
        BIP_SETTINGS_GET_DEFAULT_END


BIP_Status BIP_UdpStreamer_SetOutputSettings(
    BIP_UdpStreamerHandle          hUdpStreamer,
    BIP_UdpStreamerProtocol        streamerProtocol,
    const char                     *streamerIpAddress,
    const char                     *streamerPort,
    const char                     *streamerInterfaceName,
    BIP_UdpStreamerOutputSettings  *pOutputSettings
    );
/**
Summary:
API to optionally encode the streamer output.

See Also:
BIP_Transcode_GetDefaultProfile(profile)
BIP_Transcode_GetDefaultProfileFor_720p30_AVC_AAC_MPEG2_TS(profile)
**/
BIP_Status BIP_UdpStreamer_AddTranscodeProfile(
    BIP_UdpStreamerHandle       hUdpStreamer,
    BIP_TranscodeProfile        *pTranscodeProfile
    );

/**
Summary:
API to optionally set the Nexus Handles required for Transcode operation.

Description:
This is an optional API. If app doesn't call it, then BIP will internally open/acquire Nexus
Handles needed for the Transcode.

See Also:
BIP_Transcode_GetDefaultProfile(profile)
BIP_Transcode_GetDefaultProfileFor_720p30_AVC_AAC_MPEG2_TS(profile)
**/
BIP_Status BIP_UdpStreamer_SetTranscodeHandles(
    BIP_UdpStreamerHandle       hUdpStreamer,
    BIP_TranscodeNexusHandles   *pTranscodeNexusHandles
    );

/**
Summary:
API to start the streamer.

Description:
This API is used to start a streaming session.

Note: App must directly call the BIP_UdpStreamer_Set[File|Tuner|Ip|Bfile]InputSettings()
and BIP_UdpStreamer_SetOutputSettings() APIs before calling this BIP_UdpStreamer_Start API.

**/
typedef struct BIP_UdpStreamerStartSettings
{
    BIP_SETTINGS(BIP_UdpStreamerStartSettings) /* Internal use... for init verification. */

    BIP_DtcpIpServerHandle hInitDtcpIp;             /*!< in: optional DTCP/IP init handle returned by the DtcpAppLib_Startup(). */

    BIP_StreamingMethod     streamingMethod;
    struct
    {
        BIP_StreamingMethodRaveInterruptBasedSettings raveInterruptBasedSettings;
        BIP_StreamingMethodSystemTimerBasedSettings   systemTimerBasedSettings;
    } streamingSettings;
} BIP_UdpStreamerStartSettings;

BIP_SETTINGS_ID_DECLARE(BIP_UdpStreamerStartSettings);
/*  This always have to set irrespective of whether we are running in RaveInterruptbased or systemTimer based mode,
    since Rave interrupt internally is always enable only we don't wait for that event in systemTimer mode.
    Now in system timer mode since we are running based on systemTimer,
    so we can set dataReadyThreshold high which eventually reduce the number of interrupt. */
#define BIP_UdpStreamer_GetDefaultStartSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_UdpStreamerStartSettings) \
        /* Set non-zero defaults explicitly. */                                 \
        (pSettings)->streamingMethod = BIP_StreamingMethod_eSystemTimerBased;                  \
        (pSettings)->streamingSettings.raveInterruptBasedSettings.dataReadyScaleFactor = 10;   \
        (pSettings)->streamingSettings.raveInterruptBasedSettings.timeOutIntervalInMs = 10;    \
        (pSettings)->streamingSettings.systemTimerBasedSettings.timeOutIntervalInMs = 5;       \
         BIP_SETTINGS_GET_DEFAULT_END


BIP_Status BIP_UdpStreamer_Start(
    BIP_UdpStreamerHandle           hUdpStreamer,
    BIP_UdpStreamerStartSettings    *pSettings
    );

/**
Summary:
Stop UdpStreamer.

Description:
API to stop a streaming session.

Note: After stopping a BIP_UdpStreamer session, caller can re-use it and
Start a new Streamer session on it for the next BIP_UdpRequest.

**/
BIP_Status BIP_UdpStreamer_Stop(
    BIP_UdpStreamerHandle hUdpStreamer
    );

/**
Summary:
API to return the Udp Streamer status

Description:
**/
typedef struct BIP_UdpStreamerStats
{
    uint64_t    numBytesStreamed;           /* total bytesStreamed. */
} BIP_UdpStreamerStats;

typedef struct BIP_UdpStreamerStatus
{
    bool                    active;    /* Set to true if streaming is currently in progress on this streamer. */
    BIP_UdpStreamerStats    stats;
    BIP_StreamerStatus      streamerStatus;
} BIP_UdpStreamerStatus;

BIP_Status BIP_UdpStreamer_GetStatus(
    BIP_UdpStreamerHandle   hUdpStreamer,
    BIP_UdpStreamerStatus   *pStatus
    );

/**
Summary:
API to Destroy a UdpStreamer context
**/
void BIP_UdpStreamer_Destroy(
    BIP_UdpStreamerHandle hUdpStreamer
    );

#ifdef __cplusplus
}
#endif

#endif /* BIP_UDP_STREAMER_H */
