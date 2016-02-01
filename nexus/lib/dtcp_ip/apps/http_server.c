/***************************************************************************
 *     (c)2007-2015 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: simple test app to stream a content file over
 *                     standard or accelerated sockets using HTTP transport
 *
 * Revision History:
 *
 * $brcm_Log: $
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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/syscall.h>

#include <assert.h>
#include <stdbool.h>
#include "nexus_dma.h"
#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_platform_client.h"
#else
#include "nexus_platform.h"
#endif

BDBG_MODULE(http_server);

#ifdef B_HAS_DTCP_IP
#include "b_dtcp_applib.h"
#endif

#define DEFAULT_CONTENT_DIR "/data/videos"
#define ALIGN_4096   (4096 - 1)
#define BUF_SIZE    (188*7*16)/*(1448)*/
#define NUM_BUF_DESC    24
#define FILENAME_MAX_LEN 256

char url[256];          /* content name to stream out */
char url_root[256];     /* Root directory where content is */
char recvbuf[1024];
char sendbuf[2048];
char response[2048];
off_t filesize;

void usage(void)
{
    printf("Usage: http_server [-p <port> -r <rate> -f <content-directory> -l -h\n");
    printf("options are:\n");
    printf(" -p <port>              # port to listen on (default: 5000)\n");
    printf(" -r <rate>              # rate (default: 20Mpbs)\n");
    printf(" -f <content-directory  # (default: /data/videos) \n");
    printf(" -l                     # keep resending the file (default: stream only once)\n");
    printf(" -v                     # print periodic stats (default: no)\n");
#ifdef B_HAS_DTCP_IP
    printf(" -s                     # Send file encrypted with DTCP/IP \n");
    printf(" -k                     # Which key type to use B_DTCP_KeyFormat_eTest = 0, B_DTCP_KeyFormat_eCommonDRM = 1(default) \n");
#endif
    printf(" -h                     # prints http_server usage\n");
    printf("\n");
}

double difftime1(struct timeval *start, struct timeval *stop)
{
    double dt = 1000000.*(stop->tv_sec - start->tv_sec) + (stop->tv_usec - start->tv_usec);
    return dt;
}


/* Open the listener service on port 5000 */
int tcpServerSetup(int port, int socket_type)
{
    struct sockaddr_in localAddr;
    int sd;
    int reuse_flag = 1;

    if ( (sd = socket(AF_INET, socket_type, 0)) < 0) {
        /* Socket Create Error */
        perror("Socket Open Err");
        return -EINVAL;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(port);

    if(setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_flag, sizeof(reuse_flag) ) < 0 ) {
        printf("REUSE Socket Error\n");
        return -EINVAL;
    }

    if (bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr))) {
        perror("Socket Bind Err");
        return -EINVAL;
    }

    if (listen(sd, 4)) {
        perror("Socket listen Err");
        return -EINVAL;
    }

    return sd;
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
        if(!p_end)
            return -1;
        temp = *p_end;
        *p_end = 0;
        printf("TokenFound = [%s]\n",p_begin);
        strncpy(output,p_begin, output_len-1);
        *p_end = temp;
        return 0;
    }
    return -1; /* Not found */
}

int openUrlFile(char *url, char *filename)
{
    struct stat file_stat;
    char *p;

    filename[0] = 0;
    snprintf(filename, FILENAME_MAX_LEN, "%s/%s", url_root, url);
    while(stat(filename, &file_stat)) {
        if ( (p = strstr(filename,".mpeg"))) {
            printf("Original %s, ",filename);
            strncpy(p,".mpg", FILENAME_MAX_LEN-6);
            printf("Trying [%s] ",filename);
            if(stat(filename, &file_stat)) {
                printf("Failed\n");
                return -1;
            }
            printf("OK\n");
            break;
        }
        if ( (p = strstr(filename,".mpg"))) {
            printf("Original %s, ",filename);
            strncpy(p,".mpeg", FILENAME_MAX_LEN-5);
            printf("Trying [%s] ",filename);
            if(stat(filename, &file_stat)) {
                printf("Failed\n");
                return -1;
            }
            printf("OK\n");
            break;
        }
        return 0;
    }

    /* File found */
    filesize = file_stat.st_size;
    return 0;
}


