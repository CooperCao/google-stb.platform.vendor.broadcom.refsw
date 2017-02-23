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
 *
 * Module Description:
 *  Example program to demonstrate receiving live or playback content over IP Channels (UDP/RTP/RTSP/HTTP based)
 *
 ******************************************************************************/
#include "ip_includes.h"
#ifdef B_HAS_DTCP_IP
#include "b_dtcp_applib.h"
#endif
#include "b_os_lib.h"

#include "ip_psi.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_utils.h"
#include "nexus_core_utils.h"
#include "nexus_video_decoder_primer.h"
#include "nexus_video_decoder_extra.h"
#include "bmedia_probe.h"
#include "nexus_dma.h"
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif
#ifdef LIVEMEDIA_SUPPORT
#include "RTSPServer.hh"
#endif /* LIVEMEDIA_SUPPORT */
BDBG_MODULE(ip_client);
#define CA_CERT "./host.cert"

/* Test App to demonstrate the Live IP tuning using Nexus IP Applib */
#define IP_NETWORK_JITTER 300
#define TOTAL_PRIMERS 1
#define MPEG_DASH_MAX_SEGMENT_SIZE  (24*1024*1024)

/*TODO - For now use these three Nexus as globals.  Should ideally be placed */
/*       within context varible of HDMI callback                           */
NEXUS_AudioDecoderStartSettings audioProgram;
NEXUS_AudioDecoderHandle pcmDecoder = NULL, compressedDecoder = NULL;

bool audioStarted = false;

typedef struct {
    int iphVersion;
    char host[256];     /* IP Address of Receiving Host */
    int port;     /* Port # */
    char *url;      /* URL */
    B_PlaybackIpProtocol protocol; /* Protocol: UDP/RTP */
    bool useLiveIpMode;
    bool decoderStats;
    B_PlaybackIpSecurityProtocol security;  /* which security protocol to use */
    unsigned int preChargeTime;  /* how many seconds to precharge the buffers before display begings */
    int initialSeekTime; /* how many seconds to seek into the media before starting the initial playback */
    unsigned decodeTimeDuration; /*For how many seconds the live decode/playback will persist  */
    bool loop; /* loop around at end */
    bool playMp3; /* play MP3 */
    bool playLpcm; /* play LPCM */
    int secondsToJump;
    int dtcpAkePort;
    unsigned alternateAudioNumber;
    int dtcpKeyFormat;
    bool ckc_check;
    bool slaveMode;
    bool runUnitTests;
    bool playMultipleStreams;
    bool skipPsiParsing;
    bool fastChannelChange;
    bool gStreamerMode; /* simulates gstreamer mode where app pulls the demuxed & decrypted data from playback_ip and feeds it directly to nexus */
    bool liveChannel; /* set if app knows it is going to receive a live channel */
    int hevc; /* 1 for 1080p24, 2 for 2160p24, 3 for 1080p60, 4 for 2160p60, 0 by default for non-hevc content */
    unsigned short maxWidth;
    unsigned short maxHeight;
    bool usePlaybackIpForPsiProbing;
}ExampleIpCfg;

int feedToPlayback = 0;

void usage(void)
{
    printf("Usage: ip_client [options] [<URI>]\n");
    printf("Usage: ip_client -d <ip> -p <port> [-t <num>] [-u <url>] [-v <num>] [-S <num>] [-b <num>] [-i <num>] [-l <num>] [-T <num>] -j <num> [-H <num>] [-c] [-y] [-k] [-f] [-G] [-h] \n");
    printf("options are:\n");
    printf(" <uri>          # Complete URI specifying protocol, server, port number and url\n");
    printf("                # e.g. nexus ip_client -S 2 http://192.168.1.109:5000/AbcMpeg2HD.mpg\n");
    printf("                # e.g. nexus ip_client https://192.168.1.109/AbcMpeg2HD.mpg\n");
    printf("                # e.g. nexus ip_client udp://192.168.1.109:1234\n");
    printf("                # e.g. nexus ip_client rtp://192.168.1.109:1234\n");
    printf(" -d <ip>        # IP address of Live IP Channel (e.g. 192.168.1.110)\n");
    printf(" -p <port>      # Port of Live IP Channel (e.g. 1234)\n");
    printf(" -t <0|1|2|3|4> # protocol: UDP(0)|RTP(1)|RTSP(2)|HTTP(3)|RTP w/o RTCP(4), default is UDP\n");
    printf(" -u <url>       # URL for RTSP & HTTP protocols\n");
    printf(" -v             # IP version to use: IPv4: 4, IPv6: 6, default is IPv4\n");
    printf(" -s             # print decoder stats\n");
    printf(" -S <0|1|2|3|4> # Security: none(0) | SSL(1) | DTCP-IP(2) | RAD-EA(3) | AES128(4), default is none\n");
    printf(" -b <time>      # pre-charge the buffers for <time> seconds, default is 0\n");
    printf(" -i <time>      # initial seek time in seconds, default is 0\n");
    printf(" -j <num>       # jump forward/backward by these many seconds when j is pressed\n");
    printf(" -l <num>       # loop around after end, default is false\n");
    printf(" -T <num>       # number of seconds the live decode/playback will continue for\n");
    printf(" -n <ake-port>  # DTCP/IP AKE Port, default 8000\n");
    printf(" -K <0|1|2>     # DTCP/IP key format: test(0) | common DRM(1) | legacy production(2), default is test\n");
    printf(" -C             # DTCP/IP: Enable Content Key Confirmation procedure for Sink device\n");
    printf(" -m             # play MP3 (skips media probing)\n");
    printf(" -z             # run in slave mode\n");
    printf(" -y             # run basic unit tests on the stream (seek, pause, ff, fr, etc.)\n");
    printf(" -c             # play lpcm file\n");
    printf(" -k             # skip psi parsing (PSI info is harded in ip_client.c) \n");
    printf(" -f             # use fast channel \n");
    printf(" -x             # play multiple files back to back (channel change)\n");
    printf(" -G             # gstreamer mode (app pulls demuxed & decrypted data & directly feeds to nexus)\n");
    printf(" -L             # play a liveChannel (mainly for HTTP protocol as RTP/UDP is always live) \n");
    printf(" -P             # Use PlaybackIp Media Probing for PSI Parsing\n");
    printf(" -H<1 | 2>      # play a HEVC/H265 Channel: 1 ==> 1080p24; 2 ==> 2160p24; 3 ==> 1080p60; 4 ==> 2160p60\n");
    printf(" -h             # prints this usage\n");
    printf(" env variables:\n");
    printf(" additionalHeader=<Browser Cookie String>      # set it if URL is obtained via a Browser session requiring a HTTP Cookie\n");
    printf(" setupTimeout=<SessionSetupTimeout in msec>        # set this to high value in sec (say 20sec) if receiving HTTP session from a Internet sever \n");
    printf(" userAgent=<custom userAgent string>               # userAgent string\n");
    printf(" useProxy=<1 | 0>                                  # set to 1 if all outgoing HTTP requests must flow thru a proxy (whose address is specified via the -d option) \n");
    printf("\n");
}

char  *_getEnvVariableValue(char *pName, char *defaultValue)
{
    char *value;
    int valueLen;
    char *pValue;

    pValue = getenv(pName);

    if (pValue != NULL) {
        value = pValue;
        if (strncmp(pName, "additionalHeader", strlen("additionalHeader")) == 0) {
            valueLen = strlen(pValue)+2;
            if ((value = malloc(valueLen)) != NULL) {
                strncpy(value, pValue, strlen(pValue));
                strncat(value, "\r\n", valueLen-1);
            }
            else {
                return NULL;
            }
        }
    }
    else{
        value = defaultValue;
    }

    BDBG_MSG(("%s: %s = %s", __FUNCTION__, pName, value));
    return value;
}


ssize_t read_with_timeout(int fd, void *buf, size_t count, unsigned timeInMs)
{
    struct timeval select_timeout;
    fd_set fds;
    ssize_t retval = 0;

    select_timeout.tv_sec = timeInMs / 1000;
    select_timeout.tv_usec = (timeInMs % 1000) * 1000;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    retval = select(fd+1, &fds, NULL, NULL, &select_timeout);
    if (retval == -1) return retval;    /* select error */

    retval = 0;
    if (FD_ISSET(fd, &fds))
    {
        retval = read(fd,buf,count);
    }
    return retval;
}


static void set_default_ip_cfg(ExampleIpCfg *ipCfg)
{
    memset(ipCfg, 0, sizeof(ExampleIpCfg));

    ipCfg->protocol = B_PlaybackIpProtocol_eUdp;
    ipCfg->iphVersion = 4;
    ipCfg->url = NULL;
    ipCfg->decoderStats = false;
    ipCfg->decodeTimeDuration = 0;
    ipCfg->preChargeTime = 0;
    ipCfg->security = B_PlaybackIpSecurityProtocol_None;
    ipCfg->initialSeekTime = 0;
    ipCfg->loop = false;
    ipCfg->playMp3 = false;
    ipCfg->dtcpAkePort = 8000;
    ipCfg->alternateAudioNumber = 0;
    ipCfg->playLpcm = false;
    ipCfg->secondsToJump = 5;
    ipCfg->slaveMode = false;
    ipCfg->runUnitTests = false;
    ipCfg->playMultipleStreams = false;
    ipCfg->fastChannelChange = false;
    ipCfg->gStreamerMode = false;
    ipCfg->liveChannel = false;
    ipCfg->hevc = 0;
    ipCfg->maxWidth = 4096;
    ipCfg->maxHeight = 2160;
    ipCfg->usePlaybackIpForPsiProbing = false;
}

extern char *B_PlaybackIp_UtilsStristr(char *str, char *subStr);
int parse_url(char *urlstr, ExampleIpCfg *cfg)
{
    char *tmpPtr, *tmpPtr1, *tmpPtr2;

    /* valid exampled of urlstr: */
    /* "http://player.vimeo.com:80/play_redirect?quality=hd&codecs=h264&clip_id=638324" */
    /* "/play_redirect?quality=hd&codecs=h264&clip_id=638324" */

    tmpPtr = strstr(urlstr, "\"");
    if (tmpPtr) {
        urlstr += 1;
        urlstr[strlen(urlstr)-1] = '\0';
    }
    tmpPtr = strstr(urlstr, ":");
    if (tmpPtr) {
        *tmpPtr = '\0';
        if (B_PlaybackIp_UtilsStristr(urlstr,"udp")) {
            cfg->protocol = B_PlaybackIpProtocol_eUdp;
        }
        else if (B_PlaybackIp_UtilsStristr(urlstr,"rtp")) {
            cfg->protocol = B_PlaybackIpProtocol_eRtp;
        }
        else if (B_PlaybackIp_UtilsStristr(urlstr,"rtsp")) {
            cfg->protocol = B_PlaybackIpProtocol_eRtsp;
        }
        else if (B_PlaybackIp_UtilsStristr(urlstr,"https")) {
            cfg->protocol = B_PlaybackIpProtocol_eHttp;
            if (!cfg->port)
                cfg->port = 443;
            cfg->security = B_PlaybackIpSecurityProtocol_Ssl;
        }
        else if (B_PlaybackIp_UtilsStristr(urlstr,"http")) {
            cfg->protocol = B_PlaybackIpProtocol_eHttp;
            if (!cfg->port)
                cfg->port = 80;
        }
        else {
            BDBG_ERR(("Incorrect or not yet supported Protocol Option (%d)", cfg->protocol));
            return -1;
        }
    }
    else {
        /* not a complete absolute url */
        cfg->url = urlstr;
        return 0;
    }

    /* restore the original string */
    *tmpPtr = ':';
    tmpPtr +=3; /* now tmpPtr points to the start of the server name */
    tmpPtr1 = strstr(tmpPtr, "/");
    /* parse for host name & port number */
    if (tmpPtr1)
        *tmpPtr1 = '\0';
    tmpPtr2 = strstr(tmpPtr, ":");
    if (tmpPtr2) {
        *tmpPtr2 = '\0';
        cfg->port = atoi(tmpPtr2+1);
    }
    memset(cfg->host, 0, sizeof(cfg->host));
    strncpy(cfg->host, tmpPtr, sizeof(cfg->host)-1);
    if (tmpPtr1)
        *tmpPtr1 = '/';
    /* rest of the string is the URL */
    cfg->url = tmpPtr1;
    return 0;
}

static int parse_options(int argc, char *argv[], ExampleIpCfg *ipCfg)
{
    int ch;
    while ( (ch = getopt(argc, argv, "a:d:p:v:t:T:u:sS:b:i:l:j:n:K:zmcxykfhCGLH:P")) != -1) {
        switch (ch) {
        case 'a':
            ipCfg->alternateAudioNumber = atoi(optarg); /* alternateAudio's position number, 1st, 2nd alternate audio, etc. */
            break;
        case 'd':
            strncpy(ipCfg->host, optarg, sizeof(ipCfg->host)-1);
            break;
        case 'p':
            ipCfg->port = atoi(optarg);
            break;
        case 'n':
            ipCfg->dtcpAkePort = atoi(optarg);
            break;
        case 'K':
            ipCfg->dtcpKeyFormat = atoi(optarg);
            break;
        case 'C':
            ipCfg->ckc_check = true;
            break;
        case 't':
            ipCfg->protocol = (B_PlaybackIpProtocol)atoi(optarg);
            if (ipCfg->protocol >= B_PlaybackIpProtocol_eMax) {
                BDBG_ERR(("Incorrect Protocol Option (%d)\n", ipCfg->protocol));
                goto usage;
            }
            break;
        case 'u':
            ipCfg->url = optarg;
            BDBG_MSG(("uri: %s", optarg));
            break;
        case 'v':
            ipCfg->iphVersion = atoi(optarg);
            if (ipCfg->iphVersion != 4 && ipCfg->iphVersion != 6) {
                BDBG_ERR(("Incorrect IP Version (%d)\n", ipCfg->iphVersion));
                goto usage;
            }
            break;
        case 'f':
            ipCfg->fastChannelChange = true;
            break;
        case 's':
            ipCfg->decoderStats = true;
            break;
        case 'S':
            ipCfg->security = (B_PlaybackIpSecurityProtocol)atoi(optarg);
            if (ipCfg->security > B_PlaybackIpSecurityProtocol_Max) {
                BDBG_ERR(("Incorrect Security Option (%d)\n", ipCfg->security));
                goto usage;
            }
            break;
        case 'b':
            ipCfg->preChargeTime = atoi(optarg);
            break;
        case 'T':
            ipCfg->decodeTimeDuration = atoi(optarg);
            break;
        case 'i':
            ipCfg->initialSeekTime = atoi(optarg);
            if (ipCfg->initialSeekTime < 0) {
                BDBG_ERR(("Incorrect initial seek time (%d)\n", ipCfg->initialSeekTime));
                goto usage;
            }
            break;
        case 'j':
            ipCfg->secondsToJump = atoi(optarg);
            break;
        case 'l':
            ipCfg->loop = atoi(optarg);
            break;
        case 'm':
            ipCfg->playMp3 = true;
            break;
        case 'z':
            ipCfg->slaveMode = true;
            break;
        case 'c':
            ipCfg->playLpcm = true;
            break;
        case 'y':
            ipCfg->runUnitTests = true;
            break;
        case 'x':
            ipCfg->playMultipleStreams = true;
            break;
        case 'k':
            ipCfg->skipPsiParsing = true;
            break;
        case 'G':
            ipCfg->gStreamerMode = true;
            break;
        case 'L':
            ipCfg->liveChannel = true;
            break;
        case 'P':
            ipCfg->usePlaybackIpForPsiProbing = true;
            break;
        case 'H':
            ipCfg->hevc = atoi(optarg);
            BDBG_WRN(("hevc = %d", ipCfg->hevc));
            if (ipCfg->hevc == 0) {
                BDBG_ERR(("Provide a display format value for -H option: 1 ==> 1080p24; 2 ==> 2160p24; 3 ==> 1080p60; 4 ==> 2160p60  e.g. -H2 for full HEVC 24Hz display"));
                usage();
                exit(1);
            }
            break;
        case 'h':
        default:
usage:
            usage();
            exit(1);
        }
    }


    /* coverity[tainted_data] */
    if (ipCfg->url == NULL && argc > 1) {
        if (parse_url(argv[argc-1],ipCfg) < 0) {
            BDBG_ERR(("Incorrect uri: %s", optarg));
            goto usage;
        }
        BDBG_MSG (("URL is %s:%d%s", ipCfg->host, ipCfg->port, ipCfg->url));
    }


    if ((ipCfg->protocol == B_PlaybackIpProtocol_eRtsp || ipCfg->protocol == B_PlaybackIpProtocol_eHttp) && (ipCfg->url == NULL)) {
        BDBG_ERR(("ERROR: RTSP & HTTP protocols require URL option\n"));
        usage();
        return -1;
    }
    BDBG_WRN(("Options are: \n\tHost: %s\n\tPort %d\n\tProtocol %s\n\tUrl %s\n\tIpVersion %d\n",
            ipCfg->host, ipCfg->port,
            ipCfg->protocol == B_PlaybackIpProtocol_eUdp ? "UDP" :
            ipCfg->protocol == B_PlaybackIpProtocol_eRtp ? "RTP" :
            ipCfg->protocol == B_PlaybackIpProtocol_eRtpNoRtcp ? "RTP no RTCP" :
            ipCfg->protocol == B_PlaybackIpProtocol_eRtsp ? "RTSP" : "HTTP",
            ipCfg->url,
            ipCfg->iphVersion));
    return 0;
}



ExampleIpCfg ipCfg;
int ip_client_setup(int argc, char *argv[])
{

    /* set the IP socket related defults for program */
    set_default_ip_cfg(&ipCfg);

    /* parse command line options */
    if (parse_options(argc, argv, &ipCfg)) {
        BDBG_WRN(("ERROR: Incorrect Options\n"));
        goto error;
    }

    return 0;

error:
    return 1;
}

void setDefaultPsiSettings(B_PlaybackIpPsiInfo *psi)
{
    memset(psi, 0, sizeof(*psi));
    psi->psiValid = true;
    psi->videoCodec = NEXUS_VideoCodec_eMpeg2;
    psi->videoPid = 0x11;
    psi->audioCodec = NEXUS_AudioCodec_eAc3;
    psi->audioPid = 0x14;
    psi->pcrPid = 0x11;
    psi->mpegType = NEXUS_TransportType_eTs;
    BDBG_MSG (("Setting Default PSI Settings: vpid %d, vcodec %d, apid %d, acodec %d, container %d", psi->videoPid, psi->videoCodec, psi->audioPid, psi->audioCodec, psi->mpegType));
}

bool gEof = false;
static void errorCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    BDBG_ERR((" #### Error callback, let the playback finish via the endOfStreamCallback #### \n"));
}

static void endOfStreamCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    gEof = true;
    BDBG_ERR((" #### End of stream reached #### \n"));
}

/* returns 0 for successful buffering or -1 otherwise */
int preChargeNetworkBuffer(B_PlaybackIpHandle playbackIp, unsigned int preChargeTime /* in secs */)
{
    NEXUS_Error rc;
    B_PlaybackIpSettings playbackIpSettings;
    B_PlaybackIpStatus playbackIpStatus;
    NEXUS_PlaybackPosition prevBufferDuration = 0;
    int noChangeInBufferDepth = 0;

    /* check for EOF condition */
    if (gEof) {
        BDBG_WRN(("Buffering Aborted due to EOF\n"));
        return -1;
    }

    /* check how much data is currently buffered: this is from the last seek point */
    BKNI_Sleep(100);    /* sleep little bit to get IP thread finish off any buffering */
    rc = B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatus);
    if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); return (-1);}
    if (playbackIpStatus.curBufferDuration >= (1000*preChargeTime)) {
        BDBG_WRN(("Already buffered %lu milli-sec of data, required %u milli-sec of buffer...\n", playbackIpStatus.curBufferDuration, preChargeTime*1000));
        return 0;
    }

    /* underflowing in network buffer, tell IP Applib to start buffering */
    BDBG_WRN(("Start pre-charging n/w buffer: currently buffered %lu milli-sec of data, required %u milli-sec of buffer...\n", playbackIpStatus.curBufferDuration, preChargeTime*1000));
    memset(&playbackIpSettings, 0, sizeof(playbackIpSettings));
    /* coverity[stack_use_local_overflow] */
    /* coverity[stack_use_overflow] */
    playbackIpSettings.preChargeBuffer = true;
    rc = B_PlaybackIp_SetSettings(playbackIp, &playbackIpSettings);
    if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); return (-1);}

    /* now monitor the buffer depth until it reaches the desired preChargeTime value */
    do {
        BKNI_Sleep(100);
        rc = B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatus);
        if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); return (-1);}
        BDBG_WRN(("currently buffered %lu milli-sec of data, max duration %lu, required %u milli-sec of buffer...\n", (unsigned long)playbackIpStatus.curBufferDuration, (unsigned long)playbackIpStatus.maxBufferDuration, preChargeTime*1000));
        if (playbackIpStatus.curBufferDuration >= playbackIpStatus.maxBufferDuration) {
            BDBG_WRN(("currently buffered %lu max duration %lu milli-sec worth data, so done buffering \n", (unsigned long)playbackIpStatus.curBufferDuration, (unsigned long)playbackIpStatus.maxBufferDuration));
            break;
        }
        if (!prevBufferDuration)
            prevBufferDuration = playbackIpStatus.curBufferDuration;
        else {
            if (prevBufferDuration == playbackIpStatus.curBufferDuration)
                noChangeInBufferDepth++;
            else  {
                noChangeInBufferDepth = 0;
                prevBufferDuration = playbackIpStatus.curBufferDuration;
            }
        }
        if (noChangeInBufferDepth >= 1000) {
            BDBG_WRN(("Warning: can't buffer upto the required buffer depth, currently buffered %lu max duration %lu milli-sec worth data, so done buffering \n", (unsigned long)playbackIpStatus.curBufferDuration, (unsigned long)playbackIpStatus.maxBufferDuration));
            break;
        }
        /* keep buffering until we have buffered upto the high water mark or eof/server close events happen */
    } while (playbackIpStatus.curBufferDuration < (1000*preChargeTime) && !gEof && !playbackIpStatus.serverClosed);

    /* tell IP Applib to stop buffering */
    playbackIpSettings.preChargeBuffer = false;
    /* coverity[stack_use_local_overflow] */
    /* coverity[stack_use_overflow] */
    rc = B_PlaybackIp_SetSettings(playbackIp, &playbackIpSettings);
    if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); return (-1);}

    if (gEof) {
        BDBG_WRN(("Buffering Aborted due to EOF, buffered %lu milli-sec worth of data...\n", (unsigned long)playbackIpStatus.curBufferDuration));
        return -1;
    }
    else if (playbackIpStatus.serverClosed) {
        BDBG_WRN(("Can't Buffer anymore due to server closed connection, buffered %lu milli-sec worth of data...\n", (unsigned long)playbackIpStatus.curBufferDuration));
        /* Note: this is not an error case as server may have closed connection, but we may not have played all this data, so let playback finish */
        return 0;
    }
    else {
        BDBG_WRN(("Buffering Complete (buffered %lu milli-sec worth of data), serverClosed %d...\n", (unsigned long)playbackIpStatus.curBufferDuration, playbackIpStatus.serverClosed));
        return 0;
    }
}

