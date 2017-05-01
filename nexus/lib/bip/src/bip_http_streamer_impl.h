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

#ifndef BIP_HTTP_STREAMER_IMPL_H
#define BIP_HTTP_STREAMER_IMPL_H

#include "bip_priv.h"
#include "bip_streamer_impl.h"

/*
 * States for HttpStreamer Object (main server object),
 *  states reflecting HttpStreamer APIs visiable to Apps: Create, Start, Stop of Server
 */
typedef enum BIP_HttpStreamerOutputState
{
    BIP_HttpStreamerOutputState_eNotSet,                /* Streamer Output is not set or cleared. */
    BIP_HttpStreamerOutputState_eSet,                   /* Streamer Output is set. */
    BIP_HttpStreamerOutputState_eMax
} BIP_HttpStreamerOutputState;

typedef enum BIP_HttpStreamerResponseHeadersState
{
    BIP_HttpStreamerResponseHeadersState_eNotSet,       /* Streamer ResponseHeaders are not set or cleared. */
    BIP_HttpStreamerResponseHeadersState_eSet,          /* Streamer ResponseHeaders are set. */
    BIP_HttpStreamerResponseHeadersState_eMax
} BIP_HttpStreamerResponseHeadersState;

typedef enum BIP_HttpStreamerHlsState
{
    BIP_HttpStreamerHlsState_eUninitialized = 0,            /* Initial state: object is just created but not fully initialzed. */
    BIP_HttpStreamerHlsState_eWaitingForMasterPlaylistReq,  /* Waiting for master playlist request: we are in this state after setup is complete. */
    BIP_HttpStreamerHlsState_eWaitingForMediaPlaylistReq,   /* Waiting for media playlist request: we are in this state after we have sent the master playlist. */
    BIP_HttpStreamerHlsState_eWaitingFor1stMediaSegmentReq, /* Waiting for media segment request: we are in this state after we have sent the media playlist. */
    BIP_HttpStreamerHlsState_eWaitingForNextMediaSegmentReq,/* Waiting for media segment request: we are in this state after we have sent the media playlist. */
                                                            /* Or we can also be in this state after eStreamingDone state as we are done sending a segment & notified HttpServer about it. */
    BIP_HttpStreamerHlsState_eWaitingForEndOfSegmentCallback,/* Waiting for EndOfSegment callback from the Streamer. */
    BIP_HttpStreamerHlsState_eStreaming,                    /* Streamer is now streaming the next media segment. */
    BIP_HttpStreamerHlsState_eStreamingDone,                /* Transitional state: Streamer is now done streaming current media segment and letting us know via the callback. */
    BIP_HttpStreamerHlsState_eWaitingForStopApi,            /* Streamer has issued endOfStreaming callback to App and is waiting for Stop(). We transition into this state */
                                                            /* from _eWaitingForNextMediaSegmentReq state where we dont receive anymore requests from the peer (client). */
    BIP_HttpStreamerHlsState_eMax
} BIP_HttpStreamerHlsState;

typedef enum BIP_HttpStreamerState
{
    BIP_HttpStreamerState_eUninitialized = 0,           /* Initial state: object is just created but not fully initialzed. */
    BIP_HttpStreamerState_eIdle,                        /* Idle state: streamer object is either just created or is just stopped. */
    BIP_HttpStreamerState_eSetupComplete,               /* Transitional state when HttpStreamer_Start is called before we switch to WaitingForProcessRequestApi. */
    BIP_HttpStreamerState_eWaitingForProcessRequestApi, /* Input, Output, ResponseHeaders are all prepared, now waiting for caller to call ProcessRequest(). */
    BIP_HttpStreamerState_eStreaming,                   /* Streamer is now streaming the media. */
    BIP_HttpStreamerState_eStreamingDone,               /* Transitional state: Streamer is now done streaming the media and letting us know via the callback. */
    BIP_HttpStreamerState_eWaitingForStopApi,           /* Streamer reached either EOF or an Error, notified endOfStreaming Callback to app and now waiting for Stop(). */
    BIP_HttpStreamerState_eMax
} BIP_HttpStreamerState;

