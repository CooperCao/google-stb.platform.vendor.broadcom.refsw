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
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARNG OUT OF
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

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_recpump.h"
#include "nexus_file.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"
#include "nexus_record.h"
#include "b_asp_lib.h"
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(asp_playpump_streamer);

#define DATA_READY_TIMEOUT_INTERVAL 50

#if 1
#define HBO 0
#define ALLEGRO 0
#define CNN 1
#define HEVC 0
#endif
#if 0
#define FILE_NAME_TO_STREAM     "./videos/toondisney.ts"
#define APP_TRANSPORT_TYPE  NEXUS_TransportType_eTs
#define APP_AUDIO_CODEC     NEXUS_AudioCodec_eMpeg
#define APP_AUDIO_PID       20
#define APP_VIDEO_CODEC     NEXUS_VideoCodec_eMpeg2
#define APP_VIDEO_PID       4102
#define APP_PCR_PID         4166
#elif HEVC
#define FILE_NAME_TO_STREAM     "./videos/ces2015-elemental-cats-hevc10-4Kp60.ts"
#define APP_TRANSPORT_TYPE  NEXUS_TransportType_eTs
#define APP_AUDIO_CODEC     NEXUS_AudioCodec_eAac       /* AAC */
#define APP_AUDIO_PID       482       /* No Audio */
#define APP_VIDEO_CODEC     NEXUS_VideoCodec_eH265
#define APP_VIDEO_PID       481
#define APP_PCR_PID         481
#elif ALLEGRO
#define FILE_NAME_TO_STREAM     "./videos/Allegro_BdWidth_CABAC_03_HD_10.1.ts"
#define APP_TRANSPORT_TYPE  NEXUS_TransportType_eTs
#define APP_AUDIO_CODEC     0       /* AC3 */
#define APP_AUDIO_PID       0       /* No Audio */
#define APP_VIDEO_CODEC     NEXUS_VideoCodec_eH264
#define APP_VIDEO_PID       50
#define APP_PCR_PID         49
#elif HBO
#define FILE_NAME_TO_STREAM     "./videos/HboMpeg2HD.mpg"
#define APP_TRANSPORT_TYPE  NEXUS_TransportType_eTs
#define APP_AUDIO_CODEC     7       /* AC3 */
#define APP_AUDIO_PID       20
#define APP_VIDEO_CODEC     NEXUS_VideoCodec_eMpeg2
#define APP_VIDEO_PID       17
#define APP_PCR_PID         17
#else
#define FILE_NAME_TO_STREAM     "./videos/cnnticker.mpg"
#define APP_TRANSPORT_TYPE  NEXUS_TransportType_eTs
#define APP_AUDIO_CODEC     NEXUS_AudioCodec_eMpeg
#define APP_AUDIO_PID       34
#define APP_VIDEO_CODEC     NEXUS_VideoCodec_eMpeg2
#define APP_VIDEO_PID       33
#define APP_PCR_PID         33
#endif

static size_t       gTotalBytesStreamed = 0;
static bool         gEofReached = false;
static bool         gEnableLoop = false;

/*
 * Simple example code for streaming out a Media File using B_Asp Library.
 */

static void printUsage(
    char   *pCmdName
    )
{
    printf( "Usage: %s\n", pCmdName);
    printf(
            "  --help or -h for help\n"
            "  -port            #   Server Port (default is port 5000\n"
            "  -disableOffload  #   Dont Offload connection to ASP, instead stream it out using Host N/W Stack.\n"
            "  -disableHttp     #   Dont do HTTP Protocol, just use simple TCP protocol. \n"
            "  -disableBandHold #   Disable the RAVE's backpressure to MCPB. \n"
            "  -enableLoop      #   Keep looping at the end of file. \n"
            "  -maxBitRate <num>#   Max bitrate to send the stream at, in Mbpbs (e.g. 40 for 40Mpbs), defaults to 20Mbps\n"
          );
    exit(0);
} /* printUsage */

