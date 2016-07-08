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
#include "blst_queue.h"
#include "bip.h"

#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_platform_client.h"
#else
#include "nexus_platform.h"
#endif

BDBG_MODULE( http_simple_streamer );

#define HTTP_SERVER_PORT_STRING          "80"
#define HTTP_SERVER_MEDIA_FILE           "/data/videos/AbcMpeg2HD.mpg"
#define HTTP_SERVER_INFO_DIRECTORY_PATH  "/data/info"
#define MAX_TRACKED_EVENTS 2

typedef struct AppCtx
{
    BIP_StringHandle            hPort;
    BIP_StringHandle            hMediaFileAbsolutePathName;
    BIP_StringHandle            hInfoFileAbsolutePathName;
    BIP_StringHandle            hInfoDirectoryPath;
    BIP_MediaInfoHandle         hMediaInfo;         /* MediaInfo object handle.*/
    bool                        httpServerStarted;
    BIP_HttpServerHandle        hHttpServer;
    B_EventGroupHandle          hHttpEventGroup;
    B_EventHandle               hHttpRequestRcvdEvent;
    B_EventHandle               hHttpStreamerEvent;
    int                         maxTriggeredEvents;
    BLST_Q_HEAD( streamerListHead, AppStreamerCtx ) streamerListHead; /* List of Streamer Ctx */
} AppCtx;

typedef struct AppStreamerCtx
{
    AppCtx                      *pAppCtx;
    bool                        endOfStreamerRcvd;  /* We have received End of Streaming event for this Streamer. */
    const char                  *pUrlPath;          /* Cached URL */
    BIP_HttpStreamerHandle      hHttpStreamer;      /* Cached Streamer handle */
    uint64_t                    rangeStartOffset;   /* Optional: starting offset of the Range Header. */
    bool                        rangeLengthValid;   /* Optional: if true , rangeLength is valid.*/
    uint64_t                    rangeLength;        /* Optional: range length from the starting offset. */
    BLST_Q_ENTRY( AppStreamerCtx ) streamerListNext;    /* Next Streamer Ctx */
} AppStreamerCtx;

#define USER_INPUT_BUF_SIZE 64
/* function to allow proper exit of server */
bool exitThread( void )
{
    char    buffer[USER_INPUT_BUF_SIZE];
    size_t  bytesRead;

    BIP_Fd_ReadWithTimeout(STDIN_FILENO, sizeof(buffer)-1, 0 /*timeout*/, buffer, &bytesRead );
    buffer[bytesRead] = '\0';   /* Null-terminate whatever we read. */

    if (strstr(buffer, "q"))
    {
        BDBG_LOG((BIP_MSG_PRE_FMT "Received quit " BIP_MSG_PRE_ARG));
        return true;
    }
    return false;
}

static void endOfStreamCallbackFromBIP(
    void *context,
    int   param
    )
{
    AppStreamerCtx *pAppStreamerCtx = context;

    BSTD_UNUSED( param );

    BDBG_MSG(( BIP_MSG_PRE_FMT "B_Event_Set( pAppCtx->hHttpStreamerEvent )" BIP_MSG_PRE_ARG ));
    pAppStreamerCtx->endOfStreamerRcvd = true;
    B_Event_Set( pAppStreamerCtx->pAppCtx->hHttpStreamerEvent );
}