void waitForNetworkEvent(int sd)
{
    fd_set rfds;
    struct timeval tv;

    while (1) {
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

#define CMND_BUFF_SIZE 256
#define TEMP_BUF_SIZE 512


/* waits for incoming HTTP requests, sends out HTTP response and returns new socket descriptor */
int handleHttpRequest(int sd, char *filename, struct in_addr *remoteIp)
{
    int nsd = -1;
    struct sockaddr_in remoteAddr;
    char cmndBuf[CMND_BUFF_SIZE] = "md5sum ";
    char tempBuf[TEMP_BUF_SIZE];
    char *chkSum;
    FILE *ptr;

    int addrLen = sizeof(remoteAddr);
    int nbytes;

    *(unsigned long *)remoteIp = 0;
    while (1) {

        waitForNetworkEvent(sd);

        /* accept connection */
        if ( (nsd = accept(sd, (struct sockaddr *)&remoteAddr, (socklen_t *)&addrLen)) < 0 ) {
            perror("ERROR: accept(): exiting...");
            break;
        }
        printf("Accepted Connection from %lx:%d \n", (long int)remoteAddr.sin_addr.s_addr, (int)ntohs(remoteAddr.sin_port));

        *remoteIp = remoteAddr.sin_addr;
        waitForNetworkEvent(nsd);


        /** ProcessHttpRequestAndSendResponse()  **/
        /* Read HTTP request */
        if ((nbytes = read(nsd, recvbuf, 1024)) <= 0) {
            perror("read failed to read the HTTP Get request\n");
            continue;
        }
        recvbuf[nbytes] = 0;
        printf("Read HTTP Req ( %d bytes)\n------------------------\n%s------------------------\n", nbytes, recvbuf);

        /* Parse HTTP request & open the content file */
        parseToken(recvbuf, url, sizeof(url), "GET /", " ");
        if( openUrlFile(url, filename) ) {
            printf("File Not found, go back to listening\n");
            continue;
        }
        printf("Stream file = %s size=%lld\n", filename, filesize);

        strcat(cmndBuf,filename);

        printf("\nComputing check sum of the file ******************** %s \n", cmndBuf);

        if((ptr = popen(cmndBuf,"r")) != NULL)
        {
            while (fgets(tempBuf, TEMP_BUF_SIZE, ptr) != NULL)
                              (void) printf("%s\n", tempBuf);
            if(ptr != NULL)
                (void) pclose(ptr); /*tempBuf[TEMP_BUF_SIZE]    */

             chkSum = strtok(tempBuf," ");

             printf("checksum value is %s\n", chkSum);

            /* Build HTTP response */
            strncpy(response,
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: %lld\r\n"
                "Connection: Keep-Alive\r\n"
                "Accept-Ranges: bytes\r\n"
                "Connection: close\r\n"
                "Content-Range: bytes 0-%lld/%lld\r\n"
                "Server: Linux/2.6.18, UPnP/1.0, Broadcom UPnP SDK/1.0\r\n"
                "Checksum:%s\r\n"
                "\r\n",
                sizeof(response)-1);
            nbytes = snprintf(sendbuf, sizeof(sendbuf), response, filesize, filesize-1, filesize,chkSum);

            printf("HTTP Response \n------------------------\n%s------------------------\n", sendbuf);

            /* send out HTTP response */
            if (write(nsd, sendbuf, nbytes) != nbytes) {
                printf("Failed to write HTTP Response of %d bytes\n", nbytes);
                perror("write(): \n");
                break;
            }

            return nsd;
        }
        else
        {
            printf("\nChecksum computation failed popen Failed --------------------\n");
            /* Build HTTP response */
            strncpy(response,
                "HTTP/1.1 404 File Not Found\r\n"
                "\r\n",
                sizeof(response)-1);
            nbytes = snprintf(sendbuf, sizeof(sendbuf), response);

            printf("HTTP Response \n------------------------\n%s------------------------\n", sendbuf);

            /* send out HTTP response */
            if (write(nsd, sendbuf, nbytes) != nbytes) {
                printf("Failed to write HTTP Response of %d bytes\n", nbytes);
                perror("write(): \n");
                break;
            }

            return -1;
        }
    }
    return -1;
}

int main(int argc, char *argv[])
{
    int port = 5000;        /* Server Port */
    double rate = 19.4;     /* Default rate to stream content at */
    int loop = 0;
    struct timeval start, stop;
    int verbose = 0;
    int sd;                 /* socket descriptor */
    unsigned long count = 0;
    unsigned long total = 0;
    int buflen = BUF_SIZE;
    int c, i;
    int fd = -1;
    unsigned char *xbuf, *buf;
    loff_t bytes= 0;
    double dt=0, maxrate = 19.4;
    struct iovec iov[NUM_BUF_DESC];
    int nextBuf;
    int socket_type;
    char filename[FILENAME_MAX_LEN];
    struct in_addr remoteIp;

    int wrbuf_len;
    unsigned char *wrbuf;

#ifdef B_HAS_DTCP_IP

    int key;
    bool bDtcpEnable = false;
    void * akeHandle = NULL;
    void * dtcpCtx = NULL;
    void * streamHandle = NULL;
    B_DTCP_KeyFormat_T keyFormat = B_DTCP_KeyFormat_eCommonDRM; /* Set this flag based on -K option */

    unsigned int enc_buf_len;
    unsigned char *enc_buf = NULL;

    unsigned int clr_buf_len;
    unsigned char *clr_buf = NULL;

    unsigned int residue = 0;
    unsigned int total_size;
    int max_pkt_size = 8*1024*1024;
    B_UR_Mode_T cci = B_UR_Mode_eNoCCI;

#if B_DTCP_IP_HW_ENCRYPTION && (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA)
    NEXUS_DmaHandle dmaHandle = NULL;

#if NXCLIENT_SUPPORT
    NxClient_JoinSettings joinSettings;
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    NxClient_Join(&joinSettings);
#else
    NEXUS_PlatformSettings platformSettings;
    /* Initialize nexus before opening DMA channel */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    NEXUS_Platform_Init(&platformSettings);
#endif
#endif
#endif

    url[0] = '\0';
    url_root[0] = '\0';
    while ((c = getopt(argc, argv, "ap:c:f:r:n:lmk:vsh")) != -1)
    {
        switch (c)
        {

        case 'p':
            port = atoi(optarg);
            break;
        case 'f':
            strncpy(url_root, optarg, sizeof(url_root)-1);
            break;
        case 'r':
            maxrate = atof(optarg);
            break;
        case 'l':
            loop = 1;
            break;
        case 'v':
            verbose = 1;
            break;
#ifdef B_HAS_DTCP_IP
        case 's':
            bDtcpEnable = true;
            break;
        case 'k':
            key = atoi(optarg);
            if (key == 0) {
                keyFormat = B_DTCP_KeyFormat_eTest;
                BDBG_WRN(("********************** If you are using test key's then please make sure you compile your app with DTCP_IP_DO_NOT_CLEAN_HCI=y  *********************"));
            }
            else if (key == 1) {
                keyFormat = B_DTCP_KeyFormat_eCommonDRM;
            }
            else
            {
                BDBG_ERR(("Wrong KeyType %d", key));
                usage();
                return -1;
            }
            break;
#endif
        case 'h':
        default:
            usage();
            return -1;
        }
    }

    /* print usage anyway */
    usage();


#ifdef B_HAS_DTCP_IP
    if (bDtcpEnable)
    {
#if B_DTCP_IP_HW_ENCRYPTION && (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA)
        /* Open Dma channel to initialize DTCP HW encryption */
        if ((dmaHandle = NEXUS_Dma_Open(0, NULL)) == NULL) {
            BDBG_ERR(("ERROR: NEXUS_Dma_Open failed"));
            return (-1);
        }
#endif
        /* DTCP/IP Init source & DMA */
#if B_DTCP_IP_HW_ENCRYPTION && (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA)
        if (BERR_SUCCESS != DtcpInitHWSecurityParams(dmaHandle))
        {
            BDBG_WRN(("Error enabling hardware encryption. Quitting!! "));
            BDBG_WRN(("Please make sure you have a valid drm.bin present in the same location as executable."));
            return -1;
        }
        BDBG_LOG (("DTCP HW Init complete\n"));
#endif

        dtcpCtx = DtcpAppLib_Startup(B_DeviceMode_eSource, false, keyFormat, true);
        if (dtcpCtx == NULL)
        {
            BDBG_ERR (("DTCP Startup failed !!!\n"));
            goto dtcp_shutdown;
        }
        BDBG_LOG (("DTCP Startup complete\n"));
    }
#endif

    if (url_root[0] == '\0')
        strncpy(url_root, DEFAULT_CONTENT_DIR, sizeof(url_root)-1);

    printf("http_server: port %d, content directory %s, rate %f, loop around %d, verbose %d \n",
            port,
            url_root, maxrate, loop, verbose);



    /* allocate FIFO where data is read from disk & then sent out on the network */
    for (i=0; i<NUM_BUF_DESC; i++) {
        if ( (xbuf = malloc(BUF_SIZE + ALIGN_4096)) == NULL ) {
            perror("posix_memalign failure():");
            goto exit;
        }
        iov[i].iov_base = (unsigned char *)(((unsigned long)xbuf + ALIGN_4096) & ~ALIGN_4096);/**ArnabCheck: is this required **/
        iov[i].iov_len = BUF_SIZE;
    }
    printf("Allocated %d-bytes for buffering, # of DESC=%d\n",NUM_BUF_DESC*(BUF_SIZE + ALIGN_4096),NUM_BUF_DESC);

#ifdef B_HAS_DTCP_IP
    /* Setup DTCP/IP Listener on port+1 */
    if (bDtcpEnable)
    {
        if (BERR_SUCCESS != DtcpAppLib_Listen(dtcpCtx, NULL, port+1))
        {
            BDBG_ERR(("DTCP Listen failed"));
            goto dtcp_shutdown;
        }
        BDBG_LOG (("Listening for DTCP IP clients to initiate AKE\n"));
    }
#endif

    /* setup HTTP Server */
    socket_type = SOCK_STREAM;
    if ( (sd = tcpServerSetup(port, socket_type)) < 0) {
        printf("Failed to Setup TCP Server, Exiting...\n");
        exit(-1);
    }

    /* wait for HTTP request & send HTTP reply */
    sd = handleHttpRequest(sd, filename, &remoteIp);

    if (sd<0) {
        printf("Failed handling HttpRequest, Exiting...\n");
        goto exit;
    }


    /* get file ready for streaming */
    if ((fd = open(filename, O_RDONLY)) <= 0) {
            perror("open():");
            goto exit;
    }
    printf("Content File opened\n");

#ifdef B_HAS_DTCP_IP
    if (bDtcpEnable)
    {
        /* Get the AKE handle */
        char *ip = inet_ntoa(remoteIp);
        DtcpAppLib_GetSinkAkeSession(dtcpCtx, ip, &akeHandle);

        if (akeHandle == NULL)
        {
            BDBG_ERR(("Error getting Sink AKE session handle\n"));
            goto dtcp_shutdown;
        }

        /* Open Source DTCP/IP Stream */
        streamHandle = DtcpAppLib_OpenSourceStream(akeHandle,
                                                   B_StreamTransport_eHttp,
                                                   DTCP_CONTENT_LENGTH_UNLIMITED,
                                                   cci,
                                                   B_Content_eAudioVisual,
                                                   max_pkt_size);
        if (streamHandle == NULL)
        {
            BDBG_ERR(("ERROR: Failed to open stream for AKE handle %p\n",  akeHandle));
            goto dtcp_shutdown;
        }
        BDBG_LOG (("Opened source stream for client : %s\n", ip));
    }
#endif

#ifdef B_HAS_DTCP_IP
    /* Allocate input/output buffers used for packetizing data */
    if (bDtcpEnable)
    {
        NEXUS_MemoryAllocationSettings allocSettings;
        NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);

        enc_buf_len = 2*BUF_SIZE;
        if (NEXUS_Memory_Allocate(enc_buf_len, &allocSettings, (void *)(&enc_buf))) {
            BDBG_ERR(("%s: memory allocation failure for encrypted buffer\n", __FUNCTION__));
            goto dtcp_shutdown;
        }

        if (NEXUS_Memory_Allocate(enc_buf_len, &allocSettings, (void *)(&clr_buf))) {
            BDBG_ERR(("%s: memory allocation failure for clear buffer\n", __FUNCTION__));
            NEXUS_Memory_Free(enc_buf);
            goto dtcp_shutdown;
        }
    }
#endif

    gettimeofday(&start, NULL);
    nextBuf = 0;
    while(1) {


        /* get next buffer to read into */
        buf = (unsigned char *)iov[nextBuf++].iov_base;
        nextBuf %= NUM_BUF_DESC;


        /* read next data chunk */
        if ( (buflen  = read(fd, buf, BUF_SIZE)) < 0 ) {
            perror("read():");
            goto exit;
        }
        else if (buflen == 0) {
            printf("**** Reached EOF *******\n");
            gettimeofday(&stop, NULL);
            if (loop) {
                lseek(fd, 0, SEEK_SET);
                continue;
            }
#ifdef B_HAS_DTCP_IP
            /* Send the residue data */
            if (bDtcpEnable)
            {
                /* DTCP/IP Packetize data */
                if (residue)
                {
                    /* Round up the residue to a multiple of 16 */
                    residue += 0xF;
                    residue &= ~0xF;
                    BDBG_ERR(("residue = %d\n", residue));
                    enc_buf_len = 2*BUF_SIZE;
                    DtcpAppLib_StreamPacketizeData(streamHandle, akeHandle, &clr_buf[0], residue, &enc_buf, &enc_buf_len, &total_size);
                    /* We expect the residue bytes to be encrypted completely */
                    BDBG_ASSERT(total_size == residue);

                    if ( (bytes = write(sd, enc_buf, residue)) != residue) {
                        printf("Write failed to write residue %10u bytes, instead wrote %d bytes\n", residue, (int)bytes);
                        perror("write():");
                        goto exit;
                    }
                }
            }
#endif
            break;
        }

#ifdef B_HAS_DTCP_IP
        if (bDtcpEnable)
        {
            /* DTCP/IP Packetize data */
            enc_buf_len = 2*BUF_SIZE;
            clr_buf_len = residue + buflen;
            /* Ensure that the bytes to encrypt is less than the pkt buffer size */
            BDBG_ASSERT(clr_buf_len < enc_buf_len);

            /* Copy the bytes to encode to packet buffer at offset = residue */
            memcpy(&clr_buf[residue], &buf[0], buflen);
            DtcpAppLib_StreamPacketizeData(streamHandle, akeHandle, &clr_buf[0], clr_buf_len, &enc_buf, &enc_buf_len, &total_size);

            /* Use encrypted buffer for sending data */
            wrbuf = enc_buf;
            wrbuf_len = enc_buf_len;

            /* Re-compute residue */
            residue = clr_buf_len - total_size;
            /* Copy the residual bytes to the start of the clear buffer */
            if (residue)
            {
                memcpy(&clr_buf[0], &clr_buf[total_size], residue);
            }
        }
        else
#endif
        {
            wrbuf = buf;
            wrbuf_len = buflen;
        }

        /* send the data */
        if ( (bytes = write(sd, wrbuf, wrbuf_len)) != wrbuf_len) {
            printf("Write failed to write %10u bytes, instead wrote %d bytes\n", wrbuf_len, (int)bytes);
            perror("write():");
            goto exit;
        }

        /* pacing logic */
        count++;
        total += bytes;
        gettimeofday(&stop, NULL);
        dt = difftime1(&start, &stop);
        rate = 8.*total/dt;
        while(rate > maxrate) {
            usleep(10000);
            gettimeofday(&stop, NULL);
            dt = difftime1(&start,&stop);
            rate = 8.*total/dt;
        }

        if(verbose && (count % 100) == 0) {
            printf("[%6lu] Wrote %10lu Bytes in dt = %12.2fusec at rate=%2.1f/%2.1f\n",
                    count, total, dt, rate, maxrate);
        }
    }

exit:
#if 0
    for (i=0; i<NUM_BUF_DESC; i++)
        free(iov[i].iov_base);
#endif
    printf("Final stats: Streamed %lu Bytes of %s file in dt = %10.2fusec at rate=%2.1f/%2.1f\n",
            total, filename, dt, rate, maxrate);

#ifdef B_HAS_DTCP_IP
dtcp_shutdown:
    /* Free all allocated buffers for encryption */
    if (bDtcpEnable)
    {
        if (enc_buf)
        {
            NEXUS_Memory_Free(enc_buf);
        }
        if (clr_buf)
        {
            NEXUS_Memory_Free(clr_buf);
        }
    }

    /* Shutdown DTCP IP Lib */
    if (bDtcpEnable)
    {
        if (streamHandle)
            DtcpAppLib_CloseStream(streamHandle);

        if (dtcpCtx)
        {
            DtcpAppLib_CancelListen(dtcpCtx);
            DtcpAppLib_Shutdown(dtcpCtx);
        }
#if B_DTCP_IP_HW_ENCRYPTION && (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA)
        DtcpCleanupHwSecurityParams();
        if (dmaHandle)
            NEXUS_Dma_Close(dmaHandle);
#endif
    }
#endif

    printf("\nNumber of packets sent %d \n", (int)count);

    /* needed for all data to get drained from the socket */
    sleep(2);
#if NXCLIENT_SUPPORT
    NxClient_Uninit();
#else /* !NXCLIENT_SUPPORT */
    NEXUS_Platform_Uninit();
#endif /* NXCLIENT_SUPPORT */

    return 0;
}
