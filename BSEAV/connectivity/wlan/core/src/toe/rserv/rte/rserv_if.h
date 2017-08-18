/*
 * RSERV Interface: a lwIP interface for the dongle wl driver
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

#ifndef __RSERV_IF_H__
#define __RSERV_IF_H__

int rserv_if_init(struct dngl *dngl, struct ether_addr *macaddr);
void rserv_if_input(struct lbuf *pkt);
int rserv_if_config_set(struct rsock_ifconfig *ifc);
int rserv_if_config_get(struct rsock_ifconfig *ifc);
int rserv_if_stats_get(struct rsock_ifstats *ifs);
int rserv_if_control(struct rsock_ifcontrol *ifctl, int len);
int rserv_if_deconfig(void);

#endif /* __RSERV_IF_H__ */
