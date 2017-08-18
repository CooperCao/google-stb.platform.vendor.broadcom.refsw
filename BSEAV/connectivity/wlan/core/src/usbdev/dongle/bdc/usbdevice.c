/*
 *  USB constant definitions.
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmstdlib.h>
#include <osl.h>
#include <osl_ext.h>
#include <bdc_usb.h>
#include <usbtypes.h>
#include <usbclass.h>
#include <usbdevice.h>
#include <usbendpoint.h>
#include <usbcontroller.h>

#ifdef USB_BYPASS_REENUM
/*
 * ===============================================================================================
 * ! ALL USB globals are placed in a separate region.  If no re-enumeration is required on
 * reboot,
 * ! all USB variables will retain their value.	This header must be applied to all USB files
 * ===============================================================================================
 */
#pragma arm section rwdata = "usb_area_rw" zidata = "usb_area_zi"
#endif

#if defined(MPAF_ENABLE) && defined(BT_USB11)
#define USBDEVICE_MAX_INSTANCE 4
#else
#define USBDEVICE_MAX_INSTANCE 1
#endif

uint8 usb0_configure;
uint8 usb0_interface;
uint8 usb0_alt_setting;

void usbdevice_start(usb_device_t *device);
void usbdevice_stop(usb_device_t *device);
void usbdevice_bus_reset(usb_device_t *device);
void usbdevice_bus_suspended(usb_device_t *device);
void usbdevice_device_reset(usb_device_t *device);
void usbdevice_setup(usb_device_t *device);
void usbdevice_urb_complete(usb_device_t *device, urb_t *urb);
void usbdevice_connected(usb_device_t *device, uint8 speed);
void usbdevice_disconnected(usb_device_t *device);
void usbdevice_switch_to_polling_mode(usb_device_t *device);
void usbdevice_switch_to_normal_mode(usb_device_t *device);

bdc_status_code usbdevice_get_descriptor(usb_device_t *device);
bdc_status_code usbdevice_set_configuration(usb_device_t *device);
bdc_status_code usbdevice_get_configuration(usb_device_t *device);
bdc_status_code usbdevice_set_interface(usb_device_t *device);
bdc_status_code usbdevice_get_interface(usb_device_t *device);
bdc_status_code usbdevice_feature(usb_device_t *device, int is_set);
bdc_status_code usbdevice_get_status(usb_device_t *device);
void usbdevice_data_complete(urb_t *urb);
void usbdevice_adjust_interface(usb_class_t *a_class, uint8 *desc, uint16 length);

usb_device_t usb_devices[USBDEVICE_MAX_INSTANCE];
uint32 usb_device_instances_index = 0;
#ifdef USB_BDC_DEFAULT_COMPOSITE
bool is_composite_device_used = TRUE;
#else
bool is_composite_device_used = FALSE;
#endif

extern usb_controller *g_usb_controller;

/* PLACE_IN_DROM const usb_device_function_table usb_device_function_table_rom = */
const usb_device_function_table usb_device_function_table_rom =
{
	usbdevice_start,	/* start */
	usbdevice_stop,	/* stop */
	usbdevice_bus_reset,	/* bus_reset */
	usbdevice_bus_suspended,	/* bus_suspended */
	usbdevice_device_reset,	/* device_reset */
	usbdevice_setup,	/* setup */
	usbdevice_urb_complete,	/* urb_complete */
	usbdevice_connected,	/* connected */
	usbdevice_disconnected,	/* disconnected */
	usbdevice_switch_to_polling_mode,	/* switch_to_polling_mode */
	usbdevice_switch_to_normal_mode,	/* switch_to_normal_mode */
};

void usb_register_wait_memory(void);

void
usbdevice_start(usb_device_t *device)
{
	device->controller->ft->start(device->controller);
}

void
usbdevice_stop(usb_device_t *device)
{
	device->controller->ft->stop(device->controller);
}

