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
#include "nexus_frontend.h"
#include "nexus_parser_band.h"

BDBG_MODULE( udp_qam_streamer );

#define UDP_SERVER_PORT_STRING "1234"
#define UDP_SERVER_IP_STRING "224.1.1.10"
#define UDP_SERVER_INTERFACE_NAME_STRING "eth0"
#define UDP_SERVER_INFO_DIRECTORY_PATH  "/data/info"

/* QAM Tuner specific defines */
#define UDP_SERVER_QAM_FREQ            549000000
#define UDP_SERVER_QAM_MODE            NEXUS_FrontendQamMode_e256
#define UDP_SERVER_QAM_SYM_RATE        5360537
#define UDP_SERVER_TRANSPORT_TYPE      NEXUS_TransportType_eTs

#define ENCODER_PROFILE_720p_AVC "720pAvc"
#define ENCODER_PROFILE_480p_AVC "480pAvc"

typedef struct AppStreamerCtx
{
    BIP_StringHandle            hPort;
    BIP_StringHandle            hIpAddress;
    BIP_StringHandle            hInterfaceName;
    BIP_StringHandle            hInfoDirectoryPath;
    BIP_StringHandle            hStreamName;
    BIP_MediaInfoHandle         hMediaInfo;
    BIP_UdpStreamerProtocol     streamerProtocol;
    unsigned                    frequency;
    unsigned                    mode;
    unsigned                    symbolRate;
    B_EventHandle               hUdpStreamerEvent;
    bool                        started;            /* Streaming is started. */
    BIP_UdpStreamerHandle       hUdpStreamer;       /* Cached Streamer handle */
    NEXUS_ParserBand            parserBand;
    NEXUS_FrontendHandle        hFrontend;
    bool                        slaveModeEnabled;
    unsigned                    trackGroupId;       /* Program Index to Stream out */
    bool                        enableXcode;
    char                        *xcodeProfile;
    BIP_TranscodeNexusHandles   transcodeHandles;
    unsigned                    connectId;
    bool                        nxClientConnected;  /* set if NxClient_Connect is successful. */
} AppStreamerCtx;


#define USER_INPUT_BUF_SIZE 64
/* function to allow proper exit of server */
bool exitThread( void )
{
    fd_set fds;
    struct timeval tv;
    char buffer[USER_INPUT_BUF_SIZE];

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    B_Os_Memset( buffer, 0, USER_INPUT_BUF_SIZE );
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    if ( FD_ISSET(STDIN_FILENO, &fds) )
    {
        fgets(buffer, USER_INPUT_BUF_SIZE, stdin);
        if (strstr(buffer, "q"))
        {
            BDBG_LOG(("Received quit "));
            return true;
        }
    }
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

    if (parserBand) NEXUS_ParserBand_Close( parserBand );

    BDBG_MSG(( BIP_MSG_PRE_FMT " Done" BIP_MSG_PRE_ARG ));
} /* untuneAndReleaseFrontend */

