/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#if defined(LINUX) || defined(__vxworks)

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/sockios.h>

#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_utils.h"
#ifdef B_HAS_DTCP_IP
#include "b_dtcp_constants.h"
#endif
#ifdef B_HAS_HTTP_MP4_SUPPORT
#include "bmedia_probe.h"
#include "bmp4_probe.h"
#include "bmpeg2ts_probe.h"
#endif
#include "bhevc_video_probe.h"
#include "bfile_buffered.h"
#include "b_playback_ip_psi.h"
#include "bmpeg2ts_psi_probe.h"

#if 0
#define BDBG_MSG_FLOW(x)  BDBG_WRN (x) ;
#else
#define BDBG_MSG_FLOW(x)
#endif

BDBG_MODULE(b_playback_ip_http);
BDBG_FILE_MODULE(b_playback_ip_http_cache);
BDBG_FILE_MODULE(b_playback_ip_http_sockets);

#define PRINT_CACHE_MSG(bdbg_args)   BDBG_MODULE_MSG(b_playback_ip_http_cache,   bdbg_args)
#define PRINT_SOCKET_MSG(bdbg_args)  BDBG_MODULE_MSG(b_playback_ip_http_sockets, bdbg_args)

#define TMP_STRING_SIZE 512
#define DLNA_MAX_HTTP_RESP_SIZE (128*1024)  /* DLNA 1.5 guideline: 7.4.47 */

int B_PlaybackIp_ClearSessionOpen( B_PlaybackIpSessionOpenSettings *, int , B_PlaybackIpSecurityOpenOutputParams *);
void B_PlaybackIp_ClearSessionClose(void *);
#ifdef B_HAS_SSL
int B_PlaybackIp_SslSessionOpen( B_PlaybackIpSessionOpenSettings *, int , B_PlaybackIpSecurityOpenOutputParams *);
int B_PlaybackIp_SslCloneSessionOpen(int fd, void *securityHandle, void **newSecurityHandle);
#endif
#ifdef B_HAS_DTCP_IP
int B_PlaybackIp_DtcpIpSessionOpen( B_PlaybackIpSessionOpenSettings *, int , B_PlaybackIpSecurityOpenOutputParams *);
int B_PlaybackIp_DtcpIpCloneSessionOpen(int fd, void *securityHandle, void **newSecurityHandle);
int B_PlaybackIp_DtcpIpDecryptionEnable(void *securityHandle, char *initialPayload, int initialPayloadLength);
int B_PlaybackIp_DtcpIpDecryptionDisable(void *securityHandle);
#endif
#ifdef B_HAS_RAD_EA
int B_PlaybackIp_RadEaSessionOpen( B_PlaybackIpSessionOpenSettings *, int , B_PlaybackIpSecurityOpenOutputParams *securityOpenOutputParams);
int B_PlaybackIp_RadEaCloneSessionOpen(int fd, void *securityHandle, void **newSecurityHandle);
int B_PlaybackIp_RadEaDecryptionEnable(void *securityHandle, char *initialPayload, int initialPayloadLength);
#endif
#ifdef B_HAS_HTTP_AES_SUPPORT
int B_PlaybackIp_AesSessionOpen(B_PlaybackIpSessionOpenSettings *, int , B_PlaybackIpSecurityOpenOutputParams *);
#endif
extern void B_PlaybackIp_HttpSetDefaultTrickModeSettings(B_PlaybackIpTrickModesSettings *ipTrickModeSettings);
extern B_PlaybackIpError http_send_time_seek_request(B_PlaybackIpHandle playback_ip);
int B_PlaybackIp_HlsSessionSetup(B_PlaybackIpHandle playback_ip, char *http_hdr);
void B_PlaybackIp_HlsSessionDestroy(B_PlaybackIpHandle playback_ip);
int B_PlaybackIp_MpegDashSessionSetup(B_PlaybackIpHandle playback_ip, char *http_hdr);
void B_PlaybackIp_MpegDashSessionStop(B_PlaybackIpHandle playback_ip);
void B_PlaybackIp_MpegDashSessionDestroy(B_PlaybackIpHandle playback_ip);
B_PlaybackIpError B_PlaybackIp_MpegDashSessionStart(B_PlaybackIpHandle playback_ip,B_PlaybackIpSessionStartSettings *startSettings,B_PlaybackIpSessionStartStatus *startStatus);

char *
find_base_uri(char *curUrl)
{
    char *mediaSegmentBaseUri, *tmp;
    char *endPtr = NULL;

    mediaSegmentBaseUri = curUrl;
    tmp = curUrl;
    while ((tmp = strstr(tmp, "/")) != NULL) {
        endPtr = tmp;
        tmp += 1; /* move past "/" char */
    }
    if (endPtr) {
        *(endPtr+1) = '\0'; /* null terminate mediaSegmentBaseUri string */
        mediaSegmentBaseUri = curUrl;
    }
    else {
        /* current url itself is relative, so replace it w/ the new one */
        mediaSegmentBaseUri = NULL;
    }
    return mediaSegmentBaseUri;
}

bool
http_absolute_uri(char *url)
{
    if (B_PlaybackIp_UtilsStristr(url, "http://") || B_PlaybackIp_UtilsStristr(url, "https://"))
        /* URI starts w/ protocol header, so it is a abslute URI */
        return true;
    else
        /* relative URI */
        return false;
}

int
http_parse_url(char *server, unsigned *portPtr, char **uri, char *url)
{
    char *tmp1, *tmp2, *tmp3;
    char *newUri;

    if ( (tmp1 = B_PlaybackIp_UtilsStristr(url, "http:")) == NULL) {
        /* relative url: just replace the last path of the current url w/ the new url, rest server name & port dont change */
        char *mediaSegmentBaseUri;
        int newUriLen;
        BDBG_MSG(("%s: current uri %s, new relative URI (%s)", __FUNCTION__, *uri, url));
        if ( (mediaSegmentBaseUri = find_base_uri(*uri)) != 0) {
            newUriLen = strlen(mediaSegmentBaseUri) + strlen(url) + 1;
            if ( (newUri = (char *)BKNI_Malloc(newUriLen)) == NULL) {
                BDBG_ERR(("%s: BKNI_Malloc failure for %d bytes", __FUNCTION__, newUriLen));
                return -1;
            }
            memset(newUri, 0, newUriLen);
            strncpy(newUri, mediaSegmentBaseUri, strlen(mediaSegmentBaseUri));
            strncat(newUri, url, strlen(url));
            BKNI_Free(*uri);
            *uri = newUri;
        }
        else {
            /* base url itself is relative, so replace it w/ new one */
            if ((newUri = B_PlaybackIp_UtilsStrdup(url)) == NULL) {
                BDBG_ERR(("%s: BKNI_Malloc failure for %zu bytes", __FUNCTION__, strlen(url)));
                return -1;
            }
            BKNI_Free(*uri);
            *uri = newUri;
        }
        BDBG_MSG(("%s: updated relative URI %s", __FUNCTION__, *uri));
        return 0;
    }
    /* http protocol is being used, parse it further */
    tmp1 += strlen("http://");

    /* now take out the server string from the url */
    tmp2 = strstr(tmp1, "/");
    if (tmp2) {
        strncpy(server, tmp1, (tmp2 - tmp1));
        server[tmp2-tmp1] = 0;

        /* Check to see if a port value was specified */
        tmp3 = strstr(server, ":");
        if (tmp3) {
            tmp3[0] = '\0'; /* this null terminates the server name string */
            tmp3++;
            *portPtr = strtoul(tmp3, (char **) NULL, 10);
        }
        else
            *portPtr = 80;

        /* now get the uri */
        if ((newUri = B_PlaybackIp_UtilsStrdup(tmp2)) == NULL) {
            BDBG_ERR(("%s: BKNI_Malloc failure for %zu bytes", __FUNCTION__, strlen(tmp2)));
            return -1;
        }
        BKNI_Free(*uri);
        *uri = newUri;
        BDBG_MSG(("server %s url %s", server, *uri));
        return 0;
    }
    else {
        BDBG_ERR(("%s: Failed to find the server part from the redirected URL %s", __FUNCTION__, url));
    }

    return -1;
}

int http_redirect_get_full_location_header(char **urlPtr, char *http_hdr)
{
    char *tmp1, *tmp2;

    if ( (tmp1 = B_PlaybackIp_UtilsStristr(http_hdr, "Location: http:")) == NULL) {
        BDBG_ERR(("No Location header in the HTTP redirect\n"));
        return -1;
    }
    /* redirect header is present, move past it */
    tmp1 += strlen("Location:");
    if ( ((tmp2 = B_PlaybackIp_UtilsStristr(tmp1, "\r\n")) != NULL) || ((tmp2 = B_PlaybackIp_UtilsStristr(tmp1, "\n")) != NULL)) {
        *tmp2 = '\0';
        BKNI_Free(*urlPtr);
        if ((*urlPtr = B_PlaybackIp_UtilsStrdup(tmp1)) == NULL) {
            BDBG_ERR(("%s: BKNI_Malloc failure for %zu bytes", __FUNCTION__, strlen(tmp1)));
            return -1;
        }
        BDBG_MSG(("%s: full location header url %s", __FUNCTION__, *urlPtr));
        return 0;
    }

    return -1;
}

int http_parse_redirect(char *server, unsigned *portPtr, B_PlaybackIpProtocol *protocol, char **urlPtr, char **cookie, char *http_hdr)
{
    char *tmp1, *tmp2, *tmp3;
    *cookie = NULL;
    bool absoluteUrl = true;

    if ( (tmp1 = B_PlaybackIp_UtilsStristr(http_hdr, "Location:")) == NULL) {
        BDBG_ERR(("No Location header in the HTTP redirect"));
        return -1;
    }
    /* redirect header is present, move past it */
    tmp1 += strlen("Location:");

    /* now look for protocol: supported ones are either HTTP or HTTPs */
    if ((tmp2 = B_PlaybackIp_UtilsStristr(tmp1, "http://")) != NULL) {
        tmp1 = tmp2 + strlen("http://");
        *protocol = B_PlaybackIpProtocol_eHttp;
    }
    else if ((tmp2 = B_PlaybackIp_UtilsStristr(tmp1, "https://")) != NULL) {
        tmp1 = tmp2 + strlen("https://");
        *protocol = B_PlaybackIpProtocol_eHttps;
    }
    else {
        /* protocol should be specified but we need to be forgiving to accept relative URLs w/o server:port part */
        BDBG_MSG(("%s: Redirect URI contains relative URL: http header %s", __FUNCTION__, http_hdr));
        absoluteUrl = false;
    }

    if (absoluteUrl == true)
    {
        /* now take out the server string from the Location header */
        tmp2 = strstr(tmp1, "/");
        if (tmp2)
        {
            strncpy(server, tmp1, (tmp2 - tmp1));
            server[tmp2-tmp1] = 0;

            /* Check to see if a port value was specified */
            tmp3 = strstr(server, ":");
            if (tmp3) {
                tmp3[0] = '\0'; /* this null terminates the server name string */
                tmp3++;
                *portPtr = strtoul(tmp3, (char **) NULL, 10);
            }

            BDBG_MSG(("%s: ### server %s", __FUNCTION__, server));
        }
        else
        {
            BDBG_ERR(("Failed to find the server part from the Location string of HTTP response\n"));
            return -1;
        }
    }
    else
    {
        /* !absoluteUrl, so the relative URL starts right after the location header */
        tmp2 = tmp1; /* tmp1 was pointing to right after the Location: string */
    }
    {
        /* now get the redirected URL */
        if ( (tmp1 = B_PlaybackIp_UtilsStristr(tmp2, "\r\n")) != NULL)
        {
            if (*urlPtr)
                BKNI_Free(*urlPtr);
            *tmp1 = '\0';
            if ((*urlPtr = B_PlaybackIp_UtilsStrdup(tmp2)) == NULL) {
                BDBG_ERR(("%s: ERROR: Failed to alloc buffer of size %zu server", __FUNCTION__, strlen(tmp2)));
                return -1;
            }
            BDBG_MSG(("%s: server %s:%d url %s", __FUNCTION__, server, *portPtr, *urlPtr));
            *tmp1 = '\r';
            if ( (tmp1 = B_PlaybackIp_UtilsStristr(http_hdr, "Set-Cookie:")) != NULL) {
                tmp1 += strlen("Set-Cookie:");
                if ( (tmp2 = B_PlaybackIp_UtilsStristr(tmp1, "\r\n")) != NULL) {
                    int cookieHeaderLength;
                    *tmp2 = '\0';
                    cookieHeaderLength = strlen(tmp1) + 1; /* +1 for the string terminator */
                    if ((*cookie = BKNI_Malloc(cookieHeaderLength)) == NULL) {
                        BDBG_ERR(("%s: ERROR: Failed to alloc buffer of size %d for cookie header", __FUNCTION__, cookieHeaderLength));
                        return -1;
                    }
                    BKNI_Memset(*cookie, 0, cookieHeaderLength);
                    strncpy(*cookie, tmp1, cookieHeaderLength-1);
                    *tmp2 = '\r';
                    BDBG_MSG(("%s: found cookie in the redirect reponse: %s", __FUNCTION__, *cookie));
                }
                else {
                    BDBG_ERR(("Failed to find CRNL in the Set-Cookie header of the HTTP redirect response"));
                    return -1;
                }
            }
            return 0;
        }
        else {
            BDBG_ERR(("Failed to find CRNL in the Location header of the HTTP response\n"));
        }
    }

    return -1;
}

/* returns the URL type */
http_url_type_t http_get_url_type(char *http_hdr, char *url)
{
    char *tmp1;
    char tmp2[TMP_STRING_SIZE+1];
    /* search for Location tag in the http response, it indicates redirection */
    if (B_PlaybackIp_UtilsStristr(http_hdr, "\nLocation: http://") != NULL || B_PlaybackIp_UtilsStristr(http_hdr, "\nLocation: ") != NULL) {
        /* redirection */
        BDBG_MSG(("Response contains a HTTP Redirect\n"));
        return HTTP_URL_IS_REDIRECT;
    }

    /* find the URL extension, e.g. /download.asp */
    tmp1 = strstr(url, ".");
    if (!tmp1)
        return HTTP_URL_IS_DIRECT;
    strncpy(tmp2, tmp1, TMP_STRING_SIZE);
    /* if URL extension contains any URL modifiers, exclude them */
    tmp1 = NULL;
    tmp1 = strstr(tmp2, "?");
    if (tmp1)
        *tmp1 = 0;
    BDBG_MSG(("extension is %s\n", tmp2));
    if (
        !strcmp(tmp2, ".asp") ||
        !strcmp(tmp2, ".ASP") ||
        !strcmp(tmp2, ".asx") ||
        !strcmp(tmp2, ".ASX")
        )
        return HTTP_URL_IS_ASX;
    else if (B_PlaybackIp_UtilsStristr(tmp2, ".pls") != NULL)
        return HTTP_URL_IS_PLS;
    else
        return HTTP_URL_IS_DIRECT;
}

/* returns server & url string from the ASX header */
int http_parse_asx_payload(char *server, char **urlPtr, char *asx_hdr)
{
    char *tmp1, *tmp2;

    if ( (tmp1 = B_PlaybackIp_UtilsStristr(asx_hdr, "ENTRYREF HREF=\"http://")) != NULL) {
        /* ENTRYREF element is present, its HREF attribute points an external ASX file, get is URL */
        tmp1 += strlen("ENTRYREF HREF=\"http://");
    }
    else if ( (tmp1 = strstr(asx_hdr, "Ref href=\"http://")) != NULL) {
        /* no ENTRYREF element, so this ASX payload must contain the REF element */
        /* its HREF attribute will point to the URL of the content */
        tmp1 += strlen("Ref href=\"http://");
    }
    else {
        BDBG_ERR(("ERROR: Neither ENTRYREF HREF nor Ref href elements found\n"));
        return -1;
    }
    if ( (tmp2 = strstr(tmp1, "/")) != NULL ) {
        strncpy(server, tmp1, (tmp2 - tmp1));
        server[tmp2-tmp1] = 0;

        if ( (tmp1 = strstr(tmp2, "\"")) != NULL) {
            BKNI_Free(*urlPtr);
            *tmp1 = '\0';
            if ((*urlPtr = B_PlaybackIp_UtilsStrdup(tmp2)) == NULL) {
                BDBG_ERR(("%s: ERROR: Failed to allocate %zu bytes of tmp2 buffer", __FUNCTION__, strlen(tmp2)));
                return -1;
            }
            BDBG_MSG(("server %s url %s\n", server, *urlPtr));
            return 0;
        }
    }

    return -1;
}

/* returns server & url string from the ASX header */
int http_parse_pls_payload(char *server, unsigned *portPtr, char **urlPtr, char *pls_hdr)
{
    char *tmp1, *tmp2, *tmp3;

    if ( (tmp1 = B_PlaybackIp_UtilsStristr(pls_hdr, "File1=http://")) != NULL) {
        /* get to start of the URL */
        tmp1 += strlen("File1=http://");
    }
    else {
        BDBG_ERR(("ERROR: PLS Playlist doesn't contain Fil1 track entry"));
        return -1;
    }
    if ( (tmp2 = strstr(tmp1, "/")) != NULL ) {
        /* tmp2 now points to the start of the file name part of the URL */
        strncpy(server, tmp1, (tmp2 - tmp1));
        server[tmp2-tmp1] = '\0';

        /* Check to see if a port value was specified */
        tmp3 = strstr(server, ":");
        if (tmp3) {
            tmp3[0] = '\0'; /* this null terminates the server name string */
            tmp3++;
            *portPtr = strtoul(tmp3, (char **) NULL, 10);
        }
        else {
            *portPtr = 80;
        }

        /* free up the previous URL */
        BKNI_Free(*urlPtr);
        *tmp2 = '/'; /* this was overwritten above w/ string terminator */
        if ( (tmp3 = strstr(tmp2, "\n")) != NULL )
            /* find the end of the File1 header, so that we just have the URI string */
            *tmp3 = '\0';
        if ((*urlPtr = B_PlaybackIp_UtilsStrdup(tmp2)) == NULL) {
            BDBG_ERR(("%s: ERROR: Failed to allocate %zu bytes of tmp2 buffer", __FUNCTION__, strlen(tmp2)));
            return -1;
        }
        BDBG_MSG(("%s: url: http://%s:%d%s", __FUNCTION__, server, *portPtr, *urlPtr));
        return 0;
    }

    return -1;
}

NEXUS_TransportType
http_get_payload_content_type(char *http_hdr)
{
    char *tmp1;
    char *tmp2;
    char *contentType = NULL;
    NEXUS_TransportType transportType = NEXUS_TransportType_eUnknown;

    tmp1 = B_PlaybackIp_UtilsStristr(http_hdr, "Content-Type: ");
    if (!tmp1)
       return NEXUS_TransportType_eUnknown;
    tmp2 = strstr(tmp1, "\r");
    if (!tmp2) {
        tmp2 = strstr(tmp1, "\n");
    }
    if (!tmp2) {
       return NEXUS_TransportType_eUnknown;
    }
    contentType = strndup(tmp1, tmp2 - tmp1+1);
    if (!contentType) {
       return NEXUS_TransportType_eUnknown;
    }
    contentType[tmp2-tmp1] = '\0';

    tmp1 = B_PlaybackIp_UtilsStristr(contentType, "Content-Type: video/");
    if (tmp1) {
        tmp1 += strlen("Content-type: video/");
        if (B_PlaybackIp_UtilsStristr(tmp1, "x-ms-wmv") || B_PlaybackIp_UtilsStristr(tmp1, "x-ms-asf")) {
            BDBG_MSG(("%s:WMV Content", __FUNCTION__));
            transportType = NEXUS_TransportType_eAsf;
            goto out;
        }
        else if (B_PlaybackIp_UtilsStristr(tmp1, "mpeg") || B_PlaybackIp_UtilsStristr(tmp1, "mpg") || B_PlaybackIp_UtilsStristr(tmp1, "mp2t")) {
            BDBG_MSG(("%s:MPEG2 TS Content", __FUNCTION__));
            transportType = NEXUS_TransportType_eTs;
            goto out;
        }
        else if (B_PlaybackIp_UtilsStristr(tmp1, "mp4")) {
            BDBG_MSG(("%s:MP4 Content", __FUNCTION__));
            transportType = NEXUS_TransportType_eMp4;
            goto out;
        }
    }
    tmp1 = B_PlaybackIp_UtilsStristr(http_hdr, "Content-Type: application/flv");
    if (tmp1) {
        BDBG_MSG(("%s:MP4 Content", __FUNCTION__));
        transportType = NEXUS_TransportType_eMp4;
        goto out;
    }
    tmp1 = B_PlaybackIp_UtilsStristr(http_hdr, "Content-Type: video/x-flv");
    if (tmp1) {
        BDBG_MSG(("%s:Flash Content", __FUNCTION__));
        transportType = NEXUS_TransportType_eFlv;
        goto out;
    }
    tmp1 = B_PlaybackIp_UtilsStristr(http_hdr, "Content-Type: audio/");
    if (tmp1) {
        if (B_PlaybackIp_UtilsStristr(tmp1, "x-ms-wma") || B_PlaybackIp_UtilsStristr(tmp1, "x-ms-asf")) {
            BDBG_MSG(("%s:WMA Content", __FUNCTION__));
            transportType = NEXUS_TransportType_eAsf;
            goto out;
        }
        else if (B_PlaybackIp_UtilsStristr(tmp1, "mpeg") || B_PlaybackIp_UtilsStristr(tmp1, "mpg") ||
                 B_PlaybackIp_UtilsStristr(tmp1, "x-aac") || B_PlaybackIp_UtilsStristr(tmp1, "mp3")) {
            BDBG_MSG(("%s:MPEG Audio Content", __FUNCTION__));
            transportType = NEXUS_TransportType_eEs;
            goto out;
        }
    }
    /* some HTTP server specify MP4 movie content under audio */
    tmp1 = B_PlaybackIp_UtilsStristr(http_hdr, "Content-Type: ");
    if (tmp1) {
        tmp1 += strlen("Content-type: ");
        if (B_PlaybackIp_UtilsStristr(tmp1, "x-ms-wmv") || B_PlaybackIp_UtilsStristr(tmp1, "x-ms-asf")) {
            BDBG_MSG(("%s:WMV Content", __FUNCTION__));
            transportType = NEXUS_TransportType_eAsf;
            goto out;
        }
        else if (B_PlaybackIp_UtilsStristr(tmp1, "mpeg") || B_PlaybackIp_UtilsStristr(tmp1, "mpg")) {
            BDBG_MSG(("%s:MPEG2 TS Content", __FUNCTION__));
            transportType = NEXUS_TransportType_eTs;
            goto out;
        }
        else if (B_PlaybackIp_UtilsStristr(tmp1, "mp4")) {
            BDBG_MSG(("%s:MP4 Content", __FUNCTION__));
            transportType = NEXUS_TransportType_eMp4;
            goto out;
        }
    }
    transportType = NEXUS_TransportType_eUnknown;
out:
    if (contentType)
        free(contentType);
    return transportType;
}

/*
 * This function writes the given data to the socket and returns any errors or the bytes wrote.
 * It returns:
 *  =-1: for errors other than EINTR & EAGAIN during write call or when channel change occurs
 *  = 0: for EOF where server closed the TCP connection
 *  > 0: for success indicating number of bytes written to the socket
 */
static int _http_socket_write(void *voidHandle, volatile B_PlaybackIpState *playbackIpState, int sd, char *wbuf, int wbuf_len)
{
    int rc;

    BSTD_UNUSED(voidHandle);

    rc = B_PlaybackIp_UtilsTcpSocketWrite(playbackIpState, sd, wbuf, wbuf_len);
    if (rc <= 0) {
        BDBG_ERR(("%s: write System Call interrupted or timed out, retrying (rc %d, errno %d)\n", __FUNCTION__, rc, errno));
        return rc;
    }
    return wbuf_len;
}

/* builds HTTP get request */
int http_build_get_req(char **write_buf_pptr, B_PlaybackIpHandle playback_ip, int speed, double timeSeekRangeBegin, double timeSeekRangeEnd, off_t byteRangeStart, off_t byteRangeEnd, char *playSpeedString)
{
    B_PlaybackIpSocketOpenSettingsHandle socketOpenSettings = &playback_ip->openSettings.socketOpenSettings;
    B_PlaybackIpSessionOpenSettings *openSettings = &playback_ip->openSettings;
    unsigned int bytesLeft;
    int bytesWrote;
    int write_buf_size;
    char *write_buf;
    char *header;

    write_buf_size = strlen(socketOpenSettings->ipAddr) + strlen(socketOpenSettings->url);
    if (openSettings->u.http.userAgent)
        write_buf_size += strlen(openSettings->u.http.userAgent);
    if (openSettings->u.http.additionalHeaders)
        write_buf_size += strlen(openSettings->u.http.additionalHeaders);
    write_buf_size += 512; /* add room for rest of HTTP headers which have fixed sizes */
    write_buf = (char *)BKNI_Malloc(write_buf_size);
    if (!write_buf) {
        BDBG_ERR(("%s: ERROR: failed to allocate memory\n", __FUNCTION__));
        return -1;
    }
    memset(write_buf, 0, write_buf_size);
    bytesLeft = write_buf_size;
    bytesWrote = 0;
    header = write_buf;
    *write_buf_pptr = write_buf;

    bytesWrote = snprintf(header, bytesLeft,
            "%s %s HTTP/1.1\r\n"
            "User-Agent: %s\r\n"
            "Connection: Close\r\n"
            "%s"
            "EncEnabled: %s\r\n"
            ,
            (openSettings->u.http.useHeadRequestMethod)?"HEAD":"GET",
            socketOpenSettings->url,
            (openSettings->u.http.userAgent ? openSettings->u.http.userAgent : "BRCM HTTP Client/2.0"),
            (openSettings->u.http.additionalHeaders ? openSettings->u.http.additionalHeaders : "") ,
            openSettings->security.securityProtocol == B_PlaybackIpSecurityProtocol_DtcpIp? "Yes":"No"
            );
    bytesLeft -= bytesWrote;
    header += bytesWrote;

    if (!playback_ip->openSettings.socketOpenSettings.useProxy) {
        if (socketOpenSettings->port == HTTP_DEFAULT_PORT) {
            bytesWrote = snprintf(header, bytesLeft, "Host: %s\r\n", socketOpenSettings->ipAddr);
        }
        else {
            /* not using the default HTTP port, so Host Header needs to include the port # */
            bytesWrote = snprintf(header, bytesLeft, "Host: %s:%d\r\n", socketOpenSettings->ipAddr, socketOpenSettings->port);
        }
    }
    else {
        char *tmp1, *tmp2;
        tmp1 = strstr(socketOpenSettings->url, "://"); /* host name starts after this sequence */
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
    if (openSettings->u.http.rvuCompliant) {
        /* RVU server always requires timeSeekRange header */
        if (timeSeekRangeEnd != 0.)
            bytesWrote = snprintf(header, bytesLeft, "TimeSeekRange.dlna.org: npt=%.3f-%.3f\r\n", timeSeekRangeBegin, timeSeekRangeEnd);
        else
            bytesWrote = snprintf(header, bytesLeft, "TimeSeekRange.dlna.org: npt=%.3f-\r\n", timeSeekRangeBegin);
        bytesLeft -= bytesWrote;
        header += bytesWrote;

        if (speed == 1) {
            /* RVU server only requires play speed for normal play mode. Also, speed keyword is not required in the syntax */
            bytesWrote = snprintf(header, bytesLeft, "PlaySpeed.dlna.org: speed=%d\r\n", speed);
        }
        else {
            /* include the Frame Count header */
            bytesWrote = snprintf(header, bytesLeft, "frameCount.rvualliance.org: 1\r\n");
        }
        bytesLeft -= bytesWrote;
        header += bytesWrote;
    }
    else {
        if (timeSeekRangeBegin > 0.) {
            if (timeSeekRangeEnd > timeSeekRangeBegin) {
                bytesWrote = snprintf(header, bytesLeft, "TimeSeekRange.dlna.org: npt=%.3f-%.3f\r\n", timeSeekRangeBegin, timeSeekRangeEnd);
            }
            else {
                bytesWrote = snprintf(header, bytesLeft, "TimeSeekRange.dlna.org: npt=%.3f-\r\n", timeSeekRangeBegin);
            }
            bytesLeft -= bytesWrote;
            header += bytesWrote;
        }
        if (playSpeedString != NULL) {
#ifdef B_USE_CUSTOM_DLNA_PROTOCOL_1
#define PLAYSPEED_FORMAT "PlaySpeed.dlna.org: "
#else
#define PLAYSPEED_FORMAT "PlaySpeed.dlna.org: speed="
#endif
            bytesWrote = snprintf(header, bytesLeft, "%s%s\r\n", PLAYSPEED_FORMAT, playSpeedString);
            bytesLeft -= bytesWrote;
            header += bytesWrote;
        }
        else if (speed != 1) {
            bytesWrote = snprintf(header, bytesLeft, "%s%d\r\n", PLAYSPEED_FORMAT, speed);
            bytesLeft -= bytesWrote;
            header += bytesWrote;
        }
    }
    if (byteRangeStart != 0) {
        char *rangeString;
        if (openSettings->security.securityProtocol == B_PlaybackIpSecurityProtocol_DtcpIp)
            rangeString = "Range.dtcp.com:";
        else
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
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("Requesting URL: http://%s:%d%s", socketOpenSettings->ipAddr, socketOpenSettings->port, socketOpenSettings->url));
#endif
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog) {
        BDBG_WRN(("Complete HTTP Get request:\n%s\n", write_buf));
        fprintf(stdout, "%s", write_buf);
        fflush(stdout);
    }
#endif
    return 0;
}

int B_PlaybackIp_SecurityDecryptionDisable(
    B_PlaybackIpHandle playback_ip
    )
{
    int rc = 0;
    BDBG_ASSERT(playback_ip);

    /* set the default session settings */
    switch (playback_ip->openSettings.security.securityProtocol) {
        case B_PlaybackIpSecurityProtocol_None:    /* Clear Content */
            break;
#ifdef B_HAS_SSL
        case B_PlaybackIpSecurityProtocol_Ssl:     /* SSL/TLS protected Content */
            /* No action needs to be taken as all payload is already decrypted before it gets to this HTTP layer */
            break;
#endif
#ifdef B_HAS_DTCP_IP
        case B_PlaybackIpSecurityProtocol_DtcpIp:  /* DTCP-IP protected Content */
            rc = B_PlaybackIp_DtcpIpDecryptionDisable(playback_ip->securityHandle);
            BDBG_MSG(("%s: Disabled DTCP/IP Decryption \n", __FUNCTION__));
            break;
#endif
#ifdef B_HAS_RAD_EA
        case B_PlaybackIpSecurityProtocol_RadEa:   /* Rhapsody's RAD-EA protected Content */
            break;
#endif
        default:
            BDBG_ERR(("%s: Security protocol (%d) is not supported\n", __FUNCTION__, playback_ip->openSettings.security.securityProtocol));
            return -1;
    }
    return rc;
}

int B_PlaybackIp_SecurityDecryptionEnable(
    B_PlaybackIpHandle playback_ip,
    char *initialPayload,
    int *initialPayloadLength)
{
    int rc = 0;
    BDBG_ASSERT(playback_ip);

    /* set the default session settings */
    switch (playback_ip->openSettings.security.securityProtocol) {
        case B_PlaybackIpSecurityProtocol_None:    /* Clear Content */
            BSTD_UNUSED(initialPayloadLength);
            BSTD_UNUSED(initialPayload);
            rc = 0;
            break;
#ifdef B_HAS_SSL
        case B_PlaybackIpSecurityProtocol_Ssl:     /* SSL/TLS protected Content */
        case B_PlaybackIpSecurityProtocol_Aes128:  /* AES128 protected Content */
            BSTD_UNUSED(initialPayloadLength);
            BSTD_UNUSED(initialPayload);
            /* No action needs to be taken as Decryption is already enabled during SSL Session Open */
            rc = 0;
            break;
#endif
#ifdef B_HAS_DTCP_IP
        case B_PlaybackIpSecurityProtocol_DtcpIp:  /* DTCP-IP protected Content */
            rc = B_PlaybackIp_DtcpIpDecryptionEnable(playback_ip->securityHandle, initialPayload, *initialPayloadLength);
            BDBG_MSG(("%s: Enabled DTCP/IP Decryption and passed in intial payload of %d length to security module\n", __FUNCTION__, *initialPayloadLength));
            /* since initial payload is handled over the dtcp security layer, return 0 length to caller */
            *initialPayloadLength = 0;
            break;
#endif
#ifdef B_HAS_RAD_EA
        case B_PlaybackIpSecurityProtocol_RadEa:   /* Rhapsody's RAD-EA protected Content */
            rc = B_PlaybackIp_RadEaDecryptionEnable(playback_ip->securityHandle, initialPayload, *initialPayloadLength);
            BDBG_MSG(("%s: Enabled RAD/EA Decryption and passed in intial payload of %d length to security module\n", __FUNCTION__, *initialPayloadLength));
            /* since initial payload is handled over the security layer, return 0 length to caller */
            *initialPayloadLength = 0;
            break;
#endif
        default:
            BDBG_ERR(("%s: Security protocol (%d) is not supported\n", __FUNCTION__, playback_ip->openSettings.security.securityProtocol));
            return -1;
    }
    return rc;
}

/*
 * This function tries to read the requested amount from the socket and returns any errors or bytes read.
 * It returns:
 *  =-1: for select timeout a/f HTTP_SELECT_TIMEOUT secs, errors other than EINTR, or when channel change occurs
 *  = 0: for read ready event on the socket when some data is available for reading.
 */
int
_http_socket_select(B_PlaybackIpHandle playback_ip, int fd, int timeout)
{
    int rc = -1;
    fd_set rfds;
    struct timeval tv;
    int timeoutInterval;
    int timeoutIterations;
    int numTimeoutInterations = 0;

    if ( playback_ip->playback_state >= B_PlaybackIpState_eSessionSetup && (playback_ip->psi.liveChannel || playback_ip->setupSettings.u.http.liveChannel)) {
        /* we dont want to use the lower readTimeout before the SessionSetup state as Index reads can't fail due to timeouts, otherwise bmedia_probe will fail */
        timeoutInterval = 20; /* for live modes, we want to be more aggressive in handling the timeouts */
        timeoutIterations = playback_ip->setupSettings.u.http.readTimeout/timeoutInterval; /* readTimeout is set by the apps to a lower timeout value in live mode vs the pull mode to lower any latency for the low bitrate channels */
    }
    else {
        timeoutInterval = 200; /* 20msec */
        timeoutIterations = (timeout * 1000) / timeoutInterval;
    }
    if (timeoutIterations <= 0) timeoutIterations = 1;
    while (numTimeoutInterations++ < timeoutIterations) {
            /* user changed the channel, so return */
        if ( breakFromLoop(playback_ip)) {
            PRINT_SOCKET_MSG(("%s:%p breaking out of select loop due to state (%d) change, # of timeout iterations %d, total timeout %d seconds\n", __FUNCTION__, (void *)playback_ip, playback_ip->playback_state, numTimeoutInterations, (numTimeoutInterations*timeoutInterval)/1000));
            if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode)
                playback_ip->selectTimeout = true;
            else
                playback_ip->serverClosed = true;
            return -1;
        }

        if ( fd < 0 ) {
            playback_ip->serverClosed = true;
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
            BDBG_ERR(("!!! %s : %d => bad file descriptor(fd %d)\n", __FUNCTION__, __LINE__, fd));
#endif
            return -1;
        }

        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        tv.tv_sec = 0;
        tv.tv_usec = timeoutInterval * 1000; /* in micro-secs */

        rc = select(fd +1, &rfds, NULL, NULL, &tv);
        if (rc < 0) {
            if (errno == EINTR) {
                BDBG_MSG(("%s: select System Call interrupted, retrying\n", __FUNCTION__));
                continue;
            }
            BDBG_ERR(("%s: ERROR: select(): rc %d, errno %d, fd %d", __FUNCTION__, rc, errno, fd));
            /* got some socket error, return failure */
            playback_ip->serverClosed = true;
            return -1;
        }

        if (rc == 0 || !FD_ISSET(fd, &rfds)) {
            /* select timeout or some select event but our FD not set: No more data - wait */
            playback_ip->selectTimeout = true;
            PRINT_SOCKET_MSG(("%s: selectTimeout is true, fd=%d rc=%d", __FUNCTION__, fd, rc));
            continue;
        }
        /* ready ready event on socket, return succcess */
        PRINT_SOCKET_MSG(("%s:%p select read ready event (before the timeout of %d sec), iterations: cur %d, max %d", __FUNCTION__, (void *)playback_ip, timeout, numTimeoutInterations, timeoutIterations));
        return 0;
    }
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s:%p: select timed out after %d sec, iterations: cur %d, max %d\n", __FUNCTION__, (void *)playback_ip, timeout, numTimeoutInterations, timeoutIterations));
#endif
    /* select had just timed out due to no data being available on socket */
    playback_ip->selectTimeout = true;
    return -1;
}

/*
 * This function tries to read the requested amount from the socket and returns any errors or bytes read.
 * It returns:
 *  =-1: for errors other than EINTR & EAGAIN during read call or when channel change occurs
 *  = 0: for EOF where server closed the TCP connection
 *  > 0: for success indicating number of bytes read from the socket
 */
int
_http_socket_read(void *voidHandle, B_PlaybackIpHandle playback_ip, int sd, char *rbuf, int rbuf_len)
{
    int rc;
    BSTD_UNUSED(voidHandle);    /* not needed for clear net operations */
    while (true) {
        if ( breakFromLoop(playback_ip)) {
            /* user changed the channel, so return */
            BDBG_MSG(("%s: breaking out of read loop due to state (%d) change", __FUNCTION__, playback_ip->playback_state));
            if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode) {
                playback_ip->selectTimeout = true;
                errno = EINTR;
            }
            else {
                playback_ip->serverClosed = true;
            }
            return -1;
        }
        rc = read(sd, rbuf, rbuf_len);
        if (rc < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                BDBG_ERR(("%s: Read System Call interrupted or timed out, sleeping for 100msec & then retrying (errno %d)", __FUNCTION__, errno));
                BKNI_Sleep(100);
                continue;
            }
            BDBG_ERR(("%s: read ERROR:%d", __FUNCTION__, errno));
            return -1;
        }
        else if (rc == 0) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_ERR(("%s: Reached EOF, server closed the connection!", __FUNCTION__));
#endif
            return 0;
        }
        BDBG_MSG_FLOW(("%s: bytes read %d", __FUNCTION__, rc));
        return rc;
    }
    /* we shouldn't get here */
    BDBG_ASSERT(NULL);
}

/* This function loops until requested amount is read from the socket unless there is a socket error,
 * read interval timeout, or channel is changed. In addition, it performs all relevent error checking.
 * It returns:
 *  =-1: for errors other than EINTR & EAGAIN during read call or when channel change occurs
 *  = 0: for EOF where server closed the TCP connection, serverClosed flag is set
 *  > 0: for success indicating number of bytes read from the socket.
 *       if (bytesRead != bytesToRead), serverClosed flag is also set.
 */
