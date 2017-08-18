/*
 *  USB class root class API.
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

#ifndef _USBCLASS_H_
#define _USBCLASS_H_

#include <typedefs.h>
#include <usbendpoint.h>

#define USBCLASS_STATUS_WAIT_MEMORY 0x01

struct _usb_device;

typedef struct _usb_class {
	struct _usb_device *device;
	struct _usb_class *next;
	struct _usb_class_function_table *ft;

	uint8 *interface_desc;
	uint16 interface_desc_len;
	uint16 reserved1;
	uint8 interface_low;
	uint8 interface_high;
	uint8 usbc_current_status;
} usb_class_t;

typedef struct _usb_class_function_table {
	void (*start)(usb_class_t *);
	void (*stop)(usb_class_t *);
	void (*bus_reset)(usb_class_t *);
	void (*suspended)(usb_class_t *);
	void (*resumed)(usb_class_t *);
	bdc_status_code
		(*vendor_command)(usb_class_t *);
	bdc_status_code
		(*setup)(usb_class_t *);
	bdc_status_code
		(*configured)(usb_class_t *, int config);
	bdc_status_code
		(*alt_setting)(usb_class_t *, int interface, int altsetting);
	void (*polling)(usb_class_t *);
	void (*send_next_transfer)(usb_class_t *);
	void (*memory_available)(usb_class_t *);
	void (*set_class_descriptor)(usb_class_t *, uint8 cfa_port, uint8 *desc);
} usb_class_function_table;

void usbclass_start(usb_class_t *cls);
void usbclass_stop(usb_class_t *cls);
void usbclass_init(usb_class_t *cls, struct _usb_device *);
bdc_status_code usbclass_setup(usb_class_t *cls);
bdc_status_code usbclass_vendor_command(usb_class_t *cls);
void usbclass_suspended(usb_class_t *cls);
void usbclass_resumed(usb_class_t *cls);
void usbclass_bus_reset(usb_class_t *cls);
void usbclass_set_wait_for_memory(usb_class_t *cls);
void usbclass_clear_wait_for_memory(usb_class_t *cls);
bdc_status_code usbclass_get_ep_status(usb_class_t *cls, usb_endpoint *ep, uint8 *setup);
bdc_status_code usbclass_feature(usb_class_t *cls, usb_endpoint *ep, uint8 *setup, int is_set);
void usbclass_replace_descriptor(usb_class_t *cls, uint8 *new_desc);
void usbclass_polling(usb_class_t *cls);
void usbclass_set_class_descriptor(usb_class_t *cls, uint8 cfa_port, uint8 *desc);
bdc_status_code usbclass_configured(usb_class_t *cls, int config);
bdc_status_code usbclass_alt_setting(usb_class_t *cls, int interface, int altsetting);
void usbclass_memory_available(usb_class_t *cls);
void usbclass_send_next_transfer(usb_class_t *cls);

#endif /* _USBCLASS_H_ */
