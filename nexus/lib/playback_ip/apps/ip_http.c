/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description: http server module
 *
 *
 *************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/syscall.h>
#ifdef __mips__
#include <asm/cachectl.h>
#endif
#ifndef DMS_CROSS_PLATFORMS
#include "nexus_platform.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "nexus_parser_band.h"
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_message.h"
#include "nexus_recpump.h"
#endif /* DMS_CROSS_PLATFORMS */
#include "b_playback_ip_lib.h"
#if !defined(DMS_CROSS_PLATFORMS) && defined(B_HAS_NETACCEL_SUPPORT)
#include "bnetaccel_info.h"
#endif /* DMS_CROSS_PLATFORMS */
#include "ip_streamer_lib.h"
#include "ip_streamer.h"
#include "b_dtcp_applib.h"
BDBG_MODULE(ip_http);

#define ALIGN_4096   (4096 - 1)
#define BUF_SIZE    (188*7*128)
#define NUM_BUF_DESC    24
#ifndef O_DIRECT  /* DMS_CROSS_PLATFORMS */
#define O_DIRECT    0x8000
#endif  /* DMS_CROSS_PLATFORMS */
#define FILENAME_MAX_LEN 256

extern int gExitThread;

extern char *getXferURIfromCDS(char *url);
extern char *B_PlaybackIp_UtilsStristr ( char *str, char *subStr);
extern int B_PlaybackIp_UtilsParsePlaySpeedString(const char *playSpeedStringOrig, int *speedNumerator, int *speedDenominator, int *direction);

static char *gRootDir = NULL;

#if !defined(DMS_CROSS_PLATFORMS) && defined(B_HAS_NETACCEL_SUPPORT)
int
sockWait(int sd, int socket_type, unsigned int size)
{
    STRM_Status_t status;
    unsigned int len = sizeof(status);

    if(getsockopt(sd, socket_type, STRM_STATUS, &status, &len) != 0)
        return -1;

    while( (status.free_desc_count <= 2) ||
            ((status.buffer_locked_depth + size ) > (size * NUM_BUF_DESC))) {
        usleep(10000);
        if (getsockopt(sd,  SOCK_BRCM_DGRAM, STRM_STATUS, &status, &len) != 0)
            return -1;
    }

    return 0;
}

#if 0
static void
setThreadAffinity(int cpu_affinity_mask)
{
#ifdef __mips__
    unsigned long new_mask;

    syscall(SYS_sched_setaffinity, getpid(), 4, &cpu_affinity_mask);
    syscall(SYS_sched_getaffinity, getpid(), 4, &new_mask);

    printf("Updated CPU Affinity mask %lu", new_mask);
    /* uncomment this line to verify the cpu affinity */
    /* while (1); */
#endif
}
#endif

/* set the number of buffer descriptors in the driver */
int
sockSetDesc(int sd, int socket_type, int num_desc)
{
    STRM_SetBuffer_t bi;
    unsigned int len = sizeof(bi);
    bi.num_desc = num_desc;

    if (setsockopt(sd, socket_type, STRM_SETDESC, &bi, len) != 0)
        return -1;
    return 0;
}

int
sockSetMode(int sd, int socket_type, int disable_checksum)
{
    STRM_Ctrl_t ctrl;
    memset(&ctrl,0,sizeof(ctrl));

    BDBG_MSG(("TCP Checksum computation %s", (disable_checksum == 1) ? "OFF" : "ON"));
    ctrl.disable_checksum = disable_checksum;
    if(setsockopt(sd, socket_type, STRM_CTRL, &ctrl, sizeof(ctrl)) != 0)
            BDBG_WRN(("setsockopt: %s Failed to disable checksum", "STRM_CTRL"));
    return 0;
}
#endif /* DMS_CROSS_PLATFORMS */

/* Open the listener service on port 5000 */
int
initTcpServer(IpStreamerGlobalCfg *ipStreamerGlobalCfg)
{
    struct sockaddr_in localAddr;
    int port, socket_type;
    char *interfaceName;
    int sd = -1;
    int reuse_flag = 1;

    port = ipStreamerGlobalCfg->listeningPort;
#if !defined(DMS_CROSS_PLATFORMS) && defined(B_HAS_NETACCEL_SUPPORT)
    socket_type = (ipStreamerGlobalCfg->accelSocket == 1) ? SOCK_BRCM_STREAM : SOCK_STREAM;
#else
    socket_type = SOCK_STREAM;
#endif
    interfaceName = ipStreamerGlobalCfg->interfaceName;
    gRootDir = ipStreamerGlobalCfg->rootDir;
    if ( (sd = socket(AF_INET, socket_type, 0)) < 0) {
        /* Socket Create Error */
        perror("Socket Open Err");
        return -EINVAL;
    }

    if (setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_flag, sizeof(reuse_flag) ) < 0 ) {
        BDBG_ERR(("REUSE Socket Error"));
        close(sd);
        return -EINVAL;
    }

#if 0
    struct ifreq ifr;
    /* TODO: see if this code is needed anymore */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interfaceName, sizeof(ifr.ifr_name));
    if (setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr) ) < 0 ) {
        BDBG_ERR(("SO_BINDTODEVICE Failed"));
        return -EINVAL;
    }
    BDBG_WRN(("Binding to i/f %s", interfaceName));
#endif

    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(port);
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr))) {
        perror("Socket Bind Err to bind to actual i/f");
        close(sd);
        return -EINVAL;
    }
    BDBG_MSG(("Socket Bind to actual i/f successful"));

    if (listen(sd, 4)) {
        perror("Socket listen Err");
        close(sd);
        return -EINVAL;
    }
    BDBG_MSG(("Started listening on %d port ", port));

    /* Set stdin to non-blocking */
    if (fcntl(sd, F_SETFL, fcntl(sd, F_GETFL)|O_NONBLOCK) < 0)
        BDBG_WRN(("ERROR setting non-blocking mode on listening socket"));

    return sd;
}

/* Close the listener service */
void
unInitTcpServer(int sd)
{
    close(sd);
}

/* Parses the input for pattern begin; followed by pattern end */
int
parseToken(const char *input, char *output, int output_len, char *begin, char *end)
{
    char *p_begin = strstr(input, begin);
    char *p_end;
    char temp;

    if (p_begin)
    {
        p_begin += strlen(begin);
        p_end = strstr(p_begin,end);
        if(!p_end)
            return -1;
        temp = *p_end;
        *p_end = 0;
        BDBG_MSG(("TokenFound = [%s]; (%s)",p_begin, begin ));
        if ( output_len > 1 )
            strncpy(output,p_begin, output_len-1);
        *p_end = temp;
        return 0;
    }
    return -1; /* Not found */
}

void
waitForNetworkEvent(int sd)
{
    fd_set rfds;
    struct timeval tv;

    while (!gExitThread) {
        FD_ZERO(&rfds);
        FD_SET(sd, &rfds);
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        if ( select(sd +1, &rfds, NULL, NULL, &tv) < 0 ) {
            perror("ERROR: select(): exiting...");
            break;
        }

        if (!FD_ISSET(sd, &rfds))
            /* No request from Client yet, go back to select loop */
            continue;
        break;
    }

    return;
}

void
getContentType(B_PlaybackIpPsiInfo *psi, char *contentType, int contentTypeLength)
{
    char *contentTypeString= NULL;
    memset(contentType, 0, contentTypeLength);
    switch (psi->mpegType) {
        case NEXUS_TransportType_eTs:
        case NEXUS_TransportType_eMpeg2Pes:
        case NEXUS_TransportType_eVob:
        case NEXUS_TransportType_eMpeg1Ps:
            contentTypeString = "video/mpeg"; /* MPEG2, AVC TS files */
            break;
            /* FOR TTS files, use "video/vnd.dlna.mpeg-tts" */
        case NEXUS_TransportType_eMp4:
            contentTypeString = "video/mp4"; /* MP4 */
            break;
        case NEXUS_TransportType_eAsf:
            contentTypeString = "video/x-ms-wmv"; /* ASF: WMV/WMA */
            break;
        case NEXUS_TransportType_eAvi:
            contentTypeString = "video/x-msvideo"; /* AVI */
            break;
        case NEXUS_TransportType_eWav:
            contentTypeString = "video/x-wav"; /* WAV */
            break;
        case NEXUS_TransportType_eEs:
            if (psi->audioCodec == NEXUS_AudioCodec_eUnknown) {
                BDBG_WRN(("Couldn't determine MIME type for container NEXUS_TransportType_eEs, video codec %d, audio codec NEXUS_AudioCodec_eUnknown", psi->videoCodec));
                contentTypeString = NULL;
            }
            else {
                contentTypeString = "audio/mpeg"; /* MP3 */
            }
            break;
        default:
            contentTypeString = NULL;
    }

    if(contentTypeString == NULL ){
        BDBG_WRN(("Couldn't determine MIME type for container %d, video codec %d, audio codec %d", psi->mpegType, psi->videoCodec, psi->audioCodec));
        return;
    }

    if (strlen(contentTypeString) < (unsigned)contentTypeLength)
        snprintf(contentType, contentTypeLength-1, "%s", contentTypeString);
    return;
}