int
http_socket_read(B_PlaybackIpHandle playback_ip, void *securityHandle, int sd, char *buf, int bytesToRead, int *bytesRead, int timeout)
{
    int rc = -1;
    unsigned readTimeout;
    B_Time startTime, endTime;
    B_Time_Get(&startTime);
#ifdef B_HAS_DTCP_IP
    int encryptedBufLength = 0;
#endif
    int bytesRemaining = 0;
    /* loop until requested data is read from socket */
    *bytesRead = 0;
    playback_ip->serverClosed = false;
    playback_ip->selectTimeout = false;
    while (*bytesRead < bytesToRead) {
        if ( breakFromLoop(playback_ip)) {
            /* user changed the channel, so return */
            PRINT_SOCKET_MSG(("%s: breaking out of read loop due to state (%d) change", __FUNCTION__, playback_ip->playback_state));
            if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode)
                playback_ip->selectTimeout = true;
            else
                playback_ip->serverClosed = true;
            break;
        }
        if (playback_ip->playback_state >= B_PlaybackIpState_eSessionSetup && (playback_ip->psi.liveChannel || playback_ip->setupSettings.u.http.liveChannel)) {
            /* TODO: it may be fine to even apply this readTimeout concept for non-live channels also */
            /* we may want to timeout and return whatever we have read for slow internet connections */
            B_Time_Get(&endTime);
            if ((readTimeout = B_Time_Diff(&endTime, &startTime)) > playback_ip->setupSettings.u.http.readTimeout) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: breaking from reading loop due to exceeding the read timeout: max read timeout %d, timeout %d, bytesRead %d", __FUNCTION__, playback_ip->setupSettings.u.http.readTimeout, readTimeout, *bytesRead));
#endif
                break;
            }
        }

        /* wait for some data to be available on this socket */
        if (_http_socket_select(playback_ip, sd, timeout)) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_ERR(("%s: select ERROR:%d, state %d, local socket info: sd:ip:port = %d:%s:%d",
                        __FUNCTION__, errno, playback_ip->playback_state, sd, inet_ntoa(playback_ip->socketState.local_addr.sin_addr),
                        ntohs(playback_ip->socketState.local_addr.sin_port)));
#endif
            rc = -1;
            break;
        }

#ifdef B_HAS_DTCP_IP
        if (playback_ip->openSettings.security.securityProtocol == B_PlaybackIpSecurityProtocol_DtcpIp && playback_ip->chunkEncoding &&  /* for a session that has dtcp & chunk encoding enabled */
                playback_ip->bytesLeftInCurChunk && playback_ip->bytesLeftInCurChunk-*bytesRead < HTTP_AES_BLOCK_SIZE ) /* if current chunk has bytes left && bytes left in the current chunk fall below the AES block size, we can't ask dtcp layer to decrypt them */
            /* this is because dtcp layer will decrypt next 16 bytes where as some of them are not encrypted as they belong to the HTTP Chunk header */
        {
            if (B_PlaybackIp_SecurityDecryptionDisable(playback_ip) != 0) {
                BDBG_ERR(("%s: Failed to Disable the Decryption during next chunk xfer header processing", __FUNCTION__));
                return -1;
            }
            encryptedBufLength = playback_ip->bytesLeftInCurChunk - *bytesRead;
            bytesRemaining = encryptedBufLength;
        }
        else
#endif
        bytesRemaining = bytesToRead - *bytesRead;
        /* read from the socket */
        PRINT_SOCKET_MSG(("%s: before reading: bytes read %d, asked %d, remaining %d, chunkSize %"PRId64 "", __FUNCTION__, *bytesRead, bytesToRead, bytesRemaining, playback_ip->chunkSize));
        if ( (rc = playback_ip->netIo.read(securityHandle, playback_ip, sd, (buf+*bytesRead), bytesRemaining)) <= 0) {
            if (rc != 0 && (errno == EINTR || errno == EAGAIN)) {
                playback_ip->selectTimeout = true;
                rc = -1;
                PRINT_SOCKET_MSG(("%s: read ERROR:%d, selectTimeout is set to true\n", __FUNCTION__, errno));
#ifdef B_HAS_DTCP_IP
                if (playback_ip->dtcpPcpHeaderFound) {
                    goto dtcpHeaderAdjustment;
                }
#endif
                break;
            }
            if (rc == 0) { /* I dont think we need to wait for last chunk to arrive before settings the server closed flag */
                playback_ip->serverClosed = true;
                PRINT_SOCKET_MSG(("%s: read ERROR:%d, setting serverClosed flag to true, rc %d", __FUNCTION__, errno, rc));
            }
            break;
        }
#ifdef B_HAS_DTCP_IP
        if (encryptedBufLength) {
            /* we told dtcp lib to not decrypt these bytes, so we have to cache them and feed them back to the lib during when we re-enable the decryption after processing the next http chunk hdr */
            BDBG_ASSERT(rc < HTTP_AES_BLOCK_SIZE);
            BDBG_ASSERT(rc+playback_ip->encryptedBufLength < HTTP_AES_BLOCK_SIZE);
            memcpy(playback_ip->encryptedBuf+playback_ip->encryptedBufLength, (buf+*bytesRead), rc);
            playback_ip->encryptedBufLength += rc;
            PRINT_SOCKET_MSG(("%s: after reading: cached %d encrypted bytes, bytes read: cur %d, total %d, asked %d", __FUNCTION__, playback_ip->encryptedBufLength, rc, *bytesRead, bytesToRead));
            playback_ip->bytesLeftInCurChunk -= rc; /* this allows the caller to not expect these encrypted bytes in this chunk */
            encryptedBufLength = 0;
            break;
        }
#endif
        *bytesRead += rc;
        PRINT_SOCKET_MSG(("after reading: bytes read: cur %d, total %d, asked %d", rc, *bytesRead, bytesToRead));
#ifdef B_HAS_DTCP_IP
        if (playback_ip->dtcpPcpHeaderFound) {
dtcpHeaderAdjustment:
            playback_ip->chunkSize -= DTCP_CONTENT_PACKET_HEADER_SIZE;
            playback_ip->bytesLeftInCurChunk -= DTCP_CONTENT_PACKET_HEADER_SIZE;
            if (playback_ip->adjustBytesToRead) {
                bytesToRead -= DTCP_CONTENT_PACKET_HEADER_SIZE;
                playback_ip->adjustBytesToRead = false;
            }
            if (playback_ip->chunkSize < 0 || playback_ip->bytesLeftInCurChunk < 0 || bytesToRead < 0) {
                BDBG_ERR(("BUG: chunkSize %"PRId64 " is < 0, bytesLeftInChunk %"PRId64 ", bytesToRead %d", playback_ip->chunkSize, playback_ip->bytesLeftInCurChunk, bytesToRead));
                playback_ip->chunkSize = 0;
                BDBG_ASSERT(NULL);
            }
            playback_ip->dtcpPcpHeaderFound = false;
            PRINT_SOCKET_MSG(("%s: bytes read %d, asked %d, adjusting by DTCP PCP Header length of 14 bytes, bytesLeftInCurCHunk %"PRId64 ", chunkSize %"PRId64 "\n",
                        __FUNCTION__, *bytesRead, bytesToRead, playback_ip->bytesLeftInCurChunk, playback_ip->chunkSize));
        }
#endif
        /* if select timeout was set during the last read, break out from the
         * read loop. This is done to avoid getting stuck in read loop for
         * cases when low bitrate content is being received.
         */
        if (playback_ip->selectTimeout) {
            PRINT_SOCKET_MSG(("%s: breaking from reading loop due to select timeout, bytes read %d", __FUNCTION__, rc));
            break;
        }
    }
    if (*bytesRead) {
        playback_ip->totalRead += *bytesRead;
        PRINT_SOCKET_MSG(("%s: fd %d, bytes read %d, asked %d\n", __FUNCTION__, sd, *bytesRead, bytesToRead));
        return *bytesRead;
    }
    else
        return rc;
}

int B_PlaybackIp_ClearSessionOpen(
    B_PlaybackIpSessionOpenSettings *openSettings,
    int sd,                                                     /* input: socket descriptor */
    B_PlaybackIpSecurityOpenOutputParams *securityOpenParams) /* output: security return parameter */
{
    /* For clear content, we only need to setup the netIo interface */
    BSTD_UNUSED(openSettings);
    BSTD_UNUSED(sd);

    securityOpenParams->byteRangeOffset = 0;

    BDBG_MSG(("%s: setting up the netIo interface for socket read & write\n", __FUNCTION__));
    securityOpenParams->netIoPtr->read = _http_socket_read;
    securityOpenParams->netIoPtr->write = _http_socket_write;
    securityOpenParams->netIoPtr->close = B_PlaybackIp_ClearSessionClose;
    securityOpenParams->netIoPtr->suspend = NULL;
    securityOpenParams->netIoPtr->shutdown = NULL;
    return 0;
}

void B_PlaybackIp_ClearSessionClose(
    void *securityHandle)                                      /* input: security module specific handle */
{
    /* For clear content, there is nothing to do */
    BSTD_UNUSED(securityHandle);
}

int
B_PlaybackIp_SecuritySessionOpen(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionOpenSettings *openSettings,
    int fd,
    void **newSecurityHandle)
{
    int rc = 0;
    B_PlaybackIpSecurityOpenOutputParams securityOpenOutputParams;
    BDBG_ASSERT(playback_ip);

    securityOpenOutputParams.netIoPtr = &playback_ip->netIo;
    securityOpenOutputParams.securityHandle = newSecurityHandle;
    securityOpenOutputParams.byteRangeOffset = 0;

    /* set the default session settings */
    B_PlaybackIp_ClearSessionOpen(openSettings, fd, &securityOpenOutputParams);
    switch (openSettings->security.securityProtocol) {
        case B_PlaybackIpSecurityProtocol_None:    /* Clear Content */
            rc = 0;
            break;
#ifdef B_HAS_SSL
        case B_PlaybackIpSecurityProtocol_Ssl:     /* SSL/TLS protected Content */
            BDBG_MSG(("%s: Using SSL/TLS protocol", __FUNCTION__));
            openSettings->security.enableDecryption = true;    /* For SSL/TLS, HTTP Response is also encrypted */
            rc = B_PlaybackIp_SslSessionOpen(openSettings, fd, &securityOpenOutputParams );
            break;
#endif
#ifdef B_HAS_DTCP_IP
        case B_PlaybackIpSecurityProtocol_DtcpIp:  /* DTCP-IP protected Content */
            openSettings->security.enableDecryption = false;    /* For DTCP/IP, HTTP Response is in clear */
            BDBG_MSG(("%s: Using DTCP-IP protocol", __FUNCTION__));
            rc = B_PlaybackIp_DtcpIpSessionOpen(openSettings, fd, &securityOpenOutputParams );
            break;
#endif
#ifdef B_HAS_RAD_EA
        case B_PlaybackIpSecurityProtocol_RadEa:   /* Rhapsody's RAD-EA protected Content */
            openSettings->security.enableDecryption = false;    /* For RAD/EA, HTTP Response is in clear */
            BDBG_MSG(("%s: Using Rhapsody's RAD-EA protocol", __FUNCTION__));
            rc = B_PlaybackIp_RadEaSessionOpen(openSettings, fd, &securityOpenOutputParams );
            break;
#endif
#ifdef B_HAS_HTTP_AES_SUPPORT
        case B_PlaybackIpSecurityProtocol_Aes128:  /* AES protected Content */
            openSettings->security.enableDecryption = false;    /* For AES, HTTP Response is in clear */
            BDBG_MSG(("%s: Using AES protocol", __FUNCTION__));
            rc = B_PlaybackIp_AesSessionOpen(openSettings, fd, &securityOpenOutputParams );
            break;
#endif
        default:
            BDBG_ERR(("%s: Security protocol (%d) is not supported\n", __FUNCTION__, openSettings->security.securityProtocol));
            return -1;
    }

    playback_ip->byteRangeOffset = securityOpenOutputParams.byteRangeOffset;

    return rc;
}

int B_PlaybackIp_SecurityCloneSessionOpen(
    B_PlaybackIpHandle playback_ip,
    int fd,
    void *securityHandle,
    void **newSecurityHandle)
{
    int rc = 0;
    BDBG_ASSERT(playback_ip);

    switch (playback_ip->openSettings.security.securityProtocol) {
        case B_PlaybackIpSecurityProtocol_None:    /* Clear Content */
            BSTD_UNUSED(securityHandle);
            BSTD_UNUSED(newSecurityHandle);
            BSTD_UNUSED(fd);
            rc = 0;
            break;
#ifdef B_HAS_SSL
        case B_PlaybackIpSecurityProtocol_Ssl:     /* SSL/TLS protected Content */
            BDBG_MSG(("%s: Using SSL/TLS protocol\n", __FUNCTION__));
            rc = B_PlaybackIp_SslCloneSessionOpen(fd, securityHandle, newSecurityHandle);
            break;
#endif
#ifdef B_HAS_DTCP_IP
        case B_PlaybackIpSecurityProtocol_DtcpIp:  /* DTCP-IP protected Content */
            BDBG_MSG(("%s: Using DTCP-IP protocol\n", __FUNCTION__));
            rc = B_PlaybackIp_DtcpIpCloneSessionOpen(fd, securityHandle, newSecurityHandle);
            break;
#endif
#ifdef B_HAS_RAD_EA
        case B_PlaybackIpSecurityProtocol_RadEa:   /* Rhapsody's RAD-EA protected Content */
            BDBG_MSG(("%s: Using Rhapsody's RAD-EA protocol\n", __FUNCTION__));
            rc = B_PlaybackIp_RadEaCloneSessionOpen(fd, securityHandle, newSecurityHandle);
            break;
#endif
        default:
            BDBG_ERR(("%s: Security protocol (%d) is not supported\n", __FUNCTION__, playback_ip->openSettings.security.securityProtocol));
            return -1;
    }
    return rc;
}

int
http_read_response(B_PlaybackIpHandle playback_ip, void *securityHandle, int fd, char **rbufp, int rbuf_size, char **http_hdr, char **http_payload, int *payload_len)
{
    int bytesRead;
    char *tmp_ptr;
    char *rbuf;
#ifdef B_HAS_NETACCEL1
    STRM_SockRecvParams_t sockRecvParams = STRM_SOCK_RECV_PARAMS_HTTP_DEFAULT;
#endif
    int rc = -1;
    int initial_payload_len = 0;
    int chunk_hdr_size;
    B_PlaybackIpHttpMsgFields httpFields;
    int loopCnt = 0;

    BDBG_MSG(("%s:%p state %d, serverClosed %d\n", __FUNCTION__, (void *)playback_ip, playback_ip->playback_state, playback_ip->serverClosed));
    httpFields.parsingResult = B_PlaybackIpHttpHdrParsingResult_eStatusNotSupported;
    rbuf = *rbufp;
    if (!rbuf) {
        BDBG_ERR(("rbuf error"));
        return -1;
    }

read_again:
    memset(rbuf, 0, rbuf_size+1);
    /* coverity[ -tainted_data_argument : arg-1 ] */
    *http_hdr = NULL;
    bytesRead = 0;
    /* read response until end of response header is found or max buffer size exceeded */
    while (rbuf_size <= DLNA_MAX_HTTP_RESP_SIZE && playback_ip->serverClosed == false) {
        if ( breakFromLoop(playback_ip)) {
            /* user changed the channel, so return */
            BDBG_MSG(("%s: breaking out of while loop due to state (%d) change", __FUNCTION__, playback_ip->playback_state));
            return -1;
        }
        if (_http_socket_select(playback_ip, fd, playback_ip->networkTimeout)) {
            if (playback_ip->selectTimeout) {
                BDBG_MSG(("%s: select timeout, so retry read", __FUNCTION__));
            }
            BDBG_ERR(("%s: select ERROR:%d\n", __FUNCTION__, errno));
            return -1;
        }

        /* Assume sizeof(rbuf) >= rbuf_size  */
        /* coverity[ -tainted_data_argument : arg-2 ] */
        BDBG_MSG(("%s:%p: before netIo.read", __FUNCTION__, (void *)playback_ip));
        if ((rc = playback_ip->netIo.read(securityHandle, playback_ip, fd, rbuf+bytesRead, rbuf_size - bytesRead)) <= 0) {
            if (errno == EINTR || errno == EAGAIN) {
                if (++loopCnt > 1) {
                    playback_ip->selectTimeout = true;
                    BDBG_ERR(("%s: read ERROR:%d, selectTimeout is set to true\n", __FUNCTION__, errno));
                }
                else {
                    BDBG_MSG(("%s: read ERROR:%d, select timeout, sleeping for 500 msec and retrying once", __FUNCTION__, errno));
                    BKNI_Sleep(500);
                    continue;
                }
            }
            else
                playback_ip->serverClosed = true;
            BDBG_ERR(("%s: read failed, rc %d, read %d, asked %d", __FUNCTION__, rc, bytesRead, rbuf_size));
            return -1;
        }
        bytesRead += rc;
        playback_ip->totalRead += rc;
        BDBG_MSG(("%s:%p: after netIo.read, bytesRead %d, rc %d", __FUNCTION__, (void *)playback_ip, bytesRead, rc));
        if (B_PlaybackIp_UtilsHttpResponseParse(rbuf, bytesRead, &httpFields) == B_ERROR_SUCCESS)
            break;

        playback_ip->statusCode = httpFields.statusCode;
        /* Error in HTTP parsing, check for parsing results */
        switch (httpFields.parsingResult) {
        case B_PlaybackIpHttpHdrParsingResult_eIncompleteHdr:
            BDBG_MSG(("%s: Haven't yet received complete HTTP message header (bytesRead %d), reading more data into buffer of size %d\n",
                        __FUNCTION__, bytesRead, rbuf_size));
            if (bytesRead == rbuf_size) {
                int newSize;
                BDBG_MSG(("%s: current buffer size (%d) not big enough to read complete HTTP response message, reallocing it by %d bytes to total of %d bytes\n",
                            __FUNCTION__, rbuf_size, TMP_BUF_SIZE, rbuf_size+TMP_BUF_SIZE));
                newSize = rbuf_size + TMP_BUF_SIZE;
                if (newSize > DLNA_MAX_HTTP_RESP_SIZE) {
                    BDBG_ERR(("%s: ERROR: Did not receive complete HTTP Header Response in max allowed size (max size %d, bytes read %d)\n", __FUNCTION__, newSize, bytesRead));
                    playback_ip->serverClosed = true;
                    return -1;
                }
                if ((rbuf = B_PlaybackIp_UtilsRealloc(rbuf, rbuf_size, newSize)) == NULL) {
                    BDBG_ERR(("%s: Failed to reallocate memory by %d amount, errno = %d\n", __FUNCTION__, newSize, errno));
                    playback_ip->serverClosed = true;
                    return -1;
                }
                rbuf_size = newSize;
                *rbufp = rbuf;
            }
            continue;
        case B_PlaybackIpHttpHdrParsingResult_eStatusBadRequest:
            BDBG_ERR(("%s: ERROR: Client send a bad HTTP request to server (status code %d)!!\n", __FUNCTION__, httpFields.statusCode));
            playback_ip->serverClosed = true;
            return -1;
        case B_PlaybackIpHttpHdrParsingResult_eStatusServerError:
            BDBG_ERR(("%s: ERROR: Server couldn't satisfy a valid HTTP Request (status code %d)!!\n", __FUNCTION__, httpFields.statusCode));
            playback_ip->serverClosed = true;
            return -1;
        case B_PlaybackIpHttpHdrParsingResult_eStatusNotSupported:
            BDBG_ERR(("%s: ERROR: Unsupported HTTP Message status code %d in HTTP Response!!\n", __FUNCTION__, httpFields.statusCode));
            playback_ip->serverClosed = true;
            return -1;
        case B_PlaybackIpHttpHdrParsingResult_eReadNextHdr:
            BDBG_ERR(("%s: Received HTTP 1xx status code (%d), going back to receiving next response\n", __FUNCTION__, httpFields.statusCode));
            goto read_again;
        case B_PlaybackIpHttpHdrParsingResult_eStatusRedirect:
            BDBG_MSG(("%s: Received HTTP redirect (status code %d) in HTTP Response!!\n", __FUNCTION__, httpFields.statusCode));
            playback_ip->serverClosed = false;
            *http_hdr = httpFields.httpHdr;
            *http_payload = httpFields.httpPayload;
            /* caller will take the necessary steps to handle the redirection */
            return 0;
        case B_PlaybackIpHttpHdrParsingResult_eIncorrectHdr:
        default:
            BDBG_ERR(("%s: received Incorrect/Invalid Header\n", __FUNCTION__));
            playback_ip->serverClosed = true;
            return -1;
        }
    }

#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog) {
        char *tmpPtr;
        if ( (tmpPtr = strstr(rbuf, "\n\r\n")) || /* covers both CRNL CRNL & NL NL cases */
             (tmpPtr = strstr(rbuf, "\n\n")) ||   /* covers NL NL case */
             (tmpPtr = strstr(rbuf, "\r\r"))      /* covers CR CR case */
           ) {
            char tmpChar = *tmpPtr;
            *tmpPtr = '\0';
            BDBG_WRN(("HTTP response:"));
            fprintf(stdout, "%s", rbuf);
            fflush(stdout);
            *tmpPtr = tmpChar;
        }
    }
#endif
    if (httpFields.parsingResult != B_PlaybackIpHttpHdrParsingResult_eSuccess)  {
        BDBG_ERR(("%s: received Incorrect/Invalid Reponse Header\n", __FUNCTION__));
        if (!playback_ip->selectTimeout)
            playback_ip->serverClosed = true;
        return -1;
    }

    /* Complete HTTP Message Header is found */
    playback_ip->statusCode = httpFields.statusCode;
    *http_hdr = httpFields.httpHdr;
    *http_payload = httpFields.httpPayload;
    initial_payload_len = httpFields.httpPayloadLength;
    playback_ip->chunkEncoding = httpFields.chunkEncoding;
    if (!playback_ip->contentLength) {
        if (httpFields.contentLength)
            playback_ip->contentLength = httpFields.contentLength;
        else if (playback_ip->setupSettings.u.http.contentLengthHint)
            playback_ip->contentLength = playback_ip->setupSettings.u.http.contentLengthHint;
    }
    BDBG_MSG(("%s:%p Content Length %"PRId64 ", initial payload len %d, bytesRead %d\n", __FUNCTION__, (void *)playback_ip, playback_ip->contentLength, initial_payload_len, bytesRead));

    /* adjust the total bytes read */
    playback_ip->totalRead -= httpFields.httpPayload - httpFields.httpHdr;
    *payload_len = initial_payload_len;

    /*
    * If server is using HTTP Chunked Transfer-Encoding to send the content,
    * we will need to skip the chunk headers also from the payload pointer.
    */
    if (playback_ip->chunkEncoding) {
        /* if we haven't read any initial payload data or read data doesn't yet contain CRNL (which marks end of chunk size) */
        /* then read another 32 bytes as it must contain the chunk size */
        rbuf[bytesRead] = '\0';
        while (!initial_payload_len || ((tmp_ptr = strstr(*http_payload, "\r\n")) == NULL)) {
            if (bytesRead >= rbuf_size) {
                BDBG_ERR(("%s: ERROR: Initial read of %d bytes is not BIG enough to read chunk size header, increase it from %d",
                    __FUNCTION__, bytesRead, rbuf_size));
                playback_ip->serverClosed = true;
                return -1;
            }
            if (_http_socket_select(playback_ip, fd, playback_ip->networkTimeout)) {
                BDBG_ERR(("%s: select ERROR:%d, but continue\n", __FUNCTION__, errno));
                return -1;
            }

            if ((rc = playback_ip->netIo.read(securityHandle, playback_ip, fd, rbuf+bytesRead, rbuf_size - bytesRead)) <= 0) {
                BDBG_ERR(("%s: read failed: rc %d", __FUNCTION__, rc));
                if (errno == EINTR || errno == EAGAIN) {
                    playback_ip->selectTimeout = true;
                    BDBG_ERR(("%s: read ERROR:%d, selectTimeout is set to true\n", __FUNCTION__, errno));
                }
                else
                    playback_ip->serverClosed = true;
                return -1;
            }
            BDBG_MSG(("%s: read %d bytes, bytesRead %d, initial_payload_len %d\n", __FUNCTION__, rc, bytesRead, initial_payload_len));
            playback_ip->totalRead += rc;
            bytesRead += rc;
            initial_payload_len += rc;
            rbuf[bytesRead] = '\0';
        }
        BDBG_MSG(("%s: initial payload length including chunk size header %d\n", __FUNCTION__, initial_payload_len));
        /*
         * So we have found the valid chunk-size header, now read the chunk size. It is format:
         *  <ascii hex string indicating chunk size> <CR> <NL>
         */
        if (playback_ip->totalRead)
            playback_ip->totalRead -= 2;
        *tmp_ptr = '\0';
        playback_ip->chunkSize = strtol(*http_payload, (char **)NULL, 16);    /* base 16 */
        if (playback_ip->chunkSize == 0) {
            BDBG_MSG(("%s: received 0 chunk size in response, there is no data at the requested range, return EOF\n", __FUNCTION__));
            return 0;
        }

        *tmp_ptr = '\r';
        chunk_hdr_size = (tmp_ptr - *http_payload) + strlen("\r\n");
        initial_payload_len -= chunk_hdr_size;
        *http_payload += chunk_hdr_size;
        playback_ip->totalRead -= chunk_hdr_size;
        playback_ip->chunkEncoding = true;

        /* chunk encoding case: if DTCP encryption is being used, initial payload is now fed to the dtcp module, else it is passed to the caller. */
        /* Since for some security protocols, initial HTTP response can be in clear, now indicate to security layer */
        /* that HTTP header processing is over and it can now start decrypting the incoming data */
        /* Note: payload_len becomes 0 for DTCP as this data is handled over the dtcp_ip layer for later decryption during 1st read call */
        if ((rc = B_PlaybackIp_SecurityDecryptionEnable(playback_ip, *http_payload, &initial_payload_len)) != 0) {
            BDBG_ERR(("%s: Failed (rc %d) for Enabled Decryption for security protocol %d:", __FUNCTION__, rc, playback_ip->openSettings.security.securityProtocol));
            playback_ip->serverClosed = true;
            return -1;
        }

        if (initial_payload_len != 0) {
            /* Security layer didn't consume the data: either security module provides clear data only (complete transport encryption including chhunk headers) or its clear data */
            /* Since chunk xfer encoding is being used, we process initial data part of the chunk header processing, so copy this data and set appropriate state */
            memcpy(playback_ip->temp_buf, *http_payload, initial_payload_len);
            playback_ip->chunkPayloadLength = initial_payload_len;
            playback_ip->chunkPlayloadStartPtr = playback_ip->temp_buf;
        }
        else {
            /* initial_payload_len is made 0, so either no initial data was read or was handled over to the security module for later decryption during next read call */
            playback_ip->chunkPayloadLength = 0;
        }
        /* now set the bytes left in the chunk: we start w/ the current chunk size */
        playback_ip->bytesLeftInCurChunk = playback_ip->chunkSize;

        /* return 0 payload_len as for chunk encoding all initial data is either handled over to the security layer or handled via the chunkPlayloadStartPtr */
        *payload_len = 0;

        BDBG_MSG(("%s: chunk size %"PRId64 ", chunk header size %d, initial payload size %d, bytesLeftInCurChunk %"PRId64 "\n",
                    __FUNCTION__, playback_ip->chunkSize, chunk_hdr_size, playback_ip->chunkPayloadLength, playback_ip->bytesLeftInCurChunk));
    }
    else {
        /* non-chunk encoding case: if DTCP encryption is being used, initial payload is now fed to the dtcp module, else it is passed to the caller. */
        /* Since for some security protocols, initial HTTP response can be in clear, now indicate to security layer */
        /* that HTTP header processing is over and it can now start decrypting the incoming data */
        /* Note: payload_len becomes 0 for DTCP as this data is handled over the dtcp_ip layer for later decryption during 1st read call */
        if ((rc = B_PlaybackIp_SecurityDecryptionEnable(playback_ip, *http_payload, payload_len)) != 0) {
            BDBG_ERR(("%s: Failed (rc %d) for Enabled Decryption for security protocol %d:", __FUNCTION__, rc, playback_ip->openSettings.security.securityProtocol));
            playback_ip->serverClosed = true;
            return -1;
        }
    }

#ifdef B_HAS_NETACCEL1
    /* disabling using NETACCEL for HTTP as there aren't real gains + it is not working w/ 2.6.37 */
    /* now that we have read the HTTP response, increase the # of pkts per read */
    memset((char *)&sockRecvParams, 0, sizeof(sockRecvParams));
    sockRecvParams.pktsPerRecv = PKTS_PER_READ;
    sockRecvParams.pktOffset = 0;
    sockRecvParams.hdrOffset = 0;
    sockRecvParams.recvTimeout = 100;
    sockRecvParams.useCpuCopy = true;
    if (setsockopt(fd, SOCK_BRCM_STREAM, STRM_SOCK_RECV_PARAMS, &sockRecvParams, sizeof(sockRecvParams)) != 0)
    {
        BDBG_ERR(("%s: setsockopt() ERROR:", __FUNCTION__));
        /* in case of failure (shouldn't happen), read 1 pkt at a time */
    }
    BDBG_MSG(("Modified the default pkts per recvfrom to %d\n", PKTS_PER_READ));
#else
    BDBG_MSG(("%s:%p tuning socket", __FUNCTION__, (void *)playback_ip));
    B_PlaybackIp_UtilsTuneNetworkStack(fd);
#endif
    return (0);
}

static void
reset_playback_ip_state(B_PlaybackIpHandle playback_ip)
{
    playback_ip->chunkEncoding = false;
    playback_ip->serverClosed = false;
    playback_ip->reOpenSocket = false;
    playback_ip->socketClosed = false;
    playback_ip->chunkSize = 0;
    playback_ip->bytesLeftInCurChunk = 0;
    playback_ip->totalRead = 0;
    playback_ip->totalConsumed = 0;
    playback_ip->chunkPayloadLength = 0;
    playback_ip->rewind = false;
    playback_ip->statusCode = 0;
    playback_ip->tempBytesRead = 0;
    playback_ip->tempBytesToRead = 0;
    memset(playback_ip->temp_buf, 0, TMP_BUF_SIZE);
    playback_ip->num_chunks_received = 0;
}

/*
 * This is the high level function to read any data from a socket of a HTTP session. It is called by multiple upper
 * layer modules such as HTTP caching logic (which is used when data is being fed thru the nexus playback), direct
 * nexus playpump based feeder (which is used when server side trickmodes are being used), HLS & DASH Players, etc.
 *
 * It tries to read data requested amount of data from a network socket into the caller specified buffer.
 * It takes care of HTTP Chunk Transfer Encoding protocols where it removes the Chunk headers & trailers from the AV
 * stream (which are inter-mixed in the stream). In addition, it can also take care of decryption of data if it is
 * part of encrypted sessions for the support security protocols (DTCP, HLS-AES, SSL).
 *
 * Return values:
 *   >  0: # of bytes upto buf_size
 *   == 0: server has closed the connection
 *   <  0: network error (caller needs to use the selectTimeout flag to check if the failure is due to temporary socket timeout)
 *
 * Note: function sets following two playback_ip state that callers need to be aware of: serverClosed & selectTimeout
 *   serverClosed: when server has closed the connection (either has sent the whole file, reached EOF, and thus closed the socket)
 *   selectTimeout: when there is no data to read on the socket for networkTimeout duration (caller may retry)
 *
 */