#define EVENT_WAIT_TIME_IN_MSEC 30000
B_PlaybackIpEventIds gEventId = B_PlaybackIpEvent_eMax;
BKNI_EventHandle waitEvent = NULL;
static bool gResumePlay = 0;
static bool gEndOfSegments = 0;
void playbackIpEventCallback(void *appCtx, B_PlaybackIpEventIds eventId)
{
    B_PlaybackIpHandle playbackIp = appCtx;
#if 0
    /* App can choose to either directly call the Ip API here or send an event to the app thread */
    /* which can actually make the Ip API call. It can use the eventId to determine which call to make */
    NEXUS_Error rc;
    rc = B_PlaybackIp_SessionSetup(playbackIp, &ipSsessionSetupSettings, &ipSessionSetupStatus);
#endif
    gEventId = eventId;
    BDBG_MSG (("%s: Got EventId %d from IP library, appCtx %p", __FUNCTION__, eventId, appCtx));
    if (eventId == B_PlaybackIpEvent_eServerEndofStreamReached) {
        gEof = 1;
    }
    if (eventId == B_PlaybackIpEvent_eSeekComplete) {
        B_PlaybackIpTrickModesSettings ipTrickModeSettings;
        B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings);
        if (B_PlaybackIp_Seek(playbackIp, &ipTrickModeSettings) == B_ERROR_SUCCESS) {
            BDBG_WRN(("%s: seek successful", __FUNCTION__));
        }
        else {
            BDBG_ERR(("%s: seek failed", __FUNCTION__));
        }
        BKNI_SetEvent(waitEvent);
    }
    else if (eventId == B_PlaybackIpEvent_eClientBeginOfStream) {
        gResumePlay = true;
    }
    else if (eventId == B_PlaybackIpEvent_eClientEndofSegmentsReached) {
        gEndOfSegments = true;
    }
    else {
        BKNI_SetEvent(waitEvent);
    }
}

int
runSeekTestAndVerify(B_PlaybackIpPsiInfo *psi, B_PlaybackIpHandle playbackIp, NEXUS_PlaybackPosition seekPosition)
{
    B_PlaybackIpStatus playbackIpStatus;

    int rc = -1;
    B_PlaybackIpTrickModesSettings ipTrickModeSettings;
    B_PlaybackIpStatus playbackIpStatusNew;
    BSTD_UNUSED(psi);

    if (B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatus)) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); goto out;}

    if ((playbackIpStatus.position + seekPosition) < psi->duration)
    {
        B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings);
        ipTrickModeSettings.seekPosition = seekPosition; /* in msec */
        if (B_PlaybackIp_Seek(playbackIp, &ipTrickModeSettings)) {BDBG_ERR(("ERROR: Failed to seek Ip playback\n")); goto out;}
        sleep(2);
        if (B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatusNew)) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); goto out;}
        if (abs((unsigned long)(ipTrickModeSettings.seekPosition - playbackIpStatusNew.position)) > (4*1000) ) {BDBG_WRN(("Seek to %lu sec position failed: current position %lu", (unsigned long)seekPosition, (unsigned long)playbackIpStatusNew.position/1000)); goto out;}
        BDBG_WRN(("*********** %s: seeked successfully to %lu sec *********** ", __FUNCTION__, (unsigned long)seekPosition));
    }
    else
    {
        BDBG_WRN(("*********** %s: Can't seek %lu sec , it is beyond end of stream *********** ", __FUNCTION__, (unsigned long)seekPosition));
    }

    rc = 0;
out:
    return rc;
}

int
runMultipleSeekTestAndVerify(B_PlaybackIpPsiInfo *psi, B_PlaybackIpHandle playbackIp)
{
    int i = 5;
    while (i--) {
        /* run tests after a stream is played for sometime */
        sleep(5);
        if(runSeekTestAndVerify(psi, playbackIp, 10*1000)) {BDBG_ERR(("ERROR: seek unit test failed"));return -1;}

        sleep(3);
        if(runSeekTestAndVerify(psi, playbackIp, 50*1000)) {BDBG_ERR(("ERROR: seek unit test failed"));return -1;}
        sleep(3);
        if(runSeekTestAndVerify(psi, playbackIp, 100*1000)) {BDBG_ERR(("ERROR: seek unit test failed"));return -1;}

        if (runSeekTestAndVerify(psi, playbackIp, 150*1000)) {BDBG_ERR(("ERROR: seek unit test failed"));return -1;}
        if (runSeekTestAndVerify(psi, playbackIp, 200*1000)) {BDBG_ERR(("ERROR: seek unit test failed"));return -1;}
        sleep(3);
        if (runSeekTestAndVerify(psi, playbackIp, 160*1000)) {BDBG_ERR(("ERROR: seek unit test failed"));return -1;}
        if (runSeekTestAndVerify(psi, playbackIp, 100*1000)) {BDBG_ERR(("ERROR: seek unit test failed"));return -1;}
        if (runSeekTestAndVerify(psi, playbackIp, 200*1000)) {BDBG_ERR(("ERROR: seek unit test failed"));return -1;}
    }
    return 0;
}

int executePause(B_PlaybackIpHandle playbackIp)
{
    int rc = -1;
    B_PlaybackIpTrickModesSettings ipTrickModeSettings;
    B_PlaybackIpStatus playbackIpStatus;
    B_PlaybackIpStatus playbackIpStatusNew;

    B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings);
    ipTrickModeSettings.pauseMethod = B_PlaybackIpPauseMethod_UseConnectionStalling;
    if (B_PlaybackIp_Pause(playbackIp, &ipTrickModeSettings)) {BDBG_ERR(("ERROR: Failed to pause Ip playback\n")); goto out;}
    if (B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatus)) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); goto out;}
    if (playbackIpStatus.ipState != B_PlaybackIpState_ePaused) {BDBG_WRN(("Pause failed: state is %d", playbackIpStatus.ipState)); goto out;}
    sleep(1);
    if (B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatusNew)) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); goto out;}
    if (playbackIpStatus.position != playbackIpStatusNew.position) {BDBG_WRN(("Pause failed: positions are %lu, %lu", (unsigned long)playbackIpStatus.position, (unsigned long)playbackIpStatusNew.position)); goto out;}
    BDBG_WRN(("*********** Successfully Paused Ip playback*********** \n"));

    rc = 0;
out:
    return rc;
}

int executePlay(B_PlaybackIpHandle playbackIp)
{
    int rc = -1;
    B_PlaybackIpStatus playbackIpStatus;
    B_PlaybackIpStatus playbackIpStatusNew;
    if (B_PlaybackIp_Play(playbackIp)) {BDBG_ERR(("ERROR: Failed to pause-resume Ip playback\n")); goto out;}
    if (B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatus)) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); goto out;}
    if (playbackIpStatus.ipState != B_PlaybackIpState_ePlaying) {BDBG_WRN(("Pause-Resume failed: state is %d", playbackIpStatus.ipState)); goto out;}
    sleep(2);
    if (B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatusNew)) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); goto out;}
    if (playbackIpStatus.position == playbackIpStatusNew.position || playbackIpStatusNew.position < playbackIpStatus.position) {BDBG_WRN(("Pause-Resume failed: positions are %lu, %lu", (unsigned long)playbackIpStatus.position, (unsigned long)playbackIpStatusNew.position)); goto out;}
    BDBG_WRN(("*********** Successfully Pause-Resumed Ip playback*********** \n"));

    rc = 0;
out:
    return rc;
}

int
runPlaybackIpUnitTests(B_PlaybackIpPsiInfo *psi, B_PlaybackIpHandle playbackIp)
{
    B_PlaybackIpTrickModesSettings ipTrickModeSettings;
    int rc = -1;
    B_PlaybackIpStatus playbackIpStatus;
    B_PlaybackIpStatus playbackIpStatusNew;

    /* ip_client is also used for auto testing media streaming over IP */

    /* run tests after a stream is played for sometime */
    sleep(10);

    /* Pause */


    if(executePause(playbackIp)) {BDBG_ERR(("ERROR: Pause test failed"));return -1;}
    sleep(5);

    /* Play */
    if(executePlay(playbackIp)) {BDBG_ERR(("ERROR: Play test failed"));return -1;}
    sleep(5);

    /* seek 110sec fwd */
    if(runSeekTestAndVerify(psi, playbackIp, 110*1000)) {BDBG_ERR(("ERROR: seek unit test failed"));return -1;}
    sleep(10);

    /* seek 60sec fwd */
    if(runSeekTestAndVerify(psi, playbackIp, 60*1000)) {BDBG_ERR(("ERROR: seek unit test failed"));return -1;}
    sleep(10);

    /* case: seek relative 30sec backward */
    if (B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatus)) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); goto out;}
    B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings);
    ipTrickModeSettings.seekPositionIsRelative = true;
    ipTrickModeSettings.seekBackward = true;
    ipTrickModeSettings.seekPosition = 30*1000; /* in msec */
    if (B_PlaybackIp_Seek(playbackIp, &ipTrickModeSettings)) {BDBG_ERR(("ERROR: Failed to seek Ip playback\n")); goto out;}
    sleep(2);
    if (B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatusNew)) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); goto out;}
    if (playbackIpStatus.position-31000 > playbackIpStatusNew.position) {BDBG_WRN(("Seek relative back by 30sec position failed: positions is %lu, %lu", (unsigned long)playbackIpStatus.position, (unsigned long)playbackIpStatusNew.position)); goto out;}
    BDBG_WRN(("*********** %s: successfully seeked backward to 30sec *********** ", __FUNCTION__));
    sleep(10);

    /* skip trickplay tests for audio files */
    if (psi->videoPid == 0) {
        rc = 0;
        goto out;
    }


    /* trickplay ff */
    B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings);
    if (psi->numPlaySpeedEntries > 1) {
        ipTrickModeSettings.method = B_PlaybackIpTrickModesMethod_UsePlaySpeed;
        ipTrickModeSettings.rate = psi->playSpeed[8];
        if (ipTrickModeSettings.rate >= psi->httpMinIFrameSpeed)
            ipTrickModeSettings.frameRepeat = psi->httpFrameRepeat;
        else
            ipTrickModeSettings.frameRepeat = 1;
        if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {BDBG_WRN(("Failed to set the trick play speed")); goto out;}
        BDBG_WRN(("****************** FF Started ***************************"));
        sleep(10);
        if (B_PlaybackIp_Play(playbackIp)) {BDBG_ERR(("ERROR: Failed to resume from trickplay \n")); goto out;}
        sleep(15);

        /* rewind test */
        ipTrickModeSettings.rate = psi->playSpeed[2];
        if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {BDBG_WRN(("Failed to set the trick play speed")); goto out;}
        BDBG_WRN(("****************** FR Started ***************************"));
        sleep(12);
        if (B_PlaybackIp_Play(playbackIp)) {BDBG_ERR(("ERROR: Failed to resume from trickplay \n")); goto out;}
    }
    else {
        ipTrickModeSettings.method = B_PlaybackIpTrickModesMethod_UseByteRange;
        ipTrickModeSettings.rate = 5;
        if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {BDBG_WRN(("Failed to do client side trickmodes ")); goto out;}
        BDBG_WRN(("****************** FF Started ***************************"));
        sleep(10);
        if (B_PlaybackIp_Play(playbackIp)) {BDBG_ERR(("ERROR: Failed to resume from trickplay \n")); goto out;}
        sleep(15);

        /* fast rewind */
        ipTrickModeSettings.method = B_PlaybackIpTrickModesMethod_UseByteRange;
        ipTrickModeSettings.rate = -5;
        if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {BDBG_WRN(("Failed to do client side trickmodes ")); goto out;}
        BDBG_WRN(("****************** FR Started ***************************"));
        sleep(12);
        if (B_PlaybackIp_Play(playbackIp)) {BDBG_ERR(("ERROR: Failed to resume from trickplay \n")); goto out;}
    }

#if 0 /* Plesae review this code with Sanjeev, Pause after ff fails.*/
    /* trickplay ff and pause play combination */
    B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings);
    if (psi->numPlaySpeedEntries > 1) {
        ipTrickModeSettings.method = B_PlaybackIpTrickModesMethod_UsePlaySpeed;
        ipTrickModeSettings.rate = psi->playSpeed[8];
        if (ipTrickModeSettings.rate >= psi->httpMinIFrameSpeed)
            ipTrickModeSettings.frameRepeat = psi->httpFrameRepeat;
        else
            ipTrickModeSettings.frameRepeat = 1;
        if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {BDBG_WRN(("Failed to set the trick play speed")); goto out;}
        BDBG_WRN(("****************** FF Started ***************************"));
        sleep(10);

        if(executePause(playbackIp)) {BDBG_ERR(("ERROR: Pause test failed"));return -1;}

        sleep(5);

        if(executePlay(playbackIp)) {BDBG_ERR(("ERROR: Play test failed"));return -1;}
        sleep(15);

        /* rewind test */
        ipTrickModeSettings.rate = psi->playSpeed[2];
        if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {BDBG_WRN(("Failed to set the trick play speed")); goto out;}
        BDBG_WRN(("****************** FR Started ***************************"));
        sleep(12);
        if (B_PlaybackIp_Play(playbackIp)) {BDBG_ERR(("ERROR: Failed to resume from trickplay \n")); goto out;}
    }
    else {
        ipTrickModeSettings.method = B_PlaybackIpTrickModesMethod_UseByteRange;
        ipTrickModeSettings.rate = 5;
        if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {BDBG_WRN(("Failed to do client side trickmodes ")); goto out;}
        BDBG_WRN(("****************** FF Started ***************************"));
        sleep(10);
        if(executePause(playbackIp)) {BDBG_ERR(("ERROR: Pause test failed"));return -1;}

        sleep(5);

        if(executePlay(playbackIp)) {BDBG_ERR(("ERROR: Play test failed"));return -1;}
        sleep(15);

        /* fast rewind */
        ipTrickModeSettings.method = B_PlaybackIpTrickModesMethod_UseByteRange;
        ipTrickModeSettings.rate = -5;
        if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {BDBG_WRN(("Failed to do client side trickmodes ")); goto out;}
        BDBG_WRN(("****************** FR Started ***************************"));
        sleep(12);
        if (B_PlaybackIp_Play(playbackIp)) {BDBG_ERR(("ERROR: Failed to resume from trickplay \n")); goto out;}
    }
#endif

    sleep(15);
    rc = 0;

out:
    BDBG_WRN(("****************** AUTO Test Completed ***************************"));
    return rc;
}

/* logic to determine whether IP library should use Nexus Playpump to feed network stream or would it get pulled via Nexus Playback module */
static bool
useNexusPlaypumpForIpPlayback(
    B_PlaybackIpPsiInfo *psi,
    B_PlaybackIpProtocol protocol
    )
{
    /* For RTP, UDP, RTSP based protocols, use Nexus Playpump as data is being pushed by the server and thus can't use the Nexus playback as it pulls data from IP library */
    if (protocol != B_PlaybackIpProtocol_eHttp && protocol != B_PlaybackIpProtocol_eHttps) {
        return true;
    }

    /* For HTTP/S, we can use both Nexus Playback or Playpump as it is a pull based protocol */

    /* For following cases & protocols, we will use Nexus Playpump */
    /* - Server side trickmodes are supported (either Playspeed or TimeSeekRange ) as Nexus playback can't maintain the stream position during trickplays */
    /* - HLS protocol: stream segments are downloaded from server first and then feed into playpump */
    /* - RVU protocol: is like Server supporting server side trickmodes, Nexus Playback can't track stream position during trickplays) */
    if (psi->numPlaySpeedEntries > 1 /* DLNA App should use DLNA PS flags as well */ ||
            psi->hlsSessionEnabled || /* HLS Protocol requires us to use Playpump. */
            psi->mpegDashSessionEnabled ||
            (!psi->contentLength && (psi->mpegType == NEXUS_TransportType_eTs || psi->mpegType == NEXUS_TransportType_eMpeg2Pes) ) || /* If content-length is not available but its MPEG2 TS or PES streams, then use Playpump. */
            psi->liveChannel)
    {
        return true;
    }

    /* By default use the Nexus Playback for all protocols */
    return false;
}

/* logic to determine which clock recorvery mechanism should be programmed for IP Playback Channels */
static bool
useLiveModeForIpPlayback(
    B_PlaybackIpPsiInfo *psi,
    B_PlaybackIpProtocol protocol,
    B_PlaybackIpClockRecoveryMode *clockRecoveryModeForPlaybackIp
    )
{
    bool useLiveIpMode;

    /* psi discovery should be complete at this point, else we can't truly determine the clock mode and we default to pull mode */
    if (!psi->psiValid) {
        *clockRecoveryModeForPlaybackIp = B_PlaybackIpClockRecoveryMode_ePull;
        useLiveIpMode = false;
    }

    if (psi->mpegType != NEXUS_TransportType_eTs) {
        /* for all non mpeg2 transport streams (ES, ASF, MP4, etc. formats), we are in the non-live mode */
        /* as there is *no* good mechanism for locking to the sender's clock (TTS or PCRs are missing) */
        /* this is true irrespective of the network protocol (HTTP vs. RTP/UDP) */
        *clockRecoveryModeForPlaybackIp = B_PlaybackIpClockRecoveryMode_ePull;
        useLiveIpMode = false;
    }
    /* MPEG2 TS content */
    else if (protocol != B_PlaybackIpProtocol_eHttp && protocol != B_PlaybackIpProtocol_eHttps) {
        /* UDP, RTP, RTSP protocols carrying MPEG2 TS content are always put in live mode */
        /* TODO: add call into IP library to determine if incoming stream is a TTS stream or not */
        /* for now, use SyncSlipMode */
        /* For DLNA Apps, media profile will indicate if it is a TTS or TS stream */
        if (psi->transportTimeStampEnabled) {
            /* live channel & stream has 4 byte Timestamps, use TTS based pacing mode */
            *clockRecoveryModeForPlaybackIp = B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip;
        } else
        {
            *clockRecoveryModeForPlaybackIp = B_PlaybackIpClockRecoveryMode_ePushWithPcrNoSyncSlip;
            *clockRecoveryModeForPlaybackIp = B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip;
        }
        useLiveIpMode = true;
    }
    /* MPEG2 TS content in HTTP/HTTPS protocols only */
#if 0
    /* RVU check */
    else if (appCtx->rvuFlag) {
        /* RVU protocol: channels are always in live mode whether live or files from disk are being streamed & have 4 byte Timestamps */
        *clockRecoveryModeForPlaybackIp = B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip;
    }
#endif
    /* Non RVU MPEG2 TS content in HTTP/HTTPS protocols only */
    else if (psi->liveChannel) {
        /* special check to auto detect if this channel is in live mode (true for Broadcom servers as they send this flag) */
        if (psi->transportTimeStampEnabled) {
            /* live channel & stream has 4 byte Timestamps, use TTS based pacing mode */
            *clockRecoveryModeForPlaybackIp = B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip;
            useLiveIpMode = true;
        }
        else {
#if 1
            /* live channel & stream doesn't has 4 byte Timestamps, so use PCRs as timestamps for pacing */
            /* however, this PCR based basing is not yet fully tested, so default to pull mode for now */
            *clockRecoveryModeForPlaybackIp = B_PlaybackIpClockRecoveryMode_ePushWithPcrNoSyncSlip;
            *clockRecoveryModeForPlaybackIp = B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip;
            useLiveIpMode = true;
#else
            *clockRecoveryModeForPlaybackIp = B_PlaybackIpClockRecoveryMode_ePull;
            useLiveIpMode = false;
#endif
        }
    }
    /* Non RVU MPEG2 TS content in HTTP/HTTPS protocols from non-Broadcom servers */
    else {
        /* App has to somehow to know whether this channel needs to be in Live or playback mode */
        /* this could be based on either pre-defined channel map or from a DLNA level flag */
        /* For DLNA apps, there is a VideoItem.VideoBroadcastItem flag indicates live mode */
        /* then if the media profile is TTS, we can be in TTS mode. Else, when PCR pacing */
        /* is supported, we can use PCR pacing mode. */
        /* and default would be pull mode even though it is a live stream */

        /* we default to pull mode */
        *clockRecoveryModeForPlaybackIp = B_PlaybackIpClockRecoveryMode_ePull;
        useLiveIpMode = false;
    }

    return useLiveIpMode;
}

