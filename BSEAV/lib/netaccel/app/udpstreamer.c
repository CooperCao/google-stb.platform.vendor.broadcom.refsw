/***************************************************************************
 *     Copyright (c) 2007-2009, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: app to start HTTP streamer
 *  
 * Revision History:
 *    
 * $brcm_Log: $
 * 
 *************************************************************/ 
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/tcp.h>
#include <linux/ip.h>

#include "bsettop_ip.h"
#include "bsettop.h"
#ifdef TODO
#define BRCM_ENABLE_AES_ENCRYPT 1
#endif
#include "bnetaccel_info.h"
#include "streamer.h"

#define URL_LEN     256
#define ALIGN_4096   (4096 - 1)
#define BUF_SIZE 	(188*7*128)
#define NUM_BUF_DESC 	24
#define O_DIRECT	0x8000

/* this function is implemented in the libnetaccel.so. It uses exact PCR based pacing to stream out UDP data */
int UdpPushServer(char *url, int fec, int protocol, char *server, int port, int forceSpeed, int pktDrop, int accel_socket);

void usage()
{
    printf("Usage: udpstreamer -d <dst-ip> -p <port> -f <content-file> [-h -r <rate> -t <proto> -l -a <num> -z <affinity>]\n");
    printf("options are:\n");
    printf(" -d <dst-ip>   # dst IP address to use\n");
    printf(" -p <port>     # dst port to use\n");
    printf(" -f <file>     # url to stream; /data/videos prepended \n");
#if 0
    printf(" -e            # enable encryption (default: off)\n");
    printf(" -i            # initialize Settop APIs (default: no)\n");
    printf(" -x            # Use XPT RAVE-IP Mode (default: no)\n");
#endif
    printf(" -r <rate>     # rate in Mpbs (default: auto computes stream bitrate using PCR index)\n");
    printf(" -s <speed>    # forced trick play speed (default: 1, normal play)\n");
	printf(" -t <proto>    # protocol (default = UDP = 0; RTP = 1; RTP+FEC = 2)\n");
	printf(" -z <affinity> # affinity (default = CPU0 = 1; CPU1 = 2; ANY = 3)\n");
    printf(" -l            # keep resending the file (default: plays only once)\n");
    printf(" -v            # print debug stats\n");
    printf(" -m            # send test pattern from memory (default: stream file from disk)\n");
    printf(" -a <num>      # use accelerated socket for sending (default 1 (accel), 0 (normal socket))\n");
    printf(" -c <count>    # simulate random packet drops: drop every count pkt (default: no)\n");
    printf(" -h            # prints udpstreamer usage\n");
    printf("\n");
}

#ifdef RAVE_IP_SUPPORT
static int settopInit = 0;
void sigHandler(int signal)
{
    printf("**** Caught Signal # %d\n", signal);
    if (settopInit != 0)
    {
        printf("sighandler(): calling bsettop_uninit\n");
        settopInit = 0;
        bsettop_uninit();
    }
    exit(-1);
}
#endif

double difftime1(struct timeval *start, struct timeval *stop) 
{
	double dt = 1000000.*(stop->tv_sec - start->tv_sec) + (stop->tv_usec - start->tv_usec);
	return dt;
}

int udpSockSetDesc(int sd, int num_desc)
{
    STRM_SetBuffer_t bi;
    unsigned int len = sizeof(bi);
    bi.num_desc = num_desc;

    if (setsockopt(sd, SOCK_BRCM_DGRAM, STRM_SETDESC, &bi, len) != 0)
        return -EINVAL;

    return 0;
}

