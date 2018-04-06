/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *  Sample Code for sending HTTP Get Request and writing received data to disk.
 *
 *
 *************************************************************/
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

#include <assert.h>
#include <stdbool.h>
#include "nexus_dma.h"

#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_platform_client.h"
#else
#include "nexus_platform.h"
#endif

BDBG_MODULE(http_client);

#ifdef B_HAS_DTCP_IP
#include "b_dtcp_applib.h"
#endif


#define SERVER_PORT 5000
#define BUF_MASK    0x1ff
#define BUF_SIZE    (188*7*64)

void usage(void)
{
    printf("Usage: http_client: -d <srv-ip> -p <srv-port> -f <content-file> [-h -a -v]\n");
    printf("options are:\n");
    printf(" -d <srv-ip>   # IP address of HTTP Server (e.g. 192.168.1.110)\n");
    printf(" -p <srv-port> # Port of HTTP Server (e.g. 5000)\n");
    printf(" -f <file>     # url of stream; /data/videos prepended (e.g. portugal.mpg) \n");
    printf(" -v            # print periodic stats (default: no)\n");
#ifdef B_HAS_DTCP_IP
    printf(" -s            # Receive file encrypted with DTCP/IP \n");
    printf(" -k            # Which key type to use B_DTCP_KeyFormat_eTest = 0, B_DTCP_KeyFormat_eCommonDRM = 1(default) \n");
#endif
    printf(" -h            # prints http_client usage\n");
    printf("\n");
}

double difftime1(struct timeval *start, struct timeval *stop)
{
    double dt = 1000000.*(stop->tv_sec - start->tv_sec) + (stop->tv_usec - start->tv_usec);
    return dt;
}

/* This function creates a TCP connection to server and returns the socket descriptor */
int TcpConnect(char *server, int port )
{
    int sd,rc;

    struct sockaddr_in localAddr;
    char portString[16];
    struct addrinfo hints;
    struct addrinfo *addrInfo = NULL;
    struct addrinfo *addrInfoNode = NULL;

    printf("TCP - Connection to %s:%d ...\n",server, port);



    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* we dont know the family */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    memset(portString, 0, sizeof(portString));  /* getaddrinfo() requires port # in the string form */
    snprintf(portString, sizeof(portString)-1, "%d", port);

    if (getaddrinfo(server, portString, &hints, &addrInfo) != 0) {
        printf("\nERROR: getaddrinfo failed for server:port: %s:%d\n", server, port);
        perror("getaddrinfo");
        return -EINVAL;
    }

    for (addrInfoNode = addrInfo; addrInfoNode != NULL; addrInfoNode = addrInfo->ai_next) {
        if (addrInfoNode->ai_family == AF_INET)
            break;
    }
    if (!addrInfoNode) {
        perror("%s: ERROR: no IPv4 address available for this server, no support for IPv6 yet");
        return -EINVAL;
    }

    addrInfo = addrInfoNode;


    /* Open Socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd<0) {
        perror("Socket Open Err");
        return -EINVAL;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(0);

    if (bind(sd,(struct sockaddr *) &localAddr, sizeof(localAddr))) {
        perror("Socket Bind Err");
        return -EINVAL;
    }

    rc = connect(sd, addrInfo->ai_addr, addrInfo->ai_addrlen);

    if (rc<0) {
        perror("Connect Error: Server busy or unavailable:");
        return -EBUSY;
    }

    printf("TCP: Connection Success\n");
    return sd;
}

#define PKTS_PER_READ 64
#define MAX_BUFFER_SIZE (1316 * PKTS_PER_READ)
#define HTTP_POST_SIZE 2048
#define CMND_BUFF_SIZE 256
#define TEMP_BUF_SIZE 512

int main(int argc, char **argv)
{
    char *post;
    char *server = NULL;
    char *fname = NULL;
    int maxrate = 19;
    int c;
    int sd;
    FILE *fp = NULL;
    char *p;
    char *rbuf;
    char *chkSum1 = NULL;
    char *respCode = NULL;
    int status;
    char *content_length;
    char cmndBuf[CMND_BUFF_SIZE] = "md5sum ";
    char tempBuf[TEMP_BUF_SIZE];
    char *chkSum;
    int  port = 0;
    ssize_t n;
    unsigned char *buf, *xbuf;
    ssize_t bytes = 0;
    unsigned long total=0;
    unsigned long count=0;
    unsigned long filesize;
    char tok;
    ssize_t len;
    ssize_t offset;
    double dt;
    struct timeval start, stop;
    int verbose = 0;

    int wrbuf_len;
    unsigned char *wrbuf;

#ifdef B_HAS_DTCP_IP
    int key;
    B_DTCP_KeyFormat_T keyFormat = B_DTCP_KeyFormat_eCommonDRM; /* Set this flag based on -K option */
    bool bDtcpEnable = false;
    void * akeHandle = NULL;
    void * dtcpCtx = NULL;
    void * streamHandle = NULL;

    unsigned int enc_buf_len;
    unsigned char *enc_buf = NULL;

    unsigned int clr_buf_len;
    unsigned char *clr_buf = NULL;

    unsigned int total_bytes_written = 0;
    unsigned int residue = 0;
    unsigned int total_size;
    bool bPcpFound;

#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA)
    NEXUS_DmaHandle dmaHandle = NULL;

#if NXCLIENT_SUPPORT
    NxClient_JoinSettings joinSettings;
    NxClient_GetDefaultJoinSettings(&joinSettings);
    NxClient_Join(&joinSettings);
#else
    NEXUS_PlatformSettings platformSettings;
    /* Initialize nexus before opening DMA channel */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);

    /* Due to latest SAGE restrictions EXPORT_HEAP needs to be initialized even if we are not using SVP/EXPORT_HEAP(XRR).
       It could be any small size heap.
       Configure export heap since it's not allocated by nexus by default */
    platformSettings.heap[NEXUS_EXPORT_HEAP].size = 16*1024*1024;
    NEXUS_Platform_Init(&platformSettings);