static void sourceChangeCallback(void *context, int param)
{
    BSTD_UNUSED(context);
    BDBG_MSG (("Got Source (%s) change callback", param ==1 ? "video" : "audio"));
}

static void buffer_depth_violation(void *context, bool isMax)
{
    B_PlaybackIpSettings * pSettings = context;
    BDBG_ERR(("%s buffer depth violated: %u", isMax ? "Max" : "Min", isMax ? pSettings->ttsParams.throttleParams.maxBufDepth : pSettings->ttsParams.throttleParams.minBufDepth));
}

static int utilsGetPlaypumpBuffer(
    NEXUS_PlaypumpHandle playpump,
    size_t size,
    void **buffer
    )
{
    NEXUS_Error rc;
    size_t ppBufSize;

    for(;;) {
        if (NEXUS_Playpump_GetBuffer(playpump, buffer, &ppBufSize)) {
            BDBG_ERR(("Returned error from NEXUS_Playpump_GetBuffer()!"));
            goto error;
        }

        if (ppBufSize == 0) {
            NEXUS_PlaypumpStatus ppStatus;
            /* bplaypump_flush(playback_ip->nexusHandles.playpump); */
            rc = NEXUS_Playpump_GetStatus(playpump, &ppStatus);
            if (!rc) BDBG_MSG(("Returned 0 buffer size from GetBuffer()!, fifo dep %d, size %d, desc sz %d dep %d", ppStatus.fifoDepth, ppStatus.fifoSize, ppStatus.descFifoDepth, ppStatus.descFifoSize));
            BKNI_Sleep(200);
            continue;
        }
        else if (ppBufSize>= size) {
            /* constrain the amount of data we're going to read from the socket */
            BDBG_MSG(("%s: got buffer of size %d from the playpump\n", __FUNCTION__, ppBufSize));
            break;
        }
        BDBG_MSG(("skip buffer %d", ppBufSize));
        /* release buffer unused, it's too small */
        if(NEXUS_Playpump_ReadComplete(playpump, ppBufSize, 0)) {
            BDBG_ERR(("Returned error from NEXUS_Playpump_ReadComplete()!"));
            BKNI_Sleep(1);
            continue;
        }
    }
    return 0;
error:
    return -1;
}

NEXUS_StcChannelHandle stcChannel;
static void pcr_callback(void *context, int param)
{
    NEXUS_Timebase timebase = (NEXUS_Timebase)param;
    NEXUS_TimebaseStatus status;
    struct timeval tv;
    long miliseconds;
    uint32_t stc;
    unsigned int lastPcr = 0;
    long lastTimeMs = 0;

    BSTD_UNUSED(context);
    if (ipCfg.decoderStats) {
        NEXUS_Timebase_GetStatus(timebase, &status);
        NEXUS_StcChannel_GetStc(stcChannel, &stc);
        gettimeofday(&tv, NULL);
        miliseconds = tv.tv_usec/1000;
        BDBG_WRN(("pcr=%x stc %x lastError=%d pcrCount=%d pcrErrors=%d pcrValid=%d pcrdiff=%d timediff=%ld \n",
                    status.lastValue, stc, status.lastError,status.pcrCount,status.pcrErrors,status.pcrValid,(((status.lastValue-lastPcr)*1000)/45000),miliseconds-lastTimeMs));
        lastPcr = status.lastValue;
        lastTimeMs = miliseconds;
    }
}

/* Making these declarations global as otherwise Coverity keeps flagging this example app as large stack overuse. */
static NEXUS_PlatformSettings platformSettings;
B_PlaybackIpPsiInfo psiList[MAX_PROGRAMS_PER_FREQUENCY];
NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
NEXUS_VideoDecoderStartSettings videoProgram;
NEXUS_VideoDecoderSettings  videoDecoderSettings;
NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
NEXUS_AudioDecoderSettings audioDecoderSettings;
NEXUS_AudioOutputSettings audioOutputSettings;
NEXUS_AudioOutput output;
B_PlaybackIpOpenSettings playbackIpOpenSettings;
B_PlaybackIpSessionStartSettings ipSessionStartSettings;
B_PlaybackIpSessionStartStatus ipSessionStartStatus;
B_PlaybackIpSessionOpenSettings ipSessionOpenSettings;
B_PlaybackIpSessionOpenStatus ipSessionOpenStatus;
B_PlaybackIpSessionSetupSettings ipSessionSetupSettings;
B_PlaybackIpSessionSetupStatus ipSessionSetupStatus;
NEXUS_PlatformConfiguration platformConfig;
NEXUS_TimebaseSettings timebaseLockedSettings;
NEXUS_TimebaseSettings timebaseFreerunSettings;
NEXUS_StcChannelSettings stcChannelSettings;
NEXUS_PlaybackPidChannelSettings playbackPidChannelSettings;
B_PlaybackIpTrickModesSettings ipTrickModeSettings;
B_PlaybackIpStatus playbackIpStatus;
B_PlaybackIpPsiInfo psi;
NEXUS_PlaybackHandle playback = NULL;
NEXUS_PlaybackSettings playbackSettings;
B_PlaybackIpSettings playbackIpSettings;
NEXUS_PlaybackStartSettings playbackStartSettings;

