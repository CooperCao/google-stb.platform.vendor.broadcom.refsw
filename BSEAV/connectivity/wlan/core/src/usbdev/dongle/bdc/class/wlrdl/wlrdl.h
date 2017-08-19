/*
 * Wl Rdl Class API..
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

#ifndef _USBWLRDL_H_
#define _USBWLRDL_H_


#include <usbendpoint.h>
#include <usbclass.h>
#include <usbconst.h>
#include <usbdevice.h>

#define QUERY_STRING_MAX 32

typedef struct _wl_rdl {
	usb_class_t base;
	usb_endpoint *ep_in1;
	usb_endpoint *ep_in2;
	usb_endpoint *ep_out2;

	urb_t ep_in1Urb;
	urb_t ep_in2Urb;
	urb_t ep_out2Urb;
} wl_rdl;

extern uint8 usb_rdl_dev_des_buf[USB_DEVDES_SIZE];
extern uint8 usb_rdl_cfg_des_buf[USB_DFLT_RDL_CFGDES_SIZE];

void wlrdl_set_class(wl_rdl *wlrdl, usb_device_t *device);
void wlrdl_init(wl_rdl *wlrdl, usb_device_t *device);
void * usb_wl_create(void *device);
extern const usb_class_function_table wl_rdl_funcs;

extern usb_controller *g_usb_controller;
extern void (*controller_sco_handler)(usb_controller *controller, uint8 packet_size);

#endif	/* _USBWLRDL_H_ */