void
usbdevice_clear_wait_memory(usb_device_t *device)
{
	device->usbd_current_status &= ~USBDEVICE_STATUS_WAIT_MEMORY;
}

void
usbdevice_bus_reset(usb_device_t *device)
{
	bdc_fn(("%s\n", __FUNCTION__));

	usb_class_t *a_class = device->usb_class;
	usb_controller *controller = device->controller;
	usb_endpoint *ep0 = (usb_endpoint *)device->ep0;

	while (NULL != a_class) {
		a_class->ft->bus_reset(a_class);
		a_class = a_class->next;
	}

	/* EP0 cannot be destroyed. We only need to cancel all pending transfers. */
	if (NULL != ep0) {
		controller->ft->ep_cancel_all_transfer(controller, ep0);
	}

	device->usbd_current_status = 0;
	device->configuration = 0;

	if (USB_PORT_1 == controller->ft->get_port(controller)) {
		usb0_configure = 0;
		usb0_interface = 0;
		usb0_alt_setting = 0;
	}
}

void
usbdevice_bus_suspended(usb_device_t *device)
{
	usb_class_t *a_class;

	bdc_fn(("%s\n", __FUNCTION__));

	device->usbd_current_status |= USBDEVICE_STATUS_SUSPENDED;

	a_class = device->usb_class;
	while (NULL != a_class) {
		a_class->ft->suspended(a_class);
		a_class = a_class->next;
	}
	/* Only continue when it is the only device or internal hub */
	if (device->controller != g_usb_controller) {
		return;
	}

	if (device->usbd_current_status & USBDEVICE_STATUS_ENUMERATED) {
		/* Need to find how to do it in WLAN */
		/* TODO bttransport_set_host_wake(FALSE); */
	}
}

void
usbdevice_device_reset(usb_device_t *device)
{
	bdc_fn(("%s\n", __FUNCTION__));
	device->controller->ft->reset(device->controller);
}

bdc_status_code
usbdevice_get_descriptor(usb_device_t *device)
{
	urb_t *urb;
	uint8 *setup;
	int length;
	int index;

	bdc_fn(("%s\n", __FUNCTION__));

	setup = device->ep0->setup;
	if (USB_DIR_DEVICE_TO_HOST != USB_SETUP_BM_REQUEST_TYPE(setup))	{
		return BDC_STATUS_UNKNOWN;
	}

	urb = &device->setup_urb;
	switch (USB_SETUP_VALUE_MSB(setup)) {
	case USB_DEVICE_DESC_TYPE:
		length = USB_DEVICE_DESC_LENGTH;
		urb->data_buffer = device->device_descriptor;
		bdc_dbg(("Received: USB_DEVICE_DESC_TYPE %x\n", urb->data_buffer[0]));
		break;
	case USB_CONFIG_DESC_TYPE:
		length = device->config_descriptor_size;
		urb->data_buffer = device->config_descriptor;
		bdc_dbg(("Received: USB_CONFIG_DESC_TYPE  %x\n", urb->data_buffer[0]));
		break;
	case USB_STRING_DESC_TYPE:
		index = USB_SETUP_VALUE_LSB(setup);
		if (index >= USBDEVICE_STRING_MAX) {
			return BDC_STATUS_UNKNOWN;
		}

		urb->data_buffer = device->string_descriptor[index];
		length = device->string_descriptor_size[index];
		bdc_dbg(("Received: USB_STRING_DESC_TYPE  %c\n", urb->data_buffer[0]));
		break;
	default:
		return BDC_STATUS_UNKNOWN;
	}

	urb->expected_length = length;

	bdc_dbg(("urb->expected_length = %d \n", urb->expected_length));
	/* set_func_checkpoints2(BOOT_CHECK2_USB_GET_DESCRIPTOR); */

	return BDC_STATUS_DATA_READY;
}