int main(int argc, char *argv[])
{
    int i,errorFlag=0;
    bool userQuit = false, replayStream=false;
    unsigned videoDecoderIndex=0;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    NEXUS_PidChannelHandle videoPidChannel = NULL, audioPidChannel = NULL, pcrPidChannel = NULL, extraVideoPidChannel = NULL;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump = NULL;
    NEXUS_PlaypumpHandle playpump2 = NULL;
    NEXUS_PlaypumpOpenPidChannelSettings pidChannelSettings;
    NEXUS_DisplayHandle display = NULL;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window = NULL;
    NEXUS_VideoDecoderHandle videoDecoder = NULL;
    B_PlaybackIpHandle playbackIp = NULL;
    NEXUS_VideoDecoderPrimerHandle primer[TOTAL_PRIMERS];
    NEXUS_Timebase timebaseLocked;
    NEXUS_Timebase timebaseFreerun;
    bool timebaseInitialized = false;
    int audioPid;
    NEXUS_AudioCodec audioCodec;

    char buf[16];
    int numbytes;
    psiCollectionDataType  collectionData;
    int numPrograms;
    NEXUS_VideoDecoderStatus videoStatus;
    NEXUS_AudioDecoderStatus audioStatus;
    NEXUS_PlaybackStatus pbStatus;
    NEXUS_PlaypumpStatus ppStatus;
    NEXUS_PlaypumpStatus pp2Status;
    uint32_t stc;
#ifdef B_HAS_DTCP_IP
    void * AkeHandle = NULL;
    void * dtcpCtx = NULL;
#endif
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
    NEXUS_DmaHandle dmaHandle = NULL;
#endif
    const bmedia_probe_track *track;
    const bmedia_probe_stream *stream;

#ifdef B_HAS_SSL
    void *ssl_ctx;
    B_PlaybackIpSslInitSettings sslInitSettings;
#endif
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_SyncChannelHandle syncChannel = NULL;
#endif
    B_PlaybackIpError rrc = 0;
    NEXUS_VideoWindowScalerSettings scalerSettings;

    B_Time prevTime,curTime;
    unsigned currentAlternateAudioNumber;

#ifdef LIVEMEDIA_SUPPORT
    BDBG_ERR(("LiveMedia version: %s", LiveMediaGetVersion() ));
#endif /* LIVEMEDIA_SUPPORT */

    rrc = B_PlaybackIp_GetDefaultSettings(&playbackIpSettings);
    if (rrc !=B_ERROR_SUCCESS ) {
        BDBG_ERR((" Failed to initialize Playback IP Settings"));
        exit(1);
    }

    /* coverity[tainted_data] */
    if (ip_client_setup(argc, argv)) {
        exit(1);
    }

    if (!ipCfg.slaveMode) {
        /* Bring up all modules for a platform in a default configuration for this platform */
        NEXUS_Platform_GetDefaultSettings(&platformSettings);
        platformSettings.openFrontend = false;
        rc = NEXUS_Platform_Init(&platformSettings);
        }
    else {
        rc = NEXUS_Platform_Join();
    }
    if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
    NEXUS_Platform_GetConfiguration(&platformConfig);

    if (BKNI_CreateEvent(&waitEvent)) { BDBG_ERR(("Failed to create an event at %d", __LINE__)); exit(1); }
        /* ----------------------------------------------------------------------------------------------------------------*/
#ifdef B_HAS_SSL
        memset(&sslInitSettings, 0, sizeof(B_PlaybackIpSslInitSettings));
        sslInitSettings.rootCaCertPath=CA_CERT;
        sslInitSettings.clientAuth=false;
        sslInitSettings.ourCertPath=NULL;
        sslInitSettings.privKeyPath=NULL;
        sslInitSettings.password=NULL;
        ssl_ctx = B_PlaybackIp_SslInit(&sslInitSettings);
        if (!ssl_ctx) {
            BDBG_ERR(("SSL Security initialization failed\n"));
            exit(1);
        }
#endif

#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
        if ((dmaHandle = NEXUS_Dma_Open(0, NULL)) == NULL) {
            BDBG_ERR(("ERROR: NEXUS_Dma_Open failed"));
            return (-1);
        }
#endif
#ifdef B_HAS_DTCP_IP
        if (ipCfg.security == B_PlaybackIpSecurityProtocol_DtcpIp) {
            struct timeval t_start, t_finish;

            BDBG_MSG(("Initializing HW security params\n"));
            /* TODO: for VMS, we may need to pass in the nexus DMA handle here if it is already opened */
            if(DtcpInitHWSecurityParams(dmaHandle) != BERR_SUCCESS)
            {
                BDBG_ERR(("ERROR: Failed to init DtcpHW Security parmas\n"));
                return (-1);
            }

            /* initialize DtcpIp library */
            /* if App hasn't already opened the Nexus M2M DMA handle, then pass-in initial arg as NULL and let DTCP/IP library open the handle */
            if ((dtcpCtx = DtcpAppLib_Startup(B_DeviceMode_eSink, false, ipCfg.dtcpKeyFormat, ipCfg.ckc_check)) == NULL) {
                BDBG_ERR(("ERROR: DtcpAppLib_Startup failed\n"));
                return (-1);
            }
            if(dtcpCtx == NULL)
            {
                BDBG_ERR(("%s: Failed to Initialize Dtcp/Ip Library: DTCP/IP encryption/decryption won't work\n", __FUNCTION__));
                exit(rc);
            }

            /* Perform AKE for DTCP/IP */
            gettimeofday(&t_start, 0);
            BDBG_MSG(("host is %s, strlen %d\n", ipCfg.host, strlen(ipCfg.host)));
            if((rc = DtcpAppLib_DoAke(dtcpCtx, ipCfg.host, ipCfg.dtcpAkePort, &AkeHandle)) != BERR_SUCCESS) {
                BDBG_ERR(("DTCP AKE Failed!!!\n"));
                exit(rc);
            }
            gettimeofday(&t_finish, 0);
            BDBG_MSG (("-------------AKE took %d secs and %d msecs\n",
                (t_finish.tv_usec < t_start.tv_usec? t_finish.tv_sec - t_start.tv_sec - 1: t_finish.tv_sec - t_start.tv_sec),
                (t_finish.tv_usec < t_start.tv_usec? t_finish.tv_usec - t_start.tv_usec + 1000000: t_finish.tv_usec - t_start.tv_usec) ));
        }
#endif

#ifdef B_HAS_RAD_EA
        if (ipCfg.security == B_PlaybackIpSecurityProtocol_RadEa){
            B_PlaybackIp_RadEaInit(0);
        }
#endif

        /* ----------------------------------------------------------------------------------------------------------------*/
        /* Open Nexus Playpump, For Live Modes, IP Applib directly feeds the network data to the Nexus Playpump */
        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
        BDBG_MSG (("## fifo size %d, desc %d", playpumpOpenSettings.fifoSize, playpumpOpenSettings.numDescriptors));
        playpumpOpenSettings.fifoSize *= 2;
        if (playpumpOpenSettings.fifoSize < MPEG_DASH_MAX_SEGMENT_SIZE ) {
            playpumpOpenSettings.fifoSize = MPEG_DASH_MAX_SEGMENT_SIZE;
        }

        playpumpOpenSettings.numDescriptors = 200;
        playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
        if (!playpump) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
        /* IP Applib currently uses the FIFO mode to feed data, so app needs to set that mode */
        playpumpSettings.mode = NEXUS_PlaypumpMode_eFifo;
        rc = NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__));errorFlag=1; goto error;}
        /* ----------------------------------------------------------------------------------------------------------------*/

        /* ----------------------------------------------------------------------------------------------------------------*/
        /* Setup Nexus Playback, For Playback Modes (e.g. HTTP), IP Applib uses Nexus Playback to feed data */
        playback = NEXUS_Playback_Create();
        if (!playback) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        /* ----------------------------------------------------------------------------------------------------------------*/

        /* ----------------------------------------------------------------------------------------------------------------*/
        /* Open IP Applib Handle */
        BDBG_MSG(("Initializing IP Applib...\n"));
        playbackIp = B_PlaybackIp_Open(&playbackIpOpenSettings);
        if (!playbackIp) {BDBG_WRN(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}

    newSession:
        gEof = 0;
        gEventId = B_PlaybackIpEvent_eMax;
        /* ----------------------------------------------------------------------------------------------------------------*/
        /* Setup the security & socket setting structure used in the IP Session Open */
        B_PlaybackIp_GetDefaultSessionOpenSettings(&ipSessionOpenSettings);
        memset(&ipSessionOpenStatus, 0, sizeof(ipSessionOpenStatus));
        /* Security */
        if (ipCfg.security > B_PlaybackIpSecurityProtocol_None) {
            switch (ipCfg.security ) {
#ifdef B_HAS_SSL
            case B_PlaybackIpSecurityProtocol_Ssl:
                ipSessionOpenSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_Ssl;
                ipSessionOpenSettings.security.initialSecurityContext = ssl_ctx;
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
                ipSessionOpenSettings.security.dmaHandle = dmaHandle;
#endif
                break;
#endif
#ifdef B_HAS_DTCP_IP
            /* DTCP-IP */
            case B_PlaybackIpSecurityProtocol_DtcpIp:
                ipSessionOpenSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_DtcpIp;
                ipSessionOpenSettings.security.initialSecurityContext = AkeHandle;
                break;
#endif
            case B_PlaybackIpSecurityProtocol_Aes128:
                BDBG_MSG (("Setting the dma handle for HLS encryption"));
                ipSessionOpenSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_Aes128;
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
                ipSessionOpenSettings.security.initialSecurityContext = dmaHandle;
#endif
                break;
            /* TODO RAD-EA */
            default:
                BDBG_ERR(("TODO: Security options\n"));
                break;
            }
        }
        else {
            /* just pass the dma handle to IP library in case it needs it */
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
           ipSessionOpenSettings.security.dmaHandle = dmaHandle;
#endif
#ifdef B_HAS_SSL
           ipSessionOpenSettings.security.initialSecurityContext = ssl_ctx;
#endif
        }

        /* Set IP Address, Port, and Protocol used to receive the AV stream */
        strncpy(ipSessionOpenSettings.socketOpenSettings.ipAddr, ipCfg.host, sizeof(ipSessionOpenSettings.socketOpenSettings.ipAddr)-1);
        ipSessionOpenSettings.socketOpenSettings.port = ipCfg.port;
        ipSessionOpenSettings.socketOpenSettings.protocol = ipCfg.protocol;
        memset(&psi, 0, sizeof(psi));
        switch (ipCfg.protocol) {
        case B_PlaybackIpProtocol_eUdp:
        case B_PlaybackIpProtocol_eRtpNoRtcp:
        case B_PlaybackIpProtocol_eRtp:
            BDBG_MSG(("Udp, Rtp, RtpNoRtcp"));
            ipSessionOpenSettings.ipMode = B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip;
            ipSessionOpenSettings.maxNetworkJitter = IP_NETWORK_JITTER;
            ipSessionOpenSettings.networkTimeout = 1;  /* timeout in 1 sec during network outage events */
            ipSessionOpenSettings.eventCallback = playbackIpEventCallback;
            ipSessionOpenSettings.appCtx = playbackIp; /* this should be app handle */
            rc = B_PlaybackIp_SessionOpen(playbackIp, &ipSessionOpenSettings, &ipSessionOpenStatus);
            if (rc) {
                BDBG_ERR(("Session Open call failed: rc %d", rc));
                errorFlag=1;
                goto errorClose;
            }
            BDBG_MSG (("Session Open call succeeded"));
            if (ipCfg.usePlaybackIpForPsiProbing) {
    /*>>>>> This is where the bmedia probe is */
                B_PlaybackIp_GetDefaultSessionSetupSettings(&ipSessionSetupSettings);
                ipSessionSetupSettings.u.udp.psiParsingTimeLimit = strtol(_getEnvVariableValue("setupTimeout", "3000"), (char **)NULL, 10);
                rc = B_PlaybackIp_SessionSetup(playbackIp, &ipSessionSetupSettings, &ipSessionSetupStatus);
                if (rc == B_ERROR_IN_PROGRESS) {
                    BERR_Code err;
                    /* app can do some useful work while SessionSetup is in progress and resume when callback sends the completion event */
                    BDBG_MSG (("Session Setup call in progress, sleeping for now..."));
                    err = BKNI_WaitForEvent(waitEvent, 90000);
                    if (err == BERR_TIMEOUT || err != 0) {
                        BDBG_WRN(("Session Setup call timed out in or got error (err %d)!!!", err));
                        errorFlag=1;
                        goto errorClose;
                    }
                    /* So we got the event, call SessionSetup again to retrieve the session setup status (psi & file handle) */
                    rc = B_PlaybackIp_SessionSetup(playbackIp, &ipSessionSetupSettings, &ipSessionSetupStatus);
                }
                if (rc != B_ERROR_SUCCESS) {
                    BDBG_ERR(("Session Setup call failed: rc %d\n", rc));
                    errorFlag=1;
                    goto error;
                }
                psi = ipSessionSetupStatus.u.udp.psi;

                BDBG_MSG (("UDP Session Setup call succeeded"));
            }
            break;
        case B_PlaybackIpProtocol_eRtsp:
        #if 0
            BDBG_MSG(("case RTSP: additionalHeaders %x", &ipSessionOpenSettings.u.rtsp.additionalHeaders ));
            BDBG_MSG(("case RTSP: additionalHeaders %s", &ipSessionOpenSettings.u.rtsp.additionalHeaders ));
            {
                unsigned char * p = &ipSessionOpenSettings.u.rtsp.additionalHeaders ;
                unsigned int i;
                for (i=0; i<8; i++) {
                    BDBG_ERR(("%s: pos %x: value (0x%02x)", __FUNCTION__, &p[i], p[i] ));
                }
            }
        #endif
            ipSessionOpenSettings.ipMode = B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip;
            ipSessionOpenSettings.maxNetworkJitter = 300;
            ipSessionOpenSettings.networkTimeout = 1;  /* timeout in 1 sec during network outage events */
            ipSessionOpenSettings.socketOpenSettings.protocol = B_PlaybackIpProtocol_eRtsp;
            ipSessionOpenSettings.socketOpenSettings.url = ipCfg.url;
            /* App can set this flag to enable non-blocking mode of IP library APIs */
            /* ipSessionOpenSettings.nonBlockingMode = true;*/
            ipSessionOpenSettings.nonBlockingMode = true;
            ipSessionOpenSettings.eventCallback = playbackIpEventCallback;
            ipSessionOpenSettings.appCtx = playbackIp; /* this should be app handle */
            /* apps can request IP library to return the RTSP Response Message incase it needs to extract any custom headers */
            ipSessionOpenSettings.u.rtsp.copyResponseHeaders = true;
            /* B_PlaybackIp_SessionOpen() set some u.http variables; clear them out for RTSP */
            BKNI_Memset( &ipSessionOpenSettings.u.rtsp, 0, sizeof(ipSessionOpenSettings.u.rtsp) );
        #if 0
            {
                unsigned char * p = &ipSessionOpenSettings.u.rtsp.additionalHeaders ;
                unsigned int i;
                for (i=0; i<8; i++) {
                    BDBG_ERR(("%s: pos %x: value (0x%02x)", __FUNCTION__, &p[i], p[i] ));
                }
            }
            BDBG_MSG(("Rtsp - B_PlaybackIp_SessionOpen 1"));
        #endif
            rc = B_PlaybackIp_SessionOpen(playbackIp, &ipSessionOpenSettings, &ipSessionOpenStatus);
            if (rc == B_ERROR_IN_PROGRESS) {
                /* app can do some useful work while SessionOpen is in progress and resume when callback sends the completion event */
                BDBG_MSG (("Session Open call in progress, sleeping for now..."));
                while (gEventId != B_PlaybackIpEvent_eSessionOpenDone)
                    /* Note: instead of while loop, app can just wait on an event which is signalled by the callback function */
                    BKNI_Sleep(100);
                /* So we got the event, call SessionOpen again to retrieve the session open status */
                BDBG_MSG(("Rtsp - B_PlaybackIp_SessionOpen 2"));
                rc = B_PlaybackIp_SessionOpen(playbackIp, &ipSessionOpenSettings, &ipSessionOpenStatus);
            }
            if (rc != B_ERROR_SUCCESS) {
                BDBG_ERR(("Socket Open call failed: rc %d, HTTP Status %d\n", rc, ipSessionOpenStatus.u.rtsp.statusCode));
                errorFlag=1;
                goto errorClose;
            }
            BDBG_MSG (("Session Open call succeeded, RTSP status code %d", ipSessionOpenStatus.u.rtsp.statusCode));
            B_PlaybackIp_GetDefaultSessionSetupSettings(&ipSessionSetupSettings);
            ipSessionSetupSettings.u.rtsp.userAgent = "Broadcom's LiveMedia based test RTSP Client";
            /* apps can request IP library to return the RTSP Response Message incase it needs to extract any custom headers */
            ipSessionSetupSettings.u.rtsp.copyResponseHeaders = true;
            ipSessionSetupSettings.u.rtsp.psiParsingTimeLimit = strtol(_getEnvVariableValue("setupTimeout", "3000"), (char **)NULL, 10);
            /* Note: can optionally set the additionalHeaders field */
            rc = B_PlaybackIp_SessionSetup(playbackIp, &ipSessionSetupSettings, &ipSessionSetupStatus);
            BKNI_ResetEvent(waitEvent);
            while (rc == B_ERROR_IN_PROGRESS) {
                BERR_Code err;
                /* app can do some useful work while SessionSetup is in progress and resume when callback sends the completion event */
                BDBG_MSG (("Session Setup call in progress, sleeping for now..."));
                err = BKNI_WaitForEvent(waitEvent, EVENT_WAIT_TIME_IN_MSEC);
                if (err == BERR_TIMEOUT || err != 0) {
                    BDBG_WRN(("Session Setup call timed out or got error (err %d)!!!", err));
                    errorFlag=1;
                    goto error;
                }
                /* So we got the event, call SessionSetup again to retrieve the session setup status (psi & file handle) */
                rc = B_PlaybackIp_SessionSetup(playbackIp, &ipSessionSetupSettings, &ipSessionSetupStatus);
            }
            if (rc != B_ERROR_SUCCESS) {
                BDBG_ERR(("Session Setup call failed: rc %d\n", rc));
                errorFlag=1;
                goto error;
            }
            for (i=0; i<ipSessionSetupStatus.u.rtsp.scaleListEntries; i++) {
                BDBG_MSG(("scale list[%d] = %0.1f", i,ipSessionSetupStatus.u.rtsp.scaleList[i]));
            }
            psi = ipSessionSetupStatus.u.rtsp.psi;
            stream = (bmedia_probe_stream *)(ipSessionSetupStatus.u.rtsp.stream);
            break;
        case B_PlaybackIpProtocol_eHttp:
            ipSessionOpenSettings.socketOpenSettings.protocol = B_PlaybackIpProtocol_eHttp;
            ipSessionOpenSettings.ipMode = B_PlaybackIpClockRecoveryMode_ePull;
            ipSessionOpenSettings.networkTimeout = 10;  /* timeout in 10 sec during network outage events */
#if 0
            /* set this flag is App already knows the PSI info of the stream and it wants to use Nexus Playpump for feeding IP session data */
            ipSessionOpenSettings.useNexusPlaypump = 1;
#endif
            ipSessionOpenSettings.u.http.networkBufferSize = (30*3*1024*1024)/8; /* data cache size: 30sec worth for a max bitrate of 3Mbps */
            if (ipSessionOpenSettings.u.http.networkBufferSize < MPEG_DASH_MAX_SEGMENT_SIZE) {
                ipSessionOpenSettings.u.http.networkBufferSize = MPEG_DASH_MAX_SEGMENT_SIZE;
            }
            ipSessionOpenSettings.socketOpenSettings.url = ipCfg.url;
            ipSessionOpenSettings.socketOpenSettings.useProxy = strtol(_getEnvVariableValue("useProxy", "0"), (char **)NULL, 10);
#if 0
            /* If app needs to set any addition HTTP header fields that need to be sent in the outgoing HTTP Get request */
            /* they should be set in additionalHttpHeaderFields strings. E.g. for DLNA requests, app can specify the desired transfer mode */
            ipSessionOpenSettings.u.http.additionalHeaders = _getEnvVariableValue("additionalHeader", "Cookie: VISITOR_INFO1_LIVE=mRUZ7HDi4Yc;\r\n");
#endif
            ipSessionOpenSettings.u.http.additionalHeaders = _getEnvVariableValue("additionalHeader", "transferMode.dlna.org: Streaming\r\n");   /* Streaming mode for AV content */
            /* apps can override the default user agent string in the outgoing HTTP Get Request */

            ipSessionOpenSettings.u.http.userAgent = _getEnvVariableValue("userAgent", "BRCM IP Applib Test App/2.0");   /* Streaming mode for AV content */
            /* apps can request IP library to return the HTTP Response Message incase it needs to extract any custom headers */
            ipSessionOpenSettings.u.http.copyResponseHeaders = true;
            /* setup a callback for receiving various events from IP library */
            ipSessionOpenSettings.eventCallback = playbackIpEventCallback;
            ipSessionOpenSettings.appCtx = playbackIp; /* this should be app handle */
            /* App can set this flag to enable non-blocking mode of IP library APIs */
            /* ipSessionOpenSettings.nonBlockingMode = true;*/
            ipSessionOpenSettings.nonBlockingMode = true;
            rc = B_PlaybackIp_SessionOpen(playbackIp, &ipSessionOpenSettings, &ipSessionOpenStatus);
            if (rc == B_ERROR_IN_PROGRESS) {
                /* app can do some useful work while SessionOpen is in progress and resume when callback sends the completion event */
                BDBG_MSG (("Session Open call in progress, sleeping for now..."));
                while (gEventId != B_PlaybackIpEvent_eSessionOpenDone)
                    /* Note: instead of while loop, app can just wait on an event which is signalled by the callback function */
                    BKNI_Sleep(100);
                /* So we got the event, call SessionOpen again to retrieve the session open status */
                rc = B_PlaybackIp_SessionOpen(playbackIp, &ipSessionOpenSettings, &ipSessionOpenStatus);
            }
            if (rc != B_ERROR_SUCCESS) {
                BDBG_ERR(("Session Open call failed: rc %d, HTTP Status %d\n", rc, ipSessionOpenStatus.u.http.statusCode));
                errorFlag=1;
                goto errorClose;
            }
            BDBG_MSG (("Session Open call succeeded, HTTP status code %d", ipSessionOpenStatus.u.http.statusCode));
            if (ipSessionOpenStatus.u.http.responseHeaders) {
                BDBG_MSG(("Response Header is... \n%s", ipSessionOpenStatus.u.http.responseHeaders));
                /* Note: App can extract any customer Response Headers here: useful to extract DLNA flags from getContentFeatures Header */

                /* now free the responseHeader */
                free(ipSessionOpenStatus.u.http.responseHeaders);
            }
            B_PlaybackIp_GetDefaultSessionSetupSettings(&ipSessionSetupSettings);
            /* If app knows the media container type, then it can provide the hint to IP Applib */
            /*ipSessionSetupSettings.u.http.contentTypeHint = NEXUS_TransportType_eEs; */
            if (ipCfg.playMp3) {
                BDBG_MSG (("Playing Mp3, so tell IP Applib to skip auto PSI probing\n"));
                ipSessionSetupSettings.u.http.skipPsiParsing = false;
                ipSessionSetupSettings.u.http.avgBitRate = 192000;
                ipSessionSetupSettings.u.http.contentLengthHint = 2182835;
            }
            else if (ipCfg.playLpcm) {
                BDBG_MSG (("Playing LPCM, so tell IP Applib to skip auto PSI probing & Convert LPCM to WAV by inserting WAV header\n"));
                ipSessionSetupSettings.u.http.skipPsiParsing = true;
                ipSessionSetupSettings.u.http.avgBitRate = 192000; /* please set it if known */
                ipSessionSetupSettings.u.http.convertLpcmToWave = true;
                ipSessionSetupSettings.u.http.bitsPerSample = 16;
                ipSessionSetupSettings.u.http.sampleRate = 48000;
                ipSessionSetupSettings.u.http.numChannels = 2;
            }
#if 0
            /* If app figures out from getContentFeatures that the server doesn't support HTTP Range Headers, then it can set a flag to tell IP Applib to not include one in outgoing Get Req */
            ipSessionSetupSettings.u.http.disableRangeHeader = true;
#endif

            if (ipCfg.liveChannel) {
                /* set the readTimeout value if app apriori knows its going to play a live channel */
                ipSessionSetupSettings.u.http.readTimeout = 100; /* 100msec */
                ipSessionSetupSettings.u.http.liveChannel = true; /* we are playing a live channel */
                /* set a limit on how long the psi parsing should continue before returning */
                ipSessionSetupSettings.u.http.psiParsingTimeLimit = 100; /* 100msec: we limit the psi parsing time so as to lower the channel change time */
                /* live channels dont require local trickmode capability, so skip index related work */
                ipSessionSetupSettings.u.http.dontUseIndex = true;
            }
            else {
                /* non-live settings */
                if (ipCfg.hevc == 2) {
                    ipSessionSetupSettings.u.http.enablePayloadScanning = false; /* turn on the deep packet inspection */
                }
                else{
                    ipSessionSetupSettings.u.http.enablePayloadScanning = true;
                }
                /* set a limit on how long the psi parsing should continue before returning */
                ipSessionSetupSettings.u.http.psiParsingTimeLimit = strtol(_getEnvVariableValue("setupTimeout", "30000"), (char **)NULL, 10);
                /* if app needs to play multiple formats (such as a DLNA DMP/DMR) (e.g. TS, VOB/PES), then set this option to do deep payload inspection */
            }
            if (ipCfg.gStreamerMode)
                /* gstreamer wants to read demuxed data from playback ip and itself feed it to the h/w */
                ipSessionSetupSettings.u.http.dontFeedDataToPlaybackHw = true;

            BKNI_ResetEvent(waitEvent);
            rc = B_PlaybackIp_SessionSetup(playbackIp, &ipSessionSetupSettings, &ipSessionSetupStatus);
            if (rc == B_ERROR_IN_PROGRESS) {
                BERR_Code err;
                /* app can do some useful work while SessionSetup is in progress and resume when callback sends the completion event */
                BDBG_MSG (("Session Setup call in progress, sleeping for now..."));
                err = BKNI_WaitForEvent(waitEvent, 90000);
                if (err == BERR_TIMEOUT || err != 0) {
                    BDBG_WRN(("Session Setup call timed out in or got error (err %d)!!!", err));
                    errorFlag=1;
                    goto errorClose;
                }
                /* So we got the event, call SessionSetup again to retrieve the session setup status (psi & file handle) */
                rc = B_PlaybackIp_SessionSetup(playbackIp, &ipSessionSetupSettings, &ipSessionSetupStatus);
            }
            if (rc != B_ERROR_SUCCESS) {
                BDBG_ERR(("Session Setup call failed: rc %d\n", rc));
                errorFlag=1;
                goto error;
            }
            psi = ipSessionSetupStatus.u.http.psi;
            if (ipCfg.gStreamerMode && ipSessionSetupStatus.u.http.file == NULL) {
                BDBG_ERR(("Failed to get file handle from playback_ip, needed as we are in gstreamer mode"));
                errorFlag=1;
                goto error;
            }

            BDBG_MSG (("psi.psiValid: %u ", psi.psiValid));
            BDBG_MSG (("psi.contentLength: %lld ",      psi.contentLength));
            BDBG_MSG (("psi.audioCodec: %u ",           psi.audioCodec));
            BDBG_MSG (("psi.audioPid: %u ",             psi.audioPid));
            BDBG_MSG (("psi.avgBitRate: %u ",           psi.avgBitRate));
            BDBG_MSG (("psi.extraVideoPid: %u ",        psi.extraVideoPid));
            BDBG_MSG (("psi.hlsSessionEnabled: %u ",    psi.hlsSessionEnabled));
            BDBG_MSG (("psi.liveChannel: %u ",          psi.liveChannel));
            BDBG_MSG (("psi.mpegDashSessionEnabled: %u ", psi.mpegDashSessionEnabled));
            BDBG_MSG (("psi.mpegType: %u ",             psi.mpegType));
            BDBG_MSG (("psi.pcrPid: %u ",               psi.pcrPid));
            BDBG_MSG (("psi.transportTimeStampEnabled: %u ", psi.transportTimeStampEnabled));
            BDBG_MSG (("psi.videoFrameRate: %f ",       psi.videoFrameRate));
            BDBG_MSG (("psi.videoHeight: %u ",          psi.videoHeight));
            BDBG_MSG (("psi.videoPid: %u ",             psi.videoPid));
            BDBG_MSG (("psi.videoWidth: %u ",           psi.videoWidth));
            BDBG_MSG (("psi.playSpeed: %s ",            psi.playSpeedString));
            BDBG_MSG (("psi.usePlaypump2ForAudio: %u ", psi.usePlaypump2ForAudio));
            BDBG_WRN (("psi.extraAudioPidsCount: %u ",  psi.extraAudioPidsCount));

            BDBG_MSG (("Session Setup call succeeded, file handle %p", (void *)ipSessionSetupStatus.u.http.file));
            if (ipSessionSetupSettings.u.http.skipPsiParsing)
                goto skipTrackInfo;
            stream = (bmedia_probe_stream *)(ipSessionSetupStatus.u.http.stream);
            if (!stream)
                goto skipTrackInfo;
            for (track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
                if (track->type==bmedia_track_type_video) {
                    BDBG_MSG(("video track %u codec:%u\n", track->number, track->info.video.codec));
                }
                if (track->type==bmedia_track_type_pcr) {
                    BDBG_MSG(("pcr pid %u\n", track->number));
                }
                else if(track->type==bmedia_track_type_audio) {
                    BDBG_MSG(("audio track %u codec:%u\n", track->number, track->info.audio.codec));
                }
            }
    skipTrackInfo:
            break;
        default:
            BDBG_WRN(("Protocol %d not supported", ipCfg.protocol));
            errorFlag=1;
            goto error;
        }

        /* ----------------------------------------------------------------------------------------------------------------*/
        BDBG_MSG(("IP SessionOpen Complete"));

        if (ipCfg.playMp3) {
            psi.psiValid = true;
            psi.audioCodec = NEXUS_AudioCodec_eMp3;
            psi.audioPid = 0x1;
            psi.mpegType = NEXUS_TransportType_eEs;
        }
        else if (ipCfg.playLpcm) {
            psi.psiValid = true;
            psi.audioCodec = NEXUS_AudioCodec_ePcmWav;
            psi.audioPid = 0x1;
            psi.mpegType = NEXUS_TransportType_eWav;
        }
        /* ----------------------------------------------------------------------------------------------------------------*/
        else if (ipCfg.skipPsiParsing) {
            /* hardcode the codec info */
            /* current one is for Video Conferencing Demo case where camera output is being streamed to us via RTP */
            memset(&psi, 0, sizeof(psi));
            psi.psiValid = true;
            psi.videoCodec = NEXUS_VideoCodec_eMpeg4Part2;
            psi.videoCodec = NEXUS_VideoCodec_eH264;
            psi.videoPid = 0x1;
            psi.audioPid = 0;
            psi.pcrPid = 0x1;
            psi.mpegType = NEXUS_TransportType_eEs;
        }
        else if (psi.psiValid == false) {
            /* For non-HTTP protocols, apps will need to use tspsi2 library to do the psi discovery */
            memset(&collectionData, 0, sizeof(psiCollectionDataType));
            collectionData.playpump = playpump;
            collectionData.live = true;
            collectionData.playbackIp = playbackIp;
            /*collectionData.srcType = ipStreamerCfg->srcType;*/
            BDBG_MSG(("Acquiring PSI info..."));
            acquirePsiInfo(&collectionData,  psiList, &numPrograms);
            /* just choose the 1st program */
            psi = psiList[0];
        }
        /* ----------------------------------------------------------------------------------------------------------------*/

#if NEXUS_HAS_SYNC_CHANNEL
    if (!ipCfg.fastChannelChange) {
        /* create a sync channel */
        NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
        syncChannelSettings.enablePrecisionLipsync = true;
        syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
        if (syncChannel == NULL) {BDBG_ERR(("NEXUS Error at %d, Exiting...\n", __LINE__)); exit(1);}
        BDBG_MSG (("Using Nexus STC channel for lipsync"));
    }
#endif

    /* Now that we know the PSI info, pick the appropriate audio rendition if user provided the rendition number. */
    audioCodec = (ipCfg.alternateAudioNumber>0 && ipCfg.alternateAudioNumber <= psi.hlsAltAudioRenditionCount)?
        psi.hlsAltAudioRenditionInfo[ipCfg.alternateAudioNumber-1].codec:
        psi.audioCodec;
    audioPid = ipCfg.alternateAudioNumber>0 && ipCfg.alternateAudioNumber <= psi.hlsAltAudioRenditionCount?
        psi.hlsAltAudioRenditionInfo[ipCfg.alternateAudioNumber-1].pid:
        psi.audioPid;
    BDBG_MSG(("alternateAudioNumber %d psi.hlsAltAudioRenditionCount %d audioPid %d audioCodec %d", ipCfg.alternateAudioNumber, psi.hlsAltAudioRenditionCount, audioPid, audioCodec));

    currentAlternateAudioNumber = ipCfg.alternateAudioNumber > 0 ? ipCfg.alternateAudioNumber : 0;

    if (replayStream) goto skipNexusOpens;
    if (ipCfg.alternateAudioNumber > 0 || (psi.usePlaypump2ForAudio && audioPid)) {
        /* Open a second Playpump (probably for MPEG-DASH). */
        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
        BDBG_MSG (("## fifo size %d, desc %d", playpumpOpenSettings.fifoSize, playpumpOpenSettings.numDescriptors));
        playpumpOpenSettings.fifoSize *= 2;
        playpumpOpenSettings.numDescriptors = 200;
        playpump2 = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
        if (!playpump2) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        NEXUS_Playpump_GetSettings(playpump2, &playpumpSettings);
        /* IP Applib currently uses the FIFO mode to feed data, so app needs to set that mode */
        playpumpSettings.mode = NEXUS_PlaypumpMode_eFifo;
        rc = NEXUS_Playpump_SetSettings(playpump2, &playpumpSettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__));errorFlag=1; goto error;}
    }

    /* ----------------------------------------------------------------------------------------------------------------*/
    timebaseLocked = NEXUS_Timebase_Open(NEXUS_ANY_ID);
    timebaseFreerun = NEXUS_Timebase_Open(NEXUS_ANY_ID);
    if (!timebaseLocked || !timebaseFreerun) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
    timebaseInitialized = true;

    /* Open the StcChannel */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcChannelSettings);
    stcChannelSettings.timebase = timebaseLocked;
    stcChannelSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcChannelSettings);
    if (!stcChannel) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
    /* ----------------------------------------------------------------------------------------------------------------*/

    /* ----------------------------------------------------------------------------------------------------------------*/
    /* Open Video decoder */
    NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
    /* Increase the CDB size as it is used as the dejitter buffer */
    videoDecoderOpenSettings.fifoSize = 10 * 1024 * 1024;
    /*  If MotionJpeg or Vp6 possibly use a different videoDecoder Index*/
    if(videoDecoderIndex==0)
    {
        /* coverity[stack_use_local_overflow] */
        /* coverity[stack_use_overflow] */
        NEXUS_VideoDecoderCapabilities cap;
        NEXUS_GetVideoDecoderCapabilities(&cap);
        if (psi.videoCodec == NEXUS_VideoCodec_eMotionJpeg && cap.sidVideoDecoder.useForMotionJpeg) {
            videoDecoderIndex = cap.sidVideoDecoder.baseIndex;
        }
        else if (psi.videoCodec == NEXUS_VideoCodec_eVp6 && cap.dspVideoDecoder.useForVp6) {
            videoDecoderIndex = cap.dspVideoDecoder.baseIndex;
        }
    }
    videoDecoder = NEXUS_VideoDecoder_Open(videoDecoderIndex, &videoDecoderOpenSettings);
    if (!videoDecoder) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
    /* ----------------------------------------------------------------------------------------------------------------*/

    /* ----------------------------------------------------------------------------------------------------------------*/
    /* Open & setup audio decoders and connect outputs */
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
    BDBG_MSG(("fifo size %d, type %d\n", audioDecoderOpenSettings.fifoSize, audioDecoderOpenSettings.type));
    audioDecoderOpenSettings.fifoSize = 2*512 * 1024;
    audioDecoderOpenSettings.type = NEXUS_AudioDecoderType_eDecode;
    /*TODO: pcmDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);*/
    /* */pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    if (!pcmDecoder) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}

    /*compressedDecoder = NEXUS_AudioDecoder_Open(1, &audioDecoderOpenSettings);*/
    compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);
    if (!compressedDecoder) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}

    /* ----------------------------------------------------------------------------------------------------------------*/


    /* ----------------------------------------------------------------------------------------------------------------*/
    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    /* Note: change to display type back for panel output */
    displaySettings.format = NEXUS_VideoFormat_e720p;
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    displaySettings.format = NEXUS_VideoFormat_e720p;
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    if (ipCfg.hevc == 1) {
        displaySettings.format = NEXUS_VideoFormat_e1080p24hz;
    }
    else if (ipCfg.hevc == 2) {
        displaySettings.format = NEXUS_VideoFormat_e3840x2160p24hz;
    }
    else if (ipCfg.hevc == 3) {
        displaySettings.format = NEXUS_VideoFormat_e1080p60hz;
    }
    else if (ipCfg.hevc == 4) {
        displaySettings.format = NEXUS_VideoFormat_e3840x2160p60hz;
    }
    BDBG_MSG (("Display format %d", displaySettings.format));
    display = NEXUS_Display_Open(0, &displaySettings);
    if (!display) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
    /* ----------------------------------------------------------------------------------------------------------------*/
    /* ----------------------------------------------------------------------------------------------------------------*/
    window = NEXUS_VideoWindow_Open(display, 0);
    if (!window) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