/* waits for incoming HTTP requests, sends out HTTP response and returns new socket descriptor */
int
sendHttpResponse(IpStreamerConfig *ipStreamerCfg, int sd, B_PlaybackIpPsiInfo *psi)
{
    int nbytes;
    unsigned i;
    char responseBuf[1024], *response;
    off_t contentLength = 0;
    char contentType[32];
    int bytesCopied, bytesLeft;

    /* Build HTTP response */
    memset(responseBuf, 0, sizeof(responseBuf));
    memset(contentType, 0, sizeof(contentType));
    response = responseBuf;
    bytesLeft = sizeof(responseBuf) - 1;
    bytesCopied = snprintf(response, bytesLeft,
        "HTTP/1.1 200 OK\r\n"
        "Accept-Ranges: bytes\r\n"
#ifndef B_USE_HTTP_KEEPALIVE
        "Connection: close\r\n"
#endif
#ifdef B_USE_HTTP_CHUNK_ENCODING
        "TRANSFER-ENCODING: chunked\r\n"
#endif
        "transferMode.dlna.org: Streaming\r\n"
        "Server: Linux/2.6.xx, Broadcom IP Streamer 2.0\r\n"
        );
    bytesLeft -= bytesCopied;
    response += bytesCopied;

    if (psi) {
        getContentType(psi, contentType, sizeof(contentType));
        bytesCopied = snprintf(response, bytesLeft, "Content-Type: %s\r\n", contentType);
        bytesLeft -= bytesCopied;
        response += bytesCopied;
        if (psi->liveChannel) {
            if (!ipStreamerCfg->transcodeEnabled) {
                if (ipStreamerCfg->enableTimestamps)
                    bytesCopied = snprintf(response, bytesLeft, "BCM-LiveChannel: 1\r\nTTS: 1\r\n");
                else
                    bytesCopied = snprintf(response, bytesLeft, "BCM-LiveChannel: 1\r\n");
                bytesLeft -= bytesCopied;
                response += bytesCopied;
            }
        }
        else {
            /* File playback */
            if (ipStreamerCfg->fileOffsetEnabled) {
                bytesCopied = snprintf(response, bytesLeft, "Range: bytes=%lld-", (long long)ipStreamerCfg->beginFileOffset);
                bytesLeft -= bytesCopied;
                response += bytesCopied;
                if (!ipStreamerCfg->endFileOffset) {
                    ipStreamerCfg->endFileOffset = psi->contentLength - 1;
                }
                contentLength = ipStreamerCfg->endFileOffset - ipStreamerCfg->beginFileOffset + 1;
                bytesCopied = snprintf(response, bytesLeft, "%lld/%lld\r\n", (long long)ipStreamerCfg->endFileOffset, (long long)psi->contentLength);
                bytesLeft -= bytesCopied;
                response += bytesCopied;
            }
            else {
                /* No begin offset requested, so send content length is available */
                contentLength = psi->contentLength;
                ipStreamerCfg->endFileOffset = psi->contentLength - 1;
                ipStreamerCfg->beginFileOffset = 0;
            }
            if (!ipStreamerCfg->hlsSession)
            {
                /* we dont specify contentLength for HLS streaming case as we encode the content before streaming and thus segment length is not apriori known */
                bytesCopied = snprintf(response, bytesLeft, "Content-Length: %lld\r\n", (long long)contentLength);
                bytesLeft -= bytesCopied;
                response += bytesCopied;
            }
            if(ipStreamerCfg->dashSession) /*Note: Cfg->hlsSession set from url:  HlsEnabled, Cfg->dashSession set when .mpd is requested */
            {
                contentLength = 9999999;  /* Support for Osmo4 Dash player */
                /* we dont specify contentLength for HLS streaming case as we encode the content before streaming and thus segment length is not apriori known
                      but for the Osmo4 player its looking for the Content Length before playing*/
                bytesCopied = snprintf(response, bytesLeft, "Content-Length: %lld\r\n", (long long)contentLength);
                bytesLeft -= bytesCopied;
                response += bytesCopied;
            }
        }
        if (psi->psiValid) {
            BDBG_MSG(("Content length %lld, PSI info vpid %d, vcodec %d, pcr pid %d, apid %d, acodec %d, transport type %d",
                (long long)contentLength, psi->videoPid, psi->videoCodec, psi->pcrPid,
                psi->audioPid, psi->audioCodec, psi->mpegType));
            bytesCopied = snprintf(response, bytesLeft,
                "BCM-Video-Pid: %d\r\n"
                "BCM-Video-Type: %d\r\n"
                "BCM-Pcr-Pid: %d\r\n"
                "BCM-Audio-Pid: %d\r\n"
                "BCM-Audio-Type: %d\r\n"
                "BCM-Transport-Type: %d\r\n"
                ,
#ifdef NEXUS_HAS_VIDEO_ENCODER
                (ipStreamerCfg->transcodeEnabled ? ipStreamerCfg->transcode.outVideoPid:psi->videoPid),
                (ipStreamerCfg->transcodeEnabled ? ipStreamerCfg->transcode.outVideoCodec:psi->videoCodec),
                (ipStreamerCfg->transcodeEnabled ? ipStreamerCfg->transcode.outPcrPid:psi->pcrPid),
                (ipStreamerCfg->transcodeEnabled ? ipStreamerCfg->transcode.outAudioPid:psi->audioPid),
                (ipStreamerCfg->transcodeEnabled ? ipStreamerCfg->transcode.outAudioCodec:psi->audioCodec),
                (ipStreamerCfg->transcodeEnabled ? ipStreamerCfg->transcode.transportType: psi->mpegType)
#else
                psi->videoPid,
                psi->videoCodec,
                psi->pcrPid,
                psi->audioPid,
                psi->audioCodec,
                psi->mpegType
#endif
                );
            bytesLeft -= bytesCopied;
            response += bytesCopied;
            if (psi->extraVideoCodec != NEXUS_VideoCodec_eNone && psi->extraVideoPid != 0) {
                bytesCopied = snprintf(response, bytesLeft,
                                        "BCM-Extra-Video-Pid: %d\r\n"
                                        "BCM-Extra-Video-Type: %d\r\n",
                                        psi->extraVideoPid, psi->extraVideoCodec);
                bytesLeft -= bytesCopied;
                response += bytesCopied;
            }
            if (psi->numPlaySpeedEntries) {
                bytesCopied = snprintf(response, bytesLeft,
                    "x-intel-MediaDuration: %u\r\n"  /* this is in seconds */
                    "TTS: %d\r\n"
                    "BCM-First-PTS: %u\r\n",
                     psi->duration/1000 /* convert to seconds */,
                     psi->transportTimeStampEnabled,
                     psi->firstPts
                     );
                bytesLeft -= bytesCopied;
                response += bytesCopied;

                bytesCopied = snprintf(response, bytesLeft,
                    "ContentFeatures.dlna.org: DLNA.ORG_PN=MPEG_TS_NTSC;DLNA.ORG_OP=10;DLNA.ORG_PS=%d",
                    psi->playSpeed[0]);
                bytesLeft -= bytesCopied;
                response += bytesCopied;
                for (i=1; i<psi->numPlaySpeedEntries; i++) {
                    bytesCopied = snprintf(response, bytesLeft, ",%d", psi->playSpeed[i]);
                    bytesLeft -= bytesCopied;
                    response += bytesCopied;
                }
                bytesCopied = snprintf(response, bytesLeft, "\r\n");
                bytesLeft -= bytesCopied;
                response += bytesCopied;
            }
        }
        bytesCopied = snprintf(response, bytesLeft, "\r\n");
        bytesLeft -= bytesCopied;
        response += bytesCopied;
    }
    else {
        bytesCopied = snprintf(response, bytesLeft, "\r\n");
        bytesLeft -= bytesCopied;
        response += bytesCopied;
    }
    nbytes = response - responseBuf;
#ifdef __mips__
#if ANDROID
    syscall(cacheflush, response, nbytes, DCACHE);
#else
    syscall(SYS_cacheflush, response, nbytes, DCACHE);
#endif
#endif
    BDBG_MSG(("HTTP Response (socket %d, size %d) [%s]", sd, nbytes, responseBuf));

    /* send out HTTP response */
    if (write(sd, responseBuf, nbytes) != nbytes) {
        printf("Failed to write HTTP Response of %d bytes", nbytes);
        perror("write(): ");
        return -1;
    }
    return 0;
}

void
setDefaultIpStreamerSessionConfig(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerGlobalCfg *ipStreamerGlobalCfg
    )
{
    memset(ipStreamerCfg, 0, sizeof(IpStreamerConfig));

    /* IP tuner defaults */
    ipStreamerCfg->iphVersion = 4;
    ipStreamerCfg->srcProtocol = B_PlaybackIpProtocol_eHttp;
    strncpy(ipStreamerCfg->srcIpAddress, "127.0.0.1", sizeof(ipStreamerCfg->srcIpAddress)-1);
    ipStreamerCfg->srcPort = 1234;
    ipStreamerCfg->accelSocket = false;
    ipStreamerCfg->useLiveIpMode = true;
    ipStreamerCfg->encryptionEnabled = false;
    ipStreamerCfg->emiValue = B_CCI_eCopyNever;

    strncpy(ipStreamerCfg->interfaceName, ipStreamerGlobalCfg->interfaceName, sizeof(ipStreamerCfg->interfaceName)-1);
#if NEXUS_HAS_FRONTEND
    /* Qam tuner defaults */
    ipStreamerCfg->frequency = 111;
    ipStreamerCfg->subChannel = 1;
    ipStreamerCfg->qamMode = NEXUS_FrontendQamMode_e256;
#endif
    ipStreamerCfg->recDstEnabled = false;
    memset(ipStreamerCfg->fileName, 0, sizeof(ipStreamerCfg->fileName));
    strncpy(ipStreamerCfg->fileName, "recfile.mpg", sizeof(ipStreamerCfg->fileName)-1);

    memset(ipStreamerCfg->indexFileName, 0, sizeof(ipStreamerCfg->indexFileName));
    strncpy(ipStreamerCfg->indexFileName, "recfile.nav", sizeof(ipStreamerCfg->indexFileName)-1);
    ipStreamerCfg->streamingCfg.streamingProtocol = B_PlaybackIpProtocol_eHttp;
}

