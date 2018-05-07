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
#include "blst_queue.h"
#include "bip.h"

#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_platform_client.h"
#else
#include "nexus_platform.h"
#endif

BDBG_MODULE(http_file_streamer);

#define HTTP_SERVER_PORT_STRING "80"
#define HTTP_SERVER_DTCP_IP_AKE_PORT_STRING "8000"
#define HTTP_SERVER_MEDIA_DIRECTORY_PATH "/data/videos"
#define HTTP_SERVER_INFO_DIRECTORY_PATH  "/data/info"
#define MAX_TRACKED_EVENTS 2
#define DTCP_IP_KEY_FORMAT_COMMON_DRM   "commonDrm"
#define DTCP_IP_KEY_FORMAT_TEST         "test"

#define TRANSCODE_PROFILE_720p_AVC "720pAvc"
#define TRANSCODE_PROFILE_480p_AVC "480pAvc"

typedef struct AppCtx
{
    BIP_StringHandle            hPort;
    BIP_StringHandle            hDtcpIpAkePort;
    BIP_StringHandle            hInterfaceName;
    BIP_StringHandle            hMediaDirectoryPath;
    BIP_StringHandle            hInfoDirectoryPath;
    BIP_StringHandle            hDtcpIpKeyFormat;
    bool                        httpServerStarted;
    BIP_HttpServerHandle        hHttpServer;
    B_EventGroupHandle          hHttpEventGroup;
    B_EventHandle               hHttpRequestRcvdEvent;
    B_EventHandle               hHttpStreamerEvent;
    int                         maxTriggeredEvents;
    unsigned                    trackGroupId;       /* For MPEG2-TS this specifies the program_number. */
    unsigned                    inactivityTimeoutInMs;
    bool                        enableContinousPlay;
    bool                        disableTrickmode;
    bool                        disableAudio;
    bool                        enablePacing;
    bool                        enableDtcpIp;
    B_CCI_T                     dtcpIpCopyControlInfo;
    bool                        enableSlaveMode;
    bool                        enableXcode;
    bool                        enableHls;
    bool                        enableMpegDash;
    char                        *xcodeProfile;
    bool                        printStatus;
    bool                        disableAvHeadersInsertion;
    bool                        disableContentLengthInsertion;
    bool                        enableAllPass;
    bool                        enableHwOffload;
    bool                        enableStreamingUsingPlaybackCh;
    unsigned                    maxDataRate;
    bool                        enableHttpChunkXferEncoding;
    unsigned                    chunkSizeInBytes;
    unsigned                    pcpPayloadLengthInBytes;
    BLST_Q_HEAD( streamerListHead, AppStreamerCtx ) streamerListHead; /* List of Streamer Ctx */
} AppCtx;

typedef struct AppStreamerCtx
{
    BLST_Q_ENTRY( AppStreamerCtx ) streamerListNext;  /* Next Streamer Ctx */
    AppCtx                      *pAppCtx;
    bool                        endOfStreamerRcvd;  /* We have received End of Streaming event for this Streamer. */
    const char                  *pMediaFileAbsolutePathName; /* Absolute URL Path Name pointer */
    const char                  *pMethodName;       /* HTTP Request Method Name */
    BIP_StringHandle            hMediaFileAbsolutePathName; /* Absolute URL Path-> Media Directory Path + URL Name */
    BIP_MediaInfoHandle         hMediaInfo;         /* MediaInfo object handle.*/
    BIP_StreamerStreamInfo      streamerStreamInfo; /* StreamerStreamInfo structure will be populated from mediaInfo structure received from hMediaInfo object.*/
    const char                  *pPlaySpeed;        /* PlaySpeed string if set */
    bool                        rangeHeaderPresent;   /* Optional: if true, Request contains a Range Header. */
    uint64_t                    rangeStartOffset;   /* Optional: starting offset of the Range Header. */
    uint64_t                    rangeLength;        /* Optional: range length from the starting offset. */
    bool                        dlnaTimeSeekRangePresent; /* Optional: if true, Request contains a dlnaTimeSeekRange Header. */
    BIP_HttpStreamerHandle      hHttpStreamer;        /* Cached Streamer handle */
    unsigned                    trackGroupId;         /* For MPEG2-TS this specifies the program_number. */
    bool                        enableContinousPlay;
    bool                        disableTrickmode;
    bool                        disableAudio;
    bool                        enablePacing;
    bool                        enableDtcpIp;
    bool                        enableSlaveMode;
    bool                        enableXcode;
    bool                        enableHls;
    bool                        enableMpegDash;
    bool                        enableTransportTimestamp;
    bool                        enableAllPass;
    bool                        enableHwOffload;
} AppStreamerCtx;

#define USER_INPUT_BUF_SIZE 64
/* function to allow proper exit of server */
int readInput( void )
{
    char    buffer[USER_INPUT_BUF_SIZE];
    size_t  bytesRead;

    BIP_Fd_ReadWithTimeout(STDIN_FILENO, sizeof(buffer)-1, 0 /*timeout*/, buffer, &bytesRead );
    buffer[bytesRead] = '\0';   /* Null-terminate whatever we read. */

    if (strstr(buffer, "q"))
    {
        BDBG_LOG((BIP_MSG_PRE_FMT "Received quit " BIP_MSG_PRE_ARG));
        return 'q';
    }
    else if (strstr(buffer, "s"))
    {
        return 's';
    }
    else return 'c'; /* continue */
    return false;
}

static void endOfStreamCallbackFromBIP(
    void *context,
    int   param
    )
{
    AppStreamerCtx *pAppStreamerCtx = context;

    BSTD_UNUSED( param );

    BDBG_WRN(( BIP_MSG_PRE_FMT "ctx=%p B_Event_Set( pAppCtx->hHttpStreamerEvent )" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx ));
    pAppStreamerCtx->endOfStreamerRcvd = true;
    B_Event_Set( pAppStreamerCtx->pAppCtx->hHttpStreamerEvent );
}

