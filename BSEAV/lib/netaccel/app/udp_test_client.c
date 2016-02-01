/***************************************************************************
 *     Copyright (c) 2007-2012, Broadcom Corporation
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
 * Module Description: simple test app to demonstrate UDP/RTP receiving using Normal & Accelerated Sockets
 *  
 * Revision History:
 *    
 * $brcm_Log: $
 * 
 *************************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/time.h>
#include <assert.h>
#include <asm/cachectl.h>
#include <sys/ioctl.h>
#include <net/if.h>


#include "bnetaccel_info.h"

/* Test App to demonstrate the accelerated socket interface */
/* it supports mode to read from standard DGRAM socket */
/* as well as Broadcom's accelerated socket type */
#define ALIGN_4096   (4096 - 1)
#define MEM_SIZE 4*1024*1024
#define UDP_HDR_SIZE 8
#define RTP_HDR_SIZE 12

#define MAXADDRNAME 100

typedef enum {
    eProtoUDP,
    eProtoRTP,
    eProtoMAX
} eProtos;

typedef struct {
	int pktsPerRead;
	eProtos proto;
	int writeToDisk;
	int doSockAccel;
	int useCpuCopy;
	int timeout;
	int tsSize;
	int iphVersion;
	short port;
	char *host;
	char *service;
	char *filename;
	int pktSize;
    char *interfaceName;
}ExampleIpCfg;

typedef struct {
	int sfd;
	int wfd;
	int bufSize; 
	int hdrSize;
	struct iovec *vector;
	uint8_t *buf;
	struct sockaddr saddr;
	int bytesRecv;
	int bytesToFeed;
}ExampleIpCtx;


int feedToPlayback = 0;

#define B_REFSW_DEBUG
#ifdef B_REFSW_DEBUG
struct udphdr {
    unsigned short   source;
    unsigned short   dest;
    unsigned short   len;
    unsigned short   checksum;
};

typedef struct rtphdr {
    uint16_t word0;
#define VERSION_MASK 0xc000
#define VERSION_SHIFT 14
#define PADDING_MASK 0x2000
#define PADDING_SHIFT 13
#define EXTENSION_MASK 0x1000
#define EXTENSION_SHIFT 12
#define CSRC_MASK 0x0f00
#define CSRC_SHIFT 8
#define MARKER_MASK 0x0080
#define MARKER_SHIFT 7
#define TYPE_MASK 0x007f
#define TYPE_SHIFT 0
    uint16_t sequenceNum;
    uint32_t timestamp;
    uint32_t ssrc;
} rtphdr;

void printRtp(rtphdr *rtp)
{
	uint16_t word0;
	word0 = ntohs(rtp->word0);
	printf("RTP Hdr: Version %d Pad %d Ext %d CRSC count %d Marker %d Type %d ",
			(word0 & VERSION_MASK) >> VERSION_SHIFT,
			(word0 & PADDING_MASK) >> PADDING_SHIFT,
			(word0 & EXTENSION_MASK) >> EXTENSION_SHIFT,
			(word0 & CSRC_MASK) >> CSRC_SHIFT,
			(word0 & MARKER_MASK) >> MARKER_SHIFT,
			(word0 & TYPE_MASK) >> TYPE_SHIFT
		  );
    printf("Seq %d, TS %u, SSRC %u\n",
            ntohs(rtp->sequenceNum),
            ntohl(rtp->timestamp),
            ntohl(rtp->ssrc));
}

void printTS(unsigned char *ptr)
{
    printf("Transport Header Fields are:\n");
    printf("[0] 0x%x, [1] 0x%x, [2] 0x%x, [3] 0x%x\n", ptr[0], ptr[1], ptr[2], ptr[3]);
}

#endif /* B_REFSW_DEBUG */