#if defined(NEXUS_HAS_VIDEO_ENCODER)
int
mapFrameRateToGopSize(NEXUS_VideoFrameRate frameRate)
{
    int gopSize;
    switch (frameRate)
    {
        case NEXUS_VideoFrameRate_e23_976:
        case NEXUS_VideoFrameRate_e24:
            gopSize = 24;
            break;
        case NEXUS_VideoFrameRate_e25:
            gopSize = 25;
            break;
        case NEXUS_VideoFrameRate_e29_97:
        case NEXUS_VideoFrameRate_e30:
            gopSize = 30;
            break;
        case NEXUS_VideoFrameRate_e50:
            gopSize = 50;
            break;
        case NEXUS_VideoFrameRate_e59_94:
        case NEXUS_VideoFrameRate_e60:
            gopSize = 60;
            break;
        case NEXUS_VideoFrameRate_e14_985:
            gopSize = 15;
            break;
        case NEXUS_VideoFrameRate_e7_493:
            gopSize = 7;
            break;
        default:
            /* not specified, we default framt rate to 30 and rely on vice firmware to do cadence detection */
            gopSize = 30;
    }
    BDBG_MSG(("frame rate %d, gop size %d", frameRate, gopSize));
    return gopSize;
}

static void
mapFrameRateToNexusValue(NEXUS_VideoFrameRate* nexusFrameRate, int frameRate)
{
    switch (frameRate)
    {
        case 23976:
            *nexusFrameRate = NEXUS_VideoFrameRate_e23_976;
            break;
        case 24:
            *nexusFrameRate = NEXUS_VideoFrameRate_e24;
            break;
        case 25:
            *nexusFrameRate = NEXUS_VideoFrameRate_e25;
            break;
        case 2997:
            *nexusFrameRate = NEXUS_VideoFrameRate_e29_97;
            break;
        case 30:
            *nexusFrameRate = NEXUS_VideoFrameRate_e30;
            break;
        case 50:
            *nexusFrameRate = NEXUS_VideoFrameRate_e50;
            break;
        case 5994:
            *nexusFrameRate = NEXUS_VideoFrameRate_e59_94;
            break;
        case 60:
            *nexusFrameRate = NEXUS_VideoFrameRate_e60;
            break;
        case 14985:
        case 15:
            *nexusFrameRate = NEXUS_VideoFrameRate_e14_985;
            break;
        case 7493:
            *nexusFrameRate = NEXUS_VideoFrameRate_e7_493;
            break;
        default:
            /* not specified, we default framt rate to 30 and rely on vice firmware to do cadence detection */
            *nexusFrameRate = NEXUS_VideoFrameRate_e30;
    }
    BDBG_MSG(("frame rate %d, nexus frame rate value %d", frameRate, *nexusFrameRate));
}
#endif

int
parseTranscodeOptions(
        IpStreamerOpenSettings *openSettings,
        IpStreamerConfig *cfg
        )
{
#ifdef NEXUS_HAS_VIDEO_ENCODER
    char *url = openSettings->requestUri;
    char tmpBuf[256];
    /* TranscodeEnabled="Yes|No";TransportBitrate=xxx;OutTransport="TS|PS";*/
    /* OutVideo="Yes|No";OutVideoCodec="MPEG2|H264|MPEG4Part2";OutAspectRatio="16x9|4x3"; */
    /* OutFrameRate="2995|30";OutWidth=xxx;OutHeight=xxx;OutProfile="BaseLine|Main";OutLevel="L3|L31|L4|Main"; */
    /* OutAudio="Yes|No";OutAudioCodec="AC3|MPEG|AAC"; */
#define TRANSCODER_VIDEO_PID 0x00101
#if 0
#define TRANSCODER_AUDIO_PID 0x0000
#else
#define TRANSCODER_AUDIO_PID 0x00102
#endif
#define TRANSCODER_PCR_PID 0x00103
#define TRANSCODER_PMT_PID 0x0055
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "TranscodeEnabled=Yes", ";") == 0) {
        cfg->transcodeEnabled = true;
        cfg->ipDstEnabled = true;
        /* DTCP/IP encryption is disabled for xcode channel */
        cfg->encryptionEnabled = false;
        BDBG_MSG(("DTCP/IP encryption has been disabled for xcode channel"));
        /* default settings */
        cfg->transcode.outVideoPid = TRANSCODER_VIDEO_PID;
        cfg->transcode.outAudioPid = TRANSCODER_AUDIO_PID;
        cfg->transcode.outPcrPid = TRANSCODER_PCR_PID;
        cfg->transcode.outPatPid = 0;
        cfg->transcode.outPmtPid = TRANSCODER_PMT_PID;


        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "TransportBitrate=", ";") == 0)
            cfg->transcode.transportBitrate = strtol(tmpBuf, (char **)NULL, 10);
        else
            cfg->transcode.transportBitrate = 2000000;
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutTransport=PS", ";") == 0)
            cfg->transcode.transportType = NEXUS_TransportType_eMpeg2Pes;
        else if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutTransport=TS", ";") == 0)
            cfg->transcode.transportType = NEXUS_TransportType_eTs;
        else
            cfg->transcode.transportType = NEXUS_TransportType_eTs;

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutVideo=No", ";") == 0)
            cfg->transcode.outVideo = false;
        else
            cfg->transcode.outVideo = true;
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutVideoCodec=MPEG2", ";") == 0)
            cfg->transcode.outVideoCodec = NEXUS_VideoCodec_eMpeg2;
        else if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutVideoCodec=H264", ";") == 0)
            cfg->transcode.outVideoCodec = NEXUS_VideoCodec_eH264;
        else if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutVideoCodec=MPEG4Part2", ";") == 0)
            cfg->transcode.outVideoCodec = NEXUS_VideoCodec_eMpeg4Part2;
        else
            cfg->transcode.outVideoCodec = NEXUS_VideoCodec_eH264;

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutAspectRatio=16x9", ";") == 0)
            cfg->transcode.outAspectRatio = NEXUS_DisplayAspectRatio_e16x9;
        else if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutAspectRatio=4x3", ";") == 0)
            cfg->transcode.outAspectRatio = NEXUS_DisplayAspectRatio_e4x3;
        else
            cfg->transcode.outAspectRatio = NEXUS_DisplayAspectRatio_eSar;

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutFrameRate=", ";") == 0) {
            int frameRate;
            frameRate = strtol(tmpBuf, (char **)NULL, 10);
            mapFrameRateToNexusValue(&cfg->transcode.outFrameRate, frameRate);
        }
        else {
            /* not specified, we default framt rate to 30 and rely on vice firmware to do cadence detection */
            cfg->transcode.outFrameRate = NEXUS_VideoFrameRate_e30;
        }
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutHeight=", ";") == 0)
            cfg->transcode.outHeight = strtol(tmpBuf, (char **)NULL, 10);
        else
            cfg->transcode.outHeight = 480;

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutWidth=", ";") == 0)
            cfg->transcode.outWidth = strtol(tmpBuf, (char **)NULL, 10);
        else {
            if (cfg->transcode.outHeight == 480)
                cfg->transcode.outWidth = 640;
            else
                cfg->transcode.outWidth = 1280;
        }

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutAudio=No", ";") == 0)
            cfg->transcode.outAudio = false;
        else
            cfg->transcode.outAudio = true;

        if (cfg->transcode.outAudio) {
            if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutAudioCodec=MPEG", ";") == 0)
                cfg->transcode.outAudioCodec = NEXUS_AudioCodec_eMp3;
            else if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutAudioCodec=AC3", ";") == 0)
                cfg->transcode.outAudioCodec = NEXUS_AudioCodec_eAc3;
            else if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutAudioCodec=AAC", ";") == 0)
                cfg->transcode.outAudioCodec = NEXUS_AudioCodec_eAacAdts;
            else
                cfg->transcode.outAudioCodec = NEXUS_AudioCodec_eAacAdts;
        }
        else {
            cfg->transcode.outAudioPid = 0;
        }
        if (cfg->transcode.outAudioPid == 0)
            cfg->transcode.outAudio = false;

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OutInterlaced=Yes", ";") == 0)
            cfg->transcode.outInterlaced = true;
        else
            /* default is progressive output */
            cfg->transcode.outInterlaced = false;

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "NonRealTimeMode=Yes", ";") == 0)
            cfg->transcode.nonRealTime = true;
        else
            /* default is realTimeMode */
            cfg->transcode.nonRealTime = false;
        BDBG_WRN(("xcode: bitrate %d, transport %d, ar %d, interlaced %d, fr %d, res %dx%d, video %d, audio %d, vcodec %d, acodec %d, nrtMode %d",
                cfg->transcode.transportBitrate, cfg->transcode.transportType, cfg->transcode.outAspectRatio, cfg->transcode.outInterlaced, cfg->transcode.outFrameRate,
                cfg->transcode.outWidth, cfg->transcode.outHeight, cfg->transcode.outVideo, cfg->transcode.outAudio,  cfg->transcode.outVideoCodec,  cfg->transcode.outAudioCodec, cfg->transcode.nonRealTime));
    }
    else {
        BDBG_MSG(("Transcode Param is not set"));
        if (cfg->srcType == IpStreamerSrc_eHdmi) {
            BDBG_ERR(("ERROR: Encoding Attributes need to be set for streaming out HDMI input"));
            return -1;
        }
        cfg->transcodeEnabled = false;
    }
    return 0;
#else
    BSTD_UNUSED(openSettings);
    BSTD_UNUSED(cfg);
    return 0;
#endif
}

