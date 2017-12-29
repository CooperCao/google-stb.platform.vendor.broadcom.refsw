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
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>

#include "b_playback_ip_lib.h"
#ifndef DMS_CROSS_PLATFORMS
#endif /* DMS_CROSS_PLATFORMS */
#include "ip_streamer_lib.h"
#include "ip_streamer.h"
#include "ip_http.h"
#include "b_os_lib.h"
#include "../src/b_playback_ip_xml_helper.h"

/*CAD/BRCM 20121031 - Added RTSPServer support*/
#ifdef LIVEMEDIA_SUPPORT
#include <RTSPServer.hh>
#endif

BDBG_MODULE(ip_streamer);
#define CA_CERT "host.cert"
#define DEFAULT_CONTENT_DIR "/data/videos"
#define DEFAULT_TIMESHIFT_DIR "/data/timeshift"
#define DEFAULT_TIMESHIFT_INTERVAL (60*30)  /* 30 minutes */
#define DEFAULT_RTSP_PORT 554
#define REQUEST_URL_LEN 4096

#if 0
#define BDBG_MSG_FLOW(x)  BDBG_WRN( x );
#else
#define BDBG_MSG_FLOW(x)
#endif

/* Globals */
#define HLS_XCODE_RAMUP_SEGMENTS 1
int gExitThread = 0;
pthread_t gThreadId[IP_STREAMER_MAX_THREADS], gChannelMapThreadId=0;
#ifdef LIVEMEDIA_SUPPORT
#if LIVEMEDIA_2013_03_07
pthread_t gRTSPServerThreadId=0/*CAD/BRCM 20121031 - Added RTSPServer support*/;
#endif /*LIVEMEDIA_2013_03_07*/
#endif /* LIVEMEDIA_SUPPORT */
IpStreamerCtx * _ipStreamerCtx = NULL;
static struct ipStreamerCtxQueue ipStreamerCtxQueueHead;
static BKNI_MutexHandle ipStreamerCtxQueueMutex = NULL;
#ifdef B_USE_HTTP_KEEPALIVE
static BKNI_EventHandle ipStreamerDestroySession = NULL;
#endif
static bool gGotSigInt = 0;
static BKNI_MutexHandle          hUdpInitMutex=NULL;              /*!< Mutex to synchronize when each Udp session is initialized*/


#ifdef NEXUS_HAS_VIDEO_ENCODER
static unsigned gCookie = 0;
extern int parseTranscodeOptions( IpStreamerOpenSettings *openSettings, IpStreamerConfig *cfg);
extern int adjustEncoderSettings(IpStreamerCtx *ipStreamerCtx, IpStreamerConfig *ipStreamerCfg);
void adjustEncoderSettingsPlaylistChange(IpStreamerCtx *ipStreamerCtx, IpStreamerConfig *ipStreamerCfg);
#endif

#define HTTP_CONTENT_TYPE_HLS_PLAYLISTS "application/x-mpegURL"
#define HTTP_CONTENT_TYPE_MPEG_DASH_MPD "application/dash+xml"
#define HTTP_CONTENT_TYPE_HTML_FILE "text/html"
#define HTTP_CONTENT_TYPE_MPEG_SEGMENT "video/mpeg"

void
signalHandler(int signal)
{
    BDBG_MSG(("Got SIGINT (%d): Cleaning up!!!", signal));
    gGotSigInt = 1;
#ifdef B_USE_HTTP_KEEPALIVE
    if (ipStreamerDestroySession)
        BKNI_SetEvent(ipStreamerDestroySession);
#endif
}

static void
usage(void)
{
    printf("Usage: ip_streamer [-v] [-d] [-s] [-i <interface name>] [-n #threads] [-m media directory path] [-f] [-h] \n");
    printf("options are:\n");
    printf(" -p                  # server port (default: 5000)\n");
    printf(" -d                  # dtcp-ip server port (default: 8000)\n");
    printf(" -K <0|1|2>          # DTCP/IP key format: test(0) | common DRM(1) | legacy production(2), default is test\n");
    printf(" -C                  # DTCP/IP: Enable content key confirmation procedure for sink device\n");
    printf(" -v                  # print stats\n");
    printf(" -s                  # run in slave mode (for VMS)\n");
    printf(" -i <interface name> # name of the interface to send multicast join on (default: eth0)\n");
    printf(" -m <dir name >      # full path of content directory (default: /data/videos)\n");
    printf(" -n <# threads>      # number of streaming threads (default: 3)\n");
    printf(" -f                  # dont use frontends for live streaming (default live streaming is on)\n");
    printf(" -u <uri>            # uri for RTP/UDP streaming (e.g. rtp://192.168.1.2:8080/data/videos/AbsMpeg2HD.mpg\n");
    printf(" -c <configFile.xml> # use configFile options for multiple udp sessions\n");
    printf(" -h                  # prints this usage\n");
    printf("\n");
}

static void
setDefaultGlobalConfig(
    IpStreamerGlobalCfg *ipStreamerGlobalCfg
    )
{
    int i;
    memset(ipStreamerGlobalCfg, 0, sizeof(IpStreamerGlobalCfg));

    ipStreamerGlobalCfg->printStats = false;
    ipStreamerGlobalCfg->listeningPort = IP_STREAMER_DEFAULT_SERVER_PORT;
    ipStreamerGlobalCfg->dtcpAkePort = IP_STREAMER_DEFAULT_DTCP_IP_PORT;
    ipStreamerGlobalCfg->dtcpKeyFormat = 0;
    ipStreamerGlobalCfg->ckc_check = true;
    ipStreamerGlobalCfg->accelSocket = false;
    ipStreamerGlobalCfg->slaveMode = false;
    strncpy(ipStreamerGlobalCfg->rootDir, DEFAULT_CONTENT_DIR, sizeof(ipStreamerGlobalCfg->rootDir));
    strncpy(ipStreamerGlobalCfg->timeshiftDirPath, DEFAULT_TIMESHIFT_DIR, sizeof(ipStreamerGlobalCfg->timeshiftDirPath));
    ipStreamerGlobalCfg->timeshiftBufferInterval = DEFAULT_TIMESHIFT_INTERVAL;
    ipStreamerGlobalCfg->maxBitRate = 20*1024*1024; /* 20 Mpbs */
    strncpy(ipStreamerGlobalCfg->interfaceName, "eth0", sizeof(ipStreamerGlobalCfg->interfaceName));
    ipStreamerGlobalCfg->numStreamingSessions = 8;
    ipStreamerGlobalCfg->disableFrontend = false;
    for(i =0; i< ipStreamerGlobalCfg->numStreamingSessions; i++)
    ipStreamerGlobalCfg->streamingCfg[i].streamingProtocol = B_PlaybackIpProtocol_eHttp;
}

extern char *B_PlaybackIp_UtilsStristr(char *str, char *subStr);
extern void B_PlaybackIp_HttpParseRespHeaderForPsi(B_PlaybackIpHandle playback_ip, NEXUS_TransportType http_content_type, char *http_hdr, B_PlaybackIpPsiInfoHandle psi);
static void
closeSocket(int streamingFd)
{
    if (streamingFd) {
        BDBG_WRN(("%s: streamingFd %d", BSTD_FUNCTION, streamingFd));
        close(streamingFd);
    }
}

static bool parseUri(char *uri, IpStreamerStreamingOutCfg *streamingCfg)
{
    char *tmp1, *tmp2, *tmp3;

    memset(streamingCfg, 0, sizeof(IpStreamerStreamingOutCfg));

    if ( (tmp1 = B_PlaybackIp_UtilsStristr(uri, "rtp:")) != NULL) {
        /* rtp protocol is being used, parse uri further */
        tmp1 += strlen("rtp://");
        streamingCfg->streamingProtocol = B_PlaybackIpProtocol_eRtp;
    }
    else if ( (tmp1 = B_PlaybackIp_UtilsStristr(uri, "udp:")) != NULL) {
        /* udp protocol is being used, parse uri further */
        tmp1 += strlen("udp://");
        streamingCfg->streamingProtocol = B_PlaybackIpProtocol_eUdp;
    }
    else {
        BDBG_ERR(("incorrect usage of -u option (uri %s), use it to specify either RTP or UDP streaming", uri));
        return false;
    }

    /* now parse rest of the URL string: take out the server string from the url */
    tmp2 = strstr(tmp1, "/");
    if (tmp2) {
        streamingCfg->streamingIpAddress = strndup(tmp1, (tmp2 - tmp1));

        /* Check to see if a port value was specified */
        tmp3 = strstr(streamingCfg->streamingIpAddress, ":");
        if (tmp3) {
            tmp3[0] = '\0'; /* this null terminates the server name string */
            tmp3++;
            streamingCfg->streamingPort = strtoul(tmp3, (char **) NULL, 10);
            if (streamingCfg->streamingPort == 0) {
                BDBG_ERR(("incorrect usage of -u option (uri %s), need to specify port # for RTP or UDP streaming", uri));
                goto error;
            }
        }
        else  {
            BDBG_ERR(("incorrect usage of -u option (uri %s), need to specify port # for RTP or UDP streaming", uri));
            goto error;
        }

        /* now get the uri */
        streamingCfg->url = strdup(tmp2);
        BDBG_WRN(("streaming protocol: %s, server:port %s:%d url %s", streamingCfg->streamingProtocol == B_PlaybackIpProtocol_eRtp ? "RTP":"UDP", streamingCfg->streamingIpAddress, streamingCfg->streamingPort, streamingCfg->url));

        return true;
    }
    else {
        BDBG_ERR(("incorrect usage of -u option (uri %s), need to specify file name for RTP or UDP streaming", uri));
    }
error:
    if (streamingCfg->streamingIpAddress)
        free(streamingCfg->streamingIpAddress);
    if (streamingCfg->url)
        free(streamingCfg->url);
    memset(streamingCfg, 0, sizeof(IpStreamerStreamingOutCfg));
    return false;
}
#define XML_TAG_IPSTREAMER "ipStreamerConfig"

#define XML_TAG_GLOBAL     "globalConfig"
#define XML_TAG_INTERFACENAME "interfaceName"
#define XML_TAG_PORTNUM     "portNum"
#define XML_TAG_DTCPPORTNUM     "dtcpPortNum"
#define XML_TAG_SLAVEMODE   "enableSlaveMode"
#define XML_TAG_NUMTHREADS  "numThreads"
#define XML_TAG_VERBOSE     "enableVerbose"
#define XML_TAG_FRONTENDS   "disableFrontends"
#define XML_TAG_VIDEOSDIR   "videosDirectory"


#define XML_TAG_UDP        "udp"
#define XML_ATT_NUMSTREAMS "numStreams"
#define XML_TAG_STREAM     "stream"
#define XML_ATT_ID         "id"
#define XML_TAG_URI        "uri"

int ReadIpStreamerOptionsFromConfig(FILE *fp, IpStreamerGlobalCfg *ipStreamerGlobalCfg)
{

    int lSize,err,rc=BERR_SUCCESS;
    char *buffer;
    int urlIndex;
    char buff[256];


    B_PlaybackIp_XmlElement   xmlElemRoot                   = NULL;
    B_PlaybackIp_XmlElement   xmlElemIpStreamer                    = NULL;
    B_PlaybackIp_XmlElement   xmlElemGlobal                    = NULL;
    B_PlaybackIp_XmlElement   xmlElemGlobalChild                    = NULL;

    B_PlaybackIp_XmlElement   xmlElemUDP                    = NULL;
    B_PlaybackIp_XmlElement   xmlElemStream                    = NULL;
    B_PlaybackIp_XmlElement   xmlElemUri                    = NULL;

    BSTD_UNUSED( ipStreamerGlobalCfg );

    rc = BKNI_Init();
    if(rc!=BERR_SUCCESS) {goto error;}

    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    if (lSize < 0) {
        BDBG_ERR(("%s: ipStreamConfig.xml  is zero size, please create a correct it using ipStreamerConfig.xml as an example", BSTD_FUNCTION));
        goto error;
    }
    rewind( fp );

    /* allocate memory for entire content */

    BDBG_MSG(("xml string length: %d", lSize+1));
    buffer = (char*) BKNI_Malloc(lSize+1 );
    if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

    memset(buffer, 0, lSize + 1);
    /* copy the file into the buffer */
    if( 1!=fread( buffer , lSize, 1 , fp) )
      fclose(fp),BKNI_Free(buffer),fputs("entire read fails",stderr),exit(1);

    buffer[lSize] = '\0'; /* for coverity */
    /* printing xml string */
    BDBG_MSG(("xml string:\n%s", buffer));

    /* Parse the XML string and create a tree of XML objects.  This tree will
       need to be freed by calling B_PlaybackIp_Xml_Destroy( xmlElemRoot */
    xmlElemRoot = B_PlaybackIp_Xml_Create(buffer);

#if 0
<ipStreamerConfig>
  <globalConfig>
    <interfaceName>eth0</interfaceName>
    <portNum>80</portNum>
    <enableFrontends>true</enableFrontends>
    <numThreads>3</numThreads>
    <enableVerbose>false</enableVerbose>
    <videosDirectory>/data/videos</videosDirectory>
  </globalConfig>
  <udp numStreams="4">
    <stream id="1">
     <uri>rtp://192.168.1.2:8080/data/videos/AbsMpeg2HD.mpg</uri>
    </stream>
    <stream id="2">
     <uri>rtp://192.168.1.2:8080/data/videos/AbsMpeg2HD.mpg</uri>
    </stream>
    <stream id="3">
     <uri>rtp://192.168.1.2:8080/data/videos/AbsMpeg2HD.mpg</uri>
    </stream>
    <stream id="4">
     <uri>rtp://192.168.1.2:8080/data/videos/AbsMpeg2HD.mpg</uri>
    </stream>
    <stream id="5">
     <uri>rtp://192.168.1.2:8080/data/videos/AbsMpeg2HD.mpg</uri>
    </stream>
    <stream id="6">
     <uri>rtp://192.168.1.2:8080/data/videos/AbsMpeg2HD.mpg</uri>
    </stream>
    <stream id="7">
     <uri>rtp://192.168.1.2:8080/data/videos/AbsMpeg2HD.mpg</uri>
    </stream>
    <stream id="8">
     <uri>rtp://192.168.1.2:8080/data/videos/AbsMpeg2HD.mpg</uri>
    </stream>
  </udp>
</ipStreamerConfig

#endif
    xmlElemIpStreamer = B_PlaybackIp_XmlElem_FindChild(xmlElemRoot , XML_TAG_IPSTREAMER);

    /* Global configurations*/
    xmlElemGlobal = B_PlaybackIp_XmlElem_FindChild(xmlElemIpStreamer , XML_TAG_GLOBAL);

    xmlElemGlobalChild = B_PlaybackIp_XmlElem_FindChild(xmlElemGlobal , XML_TAG_INTERFACENAME);
    if(xmlElemGlobalChild)
    {
        strncpy(ipStreamerGlobalCfg->interfaceName,B_PlaybackIp_XmlElem_ChildData(xmlElemGlobalChild) ,sizeof(ipStreamerGlobalCfg->interfaceName));
        ipStreamerGlobalCfg->interfaceName[sizeof(ipStreamerGlobalCfg->interfaceName)-1] ='\0';
        BDBG_MSG(("interfaceName %s",ipStreamerGlobalCfg->interfaceName));
        xmlElemGlobalChild = NULL;
    }

    xmlElemGlobalChild = B_PlaybackIp_XmlElem_FindChild(xmlElemGlobal , XML_TAG_FRONTENDS);
    if(xmlElemGlobalChild)
    {
        ipStreamerGlobalCfg->disableFrontend =B_PlaybackIp_XmlElem_ChildDataBoolean(xmlElemGlobalChild,false);

        BDBG_MSG(("disableFrontends:%d",ipStreamerGlobalCfg->disableFrontend));
        xmlElemGlobalChild = NULL;
    }

    xmlElemGlobalChild = B_PlaybackIp_XmlElem_FindChild(xmlElemGlobal , XML_TAG_VERBOSE);
    if(xmlElemGlobalChild)
    {
        ipStreamerGlobalCfg->printStats = B_PlaybackIp_XmlElem_ChildDataBoolean(xmlElemGlobalChild,false);

        BDBG_MSG(("verbose %d",ipStreamerGlobalCfg->printStats));
        xmlElemGlobalChild = NULL;
    }
    xmlElemGlobalChild = B_PlaybackIp_XmlElem_FindChild(xmlElemGlobal , XML_TAG_VIDEOSDIR);
    if(xmlElemGlobalChild)
    {

        strncpy(ipStreamerGlobalCfg->rootDir,B_PlaybackIp_XmlElem_ChildData(xmlElemGlobalChild) ,sizeof(ipStreamerGlobalCfg->rootDir));
        ipStreamerGlobalCfg->rootDir[sizeof(ipStreamerGlobalCfg->rootDir)-1] ='\0';
        BDBG_MSG(("video dir %s",ipStreamerGlobalCfg->rootDir));
        xmlElemGlobalChild = NULL;
    }


        BDBG_MSG(("interfaceName %s",ipStreamerGlobalCfg->interfaceName));
        BDBG_MSG(("port %d",ipStreamerGlobalCfg->listeningPort));
        BDBG_MSG(("disableFrontends: %d",ipStreamerGlobalCfg->disableFrontend));
        BDBG_MSG(("streaming sessions %d",ipStreamerGlobalCfg->numStreamingSessions));

        BDBG_MSG(("verbose %d",ipStreamerGlobalCfg->printStats));
        BDBG_MSG(("video dir %s",ipStreamerGlobalCfg->rootDir));

   /* UDP configurations*/
    xmlElemUDP = B_PlaybackIp_XmlElem_FindChild(xmlElemIpStreamer , XML_TAG_UDP);
    ipStreamerGlobalCfg->numStreamingSessions = B_PlaybackIp_XmlElem_FindAttrValueInt(xmlElemUDP, XML_ATT_NUMSTREAMS,1);
    BDBG_MSG(("Num Streams:%d", ipStreamerGlobalCfg->numStreamingSessions ));

    BDBG_MSG(("Printing Streams"));
    for (urlIndex=0; urlIndex< ipStreamerGlobalCfg->numStreamingSessions; urlIndex++)
    {
        memset(buff, 0, sizeof(buff));
        /*  Now get the first/next child under the parent (if there are any). */
        xmlElemStream = B_PlaybackIp_XmlElem_FindNextChildSameTag(xmlElemUDP, xmlElemStream, XML_TAG_STREAM);
        if ( !xmlElemStream) break;    /* No more children... all done. */
        BDBG_MSG(("Stream Id:%ld", B_PlaybackIp_XmlElem_FindAttrValueInt(xmlElemStream, XML_ATT_ID,1)));
        xmlElemUri = B_PlaybackIp_XmlElem_FindChild(xmlElemStream , XML_TAG_URI);
        strncpy(buff,B_PlaybackIp_XmlElem_ChildData(xmlElemUri) ,sizeof(buff));
        buff[sizeof(buff)-1] ='\0';
        BDBG_MSG(("uri %s",buff));
        err = parseUri(buff, &ipStreamerGlobalCfg->streamingCfg[urlIndex]);
        if ( err != true)
        {
            return 1;
        }
        else
        {
            BDBG_MSG(("\n ReadIpStreamerOptionsFromConfig: streamingCfg: IP: %s, Port: %d, Protocol: %d, url: %s",
                                    ipStreamerGlobalCfg->streamingCfg[urlIndex].streamingIpAddress,
                                    ipStreamerGlobalCfg->streamingCfg[urlIndex].streamingPort,
                                    ipStreamerGlobalCfg->streamingCfg[urlIndex].streamingProtocol,
                                    ipStreamerGlobalCfg->streamingCfg[urlIndex].url));
        }

    }


#if 0
    /* Todo HTTP configuration */
    xmlElemGlobalChild = B_PlaybackIp_XmlElem_FindChild(xmlElemGlobal , XML_TAG_PORTNUM);
    if(xmlElemGlobalChild)
    {
        ipStreamerGlobalCfg->listeningPort =B_PlaybackIp_XmlElem_ChildDataUnsigned(xmlElemGlobalChild,80);

        BDBG_MSG(("port %d",ipStreamerGlobalCfg->listeningPort));
        xmlElemGlobalChild = NULL;
    }

#endif

/*  shutting down */
    BKNI_Free(buffer);
error:
    fclose(fp);
    BDBG_ERR(("rc value %d ", rc));
    return( rc );
}
static int
parserIpStreamerOptions(
    int argc,
    char *argv[],
    IpStreamerGlobalCfg *ipStreamerGlobalCfg
    )
{
    int ch, i;
    char *streamingUri = NULL;
    int url_index = 0;
    bool ret_status;
    FILE *config_fp;

    struct in_addr sin_temp_addr;
    bool numStreamSessionEnabled = false;

    /* set default global configuration */
    setDefaultGlobalConfig(ipStreamerGlobalCfg);

    /* # of streaming threads, cpu affinity, */
    while ( (ch = getopt(argc, argv, "p:sd:K:vi:m:n:fu:ChHc:")) != -1) {
        switch (ch) {
        case 'p':
            ipStreamerGlobalCfg->listeningPort = atoi(optarg);
            break;
        case 'd':
            ipStreamerGlobalCfg->dtcpAkePort = atoi(optarg);
            break;
        case 'K':
            ipStreamerGlobalCfg->dtcpKeyFormat = atoi(optarg);
            break;
        case 'C':
            ipStreamerGlobalCfg->ckc_check = true;
            break;
        case 'v':
            ipStreamerGlobalCfg->printStats = true;
            break;
        case 's':
            ipStreamerGlobalCfg->slaveMode = true;
            break;
        case 'i':
            memcpy(ipStreamerGlobalCfg->interfaceName, optarg, sizeof(ipStreamerGlobalCfg->interfaceName));
            break;
        case 'm':
            memcpy(ipStreamerGlobalCfg->rootDir, optarg, sizeof(ipStreamerGlobalCfg->rootDir));
            break;
        case 'n':
            ipStreamerGlobalCfg->numStreamingSessions = atoi(optarg);
            if (ipStreamerGlobalCfg->numStreamingSessions > IP_STREAMER_MAX_THREADS) {
                BDBG_ERR(("ERROR: Max Streaming Sessions supported %d, asked %d", IP_STREAMER_MAX_THREADS, ipStreamerGlobalCfg->numStreamingSessions));
                goto error;
            }
            numStreamSessionEnabled = true;
            break;
        case 'f':
            ipStreamerGlobalCfg->disableFrontend = true;
            break;
        case 'u':
            streamingUri = optarg;
            if (streamingUri) {
                ret_status = parseUri(streamingUri, &ipStreamerGlobalCfg->streamingCfg[url_index]);
                if ( ret_status != true)
                {
                    goto error;
                }
                else
                {
                    BDBG_WRN(("\n parserIpStreamerOptions: streamingCfg: IP: %s, Port: %d, Protocol: %d, url: %s",
                                            ipStreamerGlobalCfg->streamingCfg[url_index].streamingIpAddress,
                                            ipStreamerGlobalCfg->streamingCfg[url_index].streamingPort,
                                            ipStreamerGlobalCfg->streamingCfg[url_index].streamingProtocol,
                                            ipStreamerGlobalCfg->streamingCfg[url_index].url));
                    url_index++;
                }
            }
            break;
        case 'H':
            ipStreamerGlobalCfg->optimizedHlsStreaming = true;
            break;
        case 'c':
            {

                /* copy xml file to char string */
                config_fp = fopen ( optarg , "rb" );
                if( !config_fp ) perror(optarg),exit(1);


                if(ReadIpStreamerOptionsFromConfig(config_fp, ipStreamerGlobalCfg))
                {
                    BDBG_ERR(("ERROR: Incorrect Config"));
                    goto error;
                }
                numStreamSessionEnabled = true;
                /*return 0; */
            }
            break;
        case 'h':
        default:
            goto error;
        }
    }

    if (streamingUri) {
        if (parseUri(streamingUri, &ipStreamerGlobalCfg->streamingCfg[0]) != true) {
            goto error;
        }
    }

    for(i=0; i<ipStreamerGlobalCfg->numStreamingSessions; i++)
    {
        if((!numStreamSessionEnabled)&&
            ((ipStreamerGlobalCfg->streamingCfg[i].streamingProtocol == B_PlaybackIpProtocol_eRtp) ||(ipStreamerGlobalCfg->streamingCfg[i].streamingProtocol == B_PlaybackIpProtocol_eUdp)))
            {
            /** if number of streamin session is not provided by user
             *  then restrict it to one for udp and RTP , since default
             *  value is 8 *   */
            ipStreamerGlobalCfg->numStreamingSessions = 1;

        }

        if((ipStreamerGlobalCfg->streamingCfg[i].streamingProtocol == B_PlaybackIpProtocol_eRtp) ||(ipStreamerGlobalCfg->streamingCfg[i].streamingProtocol == B_PlaybackIpProtocol_eUdp)) {
            if (inet_aton(ipStreamerGlobalCfg->streamingCfg[i].streamingIpAddress, &sin_temp_addr) == 0)
            {
                BDBG_MSG(("inet_aton failed: invalid address %s", ipStreamerGlobalCfg->streamingCfg[i].streamingIpAddress));
                goto error;
            }

            ipStreamerGlobalCfg->multicastEnable = IN_MULTICAST(ntohl(sin_temp_addr.s_addr));
        }
    }
    return 0;

error:
     usage();
     exit(1);
}
#ifdef LIVEMEDIA_SUPPORT
#if LIVEMEDIA_2013_03_07
/* CAD/BRCM 20121031 - Added RTSPServer support */
static void
RTSPServerThreadExit(
   void
   )
{
    BDBG_MSG(("%s.", BSTD_FUNCTION ));

    /*pthread_exit(NULL);*/
}
/* CAD/BRCM 20121031 - Added RTSPServer support */
static void *
RTSPServerThread(
   void *data
   )
{
    IpStreamerGlobalCtx *ipStreamerGlobalCtx = (IpStreamerGlobalCtx *)data;

    atexit(RTSPServerThreadExit);

    BDBG_ERR(("############## RTSP Server Starting using Port %d ###############", DEFAULT_RTSP_PORT ));
    RTSPServerStart( DEFAULT_RTSP_PORT, (void*) ipStreamerGlobalCtx, RTSPServerCallback ); /* never returns from here */

    pthread_exit(NULL);
}