void destroyAppStreamerCtx(
    AppStreamerCtx *pAppStreamerCtx
    )
{
    if (pAppStreamerCtx == NULL)
        return;

    if (pAppStreamerCtx->hMediaFileAbsolutePathName)
    {
        BIP_String_Destroy(pAppStreamerCtx->hMediaFileAbsolutePathName);
        pAppStreamerCtx->hMediaFileAbsolutePathName = NULL;
    }
    if(pAppStreamerCtx->hMediaInfo)
    {
        BIP_MediaInfo_Destroy(pAppStreamerCtx->hMediaInfo);
    }
    B_Os_Free(pAppStreamerCtx);
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

#define HTTP_STREAMER_MAX_INACTIVITY_TIME_IN_MSEC 10000
bool stopAndDestroyInactiveStreamer(
    AppCtx          *pAppCtx
    )
{
    AppStreamerCtx          *pAppStreamerCtx;

    for ( pAppStreamerCtx = BLST_Q_FIRST(&pAppCtx->streamerListHead);
            pAppStreamerCtx;
            pAppStreamerCtx = BLST_Q_NEXT( pAppStreamerCtx, streamerListNext ) )
    {
        BIP_HttpStreamerStatus status;

        if ( BIP_HttpStreamer_GetStatus( pAppStreamerCtx->hHttpStreamer, &status ) == BIP_SUCCESS )
        {
            if ( status.inactivityTimeInMs >= HTTP_STREAMER_MAX_INACTIVITY_TIME_IN_MSEC )
            {
                BDBG_WRN(( BIP_MSG_PRE_FMT "Destroy inactive (=%dmsec) HttpStreamer=%p so as to Free-up Nexus Resources for the next request. " BIP_MSG_PRE_ARG, status.inactivityTimeInMs, (void *)pAppStreamerCtx->hHttpStreamer ));
                BLST_Q_REMOVE( &pAppCtx->streamerListHead, pAppStreamerCtx, streamerListNext );
                stopAndDestroyStreamer( pAppStreamerCtx);
                destroyAppStreamerCtx(pAppStreamerCtx);
                return true;
            }
        }
        else BDBG_ASSERT( NULL );
    }
    BDBG_WRN(( BIP_MSG_PRE_FMT "No streamers inactive at this time (inactivityMaxTime=%d" BIP_MSG_PRE_ARG, HTTP_STREAMER_MAX_INACTIVITY_TIME_IN_MSEC ));
    return false;
} /* stopAndDestroyInactiveStreamer */

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
            destroyAppStreamerCtx(pAppStreamerCtx);
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
    BIP_HttpRequestHandle  hHttpRequest,
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

static BIP_StringHandle buildNavFileName(
    const char *pInfoFilesDirectoryPath,
    const char *pMediaFileAbsolutePathname,
    unsigned videoTrackId
    )
{
    BIP_StringHandle hNavFileAbsolutePathName;
    hNavFileAbsolutePathName = BIP_String_CreateFromPrintf( "%s%s_%d.nav", pInfoFilesDirectoryPath, pMediaFileAbsolutePathname, videoTrackId);
    return (hNavFileAbsolutePathName);
} /* buildNavFileName */

static BIP_Status startStreamer(
    AppStreamerCtx *pAppStreamerCtx,
    BIP_HttpRequestHandle hHttpRequest
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_HttpResponseStatus responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
    BIP_MediaInfoStream *pMediaInfoStream = NULL;

    /* Create HttpStreamer. */
    {
        BIP_HttpServerCreateStreamerSettings createStreamerSettings;

        BIP_HttpServer_GetDefaultCreateStreamerSettings( &createStreamerSettings );
        createStreamerSettings.endOfStreamingCallback.callback = endOfStreamCallbackFromBIP;
        createStreamerSettings.endOfStreamingCallback.context = pAppStreamerCtx;

        pAppStreamerCtx->hHttpStreamer = BIP_HttpServer_CreateStreamer( pAppStreamerCtx->pAppCtx->hHttpServer, &createStreamerSettings );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( pAppStreamerCtx->hHttpStreamer ), ( "BIP_HttpServer_CreateStreamer Failed" ), rejectRequest, BIP_ERR_CREATE_FAILED, bipStatus );
        BDBG_MSG(( BIP_MSG_PRE_FMT " BIP_HttpStreamer created: %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer ));
    }

    /* Get BIP_MediaInfoStream from hMediaInfo object.*/
    pMediaInfoStream = BIP_MediaInfo_GetStream(
                            pAppStreamerCtx->hMediaInfo
                            );
    BIP_CHECK_GOTO( (pMediaInfoStream != NULL), ("BIP_MediaInfo_GetStream() Failed"), rejectRequest, BIP_ERR_INTERNAL, bipStatus);

    BIP_Streamer_GetStreamerStreamInfoFromMediaInfo( pMediaInfoStream,&(pAppStreamerCtx->streamerStreamInfo) );

    /* Now provide File input settings */
    {
        BIP_StreamerFileInputSettings fileInputSettings;

        BIP_Streamer_GetDefaultFileInputSettings(&fileInputSettings);

        /* Parse the PlaySpeed Header and if present, configure its values in the input settings. */
        {
            const char *pHeaderValue;
            BIP_HttpHeaderHandle hHeader = NULL;

            /* PlaySpeed.dlna.org: speed=6 */
            bipStatus = BIP_HttpRequest_GetNextHeader(hHttpRequest, NULL, "PlaySpeed.dlna.org", &hHeader, &pHeaderValue);

            if(bipStatus == BIP_SUCCESS)
            {
                if( strlen(pHeaderValue) > strlen("speed="))
                {
                    pAppStreamerCtx->pPlaySpeed = pHeaderValue + strlen("speed=");
                    fileInputSettings.playSpeed = pAppStreamerCtx->pPlaySpeed;
                }
            }
            else if(bipStatus == BIP_INF_NOT_AVAILABLE )
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "Header PlaySpeed.dlna.org doesn't exist." BIP_MSG_PRE_ARG ));
            }
        }

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

                BDBG_MSG(( BIP_MSG_PRE_FMT "numRangeEntries %d[#0]: Range Start %llu, Length %llu" BIP_MSG_PRE_ARG,
                                pRangeHeaderParsed->byteRangeCount, pAppStreamerCtx->rangeStartOffset, pAppStreamerCtx->rangeLength ));

                /* Range parsing is complete, so fill-in the values in the input settings. */
                fileInputSettings.beginByteOffset = pAppStreamerCtx->rangeStartOffset;
                fileInputSettings.endByteOffset = pAppStreamerCtx->rangeStartOffset + pAppStreamerCtx->rangeLength - 1;

                pAppStreamerCtx->rangeHeaderPresent = true;
            }
        }

        /* Handle the TimeSeekRange header unless there was also a Range header. */
        if (!pAppStreamerCtx->rangeHeaderPresent)
        {
            int64_t    startTimeInMs;
            int64_t    endTimeInMs;

            bipStatus = BIP_HttpRequest_ParseTimeSeekRangeDlnaOrgHeader(hHttpRequest, pMediaInfoStream->durationInMs, &startTimeInMs, &endTimeInMs );
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_NOT_AVAILABLE), ( "BIP_HttpRequest_ParseTimeSeekRangeDlnaOrgHeader Failed" ), rejectRequest, bipStatus, bipStatus );

            if (bipStatus == BIP_SUCCESS)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "TimeSeekRange.dlna.org header: startTimeInMs="BIP_I64_FMT", endTimeInMs="BIP_I64_FMT
                           BIP_MSG_PRE_ARG, BIP_I64_ARG(startTimeInMs), BIP_I64_ARG(endTimeInMs)));

                fileInputSettings.beginTimeOffsetInMs = startTimeInMs;
                fileInputSettings.endTimeOffsetInMs   = (endTimeInMs > 0) ? endTimeInMs : 0;

                pAppStreamerCtx->dlnaTimeSeekRangePresent = true;

                BDBG_MSG(( BIP_MSG_PRE_FMT "fileInputSettings: beginTimeOffsetInMs=%u, endTimeOffsetInMs=%u"
                           BIP_MSG_PRE_ARG, fileInputSettings.beginTimeOffsetInMs, fileInputSettings.endTimeOffsetInMs));
            }
        }

        if (pAppStreamerCtx->enableContinousPlay)
        {
            fileInputSettings.enableContinousPlay = true;
        }

        if ( pAppStreamerCtx->enablePacing && pMediaInfoStream->transportType == NEXUS_TransportType_eTs )
        {
            /* App wants to pace and this is a MPEG2 TS stream, so BIP can pace it using the Playback Channel. Set that option. */
            fileInputSettings.enableHwPacing = true;
        }

        if (pAppStreamerCtx->enableAllPass && !pAppStreamerCtx->enableXcode && !pAppStreamerCtx->enableHls)
        {
            fileInputSettings.enableAllPass = true;
        }
        /* Set File input Settings */
        bipStatus = BIP_HttpStreamer_SetFileInputSettings( pAppStreamerCtx->hHttpStreamer, pAppStreamerCtx->pMediaFileAbsolutePathName, &(pAppStreamerCtx->streamerStreamInfo), &fileInputSettings );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_SetFileInputSettings Failed" ), rejectRequest, bipStatus, bipStatus );

        BDBG_MSG(( BIP_MSG_PRE_FMT "Method: %s, Absolute URL: %s, PlaySpeed: %s, Byte Range: %s, DlnaTimeSeek: %s, TimeOffsetInMsec: start %u, end %u" BIP_MSG_PRE_ARG,
                    pAppStreamerCtx->pMethodName, pAppStreamerCtx->pMediaFileAbsolutePathName,
                    pAppStreamerCtx->pPlaySpeed ? pAppStreamerCtx->pPlaySpeed : "1",
                    pAppStreamerCtx->rangeHeaderPresent ? "Y" : "N",
                    pAppStreamerCtx->dlnaTimeSeekRangePresent ? "Y" : "N",
                    fileInputSettings.beginTimeOffsetInMs, fileInputSettings.endTimeOffsetInMs
                 ));
    } /* Input Settings. */

    /*
     * For AV type media file streaming, app should provide individual tracks to the streamer.
     * This allows streamer to insert the PSI information in the HTTP Response. In addition, this is
     * required if app enables HW pacing or doesn't set the enableAllPass flag in the FileInputSettings above.
     * However, user may set the enableAllPass cmdline option, however, it can't be honored & thus ignored if
     * incoming request requires us to transcode. This is true when client is requesting a URL w/ either
     * .xcode flag or with .m3u8 flag (for HLS).
     */
    if ( pMediaInfoStream->transportType != NEXUS_TransportType_eUnknown &&
         (!pAppStreamerCtx->enableAllPass || pAppStreamerCtx->enableHls || pAppStreamerCtx->enableXcode)
       )
    {
        BIP_MediaInfoTrackGroup *pMediaInfoTrackGroup = NULL;
        BIP_MediaInfoTrack      *pMediaInfoTrack = NULL;
        BIP_StreamerTrackInfo   streamerTrackInfo;
        bool                    trackGroupPresent = false;

        if(pMediaInfoStream->numberOfTrackGroups != 0)
        {
            trackGroupPresent = true;

            if( pAppStreamerCtx->trackGroupId != 0)
            {
                /* Get Track Group by Id.*/
                 pMediaInfoTrackGroup = BIP_MediaInfo_GetTrackGroupById(  pAppStreamerCtx->hMediaInfo, pAppStreamerCtx->trackGroupId );
                 responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
                 BIP_CHECK_GOTO( (pMediaInfoTrackGroup != NULL), ("BIP_MediaInfo_GetTrackGroupById() Failed"), rejectRequest, BIP_ERR_INTERNAL, bipStatus);
            }
            else
            {
                /* Get the first track group and stream data for that track group.*/
                pMediaInfoTrackGroup = pMediaInfoStream->pFirstTrackGroupInfo;
                responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
                BIP_CHECK_GOTO( (pMediaInfoTrackGroup != NULL), ("pMediaInfoStream->pFirstTrackGroupInfo is NULL"), rejectRequest, BIP_ERR_INTERNAL, bipStatus);
            }
            pMediaInfoTrack = pMediaInfoTrackGroup->pFirstTrackForTrackGroup;

            /*TODO:Only for TS we will add Pat and Pmt track. For non Ts types we will later check if we need to add any extra tracks.*/
            if(pMediaInfoStream->transportType == NEXUS_TransportType_eTs)
            {
                /************************* Add Pat ***************************/
                B_Os_Memset( &streamerTrackInfo, 0, sizeof( streamerTrackInfo ) );
                streamerTrackInfo.trackId = 0; /* PAT is always at PID == 0 */
                streamerTrackInfo.type = BIP_MediaInfoTrackType_eOther;
                bipStatus = BIP_HttpStreamer_AddTrack( pAppStreamerCtx->hHttpStreamer, &streamerTrackInfo, NULL );
                responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_AddTrack Failed for PAT" ), rejectRequest, bipStatus, bipStatus );
                /************************* Pat Added *************************/

                /************************* Add Pmt ***************************/
                B_Os_Memset( &streamerTrackInfo, 0, sizeof( streamerTrackInfo ) );
                streamerTrackInfo.trackId = pMediaInfoTrackGroup->type.Ts.pmtPid; /* PMT Pid for this program. */
                streamerTrackInfo.type = BIP_MediaInfoTrackType_ePmt;
                bipStatus = BIP_HttpStreamer_AddTrack( pAppStreamerCtx->hHttpStreamer, &streamerTrackInfo, NULL );
                responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_AddTrack Failed for PMT" ), rejectRequest, bipStatus, bipStatus );
                /************************* Pmt Added *************************/
            }
        }
        else
        {
            /* None of the track belongs to any trackGroup, in this case stream out all tracks from mediaInfoStream.*/
            pMediaInfoTrack = pMediaInfoStream->pFirstTrackInfoForStream;
        }
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO( (pMediaInfoTrack != NULL), ("First pMediaInfoTrack itself is NULL"), rejectRequest, BIP_ERR_INTERNAL, bipStatus);/* At least one track has to be present */

        while (pMediaInfoTrack)
        {
            BIP_StringHandle hNavFileAbsolutePathName = NULL;

            B_Os_Memset( &streamerTrackInfo, 0, sizeof( streamerTrackInfo ) );
            BIP_Streamer_GetStreamerTrackInfoFromMediaInfo(pMediaInfoTrack, &streamerTrackInfo );

            if ( pMediaInfoTrack->trackType == BIP_MediaInfoTrackType_eAudio && pAppStreamerCtx->disableAudio )
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "-disableAudio option is set, so NOT adding audioTrack!" BIP_MSG_PRE_ARG));
                goto nextTrack;
            }

            if ( pMediaInfoTrack->trackType == BIP_MediaInfoTrackType_eVideo
                    && ( pMediaInfoStream->transportType == NEXUS_TransportType_eTs )
                    && ( pMediaInfoTrack->info.video.codec != NEXUS_VideoCodec_eH264_Svc )
                    && ( pMediaInfoTrack->info.video.codec != NEXUS_VideoCodec_eH264_Mvc )
                    && ( pMediaInfoTrack->parsedPayload )
                    && ( pAppStreamerCtx->disableTrickmode == false))
            {
                hNavFileAbsolutePathName = buildNavFileName( BIP_String_GetString( pAppStreamerCtx->pAppCtx->hInfoDirectoryPath), BIP_String_GetString(pAppStreamerCtx->hMediaFileAbsolutePathName), streamerTrackInfo.trackId);
                responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
                BIP_CHECK_GOTO( (hNavFileAbsolutePathName != NULL), ("BIP_String_CreateFromPrintf() Failed"), rejectRequest, BIP_ERR_CREATE_FAILED, bipStatus);
                streamerTrackInfo.info.video.pMediaNavFileAbsolutePathName = BIP_String_GetString( hNavFileAbsolutePathName );
            }

            bipStatus = BIP_HttpStreamer_AddTrack( pAppStreamerCtx->hHttpStreamer, &streamerTrackInfo, NULL );
            responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
            if ( hNavFileAbsolutePathName )
            {
                BIP_String_Destroy( hNavFileAbsolutePathName );
                hNavFileAbsolutePathName = NULL;
            }
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_AddTrack Failed for track# %d", streamerTrackInfo.trackId ), rejectRequest, bipStatus, bipStatus );

