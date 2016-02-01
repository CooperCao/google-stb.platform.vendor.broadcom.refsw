/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <net/if.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <pthread.h>

#include "ethernet.h"

#undef DEBUG
#ifdef DEBUG
#define DBGPRINT(X) fprintf X
#else
#define DBGPRINT(X)
#endif

#define MAX_NUM_DEV_IF 20
#define MAX_BUF_SIZE 4096

struct net_dev_info
{
	char iface[30];
	unsigned rxbytes;
	unsigned rxpackets;
	unsigned rxerrs;
	unsigned rxdrop;
	unsigned rxfifo;
	unsigned rxframe;
	unsigned rxcompressed;
	unsigned rxmulticast;
	unsigned txbytes;
	unsigned txpackets;
	unsigned txerrs;
	unsigned txdrop;
	unsigned txfifo;
	unsigned txcolls;
	unsigned txcarrier;
	unsigned txcompressed;
};

struct link_state_info
{
    pthread_t id;
    int ifindex[MAX_NUM_DEV_IF];
    unsigned int since[MAX_NUM_DEV_IF];
};

static struct link_state_info g_link_state_info;

static int get_net_dev_active_flags(struct ifreq *ifrq, const char* name)
{
    int fd = -1, rc = 0;

    if (name == NULL)
	{
        DBGPRINT((stderr, "invalid interface name\n"));
        return -1;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
	{
        DBGPRINT((stderr, "couldn't create socket\n"));
        return -2;
    }

    sprintf(ifrq->ifr_name, "%s", name);
    rc = ioctl(fd, SIOCGIFFLAGS, ifrq);
    if (rc < 0)
	{
        DBGPRINT((stderr, "ioctl SIOCGIFFLAGS returned %d", rc));
        rc = -3;
    }

    if (fd >= 0)
    {
        close(fd);
    }

    return rc;
}

static int set_net_dev_active_flags(int flags, int and_complement, const char* name)
{
	struct ifreq ifrq;
	int fd = -1, rc = 0;

	if (name == NULL)
	{
        DBGPRINT((stderr, "invalid interface name\n"));
	    return -1;
	}

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
	{
        DBGPRINT((stderr, "couldn't create socket\n"));
        return -2;
    }

    sprintf(ifrq.ifr_name, "%s", name);
	rc = ioctl(fd, SIOCGIFFLAGS, &ifrq);
	if(rc < 0)
	{
	    DBGPRINT((stderr, "error getting flags\n"));
	    close(fd);
	    return -3;
	}
	if (and_complement == 0)
	{
	    ifrq.ifr_flags |= flags;
	}
	else
	{
	    ifrq.ifr_flags &= ~flags;
	}

	rc = ioctl(fd, SIOCSIFFLAGS, &ifrq);
	if (rc < 0)
	{
	    close(fd);
	    DBGPRINT((stderr, "error setting flags\n"));
	    return -4;
	}

	if (fd >= 0)
	{
	    close(fd);
	}

	return 0;
}

static int get_net_dev_info(int id, struct net_dev_info * info)
{
	FILE *f;
	char line[256], *lineptr = line;
	size_t size;
	char iface[30];
    unsigned rxbytes, rxpackets, rxerrs, rxdrop, rxfifo, rxframe, rxcompressed, rxmulticast;
    unsigned txbytes, txpackets, txerrs, txdrop, txfifo, txcolls, txcarrier, txcompressed;
    int i;

    memset(info, 0, sizeof(*info));

    size = sizeof(line);

	if ((f = fopen("/proc/net/dev", "r")))
    {
        for (i = 0; i < id + 2; i++)
        {
            if (getline(&lineptr, &size, f) < 0)
            {
                fclose(f);
                return -1;
            }
        }

		if ((getline(&lineptr, &size, f) >= 0))
        {
            sscanf(lineptr, "%s %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u",
                      iface, &rxbytes, &rxpackets, &rxerrs, &rxdrop, &rxfifo, &rxframe, &rxcompressed, &rxmulticast,
                             &txbytes, &txpackets, &txerrs, &txdrop, &txfifo, &txcolls, &txcarrier, &txcompressed);

			snprintf(info->iface, strlen(iface), "%s", iface);
			info->rxbytes = rxbytes;
			info->rxpackets = rxpackets;
			info->rxerrs = rxerrs;
			info->rxdrop = rxdrop;
			info->rxfifo = rxfifo;
			info->rxframe = rxframe;
			info->rxcompressed = rxcompressed;
			info->rxmulticast = rxmulticast;
			info->txbytes = txbytes;
			info->txpackets = txpackets;
			info->txerrs = txerrs;
			info->txdrop = txdrop;
			info->txfifo = txfifo;
			info->txcolls = txcolls;
			info->txcarrier = txcarrier;
			info->txcompressed = txcompressed;
			fclose(f);
			return 0;
        }
		fclose(f);
    }

    return -1;
}

static unsigned int get_time_sec(void)
{
   static int error = 0;

   struct sysinfo info;

   if (!error)
   {
      error = sysinfo(&info);

      if (error)
         DBGPRINT((stdout, "WARNING: calling sysinfo, using time() as fallback\n"));
      else
         return((unsigned int)info.uptime);
   }

   if (error)
   {
      return((unsigned int)time(NULL));
   }

   return 0;
}

static int read_event(int sockint, struct link_state_info *info)
{
    int status;
    int id, ret = 0;
    char buf[MAX_BUF_SIZE], ifname[IF_NAMESIZE];
    struct iovec iov;
    struct sockaddr_nl snl;
    struct msghdr msg;
    struct nlmsghdr *h;
    struct ifinfomsg *ifi;

    iov.iov_base = (void *) buf;
    iov.iov_len = sizeof(buf);

    msg.msg_name = (void *) &snl;
    msg.msg_namelen = sizeof(snl);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;

    status = recvmsg(sockint, &msg, 0);

    if (status < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            return ret;
        }

        DBGPRINT((stderr, "read_netlink: Error recvmsg: %d\n", status));
        return status;
    }

    if (status == 0)
    {
        DBGPRINT((stdout, "read_netlink: EOF\n"));
    }

    for (h = (struct nlmsghdr *) buf; NLMSG_OK(h, (unsigned int) status); h = NLMSG_NEXT(h, status))
    {
        switch (h->nlmsg_type)
        {
            case NLMSG_ERROR:
                DBGPRINT((stdout, "read_netlink: NLMSG_ERROR\n"));
                ret = -1;
                break;
            case NLMSG_DONE:
                DBGPRINT((stdout, "read_netlink: NLMSG_DONE\n"));
                 break;
            case RTM_NEWLINK:
                DBGPRINT((stdout, "read_netlink: RTM_NEWLINK\n"));
                ifi = NLMSG_DATA(h);
                if_indextoname(ifi->ifi_index, ifname);
                DBGPRINT((stdout, "%s is %s\n", ifname, (ifi->ifi_flags & IFF_RUNNING) ? "up" : "down"));
                for (id = 0; id < MAX_NUM_DEV_IF; id++)
                {
                    if (info->ifindex[id] == ifi->ifi_index)
                    {
                        info->since[id] = (ifi->ifi_flags & IFF_RUNNING) ? get_time_sec() : 0;
                        break;
                    }
                }
                break;
            default:
                DBGPRINT((stdout, "read_netlink: other type: %d\n", h->nlmsg_type));
                break;
        }
    }

    return ret;
}

