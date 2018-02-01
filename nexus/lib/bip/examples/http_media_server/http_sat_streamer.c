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
#include "blst_queue.h"
#include "bip.h"

#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_platform_client.h"
#else
#include "nexus_platform.h"
#endif
#include "nexus_frontend.h"
#include "nexus_parser_band.h"
#include "namevalue.h"

BDBG_MODULE( http_sat_server );

#define HTTP_SERVER_PORT_STRING "80"
#define HTTP_SERVER_INFO_DIRECTORY_PATH  "/data/info"
#define MAX_TRACKED_EVENTS 2

/* SAT Tuner specific defines */
/* Settings works with Alitronika in San Diego lab */
/* the following define the input and its characteristics -- these will vary by input type */
#define HTTP_SERVER_SAT_FREQ            1119000000
#define HTTP_SERVER_SAT_MODE            NEXUS_FrontendSatelliteMode_eDvb
#define HTTP_SERVER_SAT_SYM_RATE        20000000
#define HTTP_SERVER_TRANSPORT_TYPE      NEXUS_TransportType_eTs
#define HTTP_SERVER_SAT_TONE_ENABLED    true
#define HTTP_SERVER_SAT_TONE_MODE       NEXUS_FrontendDiseqcToneMode_eEnvelope
#define HTTP_SERVER_SAT_VOLTAGE         NEXUS_FrontendDiseqcVoltage_e13v

bool diseqcSupport = false;

#define TRANSCODE_PROFILE_720p_AVC "720pAvc"
#define TRANSCODE_PROFILE_480p_AVC "480pAvc"

typedef struct AppCtx
{
    BIP_StringHandle            hPort;
    BIP_StringHandle            hInfoDirectoryPath;
    BIP_MediaInfoHandle         hMediaInfo;
    unsigned                    adc;
    unsigned                    frequency;
    unsigned                    mode;
    unsigned                    symbolRate;
    bool                        toneEnabled;
    NEXUS_FrontendDiseqcVoltage diseqcVoltage;
    NEXUS_FrontendSatelliteNetworkSpec satNetworkSpec;
    bool                        httpServerStarted;
    BIP_HttpServerHandle        hHttpServer;
    B_EventGroupHandle          hHttpEventGroup;
    B_EventHandle               hHttpRequestRcvdEvent;
    B_EventHandle               hHttpStreamerEvent;
    unsigned                    trackGroupId;       /* For MPEG2-TS this specifies the program_number. */
    unsigned                    inactivityTimeoutInMs;
    int                         maxTriggeredEvents;
    bool                        enableDtcpIp;
    bool                        slaveModeEnabled;
    bool                        enableXcode;
    char                        *xcodeProfile;
    bool                        disableAvHeadersInsertion;
    bool                        enableAllPass;
    bool                        enableHwOffload;
    unsigned                    maxDataRate;
    unsigned                    sourceInputRate;
    bool                        printStatus;
    bool                        mtsif;
    BLST_Q_HEAD( streamerListHead, AppStreamerCtx ) streamerListHead; /* List of Streamer Ctx */
} AppCtx;

