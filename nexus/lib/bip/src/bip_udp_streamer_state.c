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
 *****************************************************************************/

#include "b_os_lib.h"
#include "bip_udp_streamer_impl.h"
#include "bip_priv.h"

BDBG_MODULE( bip_udp_streamer );
BDBG_OBJECT_ID_DECLARE( BIP_UdpStreamer );

struct BIP_UdpStreamerStateNames
{
    BIP_UdpStreamerState state;
    char *pStateName;
}gUdpStreamerState[] = {
    {BIP_UdpStreamerState_eUninitialized, "UnInitialized"},
    {BIP_UdpStreamerState_eIdle, "Idle"},
    {BIP_UdpStreamerState_eStreaming, "Streaming"},
    {BIP_UdpStreamerState_eStreamingDone, "StreamingDone"},    /* transitional state */
    {BIP_UdpStreamerState_eWaitingForStopApi, "WaitingForStopOrResumeApi"},
    {BIP_UdpStreamerState_eMax, "MaxState"}
};
#define BIP_UDP_STREAMER_STATE(state) \
    gUdpStreamerState[state].pStateName

static void resetOutputState(
    BIP_UdpStreamerHandle hUdpStreamer
    )
{
    if (hUdpStreamer->output.hIpAddress) BIP_String_Destroy( hUdpStreamer->output.hIpAddress );
    hUdpStreamer->output.hIpAddress = NULL;

    if (hUdpStreamer->output.hPort) BIP_String_Destroy( hUdpStreamer->output.hPort );
    hUdpStreamer->output.hPort = NULL;

    if (hUdpStreamer->output.hInterfaceName) BIP_String_Destroy( hUdpStreamer->output.hInterfaceName );
    hUdpStreamer->output.hInterfaceName = NULL;

    hUdpStreamer->output.state = BIP_UdpStreamerOutputState_eNotSet;
} /* resetOutputState */

static void stopPBipStreamer(
    BIP_UdpStreamerHandle hUdpStreamer
    )
{
    if (hUdpStreamer->playbackIpState.hLiveStreamer)
    {
        B_PlaybackIp_LiveStreamingStop(hUdpStreamer->playbackIpState.hLiveStreamer);
        BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hUdpStreamer %p: state %s: PBIP based live streamer is stopped"
                    BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state) ));
    }
} /* stopPBipStreamer */

static void destroyPBipStreamer(
    BIP_UdpStreamerHandle hUdpStreamer
    )
{
    if (hUdpStreamer->playbackIpState.hLiveStreamer)
    {
        B_PlaybackIp_LiveStreamingClose(hUdpStreamer->playbackIpState.hLiveStreamer);
        hUdpStreamer->playbackIpState.hLiveStreamer = NULL;
        BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hUdpStreamer %p: state %s: PBIP based live streamer is Destroyed!"
                    BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state) ));
    }
} /* destroyPBipStreamer */

static void stopAndDestroyPBipStreamer(
    BIP_UdpStreamerHandle hUdpStreamer
    )
{
    stopPBipStreamer( hUdpStreamer );
    destroyPBipStreamer( hUdpStreamer );
} /* stopAndDestroyPBipStreamer */

void processUdpStreamerState( void *jObject, int value, BIP_Arb_ThreadOrigin threadOrigin );
static void playbackIpStreamerCallbackViaArbTimer(
    void *appCtx,
    int   param
    )
{
    BIP_UdpStreamerHandle hUdpStreamer = appCtx;

    BDBG_ASSERT(hUdpStreamer);
    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer );
    BSTD_UNUSED(param);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hUdpStreamer: %p" BIP_MSG_PRE_ARG, (void *)hUdpStreamer ));
    processUdpStreamerState( (BIP_UdpStreamerHandle) hUdpStreamer, 0, BIP_Arb_ThreadOrigin_eTimer);
} /* playbackIpStreamerCallbackViaArbTimer */