static int createTcpListener(
    const char *pListenerPort
    )
{
    int                     rc;
    int                     socketFd;
    struct sockaddr_in      localAddr;
    socklen_t               localAddrLen = sizeof(localAddr);

    /* Lower the TCP TIMEWAIT timeout to 1 sec. */
    system("echo 1 > /proc/sys/net/ipv4/tcp_tw_recycle");
    system("echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse");

    /* IPv4 Listener Setup. */
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(atoi(pListenerPort));
    localAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketFd == -1)
    {
        int myerrno = errno;
        BDBG_ERR(("%s:%u ioctl() socket() failed, errno=%d", BSTD_FUNCTION, BSTD_LINE, myerrno));
        BDBG_ASSERT(false);
        return -1;
    }

    /* Bind on all interfaces so that we can receive incoming request on any network interface. */
    rc = bind(socketFd, (struct sockaddr *)&localAddr, localAddrLen);
    BDBG_ASSERT(rc == 0);

    /* Start listening. */
    rc = listen(socketFd, 32);
    BDBG_ASSERT(rc == 0);

    BDBG_LOG(("TCP Server is listening on port=%s", pListenerPort));
    return (socketFd);
}

static int acceptConnection(
    int listenerSockFd
    )
{
    int                     socketFd;
    struct sockaddr_in      remoteAddr;
    socklen_t               remoteAddrLen = sizeof(remoteAddr);

    BDBG_LOG(("%s: Waiting for TCP connection request on listenerSockFd=%d", BSTD_FUNCTION, listenerSockFd));
    socketFd = accept(listenerSockFd, (struct sockaddr *)&remoteAddr, &remoteAddrLen);
    BDBG_ASSERT(socketFd >= 0);

    BDBG_LOG(("%s: Accepted connection: on socketFd=%d", BSTD_FUNCTION, listenerSockFd, socketFd));
    return (socketFd);
} /* acceptConnection */

static int recvHttpRequest(
    int socketFd,
    char *pMediaFileNameBuffer,
    size_t  mediaFileNameBufferSize
    )
{
#define HTTP_bufferSize    2048
    char                    httpBuffer[HTTP_bufferSize+1];
    ssize_t                 bytesRead;

    BDBG_LOG(("%s: Waiting for HTTP request on socketFd=%d", BSTD_FUNCTION, socketFd ));
    BKNI_Memset(httpBuffer, 0, HTTP_bufferSize+1);
    bytesRead = read(socketFd, httpBuffer, HTTP_bufferSize);
    if (bytesRead > 0)
    {
        /* TODO: Parse HTTP Request. */
        BDBG_LOG(("HTTP Request (size=%d): -->\n", bytesRead));
        BDBG_LOG(("%s", httpBuffer));

        /* TODO: Add logic to parse the HTTP Request & return copy it to the pMediaFileNameBuffer. */
        if (pMediaFileNameBuffer)
        {
        }
        else
        {
            mediaFileNameBufferSize = 0;
        }
    }
    else
    {
        BDBG_ERR(("Error: Receiving of HTTP Request returned=%d", bytesRead));
    }
    return (bytesRead);
} /* recvHttpRequest */

static int sendHttpResponse(
    int socketFd
    )
{
#define HTTP_bufferSize    2048
    char                    httpBuffer[HTTP_bufferSize+1];
    ssize_t                 bytesWritten;
    size_t                  bytesToWrite;

    BKNI_Memset(httpBuffer, 0, HTTP_bufferSize+1);
    bytesToWrite = snprintf(httpBuffer, sizeof(httpBuffer),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: video/mpeg\r\n"
            "Connection: close\r\n"
            "Server: Broadcom HTTP Server Example 2.0\r\n"
#if 0
            "Content-length: 40189952\r\n"
#endif
            "BCM-Video-Pid: %d\r\n"
            "BCM-Video-Type: %d\r\n"
            "BCM-Audio-Pid: %d\r\n"
            "BCM-Audio-Type: %d\r\n"
            "BCM-Transport-Type: %d\r\n"
            "BCM-Pcr-Pid: %d\r\n"
            "\r\n",
            APP_VIDEO_PID,
            APP_VIDEO_CODEC,
            APP_AUDIO_PID,
            APP_AUDIO_CODEC,
            APP_TRANSPORT_TYPE,
            APP_PCR_PID
            );
    BDBG_LOG(("%s: Sending simple HTTP response on socketFd=%d", BSTD_FUNCTION, socketFd ));
    BDBG_LOG(("%s", httpBuffer));
    bytesWritten = write(socketFd, httpBuffer, bytesToWrite);
    if (bytesWritten != bytesToWrite)
    {
        BDBG_ERR(("%s: Failed to write=%u bytes, wrote=%d, errno=%d", BSTD_FUNCTION, bytesToWrite, bytesWritten, errno));
    }
    return (bytesWritten);
} /* sendHttpResponse */

