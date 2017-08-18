/*
 * @file Header file describing NETFILTER CONNTRACK(nfct) information
 * Provides structure types and function  prototypes for DHD conntrack and expect entries handling
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

#ifndef _NFCT_CONN_H
#define _NFCT_CONN_H

/* These are Required for NETFILTER functionality */
#include <linux/netlink.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_conntrack.h>

#define		NATOE_MAX_PORT			65535
#define		CT_SUBSYS_MAX			11
#define		CT_BUF_SIZE			8192

#define		CT_FAILURE			-1
#define		CT_SUCCESS			0
#define		CT_CONTINUE			2

#define		SIP_PROTO			"sip"
#define		SIP_CHAR_SZ			3
#define		H323_PORTO			"h323"
#define		H323_CHAR_SZ			4


#define		SIP_PORT			5060
#define		SIP_PORT_SEC			5061

typedef struct nlattr nl_attr;

/* message type */

#define NF_NEW				0
#define NF_UPDATE			1
#define NF_DESTROY			2
#define NF_EXPECT_NEW			3
#define NF_EXPECT_UPDATE		4
#define NF_EXPECT_DESTROY		5
#define NF_ERROR			6

#define CT_UNKNOWN			0

#define CT_NEW				(1 << NF_NEW)
#define CT_UPDATE			(1 << NF_UPDATE)
#define CT_DESTROY			(1 << NF_DESTROY)
#define CT_EXPECT_NEW			(1 << NF_EXPECT_NEW)
#define CT_EXPECT_UPDATE		(1 << NF_EXPECT_UPDATE)
#define CT_EXPECT_DESTROY		(1 << NF_EXPECT_DESTROY)
#define CT_ERROR			(1 << NF_ERROR)

#define CT_NULL_SUBSCRIPTION		0

#define CT_SUBSCRIPTION_CONNTRACK	(CT_NEW | CT_DESTROY)
#define CT_SUBSCRIPTION_EXPECT		(CT_EXPECT_NEW | CT_EXPECT_DESTROY)

#define CT_ALL				(CT_SUBSCRIPTION_CONNTRACK | CT_SUBSCRIPTION_EXPECT)

#define GET_NLA(n)			((nl_attr *)(((char *)(n)) +\
						NLMSG_ALIGN(sizeof(struct nfgenmsg))))

#define CT_MSG_TYPE(type)		((type == IPCTNL_MSG_CT_NEW) ? CT_NEW :\
						(type == IPCTNL_MSG_CT_DELETE) ? (CT_DESTROY) :\
						CT_UNKNOWN)

/* NATOE info, conntrack and expect entry structs */

#define DHD_HELPER_CHAR_MAX	20

struct dhd_nfct_tuple {
	uint32 src_ip;
	uint32 dst_ip;
	uint16 sport;
	uint16 dport;
	uint8  proto;
};

/* Struct for storing Netlink socket info */
typedef struct dhd_nfct_info {
	dhd_pub_t *dhd;
	struct socket *nl_sock;
	struct sockaddr_nl *nl_local;
	/* NAT Offload Engine basic enables struct to handle FW events to
	 * enable/disble NF Conntrack notifications
	 */
	wl_event_data_natoe_t *natoe_info;
	uint32 subscriptions;
	void *prev_sk_data_ready;
} dhd_nfct_info_t;

/* Struct for NATOE port config ioctl */
typedef struct dhd_ct_ioc {
	wl_natoe_exception_port_t port_config;
	uint8 ioc_type;
} dhd_ct_ioc_t;

dhd_nfct_info_t *dhd_ct_open(dhd_pub_t *dhd, uint8 subsys_id, uint32 groups);
void dhd_ct_close(dhd_nfct_info_t *nfct);
void dhd_ct_send_dump_req(dhd_nfct_info_t *nfct);
int  dhd_ct_nl_bind(dhd_nfct_info_t *nfct, uint32 groups);
int  dhd_natoe_prep_send_exception_port_ioctl(dhd_pub_t *dhd, dhd_ct_ioc_t *ct_ioc);
void dhd_natoe_ct_ioctl_schedule_work(dhd_pub_t *dhd, dhd_ct_ioc_t *ioc);

#endif /* _NFCT_CONN_H */
