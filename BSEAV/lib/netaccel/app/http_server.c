/******************************************************************************
 * (c) 2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#define NUM_BUF_DESC    24
#define FILENAME_MAX_LEN 256

char url[256];          /* content name to stream out */
char url_root[256];     /* Root directory where content is */
char recvbuf[1024];
char sendbuf[2048];
char response[2048];
char interfaceName[32];
char mtuSettingCommand[1024] = {"ifconfig "};
char mtuSizeString[16];

char mtuString[8]= {" mtu "};


/* global default value  */
int packetSize = 188;
int numPacketsPerTcp = 7;

int bufferSize = BUF_SIZE;

int tcpPacketSize;

int mtuSize = 1500;
int tcpHeaderSize = 20;
int ipHeaderSize = 20;


char *defaultIntrfName = "eth0";
off_t filesize;

void usage()
{
    printf("Usage: http_test_server [-p <port> -a -r <rate> -c <cpu affinity> -m -f <content-directory> -l -k -h]\n");
    printf("options are:\n");
    printf(" -p <port>              # port to listen on (default: 5000)\n");
    printf(" -r <rate>              # rate (default: 20Mpbs)\n");
    printf(" -f <content-directory  # (default: /data/videos) \n");
    printf(" -i <interface name>    # name of the interface to send multicast join on (default: eth0)\n");
    printf(" -l                     # keep resending the file (default: stream only once)\n");
    printf(" -v                     # print periodic stats (default: no)\n");
    printf(" -h                     # prints http_test_server usage\n");
    printf(" -s                     # Each packet size, Eg ts is 188, tts 192, dss 130, dsstts 134. Default 188.\n");
    printf(" -n                     # Number of packets that will be send out in each tcp packet. Default 7.\n");
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
    struct ifreq ifr;

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

    printf("\n %s: new DGRAM socket to stream content on %d using (%s) \n", __FUNCTION__,  interfaceName );
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interfaceName, sizeof(ifr.ifr_name)-1);
    if ( (setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr) )) < 0 ) {
        perror("SO_BINDTODEVICE");
        close(sd);
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
    FILE *fp;

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
int handleHttpRequest(int sd, int socket_type, char *filename)
{
    fd_set rfds;
    struct timeval tv;
    int nsd = -1;
    struct sockaddr_in remoteAddr;
    char cmndBuf[CMND_BUFF_SIZE] = "md5sum ";
    char tempBuf[TEMP_BUF_SIZE];
    char *chkSum;
    FILE *ptr;

    int addrLen = sizeof(remoteAddr);
    int nbytes;

    while (1) {

        waitForNetworkEvent(sd);

        /* accept connection */
        if ( (nsd = accept(sd, (struct sockaddr *)&remoteAddr, (socklen_t *)&addrLen)) < 0 ) {
            perror("ERROR: accept(): exiting...");
            break;
        }
        printf("Accepted Connection from %lx:%d \n", remoteAddr.sin_addr.s_addr, ntohs(remoteAddr.sin_port));

        waitForNetworkEvent(nsd);


        /** ProcessHttpRequestAndSendResponse()  **/
        /* Read HTTP request */
        if ((nbytes = read(nsd, recvbuf, 1024)) <= 0) {
            perror("read failed to read the HTTP Get request\n");
            continue;
        }
        recvbuf[nbytes] = 0;
        printf("Read HTTP Req ( %d bytes)[\n%s]\n", nbytes, recvbuf);

        /* Parse HTTP request & open the content file */
        parseToken(recvbuf, url, sizeof(url), "GET /", " ");
        if( openUrlFile(url, filename) ) {
            printf("File Not found, go back to listening\n");
            continue;
        }
        printf("Stream file = %s size=%lld\n", filename, filesize);

#if 0
        strcat(cmndBuf,filename);

        printf("\n Computing check sum of the file ******************** %s \n", cmndBuf);

        if((ptr = popen(cmndBuf,"r")) != NULL)
        {
            while (fgets(tempBuf, TEMP_BUF_SIZE, ptr) != NULL)
                              (void) printf("%s\n", tempBuf);
            if(ptr != NULL)
                (void) pclose(ptr); /*tempBuf[TEMP_BUF_SIZE]    */

             chkSum = strtok(tempBuf," ");

             printf("checksum value is %s\n", chkSum);
        }
        else
        {
            printf("\nChecksum computation failed popen Failed --------------------\n");
            return -1;

        }

#endif


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

        printf("HTTP Response [\n%s]", sendbuf);

        /* send out HTTP response */
        if (write(nsd, sendbuf, nbytes) != nbytes) {
            printf("Failed to write HTTP Response of %d bytes\n", nbytes);
            perror("write(): \n");
            break;
        }

        return nsd;
    }
    return -1;
}


int main(int argc, char *argv[])
{
    int port = 5000;        /* Server Port */
    unsigned long new_mask;
    double rate = 19.4;     /* Default rate to stream content at */
    int loop = 0;
    struct timeval start, stop;
    int verbose = 0;
    int sd;                 /* socket descriptor */
    unsigned long count = 0;
    unsigned long total = 0;
    int buflen = bufferSize;
    struct sockaddr_in dest;
    int c, i, j;
    int fd = -1;
    unsigned char *xbuf, *buf;
    int len;
    loff_t bytes= 0;
    double dt=0, maxrate = 19.4;
    struct iovec iov[NUM_BUF_DESC];
    int nextBuf;
    int socket_type;
    char filename[FILENAME_MAX_LEN];
    unsigned long packetCount = 0;


    url[0] = '\0';
    url_root[0] = '\0';


    /* set default interface name */
    strncpy(interfaceName, defaultIntrfName, sizeof(interfaceName)-1);

    while ((c = getopt(argc, argv, "ap:c:f:r:s:n:i:lmkvh")) != -1)
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
        case 'i':
            memset(interfaceName, 0, sizeof(interfaceName)-1);
            strncpy(interfaceName, optarg, sizeof(interfaceName)-1);
            break;

        case 's':
            packetSize = atof(optarg);
            break;

        case 'n':
            numPacketsPerTcp = atof(optarg);
            break;

        case 'h':
        default:
            usage();
            return -1;
        }
    }

    /* setting buffer size based on packetSize and numPacketPerTcp */

    tcpPacketSize = (packetSize * numPacketsPerTcp);

    /** compute mtu size , so that tcp only generates packet with
     *  integer number of ts/tts packets (say 7). */
    mtuSize = tcpPacketSize + tcpHeaderSize + ipHeaderSize;

    if(mtuSize > 1500)
    {
        /*restricting mtusize to ethernet max mtusize.*/
        mtuSize = 1500;
    }


    /************************ Setting MTU ******************************/
    strncat(mtuSettingCommand,interfaceName, sizeof(interfaceName)-1 );

    strncat(mtuSettingCommand,mtuString, sizeof(mtuString)-1 );
    memset(mtuSizeString, 0, sizeof(mtuSizeString));  /* getaddrinfo() requires port # in the string form */
    snprintf(mtuSizeString, sizeof(mtuSizeString)-1, "%d", mtuSize);

    strncat(mtuSettingCommand,mtuSizeString, sizeof(mtuSizeString)-1 );

    printf("\n -------------------- Final Mtu setting string is -------------\n");

    system(mtuSettingCommand);
    printf("\n|%s| \n", mtuSettingCommand);
    /*********************** Mtu setting is done **********************/


    /* print usage anyway */
    usage();

    if (url_root[0] == '\0')
        strncpy(url_root, DEFAULT_CONTENT_DIR, sizeof(url_root)-1);

    printf("http_test_server: port %d, content directory %s, rate %f, loop around %d, verbose %d \n",
            port,
            url_root, maxrate, loop, verbose);



    /* allocate FIFO where data is read from disk & then sent out on the network */
    for (i=0; i<NUM_BUF_DESC; i++) {
        if ( (xbuf = malloc(bufferSize + ALIGN_4096)) == NULL ) {
            perror("posix_memalign failure():");
            goto exit;
        }
        iov[i].iov_base = (unsigned char *)(((unsigned long)xbuf + ALIGN_4096) & ~ALIGN_4096);
        iov[i].iov_len = bufferSize;
    }
    printf("Allocated %d-bytes for buffering, # of DESC=%d\n",NUM_BUF_DESC*(bufferSize + ALIGN_4096),NUM_BUF_DESC);


    /* setup HTTP Server */
    socket_type = SOCK_STREAM;
    if ( (sd = tcpServerSetup(port, socket_type)) < 0) {
        printf("Failed to Setup TCP Server, Exiting...\n");
        exit(-1);
    }

    /* wait for HTTP request & send HTTP reply */
    sd = handleHttpRequest(sd, socket_type, filename);

    if (sd<0) {
        printf("Failed handling HttpRequest, Exiting...\n");
        goto exit;
    }


    /* get file ready for streaming */
    if ((fd = open(filename, O_RDONLY)) <= 0) {
            perror("open():");
            printf("\n File Open Error.\n");
            goto exit;
    }
    printf("Content File opened\n");



    gettimeofday(&start, NULL);
    nextBuf = 0;
    while(1) {


        /* get next buffer to read into */
        buf = (unsigned char *)iov[nextBuf++].iov_base;
        nextBuf %= NUM_BUF_DESC;


        /* read next data chunk */
        if ( (buflen  = read(fd, buf, bufferSize)) < 0 ) {
            perror("read():");

            printf("\n File read Error.\n");
            goto exit;
        }
        else if (buflen == 0) {
            printf("**** Reached EOF *******\n");
            gettimeofday(&stop, NULL);
            if (loop) {
                lseek(fd, 0, SEEK_SET);
                continue;
            }
            break;
        }
        else
        {

            printf("\n File read size = %d.\n",buflen );
        }

send_data:
        /* send the data */
        if ( (bytes = write(sd, buf, buflen)) != buflen) {
            printf("Write failed to write %10lu bytes, instead wrote %d bytes\n", buflen, bytes);
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
    for (i=0; i<NUM_BUF_DESC; i++)
        free(iov[i].iov_base);

    printf("Final stats: Streamed %lu Bytes of %s file in dt = %10.2fusec at rate=%2.1f/%2.1f\n",
            total, filename, dt, rate, maxrate);


    printf("\n Number of packet send %d \n", count);

    /* needed for all data to get drained from the socket */
    sleep(2);
    return 0;
}
