/*
 * Part of Armulator shared object plug-in to emulate a full dongle plus
 * simulated wireless interface and simulated host bus interface
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

#ifndef _dnglperiph_h_
#define _dnglperiph_h_

extern int dp_init(void);

extern void dp_poll(uint64 usec);
extern int dp_main_loop(void);

extern void dp_wlif_send(unsigned char *msg, int msglen);
extern void dp_usbif_send(unsigned char *msg, int msglen);

#endif /* _dnglperiph_h_ */
