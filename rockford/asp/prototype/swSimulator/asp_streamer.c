/***************************************************************************
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
 *************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "bip_server.h"
#include "tcp_connx_migration.h"
#include "bip_server_priv.h"
#include "nexus_asp.h"

#define DEFAULT_CONTENT_DIR "/data/videos/"
#define FILENAME_MAX_LEN 256
typedef long long int myOff_t;
char url_root[256];     /* Root directory where content is */

void usage(void)
{
    printf("Usage: aspStreamer [-p <listenerPort> -f <content-directory> -l -h]\n");
    printf("options are:\n");
    printf(" -p <listenerPort>      # port to listen on (default: 80)\n");
    printf(" -f <content-directory  # (default: /data/videos) \n");
    printf(" -l                     # rewind at EOF\n");
    printf(" -h                     # prints usage\n");
    printf("\n");
}

/* Parses the input for pattern begin; followed by pattern end */
int parseToken(char *input, char *output, int output_len, char *begin, char *end)
{
    char *p_begin = strstr(input, begin);
    char *p_end;
    char temp;

    if (p_begin)
    {
        p_begin += strlen(begin);
        p_end = strstr(p_begin,end);
        if (!p_end)
            return -1;
        temp = *p_end;
        *p_end = 0;
        //printf("TokenFound = [%s]\n",p_begin);
        strncpy(output,p_begin, output_len-1);
        *p_end = temp;
        return 0;
    }
    return -1; /* Not found */
}

int openUrlFile(char *url, char *filename, myOff_t *fileSize)
{
    int rc;
    struct stat file_stat;

    filename[0] = 0;
    snprintf(filename, FILENAME_MAX_LEN, "%s%s", url_root, url);
    DBG("file name = %s\n", filename);
    rc = access(filename, R_OK);
    CHECK_ERR_NZ_GOTO("access() Failed", rc, error);
    rc = stat(filename, &file_stat);
    CHECK_ERR_NZ_GOTO("stat() Failed", rc, error);

    /* File found */
    *fileSize = file_stat.st_size;
    DBG2("file size %lld, size %lld\n", *fileSize, file_stat.st_size);
    if (file_stat.st_size == 0) {
        printf("file size %lld, size %lld is 0\n", *fileSize, file_stat.st_size);
        return -1;
    }
    return 0;
error:
    return -1;
}

void
prepareHttpErrorResponse(char *httpRespBuf, int httpRespBufSize, int httpErrCode, char *httpErrString, int *responseLen)
{
    memset(httpRespBuf, 0, httpRespBufSize);
    *responseLen = snprintf(httpRespBuf, httpRespBufSize-1,
            "HTTP/1.1 %d %s\r\n"
            "Connection: close\r\n"
            "\r\n",
            httpErrCode, httpErrString
            );
}

void
prepareHttpResponse(char *httpRespBuf, int httpRespBufSize, myOff_t fileSize, int *responseLen)
{
    memset(httpRespBuf, 0, httpRespBufSize);
    *responseLen = snprintf(httpRespBuf, httpRespBufSize-1,
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: %lld\r\n"
            "Content-Type: video/mpeg\r\n"
            "Accept-Ranges: bytes\r\n"
            "Connection: close\r\n"
            "Content-Range: bytes 0-%lld/%lld\r\n"
            "Server: \r\n"
            "\r\n",
            fileSize, fileSize-1, fileSize
            );
}