#endif
#endif
#endif

    int readFailCount = 0;
    FILE *ptr;


    while ((c = getopt(argc, argv, "d:p:f:amvshk:")) != -1) {
        switch (c) {
        case 'd':
            server = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'f':
            fname = optarg;
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

    if (port == 0 || server == NULL || fname == NULL) {
        printf("Missing Args...\n");
        usage();
        exit(1);
    }

    printf("Server %s, Port %d, File %s, Maxrate %d\n",
            server, port, fname, maxrate);

    xbuf = (unsigned char *) malloc(BUF_SIZE + BUF_MASK);
    rbuf = (char *) malloc(1024);
    post = (char *) malloc(HTTP_POST_SIZE);

    if (xbuf == NULL || rbuf == NULL || post == NULL) {
        perror("malloc failure\n");
        exit(-1);
    }

#ifdef B_HAS_DTCP_IP
    if (bDtcpEnable)
    {
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA)
        /* Open Dma channel to initialize DTCP HW encryption */
        if ((dmaHandle = NEXUS_Dma_Open(0, NULL)) == NULL) {
            BDBG_ERR(("ERROR: NEXUS_Dma_Open failed"));
            return (-1);
        }
#endif
        /* DTCP/IP Init source & DMA */
        if (BERR_SUCCESS != DtcpInitHWSecurityParams(dmaHandle))
        {
            BDBG_WRN(("Error enabling hardware encryption"));
        }
        BDBG_LOG (("DTCP HW Init complete\n"));
        dtcpCtx = DtcpAppLib_Startup(B_DeviceMode_eSink, false, keyFormat, true);
        if (dtcpCtx == NULL)
        {
            BDBG_ERR (("DTCP Startup failed !!!\n"));
            goto dtcp_shutdown;
        }
        BDBG_LOG (("DTCP Startup complete\n"));
    }
#endif

    buf = (unsigned char *) (((unsigned long)xbuf + BUF_MASK) & ~BUF_MASK);

    /* Build HTTP Get Request */
    n = snprintf(post, HTTP_POST_SIZE,
            "GET /%s HTTP/1.1\r\n"
            "Host: %s:%d\r\n"
            "Rate: %d\r\n"
            "PlaySpeed.dlna.org: speed=1\r\n"
            "User-Agent: Test App\r\n"
            "\r\n",
            fname, server, port, maxrate);
    printf("Sending HTTP Request: \n------------------------\n%s------------------------\n", post);

    /* open file for writing */
    fp = fopen(fname,"wb");
    if (!fp) {
        perror("File open error:\n");
        exit(-1);
    }

#ifdef B_HAS_DTCP_IP
    if (bDtcpEnable)
    {
        /* Do the AKE Setup here */
        if (BERR_SUCCESS != DtcpAppLib_DoAke(dtcpCtx, server, port + 1, &akeHandle))
        {
            BDBG_ERR(("DTCP AKE failed!!!"));
            goto dtcp_shutdown;
        }

        BDBG_LOG (("DTCP AKE Successful\n"));

        /* Open Sink Stream */
        streamHandle = DtcpAppLib_OpenSinkStream(akeHandle, B_StreamTransport_eHttp);
        if (streamHandle == NULL)
        {
            BDBG_ERR(("ERROR: Failed to open stream for AKE handle %p\n",  akeHandle));
            goto dtcp_shutdown;
        }
        BDBG_LOG (("Opened sink stream Successful\n"));
    }
#endif


#ifdef B_HAS_DTCP_IP
    /* Allocate input/output buffers used for de-packetizing data */
    if (bDtcpEnable)
    {
        NEXUS_MemoryAllocationSettings allocSettings;
        NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);

        enc_buf_len = 2*BUF_SIZE;
        if (NEXUS_Memory_Allocate(enc_buf_len, &allocSettings, (void *)(&enc_buf))) {
            BDBG_ERR(("%s: memory allocation failure for encrypted buffer\n", BSTD_FUNCTION));
            goto dtcp_shutdown;
        }

        if (NEXUS_Memory_Allocate(enc_buf_len, &allocSettings, (void *)(&clr_buf))) {
            BDBG_ERR(("%s: memory allocation failure for clear buffer\n", BSTD_FUNCTION));
            NEXUS_Memory_Free(enc_buf);
            goto dtcp_shutdown;
        }
    }