static void callbackFromNexusAsp(
    void *context,
    int param
    )
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
} /* callbackFromNexusAsp */

static void drainStreamer(
    NEXUS_RecpumpHandle hRecpump,
    int   socketFd,
    B_AspChannelHandle hAspCh,
    BKNI_EventHandle hAspStateChangedEvent
    )
{
    ssize_t             rc;
    NEXUS_Error         nrc;
    uint8_t             *pBuffer;
    size_t              bufferLength;

    BDBG_LOG(("%s: hRecpump=%p disableOffload=%s socketFd=%d", BSTD_FUNCTION, hRecpump, hAspCh? "N":"Y", socketFd));

    NEXUS_StopCallbacks(hRecpump);
    if (!hAspCh)
    {
        while (true)
        {
            bufferLength = 0;
            pBuffer = NULL;
            nrc = NEXUS_Recpump_GetDataBuffer(hRecpump, (const void **)&pBuffer, &bufferLength);
            BDBG_ASSERT(nrc == NEXUS_SUCCESS);

            if (bufferLength == 0 || pBuffer == NULL) break;   /* No more data is available, we are done!. */

            rc = write(socketFd, pBuffer, bufferLength);
            if (rc != bufferLength)
            {
                BDBG_ERR(("%s: Failed to write=%u bytes, wrote=%d, errno=%d", BSTD_FUNCTION, bufferLength, rc, errno));
                break;
            }
            else
            {
                gTotalBytesStreamed += rc;
                BDBG_LOG(("%s: Wrote=%d of totalBytes=%zu bytes to socketFd=%d", BSTD_FUNCTION, rc, gTotalBytesStreamed, socketFd));
            }

            nrc = NEXUS_Recpump_DataReadComplete(hRecpump, bufferLength);
            BDBG_ASSERT(nrc == NEXUS_SUCCESS);
        }
    }
    else
    {
        B_AspChannelStatus status;

#if 0
        BKNI_WaitForEvent(hAspStateChangedEvent, BKNI_INFINITE);
        BDBG_LOG(("%s: Got hAspStateChangedEvent from ASP", BSTD_FUNCTION, rc, gTotalBytesStreamed, socketFd));
        B_AspChannel_GetStatus(hAspCh, &status);
#else
        BKNI_Sleep(5000);
#endif
        /* TODO:
        if (status.state == B_AspChannelState_)
        {
        }
        */
    }
} /* drainStreamer */

#if 1
static void dataReadFromRecpump(
   NEXUS_RecpumpHandle hRecpump,
   int   socketFd
   )
{
   ssize_t             rc;
   NEXUS_Error         nrc;
   uint8_t             *pBuffer;
   size_t              bufferLength;

   if (socketFd == -1) return; /* socketFd has been offloaded to ASP & it will consume from RAVE (Recpump) & streamout. */

   BDBG_MSG(("%s:Recpump=%p socketFd=%d", BSTD_FUNCTION, socketFd));

   nrc = NEXUS_Recpump_GetDataBuffer(hRecpump, (const void **)&pBuffer, &bufferLength);
   BDBG_ASSERT(nrc == NEXUS_SUCCESS);

   rc = write(socketFd, pBuffer, bufferLength);
   if (rc != bufferLength)
   {
       BDBG_ERR(("%s: Failed to write=%u bytes, wrote=%d, errno=%d", BSTD_FUNCTION, bufferLength, rc, errno));
   }
   else
   {
       gTotalBytesStreamed += rc;
       BDBG_MSG(("%s: Wrote=%d of totalBytes=%zu bytes to socketFd=%d", BSTD_FUNCTION, rc, gTotalBytesStreamed, socketFd));
   }

   nrc = NEXUS_Recpump_DataReadComplete(hRecpump, bufferLength);
   BDBG_ASSERT(nrc == NEXUS_SUCCESS);
}