typedef struct BIP_HttpStreamerRequestInfo
{
    BIP_HttpSocketHandle                    hHttpSocket;
    BIP_CallbackDesc                        requestProcessedCallback;
    BIP_HttpStreamerProcessRequestSettings  settings;
    bool                                    valid;
} BIP_HttpStreamerRequestInfo;

typedef struct BIP_HttpStreamer
{
    BDBG_OBJECT( BIP_HttpStreamer )

    BLST_Q_ENTRY(BIP_HttpStreamer)              httpStreamerListNext;                   /* next BIP_HttpServerSocket object that is in use */
    B_MutexHandle                               hStateMutex;                            /* Mutex to synchronize a API invocation with callback invocation */
    BIP_Status                                  completionStatus;

    BIP_StreamerHandle                          hStreamer;
    const BIP_Streamer                          *pStreamer;

    /* Cached Settings. */
    BIP_HttpStreamerCreateSettings              createSettings;
    BIP_HttpStreamerSettings                    settings;
    BIP_HttpStreamerStartSettings               startSettings;
    BIP_HttpStreamerProcessRequestSettings      processRequestSettings;

    batom_factory_t                             atomFactory;
    batom_pipe_t                                atomPipe;

    /* HttpStreamer & its sub-states. */
    BIP_HttpStreamerState                       state;                                  /* Main Streamer state. */

    BIP_CLASS_DEFINE_INSTANCE_VARS(BIP_HttpStreamer);                                   /* Per-instance data used by BIP_Class. */

#define BIP_HTTP_STREAMER_INACTIVITY_TIMER_POLL_INTERVAL_IN_MSEC 1000
    BIP_TimerHandle                             hInactivityTimer;                       /* Timer to periodically check the Inactivity status of the Streamer. */
    bool                                        inactivityTimerActive;
    B_Time                                      inactivityTimerStartTime;
    bool                                        inactivityTimerExpired;

    struct
    {
        BIP_HttpStreamerHlsState                state;                                  /* State for the Streamer using HLS output protocol. */
        BIP_TranscodeProfile                    *pCurrentTranscodeProfile;
        unsigned                                currentProfileIndex;
        unsigned                                currentSegmentSequenceNumber;
        unsigned                                numSegmentsStreamed;
        bool                                    resetStreaming;                         /* Set during Seek to indicate to PBIP to reset its streaming state. */
    } hls;
    struct
    {
        BIP_HttpStreamerOutputState             state;                                  /* Streamer sub-state for its output. */
        BIP_HttpStreamerProtocol                streamerProtocol;
        BIP_HttpStreamerOutputSettings          settings;
    } output;
    struct
    {
        BIP_HttpStreamerResponseHeadersState    state;                                  /* Streamer sub-state for Response Headers. */
        BIP_HttpResponseHandle                  hHttpResponse;                          /* HttpStreamer object to send out HttpResponse. */
    } response;
    struct
    {
        BIP_HttpSocketHandle                    hHttpSocket;
        BIP_CallbackDesc                        requestProcessedCallback;
        BIP_HttpStreamerProcessRequestSettings  settings;
    } processRequest;
    BIP_HttpStreamerRequestInfo                 pendingRequest;

    struct
    {
        B_PlaybackIpFileStreamingHandle         hFileStreamer;
        B_PlaybackIpLiveStreamingHandle         hLiveStreamer;
        BIP_CallbackDesc                        pbipEndOfStreamingCallback;
    } playbackIpState;
    int                                         currentStreamingFd;

    int64_t                                     totalBytesStreamed;
    B_Time                                      lastActivityTime;

    /* One Arb per API */
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_HttpStreamerSettings                *pSettings;
    } getSettingsApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_HttpStreamerSettings                *pSettings;
    } setSettingsApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        const char                              *pMediaFileAbsolutePathName;
        BIP_StreamerStreamInfo                  *pStreamerStreamInfo;
        BIP_StreamerFileInputSettings           *pFileInputSettings;
    } fileInputSettingsApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        NEXUS_ParserBand                        hParserBand;
        BIP_StreamerStreamInfo                  *pStreamerStreamInfo;
        BIP_StreamerTunerInputSettings          *pTunerInputSettings;
    } tunerInputSettingsApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_PlayerHandle                        hPlayer;
        BIP_StreamerStreamInfo                  *pStreamerStreamInfo;
        BIP_StreamerIpInputSettings             *pIpInputSettings;
    } ipInputSettingsApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        NEXUS_RecpumpHandle                     hRecpump;
        BIP_StreamerRecpumpInputSettings        *pRecpumpInputSettings;
    } recpumpInputSettingsApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        unsigned                                programIndex;
    } setProgramApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_StreamerTrackInfo                   *pStreamerTrackInfo;
        BIP_StreamerTrackSettings               *pTrackSettings;
    } addTrackApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_HttpStreamerProtocol                streamerProtocol;
        BIP_HttpStreamerOutputSettings          *pOutputSettings;
    } outputSettingsApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_TranscodeProfile                    *pTranscodeProfile;
    } addTranscodeProfileApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_TranscodeNexusHandles               *pTranscodeNexusHandles;
    } setTranscodeNexusHandlesApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_HttpStreamerHandle                  hHttpStreamer;
        const char                              *pHeaderName;
        const char                              **pHeaderValue;
    } getResponseHeaderApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_HttpStreamerHandle                  hHttpStreamer;
        const char                              *pHeaderName;
        const char                              *pHeaderValue;
    } setResponseHeaderApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_HttpStreamerStartSettings           *pSettings;
    } startApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_HttpSocketHandle                    hHttpSocket;
        BIP_CallbackDesc                        *pRequestProcessedCallback;
        BIP_HttpStreamerProcessRequestSettings  *pSettings;
    } processRequestApi;
    struct
    {
        BIP_ArbHandle                           hArb;
    } stopApi;
    struct
    {
        BIP_ArbHandle                           hArb;
        BIP_HttpStreamerStatus                  *pStatus;
    } getStatusApi;
    struct
    {
        BIP_ArbHandle                           hArb;
    } printStatusApi;
    struct
    {
        BIP_ArbHandle                           hArb;
    } destroyApi;

    BIP_HttpStreamerStats                       stats;