bdc_status_code
usbdevice_set_configuration(usb_device_t *device)
{
	usb_class_t *a_class;
	uint8 *setup;
	bdc_status_code status;
	usb_controller *controller = device->controller;

	bdc_fn(("%s\n", __FUNCTION__));

	/* usb_enumerated_flag |= USB_PORT_1_MASK << controller->ft->get_port(controller); */
	device->usbd_current_status |= USBDEVICE_STATUS_ENUMERATED;

	setup = device->ep0->setup;
	status = BDC_STATUS_SUCCESS;
	if (device->configuration) {
		if (device->configuration != setup[2]) {
			a_class = device->usb_class;
			while (NULL != a_class) {
				a_class->ft->configured(a_class, 0);
				a_class = a_class->next;
			}
		} else {
			return status;
		}
	}

	if (USB_PORT_1 == controller->ft->get_port(controller)) {
		usb0_configure = setup[2];
	}

	a_class = device->usb_class;
	while (NULL != a_class) {
		status = a_class->ft->configured(a_class, setup[2]);
		if (BDC_STATUS_SUCCESS != status) {
			break;
		}

		a_class = a_class->next;
	}

	if (BDC_STATUS_SUCCESS == status) {
		device->configuration = setup[2];
	}

	return status;
}

bdc_status_code
usbdevice_get_configuration(usb_device_t *device)
{
	uint8 *setup;
	urb_t *urb;

	bdc_fn(("%s\n", __FUNCTION__));

	setup = device->ep0->setup;
	if (0 != setup[2] || 0 != setup[3] || 0 != setup[4] || 0 != setup[5] ||
		1 != setup[6] || 0 != setup[7]) {
		return BDC_STATUS_INVALID_FIELD;
	}

	device->general_data[0] = device->configuration;

	urb = &device->setup_urb;
	urb->data_buffer = device->general_data;
	urb->expected_length = 1;

	return BDC_STATUS_DATA_READY;
}

bdc_status_code
usbdevice_set_interface(usb_device_t *device)
{
	usb_class_t *a_class;
	uint8 *setup;
	uint16 w_index;
	bdc_status_code status;
	usb_controller *controller = device->controller;

	bdc_fn(("%s\n", __FUNCTION__));

	status = BDC_STATUS_SUCCESS;
	setup = device->ep0->setup;
	a_class = device->usb_class;
	w_index = USB_SETUP_INDEX(setup);
	while (NULL != a_class) {
		if (w_index >= a_class->interface_low && w_index <= a_class->interface_high) {
			status = a_class->ft->alt_setting(a_class, setup[4], setup[2]);
			break;
		}

		a_class = a_class->next;
	}

	if (USB_PORT_1 == controller->ft->get_port(controller)) {
		/* Needed for ram downloading */
		usb0_interface = setup[4];
		usb0_alt_setting = setup[2];
	}

	return status;
}

bdc_status_code
usbdevice_get_interface(usb_device_t *device)
{
	uint8 *setup;
	urb_t *urb;

	bdc_fn(("%s\n", __FUNCTION__));

	setup = device->ep0->setup;
	if (0 != setup[2] || 0 != setup[3] || usb0_interface != setup[4] || 0 != setup[5] ||
		1 != setup[6] || 0 != setup[7]) {
		return BDC_STATUS_INVALID_FIELD;
	}

	device->general_data[0] = usb0_alt_setting;

	urb = &device->setup_urb;
	urb->data_buffer = device->general_data;
	urb->expected_length = 1;

	return BDC_STATUS_DATA_READY;
}

void
usbdevice_connected(usb_device_t *device, uint8 speed)
{
	bdc_fn(("%s\n", __FUNCTION__));

	bdc_dbg(("USB Device connect, speed = %d \n", speed));

	device->speed = speed;
}

void
usbdevice_disconnected(usb_device_t *device)
{
	bdc_fn(("%s\n", __FUNCTION__));
	device->speed = USBDEVICE_SPEED_UNKNOWN;
}

