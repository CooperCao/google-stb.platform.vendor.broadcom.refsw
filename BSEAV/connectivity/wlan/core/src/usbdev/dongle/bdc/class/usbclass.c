/*
 *  usb_class_t is the base class for all USB controller
 *              driver.
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
#include <bcmdevs.h>
#include <bcmutils.h>
#include <bcmstdlib.h>
#include <bdc_usb.h>
#include <usbconst.h>
#include <usbclass.h>
#include <usbdevice.h>

#ifdef USB_BYPASS_REENUM
/*
 * ===============================================================================================
 * ! ALL usb globals are placed in a separate region.  If no re-enumeration is required on
 * reboot,
 * ! all USB variables will retain their value.  This header must be applied to all USB files
 * ===============================================================================================
 */
#pragma arm section rwdata = "usb_area_rw" zidata = "usb_area_zi"
#endif

/*******************************************************************************
* Local Declarations
******************************************************************************
*/
static uint8 USB_vdr_test_mem[USB_NUM_PORTS_CFG][2 * 3];	/* 3 entries per port */
bool USB_vdr_hid_local_loopback;

/*******************************************************************************
* Function Definitions
******************************************************************************
*/
void usb_register_wait_memory(void);

void
usbclass_start(usb_class_t *cls)
{
	usb_device_t *device = cls->device;

	if (cls != device->usb_class) {
		return;
	}

	device->ft->start(device);
}

void
usbclass_stop(usb_class_t *cls)
{
	usb_device_t *device = cls->device;

	device->ft->stop(device);
}

void
usbclass_bus_reset(usb_class_t *cls)
{
	cls->ft->configured(cls, 0);

	if (cls != cls->device->usb_class) {
		return;
	}

	cls->usbc_current_status = 0;

	/* TODO
	 * if (USB_p_port_reset_callback)
	 * {
	 * USB_p_port_reset_callback(cls->mpaf_port);
	 * }
	 */
}

void
usbclass_suspended(usb_class_t *cls)
{
	if (cls != cls->device->usb_class) {
		return;
	}

	/* TODO
	 * if (USB_p_suspend_callback)
	 * {
	 * USB_p_suspend_callback(cls->mpaf_port);
	 * }
	 */
}

void
usbclass_resumed(usb_class_t *cls)
{
	if (cls != cls->device->usb_class) {
		return;
	}

	/* TODO
	 * if (USB_p_resumed_callback)
	 * {
	 * USB_p_resumed_callback(cls->mpaf_port);
	 * }
	 */
}

bdc_status_code
usbclass_vendor_command(usb_class_t *cls)
{
	uint8 *setup;
	urb_t *urb;
	usb_device_t *device;
	int index;
	int port;

	device = cls->device;
	setup = device->ep0->setup;
	urb = &device->setup_urb;
	port = device->controller->ft->get_port(device->controller);

	switch (USB_SETUP_B_REQUEST(setup)) {
	case USB_VDR_SET_TEST_MEM_VALUE:
	{
		index = USB_SETUP_INDEX_LSB(setup) * 2;

		USB_vdr_test_mem[port][index + 0] = USB_SETUP_VALUE_LSB(setup);
		USB_vdr_test_mem[port][index + 1] = USB_SETUP_VALUE_MSB(setup);
		return BDC_STATUS_SUCCESS;
	}

	case USB_VDR_GET_TEST_MEM_VALUE:
	{
		index = USB_SETUP_INDEX_LSB(setup) * 2;
		urb->data_buffer = &USB_vdr_test_mem[port][index];
		urb->expected_length = 2;
		return BDC_STATUS_DATA_READY;
	}

	case USB_VDR_SET_HID_LOCAL_LOOPBACK:
		USB_vdr_hid_local_loopback = USB_SETUP_VALUE_LSB(setup);
		return BDC_STATUS_SUCCESS;

	default:
		break;
	}

	return BDC_STATUS_UNKNOWN;
}

bdc_status_code
usbclass_setup(usb_class_t *cls)
{
	if (USB_SETUP_BM_REQUEST_TYPE(cls->device->ep0->setup) & USB_BMREQ_VDR) {
		return cls->ft->vendor_command(cls);
	}

	return BDC_STATUS_UNKNOWN;
}