#ifdef NEXUS_HAS_ASP
    B_AspChannelHandle                          hAspChannel;
#endif
} BIP_HttpStreamer;

#define BIP_HTTP_STREAMER_PRINTF_FMT  \
    "[hHttpStreamer=%p hStreamer=%p State=%s Proto=%s sockFd=%d dtcpIp=%s endOfStrmCB=%s]"

#define BIP_HTTP_STREAMER_PRINTF_ARG(pObj)                                                              \
    (void *)(pObj),                                                                                     \
    (void *)(pObj->hStreamer),                                                                          \
    (pObj)->state==BIP_HttpStreamerState_eIdle                  ? "Idle"                            :   \
    (pObj)->state==BIP_HttpStreamerState_eSetupComplete         ? "SetupComplete"                   :   \
    (pObj)->state==BIP_HttpStreamerState_eWaitingForProcessRequestApi ? "WaitingForProcessReqApi"   :   \
    (pObj)->state==BIP_HttpStreamerState_eStreaming             ? "Streaming"                       :   \
    (pObj)->state==BIP_HttpStreamerState_eStreamingDone         ? "StreamingDone"                   :   \
    (pObj)->state==BIP_HttpStreamerState_eWaitingForStopApi     ? "WaitingForStopApi"               :   \
                                                                  "StateNotMapped",                     \
                                                                                                        \
                                                                                                        \
    (pObj)->output.streamerProtocol==BIP_HttpStreamerProtocol_eDirect   ? "Direct"   :                  \
    (pObj)->output.streamerProtocol==BIP_HttpStreamerProtocol_eHls      ? "Hls"      :                  \
    (pObj)->output.streamerProtocol==BIP_HttpStreamerProtocol_eMpegDash ? "MpegDash" :                  \
                                                                                  "<undefined>",        \
    (pObj)->currentStreamingFd,                                                                         \
    (pObj)->output.settings.enableDtcpIp ?                          "Y" : "N",                          \
    (pObj)->pStreamer->createSettings.endOfStreamingCallback.callback  ?  "Y" : "N"

#endif /* BIP_HTTP_STREAMER_IMPL_H */
