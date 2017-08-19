/*
 * RServ Dongle Simulation O/S Abstraction Header
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

struct lbuf {
	struct lbuf *next;
	struct lbuf *link;
	int len;
	void *data;
};

#define RSERV_PKT			struct lbuf
#define RSERV_PKTFREE(p, send)		dongle_pktfree((p), (send))
#define RSERV_PKTGET(len, send)		dongle_pktget((len), (send))
#define RSERV_PKTDATA(p)		((p)->data)
#define RSERV_PKTLEN(p)			((p)->len)
#define RSERV_PKTNEXT(p)		((p)->next)
#define RSERV_PKTSETNEXT(p, _next)	((p)->next = (_next))
#define RSERV_PKTLINK(p)		((p)->link)
#define RSERV_PKTSETLINK(p, _link)	((p)->link = (_link))
#define RSERV_MALLOC(len)		dongle_malloc(len)
#define RSERV_FREE(buf, len)		free(buf)

#define RSERV_OUTPUT(p)			dongle_output(p)

#define RSERV_IFCONFIG_SET(ifc)		dongle_if_config_set(ifc)
#define RSERV_IFCONFIG_GET(ifc)		dongle_if_config_get(ifc)
#define RSERV_IFSTATS_GET(ifs)		dongle_if_stats_get(ifs)
#define RSERV_IFCONTROL(ifctl, len)	dongle_if_control(ifctl, len)

struct lbuf *dongle_pktget(int len, int send);
void dongle_pktfree(struct lbuf *pkt, int send);

void *dongle_malloc(int len);
void dongle_output(struct lbuf *pkt);

int dongle_if_config_set(struct rsock_ifconfig *ifc);
int dongle_if_config_get(struct rsock_ifconfig *ifc);
int dongle_if_stats_get(struct rsock_ifstats *ifs);
int dongle_if_control(struct rsock_ifcontrol *ifctl, int len);