typedef struct AppStreamerCtx
{
    AppCtx                      *pAppCtx;
    bool                        endOfStreamerRcvd;  /* We have received End of Streaming event for this Streamer. */
    const char                  *pUrlPath;          /* Cached URL */
    const char                  *pMethodName;       /* HTTP Request Method Name */
    BIP_HttpStreamerHandle      hHttpStreamer;      /* Cached Streamer handle */
    BIP_StreamerStreamInfo      streamerStreamInfo; /* StreamerStreamInfo structure will be populated from mediaInfo structure received from hMediaInfo object.*/
    BLST_Q_ENTRY( AppStreamerCtx ) streamerListNext;    /* Next Streamer Ctx */
    NEXUS_ParserBand            parserBand;
    NEXUS_FrontendHandle        hFrontend;
    bool                        enableDtcpIp;
    bool                        enableHls;
    bool                        enableMpegDash;
    bool                        enableXcode;
    char                        *xcodeProfile;
    bool                        enableTransportTimestamp;
    bool                        enableAllPass;
    bool                        enableHwOffload;
    unsigned                    maxDataRate;
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

void untuneAndReleaseFrontend(
    NEXUS_FrontendHandle hFrontend,
    NEXUS_ParserBand parserBand
    )
{
    if (hFrontend)
    {
        NEXUS_Frontend_Untune( hFrontend );
        NEXUS_Frontend_Release( hFrontend );
    }

    if (parserBand != NEXUS_ParserBand_eInvalid) NEXUS_ParserBand_Close( parserBand );

    BDBG_MSG(( BIP_MSG_PRE_FMT " Done" BIP_MSG_PRE_ARG ));
} /* untuneAndReleaseFrontend */

BIP_Status acquireAndTuneFrontend(
    AppCtx *pAppCtx,
    NEXUS_FrontendHandle *phFrontend,
    NEXUS_ParserBand *pParserBand
    )
{
    NEXUS_Error                   rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration   platformConfig;
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_FrontendCapabilities    capabilities;
    NEXUS_FrontendUserParameters  userParams;
    NEXUS_ParserBandSettings      parserBandSettings;
    NEXUS_FrontendHandle          hFrontend;
    NEXUS_ParserBand              parserBand = NEXUS_ParserBand_eInvalid;
    NEXUS_FrontendSatelliteSettings satSettings;       /* SAT Settings */
    NEXUS_FrontendDiseqcSettings    diseqcSettings;    /* DiseqC Settings */

    NEXUS_Platform_GetConfiguration( &platformConfig );
    NEXUS_Frontend_GetDefaultAcquireSettings( &settings );
    settings.capabilities.satellite = true;
    hFrontend        = NEXUS_Frontend_Acquire( &settings );
    BIP_CHECK_GOTO(( hFrontend ), ( "NEXUS_Frontend_Acquire Failed" ), error, NEXUS_NOT_INITIALIZED, rc );

    parserBand = NEXUS_ParserBand_Open( NEXUS_ANY_ID );
    BIP_CHECK_GOTO(( parserBand ), ( "NEXUS_ParserBand_Open Failed" ), error, NEXUS_NOT_INITIALIZED, rc );

    BDBG_MSG(( BIP_MSG_PRE_FMT " hFrontend & Parserband are opened!" BIP_MSG_PRE_ARG ));

    {
        NEXUS_FrontendSatelliteRuntimeSettings runtimeSettings;
        NEXUS_Frontend_GetSatelliteRuntimeSettings(hFrontend, &runtimeSettings);
        runtimeSettings.selectedAdc = pAppCtx->adc;
        NEXUS_Frontend_SetSatelliteRuntimeSettings(hFrontend, &runtimeSettings);
    }

    NEXUS_Frontend_GetDefaultSatelliteSettings( &satSettings );
    satSettings.frequency = pAppCtx->frequency;
    satSettings.mode      = pAppCtx->mode;
    satSettings.symbolRate = pAppCtx->symbolRate;
#if 0
    satSettings.lockCallback.callback = lock_callback;
    satSettings.lockCallback.context  = pAppStreamerCtx;
#endif
    NEXUS_Frontend_GetUserParameters( hFrontend, &userParams );
    NEXUS_Frontend_GetCapabilities( hFrontend, &capabilities );
    diseqcSupport = capabilities.diseqc;

    NEXUS_ParserBand_GetSettings( parserBand, &parserBandSettings );

    if(pAppCtx->mtsif)
    {
        userParams.isMtsif = true;
    }

    if (userParams.isMtsif)
    {
        parserBandSettings.sourceType               = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector( hFrontend );
    }
    else
    {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }

    if (pAppCtx->enableAllPass)
    {
       parserBandSettings.allPass = true;
       parserBandSettings.acceptNullPackets = true;
    }

    parserBandSettings.transportType = HTTP_SERVER_TRANSPORT_TYPE;
    rc = NEXUS_ParserBand_SetSettings( parserBand, &parserBandSettings );
    BIP_CHECK_GOTO(( !rc ), ( "NEXUS_ParserBand_SetSettings Failed" ), error, rc, rc );

    if (diseqcSupport)
    {
        NEXUS_Frontend_GetDiseqcSettings( hFrontend, &diseqcSettings );

        diseqcSettings.toneEnabled = pAppCtx->toneEnabled;
        diseqcSettings.toneMode    = HTTP_SERVER_SAT_TONE_MODE;
        diseqcSettings.voltage     = pAppCtx->diseqcVoltage;

        rc = NEXUS_Frontend_SetDiseqcSettings( hFrontend, &diseqcSettings );
        BIP_CHECK_GOTO(( !rc ), ( "NEXUS_Frontend_SetDiseqcSettings Failed" ), error, rc, rc );
    }

    rc = NEXUS_Frontend_TuneSatellite( hFrontend, &satSettings );
    BIP_CHECK_GOTO(( !rc ), ( "NEXUS_Frontend_TuneSatellite Failed" ), error, rc, rc );

    BDBG_MSG(( BIP_MSG_PRE_FMT " Success: hFrontend (%p)" BIP_MSG_PRE_ARG, (void *)hFrontend));
    *phFrontend = hFrontend;
    *pParserBand = parserBand;
    return (BIP_SUCCESS);

error:
    untuneAndReleaseFrontend( hFrontend, parserBand );
    return ( rc );
} /* acquireAndTuneFrontend */

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
    BDBG_MSG(( BIP_MSG_PRE_FMT " Stopping Streamer %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer));
    BIP_HttpServer_StopStreamer( pAppStreamerCtx->pAppCtx->hHttpServer, pAppStreamerCtx->hHttpStreamer );

    /* Release Frontend resources */
    untuneAndReleaseFrontend( pAppStreamerCtx->hFrontend, pAppStreamerCtx->parserBand );

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
    BIP_HttpResponseStatus responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
    BIP_MediaInfoStream *pMediaInfoStream = NULL;

    BDBG_ASSERT(pAppStreamerCtx->pAppCtx->hMediaInfo);
    /* Create HttpStreamer. */
    {
        BIP_HttpServerCreateStreamerSettings createStreamerSettings;

        BIP_HttpServer_GetDefaultCreateStreamerSettings( &createStreamerSettings );
        createStreamerSettings.endOfStreamingCallback.callback = endOfStreamCallbackFromBip;
        createStreamerSettings.endOfStreamingCallback.context = pAppStreamerCtx;
        pAppStreamerCtx->hHttpStreamer = BIP_HttpServer_CreateStreamer( pAppStreamerCtx->pAppCtx->hHttpServer, &createStreamerSettings );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( pAppStreamerCtx->hHttpStreamer ), ( "BIP_HttpServer_CreateStreamer Failed" ), rejectRequest, BIP_ERR_CREATE_FAILED, bipStatus );
        BDBG_MSG(( BIP_MSG_PRE_FMT " BIP_HttpStreamer created: %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hHttpStreamer ));
    }

    /* Get BIP_MediaInfoStream from hMediaInfo object.*/
    pMediaInfoStream = BIP_MediaInfo_GetStream(
                        pAppStreamerCtx->pAppCtx->hMediaInfo
                        );
    BIP_CHECK_GOTO( (pMediaInfoStream != NULL), ("BIP_MediaInfo_GetStream() Failed"), rejectRequest, BIP_ERR_INTERNAL, bipStatus);
    BIP_Streamer_GetStreamerStreamInfoFromMediaInfo( pMediaInfoStream,&(pAppStreamerCtx->streamerStreamInfo) );

    /* Now provide settings for media input source */
    {
        BIP_StreamerTunerInputSettings tunerInputSettings;

        /* Acquire & Tune to the frontend so that we can acquire MediaInfo */
        bipStatus = acquireAndTuneFrontend( pAppStreamerCtx->pAppCtx, &pAppStreamerCtx->hFrontend, &pAppStreamerCtx->parserBand );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "acquireAndTuneFrontend Failed" ), rejectRequest, bipStatus, bipStatus );

        BIP_Streamer_GetDefaultTunerInputSettings(&tunerInputSettings);

        if (pAppStreamerCtx->enableAllPass)
        {
           tunerInputSettings.enableAllPass = true;
        }

        bipStatus = BIP_HttpStreamer_SetTunerInputSettings( pAppStreamerCtx->hHttpStreamer, pAppStreamerCtx->parserBand, &(pAppStreamerCtx->streamerStreamInfo), &tunerInputSettings );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_SetTunerInputSettings Failed" ), rejectRequest, bipStatus, bipStatus );
    }

    /* Now specify the Tracks that should be added for streaming: done here for Tuner inputs */
    if ( !pAppStreamerCtx->enableAllPass )
    {
        BIP_MediaInfoTrackGroup *pMediaInfoTrackGroup = NULL;
        BIP_MediaInfoTrack      *pMediaInfoTrack = NULL;
        BIP_StreamerTrackInfo   streamerTrackInfo;
        bool                    trackGroupPresent = false;

        if(pMediaInfoStream->numberOfTrackGroups != 0)
        {
            trackGroupPresent = true;
            if( pAppStreamerCtx->pAppCtx->trackGroupId != 0)
            {
                /* Get Track Group by Id.*/
                 pMediaInfoTrackGroup = BIP_MediaInfo_GetTrackGroupById(  pAppStreamerCtx->pAppCtx->hMediaInfo, pAppStreamerCtx->pAppCtx->trackGroupId );
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
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_AddTrack Failed for PAT" ), rejectRequest, bipStatus, bipStatus );
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

        while(pMediaInfoTrack)
        {
            B_Os_Memset( &streamerTrackInfo, 0, sizeof( streamerTrackInfo ) );
            BIP_Streamer_GetStreamerTrackInfoFromMediaInfo(pMediaInfoTrack, &streamerTrackInfo );

            bipStatus = BIP_HttpStreamer_AddTrack( pAppStreamerCtx->hHttpStreamer, &streamerTrackInfo, NULL );
            responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_AddTrack Failed for track# %d", streamerTrackInfo.trackId ), rejectRequest, bipStatus, bipStatus );

            if(true == trackGroupPresent)
            {
                pMediaInfoTrack = pMediaInfoTrack->pNextTrackForTrackGroup;
            }
            else
            {
                pMediaInfoTrack = pMediaInfoTrack->pNextTrackForStream;
            }
        }
    }

    /* Check whether transcode and/dtcpip enabled through target request.*/
    {
        const char *pTmp;
        if ( strstr( pAppStreamerCtx->pUrlPath, ".xcode" ) )
        {
            pAppStreamerCtx->enableXcode = true;
        }
        else if ( strstr( pAppStreamerCtx->pUrlPath, ".dtcpIp" ) )
        {
            pAppStreamerCtx->enableDtcpIp = true;
        }
        else if ( strstr( pAppStreamerCtx->pUrlPath, ".m3u8" ) )
        {
            pAppStreamerCtx->enableHls = true;
            pAppStreamerCtx->enableXcode = true;
        }
        else if ( strstr( pAppStreamerCtx->pUrlPath, ".mpd" ) )
        {
            pAppStreamerCtx->enableMpegDash = true;
            pAppStreamerCtx->enableXcode = true;
        }
        else if ( (pTmp = strstr( pAppStreamerCtx->pUrlPath, ".tts" )) != NULL )
        {
            pAppStreamerCtx->enableTransportTimestamp = true;
        }
    }

    /* Provide the Streamer output settings */
    {
        BIP_HttpStreamerProtocol streamerProtocol;
        BIP_HttpStreamerOutputSettings streamerOutputSettings;

        BIP_HttpStreamer_GetDefaultOutputSettings( &streamerOutputSettings );
        if (pAppStreamerCtx->pAppCtx->enableDtcpIp)
        {
            streamerOutputSettings.enableDtcpIp = true;
            streamerOutputSettings.dtcpIpOutput.pcpPayloadLengthInBytes = (1024*1024);
            streamerOutputSettings.dtcpIpOutput.akeTimeoutInMs = 2000;
            streamerOutputSettings.dtcpIpOutput.copyControlInfo = B_CCI_eCopyNever;
        }

        if (pAppStreamerCtx->maxDataRate)
        {
            streamerOutputSettings.streamerSettings.maxDataRate = pAppStreamerCtx->maxDataRate;
        }
        streamerOutputSettings.streamerSettings.enableHwOffload = pAppStreamerCtx->enableHwOffload;


        streamerProtocol = pAppStreamerCtx->enableHls ? BIP_HttpStreamerProtocol_eHls : BIP_HttpStreamerProtocol_eDirect;
        streamerOutputSettings.streamerSettings.mpeg2Ts.enableTransportTimestamp = pAppStreamerCtx->enableTransportTimestamp;
        streamerOutputSettings.disableAvHeadersInsertion = pAppStreamerCtx->pAppCtx->disableAvHeadersInsertion;


        bipStatus = BIP_HttpStreamer_SetOutputSettings(pAppStreamerCtx->hHttpStreamer, streamerProtocol, &streamerOutputSettings);
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_SetOutputSettings Failed" ), rejectRequest, bipStatus, bipStatus );
    }

    /* Add Transcode Profile. */
    if ( pAppStreamerCtx->enableXcode )
    {
#if NEXUS_HAS_VIDEO_ENCODER
        BIP_TranscodeProfile transcodeProfile;

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

        bipStatus = BIP_HttpStreamer_AddTranscodeProfile( pAppStreamerCtx->hHttpStreamer, &transcodeProfile );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpStreamer_AddTranscodeProfile Failed" ), rejectRequest, bipStatus, bipStatus );
#else
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "NEXUS Video Transcode Feature is not supported for this platform" BIP_MSG_PRE_ARG ));
            goto rejectRequest;
        }
#endif
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
                ( "BIP_HttpStreamer_SetResponseHeader Failed: hHttpStreamer %p", (void *)pAppStreamerCtx->hHttpStreamer ), rejectRequest, bipStatus, bipStatus );

        /* App can add more custom headers here using this example above! */
    }

    /* And as a last step, start the HttpStreamer. */
    {
        BIP_HttpServerStartStreamerSettings startSettings;

        BIP_HttpServer_GetDefaultStartStreamerSettings( &startSettings );
        startSettings.streamerStartSettings.inactivityTimeoutInMs = pAppStreamerCtx->pAppCtx->inactivityTimeoutInMs;
        bipStatus = BIP_HttpServer_StartStreamer( pAppStreamerCtx->pAppCtx->hHttpServer, pAppStreamerCtx->hHttpStreamer, hHttpRequest, &startSettings );
        responseStatus = BIP_HttpResponseStatus_e500_InternalServerError;
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpServer_StartStreamer Failed: hHttpStreamer %p", (void *)pAppStreamerCtx->hHttpStreamer ), rejectRequest, bipStatus, bipStatus );
    }
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

            /* Copy Global Settings into Streamer instance. */
            pAppStreamerCtx->enableXcode = pAppCtx->enableXcode;
            pAppStreamerCtx->enableAllPass = pAppCtx->enableAllPass;
            pAppStreamerCtx->maxDataRate = pAppCtx->maxDataRate;
            pAppStreamerCtx->enableHwOffload = pAppCtx->enableHwOffload;
        }

        /* Process the received Request, start w/ the Method */
        {
            /* Note: app may retrieve various HTTP Headers from the Request using BIP_HttpRequest_GetHeader. */
            bipStatus = BIP_HttpRequest_GetMethod( hHttpRequest, &method, &pAppStreamerCtx->pMethodName );
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetMethod Failed" ), rejectRequest, bipStatus, bipStatus );

            /* Validate the Request Method! */
            responseStatus = BIP_HttpResponseStatus_e501_NotImplemented;
            BIP_CHECK_GOTO( (method == BIP_HttpRequestMethod_eHead || method == BIP_HttpRequestMethod_eGet),
                    ( BIP_MSG_PRE_FMT " ERROR: method (%s) is not recognized/supported, only HEAD & GET are supported!" BIP_MSG_PRE_ARG, pAppStreamerCtx->pMethodName ),
                    rejectRequest, BIP_ERR_INVALID_REQUEST_TARGET, bipStatus );

            /* Retrieve the requested URL */
            bipStatus = BIP_HttpRequest_GetTarget( hHttpRequest, &pAppStreamerCtx->pUrlPath );
            responseStatus = BIP_HttpResponseStatus_e400_BadRequest;
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetTarget Failed" ), rejectRequest, bipStatus, bipStatus );

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
static BIP_Status generateMediaInfoForTunerInput (
    AppCtx *pAppCtx
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    NEXUS_FrontendHandle hFrontend;
    NEXUS_ParserBand parserBand;
    BIP_StringHandle    hTunerInfoFileAbsolutePath = NULL;

    BIP_MediaInfoCreateSettings mediaInfoCreateSettings;
    BIP_MediaInfoStream *pMediaInfoStream = NULL;

    /* Acquire & Tune to the frontend so that we can acquire MediaInfo */
    bipStatus = acquireAndTuneFrontend( pAppCtx, &hFrontend, &parserBand );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "acquireAndTuneFrontend Failed" ), error, bipStatus, bipStatus );

    /* Build a unique info file name for Qam input */
    hTunerInfoFileAbsolutePath = BIP_String_CreateFromPrintf( "%s/Sat_Freq_%d.xml", BIP_String_GetString(pAppCtx->hInfoDirectoryPath), pAppCtx->frequency );
    BIP_CHECK_GOTO( (hTunerInfoFileAbsolutePath != NULL), ("BIP_String_CreateFromPrintf() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);
    BIP_MediaInfo_GetDefaultCreateSettings(&mediaInfoCreateSettings);
    mediaInfoCreateSettings.reAcquireInfo = true;

    /* If one doesn't want then they can Pass NULL instead of infoFilename path since we are anyway setting mediaInfoCreateSettings.reAcquireInfo = true.*/
    pAppCtx->hMediaInfo = BIP_MediaInfo_CreateFromParserBand(
                    parserBand,
                    BIP_String_GetString(hTunerInfoFileAbsolutePath),
                    &mediaInfoCreateSettings
                    );
    BIP_CHECK_GOTO( (pAppCtx->hMediaInfo != NULL), ("BIP_MediaInfo_CreateFromParserBand() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

    pMediaInfoStream = BIP_MediaInfo_GetStream(pAppCtx->hMediaInfo);
    BIP_CHECK_GOTO( (pMediaInfoStream != NULL), ("BIP_MediaInfo_GetStream() Failed"), error, BIP_ERR_INTERNAL, bipStatus);

    BDBG_MSG(( BIP_MSG_PRE_FMT " streamInfo: transportType %d, numberOfTrackGroups %d"
               BIP_MSG_PRE_ARG, pMediaInfoStream->transportType, pMediaInfoStream->numberOfTrackGroups));

    /* We have finished generating the MediaInfo, so untune and release frontend resources */
    untuneAndReleaseFrontend( hFrontend, parserBand );
    bipStatus = BIP_SUCCESS;

error:
    if(hTunerInfoFileAbsolutePath)
    {
        BIP_String_Destroy(hTunerInfoFileAbsolutePath);
    }
    return ( bipStatus );
} /* generateMediaInfoForTunerInput */

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

    if (pAppCtx->hMediaInfo){ BIP_MediaInfo_Destroy(pAppCtx->hMediaInfo); }
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
        httpServerStartSettings.pPort = BIP_String_GetString( pAppCtx->hPort );
        if (pAppCtx->enableDtcpIp)
        {
            httpServerStartSettings.enableDtcpIp = true;
            BIP_DtcpIpServer_GetDefaultStartSettings( &httpServerStartSettings.dtcpIpServer );
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
        "  --help or -h for help\n"
        "  -port        #   Server Port (default is port 80\n"
        "  -infoDir     #   Info Files Directory Path (default is /data/info)\n"
        "  -freq        #   frequency in MHz\n"
        "  -mode        #   dvb/dss/qpskldpc/8pskldpc/dvbs2 (default is dvb)\n"
        "  -dtcpIp      #   Start DTCP/IP Server\n"
        "  -trackGroupId    #   Use a particular program in MPTS case (defaults to 1st program)\n"
        "  -slave       #   Start Server in slave mode (Client in Nexus Multi-Process)\n"
        );
    printf( "To enable some of the above options at runtime via the URL Request, add following suffix extension to the URL: \n");
    printf(
            "  .xcode           #   For enabling xcode. .e.g AbcMpeg2HD.mpg.xcode \n"
            "  .m3u8            #   For enabling HLS Streamer Protocol. e.g. AbcMpeg2HD.m3u8 \n"
            "  .dtcpIp          #   For enabling DTCP/IP Encryption. e.g. AbcMpeg2HD.dtcpIp \n"
            "  -enableAllPass   #   Enable streaming of all AV Tracks. \n"
            "  -enableHwOffload #   Enable streaming using ASP HW Offload Engine. \n"
            "  -inactivityTimeoutInMs <num> # Timeout in msec (default=60000) after which streamer will Stop/Close streaming context if there is no activity for that long!"
          );
    printf( "  -maxDataRate  <num>          # Maximum data rate for the playback parser band in units of bits per second (default 40000000 (40Mpbs))! \n"
            "  -srcInputRate <num>          # This sets Maximum input rate taht can be handled by parser band. \n"
            "  -mtsif                       # This sets parserband input type as mtsif. \n"
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

    pAppCtx->maxDataRate = 40*1024*1024;
    pAppCtx->sourceInputRate = 45*1024*1024;
    pAppCtx->mtsif = false;
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
        else if (!strcmp(argv[i], "-adc") && i+1<argc) {
            pAppCtx->adc = atoi(argv[++i]);
        }
        else if ( !strcmp(argv[i], "-freq") && i+1<argc )
        {
            pAppCtx->frequency = 1000000 * strtoul(argv[++i], NULL, 0); /* converted to hz */
        }
        else if ( !strcmp(argv[i], "-mode") && i+1<argc )
        {
            pAppCtx->mode = lookup(g_satModeStrs, argv[++i]);
        }
        else if ( !strcasecmp(argv[i], "-trackGroupId") && i+1<argc )
        {
            pAppCtx->trackGroupId = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-voltage") && i+1<argc )
        {
            pAppCtx->diseqcVoltage = lookup(g_diseqcVoltageStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-sym") && i+1<argc) {
            pAppCtx->symbolRate = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-tone") && i+1<argc) {
            pAppCtx->toneEnabled = lookup(g_diseqcToneEnabledStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-ksyms") && i+1<argc) {
            pAppCtx->symbolRate = strtoul(argv[++i], NULL, 0) * 1000;
        }
        else if (!strcmp(argv[i], "-networkspec") && i+1<argc) {
            pAppCtx->satNetworkSpec = lookup(g_satNetworkSpecStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-kufreq") && i+1<argc) {
            unsigned freq = strtoul(argv[++i], NULL, 0);
            if (freq < 11700) {
                /* low band */
                freq -= 9750;
                pAppCtx->toneEnabled = 0;
            }
            else {
                /* high band */
                freq -= 10600;
                pAppCtx->toneEnabled = 1;
            }
            pAppCtx->frequency = freq * 1000000;
        }
        else if ( !strcmp(argv[i], "-dtcpIp") )
        {
            pAppCtx->enableDtcpIp = true;
        }
        else if ( !strcmp(argv[i], "-slave") )
        {
            pAppCtx->slaveModeEnabled = true;
        }
        else if ( !strcmp(argv[i], "-xcode") )
        {
            pAppCtx->enableXcode = true;
        }
        else if ( !strcmp(argv[i], "-xcodeProfile") )
        {
            pAppCtx->xcodeProfile = argv[++i];
        }
        else if ( !strcmp(argv[i], "-enableAllPass") )
        {
            pAppCtx->enableAllPass = true;
        }
        else if ( !strcmp(argv[i], "-enableHwOffload") )
        {
            pAppCtx->enableHwOffload = true;
        }
        else if ( !strcmp(argv[i], "-inactivityTimeoutInMs") && i+1<argc )
        {
            pAppCtx->inactivityTimeoutInMs = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-maxDataRate") && i+1<argc )
        {
            pAppCtx->maxDataRate = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-srcInputRate") && i+1<argc )
        {
            pAppCtx->sourceInputRate = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-stats") )
        {
            pAppCtx->printStatus = true;
        }
        else if ( !strcmp(argv[i], "-mtsif") )
        {
            pAppCtx->mtsif = true;
        }
        else
        {
            printf("Error: incorrect or unsupported option: %s\n", argv[i]);
            printUsage(argv[0]);
            exit(0);
        }
    }
    if ( !pAppCtx->xcodeProfile ) pAppCtx->xcodeProfile = TRANSCODE_PROFILE_720p_AVC;
    if ( pAppCtx->inactivityTimeoutInMs == 0 ) pAppCtx->inactivityTimeoutInMs = 60000; /* 60sec default. */
    bipStatus = BIP_SUCCESS;
    BDBG_LOG(( BIP_MSG_PRE_FMT " port %s, infoDir %s, frequency %u, mode %u, xcode=%s" BIP_MSG_PRE_ARG,
                BIP_String_GetString( pAppCtx->hPort ),
                BIP_String_GetString( pAppCtx->hInfoDirectoryPath ),
                pAppCtx->frequency,
                pAppCtx->mode,
                pAppCtx->enableXcode ? pAppCtx->xcodeProfile : "N"
             ));
    return ( bipStatus );
} /* parseOptions */

void unInitAppCtx(
    AppCtx *pAppCtx
    )
{
    if (!pAppCtx) return;
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
    BIP_CHECK_GOTO( (pAppCtx->hPort), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

    pAppCtx->hInfoDirectoryPath = BIP_String_CreateFromChar( HTTP_SERVER_INFO_DIRECTORY_PATH );
    BIP_CHECK_GOTO( (pAppCtx->hInfoDirectoryPath), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_CREATE_FAILED, bipStatus);

    pAppCtx->adc = 0;
    pAppCtx->frequency = HTTP_SERVER_SAT_FREQ;
    pAppCtx->mode = HTTP_SERVER_SAT_MODE;
    pAppCtx->symbolRate = HTTP_SERVER_SAT_SYM_RATE;
    pAppCtx->diseqcVoltage = HTTP_SERVER_SAT_VOLTAGE;
    pAppCtx->toneEnabled = HTTP_SERVER_SAT_TONE_ENABLED;
    pAppCtx->satNetworkSpec = NEXUS_FrontendSatelliteNetworkSpec_eDefault;

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

    /* Parse command line options */
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
    {
        NEXUS_PlatformSettings platformSettings;

        if (pAppCtx->slaveModeEnabled)
        {
            nrc = NEXUS_Platform_AuthenticatedJoin(NULL);
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_Join Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
#if 0
            /* Note: calling InitFrontend is causing HAB timeouts. Ignoring this for now! */
            /* http_qam_streamer must be started first if multiple streamers are being run! */
            nrc = NEXUS_Platform_InitFrontend();
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_InitFrontend Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
#endif
        }
        else
        {
            NEXUS_PlatformStartServerSettings serverSettings;
            NEXUS_PlatformConfiguration platformConfig;

            NEXUS_Platform_GetDefaultSettings(&platformSettings);
            platformSettings.mode = NEXUS_ClientMode_eVerified;
            platformSettings.openFrontend = true;
            /* Due to latest SAGE restrictions EXPORT_HEAP needs to be initialized even if we are not using SVP/EXPORT_HEAP(XRR).
              It could be any small size heap.            Configure export heap since it's not allocated by nexus by default */
            platformSettings.heap[NEXUS_EXPORT_HEAP].size = 32*1024;

            if (pAppCtx->sourceInputRate)
            {
               int i =0;
               for (i=0; i < NEXUS_MAX_PARSER_BANDS; i++)
               {
                  platformSettings.transportModuleSettings.maxDataRate.parserBand[i] = pAppCtx->sourceInputRate;
               }
            }

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
    }
#endif /* NXCLIENT_SUPPORT */

    /* Initialize HttpServer */
    bipStatus = initHttpServer( pAppCtx );
    BIP_CHECK_GOTO(( !bipStatus ), ( "initHttpServer Failed" ), error, bipStatus, bipStatus );

    /* Generate MediaInfo for this Tuner resource */
    bipStatus = generateMediaInfoForTunerInput( pAppCtx );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "generateMediaInfoForTunerInput Failed" ), error, bipStatus, bipStatus );

    /*
     * At this time, BIP has started listening for incoming requests from clients.
     * When new request comes from client, app gets the callback from BIP.
     * Callback function sends event to allow this main thread to process the events.
     */

    /* Wait on Event group and process events as they come */
    BDBG_LOG(( BIP_MSG_PRE_FMT " Type 'q' followed by ENTER to exit gracefully\n" BIP_MSG_PRE_ARG ));
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
    if (!pAppCtx->slaveModeEnabled)
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