void usage()
{
    printf("Usage: udp_test_client -d <ip> -p <port> [-f <content-file>] [-a] [-s <num>] [-v <num>] [-n <num>] [-t <num>] [-i name] [-w] [-h] \n");
    printf("options are:\n");
    printf(" -a            # use accelerated socket type, default is regular socket\n");
    printf(" -c            # use cpu to copy, default is to use M2M DMA Engine to copy received data to user space\n");
    printf(" -d <ip>       # IP address of Live IP Service (e.g. 192.168.1.110)\n");
    printf(" -p <port>     # Port of Live IP Service (e.g. 5000)\n");
    printf(" -f <file>     # file to write content to (default rec0.mpg\n");
    printf(" -s <num>      # size of MPEG2 TS packets, default 188 bytes\n");
    printf(" -v            # IP version to use: IPv4: 4, IPv6: 6, default is IPv4\n");
    printf(" -t <0|1>      # protocol: UDP(0)|RTP(1), default is RTP\n");
    printf(" -n <num>      # pkts to receive per read/recvfrom call, default is 32\n");
    printf(" -z <num>      # Per Datagram size, default hdr sz + 7 *TS size\n");
    printf(" -i <name>     # interface name to send multicast join on, default let kernel choose one\n");
    printf(" -w            # write To Disk, default is no write\n");
    printf(" -h            # prints netrecv usage\n");
    printf("\n");
}

void update_stats(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
	static int pktCnt = 0;
	static int total;
	int port;
	char hostbuf[MAXADDRNAME] = {0};

	total += ipCtx->bytesRecv;
	pktCnt += ipCfg->pktsPerRead;
	if (pktCnt % (ipCfg->pktsPerRead * 100) == 0) {
		if (ipCtx->saddr.sa_family == AF_INET) {
			inet_ntop(ipCtx->saddr.sa_family, &((struct sockaddr_in *)&ipCtx->saddr)->sin_addr, hostbuf, MAXADDRNAME);
			port = ntohs(((struct sockaddr_in *)&ipCtx->saddr)->sin_port);
		}
		else {
			inet_ntop(ipCtx->saddr.sa_family, &((struct sockaddr_in6 *)&ipCtx->saddr)->sin6_addr, hostbuf, MAXADDRNAME);
			port = ntohs(((struct sockaddr_in6 *)&ipCtx->saddr)->sin6_port);
		}
		printf("Rcvd %u bytes in %u pkts, total bytes rcvd %u\n", ipCtx->bytesRecv, pktCnt, total);
	}
}

void set_default_ip_cfg(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
	memset(ipCfg, 0, sizeof(ExampleIpCfg));
	memset(ipCtx, 0, sizeof(ExampleIpCtx));

	ipCfg->pktsPerRead = 32;
	ipCfg->proto = eProtoRTP;
	ipCfg->writeToDisk = 0; 
	ipCfg->doSockAccel = 0; 
	ipCfg->useCpuCopy = 1; 
	ipCfg->timeout = 20; 
	ipCfg->tsSize = 188; 
	ipCfg->iphVersion = 4;
	ipCfg->filename = "./rec0.mpg";
	ipCfg->interfaceName = NULL;
}

