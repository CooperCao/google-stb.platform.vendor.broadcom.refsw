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

#include "../include/b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_aes.h"
#include "b_playback_ip_utils.h"
#include "b_playback_ip_lm_helper.h"
#include "nexus_playback.h"
#include "nexus_audio_decoder_trick.h"
#include "nexus_core_utils.h"
#include "bid3_parser.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <ctype.h>

BDBG_MODULE(b_playback_ip_hls);

/* Overall flow
   -Browser obtains the URI of a AV stream by parsing the html page (video tag for html5)
    -URI points to a M3U8 format playlist file which contains an ordered list of media URIs & informational tags
    -Each media URI refers to a media file which is a segment of of a single continous stream
   -Browser passes the playlist URI to the IP library
   -IP library sends request to the server to obtain the playlist file and parses it
*/
/* Mandatory M3U8 tags */
#define HLS_M3U8_BEGIN_TAG                      "#EXTM3U"
#define HLS_M3U8_URL_TAG                        "#EXTINF:"    /* preceeds each media file URI entry, contains media file duration in seconds */
#define HLS_M3U8_MAX_MEDIA_SEGMENT_DURATION_TAG "#EXT-X-TARGETDURATION:"     /* maximum duation in seconds of any media file in a playlist */
#define HLS_M3U8_END_TAG                        "#EXT-X-ENDLIST"             /* indicated no more media files will be added to a playlist file */

/* Optional M3U8 tags */
#define HLS_M3U8_VERSION_TAG                    "#EXT-X-VERSION:"           /* version of the playlist file, if missing, assumed 1 */
#define HLS_M3U8_MEDIA_SEGMENT_SEQUENCE_TAG     "#EXT-X-MEDIA-SEQUENCE:"    /* sequence number of the 1st URI in a playlist, if missing, then 1st URI has seq # of 0 */
#define HLS_M3U8_STREAM_INFO_TAG                "#EXT-X-STREAM-INF:"        /* preceeds URI of a playlist file */
#define HLS_M3U8_I_FRAME_STREAM_INFO_TAG        "#EXT-X-I-FRAME-STREAM-INF:"        /* I-Frame Media Playlist tag entry. */
#define HLS_M3U8_MEDIA_TAG                      "#EXT-X-MEDIA:"             /* media tag containing alternate rendition of a variant. */
#define HLS_M3U8_DISCONTINUITY_TAG              "#EXT-X-DISCONTINUITY"      /* indicates discontinuity between the media file that follows this tag to the one before it */
#define HLS_M3U8_ENCRYPTION_KEY_TAG             "#EXT-X-KEY:"               /* indicates following segments are encrypted using this key & IV */
#define HLS_M3U8_PROGRAM_DATE_TIME_TAG          "#EXT-X-PROGRAM-DATE-TIME:" /* preceeds a URI tag */
#define HLS_M3U8_PROGRAM_RECORD_TAG             "#EXT-X-ALLOW-CACHE:"       /* if YES, client can optionally record the media files */
#define HLS_M3U8_BYTERANGE_TAG                  "#EXT-X-BYTERANGE:"         /* if YES, client can optionally record the media files */

#define PLAYLIST_FILE_COMPATIBILITY_VERSION 4

#define HLS_M3U8_TAG_ATTRIBUTE_PROGRAM_ID   "PROGRAM-ID"
#define HLS_M3U8_TAG_ATTRIBUTE_BANDWIDTH   "BANDWIDTH"
#define HLS_M3U8_TAG_ATTRIBUTE_CODECS       "CODECS"
#define HLS_M3U8_TAG_ATTRIBUTE_RESOLUTION   "RESOLUTION"
#define HLS_M3U8_TAG_ATTRIBUTE_ENC_METHOD   "METHOD"
#define HLS_M3U8_TAG_ATTRIBUTE_ENC_METHOD_NONE   "NONE"
#define HLS_M3U8_TAG_ATTRIBUTE_ENC_METHOD_AES128   "AES-128"
#define HLS_M3U8_TAG_ATTRIBUTE_ENC_URI      "URI"
#define HLS_M3U8_TAG_ATTRIBUTE_ENC_IV       "IV"
#define HLS_M3U8_TAG_ATTRIBUTE_AUDIO        "AUDIO"
#define HLS_M3U8_TAG_ATTRIBUTE_URI          "URI"
#define HLS_M3U8_TAG_ATTRIBUTE_VIDEO        "VIDEO"
#define HLS_M3U8_TAG_ATTRIBUTE_SUBTITILES   "SUBTITLES"

/* Attributes associated w/ the MEDIA_TAG */
/* Example is:
#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="group01",NAME="eng",DEFAULT=YES,AUTOSELECT=YES,LANGUAGE="en",URI="07.m3u8"
*/
#define HLS_M3U8_TAG_ATTRIBUTE_TYPE         "TYPE"
#define HLS_M3U8_TAG_ATTRIBUTE_GROUP_ID     "GROUP-ID"
#define HLS_M3U8_TAG_ATTRIBUTE_NAME         "NAME"
#define HLS_M3U8_TAG_ATTRIBUTE_DEFAULT      "DEFAULT"
#define HLS_M3U8_TAG_ATTRIBUTE_AUTOSELECT   "AUTOSELECT"
#define HLS_M3U8_TAG_ATTRIBUTE_LANGUAGE     "LANGUAGE"
#define HLS_M3U8_TAG_ATTRIBUTE_ASSOC_LANGUAGE     "ASSOC-LANGUAGE"
#define HLS_M3U8_TAG_ATTRIBUTE_URI          "URI"

#define B_PLAYBACK_IP_MAX_SESSION_OPEN_RETRY            12
#define B_PLAYBACK_IP_SESSION_OPEN_MIN_TIMEOUT          250
#define B_PLAYBACK_IP_SESSION_OPEN_MAX_TIMEOUT          1000
#define B_PLAYBACK_IP_HTTP_STATUS_SERVICE_UNAVAILABLE   503

#define HLS_PLAYLIST_FILE_SIZE (3*1024*1024)
#define HLS_EVENT_TIMEOUT_MSEC 5000 /* 5 secs */
#define HLS_EVENT_BUFFER_TIMEOUT_MSEC 300 /* Lower this to help w/ the seek & other trickmode startup latency! */
#define HLS_READ_CHUNK_SIZE (TS_PKT_SIZE*HTTP_AES_BLOCK_SIZE*10)

/* extern declarations */
extern int playback_ip_read_socket( B_PlaybackIpHandle playback_ip, void *securityHandle, int fd, char *buf, int buf_size, int timeout);
ssize_t B_PlaybackIp_HttpNetRangeReq( B_PlaybackIpHandle playback_ip, void *buf, size_t length, off_t byteRangeStart, off_t byteRangeEnd, int prevFd, int *newFd);
bool http_absolute_uri(char *url);
int B_PlaybackIp_UtilsGetLine(char **linePtr, int *linePtrLength, char *buffer, int bufferLength);
NEXUS_TransportType http_get_payload_content_type(char *http_hdr);
int B_PlaybackIp_SecuritySessionOpen( B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionOpenSettings *openSettings, int fd, void **newSecurityHandle);
extern int http_read_response(B_PlaybackIpHandle playback_ip, void *securityHandle, int fd, char **rbufp, int rbuf_size, char **http_hdr, char **http_payload, int *payload_len);
extern http_url_type_t http_get_url_type(char *http_hdr, char *url);
extern B_PlaybackIpError B_PlaybackIp_HttpGetCurrentPlaybackPosition( B_PlaybackIpHandle playback_ip, NEXUS_PlaybackPosition *currentPosition);

#if 0
#define HLS_UNIT_TEST
#endif

#ifdef HLS_UNIT_TEST
static void
dump_playlist_file(HlsSessionState *hlsSession)
{
    static FILE * playlistFile = NULL;
    playlistFile = fopen("/data/videos/playlist.m3u8", "ab+");
    fwrite(hlsSession->playlistBuffer, 1, hlsSession->playlistBufferDepth , playlistFile);
    fflush(playlistFile);
    fclose(playlistFile);
    BDBG_MSG(("%s: wrote downloaded M3U playlist file into file /data/videos/playlist.m3u8", BSTD_FUNCTION));
}
#endif

int
hls_asciiHexToDec(char ch, unsigned char *num)
{
    if (isdigit(ch))
        *num = ch - '0';
    else if (isxdigit(ch) && ch >= 'a')
        *num = 9 + ch - 'a' + 1;
    else if (isxdigit(ch) && ch >= 'A')
        *num = 9 + ch - 'A' + 1;
    else
        return false;
    return true;
}

static unsigned
myExp(unsigned power, unsigned base)
{
    unsigned i;
    unsigned exp;

    for (i=0, exp=1; i<power; i++) {
        exp *= base;
    }
    //printf("power %u, base %u, exp %u\n", power, base, exp);
    return exp;
}

static unsigned
convertStringToMsec(char *pStrMsec)
{
    unsigned msec = 0;
    int i;
    unsigned num;

#define POSITION 3
    /* we are only interested in 1st 3 digits in the string, ignore the rest microsecond part of the resolution */
    for (i=0; i<POSITION && (*pStrMsec != '\0'); pStrMsec++, i++)
    {
        num = *pStrMsec - '0';
        msec += num * myExp(POSITION-1-i, 10);
    }
    return msec;
}

static NEXUS_PlaybackPosition
convertDurationAttributeToMsec(char *duration)
{
    NEXUS_PlaybackPosition msec;
    char *tmp1;

    tmp1 = strstr(duration, ".");
    if (tmp1 == NULL) {
        /* just a simple integer */
        msec = 1000 * strtol(duration, NULL, 10);
    }
    else {
        *tmp1 = '\0';
        /* convert integer part to msec */
        msec = 1000 * strtol(duration, NULL, 10);
        /* now convert fractional part to msec */
        msec += convertStringToMsec(tmp1+1);
    }
    BDBG_MSG(("%s: msec is %lu", BSTD_FUNCTION, msec));
    return msec;
}

bool
B_PlaybackIp_IsHlsSession (
    char *playlistUri,
    char *httpResponseHeader
    )
{
    char *contentType;
    /*
       Playlist file can be either UTF-8 US-ASCII encoded. Either file name or HTTP Content-type will indicate it be a playlist file
       a.   UTF-8: if file name ends w/ .m3u8 and/or HTTP Content-type is "application/vnd.apple.mpegurl"
       b.   US-ASCII: if file name ends w/ .m3u and/or HTTP Content-type is either "application/x-mpegURL" or "audio/mpegurl"
       c.   Note: need to understand difference in parsing between UTF-8 & ASCII

    */
    if (B_PlaybackIp_UtilsStristr(playlistUri, "m3u") != NULL) {
        BDBG_MSG(("%s: M3U encoded URI: %s", BSTD_FUNCTION, playlistUri));
        return true;
    }

    if ((contentType = B_PlaybackIp_UtilsStristr(httpResponseHeader, "Content-Type: ")) != NULL) {
        contentType += strlen("Content-Type: ");

        if (B_PlaybackIp_UtilsStristr(contentType, "application/vnd.apple.mpegurl") ||
            B_PlaybackIp_UtilsStristr(contentType, "application/x-mpegurl") ||
            B_PlaybackIp_UtilsStristr(contentType, "audio/x-mpegurl") ||
            B_PlaybackIp_UtilsStristr(contentType, "audio/mpegurl")
            ) {
            BDBG_MSG(("%s: M3U encoded URI: %s, contentType %s", BSTD_FUNCTION, playlistUri, contentType));
            return true;
        }
    }

    /* neither uri name nor content type indicates that the session is HTTP Live Streaming session */
    return false;
}

char *
B_PlaybackIp_HlsBuildAbsoluteUri(char *server, int port, char *baseUri, char *fileName)
{
    int uriLength = 0;
    char portString[16] = {0,};
    char *uri = NULL;
    char *tmp1, *tmp2 = NULL;
    int baseUriLength = 0;
    char *baseUriCopy = NULL;

    if ( !server || !fileName ) {
        return NULL;
    }

    /* determine the # of char for the port */
    memset(portString, 0, sizeof(portString));
    snprintf(portString, sizeof(portString)-1, "%d", port);

    if (fileName[0] != '/') {
        /* relative uri fileName doesn't start w/ /, so we need to use path from the base URI */
        tmp1 = baseUri;
        while ((tmp1 = strstr(tmp1, "/")) != NULL) {
            tmp2 = tmp1; /* note location of next directory path */
            tmp1 += 1; /* move past the / char */
        }
        if (tmp2) {
            baseUriLength = tmp2 - baseUri + 1 + 1; /* one for / char, one for NULL char */
            if ((baseUriCopy = (char *)BKNI_Malloc(baseUriLength)) == NULL) {
                BDBG_ERR(("%s: ERROR: failed to allocate %d bytes of memory at %d\n", BSTD_FUNCTION, baseUriLength, __LINE__));
                return NULL;
            }
            BKNI_Memset(baseUriCopy, 0, baseUriLength);
            strncpy(baseUriCopy, baseUri, baseUriLength-1);
            baseUriCopy[baseUriLength-1] = '\0';
        }
    }
    else {
        baseUriLength = 0; /* nothing to use from the baseUri as relative URI is relative to just the root server name & port # */
    }

    /* now allocate space for holding the absolute URI */
    uriLength = strlen(server) + strlen(portString) + baseUriLength + strlen(fileName) + 11; /* extra characters for http header */
    if ((uri = (char *)BKNI_Malloc(uriLength)) == NULL) {
        if (baseUriCopy)
            BKNI_Free(baseUriCopy);
        BDBG_ERR(("%s: Failed to allocate %d bytes of memory for building uri", BSTD_FUNCTION, uriLength));
        return NULL;
    }
    BKNI_Memset(uri, 0, uriLength);
    uri[uriLength-1] = '\0';
    snprintf(uri, uriLength-1, "%s://%s:%s%s%s", (port == 443 ? "https":"http"), server, portString, baseUriLength ? baseUriCopy : "", fileName);
    BDBG_MSG(("%s: server %s port %s base uri %s, file %s, Absolute uri %s", BSTD_FUNCTION, server, portString, baseUriLength ? baseUri : "", fileName, uri));
    if (baseUriCopy)
        BKNI_Free(baseUriCopy);
    return uri;
}

/* download file (either playlist or decryption key) from the current network socket into the specified buffer and return the amount of bytes read. */
/* read until EOF, error condition, or channel change (state change) occurs */
bool
B_PlaybackIp_HlsDownloadFile(B_PlaybackIpHandle playback_ip, int *pFd, char *buffer, int bufferSize, int *totalBytesRead, bool nullTerminate)
{
    int fd;
    ssize_t bytesRead = 0;
    int bytesToRead;
    bool serverClosed = false;
    int timeoutRetryCount = 0;

    fd = *pFd;
    BDBG_MSG(("%s: start downloading a playlist, currently read %d", BSTD_FUNCTION, *totalBytesRead));
    while (true) {
        if ( breakFromLoop(playback_ip)) {
            BDBG_MSG(("%s: breaking file download loop due to state (%d) change", BSTD_FUNCTION, playbackIpState(playback_ip)));
            break;
        }

        if (playback_ip->contentLength > 0 && *totalBytesRead == (int)playback_ip->contentLength) {
            /* we have read all the bytes that server had indicated via contentLength, so instead of trying another read and waiting for server to close the connection */
            /* consider this as server closed event and break out of the read loop */
            BDBG_MSG(("%s: breaking out of read loop as we have read %d upto the content length %"PRId64 "", BSTD_FUNCTION, *totalBytesRead, playback_ip->contentLength));
            serverClosed = true;
            break;
        }
        bytesToRead = bufferSize > HLS_READ_CHUNK_SIZE ? HLS_READ_CHUNK_SIZE : bufferSize;
        /* make sure there is enough space in the read buffer */
        if ((*totalBytesRead + bytesToRead) > bufferSize) {
            BDBG_MSG(("%s: need bigger buffer to hold the complete downloaded file: totalBytesRead %d, size %d, returning what is read", BSTD_FUNCTION, *totalBytesRead, bufferSize));
            break;
        }

        if ((bytesRead = playback_ip_read_socket(playback_ip, playback_ip->securityHandle, fd, buffer+*totalBytesRead, bytesToRead, playback_ip->networkTimeout)) <= 0) {
            if (playback_ip->selectTimeout) {
                BDBG_MSG(("%s:%p socket error, retry read: size %zu, errno :%d, state %d, select timeout %d, server closed %d",
                    BSTD_FUNCTION, (void *)playback_ip, *totalBytesRead+bytesRead, errno, playbackIpState(playback_ip), playback_ip->selectTimeout, playback_ip->serverClosed));
                if ( timeoutRetryCount > 1 ) {
                    serverClosed = true;
                    BDBG_ERR(("!!! %s: timeoutRetryCount = %d, timing out while downloading the file, breaking out!", BSTD_FUNCTION, timeoutRetryCount));
                    break;
                }
                ++timeoutRetryCount;
                continue;
            }
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_ERR(("%s: Network Read Error, rc %zu, playback ip state %d", BSTD_FUNCTION, bytesRead, playbackIpState(playback_ip)));
#endif
            serverClosed = true;
            break;
        }

        /* read some data, increament the index count and make sure there is space left in the index cache for the next read */
        *totalBytesRead += bytesRead;
    }
    BDBG_MSG(("%s:%p finished downloading playlist: errno %d, size %d, state %d, server closed %d on socket %d", BSTD_FUNCTION, (void *)playback_ip, errno, *totalBytesRead, playbackIpState(playback_ip), serverClosed, fd));
    if (serverClosed) {
        /* close security context and socket */
        if (playback_ip->securityHandle) {
            playback_ip->netIo.close(playback_ip->securityHandle);
            playback_ip->securityHandle = NULL;
        }
        close(fd);
        *pFd = -1;
    }

    /* downloaded the file into the buffer, return success */
    if (*totalBytesRead) {
        if (*totalBytesRead < bufferSize) {
            if (nullTerminate)
                buffer[*totalBytesRead] = '\0'; /* null terminate the read playlist */
        }
        else {
            BDBG_ERR(("%s: increase the playlist buffer size from %d, it is not big enough", BSTD_FUNCTION, bufferSize));
            return false;
        }
        return true;
    }
    else
        return false;
}

static inline int min(int a, int b)
{
     return (a) < (b) ? (a) : (b);
}

/*****************************************************************************
 * Reset the bandwidth accumulator.
 *****************************************************************************/
static void B_PlaybackIp_HlsResetBandwidthSample(HlsNetworkBandwidthContext  * bandwidthContext, unsigned long sampleLimit)
{
    /* Zero out everything... */
    BKNI_Memset(bandwidthContext, 0, sizeof (*bandwidthContext));

    /* Then set the sample limit as requested. */
    bandwidthContext->sampleLimit = sampleLimit;
}

/*****************************************************************************
 * Add a bandwidth sample to the bandwidth accumulator.
 *****************************************************************************/
static void B_PlaybackIp_HlsAddBandwidthSample(HlsNetworkBandwidthContext  * bandwidthContext, unsigned long timeInMs, unsigned long bytes)
{

    if (timeInMs == 0 || bytes == 0)
        return;
    if (bandwidthContext->sampleLimit == 0 ) bandwidthContext->sampleLimit = 1;

    if (bandwidthContext->accumulatedSamples < bandwidthContext->sampleLimit) {

        bandwidthContext->accumulatedSamples++;
    }
    else {

        bandwidthContext->accumulatedBytes    -= bandwidthContext->accumulatedBytes    / bandwidthContext->sampleLimit;
        bandwidthContext->accumulatedTimeInMs -= bandwidthContext->accumulatedTimeInMs / bandwidthContext->sampleLimit;
        bandwidthContext->accumulatedTimeWeightedKBps -= bandwidthContext->accumulatedTimeWeightedKBps / bandwidthContext->sampleLimit;
        bandwidthContext->accumulatedByteWeightedKBps -= bandwidthContext->accumulatedByteWeightedKBps / bandwidthContext->sampleLimit;
    }

    bandwidthContext->accumulatedTimeInMs += timeInMs;
    bandwidthContext->accumulatedBytes    += bytes;
    bandwidthContext->accumulatedTimeWeightedKBps += bytes;     /* actually: timeInMs * (bytes / timeInMs) */
    bandwidthContext->accumulatedByteWeightedKBps += bytes * (bytes / timeInMs);

#if 0  /* ==================== Original Algorithm (time weighted)  ==================== */
    bandwidthContext->filteredBandwidthInKbitsPerSecond = (bandwidthContext->accumulatedBytes / bandwidthContext->accumulatedTimeInMs) * 8;
#elif 1  /* ==================== Alternate Algorithm (time weighted)  ==================== */
    bandwidthContext->filteredBandwidthInKbitsPerSecond = (bandwidthContext->accumulatedTimeWeightedKBps / bandwidthContext->accumulatedTimeInMs) * 8;
#elif 0  /* ==================== Alternate Algorithm (bytecount weighted)  ==================== */
    bandwidthContext->filteredBandwidthInKbitsPerSecond = (bandwidthContext->accumulatedByteWeightedKBps / bandwidthContext->accumulatedTimeInMs) * 8;
#endif

    BDBG_MSG(("%s : %d : Adding BW sample: timeInMs: %lu, bytes: %lu Kbps: %lu Averaged tw Kbps: %lu, bw kbps %lu",
                      BSTD_FUNCTION, __LINE__,
                      timeInMs,
                      bytes,
                      (bytes * 8) / timeInMs,
                      bandwidthContext->filteredBandwidthInKbitsPerSecond,
                      (bandwidthContext->accumulatedByteWeightedKBps / bandwidthContext->accumulatedTimeInMs) * 8
                      ));
}

/*****************************************************************************
 * Get the current average bandwidth.
 *****************************************************************************/
static unsigned long B_PlaybackIp_HlsGetCurrentBandwidth(HlsNetworkBandwidthContext  * bandwidthContext)
{
    return bandwidthContext->filteredBandwidthInKbitsPerSecond*1000;
}

/* download data from the current network socket into the specified buffer and return the amount of bytes read. */
/* read until bytesToRead bytes are read, EOF, error condition, or channel change (state change) occurs */
/* in addition, also measure the network bandwidth while downloading this data. */
/* current socket (accessed via playback_ip struct) can correspond to playlist file or actual media segment */
bool
B_PlaybackIp_HlsDownloadMediaSegment(
    B_PlaybackIpHandle playback_ip,
    int fd,
    char *buffer,
    int bufferSize,
    int bytesToRead,
    int *totalBytesRead,
    unsigned *networkBandwidth,
    bool *serverClosed
    )
{
    ssize_t bytesRead = 0;
    B_Time beginTime, endTime, curTime;
    unsigned int totalDownloadTime;
    int bytesLeftToRead = 0;

    if (bytesToRead <= 0) {
        BDBG_ERR(("%s: invalid bytesToRead %d for fd %d", BSTD_FUNCTION, bytesToRead, fd));
        return false;
    }
    *serverClosed = false;
    BDBG_MSG(("%s:%p download %d bytes for fd %d, bufferSize %d", BSTD_FUNCTION, (void *)playback_ip, bytesToRead, fd, bufferSize));
    /* start a timer to note the n/w b/w */
    B_Time_Get(&beginTime);
    while (true) {
        if ( breakFromLoop(playback_ip) || playback_ip->playback_state == B_PlaybackIpState_eEnteringTrickMode) {
            BDBG_MSG(("%s: breaking file download loop due to state ", BSTD_FUNCTION));
            break;
        }

        /* make sure there is enough space in the read buffer */
        bytesLeftToRead = min((bytesToRead - *totalBytesRead), (bufferSize - *totalBytesRead));
        /* futher limit the readSize to HLS read chunk size */
        bytesLeftToRead = min(bytesLeftToRead, HLS_READ_CHUNK_SIZE);
        if (bytesLeftToRead <= 0) {
            BDBG_MSG(("%s:%p breaking out of read loop as we have no more to read: bytesToRead %d, bytesRead %d, bufferSize %d, bytesLeftToRead %d", BSTD_FUNCTION, (void *)playback_ip,  bytesToRead, *totalBytesRead, bufferSize, bytesLeftToRead));
            break;
        }

        BDBG_MSG(("%s:%p: before playback_ip_read_socket: networkTimeout %d", BSTD_FUNCTION, (void *)playback_ip, playback_ip->networkTimeout));
        if ((bytesRead = playback_ip_read_socket(playback_ip, playback_ip->securityHandle, fd, buffer+*totalBytesRead, bytesLeftToRead, playback_ip->networkTimeout)) <= 0) {
            if (playback_ip->selectTimeout) {
                BDBG_WRN(("%s: select timeout, giving up reading on this segment: size %zu, errno :%d, state %d, select timeout %d, server closed %d",
                            BSTD_FUNCTION, *totalBytesRead+bytesRead, errno, playbackIpState(playback_ip), playback_ip->selectTimeout, playback_ip->serverClosed));
            }
            else {
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog)
                    BDBG_ERR(("%s:%p: Network Read Error, errno %d, serverClosed %d, bytesRead %zu, playback ip state %d", BSTD_FUNCTION, (void *)playback_ip, errno, playback_ip->serverClosed, bytesRead, playbackIpState(playback_ip)));
#endif
                *serverClosed = true;
            }
            break;
        }
#define MAX_SEGMENT_DOWNLOAD_TIME 7000
        /* read some data, increament the index count and make sure there is space left in the index cache for the next read */
        *totalBytesRead += bytesRead;
        B_Time_Get(&curTime);
        BDBG_MSG_FLOW(("%s:%p total bytesToRead %d, totalbytesRead %d, bytesToRead %d, bytes read %zu", BSTD_FUNCTION, (void *)playback_ip,
                    bytesToRead, *totalBytesRead, bytesLeftToRead, bytesRead));
        totalDownloadTime = B_Time_Diff(&curTime, &beginTime);
        if (totalDownloadTime > MAX_SEGMENT_DOWNLOAD_TIME) {
            /* we have read all the bytes that server had indicated via contentLength, so instead of trying another read and waiting for server to close the connection */
            /* consider this as server closed event and break out of the read loop */
            BDBG_MSG(("%s: breaking out of read loop as download time (msec) %u exceeded %d, total read %d", BSTD_FUNCTION, totalDownloadTime, MAX_SEGMENT_DOWNLOAD_TIME, *totalBytesRead));
            break;
        }
    }

    B_Time_Get(&endTime);
    totalDownloadTime = B_Time_Diff(&endTime, &beginTime);
    /* Add this bandwidth sample to the bandwidth accumulator */
    B_PlaybackIp_HlsAddBandwidthSample(&playback_ip->hlsSessionState->bandwidthContext, totalDownloadTime, *totalBytesRead);
    if (totalDownloadTime) {
        unsigned modifiedNetworkBandwidth;
        *networkBandwidth = ((int)((*totalBytesRead*8.)/(totalDownloadTime))*1000);
        modifiedNetworkBandwidth = (*networkBandwidth * 80)/100;
        /* 1000 for convering msec to sec, 80/100 for taking 80% of current n/w b/w */
        BDBG_MSG(("%s:%p download time (msec) %u, total read %d, b/w %u, modified b/w %u", BSTD_FUNCTION, (void *)playback_ip, totalDownloadTime, *totalBytesRead, *networkBandwidth, modifiedNetworkBandwidth));
        *networkBandwidth = modifiedNetworkBandwidth;
    }
    BDBG_MSG(("%s:%p finished downloading file (fd %d): n/w bandwidth %d, errno %d, size %d, state %d, select timeout %d, server closed %d",
            BSTD_FUNCTION, (void *)playback_ip, fd, *networkBandwidth, errno, *totalBytesRead, playbackIpState(playback_ip), playback_ip->selectTimeout, *serverClosed));

    if (*totalBytesRead)
        return true;
    else
        return false;
}

bool
B_PlaybackIp_HlsBoundedStream(B_PlaybackIpHandle playback_ip)
{
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;

    if (hlsSession && hlsSession->currentPlaylistFile && hlsSession->currentPlaylistFile->bounded)
        return true;
    else
        return false;
}

bool
B_PlaybackIp_IsPlaylistInExtendedM3uFormat(B_PlaybackIpHandle playback_ip)
{
    int rc;
    char *nextLine = NULL;
    int nextLineLength = 0;
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;

    /* based on the url extension or content type, we had assumed that we have a M3U playlist file in the index cache */
    /* HTTP Live Streaming protocol requires this file to be in the Extended M3U format */

    /* 1st line must be HLS_M3U8_BEGIN_TAG */
    if ((rc =B_PlaybackIp_UtilsGetLine(&nextLine, &nextLineLength, hlsSession->playlistBuffer, hlsSession->playlistBufferDepth)) > 0) {
        if (B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_BEGIN_TAG)) {
            BDBG_MSG(("%s: playlist is Extended M3U compliant, nextline %s, len %d, rc %d", BSTD_FUNCTION, nextLine, nextLineLength, rc));
            hlsSession->playlistBufferReadIndex += rc;
            BKNI_Free(nextLine);
            return true;
        }
    }
    BDBG_ERR(("%s: playlist is not Extended M3U compliant, missing %s tag, nextline %s, len %d", BSTD_FUNCTION, HLS_M3U8_BEGIN_TAG, nextLine, nextLineLength));

    if (nextLine)
        BKNI_Free(nextLine);
    return false;
}

void
B_PlaybackIp_FreeMediaFileSegmentInfo(MediaFileSegmentInfo *mediaFileSegmentInfo)
{
    if (mediaFileSegmentInfo->absoluteUri) {
        BKNI_Free(mediaFileSegmentInfo->absoluteUri);
        mediaFileSegmentInfo->absoluteUri = NULL;
    }
    if (mediaFileSegmentInfo->server) {
        BKNI_Free(mediaFileSegmentInfo->server);
        mediaFileSegmentInfo->server = NULL;
    }
    BDBG_MSG(("%s: freed mediaFileSegmentInfo entry %p", BSTD_FUNCTION, (void *)mediaFileSegmentInfo));
    BKNI_Free(mediaFileSegmentInfo);
}

void
B_PlaybackIp_FreeMediaFileSegmentInfoAll(struct MediaFileSegmentInfoQueue *mediaFileSegmentInfoQueueHead)
{
    MediaFileSegmentInfo *mediaFileSegmentInfo;
    for (mediaFileSegmentInfo = BLST_Q_FIRST(mediaFileSegmentInfoQueueHead);
         mediaFileSegmentInfo;
         mediaFileSegmentInfo = BLST_Q_FIRST(mediaFileSegmentInfoQueueHead))
    {
        BLST_Q_REMOVE_HEAD(mediaFileSegmentInfoQueueHead, next);
        B_PlaybackIp_FreeMediaFileSegmentInfo(mediaFileSegmentInfo);
    }

    BDBG_MSG(("%s: done", BSTD_FUNCTION));
}

void
B_PlaybackIp_FreePlaylistInfo(PlaylistFileInfo *playlistFileInfo)
{

    if (playlistFileInfo == NULL) return;
    B_PlaybackIp_FreeMediaFileSegmentInfoAll(&playlistFileInfo->mediaFileSegmentInfoQueueHead);

    if (playlistFileInfo->absoluteUri) {
        BKNI_Free(playlistFileInfo->absoluteUri);
        playlistFileInfo->absoluteUri = NULL;
    }
    if (playlistFileInfo->server) {
        BKNI_Free(playlistFileInfo->server);
        playlistFileInfo->server = NULL;
    }
    if (playlistFileInfo->altAudioRenditionGroupId) {
        BKNI_Free(playlistFileInfo->altAudioRenditionGroupId);
        playlistFileInfo->altAudioRenditionGroupId= NULL;
    }

    BDBG_MSG(("%s: freed playlistFileInfo entry %p", BSTD_FUNCTION, (void *)playlistFileInfo));
    BKNI_Free(playlistFileInfo);
}

void
B_PlaybackIp_FreeAltRenditionInfo(AltRenditionInfo *altRenditionInfo)
{

    if (altRenditionInfo->absoluteUri) {
        BKNI_Free(altRenditionInfo->absoluteUri);
        altRenditionInfo->absoluteUri = NULL;
    }
    if (altRenditionInfo->server) {
        BKNI_Free(altRenditionInfo->server);
        altRenditionInfo->server = NULL;
    }
    if (altRenditionInfo->groupId) {
        BKNI_Free(altRenditionInfo->groupId);
        altRenditionInfo->groupId= NULL;
    }
    if (altRenditionInfo->name) {
        BKNI_Free(altRenditionInfo->name);
        altRenditionInfo->name= NULL;
    }
    if (altRenditionInfo->language) {
        BKNI_Free(altRenditionInfo->language);
        altRenditionInfo->language= NULL;
    }
    if (altRenditionInfo->assocLanguage) {
        BKNI_Free(altRenditionInfo->assocLanguage);
        altRenditionInfo->assocLanguage= NULL;
    }

    BDBG_MSG(("%s: freed altRenditionInfo entry %p", BSTD_FUNCTION, (void *)altRenditionInfo));
    BKNI_Free(altRenditionInfo);
}

#if 0
#define MY_BLST_Q_P_TEST_HEAD(node, head, field) do { BDBG_ASSERT((node)->field.l_head == (const void *)(head)); }while(0)
#define MY_BLST_Q_P_SET_HEAD(node, head, field) do { BDBG_ASSERT((node)->field.l_head = (const void *)(head)); }while(0)
void
B_PlaybackIp_WalkThruPlaylistInfo(PlaylistFileInfo *playlistFileInfo)
{
    struct MediaFileSegmentInfoQueue *mediaFileSegmentInfoQueueHead;
    MediaFileSegmentInfo *mediaFileSegmentInfo;

    mediaFileSegmentInfoQueueHead = &playlistFileInfo->mediaFileSegmentInfoQueueHead;
    mediaFileSegmentInfo = BLST_Q_FIRST(mediaFileSegmentInfoQueueHead);
    while (mediaFileSegmentInfo != NULL) {
        MY_BLST_Q_P_TEST_HEAD(mediaFileSegmentInfo, mediaFileSegmentInfoQueueHead, next);
        mediaFileSegmentInfo = BLST_Q_NEXT(mediaFileSegmentInfo, next);
    }
}

void
B_PlaybackIp_WalkThruPlaylistInfoAll(B_PlaybackIpHandle playback_ip)
{
    PlaylistFileInfo *playlistFileInfo;
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;

    playlistFileInfo = BLST_Q_FIRST(&hlsSession->playlistFileInfoQueueHead);
    while (playlistFileInfo != NULL) {
        B_PlaybackIp_WalkThruPlaylistInfo(playlistFileInfo);
        playlistFileInfo = BLST_Q_NEXT(playlistFileInfo, next);
    }
    BDBG_WRN(("%s: done", BSTD_FUNCTION));
}
#endif

void
B_PlaybackIp_FreePlaylistInfoAll(B_PlaybackIpHandle playback_ip)
{
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;
    PlaylistFileInfo *playlistFileInfo;
    AltRenditionInfo *altRenditionInfo;

    for (playlistFileInfo = BLST_Q_FIRST(&hlsSession->playlistFileInfoQueueHead);
         playlistFileInfo;
         playlistFileInfo = BLST_Q_FIRST(&hlsSession->playlistFileInfoQueueHead))
    {
        BLST_Q_REMOVE_HEAD(&hlsSession->playlistFileInfoQueueHead, next);
        B_PlaybackIp_FreePlaylistInfo(playlistFileInfo);
    }

    for (altRenditionInfo = BLST_Q_FIRST(&hlsSession->altRenditionInfoQueueHead);
         altRenditionInfo;
         altRenditionInfo = BLST_Q_FIRST(&hlsSession->altRenditionInfoQueueHead))
    {
        BLST_Q_REMOVE_HEAD(&hlsSession->altRenditionInfoQueueHead, next);
        B_PlaybackIp_FreeAltRenditionInfo(altRenditionInfo);
    }

    for (playlistFileInfo = BLST_Q_FIRST(&hlsSession->iFramePlaylistFileInfoQueueHead);
         playlistFileInfo;
         playlistFileInfo = BLST_Q_FIRST(&hlsSession->iFramePlaylistFileInfoQueueHead))
    {
        BLST_Q_REMOVE_HEAD(&hlsSession->iFramePlaylistFileInfoQueueHead, next);
        B_PlaybackIp_FreePlaylistInfo(playlistFileInfo);
    }

    if (hlsSession->playlistBuffer) {
        BKNI_Free(hlsSession->playlistBuffer);
        hlsSession->playlistBuffer = NULL;
    }
    BDBG_MSG(("%s: done", BSTD_FUNCTION));
}

bool
hls_parse_url(B_PlaybackIpProtocol *protocol, char **server, unsigned *portPtr, char **uri, char *absoluteUri)
{
    char *tmp1 = NULL, *tmp2 = NULL, *tmp3 = NULL;

    if ( (tmp1 = B_PlaybackIp_UtilsStristr(absoluteUri, "http://"))) {
        *protocol = B_PlaybackIpProtocol_eHttp;
        tmp1 += strlen("http://");
        *portPtr = 80;
    }
    else if ( (tmp1 = B_PlaybackIp_UtilsStristr(absoluteUri, "https://"))) {
        *protocol = B_PlaybackIpProtocol_eHttps;
        tmp1 += strlen("https://");
        *portPtr = 443;
    }
    else {
        BDBG_ERR(("%s: unsupported protocol in the given URL %s", BSTD_FUNCTION, absoluteUri));
        return false;
    }
    /* http protocol is being used, parse it further */

    /* now take out the server string from the url */
    tmp2 = strstr(tmp1, "/");
    if (tmp2) {
        if ((*server = (char *)BKNI_Malloc(tmp2-tmp1+1)) == NULL) {
            BDBG_ERR(("%s: ERROR: failed to allocate %ld bytes of memory at %d\n", BSTD_FUNCTION, (long)(tmp2-tmp1), __LINE__));
            return false;
        }
        strncpy(*server, tmp1, tmp2-tmp1);
        (*server)[tmp2-tmp1] = '\0';

        /* Check to see if a port value was specified */
        tmp3 = strstr(*server, ":");
        if (tmp3) {
            tmp3[0] = '\0'; /* this null terminates the server name string */
            tmp3++;
            *portPtr = strtoul(tmp3, (char **) NULL, 10);
            if (*portPtr == 443)
                *protocol = B_PlaybackIpProtocol_eHttps;
        }

        /* now get the uri */
        *uri = tmp2;
        BDBG_MSG(("%s: server %s, port %d, protocol %d, url %s", BSTD_FUNCTION, *server, *portPtr, *protocol, *uri));
        return true;
    }
    else {
        BDBG_ERR(("%s: Incorrect URL: Failed to find the server part in %s", BSTD_FUNCTION, absoluteUri));
        return false;
    }
}

/* builds HTTP get request */
/* TODO: reuse the function from b_playback_ip_http.c */
static void
hls_build_get_req(B_PlaybackIpHandle playback_ip, char *write_buf, int write_buf_size, char *server, int port, char *uri, off_t byteRangeStart, off_t byteRangeEnd, bool useConnectionClosingFlag)
{
    char *header = write_buf;
    unsigned int bytesLeft = write_buf_size;;
    int bytesWrote = 0;

    BKNI_Memset(header, 0, bytesLeft);
    bytesWrote = snprintf(header, bytesLeft-1,
            "GET %s HTTP/1.1\r\n"
            "User-Agent: %s\r\n"
            "%s"
            "%s"
            ,
            uri,
            (playback_ip->openSettings.u.http.userAgent ? playback_ip->openSettings.u.http.userAgent : "BRCM HTTP Client/2.0"),
            (useConnectionClosingFlag ? "Connection: Close\r\n" : ""),
            (playback_ip->openSettings.u.http.additionalHeaders ? playback_ip->openSettings.u.http.additionalHeaders : "")
            );
    bytesLeft -= bytesWrote;
    header += bytesWrote;

    if (!playback_ip->openSettings.socketOpenSettings.useProxy) {
        if (port == HTTP_DEFAULT_PORT) {
            bytesWrote = snprintf(header, bytesLeft, "Host: %s\r\n", server);
        }
        else {
            /* not using the default HTTP port, so Host Header needs to include the port # */
            bytesWrote = snprintf(header, bytesLeft, "Host: %s:%d\r\n", server, port);
        }
    }
    else {
        char *tmp1, *tmp2;
        tmp1 = strstr(uri, "://"); /* host name starts after this sequence */
        if (tmp1) {
            tmp1 += 3; /* move past "://" */
            tmp2 = strstr(tmp1, "/");   /* host name ends here and URI starts after this */
            if (tmp2) {
                *tmp2 = '\0';
                bytesWrote = snprintf(header, bytesLeft, "Host: %s\r\n", tmp1);
                *tmp2 = '/';
            }
        }
    }
    bytesLeft -= bytesWrote;
    header += bytesWrote;

    if (byteRangeStart != 0 || byteRangeEnd != 0) {
        char *rangeString;
        rangeString = "Range:";
        if (byteRangeEnd > byteRangeStart)
            bytesWrote = snprintf(header, bytesLeft, "%s bytes=%"PRId64 "-%"PRId64 "\r\n", rangeString, byteRangeStart, byteRangeEnd);
        else {
            if (playback_ip->contentLength != 0)
                bytesWrote = snprintf(header, bytesLeft, "%s bytes=%"PRId64 "-%"PRId64 "\r\n", rangeString, byteRangeStart, playback_ip->contentLength-1);
            else
                bytesWrote = snprintf(header, bytesLeft, "%s bytes=%"PRId64 "-\r\n", rangeString, byteRangeStart);
        }
        bytesLeft -= bytesWrote;
        header += bytesWrote;
    }
    if (playback_ip->cookieFoundViaHttpRedirect) {
        /* add the cookie header */
        bytesWrote = snprintf(header, bytesLeft, "Cookie: %s\r\n", playback_ip->cookieFoundViaHttpRedirect);
        bytesLeft -= bytesWrote;
        header += bytesWrote;
    }

    bytesWrote = snprintf(header, bytesLeft, "\r\n");
    bytesLeft -= bytesWrote;
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog) {
        BDBG_WRN(("Complete HTTP Get request:"));
        fprintf(stdout, "%s", write_buf);
        fflush(stdout);
    }
#endif
}

void
B_Playback_HlsDestroyPlaybackIpSession(
    B_PlaybackIpHandle playbackIp
    )
{
    if (!playbackIp)
        return;

    playbackIp->playback_state = B_PlaybackIpState_eSessionOpened; /* change state back to Open */
    playbackIp->hlsSessionEnabled = false;

    if (B_PlaybackIp_SessionClose(playbackIp)) {
        BDBG_ERR(("%s: B_PlaybackIp_SessionClose() Failed", BSTD_FUNCTION));
        return;
    }

    if (B_PlaybackIp_Close(playbackIp)) {
        BDBG_ERR(("%s: B_PlaybackIp_Close() Failed", BSTD_FUNCTION));
        return;
    }
}

static B_PlaybackIpHandle
B_Playback_HlsCreatePlaybackIpSession(
    B_PlaybackIpHandle playback_ip,
    char *server,
    unsigned port,
    char **uri,
    B_PlaybackIpProtocol protocol,
    void *initialSecurityContext
    )
{
    B_PlaybackIpSessionOpenSettings *pIpSessionOpenSettings = NULL;
    B_PlaybackIpSessionOpenStatus ipSessionOpenStatus;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    B_PlaybackIpHandle playbackIp;

    playbackIp = B_PlaybackIp_Open(NULL);
    if (!playbackIp) {BDBG_ERR(("%s: Failed to open a Playback Ip Session", BSTD_FUNCTION)); return NULL;}
    BDBG_MSG(("%s:%p -> %p server %s, port %d, uri %s, protocol %d", BSTD_FUNCTION, (void *)playback_ip, (void *)playbackIp, server, port, *uri, protocol));

    pIpSessionOpenSettings = B_Os_Calloc( 1, sizeof(B_PlaybackIpSessionOpenSettings));
    if (pIpSessionOpenSettings == NULL) {
        BDBG_ERR(("%s: Failed to allocated %zu bytes", BSTD_FUNCTION, sizeof(B_PlaybackIpSessionOpenSettings)));
        goto error;
    }
    memset(&ipSessionOpenStatus, 0, sizeof(ipSessionOpenStatus));
    pIpSessionOpenSettings->security.initialSecurityContext = initialSecurityContext;
    pIpSessionOpenSettings->security.dmaHandle = playback_ip->openSettings.security.dmaHandle;
    if (protocol == B_PlaybackIpProtocol_eHttps) {
        if (initialSecurityContext == NULL) {
            BDBG_ERR(("%s: ERROR: App needs to call B_PlaybackIp_SslInit() and pass in the returned context in initialSecurityContext", BSTD_FUNCTION));
            goto error;
        }
        pIpSessionOpenSettings->security.securityProtocol = B_PlaybackIpSecurityProtocol_Ssl;
    }
    /* Set IP Address, Port, and Protocol used to receive the AV stream */
    strncpy(pIpSessionOpenSettings->socketOpenSettings.ipAddr, server, sizeof(pIpSessionOpenSettings->socketOpenSettings.ipAddr)-1);
    pIpSessionOpenSettings->socketOpenSettings.port = port;
    pIpSessionOpenSettings->socketOpenSettings.protocol = B_PlaybackIpProtocol_eHttp;
    pIpSessionOpenSettings->socketOpenSettings.useProxy = playback_ip->openSettings.socketOpenSettings.useProxy;
    pIpSessionOpenSettings->networkTimeout = playback_ip->settings.networkTimeout;
    pIpSessionOpenSettings->socketOpenSettings.url = *uri; /* pointer to the url only */
    pIpSessionOpenSettings->u.http.userAgent = playback_ip->openSettings.u.http.userAgent; /* pointer only */
    pIpSessionOpenSettings->u.http.additionalHeaders = playback_ip->openSettings.u.http.additionalHeaders;
    /* SessionOpen to the Server. */
    {
        int nRetry = 0;
        int nWaitTimeout = B_PLAYBACK_IP_SESSION_OPEN_MIN_TIMEOUT;

        BKNI_ResetEvent(playback_ip->sessionOpenRetryEventHandle);
        for(;;) {
            BERR_Code err;

            if ( breakFromLoop(playback_ip)) {
                goto error;
            }
            rc = B_PlaybackIp_SessionOpen(playbackIp, pIpSessionOpenSettings, &ipSessionOpenStatus);
            if ((rc == B_ERROR_PROTO) && (ipSessionOpenStatus.u.http.statusCode == B_PLAYBACK_IP_HTTP_STATUS_SERVICE_UNAVAILABLE)) {
                /* Server is temporarily unavailable, lets retry it again upto certain times!. */
                if (nRetry++ > B_PLAYBACK_IP_MAX_SESSION_OPEN_RETRY) {
                    BDBG_ERR(("%s:%p:%d max retries exceeded on B_PlaybackIp_SessionOpen: retry =%d session", BSTD_FUNCTION, (void *)playback_ip, playbackIpState(playback_ip), nRetry ));
                    goto error;
                }
                err = BKNI_WaitForEvent(playback_ip->sessionOpenRetryEventHandle, nWaitTimeout);
                if (err == BERR_TIMEOUT) {
                    nWaitTimeout *= 2;
                    if (nWaitTimeout >= B_PLAYBACK_IP_SESSION_OPEN_MAX_TIMEOUT) {
                        nWaitTimeout = B_PLAYBACK_IP_SESSION_OPEN_MAX_TIMEOUT;
                    }
                }
                else {
                    BKNI_Sleep(B_PLAYBACK_IP_SESSION_OPEN_MAX_TIMEOUT);
                }
            }
            else {
                /* Either sucess or a different error than the check above. */
                break;
            }
        }
    }
    if (pIpSessionOpenSettings) { B_Os_Free(pIpSessionOpenSettings); pIpSessionOpenSettings = NULL; }
    if (rc != B_ERROR_SUCCESS) {
        BDBG_ERR(("Session Open call failed: rc %d, HTTP Status %d\n", rc, ipSessionOpenStatus.u.http.statusCode));
        goto error;
    }
    BDBG_MSG(("%s:%p Session Open call succeeded, HTTP status code %d, server %s, port %d, uri %s, protocol %d", BSTD_FUNCTION, (void *)playback_ip, ipSessionOpenStatus.u.http.statusCode, server, port, *uri, protocol));

    playbackIp->parentPlaybackIpState = playback_ip->parentPlaybackIpState? playback_ip->parentPlaybackIpState: &playback_ip->playback_state;
    return playbackIp;

error:
    if (pIpSessionOpenSettings) {
        B_Os_Free(pIpSessionOpenSettings);
    }
    if (B_PlaybackIp_Close(playbackIp)) {
        BDBG_ERR(("%s: B_PlaybackIp_Close() Failed", BSTD_FUNCTION));
    }
    BDBG_ERR(("%s:%p:%d error in creating new PBIP session", BSTD_FUNCTION, (void *)playback_ip, playbackIpState(playback_ip)));
    return NULL;
}

/* setup HTTP Session, Open & Enable Security Context and get hls session ready for media Segment download */
    bool
B_PlaybackIp_HlsSetupHttpSessionToServer(
        B_PlaybackIpHandle playback_ip,
        MediaFileSegmentInfo *mediaFileSegmentInfo,
        bool persistentHttpSession,
        bool reOpenSocketConnection,
        int *socketFd,
        char *buffer,
        int *bufferDepth
        )
{
    bool rc = false;
    char *server = NULL;
    unsigned port = 0;
    char **uri;
    char *requestMessage = NULL, *responseMessage = NULL;
    int requestMessageSize = 0, responseMessageSize = 0;
    char *http_hdr = NULL, *http_payload = NULL;
    http_url_type_t http_url_type;
    char *serverRedirect = NULL;
    char *uriRedirect = NULL;
    B_PlaybackIpProtocol protocol;
    B_PlaybackIpSessionOpenSettings openSettings;
    socklen_t addrLen;
    struct sockaddr_in local_addr;
    bool useConnectionClosingFlag = true;
    off_t byteRangeStart, byteRangeEnd;
    int initialLen = 0;

    if (!playback_ip || !mediaFileSegmentInfo)
        return false;

    if (!playback_ip->openSettings.socketOpenSettings.useProxy) {
        server = mediaFileSegmentInfo->server;
        port = mediaFileSegmentInfo->port;
    }
    else {
        /* when proxy is enabled, connection request has to be sent to the proxy server */
        server = playback_ip->openSettings.socketOpenSettings.ipAddr;
        port = playback_ip->openSettings.socketOpenSettings.port;
    }
    protocol = mediaFileSegmentInfo->protocol;
    uri = &mediaFileSegmentInfo->uri;
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s:%p URI: http://%s:%d%s, sec protocol %d", BSTD_FUNCTION, (void *)playback_ip, server, port, *uri, mediaFileSegmentInfo->securityProtocol));
#endif

    /* reset previous content length */
    playback_ip->contentLength = 0;

    /* prepare initial Get request */
    responseMessageSize = TMP_BUF_SIZE;
    requestMessageSize = 4*TMP_BUF_SIZE;
    responseMessage = (char *) BKNI_Malloc(responseMessageSize+1);
    requestMessage = (char *)BKNI_Malloc(requestMessageSize);
    if (!responseMessage || !requestMessage) {
        BDBG_ERR(("%s: ERROR: failed to allocate memory\n", BSTD_FUNCTION));
        goto error;
    }
    memset(&openSettings, 0, sizeof(openSettings));
    openSettings.security.securityProtocol = mediaFileSegmentInfo->securityProtocol;
    openSettings.networkTimeout = playback_ip->openSettings.networkTimeout;
    for (;;) {
        memset(requestMessage, 0, requestMessageSize);
        memset(responseMessage, 0, responseMessageSize+1);
        byteRangeStart = 0;
        byteRangeEnd = 0;

        BDBG_MSG(("%s:%p Media Segment length %u, offset %"PRId64 "", BSTD_FUNCTION, (void *)playback_ip, mediaFileSegmentInfo->segmentLength, mediaFileSegmentInfo->segmentStartByteOffset));
        useConnectionClosingFlag = persistentHttpSession ? 0 : 1;
        if (mediaFileSegmentInfo->segmentLength) {
            byteRangeStart = mediaFileSegmentInfo->segmentStartByteOffset;
            byteRangeEnd = byteRangeStart + mediaFileSegmentInfo->segmentLength - 1;
        }
        hls_build_get_req(playback_ip, requestMessage, requestMessageSize, server, port, *uri, byteRangeStart, byteRangeEnd, useConnectionClosingFlag);

        if (reOpenSocketConnection) {
            /* not a persistent HTTP session, so do following: */
            /* so setup the socket connection to the server & send GET request */
            int retryCount = 0;
            B_Error rc = B_ERROR_UNKNOWN;
            int maxRetryCount;

#define TCP_CONNECT_TIMEOUT_INTERVAL 1
            maxRetryCount = playback_ip->networkTimeout / TCP_CONNECT_TIMEOUT_INTERVAL;
            if (maxRetryCount == 0) maxRetryCount = 1;
            while (retryCount++ < maxRetryCount) {
                B_PlaybackIpState playbackState = (playback_ip->parentPlaybackIpState?*playback_ip->parentPlaybackIpState:playback_ip->playback_state);
                if ( breakFromLoop(playback_ip)) {
                    goto error;
                }
                rc = B_PlaybackIp_UtilsTcpSocketConnect(&playbackState , server, port, true, TCP_CONNECT_TIMEOUT_INTERVAL, socketFd);
                if (rc == B_ERROR_SUCCESS) {
                    break;
                }
                else {
                    BDBG_ERR(("%s: ERROR: failed to send Socket Connect Request to Server: %s:%d, Retry it: retryCount %d, Max retryCount %d", BSTD_FUNCTION, server, port, retryCount, maxRetryCount));
                }
            }
            if (rc != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: ERROR: failed to send Socket Connect Request to Server: %s:%d\n", BSTD_FUNCTION, server, port));
                goto error;
            }

            addrLen = sizeof(local_addr);
            if (getsockname(*socketFd, (struct sockaddr *)&local_addr, (socklen_t *)&addrLen) != 0) {
                BDBG_ERR(("ERROR: Failed to obtain connection socket address, errno: %d \n", errno));
                perror("getsockname");
                goto error;
            }

            BDBG_MSG(("%s:%p successfully connected to server (fd %d, local ip:port = %s:%d)",
                        BSTD_FUNCTION, (void *)playback_ip, *socketFd, inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port)));
            /* now setup security context prior to downloading the media segment */
            /* currently, supported security protocols are: HTTPS (SSL/TLS), AES128, and Clear (no encryption) */
            /* Note: security protocol can change from segment to segment, so this function is called prior to each segment download */
            openSettings.security.initialSecurityContext = playback_ip->openSettings.security.initialSecurityContext; /* points to either AES (dmaHandle) or SSL initial security context */
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
            openSettings.security.dmaHandle = playback_ip->openSettings.security.dmaHandle;
#endif
            BDBG_MSG(("%s: security: protocol %d, ctx %p", BSTD_FUNCTION, openSettings.security.securityProtocol, (void *)openSettings.security.initialSecurityContext));
            switch (openSettings.security.securityProtocol) {
                case B_PlaybackIpSecurityProtocol_Aes128:
                    /* setup the new key & iv */
                    openSettings.security.enableDecryption = false;
                    openSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_Aes128;
                    memcpy(openSettings.security.settings.aes128.key, mediaFileSegmentInfo->encryptionInfo.encryptionMethod.aes128.key, sizeof(playback_ip->openSettings.security.settings.aes128.key));
                    memcpy(openSettings.security.settings.aes128.iv, mediaFileSegmentInfo->encryptionInfo.encryptionMethod.aes128.iv, sizeof(playback_ip->openSettings.security.settings.aes128.iv));
                    break;
                case B_PlaybackIpSecurityProtocol_Ssl:
                    openSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_Ssl;
                    openSettings.security.enableDecryption = true;
                    break;
                default:
                    /* Setup clear path */
                    openSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_None;
            }
            if (B_PlaybackIp_SecuritySessionOpen(playback_ip, &openSettings, *socketFd, &playback_ip->securityHandle) < 0) {
                BDBG_ERR(("%s: ERROR: failed to setup the security session", BSTD_FUNCTION));
                goto error;
            }
        }

        /* and then send the HTTP Get request */
        {
            B_PlaybackIpState playbackState = (playback_ip->parentPlaybackIpState?*playback_ip->parentPlaybackIpState:playback_ip->playback_state);
            if (playback_ip->netIo.write(playback_ip->securityHandle, &playbackState, *socketFd, requestMessage, strlen(requestMessage)) < 0) {
                BDBG_ERR(("%s: ERROR: failed to send HTTP Get request to Server: %s:%d\n", BSTD_FUNCTION, server, port));
                goto error;
            }
        }
        BDBG_MSG(("%s:%p Sent HTTP Get Request (socket %d) --->:\n %s", BSTD_FUNCTION, (void *)playback_ip, *socketFd, requestMessage));

        playback_ip->chunkEncoding = false;
        playback_ip->serverClosed = false;
        playback_ip->selectTimeout = false;
        *bufferDepth = 0;

        /* now read and process the HTTP Response headers */
        if (http_read_response(playback_ip, playback_ip->securityHandle, *socketFd, &responseMessage, responseMessageSize, &http_hdr, &http_payload, &initialLen) < 0) {
            BDBG_ERR(("%s: ERROR: failed to receive valid HTTP response\n", BSTD_FUNCTION));
            goto error;
        }
        http_url_type = http_get_url_type(http_hdr, *uri);
        if (http_url_type == HTTP_URL_IS_REDIRECT) {
            /* parse HTTP redirect and extract the new URI & server:port info */
            if ((serverRedirect = (char *)BKNI_Malloc(2048)) == NULL) {
                BDBG_ERR(("%s: failed to allocate memory for redirectServer", BSTD_FUNCTION));
                goto error;
            }
            /* free up the previous cookie */
            if (playback_ip->cookieFoundViaHttpRedirect)
                BKNI_Free(playback_ip->cookieFoundViaHttpRedirect);
            playback_ip->cookieFoundViaHttpRedirect = NULL;
            if (http_parse_redirect(serverRedirect, &port, &protocol, &uriRedirect, &(playback_ip->cookieFoundViaHttpRedirect), http_hdr) != 0) {
                BDBG_ERR(("%s: Incorrect HTTP Redirect response or parsing error", BSTD_FUNCTION));
                goto error;
            }
            BDBG_MSG(("%s: allocated cookie %s", BSTD_FUNCTION, playback_ip->cookieFoundViaHttpRedirect));
            /* previous iteration gets the new URL & server information and we send another GET request to this server */
            if (playback_ip->securityHandle)
                playback_ip->netIo.close(playback_ip->securityHandle);
            close(*socketFd);
            playback_ip->securityHandle = NULL;
            server = serverRedirect;
            uri = &uriRedirect;
            if (protocol == B_PlaybackIpProtocol_eHttps)
                openSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_Ssl;
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: HTTP redirect case, caching the redirected URL: http://%s:%d%s", BSTD_FUNCTION, server, port, *uri));
#endif
            /* set flag to reconnect to the redirected server */
            reOpenSocketConnection = true;
            continue;
        }
        else {
            /* actual content URL, get the content attributes from HTTP response header */
            BDBG_MSG(("%s:%p GOT ACTUAL CONTENT: sock fd %d", BSTD_FUNCTION, (void *)playback_ip, *socketFd));
            break;
        }
    }

    /* store the content type of the payload, this assists in providing a valuble hint to media probe and thus cuts down the probe time */
    playback_ip->http_content_type = http_get_payload_content_type(responseMessage);

    if (openSettings.security.securityProtocol == B_PlaybackIpSecurityProtocol_Aes128) {
        /* enable AES encryption as HTTP header processing (which is in clear) is done now */
        if (B_PlaybackIp_AesDecryptionEnable(playback_ip->securityHandle, http_payload, initialLen) < 0) {
            BDBG_ERR(("%s: ERROR: failed to enable the security decryption", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s:%p security context is enabled for media segment %p, seq # %d, sec protocol %d, initial encrypted bytes %d", BSTD_FUNCTION, (void *)playback_ip, (void *)mediaFileSegmentInfo, mediaFileSegmentInfo->mediaSequence, openSettings.security.securityProtocol, initialLen));
        playback_ip->initial_data_len = 0; /* initial payload is now given to security layer for later decryption during the read call */
        initialLen = 0; /* initial payload is now given to security layer for later decryption during the read call */
    }
    if (initialLen) {
        *bufferDepth = initialLen;
        BKNI_Memcpy(buffer, http_payload, *bufferDepth);
        BDBG_MSG(("%s:%p completed, %d bytes of initial playlist read", BSTD_FUNCTION, (void *)playback_ip, *bufferDepth));
    }
    rc = true;

error:
    if (serverRedirect)
        BKNI_Free(serverRedirect);
    if (uriRedirect)
        BKNI_Free(uriRedirect); /* allocated via strdup of redirected URI in http_parse_redirect */
    if (responseMessage)
        BKNI_Free(responseMessage);
    if (requestMessage)
        BKNI_Free(requestMessage);
    if (!rc && *socketFd) {
        if (playback_ip->securityHandle) {
            playback_ip->netIo.close(playback_ip->securityHandle);
            playback_ip->securityHandle = NULL;
        }
        close(*socketFd);
        *socketFd = -1;
    }
    BDBG_MSG(("%s:%p Done ", BSTD_FUNCTION, (void *)playback_ip));
    return rc;
}

bool
hls_downloadEncryptionKey(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSecurityProtocol securityProtocol,
    char *encryptionKeyUri,
    EncryptionInfo *encryptionInfo
    )
{
    char *server = NULL;
    unsigned port;
    char *uri;
    B_PlaybackIpProtocol protocol;
    int i;
    int encryptionKeyBufferSize = 4096;
    int encryptionKeyBufferDepth;
    unsigned char *encryptionKeyBuffer = NULL;
    bool status = false;
    B_PlaybackIpHandle playbackIp = NULL;

    BDBG_MSG(("%s:%p downloading key from uri %s", BSTD_FUNCTION, (void *)playback_ip, encryptionKeyUri));
    if (hls_parse_url(&protocol, &server, &port, &uri, encryptionKeyUri) == false) {
        BDBG_ERR(("Failed to parse URI at %s:%d", BSTD_FUNCTION, __LINE__));
        goto error;
    }
    if (playback_ip->openSettings.socketOpenSettings.useProxy) {
        /* when proxy is enabled, connection request has to be sent to the proxy server */
        if (server) BKNI_Free(server);
        server = playback_ip->openSettings.socketOpenSettings.ipAddr;
        port = playback_ip->openSettings.socketOpenSettings.port;
    }
    encryptionKeyBuffer = (unsigned char *)BKNI_Malloc(encryptionKeyBufferSize);
    if (!encryptionKeyBuffer) {
        BDBG_ERR(("%s: ERROR: failed to allocate memory\n", BSTD_FUNCTION));
        goto error;
    }

    if (protocol == B_PlaybackIpProtocol_eHttp || protocol == B_PlaybackIpProtocol_eHttps) {
        playbackIp = B_Playback_HlsCreatePlaybackIpSession(playback_ip, server, port, &uri, protocol, playback_ip->openSettings.security.initialSecurityContext);
        if (!playbackIp) {
            goto error;
        }
        /* restore some state from the main playback_ip session state */
        playbackIp->hlsSessionState = playback_ip->hlsSessionState;

        /* set state to be playing so that we can use the read/write calls w/o having to start the IpSessions */
        playbackIp->playback_state = B_PlaybackIpState_ePlaying;

        BDBG_MSG(("%s: Connected to the server for downloading the encryption key file", BSTD_FUNCTION));
        if (playbackIp->initial_data_len) {
            BKNI_Memcpy(encryptionKeyBuffer, playbackIp->temp_buf, playbackIp->initial_data_len);
            encryptionKeyBufferDepth = playbackIp->initial_data_len;
            playbackIp->initial_data_len = 0;
        }
        else {
            encryptionKeyBufferDepth = 0;
        }

        /* now download the key */
        if (B_PlaybackIp_HlsDownloadFile(playbackIp, &playbackIp->socketState.fd, (char *)encryptionKeyBuffer, encryptionKeyBufferSize, &encryptionKeyBufferDepth, false) != true) {
            BDBG_ERR(("%s: failed to download the encryption key file", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: downloaded encryption key file of %d bytes", BSTD_FUNCTION, encryptionKeyBufferDepth));
    }
    else {
        BDBG_ERR(("%s: not supported key file download protocol (%d) specified", BSTD_FUNCTION, protocol));
        goto error;
    }

    /* now store the key */
    if (securityProtocol == B_PlaybackIpSecurityProtocol_Aes128) {
        if (encryptionKeyBufferDepth > 16) {
            BDBG_ERR(("%s: Encryption Key length %d > 16", BSTD_FUNCTION, encryptionKeyBufferDepth));
            goto error;
        }
        memcpy(encryptionInfo->encryptionMethod.aes128.key, encryptionKeyBuffer, sizeof(encryptionInfo->encryptionMethod.aes128.key));
        for (i=0; i<16;i++) {
            BDBG_MSG(("enc key[%d] %02x", i, encryptionInfo->encryptionMethod.aes128.key[i]));
        }
        BDBG_MSG(("%s: successfully downloaded the encryption key of %d length", BSTD_FUNCTION, encryptionKeyBufferDepth));
    }
    BDBG_MSG(("%s: URI for Media Segment Encryption key: server %s, port %d, uri %s", BSTD_FUNCTION, server, port, uri));
    status = true;

error:
    if (playbackIp) {
        playbackIp->hlsSessionState = NULL; /* as it was just a pointer to the parent playback_ip object's hlsSessionState object. */
        /* now destroy this temp playback ip session */
        B_Playback_HlsDestroyPlaybackIpSession(playbackIp);
    }
    if (encryptionKeyBuffer)
        BKNI_Free(encryptionKeyBuffer);
    if (!playback_ip->openSettings.socketOpenSettings.useProxy && server)
        /* server is only allocated when we are not using proxy server, otherwise it points to the original proxy server buffer from OpenSettings */
        BKNI_Free(server);
    return status;
}

bool
B_PlaybackIp_ParsePlaylistFile(
    B_PlaybackIpHandle playback_ip,
    PlaylistFileInfo *playlistFileInfo,
    bool parseOnlyOneMediaEntry
    )
{
    /* parse the common info first */
    char *nextLine, *tag;
    int nextLineLength = 0;;
    int rc;
    MediaFileSegmentInfo *mediaFileSegmentInfo = NULL;
    int nextSeq = 0;
    bool seqTagFound = false;
    bool urlTagFound = false;
    bool versionTagFound = false;
    bool markedDiscontinuity = false;
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;
    char *encryptionKeyUri = NULL;
    EncryptionInfo encryptionInfo;
    bool ivSpecified = false;
    char *tmp, *tmp1, *attribute, *value;
    B_PlaybackIpSecurityProtocol securityProtocol = B_PlaybackIpSecurityProtocol_None;
    off_t nextSegmentOffset = 0;

    BDBG_ASSERT(playlistFileInfo);

    playlistFileInfo->bounded = false;
    playlistFileInfo->mediaSegmentBaseSequence = -1;
    memset(&encryptionInfo, 0, sizeof(encryptionInfo));

    for (
            (nextLine = NULL);
            ((rc = B_PlaybackIp_UtilsGetLine(&nextLine, &nextLineLength, hlsSession->playlistBuffer+hlsSession->playlistBufferReadIndex, hlsSession->playlistBufferDepth-hlsSession->playlistBufferReadIndex)) > 0);
            (hlsSession->playlistBufferReadIndex += rc, BKNI_Free(nextLine), nextLine = NULL)
        )
    {
        /* now parse this line */
        BDBG_MSG(("%s: next line is line size %d, index %d, %s", BSTD_FUNCTION, rc, hlsSession->playlistBufferReadIndex, nextLine));
        if ((tag = B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_VERSION_TAG)) != NULL) {
            short version;
            if (versionTagFound == true) {
                BDBG_ERR(("%s: Invalid Playlist file as it contains > 1 %s tag", BSTD_FUNCTION, HLS_M3U8_VERSION_TAG));
                goto error;
            }
            tag += strlen(HLS_M3U8_VERSION_TAG);
            version = strtol(tag, NULL, 10);
            if (version > PLAYLIST_FILE_COMPATIBILITY_VERSION) {
                BDBG_ERR(("%s: Incompatible Playlist Version %d, currently supported %d", BSTD_FUNCTION, version, PLAYLIST_FILE_COMPATIBILITY_VERSION));
            }
            versionTagFound = true;
            playlistFileInfo->version = version;
            BDBG_MSG(("%s: Playlist VERSION %d", BSTD_FUNCTION, playlistFileInfo->version));
        }
        else if ((tag = B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_MAX_MEDIA_SEGMENT_DURATION_TAG)) != NULL) {
            tag += strlen(HLS_M3U8_MAX_MEDIA_SEGMENT_DURATION_TAG);
            playlistFileInfo->maxMediaSegmentDuration = 1000 * strtol(tag, NULL, 10);
            BDBG_MSG(("%s: Max Media Segment Duration %ld msec", BSTD_FUNCTION, playlistFileInfo->maxMediaSegmentDuration));
        }
        else if ( (tag = B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_MEDIA_SEGMENT_SEQUENCE_TAG)) != NULL) {
            if (seqTagFound) {
                BDBG_ERR(("%s: Invalid Playlist file as it contains > 1 %s tag", BSTD_FUNCTION, HLS_M3U8_MEDIA_SEGMENT_SEQUENCE_TAG));
                goto error;
            }
            tag += strlen(HLS_M3U8_MEDIA_SEGMENT_SEQUENCE_TAG);
            playlistFileInfo->mediaSegmentBaseSequence = strtol(tag, NULL, 10);
            BDBG_MSG(("%s: Media Segment Base Seq# %d", BSTD_FUNCTION, playlistFileInfo->mediaSegmentBaseSequence));
            seqTagFound = true;
        }
        else if ( (tag = B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_PROGRAM_RECORD_TAG)) != NULL) {
            tag += strlen(HLS_M3U8_PROGRAM_RECORD_TAG);
            if (B_PlaybackIp_UtilsStristr(tag, "YES") != NULL) {
                playlistFileInfo->allowCaching = true;
                BDBG_MSG(("%s: Media Stream Caching (recording) is allowed (TODO: handle this value!!!!)", BSTD_FUNCTION));
            }
            else {
                playlistFileInfo->allowCaching = false;
                BDBG_MSG(("%s: Media Stream Caching (recording) is NOT allowed (TODO: handle this value!!!!)", BSTD_FUNCTION));
            }
        }
        else if ( (tag = B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_URL_TAG)) != NULL) {
            /* URL tag: allocate another media segment info entry and fill in duration & uri */
            tag += strlen(HLS_M3U8_URL_TAG);

            /* allocate/initialize media file segment info structure */
            if ((mediaFileSegmentInfo = (MediaFileSegmentInfo *)BKNI_Malloc(sizeof(MediaFileSegmentInfo))) == NULL) {
                BDBG_ERR(("%s: Failed to allocate %zu bytes of memory for media segment info structure", BSTD_FUNCTION, sizeof(MediaFileSegmentInfo)));
                goto error;
            }
            memset(mediaFileSegmentInfo, 0, sizeof(MediaFileSegmentInfo));
            playlistFileInfo->numMediaSegments++;
            if (playlistFileInfo->currentMediaFileSegment == NULL) {
                /* initialize the queue */
                BLST_Q_INIT(&playlistFileInfo->mediaFileSegmentInfoQueueHead);
                playlistFileInfo->currentMediaFileSegment = mediaFileSegmentInfo;
                if (playlistFileInfo->mediaSegmentBaseSequence != -1) {
                    /* Playlist contains the base media segment tag, so we start w/ that. */
                    mediaFileSegmentInfo->mediaSequence = playlistFileInfo->mediaSegmentBaseSequence;
                }
                else {
                    /* Playlist doesn't contain the base media segment tag, so we start from 0. */
                    mediaFileSegmentInfo->mediaSequence = 0;
                }
                nextSeq = mediaFileSegmentInfo->mediaSequence + 1;
                BDBG_MSG(("%s: initialized head for the media segment info queue, base seq# %d, mediaFileSegmentInfo entry %p", BSTD_FUNCTION, playlistFileInfo->mediaSegmentBaseSequence, (void *)mediaFileSegmentInfo));
            }
            else {
                mediaFileSegmentInfo->mediaSequence = nextSeq++;
                BDBG_MSG(("%s: seq %d", BSTD_FUNCTION, mediaFileSegmentInfo->mediaSequence));
            }
            if (markedDiscontinuity == true) {
                mediaFileSegmentInfo->markedDiscontinuity = true;
                markedDiscontinuity = false;
            }
            BDBG_MSG(("%s: allocated %zu bytes of memory for media segment info: %p, seq# %d", BSTD_FUNCTION, sizeof(MediaFileSegmentInfo), (void *)mediaFileSegmentInfo, mediaFileSegmentInfo->mediaSequence));

            /* syntax of the EXTINF line is: #EXTINF:<duration>,<title> */
            /* parse the duration attribute: it can be in integer or floating-point number in decimal notation */
            /* we convert either of these two formats into msec and store it as the media segmention duration */
            if ((tmp = strstr(tag, ",")) != NULL) {
                *tmp = '\0';
                /* duration can be either decimal or floating point, convert it into msec */
                mediaFileSegmentInfo->duration = convertDurationAttributeToMsec(tag);
                BDBG_MSG(("%s: Media Segment Duration %ld", BSTD_FUNCTION, mediaFileSegmentInfo->duration));
                if (mediaFileSegmentInfo->duration > playlistFileInfo->maxMediaSegmentDuration) {
                    BDBG_MSG(("%s: Invalid Media Segment Duration %ld, Max %ld, ignoring it", BSTD_FUNCTION, mediaFileSegmentInfo->duration, playlistFileInfo->maxMediaSegmentDuration));
                }
                playlistFileInfo->totalDuration += mediaFileSegmentInfo->duration;
                playlistFileInfo->initialMinimumReloadDelay = mediaFileSegmentInfo->duration/1000; /* incase this happens to be the last URI */
            }
            else {
                BDBG_ERR(("%s: Invalid Media Segment Duration tag: missing field separate , ", BSTD_FUNCTION));
                goto error;
            }
            urlTagFound = true;
        }
        else if (B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_DISCONTINUITY_TAG) != NULL) {
            markedDiscontinuity = true;
            BDBG_MSG(("%s: Marked discontinuity event set", BSTD_FUNCTION));
        }
        else if ( (tag = B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_PROGRAM_DATE_TIME_TAG)) != NULL) {
            tag += strlen(HLS_M3U8_PROGRAM_DATE_TIME_TAG);
            BDBG_MSG(("%s: program date & time tag set: %s (TODO: handle this value!!!!)", BSTD_FUNCTION, tag));
        }
        else if ( (tag = B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_ENCRYPTION_KEY_TAG)) != NULL) {
            memset(&encryptionInfo, 0, sizeof(encryptionInfo));
            /* parse the attributes of this tag, its format is:
                EXT-X-KEY:[attribute=value][,attribute=value]* : e.g.
                EXT-X-KEY:METHOD=<method>[,URI="<URI>"][,IV=<IV>]
             */
            for (
                    (tmp = attribute = tag + strlen(HLS_M3U8_ENCRYPTION_KEY_TAG));    /* points to 1st attribute */
                    (tmp && ((tmp = strstr(attribute, "=")) != NULL));
                    (attribute = tmp+1)
                )
            {
                *tmp = '\0'; /* null terminate attribute to convert it into a string */
                value = tmp+1;
                /* multiple attributes are separate by , */
                if ((tmp = strstr(value, ",")) != NULL) {
                    /* another attribute is present following this attribute=value pair */
                    *tmp = '\0';    /* null terminate value to convert it into a string */
                }
                if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_ENC_METHOD) != NULL) {
                    if (B_PlaybackIp_UtilsStristr(value, HLS_M3U8_TAG_ATTRIBUTE_ENC_METHOD_NONE) != NULL) {
                        BDBG_MSG(("%s: Subsequent Media Segments are not encrypted", BSTD_FUNCTION));
                        securityProtocol = B_PlaybackIpSecurityProtocol_None;
                    }
                    else if (B_PlaybackIp_UtilsStristr(value, HLS_M3U8_TAG_ATTRIBUTE_ENC_METHOD_AES128) != NULL) {
                        BDBG_MSG(("%s: Subsequent Media Segments are AES128 encrypted", BSTD_FUNCTION));
                        securityProtocol = B_PlaybackIpSecurityProtocol_Aes128;
                    }
                    else {
                        BDBG_ERR(("%s: ERROR: invalid KEY Tag: Encryption Method attribute value %s is not supported", BSTD_FUNCTION, value));
                        goto error;
                    }
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_ENC_URI) != NULL) {
                    if (securityProtocol != B_PlaybackIpSecurityProtocol_Aes128) {
                        BDBG_ERR(("%s: ERROR: invalid KEY Tag: Encryption URI specified for non-AES security protocols", BSTD_FUNCTION));
                        goto error;
                    }
                    /* uri is encapsulated in "", skip them */
                    value += 1; /* skip the 1st " char */
                    if ((tmp1 = strstr(value, "\"")) != NULL) {
                        *tmp1 = '\0'; /* leave out the last " char */
                    }
                    if (http_absolute_uri(value)) {
                        /* mediaSegment URI is absolute uri     */
                        BDBG_MSG(("%s: URI (%s) for media segment encryption key is absolute", BSTD_FUNCTION, value));
                        if ((encryptionKeyUri = B_PlaybackIp_UtilsStrdup(value)) == NULL) {
                            BDBG_ERR(("memory allocation for enc key failed at %s:%d", BSTD_FUNCTION, __LINE__));
                            goto error;
                        }
                    }
                    else {
                        /* relative url, build complete uri using server ip address & port # */
                        BDBG_MSG(("%s: URI (%s) for media segment encryption key not is absolute", BSTD_FUNCTION, value));
                        if ((encryptionKeyUri = B_PlaybackIp_HlsBuildAbsoluteUri(playlistFileInfo->server, playlistFileInfo->port, playlistFileInfo->uri, value)) == NULL) {
                            BDBG_ERR(("Failed to build URI at %s:%d", BSTD_FUNCTION, __LINE__));
                            goto error;
                        }
                    }
                    /* got the URI for the key file, now download & store the key file */
                    if (hls_downloadEncryptionKey(playback_ip, securityProtocol, encryptionKeyUri, &encryptionInfo) == false) {
                        BDBG_ERR(("Failed to download the encryption key at %s:%d", BSTD_FUNCTION, __LINE__));
                        goto error;
                    }
                    if (encryptionKeyUri)
                        BKNI_Free(encryptionKeyUri);
                        encryptionKeyUri = NULL;
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_ENC_IV) != NULL) {
                    int ivLength, i;
                    /* copy the IV */
                    if (securityProtocol == B_PlaybackIpSecurityProtocol_Aes128) {
                        if (value[0] != '0' || (value[1] != 'x' && value[1] != 'X')) {
                            BDBG_ERR(("%s: IV must begin w/ 0x or 0X, it is %c%c", BSTD_FUNCTION, value[0], value[1]));
                            goto error;
                        }
                        value +=2; /* skips 0x */
                        ivLength = strlen(value);
                        if (ivLength/2 > 16) {
                            BDBG_ERR(("%s: IV length %d > 16", BSTD_FUNCTION, ivLength/2));
                            goto error;
                        }
                        for (i=0; i<ivLength; i+=2) {
                            unsigned char hi, lo;
                            if (hls_asciiHexToDec(value[i], &hi) != true || hls_asciiHexToDec(value[i+1], &lo) != true) {
                                BDBG_ERR(("%s: Failed to convert char %c, %c", BSTD_FUNCTION, value[i], value[i+1]));
                                goto error;
                            }
                            encryptionInfo.encryptionMethod.aes128.iv[i/2] = (hi<<4)|lo;
                        }
                        for (i=0; i<16;i++) {
                            BDBG_MSG(("iv[%d] %02x", i, encryptionInfo.encryptionMethod.aes128.iv[i]));
                        }
                        ivSpecified = true;
                    }
                    else {
                        BDBG_ERR(("%s: IV is not yet supported for this encryption method %d", BSTD_FUNCTION, securityProtocol));
                        goto error;
                    }
                }
                else {
                    BDBG_ERR(("%s: ERROR: undefined Attribute: %s", BSTD_FUNCTION, attribute));
                    goto error;
                }
            }    /* end of for loop for parsing all attributes */
            if (!ivSpecified) {
                /* IV is not explicitly provided, HLS spec requires media segment sequence number to be used as the IV, set a flag to indicate that */
                encryptionInfo.useMediaSegSeqAsIv = true;
            }
            else {
                encryptionInfo.useMediaSegSeqAsIv = false;
            }
            BDBG_MSG(("%s: encryption key tag is successfully parsed", BSTD_FUNCTION));
        }
        else if (urlTagFound == true) {
            /* HLS version 4 can include a EXT-X-BYTERANGE tag between EXTINF & url tags */
            if ((tag = B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_BYTERANGE_TAG)) != NULL) {
                tag += strlen(HLS_M3U8_BYTERANGE_TAG);
                if ((tmp = strstr(tag, "@")) != NULL) {
                    /* start byte offset is present */
                    *tmp = '\0';
                    tmp += 1; /* move past the @ char and read the actual value */
                    mediaFileSegmentInfo->segmentStartByteOffset = strtol(tmp, NULL, 10);
                }
                else {
                    /* start offset of this segment is not given, so use the running offset value that we maintain by cumulating each segment length */
                    mediaFileSegmentInfo->segmentStartByteOffset = nextSegmentOffset;
                }
                /* now get the length of segment sub-range */
                mediaFileSegmentInfo->segmentLength = strtol(tag, NULL, 10);
                nextSegmentOffset += mediaFileSegmentInfo->segmentLength;
                BDBG_MSG(("%s: Media Segment length %u, offset %"PRId64 "", BSTD_FUNCTION, mediaFileSegmentInfo->segmentLength, mediaFileSegmentInfo->segmentStartByteOffset));
                /* now go back to read the next line which must be actual segment URI */
                continue;
            }

            /* we have processed all tags in-between URL tag and actual URL, so now copy the actual URL */
            if (http_absolute_uri(nextLine)) {
                /* mediaSegment URI is absolute uri */
                BDBG_MSG(("%s: media segment URI is absolute", BSTD_FUNCTION));
                if ((mediaFileSegmentInfo->absoluteUri = B_PlaybackIp_UtilsStrdup(nextLine)) == NULL) {
                    BDBG_ERR(("%s: Failed to allocate %zu bytes of memory for absolute URI ", BSTD_FUNCTION, strlen(nextLine)));
                    goto error;
                }
            }
            else {
                /* relative url, build complete uri using server ip address & port # */
                BDBG_MSG(("%s: media segment URI is not absolute URI", BSTD_FUNCTION));
                if ((mediaFileSegmentInfo->absoluteUri = B_PlaybackIp_HlsBuildAbsoluteUri(playlistFileInfo->server, playlistFileInfo->port, playlistFileInfo->uri, nextLine)) == NULL) {
                    BDBG_ERR(("Failed to build URI at %s:%d", BSTD_FUNCTION, __LINE__));
                    goto error;
                }
            }
            if ((hls_parse_url(&mediaFileSegmentInfo->protocol, &mediaFileSegmentInfo->server, &mediaFileSegmentInfo->port, &mediaFileSegmentInfo->uri, mediaFileSegmentInfo->absoluteUri) == false) || (mediaFileSegmentInfo->protocol != B_PlaybackIpProtocol_eHttp && mediaFileSegmentInfo->protocol != B_PlaybackIpProtocol_eHttps)) {
                BDBG_ERR(("Failed to parse URI at %s:%d", BSTD_FUNCTION, __LINE__));
                goto error;
            }
            /* security protocol applies for all subsequent segments for AES protocol */
            mediaFileSegmentInfo->securityProtocol = securityProtocol;
            mediaFileSegmentInfo->encryptionInfo = encryptionInfo;
            if (mediaFileSegmentInfo->protocol == B_PlaybackIpProtocol_eHttps) {
                mediaFileSegmentInfo->securityProtocol = B_PlaybackIpSecurityProtocol_Ssl;
                /* we reset the securityProtocol as for HTTPs, it is determined by the URL and thus it should be reset */
                securityProtocol = B_PlaybackIpSecurityProtocol_None;
                BDBG_MSG(("%s: HLS Session is using HTTPS Encryption Method", BSTD_FUNCTION));
            }
            else if (mediaFileSegmentInfo->securityProtocol == B_PlaybackIpSecurityProtocol_Aes128) {
                if (encryptionInfo.useMediaSegSeqAsIv == true) {
                    /* use seq # as the IV */
                         unsigned mediaSequence = mediaFileSegmentInfo->mediaSequence < 0 ? 0 : mediaFileSegmentInfo->mediaSequence;
                        mediaFileSegmentInfo->encryptionInfo.encryptionMethod.aes128.iv[0] = (unsigned char)(mediaSequence >> 24);
                        mediaFileSegmentInfo->encryptionInfo.encryptionMethod.aes128.iv[1] = (unsigned char)(mediaSequence >> 16);
                        mediaFileSegmentInfo->encryptionInfo.encryptionMethod.aes128.iv[2] = (unsigned char)(mediaSequence >> 8);
                        mediaFileSegmentInfo->encryptionInfo.encryptionMethod.aes128.iv[3] = (unsigned char)(mediaSequence >> 0);
                    BDBG_MSG(("%s: explicit IV not set, using seq # %d as iv: iv[0] %02x, iv[1] %02x, iv[2] %02x, iv[3] %02x", BSTD_FUNCTION, mediaSequence,
                                mediaFileSegmentInfo->encryptionInfo.encryptionMethod.aes128.iv[0], mediaFileSegmentInfo->encryptionInfo.encryptionMethod.aes128.iv[1], mediaFileSegmentInfo->encryptionInfo.encryptionMethod.aes128.iv[2], mediaFileSegmentInfo->encryptionInfo.encryptionMethod.aes128.iv[3]));
                }
            }
            else if (mediaFileSegmentInfo->securityProtocol != B_PlaybackIpSecurityProtocol_None) {
                BDBG_ERR(("%s: Encryption Method %d is not yet supported", BSTD_FUNCTION, mediaFileSegmentInfo->securityProtocol));
                goto error;
            }
            /* now insert this entry into the queue of media file segments */
            BLST_Q_INSERT_TAIL(&playlistFileInfo->mediaFileSegmentInfoQueueHead, mediaFileSegmentInfo, next);
            urlTagFound = false;
            mediaFileSegmentInfo = NULL; /* as this entry is now already inserted into the list of segment URIs */
            if (parseOnlyOneMediaEntry) {BDBG_WRN(("%s:%p: parseOnlyOneMediaEntry is set, so NOT parsing rest of the entries", BSTD_FUNCTION, (void *)playback_ip)); break;}
        }
        else if (B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_END_TAG) != NULL) {
            playlistFileInfo->bounded = true;
            BDBG_MSG(("%s: Playlist is bounded and thus not live stream", BSTD_FUNCTION));
            break;
        }
        else if ( (tag = B_PlaybackIp_UtilsStristr(nextLine, "#EXT")) != NULL) {
            BDBG_MSG(("%s: !!!! TODO: Yet to parse this tag %s!!!! ", BSTD_FUNCTION, tag));
        }
        else {
            BDBG_MSG(("%s: Comment line %s, ignore it", BSTD_FUNCTION, tag));
        }
    }
    if (nextLine) {
        BKNI_Free(nextLine);
        nextLine = NULL;
    }

    if (playlistFileInfo->numMediaSegments == 0) {
        BDBG_ERR(("%s: Invalid Playlist file as it doesn't contains any media segment URLs", BSTD_FUNCTION));
        goto error;
    }

    /* TODO: if the EXT-X-MEDIA-SEQUENCE is not set, then we may have to make sure that the updated playlist still contains the current uri (which is being played) */
    /* HLS_M3U8_MAX_MEDIA_SEGMENT_DURATION_TAG must be present only once */
    if (playlistFileInfo->maxMediaSegmentDuration == 0) {
        BDBG_MSG(("%s: Playlist file doesn't contains %s tag", BSTD_FUNCTION, HLS_M3U8_MEDIA_SEGMENT_SEQUENCE_TAG));
    }

#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog) {
        BDBG_WRN(("%s: finished parsing playlist file w/ uri %s",
                BSTD_FUNCTION, playlistFileInfo->absoluteUri));
        BDBG_WRN(("%s: finished parsing playlist file: total duration %ld, bounded %s, numMediaSegments %d",
                BSTD_FUNCTION, playlistFileInfo->totalDuration, playlistFileInfo->bounded ? "YES" : "NO", playlistFileInfo->numMediaSegments));
    }
#endif

    if (encryptionKeyUri)
        BKNI_Free(encryptionKeyUri);
    return true;

error:
    if (nextLine) {
        BKNI_Free(nextLine);
    }
    /* free up resources */
    if (mediaFileSegmentInfo) {
        BDBG_MSG(("%s: freeing mediaFileSegmentInfo entry %p", BSTD_FUNCTION, (void *)mediaFileSegmentInfo));
        B_PlaybackIp_FreeMediaFileSegmentInfo(mediaFileSegmentInfo);
    }
    if (encryptionKeyUri)
        BKNI_Free(encryptionKeyUri);
    return false;
}

#ifdef HLS_UNIT_TEST
static bool
printPlaylistFile(PlaylistFileInfo *playlistFileInfo)
{
    /* parse the common info first */

    MediaFileSegmentInfo *mediaFileSegmentInfo;

    BDBG_MSG(("playlist file: uri %s, version %d, program id %d, b/w %d, duration (msec): total %d, max %d, cur seq %d, bounded %d",
                playlistFileInfo->uri,
                playlistFileInfo->version,
                playlistFileInfo->programId,
                playlistFileInfo->bandwidth,
                playlistFileInfo->totalDuration,
                playlistFileInfo->maxMediaSegmentDuration,
                playlistFileInfo->mediaSegmentBaseSequence,
                playlistFileInfo->bounded
                ));
    for (mediaFileSegmentInfo = BLST_Q_FIRST(&playlistFileInfo->mediaFileSegmentInfoQueueHead);
         mediaFileSegmentInfo;
         mediaFileSegmentInfo = BLST_Q_NEXT(mediaFileSegmentInfo, next))
    {

        BDBG_MSG(("media file segment info: uri %s, duration %d msec, title %s, date %s time %s, seq %d, marked discontinuity %d",
                    mediaFileSegmentInfo->uri,
                    mediaFileSegmentInfo->duration,
                    mediaFileSegmentInfo->title,
                    mediaFileSegmentInfo->date,
                    mediaFileSegmentInfo->time,
                    mediaFileSegmentInfo->mediaSequence,
                    mediaFileSegmentInfo->markedDiscontinuity
                    ));
    }

    BDBG_MSG(("%s: finished printing playlist file", BSTD_FUNCTION));

    return true;
}
#endif

/* remove current playlistFileInfoEntry in the list which is sorted (ascending order) by the b/w of the playlist entries */
/* called is required to lock the playlist */
void
B_PlaybackIp_HlsRemovePlaylistInfoEntry(HlsSessionState *hlsSession, PlaylistFileInfo *currentPlaylistFileInfo)
{
    PlaylistFileInfo *playlistFileInfo;
    BDBG_MSG(("%s: remove playlist entry %p", BSTD_FUNCTION, (void *)currentPlaylistFileInfo));
    for (playlistFileInfo = BLST_Q_FIRST(hlsSession->useIFrameTrickmodes ? &hlsSession->iFramePlaylistFileInfoQueueHead:&hlsSession->playlistFileInfoQueueHead);
         playlistFileInfo && playlistFileInfo != currentPlaylistFileInfo;
         playlistFileInfo = BLST_Q_NEXT(playlistFileInfo, next))
    { ; }
    if (playlistFileInfo) {
        /* entry found, remove it */
        BDBG_MSG(("%s: removing playlist entry: current %p, entry %p", BSTD_FUNCTION, (void *)currentPlaylistFileInfo, (void *)playlistFileInfo));
        BLST_Q_REMOVE(hlsSession->useIFrameTrickmodes ? &hlsSession->iFramePlaylistFileInfoQueueHead:&hlsSession->playlistFileInfoQueueHead, playlistFileInfo, next);
    }
    else {
        BDBG_ERR(("%s: SW BUG: entry to be removed %p not found in the list, current entry %p", BSTD_FUNCTION, (void *)currentPlaylistFileInfo, (void *)playlistFileInfo));
        BDBG_ASSERT(NULL);
    }
}

/* insert the new the playlistFileInfoEntry in the list which is sorted (ascending order) by the b/w of the playlist entries */
/* caller is required to lock the playlist */
void
B_PlaybackIp_HlsInsertPlaylistInfoEntry(HlsSessionState *hlsSession, PlaylistFileInfo *newPlaylistFileInfo)
{
    PlaylistFileInfo *playlistFileInfo;
    BDBG_MSG(("%s: insert playlist entry %p in the ascending order of bandwidth (%d)", BSTD_FUNCTION, (void *)newPlaylistFileInfo, newPlaylistFileInfo->bandwidth));
    for (playlistFileInfo = BLST_Q_FIRST(hlsSession->useIFrameTrickmodes ? &hlsSession->iFramePlaylistFileInfoQueueHead:&hlsSession->playlistFileInfoQueueHead);
         playlistFileInfo;
         playlistFileInfo = BLST_Q_NEXT(playlistFileInfo, next))
    {

        BDBG_MSG(("%s: playlist file: uri %s, b/w %d", BSTD_FUNCTION, playlistFileInfo->uri, playlistFileInfo->bandwidth));
        if (newPlaylistFileInfo->bandwidth < playlistFileInfo->bandwidth) {
            BDBG_MSG(("%s: new playlistInfo entry %p has bandwidth %d smaller than previous entry's %p bandwidth %d", BSTD_FUNCTION, (void *)newPlaylistFileInfo, newPlaylistFileInfo->bandwidth, (void *)playlistFileInfo, playlistFileInfo->bandwidth));
            break;
        }
    }
    if (playlistFileInfo) {
        /* new entry's b/w is smaller than this entry's b/w, so insert it before this entry */
        BDBG_MSG(("%s: inserting playlist entry %p before entry %p", BSTD_FUNCTION, (void *)newPlaylistFileInfo, (void *)playlistFileInfo));
        BLST_Q_INSERT_BEFORE(hlsSession->useIFrameTrickmodes ? &hlsSession->iFramePlaylistFileInfoQueueHead:&hlsSession->playlistFileInfoQueueHead, playlistFileInfo, newPlaylistFileInfo, next);
    }
    else {
        /* insert it at the tail */
        BDBG_MSG(("%s: inserting playlist entry %p at the tail", BSTD_FUNCTION, (void *)newPlaylistFileInfo));
        BLST_Q_INSERT_TAIL(hlsSession->useIFrameTrickmodes ? &hlsSession->iFramePlaylistFileInfoQueueHead:&hlsSession->playlistFileInfoQueueHead, newPlaylistFileInfo, next);
    }
}

/* connect to the server, download the complete playlist file and then parse it to build the list of media segments */
int
B_PlaybackIp_HlsConnectDownloadAndParsePlaylistFile(
    B_PlaybackIpHandle playback_ip,
    HlsSessionState *hlsSession,
    PlaylistFileInfo *playlistFileInfo
    )
{
    B_PlaybackIpHandle playbackIp; /* temp playback ip session */

    BDBG_MSG(("%s:%p Calling B_Playback_HlsCreatePlaybackIpSession ", BSTD_FUNCTION, (void *)playback_ip));
    /* Need to create a new temporary playback IP session as playlist may be downloaded using a different security parameters */
    /* than the current playback ip session (which corresponds to top level m3u8 URL) */
    playbackIp = B_Playback_HlsCreatePlaybackIpSession(playback_ip, playlistFileInfo->server, playlistFileInfo->port, &playlistFileInfo->uri, playlistFileInfo->protocol, playback_ip->openSettings.security.initialSecurityContext);
    if (!playbackIp) {
        goto error;
    }
    /* update its state to playing so that playback ip util functions can do HTTP processing (handle redirects, Http Chunk Xfer Encoding, etc.) and provide just the payload of HTTP */
    playbackIp->playback_state = B_PlaybackIpState_ePlaying;
    playbackIp->hlsSessionState = hlsSession;
    /* allocate hls session state first */
    BDBG_MSG(("%s:%p: Connected to the server using the new URL, now download the playlist, initial data %d", BSTD_FUNCTION, (void *)playbackIp, playbackIp->initial_data_len));
    if (playbackIp->initial_data_len) {
        BKNI_Memcpy(hlsSession->playlistBuffer, playbackIp->temp_buf, playbackIp->initial_data_len);
        hlsSession->playlistBufferDepth = playbackIp->initial_data_len;
        playbackIp->initial_data_len = 0;
    }
    else {
        hlsSession->playlistBufferDepth = 0;
    }

    if (B_PlaybackIp_HlsDownloadFile(playbackIp, &playbackIp->socketState.fd, hlsSession->playlistBuffer, hlsSession->playlistBufferSize, &hlsSession->playlistBufferDepth, true) != true) {
        BDBG_ERR(("%s: failed to download the M3U playlist file", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: downloaded playlist file of %d bytes", BSTD_FUNCTION, hlsSession->playlistBufferDepth));

    hlsSession->playlistBufferReadIndex = 0;

    /* verify that playlist file is in the required Extended M3U8 format */
    if (B_PlaybackIp_IsPlaylistInExtendedM3uFormat(playbackIp) != true) {
        goto error;
    }

    if (B_PlaybackIp_ParsePlaylistFile(playbackIp, playlistFileInfo, false /*parseOnlyOneMediaEntry*/) != true) {
        BDBG_ERR(("%s: failed to parse the M3U playlist file", BSTD_FUNCTION));
        goto error;
    }

    if ( (playlistFileInfo->bounded == false && playbackIpState(playback_ip) == B_PlaybackIpState_ePlaying) ||
         (playlistFileInfo->bounded == false && !hlsSession->downloadedAllSegmentsInCurrentPlaylist) ) {
        /* unbounded playlist but we haven't yet downloaded all the segments, so update the total duration. */
        /* Otherwise, once we have downloaded all the segments (meaning reached a live point), then we dont update the duration. */
        playback_ip->psi.duration = playlistFileInfo->totalDuration;
        BDBG_MSG(("%s: updating the duration with the newly updated one from the playlist to %d", BSTD_FUNCTION, playback_ip->psi.duration));
    }

    /* now destroy this temp playback ip session */
    playbackIp->hlsSessionState = NULL; /* as it was just a pointer to the parent playback_ip object's hlsSessionState object. */
    B_Playback_HlsDestroyPlaybackIpSession(playbackIp);
    return 0;

error:
    /* destroy this temp playback ip session */
    if (playbackIp) {
        playbackIp->hlsSessionState = NULL; /* as it was just a pointer to the parent playback_ip object's hlsSessionState object. */
        B_Playback_HlsDestroyPlaybackIpSession(playbackIp);
    }
    return -1;
}

#define STREAM_INFO_SIZE 512
static int
B_PlaybackIp_HlsNetIndexBounds(bfile_io_read_t self, off_t *first, off_t *last)
{
    struct bfile_io_read_net *file = (struct bfile_io_read_net *) self;
    BSTD_UNUSED(self);
    BSTD_UNUSED(file);
    BSTD_UNUSED(first);
    BSTD_UNUSED(last);

    /* since we want to probe live channels, we dont have any content length */
    return -1;
}

static off_t
B_PlaybackIp_HlsNetIndexSeek(bfile_io_read_t self, off_t offset, int whence)
{
    struct bfile_io_read_net *file = (struct bfile_io_read_net *) self;
    BSTD_UNUSED(whence);

    if (!file) {
        return -1;
    }
    file->offset = offset;
    /* since we want to probe live channels, seek doesn't mean much, just update the offset and return */
    BDBG_MSG(("%s: mediaProbe %p, file %p, offset %"PRId64 "", BSTD_FUNCTION, (void *)(B_PlaybackIpMediaProbeState *)file->playback_ip, (void *)file, offset));
    return offset;
}

static ssize_t
B_PlaybackIp_HlsNetIndexRead(bfile_io_read_t self, void *buf, size_t length)
{
    struct bfile_io_read_net *file = (struct bfile_io_read_net *) self;
    B_PlaybackIpMediaProbeState *mediaProbe;
    ssize_t bytesToRead = 0;

    if (!file) {
        BDBG_MSG(("%s: returning error (-1) due to invalid file handle %p ", BSTD_FUNCTION, (void *)file));
        return -1;
    }
    /* just casting here, ProbeCreate had correctly set the mediaProbe pointer here */
    mediaProbe = (B_PlaybackIpMediaProbeState *)file->playback_ip;

    if (mediaProbe->quickMediaProbe) {
        /* While doing quick probe, we only look in the first two TS packets. HLS Spec requires servers to encode PAT/PMT at the start of each segment. */
        if (file->offset >= 2 * TS_PKT_SIZE) {
            BDBG_MSG(("%s: forcing EOF to complate probe faster for HLS %zu, offset %"PRId64 "", BSTD_FUNCTION, length, file->offset));
            return 0;
        }
        length = 2 * TS_PKT_SIZE; /* forcing the read length to only 1st two MPEG2 TS packets */
        BDBG_MSG(("%s: trimming index read request to complate probe faster for HLS, offset %"PRId64 "", BSTD_FUNCTION, file->offset));
    }

    if (file->offset >= mediaProbe->segmentBuffer->bufferDepth) {
        BDBG_MSG(("%s: returned enough data to allow media probe to promptly return basic PSI for HLS sessions (offset %"PRId64 ", length %zu, buffer depth %d)", BSTD_FUNCTION, file->offset, length, mediaProbe->segmentBuffer->bufferDepth));
        return -1;
    }
    else if ( file->offset+(int)length >= mediaProbe->segmentBuffer->bufferDepth)
        bytesToRead = mediaProbe->segmentBuffer->bufferDepth - (size_t)file->offset;
    else
        bytesToRead = length;

    BKNI_Memcpy(buf, mediaProbe->segmentBuffer->buffer+file->offset, bytesToRead);
    BDBG_MSG(("%s: returning %zd bytes at offst %"PRId64 "", BSTD_FUNCTION, bytesToRead, file->offset));
    return bytesToRead;
}

static const struct bfile_io_read net_io_index_read = {
    B_PlaybackIp_HlsNetIndexRead,
    B_PlaybackIp_HlsNetIndexSeek,
    B_PlaybackIp_HlsNetIndexBounds,
    BIO_DEFAULT_PRIORITY
};

void
B_PlaybackIp_HlsMediaProbeStop(
    B_PlaybackIpMediaProbeState *mediaProbe
    )
{
    BDBG_ASSERT(mediaProbe);

    if (mediaProbe->stream) {
        bmedia_probe_stream_free(mediaProbe->probe, mediaProbe->stream);
        mediaProbe->stream = NULL;
    }
    BDBG_MSG(("%s: Done, hlsCtx %p", BSTD_FUNCTION, (void *)mediaProbe));
}

bool
B_PlaybackIp_HlsMediaProbeStart(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpMediaProbeState *mediaProbe,
    HlsSegmentBuffer *segmentBuffer
    )
{
    const bmedia_probe_track *track;
    bmedia_probe_config probe_config;
    bool foundAudio = false, foundVideo = false;
    B_PlaybackIpPsiInfoHandle psi;
    bool hlsEsProbed = false;
    bool hlsTsProbed = false;

    BDBG_ASSERT(mediaProbe);
    psi = &mediaProbe->psi;
    BKNI_Memset(psi, 0, sizeof(B_PlaybackIpPsiInfo));
    psi->psiValid = false;

    mediaProbe->segmentBuffer = segmentBuffer;
    mediaProbe->index.playback_ip = (B_PlaybackIpHandle)mediaProbe;
    mediaProbe->index.socketError = false;
    mediaProbe->index.self = net_io_index_read;

    while (!hlsEsProbed)
    {
        /* we probe in this sequence: Quick TS format first (only 1st 2 TS packets), then full TS segment, and if nothing is found then ES */
        if (!hlsTsProbed) {
            bmedia_probe_default_cfg(&probe_config);
            probe_config.type = bstream_mpeg_type_ts;
            probe_config.file_name = "xxx.ts";
            probe_config.probe_payload = false;
            probe_config.probe_index = false;
            probe_config.parse_index = false;
            probe_config.probe_es = false;
            probe_config.probe_all_formats = false;
            probe_config.probe_duration = false;
            if (!mediaProbe->quickMediaProbe) {
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog)
                    BDBG_WRN(("%s: HLS Session: Quick Probing for TS format", BSTD_FUNCTION));
#endif
                mediaProbe->quickMediaProbe = true;
            }
            else {
            hlsTsProbed = true;
                mediaProbe->quickMediaProbe = false;
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog)
                    BDBG_WRN(("%s: HLS Session: Full Probing for TS format", BSTD_FUNCTION));
#endif
            }
        }
        else {
            /* try for ES formats as HLS only supports either TS or ES */
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: HLS Session: Probing for ES format", BSTD_FUNCTION));
#endif
            probe_config.type = bstream_mpeg_type_es;
            probe_config.probe_payload = true;
            probe_config.probe_es = true;
            probe_config.probe_all_formats = false;
            probe_config.file_name = "xxx.aac";
            hlsEsProbed = true;
        }

        /* start the probing */
        mediaProbe->stream = bmedia_probe_parse(mediaProbe->probe, (bfile_io_read_t)&mediaProbe->index.self, &mediaProbe->probe_config);

        if (mediaProbe->stream) {
            /* probe succeeded in finding a stream */
            bmedia_stream_to_string(mediaProbe->stream, mediaProbe->stream_info, STREAM_INFO_SIZE);
            psi->mpegType = B_PlaybackIp_UtilsMpegtype2ToNexus(mediaProbe->stream->type);
            {
                if (playback_ip->hlsSessionState->currentPlaylistFile && playback_ip->hlsSessionState->currentPlaylistFile->bounded) {
                    psi->duration = playback_ip->hlsSessionState->currentPlaylistFile->totalDuration;
                }
                else {
                    /* unbounded case, i.e. live stream, so set duration to large value */
                    /* media browser needs the duration to know when to stop the playback */
                    psi->duration = 0;
                }
            }
            BDBG_WRN(("media stream info: %s", mediaProbe->stream_info));
            BDBG_WRN(("Media Details: container type %d, index %d, avg bitrate %d, duration %d, cache buffer in msec %lu, # of programs %d, # of tracks (streams) %d",
                        psi->mpegType, mediaProbe->stream->index, psi->avgBitRate, psi->duration, psi->maxBufferDuration, mediaProbe->stream->nprograms, mediaProbe->stream->ntracks));

            for (track=BLST_SQ_FIRST(&mediaProbe->stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
                if (track->type==bmedia_track_type_video && (track->info.video.codec == bvideo_codec_h264_svc || track->info.video.codec == bvideo_codec_h264_mvc)) {
                    psi->extraVideoPid = track->number;
                    psi->extraVideoCodec = B_PlaybackIp_UtilsVideoCodec2Nexus(track->info.video.codec);
                    BDBG_MSG(("extra video track %u codec:%u", track->number, psi->extraVideoCodec));
                    continue;
                }
                else if (track->type==bmedia_track_type_video && track->info.video.codec != bvideo_codec_unknown && !foundVideo) {
                    BDBG_MSG(("video track %u codec:%u", track->number, track->info.video.codec));
                    psi->videoPid = track->number;
                    psi->videoCodec = B_PlaybackIp_UtilsVideoCodec2Nexus(track->info.video.codec);
                    psi->videoHeight = track->info.video.height;
                    psi->videoWidth = track->info.video.width;
                    foundVideo = true;
                }
                else if (track->type==bmedia_track_type_pcr) {
                    BDBG_MSG(("pcr pid %u", track->number));
                    psi->pcrPid = track->number;
                }
                else if (track->type==bmedia_track_type_audio && track->info.audio.codec != baudio_format_unknown && !foundAudio) {
                    if (psi->mpegType == NEXUS_TransportType_eEs) {
                        /* In HLS protocol, ES stream is for audio only case and we convert it to PES before feeding to playpump */
                        /* so we set the transport to PES and also set the audio pid to the PES id that we are going to define for each PES pkt */
                        psi->mpegType = NEXUS_TransportType_eMpeg2Pes;
                        psi->audioPid = HLS_PES_AUDIO_ES_ID;
                        psi->audioCodec = B_PlaybackIp_UtilsAudioCodec2Nexus(track->info.audio.codec);
                        BDBG_MSG(("audio track %u codec:%u, container %d", track->number, track->info.audio.codec, psi->mpegType));
    playback_ip->mediaStartTimeNoted = true;
                    }
                    else {
                        if (psi->audioPid == 0) {
                            BDBG_MSG(("audio track %u codec:%u\n", track->number, track->info.audio.codec));
                            psi->audioPid = track->number;
                            psi->audioCodec = B_PlaybackIp_UtilsAudioCodec2Nexus(track->info.audio.codec);
                            foundAudio = true;
                        } else {
                            if(psi->extraAudioPidsCount < AUDIO_PID_MAX_COUNT) {
                                psi->extraAudioPid[psi->extraAudioPidsCount] = track->number;
                                psi->extraAudioCodec[psi->extraAudioPidsCount] = B_PlaybackIp_UtilsAudioCodec2Nexus(track->info.audio.codec);
                                psi->extraAudioPidsCount++;
                                BDBG_MSG(("%s(%d) audio track %u, codec:%u, extraAudioPidsCount : %d\n", BSTD_FUNCTION, BSTD_LINE, track->number, track->info.audio.codec, psi->extraAudioPidsCount));
                            }
                        }
                    }
                }
            }

            if (!psi->videoPid && !psi->audioPid) {
                BDBG_ERR(("%s: Video (%d) or Audio (%d) PIDs are not found during Media Probe\n", BSTD_FUNCTION, psi->videoPid, psi->audioPid));
                bmedia_probe_stream_free(mediaProbe->probe, mediaProbe->stream);
                mediaProbe->stream = NULL;
            }
            else {
                psi->psiValid = true;
                psi->hlsSessionEnabled = true;
                mediaProbe->quickMediaProbe = false;
                bmedia_probe_stream_free(mediaProbe->probe, mediaProbe->stream);
                mediaProbe->stream = NULL;
                break;
            }
        }
        else {
            /* probe didn't find the PSI info either, return error */
            BDBG_ERR(("%s: media probe didn't find the PSI info, return error", BSTD_FUNCTION));
        }
    } /* while */
    BDBG_MSG(("%s: done", BSTD_FUNCTION));
    return psi->psiValid;
}

    void
B_PlaybackIp_HlsMediaProbeDestroy(
        B_PlaybackIpMediaProbeState *mediaProbe
        )
{
    BDBG_ASSERT(mediaProbe);

    if (mediaProbe->stream_info) {
        BKNI_Free( mediaProbe->stream_info);
    }
    if (mediaProbe->stream) {
        bmedia_probe_stream_free(mediaProbe->probe, mediaProbe->stream);
        mediaProbe->stream = NULL;
    }
    if (mediaProbe->probe) {
        bmedia_probe_destroy(mediaProbe->probe);
        mediaProbe->probe = NULL;
    }
    if (mediaProbe->segmentBuffer) {
        BKNI_Free(mediaProbe->segmentBuffer);
        mediaProbe->segmentBuffer = NULL;
    }
    BDBG_MSG(("%s: Done, hlsCtx %p", BSTD_FUNCTION, (void *)mediaProbe));
}

int
B_PlaybackIp_HlsMediaProbeCreate(
    B_PlaybackIpMediaProbeState *mediaProbe,
    char *buffer,
    int bufferSize
    )
{

    BDBG_ASSERT(mediaProbe);
    if ((mediaProbe->stream_info = BKNI_Malloc(STREAM_INFO_SIZE+1)) == NULL) {
        BDBG_ERR(("%s: memory allocation failure for stream_info structure", BSTD_FUNCTION));
        return -1;
    }
    BKNI_Memset(mediaProbe->stream_info, 0, STREAM_INFO_SIZE+1);

    /* Allocate & initiate a temporary segment buffer structure to hold the initial segment data for Media probe parsing. */
    if ((mediaProbe->segmentBuffer = BKNI_Malloc(sizeof(HlsSegmentBuffer))) == NULL) {
        BDBG_ERR(("%s: memory allocation failure for HlsSegmentBuffer structure", BSTD_FUNCTION));
        return -1;
    }
    /* Initialize it using caller provided buffer. */
    BKNI_Memset(mediaProbe->segmentBuffer, 0, sizeof(HlsSegmentBuffer));
    mediaProbe->segmentBuffer->buffer = buffer;
    mediaProbe->segmentBuffer->bufferSize = bufferSize;
    BKNI_Memset(mediaProbe->segmentBuffer->buffer, 0, mediaProbe->segmentBuffer->bufferSize);

    /* Try to find PSI info using Media Probe Interface */
    mediaProbe->probe = bmedia_probe_create();
    if (!mediaProbe->probe) {
        BDBG_ERR(("%s: failed to create the probe object", BSTD_FUNCTION));
        goto error;
    }
    bmedia_probe_default_cfg(&mediaProbe->probe_config);
    mediaProbe->probe_config.probe_es = true;
    mediaProbe->probe_config.probe_payload = false;
    mediaProbe->probe_config.probe_all_formats = false;
    mediaProbe->probe_config.probe_duration = false;
    mediaProbe->probe_config.probe_index = false;
    mediaProbe->probe_config.parse_index = false;
    mediaProbe->probe_config.type = B_PlaybackIp_UtilsNexus2Mpegtype(NEXUS_TransportType_eTs);

    BDBG_MSG(("%s: done", BSTD_FUNCTION));

    return 0;

error:
    B_PlaybackIp_HlsMediaProbeDestroy(mediaProbe);
    return -1;
}

int
B_PlaybackIp_HlsConnectDownloadAndParsePlaylistFileAnd1stSegment(
    B_PlaybackIpHandle playback_ip,
    HlsSessionState *hlsSession,
    PlaylistFileInfo *playlistFileInfo,
    HlsSegmentBuffer *segmentBuffer
    )
{
    B_PlaybackIpHandle playbackIp; /* temp playback ip session */
    MediaFileSegmentInfo *mediaFileSegmentInfo;
    unsigned networkBandwidth;
    bool serverClosed;
    unsigned bytesToRead;

    /*
     * Download the playlist first.
     */
    BDBG_MSG(("%s:%p Calling B_Playback_HlsCreatePlaybackIpSession ", BSTD_FUNCTION, (void *)playback_ip));
    /* Need to create a new temporary playback IP session as playlist may be downloaded using a different security parameters */
    /* than the current playback ip session (which corresponds to top level m3u8 URL) */
    playbackIp = B_Playback_HlsCreatePlaybackIpSession(playback_ip, playlistFileInfo->server, playlistFileInfo->port, &playlistFileInfo->uri, playlistFileInfo->protocol, playback_ip->openSettings.security.initialSecurityContext);
    if (!playbackIp) {
        goto error;
    }

    /* update its state to playing so that playback ip util functions can do HTTP processing (handle redirects, Http Chunk Xfer Encoding, etc.) and provide just the payload of HTTP */
    playbackIp->playback_state = B_PlaybackIpState_ePlaying;
    playbackIp->hlsSessionState = hlsSession;

    BDBG_MSG(("%s: Connected to the server using the new URL, now download the playlist, initial data %d", BSTD_FUNCTION, playbackIp->initial_data_len));

    /* save initiate data that was read alongside the response. */
    if (playbackIp->initial_data_len) {
        BKNI_Memcpy(hlsSession->playlistBuffer, playbackIp->temp_buf, playbackIp->initial_data_len);
        hlsSession->playlistBufferDepth = playbackIp->initial_data_len;
        playbackIp->initial_data_len = 0;
    }
    else {
        hlsSession->playlistBufferDepth = 0;
    }

    /* Now download the playlist file. */
    if (B_PlaybackIp_HlsDownloadFile(playbackIp, &playbackIp->socketState.fd, hlsSession->playlistBuffer, hlsSession->playlistBufferSize, &hlsSession->playlistBufferDepth, true) != true) {
        BDBG_ERR(("%s: failed to download the M3U playlist file", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: downloaded playlist file of %d bytes", BSTD_FUNCTION, hlsSession->playlistBufferDepth));

    hlsSession->playlistBufferReadIndex = 0;

    /* verify that playlist file is in the required Extended M3U8 format */
    if (B_PlaybackIp_IsPlaylistInExtendedM3uFormat(playbackIp) != true) {
        goto error;
    }

    if (B_PlaybackIp_ParsePlaylistFile(playbackIp, playlistFileInfo, true /*parseOnlyOneMediaEntry*/) != true) {
        BDBG_ERR(("%s: failed to parse the M3U playlist file", BSTD_FUNCTION));
        goto error;
    }

    BDBG_WRN(("%s: Downloaded & parsed playlist w/ uri=%s", BSTD_FUNCTION, playlistFileInfo->absoluteUri));

    /* Now select the first mediaSegment that we can download for probing. */
    for (mediaFileSegmentInfo = BLST_Q_FIRST(&playlistFileInfo->mediaFileSegmentInfoQueueHead);
            mediaFileSegmentInfo;
            mediaFileSegmentInfo = BLST_Q_FIRST(&playlistFileInfo->mediaFileSegmentInfoQueueHead))
    {
        if (B_PlaybackIp_HlsSetupHttpSessionToServer(playbackIp, mediaFileSegmentInfo, 0 /*persistentHttpSession*/, 1 /*reOpenSocketConnection */, &playbackIp->socketState.fd, segmentBuffer->buffer, &segmentBuffer->bufferDepth) == false)
        {
            BDBG_ERR(("%s: ERROR: Socket setup or HTTP request/response failed for downloading Media Segment, retrying next media segment", BSTD_FUNCTION));
            /* remove this media segment as we met some error in downloading this segment (most likely segment URL has expired) */
            BLST_Q_REMOVE_HEAD(&playlistFileInfo->mediaFileSegmentInfoQueueHead, next);
            B_PlaybackIp_FreeMediaFileSegmentInfo(mediaFileSegmentInfo);
            continue;
        }
        /* successfully connected & got the GET request/response from the server, so move on */
        break;
    }

    if (mediaFileSegmentInfo == NULL) {
        BDBG_ERR(("%s: ERROR: Failed to download any media segment of playlist file (uri %s) for media probe", BSTD_FUNCTION, playlistFileInfo->absoluteUri));
        goto error;
    }
    BDBG_MSG(("%s: SetupSession for downloading a media segment uri=%s", BSTD_FUNCTION, mediaFileSegmentInfo->absoluteUri));


    bytesToRead = mediaFileSegmentInfo->segmentLength ?
        mediaFileSegmentInfo->segmentLength - (unsigned)segmentBuffer->bufferDepth:
        (unsigned) segmentBuffer->bufferSize-segmentBuffer->bufferDepth;

    if (B_PlaybackIp_HlsDownloadMediaSegment(playbackIp, playbackIp->socketState.fd, segmentBuffer->buffer+segmentBuffer->bufferDepth, segmentBuffer->bufferSize, bytesToRead, &segmentBuffer->bufferDepth, &networkBandwidth, &serverClosed) != true) {
        BDBG_ERR(("%s: failed to download the current media segment of an alternate rendition for media probe!", BSTD_FUNCTION));
        if (playbackIp->securityHandle) {
            playbackIp->netIo.close(playbackIp->securityHandle);
            playbackIp->securityHandle = NULL;
        }
        close(playbackIp->socketState.fd);
        playbackIp->socketState.fd = -1;
        goto error;
    }
    if (0)
    {
        /* save the media segment. */
        FILE *fp;
        fp = fopen("./segAudio.ts", "ab+");
        fwrite(segmentBuffer->buffer, 1, segmentBuffer->bufferDepth, fp);
        fflush(fp);
        fclose(fp);
    }
    BDBG_MSG(("%s: Downloaded 1st media segment w/ uri=%s", BSTD_FUNCTION, mediaFileSegmentInfo->absoluteUri));

    if (playbackIp->securityHandle) {
        playbackIp->netIo.close(playbackIp->securityHandle);
        playbackIp->securityHandle = NULL;
    }
    close(playbackIp->socketState.fd);
    playbackIp->socketState.fd = -1;
    /* now destroy this temp playback ip session */
    playbackIp->hlsSessionState = NULL; /* as it was just a pointer to the parent playback_ip object's hlsSessionState object. */
    B_Playback_HlsDestroyPlaybackIpSession(playbackIp);
    return 0;

error:
    /* destroy this temp playback ip session */
    if (playbackIp) {
        playbackIp->hlsSessionState = NULL; /* as it was just a pointer to the parent playback_ip object's hlsSessionState object. */
        B_Playback_HlsDestroyPlaybackIpSession(playbackIp);
    }
    return -1;
}

#ifdef HLS_UNIT_TEST
static bool
printVariantPlaylistFile(HlsSessionState *hlsSession)
{
    /* parse the common info first */
    PlaylistFileInfo *playlistFileInfo;

    BDBG_MSG(("%s: playlist file q head %p, current playlist file %p", BSTD_FUNCTION, (void *)&hlsSession->playlistFileInfoQueueHead, (void *)hlsSession->currentPlaylistFile));
    for (playlistFileInfo = BLST_Q_FIRST(&hlsSession->playlistFileInfoQueueHead);
         playlistFileInfo;
         playlistFileInfo = BLST_Q_NEXT(playlistFileInfo, next))
    {

        BDBG_MSG(("%s: playlist file entry %p: uri %s, version %d, program id %d, b/w %d, duration (msec): total %d, max %d, cur seq %d, bounded %d, playlist file downloaded %s",
                BSTD_FUNCTION,
                (void *)playlistFileInfo,
                playlistFileInfo->uri,
                playlistFileInfo->version,
                playlistFileInfo->programId,
                playlistFileInfo->bandwidth,
                playlistFileInfo->totalDuration,
                playlistFileInfo->maxMediaSegmentDuration,
                playlistFileInfo->mediaSegmentBaseSequence,
                playlistFileInfo->bounded,
                playlistFileInfo->currentMediaFileSegment ? "YES":"NO"
                ));
        /* verify the playlist file */
        printPlaylistFile(playlistFileInfo);
    }

    BDBG_MSG(("%s: finished printing variant playlist file", BSTD_FUNCTION));

    return true;
}

static bool
printVariantIFramePlaylistFile(HlsSessionState *hlsSession)
{
    /* parse the common info first */
    PlaylistFileInfo *playlistFileInfo;

    BDBG_WRN(("%s: playlist file q head %p, current playlist file %p", BSTD_FUNCTION, (void *)&hlsSession->iFramePlaylistFileInfoQueueHead, (void *)hlsSession->currentPlaylistFile));
    for (playlistFileInfo = BLST_Q_FIRST(&hlsSession->iFramePlaylistFileInfoQueueHead);
         playlistFileInfo;
         playlistFileInfo = BLST_Q_NEXT(playlistFileInfo, next))
    {

        BDBG_WRN(("%s: playlist file entry %p: uri %s, version %d, program id %d, b/w %d, duration (msec): total %lu, max %lu, cur seq %d, bounded %d, playlist file downloaded %s",
                BSTD_FUNCTION,
                (void *)playlistFileInfo,
                playlistFileInfo->uri,
                playlistFileInfo->version,
                playlistFileInfo->programId,
                playlistFileInfo->bandwidth,
                playlistFileInfo->totalDuration,
                playlistFileInfo->maxMediaSegmentDuration,
                playlistFileInfo->mediaSegmentBaseSequence,
                playlistFileInfo->bounded,
                playlistFileInfo->currentMediaFileSegment ? "YES":"NO"
                ));
#if 0
        /* verify the playlist file */
        printPlaylistFile(playlistFileInfo);
#endif
    }

    BDBG_MSG(("%s: finished printing variant playlist file", BSTD_FUNCTION));

    return true;
}
#endif

bool
isAudioCodecPresent(char *codec)
{
    if (strcasestr(codec, "mp4a"))
        return true;
    else
        return false;
}

bool
isVideoCodecPresent(char *codec)
{
    if (strcasestr(codec, "mp4v") || strcasestr(codec, "avc") )
        return true;
    else
        return false;
}

HlsSegmentCodecType
parseCodecString(B_PlaybackIpHandle playback_ip, char *codec)
{
    bool audio = false, video = false;
    if (!playback_ip || !codec)
        return HlsSegmentCodecType_eUnknown;
    audio = isAudioCodecPresent(codec);
    video = isVideoCodecPresent(codec);
    if (audio && video)
        return HlsSegmentCodecType_eAudioVideo;
    else if (audio )
        return HlsSegmentCodecType_eAudioOnly;
    else if (video )
        return HlsSegmentCodecType_eVideoOnly;
    else
        return HlsSegmentCodecType_eUnknown;

}

#if 0
bool
parseMediaTagInMasterPlaylist(B_PlaybackIpHandle playback_ip, const char *pMediaTag)
{
    /*
     * Media Tag Example:
#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="group01",NAME="eng",DEFAULT=YES,AUTOSELECT=YES,LANGUAGE="en",URI="07.m3u8"
#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="group01",NAME="spa",DEFAULT=NO,AUTOSELECT=YES,LANGUAGE="es",URI="08.m3u8"
#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=688832,RESOLUTION=480x360,CODECS="avc1.42e00a,mp4a.40.2",AUDIO="group01"
03.m3u8
#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=1013696,RESOLUTION=480x360,CODECS="avc1.42e00a,mp4a.40.2",AUDIO="group01"
04.m3u8
#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=1670944,RESOLUTION=544x480,CODECS="avc1.4d400a,mp4a.40.2",AUDIO="group01"
05.m3u8
#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=2451520,RESOLUTION=544x480,CODECS="avc1.4d400a,mp4a.40.2",AUDIO="group01"
06.m3u8
     */

    /*
     * Media Tag Format:
     * #EXT-X-MEDIA:<attribute-list>
     *
     * Attributes can be required or optional:
     * Required --> TYPE, GROUD-ID, NAME,
     * Optional --> URI, LANGUAGE, DEFAULT, AUTOSELECT,
     *
     * TYPE: Enumerated String, valid strings: AUDIO, VIDEO, SUBTITLES, CLOSED-CAPTIONS
     * URI:  quoted string, must not be present for CLOSED-CAPTIONS
     * GROUP-ID: quoted-string -> which group does this rendition belong to.
     * LANGUAGE: standard tag for identifying language [RFC5646]
     * NAME: quoted string, rendition description
     * DEFAULT: enumerated string, valid strings: YES & NO. If yes, play this rendition if its URl is present. Absence of this tag indicates a value of NO.
     * AUTOSELECT: enumerated string, valid strings: YES & NO. If yes, play this rendition if its URl is present. Absence of this tag indicates a value of NO.
     */
}

#endif
/* parses the downloaded file to see if it a simple or variant playlist */
/* for simple playlist, it just creates the playlistFileInfo structure and returns that */
/* for variant playlist, it parses the variant playlist file and builds the list of playlists */
PlaylistFileInfo *
B_PlaybackIp_ParsePlaylistVariantFile(B_PlaybackIpHandle playback_ip, bool *parsingSuccess)
{
    /* parse the common info first */
    char *nextLine = NULL, *tag = NULL;
    int nextLineLength = 0;
    int rc;
    int programId = -1;
    unsigned smallestBandwidth = 0;
    PlaylistFileInfo *playlistFileInfo = NULL;
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;
    bool versionTagFound = false;
    short version = PLAYLIST_FILE_COMPATIBILITY_VERSION;
    AltRenditionInfo *altRenditionInfo = NULL;

    *parsingSuccess = false;

    /* make sure we ignore any initial comments in the playlist file */
    while (true) {
        if ((rc =B_PlaybackIp_UtilsGetLine(&nextLine, &nextLineLength, hlsSession->playlistBuffer+hlsSession->playlistBufferReadIndex, hlsSession->playlistBufferDepth-hlsSession->playlistBufferReadIndex)) < 0) {
            BDBG_ERR(("%s: Failed to get the next line from the uri file", BSTD_FUNCTION));
            return NULL;
        }
        BDBG_MSG(("%s: next line %s", BSTD_FUNCTION, nextLine));
        /* any lines that dont start w/ #EXT are comments, so we need to ignore them */
        if ((B_PlaybackIp_UtilsStristr(nextLine, "#EXT")) != NULL) {
            /* line begins w/ #EXT tag, so we are good */
            break;
        }
        /* comment line, ignore it */
        hlsSession->playlistBufferReadIndex += rc;
        BKNI_Free(nextLine);
        nextLine = NULL;
    }

    BDBG_MSG(("%s: next line %s", BSTD_FUNCTION, nextLine));
    if ((tag = B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_VERSION_TAG)) != NULL) {
        if (versionTagFound == true) {
            BDBG_ERR(("%s: Invalid Playlist file as it contains > 1 %s tag", BSTD_FUNCTION, HLS_M3U8_VERSION_TAG));
            goto error;
        }
        tag += strlen(HLS_M3U8_VERSION_TAG);
        version = strtol(tag, NULL, 10);
        if (version > PLAYLIST_FILE_COMPATIBILITY_VERSION) {
            BDBG_WRN(("%s: Not all features of HLS Version %d are currently supported %d", BSTD_FUNCTION, version, PLAYLIST_FILE_COMPATIBILITY_VERSION));
        }
        versionTagFound = true;
        BDBG_MSG(("%s: Playlist VERSION %d", BSTD_FUNCTION, version));
        hlsSession->playlistBufferReadIndex += rc;
        BKNI_Free(nextLine);
        nextLine = NULL;
        if ((rc =B_PlaybackIp_UtilsGetLine(&nextLine, &nextLineLength, hlsSession->playlistBuffer+hlsSession->playlistBufferReadIndex, hlsSession->playlistBufferDepth-hlsSession->playlistBufferReadIndex)) < 0) {
            BDBG_ERR(("%s: Failed to get the next line from the uri file", BSTD_FUNCTION));
            goto error;
        }

        BDBG_MSG(("%s: next line %s", BSTD_FUNCTION, nextLine));
    }
    /* Check to see if variant playlist tag is present */
    if (B_PlaybackIp_UtilsStristr(hlsSession->playlistBuffer, HLS_M3U8_STREAM_INFO_TAG) == NULL) {
        /* variant playinfo tag is missing, so uri contains the actual playlist */
        if (hlsSession->currentPlaylistFile != NULL) {
            BDBG_MSG(("%s: current file is not a variant playlist file and variant playlist is already created, return", BSTD_FUNCTION));
            return NULL;
        }

        /* allocate/initialize playlist file structure and return */
        if ( (playlistFileInfo = (PlaylistFileInfo *)BKNI_Malloc(sizeof(PlaylistFileInfo))) == NULL) {
            BDBG_ERR(("%s: Failed to allocate %zu bytes of memory for playlistInfo file structure", BSTD_FUNCTION, sizeof(PlaylistFileInfo)));
            return NULL;
        }
        memset(playlistFileInfo, 0, sizeof(PlaylistFileInfo));
        /* build the uri of the playlist file */
        if ((playlistFileInfo->absoluteUri = B_PlaybackIp_HlsBuildAbsoluteUri(playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port, ""/* empty base uri*/, playback_ip->openSettings.socketOpenSettings.url)) == NULL) {
            BDBG_ERR(("Failed to build URI at %s:%d", BSTD_FUNCTION, __LINE__));
            goto error;
        }
        if ((hls_parse_url(&playlistFileInfo->protocol, &playlistFileInfo->server, &playlistFileInfo->port, &playlistFileInfo->uri, playlistFileInfo->absoluteUri) == false) || (playlistFileInfo->protocol != B_PlaybackIpProtocol_eHttp && playlistFileInfo->protocol != B_PlaybackIpProtocol_eHttps)) {
            BDBG_ERR(("Failed to parse URI at %s:%d", BSTD_FUNCTION, __LINE__));
            goto error;
        }
        if (versionTagFound)
            playlistFileInfo->version = version;
        else
            playlistFileInfo->version = PLAYLIST_FILE_COMPATIBILITY_VERSION;
        /* rest of fields are 0 */

        /* initialize the queue */
        BLST_Q_INIT(&hlsSession->playlistFileInfoQueueHead);
        /* in the case of simple playlist file, it will only have element */
        BLST_Q_INSERT_TAIL(&hlsSession->playlistFileInfoQueueHead, playlistFileInfo, next);
        hlsSession->currentPlaylistFile = playlistFileInfo;
        BDBG_MSG(("%s: given uri (%s) is not a variant playlist file (%p)", BSTD_FUNCTION, playlistFileInfo->absoluteUri, (void *)playlistFileInfo));
        BKNI_Free(nextLine);
        *parsingSuccess = true;
        return playlistFileInfo;
    }

    if (nextLine)
        BKNI_Free(nextLine);

    BLST_Q_INIT(&hlsSession->altRenditionInfoQueueHead);
    BLST_Q_INIT(&hlsSession->iFramePlaylistFileInfoQueueHead);
    /* parse the variant playlist file */
    for (
            (nextLine = NULL);
            ((rc = B_PlaybackIp_UtilsGetLine(&nextLine, &nextLineLength, hlsSession->playlistBuffer+hlsSession->playlistBufferReadIndex, hlsSession->playlistBufferDepth-hlsSession->playlistBufferReadIndex)) > 0);
            (hlsSession->playlistBufferReadIndex += rc, BKNI_Free(nextLine), nextLine = NULL)
        )
    {
        /* now parse this line */
        BDBG_MSG(("%s: next line is line size %d, index %d, %s", BSTD_FUNCTION, rc, hlsSession->playlistBufferReadIndex, nextLine));
        if ( (tag = B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_STREAM_INFO_TAG)) != NULL) {
            char *tmp, *attribute;
            /* variant stream info tag: allocate & initialize another playlistInfo entry */
            if ((playlistFileInfo = (PlaylistFileInfo *)BKNI_Malloc(sizeof(PlaylistFileInfo))) == NULL) {
                BDBG_ERR(("%s: Failed to allocate %d bytes of memory for playlistInfo file structure", BSTD_FUNCTION, (int)sizeof(PlaylistFileInfo)));
                return NULL;
            }
            memset(playlistFileInfo, 0, sizeof(PlaylistFileInfo));
            playlistFileInfo->version = PLAYLIST_FILE_COMPATIBILITY_VERSION;
            /* parse the attributes of this tag, its format is:
                EXT-X-STREAM-INF:[attribute=value][,attribute=value]*
                URI
             */
            for (
                    (tmp = attribute = tag + strlen(HLS_M3U8_STREAM_INFO_TAG));    /* points to 1st attribute */
                    (tmp && ((tmp = strstr(attribute, "=")) != NULL));
                    (attribute = tmp+1)
                )
            {
                char *value;
                *tmp = '\0'; /* null terminate attribute to convert it into a string */
                value = tmp+1;
                /* multiple attributes are separate by , */
                if ((tmp = strstr(value, ",")) != NULL) {
                    /* another attribute is present following this attribute=value pair */
                    *tmp = '\0';    /* null terminate value to convert it into a string */
                }
                if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_PROGRAM_ID) != NULL) {
                    playlistFileInfo->programId = strtol(value, NULL, 10);
                    if (programId == -1)
                        programId = playlistFileInfo->programId;
                    else {
                        if (programId != playlistFileInfo->programId) {
                            BDBG_ERR(("%s: no support for multiple programs yet: Program ID current %d, new %d", BSTD_FUNCTION, programId, playlistFileInfo->programId));
                            /* TODO: goto free up current entry and move to the next program entry */
                            goto error;
                        }
                    }
                    BDBG_MSG(("%s: Program ID %d", BSTD_FUNCTION, playlistFileInfo->programId));
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_BANDWIDTH) != NULL) {
                    playlistFileInfo->bandwidth = strtol(value, NULL, 10);
                    if (playlistFileInfo->bandwidth == 0) {
                        BDBG_ERR(("%s: invalid bandwidth (%d) in the variant playlist, ignoring it", BSTD_FUNCTION, playlistFileInfo->bandwidth));
                    }
                    BDBG_MSG(("%s: Bandwidth %d", BSTD_FUNCTION, playlistFileInfo->bandwidth));
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_CODECS) != NULL) {
                    if (tmp)
                        /* codec attribute can have 1 or more values, each value is separated by a comma, replace this comma is present w/ \0 , and then look for ending " */
                        *tmp = ',';
                    BDBG_MSG(("%s: TODO: need to parse attribute %s=%s", BSTD_FUNCTION, HLS_M3U8_TAG_ATTRIBUTE_CODECS, value));
                    /* The syntax for CODECS attribute is: CODECS="[format][,format]" */
                    /* so we need to skip the initial " char & then jump to the next " char */
                    value += 1; /* skips starting " char */
                    if ((tmp = strstr(value, "\"")) != NULL) {
                        *tmp = '\0';
                        playlistFileInfo->segmentCodecType = parseCodecString(playback_ip, value);
                        BDBG_MSG(("Codec string: %s, segmentCodecType %d", value, playlistFileInfo->segmentCodecType));
                    }
                    else {
                        /* missing ending " char, so it an error as CODECS attribute is enclosed in the " " */
                        BDBG_ERR(("%s: missing ending \" char, so it an error as CODECS attribute is enclosed in the \" \", attribute: %s", BSTD_FUNCTION, value));
                        goto error;
                    }
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_RESOLUTION) != NULL) {
                    BDBG_MSG(("%s: TODO: need to parse attribute %s=%s", BSTD_FUNCTION, HLS_M3U8_TAG_ATTRIBUTE_RESOLUTION, value));
                    /* TODO: Add parsing of VIDEO, & SUB-TITLES attributes. */
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_AUDIO) != NULL) {
                    playlistFileInfo->altAudioRenditionsEnabled = true;
                    if ((playlistFileInfo->altAudioRenditionGroupId = B_PlaybackIp_UtilsStrdup(value)) == NULL) {
                        BDBG_ERR(("%s: Failed to allocate %zu bytes of memory for absolute URI ", BSTD_FUNCTION, strlen(value)));
                        goto error;
                    }
                    BDBG_MSG(("%s: AUDIO attribute %s=%s", BSTD_FUNCTION, HLS_M3U8_TAG_ATTRIBUTE_CODECS, playlistFileInfo->altAudioRenditionGroupId));
                }
                else {
                    BDBG_ERR(("%s: unsupported/unknown attribute %s=%s, ignoring it for now!!", BSTD_FUNCTION, attribute, value));
                }
                BDBG_MSG(("%s: tmp %p, attribute %p, %s", BSTD_FUNCTION, (void *)tmp, (void *)attribute, attribute));
            }

            /* finished parsing the STREAM_INFO TAG & its attributes, next line is the uri of the playlist file, parse that */
            hlsSession->playlistBufferReadIndex += rc;
            BKNI_Free(nextLine);
            nextLine = NULL;
            if ((rc =B_PlaybackIp_UtilsGetLine(&nextLine, &nextLineLength, hlsSession->playlistBuffer+hlsSession->playlistBufferReadIndex, hlsSession->playlistBufferDepth-hlsSession->playlistBufferReadIndex)) > 0) {
                if (http_absolute_uri(nextLine)) {
                    /* playlist contains the absolute uri, copy it */
                    BDBG_MSG(("%s: absolute URI: %s", BSTD_FUNCTION, nextLine));
                    if ((playlistFileInfo->absoluteUri = B_PlaybackIp_UtilsStrdup(nextLine)) == NULL) {
                        BDBG_ERR(("%s: Failed to allocate %zu bytes of memory for absolute URI ", BSTD_FUNCTION, strlen(nextLine)));
                        goto error;
                    }
                }
                else {
                    /* relative url, build complete uri using server ip address & port # */
                    BDBG_MSG(("%s: %s is not absolute URI", BSTD_FUNCTION, nextLine));
                    if ((playlistFileInfo->absoluteUri = B_PlaybackIp_HlsBuildAbsoluteUri(playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port, playback_ip->openSettings.socketOpenSettings.url, nextLine)) == NULL) {
                        BDBG_ERR(("Failed to build URI at %s:%d", BSTD_FUNCTION, __LINE__));
                        goto error;
                    }
                }
                if (hls_parse_url(&playlistFileInfo->protocol, &playlistFileInfo->server, &playlistFileInfo->port, &playlistFileInfo->uri, playlistFileInfo->absoluteUri) == false) {
                    BDBG_ERR(("Failed to parse URI at %s:%d", BSTD_FUNCTION, __LINE__));
                    goto error;
                }
                BDBG_MSG(("%s: Variant Playlist: server %s, port %d, protocol %d, uri %s, b/w %d, program id %d", BSTD_FUNCTION, playlistFileInfo->server, playlistFileInfo->port, playlistFileInfo->protocol, playlistFileInfo->uri, playlistFileInfo->bandwidth, playlistFileInfo->programId));
            }
            else {
                BDBG_ERR(("Failed to get next line at %s:%d", BSTD_FUNCTION, __LINE__));
                goto error;
            }

            /* initialize the queue */
            if (hlsSession->currentPlaylistFile == NULL) {
                BLST_Q_INIT(&hlsSession->playlistFileInfoQueueHead);
                hlsSession->currentPlaylistFile = playlistFileInfo;
                smallestBandwidth = playlistFileInfo->bandwidth;
                BDBG_MSG(("%s: initialized head for the playlist info queue, playlistInfo entry %p, bandwidth %d", BSTD_FUNCTION, (void *)playlistFileInfo, playlistFileInfo->bandwidth));
            }
            else {
                /* bitrate check here */
                if (smallestBandwidth > playlistFileInfo->bandwidth) {
                    BDBG_MSG(("%s: playlistInfo entry %p has bandwidth %d smaller than previous one %d", BSTD_FUNCTION, (void *)playlistFileInfo, playlistFileInfo->bandwidth, smallestBandwidth));
                    smallestBandwidth = playlistFileInfo->bandwidth;
                    hlsSession->currentPlaylistFile = playlistFileInfo;
                }
            }

            /* now insert this entry into the queue of playlist files */
            BKNI_AcquireMutex(hlsSession->lock);
            B_PlaybackIp_HlsInsertPlaylistInfoEntry(hlsSession, playlistFileInfo);
            BKNI_ReleaseMutex(hlsSession->lock);
            playlistFileInfo = NULL; /* reset pointer as this playlist is successfully inserted into the hlsSession */
        }
        else if ( (tag = B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_MEDIA_TAG)) != NULL) {
            char *tmp, *attribute;

            /* media tag: allocate & initialize another altRenditionInfo entry. */
            if ((altRenditionInfo = (AltRenditionInfo *)BKNI_Malloc(sizeof(AltRenditionInfo))) == NULL) {
                BDBG_ERR(("%s: Failed to allocate %d bytes of memory for altRenditionInfo structure", BSTD_FUNCTION, (int)sizeof(AltRenditionInfo)));
                return NULL;
            }
            memset(altRenditionInfo, 0, sizeof(AltRenditionInfo));
            /* parse the attributes of this tag, its format is:
                EXT-X-MEDIA:[attribute=value][,attribute=value]*
                E.g.:
                #EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="group01",NAME="eng",DEFAULT=YES,AUTOSELECT=YES,LANGUAGE="en",URI="07.m3u8"
                #EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="aac",NAME="Spanish",LANGUAGE="spa",AUTOSELECT=YES,DEFAULT=NO,URI="http://content-ausc1.uplynk.com/channel/4
             */
            for (
                    (tmp = attribute = tag + strlen(HLS_M3U8_MEDIA_TAG));    /* points to 1st attribute */
                    (tmp && ((tmp = strstr(attribute, "=")) != NULL));
                    (attribute = tmp+1)
                )
            {
                char *value;
                *tmp = '\0'; /* null terminate attribute to convert it into a string */
                value = tmp+1;
                /* multiple attributes are separate by , */
                if ((tmp = strstr(value, ",")) != NULL) {
                    /* another attribute is present following this attribute=value pair */
                    *tmp = '\0';    /* null terminate value to convert it into a string */
                }
                if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_TYPE) != NULL) {
                    if (B_PlaybackIp_UtilsStristr(value, "AUDIO") != NULL)
                        altRenditionInfo->type = HlsAltRenditionType_eAudio;
                    else if (B_PlaybackIp_UtilsStristr(value, "VIDEO") != NULL)
                        altRenditionInfo->type = HlsAltRenditionType_eVideo;
                    else if (B_PlaybackIp_UtilsStristr(value, "SUBTITLES") != NULL)
                        altRenditionInfo->type = HlsAltRenditionType_eSubtitles;
                    else if (B_PlaybackIp_UtilsStristr(value, "CLOSED-CAPTIONS") != NULL)
                        altRenditionInfo->type = HlsAltRenditionType_eClosedCaptions;
                    else {
                        BDBG_ERR(("%s: Unsupported value=%s for TYPE Attribute ", BSTD_FUNCTION, value ));
                        /* TODO: goto free up current entry and move to the next program entry */
                        goto error;
                    }
                    BDBG_MSG(("%s: AltRendition TYPE %s", BSTD_FUNCTION,
                                altRenditionInfo->type == HlsAltRenditionType_eAudio ? "AUDIO":
                                altRenditionInfo->type == HlsAltRenditionType_eVideo ? "VIDEO":
                                altRenditionInfo->type == HlsAltRenditionType_eSubtitles ? "SUBTITLES":
                                "CLOSED-CAPTIONS"
                                ));
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_GROUP_ID) != NULL) {
                    /* GroupId attribute's value is string-quoted, so we will need to remove leading & ending " from it. */
                    value += 1; /* skip past the initial " char */
                    value[strlen(value)-1] = '\0';
                    if ((altRenditionInfo->groupId = B_PlaybackIp_UtilsStrdup(value)) == NULL) {
                        BDBG_ERR(("%s: Failed to allocate %zu bytes of memory for GroupId attribute", BSTD_FUNCTION, strlen(value)));
                        goto error;
                    }
                    BDBG_MSG(("%s: GroupId attribute %s=%s", BSTD_FUNCTION, HLS_M3U8_TAG_ATTRIBUTE_GROUP_ID, altRenditionInfo->groupId));
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_NAME) != NULL) {
                    /* GroupId attribute's value is string-quoted, so we will need to remove leading & ending " from it. */
                    value += 1; /* skip past the initial " char */
                    value[strlen(value)-1] = '\0';
                    if ((altRenditionInfo->name = B_PlaybackIp_UtilsStrdup(value)) == NULL) {
                        BDBG_ERR(("%s: Failed to allocate %zu bytes of memory for Name attribute ", BSTD_FUNCTION, strlen(value)));
                        goto error;
                    }
                    BDBG_MSG(("%s: Name attribute %s=%s", BSTD_FUNCTION, HLS_M3U8_TAG_ATTRIBUTE_NAME, altRenditionInfo->name));
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_LANGUAGE) != NULL) {
                    /* GroupId attribute's value is string-quoted, so we will need to remove leading & ending " from it. */
                    value += 1; /* skip past the initial " char */
                    value[strlen(value)-1] = '\0';
                    if ((altRenditionInfo->language = B_PlaybackIp_UtilsStrdup(value)) == NULL) {
                        BDBG_ERR(("%s: Failed to allocate %zu bytes of memory for Language attribute ", BSTD_FUNCTION, strlen(value)));
                        goto error;
                    }
                    BDBG_MSG(("%s: language attribute %s=%s", BSTD_FUNCTION, HLS_M3U8_TAG_ATTRIBUTE_LANGUAGE, altRenditionInfo->language));
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_DEFAULT) != NULL) {
                    if (B_PlaybackIp_UtilsStristr(value, "YES") != NULL)
                        altRenditionInfo->defaultRendition = true;
                    else
                        altRenditionInfo->defaultRendition = false;
                    BDBG_MSG(("%s: Default attribute %s=%s", BSTD_FUNCTION, HLS_M3U8_TAG_ATTRIBUTE_DEFAULT, altRenditionInfo->defaultRendition?"YES":"NO"));
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_AUTOSELECT) != NULL) {
                    if (B_PlaybackIp_UtilsStristr(value, "YES") != NULL)
                        altRenditionInfo->autoSelect = true;
                    else
                        altRenditionInfo->autoSelect = false;
                    BDBG_MSG(("%s: Default attribute %s=%s", BSTD_FUNCTION, HLS_M3U8_TAG_ATTRIBUTE_AUTOSELECT, altRenditionInfo->autoSelect?"YES":"NO"));
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_URI) != NULL) {
                    /* GroupId attribute's value is string-quoted, so we will need to remove leading & ending " from it. */
                    value += 1; /* skip past the initial " char */
                    value[strlen(value)-1] = '\0';
                    if (http_absolute_uri(value)) {
                        /* alternateRendition URI tag contains the absolute uri, copy it */
                        BDBG_MSG(("%s: absolute URI: %s", BSTD_FUNCTION, value));
                        if ((altRenditionInfo->absoluteUri = B_PlaybackIp_UtilsStrdup(value)) == NULL) {
                            BDBG_ERR(("%s: Failed to allocate %zu bytes of memory for absolute URI ", BSTD_FUNCTION, strlen(value)));
                            goto error;
                        }
                    }
                    else {
                        /* relative url, build complete uri using server ip address & port # */
                        BDBG_MSG(("%s: %s is not absolute URI", BSTD_FUNCTION, value));
                        if ((altRenditionInfo->absoluteUri = B_PlaybackIp_HlsBuildAbsoluteUri(playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port, playback_ip->openSettings.socketOpenSettings.url, value)) == NULL) {
                            BDBG_ERR(("Failed to build URI at %s:%d", BSTD_FUNCTION, __LINE__));
                            goto error;
                        }
                    }
                    if (hls_parse_url(&altRenditionInfo->protocol, &altRenditionInfo->server, &altRenditionInfo->port, &altRenditionInfo->uri, altRenditionInfo->absoluteUri) == false) {
                        BDBG_ERR(("Failed to parse URI at %s:%d", BSTD_FUNCTION, __LINE__));
                        goto error;
                    }
                    BDBG_MSG(("%s: Variant Playlist: server %s, port %d, protocol %d, uri %s", BSTD_FUNCTION, altRenditionInfo->server, altRenditionInfo->port, altRenditionInfo->protocol, altRenditionInfo->uri));
                }
                else {
                    BDBG_ERR(("%s: unsupported/unknown attribute %s=%s, ignoring it for now!!", BSTD_FUNCTION, attribute, value));
                }
            }

            /* now insert this entry into the queue of alternateRenditionInfo entries. */
            BLST_Q_INSERT_TAIL(&hlsSession->altRenditionInfoQueueHead, altRenditionInfo, next);
            BDBG_MSG(("%s: altRenditionInfo=%p uri=%s entry is inserted into the list", BSTD_FUNCTION, (void *)altRenditionInfo, altRenditionInfo->absoluteUri));
            altRenditionInfo = NULL; /* reset pointer as this playlist is successfully inserted into the hlsSession */
        }
        else if ( (tag = B_PlaybackIp_UtilsStristr(nextLine, HLS_M3U8_I_FRAME_STREAM_INFO_TAG)) != NULL) {
            char *tmp, *attribute;
            /* variant stream info tag: allocate & initialize another playlistInfo entry */
            if ((playlistFileInfo = (PlaylistFileInfo *)BKNI_Malloc(sizeof(PlaylistFileInfo))) == NULL) {
                BDBG_ERR(("%s: Failed to allocate %d bytes of memory for playlistInfo file structure", BSTD_FUNCTION, (int)sizeof(PlaylistFileInfo)));
                return NULL;
            }
            memset(playlistFileInfo, 0, sizeof(PlaylistFileInfo));
            playlistFileInfo->version = PLAYLIST_FILE_COMPATIBILITY_VERSION;
            /* parse the attributes of this tag, its format is:
                EXT-X-I-FRAME-STREAM-INF:[attribute=value][,attribute=value]*
                E.g.
                #EXT-X-I-FRAME-STREAM-INF:BANDWIDTH=297056,CODECS="avc1.4d401f",URI="gear3/iframe_index.m3u8"
             */
            for (
                    (tmp = attribute = tag + strlen(HLS_M3U8_I_FRAME_STREAM_INFO_TAG));    /* points to 1st attribute */
                    (tmp && ((tmp = strstr(attribute, "=")) != NULL));
                    (attribute = tmp+1)
                )
            {
                char *value;
                *tmp = '\0'; /* null terminate attribute to convert it into a string */
                value = tmp+1;
                /* multiple attributes are separate by , */
                if ((tmp = strstr(value, ",")) != NULL) {
                    /* another attribute is present following this attribute=value pair */
                    *tmp = '\0';    /* null terminate value to convert it into a string */
                }
                if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_PROGRAM_ID) != NULL) {
                    playlistFileInfo->programId = strtol(value, NULL, 10);
                    if (programId == -1)
                        programId = playlistFileInfo->programId;
                    else {
                        if (programId != playlistFileInfo->programId) {
                            BDBG_ERR(("%s: no support for multiple programs yet: Program ID current %d, new %d", BSTD_FUNCTION, programId, playlistFileInfo->programId));
                            /* TODO: goto free up current entry and move to the next program entry */
                            goto error;
                        }
                    }
                    BDBG_MSG(("%s: Program ID %d", BSTD_FUNCTION, playlistFileInfo->programId));
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_BANDWIDTH) != NULL) {
                    playlistFileInfo->bandwidth = strtol(value, NULL, 10);
                    if (playlistFileInfo->bandwidth == 0) {
                        BDBG_ERR(("%s: invalid bandwidth (%d) in the variant playlist, ignoring it", BSTD_FUNCTION, playlistFileInfo->bandwidth));
                    }
                    BDBG_MSG(("%s: Bandwidth %d", BSTD_FUNCTION, playlistFileInfo->bandwidth));
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_CODECS) != NULL) {
                    if (tmp)
                        /* codec attribute can have 1 or more values, each value is separated by a comma, replace this comma is present w/ \0 , and then look for ending " */
                        *tmp = ',';
                    BDBG_MSG(("%s: TODO: need to parse attribute %s=%s", BSTD_FUNCTION, HLS_M3U8_TAG_ATTRIBUTE_CODECS, value));
                    /* The syntax for CODECS attribute is: CODECS="[format][,format]" */
                    /* so we need to skip the initial " char & then jump to the next " char */
                    value += 1; /* skips starting " char */
                    if ((tmp = strstr(value, "\"")) != NULL) {
                        *tmp = '\0';
                        playlistFileInfo->segmentCodecType = parseCodecString(playback_ip, value);
                        BDBG_MSG(("Codec string: %s, segmentCodecType %d", value, playlistFileInfo->segmentCodecType));
                    }
                    else {
                        /* missing ending " char, so it an error as CODECS attribute is enclosed in the " " */
                        BDBG_ERR(("%s: missing ending \" char, so it an error as CODECS attribute is enclosed in the \" \", attribute: %s", BSTD_FUNCTION, value));
                        goto error;
                    }
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_RESOLUTION) != NULL) {
                    BDBG_MSG(("%s: TODO: need to parse attribute %s=%s", BSTD_FUNCTION, HLS_M3U8_TAG_ATTRIBUTE_RESOLUTION, value));
                    /* TODO: Add parsing of VIDEO, & SUB-TITLES attributes. */
                }
                else if (B_PlaybackIp_UtilsStristr(attribute, HLS_M3U8_TAG_ATTRIBUTE_URI) != NULL) {
                    BDBG_MSG(("%s: URI attribute %s=%s", BSTD_FUNCTION, HLS_M3U8_TAG_ATTRIBUTE_URI, value));
                    if (http_absolute_uri(value)) {
                        /* playlist contains the absolute uri, copy it */
                        BDBG_MSG(("%s: absolute URI: %s", BSTD_FUNCTION, value));
                        if ((playlistFileInfo->absoluteUri = B_PlaybackIp_UtilsStrdup(value)) == NULL) {
                            BDBG_ERR(("%s: Failed to allocate %zu bytes of memory for absolute URI ", BSTD_FUNCTION, strlen(value)));
                            goto error;
                        }
                    }
                    else {
                        /* relative url, build complete uri using server ip address & port # */
                        /* uri is encapsulated in "", skip them */
                        char *tmp1;
                        value += 1; /* skip the 1st " char */
                        if ((tmp1 = strstr(value, "\"")) != NULL) {
                            *tmp1 = '\0'; /* leave out the last " char */
                        }
                        BDBG_MSG(("%s: %s is not absolute URI", BSTD_FUNCTION, value));
                        if ((playlistFileInfo->absoluteUri = B_PlaybackIp_HlsBuildAbsoluteUri(playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port, playback_ip->openSettings.socketOpenSettings.url, value)) == NULL) {
                            BDBG_ERR(("Failed to build URI at %s:%d", BSTD_FUNCTION, __LINE__));
                            goto error;
                        }
                    }
                    if (hls_parse_url(&playlistFileInfo->protocol, &playlistFileInfo->server, &playlistFileInfo->port, &playlistFileInfo->uri, playlistFileInfo->absoluteUri) == false) {
                        BDBG_ERR(("Failed to parse URI at %s:%d", BSTD_FUNCTION, __LINE__));
                        goto error;
                    }
                    BDBG_MSG(("%s: I-Frame URI: server %s, port %d, protocol %d, uri %s, b/w %d, program id %d", BSTD_FUNCTION, playlistFileInfo->server, playlistFileInfo->port, playlistFileInfo->protocol, playlistFileInfo->uri, playlistFileInfo->bandwidth, playlistFileInfo->programId));
                }
                else {
                    BDBG_ERR(("%s: unsupported/unknown attribute %s=%s, ignoring it for now!!", BSTD_FUNCTION, attribute, value));
                }
            }

            BKNI_AcquireMutex(hlsSession->lock);
            hlsSession->useIFrameTrickmodes = true; /* set so that InsertPlaylistInfoEntry function inserts it in the iFrame related playlist. */
            B_PlaybackIp_HlsInsertPlaylistInfoEntry(hlsSession, playlistFileInfo);
            hlsSession->useIFrameTrickmodes = false;
            BKNI_ReleaseMutex(hlsSession->lock);
            playlistFileInfo = NULL; /* reset pointer as this playlist is successfully inserted into the hlsSession */
            BDBG_MSG(("%s: iFrame playlistFileInfo=%p entry is inserted into the list", BSTD_FUNCTION, (void *)playlistFileInfo));
            playlistFileInfo = NULL; /* reset pointer as this playlist is successfully inserted into the hlsSession */

        }
        else {
            BDBG_MSG(("%s: Comment line %s, ignore it", BSTD_FUNCTION, tag));
        }
    }
    if (nextLine)
        BKNI_Free(nextLine);
    BDBG_MSG(("%s: finished parsing variant playlist file", BSTD_FUNCTION));
    *parsingSuccess = true;
#if 0
    printVariantIFramePlaylistFile(hlsSession);
#endif
    return NULL;

error:
    if (nextLine)
        BKNI_Free(nextLine);
    B_PlaybackIp_FreePlaylistInfoAll(playback_ip);
    if (playlistFileInfo)
        B_PlaybackIp_FreePlaylistInfo(playlistFileInfo);
    if (altRenditionInfo) {
        B_PlaybackIp_FreeAltRenditionInfo(altRenditionInfo);
        BKNI_Free(altRenditionInfo);
    }
    *parsingSuccess = false;
    return NULL;
}

/* select a playlist, download & parse it, playlist is selected matching closely with current n/w b/w */
PlaylistFileInfo *
B_PlaybackIp_HlsSelectDownloadAndParsePlaylist(
    B_PlaybackIpHandle playback_ip,
    HlsSessionState *hlsSession
    )
{
    PlaylistFileInfo *playlistFileInfo = NULL;
    BSTD_UNUSED(playback_ip);

    /* NOTE: we are no longer downloading all playlists in one go as it has two disadvantages: */
    /* 1) increases latency when n/w b/w is slow to a server, */
    /* 2) we anyway need to redownload the playlist when we later switch */
    /* only advantage is for bounded playlists where we dont need to redownload the playlist, but that can be ignored for the initial latency improval */

    /* we start with the first playlist containing both audio & video and thus skip the one with just Audio only */
    for (playlistFileInfo = BLST_Q_FIRST(&hlsSession->playlistFileInfoQueueHead);
         playlistFileInfo;
         playlistFileInfo = BLST_Q_NEXT(playlistFileInfo, next))
    {
        if (playlistFileInfo->segmentCodecType != HlsSegmentCodecType_eAudioOnly) {
            break;
        }
    }
    if (playlistFileInfo == NULL) playlistFileInfo = BLST_Q_FIRST(&hlsSession->playlistFileInfoQueueHead);
    hlsSession->currentPlaylistBandwidth = playlistFileInfo->bandwidth;
    /* download and parse the playlist */
    if (B_PlaybackIp_HlsConnectDownloadAndParsePlaylistFile(playback_ip, hlsSession, playlistFileInfo) < 0) {
        BDBG_ERR(("%s: Failed to download & parse playlist file entry w/ uri %s", BSTD_FUNCTION, playlistFileInfo->uri));
#if 0
        /* If we can't download a playlist, we should take an option from app whether we should remove this playlist from future access and attempt to get the next playlist. */
        B_PlaybackIp_HlsRemovePlaylistInfoEntry(hlsSession, playlistFileInfo);
        B_PlaybackIp_FreePlaylistInfo(playlistFileInfo);

        for (playlistFileInfo = BLST_Q_FIRST(&hlsSession->playlistFileInfoQueueHead);
            playlistFileInfo;
            playlistFileInfo = BLST_Q_FIRST(&hlsSession->playlistFileInfoQueueHead))
        {
            if ( playlistFileInfo )
            {
                if (B_PlaybackIp_HlsConnectDownloadAndParsePlaylistFile(playback_ip, hlsSession, playlistFileInfo) == 0)
                {
                    goto success;
                }

                // remove play list file.
                B_PlaybackIp_HlsRemovePlaylistInfoEntry(hlsSession, playlistFileInfo);
                B_PlaybackIp_FreePlaylistInfo(playlistFileInfo);
            }
        }
#endif
        goto error;
    }

#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: using playlist file entry %p, uri %s, b/w %d", BSTD_FUNCTION, (void *)playlistFileInfo, playlistFileInfo->absoluteUri, playlistFileInfo->bandwidth));
#endif
    hlsSession->currentPlaylistFile = playlistFileInfo;
    return playlistFileInfo;
error:
    return NULL;
}

void
B_PlaybackIp_HlsSegmentBufferDestroy(
    B_PlaybackIpHandle playback_ip
    )
{
    int i;
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;

    for (i=0; i<HLS_NUM_SEGMENT_BUFFERS && hlsSession->segmentBuffer[i].bufferOrig; i++) {
        if (hlsSession->segmentBuffer[i].lock) {
            BKNI_DestroyMutex(hlsSession->segmentBuffer[i].lock);
            hlsSession->segmentBuffer[i].lock = NULL;
        }
        if (hlsSession->segmentBuffer[i].bufferOrig) {
            BKNI_Free(hlsSession->segmentBuffer[i].bufferOrig);
            hlsSession->segmentBuffer[i].bufferOrig = NULL;
        }
    }
}

#define NUM_BTPS_TO_INSERT  3   /* For DQT based rewind, we will need to surround each segment with 2 BTPs in the front and one at the end. */
#define BTP_BUFFER_SIZE (TS_PKT_SIZE)
#define BTP_BUFFERS_SIZE (BTP_BUFFER_SIZE * NUM_BTPS_TO_INSERT)
int
B_PlaybackIp_HlsSegmentBufferCreate(
    B_PlaybackIpHandle playback_ip
    )
{
    int i;
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;
    size_t hlsSegmentBufferSize = HLS_NUM_SEGMENT_BUFFER_SIZE;

    BDBG_MSG(("%s: using index & data cache for downloading the consecutive media segments", BSTD_FUNCTION));
    if (playback_ip->startSettings.audioOnlyPlayback) {
        hlsSegmentBufferSize = HLS_NUM_SEGMENT_BUFFER_SIZE/8;
        BDBG_MSG(("%s: Lowered the HLS Segment Buffer Size from %u to %zu", BSTD_FUNCTION, HLS_NUM_SEGMENT_BUFFER_SIZE, hlsSegmentBufferSize));
    }
    for (i=0; i<HLS_NUM_SEGMENT_BUFFERS; i++) {
        /* coverity[missing_lock] */
        hlsSession->segmentBuffer[i].filled = false;
        hlsSession->segmentBuffer[i].bufferSize = hlsSegmentBufferSize;
        hlsSession->segmentBuffer[i].bufferDepth = 0;
        if (BKNI_CreateMutex(&hlsSession->segmentBuffer[i].lock) != 0) {
            BDBG_ERR(("%s: Failed to create BKNI mutex at %d", BSTD_FUNCTION, __LINE__));
            goto error;
        }
        if ((hlsSession->segmentBuffer[i].bufferOrig = BKNI_Malloc(hlsSegmentBufferSize + BTP_BUFFERS_SIZE + BMEDIA_PES_HEADER_MAX_SIZE)) == NULL) {
            BDBG_ERR(("%s: Failed to allocate HLS Segment Download buffer[%d] of %zu size", BSTD_FUNCTION, i, hlsSegmentBufferSize));
            goto error;
        }
        /* Note: we keep the BTP_BUFFERS_SIZE hidden from the buffer usage as this space is used to insert BTPs and thus we dont want it be used for AV data. */
        hlsSession->segmentBuffer[i].buffer = hlsSession->segmentBuffer[i].bufferOrig;
        BDBG_MSG(("%s: Allocate HLS Segment Download buffer[%d] %p of %zu size", BSTD_FUNCTION, i, (void *)hlsSession->segmentBuffer[i].bufferOrig, hlsSegmentBufferSize));
    }

    return 0;
error:
    B_PlaybackIp_HlsSegmentBufferDestroy(playback_ip);
    return -1;
}

void
B_PlaybackIp_HlsSessionDestroy(B_PlaybackIpHandle playback_ip)
{
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;
    B_PlaybackIp_FreePlaylistInfoAll(playback_ip);

    BDBG_WRN(("hlsSession = %p", (void *)hlsSession));
    if (!hlsSession)
        return;

    if (hlsSession->bufferFilledEvent) {
        BKNI_DestroyEvent(hlsSession->bufferFilledEvent);
        hlsSession->bufferFilledEvent = NULL;
    }
    if (hlsSession->bufferEmptiedEvent) {
        BKNI_DestroyEvent(hlsSession->bufferEmptiedEvent);
        hlsSession->bufferEmptiedEvent = NULL;
    }
    if (hlsSession->reDownloadPlaylistEvent) {
        BKNI_DestroyEvent(hlsSession->reDownloadPlaylistEvent);
        hlsSession->reDownloadPlaylistEvent = NULL;
    }
    if (hlsSession->segDownloadThreadPausedEvent) {
        BKNI_DestroyEvent(hlsSession->segDownloadThreadPausedEvent);
        hlsSession->segDownloadThreadPausedEvent = NULL;
    }
    if (hlsSession->playbackThreadPausedEvent) {
        BKNI_DestroyEvent(hlsSession->playbackThreadPausedEvent);
        hlsSession->playbackThreadPausedEvent = NULL;
    }
    if (hlsSession->playlistReDownloadThreadPausedEvent) {
        BKNI_DestroyEvent(hlsSession->playlistReDownloadThreadPausedEvent);
        hlsSession->playlistReDownloadThreadPausedEvent = NULL;
    }
    if (hlsSession->segDownloadThreadPauseDoneEvent) {
        BKNI_DestroyEvent(hlsSession->segDownloadThreadPauseDoneEvent);
        hlsSession->segDownloadThreadPauseDoneEvent = NULL;
    }
    if (hlsSession->playbackThreadPauseDoneEvent) {
        BKNI_DestroyEvent(hlsSession->playbackThreadPauseDoneEvent);
        hlsSession->playbackThreadPauseDoneEvent = NULL;
    }
    if (hlsSession->playlistReDownloadThreadPauseDoneEvent) {
        BKNI_DestroyEvent(hlsSession->playlistReDownloadThreadPauseDoneEvent);
        hlsSession->playlistReDownloadThreadPauseDoneEvent = NULL;
    }
    if (hlsSession->lock) {
        BKNI_DestroyMutex(hlsSession->lock);
        hlsSession->lock = NULL;
    }
    B_PlaybackIp_HlsSegmentBufferDestroy(playback_ip);

    if (hlsSession->playlistSocketFd) {
        close(hlsSession->playlistSocketFd);
        hlsSession->playlistSocketFd = 0;
    }

    if (hlsSession->playlistBuffer) {
        BKNI_Free(hlsSession->playlistBuffer);
        hlsSession->playlistBuffer = NULL;
    }

    if (hlsSession->lastSegmentUrl) {
        BKNI_Free(hlsSession->lastSegmentUrl);
        hlsSession->lastSegmentUrl = NULL;
    }
    BKNI_Free(hlsSession);
    playback_ip->hlsSessionState = NULL;
    BDBG_MSG(("%s: Done", BSTD_FUNCTION));
}

bool
B_PlaybackIp_HlsSetupNextMediaSegment(B_PlaybackIpHandle playback_ip)
{
    HlsSessionState *hlsSession;
    MediaFileSegmentInfo *mediaFileSegmentInfo;
    bool status = false;

    if (playback_ip->securityHandle) {
        playback_ip->netIo.close(playback_ip->securityHandle);
        playback_ip->securityHandle = NULL;
    }
    close(playback_ip->socketState.fd);
    playback_ip->socketState.fd = -1;

    hlsSession = playback_ip->hlsSessionState;
    mediaFileSegmentInfo = BLST_Q_NEXT(hlsSession->currentPlaylistFile->currentMediaFileSegment, next);
    BDBG_MSG(("%s: choosing next media segment: sequence # %d", BSTD_FUNCTION, mediaFileSegmentInfo->mediaSequence));
    /* for the chosen playlist, take the 1st media segment URL and connect to that server so that we can probe its media details */
    if (mediaFileSegmentInfo) {
        if (B_PlaybackIp_HlsSetupHttpSessionToServer(playback_ip, mediaFileSegmentInfo, 0 /*persistentHttpSession*/, 1 /*reOpenSocketConnection */, &playback_ip->socketState.fd, playback_ip->temp_buf, &playback_ip->initial_data_len) == false) {
            BDBG_ERR(("%s: ERROR: Socket setup or HTTP request/response failed for downloading Media Segment", BSTD_FUNCTION));
            return false;
        }
        hlsSession->currentPlaylistFile->currentMediaFileSegment = mediaFileSegmentInfo;
        /* successfully connected & got the GET request/response from the server, so move on */
        status = true;
    }
    else {
        BDBG_ERR(("%s: ERROR: Failed to download any media segment of playlist file (uri %s) for media probe", BSTD_FUNCTION, hlsSession->currentPlaylistFile->absoluteUri));
        status = false;
    }
    return status;
}

static bool
doMediaProbeOnAltRenditions(B_PlaybackIpHandle playback_ip)
{
    PlaylistFileInfo *playlistFileInfo = NULL;
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;
    AltRenditionInfo *altRenditionInfo = NULL;

    playback_ip->psi.hlsAltAudioRenditionCount = 0;
    /*
     * Go thru the list of alternate renditions and probe media on the associated segments.
     */
    for (altRenditionInfo = BLST_Q_FIRST(&hlsSession->altRenditionInfoQueueHead);
         altRenditionInfo;
         altRenditionInfo = BLST_Q_NEXT(altRenditionInfo, next)
        )
    {
        if (altRenditionInfo->type == HlsAltRenditionType_eAudio) {
            if (altRenditionInfo->language == NULL) { BDBG_WRN(("%s:%p: skipping audio alternate rendition entry as its language is NULL", BSTD_FUNCTION, (void *)playback_ip)); continue;}
            if (altRenditionInfo->absoluteUri == NULL) {
                if (altRenditionInfo->defaultRendition == true) {
                    /* Since this AUDIO rendition doesn't contain URI attribute & DEFAULT attribute is set, */
                    /* set a flag to indicate to app that it can use the audio from the main variant unless it has a different language expectation. */
                    playback_ip->psi.defaultAudioIsMuxedWithVideo = true;
                    playback_ip->psi.mainAudioLanguage = altRenditionInfo->language;
                    BDBG_MSG(("%s: AUDIO altRenditionInfo=%p doesn't contain URI attribute & this audio is part of main variant!", BSTD_FUNCTION, (void *)altRenditionInfo));
                    continue; /* we are done w/ this rendition as this is part of the main variant. */
                }
                else {
                    BDBG_ERR(("%s: playback_ip=%p: Invalid Media Tag in the variant playlist: AUDIO type entry doesn't have URI attribute set & is not the default entry!", BSTD_FUNCTION, (void *)playback_ip));
                    goto error;
                }
            }
            else { /* URI attribute is set. */
                if (altRenditionInfo->defaultRendition == true) {
                    playback_ip->psi.hlsAltAudioRenditionInfo[playback_ip->psi.hlsAltAudioRenditionCount].defaultAudio = true;
                }
                else {
                    playback_ip->psi.hlsAltAudioRenditionInfo[playback_ip->psi.hlsAltAudioRenditionCount].defaultAudio = false;
                }
                playback_ip->psi.hlsAltAudioRenditionInfo[playback_ip->psi.hlsAltAudioRenditionCount].requiresPlaypump2 = true;
                playback_ip->psi.hlsAltAudioRenditionInfo[playback_ip->psi.hlsAltAudioRenditionCount].language = altRenditionInfo->language;
                /* Since URI attribute is set, so do probe on it and set its audio codec, language, & pid values (we do this below). */
                BDBG_WRN(("%s: Do MediaProbe on AUDIO altRenditionInfo=%p URI=%s", BSTD_FUNCTION, (void *)altRenditionInfo, altRenditionInfo->absoluteUri));
            }
        }
        else {
            /* Other types such as sub-titles are not yet supported. */
            BDBG_WRN(("%s: playback_ip=%p: Type %s is not yet supported!", BSTD_FUNCTION, (void *)playback_ip,
                        altRenditionInfo->type==HlsAltRenditionType_eVideo?"Video":
                        altRenditionInfo->type==HlsAltRenditionType_eSubtitles?"Subtitles":
                        "Closed-Captions"
                     ));
            continue; /* skip them for now..*/
        }
        /* We have an alternate rendition (audio for now) URL for which we will like to download the playlist so that to get the 1st segment URL. */
        /* Then, we will download its 1st segment so that we do find its media information. */

        /*
         * To get the PSI related info for this alternate rendition, we have to do following:
         *  create a playlist object for this rendition.
         *  download the alternate rendition playlist into it & parse it (to get list of media segment URIs).
         *  download 1st media segment.
         *  do media probe on that segment and determine the PSI info.
         *  freeup the resources.
         */

        /* allocate/initialize playlist file structure so that we download & parse this alternate rendition playlist. */
        if ( (playlistFileInfo = (PlaylistFileInfo *)BKNI_Malloc(sizeof(PlaylistFileInfo))) == NULL) {
            BDBG_ERR(("%s: Failed to allocate %d bytes of memory for playlistInfo file structure", BSTD_FUNCTION, (int)sizeof(PlaylistFileInfo)));
            goto error;
        }
        memset(playlistFileInfo, 0, sizeof(PlaylistFileInfo));

        /* build the uri of the playlist file */
        if ((playlistFileInfo->absoluteUri = B_PlaybackIp_UtilsStrdup(altRenditionInfo->absoluteUri)) == NULL) {
            BDBG_ERR(("Failed to duplicate the alternateRendition URI at %s:%d", BSTD_FUNCTION, __LINE__));
            goto error;
        }

        /* parse the URI into individual fields. */
        if ((hls_parse_url(&playlistFileInfo->protocol, &playlistFileInfo->server, &playlistFileInfo->port, &playlistFileInfo->uri, playlistFileInfo->absoluteUri) == false) || (playlistFileInfo->protocol != B_PlaybackIpProtocol_eHttp && playlistFileInfo->protocol != B_PlaybackIpProtocol_eHttps)) {
            BDBG_ERR(("Failed to parse URI into sub-fields at %s:%d", BSTD_FUNCTION, __LINE__));
            goto error;
        }
        if (playback_ip->hlsSessionState->currentPlaylistFile)
            playlistFileInfo->version = playback_ip->hlsSessionState->currentPlaylistFile->version;
        else
            playlistFileInfo->version = PLAYLIST_FILE_COMPATIBILITY_VERSION;
        /* rest of fields of playlistInfo can be 0 */

        if (B_PlaybackIp_HlsMediaProbeCreate(&hlsSession->mediaProbe, playback_ip->indexCache, playback_ip->indexCacheSize) != 0) {
            BDBG_ERR(("%s: Failed to create the Media Probe context for HLS sessions ", BSTD_FUNCTION));
            goto error;
        }

        /* Download the playlist, parse it, get URL of 1st segment, download it and then media probe it. */
        if (B_PlaybackIp_HlsConnectDownloadAndParsePlaylistFileAnd1stSegment(playback_ip, hlsSession, playlistFileInfo, hlsSession->mediaProbe.segmentBuffer) < 0) {
            BDBG_ERR(("%s: Failed to download & parse playlist file entry w/ uri %s", BSTD_FUNCTION, playlistFileInfo->absoluteUri));
            goto error;
        }

        if (B_PlaybackIp_HlsMediaProbeStart(playback_ip, &hlsSession->mediaProbe, hlsSession->mediaProbe.segmentBuffer) != true) {
            BDBG_ERR(("%s: Media Probe failed during the initial probe on alternate rendition.", BSTD_FUNCTION));
            goto error;
        }
        /* Now copy the codec & pid info for this rendition. */
        if (altRenditionInfo->type == HlsAltRenditionType_eAudio) {
            playback_ip->psi.hlsAltAudioRenditionInfo[playback_ip->psi.hlsAltAudioRenditionCount].pid = hlsSession->mediaProbe.psi.audioPid;
            playback_ip->psi.hlsAltAudioRenditionInfo[playback_ip->psi.hlsAltAudioRenditionCount].codec = hlsSession->mediaProbe.psi.audioCodec;
            playback_ip->psi.hlsAltAudioRenditionInfo[playback_ip->psi.hlsAltAudioRenditionCount].groupId = altRenditionInfo->groupId;
            playback_ip->psi.hlsAltAudioRenditionInfo[playback_ip->psi.hlsAltAudioRenditionCount].languageName = altRenditionInfo->name;
            playback_ip->psi.hlsAltAudioRenditionInfo[playback_ip->psi.hlsAltAudioRenditionCount].containerType = hlsSession->mediaProbe.psi.mpegType;
        }
        playback_ip->psi.hlsAltAudioRenditionCount++;
        B_PlaybackIp_HlsMediaProbeDestroy(&hlsSession->mediaProbe);
        B_PlaybackIp_FreePlaylistInfo(playlistFileInfo);
    }
    BDBG_MSG(("%s: hlsAltAudioRenditionCount=%d ", BSTD_FUNCTION, playback_ip->psi.hlsAltAudioRenditionCount));
    playback_ip->hlsSessionEnabled = true;
    return true;
error:
    B_PlaybackIp_FreePlaylistInfo(playlistFileInfo);
    return false;
}

/* function to setup a HLS Session: */
/*  -downloads variant and simple playlists */
/*  -connects to the server for the 1st media segment and gets playback_ip session for media probing */
int
B_PlaybackIp_HlsSessionSetup(B_PlaybackIpHandle playback_ip, char *http_hdr)
{
    char *pValue;
    MediaFileSegmentInfo *mediaFileSegmentInfo;
    PlaylistFileInfo *playlistFileInfo;
    HlsSessionState *hlsSession;
    bool parsingSuccess = false;
    int initial_data_len = 0;

    if (B_PlaybackIp_IsHlsSession(playback_ip->openSettings.socketOpenSettings.url, http_hdr) != true) {
        BDBG_MSG(("%s: Not a HTTP Live Streaming (HLS) Session", BSTD_FUNCTION));
        return 0;
    }

    /* Now it is a HLS session, do further checks to validate playlist file format */
    BDBG_MSG(("%s: HTTP Live Streaming (HLS) Session: download & parse playlist", BSTD_FUNCTION));

    /* allocate hls session state */
    if ((playback_ip->hlsSessionState = (HlsSessionState *)BKNI_Malloc(sizeof(HlsSessionState))) == NULL) {
        BDBG_ERR(("%s: failed to allocate %d bytes for HLS Session state", BSTD_FUNCTION, (int)sizeof(HlsSessionState)));
        goto error;
    }
    hlsSession = playback_ip->hlsSessionState;
    BKNI_Memset(hlsSession, 0, sizeof(HlsSessionState));

    /* allocate a buffer where playlist file will be completely downloaded */
    if ((hlsSession->playlistBuffer = (char *)BKNI_Malloc(HLS_PLAYLIST_FILE_SIZE)) == NULL) {
        BDBG_ERR(("%s: failed to allocate %d bytes for downloading the playlist file", BSTD_FUNCTION, HLS_PLAYLIST_FILE_SIZE));
        goto error;
    }
    hlsSession->playlistBufferSize = HLS_PLAYLIST_FILE_SIZE;

    /* copy any initial payload data (read part of the initial HTTP response) into the playlist buffer */
    initial_data_len = playback_ip->chunkPayloadLength ? playback_ip->chunkPayloadLength : playback_ip->initial_data_len;
    if (initial_data_len) {
        memcpy(hlsSession->playlistBuffer, playback_ip->temp_buf, initial_data_len);
        hlsSession->playlistBufferDepth = initial_data_len;
        playback_ip->initial_data_len = 0;
        playback_ip->chunkPayloadLength = 0;
        playback_ip->bytesLeftInCurChunk -= initial_data_len;
        playback_ip->chunkSize -= initial_data_len;
    }
    else {
        hlsSession->playlistBufferDepth = 0;
    }

    pValue = getenv("max_download_bitrate");
    if (pValue) {
        hlsSession->maxNetworkBandwidth = strtoul(pValue, NULL, 0);
    }
#define PLAYBACK_IP_MAX_NETWORK_BANDWIDTH 5800000 /* set to 1.8Mpbs */
    if (!pValue || hlsSession->maxNetworkBandwidth == 0) {
        hlsSession->maxNetworkBandwidth = PLAYBACK_IP_MAX_NETWORK_BANDWIDTH;
    }
    BDBG_MSG(("%s: max network bandwidth set to %d", BSTD_FUNCTION, hlsSession->maxNetworkBandwidth));
    B_PlaybackIp_HlsResetBandwidthSample(&hlsSession->bandwidthContext, 10); /* Use 10-sample rolling average */

    /* download the playlist file: we dont know whether it is a simple playlist file or a variant playlist file */
    if (B_PlaybackIp_HlsDownloadFile(playback_ip, &playback_ip->socketState.fd, hlsSession->playlistBuffer, hlsSession->playlistBufferSize, &hlsSession->playlistBufferDepth, true) != true) {
        BDBG_ERR(("%s: failed to download the M3U playlist file", BSTD_FUNCTION));
        goto error;
    }
    BDBG_MSG(("%s: downloaded playlist file of %d bytes", BSTD_FUNCTION, hlsSession->playlistBufferDepth));
    hlsSession->playlistBufferReadIndex = 0;

#ifdef HLS_UNIT_TEST
    dump_playlist_file(hlsSession);
#endif

    /* verify that playlist file is in the required Extended M3U8 format */
    if (B_PlaybackIp_IsPlaylistInExtendedM3uFormat(playback_ip) != true) {
        goto error;
    }

    if (BKNI_CreateMutex(&hlsSession->lock) != 0) {
        BDBG_ERR(("%s: Failed to create BKNI mutex at %d", BSTD_FUNCTION, __LINE__));
        goto error;
    }

    /* check if the playlist is a variant playlist and if so, download the suitable simple playlist file contained in the variant playlist */
    /* suitable playlist will be the one with bitrate matching the current n/w bitrate */
    if ((playlistFileInfo = B_PlaybackIp_ParsePlaylistVariantFile(playback_ip, &parsingSuccess)) == NULL) {
        /* variant playlist case as playlistFileInfo is NULL (i.e. initial playlist is not a simple playlist but a variant one */
        if (parsingSuccess == false) {
            BDBG_ERR(("%s: Failed to parse variant playlist file", BSTD_FUNCTION));
            goto error;
        }
        /* variant playlist is downloaded & parsed, now download a playlist w/ b/w closest to current n/w b/w */
        if ((playlistFileInfo = B_PlaybackIp_HlsSelectDownloadAndParsePlaylist(playback_ip, hlsSession)) == NULL) {
            BDBG_ERR(("%s: Failed to download & parse playlist file entry", BSTD_FUNCTION));
            goto error;
        }
#ifdef HLS_UNIT_TEST
        /* print simple playlist */
        dump_playlist_file(hlsSession);
#endif
    }
    else {
        /* top level uri is not a variant playlist but a simple playlist, since we have already downloaded it, now lets parse it */
        if (B_PlaybackIp_ParsePlaylistFile(playback_ip, playlistFileInfo, false/*parseOnlyOneMediaEntry*/) != true) {
            BDBG_ERR(("%s: failed to parse the M3U playlist file", BSTD_FUNCTION));
            goto error;
        }
        if (playlistFileInfo->bounded == false) {
            playback_ip->psi.duration = playlistFileInfo->totalDuration;
            BDBG_MSG(("%s: updating the duration with the newly updated one from the playlist to %d", BSTD_FUNCTION, playback_ip->psi.duration));
        }

    }
#ifdef HLS_UNIT_TEST
    printVariantPlaylistFile(hlsSession);
#endif
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: finished parsing the M3U playlist file, now setup for media probing", BSTD_FUNCTION));
#endif

    /* at this time,  playlists (both variant & simple ones) are downloaded & parsed, so lets download the 1st segment and find out its media PSI info */

    /* for the chosen playlist, take the 1st media segment URL and connect to that server so that we can probe its media details */
    for (mediaFileSegmentInfo = BLST_Q_FIRST(&playlistFileInfo->mediaFileSegmentInfoQueueHead);
         mediaFileSegmentInfo;
         mediaFileSegmentInfo = BLST_Q_FIRST(&playlistFileInfo->mediaFileSegmentInfoQueueHead))
    {
        if ( breakFromLoop(playback_ip)) {
            BDBG_MSG(("%s: breaking out of 1st HLS Media Segment Setup loop due to state (%d) change", BSTD_FUNCTION, playbackIpState(playback_ip)));
            goto error;
        }
        if (B_PlaybackIp_HlsSetupHttpSessionToServer(playback_ip, mediaFileSegmentInfo, 0 /*persistentHttpSession*/, 1 /*reOpenSocketConnection */, &playback_ip->socketState.fd, playback_ip->temp_buf, &playback_ip->initial_data_len) == false) {
            BDBG_ERR(("%s: ERROR: Socket setup or HTTP request/response failed for downloading Media Segment, retrying next media segment", BSTD_FUNCTION));
            /* remove this media segment as we met some error in downloading this segment (most likely segment URL has expired) */
            BLST_Q_REMOVE_HEAD(&playlistFileInfo->mediaFileSegmentInfoQueueHead, next);
            B_PlaybackIp_FreeMediaFileSegmentInfo(mediaFileSegmentInfo);
            continue;
        }
        playlistFileInfo->currentMediaFileSegment = mediaFileSegmentInfo;
        hlsSession->currentPlaylistFile = playlistFileInfo;
        /* successfully connected & got the GET request/response from the server, so move on */
        break;
    }

    if (mediaFileSegmentInfo == NULL) {
        BDBG_ERR(("%s: ERROR: Failed to download any media segment of playlist file (uri %s) for media probe", BSTD_FUNCTION, playlistFileInfo->absoluteUri));
        goto error;
    }
    if ( breakFromLoop(playback_ip)) {
        BDBG_MSG(("%s: Returning from HlsSessionSetup due to state (%d) change", BSTD_FUNCTION, playbackIpState(playback_ip)));
        goto error;
    }

    /* reset the content length, otherwise media probe takes way too long */
    /* TODO: need to see if this is needed or psiParsingTimeLimit will do the trick */
    playback_ip->contentLength = 0;

#if 1
#define HLS_INTERNAL_PROBE
#endif
#ifdef HLS_INTERNAL_PROBE
    {
        unsigned networkBandwidth;
        bool serverClosed;
        HlsSegmentBuffer *segmentBuffer;

        /*
         * Media probe the media segment of the variant playlist (we selected above).
         * This will be constitue the PSI info for the main Video & Audio.
         */
        if (B_PlaybackIp_HlsMediaProbeCreate(&hlsSession->mediaProbe, playback_ip->indexCache, playback_ip->indexCacheSize) != 0) {
            BDBG_ERR(("%s: Failed to create the Media Probe context for HLS sessions ", BSTD_FUNCTION));
            goto error;
        }
        segmentBuffer = hlsSession->mediaProbe.segmentBuffer;

        if (playback_ip->initial_data_len)
        {
            memcpy(segmentBuffer->buffer, playback_ip->temp_buf, playback_ip->initial_data_len);
            segmentBuffer->bufferDepth = playback_ip->initial_data_len;
            BDBG_MSG(("%s: copied %u bytes of initial buffer", BSTD_FUNCTION, playback_ip->initial_data_len));
        }
        /* Download part of 1st segment in the segment buffer. */
        if (B_PlaybackIp_HlsDownloadMediaSegment(playback_ip, playback_ip->socketState.fd,
                    segmentBuffer->buffer,
                    segmentBuffer->bufferSize,
                    segmentBuffer->bufferSize-playback_ip->initial_data_len,
                    &segmentBuffer->bufferDepth, &networkBandwidth, &serverClosed) != true) {
            BDBG_ERR(("%s: failed to download the current media segment", BSTD_FUNCTION));
            if (playback_ip->securityHandle) {
                playback_ip->netIo.close(playback_ip->securityHandle);
                playback_ip->securityHandle = NULL;
            }
            close(playback_ip->socketState.fd);
            playback_ip->socketState.fd = -1;
        }
        if (0) {
            /* save the media segment. */
            FILE *fp;
            fp = fopen("./segVideo.ts", "ab+");
            fwrite(segmentBuffer->buffer, 1, segmentBuffer->bufferDepth, fp);
            fflush(fp);
            fclose(fp);
        }
        /* probe the media. */
        if (B_PlaybackIp_HlsMediaProbeStart(playback_ip, &hlsSession->mediaProbe, segmentBuffer) != true) {
            BDBG_ERR(("%s: Media Probe failed during the initial probe", BSTD_FUNCTION));
            goto error;
        }
        playback_ip->psi = hlsSession->mediaProbe.psi;
        B_PlaybackIp_HlsMediaProbeDestroy(&hlsSession->mediaProbe);

        /*
         * Finished media probe on a variant playlist, now do so for all alternate renditions.
         * This way app has PSI information about all available alternate renditions and can thus pick the appropriate one.
         */
        if ( doMediaProbeOnAltRenditions(playback_ip) == false ) {
            BDBG_ERR(("%s: playback_ip=%p: MediaProbe on Alternate Renditions Failed", BSTD_FUNCTION, (void *)playback_ip));
            goto error;
        }
    }
#endif
    if (BKNI_CreateEvent(&hlsSession->bufferEmptiedEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
        goto error;
    }

    if (BKNI_CreateEvent(&hlsSession->bufferFilledEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
        goto error;
    }

    if (BKNI_CreateEvent(&hlsSession->reDownloadPlaylistEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
        goto error;
    }

    if (BKNI_CreateEvent(&hlsSession->segDownloadThreadPausedEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
        goto error;
    }

    if (BKNI_CreateEvent(&hlsSession->playbackThreadPausedEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
        goto error;
    }

    if (BKNI_CreateEvent(&hlsSession->playlistReDownloadThreadPausedEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
        goto error;
    }

    if (BKNI_CreateEvent(&hlsSession->segDownloadThreadPauseDoneEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
        goto error;
    }

    if (BKNI_CreateEvent(&hlsSession->playbackThreadPauseDoneEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
        goto error;
    }

    if (BKNI_CreateEvent(&hlsSession->playlistReDownloadThreadPauseDoneEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
        goto error;
    }

    playback_ip->hlsSessionEnabled = true;
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s:%p SessionSetup including Media Probing is complete!", BSTD_FUNCTION, (void *)playback_ip));
#endif

    /* the actual media probe operation happens in the caller function of http module */
    return 0;

error:
    B_PlaybackIp_HlsMediaProbeDestroy(&playback_ip->hlsSessionState->mediaProbe);
    B_PlaybackIp_HlsSessionDestroy(playback_ip);
    return -1;
}

PlaylistFileInfo *
B_PlaybackIp_HlsAllocateDownloadAndParsePlaylistFile(
    B_PlaybackIpHandle playback_ip,
    HlsSessionState *hlsSession,
    PlaylistFileInfo *currentPlaylistFileInfo
    )
{
    PlaylistFileInfo *newPlaylistFileInfo = NULL;
    B_PlaybackIpProtocol protocol;

    if (currentPlaylistFileInfo == NULL) {
        BDBG_ERR(("%s: currentPlaylistInfo is NULL, SW bug!", BSTD_FUNCTION));
        goto error;
    }

    BDBG_MSG(("%s: playlist %p, uri %p", BSTD_FUNCTION, (void *)currentPlaylistFileInfo, (void *)currentPlaylistFileInfo->uri));
    /* allocate/initialize the new playlist file structure */
    if ((newPlaylistFileInfo = (PlaylistFileInfo *)BKNI_Malloc(sizeof(PlaylistFileInfo))) == NULL) {
        BDBG_ERR(("%s: Failed to allocate %d bytes of memory for playlistInfo file structure", BSTD_FUNCTION, (int)sizeof(PlaylistFileInfo)));
        goto error;
    }
    memset(newPlaylistFileInfo, 0, sizeof(PlaylistFileInfo));
    if ((newPlaylistFileInfo->absoluteUri = B_PlaybackIp_UtilsStrdup(currentPlaylistFileInfo->absoluteUri)) == NULL) {
        BDBG_ERR(("%s: Failed to allocate %zu bytes of memory for absolute URI ", BSTD_FUNCTION, strlen(currentPlaylistFileInfo->absoluteUri)));
        goto error;
    }
    newPlaylistFileInfo->protocol = currentPlaylistFileInfo->protocol;
    if ((hls_parse_url(&protocol, &newPlaylistFileInfo->server, &newPlaylistFileInfo->port, &newPlaylistFileInfo->uri, newPlaylistFileInfo->absoluteUri) == false) || (protocol != B_PlaybackIpProtocol_eHttp && protocol != B_PlaybackIpProtocol_eHttps) ) {
        BDBG_ERR(("Failed to parse URI at %s:%d", BSTD_FUNCTION, __LINE__));
        goto error;
    }
    newPlaylistFileInfo->bandwidth = currentPlaylistFileInfo->bandwidth;
    newPlaylistFileInfo->programId = currentPlaylistFileInfo->programId;
    newPlaylistFileInfo->segmentCodecType = currentPlaylistFileInfo->segmentCodecType;

    /* reload the playlist */
    if (B_PlaybackIp_HlsConnectDownloadAndParsePlaylistFile(playback_ip, hlsSession, newPlaylistFileInfo) < 0) {
        BDBG_ERR(("%s: Failed to download & parse playlist file entry w/ uri %s", BSTD_FUNCTION, newPlaylistFileInfo->uri));
        goto error;
    }
#ifdef HLS_UNIT_TEST
    /* verify the playlist file */
    printPlaylistFile(newPlaylistFileInfo);
#endif
    BDBG_MSG(("%s: done playlist %p, uri %s", BSTD_FUNCTION, (void *)newPlaylistFileInfo, newPlaylistFileInfo->uri));
    return newPlaylistFileInfo;

error:
    if (newPlaylistFileInfo)
        B_PlaybackIp_FreePlaylistInfo(newPlaylistFileInfo);
    return NULL;
}

void
B_PlaybackIp_HlsReplacePlaylists(
    HlsSessionState *hlsSession,
    PlaylistFileInfo *playlistFileInfo,
    PlaylistFileInfo *newPlaylistFileInfo
    )
{
    B_PlaybackIp_HlsRemovePlaylistInfoEntry(hlsSession, playlistFileInfo);
    B_PlaybackIp_HlsInsertPlaylistInfoEntry(hlsSession, newPlaylistFileInfo);
    B_PlaybackIp_FreePlaylistInfo(playlistFileInfo);
#ifdef HLS_UNIT_TEST
    printVariantPlaylistFile(hlsSession);
#endif
}

#if 0
#define HLS_BITRATE_SWITCH_TEST
#endif
#ifdef HLS_BITRATE_SWITCH_TEST
static B_Time beginTime, curTime;
#endif

MediaFileSegmentInfo *
B_PlaybackIp_HlsGetNextMediaSegmentEntry(
    B_PlaybackIpHandle playback_ip,
    HlsSessionState *hlsSession,
    int currentMediaSegmentSequenceNumber,
    NEXUS_PlaybackPosition downloadedSegmentsDuration,
    unsigned networkBandwidth,
    bool *pUseDifferentPlaylist
    )
{
    PlaylistFileInfo *playlistFileInfo = NULL;
    PlaylistFileInfo *prevPlaylistFileInfo = NULL;
    MediaFileSegmentInfo *mediaFileSegmentInfo = NULL;

    *pUseDifferentPlaylist = false;
    if (hlsSession->resetPlaylist) {
        /* resetting a playlist, start from the 1st media segment */
        hlsSession->downloadedAllSegmentsInCurrentPlaylist = false;
        hlsSession->resetPlaylist = false;
        mediaFileSegmentInfo = BLST_Q_FIRST(&hlsSession->currentPlaylistFile->mediaFileSegmentInfoQueueHead);
        BDBG_MSG(("%s: Starting w/ 1st media segment: sequence # %d, current network bandwidth %d", BSTD_FUNCTION, mediaFileSegmentInfo->mediaSequence, networkBandwidth));
        hlsSession->currentPlaylistFile->currentMediaFileSegment = mediaFileSegmentInfo;
        return mediaFileSegmentInfo;
    }
    if (hlsSession->downloadedAllSegmentsInCurrentPlaylist && B_PlaybackIp_HlsBoundedStream(playback_ip)) {
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("%s: Done downloading all the segments for a bounded playlist %p, %s...", BSTD_FUNCTION,  (void *)hlsSession->currentPlaylistFile, hlsSession->currentPlaylistFile->uri));
#endif
        return NULL;
    }
    if (hlsSession->downloadedAllSegmentsInCurrentPlaylist && (playback_ip->speedNumerator > 1 || playback_ip->speedNumerator < 0)) {
        /* We have downloaded all segments in a playlist & we are doing fast forward or rewind. */
        /* Since we are not a bounded stream (already checked above), so we are live Event type stream where new segments may get added to the end of the playlist later. */
        /* Given that we are doing trickmodes, we stop checking for the newer segments URIs from the playlist as we want the playback of currently queued */
        /* segments to finish, so that we can give ClientEndOfSegments/ClientBeginOfStream event to the app and app can start to play the stream from the live/starting point. */
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("%s: Fwd case, already downloaded all segments, waiting for app to play: %p, %s...",
                        BSTD_FUNCTION,  (void *)hlsSession->currentPlaylistFile, hlsSession->currentPlaylistFile->uri));
#endif
        return NULL;
    }

    if (1)
    {
        /* unit test specific logic to force switching to audio only segment to simulate adaptive streaming */
        char *pAudioOnly;
        int audioOnly = 0;
        pAudioOnly = getenv("audioOnly");
        if (pAudioOnly)
            audioOnly = atoi(pAudioOnly);
        for (prevPlaylistFileInfo = NULL, playlistFileInfo = BLST_Q_FIRST(&hlsSession->playlistFileInfoQueueHead);
                audioOnly && playlistFileInfo;
                prevPlaylistFileInfo=playlistFileInfo, playlistFileInfo = BLST_Q_NEXT(playlistFileInfo, next))
        {
            if (playlistFileInfo->segmentCodecType == HlsSegmentCodecType_eAudioOnly) {
                BDBG_WRN(("%s: switch to audioOnly segment (audioOnly env variable is set!)", BSTD_FUNCTION));
                goto afterPlaylistSelection;
            }
        }
    }

    /* find a playlist entry whose b/w matches the current n/w b/w */
    for (prevPlaylistFileInfo = playlistFileInfo = BLST_Q_FIRST(hlsSession->useIFrameTrickmodes ? &hlsSession->iFramePlaylistFileInfoQueueHead:&hlsSession->playlistFileInfoQueueHead);
         playlistFileInfo;
         prevPlaylistFileInfo=playlistFileInfo, playlistFileInfo = BLST_Q_NEXT(playlistFileInfo, next))
    {
        BDBG_MSG(("%s: playlist file: uri %s, b/w %d, network Bandwidth %d", BSTD_FUNCTION, playlistFileInfo->uri, playlistFileInfo->bandwidth, networkBandwidth));
        if (playbackIpState(playback_ip) == B_PlaybackIpState_eTrickMode)
        {
            if (hlsSession->useLowestBitRateSegmentOnly)
            {
                if (playlistFileInfo->segmentCodecType != HlsSegmentCodecType_eAudioOnly) {
#ifdef BDBG_DEBUG_BUILD
                    if (playback_ip->ipVerboseLog)
                        BDBG_WRN(("%s: useLowestBitRateSegmentOnly is set, state %d, playlist file: uri %s, b/w %d, network Bandwidth %d", BSTD_FUNCTION,
                                    playbackIpState(playback_ip), playlistFileInfo->uri, playlistFileInfo->bandwidth, networkBandwidth));
#endif
                    break;
                }
            }
            else
            {
                /* we try to find the playlist whoose b/w was closest to the n/w b/w at the desired trickmode rate. */
                if (playlistFileInfo->bandwidth > hlsSession->playlistBandwidthToUseInTrickmode) {
                    playlistFileInfo = prevPlaylistFileInfo;
                    BDBG_ERR(("%s: Trickmode case: using playlist file: uri %s, b/w %d, desired b/w %d, network Bandwidth %d", BSTD_FUNCTION,
                                playlistFileInfo->uri, playlistFileInfo->bandwidth, hlsSession->playlistBandwidthToUseInTrickmode, networkBandwidth));
                    break;
                }
            }
        }
        if (hlsSession->restartPlaylistRampUpByBandwidth)
        {
            hlsSession->restartPlaylistRampUpByBandwidth = false;
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: restartPlaylistRampUpByBandwidth is set, state %d, playlist file: uri %s, b/w %d, network Bandwidth %d", BSTD_FUNCTION,
                            playbackIpState(playback_ip), playlistFileInfo->uri, playlistFileInfo->bandwidth, networkBandwidth));
#endif
            /* we start w/ the 1st playlist. TODO: need to consider the links w/ audio? */
            break;
        }
        if (playlistFileInfo->bandwidth > hlsSession->maxNetworkBandwidth) {
            BDBG_MSG(("%s: using playlist w/ b/w %u > max network b/w %u", BSTD_FUNCTION, playlistFileInfo->bandwidth, hlsSession->maxNetworkBandwidth));
            break;
        }
        if (playlistFileInfo->bandwidth > networkBandwidth ) {
            /* current playlist b/w exceeds the n/w bandwidth */
            /* since the list of playlist file entries is created in the ascending order of their bandwidth value, we will use prev playlist */
            BDBG_MSG(("%s: found playlist entry whose b/w %d exceeds the network b/w %d, use previous playlist entry w/ b/w %d", BSTD_FUNCTION, playlistFileInfo->bandwidth, networkBandwidth, prevPlaylistFileInfo->bandwidth));
            playlistFileInfo = prevPlaylistFileInfo;
            break;
        }
#ifdef RAMPUP_THRU_ALL_VARIANTS
        /* enable this define if we want to linearly go up thru various bitrates instead of directly using highest bitrate allowed by the network. */
        else if (playlistFileInfo->bandwidth > hlsSession->currentPlaylistBandwidth ) {
            /* this playlist's b/w is > than the current ones, so use it */
            BDBG_MSG(("%s: found playlist entry whose b/w %d exceeds the current b/w %u, network b/w %d", BSTD_FUNCTION, playlistFileInfo->bandwidth, hlsSession->currentPlaylistBandwidth, networkBandwidth));
            break;
        }
#endif
        else {
            /* continue to the next playlist */
            continue;
        }
    }

    if (playlistFileInfo == NULL) {
        /* we have reached the end of playlists and all of them have b/w < than the n/w b/w, so just use the last one */
        playlistFileInfo = prevPlaylistFileInfo;
        BDBG_MSG(("%s: using last playlist file entry (%p) with b/w %d, network b/w %d", BSTD_FUNCTION, (void *)playlistFileInfo, playlistFileInfo->bandwidth, networkBandwidth));
    }
afterPlaylistSelection:
    if (playlistFileInfo == NULL) {
        BDBG_ERR(("%s: SW Bug: playlist list seems to be corrupted", BSTD_FUNCTION));
        goto error;
    }

    /* since playlist can change by playlist download thread, it is better to use the current bandwidth to determine if we need to change the playlist */
    if (playlistFileInfo != hlsSession->currentPlaylistFile ||
            hlsSession->currentPlaylistBandwidth != playlistFileInfo->bandwidth)
    {
        PlaylistFileInfo *newPlaylistFileInfo;
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("%s: switch playlist due to n/w b/w change, playlist file entry %p, uri %s, b/w %d, network b/w %d, bounded %d",
                    BSTD_FUNCTION, (void *)playlistFileInfo, playlistFileInfo->uri, playlistFileInfo->bandwidth, networkBandwidth, playlistFileInfo->bounded));
#endif
        /* since all playlists are not downloaded upfront, anytime we change the playlist (whether for live or bounded streams), we have to download this playlist */
        if ((newPlaylistFileInfo = B_PlaybackIp_HlsAllocateDownloadAndParsePlaylistFile(playback_ip, hlsSession, playlistFileInfo)) == NULL) {
            BDBG_ERR(("%s: B_PlaybackIp_HlsAllocateDownloadAndParsePlaylistFile() failed", BSTD_FUNCTION));
            goto error;
        }
        if (newPlaylistFileInfo->mediaSegmentBaseSequence > hlsSession->currentPlaylistFile->mediaSegmentBaseSequence) {
            /* For pure live streams where base seq# change, determine the segmentPositionOffset that should be added when selecting the next segment from the playlist. */
            /* This is bit tricky as we are switching from one playlist to another and are relying on the fact that the two playlists use same starting seq#. */
            /* otherwise, this scheme will not work. */
            unsigned i;
            unsigned segmentsRemovedCount;
            MediaFileSegmentInfo *mediaFileSegmentInfo = NULL;

            /* Check how many segments have been removed from the new playlist based on the starting seq # difference. */
            segmentsRemovedCount = newPlaylistFileInfo->mediaSegmentBaseSequence - hlsSession->currentPlaylistFile->mediaSegmentBaseSequence;

            /* Now go thru the current playlist and determine the elapsedPosition corresponding to the removed segments. */
            for (i=0,mediaFileSegmentInfo = BLST_Q_FIRST(&hlsSession->currentPlaylistFile->mediaFileSegmentInfoQueueHead);
                    mediaFileSegmentInfo && i++ < segmentsRemovedCount;
                    mediaFileSegmentInfo = BLST_Q_NEXT(mediaFileSegmentInfo, next))
            {
                hlsSession->segmentsRemovedDuration += mediaFileSegmentInfo->duration;
            }
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: base seq#: new %d, cur %d, segmentsRemovedCount %u, segmentsRemovedDuration %lu",
                            BSTD_FUNCTION, newPlaylistFileInfo->mediaSegmentBaseSequence, hlsSession->currentPlaylistFile->mediaSegmentBaseSequence,
                            segmentsRemovedCount, hlsSession->segmentsRemovedDuration ));
#endif
        }
        B_PlaybackIp_HlsReplacePlaylists(hlsSession, playlistFileInfo, newPlaylistFileInfo);
        playlistFileInfo = newPlaylistFileInfo;
        hlsSession->currentPlaylistFile = playlistFileInfo;
        hlsSession->currentPlaylistBandwidth = playlistFileInfo->bandwidth;
        *pUseDifferentPlaylist = true;
    }
    if (*pUseDifferentPlaylist == true || hlsSession->currentPlaylistFile->playlistHasChanged == true) {
        NEXUS_PlaybackPosition segmentDuration;

        BDBG_MSG(("%s: find next media segment entry w/ seq # > %d due to %s, speedNumerator %d",
                    BSTD_FUNCTION, currentMediaSegmentSequenceNumber,
                    (hlsSession->currentPlaylistFile->playlistHasChanged? "Playlist update":"n/w b/w change"), playback_ip->speedNumerator));

        if (hlsSession->currentPlaylistFile->playlistHasChanged && playback_ip->speedNumerator < 0)
        {
            /* If playlist changes when we are rewinding, it will only happen because playlist was reloaded for Event Type HLS Session. */
            /* Since we use the lowest bitrate playlist throughout the trickmodes operation, we can still use sequence number to find the previous segment to play. */
            /* This is because it is valid to use sequence #s within the same playlist but not across playlists. */
            for (mediaFileSegmentInfo = BLST_Q_FIRST(&playlistFileInfo->mediaFileSegmentInfoQueueHead);
                    mediaFileSegmentInfo;
                    mediaFileSegmentInfo = BLST_Q_NEXT(mediaFileSegmentInfo, next))
            {
                if (mediaFileSegmentInfo->mediaSequence >= currentMediaSegmentSequenceNumber) {
                    /* found the segment whose sequence # happens to be 1 > than the current segment seq #. */
                    if (mediaFileSegmentInfo != BLST_Q_FIRST(&playlistFileInfo->mediaFileSegmentInfoQueueHead)) {
                        /* since we are going back, we pick the previos segment unless we have reached the 1st one. */
                        mediaFileSegmentInfo = BLST_Q_PREV(mediaFileSegmentInfo, next);
                    }
                    else {
                        /* We are at the first entry in the rewind case. */
                        /* Set a flag to indicate that we have now downloaded all segments, so that we can give BOF event to app when we are done playing the segments. */
                        hlsSession->downloadedAllSegmentsInCurrentPlaylist = true;
                    }
#ifdef BDBG_DEBUG_BUILD
                    if (playback_ip->ipVerboseLog) {
                        BDBG_MSG(("%s: Rew case: Found next media segment using seq: asked %d, got %d", BSTD_FUNCTION, currentMediaSegmentSequenceNumber, mediaFileSegmentInfo->mediaSequence));
                        BDBG_MSG(("uriReq %s", mediaFileSegmentInfo->uri));
                    }
#endif
                    break;
                }
            }
        }
        else if (hlsSession->currentPlaylistFile->playlistHasChanged && playback_ip->speedNumerator > 1)
        {
            /* If playlist changes when we are going fast-forward, it will only happen because playlist was reloaded for Event Type HLS Session. */
            /* Since we use the lowest bitrate playlist throughout the trickmodes operation, we can still use sequence number to find the previous segment to play. */
            /* This is because it is valid to use sequence #s within the *same* playlist but not across playlist of different variants as the seq #s may not match. */
            for (mediaFileSegmentInfo = BLST_Q_FIRST(&playlistFileInfo->mediaFileSegmentInfoQueueHead);
                    mediaFileSegmentInfo;
                    mediaFileSegmentInfo = BLST_Q_NEXT(mediaFileSegmentInfo, next))
            {
                if (mediaFileSegmentInfo->mediaSequence > currentMediaSegmentSequenceNumber) {
                    /* found the segment whose sequence # happens to be 1 > than the current segment seq #, use this segment */
#ifdef BDBG_DEBUG_BUILD
                    if (playback_ip->ipVerboseLog) {
                        BDBG_MSG(("%s: Fwd case: Found next media segment using seq: asked %d, got %d", BSTD_FUNCTION, currentMediaSegmentSequenceNumber, mediaFileSegmentInfo->mediaSequence));
                        BDBG_MSG(("uriReq %s", mediaFileSegmentInfo->uri));
                    }
#endif
                    if (mediaFileSegmentInfo == BLST_Q_LAST(&playlistFileInfo->mediaFileSegmentInfoQueueHead)) {
                        /* Set a flag to indicate that we have now downloaded all segments in the fwd direction, so that we can give ClientEndOfSegment event to app when we are done playing the segments. */
                        hlsSession->downloadedAllSegmentsInCurrentPlaylist = true;
                    }
                    break;
                }
            }
        }
        else
        {
            /* Normal Play case when playlist changes! */
            /* Select the media segment in the new playlist using the time position of how much mediaSegments we have currently downloaded. */
            /* We can't rely on segment Sequence# to locate the next segment when we have switched the playlists, */
            /* as some servers are not aligning their segment numbers across different playlists!. */

            if (hlsSession->segmentsRemovedDuration) {
                /* This will be set for live playlists where older segments are removed & newer ones are added as the time passes by. */
                /* segmentRemovedDuration is the amount of time the previous playlist has removed. We this as the base of the segmentDuration. */
                /* as we match the next segment using the total media segment time position since start of the playback. */
                segmentDuration = hlsSession->segmentsRemovedDuration;
            }
            else {
                segmentDuration = 0;
            }
            BDBG_MSG(("%s: segmentDuration %lu, desired downloadedSegmentsDuration %lu", BSTD_FUNCTION, segmentDuration, downloadedSegmentsDuration));
            for (mediaFileSegmentInfo = BLST_Q_FIRST(&playlistFileInfo->mediaFileSegmentInfoQueueHead);
                    mediaFileSegmentInfo;
                    mediaFileSegmentInfo = BLST_Q_NEXT(mediaFileSegmentInfo, next))
            {
                BDBG_MSG_FLOW(("%s: segmentDuration %u, desired downloadedSegmentsDuration %d, uri %s", BSTD_FUNCTION, segmentDuration, downloadedSegmentsDuration, mediaFileSegmentInfo->uri));
                segmentDuration += mediaFileSegmentInfo->duration;
                if (downloadedSegmentsDuration < segmentDuration) {
#ifdef BDBG_DEBUG_BUILD
                    if (playback_ip->ipVerboseLog) {
                        BDBG_MSG(("%s: Found next media segment using segment position: downloadedSegmentsDuration %lu, next downloadSegmentDuration %lu, uri %s", BSTD_FUNCTION, downloadedSegmentsDuration, segmentDuration, mediaFileSegmentInfo->uri));
                        BDBG_MSG(("uriReq %s", mediaFileSegmentInfo->uri));
                    }
#endif
                    break;
                }
            }
        }
        if (mediaFileSegmentInfo == NULL && playback_ip->speedNumerator >= 0) {
            /* we have already downloaded all corresponding segments in the new playlist as well & we are going in the forward direction, so we  update the currentMediaSegment */
            playlistFileInfo->currentMediaFileSegment = BLST_Q_LAST(&playlistFileInfo->mediaFileSegmentInfoQueueHead);
        }
        hlsSession->currentPlaylistFile->playlistHasChanged = false;
    }
    else {
        /* Using same playlist, so we can use the next/prev media segment. */
        /* Plus, we need to consider if we need to skip some segments if we are falling behind the current playback rate. */
        {
            bool droppingSegment = false;
            if (playback_ip->speedNumerator >= 1) {
                /* Playing in forward direction (speed doesn't matter) within the same playlist, just pick the next media segment. */
                mediaFileSegmentInfo = BLST_Q_NEXT(playlistFileInfo->currentMediaFileSegment, next);
            }
            else {
                /* Playing in reverse direction within the same playlist, just pick the previous media segment. */
                mediaFileSegmentInfo = BLST_Q_PREV(playlistFileInfo->currentMediaFileSegment, next);
            }
            /* Now determine if we need to skip some segments if we are in trickmodes. */
            if (mediaFileSegmentInfo && playback_ip->speedNumerator != 1)
            {
                B_Time endTime;
                hlsSession->totalRealElapsedTimeInSec += hlsSession->lastSegmentDuration/1000;
                B_Time_Get(&endTime);

                if (B_Time_Diff(&endTime, &hlsSession->currentSampleTime) >= 1000) {     /* 1 sec. */
                    /* 1 sec interval has elapsed. */
                    /* Check if we have downloaded (maps to decoded/displayed) segments at the desired trickmode rate. */
                    /* if yes, we dont need to need to skip any segments & we can continue downloading the next segment. */
                    /* Otherwise, we are falling behind than the desired rate. So we need to skip some segments. */
                    BDBG_WRN(("%s: totalRealElapsedTimeInSec=%u totalIdealElapsedTimeInSec=%u",
                                BSTD_FUNCTION, hlsSession->totalRealElapsedTimeInSec, hlsSession->totalIdealElapsedTimeInSec));
                    if (hlsSession->totalRealElapsedTimeInSec < hlsSession->totalIdealElapsedTimeInSec)
                    {
                        /* We are behind either because of download speed lesser than the needed speed for a given speed, */
                        /* or because of decoder is maxing out on its decode b/w as it can only decode certain # of frames/sec for a given stream. */

                        /* Determine how many segments to skip. */
                        unsigned i=0;
                        unsigned durationToCatchupInSec = hlsSession->totalIdealElapsedTimeInSec - hlsSession->totalRealElapsedTimeInSec;
                        unsigned segmentsToSkip = (durationToCatchupInSec / (hlsSession->currentPlaylistFile->maxMediaSegmentDuration/1000));

                        /* Now skip these many segments if any! */
                        for (i=0; i<segmentsToSkip && mediaFileSegmentInfo; i++)
                        {
                            if (playback_ip->speedNumerator > 1) {
                                mediaFileSegmentInfo = BLST_Q_NEXT(mediaFileSegmentInfo, next);
                            }
                            else {
                                mediaFileSegmentInfo = BLST_Q_PREV(mediaFileSegmentInfo, next);
                            }
                        }

                        /* And update the totalRealElapsedTime based on the skipped segments. */
                        hlsSession->totalRealElapsedTimeInSec += (segmentsToSkip*(hlsSession->currentPlaylistFile->maxMediaSegmentDuration/1000));
#ifdef BDBG_DEBUG_BUILD
                        if (playback_ip->ipVerboseLog) {
                            BDBG_WRN(("%s: durationToCatchupInSec=%u segmentsToSkip=%u totalRealElapsedTimeInSec=%u", BSTD_FUNCTION, durationToCatchupInSec, segmentsToSkip, hlsSession->totalRealElapsedTimeInSec));
                        }
#endif
                    }
                    hlsSession->currentSampleTime = endTime;
                    hlsSession->totalIdealElapsedTimeInSec += abs(playback_ip->speedNumerator);
                }
                droppingSegment = true;
            }
            if (mediaFileSegmentInfo == NULL && playback_ip->speedNumerator > 1 && droppingSegment) {
                /* reached the end for the fwd case, so lets play the last segment instead of dropping it. */
                mediaFileSegmentInfo = BLST_Q_LAST(&playlistFileInfo->mediaFileSegmentInfoQueueHead);
                hlsSession->downloadedAllSegmentsInCurrentPlaylist = true;
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog) {
                    BDBG_MSG(("%s: last media segment: setting downloadedAllSegmentsInCurrentPlaylist to true!", BSTD_FUNCTION));
                    BDBG_MSG(("uriReq %s", mediaFileSegmentInfo->uri));
                }
#endif
            }
            else if (mediaFileSegmentInfo == NULL && playback_ip->speedNumerator < 1) {
                /* reached the begining, so lets play the first segment instead of dropping it. */
                mediaFileSegmentInfo = BLST_Q_FIRST(&playlistFileInfo->mediaFileSegmentInfoQueueHead);
                hlsSession->downloadedAllSegmentsInCurrentPlaylist = true;
            }
        }
        if (mediaFileSegmentInfo) {
            BDBG_MSG(("%s: using same playlist file, simply return the next entry w/ seq# %d", BSTD_FUNCTION, mediaFileSegmentInfo->mediaSequence));
            BDBG_MSG(("uriReq %s", mediaFileSegmentInfo->uri));
        }
        else {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: next entry is NULL, this may be it for bounded streams!!!", BSTD_FUNCTION));
#endif
        }
    }
    if (mediaFileSegmentInfo)
        playlistFileInfo->currentMediaFileSegment = mediaFileSegmentInfo;

    return mediaFileSegmentInfo;

error:
    return NULL;
}

static B_PlaybackIpError updateStcRate(
    B_PlaybackIpHandle playback_ip,
    int rate
    )
{
    if (playback_ip->nexusHandles.simpleStcChannel) {
        if (NEXUS_SimpleStcChannel_SetRate(playback_ip->nexusHandles.simpleStcChannel, rate, 0 ) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: failed to pause by setting stc rate to %d", BSTD_FUNCTION, rate));
            return B_ERROR_UNKNOWN;
        }
    }
    else if (playback_ip->nexusHandles.stcChannel) {
        if (NEXUS_StcChannel_SetRate(playback_ip->nexusHandles.stcChannel, rate, 0 ) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: failed to pause by setting stc rate to %d", BSTD_FUNCTION, rate));
            return B_ERROR_UNKNOWN;
        }
    }
    else {
        BDBG_ERR(("%s: ERROR: neither simpleStcChannel or stcChannel rate set to %d", BSTD_FUNCTION, rate));
        return B_ERROR_UNKNOWN;
    }
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: Updated STC Channel rate to %d", BSTD_FUNCTION, rate));
#endif
    return B_ERROR_SUCCESS;
}

static B_PlaybackIpError updateAudioDecoderTrickModeState(
    B_PlaybackIpHandle playback_ip,
    NEXUS_AudioDecoderHandle audioDecoder
    )
{
    float rate;
    int speedNumerator, speedDenominator;
    NEXUS_AudioDecoderTrickState audioDecoderTrickSettings;

    speedNumerator = playback_ip->speedNumerator;
    speedDenominator = playback_ip->speedDenominator;
    rate = (float) speedNumerator / speedDenominator;

    if (audioDecoder)
        NEXUS_AudioDecoder_GetTrickState(audioDecoder, &audioDecoderTrickSettings);
    else if (playback_ip->nexusHandles.simpleAudioDecoder)
        NEXUS_SimpleAudioDecoder_GetTrickState(playback_ip->nexusHandles.simpleAudioDecoder, &audioDecoderTrickSettings);
    if ( rate == 1.0 )
    {
        audioDecoderTrickSettings.muted = false;
        audioDecoderTrickSettings.forceStopped = false;
        audioDecoderTrickSettings.tsmEnabled = true;
        audioDecoderTrickSettings.stcTrickEnabled = false;
    }
    else if ( rate < 0)
    {
        audioDecoderTrickSettings.muted = true;
        audioDecoderTrickSettings.forceStopped = true;
        audioDecoderTrickSettings.tsmEnabled = false;
    }
    else
    {
        audioDecoderTrickSettings.muted = true;
        audioDecoderTrickSettings.forceStopped = true;
        audioDecoderTrickSettings.tsmEnabled = false;
    }

    audioDecoderTrickSettings.rate = NEXUS_NORMAL_DECODE_RATE * rate;
    if (audioDecoder) {
        if (NEXUS_AudioDecoder_SetTrickState(audioDecoder, &audioDecoderTrickSettings) != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s: NEXUS_AudioDecoder_SetTrickState() failed for primary audio decoder \n", BSTD_FUNCTION));
            return  B_ERROR_UNKNOWN;
        }
    }
    else
    {
        int i;
        if (playback_ip->nexusHandles.simpleAudioDecoderCount) {
            for (i=0; i<playback_ip->nexusHandles.simpleAudioDecoderCount; i++) {
                if (NEXUS_SimpleAudioDecoder_SetTrickState(playback_ip->nexusHandles.simpleAudioDecoders[i], &audioDecoderTrickSettings) != NEXUS_SUCCESS) {
                    BDBG_ERR(("%s: NEXUS_AudioDecoder_SetTrickState() failed for primary audio decoder \n", BSTD_FUNCTION));
                    return  B_ERROR_UNKNOWN;
                }
                BDBG_WRN(("%s: NEXUS_AudioDecoder_SetTrickState() handle=%p i=%d\n", BSTD_FUNCTION, (void *)playback_ip->nexusHandles.simpleAudioDecoders[i], i));
            }
        }
        else if (playback_ip->nexusHandles.simpleAudioDecoder) {
            if (NEXUS_SimpleAudioDecoder_SetTrickState(playback_ip->nexusHandles.simpleAudioDecoder, &audioDecoderTrickSettings) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_AudioDecoder_SetTrickState() failed for primary audio decoder \n", BSTD_FUNCTION));
                return  B_ERROR_UNKNOWN;
            }
            BDBG_WRN(("%s: NEXUS_AudioDecoder_SetTrickState() handle=%p", BSTD_FUNCTION, (void *)playback_ip->nexusHandles.simpleAudioDecoders));
        }
    }
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: audioDecoderTrickSettings: forceStopped %s, rate %d, muted %s, stcTrickEnabled %s, tsmEnabled %s", BSTD_FUNCTION,
                    audioDecoderTrickSettings.forceStopped ? "Y":"N",
                    audioDecoderTrickSettings.rate,
                    audioDecoderTrickSettings.muted ? "Y":"N",
                    audioDecoderTrickSettings.stcTrickEnabled ? "Y":"N",
                    audioDecoderTrickSettings.tsmEnabled ? "Y":"N"
                 ));
#endif
    return B_ERROR_SUCCESS;
}

static B_PlaybackIpError
updateVideoDecoderTrickModeState(
    B_PlaybackIpHandle playback_ip
    )
{
    float rate;
    bool pausePlaypump = false;
    int speedNumerator, speedDenominator;
    NEXUS_VideoDecoderTrickState videoDecoderTrickSettings;

    if (playback_ip->nexusHandles.videoDecoder)
        NEXUS_VideoDecoder_GetTrickState(playback_ip->nexusHandles.videoDecoder, &videoDecoderTrickSettings);
    else
        NEXUS_SimpleVideoDecoder_GetTrickState(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderTrickSettings);

    speedNumerator = playback_ip->speedNumerator;
    speedDenominator = playback_ip->speedDenominator;
    rate = (float) speedNumerator / speedDenominator;

    if (rate == 0.0)
        pausePlaypump = true;
    else
        pausePlaypump = false;
    if (NEXUS_Playpump_SetPause(playback_ip->nexusHandles.playpump, pausePlaypump) != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s: NEXUS_Playpump_SetPause() failed to %s", BSTD_FUNCTION, pausePlaypump ? "set pause":"unset pause"));
        return B_ERROR_UNKNOWN;
    }
    BDBG_MSG(("%s: NEXUS_Playpump_SetPause() set to %s", BSTD_FUNCTION, pausePlaypump ? "set pause":"unset pause"));
    if ( rate == 0.0 || rate == 1.0 )
    {
        videoDecoderTrickSettings.tsmEnabled = NEXUS_TsmMode_eEnabled;
        videoDecoderTrickSettings.decodeMode = NEXUS_VideoDecoderDecodeMode_eAll;
        videoDecoderTrickSettings.hostTrickModesEnabled = false;
        videoDecoderTrickSettings.rate = (rate == 1.0) ? NEXUS_NORMAL_DECODE_RATE : 0;
        videoDecoderTrickSettings.topFieldOnly = false;
        videoDecoderTrickSettings.forceStopped = false;
        videoDecoderTrickSettings.stcTrickEnabled = false;
        videoDecoderTrickSettings.reverseFields = false;
        videoDecoderTrickSettings.reorderingMode = NEXUS_VideoDecoderReorderingMode_eNone;
        videoDecoderTrickSettings.dqtEnabled = false;
    }
    else
    {
        /* for all other fast fwd or slow rewind speeds, update the settings here. */
        videoDecoderTrickSettings.tsmEnabled = NEXUS_TsmMode_eSimulated;
        videoDecoderTrickSettings.stcTrickEnabled = false;
        videoDecoderTrickSettings.rate = (speedNumerator*NEXUS_NORMAL_DECODE_RATE)/speedDenominator;
        videoDecoderTrickSettings.topFieldOnly = true;
        videoDecoderTrickSettings.stcTrickEnabled = false;
        videoDecoderTrickSettings.brcmTrickModesEnabled = false;
        videoDecoderTrickSettings.hostTrickModesEnabled = false;

        if (rate < 0)
        {
            videoDecoderTrickSettings.reverseFields = true;
            if (speedNumerator < -4) {
                videoDecoderTrickSettings.decodeMode = NEXUS_VideoDecoderDecodeMode_eI;
            }
            else {
                /* Decode I only at the lower rewind speeds, as I only mode includes both IDR & I-frames. */
                /* If we do both I & P, then rewind experience is not that smooth. */
                videoDecoderTrickSettings.decodeMode = NEXUS_VideoDecoderDecodeMode_eI;
            }
            videoDecoderTrickSettings.dqtEnabled = true;
            BDBG_ERR(("%s: DQT Settings: enabled %s, decodeMode %d", BSTD_FUNCTION, videoDecoderTrickSettings.dqtEnabled ? "Y":"N", videoDecoderTrickSettings.decodeMode));
        }
        else
        {
            videoDecoderTrickSettings.reverseFields = false;
            if (speedNumerator < 10 && !playback_ip->hlsSessionState->useIFrameTrickmodes)
            {
                /* At lower +ve speeds, we decode both IP to provide smoother trickmode experience at lower rates. */
                videoDecoderTrickSettings.decodeMode = NEXUS_VideoDecoderDecodeMode_eIP;
            }
            else
            {
                /* For all higher +ve speeds, we decode just the I-frames. */
                videoDecoderTrickSettings.decodeMode = NEXUS_VideoDecoderDecodeMode_eI;
            }
            videoDecoderTrickSettings.dqtEnabled = false;
        }
    }

    if (playback_ip->nexusHandles.videoDecoder) {
        if (NEXUS_VideoDecoder_SetTrickState(playback_ip->nexusHandles.videoDecoder, &videoDecoderTrickSettings) != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s: NEXUS_VideoDecoder_SetTrickState() failed \n", BSTD_FUNCTION));
            return B_ERROR_UNKNOWN;
        }
    }
    else {
        if (NEXUS_SimpleVideoDecoder_SetTrickState(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderTrickSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_SimpleVideoDecoder_SetTrickState() failed \n", BSTD_FUNCTION));
            return B_ERROR_UNKNOWN;
        }
    }

    {
        /* Increase the discard threshold so that video frames (which may have timeline jumps) are within the TSM threshold & thus are NOT discarded. */
        NEXUS_VideoDecoderSettings videoDecoderSettings;
        if (playback_ip->nexusHandles.videoDecoder) {
            NEXUS_VideoDecoder_GetSettings(playback_ip->nexusHandles.videoDecoder, &videoDecoderSettings);
        }
        else {
            NEXUS_SimpleVideoDecoder_GetSettings(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderSettings);
        }
        videoDecoderSettings.discardThreshold = (45000 * abs(speedNumerator) * 40); /* some really high number. */
        if (playback_ip->nexusHandles.videoDecoder) {
            if (NEXUS_VideoDecoder_SetSettings(playback_ip->nexusHandles.videoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: Failed to set the discard threshold to %d", BSTD_FUNCTION, videoDecoderSettings.discardThreshold));
                return B_ERROR_UNKNOWN;
            }
        }
        else {
            if (NEXUS_SimpleVideoDecoder_SetSettings(playback_ip->nexusHandles.simpleVideoDecoder, &videoDecoderSettings) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: Failed to set the discard threshold to %d", BSTD_FUNCTION, videoDecoderSettings.discardThreshold));
                return B_ERROR_UNKNOWN;
            }
        }
        BDBG_WRN(("%s: Set the Video Decoder PTS discard threshold to %d", BSTD_FUNCTION, videoDecoderSettings.discardThreshold));
    }
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("Video Decoder Settings: Host %s, TSM %s, TsmSimulated %s, StcTrick %s, Decode Rate %d, Mode %s, TopFieldOnly %s, RevFields %s, DQT %s",
                    videoDecoderTrickSettings.hostTrickModesEnabled? "Y":"N",
                    videoDecoderTrickSettings.tsmEnabled? "Y":"N",
                    videoDecoderTrickSettings.tsmEnabled == NEXUS_TsmMode_eSimulated? "Y":"N",
                    videoDecoderTrickSettings.stcTrickEnabled? "Y":"N",
                    videoDecoderTrickSettings.rate,
                    videoDecoderTrickSettings.decodeMode == NEXUS_VideoDecoderDecodeMode_eI ? "I-only":
                        (videoDecoderTrickSettings.decodeMode == NEXUS_VideoDecoderDecodeMode_eIP ? "IP-only": "All frames"),
                    videoDecoderTrickSettings.topFieldOnly? "Y":"N",
                    videoDecoderTrickSettings.reverseFields? "Y":"N",
                    videoDecoderTrickSettings.dqtEnabled? "Y":"N"
                 ));
#endif
    return B_ERROR_SUCCESS;
} /* updateVideoDecoderTrickModeState */

void
flushNexusPlaypump (
    B_PlaybackIpHandle playback_ip
    )
{
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: flushing Playpump", BSTD_FUNCTION));
#endif
    if (playback_ip->nexusHandles.playpump)
    {
        NEXUS_Playpump_Flush(playback_ip->nexusHandles.playpump);
    }
    if (playback_ip->nexusHandles.playpump2)
    {
        NEXUS_Playpump_Flush(playback_ip->nexusHandles.playpump2);
    }
}

void
flushNexusAudioVideoDecoders (
    B_PlaybackIpHandle playback_ip
    )
{
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: flushing AV Decoders", BSTD_FUNCTION));
#endif
    if (playback_ip->nexusHandles.stcChannel)
    {
        if (NEXUS_StcChannel_Invalidate(playback_ip->nexusHandles.stcChannel) != NEXUS_SUCCESS)
        {
            BDBG_WRN(("%s: NEXUS_StcChannel_Invalidate returned error", BSTD_FUNCTION));
        }
    }
#ifdef NEXUS_HAS_SIMPLE_DECODER
    if (playback_ip->nexusHandles.simpleStcChannel)
    {
        if (NEXUS_SimpleStcChannel_Invalidate(playback_ip->nexusHandles.simpleStcChannel) != NEXUS_SUCCESS)
        {
            BDBG_WRN(("%s: NEXUS_SimpleStcChannel_Invalidate returned error", BSTD_FUNCTION));
        }
    }
#endif
    if (playback_ip->nexusHandles.videoDecoder)
    {
        NEXUS_VideoDecoder_Flush(playback_ip->nexusHandles.videoDecoder);
    }
#ifdef NEXUS_HAS_SIMPLE_DECODER
    if (playback_ip->nexusHandles.simpleVideoDecoder)
    {
        NEXUS_SimpleVideoDecoder_Flush(playback_ip->nexusHandles.simpleVideoDecoder);
    }
#endif
    if (playback_ip->nexusHandles.primaryAudioDecoder)
    {
        if (NEXUS_AudioDecoder_Flush(playback_ip->nexusHandles.primaryAudioDecoder) != NEXUS_SUCCESS)
        {
            BDBG_WRN(("%s: NEXUS_AudioDecoder_Flush returned error", BSTD_FUNCTION));
        }
    }
    if (playback_ip->nexusHandles.secondaryAudioDecoder)
    {
        if (NEXUS_AudioDecoder_Flush(playback_ip->nexusHandles.secondaryAudioDecoder) != NEXUS_SUCCESS)
        {
            BDBG_WRN(("%s: NEXUS_AudioDecoder_Flush returned error", BSTD_FUNCTION));
        }
    }
#ifdef NEXUS_HAS_SIMPLE_DECODER
    if (playback_ip->nexusHandles.simpleAudioDecoder) {
        int i;
        if (playback_ip->nexusHandles.simpleAudioDecoderCount) {
            for (i=0; i<playback_ip->nexusHandles.simpleAudioDecoderCount; i++) {
                NEXUS_SimpleAudioDecoder_Flush(playback_ip->nexusHandles.simpleAudioDecoders[i]);
            }
        }
        else {
            NEXUS_SimpleAudioDecoder_Flush(playback_ip->nexusHandles.simpleAudioDecoder);
        }
    }
#endif

}

extern B_PlaybackIpError B_PlaybackIp_SetAudioPtsCallback( B_PlaybackIpHandle playback_ip);
extern B_PlaybackIpError B_PlaybackIp_SetVideoPtsCallback( B_PlaybackIpHandle playback_ip);
B_PlaybackIpError
B_PlaybackIp_SeekHls(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpState currentState,
    NEXUS_PlaybackPosition seekPosition,
    bool enableAccurateSeek,
    bool flushPipeline
    )
{
    B_PlaybackIpError brc;
    PlaylistFileInfo *playlistFileInfo;
    MediaFileSegmentInfo *mediaFileSegmentInfo = NULL;
    NEXUS_PlaybackPosition currentDuration = 0;
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;
    BERR_Code rc;
    int i;
    B_PlaybackIpState origCurrentState = playbackIpState(playback_ip);

    BDBG_MSG(("%s: seekPosition = %lu, state cur %d, orig %d ", BSTD_FUNCTION, seekPosition, currentState, origCurrentState));

    if (seekPosition > hlsSession->currentPlaylistFile->totalDuration) {
        BDBG_ERR(("%s: Incorrect seekPosition %lu, total media duration %lu", BSTD_FUNCTION, seekPosition, hlsSession->currentPlaylistFile->totalDuration));
        brc = B_ERROR_INVALID_PARAMETER;
        goto error;
    }

    if (hlsSession->hlsPlaybackThreadDone == true) {
        BDBG_ERR(("%s: Playback is already done, can't seek to %lu at this time, app should call stop & start again!", BSTD_FUNCTION, seekPosition));
        brc = B_ERROR_INVALID_PARAMETER;
        goto error;
    }

    /* now synchronize with the media segment download and playpump feeder threads */
    /* by changing the playback_state to WaitingToEnterTrickMode, these threads will pause their work, */
    /* signal this back to this thread and then wait for seek work to finish. Then, this thread should */
    /* signal these worker threads to resume downloading media segments and feeding to playpump. */
    BKNI_ResetEvent(hlsSession->segDownloadThreadPausedEvent);
    BKNI_ResetEvent(hlsSession->playbackThreadPausedEvent);
    BKNI_ResetEvent(hlsSession->playlistReDownloadThreadPausedEvent);
    hlsSession->seekOperationStarted = true;
    playback_ip->playback_state = B_PlaybackIpState_eWaitingToEnterTrickMode;
    rc = BKNI_WaitForEvent(hlsSession->segDownloadThreadPausedEvent, 2*HLS_EVENT_TIMEOUT_MSEC);
    if (rc == BERR_TIMEOUT) {
        BDBG_ERR(("%s: EVENT timeout: failed to receive event from HLS Segment download thread indicating its paused", BSTD_FUNCTION));
        brc = B_ERROR_TIMEOUT;
        goto error;
    } else if (rc!=0) {
        BDBG_ERR(("%s: failed to wait for event from HLS Segment download thread indicating its paused, rc = %d", BSTD_FUNCTION, rc));
        brc = B_ERROR_UNKNOWN;
        goto error;
    }
    BDBG_MSG(("%s: segment download thread is paused", BSTD_FUNCTION));
    /* now pause the playpump feeder thread */
    rc = BKNI_WaitForEvent(hlsSession->playbackThreadPausedEvent, 2*HLS_EVENT_TIMEOUT_MSEC);
    if (rc == BERR_TIMEOUT) {
        BDBG_ERR(("%s: EVENT timeout: failed to receive event from HLS Playpump feeder thread indicating its paused", BSTD_FUNCTION));
        brc = B_ERROR_TIMEOUT;
        goto error;
    } else if (rc!=0) {
        BDBG_ERR(("%s: failed to wait for event from HLS Playpump feeder thread indicating its paused, rc = %d", BSTD_FUNCTION, rc));
        brc = B_ERROR_UNKNOWN;
        goto error;
    }
    if (hlsSession->hlsPlaylistReDownloadThread && !hlsSession->hlsPlaylistReDownloadThreadDone) {
        /* now pause the playlistReDownload thread */

        /* since playlistReDownload thread may be waiting on the reDownload timer, we need to send an event to wake the thread up!. */
        BKNI_SetEvent(hlsSession->reDownloadPlaylistEvent);

        rc = BKNI_WaitForEvent(hlsSession->playlistReDownloadThreadPausedEvent, 2*HLS_EVENT_TIMEOUT_MSEC);
        if (rc == BERR_TIMEOUT) {
            BDBG_ERR(("%s: EVENT timeout: failed to receive event from HLS PlaylistReDownload thread indicating its paused", BSTD_FUNCTION));
            brc = B_ERROR_TIMEOUT;
            goto error;
        } else if (rc!=0) {
            BDBG_ERR(("%s: failed to wait for event from HLS PlaylistReDownload thread indicating its paused, rc = %d", BSTD_FUNCTION, rc));
            brc = B_ERROR_UNKNOWN;
            goto error;
        }
        BDBG_MSG(("%s: PlaylistReDownload is paused", BSTD_FUNCTION));
        /* reset the event that we had set, in case ReDownload wasn't waiting on this event to avoid it getting the stale events. */
        BKNI_ResetEvent(hlsSession->reDownloadPlaylistEvent);
    }
    BDBG_MSG(("%s: now both segment download and playpump feeder threads are paused, so do the seek related work", BSTD_FUNCTION));

    playlistFileInfo = playback_ip->hlsSessionState->currentPlaylistFile;
    BDBG_MSG(("%s: segmentCodecType %d", BSTD_FUNCTION, playlistFileInfo->segmentCodecType));
    /* install the firstPtsCallback so that we can correctly start maintaining the playback position after seek */
    if (playlistFileInfo->segmentCodecType == HlsSegmentCodecType_eAudioOnly || playback_ip->psi.mpegType == NEXUS_TransportType_eMpeg2Pes)
        B_PlaybackIp_SetAudioPtsCallback( playback_ip);
    else
        B_PlaybackIp_SetVideoPtsCallback( playback_ip);

    /* determine which mediaSegment lines up with the new seekPosition for the currentPlaylist being played out */
    /* TODO: improve this: instead of starting from start of list, figure a good location to start from */
    /* it can be 0) nearby the current position, 1) near front, 2) near end */
    for (mediaFileSegmentInfo = BLST_Q_FIRST(&playlistFileInfo->mediaFileSegmentInfoQueueHead);
            mediaFileSegmentInfo;
            mediaFileSegmentInfo = BLST_Q_NEXT(mediaFileSegmentInfo, next))
    {
        currentDuration += mediaFileSegmentInfo->duration;
        if (currentDuration > seekPosition) {
            BDBG_MSG(("%s: Found next media segment (# %d) with correct seekPosition %lu, currentDuration %lu, uri %s",
                        BSTD_FUNCTION, mediaFileSegmentInfo->mediaSequence, seekPosition, currentDuration, mediaFileSegmentInfo->uri));
            break;
        }
    }
    if (!mediaFileSegmentInfo) {
        if (B_PlaybackIp_HlsBoundedStream(playback_ip)) {
            BDBG_ERR(("%s: Failed to find a segment matching with seekPosition %lu, currentDuration %lu", BSTD_FUNCTION, seekPosition, currentDuration));
            brc = B_ERROR_UNKNOWN;
            goto error;
        }
        else {
            /* Not a bounded stream, but we dont currently have a segment at the last duration, so lets seek to the last segment. */
            /* hopefully, redownload thread will refresh the playlist w/ the new segments. */
            mediaFileSegmentInfo = BLST_Q_LAST(&playlistFileInfo->mediaFileSegmentInfoQueueHead);
        }
    }

    /* Since MediaSegment Download thread will start from the next segment (for fwd direction) & prev one (for rew case), */
    /* we will go either one segment back (for fwd direction) or forward (for reverse). */
    /* we set the currentMediaFileSegment to 1 entry before the one we want to seek to. */

    hlsSession->resetPlaylist = false;
    if (playback_ip->speedNumerator < 0) {
        /* rwd case, go one forward if not already at the last segment. */
        if (mediaFileSegmentInfo != BLST_Q_LAST(&playlistFileInfo->mediaFileSegmentInfoQueueHead)) {
            playlistFileInfo->currentMediaFileSegment = BLST_Q_NEXT(mediaFileSegmentInfo, next);
        }
        else {
            /* reached head, so use this segment only. */
           playlistFileInfo->currentMediaFileSegment = mediaFileSegmentInfo;
        }
    }
    else {
        /* fwd case, go one back if not already at the head. */
        if (mediaFileSegmentInfo != BLST_Q_FIRST(&playlistFileInfo->mediaFileSegmentInfoQueueHead)) {
            playlistFileInfo->currentMediaFileSegment = BLST_Q_PREV(mediaFileSegmentInfo, next);
        }
        else {
            /* reached the head */
            playlistFileInfo->currentMediaFileSegment = mediaFileSegmentInfo;
            if (playback_ip->speedNumerator == 1 && playback_ip->speedDenominator == 1) {
               hlsSession->resetPlaylist = true;
           }
        }
    }
    hlsSession->downloadedAllSegmentsInCurrentPlaylist = false;

#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: Seeking to URI %s (one behind than actual one!) at position %lu", BSTD_FUNCTION, playlistFileInfo->currentMediaFileSegment->uri, seekPosition));
#endif

    /* For seeks during normal play, flush the current pipeline so we resume from the new seek location */
    if (flushPipeline)
    {
        flushNexusPlaypump(playback_ip);
        flushNexusAudioVideoDecoders(playback_ip);
    }

    for (i=0; i<HLS_NUM_SEGMENT_BUFFERS; i++)
    {
        BKNI_AcquireMutex(playback_ip->hlsSessionState->segmentBuffer[i].lock);
        playback_ip->hlsSessionState->segmentBuffer[i].filled = false;
        playback_ip->hlsSessionState->segmentBuffer[i].bufferDepth = 0;
        BKNI_ReleaseMutex(playback_ip->hlsSessionState->segmentBuffer[i].lock);
    }

    /* Accurate Seek related logic */
    if (enableAccurateSeek)
    {
        uint32_t seekPts;
        bool useAccurateSeek = false;
        NEXUS_Error rc;

        seekPts = playback_ip->originalFirstPts + (seekPosition * 45);
        if (!B_PlaybackIp_HlsBoundedStream(playback_ip))
        {
            playback_ip->lastPtsExtrapolated = playback_ip->firstPts + (hlsSession->currentPlaylistFile->totalDuration *45);
        }

        if (playback_ip->speedNumerator < 0)
        {
            /* TODO: For rewind or forward case, for now, we dont use accurateSeek until it we get it reliably work! */
            useAccurateSeek = false;
        }
        else if (playback_ip->originalFirstPts < playback_ip->lastPtsExtrapolated)
        {
            /* No PTS wrap case, check if the seekPts falls in this range. */
            if (seekPts > playback_ip->originalFirstPts && seekPts < playback_ip->lastPtsExtrapolated)
                useAccurateSeek = true;
        }
        else if (playback_ip->originalFirstPts > playback_ip->lastPtsExtrapolated)
        {
            /* PTS wrap case, check if the seekPts falls in this range. */
            if (seekPts > playback_ip->originalFirstPts)
                useAccurateSeek = true;
            else if (seekPts < playback_ip->lastPtsExtrapolated)
                useAccurateSeek = true;
        }
        else
        {
            useAccurateSeek = false;
        }
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("%s: seekPosition %lu msec, seekPts 0x%x, absolutefirst pts 0x%x, firstPts 0x%x, extrapoluatedLastPts 0x%x, useAccurateSeek %d, durtion until last discontinuity %d msec", BSTD_FUNCTION,
                        seekPosition, seekPts, playback_ip->originalFirstPts, playback_ip->firstPts, playback_ip->lastPtsExtrapolated, useAccurateSeek, playback_ip->streamDurationUntilLastDiscontinuity));
#endif
        if (useAccurateSeek)
        {
            if (playback_ip->nexusHandles.videoDecoder) {
                rc = NEXUS_VideoDecoder_SetStartPts(playback_ip->nexusHandles.videoDecoder, seekPts);
            }
            else {
                rc = NEXUS_SimpleVideoDecoder_SetStartPts(playback_ip->nexusHandles.simpleVideoDecoder, seekPts);
            }
            if (rc != NEXUS_SUCCESS)
            {
                BDBG_ERR(("%s: ERROR: Failed to Set the PTS to %x, seekPosition %lu msec", BSTD_FUNCTION, seekPts, seekPosition ));
                brc = B_ERROR_UNKNOWN;
                goto error;
            }
        }
    }
    playback_ip->lastSeekPosition = seekPosition;
    playback_ip->lastSeekPositionSet = true;
    hlsSession->lastSeekPosition = seekPosition;
    hlsSession->seekOperationStarted = false;
    playback_ip->playback_state = B_PlaybackIpState_ePaused;
    hlsSession->restartPlaylistRampUpByBandwidth = true;
    BKNI_SetEvent(hlsSession->playlistReDownloadThreadPauseDoneEvent);
    BKNI_SetEvent(hlsSession->segDownloadThreadPauseDoneEvent);
    BKNI_SetEvent(hlsSession->playbackThreadPauseDoneEvent);
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: successful to %0.3f sec", BSTD_FUNCTION, seekPosition/1000.));
#endif
    return B_ERROR_SUCCESS;

error:
    /* restore the current state */
    playback_ip->playback_state = currentState;
    return (brc);
} /* B_PlaybackIp_SeekHls */

B_PlaybackIpError
B_PlaybackIp_PauseHls(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpState currentState
    )
{
    B_PlaybackIpError rc;
    BSTD_UNUSED(currentState);

    if (!playback_ip->hlsSessionState) {
        BDBG_ERR(("%s: ERROR: HlsSessionState is NULL", BSTD_FUNCTION));
        return B_ERROR_UNKNOWN;
    }

    if (currentState == B_PlaybackIpState_eTrickMode) {
        /* If Pausing from trickmode state, we need to update the Decoder settings as well as they are in Simulated STC mode */
        /* and freezing the h/w STC alone wont help. */
        playback_ip->hlsSessionState->playlistBandwidthToUseInTrickmode = 0;
        playback_ip->hlsSessionState->useLowestBitRateSegmentOnly = false;
        playback_ip->speedNumerator = 0;
        playback_ip->speedDenominator = 1;

        /* Configure VideoDecoder back to pause mode. */
        rc = updateVideoDecoderTrickModeState(playback_ip);
        if (rc != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: Failed to update VideoDecoder State during trickMode operation", BSTD_FUNCTION));
            goto error;
        }

        /* Configure AudioDecoder to pause mode */
        rc = updateAudioDecoderTrickModeState(playback_ip, playback_ip->nexusHandles.primaryAudioDecoder);
        if (rc != B_ERROR_SUCCESS)
        {
            BDBG_ERR(("%s: Failed to update AudioDecoder State during trickMode operation", BSTD_FUNCTION));
            goto error;
        }

        playback_ip->isPausedFromTrickMode = true;
    }
    else {
        playback_ip->isPausedFromTrickMode = false;
    }

    /* In HLS pause from play state, we dont disconnect the server connection and instead keep that going */
    /* We instead pause by setting STC rate to 0, this way download and playback threads still continue until all buffers get full */
    rc = updateStcRate(playback_ip, 0);
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: playback_ip %p: current state %d, PAUSED", BSTD_FUNCTION, (void *)playback_ip, currentState));
#endif
error:
    return (rc);
}

B_PlaybackIpError
B_PlaybackIp_PlayHls(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpState currentState
    )
{
    B_PlaybackIpError brc;

    /*
     * Special handling for transitioning to Play from TrickMode state.
     */
    if (currentState == B_PlaybackIpState_eTrickMode || (currentState == B_PlaybackIpState_ePaused && playback_ip->isPausedFromTrickMode == true)) {
        NEXUS_PlaybackPosition currentPosition;

        /* While resuming to Play from Trickmode or TrickMode->Pause->Play, we need to seek to the current position & update the AV decoder settings. */
        brc = updateStcRate(playback_ip, 0);
        if (brc != B_ERROR_SUCCESS)
        {
            BDBG_WRN(("%s: Failed to Pause during Trick -> Play", BSTD_FUNCTION));
            goto error;
        }

        /*
         * Get the current position where we would to resume the playback from. This position is obtained
         * using the simulated STC calculation which assumes that we dont underflow.
         * We can alternatively use the current PTS method and allow apps to pick one.
         */
        brc = B_PlaybackIp_HttpGetCurrentPlaybackPosition(playback_ip, &currentPosition);
        if (brc != B_ERROR_SUCCESS)
        {
            BDBG_WRN(("%s: Failed to determine the current playback position", BSTD_FUNCTION));
            goto error;
        }
        BDBG_MSG(("%s: Resume: Trick -> Play from position %0.3f", BSTD_FUNCTION, currentPosition/1000.));

        /* Flush Playpump buffer as once we return from Seek, feeder & playpback threads would start their work. */
        flushNexusPlaypump(playback_ip);

        playback_ip->hlsSessionState->useIFrameTrickmodes = false;
        brc = B_PlaybackIp_SeekHls( playback_ip, currentState, currentPosition, true /* enableAccurateSeek */, true /* flushPipeline */ );
        if (brc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s: ERROR: Failed to Seek to the resume from %0.3f position during TrickMode -> Play transition", BSTD_FUNCTION, currentPosition/1000.));
            goto error;
        }

        /* Reset HLS related special media segment download flags. */
        playback_ip->hlsSessionState->playlistBandwidthToUseInTrickmode = 0;
        playback_ip->hlsSessionState->useLowestBitRateSegmentOnly = false;
        playback_ip->isPausedFromTrickMode = false;

        /* Configure VideoDecoder back to the normal play mode. */
        brc = updateVideoDecoderTrickModeState(playback_ip);
        if (brc != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: Failed to update VideoDecoder State during trickMode operation", BSTD_FUNCTION));
            goto error;
        }

        /* Configure AudioDecoders back to normal play mode. */
        /* TODO: need to add simple decoder support */
        brc = updateAudioDecoderTrickModeState(playback_ip, playback_ip->nexusHandles.primaryAudioDecoder);
        if (brc != B_ERROR_SUCCESS)
        {
            BDBG_ERR(("%s: Failed to update AudioDecoder State during trickMode operation", BSTD_FUNCTION));
            goto error;
        }
        brc = updateAudioDecoderTrickModeState(playback_ip, playback_ip->nexusHandles.secondaryAudioDecoder);
        if (brc != B_ERROR_SUCCESS)
        {
            BDBG_ERR(("%s: Failed to update secondaryAudioDecoder State during play operation", BSTD_FUNCTION));
            goto error;
        }

        /* Now flush the decoders so that they use the new trickmode settngs. */
        /* Note: this flush is needed after the trickmode settings as it internally causes XVD stop/start to account for trickmode settings. */
        flushNexusAudioVideoDecoders(playback_ip);
    }

    playback_ip->hlsSessionState->segmentCount = 0;
    /* Now resume the STC rate and we should be good */
    brc = updateStcRate(playback_ip, 1);

#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: Going back to Play State: rc %d, playback_ip %p, current state %d", BSTD_FUNCTION, brc, (void *)playback_ip, currentState));
#endif

error:
    return (brc);
}

B_PlaybackIpError B_PlaybackIp_TrickModeHls(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpState currentState,
    B_PlaybackIpTrickModesSettings *pIpTrickModeSettings)
{
    B_PlaybackIpError brc;
    int requiredNetworkBandwidth = 0;
    int playbackRate, adjustedPlaybackRate;
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;
    PlaylistFileInfo *playlistFileInfo;
    PlaylistFileInfo *prevPlaylistFileInfo;
    NEXUS_PlaybackPosition currentPosition;
    int networkBandwidth;

    /*
     * Fast Fwd or Slow/Fast Rewind Case.
     * Since we buffer ahead, we will need to flush those segments. Also, we will need to know the position
     * from where the rewind should begin and then start the rewind from there.
     * After we determine the current position, we call _SeekHls to flush and seek to the right location.
     * Selecting the correct playlist & its segment will happen when segment download thread tries to pick the next playlist/segment.
     */

    /* Pause STC to stop the current Decode & Display. */
    /*brc = B_PlaybackIp_PauseHls(playback_ip, currentState);*/
    brc = updateStcRate(playback_ip, 0);
    if (brc != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: Failed to Pause STC during trickMode operation", BSTD_FUNCTION));
        goto error;
    }

    brc = B_PlaybackIp_HttpGetCurrentPlaybackPosition(playback_ip, &currentPosition);
    if (brc != B_ERROR_SUCCESS)
    {
        BDBG_WRN(("%s: Failed to determine the current playback position during HLS Rewind", BSTD_FUNCTION));
        goto error;
    }
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: %s -> %s at %d rate from position %0.3f", BSTD_FUNCTION,
                    currentState == B_PlaybackIpState_eTrickMode ? "Trickmode":"Play",
                    playback_ip->speedNumerator > 0 ? "Fast Fwd":"Rwd",
                    playback_ip->speedNumerator/playback_ip->speedDenominator, currentPosition/1000.));
#endif

    hlsSession->useIFrameTrickmodes = BLST_Q_FIRST(&hlsSession->iFramePlaylistFileInfoQueueHead) ? true : false;
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: Using %s trickmodes", BSTD_FUNCTION, hlsSession->useIFrameTrickmodes ? "Server-side (IFrame)" : "Client-side"));
#endif

    if (pIpTrickModeSettings->adaptiveStreamingTrickmodeMethod == B_PlaybackIpAdaptiveStreamingTrickModeMethod_eUseSegmentWithLowestBandwidth)
    {
        /*
         * Set a flag to indicate to that we want to switch to the media playlist w/ smallest bitrate.
         * Media Segment Download thread will switch to downloading segments from such playlist once it finishes downloading the current segment.
         * We will continue to download these segments until we resume to normal mode.
         * We are doing this as we want to download & feed the media segments AS FAST AS WE CAN during trickmodes
         * since we are using Simulated STC mode to speed up the STC to match the app desired "rate".
         */

        /* TODO: add logic to use adjustedRate if requested rate can't be played at the current network conditions. */
        hlsSession->useLowestBitRateSegmentOnly = true;
        playlistFileInfo = BLST_Q_FIRST(hlsSession->useIFrameTrickmodes ? &hlsSession->iFramePlaylistFileInfoQueueHead:&hlsSession->playlistFileInfoQueueHead);
    }
    else
    {
        /*
         * Instead of using the playlist w/ smallest bitrate, we can instead select playlist with bitrate
         * that can be played at the *requested rate* for the current network bandwidth.
         * This allows us to provide better AV quality during trickmodes.
         *
         * Check if current network bandwidth allows us to sustain the requested rate.
         * if not, we lower the rate to match the available bandwidth and choose such playlist.
         */
        playbackRate = playback_ip->speedNumerator;
        networkBandwidth = B_PlaybackIp_HlsGetCurrentBandwidth(&hlsSession->bandwidthContext);
        for (prevPlaylistFileInfo = NULL, playlistFileInfo = BLST_Q_FIRST(hlsSession->useIFrameTrickmodes ? &hlsSession->iFramePlaylistFileInfoQueueHead:&hlsSession->playlistFileInfoQueueHead);
                playlistFileInfo;
                playlistFileInfo = BLST_Q_NEXT(playlistFileInfo, next))
        {
            if (playlistFileInfo->segmentCodecType == HlsSegmentCodecType_eAudioOnly)
            {
                continue;
            }
            requiredNetworkBandwidth = playbackRate * playlistFileInfo->bandwidth;
            BDBG_WRN(("%s: playlist file: uri %s, b/w %d, requiredNetworkBandwidth %d, network Bandwidth %d", BSTD_FUNCTION, playlistFileInfo->uri, playlistFileInfo->bandwidth, requiredNetworkBandwidth, networkBandwidth));
            if (requiredNetworkBandwidth > networkBandwidth )
            {
                /* Using this playlist at requested rate will exceed the current network bandwidth, so we will use the previous playlist. */
                /* Note that the list of playlist file entries are maintained in the ascending order of their bandwidth value. */
                BDBG_WRN(("%s: requiredNetworkBandwidth %d at %d rate exceeds the network b/w %d of playlist w/ b/w %d, use previous playlist entry %p",
                            BSTD_FUNCTION, requiredNetworkBandwidth, playbackRate, networkBandwidth, playlistFileInfo->bandwidth, (void *)prevPlaylistFileInfo));
                break;
            }
            else
            {
                /* check next playlist to see if it can be played at the request rate. */
                prevPlaylistFileInfo = playlistFileInfo;
                continue;
            }
        }

        if (prevPlaylistFileInfo != NULL)
        {
            /* Found a playlist which can be played at the requested rate for the current n/w b/w. */
            playlistFileInfo = prevPlaylistFileInfo;
            BDBG_WRN(("%s: %d for this playlist b/w %d at %d rate can be played within the n/w b/w of  %d, playing at highest possible quality",
                        BSTD_FUNCTION, requiredNetworkBandwidth, playlistFileInfo->bandwidth, playbackRate, networkBandwidth));
        }
        else {
            /* Found no playlists that can be played at the request playback rate for the current n/w b/w. */
            /* In this case, we take the smallest AV b/w playlist and play it at the highest possible rate for the current n/w b/w. */
            for (playlistFileInfo = BLST_Q_FIRST(hlsSession->useIFrameTrickmodes ? &hlsSession->iFramePlaylistFileInfoQueueHead:&hlsSession->playlistFileInfoQueueHead);
                    playlistFileInfo;
                    playlistFileInfo = BLST_Q_NEXT(playlistFileInfo, next))
            {
                if (playlistFileInfo->segmentCodecType != HlsSegmentCodecType_eAudioOnly)
                {
                    break;
                }
            }
            if (playlistFileInfo == NULL)
            {
                playlistFileInfo = BLST_Q_FIRST(hlsSession->useIFrameTrickmodes ? &hlsSession->iFramePlaylistFileInfoQueueHead:&hlsSession->playlistFileInfoQueueHead);
                if (playlistFileInfo == NULL)
                {
                    BDBG_ERR(("%s: SW Bug: playlist list seems to be corrupted", BSTD_FUNCTION));
                    goto error;
                }
                else
                {
                    BDBG_WRN(("%s: no AV playlist available, so using 1st audio only playlist w/ b/w %d for this requiredNetworkBandwidth %d at the current networkBandwidth %d",
                                BSTD_FUNCTION, playlistFileInfo->bandwidth, requiredNetworkBandwidth, networkBandwidth));
                }
            }
            adjustedPlaybackRate = networkBandwidth / playlistFileInfo->bandwidth;
            BDBG_WRN(("%s: Can't play any playlists at this rate %d, lowering the rate to %d using 1st playlist of b/w %d, n/w b/w %d",
                        BSTD_FUNCTION, playback_ip->speedNumerator, adjustedPlaybackRate, playlistFileInfo->bandwidth, networkBandwidth));
            playback_ip->speedNumerator = adjustedPlaybackRate;
        }
        hlsSession->playlistBandwidthToUseInTrickmode = playlistFileInfo->bandwidth;
    }
    if (hlsSession->useIFrameTrickmodes) {
        if (B_PlaybackIp_HlsConnectDownloadAndParsePlaylistFile(playback_ip, hlsSession, playlistFileInfo) < 0) {
            BDBG_ERR(("%s: Failed to download & parse playlist file entry w/ uri %s", BSTD_FUNCTION, playlistFileInfo->uri));
        }
    }

    /* Reset the state used to determine if we are able to keep up the ideal elapsed time for a given play speed. */
    B_Time_Get(&hlsSession->currentSampleTime);
    hlsSession->totalIdealElapsedTimeInSec = abs(playback_ip->speedNumerator);
    hlsSession->totalRealElapsedTimeInSec = 0;
    hlsSession->segmentCount = 0;

    /* Now seek to the last played position from where we should start downloading the segments. */
    brc = B_PlaybackIp_SeekHls( playback_ip, currentState, currentPosition, true /* enableAccurateSeek */, true /* flushPipeline */ );
    if (brc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s: ERROR: Failed to Seek to the resume from %0.3f position during TrickMode -> Play transition", BSTD_FUNCTION, currentPosition/1000.));
        goto error;
    }
    if (hlsSession->useIFrameTrickmodes) {
        hlsSession->currentPlaylistFile = playlistFileInfo;
    }
    playback_ip->playback_state = B_PlaybackIpState_eTrickMode;

    /* Configure VideoDecoder for TrickMode state. */
    brc = updateVideoDecoderTrickModeState(playback_ip);
    if (brc != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: Failed to update VideoDecoder State during trickMode operation", BSTD_FUNCTION));
        goto error;
    }

    /* Configure AudioDecoder for TrickMode state. */
    brc = updateAudioDecoderTrickModeState(playback_ip, playback_ip->nexusHandles.primaryAudioDecoder);
    if (brc != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: Failed to update primaryAudioDecoder State during trickMode operation", BSTD_FUNCTION));
        goto error;
    }

    brc = updateAudioDecoderTrickModeState(playback_ip, playback_ip->nexusHandles.secondaryAudioDecoder);
    if (brc != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: Failed to update secondaryAudioDecoder State during trickMode operation", BSTD_FUNCTION));
        goto error;
    }

    /* Now flush the decoders so that they use the new trickmode settngs. */
    /* Note: this flush is needed after the trickmode settings as it internally causes XVD stop/start to account for DQT related settings. */
    flushNexusAudioVideoDecoders(playback_ip);
    playback_ip->beginningOfStreamCallbackIssued = false; /* reset the flag so that we issue the callback if we reach the start of the stream. */
    playback_ip->endOfClientBufferCallbackIssued = false;

    /* And then Un-Pause the STC to get decoders going! */
    brc = updateStcRate(playback_ip, 1);
    if (brc != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: Failed to Pause during trickMode operation", BSTD_FUNCTION));
        goto error;
    }
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: %p TrickMode started", BSTD_FUNCTION, (void *)playback_ip));
#endif
    return brc;

error:
    playback_ip->playback_state = currentState;
    return brc;
}

static bool
processId3v2HeaderAndConvertToPes(B_PlaybackIpHandle playback_ip, HlsSessionState *hlsSession, HlsSegmentBuffer *segmentBuffer)
{
    batom_t atom;
    batom_cursor cursor;
    bid3v2_header id3v2Header;
    size_t pesHeaderLen;
    size_t pesPayloadLen;
    uint8_t pesHeader[64];
    size_t totalId3v2HeaderSize;
    static int cnt = 0;
    unsigned timestamp=0;
    bool segmentContainsId3Header = false;

    /*
     * ID3v2 tag/header format:
     * ID3v2/file identifier      "ID3"
     * ID3v2 version              $03 00
     * ID3v2 flags                %abc00000
     * ID3v2 size             4 * %0xxxxxxx
     * Example:
        00000000  49 44 33 04 00 00 00 00  00 3f 50 52 49 56 00 00  |ID3......?PRIV..|
        00000010  00 35 00 00 63 6f 6d 2e  61 70 70 6c 65 2e 73 74  |.5..com.apple.st|
        00000020  72 65 61 6d 69 6e 67 2e  74 72 61 6e 73 70 6f 72  |reaming.transpor|
        00000030  74 53 74 72 65 61 6d 54  69 6d 65 73 74 61 6d 70  |tStreamTimestamp|
        00000040  00 00 00 00 00 05 66 76  00
     */
    if (segmentBuffer->buffer[0] == 'I' && segmentBuffer->buffer[1] == 'D' && segmentBuffer->buffer[2] == '3') {
        BDBG_MSG(("%s: ID3v2 identifier (ID3) is present, found %c %c %c", BSTD_FUNCTION, segmentBuffer->buffer[0], segmentBuffer->buffer[1], segmentBuffer->buffer[2]));
        segmentContainsId3Header = true;
    }
    else {
        BDBG_MSG(("%s: ID3v2 identifier (ID3) is missing, must be continuation of the current ID3 segment", BSTD_FUNCTION));
        segmentContainsId3Header = false;
    }
    if (segmentContainsId3Header) {
        if (!playback_ip->factory) {
            playback_ip->factory = batom_factory_create(bkni_alloc, 16);
            if (playback_ip->factory == NULL) {
                BDBG_ERR(("%s: Failed to Create batom_factory for converting HLS ES to PES", BSTD_FUNCTION));
                return false;
            }
            BDBG_MSG(("%s: Created batom_factory %p for converting HLS ES to PES", BSTD_FUNCTION, (void *)playback_ip->factory));
        }
        atom = batom_from_range(playback_ip->factory, segmentBuffer->buffer, segmentBuffer->bufferDepth, NULL, NULL);
        batom_cursor_from_atom(&cursor, atom);
        if (bid3v2_parse_header(&cursor, &id3v2Header) == false) {
            BDBG_ERR(("%s: Failed to parse ID3v2 header for playback_ip %p, hlsSession %p", BSTD_FUNCTION, (void *)playback_ip, (void *)hlsSession));
            return false;
        }
        batom_release(atom);
#define ID3V2_HEADER_LEN 10
        /* ID3 tag header doesn't include the length of the ID3v2 header itself. */
        totalId3v2HeaderSize = id3v2Header.size + ID3V2_HEADER_LEN;
        BDBG_MSG(("%s: ID3v2 id3v2Header: version %#x, size %u total size %zu %s %s %s %s", BSTD_FUNCTION, (unsigned)id3v2Header.version, (unsigned)id3v2Header.size, totalId3v2HeaderSize, id3v2Header.flags.unsynchronisation?"unsynchronisation":"", id3v2Header.flags.extended_header?"extended_id3v2Header":"", id3v2Header.flags.experimental_indicator?"experimental_indicator":"", id3v2Header.flags.footer_present?"footer_present":""));
        /* Get the timestamp from the value of PRIV frame which must be the only frame in the ID3 tag. */
        {
            /*
             * From HLS Spec:
             * Each Packed Audio segment MUST signal the timestamp of its first sample with an ID3 PRIV tag [ID3] at the beginning of the segment.
             * The ID3 PRIV owner identifier MUST be "com.apple.streaming.transportStreamTimestamp".
             * The ID3 payload MUST be a 33-bit MPEG-2 Program Elementary Stream timestamp expressed as a
             * big-endian eight-octet number, with the upper 31 bits set to zero
             */

            /* For now, we just find the right position of the timestamp dword. */
            uint8_t *tsBuffer = (uint8_t *)(segmentBuffer->buffer) + totalId3v2HeaderSize - 4;

            timestamp = tsBuffer[0]<<24 |tsBuffer[1]<<16 |tsBuffer[2]<<8 |tsBuffer[3]<<0 ;
            BDBG_MSG(("timestamp bytes: 0x%x 0x%x 0x%x 0x%x ts=0x%x", tsBuffer[0], tsBuffer[1], tsBuffer[2], tsBuffer[3], timestamp));
        }

        /*
         * Setup the PES header that will be used for all buffers containing 1 audio HLS segment. This happens when
         * an Audio Segment length is > 65K, length of a PES header.
         * Prepare PES header w/ the corret timestamp from the ID3 header and cache it.
         */
        bmedia_pes_info_init(&hlsSession->pesInfo, HLS_PES_AUDIO_ES_ID);
        BMEDIA_PES_SET_PTS(&hlsSession->pesInfo, (timestamp/2)); /* in 45Khz units, the PES timestamp is provided in the 90Khz units. */
        pesPayloadLen = segmentBuffer->bufferDepth - totalId3v2HeaderSize;
        pesHeaderLen = bmedia_pes_header_init(pesHeader, pesPayloadLen, &hlsSession->pesInfo);

        /* Before feeding to the audio decoder, we will remove the ID3 header & replace it w/ the PES header. */
        segmentBuffer->buffer += totalId3v2HeaderSize;  /* Skip past the ID3 header */
        segmentBuffer->bufferDepth -= totalId3v2HeaderSize;
    }
    else {
        /* Not the 1st buffer containing the current HLS Audio segment, so it is the continuation of the ID3 frame. */
        pesPayloadLen = segmentBuffer->bufferDepth;
        hlsSession->pesInfo.pts_valid = false; /* Let decoder interpolate the PTS for the trailing PES packets corresponding to a segment. */
        pesHeaderLen = bmedia_pes_header_init(pesHeader, pesPayloadLen, &hlsSession->pesInfo);
    }

    /* Now prefix the PES header into the segment */
    /* Note: since we leave space for the PES header in the start of each segment, goback that many bytes and then copy the PES header. */
    segmentBuffer->buffer -= pesHeaderLen;
    BKNI_Memcpy(segmentBuffer->buffer, pesHeader, pesHeaderLen);
    segmentBuffer->bufferDepth += pesHeaderLen;
    BDBG_MSG(("%s: buffer=%p cnt %d, pes header len %zu, pes payload len %zu, pts %d, total buffer size %d", BSTD_FUNCTION, (void *)segmentBuffer->buffer, ++cnt, pesHeaderLen, pesPayloadLen, timestamp/2, segmentBuffer->bufferDepth));

    return true;
}

void
B_PlaybackIp_HlsMediaSegmentDownloadThread(
    void *data
    )
{
    int i;
    BERR_Code rc;
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)data;
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;
    HlsSegmentBuffer *segmentBuffer = NULL;
    unsigned networkBandwidth = 0;
    MediaFileSegmentInfo *mediaFileSegmentInfo = NULL;
    int currentMediaSegmentSequenceNumber = -1;
    bool previousMediaSegmentCompletelyDownloaded = true;
    bool serverClosed = true;
    bool gotoTopOfLoop = false;
    int nextSegmentBufferIndex = 0;
#ifdef BITRATE_SWITCH_LATENCY_PROTOTYPE
    NEXUS_PlaybackPosition currentDownloadPosition = 0;
    NEXUS_PlaybackPosition currentPlayPosition = 0;
#endif
    int bytesToReadInSegment = 0;
    bool persistentHttpSession = false;
    bool reOpenSocketConnection = true; /* start from a new socket connection */
    NEXUS_PlaybackPosition downloadedSegmentsDuration = 0;
    bool playlistSwitched = false;
    bool wrapEsWithPes = false;
    unsigned timestampOffset = 0; /* HLS doesn't use transport timestamps. */
    B_PlaybackIpSecurityProtocol currentMediaSegmentSecurityProtocol = B_PlaybackIpSecurityProtocol_None;

    BDBG_MSG(("%s: Started", BSTD_FUNCTION));
    /* start from the 1st segment in the current playlist */
    hlsSession->resetPlaylist = true;
    if (playback_ip->psi.psiValid == true && playback_ip->psi.mpegType == NEXUS_TransportType_eMpeg2Pes) {
        wrapEsWithPes = true;
    }
    BDBG_MSG(("%s:%p: wrapEsWithPes =%d psiValid=%d mpegType=%d", BSTD_FUNCTION, (void *)playback_ip, wrapEsWithPes, playback_ip->psi.psiValid, playback_ip->psi.mpegType));
    while (true) {
        unsigned controlBytesLength = 0;    /* keeps track of any control bytes that get inserted into the stream. */

        gotoTopOfLoop = false;
        BDBG_MSG(("%s:%p:%d At the top of the loop", BSTD_FUNCTION, (void *)playback_ip, playbackIpState(playback_ip)));
        if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
            BDBG_MSG(("%s:%p breaking out of HLS Media Segment Download loop due to state (%d) change", BSTD_FUNCTION, (void *)playback_ip, playbackIpState(playback_ip)));
            break;
        }
        if (hlsSession->hlsPlaybackThreadDone) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s:%p HLS Playback thread is done, so stopping the HLS Segment Download thread", BSTD_FUNCTION, (void *)playback_ip));
#endif
            goto error;
        }
        if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode && hlsSession->seekOperationStarted) {
            BDBG_MSG(("%s: App is trying for a trickmode command, pause this thread", BSTD_FUNCTION));
            BKNI_ResetEvent(hlsSession->segDownloadThreadPauseDoneEvent);
            /* signal the control thread that we have paused and thus it can continue w/ setting up the trickmode operation */
            BKNI_SetEvent(hlsSession->segDownloadThreadPausedEvent);

            /* wait on signal from the control thread to indicate trickmode work is done */
            rc = BKNI_WaitForEvent(hlsSession->segDownloadThreadPauseDoneEvent, HLS_EVENT_TIMEOUT_MSEC*2);
            if (rc == BERR_TIMEOUT || rc!=0) {
                BDBG_ERR(("%s: EVENT %s: failed to receive event from control thread indicating trickmode completion", BSTD_FUNCTION, rc==BERR_TIMEOUT?"Timeout":"Error"));
                goto error;
            }

            /* set this flag so that we dont download anything remaining from the previous segment */
            previousMediaSegmentCompletelyDownloaded = true;
            /* and we set the flag to reOpen the socket connection */
            reOpenSocketConnection = true;
            /* since seek operation will change the next sequence # to download, update the current media sequence # to it */
            currentMediaSegmentSequenceNumber = hlsSession->currentPlaylistFile->currentMediaFileSegment->mediaSequence;
            /* adjust the total duration to the seek position */
            downloadedSegmentsDuration = hlsSession->lastSeekPosition;
            nextSegmentBufferIndex = 0;

            if ( playback_ip->socketState.fd != -1) {
                /* close security context and socket if they were closed because we didn't yet get close event from the server */
                if (playback_ip->securityHandle) {
                    playback_ip->netIo.close(playback_ip->securityHandle);
                    playback_ip->securityHandle = NULL;
                }
                close(playback_ip->socketState.fd);
                playback_ip->socketState.fd = -1;
            }
            BDBG_MSG(("%s: resuming seg download thread after trickmode command, downloadedSegmentsDuration %lu", BSTD_FUNCTION, downloadedSegmentsDuration));
        }

#ifdef BITRATE_SWITCH_LATENCY_PROTOTYPE
        if (B_PlaybackIp_HttpGetCurrentPlaybackPosition(playback_ip, &currentPlayPosition) != B_ERROR_SUCCESS) {
            BDBG_MSG(("%s: Failed to determine the current playback position, setting it to 0\n", BSTD_FUNCTION));
        }
        BDBG_MSG(("current positions: download %d, play %d..........", currentDownloadPosition, currentPlayPosition));
        if (!hlsSession->downloadedAllSegmentsInCurrentPlaylist && currentDownloadPosition && currentPlayPosition && (currentDownloadPosition - (currentPlayPosition) >= 15000)) {
            BDBG_WRN(("downloaded enough, throttle back download thread, current positions: download %d, play %d..........", currentDownloadPosition, currentPlayPosition));
            BKNI_Sleep(500);
            continue;
        }
#endif
        /* determine the next buffer to use for downloading next media segment */
        segmentBuffer = NULL;
        while (!segmentBuffer) {
            if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
                /* user changed the channel, so return */
                BDBG_MSG(("%s:%p breaking out of HLS Download Media Segment Download loop due to state (%d) change", BSTD_FUNCTION, (void *)playback_ip, playback_ip->playback_state));
                goto error;
            }
            if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode || playback_ip->playback_state == B_PlaybackIpState_eEnteringTrickMode) {
                gotoTopOfLoop = true;
                break;
            }
            for (i=0; !segmentBuffer && i<HLS_NUM_SEGMENT_BUFFERS; i++) {
                BKNI_AcquireMutex(hlsSession->segmentBuffer[nextSegmentBufferIndex].lock);
                if (!hlsSession->segmentBuffer[nextSegmentBufferIndex].filled) {
                    segmentBuffer = &hlsSession->segmentBuffer[nextSegmentBufferIndex];
                    segmentBuffer->bufferDepth = 0;
                    segmentBuffer->buffer = wrapEsWithPes? segmentBuffer->bufferOrig+BMEDIA_PES_HEADER_MAX_SIZE : segmentBuffer->bufferOrig; /* When wrapping ES w/ PES, leave space for the PES header. */
                    BDBG_MSG(("%s:%p using download buffer [%d] %p buffer=%p", BSTD_FUNCTION, (void *)playback_ip, nextSegmentBufferIndex, (void *)segmentBuffer, (void *)segmentBuffer->buffer));
                }
                BKNI_ReleaseMutex(hlsSession->segmentBuffer[nextSegmentBufferIndex].lock);
            }

            if (!segmentBuffer) {
                if (hlsSession->hlsPlaybackThreadDone) {
                    BDBG_ERR(("%s: HLS Playback thread is done, so stopping the HLS Download thread", BSTD_FUNCTION));
                    goto error;
                }
                /* wait on signal from playback thread to consume and free up one of the buffers */
                BDBG_MSG(("%s:%p Waiting before the event", BSTD_FUNCTION, (void *)playback_ip));
                rc = BKNI_WaitForEvent(hlsSession->bufferEmptiedEvent, HLS_EVENT_BUFFER_TIMEOUT_MSEC);
                if (rc == BERR_TIMEOUT) {
                    BDBG_MSG(("%s: EVENT timeout: failed to receive event from HLS Playback thread indicating buffer availability, continue waiting", BSTD_FUNCTION));
                    continue;
                } else if (rc!=0) {
                    BDBG_ERR(("%s: failed to wait for event indicating buffer consumption from HLS Playback thread, rc = %d", BSTD_FUNCTION, rc));
                    goto error;
                }
            }
        }
        if (gotoTopOfLoop)
            continue;

        if (previousMediaSegmentCompletelyDownloaded == true) {
            /* now download the next segment */
            BDBG_MSG(("%s:%p:%d before lock", BSTD_FUNCTION, (void *)playback_ip, playback_ip->playback_state));
            BKNI_AcquireMutex(hlsSession->lock);
            segmentBuffer->containsEndOfSegment = true;
            hlsSession->segmentCounter++; /* increment the segment counter. */
            /* now we have a buffer, pick the next media segment to download, it may be from a different playlist file if our n/w b/w has changed */
            networkBandwidth = B_PlaybackIp_HlsGetCurrentBandwidth(&hlsSession->bandwidthContext);
            BDBG_MSG(("%s:%p network bandwidth %u, position %lu", BSTD_FUNCTION, (void *)playback_ip, networkBandwidth, downloadedSegmentsDuration));
            if ((mediaFileSegmentInfo = B_PlaybackIp_HlsGetNextMediaSegmentEntry(playback_ip, hlsSession, hlsSession->resetPlaylist ? -1:currentMediaSegmentSequenceNumber, downloadedSegmentsDuration, networkBandwidth, &playlistSwitched)) == NULL) {
                /* no more segments to download, we keep looping in this thread until playback thread is done feeding and playing all data */
                /* this is done so that if the user wants to re-seek to a previously played position, we can still support it */
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog)
                    BDBG_WRN(("%s:%p:%d No More Media Segment URI left in the Playlist URI %s, we keep looping !!", BSTD_FUNCTION, (void *)playback_ip, playback_ip->playback_state, hlsSession->currentPlaylistFile->uri));
#endif
                hlsSession->downloadedAllSegmentsInCurrentPlaylist = true;
                BKNI_ReleaseMutex(hlsSession->lock);
                BDBG_MSG(("%s:%p:%d after lock", BSTD_FUNCTION, (void *)playback_ip, playback_ip->playback_state));
                BKNI_Sleep(200); /* allow the playlist redownload thread to refetch the playlist and then retry or playback thread to finish playing */
                continue;
            }
            currentMediaSegmentSequenceNumber = mediaFileSegmentInfo->mediaSequence;
            if (playback_ip->speedNumerator >= 0) {
                downloadedSegmentsDuration += mediaFileSegmentInfo->duration;
            }
            else {
                /* rewind case: decrement the downloadedSegmentsDuration by the one segments we are going back from. */
                if (downloadedSegmentsDuration >= mediaFileSegmentInfo->duration) {
                    downloadedSegmentsDuration -= mediaFileSegmentInfo->duration;
                }
                else {
                    downloadedSegmentsDuration = 0;
                }
            }
            BDBG_MSG(("%s:%p seg pos %lu, position %lu", BSTD_FUNCTION, (void *)playback_ip, mediaFileSegmentInfo->duration, downloadedSegmentsDuration));

            /* determine if server is using HTTP persistent connection, we know based on the EXT-X-BYTERANGE tag before a URL */
            /* if it is specified, then this segment is a sub-range of the URI of the media segment and thus we can reuse the */
            /* TCP connection unless server had closed it */
            persistentHttpSession = (mediaFileSegmentInfo->segmentLength) ? 1 : 0;
            if (mediaFileSegmentInfo->protocol == B_PlaybackIpProtocol_eHttps) {
                /* turn off persistent HTTP sessions for SSL protocol */
                /* otherwise, we are getting stuck in the SSL read if remaining server bytes are not mod16 aligned */
                /* having server close the connection forces the server to pad the non-aligned bytes and thus decryption works */
                persistentHttpSession = 0;
                BDBG_MSG(("%s: turn off persistent HTTP sessions for SSL protocol", BSTD_FUNCTION));
            }
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s:%p next media segment: uri %s, seq# %d, prev fd %d, persistentHttpSession %d, reOpenSocketConnection %d", BSTD_FUNCTION, (void *)playback_ip, mediaFileSegmentInfo->absoluteUri, mediaFileSegmentInfo->mediaSequence, playback_ip->socketState.fd, persistentHttpSession, reOpenSocketConnection));
#endif
            if (playback_ip->speedNumerator < 0)
            {
                /*
                 * For rewind, we use DQT (Display Queue Trickmode):
                 * -Before we download & feed of each segment, we feed a BTP specifying # of frames to decode,
                 * -Then, feed a Picture Tag BTP, that contains a number being sequentially incremented per segment,
                 * -Follow that with feeding the segment which may contain 1 or more GOPs.
                 * -After feeding the whole segment, we feed InLineFlush BTP indicating the end of GOP.
                 */

                /* Prepare PictureCount BTP: how many frames decoder should decode in this segment. */
                {
                    unsigned pictureCount;
                    uint8_t *pkt;

                    pkt = (uint8_t *)&segmentBuffer->buffer[0];     /* 1st BTP is placed at the start of the buffer. */
                    if (playback_ip->speedNumerator <= -15) {
                        pictureCount = 3;   /* keeping it as high number as we are only decoding IDR & I frames per segment. */
                    }
                    else {
                        pictureCount = 6;   /* keeping it as high number as we are only decoding IDR & I frames per segment. */
                    }
                    B_PlaybackIp_UtilsBuildPictureOutputCountBtp(playback_ip->psi.videoPid, pictureCount, timestampOffset, pkt);
                    controlBytesLength += BTP_BUFFER_SIZE;
#ifdef BDBG_DEBUG_BUILD
                    if (playback_ip->ipVerboseLog)
                        BDBG_WRN(("%s: hlsSession %p, built PictureCount %d BTP for pid %d at %p", BSTD_FUNCTION, (void *)hlsSession, pictureCount, playback_ip->psi.videoPid, (void *)pkt));
#endif
                }

                /* Prepare PictureTag BTP: unique sequence # corresponding to this segment. */
                {
                    unsigned pictureTag;
                    uint8_t *pkt;

                    pkt = (uint8_t *)&segmentBuffer->buffer[0+BTP_BUFFER_SIZE];     /* 2nd BTP is placed at the start of the buffer. */
                    pictureTag = hlsSession->segmentCounter; /* unique as it is incremented per new segment. */
                    B_PlaybackIp_UtilsBuildPictureTagBtp(playback_ip->psi.videoPid, pictureTag, timestampOffset, pkt);
                    controlBytesLength += BTP_BUFFER_SIZE;
#ifdef BDBG_DEBUG_BUILD
                    if (playback_ip->ipVerboseLog)
                        BDBG_WRN(("%s: hlsSession %p, built PictureTag %d BTP for pid %d at %p", BSTD_FUNCTION, (void *)hlsSession, pictureTag, playback_ip->psi.videoPid, (void *)pkt));
#endif
                }
            }
            /* note time: start */
            B_Time_Get(&hlsSession->lastSegmentDownloadStartTime);
            hlsSession->lastPartialSegmentDownloadTime = 0;
            if (B_PlaybackIp_HlsSetupHttpSessionToServer(playback_ip, mediaFileSegmentInfo, persistentHttpSession, reOpenSocketConnection, &playback_ip->socketState.fd, segmentBuffer->buffer+controlBytesLength, &segmentBuffer->bufferDepth) == false) {
                BDBG_ERR(("%s:%p ERROR: Socket setup or HTTP request/response failed for downloading next Media Segment, skip to next uri", BSTD_FUNCTION, (void *)playback_ip));
                BKNI_ReleaseMutex(hlsSession->lock);
                BDBG_MSG(("%s:%p:%d after lock", BSTD_FUNCTION, (void *)playback_ip, playback_ip->playback_state));
                BKNI_Sleep(10);
                continue;
            }
            BDBG_MSG(("%s:%p:%d after SetupHttpSessionToServer ", BSTD_FUNCTION, (void *)playback_ip, playback_ip->playback_state));
            if (hlsSession->currentPlaylistFile) {
                hlsSession->lastSegmentBitrate = hlsSession->currentPlaylistFile->bandwidth;
                if (hlsSession->lastSegmentUrl) BKNI_Free(hlsSession->lastSegmentUrl);
                if ( (hlsSession->lastSegmentUrl = B_PlaybackIp_UtilsStrdup(hlsSession->currentPlaylistFile->absoluteUri)) == NULL ) {
                    BDBG_ERR(("%s: Failed to allocate %zu bytes of memory for absolute URI ", BSTD_FUNCTION, strlen(hlsSession->currentPlaylistFile->absoluteUri) ));
                    goto error;
                }
            }
            hlsSession->lastSegmentDuration = mediaFileSegmentInfo->duration;
            hlsSession->lastSegmentSequence = (unsigned) mediaFileSegmentInfo->mediaSequence;
            currentMediaSegmentSecurityProtocol = mediaFileSegmentInfo->securityProtocol;
            /* determine how many bytes to download */
            if (mediaFileSegmentInfo->segmentLength > 0) {
                /* we know this segment length, so server is supporting byte-range header. Since a persistent TCP connection is maintained in this case, */
                /* we can't rely either on server closing the socket or contentLength to know how much of segment to download */
                /* Instead, we will use the segmentLength to download only that many bytes from the server */
                bytesToReadInSegment = mediaFileSegmentInfo->segmentLength;
            }
            else if (wrapEsWithPes) {
                /* Since we have to wrap ES w/ PES & each PES packet can atmost be 65K in length, we only read that much Audio Segment data into the Buffer. */
                bytesToReadInSegment = 65530;
            }
            else if (playback_ip->contentLength > 0) {
                /* we try to download that many bytes or until server closes the connection */
                bytesToReadInSegment = (int) playback_ip->contentLength;
            }
            else {
                /* neither we know the segmentSize nor content length, download a max of bufferSize unless server closes the socket before that */
                bytesToReadInSegment = segmentBuffer->bufferSize;
            }
            BKNI_ReleaseMutex(hlsSession->lock);
            BDBG_MSG(("%s:%p:%d after releasing lock", BSTD_FUNCTION, (void *)playback_ip, playback_ip->playback_state));
        }
        else {
            /* previous media segment wasn't completely downloaded due to bytesToReadInSegment being larger than segment buffer */
            BDBG_MSG(("%s:%p previous media segment wasn't completely downloaded (%d bytes remaining) due to segment buffer being smaller than the content length", BSTD_FUNCTION, (void *)playback_ip, bytesToReadInSegment));
            /* note time: start */
            B_Time_Get(&hlsSession->lastSegmentDownloadStartTime);
            /* note the time delta as partialSegmentDownloadTime */
        }

        BDBG_MSG(("%s:%p now download the media segment at buffer offset 0x%p, bytesToReadInSegment %d", BSTD_FUNCTION, (void *)playback_ip, (void *)(segmentBuffer->buffer+controlBytesLength), bytesToReadInSegment));
        /* now download the actual media segment */
        serverClosed = false;
        if (B_PlaybackIp_HlsDownloadMediaSegment(playback_ip, playback_ip->socketState.fd, segmentBuffer->buffer+controlBytesLength, segmentBuffer->bufferSize, bytesToReadInSegment, &segmentBuffer->bufferDepth, &networkBandwidth, &serverClosed) != true) {
            BDBG_ERR(("%s:%p failed to download the current media segment, skip to next media segment", BSTD_FUNCTION, (void *)playback_ip));
            previousMediaSegmentCompletelyDownloaded = true;
            reOpenSocketConnection = true;
            if (playback_ip->securityHandle) {
                playback_ip->netIo.close(playback_ip->securityHandle);
                playback_ip->securityHandle = NULL;
            }
            close(playback_ip->socketState.fd);
            playback_ip->socketState.fd = -1;
            serverClosed = true;
            continue;
        }

        BDBG_MSG(("%s:%p Downloaded current media segment", BSTD_FUNCTION, (void *)playback_ip));
        if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode && hlsSession->seekOperationStarted) {
            /* go back to top as it will have a check there to handle this seek operation properly */
            BDBG_MSG(("%s:%p state %d: Going back to top as seek operation is in progress!", BSTD_FUNCTION, (void *)playback_ip, playback_ip->playback_state));
            continue;
        }

        /* check if we downloaded the complete media segment, if not set a flag to continue downloading in the next iteration */
        if (serverClosed || (bytesToReadInSegment && segmentBuffer->bufferDepth == bytesToReadInSegment)) {
            B_Time lastSegmentDownloadEndTime;
            /* note time: stop & calculate lastSegmentDownloadTime */
            B_Time_Get(&lastSegmentDownloadEndTime);
            hlsSession->lastSegmentDownloadTime = hlsSession->lastPartialSegmentDownloadTime + B_Time_Diff(&lastSegmentDownloadEndTime, &hlsSession->lastSegmentDownloadStartTime);
            hlsSession->lastPartialSegmentDownloadTime = 0;
            BDBG_MSG(("%s:%p downloaded complete media segment bytesDownloaded %d, serverClosed %d, last byte 0x%x, lastSegmentDownloadTime %u", BSTD_FUNCTION, (void *)playback_ip, segmentBuffer->bufferDepth, serverClosed, segmentBuffer->buffer[segmentBuffer->bufferDepth-1], hlsSession->lastSegmentDownloadTime));
#if 1
            if (currentMediaSegmentSecurityProtocol == B_PlaybackIpSecurityProtocol_Aes128)
#else
            if (mediaFileSegmentInfo->securityProtocol == B_PlaybackIpSecurityProtocol_Aes128)
#endif
            {
                /* HLS uses PKCS7 padding. If some media segments are not 16 byte aligned, the server will pad */
                /* from 1 (size %16 == 15) to 16 (size % 16 == 0) bytes of padding. Each byte in PKCS7 padding */
                /* is equal to the # of bytes of padding. So we just take the last byte and subtract that many bytes */
                int i;
                int padBytes = segmentBuffer->buffer[segmentBuffer->bufferDepth-1];
                char *padBuffer = segmentBuffer->buffer + segmentBuffer->bufferDepth-padBytes;
                segmentBuffer->bufferDepth -= padBytes;
                BDBG_MSG(("%s:%p ############## removed %d padBytes, updated segment size %d", BSTD_FUNCTION, (void *)playback_ip, padBytes, segmentBuffer->bufferDepth));
                for (i=0;i < padBytes; i++) {
                    if (padBuffer[i] != padBytes) {
                        BDBG_ERR(("%s:%p PKCS7 padding error, padByte %d, byte[%d] 0x%x, bufferDepth %d", BSTD_FUNCTION, (void *)playback_ip, padBytes, i, padBuffer[i], segmentBuffer->bufferDepth));
                        goto error;
                    }
                }
            }
            previousMediaSegmentCompletelyDownloaded = true;
            if (!persistentHttpSession || serverClosed)
                reOpenSocketConnection = true;
            else
                reOpenSocketConnection = false;
            if (!persistentHttpSession && !wrapEsWithPes) {
                BDBG_MSG(("%s:%p Downloaded the full segment, so close the socket %d", BSTD_FUNCTION, (void *)playback_ip, playback_ip->socketState.fd));
                /* if we are not doing persistent HTTP connections ( e.g. for HTTPS protocol (see previous comment above by searching eHttps)) */
                /* we close security context and socket */
                if (playback_ip->securityHandle) {
                    playback_ip->netIo.close(playback_ip->securityHandle);
                    playback_ip->securityHandle = NULL;
                }
                close(playback_ip->socketState.fd);
                playback_ip->socketState.fd = -1;
            }

            if (playback_ip->speedNumerator < 0)
            {
                /* Prepare InLineFlush BTP to indicate end of segment. */
                {
                    uint8_t *pkt;
                    /* Note: when allocating a segment buffer, we reserve space for these special BTPs which is not accounted in the bufferSize. */
                    pkt = (uint8_t *)segmentBuffer->buffer
                        + controlBytesLength /* initial two BTPs */
                        + segmentBuffer->bufferDepth; /* Segment itself. */
                    B_PlaybackIp_UtilsBuildInlineFlushBtp(playback_ip->psi.videoPid, timestampOffset, pkt);
                    controlBytesLength += BTP_BUFFER_SIZE;
                    segmentBuffer->bufferDepth += controlBytesLength; /* add these extra control bytes into the segment buffer depth. */
#ifdef BDBG_DEBUG_BUILD
                    if (playback_ip->ipVerboseLog)
                        BDBG_WRN(("%s: hlsSession %p, built InLineFlush BTP for pid %d at %p, controlBytesAdded %d, bytesDepth %d", BSTD_FUNCTION, (void *)hlsSession, playback_ip->psi.videoPid, (void *)pkt, controlBytesLength, segmentBuffer->bufferDepth ));
#endif
                }
            }
        }
        else {
            B_Time lastSegmentDownloadEndTime;
            /* note time: stop & calculate lastSegmentDownloadTime */
            B_Time_Get(&lastSegmentDownloadEndTime);
            hlsSession->lastPartialSegmentDownloadTime = B_Time_Diff(&lastSegmentDownloadEndTime, &hlsSession->lastSegmentDownloadStartTime);
            BDBG_MSG(("%s:%p didn't download all bytes (%d) of current media segment: bytesDownloaded %d, lastPartialSegmentDownloadTime %u", BSTD_FUNCTION, (void *)playback_ip, bytesToReadInSegment, segmentBuffer->bufferDepth, hlsSession->lastPartialSegmentDownloadTime));
            previousMediaSegmentCompletelyDownloaded = false;
            reOpenSocketConnection = false;
            bytesToReadInSegment -= segmentBuffer->bufferDepth;
        }

#ifdef TODO
        /* Commenting this logic as it switching codec from AV to Audio only when they are not same (TS vs ES) requires lot more work. */
        /* For now, only TS AV to TS Audio only is supported!! */

        /* if switching URLs, then do media probe again and determine if pids/codecs have changed. If so, AV decoders will need to be restarted */
        if (playlistSwitched)
        {
            B_PlaybackIpPsiInfoHandle newPsi, curPsi;
            if (B_PlaybackIp_HlsMediaProbeStart(playback_ip, &hlsSession->mediaProbe, segmentBuffer) != true) {
                BDBG_ERR(("%s: Media Probe failed during the playlist switch", BSTD_FUNCTION));
                BDBG_ASSERT(NULL);
                goto error;
            }
            newPsi = &hlsSession->mediaProbe.psi;
            curPsi = &playback_ip->psi;
            if (newPsi->mpegType == NEXUS_TransportType_eMpeg2Pes)
                wrapEsWithPes = true;
        }
#endif

        /* for audio ES streams, we need to process the ID3v2 header and convert to PES by wrapping the current segment buffer w/ 1 PES header */
        if (wrapEsWithPes) {
            /* For ES streams, HLS protocol requires each segment to start w/ ID3 header w/ PRIV header */
            /* so far, we haven't found a proper usage of the data contained in this header (some timing info) */
            /* and thus are going to parse and remove ID3 from the stream. In addition, we will package */
            /* each ES segment with PES header along with PTS info so that decoder can provide us that data */
            if (processId3v2HeaderAndConvertToPes(playback_ip, hlsSession, segmentBuffer) == false) {
                BDBG_ERR(("%s: failed to parse and strip ID3v2 header from the start of HLS ES segment", BSTD_FUNCTION));
                goto error;
            }
            if (serverClosed) {
                BDBG_MSG(("%s:%p Downloaded the full segment, so close the socket %d", BSTD_FUNCTION, (void *)playback_ip, playback_ip->socketState.fd));
                /* if we are not doing persistent HTTP connections ( e.g. for HTTPS protocol (see previous comment above by searching eHttps)) */
                /* we close security context and socket */
                if (playback_ip->securityHandle) {
                    playback_ip->netIo.close(playback_ip->securityHandle);
                    playback_ip->securityHandle = NULL;
                }
                close(playback_ip->socketState.fd);
                playback_ip->socketState.fd = -1;
                previousMediaSegmentCompletelyDownloaded = true;
            }
            else {
                BDBG_MSG(("%s:%p Still haven't fully downloaded the current segment, so continue w/ the same segment!, socket=%d", BSTD_FUNCTION, (void *)playback_ip, playback_ip->socketState.fd));
                previousMediaSegmentCompletelyDownloaded = false;
            }
        }

        /* inform HLS playback thread that buffer is filled w/ the media segment */
        BKNI_AcquireMutex(segmentBuffer->lock);
        segmentBuffer->filled = true;
        nextSegmentBufferIndex = (nextSegmentBufferIndex+1) % HLS_NUM_SEGMENT_BUFFERS;
#if 0
        /* TODO: commenting out this code as discontinuity related logic is not yet complete. */
        if (mediaFileSegmentInfo->markedDiscontinuity)
            segmentBuffer->markedDiscontinuity = true;
        else
            segmentBuffer->markedDiscontinuity = false;
#endif
        BKNI_ReleaseMutex(segmentBuffer->lock);
        BKNI_SetEvent(hlsSession->bufferFilledEvent);
#ifdef BITRATE_SWITCH_LATENCY_PROTOTYPE
        if (previousMediaSegmentCompletelyDownloaded == true)
            currentDownloadPosition += mediaFileSegmentInfo->duration;
#endif
        BDBG_MSG(("%s: playback_ip=%p: informed HLS playback thread that buffer %p: depth %d is filled , current network bandwidth %d", BSTD_FUNCTION, (void *)playback_ip, (void *)segmentBuffer, segmentBuffer->bufferDepth, networkBandwidth));
    }
error:
    BDBG_ERR(("%s: playback_ip=%p: Done", BSTD_FUNCTION, (void *)playback_ip));

    hlsSession->hlsSegmentDownloadThreadDone = true;
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s:%p HLS Media Segment Download thread is exiting...", BSTD_FUNCTION, (void *)playback_ip));
#endif
    return;
}

void
B_PlaybackIp_HlsPrintAvStats(B_PlaybackIpHandle playback_ip)
{
    NEXUS_PlaypumpStatus ppStatus;
    NEXUS_VideoDecoderStatus videoStatus;
    NEXUS_AudioDecoderStatus audioStatus;
    NEXUS_Error rc;
    rc = NEXUS_Playpump_GetStatus(playback_ip->nexusHandles.playpump, &ppStatus);
    if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); return;}
    BDBG_WRN(("%s: before playpump fifo size %zu, depth %zu, bytesPlayed %"PRIu64 "", BSTD_FUNCTION, ppStatus.fifoSize, ppStatus.fifoDepth, ppStatus.bytesPlayed));
    if (playback_ip->nexusHandles.videoDecoder) {
        rc = NEXUS_VideoDecoder_GetStatus(playback_ip->nexusHandles.videoDecoder, &videoStatus);
        if (rc) {BDBG_ERR(("NEXUS Error (%d) at %d, returning...\n", rc, __LINE__)); return;}
        BDBG_WRN(("before decode %.4dx%.4d, pts %#x, fifo size %u, depth %u, fullness %d%%\n",
                    videoStatus.source.width, videoStatus.source.height, videoStatus.pts, videoStatus.fifoSize, videoStatus.fifoDepth, videoStatus.fifoSize?(videoStatus.fifoDepth*100)/videoStatus.fifoSize:0));
    }
    if (playback_ip->nexusHandles.primaryAudioDecoder) {
        rc = NEXUS_AudioDecoder_GetStatus(playback_ip->nexusHandles.primaryAudioDecoder, &audioStatus);
        BDBG_WRN(("before audio0            pts %#x, fifo size %u, depth %u, fullness %d%%\n",
                    audioStatus.pts, audioStatus.fifoSize, audioStatus.fifoDepth, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0));
    }
}

void
B_PlaybackIp_HlsPlaylistReDownloadThread(
    void *data
    )
{
    int rc;
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)data;
    HlsSessionState *hlsSession = playback_ip->hlsSessionState;
    PlaylistFileInfo *playlistFileInfo = NULL;
    PlaylistFileInfo *newPlaylistFileInfo = NULL;
    int reloadTimer;
    int playlistDownloadErrCnt = 0;

#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: Started", BSTD_FUNCTION));
#endif
    if (hlsSession->currentPlaylistFile) {
        reloadTimer = hlsSession->currentPlaylistFile->maxMediaSegmentDuration;
    }
    else {
        reloadTimer = 10000; /* 10 seconds. */
    }
    while (true) {
        newPlaylistFileInfo = NULL;
        if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
            BDBG_MSG(("%s: breaking out of HLS Playlist Download loop due to state (%d) change", BSTD_FUNCTION, playback_ip->playback_state));
            break;
        }

        if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode && hlsSession->seekOperationStarted) {
            BDBG_MSG(("%s: App is trying for a trickmode command, pause this thread", BSTD_FUNCTION));
            BKNI_ResetEvent(hlsSession->playlistReDownloadThreadPauseDoneEvent);
            /* signal the control thread that we have paused and thus it can continue w/ setting up the trickmode operation */
            BKNI_SetEvent(hlsSession->playlistReDownloadThreadPausedEvent);
            /* wait on signal from the control thread to indicate trickmode work is done */
            rc = BKNI_WaitForEvent(hlsSession->playlistReDownloadThreadPauseDoneEvent, HLS_EVENT_TIMEOUT_MSEC*2);
            if (rc == BERR_TIMEOUT || rc!=0) {
                BDBG_ERR(("%s: EVENT %s: failed to receive event from control thread indicating trickmode completion", BSTD_FUNCTION, rc==BERR_TIMEOUT?"Timeout":"Error"));
                goto error;
            }
            BDBG_MSG(("%s: resuming Playlist ReDownload loop after trickmode command", BSTD_FUNCTION));
        }

        BDBG_MSG(("%s: playback_ip=%p WaitingForEvent", BSTD_FUNCTION, (void *)playback_ip));
        rc = BKNI_WaitForEvent(hlsSession->reDownloadPlaylistEvent, reloadTimer);
        if (rc!=BERR_SUCCESS && rc != BERR_TIMEOUT) {
            BDBG_ERR(("%s: failed to wait for event indicating playlist reload timer, rc = %d", BSTD_FUNCTION, rc));
            goto error;
        }
        else if (rc != BERR_TIMEOUT) {
            /* It has to be the success case, meaning we got this event. */
            /* Either it came from playback thread asking us to stop or from the SeekHls function on behalf of the control thread. */
            if (hlsSession->hlsPlaybackThreadDone) {
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog)
                    BDBG_WRN(("%s: HLS Playback thread is done, so stopping the HLS Playlist ReDownload thread", BSTD_FUNCTION));
#endif
                goto error;
            }
            else {
                /* Else it must be from the SeekHls(), so lets goback to top to handle the seek/trickmode related logic! */
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog)
                    BDBG_WRN(("%s: Trickmode operation is pending, so pause our work until that is complete!", BSTD_FUNCTION));
#endif
                BKNI_ResetEvent(hlsSession->reDownloadPlaylistEvent);
                continue;
            }
        }
        else {
            /* this it the timeout case, so time is up for refreshing the playlist. */
            /* we just continue below. */
        }
        if (playback_ip->mediaStartTimeNoted == false) {
            BDBG_ERR(("%s: media playback hasn't started for this long, so breaking out", BSTD_FUNCTION));
            break;
        }
        BDBG_MSG(("%s:%p:%d before acquring lock Redownload playlist as %s, rc %d, reloadTimer %d", BSTD_FUNCTION,(void *)playback_ip, playback_ip->playback_state, rc != BERR_TIMEOUT? "we've downloaded all segments in current playlist":"reload timer expired", rc, reloadTimer));

        BKNI_AcquireMutex(hlsSession->lock);
        BDBG_MSG(("%s:%p after acquiring lock", BSTD_FUNCTION, (void *)playback_ip));
        playlistFileInfo = hlsSession->currentPlaylistFile; /* Segment Download Thread can change the playlist due to n/w b/w change */
        if ((newPlaylistFileInfo = B_PlaybackIp_HlsAllocateDownloadAndParsePlaylistFile(playback_ip, hlsSession, playlistFileInfo)) == NULL) {
            ++playlistDownloadErrCnt;
            reloadTimer = 1000;    /* retry playlist download after sec */
            BKNI_ReleaseMutex(hlsSession->lock);
            BDBG_ERR(("%s:%p B_PlaybackIp_HlsAllocateDownloadAndParsePlaylistFile() failed, retry, # of attempts %d, released lock ", BSTD_FUNCTION, (void *)playback_ip, playlistDownloadErrCnt));
            continue;
        }
        BDBG_MSG(("%s:%p after DwnNParse Playlist", BSTD_FUNCTION, (void *)playback_ip));
        playlistDownloadErrCnt = 0;
        if (newPlaylistFileInfo->mediaSegmentBaseSequence != -1) {
            /* seq # of the 1st segment in this playlist is specified, makes it quite easy to determine whether server has updated the playlist or not */
            if (newPlaylistFileInfo->mediaSegmentBaseSequence == playlistFileInfo->mediaSegmentBaseSequence && newPlaylistFileInfo->numMediaSegments == playlistFileInfo->numMediaSegments) {
                /* new playlist file hasn't changed yet, restart the timer and retry it */
                B_PlaybackIp_FreePlaylistInfo(newPlaylistFileInfo);
                newPlaylistFileInfo = NULL;
                if (hlsSession->hlsPlaylistReDownloadThreadDone) {
                    BDBG_ERR(("%s: new playlist has not changed & playlist ReDownload thread has existed, so breaking out...", BSTD_FUNCTION));
                    BKNI_ReleaseMutex(hlsSession->lock);
                    break;
                }
                /* just playlist hasn't changed, try to fetch it faster next time */
                reloadTimer = playlistFileInfo->maxMediaSegmentDuration/2;
                BDBG_MSG(("%s: new playlist has not changed yet, restart the timer by %d msec (1/2 of target duration %lu) and retry it", BSTD_FUNCTION, reloadTimer, playlistFileInfo->maxMediaSegmentDuration));
            }
            else if (newPlaylistFileInfo->mediaSegmentBaseSequence < playlistFileInfo->mediaSegmentBaseSequence && !hlsSession->downloadedAllSegmentsInCurrentPlaylist) {
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog)
                    BDBG_WRN(("%s: even though playlist base seq (new %d, current %d) has wrapped around: ignore it for now we haven't yet downloaed all segments in the current playlist", BSTD_FUNCTION, newPlaylistFileInfo->mediaSegmentBaseSequence, playlistFileInfo->mediaSegmentBaseSequence));
#endif
                B_PlaybackIp_FreePlaylistInfo(newPlaylistFileInfo);
                reloadTimer = 1000; /* retry the playlist every second so that we have the playlist ready to go once media downthread finishes segments from previous thread */
            }
            else {
                /* Either base seq# (pure live) or number of segments (Event type), or both have changed. */
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog)
                    BDBG_WRN(("%s:%p:%d playlist has changed (base seq: cur %d, new %d, # of seg: cur %d, new %d), remove %p playlist and insert %p playlist", BSTD_FUNCTION,
                                (void *)playback_ip, playback_ip->playback_state, playlistFileInfo->mediaSegmentBaseSequence, newPlaylistFileInfo->mediaSegmentBaseSequence,
                                playlistFileInfo->numMediaSegments, newPlaylistFileInfo->numMediaSegments,
                                (void *)playlistFileInfo, (void *)newPlaylistFileInfo));
#endif
                if (hlsSession->downloadedAllSegmentsInCurrentPlaylist && newPlaylistFileInfo->mediaSegmentBaseSequence < playlistFileInfo->mediaSegmentBaseSequence) {
                    /* we have downloaded all segments in the current playlist and new playlist has wrapped around (new base seq < cur base seq) */
                    /* set a flag to indicate starting from the 1st segment of this playlist */
                    BDBG_MSG(("%s:%p playlist base seq has wrapped around: & we've downloaed all segments in the current playlist, so resetting to use new playlist from starting seq # %d, old playlist seq# %d", BSTD_FUNCTION, (void *)playback_ip, newPlaylistFileInfo->mediaSegmentBaseSequence, playlistFileInfo->mediaSegmentBaseSequence));
                    hlsSession->resetPlaylist = true;
                }
                if (newPlaylistFileInfo->mediaSegmentBaseSequence > playlistFileInfo->mediaSegmentBaseSequence) {
                    /* For pure live streams where base seq# change, determine the segmentPositionOffset that should be added when selecting the next segment from the playlist. */
                    unsigned i;
                    unsigned segmentsRemovedCount;
                    MediaFileSegmentInfo *mediaFileSegmentInfo = NULL;

                    segmentsRemovedCount = newPlaylistFileInfo->mediaSegmentBaseSequence - playlistFileInfo->mediaSegmentBaseSequence;
                    /* Now go thru the current playlist and determine the elapsedPosition corresponding to the removed segments. */
                    for (i=0,mediaFileSegmentInfo = BLST_Q_FIRST(&playlistFileInfo->mediaFileSegmentInfoQueueHead);
                            mediaFileSegmentInfo && i++ < segmentsRemovedCount;
                            mediaFileSegmentInfo = BLST_Q_NEXT(mediaFileSegmentInfo, next))
                    {
                        hlsSession->segmentsRemovedDuration += mediaFileSegmentInfo->duration;
                    }
#ifdef BDBG_DEBUG_BUILD
                    if (playback_ip->ipVerboseLog)
                        BDBG_WRN(("%s:%p base seq#: new %d, cur %d, segmentsRemovedCount %u, segmentsRemovedDuration %lu",
                                    BSTD_FUNCTION, (void *)playback_ip, newPlaylistFileInfo->mediaSegmentBaseSequence, playlistFileInfo->mediaSegmentBaseSequence,
                                    segmentsRemovedCount, hlsSession->segmentsRemovedDuration ));
#endif
                }
                /* else is Event type playlist where new segments are added to the end and thus the segment position doesn't change. */
                B_PlaybackIp_HlsReplacePlaylists(hlsSession, playlistFileInfo, newPlaylistFileInfo);
                hlsSession->currentPlaylistFile = newPlaylistFileInfo;
                hlsSession->currentPlaylistFile->playlistHasChanged = true;
                playlistFileInfo = newPlaylistFileInfo;
                reloadTimer = playlistFileInfo->maxMediaSegmentDuration;
                if (playlistFileInfo->bounded) {
                    BDBG_WRN(("%s: updated playlist has END TAG set, stopping the ReDownloading thread as Live Event has reached towards its end", BSTD_FUNCTION));
                    BKNI_ReleaseMutex(hlsSession->lock);
                    goto bounded;
                }
            }
        }
        else {
            /* TODO: add this code */
            BDBG_WRN(("%s: TODO: base media sequence is not explicitly specified in the playlist, need to manually compare the uris to determine if the new playlist has changed", BSTD_FUNCTION));
        }
        BKNI_ReleaseMutex(hlsSession->lock);
        BDBG_MSG(("%s: playback_ip=%p Done", BSTD_FUNCTION, (void *)playback_ip));
    }
    BDBG_MSG(("%s: playback_ip=%p Done", BSTD_FUNCTION, (void *)playback_ip));

bounded:
    if(playlistFileInfo && playlistFileInfo->bounded)
    {
        hlsSession->hlsPlaylistReDownloadThreadDone = true;
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: HLS Playlist ReDownload thread is exiting as playlist is now bounded.", BSTD_FUNCTION));
#endif
        return;
    }

error:
    if (newPlaylistFileInfo)
        B_PlaybackIp_FreePlaylistInfo(newPlaylistFileInfo);
    hlsSession->hlsPlaylistReDownloadThreadDone = true;
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: HLS Playlist ReDownload thread is exiting...", BSTD_FUNCTION));
#endif
    return;
}

static void
B_PlaybackIp_ReadCallback(void *context, int param)
{
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)context;
    BKNI_SetEvent(playback_ip->read_callback_event);
    BSTD_UNUSED(param);
}

extern unsigned B_PlaybackIp_UtilsPlayedCount( B_PlaybackIpHandle playback_ip );
static bool hlsEndOfStream(
    B_PlaybackIpHandle playback_ip
    )
{
    bool status = true;
    unsigned int currentPlayed;

    currentPlayed = B_PlaybackIp_UtilsPlayedCount(playback_ip);
    if (playback_ip->prevPlayed == 0) {
        /* 1st call to this function */
        playback_ip->prevPlayed = currentPlayed;
        status = false;
    }
    else {
        if (currentPlayed == playback_ip->prevPlayed) {
            /* audio PTS or display count isn't changing, so we may be done playing the stream */
            NEXUS_PlaybackPosition currentPosition;

            if (B_PlaybackIp_HttpGetCurrentPlaybackPosition(playback_ip, &currentPosition) != B_ERROR_SUCCESS) {
                BDBG_WRN(("%s: Failed to determine the current playback position", BSTD_FUNCTION));
            }
            else
                BDBG_MSG(("%s: last position %0.3f, duration %d", BSTD_FUNCTION, currentPosition/1000., playback_ip->psi.duration));
            if (/*playback_ip->startSettings.mediaPositionUsingWallClockTime && */ playback_ip->playback_state == B_PlaybackIpState_eTrickMode &&
                    ( (playback_ip->speedNumerator < 0 && currentPosition/1000 > 0) || /* rewind case */
                      (playback_ip->speedNumerator > 1 && playback_ip->speedDenominator == 1 &&
                       currentPosition <= (playback_ip->psi.duration- (playback_ip->hlsSessionState->currentPlaylistFile->maxMediaSegmentDuration*3)))  /* fwd case, we only go upto the last 1 sec. */
                    )) {
                /* even though picture count didn't change from the last time, but the position hasn't yet reached the beginning or end of the stream. */
                /* so we dont yet declare that we are done. This can happen for H264/H265 Codecs where I-frames are sparsely present due to biggers GOPs. */
                status = false;
                BDBG_MSG(("%s: picture count (%d) didin't change, but still haven't reached the beginning or end of stream: currentPosition %0.3f", BSTD_FUNCTION, currentPlayed, currentPosition/1000.));
            }
            else {
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog)
                    BDBG_WRN(("%s: %s isn't changing, so we may be done playing the stream", BSTD_FUNCTION, (playback_ip->nexusHandles.videoDecoder || playback_ip->nexusHandles.simpleVideoDecoder)? "Displaed Picture Count" : "Audio PTS"));
#endif
                /* reached EOF */
                status = true;
            }
        }
        else {
            /* still changing, so not yet reached end of stream */
            playback_ip->prevPlayed = currentPlayed;
            status = false;
        }
    }
    BDBG_MSG(("%s: status %d, currentPlayed %d, prevPlayed %d", BSTD_FUNCTION, status, currentPlayed, playback_ip->prevPlayed));

    return status;
}

bool
setupPlaybackIpSessionForAltAudio(B_PlaybackIpHandle playback_ip, B_PlaybackIpHlsAltAudioRenditionInfo *altAudioRenditionInfo, NEXUS_PlaypumpHandle playpump2)
{
    AltRenditionInfo *altRenditionInfo = NULL;
    char *server = NULL;
    unsigned port;
    char *uri;
    B_PlaybackIpProtocol protocol;
    B_PlaybackIpSessionStartSettings *pStartSettings = NULL;
    B_PlaybackIpSessionStartStatus startStatus;
    B_PlaybackIpHandle hAltAudioPlaybackIp = NULL;

    /*
     * Player wants to play the alternate audio. It must have used this alternate audio PID to create the audio pid channel.
     * We will use this pid to find the matching alternate audio rendition and then use its URL to start another playback_ip
     * session that will feed audio thru the playpump2 handle.
     */
    for (altRenditionInfo = BLST_Q_FIRST(&playback_ip->hlsSessionState->altRenditionInfoQueueHead);
         altRenditionInfo;
         altRenditionInfo = BLST_Q_NEXT(altRenditionInfo, next)
        )
    {
        BDBG_MSG(("%s: current alternateRendition: language=%s URI=%s", BSTD_FUNCTION, altRenditionInfo->language, altRenditionInfo->absoluteUri));
        if (altRenditionInfo->type != HlsAltRenditionType_eAudio) continue;
        if (altRenditionInfo->absoluteUri == NULL) continue;
        if (strncmp(altRenditionInfo->language, altAudioRenditionInfo->language, strlen(altRenditionInfo->language))) continue;
        if (strncmp(altRenditionInfo->groupId, altAudioRenditionInfo->groupId, strlen(altRenditionInfo->groupId))) continue;
        /* this audio entry had the matching groupId & langugage, so this rendition is the one to pick. */
        break;
    }

    if (altRenditionInfo == NULL) {
        BDBG_ERR(("%s: Didn't find the matching alternate audio rendition: language=%s groupId=%s containerType=%d", BSTD_FUNCTION,
                    altAudioRenditionInfo->language, altAudioRenditionInfo->groupId, altAudioRenditionInfo->containerType));
        goto error;
    }
    BDBG_MSG(("%s: alternateRendition: URI=%s", BSTD_FUNCTION, altRenditionInfo->absoluteUri));
    if ((hls_parse_url(&protocol, &server, &port, &uri, altRenditionInfo->absoluteUri) == false) || (protocol != B_PlaybackIpProtocol_eHttp && protocol != B_PlaybackIpProtocol_eHttps)) {
        BDBG_ERR(("Failed to parse sub-fields at %s:%d of URI=%s", BSTD_FUNCTION, __LINE__, altRenditionInfo->absoluteUri));
        goto error;
    }
    BDBG_MSG(("%s:%p calling IpSessionOpen ", BSTD_FUNCTION, (void *)playback_ip));
    hAltAudioPlaybackIp = B_Playback_HlsCreatePlaybackIpSession(playback_ip, server, port, &uri, protocol, playback_ip->openSettings.security.initialSecurityContext);
    if (!hAltAudioPlaybackIp) {
        BDBG_ERR(("%s: Failed to Open/Setup PlaybackIpSession for Alternate Audio", BSTD_FUNCTION));
        goto error;
    }
    {
        B_PlaybackIpSessionSetupSettings setupSettings;
        B_PlaybackIpSessionSetupStatus *pSetupStatus;
        B_PlaybackIpError               pbipStatus;

        B_PlaybackIp_GetDefaultSessionSetupSettings(&setupSettings);
        pSetupStatus = BKNI_Malloc(sizeof *pSetupStatus);
        if (!pSetupStatus) {
            BDBG_ERR(("%s: BKNI_Malloc() failed for B_PlaybackIpSessionSetupStatus", BSTD_FUNCTION));
            goto error;
        }

        memset(pSetupStatus, 0, sizeof(*pSetupStatus));
        setupSettings.u.http.contentTypeHint = altAudioRenditionInfo->containerType;
        setupSettings.u.http.skipPsiParsing = false;
        pbipStatus = B_PlaybackIp_SessionSetup(hAltAudioPlaybackIp, &setupSettings, pSetupStatus);
        BKNI_Free(pSetupStatus);
        if (pbipStatus != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: Failed to setup PlaybackIpSession for Alternate Audio", BSTD_FUNCTION));
            goto error;
        }
    }
    {
        B_PlaybackIpSettings settings;

        B_PlaybackIp_GetSettings(hAltAudioPlaybackIp, &settings);
        settings.useNexusPlaypump = true;
        settings.nexusHandles.playpump = playpump2;
        settings.nexusHandlesValid = true;
        settings.ipMode = B_PlaybackIpClockRecoveryMode_ePull;
        if (B_PlaybackIp_SetSettings(hAltAudioPlaybackIp, &settings) != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: B_PlaybackIp_SetSettings Failed for Alternate Audio", BSTD_FUNCTION));
            goto error;
        }
    }
    {
        pStartSettings = B_Os_Calloc( 1, sizeof(B_PlaybackIpSessionStartSettings));
        if (pStartSettings == NULL) {
            BDBG_ERR(("%s: Failed to allocate %d bytes for B_PlaybackIpSessionStartSettings", BSTD_FUNCTION, (int)sizeof(B_PlaybackIpSessionStartSettings)));
            goto error;
        }
        memset(&startStatus, 0, sizeof(startStatus));
        pStartSettings->nexusHandles.playpump = playpump2;
        pStartSettings->nexusHandles.primaryAudioDecoder = playback_ip->nexusHandles.primaryAudioDecoder;
        pStartSettings->nexusHandles.simpleAudioDecoder = playback_ip->nexusHandles.simpleAudioDecoder;
        pStartSettings->nexusHandlesValid = true;
        pStartSettings->mediaPositionUsingWallClockTime = true;
        pStartSettings->mpegType = altAudioRenditionInfo->containerType;
        pStartSettings->audioOnlyPlayback = true;
        if (B_PlaybackIp_SessionStart(hAltAudioPlaybackIp, pStartSettings, &startStatus) != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: Failed to Start PlaybackIpSession for Alternate Audio", BSTD_FUNCTION));
            B_Os_Free(pStartSettings);
            goto error;
        }
        B_Os_Free(pStartSettings);
    }

    /* Since this PBIP context is successfully started for Alternate audio, add it to the list of such alternate audio contexts. */
    {
        BLST_Q_INSERT_TAIL(&playback_ip->altAudioPlaybackIpListHead, hAltAudioPlaybackIp, altAudioPlaybackIpListEntry);
    }
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog) {
        BDBG_MSG(("%s: Successfully setup the alternate audio rendition: playback_ip=%p hAltAudioPlaybackIp=%p language=%s groupId=%s containerType=%d, plapump2=%p", BSTD_FUNCTION,
                (void *)playback_ip, (void *)hAltAudioPlaybackIp,
                altAudioRenditionInfo->language, altAudioRenditionInfo->groupId,
                altAudioRenditionInfo->containerType, (void *)playpump2));
    }
#endif
    if (server) BKNI_Free(server);
    server=NULL;
    return true;

error:
    if (hAltAudioPlaybackIp) {
        if (B_PlaybackIp_Close(hAltAudioPlaybackIp)) {
            BDBG_ERR(("%s: B_PlaybackIp_Close() Failed", BSTD_FUNCTION));
        }
    }
    if (server) BKNI_Free(server);
    hAltAudioPlaybackIp = NULL;
    return false;
}

void B_PlaybackIp_HlsStopAlternateRendition(B_PlaybackIpHandle playback_ip)
{
    B_PlaybackIpHandle hAltAudioPlaybackIp = NULL;

    if (!playback_ip) return;
    for (hAltAudioPlaybackIp = BLST_Q_FIRST(&playback_ip->altAudioPlaybackIpListHead);
         hAltAudioPlaybackIp;
         hAltAudioPlaybackIp = BLST_Q_FIRST(&playback_ip->altAudioPlaybackIpListHead)
        )
    {
        BLST_Q_REMOVE(&playback_ip->altAudioPlaybackIpListHead, hAltAudioPlaybackIp, altAudioPlaybackIpListEntry);

        BDBG_MSG(("%s:%p: Stopping altRenditionSession: hAltAudioPlaybackIp=%p", BSTD_FUNCTION, (void *)playback_ip, (void *)hAltAudioPlaybackIp));
        if (B_PlaybackIp_SessionStop(hAltAudioPlaybackIp)) {
            BDBG_ERR(("%s: B_PlaybackIp_Stop() Failed", BSTD_FUNCTION));
        }
#if (BCHP_CHIP != 7408)
        NEXUS_StopCallbacks(hAltAudioPlaybackIp->nexusHandles.playpump);
#endif
        BDBG_MSG(("%s:%p: Stopped altRenditionSession: hAltAudioPlaybackIp=%p", BSTD_FUNCTION, (void *)playback_ip, (void *)hAltAudioPlaybackIp));
        if (B_PlaybackIp_Close(hAltAudioPlaybackIp)) {
            BDBG_ERR(("%s: B_PlaybackIp_Close() Failed", BSTD_FUNCTION));
        }
        BDBG_MSG(("%s:%p: Stopped & Closed altRenditionSession!", BSTD_FUNCTION, (void *)playback_ip));
    }
}

B_Error B_PlaybackIp_HlsStartAlternateRendition(B_PlaybackIpHandle playback_ip, B_PlaybackIpHlsAltAudioRenditionInfo *altAudioRenditionInfo)
{
    if (setupPlaybackIpSessionForAltAudio(playback_ip, altAudioRenditionInfo, playback_ip->nexusHandles.playpump2) != true) {
        BDBG_ERR(("%s:%p Failed to setupPlaybackIpSessionForAltAudio!", BSTD_FUNCTION, (void *)playback_ip));
        return (B_ERROR_PROTO);
    }
    BDBG_MSG(("%s:%p Started!", BSTD_FUNCTION, (void *)playback_ip));
    return (B_ERROR_SUCCESS);
}

extern void B_PlaybackIp_HttpDestroyFilePlayHandle(B_PlaybackIpHandle playback_ip);
#define HTTP_PLAYPUMP_BUF_SIZE (19200)
void
B_PlaybackIp_HlsPlaybackThread(
    void *data
    )
{
    int i;
    int bytesToCopy;
    B_ThreadSettings settingsThread;
    B_PlaybackIpHandle playback_ip;
    size_t bytesFed;
    ssize_t rc = -1;
    HlsSessionState *hlsSession;
    HlsSegmentBuffer *segmentBuffer;
    NEXUS_PlaypumpSettings nSettings;
    PlaylistFileInfo *playlistFileInfo;
    static int fileNameSuffix = 0;
    char recordFileName[32];
    FILE *fclear = NULL;
    bool gotoTopOfLoop = false;
    int nextSegmentBufferIndex = 0;
    char *enableSegmentedRecording = NULL;
    int segmentNumber = 0;
    int myFileNameSuffix = 0;

    playback_ip = (B_PlaybackIpHandle)data;
    hlsSession = playback_ip->hlsSessionState;
    playlistFileInfo = hlsSession->currentPlaylistFile;

    if (playback_ip->settings.networkTimeout) {
        playback_ip->networkTimeout = playback_ip->settings.networkTimeout;
    }
    else {
        playback_ip->networkTimeout = HTTP_SELECT_TIMEOUT/20;
        playback_ip->settings.networkTimeout = playback_ip->networkTimeout;
    }
    BDBG_MSG(("%s:%p Starting (n/w timeout %d secs)", BSTD_FUNCTION, (void *)playback_ip, playback_ip->networkTimeout));

    /* close the previous security handle and socket used for downloading the 1st media segment during media probe */
    if (playback_ip->securityHandle) {
        playback_ip->netIo.close(playback_ip->securityHandle);
        playback_ip->securityHandle = NULL;
    }
    close(playback_ip->socketState.fd);
    playback_ip->socketState.fd = -1;

    /* Free up the HTTP cache as it is not used for HLS playback */
    B_PlaybackIp_HttpDestroyFilePlayHandle(playback_ip);

    /* And instead allocate segment download buffers */
    if (B_PlaybackIp_HlsSegmentBufferCreate(playback_ip) < 0) {
        goto error;
    }

    if (playback_ip->startSettings.startAlternateAudio) {
        int i;
        /* coverity[stack_use_local_overflow] */
        /* coverity[stack_use_overflow] */
        if (setupPlaybackIpSessionForAltAudio(playback_ip, &playback_ip->startSettings.alternateAudio, playback_ip->startSettings.nexusHandles.playpump2) != true) {
            BDBG_ERR(("%s: Failed to setupPlaybackIpSessionForAltAudio!", BSTD_FUNCTION));
            goto error;
        }
        for (i=0; i < playback_ip->startSettings.additionalAltAudioRenditionInfoCount; i++)
        {
            /* coverity[stack_use_local_overflow] */
            /* coverity[stack_use_overflow] */
            if (setupPlaybackIpSessionForAltAudio(playback_ip, &playback_ip->startSettings.additionalAltAudioInfo[i], playback_ip->startSettings.additionalAltAudioInfo[i].hPlaypump) != true) {
                BDBG_ERR(("%s: Failed to additional setupPlaybackIpSessionForAltAudio, i = %d!", BSTD_FUNCTION, i));
                goto error;
            }
        }
    }
#if 0
    if (B_PlaybackIp_HlsMediaProbeCreate(playback_ip, &hlsSession->mediaProbe) != 0) {
        BDBG_ERR(("%s: Failed to create the Media Probe context for HLS sessions ", BSTD_FUNCTION));
        goto error;
    }
#endif

    /* start a thread to download the next media file segment into the read buffer */
    B_Thread_GetDefaultSettings(&settingsThread);
    hlsSession->hlsSegmentDownloadThread = B_Thread_Create("HlsMediaSegmentDownloadThread", B_PlaybackIp_HlsMediaSegmentDownloadThread, (void *)playback_ip, &settingsThread);
    if (NULL == hlsSession->hlsSegmentDownloadThread) {
        BDBG_ERR(("%s: Failed to create the %s thread for HLS protocol\n", BSTD_FUNCTION, "HlsMediaSegmentDownloadThread"));
        goto error;
    }
    BDBG_MSG(("%s: Created the %s for HLS protocol", BSTD_FUNCTION, "HlsMediaSegmentDownloadThread"));

    if (playback_ip->nexusHandles.playpump) {
        NEXUS_Playpump_GetSettings(playback_ip->nexusHandles.playpump, &nSettings);
        /* TODO: understand this */
        nSettings.dataCallback.callback = B_PlaybackIp_ReadCallback;
        nSettings.dataCallback.context = playback_ip;
        if (NEXUS_Playpump_SetSettings(playback_ip->nexusHandles.playpump, &nSettings)) {
            BDBG_ERR(("%s:%d Nexus Error: %zd\n", BSTD_FUNCTION, __LINE__, rc));
            goto error;
        }
    }
    else {
        BDBG_ERR(("%s: playback_ip->nexusHandles.playpump is NULL", BSTD_FUNCTION));
        goto error;
    }

    if (playback_ip->enableRecording) {
        enableSegmentedRecording = getenv("enableSegmentedRecording");
        myFileNameSuffix = fileNameSuffix++;
        memset(recordFileName, 0, sizeof(recordFileName));
        snprintf(recordFileName, sizeof(recordFileName)-1, "./videos/hls_rec%d_%d.ts", myFileNameSuffix, segmentNumber);
        fclear = fopen(recordFileName, "w+b");
    }
    if (B_PlaybackIp_UtilsWaitForPlaypumpDecoderSetup(playback_ip))
        goto error;

    if (playlistFileInfo->bounded == false) {
        /* stream is unbounded, meaning it is live stream, we need to start a thread to periodically download the updated playlist */
        B_Thread_GetDefaultSettings(&settingsThread);
        hlsSession->hlsPlaylistReDownloadThread = B_Thread_Create("HlsPlaylistReDownloadThread", B_PlaybackIp_HlsPlaylistReDownloadThread, (void *)playback_ip, &settingsThread);
        if (NULL == hlsSession->hlsPlaylistReDownloadThread) {
            BDBG_ERR(("%s: Failed to create the %s thread for HLS protocol\n", BSTD_FUNCTION, "HlsPlaylistReDownloadThread"));
            goto error;
        }
        BDBG_MSG(("%s: Created %s for HLS protocol", BSTD_FUNCTION, "HlsPlaylistReDownloadThread"));

        /* for unbounded streams, HLS spec recommends that client should not start the normal playback */
        /* if playlist total duration is less than 3 target durations. This is to avoid stalls due n/w b/w fluctuations & latency to server */
#if 0
        /* commenting this out as it is causing large initial latency while playing from wowza HLS server. This server only had 3 segments in the */
        /* initial playlist whose total duration happened to be < 3*target duration. Thus, this loop of 20sec seems to be large and wasteful */
        while (playlistFileInfo->totalDuration < 3*playlistFileInfo->maxMediaSegmentDuration) {
            /* TODO: double check this logic whether totalDuration is getting updated at the correct play a/f media seg is downloaded */
            int count = 0;
            if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
                /* user changed the channel, so return */
                BDBG_MSG(("%s: breaking out of HLS Playback loop due to state (%d) change", BSTD_FUNCTION, playback_ip->playback_state));
                break;
            }
            if (count++ > 20) {
                BDBG_ERR(("%s: Unbound stream didn't update in over 20 sec, aborting: total current duration %d is less than 3 times max segment duration %d", BSTD_FUNCTION, playlistFileInfo->totalDuration, playlistFileInfo->maxMediaSegmentDuration));
                break;
            }
            BDBG_MSG(("%s: Unbound stream: total duration %d is less than 3 times max segment duration %d, wait to cross this threshold", BSTD_FUNCTION, playlistFileInfo->totalDuration, playlistFileInfo->maxMediaSegmentDuration));
            /* sleep and retry, hopefully reDownload thread gets a chance to fetch the new playlist w/ more than 3*seg duration */
            BKNI_Sleep(1000);
        }
#endif
    }

    /* main loop */
    BDBG_MSG(("%s: Read from segment buffer and feed to playpump", BSTD_FUNCTION));
    while (true) {
        if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
            /* user changed the channel, so return */
            BDBG_MSG(("%s: breaking out of HLS Playback loop due to state (%d) change", BSTD_FUNCTION, playback_ip->playback_state));
            break;
        }
        gotoTopOfLoop = false;
        if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode && hlsSession->seekOperationStarted) {
            BDBG_MSG(("%s: App is trying for a trickmode command, pause this thread", BSTD_FUNCTION));
            BKNI_ResetEvent(hlsSession->playbackThreadPauseDoneEvent);
            /* signal the control thread that we have paused and thus it can continue w/ setting up the trickmode operation */
            BKNI_SetEvent(hlsSession->playbackThreadPausedEvent);
            /* wait on signal from the control thread to indicate trickmode work is done */
            rc = BKNI_WaitForEvent(hlsSession->playbackThreadPauseDoneEvent, HLS_EVENT_TIMEOUT_MSEC*2);
            if (rc == BERR_TIMEOUT || rc!=0) {
                BDBG_ERR(("%s: EVENT %s: failed to receive event from control thread indicating trickmode completion", BSTD_FUNCTION, rc==BERR_TIMEOUT?"Timeout":"Error"));
                goto error;
            }
            nextSegmentBufferIndex = 0;
            BDBG_MSG(("%s: resuming HLS Playback loop after trickmode command", BSTD_FUNCTION));
        }

        /* determine the segment buffer to read data from */
        segmentBuffer = NULL;
        while (!segmentBuffer) {
            if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
                /* user changed the channel, so return */
                BDBG_MSG(("%s: breaking out of HLS Playback loop due to state (%d) change", BSTD_FUNCTION, playback_ip->playback_state));
                goto error;
            }
            if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode || playback_ip->playback_state == B_PlaybackIpState_eEnteringTrickMode) {
                gotoTopOfLoop = true;
                break;
            }
            for (i=0; !segmentBuffer && i<HLS_NUM_SEGMENT_BUFFERS; i++) {
                BKNI_AcquireMutex(hlsSession->segmentBuffer[nextSegmentBufferIndex].lock);
                if (hlsSession->segmentBuffer[nextSegmentBufferIndex].filled) {
                    segmentBuffer = &hlsSession->segmentBuffer[nextSegmentBufferIndex];
                    BDBG_MSG(("%s: buffer[%d] %p is filled up", BSTD_FUNCTION, nextSegmentBufferIndex, (void *)segmentBuffer));
                }
                else
                    BDBG_MSG(("%s: still no filled buffer, wait for signal from download thread, nextSegmentBufferIndex %d", BSTD_FUNCTION, nextSegmentBufferIndex));
                BKNI_ReleaseMutex(hlsSession->segmentBuffer[nextSegmentBufferIndex].lock);
            }

            if (!segmentBuffer) {
                if (hlsSession->downloadedAllSegmentsInCurrentPlaylist &&
                        (playback_ip->playback_state == B_PlaybackIpState_ePlaying || playback_ip->playback_state == B_PlaybackIpState_eTrickMode)) {
                    BDBG_MSG(("%s: No more segments available, check if we reaching the end (normal or fwd) or beginning (rew) of the stream, state %d", BSTD_FUNCTION, playback_ip->playback_state));
                    if (hlsEndOfStream(playback_ip) == true) {
                        if ( playback_ip->playback_state == B_PlaybackIpState_eTrickMode) {
                            if ( playback_ip->speedNumerator < 0 && playback_ip->openSettings.eventCallback )
                            {
                                /* Rewind case w/ callback defined: send begin of stream event & wait for app to resume or stop the playback. */
#ifdef BDBG_DEBUG_BUILD
                                if (playback_ip->ipVerboseLog)
                                    BDBG_WRN(("%s: Rewind case & eventCallback defined: Reached BeginOfStream, wait for App to Stop or Resume Play: (state %d), total fed %"PRId64 "", BSTD_FUNCTION, playback_ip->playback_state, playback_ip->totalConsumed));
#endif
                                if (!playback_ip->beginningOfStreamCallbackIssued) {
                                    playback_ip->openSettings.eventCallback(playback_ip->openSettings.appCtx, B_PlaybackIpEvent_eClientBeginOfStream);
                                    playback_ip->beginningOfStreamCallbackIssued = true;
                                }
                                playback_ip->serverClosed = true;
                                /* rewind case, dont set the mediaEndTime flag. */
                                playback_ip->mediaEndTimeNoted = false;
                                BKNI_Sleep(200);
                                gotoTopOfLoop = true;
                                break;
                            }
                            else if (playback_ip->speedNumerator > 1 && playback_ip->openSettings.eventCallback && !B_PlaybackIp_HlsBoundedStream(playback_ip))
                            {
                                /* Fwd case w/ callback defined: send end of currentBuffer event & wait for app to resume or stop the playback. */
#ifdef BDBG_DEBUG_BUILD
                                if (playback_ip->ipVerboseLog)
                                    BDBG_WRN(("%s: Fwd case, eventCallback defined & Event type playlist w/ not EOF yet: send endOfBuffer event, wait for App to Stop or Resume Play: (state %d), total fed %"PRId64 "", BSTD_FUNCTION, playback_ip->playback_state, playback_ip->totalConsumed));
#endif
                                if (!playback_ip->endOfClientBufferCallbackIssued) {
                                    playback_ip->openSettings.eventCallback(playback_ip->openSettings.appCtx, B_PlaybackIpEvent_eClientEndofSegmentsReached);
                                    playback_ip->endOfClientBufferCallbackIssued = true;
                                }
                                playback_ip->serverClosed = true;
                                /* fwd case, dont set the mediaEndTime flag. */
                                playback_ip->mediaEndTimeNoted = false;
                                BKNI_Sleep(200);
                                gotoTopOfLoop = true;
                                break;
                            }
                            else {
                                /* trickmode with no callback defined case: in either case, we treat this as end-of-stream, so we are done! */
#ifdef BDBG_DEBUG_BUILD
                                if (playback_ip->ipVerboseLog)
                                    BDBG_WRN(("%s: Fwd case or Rewind case w/ no eventCallback defined (Reached BeginOfStream): We are done w/ playback!! (state %d), total fed %"PRId64 "", BSTD_FUNCTION, playback_ip->playback_state, playback_ip->totalConsumed));
#endif
                                B_PlaybackIp_HttpGetCurrentPlaybackPosition(playback_ip, &playback_ip->lastPosition);
                                playback_ip->mediaEndTimeNoted = true;
                                playback_ip->serverClosed = true;
                                goto error;
                            }
                        }
                        else if (playback_ip->playback_state == B_PlaybackIpState_ePlaying && B_PlaybackIp_HlsBoundedStream(playback_ip)) {
                            /* We are done if we are playing bounded streams in the normal playing state. */
#ifdef BDBG_DEBUG_BUILD
                            if (playback_ip->ipVerboseLog)
                                BDBG_WRN(("%s: Normal Play State for Bounded Streamer: Reached EndOfStream: We are done w/ playback!! (state %d), total fed %"PRId64 "", BSTD_FUNCTION, playback_ip->playback_state, playback_ip->totalConsumed));
#endif
                            B_PlaybackIp_HttpGetCurrentPlaybackPosition(playback_ip, &playback_ip->lastPosition);
                            playback_ip->mediaEndTimeNoted = true;
                            playback_ip->serverClosed = true;
                            goto error;
                        }
                    }
                    /* Above logic will break or goto error if it is gets end of stream in the correct state & playing mode. */
                    /* Otherwise, we keep waiting for new segments to show up from the download thread. */
                    {
#ifdef BDBG_DEBUG_BUILD
                        if (playback_ip->ipVerboseLog)
                            BDBG_WRN(("%s: Continue waiting to re-reading from socket incase it becomes valid again by a seek or rewind trickplay", BSTD_FUNCTION));
#endif
                        BKNI_Sleep(200);
                    }
                }
                /* wait on signal from HLS download thread to fill up one of the buffers */
                BDBG_MSG(("%s: wait %d msec for a signal from download thread", BSTD_FUNCTION, HLS_EVENT_BUFFER_TIMEOUT_MSEC));
                rc = BKNI_WaitForEvent(hlsSession->bufferFilledEvent, HLS_EVENT_BUFFER_TIMEOUT_MSEC);
                if (rc == BERR_TIMEOUT) {
                    BDBG_MSG(("%s: EVENT timeout: failed to receive event from HLS Download thread indicating buffer availability, continue waiting", BSTD_FUNCTION));
                    continue;
                } else if (rc!=0) {
                    BDBG_ERR(("%s: failed to wait for event indicating buffer availability from HLS Download thread, rc = %zd", BSTD_FUNCTION, rc));
                    goto error;
                }
                BDBG_MSG(("%s: got a signal from download thread, rc %zd", BSTD_FUNCTION, rc));
            }
        }
        if (gotoTopOfLoop)
            continue;
        BDBG_MSG(("%s: Read from segment buffer %p", BSTD_FUNCTION, (void *)segmentBuffer));

        bytesFed = 0;
        BDBG_MSG(("%s: Feed %d bytes to Playpump, discontinuity flag %d", BSTD_FUNCTION, segmentBuffer->bufferDepth, segmentBuffer->markedDiscontinuity));

#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog) {
            if (segmentBuffer->markedDiscontinuity) {
                BDBG_WRN(("%s: TODO: this segment has marked discontinuity: need to check what can change & accordingly modify the av pipeline!!", BSTD_FUNCTION));
            }
        }
#endif

        while (segmentBuffer->bufferDepth) {
            if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
                /* user changed the channel, so return */
                BDBG_MSG(("%s: breaking out of HLS Playback loop due to state (%d) change", BSTD_FUNCTION, playback_ip->playback_state));
                goto error;
            }
            if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode || playback_ip->playback_state == B_PlaybackIpState_eEnteringTrickMode) {
                gotoTopOfLoop = true;
                break;
            }
            BDBG_MSG_FLOW(("%s: Get playpump buffer to read data into", BSTD_FUNCTION));
            /* get an adequately sized buffer from the playpump */
            if (B_PlaybackIp_UtilsGetPlaypumpBuffer(playback_ip, HTTP_PLAYPUMP_BUF_SIZE) < 0) {
                BDBG_MSG(("%s: Failed to get buffer from playpump, breaking out of HTTP loop", BSTD_FUNCTION));
                gotoTopOfLoop = true;
                break;
            }
            BDBG_MSG_FLOW(("%s: Got playpump buffer to read more data into", BSTD_FUNCTION));

            /* determine how much data can be copied */
            bytesToCopy = segmentBuffer->bufferDepth > HTTP_PLAYPUMP_BUF_SIZE ? HTTP_PLAYPUMP_BUF_SIZE:segmentBuffer->bufferDepth;
            memcpy(playback_ip->buffer, segmentBuffer->buffer+bytesFed, bytesToCopy);

            BDBG_MSG_FLOW(("%s: copied %d bytes from segment buffer into playpump buffer", BSTD_FUNCTION, bytesToCopy));

            /* write data to file */
            if (playback_ip->enableRecording && fclear) {
                fwrite(playback_ip->buffer, 1, bytesToCopy, fclear);
            }

            /* now feed appropriate data it to the playpump */
            if (NEXUS_Playpump_ReadComplete(playback_ip->nexusHandles.playpump, 0, bytesToCopy)) {
                BDBG_ERR(("%s: NEXUS_Playpump_ReadComplete failed, continuing...", BSTD_FUNCTION));
                continue;
            }
            bytesFed += bytesToCopy;
            playback_ip->totalConsumed += bytesToCopy;
            segmentBuffer->bufferDepth -= bytesToCopy;
            BDBG_MSG_FLOW(("%s: Fed %zu bytes to Playpump, total fed %d, remaining %d\n", BSTD_FUNCTION, bytesToCopy, bytesFed, segmentBuffer->bufferDepth));
        }

        if (fclear && playback_ip->enableRecording && enableSegmentedRecording && segmentBuffer->containsEndOfSegment) {
            fflush(fclear);
            fclose(fclear);
            memset(recordFileName, 0, sizeof(recordFileName));
            snprintf(recordFileName, sizeof(recordFileName)-1, "./videos/hls_rec%d_%d.ts", myFileNameSuffix, ++segmentNumber);
            fclear = fopen(recordFileName, "w+b");
        }

        if (gotoTopOfLoop)
            continue;
        BDBG_MSG(("%s: playback_ip=%p: Finished feeding %zu bytes to Playpump", BSTD_FUNCTION, (void *)playback_ip, bytesFed));

        /* inform HLS Download thread that buffer is emptied and fed to the playback h/w */
        BKNI_AcquireMutex(segmentBuffer->lock);
        segmentBuffer->filled = false;
        nextSegmentBufferIndex = (nextSegmentBufferIndex+1) % HLS_NUM_SEGMENT_BUFFERS;
        BKNI_ReleaseMutex(segmentBuffer->lock);
        BKNI_SetEvent(hlsSession->bufferEmptiedEvent);
    }
    BDBG_MSG(("%s: Done", BSTD_FUNCTION));

error:
    B_PlaybackIp_HlsStopAlternateRendition(playback_ip);
    hlsSession->hlsPlaybackThreadDone = true;

    /* Wait for HLS Media & Playlist Download threads to finish as well. */
    {
        static int count = 0, maxCount, sleepIntervalInMsec = 100;

        maxCount = (2*playback_ip->networkTimeout*1000) / sleepIntervalInMsec;
        while (hlsSession->hlsSegmentDownloadThread && !hlsSession->hlsSegmentDownloadThreadDone) {
            if (count++ > maxCount) {
                BDBG_ERR(("%s:%p Failed to wait for HLS Media Segment Download thread to finish for over %d attempts in %d duration", BSTD_FUNCTION, (void *)playback_ip, count, sleepIntervalInMsec));
                break;
            }
            BDBG_WRN(("%s:%p: HLS Playback thread ran into some error, wait for Download thread to finish: cur waitAttempts=%d, maxCount=%d", BSTD_FUNCTION, (void *)playback_ip, count, maxCount));
            BKNI_Sleep(sleepIntervalInMsec);
        }
        if (hlsSession->hlsPlaylistReDownloadThread && !hlsSession->hlsPlaylistReDownloadThreadDone) {
            count = 0;
            BKNI_SetEvent(hlsSession->reDownloadPlaylistEvent);
            while (!hlsSession->hlsPlaylistReDownloadThreadDone) {
                if (count++ > maxCount) {
                    BDBG_ERR(("%s: Failed to wait for HLS Playlist ReDownload thread to finish for over %d attempts", BSTD_FUNCTION, count));
                    break;
                }
                BDBG_WRN(("%s:%p: HLS Playback thread ran into some error, wait for Playlist Download thread to finish: cur waitAttempts=%d, maxCount=%d", BSTD_FUNCTION, (void *)playback_ip, count, maxCount));
                BKNI_Sleep(sleepIntervalInMsec);
            }
        }
    }
#if 0
    B_PlaybackIp_HlsMediaProbeDestroy(&hlsSession->mediaProbe);
#endif
    if (playback_ip->enableRecording && fclear) {
        fflush(fclear);
        fclose(fclear);
    }
    if (hlsSession->hlsSegmentDownloadThread) {
        B_Thread_Destroy(hlsSession->hlsSegmentDownloadThread);
    }

    if (hlsSession->hlsPlaylistReDownloadThread) {
        B_Thread_Destroy(hlsSession->hlsPlaylistReDownloadThread);
    }

    if (playback_ip->openSettings.eventCallback &&
            playback_ip->playback_state != B_PlaybackIpState_eStopping &&
            playback_ip->playback_state != B_PlaybackIpState_eStopped)
    {
        B_PlaybackIpEventIds eventId;
        if (playback_ip->serverClosed)
            eventId = B_PlaybackIpEvent_eServerEndofStreamReached;
        else
            eventId = B_PlaybackIpEvent_eErrorDuringStreamPlayback;
        eventId = B_PlaybackIpEvent_eServerEndofStreamReached;
        playback_ip->openSettings.eventCallback(playback_ip->openSettings.appCtx, eventId);
    }
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s:%p HLS Playback thread is exiting...", BSTD_FUNCTION, (void *)playback_ip));
#endif
    BKNI_SetEvent(playback_ip->playback_halt_event);
}
