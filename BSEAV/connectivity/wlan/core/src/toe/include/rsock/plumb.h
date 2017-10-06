/*
 * Remote Sockets Plumbing
 *
 * In order to port rsock into a given O/S environment, a file os.h must
 * be written to abstract out O/S memory, packet and synchronization
 * primitives.  At runtime, the application must call rsock_init() at
 * startup, and then call rsock_input() to pass in each rsock protocol
 * packet received from the rserv device.
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

#ifndef _BCMRSOCK_PLUMB_H
#define _BCMRSOCK_PLUMB_H

int rsock_init(void);
void rsock_input(os_pkt_t *pkt);
void rsock_term(void);

#endif /* _BCMRSOCK_PLUMB_H */