int
playback_ip_read_socket(
    B_PlaybackIpHandle playback_ip,
    void *securityHandle,
    int fd,
    char *buf,
    int buf_size,
    int timeout      /* indicates the timeout in seconds for network events */
    )
{
    int bytesRead = 0;
    int totalBytesRead = 0;
    int bytesToRead = 0;
    char *chunk_hdr_ptr = NULL;
    int chunk_hdr_size;
    char *tmp_ptr;

    PRINT_SOCKET_MSG(("%s:%p read %d bytes", __FUNCTION__, (void *)playback_ip, buf_size));
    playback_ip->selectTimeout = false;
    if (!playback_ip->chunkEncoding) {
        /* server is sending content w/o using HTTP xfer chunk encoding */
        bytesToRead = buf_size;
        if (http_socket_read(playback_ip, securityHandle, fd, buf, bytesToRead, &bytesRead, timeout) < 0) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_ERR(("%s: ERROR:%d, state %d, select timeout %d, server closed %d", __FUNCTION__, errno, playback_ip->playback_state, playback_ip->selectTimeout, playback_ip->serverClosed));
#endif
            return -1;
        }
        if (bytesRead != bytesToRead) {
            if ( breakFromLoop(playback_ip)) {
                /* can happen in some cases where http_socket_read() takes long time to timeout and app has already decided to close the playback channel */
                return -1;
            }
            /* otherwise, we should consume this data even though we didn't read all that we had wanted */
            BDBG_MSG(("%s:%d: didn't read (%d) the requested amount (%d) from socket, server closed %d", __FUNCTION__, __LINE__, bytesRead, bytesToRead, playback_ip->serverClosed));
        }
        return bytesRead;
    }

    /*
     * Server is sending content using HTTP xfer chunk encoding. Chunk headers are interspersed with data.
     * Format is:
     *  <HTTP Resp Hdr>
     *  <chunk size string>CRNL
     *  <chunk data>CRNL
     *  <chunk size string>CRNL
     *  <chunk data>CRNL
     *  ...
     *
     *  <0>CRNL #indicates end of chunk headers
     * Our approach is like this: Read from socket until current chunk is completely read.
     * Read the next chunk header and then the content again.
     * Loop until all requested bytes are read or n/w error happens.
     */
    while (totalBytesRead < buf_size && playback_ip->serverClosed == false) {
        if ( breakFromLoop(playback_ip)) {
            /* user changed the channel, so return */
            BDBG_MSG(("%s: breaking out of while loop due to state (%d) change, totalBytesRead %d\n", __FUNCTION__, playback_ip->playback_state, totalBytesRead));
            if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode) {
                playback_ip->selectTimeout = true;
                errno = EINTR;
            }
            else {
                playback_ip->serverClosed = true;
            }
            return totalBytesRead;
        }
        if (playback_ip->bytesLeftInCurChunk) {
            /*
            * There are still bytes left in the current chunk, so read that much data first.
            * This data can either be pre-read in the temp buf or needs to be read from socket.
            */
            bytesToRead = buf_size - totalBytesRead;
            if (playback_ip->chunkPayloadLength) {
                /* copy lower of remaining bytes or what was read in the temp buffer to caller's buffer */
                bytesToRead = (bytesToRead <= playback_ip->chunkPayloadLength) ? bytesToRead : playback_ip->chunkPayloadLength;
                if (bytesToRead > playback_ip->bytesLeftInCurChunk) {
                    BDBG_MSG(("bytesToRead %d > bytesLeftInChunk %"PRId64 ", chunkPayloadLength %d",
                            bytesToRead, playback_ip->bytesLeftInCurChunk, playback_ip->chunkPayloadLength));
                    bytesToRead = playback_ip->bytesLeftInCurChunk;
                }
                /* remaining bytes are available in the temp buffer, copy them and return */
                BDBG_MSG(("%s: copying %d bytes from temp buffer, bytes remaining in current chunk %"PRId64 "\n",
                            __FUNCTION__, bytesToRead, playback_ip->bytesLeftInCurChunk));
                memcpy(buf, playback_ip->chunkPlayloadStartPtr, bytesToRead);
                bytesRead = bytesToRead;
                playback_ip->chunkPlayloadStartPtr += bytesRead;
                playback_ip->chunkPayloadLength -= bytesRead;
                if (playback_ip->chunkPayloadLength < 0) {
                    BDBG_ERR(("%s: ERROR: SW bug: remaining bytes (%d) in temp buf can't be < 0, copied %d\n",
                                __FUNCTION__, playback_ip->chunkPayloadLength, bytesRead));
                    return -1;
                }
            }
            else {
                /* no bytes left in the temp buffer, so read from the socket */
                /* determine how much to read: read lower of the remaining data in current request or remaining data in current chunk */
                if (bytesToRead > playback_ip->bytesLeftInCurChunk) {
                    bytesToRead = playback_ip->bytesLeftInCurChunk;
                    /* since bytesToRead are now based on the bytes left in the current chunk, set a flag to indicate so */
                    /* as we may need to adjust bytesToRead if PCP header is found for DTCP */
                    BDBG_MSG(("%s: set flag to indicate that we may need to adjust bytesToRead if PCP header is found for DTCP", __FUNCTION__));
                    playback_ip->adjustBytesToRead = true;
                }
                else {
                    playback_ip->adjustBytesToRead = false;
                }

                BDBG_MSG(("%s: reading %d bytes from socket, total read so far %d, total asked %d, bytesLeftInCurChunk %"PRId64 "",
                            __FUNCTION__, bytesToRead, totalBytesRead, buf_size, playback_ip->bytesLeftInCurChunk));
                if (http_socket_read(playback_ip, securityHandle, fd, buf, bytesToRead, &bytesRead, timeout) < 0) {
                    BDBG_MSG(("%s: errno :%d, bytes left in chunk %"PRId64 ",total read so far %d\n", __FUNCTION__, errno, playback_ip->bytesLeftInCurChunk, totalBytesRead));
                    if (totalBytesRead > 0)
                        return totalBytesRead;
                    else
                        return -1;
                }
                if (bytesRead != bytesToRead && playback_ip->serverClosed == false) {
                    BDBG_MSG(("%s:%d didn't read (%d) the requested amount (%d) from socket, select timeout %d, bytes left in chunk %"PRId64 ", chunk size %"PRId64 "\n",
                                __FUNCTION__, __LINE__, bytesRead, bytesToRead, playback_ip->selectTimeout, playback_ip->bytesLeftInCurChunk, playback_ip->chunkSize));
                }
            }
            totalBytesRead += bytesRead;
            buf += bytesRead;
            playback_ip->bytesLeftInCurChunk -= bytesRead;
            if (playback_ip->selectTimeout || playback_ip->serverClosed) {
                BDBG_MSG(("%s:%d select timeout(%d), server closed(%d): returning read (%d), the requested amount (%d), bytes left in chunk %"PRId64 "\n",
                           __FUNCTION__, __LINE__, playback_ip->selectTimeout, playback_ip->serverClosed, totalBytesRead, bytesToRead, playback_ip->bytesLeftInCurChunk));
                if (totalBytesRead > 0)
                    return totalBytesRead;
                else
                    return -1;
            }
            if (playback_ip->bytesLeftInCurChunk < 0) {
                BDBG_ERR(("%s:%d ERROR: SW bug: bytes left in a chunk (%"PRId64 ") can't be < 0, just read %d, totalRead %d, total asked %d\n",
                            __FUNCTION__, __LINE__, playback_ip->bytesLeftInCurChunk, bytesRead, totalBytesRead, buf_size));
                return -1;
            }
            BDBG_MSG(("%s: after update: chunk size %"PRId64 ", bytes left in chunk %"PRId64 ", bytes read %d, to read %d, totalBytesRead %d", __FUNCTION__, playback_ip->chunkSize, playback_ip->bytesLeftInCurChunk, bytesRead, bytesToRead, totalBytesRead));
            continue;
        }
        else {
            /* bytesLeftInCurChunk == 0 case */
            /*
            * So we finished reading the current chunk. Now read the next chunk header.
            * The extra data read while reading the chunk header is stored in the temp_buf.
            */
            if (playback_ip->chunkPayloadLength > 0) {
                /* there is still some data left in the temporary buffer when previous chunk header was read into */
                /* so skip the network read and instead move these bytes into the begining of the buffer */
                memmove(playback_ip->temp_buf, playback_ip->chunkPlayloadStartPtr, playback_ip->chunkPayloadLength);
                bytesRead = playback_ip->chunkPayloadLength;
                /* clear the remaining bytes of the tempBuf */
                memset(playback_ip->temp_buf+bytesRead, 0, TMP_BUF_SIZE-bytesRead);
                playback_ip->tempBytesToRead = TMP_BUF_SIZE; /* start w/ total tmp buf size and it gets adjusted by bytesRead below after the else case */
                playback_ip->chunkPayloadLength = 0;
                BDBG_MSG(("%s: Finished reading current data chunk, next chunk header may already be previously read (read so far %d, total %d), # of chunk rcvd %d\n", __FUNCTION__, bytesRead, totalBytesRead, playback_ip->num_chunks_received));
                playback_ip->num_chunks_received++;
            }
            else {
                BDBG_MSG(("%s: Finished reading current data chunk, now read next chunk header (total read so far %d), # of chunk rcvd %d\n", __FUNCTION__, totalBytesRead, playback_ip->num_chunks_received));
                if (playback_ip->tempBytesRead == 0) {
                    /* we haven't read anything so far for this new chunk, so try to read enough to process a complete chunk header */
                    playback_ip->tempBytesToRead = TMP_BUF_SIZE;
                    memset(playback_ip->temp_buf, 0, TMP_BUF_SIZE);
                    playback_ip->num_chunks_received++;
                }
                else {
                    /* we have read tempBytesRead so far for this new chunk, so try to read enough to process a complete chunk header */
                    memset(playback_ip->temp_buf+playback_ip->tempBytesRead, 0, TMP_BUF_SIZE-playback_ip->tempBytesRead);
                    playback_ip->num_chunks_received++;
                    BDBG_MSG(("tempBytesRead %d, tempBytesToRead %d", playback_ip->tempBytesRead, playback_ip->tempBytesToRead));
                }

                /* Since some security protocols like DTCP/IP only encrypt the AV content, we will need to disable the decryption */
                /* so that we can successfully read the current chunk header terminators & next chunk headers (which are both in clear) */
                if (B_PlaybackIp_SecurityDecryptionDisable(playback_ip) != 0) {
                    BDBG_ERR(("%s: Failed to Disable the Decryption during next chunk xfer header processing", __FUNCTION__));
                    return -1;
                }

                /* Note: we are reading into a temp buffer as we dont want to pass in the chunk header fields (control data) back to the playback */
                if (http_socket_read(playback_ip, securityHandle, fd, playback_ip->temp_buf+playback_ip->tempBytesRead, playback_ip->tempBytesToRead, &bytesRead, timeout) <= 0) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                    BDBG_ERR(("%s: read ERROR:%d", __FUNCTION__, errno));
#endif
                    if (totalBytesRead > 0)
                        return totalBytesRead;
                    else
                        return -1;
                }
            }
            if (bytesRead + playback_ip->tempBytesRead < 5) {
                /* unless we read minimum of 5 bytes, we shouldn't do any chunk header processing, so save these bytes and read more (this can most likely happen due to network congestion shown by select timeout) */
                BDBG_MSG(("%s:%d : didn't read (%d) the requested amount (%d) from socket, select timeout %d\n", __FUNCTION__, __LINE__, bytesRead, playback_ip->tempBytesToRead, playback_ip->selectTimeout));
                playback_ip->tempBytesToRead -= bytesRead;
                playback_ip->tempBytesRead += bytesRead;
                playback_ip->selectTimeout = true;
                /* we return here as we expect to read the remaining bytesToRead whenever the next read call may succeed */
                if (totalBytesRead > 0)
                    return totalBytesRead;
                else
                    return -1;
            }
            else {
                /* we have read atleast 5 bytes */
                /* First, make sure the end of chunk marker (CRNL) for previous chunk are present  as each chunk data ends w/ CRNL */
                if (playback_ip->temp_buf[0] != '\r' || playback_ip->temp_buf[1] != '\n') {
                    BDBG_ERR(("%s: ERROR in HTTP chunk header processing: server didn't send End Markers (CR NL) of current data chunk, instead we got: 0x%x, 0x%x\n",
                        __FUNCTION__, playback_ip->temp_buf[0], playback_ip->temp_buf[1]));
                    if (totalBytesRead > 0)
                        return totalBytesRead;
                    else
                        return -1;
                }
                /* now check if next chunk header is completely read, meaning we have read a string w/ some hex digits (atleast one 0) followed by CRNL markers */
                if (strstr(&playback_ip->temp_buf[2], "\r\n") == 0) {
                    BDBG_MSG(("%s: End Markers for next Chunk Header are not yet read, go back & read more (bytesRead %d)\n", __FUNCTION__, bytesRead));
                    playback_ip->tempBytesToRead -= bytesRead;
                    playback_ip->tempBytesRead += bytesRead;
                    if (playback_ip->tempBytesRead == TMP_BUF_SIZE) {
                        /* if we haven't found the end of chunk markers in TMP_BUF_SIZE bytes, there is an error in chunk processing, bail out */
                        BDBG_ERR(("%s: ERROR: End Markers (CRNL) for next Chunk Header are not found in %d bytes\n", __FUNCTION__, playback_ip->tempBytesRead));
                        return -1;
                    }
                    /* we return here as we expect to read the remaining bytesToRead whenever the next read call may succeed */
                    playback_ip->selectTimeout = true;
                    if (totalBytesRead > 0)
                        return totalBytesRead;
                    else
                        return -1;
                }
                BDBG_MSG(("%s: Chunk Header Start End Markers are read, so process the chunk header (bytes read %d, temp read %d)\n", __FUNCTION__, bytesRead, playback_ip->tempBytesRead));

                /* adjust total bytes read */
                playback_ip->tempBytesRead += bytesRead;

                /* at this point we have gotten end markers for the previous chunk & next chunk header as well, so do the math to account for them */
                playback_ip->chunkPayloadLength = playback_ip->tempBytesRead - 2; /* 2 bytes for CRNL of previous chunk end */
                chunk_hdr_ptr = &playback_ip->temp_buf[2];
                if (playback_ip->totalRead)
                    playback_ip->totalRead -= 2;
                playback_ip->tempBytesRead = 0; /* resets temp bytes read  for processing of next chunk header */

                /* now get the size of next chunk, format is <hex string of chunk size>CRNL  */
                tmp_ptr = strstr(chunk_hdr_ptr, "\r\n");
                /* tmp_ptr can't be NULL as we have already verified above that CRNL are in the read buffer */
                BDBG_ASSERT(tmp_ptr);
                *tmp_ptr = '\0';
                playback_ip->chunkSize = strtol(chunk_hdr_ptr, (char **)NULL, 16); /* base 16 */
                *tmp_ptr = '\r';
                chunk_hdr_size = (tmp_ptr - chunk_hdr_ptr) + strlen("\r\n");
                playback_ip->chunkPayloadLength -= chunk_hdr_size;
                playback_ip->chunkPlayloadStartPtr = chunk_hdr_ptr + chunk_hdr_size;
                playback_ip->totalRead -= chunk_hdr_size;
                if (playback_ip->chunkSize == 0) {
                    playback_ip->serverClosed = true;
                    playback_ip->selectTimeout = false;
                    BDBG_MSG(("%s: received chunk size of 0, received all data (%"PRId64 ") from server, setting serverClosed flag\n", __FUNCTION__, playback_ip->totalRead));
                    /* NOTE: we cannot set the contentLength to totalRead here as for trick modes or ASF playback case, we may be getting EOF even though we haven't read whole file */
                }
                else {
                    /* now re-enable the decryption */
#ifdef B_HAS_DTCP_IP
                    char *tempBuf = NULL;
                    if (playback_ip->encryptedBufLength > 0) {
                        /* we have encrypted bytes from previous chunk that we need to also pass to the dtcp security module */
                        /* so we combine the encrypted data from previous chunk and the start of the new chunk */
                        int tempBufLen = playback_ip->encryptedBufLength + playback_ip->chunkPayloadLength ;
                        tempBuf = (char *)BKNI_Malloc(tempBufLen);
                        if (tempBuf == NULL) {
                            BDBG_ERR(("%s: BKNI_Malloc() Failed to allocate %d bytes for copying the encrypted payload chunks", __FUNCTION__, tempBufLen));
                            return -1;
                        }
                        memcpy(tempBuf, playback_ip->encryptedBuf, playback_ip->encryptedBufLength);
                        memcpy(tempBuf+playback_ip->encryptedBufLength, playback_ip->chunkPlayloadStartPtr, playback_ip->chunkPayloadLength);
                        playback_ip->chunkPayloadLength = tempBufLen;
                        playback_ip->chunkPlayloadStartPtr = tempBuf;
                        BDBG_MSG(("%s: combined the encrypted payloads: bytes from prev chunk %d, cur chunk %d ", __FUNCTION__, playback_ip->encryptedBufLength, playback_ip->chunkPayloadLength));
                    }
#endif
                    if (B_PlaybackIp_SecurityDecryptionEnable(playback_ip, playback_ip->chunkPlayloadStartPtr, &playback_ip->chunkPayloadLength) != 0) {
                        BDBG_ERR(("%s: Failed for Enabled Decryption for security protocol %d:", __FUNCTION__, playback_ip->openSettings.security.securityProtocol));
                        return -1;
                    }
                    playback_ip->bytesLeftInCurChunk = playback_ip->chunkSize; /* start of new chunk */
#ifdef B_HAS_DTCP_IP
                    if (playback_ip->encryptedBufLength > 0) {
                        if (tempBuf)
                            BKNI_Free(tempBuf);
                        /* since we had combined some encrypted bytes from the previous chunk, we need to update the bytes left & size of this chunk by this amount */
                        playback_ip->bytesLeftInCurChunk += playback_ip->encryptedBufLength;
                        playback_ip->chunkSize += playback_ip->encryptedBufLength;
                        playback_ip->encryptedBufLength = 0;
                    }
#endif
                    BDBG_MSG(("%s: read new chunk: chunk size %"PRId64 ", chunk header size %d, initial payload size %d, bytesLeftInCurChunk %"PRId64 "\n",
                        __FUNCTION__, playback_ip->chunkSize, chunk_hdr_size, playback_ip->chunkPayloadLength, playback_ip->bytesLeftInCurChunk));
                }
            }
        }
    }
    if (totalBytesRead > buf_size) {
        BDBG_ERR(("%s: ERROR: SW bug: read bytes (%d) are more than total requested (%d), bytes remaining in current chunk %"PRId64 "",
                __FUNCTION__, totalBytesRead, buf_size, playback_ip->bytesLeftInCurChunk));
    }
    BDBG_MSG(("%s: bytes read %d, asked %d, remaining in cur chunk %"PRId64 ", total read %"PRId64 "\n",
            __FUNCTION__, totalBytesRead, buf_size, playback_ip->bytesLeftInCurChunk, playback_ip->totalRead));
    return totalBytesRead;
}

B_PlaybackIpError
http_do_server_trick_modes_socket_transition(
    B_PlaybackIpHandle playback_ip,
    double timeSeekRangeBegin,
    double timeSeekRangeEnd,
    int rate,
    char *playSpeedString
    )
{
    B_PlaybackIpError rc = B_ERROR_SUCCESS;
    void *securityHandle;
    char *http_hdr = NULL;
    char *http_payload = NULL;
    int http_payload_length;
    socklen_t addrLen;

#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: rate %d, timeSeekRangeBegin %0.3f, timeSeekRangeEnd %0.3f, state %d", __FUNCTION__, rate, timeSeekRangeBegin, timeSeekRangeEnd, playback_ip->playback_state));
#endif
    /*
     * Here are the high level steps:
     * -prepare & send new HTTP request for server w/ new rate (speed) and time position
     * -process the HTTP response
     */

    /* shutdown the current TCP connection, this way we dont need to worry about having any stale data from previous speed */
    if (playback_ip->netIo.shutdown)
        playback_ip->netIo.shutdown(playback_ip->securityHandle, 0);
    close(playback_ip->socketState.fd);
    BDBG_MSG(("%s:%p: closed fd=%d", __FUNCTION__, (void *)playback_ip, playback_ip->socketState.fd));

    /* prepare the HTTP Get Request with new speed & time seek range */
    memset(playback_ip->responseMessage, 0, TMP_BUF_SIZE+1);

    /* note: range parameters are set to 0 as they are un-unsed in server based trick modes */
    if (http_build_get_req(&playback_ip->requestMessage, playback_ip, rate, timeSeekRangeBegin, timeSeekRangeEnd, 0, 0, playSpeedString) < 0) {
        BDBG_ERR(("%s: ERROR: failed to build HTTP Get Request", __FUNCTION__));
        goto error;
    }

    /* now open the new socket connection to the server */
    rc = B_PlaybackIp_UtilsTcpSocketConnect(&playback_ip->playback_state, playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port, true, playback_ip->networkTimeout, &playback_ip->socketState.fd);
    if (rc != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: ERROR: failed to send Socket Connect Request to Server: %s:%d\n",
               __FUNCTION__, playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port));
        goto error;
    }
    addrLen = sizeof(playback_ip->socketState.local_addr);
    if (getsockname(playback_ip->socketState.fd, (struct sockaddr *)&playback_ip->socketState.local_addr, (socklen_t *)&addrLen) != 0) {
        BDBG_ERR(("ERROR: Failed to obtain connection socket address, errno: %d \n", errno));
        perror("getsockname");
        goto error;
    }

    BDBG_MSG(("%s: new fd=%d local ip:port = %s:%d", __FUNCTION__, playback_ip->socketState.fd, inet_ntoa(playback_ip->socketState.local_addr.sin_addr), ntohs(playback_ip->socketState.local_addr.sin_port)));

    /* SECURITY: clone the new security session using the current one */
    /* Idea is to avoid any re-authentication to the server and instead reuse the session info from the current session */
    securityHandle = playback_ip->securityHandle;
    if (B_PlaybackIp_SecurityCloneSessionOpen(playback_ip, playback_ip->socketState.fd, securityHandle, &playback_ip->securityHandle) < 0) {
        rc = B_ERROR_UNKNOWN;
        BDBG_ERR(("%s: ERROR: failed to clone the security session, rc %d\n", __FUNCTION__, rc));
        goto error;
    }
    /* free up the older security session */
    playback_ip->netIo.close(securityHandle);

    /* now send the GET request */
    if (playback_ip->netIo.write(playback_ip->securityHandle, &playback_ip->playback_state, playback_ip->socketState.fd, playback_ip->requestMessage, strlen(playback_ip->requestMessage)) < 0) {
        rc = B_ERROR_UNKNOWN;
        BDBG_ERR(("%s: ERROR: failed to send HTTP Get request to Server: %s:%d\n", __FUNCTION__, playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port));
        goto error;
    }
    BDBG_MSG(("%s: HTTP Get Request--->: \n%s", __FUNCTION__, playback_ip->requestMessage));
    BKNI_Free(playback_ip->requestMessage);
    playback_ip->requestMessage = NULL;

    /* reset the playback ip state as we starting from a new point in stream */
    reset_playback_ip_state(playback_ip);

    /* now read the http response */
    /* RVU TODO: may need to parse the FrameMap in the RVU response for exact frame time and use it for next time seek range */
    if (http_read_response(playback_ip, playback_ip->securityHandle, playback_ip->socketState.fd, &playback_ip->responseMessage, TMP_BUF_SIZE, &http_hdr, &http_payload, &http_payload_length) < 0 ) {
        rc = B_ERROR_UNKNOWN;
        BDBG_ERR(("%s: ERROR: failed to get the proper response for the trickmode request, rc %d\n", __FUNCTION__, rc));
        goto error;
    }
    if (playback_ip->openSettings.u.http.rvuCompliant) {
        /* check for end of stream header from the server */
        if (strstr(playback_ip->responseMessage, "eom-indicator.rvualliance.org:")) {
            BDBG_MSG(("%s: Got End of Media Indicator from RVU Server, closing session", __FUNCTION__));
            playback_ip->serverClosed = true;
            rc = B_ERROR_UNKNOWN;
            goto error;
        }
    }
    if (!playback_ip->useNexusPlaypump) {
        /* re-initialze the cache for pull mode */
        memcpy(playback_ip->dataCache[0].cache, http_payload, http_payload_length);
        playback_ip->dataCache[0].startOffset = 0;
        playback_ip->dataCache[0].readIndex = 0;
        playback_ip->dataCache[0].endOffset = playback_ip->dataCache[0].startOffset - 1;
        playback_ip->dataCache[0].depth = http_payload_length;
        playback_ip->dataCache[0].writeIndex = http_payload_length;
        playback_ip->dataCache[0].inUse = true;
    }
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        /* TODO: is http_payload_length amount of data getting lost? should it be in the playback_ip->initial_data_len? Look into this */
        BDBG_WRN(("%s: done, initial bytes read %d\n", __FUNCTION__, http_payload_length));
#endif
    rc = B_ERROR_SUCCESS;
    BDBG_MSG(("%s:%p: returning fd=%d", __FUNCTION__, (void *)playback_ip, playback_ip->socketState.fd));
error:
    if (playback_ip->requestMessage) {
        BKNI_Free(playback_ip->requestMessage);
        playback_ip->requestMessage = NULL;
    }
    return rc;
}

/* This code runs in following 3 cases: normal rewind, separate connection for index when it is at end (e.g. for some MP4 & ASF) */
/* and 3rd case is when we seek to a way different offset where it doesn't make sense to keep receiving data from current location */
ssize_t
B_PlaybackIp_HttpNetRangeReq(
    B_PlaybackIpHandle playback_ip,
    void *buf,
    size_t length,
    off_t byteRangeStart,
    off_t byteRangeEnd,
    int prevFd,
    int *newFd
    )
{
    unsigned int bytesRead = 0;
    ssize_t rc = -1;
    char *http_hdr = NULL;
    char *http_payload = NULL;
    int initialLen = 0;
    void *securityHandle = playback_ip->securityHandle;
    void **newSecurityHandle = &playback_ip->securityHandle;
    char *requestMessage = NULL, *responseMessage;
    int loopCnt = 0;
    http_url_type_t http_url_type;

#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog)
        BDBG_WRN(("%s: read %zu bytes of content data at range start %"PRIu64 " end %"PRId64 ", prevFd %d, fd %d, securityHandle %p", __FUNCTION__, length, byteRangeStart, byteRangeEnd, prevFd, playback_ip->socketState.fd, (void *)securityHandle));
#endif

    if (byteRangeEnd) {
        size_t validRangeLength;
        validRangeLength = byteRangeEnd - byteRangeStart + 1;
        if (validRangeLength < length) length = validRangeLength;
    }

    /* prepare initial Get request */
    responseMessage = (char *) BKNI_Malloc(TMP_BUF_SIZE+1);
    if (!responseMessage) {
        BDBG_ERR(("%s: ERROR: failed to allocate memory\n", __FUNCTION__));
        goto error;
    }
    for (;;) {
        memset(responseMessage, 0, TMP_BUF_SIZE+1);

        if (!playback_ip->socketClosed && prevFd) {
            /* if security layer has defined the shutdown callback, we call it to start the shutdown of the security handle */
            /* this basically tells security module to free up an security context w/o destroying the ability to clone it */
            if (playback_ip->netIo.shutdown && !playback_ip->serverClosed)
                playback_ip->netIo.shutdown(securityHandle, 0);
            if (close(prevFd)) {
                BDBG_MSG(("%s: failed to close prev socket (%d), ignore it\n", __FUNCTION__, prevFd));
                perror("close");
            }
            else
                BDBG_MSG(("%s: closed prev socket (fd %d)", __FUNCTION__, prevFd));
            /* reset the socketClosed flag */
            playback_ip->socketClosed = false;
        }

        if (http_build_get_req(&requestMessage, playback_ip, 1, 0., 0., byteRangeStart, byteRangeEnd, NULL) < 0) {
            BDBG_ERR(("%s: ERROR: failed to build HTTP Get request to Server", __FUNCTION__));
            goto error;
        }

        /* setup the socket connection to the server & send GET request */
        rc = B_PlaybackIp_UtilsTcpSocketConnect(&playback_ip->playback_state, playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port, true, playback_ip->networkTimeout, newFd);
        if (rc != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: ERROR: failed to send Socket Connect Request to Server: %s:%d\n",
                        __FUNCTION__, playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port));
            rc = -1;
            goto error;
        }
        prevFd = *newFd; /* incase we get a redirect, so we will need to close this newly opened socket */
        /* set this flag to indicate to playpump thread that the socket was restarted, so it should reconnect to server for restarting the playback from start */
        playback_ip->netRangeFunctionInvoked = true;

        /* SECURITY: we need to clone the new security session using the current one */
        /* Idea is to avoid any re-authentication to the server and instead reuse the session info from the current session */
        rc = B_PlaybackIp_SecurityCloneSessionOpen(playback_ip, *newFd, securityHandle, newSecurityHandle);
        if (rc < 0) {
            BDBG_ERR(("%s: ERROR: failed to clone the security session, rc %zu\n", __FUNCTION__, rc));
            goto error;
        }

        if (playback_ip->netIo.close) {
            /* free up the current security session */
            playback_ip->netIo.close(securityHandle);
            BDBG_MSG(("%s: freed up the previous (%p) security session, new session %p\n", __FUNCTION__, (void *)securityHandle, (void *)*newSecurityHandle));
        }
        securityHandle = *newSecurityHandle;

        /* now send the GET request */
        rc = playback_ip->netIo.write(securityHandle, &playback_ip->playback_state, *newFd, requestMessage, strlen(requestMessage));
        if (rc < 0) {
            BDBG_ERR(("%s: ERROR: failed to send HTTP Get request to Server: %s:%d\n", __FUNCTION__, playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port));
            goto error;
        }
        BDBG_MSG(("%s: Sent HTTP Get Request (fd %d)", __FUNCTION__, *newFd));
        BKNI_Free(requestMessage);
        requestMessage = NULL;

        reset_playback_ip_state(playback_ip);

        /* now read the http response */
        if ((rc = http_read_response(playback_ip, securityHandle, *newFd, &responseMessage, TMP_BUF_SIZE, &http_hdr, &http_payload, &initialLen)) < 0 ) {
            goto error;
        }

        http_url_type = http_get_url_type(http_hdr, playback_ip->openSettings.socketOpenSettings.url);
        BDBG_MSG(("http url type %d\n", http_url_type));
        if (http_url_type == HTTP_URL_IS_REDIRECT) {
            /* parse HTTP redirect and extract the new URL & Server */
            /* free up the previous cookie */
            if (playback_ip->cookieFoundViaHttpRedirect)
                BKNI_Free(playback_ip->cookieFoundViaHttpRedirect);
            playback_ip->cookieFoundViaHttpRedirect = NULL;
            if (http_parse_redirect(playback_ip->openSettings.socketOpenSettings.ipAddr, &playback_ip->openSettings.socketOpenSettings.port, &playback_ip->openSettings.socketOpenSettings.protocol, &playback_ip->openSettings.socketOpenSettings.url, &playback_ip->cookieFoundViaHttpRedirect, http_hdr) != 0) {
                BDBG_ERR(("Incorrect HTTP Redirect response or parsing error\n'"));
                goto error;
            }
            /* previous function gets the new URL & server information and we send another GET request to this server */
            playback_ip->serverClosed = false; /* clear this flag as server anyway closes socket during redirect and we want to call netIo.shutdown above */
            BDBG_MSG(("%s: HTTP redirect case, (resetting ranges) caching the redirected URL: http://%s:%d%s", __FUNCTION__, playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port, playback_ip->openSettings.socketOpenSettings.url));
            byteRangeEnd = byteRangeStart = 0;
        }
        else {
            /* actual content URL, get the content attributes from HTTP response header */
            BDBG_MSG(("GOT ACTUAL CONTENT\n"));
            break;
        }

        if (loopCnt++ >5) {
            BDBG_ERR(("%s: ERROR: Can't resolve a URL Link in 5 attempts", __FUNCTION__));
            goto error;
        }
    }

    /* copy any payload bytes into a temp buffer */
    if (initialLen) {
        /* initial data was read part of the HTTP response */
        memcpy(buf, http_payload, initialLen);
        bytesRead = initialLen;
        BDBG_MSG(("%s: COPYING initial data of %d bytes\n", __FUNCTION__, initialLen));
    }

    rc = 0;
    if (length && length-bytesRead > 0) {
        BDBG_MSG(("%s: remaining %zu, bytesRead %u, length %zu, initial len %d\n", __FUNCTION__, (length-bytesRead), bytesRead, length, initialLen));
        /* read next chunk of data from socket */
        if ((rc = playback_ip_read_socket(playback_ip, securityHandle, *newFd, ((char *)buf)+bytesRead, length-bytesRead, playback_ip->networkTimeout)) <= 0 ) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_ERR(("%s: Network Read Error: serverClosed %d, selectTimeout %d", __FUNCTION__, playback_ip->serverClosed, playback_ip->selectTimeout));
#endif
            if (playback_ip->serverClosed || playback_ip->selectTimeout)
                rc = bytesRead; /* hard coding to allow player to consume all data before closing */
            goto error;
        }
    }
    bytesRead += rc;

    BDBG_MSG(("%s: buf %p, asked %zu, read %d, start %"PRIu64 ", end %"PRIu64 "\n",
                __FUNCTION__, (void *)buf, length, bytesRead, byteRangeStart, byteRangeEnd));
    rc = bytesRead;

error:
    if (responseMessage)
        BKNI_Free(responseMessage);
    if (requestMessage)
        BKNI_Free(requestMessage);
    return rc;
}

int http_read_data_from_index_cache(
    B_PlaybackIpHandle playback_ip,
    off_t byteRangeStart,
    off_t byteRangeEnd,
    char *buf,
    size_t length
    )
{
    unsigned int readIndexOffset = 0;
    BDBG_MSG(("%s: Cache hit: range start %"PRIu64 ", end %"PRIu64 ", i cache depth %d, off: start %"PRId64 ", end %"PRId64 ", idx %d, length %zu\n",
            __FUNCTION__, byteRangeStart, byteRangeEnd, playback_ip->indexCacheDepth, playback_ip->indexCacheStartOffset, playback_ip->indexCacheEndOffset, playback_ip->indexCacheReadIndex, length));
    readIndexOffset = playback_ip->indexCacheReadIndex + (byteRangeStart - playback_ip->indexCacheStartOffset);
    readIndexOffset %= playback_ip->indexCacheSize;

    /* look for wrap around */
    if (readIndexOffset + length <= playback_ip->indexCacheSize) {
        /* all index data is sequentially available in the buffer w/o any wrap around, copy it */
        BDBG_MSG(("%s: single copy from index cache offset %d, bytes copied %zu\n", __FUNCTION__, readIndexOffset, length));
        memcpy(buf, playback_ip->indexCache+readIndexOffset, length);
        return length;
    }
    else {
        /* initial part of the data is at the end of cache and other is at the begining (due to cache wrap around) of cache */
        int bytesAtCacheEnd;
        int bytesRemaining;
        /* wrap around case, first copy data that is available at the end of the buffer */
        bytesAtCacheEnd = playback_ip->indexCacheSize - readIndexOffset;
        bytesRemaining = length - bytesAtCacheEnd;
        /* first copy of bytes at the end of the cache */
        memcpy(buf, playback_ip->indexCache+readIndexOffset, bytesAtCacheEnd);

        /* and copy remaining bytes from the begining of the cache */
        memcpy((char *)buf+bytesAtCacheEnd, playback_ip->indexCache, bytesRemaining);
        BDBG_MSG(("%s: Two copies: first of %d bytes from %d index to %d, 2nd of %d bytes from cache begining, copied %zu, bytes\n",
                __FUNCTION__, bytesAtCacheEnd, readIndexOffset, playback_ip->indexCacheSize-1, bytesRemaining, length));
        return length;
    }
}


typedef enum HttpCacheLookupResult {
    HTTP_CACHE_HIT,
    HTTP_CACHE_PARTIAL_HIT,
    HTTP_CACHE_RANGE_CACHEABLE,
    HTTP_CACHE_MISS
}HttpCacheLookupResult;

/* checks if a requested byte range is available in the cache or not. It returns the lookup result & cache index (in case of hit). */
HttpCacheLookupResult
httpAvCache_lookup(
    B_PlaybackIpHandle playback_ip,
    off_t byteRangeStart,
    off_t byteRangeEnd,
    int *cacheIndex)
{
    int i;
    B_Time curTime;

    *cacheIndex = HTTP_MAX_DATA_CACHES;

    for (i=0; i < HTTP_MAX_DATA_CACHES; i++) {
        if (!playback_ip->dataCache[i].inUse)
            continue;
        if (byteRangeStart >= playback_ip->dataCache[i].startOffset &&
                byteRangeEnd <= playback_ip->dataCache[i].endOffset) {
            /* cache hit: requested byte range is completely in the data cache */
            *cacheIndex = i;
            playback_ip->dataCache[i].stats.hits++;
            PRINT_CACHE_MSG(("%s: cache[%d] hit (total %d): byte range start %"PRId64 ", end %"PRId64 ", cache offset start %"PRId64 ", end %"PRId64 "\n", __FUNCTION__, i,
                        playback_ip->dataCache[i].stats.hits, byteRangeStart, byteRangeEnd,
                        playback_ip->dataCache[i].startOffset, playback_ip->dataCache[i].endOffset));
            B_Time_Get(&playback_ip->dataCache[i].lastUsedTime);
            return HTTP_CACHE_HIT;
        }
        if (byteRangeStart >= playback_ip->dataCache[i].startOffset &&
                byteRangeStart <= playback_ip->dataCache[i].endOffset) {
            /* partial cache hit: some initial part of the requested byte range is in the data cache */
            *cacheIndex = i;
            playback_ip->dataCache[i].stats.partialHits++;
            PRINT_CACHE_MSG(("%s: cache[%d] partial hit (total %d): byte range start %"PRId64 ", end %"PRId64 ", cache offset start %"PRId64 ", end %"PRId64 "\n", __FUNCTION__, i,
                        playback_ip->dataCache[i].stats.partialHits, byteRangeStart, byteRangeEnd,
                        playback_ip->dataCache[i].startOffset, playback_ip->dataCache[i].endOffset));
            B_Time_Get(&playback_ip->dataCache[i].lastUsedTime);
            return HTTP_CACHE_PARTIAL_HIT;
        }
        /* if we are not doing trick modes, then check if the requested range can be cached with whole cache size */
        /* if so, we try to read data sequentially */
        if (!playback_ip->lastSeekPositionSet && playback_ip->playback_state != B_PlaybackIpState_eTrickMode && byteRangeStart >= playback_ip->dataCache[i].startOffset &&
                byteRangeStart <= (playback_ip->dataCache[i].endOffset + playback_ip->dataCache[i].size)) {
            /* byte range start is not currently cached, but can fit in the cache and thus is cacheable */
            *cacheIndex = i;
            playback_ip->dataCache[i].stats.cacheable++;
            PRINT_CACHE_MSG(("%s: cache[%d] miss, but range is cacheable (# %d): byte range start %"PRId64 ", end %"PRId64 ", cache offset start %"PRId64 ", end %"PRId64 "\n", __FUNCTION__, i,
                        playback_ip->dataCache[i].stats.cacheable, byteRangeStart, byteRangeEnd,
                        playback_ip->dataCache[i].startOffset, playback_ip->dataCache[i].endOffset));
            B_Time_Get(&playback_ip->dataCache[i].lastUsedTime);
            return HTTP_CACHE_RANGE_CACHEABLE;
        }
        /* if we have done seek and haven't yet finished received & displayed data from the seek point (i.e. lastSeekPositionSet flag is true!) */
        /* and requested range starts right after what we have currently read, continue reading into the cache */
        if (playback_ip->lastSeekPositionSet && byteRangeStart >= playback_ip->dataCache[i].startOffset &&
                byteRangeStart == (playback_ip->dataCache[i].endOffset + 1)) {
            /* byte range start is not currently cached, but can fit in the cache and thus is cacheable */
            *cacheIndex = i;
            playback_ip->dataCache[i].stats.cacheable++;
            PRINT_CACHE_MSG(("%s: cache[%d] miss, but range is +1 from the current cache end and thus is cacheable (# %d): byte range start %"PRId64 ", end %"PRId64 ", cache offset start %"PRId64 ", end %"PRId64 "\n", __FUNCTION__, i,
                        playback_ip->dataCache[i].stats.cacheable, byteRangeStart, byteRangeEnd,
                        playback_ip->dataCache[i].startOffset, playback_ip->dataCache[i].endOffset));
            return HTTP_CACHE_RANGE_CACHEABLE;
        }
    }

    /* if we reach here, that means the requested range is not currently in the cache */
    /* or cacheable in the current cache range, so this is CACHE_MISS scenario */

    /* thus caller will need to purge the least recently used data cache and refill it with requested data range */
    /* so we determine the least recently used cache index here */
    B_Time_Get(&curTime);
    for (i=0; i < HTTP_MAX_DATA_CACHES; i++) {
        if (!playback_ip->dataCache[i].inUse) {
            /* found a unused cache, use it */
            *cacheIndex = i;
            break;
        }
    }
    if (*cacheIndex >= HTTP_MAX_DATA_CACHES) {
        if (HTTP_MAX_DATA_CACHES == 1) {
            PRINT_CACHE_MSG(("%s: only using one data cache, so overwriting its content w/ new request\n", __FUNCTION__));
        }
        else {
            BDBG_ERR(("%s: Error: we should always find (%d) one the data caches for use, defaulting it to cache 0\n", __FUNCTION__, *cacheIndex));
        }
        *cacheIndex = 0;
    }
    playback_ip->dataCache[*cacheIndex].stats.miss++;
    PRINT_CACHE_MSG(("%s: cache[%d] miss, range is outside cacheable range (# %d) for byte range start %"PRId64 ", end %"PRId64 ", cache offset start %"PRId64 ", end %"PRId64 "\n", __FUNCTION__, *cacheIndex,
        playback_ip->dataCache[*cacheIndex].stats.miss, byteRangeStart, byteRangeEnd, playback_ip->dataCache[*cacheIndex].startOffset, playback_ip->dataCache[*cacheIndex].endOffset));
    return HTTP_CACHE_MISS;
}

ssize_t
http_avCacheMissProcessing(
    B_PlaybackIpHandle playback_ip,
    off_t byteRangeStart,
    off_t byteRangeEnd,
    size_t length,
    int cacheIndex)
{
    int rc;
    ssize_t bytesRead;
    /* this function handles the cache miss event where the requested data is not in the data cache */
    /* this can happen in two cases: during normal rewind of content that is cached or when user issues */
    /* seek operation to a range that is outside of cacheable range */
    /* Our action is to close the current connection, setup a new one & ask the server to send data starting from new range */
    PRINT_CACHE_MSG(("%s: rewind/seek case: requested data is not in the cache, so resending the HTTP req to server, first data offset %"PRId64 ", total read %"PRId64 ", consumed %"PRId64 ", rewind %d\n",
            __FUNCTION__, playback_ip->dataCache[cacheIndex].firstOffset, playback_ip->totalRead, playback_ip->totalConsumed, playback_ip->rewind));
    PRINT_CACHE_MSG(("%s: byte range start %"PRId64 ", end %"PRId64 ", length %zu, offset start %"PRId64 ", end %"PRId64 "\n",
            __FUNCTION__, byteRangeStart, byteRangeEnd, length, playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset));

    if (playback_ip->reOpenSocket)
        playback_ip->reOpenSocket = false;
    /* send new HTTP Range request on a new socket */
    rc = B_PlaybackIp_HttpNetRangeReq(playback_ip, playback_ip->dataCache[cacheIndex].cache, HTTP_DATA_CACHE_CHUNK_SIZE, byteRangeStart, 0, playback_ip->socketState.fd, &playback_ip->socketState.fd);
    if (rc < 0) {
        BDBG_ERR(("%s: Failed to rewind the content, read %d, asked %u\n", __FUNCTION__, rc, HTTP_DATA_CACHE_CHUNK_SIZE));
        return -1 ;
    }
    bytesRead = rc;

    if (bytesRead != HTTP_DATA_CACHE_CHUNK_SIZE || (size_t)bytesRead < length) {
        PRINT_CACHE_MSG(("%s: during rewind/re-seek operation, data cache case: not able to read %zd the requested amount %d, remaining amount %zu\n",
                __FUNCTION__, bytesRead, HTTP_DATA_CACHE_CHUNK_SIZE, length));
    }
    /* update the data cache pointers */
    playback_ip->rewind = false;
    playback_ip->dataCache[cacheIndex].startOffset = byteRangeStart;
    /* indicate addition of this new chunk by moving forward the end offset by a chunk size */
    playback_ip->dataCache[cacheIndex].endOffset = playback_ip->dataCache[cacheIndex].startOffset + bytesRead - 1;

    playback_ip->dataCache[cacheIndex].readIndex = 0;
    /* increase data cache depth by 1 chunk size */
    playback_ip->dataCache[cacheIndex].depth = bytesRead;

    /* increment data cache write index */
    playback_ip->dataCache[cacheIndex].writeIndex = bytesRead;

    /* now copy the remaining data from data cache into the requested buffer */
    PRINT_CACHE_MSG(("%s: during rewind/seek operation, copied %zd bytes (asked %zu) into data cache index %d\n", __FUNCTION__, bytesRead, length, cacheIndex));
    playback_ip->dataCache[cacheIndex].inUse = true;
    PRINT_CACHE_MSG(("%s: Data Cache[%d] Setup after rewind/seek: dcache offset start %"PRId64 ", end %"PRId64 ", size %d, index cache end offset %"PRId64 "\n",
            __FUNCTION__, cacheIndex, playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
            playback_ip->dataCache[cacheIndex].size, playback_ip->indexCacheEndOffset));
    PRINT_CACHE_MSG(("%s: dcache: hit range start %"PRId64 ", end %"PRId64 ", cache start %"PRId64 ", end %"PRId64 " depth %d, rd idx %d, wr idx %d, read %zd\n",
            __FUNCTION__, byteRangeStart, byteRangeEnd,
            playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
            playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].readIndex, playback_ip->dataCache[cacheIndex].writeIndex, bytesRead));
    return bytesRead;
}