static void dataReadyCallbackFromRecpump(
    void *pContext,
    int   param
    )
{
   BKNI_EventHandle hEvent = pContext;

   BSTD_UNUSED( param );
   BDBG_ASSERT(hEvent);

   BKNI_SetEvent(hEvent);
}
#else
static void dataReadyCallbackFromRecpump(
    void *pContext,
    int   socketFd
    )
{
    ssize_t             rc;
    NEXUS_Error         nrc;
    NEXUS_RecpumpHandle hRecpump = pContext;
    uint8_t             *pBuffer;
    size_t              bufferLength;

    if (socketFd == -1) return; /* socketFd has been offloaded to ASP & it will consume from RAVE (Recpump) & streamout. */

    BDBG_LOG(("%s: ============> Recpump=%p socketFd=%d", BSTD_FUNCTION, socketFd));

    nrc = NEXUS_Recpump_GetDataBuffer(hRecpump, (const void **)&pBuffer, &bufferLength);
    BDBG_ASSERT(nrc == NEXUS_SUCCESS);

    rc = write(socketFd, pBuffer, bufferLength);
    if (rc != bufferLength)
    {
        BDBG_ERR(("%s: Failed to write=%u bytes, wrote=%d, errno=%d", BSTD_FUNCTION, bufferLength, rc, errno));
    }
    else
    {
        gTotalBytesStreamed += rc;
        BDBG_LOG(("%s: Wrote=%d of totalBytes=%zu bytes to socketFd=%d", BSTD_FUNCTION, rc, gTotalBytesStreamed, socketFd));
    }

    nrc = NEXUS_Recpump_DataReadComplete(hRecpump, bufferLength);
    BDBG_ASSERT(nrc == NEXUS_SUCCESS);

} /* dataReadyCallbackFromRecpump */

#endif


#define ASP_PLAYPUMP_FRAME_BUFFERSIZE 1316
#define ASP_PLAYPUMP_PKTS_PER_READ 64
uint8_t ui8PacketBuffer[ASP_PLAYPUMP_FRAME_BUFFERSIZE];