/* CAD/BRCM 20121114 - Added RTSPServer support */
static int
IpStreamerRtspCloseSocket(
   int streamingFd
   )
{
    if (streamingFd) {
        BDBG_MSG(("%s: fd", BSTD_FUNCTION, streamingFd));
        closeSocket(streamingFd);
    }

    return 0;
}

static int
IpStreamerRtspCloseSession(
   IpStreamerCtx * ipStreamerCtx,
   int streamingFd
   )
{
    B_IpStreamer_SessionClose(ipStreamerCtx);
    IpStreamerRtspCloseSocket(streamingFd);

    return 0;
}

static int
IpStreamerRtspCloseAll(
   IpStreamerCtx * ipStreamerCtx,
   int streamingFd
   )
{
    B_IpStreamer_SessionStop(ipStreamerCtx);
    IpStreamerRtspCloseSession( ipStreamerCtx, streamingFd );

    return 0;
}

/*
    Create a new socket descriptor and bind it to the global ethernet interface (eth0 or eth1 or eth2)
*/
static int
NewSocketAndBindToIf(
    const char * interfaceName
    )
{
    int rc=0;
    int streamingFd = 0;
    struct ifreq ifr;

    if ( (streamingFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        /* Socket Create Error */
        perror("Socket Open Err");
        return streamingFd;
    }
    BDBG_MSG(("%s: new DGRAM socket to stream content on %d using (%s)", BSTD_FUNCTION, streamingFd, interfaceName ));
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interfaceName, sizeof(ifr.ifr_name)-1);
    if ( (rc=setsockopt(streamingFd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr) )) < 0 ) {
        perror("SO_BINDTODEVICE");
        close(streamingFd);
        return rc;
    }

    return streamingFd;
}

static char *
getRtspCallbackStringName(
   enRTSP_CMD command
   )
{
    return (command == RTSP_CMD_SATFILE_CREATE_NAME)?"CREATE_NAME":
    (command == RTSP_CMD_SATFILE_OPEN)?"OPEN":
    (command == RTSP_CMD_SATFILE_SET_SERVER_PORT)?"SET_SERVER_PORT":
    (command == RTSP_CMD_SATFILE_SET_CLIENT_PORT)?"SET_CLIENT_PORT":
    (command == RTSP_CMD_SATFILE_PLAY)?"PLAY":
    (command == RTSP_CMD_SATFILE_PAUSE)?"PAUSE":
    (command == RTSP_CMD_SATFILE_JOIN)?"JOIN":
    (command == RTSP_CMD_SATFILE_CLOSE)?"CLOSE":"UNKNOWN";
}

/*
    Function gets called from LiveMedia when RTSP Server request arrives for live satellite streaming
*/
long int
RTSPServerCallback(
   void *data,
   enRTSP_CMD command,
   char * requestUrl
   )
{
    long int rc=0;
    B_PlaybackIpPsiInfo psi;
    IpStreamerCtx *ipStreamerCtx = NULL;
    IpStreamerGlobalCtx *ipStreamerGlobalCtx = (IpStreamerGlobalCtx *)data;
    int requestUrlLen = 0;

    BDBG_MSG(("%s - cmd %s", BSTD_FUNCTION, getRtspCallbackStringName( command ) ));

    /*
     * Main flow (in a while loop):
     * Wait on new request from client (client can be remote (streaming request)
     * Accept HTTP Request from client and parse the request type:
     *  -determine src type: IP, QAM, Disk, etc.
     *  -determine dst type: IP, and/or Record
     * Call SessionOpen() to:
     *  -select a free src of requested srcType
     * Use SessionAcquitePsiInfo to determine AV PIDS & Codecs (internally starts frontends so that it can receive PSI tables & then stops them)
     * Send HTTP Response back to the client
     * Call SessionStart() to:
     *  -select free dst: IP and/or Record
     *  -start dst(s)
     *  -start src
     * Call SessionStatus() in a loop to check client connection status or signal from local app to exit
     * Call SessionStop() to
     *  -stop dst(s)
     *  -stop dst(s)
     * Call SessionClose() to:
     *  -close dst(s)
     *  -close src
     * go back & wait on new request from client
     */

    memset(&psi, 0, sizeof(psi) );

    switch (command)
    {
    case RTSP_CMD_SATFILE_CREATE_NAME:
        {
#if NEXUS_HAS_FRONTEND
            IpStreamerOpenSettings openSettings;
            char * newfilename = malloc(SATFILE_MAX_LEN);

            /* if the malloc worked */
            if (newfilename != NULL) {
                int len=strlen(requestUrl);
                IpStreamerConfig cfg;
                /* now pad the requestUrl w/ & (ampersand) char so the parseToken() api works at the end of the string */
                requestUrl[len] = '&';
                requestUrl[len+1] = '\0';
                cfg.streamingCfg.streamingProtocol = B_PlaybackIpProtocol_eRtp;
                if ( parseSatIpOptionsUrl ( &cfg, requestUrl ) == 0)
                {
                    struct in_addr destinationAddr;
                    memset(&openSettings, 0, sizeof(openSettings));

                    if (cfg.pidListCount > 0) {
                        openSettings.skipPsiAcquisition = true;
                    }
                    /* Open the streaming session */
                    openSettings.requestUri = requestUrl;
                    openSettings.mediaProbeOnly = false;
                    openSettings.liveStreamingSession = true; /* used to tell openNexusSrc() to share tuner */
                    /* coverity[stack_use_local_overflow] */
                    /* coverity[stack_use_overflow] */
                    ipStreamerCtx = (IpStreamerCtx *)B_IpStreamer_SessionOpen(ipStreamerGlobalCtx, &openSettings);
                    if (!ipStreamerCtx) break;

                    ipStreamerCtx->cfg.streamingCfg.streamingProtocol = B_PlaybackIpProtocol_eRtp;
                    ipStreamerCtx->cfg.streamingFd = NewSocketAndBindToIf (  ipStreamerGlobalCtx->globalCfg.interfaceName );
                    /* msys=dvbs  ...                cfg->satMode = NEXUS_FrontendSatelliteMode_eDvb;
                       msys=dvbs2 ... mtype=qpsk ... cfg->satMode = NEXUS_FrontendSatelliteMode_eQpskLdpc;
                       msys=dvbs2 ... mtype=8psk ... cfg->satMode = NEXUS_FrontendSatelliteMode_e8pskLdpc;
                    */
                    inet_pton(AF_INET, cfg.srcIpAddress, &destinationAddr );
                    sprintf(newfilename, SATFILE_FORMAT, (long unsigned int) ipStreamerCtx );
                    rc = (unsigned long int) newfilename;
                }
            }
#endif /* NEXUS_HAS_FRONTEND */
        }
        break;
    case RTSP_CMD_SATFILE_OPEN:
        {
            unsigned long int streamingFd = 0;
            IpStreamerConfig cfg;
            FILE * dummyFp = NULL;

            memset(&cfg, 0, sizeof(cfg));
            requestUrlLen = strlen(requestUrl);

            /* brcmsat.70a258.ts/59910 */
            parseSatIpOptionsFromFilename( &cfg, requestUrl );
            ipStreamerCtx = (IpStreamerCtx*) cfg.ipStreamerCtx;

            if (!ipStreamerCtx)
            {
                BDBG_ERR(("ERROR: Failed to Open IP Streaming Context"));
                IpStreamerRtspCloseSocket ( streamingFd );
                break;
            }

            streamingFd = ipStreamerCtx->cfg.streamingFd;
            if (cfg.srcPort > 0) {
                ipStreamerCtx->cfg.streamingCfg.streamingPort = cfg.srcPort; // also sets server_port
            }
            BDBG_MSG(("Open the streaming session using socket %d; srcPort (%u); pidListCount (%u)", streamingFd, ipStreamerCtx->cfg.srcPort, cfg.pidListCount ));

            if /*(!psi.psiValid) */ (cfg.pidListCount == 0)  {
                BDBG_ERR(("calling B_IpStreamer_SessionAcquirePsiInfo; sock %d", streamingFd ));
                /* psi info wasn't sent from the client, so obtain PSI info from requested source */
                rc = B_IpStreamer_SessionAcquirePsiInfo(ipStreamerCtx, &psi);
                if (rc)
                {
                    BDBG_ERR(("ERROR:"));
                    BDBG_ERR(("ERROR: Failed to acquire PSI Info 1"));
                    BDBG_ERR(("ERROR:"));
                    IpStreamerRtspCloseSession (ipStreamerCtx, streamingFd);
                    break;
                }
            }

            dummyFp = fopen("/dev/null", "w"); /* this will be closed in LiveMedia */

            rc = (unsigned long int) dummyFp;
        }

        break;
    case RTSP_CMD_SATFILE_SET_SERVER_PORT:
        {
            unsigned long int ipPort=0, ipSocket=0;
            unsigned long int Ctx=0;
            char * pos = NULL;

            if ( ipStreamerGlobalCtx == NULL)
            {
                BDBG_ERR(("%s - ipStreamerGlobalCtx can't be null", BSTD_FUNCTION ));
                break;
            }

            /* brcmsat.70a258/6970 */
            sscanf ( requestUrl, "brcmsat.%lx", &Ctx );
            pos = strstr(requestUrl, "/" );
            if (pos) {
                pos++; /* advance past the slash */
                sscanf ( pos, "%lu", &ipPort );

                pos = strstr(pos, "/" );
                if (pos) {
                    pos++; /* advance past the slash */
                    sscanf ( pos, "%lu", &ipSocket );
                }
            }
            BDBG_MSG(("%s - Ctx (%x); ipPort (%lu); ipSocket (%lu)", BSTD_FUNCTION, Ctx, ipPort, ipSocket ));

            ipStreamerCtx = (IpStreamerCtx*) Ctx;
            if ( ipStreamerCtx == NULL)
            {
                BDBG_ERR(("%s - ipStreamerCtx can't be null", BSTD_FUNCTION ));
                break;
            }

            if (ipPort > 0) {
                ipStreamerCtx->cfg.srcPort = ipPort;
            }

            if (ipSocket > 0) {
                ipStreamerCtx->cfg.streamingFd = ipSocket;
            }
        }
        break;
    case RTSP_CMD_SATFILE_SET_CLIENT_PORT:
        {
            unsigned long int ipPort=0;
            unsigned long int Ctx=0;
            char * pos = NULL;

            /* brcmsat.70a258/28345*/
            sscanf ( requestUrl, "brcmsat.%lx", &Ctx );
            pos = strstr(requestUrl, "/" );
            if (pos) {
                pos++; /* advance past the slash */
                sscanf ( pos, "%lu", &ipPort );
            }
            BDBG_MSG(("%s - Ctx (%x); ipPort (%lu)", BSTD_FUNCTION, Ctx, ipPort ));

            ipStreamerCtx = (IpStreamerCtx*) Ctx;
            if ( ipStreamerCtx == NULL)
            {
                BDBG_ERR(("%s - ipStreamerCtx can't be null", BSTD_FUNCTION ));
                break;
            }

            if (ipPort > 0) {
                ipStreamerCtx->cfg.srcPort = ipPort;
                ipStreamerCtx->cfg.streamingCfg.streamingPort = ipPort;
            }
        }
        break;
    case RTSP_CMD_SATFILE_PLAY:
        {
            IpStreamerOpenSettings openSettings;
            IpStreamerConfig cfg;

            if ( ipStreamerGlobalCtx == NULL)
            {
                BDBG_ERR(("%s - ipStreamerGlobalCtx can't be null", BSTD_FUNCTION ));
                break;
            }
            /* once client sends HTTP Get request, incoming request should give us info on what type of src & dst to use */
            memset(&openSettings, 0, sizeof(openSettings));
            memset(&cfg, 0, sizeof(cfg));
            requestUrlLen = strlen(requestUrl);

            parseSatIpOptionsFromFilename( &cfg, requestUrl );

            ipStreamerCtx = (IpStreamerCtx*) cfg.ipStreamerCtx;
            if ( ipStreamerCtx == NULL)
            {
                BDBG_ERR(("%s - ipStreamerCtx can't be null", BSTD_FUNCTION ));
                break;
            }

            ipStreamerCtx->PlayCount++;


            if ( ipStreamerCtx->ipStreamingInProgress && ipStreamerCtx->PlayCount>1 ) {
                break;
            }

            if ((ipStreamerCtx->ipDst) && (ipStreamerCtx->ipDst->liveStreamingHandle) ) {
                B_PlaybackIpLiveStreamingSettings settings;
                memset(&settings, 0, sizeof(settings));
                settings.streamingFd = ipStreamerCtx->cfg.streamingFd;
                B_PlaybackIp_LiveStreamingSetSettings(ipStreamerCtx->ipDst->liveStreamingHandle, &settings);
            }
            /* start streaming session ... starts a new thread that monitors gExitThread */
            rc = B_IpStreamer_SessionStart((void *)ipStreamerCtx, &psi);
            if (rc < 0)
            {
                BDBG_ERR(("ERROR: Failed to start IP Streamer Session"));
                IpStreamerRtspCloseSession (ipStreamerCtx, cfg.streamingFd );
                break;
            }
        }
        break;
    case RTSP_CMD_SATFILE_PAUSE:
        {
        }
        break;
    case RTSP_CMD_SATFILE_JOIN:
        {
            char * pos = NULL;
            IpStreamerCtx * Ctx=0;
            IpStreamerCtx * origIpStreamerCtx = NULL;

            /* brcmsat.70a258/brcmsat.70c567 */
            sscanf ( requestUrl, "brcmsat.%lx", (long unsigned int*) &Ctx );
            pos = strstr(requestUrl, "/" );
            if (pos) {
                pos++; /* advance past the slash */
                sscanf ( pos, "brcmsat.%lx", (unsigned long int *) &origIpStreamerCtx );
            }

            ipStreamerCtx = (IpStreamerCtx*) Ctx;
            if ( ipStreamerCtx == NULL)
            {
                BDBG_ERR(("%s - ipStreamerCtx can't be null", BSTD_FUNCTION ));
                break;
            }

            if (origIpStreamerCtx != NULL) {
                ipStreamerCtx->cfg.liveStreamingCtx = (long unsigned int*) origIpStreamerCtx;
                ipStreamerCtx->cfg.streamingFd = origIpStreamerCtx->cfg.streamingFd;
                ipStreamerCtx->cfg.srcPort = origIpStreamerCtx->cfg.srcPort;
            }
        }
        break;
    case RTSP_CMD_SATFILE_CLOSE:
        {
            IpStreamerConfig cfg;

            parseSatIpOptionsFromFilename( &cfg, requestUrl );

            ipStreamerCtx = (IpStreamerCtx*) cfg.ipStreamerCtx;

            if ( ipStreamerCtx == NULL)
            {
                BDBG_ERR(("%s - ipStreamerCtx can't be null", BSTD_FUNCTION ));
                break;
            }

            BDBG_MSG(("CLOSE the streaming session (%p) using socket %d", ipStreamerCtx, ipStreamerCtx->cfg.streamingFd ));
            IpStreamerRtspCloseAll(ipStreamerCtx, ipStreamerCtx->cfg.streamingFd );

            BDBG_MSG(("CTX %p: Current ip streaming / local recording session is closed; go back to listening for new requests", ipStreamerCtx));
        }
        break;
    }

    BDBG_MSG(("%s - cmd %s: returning 0x%x", BSTD_FUNCTION ,getRtspCallbackStringName(command) ,rc ));
    return rc;
}
#endif /*LIVEMEDIA_2013_03_07*/
#endif /* LIVEMEDIA_SUPPORT */

