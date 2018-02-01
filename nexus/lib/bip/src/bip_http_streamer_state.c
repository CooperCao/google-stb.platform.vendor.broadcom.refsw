/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "b_os_lib.h"
#include "bip_priv.h"
#include "bip_http_socket_impl.h"
#include "bip_http_streamer_impl.h"
#include "bip_streamer_priv.h"
#include "b_playback_ip_lib.h"
#include "namevalue.h"
#ifdef NEXUS_HAS_ASP
#include "b_asp_lib.h"
#endif


BDBG_MODULE( bip_http_streamer );
BDBG_OBJECT_ID_DECLARE( BIP_HttpStreamer );

BIP_CLASS_DECLARE(BIP_HttpStreamer);

struct BIP_HttpStreamerHlsStateNames
{
    BIP_HttpStreamerHlsState state;
    char *pStateName;
}gHttpStreamerHlsState[] = {
    {BIP_HttpStreamerHlsState_eUninitialized, "UnInitialized"},
    {BIP_HttpStreamerHlsState_eWaitingForMasterPlaylistReq, "WaitingForMasterPlaylistReq"},
    {BIP_HttpStreamerHlsState_eWaitingForMediaPlaylistReq,  "WaitingForMediaPlaylistReq"},
    {BIP_HttpStreamerHlsState_eWaitingFor1stMediaSegmentReq,"WaitingFor1stMediaSegmentReq"},
    {BIP_HttpStreamerHlsState_eWaitingForNextMediaSegmentReq,"WaitingForNextMediaSegmentReq"},
    {BIP_HttpStreamerHlsState_eWaitingForEndOfSegmentCallback,"WaitingForEndOfSegmentCallback"},
    {BIP_HttpStreamerHlsState_eStreaming, "Streaming"},
    {BIP_HttpStreamerHlsState_eStreamingDone, "StreamingDone"},    /* transitional state */
    {BIP_HttpStreamerHlsState_eWaitingForStopApi, "WaitingForStopApi"},
    {BIP_HttpStreamerHlsState_eMax, "MaxState"}
};
#define BIP_HTTP_STREAMER_HLS_STATE(state) \
    gHttpStreamerHlsState[state].pStateName

struct BIP_HttpStreamerStateNames
{
    BIP_HttpStreamerState state;
    char *pStateName;
}gHttpStreamerState[] = {
    {BIP_HttpStreamerState_eUninitialized,                  "UnInitialized"},
    {BIP_HttpStreamerState_eIdle,                           "Idle"},
    {BIP_HttpStreamerState_eSetupComplete,                  "SetupComplete"},    /* transitional state */
    {BIP_HttpStreamerState_eWaitingForProcessRequestApi,    "WaitingForProcessRequestApi"},
    {BIP_HttpStreamerState_eWaitingForClientAke,            "WaitingForClientAke"},
    {BIP_HttpStreamerState_eStartStreaming,                 "StartStreaming"},
    {BIP_HttpStreamerState_eStreaming,                      "Streaming"},
    {BIP_HttpStreamerState_eStreamingDone,                  "StreamingDone"},    /* transitional state */
    {BIP_HttpStreamerState_eWaitingForStopApi,              "WaitingForStopApi"},
    {BIP_HttpStreamerState_eMax,                            "MaxState"}
};
#define BIP_HTTP_STREAMER_STATE(state) \
    gHttpStreamerState[state].pStateName

static void httpStreamerPrintStatus(
    BIP_HttpStreamerHandle hHttpStreamer
    )
{
#ifdef NEXUS_HAS_ASP
    if (hHttpStreamer->hAspChannel)
    {
        B_AspChannel_PrintStatus(hHttpStreamer->hAspChannel);
    }
#endif

    if (hHttpStreamer->playbackIpState.hFileStreamer)
    {
        B_PlaybackIpFileStreamingStatus status;

        B_PlaybackIp_FileStreamingGetStatus( hHttpStreamer->playbackIpState.hFileStreamer, &status );
        BDBG_WRN(("PBIP FileStreaming[sockFd=%d]: state=%s bytesStreamed=%"PRId64 ,
                    hHttpStreamer->currentStreamingFd,
                    status.connectionState == B_PlaybackIpConnectionState_eActive  ? "Streaming":
                    status.connectionState == B_PlaybackIpConnectionState_eTimeout ? "Timedout":
                    status.connectionState == B_PlaybackIpConnectionState_eError   ? "Error":
                    status.connectionState == B_PlaybackIpConnectionState_eEof     ? "EOF":
                    status.connectionState == B_PlaybackIpConnectionState_eSetup   ? "Setup":
                                                                                     "Other",
                    status.bytesStreamed));
    }
    if (hHttpStreamer->playbackIpState.hLiveStreamer)
    {
        B_PlaybackIpLiveStreamingStatus status;

        B_PlaybackIp_LiveStreamingGetStatus( hHttpStreamer->playbackIpState.hLiveStreamer, &status );
        BDBG_WRN(("PBIP LiveStreaming[fd=%d]: state=%s bytesStreamed=%"PRId64 ,
                    hHttpStreamer->currentStreamingFd,
                    status.connectionState == B_PlaybackIpConnectionState_eActive  ? "Streaming":
                    status.connectionState == B_PlaybackIpConnectionState_eTimeout ? "Timedout":
                    status.connectionState == B_PlaybackIpConnectionState_eError   ? "Error":
                    status.connectionState == B_PlaybackIpConnectionState_eEof     ? "EOF":
                    status.connectionState == B_PlaybackIpConnectionState_eSetup   ? "Setup":
                                                                                     "Other",
                    status.bytesStreamed));
    }

} /* httpStreamerPrintStatus */

static int convertPlaySpeedToInt(
    const char *playSpeed )
{
    int playSpeedInt = 1;
    if (playSpeed && *playSpeed != '\0')
    {
        /*
         * Note: even though app may have specified playSpeed string to
         * contain numerator & denominator (for slow-fwd or slow rwd) cases,
         * we just use the atoi to convert this to an int. This is becacause
         * we always send these slow speeds as 1x and let the client use its
         * STC to achieve the desired slow speed.
         */
        playSpeedInt = atoi(playSpeed);
        if (playSpeedInt == 0) /* something went wrong in the coversion, treat it as 1x */
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "app provided playSpeed: %s, atoi was 0, treating it as 1x" BIP_MSG_PRE_ARG, playSpeed ));
            playSpeedInt = 1;
        }
    }
    else
    {
        playSpeedInt = 1;
    }

    return (playSpeedInt);
} /* convertPlaySpeedToInt */

static void resetHttpStreamerResponseState(
    BIP_HttpStreamerHandle hHttpStreamer
    )
{
    BDBG_ASSERT(hHttpStreamer);

    BIP_HttpResponse_Clear( hHttpStreamer->response.hHttpResponse, NULL);
    hHttpStreamer->response.state = BIP_HttpStreamerResponseHeadersState_eNotSet;

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Done" BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
} /* resetHttpStreamerResponseState */

static void stopPBipStreamer(
    BIP_HttpStreamerHandle hHttpStreamer
    )
{
    if (hHttpStreamer->playbackIpState.hLiveStreamer)
    {
        B_PlaybackIp_LiveStreamingStop(hHttpStreamer->playbackIpState.hLiveStreamer);
        BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpStreamer %p: state %s: PBIP based live streamer is stopped"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
    }
    else if (hHttpStreamer->playbackIpState.hFileStreamer)
    {
        B_PlaybackIp_FileStreamingStop(hHttpStreamer->playbackIpState.hFileStreamer);
        BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpStreamer %p: state %s: PBIP based file streamer is stopped"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
    }
} /* stopPBipStreamer */

static void destroyPBipStreamer(
    BIP_HttpStreamerHandle hHttpStreamer
    )
{
    if (hHttpStreamer->playbackIpState.hLiveStreamer)
    {
        B_PlaybackIp_LiveStreamingClose(hHttpStreamer->playbackIpState.hLiveStreamer);
        hHttpStreamer->playbackIpState.hLiveStreamer = NULL;
        BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpStreamer %p: state %s: PBIP based live streamer is Destroyed!"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
    }
    else if (hHttpStreamer->playbackIpState.hFileStreamer)
    {
        B_PlaybackIp_FileStreamingClose(hHttpStreamer->playbackIpState.hFileStreamer);
        hHttpStreamer->playbackIpState.hFileStreamer = NULL;
        BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpStreamer %p: state %s: PBIP based file streamer is Destroyed!"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
    }
} /* destroyPBipStreamer */

static void stopAndDestroyPBipStreamer(
    BIP_HttpStreamerHandle hHttpStreamer
    )
{
    stopPBipStreamer( hHttpStreamer );
    destroyPBipStreamer( hHttpStreamer );
} /* stopAndDestroyPBipStreamer */

void processHttpStreamerState( void *jObject, int value, BIP_Arb_ThreadOrigin threadOrigin );
static void playbackIpStreamerCallbackViaArbTimer(
    void *appCtx,
    int   param
    )
{
    BIP_Status             bipStatus;
    BIP_HttpStreamerHandle hHttpStreamer = appCtx;
    B_PlaybackIpEventIds eventId;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    if (bipStatus != BIP_SUCCESS)
    {
        return;
    }

    BDBG_ASSERT(hHttpStreamer);
    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer );
    eventId = param;

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer: %p: Running Deferred Callback: eventId=%d" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, eventId ));

    B_Mutex_Lock( hHttpStreamer->hStateMutex );
    /* Change state to streaming done only if we are in streaming state. */
    if ( hHttpStreamer->state == BIP_HttpStreamerState_eStreaming )
    {
        hHttpStreamer->state = BIP_HttpStreamerState_eStreamingDone;
        ++hHttpStreamer->hls.numSegmentsStreamed;
    }
    B_Mutex_Unlock( hHttpStreamer->hStateMutex );
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    processHttpStreamerState( (BIP_HttpStreamerHandle) hHttpStreamer, 0, BIP_Arb_ThreadOrigin_eTimer);
} /* playbackIpStreamerCallbackViaArbTimer */

static void playbackIpStreamerCallback(
    void *appCtx,
    B_PlaybackIpEventIds eventId
    )
{
    BIP_Status brc;
    BIP_HttpStreamerHandle hHttpStreamer = appCtx;

    brc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    if (brc != BIP_SUCCESS)
    {
        return;
    }
    BDBG_ASSERT(hHttpStreamer);
    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer:state %p: %s, got eventId %d from PBIP: Defer the callback"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), eventId ));

    B_Mutex_Lock( hHttpStreamer->hStateMutex );
    {
        hHttpStreamer->playbackIpState.pbipEndOfStreamingCallback.callback = &playbackIpStreamerCallbackViaArbTimer;
        hHttpStreamer->playbackIpState.pbipEndOfStreamingCallback.context = hHttpStreamer;
        hHttpStreamer->playbackIpState.pbipEndOfStreamingCallback.param = eventId;
        BIP_Arb_AddDeferredCallback( hHttpStreamer->startApi.hArb, &hHttpStreamer->playbackIpState.pbipEndOfStreamingCallback );

        brc = BIP_Arb_DoDeferred( hHttpStreamer->startApi.hArb, BIP_Arb_ThreadOrigin_eUnknown);
        BDBG_ASSERT( brc == BIP_SUCCESS );
    }
    B_Mutex_Unlock( hHttpStreamer->hStateMutex );
    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

} /* playbackIpStreamerCallback */

#ifdef NEXUS_HAS_ASP
static void aspLibCallbackViaArbCallback(
    void *appCtx,
    int eventId
    )
{
    BIP_Status brc;
    BIP_HttpStreamerHandle hHttpStreamer = appCtx;

    brc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    if(brc != BIP_SUCCESS)
    {
        return;
    }

    BDBG_ASSERT(hHttpStreamer);
    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer:state %p: %s, got eventId %d from PBIP: Defer the callback"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), eventId ));
    B_Mutex_Lock( hHttpStreamer->hStateMutex );
    /* Change state to streaming done only if we are in streaming state. */
    if ( hHttpStreamer->state == BIP_HttpStreamerState_eStreaming )
    {
        hHttpStreamer->state = BIP_HttpStreamerState_eStreamingDone;
        ++hHttpStreamer->hls.numSegmentsStreamed;
    }

    hHttpStreamer->playbackIpState.pbipEndOfStreamingCallback.callback = &playbackIpStreamerCallbackViaArbTimer;
    hHttpStreamer->playbackIpState.pbipEndOfStreamingCallback.context = hHttpStreamer;
    BIP_Arb_AddDeferredCallback( hHttpStreamer->startApi.hArb, &hHttpStreamer->playbackIpState.pbipEndOfStreamingCallback );
    B_Mutex_Unlock( hHttpStreamer->hStateMutex );

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);
    brc = BIP_Arb_DoDeferred( hHttpStreamer->startApi.hArb, BIP_Arb_ThreadOrigin_eUnknown);
    BDBG_ASSERT( brc == BIP_SUCCESS );

} /* aspLibCallbackViaArbCallback */

static void aspLibCallback(
    void *appCtx,
    int param
    )
{
    BIP_HttpStreamerHandle hHttpStreamer = appCtx;
    BIP_CallbackDesc callbackDesc;
    BIP_Status       brc;

    brc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    if (brc != BIP_SUCCESS)
    {
        return;
    }
    BDBG_ASSERT( hHttpStreamer );
    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Received callback from aspLib, state=%s" BIP_MSG_PRE_ARG,
                (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));

    callbackDesc.callback = &aspLibCallbackViaArbCallback;
    callbackDesc.context = hHttpStreamer;
    callbackDesc.param = param;
    BIP_Arb_AddDeferredCallback( NULL, &callbackDesc);

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    brc = BIP_Arb_DoDeferred( NULL, BIP_Arb_ThreadOrigin_eUnknown);
    BDBG_ASSERT( brc == BIP_SUCCESS );
} /* aspLibCallback */
#endif

static NEXUS_HeapHandle getStreamerHeapHandle(
   NEXUS_HeapHandle heapHandleFromSettings)
{
    NEXUS_HeapHandle selectedHeapHandle = heapHandleFromSettings;

#if NXCLIENT_SUPPORT
    if (selectedHeapHandle == NULL)
    {
        NEXUS_ClientConfiguration   clientConfig;
        unsigned        i;

        NEXUS_Platform_GetClientConfiguration(&clientConfig);
        for (i=0;i<NEXUS_MAX_HEAPS;i++)
        {
            NEXUS_MemoryStatus  status;
            NEXUS_Error         nrc;

            if (!clientConfig.heap[i]) continue;
            nrc = NEXUS_Heap_GetStatus(clientConfig.heap[i], &status);
            if (nrc == NEXUS_SUCCESS && status.memoryType == NEXUS_MemoryType_eFull) break;  /* Found it! */
        }
        BIP_CHECK_GOTO(( i < NEXUS_MAX_HEAPS ),
                       ( "Couldn't find NEXUS_MemoryType_eFull: i=%d, NEXUS_MAX_HEAPS=%d", i, NEXUS_MAX_HEAPS ), error, i, i );
        BDBG_MSG(( BIP_MSG_PRE_FMT " Using heapIndex %d for NEXUS_MemoryType_eFull" BIP_MSG_PRE_ARG, i));

        selectedHeapHandle = clientConfig.heap[i];
    }
error:
#endif /*  NXCLIENT_SUPPORT */

    return (selectedHeapHandle);
} /* getStreamerHeapHandle */

#if NEXUS_HAS_VIDEO_ENCODER
static BIP_Status resumePBipLiveStreamer(
    BIP_HttpStreamerHandle  hHttpStreamer,
    int                     socketFd
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    B_Error rc;

    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTRY ---> hHttpStreamer %p: state %s"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));

    /* Setup a PBIP LiveStreaming Session */
    {
        B_PlaybackIpLiveStreamingSettings liveStreamingSettings;

        memset( &liveStreamingSettings, 0, sizeof( liveStreamingSettings ));
        liveStreamingSettings.resumeStreaming = true;
        liveStreamingSettings.streamingEnabled = true;
        liveStreamingSettings.streamingFd = socketFd;
        liveStreamingSettings.resetStreaming = hHttpStreamer->hls.resetStreaming;
        hHttpStreamer->hls.resetStreaming = false;
        rc = B_PlaybackIp_LiveStreamingSetSettings( hHttpStreamer->playbackIpState.hLiveStreamer, &liveStreamingSettings );
        BIP_CHECK_GOTO(( !rc ), ( "B_PlaybackIp_LiveStreamingSetSettings Failed!" ), error, BIP_ERR_INTERNAL, bipStatus );

        bipStatus = BIP_SUCCESS;
    }

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--- hHttpStreamer %p: state %s, bipStatus 0x%x"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), bipStatus));

    return bipStatus;
} /* resumePBipLiveStreamer */

static BIP_Status resumePBipStreamer(
    BIP_HttpStreamerHandle  hHttpStreamer,
    int                     socketFd
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

#if 0
    /* TODO: File streaming currently doesn't need to support segmented streaming, so dont need this logic. */
    /* Determine whether Streamer should use Playback or Direct Network path for Streaming. */
    if ( hHttpStreamer->pStreamer->inputType == BIP_StreamerInputType_eFile && hHttpStreamer->pStreamer->file.feedUsingPlaybackChannel == false )
    {
        bipStatus = resumePBipFileStreamer( hHttpStreamer, socketFd );
    }
    else
#endif
    {
        bipStatus = resumePBipLiveStreamer( hHttpStreamer, socketFd );
    }

    return ( bipStatus );
} /* resumePBipStreamer */
#endif /* NEXUS_HAS_VIDEO_ENCODER */


