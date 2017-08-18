/*
 * Virtual Ethernet Client Library Definitions
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

#ifndef _VENETC_H
#define _VENETC_H

typedef struct {
	int serv_fd;
	unsigned char mac[6];
} venetc_t;

extern venetc_t *venetc_open(char *hostname, int port);
extern int venetc_close(venetc_t *vt);

extern int venetc_send(venetc_t *vt, unsigned char *msg, int msglen);
extern int venetc_recv(venetc_t *vt, unsigned char **msg_ptr, int *msglen_ptr);
extern void venetc_free(venetc_t *vt, unsigned char *msg);

#endif /* _VENETC_H */