bdc_status_code
usbdevice_feature(usb_device_t *device, int is_set)
{
	uint8 *setup;
	uint16 feature;
	usb_controller *controller = device->controller;

	bdc_fn(("%s\n", __FUNCTION__));

	setup = device->ep0->setup;
	feature = setup[2] | (setup[3] << 8);
	if (0 != setup[0] ||
	    (USBDEVICE_FEATURE_REMOTE_WAKEUP != feature && USBDEVICE_FEATURE_TEST_MODE !=
	     feature)) {
		return BDC_STATUS_INVALID_FIELD;
	}

	return controller->ft->feature(controller, feature, is_set);
}

bdc_status_code
usbdevice_get_status(usb_device_t *device)
{
	uint8 *setup;
	urb_t *urb;
	uint8 device_status;

	bdc_fn(("%s\n", __FUNCTION__));

	urb = &device->setup_urb;
	setup = device->ep0->setup;

	if (USB_DIR_DEVICE_TO_HOST != setup[0])	{
		return BDC_STATUS_INVALID_FIELD;
	}

	/* ASSERT(USBDEVICE_MAX_GENERAL_DATA >= 2); */
	device->general_data[0] = 0;
	device->general_data[1] = 0;

	device_status = device->config_descriptor[7];	/* bm_attributes */
	if (device_status & 0x40) {
		device->general_data[0] |= 0x01;	/* Self-powered */
	}

	if (device->controller->attribute & USBDEVICE_FEATURE_REMOTE_WAKEUP) {
		device->general_data[0] |= 0x02;	/* Support Remote Wakeup */
	}

	urb->data_buffer = device->general_data;
	urb->expected_length = 2;
	if (urb->expected_length > setup[6]) {
		urb->expected_length = setup[6];
	}

	return BDC_STATUS_DATA_READY;
}

void
usbdevice_data_complete(urb_t *urb)
{
	usb_device_t *device;
	uint8 *setup;
	usb_controller *controller;

	bdc_fn(("%s\n", __FUNCTION__));

	if (BDC_STATUS_BUS_RESET == urb->status) {
		return;
	}

	device = urb->ep->device;
	controller = device->controller;
	if (urb->expected_length &&
		((urb->expected_length % (1 << urb->ep->max_packet_size)) == 0)) {
		setup = device->ep0->setup;
		if (USB_SETUP_LENGTH(setup) > urb->expected_length) {
			/* Need to send a ZLP */
			urb->expected_length = 0;
			controller->ft->transfer_start(controller, urb);
			return;
		}
	}

	controller->ft->setup_complete(controller, urb->status);
}