static void *link_state_monitor_thread(void * context)
{
    struct link_state_info *info = (struct link_state_info *)context;
    fd_set rfds;
    struct timeval tv;
    int retval;
    struct sockaddr_nl addr;

    int nl_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (nl_socket < 0)
    {
        DBGPRINT((stderr, "socket open failed\n"));
        return NULL;
    }

    memset((void *) &addr, 0, sizeof (addr));

    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    if (bind(nl_socket, (struct sockaddr *) &addr, sizeof (addr)) < 0)
    {
        DBGPRINT((stderr, "socket bind failed\n"));
        return NULL;
    }

    while (1)
    {
        FD_ZERO(&rfds);
        FD_CLR(nl_socket, &rfds);
        FD_SET(nl_socket, &rfds);

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
        if (retval)
        {
            read_event(nl_socket, info);
        }
        else
        if (retval == -1)
        {
            DBGPRINT((stderr, "error select()\n"));
        }
        else
        {
            DBGPRINT((stdout, "select timed out\n"));
        }
    }

    return NULL;
}

int ethernet_link_state_monitor_init(void)
{
    struct net_dev_info info;
    int id = 0, ret = 0;

    memset(&g_link_state_info, 0, sizeof(struct link_state_info));

    while (get_net_dev_info(id, &info) != -1)
    {
        g_link_state_info.ifindex[id] = if_nametoindex(info.iface);
        g_link_state_info.since[id] = get_time_sec();
        id++;
    }

    ret = pthread_create(&g_link_state_info.id, NULL, link_state_monitor_thread, &g_link_state_info);
    if (ret != 0)
    {
        DBGPRINT((stderr, "pthread_create() failed\n"));
    }

    return ret;
}

