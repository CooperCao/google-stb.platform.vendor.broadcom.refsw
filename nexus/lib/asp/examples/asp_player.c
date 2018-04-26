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
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
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

BDBG_MODULE(asp_player);

#define HTTP_GET_REQUEST_STRING \
    "GET /%s HTTP/1.1\r\n" \
    "Connection: Close\r\n" \
    "User-Agent: ASP Test Player\r\n" \
    "\r\n"
#define MAX_DURATION_IN_SEC 600

static bool stateChanged = false;
/*
 * Psuedo code showing usage of B_ASP library APIs as Media Player.
 */
static void callbackFromAspLib(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    BDBG_LOG(("%s: Got callback w/ param=%d", BSTD_FUNCTION, param));
    if (param == 1) stateChanged = true;
}

static void printUsage(
    char   *pCmdName
    )
{
    printf( "Usage: %s\n", pCmdName);
    printf(
            "  --help or -h for help\n"
            "  -port <num>      #   Server Port (default is port 81\n"
            "  -ip <address>    #   Server IP   (default is 192.168.2.13\n"
            "  -target <name>   #   Target file to Get from Server\n"
            "  -disableOffload  #   Dont Offload connection to ASP, instead play it using Host N/W Stack.\n"
            "  -dontSave        #   Dont save file to the ./rec.ts file.\n"
            "  -duration <sec>  #   How long to play/receive the stream before quitting.\n"
          );
    exit(0);
} /* printUsage */

/* This function creates a TCP connection to server and returns the socket descriptor */
static int tcpConnect(char *pServer, char *pPort)
{
    int sd=-1, rc;
    struct sockaddr_in localAddr;
    struct addrinfo hints;
    struct addrinfo *addrInfo = NULL;
    struct addrinfo *addrInfoNode = NULL;

    BDBG_WRN(("%s: Connecting to %s:%s ...", BSTD_FUNCTION, pServer, pPort));
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    rc = getaddrinfo(pServer, pPort, &hints, &addrInfo);
    BDBG_ASSERT(rc == 0);

    for (addrInfoNode = addrInfo; addrInfoNode != NULL; addrInfoNode = addrInfo->ai_next)
    {
        if (addrInfoNode->ai_family == AF_INET)
            break;
    }
    if (!addrInfoNode) {
        BDBG_ERR(("%s: ERROR: no IPv4 address available for server ip:port=%s:%s", BSTD_FUNCTION, pServer, pPort));
        return -1;
    }
    addrInfo = addrInfoNode;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    BDBG_ASSERT(sd>0);

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(0);

    rc = bind(sd, (struct sockaddr *)&localAddr, sizeof(localAddr));
    BDBG_ASSERT(rc == 0);

    rc = connect(sd, addrInfo->ai_addr, addrInfo->ai_addrlen);
    BDBG_ASSERT(rc == 0);

    BDBG_WRN(("%s: Connected to %s:%s ...", BSTD_FUNCTION, pServer, pPort));
    return sd;
}

static void dataReadyCallback(void *context, int param)
{
    NEXUS_RecpumpHandle hRecpump = context;

    BSTD_UNUSED(hRecpump);
    {
        BDBG_MSG(("%s: param=%d!!", BSTD_FUNCTION, param));
    }

}

bool g_quit = false;
void sigHandler(int signalNum)
{
    g_quit = true;
    printf("Got Signal=%d, quitting (g_quit=%d) app...\n", signalNum, g_quit);
}

static FILE *fout = NULL;
static void dataReadyCallback1(void *context, int param)
{
    NEXUS_Error nrc;
    NEXUS_RecpumpHandle hRecpump = context;
    NEXUS_RecpumpStatus status;
    const void *pDataBuffer;
    size_t dataBufferLength;
    static uint64_t totalRead = 0;
    static unsigned count = 0;

    if (param==2)
    {
        BDBG_MSG(("%s: param=%d!!", BSTD_FUNCTION, param));
        BDBG_MSG(("%s: Recpump Overflow!!", BSTD_FUNCTION));
    }

    while (true)
    {
        nrc = NEXUS_Recpump_GetDataBuffer(hRecpump, &pDataBuffer, &dataBufferLength);
        BDBG_ASSERT(!nrc);

        nrc = NEXUS_Recpump_GetStatus(hRecpump, &status);
        BDBG_ASSERT(!nrc);
        BDBG_MSG(("depth=%u size=%u", status.data.fifoDepth, status.data.fifoSize));
        if (dataBufferLength == 0)
        {
            BDBG_MSG(("No data available to read, re-wait for the callback!"));
            return;
        }

        /* Save the data. */
        {
            if (fout)
            {
                fwrite(pDataBuffer, 1, dataBufferLength, fout);
                fflush(fout);
            }

            nrc = NEXUS_Recpump_DataReadComplete(hRecpump, dataBufferLength);
            BDBG_ASSERT(!nrc);
            totalRead += dataBufferLength;
            if (count++%10 == 0) BDBG_WRN(("dumped/wrote %u total=%llu bytes", dataBufferLength, totalRead));
        }
    }
}