static BIP_Status startPBipLiveStreamer(
    BIP_HttpStreamerHandle  hHttpStreamer,
    int                     socketFd
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    B_Error rc;

    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTRY ---> hHttpStreamer %p: state %s"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));

    /* Setup a PBIP LiveStreaming Session */
    {
        B_PlaybackIpLiveStreamingOpenSettings liveStreamingOpenSettings;

        memset( &liveStreamingOpenSettings, 0, sizeof( liveStreamingOpenSettings ));
        /* TODO: appPrivate header settings need to correctly copied from the Streamer object. */
        if (hHttpStreamer->output.settings.appInitialPayload.valid)
        {
            liveStreamingOpenSettings.appHeader.valid = true;
            liveStreamingOpenSettings.appHeader.length = hHttpStreamer->output.settings.appInitialPayload.length;
            memcpy( liveStreamingOpenSettings.appHeader.data, hHttpStreamer->output.settings.appInitialPayload.pPayload, sizeof(liveStreamingOpenSettings.appHeader.data));
        }

        /* Set Streaming Socket & Protocol. */
        {
            liveStreamingOpenSettings.streamingFd = socketFd;
            liveStreamingOpenSettings.dontCloseSocket = true;
            liveStreamingOpenSettings.protocol = B_PlaybackIpProtocol_eHttp;
            liveStreamingOpenSettings.hlsSession = hHttpStreamer->output.streamerProtocol == BIP_HttpStreamerProtocol_eDirect ? false:true;
        }

        /* Set Stream related info. */
        {
            liveStreamingOpenSettings.transportTimestampEnabled = hHttpStreamer->pStreamer->output.settings.mpeg2Ts.enableTransportTimestamp;
        }

        /* Set callback to know when the streaming is over. */
        {
            liveStreamingOpenSettings.eventCallback = playbackIpStreamerCallback;
            liveStreamingOpenSettings.appCtx = hHttpStreamer;
        }

        /* Set any Security related settings. */
        if (hHttpStreamer->output.settings.enableDtcpIp)
        {
            BIP_DtcpIpServerStatus dtcpIpServerStatus;

            BIP_DtcpIpServer_GetStatus( hHttpStreamer->startSettings.hInitDtcpIp, &dtcpIpServerStatus );

            liveStreamingOpenSettings.securitySettings.securityProtocol = B_PlaybackIpSecurityProtocol_DtcpIp;
            liveStreamingOpenSettings.securitySettings.enableEncryption = true;
            liveStreamingOpenSettings.securitySettings.initialSecurityContext = dtcpIpServerStatus.pDtcpIpLibCtx;
            liveStreamingOpenSettings.securitySettings.settings.dtcpIp.emiValue = hHttpStreamer->output.settings.dtcpIpOutput.copyControlInfo;
            liveStreamingOpenSettings.securitySettings.settings.dtcpIp.akeTimeoutInMs = hHttpStreamer->output.settings.dtcpIpOutput.akeTimeoutInMs;
            liveStreamingOpenSettings.securitySettings.settings.dtcpIp.pcpPayloadLengthInBytes = hHttpStreamer->output.settings.dtcpIpOutput.pcpPayloadLengthInBytes;
        }

        /* Set remaining settings. */
        {
            liveStreamingOpenSettings.recpumpHandle = hHttpStreamer->pStreamer->hRecpump;
            liveStreamingOpenSettings.heapHandle = getStreamerHeapHandle(hHttpStreamer->output.settings.heapHandle);
            liveStreamingOpenSettings.enableHttpChunkTransferEncoding = hHttpStreamer->output.settings.enableHttpChunkXferEncoding;
            if(hHttpStreamer->startSettings.streamingMethod == BIP_StreamingMethod_eRaveInterruptBased)
            {
                liveStreamingOpenSettings.streamingMethod = B_PlaybackIpStreamingMethod_eRaveInterruptBased;
                liveStreamingOpenSettings.timeOutIntervalInMs =
                    hHttpStreamer->startSettings.streamingSettings.raveInterruptBasedSettings.timeOutIntervalInMs;
            }
            else if(hHttpStreamer->startSettings.streamingMethod == BIP_StreamingMethod_eSystemTimerBased)
            {
                liveStreamingOpenSettings.streamingMethod = B_PlaybackIpStreamingMethod_eSystemTimerBased;
                liveStreamingOpenSettings.timeOutIntervalInMs =
                    hHttpStreamer->startSettings.streamingSettings.systemTimerBasedSettings.timeOutIntervalInMs;
            }
        }

        hHttpStreamer->playbackIpState.hLiveStreamer = B_PlaybackIp_LiveStreamingOpen( &liveStreamingOpenSettings );
        BIP_CHECK_GOTO(( hHttpStreamer->playbackIpState.hLiveStreamer ), ( "B_PlaybackIp_LiveStreamingOpen Failed!" ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    /* Start PBIP Streamer */
    {
        rc = B_PlaybackIp_LiveStreamingStart( hHttpStreamer->playbackIpState.hLiveStreamer );
        BIP_CHECK_GOTO(( !rc ), ( "B_PlaybackIp_LiveStreamingStart Failed!" ), error, BIP_ERR_INTERNAL, bipStatus );
        bipStatus = BIP_SUCCESS;
    }

error:
    if (bipStatus != BIP_SUCCESS)
    {
        stopAndDestroyPBipStreamer( hHttpStreamer );
    }
    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--- hHttpStreamer=%p: state=%s socketFd=%d bipStatus=%s"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), socketFd, BIP_StatusGetText(bipStatus) ));

    return bipStatus;
} /* startPBipLiveStreamer */

static BIP_Status startPBipFileStreamer(
    BIP_HttpStreamerHandle  hHttpStreamer,
    int                     socketFd
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    B_PlaybackIpError playbackIpError;

    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTRY ---> hHttpStreamer %p: state %s"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));

    /* Open PBIP FileStreaming Context. */
    {
        B_PlaybackIpFileStreamingOpenSettings fileStreamingOpenSettings;

        BDBG_MSG(( BIP_MSG_PRE_FMT "FileInputSettings: Stream %s, streamingFd %d, PlaySpeed %d, Time Offsets: %u - %u, ByteOffsets: %"PRIu64 " - %"PRIu64 BIP_MSG_PRE_ARG,
                    BIP_String_GetString( hHttpStreamer->pStreamer->file.hMediaFileAbsolutePathName ), socketFd,
                    convertPlaySpeedToInt(BIP_String_GetString(hHttpStreamer->pStreamer->file.hPlaySpeed)),
                    hHttpStreamer->pStreamer->file.inputSettings.beginTimeOffsetInMs, hHttpStreamer->pStreamer->file.inputSettings.endTimeOffsetInMs,
                    hHttpStreamer->pStreamer->file.inputSettings.beginByteOffset, hHttpStreamer->pStreamer->file.inputSettings.endByteOffset
                 ));
        B_Os_Memset( &fileStreamingOpenSettings, 0, sizeof( fileStreamingOpenSettings ));
        /* Set Streaming Socket & Protocol. */
        {
            fileStreamingOpenSettings.streamingFd = socketFd;
            fileStreamingOpenSettings.protocol = B_PlaybackIpProtocol_eHttp;
        }

        /* Set Byte/Time Range & Trickmode related options. */
        {
            if (hHttpStreamer->pStreamer->file.inputSettings.beginByteOffset)
            {
                fileStreamingOpenSettings.beginFileOffset = hHttpStreamer->pStreamer->file.inputSettings.beginByteOffset;
            }
            if ( hHttpStreamer->pStreamer->file.inputSettings.endByteOffset )
            {
                fileStreamingOpenSettings.endFileOffset = hHttpStreamer->pStreamer->file.inputSettings.endByteOffset;
            }
            if (hHttpStreamer->pStreamer->file.inputSettings.beginTimeOffsetInMs)
            {
                fileStreamingOpenSettings.beginTimeOffset = hHttpStreamer->pStreamer->file.inputSettings.beginTimeOffsetInMs/1000.;
            }
            if (hHttpStreamer->pStreamer->file.inputSettings.endTimeOffsetInMs)
            {
                fileStreamingOpenSettings.endTimeOffset = hHttpStreamer->pStreamer->file.inputSettings.endTimeOffsetInMs/1000.;
            }
            fileStreamingOpenSettings.playSpeed = convertPlaySpeedToInt(BIP_String_GetString(hHttpStreamer->pStreamer->file.hPlaySpeed));
        }

        /* Set Stream related info. */
        {
            fileStreamingOpenSettings.disableIndexGeneration = true;
            fileStreamingOpenSettings.disableHlsPlaylistGeneration = true;
            fileStreamingOpenSettings.autoRewind = hHttpStreamer->pStreamer->file.inputSettings.enableContinousPlay;
            strncpy( fileStreamingOpenSettings.fileName,
                    BIP_String_GetString( hHttpStreamer->pStreamer->file.hMediaFileAbsolutePathName ),
                    sizeof(fileStreamingOpenSettings.mediaInfoFilesDir)-1
                   );
            fileStreamingOpenSettings.transportTimestampEnabled = hHttpStreamer->pStreamer->streamerStreamInfo.transportTimeStampEnabled;
        }

        /* Set callback to know when the streaming is over. */
        {
            fileStreamingOpenSettings.eventCallback = playbackIpStreamerCallback;
            fileStreamingOpenSettings.appCtx = hHttpStreamer;
        }

        /* Set any Security related settings. */
        if (hHttpStreamer->output.settings.enableDtcpIp)
        {
            BIP_DtcpIpServerStatus dtcpIpServerStatus;

            BIP_DtcpIpServer_GetStatus( hHttpStreamer->startSettings.hInitDtcpIp, &dtcpIpServerStatus );
            fileStreamingOpenSettings.securitySettings.securityProtocol = B_PlaybackIpSecurityProtocol_DtcpIp;
            fileStreamingOpenSettings.securitySettings.enableEncryption = true;
            fileStreamingOpenSettings.securitySettings.initialSecurityContext = dtcpIpServerStatus.pDtcpIpLibCtx;
            fileStreamingOpenSettings.securitySettings.settings.dtcpIp.emiValue = hHttpStreamer->output.settings.dtcpIpOutput.copyControlInfo;
            fileStreamingOpenSettings.securitySettings.settings.dtcpIp.akeTimeoutInMs = hHttpStreamer->output.settings.dtcpIpOutput.akeTimeoutInMs;
            fileStreamingOpenSettings.securitySettings.settings.dtcpIp.pcpPayloadLengthInBytes = hHttpStreamer->output.settings.dtcpIpOutput.pcpPayloadLengthInBytes;
            BDBG_MSG(( BIP_MSG_PRE_FMT "DTCP/IP Encryption is enabled: PCP Size %d" BIP_MSG_PRE_ARG, hHttpStreamer->output.settings.dtcpIpOutput.pcpPayloadLengthInBytes ));
        }

        /* Set remaining settings. */
        {
            fileStreamingOpenSettings.heapHandle = getStreamerHeapHandle(hHttpStreamer->output.settings.heapHandle);
            fileStreamingOpenSettings.enableHttpChunkTransferEncoding = hHttpStreamer->output.settings.enableHttpChunkXferEncoding;
        }

        /* Open the PBIP FileStreaming Handle */
        {
            hHttpStreamer->playbackIpState.hFileStreamer = B_PlaybackIp_FileStreamingOpen(&fileStreamingOpenSettings);
            BIP_CHECK_GOTO(( hHttpStreamer->playbackIpState.hFileStreamer ), ( "hHttpStreamer %p, state %s: B_PlaybackIp_FileStreamingOpen() Failed",
                        (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ), error, BIP_ERR_INTERNAL, bipStatus );
        }
    }

    /* For TS formats, we need to open the NAV indexer file. */
    {
        const char *navFile = NULL;
        BIP_StreamerTrackListEntryHandle hTrackEntry = NULL;

        if (hHttpStreamer->pStreamer->streamerStreamInfo.transportType == NEXUS_TransportType_eTs &&
                BIP_Streamer_GetTrackEntry_priv( hHttpStreamer->hStreamer, BIP_MediaInfoTrackType_eVideo, &hTrackEntry ) == BIP_SUCCESS &&
                hTrackEntry->hMediaNavFileAbsolutePathName )
        {
            bool rc;
            /* coverity[stack_use_local_overflow] */
            /* coverity[stack_use_overflow] */
            B_PlaybackIpPsiInfo psiInfo;

            navFile = BIP_String_GetString( hTrackEntry->hMediaNavFileAbsolutePathName );
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: nag file: %s" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, navFile ));

            /* Now open the indexer */
            memset(&psiInfo, 0, sizeof(B_PlaybackIpPsiInfo));
            psiInfo.mpegType = hHttpStreamer->pStreamer->streamerStreamInfo.transportType;
            psiInfo.transportTimeStampEnabled = hHttpStreamer->pStreamer->streamerStreamInfo.transportTimeStampEnabled;
            psiInfo.videoCodec = hTrackEntry->streamerTrackInfo.info.video.codec;
            psiInfo.videoPid = hTrackEntry->streamerTrackInfo.trackId;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Using NAV File %s" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, navFile ));

            rc =  openMediaIndexer(hHttpStreamer->playbackIpState.hFileStreamer, navFile, &psiInfo);
            BIP_CHECK_GOTO(( rc == true ), ( "hHttpStreamer %p, state %s: PBIP openMediaIndex() Failed",
                        (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ), error, BIP_ERR_INTERNAL, bipStatus );
        }
    }

    /* Start the PBIP Streamer */
    {
        playbackIpError = B_PlaybackIp_FileStreamingStart(hHttpStreamer->playbackIpState.hFileStreamer);
        BIP_CHECK_GOTO(( playbackIpError == B_ERROR_SUCCESS ), ( "hHttpStreamer %p, state %s: B_PlaybackIp_FileStreamingStart() Failed",
                    (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ), error, BIP_ERR_INTERNAL, bipStatus );

        bipStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Streaming from File Src is successfully setup using PBIP (socketFd %d)"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, socketFd));
    }

error:
    if (bipStatus != BIP_SUCCESS)
    {
        stopAndDestroyPBipStreamer( hHttpStreamer );
    }
    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--- hHttpStreamer %p: state %s, bipStatus 0x%x"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), bipStatus));

    return bipStatus;
} /* startPBipFileStreamer */

static BIP_Status createAndStartPBipStreamer(
    BIP_HttpStreamerHandle hHttpStreamer,
    int                     socketFd
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    /* Determine whether Streamer should use Playback or Direct Network path for Streaming. */
    if ( hHttpStreamer->pStreamer->inputType == BIP_StreamerInputType_eFile && hHttpStreamer->pStreamer->file.feedUsingPlaybackChannel == false )
    {
        bipStatus = startPBipFileStreamer( hHttpStreamer, socketFd );
    }
    else
    {
        bipStatus = startPBipLiveStreamer( hHttpStreamer, socketFd );
    }

    return ( bipStatus );
} /* createAndStartPBipStreamer */

#ifdef NEXUS_HAS_ASP
static BIP_Status createAndStartAspStreamer(
    BIP_HttpStreamerHandle hHttpStreamer,
    int                     socketFd
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_DtcpIpServerStreamStatus dtcpIpStreamStatus;
    B_Error rc;

    if ( hHttpStreamer->output.settings.enableDtcpIp )
    {

        /* Get DTCP/IP Key related info & pass it to ASP. */
        bipStatus = BIP_DtcpIpServer_OpenStream(hHttpStreamer->startSettings.hInitDtcpIp, hHttpStreamer->httpSocketStatus.pRemoteIpAddress, hHttpStreamer->output.settings.dtcpIpOutput.copyControlInfo, &hHttpStreamer->hDtcpIpServerStream);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_DtcpIpServer_OpenStream Failed" ), error, bipStatus, bipStatus );
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p: BIP_DtcpIpServer_OpenStream() hDtcpIpServerStream=%p..." BIP_MSG_PRE_ARG,
                    (void *)hHttpStreamer, (void *)hHttpStreamer->hDtcpIpServerStream ));

        bipStatus = BIP_DtcpIpServer_GetStreamStatus(hHttpStreamer->startSettings.hInitDtcpIp, hHttpStreamer->hDtcpIpServerStream, &dtcpIpStreamStatus);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_DtcpIpServer_GetStreamStatus() Failed" ), error, bipStatus, bipStatus );
    }

    {
        B_AspChannelCreateSettings createSettings;

        B_AspChannel_GetDefaultCreateSettings( B_AspStreamingProtocol_eHttp,  /* example assumes HTTP protocol based streaming. */
                                                  &createSettings);

        /* Setup HTTP Protocol related settings. */
        createSettings.protocol = B_AspStreamingProtocol_eHttp;
        createSettings.mode = B_AspStreamingMode_eOut;
        createSettings.modeSettings.streamOut.hRecpump = hHttpStreamer->pStreamer->hRecpump;   /* source of AV data. */
        createSettings.modeSettings.streamOut.hPlaypump = hHttpStreamer->pStreamer->hPlaypump;   /* source of AV data. */
        createSettings.mediaInfoSettings.transportType = hHttpStreamer->pStreamer->streamerStreamInfo.transportType;
        createSettings.mediaInfoSettings.maxBitRate = hHttpStreamer->output.settings.streamerSettings.maxDataRate;
        createSettings.protocolSettings.http.enableChunkTransferEncoding = hHttpStreamer->output.settings.enableHttpChunkXferEncoding;
        createSettings.protocolSettings.http.chunkSize = hHttpStreamer->output.settings.chunkSize;

        if ( hHttpStreamer->output.settings.enableDtcpIp )
        {
            createSettings.drmType = NEXUS_AspChannelDrmType_eDtcpIp;
        }
        /* Create ASP Channel. */
        hHttpStreamer->hAspChannel = B_AspChannel_Create(socketFd, &createSettings);
        BIP_CHECK_GOTO(( hHttpStreamer->hAspChannel ), ( "B_AspChannel_Create Failed" ), error, BIP_ERR_NEXUS, bipStatus );
    }

    /* Setup callbacks. */
    {
        B_AspChannelSettings settings;

        B_AspChannel_GetSettings(hHttpStreamer->hAspChannel, &settings);

        /* Setup a callback to notify state transitions indicating either network errors or EOF condition. */
        settings.stateChanged.context = hHttpStreamer;
        settings.stateChanged.callback = aspLibCallback;
        rc = B_AspChannel_SetSettings(hHttpStreamer->hAspChannel, &settings);
        BIP_CHECK_GOTO((rc == B_ERROR_SUCCESS), ( "B_AspChannel_SetSettings Failed" ), error, BIP_ERR_NEXUS, bipStatus );
    }

    if ( hHttpStreamer->output.settings.enableDtcpIp )
    {
        B_AspChannelDtcpIpSettings dtcpIpSettings;
        dtcpIpSettings.settings.exchKeyLabel = dtcpIpStreamStatus.exchKeyLabel;
        dtcpIpSettings.settings.emiModes = dtcpIpStreamStatus.emiModes;
        dtcpIpSettings.settings.pcpPayloadSize = hHttpStreamer->output.settings.dtcpIpOutput.pcpPayloadLengthInBytes;
        BKNI_Memcpy(dtcpIpSettings.settings.exchKey,  dtcpIpStreamStatus.exchKey, NEXUS_ASP_DTCP_IP_EXCH_KEY_IN_BYTES);
        BKNI_Memcpy(dtcpIpSettings.settings.initialNonce,  dtcpIpStreamStatus.initialNonce, NEXUS_ASP_DTCP_IP_NONCE_IN_BYTES);
        rc = B_AspChannel_SetDtcpIpSettings( hHttpStreamer->hAspChannel, &dtcpIpSettings);
        BIP_CHECK_GOTO((rc == B_ERROR_SUCCESS), ( "B_AspChannel_SetDtcpIpSettings Failed" ), error, BIP_ERR_NEXUS, bipStatus );
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p: Set DTCP/IP Exch Kay & Nonce settings w/ ASP Lib" BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
    }

    /* Start Streaming. */
    {
        rc = B_AspChannel_StartStreaming(hHttpStreamer->hAspChannel);
        BIP_CHECK_GOTO((rc == B_ERROR_SUCCESS), ( "B_AspChannel_SetSettings Failed" ), error, BIP_ERR_NEXUS, bipStatus );
    }
    BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpStreamer=%p: Started Streaming using ASP..." BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));

error:
    if (bipStatus != BIP_SUCCESS)
    {
        if (hHttpStreamer->hAspChannel) B_AspChannel_Destroy(hHttpStreamer->hAspChannel, NULL);
        hHttpStreamer->hAspChannel = NULL;
    }
    return ( bipStatus );
} /* createAndStartAspStreamer */

#else
static BIP_Status createAndStartAspStreamer(
    BIP_HttpStreamerHandle hHttpStreamer,
    int                     socketFd
    )
{
    BSTD_UNUSED(hHttpStreamer);
    BSTD_UNUSED(socketFd);
    BDBG_ERR(("NEXUS_ASP_SUPPORT is not enabled! ASP h/w is current only supported on 97278 platforms."));
    return (BIP_ERR_NOT_AVAILABLE);
} /* createAndStartAspStreamer */
#endif

static BIP_Status startAspOrPBipStreamer(
    BIP_HttpStreamerHandle hHttpStreamer,
    int                     socketFd
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;

    /* Determine whether Streamer should use Playback or Direct Network path for Streaming. */
    if ( hHttpStreamer->pStreamer->offloadStreamerToAsp )
    {
        bipStatus = createAndStartAspStreamer(hHttpStreamer, socketFd);
    }
    else
    {
        bipStatus = createAndStartPBipStreamer(hHttpStreamer, socketFd);
    }

    return ( bipStatus );
} /* startAspOrPBipStreamer */

static BIP_Status addContentTypeHeader(
    BIP_HttpStreamerHandle hHttpStreamer,
    BIP_TranscodeProfile *pTranscodeProfile,
    BIP_HttpResponseHandle hHttpResponse
    )
{
    BIP_Status bipStatus;
    const char *pContentTypeString = NULL;
    NEXUS_TransportType transportType;
    bool transportTimeStampEnabled = false;
    BIP_StringHandle hContentTypeValueBase = NULL;
    BIP_StringHandle hContentTypeValue = NULL;
    BIP_HttpHeaderHandle hHeader;

    if ( pTranscodeProfile )
    {
        transportType = pTranscodeProfile->containerType;
    }
    else
    {
        transportType = hHttpStreamer->pStreamer->streamerStreamInfo.transportType;
    }

    if ( transportType == NEXUS_TransportType_eUnknown )
    {
        BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpStreamer=%p: Media Container type is Unknown, so not adding Content-Type Header" BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
        return ( BIP_SUCCESS );
    }

    if ( hHttpStreamer->output.settings.enableDtcpIp )
    {
        BIP_HttpSocketStatus httpSocketStatus;
        BIP_DtcpIpServerStatus dtcpIpServerStatus;

        /* For DTCP/IP Content-Type header value looks like this: */
        /* Content-Type: application/x-dtcp1;DTCP1HOST=192.168.1.121;DTCP1PORT=5000;CONTENTFORMAT=video/vnd.dlna.mpeg-tts */

        /* Get the IP Address associated with the local DTCP/IP Authentication (AKE) Server. */
        bipStatus = BIP_HttpSocket_GetStatus( hHttpStreamer->processRequest.hHttpSocket, &httpSocketStatus );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpSocket_GetStatus Failed" ), error, bipStatus, bipStatus );

        bipStatus = BIP_DtcpIpServer_GetStatus( hHttpStreamer->startSettings.hInitDtcpIp, &dtcpIpServerStatus );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_DtcpIpServer_GetStatus Failed" ), error, bipStatus, bipStatus );

        hContentTypeValueBase = BIP_String_CreateFromPrintf("application/x-dtcp1;DTCP1HOST=%s;DTCP1PORT=%s;CONTENTFORMAT=",
                httpSocketStatus.pLocalIpAddress,
                dtcpIpServerStatus.pAkePort
                );
        BIP_CHECK_GOTO(( hContentTypeValueBase ), ( "BIP_String_CreateFromPrintf Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
        BDBG_MSG(( BIP_MSG_PRE_FMT "local Ip: %s, port: %s" BIP_MSG_PRE_ARG, httpSocketStatus.pLocalIpAddress, dtcpIpServerStatus.pAkePort ));
    }

    transportTimeStampEnabled = hHttpStreamer->output.settings.streamerSettings.mpeg2Ts.enableTransportTimestamp;
    switch ( transportType )
    {
        case NEXUS_TransportType_eTs:
        case NEXUS_TransportType_eMpeg2Pes:
        case NEXUS_TransportType_eVob:
        case NEXUS_TransportType_eMpeg1Ps:
            if ( hHttpStreamer->output.settings.enableDtcpIp )
            {
                pContentTypeString = transportTimeStampEnabled ? "video/vnd.dlna.mpeg-tts" : "video/vnd.dlna.mpeg"; /* MPEG2, AVC TS files */
            }
            else
            {
                pContentTypeString = transportTimeStampEnabled ? "video/mpeg-tts" : "video/mpeg"; /* MPEG2, AVC TS files */
            }
            break;
        case NEXUS_TransportType_eMp4:
            pContentTypeString = "video/mp4"; /* MP4 */
            break;
        case NEXUS_TransportType_eAsf:
            pContentTypeString = "video/x-ms-wmv"; /* ASF: WMV */
            break;
        case NEXUS_TransportType_eAvi:
            pContentTypeString = "video/x-msvideo"; /* AVI */
            break;
        case NEXUS_TransportType_eWav:
            pContentTypeString = "video/x-wav"; /* WAV */
            break;
        case NEXUS_TransportType_eMkv:
            pContentTypeString = "video/x-matroska"; /* MKV */
            break;
            /* TODO: add mapping for additional supported container formats ! */
        default:
            pContentTypeString = "text/plain";
    }

    if ( hHttpStreamer->output.settings.enableDtcpIp )
    {
        hContentTypeValue = BIP_String_CreateFromPrintf( "%s%s", BIP_String_GetString(hContentTypeValueBase), pContentTypeString );
        BIP_CHECK_GOTO(( hContentTypeValue ), ( "BIP_String_CreateFromPrintf Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        pContentTypeString = BIP_String_GetString( hContentTypeValue );
    }

    /* Add content type header. */
    hHeader = BIP_HttpResponse_AddHeader( hHttpResponse, "Content-Type", pContentTypeString, NULL );
    BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader Failed" ), error, BIP_ERR_INTERNAL, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT " container %d: Mime type: %s" BIP_MSG_PRE_ARG, transportType, pContentTypeString ));

error:
    if ( hContentTypeValue ) BIP_String_Destroy( hContentTypeValue );
    if ( hContentTypeValueBase ) BIP_String_Destroy( hContentTypeValueBase );
    return ( bipStatus );
} /* addContentTypeHeader */

static BIP_Status addHeaderValueUnsigned(
    BIP_HttpResponseHandle      hHttpResponse,
    const char                  *pHeaderName,
    unsigned                    headerValue
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_HttpHeaderHandle hHeader;
    char tempString[12];/* (2^32 - 1) = 4294967295 -> 10 digits, one for sign, one for '\0' */

    snprintf(tempString,sizeof(tempString), "%u", headerValue ); /* TODO: Later this will be part of custom header.*/

    hHeader = BIP_HttpResponse_AddHeader( hHttpResponse, pHeaderName, tempString, NULL);
    BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
error:
    return bipStatus;
}

/* Function to add AV Specific Response Headers */
static BIP_Status addAVInfoHeadersForTranscode(
    BIP_HttpStreamerHandle hHttpStreamer,
    BIP_TranscodeProfile *pTranscodeProfile,
    BIP_HttpResponseHandle hHttpResponse
    )
{
    BIP_Status bipStatus = BIP_ERR_NOT_AVAILABLE;

#if NEXUS_HAS_VIDEO_ENCODER
    BIP_HttpHeaderHandle hHeader;
    /* Add content type header. */
    bipStatus = addContentTypeHeader( hHttpStreamer, pTranscodeProfile, hHttpResponse );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addContentTypeHeader() Failed" ), error, bipStatus, bipStatus );

    /* Add Broadcom Proprietary Headers to indicate PSI Info to the client via the HTTP Response. */
    /* This allows client to skip the PSI parsing all together and simply determine that info from the HTTP Response. */

    /* Add Container Type */
    bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Transport-Type", pTranscodeProfile->containerType );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

    if ( hHttpStreamer->pStreamer->inputType == BIP_StreamerInputType_eTuner )
    {
        /* Insert a Header to indicate to client that it should be in the live mode. */
        hHeader = BIP_HttpResponse_AddHeader( hHttpResponse, "BCM-LiveChannel", "1", NULL);
        BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    hHeader = BIP_HttpResponse_AddHeader(
                    hHttpResponse,
                    "TTS",
                    (hHttpStreamer->output.settings.streamerSettings.mpeg2Ts.enableTransportTimestamp?"1":"0"),
                    NULL
                    );
    BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader Failed" ), error, BIP_ERR_INTERNAL, bipStatus );

    /* Add AV track info into the HTTP Response. */
    {

        if ( !pTranscodeProfile->disableVideo )
        {

            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Video-Pid", pTranscodeProfile->video.trackId);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Video-Type", pTranscodeProfile->video.startSettings.codec);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Video-Width", pTranscodeProfile->video.width);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Video-Height", pTranscodeProfile->video.height);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );
        }
        if ( !pTranscodeProfile->disableAudio )
        {
            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Audio-Pid", pTranscodeProfile->audio.trackId);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Audio-Type", pTranscodeProfile->audio.audioCodec);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );
        }

        bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Pcr-Pid", pTranscodeProfile->mpeg2Ts.pcrPid);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );
    }
    bipStatus = BIP_SUCCESS;

error:
#else /* NEXUS_HAS_VIDEO_ENCODER */
    BSTD_UNUSED(hHttpStreamer);
    BSTD_UNUSED(pTranscodeProfile);
    BSTD_UNUSED(hHttpResponse);
    bipStatus = BIP_ERR_NOT_AVAILABLE;
#endif /* NEXUS_HAS_VIDEO_ENCODER */
    return bipStatus;
} /* addAVInfoHeadersForTranscode */

static BIP_Status addContentFeaturesHeader(
    BIP_HttpStreamerHandle    hHttpStreamer,
    BIP_HttpResponseHandle    hHttpResponse
    )
{
    BIP_Status          bipStatus = BIP_ERR_INTERNAL;
    BIP_StringHandle    hContentFeatures = NULL;

    hContentFeatures = BIP_String_Create();
    BIP_CHECK_GOTO(( hContentFeatures ), ( "BIP_String_CreateFromChar Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    /* TODO: Add profile Name based on the stream's container type! */

    /* Add DLNA.OP_FLAGS */
    if ( hHttpStreamer->pStreamer->inputType == BIP_StreamerInputType_eFile )
    {
        /* For File inputs, we can support both Time & Byte based Seek. So set flags to indicate it. */

        /*
         * BNF Format of DLNA.OP_FLAGS:
         * op-param = [op-param-delim] "DLNA.ORG_OP=" op-value
         * op-param-delim = ";"
         * op-value = a-val b-val
         * a-val = Boolean
         * b-val = Boolean
         *
         * Here a-val indicates support of TimeSeekRange.dlna.org HTTP header.
         * Here b-val indicates support of Range HTTP header.
         * Example: DLNA.ORG_OP=10
         */
        bool supportTimeBasedSeek = false;
        BIP_StreamerTrackListEntryHandle hTrackEntry = NULL;

        if (
                hHttpStreamer->pStreamer->streamerStreamInfo.transportType == NEXUS_TransportType_eTs &&
                BIP_Streamer_GetTrackEntry_priv( hHttpStreamer->hStreamer, BIP_MediaInfoTrackType_eVideo, &hTrackEntry ) == BIP_SUCCESS &&
                hTrackEntry->hMediaNavFileAbsolutePathName )
        {
            /* We only support Time based Seek for TS formats when Video track is enabled */
            supportTimeBasedSeek = true;
        }
        bipStatus = BIP_String_StrcatPrintf(hContentFeatures, ";DLNA.ORG_OP=%s%s",
                supportTimeBasedSeek ?  "1":"0",
                "1");   /* For files, we can always support range header, so we set the b-val as 1. */
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_StrcatPrintf() Failed" ), error, bipStatus, bipStatus );
    }

    /* Add DLNA.ORG_Flags */
    {
        /*
         * Add DLNA.ORG_Flags:
         *  flags-param = flags-param-delim "DLNA.ORG_FLAGS=" flags-value
         *  flags-param-delim = ";"
         *  flags-value = primary-flags reserved-data
         *  primary-flags = 8 hexdigit
         *  reserved-data = 24 reserved-hexdigit
         *  hexdigit = <hexadecimal digit: "0"-"9", "A"-"F", "a"-"f">
         *  reserved-hexdigit = "0"
         */
        uint8_t dlnaFlagNibbles[8] = {0,};
        unsigned dlnaOrgFlags = 0;

        /* Set the relevant DLNA ORG Flags. */
        {
            if ( hHttpStreamer->pStreamer->inputType == BIP_StreamerInputType_eFile )
            {
                dlnaOrgFlags |= DLNA_ORG_FLAGS_CONNECTION_STALLING;
            }
            if ( hHttpStreamer->output.settings.enableDtcpIp )
            {
                dlnaOrgFlags |= DLNA_ORG_FLAGS_LINK_PROTECTED_CONTENT;
            }
            if (
                    hHttpStreamer->pStreamer->inputType == BIP_StreamerInputType_eTuner ||
                    (hHttpStreamer->pStreamer->inputType == BIP_StreamerInputType_eFile && hHttpStreamer->pStreamer->streamerStreamInfo.transportType == NEXUS_TransportType_eTs  && hHttpStreamer->pStreamer->file.inputSettings.enableHwPacing) )
            {
                /* Set bit to indicate Sender Pacing. */
                dlnaOrgFlags |= DLNA_ORG_FLAGS_SENDER_PACED;
            }
            {
                /* All AV is being streamed out, so set the StreamingMode flag. */
                dlnaOrgFlags |= DLNA_ORG_FLAGS_STREAMING_MODE;
            }
        }

        /* Prepare the Nibbles for converting to ASCII. */
        {
            dlnaFlagNibbles[0] = (dlnaOrgFlags >> 0)  & 0xf;
            dlnaFlagNibbles[1] = (dlnaOrgFlags >> 4)  & 0xf;

            dlnaFlagNibbles[2] = (dlnaOrgFlags >> 8)  & 0xf;
            dlnaFlagNibbles[3] = (dlnaOrgFlags >> 12) & 0xf;

            dlnaFlagNibbles[4] = (dlnaOrgFlags >> 16) & 0xf;
            dlnaFlagNibbles[5] = (dlnaOrgFlags >> 20) & 0xf;

            dlnaFlagNibbles[6] = (dlnaOrgFlags >> 24) & 0xf;
            dlnaFlagNibbles[7] = (dlnaOrgFlags >> 28) & 0xf;
        }

        /* Add to the contentFeatures String. */
        {
            bipStatus = BIP_String_StrcatPrintf(hContentFeatures, ";DLNA.ORG_FLAGS=");
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_StrcatPrintf() Failed" ), error, bipStatus, bipStatus );
            {
                int i;
                bipStatus = BIP_String_StrcatPrintf(hContentFeatures, "%x%x%x%x%x%x%x%x",
                    dlnaFlagNibbles[7], dlnaFlagNibbles[6],
                    dlnaFlagNibbles[5], dlnaFlagNibbles[4],
                    dlnaFlagNibbles[3], dlnaFlagNibbles[2],
                    dlnaFlagNibbles[1], dlnaFlagNibbles[0]);
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_StrcatPrintf() Failed" ), error, bipStatus, bipStatus );
                /* Now add the 24 reserved hex-digits representing the reserved nibbles. */
                for (i=0; i<24; i++)
                {
                    bipStatus = BIP_String_StrcatPrintf(hContentFeatures, "%x", 0);
                }
            }
            BDBG_MSG((BIP_MSG_PRE_FMT "hHttpStreamer=%p: dlnaOrgFlags=%s" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_String_GetString(hContentFeatures) ));
        }
    } /* DLNA.ORG_FLAGS */

    /* Add DLNA.ORG_PS */
    {
        BIP_StreamerTrackListEntryHandle hTrackEntry = NULL;
        if (
                hHttpStreamer->pStreamer->streamerStreamInfo.transportType == NEXUS_TransportType_eTs &&
                BIP_Streamer_GetTrackEntry_priv( hHttpStreamer->hStreamer, BIP_MediaInfoTrackType_eVideo, &hTrackEntry ) == BIP_SUCCESS &&
                hTrackEntry->hMediaNavFileAbsolutePathName )
        {
            /* For now, hardcode the supported playSpeeds. But TODO: is need a proper way to define them. */
            /* App can Get this header and overwrite it if apps wants to use different play speed values. */
            bipStatus = BIP_String_StrcatPrintf(hContentFeatures, ";DLNA.ORG_PS=-48,-36,-18,-12,-6,6,12,18,36,48");
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_StrcatPrintf() Failed" ), error, bipStatus, bipStatus );
            BDBG_MSG((BIP_MSG_PRE_FMT "hHttpStreamer=%p: DLNA.ORG_PS=%s" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_String_GetString(hContentFeatures) ));
        }
    }

    /* We are done building the various parts of the ContentFeatures header. So add it now. */
    {
        BIP_HttpHeaderHandle hHeader;
        hHeader = BIP_HttpResponse_AddHeader( hHttpResponse, "ContentFeatures.dlna.org", BIP_String_GetString(hContentFeatures), NULL );
        BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }
    bipStatus = BIP_SUCCESS;

error:
    if (hContentFeatures) BIP_String_Destroy(hContentFeatures);
    return bipStatus;
} /* addContentFeaturesHeader */

/* Function to add AV Specific Response Headers */
static BIP_Status addAVInfoHeaders(
    BIP_HttpStreamerHandle    hHttpStreamer,
    BIP_HttpResponseHandle hHttpResponse
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;

    /* Add content type header. */
    bipStatus = addContentTypeHeader( hHttpStreamer, NULL, hHttpResponse );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addContentTypeHeader() Failed" ), error, bipStatus, bipStatus );

    /* Add Broadcom Proprietary Headers to indicate PSI Info to the client via the HTTP Response. */
    /* This allows client to skip the PSI parsing all together and simply determine that info from the HTTP Response. */

    if ( hHttpStreamer->pStreamer->streamerStreamInfo.transportType == NEXUS_TransportType_eUnknown )
    {
        BDBG_WRN(( BIP_MSG_PRE_FMT "Can't set AV headers for containerType %d" BIP_MSG_PRE_ARG, hHttpStreamer->pStreamer->streamerStreamInfo.transportType ));
        return BIP_SUCCESS;
    }

    /* Add header for Stream duration. */
    {
        BIP_HttpHeaderHandle hHeader = NULL;
        bipStatus = BIP_HttpRequest_GetNextHeader(hHttpStreamer->processRequest.settings.hHttpRequest, NULL, "getAvailableSeekRange.dlna.org", &hHeader, NULL);
        if (bipStatus == BIP_SUCCESS && hHeader && hHttpStreamer->pStreamer->streamerStreamInfo.durationInMs > 0)
        {
            BIP_StringHandle hSeekRange = NULL;
            unsigned modeFlag = 0;
            hSeekRange = BIP_String_CreateFromPrintf("%d npt=0-%u.%u",
                    modeFlag,
                    hHttpStreamer->pStreamer->streamerStreamInfo.durationInMs/1000,     /* Converted to sec. */
                    hHttpStreamer->pStreamer->streamerStreamInfo.durationInMs%1000);    /* Converted to msec. */
            BIP_CHECK_GOTO(( hSeekRange ), ( "BIP_String_CreateFromPrintf Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
            hHeader = BIP_HttpResponse_AddHeader( hHttpResponse, "availableSeekRange.dlna.org", BIP_String_GetString(hSeekRange), NULL);
            if (hSeekRange) BIP_String_Destroy(hSeekRange);
            BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
        }

        /* For backward compatibility with older clients, also insert proprietary header to indicate stream duration. */
        bipStatus = addHeaderValueUnsigned( hHttpResponse, "x-intel-MediaDuration", hHttpStreamer->pStreamer->streamerStreamInfo.durationInMs/1000 /*convert to sec*/);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

    }

    if ( hHttpStreamer->output.settings.disableDlnaContentFeatureHeaderInsertion == false )
    {
        bipStatus = addContentFeaturesHeader(hHttpStreamer, hHttpResponse);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addContentFeaturesHeader Failed" ), error, bipStatus, bipStatus );
    }

    /* if App hasn't disabled AV Header Insertion, then add headers to indicate AV Track info (pids, codecs, container, etc.) */
    if ( hHttpStreamer->output.settings.disableAvHeadersInsertion == false )
    {
        /* Add Container Type */
        bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Transport-Type", hHttpStreamer->pStreamer->streamerStreamInfo.transportType );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

        if ( ( hHttpStreamer->pStreamer->inputType == BIP_StreamerInputType_eFile &&
                    hHttpStreamer->pStreamer->file.inputSettings.enableHwPacing &&
                    hHttpStreamer->pStreamer->file.feedUsingPlaybackChannel && convertPlaySpeedToInt(BIP_String_GetString(hHttpStreamer->pStreamer->file.hPlaySpeed)) == 1 ) ||
                (hHttpStreamer->pStreamer->inputType == BIP_StreamerInputType_eTuner )
           )
        {
            /* If we are feeding via the playback channel && client hasn't requested any non-1x playSpeeds, */
            /* then we insert a Header to indicate to client that it should be in the live mode. */
            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-LiveChannel", 1);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );
        }

        {
            bool ttsMode =
                (hHttpStreamer->output.settings.streamerSettings.mpeg2Ts.enableTransportTimestamp ||
                 hHttpStreamer->pStreamer->streamerStreamInfo.transportTimeStampEnabled)
                ? true : false;
            bipStatus = addHeaderValueUnsigned( hHttpResponse, "TTS", ttsMode);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );
        }

        /* Add AV track info into the HTTP Response. */
        {
            BIP_StreamerTrackListEntryHandle hTrackEntry;

            for (
                    hTrackEntry = BLST_Q_FIRST( &hHttpStreamer->pStreamer->track.listHead);
                    hTrackEntry;
                    hTrackEntry = BLST_Q_NEXT( hTrackEntry, trackListNext)
                )
            {
                switch (hTrackEntry->streamerTrackInfo.type)
                {
                    case BIP_MediaInfoTrackType_eVideo:
                        {
                            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Video-Pid", hTrackEntry->streamerTrackInfo.trackId);
                            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

                            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Video-Type", hTrackEntry->streamerTrackInfo.info.video.codec);
                            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );/* TODO: I have intentionally done this addition different way in different places , based on review we can consider one of them.*/

                            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Video-Width", hTrackEntry->streamerTrackInfo.info.video.width);
                            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

                            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Video-Height", hTrackEntry->streamerTrackInfo.info.video.height);
                            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

                            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Video-ColorDepth", hTrackEntry->streamerTrackInfo.info.video.colorDepth);
                            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

                            BDBG_MSG((BIP_MSG_PRE_FMT "trackPid %d, video codec %d" BIP_MSG_PRE_ARG,hTrackEntry->streamerTrackInfo.trackId, hTrackEntry->streamerTrackInfo.info.video.codec));
                            break;
                        }
                    case BIP_MediaInfoTrackType_eAudio:
                        {
                            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Audio-Pid", hTrackEntry->streamerTrackInfo.trackId);
                            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

                            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Audio-Type", hTrackEntry->streamerTrackInfo.info.video.codec);
                            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

                            BDBG_MSG((BIP_MSG_PRE_FMT "trackPid d, pid %d, audio codec %d" BIP_MSG_PRE_ARG, hTrackEntry->streamerTrackInfo.trackId, hTrackEntry->streamerTrackInfo.info.audio.codec));
                            break;
                        }
                    case BIP_MediaInfoTrackType_ePcr:
                        {
                            bipStatus = addHeaderValueUnsigned( hHttpResponse, "BCM-Pcr-Pid", hTrackEntry->streamerTrackInfo.trackId);
                            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addHeaderValueUnsigned Failed" ), error, bipStatus, bipStatus );

                            BDBG_MSG((BIP_MSG_PRE_FMT "trackPid %d, pcr " BIP_MSG_PRE_ARG, hTrackEntry->streamerTrackInfo.trackId));
                            break;
                        }
                    default:
                        break;
                } /* switch */
            } /* for () */
        }
    }
    bipStatus = BIP_SUCCESS;

error:
    return bipStatus;
} /* addAVInfoHeaders */

static BIP_Status prepareCommonResponseHeaders(
    BIP_HttpStreamerHandle  hHttpStreamer
    )
{
    BIP_Status                   bipStatus = BIP_ERR_INTERNAL;
    BIP_HttpResponseHandle       hHttpResponse = hHttpStreamer->response.hHttpResponse;
    BIP_HttpResponseStatus       responseStatus = BIP_HttpResponseStatus_e200_OK;
    BIP_HttpHeaderHandle         hHeader;

    /* Add Standard HTTP Headers */
    {
        BIP_HttpResponse_Clear( hHttpResponse , NULL);

        /* Add Response Status. */
        {
            /* Add Successful Status: can be 200 or 206 depending upon if response includes a range. */
            /* TODO: Need to add a _eSuccess206 status code and set it when inputSetting includes a valid byte-range. */
            bipStatus = BIP_HttpResponse_SetStatus( hHttpResponse, responseStatus );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpResponse_SetStatus Failed" ), error, bipStatus, bipStatus );
        }

        /* Add a Server Name header. */
        {
            hHeader = BIP_HttpResponse_AddHeader( hHttpResponse, "Server", "BIP Based HTTP Media Streamer: 0.7", NULL);
            BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
        }

        /* TODO: Add Date Header. */
    }
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Response headers prepared!" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));
#if 0 /* TODO: Shall we remove this ?*/
    /* Unit test code. */
    bipStatus = BIP_ERR_INVALID_PARAMETER;
#endif

error:
    return (bipStatus);
} /* prepareCommonResponseHeaders */

static BIP_Status sendErrorResponse(
    BIP_HttpStreamerHandle      hHttpStreamer,
    BIP_HttpSocketHandle        hHttpSocket,
    BIP_HttpRequestHandle       hHttpRequest
    )
{
    BIP_Status bipStatus;
    BIP_HttpResponseHandle hHttpResponse = hHttpStreamer->response.hHttpResponse;

    {
        /* Prepare the Error Response. */
        BIP_HttpResponse_Clear( hHttpResponse, NULL );

        /* TODO: need to look into the Status code for errors, may be caller should be setting it as per the error it has run into. */
        bipStatus = BIP_HttpResponse_SetStatus( hHttpResponse, BIP_HttpResponseStatus_e500_InternalServerError);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpResponse_SetStatus Failed" ), error, bipStatus, bipStatus );
    }

    /* Send the Response. */
    {
        BIP_HttpSocketSendResponseSettings sendResponseSettings;

        BIP_HttpSocket_GetDefaultSendResponseSettings( &sendResponseSettings );
        sendResponseSettings.noPayload = true;
        sendResponseSettings.timeoutInMs = -1;

        bipStatus = BIP_HttpSocket_SendResponse( hHttpSocket, hHttpRequest, hHttpStreamer->response.hHttpResponse, 0 /*messageLength*/, &sendResponseSettings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpSocket_SendResponse Failed to send Error Response" ), error, bipStatus, bipStatus );
    }
error:
    return (bipStatus);
} /* sendErrorResponse */

static void inactivityTimerCallback(
    void *pContext
    )
{
    BIP_Status bipStatus;
    BIP_HttpStreamerHandle    hHttpStreamer = (BIP_HttpStreamerHandle) pContext;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    if(bipStatus != BIP_SUCCESS)
    {
        return;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p --------------------> "  BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));

    B_Mutex_Lock( hHttpStreamer->hStateMutex );
    if ( hHttpStreamer->hInactivityTimer )
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Got BIP_Timer callback, marking timer as self-destructed" BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
        hHttpStreamer->hInactivityTimer = NULL;   /* Indicate timer not active. */
    }
    B_Mutex_Unlock( hHttpStreamer->hStateMutex );

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    processHttpStreamerState( hHttpStreamer, 0, BIP_Arb_ThreadOrigin_eTimer);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: <-------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
} /* inactivityTimerCallback */

static void destroyInactivityTimer(
    BIP_HttpStreamerHandle hHttpStreamer
    )
{
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Cancelling inactivityTimer: hTimer=%p timerActive=%s" BIP_MSG_PRE_ARG,
                (void *)hHttpStreamer, (void *)hHttpStreamer->hInactivityTimer, hHttpStreamer->inactivityTimerActive?"Y":"N" ));

    if ( hHttpStreamer->hInactivityTimer ) BIP_Timer_Destroy( hHttpStreamer->hInactivityTimer );
    hHttpStreamer->hInactivityTimer = NULL;
    hHttpStreamer->inactivityTimerActive = false;

} /* destroyInactivityTimer */

static BIP_Status createInactivityTimer(
    BIP_HttpStreamerHandle hHttpStreamer
    )
{
    BIP_Status bipStatus;
    BIP_TimerCreateSettings     timerCreateSettings;

    if ( hHttpStreamer->hInactivityTimer ) BIP_Timer_Destroy( hHttpStreamer->hInactivityTimer );

    BIP_Timer_GetDefaultCreateSettings( &timerCreateSettings );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Starting streamer inactivity polling timer for %d ms" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_INACTIVITY_TIMER_POLL_INTERVAL_IN_MSEC ));
    timerCreateSettings.input.callback    = inactivityTimerCallback;
    timerCreateSettings.input.pContext    = hHttpStreamer;
    timerCreateSettings.input.timeoutInMs = BIP_HTTP_STREAMER_INACTIVITY_TIMER_POLL_INTERVAL_IN_MSEC;
    hHttpStreamer->hInactivityTimer = BIP_Timer_Create( &timerCreateSettings );
    if ( hHttpStreamer->hInactivityTimer )
    {
        hHttpStreamer->inactivityTimerActive = true;
        B_Time_Get( &hHttpStreamer->inactivityTimerStartTime);
        bipStatus = BIP_SUCCESS;
    }
    else
    {
        bipStatus = BIP_ERR_INTERNAL;
    }
    return ( bipStatus );
} /* createInactivityTimer */

static void dtcpIpAkeTimerCallback(
    void *pContext
    )
{
    BIP_Status bipStatus;
    BIP_HttpStreamerHandle    hHttpStreamer = (BIP_HttpStreamerHandle) pContext;

    bipStatus = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    if(bipStatus != BIP_SUCCESS)
    {
        return;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpStreamer %p --------------------> "  BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));

    B_Mutex_Lock( hHttpStreamer->hStateMutex );
    if ( hHttpStreamer->hDtcpIpAkeTimer )
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Got BIP_Timer callback, marking timer as self-destructed" BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
        hHttpStreamer->hDtcpIpAkeTimer = NULL;   /* Indicate timer not active. */
    }
    B_Mutex_Unlock( hHttpStreamer->hStateMutex );

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    processHttpStreamerState( hHttpStreamer, 0, BIP_Arb_ThreadOrigin_eTimer);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpStreamer %p: <-------------------- " BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
} /* dtcpIpAkeTimerCallback */

static void destroyDtcpIpAkeTimer(
    BIP_HttpStreamerHandle hHttpStreamer
    )
{
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Cancelling dtcpIpAkeTimer: hTimer=%p timerActive=%s" BIP_MSG_PRE_ARG,
                (void *)hHttpStreamer, (void *)hHttpStreamer->hDtcpIpAkeTimer, hHttpStreamer->dtcpIpAkeTimerActive?"Y":"N" ));

    if ( hHttpStreamer->hDtcpIpAkeTimer ) BIP_Timer_Destroy( hHttpStreamer->hDtcpIpAkeTimer );
    hHttpStreamer->hDtcpIpAkeTimer = NULL;
    hHttpStreamer->dtcpIpAkeTimerActive = false;
    hHttpStreamer->dtcpIpAkeTimeNoted = false;

} /* destroyDtcpIpAkeTimer */

static BIP_Status createDtcpIpAkeTimer(
    BIP_HttpStreamerHandle hHttpStreamer
    )
{
    BIP_Status bipStatus;
    BIP_TimerCreateSettings     timerCreateSettings;

    if ( hHttpStreamer->hDtcpIpAkeTimer ) BIP_Timer_Destroy( hHttpStreamer->hDtcpIpAkeTimer );

    BIP_Timer_GetDefaultCreateSettings( &timerCreateSettings );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Starting dtcpIpAke polling timer for %d ms" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_DTCP_IP_AKE_TIMER_POLL_INTERVAL_IN_MSEC ));
    timerCreateSettings.input.callback    = dtcpIpAkeTimerCallback;
    timerCreateSettings.input.pContext    = hHttpStreamer;
    timerCreateSettings.input.timeoutInMs = BIP_HTTP_STREAMER_DTCP_IP_AKE_TIMER_POLL_INTERVAL_IN_MSEC;
    hHttpStreamer->hDtcpIpAkeTimer = BIP_Timer_Create( &timerCreateSettings );
    if ( hHttpStreamer->hDtcpIpAkeTimer )
    {
        hHttpStreamer->dtcpIpAkeTimerActive = true;
        B_Time_Get( &hHttpStreamer->dtcpIpAkeTimerStartTime);
        bipStatus = BIP_SUCCESS;
    }
    else
    {
        bipStatus = BIP_ERR_INTERNAL;
    }
    return ( bipStatus );
} /* createDtcpIpAkeTimer */

static void initiateEndOfStreamingCallback(
    BIP_HttpStreamerHandle hHttpStreamer
    )
{
    if (hHttpStreamer->createSettings.endOfStreamingCallback.callback)
    {
        BIP_Arb_AddDeferredCallback( hHttpStreamer->processRequestApi.hArb, &hHttpStreamer->createSettings.endOfStreamingCallback );
    }
    /* Change state to reflect this transition. */
    hHttpStreamer->state = BIP_HttpStreamerState_eWaitingForStopApi;
    hHttpStreamer->hls.state = BIP_HttpStreamerHlsState_eWaitingForStopApi;

    /* Cancel the inactivity timer. */
    destroyInactivityTimer( hHttpStreamer );
    destroyDtcpIpAkeTimer( hHttpStreamer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p state=%s: initiateEndOfStreamingCallback" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE( hHttpStreamer->state ) ));
} /* initiateEndOfStreamingCallback */

#ifdef NEXUS_HAS_VIDEO_ENCODER
static BIP_Status finalizeHeadersAndSendResponseForHls(
    BIP_HttpStreamerHandle      hHttpStreamer,
    BIP_HttpSocketHandle        hHttpSocket,
    BIP_HttpRequestHandle       hHttpRequest,
    BIP_TranscodeProfile        *pTranscodeProfile,
    int64_t                     messageLength,
    bool                        requestForMediaSegment,
    bool                        *successfullResponseSent
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    BIP_HttpResponseHandle hHttpResponse = hHttpStreamer->response.hHttpResponse;
    BIP_HttpHeaderHandle hHeader;

    if (hHttpStreamer->response.state == BIP_HttpStreamerResponseHeadersState_eNotSet)
    {
        bipStatus = prepareCommonResponseHeaders( hHttpStreamer );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "prepareCommonResponseHeaders Failed" ), error, bipStatus, bipStatus );
        hHttpStreamer->response.state = BIP_HttpStreamerResponseHeadersState_eSet;
    }

    if ( requestForMediaSegment )
    {
        bipStatus = addAVInfoHeadersForTranscode(hHttpStreamer, pTranscodeProfile, hHttpResponse );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addAVInfoHeadersForTranscode Failed" ), error, bipStatus, bipStatus );

        /* Also, we ignore any byte-range headers in the request from the client, it can't be supported on the encoded content. */
    }
    else
    {
        /* Response is for Playlists, Master or Media. */
        hHeader = BIP_HttpResponse_AddHeader( hHttpResponse, "Content-Type", "application/x-mpegURL", NULL );
        BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    /*
     * Send the Response.
     * Note: All HTTP Resvd headers such as Content-Length, Transfer-Encoding, Connection, etc. will be added in the HttpSocket_SendResponse().
     */
    {
        BIP_HttpRequestMethod method;
        const char *pMethodName = NULL;
        BIP_HttpSocketSendResponseSettings sendResponseSettings;

        bipStatus = BIP_HttpRequest_GetMethod( hHttpRequest, &method, &pMethodName);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetMethod Failed" ), error, bipStatus, bipStatus );

        BIP_HttpSocket_GetDefaultSendResponseSettings( &sendResponseSettings );
        sendResponseSettings.noPayload = (method == BIP_HttpRequestMethod_eHead) ? true : false;
        sendResponseSettings.timeoutInMs = -1;

        bipStatus = BIP_HttpSocket_SendResponse( hHttpSocket, hHttpRequest, hHttpStreamer->response.hHttpResponse, messageLength, &sendResponseSettings );
    }
error:
    *successfullResponseSent = (bipStatus == BIP_SUCCESS) ? true : false;
    return (bipStatus);
} /* finalizeHeadersAndSendResponseForHls */
#endif /* NEXUS_HAS_VIDEO_ENCODER */

/*TODO: Later this will be replaced with customApis */
static BIP_Status createContentRange(
                    BIP_HttpResponseHandle      hHttpResponse,
                    uint64_t                    startOffset,
                    uint64_t                    endOffset,
                    uint64_t                    contentLength
                    )
{
    BIP_StringHandle        hString = NULL;
    BIP_Status              bipStatus = BIP_SUCCESS;
    BIP_HttpHeaderHandle    hHeader;

    hString = BIP_String_CreateFromPrintf("bytes %llu-%llu", startOffset, endOffset);

    if(contentLength)
    {
        bipStatus = BIP_String_StrcatPrintf(hString, "/%llu", contentLength);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_StrcatPrintf Failed" ), error, bipStatus, bipStatus );
    }
    else
    {
        bipStatus = BIP_String_StrcatPrintf(hString, "/*");
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_StrcatPrintf Failed" ), error, bipStatus, bipStatus );
    }
    hHeader = BIP_HttpResponse_AddHeader(hHttpResponse , "Content-Range", BIP_String_GetString(hString), NULL);
    BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader Failed" ), error, BIP_ERR_INTERNAL, bipStatus );

error:
    if(hString)
    {
        BIP_String_Destroy(hString);
    }
    return bipStatus;
}

static BIP_Status getSocketFdFromHttpSocket(
    BIP_HttpSocketHandle    hHttpSocket,
    int                     *pSocketFd
    )
{
    BIP_Status bipStatus;
    BIP_HttpSocketStatus httpSocketStatus;

    BDBG_ASSERT( hHttpSocket );
    if ( !hHttpSocket ) return (BIP_ERR_INVALID_PARAMETER);

    /* Get raw socketFd using the HttpSocket object's status that PBIP would use for streaming. */
    bipStatus = BIP_HttpSocket_GetStatus( hHttpSocket, &httpSocketStatus );

    if ( bipStatus == BIP_SUCCESS )
    {
        *pSocketFd = httpSocketStatus.socketFd;
    }
    return ( bipStatus );
} /* getSocketFdFromHttpSocket */

static BIP_Status finalizeHeadersAndSendResponse(
    BIP_HttpStreamerHandle hHttpStreamer,
    BIP_TranscodeProfile *pTranscodeProfile,
    bool *successfullResponseSent
    )
{
    int64_t messageLength = 0;/*TODO: This not used , it may be removed*/
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    BIP_HttpResponseHandle hHttpResponse = hHttpStreamer->response.hHttpResponse;

    /* Add special headers to indicate AV Track info (pids, codecs, container, etc.) */
    /* However, since we have very limited information about the recpump input, we dont insert AV headers (containing tracks info), etc. */
    if ( hHttpStreamer->pStreamer->inputType != BIP_StreamerInputType_eRecpump )
    {
        if ( pTranscodeProfile )
        {
            bipStatus = addAVInfoHeadersForTranscode(hHttpStreamer, pTranscodeProfile, hHttpResponse );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addAVInfoHeadersForTranscode Failed" ), error, bipStatus, bipStatus );
        }
        else
        {
            bipStatus = addAVInfoHeaders(hHttpStreamer, hHttpResponse );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "addAVInfoHeaders Failed" ), error, bipStatus, bipStatus );
        }

        if ( hHttpStreamer->output.settings.disableContentLengthInsertion )
        {
            BDBG_WRN((BIP_MSG_PRE_FMT "!!!!!! Note: disableContentLengthInsertion is set: so not sending the HTTP Content Length in HTTP Response !!!!!" BIP_MSG_PRE_ARG));
            messageLength = 0;
        }
        else if ( pTranscodeProfile )
        {
            /* If stream is being encoded, we dont know the encoded length, so clear the message length. */
            messageLength = 0;
        }
        /* Add Content-Range Header if app has specified a specific byte range. */
        else if(hHttpStreamer->pStreamer->file.inputSettings.beginByteOffset || hHttpStreamer->pStreamer->file.inputSettings.endByteOffset)
        {
            bipStatus = createContentRange(
                    hHttpResponse,
                    hHttpStreamer->pStreamer->file.inputSettings.beginByteOffset,
                    hHttpStreamer->pStreamer->file.inputSettings.endByteOffset,
                    hHttpStreamer->pStreamer->streamerStreamInfo.contentLength
                    );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "createContentRange Failed" ), error, bipStatus, bipStatus );

            /* Now add the correct content-length */
            messageLength = hHttpStreamer->pStreamer->file.inputSettings.endByteOffset - hHttpStreamer->pStreamer->file.inputSettings.beginByteOffset + 1;
        }
#if 0  /*TODO: Will be added with the custome header changes.*/
        /* Else see if app has specified time-range & add the Response Header for the dlnaTimeSeekRange Header Request */
        else if (hHttpStreamer->pStreamer->file.inputSettings.beginTimeOffsetInMs || hHttpStreamer->pStreamer->file.inputSettings.endTimeOffsetInMs)
        {
            BIP_HttpResponseDlnaTimeSeekRange   dlnaTimeSeekRange;

            dlnaTimeSeekRange.contentDuration = hHttpStreamer->pStreamer->streamerStreamInfo.durationInMs;
            dlnaTimeSeekRange.contentDurationValid = true;
            dlnaTimeSeekRange.startTime = hHttpStreamer->pStreamer->file.inputSettings.beginTimeOffsetInMs;
            dlnaTimeSeekRange.startTimeValid = true;
            dlnaTimeSeekRange.endTime = hHttpStreamer->pStreamer->file.inputSettings.endTimeOffsetInMs;
            dlnaTimeSeekRange.endTimeValid = true;
            bipStatus = BIP_HttpResponse_SetDlnaTimeSeekRange( hHttpResponse, &dlnaTimeSeekRange );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpResponse_SetDlnaTimeSeekRange Failed" ), error, bipStatus, bipStatus );

            /* Since we are sending part of a stream offset by the time, we dont know its correct length. */
            messageLength = 0;
        }
#endif
        else
        {
            messageLength = hHttpStreamer->pStreamer->streamerStreamInfo.contentLength;
        }
    }

    {
        /* Set the DSCP Value for the outgoing Response & AV TCP Segments. */
#define DSCP_CLASS_SELECTOR_VIDEO 0xa0
        int dscpTcValue = DSCP_CLASS_SELECTOR_VIDEO;
        int socketFd;

        bipStatus = getSocketFdFromHttpSocket( hHttpStreamer->processRequest.hHttpSocket, &socketFd );
        if (bipStatus == BIP_SUCCESS)
        {
            if (setsockopt(socketFd, IPPROTO_IP, IP_TOS, &dscpTcValue, sizeof(dscpTcValue)) < 0)
            {
                BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpStreamer=%p: Can't set DSCP value, socketFd=%d errno=%d" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, socketFd, errno));
            }
        }
    }
    /*
     * Send the Response.
     * Note: All HTTP Resvd headers such as Content-Length, Transfer-Encoding, Connection, etc. will be added in the HttpSocket_SendResponse().
     */
    {
        BIP_HttpRequestMethod method;
        const char *pMethodName = NULL;
        BIP_HttpSocketSendResponseSettings sendResponseSettings;

        bipStatus = BIP_HttpRequest_GetMethod( hHttpStreamer->processRequest.settings.hHttpRequest, &method, &pMethodName);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetMethod Failed" ), error, bipStatus, bipStatus );

        BIP_HttpSocket_GetDefaultSendResponseSettings( &sendResponseSettings );
        sendResponseSettings.noPayload = (method == BIP_HttpRequestMethod_eHead) ? true : false;
        sendResponseSettings.timeoutInMs = -1;
        sendResponseSettings.enableHttpChunkXferEncoding = hHttpStreamer->output.settings.enableHttpChunkXferEncoding;

        bipStatus = BIP_HttpSocket_SendResponse(hHttpStreamer->processRequest.hHttpSocket, hHttpStreamer->processRequest.settings.hHttpRequest, hHttpStreamer->response.hHttpResponse, messageLength, &sendResponseSettings);
    }