void *
channelMapThread(
   void *data
   )
{
    IpStreamerGlobalCfg *ipStreamerGlobalCfg = (IpStreamerGlobalCfg *)data;
    int listeningFd=-1, sockFd;
    struct sockaddr_in localAddr;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    char name[264];
    FILE *channelFileFp = NULL;
    char *buffer = NULL, *buf;
    int bytes, bytesLeft;
    int reuse_flag = 1;

    BDBG_MSG(("Starting Channel Map Thread "));
    if ((listeningFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        /* Socket Create Error */
        perror("Socket Open Err");
        goto error;
    }
    if (setsockopt(listeningFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_flag, sizeof(reuse_flag) ) < 0 ) {
        BDBG_ERR(("REUSE Socket Error"));
        goto error;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(ipStreamerGlobalCfg->listeningPort+2);
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listeningFd, (struct sockaddr *) &localAddr, sizeof(localAddr))) {
        perror("Socket Bind Err to bind to actual i/f");
        goto error;
    }
    if (listen(listeningFd, 8)) {
        perror("Socket listen Err");
        goto error;
    }
    /* Set stdin to non-blocking */
    if (fcntl(listeningFd, F_SETFL, fcntl(listeningFd, F_GETFL)|O_NONBLOCK) < 0)
        BDBG_WRN(("Failed to set non-blocking mode on listening socket"));

    snprintf(name, sizeof(name)-1, "%s%s%s", ipStreamerGlobalCfg->rootDir, "/", "httpChannelMap.txt");
    if ((channelFileFp = fopen(name, "r" )) == NULL) {
        BDBG_ERR(("%s: Unable to open HTTP channel map file file %s", BSTD_FUNCTION, name));
        goto error;
    }
    if ((buffer = BKNI_Malloc(8192)) == NULL) { BDBG_ERR(("BKMI_Malloc Failure at %d", __LINE__)); goto error;}

    while (!gExitThread) {
        waitForNetworkEvent(listeningFd);
        /* accept connection */
        if ((sockFd = accept(listeningFd, NULL, NULL)) < 0) {
            perror("ERROR: accept():");
            goto error;
        }

        if (getsockname(sockFd, (struct sockaddr *)&addr, &addrlen)) {
            perror("getsockname():");
            close(sockFd);
            goto error;
        }

        /* open the channel map file which contains all the streamable media file names */
        rewind(channelFileFp);
        bytesLeft = 8192;
        buf = buffer;
        while (fgets(name, sizeof(name)-1, channelFileFp)) {
            bytes = snprintf(buf, bytesLeft, "IP_HTTP 0 %s %d %s%s", inet_ntoa(addr.sin_addr), ipStreamerGlobalCfg->listeningPort, "/", name);
            buf += bytes;
            bytesLeft -= bytes;
        }
        bytes = buf - buffer;
        if (send(sockFd, (void*)buffer, bytes, MSG_NOSIGNAL) < 0) {
            BDBG_MSG(("Failed to send %d bytes of channel map data, errno %d", bytes, errno));
        }
        BDBG_MSG(("%s: send %d bytes of channel map data", BSTD_FUNCTION, bytes));
        close(sockFd);
    }

error:
    BDBG_MSG(("Exiting Channel Map Thread"));
    if (listeningFd >= 0)
        close(listeningFd);
    if (channelFileFp) fclose(channelFileFp);
    if (buffer) BKNI_Free(buffer);
    pthread_exit(NULL);
}

bool
createInfoAndNavFiles(
    IpStreamerGlobalCfg *ipStreamerGlobalCfg,
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
#define FULL_FILE_PATH_LEN 256
#define URL_LEN 264
    int rc;
    struct dirent **namelist=NULL;
    int numDirEntries, i;
    struct stat fileStats;
    char *fullFilePath=NULL;
    char *url=NULL, *name;
    IpStreamerCtx *ipStreamerCtx;
    IpStreamerOpenSettings openSettings;
    B_PlaybackIpPsiInfo *psi=NULL;
    FILE *channelFileFp = NULL;

    fullFilePath = (char*) BKNI_Malloc(FULL_FILE_PATH_LEN);
    if (fullFilePath==NULL) {
        BDBG_ERR(("Could not malloc(%u) bytes for fullFilePath ", FULL_FILE_PATH_LEN ));
        goto error;
    }
    BDBG_MSG(("%s: SUCCESSFULL malloc(%u) bytes for fullFilePath (%s)", BSTD_FUNCTION, FULL_FILE_PATH_LEN, fullFilePath ));

    url = (char*) BKNI_Malloc(URL_LEN);
    if (url==NULL) {
        BDBG_ERR(("Could not malloc(%u) bytes for url ", URL_LEN ));
        goto error;
    }
    BDBG_MSG(("%s: SUCCESSFULL malloc(%u) bytes for url (%s)", BSTD_FUNCTION, URL_LEN, url ));

    psi = (B_PlaybackIpPsiInfo *) BKNI_Malloc( sizeof(*psi) );
    if (psi == NULL) {
        BDBG_ERR(("%s: could not malloc(%u) bytes for psi", BSTD_FUNCTION, sizeof(*psi) ));
        goto error;
    }
    BDBG_MSG(("%s: SUCCESSFULL malloc(%u) bytes for psi (%p)", BSTD_FUNCTION, sizeof(*psi), (void *)psi ));

    if ( (numDirEntries = scandir(ipStreamerGlobalCfg->rootDir, &namelist, NULL, NULL)) < 0) {
        BDBG_ERR(("Failed to sacn media directory %s", ipStreamerGlobalCfg->rootDir));
        goto error;
    }

    name = url; /* use url storage */
    snprintf(name, URL_LEN-1, "%s%s%s", ipStreamerGlobalCfg->rootDir, "/", "httpChannelMap.txt");
    if ((channelFileFp = fopen(name, "w+" )) == NULL) {
        BDBG_ERR(("%s: Unable to open HTTP channel map file file %s", BSTD_FUNCTION, name));
        goto error;
    }

    for (i=0; i < numDirEntries; i++) {
        snprintf(fullFilePath, FULL_FILE_PATH_LEN-1, "%s%s%s",
                ipStreamerGlobalCfg->rootDir, "/", namelist[i]->d_name);
        if (stat(fullFilePath, &fileStats) < 0) {
            BDBG_WRN(("File stat() failed on %s, ignoring it", fullFilePath));
            continue;
        }
        if (S_ISDIR(fileStats.st_mode)) {
            BDBG_MSG(("Ignoring sub-directories (%s) from media search", fullFilePath));
            continue;
        }
        if (strstr(fullFilePath, ".m3u8") || strstr(fullFilePath, ".mpd") || strstr(fullFilePath, ".info") || strstr(fullFilePath, ".nav") || strstr(fullFilePath, ".txt")) {
            BDBG_MSG(("Ignoring nav/info files (%s) from media search for now", fullFilePath));
            continue;
        }

        /* !dir, !info, !nav, so try to get probe info on this file */

        memset(&openSettings, 0, sizeof(openSettings));
        openSettings.streamingFd = 0; /* nop */
        snprintf(url, URL_LEN-1, "/File=%s;", fullFilePath);
        openSettings.requestUri = url;
        openSettings.mediaProbeOnly = true;
        /* coverity[stack_use_local_overflow] */
        /* coverity[stack_use_overflow] */
        ipStreamerCtx = (IpStreamerCtx *)B_IpStreamer_SessionOpen(ipStreamerGlobalCtx, &openSettings);
        if (!ipStreamerCtx) {BDBG_ERR(("ERROR: Failed to Open IP Streaming Context")); continue;}

        rc = B_IpStreamer_SessionAcquirePsiInfo(ipStreamerCtx, psi);
        if (rc) {BDBG_ERR(("Failed to acquire PSI Info ")); /* TODO: dont add it to channel map */}
        B_IpStreamer_SessionClose(ipStreamerCtx);
        BDBG_MSG(("File name %s, psi valid %d", fullFilePath, psi->psiValid));
        fwrite(namelist[i]->d_name, 1, strlen(namelist[i]->d_name), channelFileFp);
        fwrite("\n", 1, 1, channelFileFp);
        free(namelist[i]);
    }
    BDBG_WRN(("Built Media Info and Nav files for media"));
error:
    if (channelFileFp) fclose(channelFileFp);
    if (namelist) free(namelist);
    if (psi!= NULL) {
        BDBG_MSG(("%s: free(%u) bytes for psi (%p)", BSTD_FUNCTION, sizeof(*psi), (void *)psi ));
        BKNI_Free ( psi );
    }
    if (url!= NULL) {
        BDBG_MSG(("%s: free(%u) bytes for url (%p)", BSTD_FUNCTION, URL_LEN, (void *)url ));
        BKNI_Free ( url );
    }
    if (fullFilePath!= NULL) {
        BDBG_MSG(("%s: free(%u) bytes for fullFilePath (%p)", BSTD_FUNCTION, FULL_FILE_PATH_LEN, (void *)fullFilePath ));
        BKNI_Free ( fullFilePath );
    }
    return true;
}

int ipStreamerSendHttpResponse(int streamingFd, unsigned cookie, int httpStatusCode, char *contentType)
{
    char header[1024];
    int len=0;

    memset(header, 0, sizeof(header));
    len = snprintf(header, sizeof(header)-1,
            "HTTP/1.1 %d File Not Found\r\n"
            "Content-Type: %s\r\n"
            "SERVER: Broadcom Test HLS Server\r\n"
            "Accept-Ranges: bytes\r\n"
            "Set-Cookie: id=%u\r\n"
            ,
            httpStatusCode,
            contentType,
            cookie
            );
    /* send out HTTP response */
    if (write(streamingFd, header, len) != len) {
        BDBG_ERR(("Failed to write HTTP Response of %d bytes", len));
        perror("write(): ");
        return -1;
    }
    BDBG_MSG(("%s: Sending HTTP response: %s", BSTD_FUNCTION, header));
    return 0;
}

static void
ipStreamerEventCallback(void *appCtx, B_PlaybackIpEventIds eventId)
{
    IpStreamerCtx *ipStreamerCtx = (IpStreamerCtx *)appCtx;
    BDBG_MSG(("%s: Got EventId %d from IP Streamer for appCtx:streamingFd %p:%d", BSTD_FUNCTION, eventId, appCtx, ipStreamerCtx->cfg.streamingFd));
    ipStreamerCtx->eventId = eventId;
    BKNI_AcquireMutex(ipStreamerCtx->lock);
#ifndef B_USE_HTTP_KEEPALIVE
    /* this callback is invoked by the live streaming thread inside the playback ip library */
    /* we check here if we dont yet have a penidng request for next segment from the client */
    /* if that is the case, then we just update our state to indicate that this ipStreamerCtx is now waiting for next request. */
    /* Otherwise, client has already requested us a segment, so we will wake up the main thread working on this Ctx */
    if (!ipStreamerCtx->pendingReqForNextSegment)
    {
        switch(eventId) {
            case B_PlaybackIpEvent_eServerEndofSegmentReached:
                BDBG_MSG(("%s: endOfSegment event: appCtx %p: delaying wakeup event to hls thread", BSTD_FUNCTION, appCtx));
                ipStreamerCtx->waitingForNextRequest = true;
                ipStreamerCtx->hlsNextSegmentNum++;
                ipStreamerCtx->cfg.streamingFd = 0; /* clear it as socket is already closed by IP lib when it closed the segment */
                /* return from here, two things can happen */
                /* -1: another main thread receives the request for the next segment and will signal this main thread to continue */
                /* -2: live streaming thread will timeout and send either event listed below */
                break;
            case B_PlaybackIpEvent_eServerErrorStreaming:
                /* ran into some error while streaming: possibly write had failed due to client doing channel change or didn't liking our segment */
                BDBG_MSG(("%s: ServerErrorStreaming event: appCtx %p: delaying wakeup event to hls thread", BSTD_FUNCTION, appCtx));
                ipStreamerCtx->resetTranscodePipe = true;
                ipStreamerCtx->waitingForNextRequest = true;
                closeSocket(ipStreamerCtx->cfg.streamingFd);
                ipStreamerCtx->cfg.streamingFd = 0; /* clear it as socket is closed now */
                break;
                /* Note: the following events shouldn't really come as ip lib is no longer sending them for hls streaming case */
            case B_PlaybackIpEvent_eServerStartStreamingTimedout:
            case B_PlaybackIpEvent_eServerEndofStreamReached:
            default:
                if (ipStreamerCtx->statusEvent)
                    BKNI_SetEvent(ipStreamerCtx->statusEvent);
                break;
        }
    }
    else
    {
        /* there is already a pendingReqForNextSegment from client, so we will need to wakeup main thread to process it */
        if (eventId == B_PlaybackIpEvent_eServerErrorStreaming) {
            ipStreamerCtx->resetTranscodePipe = true;
            closeSocket(ipStreamerCtx->cfg.streamingFd);
            ipStreamerCtx->cfg.streamingFd = 0; /* clear it as socket */
        }
        ipStreamerCtx->hlsNextSegmentNum++;
        ipStreamerCtx->waitingForNextRequest = true;
        BKNI_SetEvent(ipStreamerCtx->statusEvent);
        BDBG_MSG(("%s: ctx:streamingFd %p: sent event to wakeup hls session thread", BSTD_FUNCTION, (void *)ipStreamerCtx));
    }
#else
    /* HTTP Keep-alive enabled */
    switch(eventId)
    {
        case B_PlaybackIpEvent_eServerEndofSegmentReached:
            ipStreamerCtx->hlsNextSegmentNum++;
            ipStreamerCtx->waitingForNextRequest = true;
            break;
        case B_PlaybackIpEvent_eServerStartStreamingTimedout:
        case B_PlaybackIpEvent_eServerEndofStreamReached:
        default:
            ipStreamerCtx->destroySession = true;
            BKNI_SetEvent(ipStreamerDestroySession);
            break;
    }
#endif
    BKNI_ReleaseMutex(ipStreamerCtx->lock);
}

