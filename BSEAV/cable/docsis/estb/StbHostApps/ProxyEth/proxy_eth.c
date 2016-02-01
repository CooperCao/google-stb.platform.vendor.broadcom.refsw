/******************************************************************************
 *    (c)2010-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <ctype.h>
#include <linux/types.h>
#include <asm/types.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/time.h>

typedef unsigned char uint8_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long long u64;

#define IF_DOWN 0
#define IF_UP 1
#define IF_INVALID -1

#define ETHTOOL_FWVERS_LEN	32
#define ETHTOOL_BUSINFO_LEN	32

#ifndef SIOCETHTOOL
#define SIOCETHTOOL     0x8946
#endif

#define ETHTOOL_GDRVINFO	0x00000003 /* Get driver info. */
#define ETH_GSTRING_LEN		32
#define ETHTOOL_GSTRINGS	0x0000001b /* get specified string set */
#define ETHTOOL_GSTATS		0x0000001d /* get NIC-specific statistics */

//default proxy interface "eth0"
char proxy_dev[8] = "eth0";
char other_eth_dev[8] = "eth1";

enum ethtool_stringset {
	ETH_SS_TEST		= 0,
	ETH_SS_STATS,
	ETH_SS_PRIV_FLAGS,
	ETH_SS_NTUPLE_FILTERS,
};

/* these strings are set to whatever the driver author decides... */
struct ethtool_drvinfo {
  __u32	cmd;
  char	driver[32];	/* driver short name, "tulip", "eepro100" */
  char	version[32];	/* driver version string */
  char	fw_version[ETHTOOL_FWVERS_LEN];	/* firmware version string */
  char	bus_info[ETHTOOL_BUSINFO_LEN];	/* Bus info for this IF. */
  /* For PCI devices, use pci_name(pci_dev). */
  char	reserved1[32];
  char	reserved2[12];
  /*
   * Some struct members below are filled in
   * using ops->get_sset_count().  Obtaining
   * this info from ethtool_drvinfo is now
   * deprecated; Use ETHTOOL_GSSET_INFO
   * instead.
   */
  __u32	n_priv_flags;	/* number of flags valid in ETHTOOL_GPFLAGS */
  __u32	n_stats;	/* number of u64's from ETHTOOL_GSTATS */
  __u32	testinfo_len;
  __u32	eedump_len;	/* Size of data from ETHTOOL_GEEPROM (bytes) */
  __u32	regdump_len;	/* Size of data from ETHTOOL_GREGS (bytes) */
};

/* for passing string sets for data tagging */
struct ethtool_gstrings {
	__u32	cmd;		/* ETHTOOL_GSTRINGS */
	__u32	string_set;	/* string set id e.c. ETH_SS_TEST, etc*/
	__u32	len;		/* number of strings in the string set */
	__u8	data[0];
};

/* for dumping NIC-specific statistics */
struct ethtool_stats {
	__u32	cmd;		/* ETHTOOL_GSTATS */
	__u32	n_stats;	/* number of u64's being returned */
	__u64	data[0];
};

#define BRPC_RMAGNUM_MAX_MSG_LEGNTH 1472
#define BRPC_RMAGNUM_SERVER_IP_ADDR_STR	 "192.168.17.1"
#define BRPC_RMAGNUM_SERVER_IP_ADDR	 0xC0A81101
#define BRPC_RMAGNUM_LOCAL_IP_ADDR_STR	 "192.168.17.10"
#define BRPC_RMAGNUM_LOCAL_IP_ADDR	 0xC0A8110A
#define RMAGNUM_REQUEST_PORT 0xBEEF
#define RMAGNUM_PROXY_ENET_REQUEST_PORT      (RMAGNUM_REQUEST_PORT + 4)
#define RMAGNUM_PROXY_ENET_NOTIFICATION_PORT (RMAGNUM_REQUEST_PORT + 5)
#define BRPC_MAGIC_CODE 0xDEADBEEF
#define SOCKET_BUF_LEN 200