error:
    *successfullResponseSent = (bipStatus == BIP_SUCCESS) ? true : false;
    return (bipStatus);
} /* finalizeHeadersAndSendResponse */

static void stopAndDestroyStreamer(
        BIP_HttpStreamerHandle hHttpStreamer
        )
{
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s:" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
#ifdef NEXUS_HAS_ASP
    /* Notify ASP or PBIP to stop streaming. */
    if (hHttpStreamer->hAspChannel)
    {
        B_AspChannel_StopStreaming( hHttpStreamer->hAspChannel );
    }
    else
#endif
    {
        stopPBipStreamer( hHttpStreamer );
    }

    /* Then stop the streamer so that it stops/releases the resources associated w/ streaming pipe. */
    if ( BIP_Streamer_Stop( hHttpStreamer->hStreamer ) != BIP_SUCCESS )
    {
        BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: Stop Streamer Failed" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
        /* continue w/ stopping rest of the streamer related resources! */
    }

    /* Notify ASP or PBIP to Destroy the streaming channel. */
#ifdef NEXUS_HAS_ASP
    if (hHttpStreamer->hAspChannel)
    {
        B_AspChannel_Destroy( hHttpStreamer->hAspChannel, NULL/* TODO: &socketFd: need to see if this is a new or existing socket. If new, then need a way to let HttpSocket know about this!!!*/ );
        hHttpStreamer->hAspChannel = NULL;
        if (hHttpStreamer->hDtcpIpServerStream)
        {
            if (BIP_DtcpIpServer_CloseStream(hHttpStreamer->startSettings.hInitDtcpIp, hHttpStreamer->hDtcpIpServerStream) != BIP_SUCCESS)
            {
                BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: BIP_DtcpIpServer_CloseStream() Streamer Failed" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
            }
        }
    }
    else
#endif
    {
        destroyPBipStreamer( hHttpStreamer );
    }

    /* Only issue requestProcessed callback if we either we haven't already done it or ProcessRequest didn't even come. */
    if ( (hHttpStreamer->state != BIP_HttpStreamerState_eWaitingForStopApi &&
            hHttpStreamer->state != BIP_HttpStreamerState_eWaitingForProcessRequestApi &&
            hHttpStreamer->state != BIP_HttpStreamerState_eWaitingForClientAke &&
            hHttpStreamer->processRequest.requestProcessedCallback.callback) ||
            hHttpStreamer->inactivityTimerExpired
                    )
    {
        if (hHttpStreamer->processRequest.callbackIssued == false)
        {
            /* Add a deferred callback to let HttpServerSocket know that we are done processing the Request. */
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p: Added requestProcessedCallback" BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
            BIP_Arb_AddDeferredCallback( hHttpStreamer->processRequestApi.hArb, &hHttpStreamer->processRequest.requestProcessedCallback );
            hHttpStreamer->inactivityTimerExpired = false;
            hHttpStreamer->processRequest.callbackIssued = true;
        }
    }

    return;
} /* stopAndDestroyStreamer */