int allocate_buffers(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
	int i;
	int proto; 
	uint8_t *xbuf;

    /* 
	 * Note: on NetAccel copies UDP & RTP headers along with the each video payload 
     * This allows applications do any RTP speoific processing (e.g FEC). 
     * In some instances, the payload size can vary from packet to packet (upto 1-7 MPEG2 TS packets)
     * For this reason, UDP header is also copied along w/ RTP header. This allows apps to determine
     * per packet payload length (udp length field 
	 */

    /* Calculate the protocol header length: Add RTP Header Size for RTP Protocol */
    if (ipCfg->doSockAccel)
        (ipCfg->proto == eProtoUDP) ? (ipCtx->hdrSize = UDP_HDR_SIZE) : (ipCtx->hdrSize = UDP_HDR_SIZE + RTP_HDR_SIZE);
    else {
        /* standard linux network stack doesn't provide access to UDP header */
        (ipCfg->proto == eProtoUDP) ? (ipCtx->hdrSize = 0) : (ipCtx->hdrSize = RTP_HDR_SIZE);
        ipCfg->useCpuCopy = 1;
    }

	/* if user didn't specify per packet size, calculate it */
    if (!ipCfg->pktSize)
        ipCfg->pktSize = ipCtx->hdrSize + (7*ipCfg->tsSize);

	/* and use that to determine buffer needed to hold pktsPerRead worth of data */
    ipCtx->bufSize = ipCfg->pktSize * ipCfg->pktsPerRead;
    printf("Options used are --> proto %d, hdrSize = %d, bufSize %d, writeToDisk %d, pktsPerRead %d, using %s socket, recv copy using %s\n", 
            ipCfg->proto, ipCtx->hdrSize, ipCtx->bufSize, ipCfg->writeToDisk, ipCfg->pktsPerRead, ipCfg->doSockAccel ? "Accelerated" : "Standard", ipCfg->useCpuCopy ? "CPU" : "M2M DMA");

    /* allocate receive buffer */
    if ( (xbuf = malloc(ipCtx->bufSize + ALIGN_4096)) == NULL ) {
        perror("malloc failure():");
        goto exit;
    }
    ipCtx->buf = (uint8_t *)(((unsigned long)xbuf + ALIGN_4096) & ~ALIGN_4096);
    memset(xbuf, 0, (ipCtx->bufSize + ALIGN_4096));
    printf("Successfully malloced (addr = 0x%x), pkts per read %d\n", ipCtx->buf, ipCfg->pktsPerRead);

    /* setup the vector arrary: each vector corresponds to a pkt */
    if ( (ipCtx->vector = malloc(sizeof(struct iovec) * ipCfg->pktsPerRead)) == NULL ) {
        perror("malloc failure():");
        goto exit;
    }
    for (i=0; i < ipCfg->pktsPerRead; i++) {
        ipCtx->vector[i].iov_base = ipCtx->buf + i*ipCfg->pktSize;
        ipCtx->vector[i].iov_len = ipCfg->pktSize;
    }
	return 0;

exit:
	return -1;
}