bdc_status_code
usbclass_configured(usb_class_t *cls, int config)
{
	return BDC_STATUS_SUCCESS;
}

void
usbclass_polling(usb_class_t *cls)
{
}

bdc_status_code
usbclass_alt_setting(usb_class_t *cls, int interface, int altsetting)
{
	return BDC_STATUS_SUCCESS;
}

void
usbclass_send_next_transfer(usb_class_t *cls)
{
}

void
usbclass_memory_available(usb_class_t *cls)
{
}

void
usbclass_set_wait_for_memory(usb_class_t *cls)
{
	cls->usbc_current_status |= USBCLASS_STATUS_WAIT_MEMORY;
	/* USB_out_ep_waiting_mask |= PORT1_EP2_WAIT_MASK;	//TODO */
}

void
usbclass_clear_wait_for_memory(usb_class_t *cls)
{
	cls->usbc_current_status &= ~USBCLASS_STATUS_WAIT_MEMORY;
}

bdc_status_code
usbclass_get_ep_status(usb_class_t *cls, usb_endpoint *ep, uint8 *setup)
{
	usb_device_t *device;
	urb_t *urb;

	device = cls->device;
	urb = &device->setup_urb;

	ASSERT(USBDEVICE_MAX_GENERAL_DATA >= 2);
	device->general_data[0] = ep->ep_status;
	device->general_data[1] = 0;

	urb->data_buffer = device->general_data;
	urb->expected_length = 2;
	if (urb->expected_length > setup[6]) {
		urb->expected_length = setup[6];
	}

	return BDC_STATUS_DATA_READY;
}

bdc_status_code
usbclass_feature(usb_class_t *cls, usb_endpoint *ep, uint8 *setup, int is_set)
{
	/* Only can do end point halt */
	if (0 != USB_SETUP_VALUE(setup)) {
		return BDC_STATUS_UNKNOWN;
	}

	if (is_set) {
		ep->ep_status |= ENDPOINT_STATUS_HALT;
	} else {
		ep->ep_status &= ~ENDPOINT_STATUS_HALT;
	}

	ep->clear_or_set_stall(ep, is_set);

	return BDC_STATUS_SUCCESS;
}

void
usbclass_set_class_descriptor(usb_class_t *cls, uint8 cfa_port, uint8 *desc)
{

}

/* The parameter new_desc must be a full configuration descriptor. */
void
usbclass_replace_descriptor(usb_class_t *cls, uint8 *new_desc)
{
	uint8 *class_descriptor;
	uint8 *cp;
	uint16 length;
	uint16 new_itf_desc_len;
	uint8 interface_index;
	int len;

	class_descriptor = cls->interface_desc;

	/* Remove the config descriptor. */
	new_itf_desc_len = (new_desc[2] | (new_desc[3] << 8)) - new_desc[0];
	new_desc = &new_desc[new_desc[0]];

	interface_index = cls->interface_low;
	length = new_itf_desc_len;
	cp = new_desc;
	while (length) {
		if (USB_INTERFACE_DESC_TYPE == cp[1]) {
			cp[2] = interface_index++;
		}

		length -= cp[0];
		cp += cp[0];
	}

	len = new_itf_desc_len - cls->interface_desc_len;
	if (0 != len) {
		usb_class_t *a_class;

		length = cls->device->config_descriptor_size;
		length -= (class_descriptor - cls->device->config_descriptor);
		length -= cls->interface_desc_len;

		cp = class_descriptor + cls->interface_desc_len + len;
		memmove(cp, class_descriptor + cls->interface_desc_len, length);
		cls->interface_desc_len = new_itf_desc_len;
		cp = cls->device->config_descriptor;
		length = cls->device->config_descriptor_size;
		length += len;
		cls->device->config_descriptor_size = length;
		cp[2] = (uint8)length;
		cp[3] = (uint8)(length >> 8);

		/* Move the interface desc position for trailing classes. */
		a_class = cls->next;
		while (NULL != a_class) {
			a_class->interface_desc += len;
			a_class = a_class->next;
		}
	}

	memcpy(class_descriptor, new_desc, cls->interface_desc_len);
}

void
usbclass_init(usb_class_t *cls, usb_device_t *device)
{
	cls->device = device;
}