#if NEXUS_HAS_VIDEO_ENCODER

#define BIP_PLAYLIST_BUFFER_BLOCK_SIZE (64*1024)
#define BIP_PLAYLIST_URL_BASE_PREFIX "com.broadcom.bcg.bse.bipServer"
#define BIP_PLAYLIST_URL_PROFILE_PREFIX "_profile"
#define BIP_PLAYLIST_URL_STREAMER_ID_PREFIX "_streamerId"
#define BIP_PLAYLIST_URL_SUFFIX ".m3u8"
#define BIP_SEGMENT_URL_PREFIX "_seg"
#define BIP_SEGMENT_URL_SUFFIX ".ts"
#define BIP_HLS_SEGMENT_START_SEQUENCE_NUMBER 0
#define BIP_HLS_SERVER_VERSION 3

static void getFieldFromRequestTarget(
    const char  *pRequestTarget,
    const char  *pFieldPrefix,
    const char  *pFieldSuffix,
    unsigned    *pFieldValue
    )
{
    const char *pTmp1, *pTmp2;
    unsigned fieldLength;
    char *pTmpBuffer;

    *pFieldValue = 0;

    if ( pRequestTarget == NULL ) return;

    pTmp1 = strstr( pRequestTarget, pFieldPrefix );
    if ( pTmp1 == NULL ) return;
    pTmp1 += strlen( pFieldPrefix );

    /* pTmp1 points to the field. Now find where it ends. */
    pTmp2 = strstr( pTmp1, pFieldSuffix );
    if ( pTmp2 == NULL ) return;

    /* Length of field in bytes. */
    fieldLength = pTmp2 - pTmp1;

    /* Allocate space for it so that we can convert it from ASCII to Unsigned. */
    if ( (pTmpBuffer = B_Os_Calloc( 1, fieldLength + 1 ) ) == NULL )
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT "Failed to allocate %d bytes" BIP_MSG_PRE_ARG, fieldLength ));
        return;
    }
    strncpy( pTmpBuffer, pTmp1, fieldLength );

    *pFieldValue = strtoul( pTmpBuffer, NULL, 0 );

    if ( pTmpBuffer ) B_Os_Free( pTmpBuffer );

    return;
} /* getFieldValueFromUrl */

static void getProfileIndexFromUrl(
    const char *pUrl,
    unsigned *pProfileIndex
    )
{
    getFieldFromRequestTarget( pUrl, BIP_PLAYLIST_URL_PROFILE_PREFIX, "_", pProfileIndex );
    BDBG_MSG(( BIP_MSG_PRE_FMT "RequestTarget=%s, pFieldValue=%u" BIP_MSG_PRE_ARG, pUrl, *pProfileIndex ));
    return;
} /* getProfileIndexFromUrl */

static void getSegmentSequenceNumFromUrl(
    const char *pUrl,
    unsigned *pSegmentSequenceNum
    )
{
    getFieldFromRequestTarget( pUrl, BIP_SEGMENT_URL_PREFIX, BIP_SEGMENT_URL_SUFFIX, pSegmentSequenceNum );
    BDBG_MSG(( BIP_MSG_PRE_FMT "RequestTarget=%s, pFieldValue=%u" BIP_MSG_PRE_ARG, pUrl, *pSegmentSequenceNum ));
    return;
} /* getSegmentSequenceNumFromUrl */

void BIP_HttpStreamer_GetStreamerIdFromUrl(
    const char *pUrl,
    unsigned *phHttpStreamer
    )
{
    getFieldFromRequestTarget( pUrl, BIP_PLAYLIST_URL_STREAMER_ID_PREFIX, BIP_PLAYLIST_URL_SUFFIX, phHttpStreamer );
    BDBG_MSG(( BIP_MSG_PRE_FMT "RequestTarget=%s, pFieldValue=0x%x" BIP_MSG_PRE_ARG, pUrl, *phHttpStreamer ));
    return;
} /* BIP_HttpStreamer_GetStreamerIdFromUrl */

static void playlistAtomUserFree(
    batom_t atom,
    void *user
    )
{
    char *playlistBuffer = *(char **)user;

    BSTD_UNUSED(atom);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Freeing up playlist buffer %p" BIP_MSG_PRE_ARG, (void *)playlistBuffer ));

    if ( playlistBuffer ) B_Os_Free( playlistBuffer );
}

static const batom_user playlistAtomUser = {
    playlistAtomUserFree,
    sizeof(void **)
};

static BIP_Status createAndPushAtom(
    batom_factory_t     pAtomFactory,
    batom_pipe_t        pAtomPipe,
    const void          *pBuffer,
    size_t              bufferSize
    )
{
    BIP_Status bipStatus;
    batom_t pAtom;

    /* Create an Atom from the buffer range. */
    pAtom = batom_from_range( pAtomFactory, pBuffer, bufferSize, &playlistAtomUser, &pBuffer );
    BIP_CHECK_GOTO(( pAtom ), ( "Failed to create an atom using batom_from_range()" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    /* Push it in the pipe to be processed later. */
    batom_pipe_push( pAtomPipe, pAtom );

    BDBG_MSG(( BIP_MSG_PRE_FMT "pAtom=%p created from range=%p size=%u" BIP_MSG_PRE_ARG, (void *)pAtom, (void *)pBuffer, (unsigned)bufferSize ));
    bipStatus = BIP_SUCCESS;

error:
    return ( bipStatus );
}

static BIP_Status buildMediaPlaylist(
    BIP_HttpStreamerHandle  hHttpStreamer,
    batom_factory_t         pAtomFactory,
    batom_pipe_t            pAtomPipe,
    BIP_TranscodeProfile    *pTranscodeProfile,
    const BIP_StreamerStreamInfo     *pStreamerStreamInfo,
    const char              *pUrl,
    size_t                  *pPlaylistLength
    )
{
    unsigned    i;
    BIP_Status  bipStatus = BIP_SUCCESS;
    char        *pPlaylistBuffer = NULL;
    size_t      bytesLeft = 0;
    size_t      bytesNeeded = 0;
    size_t      bytesCopied = 0;
    bool        playlistHeaderAdded = false;
    bool        wroteCurrentSegmentEntry = false;
    unsigned    numMediaSegments;
    unsigned    mediaSegmentDurationInMs;
    unsigned    addEndTag;
    unsigned    currentMediaSegmentSeq;

    *pPlaylistLength = 0;
    if ( pStreamerStreamInfo->durationInMs && pTranscodeProfile->video.settings.streamStructure.duration )
    {
        mediaSegmentDurationInMs = pTranscodeProfile->video.settings.streamStructure.duration;
        numMediaSegments = pStreamerStreamInfo->durationInMs / mediaSegmentDurationInMs;
        addEndTag = 1;
        currentMediaSegmentSeq = BIP_HLS_SEGMENT_START_SEQUENCE_NUMBER;
    }
    else
    {
        /* We either dont have the total mediaSegment duration as it is a live stream or segment duration, so we pick the # & segment duration. */
        numMediaSegments = 3;
        mediaSegmentDurationInMs = pTranscodeProfile->video.settings.streamStructure.duration;
        addEndTag = 0;
        currentMediaSegmentSeq = hHttpStreamer->hls.numSegmentsStreamed;
    }
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p: segDurationInMs=%d #Segments=%d" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, mediaSegmentDurationInMs, numMediaSegments ));
    /* Add media playlist entry for each media segment. */
    for ( i=0; i<numMediaSegments+addEndTag; i++ )
    {
        wroteCurrentSegmentEntry = false;
        while ( !wroteCurrentSegmentEntry )
        {
            if ( bytesLeft == 0 )
            {
                /* Allocate pPlaylistBuffer. */
                BDBG_ASSERT( pPlaylistBuffer == NULL );
                pPlaylistBuffer = B_Os_Calloc( 1, BIP_PLAYLIST_BUFFER_BLOCK_SIZE+1 );
                BIP_CHECK_GOTO(( pPlaylistBuffer != NULL ), ( "Failed to allocate memory (%d bytes) for pPlaylistBuffer", BIP_PLAYLIST_BUFFER_BLOCK_SIZE ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
                bytesLeft = BIP_PLAYLIST_BUFFER_BLOCK_SIZE;
            }

            /* Add playlist Header if its not yet added. Its only present in the start of the playlist. */
            if ( !playlistHeaderAdded )
            {
                bytesNeeded = snprintf( pPlaylistBuffer, bytesLeft,
                        "#EXTM3U\n"
                        "#EXT-X-VERSION:%d\n"
                        "#EXT-X-TARGETDURATION:%d\n"
                        "#EXT-X-MEDIA-SEQUENCE:%d\n"
                        ,
                        BIP_HLS_SERVER_VERSION, (mediaSegmentDurationInMs/1000 + 1), currentMediaSegmentSeq);
                BDBG_MSG(( BIP_MSG_PRE_FMT "[%d] pBuffer=%p bytesCopied=%u bytesLeft=%u bytesNeeded=%u" BIP_MSG_PRE_ARG, i, (void *)pPlaylistBuffer, (unsigned)bytesCopied, (unsigned)bytesLeft, (unsigned)bytesNeeded ));
                if ( bytesNeeded < bytesLeft )
                {
                    bytesCopied += snprintf( pPlaylistBuffer, bytesLeft,
                            "#EXTM3U\n"
                            "#EXT-X-VERSION:%d\n"
                            "#EXT-X-TARGETDURATION:%d\n"
                            "#EXT-X-MEDIA-SEQUENCE:%d\n"
                            ,
                             BIP_HLS_SERVER_VERSION, (mediaSegmentDurationInMs/1000 + 1), currentMediaSegmentSeq);
                    bytesLeft = BIP_PLAYLIST_BUFFER_BLOCK_SIZE - bytesCopied;
                    playlistHeaderAdded = true;
                }
                else
                {
                    /* Not enough space in the current buffer, so create atom from the current buffer and push it in the pipe to be processed later. */
                    bipStatus = createAndPushAtom( pAtomFactory, pAtomPipe, (const void *)pPlaylistBuffer, bytesCopied );
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "createAndPushAtom() Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
                    bytesLeft = 0;
                    bytesCopied = 0;
                    pPlaylistBuffer = NULL;
                    *pPlaylistLength += bytesCopied;
                    continue;
                }
            }

            /* Add MediaSegment URL tags first. */
            if ( i < numMediaSegments )
            {
            /*
             * Now add the Per Media Segment entry, it contains two lines like this:
             *    #EXTINF:2.0
             *    low0.ts
             *    #EXTINF:2.0
             *    low1.ts
             *
             * We are following this convention for the URL:
             * <URL_BASE_PREFIX>_<URL_PROFILE_PREFIX><profileNum>_<width>_<height>_p<frameRate>_<bitRate>bps_<vCodec>_<aCodec>_<transportType>_<streamerId>_seg<num>.<SegmentUrlSuffix>
             * E.g.
             * com.broadcom.bcg.bse.bip_profile0_1280_720_p30_6000000bps_avc_aac_ts_streamerId0x454948_seg0.ts
             */
                /* Determine the string size of the two lines and make sure current buffer has space to hold them. */
                bytesNeeded = snprintf( pPlaylistBuffer+bytesCopied, 1,
                        "#EXTINF:%d.%d,\n"
                        "%s%s%d%s\n"
                        ,
                        (mediaSegmentDurationInMs/1000), (mediaSegmentDurationInMs%1000),
                        pUrl+1, BIP_SEGMENT_URL_PREFIX, i+currentMediaSegmentSeq, BIP_SEGMENT_URL_SUFFIX       /* Skip one byte as URLs seem to start w/ / */
                        );
                BDBG_MSG(( BIP_MSG_PRE_FMT "[%d] pBuffer=%p bytesCopied=%u bytesLeft=%u bytesNeeded=%u" BIP_MSG_PRE_ARG, i, (void *)pPlaylistBuffer, (unsigned)bytesCopied, (unsigned)bytesLeft, (unsigned)bytesNeeded ));
                if ( bytesNeeded < bytesLeft )
                {
                    bytesCopied += snprintf( pPlaylistBuffer+bytesCopied, bytesLeft,
                        "#EXTINF:%d.%d,\n"
                        "%s%s%d%s\n"
                        ,
                        (mediaSegmentDurationInMs/1000), (mediaSegmentDurationInMs%1000),
                        pUrl+1, BIP_SEGMENT_URL_PREFIX,i+currentMediaSegmentSeq, BIP_SEGMENT_URL_SUFFIX        /* Skip one byte as URLs seem to start w/ / */
                        );
                    bytesLeft = BIP_PLAYLIST_BUFFER_BLOCK_SIZE - bytesCopied;
                    wroteCurrentSegmentEntry = true;
                }
                else
                {
                    /* Not enough space in the current buffer, so create atom from the current buffer and push it in the pipe to be processed later. */
                    bipStatus = createAndPushAtom( pAtomFactory, pAtomPipe, (const void *)pPlaylistBuffer, bytesCopied );
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "createAndPushAtom() Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
                    *pPlaylistLength += bytesCopied;
                    BDBG_MSG(( BIP_MSG_PRE_FMT "[%d] Current Playlist: pPlaylistLength=%u bytesCopied=%u playlist:\n%s" BIP_MSG_PRE_ARG, i, (unsigned)*pPlaylistLength, (unsigned)bytesCopied, pPlaylistBuffer ));
                    pPlaylistBuffer = NULL;
                    bytesLeft = 0;
                    bytesCopied = 0;
                    continue;
                }
            }
            else
            {
                /* Add the END-Tag for bounded playlists. */
                BDBG_ASSERT( addEndTag );
                BDBG_ASSERT( i==numMediaSegments );
                bytesNeeded = snprintf( pPlaylistBuffer+bytesCopied, 1, "#EXT-X-ENDLIST\n");
                BDBG_MSG(( BIP_MSG_PRE_FMT "[%d] pBuffer=%p bytesCopied=%u bytesLeft=%u bytesNeeded=%u" BIP_MSG_PRE_ARG, i, (void *)pPlaylistBuffer, (unsigned)bytesCopied, (unsigned)bytesLeft, (unsigned)bytesNeeded ));
                if ( bytesNeeded < bytesLeft )
                {
                    bytesCopied += snprintf( pPlaylistBuffer+bytesCopied, bytesLeft,"#EXT-X-ENDLIST\n");
                    bytesLeft = BIP_PLAYLIST_BUFFER_BLOCK_SIZE - bytesCopied;
                    wroteCurrentSegmentEntry = true;
                }
                else
                {
                    /* Not enough space in the current buffer, so create atom from the current buffer and push it in the pipe to be processed later. */
                    bipStatus = createAndPushAtom( pAtomFactory, pAtomPipe, (const void *)pPlaylistBuffer, bytesCopied );
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "createAndPushAtom() Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
                    bytesLeft = 0;
                    pPlaylistBuffer = NULL;
                    *pPlaylistLength += bytesCopied;
                    bytesCopied = 0;
                    continue;
                }
            }
        }
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "[%d] pBuffer=%p bytesCopied=%u bytesLeft=%u bytesNeeded=%u" BIP_MSG_PRE_ARG, i, (void *)pPlaylistBuffer, (unsigned)bytesCopied, (unsigned)bytesLeft, (unsigned)bytesNeeded ));
    if ( pPlaylistBuffer )
    {
        bipStatus = createAndPushAtom( pAtomFactory, pAtomPipe, (const void *)pPlaylistBuffer, bytesCopied );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "createAndPushAtom() Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
        *pPlaylistLength += bytesCopied;
        BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpStreamer %p: playlistLength=%u" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, (unsigned)*pPlaylistLength ));
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: playlistLength=%u, MediaPlaylist \n%s" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, (unsigned)*pPlaylistLength, pPlaylistBuffer ));
        pPlaylistBuffer = NULL;
        bytesCopied = 0;
    }
    bipStatus = BIP_SUCCESS;

error:
    if ( bipStatus != BIP_SUCCESS )
    {
        batom_pipe_flush( pAtomPipe );
        if ( pPlaylistBuffer ) B_Os_Free( pPlaylistBuffer );
    }
    return ( bipStatus );
} /* buildMediaPlaylist */

static BIP_TranscodeProfile *findStreamerTranscodeProfile(
    BIP_HttpStreamerHandle  hHttpStreamer,
    unsigned                profileIndex
    )
{
    BIP_StreamerTranscodeProfileListEntry *pTranscodeProfileEntry = NULL;

    /* We find the transcodeProfile info for profile index and use it to build the MediaSegment Playlist.  */
    for (
            pTranscodeProfileEntry = BLST_Q_FIRST( &hHttpStreamer->pStreamer->transcode.profileListHead );
            pTranscodeProfileEntry;
            pTranscodeProfileEntry = BLST_Q_NEXT( pTranscodeProfileEntry , transcodeProfileListNext )
        )
    {
        if ( pTranscodeProfileEntry->profileIndex == profileIndex )
            break;
    }
    if ( pTranscodeProfileEntry ) return ( &pTranscodeProfileEntry->transcodeProfile );
    else return NULL;
} /* findStreamerTranscodeProfile */

static BIP_Status prepareMediaPlaylist(
    BIP_HttpStreamerHandle      hHttpStreamer,
    const char                  *pUrl,
    const BIP_StreamerStreamInfo   *pStreamerStreamInfo,
    unsigned                    profileIndex,
    batom_factory_t             pAtomFactory,
    batom_pipe_t                pAtomPipe,
    size_t                      *pPlaylistLength
    )
{
    BIP_Status bipStatus;
    BIP_TranscodeProfile *pTranscodeProfile = NULL;

    pTranscodeProfile = findStreamerTranscodeProfile( hHttpStreamer, profileIndex );
    if ( pTranscodeProfile )
    {
        bipStatus = buildMediaPlaylist( hHttpStreamer, pAtomFactory, pAtomPipe, pTranscodeProfile, pStreamerStreamInfo, pUrl, pPlaylistLength );
    }
    else
    {
        bipStatus = BIP_ERR_INTERNAL;
        BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer=%p hlsState=%s: Didn't find transcode profile entry for profileIndex=%u" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_HLS_STATE( hHttpStreamer->hls.state ), profileIndex ));
    }

    return ( bipStatus );
} /* prepareMediaPlaylist */

static BIP_Status prepareMasterPlaylist(
    BIP_HttpStreamerHandle  hHttpStreamer,
    batom_factory_t         pAtomFactory,
    batom_pipe_t            pAtomPipe,
    const struct transcodeProfileListHead *pTranscodeProfileListHead,
    size_t                  *pPlaylistLength
    )
{
    BIP_StreamerTranscodeProfileListEntry   *pTranscodeProfileEntry;
    BIP_Status  bipStatus = BIP_SUCCESS;
    char        *pPlaylistBuffer = NULL;
    size_t      bytesLeft = 0;
    size_t      bytesNeeded = 0;
    size_t      bytesCopied = 0;
    bool        playlistHeaderAdded = false;
    bool        wroteCurrentProfileEntry = false;

    *pPlaylistLength = 0;
    /* Add media playlist entry for each transcode profile. */
    for (
            pTranscodeProfileEntry = BLST_Q_FIRST( pTranscodeProfileListHead );
            pTranscodeProfileEntry;
            pTranscodeProfileEntry = BLST_Q_NEXT( pTranscodeProfileEntry, transcodeProfileListNext )
        )
    {
        wroteCurrentProfileEntry = false;
        while ( !wroteCurrentProfileEntry )
        {
            if ( bytesLeft == 0 )
            {
                /* Allocate pPlaylistBuffer. */
                pPlaylistBuffer = B_Os_Calloc( 1, BIP_PLAYLIST_BUFFER_BLOCK_SIZE+1 );
                BIP_CHECK_GOTO(( pPlaylistBuffer != NULL ), ( "Failed to allocate memory (%d bytes) for pPlaylistBuffer", BIP_PLAYLIST_BUFFER_BLOCK_SIZE ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
                bytesLeft = BIP_PLAYLIST_BUFFER_BLOCK_SIZE;
            }

            /* Add playlist Header if its not yet added. Its only present in the start of the playlist. */
            if ( !playlistHeaderAdded )
            {
                bytesNeeded = snprintf( pPlaylistBuffer, 1, "#EXTM3U\n" );
                if ( bytesNeeded < bytesLeft )
                {
                    bytesCopied += snprintf( pPlaylistBuffer, bytesLeft, "#EXTM3U\n" );
                    bytesLeft = BIP_PLAYLIST_BUFFER_BLOCK_SIZE - bytesCopied;
                    playlistHeaderAdded = true;
                }
                else
                {
                    /* Not enough space in the current buffer, so create atom from the current buffer and push it in the pipe to be processed later. */
                    bipStatus = createAndPushAtom( pAtomFactory, pAtomPipe, (const void *)pPlaylistBuffer, bytesCopied );
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "createAndPushAtom() Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
                    bytesLeft = 0;
                    bytesCopied = 0;
                    pPlaylistBuffer = NULL;
                    *pPlaylistLength += bytesCopied;
                    continue;
                }
            }

            /*
             * Now add the Per Profile entry, it contains two lines per profile like this:
             *    #EXT-X-STREAM-INF:BANDWIDTH=1280000
             *    low.m3u8
             * We are following this convention for the URL:
             * <URL_BASE_PREFIX>_<URL_PROFILE_PREFIX><profileNum>_<width>_<height>_p<frameRate>_<bitRate>bps_<vCodec>_<aCodec>_<transportType>_<streamerId>.<PlaylistPrefix>
             * E.g.
             * com.broadcom.bcg.bse.bip_profile0_1280_720_p30_6000000bps_avc_aac_ts_streamerId0x454948.m3u8
             */
            {
                /* Determine the string size of the two lines and make sure current buffer has space to hold them. */
                bytesNeeded = snprintf( pPlaylistBuffer+bytesCopied, 1,
                        "#EXT-X-STREAM-INF:PROGRAM-ID=1, BANDWIDTH=%d\n"
                        "%s%s%d_%d_%d_p%s_%dbps_%s_%s_%s%s%p%s\n",
                        pTranscodeProfileEntry->transcodeProfile.video.settings.bitrateMax,
                        BIP_PLAYLIST_URL_BASE_PREFIX,
                        BIP_PLAYLIST_URL_PROFILE_PREFIX, pTranscodeProfileEntry->profileIndex,
                        pTranscodeProfileEntry->transcodeProfile.video.width, pTranscodeProfileEntry->transcodeProfile.video.height,
                        lookup_name( g_videoFrameRateStrs, pTranscodeProfileEntry->transcodeProfile.video.settings.frameRate ),
                        pTranscodeProfileEntry->transcodeProfile.video.settings.bitrateMax,
                        lookup_name( g_videoCodecStrs, pTranscodeProfileEntry->transcodeProfile.video.startSettings.codec ),
                        lookup_name( g_audioCodecStrs, pTranscodeProfileEntry->transcodeProfile.audio.audioCodec ),
                        lookup_name( g_transportTypeStrs, pTranscodeProfileEntry->transcodeProfile.containerType ),
                        BIP_PLAYLIST_URL_STREAMER_ID_PREFIX, (void *)hHttpStreamer,
                        BIP_PLAYLIST_URL_SUFFIX
                        );
                if ( bytesNeeded < bytesLeft )
                {
                    bytesCopied += snprintf( pPlaylistBuffer+bytesCopied, bytesLeft,
                            "#EXT-X-STREAM-INF:PROGRAM-ID=1, BANDWIDTH=%d\n"
                            "%s%s%d_%d_%d_p%s_%dbps_%s_%s_%s%s%p%s\n",
                            pTranscodeProfileEntry->transcodeProfile.video.settings.bitrateMax,
                            BIP_PLAYLIST_URL_BASE_PREFIX,
                            BIP_PLAYLIST_URL_PROFILE_PREFIX, pTranscodeProfileEntry->profileIndex,
                            pTranscodeProfileEntry->transcodeProfile.video.width, pTranscodeProfileEntry->transcodeProfile.video.height,
                            lookup_name( g_videoFrameRateStrs, pTranscodeProfileEntry->transcodeProfile.video.settings.frameRate ),
                            pTranscodeProfileEntry->transcodeProfile.video.settings.bitrateMax,
                            lookup_name( g_videoCodecStrs, pTranscodeProfileEntry->transcodeProfile.video.startSettings.codec ),
                            lookup_name( g_audioCodecStrs, pTranscodeProfileEntry->transcodeProfile.audio.audioCodec ),
                            lookup_name( g_transportTypeStrs, pTranscodeProfileEntry->transcodeProfile.containerType ),
                            BIP_PLAYLIST_URL_STREAMER_ID_PREFIX, (void *)hHttpStreamer,
                            BIP_PLAYLIST_URL_SUFFIX
                            );
                    bytesLeft = BIP_PLAYLIST_BUFFER_BLOCK_SIZE - bytesCopied;
                    wroteCurrentProfileEntry = true;
                }
                else
                {
                    /* Not enough space in the current buffer, so create atom from the current buffer and push it in the pipe to be processed later. */
                    bipStatus = createAndPushAtom( pAtomFactory, pAtomPipe, (const void *)pPlaylistBuffer, bytesCopied );
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "createAndPushAtom() Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
                    bytesLeft = 0;
                    pPlaylistBuffer = NULL;
                    *pPlaylistLength += bytesCopied;
                    continue;
                }
            }
        }
    }

    if ( pPlaylistBuffer )
    {
        bipStatus = createAndPushAtom( pAtomFactory, pAtomPipe, (const void *)pPlaylistBuffer, bytesCopied );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "createAndPushAtom() Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
        *pPlaylistLength += bytesCopied;
        BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpStreamer %p: MasterPlaylistLength=%u" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, (unsigned)*pPlaylistLength ));
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: MasterPlaylist \n%s" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, pPlaylistBuffer ));
        pPlaylistBuffer = NULL;
    }
    bipStatus = BIP_SUCCESS;

error:
    if ( bipStatus != BIP_SUCCESS )
    {
        batom_pipe_flush( pAtomPipe );
        if ( pPlaylistBuffer ) B_Os_Free( pPlaylistBuffer );
    }
    return ( bipStatus );
} /* prepareMasterPlaylist */

static BIP_Status sendHttpPayloadUsingAtom(
    batom_pipe_t pAtomPipe,
    BIP_HttpSocketHandle hHttpSocket
    )
{
    BIP_Status  bipStatus = BIP_SUCCESS;
    batom_t     pAtom = NULL;

    while ( bipStatus == BIP_SUCCESS && (pAtom = batom_pipe_pop( pAtomPipe ) ) != NULL )
    {
        /* Send Payload. */
        BIP_HttpSocketSendPayloadSettings sendPayloadSettings;

        BIP_HttpSocket_GetDefaultSendPayloadSettings( &sendPayloadSettings );
        sendPayloadSettings.timeoutInMs = -1; /* blocking call, should complete immediately for <250K write size. */
        /* Iterate thru the each vector inside atom and send that. */
        {
            unsigned i, numVectors;
            const   batom_vec *pAtomVector;

            pAtomVector = batom_get_vectors( pAtom, &numVectors );
            for( i=0; i<numVectors; i++ )
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket=%p pAtom=%p, Sending vector[%d]=%u Size" BIP_MSG_PRE_ARG, (void *)hHttpSocket, (void *)pAtom, i, pAtomVector[i].len ));
                bipStatus = BIP_HttpSocket_SendPayload( hHttpSocket, (uint8_t *)pAtomVector[i].base, pAtomVector[i].len, &sendPayloadSettings);
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpSocket_SendPayload() Failed" ), error, bipStatus, bipStatus );
            }
        }

        batom_release( pAtom );
        pAtom = NULL;
        /* TODO: this is a work-around for a possible issue in the HttpSocket_Send() where if it is called too fast, it seems to lock up. */
        /* I will debug this later after stablizing the transcode. */
        BKNI_Sleep(10);
    }
error:
    if ( pAtom ) batom_release( pAtom );
    return ( bipStatus );
}

static BIP_Status isHttpPayloadRequested(
        BIP_HttpRequestHandle       hHttpRequest,
        bool                        *sendPayload
        )
{
    BIP_Status                  bipStatus;
    BIP_HttpRequestMethod       method;
    const char                  *pMethodName = NULL;

    BIP_CHECK_GOTO(( hHttpRequest ), ( "hHttpRequest can't be NULL!" ), error, BIP_ERR_INVALID_PARAMETER, bipStatus );
    bipStatus = BIP_HttpRequest_GetMethod( hHttpRequest, &method, &pMethodName);
    if ( bipStatus == BIP_SUCCESS ) *sendPayload = (method == BIP_HttpRequestMethod_eHead) ? false : true;

error:
    return ( bipStatus );
} /* isHttpPayloadRequested */

static BIP_Status prepareStreamerForTranscode(
    BIP_HttpStreamerHandle  hHttpStreamer,
    BIP_TranscodeProfile    *pTranscodeProfile,
    unsigned                initialSeekPosition
    )
{
    BIP_Status bipStatus;
    BIP_StreamerPrepareSettings prepareSettings;

    BIP_Streamer_GetDefaultPrepareSettings( &prepareSettings );
    /* We have already verified during start that EncoderProfile is set by app, */
    /* so we use 1st profile as it will have the smallest b/w & resolution. */
    /* Streamer objects keeps the transcode profile list sorted by the video bitrate. */
    if ( pTranscodeProfile == NULL )
    {
        BIP_StreamerTranscodeProfileListEntry *pTranscodeProfileEntry;
        pTranscodeProfileEntry = BLST_Q_FIRST( &hHttpStreamer->pStreamer->transcode.profileListHead );
        BDBG_ASSERT( pTranscodeProfileEntry );
        prepareSettings.pTranscodeProfile = hHttpStreamer->hls.pCurrentTranscodeProfile = &pTranscodeProfileEntry->transcodeProfile;
        hHttpStreamer->hls.currentProfileIndex = 0;
    }
    else
    {
        prepareSettings.pTranscodeProfile = pTranscodeProfile;
    }
    /* TODO: need to tune the dataReadyThreshold, this determines when the streamer will get the dataReadyCallback from Recpump. */
    /* We have to have a balance between latency & system call overhead to send the data on the network. */
    /* For now, we double it. */
    prepareSettings.recpumpOpenSettings.data.dataReadyThreshold = prepareSettings.recpumpOpenSettings.data.atomSize * 2;
    prepareSettings.enableRaiIndex = true; /* this enables the TPIT indexing in RAVE/Recpump that aids in HLS Segmention. */
    if ( initialSeekPosition )
    {
        prepareSettings.seekPositionInMs = initialSeekPosition;
        prepareSettings.seekPositionValid = true;
    }
    bipStatus = BIP_Streamer_Prepare( hHttpStreamer->hStreamer, &prepareSettings );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Streamer_Prepare Failed!" ), error, bipStatus, bipStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p state=%s hlsState=%s: BIP_Streamer_Prepare is successful!"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_HTTP_STREAMER_HLS_STATE(hHttpStreamer->hls.state) ));
error:
    return ( bipStatus );
} /* prepareStreamerForTranscode */

static BIP_Status checkAndUpdateStreamerSettings(
    BIP_HttpStreamerHandle  hHttpStreamer,
    unsigned                requestedProfileIndex,
    unsigned                currentProfileIndex,
    unsigned                requestedSegmentSequenceNum,
    unsigned                expectedSegmentSequenceNum
    )
{
    BIP_Status              bipStatus;
    BIP_StreamerSettings    streamerSettings;
    BIP_TranscodeProfile    transcodeProfile;

    streamerSettings.pTranscodeProfile = &transcodeProfile;
    BIP_Streamer_GetSettings( hHttpStreamer->hStreamer, &streamerSettings );

    if ( requestedProfileIndex != currentProfileIndex )
    {
        /* Requested PlaylistProfile doesn't match the current PlaylistProfile, so we need to get the associated transcode settings w/ this profile and set them. */
        streamerSettings.pTranscodeProfile = findStreamerTranscodeProfile( hHttpStreamer, requestedProfileIndex );
        BIP_CHECK_GOTO( (streamerSettings.pTranscodeProfile ), ( "hHttpStreamer=%p hlsState=%s: Didn't find transcode profile entry for profileIndex=%u", (void *)hHttpStreamer, BIP_HTTP_STREAMER_HLS_STATE( hHttpStreamer->hls.state ), requestedProfileIndex ), error, BIP_ERR_INVALID_REQUEST_TARGET, bipStatus );
        hHttpStreamer->hls.currentProfileIndex = requestedProfileIndex;
        BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpStreamer=%p state=%s: Next Media Segment requested from different profileIndex: cur=%u new=%u videoBitrate=%u " BIP_MSG_PRE_ARG,
                    (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), currentProfileIndex, requestedProfileIndex, streamerSettings.pTranscodeProfile->video.settings.bitrateMax ));
    }

    if ( requestedSegmentSequenceNum != expectedSegmentSequenceNum )
    {
        /* Only seek to a different position if input media stream has a known duration (e.g. a media file). */
        if ( hHttpStreamer->pStreamer->streamerStreamInfo.durationInMs > 0 )
        {
            /* Requested segment seq# doesn't match the expected next seq#, so we will need tell streamer to seek to the new position. */
            streamerSettings.seekPositionValid = true;
            streamerSettings.seekPositionInMs =  requestedSegmentSequenceNum * transcodeProfile.video.settings.streamStructure.duration;
            BIP_CHECK_GOTO( ( streamerSettings.seekPositionInMs < hHttpStreamer->hStreamer->streamerStreamInfo.durationInMs ),
                    ( "Requeste Segment# (%u) points to an invalid duration=%u", requestedSegmentSequenceNum,  hHttpStreamer->hStreamer->streamerStreamInfo.durationInMs )
                    , error, BIP_ERR_INVALID_REQUEST_TARGET, bipStatus );
            BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpStreamer=%p state=%s: Next Media Segment requested from position index: cur=%u new=%u " BIP_MSG_PRE_ARG,
                        (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), expectedSegmentSequenceNum, requestedSegmentSequenceNum ));
        }
        hHttpStreamer->hls.resetStreaming = true;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p hlsState=%s: profileIndex cur=%u req=%u, segSeq cur=%u req=%u, seekPositionInMs=%u" BIP_MSG_PRE_ARG,
                (void *)hHttpStreamer, BIP_HTTP_STREAMER_HLS_STATE( hHttpStreamer->hls.state ),
                currentProfileIndex, requestedProfileIndex,
                hHttpStreamer->hls.currentSegmentSequenceNumber, requestedSegmentSequenceNum,
                streamerSettings.seekPositionInMs ));
    /* Update Streamer's runtime settings. */
    bipStatus = BIP_Streamer_SetSettings( hHttpStreamer->hStreamer, &streamerSettings );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Streamer_SetSettings Failed!" ), error, bipStatus, bipStatus );

