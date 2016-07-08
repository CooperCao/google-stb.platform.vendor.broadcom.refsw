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
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <stdlib.h>
#include "bip_server.h"
#include "tcp_connx_migration.h"

#define SERVER_PORT 80
#define BUF_MASK    0x1ff
#define BUF_SIZE    (188*7*64)

void usage(void)
{
    printf("Usage: http_test_client: -d <srv-ip> -p <srv-port> -f <content-file> [-h -a -v]\n");
    printf("options are:\n");
    printf(" -d <srv-ip>   # IP address of HTTP Server (e.g. 192.168.1.110)\n");
    printf(" -p <srv-port> # Port of HTTP Server (e.g. 80)\n");
    printf(" -f <file>     # url of stream; /data/videos prepended (e.g. portugal.mpg) \n");
    printf(" -m            # just leave content in memory, dont write to disk (default: write to file)\n");
    printf(" -v            # print periodic stats (default: no)\n");
    printf(" -h            # prints http_test_client usage\n");
    printf("\n");
}

double difftime1(struct timeval *start, struct timeval *stop)
{
    double dt = 1000000.*(stop->tv_sec - start->tv_sec) + (stop->tv_usec - start->tv_usec);
    return dt;
}

/* This function creates a TCP connection to server and returns the socket descriptor */
int TcpConnect(char *server, int port, int socket_type)
{
    int sd,rc;

    struct sockaddr_in localAddr, servAddr;
    struct hostent *h;

    printf("TCP - Connection to %s:%d ...\n",server, port);

    h = gethostbyname(server);
    if (h == NULL) {
        perror("Unknown Host");
        return (-EINVAL);
    }

    servAddr.sin_family = h->h_addrtype;
    memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0],h->h_length);
    servAddr.sin_port = htons(port);

    /* Open Socket */
    sd = socket(AF_INET, socket_type, 0);
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

    /* Connect to server */
    rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
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

#if 0
int BIP_Server_Stop(BipServerHandle serverCtx)
{
    int rc;
    int socketFd;

    myPrint("Press any key to proceed with Connection Migration back to Linux !!!");
    printLinuxConnxState();
    getchar();
    /* stop the ASP channel: this ensures that ASP has stopped streaming and synchronized the TCP state */
    /* this means that all ACKs for any streamed out data has been received */
    /* also, this call returns the current TCP session state */
    rc = NEXUS_Asp_Stop(&serverCtx->socketState);
    CHECK_ERR_NZ_GOTO("NEXUS_Asp_Stop failed...", rc, error);

    /* now migrate the connection back to the host */
    rc = setTcpStateAndMigrateToLinux(&serverCtx->socketState, &socketFd);
    CHECK_ERR_NZ_GOTO("setTcpStateAndMigrateToLinux Failed ...", rc, error);

    /* stop filtering of packets */
    stopAspManager(serverCtx);

    myPrint("Press any key to proceed with Connection tear-down !!!");
    getchar();
    /* now close the socket: Linux should send out a FIN to the client */
    close(socketFd);
    myPrint("Linux socket status: server side TCP connection should now be in the TIME_WAIT state !!!!!");
    printLinuxConnxState();

    printf("%s: Done\n", __FUNCTION__);
    return 0;

error:
    return -1;
}
#endif