static void playbackIpStreamerCallback(
    void *appCtx,
    B_PlaybackIpEventIds eventId
    )
{
    BIP_Status brc;
    BIP_UdpStreamerHandle hUdpStreamer = appCtx;

    BDBG_ASSERT(hUdpStreamer);
    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hUdpStreamer:state %p: %s, got eventId %d from PBIP: Defer the callback"
                BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state), eventId ));
    if (hUdpStreamer)
    {
        B_Mutex_Lock( hUdpStreamer->hStateMutex );
        hUdpStreamer->state = BIP_UdpStreamerState_eStreamingDone;
        hUdpStreamer->playbackIpState.pbipEndOfStreamingCallback.callback = &playbackIpStreamerCallbackViaArbTimer;
        hUdpStreamer->playbackIpState.pbipEndOfStreamingCallback.context = hUdpStreamer;
        BIP_Arb_AddDeferredCallback( hUdpStreamer->startApi.hArb, &hUdpStreamer->playbackIpState.pbipEndOfStreamingCallback );
        B_Mutex_Unlock( hUdpStreamer->hStateMutex );

        brc = BIP_Arb_DoDeferred( hUdpStreamer->startApi.hArb, BIP_Arb_ThreadOrigin_eUnknown);
        BDBG_ASSERT( brc == BIP_SUCCESS );
    }
} /* playbackIpStreamerCallback */

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