void stopAndDestroyStreamer(
    AppStreamerCtx *pAppStreamerCtx
    )
{
    if (!pAppStreamerCtx->hHttpStreamer)
        return;
    BDBG_MSG(( BIP_MSG_PRE_FMT " Stopping Streamer %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer));
    BIP_HttpServer_StopStreamer( pAppStreamerCtx->pAppCtx->hHttpServer, pAppStreamerCtx->hHttpStreamer );

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
    BIP_HttpRequestHandle hHttpRequest,
    BIP_HttpResponseStatus responseStatus
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

static BIP_Status startStreamer(
    AppStreamerCtx *pAppStreamerCtx,
    BIP_HttpRequestHandle hHttpRequest
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_HttpResponseStatus responseStatus;
    BIP_MediaInfoStream *pMediaInfoStream = NULL;
    AppCtx *pAppCtx = pAppStreamerCtx->pAppCtx;

    /* Create HttpStreamer: app can alternatively create the streamers at the init time */
    {
        BIP_HttpServerCreateStreamerSettings createStreamerSettings;

        BIP_HttpServer_GetDefaultCreateStreamerSettings( &createStreamerSettings );
        createStreamerSettings.endOfStreamingCallback.callback = endOfStreamCallbackFromBIP;
        createStreamerSettings.endOfStreamingCallback.context = pAppStreamerCtx;

        pAppStreamerCtx->hHttpStreamer = BIP_HttpServer_CreateStreamer( pAppCtx->hHttpServer, &createStreamerSettings );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( pAppStreamerCtx->hHttpStreamer ), ( "BIP_HttpServer_CreateStreamer Failed" ), rejectRequest, BIP_ERR_CREATE_FAILED, bipStatus );
        BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_HttpStreamer created: %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer ));
    }

    /* Now provide File input settings */
    {
        BIP_StreamerFileInputSettings fileInputSettings;
        BIP_StreamerStreamInfo streamerStreamInfo;  /* Stream related information of this file. */

        pMediaInfoStream = BIP_MediaInfo_GetStream(pAppCtx->hMediaInfo);
        responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
        BIP_CHECK_GOTO( (pMediaInfoStream != NULL), ("BIP_MediaInfoGetStream() Failed"), rejectRequest, BIP_ERR_INTERNAL, bipStatus);

        BIP_Streamer_GetDefaultFileInputSettings(&fileInputSettings);

        /* Parse the Byte Range Headers. */
        {
            BIP_HttpRequestParsedRangeHeaderForBytes *pRangeHeaderParsed = NULL;

            /* Check if either TimeSeekRange and/or Byte-Range Headers are present. */
            /* If both type of Range headers are present, then app can choose to prefer one over another. */
            /* DLNA Specs recommend ByteRange over TimeSeekRange: indicated in 7.5.4.3.3.17 of Dlna spec. */
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            bipStatus = BIP_HttpRequest_ParseRangeHeaderForBytes(hHttpRequest, &pRangeHeaderParsed );
            BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE), ( "BIP_HttpRequest_ParseRangeHeaderForBytes() failed" ), rejectRequest, bipStatus, bipStatus );

            if( bipStatus == BIP_SUCCESS )
            {
                /* Note: this example is only handling the 1st Range Entry, but app can use all Entries! */
                bipStatus = BIP_HttpRequest_GetRangeEntryOffset(
                                &(pRangeHeaderParsed->pByteRangeSet[0]),
                                pMediaInfoStream->contentLength,
                                &pAppStreamerCtx->rangeStartOffset,
                                &pAppStreamerCtx->rangeLength
                                );
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE), ( "BIP_HttpRequest_GetRangeEntryOffset Failed" ), rejectRequest, bipStatus, bipStatus );
                /* NOTE: In this app we only check for success, other app can also check for BIP_INF_NOT_AVAILABLE and take action appropriate action based on its requirement.*/

                BDBG_ERR(( BIP_MSG_PRE_FMT "numRangeEntries %d[#0]: Range Start %llu, Length %llu" BIP_MSG_PRE_ARG,
                                pRangeHeaderParsed->byteRangeCount, pAppStreamerCtx->rangeStartOffset, pAppStreamerCtx->rangeLength ));

                /* Range parsing is complete, so fill-in the values in the input settings. */
                fileInputSettings.beginByteOffset = pAppStreamerCtx->rangeStartOffset;
                fileInputSettings.endByteOffset = pAppStreamerCtx->rangeStartOffset + pAppStreamerCtx->rangeLength - 1;
            }
        }

#if  0  /** TODO:Will be added once we add custom header apis.**/
        /* Parse the Byte Range Headers. */
        if (BIP_HttpRequest_HeaderPresent( hHttpRequest,"Range") == BIP_SUCCESS)
        {
            const char *pRangeUnits;
            int numRangeEntries;

            if (BIP_HttpRequest_GetNumRangeEntries( hHttpRequest, &pRangeUnits, &numRangeEntries ) == BIP_SUCCESS)
            {
                /* Request contains the Range Header, it can have 0 or more entries! */
                BIP_HttpRequestRangeEntry rangeEntry;

                /* Note: this example is only handling the 1st Range Entry, but app can use all of numEntries! */
                bipStatus = BIP_HttpRequest_GetRangeEntry( hHttpRequest, 0 /* index */, &rangeEntry );
                responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetRangeEntry Failed" ), rejectRequest, bipStatus, bipStatus );
                bipStatus = BIP_HttpRequest_GetRangeEntryOffset(
                        &rangeEntry,
                        true,/* contentLengthValid */
                        pMediaInfoStream->contentLength,
                        &pAppStreamerCtx->rangeStartOffset,
                        &pAppStreamerCtx->rangeLengthValid,
                        &pAppStreamerCtx->rangeLength
                        );
                responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetRangeEntryOffset Failed" ), rejectRequest, bipStatus, bipStatus );
                BDBG_MSG(( BIP_MSG_PRE_FMT "numRangeEntries %d[#0]: Range Start %llu, Length %llu, RangeLengthValid %d" BIP_MSG_PRE_ARG,
                            numRangeEntries, pAppStreamerCtx->rangeStartOffset, pAppStreamerCtx->rangeLength, pAppStreamerCtx->rangeLengthValid ));

                /* Range parsing is complete, so fill-in the values in the input settings. */
                fileInputSettings.beginByteOffset = pAppStreamerCtx->rangeStartOffset;
                if (pAppStreamerCtx->rangeLengthValid)
                {
                    fileInputSettings.endByteOffset = pAppStreamerCtx->rangeStartOffset + pAppStreamerCtx->rangeLength - 1;
                }
            }
        }
