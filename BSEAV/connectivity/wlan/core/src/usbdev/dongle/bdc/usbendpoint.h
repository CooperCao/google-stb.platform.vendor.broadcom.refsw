/*
 *  USB endpoint root class API
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

#ifndef _USBENDPOINT_H_
#define _USBENDPOINT_H_

#include <typedefs.h>
#include <usbtypes.h>

#define ENDPOINT_TYPE_CTRL	0
#define ENDPOINT_TYPE_ISOC	1
#define ENDPOINT_TYPE_BULK	2
#define ENDPOINT_TYPE_INTR	3

#define ENDPOINT_STATUS_HALT 0x01

typedef enum _endpoint_usage {
	ENDPOINT_USAGE_CONTROL,
	ENDPOINT_USAGE_INTERRUPT_IN,
	ENDPOINT_USAGE_BULK_IN,
	ENDPOINT_USAGE_BULK_OUT,
	ENDPOINT_USAGE_ISOC_IN,
	ENDPOINT_USAGE_ISOC_OUT,
	ENDPOINT_USAGE_DEBUG_BULK_IN,
	ENDPOINT_USAGE_DEBUG_BULK_OUT,
	ENDPOINT_USAGE_KEYBOARD_IN,
	ENDPOINT_USAGE_MOUSE_IN
} endpoint_usage;

struct _usb_device;
struct _usb_controller;

typedef struct _usb_endpoint {
	uint8 address;	/* MSB = 1 - IN transfer */
	uint8 type;
	uint8 max_packet_size;	/* 2's power, i.e. 3 = 8 bytes, 6 = 64 bytes. */
	uint8 ep_status;
	uint16 fifo_size;
	uint16 pending_size;
	uint8 *fifo_address;
	/*
	 * uint8 max_rXrsv_buff;
	 * uint8 malloc_fail;
	 */
	struct _usb_device *device;
	struct _urb_t *urb_list;

	bdc_status_code
		(*transfer_start)(struct _urb_t *);
	void (*transfer_cancel)(struct _urb_t *);
	void (*clear_or_set_stall)(struct _usb_endpoint *, int is_set);
	void (*set_alt_setting)(struct _usb_endpoint *, int altsetting, int packet_size);
} usb_endpoint;

void usbendpoint_init(usb_endpoint *ep, struct _usb_controller *);

#endif /* _USBENDPOINT_H_ */