int main(int argc, char *argv[])
{
    int                             i;
    char                            *pListenerPort;
    bool                            disableHttp = false;
    bool                            disableBandHold = false;
    bool                            disableOffload = false;
    unsigned                        maxBitRate = 40;    /* In Mbps */
    B_Error                         rc;
    NEXUS_Error                     nrc;
    B_AspChannelHandle              hAspCh = NULL;
    BKNI_EventHandle                hAspStateChangedEvent;
    B_AspChannelStatus              status;
    int                             listenerFd;
    int                             socketFdToOffload;
    FILE *f;

    NEXUS_RecpumpOpenSettings       recpumpOpenSettings;
    NEXUS_RecpumpHandle             hRecpump;
    NEXUS_PidChannelHandle          hAllPassPidCh;
    NEXUS_PlaypumpHandle            hPlaypump;
    NEXUS_RecpumpSettings           recpumpSettings;
    NEXUS_PlaypumpOpenPidChannelSettings playpumpOpenPidSettings;
    BKNI_EventHandle                hDataReadyEvent = NULL;


    f = fopen(FILE_NAME_TO_STREAM, "r");
    BDBG_ASSERT(f);

    BKNI_Init();

    /* Parse command line options. */
    {

        pListenerPort = "5000";
        for (i=1; i<argc; i++)
        {
            if ( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") )
            {
                printUsage(argv[0]);
            }
            else if ( !strcmp(argv[i], "-port") && i+1<argc )
            {
                pListenerPort = argv[++i];
            }
            else if ( !strcmp(argv[i], "-disableOffload") )
            {
                disableOffload = true;
            }
            else if ( !strcmp(argv[i], "-disableHttp") )
            {
                disableHttp = true;
            }
            else if ( !strcmp(argv[i], "-disableBandHold") )
            {
                disableBandHold = true;
            }
            else if ( !strcmp(argv[i], "-maxBitRate") )
            {
                maxBitRate = strtoul(argv[++i], NULL, 0);
            }
            else if ( !strcmp(argv[i], "-enableLoop") )
            {
                gEnableLoop = true;
            }
            else
            {
                BDBG_ERR(("Error: incorrect or unsupported option: %s", argv[i]));
                printUsage(argv[0]);
            }
        }
        BDBG_LOG(("asp_streamer options: listenerPort=%s offloadDisabled=%s HTTP disabled=%s, maxBitRate=%u Mbps, enableLoop=%s",
                    pListenerPort, disableOffload?"Y":"N", disableHttp?"Y":"N", maxBitRate, gEnableLoop?"Y":"N"));
    }

    /* Initialize Nexus. */
    {
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

            {
                NEXUS_PlatformStartServerSettings serverSettings;
                NEXUS_PlatformConfiguration platformConfig;

                NEXUS_Platform_GetDefaultSettings(&platformSettings);
                platformSettings.mode = NEXUS_ClientMode_eVerified;
                nrc = NEXUS_Platform_Init(&platformSettings);
                BDBG_ASSERT( nrc == NEXUS_SUCCESS );

                NEXUS_Platform_GetConfiguration(&platformConfig);

                NEXUS_Platform_GetDefaultStartServerSettings(&serverSettings);
                serverSettings.allowUnauthenticatedClients = true; /* client is written this way */
                serverSettings.unauthenticatedConfiguration.mode = NEXUS_ClientMode_eVerified;
                serverSettings.unauthenticatedConfiguration.heap[1] = platformConfig.heap[0]; /* for purposes of example, allow access to main heap */
                nrc = NEXUS_Platform_StartServer(&serverSettings);
                BDBG_ASSERT( nrc == NEXUS_SUCCESS );
            }
        }
#endif /* NXCLIENT_SUPPORT */
    }

    /* Create resources needed for streaming out. */
    {
        BKNI_CreateEvent(&hAspStateChangedEvent);
        BDBG_ASSERT(hAspStateChangedEvent);

        listenerFd = createTcpListener(pListenerPort);
        BDBG_ASSERT(listenerFd > 0);
    }

    /* Wait for TCP Connection Request from a client. */
    {
        /* Wait for client connection. */
        socketFdToOffload = acceptConnection(listenerFd);
        BDBG_ASSERT(socketFdToOffload);
    }

    if (!disableHttp)
    {
        /* Recv HTTP Request & get the mediaFile name. */
        rc = recvHttpRequest(socketFdToOffload, NULL, 0);
        BDBG_ASSERT( rc >= 0);

        rc = sendHttpResponse(socketFdToOffload);
        BDBG_ASSERT( rc >= 0);
    }

    /* Now setup the streaming out flow. */
    /* HW Flow: Data -> Playpump -> RAVE -> ASP -> Network. */
    /* SW Flow: Data -> Nexus Playpump -> Nexus Recpump -> ASP -> Network. */
    {
        hPlaypump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
        BDBG_ASSERT(hPlaypump);

        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&playpumpOpenPidSettings);
        {
           NEXUS_PlaypumpSettings settings;
           NEXUS_Playpump_GetSettings(hPlaypump, &settings );
           settings.allPass = true;
           settings.acceptNullPackets = true;

           NEXUS_Playpump_SetSettings(hPlaypump, &settings );
        }

        NEXUS_Playpump_GetAllPassPidChannelIndex(hPlaypump, &playpumpOpenPidSettings.pidSettings.pidChannelIndex );
        hAllPassPidCh = NEXUS_Playpump_OpenPidChannel(hPlaypump, 0x00, &playpumpOpenPidSettings);   /* PID is ignored in allPass mode */
        BDBG_ASSERT( hAllPassPidCh );

        NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
        /* TODO: w/o this increase, RAVE seems to back pressure MCPB & thus thruput drops every other sec. Root cause it. */

        BDBG_MSG((" recpumpOpenSettings.data.bufferSize ===================> %u", recpumpOpenSettings.data.bufferSize ));
#if 0
        recpumpOpenSettings.data.bufferSize = recpumpOpenSettings.data.bufferSize * 5;
#endif

#if 1
        BKNI_CreateEvent(&hDataReadyEvent);
        BDBG_ASSERT(hDataReadyEvent);

        /*recpumpOpenSettings.data.dataReadyThreshold = 1024*32; *//*(recpumpOpenSettings.data.bufferSize*8)/10;*//*recpumpOpenSettings.data.atomSize * 2; */    /* May need to tune this for offloadDisabled case. */

        BDBG_LOG(("recpumpOpenSettings.data.dataReadyThreshold      --------------> %u", recpumpOpenSettings.data.dataReadyThreshold));
#endif
        hRecpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
        BDBG_ASSERT( hRecpump );

        NEXUS_Recpump_GetSettings(hRecpump, &recpumpSettings);
        if (!disableBandHold)
        {
           recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eAuto;
        }
        else
        {
           recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eDisable;
        }

#if 0
        recpumpSettings.data.dataReady.callback = dataReadyCallbackFromRecpump;
        recpumpSettings.data.dataReady.context  = hRecpump;
        recpumpSettings.data.dataReady.param  = disableOffload ?
                                                socketFdToOffload :     /* Offload is disabled, so host will directly stream out using this socketFd. */
                                                -1;                     /* Offload is enabled, so host doesn't need socketFd as ASP is streaming out. */
#else
        recpumpSettings.data.dataReady.callback = dataReadyCallbackFromRecpump;
        recpumpSettings.data.dataReady.context  = hDataReadyEvent;
        recpumpSettings.data.dataReady.param  = disableOffload ?
                                                socketFdToOffload :     /* Offload is disabled, so host will directly stream out using this socketFd. */
                                                -1;                     /* Offload is enabled, so host doesn't need socketFd as ASP is streaming out. */
#endif



        BDBG_MSG((" disableOffload --------------->%d , recpumpSettings.data.dataReady.param =%d", disableOffload, recpumpSettings.data.dataReady.param ));

        nrc = NEXUS_Recpump_SetSettings(hRecpump, &recpumpSettings);
        BDBG_ASSERT( nrc == NEXUS_SUCCESS );

        nrc = NEXUS_Recpump_AddPidChannel(hRecpump, hAllPassPidCh, NULL);
        BDBG_ASSERT( nrc == NEXUS_SUCCESS );
    }

    if (!disableOffload)
    {
        /* Offload is NOT disabled, so setup ASP lib to enable ASP to stream out from RAVE (Recpump). */

        /* Create ASP Channel in the StreamOut mode. This example assumes that HTTP Response has already been sent. */
        {
            B_AspChannelCreateSettings createSettings;

            B_AspChannel_GetDefaultCreateSettings( B_AspStreamingProtocol_eHttp,  /* example assumes HTTP protocol based streaming. */
                    &createSettings);
            /* Setup HTTP Protocol related settings. */
            createSettings.protocol = B_AspStreamingProtocol_eHttp;
            createSettings.mode = B_AspStreamingMode_eOut;
            createSettings.modeSettings.streamOut.hRecpump = hRecpump;   /* source of AV data. */
            createSettings.mediaInfoSettings.transportType = NEXUS_TransportType_eTs;
            createSettings.mediaInfoSettings.maxBitRate = maxBitRate * 1024 *1024;

            /* Create ASP Channel. */
            hAspCh = B_AspChannel_Create(socketFdToOffload, &createSettings);
            BDBG_ASSERT(hAspCh);
        }

        /* Setup callbacks. */
        {
            B_AspChannelSettings settings;

            B_AspChannel_GetSettings(hAspCh, &settings);

            /* Setup a callback to notify state transitions indicating either network errors or EndOfStreaming condition. */
            settings.stateChanged.context = hAspStateChangedEvent;
            settings.stateChanged.callback = callbackFromNexusAsp;

            rc = B_AspChannel_SetSettings(hAspCh, &settings);
            BDBG_ASSERT(rc == B_ERROR_SUCCESS);
        }

        /* Start Streaming. */
        {
            rc = B_AspChannel_StartStreaming(hAspCh);
            BDBG_ASSERT(rc == B_ERROR_SUCCESS);

#if   0
            if (0)
            {
                BKNI_WaitForEvent(hAspStateChangedEvent, BKNI_INFINITE);
                B_AspChannel_GetStatus(hAspCh, &status);
                if (status.state == B_AspChannelState_eStartedStreaming)
                {
                    /* Now we are streaming out. */
                }
            }
#endif
        }
    } /* !disableOffload */


