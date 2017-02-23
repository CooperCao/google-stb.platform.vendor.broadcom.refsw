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
#include "blst_queue.h"
#include "bip.h"

#include "nexus_platform.h"
#include "nexus_frontend.h"
#include "nexus_parser_band.h"

BDBG_MODULE( http_recpump_server );

#define HTTP_SERVER_PORT_STRING "80"
#define HTTP_SERVER_INFO_DIRECTORY_PATH  "/data/info"
#define MAX_TRACKED_EVENTS 2

/* Values for spiderman_aes.ts */
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define AUDIO_PID 0x14

static const char source_fname[] = "videos/spiderman_aes.ts";
#define CHUNK_SIZE        (188 * 128)
#define CHUNK_COUNT       (128)

enum {
    PID_INDEX_eVideo,
    PID_INDEX_eAudio,
    PID_INDEX_eSubtitle,
    PID_INDEX_ePAT,
    PID_INDEX_ePMT,
    PID_INDEX_eMax
};

typedef struct AppCtx
{
    BIP_MediaInfoHandle         hMediaInfo;
    BIP_StringHandle            hPort;
    BIP_StringHandle            hInfoDirectoryPath;
    BIP_StringHandle            hStreamName;
    bool                        httpServerStarted;
    BIP_HttpServerHandle        hHttpServer;
    B_EventGroupHandle          hHttpEventGroup;
    B_EventHandle               hHttpRequestRcvdEvent;
    B_EventHandle               hHttpStreamerEvent;
    int                         maxTriggeredEvents;
    bool                        enableDtcpIp;
    bool                        slaveModeEnabled;
    BLST_Q_HEAD( streamerListHead, AppStreamerCtx ) streamerListHead; /* List of Streamer Ctx */
} AppCtx;

typedef struct AppStreamerCtx
{
    AppCtx                      *pAppCtx;
    bool                        endOfStreamerRcvd;  /* We have received End of Streaming event for this Streamer. */
    const char                  *pUrlPath;          /* Cached URL */
    const char                  *pMethodName;       /* HTTP Request Method Name */
    BIP_HttpStreamerHandle      hHttpStreamer;      /* Cached Streamer handle */
    BLST_Q_ENTRY( AppStreamerCtx ) streamerListNext;    /* Next Streamer Ctx */
    NEXUS_ParserBand            parserBand;
    NEXUS_FrontendHandle        hFrontend;
    NEXUS_RecpumpHandle         hRecpump;
    NEXUS_PlaypumpHandle        hPlaypump;
    NEXUS_PlaybackHandle        hPlayback;
    NEXUS_FilePlayHandle        hPlayfile;
    NEXUS_PidChannelHandle      pidChannel[PID_INDEX_eMax];
    BIP_HttpSocketHandle        hHttpSocket;
    NEXUS_KeySlotHandle         decCaHandle;
    NEXUS_PidChannelStatus      pidStatus;
} AppStreamerCtx;

int prepare_keyslots(int operation, NEXUS_KeySlotHandle *pVideoCaKeySlotHandle, uint32_t exportTaId, bool bIsolation);
void clean_keyslots(NEXUS_KeySlotHandle videoCaKeySlotHandle);

#define USER_INPUT_BUF_SIZE 64
/* function to allow proper exit of server */
bool exitThread( void )
{
    #if 0
    char    buffer[USER_INPUT_BUF_SIZE];
    size_t  bytesRead;

    BIP_Fd_ReadWithTimeout(STDIN_FILENO, sizeof(buffer)-1, 0 /*timeout*/, buffer, &bytesRead );
    buffer[bytesRead] = '\0';   /* Null-terminate whatever we read. */

    if (strstr(buffer, "q"))
    {
        BDBG_LOG((BIP_MSG_PRE_FMT "Received quit " BIP_MSG_PRE_ARG));
        return true;
    }
    #endif
    return false;
}
static void endOfStreamCallbackFromBip(
    void *context,
    int   param
    )
{
    AppStreamerCtx *pAppStreamerCtx = context;

    BSTD_UNUSED( param );

    BDBG_MSG(( BIP_MSG_PRE_FMT " B_Event_Set( pAppCtx->hHttpStreamerEvent )" BIP_MSG_PRE_ARG ));
    pAppStreamerCtx->endOfStreamerRcvd = true;
    B_Event_Set( pAppStreamerCtx->pAppCtx->hHttpStreamerEvent );
}