BIP_Status acquireAndTuneFrontend(
    AppStreamerCtx *pAppStreamerCtx,
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
    NEXUS_FrontendQamSettings     qamSettings;

    NEXUS_Platform_GetConfiguration( &platformConfig );
    NEXUS_Frontend_GetDefaultAcquireSettings( &settings );
    settings.capabilities.qam = true;
    hFrontend        = NEXUS_Frontend_Acquire( &settings );
    BIP_CHECK_GOTO(( hFrontend ), ( "NEXUS_Frontend_Acquire Failed" ), error, NEXUS_NOT_INITIALIZED, rc );

    parserBand = NEXUS_ParserBand_Open( NEXUS_ANY_ID );
    BIP_CHECK_GOTO(( parserBand ), ( "NEXUS_ParserBand_Open Failed" ), error, NEXUS_NOT_INITIALIZED, rc );

    BDBG_MSG(( BIP_MSG_PRE_FMT " hFrontend & Parserband are opened!" BIP_MSG_PRE_ARG ));

    NEXUS_Frontend_GetDefaultQamSettings( &qamSettings );
    qamSettings.frequency  = pAppStreamerCtx->frequency;
    qamSettings.mode       = pAppStreamerCtx->mode;
    qamSettings.symbolRate = pAppStreamerCtx->symbolRate;
#if 0
    qamSettings.lockCallback.callback = lock_changed_callback;
    qamSettings.lockCallback.context  = pAppStreamerCtx;
#endif

    NEXUS_Frontend_GetUserParameters( hFrontend, &userParams );
    NEXUS_Frontend_GetCapabilities( hFrontend, &capabilities );

    NEXUS_ParserBand_GetSettings( parserBand, &parserBandSettings );
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
    parserBandSettings.transportType = UDP_SERVER_TRANSPORT_TYPE;
    rc = NEXUS_ParserBand_SetSettings( parserBand, &parserBandSettings );
    BIP_CHECK_GOTO(( !rc ), ( "NEXUS_ParserBand_SetSettings Failed" ), error, rc, rc );

    rc = NEXUS_Frontend_TuneQam( hFrontend, &qamSettings );
    BIP_CHECK_GOTO(( !rc ), ( "NEXUS_Frontend_TuneQam Failed" ), error, rc, rc );

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

    BDBG_MSG(( BIP_MSG_PRE_FMT " B_Event_Set( pAppStreamerCtx->hUdpStreamerEvent )" BIP_MSG_PRE_ARG ));
    B_Event_Set( pAppStreamerCtx->hUdpStreamerEvent );
}