error:
    return ( bipStatus );
} /* checkAndUpdateStreamerSettings */

static BIP_Status getHttpSocketAndRequestInfo (
    BIP_HttpSocketHandle        hHttpSocket,
    BIP_HttpRequestHandle       hHttpRequest,
    const char                  **ppRequestTarget,
    bool                        *pSendPayload,
    int                         *pSocketFd
    )
{
    BIP_Status bipStatus;

    /* Determine if the Request contains HEAD or GET Method and thus if we need to actually send the payload. */
    bipStatus = isHttpPayloadRequested( hHttpRequest, pSendPayload );

    *pSocketFd = -1;

    /* Get the Request Target (loosely called URL in this code :-( ) */
    if (bipStatus == BIP_SUCCESS)
    {
        bipStatus = BIP_HttpRequest_GetTarget( hHttpRequest, ppRequestTarget );
    }

    if (bipStatus == BIP_SUCCESS)
    {
        bipStatus = getSocketFdFromHttpSocket( hHttpSocket, pSocketFd );
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "bipStatus=%s hHttpSocket=%p hHttpRequest=%p socketFd=%d sendPayload=%s RequestTarget=%s"
                BIP_MSG_PRE_ARG, BIP_StatusGetText( bipStatus ), (void *)hHttpSocket, (void *)hHttpRequest, *pSocketFd, *pSendPayload?"Y":"N", *ppRequestTarget ));
    return ( bipStatus );
} /* getHttpSocketAndRequestInfo */

static BIP_Status isRequestForMasterPlaylist(
    BIP_HttpStreamerHandle hHttpStreamer,
    const char *pRequestTarget
    )
{
    BSTD_UNUSED( pRequestTarget );

    if ( hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingForMasterPlaylistReq )
        /* Since we dont know the URL for the Master Playlist (as app exposes this URL), we can only go by the HLS state. */
        return true;
    else
        return false;
} /* isRequestForMasterPlaylist */

static BIP_Status isRequestForMediaPlaylist(
    const char *pRequestTarget
    )
{
    if ( strstr( pRequestTarget, BIP_PLAYLIST_URL_SUFFIX ) &&           /* RequestTarget contains Playlist URL suffix and */
            strstr( pRequestTarget, BIP_SEGMENT_URL_PREFIX ) == NULL && /* doesn't contain Segment URL Prefix & Suffix. */
            strstr( pRequestTarget, BIP_SEGMENT_URL_SUFFIX ) == NULL )
        return ( true );
    else
        return ( false );
} /* isRequestForMediaPlaylist */

static BIP_Status isRequestForMediaSegment(
    const char *pRequestTarget
    )
{
    if ( strstr( pRequestTarget, BIP_PLAYLIST_URL_SUFFIX ) &&   /* RequestTarget contains Playlist URL suffix and */
            strstr( pRequestTarget, BIP_SEGMENT_URL_PREFIX ) && /* it does contain Segment URL Prefix & Suffix. */
            strstr( pRequestTarget, BIP_SEGMENT_URL_SUFFIX ) )
        return ( true );
    else
        return ( false );
} /* isRequestForMediaSegment */

static BIP_Status prepareAndSendMasterPlaylist(
    BIP_HttpStreamerHandle  hHttpStreamer,
    bool                    sendPayload,
    bool                    *pSuccessfullResponseSent
    )
{
    BIP_Status bipStatus;
    size_t messageLength = 0;

    /* Prepare the Media Playlist in memory so that we can determine the messageLength to be included in the HTTP Response. */
    {
        /* Prepare the playlist using the provided Atom factory & queue them into the atomPipe. */
        bipStatus = prepareMasterPlaylist( hHttpStreamer, hHttpStreamer->atomFactory, hHttpStreamer->atomPipe, &hHttpStreamer->pStreamer->transcode.profileListHead, &messageLength );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "prepareMasterPlaylist Failed!" ), error, bipStatus, bipStatus );
    }

    /* Successful in preparing the MasterPlaylist, prepare and send HTTP Response . */
    {
        bipStatus = finalizeHeadersAndSendResponseForHls( hHttpStreamer,
                hHttpStreamer->processRequestApi.hHttpSocket, hHttpStreamer->processRequestApi.pSettings->hHttpRequest,
                hHttpStreamer->hls.pCurrentTranscodeProfile, messageLength, false /*requestForMediaSegment */, pSuccessfullResponseSent );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "finalizeHeadersAndSendResponseForHls Failed!" ), error, bipStatus, bipStatus );
    }

    /* We have successfully sent the Response for the MasterPlaylist. Send the media playlist if we are sending the payload. */
    if ( sendPayload)
    {
        bipStatus = sendHttpPayloadUsingAtom( hHttpStreamer->atomPipe, hHttpStreamer->processRequestApi.hHttpSocket );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "sendHttpPayloadUsingAtom Failed!" ), error, bipStatus, bipStatus );
    }

    bipStatus = BIP_SUCCESS;

error:
    return ( bipStatus );
} /* prepareAndSendMasterPlaylist */

static BIP_Status prepareAndSendMediaPlaylist(
    BIP_HttpStreamerHandle  hHttpStreamer,
    const char              *pRequestTarget,
    bool                    sendPayload,
    bool                    *pSuccessfullResponseSent
    )
{
    BIP_Status  bipStatus;
    size_t      messageLength = 0;
    unsigned    requestedProfileIndex;

    /* Prepare the Media Playlist in memory so that we can determine the messageLength to be included in the HTTP Response. */
    {
        /* Get the profile index that is requested. Defaults to 0. */
        getProfileIndexFromUrl( pRequestTarget, &requestedProfileIndex );

        /* And then prepare the playlist using the provided Atom factory & queue them into the atomPipe. */
        bipStatus = prepareMediaPlaylist(
                hHttpStreamer, pRequestTarget,
                &hHttpStreamer->pStreamer->streamerStreamInfo, requestedProfileIndex,
                hHttpStreamer->atomFactory, hHttpStreamer->atomPipe,
                &messageLength );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "prepareMediaPlaylist Failed!" ), error, bipStatus, bipStatus );
    }

    /* Successful in preparing the MediaPlaylist, prepare and send HTTP Response . */
    {
        bipStatus = finalizeHeadersAndSendResponseForHls( hHttpStreamer,
                hHttpStreamer->processRequestApi.hHttpSocket, hHttpStreamer->processRequestApi.pSettings->hHttpRequest,
                hHttpStreamer->hls.pCurrentTranscodeProfile, messageLength, false /*requestForMediaSegment */, pSuccessfullResponseSent );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "finalizeHeadersAndSendResponseForHls Failed!" ), error, bipStatus, bipStatus );
    }

    /* We have successfully sent the Response for the MediaPlaylist. Send the media playlist if we are sending the payload. */
    if ( sendPayload)
    {
        bipStatus = sendHttpPayloadUsingAtom( hHttpStreamer->atomPipe, hHttpStreamer->processRequestApi.hHttpSocket );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "sendHttpPayloadUsingAtom Failed!" ), error, bipStatus, bipStatus );
    }

    bipStatus = BIP_SUCCESS;

error:
    return ( bipStatus );
} /* prepareAndSendMediaPlaylist */

/* This function can be called either when ProcessRequest() is called or later when PBIP Streamer finishes & we have a pending Request. */
static BIP_Status processRequestForMediaSegment(
    BIP_HttpStreamerHandle          hHttpStreamer,
    const char                      *pRequestTarget,
    bool                            sendPayload,
    int                             socketFd,
    BIP_HttpStreamerRequestInfo     *currentRequest,
    bool                            *pSuccessfullResponseSent
    )
{
    BIP_Status bipStatus;
    unsigned expectedSegmentSequenceNum;
    unsigned requestedSegmentSequenceNum;
    unsigned requestedProfileIndex;
    int64_t  messageLength;

    if ( hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eStreaming ||  hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingForEndOfSegmentCallback )
    {
        /*
         * We got a Request for Media Segment while still in the Streaming state.
         * This means that client's Request arrived before PBIP's end of streaming callback.
         * This can't happen in the normal scenario as client can't ask for a next segment until server closes the previous segment
         * and the socket close is only done after PBIP CB in this flow -> HttpStreamer -> HttpServerSocket -> HttpSocket.
         *
         * So this request must be due to either Player Seek or player app not getting our previous segment data within some timeout interval.
         * We will need to cache this request and wait for PBIP to finish streaming previous segment before processing this new request.
         * In the above case, PBIP will get the network send error and give endOfStreaming callback. So this request should get processed soon.
         *
         * Also, we can be asked to process a request when we were in the WaitingForEndOfSegmentCallback state.
         * That can happen when client may send us a multiple requests before we finish the current segment streaming.
         * In this case, we cache only the last request from the client as the pending request and keep rejecting the previous ones.
         */

        /* Before we cache this request, check if we had previously cached another request. If so, we send error on that and mark it complete. */
        if ( hHttpStreamer->pendingRequest.valid )
        {
            sendErrorResponse( hHttpStreamer, hHttpStreamer->pendingRequest.hHttpSocket, hHttpStreamer->pendingRequest.settings.hHttpRequest );
            if (hHttpStreamer->pendingRequest.requestProcessedCallback.callback)
            {
                BIP_Arb_AddDeferredCallback( hHttpStreamer->processRequestApi.hArb, &hHttpStreamer->pendingRequest.requestProcessedCallback );
                hHttpStreamer->processRequest.callbackIssued = true;
            }
        }

        /* Cache the new request as the pending request. This will get processed after we get the PBIP endOfStreaming Callback. */
        {
            hHttpStreamer->pendingRequest.requestProcessedCallback = *hHttpStreamer->processRequestApi.pRequestProcessedCallback;
            hHttpStreamer->pendingRequest.hHttpSocket = hHttpStreamer->processRequestApi.hHttpSocket;
            hHttpStreamer->pendingRequest.settings = *hHttpStreamer->processRequestApi.pSettings;
            hHttpStreamer->pendingRequest.valid = true;
        }

        /* Note: we mark this ProcessRequest complete as it is an async call and we will issue the callback later.  */
        bipStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p, state=%s hlsState=%s: We are still streaming, so queued this request for processing after finishing current one. RequestTarget=%s"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_HTTP_STREAMER_HLS_STATE(hHttpStreamer->hls.state), pRequestTarget ));
    }
    else if ( hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingFor1stMediaSegmentReq ||
              hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingForNextMediaSegmentReq ||
              hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eStreamingDone )
    {
        /*
         * We can be in WaitingForMediaSegment, timedWaitingForMediaSegment, or StreamingDone states and thus can immediately start processing this request.
         *
         * If are in StreamingDone state (meaning we have finished streaming & gotten the callback & thus changed our state from Streaming to StreamingDone)
         * but we got a request before ARB timer got to run this state machine.
         * [Streaming] --PBIP CB--> [StreamingDone] --ProcessReqeuest--> here...
         *
         * We will process the request here just like we would in the WaitingForMediaSegment states.
         * The ARB timer will later run the HlsStreamer state machine and do nothing as we will be in the Streaming state.
         */

        /* Get the requested playlist profile index. */
        getProfileIndexFromUrl( pRequestTarget, &requestedProfileIndex );

        /* Get the requested segment seq#. */
        getSegmentSequenceNumFromUrl( pRequestTarget, &requestedSegmentSequenceNum );

        /* Copy the current request parameters. */
        hHttpStreamer->processRequest.requestProcessedCallback = currentRequest->requestProcessedCallback;
        hHttpStreamer->processRequest.hHttpSocket = currentRequest->hHttpSocket;
        hHttpStreamer->processRequest.settings = currentRequest->settings;

        if ( hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingFor1stMediaSegmentReq )
        {
            /* For very 1st segment request, we are expecting request for the 1st segment#. */
            expectedSegmentSequenceNum = hHttpStreamer->hls.currentSegmentSequenceNumber;
        }
        else
        {
            /* For subsequent segment requests, we should be getting request for current+1 sequence# unless it is a seek event. */
            expectedSegmentSequenceNum = hHttpStreamer->hls.currentSegmentSequenceNumber + 1;
        }
        if ( sendPayload )
        {
            /* Call function to update the streamer settings based on the profileIndex & segSeq# (if they have changed from the previous request. ) */
            bipStatus = checkAndUpdateStreamerSettings( hHttpStreamer, requestedProfileIndex, hHttpStreamer->hls.currentProfileIndex, requestedSegmentSequenceNum, expectedSegmentSequenceNum );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "checkAndUpdateStreamerSettings Failed!" ), error, bipStatus, bipStatus );
        }
        messageLength = 0; /* Since we xcode the segment, we dont apriori know the message length. */

        /* If we can process the request successfully, then prepare & send the response. */
        {
            bipStatus = finalizeHeadersAndSendResponseForHls( hHttpStreamer,
                    hHttpStreamer->processRequest.hHttpSocket, hHttpStreamer->processRequest.settings.hHttpRequest,
                    hHttpStreamer->hls.pCurrentTranscodeProfile, messageLength, true /*requestForMediaSegment*/, pSuccessfullResponseSent );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "finalizeHeadersAndSendResponseForHls Failed!" ), error, bipStatus, bipStatus );
        }

        /* We have sent the Response. Now setup the PBIP streamer. */
        if ( sendPayload )
        {
            if ( hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingFor1stMediaSegmentReq )
            {
                /* This is the 1stMediaSegment being sent or one after the request timeout, we will need to [re]start streamer & setup PBIP streamer. */
                bipStatus = BIP_Streamer_Start( hHttpStreamer->hStreamer, NULL );
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Streamer_Start Failed!" ), error, bipStatus, bipStatus );

                bipStatus = createAndStartPBipStreamer( hHttpStreamer, socketFd );
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "createAndStartPBipStreamer Failed!" ), error, bipStatus, bipStatus );
            }
            else /* _eWaitingForNextMediaSegmentReq || eStreamingDone */
            {
                /* For subsequent segment request, we just need to resume streaming w/ PBIP. */
                bipStatus = resumePBipStreamer( hHttpStreamer, socketFd );
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "resumePBipStreamer Failed!" ), error, bipStatus, bipStatus );
            }

            /* Update the mediaSegmentSeq# */
            hHttpStreamer->hls.currentSegmentSequenceNumber = requestedSegmentSequenceNum;
            hHttpStreamer->currentStreamingFd = socketFd;
        }
        bipStatus = BIP_SUCCESS;
    }
    else
    {
        bipStatus = BIP_ERR_INVALID_PARAMETER;
        BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer=%p, state=%s hlsState=%s: MediaSegment Request is not allowed in this state: RequestTarget=%s"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_HTTP_STREAMER_HLS_STATE(hHttpStreamer->hls.state), pRequestTarget ));
    }