int main(int argc, char *argv[])
{
    int                             i;
    B_Error                         rc;
    NEXUS_Error                     nrc;
    char                            *pServerIp;
    char                            *pServerPort;
    char                            *pTarget;
    int                             socketFdToOffload;
    bool                            disableOffload = false;
    B_AspChannelHandle              hAspCh;
    NEXUS_PlaypumpHandle            hPlaypump;
    NEXUS_RecpumpHandle             hRecpump;
    BKNI_EventHandle                hDataReadyEvent;
    BKNI_EventHandle                hStateChangedEvent;
    B_AspChannelStatus              status;
    NEXUS_PlaypumpSettings          playpumpSettings;
    NEXUS_PlaypumpOpenPidChannelSettings pidChannelSettings;
    NEXUS_PidChannelHandle          hAllPassPidChannel;
    NEXUS_RecpumpSettings           recpumpSettings;
    NEXUS_RecpumpOpenSettings       recpumpOpenSettings;
    unsigned                        maxDurationInSec;
    bool                            dontSave = false;

    /* Initialize */
    {
        B_Asp_Init(NULL);
        signal(SIGINT, sigHandler);
    }

    /* Parse command line options. */
    {
        pServerPort = "81";
        pServerIp   = "192.168.2.13";
        pTarget     = "big_buck_bunny_1080p60_40Mbps.ts";
        maxDurationInSec = MAX_DURATION_IN_SEC;
        for (i=1; i<argc; i++)
        {
            if ( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") )
            {
                printUsage(argv[0]);
            }
            else if ( !strcmp(argv[i], "-port") && i+1<argc )
            {
                pServerPort = argv[++i];
            }
            else if ( !strcmp(argv[i], "-ip") && i+1<argc )
            {
                pServerIp = argv[++i];
            }
            else if ( !strcmp(argv[i], "-target") && i+1<argc )
            {
                pTarget = argv[++i];
            }
            else if ( !strcmp(argv[i], "-disableOffload") )
            {
                disableOffload = true;
            }
            else if ( !strcmp(argv[i], "-dontSave") )
            {
                dontSave = true;
            }
            else if ( !strcmp(argv[i], "-duration") && i+1<argc )
            {
                maxDurationInSec = strtoul(argv[++i], NULL, 10);
            }
            else
            {
                BDBG_ERR(("Error: incorrect or unsupported option: %s", argv[i]));
                printUsage(argv[0]);
            }
        }
        BDBG_LOG(("%s options: server IP:Port=%s:%s OffloadDisabled=%s", argv[0], pServerIp, pServerPort, disableOffload?"Y":"N"));
    }

    if (!dontSave)
    {
        fout=fopen("./rec.ts", "w");
        BDBG_ASSERT(fout);
    }

    /* Initialize Nexus. */
    {
#if NXCLIENT_SUPPORT
        {
            NxClient_JoinSettings joinSettings;

            NxClient_GetDefaultJoinSettings(&joinSettings);
            snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
            nrc = NxClient_Join(&joinSettings);
            BDBG_ASSERT( nrc == NEXUS_SUCCESS );
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

    /* Open/Create resources needed for Playing AV Stream. */
    {
        BKNI_CreateEvent(&hDataReadyEvent);
        BDBG_ASSERT(hDataReadyEvent);
        BKNI_CreateEvent(&hStateChangedEvent);
        BDBG_ASSERT(hStateChangedEvent);

        hPlaypump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
        BDBG_ASSERT(hPlaypump);

        NEXUS_Playpump_GetSettings(hPlaypump, &playpumpSettings);
        playpumpSettings.transportType = NEXUS_TransportType_eTs;
        playpumpSettings.allPass = true;
        nrc = NEXUS_Playpump_SetSettings(hPlaypump, &playpumpSettings);
        BDBG_ASSERT( nrc == NEXUS_SUCCESS );

        /* Open an allPass Pid Channel. */
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        NEXUS_Playpump_GetAllPassPidChannelIndex(hPlaypump, &pidChannelSettings.pidSettings.pidChannelIndex );
        hAllPassPidChannel = NEXUS_Playpump_OpenPidChannel(hPlaypump, 0, &pidChannelSettings);
        BDBG_ASSERT(hAllPassPidChannel);
    }

    /*
     * At this point, Media Player would have already connected to Server, Probed Media,
     * Opened Pid Channels, started decoders, etc.
     * And it is ready to offload a connection socketFdToOffload to ASP.
     *
     * Streaming-in flow will look like this:
     * Network -> ASP -> Playpump -> AV Decoders -> Display
     *
     * Streaming-in Recording flow will look like this:
     * Network -> ASP -> Playpump -> Recpump
     *
     * This example shows the simple allpass recording of the incoming stream.
     */
    {
        /* Open an all-pass PID channel. */
        NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
        BDBG_ERR(("current: size=%u atomSize=%u threshold=%u",
                    recpumpOpenSettings.data.bufferSize,
                    recpumpOpenSettings.data.atomSize,
                    recpumpOpenSettings.data.dataReadyThreshold));
        recpumpOpenSettings.data.bufferSize = recpumpOpenSettings.data.bufferSize * 4;
        recpumpOpenSettings.data.dataReadyThreshold = 16384;
        hRecpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
        BDBG_ASSERT(hRecpump);

        NEXUS_Recpump_GetSettings(hRecpump, &recpumpSettings);
        recpumpSettings.data.dataReady.callback = dataReadyCallback;
        recpumpSettings.data.dataReady.context = hRecpump;
        recpumpSettings.data.dataReady.param = 0;
        recpumpSettings.data.overflow.callback = dataReadyCallback;
        recpumpSettings.data.overflow.context = hRecpump;
        recpumpSettings.data.dataReady.param = 1;
        recpumpSettings.bandHold = true;
        nrc = NEXUS_Recpump_SetSettings(hRecpump, &recpumpSettings);
        BDBG_ASSERT(nrc == NEXUS_SUCCESS);

        nrc = NEXUS_Recpump_AddPidChannel(hRecpump, hAllPassPidChannel, NULL);

        nrc = NEXUS_Recpump_Start(hRecpump);
        BDBG_ASSERT(nrc == NEXUS_SUCCESS);

        nrc = NEXUS_Playpump_Start(hPlaypump);
        BDBG_ASSERT( nrc == NEXUS_SUCCESS );
    }

    /* Now the backend (record/decode) is all setup, so setup the frontend (network & ASP). */
    /* Setup TCP connection to the server. */
    {
        socketFdToOffload = tcpConnect(pServerIp, pServerPort);
        BDBG_ASSERT(socketFdToOffload>0);
    }

    /* Create an ASP Channel in the StreamIn mode. */
    {
        B_AspChannelCreateSettings createSettings;

        B_AspChannel_GetDefaultCreateSettings( B_AspStreamingProtocol_eHttp,  /* example assumes HTTP protocol based streaming. */
                                                  &createSettings);

        /* Setup HTTP Protocol related settings. */
        createSettings.protocol = B_AspStreamingProtocol_eHttp;

        /* fill-in StreamingIn mode related settings. */
        createSettings.mode = B_AspStreamingMode_eIn;
        createSettings.modeSettings.streamIn.hPlaypump = hPlaypump;
        createSettings.modeSettings.streamIn.hRecpump = hRecpump;
        createSettings.mediaInfoSettings.transportType = NEXUS_TransportType_eTs;

        /* Create ASP Channel. This will allow us to send & receive HTTP Request & Response using this Channel. */
        hAspCh = B_AspChannel_Create(socketFdToOffload, &createSettings);
        BDBG_ASSERT(hAspCh);
    }

    /* Setup callbacks. */
    {
        B_AspChannelSettings settings;

        B_AspChannel_GetSettings(hAspCh, &settings);

        /* Setup a callback to notify when data (HTTP Response) will be available. */
        settings.dataReady.context = hDataReadyEvent;
        settings.dataReady.callback = callbackFromAspLib;
        settings.dataReady.param = 0;

        /* Setup a callback to notify state transitions indicating either network errors or EOF condition. */
        settings.stateChanged.context = hStateChangedEvent;
        settings.stateChanged.callback = callbackFromAspLib;
        settings.stateChanged.param = 1;

        rc = B_AspChannel_SetSettings(hAspCh, &settings);
        BDBG_ASSERT(rc==0);
    }

    /* Send HTTP Request using AspChannel. */
    {
        void *pHttpReqBuf;
        unsigned httpReqBufSize;
        void *pHttpReqBuf1;
        unsigned httpReqBufSize1;

        /* Get Buffer where HTTP Request will be written into. */
        rc = B_AspChannel_GetWriteBufferWithWrap(hAspCh, &pHttpReqBuf, &httpReqBufSize, &pHttpReqBuf1, &httpReqBufSize1);
        BDBG_ASSERT(rc==0);
        BDBG_WRN(("pHttpReqBuf=%p size=%u", pHttpReqBuf, httpReqBufSize));
        BKNI_Memset(pHttpReqBuf, 0, httpReqBufSize);

        /* Prepare HTTP Request into this buffer. */
        snprintf(pHttpReqBuf, httpReqBufSize-1, HTTP_GET_REQUEST_STRING, pTarget);
        BDBG_WRN(("httpReq=%s", pHttpReqBuf));

        /* Provide this buffer to ASP so that it can send it out on the network. */
        B_AspChannel_WriteComplete(hAspCh, strlen(pHttpReqBuf));
    }

    /* HTTP Request is sent, wait for HTTP Response to arrive & parse it. */
    while (true)
    {
        const void *pHttpRespBuf;
        unsigned httpRespBufSize;
        const void *pHttpRespBuf1;
        unsigned httpRespBufSize1;

        /* Check if ASP has received any data from network. */
        rc = B_AspChannel_GetReadBufferWithWrap(hAspCh, &pHttpRespBuf, &httpRespBufSize, &pHttpRespBuf1, &httpRespBufSize1);
        BDBG_ASSERT(rc==B_ERROR_SUCCESS);
        BSTD_UNUSED(pHttpRespBuf1);
        BSTD_UNUSED(httpRespBufSize1);

        if (httpRespBufSize)
        {
            /* TODO: HTTP Response is available, parse it & check if full Response has been read. */
            BDBG_WRN(("ResponseLength=%u response=%s", httpRespBufSize, pHttpRespBuf));

            if (strcasestr(pHttpRespBuf, "200 ok") == NULL)
            {
                BDBG_ERR(("Failed to get valid HTTP Response!"));
                goto out;
            }

            /* Let ASP know that we have consumed either whole or some part of this buffer. */
            B_AspChannel_ReadComplete(hAspCh, httpRespBufSize);

            /* break if full Response is read. */
            break;
        }

        /* Either we didn't read anything or current buffer doesn't contain the full HTTP Response, repeat this loop until full response is received. */
        BKNI_WaitForEvent(hDataReadyEvent, 5000);
    }

    /* We have parsed the HTTP Response and it's all well. So notify ASP to start the stream-in flow. */
    {
        BKNI_ResetEvent(hStateChangedEvent);    /* Reset this event if we need to know when the B_AspChannel_StartStreaming() completes. */
        BKNI_ResetEvent(hDataReadyEvent);

        B_AspChannel_StartStreaming(hAspCh);
    }

    /* Optionally wait for stateChanged Callback to indicate Streaming has indeed stated. */
    if (0)
    {
        BKNI_WaitForEvent(hStateChangedEvent, BKNI_INFINITE);
        B_AspChannel_GetStatus(hAspCh, &status);
        if (status.state == B_AspChannelState_eStartedStreaming)
        {
            /* Now we are playing the stream. */
        }
    }

    stateChanged = false;
    while (g_quit == false)
    {
        unsigned timeout = 100; /* msec */
        static unsigned count = 0, totalTimeout = 0;

        dataReadyCallback1(hRecpump, 1);
        if (count++ % 10 == 0)
        {
            B_AspChannel_PrintStatus(hAspCh);
            totalTimeout++;
        }
        BKNI_WaitForEvent(hStateChangedEvent, timeout);
        if (totalTimeout == maxDurationInSec) {BDBG_LOG(("%u sec passed, stopping!!", maxDurationInSec)); break;}
        if (stateChanged) {BDBG_LOG(("breaking loop due to stateChanged Callback from ASP Lib.")); break;}
    }

    /*
     * To show playing for some finite time, set the timeout value. By default, this example will play till it
     * gets EndOfStream (EOS) indication using the stateChanged Callback.
     */
out:
    /* Initiate Stop Sequence. */
    {
        /* Stop the ASP Channel. */
        if (hAspCh) B_AspChannel_StopStreaming(hAspCh);
        if (hPlaypump) NEXUS_Playpump_Stop(hPlaypump);
        if (hRecpump) NEXUS_Recpump_Stop(hRecpump);
    }

    /* Free-up the resources. */
    if (hAspCh) B_AspChannel_Destroy(hAspCh, NULL);
    NEXUS_Recpump_RemoveAllPidChannels(hRecpump);
    NEXUS_Playpump_ClosePidChannel(hPlaypump, hAllPassPidChannel);
    if (hPlaypump) NEXUS_Playpump_Close(hPlaypump);
    if (hRecpump) NEXUS_Recpump_Close(hRecpump);
    BKNI_DestroyEvent(hStateChangedEvent);
    BKNI_DestroyEvent(hDataReadyEvent);

    /* Uninitialize */
    {
        B_Asp_Uninit();
    }
    printf("exiting..\n");
    return (0);
}