int
http_avCachePartialHitProcessing(
    B_PlaybackIpHandle playback_ip,
    off_t byteRangeStart,
    off_t byteRangeEnd,
    size_t length,
    int cacheIndex)
{
    int rc =-1;
    ssize_t bytesRead = 0;
    int cacheMissCount = 0;
    int bytesToRead = 0;
    int spaceAvailableAtEnd;

    BSTD_UNUSED(length);

    /* the requested data range is not completely in the data cache, read chunk at a time from network until we can service this range */
    while (byteRangeEnd > playback_ip->dataCache[cacheIndex].endOffset) {
        if (playback_ip->playback_state == B_PlaybackIpState_eStopping || (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode)) {
            PRINT_CACHE_MSG(("%s: breaking of the while loop due to state change (state %d)", __FUNCTION__, playback_ip->playback_state));
            if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode) {
                playback_ip->selectTimeout = true;
                errno = EINTR;
            }
            else {
                playback_ip->serverClosed = true;
            }
            break;
        }
        /* See if there is room in cache to read n/w data */
        /* else retire one older chunk from cache to make space for new one */
        if (playback_ip->dataCache[cacheIndex].depth >= playback_ip->dataCache[cacheIndex].size) {
            /* cache full, wrap around and retire one chunk, this is the least recently used chunk. */
            /* drop this oldest chunk of data by moving forward the start offset: increases linearly */
            playback_ip->dataCache[cacheIndex].startOffset += HTTP_DATA_CACHE_CHUNK_SIZE;
            /* move the read index forward as well so that it aligns with the StartOffset */
            playback_ip->dataCache[cacheIndex].readIndex += HTTP_DATA_CACHE_CHUNK_SIZE;
            if (playback_ip->dataCache[cacheIndex].readIndex >= playback_ip->dataCache[cacheIndex].size) {
                playback_ip->dataCache[cacheIndex].readIndex %= playback_ip->dataCache[cacheIndex].size;
                PRINT_CACHE_MSG(("%s: read index Wrap around: rd idx %d, depth %d, size %d\n",
                        __FUNCTION__, playback_ip->dataCache[cacheIndex].readIndex,
                        playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].size));
            }

            /* data cache depth is reduced by 1 chunk size */
            playback_ip->dataCache[cacheIndex].depth -= HTTP_DATA_CACHE_CHUNK_SIZE;
            PRINT_CACHE_MSG(("%s: dCache Wrap around: Retire %d byte chunk, start offset %"PRId64 ", rd idx %d, depth %d, size %d\n",
                        __FUNCTION__, HTTP_DATA_CACHE_CHUNK_SIZE,
                        playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].readIndex,
                        playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].size));
        }

        /* read lower of data chunk size or space available at end */
        spaceAvailableAtEnd = playback_ip->dataCache[cacheIndex].size - playback_ip->dataCache[cacheIndex].writeIndex;
        if (spaceAvailableAtEnd < HTTP_DATA_CACHE_CHUNK_SIZE) {
            PRINT_CACHE_MSG(("%s: reached towards end of data cache, so reading (%d) less than chunk size (%d), cache size %d, cache wr idx %d\n",
                        __FUNCTION__, spaceAvailableAtEnd, HTTP_DATA_CACHE_CHUNK_SIZE, playback_ip->dataCache[cacheIndex].size, playback_ip->dataCache[cacheIndex].writeIndex));
            bytesToRead = spaceAvailableAtEnd;
        }
        else
            bytesToRead = HTTP_DATA_CACHE_CHUNK_SIZE;
        /* read 1 chunk worth data from n/w */
        if (playback_ip->reOpenSocket /* true if index was at the end (e.g. for ASF) and thus server had closed socket a/f sending the index part || pause is done using disconnect & seek */
                || playback_ip->cacheIndexUsingSocket != cacheIndex /* true if we are switching the data cache */
           ) {
            /* in both cases, we need to close the current socket & send a new request to server */
            /* both are done in NetRangeReq()) */
            BDBG_MSG(("%s: re-setup the connection for cache idx %d, prev cache idx %d, re-open socket %d, fd %d\n", __FUNCTION__, cacheIndex, playback_ip->cacheIndexUsingSocket, playback_ip->reOpenSocket, playback_ip->socketState.fd));
            playback_ip->reOpenSocket = false;
            playback_ip->cacheIndexUsingSocket = cacheIndex;
            /* send new HTTP Range request on a new socket */
            rc = B_PlaybackIp_HttpNetRangeReq(playback_ip, playback_ip->dataCache[cacheIndex].cache + playback_ip->dataCache[cacheIndex].writeIndex, bytesToRead, playback_ip->dataCache[cacheIndex].endOffset+1, 0, playback_ip->socketState.fd, &playback_ip->socketState.fd);
        }
        else {
            rc = playback_ip_read_socket(playback_ip, playback_ip->securityHandle, playback_ip->socketState.fd, playback_ip->dataCache[cacheIndex].cache + playback_ip->dataCache[cacheIndex].writeIndex, bytesToRead, playback_ip->networkTimeout);
        }
        if (rc < 0) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_ERR(("%s: Network Read Error, rc %d", __FUNCTION__, rc));
#endif
            /* return what ever has been read so far */
            goto error;
        }
        else if (rc == 0 && bytesRead == 0) {
            PRINT_CACHE_MSG(("%s: read 0 bytes from socket, server has already closed, rc=%d\n", __FUNCTION__, rc));
            goto error;
        }
        bytesRead += rc;

        /* increment data cache write index */
        playback_ip->dataCache[cacheIndex].writeIndex += rc;
        playback_ip->dataCache[cacheIndex].writeIndex %= playback_ip->dataCache[cacheIndex].size;
        /* indicate addition of this new chunk by moving forward the end offset by a chunk size */
        playback_ip->dataCache[cacheIndex].endOffset == 0 ?
            playback_ip->dataCache[cacheIndex].endOffset = rc - 1 :
            (playback_ip->dataCache[cacheIndex].endOffset += rc);
        /* increase data cache depth by bytesRead */
        playback_ip->dataCache[cacheIndex].depth += rc;

        cacheMissCount++;
        if (playback_ip->serverClosed) {
            PRINT_CACHE_MSG(("%s: breaking from reading loop due to server closing connection, bytes read %d", __FUNCTION__, rc));
            break;
        }
        if (playback_ip->selectTimeout) {
            PRINT_CACHE_MSG(("%s: breaking from reading loop due to select timeout, bytes read %d", __FUNCTION__, rc));
            break;
        }

        /* otherwise, go back to see if the byte range request can be serviced */
    }
error:
    PRINT_CACHE_MSG(("%s: dCache: fd %d, asked %d, read %zd: miss %d, cache start %"PRId64 ", end %"PRId64 " depth %d, wr idx %d\n",
            __FUNCTION__, playback_ip->socketState.fd, bytesToRead, bytesRead, cacheMissCount,
            playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
            playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].writeIndex));
    if (bytesRead) {
        /* we have read something: now make sure if we have satisfied all conditions */
        if (byteRangeEnd <= playback_ip->dataCache[cacheIndex].endOffset) {
            /* we have read all requested data from byteRangeStart to End */
            PRINT_CACHE_MSG(("%s: dCache: requested data (%zu) was read (%zd) w/o interrutions, server closed %d, select timeout %d\n", __FUNCTION__, length, bytesRead, playback_ip->serverClosed, playback_ip->selectTimeout));
            return (int)bytesRead;
        }
        /* now we know that end range is outside the current cache end, check where is the start range, we know that it is > startRange */
        else if (byteRangeStart > playback_ip->dataCache[cacheIndex].endOffset) {
            /* startRange is outside what has been currently read */
            /* even though we have read some data, requested byteRangeStart is still not read possibly due to server close or timeout event */
            PRINT_CACHE_MSG(("%s: dCache: some requested data (%zu) read (%zd), but still before start range, server closed %d, select timeout %d\n", __FUNCTION__, length, bytesRead, playback_ip->serverClosed, playback_ip->selectTimeout));
            return -1;
        } else {
            /* this means that start is <= endOffset, so we return data that is in the cache */
            rc = playback_ip->dataCache[cacheIndex].endOffset - byteRangeStart + 1;
            PRINT_CACHE_MSG(("%s: dCache: requested data (%zu) was partially read (%zd), so returning %d, server closed %d, select timeout %d\n", __FUNCTION__, length, bytesRead, rc, playback_ip->serverClosed, playback_ip->selectTimeout));
            return rc;
        }
    }
    else
        return rc;
}

int
http_avCacheHitProcessing(
    B_PlaybackIpHandle playback_ip,
    off_t byteRangeStart,
    off_t byteRangeEnd,
    char *buf,
    size_t length,
    int cacheIndex)
{
    unsigned int readIndexOffset = 0;
    int bytesCopied = 0;
    /* make sure asked byte range is in data cache & then copy it into the requested buffer */
    if (byteRangeEnd <= playback_ip->dataCache[cacheIndex].endOffset && byteRangeStart >= playback_ip->dataCache[cacheIndex].startOffset) {
        /* calculate the read index into the cache for this byte range */
        readIndexOffset = playback_ip->dataCache[cacheIndex].readIndex + (byteRangeStart - playback_ip->dataCache[cacheIndex].startOffset);
        readIndexOffset %= playback_ip->dataCache[cacheIndex].size;

        /* look for wrap around */
        if ((readIndexOffset + length) <= playback_ip->dataCache[cacheIndex].size) {
            /* all data range is linealy present in the cache before its end, so do a straight copy */
            memcpy((unsigned char *)buf, playback_ip->dataCache[cacheIndex].cache+readIndexOffset, length);
            BDBG_MSG(("%s: One copy of %zu bytes from %d index\n", __FUNCTION__, length, readIndexOffset));
            bytesCopied = length;
        }
        else {
            /* initial part of the data is at the end and other is at the begining (due to cache wrap around) of data cache */
            int bytesAtCacheEnd;
            int bytesRemaining;
            /* wrap around case, first copy data that is available at the end of the buffer */
            bytesAtCacheEnd = (playback_ip->dataCache[cacheIndex].size - readIndexOffset);
            bytesRemaining = length - bytesAtCacheEnd;
            /* first copy of bytes at the end of the cache */
            memcpy(buf, playback_ip->dataCache[cacheIndex].cache+readIndexOffset, bytesAtCacheEnd);

            /* and copy remaining bytes from the begining of the cache */
            memcpy(buf+bytesAtCacheEnd, playback_ip->dataCache[cacheIndex].cache, bytesRemaining);
            bytesCopied = bytesAtCacheEnd + bytesRemaining;
            BDBG_MSG(("%s: Two copies: first of %d bytes from %d index to %d, 2nd of %d bytes from cache begining, copied %d, asked %zu bytes\n",
                        __FUNCTION__, bytesAtCacheEnd, readIndexOffset, playback_ip->dataCache[cacheIndex].size-1,
                        bytesRemaining, bytesCopied, length));
        }
        BDBG_MSG(("dcache: hit range start %"PRId64 ", end %"PRId64 ", cache start %"PRId64 ", end %"PRId64 " depth %d, rd idx %d, rd offset %d, read %zu\n",
                    byteRangeStart, byteRangeEnd,
                    playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
                    playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].readIndex, readIndexOffset, length));
        if (playback_ip->setupSettings.u.http.convertLpcmToWave) {
            /* LPCM data is in the BE format, but Audio FW only understands LE format WAV. So convert into LE by byte-swapping the data */
            B_PlaybackIp_UtilsByteSwap((char *)buf, bytesCopied);
        }
        return bytesCopied;
    }
    else {
        BDBG_ERR(("%s: SW bug, requested range should have been in data cache : range start %"PRId64 ", end %"PRId64 ", cache start %"PRId64 ", end %"PRId64 " depth %d, rd idx %d, rd offset %d, read %zu\n",
                    __FUNCTION__, byteRangeStart, byteRangeEnd,
                    playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
                    playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].readIndex, readIndexOffset, length));
        return -1;
    }
}

int http_validate_and_adjust_byte_ranges(B_PlaybackIpHandle playback_ip, off_t *byteRangeStartPtr, off_t *byteRangeEndPtr, size_t *lengthPtr)
{
    off_t byteRangeStart;
    off_t byteRangeEnd;
    size_t length;
    int cacheIndex;

    length = *lengthPtr;
    byteRangeStart = *byteRangeStartPtr;
    byteRangeEnd = *byteRangeEndPtr;
    cacheIndex = playback_ip->lastUsedCacheIndex;
    if (playback_ip->serverClosed && playback_ip->dataCache[cacheIndex].inUse)
    {
        /* we may need to adjust the requested if server is no longer sending data & player is asking for more data outside the cache */
        cacheIndex = playback_ip->cacheIndexUsingSocket;
        if (byteRangeStart > playback_ip->dataCache[cacheIndex].endOffset) {
            /* TODO: this check makes us potentially not reconnect to the server when we haven't cached enough data, may need to take this out */
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_ERR(("%s: server closed case: requested range start is outside data cache end, returning ERROR : range start %"PRId64 ", end %"PRId64 ", cache start %"PRId64 ", end %"PRId64 " depth %d, rd idx %d, read %zu\n",
                    __FUNCTION__, byteRangeStart, byteRangeEnd,
                    playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
                    playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].readIndex, length));
#endif
            return -1;
        }
        /* now byteRangeStart is within cache, check for end for it being outside and trim it */
        if (byteRangeEnd > playback_ip->dataCache[cacheIndex].endOffset) {
            byteRangeEnd = playback_ip->dataCache[cacheIndex].endOffset;
            length = byteRangeEnd - byteRangeStart +1;
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog) {
                BDBG_ERR(("%s: server closed case: requested range start %"PRId64 ", end %"PRId64 " is outside data cache start %"PRId64 " end %"PRId64 ", trimming it\n",
                    __FUNCTION__, byteRangeStart, byteRangeEnd, playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset));
                BDBG_ERR(("%s: new range start %"PRId64 ", new end %"PRId64 ", cache start %"PRId64 ", end %"PRId64 " depth %d, rd idx %d, read %zu\n",
                    __FUNCTION__, byteRangeStart, byteRangeEnd,
                    playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
                    playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].readIndex, length));
            }
#endif
        }
    }
    else if (playback_ip->contentLength) {
        /* we may need to adjust the requested range if player is asking for more data outside the content length */
        cacheIndex = playback_ip->lastUsedCacheIndex;
        if (byteRangeStart >= playback_ip->contentLength) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_ERR(("%s: requested range start is outside content length %"PRId64 ", returning ERROR : range start %"PRId64 ", end %"PRId64 ", cache start %"PRId64 ", end %"PRId64 " depth %d, rd idx %d, read %zu\n",
                    __FUNCTION__, playback_ip->contentLength, byteRangeStart, byteRangeEnd,
                    playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
                    playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].readIndex, length));
#endif
            return -1;
        }
        /* now byteRangeStart is within cache, check for end for it being outside and trim it */
        if (byteRangeEnd >= playback_ip->contentLength) {
            byteRangeEnd = playback_ip->contentLength -1;
            length = byteRangeEnd - byteRangeStart +1;
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog) {
                BDBG_ERR(("%s: range end is after content length %"PRId64 ": requested range start %"PRId64 ", end %"PRId64 " is outside data cache start %"PRId64 " end %"PRId64 ", trimming it\n",
                        __FUNCTION__, playback_ip->contentLength, byteRangeStart, byteRangeEnd, playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset));
                BDBG_ERR(("%s: new range start %"PRId64 ", new end %"PRId64 ", cache start %"PRId64 ", end %"PRId64 " depth %d, rd idx %d, read %zu\n",
                        __FUNCTION__, byteRangeStart, byteRangeEnd,
                        playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
                        playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].readIndex, length));
            }
#endif
        }
    }
    *lengthPtr = length;
    *byteRangeStartPtr = byteRangeStart;
    *byteRangeEndPtr = byteRangeEnd;
    return 0;
}

int
http_read_data_from_data_cache(
    B_PlaybackIpHandle playback_ip,
    struct bfile_io_read_net *file,
    off_t byteRangeStart,
    off_t byteRangeEnd,
    char *buf,
    size_t length
    )
{
    int rc = -1;
    int bytesRead = 0;
    unsigned int bytesToCopy;
    size_t origLength;
    int cacheIndex;
    HttpCacheLookupResult cacheLookupResult;
    char *origBuf = buf;

    origLength = length;
    if (http_validate_and_adjust_byte_ranges(playback_ip, &byteRangeStart, &byteRangeEnd, &length)) {
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_ERR(("%s: Invalid byte range requested, returning EOF", __FUNCTION__));
#endif
        return (ssize_t)0;
    }

    /* set a flag to indicate that read call back in progress */
    /* Note: this flag needs to be cleared before we return from this function, so can't have any more return statements until end of this function (instead use goto out/error) */
    playback_ip->readCallBackInProgress = true;

    /* special check for playing LPCM files: since Audio decoder doesn't have support for native LPCM files, */
    /* we convert LPCM files into WAV by inserting WAV header in the begining. */
    if (playback_ip->setupSettings.u.http.convertLpcmToWave) {
        /* length needs to be 2byte aligned as we will need to convert the data from BE to LE format */
        length -= length%2;
        if (!playback_ip->waveHeaderInserted) {
            rc = B_PlaybackIp_UtilsBuildWavHeader((void *)buf, length, playback_ip->setupSettings.u.http.bitsPerSample, playback_ip->setupSettings.u.http.sampleRate, playback_ip->setupSettings.u.http.numChannels);
            if (rc < 0) {
                BDBG_ERR(("%s: Failed to build Wave Header for LPCM files\n", __FUNCTION__));
                goto error;
            }
            buf = (char *)buf + rc;
            length -= rc;
            byteRangeEnd = byteRangeStart+length-1;
            bytesRead = rc;
            playback_ip->waveHeaderInserted = true;
            BDBG_MSG(("%s: Built & Inserted Wave Header ( bits/sample %d, rate %d, #ch %d) for LPCM files\n", __FUNCTION__,
                playback_ip->setupSettings.u.http.bitsPerSample, playback_ip->setupSettings.u.http.sampleRate, playback_ip->setupSettings.u.http.numChannels));
        }
    }

    if (!playback_ip->dataCache[0].inUse) {
        if (playback_ip->initial_data_len) {
            /* some data got read part of reading the HTTP response, copy that data into data cache & initialize cache state variables */
            memcpy(playback_ip->dataCache[0].cache, playback_ip->temp_buf, playback_ip->initial_data_len);
            playback_ip->dataCache[0].depth = playback_ip->initial_data_len;
            playback_ip->dataCache[0].startOffset = 0;
            playback_ip->dataCache[0].endOffset = playback_ip->initial_data_len -1;
            playback_ip->dataCache[0].readIndex = 0;
            playback_ip->dataCache[0].writeIndex = playback_ip->initial_data_len;
            BDBG_MSG(("%s: Copied initial payload of %d bytes into the data cache\n", __FUNCTION__, playback_ip->initial_data_len));
            playback_ip->initial_data_len = 0;
        }
        else {
            /* initialize data cache */
            /* starting offset of the data data: it should be the next byte after what is present in the index cache */
            if (playback_ip->indexCacheInit && playback_ip->indexCacheEndOffset > 0)
                playback_ip->dataCache[0].startOffset = playback_ip->indexCacheEndOffset + 1;
            else
                playback_ip->dataCache[0].startOffset = 0;
            playback_ip->dataCache[0].readIndex = 0;
            playback_ip->dataCache[0].endOffset = playback_ip->dataCache[0].startOffset - 1;
            playback_ip->dataCache[0].depth = 0;
            playback_ip->dataCache[0].writeIndex = 0;
        }
        BDBG_MSG(("%s: Data Cache Setup: dcache offset start %"PRId64 ", end %"PRId64 ", size %d, index cache end offset %"PRId64 "\n",
                    __FUNCTION__, playback_ip->dataCache[0].startOffset, playback_ip->dataCache[0].endOffset,
                    playback_ip->dataCache[0].size, playback_ip->indexCacheEndOffset));
        playback_ip->dataCache[0].inUse = true;
        playback_ip->dataCache[0].firstOffset = file->offset;
    }
    else {
        /* data Cache is initialized, check if offset has been rewind, then reset state */
        if (file->offset <= playback_ip->dataCache[0].firstOffset && playback_ip->playback_state != B_PlaybackIpState_eTrickMode) {
            /* rewind flag is only used for cases where we are not caching the data as we dont know when the wrap around happens in those cases */
            playback_ip->rewind = true;
            playback_ip->totalConsumed = 0;
            BDBG_MSG(("%s: rewinding: first data offset %"PRId64 ", cur %"PRId64 ", total read %"PRId64 ", consumed %"PRId64 "\n",
                        __FUNCTION__, playback_ip->dataCache[0].firstOffset, file->offset, playback_ip->totalRead, playback_ip->totalConsumed));
        }
    }

    /*
     * Look if asked byte range is in the index cache. This is only possible when index is in the beginging.
     * For container formats like MPEG TS, there is no separate index and psi information is only derived from parsing the content.
     * When index is at the end (possible for ASF & some MP4 content), data can't be in the index cache.
     */
    if (!playback_ip->indexAtEnd && playback_ip->indexCacheEndOffset ) {
        /* index is not at the end of stream (thus is available in index cache) and is valid */
        if (byteRangeStart >= playback_ip->indexCacheStartOffset && byteRangeEnd <= playback_ip->indexCacheEndOffset) {
            /* asked byte range is completely in index cache */
            BDBG_MSG(("%s: Cache hit in icache: range start %"PRIu64 ", end %"PRIu64 ", icache end off %"PRId64 ", asked %zu, total read %"PRId64 ", consumed %"PRId64 "\n",
                     __FUNCTION__, byteRangeStart, byteRangeEnd, playback_ip->indexCacheEndOffset, length, playback_ip->totalRead, playback_ip->totalConsumed));
            bytesRead = http_read_data_from_index_cache(playback_ip, byteRangeStart, byteRangeEnd, buf, length);
            goto out;
        }
        else if (byteRangeStart <= playback_ip->indexCacheEndOffset && byteRangeEnd > playback_ip->indexCacheEndOffset) {
            /* asked byte range is partially in index cache */
            unsigned int idx;
            idx = (unsigned)byteRangeStart;
            bytesToCopy = playback_ip->indexCacheEndOffset - byteRangeStart + 1;
            bytesRead = http_read_data_from_index_cache(playback_ip, byteRangeStart, byteRangeEnd, buf, bytesToCopy);
            bytesRead = bytesToCopy;

            length -= bytesRead;
            byteRangeStart += bytesRead;
            buf = (char *)buf + bytesRead;
            playback_ip->totalConsumed += bytesRead;
            BDBG_MSG(("%s: Partial Cache hit in icache: range start %"PRIu64 ", end %"PRIu64 ", icache depth %"PRId64 ", copied %u, remaining %zu\n",
                     __FUNCTION__, byteRangeStart, byteRangeEnd, playback_ip->indexCacheEndOffset, bytesToCopy, length));
        }
    }

    /* At this point, requested range is not in the index cache. */

    /* Check if this is a live channel session. Then, we dont go thru the data cache and instead directly return the read data */
    if (playback_ip->psi.liveChannel || playback_ip->setupSettings.u.http.liveChannel) {
        rc = playback_ip_read_socket(playback_ip, playback_ip->securityHandle, file->playback_ip->socketState.fd, (char *)buf, length, playback_ip->networkTimeout);
        if (rc <= 0 && bytesRead == 0) {
            goto error;
        }
        bytesRead += rc;
        BDBG_MSG(("%s: live flow: returning %d bytes, read from n/w length %d, from icache %zu, total Read %"PRId64 "", __FUNCTION__, bytesRead, rc, length, playback_ip->totalRead));
        goto out;
    }

    /*
     * If we are here, then we are always caching movie data. This is currently true for both MP4 & ASF formats.
     * Note: we always cache the index related data.
     */

    /* check if requested range is in one of the data caches */
    cacheLookupResult = httpAvCache_lookup(playback_ip, byteRangeStart, byteRangeEnd, &cacheIndex);
    switch (cacheLookupResult) {
    case HTTP_CACHE_MISS:
        /* requested range is not in cache, restart connection to server and refill cache from the requested range */
        rc = http_avCacheMissProcessing(playback_ip, byteRangeStart, byteRangeEnd, length, cacheIndex);
        if (rc < 0) {
            BDBG_ERR(("%s: failed to restart connection to server to refill cache from new range, rc %d\n", __FUNCTION__, rc));
            goto error;
        }
        else if (rc < (int)length) {
            BDBG_MSG(("%s: lowered length to rc asked %zu, read %d\n",__FUNCTION__, length, rc));
            length = rc;
        }
        /* now copy the read data */
        bytesToCopy = length;
        memcpy(buf, playback_ip->dataCache[cacheIndex].cache, bytesToCopy);
        if (playback_ip->setupSettings.u.http.convertLpcmToWave) {
            /* LPCM data is in the BE format, but Audio FW only understands LE format WAV. So convert into LE by byte-swapping the data */
            B_PlaybackIp_UtilsByteSwap((char *)buf, bytesToCopy);
        }
        bytesRead += bytesToCopy;
        playback_ip->totalConsumed += bytesToCopy;
        playback_ip->cacheIndexUsingSocket = cacheIndex;
        playback_ip->lastUsedCacheIndex = cacheIndex;
        BDBG_MSG(("%s: returning %d bytes, orig length %zu, length %zu, byteRangeStart %"PRId64 ", byteRangeEnd %"PRId64 ", total read %"PRId64 "\n", __FUNCTION__, bytesRead, origLength, length, byteRangeStart, byteRangeEnd, playback_ip->totalRead));
        goto out;
    case HTTP_CACHE_PARTIAL_HIT:
    case HTTP_CACHE_RANGE_CACHEABLE:
        /* since initial part of the data is in cache or atleast the requested range is cacheable */
        /* meaning that it fits in the cache size, we try to read more data from network until we satify the request */
        rc = http_avCachePartialHitProcessing(playback_ip, byteRangeStart, byteRangeEnd, length, cacheIndex);
        if (rc < 0) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_MSG(("%s: failed to fill up the current data cache, rc %d", __FUNCTION__, rc));
#endif
            goto error;
        }
        else if (rc < (int)length && byteRangeEnd > playback_ip->dataCache[cacheIndex].endOffset) {
            BDBG_MSG(("%s: couldn't read all requested amount from n/w, read %d, asked %zu, server has closed %d, select timeout %d\n", __FUNCTION__, rc, length, playback_ip->serverClosed, playback_ip->selectTimeout));
            if (byteRangeStart > playback_ip->dataCache[cacheIndex].endOffset) {
                BDBG_MSG(("%s: didn't get to read any data, byteRate start %"PRId64 ", end %"PRId64 ", cache end %"PRId64 ", selectTImeout %d",
                            __FUNCTION__, byteRangeStart, byteRangeEnd, playback_ip->dataCache[cacheIndex].endOffset, playback_ip->selectTimeout));
                goto error;
            }
            byteRangeEnd = playback_ip->dataCache[cacheIndex].endOffset;
            length = byteRangeEnd - byteRangeStart + 1;
            BDBG_MSG(("%s: modified new byteRange end to %"PRId64 ", start %"PRId64 "\n", __FUNCTION__, byteRangeEnd, byteRangeStart));
        }
        /* at this point, we should have the requested byte range in the cache, so we drop into the cache hit case below */
    case HTTP_CACHE_HIT:
        /* requested range is in the cache, copy it to the caller's buffer and return */
        rc = http_avCacheHitProcessing(playback_ip, byteRangeStart, byteRangeEnd, buf, length, cacheIndex);
        if (rc <= 0) {
            BDBG_ERR(("%s: failed to return requested data, rc %d\n", __FUNCTION__, rc));
            goto error;
        }
        bytesRead += rc;
        playback_ip->lastUsedCacheIndex = cacheIndex;
        BDBG_MSG(("%s: returning %d bytes, orig length %zu, length %zu, byteRangeStart %"PRId64 ", byteRangeEnd %"PRId64 ", total Read %"PRId64 "\n", __FUNCTION__, bytesRead, origLength, length, byteRangeStart, byteRangeEnd, playback_ip->totalRead));
        goto out;
    default:
        BDBG_ASSERT(NULL);
    }

error:
    bytesRead = rc;
out:
    if (bytesRead > 0) {
        file->offset += bytesRead;
        if (playback_ip->fclear) {
            fwrite(origBuf, 1, bytesRead , playback_ip->fclear);
            fflush(playback_ip->fclear);
        }
    }

    playback_ip->readCallBackInProgress = false;
    if (playback_ip->serverClosed && playback_ip->contentLength == 0) {
        /* content length is not specified when server is sending data using chunk xfer encoding */
        /* we can set the content length once we have reached the end of stream as it helps w/ rewind purposes */
        playback_ip->contentLength = playback_ip->dataCache[playback_ip->lastUsedCacheIndex].endOffset + 1;
        BDBG_MSG(("%s: for chunk encoding case, set the content length %"PRId64 "\n", __FUNCTION__, playback_ip->contentLength));
    }
    if (bytesRead > 0)
        return (ssize_t)bytesRead;
    else {
         if (playback_ip->selectTimeout || playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode) {
             /* either select timeout or we existed because another app thread is waiting to enter into trickmode state */
             /* we return NO data and give other thread a chance to run */
#ifdef BDBG_DEBUG_BUILD
             if (playback_ip->ipVerboseLog)
                 BDBG_ERR(("%s: select timeout, tell playback to retry read, state %d", __FUNCTION__, playback_ip->playback_state));
#endif
            return BFILE_ERROR_NO_DATA;
         }
         else if (bytesRead == 0) {
             /* EOF */
             return 0;
         }
         else {
            BDBG_ERR(("%s: ERROR: read callback failed, rc %d, returing EOF", __FUNCTION__, bytesRead));
            return (ssize_t)0;
         }
    }
}

static ssize_t
B_PlaybackIp_HttpNetDataRead(bfile_io_read_t self, void *buf, size_t length)
{
    ssize_t rc;
    struct bfile_io_read_net *file = (struct bfile_io_read_net *) self;
    B_PlaybackIpHandle playback_ip;
    off_t byteRangeStart;
    off_t byteRangeEnd;
    size_t origLength;
    static int fileNameSuffix = 0;
    char recordFileName[32];
#ifdef BDBG_DEBUG_BUILD
    B_Time startTime, endTime;
    static unsigned min = 0, max = 0, avg = 0;
    unsigned curTime;
    B_Time_Get(&startTime);
#endif

    if (!file || !file->playback_ip) {
        return -1;
    }
    playback_ip = file->playback_ip;

    BDBG_MSG(("%s: read %zu bytes at offset %"PRId64 "", __FUNCTION__, length, file->offset));
    if (BKNI_TryAcquireMutex(playback_ip->lock) == BERR_TIMEOUT) {
        BDBG_MSG(("%s: can't get the lock, returning", __FUNCTION__));
        BKNI_Sleep(80); /* delay returning back to the caller (nexus io thread) otherwise it re-invokes this call rightaway and thus wastes cycles in preventing the thread holding the lock to run */
        return BFILE_ERROR_NO_DATA;
    }
    if (playback_ip->enableRecording && !playback_ip->fclear) {
        memset(recordFileName, 0, sizeof(recordFileName));
        snprintf(recordFileName, sizeof(recordFileName)-1, "./videos/http_playback_rec%d.ts", fileNameSuffix++);
        playback_ip->fclear = fopen(recordFileName, "w+b");
    }
    if ((((struct bfile_in_net *)playback_ip->nexusFileHandle)->self.file.index != NULL) &&
        (playback_ip->playback_state == B_PlaybackIpState_eBuffering ||  /* HTTP thread is currently buffering */
        playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode || /* app is waiting to initiate a trickmode operation, but can't proceed until IO thread completes */
        (playback_ip->playback_state == B_PlaybackIpState_ePaused && playback_ip->ipTrickModeSettings.pauseMethod == B_PlaybackIpPauseMethod_UseDisconnectAndSeek) || /* If pausing using disconnect & seek method, we can't return any data until pause-release (otherwise, this function will reconnect to server) */
        playback_ip->playback_state == B_PlaybackIpState_eEnteringTrickMode)) /* a user thread called trickmode function which is being setup */
    {
        if (!playback_ip->printedOnce) {
            BDBG_MSG(("%s: playback is requesting data when %s, state %d, return w/o data",
                        __FUNCTION__,
                        playback_ip->playback_state == B_PlaybackIpState_eBuffering ? "pre-charging is going on" : "transitioning to trick mode state",
                        playback_ip->playback_state));
            playback_ip->printedOnce = true;
        }
        BKNI_ReleaseMutex(playback_ip->lock);
        BKNI_Sleep(80);
        return BFILE_ERROR_NO_DATA;
    }
    else
        playback_ip->printedOnce = false;
    origLength = length;
    byteRangeStart = file->offset;
    byteRangeEnd = byteRangeStart+length-1;
    rc = http_read_data_from_data_cache(playback_ip, file, byteRangeStart, byteRangeEnd, buf, length);
    if (rc > 0)
        playback_ip->totalConsumed += rc;
    BKNI_ReleaseMutex(playback_ip->lock);
    BDBG_MSG(("%s: returning %zd bytes", __FUNCTION__, rc));
#ifdef BDBG_DEBUG_BUILD
    B_Time_Get(&endTime);
    curTime = B_Time_Diff(&endTime, &startTime);
    if (min == 0 && max == 0) {
        min = max = curTime;
    }
    else if (curTime < min)
        min = curTime;
    else if (curTime > max)
        max = curTime;
    if (avg)
        avg = (avg + curTime)/2;
    else
        avg = curTime;
    BDBG_MSG(("%s time: min %d, cur %d, max %d avg %d", __FUNCTION__, min, curTime, max, avg));
#endif
    return rc;
}

static off_t
B_PlaybackIp_HttpNetDataSeek(bfile_io_read_t self, off_t offset, int whence)
{
    struct bfile_io_read_net *file = (struct bfile_io_read_net *) self;
    if (!file || !file->playback_ip) {
        return -1;
    }

    if (file->playback_ip->contentLength && offset > file->playback_ip->contentLength) {
        BDBG_WRN(("%s: seeking beyond EOF (content length %"PRId64 ", offset %"PRId64 ", whence %d), returning error", __FUNCTION__, file->playback_ip->contentLength, offset, whence));
        return -1;
    }

    if (whence != 0) {
        BDBG_MSG(("%s: whence is not 0: playback_ip %p, file %p, offset %"PRId64 ", whence %d\n", __FUNCTION__, (void *)file->playback_ip, (void *)file, offset, whence));
    }
#ifdef THIS_MAYNOT_BE_NEEDED
    if (file->playback_ip->playback_state == B_PlaybackIpState_eEnteringTrickMode) {
        BDBG_WRN(("%s: during trick modes transition, temporarily fail the seek call \n", __FUNCTION__));
        return -1;
    }
#endif
    BDBG_MSG(("%s: playback_ip %p, file %p, offset %"PRId64 ", whence %d, fd %d, reOpenSocket %d\n", __FUNCTION__, (void *)file->playback_ip, (void *)file, offset, whence, file->playback_ip->socketState.fd, file->playback_ip->reOpenSocket));
    file->offset = offset;
    if (file->playback_ip->playback_state != B_PlaybackIpState_eBuffering)
        /* only update lastSeekOffset if we are not buffering */
        file->playback_ip->lastSeekOffset = offset;
    return offset;
}

static int
B_PlaybackIp_HttpNetDataBounds(bfile_io_read_t self, off_t *first, off_t *last)
{
    struct bfile_io_read_net *file = (struct bfile_io_read_net *) self;

    if (!file || !file->playback_ip) {
        return -1;
    }
    /* if content length is not known (either server didn't send it or is sending content using chunk encoding */
    /* then returning -1 here puts players in loop forever mode until server closes the connection or we channel changes */
    if (file->playback_ip->contentLength == 0 || file->playback_ip->serverClosed) {
        BDBG_MSG(("%s: returning -1, content length %"PRId64 ", server closed %d\n",
                    __FUNCTION__, file->playback_ip->contentLength, file->playback_ip->serverClosed));
        return -1;
    }
    *first = 0;
    *last = file->playback_ip->contentLength;
    BDBG_MSG(("%s: playback_ip %p, file %p, last %"PRId64 ", size %"PRId64 "\n", __FUNCTION__,
                (void *)file->playback_ip, (void *)file, *last, file->playback_ip->contentLength));
    return 0;
}

static int
B_PlaybackIp_HttpNetIndexBounds(bfile_io_read_t self, off_t *first, off_t *last)
{
    struct bfile_io_read_net *file = (struct bfile_io_read_net *) self;

    if (!file || !file->playback_ip) {
        return -1;
    }
    /* if content length is not known (either server didn't send it or is sending content using chunk encoding */
    /* then returning -1 here puts players in loop forever mode until server closes the connection or we channel change */
    if (file->playback_ip->contentLength == 0 /* || file->playback_ip->serverClosed */ ) {
        BDBG_MSG(("%s: returning -1, content length %"PRId64 ", server closed %d",
                    __FUNCTION__, file->playback_ip->contentLength, file->playback_ip->serverClosed));
        return -1;
    }
    *first = 0;
    *last = file->playback_ip->contentLength;
    BDBG_MSG(("%s: playback_ip %p, file %p, last %"PRId64 "\n", __FUNCTION__, (void *)file->playback_ip, (void *)file, *last));
    return 0;
}

static off_t
B_PlaybackIp_HttpNetIndexSeek(bfile_io_read_t self, off_t offset, int whence)
{
    struct bfile_io_read_net *file = (struct bfile_io_read_net *) self;
    B_PlaybackIpHandle playback_ip;

    if (!file || !file->playback_ip) {
        return -1;
    }
    playback_ip = file->playback_ip;
    BDBG_MSG(("%s: playback_ip %p, file %p, offset %"PRId64 ", whence %d\n", __FUNCTION__, (void *)file->playback_ip, (void *)file, offset, whence));
    if (playback_ip->contentLength && offset > file->playback_ip->contentLength) {
        BDBG_WRN(("%s: seeking beyond EOF (content length %"PRId64 ", offset %"PRId64 ", whence %d), returning error", __FUNCTION__, file->playback_ip->contentLength, offset, whence));
        return -1;
    }

    if (!playback_ip->indexAtEnd && !playback_ip->setupSettings.u.http.disableRangeHeader) {
        /* to determine if index is at the end, we use following logic: */
        /* if content length is known (in most cases unless server is using HTTP chunk xfer encoding to send data), */
        /* then if requested offset is after 3/4th the content length, then index is at the end. */
        /* if content length is not known, then we check is requested offset is after 10MB. This gives enough data to determine media info */
        /* of the MPEG2/AVC TS type files, but still works for ASF & MP4 where index can be at the end. */
        if ((playback_ip->contentLength && offset > ((3*file->playback_ip->contentLength)/4)) ||
           (!playback_ip->contentLength && offset > (10*1024*1024))) {  /* ContentLength is not known in chunk encoding case */
            BDBG_MSG(("%s: index is towards end, resetting index cache: offset %"PRId64 ", content length %"PRId64 "\n", __FUNCTION__, offset, file->playback_ip->contentLength));
            if (!playback_ip->dataCache[0].inUse) {
                /* data cache is not yet setup (it can be setup if pre-buffering is already done, true for ASF files) */
                /* Copy currently read data from index to data cache as it is really the movie data & not index data */

                if (playback_ip->indexCacheDepth > playback_ip->dataCache[0].size) {
                    BDBG_MSG(("%s: index cache %u is too big to be copied into data cache %d", __FUNCTION__, playback_ip->indexCacheDepth, playback_ip->dataCache[0].size));
                    goto out;
                }
                memcpy(playback_ip->dataCache[0].cache, playback_ip->indexCache, playback_ip->indexCacheDepth);
                BDBG_MSG(("%s: copied %d bytes into data cache\n", __FUNCTION__, playback_ip->indexCacheDepth));
                /* increment data cache write index */
                playback_ip->dataCache[0].writeIndex = playback_ip->indexCacheDepth;
                /* indicate addition of this new chunk by moving forward the end offset by a chunk size */
                playback_ip->dataCache[0].endOffset = playback_ip->indexCacheEndOffset;
                /* increase data cache depth by 1 chunk size */
                playback_ip->dataCache[0].depth = playback_ip->indexCacheDepth;
                playback_ip->dataCache[0].readIndex = 0;
                playback_ip->dataCache[0].startOffset = playback_ip->indexCacheStartOffset;
                playback_ip->dataCache[0].inUse = true;
                BDBG_MSG(("%s: data cache start off %"PRId64 ", end %"PRId64 ", depth %u, wr idx %u\n",
                        __FUNCTION__, playback_ip->dataCache[0].startOffset, playback_ip->dataCache[0].endOffset, playback_ip->dataCache[0].depth, playback_ip->dataCache[0].writeIndex));
            }
            /* tell Read callback to open a separate connection to server for downloading the index (MOOV object) as it is towards the end of the file */
            playback_ip->indexAtEnd = true;
            playback_ip->indexCacheInit = 0;
        }
    }

out:
    file->offset = offset;
    playback_ip->indexSeekCount++;
    return offset;
}