static BIP_Status startPBipLiveStreamer(
    BIP_UdpStreamerHandle hUdpStreamer
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    B_Error rc;

    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTRY ---> hUdpStreamer %p: state %s"
                BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state) ));

    /* Setup a PBIP LiveStreaming Session */
    {
        B_PlaybackIpLiveStreamingOpenSettings liveStreamingOpenSettings;

        memset( &liveStreamingOpenSettings, 0, sizeof( liveStreamingOpenSettings ));
        if (hUdpStreamer->output.settings.appInitialPayload.valid)
        {
            liveStreamingOpenSettings.appHeader.valid = true;
            liveStreamingOpenSettings.appHeader.length = hUdpStreamer->output.settings.appInitialPayload.length;
            memcpy( liveStreamingOpenSettings.appHeader.data, hUdpStreamer->output.settings.appInitialPayload.pPayload, sizeof(liveStreamingOpenSettings.appHeader.data));
        }

        /* Set Streaming IP, Port, Protocol, & Interface Info. */
        {
            if ( hUdpStreamer->output.streamerProtocol == BIP_UdpStreamerProtocol_eRtp )
                liveStreamingOpenSettings.protocol = B_PlaybackIpProtocol_eRtp;
            else
                liveStreamingOpenSettings.protocol = B_PlaybackIpProtocol_eUdp;
            liveStreamingOpenSettings.rtpUdpSettings.interfaceName = BIP_String_GetString(hUdpStreamer->output.hInterfaceName);
            liveStreamingOpenSettings.rtpUdpSettings.streamingPort = atoi(BIP_String_GetString(hUdpStreamer->output.hPort));
            strncpy(liveStreamingOpenSettings.rtpUdpSettings.streamingIpAddress, BIP_String_GetString(hUdpStreamer->output.hIpAddress), sizeof(liveStreamingOpenSettings.rtpUdpSettings.streamingIpAddress)-1);
        }

        /* Set Stream related info. */
        {
            liveStreamingOpenSettings.transportTimestampEnabled = hUdpStreamer->pStreamer->output.settings.mpeg2Ts.enableTransportTimestamp;
        }

        /* Set callback to know when the streaming is over. */
        {
            liveStreamingOpenSettings.eventCallback = playbackIpStreamerCallback;
            liveStreamingOpenSettings.appCtx = hUdpStreamer;
        }

        /* Set any Security related settings. */
#if 0
        /* TODO: enable this when DTCP/IP library supports UDP/RTP Protocols. */
        if (hUdpStreamer->output.settings.enableDtcpIp)
        {
            BIP_DtcpIpServerStatus dtcpIpServerStatus;

            BIP_DtcpIpServer_GetStatus( hUdpStreamer->startSettings.hInitDtcpIp, &dtcpIpServerStatus );

            liveStreamingOpenSettings.securitySettings.securityProtocol = B_PlaybackIpSecurityProtocol_DtcpIp;
            liveStreamingOpenSettings.securitySettings.enableEncryption = true;
            liveStreamingOpenSettings.securitySettings.initialSecurityContext = dtcpIpServerStatus.pDtcpIpLibCtx;
            liveStreamingOpenSettings.securitySettings.settings.dtcpIp.emiValue = hUdpStreamer->output.settings.dtcpIpOutput.copyControlInfo;
            liveStreamingOpenSettings.securitySettings.settings.dtcpIp.akeTimeoutInMs = hUdpStreamer->output.settings.dtcpIpOutput.akeTimeoutInMs;
            liveStreamingOpenSettings.securitySettings.settings.dtcpIp.pcpPayloadLengthInBytes = hUdpStreamer->output.settings.dtcpIpOutput.pcpPayloadLengthInBytes;
        }
#endif

        /* Set remaining settings. */
        {
            liveStreamingOpenSettings.recpumpHandle = hUdpStreamer->pStreamer->hRecpump;
            liveStreamingOpenSettings.heapHandle = getStreamerHeapHandle(hUdpStreamer->output.settings.heapHandle);
            if(hUdpStreamer->startSettings.streamingMethod == BIP_StreamingMethod_eRaveInterruptBased)
            {
                liveStreamingOpenSettings.streamingMethod = B_PlaybackIpStreamingMethod_eRaveInterruptBased;
                liveStreamingOpenSettings.timeOutIntervalInMs =
                    hUdpStreamer->startSettings.streamingSettings.raveInterruptBasedSettings.timeOutIntervalInMs;
            }
            else if(hUdpStreamer->startSettings.streamingMethod == BIP_StreamingMethod_eSystemTimerBased)
            {
                liveStreamingOpenSettings.streamingMethod = B_PlaybackIpStreamingMethod_eSystemTimerBased;
                liveStreamingOpenSettings.timeOutIntervalInMs =
                    hUdpStreamer->startSettings.streamingSettings.systemTimerBasedSettings.timeOutIntervalInMs;
            }

        }

        hUdpStreamer->playbackIpState.hLiveStreamer = B_PlaybackIp_LiveStreamingOpen( &liveStreamingOpenSettings );
        BIP_CHECK_GOTO(( hUdpStreamer->playbackIpState.hLiveStreamer ), ( "B_PlaybackIp_LiveStreamingOpen Failed!" ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    /* Start PBIP Streamer */
    {
        rc = B_PlaybackIp_LiveStreamingStart( hUdpStreamer->playbackIpState.hLiveStreamer );
        BIP_CHECK_GOTO(( !rc ), ( "B_PlaybackIp_LiveStreamingStart Failed!" ), error, BIP_ERR_INTERNAL, bipStatus );
        bipStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hUdpStreamer %p: Streaming from inputType %d Started!" BIP_MSG_PRE_ARG, (void *)hUdpStreamer, hUdpStreamer->hStreamer->inputType));
    }

error:
    if (bipStatus != BIP_SUCCESS)
    {
        stopAndDestroyPBipStreamer( hUdpStreamer );
    }
    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--- hUdpStreamer %p: state %s, bipStatus 0x%x"
                BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state), bipStatus));

    return bipStatus;
} /* startPBipLiveStreamer */