#endif

        BIP_Streamer_GetStreamerStreamInfoFromMediaInfo( pMediaInfoStream, &streamerStreamInfo );

        bipStatus = BIP_HttpStreamer_SetFileInputSettings( pAppStreamerCtx->hHttpStreamer, BIP_String_GetString(pAppCtx->hMediaFileAbsolutePathName), &streamerStreamInfo, &fileInputSettings );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_SetFileInputSettings Failed" ), rejectRequest, bipStatus, bipStatus );
    }

    /* Now specify the Tracks that could be added for streaming. */
    /* Note: this example assumes that all programs & tracks in a stream will be streamed out! */

    /* Provide Streamer output settings */
    {
        BIP_HttpStreamerOutputSettings streamerOutputSettings;

        BIP_HttpStreamer_GetDefaultOutputSettings( &streamerOutputSettings );
        bipStatus = BIP_HttpStreamer_SetOutputSettings(pAppStreamerCtx->hHttpStreamer, BIP_HttpStreamerProtocol_eDirect, &streamerOutputSettings);
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_SetOutputSettings Failed" ), rejectRequest, bipStatus, bipStatus );
    }

    /* And as a last step, start the HttpStreamer. */
    {
        BIP_HttpServerStartStreamerSettings startSettings;

        BIP_HttpServer_GetDefaultStartStreamerSettings( &startSettings );
        startSettings.streamerStartSettings.inactivityTimeoutInMs = 50000;
        bipStatus = BIP_HttpServer_StartStreamer( pAppCtx->hHttpServer, pAppStreamerCtx->hHttpStreamer, hHttpRequest, &startSettings );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpServer_StartStreamer Failed: hHttpStreamer %p", (void *)pAppStreamerCtx->hHttpStreamer ), rejectRequest, bipStatus, bipStatus );
    }
    return (bipStatus);

rejectRequest:
    /* This label handles the error cases when app encounters an error before BIP_HttpServer_StartStreamer(). */
    /* Thus it needs to reject the current hHttpRequest. */
    rejectRequestAndSetResponseHeaders( pAppCtx, hHttpRequest, responseStatus );

    if (pAppStreamerCtx->hHttpStreamer)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT " Destroying Streamer %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer));
        /* Note: App doesn't need to destroy the streamer & can maintain a free-list of them. */
        BIP_HttpServer_DestroyStreamer( pAppCtx->hHttpServer,  pAppStreamerCtx->hHttpStreamer );
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
    const char               *pMethodName;
    AppStreamerCtx           *pAppStreamerCtx = NULL;
    BIP_HttpRequestHandle    hHttpRequest;
    BIP_HttpResponseStatus   responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;

    while (true)
    {
        hHttpRequest = NULL;
        /* Check if there is a HTTP Request available for processing! */
        {
            bipStatus = BIP_HttpServer_RecvRequest( pAppCtx->hHttpServer, &hHttpRequest, NULL );
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
            BIP_CHECK_GOTO(( pAppStreamerCtx ), ( "Memory Allocation Failed" ), rejectRequest, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

            pAppStreamerCtx->pAppCtx = pAppCtx;
            BDBG_MSG(( BIP_MSG_PRE_FMT "AppStreamerCtx %p: Setting up Streamer" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx));
        }

        /* Process the received Request, start w/ the Method */
        {
            bipStatus = BIP_HttpRequest_GetMethod( hHttpRequest, &method, &pMethodName );
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetMethod Failed" ), rejectRequest, bipStatus, bipStatus );

            /* Validate the Request Method! */
            responseStatus = BIP_HttpResponseStatus_e501_NotImplemented;
            BIP_CHECK_GOTO( (method == BIP_HttpRequestMethod_eHead || method == BIP_HttpRequestMethod_eGet),
                    ( BIP_MSG_PRE_FMT " ERROR: method (%s) is not recognized/supported, only HEAD & GET are supported!" BIP_MSG_PRE_ARG, pMethodName ),
                    rejectRequest, BIP_ERR_INVALID_REQUEST_TARGET, bipStatus );

            /* Retrieve the requested URL */
            bipStatus = BIP_HttpRequest_GetTarget( hHttpRequest, &pAppStreamerCtx->pUrlPath);
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetTarget Failed" ), rejectRequest, bipStatus, bipStatus );
            BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_HttpRequest_GetMethod: Method %s, URL %s" BIP_MSG_PRE_ARG,
                        pMethodName, pAppStreamerCtx->pUrlPath
                        ));
        }

        /* Note: App will map this Url to Stream & ProgramIndex. */
        /* For File streaming example, URL maps to the actual media file name. */


        /* All is well, so Start Streamer */
        {
            bipStatus = startStreamer( pAppStreamerCtx, hHttpRequest);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "startStreamer Failed" ), error, bipStatus, bipStatus );

            /* Streamer is started, now track this streamer context. */
            BLST_Q_INSERT_TAIL( &pAppCtx->streamerListHead, pAppStreamerCtx, streamerListNext );

            BDBG_MSG(( BIP_MSG_PRE_FMT "AppStreamerCtx %p: Streaming is started!" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx));
        }
        continue;