typedef enum BRPC_IfMib_OId
{
    BRPC_IfMib_OId_Speed = 5,
    BRPC_IfMib_OId_OperStatus = 7,
    BRPC_IfMib_OId_InDiscards = 13,
    BRPC_IfMib_OId_InErrors = 14,
    BRPC_IfMib_OId_OutErrors = 20,
    BRPC_OID_DOT3ALIGNMENTERRORS = 0x32,   /* dot3StatsAlignmentErrors - RO 32 bit counter*/
    BRPC_OID_DOT3FCSERRORS,              /* dot3StatsFCSErrors - RO 32 bit counter*/
    BRPC_OID_DOT3SINGLECOLLFRAMES,       /* dot3StatsSingleCollisionFrames - RO 32 bit counter */
    BRPC_OID_DOT3MULTCOLLFRAMES,         /* dot3StatsMultipleCollisionFrames - RO 32 bit counter */
    BRPC_OID_DOT3SQETESTERRORS,          /* dot3StatsSQETestErrors - RO 32 bit counter */
    BRPC_OID_DOT3DEFERREDTXS,            /* dot3StatsDeferredTransmissions - RO 32 bit counter */
    BRPC_OID_DOT3LATECOLLS,              /* dot3StatsLateCollisions  - RO 32 bit counter*/
    BRPC_OID_DOT3EXCESSIVECOLLS,         /* dot3StatsExcessiveCollisions - RO 32 bit counter*/
    BRPC_OID_DOT3INTERNALMACTXERRS,      /* dot3StatsInternalMacTransmitErrors - RO 32 bit counter */
    BRPC_OID_DOT3CARRIERSENSEERRS,       /* dot3StatsCarrierSenseErrors - RO 32 bit counter */
    BRPC_OID_DOT3FRAMETOOLONGS,          /* dot3StatsFrameTooLongs - RO 32 bit counter */
    BRPC_OID_DOT3INTERNALMACRXERRS = 0x40,      /* dot3StatsInternalMacReceiveErrors - RO 32 bit counter */
    //MIB_OID_DOT3ETHERCHIPSET,         /* dot3StatsEtherChipSet - RO OID */
    BRPC_OID_DOT3SYMBOLERRS = 0x42,             /* dot3StatsSymbolErrors - RO 32 bit counter */
    BRPC_OID_DOT3DUPLEXSTATUS,
    //BRPC_OID_DOT3COLLCOUNT = 0x51, /* dot3CollCount - RO integer*/
    //BRPC_OID_DOT3COLLFREQUENCIES  /* dot3CollFrequencies - RO 32 bit counter*/
    BRPC_Get_All_Address = 0x60
} BRPC_IfMib_OId;

struct ethdot3stats
{
  BRPC_IfMib_OId oid;
  char ethtool_gstr[ETH_GSTRING_LEN];
};

typedef struct BPRC_Ntf_Msg
{
    uint32_t magicCode;
    uint32_t event;
} BRPC_Ntf_Msg;

typedef struct BPRC_Req_Msg
{
    uint32_t magicCode;
    uint32_t reqId;
    uint32_t param[3];
} BRPC_Req_Msg;

struct ethdot3stats dot3statsmap[] =
{
   { BRPC_OID_DOT3ALIGNMENTERRORS, "rx_align" },
   { BRPC_OID_DOT3FCSERRORS, "rx_fcs" },
   { BRPC_OID_DOT3SINGLECOLLFRAMES, "tx_single_col" },
   { BRPC_OID_DOT3MULTCOLLFRAMES, "tx_multi_col" },
   //{ BRPC_OID_DOT3SQETESTERRORS, "not support" }, valid for < 10Mbps interface, always return 0
   { BRPC_OID_DOT3DEFERREDTXS, "tx_defer" },
   { BRPC_OID_DOT3LATECOLLS, "tx_late_col" },
   { BRPC_OID_DOT3EXCESSIVECOLLS, "tx_excess_col" },
   //{ BRPC_OID_DOT3INTERNALMACTXERRS, "not supported" }, can't find it, always return 0
   //{ BRPC_OID_DOT3CARRIERSENSEERRS, "not supported" }, read it from sysfs
   { BRPC_OID_DOT3FRAMETOOLONGS, "rx_mtu_err" }
   //{ BRPC_OID_DOT3INTERNALMACRXERRS, "not supported" }, can't find it, always return 0
   //{ BRPC_OID_DOT3SYMBOLERRS, "not supported" }, can't find it, always return 0
   //{ BRPC_OID_DOT3DUPLEXSTATUS,"not supported" } read it from sysfs
   //{ BRPC_OID_DOT3COLLCOUNT, "" },
   //{ BRPC_OID_DOT3COLLFREQUENCIES, "" }
};