nextTrack:
            if (true == trackGroupPresent)
            {
                pMediaInfoTrack = pMediaInfoTrack->pNextTrackForTrackGroup;
            }
            else
            {
                pMediaInfoTrack = pMediaInfoTrack->pNextTrackForStream;
            }
        }
    }

    /* Provide the Streamer output settings */
    {
        BIP_HttpStreamerProtocol streamerProtocol;
        BIP_HttpStreamerOutputSettings streamerOutputSettings;

        BIP_HttpStreamer_GetDefaultOutputSettings( &streamerOutputSettings );
        if (pAppStreamerCtx->enableDtcpIp)
        {
            streamerOutputSettings.enableDtcpIp = true;
            streamerOutputSettings.dtcpIpOutput.pcpPayloadLengthInBytes = pAppStreamerCtx->pAppCtx->pcpPayloadLengthInBytes;
            streamerOutputSettings.dtcpIpOutput.akeTimeoutInMs = 2000;
            streamerOutputSettings.dtcpIpOutput.copyControlInfo = pAppStreamerCtx->pAppCtx->dtcpIpCopyControlInfo;
        }

        streamerOutputSettings.streamerSettings.enableHwOffload = pAppStreamerCtx->enableHwOffload;
        streamerOutputSettings.streamerSettings.enableStreamingUsingPlaybackCh = pAppStreamerCtx->pAppCtx->enableStreamingUsingPlaybackCh;

        streamerProtocol = pAppStreamerCtx->enableHls ? BIP_HttpStreamerProtocol_eHls : BIP_HttpStreamerProtocol_eDirect;
        streamerOutputSettings.streamerSettings.mpeg2Ts.enableTransportTimestamp = pAppStreamerCtx->enableTransportTimestamp;
        streamerOutputSettings.disableAvHeadersInsertion = pAppStreamerCtx->pAppCtx->disableAvHeadersInsertion;
        streamerOutputSettings.disableContentLengthInsertion = pAppStreamerCtx->pAppCtx->disableContentLengthInsertion;
        if (pAppStreamerCtx->pAppCtx->maxDataRate > 0)
        {
            streamerOutputSettings.streamerSettings.maxDataRate = pAppStreamerCtx->pAppCtx->maxDataRate;
            streamerOutputSettings.streamerSettings.enableStreamingUsingPlaybackCh = true;
        }
        streamerOutputSettings.enableHttpChunkXferEncoding = pAppStreamerCtx->pAppCtx->enableHttpChunkXferEncoding;
        streamerOutputSettings.chunkSize = pAppStreamerCtx->pAppCtx->chunkSizeInBytes;

        bipStatus = BIP_HttpStreamer_SetOutputSettings(pAppStreamerCtx->hHttpStreamer, streamerProtocol, &streamerOutputSettings);
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_SetOutputSettings Failed" ), rejectRequest, bipStatus, bipStatus );

    }

    /* Add Encoder Profile. */
    {
#if NEXUS_HAS_VIDEO_ENCODER
        BIP_TranscodeProfile transcodeProfile;

        if ( pAppStreamerCtx->enableXcode )
        {
            if ( strncmp( pAppStreamerCtx->pAppCtx->xcodeProfile, TRANSCODE_PROFILE_720p_AVC, strlen(TRANSCODE_PROFILE_720p_AVC) ) == 0 )
            {
                BIP_Transcode_GetDefaultProfileFor_720p30_AVC_AAC_MPEG2_TS( &transcodeProfile );
            }
            else if ( strncmp( pAppStreamerCtx->pAppCtx->xcodeProfile, TRANSCODE_PROFILE_480p_AVC, strlen(TRANSCODE_PROFILE_480p_AVC) ) == 0 )
            {
                BIP_Transcode_GetDefaultProfileFor_480p30_AVC_AAC_MPEG2_TS( &transcodeProfile );
            }
            else
            {
                BIP_Transcode_GetDefaultProfile( &transcodeProfile );
            }
            transcodeProfile.disableAudio = pAppStreamerCtx->disableAudio;

            bipStatus = BIP_HttpStreamer_AddTranscodeProfile( pAppStreamerCtx->hHttpStreamer, &transcodeProfile );
            responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_AddTranscodeProfile Failed" ), rejectRequest, bipStatus, bipStatus );
        }
        else if ( pAppStreamerCtx->enableHls )
        {
            if ( 0 )
            /* Add a 480p profile. */
                /* NOTE: this is being commented out as some 480p profile encode profile default settings are not yet set correctly and thus players are not playing the initial ~10sec worth of frames. */
            {
                BIP_Transcode_GetDefaultProfileFor_480p30_AVC_AAC_MPEG2_TS( &transcodeProfile );
                transcodeProfile.disableAudio = pAppStreamerCtx->disableAudio;
                bipStatus = BIP_HttpStreamer_AddTranscodeProfile( pAppStreamerCtx->hHttpStreamer, &transcodeProfile );
                responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_AddTranscodeProfile Failed" ), rejectRequest, bipStatus, bipStatus );
            }
            /* Add a 720p profile. */
            {
                BIP_Transcode_GetDefaultProfileFor_720p30_AVC_AAC_MPEG2_TS( &transcodeProfile );
                transcodeProfile.disableAudio = pAppStreamerCtx->disableAudio;
                bipStatus = BIP_HttpStreamer_AddTranscodeProfile( pAppStreamerCtx->hHttpStreamer, &transcodeProfile );
                responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_AddTranscodeProfile Failed" ), rejectRequest, bipStatus, bipStatus );
            }
        }
#else
        if(pAppStreamerCtx->enableXcode || pAppStreamerCtx->enableHls )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "NEXUS Video Encoder Feature is not supported for this platform" BIP_MSG_PRE_ARG ));
            goto rejectRequest;
        }