#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
extern bool B_PlaybackIp_HlsSetupNextMediaSegment(B_PlaybackIpHandle playback_ip);
#endif

static ssize_t
B_PlaybackIp_HttpNetIndexRead(bfile_io_read_t self, void *buf, size_t length)
{
    int rc = -1;
    B_PlaybackIpHandle playback_ip;
    off_t byteRangeStart;
    off_t byteRangeEnd;
    int cacheMissCount = 0;
    struct bfile_io_read_net *file = (struct bfile_io_read_net *) self;
    ssize_t bytesRead = 0;
    ssize_t bytesToRead = 0;
    off_t rangeEnd;
    int spaceAvailableAtEnd, spaceLeftInCache;
    static int indexFileNameSuffix = 0;
    char recordFileName[32];
    B_Time curTime;
    int mediaProbeTime;

    if (!file || !file->playback_ip) {
        return -1;
    }
    playback_ip = file->playback_ip;
    file->fd = playback_ip->socketState.fd;
    if (playback_ip->playback_state == B_PlaybackIpState_eStopping || (playback_ip->playback_state == B_PlaybackIpState_eStopped)) {
        BDBG_MSG(("%s: returning error (-1) due to state %d change (stop event)", __FUNCTION__, playback_ip->playback_state));
        rc = -1;
        goto error;
    }
    if (playback_ip->enableRecording && playback_ip->fclearIndex == NULL) {
        memset(recordFileName, 0, sizeof(recordFileName));
        snprintf(recordFileName, sizeof(recordFileName)-1, "./videos/http_index_rec%d.ts", indexFileNameSuffix++);
        playback_ip->fclearIndex = fopen(recordFileName, "w+b");
    }
#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    if (playback_ip->hlsSessionEnabled && playback_ip->quickMediaProbe) {
        /* While doing quick probe, we only look in the first two TS packets. HLS Spec requires servers to encode PAT/PMT at the start of each segment. */
        if (file->offset >= 2 * TS_PKT_SIZE) {
            BDBG_MSG(("%s: forcing EOF to complate probe faster for HLS %zu, offset %"PRId64 "", __FUNCTION__, length, file->offset));
            return 0;
        }
        length = 2 * TS_PKT_SIZE; /* forcing the read length to only 1st two MPEG2 TS packets */
        BDBG_MSG(("%s: trimming index read request to complate probe faster for HLS, offset %"PRId64 "", __FUNCTION__, file->offset));
    }
#endif
    byteRangeStart = file->offset;
    byteRangeEnd = byteRangeStart+length-1;
    if (playback_ip->contentLength && byteRangeEnd >= playback_ip->contentLength) {
        if (byteRangeStart >= playback_ip->contentLength) {
            BDBG_MSG(("%s: asking to read beyond EOF, returning 0", __FUNCTION__));
            return 0;
        }
        byteRangeEnd = playback_ip->contentLength -1;
        BDBG_MSG(("%s: trimmed byteRangeEnd from %"PRId64 " to %"PRId64 "\n", __FUNCTION__, byteRangeStart+length,  byteRangeEnd));
        length = byteRangeEnd - byteRangeStart +1;
    }
    BDBG_MSG(("%s: socketFd %d, read %zu bytes of index data at range start %"PRId64 ", end %"PRId64 "", __FUNCTION__, playback_ip->socketState.fd, length, byteRangeStart, byteRangeEnd));

    if (playback_ip->playback_state == B_PlaybackIpState_eSessionSetupInProgress &&
        playback_ip->setupSettings.u.http.psiParsingTimeLimit) {
        if (!playback_ip->mediaProbeStartTimeNoted) {
            BDBG_MSG(("%s: parsingTimeLimit %ld", __FUNCTION__, playback_ip->setupSettings.u.http.psiParsingTimeLimit));
            B_Time_Get(&playback_ip->mediaProbeStartTime);
            playback_ip->mediaProbeStartTimeNoted = true;
        }
        else {
            B_Time_Get(&curTime);
            mediaProbeTime = B_Time_Diff(&curTime, &playback_ip->mediaProbeStartTime);
            if (mediaProbeTime >= playback_ip->setupSettings.u.http.psiParsingTimeLimit) {
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog)
                    BDBG_WRN(("%s: media probe time %d exceeded user specified limit %ld", __FUNCTION__, mediaProbeTime, playback_ip->setupSettings.u.http.psiParsingTimeLimit));
#endif
                rc = -1;
                goto error;
            }
        }
    }

#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    if (playback_ip->hlsSessionEnabled) {
        /* we have hit a socketError. This most likely means that we ran into an error during 1st segment download or didn't find PSI info in that */
        /* we continue reading the next segment until we have read & returned enough data to allow media probe to return successful */
        /* such small segments are only possible for audio only ES streams that was recently added to HLS spec */
        if (file->socketError == true) {
            if (B_PlaybackIp_HlsSetupNextMediaSegment(playback_ip) != true) {
                BDBG_ERR(("%s: coundn't connect to the server for next segment, so returning error to finish media probe", __FUNCTION__));
                rc = -1;
                goto error;
            }
            file->socketError = false;
        }
    }
#endif

    if (!playback_ip->indexCacheInit) {
        /* initialize index cache */
        /* lower the default select timeout to 1 sec */
        if (playback_ip->networkTimeout == HTTP_SELECT_TIMEOUT) {
            BDBG_MSG(("%s: lowering default select timeout from %d to 1 sec\n", __FUNCTION__, playback_ip->networkTimeout));
            playback_ip->networkTimeout = 1;
        }
        if (playback_ip->indexAtEnd) {
            /* 2nd seek operation determines if index is at the end. */
            /* In that case, we need to setup a separate connection for getting the index data */
            /* if contentLength is not known, set rangeEnd to empty */
            file->socketError = false;
            playback_ip->serverClosed = false;
            if (playback_ip->contentLength)
                rangeEnd = playback_ip->contentLength -1;
            else
                rangeEnd = 0;
            rc = B_PlaybackIp_HttpNetRangeReq(playback_ip, playback_ip->indexCache, HTTP_INDEX_CACHE_CHUNK_SIZE, byteRangeStart, rangeEnd, playback_ip->socketState.fd, &playback_ip->socketState.fd);
            if (rc < 0) {
                BDBG_ERR(("%s: ERROR: Couldn't setup socket for reading when index data is towards end, bytes read %d, length %zu\n",
                            __FUNCTION__, rc, length));
                file->socketError = true;
                playback_ip->serverClosed = false;
                /* set flag to indicate data flow to re-open the socket */
                playback_ip->reOpenSocket = true;
                goto error;
            }
            if (rc != HTTP_INDEX_CACHE_CHUNK_SIZE) {
                BDBG_MSG(("%s: couldn't read (%d) all the requested bytes (%d) from socket due to n/w error or EOF, server closed %d\n",
                            __FUNCTION__, rc, HTTP_INDEX_CACHE_CHUNK_SIZE, playback_ip->serverClosed));
            }
            playback_ip->indexCacheDepth = rc;
            playback_ip->indexCacheWriteIndex = playback_ip->indexCacheDepth;
            playback_ip->indexCacheStartOffset = byteRangeStart;
            playback_ip->indexCacheEndOffset = byteRangeStart + rc - 1;
            playback_ip->indexCacheReadIndex = 0;
            /* set flag to indicate data flow to re-open the socket */
            playback_ip->reOpenSocket = true;
            file->fd = playback_ip->socketState.fd;
            BDBG_MSG(("%s: Index at end, opened separate connx for index, start off %"PRId64 ", end %"PRId64 ", depth %u, wr idx %u\n",
                        __FUNCTION__, playback_ip->indexCacheStartOffset,
                        playback_ip->indexCacheEndOffset, playback_ip->indexCacheDepth, playback_ip->indexCacheWriteIndex));
            playback_ip->indexCacheInit = 1;
        } else if (playback_ip->initial_data_len) {
            /* some index data got read part of reading the HTTP response, copy that data into icache & initialize cache state variables */
            memcpy(playback_ip->indexCache, playback_ip->temp_buf, playback_ip->initial_data_len);
            playback_ip->indexCacheDepth = playback_ip->initial_data_len;
            playback_ip->indexCacheEndOffset = playback_ip->indexCacheDepth -1;
            playback_ip->indexCacheWriteIndex = playback_ip->indexCacheDepth;
            bytesRead = playback_ip->initial_data_len;
            BDBG_MSG(("%s: Copied initial payload of %d bytes into the index cache\n", __FUNCTION__, playback_ip->initial_data_len));
            if (playback_ip->fclearIndex) {
                fwrite(playback_ip->temp_buf, 1, playback_ip->initial_data_len, playback_ip->fclearIndex);
                fflush(playback_ip->fclearIndex);
            }
            playback_ip->initial_data_len = 0;
            playback_ip->indexCacheStartOffset = 0;
            playback_ip->indexCacheReadIndex = 0;
        }
        else {
            /* Initialize indexCache state variables. No index data was read part of initial HTTP response data */
            playback_ip->indexCacheDepth = 0;
            playback_ip->indexCacheEndOffset = 0;
            playback_ip->indexCacheWriteIndex = playback_ip->indexCacheDepth;
            playback_ip->initial_data_len = 0;
            playback_ip->indexCacheStartOffset = 0;
            playback_ip->indexCacheReadIndex = 0;
            BDBG_MSG(("%s: index cache is initialized with no initial payload data", __FUNCTION__));
        }
        playback_ip->indexCacheInit = 1;
    }
    /* else case: index cache is already initialized */
    /* check if data cache is setup */
    else if (playback_ip->dataCache[0].inUse) {
        /* both index and data cache are setup */

        /* if requested range is completely in the index cache, then return the requested data */
        if (byteRangeStart >= playback_ip->indexCacheStartOffset && byteRangeEnd <= playback_ip->indexCacheEndOffset) {
            return http_read_data_from_index_cache(playback_ip, byteRangeStart, byteRangeEnd, buf, length);
        }
        /* requested range is not completely in the index cache, */

        if (playback_ip->psi.psiValid) {
            /* we have finished the media probe (psiValid is true), so all index related data must */
            /* be in the index cache. However, since we didn't find it in the index cache, either */
            /* the index data is interleved with AV data (true for MKV formats) or */
            /* scattered at different places. In either case, we go thru the data cache for this requested range */
            bytesRead = 0;
            bytesToRead = length;
            while ((size_t)bytesRead < length) {
                if (playback_ip->playback_state == B_PlaybackIpState_eStopping || (playback_ip->playback_state == B_PlaybackIpState_eStopped)) {
                    BDBG_MSG(("%s: breaking out of read loop due to state %d change (stop event)", __FUNCTION__, playback_ip->playback_state));
                    rc = -1;
                    goto error;
                }
                if (playback_ip->playback_state == B_PlaybackIpState_eSessionSetupInProgress && playback_ip->setupSettings.u.http.psiParsingTimeLimit) {
                    /* check if we have exceeded the media probe time */
                    B_Time_Get(&curTime);
                    mediaProbeTime = B_Time_Diff(&curTime, &playback_ip->mediaProbeStartTime);
                    if (mediaProbeTime >= playback_ip->setupSettings.u.http.psiParsingTimeLimit) {
#ifdef BDBG_DEBUG_BUILD
                        if (playback_ip->ipVerboseLog)
                            BDBG_WRN(("%s: media probe time %d exceeded user specified limit %ld", __FUNCTION__, mediaProbeTime, playback_ip->setupSettings.u.http.psiParsingTimeLimit));
#endif
                        rc = -1;
                        goto error;
                    }
                }
                if ((rc = http_read_data_from_data_cache(playback_ip, file, byteRangeStart, byteRangeEnd, buf, bytesToRead)) <= 0) {
                    if (playback_ip->selectTimeout && !playback_ip->serverClosed) {
                        /* this is a hack just to appease the broken DLNA CTT server as it doesn't close the connection after it done sending data */
                        if (playback_ip->setupSettings.u.http.disableRangeHeader) {
                            BDBG_ERR(("%s: ERROR: Can't seek when Range Header is not supported\n", __FUNCTION__));
                            return -1;
                        }
                        BDBG_MSG(("%s: select timeout, so retry read", __FUNCTION__));
                        continue;
                    }
                    BDBG_WRN(("%s: rc (%d) == 0 (EOF), failed to read index data via the data cache, selectTimeout %d", __FUNCTION__, rc, playback_ip->selectTimeout));
                    bytesRead = rc;
                    break;
                }
                bytesRead += rc;
                bytesToRead -= rc;
                BDBG_MSG(("%s: read %d bytes from data_cache, total read so far %zd, remaining to read %zu", __FUNCTION__, rc, bytesRead, length));
            }
            return bytesRead;
        }
        else {
            BDBG_MSG(("%s: requested range is neither in index or data cache, since probing is still going on, go thru index cache", __FUNCTION__));
        }
    }

    /* index cache is already setup and data cache is not yet setup, that means we are still in the media probe phase, continue building up index cache */
    if (byteRangeStart < playback_ip->indexCacheStartOffset || (byteRangeStart > (playback_ip->indexCacheEndOffset + playback_ip->indexCacheSize)) || playback_ip->reOpenIndexSocket ) {
        /* requested byte range start is not cacheable, so modify the servers sending range */
        if (playback_ip->setupSettings.u.http.disableRangeHeader) {
            BDBG_ERR(("%s: ERROR: Can't seek when Range Header is not supported\n", __FUNCTION__));
            return -1;
        }
        file->socketError = false;
        playback_ip->serverClosed = false;
        rc = B_PlaybackIp_HttpNetRangeReq(playback_ip, playback_ip->indexCache, HTTP_INDEX_CACHE_CHUNK_SIZE, byteRangeStart, 0, playback_ip->socketState.fd, &playback_ip->socketState.fd);
        if (rc < 0) {
            BDBG_ERR(("%s: ERROR: Couldn't setup socket for reading when index data is towards end, bytes read %d, length %zu\n", __FUNCTION__, rc, length));
            file->socketError = true;
            playback_ip->serverClosed = false;
            playback_ip->reOpenSocket = true;
            goto error;
        }
        if (rc != HTTP_INDEX_CACHE_CHUNK_SIZE) {
            BDBG_MSG(("%s: couldn't read (%d) all the requested bytes (%d) from socket due to n/w error or EOF, server closed %d\n",
                        __FUNCTION__, rc, HTTP_INDEX_CACHE_CHUNK_SIZE, playback_ip->serverClosed));
            playback_ip->serverClosed = false;
            playback_ip->reOpenSocket = true;
        }
        playback_ip->reOpenIndexSocket = false;
        playback_ip->indexCacheDepth = rc;
        playback_ip->indexCacheWriteIndex = rc;
        playback_ip->indexCacheStartOffset = byteRangeStart;
        playback_ip->indexCacheEndOffset = byteRangeStart + rc - 1;
        playback_ip->indexCacheReadIndex = 0;
        /* set flag to indicate data flow to re-open the socket */
        playback_ip->reOpenSocket = true;
        file->fd = playback_ip->socketState.fd;
        BDBG_MSG(("%s: Index range not cacheable, re-requested new range , start off %"PRId64 ", end %"PRId64 ", depth %u, wr idx %u\n",
                    __FUNCTION__, playback_ip->indexCacheStartOffset,
                    playback_ip->indexCacheEndOffset, playback_ip->indexCacheDepth, playback_ip->indexCacheWriteIndex));
    }

    while (byteRangeEnd > playback_ip->indexCacheEndOffset && file->socketError == false) {
        /* requested range is not in the index cache, so try to read more data from server */
        BDBG_MSG(("%s: iCache miss count %d, range end %"PRId64 ", cache end %"PRId64 "\n", __FUNCTION__, cacheMissCount, byteRangeEnd, playback_ip->indexCacheEndOffset));
        if (playback_ip->playback_state == B_PlaybackIpState_eStopping || (playback_ip->playback_state == B_PlaybackIpState_eStopped)) {
            BDBG_MSG(("%s: breaking out of read loop due to state %d change (stop event)", __FUNCTION__, playback_ip->playback_state));
            rc = -1;
            goto error;
        }

        if (playback_ip->playback_state == B_PlaybackIpState_eSessionSetupInProgress && playback_ip->setupSettings.u.http.psiParsingTimeLimit) {
            /* check if we have exceeded the media probe time */
            B_Time_Get(&curTime);
            mediaProbeTime = B_Time_Diff(&curTime, &playback_ip->mediaProbeStartTime);
            if (mediaProbeTime >= playback_ip->setupSettings.u.http.psiParsingTimeLimit) {
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog)
                    BDBG_WRN(("%s: media probe time %d exceeded user specified limit %ld", __FUNCTION__, mediaProbeTime, playback_ip->setupSettings.u.http.psiParsingTimeLimit));
#endif
                rc = -1;
                goto error;
            }
        }

        /* since we always try to read fixed chunk size data into icache, see if some data was already read part of the initial HTTP resp processing */
        if (bytesRead) {
            bytesToRead = HTTP_INDEX_CACHE_CHUNK_SIZE - bytesRead;
        }
        else {
            bytesToRead = HTTP_INDEX_CACHE_CHUNK_SIZE;
        }
#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    if (playback_ip->hlsSessionEnabled && playback_ip->quickMediaProbe) {
        bytesToRead = 2 * TS_PKT_SIZE; /* forcing the read length to only 1st two MPEG2 TS packets as HLS requires PAT &PMT to be present at the start of each segment. */
        BDBG_MSG(("%s: trimming index read request to complate probe faster for HLS, offset %"PRId64 "", __FUNCTION__, file->offset));
    }
#endif

        /* adjust bytesToRead: read lower of: 1) index chunk size, 2) space available at end of cache (size-wrIndex), 3) actual space left in the cache (size-depth) */
        spaceAvailableAtEnd = playback_ip->indexCacheSize - playback_ip->indexCacheWriteIndex;
        spaceLeftInCache = playback_ip->indexCacheSize - playback_ip->indexCacheDepth;
        if (spaceAvailableAtEnd < bytesToRead || (spaceLeftInCache && spaceLeftInCache < bytesToRead)) {
            bytesToRead = spaceAvailableAtEnd > spaceLeftInCache ? spaceLeftInCache : spaceAvailableAtEnd;
            BDBG_MSG(("%s: reached towards end of index cache or cache getting full, so reading (%zd) less than chunk size (%d), cache size %d, cache wr idx %d, depth %d, spaceAvailAtEnd %d, spaceLeftInCache %d",
                    __FUNCTION__, bytesToRead, HTTP_INDEX_CACHE_CHUNK_SIZE, playback_ip->indexCacheSize, playback_ip->indexCacheWriteIndex,
                    playback_ip->indexCacheDepth, spaceAvailableAtEnd, spaceLeftInCache));
        }
        /* if index cache is full, we retire the oldest entry from the cache */
        if (playback_ip->indexCacheDepth == playback_ip->indexCacheSize && !file->socketError) {
            playback_ip->indexCacheDepth -= HTTP_INDEX_CACHE_CHUNK_SIZE;
            playback_ip->indexCacheStartOffset += HTTP_INDEX_CACHE_CHUNK_SIZE;
            playback_ip->indexCacheReadIndex += HTTP_INDEX_CACHE_CHUNK_SIZE;
            playback_ip->indexCacheReadIndex %= playback_ip->indexCacheSize;
            BDBG_MSG(("%s: icache wrap: depth %d, size %d, rd idx %d, offset start %"PRId64 ", end %"PRId64 "\n",
                        __FUNCTION__, playback_ip->indexCacheDepth, playback_ip->indexCacheSize, playback_ip->indexCacheReadIndex,
                        playback_ip->indexCacheStartOffset, playback_ip->indexCacheEndOffset));

        }
        if (playback_ip->indexCacheDepth > playback_ip->indexCacheSize) {
            BDBG_MSG(("%s: icache pointers bug: depth %d, size %d, rd idx %d, offset start %"PRId64 ", end %"PRId64 "\n",
                        __FUNCTION__, playback_ip->indexCacheDepth, playback_ip->indexCacheSize, playback_ip->indexCacheReadIndex,
                        playback_ip->indexCacheStartOffset, playback_ip->indexCacheEndOffset));
            BDBG_ASSERT(NULL);
            goto error;
        }

        rc = playback_ip_read_socket(playback_ip, playback_ip->securityHandle, file->fd, playback_ip->indexCache + playback_ip->indexCacheWriteIndex, bytesToRead, playback_ip->networkTimeout);
        if (rc <= 0 ) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_ERR(("%s: read failed, rc %d, Network timeout (%d) or EOF (%d)\n", __FUNCTION__, rc, playback_ip->selectTimeout, playback_ip->serverClosed));
#endif
            if (playback_ip->selectTimeout && !playback_ip->serverClosed) {
                /* this is a hack just to appease the broken DLNA CTT server as it doesn't close the connection after it done sending data */
                if (playback_ip->setupSettings.u.http.disableRangeHeader) {
                    BDBG_ERR(("%s: ERROR: Can't seek when Range Header is not supported\n", __FUNCTION__));
                    return -1;
                }
                BDBG_MSG(("select timeout, so retry read\n"));
                continue;
            }
            file->socketError = true;
            playback_ip->serverClosed = false;
            /* set flag to indicate data flow to re-open the socket */
            playback_ip->reOpenSocket = true;
            goto error;
        }
        if (rc != bytesToRead) {
            BDBG_MSG(("%s: couldn't read (%d) all the requested bytes (%d) from socket due to n/w timeout (%d) or EOF (%d)\n", __FUNCTION__, rc, HTTP_INDEX_CACHE_CHUNK_SIZE, playback_ip->selectTimeout, playback_ip->serverClosed));
            bytesRead += rc;
            if (!playback_ip->selectTimeout) {
                /* only consider this as an error if select hasn't timed out */
                file->socketError = true;
                /* set flag to indicate data flow to re-open the socket */
                playback_ip->reOpenSocket = true;
                playback_ip->serverClosed = false;
            }
        }
        else {
            bytesRead = 0;
        }
        if (playback_ip->fclearIndex) {
            fwrite(playback_ip->indexCache + playback_ip->indexCacheWriteIndex, 1, rc , playback_ip->fclearIndex);
            fflush(playback_ip->fclearIndex);
        }
        playback_ip->indexCacheDepth += rc;
        if (playback_ip->indexCacheEndOffset)
            playback_ip->indexCacheEndOffset += rc;
        else
            playback_ip->indexCacheEndOffset = rc-1;
        playback_ip->indexCacheWriteIndex += rc;
        playback_ip->indexCacheWriteIndex %= playback_ip->indexCacheSize;
        BDBG_MSG(("%s: read %d bytes into index cache of depth %d, size %d bytes, next wr index %d\n",
                    __FUNCTION__, rc, playback_ip->indexCacheDepth, playback_ip->indexCacheSize, playback_ip->indexCacheWriteIndex));
        cacheMissCount++;
    }

    if (file->socketError == true) {
        /* server has closed connection, check if the requested range is with-in cache */
        if (byteRangeStart > playback_ip->indexCacheEndOffset) {
            BDBG_MSG(("%s: requested range is not in index: range start %"PRIu64 ", end %"PRIu64 ", cache depth %d, end off %"PRId64 ", length %zu\n",
                    __FUNCTION__, byteRangeStart, byteRangeEnd, playback_ip->indexCacheDepth, playback_ip->indexCacheEndOffset, length));
            rc = 0;
            goto error;
        }
        /* so we know atleast the byteRangeStart is in the cache, trim the byteRangeEnd if it is outside the Cache */
        if (byteRangeEnd > playback_ip->indexCacheEndOffset) {
            byteRangeEnd = playback_ip->indexCacheEndOffset;
            length = byteRangeEnd - byteRangeStart + 1;
            BDBG_MSG(("%s: Server has already closed connection, so resetting byteRangeEnd to %"PRId64 ", length to %zu\n", __FUNCTION__, byteRangeEnd, length));
        }
    }

    if (byteRangeEnd <= playback_ip->indexCacheEndOffset) {
        return http_read_data_from_index_cache(playback_ip, byteRangeStart, byteRangeEnd, buf, length);
    }
    else {
        BDBG_WRN(("%s: SW bug: requested range is not in index: range start %"PRIu64 ", end %"PRIu64 ", cache depth %d, end off %"PRId64 ", length %zu\n",
                    __FUNCTION__, byteRangeStart, byteRangeEnd, playback_ip->indexCacheDepth, playback_ip->indexCacheEndOffset, length));
        rc = -1;
    }

error:
    return rc;
}

/* allocate memory for caches */
static int
_http_av_cache_alloc(B_PlaybackIpAvCache *avCache, int size)
{
    avCache->cache = (char *)BKNI_Malloc(size);
    if (avCache->cache == NULL) {
        BDBG_ERR(("%s: Failed to allocate data cache of %d bytes\n", __FUNCTION__, size));
        return -1;
    }
    BDBG_MSG(("%s: Allocated %d byte size data cache, buf %p", __FUNCTION__, size, (void *)avCache->cache));
    avCache->size = size;
    avCache->depth = 0;
    return 0;
}

void
http_av_cache_dealloc(B_PlaybackIpHandle playback_ip)
{
    int i;
    for (i=0; i<HTTP_MAX_DATA_CACHES; i++) {
        if (playback_ip->dataCache[i].cache) {
            BKNI_Free(playback_ip->dataCache[i].cache);
            playback_ip->dataCache[i].cache = NULL;
            playback_ip->dataCache[i].inUse = false;
        }
    }
    if (playback_ip->indexCache) {
        BKNI_Free(playback_ip->indexCache);
        playback_ip->indexCache = NULL;
    }
    BDBG_MSG(("%s: Freed Data & Index Caches\n", __FUNCTION__));
}

B_PlaybackIpError
http_av_cache_alloc(B_PlaybackIpHandle playback_ip)
{
    int i;
    int cacheSize;
    /* allocate data cache */
    if (playback_ip->openSettings.u.http.networkBufferSize > HTTP_DATA_CACHE_SIZE) {
        /* increase the user requested cache size to the next multiple of data chunk size */
        cacheSize = playback_ip->openSettings.u.http.networkBufferSize + (HTTP_DATA_CACHE_CHUNK_SIZE - (playback_ip->openSettings.u.http.networkBufferSize%HTTP_DATA_CACHE_CHUNK_SIZE));
    }
    else
        cacheSize = HTTP_DATA_CACHE_SIZE;
    BDBG_MSG(("asked data cache size %d, new size %d\n", playback_ip->openSettings.u.http.networkBufferSize, cacheSize));
    for (i=0; i<HTTP_MAX_DATA_CACHES; i++) {
        if (_http_av_cache_alloc(&playback_ip->dataCache[i], cacheSize) < 0) {
            BDBG_ERR(("%s: Failed to create data cache # %d\n", __FUNCTION__, i));
            goto error;
        }
        playback_ip->dataCache[i].inUse = false;
    }

    /* allocate index cache */
    if (playback_ip->openSettings.u.http.networkBufferSize > HTTP_INDEX_CACHE_SIZE) {
        /* increase the user requested cache size to the next multiple of index chunk size */
        cacheSize = playback_ip->openSettings.u.http.networkBufferSize + (HTTP_INDEX_CACHE_CHUNK_SIZE - (playback_ip->openSettings.u.http.networkBufferSize%HTTP_INDEX_CACHE_CHUNK_SIZE));
    }
    else
        cacheSize = HTTP_INDEX_CACHE_SIZE;
    BDBG_MSG(("asked index cache size %d, new size %d\n", playback_ip->openSettings.u.http.networkBufferSize, cacheSize));
    playback_ip->indexCacheSize = cacheSize;
    playback_ip->indexCache = (char *)BKNI_Malloc(cacheSize);
    if (playback_ip->indexCache == NULL) {
        BDBG_ERR(("%s: Failed to allocate index cache of %d bytes\n", __FUNCTION__, playback_ip->indexCacheSize));
        goto error;
    }
    BDBG_MSG(("%s: Allocated %d byte size index cache", __FUNCTION__, playback_ip->indexCacheSize));
    playback_ip->indexCacheDepth = 0;

    return B_ERROR_SUCCESS;
error:
    http_av_cache_dealloc(playback_ip);
    return B_ERROR_OUT_OF_MEMORY;
}

void
B_PlaybackIp_HttpDestroyFilePlayHandle(
    B_PlaybackIpHandle playback_ip
    )
{
    if (playback_ip->file) {
        BKNI_Free(playback_ip->file);
        playback_ip->file = NULL;
    }
    if (playback_ip->fileBuffered) {
        bfile_buffered_detach(playback_ip->fileBuffered);
        bfile_buffered_destroy(playback_ip->fileBuffered);
        playback_ip->fileBuffered = NULL;
    }
    if (playback_ip->factory) {
        batom_factory_destroy(playback_ip->factory);
        playback_ip->factory = NULL;
    }

    http_av_cache_dealloc(playback_ip);
}

void
B_PlaybackIp_HttpSessionClose(B_PlaybackIpHandle playback_ip)
{
    BDBG_MSG(("%s: ", __FUNCTION__));

    B_PlaybackIp_HttpDestroyFilePlayHandle(playback_ip);

    if (playback_ip->securityHandle) {
        playback_ip->netIo.close(playback_ip->securityHandle);
        playback_ip->securityHandle = NULL;
    }

    if (playback_ip->responseMessage) {
        BKNI_Free(playback_ip->responseMessage);
        playback_ip->responseMessage = NULL;
    }

    if (playback_ip->psi.playSpeedString) {
        BKNI_Free(playback_ip->psi.playSpeedString);
        playback_ip->psi.playSpeedString = NULL;
    }
    playback_ip->netIo.read = NULL;
    playback_ip->netIo.write = NULL;
    playback_ip->netIo.close = NULL;
    playback_ip->netIo.suspend = NULL;
    playback_ip->netIo.shutdown = NULL;
    playback_ip->openSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_None;

    playback_ip->indexCacheInit = 0;
    playback_ip->indexSeekCount = 0;
    playback_ip->indexAtEnd = false;
    playback_ip->contentLength = 0;
    playback_ip->preChargeBuffer = false;
    playback_ip->lastUsedCacheIndex = 0;
    playback_ip->cacheIndexUsingSocket = 0;
    playback_ip->serverClosed = false;
    playback_ip->socketClosed = false;
    playback_ip->reOpenSocket = false;
    playback_ip->tempBytesRead = 0;
    playback_ip->lastSeekOffset = 0;
    /* coverity[missing_lock] */
    playback_ip->cacheDepthFudgeFactor = 0;
    playback_ip->statusCode = 0;
    playback_ip->firstDataReadCallbackDone = false;
    playback_ip->waveHeaderInserted = false;
    playback_ip->indexCacheEndOffset = 0;

    if (playback_ip->stream) {
        bmedia_probe_stream_free(playback_ip->probe, playback_ip->stream);
        playback_ip->stream = NULL;
    }
    if (playback_ip->probe) {
        bmedia_probe_destroy(playback_ip->probe);
        playback_ip->probe = NULL;
    }

    if (playback_ip->sessionOpenThread) {
        /* destroy thread that was created during SessionOpen */
        /* it is not destroyed during sessionOpen completion as app can re-invoke sessionOpen in the callback itself */
        B_Thread_Destroy(playback_ip->sessionOpenThread);
        playback_ip->sessionOpenThread = NULL;
        BDBG_MSG(("%s: destroying temporary thread created during HTTP session Open", __FUNCTION__));
    }
    if (playback_ip->sessionSetupThread) {
        /* destroy thread that was created during SessionSetup */
        /* it is not destroyed during sessionSetup completion as app can re-invoke sessionSetup in the callback itself */
        B_Thread_Destroy(playback_ip->sessionSetupThread);
        playback_ip->sessionSetupThread = NULL;
        BDBG_MSG(("%s: destroying temporary thread created during HTTP session setup", __FUNCTION__));
    }
    if (playback_ip->trickModeThread) {
        /* destroy thread that was created during 1st non-blocking trickmode call and re-used for all subsequent trickmode calls */
        B_Thread_Destroy(playback_ip->trickModeThread);
        playback_ip->trickModeThread = NULL;
        BDBG_MSG(("%s: destroying temporary thread created during HTTP trickmode setup", __FUNCTION__));
    }
    if (playback_ip->newTrickModeJobEvent) {
        BKNI_DestroyEvent(playback_ip->newTrickModeJobEvent);
        playback_ip->newTrickModeJobEvent = NULL;
    }
    if (playback_ip->openSettings.u.http.additionalHeaders) {
        BKNI_Free(playback_ip->openSettings.u.http.additionalHeaders);
        playback_ip->openSettings.u.http.additionalHeaders = NULL;
    }
    if (playback_ip->openSettings.u.http.userAgent) {
        BKNI_Free(playback_ip->openSettings.u.http.userAgent);
        playback_ip->openSettings.u.http.userAgent = NULL;
    }
    if (playback_ip->cookieFoundViaHttpRedirect)
        BKNI_Free(playback_ip->cookieFoundViaHttpRedirect);
#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    if (playback_ip->hlsSessionEnabled)
        B_PlaybackIp_HlsSessionDestroy(playback_ip);
#endif
#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
    if (playback_ip->mpegDashSessionEnabled)
        B_PlaybackIp_MpegDashSessionDestroy(playback_ip);
#endif

    if (playback_ip->fclear) {
        fclose(playback_ip->fclear);
    }
    if (playback_ip->fclearIndex) {
        fclose(playback_ip->fclearIndex);
    }
    BDBG_MSG(("%s: Done", __FUNCTION__));
}

void B_PlaybackIp_HttpNetClose(
    NEXUS_FilePlayHandle file
    )
{
    struct bfile_in_net *filePlay = (struct bfile_in_net *)file;

    BDBG_MSG(("%s: \n", __FUNCTION__));
    B_PlaybackIp_HttpSessionClose(filePlay->data.playback_ip);
    return;
}

static const struct bfile_io_read net_io_data_read = {
    B_PlaybackIp_HttpNetDataRead,
    B_PlaybackIp_HttpNetDataSeek,
    B_PlaybackIp_HttpNetDataBounds,
    BIO_DEFAULT_PRIORITY
};

static const struct bfile_io_read net_io_index_read = {
    B_PlaybackIp_HttpNetIndexRead,
    B_PlaybackIp_HttpNetIndexSeek,
    B_PlaybackIp_HttpNetIndexBounds,
    BIO_DEFAULT_PRIORITY
};

NEXUS_FilePlayHandle
B_PlaybackIp_HttpCreateFilePlayHandle(
    B_PlaybackIpHandle playback_ip,
    bfile_io_read_t *bufferedFdIndex
    )
{
    struct bfile_in_net *filePlay;
    bfile_buffered_cfg file_buffered_cfg;

    filePlay = BKNI_Malloc(sizeof(*filePlay));
    if (!filePlay) {
        BDBG_ERR(("%s: memory allocation failure\n", __FUNCTION__));
        return NULL;
    }
    BKNI_Memset(filePlay, 0, sizeof(*filePlay));
    BDBG_MSG(("%s: playback_ip %p, filePlay %p, data %p, index %p, size of file struct %zu\n",
                __FUNCTION__, (void *)playback_ip, (void *)filePlay, (void *)&filePlay->data, (void *)&filePlay->index, sizeof(*filePlay)));

    filePlay->data.fd = playback_ip->socketState.fd;
    filePlay->data.playback_ip = playback_ip;
    filePlay->data.socketError = false;
    filePlay->data.self = net_io_data_read;
    filePlay->self.file.data = &(filePlay->data.self);

    playback_ip->file = filePlay;
    filePlay->index.fd = playback_ip->socketState.fd;
    filePlay->index.playback_ip = playback_ip;
    filePlay->index.socketError = false;
    filePlay->index.self = net_io_index_read;
    filePlay->self.file.index = &(filePlay->index.self);
    filePlay->self.file.close = B_PlaybackIp_HttpNetClose;
    /* create buffered cache for index so that this gets cached inside the player */
    playback_ip->factory = batom_factory_create(bkni_alloc, 256);
    BDBG_ASSERT(playback_ip->factory);

    bfile_buffered_default_cfg(&file_buffered_cfg);
    file_buffered_cfg.nsegs = 128;
    file_buffered_cfg.buf_len = file_buffered_cfg.nsegs * (4096*4);
    playback_ip->fileBuffered = bfile_buffered_create(playback_ip->factory, &file_buffered_cfg);
    BDBG_ASSERT(playback_ip->fileBuffered);

    *bufferedFdIndex = bfile_buffered_attach(playback_ip->fileBuffered, &(filePlay->index.self));
    BDBG_ASSERT(*bufferedFdIndex);
    filePlay->self.file.index = *bufferedFdIndex;

    if (http_av_cache_alloc(playback_ip) != B_ERROR_SUCCESS)
        goto error;
    return &filePlay->self;

error:
    http_av_cache_dealloc(playback_ip);
    if (filePlay) {
        BKNI_Free(filePlay);
    }
    if (playback_ip->fileBuffered) {
        bfile_buffered_detach(playback_ip->fileBuffered);
        bfile_buffered_destroy(playback_ip->fileBuffered);
        playback_ip->fileBuffered = NULL;
    }
    if (playback_ip->factory){
        batom_factory_destroy(playback_ip->factory);
        playback_ip->factory = NULL;
    }
    return (NULL);
}