void stopAndDestroyStreamer(
    AppStreamerCtx *pAppStreamerCtx
    )
{
    if (!pAppStreamerCtx->hUdpStreamer)
        return;

    BDBG_MSG(( BIP_MSG_PRE_FMT " Stopping Streamer %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hUdpStreamer));
    BIP_UdpStreamer_Stop( pAppStreamerCtx->hUdpStreamer );

    /* Release Frontend resources */
    untuneAndReleaseFrontend( pAppStreamerCtx->hFrontend, pAppStreamerCtx->parserBand );

    BDBG_MSG(( BIP_MSG_PRE_FMT " Destroying Streamer %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hUdpStreamer));
    BIP_UdpStreamer_Destroy( pAppStreamerCtx->hUdpStreamer );

    return;
} /* stopAndDestroyStreamer */

static BIP_Status startStreamer(
    AppStreamerCtx *pAppStreamerCtx
    )
{
    BIP_Status              bipStatus = BIP_SUCCESS;
    BIP_MediaInfoStream     *pMediaInfoStream;

    /* Now provide settings for media input source */
    {
        BIP_StreamerTunerInputSettings  tunerInputSettings;
        BIP_StreamerStreamInfo          streamerStreamInfo;

        /* Get StreamInfo. */
        pMediaInfoStream = BIP_MediaInfo_GetStream(pAppStreamerCtx->hMediaInfo);
        BDBG_ASSERT(pMediaInfoStream);
        BIP_Streamer_GetStreamerStreamInfoFromMediaInfo( pMediaInfoStream, &streamerStreamInfo );

        /* Acquire & Tune to the frontend so that we can acquire MediaInfo */
        bipStatus = acquireAndTuneFrontend( pAppStreamerCtx, &pAppStreamerCtx->hFrontend, &pAppStreamerCtx->parserBand );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "acquireAndTuneFrontend Failed" ), error, bipStatus, bipStatus );

        BIP_Streamer_GetDefaultTunerInputSettings(&tunerInputSettings);
        bipStatus = BIP_UdpStreamer_SetTunerInputSettings( pAppStreamerCtx->hUdpStreamer, pAppStreamerCtx->parserBand, &streamerStreamInfo, &tunerInputSettings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_UdpStreamer_SetTunerInputSettings Failed" ), error, bipStatus, bipStatus );
    }

    /* Now specify the Tracks that should be added for streaming. */
    {
        BIP_MediaInfoTrackGroup *pMediaInfoTrackGroup = NULL;
        BIP_MediaInfoTrack      *pMediaInfoTrack = NULL;
        BIP_StreamerTrackInfo   streamerTrackInfo;
        bool                    trackGroupPresent = false;

        if (pMediaInfoStream->numberOfTrackGroups != 0)
        {
            trackGroupPresent = true;
            if (pAppStreamerCtx->trackGroupId != 0)
            {
                /* Get Track Group by Id.*/
                 pMediaInfoTrackGroup = BIP_MediaInfo_GetTrackGroupById( pAppStreamerCtx->hMediaInfo, pAppStreamerCtx->trackGroupId );
                 BIP_CHECK_GOTO( (pMediaInfoTrackGroup != NULL), ("BIP_MediaInfo_GetTrackGroupById() Failed"), error, BIP_ERR_INTERNAL, bipStatus);
            }
            else
            {
                /* Get the first track group and stream data for that track group.*/
                pMediaInfoTrackGroup = pMediaInfoStream->pFirstTrackGroupInfo;
                BIP_CHECK_GOTO( (pMediaInfoTrackGroup != NULL), ("pMediaInfoStream->pFirstTrackGroupInfo is NULL"), error, BIP_ERR_INTERNAL, bipStatus);
            }
            pMediaInfoTrack = pMediaInfoTrackGroup->pFirstTrackForTrackGroup;

            /* Add PAT & PMT for TS transport type. */
            if (pMediaInfoStream->transportType == NEXUS_TransportType_eTs)
            {
                B_Os_Memset( &streamerTrackInfo, 0, sizeof( streamerTrackInfo ) );
                streamerTrackInfo.trackId = 0; /* PAT is always at PID == 0 */
                streamerTrackInfo.type = BIP_MediaInfoTrackType_eOther;
                bipStatus = BIP_UdpStreamer_AddTrack( pAppStreamerCtx->hUdpStreamer, &streamerTrackInfo, NULL );
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_UdpStreamer_AddTrack Failed for PAT" ), error, bipStatus, bipStatus );
                /* Add PMT. */
                B_Os_Memset( &streamerTrackInfo, 0, sizeof( streamerTrackInfo ) );
                streamerTrackInfo.trackId = pMediaInfoTrackGroup->type.Ts.pmtPid; /* PMT Pid for this program. */
                streamerTrackInfo.type = BIP_MediaInfoTrackType_ePmt;
                bipStatus = BIP_UdpStreamer_AddTrack( pAppStreamerCtx->hUdpStreamer, &streamerTrackInfo, NULL );
                BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_UdpStreamer_AddTrack Failed for PAT" ), error, bipStatus, bipStatus );
                BDBG_MSG(( BIP_MSG_PRE_FMT "Added PAT & PMT (pid=%d)" BIP_MSG_PRE_ARG, streamerTrackInfo.trackId));
            }
        }
        else
        {
            /* None of the track belongs to any trackGroup, in this case stream out all tracks from mediaInfoStream.*/
            pMediaInfoTrack = pMediaInfoStream->pFirstTrackInfoForStream;
        }
        BIP_CHECK_GOTO( (pMediaInfoTrack != NULL), ("First pMediaInfoTrack itself is NULL"), error, BIP_ERR_INTERNAL, bipStatus);/* At least one track has to be present */

        while (pMediaInfoTrack)
        {
            B_Os_Memset( &streamerTrackInfo, 0, sizeof( streamerTrackInfo ) );
            BIP_Streamer_GetStreamerTrackInfoFromMediaInfo(pMediaInfoTrack, &streamerTrackInfo );

            bipStatus = BIP_UdpStreamer_AddTrack( pAppStreamerCtx->hUdpStreamer, &streamerTrackInfo, NULL );
            BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_UdpStreamer_AddTrack Failed for PAT" ), error, bipStatus, bipStatus );
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
        BIP_UdpStreamerOutputSettings streamerOutputSettings;

        BIP_UdpStreamer_GetDefaultOutputSettings( &streamerOutputSettings );
        bipStatus = BIP_UdpStreamer_SetOutputSettings(
                pAppStreamerCtx->hUdpStreamer,
                pAppStreamerCtx->streamerProtocol,
                BIP_String_GetString( pAppStreamerCtx->hIpAddress ),
                BIP_String_GetString( pAppStreamerCtx->hPort ),
                BIP_String_GetString( pAppStreamerCtx->hInterfaceName ),
                &streamerOutputSettings);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_UdpStreamer_SetOutputSettings Failed" ), error, bipStatus, bipStatus );
    }

    /* Add Encoder Profile. */
    if ( pAppStreamerCtx->enableXcode )
    {
#if NEXUS_HAS_VIDEO_ENCODER
        BIP_TranscodeProfile transcodeProfile;

        if ( strncmp( pAppStreamerCtx->xcodeProfile, ENCODER_PROFILE_720p_AVC, strlen(ENCODER_PROFILE_720p_AVC) ) == 0 )
        {
            BIP_Transcode_GetDefaultProfileFor_720p30_AVC_AAC_MPEG2_TS( &transcodeProfile );
        }
        else if ( strncmp( pAppStreamerCtx->xcodeProfile, ENCODER_PROFILE_480p_AVC, strlen(ENCODER_PROFILE_480p_AVC) ) == 0 )
        {
            BIP_Transcode_GetDefaultProfileFor_480p30_AVC_AAC_MPEG2_TS( &transcodeProfile );
        }
        else
        {
            BIP_Transcode_GetDefaultProfile( &transcodeProfile );
        }

        bipStatus = BIP_UdpStreamer_AddTranscodeProfile( pAppStreamerCtx->hUdpStreamer, &transcodeProfile );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_UdpStreamer_AddTranscodeProfile Failed" ), error, bipStatus, bipStatus );
#else
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "NEXUS Video Encoder Feature is not supported for this platform" BIP_MSG_PRE_ARG ));
            goto error;
        }
#endif
    }

    /* At this point, we have set Streamer's Input & Output settings, so start the streamer */
    {
        bipStatus = BIP_UdpStreamer_Start( pAppStreamerCtx->hUdpStreamer, NULL );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_UdpStreamer_Start Failed: hUdpStreamer %p", (void *)pAppStreamerCtx->hUdpStreamer ), error, bipStatus, bipStatus );
    }

error:
    return (bipStatus);
} /* startStreamer */

/* Scan Media Directory and generate MediaInfo for each file. */
static BIP_Status generateMediaInfoForTunerInput (
    AppStreamerCtx *pAppStreamerCtx,
    const char *pInfoFilesDirectoryPath
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    NEXUS_FrontendHandle hFrontend;
    NEXUS_ParserBand parserBand;
    BIP_StringHandle    hTunerInfoFileAbsolutePath = NULL;
    BIP_MediaInfoCreateSettings mediaInfoCreateSettings;
    BIP_MediaInfoStream *pMediaInfoStream = NULL;

    /* Acquire & Tune to the frontend so that we can acquire MediaInfo */
    bipStatus = acquireAndTuneFrontend( pAppStreamerCtx, &hFrontend, &parserBand );
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "acquireAndTuneFrontend Failed" ), error, bipStatus, bipStatus );

    /* Build a unique info file name for Qam input */
    hTunerInfoFileAbsolutePath = BIP_String_CreateFromPrintf( "%s/Qam_Freq_%d.xml", pInfoFilesDirectoryPath, pAppStreamerCtx->frequency );
    BIP_CHECK_GOTO( (hTunerInfoFileAbsolutePath != NULL), ("BIP_String_CreateFromPrintf() Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    BIP_MediaInfo_GetDefaultCreateSettings(&mediaInfoCreateSettings);
    mediaInfoCreateSettings.reAcquireInfo = true;
    pAppStreamerCtx->hMediaInfo = BIP_MediaInfo_CreateFromParserBand(
                    parserBand,
                    BIP_String_GetString(hTunerInfoFileAbsolutePath),
                    &mediaInfoCreateSettings
                    );
    BIP_CHECK_GOTO( (pAppStreamerCtx->hMediaInfo != NULL), ("BIP_MediaInfo_CreateFromParserBand() Failed"), error, BIP_ERR_INTERNAL, bipStatus);

    pMediaInfoStream = BIP_MediaInfo_GetStream(pAppStreamerCtx->hMediaInfo);
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

void printUsage( char *pCmdName )
{
    printf( "Usage: %s\n", pCmdName );
    printf(
        "  --help or -h for help\n"
        "  -ip          #   IP Address to which media is streamed out to (default : 224.1.1.10)\n"
        "  -port        #   Port to which media is streamed out to (default : 1234)\n"
        "  -proto       #   Protocol string: udp | rtp (default is udp\n"
        "  -interface   #   Interface Name on which media is streamed out to (default : eth0\n"
        );
    printf(
        "  -trackGroupId#   program Number of a particular program in MPTS (defaults to 1st program)\n"
        "  -infoDir     #   Info Files Directory Path (default is /data/info)\n"
        "  -freq        #   frequency in MHz (default is 549 Mhz)\n"
        "  -mode        #   64 = QAM-64, 256 = QAM-256 (default is QAM-256)\n"
        "  -slave       #   Start Server in slave mode (Client in Nexus Multi-Process)\n"
        );
    printf(
        "  -xcode           #   Xcode the input using XcodeProfile (default No xcode)\n"
        "  -xcodeProfile    #   Pre-defined xcode profile string: 720pAvc (default), 480pAvc, \n"
        );
}

BIP_Status parseOptions(
    int    argc,
    char   *argv[],
    AppStreamerCtx *pAppStreamerCtx
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
            BIP_String_StrcpyChar( pAppStreamerCtx->hPort, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-ip") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppStreamerCtx->hIpAddress, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-proto") && i+1<argc )
        {
            BDBG_ERR(("proto is %s", argv[i+1]));
            if (strcmp(argv[++i], "udp") == 0)
                pAppStreamerCtx->streamerProtocol = BIP_UdpStreamerProtocol_ePlainUdp;
            else
                pAppStreamerCtx->streamerProtocol = BIP_UdpStreamerProtocol_eRtp;
        }
        else if ( !strcmp(argv[i], "-interface") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppStreamerCtx->hInterfaceName, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-infoDir") && i+1<argc )
        {
            BIP_String_StrcpyChar( pAppStreamerCtx->hInfoDirectoryPath, argv[++i] );
        }
        else if ( !strcmp(argv[i], "-freq") && i+1<argc )
        {
            pAppStreamerCtx->frequency = 1000000 * strtoul(argv[++i], NULL, 0); /* converted to hz */
        }
        else if ( !strcmp(argv[i], "-mode") && i+1<argc )
        {
            pAppStreamerCtx->mode = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-slave") )
        {
            pAppStreamerCtx->slaveModeEnabled = true;
        }
        else if ( !strcmp(argv[i], "-trackGroupId") && i+1<argc )
        {
            pAppStreamerCtx->trackGroupId = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-xcode") )
        {
            pAppStreamerCtx->enableXcode = true;
        }
        else if ( !strcmp(argv[i], "-xcodeProfile") )
        {
            pAppStreamerCtx->xcodeProfile = argv[++i];
        }
        else
        {
            printf("Error: incorrect or unsupported option: %s\n", argv[i]);
            printUsage(argv[0]);
            exit(0);
        }
    }
    bipStatus = BIP_SUCCESS;
    if ( pAppStreamerCtx->enableXcode && !pAppStreamerCtx->xcodeProfile ) pAppStreamerCtx->xcodeProfile = ENCODER_PROFILE_720p_AVC;
    BDBG_LOG(( BIP_MSG_PRE_FMT " Starting UDP STreamer on ip:port %s:%s on [%s], infoDir %s, frequency %u, mode %u, Xcode %s" BIP_MSG_PRE_ARG,
                BIP_String_GetString( pAppStreamerCtx->hIpAddress ),
                BIP_String_GetString( pAppStreamerCtx->hPort ),
                BIP_String_GetString( pAppStreamerCtx->hInterfaceName ),
                BIP_String_GetString( pAppStreamerCtx->hInfoDirectoryPath ),
                pAppStreamerCtx->frequency,
                pAppStreamerCtx->mode,
                pAppStreamerCtx->enableXcode ? pAppStreamerCtx->xcodeProfile : "N"
             ));
    return ( bipStatus );
} /* parseOptions */

void unInitAppStreamerCtx(
    AppStreamerCtx *pAppStreamerCtx
    )
{
    if (!pAppStreamerCtx) return;
    if (pAppStreamerCtx->hMediaInfo) { BIP_MediaInfo_Destroy(pAppStreamerCtx->hMediaInfo); }
    if (pAppStreamerCtx->hStreamName) BIP_String_Destroy( pAppStreamerCtx->hStreamName);
    if (pAppStreamerCtx->hPort) BIP_String_Destroy( pAppStreamerCtx->hPort);
    if (pAppStreamerCtx->hIpAddress) BIP_String_Destroy( pAppStreamerCtx->hIpAddress);
    if (pAppStreamerCtx->hInterfaceName) BIP_String_Destroy( pAppStreamerCtx->hInterfaceName);
    if (pAppStreamerCtx->hInfoDirectoryPath) BIP_String_Destroy( pAppStreamerCtx->hInfoDirectoryPath);
    if (pAppStreamerCtx->hUdpStreamerEvent) {B_Event_Destroy( pAppStreamerCtx->hUdpStreamerEvent ); }
    if (pAppStreamerCtx) BKNI_Free( pAppStreamerCtx );
} /* unInitAppStreamerCtx */

AppStreamerCtx *initAppStreamerCtx( void )
{
    AppStreamerCtx *pAppStreamerCtx = NULL;
    BIP_Status bipStatus = BIP_ERR_INTERNAL;

    pAppStreamerCtx = B_Os_Calloc( 1, sizeof( AppStreamerCtx ));
    BIP_CHECK_GOTO(( pAppStreamerCtx ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    /* Setup default values for global App settings */
    pAppStreamerCtx->hPort = BIP_String_CreateFromChar( UDP_SERVER_PORT_STRING );
    BIP_CHECK_GOTO( (pAppStreamerCtx->hPort), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    pAppStreamerCtx->hIpAddress = BIP_String_CreateFromChar( UDP_SERVER_IP_STRING );
    BIP_CHECK_GOTO( (pAppStreamerCtx->hPort), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    pAppStreamerCtx->hInterfaceName = BIP_String_CreateFromChar( UDP_SERVER_INTERFACE_NAME_STRING );
    BIP_CHECK_GOTO( (pAppStreamerCtx->hPort), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    pAppStreamerCtx->hInfoDirectoryPath = BIP_String_CreateFromChar( UDP_SERVER_INFO_DIRECTORY_PATH );
    BIP_CHECK_GOTO( (pAppStreamerCtx->hInfoDirectoryPath), ("BIP_String_CreateFromChar() Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    pAppStreamerCtx->hUdpStreamerEvent = B_Event_Create( NULL );
    BIP_CHECK_GOTO(( pAppStreamerCtx->hUdpStreamerEvent ), ( "Event Creation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    pAppStreamerCtx->frequency = UDP_SERVER_QAM_FREQ;
    pAppStreamerCtx->mode = UDP_SERVER_QAM_MODE;
    pAppStreamerCtx->symbolRate = UDP_SERVER_QAM_SYM_RATE;
    pAppStreamerCtx->streamerProtocol = BIP_UdpStreamerProtocol_ePlainUdp;

error:
    return pAppStreamerCtx;
} /* initAppStreamerCtx */

int main(
    int    argc,
    char **argv
    )
{
    BIP_Status bipStatus;
    NEXUS_Error nrc;
    AppStreamerCtx *pAppStreamerCtx = NULL;

    /* Initialize BIP */
    bipStatus = BIP_Init(NULL);
    BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Init Failed" ), errorBipInit, bipStatus, bipStatus );

    /* Allocate & initialize App Ctx */
    pAppStreamerCtx = initAppStreamerCtx();
    BIP_CHECK_GOTO(( pAppStreamerCtx ), ( "initAppStreamerCtx Failed" ), errorParseOptions, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

    /* Parse command line options */
    bipStatus = parseOptions( argc, argv, pAppStreamerCtx );
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

        if (pAppStreamerCtx->slaveModeEnabled)
        {
            nrc = NEXUS_Platform_AuthenticatedJoin(NULL);
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_Join Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
        }
        else
        {
            NEXUS_PlatformStartServerSettings serverSettings;
            NEXUS_PlatformConfiguration platformConfig;

            NEXUS_Platform_GetDefaultSettings(&platformSettings);
            platformSettings.mode = NEXUS_ClientMode_eVerified;
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

    /* Generate MediaInfo for this Tuner resource */
    {
        bipStatus = generateMediaInfoForTunerInput( pAppStreamerCtx, BIP_String_GetString( pAppStreamerCtx->hInfoDirectoryPath) );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "generateMediaInfoForFiles Failed" ), error, bipStatus, bipStatus );
    }

    /* Create UdpStreamer */
    {
        BIP_UdpStreamerCreateSettings createStreamerSettings;

        BIP_UdpStreamer_GetDefaultCreateSettings( &createStreamerSettings );
        createStreamerSettings.endOfStreamingCallback.callback = endOfStreamCallbackFromBip;
        createStreamerSettings.endOfStreamingCallback.context = pAppStreamerCtx;
        pAppStreamerCtx->hUdpStreamer = BIP_UdpStreamer_Create( &createStreamerSettings );
        BIP_CHECK_GOTO(( pAppStreamerCtx->hUdpStreamer ), ( "BIP_UdpStreamer_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
        BDBG_MSG(( BIP_MSG_PRE_FMT " New BIP_UdpStreamer ... %p" BIP_MSG_PRE_ARG, (void *)pAppStreamerCtx->hUdpStreamer ));
    }

    /* Start UdpStreamer */
    {
        bipStatus = startStreamer( pAppStreamerCtx );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "startStreamer Failed" ), error, bipStatus, bipStatus );
    }

    /*
     * At this time, BIP has started listening for incoming requests from clients.
     * When new request comes from client, app gets the callback from BIP.
     * Callback function sends event to allow this main thread to process the events.
     */

    /* Wait on Event group and process events as they come */
    BDBG_LOG(( BIP_MSG_PRE_FMT " Type 'q' followed by ENTER to exit gracefully\n" BIP_MSG_PRE_ARG ));
    while (!exitThread())
    {
        if (B_Event_Wait( pAppStreamerCtx->hUdpStreamerEvent, 1000/*1sec*/ ) == B_ERROR_SUCCESS)
            break;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT " Stopping & Destroying UDP Streamer..." BIP_MSG_PRE_ARG ));

error:
    stopAndDestroyStreamer( pAppStreamerCtx );
#if NXCLIENT_SUPPORT
    NxClient_Uninit();
#else /* !NXCLIENT_SUPPORT */
    if (!pAppStreamerCtx->slaveModeEnabled)
        NEXUS_Platform_StopServer();
    NEXUS_Platform_Uninit();
#endif /* NXCLIENT_SUPPORT */

errorParseOptions:
    unInitAppStreamerCtx( pAppStreamerCtx );
errorBipInit:
    BIP_Uninit();

    BDBG_ERR(( "All done!" ));
    return (0); /* main */
}