int ethernet_link_state_monitor_uninit(void)
{
    pthread_join(g_link_state_info.id, NULL);
    return 0;
}

/* Device.Ethernet.Interface.{i}.Stats. */

int ethernet_if_stats_bytes_sent(int id, int *value)
{
	struct net_dev_info info;

	if (get_net_dev_info(id, &info) == 0)
	{
        *value = info.txbytes;
	}
    else
    {
        *value = 0;
    }
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_bytes_received(int id, int *value)
{
	struct net_dev_info info;

	if (get_net_dev_info(id, &info) == 0)
	{
        *value = info.rxbytes;
	}
    else
    {
        *value = 0;
    }
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_packets_sent(int id, int *value)
{
	struct net_dev_info info;

	if (get_net_dev_info(id, &info) == 0)
	{
        *value = info.txpackets;
	}
    else
    {
        *value = 0;
    }
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_packets_received(int id, int *value)
{
	struct net_dev_info info;

	if (get_net_dev_info(id, &info) == 0)
	{
        *value = info.rxpackets;
	}
    else
    {
        *value = 0;
    }
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_errors_sent(int id, int *value)
{
	struct net_dev_info info;

	if (get_net_dev_info(id, &info) == 0)
	{
        *value = info.txerrs;
	}
    else
    {
        *value = 0;
    }
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_errors_received(int id, int *value)
{
	struct net_dev_info info;

	if (get_net_dev_info(id, &info) == 0)
	{
        *value = info.rxerrs;
	}
    else
    {
        *value = 0;
    }
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_unicast_packets_sent(int id, int *value)
{
    (void) id;
    *value = 0;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_unicast_packets_received(int id, int *value)
{
    (void) id;
    *value = 0;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_discard_packets_sent(int id, int *value)
{
	struct net_dev_info info;

	if (get_net_dev_info(id, &info) == 0)
	{
        *value = info.txdrop;
	}
    else
    {
        *value = 0;
    }
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_discard_packets_received(int id, int *value)
{
	struct net_dev_info info;

	if (get_net_dev_info(id, &info) == 0)
	{
        *value = info.rxdrop;
	}
    else
    {
        *value = 0;
    }
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_multicast_packets_sent(int id, int *value)
{
    (void) id;
    *value = 0;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_multicast_packets_received(int id, int *value)
{
    (void) id;
    *value = 0;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_broadcast_packets_sent(int id, int *value)
{
    (void) id;
    *value = 0;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_broadcast_packets_received(int id, int *value)
{
    (void) id;
    *value = 0;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_stats_unknown_proto_packets_received(int id, int *value)
{
    (void) id;
    *value = 0;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

/* Device.Ethernet.Interface.{i}. */

int ethernet_if_get_enable(int id, int *value)
{
	struct net_dev_info info;
    struct ifreq ifrq;
    int ret = 0;

	if (get_net_dev_info(id, &info) == 0)
	{
        ret = get_net_dev_active_flags(&ifrq, info.iface);

        if (ret == 0)
        {
            *value = (ifrq.ifr_flags & IFF_UP) ? 1 : 0;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
	}

    return ret;
}

int ethernet_if_set_enable(int id, int value)
{
	struct net_dev_info info;
    int ret = 0;

    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, value));

	if (get_net_dev_info(id, &info) == 0)
	{
        ret = set_net_dev_active_flags(IFF_UP, value, info.iface);
	}

    return ret;
}

int ethernet_if_get_status(int id, int *value)
{
	struct net_dev_info info;
    struct ifreq ifrq;
    int ret = 0;

	if (get_net_dev_info(id, &info) == 0)
	{
        ret = get_net_dev_active_flags(&ifrq, info.iface);

        if (ret == 0)
        {
            if (ifrq.ifr_flags & IFF_UP)
            {
                if (ifrq.ifr_flags & IFF_RUNNING)
                {
                    *value = ETHERNET_IF_STATUS_UP;
                }
                else
                {
                    *value = ETHERNET_IF_STATUS_DOWN;
                }
            }
            else
            {
                *value = ETHERNET_IF_STATUS_DOWN;
            }
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
	}

    return ret;
}

int ethernet_if_get_name(int id, char *value)
{
	struct net_dev_info info;

	if (get_net_dev_info(id, &info) == 0)
	{
        sprintf(value, "%s", info.iface);
	}
    DBGPRINT((stdout, "%s: %s\n", __FUNCTION__, value));

    return 0;
}

int ethernet_if_get_last_change(int id, int *value)
{
    unsigned int time_diff;

    time_diff = get_time_sec() - g_link_state_info.since[id];
    *value = (time_diff > 0) ? time_diff : 0xffffffff - time_diff;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_get_lower_layers(int id, int *value)
{
    (void) id;
    *value = 0;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_set_lower_layers(int id, int value)
{
    (void) id;
    (void) value;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, value));
    return 0;
}

int ethernet_if_get_upstream(int id, int *value)
{
    (void) id;
    *value = 0;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_get_mac_addr(int id, char *value)
{
	struct net_dev_info info;
	FILE *f;
	char line[256], *lineptr = line;
	size_t size;
    char fname[30], macaddr[18];

    memset(macaddr, '\0', 18);

	if (get_net_dev_info(id, &info) == 0)
	{
        sprintf(fname, "/sys/class/net/%s/address", info.iface);
        size = sizeof(line);
        if ((f = fopen(fname, "r")))
        {
            if ((getline(&lineptr, &size, f) >= 0))
            {
                sscanf(lineptr, "%s", macaddr);
            }
            fclose(f);
        }
	}
    sprintf(value, "%s", macaddr);
    DBGPRINT((stdout, "%s: %s\n", __FUNCTION__, value));

    return 0;
}

int ethernet_if_get_max_bitrate(int id, int *value)
{
	struct net_dev_info info;
	FILE *f;
	char line[256], *lineptr = line;
	size_t size;
    char fname[30];
    unsigned speed = 0;

	if (get_net_dev_info(id, &info) == 0)
	{
        sprintf(fname, "/sys/class/net/%s/speed", info.iface);
        size = sizeof(line);
        if ((f = fopen(fname, "r")))
        {
            if ((getline(&lineptr, &size, f) >= 0))
            {
                sscanf(lineptr, "%u", &speed);
            }
            fclose(f);
        }
	}
    *value = speed;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_set_max_bitrate(int id, int value)
{
    (void) id;
    (void) value;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, value));
    return 0;
}

int ethernet_if_get_current_bitrate(int id, int *value)
{
	struct net_dev_info info;
	FILE *f;
	char line[256], *lineptr = line;
	size_t size;
    char fname[30];
    unsigned speed = 0;

	if (get_net_dev_info(id, &info) == 0)
	{
        sprintf(fname, "/sys/class/net/%s/speed", info.iface);
        size = sizeof(line);
        if ((f = fopen(fname, "r")))
        {
            if ((getline(&lineptr, &size, f) >= 0))
            {
                sscanf(lineptr, "%u", &speed);
            }
            fclose(f);
        }
	}
    *value = speed;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_get_duplex_mode(int id, char *value)
{
	struct net_dev_info info;
	FILE *f;
	char line[256], *lineptr = line;
	size_t size;
    char fname[30], duplex[10];

    memset(duplex, '\0', 10);

	if (get_net_dev_info(id, &info) == 0)
	{
        sprintf(fname, "/sys/class/net/%s/duplex", info.iface);
        size = sizeof(line);
        if ((f = fopen(fname, "r")))
        {
            if ((getline(&lineptr, &size, f) >= 0))
            {
                sscanf(lineptr, "%s", duplex);
            }
            fclose(f);
        }
	}
    sprintf(value, "%s", duplex);
    DBGPRINT((stdout, "%s: %s\n", __FUNCTION__, value));

    return 0;
}

int ethernet_if_set_duplex_mode(int id, char *value)
{
    (void) id;
    (void) value;
    DBGPRINT((stdout, "%s: %s\n", __FUNCTION__, value));
    return 0;
}


int ethernet_if_get_eee_capability(int id, int *value)
{
    (void) id;
    *value = 0;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_get_eee_enable(int id, int *value)
{
    (void) id;
    *value = 0;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}

int ethernet_if_set_eee_enable(int id, int value)
{
    (void) id;
    (void) value;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, value));
    return 0;
}

/* Device.Ethernet.Interface. */

int ethernet_if_get_number_of_entries(int *value)
{
	FILE *f;
	char line[256], *lineptr = line;
	size_t size;
    int num_of_entries = 0;

    size = sizeof(line);

	if ((f = fopen("/proc/net/dev", "r")))
    {
		while ((getline(&lineptr, &size, f) >= 0))
        {
            num_of_entries++;
        }
		fclose(f);
    }

    *value = num_of_entries - 2;
    if (*value > MAX_NUM_DEV_IF) *value = MAX_NUM_DEV_IF;
    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

    return 0;
}
