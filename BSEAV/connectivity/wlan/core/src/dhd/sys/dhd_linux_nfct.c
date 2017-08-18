/*
 * @file to track Linux Netfilter CONNTRACK(nfct) information
 * To avoid port collision of NATOE ports with Linux host ports
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#include <linux/socket.h>
#include <net/sock.h>

/* These are Required for NETFILTER functionality */
#include <linux/netfilter.h>
#include <net/netfilter/nf_conntrack_expect.h>

/* DHD header files */
#include <proto/ethernet.h>
#include <proto/bcmevent.h>
#include <bcmutils.h>
#include <hnd_pktq.h>
#include <osl.h>
#include <dhd_linux.h>
#include <dhd_dbg.h>
#include <dhd_linux_nfct.h>
#include <dhd_linux_wq.h>

static struct nla_policy policy[__CTA_TUPLE_MAX] = {
	[CTA_TUPLE_IP]		= { .type = NLA_NESTED },
	[CTA_TUPLE_PROTO]	= { .type = NLA_NESTED },
};

static int dhd_natoe_issue_port_config_ioctl(dhd_pub_t *dhd, wl_natoe_ioc_t *params, uint32 size);
static int dhd_ct_get_ip(const nl_attr *attr, struct dhd_nfct_tuple *tuple);
static int dhd_ct_get_proto(const nl_attr *attr, struct dhd_nfct_tuple *tuple);
static int dhd_ct_extract_tuple(const nl_attr *attr, struct dhd_nfct_tuple *tuple);
static int dhd_ct_parse_conntrack_entry(dhd_nfct_info_t *nfct, nl_attr *nla_buf[], uint32 type);
static int dhd_ct_parse_expect_entry(dhd_nfct_info_t *nfct, nl_attr *nla_buf[], uint32 type);
static int dhd_ct_parse_ctnetlink_msg(dhd_nfct_info_t *nfct, nl_attr *nfa[], uint8 subsys,
		uint16 nlmsg_type);
static dhd_nfct_info_t * dhd_ct_alloc(dhd_pub_t *dhd);
static int dhd_ct_process_subsys(dhd_nfct_info_t *nfct, struct nlmsghdr *nlh);
static int dhd_ct_nl_process_msg(dhd_nfct_info_t *nfct, uint8 *buf, size_t len);
static int dhd_ct_get_nl_msg(dhd_nfct_info_t *nfct, int len);
static void dhd_ct_data_ready(struct sock *sk, int len);
static void dhd_ct_free(dhd_nfct_info_t *nfct);

static int
dhd_natoe_issue_port_config_ioctl(dhd_pub_t *dhd, wl_natoe_ioc_t *params, uint32 size)
{
	char *buf = NULL;
	char iovar[] = "natoe";
	uint32 allocsize = 0;
	wl_ioctl_t ioctl;
	int status = BCME_ERROR;

	if (params) {
		memset(&ioctl, 0, sizeof(wl_ioctl_t));

		allocsize = (size + strlen(iovar) + 1);
		if ((allocsize < size) || (allocsize < strlen(iovar))) {
			DHD_ERROR(("%s: overflow - allocation size too large %d < %d + %d!\n",
				__FUNCTION__, allocsize, size, (uint16)strlen(iovar)));
			return status;
		}
		buf = MALLOC(dhd->osh, allocsize);

		if (!buf) {
			DHD_ERROR(("%s: malloc of size %d failed!\n", __FUNCTION__, allocsize));
			return status;
		}
		ioctl.cmd = WLC_SET_VAR;
		ioctl.set = TRUE;
		ioctl.len = allocsize;
		bcm_mkiovar(iovar, (char *)params, size, buf, allocsize);
		status = dhd_wl_ioctl(dhd, 0, &ioctl, buf, allocsize);

		MFREE(dhd->osh, buf, allocsize);
	}
	return status;
}