void stopAndDestroyStreamer(
    AppStreamerCtx *pAppStreamerCtx
    )
{
    if (!pAppStreamerCtx->hHttpStreamer)
        return;

    clean_keyslots(pAppStreamerCtx->decCaHandle);
    NEXUS_FilePlay_Close(pAppStreamerCtx->hPlayfile);
    NEXUS_Playback_Destroy(pAppStreamerCtx->hPlayback);
    NEXUS_Playpump_Close(pAppStreamerCtx->hPlaypump);


    BDBG_MSG(( BIP_MSG_PRE_FMT " Stopping Streamer %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer));
    BIP_HttpServer_StopStreamer( pAppStreamerCtx->pAppCtx->hHttpServer, pAppStreamerCtx->hHttpStreamer );

    /* TODO: Release resources */

    BDBG_MSG(( BIP_MSG_PRE_FMT " Destroying Streamer %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer));
    BIP_HttpServer_DestroyStreamer( pAppStreamerCtx->pAppCtx->hHttpServer,  pAppStreamerCtx->hHttpStreamer );

    return;
} /* stopAndDestroyStreamer */

/* Function to process HttpStreamer related events */
static void processHttpStreamerEvent(
    AppCtx *pAppCtx
    )
{
    AppStreamerCtx          *pAppStreamerCtx;
    AppStreamerCtx          *pAppStreamerCtxNext;

    pAppStreamerCtx = BLST_Q_FIRST(&pAppCtx->streamerListHead);
    while ( pAppStreamerCtx )
    {
        if (pAppStreamerCtx->endOfStreamerRcvd)
        {
            /* BIP has indicated end of streaming, so cleanup! */
            pAppStreamerCtxNext = BLST_Q_NEXT( pAppStreamerCtx, streamerListNext );
            BLST_Q_REMOVE( &pAppStreamerCtx->pAppCtx->streamerListHead, pAppStreamerCtx, streamerListNext );
            stopAndDestroyStreamer( pAppStreamerCtx);
            B_Os_Free( pAppStreamerCtx );
            pAppStreamerCtx = pAppStreamerCtxNext;
        }
        else
        {
            pAppStreamerCtx = BLST_Q_NEXT( pAppStreamerCtx, streamerListNext );
        }
    }
} /* processHttpStreamerEvent */

static BIP_Status rejectRequestAndSetResponseHeaders(
    AppCtx *pAppCtx,
    BIP_HttpRequestHandle    hHttpRequest,
    BIP_HttpResponseStatus   responseStatus
    )
{
    BIP_Status bipStatus;
    BIP_HttpServerRejectRequestSettings rejectSettings;

    BIP_HttpServer_GetDefaultRejectRequestSettings(&rejectSettings);
    rejectSettings.httpStatus = responseStatus;
    rejectSettings.customResponseHeaders = "Custom Header1\r\nCustomHeader2\r\n";
    bipStatus = BIP_HttpServer_RejectRequest( pAppCtx->hHttpServer, hHttpRequest, &rejectSettings );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpServer_RejectRequest Failed" ), error, bipStatus, bipStatus );

    BDBG_ERR(( BIP_MSG_PRE_FMT "BIP_HttpServer_RejectRequest() for hHttpRequest %p" BIP_MSG_PRE_ARG, (void *)hHttpRequest));

error:
    return (bipStatus);
}

static BIP_Status addTrackToRecpump(
    AppStreamerCtx *pAppStreamerCtx,
    NEXUS_PidChannelHandle hPidChannel
    )
{
    NEXUS_Error nrc;
    BIP_Status bipStatus;
#if 0
    hPidChannel = NEXUS_PidChannel_Open( pAppStreamerCtx->parserBand, pid, NULL);
    BIP_CHECK_GOTO(( hPidChannel ), ( "NEXUS_PidChannel_Open Failed!" ), error, BIP_ERR_NEXUS, bipStatus );
 #endif
    nrc = NEXUS_Recpump_AddPidChannel( pAppStreamerCtx->hRecpump, hPidChannel, NULL );
    BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Recpump_AddPidChannel() Failed, nrc %d", nrc), error, BIP_ERR_NEXUS, bipStatus );

    bipStatus = BIP_SUCCESS;
error:
    return ( bipStatus );
} /* addTrackToRecpump */

int openRecpump(
    AppStreamerCtx *pAppStreamerCtx
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_RecpumpSettings recpumpSettings;

    NEXUS_Platform_GetConfiguration(&platformConfig);
    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    BDBG_MSG((BIP_MSG_PRE_FMT "atomSize %d, dataReadyThreshold %d" BIP_MSG_PRE_ARG, recpumpOpenSettings.data.atomSize, recpumpOpenSettings.data.dataReadyThreshold));

    recpumpOpenSettings.data.alignment = 12;
    recpumpOpenSettings.data.atomSize = CHUNK_SIZE;
    recpumpOpenSettings.data.bufferSize = (CHUNK_COUNT * CHUNK_SIZE);
    recpumpOpenSettings.data.dataReadyThreshold = CHUNK_SIZE;
    #if 1
    recpumpOpenSettings.data.heap = platformConfig.heap[NEXUS_EXPORT_HEAP];
    recpumpOpenSettings.useSecureHeap = false;
    #endif
    /* set threshold to 80%. with band hold enabled, it's not actually a dataready threshold. it's
       a bandhold threshold. we are relying on the timer that's already in record. */
    recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.bufferSize * 5 / 10;
    recpumpOpenSettings.index.dataReadyThreshold = recpumpOpenSettings.index.bufferSize * 5 / 10;

    pAppStreamerCtx->hRecpump = NEXUS_Recpump_Open( NEXUS_ANY_ID, &recpumpOpenSettings );
    BIP_CHECK_GOTO(( pAppStreamerCtx->hRecpump ), ( "NEXUS_Recpump_Open Failed!" ), error, BIP_ERR_NEXUS, bipStatus );

    NEXUS_Recpump_GetSettings(pAppStreamerCtx->hRecpump, &recpumpSettings);
    /* enable bandhold. required for record from playback. */
    recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
    NEXUS_Recpump_SetSettings(pAppStreamerCtx->hRecpump, &recpumpSettings);
error:
    return bipStatus;
}

static BIP_Status startStreamer(
    AppStreamerCtx *pAppStreamerCtx,
    BIP_HttpRequestHandle hHttpRequest
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_HttpResponseStatus responseStatus;
    NEXUS_RecpumpStatus status;
    uint32_t exportTaId = 0; /*Dont need*/
    bool bIsolation = false; /* isolation is off */


    /* Create HttpStreamer. */
    {
        BIP_HttpServerCreateStreamerSettings createStreamerSettings;

        BIP_HttpServer_GetDefaultCreateStreamerSettings( &createStreamerSettings );
        createStreamerSettings.endOfStreamingCallback.callback = endOfStreamCallbackFromBip;
        createStreamerSettings.endOfStreamingCallback.context = pAppStreamerCtx;
        pAppStreamerCtx->hHttpStreamer = BIP_HttpServer_CreateStreamer( pAppStreamerCtx->pAppCtx->hHttpServer, &createStreamerSettings );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( pAppStreamerCtx->hHttpStreamer ), ( "BIP_HttpServer_CreateStreamer Failed" ), rejectRequest, BIP_ERR_CREATE_FAILED, responseStatus );
        BDBG_MSG(( BIP_MSG_PRE_FMT " BIP_HttpStreamer created: %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer ));
    }

    /* Now provide settings for media input source */
    {
        BIP_StreamerRecpumpInputSettings recpumpInputSettings;

        NEXUS_PlaybackSettings playbackSettings;

        pAppStreamerCtx->hPlaypump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
        BDBG_ASSERT(pAppStreamerCtx->hPlaypump);
        BDBG_MSG(( BIP_MSG_PRE_FMT " Playpump Open : %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hPlaypump ));

        pAppStreamerCtx->hPlayback = NEXUS_Playback_Create();
        BDBG_ASSERT(pAppStreamerCtx->hPlayback);
        BDBG_MSG(( BIP_MSG_PRE_FMT " Playback Created : %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hPlayback ));

        pAppStreamerCtx->hPlayfile = NEXUS_FilePlay_OpenPosix(source_fname, NULL);
        BDBG_ASSERT(pAppStreamerCtx->hPlayfile);
        BDBG_MSG(( BIP_MSG_PRE_FMT " Playfile %s Open  : %p" BIP_MSG_PRE_ARG, source_fname, (void *)pAppStreamerCtx->hPlayfile ));

        NEXUS_Playback_GetSettings(pAppStreamerCtx->hPlayback, &playbackSettings);
        playbackSettings.playpump = pAppStreamerCtx->hPlaypump;
        playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
        playbackSettings.playpumpSettings.acceptNullPackets = true;

        playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
        NEXUS_Playback_SetSettings(pAppStreamerCtx->hPlayback, &playbackSettings);

    /* Prepare CA Keyslot */
        bipStatus = prepare_keyslots(0,
                     &pAppStreamerCtx->decCaHandle,
                     exportTaId,
                     bIsolation);
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "prepareCaKeyslot Failed" ), rejectRequest, bipStatus, responseStatus );

        /* Open Recpump */
        bipStatus = openRecpump( pAppStreamerCtx );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "openRecpump Failed" ), rejectRequest, bipStatus, responseStatus );

        BIP_Streamer_GetDefaultRecpumpInputSettings(&recpumpInputSettings);
        bipStatus = BIP_HttpStreamer_SetRecpumpInputSettings( pAppStreamerCtx->hHttpStreamer, pAppStreamerCtx->hRecpump, &recpumpInputSettings );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_SetRecpumpInputSettings Failed" ), rejectRequest, bipStatus, responseStatus );
    }

            /* Open Audio and Video PIDs */
    {
        NEXUS_PlaybackPidChannelSettings playbackPidSettings;

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        pAppStreamerCtx->pidChannel[0] = NEXUS_Playback_OpenPidChannel(pAppStreamerCtx->hPlayback, VIDEO_PID, &playbackPidSettings);
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        pAppStreamerCtx->pidChannel[1] = NEXUS_Playback_OpenPidChannel(pAppStreamerCtx->hPlayback, AUDIO_PID, &playbackPidSettings);
    }
    bipStatus = addTrackToRecpump( pAppStreamerCtx, pAppStreamerCtx->pidChannel[0] );
    bipStatus = addTrackToRecpump( pAppStreamerCtx, pAppStreamerCtx->pidChannel[1] );
    /* In case it is a CA-encrypted stream, attach Video and Audio PIDs to CA keyslots */
    NEXUS_PidChannel_GetStatus(pAppStreamerCtx->pidChannel[0], &pAppStreamerCtx->pidStatus);
    NEXUS_Security_AddPidChannelToKeySlot(pAppStreamerCtx->decCaHandle, pAppStreamerCtx->pidStatus.pidChannelIndex);
    NEXUS_PidChannel_GetStatus(pAppStreamerCtx->pidChannel[1], &pAppStreamerCtx->pidStatus);
    NEXUS_Security_AddPidChannelToKeySlot(pAppStreamerCtx->decCaHandle, pAppStreamerCtx->pidStatus.pidChannelIndex);

    /* Provide the Streamer output settings */
    {
        BIP_HttpStreamerOutputSettings streamerOutputSettings;

        BIP_HttpStreamer_GetDefaultOutputSettings( &streamerOutputSettings );
        if (pAppStreamerCtx->pAppCtx->enableDtcpIp)
        {
            streamerOutputSettings.enableDtcpIp = true;
            streamerOutputSettings.dtcpIpOutput.pcpPayloadLengthInBytes = (1024*1024);
            streamerOutputSettings.dtcpIpOutput.akeTimeoutInMs = 2000;
            streamerOutputSettings.dtcpIpOutput.copyControlInfo = B_CCI_eCopyNever;
        BDBG_WRN(("Enabling DTCP-IP"));
        }

        bipStatus = BIP_HttpStreamer_SetOutputSettings(pAppStreamerCtx->hHttpStreamer, BIP_HttpStreamerProtocol_eDirect, &streamerOutputSettings);
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_SetOutputSettings Failed" ), rejectRequest, bipStatus, responseStatus );
    }

    /* At this point, we have set sucessfully configured the Input & Output settings of the HttpStreamer. */

    /* Now Set any custom or app specific HTTP Headers that HttpStreamer should include when it sends out the Response Message. */
    {
        bipStatus = BIP_HttpStreamer_SetResponseHeader(
                pAppStreamerCtx->hHttpStreamer,
                "transferMode.dlna.org",
                "Streaming"
                );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_HTTP_MESSAGE_HEADER_ALREADY_SET),
                ( "BIP_HttpStreamer_SetResponseHeader Failed: hHttpStreamer %p", (void *)pAppStreamerCtx->hHttpStreamer ), rejectRequest, bipStatus, responseStatus );
        if (pAppStreamerCtx->pAppCtx->enableDtcpIp)
        {
            bipStatus = BIP_HttpStreamer_SetResponseHeader(
                    pAppStreamerCtx->hHttpStreamer,
                    "contentFeatures.dlna.org",
                    ";DLNA.ORG_OP=11;DLNA.ORG_FLAGS=01210000000000000000000000000000;DLNA.ORG_PS=-48,-36,-18,-12,-6,6,12,18,36,48"
            /*"DLNA.ORG_FLAGS=00008000000000000000000000000000"*/
                    );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_HTTP_MESSAGE_HEADER_ALREADY_SET),
                    ( "BIP_HttpStreamer_SetResponseHeader Failed: hHttpStreamer %p", (void *)pAppStreamerCtx->hHttpStreamer ), rejectRequest, bipStatus, responseStatus );
            {
                BIP_StringHandle hContentTypeValueBase = NULL;
                BIP_HttpHeaderHandle hHeader;
                BIP_HttpSocketStatus httpSocketStatus;
                BIP_DtcpIpServerStatus dtcpIpServerStatus;

                /* For DTCP/IP Content-Type header value looks like this: */
                /* Content-Type: application/x-dtcp1;DTCP1HOST=192.168.1.121;DTCP1PORT=5000;CONTENTFORMAT=video/vnd.dlna.mpeg-tts */

                /* Get the IP Address associated with the local DTCP/IP Authentication (AKE) Server. */
                bipStatus = BIP_HttpSocket_GetStatus( pAppStreamerCtx->hHttpSocket, &httpSocketStatus );
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpSocket_GetStatus Failed" ), rejectRequest, bipStatus, bipStatus );

                hContentTypeValueBase = BIP_String_CreateFromPrintf("application/x-dtcp1;DTCP1HOST=%s;DTCP1PORT=%s;",
                        httpSocketStatus.pLocalIpAddress,
                        "8000"
                        );
                BIP_CHECK_GOTO(( hContentTypeValueBase ), ( "BIP_String_CreateFromPrintf Failed" ), rejectRequest, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
                BDBG_MSG(( BIP_MSG_PRE_FMT "local Ip: %s, port: 8000" BIP_MSG_PRE_ARG, httpSocketStatus.pLocalIpAddress));

                /* Add content type header. */
                bipStatus = BIP_HttpStreamer_SetResponseHeader(
                        pAppStreamerCtx->hHttpStreamer,
                        "Content-Type",
                        BIP_String_GetString(hContentTypeValueBase)
                        );
                if ( hContentTypeValueBase ) BIP_String_Destroy( hContentTypeValueBase );
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_HTTP_MESSAGE_HEADER_ALREADY_SET),
                        ( "BIP_HttpStreamer_SetResponseHeader Failed: hHttpStreamer %p", (void *)pAppStreamerCtx->hHttpStreamer ), rejectRequest, bipStatus, responseStatus );
            }
        }

        /* App can add more custom headers here using this example above! */
    }

    NEXUS_Playback_Start(pAppStreamerCtx->hPlayback, pAppStreamerCtx->hPlayfile, NULL);
    /* And as a last step, start the HttpStreamer. */
    {
        BIP_HttpServerStartStreamerSettings startSettings;

        BIP_HttpServer_GetDefaultStartStreamerSettings( &startSettings );
        startSettings.streamerStartSettings.inactivityTimeoutInMs = 50000;
        bipStatus = BIP_HttpServer_StartStreamer( pAppStreamerCtx->pAppCtx->hHttpServer, pAppStreamerCtx->hHttpStreamer, hHttpRequest, &startSettings );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpServer_StartStreamer Failed: hHttpStreamer %p", (void *)pAppStreamerCtx->hHttpStreamer ), rejectRequest, bipStatus, responseStatus );
    }
    NEXUS_Recpump_GetStatus(pAppStreamerCtx->hRecpump, &status);
    if (status.started == true)
   {
       BDBG_MSG(( BIP_MSG_PRE_FMT " Recpump started : %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hRecpump ));
   }
   else
       BDBG_MSG(( BIP_MSG_PRE_FMT " Recpump not started yet : %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hRecpump ));

    return (bipStatus);

rejectRequest:
    /* This label handles the error cases when app encounters an error before BIP_HttpServer_StartStreamer(). */
    /* Thus it needs to reject the current hHttpRequest. */
    rejectRequestAndSetResponseHeaders( pAppStreamerCtx->pAppCtx, hHttpRequest, responseStatus );

    if (pAppStreamerCtx->hHttpStreamer)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT " Destroying Streamer %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer));
        /* Note: App doesn't need to destroy the streamer & can maintain a free-list of them. */
        BIP_HttpServer_DestroyStreamer( pAppStreamerCtx->pAppCtx->hHttpServer,  pAppStreamerCtx->hHttpStreamer );
        pAppStreamerCtx->hHttpStreamer = NULL;
    }

    /*
     * This label handles the error cases when app encounters an error during BIP_HttpServer_StartStreamer().
     * In that case, BIP_HttpServer_StartStreamer() has internally send out the error response.
     * Caller will cleanup the AppStreamerCtx.
     */
    return (bipStatus);
} /* startStreamer */

/* Function to process incoming Requests */
static void processHttpRequestEvent(
    AppCtx *pAppCtx
    )
{
    BIP_Status               bipStatus = BIP_SUCCESS;
    BIP_HttpRequestMethod    method;
    AppStreamerCtx           *pAppStreamerCtx = NULL;
    BIP_HttpRequestHandle    hHttpRequest;
    BIP_HttpSocketHandle     hHttpSocket;
    BIP_HttpResponseStatus   responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
    BIP_HttpServerRecvRequestSettings recvReqSettings;

    while (true)
    {
        hHttpRequest = NULL;
        /* Check if there is a HTTP Request available for processing! */
        {
            BIP_HttpServer_GetDefaultRecvRequestSettings(&recvReqSettings);
            recvReqSettings.phHttpSocket = &hHttpSocket;
            bipStatus = BIP_HttpServer_RecvRequest( pAppCtx->hHttpServer, &hHttpRequest, &recvReqSettings );
            if ( bipStatus == BIP_INF_TIMEOUT )
            {
                /* No request available at this time, return! */
                BDBG_MSG(( "No HTTPRequest is currently available, we are done for now! " ));
                return;
            }
            /* Make sure it is not an error while receiving the request! */
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpServer_RecvRequest Failed: bipStatus 0x%x", bipStatus ), error, bipStatus, bipStatus );
        }

        /* Successfully Received a Request, create Streamer specific app context */
        {
            pAppStreamerCtx = B_Os_Calloc( 1, sizeof( AppStreamerCtx ));
            responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
            BIP_CHECK_GOTO(( pAppStreamerCtx ), ( "Memory Allocation Failed" ), rejectRequest, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

            pAppStreamerCtx->pAppCtx = pAppCtx;
            pAppStreamerCtx->hHttpSocket = hHttpSocket;
            BDBG_MSG(( BIP_MSG_PRE_FMT "AppStreamerCtx %p: Setting up Streamer" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx));
        }

        /* Process the received Request, start w/ the Method */
        {
            /* Note: app may retrieve various HTTP Headers from the Request using BIP_HttpRequest_GetHeader. */
            bipStatus = BIP_HttpRequest_GetMethod( hHttpRequest, &method, &pAppStreamerCtx->pMethodName );
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetMethod Failed" ), rejectRequest, BIP_ERR_INVALID_REQUEST_TARGET, bipStatus );

            /* Validate the Request Method! */
            responseStatus = BIP_HttpResponseStatus_e501_NotImplemented;
            BIP_CHECK_GOTO( (method == BIP_HttpRequestMethod_eHead || method == BIP_HttpRequestMethod_eGet),
                    ( BIP_MSG_PRE_FMT " ERROR: method (%s) is not recognized/supported, only HEAD & GET are supported!" BIP_MSG_PRE_ARG, pAppStreamerCtx->pMethodName ),
                    rejectRequest, BIP_ERR_INVALID_REQUEST_TARGET, bipStatus );

            /* Retrieve the requested URL */
            bipStatus = BIP_HttpRequest_GetTarget( hHttpRequest, &pAppStreamerCtx->pUrlPath );
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetTarget Failed" ), rejectRequest, BIP_ERR_INVALID_REQUEST_TARGET, bipStatus );
        }

        /* All is well, so Start Streamer */
        {
            bipStatus = startStreamer( pAppStreamerCtx, hHttpRequest );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "startStreamer Failed" ), error, bipStatus, responseStatus );
            BLST_Q_INSERT_TAIL( &pAppCtx->streamerListHead, pAppStreamerCtx, streamerListNext );

            BDBG_MSG(( BIP_MSG_PRE_FMT "AppStreamerCtx %p: Streaming is started!" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx));
        }
        continue;

rejectRequest:
        /* Some error happened, so reject the current hHttpRequest. */
        rejectRequestAndSetResponseHeaders( pAppStreamerCtx->pAppCtx, hHttpRequest, responseStatus );
        if (pAppStreamerCtx)
        {
            B_Os_Free(pAppStreamerCtx);
            pAppStreamerCtx = NULL;
        }
        /* continue back to the top of the loop. */
    } /* while */

error:
    if (pAppStreamerCtx)
    {
        if (pAppStreamerCtx->hHttpStreamer) stopAndDestroyStreamer( pAppStreamerCtx );
        B_Os_Free(pAppStreamerCtx);
        pAppStreamerCtx = NULL;
    }
    BDBG_ERR(( BIP_MSG_PRE_FMT " Error case: DONE" BIP_MSG_PRE_ARG ));
    return;
} /* processHttpRequestEvent */

/* Callback from BIP_Server: indicates that one or more HttpRequest are ready for app to receive! */
static void requestReceivedCallbackFromBip(
    void *context,
    int   param
    )
{
    AppCtx *pAppCtx = context;
    BSTD_UNUSED( param );

    BDBG_MSG(( BIP_MSG_PRE_FMT " B_Event_Set( pAppCtx->hHttpRequestRcvdEvent )" BIP_MSG_PRE_ARG ));
    B_Event_Set( pAppCtx->hHttpRequestRcvdEvent );
} /* requestReceivedCallbackFromBip */

/* Scan Media Directory and generate MediaInfo for each file. */
static void unInitHttpServer(
    AppCtx *pAppCtx
    )
{
    AppStreamerCtx *pAppStreamerCtx;

    if (!pAppCtx)
        return;
    BDBG_MSG(( BIP_MSG_PRE_FMT " appCtx %p" BIP_MSG_PRE_ARG, (void *)pAppCtx ));

    if (pAppCtx->httpServerStarted)
        BIP_HttpServer_Stop( pAppCtx->hHttpServer );
    pAppCtx->httpServerStarted = false;

    /* Now Stop & Destroy any currently active streamers! */
    for (
         pAppStreamerCtx = BLST_Q_FIRST(&pAppCtx->streamerListHead);
         pAppStreamerCtx;
         pAppStreamerCtx = BLST_Q_FIRST(&pAppCtx->streamerListHead)
        )
    {
        BLST_Q_REMOVE( &pAppStreamerCtx->pAppCtx->streamerListHead, pAppStreamerCtx, streamerListNext );
        stopAndDestroyStreamer( pAppStreamerCtx );
        B_Os_Free( pAppStreamerCtx );
    }

    if (pAppCtx->hHttpServer)
        BIP_HttpServer_Destroy( pAppCtx->hHttpServer );
    pAppCtx->hHttpServer = NULL;

    if (pAppCtx->hMediaInfo) { BIP_MediaInfo_Destroy(pAppCtx->hMediaInfo); }
    if (pAppCtx->hHttpRequestRcvdEvent) {B_EventGroup_RemoveEvent( pAppCtx->hHttpEventGroup, pAppCtx->hHttpRequestRcvdEvent ); }
    if (pAppCtx->hHttpStreamerEvent) {B_EventGroup_RemoveEvent( pAppCtx->hHttpEventGroup, pAppCtx->hHttpStreamerEvent ); }
    if (pAppCtx->hHttpRequestRcvdEvent) {B_Event_Destroy( pAppCtx->hHttpRequestRcvdEvent ); }
    if (pAppCtx->hHttpStreamerEvent) {B_Event_Destroy( pAppCtx->hHttpStreamerEvent ); }
    if (pAppCtx->hHttpEventGroup) {B_EventGroup_Destroy( pAppCtx->hHttpEventGroup ); }

} /* unInitHttpServer */

/* Initialize HttpServer */
static BIP_Status initHttpServer(
    AppCtx *pAppCtx
    )
{
    B_Error rc = B_ERROR_UNKNOWN;
    BIP_Status bipStatus = BIP_ERR_INTERNAL;

    /* Create events for callbacks */
    {
        /* Create Event group */
        pAppCtx->hHttpEventGroup = B_EventGroup_Create( NULL );
        BIP_CHECK_GOTO(( pAppCtx->hHttpEventGroup ), ( "Event Group Creation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        /* Create an event to indicate a callback on the listener */
        pAppCtx->hHttpRequestRcvdEvent = B_Event_Create( NULL );
        BIP_CHECK_GOTO(( pAppCtx->hHttpRequestRcvdEvent ), ( "Event Creation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
        pAppCtx->maxTriggeredEvents++;
        /* Add this event to the event group */
        rc = B_EventGroup_AddEvent( pAppCtx->hHttpEventGroup, pAppCtx->hHttpRequestRcvdEvent );
        BIP_CHECK_GOTO(( !rc ), ( "Failed to Add event to group" ), error, BIP_ERR_INTERNAL, bipStatus );

        /* Create an event to indicate a callback on the session */
        pAppCtx->hHttpStreamerEvent = B_Event_Create( NULL );
        BIP_CHECK_GOTO(( pAppCtx->hHttpStreamerEvent ), ( "Event Creation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
        pAppCtx->maxTriggeredEvents++;
        /* Add this event to the event group */
        rc = B_EventGroup_AddEvent( pAppCtx->hHttpEventGroup, pAppCtx->hHttpStreamerEvent );
        BIP_CHECK_GOTO(( !rc ), ( "Failed to Add event to group" ), error, BIP_ERR_INTERNAL, bipStatus );

        BIP_CHECK_GOTO( !( pAppCtx->maxTriggeredEvents > MAX_TRACKED_EVENTS ), ( "need to increase the MAX_TRACKED_EVENTS to %d", pAppCtx->maxTriggeredEvents ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    /* Create HTTP Server */
    {
        pAppCtx->hHttpServer = BIP_HttpServer_Create( NULL ); /* Use default settings. */
        BIP_CHECK_GOTO(( pAppCtx->hHttpServer ), ( "BIP_HttpServer_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
    }

    /* Set Server settings: Listening port & Callbacks */
    {
        BIP_HttpServerSettings httpServerSettings;
        BIP_HttpServer_GetSettings( pAppCtx->hHttpServer, &httpServerSettings );
        httpServerSettings.requestReceivedCallback.callback = requestReceivedCallbackFromBip;
        httpServerSettings.requestReceivedCallback.context = pAppCtx;
        bipStatus = BIP_HttpServer_SetSettings( pAppCtx->hHttpServer, &httpServerSettings );
        BIP_CHECK_GOTO(( !bipStatus ), ( "BIP_HttpServer_SetSettings Failed" ), error, bipStatus, bipStatus );
    }

    /* Start the Server */
    {
        BIP_HttpServerStartSettings httpServerStartSettings;

        BIP_HttpServer_GetDefaultStartSettings(&httpServerStartSettings);
        BDBG_MSG(("%s: Starting HttpServer...", __FUNCTION__));
        httpServerStartSettings.pPort = BIP_String_GetString( pAppCtx->hPort );
        if (pAppCtx->enableDtcpIp)
        {
            httpServerStartSettings.enableDtcpIp = true;
            httpServerStartSettings.dtcpIpServer.pAkePort = "8000";
        }
        bipStatus = BIP_HttpServer_Start( pAppCtx->hHttpServer, &httpServerStartSettings );
        BIP_CHECK_GOTO(( !bipStatus ), ( "BIP_HttpServer_Start Failed" ), error, bipStatus, bipStatus );
    }

    pAppCtx->httpServerStarted = true;
    bipStatus = BIP_SUCCESS;

error:
    return ( bipStatus );
} /* initHttpServer */

void printUsage( char *pCmdName )
{
    printf( "Usage: %s\n", pCmdName );
    printf(
        "  --help or -h for help \n"
        "  -port        #   Server Port (default is port 80 \n"
        "  -infoDir     #   Info Files Directory Path (default is /data/info) \n"
        "  -freq        #   frequency in MHz (default is 549 Mhz) \n"
        "  -mode        #   64 = QAM-64, 256 = QAM-256 (default is QAM-256) \n"
        "  -dtcpIp      #   Start DTCP/IP Server \n"
        "  -slave       #   Start Server in slave mode (Client in Nexus Multi-Process) \n"
        );
}

BIP_Status parseOptions(
    int    argc,
    char   *argv[],
    AppCtx *pAppCtx
    )
{
    int i;
    BIP_Status bipStatus = BIP_ERR_INTERNAL;

    for (i=1; i<argc; i++)
    {
        if ( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") )
        {
            printUsage(argv[0]);
            exit(0);
        }
        else if ( !strcmp(argv[i], "-port") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hPort, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-infoDir") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hInfoDirectoryPath, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-freq") && i+1<argc )
        {
        }
        else if ( !strcmp(argv[i], "-mode") && i+1<argc )
        {
        }
        else if ( !strcmp(argv[i], "-dtcpIp") )
        {
            pAppCtx->enableDtcpIp = true;
        }
        else if ( !strcmp(argv[i], "-slave") )
        {
            pAppCtx->slaveModeEnabled = true;
        }
        else
        {
            printf("Error: incorrect or unsupported option: %s\n", argv[i]);
            printUsage(argv[0]);
            exit(0);
        }
    }
    bipStatus = BIP_SUCCESS;
    BDBG_LOG(( BIP_MSG_PRE_FMT " port %s, infoDir %s" BIP_MSG_PRE_ARG,
                BIP_String_GetString( pAppCtx->hPort ),
                BIP_String_GetString( pAppCtx->hInfoDirectoryPath )
             ));
    BDBG_LOG(( "**********THIS IS A SPECIAL EXAMPLE TO TEST SPECIFICALLY [CA input ---CA decrypt---> XRR ---DTCP encrypt---> GLR --> N/W STREAM] ***********\n"
           "**********THIS EXAMPLE ASSUMES PREENCRYPTED CA FILE AT THIS LOCATION [/data/videos/spiderman_aes.ts]************\n"
           "**********THIS EXAMPLE WILL WORK ONLY WITH SAGE DTCP IP ENABLED. PLEASE export DTCP_IP_SAGE_SUPPORT=y before building.*******************" ));
    return ( bipStatus );
} /* parseOptions */

void unInitAppCtx(
    AppCtx *pAppCtx
    )
{
    if (!pAppCtx) return;
    if (pAppCtx->hStreamName) BIP_String_Destroy( pAppCtx->hStreamName);
    if (pAppCtx->hPort) BIP_String_Destroy( pAppCtx->hPort);
    if (pAppCtx->hInfoDirectoryPath) BIP_String_Destroy( pAppCtx->hInfoDirectoryPath);
    if (pAppCtx) B_Os_Free( pAppCtx );
} /* unInitAppCtx */

AppCtx *initAppCtx( void )
{
    AppCtx *pAppCtx = NULL;
    BIP_Status bipStatus = BIP_ERR_INTERNAL;

    pAppCtx = B_Os_Calloc( 1, sizeof( AppCtx ));
    BIP_CHECK_GOTO(( pAppCtx ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    /* Setup default values for global App settings */
    pAppCtx->hPort = BIP_String_CreateFromChar( HTTP_SERVER_PORT_STRING );
    BIP_CHECK_GOTO( (pAppCtx->hPort), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    pAppCtx->hInfoDirectoryPath = BIP_String_CreateFromChar( HTTP_SERVER_INFO_DIRECTORY_PATH );
    BIP_CHECK_GOTO( (pAppCtx->hInfoDirectoryPath), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

error:
    return pAppCtx;
} /* initAppCtx */

int main(
    int    argc,
    char *argv[]
    )
{
    BIP_Status bipStatus;
    NEXUS_Error nrc;
    AppCtx *pAppCtx = NULL;

    /* Initialize BIP */
    bipStatus = BIP_Init(NULL);
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Init Failed" ), errorBipInit, bipStatus, bipStatus );

    /* Allocate & initialize App Ctx */
    pAppCtx = initAppCtx();
    BIP_CHECK_GOTO(( pAppCtx ), ( "initAppCtx Failed" ), errorBipInit, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    /* Parse command line options */
    bipStatus = parseOptions( argc, argv, pAppCtx );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "parseOptions Failed" ), errorParseOptions, bipStatus, bipStatus );

    /* Initialize NEXUS */
    {
        NEXUS_PlatformSettings platformSettings;

        NEXUS_PlatformStartServerSettings serverSettings;
        NEXUS_PlatformConfiguration platformConfig;

        NEXUS_Platform_GetDefaultSettings(&platformSettings);
        platformSettings.mode = NEXUS_ClientMode_eVerified;
        platformSettings.openFrontend = false;

    /* Configure export heap since it's not allocated by nexus by default */
    platformSettings.heap[NEXUS_EXPORT_HEAP].size = 32*1024*1024;

        nrc = NEXUS_Platform_Init(&platformSettings);
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_Init Failed" ), error, BIP_ERR_INTERNAL, bipStatus );

        NEXUS_Platform_GetConfiguration(&platformConfig);

        NEXUS_Platform_GetDefaultStartServerSettings(&serverSettings);
        serverSettings.allowUnauthenticatedClients = true; /* client is written this way */
        serverSettings.unauthenticatedConfiguration.mode = NEXUS_ClientMode_eVerified;
        serverSettings.unauthenticatedConfiguration.heap[1] = platformConfig.heap[0]; /* for purposes of example, allow access to main heap */
        nrc = NEXUS_Platform_StartServer(&serverSettings);
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_StartServer Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    /* Initialize HttpServer */
    bipStatus = initHttpServer( pAppCtx );
    BIP_CHECK_GOTO(( !bipStatus ), ( "initHttpServer Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    /*
     * At this time, BIP has started listening for incoming requests from clients.
     * When new request comes from client, app gets the callback from BIP.
     * Callback function sends event to allow this main thread to process the events.
     */

    /* Wait on Event group and process events as they come */
    BDBG_LOG(( BIP_MSG_PRE_FMT " Type 'q' followed by ENTER to exit gracefully\n" BIP_MSG_PRE_ARG ));
    while (!exitThread())
    {
        unsigned      i;
        unsigned      numTriggeredEvents;
        B_EventHandle triggeredEvents[MAX_TRACKED_EVENTS];

        B_EventGroup_Wait( pAppCtx->hHttpEventGroup, 1000/*1sec*/, triggeredEvents, pAppCtx->maxTriggeredEvents, &numTriggeredEvents );
        for (i = 0; i < numTriggeredEvents; i++)
        {
            if (triggeredEvents[i] == pAppCtx->hHttpRequestRcvdEvent)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT " Process HttpRequestEvent[i = %d]" BIP_MSG_PRE_ARG, i));
                /* process all events on the HttpServer */
                processHttpRequestEvent( pAppCtx );
                /* Continue to process next event */
                continue;
            }
            if (triggeredEvents[i] == pAppCtx->hHttpStreamerEvent)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT " Process events on all HttpStreamers" BIP_MSG_PRE_ARG ));
                processHttpStreamerEvent( pAppCtx );
                continue;
            }
            BDBG_WRN(( BIP_MSG_PRE_FMT " didn't process event (i %d)" BIP_MSG_PRE_ARG, i ));
        }
    }

    BDBG_LOG(( BIP_MSG_PRE_FMT " Shutting HttpServer down..." BIP_MSG_PRE_ARG ));

error:
    unInitHttpServer( pAppCtx );
    if (!pAppCtx->slaveModeEnabled)
        NEXUS_Platform_StopServer();
    NEXUS_Platform_Uninit();
errorParseOptions:
    unInitAppCtx( pAppCtx );
errorBipInit:
    BIP_Uninit();

    BDBG_LOG(( "All done!" ));
    return (0); /* main */
}
