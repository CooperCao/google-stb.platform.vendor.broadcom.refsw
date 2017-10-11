/*
 * RSERV: Remote Sockets Server (RTE-specific portion)
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

extern osl_t *rserv_osh;

#define RSERV_PKT			struct lbuf
#define RSERV_PKTFREE(p, send)		PKTFREE(rserv_osh, (p), (send))
#define RSERV_PKTGET(len, send)		rserv_rte_pktget((len), (send))
#define RSERV_PKTDATA(p)		((void *)PKTDATA(rserv_osh, (p)))
#define RSERV_PKTLEN(p)			PKTLEN(rserv_osh, (p))
#define RSERV_PKTNEXT(p)		((p)->next)
#define RSERV_PKTSETNEXT(p, _next)	((p)->next = (_next))
#define RSERV_PKTLINK(p)		((p)->link)
#define RSERV_PKTSETLINK(p, _link)	((p)->link = (_link))
#define RSERV_MALLOC(len)		rserv_rte_malloc(len)
#define RSERV_FREE(buf, len)		MFREE(rserv_osh, (buf), (len))

#define RSERV_OUTPUT(p)			rserv_rte_output(p)

#define RSERV_IFCONFIG_SET(ifc)		rserv_if_config_set(ifc)
#define RSERV_IFCONFIG_GET(ifc)		rserv_if_config_get(ifc)
#define RSERV_IFSTATS_GET(ifs)		rserv_if_stats_get(ifs)
#define RSERV_IFCONTROL(ifctl, len)	rserv_if_control(ifctl, len)

struct lbuf *rserv_rte_pktget(int len, int send);
void *rserv_rte_malloc(int len);
void rserv_rte_free(void *ptr, int len);
int rserv_rte_init(struct dngl *dngl, struct ether_addr *macaddr);
void rserv_rte_output(struct lbuf *pkt);