error:
    return ( bipStatus );
} /* processRequestForMediaSegment */

/* Function to handle HLS adaptive streaming specific logic. */
static void processHttpHlsStreamerState(
    BIP_HttpStreamerHandle hHttpStreamer,
    int value,
    BIP_Arb_ThreadOrigin threadOrigin
    )
{
    bool reRunProcessState;
    bool successfullResponseSent = false;
    bool sendPayload = true;
    const char *pRequestTarget = NULL;
    int socketFd;
    bool requestForMediaSegment = false;

    BSTD_UNUSED(value);
    BSTD_UNUSED(threadOrigin);

    BDBG_ASSERT(hHttpStreamer);
    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer);

    if ( hHttpStreamer->state == BIP_HttpStreamerState_eIdle && BIP_Arb_IsBusy(hHttpStreamer->processRequestApi.hArb) == true )
    {
        /* Streamer has gone back to the idle state (after Stop) but we just a new request. Return error. */
        return;
    }
    if ( hHttpStreamer->state == BIP_HttpStreamerState_eIdle && BIP_Arb_IsBusy(hHttpStreamer->destroyApi.hArb) != true )
    {
        /* No specific work to do if we are still in Idle state & its not a _Destroy(). */
        return;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTRY ---> hHttpStreamer=%p state=%s hlsState=%s"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_HTTP_STREAMER_HLS_STATE(hHttpStreamer->hls.state) ));

    /*
     * This state machine can be run by following events:
     * -Start() starts the Streamer which essentially does some parameter validation and waits for the ProcessRequest() to start streaming using 1st Request.
     *
     * -ProcessRequest() has a new request for Hls streamer. We will take the action based on current hlsState.
     *  Note: main processState function has already ensured that this API can be called only in WaitingForProcessRequestApi state.
     *  In HLS case, HttpStreamer will receive first Request for Master Playlist, then for Media Playlist, followed by requests for Media Segments.
     *
     * -
     */
    reRunProcessState = true;
    while (reRunProcessState)
    {
        reRunProcessState = false;
        if ( BIP_Arb_IsBusy(hHttpStreamer->startApi.hArb) == true )
        {
            /* Main processState function has already ensured that app can call _Start() in only Idle state. */
            /* Also, main thread had changed the state to BIP_HttpStreamerState_eSetupComplete to indicate us */
            /* that all input & output related settings have been setup. */

            hHttpStreamer->completionStatus = BIP_SUCCESS;

            /* Check if transcode profile has been set by the app. */
            if ( hHttpStreamer->pStreamer->transcode.profileState == BIP_StreamerOutputState_eNotSet )
            {
                /* Note:
                 * Since HLS streaming requires media stream to be segmented and only AVC-MPEG2-TS format is supported by HLS spec,
                 * and since multiple profiles of a stream may need to be supported for adaptive streaming, we require the usage of
                 * transcode pipe for HLS streaming of the stream.
                 *
                 * If an app already has a pre-segmented HLS ready media stream available, then it can just use the direct streaming
                 * protocol to send out individual segments.
                 */
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: App must provide atleast 1 transcode profile via the BIP_HttpStreamer_AddTranscodeProfile()" BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
                hHttpStreamer->completionStatus = BIP_ERR_INVALID_PARAMETER;
            }

            if ( hHttpStreamer->completionStatus == BIP_SUCCESS && hHttpStreamer->state == BIP_HttpStreamerState_eSetupComplete && hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eUninitialized )
            {
                /* We transition to Waiting for ProcessRequestApi state as we would want to get the 1st Request to process. */
                hHttpStreamer->state = BIP_HttpStreamerState_eWaitingForProcessRequestApi;

                /* For HLS sub-state, we will be waiting for the master playlist request. */
                hHttpStreamer->hls.state = BIP_HttpStreamerHlsState_eWaitingForMasterPlaylistReq;
            }
            BIP_Arb_CompleteRequest( hHttpStreamer->startApi.hArb, hHttpStreamer->completionStatus);
        }
        else if ( BIP_Arb_IsNew(hHttpStreamer->processRequestApi.hArb) == true )
        {
            bool requestProcessingComplete = false;     /* If set, this request processing is completed in this flow and thus requestProcessedCB can be issued. */

            BIP_Arb_AcceptRequest(hHttpStreamer->processRequestApi.hArb);

            /* Make sure ProcessRequest is not received in invalid HLS Streamer states. */
            BIP_CHECK_GOTO(( hHttpStreamer->state != BIP_HttpStreamerState_eWaitingForStopApi ), ( "BIP_HttpStreamer_ProcessRequest() is not allowed in WaitingForStopApi state(streamer is already stopped!) " ), errorInProcessRequest, BIP_ERR_INVALID_PARAMETER, hHttpStreamer->completionStatus );

            /*
             * We need to process a request. It can be for Master Playlist, Media Playlist, or Media Segment.
             * We process it based on this request type and determine how a given request type should
             * be handled in the current HLS Streamer state.
             */

            /* Retrieve the Request Target, sendPayload, & Socket related information from the Request. */
            hHttpStreamer->completionStatus = getHttpSocketAndRequestInfo( hHttpStreamer->processRequestApi.hHttpSocket, hHttpStreamer->processRequestApi.pSettings->hHttpRequest, &pRequestTarget, &sendPayload, &socketFd );
            BIP_CHECK_GOTO(( hHttpStreamer->completionStatus == BIP_SUCCESS ), ( "getHttpSocketAndRequestInfo Failed!" ), errorInProcessRequest, hHttpStreamer->completionStatus, hHttpStreamer->completionStatus );

            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p hlsState=%s: ProcessRequest: requestTarget=%s"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_HLS_STATE( hHttpStreamer->hls.state ), pRequestTarget ));
            if ( isRequestForMasterPlaylist( hHttpStreamer, pRequestTarget ) == true )
            {
                BDBG_ASSERT( hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingForMasterPlaylistReq );

                /* If we are sending payload, only then Prepare the Streamer for streaming the media stream. */
                /* This way we make sure that we have successfully acquired all needed resources before we send the Response. */
                /* We do this early on in the setup so as to hide the transcode startup latency during the time we are still sending Master & Media Playlists. */
                if ( sendPayload )
                {
                    hHttpStreamer->completionStatus = prepareStreamerForTranscode( hHttpStreamer, NULL /*initialTransocdeProfile*/, 0 );
                    BIP_CHECK_GOTO(( hHttpStreamer->completionStatus == BIP_SUCCESS ), ( "prepareStreamerForTranscode Failed!" ), errorInProcessRequest, hHttpStreamer->completionStatus, hHttpStreamer->completionStatus );
                }

                /* Now prepare the Master Playlist in memory so that we know the messageLength to be included in the HTTP Response. */
                hHttpStreamer->completionStatus = prepareAndSendMasterPlaylist( hHttpStreamer, sendPayload, &successfullResponseSent );
                BIP_CHECK_GOTO(( hHttpStreamer->completionStatus == BIP_SUCCESS ), ( "prepareAndSendMasterPlaylist Failed!" ), errorInProcessRequest, hHttpStreamer->completionStatus, hHttpStreamer->completionStatus );

                /* Successfully sent the response & payload, so we are done w/ the request. */
                requestProcessingComplete = true;
                requestForMediaSegment = false;
            }
            else if ( isRequestForMediaPlaylist( pRequestTarget ) )
            {
                /* We have a new request for Media Segment. We should be simply able to respond to it irrespective of what is happening w/ the media segment streaming. */
                hHttpStreamer->completionStatus = prepareAndSendMediaPlaylist( hHttpStreamer, pRequestTarget, sendPayload, &successfullResponseSent );
                BIP_CHECK_GOTO(( hHttpStreamer->completionStatus == BIP_SUCCESS ), ( "prepareAndSendMediaPlaylist Failed!" ), errorInProcessRequest, hHttpStreamer->completionStatus, hHttpStreamer->completionStatus );

                /* Successfully sent the response & payload (if asked) for Media Playlist, so we are done w/ the request. */
                requestProcessingComplete = true;
                requestForMediaSegment = false;
            }
            else if ( isRequestForMediaSegment( pRequestTarget ) == true )
            {
                BIP_HttpStreamerRequestInfo currentRequest;

                /* We have a new request for Media Segment. We handle it based on the current HLS state. */
                currentRequest.settings = *hHttpStreamer->processRequestApi.pSettings;
                currentRequest.hHttpSocket = hHttpStreamer->processRequestApi.hHttpSocket;
                currentRequest.requestProcessedCallback = *hHttpStreamer->processRequestApi.pRequestProcessedCallback;
                currentRequest.valid = true;
                hHttpStreamer->completionStatus = processRequestForMediaSegment( hHttpStreamer, pRequestTarget, sendPayload, socketFd, &currentRequest, &successfullResponseSent );
                BIP_CHECK_GOTO(( hHttpStreamer->completionStatus == BIP_SUCCESS ), ( "processRequestForMediaSegment Failed!" ), errorInProcessRequest, hHttpStreamer->completionStatus, hHttpStreamer->completionStatus );
                requestForMediaSegment = true;
            }
            else
            {
                /* Request is invalid: neither its for a MediaPlaylist nor for a MediaSegment. */
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer=%p hlsState=%s: Invalid Request: requestTarget=%s"
                            BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_HLS_STATE( hHttpStreamer->hls.state ), pRequestTarget ));
                hHttpStreamer->completionStatus = BIP_ERR_INVALID_PARAMETER;
                goto errorInProcessRequest;
            }

            /* We have sucessfully sent the Master Playlist, update state according to Request. */
            BDBG_ASSERT( hHttpStreamer->completionStatus == BIP_SUCCESS );

            /* Since we successfully processed the request, we may transition to new state from the current state is applicable.  */
            if ( hHttpStreamer->completionStatus == BIP_SUCCESS && hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingForMasterPlaylistReq )
            {
                /*
                 * Successfully send the Master Playlist Response & Payload (if asked by the client using the GET & not HEAD request).
                 * We transition to next state if we had sent the payload & not for the HEAD request.
                 */
                if ( sendPayload )
                {
                    /* We have successfully send the Master Playlist and now need to wait for MediaPlaylist Request. */
                    hHttpStreamer->hls.state = BIP_HttpStreamerHlsState_eWaitingForMediaPlaylistReq;
                    hHttpStreamer->state = BIP_HttpStreamerState_eWaitingForProcessRequestApi;

                    BIP_MSG_SUM(( BIP_MSG_PRE_FMT "MasterPlaylistSent: " BIP_HTTP_STREAMER_PRINTF_FMT BIP_MSG_PRE_ARG, BIP_HTTP_STREAMER_PRINTF_ARG(hHttpStreamer)));
                }
                else
                {
                    /* We didn't need to send the payload, so it must be a HEAD Request, we are done! */
                    /* Tell app that this request is done (not streaming is involved!). */
                    initiateEndOfStreamingCallback( hHttpStreamer ); /* function changes the state to eWaitingForStopApi */
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p, state=%s hlsState=%s: No Payload to send for HEAD request, done w/ this request."
                                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_HTTP_STREAMER_HLS_STATE(hHttpStreamer->hls.state) ));
                }
            }
            else if ( hHttpStreamer->completionStatus == BIP_SUCCESS && hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingForMediaPlaylistReq )
            {
                /*
                 * Successfully sent the Media Playlist Response & Payload (if asked by the client using the GET & not HEAD request).
                 * We transition to next state if we had sent the payload & not for the HEAD request.
                 */

                /* Being an initial state, we shouldn't be sending any media segment before sending the Media Playlist. */
                BDBG_ASSERT ( requestForMediaSegment == false );

                if ( sendPayload )
                {
                    /* We have successfully send the Media Playlist and now need to wait for 1stMediaSegment. */
                    hHttpStreamer->hls.state = BIP_HttpStreamerHlsState_eWaitingFor1stMediaSegmentReq;
                    hHttpStreamer->state = BIP_HttpStreamerState_eWaitingForProcessRequestApi;
                    BIP_MSG_SUM(( BIP_MSG_PRE_FMT "MediaPlaylistSent: " BIP_HTTP_STREAMER_PRINTF_FMT BIP_MSG_PRE_ARG, BIP_HTTP_STREAMER_PRINTF_ARG(hHttpStreamer)));
                }
                else
                {
                    /* We had sent the HTTP Response but didn't need to send the payload as it must be a HEAD request. */
                    /* Client will send the GET Request for MediaPlaylist again. So we stay in the same state. */
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p, state=%s hlsState=%s: No Payload to send for HEAD request, Waiting for Get Request!"
                                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_HTTP_STREAMER_HLS_STATE(hHttpStreamer->hls.state) ));
                }
            }
            else if ( hHttpStreamer->completionStatus == BIP_SUCCESS &&
                    (hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingFor1stMediaSegmentReq ||
                     hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingForNextMediaSegmentReq ||
                     hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eStreamingDone ))
            {
                /*
                 * In these states, we have successfully sent either Media Playlist Response & Payload (if asked by the client using the GET & not HEAD request).
                 * Or, we may have initiated sending the media segment for the segment request.
                 * We transition to next state if we have sent the media segment payload & not for the HEAD or playlist request.
                 */
                if ( sendPayload && requestForMediaSegment )
                {
                    /* We have successfully sent the Media Segment Payload and now are in the streaming state. */
                    hHttpStreamer->hls.state = BIP_HttpStreamerHlsState_eStreaming;
                    hHttpStreamer->state = BIP_HttpStreamerState_eStreaming;
                    BIP_MSG_SUM(( BIP_MSG_PRE_FMT "MediaSegment Streaming Started: " BIP_HTTP_STREAMER_PRINTF_FMT BIP_MSG_PRE_ARG, BIP_HTTP_STREAMER_PRINTF_ARG(hHttpStreamer)));
                }
                else
                {
                    /* Either we didn't need to send the payload (HEAD request for Media Segment or Playlist) or it was GET Request for Media Playlist. */
                    /* Client will send the GET Request for MediaSegment again. So we stay in the current state. */
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p, state=%s hlsState=%s: %s"
                                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_HTTP_STREAMER_HLS_STATE(hHttpStreamer->hls.state),
                                !sendPayload && !requestForMediaSegment ? "No Payload to send for MediaPlaylist HEAD Request":
                                !requestForMediaSegment ? "Sent MediaPlaylist Payload" : "No Payload to send for MediaSegment HEAD Request"
                             ));
                }
            }
            else if ( hHttpStreamer->completionStatus == BIP_SUCCESS &&
                     (hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eStreaming || hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingForEndOfSegmentCallback) )
            {
                /*
                 * We either successfully sent the Media Playlist Response & Payload (if asked by the client using the GET & not HEAD request).
                 * Or, we would have deferred the Media Segment request as we were still streaming the previous segment and haven't yet received the PBIP callback.
                 */
                if ( requestForMediaSegment )
                {
                    /* Request was for MediaSegment, so we change state to reflect waiting for PBIP callback. */
                    hHttpStreamer->hls.state = BIP_HttpStreamerHlsState_eWaitingForEndOfSegmentCallback;
                    hHttpStreamer->state = BIP_HttpStreamerState_eStreaming;
                    BIP_MSG_SUM(( BIP_MSG_PRE_FMT "Waiting for PBIP EndOfStreaming Callback: " BIP_HTTP_STREAMER_PRINTF_FMT BIP_MSG_PRE_ARG, BIP_HTTP_STREAMER_PRINTF_ARG(hHttpStreamer)));
                }
                else
                {
                    /* It was a request for Media Playlist, we have successfully sent it. Nothing to do here. */
                    /* Client will send the GET Request for MediaSegment again. So we stay in the current state. */

                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p, state=%s hlsState=%s: Sent Response for Playlist"
                                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_HTTP_STREAMER_HLS_STATE(hHttpStreamer->hls.state)));
                }
            }
            else
            {
                BDBG_ASSERT(NULL);
            }

            /* Finished w/ the state transition, check if we completely processed the request. */
            if ( requestProcessingComplete == true )
            {
                /* Add a deferred callback to let HttpServerSocket know that we are done processing the Request & HttpSocket. */
                if ( hHttpStreamer->processRequestApi.pRequestProcessedCallback->callback )
                {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p: Added requestProcessedCallback" BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
                    BIP_Arb_AddDeferredCallback( hHttpStreamer->processRequestApi.hArb, hHttpStreamer->processRequestApi.pRequestProcessedCallback );
                    hHttpStreamer->processRequest.callbackIssued = true;
                }
            }
            /* Reset the Response Object state since we are done with this Request. */
            resetHttpStreamerResponseState( hHttpStreamer );

            if ( hHttpStreamer->completionStatus != BIP_SUCCESS )
            {
errorInProcessRequest:
                if ( hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingForMasterPlaylistReq )
                {
                    /* Failure can happen either in BIP_Streamer_Prepare, preparing Response, Sending Response, or Sending Media Playlist. */
                    /* Since this state is run part of the 1st Request in the App's context, failure status here will be conveyed to the caller (app) upon return. */

                    /* Stop streamer to release its resources. */
                    BIP_Streamer_Stop( hHttpStreamer->hStreamer );

                    resetHttpStreamerResponseState( hHttpStreamer );

                    if ( successfullResponseSent == true )
                    {
                        /* We ran into an error after we have already sent a succesful response, so can't fail this ProcessRequest. */
                        /* Instead, we return a success here and issue an endOfStreaming callback to the app. This way app can simply Stop the streamer. */
                        BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: BIP_HttpStreamer_ProcessRequest Failed to send Master Playlist, completionStatus=%s, queued up endOfStreamingCallback"
                                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_StatusGetText(hHttpStreamer->completionStatus) ));
                        hHttpStreamer->completionStatus = BIP_SUCCESS;
                        initiateEndOfStreamingCallback( hHttpStreamer ); /* function changes the state to eWaitingForStopApi */
                    }
                    else
                    {
                        /* We haven't yet send the response, so we will return the completion status to the app.  */
                        /* We also go back to the idle state incase app wants to reuse this streamer. */
                        hHttpStreamer->state = BIP_HttpStreamerState_eIdle;
                        hHttpStreamer->hls.state = BIP_HttpStreamerHlsState_eUninitialized;
                        BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: BIP_HttpStreamer_ProcessRequest Failed to send Master Playlist, completionStatus=%s"
                                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_StatusGetText(hHttpStreamer->completionStatus) ));
                    }
                }
                else
                {
                    /*
                     * This is a catch-all for any failures to process a request in rest of the states.
                     * We treat it like a hard-error and notify end of streaming to the app.
                     */

                    /* If we ran into an error before sending the response, then we send an explicit error response. */
                    if ( successfullResponseSent == false )
                    {
                        sendErrorResponse( hHttpStreamer, hHttpStreamer->processRequestApi.hHttpSocket, hHttpStreamer->processRequestApi.pSettings->hHttpRequest ); /* Note we ignore the status of sendErrorResponse as we want to return the previous error. */
                    }

                    if ( hHttpStreamer->completionStatus == BIP_ERR_OS_ERRNO && BIP_StatusToErrno( hHttpStreamer->completionStatus ) == EPIPE )
                    {
                        BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: Failed to send HTTP Response for Media Segment Request due to client aborting connection (may be seek), remain in same state!"
                                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
                    }
                    else
                    {
                        initiateEndOfStreamingCallback( hHttpStreamer ); /* function changes the state to eWaitingForStopApi */

                        BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer=%p hlsState=%s: Got an error=%s while ProcessingRequest URL=%s, Waiting for App to Stop HttpStreamer!"
                                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_HLS_STATE(hHttpStreamer->hls.state), BIP_StatusGetText( hHttpStreamer->completionStatus ), pRequestTarget ));
                    }
                }
            } /* Error handling case. */

            /* And finally, we complete the ARB w/ either success or failure. */
            BIP_Arb_CompleteRequest( hHttpStreamer->processRequestApi.hArb, hHttpStreamer->completionStatus);
        } /* ProcessRequest State processsing. */
        else if (hHttpStreamer->state == BIP_HttpStreamerState_eStreamingDone && hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eStreaming )
        {
            /* We are in StreamingDone state. This is how we got there: */
            /* Note: [Streaming] --PBIP CB--> [StreamingDone] --BIP ARB Timer--> here... */

            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p hlsState=%s: Finished Streaming Segment[%u]: requestTarget=%s"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_HLS_STATE( hHttpStreamer->hls.state ), hHttpStreamer->hls.currentSegmentSequenceNumber, pRequestTarget ));
            /* Add a deferred callback to let HttpServerSocket know that we are done processing the Request. */
            if ( hHttpStreamer->processRequest.requestProcessedCallback.callback )
            {
                BIP_Arb_AddDeferredCallback( hHttpStreamer->processRequestApi.hArb, &hHttpStreamer->processRequest.requestProcessedCallback );
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p: Added requestProcessedCallback for hHttpSocket=%p" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, (void *)hHttpStreamer->processRequest.hHttpSocket ));
                hHttpStreamer->processRequest.callbackIssued = true;
            }

            resetHttpStreamerResponseState( hHttpStreamer );

            hHttpStreamer->hls.state = BIP_HttpStreamerHlsState_eWaitingForNextMediaSegmentReq;
            hHttpStreamer->state = BIP_HttpStreamerState_eWaitingForProcessRequestApi;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p state=%s hlsState=%s: Waiting for next segment request or timeout. "
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_HTTP_STREAMER_HLS_STATE( hHttpStreamer->hls.state) ));
        } /* StreamingDone state processing when hlsState is still streaming. */
        else if ( hHttpStreamer->state == BIP_HttpStreamerState_eStreamingDone && hHttpStreamer->hls.state == BIP_HttpStreamerHlsState_eWaitingForEndOfSegmentCallback )
        {
            /*
             * We were waiting for PBIP end of segment callback and now we got it.
             * This is how we got there:
             * Note: [Streaming] --Request--> [WaitingForEndOfSeg] --PBIP CB--> [StreamingDone] --BIP ARB Timer--> here...
             *
             * Also, we must have a pending request waiting for this PBIP callback before it can be processed.
             * Now we will process this media segment request.
             */

            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p hlsState=%s: Finished Streaming Segment[%u]: requestTarget=%s"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_HLS_STATE( hHttpStreamer->hls.state ), hHttpStreamer->hls.currentSegmentSequenceNumber, pRequestTarget ));

            /* First, we complete the request for which we finished streaming the current segment. */
            if ( hHttpStreamer->processRequest.requestProcessedCallback.callback )
            {
                BIP_Arb_AddDeferredCallback( hHttpStreamer->processRequestApi.hArb, &hHttpStreamer->processRequest.requestProcessedCallback );
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p: Added requestProcessedCallback for hHttpSocket=%p" BIP_MSG_PRE_ARG, (void *)hHttpStreamer, (void *)hHttpStreamer->processRequest.hHttpSocket ));
                hHttpStreamer->processRequest.callbackIssued = true;
            }

            /* And we change our state to reflect as if we were waiting for next segment. */
            hHttpStreamer->hls.state = BIP_HttpStreamerHlsState_eWaitingForNextMediaSegmentReq;
            resetHttpStreamerResponseState( hHttpStreamer );

            /* Then, we process the pending request from the client. */
            BDBG_ASSERT( hHttpStreamer->pendingRequest.valid );
            {
                int socketFd;
                BIP_HttpStreamerRequestInfo currentRequest;

                currentRequest = hHttpStreamer->pendingRequest;
                /* Retrieve the Request Target, sendPayload, & Socket related information from the Request. */
                hHttpStreamer->completionStatus = getHttpSocketAndRequestInfo( currentRequest.hHttpSocket, currentRequest.settings.hHttpRequest, &pRequestTarget, &sendPayload, &socketFd );
                BIP_CHECK_GOTO(( hHttpStreamer->completionStatus == BIP_SUCCESS ), ( "getHttpSocketAndRequestInfo Failed!" ), errorInWaitingForEndOfSegment, hHttpStreamer->completionStatus, hHttpStreamer->completionStatus );

                hHttpStreamer->completionStatus = processRequestForMediaSegment( hHttpStreamer, pRequestTarget, sendPayload, socketFd, &currentRequest, &successfullResponseSent );
                BIP_CHECK_GOTO(( hHttpStreamer->completionStatus == BIP_SUCCESS ), ( "processRequestForMediaSegment Failed!" ), errorInWaitingForEndOfSegment, hHttpStreamer->completionStatus, hHttpStreamer->completionStatus );
                hHttpStreamer->pendingRequest.valid = false; /* request has now been processed. */

                if ( hHttpStreamer->completionStatus == BIP_SUCCESS )
                {
                    hHttpStreamer->hls.state = BIP_HttpStreamerHlsState_eStreaming;
                    hHttpStreamer->state = BIP_HttpStreamerState_eStreaming;
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p, state=%s, hlsState=%s: Resumed Streaming using the pending Request"
                                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_HTTP_STREAMER_HLS_STATE( hHttpStreamer->hls.state) ));
                }
                else
                {
errorInWaitingForEndOfSegment:
                    /* If we ran into an error before sending the response, then we send an explicit error response. */
                    if ( successfullResponseSent == false )
                    {
                        sendErrorResponse( hHttpStreamer, hHttpStreamer->processRequest.hHttpSocket, hHttpStreamer->processRequest.settings.hHttpRequest ); /* Note we ignore the status of sendErrorResponse as we want to return the previous error. */
                    }

                    if ( hHttpStreamer->completionStatus == BIP_ERR_OS_ERRNO && BIP_StatusToErrno( hHttpStreamer->completionStatus ) == EPIPE )
                    {
                        BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: Failed to send HTTP Response for Media Segment Request due to client aborting connection (may be seek), remain in same state!"
                                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
                    }
                    else
                    {
                        /* Some other non-client related error and we failed to resume streaming, notify app. */
                        initiateEndOfStreamingCallback( hHttpStreamer ); /* function changes the state to eWaitingForStopApi */

                        /* Also, notify caller about being done w/ the request. */
                        if ( hHttpStreamer->processRequest.requestProcessedCallback.callback )
                        {
                            BIP_Arb_AddDeferredCallback( hHttpStreamer->processRequestApi.hArb, &hHttpStreamer->processRequest.requestProcessedCallback );
                            hHttpStreamer->processRequest.callbackIssued = true;
                        }
                        BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: Error during resuming to stream using the pending Request, so moving to WaitingForStop state!"
                                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
                    }
                }
            }
        } /* StreamingDone state processing when hlsState is WaitingForEndOfSegment CB. */
        /* State processing for _Stop & _Destroy() APIs is almost the same. */
        else if ( BIP_Arb_IsBusy(hHttpStreamer->stopApi.hArb) == true || BIP_Arb_IsBusy(hHttpStreamer->destroyApi.hArb) == true )
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p hlsState=%s: Starting %s Sequence"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_HLS_STATE( hHttpStreamer->hls.state ), BIP_Arb_IsBusy( hHttpStreamer->stopApi.hArb)?"Stop":"Destroy" ));
            if ( hHttpStreamer->state == BIP_HttpStreamerState_eIdle )
            {
                /* has to be for _Destroy(), just reset the state incase, we had failed to start & app is just destroying us. */
                resetHttpStreamerResponseState( hHttpStreamer );
                hHttpStreamer->completionStatus = BIP_SUCCESS;
            }
            else
            {
                /* For all other states, we will stop the Streamer. */

                stopAndDestroyStreamer( hHttpStreamer );

                /* Since we have stopped the Streamer, we are back to the idle state */
                hHttpStreamer->state = BIP_HttpStreamerState_eIdle;
                hHttpStreamer->hls.state = BIP_HttpStreamerHlsState_eUninitialized;

                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: BIP_HttpStreamer Stopped!"
                            BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));

                hHttpStreamer->completionStatus = BIP_SUCCESS;
            }
            if ( BIP_Arb_IsBusy(hHttpStreamer->stopApi.hArb) == true )
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: BIP_HttpStreamer_Stop is done!"
                            BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
                BIP_Arb_CompleteRequest( hHttpStreamer->stopApi.hArb, hHttpStreamer->completionStatus);
            }
            else if ( BIP_Arb_IsBusy(hHttpStreamer->destroyApi.hArb) == true )
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: BIP_HttpStreamer Destorying is almost done!"
                            BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
                BIP_Arb_CompleteRequest( hHttpStreamer->destroyApi.hArb, hHttpStreamer->completionStatus);
            }
        } /* stop or destroy ARB handling. */

        BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--- hHttpStreamer %p: state=%s hlsState=%s"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_HTTP_STREAMER_HLS_STATE(hHttpStreamer->hls.state) ));
    }
} /* processHttpHlsStreamerState */
#else /* NEXUS_HAS_VIDEO_ENCODER */
void BIP_HttpStreamer_GetStreamerIdFromUrl(
    const char *pUrl,
    unsigned *phHttpStreamer
    )
{
    BSTD_UNUSED( pUrl );
    *phHttpStreamer = 0;
    return;
} /* BIP_HttpStreamer_GetStreamerIdFromUrl */

