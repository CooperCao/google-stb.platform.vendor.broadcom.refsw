/*
 * RSock User Mode Host Bus Interface Definition
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

#ifndef _HOSTBUS_H
#define _HOSTBUS_H

void hostbus_attach(void);
int hostbus_mtu(void);
int hostbus_read(void **req_buf);
int hostbus_output(void *msg, int msg_len);
void hostbus_close(void);

#endif /* _HOSTBUS_H */
