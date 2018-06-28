/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
#include "b_asp_output.h"
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(asp_streamer);

#if 1
#define BIG_BUCK_BUNNEY 1
#define HBO 0
#define ALLEGRO 0
#define CNN 0
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
/*#define FILE_NAME_TO_STREAM     "./videos/HboMpeg2HD.mpg"*/
#define FILE_NAME_TO_STREAM     "./videos/Hbo1.mpg"
#define APP_TRANSPORT_TYPE  NEXUS_TransportType_eTs
#define APP_AUDIO_CODEC     7       /* AC3 */
#define APP_AUDIO_PID       20
#define APP_VIDEO_CODEC     NEXUS_VideoCodec_eMpeg2
#define APP_VIDEO_PID       17
#define APP_PCR_PID         17
#elif BIG_BUCK_BUNNEY
#define FILE_NAME_TO_STREAM     "./videos/big_buck_bunny_1080p60_40Mbps.ts"
#define APP_TRANSPORT_TYPE  NEXUS_TransportType_eTs
#define APP_AUDIO_CODEC     7       /* AC3 */
#define APP_AUDIO_PID       258
#define APP_VIDEO_CODEC     NEXUS_VideoCodec_eH264
#define APP_VIDEO_PID       257
#define APP_PCR_PID         257
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

static int getSocketAddrInfo(
    int                         socketFd,           /* in: fd of socket to be offloaded from host to ASP. */
    struct sockaddr_in          *pLocalIpAddr,
    struct sockaddr_in          *pRemoteIpAddr
    )
{
    int rc;
    int addrLen = sizeof(*pLocalIpAddr);

    /* Get local ip_addr and port details.*/
    rc = getsockname(socketFd, (struct sockaddr *)pLocalIpAddr, (socklen_t *)&addrLen);
    if (rc!=0) {BDBG_ERR(("failed to get local address info..")); goto error;}
    BDBG_LOG(("%s: LocalIpAddr:Port %s:%d", __FUNCTION__, inet_ntoa(pLocalIpAddr->sin_addr), ntohs(pLocalIpAddr->sin_port)));

#if ENDIANNESS_DEBUG
    {
        uint8_t *pByte = (uint8_t *)&pSocketState->localIpAddr.sin_port;
        BDBG_MSG(("local port in hex=0x%x (Linux returned in BE, shown in LE on ARM), byte[0]=0x%x byte[1]=0x%x", pSocketState->localIpAddr.sin_port, pByte[0], pByte[1]));
        pByte = (uint8_t *)&pSocketState->localIpAddr.sin_addr.s_addr;
        BDBG_LOG(("local IP in hex=0x%x , Linux returned in BE, shown in LE on ARM), byte[0]=0x%x byte[1]=0x%x byte[2]=0x%x byte[3]=0x%x", pSocketState->localIpAddr.sin_addr.s_addr, pByte[0], pByte[1], pByte[2], pByte[3]));
    }
#endif
    /* Get remote ip_addr and port details.*/
    addrLen = sizeof(*pRemoteIpAddr);
    rc = getpeername(socketFd, (struct sockaddr *)pRemoteIpAddr, (socklen_t *)&addrLen);
    if (rc!=0) {BDBG_ERR(("failed to get local address info..")); goto error;}
    BDBG_LOG(("%s: RemoteIpAddr:Port %s:%d", __FUNCTION__, inet_ntoa(pRemoteIpAddr->sin_addr), ntohs(pRemoteIpAddr->sin_port)));
    return 0;

error:
    return -1;
} /* getSocketAddrInfo */

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
#define HTTP_BUFFER_SIZE    2048
    char                    httpBuffer[HTTP_BUFFER_SIZE+1];
    ssize_t                 bytesRead;

    BDBG_LOG(("%s: Waiting for HTTP request on socketFd=%d", BSTD_FUNCTION, socketFd ));
    BKNI_Memset(httpBuffer, 0, HTTP_BUFFER_SIZE+1);
    bytesRead = read(socketFd, httpBuffer, HTTP_BUFFER_SIZE);
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

