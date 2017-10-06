/*
 * Dongle simulated bus interface
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

#ifndef _DEVBUS_H
#define _DEVBUS_H

#define DEVBUS_MTU		1518
#define DEVBUS_PORT_DEFAULT	7200

typedef struct {
	int serv_fd;
} devbus_t;

devbus_t *devbus_attach(int port);
int devbus_read(devbus_t *db, unsigned char **req_buf, struct timeval *tv);
void devbus_output(devbus_t *db, void *msg, int msg_len);
void devbus_close(devbus_t *db);

#endif /* _DEVBUS_H */