int setup_ip_socket(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
	int sfd;
	int ret;
	struct addrinfo  hints;
	struct addrinfo *addrInfo;
	char hostbuf[MAXADDRNAME];
	char serverbuf[NI_MAXSERV];
	unsigned char sockaddr_storage[MAXADDRNAME];
	struct sockaddr *saddr;
	socklen_t saddrLen;
	struct ip_mreq mreq;
	struct ipv6_mreq ipv6mreq;
	STRM_SockRecvParams_t sockRecvParams = STRM_SOCK_RECV_PARAMS_UDP_DEFAULT;


    /* prep up fields for the getaddrinfo() */
    memset(&hints, 0, sizeof(hints));

    /* set socket family */
    (ipCfg->iphVersion == 4) ? (hints.ai_family = AF_INET) : (hints.ai_family = AF_INET6);
	hints.ai_protocol = IPPROTO_UDP;

    /* set socket type */
    /* set it to standard socktype as getaddrinfo doesn't understand our new sock type */
    /* after getaddrinfo() call, we change the type to new Broadcom accelerated socket type */
	hints.ai_socktype = SOCK_DGRAM;
    if ((ret = getaddrinfo(ipCfg->host, ipCfg->service, &hints, &addrInfo)) != 0) {
        printf("getaddrinfo() Failed: error code = %d\n", ret);
        goto exit;
    }

    /* now update the sock type to our new type */
    /* this is a hack until we update the uclibc to understand the new socket type */
    if (ipCfg->doSockAccel)
            addrInfo->ai_socktype = SOCK_BRCM_DGRAM;

    saddrLen = MAXADDRNAME;
    saddr = (struct sockaddr *)sockaddr_storage;

    if ( ipCfg->iphVersion == 6 && ((struct sockaddr_in6 *)addrInfo->ai_addr)->sin6_scope_id == 0) {
        printf("IPv6 scope-ID is not set\n");
		((struct sockaddr_in6 *)addrInfo->ai_addr)->sin6_scope_id = 1;
    }

    printf("IP Address Info:\n"
            "   family       = %2d (PF_INET = %d, PF_INET6 = %d)\n"
            "   socktype     = %2d (SOCK_DRAM = %d, SOCK_BRCM_DGRAM %d, SOCK_STREAM = %d, SOCK_BRCM_STREAM %d)\n"
            "   protocol     = %2d (IPPROTO_UDP = %d, IPPROTO_TCP %d)\n"
            "   addrlen      = %2d\n"
            "   flags        = %d\n",
            addrInfo->ai_family,
            PF_INET, PF_INET6,
            addrInfo->ai_socktype,
            SOCK_DGRAM, SOCK_BRCM_DGRAM,
            SOCK_STREAM, SOCK_BRCM_STREAM,
            addrInfo->ai_protocol,
            IPPROTO_UDP, IPPROTO_TCP,
            addrInfo->ai_addrlen,
            addrInfo->ai_flags);

    getnameinfo(addrInfo->ai_addr, addrInfo->ai_addrlen, hostbuf, sizeof(hostbuf),
                serverbuf, sizeof(serverbuf), NI_NUMERICHOST | NI_NUMERICSERV);

    printf("   IP Addr      = %s\n" "   Port         = %s\n", hostbuf, serverbuf);
    
    /* Create Datagram socket */
    if ( (sfd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol)) < 0 ) {
        perror("socket():");
        goto exit;
    }
    printf("Socket Created\n");
	ipCtx->sfd = sfd;

    /* Bind to the Receiving IP Address & Port */
    if (bind(sfd, addrInfo->ai_addr, addrInfo->ai_addrlen) < 0) {
        perror("bind():");
        goto exit;
    }
    printf("Socket bind done\n");

    /* Note: */
    /* Accelerated sockets interface allows apps to receive multiple packets in one recvfrom call */
    /* Apps specify a maximum number of packets they want to receive per call and provide big enough */
    /* space to accomodate that many packets. Driver then waits upto a configurable timeout interval */
    /* to receive these packets and returns however many packets are received in that interval. */
    /* To find out how many packets are received, apps will need to use the total bytes recevied */
    /* and UDP header length of each packet to deterimine how many packets are received. */
    /* Apps can either receive packets starting from UDP header or after UDP header. The former */
    /* helps with the case where each UDP packet can be of variable length (using the UDP header total length) */
    /* Also, apps can set an option to either copy multiple packets back to back or copy each packet */
    /* at a fixed offset */

    /* set the number of pkts to wait before returning from recvfrom */
    if (ipCfg->doSockAccel) {
        /* maximum # of packets to receive in one recvfrom call */
        sockRecvParams.pktsPerRecv = ipCfg->pktsPerRead;    /* maximum # of packets to receive in one recvfrom call */

		/* copy packets back to back (instead of each packet begining at a fixed offset of say 1500 bytes) */
        sockRecvParams.pktOffset = 0;

		/* received data starts from UDP header. To skip it, set the hdrOffset to the udp header size */
        sockRecvParams.hdrOffset = 0;
        /* driver waits for 30msec for pktsPerRead # of pkts to arrive */
        /* returns either EAGAIN if nothing is received in this interval */
        /* or upto pktsPerRead if that many packets are received */
        sockRecvParams.recvTimeout = 800;

        /* CPU Vs. M2M DMA Copy */
        sockRecvParams.useCpuCopy = ipCfg->useCpuCopy;  
        if (setsockopt(sfd, SOCK_BRCM_DGRAM, STRM_SOCK_RECV_PARAMS, &sockRecvParams, sizeof(sockRecvParams)) != 0) {
            perror("setsockopt():recv options:");
            goto exit;
        }
        printf("Modified the default pkts per recvfrom to %d, pkt offset to %d\n", sockRecvParams.pktsPerRecv, sockRecvParams.pktOffset);
    }
	else {
        /* increase the receive buffer size for UDP socket, network 
           stack will drop pkts at >14Mbps rate */
        int size = 550000;
        if (setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, &size, 4) != 0) {
            perror("setsockopt():");
        	goto exit;
        }
        printf("Increased the Socket RCVBUF size to %d bytes\n", size);
    }

    /* Join the Multicast Group */
	if (ipCfg->iphVersion == 4) {
		if (IN_MULTICAST(ntohl(((struct sockaddr_in *)(addrInfo->ai_addr))->sin_addr.s_addr))) {
            struct ifreq ifr;
        	printf("Joining IPv4 Multicast Address Group\n");
    		mreq.imr_multiaddr.s_addr = ((struct sockaddr_in *)(addrInfo->ai_addr))->sin_addr.s_addr;
            if (ipCfg->interfaceName) {
                strncpy(ifr.ifr_name, ipCfg->interfaceName, IFNAMSIZ);
                if (ioctl(sfd, SIOCGIFADDR, &ifr) != 0) {
                    printf("ERROR: failed to get the IP Address for Interface name %s, errno %d\n", ipCfg->interfaceName, errno);
                    goto exit;
                }
    		    mreq.imr_interface.s_addr = ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr;
            }
            else {
    		    mreq.imr_interface.s_addr = INADDR_ANY;
            }
        	if (setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0) {
            	perror("setsockopt():");
        		goto exit;
        	}
    	}
	}
	else {
		/* IPv6 case */
		if ( IN6_IS_ADDR_MULTICAST(((struct sockaddr_in6 *)(addrInfo->ai_addr))->sin6_addr.s6_addr)) {
        	printf("Joining IPv6 Multicast Address Group\n");
			ipv6mreq.ipv6mr_multiaddr = ((struct sockaddr_in6 *)(addrInfo->ai_addr))->sin6_addr;
			ipv6mreq.ipv6mr_interface = 0;	/* chooses default interface */
        	if (setsockopt(sfd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &ipv6mreq, sizeof(ipv6mreq)) < 0) {
            	perror("setsockopt():");
        		goto exit;
        	}
		}
	}
	return 0;