int
dhd_natoe_prep_send_exception_port_ioctl(dhd_pub_t *dhd, dhd_ct_ioc_t *ct_ioc)
{
	wl_natoe_ioc_t *natoe_ioc;
	uint32 iocsz = sizeof(*natoe_ioc) + WL_NATOE_IOC_BUFSZ;
	uint16 buflen = WL_NATOE_IOC_BUFSZ;
	uint16 buflen_at_start = WL_NATOE_IOC_BUFSZ;
	bcm_xtlv_t *pxtlv = NULL;
	int ret = BCME_OK;

	/* alloc mem for ioctl headr + tlv data */
	natoe_ioc = MALLOCZ(dhd->osh, iocsz);
	if (!natoe_ioc) {
		DHD_ERROR(("ioctl header memory alloc failed\n"));
		ret = BCME_ERROR;
		goto exit;
	}

	/* make up natoe cmd ioctl header */
	natoe_ioc->version = WL_NATOE_IOCTL_VERSION;
	natoe_ioc->id = ct_ioc->ioc_type;
	natoe_ioc->len = WL_NATOE_IOC_BUFSZ;
	pxtlv = (bcm_xtlv_t *)natoe_ioc->data;

	ret = bcm_pack_xtlv_entry((uint8**)&pxtlv, &buflen, ct_ioc->ioc_type,
			sizeof(wl_natoe_exception_port_t),
			&ct_ioc->port_config, BCM_XTLV_OPTION_ALIGN32);

	if (ret != BCME_OK) {
		goto exit;
	}

	/* adjust iocsz to the end of last data record */
	natoe_ioc->len = (buflen_at_start - buflen);
	iocsz = sizeof(*natoe_ioc) + natoe_ioc->len;

	ret = dhd_natoe_issue_port_config_ioctl(dhd, natoe_ioc, iocsz);

	if (ret != BCME_OK) {
		DHD_ERROR(("Fail to set iovar %d\n", ret));
	}

exit:
	MFREE(dhd->osh, ct_ioc, sizeof(*ct_ioc));
	if (natoe_ioc) {
		MFREE(dhd->osh, natoe_ioc, (sizeof(*natoe_ioc) + WL_NATOE_IOC_BUFSZ));
	}
	return ret;
}

static int
dhd_ct_get_ip(const nl_attr *attr, struct dhd_nfct_tuple *tuple)
{
	nl_attr *attr_buf[__CTA_IP_MAX];
	int ret;

	ret = nla_parse_nested(attr_buf, CTA_IP_MAX, attr, NULL);

	if (ret < 0) {
		DHD_ERROR(("%s ERROR in extracting IP: %d\n", __FUNCTION__, ret));
		return CT_FAILURE;
	}

	if (attr_buf[CTA_IP_V4_SRC]) {
		tuple->src_ip = *(uint32 *)nla_data(attr_buf[CTA_IP_V4_SRC]);
	}
	if (attr_buf[CTA_IP_V4_DST]) {
		tuple->dst_ip = *(uint32 *)nla_data(attr_buf[CTA_IP_V4_DST]);
	}

	return CT_SUCCESS;
}

static int
dhd_ct_get_proto(const nl_attr *attr, struct dhd_nfct_tuple *tuple)
{
	nl_attr *attr_buf[__CTA_PROTO_MAX];
	int ret;

	ret = nla_parse_nested(attr_buf, CTA_PROTO_MAX, attr, NULL);

	if (ret < 0) {
		DHD_ERROR(("%s ERROR in extracting PROTO: %d\n", __FUNCTION__, ret));
		return CT_FAILURE;
	}
	if (attr_buf[CTA_PROTO_NUM]) {
		tuple->proto = *(uint8 *)nla_data(attr_buf[CTA_PROTO_NUM]);
	}
	if (attr_buf[CTA_PROTO_SRC_PORT]) {
		tuple->sport = ntohs(*(uint16 *)nla_data(attr_buf[CTA_PROTO_SRC_PORT]));
	}
	if (attr_buf[CTA_PROTO_DST_PORT]) {
		tuple->dport = ntohs(*(uint16 *)nla_data(attr_buf[CTA_PROTO_DST_PORT]));
	}
	return CT_SUCCESS;
}

static int
dhd_ct_extract_tuple(const nl_attr *attr, struct dhd_nfct_tuple *tuple)
{
	nl_attr *attr_buf[__CTA_TUPLE_MAX];
	int ret;

	ret = nla_parse_nested(attr_buf, CTA_TUPLE_MAX, attr, policy);

	if (ret < 0) {
		DHD_ERROR(("%s ERROR in extracting tuple: %d\n", __FUNCTION__, ret));
		return CT_FAILURE;
	}

	if (attr_buf[CTA_TUPLE_IP]) {
		if (dhd_ct_get_ip(attr_buf[CTA_TUPLE_IP], tuple) < CT_SUCCESS) {
			return CT_FAILURE;
		}
	}
	if (attr_buf[CTA_TUPLE_PROTO]) {
		if (dhd_ct_get_proto(attr_buf[CTA_TUPLE_PROTO], tuple) < CT_SUCCESS) {
			return CT_FAILURE;
		}
	}
	return CT_SUCCESS;
}