int sockWait(int sd, int size)
{
    STRM_Status_t status;
    unsigned int len = sizeof(status);

    if(getsockopt(sd,  SOCK_BRCM_DGRAM, STRM_STATUS, &status, &len) != 0)
        return -EINVAL;

    while((status.free_desc_count <= 2) || ((status.buffer_locked_depth + size ) > (BUF_SIZE * NUM_BUF_DESC))) {
        usleep(10000);
        if (getsockopt(sd,  SOCK_BRCM_DGRAM, STRM_STATUS, &status, &len) != 0)
            return -EINVAL;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in dest;
    int c, i, j;
    int sfd, payload_len;
    char *srcip = "192.168.1.111";
    char *dstip = NULL;
    int dstport = 0;
    char fileName[128];
    int fd = -1;
    unsigned char *xbuf, *buf;
    int bufLen = BUF_SIZE; 
    int len; 
    unsigned long count = 0;
	loff_t bytes= 0;
	struct timeval start, stop;
	double rate = 19.4,dt=0,maxrate = 0;
    FILE *file;
    struct iovec iov[NUM_BUF_DESC];
    int nextBuf;
    char pingStr[256];
	int protocol = 0;
	int affinity = 2;
    int encrypt = 0;
    int loop = 0;
	char url[URL_LEN];
    int settopInit = 0;
	int forceSpeed = 1;
	int pktDrop = 0;
	int accel_socket = 1;		/* Use Accelerated Socket by default */
	int socket_type;
	int verbose = 0;
    int send_from_memory = 0;

	url[0] = '\0';
    while ((c = getopt(argc, argv, "c:s:lea:f:d:p:t:r:z:xvmh")) != -1)
    {
        switch (c) 
        {
        case 'c':
            pktDrop = atoi(optarg);
            break;
        case 'i':
            settopInit = 1;
            break;
        case 'l':
            loop = 1;
            break;
        case 'e':
            encrypt = 1;
            break;
        case 'd':
            dstip = optarg;
            break;
        case 's':
            forceSpeed = atoi(optarg);
            break;
        case 'f':
            strlcpy(url, optarg, URL_LEN-1);
            break;
		case 't':
            protocol = atoi(optarg);
            break;
        case 'p':
            dstport = atoi(optarg);
            break;
		case 'z':
            affinity = atoi(optarg);
            break;
        case 'r':
            maxrate = atof(optarg);
            break;
        case 'a':
            accel_socket = atoi(optarg);
			if (accel_socket != 0)
				accel_socket = 1;
			else
				accel_socket = 0;
            break;
        case 'v':
            verbose = 1;
            break;
        case 'm':
            send_from_memory = 1;
            break;
#if 0
        case 'x':
            raveip = 1;
            break;
#endif
        case 'h':
        default:
            usage();
            return -1;
        }
    }

	if (dstip == NULL || dstport == 0 || *url == '\0') {
		usage();
		exit(-1);
	}

	setThreadAffinity(affinity);

#ifdef RAVE_IP_SUPPORT
    if (settopInit) {
        bsettop_init(BSETTOP_VERSION);
		settopInit = 1;
        signal(SIGHUP, sigHandler);
        signal(SIGINT, sigHandler);
        signal(SIGTERM, sigHandler);
        signal(SIGQUIT, sigHandler);
        signal(SIGSEGV, sigHandler);
        signal(SIGPIPE, sigHandler);
	}
#endif
	/* if user doesn't specify rate w/ -r option, then we use the libnetaccel library to do the pacing and stream out */
	/* otherwise, we do a approximate pacing using the code below */
	if(!maxrate) {
		int fec = (protocol == 2);
		StreamProtocol s = (protocol == 0) ? eStream_UDP : eStream_RTP;
		UdpPushServer(url,fec, s, dstip, dstport, forceSpeed, pktDrop, accel_socket);
		exit(0);
	}

    for (i=0; i<NUM_BUF_DESC; i++) {
        if ( (xbuf = malloc(BUF_SIZE + ALIGN_4096)) == NULL ) {
            perror("posix_memalign failure():");
            goto exit;
        }
        iov[i].iov_base = (unsigned char *)(((unsigned long)xbuf + ALIGN_4096) & ~ALIGN_4096);
        iov[i].iov_len = BUF_SIZE;
    }
	printf("Allocated %d-bytes for buffering, DESC=%d\n",NUM_BUF_DESC*(BUF_SIZE + ALIGN_4096),NUM_BUF_DESC);

	if (send_from_memory) {
    	for (i=0; i<NUM_BUF_DESC; i++) 
			memset(iov[i].iov_base, 0xff, BUF_SIZE);
	}
	else {
		strlcpy(fileName,"/data/videos/", sizeof(fileName)-1);
		strlcat(fileName,url, sizeof(fileName));

    	if ( (fd = open(fileName, O_RDONLY|O_DIRECT)) <= 0) {
        	perror("open():");
        	goto exit;
    	}
    	printf("Content File opened\n");
	}

	if (accel_socket)
		socket_type = SOCK_BRCM_DGRAM;
	else
		socket_type = SOCK_DGRAM;
    if ( (sfd = socket(PF_INET, socket_type, 0)) < 0 ) {
        perror("socket():");
        goto exit;
    }
    printf("%s Socket Created\n", (accel_socket == 1) ? "Broadcom Accelerated Socket": "Normal Linux Socket");


	/* optional: notify driver about the protocol to use, if not specified, driver assumes plain UDP */
	if (accel_socket) {
		STRM_Protocol_t proto;
		memset(&proto, 0, sizeof(proto));
		switch(protocol){
		case 1: /* RTP */
			proto.protocol = eStream_RTP;
			proto.fec = 0;
			break;
		case 2: /* RTP with FEC */
			proto.protocol = eStream_RTP;
			proto.fec = 1;
			break;
		default: /* UDP */
			proto.protocol = eStream_UDP;
			proto.fec = 0;
			break;
		}

		if(setsockopt(sfd, SOCK_BRCM_DGRAM, STRM_PROTOCOL, &proto, sizeof(proto)) != 0)
			printf("Sockopt Error\n");
	}
	else {
		/* increase the socket receive buffer */
		int rcv_buf = 600000;   
		if (setsockopt( sfd, SOL_SOCKET, SO_SNDBUF, (const char*)&rcv_buf, sizeof(rcv_buf) ) < 0 ){
			perror("SO_RCVBUF Socket Error:\n");
			return -EINVAL;
		}
	}

    /* initialize the destination address */
    dest.sin_family = AF_INET;
    dest.sin_port = htons(dstport);
    dest.sin_addr.s_addr = inet_addr(dstip);

	/* need to get the arp resolution done for the destination host */
    if (!IN_MULTICAST(ntohl(dest.sin_addr.s_addr))) {
        snprintf(pingStr, sizeof(pingStr), "ping -q -c 1 %s > /dev/null", dstip);
        system(pingStr);
    }
   
#ifdef RAVE_IP_SUPPORT
    if (encrypt) {
        int val = 1;
        if (setsockopt(sfd, IPPROTO_UDP, BRCM_ENABLE_AES_ENCRYPT, &val, 4) != 0)
        {
            perror("setsockopt(): failed to enable encryption");
            goto exit;
        }
        printf("Enabled AES-128 Encryption\n");
    }
	{
		STRM_SockRaveIpCfg_t rave_ip_cfg;
		rave_ip_cfg.params.mpeg_info.video[0].pid = 0x31;
		rave_ip_cfg.params.mpeg_info.video[0].format = bvideo_codec_mpeg2;
		rave_ip_cfg.params.mpeg_info.audio[0].pid = 0x34;
		rave_ip_cfg.params.mpeg_info.audio[0].format = baudio_format_ac3;
		rave_ip_cfg.params.mpeg_info.pcr_pid = 0x31;
		rave_ip_cfg.params.pb_parser_id = 0;
		if(setsockopt(sfd, SOCK_BRCM_DGRAM, STRM_SOCK_RAVE_IP_CFG, &rave_ip_cfg, sizeof(STRM_SockRaveIpCfg_t)) != 0) {
			printf("Sockopt %s Error\n", "STRM_SOCK_RAVE_IP_CFG");
			perror("");
			return -1;
		}
		printf("setsockopt: STRM_SOCK_RAVE_IP_CFG: opt 0x%x\n", STRM_SOCK_RAVE_IP_CFG);
	}
#endif

	/* streaming time... */
	gettimeofday(&start,NULL);
    nextBuf = 0;
	DIO_ENABLE(fd);

	while(1) {
		/* make sure driver has finished sending all the queued up data */
		if (accel_socket)
			sockWait(sfd, BUF_SIZE);

        buf = (unsigned char *)iov[nextBuf++].iov_base;

        nextBuf %= NUM_BUF_DESC;

		if (send_from_memory) 
			goto send_data;

		/* read the next block */
		if ( (bufLen = read(fd, buf, BUF_SIZE)) < 0 ) {
            perror("read():");
            goto exit;
        }
        else if (bufLen == 0) {
            printf("**** Reached EOF *******\n");
            if (loop) {
                lseek(fd, 0, SEEK_SET); 
                continue;
            }
            break;
        }
        
send_data:
        /* send the data */
        if ( (bytes += sendto(sfd, buf, bufLen, 0, (struct sockaddr *)&dest, sizeof(struct sockaddr_in))) <= 0) {
            perror("sendto():");
            goto exit;
        }

        /* simple pacing logic */ 
		count++;
		gettimeofday(&stop,NULL);
		dt = difftime1(&start,&stop);
		rate = 8.*bytes/dt;
		while(rate > maxrate) {
			usleep(10000);
			gettimeofday(&stop,NULL);
			dt = difftime1(&start,&stop);
			rate = 8.*bytes/dt;
		} 

		if(verbose && (count % 100) == 0) {
			printf("%u (%u) Packets (Bytes) dt=%lf rate=%lf/%lf written\n", count, bytes, dt, rate, maxrate);  
		}
	}

exit:
    for (i=0; i<NUM_BUF_DESC; i++)
	    free(iov[i].iov_base);
#ifdef RAVE_IP_SUPPORT
	if (settopInit)
		bsettop_uninit();
#endif
	printf("Final stats: %u (%u) Packets (Bytes) dt=%lf rate=%lf/%lf written\n", count, bytes, dt, rate, maxrate);  
    return 0;
}