#endif

    /* Connect to Server */
    sd = TcpConnect(server, port);
    if (sd < 0) {
            printf("Connection to Server Failed, Exiting...\n");
            exit(-1);
    }
    /* Send HTTP Get Request */
    n = write(sd, post, strlen(post));
    if ((unsigned)n != strlen(post)) {
        printf("Failed to write HTTP Get Request: rc %d\n", n);
        perror("write(): ");
        exit(-1);
    }

    printf("Succesfully Send HTTP Get Request to to %s:%d\n", server, port);
    usleep(10000);

    /* Read HTTP Response */
    memset(rbuf, 0, 1024);
    if ( (n = read(sd, rbuf, 1024)) <= 0) {
        printf("Failed to read HTTP Response: rc = %d\n", n);
        perror("read(): ");
        exit(-1);
    }

    rbuf[n] = '\0';

    /* Scan for end of HTTP header */
    p = strstr(rbuf,"\r\n\r\n");
    if(!p) {
        printf("No HTTP Header\n");
        len = 0;
        p = rbuf;
        tok = 0;
    }
    else {
        p+=4;
        tok = *p;
        *p = 0;
        len = strlen(rbuf);
    }

    printf("Total response len %d, payload len %d\n", n, n-len);
    printf("HTTP Response: \n------------------------\n%s------------------------\n", rbuf);
    *p = tok;

     /**    Extracting HTTP Response code       **/
    offset = sizeof("HTTP/1.1") - 1;
    respCode = strstr(rbuf,"HTTP/1.1");
    respCode = strndup(respCode+ offset,6);
    sscanf(respCode, "%d", &status);
    if (status != 200)
    {
        printf ("HTTP request unsuccessul : Status Code (%d)\n", status);
        goto http_client_exit;
    }

     /**    Extracting Checksum from response header        **/
    offset = sizeof("Checksum:") - 1;
    chkSum1 = strstr(rbuf,"Checksum:");
    chkSum1 = strndup(chkSum1+ offset,32);
    printf("Received checksum value is %s\n", chkSum1);

     /**    Extracting filesize from response header        **/
    offset = sizeof("Content-Length:") - 1;
    content_length = strstr(rbuf,"Content-Length:");
    content_length = strndup(content_length+ offset,32);
    sscanf(content_length, "%lu", &filesize);
    printf("Received file size is %lu\n", filesize);

    /* write any data that was read part of the initial read */
    if (n>len) {
#ifdef B_HAS_DTCP_IP
        /* Decrypt initial data */
        if (bDtcpEnable)
        {
            enc_buf_len = n - len;
            clr_buf_len = 2*BUF_SIZE;
            /* Copy the residual bytes to the start of the clear buffer */
            memcpy (&enc_buf[0], p, enc_buf_len);
            BDBG_ERR(("Length = %d\n", n - len));
            DtcpAppLib_StreamDepacketizeData(streamHandle, akeHandle, &enc_buf[0], enc_buf_len, clr_buf, &clr_buf_len, &total_size, &bPcpFound);

            /* We expect all the data to be decrypted here */
            BDBG_ASSERT(total_size == enc_buf_len);

            /* Make sure the padding encrypted data sent by the server is discarded */
            if (clr_buf_len > filesize)
            {
                clr_buf_len = filesize;
            }
            wrbuf = clr_buf;
            wrbuf_len = clr_buf_len;
            total_bytes_written += clr_buf_len;
        }
        else
#endif
        {
            wrbuf = (unsigned char *)p;
            wrbuf_len = n - len;
        }
        if ( (total = write(fileno(fp), wrbuf, wrbuf_len)) <= 0 ) {
            printf("Failed to write initial payload bytes (%lu)\n", total);
            perror("write():\n");
            exit(-1);
        }
    }


    sleep(1);

    gettimeofday(&start, NULL);
    /* read data from network & write to the file */
    while(1) {
        if ( (n = read(sd, buf, BUF_SIZE)) <= 0) {
            gettimeofday(&stop, NULL);
            if (readFailCount == 0)
            {
                printf("read failed: n = %d\n", n);
                perror("read(): ");
            }
            usleep(10000);
            readFailCount++;
            if(readFailCount == 30)
            {
                goto    http_client_exit;
            }
            continue;

            /* At this point the residue has to be 0, if not we need
             * to decrypt and write the residual bytes to disk */
            BDBG_ASSERT(residue == 0);
            break;
        }

        gettimeofday(&stop, NULL);

#ifdef B_HAS_DTCP_IP
        if (bDtcpEnable)
        {
            enc_buf_len = residue + n;
            clr_buf_len = 2*BUF_SIZE;
            memcpy (&enc_buf[residue], buf, n);
            DtcpAppLib_StreamDepacketizeData(streamHandle, akeHandle, &enc_buf[0], enc_buf_len, clr_buf, &clr_buf_len, &total_size, &bPcpFound);
            total_bytes_written += clr_buf_len;

            if (total_bytes_written > filesize)
            {
                unsigned int padding = filesize & 0xF;
                if (padding != 0)
                {
                    padding = 16 - padding;
                }
                clr_buf_len -= padding;
                BDBG_ERR(("Removing padding bytes (%d)\n", padding));
            }
            wrbuf = clr_buf;
            wrbuf_len = clr_buf_len;

            /* Re-compute residue */
            residue = enc_buf_len - total_size;
            /* Copy the residual bytes to the start of the clear buffer */
            memcpy(&enc_buf[0], &enc_buf[total_size], residue);
        }
        else
#endif
        {
            wrbuf = buf;
            wrbuf_len = n;
        }

        if (((bytes = write(fileno(fp), wrbuf, wrbuf_len)) <= 0) || bytes != wrbuf_len) {
            printf("Failed to write payload bytes (%d)\n", bytes);
            perror("write():\n");
            break;
        }

        dt = difftime1(&start, &stop);
        total += bytes;
        count++;
        if (verbose && (count % 100) == 0) {
            printf("[%10lu] Received %10lu Bytes in dt = %10.2fusec at rate %2.1f\n",
                    count, total, dt, (total * 8. / dt));
        }
    }