static int
dhd_ct_parse_conntrack_entry(dhd_nfct_info_t *nfct, nl_attr *nla_buf[], uint32 type)
{
	dhd_pub_t *dhd = nfct->dhd;
	wl_event_data_natoe_t *natoe = nfct->natoe_info;
	struct dhd_nfct_tuple reply;
	dhd_ct_ioc_t *ct_ioc;
	uint8 ct_schedule = FALSE;

	memset(&reply, 0, sizeof(struct dhd_nfct_tuple));

	if (nla_buf[CTA_TUPLE_REPLY]) {

		if (dhd_ct_extract_tuple(nla_buf[CTA_TUPLE_REPLY], &reply) < CT_SUCCESS) {
			return CT_FAILURE;
		}

		/* Inform dongle using IOCTL if port collision occurs
		 * Check if host/STA port falls in dongle NATOE port range
		 */

		spin_lock_bh(&dhd->nfct_lock);
		if (reply.src_ip == natoe->sta_ip) {
			if ((reply.sport >= natoe->start_port) &&
					(reply.sport <= natoe->end_port)) {

				ct_ioc = MALLOC(dhd->osh, sizeof(*ct_ioc));
				if (!ct_ioc) {
					DHD_ERROR(("ERROR: conntrack ioctl mem alloc failed\n"));
					return CT_FAILURE;
				}

				/* send sta_port, dst_port and final_dst_ip info */
				ct_ioc->port_config.sta_port_num = reply.sport;
				ct_ioc->port_config.ip = reply.dst_ip;
				ct_ioc->port_config.dst_port_num = reply.dport;

				ct_schedule = TRUE;
			}
		} else if (reply.dst_ip == natoe->sta_ip) {
			if ((reply.dport >= natoe->start_port) &&
					(reply.dport <= natoe->end_port)) {

				ct_ioc = MALLOC(dhd->osh, sizeof(*ct_ioc));
				if (!ct_ioc) {
					DHD_ERROR(("ERROR: conntrack ioctl mem alloc failed\n"));
					return CT_FAILURE;
				}

				/* send sta_port, dst_port and final_dst_ip info
				 * NOTE: Here STA Info is in destination fields
				 */
				ct_ioc->port_config.sta_port_num = reply.dport;
				ct_ioc->port_config.ip = reply.src_ip;
				ct_ioc->port_config.dst_port_num = reply.sport;

				ct_schedule = TRUE;
			}
		}
		spin_unlock_bh(&dhd->nfct_lock);

		if (ct_schedule) {
			ct_ioc->port_config.entry_type = (uint8)type;
			ct_ioc->ioc_type = WL_NATOE_XTLV_EXCEPTION_PORT;

			/* Schedule workq to send ioctl to dongle */
			dhd_natoe_ct_ioctl_schedule_work(dhd, ct_ioc);
		}
	}
	return CT_SUCCESS;
}

/* Expect tuple contains STA info, as host is performing NAT for SIP/H323 related protocols
 * SOft AP_cli tuple is also processed to find which cli has established SIP session
 */