#if 0
    if (ipCfg.skipPsiParsing) {
        NEXUS_VideoWindowSettings windowSettings;
        /* is app is asking to skip psi parsing, we are lowering the window size */
        /* this is mainly done for the video conferencing demo case as we know the PSI info in advance and want to use smaller display window */
        NEXUS_VideoWindow_GetSettings(window, &windowSettings);
        windowSettings.position.x = 50 + (70 * 2);
        windowSettings.position.y = 50 + (30 * 2);
        windowSettings.position.width = 640/2;
        windowSettings.position.height = 480/2;
        NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    }
#endif
#if NEXUS_NUM_COMPONENT_OUTPUTS
    if (ipCfg.hevc != 2 && ipCfg.hevc != 4) {
        /* can't enable component for 4k display */
        rc = NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
    }
#endif
#if 0
    /* Note: enable for composite output */
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    if ( platformConfig.outputs.composite[0] ) {
        rc = NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
    }
#endif
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    /* Install hotplug callback -- video only for now */
    rc = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
#endif
    BDBG_MSG(("Display Manager is Opened\n"));
skipNexusOpens:
    /* ----------------------------------------------------------------------------------------------------------------*/
    /* If IP Applib is not able to find the PSI info, then set them to defaults */
    if (psi.psiValid == false) {BDBG_ERR(("Failed to find the PSI info from the stream at %d", __LINE__)); errorFlag=1; goto error;}

    BDBG_WRN(("Video Pid %d, Video Codec %d, Audio Pid %d, Audio Codec %d, Language %s, DefaultAudioInMain %s, Pcr Pid %d, Transport Type %d",
                psi.videoPid, psi.videoCodec, audioPid, audioCodec, psi.mainAudioLanguage, psi.defaultAudioIsMuxedWithVideo?"Y":"N", psi.pcrPid, psi.mpegType));
    {
        unsigned i;
        for (i=0; i < psi.hlsAltAudioRenditionCount; i++)
        {
            BDBG_WRN(("Seconday Audio: Container %d, Pid %d, Audio Codec %d, Language %s:%s, DefaultAudio %s, RequiresPlaypump2 %s",
                        psi.hlsAltAudioRenditionInfo[i].containerType,
                        psi.hlsAltAudioRenditionInfo[i].pid,
                        psi.hlsAltAudioRenditionInfo[i].codec,
                        psi.hlsAltAudioRenditionInfo[i].language,
                        psi.hlsAltAudioRenditionInfo[i].languageName,
                        psi.hlsAltAudioRenditionInfo[i].defaultAudio?"Y":"N",
                        psi.hlsAltAudioRenditionInfo[i].requiresPlaypump2?"Y":"N" ));
        }
    }

    if (psi.hlsSessionEnabled || psi.mpegDashSessionEnabled)
    {
        {
            NEXUS_VideoWindowSettings windowSettings;

            NEXUS_VideoWindow_GetSettings(window, &windowSettings);
            /* (1) Won't re-alloc buffers to meet source size changges and also put MADR/MCVP HW into bypass when not needed. */
            windowSettings.minimumSourceFormat = NEXUS_VideoFormat_e1080p;

            /* (2) Use fullscreen size buffer instead of PIP/PIG size to determine allocation. */
            windowSettings.allocateFullScreen = true;

            /* (3) Not auto snapping to integer scaling factor. */
            windowSettings.scaleFactorRounding.enabled = true;
            windowSettings.scaleFactorRounding.verticalTolerance = 0;
            windowSettings.scaleFactorRounding.horizontalTolerance = 0;

            rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
            if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        }

        {
            /* (4) Fixed scaler -> capture vs capture -> scaler orienation to avoid reconfiguration of VNET (Video Network). */
            /* This require RTS and Usage analysis to make sure it supportted. */
            NEXUS_VideoWindow_GetScalerSettings(window, &scalerSettings);
            scalerSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerBeforeCapture;
            scalerSettings.bandwidthEquationParams.delta = 1*1000*1000;
            rc = NEXUS_VideoWindow_SetScalerSettings(window, &scalerSettings);
            if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        }

        {
            NEXUS_VideoWindowMadSettings madSettings;
            /* (5) For chips (7400/7405/7420/7408) that have older deinterlacer that does not have HW bypass capability setting this flag will */
            /* prevent deinteralcer forced off when in 480i->480i, * 576i->576i, and 1080i->1080i non-scaled mode.  See also jira  SW7420-2423.*/
            NEXUS_VideoWindow_GetMadSettings(window, &madSettings);
            madSettings.pqEnhancement = NEXUS_MadPqEnhancement_eOff;
            rc = NEXUS_VideoWindow_SetMadSettings(window, &madSettings);
            if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        }
        BDBG_ERR (("HLS or MPEG-DASH Session: Updated settings for smooth transitions during stream parameter changes"));
    }

    /* ----------------------------------------------------------------------------------------------------------------*/
    if (ipCfg.fastChannelChange) {
        for (i=0;i<TOTAL_PRIMERS;i++) {
            /* coverity[stack_use_local_overflow] */
            /* coverity[stack_use_overflow] */
            primer[i] = NEXUS_VideoDecoderPrimer_Open(videoDecoder);
        }
    }

    B_PlaybackIp_GetSettings(playbackIp, &playbackIpSettings);
    ipCfg.useLiveIpMode = useLiveModeForIpPlayback(&psi, ipCfg.protocol, &playbackIpSettings.ipMode);
    playbackIpSettings.useNexusPlaypump = useNexusPlaypumpForIpPlayback(&psi, ipCfg.protocol);
    if (ipCfg.loop)
        playbackIpSettings.enableEndOfStreamLooping = true;

    if (playbackIpSettings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip)
    {
        playbackIpSettings.ttsParams.autoDetect = false;
        playbackIpSettings.ttsParams.pacingMaxError = 2636;
        playbackIpSettings.ttsParams.throttleParams.initBufDepth = 625000;
        playbackIpSettings.ttsParams.throttleParams.maxBufDepth = 2250000;
        playbackIpSettings.ttsParams.throttleParams.minBufDepth = 125000;
        playbackIpSettings.ttsParams.throttleParams.maxClockMismatch = 60;
        playbackIpSettings.ttsParams.throttleParams.bufDepthViolationCbContext = &playbackIpSettings;
        playbackIpSettings.ttsParams.throttleParams.bufDepthViolationCallback = &buffer_depth_violation;
    }
    else if (playbackIpSettings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithPcrNoSyncSlip)
    {
        playbackIpSettings.ttsParams.autoDetect = false;
        /* set to 200 ms per RDB */
        playbackIpSettings.ttsParams.pacingMaxError = 18000;
        /* add 200 ms for PCR pacing */
        playbackIpSettings.ttsParams.throttleParams.initBufDepth += 202500;
        playbackIpSettings.ttsParams.throttleParams.maxBufDepth = 8250000;
        /* debug */
        playbackIpSettings.ttsParams.pacingMaxError = 27000;
        playbackIpSettings.ttsParams.throttleParams.initBufDepth = 1500000;
        playbackIpSettings.ttsParams.throttleParams.minBufDepth = 500000;
        playbackIpSettings.ttsParams.throttleParams.maxClockMismatch = 100;
        playbackIpSettings.ttsParams.throttleParams.bufDepthViolationCbContext = &playbackIpSettings;
        playbackIpSettings.ttsParams.throttleParams.bufDepthViolationCallback = &buffer_depth_violation;
    }
    rc = B_PlaybackIp_SetSettings(playbackIp, &playbackIpSettings);

    if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); return (-1);}
    BDBG_WRN(("IP Clock Recovery Mode set to %s Mode (%d), using Nexus %s to feed stream over IP",
                playbackIpSettings.ipMode == B_PlaybackIpClockRecoveryMode_ePull? "non-Live":"Live", playbackIpSettings.ipMode,
                playbackIpSettings.useNexusPlaypump? "Playpump" : "Playback"));

    /* do the initial configuration of Nexus Playpump/Playback modules: set modes & open pid channels */
    if (playbackIpSettings.useNexusPlaypump) {
        NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
        playpumpSettings.transportType = psi.mpegType;
        if (playbackIpSettings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip) {
            playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_e32_Binary;
            playpumpSettings.timestamp.timebase = timebaseFreerun;
            playpumpSettings.timestamp.pacingMaxError = playbackIpSettings.ttsParams.pacingMaxError;
            playpumpSettings.timestamp.pacing = true;
            playpumpSettings.timestamp.pacingOffsetAdjustDisable = true;
            playpumpSettings.timestamp.resetPacing = true;
        }
        else if (playbackIpSettings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithPcrNoSyncSlip) {
            playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_eNone;
            playpumpSettings.timestamp.resetPacing = true;
            playpumpSettings.timestamp.timebase = timebaseFreerun;
            playpumpSettings.timestamp.pacingMaxError = playbackIpSettings.ttsParams.pacingMaxError;
            playpumpSettings.timestamp.pacing = true;
            playpumpSettings.timestamp.pcrPacingPid = psi.pcrPid;
            playpumpSettings.timestamp.pacingOffsetAdjustDisable = true;
        } else if (psi.transportTimeStampEnabled) {
            playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_eMod300;
            playpumpSettings.timestamp.pacing = false;
            BDBG_MSG (("Setting timestamp flag"));
        }
        rc = NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); errorFlag=1; goto error;}

        /* If we're using two playpumps, configure the second one, also. */
        if (playpump2)
        {
            NEXUS_Playpump_GetSettings(playpump2, &playpumpSettings);
            if (ipCfg.alternateAudioNumber>0 && ipCfg.alternateAudioNumber <= psi.hlsAltAudioRenditionCount)
                playpumpSettings.transportType = psi.hlsAltAudioRenditionInfo[ipCfg.alternateAudioNumber-1].containerType;
            else
                playpumpSettings.transportType = psi.mpegType;
            if (playbackIpSettings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip) {
                playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_e32_Binary;
                playpumpSettings.timestamp.timebase = timebaseFreerun;
                playpumpSettings.timestamp.pacingMaxError = playbackIpSettings.ttsParams.pacingMaxError;
                playpumpSettings.timestamp.pacing = true;
                playpumpSettings.timestamp.pacingOffsetAdjustDisable = true;
                playpumpSettings.timestamp.resetPacing = true;
            } else if (psi.transportTimeStampEnabled) {
                playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_eMod300;
                playpumpSettings.timestamp.pacing = false;
                BDBG_MSG (("Setting timestamp flag"));
            }
            rc = NEXUS_Playpump_SetSettings(playpump2, &playpumpSettings);
            if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); errorFlag=1; goto error;}
        }

        /* Open the video pid channel */
        if (psi.videoCodec != NEXUS_VideoCodec_eNone && psi.videoPid) {
            NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
            pidChannelSettings.pidType = NEXUS_PidType_eVideo;
            videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, psi.videoPid, &pidChannelSettings);
            if (!videoPidChannel) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
            BDBG_MSG (("%s: Opened video pid channel %p for video pid %u ", __FUNCTION__, (void *)videoPidChannel, psi.videoPid));
        }

        /* Open the extra video pid channel if present in the stream */
        if (psi.extraVideoCodec != NEXUS_VideoCodec_eNone && psi.extraVideoPid != 0) {
            NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
            pidChannelSettings.pidType = NEXUS_PidType_eVideo;
            extraVideoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, psi.extraVideoPid, &pidChannelSettings);
            if (!extraVideoPidChannel) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
            BDBG_MSG (("%s: extra video pid %d, codec %d is present, pid channel created", __FUNCTION__, psi.extraVideoPid, psi.extraVideoCodec));
        }

        if ((audioCodec != NEXUS_AudioCodec_eUnknown && audioPid)) {
            /* Open the audio pid channel */
            NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
            pidChannelSettings.pidType = NEXUS_PidType_eAudio;
            pidChannelSettings.pidTypeSettings.audio.codec = audioCodec;
            audioPidChannel = NEXUS_Playpump_OpenPidChannel(playpump2?playpump2:playpump, audioPid, &pidChannelSettings);
            if (!audioPidChannel) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
            BDBG_MSG (("%s: Opened audio pid channel %p for audio pid %u ", __FUNCTION__, (void *)audioPidChannel, audioPid));
        }

        if (psi.pcrPid && psi.pcrPid != audioPid && psi.pcrPid != psi.videoPid) {
            /* Open the pcr pid channel */
            NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
            pidChannelSettings.pidType = NEXUS_PidType_eUnknown;
            pcrPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, psi.pcrPid, &pidChannelSettings);
            if (!pcrPidChannel) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
            BDBG_MSG (("%s: Opened pcr pid channel %p for pcr pid %u ", __FUNCTION__, (void *)pcrPidChannel, psi.pcrPid));
            }
        else
        {
            if (psi.pcrPid == audioPid)
            {
                pcrPidChannel = audioPidChannel;
                BDBG_MSG (("%s: Setting pcrPidChannel to audioPidChannel %p", __FUNCTION__, (void *)pcrPidChannel));
            }
            else
            {
                pcrPidChannel = videoPidChannel;
                BDBG_MSG (("%s: Setting pcrPidChannel to videoPidChannel %p", __FUNCTION__, (void *)pcrPidChannel));
            }
        }
    }
    else {
        /* using Nexus Playback module */
        NEXUS_Playback_GetSettings(playback, &playbackSettings);
        playbackSettings.playpump = playpump;
        if (psi.videoCodec != NEXUS_VideoCodec_eNone && psi.videoPid)
            playbackSettings.stcChannel = stcChannel;
        if (ipCfg.initialSeekTime || ipCfg.preChargeTime)
            playbackSettings.startPaused = true;
        if (!ipCfg.loop) {
            playbackSettings.endOfStreamCallback.callback = endOfStreamCallback;
            playbackSettings.endOfStreamCallback.context = NULL;
            playbackSettings.errorCallback.callback = errorCallback;
            playbackSettings.errorCallback.context = NULL;
            playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
        }
        else {
            playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
        }
        if (ipCfg.playMp3 || ipCfg.playLpcm || audioCodec == NEXUS_AudioCodec_eMp3) {
            BDBG_MSG (("Enabling streamProcessing flag for audio seeking capability"));
            playbackSettings.enableStreamProcessing = true; /* needed for seek capability */
        }
        playbackSettings.playpumpSettings.transportType = psi.mpegType;
        playbackSettings.playpumpSettings.mode = NEXUS_PlaypumpMode_eFifo;
        if (psi.transportTimeStampEnabled) {
            playbackSettings.playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_eMod300;
            playbackSettings.playpumpSettings.timestamp.pacing = false;
            BDBG_MSG (("Setting timestamp flag"));
        }
        rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        /* TODO: type setting following:
            playbackStartSettings.mode = NEXUS_PlaybackMode_eAutoBitrate;
        */

        if (psi.videoCodec != NEXUS_VideoCodec_eNone && psi.videoPid) {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidChannelSettings);
            playbackPidChannelSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            playbackPidChannelSettings.pidTypeSettings.video.codec = psi.videoCodec;
            playbackPidChannelSettings.pidTypeSettings.video.decoder = videoDecoder;        /* Decode will set this later */
            playbackPidChannelSettings.pidTypeSettings.video.index = true;
            videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, psi.videoPid, &playbackPidChannelSettings);
            if (!videoPidChannel) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        }

        /* Open the extra video pid channel if present in the stream */
        if (psi.extraVideoCodec != NEXUS_VideoCodec_eNone && psi.extraVideoPid != 0) {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidChannelSettings);
            playbackPidChannelSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            playbackPidChannelSettings.pidTypeSettings.video.codec = psi.extraVideoCodec;
            playbackPidChannelSettings.pidTypeSettings.video.decoder = videoDecoder;        /* Decode will set this later */
            playbackPidChannelSettings.pidTypeSettings.video.index = true;
            extraVideoPidChannel = NEXUS_Playback_OpenPidChannel(playback, psi.extraVideoPid, &playbackPidChannelSettings);
            if (!extraVideoPidChannel) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
            BDBG_MSG (("%s: extra video pid %d, codec %d is present, pid channel created", __FUNCTION__, psi.extraVideoPid, psi.extraVideoCodec));
        }

        if (audioCodec != NEXUS_AudioCodec_eUnknown && audioPid) {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidChannelSettings);
            playbackPidChannelSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            playbackPidChannelSettings.pidSettings.pidTypeSettings.audio.codec = audioCodec;
            playbackPidChannelSettings.pidTypeSettings.audio.primary = pcmDecoder;
            audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, audioPid, &playbackPidChannelSettings);
            if (!audioPidChannel) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
            BDBG_MSG(("Playback Audio Pid channel is opened\n"));
        }
        if (psi.pcrPid && psi.pcrPid != audioPid && psi.pcrPid != psi.videoPid) {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidChannelSettings);
            playbackPidChannelSettings.pidSettings.pidType = NEXUS_PidType_eOther;
            pcrPidChannel = NEXUS_Playback_OpenPidChannel(playback, psi.pcrPid, &playbackPidChannelSettings);
            if (!pcrPidChannel) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        }
        else
        {
            if (psi.pcrPid == audioPid)
                pcrPidChannel = audioPidChannel;
            else
                pcrPidChannel = videoPidChannel;
            BDBG_MSG (("%s: Using either video or audio pid channel for pcr pid channel ", __FUNCTION__));
        }
    }
    /* ----------------------------------------------------------------------------------------------------------------*/

    /* ----------------------------------------------------------------------------------------------------------------*/
    /* Stc Channel configuration */
    NEXUS_StcChannel_GetSettings(stcChannel, &stcChannelSettings);
    NEXUS_Timebase_GetSettings(timebaseLocked, &timebaseLockedSettings);
    NEXUS_Timebase_GetSettings(timebaseFreerun, &timebaseFreerunSettings);
    switch (playbackIpSettings.ipMode) {
    case B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip:
        /* Following Nexus configuration is done for absorbing high IP network jitter
         -DPCR & PCR Offset blocks are programmed w/ increased thresholds for errors
         -Both display & audio outputs are decoupled from input time base & run from a free clock
         -AV CDBs sizes are increased to absorb the network jitter
         -AV decodes are delayed by the amount of jitter buffer depth
        */
        if (psi.pcrPid) {
            /* program the lockedTimebase: increase its track range & max pcr errors */
            timebaseLockedSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
            timebaseLockedSettings.freeze = false;
            timebaseLockedSettings.sourceSettings.pcr.pidChannel = pcrPidChannel;
            timebaseLockedSettings.sourceSettings.pcr.maxPcrError = IP_NETWORK_JITTER * 183/2;    /* in milliseconds: based on 90Khz clock */
            timebaseLockedSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e244ppm;
            /* disable the jitter correction that XPT can perform */
            timebaseLockedSettings.sourceSettings.pcr.jitterCorrection = NEXUS_TristateEnable_eDisable;
            /* this per PCR callback is only for debug purposes */
            timebaseLockedSettings.pcrCallback.callback = pcr_callback;
            timebaseLockedSettings.pcrCallback.context = NULL;
            BDBG_MSG (("Configured Timebase %u with jitter %d", (unsigned)timebaseLocked, IP_NETWORK_JITTER));
            BDBG_WRN(("timebase %u: type %d, fr %d, PcrErr %d, trackRange %d, jitterCorr %d",
                        (unsigned)timebaseLocked,
                        timebaseLockedSettings.sourceType,
                        timebaseLockedSettings.freeze,
                        timebaseLockedSettings.sourceSettings.pcr.maxPcrError,
                        timebaseLockedSettings.sourceSettings.pcr.trackRange,
                        timebaseLockedSettings.sourceSettings.pcr.jitterCorrection));

            /* Update STC Channel Settings to accomodate Network Jitter */
            /* configure the StcChannel to do lipsync with the PCR */
            stcChannelSettings.timebase = timebaseLocked;     /* timebase */
            stcChannelSettings.mode = NEXUS_StcChannelMode_ePcr; /* Live Mode */
            /* offset threshold: uses upper 32 bits (183ticks/msec) of PCR clock */
            stcChannelSettings.modeSettings.pcr.offsetThreshold = IP_NETWORK_JITTER * 183;
            /* max pcr error: uses upper 32 bits (183ticks/msec) of PCR clock */
            stcChannelSettings.modeSettings.pcr.maxPcrError =  IP_NETWORK_JITTER * 183;
            stcChannelSettings.modeSettings.pcr.pidChannel = pcrPidChannel;
            /*  PCR Offset "Jitter Adjustment" is not suitable for use with IP channels Channels, so disable it */
            stcChannelSettings.modeSettings.pcr.disableJitterAdjustment = true;
            /* Disable Auto Timestamp correction for PCR Jitter */
            stcChannelSettings.modeSettings.pcr.disableTimestampCorrection = true;
            /* We just configured the Timebase, so turn off auto timebase config */
            stcChannelSettings.autoConfigTimebase = false;
            BDBG_MSG(("Configured stc channel: timebase %u, jitter %d", (unsigned)timebaseLocked, IP_NETWORK_JITTER));
            BDBG_WRN(("stc: timebase %u, mode %d, offsetThr %d, pcrErr %d disJitAdj %d disTs %d, autoConfig %d",
                        (unsigned)stcChannelSettings.timebase,
                        stcChannelSettings.mode,
                        stcChannelSettings.modeSettings.pcr.offsetThreshold,
                        stcChannelSettings.modeSettings.pcr.maxPcrError,
                        stcChannelSettings.modeSettings.pcr.disableJitterAdjustment,
                        stcChannelSettings.modeSettings.pcr.disableTimestampCorrection,
                        stcChannelSettings.autoConfigTimebase));
        }

        /* Setup 2nd timebase to freerun, this is used for display & audioOutputs */
        timebaseFreerunSettings.freeze = true;
        timebaseFreerunSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
        timebaseFreerunSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
        BDBG_MSG(("Configured Timebase %u to freerun \n", (unsigned)timebaseFreerun));
        BDBG_WRN(("timebase %u: type %d, frz %d, traackRange %d",
                    (unsigned)timebaseFreerun,
                    timebaseFreerunSettings.sourceType,
                    timebaseFreerunSettings.freeze,
                    timebaseFreerunSettings.sourceSettings.pcr.trackRange
                 ));
                    break;

    case B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip:
    case B_PlaybackIpClockRecoveryMode_ePushWithPcrNoSyncSlip:
        if (psi.pcrPid) {
            /* program the lockedTimebase: increase its track range & max pcr errors */
            timebaseLockedSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
            timebaseLockedSettings.freeze = false;
            timebaseLockedSettings.sourceSettings.pcr.pidChannel = pcrPidChannel;
            timebaseLockedSettings.sourceSettings.pcr.maxPcrError = 255;
            timebaseLockedSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
            BDBG_MSG(("Configured Timebase %u with increased tracking range & maxPcrError \n", (unsigned)timebaseLocked));

        }
        else {
            /* program the timebase 0: increase its track range & max pcr errors */
            timebaseLockedSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
            timebaseLockedSettings.freeze = true;
            timebaseLockedSettings.sourceSettings.freeRun.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
            timebaseLockedSettings.sourceSettings.freeRun.centerFrequency = 0x400000;
            BDBG_MSG(("Configured Timebase %u with increased tracking range & maxPcrError \n", (unsigned)timebaseLocked));
        }
        /* program the timebase 1: for monitoring the playpump buffer */
        timebaseFreerunSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
        timebaseFreerunSettings.freeze = true;
        timebaseFreerunSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e122ppm;
        BDBG_MSG(("Configured Timebase %u with increased tracking range & maxPcrError \n", (unsigned)timebaseFreerun));

        /* Update STC Channel Settings */
        /* configure the StcChannel to do lipsync with the PCR */
        stcChannelSettings.timebase = timebaseLocked;     /* timebase */
        stcChannelSettings.mode = NEXUS_StcChannelMode_ePcr; /* Live Mode */
        /* offset threshold: uses upper 32 bits (183ticks/msec) of PCR clock */
        stcChannelSettings.modeSettings.pcr.offsetThreshold = 8;
        /* max pcr error: uses upper 32 bits (183ticks/msec) of PCR clock */
        stcChannelSettings.modeSettings.pcr.maxPcrError = 255;
        stcChannelSettings.modeSettings.pcr.pidChannel = pcrPidChannel;
        /*  PCR Offset "Jitter Adjustment" is not suitable for use with IP channels Channels, so disable it */
        stcChannelSettings.modeSettings.pcr.disableJitterAdjustment = true;
        /* Disable Auto Timestamp correction for PCR Jitter */
        stcChannelSettings.modeSettings.pcr.disableTimestampCorrection = true;
        /* We just configured the Timebase, so turn off auto timebase config */
        stcChannelSettings.autoConfigTimebase = false;
        break;

    case B_PlaybackIpClockRecoveryMode_ePull:
        /* PVR/Pull mode */
        if (psi.videoCodec != NEXUS_VideoCodec_eNone && psi.videoPid) {
            stcChannelSettings.timebase = timebaseLocked;     /* timebase */
            stcChannelSettings.mode = NEXUS_StcChannelMode_eAuto; /* PVR (Pull) Mode */
            stcChannelSettings.modeSettings.Auto.transportType = psi.mpegType;

            if (psi.mpegType == NEXUS_TransportType_eMp4Fragment) {
                stcChannelSettings.modeSettings.Auto.transportType = NEXUS_TransportType_eMpeg2Pes;
            }
        }
        break;
    }
