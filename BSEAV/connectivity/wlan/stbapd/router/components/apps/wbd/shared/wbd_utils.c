/*
 * WBD utility functions
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wbd_utils.c 750292 2018-03-06 04:32:13Z cb888957 $
 */

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "wbd.h"
#include "wbd_shared.h"
#include "wbd_sock_utility.h"

struct route_info
{
	struct in_addr dstAddr;
	struct in_addr srcAddr;
	struct in_addr gateWay;
	char ifName[IF_NAMESIZE];
};

/* Removes all occurence of the character from a string */
void
wbd_remove_char(char *str, char let)
{
	unsigned int i, j;

	for (i = j = 0; str[i] != 0; i++) {
		if (str[i] == let)
			continue;
		else
			str[j++] = str[i];
	}

	str[j] = '\0';
}

/* parse the route info returned */
static int
wbd_parse_routes(struct nlmsghdr *nlHdr, struct route_info *rtInfo)
{
	struct rtmsg *rtMsg;
	struct rtattr *rtAttr;
	int rtLen, found = 0;

	rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);

	/* If the route is not for AF_INET or does not belong to main routing table then return. */
	if ((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))
		return found;

	/* get the rtattr field */
	rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
	rtLen = RTM_PAYLOAD(nlHdr);

	for (; RTA_OK(rtAttr, rtLen); rtAttr = RTA_NEXT(rtAttr, rtLen)) {
		switch (rtAttr->rta_type)
		{
			case RTA_OIF:
				if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);
				break;

			case RTA_GATEWAY:
				memcpy(&rtInfo->gateWay, RTA_DATA(rtAttr), sizeof(rtInfo->gateWay));
				found = 1;
				break;

			case RTA_PREFSRC:
				memcpy(&rtInfo->srcAddr, RTA_DATA(rtAttr), sizeof(rtInfo->srcAddr));
				break;

			case RTA_DST:
				memcpy(&rtInfo->dstAddr, RTA_DATA(rtAttr), sizeof(rtInfo->dstAddr));
				found = 1;
				break;
		}
	}

	return found;
}

/* Get the gateway IP address */
int
wbd_get_gatewayip(char *gatewayip, socklen_t size)
{
	int ret = WBDE_OK;
	struct nlmsghdr *nlMsg;
	struct route_info *rtInfo;
	char msgBuf[NL_SOCK_BUFSIZE]; /* pretty large buffer */

	int sock, len, msgSeq = 0;

	/* Create Socket */
	if ((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0) {
		WBD_DEBUG("Socket Creation %s\n", strerror(errno));
		ret = WBDE_SOCK_ERROR;
		goto end;
	}

	/* Initialize the buffer */
	memset(msgBuf, 0, sizeof(msgBuf));

	/* point the header and the msg structure pointers into the buffer */
	nlMsg = (struct nlmsghdr *)msgBuf;

	/* Fill in the nlmsg header */
	nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); /* Length of message. */
	nlMsg->nlmsg_type = RTM_GETROUTE; /* Get the routes from kernel routing table */

	nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; /* The message is a request for dump */
	nlMsg->nlmsg_seq = msgSeq++; /* Sequence of the message packet */
	nlMsg->nlmsg_pid = getpid(); /* PID of process sending the request */

	/* Send the request */
	if (send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0) {
		WBD_DEBUG("Write To Socket Failed %s\n", strerror(errno));
		ret = WBDE_SOCK_ERROR;
		goto end;
	}

	/* Read the response */
	if ((len = wbd_read_nl_sock(sock, msgBuf, msgSeq, getpid())) < 0) {
		WBD_DEBUG("Read From Socket Failed %s\n", strerror(errno));
		ret = WBDE_SOCK_ERROR;
		goto end;
	}

	/* Parse and print the response */
	rtInfo = (struct route_info *)wbd_malloc(sizeof(*rtInfo), &ret);
	WBD_ASSERT();

	ret = WBDE_INVALID_GATEWAY;
	for (; NLMSG_OK(nlMsg, len); nlMsg = NLMSG_NEXT(nlMsg, len)) {
		memset(rtInfo, 0, sizeof(struct route_info));
		if (wbd_parse_routes(nlMsg, rtInfo) == 0)
			continue;

		/* Check if default gateway */
		if (strstr((char *)inet_ntoa(rtInfo->dstAddr), "0.0.0.0")) {
			/* copy it over */
			inet_ntop(AF_INET, &rtInfo->gateWay, gatewayip, size);
			/* found default gateway */
			ret = WBDE_OK;
			break;
		}
	}

	/* try again in next iteration and look for 0.0.0.0 */
	if (ret != WBDE_OK) {
		WBD_DEBUG(" %s need to try again\n", strerror(ret));
	}
	free(rtInfo);
	close(sock);

end:
	return ret;
}

/* Convert Ethernet address string representation to binary data */
int
wbd_ether_atoe(const char *a, struct ether_addr *n)
{
	char *c = NULL;
	int i = 0;

	memset(n, 0, ETHER_ADDR_LEN);
	for (;;) {
		n->octet[i++] = (uint8)strtoul(a, &c, 16);
		if (!*c++ || i == ETHER_ADDR_LEN)
			break;
		a = c;
	}

	return (i == ETHER_ADDR_LEN);
}

/* Convert binary data to Ethernet address string representation */
char *
wbd_ether_etoa(const unsigned char *e, char *a)
{
	char *c = a;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", e[i] & 0xff);
	}
	return a;
}

/* Returns current local time in YYYY:MM:DD HH:MM:SS format. */
char*
wbd_get_formated_local_time(char* buf, int size)
{
	struct timeval tv;
	struct tm *tm_info;

	gettimeofday(&tv, NULL);
	tm_info = localtime(&(tv.tv_sec));
	strftime(buf, size, WBD_DATE_TIME_FORMAT, tm_info);

	return buf;
}
