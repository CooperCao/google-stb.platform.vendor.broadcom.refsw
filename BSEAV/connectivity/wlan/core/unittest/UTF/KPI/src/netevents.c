/*
 * Network events that can be controlled remotely (e.g. by UTF scripts)
 *
 * Author Robert J. McMahon (rmcmahon)
 * June 2015
 * 
 * $Copyright Open Broadcom Corporation$
 *
 */ 
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

void sigint();
void siguser();
void sigalrm();

int main() {
    int fd;
    struct sockaddr_nl sa;
    struct ifaddrmsg *rtmp;
    struct rtattr *rtatp;
    int rtattrlen;
    struct ifinfomsg *rtif;
    char ifname[IF_NAMESIZE];
    int len;
    char buf[4096];
    struct iovec iov = { buf, sizeof(buf) };
    struct nlmsghdr *nh;

    signal(SIGINT, sigint);

    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    bind(fd, (struct sockaddr *) &sa, sizeof(sa));

    while(1) {
	struct msghdr msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
	len = recvmsg(fd, &msg, 0);

	for (nh = (struct nlmsghdr *) buf; NLMSG_OK (nh, len); nh = NLMSG_NEXT (nh, len)) {
	    /* The end of multipart message. */
	    if (nh->nlmsg_type == NLMSG_DONE){
		printf("got msg done\n");
		fflush (stdout);
		break;
	    }

	    if (nh->nlmsg_type == NLMSG_ERROR){
		printf("got msg error\n");
		fflush (stdout);
		continue;
	    }
	    if (nh->nlmsg_type < RTM_NEWADDR){
		rtif=(struct ifinfomsg *)NLMSG_DATA(nh);
		printf("Interface %s %s\n",if_indextoname(rtif->ifi_index,ifname), (rtif->ifi_flags & IFF_UP) ? "link up":"link down");
		fflush (stdout);
	    }
	}
    }
    return 0;
}

void sigint (void) {
    exit(0);
}