#if 0
    /* check if helpful */
    stcChannelSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eVideoMaster;
    stcChannelSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eAudioMaster;
#endif
    rc = NEXUS_StcChannel_SetSettings(stcChannel, &stcChannelSettings);
    if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}

    rc = NEXUS_Timebase_SetSettings(timebaseLocked, &timebaseLockedSettings);
    if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}

    rc = NEXUS_Timebase_SetSettings(timebaseFreerun, &timebaseFreerunSettings);
    if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
    /* ----------------------------------------------------------------------------------------------------------------*/


    /* ----------------------------------------------------------------------------------------------------------------*/
    /* Setup up video decoder */
    BDBG_MSG(("Video PID %d, Video Codec %d, Audio PID %d, Audio Codec %d, PCR PID %d, transport type %d, duration %d colorDepth %d\n",
            psi.videoPid, psi.videoCodec, audioPid, audioCodec, psi.pcrPid, psi.mpegType, psi.duration, psi.colorDepth));
    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    if (psi.videoCodec != NEXUS_VideoCodec_eNone && psi.videoPid) {
        videoProgram.codec = psi.videoCodec;
        videoProgram.pidChannel = videoPidChannel;
        videoProgram.stcChannel = stcChannel;
        if (ipCfg.hevc == 1 || ipCfg.hevc == 2)
            videoProgram.frameRate = NEXUS_VideoFrameRate_e24;
        if (extraVideoPidChannel) {
            videoProgram.enhancementPidChannel = extraVideoPidChannel;
            videoProgram.codec = psi.extraVideoCodec;
            BDBG_MSG (("%s: extra video pid channel:%p programmed", __FUNCTION__, (void *)extraVideoPidChannel));
        }
    }

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = audioCodec;
    audioProgram.pidChannel = audioPidChannel;
    if (audioCodec != NEXUS_AudioCodec_eUnknown && audioPid) {
        audioProgram.stcChannel = stcChannel;
    }

    /* configure output rate managers */
    switch (playbackIpSettings.ipMode) {
    case B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip:
        /* Both display & audio outputs are decoupled from input time base & run from a free clock */
        #if NEXUS_NUM_AUDIO_DACS
        output = NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]);
        NEXUS_AudioOutput_GetSettings(output, &audioOutputSettings);
        audioOutputSettings.timebase = timebaseFreerun;
        rc = NEXUS_AudioOutput_SetSettings(output, &audioOutputSettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        #endif
        #if NEXUS_NUM_SPDIF_OUTPUTS
        output = NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]);
        NEXUS_AudioOutput_GetSettings(output, &audioOutputSettings);
        audioOutputSettings.timebase = timebaseFreerun;
        rc = NEXUS_AudioOutput_SetSettings(output, &audioOutputSettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        #endif
#if NEXUS_NUM_HDMI_OUTPUTS
        output = NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0] );
        NEXUS_AudioOutput_GetSettings(output, &audioOutputSettings);
        audioOutputSettings.timebase = timebaseFreerun;
        rc = NEXUS_AudioOutput_SetSettings(output, &audioOutputSettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
#endif
        /* TODO: Modify timebase of other audio outputs as well (e.g. i2s, etc) */

        /* Decouple the Display from input timebase and run it on free running timebase */
        NEXUS_Display_GetSettings(display, &displaySettings);
        displaySettings.timebase = timebaseFreerun;
        rc = NEXUS_Display_SetSettings(display, &displaySettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        break;
    case B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip:
    case B_PlaybackIpClockRecoveryMode_ePushWithPcrNoSyncSlip:
        /* For TTS mode, make sure that timebase that is used for live decode is being also programmed for the output rate managers & display */
#if NEXUS_NUM_AUDIO_DACS
        output = NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]);
        NEXUS_AudioOutput_GetSettings(output, &audioOutputSettings);
        audioOutputSettings.timebase = timebaseLocked;
        rc = NEXUS_AudioOutput_SetSettings(output, &audioOutputSettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
        output = NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]);
        NEXUS_AudioOutput_GetSettings(output, &audioOutputSettings);
        audioOutputSettings.timebase = timebaseLocked;
        rc = NEXUS_AudioOutput_SetSettings(output, &audioOutputSettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
        output = NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0] );
        NEXUS_AudioOutput_GetSettings(output, &audioOutputSettings);
        audioOutputSettings.timebase = timebaseLocked;
        rc = NEXUS_AudioOutput_SetSettings(output, &audioOutputSettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
#endif
        /* TODO: Modify timebase of other audio outputs as well (e.g. i2s, etc) */

        /* Decouple the Display from input timebase and run it on free running timebase */
        NEXUS_Display_GetSettings(display, &displaySettings);
        displaySettings.timebase = timebaseLocked;
        rc = NEXUS_Display_SetSettings(display, &displaySettings);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        break;

    case B_PlaybackIpClockRecoveryMode_ePull:
        break;
    }
    /* ----------------------------------------------------------------------------------------------------------------*/

    /* ----------------------------------------------------------------------------------------------------------------*/
    /* Two additional AV decoder seetings are needed for Live IP Mode */
    /*  1. Delay the start of decoder as per the network jitter by increasing the PTS Offset */
    if (psi.videoCodec != NEXUS_VideoCodec_eNone && psi.videoPid) {
        NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
        if (playbackIpSettings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip) {
            /* nexclient */
            videoDecoderSettings.ptsOffset = IP_NETWORK_JITTER * 45;    /* In 45Khz clock */
        }

        videoDecoderSettings.sourceChanged.callback = sourceChangeCallback;
        videoDecoderSettings.sourceChanged.context = videoDecoder;
        videoDecoderSettings.sourceChanged.param = 1;
#if (BCHP_CHIP == 7445)
        if (ipCfg.hevc) {
            videoDecoderSettings.maxWidth = ipCfg.maxWidth;
            videoDecoderSettings.maxHeight = ipCfg.maxHeight;
            BDBG_MSG(("HEVC decode: program video decoder max width %d, height %d", ipCfg.maxWidth, ipCfg.maxHeight));
        }
#endif
        rc = NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
        if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        BDBG_MSG(("Video Decoder settings are modified for IP \n"));
        if (!audioPid) {
            NEXUS_VideoDecoderExtendedSettings pSettings;
            NEXUS_VideoDecoder_GetExtendedSettings(videoDecoder, &pSettings);
            /* pSettings.zeroDelayOutputMode = true;   Commenting out because it causes jerkiness during playback of normal streams. */
            pSettings.ignoreDpbOutputDelaySyntax = true;
            rc = NEXUS_VideoDecoder_SetExtendedSettings(videoDecoder, &pSettings);
            if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
            BDBG_MSG (("Video Decoder extended settings are modified Ip Mode"));
        }
    }

    if (audioCodec != NEXUS_AudioCodec_eUnknown && audioPid) {
        NEXUS_AudioDecoder_GetSettings(pcmDecoder, &audioDecoderSettings);
        if (playbackIpSettings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip) {
            audioDecoderSettings.ptsOffset = IP_NETWORK_JITTER * 45;    /* In 45Khz clock */
        }

        audioDecoderSettings.sourceChanged.callback = sourceChangeCallback;
        audioDecoderSettings.sourceChanged.context = pcmDecoder;
        audioDecoderSettings.sourceChanged.param = 0;
        rc = NEXUS_AudioDecoder_SetSettings(pcmDecoder, &audioDecoderSettings);
        if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        BDBG_MSG(("PCM Audio Decoder settings are modified for IP \n"));

        NEXUS_AudioDecoder_GetSettings(compressedDecoder, &audioDecoderSettings);
        audioDecoderSettings.ptsOffset = IP_NETWORK_JITTER * 45;    /* In 45Khz clock */
        rc = NEXUS_AudioDecoder_SetSettings(compressedDecoder, &audioDecoderSettings);
        if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        BDBG_MSG(("Compressed Audio Decoder settings are modified for IP \n"));
    }
    /* ----------------------------------------------------------------------------------------------------------------*/

#if NEXUS_HAS_SYNC_CHANNEL
    if (!ipCfg.fastChannelChange) {
        /* connect sync channel */
        NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
        syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);
        if (audioCodec != NEXUS_AudioCodec_eUnknown && audioPid) {
            syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
#if 0
            syncChannelSettings.audioInput[1] = NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed);
#endif
        }
        NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
    }
#endif

#if 0
    /* Needed to comment out starting the compressed decoder, otherwise client based trickmodes for TS content were not working */
    /* as this decoder was not getting flushed during trick play transition. Need to look into this at some point */
    if (audioPid && audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
        rc = NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__); exit(1);}
    }
#endif

    /* connect video decoder to display */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    if (audioCodec != NEXUS_AudioCodec_eUnknown && audioPid) {
#if NEXUS_NUM_AUDIO_DACS
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
            NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
        /* Only pass through AC3 */
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
        /* Only pass through AC3 */
        NEXUS_AudioOutput_AddInput(
            NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0] ),
            NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    }
    if (ipCfg.fastChannelChange) {
        NEXUS_VideoDecoderPrimerSettings primerSettings;
        for (i=0;i<TOTAL_PRIMERS;i++) {
            NEXUS_VideoDecoderPrimer_GetSettings(primer[i], &primerSettings);
            primerSettings.pastTolerance = 1500; /* amount of time willing to race: for IP, start decode from the previous GOP */
            primerSettings.futureTolerance = 0;
            rc = NEXUS_VideoDecoderPrimer_SetSettings(primer[i], &primerSettings);

            /* now start the primer */
            NEXUS_VideoDecoderPrimer_Start(primer[i], &videoProgram);
        }
    }

    /* Start Playing Media */
    if (!playbackIpSettings.useNexusPlaypump) {
        BDBG_MSG(("Start Nexus Playback \n"));
        NEXUS_Playback_GetDefaultStartSettings(&playbackStartSettings);
        playbackStartSettings.mpeg2TsIndexType = NEXUS_PlaybackMpeg2TsIndexType_eSelf;
        rc = NEXUS_Playback_Start(playback, ipSessionSetupStatus.u.http.file, &playbackStartSettings);
        if (rc) {
            BDBG_ERR(("NEXUS Error (%d) at %d, retrying w/o index..\n", rc, __LINE__));
            rc = B_PlaybackIp_SessionStop(playbackIp);
            if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
            rc = B_PlaybackIp_SessionClose(playbackIp);
            if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}

            BDBG_WRN(("Starting IP Applib: w/o index this time..."));
            ipSessionOpenSettings.nonBlockingMode = false;
            rc = B_PlaybackIp_SessionOpen(playbackIp, &ipSessionOpenSettings, &ipSessionOpenStatus);
            if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
            ipSessionSetupSettings.u.http.dontUseIndex = true;
            rc = B_PlaybackIp_SessionSetup(playbackIp, &ipSessionSetupSettings, &ipSessionSetupStatus);
            if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}

            rc = NEXUS_Playback_Start(playback, ipSessionSetupStatus.u.http.file, NULL);
            if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        }
        BDBG_MSG(("Nexus Playback is started\n"));
    }
    else {
        /* playpump is used for all Live modes including Http Live Streaming */
        rc = NEXUS_Playpump_Start(playpump);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        if (playpump2)
        {
            rc = NEXUS_Playpump_Start(playpump2);
            if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        }
        BDBG_MSG(("Nexus Playpump is started\n"));
    }
    audioStarted = true;
    /* ----------------------------------------------------------------------------------------------------------------*/

    /* let IP Applib go ... */
    memset(&ipSessionStartSettings, 0, sizeof(ipSessionStartSettings));
    if (ipCfg.protocol == B_PlaybackIpProtocol_eHttp) {
        ipSessionStartSettings.u.http.preChargeBuffer = false;    /* precharging done via the preChargeNetworkBuffer() call below */
    }
    else if (ipCfg.protocol == B_PlaybackIpProtocol_eRtsp) {
        ipSessionStartSettings.u.rtsp.mediaTransportProtocol = B_PlaybackIpProtocol_eRtp;  /* protocol used to carry actual media */
        ipSessionStartSettings.u.rtsp.keepAliveInterval = 10;  /* send rtsp heart beats (keepalive) every 10sec */
    }
#if 0
    /* Optionally: app can specify RTP Payload type. */
    else if (ipCfg.protocol == B_PlaybackIpProtocol_eRtp) {
        ipSessionStartSettings.u.rtp.rtpPayloadType = 33;
    }
#endif
    /* set Nexus handles */
    if (playpump)
        ipSessionStartSettings.nexusHandles.playpump = playpump;
    if (playpump2)
        ipSessionStartSettings.nexusHandles.playpump2 = playpump2;
    if (playback)
        ipSessionStartSettings.nexusHandles.playback = playback;
    if (psi.videoCodec != NEXUS_VideoCodec_eNone && psi.videoPid) {
        ipSessionStartSettings.nexusHandles.videoDecoder = videoDecoder;
    }
    if (stcChannel)
        ipSessionStartSettings.nexusHandles.stcChannel = stcChannel;
    if (audioCodec != NEXUS_AudioCodec_eUnknown && audioPid && pcmDecoder) {
        ipSessionStartSettings.nexusHandles.primaryAudioDecoder = pcmDecoder;
    }

    if (!ipCfg.gStreamerMode) {
        /* Each of the following are NULL if they are not in use. */
        ipSessionStartSettings.nexusHandles.videoPidChannel =       videoPidChannel;
        ipSessionStartSettings.nexusHandles.extraVideoPidChannel =  extraVideoPidChannel;
        ipSessionStartSettings.nexusHandles.audioPidChannel =       audioPidChannel;
        ipSessionStartSettings.nexusHandles.pcrPidChannel   =       pcrPidChannel;

#if 0
        if (compressedDecoder)
            ipSessionStartSettings.nexusHandles.secondaryAudioDecoder = compressedDecoder;
#endif
        ipSessionStartSettings.nexusHandlesValid = true;
        ipSessionStartSettings.mpegType = psi.mpegType;
        ipSessionStartSettings.mediaPositionUsingWallClockTime = true;

        if ( ipCfg.alternateAudioNumber > 0 && ipCfg.alternateAudioNumber <= psi.hlsAltAudioRenditionCount ) {
            ipSessionStartSettings.startAlternateAudio = true;
            ipSessionStartSettings.alternateAudio.pid = psi.hlsAltAudioRenditionInfo[ipCfg.alternateAudioNumber-1].pid;
            ipSessionStartSettings.alternateAudio.containerType = psi.hlsAltAudioRenditionInfo[ipCfg.alternateAudioNumber-1].containerType;
            ipSessionStartSettings.alternateAudio.language =  psi.hlsAltAudioRenditionInfo[ipCfg.alternateAudioNumber-1].language;
            ipSessionStartSettings.alternateAudio.groupId = psi.hlsAltAudioRenditionInfo[ipCfg.alternateAudioNumber-1].groupId;
        }
        rc = B_PlaybackIp_SessionStart(playbackIp, &ipSessionStartSettings, &ipSessionStartStatus);
        while (rc == B_ERROR_IN_PROGRESS) {
            /* app can do some useful work while SessionSetup is in progress and resume when callback sends the completion event */
            BDBG_MSG (("Session Start call in progress, sleeping for now..."));
            BKNI_Sleep(100);
            rc = B_PlaybackIp_SessionStart(playbackIp, &ipSessionStartSettings, &ipSessionStartStatus);
        }
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); errorFlag=1; goto error;}
    }
    else {
        /* In gStreamerMode, we dont need to start ip_playback, instead app will directly pull data from ip lib & feed to nexus */
    }
    if (ipCfg.fastChannelChange && psi.videoCodec != NEXUS_VideoCodec_eNone && psi.videoPid) {
        int ch;
        BDBG_WRN(("Press ENTER to switch programs\n"));
        ch = getchar();
        BSTD_UNUSED(ch);
        BDBG_WRN(("Starting Decoder w/ Primer ...\n"));
        rc = NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode(primer[0],videoDecoder);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); errorFlag=1;  goto errorClose;}
        BDBG_MSG(("Video Decoder Primer is Started\n"));
    }
    else {
        /* Start Decoders */
        if (psi.videoCodec != NEXUS_VideoCodec_eNone && psi.videoPid) {
            rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
            if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); errorFlag=1;  goto errorClose;}
            BDBG_MSG(("Video Decoder is Started\n"));
        }
    }

    if (audioCodec != NEXUS_AudioCodec_eUnknown && audioPid) {
        rc = NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        BDBG_MSG(("Audio Decoder is Started\n"));
    }

    /* TODO: wait for PreChangeEvent */
    /* Now pre-charge n/w buffer if configured */
    if (ipCfg.preChargeTime && !ipCfg.initialSeekTime) {
        /* do initial pre-charging only if we are not doing any initial seeks */
        if (preChargeNetworkBuffer(playbackIp, ipCfg.preChargeTime)) {
            BDBG_ERR((" #### Initial pre-charge of Network buffer of %d sec failed\n", ipCfg.preChargeTime));
            exit(1);
        }
        BDBG_MSG ((" #### Initial Network pre-charge of %d sec is complete", ipCfg.preChargeTime));
        rc = NEXUS_Playback_Play(playback);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
    }

    /* ----------------------------------------------------------------------------------------------------------------*/


    BDBG_MSG (("Nexus & Platform Setup complete"));
    if (ipCfg.initialSeekTime) {
        /* user wants to skip to specified time in seconds */
        if (B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
            BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__));
            exit(1);
        }