static int
dhd_ct_parse_expect_entry(dhd_nfct_info_t *nfct, nl_attr *nla_buf[], uint32 type)
{
	struct dhd_nfct_tuple expect;
	/* ap_cli tuple contains expect table Master info */
	struct dhd_nfct_tuple ap_cli;
	char   helper[DHD_HELPER_CHAR_MAX];
	dhd_ct_ioc_t *ct_ioc;
	uint32 class = 0;

	memset(&expect, 0, sizeof(struct dhd_nfct_tuple));
	memset(&ap_cli, 0, sizeof(struct dhd_nfct_tuple));

	if (nla_buf[CTA_EXPECT_MASTER]) {
		if (dhd_ct_extract_tuple(nla_buf[CTA_EXPECT_MASTER], &ap_cli) < CT_SUCCESS) {
			return CT_FAILURE;
		}
	}

	if (nla_buf[CTA_EXPECT_TUPLE]) {
		if (dhd_ct_extract_tuple(nla_buf[CTA_EXPECT_TUPLE], &expect) < CT_SUCCESS) {
			return CT_FAILURE;
		}
	}

	if (nla_buf[CTA_EXPECT_HELP_NAME]) {
		memcpy(helper, nla_data(nla_buf[CTA_EXPECT_HELP_NAME]),
				nla_len(nla_buf[CTA_EXPECT_HELP_NAME]));
	}
	if (nla_buf[CTA_EXPECT_CLASS]) {
		class = ntohl(*(uint32 *)nla_data(nla_buf[CTA_EXPECT_CLASS]));
	}

	if ((expect.dst_ip == nfct->natoe_info->sta_ip) &&
			(!memcmp(helper, SIP_PROTO, SIP_CHAR_SZ) || !memcmp(helper, H323_PORTO,
			H323_CHAR_SZ)) && (class) &&
			((ap_cli.dport == SIP_PORT) || (ap_cli.dport == SIP_PORT_SEC))) {

		ct_ioc = MALLOC(nfct->dhd->osh, sizeof(*ct_ioc));
		if (!ct_ioc) {
			DHD_ERROR(("ERROR: ct_expect ioctl mem alloc failed\n"));
			return CT_FAILURE;
		}

		/* send sta_port, 0(discard this value), softAP client_ip */
		ct_ioc->port_config.sta_port_num = expect.dport;
		ct_ioc->port_config.ip = ap_cli.src_ip;
		ct_ioc->port_config.dst_port_num = 0;
		ct_ioc->port_config.entry_type = (uint8)type;
		ct_ioc->ioc_type = WL_NATOE_XTLV_SKIP_PORT;

		/* Schedule workq to send ioctl to dongle */
		dhd_natoe_ct_ioctl_schedule_work(nfct->dhd, ct_ioc);
	}
	return CT_SUCCESS;
}

static int
dhd_ct_parse_ctnetlink_msg(dhd_nfct_info_t *nfct, nl_attr *nfa[], uint8 subsys, uint16 nlmsg_type)
{
	int ret = CT_SUCCESS;
	uint32 type = CT_MSG_TYPE(nlmsg_type);

	if (!(type & nfct->subscriptions)) {
		return CT_CONTINUE;
	}

	switch (subsys) {
	case NFNL_SUBSYS_CTNETLINK:
		ret = dhd_ct_parse_conntrack_entry(nfct, nfa, type);
		break;

	case NFNL_SUBSYS_CTNETLINK_EXP:
		ret = dhd_ct_parse_expect_entry(nfct, nfa, type);
		break;

	default:
		return CT_FAILURE;
	}
	return ret;
}

static int
dhd_ct_process_subsys(dhd_nfct_info_t *nfct, struct nlmsghdr *nlh)
{
	nl_attr *attr_buf[__CTA_MAX];
	nl_attr *attr = GET_NLA(NLMSG_DATA(nlh));
	uint8 min_len = NLMSG_SPACE(sizeof(struct nfgenmsg));
	uint8 type;
	uint8 subsys_id;
	int status = CT_FAILURE;
	int len;

	/* Check for error in the NLMSG */
	if ((nlh->nlmsg_type == NLMSG_ERROR) || (nlh->nlmsg_len < min_len)) {
		DHD_ERROR(("%s  NLMSG_ERROR  or len is less than min_len\n", __FUNCTION__));
		return status;
	}

	type = NFNL_MSG_TYPE(nlh->nlmsg_type);
	subsys_id = NFNL_SUBSYS_ID(nlh->nlmsg_type);

	if (subsys_id > CT_SUBSYS_MAX) {
		return status;
	}

	len = nlh->nlmsg_len - NLMSG_ALIGN(min_len);
	memset(attr_buf, 0, sizeof(nl_attr *) * __CTA_MAX);

	/* Extract the entire msg to attr_buf array */
	while (len > 0) {
		if (nla_ok(attr, len)) {
			if (nla_type(attr) < __CTA_MAX) {
				attr_buf[nla_type(attr)] = attr;
			}
			attr = nla_next(attr, &len);
		} else {
			break;
		}
	}

	if (len < 0) {
		DHD_ERROR(("%s length after parsing should not be less than 0\n", __FUNCTION__));
		return status;
	}

	status = dhd_ct_parse_ctnetlink_msg(nfct, attr_buf, subsys_id, type);

	return status;
}