void
parsePidsFromCsvList(char *pidListBuffer, uint16_t *pidList, int *pidListCount)
{
    char *tmp1, *tmp2;
    *pidListCount = 0;
    /* tmpBuf string point to <num>,<num2>,<num3>... 0*/
    if (pidListBuffer == NULL)
        return;
    for (tmp1 = pidListBuffer;
         ((tmp2 = strstr(tmp1, ",")) && (*pidListCount <IP_STREAMER_MAX_PIDS_PER_PROGRAM-1)); /* leaving one space for last entry */
         tmp1 = tmp2+1
        ) {
        *tmp2 = '\0'; /* replace , with null char, so as to null terminate the string */
        pidList[(*pidListCount)++] = strtol(tmp1, (char **)NULL, 10);
        *tmp2 = ','; /* replace back with , */
    }
    /* now get the last pid */
    pidList[(*pidListCount)++] = strtol(tmp1, (char **)NULL, 10);
    if (*pidListCount == IP_STREAMER_MAX_PIDS_PER_PROGRAM && tmp2 != NULL /*there was still an unprocessed pid in the list */) {
        BDBG_ERR(("%s: pidList is not big enough to store all pids, current size %d", __FUNCTION__, IP_STREAMER_MAX_PIDS_PER_PROGRAM));
        return;
    }

#ifdef TEST
    {
        int i;
        for (i=0;i<*pidListCount; i++)
            BDBG_MSG(("pid[%d] = 0x%x of %d pids", i, pidList[i], *pidListCount));
    }
#endif
}

int
parseSatIpOptionsUrl (  IpStreamerConfig *cfg, const char * url )
{

#if NEXUS_HAS_FRONTEND
    char tmpBuf[256];
    uint32_t freqKHz;
    float freqMHzInFixedPoint;

    /* "http://192.168.1.109:5000/?src=1&fe=1&freq=12226&pol=v&msys=dvbs&mtype=qpsk&sr=20000&fec=12&pids=0,16,17,18,20,260" */
    if (strstr(url, "&msys=") == NULL) {
        return -1;
    }
    cfg->srcType = IpStreamerSrc_eSat;

    BDBG_MSG(("%s: URL is %s", __FUNCTION__, url));

    /* freq=<int or fixed point num> units in MHz */
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "freq=", "&")) {
        BDBG_ERR(("%s: No Frequency Attribute found", __FUNCTION__));
        return -1;
    }
    freqMHzInFixedPoint = strtof(tmpBuf, (char **)NULL);
    freqKHz = freqMHzInFixedPoint * 1000;
    BDBG_MSG(("freq in Mhz %f, freq in Khz %u", freqMHzInFixedPoint, freqKHz));
#ifdef NEXUS_USE_7425_SATIPSW
    /* 7425/4528 based SAT>IP platform required special freq conversion */
    if (freqKHz < 11700000) {
        uint32_t interFreqKHz;
        /* low band case */
        interFreqKHz = (freqKHz - 9750000);
        BDBG_MSG(("low band case: freq in Khz %lu, interFreqKHz %d ", freqKHz, interFreqKHz));
        cfg->frequency = (2000000 - interFreqKHz) * 1000; /* nexus tune freq */
        cfg->toneEnabled = false;
    }
    else {
        /* high band case */
        cfg->frequency = (freqKHz - 10600000) * 1000;
        cfg->toneEnabled = true;
    }
#else /* no freq conversion for other platforms */
    cfg->frequency = freqKHz * 1000;
#endif
    BDBG_MSG(("frequency is %ul, tone %s", cfg->frequency, cfg->toneEnabled ? "Y":"N"));


    /* msys=dvbs | dvbs2 */
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "msys=", "&")) {
        BDBG_ERR(("%s: Sat Modulation mode is not set", __FUNCTION__));
        return -1;
    }
    if (strcmp(tmpBuf, "dvbs") == 0) {
        cfg->satMode = NEXUS_FrontendSatelliteMode_eDvb;
    }
    else if (strcmp(tmpBuf, "dvbs2") == 0) {
        /* mtype=qpsk | 8qpsk */
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "mtype=", "&")) {
            BDBG_ERR(("%s: Sat Modulation type is not set", __FUNCTION__));
            return -1;
        }
        if (strcmp(tmpBuf, "qpsk") == 0)
            cfg->satMode = NEXUS_FrontendSatelliteMode_eQpskLdpc;
        else if (strcmp(tmpBuf, "8psk") == 0)
            cfg->satMode = NEXUS_FrontendSatelliteMode_e8pskLdpc;
        else {
            BDBG_ERR(("%s: Sat Modulation type %s is valid/supported", __FUNCTION__, tmpBuf));
            return -1;
        }
    }
    else {
        BDBG_MSG(("%s: TODO: Sat Modulation mode %s is not yet supported", __FUNCTION__, tmpBuf));
        return -1;
    }
    BDBG_MSG(("Sat Modulation mode %d, type %d", cfg->satMode, cfg->satMode));

    /* plts=on | off */
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "plts=", "&") == 0) {
        cfg->pilotToneSpecified = true;
        if (strcmp(tmpBuf, "on") == 0)
            cfg->pilotTone = 1;
        else
            cfg->pilotTone = 0;
        BDBG_MSG(("%s: pilot tone is %s", __FUNCTION__, cfg->pilotTone?"on":"off"));
    }

    /* sr=2000 in kSymb/s */
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "sr=", "&")) {
        BDBG_ERR(("%s: No SymbolRate Attribute found", __FUNCTION__));
        return -1;
    }
    cfg->symbolRate = strtol(tmpBuf, (char **)NULL, 10);
    cfg->symbolRate *= 1000;
    BDBG_MSG(("%s: symbolRate is %d", __FUNCTION__, cfg->symbolRate));

    /* fec=12 | 23 | 34 | 56 | 78 | 35 | 45 | 910 */
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "fec=", "&")) {
        BDBG_ERR(("%s: No fec Attribute found", __FUNCTION__));
        return -1;
    }
    cfg->fec = strtol(tmpBuf, (char **)NULL, 10);

    /* pol=h | v | l | r polarization */
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "pol=", "&")) {
        BDBG_ERR(("%s: ERROR: No Polarization Attribute found", __FUNCTION__));
        return -1;
    }
    if (strcmp(tmpBuf, "v") == 0)
        cfg->verticalPolarization = true;
    else
        cfg->verticalPolarization = false;
    BDBG_MSG(("verticalPolarization is %d", cfg->verticalPolarization));

    /* parse these pid fields */
    /* pids=<num>,<num2>,<num3>... */
    /* pids=all | none */
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "pids=", "&") == 0) {
        if (strcmp(tmpBuf, "all") == 0) {
            /* pids=all | none */
            BDBG_MSG(("%s: pids is set to all for MPTS streaming", __FUNCTION__));
            cfg->enableAllpass = true;
        }
        else if (strcmp(tmpBuf, "none") == 0) {
            BDBG_MSG(("%s: pids is set to all for MPTS streaming", __FUNCTION__));
            cfg->noDemuxOutput = true;
        }
        else {
            /* pids=<num>,<num2>,<num3>... */
            parsePidsFromCsvList(tmpBuf, cfg->pidList, &cfg->pidListCount);
        }
        cfg->skipPsiAcquisition = true;
    }

    if (!cfg->noDemuxOutput)
        cfg->ipDstEnabled = true;

    /* some sat farms have multiple sat dishes ... src=1 tells which of multiple dishes we are trying to access */
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "src=", "&")) {
        BDBG_ERR(("%s: No src Attribute found", __FUNCTION__));
        return -1;
    }
    cfg->srcPosition = strtol(tmpBuf, (char **)NULL, 10);

    /* some sat dishes have multiple fe in the center of the disk ... up to 4 */
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "fe=", "&")) {
        BDBG_ERR(("%s: No fe Attribute found", __FUNCTION__));
        return -1;
    }
    cfg->srcFe = strtol(tmpBuf, (char **)NULL, 10);

    /* if url provided ip addr*/
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "addr=", "&")) {
        BDBG_ERR(("%s: No ipaddr Attribute found", __FUNCTION__));
    } else {
        #if 0
        unsigned long int ipaddr = strtol(tmpBuf, (char **)NULL, 10);
        struct in_addr destinationAddr; destinationAddr.s_addr = htonl(ipaddr);
        BDBG_MSG(("%s: addr (%lx);", __FUNCTION__, destinationAddr.s_addr ));
        strncpy ( cfg->srcIpAddress, inet_ntoa(destinationAddr), sizeof(cfg->srcIpAddress) );
        cfg->streamingCfg.streamingIpAddress = strdup(cfg->srcIpAddress);
        #else
        cfg->streamingCfg.streamingIpAddress = strdup(tmpBuf);
        #endif
        BDBG_MSG(("%s: IPaddr (%s)", __FUNCTION__, cfg->streamingCfg.streamingIpAddress ));
    }
    /* ir url provided ip port */
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "port=", "&")) {
        BDBG_MSG(("%s: No port Attribute found", __FUNCTION__));
    } else {
        cfg->srcPort = cfg->streamingCfg.streamingPort = strtol(tmpBuf, (char **)NULL, 10);
        BDBG_MSG(("%s: IPport (%d)", __FUNCTION__, cfg->srcPort ));
    }


    /* TODO: */
    /* addpids=<num>,<num2>,<num3>... */
    /* delpids=<num>,<num2>,<num3>... */

    /* TODO: not needed for DVBS */
    /* ro=<fixed point num> ignored for DVBS and for now */
    /* plts= on | off ignored for DVBS and for now */
    /* mcast=<dot-decimal ip addr> */
#else
        BSTD_UNUSED(cfg);
        BSTD_UNUSED(url);
#endif /* NEXUS_HAS_FRONTEND */

    return 0;
}

int
parseSatIpOptions( IpStreamerOpenSettings *openSettings, IpStreamerConfig *cfg )
{
    char *url = openSettings->requestUri;
    return parseSatIpOptionsUrl ( cfg, url );
}