void processUdpStreamerState(
    void *hObject,
    int value,
    BIP_Arb_ThreadOrigin threadOrigin
    )
{
    BIP_UdpStreamerHandle  hUdpStreamer = hObject;    /* UdpStreamer object handle */
    BIP_ArbHandle           hArb;
    BIP_Status              brc = BIP_ERR_INTERNAL;
    BIP_Status              completionStatus = BIP_ERR_INTERNAL;

    BSTD_UNUSED(value);

    BDBG_ASSERT(hUdpStreamer);
    BDBG_OBJECT_ASSERT( hUdpStreamer, BIP_UdpStreamer);

    B_Mutex_Lock( hUdpStreamer->hStateMutex );
    BDBG_MSG(( BIP_MSG_PRE_FMT "ENTRY ---> hUdpStreamer %p: state %s"
                BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state) ));

    if (BIP_Arb_IsNew(hArb = hUdpStreamer->getSettingsApi.hArb))
    {
        /* App is request current UdpStreamer settings. */
        BIP_Arb_AcceptRequest(hArb);

        /* Return the current cached settings. */
        *hUdpStreamer->getSettingsApi.pSettings  = hUdpStreamer->settings;

        /* We are done this API Arb, so set its completion status. */
        hUdpStreamer->completionStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hUdpStreamer %p: GetSettings Arb request is complete: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state) ));
        BIP_Arb_CompleteRequest( hArb, hUdpStreamer->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hUdpStreamer->getStatusApi.hArb))
    {
        /* App is request current UdpStreamer settings. */
        BIP_Arb_AcceptRequest(hArb);

        /* Return the current status. */
        hUdpStreamer->getStatusApi.pStatus->active = hUdpStreamer->state == BIP_UdpStreamerState_eStreaming ? true: false;
        hUdpStreamer->getStatusApi.pStatus->stats = hUdpStreamer->stats;
        hUdpStreamer->completionStatus = BIP_Streamer_GetStatus( hUdpStreamer->hStreamer, &hUdpStreamer->getStatusApi.pStatus->streamerStatus );

        hUdpStreamer->completionStatus = BIP_SUCCESS;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hUdpStreamer %p: GetStatus Arb request is complete: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state) ));
        BIP_Arb_CompleteRequest( hArb, hUdpStreamer->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hUdpStreamer->setSettingsApi.hArb))
    {
        BIP_Arb_AcceptRequest(hArb);
        hUdpStreamer->settings = *hUdpStreamer->setSettingsApi.pSettings;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hUdpStreamer %p: SetSettings Arb request is complete : state %s!"
                    BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state) ));
        hUdpStreamer->completionStatus = BIP_SUCCESS;
        BIP_Arb_CompleteRequest( hArb, hUdpStreamer->completionStatus);
    }
    else if (BIP_Arb_IsNew(hArb = hUdpStreamer->fileInputSettingsApi.hArb))
    {
        /* We only allow streamer sub-state changes in the Idle state. */

        if (hUdpStreamer->state != BIP_UdpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hUdpStreamer %p: Calling BIP_Arb_RejectRequest(): BIP_UdpStreamer_SetFileInputSettings not allowed in this state: %s, Streamer must be in the Idle state"
                        BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state)));

            hUdpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hUdpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Since UDP Streaming is a push based model, we will turn-on the h/w pacing. */
            hUdpStreamer->fileInputSettingsApi.pFileInputSettings->enableHwPacing = true;
            hUdpStreamer->completionStatus = BIP_Streamer_SetFileInputSettings(
                    hUdpStreamer->hStreamer,
                    hUdpStreamer->fileInputSettingsApi.pMediaFileAbsolutePathName,
                    hUdpStreamer->fileInputSettingsApi.pStreamerStreamInfo,
                    hUdpStreamer->fileInputSettingsApi.pFileInputSettings);

            BIP_Arb_CompleteRequest( hArb, hUdpStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hUdpStreamer->tunerInputSettingsApi.hArb))
    {
        if (hUdpStreamer->state != BIP_UdpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hUdpStreamer %p: Calling BIP_Arb_RejectRequest(): BIP_UdpStreamer_SetTunerInputSettings not allowed in this state: %s, Streamer must be in the Idle state"
                        BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state)));
            hUdpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hUdpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Pass input settings directly to the Streamer object and let it validate it. */
            hUdpStreamer->completionStatus = BIP_Streamer_SetTunerInputSettings(
                    hUdpStreamer->hStreamer,
                    hUdpStreamer->tunerInputSettingsApi.hParserBand,
                    hUdpStreamer->tunerInputSettingsApi.pStreamerStreamInfo,
                    hUdpStreamer->tunerInputSettingsApi.pTunerInputSettings);

            BIP_Arb_CompleteRequest( hArb, hUdpStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hUdpStreamer->recpumpInputSettingsApi.hArb))
    {
        if (hUdpStreamer->state != BIP_UdpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hUdpStreamer %p: Calling BIP_Arb_RejectRequest(): BIP_UdpStreamer_SetRecpumpInputSettings not allowed in this state: %s, Streamer must be in the Idle state"
                        BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state)));
            hUdpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hUdpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Pass input settings directly to the Streamer object and let it validate it. */
            hUdpStreamer->completionStatus = BIP_Streamer_SetRecpumpInputSettings(
                    hUdpStreamer->hStreamer,
                    hUdpStreamer->recpumpInputSettingsApi.hRecpump,
                    hUdpStreamer->recpumpInputSettingsApi.pRecpumpInputSettings);

            BIP_Arb_CompleteRequest( hArb, hUdpStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hUdpStreamer->outputSettingsApi.hArb))
    {
        /*
         * We cache the settings into streamer object.
         * Note: API side code has already verified the requied Setting parameters!
         * Input & Output Settings can be set in any order.
         */
        if (hUdpStreamer->state != BIP_UdpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hUdpStreamer %p: BIP_UdpStreamer_SetOutputSettings not allowed in this state: %s"
                        BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state)));
            hUdpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hUdpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Free-up any previously allocated outputState variables. */
            resetOutputState( hUdpStreamer );

            hUdpStreamer->completionStatus = BIP_Streamer_SetOutputSettings( hUdpStreamer->hStreamer, BIP_StreamerProtocol_ePlainUdp, NULL );
            if ( hUdpStreamer->completionStatus == BIP_SUCCESS )
            {
                hUdpStreamer->output.hIpAddress = BIP_String_CreateFromChar( hUdpStreamer->outputSettingsApi.pStreamerIpAddress );
                hUdpStreamer->output.hPort = BIP_String_CreateFromChar( hUdpStreamer->outputSettingsApi.pStreamerPort );
                hUdpStreamer->output.hInterfaceName = BIP_String_CreateFromChar( hUdpStreamer->outputSettingsApi.pStreamerInterfaceName );
                if (hUdpStreamer->output.hIpAddress && hUdpStreamer->output.hPort && hUdpStreamer->output.hInterfaceName)
                {
                    /* Save the output settings */
                    hUdpStreamer->output.settings = *hUdpStreamer->outputSettingsApi.pOutputSettings;
                    hUdpStreamer->output.streamerProtocol = hUdpStreamer->outputSettingsApi.streamerProtocol;
                    hUdpStreamer->output.state = BIP_UdpStreamerOutputState_eSet;
                    hUdpStreamer->completionStatus = BIP_SUCCESS;
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hUdpStreamer %p: ip:port:iface %s:%s:%s, timestamp=%s, protocol=%s"
                                BIP_MSG_PRE_ARG, (void *)hUdpStreamer,
                                BIP_String_GetString(hUdpStreamer->output.hIpAddress),
                                BIP_String_GetString(hUdpStreamer->output.hPort),
                                BIP_String_GetString(hUdpStreamer->output.hInterfaceName),
                                hUdpStreamer->output.settings.streamerSettings.mpeg2Ts.enableTransportTimestamp?"Y":"N",
                                hUdpStreamer->output.streamerProtocol == BIP_UdpStreamerProtocol_eRtp ? "RTP" : "UDP"
                             ));
                }
                else
                {
                    resetOutputState( hUdpStreamer );
                    hUdpStreamer->completionStatus = BIP_ERR_OUT_OF_SYSTEM_MEMORY;
                }
            }
            BIP_Arb_CompleteRequest( hArb, hUdpStreamer->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hUdpStreamer->addTrackApi.hArb))
    {
        if (hUdpStreamer->state != BIP_UdpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hUdpStreamer %p: BIP_UdpStreamer_AddTrack() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state)));
            hUdpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hUdpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            /* Add the Track settings to a list of tracks. */
            hUdpStreamer->completionStatus = BIP_Streamer_AddTrack( hUdpStreamer->hStreamer, hUdpStreamer->addTrackApi.pStreamerTrackInfo , hUdpStreamer->addTrackApi.pTrackSettings );
            BIP_Arb_CompleteRequest( hArb, hUdpStreamer->completionStatus);
        }
    }
    else if (BIP_Arb_IsNew(hArb = hUdpStreamer->addTranscodeProfileApi.hArb))
    {
        if (hUdpStreamer->state != BIP_UdpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hUdpStreamer %p: BIP_UdpStreamer_AddTranscodeProfile() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state)));
            hUdpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hUdpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            hUdpStreamer->completionStatus = BIP_Streamer_AddTranscodeProfile( hUdpStreamer->hStreamer, hUdpStreamer->addTranscodeProfileApi.pTranscodeProfile );
            if ( hUdpStreamer->completionStatus != BIP_SUCCESS )
            {
                resetOutputState( hUdpStreamer );
            }
            BIP_Arb_CompleteRequest( hArb, hUdpStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hUdpStreamer->setTranscodeNexusHandlesApi.hArb))
    {
        if (hUdpStreamer->state != BIP_UdpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hUdpStreamer %p: BIP_UdpStreamer_SetTranscodeProfile() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state)));
            hUdpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hUdpStreamer->completionStatus);
        }
        else
        {
            BIP_Arb_AcceptRequest(hArb);

            hUdpStreamer->completionStatus = BIP_Streamer_SetTranscodeHandles( hUdpStreamer->hStreamer, hUdpStreamer->setTranscodeNexusHandlesApi.pTranscodeNexusHandles );
            if ( hUdpStreamer->completionStatus != BIP_SUCCESS )
            {
                resetOutputState( hUdpStreamer );
            }
            BIP_Arb_CompleteRequest( hArb, hUdpStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hUdpStreamer->startApi.hArb))
    {
        /* App is starting the Server, make sure we are in the correct state to do so! */
        if (hUdpStreamer->state != BIP_UdpStreamerState_eIdle)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hUdpStreamer %p: BIP_UdpStreamer_Start() is only allowed in Idle state, current state: %s"
                        BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state)));
            hUdpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hUdpStreamer->completionStatus);
        }
        /* Make sure required settings for streaming are provided, can't start streamer otherwise! */
        else if (hUdpStreamer->pStreamer->file.inputState == BIP_StreamerInputState_eNotSet &&
                 hUdpStreamer->pStreamer->tuner.inputState == BIP_StreamerInputState_eNotSet &&
                 hUdpStreamer->pStreamer->recpump.inputState == BIP_StreamerInputState_eNotSet &&
                 hUdpStreamer->output.state == BIP_UdpStreamerOutputState_eNotSet
                )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hUdpStreamer %p: BIP_UdpStreamer_Start() is not allowed when Input or Output are not set!." BIP_MSG_PRE_ARG, (void *)hUdpStreamer ));
            hUdpStreamer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hUdpStreamer->completionStatus);
        }
        else
        {
            BIP_StreamerPrepareSettings prepareSettings;
            BIP_Arb_AcceptRequest(hArb);
            hUdpStreamer->completionStatus = BIP_SUCCESS;

            /* We have confirmed that all streaming related states are valid and thus their settings are in place. */
            /* Note: we dont acquire & setup any Nexus streaming resources until caller invokes the _ProcessRequest(). */
            hUdpStreamer->startSettings = *hUdpStreamer->startApi.pSettings;

            BIP_Streamer_GetDefaultPrepareSettings( &prepareSettings );

            /*  This always have to set irrespective of whether we are running in RaveInterruptbased or systemTimer based mode,
                since Rave interrupt internally is always enable only we don't wait for that event in systemTimer mode.
                Now in system timer mode since we are running based on systemTimer,
                so we can set dataReadyThreshold high which eventually reduce the number of interrupt.*/
            prepareSettings.recpumpOpenSettings.data.dataReadyThreshold =
                prepareSettings.recpumpOpenSettings.data.atomSize * hUdpStreamer->startSettings.streamingSettings.raveInterruptBasedSettings.dataReadyScaleFactor;

            hUdpStreamer->completionStatus = BIP_Streamer_Prepare( hUdpStreamer->hStreamer, &prepareSettings );
            if ( hUdpStreamer->completionStatus == BIP_SUCCESS )
            {
                hUdpStreamer->completionStatus = BIP_Streamer_Start( hUdpStreamer->hStreamer, NULL );
            }
            if ( hUdpStreamer->completionStatus == BIP_SUCCESS )
            {
                hUdpStreamer->completionStatus = startPBipLiveStreamer( hUdpStreamer );
            }
            if ( hUdpStreamer->completionStatus == BIP_SUCCESS )
            {
                hUdpStreamer->state = BIP_UdpStreamerState_eStreaming;
                BIP_MSG_SUM(( BIP_MSG_PRE_FMT "StreamingStarted: " BIP_UDP_STREAMER_PRINTF_FMT
                            BIP_MSG_PRE_ARG, BIP_UDP_STREAMER_PRINTF_ARG(hUdpStreamer)));
            }
            BIP_Arb_CompleteRequest( hArb, hUdpStreamer->completionStatus );
        }
    }
    else if (BIP_Arb_IsNew(hArb = hUdpStreamer->stopApi.hArb))
    {
        BIP_Arb_AcceptRequest(hArb);

        /* NOTE: We dont check for state here as these functions validate handles before taking action. */
        /* This takes care of cases where App call UdpStreamer_Set Input/Output/Track APIs, but then just call Stop/Destroy API. */
        stopPBipStreamer( hUdpStreamer );
        BIP_Streamer_Stop( hUdpStreamer->hStreamer );
        destroyPBipStreamer( hUdpStreamer );
        resetOutputState( hUdpStreamer );

        hUdpStreamer->state = BIP_UdpStreamerState_eIdle;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hUdpStreamer %p: BIP_UdpStreamer_Stop successful: %s"
                    BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state)));
        BIP_Arb_CompleteRequest( hArb, hUdpStreamer->completionStatus );
    }
    else if (BIP_Arb_IsNew(hArb = hUdpStreamer->destroyApi.hArb))
    {
        BIP_Arb_AcceptRequest(hArb);
        hUdpStreamer->completionStatus = BIP_INF_IN_PROGRESS;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hUdpStreamer %p: Accepted _Destroy Arb: state %s!"
                    BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state) ));

        /* NOTE: the same comment as above in the stopApi case applies here!!!. */
        stopPBipStreamer( hUdpStreamer );
        BIP_Streamer_Stop( hUdpStreamer->hStreamer );
        destroyPBipStreamer( hUdpStreamer );
        resetOutputState( hUdpStreamer );

        BIP_Arb_CompleteRequest(hArb, BIP_SUCCESS);
    }

    if ( hUdpStreamer->state == BIP_UdpStreamerState_eStreamingDone )
    {
        /* Add a deferred callback to let App know as well that we are done with Streaming. */
        if (hUdpStreamer->createSettings.endOfStreamingCallback.callback)
        {
            BIP_Arb_AddDeferredCallback( hUdpStreamer->startApi.hArb, &hUdpStreamer->createSettings.endOfStreamingCallback );
        }
        hUdpStreamer->state = BIP_UdpStreamerState_eWaitingForStopApi;
        BDBG_MSG(( BIP_MSG_PRE_FMT "hUdpStreamer %p, state %s: Done from StreamingDone state and moving to WaitingForStop state!"
                        BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state)));
    }

    /*
     * Done with state processing. We have to unlock state machine before asking Arb to do any deferred callbacks!
     */
    B_Mutex_Unlock( hUdpStreamer->hStateMutex );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Finished Processing UDP State for hUdpStreamer %p: state %s, before issuing the callbacks with completionStatus 0x%x"
            BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state), completionStatus ));

    /* Tell ARB to do any deferred work. */
    brc = BIP_Arb_DoDeferred( hUdpStreamer->startApi.hArb, threadOrigin );
    BDBG_ASSERT( brc == BIP_SUCCESS );


    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--- hUdpStreamer %p: state %s"
                BIP_MSG_PRE_ARG, (void *)hUdpStreamer, BIP_UDP_STREAMER_STATE(hUdpStreamer->state)));
    return;
} /* processUdpStreamerState */