static int
dhd_ct_nl_process_msg(dhd_nfct_info_t *nfct, uint8 *buf, size_t len)
{
	int ret = 0;
	struct nlmsghdr *nlh = (struct nlmsghdr *)buf;

	ASSERT(buf);
	ASSERT(len > 0);

	while (NLMSG_OK(nlh, len)) {
		/* Check to skip ACK msgs */
		if (*(int *)NLMSG_DATA(nlh)) {
			ret = dhd_ct_process_subsys(nfct, nlh);
			if (ret < CT_SUCCESS) {
				break;
			}
		}
		nlh = NLMSG_NEXT(nlh, len);
	}
	return ret;
}

static int
dhd_ct_get_nl_msg(dhd_nfct_info_t *nfct, int allocsz)
{
	int len = 0;
	int status;
	uint8 *buf = NULL;
	struct sk_buff *skb;

	buf = MALLOC(nfct->dhd->osh, allocsz);
	if (!buf) {
		DHD_ERROR(("%s unable to alloc memeory\n", __FUNCTION__));
		return CT_FAILURE;
	}

	while (1) {
		/* Run this in Non-blocking mode */
		skb = skb_recv_datagram(nfct->nl_sock->sk, MSG_DONTWAIT, 0, &status);

		if (skb) {
			len = (int)skb->len;
			if (!len || !skb->data) {
				status = CT_FAILURE;
				goto out;
			}
			memcpy(buf, skb->data, len);
			skb_free_datagram(nfct->nl_sock->sk, skb);
		} else {
			status = CT_SUCCESS;
			goto out;
		}

		if (!nfct && !nfct->natoe_info->natoe_active) {
			continue;
		}

		status = dhd_ct_nl_process_msg(nfct, buf, len);
		if (status < CT_SUCCESS) {
			DHD_ERROR(("%s  Failure in processing nl_msg\n", __FUNCTION__));
		}
	}
out:
	MFREE(nfct->dhd->osh, buf, allocsz);
	return status;
}

/* When data is sent to our registered socket, this function is called to notify us that
 * skb is pending to be read
 */
static void
dhd_ct_data_ready(struct sock *sk, int len)
{
	dhd_nfct_info_t *nfct;

	read_lock(&sk->sk_callback_lock);

	nfct = (dhd_nfct_info_t *)sk->sk_user_data;
	if (dhd_ct_get_nl_msg(nfct, len) < 0) {
		DHD_ERROR(("%s  Failure in processing skb & nl_msg\n", __FUNCTION__));
	}

	read_unlock(&sk->sk_callback_lock);
}

void
dhd_ct_send_dump_req(dhd_nfct_info_t *nfct)
{
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfmsg;
	struct sk_buff *skb;
	uint16 len;

	ASSERT(nfct);

	len = NLMSG_LENGTH(sizeof(*nfmsg));

	skb = alloc_skb(len, (in_interrupt() ? GFP_ATOMIC : GFP_KERNEL));
	if (skb) {
		skb_put(skb, len);
	} else {
		return;
	}

	nlh = (struct nlmsghdr *)skb->data;
	memset(nlh, 0, len);

	nlh->nlmsg_len = len;
	nlh->nlmsg_type = (NFNL_SUBSYS_CTNETLINK) | (IPCTNL_MSG_CT_GET << 8);
	nlh->nlmsg_flags = (NLM_F_REQUEST | NLM_F_DUMP);
	/* dst pid of kernel */
	nlh->nlmsg_pid = 0;

	nlh->nlmsg_seq = 0;

	nfmsg = nlmsg_data(nlh);
	nfmsg->nfgen_family = AF_INET;
	nfmsg->version = NFNETLINK_V0;
	nfmsg->res_id = 0;

	NETLINK_CB(skb).portid = nfct->nl_local->nl_pid;

	netlink_unicast(nfct->nl_sock->sk, skb, nlh->nlmsg_pid, MSG_DONTWAIT);
}