#if 0
static int
ipStreamerSendVariantPlaylistFile(
    char *url,
    int streamingFd,
    unsigned hlsNextSegmentNum,
    char *rootDir
    )
{
    FILE *fp = NULL;
    char path[256];
    char *tmpPtr=NULL, *tmpPtr1=NULL;
    long fileSize;
    char *playlistFileBuffer = NULL;
    char *playlistFileBufferUpdated = NULL;
    char *playlistBufferPtr;
    char header[1024];
    int len=0;
    int rc = -1;
#ifdef B_USE_HTTP_CHUNK_ENCODING
    char chunkHdr[16];
    int chunkHdrToSend;
#endif
    int payloadLength;
    int i;

    tmpPtr = strstr(url, "/");
    if (!tmpPtr) {
        BDBG_ERR(("%s: Incorrest format for the HTTP request for HLS playlist: is missing '/', req %s", BSTD_FUNCTION, url));
        return -1;
    }
    tmpPtr1 = strstr(tmpPtr, " "); /* searches where the file name ends */
    if (!tmpPtr1) {
        BDBG_ERR(("%s: Incorrest format for the HTTP request for HLS playlist: is missing ' ' at end of file name, req %s", BSTD_FUNCTION, url));
        return -1;
    }
    *tmpPtr1 = '\0';

    memset(path, 0, sizeof(path));
    strncpy(path, rootDir, sizeof(path)-1);
    strncat(path, tmpPtr, sizeof(path)-1);
    BDBG_MSG(("%s: Client is requesting playlist file %s", BSTD_FUNCTION, path));
    fp = fopen(path, "r");
    if (!fp) {
        BDBG_ERR(("%s: Requested HLS Playlist file (%s) doesn't exist, please create it using ch1_live_playlist.m3u8 as an example, errno %d", BSTD_FUNCTION, path, errno));
        perror("fclose");
        return -1;
    }
    if (fseek(fp, 0L, SEEK_END) < 0) {
        BDBG_ERR(("%s: fseek failed for %s file, errno %d", BSTD_FUNCTION, url, errno));
        goto error;
    }
    fileSize = ftell(fp);
    if (fileSize <= 0) {
        BDBG_ERR(("%s: HLS Playlist file %s is zero size, please create a correct it using ch1_live_playlist.m3u8 as an example", BSTD_FUNCTION, path));
        goto error;
    }
    if (fseek(fp, 0L, SEEK_SET) < 0) {
        BDBG_ERR(("%s: fseek failed for %s file, errno %d", BSTD_FUNCTION, url, errno));
        goto error;
    }
    if ((playlistFileBuffer = BKNI_Malloc(fileSize+1)) == NULL) {BDBG_ERR(("BKMI_Malloc Failure at %d", __LINE__)); goto error;}
    memset(playlistFileBuffer, 0, fileSize + 1);
    if (fread(playlistFileBuffer, 1, fileSize, fp) <= 0) {
        BDBG_ERR(("%s: fread failed for %s file, errno %d", BSTD_FUNCTION, url, errno));
        perror("fread");
        goto error;
    }
    playlistFileBuffer[fileSize] = '\0'; /* for coverity */
#define PLAYLIST_FILE_BUFFER_SIZE 1096000
    if ((playlistFileBufferUpdated = BKNI_Malloc(PLAYLIST_FILE_BUFFER_SIZE)) == NULL) {BDBG_ERR(("BKMI_Malloc Failure at %d", __LINE__)); goto error;}
    memset(playlistFileBufferUpdated, 0, PLAYLIST_FILE_BUFFER_SIZE);
    tmpPtr = strstr(playlistFileBuffer, "SEQUENCE:0"); /* EXTINF entries are after this */
    if (tmpPtr) {
        int bytesCopied = 0;
        int bytesLeft = PLAYLIST_FILE_BUFFER_SIZE -1;
        char tempBuf[32];
        /* copies the original playlist file upto the SEQUENCE tag */
        tmpPtr += strlen("SEQUENCE:");
        *tmpPtr = '\0';
        playlistBufferPtr = playlistFileBufferUpdated;
        bytesCopied = snprintf(playlistBufferPtr, bytesLeft, "%s", playlistFileBuffer);
        playlistBufferPtr += bytesCopied;
        bytesLeft -= bytesCopied;

        /* add the segment sequence # at the end of this tag */
        memset(tempBuf, 0, sizeof(tempBuf));
        snprintf(tempBuf, sizeof(tempBuf)-1, "%d", hlsNextSegmentNum);
        bytesCopied = snprintf(playlistBufferPtr, bytesLeft, "%s", tempBuf);
        playlistBufferPtr += bytesCopied;
        bytesLeft -= bytesCopied;

        /* tmpPtr was pointing to the end of SEQUENCE:, skip this seq value */
        tmpPtr += 1;
        /* tmpPtr now points to the next #EXTINF header, copy that over first */
        /* however, there is an extra \n at the end of the tempPtr (since playlist files are manually created using an editor) */
        /* we need to remove that first */
        tmpPtr[strlen(tmpPtr)-1] = '\0';
#define NUM_OF_INITIAL_HLS_SEGMENTS 6000 /* 3 for live, increase it for bounded playlist and enable the END-TAG below */
        for (i=0; i < NUM_OF_INITIAL_HLS_SEGMENTS; i++)
        {
            bytesCopied = snprintf(playlistBufferPtr, bytesLeft, "%s", tmpPtr);
            playlistBufferPtr += bytesCopied;
            bytesLeft -= bytesCopied;

            /* now append the _Seg<num> to the URL string so as to make it unique */
            snprintf(tempBuf, sizeof(tempBuf)-1, "_Seg%d.ts", hlsNextSegmentNum+i);
            bytesCopied = snprintf(playlistBufferPtr, bytesLeft, "%s", tempBuf);
            playlistBufferPtr += bytesCopied;
            bytesLeft -= bytesCopied;
        }
#if 1
        /* Optionally add END tag to make it a bounded playlist */
        bytesCopied = snprintf(playlistBufferPtr, bytesLeft, "\n#EXT-X-ENDLIST");
        playlistBufferPtr += bytesCopied;
        bytesLeft -= bytesCopied;
#endif
    }
    else {
        BDBG_ERR(("%s: missing Seq Header from playlist file, please create a correct it using ch1_live_playlist.m3u8 as an example", BSTD_FUNCTION, path));
        goto error;
    }
    memset(header, 0, sizeof(header));
    payloadLength = (playlistBufferPtr - playlistFileBufferUpdated);
    len = snprintf(header, sizeof(header)-1,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/x-mpegURL\r\n"
#ifdef B_USE_HTTP_CHUNK_ENCODING
            "TRANSFER-ENCODING: chunked\r\n"
#else
            "Content-Length: %d\r\n"
#endif
            "SERVER: Broadcom Test HLS Server\r\n"
            "Accept-Ranges: bytes\r\n"
            "\r\n"
#ifndef B_USE_HTTP_CHUNK_ENCODING
            ,payloadLength
#endif
            );
    /* send out HTTP response */
    if (write(streamingFd, header, len) != len) {
        BDBG_ERR(("Failed to write HTTP Response of %d bytes", len));
        perror("write(): ");
        goto error;
    }
#ifdef B_USE_HTTP_CHUNK_ENCODING
        {
            memset(chunkHdr, 0, sizeof(chunkHdr));
            chunkHdrToSend = snprintf(chunkHdr, sizeof(chunkHdr)-1, "%x\r\n", payloadLength);
            rc = write(streamingFd, chunkHdr, chunkHdrToSend);
            if (rc != chunkHdrToSend) {
                BDBG_MSG(("%s: Failed to send %d bytes of chunk header begining, sent %d for socket %d", BSTD_FUNCTION, chunkHdrToSend, rc, streamingFd));
                return -1;
            }
            BDBG_MSG(("wrote %d bytes of chunk hdr beginging: %s, chunkhdr size %d", chunkHdrToSend, chunkHdr, payloadLength));
        }
#endif
    if (write(streamingFd, playlistFileBufferUpdated, (playlistBufferPtr - playlistFileBufferUpdated)) < 0) {
        BDBG_ERR(("Failed to write data part of HTTP Response of %d bytes", len));
        perror("write(): ");
        goto error;
    }
#ifdef B_USE_HTTP_CHUNK_ENCODING
        {
            /* send end of current chunk header */
            memset(chunkHdr, 0, sizeof(chunkHdr));
            chunkHdrToSend = snprintf(chunkHdr, sizeof(chunkHdr)-1, "\r\n");
            rc = write(streamingFd, chunkHdr, chunkHdrToSend);
            if (rc != chunkHdrToSend) {
                BDBG_MSG(("%s: Failed to send %d bytes of chunk header end, sent %d for socket %d", BSTD_FUNCTION, chunkHdrToSend, rc, streamingFd));
                return -1;
            }
            BDBG_MSG(("wrote %d bytes of chunk hdr end: %x %x", chunkHdrToSend, chunkHdr[0], chunkHdr[1]));
        }
#endif
#ifdef B_USE_HTTP_CHUNK_ENCODING
        {
            memset(chunkHdr, 0, sizeof(chunkHdr));
            chunkHdrToSend = snprintf(chunkHdr, sizeof(chunkHdr)-1, "%x\r\n", 0);
            rc = write(streamingFd, chunkHdr, chunkHdrToSend);
            if (rc != chunkHdrToSend) {
                BDBG_MSG(("%s: Failed to send %d bytes of chunk header begining, sent %d for socket %d", BSTD_FUNCTION, chunkHdrToSend, rc, streamingFd));
                return -1;
            }
            BDBG_MSG(("wrote %d bytes of chunk hdr beginging: %s, chunkhdr size %d", chunkHdrToSend, chunkHdr, payloadLength));
        }
#endif
#if 0
        {
            FILE *playlistFp;
            BDBG_WRN(("writing playlist file ....."));
            playlistFp = fopen("/data/media/playlist.m3u8", "w"); /* this will be closed in LiveMedia */
            BDBG_ASSERT(playlistFp);
            fwrite(playlistFileBufferUpdated, 1, (playlistBufferPtr - playlistFileBufferUpdated), playlistFp);
            fclose(playlistFp);
        }
#endif
    BDBG_MSG(("%s: HTTP response w/ Playlist\n%s%s", BSTD_FUNCTION, header, playlistFileBufferUpdated));
    rc = 0;
error:
    if (playlistFileBufferUpdated)
        BKNI_Free(playlistFileBufferUpdated);
    if (playlistFileBuffer)
        BKNI_Free(playlistFileBuffer);
    if (fp)
        fclose(fp);
    return rc;
}
#endif

#ifdef NEXUS_HAS_VIDEO_ENCODER
#define HLS_XCODE_FILE_EXTENSION "_playlist_high.txt"
static int
IpStreamerReadXcodeParametersFile(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx,
    char *inputUri,
    char *urlBuffer,
    int urlBufferLen
    )
{
    char *urlStart;
    char *tmpPtr1;
    FILE *fp;

    BDBG_MSG(("%s: Complete Request %s", BSTD_FUNCTION, inputUri));
    /* Example inputUri: GET /macysParade_Tivo_master.m3u8 */
    if ((urlStart = strstr(inputUri, "/")) == NULL) {
        BDBG_ERR(("%s: invalid URL format, missing / before file name: %s", BSTD_FUNCTION, inputUri));
        return -1;
    }
    else {
        if ((tmpPtr1 = strstr(urlStart, "_master")) == NULL) {
            BDBG_ERR(("%s: Incoming URL %s is not in correct HLS format", BSTD_FUNCTION, urlStart));
            return -1;
        }
        else {
            char path[256];
            char *fullUrlPath = NULL;
            size_t fullUrlPathLength;
            /* tmpPtr1 now points to _master part of the inputUri: terminate the string there to make it just the media file name */
            *tmpPtr1 = '\0';
            BDBG_MSG(("%s: URL %s", BSTD_FUNCTION, urlStart));
            fullUrlPathLength = strlen(ipStreamerGlobalCtx->globalCfg.rootDir) + strlen(urlStart) + strlen(HLS_XCODE_FILE_EXTENSION) + 1;
            fullUrlPath = BKNI_Malloc(fullUrlPathLength);
            if (fullUrlPath == NULL) {
                BDBG_ERR(("%s: BKNI_Malloc failed for %d bytes", BSTD_FUNCTION, fullUrlPathLength));
                return -1;
            }
            snprintf(fullUrlPath, fullUrlPathLength, "%s%s%s", ipStreamerGlobalCtx->globalCfg.rootDir, urlStart, HLS_XCODE_FILE_EXTENSION);
            BDBG_MSG(("%s: fullUrlPath %s", BSTD_FUNCTION, fullUrlPath));
            if ((fp = fopen(fullUrlPath, "r")) == NULL) {
                BDBG_ERR(("%s: Failed to open ch info file %s", BSTD_FUNCTION, fullUrlPath));
                perror("fopen");
                if (fullUrlPath) BKNI_Free(fullUrlPath);
                return -1;
            }
            memset(urlBuffer, 0, urlBufferLen);
            if (fread(urlBuffer, 1, urlBufferLen, fp) <= 0) {
                BDBG_ERR(("%s: fread failed for %s file, errno %d", BSTD_FUNCTION, path, errno));
                fclose(fp);
                perror("fread");
                if (fullUrlPath) BKNI_Free(fullUrlPath);
                return -1;
            }
            fclose(fp);
            BDBG_MSG(("%s: Client is requesting a HLS session: url %s actual req %s", BSTD_FUNCTION, fullUrlPath, urlBuffer));
            if (fullUrlPath) BKNI_Free(fullUrlPath);
            return 0;
        }
    }
    return -1;
}

static int
IpStreamerReadUrlFile(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx,
    char *inputUri,
    char *urlBuffer,
    int urlBufferLen
    )
{
    char *tmpPtr;
    char *tmpPtr1;
    FILE *fp;

    if ((tmpPtr = strstr(inputUri, "/")) == NULL) {
        BDBG_ERR(("%s: invalid URL format, missing / before file name: %s", BSTD_FUNCTION, inputUri));
        return -1;
    }
    else {
        if ((tmpPtr1 = strstr(tmpPtr, "_Seg")) == NULL) {
            BDBG_ERR(("%s: Incoming URL %s is not in correct HLS format", BSTD_FUNCTION, tmpPtr));
            return -1;
        }
        else {
            char path[256];
            *tmpPtr1 = '\0';
            BDBG_MSG(("%s: file %s to open", BSTD_FUNCTION, tmpPtr));
            memset(path, 0, sizeof(path));
            strncpy(path, ipStreamerGlobalCtx->globalCfg.rootDir, sizeof(path)-1);
            strncat(path, tmpPtr, sizeof(path)-1);
            if ((fp = fopen(path, "r")) == NULL) {
                BDBG_ERR(("%s: Failed to open ch info file %s", BSTD_FUNCTION, path));
                perror("fopen");
                return -1;
            }
            memset(urlBuffer, 0, urlBufferLen);
            if (fread(urlBuffer, 1, urlBufferLen, fp) <= 0) {
                BDBG_ERR(("%s: fread failed for %s file, errno %d", BSTD_FUNCTION, path, errno));
                fclose(fp);
                perror("fread");
                return -1;
            }
            fclose(fp);
            BDBG_MSG(("%s: Client is requesting a HLS session: url %s actual req %s", BSTD_FUNCTION, path, urlBuffer));
            return 0;
        }
    }
    return -1;
}

int ipStreamerSendDummyBytes(int streamingFd, unsigned requestedBytes, off_t rangeStart, off_t rangeEnd)
{
    char header[1024];
    int len=0;

    memset(header, 0, sizeof(header));
    len = snprintf(header, sizeof(header)-1,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/x-mpegURL\r\n"
            "SERVER: Broadcom Test HLS Server\r\n"
            "Content-Length: %d\r\n"
            "Accept-Ranges: bytes\r\n"
            "Range: bytes=%lld-%lld\r\n"
            "\r\n",
            requestedBytes,
            rangeStart, rangeEnd
            );
    /* send out HTTP response */
    if (write(streamingFd, header, len) != len) {
        BDBG_ERR(("Failed to write HTTP Response of %d bytes", len));
        perror("write(): ");
        goto error;
    }
    if (write(streamingFd, header, requestedBytes) < 0) {
        BDBG_ERR(("Failed to write data part of HTTP Response of %d bytes", requestedBytes));
        perror("write(): ");
        goto error;
    }
    return 0;
error:
    return -1;
}

static int
ipStreamerSendFile(
    char *url,
    int streamingFd,
    char *rootDir,
    unsigned cookie,
    char *contentType
    )
{
    FILE *fp = NULL;
    /* TODO: use BKNI_Malloc here */
    char path[256];
    char *tmpPtr=NULL, *tmpPtr1=NULL;
    long fileSize;
    char *playlistFileBuffer = NULL;
    char header[1024];
    int len=0;
    int rc = -1;

    tmpPtr = strstr(url, "/");
    if (!tmpPtr) {
        BDBG_ERR(("%s: Incorrest format for the HTTP request for HLS playlist: is missing '/', req %s", BSTD_FUNCTION, url));
        return -1;
    }
    tmpPtr1 = strstr(tmpPtr, " "); /* searches where the file name ends: right before HTTP 1.1 token */
    if (!tmpPtr1) {
        BDBG_ERR(("%s: Incorrest format for the HTTP request for HLS playlist: is missing ' ' at end of file name, req %s", BSTD_FUNCTION, url));
        return -1;
    }
    *tmpPtr1 = '\0';

    memset(path, 0, sizeof(path));
    strncpy(path, rootDir, sizeof(path)-1);
    strncat(path, tmpPtr, sizeof(path)-1);
    BDBG_MSG(("%s: Client is requesting playlist file %s", BSTD_FUNCTION, path));
    fp = fopen(path, "r");
    if (!fp) {
        BDBG_ERR(("%s: Requested HLS Playlist file (%s) doesn't exist, errno %d", BSTD_FUNCTION, path, errno));
        perror("fclose");
        return -1;
    }
    if (fseek(fp, 0L, SEEK_END) < 0) {
        BDBG_ERR(("%s: fseek failed for %s file, errno %d", BSTD_FUNCTION, url, errno));
        goto error;
    }
    fileSize = ftell(fp);
    if (fileSize <= 0) {
        BDBG_ERR(("%s: HLS Playlist file %s is zero size, please create a correct it using ch1_live_playlist.m3u8 as an example", BSTD_FUNCTION, path));
        goto error;
    }
    if (fseek(fp, 0L, SEEK_SET) < 0) {
        BDBG_ERR(("%s: fseek failed for %s file, errno %d", BSTD_FUNCTION, url, errno));
        goto error;
    }
    if ((playlistFileBuffer = BKNI_Malloc(fileSize+1)) == NULL) {BDBG_ERR(("BKMI_Malloc Failure at %d", __LINE__)); goto error;}
    memset(playlistFileBuffer, 0, fileSize + 1);
    if (fread(playlistFileBuffer, 1, fileSize, fp) <= 0) {
        BDBG_ERR(("%s: fread failed for %s file, errno %d", BSTD_FUNCTION, url, errno));
        perror("fread");
        goto error;
    }
    playlistFileBuffer[fileSize] = '\0'; /* for coverity */
    memset(header, 0, sizeof(header));
    len = snprintf(header, sizeof(header)-1,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "SERVER: Broadcom Test HLS Server\r\n"
            "Content-Length: %d\r\n"
            "Accept-Ranges: bytes\r\n"
            "Set-Cookie: id=%u\r\n"
            "\r\n",
            contentType,
            strlen(playlistFileBuffer),
            cookie
            );
    /* send out HTTP response */
    if (write(streamingFd, header, len) != len) {
        BDBG_ERR(("Failed to write HTTP Response of %d bytes", len));
        perror("write(): ");
        goto error;
    }
    if (write(streamingFd, playlistFileBuffer, strlen(playlistFileBuffer)) < 0) {
        BDBG_ERR(("Failed to write data part of HTTP Response of %d bytes", len));
        perror("write(): ");
        goto error;
    }
    BDBG_MSG(("%s: Sending HTTP response: %s", BSTD_FUNCTION, header));
    BDBG_MSG(("%s: Playlist %s", BSTD_FUNCTION, playlistFileBuffer));
    rc = 0;
error:
    if (playlistFileBuffer)
        BKNI_Free(playlistFileBuffer);
    if (fp)
        fclose(fp);
    return rc;
}
#endif

#ifdef B_USE_HTTP_KEEPALIVE
static void
removeSocketFdToMonitorList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx,
    int sfd
    )
{
    FD_CLR(sfd, &ipStreamerGlobalCtx->rfds);
    if (sfd == ipStreamerGlobalCtx->maxFd)
        ipStreamerGlobalCtx->maxFd = sfd - 1;
    BDBG_WRN(("%s: sfd %d", BSTD_FUNCTION, sfd));
}

