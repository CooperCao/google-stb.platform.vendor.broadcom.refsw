/*
 *  URB declaration
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#ifndef _URB_H_
#define _URB_H_

#include <typedefs.h>
#include <usbtypes.h>

#define URB_ATTRIBUTE_UNLIMITED 0x0001
#define URB_ATTRIBUTE_NOT_LAST	0x0002

struct _usb_endpoint;

typedef struct _urb_t {
	struct _usb_endpoint *ep;
	uint16 expected_length;
	uint16 finished_length;
	uint8 *data_buffer;
	uint16 attribute;
	bdc_status_code status;
	uint8 mpaf_port;
	void *msg_ptr;
	struct _urb_t *next;

	void (*complete_callback)(struct _urb_t *urb);
	void (*transfer_callback)(struct _urb_t *urb, int bytes_available);
} urb_t;

#endif /* _URB_H_ */