int
getRequestInfoFromUrl(
        IpStreamerOpenSettings *openSettings,
        IpStreamerConfig *cfg,
        IpStreamerGlobalCfg *ipStreamerGlobalCfg
    )
{
    char tmpBuf[256];
    char *tmpPtr;
    char *url;
    int newFd;

    /* default configuration */
    setDefaultIpStreamerSessionConfig(cfg, ipStreamerGlobalCfg);

    url = openSettings->requestUri;
    newFd = openSettings->streamingFd;
    cfg->streamingFd = newFd;
    cfg->streamingFdLocal = openSettings->streamingFdLocal;
    cfg->mediaProbeOnly = openSettings->mediaProbeOnly;
    cfg->skipPsiAcquisition = openSettings->skipPsiAcquisition;
    cfg->eventCallback = openSettings->eventCallback;
    cfg->appHeader = openSettings->appHeader;
    cfg->appCtx = openSettings->appCtx;
    cfg->pvrDecKeyHandle = openSettings->pvrDecKeyHandle;
    cfg->hlsSession = openSettings->hlsSession;
    cfg->hlsSegmentSize = openSettings->hlsSegmentSize;
    cfg->autoRewind = openSettings->autoRewind;

    BDBG_MSG(("hlsSession %s, URL is:%s", openSettings->hlsSession ? "Yes" : "No", url));
    memset(tmpBuf, 0, sizeof(tmpBuf));

    tmpPtr = strstr(url, "&msys=");
    if (tmpPtr) {
        char *urlCopy;
        int urlLength;

        BDBG_MSG(("%s - msys= detected", __FUNCTION__ ));
        if(url[0] == '?'){
            /*trellis or bcmmserver handling the request*/
            urlLength = strlen(url);
            if ((urlCopy = BKNI_Malloc(urlLength+2)) == NULL) { /* one extra char for & to be added and other one for the null char */
                BDBG_ERR(("%s: Failed to allocate %d bytes of memory for url copy",__FUNCTION__ , urlLength+2));
                return -1;
            }
            strncpy(urlCopy, url, urlLength);
            /* now pad the urlCopy w/ & char */
            urlCopy[urlLength] = '&';
            urlCopy[urlLength+1] = '\0';
            if (parseSatIpOptions(openSettings, cfg)) {
                BDBG_ERR(("%s: Failed to parse sat-ip related parameters",__FUNCTION__));
                BKNI_Free(urlCopy);
                return -1;
            }
            BKNI_Free(urlCopy);
            return 0;
        } else {
            /* Sat/Ip Protocol Specific Request, it has all the input params in a special format and thus needs special handling */
            /* http://192.168.0.3/?src=1&fe=1&freq=12226&pol=h&msys=dvbs&mtype=qpsk&sr=27500&fec=34&pids=0,16,17,18,20,260,2432,2434,280,31200,101 */
            tmpPtr = strstr(url, "\r\n");
            if (!tmpPtr) {
                BDBG_ERR(("%s: Missing CRNL from the HTTP URL header: url is %s ", __FUNCTION__, url));
                return -1;
            }
            *tmpPtr = '\0';
            tmpPtr = strstr(url, " HTTP");
            if (!tmpPtr) {
                BDBG_ERR(("%s: Incorrect HTTP URL header: url is %s ", __FUNCTION__, url));
                return -1;
            }
            cfg->streamingCfg.streamingProtocol = B_PlaybackIpProtocol_eHttp;
            *tmpPtr = '&';
            *(tmpPtr+1) = '\0';
            if (parseSatIpOptions(openSettings, cfg)) {
                BDBG_ERR(("%s: Failed to parse sat-ip related parameters",__FUNCTION__));
                return -1;
            }
            return 0;
        }
    }

    tmpPtr = strstr(url, "LiveChannel;");
    if (!tmpPtr) {
        /* it is not a live channel, return error if RecEnabled is set */
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "RecEnabled=", ";") == 0) {
            BDBG_WRN(("Recording is being requested for non-Live channels, return error"));
            return -1;
        }
        goto fileParsing;
    }

    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "AllPassEnabled=", ";") == 0) {
        if (strcmp(tmpBuf, "Yes") == 0) {
            cfg->enableAllpass = true;
            cfg->skipPsiAcquisition = true;
        }
        else {
            cfg->enableAllpass = false;
        }
    }

    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Type=", ";")) {
        BDBG_MSG(("No Type Attribute found"));
        return -1;
    }
    BDBG_MSG(("Type is %s", tmpBuf));
    cfg->ipDstEnabled = true;
    if (strcmp(tmpBuf, "IP") == 0) {
    /* Syntax is --> */
    /* /LiveChannel;Type=IP;Address=192.168.1.1.;Port=1234;InterfaceName=eth0;Protocol=UDP;EncEnabled=Yes;RecEnabled=Yes;FileName=/data/videos/rec0.mpg; */
        cfg->srcType = IpStreamerSrc_eIp;
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Address=", ";")) {
            BDBG_WRN(("ERROR: No Address Attribute found"));
            return -1;
        }
        memcpy(cfg->srcIpAddress, tmpBuf, sizeof(cfg->srcIpAddress));
        BDBG_MSG(("Host is %s", cfg->srcIpAddress));
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Port=", ";")) {
            BDBG_MSG(("ERROR: No Port Attribute found"));
            return -1;
        }
        cfg->srcPort = strtol(tmpBuf, (char **)NULL, 10);
        BDBG_MSG(("port is %d", cfg->srcPort));
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Protocol=", ";") == 0) {
            BDBG_MSG(("Protocol is %s", tmpBuf));
            if (strcmp(tmpBuf, "UDP") == 0)
                cfg->srcProtocol = B_PlaybackIpProtocol_eUdp;
            else
                cfg->srcProtocol = B_PlaybackIpProtocol_eRtp;
        }
        else {
            BDBG_MSG(("No Protocol Attribute found, assuming UDP"));
            cfg->srcProtocol = B_PlaybackIpProtocol_eUdp;
        }
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "InterfaceName=", ";") == 0) {
            BDBG_MSG(("Interface is %s", tmpBuf));
            strncpy(cfg->interfaceName, tmpBuf, sizeof(cfg->interfaceName)-1);
        }
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "IpVersion=", ";") == 0) {
            BDBG_MSG(("IP version is %s", tmpBuf));
            if (strcmp(tmpBuf, "4") == 0)
                cfg->iphVersion = 4;
            else
                cfg->iphVersion = 6;
        }
        else {
            BDBG_MSG(("No IP Version Attribute, assuming IPv4"));
            cfg->iphVersion = 4;
        }
    }
#if NEXUS_HAS_FRONTEND
    /* Syntax is --> */
    /* /LiveChannel;Type=QAM;QamMode=QAM256;Freq=357.00;SubChannel=2;EncEnabled=Yes;TranscodeEnabled=Yes;AspectRatio=;Width=;Height=;BitRate= */
    else if (strcmp(tmpBuf, "QAM") == 0) {
        cfg->srcType = IpStreamerSrc_eQam;
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Freq=", ";")) {
            BDBG_MSG(("No Frequency Attribute found"));
            return -1;
        }
        cfg->frequency = strtol(tmpBuf, (char **)NULL, 10);
        BDBG_MSG(("frequency is %d", cfg->frequency));
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "QamMode=", ";") == 0) {
            if (strcmp(tmpBuf, "QAM256") == 0)
                cfg->qamMode = NEXUS_FrontendQamMode_e256;
            else
                cfg->qamMode = NEXUS_FrontendQamMode_e64;
            BDBG_MSG(("QAM Modulation type set to %d", cfg->qamMode));
        }
        else {
            BDBG_MSG(("No QAM Modulation type set, assuming QAM256 "));
            cfg->qamMode = NEXUS_FrontendQamMode_e256;
        }

#ifdef STREAMER_CABLECARD_SUPPORT
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "SourceID=", ";")) {
            BDBG_MSG(("No Source ID Attribute found"));
            return -1;
        }
        cfg->sourceId = strtol(tmpBuf, (char **)NULL, 10);
