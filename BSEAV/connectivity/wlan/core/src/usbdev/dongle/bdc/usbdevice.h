/*
 *  USB device API.
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

#ifndef _USBDEVICE_H_
#define _USBDEVICE_H_

#include <typedefs.h>
#include <urb.h>
#include <usbclass.h>
#include <usbendpoint0.h>
#include <usbcontroller.h>

#define USBDEVICE_STRING_MAX 4

#define USBDEVICE_STATUS_ENUMERATED	0x01
#define USBDEVICE_STATUS_SUSPENDED	0x02
#define USBDEVICE_STATUS_RESUMING	0x04
#define USBDEVICE_STATUS_WAIT_MEMORY	0x08

#define USBDEVICE_MODE_TO_POLLING 0x01

#define USBDEVICE_SPEED_UNKNOWN 0x00
#define USBDEVICE_SPEED_FULL	0x01
#define USBDEVICE_SPEED_LOW	0x02
#define USBDEVICE_SPEED_HIGH	0x03
#define USBDEVICE_SPEED_SUPER	0x04

#define USBDEVICE_FEATURE_REMOTE_WAKEUP 1
#define USBDEVICE_FEATURE_TEST_MODE	2

#define USBDEVICE_MAX_GENERAL_DATA 2

struct _usb_controller;
struct _usb_class;

typedef struct _usb_device {
	struct _usb_controller *controller;
	struct _usb_class *usb_class;
	struct _usb_device_function_table *ft;

	uint8 *device_descriptor;
	uint8 *config_descriptor;
	uint8 *string_descriptor[USBDEVICE_STRING_MAX];

	uint8 state;
	uint8 max_string_index;
	uint8 configuration;
	uint8 usbd_current_status;	/* Bit field - status of this device */
	uint8 running_mode;
	uint8 speed;
	uint8 general_data[USBDEVICE_MAX_GENERAL_DATA];

	usb_endpoint0 *ep0;
	urb_t setup_urb;

	uint16 config_descriptor_size;
	uint16 max_configuration_size;
	uint8 string_descriptor_size[USBDEVICE_STRING_MAX];
} usb_device_t;

typedef struct _usb_device_function_table {
	void (*start)(usb_device_t *);
	void (*stop)(usb_device_t *);
	void (*bus_reset)(usb_device_t *);
	void (*bus_suspended)(usb_device_t *);
	void (*device_reset)(usb_device_t *);
	void (*setup)(usb_device_t *);
	void (*urb_complete)(usb_device_t *, urb_t *urb);
	void (*connected)(usb_device_t *, uint8 speed);
	void (*disconnected)(usb_device_t *);
	void (*switch_to_polling_mode)(usb_device_t *);
	void (*switch_to_normal_mode)(usb_device_t *);
} usb_device_function_table;

/*******************************************************************************
* Function Prototypes
******************************************************************************
*/
/* void usbdevice_set_wait_memory(usb_device_t* device); */
void usbdevice_clear_wait_memory(usb_device_t *device);

void usbdevice_init(usb_device_t *device, struct _usb_controller *);
void usb_device_create(void *controller, usb_device_t *device);
void usbdevice_add_string_descriptor(usb_device_t *device, uint8 *string_descriptor, uint8 siz);
void usbdevice_set_configure_descriptor(usb_device_t *device, uint8 descriptor[], uint16 max_size);
uint8 * usbdevice_add_configure_descriptor(struct _usb_class *, uint8 descriptor[], bool attach);
void usbdevice_set_class(usb_device_t *device, struct _usb_class *cls);
void usbdevice_create_composite_device(usb_device_t *device, uint8 *dev_desc,
	uint8 *config_desc_buf, uint16 max_size);
uint8 * usbdevice_add_iad(usb_device_t *device);
void usbdevice_set_iad(usb_device_t *device, struct _usb_class *composite_class,
		uint8 *iad, uint8 *desc);
void usbdevice_add_to_iad(usb_device_t *device);
void usbdevice_setup(usb_device_t *device);
void usbdevice_set_class_descriptor(usb_device_t *device, uint8 cfa_port, uint8 *desc);

extern bool is_composite_device_used;

#endif /* _USBDEVICE_H_ */