void newConnectionCallback(BipServerHandle serverCtx)
{
    int rc;
    char *httpReqBuf;
    size_t httpReqBufLen;
    char url[1024];
    char httpRespBuf[2048];
    int httpRespLen;
    char fileName[FILENAME_MAX_LEN];
    myOff_t fileSize;
    printf("%s: processed a new connection request from client, serverCtx %p\n", __FUNCTION__, (void *)serverCtx);

    /* This code is driven by the callback from BIP Server listener thread */
    /* This callback will be invoked when client sends a new TCP request */
    /* BIP Server's Listening thread will receive this request, start a new child thread */
    /* and invoke this callback. The remaining media streaming pipe setup happens in that context */
    /* And control remains in this thread until connection is torn down by either end */

    /* Read HTTP Get Request */
    rc = BIP_Server_RecvHttpRequest(serverCtx, &httpReqBuf, &httpReqBufLen);
    CHECK_ERR_NZ_GOTO("BIP_Server_RecvHttpRequest() Failed", rc, error);
    printf("Initial Req (of %d bytes)[\n%s]\n", httpReqBufLen, httpReqBuf);

    /* Parse HTTP request & open the content file */
    rc = parseToken(httpReqBuf, url, sizeof(url), "GET /", " ");
    CHECK_ERR_NZ_GOTO("Failed to find the URL token", rc, error);
    rc = openUrlFile(url, fileName, &fileSize);
    CHECK_ERR_NZ_GOTO("Failed to open the file", rc, error);

    /* app determines if it can support the requested URL (file or live channel), */
    /* sets up the nexus pipe for either case (more work required in the live case), */
    /* then prepares the initial Http response and sends it over to the client */

    /* app makes a BIP Server API call to determine if session can be offloaded to ASP or not */
    /* Open a ASP Channel */
    rc = NEXUS_Asp_Open();
    CHECK_ERR_NZ_GOTO("NEXUS_Asp_Open failed...", rc, error);

    /* TODO: Call BIP_Server_SetSettings() to pass the needed nexus handles to BIP */
    /* this includes: ASP Channel, Rave Channel, other xpt needed resources */

    /* Build HTTP response and setup the pipe (frontend, file, etc.) */
    prepareHttpResponse(httpRespBuf, sizeof(httpRespBuf), fileSize, &httpRespLen);
    DBG("HTTP response: [\n%s]\n", httpRespBuf);
    rc = BIP_Server_SendHttpResponse(serverCtx, httpRespBuf, httpRespLen);
    CHECK_ERR_NZ_GOTO("BIP_Server_SendHttpResponse() Failed", rc, error);

    /* received a valid HTTP request, start Connection Offload to ASP and include the HTTP response along */
    rc = BIP_Server_Start(serverCtx, fileName);
    CHECK_ERR_NZ_GOTO("BIP_ServerStart() Failed ", rc, error);

#if 0
    rc = BIP_Server_StartStreaming(serverCtx, fileName);
    CHECK_ERR_NZ_GOTO("BIP_ServerStreaming() Failed ", rc, error);
#endif

    /* control only returns from BIP_Server_Start() when either: */
    /* -we have reached EOF */
    /* -connection is closed because client did a channel change */
    /* -or, there is an error while streaming out */
    printf("%s: Done: Proceed with Session Stop and Tear-down:...\n", __FUNCTION__);

    /* done streaming, so stop this session */
    rc = BIP_Server_Stop(serverCtx);
    CHECK_ERR_NZ_GOTO("BIP_ServerStop() Failed ", rc, error);

    /* Close ASP or any other resources */
    NEXUS_Asp_Close();
    return;

error:
    /* send a HTTP error back to the client */
    prepareHttpErrorResponse(httpRespBuf, sizeof(httpRespBuf), 400, "Bad Request", &httpRespLen);
    printf("HTTP response: [\n%s]\n", httpRespBuf);
    BIP_Server_SendHttpResponse(serverCtx, httpRespBuf, httpRespLen);
    return;
}

int main(int argc, char *argv[])
{
    int c;
    int listenerPort = 80;        /* Server Port */
    BIP_ServerListenerSettings listenerSettings;
    ListenerStateHandle listenerHandle;

    url_root[0] = '\0';
    while ((c = getopt(argc, argv, "p:f:h")) != -1) {
        switch (c) {
            case 'p':
                listenerPort = atoi(optarg);
                break;
            case 'f':
                strncpy(url_root, optarg, sizeof(url_root)-1);
                break;
            case 'h':
            default:
                usage();
                return -1;
        }
    }

    if (url_root[0] == '\0')
        strncpy(url_root, DEFAULT_CONTENT_DIR, sizeof(url_root)-1);

    /* Following code is typically run in the initialization sequence of a media server */
    BIP_Server_GetDefaultListenerSettings(&listenerSettings);
    listenerSettings.port = listenerPort;
    listenerSettings.newConnectionCallback = newConnectionCallback;
    listenerHandle = BIP_Server_CreateTcpListener(&listenerSettings);
    CHECK_PTR_GOTO("BIPServer Create Failed...", listenerHandle, exit);

    /* initialization done, now app continues with its other business */
    /* later when client sends a new request, that work will happen in the newConnectionCallback above */
    DBG("%s: main app thread continues with its business...........\n", __FUNCTION__);
    pause();

exit:
    if (listenerHandle)
        BIP_Server_DestroyTcpListener(listenerHandle);
    printf("main done...\n");
    return 0;
}