exit:
	return -1;
}

int recv_av_data_from_socket(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
	int i;
	int ret;
	uint8_t *ptr;
	fd_set rfds;
	struct timeval tv;
	socklen_t saddrLen;
	int nwritten;
	int sfd = ipCtx->sfd;

	FD_ZERO(&rfds);
	FD_SET(sfd, &rfds);
	tv.tv_sec = ipCfg->timeout;
	tv.tv_usec = 0;

	if ( (ret = select(sfd +1, &rfds, NULL, NULL, &tv)) < 0 ) {
		perror("select(): exiting...");
		goto exit;
	}

	if (!FD_ISSET(sfd, &rfds)) {
		printf("**** No Input From Server, EXITING.... \n");
		goto exit;
	}

	/* Use recvfrom() to receive the RTP header (if present) + payload */
	if (ipCfg->doSockAccel) {
		if ( (ipCtx->bytesRecv = recvfrom(sfd, ipCtx->buf, ipCtx->bufSize, 0, &ipCtx->saddr, &saddrLen)) < 0) {
			perror("recvfrom():");
			goto exit;
		}
	}
	else {
		/* need to make multiple recvfrom calls to read multiple packets */
		ptr = ipCtx->buf;
		for (i=0; i < ipCfg->pktsPerRead; i++) {
			if ( (ipCtx->bytesRecv = recvfrom(sfd, ptr, ipCfg->pktSize, 0, &ipCtx->saddr, &saddrLen)) < 0) {
				perror("recvfrom():");
				goto exit;
			}
			ipCtx->vector[i].iov_base = ptr;
			ipCtx->vector[i].iov_len = ipCtx->bytesRecv;
			ptr += ipCfg->pktSize;
			/*printf("read %d in 0x%x, ", ipCtx->bytesRecv, ipCtx->vector[i].iov_base); */
		}
		/*printf("\n");*/
	}
	return 0;

exit:
	return -1;
}