static int prepareHttpResponse(
    char *pHttpBuffer,
    size_t httpBufferSize,
    size_t *pBytesToWrite
    )
{
    BKNI_Memset(pHttpBuffer, 0, httpBufferSize);
    *pBytesToWrite = snprintf(pHttpBuffer, httpBufferSize-1,
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
    BDBG_LOG(("HTTP Response (length=%u) -->\n\t%s", *pBytesToWrite, pHttpBuffer));
} /* prepareHttpResponse */

static int sendHttpResponse(
    int socketFd,
    char *pHttpBuffer,
    size_t bytesToWrite
    )
{
    size_t bytesWritten;

    bytesWritten = write(socketFd, pHttpBuffer, bytesToWrite);
    if (bytesWritten != bytesToWrite)
    {
        BDBG_ERR(("%s: Failed to write=%u bytes, wrote=%d, errno=%d", BSTD_FUNCTION, bytesToWrite, bytesWritten, errno));
        return -1;
    }
    return (0);
}

static void callbackFromNexusAsp(
    void *context,
    int param
    )
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    BDBG_LOG(("%s: Got callback w/ param=%d from Nexus", BSTD_FUNCTION, param));
} /* callbackFromNexusAsp */

static void callbackFromNexusPlayback(
    void *context,
    int param
    )
{
    BSTD_UNUSED(param);

    if (!gEnableLoop)
    {
        BKNI_SetEvent((BKNI_EventHandle)context);
        BDBG_LOG(("%s: Got EOF callback!", BSTD_FUNCTION));
        gEofReached = true;
    }
    else
    {
        BDBG_LOG(("%s: Got EOF callback, but looping is enabled!", BSTD_FUNCTION));
    }
} /* callbackFromNexusPlayback */

static void drainStreamer(
    NEXUS_RecpumpHandle hRecpump,
    int   socketFd,
    B_AspOutputHandle hAspOutput,
    BKNI_EventHandle hAspStateChangedEvent
    )
{
    ssize_t             rc;
    NEXUS_Error         nrc;
    uint8_t             *pBuffer;
    size_t              bufferLength;

    BDBG_LOG(("%s: hRecpump=%p disableOffload=%s socketFd=%d", BSTD_FUNCTION, hRecpump, hAspOutput? "N":"Y", socketFd));

    NEXUS_StopCallbacks(hRecpump);
    if (!hAspOutput)
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
        B_AspOutputStatus status;

#if 0
        BKNI_WaitForEvent(hAspStateChangedEvent, BKNI_INFINITE);
        BDBG_LOG(("%s: Got hAspStateChangedEvent from ASP", BSTD_FUNCTION, rc, gTotalBytesStreamed, socketFd));
        B_AspOutput_GetStatus(hAspOutput, &status);
#else
        BKNI_Sleep(5000);
#endif
        /* TODO:
        if (status.state == B_AspOutputState_)
        {
        }
        */
    }
} /* drainStreamer */