#endif

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "QamAnnex=", ";") == 0) {
            if (strcmp(tmpBuf, "A") == 0)
                cfg->qamAnnex = NEXUS_FrontendQamAnnex_eA;
            else if (strcmp(tmpBuf, "B") == 0)
                cfg->qamAnnex = NEXUS_FrontendQamAnnex_eB;
            else if (strcmp(tmpBuf, "C") == 0)
                cfg->qamAnnex = NEXUS_FrontendQamAnnex_eC;
            else
                cfg->qamAnnex = NEXUS_FrontendQamAnnex_eB;
            BDBG_MSG(("QAM Annex set to %d", cfg->qamAnnex));
        }
        else {
            BDBG_MSG(("No QAM Annex set, assuming AnnexB "));
                cfg->qamAnnex = NEXUS_FrontendQamAnnex_eB;
        }

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "SymbolRate=", ";") == 0) {
            cfg->symbolRate = strtol(tmpBuf, (char **)NULL, 10);
            BDBG_MSG(("%s: symbolRate is %d", __FUNCTION__, cfg->symbolRate));
        }
        else {
            switch(cfg->qamMode) {
            case NEXUS_FrontendQamMode_e64: cfg->symbolRate = 5056900; break;
            case NEXUS_FrontendQamMode_e256: cfg->symbolRate = 5360537; break;
            default:
            case NEXUS_FrontendQamMode_e1024: cfg->symbolRate = 0; /* TODO */ break;
            }
            BDBG_MSG(("No Symbol Rate set, setting based on the qamMode to %d", cfg->symbolRate));
        }

    }
    else if (strcmp(tmpBuf, "SAT") == 0) {
        /* Complete string is */
        /* /LiveChannel\;Type=SAT\;SatMode=DVB\;DiseqcVoltage=13\;ToneEnabled=Yes\;SymbolRate=20000000\;Freq=1207\;SubChannel=1\; */
        /* Fields are: */
        /* SatMode = DVB/DSS/etc.. */
        /* ToneEnabled=Yes|No */
        /* SymbolRate=20000000 */
        /* ToneEnabled=Yes|No */
        /* DiseqcVoltage=13|18 */
        /* AdcIndex = 0,1,2,3,  default is 0*/
        int voltage;
        int adcIndex;
        cfg->srcType = IpStreamerSrc_eSat;
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Freq=", ";")) {
            BDBG_ERR(("%s: No Frequency Attribute found", __FUNCTION__));
            return -1;
        }
        cfg->frequency = strtol(tmpBuf, (char **)NULL, 10);
        BDBG_MSG(("frequency is %d", cfg->frequency));

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "SymbolRate=", ";")) {
            BDBG_ERR(("%s: No SymbolRate Attribute found", __FUNCTION__));
            return -1;
        }
        cfg->symbolRate = strtol(tmpBuf, (char **)NULL, 10);
        BDBG_MSG(("%s: symbolRate is %d", __FUNCTION__, cfg->symbolRate));

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "SatMode=", ";")) {
            BDBG_ERR(("%s: No Sat Modulation type set", __FUNCTION__));
            return -1;
        }
        if (strcmp(tmpBuf, "DVB") == 0)
            cfg->satMode = NEXUS_FrontendSatelliteMode_eDvb;
        else if (strcmp(tmpBuf, "DSS") == 0)
            cfg->satMode = NEXUS_FrontendSatelliteMode_eDss;
        else if (strcmp(tmpBuf, "LDPC") == 0)
            cfg->satMode = NEXUS_FrontendSatelliteMode_eLdpc;
        else if (strcmp(tmpBuf, "QPSKLDPC") == 0)
            cfg->satMode = NEXUS_FrontendSatelliteMode_eQpskLdpc;
        else if (strcmp(tmpBuf, "TURBO") == 0)
            cfg->satMode = NEXUS_FrontendSatelliteMode_eTurbo;
        else if (strcmp(tmpBuf, "DVBS28PSK") == 0)
            cfg->satMode = NEXUS_FrontendSatelliteMode_eDvbs28psk;
        else {
            BDBG_MSG(("%s: TODO: Sat Modulation type %d is not yet supported", __FUNCTION__, cfg->satMode));
            return -1;
        }
        BDBG_MSG(("Sat Modulation type set to %d", cfg->satMode));

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "DiseqcVoltage=", ";")) {
            BDBG_ERR(("%s: ERROR: No DiseqcVoltage Attribute found", __FUNCTION__));
            return -1;
        }
        voltage = strtol(tmpBuf, (char **)NULL, 10);
        switch(voltage) {
            case 13:
                cfg->diseqcVoltage = NEXUS_FrontendDiseqcVoltage_e13v;
                break;
            case 14:
            default:
                cfg->diseqcVoltage = NEXUS_FrontendDiseqcVoltage_e18v;
                break;
        }
        BDBG_MSG(("DiseqcVoltage is set to %d", cfg->diseqcVoltage));

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "ToneEnabled=", ";") == 0) {
            if (strcmp(tmpBuf, "Yes") == 0) {
                cfg->toneEnabled = true;
                BDBG_MSG(("ToneEnabled Param is set"));
            }
            else {
                cfg->toneEnabled = false;
            }
        } else {
            BDBG_MSG(("ToneEnabled Param is not set"));
            cfg->toneEnabled = false;
        }

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "AdcIndex=", ";")) {
            BDBG_MSG(("%s: No AdcIndexFound Attribute. default to index 0", __FUNCTION__));
            cfg->newAdc =0;
            cfg->isNewAdc=false;
        }
        else {
            adcIndex = strtol(tmpBuf, (char **)NULL, 10);
            if(adcIndex < 0 || adcIndex >3 )
            {
                BDBG_ERR(("%s: ERROR: Adc index(%d) is invalid or value looks out of range", __FUNCTION__, adcIndex));
                return -1;
            }
            else
            {
                BDBG_MSG(("%s:  AdcIndex set to %d ", __FUNCTION__, cfg->newAdc));
                cfg->newAdc = adcIndex;
                cfg->isNewAdc=true;
            }
        }

    }
    else if (strcmp(tmpBuf, "OFDM") == 0) {
        /* Complete string is */
        /* /LiveChannel\;Type=OFDM\;OfdmMode=DVBT|DVBT2|DVBC2|ISDBT\;Bandwidth=6000000;Freq=473000000;Terrestrial=Yes;SubChannel=1;*/
        /* cciMode=OFF|AUTO; */
        /* priority=LOW|HIGH; */
        cfg->srcType = IpStreamerSrc_eOfdm;
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Freq=", ";")) {
            BDBG_ERR(("%s: No Frequency Attribute found", __FUNCTION__));
            return -1;
        }
        cfg->frequency = strtol(tmpBuf, (char **)NULL, 10);
        BDBG_MSG(("frequency is %d", cfg->frequency));

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Bandwidth=", ";")) {
            BDBG_ERR(("%s: No Bandwidth Attribute found", __FUNCTION__));
            return -1;
        }
        cfg->bandwidth = strtol(tmpBuf, (char **)NULL, 10);
        BDBG_MSG(("%s: bandwidth is %d", __FUNCTION__, cfg->bandwidth));

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "OfdmMode=", ";")) {
            BDBG_ERR(("%s: No OFDM Mode set", __FUNCTION__));
            return -1;
        }
        if (strcmp(tmpBuf, "DVBT") == 0)
            cfg->ofdmMode = NEXUS_FrontendOfdmMode_eDvbt;
        else if (strcmp(tmpBuf, "DVBT2") == 0)
            cfg->ofdmMode = NEXUS_FrontendOfdmMode_eDvbt2;
        else if (strcmp(tmpBuf, "DVBC2") == 0)
            cfg->ofdmMode = NEXUS_FrontendOfdmMode_eDvbc2;
        else if (strcmp(tmpBuf, "ISDBT") == 0)
            cfg->ofdmMode = NEXUS_FrontendOfdmMode_eIsdbt;
        else {
            BDBG_MSG(("%s: TODO: OFDM mode %s is not yet supported", __FUNCTION__, tmpBuf));
            return -1;
        }
        BDBG_MSG(("OFDM Mode set to %d", cfg->ofdmMode));

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "CciMode=", ";") == 0) {
            if (strcmp(tmpBuf, "AUTO") == 0)
                cfg->cciMode = NEXUS_FrontendOfdmCciMode_eAuto;
            else if (strcmp(tmpBuf, "OFF") == 0)
                cfg->cciMode = NEXUS_FrontendOfdmCciMode_eNone;
            else
                cfg->cciMode = NEXUS_FrontendOfdmCciMode_eAuto;
        }
        else
            cfg->cciMode = NEXUS_FrontendOfdmCciMode_eAuto;
        BDBG_MSG(("cciMode Mode set to %d", cfg->cciMode));

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Priority=", ";") == 0) {
            if (strcmp(tmpBuf, "HIGH") == 0)
                cfg->ofdmPriority = NEXUS_FrontendOfdmPriority_eHigh;
            else if (strcmp(tmpBuf, "LOW") == 0)
                cfg->ofdmPriority = NEXUS_FrontendOfdmPriority_eLow;
            else
                cfg->ofdmPriority = NEXUS_FrontendOfdmPriority_eHigh;
        }
        else
            cfg->ofdmPriority = NEXUS_FrontendOfdmPriority_eHigh;
        BDBG_MSG(("priority set to %d", cfg->ofdmPriority));

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Terrestrial=", ";") == 0) {
            if (strcmp(tmpBuf, "Yes") == 0) {
                cfg->terrestrial = true;
                BDBG_MSG(("Terrestrial Param is set"));
            }
            else {
                cfg->terrestrial = false;
            }
        } else {
            BDBG_MSG(("Terrestrial Param is not set"));
            cfg->terrestrial = true;
        }
    }
#endif /* NEXUS_HAS_FRONTEND */
    else if (strcmp(tmpBuf, "HDMI") == 0) {
        /* Syntax is --> */
        /* /LiveChannel;Type=HDMI;InputId=0;TranscodeEnabled=Yes;AspectRatio=;Width=;Height=;BitRate= */
#if !NEXUS_HAS_HDMI_INPUT
        BDBG_ERR(("ERROR: HDMI Input is not enabled/available for this platform"));
        return -1;
#elif !NEXUS_HAS_VIDEO_ENCODER
        BDBG_ERR(("ERROR: HDMI Encoder is not enabled/available for this platform"));
        return -1;
#elif NEXUS_HAS_HDMI_INPUT
        cfg->srcType = IpStreamerSrc_eHdmi;
        /* TODO transcode: add any HDMI specific params */
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "InputId=", ";")) {
            BDBG_MSG(("HDMI InputId not set, defaulting to 0"));
            cfg->hdmiInputId = 0;
        }
        else {
            cfg->hdmiInputId = strtol(tmpBuf, (char **)NULL, 10);
            if (cfg->hdmiInputId >= NEXUS_NUM_HDMI_INPUTS) {
                BDBG_ERR(("ERROR: HDMI Input Id %d is not valid, Max for this platform %d", cfg->hdmiInputId, NEXUS_NUM_HDMI_INPUTS-1));
                return -1;
            }
        }