void process_ip_pkts(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
	int i;
	int num = 0;
	uint8_t *ptr;
	uint16_t seq =0;
	static uint16_t expectedSeq = 0;
	uint16_t udpLen = 0;
	struct udphdr *udp;
	struct rtphdr *rtp;
	static int firstTime = 1;

	/* Adjust payload pointers & optionally verify RTP Sequence */
	ptr = ipCtx->buf;
	ipCtx->bytesToFeed = 0;
	for (i=0; i< ipCfg->pktsPerRead; i++) {
		num++;
		expectedSeq++;
		if (ipCfg->doSockAccel) {
			udp = (struct udphdr *)ptr;
			udpLen = ntohs(udp->len);
            if (udpLen > 1500)
                printf("ERROR: Invalid UDP header %d\n", udpLen);
			rtp = (rtphdr *) (ptr + sizeof(struct udphdr));
			seq = ntohs(rtp->sequenceNum);
		}
		else {
			/* standard socket recvfrom: kernel returns only one pkt starting at RTP header */
			udpLen = ipCtx->vector[i].iov_len;
			rtp = (rtphdr *) ptr;
			seq = ntohs(rtp->sequenceNum);
		}

		/* adjust the payload pointer */
		ipCtx->vector[i].iov_base = ptr + ipCtx->hdrSize;
		ipCtx->vector[i].iov_len = udpLen - ipCtx->hdrSize;
		ipCtx->bytesToFeed += udpLen - ipCtx->hdrSize;
		if (ipCfg->proto == eProtoRTP) {
			if (!firstTime) {
				if ( seq != expectedSeq ) {
					printf("********* Break in RTP Seq (expected seq %u, current seq %u, pktnum %d ***********\n", expectedSeq-1, seq, num);
					printRtp(rtp);
					expectedSeq = seq;
					/* goto exit; */
				}
			}
			else {
				expectedSeq = ntohs(rtp->sequenceNum);
				firstTime = 0;
				printf("########## Initial RTP Seq %d, pktsize %d, pktnum %d ###########\n", expectedSeq, udpLen, num);
				printRtp(rtp);
			}
		} /* if RTP */

		/* next pkt */
		/*TODO ptr += udpLen;*/
		/* */ptr += ipCfg->pktSize;
	}

	update_stats(ipCtx, ipCfg);
}

int record_av_data(ExampleIpCtx *ipCtx, ExampleIpCfg *ipCfg)
{
	int nwritten;

	if (ipCfg->proto == eProtoUDP && !ipCfg->doSockAccel) {
		/* Plain UDP, do a block write */
		if ( (nwritten = write(ipCtx->wfd, ipCtx->buf, ipCfg->pktsPerRead * ipCtx->bytesRecv)) < 0 ) {
			perror("write():");
			goto exit;
		}
		if (nwritten != ipCtx->bytesRecv * ipCfg->pktsPerRead) {
			printf("Write Error: only wrote %u our of %u received bytes\n", nwritten, ipCtx->bytesRecv);
			goto exit;
		}
	}
	else {
		/* RTP + UDP, use vectored write to skip the RTP header */
		if ( (nwritten = writev(ipCtx->wfd, ipCtx->vector, ipCfg->pktsPerRead)) < 0 ) {
			perror("write():");
			goto exit;
		}
	}
	return 0;
exit:
	return -1;
}