rejectRequest:
        /* Some error happened, so reject the current hHttpRequest. */
        rejectRequestAndSetResponseHeaders( pAppCtx, hHttpRequest, responseStatus );
        if (pAppStreamerCtx) B_Os_Free(pAppStreamerCtx);
        pAppStreamerCtx = NULL;
        /* continue back to the top of the loop. */
    } /* while */

error:
    if (pAppStreamerCtx)
    {
        if (pAppStreamerCtx->hHttpStreamer) stopAndDestroyStreamer( pAppStreamerCtx );
        B_Os_Free(pAppStreamerCtx);
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

    BDBG_MSG(( BIP_MSG_PRE_FMT "B_Event_Set( pAppCtx->hHttpRequestRcvdEvent )" BIP_MSG_PRE_ARG ));
    B_Event_Set( pAppCtx->hHttpRequestRcvdEvent );
} /* requestReceivedCallbackFromBip */

static void unInitHttpServer(
    AppCtx *pAppCtx
    )
{
    AppStreamerCtx *pAppStreamerCtx;

    if (!pAppCtx)
        return;
    BDBG_MSG(( BIP_MSG_PRE_FMT "appCtx %p" BIP_MSG_PRE_ARG, (void *)pAppCtx ));

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
        pAppCtx->hHttpServer = BIP_HttpServer_Create( NULL );
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
        bipStatus = BIP_HttpServer_Start( pAppCtx->hHttpServer, &httpServerStartSettings );
        BIP_CHECK_GOTO(( bipStatus==BIP_SUCCESS ), ( "BIP_HttpServer_Start Failed" ), error, bipStatus, bipStatus );
    }

    pAppCtx->httpServerStarted = true;
    bipStatus = BIP_SUCCESS;

error:
    return ( bipStatus );
} /* initHttpServer */

static void printUsage(
    char   *pCmdName
    )
{
    printf( "Usage: %s\n", pCmdName);
    printf(
            "  --help or -h for help\n"
            "  -port        #   Server Port (default is port 80\n"
            "  -mediaFile   #   Media File Path (default /data/videos/AbcMpeg2HD.mpg)\n"
            "  -infoDir     #   Info Files Directory Path (default is /data/info)\n"
          );
    exit(0);
} /* printUsage */

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
        }
        else if ( !strcmp(argv[i], "-port") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hPort, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-mediaFile") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hMediaFileAbsolutePathName, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-infoDir") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hInfoDirectoryPath, argv[++i] );
        }
        else
        {
            printf("Error: incorrect or unsupported option: %s\n", argv[i]);
            printUsage(argv[0]);
            exit(0);
        }
    }
    bipStatus = BIP_SUCCESS;
    BDBG_LOG(( BIP_MSG_PRE_FMT " port %s, mediaFile %s, infoDir %s" BIP_MSG_PRE_ARG,
                BIP_String_GetString( pAppCtx->hPort ),
                BIP_String_GetString( pAppCtx->hMediaFileAbsolutePathName ),
                BIP_String_GetString( pAppCtx->hInfoDirectoryPath )
             ));
    return ( bipStatus );
} /* parseOptions */