void
B_PlaybackIp_HttpParseRespHeaderForPsi(
    B_PlaybackIpHandle playback_ip,
    NEXUS_TransportType http_content_type,
    char *http_hdr,
    B_PlaybackIpPsiInfoHandle psi
    )
{
    char *p_psi;
    char *end_str;
    BSTD_UNUSED(playback_ip);

    memset(psi, 0, sizeof(B_PlaybackIpPsiInfo));
    p_psi = B_PlaybackIp_UtilsStristr(http_hdr, "FrameRateInTrickMode.dlna.org:");
    if (p_psi) {
        p_psi += strlen("FrameRateInTrickMode.dlna.org:");
        end_str = strstr(p_psi, "\r\n");
        if (end_str) {
            char *tmp1 = NULL, *tmp2 = NULL;
            end_str[0] = '\0';
            if ( (tmp1 = B_PlaybackIp_UtilsStristr(p_psi, "rate=")) != NULL || (tmp2 = B_PlaybackIp_UtilsStristr(p_psi, "rate =")) != NULL) {
                if (tmp1) p_psi = tmp1 + strlen("rate=");
                else if (tmp2) p_psi = tmp2 + strlen("rate =");
                psi->frameRateInTrickMode = strtol(p_psi, (char **)NULL, 10);
            }
            end_str[0] = '\r';
            BDBG_MSG(("FrameRateInTrickMode.dlna.org: rate= %d\n",psi->frameRateInTrickMode));
        }
    }

    p_psi = B_PlaybackIp_UtilsStristr(http_hdr, "DLNA.ORG_PS=");
    if (p_psi) {
#define HTTP_PLAY_SPEED_STRING_LENGTH 256
        char playSpeed[256] = {0,};
        char *p_end, *p_start;
        int ps_count = 0;

        p_psi += strlen("DLNA.ORG_PS=");
        end_str = strstr(p_psi, "\r\n");
        if (end_str) {
            end_str[0] = '\0';
            if(strlen(p_psi) > 255) {
                BDBG_MSG(("playSpeeds are too many! Truncate!"));
                *(p_psi + 255) = 0;
            }
            strncpy(playSpeed, p_psi, 255);
            /* also copy the playSpeed in the string formats for apps */
            psi->playSpeedString = BKNI_Malloc(HTTP_PLAY_SPEED_STRING_LENGTH);
            if (psi->playSpeedString) {
                BKNI_Memset(psi->playSpeedString, 0, HTTP_PLAY_SPEED_STRING_LENGTH);
                strncpy(psi->playSpeedString, p_psi, HTTP_PLAY_SPEED_STRING_LENGTH-1);
            }
            end_str[0] = '\r';
            BDBG_MSG(("playSpeed : %s\n",playSpeed));
        }

        /* parse and store play speed set */
        p_start = playSpeed;
        while((p_end = strstr(playSpeed, ",")) != NULL) {
            *p_end = 0;
            BDBG_MSG(("PS[%d] = %s", ps_count, p_start));
            psi->playSpeed[ps_count++] = strtol(p_start, (char **)NULL, 10);
            *p_end = ' ';
            p_start = p_end; /* next */
            if(ps_count >= IP_PVR_PLAYSPEED_MAX_COUNT-1)
                break;
        }
        if(*p_start != '\0') {
            psi->playSpeed[ps_count++] = strtol(p_start, (char **)NULL, 10);
            BDBG_MSG(("PS[%d] = %d", ps_count-1, psi->playSpeed[ps_count-1]));
        }
        psi->numPlaySpeedEntries = ps_count;
        BDBG_MSG(("Server trickmodes supported: # of speeds %d", psi->numPlaySpeedEntries));
    }

    if (http_content_type == NEXUS_TransportType_eTs) {
        psi->mpegType = NEXUS_TransportType_eTs;
        BDBG_MSG(("HTTP Content Type: MPEG2 TS\n"));
        psi->psiValid = false;
#if 0
        /* Debug code to hardcode the AV PIDS. */
        psi->pmtPid = 0x10a4;
        psi->pcrPid = 0x10a5;
        psi->videoPid = 0x823;
        psi->videoCodec = NEXUS_VideoCodec_eMpeg2;
        psi->audioPid = 0x10a5;
        psi->audioCodec = NEXUS_AudioCodec_eAc3;
        psi->psiValid = true;
        BDBG_WRN((">>>>>>>>>>>>>>.. HARDCODING the pids...."));
        return ;
#endif

        p_psi = strstr(http_hdr,"BCM-LiveChannel: ");
        if (p_psi)
        {
            p_psi += strlen("BCM-LiveChannel: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                int tmp;
                end_str[0] = '\0';
                tmp = strtol(p_psi, (char **)NULL, 10);
                if (tmp > 0)
                    psi->liveChannel = true;
                else
                    psi->liveChannel = false;
                end_str[0] = '\r';
                BDBG_MSG(("BCM-LiveChannel: %d",psi->liveChannel));
            }
        }
        else
            psi->liveChannel = false;

        p_psi = strstr(http_hdr,"Video-Pid: ");
        if (p_psi) {
            p_psi += strlen("Video-Pid: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                end_str[0] = '\0';
                psi->videoPid = strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                BDBG_MSG(("video-PID: %d\n",psi->videoPid));
            }
        }

        p_psi = strstr(http_hdr,"Video-Type: ");
        if (p_psi)
        {
            p_psi += strlen("Video-Type: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                int videoCodec;
                end_str[0] = '\0';
                videoCodec = (NEXUS_VideoCodec)strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                if (strstr(http_hdr,"SDK/1.0"))
                    /* older streamer "httpstreamer" (SDK/1.0) uses Settop API types, so we need to convert it to Nexus types */
                    psi->videoCodec = B_PlaybackIp_UtilsVideoCodec2Nexus(videoCodec);
                else
                    /* new streamer "ip_streamer" SDK/2.0 uses Nexus types, so we dont any conversion */
                    psi->videoCodec = videoCodec;
                BDBG_MSG(("video-Type: %d\n",psi->videoCodec));
                psi->psiValid = true;
            }
        }

        p_psi = strstr(http_hdr,"Video-Width: ");
        if (p_psi) {
            p_psi += strlen("Video-Width: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                end_str[0] = '\0';
                psi->videoWidth = strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                BDBG_MSG(("video-width: %d\n",psi->videoWidth));
            }
        }

        p_psi = strstr(http_hdr,"Video-Height: ");
        if (p_psi) {
            p_psi += strlen("Video-Height: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                end_str[0] = '\0';
                psi->videoHeight = strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                BDBG_MSG(("video-height: %d\n",psi->videoHeight));
            }
        }

        p_psi = strstr(http_hdr,"Video-ColorDepth: ");
        if (p_psi) {
            p_psi += strlen("Video-ColorDepth: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                end_str[0] = '\0';
                psi->colorDepth = strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                BDBG_MSG(("Video-ColorDepth: %d\n",psi->colorDepth));
            }
        }

        p_psi = strstr(http_hdr,"Extra-Video-Pid: ");
        if (p_psi) {
            p_psi += strlen("Extra-Video-Pid: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                end_str[0] = '\0';
                psi->extraVideoPid = strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                BDBG_MSG(("extra-video-PID: %d\n",psi->extraVideoPid));
            }
        }

        p_psi = strstr(http_hdr,"Extra-Video-Type: ");
        if (p_psi)
        {
            p_psi += strlen("Extra-Video-Type: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                int videoCodec;
                end_str[0] = '\0';
                videoCodec = (NEXUS_VideoCodec)strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                if (strstr(http_hdr,"SDK/1.0"))
                    /* older streamer "httpstreamer" (SDK/1.0) uses Settop API types, so we need to convert it to Nexus types */
                    psi->extraVideoCodec = B_PlaybackIp_UtilsVideoCodec2Nexus(videoCodec);
                else
                    /* new streamer "ip_streamer" SDK/2.0 uses Nexus types, so we dont any conversion */
                    psi->extraVideoCodec = videoCodec;
                BDBG_MSG(("extra-video-Type: %d\n",psi->extraVideoCodec));
            }
        }

        p_psi = strstr(http_hdr,"Pcr-Pid: ");
        if (p_psi)
        {
            p_psi += strlen("Pcr-Pid: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                end_str[0] = '\0';
                psi->pcrPid= strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                BDBG_MSG(("pcr-PID: %d\n",psi->pcrPid));
            }
        }

        p_psi = strstr(http_hdr,"Audio-Pid: ");
        if (p_psi)
        {
            p_psi += strlen("Audio-Pid: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                end_str[0] = '\0';
                psi->audioPid = strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                BDBG_MSG(("audio-PID: %d\n",psi->audioPid));
            }
        }

        p_psi = strstr(http_hdr,"Audio-Type: ");
        if (p_psi)
        {
            p_psi += strlen("Audio-Type: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                int audioCodec;
                end_str[0] = '\0';
                audioCodec = (NEXUS_AudioCodec)strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                if (strstr(http_hdr,"SDK/1.0"))
                    /* older streamer "httpstreamer" (SDK/1.0) uses Settop API types, so we need to convert it to Nexus types */
                    psi->audioCodec = B_PlaybackIp_UtilsAudioCodec2Nexus(audioCodec);
                else
                    /* new streamer "ip_streamer" SDK/2.0 uses Nexus types, so we dont any conversion */
                    psi->audioCodec = audioCodec;
                BDBG_MSG(("audio-Type: %d\n",psi->audioCodec));
                psi->psiValid = true;
            }
        }

        p_psi = strstr(http_hdr,"TTS: ");
        if (p_psi)
        {
            p_psi += strlen("TTS: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                end_str[0] = '\0';
                psi->transportTimeStampEnabled = strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                BDBG_MSG(("TTS: %d\n",psi->transportTimeStampEnabled));
            }
        }
        else
            psi->transportTimeStampEnabled = false;

        p_psi = strstr(http_hdr,"MediaDuration: ");
        if (p_psi)
        {
            p_psi += strlen("MediaDuration: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                end_str[0] = '\0';
                psi->duration = strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                BDBG_MSG(("media-Duration[sec]: %d\n",psi->duration));
                psi->duration *= 1000; /* in msec */
            }
        }
        p_psi = strstr(http_hdr,"First-PTS: ");
        if (p_psi)
        {
            p_psi += strlen("First-PTS: ");
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                end_str[0] = '\0';
                psi->firstPts = strtoul(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                BDBG_MSG(("first-PTS: %u\n",psi->firstPts));
            }
        }

        p_psi = strstr(http_hdr,"BCM-Min-Iframe-Speed: ");
        if (p_psi)
        {
            p_psi += strlen("BCM-Min-Iframe-Speed: ");
            psi->httpMinIFrameSpeed = 8;
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                end_str[0] = '\0';
                psi->httpMinIFrameSpeed = strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                BDBG_MSG(("BCM-Min-Iframe-Speed: %d\n",psi->httpMinIFrameSpeed));
            }
        }

        p_psi = strstr(http_hdr,"BCM-Frame-Repeat: ");
        if (p_psi)
        {
            p_psi += strlen("BCM-Frame-Repeat: ");
            psi->httpFrameRepeat=1;
            end_str = strstr(p_psi, "\r\n");
            if (end_str) {
                end_str[0] = '\0';
                psi->httpFrameRepeat = strtol(p_psi, (char **)NULL, 10);
                end_str[0] = '\r';
                BDBG_MSG(("BCM-Frame-Repeat: %d\n",psi->httpFrameRepeat));
            }
        }
    }
    psi->contentLength = playback_ip->contentLength;
}

#include "bmp4_util.h"
#define B_MP4_REF_MOV_DESC   BMP4_TYPE('r','m','d','a')
#define B_MP4_REF_DATA_REF   BMP4_TYPE('r','d','r','f')
#define B_MP4_REF_DATA_RATE   BMP4_TYPE('r','m','d','r')
typedef struct QuickTime_Rate_Atom {
    size_t size;
    unsigned type;
    unsigned flags;
    unsigned rate;
} QuickTime_Rate_Atom;

void
parse_quicktime_ref_data_rate(char *buffer, unsigned *rate)
{
    QuickTime_Rate_Atom *dataRateAtom;

    dataRateAtom = (QuickTime_Rate_Atom *)buffer;
    BDBG_MSG(("%s: size %u, type 0x%x, flags %d, rate %u",
                __FUNCTION__, ntohl(dataRateAtom->size), ntohl(dataRateAtom->type), ntohl(dataRateAtom->flags), ntohl(dataRateAtom->rate)));
    *rate = ntohl(dataRateAtom->rate);
}

typedef struct QuickTime_Ref_Atom {
    size_t size;
    unsigned type;
    unsigned flags;
    unsigned refType;
    unsigned refSize;
    char url[];
} QuickTime_Ref_Atom;

void parse_quicktime_ref_data_ref(char *buffer, char **url)
{
    QuickTime_Ref_Atom *dataRefAtom;
    size_t size;

    dataRefAtom = (QuickTime_Ref_Atom *)buffer;
    size = ntohl(dataRefAtom->refSize);
    BDBG_MSG(("%s: size %u, type 0x%x, flags %d, refType 0x%x, url size %zu, url %s",
                __FUNCTION__, ntohl(dataRefAtom->size), ntohl(dataRefAtom->type), ntohl(dataRefAtom->flags), ntohl(dataRefAtom->refType), size, dataRefAtom->url));
    *url = dataRefAtom->url;
}

void
parse_quicktime_ref_mov_desc(B_PlaybackIpHandle playback_ip, char *buffer, size_t length, char **url, unsigned *bitRate)
{
    batom_t atom;
    batom_cursor cursor;
    bmp4_box box;
    size_t boxHeaderSize;
    size_t bytesProcessed = 0;
    char *rmda;

    while (bytesProcessed < length) {
        rmda = buffer + bytesProcessed;
        atom = batom_from_range(playback_ip->factory, rmda, length-bytesProcessed, NULL, NULL);
        batom_cursor_from_atom(&cursor, atom);
        boxHeaderSize = bmp4_parse_box(&cursor, &box);
        BDBG_MSG(("%s: hdr size %zu, type 0x%x, size %"PRId64 "", __FUNCTION__, boxHeaderSize, box.type, box.size));
        switch (box.type) {
            case B_MP4_REF_DATA_REF:
                parse_quicktime_ref_data_ref(rmda, url);
                bytesProcessed += box.size;
                BDBG_MSG(("url %s", *url));
                break;
            case B_MP4_REF_DATA_RATE:
                parse_quicktime_ref_data_rate(rmda, bitRate);
                BDBG_MSG(("bitrate %d", *bitRate));
                bytesProcessed += box.size;
                break;
            default:
                bytesProcessed += box.size;
        }
        batom_release(atom);
    }
}

char *
parse_quicktime_redirects(B_PlaybackIpHandle playback_ip, char *buffer, size_t length)
{
    batom_t atom;
    batom_cursor cursor;
    bmp4_box box;
    size_t boxHeaderSize;
    size_t bytesProcessed = 0;
    char *url, *newUrl = NULL;
    unsigned bitRate, maxBitRate = 0;
    char *rmda;

    /* buffer points to the start of the 1st rmda header */
    while (bytesProcessed < length) {
        rmda = buffer + bytesProcessed;
        atom = batom_from_range(playback_ip->factory, rmda, length-bytesProcessed, NULL, NULL);
        batom_cursor_from_atom(&cursor, atom);
        boxHeaderSize = bmp4_parse_box(&cursor, &box);
        BDBG_MSG(("%s: hdr size %zu, type 0x%x, size %"PRId64 "", __FUNCTION__, boxHeaderSize, box.type, box.size));
        if (box.type != B_MP4_REF_MOV_DESC) {
            BDBG_WRN(("%s: Error: Invalid Reference Movie Atom as it doesn't contain Reference Movie Descriptor Atom", __FUNCTION__));
            return NULL;
        }
        parse_quicktime_ref_mov_desc(playback_ip, rmda+boxHeaderSize, box.size-boxHeaderSize, &url, &bitRate);
        BDBG_MSG(("Done parsing one RMDA atom, bitrate %d, url %s", bitRate, url));
        if (bitRate > maxBitRate) {
            maxBitRate = bitRate;
            newUrl = url;
            BDBG_MSG(("%s: bitRate %d, url %s", __FUNCTION__, maxBitRate, newUrl));
        }
        bytesProcessed += box.size;
        BDBG_MSG(("proc %zu, len %zu, size %"PRId64 "", bytesProcessed, length, box.size));
        batom_release(atom);
    }
    BDBG_MSG(("returning url %s", newUrl));
    return newUrl;
}

/* parses AV info from the HTTP response and sets the param structure for decoder configuration */
void
http_parse_av_info(
    void *data
    )
{
    int rc = -1;
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)data;
    B_PlaybackIpPsiInfoHandle psi = &playback_ip->psi;
#ifdef B_HAS_HTTP_MP4_SUPPORT
    const bmedia_probe_track *track;
    bmedia_probe_config probe_config;
    char *stream_info = NULL;
#endif
    bfile_io_read_t bufferedFdIndex;
    B_PlaybackIpHttpSessionSetupSettings *setupSettings;
    char *http_hdr;
    NEXUS_TransportType http_content_type;
    bool foundAudio = false, foundVideo = false;
    bool sessionTypeIsDetermined;
#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    bool hlsEsProbed = false;
    bool hlsTsProbed = false;
#endif
    bool fullProbeDone = false;

    setupSettings = &playback_ip->setupSettings.u.http;
    http_hdr = playback_ip->responseMessage;

    sessionTypeIsDetermined = false;

#define STREAM_INFO_SIZE 512
    if ((stream_info = BKNI_Malloc(STREAM_INFO_SIZE)) == NULL) {
        BDBG_ERR(("%s: memory allocation failure", __FUNCTION__));
        return;
    }

#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    BDBG_MSG(("%s : %d : Calling B_PlaybackIp_HlsSessionSetup() ", __FUNCTION__, __LINE__   ));
    if (B_PlaybackIp_HlsSessionSetup(playback_ip, http_hdr) < 0) {
        BDBG_ERR(("%s: Error during HLS Session Probe & Setup", __FUNCTION__));
        goto error;
    }
    if (playback_ip->hlsSessionEnabled ) { sessionTypeIsDetermined = true; goto out; }
#endif

#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
    if (!sessionTypeIsDetermined ) {
        BDBG_MSG(("%s : %d : Calling B_PlaybackIp_MpegDashSessionSetup() ", __FUNCTION__, __LINE__   ));
        if (B_PlaybackIp_MpegDashSessionSetup(playback_ip, http_hdr) < 0) {
            BDBG_ERR(("%s: Error during MPEG-DASH Session Probe & Setup", __FUNCTION__));
            goto error;
        }
        /* MpegDashSessionSetup() does stream probing and PSI extraction, so there's nothing left to do here. */
        if (playback_ip->mpegDashSessionEnabled ) goto out;
    }
#endif

    http_content_type = playback_ip->http_content_type;

#ifdef B_HAS_HTTP_MP4_SUPPORT
redoProbe:
    bufferedFdIndex = playback_ip->bufferedFdIndex;
    /* Try to find PSI info using Media Probe Interface */
    playback_ip->probe = bmedia_probe_create();
    if (!playback_ip->probe) {
        BDBG_ERR(("%s: failed to create the probe object\n", __FUNCTION__));
        goto error;
    }
    bmedia_probe_default_cfg(&probe_config);

    probe_config.probe_es = false;
    probe_config.probe_payload = true;
    probe_config.probe_all_formats = false;
    if (http_content_type == NEXUS_TransportType_eUnknown) {
        BDBG_MSG(("%s: HTTP Response header didn't provide any well known content type, using the hint %d", __FUNCTION__, setupSettings->contentTypeHint));
        if (setupSettings->contentTypeHint != NEXUS_TransportType_eUnknown) {
            http_content_type = setupSettings->contentTypeHint;
        }
    }
    switch (http_content_type) {
    case NEXUS_TransportType_eAsf:
        probe_config.file_name = "xxx.wmv";
        break;
    case NEXUS_TransportType_eTs:
        probe_config.file_name = "xxx.mpg";
        break;
    case NEXUS_TransportType_eMp4:
        probe_config.file_name = "xxx.mp4";
        break;
    case NEXUS_TransportType_eEs:
        probe_config.probe_es = true;
        probe_config.probe_all_formats = true;
        probe_config.file_name = playback_ip->openSettings.socketOpenSettings.url;
        break;
    case NEXUS_TransportType_eFlv:
        probe_config.probe_es = false;
        probe_config.file_name = "xxx.flv";
        probe_config.probe_all_formats = true;
        break;
    default:
        probe_config.file_name = playback_ip->openSettings.socketOpenSettings.url;
        probe_config.probe_all_formats = true;
        probe_config.probe_es = true;
        break;
    }

    if (http_content_type != NEXUS_TransportType_eUnknown &&
        http_content_type != NEXUS_TransportType_eFlv)
        probe_config.type = B_PlaybackIp_UtilsNexus2Mpegtype(http_content_type);
    else
        probe_config.type = bstream_mpeg_type_unknown;
    if (setupSettings->enablePayloadScanning) {
        probe_config.probe_duration = true;
    }
    else {
        /* payload scanning is not set, do the minimum probing work */
        probe_config.probe_es = false;
        probe_config.probe_all_formats = false;
        probe_config.probe_payload = false;
        probe_config.probe_duration = false;
        probe_config.probe_index = false;
        probe_config.parse_index = false;
    }
#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    if (playback_ip->hlsSessionEnabled) {
        if (playback_ip->psi.psiValid) {
            BDBG_WRN(("%s: PSI info already known via HTTP response for a HLS session!!", __FUNCTION__));
            goto out;
        }
        if (!hlsTsProbed) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: HLS Session: Probing for TS format first", __FUNCTION__));
#endif
            bmedia_probe_default_cfg(&probe_config);
            probe_config.type = B_PlaybackIp_UtilsNexus2Mpegtype(NEXUS_TransportType_eTs);
            probe_config.file_name = "xxx.ts";
            probe_config.probe_payload = false;
            probe_config.probe_index = false;
            probe_config.parse_index = false;
            probe_config.probe_es = false;
            probe_config.probe_all_formats = false;
            probe_config.probe_duration = false;
            if (!playback_ip->quickMediaProbe)
                playback_ip->quickMediaProbe = true;
            else {
                hlsTsProbed = true;
                playback_ip->quickMediaProbe = false;
            }
        }
        else {
            /* try for ES formats as HLS only supports either TS or ES */
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: HLS Session: Probing for ES format", __FUNCTION__));
#endif
            probe_config.type = B_PlaybackIp_UtilsNexus2Mpegtype(NEXUS_TransportType_eEs);
            probe_config.probe_payload = true;
            probe_config.probe_es = true;
            probe_config.probe_all_formats = false;
            probe_config.file_name = "xxx.aac";
            hlsEsProbed = true;
        }
    }
#endif

    if (playback_ip->fileBuffered) {
        probe_config.buffer = bfile_buffered_get_buffer(playback_ip->fileBuffered);
    }

probeAgain:
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog) {
        BDBG_WRN(("%s: Begin Probing Media for (content type %d) obtaining the PSI info (it may take some time ...)", __FUNCTION__, http_content_type));
        BDBG_WRN(("%s: probe_config: duration=%s index=%s es=%s payload=%s probeAllFormats=%s parseIndex=%s typeEnum=%d minProbeOffset=%"PRId64 " probeOffset=%"PRId64 "", __FUNCTION__,
                probe_config.probe_duration?"Y":"N",
                probe_config.probe_index?"Y":"N",
                probe_config.probe_es?"Y":"N",
                probe_config.probe_payload?"Y":"N",
                probe_config.probe_all_formats?"Y":"N",
                probe_config.parse_index?"Y":"N",
                probe_config.type,
                probe_config.min_probe_request,
                probe_config.probe_offset));
    }
#endif
    playback_ip->stream = bmedia_probe_parse(playback_ip->probe, bufferedFdIndex, &probe_config);

    if (playback_ip->stream == NULL && !fullProbeDone) {
        /* minimum probing didn't provide results, lets do full probe. */
        probe_config.probe_es = true;
        probe_config.probe_all_formats = true;
        probe_config.probe_payload = true;
        probe_config.probe_duration = true;
        probe_config.probe_index = true;
        probe_config.parse_index = true;
        probe_config.file_name = "";
        probe_config.type = bstream_mpeg_type_unknown;
        fullProbeDone = true;
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("%s: playback_ip=%p: Minimum probe didn't return media info, do a detailed probe!", __FUNCTION__, (void *)playback_ip));
#endif
        goto probeAgain;
    }

    if (playback_ip->stream) {
        /* probe succeeded in finding a stream */
        bmedia_stream_to_string(playback_ip->stream, stream_info, STREAM_INFO_SIZE);
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("media stream info: %s", stream_info));
#endif
        psi->mpegType = B_PlaybackIp_UtilsMpegtype2ToNexus(playback_ip->stream->type);

#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
        if (playback_ip->mpegDashSessionEnabled) {
            if (playback_ip->mpegDashSessionState && playback_ip->mpegDashSessionState->currentRepresentation && playback_ip->mpegDashSessionState->currentRepresentation->bounded) {
                psi->duration = playback_ip->mpegDashSessionState->currentRepresentation->totalDuration;
            }
            else {
                /* unbounded case, i.e. live stream, so set duration to large value */
                /* media browser needs the duration to know when to stop the playback */
                psi->duration = 0;
            }
            psi->mpegDashSessionEnabled = true;
        }
#endif
#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
        if (playback_ip->hlsSessionEnabled) {
            if (playback_ip->hlsSessionState && playback_ip->hlsSessionState->currentPlaylistFile && playback_ip->hlsSessionState->currentPlaylistFile->bounded) {
                psi->duration = playback_ip->hlsSessionState->currentPlaylistFile->totalDuration;
            }
            else {
                /* unbounded case, i.e. live stream, so set duration to large value */
                /* media browser needs the duration to know when to stop the playback */
                psi->duration = 0;
            }
            psi->hlsSessionEnabled = true;
        }
        else
#endif

        if (probe_config.probe_duration && playback_ip->stream->duration) {
            BDBG_MSG((" %s: duration from HTTP ResponseHeader=%u, Duration from mediaProbe=%u", __FUNCTION__, psi->duration, playback_ip->stream->duration));
            /* Only override duration if we had asked media probe to determine it & it had found one. Otherwise, keep what we find from the HTTP Response header. */
            psi->duration = playback_ip->stream->duration;
        }
        psi->avgBitRate = playback_ip->stream->max_bitrate;
        BDBG_MSG(("%s: avg bitrate of stream %d, user supplied %d", __FUNCTION__, psi->avgBitRate, setupSettings->avgBitRate));
        if (psi->avgBitRate == 0 && setupSettings->avgBitRate) {
            psi->avgBitRate = setupSettings->avgBitRate;
        }
        if (psi->avgBitRate)
            psi->maxBufferDuration = (playback_ip->dataCache[0].size*8 / psi->avgBitRate)*1000;  /* in msec */
        else {
            BDBG_MSG(("%s: Warning: Media Probe couldn't determine the avg bitrate of stream!!!", __FUNCTION__));
            psi->avgBitRate = 0;
        }
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("Media Details: container type %d, index %d, avg bitrate %d, duration %d, cache buffer in msec %lu, # of programs %d, # of tracks (streams) %d",
                    psi->mpegType, playback_ip->stream->index, psi->avgBitRate, psi->duration, psi->maxBufferDuration, playback_ip->stream->nprograms, playback_ip->stream->ntracks));
#endif
        if (
#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
                !playback_ip->hlsSessionEnabled &&
#endif
                ((setupSettings->dontUseIndex && playback_ip->stream->type != bstream_mpeg_type_mp4)
                /* App doesn't want to use the index info for all but MP4 files as some streams falsely indicate that it is present, but its really not */
                || playback_ip->stream->index == bmedia_probe_index_unknown /* true for VOBs & TS formats */
                || playback_ip->stream->index == bmedia_probe_index_missing    /* true for some WMA/WMV files w/o index */
                )
            ) {
            struct bfile_in_net *filePlay;
            BDBG_MSG(("%s: Not setting index descriptor because %s for this content format %d, (probe index search code %d)",
                        __FUNCTION__, (setupSettings->dontUseIndex?"app set dontUseIndex flag":"no index info is available"), psi->mpegType, playback_ip->stream->index));
            if (playback_ip->fileBuffered) {
                bfile_buffered_detach(playback_ip->fileBuffered);
                bfile_buffered_destroy(playback_ip->fileBuffered);
                playback_ip->fileBuffered = NULL;
            }
            if (playback_ip->factory) {
                batom_factory_destroy(playback_ip->factory);
                playback_ip->factory = NULL;
            }
            filePlay = (struct bfile_in_net *)playback_ip->nexusFileHandle;
            memset(&filePlay->index, 0, sizeof(filePlay->index));
            filePlay->self.file.index = NULL;
        }

#define QUICK_TIME_REDIRECT_SUPPORT
#ifdef QUICK_TIME_REDIRECT_SUPPORT
        /* check for quick time redirect streams */
        if (playback_ip->stream->type == bstream_mpeg_type_mp4 && ((((bmp4_probe_stream*)playback_ip->stream)->rmra.size) != 0)) {
            char *url;
            BDBG_MSG(("%s: QuickTime redirect stream: rmra size %"PRId64 ", offset %"PRId64 "", __FUNCTION__, ((bmp4_probe_stream*)playback_ip->stream)->rmra.size, ((bmp4_probe_stream*)playback_ip->stream)->rmra.offset));
            url = parse_quicktime_redirects(playback_ip, playback_ip->indexCache+((bmp4_probe_stream*)playback_ip->stream)->rmra.offset, ((bmp4_probe_stream*)playback_ip->stream)->rmra.size);
            if (url == NULL) {
                BDBG_ERR(("%s: ERROR: QuickTime redirect stream: however failed to retrieve a valid URL", __FUNCTION__));
            }
            else {
                /* separate server name from the URL */
                if (http_parse_url(playback_ip->openSettings.socketOpenSettings.ipAddr, &playback_ip->openSettings.socketOpenSettings.port, &playback_ip->openSettings.socketOpenSettings.url, url) != 0) {
                    BDBG_ERR(("%s: ERROR: Failed to parse server name, port, and uri fro the redirected URL %s", __FUNCTION__, url));
                    goto error;
                }
                BDBG_MSG(("%s: QuickTime redirect stream: server %s, port %d, uri %s",
                            __FUNCTION__, playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port, playback_ip->openSettings.socketOpenSettings.url));

                /* reset index cache */
                playback_ip->indexCacheDepth = 0;
                playback_ip->indexCacheEndOffset = 0;
                playback_ip->indexCacheWriteIndex = playback_ip->indexCacheDepth;
                playback_ip->initial_data_len = 0;
                playback_ip->indexCacheStartOffset = 0;
                playback_ip->indexCacheReadIndex = 0;

                /* destroy and recreate the file handles */
                B_PlaybackIp_HttpDestroyFilePlayHandle(playback_ip);
                playback_ip->nexusFileHandle = B_PlaybackIp_HttpCreateFilePlayHandle(playback_ip, &playback_ip->bufferedFdIndex);
                if (playback_ip->nexusFileHandle == NULL) {
                    BDBG_ERR(("%s: Failed to Create the Playback Handles\n", __FUNCTION__));
                    goto error;
                }
                BDBG_MSG(("%s: re-created Nexus file IO handle %p", __FUNCTION__, (void *)playback_ip->nexusFileHandle));

                /* reconnect to server and send new HTTP request using new URL */
                rc = B_PlaybackIp_HttpNetRangeReq(playback_ip, playback_ip->indexCache, HTTP_INDEX_CACHE_CHUNK_SIZE, 0, 0, playback_ip->socketState.fd, &playback_ip->socketState.fd);
                if (rc < 0) {
                    BDBG_ERR(("%s: ERROR: Socket setup or HTTP request/response failed for QuickTime redirected URL", __FUNCTION__));
                    goto error;
                }
                if (rc > 0) {
                    /* setup the index cache w/ the initial data */
                    playback_ip->indexCacheInit = 1;
                    playback_ip->indexCacheDepth = rc;
                    playback_ip->indexCacheWriteIndex = playback_ip->indexCacheDepth;
                    playback_ip->indexCacheStartOffset = 0;
                    playback_ip->indexCacheEndOffset = rc - 1;
                    playback_ip->indexCacheReadIndex = 0;
                }
                BDBG_MSG(("%s: Connected to the server using the new URL, now redo the probe", __FUNCTION__));

                /* redo the probe */
                bmedia_probe_stream_free(playback_ip->probe, playback_ip->stream);
                bmedia_probe_destroy(playback_ip->probe);
                playback_ip->probe = NULL;
                playback_ip->contentLength = 0;
                goto redoProbe;
            }
        }
        else
#endif
            if (playback_ip->stream->type == bstream_mpeg_type_ts && ((((bmpeg2ts_probe_stream*)playback_ip->stream)->pkt_len) == 192)) {
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog)
                    BDBG_WRN(("%s: TTS Stream\n", __FUNCTION__));
#endif
            psi->transportTimeStampEnabled = true;
        }
        else
            psi->transportTimeStampEnabled = false;
        rc = 0;
        psi->psiValid = true;
        for(track=BLST_SQ_FIRST(&playback_ip->stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
            if (track->type==bmedia_track_type_video && (track->info.video.codec == bvideo_codec_h264_svc || track->info.video.codec == bvideo_codec_h264_mvc)) {
                psi->extraVideoPid = track->number;
                psi->extraVideoCodec = B_PlaybackIp_UtilsVideoCodec2Nexus(track->info.video.codec);
                BDBG_MSG(("extra video track %u codec:%u\n", track->number, psi->extraVideoCodec));
                continue;
            }
            else if (track->type==bmedia_track_type_video && track->info.video.codec != bvideo_codec_unknown && !foundVideo) {
                BDBG_MSG(("video track %u codec:%u\n", track->number, track->info.video.codec));
                psi->videoPid = track->number;
                psi->videoCodec = B_PlaybackIp_UtilsVideoCodec2Nexus(track->info.video.codec);
                psi->videoHeight = track->info.video.height;
                psi->videoWidth = track->info.video.width;
                psi->colorDepth = bmedia_probe_get_video_color_depth(track);
                foundVideo = true;
            }
            else if (track->type==bmedia_track_type_pcr) {
                BDBG_MSG(("pcr pid %u\n", track->number));
                psi->pcrPid = track->number;
            }
            else if (track->type==bmedia_track_type_other &&
                    playback_ip->stream->type == bstream_mpeg_type_ts &&
                    ((bmpeg2ts_psi_probe_track *)track)->transport_type==bmpeg2ts_psi_transport_pmt &&
                    (playback_ip->stream->probe_id == BMPEG2TS_PSI_PROBE_ID || playback_ip->stream->probe_id == BMPEG2TS192_PSI_PROBE_ID)
                    ) {
                BDBG_MSG(("pmt pid 0x%x type=%d", track->number, track->type));
                psi->pmtPid = track->number;
            }
            else if (track->type==bmedia_track_type_audio && track->info.audio.codec != baudio_format_unknown && !foundAudio) {
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
                        BDBG_WRN(("%s(%d) audio track %u, codec:%u, extraAudioPidsCount : %d\n", __func__, __LINE__, track->number, track->info.audio.codec, psi->extraAudioPidsCount));
                    }
                }
            }
        }
#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
        if (playback_ip->hlsSessionEnabled && psi->mpegType == NEXUS_TransportType_eEs) {
            /* In HLS protocol, ES stream is for audio only case and we convert it to PES before feeding to playpump */
            /* so we set the transport to PES and also set the audio pid to the PES id that we are going to define for each PES pkt */
            psi->mpegType = NEXUS_TransportType_eMpeg2Pes;
            psi->audioPid = HLS_PES_AUDIO_ES_ID;
        }
        if (playback_ip->hlsSessionEnabled && !hlsEsProbed && !psi->videoPid && !psi->audioPid) {
            BDBG_ERR(("%s: Video (%d) or Audio (%d) PIDs are not found during Quick Media Probe, redo a full probe", __FUNCTION__, psi->videoPid, psi->audioPid));
            /* we failed to probe 1st time for HLS which was done for TS format assuming that PAT/PMT will be present at the start of the segment (as required by HLS Spec).
             * however, to be forgiving for non-compliant servers, we will retry this probe. This will also be done for once more for Audio (ES) only streams. */
            psi->psiValid = false;
            bmedia_probe_stream_free(playback_ip->probe, playback_ip->stream);
            bmedia_probe_destroy(playback_ip->probe);
            goto redoProbe;
        }
#endif
        if (!psi->videoPid && !psi->audioPid) {
            BDBG_ERR(("%s: Video (%d) or Audio (%d) PIDs are not found during Media Probe\n", __FUNCTION__, psi->videoPid, psi->audioPid));
            psi->psiValid = false;
            goto error;
        }

        rc = 0;
        goto out;
    }
    else {
        /* probe didn't find the PSI info either, return error */
        BDBG_ERR(("%s: media probe didn't find the PSI info, return error", __FUNCTION__));
        goto error;
    }
#endif /* B_HAS_HTTP_MP4_SUPPORT */

error:
    BDBG_ERR(("%s: ERROR: Couldn't obtain PSI info from HTTP response or Media Probe\n", __FUNCTION__));
    psi->numPlaySpeedEntries = 1;
    psi->mpegType = NEXUS_TransportType_eUnknown;
    psi->psiValid = false;

out:
    if (playback_ip->playback_halt_event)
        BKNI_SetEvent(playback_ip->playback_halt_event);
    playback_ip->apiCompleted = true;
    playback_ip->apiInProgress = false;
    psi->contentLength = playback_ip->contentLength;
    if (playback_ip->serverClosed == true)
    {
        playback_ip->reOpenSocket = true;
        playback_ip->serverClosed = false;
        BDBG_WRN(("%s: ServerClosed Set, clear it and set reOpenSocket flag!", __FUNCTION__));
    }
    BDBG_MSG(("state %d", playback_ip->playback_state));
    if (playback_ip->openSettings.eventCallback && playback_ip->playback_state != B_PlaybackIpState_eStopping && playback_ip->playback_state != B_PlaybackIpState_eStopped)
    {
        playback_ip->openSettings.eventCallback(playback_ip->openSettings.appCtx, B_PlaybackIpEvent_eSessionSetupDone);
    }
    if (stream_info)
        BKNI_Free(stream_info);
    return;
}