#endif /* NEXUS_HAS_VIDEO_ENCODER */

/* Function to handle direct HTTP (i.e. no adaptive) streaming specific logic. */
static void processHttpDirectStreamerState(
    BIP_HttpStreamerHandle hHttpStreamer,
    int value,
    BIP_Arb_ThreadOrigin threadOrigin
    )
{
    bool reRunProcessState;

    BSTD_UNUSED(value);
    BSTD_UNUSED(threadOrigin);

    BDBG_ASSERT(hHttpStreamer);
    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer);

    if (hHttpStreamer->state == BIP_HttpStreamerState_eIdle &&  BIP_Arb_IsBusy(hHttpStreamer->destroyApi.hArb) != true )
    {
        /* No specific work to do if we are still in Idle state & its not a _Destroy(). */
        return;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTRY ---> hHttpStreamer %p: state %s"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));

    reRunProcessState = true;
    while (reRunProcessState)
    {
        reRunProcessState = false;
        if ( BIP_Arb_IsBusy(hHttpStreamer->startApi.hArb) == true )
        {
            /* Main processState function has already ensured that app can call _Start() in only Idle state. */
            /* Also, main thread had changed the state to BIP_HttpStreamerState_eSetupComplete to indicate us */
            /* that all input & output related settings have been setup. */
            if (hHttpStreamer->state == BIP_HttpStreamerState_eSetupComplete)
            {
                /* For now, not much do in the _Start(), so wait for caller to give us request to preocess. */
                hHttpStreamer->state = BIP_HttpStreamerState_eWaitingForProcessRequestApi;
            }
            else
            {
                /* shouldn't happen as SetupComplete state is set during _Start(). */
                BDBG_ASSERT(NULL);
            }
            hHttpStreamer->completionStatus = BIP_SUCCESS;
            BIP_Arb_CompleteRequest( hHttpStreamer->startApi.hArb, hHttpStreamer->completionStatus);
        }
        else if ( BIP_Arb_IsNew(hHttpStreamer->processRequestApi.hArb) == true )
        {
            BIP_StreamerTranscodeProfileListEntry *pTranscodeProfileEntry = NULL;

            BIP_Arb_AcceptRequest(hHttpStreamer->processRequestApi.hArb);
            /* Copy the process request API settings. */
            hHttpStreamer->processRequest.requestProcessedCallback = *hHttpStreamer->processRequestApi.pRequestProcessedCallback;
            hHttpStreamer->processRequest.hHttpSocket = hHttpStreamer->processRequestApi.hHttpSocket;
            hHttpStreamer->processRequest.settings = *hHttpStreamer->processRequestApi.pSettings;

            /* If EncoderProfile is set by app, then we use the 1st profile for Direct Streaming. */
            if ( hHttpStreamer->pStreamer->transcode.profileState == BIP_StreamerOutputState_eSet )
            {
                pTranscodeProfileEntry = BLST_Q_FIRST( &hHttpStreamer->pStreamer->transcode.profileListHead );
            }

            /* For Direct Streaming, _ProcessRequest() API can be called only in the WaitingForProcessRequestApi state. */
            if (hHttpStreamer->state != BIP_HttpStreamerState_eWaitingForProcessRequestApi)
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_ProcessRequest() is only allowed in WaitingForProcessRequestApi state, current state: %s"
                            BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
                hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
                BIP_Arb_RejectRequest( hHttpStreamer->processRequestApi.hArb, hHttpStreamer->completionStatus );
            }
            else if (hHttpStreamer->state == BIP_HttpStreamerState_eWaitingForProcessRequestApi)
            {
                /*
                 * Caller provides two parameters in the _ProcessRequest():
                 * -optional HttpRequest -> if given, we are supposed to send the Response for this Request.
                 * -required HttpSocket  -> we will send HttpResonse (if needed) & then stream media using
                 *  this object.
                 */

                hHttpStreamer->completionStatus = BIP_SUCCESS;

                if ( hHttpStreamer->processRequest.settings.hHttpRequest )
                {
                    BIP_HttpRequestMethod method;
                    const char *pMethodName = NULL;
                    /* Caller has provided HttpRequest object, check if it Method is HEAD or not. */
                    /* We dont need to send payload for HEAD method. */

                    hHttpStreamer->completionStatus = BIP_HttpRequest_GetMethod( hHttpStreamer->processRequest.settings.hHttpRequest, &method, &pMethodName);
                    if ( hHttpStreamer->completionStatus == BIP_SUCCESS )
                    {
                        hHttpStreamer->sendPayload = (method == BIP_HttpRequestMethod_eHead) ? false : true;
                    }
                }
                else
                {
                    /* We didn't have HttpRequest object, so caller(app) must have already sent the response. */
                    /* Since app is calling us, it must want us to stream. TODO: can further validate content length for file inputs. */
                    hHttpStreamer->sendPayload = true;
                }
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Calling StreamerPrepare(): current state: %s sendPayload=%d"
                            BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), hHttpStreamer->sendPayload));

                /* First, open any Nexus resources required for streaming (such as Playback, Recpump, etc.). */
                /* This way we make sure that we have acquired all needed resources before we send the Response (if asked to do so). */
                if ( hHttpStreamer->completionStatus == BIP_SUCCESS && hHttpStreamer->sendPayload )
                {
                    /* If we are sending payload, only then prepare the streamer for streaming, otherwise, it doesn't really help to prepare streaming resources at this time. */
                    /* Streamer will acquire all resources it needs for streaming and get them ready. */
                    BIP_StreamerPrepareSettings prepareSettings;

                    BIP_Streamer_GetDefaultPrepareSettings( &prepareSettings );

                    /*  This always have to set irrespective of whether we are running in RaveInterruptbased or systemTimer based mode,
                        since Rave interrupt internally is always enable only we don't wait for that event in systemTimer mode.
                        Now in system timer mode since we are running based on systemTimer,
                        so we can set dataReadyThreshold high based on the systemTimer duration, which eventually reduce the number of interrupt.*/
#if 0
                    prepareSettings.recpumpOpenSettings.data.dataReadyThreshold =
                            prepareSettings.recpumpOpenSettings.data.atomSize * hHttpStreamer->startSettings.streamingSettings.raveInterruptBasedSettings.dataReadyScaleFactor;
#endif
                    prepareSettings.recpumpOpenSettings.data.bufferSize = prepareSettings.recpumpOpenSettings.data.bufferSize;

                    hHttpStreamer->completionStatus = BIP_Streamer_Prepare( hHttpStreamer->hStreamer, &prepareSettings );
                }

                /* If successful in opening Nexus Resources & HttpRequest object is provided, then prepare and send HTTP Response . */
                /* Note: HttpRequest object will be provided by HttpServerSocket when App is using the BIP_HttpServer class interface. */
                /* Otherwise, App may choose to send HTTP Request & Response on its own and use BIP_HttpStreamer class for just streaming. */
                if (hHttpStreamer->completionStatus == BIP_SUCCESS && hHttpStreamer->processRequest.settings.hHttpRequest)
                {
                    /* Check if we have already prepared the Response (if app had called the _SetResponseHeader()), otherwise prepare it. */
                    if (hHttpStreamer->response.state == BIP_HttpStreamerResponseHeadersState_eNotSet)
                    {
                        hHttpStreamer->completionStatus = prepareCommonResponseHeaders(hHttpStreamer );
                        if (hHttpStreamer->completionStatus == BIP_SUCCESS)
                        {
                            /* We have successfully prepared the default Http Response, update state. */
                            hHttpStreamer->response.state = BIP_HttpStreamerResponseHeadersState_eSet;
                            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Response headers prepared!" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));
                        }
                        /* Note: error cases are handled in one go below. */
                    }

                    /* Response headers are/were prepared, so send the response. */
                    if (hHttpStreamer->completionStatus == BIP_SUCCESS)
                    {
                        hHttpStreamer->completionStatus = finalizeHeadersAndSendResponse( hHttpStreamer, &pTranscodeProfileEntry->transcodeProfile, &hHttpStreamer->successfullResponseSent );
                    }
                }

                if ( hHttpStreamer->completionStatus == BIP_SUCCESS )
                {
                    /* Get raw socketFd using the HttpSocket object's status that PBIP would use for streaming. */
                    hHttpStreamer->completionStatus = BIP_HttpSocket_GetStatus( hHttpStreamer->processRequest.hHttpSocket, &hHttpStreamer->httpSocketStatus );
                }

                /* We have successfully prepared the streamer. */
                /* Also, we have successfully prepared & sent the HTTP Response (if asked). */
                /* So check if DTCP/IP AKE is completed by the client (if enabled). */
                if ( hHttpStreamer->completionStatus == BIP_SUCCESS && hHttpStreamer->sendPayload && hHttpStreamer->output.settings.enableDtcpIp)
                {
                    /* Change state to WaitingForClientAke & rerun the state machine. */
                    /* It will check if client has completed AKE with the server. */
                    hHttpStreamer->state = BIP_HttpStreamerState_eWaitingForClientAke;
                    reRunProcessState = true;
                }
                if (hHttpStreamer->state == BIP_HttpStreamerState_eWaitingForProcessRequestApi)
                {
                    /* We are still in the current state & haven't changed to WaitingForClientAke state, */
                    /* so lets move on to the StartStreaming state. */
                    hHttpStreamer->state = BIP_HttpStreamerState_eStartStreaming;
                    reRunProcessState = true;
                }
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: initial part of ProcessRequest is completed, completionStatus=%d!"
                            BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), hHttpStreamer->completionStatus));
            } /* _eWaitingForProcessRequestApi state processsing. */
            else
            {
                /* _ProcessRequest() should only come in the _eWaitingForProcessRequestApi state. */
                BDBG_ASSERT(NULL);
            }
            /* Go back to top to re-run the state machine if we determined to do so! */
            if ( reRunProcessState == true ) continue;

        } /* processRequestApi */
        /* State processing for _Stop & _Destroy() APIs is almost the same. */
        else if ( BIP_Arb_IsBusy(hHttpStreamer->stopApi.hArb) == true || BIP_Arb_IsBusy(hHttpStreamer->destroyApi.hArb) == true )
        {
            if ( hHttpStreamer->state == BIP_HttpStreamerState_eIdle)
            {
                /* has to be for _Destroy(), just reset the state incase, we had failed to start & app is just destroying us. */
                resetHttpStreamerResponseState( hHttpStreamer );
                hHttpStreamer->completionStatus = BIP_SUCCESS;
            }
            else if (hHttpStreamer->state == BIP_HttpStreamerState_eWaitingForProcessRequestApi)
            {
                /* Caller didn't issue _ProcessRequest & instead called _Stop(), just reset the steamer state. */
                resetHttpStreamerResponseState( hHttpStreamer );
                hHttpStreamer->state = BIP_HttpStreamerState_eIdle;
                hHttpStreamer->completionStatus = BIP_SUCCESS;
            }
            else if ( hHttpStreamer->state == BIP_HttpStreamerState_eStreaming || /* App is Stopping us on its own (w/o our Callback) */
                    /* PBIP playbackIpStreamerCallback() came (state is eStreamingDone) but app Stop() ran the state m/c before ArbTimer could on PBIP behalf. */
                    hHttpStreamer->state == BIP_HttpStreamerState_eStreamingDone ||
                    /* We are waiting for Client to finish AKE, but app called Stop before that. */
                    hHttpStreamer->state == BIP_HttpStreamerState_eWaitingForClientAke ||
                    /* Normal case where PBIP callback came, ArbTimer ran our state, we notified app about endOfStreaming, and it issued _Stop(). */
                    hHttpStreamer->state == BIP_HttpStreamerState_eWaitingForStopApi
                    )
            {
                /* Stop the Streamer. */

                stopAndDestroyStreamer( hHttpStreamer );

                if ( BIP_Arb_IsBusy(hHttpStreamer->processRequestApi.hArb) == true )
                {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: Completed ProcessRequest ARB as we were stopped while waiting for AKE!"
                                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
                    BIP_Arb_CompleteRequest( hHttpStreamer->processRequestApi.hArb, BIP_ERR_ASYNC_API_ABORT );
                }

                /* Since we have stopped the Streamer, we are back to the idle state */
                hHttpStreamer->state = BIP_HttpStreamerState_eIdle;

                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: BIP_HttpStreamer Stopped!"
                            BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));

                hHttpStreamer->completionStatus = BIP_SUCCESS;
            }
            else
            {
                /* not valid states. */
                BDBG_ASSERT(NULL);
            }
            if ( BIP_Arb_IsBusy(hHttpStreamer->processRequestApi.hArb) == true )
            {
                BIP_Arb_CompleteRequest( hHttpStreamer->processRequestApi.hArb, hHttpStreamer->completionStatus);
            }
            if ( BIP_Arb_IsBusy(hHttpStreamer->stopApi.hArb) == true )
            {
                BIP_Arb_CompleteRequest( hHttpStreamer->stopApi.hArb, hHttpStreamer->completionStatus);
            }
            else if ( BIP_Arb_IsBusy(hHttpStreamer->destroyApi.hArb) == true )
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: BIP_HttpStreamer Destorying is almost done!"
                            BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
                BIP_Arb_CompleteRequest( hHttpStreamer->destroyApi.hArb, hHttpStreamer->completionStatus);
            }
        }
        else if (hHttpStreamer->state == BIP_HttpStreamerState_eStreamingDone)
        {
            /* Note: We were in the Streaming state and PBIP issued endOfStreaming Callback. In that callback, */
            /* we change state to StreamingDone to indicate that we are done w/ streaming. */
            /* The callback then queues the deferred callback to run this state machine. */
            /* ArbTimer then runs the streamer state machine. */
            /* Since _Stop() hasn't yet come, so we need to let both HttpServerSocket & app know about end of streaming. */

            /* Add a deferred callback to let HttpServerSocket know that we are done processing the Request. */
            if ( hHttpStreamer->processRequest.requestProcessedCallback.callback )
            {
                BIP_Arb_AddDeferredCallback( hHttpStreamer->processRequestApi.hArb, &hHttpStreamer->processRequest.requestProcessedCallback );
                hHttpStreamer->processRequest.callbackIssued = true;
            }

            /* Add a deferred callback to let App know as well that we are done with Streaming. */
            initiateEndOfStreamingCallback( hHttpStreamer ); /* function changes the state to eWaitingForStopApi */

            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: Done from StreamingDone state and moving to WaitingForStop state!"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
        }
        else if (hHttpStreamer->state == BIP_HttpStreamerState_eWaitingForClientAke)
        {
            /* Check if client has completed AKE with the server. */
            hHttpStreamer->completionStatus = BIP_DtcpIpServer_CheckForClientAke(hHttpStreamer->startSettings.hInitDtcpIp, hHttpStreamer->httpSocketStatus.pRemoteIpAddress);
            if (hHttpStreamer->completionStatus == BIP_SUCCESS || hHttpStreamer->startSettings.timeoutInMs == 0)
            {
                /* Client has completed the AKE or we were in non-blocking mode, so move on to the streaming state. */
                hHttpStreamer->state = BIP_HttpStreamerState_eStartStreaming;
                reRunProcessState = true;
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: AKE is %s, moving to StartStreaming state!"
                            BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state),
                            hHttpStreamer->completionStatus == BIP_SUCCESS ? "Completed":"Not complete, but API timeoutInMs is 0 (non-blocking)"
                            ));
            }
            else
            {
                /* Client has NOT yet completed the AKE, so set a timer to periodically check the completion. */
                /* And remain in the WaitingForClientAke state. */
                bool reStartTimer = false;

                if ( hHttpStreamer->dtcpIpAkeTimerActive )
                {
                    B_Time endTime;

                    B_Time_Get( &endTime );
                    /* Check if dtcpIpAke poll interval has expired. */
                    if ( B_Time_Diff( &endTime, &hHttpStreamer->dtcpIpAkeTimerStartTime ) >= BIP_HTTP_STREAMER_DTCP_IP_AKE_TIMER_POLL_INTERVAL_IN_MSEC )
                    {
                        /* AKE Timer's poll interval has expired. Check if we have exceeded the max allowed timeout for the Start API. */
                        if ( hHttpStreamer->startSettings.timeoutInMs > 0 && B_Time_Diff( &endTime, &hHttpStreamer->dtcpIpAkeTimerInitialTime ) >= hHttpStreamer->startSettings.timeoutInMs )
                        {
                            /* Client didn't complete the AKE in the max allotted timeout for this API (initiated by the process request API). */
                            hHttpStreamer->completionStatus = BIP_INF_TIMEOUT;
                            /* Change to StartStreaming state as both normal & error processing happens there based on the completion status. */
                            hHttpStreamer->state = BIP_HttpStreamerState_eStartStreaming;
                            BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: dtcpIpAke timer expired, timeoutInMs=%d elapsedTimeInMs=%ld"
                                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), hHttpStreamer->startSettings.timeoutInMs,
                                        B_Time_Diff( &endTime, &hHttpStreamer->dtcpIpAkeTimerInitialTime ) ));
                            reRunProcessState = true;
                        }
                        else
                        {
                            /* AKE still haven't finished & we haven't yet exceeded the API timeout. So restart the timer to run the state machine later. */
                            reStartTimer = true;
                            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: dtcpIpAke timer has not yet expired, timeoutInMs=%d elapsedTimeInMs=%ld"
                                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), hHttpStreamer->startSettings.timeoutInMs,
                                        B_Time_Diff( &endTime, &hHttpStreamer->dtcpIpAkeTimerInitialTime ) ));
                            reRunProcessState = true;
                        }
                    } /* dtcpIpAke poll timer expired. */
                    else
                    {
                        reStartTimer = true;
                    }
                    /* else: timer hasn't yet expired, so nothing to do. */
                } /* dtcpIpAkeTimer is active. */
                else
                {
                    /* first time only, restart the time. */
                    reStartTimer = true;
                }

                /* Start/Re-start the timer. */
                if (reStartTimer)
                {
                    hHttpStreamer->completionStatus = createDtcpIpAkeTimer( hHttpStreamer );
                    if ( hHttpStreamer->completionStatus != BIP_SUCCESS )
                    {
                        hHttpStreamer->state = BIP_HttpStreamerState_eStartStreaming;
                        BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: Failed to restart dtcpIpAke timer, moving to StartStreaming state to complete error processing!"
                                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
                        reRunProcessState = true;
                    }
                    else
                    {
                        if (!hHttpStreamer->dtcpIpAkeTimeNoted)
                        {
                            B_Time_Get( &hHttpStreamer->dtcpIpAkeTimerInitialTime );
                            hHttpStreamer->dtcpIpAkeTimeNoted = true;
                        }
                        hHttpStreamer->state = BIP_HttpStreamerState_eWaitingForClientAke;
                        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: restarted dtcpIpAke timer, waiting for client AKE to finish!"
                                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
                    }
                }
            }
            if ( reRunProcessState == true ) continue;
        } /* (hHttpStreamer->state == BIP_HttpStreamerState_eWaitingForClientAke) */

        else if (hHttpStreamer->state == BIP_HttpStreamerState_eStartStreaming)
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: 2nd part of ProcessRequest: Start Streaming, completionStatus=%d!"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), hHttpStreamer->completionStatus));
            reRunProcessState = false;
            /* So lets start the streamer if we are sending payload. */
            if ( hHttpStreamer->completionStatus == BIP_SUCCESS && hHttpStreamer->sendPayload)
            {
                if (hHttpStreamer->completionStatus == BIP_SUCCESS)
                {
                    hHttpStreamer->completionStatus = BIP_Streamer_Start( hHttpStreamer->hStreamer, NULL );
                }
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: After StreamerStart: Start Streaming, completionStatus=%d!"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), hHttpStreamer->completionStatus));
                if (hHttpStreamer->completionStatus == BIP_SUCCESS)
                {
                    /* Get raw socketFd using the HttpSocket object's status that PBIP would use for streaming. */
                    hHttpStreamer->completionStatus = BIP_HttpSocket_GetStatus( hHttpStreamer->processRequest.hHttpSocket, &hHttpStreamer->httpSocketStatus );
                }
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: After GetStatus: Start Streaming, completionStatus=%d!"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), hHttpStreamer->completionStatus));

#if DTCP_TODO
                if (hHttpStreamer->completionStatus == BIP_SUCCESS && hHttpStreamer->output.settings.enableDtcpIp)
                {
                    BIP_DtcpIpServerStreamStatus dtcpIpServerStreamStatus;

                    hHttpStreamer->completionStatus = BIP_DtcpIpServer_GetStreamStatus( hHttpStreamer->startSettings.hInitDtcpIp, hHttpStreamer->httpSocketStatus.pRemoteIpAddress, &dtcpIpServerStreamStatus );
#if 0
                    liveStreamingOpenSettings.securitySettings.settings.dtcpIp.emiValue = hHttpStreamer->output.settings.dtcpIpOutput.copyControlInfo;
                    liveStreamingOpenSettings.securitySettings.settings.dtcpIp.akeTimeoutInMs = hHttpStreamer->output.settings.dtcpIpOutput.akeTimeoutInMs;
                    liveStreamingOpenSettings.securitySettings.settings.dtcpIp.pcpPayloadLengthInBytes = hHttpStreamer->output.settings.dtcpIpOutput.pcpPayloadLengthInBytes;
#endif
                }
#endif

                if (hHttpStreamer->completionStatus == BIP_SUCCESS)
                {
                    hHttpStreamer->currentStreamingFd = hHttpStreamer->httpSocketStatus.socketFd;
                    hHttpStreamer->completionStatus = startAspOrPBipStreamer( hHttpStreamer, hHttpStreamer->httpSocketStatus.socketFd );
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: startAspOrPBipStreamer() done, completionStatus=%d!"
                                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), hHttpStreamer->completionStatus));
                }
            }

            /* This should be the last step... */
            if (hHttpStreamer->completionStatus == BIP_SUCCESS)
            {
                if ( hHttpStreamer->sendPayload )
                {
                    /* We have successfully started streaming. */
                    hHttpStreamer->state = BIP_HttpStreamerState_eStreaming;
                    BIP_MSG_SUM(( BIP_MSG_PRE_FMT "StreamingStarted: " BIP_HTTP_STREAMER_PRINTF_FMT
                                BIP_MSG_PRE_ARG, BIP_HTTP_STREAMER_PRINTF_ARG(hHttpStreamer)));
                }
                else
                {
                    /* We didn't need to send the payload. Since we had successfully sent the HTTP Response, */
                    /* we are done. We change state to _eStreamingDone (as if we got the PBIP callback for streaming done). */
                    /* And then we re-run this state machine to carry out the eStreamingDone state processing. */
                    /* That will make the state machine send the requestProcessed CB to ServerSocket & endOfStreaming CB to app. */
                    hHttpStreamer->state = BIP_HttpStreamerState_eStreamingDone;
                    reRunProcessState = true;
                    BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: No Payload to send, switched to StreamingDone state!"
                                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
                }
            }
            else
            {
                /* This is a catch-all for any failures in the above code. */

                /* Failure can happen either in BIP_Streamer_Prepare, preparing Response, Sending Response, or in BIP_Streamer_Start. */
                /* If we had run into an error before sending the response, then we send error response. */
                /* Otherwise, _ProcessRequest() failure will allow caller to destroy HttpSocket (& finally socket will be closed). */
                if (hHttpStreamer->successfullResponseSent == false)
                {
                    reRunProcessState = false;
                    sendErrorResponse( hHttpStreamer, hHttpStreamer->processRequestApi.hHttpSocket, hHttpStreamer->processRequestApi.pSettings->hHttpRequest ); /* Note we ignore the status of sendErrorResponse as we want to return the previous error. */
                }

                /* Stop streamer to release any of its resources. */
                BIP_Streamer_Stop( hHttpStreamer->hStreamer );

                /* Start related cleanup (including closing of Nexus Resources) should be already done either above or in the createAndStartPBipStreamer(). */
                /* So we just reset the streamer state. */
                resetHttpStreamerResponseState( hHttpStreamer );
                hHttpStreamer->state = BIP_HttpStreamerState_eIdle;
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: BIP_HttpStreamer_ProcessRequest Failed, completionStatus %s"
                            BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_StatusGetText(hHttpStreamer->completionStatus) ));
            }
            if (hHttpStreamer->completionStatus != BIP_INF_IN_PROGRESS)
            {
                BIP_Arb_CompleteRequest( hHttpStreamer->processRequestApi.hArb, hHttpStreamer->completionStatus);
            }