static void
dhd_ct_free(dhd_nfct_info_t *nfct)
{
	osl_t *osh = nfct->dhd->osh;

	if (nfct->nl_local) {
		MFREE(osh, nfct->nl_local, sizeof(struct sockaddr_nl));
	}
	if (nfct->natoe_info) {
		MFREE(osh, nfct->natoe_info, sizeof(wl_event_data_natoe_t));
	}
	MFREE(osh, nfct, sizeof(*nfct));
}

static dhd_nfct_info_t *
dhd_ct_alloc(dhd_pub_t *dhd)
{
	dhd_nfct_info_t	*nfct;
	struct sockaddr_nl *nl_local;
	wl_event_data_natoe_t *natoe_info;
	osl_t *osh = dhd->osh;

	nfct = MALLOCZ(osh, sizeof(*nfct));
	if (!nfct) {
		return NULL;
	}

	nfct->nl_local = MALLOCZ(osh, sizeof(*nl_local));
	if (!nfct->nl_local) {
		goto exit;
	}

	nfct->natoe_info = MALLOCZ(osh, sizeof(*natoe_info));
	if (!nfct->natoe_info) {
		goto exit;
	}
	return nfct;

exit:
	dhd_ct_free(nfct);
	return NULL;
}

/* Bind with NL sockets and update subscription details to nl_groups
 * Each SUBCRIPTION is maintained as a seperate bit-map for nl_groups
 */
int
dhd_ct_nl_bind(dhd_nfct_info_t *nfct, uint32 subscriptions)
{
	int err;

	nfct->nl_local->nl_groups = subscriptions;

	err = nfct->nl_sock->ops->bind(nfct->nl_sock, (struct sockaddr *)nfct->nl_local,
			sizeof(*nfct->nl_local));
	if (err < 0) {
		DHD_ERROR(("%s ERROR in socket bind %d\n", __FUNCTION__, err));
		return CT_FAILURE;
	}

	return CT_SUCCESS;
}

/* open a NETLINK socket to subscribe for ctnetlink groups
 * subsys_id:
 * NFNL_SUBSYS_CTNETLINK: For conntrack entries
 * NFNL_SUBSYS_CTNETLINK_EXP: For expect entries
 * subscriptions: NEW and DESTROY events for CONNTRACK and EXPECT entries
 * On error return NULL.
 */
dhd_nfct_info_t *
dhd_ct_open(dhd_pub_t *dhd, uint8 subsys_id, uint32 subscriptions)
{
	dhd_nfct_info_t *nfct;
	struct sock *sk;
	int fd;

	nfct = dhd_ct_alloc(dhd);
	if (!nfct) {
		return NULL;
	}

	nfct->dhd = dhd;

	fd = sock_create(AF_NETLINK, SOCK_RAW, NETLINK_NETFILTER, &nfct->nl_sock);
	if (fd < 0) {
		DHD_ERROR(("%s ERROR in socket creation: %d. Check if NETLINK module is loaded\n",
				__FUNCTION__, fd));
		goto out;
	}

	sk = nfct->nl_sock->sk;
	nfct->nl_local->nl_family = AF_NETLINK;
	nfct->subscriptions = subscriptions;

	write_lock_bh(&sk->sk_callback_lock);

	sk->sk_user_data = nfct;
	nfct->prev_sk_data_ready = sk->sk_data_ready;
	sk->sk_data_ready = dhd_ct_data_ready;

	write_unlock_bh(&sk->sk_callback_lock);

	/* Initially bind with Zero subscriptions to avoid being notified right from now
	 * Rebind with actual subscriptions when NATOE enable is called
	 */
	if (dhd_ct_nl_bind(nfct, CT_NULL_SUBSCRIPTION) < 0) {
		DHD_ERROR(("%s Error in binding socket \n", __FUNCTION__));
		goto out;
	}

	nfct->nl_local->nl_pid = current->pid;
	return nfct;
out:
	dhd_ct_close(nfct);
	return NULL;
}

void
dhd_ct_close(dhd_nfct_info_t *nfct)
{
	struct sock *sk;

	if (nfct) {
		if (nfct->nl_sock) {
			sk = nfct->nl_sock->sk;
			write_lock_bh(&sk->sk_callback_lock);

			sk->sk_data_ready = nfct->prev_sk_data_ready;
			sk->sk_user_data = NULL;

			write_unlock_bh(&sk->sk_callback_lock);

			sock_release(nfct->nl_sock);
		}
		dhd_ct_free(nfct);
	}
}