static void
addSocketFdToMonitorList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx,
    bool isListenerSocket,
    int sd
    )
{
    if (isListenerSocket)
        FD_ZERO(&ipStreamerGlobalCtx->rfds);
    FD_SET(sd, &ipStreamerGlobalCtx->rfds);
    FD_SET(ipStreamerGlobalCtx->listeningFd, &ipStreamerGlobalCtx->rfds);
    if (sd > ipStreamerGlobalCtx->maxFd)
        ipStreamerGlobalCtx->maxFd = sd;
    BDBG_MSG(("%s: sfd %d, lfd %d", BSTD_FUNCTION, sd, ipStreamerGlobalCtx->listeningFd));
}

int
readSocketData(
        int sockFd,
        char *requestUrl,
        int requestUrlLen
        )
{
    int nbytes = -1;

    while (!gExitThread) {
        /* Read HTTP request */
        if ((nbytes = read(sockFd, requestUrl, requestUrlLen-1)) <= 0) {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            perror("read failed: ");
            return -1;
        }
        requestUrl[nbytes] = '\0';
        BDBG_MSG(("Read HTTP Req (socket %d, %d bytes, req %s\n)", sockFd, nbytes, requestUrl));
        break;
    }
    return nbytes;
}

bool hlsUrl(
    char *requestUrl
    )
{
    if (strstr(requestUrl, "_playlist") || strstr(requestUrl, ".m3u8"))
        return true;
    else
        return false;
}

bool
processRequestForHlsPlaylists(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx,
    int streamingFd,
    char *requestUrl
    )
{
    int rc = -1;
    bool matchFound = false;
    char * requestUri = requestUrl;
    int requestUrlLen = REQUEST_URL_LEN;
    char *urlPtr = NULL;
    struct sockaddr_in curRemoteAddr;
    int curRemoteAddrLen;
#if 0
    /* ios7 */
    char *tmpPtr1, *tmpPtr2;
    bool xcodeModifyBitrate = false;
#endif
    IpStreamerCtx *ipStreamerCtx = NULL;
    bool urlProcessed = true;
    B_PlaybackIpPsiInfo *psi = NULL;

    /* now check if the new request happens to be for the same HLS session */
    curRemoteAddrLen = sizeof(curRemoteAddr);
    memset(&curRemoteAddr, 0, sizeof(curRemoteAddr));
    if (getpeername(streamingFd, (struct sockaddr *)&curRemoteAddr, (socklen_t *)&curRemoteAddrLen) != 0) {
        BDBG_ERR(("%s: ERROR: Failed to obtain remote IP address at line %d, errno: %d, socket %d", BSTD_FUNCTION, __LINE__, errno, streamingFd));
        goto errorCloseHttp;
    }
    /*if (strstr(requestUri, "_live_playlist.m3u8")) */
    if (strstr(requestUri, "_playlist") && strstr(requestUri, ".m3u8"))
    {
        /* incoming request is for downloading the playlist corresponding to a variant */
        /* user only creates the basic playlist file on the disk before starting ip_streamer */
        /* we append the EXTINF entries here for the live transcoded content */
        unsigned hlsNextSegmentNum = 0;

        /* first we look up the matching ipStreamerCtx for this client, we need this to get the next segment number */
        BKNI_AcquireMutex(ipStreamerCtxQueueMutex);
        for (ipStreamerCtx = BLST_Q_FIRST(&ipStreamerCtxQueueHead), hlsNextSegmentNum = 0;
                ipStreamerCtx;
                ipStreamerCtx = BLST_Q_NEXT(ipStreamerCtx, next))
        {
            IpStreamerConfig *ipStreamerCfg = &ipStreamerCtx->cfg;
            if (!ipStreamerCfg->hlsSession) {
                continue;
            }
            if (memcmp(&curRemoteAddr.sin_addr.s_addr, &ipStreamerCtx->remoteAddr.sin_addr.s_addr, sizeof(curRemoteAddr.sin_addr.s_addr)) == 0) {
                BDBG_MSG(("%s: Found a matching ip ctx %p for Request from: %s:%d, segNum %d", BSTD_FUNCTION, ipStreamerCtx, inet_ntoa(ipStreamerCtx->remoteAddr.sin_addr), htons(ipStreamerCtx->remoteAddr.sin_port), ipStreamerCtx->hlsNextSegmentNum));
                /* now check if this playlist request: add logic to include URL in the match as well */
                if (ipStreamerCtx->cfg.transcode.nonRealTime)
                    hlsNextSegmentNum = 0;
                else
                    hlsNextSegmentNum = ipStreamerCtx->hlsNextSegmentNum;
                break;
            }
        }
        BKNI_ReleaseMutex(ipStreamerCtxQueueMutex);
        /* build the playlist file & send it to the client */
        /* note: if this is the 1st request for the playlist from a client, then no match is found in the search above and hlsNextSegmentNum is 0 */
        if (ipStreamerSendVariantPlaylistFile(requestUri, streamingFd, hlsNextSegmentNum, ipStreamerGlobalCtx->globalCfg.rootDir) < 0) {
            BDBG_ERR(("%s: Failed to send the playlist", BSTD_FUNCTION));
        }
        urlProcessed = true;
    }
    else if (strstr(requestUri, "_playlist") && strstr(requestUri, ".txt"))
    {
        /* incoming request is a HLS segment */
        /* see if there is an existing session for this request, meaning this request is for a subsequent segment for the same HLS session */
        /* in case of match, we just update the streaming socket and wake up the original thread so that it continues streaming to the new socket */
        BKNI_AcquireMutex(ipStreamerCtxQueueMutex);
        for (ipStreamerCtx = BLST_Q_FIRST(&ipStreamerCtxQueueHead), matchFound = false;
                ipStreamerCtx;
                ipStreamerCtx = BLST_Q_NEXT(ipStreamerCtx, next))
        {
            IpStreamerConfig *ipStreamerCfg = &ipStreamerCtx->cfg;
            if (!ipStreamerCfg->hlsSession) {
                continue;
            }
            /* TODO: add logic to include URL in the match as well */
            /* otherwise, 2nd hls request from a client would be treated for the previous hls session */
            /* however, this is not an issue for mobile clients where HLS is primarily used as mobile client typically doesn't have pip */
            if (memcmp(&curRemoteAddr.sin_addr.s_addr, &ipStreamerCtx->remoteAddr.sin_addr.s_addr, sizeof(curRemoteAddr.sin_addr.s_addr)) != 0) {
                /* dont match */
                continue;
            }

            BDBG_MSG(("ipStreamerCtx %p, Request URL %s", ipStreamerCtx, requestUri));
            break;
#ifdef TODO
            /* ssood: ios7 fix this for the variant stream */
            /* found an existing session from this client, now check following about this URL */
            /* -URL for the next segment of a different HLS variant but same channel/HLS session */
            /* -URL for a different channel/HLS session from the same client: channel change */
            urlPtr = strstr(requestUri, "/");
            if (!urlPtr)
                /* seems like an invalid URL, let the else case below figure that out */
                break;
            tmpPtr1 = strstr(urlPtr, "_Seg");
            if (!tmpPtr1)
                /* seems like an invalid URL, let the else case below figure that out */
                break;
            *tmpPtr1 = '\0';
            if (!strcmp(urlPtr, ipStreamerCtx->urlPtr)) {
                /* -URL for the next segment of same HLS variant encoding */
                matchFound = true;
                *tmpPtr1 = '_';
                break;
            }
            *tmpPtr1 = '_';
            /* URL dont match, check if it for a different variant of the same hls session or a different hls session */
            tmpPtr1 = strstr(urlPtr, "_playlist");
            if (!tmpPtr1)
                /* seems like an invalid URL, let the else case below figure that out */
                break;
            tmpPtr2 = strstr(ipStreamerCtx->urlPtr, "_playlist");
            if (!tmpPtr2)
                /* seems like an invalid URL, let the else case below figure that out */
                break;
            *tmpPtr2 = '\0';
            *tmpPtr1 = '\0';
            if (!strcmp(urlPtr, ipStreamerCtx->urlPtr)) {
                /* -URL for the next segment of same HLS variant encoding */
                BDBG_MSG(("%s: switching to another variant %s of the same hls session, prev playlist %s", BSTD_FUNCTION, urlPtr, ipStreamerCtx->urlPtr));
                matchFound = true;
                *tmpPtr1 = '_';
                *tmpPtr2= '_';
                xcodeModifyBitrate = true;
                break;
            }
#endif
        }
        BKNI_ReleaseMutex(ipStreamerCtxQueueMutex);
#if 0
        if (matchFound && ipStreamerCtx)
        {
            /* make sure that this context is waiting for the new connection and then send an event to signal this new connection */
            if (!ipStreamerCtx->waitingForNextRequest) {
                /* since waitingForNextRequest flag is not set, thread sending current segment for this hls session is not yet done sending it */
                /* and client has either asked for the next segment or switched the playlist & thus asked for a different segment */
                /* so this new thread needs to wait until 1st thread finishes (either successfully sending the current segment or an error case) */
                /* then let 1st thread resume with the new socket & xcode params */
                ipStreamerCtx->pendingReqForNextSegment = true;
                BKNI_ResetEvent(ipStreamerCtx->currentSegmentSentEvent);
                BDBG_MSG(("%s: waiting for 1st thread to finish sending the current segment, ctx %p", BSTD_FUNCTION, ipStreamerCtx));
                BKNI_ReleaseMutex(ipStreamerCtxQueueMutex);
                if (BKNI_WaitForEvent(ipStreamerCtx->currentSegmentSentEvent, BKNI_INFINITE)) {
                    BDBG_ERR(("%s: Failed to wait on event at %d", BSTD_FUNCTION, __LINE__));
                    goto errorCloseHttp;
                }
                BKNI_AcquireMutex(ipStreamerCtxQueueMutex);
            }
            /* 1st thread is waiting at this point, so hand over the modified params to this thread */
            ipStreamerCtx->cfg.streamingFd = streamingFd;
#ifdef NEXUS_HAS_VIDEO_ENCODER1
            /* TODO: need to port this code over */
            /* check if xcode params have changed and if so, get the new params */
            if (xcodeModifyBitrate) {
                IpStreamerConfig *cfg;
                if (IpStreamerReadUrlFile(ipStreamerGlobalCtx, requestUri, requestUrl, requestUrlLen, &urlPtr) < 0) {
                    BKNI_ReleaseMutex(ipStreamerCtxQueueMutex);
                    goto errorCloseHttp;
                }
                openSettings.requestUri = requestUrl;
                if ((cfg = BKNI_Malloc(sizeof(IpStreamerConfig))) == NULL) {BDBG_ERR(("BKMI_Malloc Failure at %d", __LINE__)); goto errorCloseHttp;}
                if (parseTranscodeOptions(&openSettings, cfg)) {
                    BDBG_ERR(("%s: Failed to parse transcode related parameters", BSTD_FUNCTION));
                    BKNI_ReleaseMutex(ipStreamerCtxQueueMutex);
                    BKNI_Free(cfg);
                    goto errorCloseHttp;
                }
                if (cfg->transcode.transportBitrate != ipStreamerCtx->cfg.transcode.transportBitrate) {
                    ipStreamerCtx->sessionSettings.xcodeBitrate = cfg->transcode.transportBitrate;
                    ipStreamerCtx->sessionSettings.xcodeModifyBitrate = true;
                }
                BKNI_Free(cfg);
            }
#endif
            ipStreamerCtx->waitingForNextRequest = false;
            BKNI_ReleaseMutex(ipStreamerCtxQueueMutex);
            BDBG_MSG(("%s: passed over the new socket & xcode info to the 1st hls session thread, CTX %p", BSTD_FUNCTION, ipStreamerCtx));
            BKNI_SetEvent(ipStreamerCtx->statusEvent);
            /* we dont need to proceed w/ setting up a new context, so this request is done */
            urlProcessed = true;
        }
        else
#endif
            if (/*matchFound == false &&*/ ipStreamerCtx)
        {
            IpStreamerSettings ipSessionSettings;
            rc = sendHttpResponse(&ipStreamerCtx->cfg, streamingFd, psi);
            if (rc) {
                BDBG_ERR(("Failed to send HTTP response (%d)... ", rc));
                goto errorCloseHttp;
            }
            BDBG_MSG(("%s: Sent HTTP Response to server on socket %d, segCount %d", BSTD_FUNCTION, streamingFd, ipStreamerCtx->transcoderDst->segmentCount));
            if (ipStreamerCtx->transcoderDst->segmentCount++ == HLS_XCODE_RAMUP_SEGMENTS) {
                adjustEncoderSettings(ipStreamerCtx, &ipStreamerCtx->cfg);
            }
            /* send event to IP library to resume streaming and continue waiting */
            memset(&ipSessionSettings, 0, sizeof(ipSessionSettings));
            ipSessionSettings.streamingEnabled = false; /* as this is only set one time during the session start */
            ipSessionSettings.resumeStreaming = true;
            ipSessionSettings.streamingFd = streamingFd;
            ipSessionSettings.xcodeModifyBitrate = ipStreamerCtx->sessionSettings.xcodeModifyBitrate;
            ipSessionSettings.xcodeBitrate = ipStreamerCtx->sessionSettings.xcodeBitrate;
            if (B_IpStreamer_SessionSetSettings(ipStreamerCtx,&ipSessionSettings)) {
                BDBG_ERR(("%s: Failed to resume streaming for the next segment", BSTD_FUNCTION));
                goto errorCloseHttp;
            }
            urlProcessed = true;
            /* we have indicated to resume streaming, thus go back to wait on the event */
            BDBG_MSG(("%s: CTX %p, we have indicated playback_ip to resume streaming, thus go back to wait on the event", BSTD_FUNCTION, ipStreamerCtx));
        }
        else {
            /* hls session is not yet setup, need to open the *_hls_info.txt file and get the actual URL */
            if (IpStreamerReadUrlFile(ipStreamerGlobalCtx, requestUri, requestUrl, requestUrlLen, &urlPtr) < 0) {
                goto errorCloseHttp;
            }
            requestUri = requestUrl;
            urlProcessed = false;
        }
    }
    else if (strstr(requestUri, ".m3u8"))
    {
        /* incoming request is for downloading the top level playlist containing the various playlists */
        /* simply stream that file out */
        if ((rc = ipStreamerSendFile(requestUrl, streamingFd, ipStreamerGlobalCtx->globalCfg.rootDir)) < 0)
        {
            BDBG_ERR(("%s: Failed to send top level playlist", BSTD_FUNCTION));
        }
    }
errorCloseHttp:
    return urlProcessed;
}

int
processIncomingRequest(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx,
    int streamingFd
    )
{
    int rc;
    B_PlaybackIpPsiInfo *psi = NULL;
    IpStreamerCtx *ipStreamerCtx;
    char * requestUrl = NULL;
    IpStreamerOpenSettings openSettings;
    char *urlPtr = NULL;
    bool urlProcessed = false;

    requestUrl = (char *)BKNI_Malloc(REQUEST_URL_LEN);
    BKNI_Memset(requestUrl, 0, REQUEST_URL_LEN);
    BDBG_ASSERT(requestUrl);
    psi = (B_PlaybackIpPsiInfo *) BKNI_Malloc( sizeof(*psi) );
    BKNI_Memset(psi, 0, sizeof(*psi));
    BDBG_ASSERT(psi);

    if ( (rc = readSocketData(streamingFd, requestUrl, REQUEST_URL_LEN)) <= 0)
    {
        BDBG_WRN(("%s: got rc %d, done with this streamingFd %d", BSTD_FUNCTION, rc, streamingFd));
        removeSocketFdToMonitorList(ipStreamerGlobalCtx, streamingFd);
        close(streamingFd);
    }
    else
    {
        BDBG_WRN(("process request %s", requestUrl));
        if ( hlsUrl(requestUrl) == true )
        {
            urlProcessed = processRequestForHlsPlaylists(ipStreamerGlobalCtx, streamingFd, requestUrl);
        }
        else
        {
            urlProcessed = false;
        }
        if (!urlProcessed)
        {
            /* Open the streaming session */
            memset(&openSettings, 0, sizeof(openSettings));
            openSettings.streamingFd = streamingFd;
            openSettings.requestUri = requestUrl;
            openSettings.mediaProbeOnly = false;
            openSettings.autoRewind = getEnvVariableValue("autoRewind", 1);
            openSettings.eventCallback = ipStreamerEventCallback;
            /* coverity[stack_use_local_overflow] */
            /* coverity[stack_use_overflow] */
            ipStreamerCtx = (IpStreamerCtx *)B_IpStreamer_SessionOpen(ipStreamerGlobalCtx, &openSettings);
            if (!ipStreamerCtx) {BDBG_ERR(("ERROR: Failed to Open IP Streaming Context")); goto errorCloseHttp;}
            if (urlPtr)
                /* TODO ios: this needs to be set to the actual file URL */
                ipStreamerCtx->urlPtr = urlPtr;

            if (!psi->psiValid) {
                /* psi info wasn't sent from the client, so obtain PSI info from requested source */
                rc = B_IpStreamer_SessionAcquirePsiInfo(ipStreamerCtx, psi);
                if (rc) {BDBG_ERR(("Failed to acquire PSI Info ")); goto errorCloseSrc;}
            }

            /* send response to client for Live streaming request, we include the PSI info in the response */
            BDBG_MSG(("CTX %p: vpid %d, apid %d", ipStreamerCtx, psi->videoPid, psi->audioPid));
            rc = sendHttpResponse(&ipStreamerCtx->cfg, streamingFd, psi);
            if (rc) {BDBG_ERR(("Failed to send HTTP response (%d)... ", rc)); goto errorCloseSrc;}
            BDBG_MSG(("Sent HTTP Response to server on socket %d", streamingFd));

            /* now start streaming session */
            rc = B_IpStreamer_SessionStart((void *)ipStreamerCtx, psi);
            if (rc < 0) {BDBG_ERR(("ERROR: Failed to start IP Streamer Session")); goto errorCloseSrc;}

            if (ipStreamerCtx->cfg.hlsSession)
            {
                int remoteAddrLen;
                BKNI_AcquireMutex(ipStreamerCtxQueueMutex);
                /* we maintain a list of hls session contexts, this enables us to find an existing context when a new request comes for the next segment of a the same HLS session */
                BLST_Q_INSERT_TAIL(&ipStreamerCtxQueueHead, ipStreamerCtx, next);
                BKNI_ReleaseMutex(ipStreamerCtxQueueMutex);
                remoteAddrLen = sizeof(ipStreamerCtx->remoteAddr);
                memset(&ipStreamerCtx->remoteAddr, 0, sizeof(ipStreamerCtx->remoteAddr));
                if (getpeername(ipStreamerCtx->cfg.streamingFd, (struct sockaddr *)&ipStreamerCtx->remoteAddr, (socklen_t *)&remoteAddrLen) != 0) {
                    BDBG_ERR(("%s: ERROR: Failed to obtain remote IP address at line %d, errno: %d, socket %d", BSTD_FUNCTION, __LINE__, errno, ipStreamerCtx->cfg.streamingFd));
                    goto errorCloseHttp;
                }
                BDBG_MSG(("%s: Saving HLS Request session from: %s:%d", BSTD_FUNCTION, inet_ntoa(ipStreamerCtx->remoteAddr.sin_addr), htons(ipStreamerCtx->remoteAddr.sin_port)));

            }
            return 0;

errorCloseSrc:
            B_IpStreamer_SessionClose(ipStreamerCtx);
errorCloseHttp:

            closeSocket(streamingFd);
        }
    }
    if (requestUrl) BKNI_Free(requestUrl);
    if (psi) BKNI_Free(psi);
    return 0;
}

    int