#if 0
            /* TODO: remove this. */
            if (hHttpStreamer->startSettings.asyncCallback.callback)
            {
                BIP_Arb_AddDeferredCallback( hHttpStreamer->startApi.hArb, &hHttpStreamer->startSettings.asyncCallback );
                *hHttpStreamer->startSettings.pAsyncStatus = hHttpStreamer->completionStatus;
            }
#endif

            destroyDtcpIpAkeTimer( hHttpStreamer );

            /* Go back to top to re-run the state machine if we determined to do so! */
            if ( reRunProcessState == true ) continue;

        }
        BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--- hHttpStreamer %p: state %s completionStatus=%s"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), BIP_StatusGetText(hHttpStreamer->completionStatus) ));
    }
} /* processHttpDirectStreamerState */

void processHttpStreamerState(
    void *hObject,
    int value,
    BIP_Arb_ThreadOrigin threadOrigin
    )
{
    BIP_HttpStreamerHandle  hHttpStreamer = hObject;    /* HttpStreamer object handle */
    BIP_ArbHandle           hArb;
    BIP_Status              brc = BIP_ERR_INTERNAL;
    BIP_Status              completionStatus = BIP_ERR_INTERNAL;

    BSTD_UNUSED(value);

    brc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpStreamer, hHttpStreamer);
    if (brc != BIP_SUCCESS) { return; }

    BDBG_ASSERT(hHttpStreamer);
    BDBG_OBJECT_ASSERT( hHttpStreamer, BIP_HttpStreamer);

    /*
     ***************************************************************************************************************
     * HttpStreamer State Machine Processing:
     *
     * Note: Streamer Settings related APIs are required to be called before the _HttpStreamer_Start().
     * These are _Set*Input, _SetOutput, _AddTracks, _SetProgram, _SetResponseHeaders, etc.
     * In these APIs, we will just cache the caller provided settings but not acquire any
     * Nexus Resources needed for streaming. Once _Start() is called, then we will acquire & setup
     * the required resources for streaming from a input to a particular output method.
     *
     ***************************************************************************************************************
     */

    B_Mutex_Lock( hHttpStreamer->hStateMutex );
    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTRY ---> hHttpStreamer %p: state %s"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));

    if (BIP_Arb_IsNew(hArb = hHttpStreamer->getSettingsApi.hArb))
    {
        /* App is requesting current HttpStreamer settings. */
        BIP_Arb_AcceptRequest(hArb);

        /* Return the current cached settings. */
        *hHttpStreamer->getSettingsApi.pSettings  = hHttpStreamer->settings;

        /* We are done this API Arb, so set its completion status. */
        hHttpStreamer->completionStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: GetSettings Arb request is complete: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
        BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->getStatusApi.hArb))
    {
        /* App is requesting current HttpStreamer settings. */
        BIP_Arb_AcceptRequest(hArb);

        /* Return the current status. */
        hHttpStreamer->getStatusApi.pStatus->stats = hHttpStreamer->stats;
        hHttpStreamer->completionStatus = BIP_Streamer_GetStatus( hHttpStreamer->hStreamer, &hHttpStreamer->getStatusApi.pStatus->streamerStatus );

        {
            B_Time curTime;
            B_Time_Get( &curTime );
            hHttpStreamer->getStatusApi.pStatus->inactivityTimeInMs = B_Time_Diff( &curTime, &hHttpStreamer->lastActivityTime );
        }
        hHttpStreamer->completionStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: GetStatus Arb request is complete: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
        BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->printStatusApi.hArb))
    {
        /* App is requesting to print HttpStreamer status. */
        BIP_Arb_AcceptRequest(hArb);

#ifdef NEXUS_HAS_ASP
        if (!hHttpStreamer->hAspChannel)
#endif
        {
            BIP_Streamer_PrintStatus( hHttpStreamer->hStreamer );
        }
        httpStreamerPrintStatus( hHttpStreamer );

        hHttpStreamer->completionStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: GetStatus Arb request is complete: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
        BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->setSettingsApi.hArb))
    {
        BIP_Arb_AcceptRequest(hArb);
        hHttpStreamer->settings = *hHttpStreamer->setSettingsApi.pSettings;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: SetSettings Arb request is complete : state %s!"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
        hHttpStreamer->completionStatus = BIP_SUCCESS;
        BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->fileInputSettingsApi.hArb))
    {
        /* We only allow streamer sub-state changes in the Idle state. */

        if (hHttpStreamer->state != BIP_HttpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Calling BIP_Arb_RejectRequest(): BIP_HttpStreamer_SetFileInputSettings not allowed in this state: %s, Streamer must be in the Idle state"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));

            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Pass input settings directly to the Streamer object and let it validate it. */
            hHttpStreamer->completionStatus = BIP_Streamer_SetFileInputSettings(
                    hHttpStreamer->hStreamer,
                    hHttpStreamer->fileInputSettingsApi.pMediaFileAbsolutePathName,
                    hHttpStreamer->fileInputSettingsApi.pStreamerStreamInfo,
                    hHttpStreamer->fileInputSettingsApi.pFileInputSettings);

            BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->tunerInputSettingsApi.hArb))
    {
        if (hHttpStreamer->state != BIP_HttpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Calling BIP_Arb_RejectRequest(): BIP_HttpStreamer_SetTunerInputSettings not allowed in this state: %s, Streamer must be in the Idle state"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Pass input settings directly to the Streamer object and let it validate it. */
            hHttpStreamer->completionStatus = BIP_Streamer_SetTunerInputSettings(
                    hHttpStreamer->hStreamer,
                    hHttpStreamer->tunerInputSettingsApi.hParserBand,
                    hHttpStreamer->tunerInputSettingsApi.pStreamerStreamInfo,
                    hHttpStreamer->tunerInputSettingsApi.pTunerInputSettings);

            BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->ipInputSettingsApi.hArb))
    {
        if (hHttpStreamer->state != BIP_HttpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Calling BIP_Arb_RejectRequest(): BIP_HttpStreamer_SetIpInputSettings not allowed in this state: %s, Streamer must be in the Idle state"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Pass input settings directly to the Streamer object and let it validate it. */
            hHttpStreamer->completionStatus = BIP_Streamer_SetIpInputSettings(
                    hHttpStreamer->hStreamer,
                    hHttpStreamer->ipInputSettingsApi.hPlayer,
                    hHttpStreamer->ipInputSettingsApi.pStreamerStreamInfo,
                    hHttpStreamer->ipInputSettingsApi.pIpInputSettings);

            BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->recpumpInputSettingsApi.hArb))
    {
        if (hHttpStreamer->state != BIP_HttpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Calling BIP_Arb_RejectRequest(): BIP_HttpStreamer_SetRecpumpInputSettings not allowed in this state: %s, Streamer must be in the Idle state"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Pass input settings directly to the Streamer object and let it validate it. */
            hHttpStreamer->completionStatus = BIP_Streamer_SetRecpumpInputSettings(
                    hHttpStreamer->hStreamer,
                    hHttpStreamer->recpumpInputSettingsApi.hRecpump,
                    hHttpStreamer->recpumpInputSettingsApi.pRecpumpInputSettings);

            BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus );
        }
    }
#if NEXUS_HAS_HDMI_INPUT
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->hdmiInputSettingsApi.hArb))
    {
        if (hHttpStreamer->state != BIP_HttpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Calling BIP_Arb_RejectRequest(): BIP_HttpStreamer_SetHdmiInputSettings not allowed in this state: %s, Streamer must be in the Idle state"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Pass input settings directly to the Streamer object and let it validate it. */
            hHttpStreamer->completionStatus = BIP_Streamer_SetHdmiInputSettings(
                    hHttpStreamer->hStreamer,
                    hHttpStreamer->hdmiInputSettingsApi.hHdmiInput,
                    hHttpStreamer->hdmiInputSettingsApi.pHdmiInputSettings);

            BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus );
        }
    }
#endif
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->outputSettingsApi.hArb))
    {
        /*
         * We cache the settings into streamer object.
         * Note: API side code has already verified the requied Setting parameters!
         * Input & Output Settings can be set in any order.
         */
        if (hHttpStreamer->state != BIP_HttpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_SetOutputSettings not allowed in this state: %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else if ( hHttpStreamer->outputSettingsApi.pOutputSettings->appInitialPayload.valid )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_SetOutputSettings: output.settings: appInitialPayload are not yet supported!"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Save the output settings */
            hHttpStreamer->output.settings = *hHttpStreamer->outputSettingsApi.pOutputSettings;
            hHttpStreamer->output.streamerProtocol = hHttpStreamer->outputSettingsApi.streamerProtocol;
            hHttpStreamer->completionStatus = BIP_Streamer_SetOutputSettings(
                    hHttpStreamer->hStreamer,
                    BIP_StreamerProtocol_eTcp,
                    &hHttpStreamer->outputSettingsApi.pOutputSettings->streamerSettings );
            if ( hHttpStreamer->completionStatus == BIP_SUCCESS )
            {
                hHttpStreamer->output.state = BIP_HttpStreamerOutputState_eSet;
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: output.settings are cached: timestamp=%s, protocol=%d"
                            BIP_MSG_PRE_ARG, (void *)hHttpStreamer, hHttpStreamer->output.settings.streamerSettings.mpeg2Ts.enableTransportTimestamp?"Y":"N",
                            hHttpStreamer->output.streamerProtocol
                         ));
            }
            BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->addTrackApi.hArb))
    {
        if (hHttpStreamer->state != BIP_HttpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_AddTrack() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Add the Track settings to a list of tracks. */
            hHttpStreamer->completionStatus = BIP_Streamer_AddTrack( hHttpStreamer->hStreamer, hHttpStreamer->addTrackApi.pStreamerTrackInfo , hHttpStreamer->addTrackApi.pTrackSettings );
            BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->addTranscodeProfileApi.hArb))
    {
        if (hHttpStreamer->state != BIP_HttpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_AddTranscodeProfile() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            hHttpStreamer->completionStatus = BIP_Streamer_AddTranscodeProfile( hHttpStreamer->hStreamer, hHttpStreamer->addTranscodeProfileApi.pTranscodeProfile );
            BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->setTranscodeNexusHandlesApi.hArb))
    {
        if (hHttpStreamer->state != BIP_HttpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_SetTranscodeProfile() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            hHttpStreamer->completionStatus = BIP_Streamer_SetTranscodeHandles( hHttpStreamer->hStreamer, hHttpStreamer->setTranscodeNexusHandlesApi.pTranscodeNexusHandles );
            BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->setResponseHeaderApi.hArb))
    {
        /* App wants to set a HTTP Header in the Response. */
        if (hHttpStreamer->state != BIP_HttpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_SetResponseHeader() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        /* Make sure both Input & Output are set, can't prepare the response otherwise. */
        else if (hHttpStreamer->pStreamer->file.inputState == BIP_StreamerInputState_eNotSet &&
                 hHttpStreamer->pStreamer->tuner.inputState == BIP_StreamerInputState_eNotSet &&
                 hHttpStreamer->pStreamer->recpump.inputState == BIP_StreamerInputState_eNotSet &&
                 hHttpStreamer->output.state == BIP_HttpStreamerOutputState_eNotSet
                )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_SetResponseHeader() is not allowed when Input or Output are not set!." BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            hHttpStreamer->completionStatus = BIP_SUCCESS;

            /* Check if we have already prepared the Response, otherwise do it one time before adding this new header. */
            if (hHttpStreamer->response.state == BIP_HttpStreamerResponseHeadersState_eNotSet)
            {
                hHttpStreamer->completionStatus = prepareCommonResponseHeaders( hHttpStreamer );
                if (hHttpStreamer->completionStatus == BIP_SUCCESS)
                {
                    /* We have successfully prepared the default Http Response, update ResponseHeader state. */
                    hHttpStreamer->response.state = BIP_HttpStreamerResponseHeadersState_eSet;
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Response headers prepared!" BIP_MSG_PRE_ARG, (void *)hHttpStreamer));
                }
            }
            if (hHttpStreamer->completionStatus == BIP_SUCCESS)
            {
                BIP_HttpHeaderHandle hHeader;
                /* Base Response Headers are (already) prepared, so Now add the app provided header. */
                /* TODO: Currently, HttpResponse class doesn't provide API to _GetHeader so that we can validate */
                /* if app is trying to add a BIP Reserved Header or already added header. Until then, we just blindly add the header. */
                hHeader = BIP_HttpResponse_AddHeader(
                            hHttpStreamer->response.hHttpResponse,
                            hHttpStreamer->setResponseHeaderApi.pHeaderName,
                            hHttpStreamer->setResponseHeaderApi.pHeaderValue,
                            NULL
                            );
                if(hHeader == NULL)
                {
                    hHttpStreamer->completionStatus = BIP_ERR_INTERNAL;
                }

            }
            if ( hHttpStreamer->completionStatus == BIP_SUCCESS )
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_SetResponseHeader success: header name: %s, value: %s" BIP_MSG_PRE_ARG,
                            (void *)hHttpStreamer,
                            hHttpStreamer->setResponseHeaderApi.pHeaderName,
                            hHttpStreamer->setResponseHeaderApi.pHeaderValue
                         ));
            }
            else
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Failed to %s: header name: %s, value: %s" BIP_MSG_PRE_ARG,
                            (void *)hHttpStreamer,
                            hHttpStreamer->response.state == BIP_HttpStreamerResponseHeadersState_eSet ? "Add HTTP Header" : "Prepare Base Headers",
                            hHttpStreamer->setResponseHeaderApi.pHeaderName,
                            hHttpStreamer->setResponseHeaderApi.pHeaderValue
                         ));
            }
            BIP_Arb_CompleteRequest( hArb, hHttpStreamer->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->getResponseHeaderApi.hArb))
    {
        /* App is starting the Server, make sure we are in the correct state to do so! */
        if (hHttpStreamer->state != BIP_HttpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_GetResponseHeader() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        /* Make sure both Input & Output are set, can't prepare the response otherwise. */
        else if (hHttpStreamer->pStreamer->file.inputState == BIP_StreamerInputState_eNotSet &&
                 hHttpStreamer->pStreamer->tuner.inputState == BIP_StreamerInputState_eNotSet &&
                 hHttpStreamer->pStreamer->recpump.inputState == BIP_StreamerInputState_eNotSet &&
                 hHttpStreamer->output.state == BIP_HttpStreamerOutputState_eNotSet
                )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_GetResponseHeader() is not allowed when Input or Output are not set!." BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* TODO: Not supporting this API until HttpResponse object supports the GetHeader(). */
            hHttpStreamer->completionStatus = BIP_ERR_NOT_AVAILABLE;
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Failed to Prepare the HTTP Response Headers!" BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->startApi.hArb))
    {
        /* App is starting the Server, make sure we are in the correct state to do so! */
        if (hHttpStreamer->state != BIP_HttpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_Start() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        /* Make sure required settings for streaming are provided, can't start streamer otherwise! */
        else if (hHttpStreamer->pStreamer->file.inputState == BIP_StreamerInputState_eNotSet &&
                 hHttpStreamer->pStreamer->tuner.inputState == BIP_StreamerInputState_eNotSet &&
                 hHttpStreamer->pStreamer->recpump.inputState == BIP_StreamerInputState_eNotSet &&
                 hHttpStreamer->output.state == BIP_HttpStreamerOutputState_eNotSet
                )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_Start() is not allowed when Input or Output are not set!." BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else if ( hHttpStreamer->pStreamer->recpump.inputState == BIP_StreamerInputState_eSet && hHttpStreamer->output.streamerProtocol != BIP_HttpStreamerProtocol_eDirect )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_Start() is not allowed when Output StreamerProtocol is not set to eDirect for Recpump input!." BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else if ( hHttpStreamer->pStreamer->file.inputState == BIP_StreamerInputState_eSet && hHttpStreamer->output.streamerProtocol == BIP_HttpStreamerProtocol_eHls && hHttpStreamer->pStreamer->streamerStreamInfo.durationInMs == 0 )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_Start() Failed as for Adaptive Streaming of File Input, streamerStreamInfo duration is required to be NON-Zero!." BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else if ( hHttpStreamer->output.settings.enableDtcpIp && hHttpStreamer->startApi.pSettings->hInitDtcpIp == NULL )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_Start() Failed as App has enabled DTCP/IP streamer option but hasn't enabled DTCP/IP Server during HttpServer Start. " BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
#if NEXUS_HAS_HDMI_INPUT
        else if ( hHttpStreamer->pStreamer->hdmiInput.inputState == BIP_StreamerInputState_eSet && hHttpStreamer->pStreamer->transcode.profileState == BIP_StreamerOutputState_eNotSet )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_Start() Failed HDMI input requires app tp setup transcode profile via BIP_HttpStreamer_AddTranscodeProfile()!." BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
#endif
        else
        {
            BIP_Arb_AcceptRequest(hArb);
            hHttpStreamer->completionStatus = BIP_SUCCESS;

            /* We have confirmed that all streaming related states are valid and thus their settings are in place. */
            /* Note: we dont acquire & setup any Nexus streaming resources until caller invokes the _ProcessRequest(). */
            hHttpStreamer->startSettings = *hHttpStreamer->startApi.pSettings;

            hHttpStreamer->completionStatus = createInactivityTimer( hHttpStreamer );
            if ( hHttpStreamer->completionStatus == BIP_SUCCESS )
            {
                B_Time_Get( &hHttpStreamer->lastActivityTime );
                /* NOTE: we change to a temporary SetupComplete state (w/o completing the ARB) */
                /* This way we let the per streaming protocol state logic below do any streaming start related processing. */
                hHttpStreamer->state = BIP_HttpStreamerState_eSetupComplete;
            }
            else
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_Start() Failed due to createInactivityTimer() failure to create a timer! " BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
                BIP_Arb_RejectRequest((void *)hArb, hHttpStreamer->completionStatus);
            }
        }
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->processRequestApi.hArb))
    {
        if (hHttpStreamer->state == BIP_HttpStreamerState_eIdle || hHttpStreamer->state == BIP_HttpStreamerState_eWaitingForStopApi)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Can't process new request in state=%s (either streamer is already stopped or has run into an internal error & waiting for app to Stop it."
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
            sendErrorResponse( hHttpStreamer, hHttpStreamer->processRequestApi.hHttpSocket, hHttpStreamer->processRequestApi.pSettings->hHttpRequest ); /* Note we ignore the status of sendErrorResponse as we want to return the previous error. */
            hHttpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else
        {
            /* Let the individual sub-states determine if this API could be called in their states. */
            hHttpStreamer->completionStatus = BIP_INF_IN_PROGRESS;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Accepted _ProcessRequest API: state %s!"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
        }
        /* NOTE: The remaining ProcessRequest processing happens in the per streaming protocol state logic below. */
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->stopApi.hArb))
    {
        if ( hHttpStreamer->state != BIP_HttpStreamerState_eStreaming &&
             hHttpStreamer->state != BIP_HttpStreamerState_eStreamingDone &&
             hHttpStreamer->state != BIP_HttpStreamerState_eWaitingForClientAke &&
             hHttpStreamer->state != BIP_HttpStreamerState_eWaitingForStopApi &&
             hHttpStreamer->state != BIP_HttpStreamerState_eWaitingForProcessRequestApi
           )
        {
            BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpStreamer %p: BIP_HttpStreamer_Stop not allowed in this state: %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
            hHttpStreamer->completionStatus = BIP_SUCCESS;
            /* Cancel the current inactivity timer. */
            destroyInactivityTimer( hHttpStreamer );
            destroyDtcpIpAkeTimer( hHttpStreamer );
            BIP_Arb_RejectRequest(hArb, hHttpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);
            hHttpStreamer->completionStatus = BIP_INF_IN_PROGRESS;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Accepted _Stop Arb: state %s!"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));

            /* Cancel the current inactivity timer. */
            destroyInactivityTimer( hHttpStreamer );
            destroyDtcpIpAkeTimer( hHttpStreamer );

            /* NOTE: The remaining ProcessRequest processing happens in the per streaming protocol state logic below. */
        }
    }
    else if (BIP_Arb_IsNew(hArb = hHttpStreamer->destroyApi.hArb))
    {
        BIP_Arb_AcceptRequest(hArb);
        hHttpStreamer->completionStatus = BIP_INF_IN_PROGRESS;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Accepted _Destroy Arb: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));

        /* Cancel the current inactivity timer. */
        destroyInactivityTimer( hHttpStreamer );
        /* NOTE: The remaining DestroyApi processing happens in the per streaming protocol state logic below. */
    }

    /* Now run the state machine of streaming protocol specific logic. */
    switch (hHttpStreamer->output.streamerProtocol)
    {
        case BIP_HttpStreamerProtocol_eDirect:
            /* Note: Arb is accepted inside this state function. */
            processHttpDirectStreamerState( hHttpStreamer, 0, threadOrigin );
            break;
#if NEXUS_HAS_VIDEO_ENCODER
        case BIP_HttpStreamerProtocol_eHls:
            processHttpHlsStreamerState( hHttpStreamer, 0, threadOrigin );
            break;
        case BIP_HttpStreamerProtocol_eMpegDash:
#endif /* NEXUS_HAS_VIDEO_ENCODER */
        default:
            BDBG_ASSERT(NULL);
            break;
    }

    /* Check if streamer inactivity timer has expired. */
    if ( hHttpStreamer->inactivityTimerActive && hHttpStreamer->state > BIP_HttpStreamerState_eSetupComplete )
    {
        B_Time endTime;

        B_Time_Get( &endTime );
        /* Check streamer status if poll interval has expired. */
        if ( B_Time_Diff( &endTime, &hHttpStreamer->inactivityTimerStartTime ) >= BIP_HTTP_STREAMER_INACTIVITY_TIMER_POLL_INTERVAL_IN_MSEC )
        {
            bool timerRestarted = false, inactivityTimeoutCallbackIssued=false, endOfStreamingCallbackIssued=false;
            int64_t bytesStreamed = 0;

            /* Check the streamer status. */
            if ( hHttpStreamer->playbackIpState.hLiveStreamer )
            {
                B_PlaybackIpLiveStreamingStatus status;
                B_PlaybackIp_LiveStreamingGetStatus( hHttpStreamer->playbackIpState.hLiveStreamer, &status );
                bytesStreamed = status.bytesStreamed;
            }
            else if ( hHttpStreamer->playbackIpState.hFileStreamer )
            {
                B_PlaybackIpFileStreamingStatus status;
                B_PlaybackIp_FileStreamingGetStatus( hHttpStreamer->playbackIpState.hFileStreamer, &status );
                bytesStreamed = status.bytesStreamed;
            }
#ifdef NEXUS_HAS_ASP
            else if (hHttpStreamer->hAspChannel)
            {
                B_AspChannelStatus status;
                B_AspChannel_GetStatus(hHttpStreamer->hAspChannel, &status);
                bytesStreamed = status.nexusStatus.stats.mcpbConsumedInBytes;
            }
#endif
            if ( bytesStreamed != hHttpStreamer->totalBytesStreamed )
            {
                /* Currently streamed bytes dont match the one from the last poll interval, so streamer is still actively streaming. */
                hHttpStreamer->lastActivityTime = endTime;
                hHttpStreamer->totalBytesStreamed = bytesStreamed;
            }
            else
            {
                /* Streamed bytes count haven't changed. Check if we have exceeded the inactivity timeout interval. */
                if ( hHttpStreamer->startSettings.inactivityTimeoutInMs > 0 && B_Time_Diff( &endTime, &hHttpStreamer->lastActivityTime ) >= hHttpStreamer->startSettings.inactivityTimeoutInMs )
                {
                    /* App has defined inactivity timeout and streamer has been inactive for that duration, so time to let app know about it. */
                    BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: inactivity timer = %d expired, totalBytesStreamed=%" PRIu64
                                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), hHttpStreamer->startSettings.inactivityTimeoutInMs, hHttpStreamer->totalBytesStreamed ));
                    hHttpStreamer->inactivityTimerExpired = true;
                    if (hHttpStreamer->startSettings.inactivityTimeoutCallback.callback)
                    {
                        /* App wants to get the in-activity timeout callback, so lets issue that. */
                        BIP_Arb_AddDeferredCallback( hHttpStreamer->startApi.hArb, &hHttpStreamer->startSettings.inactivityTimeoutCallback );
                        /* Note: we remain in the same state and let app decide how it wants to handle the inactivity timeout case. */
                        inactivityTimeoutCallbackIssued = true;
                    }
                    else if (hHttpStreamer->createSettings.endOfStreamingCallback.callback)
                    {
                        /* Else, app didn't define the inactivity timeout callback but had the inactivity timeout set, so we issue the endOfStreaming Callback. */

                        /* queue up endOfStreaming CB */
                        initiateEndOfStreamingCallback( hHttpStreamer ); /* function changes the state to eWaitingForStopApi */

                        /* Restart the inactivity poll timer if we are not in the WaitingForStopApi state. */
                        endOfStreamingCallbackIssued = true;
                    }
                }
            }
            if ( hHttpStreamer->state != BIP_HttpStreamerState_eWaitingForStopApi && hHttpStreamer->state != BIP_HttpStreamerState_eIdle )
            {
                hHttpStreamer->completionStatus = createInactivityTimer( hHttpStreamer );
                if ( hHttpStreamer->completionStatus != BIP_SUCCESS )
                {
                    initiateEndOfStreamingCallback( hHttpStreamer ); /* function changes the state to eWaitingForStopApi */
                    BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpStreamer %p, state %s: Failed to restart inactivity timer, issued endOfStreaming Callback to App!"
                                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state) ));
                }
                else timerRestarted = true;
            }
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p: state=%s inactivityTimerRestarted=%s inactivityTimeoutCallbackIssued=%s endOfStreamingCallbackIssued=%s totalBytesStreamed=%"PRId64 " inactivityTimeout=%d"
                        BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state),
                        timerRestarted?"Y":"N", inactivityTimeoutCallbackIssued?"Y":"N", endOfStreamingCallbackIssued?"Y":"N", hHttpStreamer->totalBytesStreamed, hHttpStreamer->startSettings.inactivityTimeoutInMs  ));
        } /* inactivity poll timer expired. */
    } /* inactivityTimer is active. */

    /*
     * Done with state processing. We have to unlock state machine before asking Arb to do any deferred callbacks!
     */
    B_Mutex_Unlock( hHttpStreamer->hStateMutex );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Finished Processing HTTP State for hHttpStreamer %p: state %s, before issuing the callbacks with completionStatus 0x%x"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state), completionStatus ));

    BIP_CLASS_UNLOCK(BIP_HttpStreamer, hHttpStreamer);

    /* Tell ARB to do any deferred work. */
    brc = BIP_Arb_DoDeferred( hHttpStreamer->startApi.hArb, threadOrigin );
    BDBG_ASSERT( brc == BIP_SUCCESS );


    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--- hHttpStreamer %p: state %s"
                BIP_MSG_PRE_ARG, (void *)hHttpStreamer, BIP_HTTP_STREAMER_STATE(hHttpStreamer->state)));
    return;
} /* processHttpStreamerState */