#endif /* NEXUS_HAS_HDMI_INPUT */
    }
    else if (strcmp(tmpBuf, "STREAMER") == 0) {
        /* /LiveChannel;Type=STREAMER;SubChannel=<number>;TranscodeEnabled=Yes; ...*/
        cfg->srcType = IpStreamerSrc_eStreamer;
    }
    /* Attributes common */
    /* EncEnabled=Yes;SubChannel=number;RecEnabled=Yes;FileName=/data/videos/rec0.mpg; */
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "SubChannel=", ";") == 0) {
        cfg->subChannel = strtol(tmpBuf, (char **)NULL, 10);
        BDBG_MSG(("Sub-Channel # is %d", cfg->subChannel));
    }
    else {
        BDBG_MSG(("No SubChannel Attribute found, assuming 1st channel"));
        cfg->subChannel = 1;
    }
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "StreamingEnabled=", ";") == 0) {
        /* by default, IP Streaming is always enabled unless specifically disabled */
        if (strcasecmp(tmpBuf, "No") == 0) {
            cfg->ipDstEnabled = false;
            BDBG_MSG(("IP Streaming is disabled "));
        }
    }

    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "EncEnabled=", ";") == 0) {
        if (!cfg->mediaProbeOnly && cfg->ipDstEnabled && strcmp(tmpBuf, "Yes") == 0) {
            struct sockaddr_in remoteAddr, selfAddr;
            int addrLen;

            addrLen = sizeof(remoteAddr);
            if (getpeername(cfg->streamingFd, (struct sockaddr *)&remoteAddr, (socklen_t *)&addrLen) != 0) {
                BDBG_ERR(("ERROR: Failed to obtain remote IP address, errno: %d \n", errno));
                perror("getpeername");
                return -1;
            }
            if (getsockname(cfg->streamingFd, (struct sockaddr *)&selfAddr, (socklen_t *)&addrLen) != 0) {
                BDBG_ERR(("ERROR: Failed to obtain self IP address, errno: %d \n", errno));
                perror("getsockname");
                return -1;
            }
            if (remoteAddr.sin_addr.s_addr == selfAddr.sin_addr.s_addr) {
                BDBG_MSG(("Disable DTCP/IP encryption for local client"));
                cfg->encryptionEnabled = false;
            } else {
                cfg->encryptionEnabled = true;
            }
            BDBG_MSG(("EncEnabled Param is set, will encrypt live stream"));
        }
    }
    else {
        BDBG_MSG(("EncEnabled Param set is not, assuming clear session "));
        cfg->encryptionEnabled = false;
    }

    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "PvrEncDecEnabled=", ";") == 0) {
        if (strcmp(tmpBuf, "Yes") == 0) {
            cfg->pvrEncryptionEnabled = true;
            BDBG_MSG(("PvrEncDecEnabled Param is set, will encrypt live stream"));
        }
    }
    else {
        BDBG_MSG(("PvrEncDecEnabled Param set is not, assuming clear session "));
        cfg->pvrEncryptionEnabled = false;
    }

    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "TimeshiftingEnabled=", ";") == 0) {
        if (strcmp(tmpBuf, "Yes") == 0) {
            cfg->enableTimeshifting = true;
            cfg->usePlaybackForStreamingFiles = true;
            cfg->enableTimestamps = true;
            BDBG_MSG(("Timeshifting Param is set"));
        }
    }
    else {
        BDBG_MSG(("Timeshifting Param is not set"));
        cfg->enableTimeshifting = false;
        cfg->usePlaybackForStreamingFiles = false;
        cfg->enableTimestamps = false;
    }

    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "RecEnabled=", ";") == 0) {
        if (strcmp(tmpBuf, "Yes") == 0) {
            cfg->recDstEnabled = true;
            BDBG_MSG(("RecEnabled Param is set"));
            if (parseToken(url, tmpBuf, sizeof(tmpBuf), "FileName=", ";") == 0) {
                if (strlen(tmpBuf) < sizeof(cfg->fileName) && strlen(tmpBuf) != 0) {
                    strncpy(cfg->fileName, tmpBuf, sizeof(cfg->fileName)-1);
                    BDBG_MSG(("record live stream to %s", cfg->fileName));
                }
                else {
                    BDBG_MSG(("Invalid or too long of a file name %s", cfg->fileName));
                    strncpy(cfg->fileName, "recFile.mpg", sizeof(cfg->fileName)-1);
                    BDBG_MSG(("defaulting record file name to %s ", cfg->fileName));
                }
            }
            else {
                strncpy(cfg->fileName, "recFile.mpg", sizeof(cfg->fileName)-1);
                BDBG_MSG(("recording file name is not specified, defaulting to %s", cfg->fileName));
            }
            strncpy(cfg->indexFileName, cfg->fileName, sizeof(cfg->indexFileName)-1);
            tmpPtr = strstr(cfg->indexFileName, ".");
            if (tmpPtr)
                *tmpPtr = '\0';
            strncat(cfg->indexFileName, "nav", sizeof(cfg->indexFileName)-1);
            BDBG_MSG(("index file %s", cfg->indexFileName));
        }
    }
    else {
        BDBG_MSG(("RecEnabled Param is not set, so not recording live session "));
        cfg->recDstEnabled = false;
    }
    /* Transcoding String */
    if (parseTranscodeOptions(openSettings, cfg)) {
        BDBG_ERR(("%s: Failed to parse transcode related parameters",__FUNCTION__));
        return -1;
    }
#if NEXUS_HAS_VIDEO_ENCODER
    if (cfg->transcodeEnabled) {
        cfg->transcode.nonRealTime = false; /* always use RT xcode mode for live sources */
    }
#endif
    if (cfg->hlsSession != true && parseToken(url, tmpBuf, sizeof(tmpBuf), "HlsEnabled=", ";") == 0) {
        if (strcmp(tmpBuf, "Yes") == 0) {
            cfg->hlsSession = true;
            BDBG_MSG(("HlsEnabled Param is set, will stream using HLS protocol"));
        }
    }
    return 0;

fileParsing:
    memset(tmpBuf, 0, sizeof(tmpBuf));
    /* /File=<name>;ByteRangeBegin=<start>;ByteRangeEnd=<end>; */
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "/File=", ";") == 0) {
        /* we got this URL via the local DMS as regular test client doesn't send such URL param to us */
        strncpy(cfg->fileName, tmpBuf, sizeof(cfg->fileName)-1);
        BDBG_MSG(("File %s playback requested by DMS", cfg->fileName));
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "MediaInfoDir=", ";") == 0) {
            /* we got this URL via the local DMS as regular test client doesn't send such URL param to us */
            strncpy(cfg->mediaInfoFilesDir, tmpBuf, sizeof(cfg->mediaInfoFilesDir)-1);
        }
        else {
            /* since app didn't specify mediaInfoDir, we use the media file directory for storing info & nav files */
            char *endPtr = NULL;
            char *tmp;
            int rootDirLen;

            /* extract the media file directory name by seaching the file name in the string */
            tmp = cfg->fileName;
            while ((tmp = strstr(tmp, "/")) != NULL) {
                endPtr = tmp;
                tmp += 1; /* move past "/" char */
            }
            /* now mediaRelativeFileNamePtr points to the relative media file name */
            if (endPtr && ((unsigned)(rootDirLen = endPtr - cfg->fileName)) < sizeof(cfg->mediaInfoFilesDir)) {
                strncpy(cfg->mediaInfoFilesDir, cfg->fileName, rootDirLen);
                cfg->mediaInfoFilesDir[rootDirLen] = '\0';
            }
            else {
                BDBG_ERR(("%s: media file name (%s) needs to contain absolute path", __FUNCTION__, cfg->fileName));
                return -1;
            }
        }
        BDBG_MSG(("Media Info files dir is %s", cfg->mediaInfoFilesDir));
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "ByteRangeBegin=", ";") == 0) {
            cfg->beginFileOffset = strtoll(tmpBuf, (char **)NULL, 10);
            BDBG_MSG(("Byte Range: Begin %lld", cfg->beginFileOffset));
            cfg->fileOffsetEnabled = true;
        }
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "ByteRangeEnd=", ";") == 0) {
            cfg->endFileOffset = strtoll(tmpBuf, (char **)NULL, 10);
            BDBG_MSG(("Byte Range: End %lld", cfg->endFileOffset));
            cfg->fileOffsetEnabled = true;
        }
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "EncEnabled=", ";") == 0) {
            if (strcmp(tmpBuf, "Yes") == 0) {
                cfg->encryptionEnabled = true;
                BDBG_MSG(("EncEnabled Param is set, will encrypt live stream"));
            }
        }
        else {
            BDBG_MSG(("EncEnabled Param set is not, assuming clear session"));
            cfg->encryptionEnabled = false;
        }
        /* Parse for ByteRangeStart/End, || TimeRangeStart/End, PlaySpeed, */
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "PlaySpeed=", ";") == 0) {
            cfg->playSpeed = strtol(tmpBuf, (char **)NULL, 10);
            BDBG_MSG(("PlaySpeed %d", cfg->playSpeed));
        }
        else
            cfg->playSpeed = 1;
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "TimeOffset=", ";") == 0) {
            #if ANDROID
            cfg->beginTimeOffset = strtod(tmpBuf, (char **)NULL);
            #else
            cfg->beginTimeOffset = strtold(tmpBuf, (char **)NULL);
            #endif /* ANDROID */
            BDBG_MSG(("TimeOffset %.3f", cfg->beginTimeOffset));
        }
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "endTimeOffset=", ";") == 0) {
            #if ANDROID
            cfg->endTimeOffset = strtod(tmpBuf, (char **)NULL);
            #else
            cfg->endTimeOffset = strtold(tmpBuf, (char **)NULL);
            #endif /* ANDROID */
            BDBG_MSG(("TimeOffset %.3f", cfg->endTimeOffset));
        }
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Address=", ";") == 0) {
            cfg->streamingCfg.streamingIpAddress = strdup(tmpBuf);
            BDBG_MSG(("Streaming to Host %s", cfg->streamingCfg.streamingIpAddress));
        }
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Port=", ";") == 0) {
            cfg->streamingCfg.streamingPort = strtol(tmpBuf, (char **)NULL, 10);
            BDBG_MSG(("port is %d", cfg->streamingCfg.streamingPort));
        }

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Protocol=", ";") == 0) {
            BDBG_MSG(("Protocol is %s", tmpBuf));
            if (strcmp(tmpBuf, "UDP") == 0)
                cfg->streamingCfg.streamingProtocol = B_PlaybackIpProtocol_eUdp;
            else if (strcmp(tmpBuf, "RTP") == 0)
                cfg->streamingCfg.streamingProtocol = B_PlaybackIpProtocol_eRtp;
            else
                cfg->streamingCfg.streamingProtocol = B_PlaybackIpProtocol_eHttp;
        }
        else {
            BDBG_MSG(("No Protocol Attribute found, assuming HTTP"));
            cfg->streamingCfg.streamingProtocol = B_PlaybackIpProtocol_eHttp;
        }
        if (cfg->streamingCfg.streamingProtocol == B_PlaybackIpProtocol_eUdp || cfg->streamingCfg.streamingProtocol == B_PlaybackIpProtocol_eRtp)
            cfg->streamingCfg.url = strdup(cfg->fileName);
        cfg->srcType = IpStreamerSrc_eFile;

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "PvrDecEnabled=", ";") == 0) {
            if (strcmp(tmpBuf, "Yes") == 0) {
                cfg->pvrDecryptionEnabled = true;
                BDBG_MSG(("PvrEncEnabled Param is set, will decrypt the encrypted stream"));
            }
        }
        else {
            BDBG_MSG(("PvrDecEnabled Param set is not, assuming clear session "));
            cfg->pvrDecryptionEnabled = false;
        }

        if (parseTranscodeOptions(openSettings, cfg)) {
            BDBG_ERR(("%s: Failed to parse transcode related parameters",__FUNCTION__));
            return -1;
        }