acceptNewConnection(
        int listeningFd
        )
{
    struct sockaddr_in remoteAddr;
    int addrLen = sizeof(remoteAddr);
    int sockFd;

    while (!gExitThread)
    {
        /* accept connection */
        if ((sockFd = accept(listeningFd, (struct sockaddr *)&remoteAddr, (socklen_t *)&addrLen)) < 0)
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            perror("ERROR: accept(): return -1...");
            return -1;
        }
        break;
    }
    BDBG_WRN(("%s: new connection from %s:%d on sfd %d", BSTD_FUNCTION, inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port), sockFd));
    return (sockFd);
}

static int
processNetworkEvent(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx,
    int sfd
    )
{
    int rc;
    int newSocketFd;
    BDBG_MSG(("%s: sfd %d", BSTD_FUNCTION, sfd));
    if (sfd == ipStreamerGlobalCtx->listeningFd)
    {
        newSocketFd = acceptNewConnection(ipStreamerGlobalCtx->listeningFd);
        BDBG_ASSERT(newSocketFd > 0);
        addSocketFdToMonitorList(ipStreamerGlobalCtx, false /* isListenerSocket */, newSocketFd);
    }
    else
    {
        rc = processIncomingRequest(ipStreamerGlobalCtx, sfd);
        if (rc < 0)
        {
            BDBG_ERR(("processIncomingRequest Failed"));
            return -1;
        }
    }
    return 0;
}

static int
waitForNetworkEventAndProcessThem(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    int sfd;
    fd_set rfds;
    int numReadReadyFds = -1;
    struct timeval tv;

    while (!gExitThread)
    {
        FD_ZERO(&rfds);
        rfds = ipStreamerGlobalCtx->rfds;
        FD_SET(ipStreamerGlobalCtx->listeningFd, &rfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        if ( (numReadReadyFds = select(ipStreamerGlobalCtx->maxFd +1, &rfds, NULL, NULL, &tv)) < 0 )
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            perror("ERROR: select(): returning error...");
            return -1;
        }
        else if ( numReadReadyFds == 0 )
        {
            /* timeout: continue */
            continue;
        }
        /* got some events, break to process them */
        break;
    }

    /* check if next socket has a read event: we always start w/ the listener socket as it's the 1st one created for now ! */
    sfd = ipStreamerGlobalCtx->listeningFd;
    BDBG_MSG(("%s: select returned %d", BSTD_FUNCTION, numReadReadyFds));
    for (i = 0; i < numReadReadyFds; i++)
    {
        int j = 0;
        BDBG_MSG(("%s: checking for fd %d, is set %d ", BSTD_FUNCTION, sfd, FD_ISSET(sfd, &rfds)));
        while (j++ < ipStreamerGlobalCtx->maxFd && (!FD_ISSET(sfd, &rfds)))
        {
            sfd += 1;
        }
        if (sfd <= ipStreamerGlobalCtx->maxFd && (FD_ISSET(sfd, &rfds)))
        {
            processNetworkEvent(ipStreamerGlobalCtx, sfd);
            FD_CLR(sfd, &rfds);
        }
    }

    return numReadReadyFds;
}

void stopAndCloseSession(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    IpStreamerCtx *ipStreamerCtx;
    BSTD_UNUSED(ipStreamerGlobalCtx);

    BKNI_AcquireMutex(ipStreamerCtxQueueMutex);
    ipStreamerCtx = BLST_Q_FIRST(&ipStreamerCtxQueueHead);
    while (ipStreamerCtx)
    {
        if (ipStreamerCtx->destroySession)
        {
            IpStreamerCtx *ipStreamerCtxNext;

            BDBG_WRN(("%s: destroying ctx %p", BSTD_FUNCTION, ipStreamerCtx));
            ipStreamerCtxNext = BLST_Q_NEXT(ipStreamerCtx, next);
            BLST_Q_REMOVE(&ipStreamerCtxQueueHead, ipStreamerCtx, next);
            B_IpStreamer_SessionStop(ipStreamerCtx);
            B_IpStreamer_SessionClose(ipStreamerCtx);
            ipStreamerCtx = ipStreamerCtxNext;
        }
        else
            ipStreamerCtx = BLST_Q_NEXT(ipStreamerCtx, next);
    }
    BKNI_ReleaseMutex(ipStreamerCtxQueueMutex);
}
#endif

extern int parseToken(const char *input, char *output, int output_len, char *begin, char *end);
unsigned getRangeHeaderValue(char *requestUri, off_t *rangeStart, off_t *rangeEnd)
{
    char tmpBuf[256];
    if (!requestUri)
        return 0;
    if (parseToken(requestUri, tmpBuf, sizeof(tmpBuf), "Range:", "\r\n") == 0)
    {
        char tmpBuf1[64], *tmpPtr;
        tmpPtr = tmpBuf;    /* points to "bytes=" string */
        if (parseToken(tmpBuf, tmpBuf1, sizeof(tmpBuf1), "bytes=", "-") == 0)
        {
            *rangeStart = strtoll(tmpBuf1, (char **)NULL, 10);
            BDBG_MSG(("Byte Range: Start %lld", *rangeStart));
        }
        tmpPtr[strlen(tmpPtr)] = '\n';
        if (parseToken(tmpPtr, tmpBuf1, sizeof(tmpBuf1), "-", "\n") == 0)
        {
            *rangeEnd = strtoll(tmpBuf1, (char **)NULL, 10);
            BDBG_MSG(("Byte Range: End %lld", *rangeEnd));
        }
    }
    return ((*rangeEnd > 0) && *rangeEnd) > *rangeStart ? (*rangeEnd - *rangeStart+1): *rangeStart;
}

#ifdef NEXUS_HAS_VIDEO_ENCODER
#define HLS_SEGMENT_NAME_PREFIX "_Seg"
static unsigned findSegmentNumber(
    char *requestUri
    )
{
    unsigned segmentNumber = 0;;
    char *tmpPtr1;
    /* /ch3_playlist1_file.txt_Seg0.ts */
    if (!requestUri)
        return 0;
    tmpPtr1 = strstr(requestUri, HLS_SEGMENT_NAME_PREFIX);
    if (tmpPtr1)
    {
        char *tmpPtr2;
        tmpPtr1 += strlen(HLS_SEGMENT_NAME_PREFIX);
        tmpPtr2 = strchr(tmpPtr1, '.');
        if (tmpPtr2)
        {
            *tmpPtr2 = '\0';
            segmentNumber = strtoul(tmpPtr1, (char **)NULL, 10);
            *tmpPtr2 = '.';
        }
    }
    return segmentNumber;
}

IpStreamerCtx *ipStreamerLookupHlsSession(unsigned cookie, struct sockaddr_in *curRemoteAddr)
{
    IpStreamerCtx *ipStreamerCtx;

    BDBG_MSG(("%s: cookie:RemoteIp:Port %d:%s:%d", BSTD_FUNCTION, cookie, inet_ntoa(curRemoteAddr->sin_addr), htons(curRemoteAddr->sin_port)));
    BKNI_AcquireMutex(ipStreamerCtxQueueMutex);
    for (ipStreamerCtx = BLST_Q_FIRST(&ipStreamerCtxQueueHead);
            ipStreamerCtx;
            ipStreamerCtx = BLST_Q_NEXT(ipStreamerCtx, next))
    {
    if (ipStreamerCtx) BDBG_MSG(("%s: Entry hls session: ctx:cookie:RemoteIp:Port %p:%d:%s:%d", BSTD_FUNCTION, (void *)ipStreamerCtx, ipStreamerCtx->cookie, inet_ntoa(ipStreamerCtx->remoteAddr.sin_addr), htons(ipStreamerCtx->remoteAddr.sin_port)));
        if (memcmp(&curRemoteAddr->sin_addr.s_addr, &ipStreamerCtx->remoteAddr.sin_addr.s_addr, sizeof(curRemoteAddr->sin_addr.s_addr)) != 0)
            /* remote IP address doesn't match, try next hlsSession */
            continue;
#if 0
        if (ipStreamerCtx->cookie != cookie)
            continue;
#endif
        /* found a matching hlsSession */
        break;
    }
    BKNI_ReleaseMutex(ipStreamerCtxQueueMutex);
    if (ipStreamerCtx) BDBG_MSG(("%s: Found a matching hls session: ctx:cookie:RemoteIp:Port %p:%d:%s:%d", BSTD_FUNCTION, (void *)ipStreamerCtx, ipStreamerCtx->cookie, inet_ntoa(ipStreamerCtx->remoteAddr.sin_addr), htons(ipStreamerCtx->remoteAddr.sin_port)));
    return ipStreamerCtx;
}

void ipStreamerInsertHlsSession(unsigned cookie, struct sockaddr_in *curRemoteAddr, IpStreamerCtx * ipStreamerCtx)
{
    BDBG_ASSERT(ipStreamerCtx);
    BKNI_AcquireMutex(ipStreamerCtxQueueMutex);
    ipStreamerCtx->cookie = cookie;
    ipStreamerCtx->remoteAddr = *curRemoteAddr;
    BLST_Q_INSERT_TAIL(&ipStreamerCtxQueueHead, ipStreamerCtx, next);
    BKNI_ReleaseMutex(ipStreamerCtxQueueMutex);
    BDBG_MSG(("%s: ipStreamerCtx %p", BSTD_FUNCTION, (void *)ipStreamerCtx));
}

void ipStreamerRemoveHlsSession(IpStreamerCtx * ipStreamerCtx)
{
    BDBG_ASSERT(ipStreamerCtx);
    if (ipStreamerCtx->cfg.hlsSession) {
        BKNI_AcquireMutex(ipStreamerCtxQueueMutex);
        BLST_Q_REMOVE(&ipStreamerCtxQueueHead, ipStreamerCtx, next);
        BKNI_ReleaseMutex(ipStreamerCtxQueueMutex);
    }
}

#define COOKIE_HEADER "Cookie: id="
unsigned getCookeiHeaderValue(char *requestUri)
{
    unsigned currentCookie = 0;
    char tmpBuf[256];
    if (!requestUri) return currentCookie;
    if (parseToken(requestUri, tmpBuf, sizeof(tmpBuf), COOKIE_HEADER, "\r\n") == 0)
    {
        currentCookie = strtoul(tmpBuf, (char **)NULL, 10);
    }
    return currentCookie;
}

#define HLS_PLAYLIST_PREFIX "_playlist_"
#define HLS_PLAYLIST_SUFFIX ".txt_Seg"
bool matchPlaylists(const char *requestUri, const char *currentUri)
{
    char *tmpPtr1, *tmpPtr2;

    if (!requestUri  || !currentUri)
        return false;
    tmpPtr1 = strstr(requestUri, HLS_PLAYLIST_PREFIX);
    tmpPtr2 = strstr(currentUri, HLS_PLAYLIST_PREFIX);
    if (tmpPtr1 && tmpPtr2)
    {
        char *end1, *end2;
        end1 = strstr(tmpPtr1, HLS_PLAYLIST_SUFFIX);
        end2 = strstr(tmpPtr2, HLS_PLAYLIST_SUFFIX);
        if (end1 && end2)
        {
            if (strncmp(tmpPtr1, tmpPtr2, (end1 - tmpPtr1)) == 0)
                return true;
        }
    }
    return false;
}

