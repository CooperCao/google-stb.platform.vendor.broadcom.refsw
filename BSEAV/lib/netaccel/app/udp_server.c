/******************************************************************************
 * (c) 2007-2014 Broadcom Corporation
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


#define PING_STR_LEN  256

void usage()
{
    printf("Usage:\n udp_test_server -d <dst-ip> -p <port> -f <content-file> [-h -r <rate> -l -a -c <affinity>]\n");
    printf("options are:\n");
    printf(" -d <dst-ip>   # dst IP address to use\n");
    printf(" -p <port>     # dst port to use\n");
    printf(" -f <file>     # url to stream; /data/videos prepended \n");
    printf(" -r <rate>     # rate (default: 19.4Mpbs)\n");
    printf(" -c <affinity> # affinity (default = CPU0 = 1; CPU1 = 2; ANY = 3)\n");
    printf(" -l            # keep resending the file (default: plays only once)\n");
    printf(" -v            # print debug stats\n");
    printf(" -m            # send test pattern from memory (default: stream file from disk)\n");
    printf(" -a            # use accelerated sockets (default: STD Socket)\n");
    printf(" -t <proto>    # protocol (default = RTP = 1; UDP = 1)\n");
    printf(" -k            # disable UDP checksum computation (default: enabled)\n");
    printf(" -n            # send number of buffersize worth of data\n");

    printf(" -h            # prints udp_test_server usage\n");
    printf("\n");
}

double difftime1(struct timeval *start, struct timeval *stop)
{
    double dt = 1000000.*(stop->tv_sec - start->tv_sec) + (stop->tv_usec - start->tv_usec);
    return dt;
}


int sockSetDesc(int sd, int socket_type, int num_desc)
{

#if 0
    STRM_SetBuffer_t bi;
    unsigned int len = sizeof(bi);
    bi.num_desc = num_desc;

    if (setsockopt(sd, SOCK_BRCM_DGRAM, STRM_SETDESC, &bi, len) != 0)
        return -1;
#endif

    return 0;
}

int sockSetMode(int sd, int socket_type, int disable_checksum)
{
    STRM_Ctrl_t ctrl;
    memset(&ctrl,0,sizeof(ctrl));

    printf("TCP Checksum computation %s\n", (disable_checksum == 1) ? "OFF" : "ON");
    if (disable_checksum == 0)
        return 0;
#if 0
    if (socket_type == SOCK_BRCM_DGRAM) {
        ctrl.disable_checksum = disable_checksum;
        if(setsockopt(sd, socket_type, STRM_CTRL, &ctrl, sizeof(ctrl)) != 0)
            printf("setsockopt: %s Failed to disable checksum\n", "STRM_CTRL");
    }
    else
#endif
    {
        if (setsockopt( sd, SOL_SOCKET, SO_NO_CHECK, (const char*)&disable_checksum, sizeof(disable_checksum) ) < 0 ){
            perror("SO_NO_CHECK error\n");
            return -1;
        }
    }
    printf("********* successfully turned off the UDP checksum ************\n");
    return 0;
}

int sockWait(int sd, uint32_t size)
{

#if 0
    STRM_Status_t status;
    unsigned int len = sizeof(status);

    if(getsockopt(sd,  SOCK_BRCM_DGRAM, STRM_STATUS, &status, &len) != 0)
        return -1;

    while((status.free_desc_count <= 2) || ((status.buffer_locked_depth + size ) > (size * NUM_BUF_DESC))) {
        usleep(10000);fwait

        if (getsockopt(sd,  SOCK_BRCM_DGRAM, STRM_STATUS, &status, &len) != 0)
            return -1;
    }
#endif

    return 0;
}

static void setThreadAffinity(int cpu_affinity_mask)
{
#ifdef __mips__
    unsigned long new_mask;

    syscall(SYS_sched_setaffinity, getpid(), 4, &cpu_affinity_mask);
    syscall(SYS_sched_getaffinity, getpid(), 4, &new_mask);

    printf("Updated CPU Affinity mask %lu\n", new_mask);
    /* uncomment this line to verify the cpu affinity */
    /* while (1); */
#endif
}