void unInitAppCtx(
    AppCtx *pAppCtx
    )
{
    if (!pAppCtx) return;
    if (pAppCtx->hPort) BIP_String_Destroy( pAppCtx->hPort);
    if (pAppCtx->hMediaFileAbsolutePathName) BIP_String_Destroy( pAppCtx->hMediaFileAbsolutePathName);
    if (pAppCtx->hInfoFileAbsolutePathName) BIP_String_Destroy( pAppCtx->hInfoFileAbsolutePathName);
    if (pAppCtx->hInfoDirectoryPath) BIP_String_Destroy( pAppCtx->hInfoDirectoryPath);
    if (pAppCtx->hMediaInfo) BIP_MediaInfo_Destroy( pAppCtx->hMediaInfo);

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

    pAppCtx->hMediaFileAbsolutePathName = BIP_String_CreateFromChar( HTTP_SERVER_MEDIA_FILE );
    BIP_CHECK_GOTO( (pAppCtx->hMediaFileAbsolutePathName), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    pAppCtx->hInfoFileAbsolutePathName = BIP_String_Create( );
    BIP_CHECK_GOTO( (pAppCtx->hInfoFileAbsolutePathName), ("BIP_String_Create() Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

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

    /* Parse commandline options */
    bipStatus = parseOptions( argc, argv, pAppCtx );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "parseOptions Failed" ), errorParseOptions, bipStatus, bipStatus );

    /* Initialize NEXUS */
#if NXCLIENT_SUPPORT
    {
        NxClient_JoinSettings joinSettings;

        NxClient_GetDefaultJoinSettings(&joinSettings);
        snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
        nrc = NxClient_Join(&joinSettings);
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NxClient_Join Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }
#else /* !NXCLIENT_SUPPORT */
    nrc = NEXUS_Platform_Init( NULL );
    BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_Init Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
#endif /* NXCLIENT_SUPPORT */

    /* Generate MediaInfo for media files */
    {
        bipStatus = BIP_String_StrcpyPrintf( pAppCtx->hInfoFileAbsolutePathName,
                                                        "%s%s.xml",
                                                        BIP_String_GetString( pAppCtx->hInfoDirectoryPath ),
                                                        BIP_String_GetString( pAppCtx->hMediaFileAbsolutePathName ) );
        BIP_CHECK_GOTO( (bipStatus==BIP_SUCCESS), ("BIP_String_CreateFromPrintf() Failed"), error, bipStatus, bipStatus);

        BDBG_MSG(( BIP_MSG_PRE_FMT "Creating MediaInfo for %s" BIP_MSG_PRE_ARG, BIP_String_GetString(pAppCtx->hMediaFileAbsolutePathName)));
        BDBG_MSG(( BIP_MSG_PRE_FMT "Info File is %s" BIP_MSG_PRE_ARG, BIP_String_GetString(pAppCtx->hInfoFileAbsolutePathName)));

        /* This will create the mediaInfo (meta data) xml file if they don't already exist.*/
        pAppCtx->hMediaInfo = BIP_MediaInfo_CreateFromMediaFile(
                            BIP_String_GetString(pAppCtx->hMediaFileAbsolutePathName),
                            BIP_String_GetString(pAppCtx->hInfoFileAbsolutePathName),
                            NULL
                            );
        BIP_CHECK_GOTO( (pAppCtx->hMediaInfo != NULL), ("BIP_MediaInfo_CreateFromMediaFile() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);
   }

    /* Initialize HttpServer */
    bipStatus = initHttpServer( pAppCtx );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "initHttpServer Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

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
                BDBG_MSG(( BIP_MSG_PRE_FMT "Process HttpRequestEvent[i = %d]" BIP_MSG_PRE_ARG, i));
                /* process all events on the HttpServer */
                processHttpRequestEvent( pAppCtx );
                /* Continue to process next event */
                continue;
            }
            if (triggeredEvents[i] == pAppCtx->hHttpStreamerEvent)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "Process events on all HttpStreamers" BIP_MSG_PRE_ARG ));
                processHttpStreamerEvent( pAppCtx );
                continue;
            }
            BDBG_WRN(( BIP_MSG_PRE_FMT " didn't process event (i %d)" BIP_MSG_PRE_ARG, i ));
        }
    }

    BDBG_LOG(( BIP_MSG_PRE_FMT " Shutting HttpServer down..." BIP_MSG_PRE_ARG ));

error:
    unInitHttpServer( pAppCtx );
#if NXCLIENT_SUPPORT
    NxClient_Uninit();
#else /* !NXCLIENT_SUPPORT */
    NEXUS_Platform_Uninit();
#endif /* NXCLIENT_SUPPORT */
errorParseOptions:
    unInitAppCtx( pAppCtx );
errorBipInit:
    BIP_Uninit();

    BDBG_LOG(( "All done!" ));
    return (0); /* main */
}
