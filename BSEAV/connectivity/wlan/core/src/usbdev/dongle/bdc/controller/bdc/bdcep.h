/*
 *  Defines end point for BDC
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

#ifndef _BDCEP_H_
#define _BDCEP_H_

#include <usbendpoint.h>
#include <urb.h>
#include <bdcbdlist.h>
#include <usbcontroller.h>

#define BDCEP_INTERRUPT_IN	0x81
#define BDCEP_BULK_IN		0x82
#define BDCEP_BULK_OUT		0x02
#define BDCEP_ISOC_IN		0x83
#define BDCEP_ISOC_OUT		0x03
#define BDCEP_DEBUG_BULK_IN	0x84
#define BDCEP_DEBUG_BULK_OUT	0x04
#define BDCEP_KEYBOARD_IN	0x85
#define BDCEP_MOUSE_IN		0x86

#define BDCEP_STATUS_ENABLED		0x01
#define BDCEP_STATUS_STALLED		0x02
#define BDCEP_STATUS_TRANSFER_READY	0x04
#define BDCEP_STATUS_DATA_READY		0x08

struct _usb_controller;
struct _bdcd;

typedef struct _bdc_ep {
	usb_endpoint base;

	uint8 status;
	uint8 index;
	uint8 bd_count;
	uint8 reserved1;
	uint16 max_packet_size;
	uint16 packet_size;

	bdc_bd_list transfer_bd_list;

	void (*transfer_event)(struct _bdc_ep *ep, struct _bdcd *bdc, bdc_bd *bd);
} bdc_ep;

void bdcep_init(bdc_ep *ep, struct _usb_controller *controller);
bdc_status_code bdcep_bulk_transfer(bdc_ep *ep, struct _bdcd *bdc, urb_t *urb,
	uint32 max_transfer_size);
void bdcep_config(bdc_ep *ep, struct _bdcd *bdc);
void bdcep_un_config(bdc_ep *ep, struct _bdcd *bdc);
void bdcep_transfer_event(bdc_ep *ep, struct _bdcd *bdc, bdc_bd *evt);

#endif /* _BDCEP_H_ */