#define BRPC_GET_NOTIFICATION_REVERSE_RMAGNUM_EVENT_PROXYENET_MIB_OID(x)    (x&0xFF)
#define BRPC_GET_NOTIFICATION_REVERSE_RMAGNUM_EVENT_PROXYENET_METHOD(x)     ((x&0xFF00)>>8)
#define BRPC_GET_NOTIFICATION_REVERSE_RMAGNUM_EVENT_PROXYENET_VALUE(x)      ((x&0xFF0000)>>16)

static int32_t BRPC_P_UdpReadTimeout( int32_t fd, uint8_t *pBuf, uint32_t buf_size, int32_t timeoutMS)
{
	int32_t maxfdp1;
	fd_set rfds;
	int32_t retcode;
    register int32_t cnt = -1;
	struct sockaddr_in frominet;
	int32_t                inetsize;
	struct timeval tv;
#ifdef RPC_SOCKET_DEBUG
	uint32_t i;
#endif

	maxfdp1 = fd + 1;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	if(timeoutMS < 0)
	{
		retcode = select(maxfdp1, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);
	}
	else
	{
		tv.tv_sec = timeoutMS / 1000;
		tv.tv_usec = (timeoutMS - (1000 * tv.tv_sec)) * 1000;
		retcode = select(maxfdp1, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)&tv);
	}

	if (retcode < 0)
	{
		printf("select return error: %d\n", retcode);
	}
	else if (retcode == 0)
	{
		if(timeoutMS != 0)
		{
			printf("select(): timeout!\n");
		}
	}
	else
	{
		if(FD_ISSET(fd, &rfds))
		{
			inetsize = sizeof(frominet);
			cnt = recvfrom(fd, pBuf, buf_size, 0, (struct sockaddr*)&frominet, (socklen_t*) &inetsize);
			if( cnt <= 0 )
			{
				printf("recvfrom error. \n");
			}
		}
		retcode = cnt;
	}

	return retcode;
}

static int32_t BRPC_P_UdpWrite( int32_t fd, uint8_t *pBuf, uint32_t tpkt_size, uint32_t ipaddr, uint16_t port)
{
	struct sockaddr_in sin_dst;
    int32_t cnt;

	/* configure the destionation addr */
	bzero((int8_t*)&sin_dst, sizeof(sin_dst));
    sin_dst.sin_family = AF_INET;
    sin_dst.sin_port = htons(port);
	sin_dst.sin_addr.s_addr = htonl(ipaddr);

	/* send the packet */
    cnt=sendto(fd, pBuf, tpkt_size, 0, (struct sockaddr*)&sin_dst, sizeof(sin_dst));

    /* printf("send to %x:%x %d bytes, data: %x, %x, %x \n", ipaddr, port, tpkt_size, *((uint32_t *)pBuf), *(((uint32_t *)pBuf+1)), *(((uint32_t *)pBuf+2))); */

	if( cnt != (int32_t)tpkt_size )
	{
		printf("\nError: sendto returns %d. \n", cnt);
		return cnt;
	}
	return 0;
}