static void httpSessionOpenThread(
    void *data
    )
{
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)data;
    int fd = 0;
    int rc = -1;
    char *http_hdr = NULL;
    char *http_payload = NULL;
    http_url_type_t http_url_type;
    char *server = NULL;
    int port;
    int loopCnt = 0;
    double seek_range = 0.;
    int speed = 1;
    off_t byteRangeStart, byteRangeEnd;
    B_PlaybackIpError errorCode = B_ERROR_PROTO;
    struct addrinfo hints;
    struct addrinfo *addrInfo = NULL;
    char portString[16];
    socklen_t addrLen;
    B_PlaybackIpSocketState *socketState;

    socketState = &playback_ip->openStatus.socketState;

    playback_ip->responseMessage = (char *) BKNI_Malloc(TMP_BUF_SIZE+1);
    server = (char *)BKNI_Malloc(TMP_STRING_SIZE+1);
    if (!playback_ip->responseMessage || !server) {
        BDBG_ERR(("%s: ERROR: failed to allocate memory\n", __FUNCTION__));
        errorCode = B_ERROR_OUT_OF_MEMORY;
        goto error;
    }
    server[TMP_STRING_SIZE] = '\0';
    playback_ip->responseMessage[TMP_BUF_SIZE] = '\0';

    strncpy(server, playback_ip->openSettings.socketOpenSettings.ipAddr, TMP_STRING_SIZE);
    port = playback_ip->openSettings.socketOpenSettings.port;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    memset(portString, 0, sizeof(portString));  /* getaddrinfo() requires port # in the string form */
    snprintf(portString, sizeof(portString)-1, "%d", port);
    if (getaddrinfo(server, portString, &hints, &addrInfo) != 0) {
        BDBG_ERR(("%s: ERROR: getaddrinfo failed for server:port: %s:%d, errno %d", __FUNCTION__, server, port, errno));
        perror("getaddrinfo");
        errorCode = B_ERROR_SOCKET_ERROR;
        goto error;
    }
    socketState->remote_addr.sin_family = addrInfo->ai_family;
    socketState->remote_addr.sin_port = htons(playback_ip->openSettings.socketOpenSettings.port);
    socketState->remote_addr.sin_addr.s_addr = ((struct sockaddr_in *)(addrInfo->ai_addr))->sin_addr.s_addr;
    freeaddrinfo(addrInfo);

    /* re-initialize the playback_ip state as this structure is not freed during each Socket Open/Close sequence */
    reset_playback_ip_state(playback_ip);
    for (;;)
    {
        rc = -1;
        {
            int retryCount = 0;
            int maxRetryCount;
#define TCP_CONNECT_TIMEOUT_INTERVAL 1
            maxRetryCount = playback_ip->networkTimeout / TCP_CONNECT_TIMEOUT_INTERVAL;
            if (maxRetryCount == 0) maxRetryCount = 1;
            while (retryCount++ < maxRetryCount) {
                if  (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
                    /* user changed the channel, so return */
                    BDBG_WRN(("%s: breaking out of main pre-charging loop due to state (%d) change", __FUNCTION__, playback_ip->playback_state));
                    break;
                }
                rc = B_PlaybackIp_UtilsTcpSocketConnect(&playback_ip->playback_state, server, port, true, TCP_CONNECT_TIMEOUT_INTERVAL, &socketState->fd);
                if (rc == B_ERROR_SUCCESS) {
                    break;
                }
                else {
                    BDBG_ERR(("%s:%p: state %d ERROR: failed to send Socket Connect Request to Server: %s:%d, Retry it!!!!: retryCount %d, Max retryCount %d", __FUNCTION__, (void *)playback_ip, playback_ip->playback_state, server, port, retryCount, maxRetryCount ));
                    continue;
                }
            }
            if (rc != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: ERROR: failed to send Socket Connect Request to Server: %s:%d\n", __FUNCTION__, server, port));
                goto error;
            }
        }
        playback_ip->socketState.fd = fd = socketState->fd;
        /* we cache the fd in the data cache as well. This is used when we are caching data */
        /* Do security session setup here as Get request to server may be encrypted */
        rc = B_PlaybackIp_SecuritySessionOpen(playback_ip, &playback_ip->openSettings, fd, &playback_ip->securityHandle);
        if (rc < 0) {
            BDBG_ERR(("%s: ERROR: failed to setup the session, rc = %d\n", __FUNCTION__, rc));
            errorCode = B_ERROR_PROTO;
            goto error;
        }

        /* prepare initial Get request */
        byteRangeStart = playback_ip->byteRangeOffset; /* default is 0; set in security session open */
        if (playback_ip->setupSettings.u.http.disableRangeHeader)
            byteRangeStart = 0; /* this ensures that we dont include Range Header in the outgoing requests */
        byteRangeEnd = 0;
        if (http_build_get_req(&playback_ip->requestMessage, playback_ip, speed, seek_range, 0., byteRangeStart, byteRangeEnd, NULL) < 0) {
            BDBG_ERR(("%s: ERROR: failed to build HTTP Get request to Server", __FUNCTION__));
            errorCode = B_ERROR_PROTO;
            goto error;
        }

        /* now send GET request */
        if ( (rc = playback_ip->netIo.write(playback_ip->securityHandle, &playback_ip->playback_state, socketState->fd, playback_ip->requestMessage, strlen(playback_ip->requestMessage))) < 0 ) {
            BDBG_ERR(("%s: ERROR: failed to send HTTP Get request to Server: %s:%d\n", __FUNCTION__, server, port));
            errorCode = B_ERROR_PROTO;
            goto error;
        }
        BDBG_MSG(("%s: Sent HTTP Get Request (fd %d)", __FUNCTION__, playback_ip->socketState.fd));
        BKNI_Free(playback_ip->requestMessage);
        playback_ip->requestMessage = NULL;

        /* now read the http response */
        if ( (rc = http_read_response(playback_ip, playback_ip->securityHandle, fd, &playback_ip->responseMessage, TMP_BUF_SIZE, &http_hdr, &http_payload, &playback_ip->initial_data_len)) < 0 ) {
            BDBG_ERR(("%s: ERROR: failed to receive valid HTTP response\n", __FUNCTION__));
            errorCode = B_ERROR_PROTO;
            goto error;
        }

        http_url_type = http_get_url_type(http_hdr, playback_ip->openSettings.socketOpenSettings.url);
        BDBG_MSG(("http url type %d", http_url_type));
        if (http_url_type == HTTP_URL_IS_ASX) {
            /* the response for a URL (which was a link to a content) would contain ASX Elements */
            /* Parse ASX header, it will either contains link to another ASX file or to the actual URL */
            if (http_parse_asx_payload(server, &playback_ip->openSettings.socketOpenSettings.url, http_payload) != 0) {
                BDBG_ERR(("Incorrect ASX Payload or parsing error\n'"));
                errorCode = B_ERROR_PROTO;
                goto error;
            }
            /* previous function gets the new URL & server information and we send another GET request to this server */
            close(fd);
            playback_ip->netIo.close(playback_ip->securityHandle);
            playback_ip->contentLength = 0;
            strncpy(playback_ip->openSettings.socketOpenSettings.ipAddr, server, sizeof(playback_ip->openSettings.socketOpenSettings.ipAddr) -1);
            playback_ip->openSettings.socketOpenSettings.ipAddr[sizeof(playback_ip->openSettings.socketOpenSettings.ipAddr)-1] = '\0';
            playback_ip->serverClosed = false;
        }
        else if (http_url_type == HTTP_URL_IS_PLS) {
            /* the response for a URL which contains a PLS format playlist, used by some radio stations */
            /* Parse this playlist to get the actual URL */
            unsigned bytesRead;
            bytesRead = (http_payload - playback_ip->responseMessage) + playback_ip->initial_data_len;
            rc = playback_ip_read_socket(playback_ip, playback_ip->securityHandle, fd, http_payload+playback_ip->initial_data_len, TMP_BUF_SIZE-bytesRead, playback_ip->networkTimeout);
            if (rc <= 0 && playback_ip->initial_data_len == 0) {
                BDBG_ERR(("%s: Shoutcast Playlist payload data is missing, rc %d, serverClosed %d, selectTimeout %d", __FUNCTION__, rc, playback_ip->serverClosed, playback_ip->selectTimeout));
                errorCode = B_ERROR_PROTO;
                goto error;
            }
            http_payload[rc>0?rc:0+playback_ip->initial_data_len] = '\0'; /* null terminate the read playlist */
            if (http_parse_pls_payload(server, &playback_ip->openSettings.socketOpenSettings.port, &playback_ip->openSettings.socketOpenSettings.url, http_payload) != 0) {
                BDBG_ERR(("Incorrect PLS Payload or parsing error\n'"));
                errorCode = B_ERROR_PROTO;
                goto error;
            }
            /* previous function gets the new URL & server information and we send another GET request to this server */
            close(fd);
            playback_ip->netIo.close(playback_ip->securityHandle);
            playback_ip->contentLength = 0;
            strncpy(playback_ip->openSettings.socketOpenSettings.ipAddr, server, sizeof(playback_ip->openSettings.socketOpenSettings.ipAddr) -1);
            playback_ip->openSettings.socketOpenSettings.ipAddr[sizeof(playback_ip->openSettings.socketOpenSettings.ipAddr)-1] = '\0';
            playback_ip->serverClosed = false;
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: Shoutcast/PLS playlist case, caching the redirected URL: http://%s:%d%s", __FUNCTION__, playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port, playback_ip->openSettings.socketOpenSettings.url));
#endif
        }
        else if (http_url_type == HTTP_URL_IS_REDIRECT) {
            if (!playback_ip->openSettings.socketOpenSettings.useProxy) {
                /* parse HTTP redirect and extract the new URI & server:port info */
                /* free up the previous cookie */
                if (playback_ip->cookieFoundViaHttpRedirect)
                    BKNI_Free(playback_ip->cookieFoundViaHttpRedirect);
                playback_ip->cookieFoundViaHttpRedirect = NULL;
                if (http_parse_redirect(server, &playback_ip->openSettings.socketOpenSettings.port, &playback_ip->openSettings.socketOpenSettings.protocol, &playback_ip->openSettings.socketOpenSettings.url, &playback_ip->cookieFoundViaHttpRedirect, http_hdr) != 0) {
                    BDBG_ERR(("Incorrect HTTP Redirect response or parsing error'"));
                    errorCode = B_ERROR_PROTO;
                    goto error;
                }
                /* now copy over the server IP address from the redicted URL for future byte range requests */
                strncpy(playback_ip->openSettings.socketOpenSettings.ipAddr, server, sizeof(playback_ip->openSettings.socketOpenSettings.ipAddr) -1);
                playback_ip->openSettings.socketOpenSettings.ipAddr[sizeof(playback_ip->openSettings.socketOpenSettings.ipAddr)-1] = '\0';
                port = playback_ip->openSettings.socketOpenSettings.port;
            }
            else {
                /* app is asking us to use proxy server, so all subsequent HTTP Get requests need to go thru this proxy server */
                /* We get the absolute URL from the Location Header in the HTTP Redirect response and use that as the URL in the Get Header */
                if (http_redirect_get_full_location_header(&playback_ip->openSettings.socketOpenSettings.url, http_hdr) != 0) {
                    BDBG_ERR(("Incorrect HTTP Redirect response or parsing error'"));
                    errorCode = B_ERROR_PROTO;
                    goto error;
                }
            }
            /* previous function gets the new URL & server information and we send another GET request to this server */
            close(fd);
            playback_ip->netIo.close(playback_ip->securityHandle);
            playback_ip->contentLength = 0;
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: HTTP redirect case, caching the redirected URL: http://%s:%d%s", __FUNCTION__, playback_ip->openSettings.socketOpenSettings.ipAddr, playback_ip->openSettings.socketOpenSettings.port, playback_ip->openSettings.socketOpenSettings.url));
#endif
        }
        else {
            /* actual content URL, get the content attributes from HTTP response header */
            BDBG_MSG(("GOT ACTUAL CONTENT\n"));

            /* copy any payload bytes (which is actual AV content) into a temp buffer */
            /* playback start copies this to playpump as well */
            if (playback_ip->initial_data_len != 0) {
                BDBG_MSG(("copying initial data (part of HTTP response) of len %d to temp buffer\n", playback_ip->initial_data_len));
                if (playback_ip->initial_data_len > TMP_BUF_SIZE) {
                    BDBG_ERR(("%s: ERROR: initial data read %d is greater than the temp buffer size %d, need to increase its size", __FUNCTION__, playback_ip->initial_data_len, TMP_BUF_SIZE));
                    errorCode = B_ERROR_PROTO;
                    goto error;
                }
                memcpy(playback_ip->temp_buf, http_payload, playback_ip->initial_data_len);
            }

            *http_payload = '\0';   /* for string parsing of HTTP header, payload is already copied into temp_buf */
            if (playback_ip->openSettings.u.http.copyResponseHeaders) {
                /* app needs a copy of the response headers for further app specific header parsing */
                playback_ip->openStatus.u.http.responseHeaders = strdup(http_hdr);
                if (playback_ip->openStatus.u.http.responseHeaders == NULL) {
                    BDBG_ERR(("%s: ERROR: failed to duplicate HTTP Response header string\n", __FUNCTION__));
                    errorCode = B_ERROR_OUT_OF_MEMORY;
                    goto error;
                }
            }
            /* TODO: copy local address in the socketState */

            /* we have the content now, so break out this loop */
            playback_ip->socketState.fd = fd = socketState->fd;
            break;
        }

        if (loopCnt++ >5) {
            BDBG_ERR(("ERROR: Can't resolve a URL Link in 5 attempts\n"));
            errorCode = B_ERROR_PROTO;
            goto error;
        }
    }

    /* success */
    addrLen = sizeof(socketState->local_addr);
    if (getsockname(socketState->fd, (struct sockaddr *)&socketState->local_addr, (socklen_t *)&addrLen) != 0) {
        BDBG_ERR(("ERROR: Failed to obtain connection socket address, errno: %d \n", errno));
        perror("getsockname");
        goto error;
    }

    BDBG_MSG(("%s: successfully received the HTTP Get Response (fd %d, local ip:port = %s:%d)",
                __FUNCTION__, socketState->fd,
                inet_ntoa(socketState->local_addr.sin_addr),
                ntohs(socketState->local_addr.sin_port)));
    errorCode = B_ERROR_SUCCESS;

error:
    if (playback_ip->statusCode)
    {
        playback_ip->openStatus.u.http.statusCode = playback_ip->statusCode;
    }
    if (server) BKNI_Free(server);
    if (errorCode != B_ERROR_SUCCESS ) {
        if (playback_ip->cookieFoundViaHttpRedirect) {
            BKNI_Free(playback_ip->cookieFoundViaHttpRedirect);
            playback_ip->cookieFoundViaHttpRedirect = NULL;
        }
        if (fd)
            close(fd);
        if (playback_ip->responseMessage) {
            BKNI_Free(playback_ip->responseMessage);
            playback_ip->responseMessage = NULL;
        }
        if (playback_ip->requestMessage) {
            BKNI_Free(playback_ip->requestMessage);
            playback_ip->requestMessage = NULL;
        }
        if (playback_ip->openSettings.u.http.copyResponseHeaders && playback_ip->openStatus.u.http.responseHeaders) {
            free(playback_ip->openStatus.u.http.responseHeaders);
            playback_ip->openStatus.u.http.responseHeaders = NULL;
        }
        if (playback_ip->netIo.close && playback_ip->securityHandle) {
            playback_ip->netIo.close(playback_ip->securityHandle);
            playback_ip->securityHandle = NULL;
        }
        playback_ip->sessionOpenStatus = errorCode;
    }
    else {
        playback_ip->sessionOpenStatus = B_ERROR_SUCCESS;
    }
    playback_ip->apiCompleted = true;
    playback_ip->apiInProgress = false;
    BDBG_MSG(("state %d, protocol %d", playback_ip->playback_state, playback_ip->protocol));
    if (playback_ip->playback_halt_event) BKNI_SetEvent(playback_ip->playback_halt_event);
    if (playback_ip->openSettings.eventCallback && playback_ip->playback_state != B_PlaybackIpState_eStopping && playback_ip->playback_state != B_PlaybackIpState_eStopped)
    {
        playback_ip->openSettings.eventCallback(playback_ip->openSettings.appCtx, B_PlaybackIpEvent_eSessionOpenDone);
    }
}

/*
   Function does following:
   -sdf
   -sdf
*/
B_PlaybackIpError
B_PlaybackIp_HttpSessionOpen(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionOpenSettings *openSettings,
    B_PlaybackIpSessionOpenStatus *openStatus /* [out] */
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;

    if (!playback_ip || !openSettings || !openStatus) {
        BDBG_ERR(("%s: invalid params, playback_ip %p, openSettings %p, openStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)openSettings, (void *)openStatus));
        errorCode = B_ERROR_INVALID_PARAMETER;
        return errorCode;
    }

    /* if SessionOpen is in progress, return INCOMPLETE */
    if (playback_ip->apiInProgress)
        return B_ERROR_IN_PROGRESS;

    /* if SessionOpen is completed, return results to app */
    if (playback_ip->apiCompleted) {
        BDBG_MSG(("%s: previously started session open operation completed, playback_ip %p", __FUNCTION__, (void *)playback_ip));
        goto apiDone;
    }

    /* Neither SessionOpen is in progress nor it is completed, so start setup */
    playback_ip->sessionOpenStatus = B_ERROR_IN_PROGRESS;

    playback_ip->openSettings.u.http.additionalHeaders = NULL;
    playback_ip->openSettings.u.http.userAgent = NULL;
    if (openSettings->u.http.additionalHeaders) {
        if (!strstr(openSettings->u.http.additionalHeaders, "\r\n")) {
            BDBG_ERR(("%s: additional HTTP header is NOT properly terminated (missing \\r\\n), header is %s\n", __FUNCTION__, openSettings->u.http.additionalHeaders));
            errorCode = B_ERROR_INVALID_PARAMETER;
            goto error;
        }
        /* cache a copy of this field */
        if ((playback_ip->openSettings.u.http.additionalHeaders = B_PlaybackIp_UtilsStrdup(openSettings->u.http.additionalHeaders)) == NULL) {
            BDBG_ERR(("%s: Failed to duplicate additional headers string due to out of memory condition", __FUNCTION__));
            errorCode = B_ERROR_OUT_OF_MEMORY;
            goto error;
        }
    }
    if (openSettings->u.http.userAgent) {
        /* note: app is not required to terminate userAgent w/ \r\n as as accept it as a string only and add these terminators ourselves */
        /* cache a copy of this field */
        if ((playback_ip->openSettings.u.http.userAgent = B_PlaybackIp_UtilsStrdup(openSettings->u.http.userAgent)) == NULL) {
            BDBG_ERR(("%s: Failed to duplicate userAgent field string due to out of memory condition", __FUNCTION__));
            errorCode = B_ERROR_OUT_OF_MEMORY;
            goto error;
        }
    }
    /* cache a copy of this field as we may have to re-send request to server at a later point */
    BDBG_MSG(("%s: ip %s, port %d, proto %d, n/w buf sz %d, copyResp %d, url %s, addHdr %s, userAgent %s", __FUNCTION__,
                openSettings->socketOpenSettings.ipAddr,
                openSettings->socketOpenSettings.port,
                openSettings->socketOpenSettings.protocol,
                openSettings->u.http.networkBufferSize,
                openSettings->u.http.copyResponseHeaders,
                openSettings->socketOpenSettings.url,
                openSettings->u.http.additionalHeaders,
                openSettings->u.http.userAgent
                ));

    if (playback_ip->openSettings.nonBlockingMode) {
        /* do PSI parsing in a thread and return back to app */
        playback_ip->sessionOpenThread = B_Thread_Create("SessionOpenThread", (B_ThreadFunc)httpSessionOpenThread, (void *)playback_ip, NULL);
        if (NULL == playback_ip->sessionOpenThread) {
            BDBG_ERR(("%s: Failed to create thread for Session Open", __FUNCTION__));
            errorCode = B_ERROR_UNKNOWN;
            goto error;
        }
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("%s: Non blocking session open operation started: playback_ip %p\n", __FUNCTION__, (void *)playback_ip));
#endif
        errorCode = B_ERROR_IN_PROGRESS;
    }
    else {
        httpSessionOpenThread(playback_ip);
        /* success case, fall below to the done label */
    }

apiDone:
    errorCode = playback_ip->sessionOpenStatus;
    if (errorCode == B_ERROR_IN_PROGRESS)
    {
        playback_ip->apiInProgress = true;
        playback_ip->apiCompleted = false;
    }
    else if (errorCode == B_ERROR_SUCCESS)
    {
        playback_ip->playback_state = B_PlaybackIpState_eOpened;
        *openStatus = playback_ip->openStatus;
        playback_ip->apiCompleted = false; /* we are done. */
    }
    else
    {
error:
        playback_ip->apiInProgress = false;
        playback_ip->apiCompleted = false;
        B_PlaybackIp_HttpSessionClose(playback_ip);
    }
    return (errorCode);
}

static int
readAppHeader(
    B_PlaybackIpHandle playback_ip
    )
{
    int rc = -1;
    char *tmpPtr;
    do {
        tmpPtr = strstr(playback_ip->responseMessage, "BCM-App-Header-Inserted: length=");
        if (tmpPtr) {
            char *tmpPtr1;
            B_PlaybackIpAppHeader *appHeader;
            tmpPtr += strlen("BCM-App-Header-Inserted: length=");
            tmpPtr1 = strstr(tmpPtr, "\r\n");
            if (tmpPtr1) {
                appHeader = &playback_ip->appHeader;
                *tmpPtr1 = '\0';
                appHeader->length = strtol(tmpPtr, (char **)NULL, 10);
                *tmpPtr1 = '\r';
                BDBG_MSG(("%s: length %d", __FUNCTION__, appHeader->length));
                if (appHeader->length <= 0 || appHeader->length > sizeof(appHeader->data)) {
                    BDBG_ERR(("%s: read invalid appHeader length %d, should be > 0 & < %zu", __FUNCTION__, appHeader->length, sizeof(appHeader->data)));
                    rc = -1;
                    break;
                }
                rc = playback_ip_read_socket(playback_ip, playback_ip->securityHandle, playback_ip->socketState.fd, (char *)appHeader->data, appHeader->length, playback_ip->networkTimeout);
                if (rc <= 0) {
                    if (errno == EAGAIN) {
                        /* can happen due to DTCP/IP PCP header being present before the app header, that is why need to read another time */
                        rc = playback_ip_read_socket(playback_ip, playback_ip->securityHandle, playback_ip->socketState.fd, (char *)appHeader->data, appHeader->length, playback_ip->networkTimeout);
                    }
                    if (rc <=0) {
                        BDBG_ERR(("%s: failed to read appHeader: length %d", __FUNCTION__, appHeader->length));
                        rc = -1;
                        break;
                    }
                }
                BDBG_MSG(("%s: appHeader data: %d, %d, %d, %d, %d, %d, %d, %d", __FUNCTION__, appHeader->data[0], appHeader->data[1], appHeader->data[2], appHeader->data[3], appHeader->data[4], appHeader->data[5], appHeader->data[6], appHeader->data[7]));
            }
        }
    } while(0);

    return rc;
}

/*
   Function does following:
   -sdf
   -sdf
*/
B_PlaybackIpError
B_PlaybackIp_HttpSessionSetup(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionSetupSettings *setupSettings,
    B_PlaybackIpSessionSetupStatus *setupStatus /* [out] */
    )
{
    int rc = -1;
    B_PlaybackIpError errorCode = B_ERROR_PROTO;
    char *http_hdr;
    B_PlaybackIpPsiInfo *psi;

    if (!playback_ip || !setupSettings || !setupStatus) {
        BDBG_ERR(("%s: invalid params, playback_ip %p, setupSettings %p, setupStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)setupSettings, (void *)setupStatus));
        errorCode = B_ERROR_INVALID_PARAMETER;
        return errorCode;
    }

    /* if SessionSetup is in progress, return INCOMPLETE */
    if (playback_ip->apiInProgress)
        return B_ERROR_IN_PROGRESS;

    /* if SessionSetup is completed, return results to app */
    if (playback_ip->apiCompleted) {
        BDBG_MSG(("%s: previously started session setup operation completed, playback_ip %p", __FUNCTION__, (void *)playback_ip));
        /* Note: since this api was run in a separate thread, we defer thread cleanup until the Ip_Start */
        /* as this call to read up the session status may be invoked in the context of this thread via the callback */
        goto done;
    }

    /* Neither SessionSetup is in progress nor it is completed, so start setup */
    playback_ip->apiInProgress = true;
    playback_ip->apiCompleted = false;

    if (strstr(playback_ip->responseMessage, "BCM-App-Header-Inserted")) {
        if (readAppHeader(playback_ip) <= 0) {
            BDBG_ERR(("%s: Failed to read App specific header", __FUNCTION__));
            errorCode = B_ERROR_PROTO;
            goto error;
        }
        playback_ip->appHeader.valid = true;
    }

    /* If App didn't specify securityProtocol in SessionOpen but does it during SessionSetup, configure the security. */
    /* This can happen when app doesn't apriori know the security protocol but discovers it via the HTTP Response headers (e.g. in DTCP/IP). */
    if (playback_ip->openSettings.security.securityProtocol == B_PlaybackIpSecurityProtocol_None && setupSettings->security.securityProtocol != B_PlaybackIpSecurityProtocol_None)
    {
        int rc = 0;
        void *securityHandle = playback_ip->securityHandle;

        playback_ip->openSettings.security.securityProtocol = setupSettings->security.securityProtocol;
        playback_ip->openSettings.security.initialSecurityContext = setupSettings->security.initialSecurityContext;

        rc = B_PlaybackIp_SecuritySessionOpen(playback_ip, &playback_ip->openSettings, playback_ip->socketState.fd, &playback_ip->securityHandle);

        if (rc < 0) {
            BDBG_ERR(("%s: ERROR: failed to open security session, rc = %d\n", __FUNCTION__, rc));
            errorCode = B_ERROR_PROTO;
            goto error;
        }

        if (playback_ip->netIo.close)
            playback_ip->netIo.close(securityHandle);

        rc = B_PlaybackIp_SecurityDecryptionEnable(playback_ip, playback_ip->temp_buf, &playback_ip->initial_data_len);
        if (rc < 0) {
            BDBG_ERR(("%s: ERROR: failed to enable descryption, rc = %d\n", __FUNCTION__, rc));
            errorCode = B_ERROR_PROTO;
            goto error;
        }
    }

    if (!playback_ip->contentLength && setupSettings->u.http.contentLengthHint) {
        /* if server is streaming content using HTTP chunk xfer encoding, content length is not set */
        /* setting this value allows index information to be built and thus trickmodes will work on stream */
        playback_ip->contentLength = setupSettings->u.http.contentLengthHint;
        BDBG_MSG(("%s: Updated Content Length to user supplied hint %"PRId64 "", __FUNCTION__, playback_ip->contentLength));
    }
    /* check if HTTP server provided any PSI info in the HTTP response header (Broadcom HTTP streamer does that) */
    /* in that case, we dont need to do explicit probe operation (which can take several seconds) and */
    /* SessionSetup is immediately completed. Also, Nexus Playpump must be used to feed IP streams, so we dont need to create Nexus file handle */
    http_hdr = playback_ip->responseMessage;
    playback_ip->http_content_type = http_get_payload_content_type(http_hdr);
    psi = &playback_ip->psi;
    B_PlaybackIp_HttpParseRespHeaderForPsi(playback_ip, playback_ip->http_content_type, http_hdr, psi);
    if ((psi->psiValid && psi->numPlaySpeedEntries > 1) || /* found valid PSI info & server supports the server side trick modes, thus we have already obtained media info in HTTP response and thus dont need to setup the index */
            (psi->psiValid && playback_ip->contentLength == 0) || /* PSI info is known in the response & content length is 0, no point in doing detailed media probing to build th index. */
            playback_ip->openSettings.u.http.rvuCompliant || /* RVU: it uses trickmodes similar to server side trickmodes */
            (setupSettings->u.http.skipPsiParsing && playback_ip->openSettings.useNexusPlaypump) || /* we only allow app to tell us to skip PSI parsing if app is going to use Nexus Playpump to feed IP stream */
            (!setupSettings->u.http.dontFeedDataToPlaybackHw && psi->liveChannel) /* we skip Media probe for Live Channels if app doesn't want to playback ip to feed data to hw for two reasons: server provides all necessary info via HTTP response & we dont want to delay the live channels because of probing, saves 2-3 secs */
       ){
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("%s: Skipping Media Probe: %s",
                    __FUNCTION__, playback_ip->openSettings.u.http.rvuCompliant ? "RVU Session: App knows PSI info" :
                                ((psi->psiValid && psi->numPlaySpeedEntries > 1) ? "Server Side Trickmodes" :
                                 (psi->liveChannel ? "Live Channel" : "App knows PSI & is using Nexus Playpump"))));
#endif
        /* set flag to use Nexus Playpump as for any of these cases above, we have to use Nexus Playpump to feed IP streams. */
        /* we can't use Nexus Playback interface because all of these modes can do server side trickmodes which causes Nexus Playback to loose position info */
        /* as Nexus Playback doesn't know about the scaled position that server sends back. */
        playback_ip->useNexusPlaypump = true;
    }
    else {
        /* we didn't get PSI info from the server, it is a non-RVU non-live session where app doesn't want to skip the PSI discovery & not use Playpump*/
        /* so we may have to setup the Nexus File Handle for both data and index and optionally do PSI discovery. */

        if (psi->numPlaySpeedEntries > 1 || setupSettings->u.http.liveChannel)
            /* this happens when server hasn't sent us the PSI info in the response. But since it supports playspeeds, we will need to use playpump for feeding data */
            /* as only it (& not nexus playback) gives us control for the server side trickmodes. We will let the code below use the media probe to find the PSI info and continue w/ playpump thread */
            playback_ip->useNexusPlaypump = true;

        /* so we create a Nexus File handle representing the network socket (both data & index handles are mapped to the network socket) */
        playback_ip->nexusFileHandle = B_PlaybackIp_HttpCreateFilePlayHandle(playback_ip, &playback_ip->bufferedFdIndex);
        if (playback_ip->nexusFileHandle == NULL) {
            BDBG_ERR(("%s: Failed to Create the Playback Handles\n", __FUNCTION__));
            errorCode = B_ERROR_OUT_OF_MEMORY;
            goto error;
        }
        BDBG_MSG(("%s: created Nexus file IO handle %p", __FUNCTION__, (void *)playback_ip->nexusFileHandle));

        /* check if we need to skip the probe, if so, we can't not setup the index descriptor as index info is built/discovered due to skipping PSI discovery */
        if (setupSettings->u.http.skipPsiParsing) {
            /* app told us to skip the probing as it already knows the media info */
            /* typically app does this for LPCM, RAD/EA encrypted MP3 streams */
            struct bfile_in_net *filePlay;
            BDBG_MSG(("%s: Skipping Media Probe as App asked to skip it", __FUNCTION__));

            /* turn off trick modes (by not providing the index file descriptor in file handle to the Playback */
            if (playback_ip->fileBuffered) {
                bfile_buffered_detach(playback_ip->fileBuffered);
                bfile_buffered_destroy(playback_ip->fileBuffered);
                playback_ip->fileBuffered = NULL;
            }
            if (playback_ip->factory) {
                batom_factory_destroy(playback_ip->factory);
                playback_ip->factory = NULL;
            }

            /* reset index handle */
            filePlay = (struct bfile_in_net *)playback_ip->nexusFileHandle;
            memset(&filePlay->index, 0, sizeof(filePlay->index));
            filePlay->self.file.index = NULL;

            if (setupSettings->u.http.avgBitRate) {
                psi->maxBufferDuration = (playback_ip->dataCache[0].size*8 / setupSettings->u.http.avgBitRate)*1000;  /* in msec */
                psi->avgBitRate = setupSettings->u.http.avgBitRate;
            }
        }
        else {
            /* App has asked for PSI info and it was not available via HTTP header response, so we will need to probe the media */
            /* This probe operation may require requesting data from server at different offsets and thus can take over seconds */
            /* So if app has requested *nonBlockingMode*, then carry out probe operation in a thread and return IN_PROGRESS to app */
            /* When this operation is completed, app is notified via the eventCallback and it can then re-issue SessionSetup to */
            /* read the results. */
            if (playback_ip->openSettings.nonBlockingMode) {
                /* do PSI parsing in a thread and return back to app */
                playback_ip->sessionSetupThread = B_Thread_Create("SessionSetupThread", (B_ThreadFunc)http_parse_av_info, (void *)playback_ip, NULL);
                if (NULL == playback_ip->sessionSetupThread) {
                    BDBG_ERR(("%s: Failed to create thread for media probe during Session Setup\n", __FUNCTION__));
                    errorCode = B_ERROR_UNKNOWN;
                    goto error;
                }
#ifdef BDBG_DEBUG_BUILD
                if (playback_ip->ipVerboseLog)
                    BDBG_WRN(("%s: Non blocking media probe operation started: playback_ip %p\n", __FUNCTION__, (void *)playback_ip));
#endif
                errorCode = B_ERROR_IN_PROGRESS;
                goto error;
            }
            else {
                http_parse_av_info(playback_ip);
                /* success case, fall below to the done label */
            }
        }
    }

done:
    /* dont copy PSI info if App had asked us to skip PSI parsing */
    if (!setupSettings->u.http.skipPsiParsing) {
        if (!playback_ip->psi.psiValid) {
            BDBG_ERR(("%s: Failed to acquire PSI info via media probe\n", __FUNCTION__));
            errorCode = B_ERROR_UNKNOWN;
            goto error;
        }
        setupStatus->u.http.psi = playback_ip->psi;
        setupStatus->u.http.stream = (void *)playback_ip->stream;
    }
    if (setupSettings->u.http.liveChannel)
        /* app told us that it is going to play a live channel, we will set that flag in the PSI structure */
        setupStatus->u.http.psi.liveChannel = true;
    if (setupSettings->u.http.readTimeout == 0)
        /* app probably didn't configure this value */
        playback_ip->setupSettings.u.http.readTimeout = 100; /* 100msec */
    setupStatus->u.http.file = playback_ip->nexusFileHandle;
    if (playback_ip->appHeader.valid) {
        setupStatus->u.http.appHeader = playback_ip->appHeader;
    }
    errorCode = B_ERROR_SUCCESS;

    /* success */
    rc = 0;

error:
    if (errorCode != B_ERROR_IN_PROGRESS) {
        playback_ip->apiInProgress = false;
        playback_ip->apiCompleted = false;
    }
    if (rc < 0) {
        return errorCode;
    }
    else {
        return B_ERROR_SUCCESS;
    }
}

/* HTTP processing thread: happens in the context of the Nexus IO threads */
/* Main purpose of this thread is to perform the pre-charging function when requested by app. */
/* App can request to do pre-charging either at the Ip_Start() time or during runtime. */
/* For runtime pre-charging, app uses the SetSettings function to indicate to this thread */
/* to start pre-charging. */
/* Note: once pre-charging is done, most content processing happens in the context of the Nexus IO threads */
void B_PlaybackIp_HttpPlaybackThread(
    void *data
    )
{
    B_PlaybackIpHandle playback_ip;
    int cacheIndex;
    off_t offset, adjustedOffset;
    ssize_t bytesRead = 0;
    int bytesToRead = 0;
    int spaceAvailableAtEnd;
    int rc = -1;
    bool sendEvent = false;
    char *internalError = NULL;

    playback_ip = (B_PlaybackIpHandle)data;
    if (playback_ip->settings.networkTimeout) {
        if (playback_ip->settings.networkTimeout > (HTTP_SELECT_TIMEOUT/10))
            playback_ip->networkTimeout = HTTP_SELECT_TIMEOUT/10;
        else
            playback_ip->networkTimeout = playback_ip->settings.networkTimeout;
    }
    else {
        playback_ip->networkTimeout = HTTP_SELECT_TIMEOUT/10;
    }
    BDBG_MSG(("%s: n/w timeout %d secs", __FUNCTION__, playback_ip->networkTimeout));

    /* main loop */
    /* TODO: extend this thread to do multiple functions:
       -process 2nd half of the SessionSetup command (where actual media happens)
       -process request to do runtime buffering
    */
    while (true) {
        if  (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
            /* user changed the channel, so return */
            BDBG_MSG(("%s: breaking out of main pre-charging loop due to state (%d) change", __FUNCTION__, playback_ip->playback_state));
            break;
        }
        if (!playback_ip->preChargeBuffer && !playback_ip->sendNextTimeSeekReq) {
            /* app hasn't yet asked us to start pre-charging or turned off the flag, nor it wants us to send the time seek req for rvus, so wait */
            if (playback_ip->playback_state == B_PlaybackIpState_eBuffering) {
                playback_ip->playback_state = B_PlaybackIpState_ePlaying;
                BKNI_SetEvent(playback_ip->preChargeBufferEvent);
            }
            /* RVU TODO: check if 20msec isn't too low to impose extra overhead or not too high cause extra latency during trickplay */
            rc = BKNI_WaitForEvent(playback_ip->newJobEvent, 20);
            if (rc == BERR_TIMEOUT) {
                /* event timeout, thus no new task to do, go back and retry it */
                continue;
            } else if (rc!=0) {
                BDBG_ERR(("%s: got error while waiting for new task event, rc %d", __FUNCTION__, rc));
                break;
            }
            /* else we got a new task to do: so do the work below */
        }

        BKNI_AcquireMutex(playback_ip->lock);

        /* change playback_ip state to buffering state: this forces the read callbacks from the Nexus IO threads to return a timeout condition */
        /* this way we ensure that only one thread is working on the data cache */
        playback_ip->playback_state = B_PlaybackIpState_eBuffering;

        /* send event to let app thread know we have started buffering */
        BKNI_SetEvent(playback_ip->preChargeBufferEvent);

        /* wait until the nexus io threads has finished reading socket data in the read callbacks, otherwise we will have cache corruption */
        if (playback_ip->readCallBackInProgress) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: read callback via nexus io thread is still active, HTTP thread has to wait for its completion, sleep & retry\n", __FUNCTION__));
#endif
            BKNI_ReleaseMutex(playback_ip->lock);
            BKNI_Sleep(100);
            continue;
        }

        /* determine which data cache will be used for pre-charging: start with data cache 0 */
        if (!playback_ip->dataCache[0].inUse) {
            int initialDataLength;
            /* Since data cache is not yet setup, this must be the initial buffering request before playback has started. */
            /* Initialize data cache: index cache should already be setup by now (happens during media probe) */
            if (playback_ip->setupSettings.u.http.disableRangeHeader) {
                /* Since we can't send Range Request, we just start from the begining */
                initialDataLength = 0;
                playback_ip->dataCache[0].startOffset = 0;
                playback_ip->dataCache[0].endOffset = playback_ip->dataCache[0].startOffset - 1;
                BDBG_MSG(("%s: Can't seek when Range Header is not supported, so cache from begining!\n", __FUNCTION__));
            }
            /* However, for ASF, index data (if present) is at the end of stream. True index data is not acquired during probe until */
            /* playback starts. Thus, during initial buffering for ASF, we copy the content of index cache (mostly ASF headers) into the data cache*/
            else if (playback_ip->psi.mpegType == NEXUS_TransportType_eAsf) {
                /* Copy currently read data from index to data cache as it is really the movie hdr & data & not index data */
                initialDataLength = playback_ip->indexCacheDepth;
                memcpy(playback_ip->dataCache[0].cache, playback_ip->indexCache, initialDataLength);
                BDBG_MSG(("%s: copied %d bytes from index to data cache for ASF\n", __FUNCTION__, initialDataLength));

                playback_ip->dataCache[0].endOffset = playback_ip->indexCacheDepth -1;
                playback_ip->dataCache[0].startOffset = 0;
            }
            else {
                initialDataLength = 0;
                if (playback_ip->indexCacheEndOffset > 0)
                    playback_ip->dataCache[0].startOffset = playback_ip->indexCacheEndOffset + 1;
                else
                    playback_ip->dataCache[0].startOffset = 0;
                playback_ip->dataCache[0].endOffset = playback_ip->dataCache[0].startOffset - 1;
            }

            /* need to set this as this otherwise get status keeps returning 0 buffer depth due to lastSeekOffset not changing */
            playback_ip->lastSeekOffset = playback_ip->dataCache[0].startOffset;
            playback_ip->dataCache[0].readIndex = 0;
            playback_ip->dataCache[0].depth = initialDataLength;
            playback_ip->dataCache[0].writeIndex = initialDataLength;
            playback_ip->dataCache[0].inUse = true;
            playback_ip->cacheDepthFudgeFactor = 0;
            cacheIndex = 0;
            BDBG_MSG(("%s: Data Cache (# %d) setup during pre-charging: offset start %"PRId64 ", end %"PRId64 ", depth %d, index rd %u, wr %u\n",
                    __FUNCTION__, cacheIndex, playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
                    playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].readIndex, playback_ip->dataCache[cacheIndex].writeIndex));
        }
        else if ( (playback_ip->dataCache[playback_ip->lastUsedCacheIndex].endOffset+1) == playback_ip->contentLength) {
            /* we are asked to buffer, but we have already received the complete content from server */
            /* so this must be the rewind case where app is asking us to pre-charge before replaying */
            /* we reset the cache and start buffering from begining */

            /* select the cache that was used last by the io threads to read data */
            cacheIndex = playback_ip->lastUsedCacheIndex;
            playback_ip->dataCache[cacheIndex].readIndex = 0;
            playback_ip->dataCache[cacheIndex].depth = 0;
            playback_ip->dataCache[cacheIndex].writeIndex = 0;
            playback_ip->dataCache[cacheIndex].startOffset = 0;
            playback_ip->lastSeekOffset = playback_ip->dataCache[0].startOffset;
            playback_ip->dataCache[cacheIndex].endOffset = playback_ip->dataCache[cacheIndex].startOffset - 1;
            playback_ip->cacheDepthFudgeFactor = 0;
            /* set flag to indicate that we need to re-open the socket */
            playback_ip->reOpenSocket = true;
            playback_ip->serverClosed = false;
            BDBG_MSG(("%s: Data Cache (# %d) pre-charging during rewind: offset start %"PRId64 ", end %"PRId64 ", depth %d, index rd %u, wr %u\n",
                    __FUNCTION__, cacheIndex, playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
                    playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].readIndex, playback_ip->dataCache[cacheIndex].writeIndex));
        }
        else {
            /* This pre-charging request is after the data cache has been setup (either during playback or when index is at end for MP4) */
            /* We move the start index forward to the last used offset value, accordingly reduce the buffer depth, */
            /* and then start buffering after the point upto which we have already buffered */

            /* select the cache that was used last by the io threads to read data */
            cacheIndex = playback_ip->lastUsedCacheIndex;
            BDBG_MSG(("%s: before adjustments: Data Cache (# %d): lastSeekOffset %"PRId64 ", offset start %"PRId64 ", end %"PRId64 ", depth %d, index rd %u, wr %u\n",
                    __FUNCTION__, cacheIndex, playback_ip->lastSeekOffset, playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
                    playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].readIndex, playback_ip->dataCache[cacheIndex].writeIndex));
            if (playback_ip->lastSeekOffset > playback_ip->dataCache[cacheIndex].startOffset && playback_ip->lastSeekOffset <= playback_ip->dataCache[cacheIndex].endOffset) {
                int remainder;
                /* last seek offset is in the cache */
                adjustedOffset = playback_ip->lastSeekOffset;
                /* round off the offset to the chunk size */
                remainder = (adjustedOffset%HTTP_DATA_CACHE_CHUNK_SIZE);
                adjustedOffset -= remainder;
                /* move offset back by n chunks: mainly because for MP4 when we resume, player might need some of the previous data */
                adjustedOffset -= HTTP_CACHE_DEPTH_FUDGE_FACTOR;
                /* we rememeber this fudge factor as we adjust max buffer size by this amount in the GetStatus call */
                playback_ip->cacheDepthFudgeFactor = HTTP_CACHE_DEPTH_FUDGE_FACTOR + remainder;
                if (adjustedOffset <= playback_ip->dataCache[cacheIndex].startOffset) {
                    BDBG_MSG(("%s: adjusted offset was going to go before cache start, resetting it to cacheStart\n", __FUNCTION__));
                    offset = 0;
                }
                else {
                    offset = adjustedOffset - playback_ip->dataCache[cacheIndex].startOffset;
                    BDBG_MSG(("%s: moving start offset by %"PRId64 " bytes\n", __FUNCTION__, offset));
                }
                /* startOffset is moved forward to the adjustedOffset */
                playback_ip->dataCache[cacheIndex].startOffset += offset;
                /* readIndex is moved by the offset amount, taking care of the wrapping */
                playback_ip->dataCache[cacheIndex].readIndex += offset;
                playback_ip->dataCache[cacheIndex].readIndex %= playback_ip->dataCache[cacheIndex].size;
                /* cache depth is accordingly adjusted to indicate additional space that has been created for buffering */
                playback_ip->dataCache[cacheIndex].depth = playback_ip->dataCache[cacheIndex].endOffset - playback_ip->dataCache[cacheIndex].startOffset + 1;
                if (playback_ip->dataCache[cacheIndex].depth >= playback_ip->dataCache[cacheIndex].size || playback_ip->dataCache[cacheIndex].depth <= 0) {
                    internalError = "CacheDepthNotCorrect";
                    goto error;
                }
                BDBG_MSG(("%s: Data Cache (# %d) Pruning during pre-charging: lastSeekOffset %"PRId64 ", offset start %"PRId64 ", end %"PRId64 ", depth %d, index rd %u, wr %u\n",
                    __FUNCTION__, cacheIndex, playback_ip->lastSeekOffset, playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
                    playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].readIndex, playback_ip->dataCache[cacheIndex].writeIndex));
            }
            else {
                int remainder;
                /* last seek offset is NOT in the cache, reset cache & reopen connex from last seek offset */
                playback_ip->dataCache[cacheIndex].readIndex = 0;
                playback_ip->dataCache[cacheIndex].depth = 0;
                playback_ip->dataCache[cacheIndex].writeIndex = 0;
                /* last seek offset is in the cache */
                adjustedOffset = playback_ip->lastSeekOffset;
                /* round off the offset to the chunk size */
                remainder = (adjustedOffset%HTTP_DATA_CACHE_CHUNK_SIZE);
                adjustedOffset -= remainder;
                /* move offset back by n chunks: mainly because for MP4 when we resume, player might need some of the previous data */
                adjustedOffset -= HTTP_CACHE_DEPTH_FUDGE_FACTOR;
                if (adjustedOffset < 0) {
                    adjustedOffset = 0;
                }
                else
                    /* we rememeber this fudge factor as we adjust max buffer size by this amount in the GetStatus call */
                    playback_ip->cacheDepthFudgeFactor = HTTP_CACHE_DEPTH_FUDGE_FACTOR + remainder;
                playback_ip->dataCache[cacheIndex].startOffset = adjustedOffset;
                playback_ip->dataCache[cacheIndex].endOffset = playback_ip->dataCache[cacheIndex].startOffset - 1;
                /* set flag to indicate that we need to re-open the socket */
                playback_ip->reOpenSocket = true;
                BDBG_MSG(("%s: Data Cache (# %d) pre-charging: seekOffset is outside current cache, restarting cache from it: offset lastSeek %"PRId64 ", start %"PRId64 ", end %"PRId64 ", depth %d, index rd %u, wr %u\n",
                        __FUNCTION__, cacheIndex, playback_ip->lastSeekOffset, playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
                        playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].readIndex, playback_ip->dataCache[cacheIndex].writeIndex));
            }
        }

        /* we keep pre-charging the network buffer until either preChargeBuffer flag is turned off (via B_PlaybackIp_SetSetting()) or network buffer gets full. */
        while (true) {
            sendEvent = true;
            if  (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
                /* user changed the channel, so return */
                BDBG_MSG(("%s: breaking out of current pre-charging loop due to state (%d) change\n", __FUNCTION__, playback_ip->playback_state));
                break;
            }
            if (!playback_ip->preChargeBuffer) {
                /* app is now asking to stop preCharging, so break */
                BDBG_MSG(("%s: preChargeBuffer flag is now false, stop pre-charging (playback ip state %d) \n", __FUNCTION__, playback_ip->playback_state));
                break;
            }

            /* See if there is room in cache to read n/w data */
            if (playback_ip->dataCache[cacheIndex].depth >= playback_ip->dataCache[cacheIndex].size || playback_ip->serverClosed) {
                BDBG_MSG(("%s: Pre-Charge buffer is full (size %d, depth %d) or eof (%d), waiting for app to stop pre-charging, till then sleeping...\n",
                            __FUNCTION__, playback_ip->dataCache[cacheIndex].size, playback_ip->dataCache[cacheIndex].depth, playback_ip->serverClosed));
                BKNI_Sleep(500);
                continue;
            }

            /* read lower of data chunk size or space available at end of data cache FIFO */
            spaceAvailableAtEnd = playback_ip->dataCache[cacheIndex].size - playback_ip->dataCache[cacheIndex].writeIndex;
            if (spaceAvailableAtEnd < HTTP_DATA_CACHE_CHUNK_SIZE) {
                BDBG_MSG(("%s: reached towards end of data cache, so reading (%d) less than chunk size (%d), cache size %d, cache wr idx %d\n",
                            __FUNCTION__, spaceAvailableAtEnd, HTTP_DATA_CACHE_CHUNK_SIZE, playback_ip->dataCache[cacheIndex].size, playback_ip->dataCache[cacheIndex].writeIndex));
                bytesToRead = spaceAvailableAtEnd;
            }
            else
                bytesToRead = HTTP_DATA_CACHE_CHUNK_SIZE;

            if (playback_ip->reOpenSocket /* true if index was at the end (e.g. for ASF) and thus server had closed socket a/f sending the index part */
                    || playback_ip->cacheIndexUsingSocket != cacheIndex /* true if we are switching the data caches */
            ) {
                /* in both cases, we need to close the current socket & send a new request to server */
                /* both are done in NetRangeReq()) */
                BDBG_MSG(("%s: re-setup the connection for cache idx %d, prev cache idx %d, re-open socket %d\n", __FUNCTION__, cacheIndex, playback_ip->cacheIndexUsingSocket, playback_ip->reOpenSocket));
                playback_ip->reOpenSocket = false;
                playback_ip->cacheIndexUsingSocket = cacheIndex;
                /* send new HTTP Range request on a new socket */
                rc = B_PlaybackIp_HttpNetRangeReq(playback_ip, playback_ip->dataCache[cacheIndex].cache + playback_ip->dataCache[cacheIndex].writeIndex, bytesToRead, playback_ip->dataCache[cacheIndex].endOffset+1, 0, playback_ip->socketState.fd, &playback_ip->socketState.fd);
            }
            else {
                /* read 1 chunk worth data from n/w */
                rc = playback_ip_read_socket(playback_ip, playback_ip->securityHandle,
                        playback_ip->socketState.fd,
                        playback_ip->dataCache[cacheIndex].cache + playback_ip->dataCache[cacheIndex].writeIndex,
                        bytesToRead, playback_ip->networkTimeout);
           }
           if (rc <= 0) {
#ifdef BDBG_DEBUG_BUILD
               if (playback_ip->ipVerboseLog)
                   BDBG_ERR(("%s: Network Read Error, rc %d, playback ip state %d", __FUNCTION__, rc, playback_ip->playback_state));
#endif
                /* we wait until we recover from n/w error (e.g. select timeout due to temporary n/w loss) or app turns off the pre-charging */
                BKNI_Sleep(100);
                continue;
            }
            bytesRead += rc;

            /* increment data cache write index */
            playback_ip->dataCache[cacheIndex].writeIndex += rc;
            playback_ip->dataCache[cacheIndex].writeIndex %= playback_ip->dataCache[cacheIndex].size;
            /* indicate addition of this new chunk by moving forward the end offset by a chunk size */
            playback_ip->dataCache[cacheIndex].endOffset == 0 ?
                playback_ip->dataCache[cacheIndex].endOffset = rc - 1 :
                (playback_ip->dataCache[cacheIndex].endOffset += rc);
            /* increase data cache depth by read amount */
            playback_ip->dataCache[cacheIndex].depth += rc;

            if (!playback_ip->dataCache[cacheIndex].inUse) {
                playback_ip->dataCache[cacheIndex].inUse = true;
            }
        }

        /* change playback_ip state to back to playing state */
        if (playback_ip->playback_state == B_PlaybackIpState_eBuffering)
            playback_ip->playback_state = B_PlaybackIpState_ePlaying;
        if (sendEvent) {
            BKNI_SetEvent(playback_ip->preChargeBufferEvent);
            sendEvent = false;
        }

        BDBG_MSG(("%s: pre-charged data cache[%d] read %zd: cache start %"PRId64 ", end %"PRId64 " depth %d, wr idx %d",
            __FUNCTION__, cacheIndex, bytesRead, playback_ip->dataCache[cacheIndex].startOffset, playback_ip->dataCache[cacheIndex].endOffset,
            playback_ip->dataCache[cacheIndex].depth, playback_ip->dataCache[cacheIndex].writeIndex));
        bytesRead = 0;
        /* reset the fudge factor */
        playback_ip->cacheDepthFudgeFactor = 0;
        BKNI_ReleaseMutex(playback_ip->lock);
    }