int setupTcpMigration(int socketFd, SocketStateHandle hSocketState, char *pRequestBuf, unsigned requestBufLength)
{
    int rc;

    /* we determine here if we can use ASP for this streaming session (true for most streaming out sessions) */
    /* for now, assume that we can and proceed below w/ ASP setup */
    myPrint("Press any key to start TCP Connection Offload to ASP simulator....");
    getchar();

    myPrint("Linux socket status: client side TCP connection state before migration!!!!!");
    printLinuxConnxState();

    /* setup Asp Manager: it handles all runtime packets that ASP h/w can't directly receive or process */
    /* e.g. fragmented packets, ICMP PATH MPU pkts, etc. */
    startAspManager();

    /* gather TCP state info from Linux and Freeze the TCP socket */
    rc = getTcpStateAndMigrateFromLinux(socketFd, hSocketState);
    CHECK_ERR_NZ_GOTO("getTcpStateAndMigrateFromLinux Failed ...", rc, error);

    /* now close the socket */
    /* NOTE!!: since we have frozen the socket, it doesn't cause Linux to send out TCP FIN to the server. */
    /* And thus the server doesn't know about this migration to ASP */
    close(socketFd);
    myPrint("Linux socket status: client side TCP connection should disappear at this point!!!!!");

#if 0 //Commented for now
    /* pass TCP state to ASP so that it is setup */
    NEXUS_Asp_GetDefaultetSettings();
    NEXUS_Asp_SetSettings();
    CHECK_ERR_NZ_GOTO("NEXUS_Asp_SetSettings failed...", rc, error);
#endif

    /* start the ASP channel: currently passing TCP state here */
    rc = NEXUS_Asp_Start(hSocketState, pRequestBuf, requestBufLength);
    CHECK_ERR_NZ_GOTO("NEXUS_Asp_Start failed...", rc, error);

    /* TODO: now program the switch to redirect all packets to ASP instead of host */

    printf("%s: Done\n", __FUNCTION__);
    return 0;

error:
    return -1;
}

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
    int  port = 0;
    ssize_t n;
    unsigned char *buf, *xbuf;
    ssize_t bytes = 0;
    unsigned long total=0;
    unsigned long count=0;
    char tok;
    ssize_t len;
    double dt;
    struct timeval start, stop;
    int dont_write_to_disk = 0;
    int verbose = 0;
    SocketState socketState;

    while ((c = getopt(argc, argv, "d:p:f:mvh")) != -1) {
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
        case 'm':
            dont_write_to_disk = 1;
            break;
        case 'v':
            verbose = 1;
            break;
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

    buf = (unsigned char *) (((unsigned long)xbuf + BUF_MASK) & ~BUF_MASK);

    /* Build HTTP Get Request */
    n = snprintf(post, HTTP_POST_SIZE,
            "GET /%s HTTP/1.1\r\n"
            "Host: %s:%d\r\n"
            "User-Agent: BRCM ASP Test App\r\n"
            "\r\n",
            fname, server, port);
    printf("Sending HTTP Request:----->[\n%s]\n", post);
#ifdef __mips__
//  syscall(SYS_cacheflush, post, n, DCACHE);
#endif

    /* open file for writing */
    if (!dont_write_to_disk) {
        fp = fopen(fname,"wb");
        if (!fp) {
            perror("File open error:\n");
            exit(-1);
        }
    }

    /* Connect to Server */
    sd = TcpConnect(server, port, SOCK_STREAM);
    if (sd < 0) {
            printf("Connection to Server Failed, Exiting...\n");
            exit(-1);
    }

    /* Initiate Connection Offload */
    if ( setupTcpMigration(sd, &socketState, post, strlen(post)) != 0 ) {
        printf("Failed to migrate the connection to Linux & send the initial request");
        exit(-1);
    }

    printf("Succesfully Send HTTP Get Request to %s:%d\n", server, port);
    sleep(1);

    printf("Exiting here for now: TODO: need to add logic to receive incoming PKTS via asp simulator!...\n");
    exit(0);

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
    printf("HTTP Response: -----> [\n%s]\n", rbuf);
    *p = tok;

    /* write any data that was read part of the initial read */
    if (!dont_write_to_disk && n>len) {
        if ( (total = write(fileno(fp), p, n - len)) <= 0 ) {
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
            printf("read failed: n = %d\n", n);
            perror("read(): ");
            break;
        }

        gettimeofday(&stop, NULL);
        if (dont_write_to_disk) {
            bytes = n;
            goto after_write;
        }
        if (((bytes = write(fileno(fp), buf, n)) <= 0) || bytes != n) {
            printf("Failed to write payload bytes (%d)\n", bytes);
            perror("write():\n");
            break;
        }

after_write:
        dt = difftime1(&start, &stop);
        total += bytes;
        count++;
        if (verbose && (count % 100) == 0) {
            printf("[%10lu] Received %10lu Bytes in dt = %10.2fusec at rate %2.1f\n",
                    count, total, dt, (total * 8. / dt));
        }
    }

    dt = difftime1(&start, &stop);
    printf("Final stats: Received %10lu bytes to %s file in %10.1fusec at %2.1f rate\n",
            total, fname, dt, (total * 8. / dt));
    if (fp) fclose(fp);
    free(xbuf);
    free(rbuf);
    free(post);
    return 0;
}