int GetEth0Info(BRPC_IfMib_OId oid, unsigned *value)
{
    char buf[100];
    char *dir;
    FILE *f;
    int v;

    switch(oid)
    {
    case BRPC_IfMib_OId_Speed:
        dir = "/sys/class/net/eth0/speed";
        break;
    case BRPC_IfMib_OId_OperStatus:
        dir = "/sys/class/net/eth0/operstate";
        break;
    case BRPC_IfMib_OId_InDiscards:
        dir = "/sys/class/net/eth0/statistics/rx_dropped";
        break;
    case BRPC_IfMib_OId_InErrors:
        dir = "/sys/class/net/eth0/statistics/rx_errors";
        break;
    case BRPC_IfMib_OId_OutErrors:
        dir = "/sys/class/net/eth0/statistics/tx_errors";
        break;
      case BRPC_OID_DOT3CARRIERSENSEERRS:
        dir = "/sys/class/net/eth0/statistics/tx_carrier_errors";
        break;
      case BRPC_OID_DOT3DUPLEXSTATUS:
        dir = "/sys/class/net/eth0/duplex";
        break;
      default:
        printf("Invalid oid %d\n", oid);
        return -1;
    }

    f = fopen(dir, "r");

    if (!f)
    {
        printf("Open %s fail.\n", dir);
        return -1;
    }

    if (fgets(buf, 100, f))
    {
        buf[99] = 0;

        switch(oid)
        {
        case BRPC_IfMib_OId_OperStatus:
            if(buf[0] == 'u') /* up */
                v = 1;
            else if(buf[0] == 'd') /* down */
                v = 2;
            else
                printf("Invalid OperState %s\n", buf);
            break;

        case BRPC_OID_DOT3DUPLEXSTATUS:
            if(buf[0] == 'f') /* up */
                v = 3;
            else if(buf[0] == 'h') /* down */
                v = 2;
            else
                printf("Invalid dumplex mode%s\n", buf);

        default:
            sscanf(buf, "%d", &v);
            break;
        }

        /*printf("Read from %s is: %s|%d \n", dir, buf, v);*/
    }
    else
    {
        printf("Read %s fail.\n", dir);
        return -1;
    }

    if(oid == BRPC_IfMib_OId_Speed)
        v *= 1000000;

    fclose(f);
    *value = v;

    return 0;
}

static int send_ioctl(int fd, struct ifreq *ifr)
{
	return ioctl(fd, SIOCETHTOOL, ifr);
}

static int GetEth0Dot3statsInfo(int fd, BRPC_IfMib_OId oid, unsigned *value)
{
  struct ifreq ifr;
  struct ifreq * pifr = &ifr;
  struct ethtool_drvinfo drvinfo;
  struct ethtool_gstrings *strings;
  struct ethtool_stats *stats;
  unsigned int n_stats, sz_str, sz_stats, i;
  int err;

  char *oid_gstring = NULL;

  for(i =0; i < sizeof(dot3statsmap)/sizeof(dot3statsmap[0]); i++){
    if( oid == dot3statsmap[i].oid){
      oid_gstring = dot3statsmap[i].ethtool_gstr;
      break;
    }
  }

  if(oid_gstring == NULL)
  {
    *value = 0;
    fprintf(stderr,"can't find it in gstats info\n");
    return -1;
  }

  /* Setup our control structures. */
  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name,proxy_dev);


  drvinfo.cmd = ETHTOOL_GDRVINFO;
  pifr->ifr_data = (caddr_t)&drvinfo;
  err = send_ioctl(fd, pifr);
  if (err < 0) {
    perror("Cannot get driver information");
    return 71;
  }

  n_stats = drvinfo.n_stats;
  if (n_stats < 1) {
    fprintf(stderr, "no stats available\n");
    return 94;
  }

  sz_str = n_stats * ETH_GSTRING_LEN;
  sz_stats = n_stats * sizeof(u64);

  strings = calloc(1, sz_str + sizeof(struct ethtool_gstrings));
  stats = calloc(1, sz_stats + sizeof(struct ethtool_stats));
  if (!strings || !stats) {
    fprintf(stderr, "no memory available\n");
    return 95;
  }

  strings->cmd = ETHTOOL_GSTRINGS;
  strings->string_set = ETH_SS_STATS;
  strings->len = n_stats;
  pifr->ifr_data = (caddr_t) strings;
  err = send_ioctl(fd, pifr);
  if (err < 0) {
    perror("Cannot get stats strings information");
    free(strings);
    free(stats);
    return 96;
  }

  stats->cmd = ETHTOOL_GSTATS;
  stats->n_stats = n_stats;
  pifr->ifr_data = (caddr_t) stats;
  err = send_ioctl(fd, pifr);
  if (err < 0) {
    perror("Cannot get stats information");
    free(strings);
    free(stats);
    return 97;
  }

  /* todo - pretty-print the strings per-driver */
  for (i = 0; i < n_stats; i++) {
    if(!strcmp(oid_gstring , (char *)&strings->data[i * ETH_GSTRING_LEN])){
      *value = stats->data[i];
      //fprintf(stdout, "match: strings->data[%d] %s\n",i, &strings->data[i * ETH_GSTRING_LEN], *value);
      break;
    }
  }

  if(i == (n_stats-1))
    fprintf(stderr, "run over,no item found\n");

  free(strings);
  free(stats);

  return 0;
}