void
usbdevice_setup(usb_device_t *device)
{
	usb_class_t *a_class;
	uint8 *setup;
	bdc_status_code status;
	int asking_length;
	urb_t *urb;
	usb_controller *controller = device->controller;

	bdc_fn(("%s\n", __FUNCTION__));

	status = BDC_STATUS_UNKNOWN;
	setup = device->ep0->setup;
	device->setup_urb.complete_callback = usbdevice_data_complete;

	a_class = device->usb_class;
	while (NULL != a_class) {
		status = a_class->ft->setup(a_class);
		if (BDC_STATUS_UNKNOWN != status) {
			break;
		}

		a_class = a_class->next;
	}

	if (BDC_STATUS_UNKNOWN == status) {
		switch (USB_SETUP_B_REQUEST(setup)) {
		case USB_STD_GET_DESCRIPTOR_REQ_VALUE:
			status = usbdevice_get_descriptor(device);
			break;
		case USB_STD_GET_CONFIG_REQ_VALUE:
			status = usbdevice_get_configuration(device);
			break;
		case USB_STD_SET_CONFIG_REQ_VALUE:
			status = usbdevice_set_configuration(device);
			break;
		case USB_STD_GET_INTERFACE_REQ_VALUE:
			status = usbdevice_get_interface(device);
			break;
		case USB_STD_SET_INTERFACE_REQ_VALUE:
			status = usbdevice_set_interface(device);
			break;
		case USB_STD_CLEAR_FEATURE_REQ_VALUE:
			status = usbdevice_feature(device, 0);
			break;
		case USB_STD_SET_FEATURE_REQ_VALUE:
			status = usbdevice_feature(device, 1);
			break;
		case USB_STD_GET_STATUS_REQ_VALUE:
			status = usbdevice_get_status(device);
			break;
		default:
			status = BDC_STATUS_UNKNOWN;
			break;
		}
	} else if (BDC_STATUS_PENDING == status)   {
		/* Another thread will provide data. Don't do anything here */
		return;
	}

	if (BDC_STATUS_SUCCESS == status) {
		goto complete;
	}

	if (BDC_STATUS_DATA_READY != status) {
		goto complete;
	}

	urb = &device->setup_urb;
	asking_length = USB_SETUP_LENGTH(setup);
	if (asking_length < urb->expected_length)	{
		urb->expected_length = asking_length;
	}

	if (urb->expected_length == 0) {
		status = BDC_STATUS_SUCCESS;
		goto complete;
	}

	if (setup[0] & USB_DIR_DEVICE_TO_HOST) {
		urb->ep->address |= 0x80;
	} else {
		urb->ep->address &= ~0x80;
	}

	controller->ft->transfer_start(controller, urb);
	return;

complete:
	controller->ft->setup_complete(controller, status);
}

void
usbdevice_urb_complete(usb_device_t *device, urb_t *urb)
{
	bdc_fn(("%s\n", __FUNCTION__));
	if (urb->complete_callback) {
		urb->complete_callback(urb);
	}
}

void
usbdevice_adjust_interface(usb_class_t *a_class, uint8 *desc, uint16 length)
{
	usb_device_t *device;
	uint8 max_interface;
	uint8 interface_index;

	bdc_fn(("%s\n", __FUNCTION__));

	device = a_class->device;
	max_interface = 0xFF;
	interface_index = a_class->interface_low;
	a_class->interface_high = interface_index;

	while (length) {
		if (USB_INTERFACE_DESC_TYPE == desc[1])	{
			if (desc[2] != max_interface) {
				device->config_descriptor[4]++;
				a_class->interface_high = interface_index++;
			}

			max_interface = desc[2];
			desc[2] = a_class->interface_high;
		}

		length -= desc[0];
		desc += desc[0];
	}

	/* ASSERT(a_class->interface_high >= a_class->interface_low); */
}

uint8 *
usbdevice_add_iad(usb_device_t *device)
{
	uint8 *iad;
	uint8 *desc;
	int length;

	bdc_fn(("%s\n", __FUNCTION__));

	/* dbguart_print_word_in_ascii("xdc_setup_received: reset ep0, trb=", TVF_D(trb)); */
	iad = &device->config_descriptor[device->config_descriptor_size];
	length = device->max_configuration_size - device->config_descriptor_size - 8;
	memmove(iad + 8, iad, length);

	iad[0] = 8;
	iad[1] = USB_INTFC_ASSOC_DESC_TYPE;
	iad[2] = 0;
	iad[3] = 0;
	iad[4] = 0;
	iad[5] = 0;
	iad[6] = 0;
	iad[7] = 0;

	desc = device->config_descriptor;
	length = device->config_descriptor_size;
	length += 8;
	desc[2] = (uint8)(length & 0xFF);
	desc[3] = (uint8)(length >> 8);
	device->config_descriptor_size = length;

	/* Change device descriptor class to multi-interface function class */
	desc = device->device_descriptor;
	desc[4] = 0xEF;
	desc[5] = 0x02;
	desc[6] = 0x01;

	return iad;
}

