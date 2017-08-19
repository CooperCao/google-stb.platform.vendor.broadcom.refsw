/*
 *  USB endpoint0 API.
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

#ifndef _USBENDPOINT0_H_
#define _USBENDPOINT0_H_

#include <usbendpoint.h>

struct _usb_controller;

typedef struct _usb_endpoint0 {
	usb_endpoint base;
	uint8 setup[8];
} usb_endpoint0;

void usbendpoint0_init(usb_endpoint0 *ep, struct _usb_controller *);

#endif