static bool socketError = false;
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

    BDBG_MSG(("%s: hRecpump=%p socketFd=%d", BSTD_FUNCTION, hRecpump, socketFd));

    nrc = NEXUS_Recpump_GetDataBuffer(hRecpump, (const void **)&pBuffer, &bufferLength);
    BDBG_ASSERT(nrc == NEXUS_SUCCESS);

    rc = write(socketFd, pBuffer, bufferLength);
    if (rc != bufferLength)
    {
        BDBG_ERR(("%s: Failed to write=%u bytes, wrote=%d, errno=%d", BSTD_FUNCTION, bufferLength, rc, errno));
        socketError = true;
    }
    else
    {
        gTotalBytesStreamed += rc;
        BDBG_MSG(("%s: Wrote=%d of totalBytes=%zu bytes to socketFd=%d", BSTD_FUNCTION, rc, gTotalBytesStreamed, socketFd));
    }

    nrc = NEXUS_Recpump_DataReadComplete(hRecpump, bufferLength);
    BDBG_ASSERT(nrc == NEXUS_SUCCESS);

} /* dataReadyCallbackFromRecpump */

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
    B_AspOutputHandle               hAspOutput = NULL;
    BKNI_EventHandle                hAspStateChangedEvent;
    BKNI_EventHandle                hEofEvent;
    B_AspOutputStatus               status;
    int                             listenerFd;
    int                             socketFdToOffload;
    char                            *pPlayFileName = FILE_NAME_TO_STREAM;
    NEXUS_FilePlayHandle            hPlayFile;
    NEXUS_RecpumpOpenSettings       recpumpOpenSettings;
    NEXUS_RecpumpHandle             hRecpump;
    NEXUS_PidChannelHandle          hAllPassPidCh;
    NEXUS_PlaybackPidChannelSettings pidChannelConfig;
    NEXUS_PlaypumpHandle            hPlaypump;
    NEXUS_PlaybackHandle            hPlayback;
    NEXUS_PlaybackSettings          playbackSettings;
    NEXUS_RecpumpSettings           recpumpSettings;

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
            assert( nrc == NEXUS_SUCCESS );
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

    /* Initialize ASP Library. */
    {
        B_Asp_Init(NULL);
    }

    /* Create resources needed for streaming out. */
    {
        BKNI_CreateEvent(&hAspStateChangedEvent);
        BDBG_ASSERT(hAspStateChangedEvent);
        BKNI_CreateEvent(&hEofEvent);
        BDBG_ASSERT(hEofEvent);

        listenerFd = createTcpListener(pListenerPort);
        BDBG_ASSERT(listenerFd > 0);
    }

    /* Wait for TCP Connection Request from a client. */
    {
        socketFdToOffload = acceptConnection(listenerFd);
        BDBG_ASSERT(socketFdToOffload);
    }

    if (!disableHttp)
    {
        /* Recv HTTP Request & get the mediaFile name. */
        rc = recvHttpRequest(socketFdToOffload, NULL, 0);
        BDBG_ASSERT( rc >= 0);
    }

    /* Now setup the streaming out flow. */
    /* HW Flow: HDD  -> XPT Playback   -> XPT RAVE      -> ASP    -> Network. */
    /* SW Flow: File -> Nexus Playback -> Nexus Recpump -> ASP FW -> Network. */
    {
        /* Open & Connect Nexus Playback & Nexus Recpump. */
        hPlayFile = NEXUS_FilePlay_OpenPosix(pPlayFileName, NULL);
        if (!hPlayFile)
        {
            BDBG_ERR(("Can't open file=%s, make sure it is available!", pPlayFileName));
            exit(-1);
        }

        hPlaypump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
        BDBG_ASSERT(hPlaypump);
        hPlayback = NEXUS_Playback_Create();
        BDBG_ASSERT(hPlayback);

        NEXUS_Playback_GetSettings(hPlayback, &playbackSettings);
        playbackSettings.playpump = hPlaypump;
        playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
        playbackSettings.playpumpSettings.allPass = true;   /* Stream out all PIDs. */
        playbackSettings.playpumpSettings.acceptNullPackets = true;
        playbackSettings.endOfStreamAction = gEnableLoop == true ? NEXUS_PlaybackLoopMode_eLoop : NEXUS_PlaybackLoopMode_ePause;
        playbackSettings.endOfStreamCallback.callback = callbackFromNexusPlayback;
        playbackSettings.endOfStreamCallback.context = hEofEvent;
        nrc = NEXUS_Playback_SetSettings(hPlayback, &playbackSettings);
        BDBG_ASSERT( nrc == NEXUS_SUCCESS );

        NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelConfig);
        NEXUS_Playpump_GetAllPassPidChannelIndex(playbackSettings.playpump, &pidChannelConfig.pidSettings.pidSettings.pidChannelIndex );
        hAllPassPidCh = NEXUS_Playback_OpenPidChannel(hPlayback, 0x00, &pidChannelConfig);   /* PID is ignored in allPass mode */
        BDBG_ASSERT( hAllPassPidCh );

        NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
        hRecpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
        BDBG_ASSERT( hRecpump );

        NEXUS_Recpump_GetSettings(hRecpump, &recpumpSettings);
        if (!disableBandHold)
        {
            recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
        }
        recpumpSettings.data.dataReady.callback = dataReadyCallbackFromRecpump;
        recpumpSettings.data.dataReady.context  = hRecpump;
        recpumpSettings.data.dataReady.param  = disableOffload ?
                                                socketFdToOffload :     /* Offload is disabled, so host will directly stream out using this socketFd. */
                                                -1;                     /* Offload is enabled, so host doesn't need socketFd as ASP is streaming out. */
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
            B_AspOutputCreateSettings createSettings;

            B_AspOutput_GetDefaultCreateSettings(&createSettings);
            /* Note: Apps can update the FIFO sizes here. */
            /* Create ASP Channel. */
            hAspOutput = B_AspOutput_Create(&createSettings);
            BDBG_ASSERT(hAspOutput);
        }

        /* Setup callbacks. */
        {
            B_AspOutputSettings settings;

            B_AspOutput_GetSettings(hAspOutput, &settings);

            /* Setup a callback to notify state transitions indicating either network errors or EndOfStreaming condition. */
            settings.endOfStreaming.callback = callbackFromNexusAsp;
            settings.endOfStreaming.context = hEofEvent;
            settings.endOfStreaming.param = 1;

            nrc = B_AspOutput_SetSettings(hAspOutput, &settings);
            BDBG_ASSERT(nrc == NEXUS_SUCCESS);
        }

        /* Connect to ASP */
        {
            B_AspOutputConnectHttpSettings settings;

            B_AspOutput_GetDefaultConnectHttpSettings(&settings);

            settings.maxBitRate = maxBitRate * 1024 * 1024;
            settings.transportType = NEXUS_TransportType_eTs;
            settings.hRecpump  = hRecpump;
            settings.hPlaypump = hPlaypump;

            nrc = B_AspOutput_ConnectHttp(hAspOutput, socketFdToOffload, &settings);
            BDBG_ASSERT(nrc == NEXUS_SUCCESS);
        }

        /* Send HTTP Response */
        {
#define HTTP_BUFFER_SIZE    2048
            char                    httpBuffer[HTTP_BUFFER_SIZE];
            size_t                  bytesToWrite;

            rc = prepareHttpResponse( httpBuffer, HTTP_BUFFER_SIZE, &bytesToWrite );
            BDBG_ASSERT(rc == 0);

            nrc = B_AspOutput_SendHttpResponse( hAspOutput, httpBuffer, bytesToWrite );
            BDBG_ASSERT(nrc == NEXUS_SUCCESS);
        }

        /* Start Streaming. */
        {
            B_AspOutputStartSettings startSettings;

            B_AspOutput_GetDefaultStartSettings(&startSettings);
            startSettings.feedMode = B_AspOutputFeedMode_eAuto;
            nrc = B_AspOutput_Start(hAspOutput, &startSettings);
            BDBG_ASSERT(nrc == NEXUS_SUCCESS);
        }
    } /* !disableOffload */
    else
    {
        /* Send HTTP Response */
#define HTTP_BUFFER_SIZE    2048
        char                    httpBuffer[HTTP_BUFFER_SIZE];
        size_t                  bytesToWrite;

        rc = prepareHttpResponse( httpBuffer, HTTP_BUFFER_SIZE, &bytesToWrite );
        BDBG_ASSERT(rc == 0);

        nrc = sendHttpResponse( socketFdToOffload, httpBuffer, bytesToWrite );
        BDBG_ASSERT(nrc == 0);
    }


    /* All Streaming related setup is complete, so start Nexus Playback Producer. */
    {
        nrc = NEXUS_Recpump_Start(hRecpump);
        BDBG_ASSERT( nrc == NEXUS_SUCCESS );

        nrc = NEXUS_Playback_Start(hPlayback, hPlayFile, NULL);
        BDBG_ASSERT( nrc == NEXUS_SUCCESS );
    }

    BDBG_LOG(("Waiting for EOF Event from Playback. "));
    while (1)
    {
        BERR_Code rc;

        rc = BKNI_WaitForEvent(hEofEvent, 1000);
        if (socketError)
        {
            break;
        }
        else if (rc == BERR_TIMEOUT && !hAspOutput)
        {
            continue;
        }
        else if (rc == BERR_TIMEOUT)
        {
            B_AspOutput_PrintStatus(hAspOutput);
        }
        else
        {
            break;
        }
    }

    /* Drain data after Playback notifies EOF. */
    {
        drainStreamer(hRecpump, socketFdToOffload, hAspOutput, hAspStateChangedEvent);
    }

    /* Stop & Cleanup Sequence. */
    {
        BDBG_LOG(("Stop & Close resources..."));
        if (hAspOutput) B_AspOutput_Stop(hAspOutput);
        NEXUS_Playback_Stop(hPlayback);
        NEXUS_StopCallbacks(hRecpump);
        NEXUS_Recpump_Stop(hRecpump);

        NEXUS_Recpump_RemoveAllPidChannels(hRecpump);
        NEXUS_Playback_ClosePidChannel(hPlayback, hAllPassPidCh);
        NEXUS_Playpump_Close(hPlaypump);
        NEXUS_Playback_Destroy(hPlayback);
        NEXUS_Recpump_Close(hRecpump);
        if (hPlayFile) NEXUS_FilePlay_Close(hPlayFile);

        if (hAspOutput) B_AspOutput_Destroy(hAspOutput);
        /* For now, app is closing the socket, but this needs to be revisted after the intial ASP bringup! */
        BDBG_WRN(("Closing socket: socketFdToOffload=%d", socketFdToOffload));
        close(socketFdToOffload);

        BKNI_DestroyEvent(hEofEvent);
        BKNI_DestroyEvent(hAspStateChangedEvent);
    }

    /* Uninitialize */
    {
        B_Asp_Uninit();
    }

#if NXCLIENT_SUPPORT
    NxClient_Uninit();
#else /* !NXCLIENT_SUPPORT */
    NEXUS_Platform_StopServer();
    NEXUS_Platform_Uninit();
#endif

    BDBG_LOG(("Done!...."));
}