#if NEXUS_HAS_VIDEO_ENCODER
        if (cfg->transcodeEnabled) {
            cfg->usePlaybackForStreamingFiles = true; /* for xcoding files, we use playback channel to feed the files to xpt */
            cfg->transcode.nonRealTime = true; /* always use NRT modes for files */
        }
#endif
        if (cfg->hlsSession != true && parseToken(url, tmpBuf, sizeof(tmpBuf), "HlsEnabled=", ";") == 0) {
            if (strcmp(tmpBuf, "Yes") == 0) {
                cfg->hlsSession = true;
                BDBG_MSG(("HlsEnabled Param is set, will stream using HLS protocol"));
            }
        }
        return 0;
    }

    /* we got this URL from a test client (not DLNA client), so need to add root dir to file name */
    if (parseToken(url, tmpBuf, sizeof(tmpBuf), "/", " ") == 0) {
        /* we got this URL directly from the regular test client */
        /* look for any additional arguments appended to the URL */
        tmpPtr = strstr(tmpBuf, ";");
        if (tmpPtr)
            /* ; is present in the URL, so it contains additional arguments, replace it w/ null char so that we get get the exact file name */
            *tmpPtr = '\0';
        cfg->fileName[sizeof(cfg->fileName)-1] = '\0';
        strncpy(cfg->fileName, gRootDir, sizeof(cfg->fileName)-1);
        strncat(cfg->fileName, "/", 1);
        strncat(cfg->fileName, tmpBuf, sizeof(cfg->fileName)-1);

        BDBG_MSG(("File %s playback requested by ip test client", cfg->fileName));
        strncpy(cfg->mediaInfoFilesDir, gRootDir, sizeof(cfg->mediaInfoFilesDir)-1);
        cfg->mediaInfoFilesDir[sizeof(cfg->mediaInfoFilesDir)-1] = '\0';
        BDBG_MSG(("Media Info files dir is %s", cfg->mediaInfoFilesDir));
        if (tmpPtr)
            /* replace back the ; */
            *tmpPtr = ';';

        if (parseTranscodeOptions(openSettings, cfg)) {
            BDBG_ERR(("%s: Failed to parse transcode related parameters", __FUNCTION__));
            return -1;
        }
#if NEXUS_HAS_VIDEO_ENCODER
        if (cfg->transcodeEnabled) {
            cfg->usePlaybackForStreamingFiles = true; /* for xcoding files, we use playback channel to feed the files to xpt */
            cfg->transcode.nonRealTime = true; /* always use NRT modes for files */
        }
#endif
        /* Parse for ByteRangeStart/End, || TimeRangeStart/End, PlaySpeed, */
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Range.dtcp.com:", "\r\n") == 0) {
            char tmpBuf1[64];
            tmpPtr = tmpBuf;    /* points to "bytes=" string */
            if (parseToken(tmpBuf, tmpBuf1, sizeof(tmpBuf1), "bytes=", "-") == 0) {
                cfg->beginFileOffset = strtoll(tmpBuf1, (char **)NULL, 10);
                BDBG_MSG(("Byte Range: Start %lld", cfg->beginFileOffset));
            }
            tmpPtr[strlen(tmpPtr)] = '\n';
            if (parseToken(tmpPtr, tmpBuf1, sizeof(tmpBuf1), "-", "\n") == 0) {
                cfg->endFileOffset = strtoll(tmpBuf1, (char **)NULL, 10);
                BDBG_MSG(("Byte Range: End %lld", cfg->endFileOffset));
            }
        }
        else if (parseToken(url, tmpBuf, sizeof(tmpBuf), "Range:", "\r\n") == 0) {
            char tmpBuf1[64];
            tmpPtr = tmpBuf;    /* points to "bytes=" string */
            if (parseToken(tmpBuf, tmpBuf1, sizeof(tmpBuf1), "bytes=", "-") == 0) {
                cfg->beginFileOffset = strtoll(tmpBuf1, (char **)NULL, 10);
                BDBG_MSG(("Byte Range: Start %lld", cfg->beginFileOffset));
                cfg->fileOffsetEnabled = true;
            }
            tmpPtr[strlen(tmpPtr)] = '\n';
            if (parseToken(tmpPtr, tmpBuf1, sizeof(tmpBuf1), "-", "\n") == 0) {
                cfg->endFileOffset = strtoll(tmpBuf1, (char **)NULL, 10);
                BDBG_MSG(("Byte Range: End %lld", cfg->endFileOffset));
                cfg->fileOffsetEnabled = true;
            }
        }
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "EncEnabled: ", "\r\n") == 0) {
            if (strcmp(tmpBuf, "Yes") == 0) {
                cfg->encryptionEnabled = true;
                BDBG_MSG(("EncEnabled Param is set, will encrypt file being streamed"));
            }
        }
        else {
            BDBG_MSG(("EncEnabled Param set is not, assuming clear session "));
            cfg->encryptionEnabled = false;
        }
        cfg->srcType = IpStreamerSrc_eFile;

        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "PlaySpeed.dlna.org: speed=", "\r\n") == 0) {
            int speedNumerator =1, speedDenominator=1, direction=1;
            if (B_PlaybackIp_UtilsParsePlaySpeedString(tmpBuf, &speedNumerator, &speedDenominator, &direction) < 0) {
                BDBG_ERR(("%s: Failed to parse the playSpeedString %s", __FUNCTION__, tmpBuf));
                return B_ERROR_UNKNOWN;
            }
            if ( abs(speedDenominator) > abs(speedNumerator) )
                /* for fractional speeds, we will transmit as 1X and let the client control the playback rate */
                cfg->playSpeed = 1*direction;
            else
                cfg->playSpeed = speedNumerator/speedDenominator;
            BDBG_MSG(("Play Speed %d", cfg->playSpeed));
        }
        if (parseToken(url, tmpBuf, sizeof(tmpBuf), "TimeSeekRange.dlna.org: npt=", "\r\n") == 0) {
            cfg->beginTimeOffset = strtod(tmpBuf, (char **)NULL);
            BDBG_MSG(("start time offset %.3f", cfg->beginTimeOffset));
            /* find end time offset */
            tmpPtr = strstr(tmpBuf, "-");
            cfg->endTimeOffset = strtod(tmpPtr+1, (char **)NULL);
            BDBG_MSG(("end time offset %.3f", cfg->endTimeOffset));
        }
        cfg->streamingCfg.streamingProtocol = B_PlaybackIpProtocol_eHttp;
        if (cfg->hlsSession != true && parseToken(url, tmpBuf, sizeof(tmpBuf), "HlsEnabled=", ";") == 0) {
            if (strcmp(tmpBuf, "Yes") == 0) {
                cfg->hlsSession = true;
                BDBG_MSG(("HlsEnabled Param is set, will stream using HLS protocol"));
            }
        }
        return 0;
    }
    return -1;
}

char *
acceptNewHttpRequest(
        int listeningFd,
        char *requestUrl,
        int requestUrlLen,
        int *newFd
        )
{
    struct sockaddr_in remoteAddr;
    int addrLen = sizeof(remoteAddr);
    int nbytes = 0;
    int sockFd = 0;

    /* wait for HTTP request & send HTTP reply */
    while (!gExitThread) {
        waitForNetworkEvent(listeningFd);

        /* accept connection */
        if ((sockFd = accept(listeningFd, (struct sockaddr *)&remoteAddr, (socklen_t *)&addrLen)) < 0) {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            perror("ERROR: accept(): exiting...");
            break;
        }

        BKNI_Memset(requestUrl, 0, requestUrlLen);
        do {
            waitForNetworkEvent(sockFd);
            /* Read HTTP request */
            /* coverity[string_null_argument: FALSE] */
            if ((nbytes += read(sockFd, requestUrl+nbytes, requestUrlLen-nbytes-1)) <= 0) {
                perror("read failed to read the HTTP Get request");
                break;
            }
            /* coverity[string_null: FALSE] */
        } while (strstr(requestUrl, "\r\n\r\n") == NULL);
        if (nbytes <= 0)
            break;
        requestUrl[nbytes] = '\0';
        BDBG_MSG(("Read HTTP Req (socket %d, %d bytes)", sockFd, nbytes));

        *newFd = sockFd;
        /* we dont do any parsing part here */
        return requestUrl;
    }
    BDBG_MSG(("Failed handling HttpRequest..."));
    if (sockFd > 0)
        close(sockFd);
    requestUrl[0] = '\0';
    return NULL;
}

void
closeHttpSession(int sd)
{
    BDBG_MSG(("%s: Closing socket %d", __FUNCTION__, sd));
    close(sd);
}