int GetHwAddress(int iocsock, const char * devname, uint8_t *macaddr)
{
    struct ifreq ifr;

    /* Setup our control structures. */
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, devname, IFNAMSIZ-1);

    if (ioctl(iocsock, SIOCGIFHWADDR, &ifr) < 0) {
        perror("Cannot get mac address");
        return -1;
    }

    memcpy(macaddr, ifr.ifr_hwaddr.sa_data, 6);
    return 0;
}

int read_event (int sockint,unsigned int ifindex)
{
    int status;
    char buf[4096];
    struct iovec iov = { buf, sizeof buf };
    struct sockaddr_nl snl;
    struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
    struct nlmsghdr *h;
    struct ifinfomsg *ifi;

    status = recvmsg (sockint, &msg, 0);

    if (status < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return IF_INVALID;

        perror ("read_netlink: Error recvmsg: ");
        return IF_INVALID;
    }

    // We need to handle more than one message per 'recvmsg'
    for (h = (struct nlmsghdr *) buf; NLMSG_OK (h, (unsigned int) status);
            h = NLMSG_NEXT (h, status))
    {
        //Finish reading
        if (h->nlmsg_type == NLMSG_DONE)
            return IF_INVALID;

        // Message is some kind of error
        if (h->nlmsg_type == NLMSG_ERROR)
        {
            return IF_INVALID;        // Error
        }

        if (h->nlmsg_type == RTM_NEWLINK)
        {
            ifi = NLMSG_DATA (h);
            if(ifi->ifi_index == ifindex)
            {
                printf ("NETLINK::eth0 %s\n", (ifi->ifi_flags & IFF_RUNNING) ? "Up" : "Down");
                if(ifi->ifi_flags & IFF_RUNNING)
                    return IF_UP;
                else
                    return IF_DOWN;
            }
            return IF_INVALID;
        }
    }

    return IF_INVALID;
}