int parse_options(int argc, char *argv[], ExampleIpCfg *ipCfg)
{
	int c;
    while ( (c = getopt(argc, argv, "d:p:f:z:s:v:i:wn:t:ach")) != -1) {
        switch (c) {
        case 'd':
            ipCfg->host = optarg;
            break;
        case 'p':
            ipCfg->service = optarg;
            break;
        case 'f':
            ipCfg->filename = optarg;
            break;
        case 'v':
            ipCfg->iphVersion = atoi(optarg);
            if (ipCfg->iphVersion != 4 && ipCfg->iphVersion != 6) {
                printf("Incorrect IP Version (%d)\n", ipCfg->iphVersion);
                goto usage;
            }
            break;
        case 't':
            ipCfg->proto = atoi(optarg);
            if (ipCfg->proto > eProtoMAX) {
                printf("Incorrect Protocol Option (%d)\n", ipCfg->proto);
                goto usage;
            }
            break;
        case 's':
            ipCfg->tsSize = atoi(optarg);
            if (ipCfg->tsSize != 188 && ipCfg->tsSize != 192 && ipCfg->tsSize != 130) {
                printf("Not supported transport Size (%d)\n", ipCfg->tsSize);
                goto usage;
            }
            break;
        case 'w':
            ipCfg->writeToDisk = 1;
            break;
        case 'i':
            ipCfg->interfaceName = optarg;
            break;
        case 'n':
            ipCfg->pktsPerRead = atoi(optarg);
            if (ipCfg->pktsPerRead > 128) {
                printf("Incorrect Pkts Per Read (%d)\n", ipCfg->pktsPerRead);
                goto usage;
            }
            break;
        case 'z':
            ipCfg->pktSize = atoi(optarg);
            if (ipCfg->pktSize > 2000) {
                printf("Per UDP Datagram size %d is too large?\n", ipCfg->pktSize);
                goto usage;
            }
            break;
        case 'a':
            ipCfg->doSockAccel = 1;
            break;
        case 'c':
            ipCfg->useCpuCopy = 1;
            break;
        case 'h':
            usage();
            exit(0);
        default:
usage:
            usage();
            return -1;
        }
    }
	if (ipCfg->host == NULL || ipCfg->service == NULL) {
		usage();
		return -1;
	}
	return 0;
}

ExampleIpCtx ipCtx;
ExampleIpCfg ipCfg;
int main(int argc, char *argv[])
{

	/* set the IP socket related defults for program */
	set_default_ip_cfg(&ipCtx, &ipCfg);

	/* parse command line options */
	if (parse_options(argc, argv, &ipCfg)) {
		printf("ERROR: Incorrect Options\n");
		goto exit;
	}

	/* allocate buffer to receive UDP/RTP Headers + Payload from Network */
	if (allocate_buffers(&ipCtx, &ipCfg)) {
		printf("ERROR: Failed to Allocate Memory\n");
		goto exit;
	}
    
	/* create the socket to receive AV Data over UDP socket */
	if (setup_ip_socket(&ipCtx, &ipCfg)) {
		printf("ERROR: Failed to Setup the socket\n");
		goto exit;
	}

    /* Open the temp file where received content is written to */
    if (ipCfg.writeToDisk && (ipCtx.wfd = open(ipCfg.filename, O_RDWR|O_CREAT|O_TRUNC)) < 0) {
        perror("open():");
        goto exit;
    }
    printf("Ready to Receive Data...\n");

    while (1) {
		/* read av data from the socket */
		if (recv_av_data_from_socket(&ipCtx, &ipCfg))
			goto exit;

		/* process IP packets: verify RTP Header (optionally decrypt AV payload) */
		process_ip_pkts(&ipCtx, &ipCfg);

        /* now write it to disk either using write or writev*/
        if (ipCfg.writeToDisk) {
			if (record_av_data(&ipCtx, &ipCfg))
				goto exit;
        }

    } /* while (1) */

exit:
    close(ipCtx.sfd);
    return 0;
}