int main(int argc, char *argv[])
{
    struct sockaddr_in dest;
    int c, i;
    int sfd;
    char *dstip = NULL;
    int dstport = 0;
    char fileName[128];
    int fd = -1;
    unsigned char *xbuf, *buf;
    int buf_len;
    int buf_size;
    unsigned long count = 0;
    loff_t bytes= 0;
    struct timeval start, stop;
    double rate = 20,dt=0,maxrate = 19.4;
    struct iovec iov[NUM_BUF_DESC];
    int nextBuf;
    char pingStr[PING_STR_LEN];
    int loop = 0;
    char url[256];
    int accel_socket = 0;       /* Dont Accelerated Socket by default */
    int socket_type;
    int verbose = 0;
    int send_from_memory = 0;
    unsigned long cpu_affinity_mask = 1;    /* CPU0 = 1, CPU1 = 2; CPU0 || CPU1 = 3 */
    StreamProtocol protocol = eStream_RTP;
    int disable_checksum = 0;
    struct ifreq ifr;
    struct sockaddr_in localAddr;
    char *interfaceName = "eth0";
    int numBuffer = 0;
    int numbufferSet = 0;

#if 1 /** ArnabCheck : Implementating One big buffer for entire O_DIRECT read **/

    int dataRead=0;
    int totalBufSize;
    unsigned char *tempBuf;
    unsigned char *tempBufPtr;
    struct msghdr message;

#endif


    url[0] = '\0';
    while ((c = getopt(argc, argv, "i:laf:d:p:r:c:t:n:kvmh")) != -1)
    {
        switch (c)
        {
        case 'l':
            loop = 1;
            break;
        case 'd':
            dstip = optarg;
            break;
        case 'f':
            strncpy(url, optarg, sizeof(url));
            break;
        case 'p':
            dstport = atoi(optarg);
            break;
        case 'c':
            cpu_affinity_mask = atoi(optarg);
            break;
        case 'r':
            maxrate = atof(optarg);
            break;
        case 'a':
            accel_socket = 1;
            break;
        case 'v':
            verbose = 1;
            break;
        case 'i':
            interfaceName = optarg;
            break;
        case 'm':
            send_from_memory = 1;
            break;
        case 'k':
            disable_checksum = 1;
            break;
        case 't':
            protocol = atoi(optarg);
            if (protocol != eStream_UDP && protocol != eStream_RTP) {
                printf("Incorrect Protocol %d\n", protocol);
                usage();
                return -1;
            }
            break;
        case 'n':
                numBuffer = atoi(optarg);

                printf("\n Numbers is %d ", numBuffer);
                numbufferSet = 1;
            break;
        case 'h':
        default:
            usage();
            return -1;
        }
    }

    if (dstip == NULL || dstport == 0 || (*url == '\0' && !send_from_memory)) {
        usage();
        exit(-1);
    }

    if (accel_socket) {
        /* accelerated sockets allows you to send any size packets */
        buf_size = (188*7*128);

        /* optional to set the affinity */
        setThreadAffinity(cpu_affinity_mask);
        socket_type = SOCK_BRCM_DGRAM;
    }
    else {
        /* we can only write Ethernet MTU worth of data using normal sockets */
        /* otherwise, we will cause IP Fragmentation */
        /*  buf_size = (188*7); */
        buf_size = 1316;/*(188*7*2);*/
        socket_type = SOCK_DGRAM;
    }
    buf_len = buf_size;

    /* allocate buffer */
    for (i=0; i<NUM_BUF_DESC; i++) {
        if ( (xbuf = malloc(buf_size + ALIGN_4096)) == NULL ) {
            perror("posix_memalign failure():");
            goto exit;
        }
        iov[i].iov_base = (unsigned char *)(((unsigned long)xbuf + ALIGN_4096) & ~ALIGN_4096);
        iov[i].iov_len = buf_size;
    }
    printf("Allocated %d-bytes for streaming buffer, # of s/w DESC=%d\n", NUM_BUF_DESC*(buf_size + ALIGN_4096), NUM_BUF_DESC);

    /** ArnabCheck : Implementating One big buffer , this is aligned so that it can be used for O_DIRECT read **/

    totalBufSize = (buf_size * 512*8);
    if((tempBufPtr = malloc(totalBufSize + ALIGN_4096)) == NULL ) {
            perror("posix_memalign failure():");
            goto exit;
        }
    tempBufPtr = (unsigned char *)(((unsigned long)tempBufPtr + ALIGN_4096) & ~ALIGN_4096);



    if (send_from_memory) {
        for (i=0; i<NUM_BUF_DESC; i++)
            memset(iov[i].iov_base, 0xff, buf_size);
    }
    else {
        strlcpy(fileName,"/data/videos/", sizeof(fileName)-1);
        strlcat(fileName, url, sizeof(fileName));

#if 0   /* def BRCM_ODIRECT_ENABLE  */
        if ( (fd = open(fileName,O_DIRECT, O_RDONLY)) <= 0) {
            perror("open():");
            goto exit;
        }
#else
        if ( (fd = open(fileName, O_RDONLY)) <= 0) {
            perror("open():");
            goto exit;
        }
#endif
        printf("Content File opened\n");
        /* enabled Direct IO */
        if (accel_socket)
            fcntl(fd, F_SETFL, O_DIRECT);
    }

    if ( (sfd = socket(PF_INET, socket_type, 0)) < 0 ) {
        perror("socket():");
        goto exit;
    }
    printf("%s Socket Created\n", (accel_socket == 1) ? "Broadcom Accelerated Socket": "Normal Linux Socket");
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interfaceName, sizeof(ifr.ifr_name));
    printf("Binding to i/f %s\n", interfaceName);
    if (setsockopt(sfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr) ) < 0 ) {
        printf("SO_BINDTODEVICE Failed\n");
        exit(-1);
    }

    /* initialize the destination address */
    dest.sin_family = AF_INET;
    dest.sin_port = htons(dstport);
    dest.sin_addr.s_addr = inet_addr(dstip);

    sockSetMode(sfd, socket_type, disable_checksum);

    /* need to get the arp resolution done for the destination host */
    if (accel_socket) {
        STRM_Protocol_t proto;

        if (!IN_MULTICAST(ntohl(dest.sin_addr.s_addr))) {
            snprintf(pingStr, PING_STR_LEN, "ping -q -c 1 %s > /dev/null", dstip);
            system(pingStr);
        }
        /* increase the number of s/w descriptors */
        sockSetDesc(sfd, socket_type, NUM_BUF_DESC);

        /* Notify Driver about the Protocol to use for streaming: UDP or RTP */
        memset(&proto, 0, sizeof(proto));
        proto.protocol = protocol;
        proto.fec = 0;

#if 0
        if(setsockopt(sfd, SOCK_BRCM_DGRAM, STRM_PROTOCOL, &proto, sizeof(proto)) != 0)
            printf("Sockopt Error\n");
        printf("Updated streaming protocol to %s\n", protocol == eStream_UDP ? "UDP" : "RTP");
#endif

    }
    else {
        /* increase the socket receive buffer */
        int send_buf = 600000;/*2048000;*//*600000;   */
        if (setsockopt( sfd, SOL_SOCKET, SO_SNDBUF, (const char*)&send_buf, sizeof(send_buf) ) < 0 ){
            perror("SO_SNDBUF Socket Error:\n");
            return -1;
        }
    }

    /* streaming time... */
    printf("Start streaming ...\n");
    gettimeofday(&start,NULL);
    nextBuf = 0;

    tempBuf = tempBufPtr;

    while(1) {


        int sendSize ;
        int data_len = 0;


        loff_t Tempbytes= 0;
        if (accel_socket)
            /* make sure driver has finished sending one buffer worth data */
            sockWait(sfd, buf_size);

        buf = (unsigned char *)iov[nextBuf++].iov_base;
        nextBuf %= NUM_BUF_DESC;

        if (send_from_memory)
            goto send_data;

#if 1

        if(dataRead == 0)
        {

            tempBuf = tempBufPtr;
            /* read the next block */

            if ( (data_len = read(fd, tempBuf, totalBufSize)) < 0 ) {
                perror("read():");
                goto exit;
            }
            else if (data_len == 0) {
                printf("**** Reached EOF *******\n");
                if (loop) {
                    lseek(fd, 0, SEEK_SET);
                    continue;
                }
                break;
            }

            dataRead = data_len;
        }
#else
            if ( (data_len = readv(fd, iov, NUM_BUF_DESC)) < 0 ) {
                perror("read():");
                goto exit;
            }
#endif

send_data:
        /* send the data */
        if(send_from_memory)
        {
            if ( (Tempbytes= sendto(sfd, buf, buf_len, 0, (struct sockaddr *)&dest, sizeof(struct sockaddr_in))) <= 0) {
                perror("sendto():");
                goto exit;
            }
            count++;
            bytes +=Tempbytes;

        }
        else
        {

#if 1
            if(dataRead >= buf_size)
            {
                sendSize =  buf_size;
            }
            else
            {
                sendSize =  dataRead;
            }

            if ( (Tempbytes= sendto(sfd, tempBuf, sendSize, 0, (struct sockaddr *)&dest, sizeof(struct sockaddr_in))) <= 0) {
                perror("sendto():");
                goto exit;
            }


            if(Tempbytes != 0)
            {
                tempBuf = (tempBuf + Tempbytes);
                dataRead -= Tempbytes;

                /* simple pacing logic */
                count++;
                bytes +=Tempbytes;
            }
        }
#else /** Adding sendmsg from vector ****/

        memset(&message, 0, sizeof(message));

        message.msg_name=&dest;
        message.msg_namelen=sizeof(struct sockaddr_in);
        message.msg_iov=iov;
        message.msg_iovlen=(data_len/buf_size);
        message.msg_control=0;
        message.msg_controllen=0;

        if (sendmsg(sfd,&message,0)==-1) {
            perror("\n Error in sendmsg():\n");
                goto exit;
        }
        bytes += data_len;
        count += (data_len/buf_size);
    }
#endif


        gettimeofday(&stop,NULL);
        dt = difftime1(&start,&stop);
        rate = 8.*bytes/dt;

        /** to achieve 980Mbps throughput using a single udp_server app,
            we need to disable the following pacing code.   ****/
        while(rate > maxrate) {
            usleep(10000);
            gettimeofday(&stop,NULL);
            dt = difftime1(&start,&stop);
            rate = 8.*bytes/dt;
        }
        if(verbose && (count % 100) == 0) {

#if 0
            printf("%lu (%u) Packets (Bytes) dt=%lf rate=%lf/%lf written\n", count, (uint32_t)bytes, dt, rate, maxrate);
#endif

        }

        if(numbufferSet)
        {
            numBuffer--;
        }

        if(numbufferSet && (numBuffer==0))
        {
            goto exit;
        }
    }

exit:
    for (i=0; i<NUM_BUF_DESC; i++)
        free(iov[i].iov_base);

    printf("Final stats: %lu (%u) Packets (Bytes) dt=%lf rate=%lf/%lf written\n", count, (uint32_t)bytes, dt, rate, maxrate);
    return 0;
}