// genet driver may take 3 seconds to bring up eth0, and the link status may be changed as below in this process: up->down->up
// , this may cause cm to send 3 link status change traps, so wait it to become a stable state after bring it up....
int WaitLinkStatus(const char *ifname)
{
    fd_set rfds;
    struct timeval tv;
    int retval;
    struct sockaddr_nl addr;
    unsigned int ifindex;
    int linkstatus = IF_INVALID;
    int linkupcount = 0, linkdowncount = 0;

    int nl_socket = socket (AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (nl_socket < 0)
    {
        perror ("Socket Open Error!");
        return -1;
    }

    memset ((void *) &addr, 0, sizeof (addr));

    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid ();
    addr.nl_groups = RTMGRP_LINK;

    if (bind (nl_socket, (struct sockaddr *) &addr, sizeof (addr)) < 0)
    {
        perror ("Socket bind failed!");
        close(nl_socket);
        return -1;
    }

    ifindex = if_nametoindex(ifname);
    if(!ifindex)
    {
        perror("can't find such interface");
        close(nl_socket);
        return -1;
    }

    tv.tv_sec = 3;
    tv.tv_usec = 0;

    FD_ZERO (&rfds);
    FD_CLR (nl_socket, &rfds);
    FD_SET (nl_socket, &rfds);
    while ((retval = select (FD_SETSIZE, &rfds, NULL, NULL, &tv)))
    {
        if (retval == -1)
            perror ("Error select() \n");
        else if (retval)
        {
            //printf ("Event recieved >> ");
            linkstatus = read_event(nl_socket,ifindex);
            if(linkstatus == IF_UP)
                linkupcount++;
            else if(linkstatus == IF_DOWN)
                linkdowncount++;
        }
        else
            printf ("## Select TimedOut ## \n");

        //if((linkupcount == 2) && (linkdowncount == 1))
        if(linkupcount > 1)
        {
            printf ("complete this ifup action....\n");
            break;
        }
    }

    close(nl_socket);
    return 0;
}

int main(int argc, char **argv)
{
    int arg = 1, opt;
    int fReqSock, fNtfSock, fDot3IocSock;
    struct sockaddr_in sockaddr_param;

    uint8_t ntfRcvBuf[SOCKET_BUF_LEN];
    uint8_t reqSendBuf[SOCKET_BUF_LEN];

    BRPC_Ntf_Msg *pNtfMsg;
    uint32_t ntfMsgMagicCode;
    uint32_t ntfMsgEvent;

    BRPC_Req_Msg *pReqMsg = NULL;
    uint8_t *pchar = NULL;
    uint8_t ethmac[6] = {0,0,0,0,0,0};

    int32_t rc;

    uint8_t oid, method, value;
    uint32_t ethInfo;
    uint32_t operstatus = 2;

    while((opt=getopt(argc,argv,"i:"))!= -1){
      switch(opt){
        case 'i':
          strncpy(proxy_dev,optarg,sizeof(proxy_dev));
          break;
        default:
          fprintf(stderr,"usage: %s [-i interface]\n",argv[0]);
          exit(EXIT_FAILURE);
      }
    }

    printf("proxyd device:%s\n",proxy_dev);

    memset(ntfRcvBuf, 0, SOCKET_BUF_LEN);
    memset(reqSendBuf, 0, SOCKET_BUF_LEN);

    /* Creating sockets */
    if ((fNtfSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Failed to create fNtfSock. \n");
    }

    if ((fReqSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Failed to create fReqSock.\n");
    }

    /* Configure the address and port */
    memset( &sockaddr_param, 0, sizeof(sockaddr_param) );
    sockaddr_param.sin_addr.s_addr = htonl(BRPC_RMAGNUM_LOCAL_IP_ADDR);
    sockaddr_param.sin_port = htons(RMAGNUM_PROXY_ENET_NOTIFICATION_PORT);
    sockaddr_param.sin_family = AF_INET;

    /* bind to the notification port */
    if (bind(fNtfSock, (struct sockaddr *)&sockaddr_param, sizeof(sockaddr_param)) < 0)
    {
        printf("Failed Socket Bind to(fNtfSock): %s \n", BRPC_RMAGNUM_LOCAL_IP_ADDR_STR);
        goto err_ntf_bind;
    }

    memset( &sockaddr_param, 0, sizeof(sockaddr_param) );
    sockaddr_param.sin_addr.s_addr = htonl(BRPC_RMAGNUM_LOCAL_IP_ADDR);
    sockaddr_param.sin_port = htons(RMAGNUM_PROXY_ENET_REQUEST_PORT);
    sockaddr_param.sin_family = AF_INET;

    /* bind to the request port */
    if (bind(fReqSock, (struct sockaddr *)&sockaddr_param, sizeof(sockaddr_param)) < 0)
    {
        printf("Failed Socket Bind to(fReqSock): %s \n", BRPC_RMAGNUM_LOCAL_IP_ADDR_STR);
        goto err_req_bind;
    }

    /* Set the socket to be non-blocking. */
    ioctl(fNtfSock, FIONBIO, &arg);
    ioctl(fReqSock, FIONBIO, &arg);

    /* Open control socket. */
    fDot3IocSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (fDot3IocSock < 0) {
      perror("Cannot get control socket");
      goto err_ioc_sock;
    }

    while(1)
    {
        rc = BRPC_P_UdpReadTimeout(fNtfSock, ntfRcvBuf, SOCKET_BUF_LEN, 5000);

        /*if(rc > 0)
            printf("Received %d bytes. \n", rc);
        else
            printf("proxy_eth thread is running.\n");*/

        if(rc == sizeof(BRPC_Ntf_Msg))
        {
            pNtfMsg = (BRPC_Ntf_Msg *)ntfRcvBuf;
            ntfMsgMagicCode = htonl(pNtfMsg->magicCode);
            ntfMsgEvent = htonl(pNtfMsg->event);

            if(ntfMsgMagicCode == BRPC_MAGIC_CODE)
            {
                oid = BRPC_GET_NOTIFICATION_REVERSE_RMAGNUM_EVENT_PROXYENET_MIB_OID(ntfMsgEvent);
                method = BRPC_GET_NOTIFICATION_REVERSE_RMAGNUM_EVENT_PROXYENET_METHOD(ntfMsgEvent);
                value = BRPC_GET_NOTIFICATION_REVERSE_RMAGNUM_EVENT_PROXYENET_VALUE(ntfMsgEvent);

                //printf("magicCode: %x, oid: %d, method: %d, value: %d \n", ntfMsgMagicCode, oid, method, value);

                /* get */
                if(method == 0)
                {
                  pReqMsg = (BRPC_Req_Msg *)reqSendBuf;
                  pReqMsg->magicCode = htonl(BRPC_MAGIC_CODE);
                  pReqMsg->reqId = htonl(oid);

                  if(oid == BRPC_OID_DOT3SQETESTERRORS || oid == BRPC_OID_DOT3INTERNALMACTXERRS || \
                      oid == BRPC_OID_DOT3INTERNALMACRXERRS || oid == BRPC_OID_DOT3SYMBOLERRS){
                    ethInfo = 0;
                    pReqMsg->param[0] = htonl(ethInfo);
                  }
                  else if(((BRPC_IfMib_OId_Speed <= oid) && (oid <= BRPC_IfMib_OId_OutErrors)) || \
                       oid == BRPC_OID_DOT3CARRIERSENSEERRS || oid == BRPC_OID_DOT3DUPLEXSTATUS){
                    GetEth0Info(oid, &ethInfo);
                    pReqMsg->param[0] = htonl(ethInfo);
                  }
                  else if((BRPC_OID_DOT3ALIGNMENTERRORS <= oid) && (oid < BRPC_OID_DOT3DUPLEXSTATUS)){
                    GetEth0Dot3statsInfo(fDot3IocSock, oid, &ethInfo);
                    pReqMsg->param[0] = htonl(ethInfo);
                  }
                  else if(BRPC_Get_All_Address == oid){
                    pchar = (uint8_t *)(&pReqMsg->param[0]);

                    GetHwAddress(fDot3IocSock,proxy_dev,ethmac);
                    memcpy(pchar, ethmac, 6);

                    memset(ethmac, 0, sizeof(ethmac));
                    GetHwAddress(fDot3IocSock,other_eth_dev,ethmac);
                    memcpy(pchar + 6, ethmac, 6);
                    //printf("maclist:%02x%2x%2x%2x%2x%2x %02x%2x%2x%2x%2x%2x\n", \
                            *pchar, *(pchar+1), *(pchar+2),*(pchar+3), *(pchar+4), *(pchar+5),\
                            *(pchar+6), *(pchar+7), *(pchar+8), *(pchar+9), *(pchar+10), *(pchar+11));
                  }
                  else
                    printf("Invalid oid %d\n", oid);

                    BRPC_P_UdpWrite(fReqSock, reqSendBuf, sizeof(BRPC_Req_Msg), BRPC_RMAGNUM_SERVER_IP_ADDR, RMAGNUM_PROXY_ENET_REQUEST_PORT);
                }
                /* set */
                else if((oid == BRPC_IfMib_OId_OperStatus) && (method == 1)) /* set operStatus */
                {
                    if(value == 0)
                        system("ifconfig eth0 down");
                    else if(value == 1)
                    {
                        GetEth0Info(BRPC_IfMib_OId_OperStatus,&operstatus);
                        if(operstatus != 1)
                        {
                            system("ifconfig eth0 up");
                            //system("sleep 3");
                            WaitLinkStatus(proxy_dev);
                            printf("eth0 bring up finally............\n");
                        }
                    }
                }
            }
        }
    }

    close(fDot3IocSock);
    close(fReqSock);
    close(fNtfSock);

    return 0;

err_ioc_sock:
    close(fDot3IocSock);
err_req_bind:
    close(fReqSock);
err_ntf_bind:
  close(fNtfSock);
    return -1;
}