extern void closeNexusTranscoderPipe( IpStreamerCtx *ipStreamerCtx, TranscoderDst *transcoderDst);
extern int seekNexusTranscoderPipeNonRealTime( IpStreamerCtx *ipStreamerCtx, TranscoderDst *transcoderDst, NEXUS_PlaybackPosition seekPosition);
#endif
void * ipStreamerHttpStreamingThread(
    void *data
    )
{
    int err;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    B_PlaybackIpPsiInfo *psi = NULL;
    IpStreamerCtx *ipStreamerCtx;
    IpStreamerGlobalCtx *ipStreamerGlobalCtx = (IpStreamerGlobalCtx *)data;
    int streamingFd;
    char *requestUri = NULL;
    char * requestUrl = NULL;
    int requestUrlLen = REQUEST_URL_LEN;
    IpStreamerOpenSettings openSettings;
    struct sockaddr_in curRemoteAddr;
    int curRemoteAddrLen;
    char *urlPtr = NULL;

    BDBG_WRN(("Starting HTTP Streaming Thread"));

    requestUrl = (char*) BKNI_Malloc(REQUEST_URL_LEN);
    if (requestUrl == NULL) {
        BDBG_ERR(("%s: could not malloc(%u) bytes for requestUrl ", BSTD_FUNCTION, REQUEST_URL_LEN ));
        goto error_free_the_mem;
    }
    BDBG_MSG(("%s: SUCCESSFULL malloc(%u) bytes for requestUrl (%p)", BSTD_FUNCTION, REQUEST_URL_LEN, (void *)requestUrl ));

    psi = (B_PlaybackIpPsiInfo *) BKNI_Malloc( sizeof(*psi) );
    if (psi == NULL) {
        BDBG_ERR(("%s: could not malloc(%u) bytes for psi", BSTD_FUNCTION, sizeof(*psi) ));
        goto error_free_the_mem;
    }
    BDBG_MSG(("%s: SUCCESSFULL malloc(%u) bytes for psi (%p)", BSTD_FUNCTION, sizeof(*psi), (void *)psi ));

    while (!gExitThread)
    {
#ifdef NEXUS_HAS_VIDEO_ENCODER
        bool hlsSession = false;
        bool mpegDashSession = false;
        unsigned currentCookie = 0;
#endif
        unsigned requestedBytes;
        off_t rangeStart = 0;
        off_t rangeEnd = 0;
#define URL_VALUE_LENGTH 256
        char uriValue[URL_VALUE_LENGTH];
        char *tmpPtr;

        memset(uriValue, 0, 256);
        /* coverity[overwrite_var: FALSE] */
        urlPtr = NULL;
        memset(psi, 0, sizeof(B_PlaybackIpPsiInfo));
        requestUrlLen = REQUEST_URL_LEN;

        /* wait on new client request */
        memset(requestUrl, 0, REQUEST_URL_LEN );
        requestUri = acceptNewHttpRequest(ipStreamerGlobalCtx->listeningFd, requestUrl, requestUrlLen-1, &streamingFd);
        if (requestUri == NULL) {BDBG_ERR(("Accept failed, go back to receiving new client request")); continue;}
        requestUri = strchr(requestUri, '/'); /* move past the /GET or /HEAD request */
        if (requestUri == NULL) {BDBG_ERR(("Invalid URI: Missing Method (GET or HEAD) Name, ignoring it ")); continue;}
        tmpPtr = strchr(requestUri, '\r');
        if (tmpPtr == NULL) {BDBG_ERR(("Invalid URI: Missing Header terminator (CR), ignoring it ")); continue;}
        *tmpPtr = '\0';
        strncpy(uriValue, requestUri, URL_VALUE_LENGTH);
        *tmpPtr = '\r';

        curRemoteAddrLen = sizeof(curRemoteAddr);
        memset(&curRemoteAddr, 0, sizeof(curRemoteAddr));
        if (getpeername(streamingFd, (struct sockaddr *)&curRemoteAddr, (socklen_t *)&curRemoteAddrLen) != 0)
        {
            BDBG_ERR(("%s: ERROR: Failed to obtain remote IP address at line %d, errno: %d, socket %d", BSTD_FUNCTION, __LINE__, errno, streamingFd));
            closeSocket(streamingFd);
            continue;
        }
        requestedBytes = getRangeHeaderValue(requestUri, &rangeStart, &rangeEnd);
        BDBG_MSG(("%s: Request from: %s:%d, streamingFd %d, req bytes %u\n%s", BSTD_FUNCTION, inet_ntoa(curRemoteAddr.sin_addr), htons(curRemoteAddr.sin_port), streamingFd, requestedBytes, requestUri));

#ifdef NEXUS_HAS_VIDEO_ENCODER
        currentCookie = getCookeiHeaderValue(requestUri);
        if (requestedBytes && strstr(requestUri, "_master") && strstr(requestUri, ".m3u8"))
        {
            /* older iOS devices request just first few bytes of the master playlist, we directly send the requested bytes */
            if (ipStreamerSendDummyBytes(streamingFd, requestedBytes, rangeStart, rangeEnd) < 0) {
                BDBG_ERR(("%s: !!! Failed to send the dummy bytes for requestUri %s", BSTD_FUNCTION, requestUri));
                /* continue below for cleanup */
            }
            closeSocket(streamingFd);
            continue;
        }
        else if (strstr(uriValue, "ch.htm"))
        {
            /* Request is for a top level channel htm file, send it directly */
            if (ipStreamerSendFile(requestUri, streamingFd, ipStreamerGlobalCtx->globalCfg.rootDir, gCookie, HTTP_CONTENT_TYPE_HTML_FILE) < 0) {
                BDBG_ERR(("%s: !!! Failed to send the playlist for requestUri %s", BSTD_FUNCTION, requestUri));
                /* continue below for cleanup */
            }
            closeSocket(streamingFd);
            continue;
        }
        else if (strstr(requestUri, "_playlist") && strstr(requestUri, ".m3u8"))
        {
            /* Request is for a media playlist (not the master playlist) */
            /* For now, we assume that client has already requested the master  playlist and HLS session has been created during that request */
            /* simply send back the playlist and we are done !!!! */
            if (ipStreamerSendFile(requestUri, streamingFd, ipStreamerGlobalCtx->globalCfg.rootDir, gCookie, HTTP_CONTENT_TYPE_HLS_PLAYLISTS) < 0) {
                BDBG_ERR(("%s: !!! Failed to send the playlist for requestUri %s", BSTD_FUNCTION, requestUri));
                /* continue below for cleanup */
            }
            closeSocket(streamingFd);
            continue;
        }
        else if (strstr(requestUri, "_Seg") && strstr(requestUri, ".ts"))
        {
            /* incoming request is for a HLS Segment, see if there is an existing HLS session from this client */
            ipStreamerCtx = ipStreamerLookupHlsSession(currentCookie, &curRemoteAddr);
            if (ipStreamerCtx)
            {
                /* Found an existing session from this client, now check following about this URL */
                /* -1> URL for the next segment of a different HLS variant of this channel/file, we will need to accordingly adjust  the encoding parameters */
                /* -2> URL for the same HLS variant but segment number is not the next one, so it is a seek event */
                /* -3> URL for the next segment of the same HLS variant, so it is a normal playback */

                /* -1> URL for the next segment of a different HLS variant of this channel/file, we will need to accordingly adjust  the encoding parameters */
                BKNI_AcquireMutex(ipStreamerCtx->lock);
                ipStreamerCtx->hlsRequestedSegmentNumber = findSegmentNumber(requestUri);
                if (ipStreamerCtx->firstSegmentReqReceived && matchPlaylists(requestUri, ipStreamerCtx->urlPtr) == false)
                {
                    IpStreamerOpenSettings openSettings;
                    BDBG_MSG(("%s: ####### switching to playlist: requestUri %s to existing Uri %s", BSTD_FUNCTION, ipStreamerCtx->urlPtr, requestUri));
                    /* read the playlist txt file to find the encode parameters to adjust to */
                    if (ipStreamerCtx->urlPtr) free(ipStreamerCtx->urlPtr);
                    ipStreamerCtx->urlPtr = strdup(requestUri); /* save new playlist name */
                    if (IpStreamerReadUrlFile(ipStreamerGlobalCtx, requestUri, requestUrl, requestUrlLen) < 0)
                    {
                        closeSocket(streamingFd);
                        BKNI_ReleaseMutex(ipStreamerCtx->lock);
                        continue;
                    }
                    memset(&openSettings, 0, sizeof(openSettings));
                    openSettings.requestUri = requestUrl;
                    if ( parseTranscodeOptions( &openSettings, &ipStreamerCtx->cfg) < 0)
                    {
                        BDBG_ERR(("%s: ctx %p: Failed to parse transcode related parameters", BSTD_FUNCTION, (void *)ipStreamerCtx));
                        closeSocket(streamingFd);
                        BKNI_ReleaseMutex(ipStreamerCtx->lock);
                        continue;
                    }
                    ipStreamerCtx->switchPlaylist = true;
                }
                else {
                    BDBG_MSG(("%s: playlist match, continue with segment # check: urlPtr %s to requestUri %s", BSTD_FUNCTION, requestUri, ipStreamerCtx->urlPtr));
                }

                if (ipStreamerCtx->firstSegmentReqReceived == false)
                {
                    /* very first segment request: note that we setting up the pipe when the request for master playlist itself arrives and now we got the request for 1st segment in a playlist */
                    /* this is done to lower the xcode setup related latency !!!! */
                    /* save the copy of the current segment URL being played for the purpose of comparing variant playlist switch */
                    ipStreamerCtx->urlPtr = strdup(requestUri);
                    ipStreamerCtx->transcoderDst->segmentCount = 0;
                }
                /* note the necessary info from this segment request */
                ipStreamerCtx->nextSegmentStreamingFd = streamingFd;
                ipStreamerCtx->pendingReqForNextSegment = true;

                if (ipStreamerCtx->waitingForNextRequest || ipStreamerCtx->firstSegmentReqReceived == false)
                {
                    /* since waitingForNextRequest flag is set, main hls streaming thread is waiting for the next segment request, we send an event to indicate the new request */
                    /* this will be true for the when we receive the request for very 1st segment or the streaming thread has finished sending the current segment and is waiting for the next request */
                    BKNI_SetEvent(ipStreamerCtx->statusEvent);
                    BDBG_MSG(("%s: ctx:streamingFd %p:%d: sent event to wakeup hls session thread", BSTD_FUNCTION, (void *)ipStreamerCtx, streamingFd));
                }
                else {
                    /* main hls thread is still waiting for endOfSegment or errorStreaming callback from ip library. when callback fires, it will wake up the main thread. */
                    /* we have already noted the streamingFd for the new connection request and its segment number in ipStreamerCtx. So we are done here. */

                    /* before we return, we check if the requested segment is not equal to the next+1, which would mean that client has either seeked or requested the same segment due to some error */
                    /* we tell playback ip streaming module to abort its current streaming operation. This is done to minimize latency in the event that streaming lib is still waiting for ~200+msec for next GOP data to show up */
                    if (ipStreamerCtx->hlsNextSegmentNum+1 != ipStreamerCtx->hlsRequestedSegmentNumber) /* +1 to account for the currentSegment that we would be finishing */
                    {
                        /* setting this abort flag causes the streaming loop to break out on waiting on encoder output and invoke the erroStreaming callback to app. That will triegger to wake up the main thread */
                        BDBG_MSG(("%s: ctx:streamingFd %p:%d: sent event to ip lib to abort the streaming loop", BSTD_FUNCTION, (void *)ipStreamerCtx, streamingFd));
                        B_PlaybackIpLiveStreamingSettings settings;
                        memset(&settings, 0, sizeof(settings));
                        settings.abortStreaming = true;
                        B_PlaybackIp_LiveStreamingSetSettings(ipStreamerCtx->ipDst->liveStreamingHandle, &settings);
                    }
                    else
                        BDBG_MSG(("%s: ctx:streamingFd %p:%d: delaying sending wakeup event to hls session thread as it is still not done sending segment", BSTD_FUNCTION, (void *)ipStreamerCtx, streamingFd));
                }
                BKNI_ReleaseMutex(ipStreamerCtx->lock);
            }
            else {
                BDBG_ERR(("%s: ######## we must have existing ipStreamerCtx for a media segment request to come in for uri %s", BSTD_FUNCTION   , requestUri));
                /* TODO: send HTTP error here */
                closeSocket(streamingFd);
            }
            continue; /* this thread is done, go back to top to receive the next request from any client */
        }
        else if ((strstr(requestUri, "_master") && strstr(requestUri, ".m3u8"))|| (strstr(requestUri, "_master") && strstr(requestUri, ".mpd")))
        {
            /* incoming request is for master playlist corresponding to a media file */
            if (strstr(requestUri, "_master") && strstr(requestUri, ".mpd"))
                mpegDashSession = true;
            else
                hlsSession = true;

            /* first we look up the matching ipStreamerCtx for this client */
            ipStreamerCtx = ipStreamerLookupHlsSession(currentCookie, &curRemoteAddr);
            if (ipStreamerCtx)
            {
                /* since there is an existing context for this client, it must be a channel change event */
                BDBG_MSG(("%s: ctx %p: ############### switching to another file/channel %s, prev %s", BSTD_FUNCTION, (void *)ipStreamerCtx, requestUri, ipStreamerCtx->urlPtr));
                ipStreamerCtx->channelChange = true;
                requestUri = requestUrl; /* TODO: take this out */
                BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                /* we take away the transcode resources being used for the previous channel/file from this client as client has done channel change! */
                closeNexusTranscoderPipe(ipStreamerCtx, ipStreamerCtx->transcoderDst);
                ipStreamerCtx->transcoderDst = NULL;
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                /* signal the thread working on the previous channel/file from client that it needs to cleanup & finish! */
                BKNI_SetEvent(ipStreamerCtx->statusEvent);
                BDBG_MSG(("%s: ctx %p: Channel Change Event: notified other thread to tear down this context", BSTD_FUNCTION, (void *)ipStreamerCtx));
                /* we will continue below w/ this new HLS session setup */
                /* NOTE: we do the session setup during the master playlist request */
            }
            /* now setup the hls session for this new file/channel */
        }

        /* Open the streaming session */
        memset(&openSettings, 0, sizeof(openSettings));
        if (hlsSession || mpegDashSession)
        {
            /* Get the initial xcode parameters from the file corresponding to this channel/file */
            urlPtr = strdup(requestUri);
            if (IpStreamerReadXcodeParametersFile(ipStreamerGlobalCtx, requestUri, requestUrl, requestUrlLen) < 0)
            {
                /* We shouldn't really get a failure here, assert to catch such bugs early! */
                BDBG_ASSERT(NULL);
                closeSocket(streamingFd);
                if (urlPtr) free(urlPtr);
                continue;
            }
        }
        else
#endif
        {
            /* streamingFd would get set later for HLS sessions when request for the 1st segment comes in */
            openSettings.streamingFd = streamingFd;
        }
        openSettings.requestUri = requestUrl;
        openSettings.mediaProbeOnly = false;
        openSettings.autoRewind = getEnvVariableValue("autoRewind", 0);
        openSettings.eventCallback = ipStreamerEventCallback;
        /* coverity[stack_use_local_overflow] */
        /* coverity[stack_use_overflow] */
        ipStreamerCtx = (IpStreamerCtx *)B_IpStreamer_SessionOpen(ipStreamerGlobalCtx, &openSettings);
        if (!ipStreamerCtx) {BDBG_ERR(("ERROR: Failed to Open IP Streaming Context")); goto errorCloseHttp;}

        rc = B_IpStreamer_SessionAcquirePsiInfo(ipStreamerCtx, psi);
        if (rc) {BDBG_ERR(("Failed to acquire PSI Info ")); goto errorCloseSrc;}
#ifdef NEXUS_HAS_VIDEO_ENCODER
        if (hlsSession || mpegDashSession)
        {
            int rc;

            /* we maintain a list of hls session contexts, this enables us to find an existing context when a new request comes for the next segment of a the same HLS session */
            ipStreamerInsertHlsSession( gCookie, &curRemoteAddr, ipStreamerCtx);
            BDBG_MSG(("%s: Saving HLS Request session from: %s:%d", BSTD_FUNCTION, inet_ntoa(ipStreamerCtx->remoteAddr.sin_addr), htons(ipStreamerCtx->remoteAddr.sin_port)));

            /* we then directly send the master playlist file to the client */
            if (mpegDashSession)
            {
                ipStreamerCtx->cfg.dashSession = true; /* used to set content length for  Osmo4 player */
                rc = ipStreamerSendFile(urlPtr, streamingFd, ipStreamerGlobalCtx->globalCfg.rootDir, gCookie, HTTP_CONTENT_TYPE_MPEG_DASH_MPD);
            }
            else
                rc = ipStreamerSendFile(urlPtr, streamingFd, ipStreamerGlobalCtx->globalCfg.rootDir, gCookie, HTTP_CONTENT_TYPE_HLS_PLAYLISTS);
            if (rc < 0) {
                BDBG_ERR(("%s: !!! Failed to send the playlist for requestUri %s", BSTD_FUNCTION, requestUri));
                /* continue below for cleanup */
            }
            free(urlPtr);
            gCookie++;
            closeSocket(streamingFd);
            BDBG_MSG(("%s: Sent Playlist to: %s:%d on streamingFd %d", BSTD_FUNCTION, inet_ntoa(ipStreamerCtx->remoteAddr.sin_addr), htons(ipStreamerCtx->remoteAddr.sin_port), streamingFd));
            streamingFd = 0;
            ipStreamerCtx->cfg.streamingFd = 0;
            /* set this flag to indicate that we are waiting for subsequent 1st segment request to come in so that we can start streaming on that socket */
            ipStreamerCtx->waitingForNextRequest = true;
        }
        else
#endif
        {
            /* send response to client */
            BDBG_MSG(("CTX %p: streamingFd %d, vpid %d, apid %d", (void *)ipStreamerCtx, streamingFd, psi->videoPid, psi->audioPid));
            rc = sendHttpResponse(&ipStreamerCtx->cfg, streamingFd, psi);
            if (rc)
            {
                BDBG_ERR(("Failed to send HTTP response (%d on streamingFd %d)... ", rc, streamingFd));
#ifdef NEXUS_HAS_VIDEO_ENCODER
                ipStreamerRemoveHlsSession(ipStreamerCtx);
#endif
                goto errorCloseSrc;
            }
            BDBG_MSG(("Sent HTTP Response to server on socket %d", streamingFd));
        }

        err = B_IpStreamer_SessionStart((void *)ipStreamerCtx, psi);
        if (err < 0)
        {
            BDBG_ERR(("ERROR: Failed to start IP Streamer Session"));
#ifdef NEXUS_HAS_VIDEO_ENCODER
            ipStreamerRemoveHlsSession(ipStreamerCtx);
#endif
            goto errorCloseSrc;
        }


#ifdef NEXUS_HAS_VIDEO_ENCODER
        while (ipStreamerCtx->cfg.hlsSession && !gExitThread)
        {
            BERR_Code rc;
            IpStreamerSettings ipSessionSettings;
            /* Streaming has started, now wait on either channel change, pendingSegment events */
            rc = BKNI_WaitForEvent(ipStreamerCtx->statusEvent, 100);
            if (rc == BERR_TIMEOUT) {
                IpStreamerSessionStatus status;
                B_IpStreamer_SessionStatus(ipStreamerCtx, &status);
                continue;
            }
            else if (rc != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: Failed to wait on event at %d", BSTD_FUNCTION, __LINE__));
                break;
            }
            else {
                IpStreamerSessionStatus status;
                B_IpStreamer_SessionStatus(ipStreamerCtx, &status);
            }
            /* we are here because we either got an event from endOfSegment callback when there is a pendingReq, or endOfSegment callback had aleady come but now we got the next request, or client changed the channel on us! */
            if (ipStreamerCtx->channelChange)
            {
                BDBG_MSG(("%s: ctx %p: Channel Change Event: close & tear down this context", BSTD_FUNCTION, (void *)ipStreamerCtx));
                /* we break out of this loop & continue w/ closing this context */
                break;
            }
            else if (ipStreamerCtx->pendingReqForNextSegment)
            {
                /* we have pendingReqForNextSegment which was received by another ipStreamer thread & its segmentNumber & streamingFd was saved in our ipStreamerCtx by that thread */
                /* we check if this is a in-sequence segment request or a seek type jump and accordingly process them. */
                BKNI_AcquireMutex(ipStreamerCtx->lock);
                ipStreamerCtx->cfg.streamingFd = ipStreamerCtx->nextSegmentStreamingFd;
                rc = sendHttpResponse(&ipStreamerCtx->cfg, ipStreamerCtx->cfg.streamingFd, psi);
                if (rc) {
                    BDBG_ERR(("%s: ctx %p: Failed to send HTTP response (%d)..., contine waiting for next request ", BSTD_FUNCTION, (void *)ipStreamerCtx, rc));
                    BKNI_ReleaseMutex(ipStreamerCtx->lock);
                    continue;
                }
                BDBG_MSG(("%s: ctx:streamingFd %p:%d: Sent HTTP Response, segCount %d, hlsNextSegmentNum %u, hlsRequestedSegmentNumber %u", BSTD_FUNCTION, (void *)ipStreamerCtx, ipStreamerCtx->cfg.streamingFd, ipStreamerCtx->transcoderDst->segmentCount,
                           ipStreamerCtx->hlsNextSegmentNum, ipStreamerCtx->hlsRequestedSegmentNumber));

                /* reset state flags that tell either ipStreamer threads or callback from IP lib thread when to send us an event */
                ipStreamerCtx->pendingReqForNextSegment = false;
                ipStreamerCtx->waitingForNextRequest = false;

                memset(&ipSessionSettings, 0, sizeof(ipSessionSettings));
                if (ipStreamerCtx->hlsNextSegmentNum != ipStreamerCtx->hlsRequestedSegmentNumber || ipStreamerCtx->resetTranscodePipe)
                {
                    NEXUS_PlaybackPosition seekPosition;

                    /* -2> URL for the same HLS variant but segment number is not the next one, so most likely client had done a seek operation. In some cases, client may re-request the segment if it doesn't like it */
                    ipStreamerCtx->resetTranscodePipe = true;
                    BDBG_MSG(("%s: ctx %p: Reset Xcode Pipe due to %s: hlsNextSegmentNum %u, hlsRequestedSegmentNumber %u", BSTD_FUNCTION, (void *)ipStreamerCtx,
                                ipStreamerCtx->resetTranscodePipe ? "Streaming Error" : "Client Requested Seek", ipStreamerCtx->hlsNextSegmentNum, ipStreamerCtx->hlsRequestedSegmentNumber));

                    /* since our segments are 1 sec long, the requested segment number is our new seek position */
                    /* TODO: may need a better way to calculate the better seek position. One possibility would be to use the bcmplayer to seek to the correct i-frame */
                    ipStreamerCtx->hlsNextSegmentNum = ipStreamerCtx->hlsRequestedSegmentNumber;
                    if ( ipStreamerCtx->hlsRequestedSegmentNumber > 2)
                        seekPosition = ipStreamerCtx->hlsRequestedSegmentNumber - 2; /* we go back couple of segments */
                    else
                        seekPosition = 0;
                    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                    if (seekNexusTranscoderPipeNonRealTime(ipStreamerCtx, ipStreamerCtx->transcoderDst, seekPosition*1000) < 0)
                    {
                        BDBG_ERR(("%s: ctx %p: seek to position %d FAILED: hlsNextSegmentNum %u, hlsRequestedSegmentNumber %u", BSTD_FUNCTION, (void *)ipStreamerCtx, (int)seekPosition, ipStreamerCtx->hlsNextSegmentNum, ipStreamerCtx->hlsRequestedSegmentNumber));
                        ipStreamerSendHttpResponse(streamingFd, 0, 404, HTTP_CONTENT_TYPE_MPEG_SEGMENT);
                        closeSocket(streamingFd);
                        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                        BKNI_ReleaseMutex(ipStreamerCtx->lock);
                        continue;
                    }
                    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                    BDBG_MSG(("%s: ctx %p: seek done: hlsNextSegmentNum %u, hlsRequestedSegmentNumber %u", BSTD_FUNCTION, (void *)ipStreamerCtx, ipStreamerCtx->hlsNextSegmentNum, ipStreamerCtx->hlsRequestedSegmentNumber));

                    /* reset segment count for ramping up purposes */
                    ipStreamerCtx->transcoderDst->segmentCount = 0;
                    ipStreamerCtx->resetTranscodePipe = false;
                    ipSessionSettings.resetStreaming = true;
                }

#ifndef HLS_DISABLE_RAMPUP
                /* first trigger the encoding rampup logic if we have sent enough initial segments */
                if (ipStreamerCtx->transcoderDst->segmentCount++ == HLS_XCODE_RAMUP_SEGMENTS)
                {
                    adjustEncoderSettings(ipStreamerCtx, &ipStreamerCtx->cfg);
                }
                if (ipStreamerCtx->switchPlaylist)
                {
                    adjustEncoderSettingsPlaylistChange(ipStreamerCtx, &ipStreamerCtx->cfg);
                    ipStreamerCtx->switchPlaylist = false;
                }
#endif

                /* tell IP library to start or resume streaming */
                ipSessionSettings.xcodeModifyBitrate = ipStreamerCtx->sessionSettings.xcodeModifyBitrate;
                ipSessionSettings.xcodeBitrate = ipStreamerCtx->sessionSettings.xcodeBitrate;
                if (ipStreamerCtx->firstSegmentReqReceived == false)
                {
                    /* we are processing 1st segment request, so enable streaming */
                    ipSessionSettings.streamingEnabled = true;
                    ipSessionSettings.resumeStreaming = false;
                    ipStreamerCtx->firstSegmentReqReceived = true;
                }
                else {
                    /* subsequent segment processing, so resume streaming from new streamingFd */
                    ipSessionSettings.streamingEnabled = false; /* as this is only set one time during the session start */
                    ipSessionSettings.resumeStreaming = true;
                }
                ipSessionSettings.streamingFd = ipStreamerCtx->cfg.streamingFd;
                if (B_IpStreamer_SessionSetSettings(ipStreamerCtx, &ipSessionSettings)) {
                    BDBG_ERR(("%s: Failed to resume streaming for the next segment, ignore it and continue!!", BSTD_FUNCTION));
                }

                /* we have started/restarted streaming and now go back to wait for next event */
                BDBG_MSG(("%s: CTX %p, we have indicated playback_ip to resume streaming, thus go back to wait on the event", BSTD_FUNCTION, (void *)ipStreamerCtx));
                BKNI_ReleaseMutex(ipStreamerCtx->lock);
                continue;
            }
            else {
                /* neither a channel change nor a pendingReqForNextSegment, so we continue waiting for either of these to happen */
                BDBG_MSG(("%s: event id %d, CTX %p, we are done due to error or eof on streaming, ignore it and continue!", BSTD_FUNCTION, ipStreamerCtx->eventId, (void *)ipStreamerCtx));
                BDBG_MSG(("%s: CTX %p, neither a channel change nor a pendingReqForNextSegment, so we continue waiting for either of these to happen ", BSTD_FUNCTION, (void *)ipStreamerCtx));
                continue;
            }
        }
#endif
        while (!ipStreamerCtx->cfg.hlsSession && !gExitThread)
        {
            /* loop until thread is stopped, client stops the sessions or incoming live stream underflows */
            IpStreamerSessionStatus status;

            BKNI_Sleep(1000);
            B_IpStreamer_SessionStatus(ipStreamerCtx, &status);
            if (!status.active) {
                BDBG_MSG(("Session is not longer active, breaking out..."));
                break;
            }
        }

        /* we are done w/ this context */
#ifdef NEXUS_HAS_VIDEO_ENCODER
        if (ipStreamerCtx->cfg.hlsSession)
            ipStreamerRemoveHlsSession(ipStreamerCtx);
#endif
        B_IpStreamer_SessionStop(ipStreamerCtx);
        if (ipStreamerCtx->urlPtr) free(ipStreamerCtx->urlPtr);
        ipStreamerCtx->urlPtr = NULL;
        streamingFd = ipStreamerCtx->cfg.streamingFd;
errorCloseSrc:
        B_IpStreamer_SessionClose(ipStreamerCtx);
errorCloseHttp:
        closeSocket(streamingFd);
        if (gExitThread)
            break;
        BDBG_WRN(("CTX %p: Current ip streaming / local recording session is closed, go back to listening for new requests", (void *)ipStreamerCtx));
    } /* while (!gExitThread) */

error_free_the_mem:
    if (psi!= NULL) {
        BDBG_MSG(("%s: free(%u) bytes for psi (%p)", BSTD_FUNCTION, sizeof(*psi), (void *)psi ));
        BKNI_Free ( psi );
    }

    if (requestUrl != NULL) {
        BDBG_MSG(("%s: free(%u) bytes for requestUrl (%p)", BSTD_FUNCTION, REQUEST_URL_LEN, (void *)requestUrl ));
        BKNI_Free ( requestUrl );
    }

    BDBG_WRN(("Exiting Streamer Thread"));
    pthread_exit(NULL);

    return NULL;
}