#endif
    }

    /* At this point, we have set sucessfully configured the Input & Output settings of the HttpStreamer. */

    if ( strstr( pAppStreamerCtx->pMediaFileAbsolutePathName, ".htm" ) )
    {
        bipStatus = BIP_HttpStreamer_SetResponseHeader( pAppStreamerCtx->hHttpStreamer, "Content-Type", "text/html" );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_HTTP_MESSAGE_HEADER_ALREADY_SET),
                ( "BIP_HttpStreamer_SetResponseHeader Failed: hHttpStreamer %p", (void *)pAppStreamerCtx->hHttpStreamer ), rejectRequest, bipStatus, bipStatus );

    }

    /* Now Set any custom or app specific HTTP Headers that HttpStreamer should include when it sends out the Response Message. */
    {
        bipStatus = BIP_HttpStreamer_SetResponseHeader( pAppStreamerCtx->hHttpStreamer, "transferMode.dlna.org", "Streaming" );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_INF_HTTP_MESSAGE_HEADER_ALREADY_SET),
                ( "BIP_HttpStreamer_SetResponseHeader Failed: hHttpStreamer %p", (void *)pAppStreamerCtx->hHttpStreamer ), rejectRequest, bipStatus, bipStatus );

        /* App can add more custom headers here using this example above! */
    }

    /* And as a last step, start the HttpStreamer. */
    while ( true )
    {
        BIP_HttpServerStartStreamerSettings startSettings;

        BIP_HttpServer_GetDefaultStartStreamerSettings( &startSettings );
        startSettings.streamerStartSettings.inactivityTimeoutInMs = pAppStreamerCtx->pAppCtx->inactivityTimeoutInMs;
        bipStatus = BIP_HttpServer_StartStreamer( pAppStreamerCtx->pAppCtx->hHttpServer, pAppStreamerCtx->hHttpStreamer, hHttpRequest, &startSettings);
        if ( bipStatus == BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE )
        {
            /* Since Streamer start failed because a Nexus resource wasn't available. We will try to see if there are any inactive streamers and destroy them. */
            if ( stopAndDestroyInactiveStreamer( pAppStreamerCtx->pAppCtx ) == true )
            {
                BDBG_WRN(( BIP_MSG_PRE_FMT "Successfully Released an active Streamer, Retrying BIP_HttpServer_StartStreamer()" BIP_MSG_PRE_ARG ));
                continue;
            }
            else break; /* No inactive streamers at this time, goto reject request. */
        }
        else break; /* Some other error, so no point in destroying the inactive streamers. */
    }
    responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpServer_StartStreamer Failed: hHttpStreamer %p", (void *)pAppStreamerCtx->hHttpStreamer ), rejectRequest, bipStatus, bipStatus );
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

#define PACING "Pacing"
#define PROGRAM   "Program"

/* Parse the url and extract the query string parameters like Program and Pacing information.
   It also extracts and adds the target's realtive path in hTarget.*/
static BIP_Status parseUrl(
    const char *pUrl,
    AppStreamerCtx          *pAppStreamerCtx,
    BIP_StringHandle        hTarget
    )
{
    BIP_Status              bipStatus = BIP_SUCCESS;
    char *saveptr1;
    char *pTempChar1 = NULL;
    char *pTempChar2 = NULL;
    char *pLocalCopy= strdup(pUrl);

    pTempChar1 = strstr(pLocalCopy, "?");
    if(pTempChar1 == NULL) {
        bipStatus = BIP_String_StrcpyChar(hTarget, pUrl);
        BIP_CHECK_GOTO(( bipStatus==BIP_SUCCESS ), ( "BIP_String_StrcpyChar() Failed" ), end, BIP_ERR_CREATE_FAILED, bipStatus );
        BDBG_MSG(("Query string not available"));
        goto end;
    }
    bipStatus = BIP_String_StrcpyCharN(hTarget, pUrl, (pTempChar1 - pLocalCopy));
    BIP_CHECK_GOTO(( bipStatus==BIP_SUCCESS ), ( "BIP_String_StrcpyChar() Failed" ), end, BIP_ERR_CREATE_FAILED, bipStatus );

    pTempChar1++;

    pTempChar1 = strtok_r(pTempChar1, "=&", &saveptr1);
    if(pTempChar1 == NULL)
    {
       BDBG_MSG((BIP_MSG_PRE_FMT "Query string not available" BIP_MSG_PRE_ARG));
       goto end;
    }

    while(pTempChar1 != NULL)
    {
       if(!strcasecmp(pTempChar1, PACING))
       {
           pTempChar2 = strtok_r(NULL, "=&", &saveptr1);
           if(pTempChar2 == NULL)
           {
              BDBG_MSG((BIP_MSG_PRE_FMT "Incomplete Query string" BIP_MSG_PRE_ARG));
              goto end;
           }

           if(!strcasecmp(pTempChar2, "y"))
           {
               pAppStreamerCtx->enablePacing = true;
               BDBG_MSG((BIP_MSG_PRE_FMT "QueryString: enablePacing = %d" BIP_MSG_PRE_ARG, pAppStreamerCtx->enablePacing));
           }
           else if(!strcasecmp(pTempChar2, "n"))
           {
               pAppStreamerCtx->enablePacing = false;
               BDBG_MSG((BIP_MSG_PRE_FMT "QueryString: enablePacing = %d"BIP_MSG_PRE_ARG, pAppStreamerCtx->enablePacing));
           }
       }
       else if(!strcasecmp(pTempChar1, PROGRAM))
       {
           pTempChar2 = strtok_r(NULL, "=&", &saveptr1);
           if(pTempChar2 == NULL)
           {
              BDBG_MSG((BIP_MSG_PRE_FMT "Incomplete Query string" BIP_MSG_PRE_ARG));
              goto end;
           }
           pAppStreamerCtx->trackGroupId = strtol(pTempChar2, NULL, 10);

           BDBG_MSG((BIP_MSG_PRE_FMT "QueryString: trackGroupId = %d" BIP_MSG_PRE_ARG, pAppStreamerCtx->trackGroupId));
       }
       pTempChar1 = strtok_r(NULL, "=&", &saveptr1);
    }
end:
    free(pLocalCopy);
    return bipStatus;
}