#if 0
        BDBG_WRN(("Seeking Playback by %d sec", ipCfg.initialSeekTime));
        rc = NEXUS_Playback_Seek(playback, ipCfg.initialSeekTime * 1000);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        BDBG_WRN(("Seeked Playback by %d sec\n", ipCfg.initialSeekTime));

        if (preChargeNetworkBuffer(playbackIp, ipCfg.preChargeTime)) {
            BDBG_ERR((" #### Initial pre-charge of Network buffer of %d sec failed\n", ipCfg.preChargeTime));
            exit(1);
        }
        BDBG_WRN((" #### initial Seek: Network pre-charge of %d sec is complete", ipCfg.preChargeTime));

        rc = NEXUS_Playback_Play(playback);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
#else
        B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings);
        ipTrickModeSettings.seekPositionIsRelative = false;
        ipTrickModeSettings.seekBackward = false;
        ipTrickModeSettings.seekPosition = ipCfg.initialSeekTime*1000; /* in msec */
        if (B_PlaybackIp_Seek(playbackIp, &ipTrickModeSettings)) {
            BDBG_ERR(("ERROR: Failed to seek Ip playback\n"));
            errorFlag=1;
            goto error;
        }
#endif
        BDBG_MSG (("Initial Seek successful, playback is started at correct time offset"));
    }

    if (ipCfg.gStreamerMode) {
        /* Note we set the size of bytes to read based on audio only or audio & video channel */
        /* we may or may not know the stream bitrate and thus can't set the read size based on that */
        /* that is where readTimeout (set during sessionSetup) comes handy where it will return */
        /* either when all requested amount is available or when the timeout happens */
        size_t size;
        if (ipSessionSetupStatus.u.http.file == NULL) {
        }
        if (psi.videoCodec == NEXUS_VideoCodec_eNone && !psi.videoPid && audioPid) {
            /* audio only case */
            size = 1024*10;
        }
        else {
            /* audio & video case */
            size = 1024*60;
        }
        while (!gEof) {
            /* get an adequately sized buffer from the playpump */
            static long long totalConsumed;
            void *buffer;
            ssize_t retSize;
            if (utilsGetPlaypumpBuffer(playpump, size, &buffer) < 0) {
                BDBG_ERR(("Can't get a playpump buffer..."));
                break;
            }

            /* read the requested range of data chunk from socket */
            retSize = ipSessionSetupStatus.u.http.file->file.data->read(ipSessionSetupStatus.u.http.file->file.data, (unsigned char *)buffer, size);
            if (retSize == BFILE_ERROR_NO_DATA) {
                BDBG_MSG(("no data available from playback_ip (asked %u), continue reading ...", size));
                continue;
            }
            else if (retSize == 0) {
                BDBG_WRN(("playback_ip returned EOF, we are done ..."));
                break;
            }
            else {
                BDBG_MSG(("playback_ip returned: asked %u, returned %d", size, retSize));
            }

            totalConsumed += retSize;

            /* now feed appropriate data it to the playpump */
            if (NEXUS_Playpump_WriteComplete(playpump, 0, retSize)) {
                BDBG_WRN(("%s: NEXUS_Playpump_WriteComplete failed, continuing...", __FUNCTION__));
                continue;
            }
            BDBG_MSG(("%s: Fed %d bytes (total so far %lld) to Playpump", __FUNCTION__, retSize, totalConsumed));
            /* Exit loop if character q is entered */
            memset(buf, 0, sizeof(buf));
            numbytes = read_with_timeout(0,buf,sizeof(buf),0);
            BDBG_MSG(("ip_client loop: user entered %s", buf));
            if (strncmp(buf, "q", 1) == 0) {
                BDBG_WRN(("breaking from loop as user entered %s", buf));
                break;
            }
        }
        /* we are done, so goto Error */
        goto error;
    }

    B_Time_Get(&prevTime);
    while (!gEof) {
        static int state = 1;
        int curBufferDuration;
        static unsigned prevPts = 0;
        static int rate = 1;
        static unsigned playSpeedIndex;
        static bool usingServerBasedTrickModes = false;
        /* Sleep for a second before we check any status */
        gEof = false;
        static int firstTime = 1;
        static bool printStats = false;

        /* Print various status while decoding */

        NEXUS_VideoDecoder_GetStatus(videoDecoder, &videoStatus);
        rc = NEXUS_Playback_GetStatus(playback, &pbStatus);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        NEXUS_AudioDecoder_GetStatus(pcmDecoder, &audioStatus);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        rc = NEXUS_Playpump_GetStatus(playpump, &ppStatus);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        if (playpump2)
        {
            rc = NEXUS_Playpump_GetStatus(playpump2, &pp2Status);
            if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
        }
        rc = B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatus);
        if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); return (-1);}
        if (psi.videoPid && audioPid)
            NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);

        if (ipCfg.decoderStats && ipCfg.protocol == B_PlaybackIpProtocol_eRtp) {
            B_PlaybackIpRtpRxStats *rtpStats;
            rtpStats = &(playbackIpStatus.rtpStats);
            BDBG_WRN(("rtp stats: bytes rcvd %u, pkts rcvd %llu, discarded %u, outOfSeq %u lost %u lostBefEC %u, lossEvents %u, lossEventsBefEC %u",
                rtpStats->packetsReceived, rtpStats->bytesReceived, rtpStats->packetsDiscarded, rtpStats->packetsOutOfSequence, rtpStats->packetsLost,
                rtpStats->packetsLostBeforeErrorCorrection, rtpStats->lossEvents, rtpStats->lossEventsBeforeErrorCorrection));
        }

        if (ipCfg.decoderStats || printStats) {
            printStats = false;
            BDBG_WRN(("decode %.4dx%.4d, pts %#x, stc %#x (diff %d) fifo size %d, depth %d, fullness %d%%\n",
                        videoStatus.source.width, videoStatus.source.height, videoStatus.pts, stc, videoStatus.ptsStcDifference, videoStatus.fifoSize, videoStatus.fifoDepth, videoStatus.fifoSize?(videoStatus.fifoDepth*100)/videoStatus.fifoSize:0));
            BDBG_WRN(("audio0            pts %#x, stc %#x (diff %d), SR %u, Decoded %u, fifo size %d, depth %d, fullness %d%%\n",
                        audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.sampleRate, audioStatus.framesDecoded, audioStatus.fifoSize, audioStatus.fifoDepth, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0));
#if 0
            NEXUS_AudioDecoder_GetStatus(compressedDecoder, &audioStatus);
            if ( audioStatus.started ) {
                BDBG_WRN(("audio1            pts %#x, stc %#x (diff %d), fifo size %d, depth %d, fullness %d%%",
                            audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize, audioStatus.fifoDepth, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0));
            }
#endif
            BDBG_WRN(("q depth %d, decoded %d, displayed %d, decode errs %d, display errs %d, decode drops %d, display drops %d, display underflow %d, received %d, pts errs %d",
                        videoStatus.queueDepth, videoStatus.numDecoded, videoStatus.numDisplayed, videoStatus.numDecodeErrors,
                        videoStatus.numDisplayErrors, videoStatus.numDecodeDrops, videoStatus.numDisplayDrops, videoStatus.numDisplayUnderflows, videoStatus.numPicturesReceived, videoStatus.ptsErrorCount));

            BDBG_WRN(("playback: ip pos %lu, last %lu, pb pos %lu, fed %lu, first %lu, last %lu, PB buffer depth %d, size %d, fullness %d%%, played bytes %lld, ip bytes consumed %lld",
                        playbackIpStatus.position, playbackIpStatus.last, pbStatus.position, pbStatus.readPosition, pbStatus.first, pbStatus.last, ppStatus.fifoDepth, ppStatus.fifoSize,
                        (ppStatus.fifoDepth*100)/ppStatus.fifoSize, ppStatus.bytesPlayed, (long long)playbackIpStatus.totalConsumed));
            if (playpump2)
            {
                BDBG_WRN(("playback2: ip pos %lu, last %lu, pb pos %lu, fed %lu, first %lu, last %lu, PB buffer depth %d, size %d, fullness %d%%, played bytes %lld, ip bytes consumed %lld",
                            playbackIpStatus.position, playbackIpStatus.last, pbStatus.position, pbStatus.readPosition, pbStatus.first, pbStatus.last, pp2Status.fifoDepth, pp2Status.fifoSize,
                            (pp2Status.fifoDepth*100)/pp2Status.fifoSize, pp2Status.bytesPlayed, (long long)playbackIpStatus.totalConsumed));
            }
            if (firstTime) {
                BDBG_WRN(("Proto %d, URL: %s:%d%s, hls %d, mpegDash %d, security %d",
                            playbackIpStatus.sessionInfo.protocol,
                            playbackIpStatus.sessionInfo.ipAddr,
                            playbackIpStatus.sessionInfo.port,
                            playbackIpStatus.sessionInfo.url,
                            playbackIpStatus.sessionInfo.hlsSessionEnabled,
                            playbackIpStatus.sessionInfo.mpegDashSessionEnabled,
                            playbackIpStatus.sessionInfo.securityProtocol));
                firstTime = 0;
            }
            BDBG_WRN(("HLS stats: url %s, lastSegmentDownloadTime %u, lastSegmentBitrate %u, lastSegmentDuration %u, lastSegmentSequence %u",
                        playbackIpStatus.hlsStats.lastSegmentUrl,
                        playbackIpStatus.hlsStats.lastSegmentDownloadTime,
                        playbackIpStatus.hlsStats.lastSegmentBitrate,
                        playbackIpStatus.hlsStats.lastSegmentDuration,
                        playbackIpStatus.hlsStats.lastSegmentSequence
                        ));
        }
        B_Time_Get(&curTime);
        /*Limited time decode for liveChannel if its specified using -T option*/
        if ((ipCfg.decodeTimeDuration > 0) && (unsigned)(B_Time_Diff(&curTime,&prevTime)/1000) > ipCfg.decodeTimeDuration) {
            BDBG_WRN(("*********** stopping ip playback session, total playback time (%d) defined by user exceeded ***********", ipCfg.decodeTimeDuration));
            break;
        }

        if (playbackIpStatus.ipState == B_PlaybackIpState_eTrickMode && gEof) {
            if (B_PlaybackIp_Play(playbackIp)) {
                BDBG_ERR(("ERROR: Failed to pause Ip playback"));
                errorFlag=1;
                goto error;
            }
            state = 1;
            rate = 1;
        }
        else if (playbackIpStatus.ipState == B_PlaybackIpState_ePaused || playbackIpStatus.ipState == B_PlaybackIpState_eTrickMode)
            goto skip_runtime_buffering_check;
#define RUNTIME_BUFFERING_CODE
#ifdef RUNTIME_BUFFERING_CODE
        if (psi.mpegType == NEXUS_TransportType_eMp4) {
            curBufferDuration = pbStatus.readPosition - pbStatus.position;
            BDBG_MSG(("buffered %d mill-seconds worth of MP4 content", curBufferDuration));
        }
        else if (psi.mpegType == NEXUS_TransportType_eAsf) {
            curBufferDuration = (ppStatus.mediaPts - (psi.videoPid?videoStatus.pts:audioStatus.pts))/45;
            BDBG_MSG(("buffered %d milli-seconds worth of ASF content", curBufferDuration));
        }
        else {
            /* we need to use alternate means to determine the amount of buffering in system since such formats are not processed in sw and thus we dont know the curBufferDepth */
            /* instead, we can detect the underflow condition by monitoring the last pts displayed, once it doesn't change, we are definitely underflowing and thus can precharge */
            /* by default, set curBufferDuration to a higher number to avoid precharging */
            curBufferDuration = 99999; /* set to some large value */
            if (prevPts) {
                if (prevPts == (psi.videoPid?videoStatus.pts:audioStatus.pts)) {
                    /* pts hasn't changed, so we are underflowing, set flag to precharge */
                    BDBG_MSG(("pts hasn't changed, so we are underflowing, prev pts, %u, cur pts %u, pre charge time %d", prevPts, psi.videoPid?videoStatus.pts:audioStatus.pts, ipCfg.preChargeTime));
                    curBufferDuration = 0;
                }
            }
            prevPts = psi.videoPid?videoStatus.pts:audioStatus.pts;
            BDBG_MSG(("prev pts, %u, cur pts %u", prevPts, psi.videoPid?videoStatus.pts:audioStatus.pts));
        }
        if (ipCfg.preChargeTime && (curBufferDuration < 200)) {
            /* we are precharging & current buffer level is below the low water mark, so start pre-charging */
            /* however, sleep quickly to see if underflow is due to EOF. Otherwise, we will Pause Playback too quickly */
            /* before we get the EOF callback. Sleep gives Nexus Playback a chance to invoke the eof callback. */
            BKNI_Sleep(500);
            if (gEof) {
                BDBG_WRN(("Underflow is due to EOF, breaking out..."));
                break;
            }
            BDBG_WRN(("Underflowing, so pausing the playback until enough buffering is done..."));
            if (NEXUS_Playback_Pause(playback)) {
                BDBG_WRN(("ERROR: Failed to pause Nexus playback"));
                break;
            }
            BDBG_MSG(("Paused Nexus Playback..."));

            /* Now pre-charge n/w buffer */
            if (preChargeNetworkBuffer(playbackIp, ipCfg.preChargeTime)) {
                BDBG_ERR((" #### runtime pre-charge of Network buffer of %d sec failed", ipCfg.preChargeTime));
                break;
            }
            BDBG_MSG(("Resuming Playback"));
            if (NEXUS_Playback_Play(playback)) {
                BDBG_WRN(("ERROR: Failed to play Nexus playback from pause"));
                break;
            }
        }