void
usbdevice_set_iad(usb_device_t *device, usb_class_t *composite_class, uint8 *iad, uint8 *desc)
{
	uint8 *first_interface;
	uint16 length;

	/* ASSERT(iad); */

	bdc_fn(("%s\n", __FUNCTION__));

	length = device->config_descriptor_size - (iad - device->config_descriptor);
	first_interface = NULL;
	while (length) {
		if (USB_INTERFACE_DESC_TYPE == desc[1])	{
			first_interface = desc;
			break;
		}

		length -= desc[0];
		desc += desc[0];
	}

	if (NULL != first_interface) {
		iad[2] = composite_class->interface_low;
		iad[3] = composite_class->interface_high - composite_class->interface_low + 1;
		iad[4] = first_interface[5];
		iad[5] = first_interface[6];
		iad[6] = first_interface[7];
		iad[7] = first_interface[8];
	}
}

void
usbdevice_add_to_iad(usb_device_t *device)
{
	uint8 *iad;
	uint8 *desc;
	uint16 length;

	bdc_fn(("%s\n", __FUNCTION__));

	desc = device->config_descriptor;
	length = device->config_descriptor_size;
	iad = NULL;
	while (length) {
		if (USB_INTFC_ASSOC_DESC_TYPE == desc[1]) {
			iad = desc;
		}

		length -= desc[0];
		desc += desc[0];
	}

	if (NULL != iad) {
		iad[3]++;
	}
}

/*
 * Assume the buffer for first USB device is big enough to hold all added
 * descriptors. To insure that, there is a check to make sure it is not
 * broken.
 */
uint8 *
usbdevice_add_configure_descriptor(usb_class_t *a_class, uint8 descriptor[], bool attach)
{
	usb_device_t *device;
	uint8 *desc;
	uint8 *cp;
	usb_class_t *cls;
	uint16 length;
	uint16 len;

	bdc_fn(("%s\n", __FUNCTION__));

	device = a_class->device;

	cls = device->usb_class;
	if (NULL != cls) {
		while (NULL != cls->next) {
			cls = cls->next;
		}
	}

	length = descriptor[2] | (descriptor[3] << 8);
	a_class->next = NULL;
	if (cls == NULL) {
		a_class->interface_low = 0;
		device->usb_class = a_class;
		desc = descriptor;
	} else {
		/* Remove configuration descriptor */
		length -= descriptor[0];
		desc = &descriptor[descriptor[0]];
		a_class->interface_low = cls->interface_high + 1;
		cls->next = a_class;
	}

	if ((device->config_descriptor_size + length) > device->max_configuration_size) {
		/* ASSERT(0); */
		return NULL;
	}

	/* Move original descriptor since we could add on to BT descriptor */
	cp = &device->config_descriptor[device->config_descriptor_size];

	/* If this descriptor is in place, just take out configuration descriptor if necessary */
	if (cp == descriptor) {
		memmove(cp, desc, length);
	} else {
		len = device->max_configuration_size - device->config_descriptor_size - length;
		memmove(cp + length, cp, len);
		memmove(cp, desc, length);
	}

	/* If this is the first descriptor, clear out the number of interface fields. */
	if (cp == device->config_descriptor) {
		cp[4] = 0;
	}

	usbdevice_adjust_interface(a_class, cp, length);

	device->config_descriptor_size += length;
	desc = device->config_descriptor;
	desc[2] = (uint8)device->config_descriptor_size;
	desc[3] = (uint8)(device->config_descriptor_size >> 8);

	if (attach && is_composite_device_used) {
		usbdevice_add_to_iad(device);
	}

	if (cp == device->config_descriptor) {
		a_class->interface_desc    = cp + device->config_descriptor[0];
		a_class->interface_desc_len = length - device->config_descriptor[0];
		return a_class->interface_desc;
	}
	a_class->interface_desc    = cp;
	a_class->interface_desc_len = length;
	return a_class->interface_desc;
}