/* Function to process incoming Requests */
static void processHttpRequestEvent(
    AppCtx *pAppCtx
    )
{
    BIP_Status                  bipStatus = BIP_SUCCESS;
    BIP_HttpRequestMethod       method;
    AppStreamerCtx              *pAppStreamerCtx = NULL;
    BIP_HttpRequestHandle       hHttpRequest;
    BIP_HttpResponseStatus      responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
    BIP_StringHandle            hTarget = NULL;
    BIP_StringHandle            hInfoFileAbsolutePathName = NULL;

    hTarget = BIP_String_Create();
    hInfoFileAbsolutePathName = BIP_String_Create();

    while (true)
    {
        hHttpRequest = NULL;
        /* Check if there is a HTTP Request available for processing! */
        {
            bipStatus = BIP_HttpServer_RecvRequest( pAppCtx->hHttpServer, &hHttpRequest, NULL );
            if ( bipStatus == BIP_INF_TIMEOUT )
            {
                if(hTarget)
                {
                    BIP_String_Destroy(hTarget);
                }

                if(hInfoFileAbsolutePathName)
                {
                    BIP_String_Destroy(hInfoFileAbsolutePathName);
                }

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

            /* Copy Global Settings into Streamer instance. */
            pAppStreamerCtx->enableContinousPlay = pAppCtx->enableContinousPlay;
            pAppStreamerCtx->disableTrickmode = pAppCtx->disableTrickmode;
            pAppStreamerCtx->disableAudio = pAppCtx->disableAudio;
            /*Initialize enablePacing from pAppCtx->enablePacing, if URL provides enablePacing flag then parseUrl will overwrite it.*/
            pAppStreamerCtx->enablePacing = pAppCtx->enablePacing;
            pAppStreamerCtx->enableSlaveMode = pAppCtx->enableSlaveMode;
            pAppStreamerCtx->enableXcode = pAppCtx->enableXcode;
            pAppStreamerCtx->enableHls = pAppCtx->enableHls;
            pAppStreamerCtx->enableMpegDash = pAppCtx->enableMpegDash;
            pAppStreamerCtx->enableAllPass = pAppCtx->enableAllPass;
            pAppStreamerCtx->enableHwOffload = pAppCtx->enableHwOffload;
            /*Initialize trackGroupId from pAppCtx->trackGroupId, if URL provides trackGroupId, then parseUrl will overwrite it.*/
            pAppStreamerCtx->trackGroupId = pAppCtx->trackGroupId;

            /* Create a BIP_String for URL Absolute Path Name. */
            pAppStreamerCtx->hMediaFileAbsolutePathName = BIP_String_Create();
            responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
            BIP_CHECK_GOTO(( pAppStreamerCtx->hMediaFileAbsolutePathName ), ( "Memory Allocation Failed" ), rejectRequest, BIP_ERR_CREATE_FAILED, bipStatus );
        }

        /* Process the received Request, start w/ the Method */
        {
            const char *pTmpUrlPath;

            bipStatus = BIP_HttpRequest_GetMethod( hHttpRequest, &method, &pAppStreamerCtx->pMethodName );
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetMethod Failed" ), rejectRequest, bipStatus, bipStatus );

            /* Validate the Request Method! */
            responseStatus = BIP_HttpResponseStatus_e501_NotImplemented;
            BIP_CHECK_GOTO( (method == BIP_HttpRequestMethod_eHead || method == BIP_HttpRequestMethod_eGet),
                    ( BIP_MSG_PRE_FMT " ERROR: method (%s) is not recognized/supported, only HEAD & GET are supported!" BIP_MSG_PRE_ARG, pAppStreamerCtx->pMethodName ),
                    rejectRequest, BIP_ERR_INVALID_REQUEST_TARGET, bipStatus );

            /* Retrieve the requested URL */
            bipStatus = BIP_HttpRequest_GetTarget( hHttpRequest, &pTmpUrlPath);
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetTarget Failed" ), rejectRequest, bipStatus, bipStatus );

            /* Parse the url and extract the query string parameters like Program and Pacing information. It also extracts and adds the target's realtive path in hTarget.*/
            bipStatus = parseUrl(pTmpUrlPath, pAppStreamerCtx, hTarget);

            /* Note: app may retrieve any private/custom app specific Headers from the Request using BIP_HttpRequest_GetHeader(). */

            /* Prefix the base media directory path to the URL name as this example is using the absolute file path for stream names so that it becomes unique! */

            /* Before we do that, this example also uses some additional file name extensions to indicate if client is request HLS, Xcode, or other stream variations. */
            /*
             * Here are few examples of the supported extensions:
             * -normal file:    AbcMpeg2HD.mpg
             * -xcoded file:    AbcMpeg2HD.mpg.xcode
             * -HLS file:       AbcMpeg2HD.mpg.m3u8
             * -MPEG DASH file: AbcMpeg2HD.mpg.mpd
             */
            {
                const char *pTmp;

                bipStatus = BIP_String_StrcpyPrintf( pAppStreamerCtx->hMediaFileAbsolutePathName, "%s%s", BIP_String_GetString(pAppCtx->hMediaDirectoryPath), BIP_String_GetString(hTarget));
                responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_StrcpyPrintf Failed" ), rejectRequest, bipStatus, bipStatus );

                /* Trim out the extension from the URL String. */
                if ( (pTmp = strstr( BIP_String_GetString( pAppStreamerCtx->hMediaFileAbsolutePathName), ".m3u8" )) != NULL )
                {
                    pAppStreamerCtx->enableHls = true;
                    bipStatus = BIP_String_Trim( pAppStreamerCtx->hMediaFileAbsolutePathName, pTmp, 0 );
                }
                else if ( (pTmp = strstr( BIP_String_GetString( pAppStreamerCtx->hMediaFileAbsolutePathName), ".mpd" )) != NULL )
                {
                    pAppStreamerCtx->enableMpegDash = true;
                    bipStatus = BIP_String_Trim( pAppStreamerCtx->hMediaFileAbsolutePathName, pTmp, 0 );
                }
                else
                {
                    if ( (pTmp = strstr( BIP_String_GetString( pAppStreamerCtx->hMediaFileAbsolutePathName), ".xcode" )) != NULL )
                    {
                        pAppStreamerCtx->enableXcode = true;
                        bipStatus = BIP_String_Trim( pAppStreamerCtx->hMediaFileAbsolutePathName, pTmp, strlen(".xcode") );
                        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_Trim Failed" ), rejectRequest, bipStatus, bipStatus );
                    }
                    if ( (pTmp = strstr( BIP_String_GetString( pAppStreamerCtx->hMediaFileAbsolutePathName), ".dtcpIp" )) != NULL )
                    {
                        pAppStreamerCtx->enableDtcpIp = true;
                        bipStatus = BIP_String_Trim( pAppStreamerCtx->hMediaFileAbsolutePathName, pTmp, strlen(".dtcpIp") );
                        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_Trim Failed" ), rejectRequest, bipStatus, bipStatus );
                    }
                    if ( (pTmp = strstr( BIP_String_GetString( pAppStreamerCtx->hMediaFileAbsolutePathName), ".tts" )) != NULL )
                    {
                        pAppStreamerCtx->enableTransportTimestamp = true;
                        bipStatus = BIP_String_Trim( pAppStreamerCtx->hMediaFileAbsolutePathName, pTmp, strlen(".tts") );
                        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_Trim Failed" ), rejectRequest, bipStatus, bipStatus );
                    }
                }
                responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_Trim Failed" ), rejectRequest, bipStatus, bipStatus );

                /* Now trim any query string from the Request Target until we support parsing & interpretting it. */
                if ( (pTmp = strstr( BIP_String_GetString( pAppStreamerCtx->hMediaFileAbsolutePathName), "?" )) != NULL )
                {
                    bipStatus = BIP_String_Trim( pAppStreamerCtx->hMediaFileAbsolutePathName, pTmp, 0 );
                    responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
                    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_Trim Failed" ), rejectRequest, bipStatus, bipStatus );
                }
            }
            pAppStreamerCtx->pMediaFileAbsolutePathName = BIP_String_GetString(pAppStreamerCtx->hMediaFileAbsolutePathName);

            bipStatus = BIP_String_StrcpyPrintf(hInfoFileAbsolutePathName, "%s%s.xml",
                                                                     BIP_String_GetString( pAppStreamerCtx->pAppCtx->hInfoDirectoryPath),
                                                                     pAppStreamerCtx->pMediaFileAbsolutePathName );
            responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_String_Trim Failed" ), rejectRequest, bipStatus, bipStatus );

            /* Now create the mediaInfo object.*/
            /* This will create the mediaInfo (meta data) xml file if they don't already exist.*/
            pAppStreamerCtx->hMediaInfo = BIP_MediaInfo_CreateFromMediaFile(
                                                pAppStreamerCtx->pMediaFileAbsolutePathName ,
                                                BIP_String_GetString(hInfoFileAbsolutePathName),
                                                NULL        /* Use default settings. */
                                                );
            responseStatus = BIP_HttpResponseStatus_e404_NotFound;/* We are assuming that BIP_MediaInfo_CreateFromMediaFile will only fail when file is not available.*/
            BIP_CHECK_GOTO( (pAppStreamerCtx->hMediaInfo != NULL), ("BIP_MediaInfo_CreateFromMediaFile() Failed"), rejectRequest, BIP_ERR_INTERNAL, bipStatus);
            /* We have a valid request. */
        }

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
        rejectRequestAndSetResponseHeaders( pAppStreamerCtx->pAppCtx, hHttpRequest, responseStatus );
        destroyAppStreamerCtx(pAppStreamerCtx);
        pAppStreamerCtx = NULL;
        /* continue back to the top of the loop. */
    } /* while */

error:
    if (hTarget)
    {
        BIP_String_Destroy(hTarget);
    }
    if (hInfoFileAbsolutePathName)
    {
        BIP_String_Destroy(hInfoFileAbsolutePathName);
    }
    if (pAppStreamerCtx)
    {
        if (pAppStreamerCtx->hHttpStreamer) stopAndDestroyStreamer( pAppStreamerCtx );
        destroyAppStreamerCtx(pAppStreamerCtx);
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

/* Function to generate Nav */
static BIP_Status generateNav(
    BIP_MediaInfoStream *pMediaInfoStream,
    const char *pInfoFilesDirectoryPath,
    const char *pMediaFileAbsolutePathName
    )
{
    BIP_MediaInfoTrack      *pMediaInfoTrack = NULL;
    BIP_StringHandle        hAbsoluteNavFileName = NULL;
    BIP_MediaInfoMakeNavForTsSettings  navForTsSetting;
    BIP_Status rc = BIP_SUCCESS;

    /* Generate nav for all video tracks present in the stream.*/
    pMediaInfoTrack = pMediaInfoStream->pFirstTrackInfoForStream;
    while(pMediaInfoTrack)
    {
        /* All these conditions has to be satisfied before we try to generate the nav file.*/
        if((BIP_MediaInfoTrackType_eVideo == pMediaInfoTrack->trackType)
            && ( pMediaInfoTrack->info.video.codec != NEXUS_VideoCodec_eH264_Svc )
            && ( pMediaInfoTrack->info.video.codec != NEXUS_VideoCodec_eH264_Mvc )
            && ( pMediaInfoTrack->parsedPayload )
            )
        {
            BIP_Status nv_status;
            /*  We just add here _trackId.nv extension for the file name.*/
            hAbsoluteNavFileName = buildNavFileName( pInfoFilesDirectoryPath, pMediaFileAbsolutePathName, pMediaInfoTrack->trackId );
            BIP_CHECK_GOTO(( hAbsoluteNavFileName ), ( "%s: BIP_String_StrcpyPrintf Failed to create nav file file=%s trackId=%d", BSTD_FUNCTION, pMediaFileAbsolutePathName, pMediaInfoTrack->trackId ), error, rc, rc );

            BIP_MediaInfo_GetDefaultMakeNavForTsSettings(&navForTsSetting);

            nv_status = BIP_MediaInfo_MakeNavForTsFile(
                    pMediaFileAbsolutePathName,
                    BIP_String_GetString(hAbsoluteNavFileName),
                    pMediaInfoTrack->trackId,
                    &navForTsSetting
                    );
            if (hAbsoluteNavFileName)
            {
                BIP_String_Destroy(hAbsoluteNavFileName);
                hAbsoluteNavFileName = NULL;
            }
            if(nv_status == BIP_SUCCESS) /*nav created Success*/
            {
                BDBG_MSG(("Nav successfull created for stream %s  and trackId = %d", pMediaFileAbsolutePathName, pMediaInfoTrack->trackId));
            }
            else   /* Nav Failed */
            {
                BDBG_WRN(("Failed to create nav file for stream  %s and trackId = %d", pMediaFileAbsolutePathName, pMediaInfoTrack->trackId));
            }
        }
        pMediaInfoTrack = pMediaInfoTrack->pNextTrackForStream;
    }

error:
    return rc;
}

/* Function to generate MediaInfo for a given media file */
static BIP_Status generateMediaInfo(
    const char *pMediaFileAbsolutePathname,
    const char *pInfoFilesDirectoryPath,
    bool disableTrickMode
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    BIP_StringHandle hInfoFileAbsolutePathName = NULL;
    BIP_MediaInfoHandle hMediaInfo = NULL;
    BIP_MediaInfoStream *pMediaInfoStream = NULL;

    hInfoFileAbsolutePathName = BIP_String_CreateFromPrintf( "%s%s.xml",pInfoFilesDirectoryPath,pMediaFileAbsolutePathname );
    BIP_CHECK_GOTO( (hInfoFileAbsolutePathName != NULL), ("BIP_String_CreateFromPrintf() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

    /* This will create the mediaInfo (meta data) xml file if they don't already exist.*/
    hMediaInfo = BIP_MediaInfo_CreateFromMediaFile(
                        pMediaFileAbsolutePathname,
                        BIP_String_GetString(hInfoFileAbsolutePathName),
                        NULL
                        );
    BIP_CHECK_GOTO( (hMediaInfo != NULL), ("BIP_MediaInfo_CreateFromMediaFile() Failed"), error, BIP_ERR_INTERNAL, bipStatus);

    pMediaInfoStream = BIP_MediaInfo_GetStream(hMediaInfo);
    BIP_CHECK_GOTO( (pMediaInfoStream != NULL), ("BIP_MediaInfo_GetStream() Failed"), error, BIP_ERR_INTERNAL, bipStatus);

    if((false == disableTrickMode)
       && (pMediaInfoStream->transportType == NEXUS_TransportType_eTs)
       )
    {
        bipStatus = generateNav(pMediaInfoStream,
                                pInfoFilesDirectoryPath,
                                pMediaFileAbsolutePathname
                                 );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "generateNav failed for %s", pMediaFileAbsolutePathname ), error, bipStatus, bipStatus );
    }

error:
    if(hInfoFileAbsolutePathName)
    {
        BIP_String_Destroy(hInfoFileAbsolutePathName);
    }
    if(hMediaInfo)
    {
        BIP_MediaInfo_Destroy(hMediaInfo);
    }
    return bipStatus;
} /* generateMediaInfo */

/* Function to filter out the files that shouldn't be used in metadata generation */
#include <dirent.h>
int fileNameFilter(
    const struct dirent *pDirEntry
    )
{
    if ( strstr(pDirEntry->d_name, ".nav") ||
         strstr(pDirEntry->d_name, ".info") ||
         strstr(pDirEntry->d_name, ".nfo") ||
         strstr(pDirEntry->d_name, ".pcm") ||
         strstr(pDirEntry->d_name, ".htm") ||
         strstr(pDirEntry->d_name, ".jpg") ||
         !strcmp(pDirEntry->d_name, ".") ||
         !strcmp(pDirEntry->d_name, "..")
       )
    {
        BDBG_MSG(("Ignoring file (%s) from media generation", pDirEntry->d_name));
        return (0);
    }
    else if (
            pDirEntry->d_type == DT_DIR ||
            pDirEntry->d_type == DT_REG
            )
    {
        /* Select only directory type or regular files */
        return (1);
    }
    else
    {
        BDBG_MSG(("Ignoring file (%s) from media generation", pDirEntry->d_name));
        return (0);
    }
} /* fileNameFilter */

/* Scan Media Directory and generate MediaInfo for each file. */
/* Note: This function can be run in a separate thread context since */
/* first time generation of the metadata consumes more time and */
/* this can be done parallel to other app initialization work. */
static BIP_Status generateMediaInfoForFiles(
    const char *pMediaFilesDirectoryPath,
    const char *pInfoFilesDirectoryPath,
    bool disableTrickMode
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    struct dirent **namelist = NULL;
    int numDirEntries, i;
    BIP_StringHandle hMediaPath = NULL; /* pointer to array of media Directory */

    numDirEntries = scandir( pMediaFilesDirectoryPath, &namelist, fileNameFilter, NULL );
    BIP_CHECK_GOTO(( numDirEntries > 0 ), ( "%s: Failed to scan media directory %s, numDirEntries %d, errno %d, errnoString: %s",
                BSTD_FUNCTION, pMediaFilesDirectoryPath, numDirEntries, errno, strerror(errno) ), error, BIP_ERR_INTERNAL, bipStatus );

    hMediaPath = BIP_String_Create();
    BIP_CHECK_GOTO( (hMediaPath != NULL), ("BIP_String_Create() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

    for (i=0; i < numDirEntries; i++)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT " DirEntry Info: name %s, type %d" BIP_MSG_PRE_ARG, namelist[i]->d_name, namelist[i]->d_type));
        if (namelist[i]->d_type == DT_DIR)
        {
            BIP_CHECK_GOTO( (hMediaPath), ("BIP_String_Create() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

            bipStatus = BIP_String_StrcpyPrintf( hMediaPath, "%s/%s", pMediaFilesDirectoryPath, namelist[i]->d_name);
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "StrcpyPrintf failed to form  %s%s/", pMediaFilesDirectoryPath, namelist[i]->d_name ), error, bipStatus, bipStatus );
            BDBG_MSG(("Recursive call to generateMediaInfoForFiles for %s", namelist[i]->d_name ));
            bipStatus = generateMediaInfoForFiles( BIP_String_GetString(hMediaPath), pInfoFilesDirectoryPath, disableTrickMode );
        }
        else
        {
            /*
             * Generate media info for regular files.
             * Build a unique streamName. One way to make a unique name is to use the absolute file path as the stream name!
             * This way even though two different directores can have media with the same name,
             * but since their absolute paths are different, each file will have unique streamName.
             *
             * Note: metadata generation only happens during the very 1st run.
             */
            BIP_StringHandle hMediaFileAbsolutePathName = NULL;
            const char *pStreamName;

            /* Prepare the absolute file name. */
            hMediaFileAbsolutePathName = BIP_String_CreateFromPrintf(
                    "%s/%s",
                    pMediaFilesDirectoryPath,
                    namelist[i]->d_name
                    );
            BIP_CHECK_GOTO( (hMediaFileAbsolutePathName != NULL), ("BIP_String_CreateFromPrintf() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

            /* NOTE: this example is using the absolute file path for stream name as that is one way to assign a unique stream name for each file name! */
            pStreamName = BIP_String_GetString( hMediaFileAbsolutePathName );

            bipStatus = generateMediaInfo( BIP_String_GetString( hMediaFileAbsolutePathName ), pInfoFilesDirectoryPath, disableTrickMode );

            if(bipStatus != BIP_SUCCESS)
            {
                BDBG_WRN(( BIP_MSG_PRE_FMT " generateMediaInfo Failed for stream: %s"BIP_MSG_PRE_ARG, pStreamName));
            }

            BIP_String_Destroy( hMediaFileAbsolutePathName );
        }
    }
    bipStatus = BIP_SUCCESS;

error:
    if (hMediaPath)
    {
        BIP_String_Destroy(hMediaPath);
        hMediaPath = NULL;
    }
    if (namelist)
    {
        for (i=numDirEntries-1; i >= 0; i--)
        {
            if (namelist[i])
            {
                free(namelist[i]);
            }
        }
        free(namelist);
    }

    return ( bipStatus );
} /* generateMediaInfoForFiles */

static void unInitHttpServer(
    AppCtx *pAppCtx
    )
{
    AppStreamerCtx *pAppStreamerCtx;

    if (!pAppCtx)
        return;
    BDBG_MSG(( BIP_MSG_PRE_FMT " appCtx %p" BIP_MSG_PRE_ARG, (void *)pAppCtx ));

    /* Stop the Server first, so that it doesn't accept any new connections. */
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
        destroyAppStreamerCtx(pAppStreamerCtx);
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
        BIP_CHECK_GOTO(( pAppCtx->hHttpEventGroup ), ( "Event Group Creation Failed" ), error, BIP_ERR_CREATE_FAILED, bipStatus );

        /* Create an event to indicate a callback on the listener */
        pAppCtx->hHttpRequestRcvdEvent = B_Event_Create( NULL );
        BIP_CHECK_GOTO(( pAppCtx->hHttpRequestRcvdEvent ), ( "Event Creation Failed" ), error, BIP_ERR_CREATE_FAILED, bipStatus );
        pAppCtx->maxTriggeredEvents++;
        /* Add this event to the event group */
        rc = B_EventGroup_AddEvent( pAppCtx->hHttpEventGroup, pAppCtx->hHttpRequestRcvdEvent );
        BIP_CHECK_GOTO(( !rc ), ( "Failed to Add event to group" ), error, BIP_ERR_INTERNAL, bipStatus );

        /* Create an event to indicate a callback on the session */
        pAppCtx->hHttpStreamerEvent = B_Event_Create( NULL );
        BIP_CHECK_GOTO(( pAppCtx->hHttpStreamerEvent ), ( "Event Creation Failed" ), error, BIP_ERR_CREATE_FAILED, bipStatus );
        pAppCtx->maxTriggeredEvents++;
        /* Add this event to the event group */
        rc = B_EventGroup_AddEvent( pAppCtx->hHttpEventGroup, pAppCtx->hHttpStreamerEvent );
        BIP_CHECK_GOTO(( !rc ), ( "Failed to Add event to group" ), error, BIP_ERR_INTERNAL, bipStatus );

        BIP_CHECK_GOTO( !( pAppCtx->maxTriggeredEvents > MAX_TRACKED_EVENTS ), ( "need to increase the MAX_TRACKED_EVENTS to %d", pAppCtx->maxTriggeredEvents ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    /* Create HTTP Server */
    {
        pAppCtx->hHttpServer = BIP_HttpServer_Create( NULL ); /* Use default settings. */
        BIP_CHECK_GOTO(( pAppCtx->hHttpServer ), ( "BIP_HttpServer_Create Failed" ), error, BIP_ERR_CREATE_FAILED, bipStatus );
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
        BDBG_MSG(("%s: Starting HttpServer...", BSTD_FUNCTION));

        httpServerStartSettings.pPort = BIP_String_GetString(pAppCtx->hPort);
        httpServerStartSettings.pInterfaceName = BIP_String_GetString(pAppCtx->hInterfaceName);
        if (pAppCtx->enableDtcpIp)
        {
            httpServerStartSettings.enableDtcpIp = true;
            BIP_DtcpIpServer_GetDefaultStartSettings( &httpServerStartSettings.dtcpIpServer );
            httpServerStartSettings.dtcpIpServer.pAkePort = BIP_String_GetString(pAppCtx->hDtcpIpAkePort);
            httpServerStartSettings.dtcpIpServer.enableHwOffload = pAppCtx->enableHwOffload;
            if (strcasecmp(BIP_String_GetString(pAppCtx->hDtcpIpKeyFormat), DTCP_IP_KEY_FORMAT_COMMON_DRM ) == 0)
            {
                httpServerStartSettings.dtcpIpServer.keyFormat = B_DTCP_KeyFormat_eCommonDRM;
            }
            else
            {
                httpServerStartSettings.dtcpIpServer.keyFormat = B_DTCP_KeyFormat_eTest;
            }
        }
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
            "  -port            #   Server Port (default is port 80\n"
            "  -interface       #   Optional Interface name to bind the server to\n"
            "  -mediaDir        #   Media Directory Path (default is /data/videos). Note: URL names is fileName path under this directory.\n"
            "  -infoDir         #   Info Files Directory Path (default is /data/info)\n"
            "  -stats           #   Print Periodic stats. \n"
            "  -loop            #   ContinousLoop after reach EOF\n"
          );
    printf(
            "  -trackGroupId    #   Use a particular program in MPTS case (defaults to 1st program)\n"
            "  -disableAudio    #   Disable sending audio track (defaults is enabled)\n"
          );
    printf(
            "  -disableTrickmode#   Disable trickmode support in server (defaults is enabled)\n"
            "                   #   NAV files are NOT generated during startup.\n"
            "  -dontAddAvInfo   #   Dont insert AV Track Info in the HTTP Response (default: Insert it)\n"
            "  -dontAddContentLength #   Dont insert HTTP Content-Length header in the HTTP Response (default: Insert it)\n"
            "                   #   Forces Client to determine AV info from either HTTP Response Headers or basic PAT/PMT parsing.\n"
            );
    printf(
            "  -dtcpIp          #   Start DTCP/IP Server\n"
            "  -akePort         #   DTCP/IP Ake Port# (default 8000)\n"
            "  -dtcpIpKeyFormat <keyFormat> # keyFormat values are: [commonDrm | test]. Default is commonDrm \n"
            "  -dtcpIpCci <cci> # CCI enum values as defined in dtcp_ip/include/b_dtcp_applib.h: CopyFree=0, CopyNever=3, etc.\n"
            "  -pcpPayloadLengthInBytes <num>    # Size of PCP payload in bytes. \n"
          );
    printf(
            "  -xcode           #   Xcode the input using XcodeProfile (default No xcode)\n"
            "  -xcodeProfile    #   Pre-defined xcode profile string: 720pAvc (default), 480pAvc, \n"
            "  .xcode           #   For enabling xcode. .e.g AbcMpeg2HD.mpg.xcode \n"
          );
    printf( "To enable some of the above options at runtime via the URL Request, add following suffix extension to the URL: \n");
    printf(
            "  .m3u8            #   For enabling HLS Streamer Protocol. e.g. AbcMpeg2HD.m3u8 \n"
            "  .dtcpIp          #   For enabling DTCP/IP Encryption. e.g. AbcMpeg2HD.dtcpIp \n"
            "  -hls             #   Enable HLS Output. \n"
          );
    printf(
            "  -inactivityTimeoutInMs <num> # Timeout in msec (default=60000) after which streamer will Stop/Close streaming context if there is no activity for that long!\n"
            "  -enableChunkXferEncoding     # Enable HTTP Chunk Transfer Encoding\n"
            "  -chunkSizeInBytes   <num>    # Size of each chunk in bytes. \n"
          );
    printf(
            "  -pace            #   Pace Streaming out using PCRs (stream is fed thru Playback Channel.\n"
            "                   #   Cann't be used if enableAllPass option is also used. Instead, use maxDataRate option.\n"
            "  -maxDataRate <num> # Maximum data rate for the playback parser band in units of bits per second (default 40000000 (40Mpbs))!\n"
            "                   #   Allows sender to stream out a certail user specified rate instead of pacing using PCRs.\n"
          );
    printf(
            "  -streamUsingPlaybackCh #   Enable streaming using Nexus Playback -> Recpump -> PBIP Streaming Path.\n"
            "                   #   Option is internally enabled if -pace or -maxDataRate options are used.\n"
            "  -enableAllPass   #   Enable streaming of all AV Tracks. \n"
            "                   #   By default, app will insert PAT, PMT, All Audio & Video Tracks for streaming.\n"
          );
    printf(
            "  -enableHwOffload #   Enable streaming using ASP HW Offload Engine. \n"
            "                   #   User can specify either -pace or -maxDataRate options to control the rate of Playback channel.\n"
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

    pAppCtx->chunkSizeInBytes = 192*5461; /* ~1MB */
    pAppCtx->pcpPayloadLengthInBytes = 192*5461; /* ~1MB */
    pAppCtx->dtcpIpCopyControlInfo = B_CCI_eCopyNever;
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
        else if ( !strcmp(argv[i], "-interface") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hInterfaceName, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-mediaDir") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hMediaDirectoryPath, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-infoDir") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hInfoDirectoryPath, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-pace") )
        {
            pAppCtx->enablePacing = true;
        }
        else if ( !strcmp(argv[i], "-trackGroupId") && i+1<argc )
        {
            pAppCtx->trackGroupId = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-inactivityTimeoutInMs") && i+1<argc )
        {
            pAppCtx->inactivityTimeoutInMs = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-maxDataRate") && i+1<argc )
        {
            pAppCtx->maxDataRate = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-loop") )
        {
            pAppCtx->enableContinousPlay = true;
        }
        else if ( !strcmp(argv[i], "-dtcpIp") )
        {
            pAppCtx->enableDtcpIp = true;
        }
        else if ( !strcmp(argv[i], "-akePort") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hDtcpIpAkePort, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-dtcpIpKeyFormat") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppCtx->hDtcpIpKeyFormat, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-slave") )
        {
            pAppCtx->enableSlaveMode = true;
        }
        else if ( !strcmp(argv[i], "-xcode") )
        {
            pAppCtx->enableXcode = true;
        }
        else if ( !strcmp(argv[i], "-xcodeProfile") )
        {
            pAppCtx->xcodeProfile = argv[++i];
        }
        else if ( !strcmp(argv[i], "-stats") )
        {
            pAppCtx->printStatus = true;
        }
        else if ( !strcmp(argv[i], "-hls") )
        {
            pAppCtx->enableHls = true;
            pAppCtx->enableXcode = true;
        }
        else if(!strcmp(argv[i], "-disableTrickmode"))
        {
            pAppCtx->disableTrickmode = true;
        }
        else if(!strcmp(argv[i], "-disableAudio"))
        {
            pAppCtx->disableAudio = true;
        }
        else if ( !strcmp(argv[i], "-dontAddAvInfo") )
        {
            pAppCtx->disableAvHeadersInsertion = true;
        }
        else if ( !strcmp(argv[i], "-dontAddContentLength") )
        {
            pAppCtx->disableContentLengthInsertion = true;
        }
        else if ( !strcmp(argv[i], "-enableAllPass") )
        {
            pAppCtx->enableAllPass = true;
        }
        else if ( !strcmp(argv[i], "-enableHwOffload") )
        {
            pAppCtx->enableHwOffload = true;
        }
        else if ( !strcmp(argv[i], "-streamUsingPlaybackCh") )
        {
            pAppCtx->enableStreamingUsingPlaybackCh = true;
        }
        else if ( !strcmp(argv[i], "-enableChunkXferEncoding") )
        {
            pAppCtx->enableHttpChunkXferEncoding = true;
        }
        else if ( !strcmp(argv[i], "-chunkSizeInBytes") && i+1<argc )
        {
            pAppCtx->chunkSizeInBytes = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-pcpPayloadLengthInBytes") && i+1<argc )
        {
            pAppCtx->pcpPayloadLengthInBytes = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-dtcpIpCci") && i+1<argc )
        {
            pAppCtx->dtcpIpCopyControlInfo = strtoul(argv[++i], NULL, 0);
        }
        else
        {
            printf("Error: incorrect or unsupported option: %s\n", argv[i]);
            printUsage(argv[0]);
        }
    }

    if (pAppCtx->enablePacing && pAppCtx->enableAllPass)
    {
        printf("Error: incorrect options: can't specify -pace along with -enableAllPass option, use -maxDataRate instead of -pace if -enableAllPass needs to be enabled\n");
        printUsage(argv[0]);
        return (bipStatus);
    }

    if ( !pAppCtx->xcodeProfile ) pAppCtx->xcodeProfile = TRANSCODE_PROFILE_720p_AVC;
    if (pAppCtx->inactivityTimeoutInMs == 0) pAppCtx->inactivityTimeoutInMs = 60000; /* 60sec default. */
    bipStatus = BIP_SUCCESS;
    BDBG_LOG(( BIP_MSG_PRE_FMT " port %s, interface %s, mediaDir %s, infoDir %s, DTCP/IP %s: AKE Port %s, dtcpIpKeyFormat=%s Xcode %s, trackGroupId %d, inactivityTimeoutInMs=%u, dontAddContentLength=%s maxDataRate=%u httpChunking=%s CCI=0x%x dtcpPcpPayloadSize=%u httpChunkSize=%u" BIP_MSG_PRE_ARG,
                BIP_String_GetString( pAppCtx->hPort ),
                BIP_String_GetString( pAppCtx->hInterfaceName ),
                BIP_String_GetString( pAppCtx->hMediaDirectoryPath ),
                BIP_String_GetString( pAppCtx->hInfoDirectoryPath ),
                pAppCtx->enableDtcpIp ? "Y":"N",
                BIP_String_GetString( pAppCtx->hDtcpIpAkePort ),
                BIP_String_GetString( pAppCtx->hDtcpIpKeyFormat ),
                pAppCtx->enableXcode ? pAppCtx->xcodeProfile : "N",
                pAppCtx->trackGroupId,
                pAppCtx->inactivityTimeoutInMs,
                pAppCtx->disableContentLengthInsertion ? "Y":"N",
                pAppCtx->maxDataRate,
                pAppCtx->enableHttpChunkXferEncoding ? "Y":"N",
                pAppCtx->dtcpIpCopyControlInfo,
                pAppCtx->pcpPayloadLengthInBytes,
                pAppCtx->chunkSizeInBytes
             ));
    return ( bipStatus );
} /* parseOptions */

void unInitAppCtx(
    AppCtx *pAppCtx
    )
{
    if (!pAppCtx) return;
    if (pAppCtx->hPort) BIP_String_Destroy( pAppCtx->hPort);
    if (pAppCtx->hDtcpIpAkePort) BIP_String_Destroy( pAppCtx->hDtcpIpAkePort);
    if (pAppCtx->hInterfaceName) BIP_String_Destroy( pAppCtx->hInterfaceName);
    if (pAppCtx->hMediaDirectoryPath) BIP_String_Destroy( pAppCtx->hMediaDirectoryPath);
    if (pAppCtx->hInfoDirectoryPath) BIP_String_Destroy( pAppCtx->hInfoDirectoryPath);
    if (pAppCtx->hDtcpIpKeyFormat) BIP_String_Destroy( pAppCtx->hDtcpIpKeyFormat);
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
    BIP_CHECK_GOTO( (pAppCtx->hPort), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

    /* Setup default values for global App settings */
    pAppCtx->hDtcpIpAkePort = BIP_String_CreateFromChar( HTTP_SERVER_DTCP_IP_AKE_PORT_STRING );
    BIP_CHECK_GOTO( (pAppCtx->hDtcpIpAkePort), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

    pAppCtx->hInterfaceName = BIP_String_Create();
    BIP_CHECK_GOTO( (pAppCtx->hInterfaceName), ("BIP_String_Create() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

    pAppCtx->hMediaDirectoryPath = BIP_String_CreateFromChar( HTTP_SERVER_MEDIA_DIRECTORY_PATH );
    BIP_CHECK_GOTO( (pAppCtx->hMediaDirectoryPath), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

    pAppCtx->hDtcpIpKeyFormat = BIP_String_CreateFromChar(DTCP_IP_KEY_FORMAT_COMMON_DRM);
    BIP_CHECK_GOTO( (pAppCtx->hDtcpIpKeyFormat), ("BIP_String_Create() Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    pAppCtx->hInfoDirectoryPath = BIP_String_CreateFromChar( HTTP_SERVER_INFO_DIRECTORY_PATH );
    BIP_CHECK_GOTO( (pAppCtx->hInfoDirectoryPath), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

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
    BIP_CHECK_GOTO(( pAppCtx ), ( "initAppCtx Failed" ), errorBipInit, BIP_ERR_CREATE_FAILED, bipStatus );

    /* Parse commandline options */
    bipStatus = parseOptions( argc, argv, pAppCtx );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "parseOptions Failed" ), errorParseOptions, bipStatus, bipStatus );

    BDBG_MSG((BIP_MSG_PRE_FMT"http_file_streamer started ------------------->"BIP_MSG_PRE_ARG));

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
    {
        NEXUS_PlatformSettings platformSettings;

        if (pAppCtx->enableSlaveMode)
        {
            nrc = NEXUS_Platform_AuthenticatedJoin(NULL);
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_Join Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
        }
        else
        {
            NEXUS_PlatformStartServerSettings serverSettings;
            NEXUS_PlatformConfiguration platformConfig;

            NEXUS_Platform_GetDefaultSettings(&platformSettings);

            /* Increase Nexus' MAIN heap to allow for 32 ASP channels...
             * 32 channels * 3 MB per channel = 96 MB  */
            platformSettings.heap[0].size += 96 *1024*1024;
            platformSettings.fileModuleSettings.maxQueuedElements = 100;
            platformSettings.fileModuleSettings.workerThreads = NEXUS_FILE_MAX_IOWORKERS;

            platformSettings.mode = NEXUS_ClientMode_eVerified;
            /* Due to latest SAGE restrictions EXPORT_HEAP needs to be initialized even if we are not using SVP/EXPORT_HEAP(XRR).
             * It could be any small size heap.
             * Configure export heap since it's not allocated by nexus by default */
            platformSettings.heap[NEXUS_EXPORT_HEAP].size = 16*1024*1024;
            nrc = NEXUS_Platform_Init(&platformSettings);
            if (nrc == NEXUS_OUT_OF_DEVICE_MEMORY)
            {
                BDBG_ERR((BIP_MSG_PRE_FMT""BIP_MSG_PRE_ARG));
                BDBG_ERR((BIP_MSG_PRE_FMT"               IMPORTANT!"BIP_MSG_PRE_ARG));
                BDBG_ERR((BIP_MSG_PRE_FMT"Please check that your bmem settings of the Linux boot parameters meet the values suggested above"BIP_MSG_PRE_ARG));
            }
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_Init Failed" ), error, nrc, bipStatus );

            NEXUS_Platform_GetConfiguration(&platformConfig);

            NEXUS_Platform_GetDefaultStartServerSettings(&serverSettings);
            serverSettings.allowUnauthenticatedClients = true; /* client is written this way */
            serverSettings.unauthenticatedConfiguration.mode = NEXUS_ClientMode_eVerified;
            serverSettings.unauthenticatedConfiguration.heap[1] = platformConfig.heap[0]; /* for purposes of example, allow access to main heap */
            nrc = NEXUS_Platform_StartServer(&serverSettings);
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_StartServer Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
        }
    }
#endif /* NXCLIENT_SUPPORT */

    /* Generate MediaInfo for media files */
    bipStatus = generateMediaInfoForFiles( BIP_String_GetString( pAppCtx->hMediaDirectoryPath), BIP_String_GetString( pAppCtx->hInfoDirectoryPath), pAppCtx->disableTrickmode );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "generateMediaInfoForFiles Failed " ), error, bipStatus, bipStatus );

    /* Initialize HttpServer */
    bipStatus = initHttpServer( pAppCtx );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "initHttpServer Failed" ), error, bipStatus, bipStatus );

    /*
     * At this time, BIP has started listening for incoming requests from clients.
     * When new request comes from client, app gets the callback from BIP.
     * Callback function sends event to allow this main thread to process the events.
     */

    /* Wait on Event group and process events as they come */
    BDBG_LOG(( BIP_MSG_PRE_FMT
                " Type 'q' followed by ENTER to exit gracefully\n"
                " Type 's' followed by ENTER to print stats\n"
                BIP_MSG_PRE_ARG ));
    while ( true )
    {
        unsigned      userInput;
        unsigned      i;
        unsigned      numTriggeredEvents;
        B_EventHandle triggeredEvents[MAX_TRACKED_EVENTS];

        if ( (userInput = readInput()) == 'q' ) break;

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
        if ( userInput == 's' || pAppCtx->printStatus )
        {
            AppStreamerCtx *pAppStreamerCtx;
            for ( pAppStreamerCtx = BLST_Q_FIRST(&pAppCtx->streamerListHead);
                  pAppStreamerCtx;
                  pAppStreamerCtx = BLST_Q_NEXT( pAppStreamerCtx, streamerListNext ) )
            {
                BIP_HttpStreamer_PrintStatus( pAppStreamerCtx->hHttpStreamer );
            }
        }
    }

    BDBG_LOG(( BIP_MSG_PRE_FMT " Shutting HttpServer down..." BIP_MSG_PRE_ARG ));

error:
    unInitHttpServer( pAppCtx );
#if NXCLIENT_SUPPORT
    NxClient_Uninit();
#else /* !NXCLIENT_SUPPORT */
    if (!pAppCtx->enableSlaveMode)
        NEXUS_Platform_StopServer();
    NEXUS_Platform_Uninit();
#endif /* NXCLIENT_SUPPORT */
errorParseOptions:
    unInitAppCtx( pAppCtx );
errorBipInit:
    BIP_Uninit();

    BDBG_LOG(( "All done!" ));
    return (0); /* main */
}