void *
ipStreamerRtpUdpStreamingThread(
    void *data
    )
{
    int err;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    B_PlaybackIpPsiInfo *psi=NULL;
    IpStreamerCtx *ipStreamerCtx = NULL;
    IpStreamerGlobalCtx *ipStreamerGlobalCtx = (IpStreamerGlobalCtx *)data;
    char * requestUrl = NULL;
    IpStreamerOpenSettings openSettings;
    IpStreamerStreamingOutCfg *streamingCfg;

    BDBG_WRN(("%s: Starting Streaming Thread", BSTD_FUNCTION));

    /*
     * Main flow:
     * Open the streaming socket
     * Call SessionOpen() to:
     *  -select a free src of requested srcType
     * Use SessionAcquitePsiInfo to determine AV PIDS & Codecs (internally starts frontends so that it can receive PSI tables & then stops them)
     * Call SessionStart() to:
     *  -select free dst: IP and/or Record
     *  -start dst(s)
     *  -start src
     * Call SessionStatus() in a loop to check client connection status or signal from local app to exit
     * Call SessionStop() to
     *  -stop dst(s)
     *  -stop dst(s)
     * Call SessionClose() to:
     *  -close dst(s)
     *  -close src
     * exit
     */
    if (!gExitThread) {
        int i;
        BKNI_AcquireMutex(hUdpInitMutex);

        BDBG_MSG(("ipStreamerRtpUdpStreamingThread Current Active sessions: %d", ipStreamerGlobalCtx->globalCfg.active_sessions));

        streamingCfg = &(ipStreamerGlobalCtx->globalCfg.streamingCfg[ipStreamerGlobalCtx->globalCfg.active_sessions]);
        BDBG_MSG((" ipStreamerRtpUdpStreamingThread URL: %s Protocol: %s IP: %s Port: %d",streamingCfg->url, streamingCfg->streamingProtocol == B_PlaybackIpProtocol_eRtp ? "RTP":"UDP",
                        streamingCfg->streamingIpAddress, streamingCfg->streamingPort));
        memset(&openSettings, 0, sizeof(openSettings));
        openSettings.streamingFdLocal = -1; /* indicates no local streaming need */
        /* Open the streaming session */
        openSettings.streamingFd = -1;
        openSettings.mediaProbeOnly = false;

        requestUrl = (char*) BKNI_Malloc(REQUEST_URL_LEN);
        if (requestUrl == NULL) {
            BDBG_ERR(("%s: could not malloc(%u) bytes for requestUrl ", BSTD_FUNCTION, REQUEST_URL_LEN ));
            goto errorCloseHttp;
        }
        BDBG_MSG(("%s: SUCCESSFULL malloc(%u) bytes for requestUrl (%p)", BSTD_FUNCTION, REQUEST_URL_LEN, (void *)requestUrl ));

        psi = (B_PlaybackIpPsiInfo *) BKNI_Malloc( sizeof(*psi) );
        if (psi == NULL) {
            BDBG_ERR(("%s: could not malloc(%u) bytes for psi", BSTD_FUNCTION, sizeof(*psi) ));
            goto errorCloseHttp;
        }
        BDBG_MSG(("%s: SUCCESSFULL malloc(%u) bytes for psi (%p)", BSTD_FUNCTION, sizeof(*psi), (void *)requestUrl ));

        if (strstr(streamingCfg->url, "LiveChannel;") == NULL) {
            i = snprintf(requestUrl, REQUEST_URL_LEN-1, "/File=%s;Protocol=%s;Address=%s;Port=%d;",
                    streamingCfg->url, streamingCfg->streamingProtocol == B_PlaybackIpProtocol_eRtp ? "RTP":"UDP",
                    streamingCfg->streamingIpAddress, streamingCfg->streamingPort);
        }
        else {
            i = snprintf(requestUrl, REQUEST_URL_LEN-1, streamingCfg->url);
        }
        requestUrl[i] = '\0';
        BDBG_MSG(("%s: RTP/UDP source url %s", BSTD_FUNCTION, requestUrl));

        openSettings.requestUri = requestUrl;
        /* coverity[stack_use_local_overflow] */
        /* coverity[stack_use_overflow] */
        ipStreamerCtx = (IpStreamerCtx *)B_IpStreamer_SessionOpen(ipStreamerGlobalCtx, &openSettings);
        if (!ipStreamerCtx) {BDBG_ERR(("ERROR: Failed to Open IP Streaming Context")); goto errorCloseHttp;}

        ipStreamerCtx->cfg.streamingCfg = *streamingCfg;
        /* obtain PSI info from requested source */
        rc = B_IpStreamer_SessionAcquirePsiInfo(ipStreamerCtx, psi);
        if (rc) {BDBG_ERR(("Failed to acquire PSI Info ")); goto errorCloseSrc;}

        BDBG_MSG(("CTX %p: vpid %d, apid %d", (void *)ipStreamerCtx, psi->videoPid, psi->audioPid));

        /* now start streaming session */
        err = B_IpStreamer_SessionStart((void *)ipStreamerCtx, psi);
        BKNI_ReleaseMutex(hUdpInitMutex);
        if (err < 0) {BDBG_ERR(("ERROR: Failed to start IP Streamer Session")); goto errorCloseSrc;}

        while (!gExitThread) {
            IpStreamerSessionStatus status;
            /* loop until thread is stopped, client stops the sessions or incoming live stream underflows */
            BKNI_Sleep(1000);
            B_IpStreamer_SessionStatus(ipStreamerCtx, &status);
            if (!status.active) {
                BDBG_MSG(("Session is not longer active, breaking out..."));
                break;
            }
        } /* while (!gExitThread) */

        B_IpStreamer_SessionStop(ipStreamerCtx);
errorCloseSrc:
        B_IpStreamer_SessionClose(ipStreamerCtx);
errorCloseHttp:
        BKNI_ReleaseMutex(hUdpInitMutex);
        BDBG_WRN(("CTX %p: Current ip streaming / local recording session is done, exiting..", (void *)ipStreamerCtx));
    } /* if (!gExitThread) */

    if (psi!= NULL) {
        BDBG_MSG(("%s: free(%u) bytes for psi (%p)", BSTD_FUNCTION, sizeof(*psi), (void *)psi ));
        BKNI_Free ( psi );
    }

    if (requestUrl != NULL) {
        BDBG_MSG(("%s: free(%u) bytes for requestUrl (%p)", BSTD_FUNCTION, REQUEST_URL_LEN, (void *)requestUrl ));
        BKNI_Free ( requestUrl );
    }

    BDBG_WRN(("Exiting Streamer Thread"));
    gExitThread = 1;
    pthread_exit(NULL);

    return NULL;
}

#define IP_ADDRESS_SEPARATOR    '.'

int main(int argc, char *argv[])
{
    int i;
    int appExitCode = 1;
    IpStreamerGlobalCfg ipStreamerGlobalCfg;
    IpStreamerGlobalCtx *ipStreamerGlobalCtx = NULL;
    void *(*threadFunc)(void*);
    struct ifreq ifr;
#if 0
    char *tempCharPtr;
    unsigned char baseAddr;
    unsigned char tempBaseAddr;
    unsigned int portNum;
#endif
    /* parse command line options */
    if (parserIpStreamerOptions(argc, argv, &ipStreamerGlobalCfg)) {
        BDBG_ERR(("ERROR: Incorrect Command line Options"));
        goto exitApp;
    }
    signal(SIGINT, signalHandler);

    /* ignore SIGPIPE otherwise abnormal termination on client can cause server to crash */
    signal(SIGPIPE, SIG_IGN);

    ipStreamerGlobalCfg.multiProcessEnv = false;

    /* initialize the IP Streamer */
    ipStreamerGlobalCtx = (IpStreamerGlobalCtx *)B_IpStreamer_Init(&ipStreamerGlobalCfg);
    if (!ipStreamerGlobalCtx) {BDBG_ERR(("B_IpStreamer_Init() failed at %d, Exiting...", __LINE__)); goto error;}

    if (BKNI_CreateMutex(&hUdpInitMutex) != 0) {
        BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
        goto exitApp;
    }
    BLST_Q_INIT(&ipStreamerCtxQueueHead);
    if (BKNI_CreateMutex(&ipStreamerCtxQueueMutex) != 0) {
        BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
        goto exitApp;
    }

    /* generate info & nav files for AV content if not already there */
    createInfoAndNavFiles(&ipStreamerGlobalCfg, ipStreamerGlobalCtx);

    /*  assign function to start worker threads */
    for (i=0; i<ipStreamerGlobalCfg.numStreamingSessions; i++)
    {

        if (ipStreamerGlobalCfg.streamingCfg[i].streamingProtocol == B_PlaybackIpProtocol_eRtp ||
            ipStreamerGlobalCfg.streamingCfg[i].streamingProtocol == B_PlaybackIpProtocol_eUdp) {
            threadFunc = ipStreamerRtpUdpStreamingThread;
        }
        else {
            /* setup HTTP listening Server for receiving live streaming and/or local recording requests from clients */
            ipStreamerGlobalCtx->listeningFd = initTcpServer(&ipStreamerGlobalCfg);
            if (ipStreamerGlobalCtx->listeningFd < 0) {BDBG_ERR(("Failed to open lisenting socket (%d) at %d, Exiting...", ipStreamerGlobalCtx->listeningFd, __LINE__)); goto error;}
            threadFunc = ipStreamerHttpStreamingThread;
#ifdef B_USE_HTTP_KEEPALIVE
            addSocketFdToMonitorList(ipStreamerGlobalCtx, true /* isListenerSocket */, ipStreamerGlobalCtx->listeningFd);
#endif
            break;
        }
    }
#ifdef B_USE_HTTP_KEEPALIVE
    /* we only need one thread in this case */
    ipStreamerGlobalCfg.numStreamingSessions = 1;
#endif
#if 0

     if(ipStreamerGlobalCfg.multicastEnable) {
            tempCharPtr = strrchr(ipStreamerGlobalCtx->globalCfg.streamingCfg[i].streamingIpAddress ,IP_ADDRESS_SEPARATOR );
            tempCharPtr++;
            baseAddr = atoi(tempCharPtr);

     }
     else/**unicast case ***/
     {
             portNum = ipStreamerGlobalCtx->globalCfg.streamingCfg[i].streamingPort; /**start port number **/
     }



        if(ipStreamerGlobalCfg.multicastEnable) {
            tempBaseAddr = (i+baseAddr);/** this will take care of wrap around **/
            /** for muticast enable case  tempCharPtr already points to
               the last entry  **/
            snprintf(tempCharPtr,15,"%d",tempBaseAddr);

            BDBG_ERR(("tempCharPtr %s  ",tempCharPtr));
        }
        else {
            ipStreamerGlobalCtx->globalCfg.streamingCfg[i].streamingPort = portNum + i;
        }
#endif
     ipStreamerGlobalCtx->globalCfg.active_sessions = 0;
     BDBG_MSG((" main: numStreamingSessions: %d", ipStreamerGlobalCfg.numStreamingSessions));
     for (i=0; i<ipStreamerGlobalCfg.numStreamingSessions; i++)
     {

        if (pthread_create(&gThreadId[i], NULL, threadFunc, (void *)ipStreamerGlobalCtx)) {
            BDBG_ERR(("Failed to create pthread, errno %d", errno));
            goto exitApp;
        }
        BKNI_Sleep(1000);
        ipStreamerGlobalCtx->globalCfg.active_sessions++;

    }

    if (pthread_create(&gChannelMapThreadId, NULL, channelMapThread, (void *)&ipStreamerGlobalCfg)) {
        BDBG_ERR(("Failed to create channel map pthread, errno %d", errno));
        goto exitApp;
    }

#ifdef B_USE_HTTP_KEEPALIVE
    if (BKNI_CreateEvent(&ipStreamerDestroySession)) {
        BDBG_ERR(("%s: Failed to create event at %d", BSTD_FUNCTION, __LINE__));
        goto exitApp;
    }
#endif
#ifdef LIVEMEDIA_SUPPORT
#if LIVEMEDIA_2013_03_07
    /*CAD/BRCM 20121031 - Added RTSPServer support*/
    if (pthread_create(&gRTSPServerThreadId, NULL, RTSPServerThread, (void *)ipStreamerGlobalCtx)) {
        BDBG_ERR(("Failed to create rtsp server pthread, errno %d", errno));
        goto exitApp;
    }
    BDBG_WRN(("Started RTSPServer Thread (with live satellite streaming)" ));
#endif /*LIVEMEDIA_2013_03_07*/
#endif /* LIVEMEDIA_SUPPORT */

    strncpy(ifr.ifr_name, ipStreamerGlobalCfg.interfaceName, IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1] = '\0';
    BDBG_MSG(("%s: interface name %s", BSTD_FUNCTION, ifr.ifr_name));


    /* now retrieve the IP address associated with the media */
    if (ipStreamerGlobalCfg.streamingCfg[0].streamingProtocol == B_PlaybackIpProtocol_eHttp) {
        if (ioctl(ipStreamerGlobalCtx->listeningFd, SIOCGIFADDR, &ifr) != 0) {
            BDBG_WRN(("%s: Failed to get Interface Address Information for %s", BSTD_FUNCTION, ifr.ifr_name));
            goto exitApp;
        }
        BDBG_WRN(("############## Server Listening on IP Address:Port %s:%d ###############", inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr), ipStreamerGlobalCfg.listeningPort));
    }

    while (!gExitThread) {
        /* main thread waits until app is asked to stop */
#ifdef B_USE_HTTP_KEEPALIVE
        BERR_Code rc;
#endif
        if (gGotSigInt) {
            BDBG_WRN(("Exiting from main thread, requesting children threads to exit as well"));
            gExitThread = 1;
            break;
        }
#ifdef B_USE_HTTP_KEEPALIVE
        rc = BKNI_WaitForEvent(ipStreamerDestroySession, 1000);
        if (rc == BERR_TIMEOUT)
            continue;
        else if (rc!=0)
        {
            BDBG_WRN(("%s: got error while waiting for ipStreamerDestroySession event, rc %d", BSTD_FUNCTION, rc));
            continue;
        }
        /* otherwise, destroy the ipStreamer Session */
        stopAndCloseSession(ipStreamerGlobalCtx);
#else
        BKNI_Sleep(1000);
#endif
    }
    /* app is being shutdown, do cleanup */

#ifdef B_USE_HTTP_KEEPALIVE
    /* destroy any remaining ipStreamer Sessions */
    stopAndCloseSession(ipStreamerGlobalCtx);
#endif
    /* wait for children thread to complete */
    for (i=0; i<ipStreamerGlobalCfg.numStreamingSessions; i++) {
        if (pthread_join(gThreadId[i], NULL)) {
            BDBG_ERR(("Failed to join pthread, errno %d", errno));
            goto exitApp;
        }
    }

    if ( gChannelMapThreadId ) {
        pthread_join(gChannelMapThreadId, NULL);
    }

#ifdef LIVEMEDIA_SUPPORT
#if LIVEMEDIA_2013_03_07
    if ( gRTSPServerThreadId ) {
        /* CAD/BRCM 20121031 - Added RTSPServer support */
        BDBG_WRN(("Exiting from rtsp server thread (SIGINT)."));
        pthread_kill(gRTSPServerThreadId, SIGINT);
    }
#endif /*LIVEMEDIA_2013_03_07*/
#endif /* LIVEMEDIA_SUPPORT */

    if (ipStreamerGlobalCfg.streamingCfg[0].streamingProtocol == B_PlaybackIpProtocol_eHttp)
        unInitTcpServer(ipStreamerGlobalCtx->listeningFd);

#ifdef B_USE_HTTP_KEEPALIVE
    if (ipStreamerDestroySession)
        BKNI_DestroyEvent(ipStreamerDestroySession);
#endif
    if (ipStreamerCtxQueueMutex)
        BKNI_DestroyMutex(ipStreamerCtxQueueMutex);

    if(hUdpInitMutex)
        BKNI_DestroyMutex(hUdpInitMutex);

    appExitCode = 0;
error:
    B_IpStreamer_UnInit(ipStreamerGlobalCtx);
exitApp:
    {
        FILE *fp;
        char buf[16];
        int len;
        fp = fopen("./result.txt", "w+");
        if (fp) {
            len = snprintf(buf, 15, "%d", appExitCode);
            fwrite(buf, 1, len, fp);
            fclose(fp);
        }
        else {
            printf("Failed to open ./result.txt file\n");
        }
    }
    return 0;
}