void
usbdevice_set_configure_descriptor(usb_device_t *device, uint8 descriptor[], uint16 max_size)
{
	usb_class_t *a_class;

	bdc_fn(("%s\n", __FUNCTION__));

	device->config_descriptor = descriptor;
	device->max_configuration_size = max_size;
	device->config_descriptor_size = descriptor[2] | (descriptor[3] << 8);

	a_class = device->usb_class;
	a_class->interface_low = 0;
	a_class->interface_desc = &descriptor[descriptor[0]];
	a_class->interface_desc_len = device->config_descriptor_size - descriptor[0];
	device->config_descriptor[4] = 0;
	usbdevice_adjust_interface(a_class, device->config_descriptor,
		device->config_descriptor_size);
}

void
usbdevice_add_string_descriptor(usb_device_t *device, uint8 *string_descriptor, uint8 siz)
{
	/* ASSERT(string_descriptor[0] <= siz); */
	bdc_fn(("%s\n", __FUNCTION__));

	device->string_descriptor[device->max_string_index] = string_descriptor;
	device->string_descriptor_size[device->max_string_index] = string_descriptor[0];
	device->max_string_index++;
}

void
usbdevice_set_class_descriptor(usb_device_t *device, uint8 cfa_port, uint8 *desc)
{
	usb_class_t *a_class;

	bdc_fn(("%s\n", __FUNCTION__));

	a_class = device->usb_class;
	while (NULL != a_class) {
		a_class->ft->set_class_descriptor(a_class, cfa_port, desc);
		a_class = a_class->next;
	}
}

void
usbdevice_switch_to_polling_mode(usb_device_t *device)
{
	usb_class_t *a_class;

	bdc_fn(("%s\n", __FUNCTION__));

	while (TRUE) {
		/* TODO device->controller->ft->polling(device->controller); */

		a_class = device->usb_class;
		while (NULL != a_class) {
			/* TODO a_class->ft->polling(a_class); */
			a_class = a_class->next;
		}

		/*
		 * Kick the watch dog
		 * watchdog_pet_watch_dog();
		 */
	}
}

void
usbdevice_switch_to_normal_mode(usb_device_t *device)
{
}

void
usbdevice_set_class(usb_device_t *device, usb_class_t *cls)
{
	bdc_fn(("%s\n", __FUNCTION__));

	cls->next = NULL;
	device->usb_class = cls;

	device->max_string_index = 0;
	usbdevice_add_string_descriptor(device, usb_language_id_buf, sizeof(usb_language_id_buf));
}

void
usbdevice_create_composite_device(usb_device_t *device, uint8 *dev_desc, uint8 *config_desc_buf,
		uint16 max_size)
{
	bdc_fn(("%s\n", __FUNCTION__));

	device->usb_class = NULL;

	device->device_descriptor = dev_desc;

	device->config_descriptor = config_desc_buf;
	device->max_configuration_size = max_size;
	device->config_descriptor_size = 0;

	device->max_string_index = 0;
	usbdevice_add_string_descriptor(device, usb_language_id_buf, sizeof(usb_language_id_buf));
	usbdevice_add_string_descriptor(device, usb_manufacturer_id_buf,
		sizeof(usb_manufacturer_id_buf));
	usbdevice_add_string_descriptor(device, usb_product_id_buf, sizeof(usb_product_id_buf));
	usbdevice_add_string_descriptor(device, usb_serial_num_buf, sizeof(usb_serial_num_buf));
}

void
usbdevice_init(usb_device_t *device, usb_controller *controller)
{
	bdc_fn(("%s\n", __FUNCTION__));

	device->controller = controller;
	controller->device = device;

	device->ft = (usb_device_function_table *)&usb_device_function_table_rom;
	device->ep0 = (usb_endpoint0 *)controller->ft->ep_create(controller,
		ENDPOINT_USAGE_CONTROL);

	device->setup_urb.ep = (usb_endpoint *)device->ep0;
	device->speed = USBDEVICE_SPEED_UNKNOWN;
}

void
usb_device_create(void *controller, usb_device_t *device)
{
	usbdevice_init(device, (usb_controller *)controller);

	return;
}