skip_runtime_buffering_check:
#endif

        /* Exit loop if character is entered */
        memset(buf, 0, sizeof(buf));
        numbytes = read_with_timeout(0,buf,sizeof(buf),1000);
        if (numbytes > 0) {
            if (buf[0] == 'p' && state == 1) {
                state = 0;
                if (B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...", rc, __LINE__));
                    exit(1);
                }
                ipTrickModeSettings.pauseMethod = B_PlaybackIpPauseMethod_UseDisconnectAndSeek;
                ipTrickModeSettings.pauseMethod = B_PlaybackIpPauseMethod_UseConnectionStalling;
                BDBG_WRN(("Pausing Playback using method %d", ipTrickModeSettings.pauseMethod));
                if (B_PlaybackIp_Pause(playbackIp, &ipTrickModeSettings)) {
                    BDBG_ERR(("ERROR: Failed to pause Ip playback"));
                    errorFlag=1;
                    goto error;
                }
            }
            else if (strncmp(buf, "pause", 5) == 0) {
                /* Explicit pause. */
                state = 0;
                if (B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...", rc, __LINE__));
                    exit(1);
                }
                ipTrickModeSettings.pauseMethod = B_PlaybackIpPauseMethod_UseConnectionStalling;
                BDBG_WRN(("Pausing Playback using method %d", ipTrickModeSettings.pauseMethod));
                if (B_PlaybackIp_Pause(playbackIp, &ipTrickModeSettings)) {
                    BDBG_ERR(("ERROR: Failed to pause Ip playback"));
                    errorFlag=1;
                    goto error;
                }
            }
            else if (strncmp(buf, "sr1", 3) == 0) {
                if (B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...", rc, __LINE__));
                    exit(1);
                }
                rc = B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatus);
                if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); return (-1);}
                if (playbackIpStatus.last > 10000)
                    ipTrickModeSettings.seekPosition = playbackIpStatus.last - 10000;
                else
                    ipTrickModeSettings.seekPosition = 0;
                BDBG_WRN(("Seek, Sleep, Rew Test: jumping to %u sec ", (unsigned int)ipTrickModeSettings.seekPosition));
                if (B_PlaybackIp_Seek(playbackIp, &ipTrickModeSettings)) {
                    BDBG_ERR(("ERROR: Failed to seek Ip playback"));
                    errorFlag = 1;
                    break;
                }
                BKNI_Sleep(500);
                if (B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_ERR(("Error (%d) at %d, Exiting...", rc, __LINE__));
                    exit(1);
                }
                ipTrickModeSettings.playSpeedString = "-15";
                ipTrickModeSettings.playSpeedStringDefined = true;
                if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_WRN(("Failed to set the trick play speed %d, index %d\n", psi.playSpeed[playSpeedIndex], playSpeedIndex));
                    break;
                }
                BDBG_WRN(("Rewind is started!"));
            }
            else if (buf[0] == 'p' && state == 0) {
                BDBG_WRN(("Resuming Playback"));
                state = 1;
                rate = 1;
                if (B_PlaybackIp_Play(playbackIp)) {
                    BDBG_ERR(("ERROR: Failed to pause Ip playback"));
                    errorFlag=1;
                    goto error;
                }
                BDBG_WRN(("After Resuming Playback"));
            }
            else if (strncmp(buf, "fs", 2) == 0 || strncmp(buf, "rs", 2) == 0 ) {
                int dir = (strncmp(buf, "fs", 2) == 0) ? 1 : -1;
                int rate;
                buf[5] = buf[6] = '\0';
                rate = atoi(&buf[2]);
                if (rate == 0 || rate > NEXUS_NORMAL_PLAY_SPEED)
                    rate = NEXUS_NORMAL_PLAY_SPEED;
                state = 0;  /* goto to play when when p is entered */
                BDBG_WRN(("set the trick play slow %s command", dir == 1?"fwd": "rwd"));
                if (B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...", rc, __LINE__));
                    exit(1);
                }
                ipTrickModeSettings.absoluteRateDefined = true;
                if (dir > 0)
                    ipTrickModeSettings.absoluteRate = rate;
                else
                    ipTrickModeSettings.absoluteRate = -rate;
                if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_WRN(("Failed to set the trick play speed %d, index %d", psi.playSpeed[playSpeedIndex], playSpeedIndex));
                    break;
                }
                BDBG_WRN(("Successfully set the trick play slow %s command", dir == 1?"fwd": "rwd"));
            }
            else if (buf[0] == 's' && buf[1] == 't') {
                printStats = true;
            }
            else if (buf[0] == 's') {
                int seekTime = 0;
                buf[6] = '\0';
                if (B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...", rc, __LINE__));
                    exit(1);
                }
                if (buf[1] == 'r') {
                    /* seek relative */
                    seekTime = atoi(&buf[2]);
                    ipTrickModeSettings.seekPositionIsRelative = true;
                }
                else {
                    seekTime = atoi(&buf[1]);
                    ipTrickModeSettings.seekPositionIsRelative = false;
                }

                if (seekTime < 0) {
                    ipTrickModeSettings.seekPositionIsRelative = true;
                    ipTrickModeSettings.seekBackward = true;
                    seekTime = ~seekTime + 1;
                }
                else
                    ipTrickModeSettings.seekBackward = false;
                /* user wants to skip to specified time in seconds */
                ipTrickModeSettings.seekPosition = seekTime*1000; /* in msec */
                /* this seek example shows that apps can cause seek in the non-blocking mode */
                ipTrickModeSettings.nonBlockingMode = true;
                if (psi.numPlaySpeedEntries <= 1)
                    ipTrickModeSettings.method = B_PlaybackIpTrickModesMethod_UseByteRange;
                else
                    ipTrickModeSettings.method = B_PlaybackIpTrickModesMethod_UseTimeSeekRange;
                ipTrickModeSettings.enableAccurateSeek = true;
                BDBG_WRN(("IP Playback seeking to %d sec, method %d ", seekTime, ipTrickModeSettings.method));
                BKNI_ResetEvent(waitEvent);
                rc = B_PlaybackIp_Seek(playbackIp, &ipTrickModeSettings);
                if (rc == B_ERROR_IN_PROGRESS) {
                    BERR_Code err;
                    /* app can do some useful work while seek is in progress and resume when callback sends the completion event */
                    BDBG_MSG (("seek call in progress, sleeping for now..."));
                    err = BKNI_WaitForEvent(waitEvent, 10000);
                    if (err == BERR_TIMEOUT || err != 0) {
                        BDBG_WRN(("Seek call timed out in or got error (err %d)!!!", err));
                        errorFlag=1;
                        goto errorClose;
                    }
                    /* in this case, callback function has already checked the status of call, so we can proceed here */
                }
                else if (rc != B_ERROR_SUCCESS) {
                    BDBG_WRN(("Seek call failed (rc = %d)!!!", rc));
                    errorFlag=1;
                    goto errorClose;
                }
                state = 1;
                BDBG_WRN(("IP Playback is started at %d time pos %lu", seekTime, (unsigned long)ipTrickModeSettings.seekPosition));
            }
            else if (buf[0] == 'j') {
                /* jump forward by a fixed time (defaults to 5 sec) */
                int seekTime = 0;
                if (B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...", rc, __LINE__));
                    exit(1);
                }
                /* seek relative */
                seekTime = ipCfg.secondsToJump;
                ipTrickModeSettings.seekPositionIsRelative = true;
                if (seekTime < 0) {
                    ipTrickModeSettings.seekBackward = true;
                    seekTime = ~seekTime + 1;
                }
                else
                    ipTrickModeSettings.seekBackward = false;
                /* user wants to skip to specified time in seconds */
                ipTrickModeSettings.seekPosition = seekTime*1000; /* in msec */
                BDBG_WRN(("IP Playback jumping to %d sec ", seekTime));
                if (B_PlaybackIp_Seek(playbackIp, &ipTrickModeSettings)) {
                    BDBG_ERR(("ERROR: Failed to seek Ip playback"));
                    errorFlag=1;
                    goto error;
                }
                state = 1;
                BDBG_WRN(("IP Playback is started at %d time pos %lu", seekTime, (unsigned long)ipTrickModeSettings.seekPosition));
            }
            else if (strncmp(buf, "tm", 2) == 0) {
                if (B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...", rc, __LINE__));
                    exit(1);
                }
                ipTrickModeSettings.method = B_PlaybackIpTrickModesMethod_UsePlaySpeed;
                ipTrickModeSettings.playSpeedString = &buf[2];
                ipTrickModeSettings.playSpeedStringDefined = true;
                if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_WRN(("Failed to set the trick play rate %s for server side trickmodes", &buf[2]));
                    break;
                }
                BDBG_WRN(("%s: playSpeed = %s", __FUNCTION__, &buf[2]));
                state = 0;  /* goto to play when when p is entered */
            }
            else if (strncmp(buf, "ff", 2) == 0 || strncmp(buf, "fr", 2) == 0 ) {
                int dir = (strncmp(buf, "ff", 2) == 0) ? 1 : -1;
                state = 0;  /* goto to play when when p is entered */
                if (B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...", rc, __LINE__));
                    exit(1);
                }
                if (psi.numPlaySpeedEntries <= 1) {
                    /* client side trickmodes */
                    if (dir > 0)
                        rate++;
                    else {
                        if (rate == 1)
                            rate = -1;
                        else
                            rate--;
                    }
                    ipTrickModeSettings.method = B_PlaybackIpTrickModesMethod_UseByteRange;
                    ipTrickModeSettings.rate = rate;
#ifdef USE_ABSOLUTE_RATE
                    if (dir > 0) {
                        ipTrickModeSettings.absoluteRateDefined = true;
                        ipTrickModeSettings.absoluteRate = NEXUS_NORMAL_PLAY_SPEED/2;
                    }
                    else {
                        ipTrickModeSettings.absoluteRateDefined = true;
                        ipTrickModeSettings.absoluteRate = -NEXUS_NORMAL_PLAY_SPEED/2;
                    }
#endif
                    ipTrickModeSettings.pauseMethod = B_PlaybackIpPauseMethod_UseDisconnectAndSeek;
                    if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                            BDBG_WRN(("Failed to set the trick play rate %d for client side trickmodes", rate));
                            break;
                    }
                    BDBG_WRN(("Successfuly set the trick play rate %d for client side trickmodes", rate));
                    usingServerBasedTrickModes = false;
                }
                else {
                    /* server side trickmodes */
                    BDBG_WRN(("trick play rate %d, index %d, totol entries %d", rate, playSpeedIndex, psi.numPlaySpeedEntries));
                    unsigned i;
                    /* server has provided various supported speeds during trick mode */
                    ipTrickModeSettings.method = B_PlaybackIpTrickModesMethod_UsePlaySpeed;
                    ipTrickModeSettings.pauseMethod = B_PlaybackIpPauseMethod_UseDisconnectAndSeek;
                    /* now pick the correct speed from the playSpeed Arrary */
                    if (rate == 1 && dir> 0) {
                        /* we are now switching from normal to FFWD trick play, find the 1st positive speed */
                        for (i=0; i< psi.numPlaySpeedEntries; i++) {
                            if (psi.playSpeed[i] > 0) {
                                /* 1st positive speed */
                                playSpeedIndex = i;
                                break;
                            }
                        }
                    }
                    else if (rate == 1 && dir < 0) {
                        /* we are now switching from normal to FREW trick play, find the 1st -ve speed */
                        for (i=0; i< psi.numPlaySpeedEntries; i++) {
                            if (psi.playSpeed[i] > 0) {
                                /* 1st positive speed */
                                playSpeedIndex = i-1;
                                break;
                            }
                        }
                    }
                    else {
                        /* we are already in the fwd trick play state, update the index */
                        if (dir>0)
                            playSpeedIndex++;
                        else
                            playSpeedIndex--;
                    }
                    if (playSpeedIndex >= psi.numPlaySpeedEntries) {
                        playSpeedIndex = psi.numPlaySpeedEntries - 1;
                    }
                    ipTrickModeSettings.rate = psi.playSpeed[playSpeedIndex];
                    if (ipTrickModeSettings.rate >= psi.httpMinIFrameSpeed || dir < 0)
                        ipTrickModeSettings.frameRepeat = psi.httpFrameRepeat;
                    else
                        ipTrickModeSettings.frameRepeat = 1;
#if 0
                        /* test code to try out the fractional playspeeds */
                    if (1)
                    {
                        char playSpeedString[16];
                        /*sprintf(playSpeedString, " 1 / %d", playSpeedIndex);*/
                        sprintf(playSpeedString, " 7/8");
                        printf(">>>>>>>>> playSpeed %s\n", playSpeedString);
                        ipTrickModeSettings.playSpeedString = playSpeedString;
                        ipTrickModeSettings.playSpeedStringDefined = true;
                    }
#endif
                    if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                        BDBG_WRN(("Failed to set the trick play speed %d, index %d\n", psi.playSpeed[playSpeedIndex], playSpeedIndex));
                        break;
                    }
                    BDBG_WRN(("Successfully set the trick play speed %d, index %d, slow motion %d", psi.playSpeed[playSpeedIndex], playSpeedIndex, ipTrickModeSettings.frameRepeat));
                    usingServerBasedTrickModes = true;
                    rate++;
                }
            }
            else if (buf[0] == 'f' || buf[0] == 'r') {
                BDBG_WRN(("speed %s", &buf[1]));
                state = 0;  /* goto to play when when p is entered */
                if (B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_ERR(("Error (%d) at %d, Exiting...", rc, __LINE__));
                    exit(1);
                }
                ipTrickModeSettings.playSpeedString = &buf[1];
                ipTrickModeSettings.playSpeedStringDefined = true;
                if (B_PlaybackIp_TrickMode(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                    BDBG_WRN(("Failed to set the trick play speed %d, index %d\n", psi.playSpeed[playSpeedIndex], playSpeedIndex));
                    break;
                }
            }
            else if (buf[0] == 'a' && buf[1] == 'l') {
                unsigned alternateAudioNumber;
                alternateAudioNumber = atoi(&buf[2]);
                if (alternateAudioNumber > psi.hlsAltAudioRenditionCount) {
                    BDBG_WRN(("Requested alternateAudioNumber=%d can't be played, its > hlsAltAudioRenditionCount=%d", alternateAudioNumber, psi.hlsAltAudioRenditionCount));
                    continue;
                }
                else if (currentAlternateAudioNumber == alternateAudioNumber) {
                    BDBG_WRN(("Already playing this audio rendition# %d", currentAlternateAudioNumber));
                    continue;
                }
                else if (alternateAudioNumber != 0) {
                    BDBG_WRN(("Switching from %s (%d) to alternateAudioNumber=%d", currentAlternateAudioNumber==0?"Main":"Another alternate", currentAlternateAudioNumber, alternateAudioNumber));
                    /* We are not switching to the main audio and thus to one of the audio renditions. */
                    BDBG_ASSERT(alternateAudioNumber > 0);
                }
                else {
                    BDBG_WRN(("Switching from current alternateAudioNumber=%d to main audio", currentAlternateAudioNumber));
                    BDBG_ASSERT(alternateAudioNumber == 0);
                    /* When coming back to main. */
                }

                {
                    B_PlaybackIp_GetSettings(playbackIp, &playbackIpSettings);

                    /* Stop Alternate Audio if it is currently being played. */
                    if (currentAlternateAudioNumber != 0) {
                        playbackIpSettings.stopAlternateAudio = true;
                        rc = B_PlaybackIp_SetSettings(playbackIp, &playbackIpSettings);
                        if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); return (-1);}
                        BDBG_WRN(("Stopped current alternate rendition playback"));
                    }

                    /* Now stop the audio decoder & close existingPidChannel. */
                    if (audioPidChannel) {
#if 0
                        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
                        NEXUS_AudioDecoder_Stop(pcmDecoder);
                        NEXUS_Playpump_ClosePidChannel(playpump2?playpump2:playpump, audioPidChannel);
                        BDBG_WRN(("Stopped Audio decoder & closed the existing Audio PidChannel"));
                        audioPidChannel = NULL;
                    }

                    /* Stop/Close Playpump2 if its being used. */
                    if (playpump2) {
                        NEXUS_Playpump_Stop(playpump2);
                        NEXUS_Playpump_Close(playpump2);
                        playpump2 = NULL;
                    }
                    if (alternateAudioNumber) BDBG_WRN(("Now switch to audio rendition w/ %d: language=%s", alternateAudioNumber, psi.hlsAltAudioRenditionInfo[alternateAudioNumber-1].language));

                    /* Open/Start playpump2 if we are switching to alternate audio rendition. */
                    if (alternateAudioNumber != 0) {
                        playpump2 = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
                        if (!playpump2) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
                        NEXUS_Playpump_GetSettings(playpump2, &playpumpSettings);
                        playpumpSettings.transportType = psi.hlsAltAudioRenditionInfo[alternateAudioNumber-1].containerType;
                        rc = NEXUS_Playpump_SetSettings(playpump2, &playpumpSettings);
                        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); errorFlag=1; goto error;}
                        rc = NEXUS_Playpump_Start(playpump2);
                        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); errorFlag=1; goto error;}
                        BDBG_WRN (("Started playpump2 for alternateAudio"));
                    }

                    /* Open new pid channel next. */
                    {
                        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
                        pidChannelSettings.pidType = NEXUS_PidType_eAudio;
                        audioCodec = alternateAudioNumber==0?  psi.audioCodec: psi.hlsAltAudioRenditionInfo[alternateAudioNumber-1].codec;
                        pidChannelSettings.pidTypeSettings.audio.codec = audioCodec;
                        audioPid = alternateAudioNumber==0?  psi.audioPid: psi.hlsAltAudioRenditionInfo[alternateAudioNumber-1].pid;
                        if (audioCodec != NEXUS_AudioCodec_eUnknown) {
                            audioPidChannel = NEXUS_Playpump_OpenPidChannel(playpump2?playpump2:playpump, audioPid, &pidChannelSettings);
                            if (!audioPidChannel) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
                            BDBG_WRN (("Opened audio pid channel %p for alternateAudioNumber %u audio pid %d ", (void *)audioPidChannel, alternateAudioNumber, audioPid));

                            NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
                            audioProgram.codec = audioCodec;
                            audioProgram.pidChannel = audioPidChannel;
                            audioProgram.stcChannel = stcChannel;
                            rc = NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
                            if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...\n", rc, __LINE__)); exit(1);}
                            BDBG_WRN (("Started Audio Decoder"));
                        }
                    }

                    /* And finally start the alternate rendition playback. */
                    if (alternateAudioNumber != 0) {
                        playbackIpSettings.startAlternateAudio = true;
                        playbackIpSettings.nexusHandles.playpump2 = playpump2;
                        playbackIpSettings.nexusHandlesValid = true;
                        playbackIpSettings.alternateAudio.pid = psi.hlsAltAudioRenditionInfo[alternateAudioNumber-1].pid;
                        playbackIpSettings.alternateAudio.containerType = psi.hlsAltAudioRenditionInfo[alternateAudioNumber-1].containerType;
                        playbackIpSettings.alternateAudio.language =  psi.hlsAltAudioRenditionInfo[alternateAudioNumber-1].language;
                        playbackIpSettings.alternateAudio.groupId = psi.hlsAltAudioRenditionInfo[alternateAudioNumber-1].groupId;
                        rc = B_PlaybackIp_SetSettings(playbackIp, &playbackIpSettings);
                        if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); return (-1);}
                    }
                    BDBG_WRN(("Successfully switched from %s (%d) to alternateAudioNumber=%d", currentAlternateAudioNumber==0?"Main":"Another alternate", currentAlternateAudioNumber, alternateAudioNumber));
                    currentAlternateAudioNumber = alternateAudioNumber;
                }
            }
            else if (buf[0] == 'a') {
                bool audioOnly;
                audioOnly = atoi(&buf[1]);
                BDBG_WRN(("switching to %s segment", audioOnly ? "audioOnly":"audioVideo"));
                if (audioOnly)
                    setenv("audioOnly", "1", 1);
                else
                    setenv("audioOnly", "0", 1);
            }
            else if (strncmp(buf, "q", 1) == 0) {
                BDBG_WRN(("breaking from loop as user entered %s", buf));
                userQuit = true;
                replayStream = false;
                break;
            }
            else if (strncmp(buf, "chg", 3) == 0) {
                replayStream = true;
                BDBG_WRN((" ############## replaying stream ################ "));
                break;
            }
            else if (strncmp(buf, "h", 1) == 0 || strncmp(buf, "?", 1) == 0) {
                BDBG_WRN(("runtime options are--->"));
                printf("\ts[-+]<num>: seek to this absolute position in sec. e.g. s30 to seek to 30sec in movie\n");
                printf("\tsr[-+]<num>: seek relative in backward or forward direction by sec. e.g. sr-30 to seek to 30sec backwards from current point\n");
                printf("\tj<: jump forward or backward by fixed time (defaults to 5sec, modified via -j startup option.\n");
                printf("\tp: toggle between pause/play playback\n");
                printf("\tff: fast forward to next +ve speed \n");
                printf("\tfr: fast rewind to previous -ve speed\n");
                printf("\tf <speed>: fast forward to specified +ve speed \n");
                printf("\tr <rewind>: fast rewind to specified -ve speed\n");
                printf("\tst: print currnet AV decoders & Playback stats\n");
                printf("\ta[0|1]: switch to audio only (a1) segment playlist, default is AV playlist\n");
                printf("\tal[number]: switch to this audio rendition position. 0: main audio, 1: 1st alternate, 2: 2nd alternate, upto max of psi.hlsAltAudioRenditionCount]\n");
            }
            else {
                BDBG_WRN(("Continuing loop: user entered %s", buf));
                printStats = false;
            }
        } /* else assume EAGAIN */

        if (gResumePlay) {
            if (B_PlaybackIp_Play(playbackIp) == B_ERROR_SUCCESS) {
                BDBG_WRN(("%s: rewind reached beginning of file, so back to play successful", __FUNCTION__));
            }
            else {
                BDBG_WRN(("%s: rewind reached beginning of file, but back to play failed", __FUNCTION__));
                errorFlag = 1;
                break;
            }
            gResumePlay = false;
        }
        if (gEndOfSegments) {
            if (B_PlaybackIp_GetTrickModeSettings(playbackIp, &ipTrickModeSettings) != B_ERROR_SUCCESS) {
                BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...", rc, __LINE__));
                exit(1);
            }
            rc = B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatus);
            if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); return (-1);}
            if (playbackIpStatus.position > 10000)
                ipTrickModeSettings.seekPosition = playbackIpStatus.position - 10000;
            else
                ipTrickModeSettings.seekPosition = 0;
            BDBG_WRN(("gEndOfSegments Event: IP Playback jumping to %lu sec ", (unsigned long)ipTrickModeSettings.seekPosition));
            if (B_PlaybackIp_Seek(playbackIp, &ipTrickModeSettings)) {
                BDBG_ERR(("ERROR: Failed to seek Ip playback"));
                errorFlag = 1;
                break;
            }
            gEndOfSegments = false;
            BDBG_WRN(("IP Playback is started at time pos %lu after reaching end of current segments!", (unsigned long)ipTrickModeSettings.seekPosition));
        }
        if (ipCfg.runUnitTests) {
            if (runMultipleSeekTestAndVerify(&psi, playbackIp) < 0) {
                BDBG_ERR(("Auto tests failed"));
                errorFlag = 1;
            }
            if (runPlaybackIpUnitTests(&psi, playbackIp) < 0) {
                BDBG_ERR(("Auto tests failed"));
                errorFlag = 1;
            }
            else
                BDBG_ERR(("Auto tests worked"));
            if (!ipCfg.loop) {
                BDBG_WRN(("Breaking main loop"));
                break;
            }
        }
    }

error:
    if (!replayStream && !userQuit && !ipCfg.runUnitTests && !errorFlag && psi.duration && ipCfg.protocol == B_PlaybackIpProtocol_eHttp) {
        unsigned endDuration;
        rc = B_PlaybackIp_GetStatus(playbackIp, &playbackIpStatus);
        if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); return (-1);}

        endDuration = psi.duration/1000;
        if (ipCfg.decodeTimeDuration && ipCfg.decodeTimeDuration < psi.duration/1000)
            endDuration = ipCfg.decodeTimeDuration;
        if (playbackIpStatus.position/1000 < (endDuration-1)) {
            BDBG_WRN(("###########Didn't play the whole file: duration %d, last position %lu", endDuration, (unsigned long)playbackIpStatus.position));
            errorFlag = 1;
        }
        else
            BDBG_WRN(("Played the whole file: duration %d, last position %lu, errorFlag %d", psi.duration, (unsigned long)playbackIpStatus.position, errorFlag));
    }
    if (playbackIp)
        B_PlaybackIp_SessionStop(playbackIp);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
#endif

    if (audioPidChannel) NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    //NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
    if (audioPidChannel) NEXUS_AudioDecoder_Stop(pcmDecoder);
    if (videoPidChannel) NEXUS_VideoDecoder_Stop(videoDecoder);

    /* Do cleanup necessary for changing channels */
    if (!playbackIpSettings.useNexusPlaypump) {
        BDBG_MSG (("Stopping Nexus Playback module"));
        if(playback)
        {
            NEXUS_Playback_CloseAllPidChannels(playback);
            NEXUS_Playback_Stop(playback);
        }
    }
    else {
        BDBG_MSG (("Stopping Nexus Playpump module"));
        NEXUS_Playpump_Stop(playpump);
        NEXUS_Playpump_CloseAllPidChannels(playpump);
        if (playpump2) {
            NEXUS_Playpump_Stop(playpump2);
            NEXUS_Playpump_CloseAllPidChannels(playpump2);
        }
    }

errorClose:
#if NEXUS_HAS_SYNC_CHANNEL
    if (!ipCfg.fastChannelChange && syncChannel) {
        /* disconnect sync channel */
        NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
        syncChannelSettings.videoInput = NULL;
        syncChannelSettings.audioInput[0] = NULL;
        syncChannelSettings.audioInput[1] = NULL;
        NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
    }
#endif

    /* Close Socket related resources */
    if (playbackIp != NULL) {
        /* coverity[freed_arg=FALSE] */
        /* coverity[double_free=FALSE] */
        rc = B_PlaybackIp_SessionClose(playbackIp);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, Exiting...", rc, __LINE__)); exit(1);}
    }

    if (ipCfg.playMultipleStreams||replayStream) {
        BDBG_WRN((" ############## replaying stream ################ "));
        goto newSession;
    }
#ifdef B_HAS_DTCP_IP
    /* close the per session handle */
    if (ipCfg.security == B_PlaybackIpSecurityProtocol_DtcpIp) {
        if (DtcpAppLib_CloseAke(dtcpCtx, AkeHandle) != BERR_SUCCESS) {
            BDBG_WRN(("%s: failed to close the DTCP AKE session", __FUNCTION__));
        }
        DtcpAppLib_Shutdown(dtcpCtx);
#if 0
        /* removing call to PlaybackIp_DtcpIpUnInit() for now since ip_client is directly calling Dtcp lib's init function */
        B_PlaybackIp_DtcpIpUnInit(dtcpCtx);
#endif
#ifdef B_DTCP_IP_HW_DECRYPTION
        DtcpCleanupHwSecurityParams();
#endif
    }
#endif

#ifdef B_HAS_SSL
    if (ipCfg.security == B_PlaybackIpSecurityProtocol_Ssl){
        B_PlaybackIp_SslUnInit(ssl_ctx);
    }
#endif

    if (ipCfg.fastChannelChange) {
        for (i=0;i<TOTAL_PRIMERS;i++) {
            NEXUS_VideoDecoderPrimer_Close(primer[i]);
        }
    }

    if (pcmDecoder) NEXUS_AudioDecoder_Close(pcmDecoder);
    if (compressedDecoder) NEXUS_AudioDecoder_Close(compressedDecoder);
    if (window) NEXUS_VideoWindow_RemoveAllInputs(window);
    if (videoDecoder) NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    if (window) NEXUS_VideoWindow_Close(window);
    if (display) NEXUS_Display_Close(display);
    if (videoDecoder) NEXUS_VideoDecoder_Close(videoDecoder);
    if (stcChannel) NEXUS_StcChannel_Close(stcChannel);
    if (timebaseInitialized) {
        NEXUS_Timebase_Close(timebaseLocked);
        NEXUS_Timebase_Close(timebaseFreerun);
    }
    /* Do cleanup necessary for quitting application */
    if (playback) NEXUS_Playback_Destroy(playback);
    if (playpump) NEXUS_Playpump_Close(playpump);
    if (playpump2) NEXUS_Playpump_Close(playpump2);

    if (playbackIp) B_PlaybackIp_Close(playbackIp);
#if NEXUS_HAS_SYNC_CHANNEL
    if (syncChannel) NEXUS_SyncChannel_Destroy(syncChannel);
#endif
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
    if (dmaHandle != NULL) {
        NEXUS_Dma_Close(dmaHandle);
    }
#endif
    if (waitEvent)
        BKNI_DestroyEvent(waitEvent);
    NEXUS_Platform_Uninit();
    /* For debugging any memory leaks, use this */
    BDBG_ERR(("!!!! errorFlag %d", errorFlag));
    BKNI_Uninit();
    {
        FILE *fp = NULL;
        char buf[16];
        int len;
        int appExitCode;
        fp = fopen("./result.txt", "w+");
        if (errorFlag)
            appExitCode = 1;
        else
            appExitCode = 0;
        len = snprintf(buf, 15, "%d", appExitCode);
        if (fp) {
            fwrite(buf, 1, len, fp);
            fclose(fp);
        }
    }
    if(errorFlag == 1) {  /* Checking if the run completes successfully or with the errors */
        return -1;
    }
    else{
        return 0;
    }

}
