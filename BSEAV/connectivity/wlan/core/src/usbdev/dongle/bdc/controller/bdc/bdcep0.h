/*
 *  Defines end point 0 for BDC
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

#ifndef _BDCEP0_H_
#define _BDCEP0_H_

#include <usbendpoint0.h>
#include <bdcbdlist.h>

#define BDCEP0_STATUS_DATA_STAGE	0x01
#define BDCEP0_STATUS_STATUS_STAGE	0x02
#define BDCEP0_STATUS_COMPLETED		0x04
#define BDCEP0_STATUS_STALL		0x08
#define BDCEP0_STATUS_SET_ADDRESS	0x10
#define BDCEP0_STATUS_EXPECTING_DATA	0x20
#define BDCEP0_STATUS_PENDING_RESET	0x40

struct _usb_controller;
struct _bdcd;

typedef struct _bdc_ep0 {
	usb_endpoint0 base;

	uint8 status;
	uint8 reserved1;
	uint16 reserved2;

	bdc_bd_list transfer_bd_list;

	void (*transfer_event)(struct _bdc_ep0 *ep0, struct _bdcd *bdc, bdc_bd *bd);
} bdc_ep0;

void bdcep0_set_speed(bdc_ep0 *ep0, uint32 speed);
void bdcep0_init(bdc_ep0 *ep0, struct _usb_controller *);
void bdcep0_status_start(bdc_ep0 *ep0, struct _bdcd *bdc);
void bdcep0_setup_complete(bdc_ep0 *ep0, struct _bdcd *bdc, bdc_status_code status);
void bdcep0_config(bdc_ep0 *ep0, struct _bdcd *bdc);
void bdcep0_un_config(bdc_ep0 *ep, struct _bdcd *bdc);

#endif /* _BDCEP0_H_ */