error:
    if (internalError) {
        BDBG_ERR(("%s: sw bug: error hint: %s\n", __FUNCTION__, internalError));
        BKNI_SetEvent(playback_ip->preChargeBufferEvent);
    }
    BDBG_MSG(("%s: thread is exiting..., state %d", __FUNCTION__, playback_ip->playback_state));
    BKNI_SetEvent(playback_ip->playback_halt_event);
    playback_ip->speed = 0;
    BKNI_ReleaseMutex(playback_ip->lock);
    return;
}

void B_PlaybackIp_HttpPlaypumpThread(
    void *data
    )
{
    B_PlaybackIpHandle playback_ip;
    size_t totalBytesRecv;
    ssize_t rc = -1;
    static int fileNameSuffix = 0;
    char recordFileName[32];
    size_t readBufSize;

    playback_ip = (B_PlaybackIpHandle)data;
    if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
        /* user changed the channel before this thread got a chance to start, we are done */
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("%s: returning due to state (%d) change", __FUNCTION__, playback_ip->playback_state));
#endif
        goto out;
    }

    if (playback_ip->settings.networkTimeout) {
        if (playback_ip->settings.networkTimeout > (HTTP_SELECT_TIMEOUT/10))
            playback_ip->networkTimeout = HTTP_SELECT_TIMEOUT/10;
        else
            playback_ip->networkTimeout = playback_ip->settings.networkTimeout;
    }
    else {
        playback_ip->networkTimeout = HTTP_SELECT_TIMEOUT/10;
    }
    BDBG_MSG(("%s: n/w timeout %d secs", __FUNCTION__, playback_ip->networkTimeout));

    if (B_PlaybackIp_UtilsWaitForPlaypumpDecoderSetup(playback_ip))
        goto error;

    if (playback_ip->enableRecording) {
        memset(recordFileName, 0, sizeof(recordFileName));
        snprintf(recordFileName, sizeof(recordFileName)-1, "./videos/http_playpump_rec%d.ts", fileNameSuffix++);
        playback_ip->fclear = fopen(recordFileName, "w+b");
    }

    if (playback_ip->startSettings.monitorPsi) {
        if ( (playback_ip->pPsiState = B_PlaybackIp_CreatePsiState(playback_ip)) == NULL ) {
            BDBG_ERR(("%s: B_PlaybackIp_CreatePsiState() Failed to allocate memory for psiState", __FUNCTION__));
            goto error;
        }
    }
    if (playback_ip->serverClosed || playback_ip->netRangeFunctionInvoked) {
        /* this is to allow the case where app can restart the session by issuing stop & start again */
        /* or when media probe was used and it may have seeked to a different offset (than the start of file) using NetRange() */
        /* and we thus need to reconnect to server to stream from starting offset */
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("%s: serverClosed %d, netRangeInvoked %d", __FUNCTION__, playback_ip->serverClosed, playback_ip->netRangeFunctionInvoked));
#endif
        rc = B_PlaybackIp_HttpNetRangeReq(playback_ip, playback_ip->temp_buf, 0, 0, 0, playback_ip->socketState.fd, &playback_ip->socketState.fd);
        if (rc < 0) {
            BDBG_ERR(("%s: Failed to re-open socket connection", __FUNCTION__));
            goto error;
        }
        playback_ip->initial_data_len = rc;
        playback_ip->serverClosed = false;
        playback_ip->netRangeFunctionInvoked = false;
        playback_ip->indexCacheDepth = 0;
    }

    if (playback_ip->settings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip
        || playback_ip->settings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithPcrNoSyncSlip) {
        B_PlaybackIp_TtsThrottle_Params throttleParams;
        NEXUS_PlaypumpSettings playpumpSettings;

        playback_ip->ttsThrottle = B_PlaybackIp_TtsThrottle_Open();

        B_PlaybackIp_TtsThrottle_GetSettings(playback_ip->ttsThrottle, &throttleParams);
        BKNI_Memcpy(&throttleParams, &playback_ip->settings.ttsParams.throttleParams, sizeof(B_PlaybackIp_TtsThrottle_Params));
        throttleParams.playPump = playback_ip->nexusHandles.playpump;
        NEXUS_Playpump_GetSettings(playback_ip->nexusHandles.playpump, &playpumpSettings);
        throttleParams.timebase = playpumpSettings.timestamp.timebase;
        B_PlaybackIp_TtsThrottle_SetSettings(playback_ip->ttsThrottle, &throttleParams);
        B_PlaybackIp_TtsThrottle_Start(playback_ip->ttsThrottle);
    }

    /* Note we set the size of bytes to read based on audio only or audio & video channel */
    /* we may or may not know the stream bitrate and thus can't set the read size based on that */
    /* that is where readTimeout (set during sessionSetup) comes handy where it will return */
    /* either when all requested amount is available or when the timeout happens */
    if (playback_ip->psi.videoCodec == NEXUS_VideoCodec_eNone && !playback_ip->psi.videoPid && playback_ip->psi.audioPid) {
        /* audio only case */
        readBufSize = 1024*10;
    }
    else {
        /* audio & video case */
        readBufSize = 1024*60;
    }
    /* main loop */
    while (true) {
        BDBG_MSG(("%s:%p: Before Acquiring Mutex! ", __FUNCTION__, (void *)playback_ip));
        BKNI_AcquireMutex(playback_ip->lock);
        if (playback_ip->playback_state == B_PlaybackIpState_eStopping || playback_ip->playback_state == B_PlaybackIpState_eStopped) {
            /* user changed the channel, so return */
            BDBG_MSG(("%s: breaking out of HTTP loop due to state (%d) change", __FUNCTION__, playback_ip->playback_state));
            break;
        }
        if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode) {
            BKNI_ReleaseMutex(playback_ip->lock);
            BDBG_MSG(("%s: pausing feeder thread to allow trickmode transitions", __FUNCTION__));
            BKNI_Sleep(20); /* wait little bit before going back to the top, this allows trickmode thread to get the lock first, finish the trickmode transition and then release the lock */
            continue;
        }

        /* get an adequately sized buffer from the playpump */
        if (B_PlaybackIp_UtilsGetPlaypumpBuffer(playback_ip, readBufSize) < 0) {
            BKNI_ReleaseMutex(playback_ip->lock);
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: Failed to get buffer from playpump, retry ", __FUNCTION__));
#endif
            BKNI_Sleep(100);
            continue;
        }

        if (playback_ip->indexCacheDepth) {
            unsigned bytesToCopy;
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_MSG(("%s: copying the initial %d bytes to playpump buffer", __FUNCTION__, playback_ip->indexCacheDepth));
#endif
            /* copy lower of the indexCacheBuffer depth or available playpump buffer. */
            bytesToCopy = playback_ip->indexCacheDepth < readBufSize ? playback_ip->indexCacheDepth : readBufSize;
            BKNI_Memcpy(playback_ip->buffer, playback_ip->indexCache, bytesToCopy);
            rc = bytesToCopy;
            playback_ip->indexCacheDepth -= bytesToCopy;
        }
        else if (playback_ip->initial_data_len) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_WRN(("%s: copying the initial %d bytes to playpump buffer", __FUNCTION__, playback_ip->initial_data_len));
#endif
            BKNI_Memcpy(playback_ip->buffer, playback_ip->temp_buf, playback_ip->initial_data_len);
            rc = playback_ip->initial_data_len;
            playback_ip->initial_data_len = 0;
        }
        else {
            /* read the requested range of data chunk from socket */
            BDBG_MSG(("%s: Got playpump buffer to read more data into, now Read %zu bytes from socket (fd=%d) into this buffer", __FUNCTION__, readBufSize, playback_ip->socketState.fd ));
            rc = playback_ip_read_socket(playback_ip, playback_ip->securityHandle, playback_ip->socketState.fd, (char *)playback_ip->buffer, readBufSize, playback_ip->networkTimeout);
            if (rc <= 0) {
                BDBG_MSG(("%s: rc %zd, serverClosed %d, selectTimeout %d", __FUNCTION__, rc, playback_ip->serverClosed, playback_ip->selectTimeout));
                if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode || playback_ip->selectTimeout) {
                    BKNI_ReleaseMutex(playback_ip->lock);
#ifdef BDBG_DEBUG_BUILD
                    if (playback_ip->ipVerboseLog)
                        BDBG_WRN(("%s: %s", __FUNCTION__, playback_ip->selectTimeout? "no data coming from server, retry!": "pausing feeder thread to allow trickmode transitions"));
#endif
                    BKNI_Sleep(50);
                    continue;
                }
                else if (playback_ip->serverClosed && playback_ip->openSettings.u.http.rvuCompliant && playback_ip->playback_state == B_PlaybackIpState_eTrickMode) {
                    /* server has closed (either via end of chunk header or socket close) & we are in trickmode state */
                    /* set a flag to indicate to the HTTP thread to send next time seek request */
                    /* fake these values as it will make this function to return TIMEOUT message to the player and make it retry the read */
                    rc = 0;
                    playback_ip->serverClosed = false;
                    playback_ip->selectTimeout = true;
                    playback_ip->sendNextTimeSeekReq = true;
                    /* TODO: needs more work */
                    BDBG_MSG(("%s: RVU trickmode case, finished reading one set of frames, send request for the next frame", __FUNCTION__));
                    /* we are in the trick mode state where we need to send new time seek request */
                    if ((rc = http_send_time_seek_request(playback_ip)) != B_ERROR_SUCCESS) {
                        if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode) {
                            BKNI_ReleaseMutex(playback_ip->lock);
                            BDBG_MSG(("%s: pausing feeder thread to allow trickmode transitions", __FUNCTION__));
                            BKNI_Sleep(50);
                            continue;
                        }
                        BDBG_MSG(("%s: RVU Trick mode transition had some error, ip state %d \n", __FUNCTION__, playback_ip->playback_state));
                        goto error;
                    }
                    BKNI_ReleaseMutex(playback_ip->lock);
                    BDBG_MSG(("%s: RVU trickmode: done sending next time seek request, continue reading & feeding", __FUNCTION__));
                    continue;
                }
                else if (playback_ip->serverClosed && playback_ip->playback_state == B_PlaybackIpState_ePaused && playback_ip->ipTrickModeSettings.pauseMethod == B_PlaybackIpPauseMethod_UseDisconnectAndSeek ) {
                    BKNI_ReleaseMutex(playback_ip->lock);
                    BDBG_MSG(("%s: Paused using Disconnect & Seek Method, so keep waiting until app resumes!", __FUNCTION__));
                    BKNI_Sleep(20);
                    continue;
                }
                else {
                    if (playback_ip->settings.enableEndOfStreamLooping) {
#define PLAYSPEED_STRING_SIZE 16
                        char playSpeedString[PLAYSPEED_STRING_SIZE];
                        double timeSeekRangeBegin = 0.;
                        playback_ip->serverClosed = false;

                        if ( playback_ip->playback_state == B_PlaybackIpState_eTrickMode) {
                            if (playback_ip->ipTrickModeSettings.playSpeedStringDefined) {
                                BKNI_Memset(playSpeedString, 0, PLAYSPEED_STRING_SIZE);
                                snprintf(playSpeedString, PLAYSPEED_STRING_SIZE-1, "%d/%d", playback_ip->speedNumerator, playback_ip->speedDenominator);
                            }
                            if (playback_ip->speedNumerator > 0)
                                timeSeekRangeBegin = 0;
                            else {
                                if (playback_ip->psi.duration > 0)
                                    timeSeekRangeBegin = playback_ip->psi.duration-1;
                                else {
                                    BDBG_ERR(("%s: Failed to loop around when duration is not known!", __FUNCTION__));
                                    break;
                                }
                            }
                            BDBG_MSG(("%s: state %d, playSpeedString %s",
                                        __FUNCTION__, playback_ip->playback_state,  playback_ip->ipTrickModeSettings.playSpeedStringDefined ? playback_ip->ipTrickModeSettings.playSpeedString : NULL));
                            rc = http_do_server_trick_modes_socket_transition(playback_ip, timeSeekRangeBegin, 0., playback_ip->speedNumerator/playback_ip->speedDenominator /* play speed */,
                                    playback_ip->ipTrickModeSettings.playSpeedStringDefined ? playSpeedString : NULL);
                            playback_ip->reOpenSocket = true; /* this flags tells HTTP layer to reopen the connection */
                        }
                        else {
                            rc = B_PlaybackIp_HttpNetRangeReq(playback_ip, playback_ip->temp_buf, 0, 0, 0, playback_ip->socketState.fd, &playback_ip->socketState.fd);
                        }
                        if (rc < 0) {
                            BDBG_ERR(("%s: Failed to re-open socket connection", __FUNCTION__));
                            break;
                        }
                        playback_ip->initial_data_len = rc;
                        BKNI_ReleaseMutex(playback_ip->lock);
#ifdef BDBG_DEBUG_BUILD
                        if (playback_ip->ipVerboseLog)
                            BDBG_WRN(("%s: Server has Closed but endOfStreamLooping is enabled, so resetup server connection", __FUNCTION__));
#endif
                        continue;
                    }
                    else {
                        BDBG_MSG(("%s: Network Read Error, wait to play out the stream", __FUNCTION__));
                        /* wait for h/w pipeline to playout the buffered up data and then issue the error/eof callback */
                        if (B_PlaybackIp_UtilsEndOfStream(playback_ip) == true)
                        {
                            if (playback_ip->settings.waitOnEndOfStream)
                            {
                                /* We have played all the AV data, so we move to the paused state and wait for app to take some action: Play/Stop/ etc. */
                                BKNI_ReleaseMutex(playback_ip->lock);
                                if (playback_ip->playback_state == B_PlaybackIpState_eStopping) {
                                    BDBG_WRN(("%s:%p Reached endOfStream & app also called Stop, so breaking from loop!", __FUNCTION__, (void *)playback_ip));
                                    break;
                                }
                                playback_ip->playback_state = B_PlaybackIpState_ePaused;
                                if (playback_ip->openSettings.eventCallback)
                                {
                                    B_PlaybackIpEventIds eventId;
                                    eventId = (playback_ip->speedNumerator > 0) ? B_PlaybackIpEvent_eServerEndofStreamReached: B_PlaybackIpEvent_eClientBeginOfStream;
#ifdef BDBG_DEBUG_BUILD
                                    if (playback_ip->ipVerboseLog)
                                        BDBG_WRN(("%s: Reached endOfStream (server has closed & nothing to play anymore) & app wants us to wait for next API, issue callback w/ eventId %d & do so!", __FUNCTION__, eventId));
#endif
                                    playback_ip->openSettings.eventCallback(playback_ip->openSettings.appCtx, eventId);
                                }
                                BKNI_Sleep(20);
                                continue;
                            }
                            else
                            {
                                /* We are done, */
                                break;
                            }
                        }
                        else {
                            /* continue waiting to either playout the whole stream or re-reading from socket incase it made valid again by a seek or rewind trickplay */
                            BKNI_ReleaseMutex(playback_ip->lock);
#ifdef BDBG_DEBUG_BUILD
                            if (playback_ip->ipVerboseLog)
                                BDBG_WRN(("%s: Continue waiting to either playout the whole stream or re-reading from socket incase it becomes valid again by a seek or rewind trickplay", __FUNCTION__));
#endif
                            BKNI_Sleep(B_PlaybackIp_UtilsGetEndOfStreamTimeout(playback_ip));
                            continue;
                        }
                    }
                }
            }
        }
        playback_ip->totalConsumed += rc;
        totalBytesRecv = rc;
        BDBG_MSG(("%s: Read from socket: asked %zu bytes, returning %zd bytes, total Read %"PRId64 ", Consumed %"PRId64 "",
                    __FUNCTION__, totalBytesRecv, rc, playback_ip->totalRead, playback_ip->totalConsumed));

        /* write data to file */
        if (playback_ip->enableRecording && playback_ip->fclear) {
            fwrite(playback_ip->buffer, 1, totalBytesRecv, playback_ip->fclear);
            fflush(playback_ip->fclear);
        }
        if (playback_ip->startSettings.monitorPsi) {
            if (B_PlaybackIp_ParseAndProcessPsiState( playback_ip, playback_ip->buffer, totalBytesRecv ) != B_ERROR_SUCCESS ) {
                BDBG_WRN(("%s: B_PlaybackIp_ParseAndProcessPsi failed but feed %zu bytes into Playpump...", __FUNCTION__, totalBytesRecv));
            }
        }

#if 0
        /* Inject noise. */
        if (playback_ip->totalConsumed > 4000000) {
            static int loopCnt = 0;
            if (loopCnt++ % 5 == 0) {
                BKNI_Memset(playback_ip->buffer, 0, 2000);
            }
        }
#endif
        /* now feed appropriate data it to the playpump */
        if (NEXUS_Playpump_WriteComplete(playback_ip->nexusHandles.playpump, 0, totalBytesRecv)) {
            BDBG_WRN(("%s: NEXUS_Playpump_WriteComplete failed, continuing...", __FUNCTION__));
            BKNI_ReleaseMutex(playback_ip->lock);
            continue;
        }
        BKNI_ReleaseMutex(playback_ip->lock);
        BDBG_MSG(("%s: Fed %zu bytes to Playpump\n", __FUNCTION__, totalBytesRecv));
    }

error:
    BKNI_ReleaseMutex(playback_ip->lock);
    /* TODO: look if tts stop/close needs to be called here or get called within ip stop, most likely should be done here */

    BDBG_MSG(("%s: thread is exiting..., state %d", __FUNCTION__, playback_ip->playback_state));

    if (playback_ip->settings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip
        || playback_ip->settings.ipMode == B_PlaybackIpClockRecoveryMode_ePushWithPcrNoSyncSlip) {
        B_PlaybackIp_TtsThrottle_Stop(playback_ip->ttsThrottle);
        B_PlaybackIp_TtsThrottle_Close(playback_ip->ttsThrottle);
    }

out:
    if (playback_ip->openSettings.eventCallback &&
            playback_ip->playback_state != B_PlaybackIpState_eStopping &&
            playback_ip->playback_state != B_PlaybackIpState_eStopped)
    {
        B_PlaybackIpEventIds eventId;
        if (playback_ip->serverClosed)
            eventId = B_PlaybackIpEvent_eServerEndofStreamReached;
        else
            eventId = B_PlaybackIpEvent_eErrorDuringStreamPlayback;
        playback_ip->openSettings.eventCallback(playback_ip->openSettings.appCtx, eventId);
    }
    if (playback_ip->playback_halt_event)
        BKNI_SetEvent(playback_ip->playback_halt_event);
    return;
}

#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
void B_PlaybackIp_HlsPlaybackThread(void *data);
#endif

#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
void B_PlaybackIp_MpegDashPlaybackThread(void *data);
#endif

static B_PlaybackIpError validateAlternateAudioRenditionParams(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpHlsAltAudioRenditionInfo *pAltAudioRenditionInfo,
    NEXUS_PlaypumpHandle hPlaypump
    )
{
    B_PlaybackIpError errorCode = B_ERROR_INVALID_PARAMETER;
    if (pAltAudioRenditionInfo->pid == 0) {
        BDBG_ERR(("%s: alternateAudio.pid can't be 0 if alternate audio is enabled.", __FUNCTION__));
        BDBG_ASSERT(NULL);
        goto error;
    }
    if (pAltAudioRenditionInfo->containerType == NEXUS_TransportType_eUnknown) {
        BDBG_ERR(("%s: alternateAudio.containerType can't be unknown if alternate audio is enabled.", __FUNCTION__));
        goto error;
    }
    if (pAltAudioRenditionInfo->language == NULL ) {
        BDBG_ERR(("%s: alternateAudio.language can't be NULL if alternate audio is enabled.", __FUNCTION__));
        goto error;
    }
    if (pAltAudioRenditionInfo->groupId == NULL ) {
        BDBG_ERR(("%s: alternateAudio.groupId can't be NULL if alternate audio is enabled.", __FUNCTION__));
        goto error;
    }
    if ( hPlaypump == NULL) {
        BDBG_ERR(("%s: Playpump handle is NULL, it must be set for playing the alternate audio!", __FUNCTION__));
        goto error;
    }
    errorCode = B_ERROR_SUCCESS;
    BDBG_MSG(("%s: playback_ip=%p: Verified params", __FUNCTION__, (void *)playback_ip));
error:
    return (errorCode);
}

B_PlaybackIpError
B_PlaybackIp_HttpSessionStart(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionStartSettings *startSettings,
    B_PlaybackIpSessionStartStatus *startStatus /* [out] */
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;
    B_ThreadSettings settingsThread;
    char *threadName;
    BERR_Code rc;
    B_ThreadFunc httpThreadFuncName;

    if (!playback_ip || !startSettings || !startStatus) {
        BDBG_ERR(("%s: invalid params, playback_ip %p, startSettings %p, startStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)startSettings, (void *)startStatus));
        errorCode = B_ERROR_INVALID_PARAMETER;
        return errorCode;
    }

    if (playback_ip->startSettings.startAlternateAudio) {
        int i;

        errorCode = validateAlternateAudioRenditionParams(playback_ip, &playback_ip->startSettings.alternateAudio, playback_ip->startSettings.nexusHandles.playpump2 );
        if (errorCode != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: validateAlternateAudioRenditionParams Failed.", __FUNCTION__));
            goto error;
        }
        for (i=0; i < playback_ip->startSettings.additionalAltAudioRenditionInfoCount; i++)
        {
            errorCode = validateAlternateAudioRenditionParams(playback_ip, &playback_ip->startSettings.additionalAltAudioInfo[i], playback_ip->startSettings.additionalAltAudioInfo[i].hPlaypump );
            if (errorCode != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: validateAlternateAudioRenditionParams for additional altAudioRenditionInfo Failed.", __FUNCTION__));
                goto error;
            }
        }
    }
    playback_ip->speedNumerator = 1;
    playback_ip->speedDenominator = 1;
    playback_ip->lastPosition = startSettings->u.http.initialPlayPositionOffsetInMs;

    B_PlaybackIp_HttpSetDefaultTrickModeSettings(&playback_ip->ipTrickModeSettings);
    threadName = "PlaybackIpHttp";

#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
    if (playback_ip->hlsSessionEnabled) {
        httpThreadFuncName = B_PlaybackIp_HlsPlaybackThread;
    }
    else
#endif
#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
    if (playback_ip->mpegDashSessionEnabled) {
        httpThreadFuncName = B_PlaybackIp_MpegDashPlaybackThread;
        errorCode = B_PlaybackIp_MpegDashSessionStart(playback_ip, startSettings, startStatus);
        if (errorCode) goto error;
    }
    else
#endif
    if (playback_ip->useNexusPlaypump) {
        /* Feed media stream to Nexus Playpump, done when Nexus Playback can't keep track of stream position (e.g. in server side trickmodes case) */
        httpThreadFuncName = B_PlaybackIp_HttpPlaypumpThread;
    }
    else {
        /* by default for HTTP playback, we use Nexus Playback to pull media stream from IP library */
        httpThreadFuncName = B_PlaybackIp_HttpPlaybackThread;
    }

    /* reset this event as it may have been set during the psi media probe */
    BKNI_ResetEvent(playback_ip->playback_halt_event);

    /* create thread to process incoming IP packets & feed them to PB hw */
    B_Thread_GetDefaultSettings(&settingsThread);
    playback_ip->playbackIpThread = B_Thread_Create(threadName, httpThreadFuncName, (void *)playback_ip, &settingsThread);
    if (NULL == playback_ip->playbackIpThread) {
        BDBG_ERR(("%s: Failed to create the %s thread for HTTP media transport protocol\n", __FUNCTION__, threadName));
        goto error;
    }


    if (
#ifdef B_HAS_HLS_PROTOCOL_SUPPORT
        !playback_ip->hlsSessionEnabled &&
#endif
#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
        !playback_ip->mpegDashSessionEnabled &&
#endif
        playback_ip->settings.ipMode == B_PlaybackIpClockRecoveryMode_ePull &&
        playback_ip->preChargeBuffer) {

        /* wait until pre-charging has started */
        rc = BKNI_WaitForEvent(playback_ip->preChargeBufferEvent, IP_HALT_TASK_TIMEOUT_MSEC);
        if (rc == BERR_TIMEOUT) {
            BDBG_WRN(("%s: timed out for pre-charging start event", __FUNCTION__));
            return B_ERROR_UNKNOWN;
        } else if (rc!=0) {
            BDBG_WRN(("%s: got error while trying to wait for pre-charging start event, rc %d", __FUNCTION__, rc));
            return B_ERROR_UNKNOWN;
        }
        BDBG_MSG(("%s: Enabled pre-charging of network buffer", __FUNCTION__));
    }
    errorCode = B_ERROR_SUCCESS;

error:
    return errorCode;
}

void
B_PlaybackIp_HttpSessionStop(B_PlaybackIpHandle playback_ip)
{
    /* TODO: make this more deterministic by waiting on the playback_halt_event and making DataRead send this event when state is stopped */
    while (playback_ip->readCallBackInProgress) {
#ifdef BDBG_DEBUG_BUILD
        if (playback_ip->ipVerboseLog)
            BDBG_WRN(("read callback via nexus io thread is still active, %s() has to wait for its completion, for now sleeping for 100msec & retrying", __FUNCTION__));
#endif
        BKNI_Sleep(100);
        continue;
    }

#ifdef B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
    if (playback_ip->mpegDashSessionEnabled) {
        B_PlaybackIp_MpegDashSessionStop(playback_ip);
    }
#endif

    if (playback_ip->startSettings.monitorPsi && playback_ip->pPsiState) { B_PlaybackIp_DestroyPsiState(playback_ip->pPsiState); }
    /* force set the serverClosed flag, this allows next Start (if app chooses to Start w/o calling SessionClose) to re-open the socket. */
    playback_ip->serverClosed = true;
    if (playback_ip->newTrickModeJobEvent && playback_ip->trickModeThread) {
        /* send event to trickModeThread to wake up, it will then check the state and break out from the loop */
        BKNI_SetEvent(playback_ip->newTrickModeJobEvent);
        /* wait for a finite time for either trickModeThread to finish from currently going seek operation or wakeup from waiting on this event */
        BKNI_Sleep(20);
        if (!playback_ip->trickModeThreadDone)
            BDBG_WRN(("%s: trickModeThread is still not stopped, continuing w/ session stop", __FUNCTION__));
    }
}

#endif /* LINUX || VxWorks */