http_client_exit:

    strcat(cmndBuf,fname);

    /* Compute checksum only if a valid checksum is received from in the HTTP header */
    if (chkSum1)
    {
        printf("\nCOMPUTING CHECKSUM OF THE RECEIVED FILE ******** %s \n", fname);

        if((ptr = popen(cmndBuf,"r")) != NULL)
        {
            printf("\nMd5Sum output ====================================\n");
            while (fgets(tempBuf, TEMP_BUF_SIZE, ptr) != NULL)
                          (void) printf("%s\n", tempBuf);
            if(ptr != NULL)
                (void) pclose(ptr);

             chkSum = strtok(tempBuf," ");

             printf("%s\n", chkSum);

            if(strcmp(chkSum,chkSum1)==0)
            {
                printf("\nYeeeeeeeeeee Check sum  matched -- Passed the test\n");
            }
            else
            {
                printf("\nOh noooo Check sum didn't match -- Test Failed  \n");
            }
        }
        else
        {
            printf("\nChecksum computation failed popen Failed --------------------\n");
            return -1;

        }
    }

#ifdef B_HAS_DTCP_IP
dtcp_shutdown:
    /* Free all allocated buffers for decryption */
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
            DtcpAppLib_CloseAke(dtcpCtx, akeHandle);
            DtcpAppLib_Shutdown(dtcpCtx);
        }
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA)
        DtcpCleanupHwSecurityParams();
        if (dmaHandle)
            NEXUS_Dma_Close(dmaHandle);
#endif
    }
#endif

    dt = difftime1(&start, &stop);
    printf("Final stats: Received %10lu bytes to %s file in %10.1fusec at %2.1f rate\n",
            total, fname, dt, (total * 8. / dt));
    fclose(fp);
    free(xbuf);
    free(rbuf);
    free(post);

#if NXCLIENT_SUPPORT
    NxClient_Uninit();
#else /* !NXCLIENT_SUPPORT */
    NEXUS_Platform_Uninit();
#endif /* NXCLIENT_SUPPORT */

    return 0;
}