#if 0
    BDBG_WRN((">>>> hit enter to proceed\n"));
    getchar();
#endif
    /* All Streaming related setup is complete, so start Nexus Playback Producer. */
    {
        nrc = NEXUS_Recpump_Start(hRecpump);
        BDBG_ASSERT( nrc == NEXUS_SUCCESS );
        nrc = NEXUS_Playpump_Start(hPlaypump);
        BDBG_ASSERT( nrc == NEXUS_SUCCESS );
    }

    {
        BERR_Code rc;

        {
           unsigned i;
           size_t   bufferSize;
           void    *buffer = NULL;
           unsigned dataWritten = 0;
           bool eof=false;
           unsigned frameCountframeCount = 0;
           unsigned totalFileReadSize = 0;

           /* Now get the Playpump buffer and write data into it */
           while (!eof)
           {
              unsigned writeSize = 0;
              unsigned dataToReadFromFile = (ASP_PLAYPUMP_PKTS_PER_READ * ASP_PLAYPUMP_FRAME_BUFFERSIZE );

              BDBG_MSG(("Calling NEXUS_Playpump_GetBuffer---------- "));
              nrc = NEXUS_Playpump_GetBuffer( hPlaypump, &buffer, &bufferSize );
              if(nrc)
              {
                 BDBG_ERR(( "Returned error from NEXUS_Playpump_GetBuffer()!" ));
                 return -1;
              }

              if (bufferSize > dataToReadFromFile)
              {
                 writeSize = fread(buffer, 1, dataToReadFromFile, f);
                 if (writeSize < dataToReadFromFile) {
                    int ch;
                    ch = getc(f);
                    if (ch ==EOF ) {
                       eof = true;
                    }
                 }
              }
              else if( (bufferSize < dataToReadFromFile) && (bufferSize != 0) )
              {
                 writeSize = fread(buffer, 1, bufferSize, f);
                 if (writeSize < bufferSize) {
                    int ch;
                    ch = getc(f);
                    if (ch ==EOF ) {
                       eof = true;
                    }
                 }
              }
              BDBG_MSG(("Calling NEXUS_Playpump_WriteComplete with writeSize = %u---------- ", writeSize));

              totalFileReadSize += writeSize;

              if (writeSize != 0)
              {
                 if (NEXUS_Playpump_WriteComplete( hPlaypump, 0, writeSize ))
                 {
                     BDBG_ERR(( "Returned error from NEXUS_Playpump_WriteComplete()!" ));
                     return -1;
                 }
              }

              {
                 NEXUS_PlaypumpStatus playpumpStatus;
                 NEXUS_Playpump_GetStatus(hPlaypump , &playpumpStatus);
                 BDBG_MSG(("================ playpumpStatus.fifoDepth = %u and playpumpStatus.bytesPlayed = %u, playpumpStatus.streamErrors = %u, playpumpStatus.syncErrors = %u \n",
                           playpumpStatus.fifoDepth,
                           playpumpStatus.bytesPlayed,
                           playpumpStatus.streamErrors,
                           playpumpStatus.syncErrors
                           ));
              }

#if   0
              {
                 void    *bufferRecpump = NULL;
                 size_t recpumpBufferSize = 0;
                 nrc = NEXUS_Recpump_GetDataBuffer(hRecpump, (const void **)&bufferRecpump, &recpumpBufferSize);
                 BDBG_ASSERT(!nrc);
                 BDBG_LOG(("NEXUS_Recpump_GetDataBuffer available buffer size is %u", recpumpBufferSize));
              }
#endif

              if (eof == true) {

                 BDBG_LOG((" ############################### EOF Reached and the totalFileReadSize = %u, writeSize = %u", totalFileReadSize, writeSize));
                 break;
              }
              BKNI_Sleep(DATA_READY_TIMEOUT_INTERVAL);

              if (disableOffload)
              {
                 dataReadFromRecpump(hRecpump, socketFdToOffload);
              }
           }

           if (hAspCh)
           {
              B_AspChannel_PrintStatus(hAspCh);
           }
        }
    }

    /* Drain data after Playback notifies EOF. */
    if ( hAspCh )
    {
       /* drainStreamer(hRecpump, socketFdToOffload, hAspCh, hAspStateChangedEvent); */
    }

    /* Stop & Cleanup Sequence. */
    {
        BDBG_LOG(("Stop & Close resources..."));
        if (hAspCh) B_AspChannel_StopStreaming(hAspCh);

        NEXUS_Playpump_Stop(hPlaypump);

        NEXUS_StopCallbacks(hRecpump);
        NEXUS_Recpump_Stop(hRecpump);

        NEXUS_Recpump_RemoveAllPidChannels(hRecpump);

        NEXUS_Playpump_ClosePidChannel(hPlaypump, hAllPassPidCh);

        NEXUS_Playpump_Close(hPlaypump);
        NEXUS_Recpump_Close(hRecpump);

        if (hDataReadyEvent) BKNI_DestroyEvent(hDataReadyEvent);

        if (hAspCh) B_AspChannel_Destroy(hAspCh, NULL /*&socketFdToOffload*/);
        BKNI_DestroyEvent(hAspStateChangedEvent);
    }

#if NXCLIENT_SUPPORT
    NxClient_Uninit();
#else /* !NXCLIENT_SUPPORT */
    NEXUS_Platform_StopServer();
    NEXUS_Platform_Uninit();
#endif
    BDBG_LOG(("Done!...."));
    return 0;
}
