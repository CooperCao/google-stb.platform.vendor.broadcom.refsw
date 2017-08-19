/*
 * Wl Rdl Class.
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
#include <bcmutils.h>
#include <bcmstdlib.h>
#include <usbrdl.h>
#include <trxhdr.h>
#include <rte_mem.h>
#include <bdc_usb.h>
#include <usbconst.h>
#include <wlrdl.h>
#include <usbrdl.h>
#include <bdc_rte.h>
#include <usbcontroller.h>
#include <usbclass.h>


/*******************************************************************************
* Function Prototypes
******************************************************************************
*/
bdc_status_code wlrdl_configured(usb_class_t *cls, int config);
bdc_status_code wlrdl_get_ep_status(wl_rdl *wlrdl, uint8 *setup);
bdc_status_code wlrdl_feature(wl_rdl *wlrdl, uint8 *setup, int is_set);
bdc_status_code wlrdl_setup(usb_class_t *cls);
void wlrdl_event_sent(urb_t *urb);
void wlrdl_data_sent(urb_t *urb);
void wlrdl_set_class(wl_rdl *wlrdl, usb_device_t *device);
void wlrdl_bus_reset(usb_class_t *cls);
void wlrdl_data_received(urb_t *urb);


/*******************************************************************************
* Local Declarations
******************************************************************************
*/

const usb_class_function_table wl_rdl_funcs =
{
	usbclass_start,	/* start */
	usbclass_stop,	/* stop */
	wlrdl_bus_reset,	/* bus_reset */
	usbclass_suspended,	/* suspended */
	usbclass_resumed,	/* resumed */
	usbclass_vendor_command,	/* vendor_command */
	wlrdl_setup,	/* setup */
	wlrdl_configured,	/* configured */
	usbclass_alt_setting,	/* alt_setting */
	usbclass_polling,	/* polling */
	usbclass_send_next_transfer,	/* send_next_transfer */
	usbclass_memory_available,	/* memory_available */
	usbclass_set_class_descriptor,	/* set_class_descriptor */
};

/*****************************************************************************
* Function Definitions
******************************************************************************
*/

bdc_status_code
wlrdl_configured(usb_class_t *cls, int config)
{
	usb_controller *controller;

	bdc_trace(("%s \n", __FUNCTION__));

	controller = cls->device->controller;

	switch (config) {
	case 0:
		if (NULL != ((wl_rdl *)cls)->ep_in1) {
			controller->ft->ep_destroy(controller,
				((wl_rdl *)cls)->ep_in1);
			((wl_rdl *)cls)->ep_in1 = NULL;
		}
		if (NULL != ((wl_rdl *)cls)->ep_in2) {
			controller->ft->ep_destroy(controller,
				((wl_rdl *)cls)->ep_in2);
			((wl_rdl *)cls)->ep_in2 = NULL;
		}
		if (NULL != ((wl_rdl *)cls)->ep_out2) {
			controller->ft->ep_destroy(controller,
				((wl_rdl *)cls)->ep_out2);
			((wl_rdl *)cls)->ep_out2 = NULL;
		}
		break;

	case 1:
		((wl_rdl *)cls)->ep_in1 = controller->ft->ep_create(controller,
			ENDPOINT_USAGE_INTERRUPT_IN);
		((wl_rdl *)cls)->ep_in2 = controller->ft->ep_create(controller,
			ENDPOINT_USAGE_BULK_IN);
		((wl_rdl *)cls)->ep_out2 = controller->ft->ep_create(controller,
			ENDPOINT_USAGE_BULK_OUT);

		((wl_rdl *)cls)->ep_in1Urb.ep = ((wl_rdl *)cls)->ep_in1;
		((wl_rdl *)cls)->ep_in2Urb.ep = ((wl_rdl *)cls)->ep_in2;
		((wl_rdl *)cls)->ep_out2Urb.ep = ((wl_rdl *)cls)->ep_out2;
		break;

	default:
		return BDC_STATUS_INVALID_FIELD;
	}

	return BDC_STATUS_SUCCESS;
}


void
wlrdl_bus_reset(usb_class_t *cls)
{
	bdc_trace(("%s \n", __FUNCTION__));

	usbclass_bus_reset(cls);
}

bdc_status_code
wlrdl_get_ep_status(wl_rdl *wlrdl, uint8 *setup)
{
	uint16 w_index;

	bdc_trace(("%s \n", __FUNCTION__));

	w_index = USB_SETUP_INDEX(setup);
	if (wlrdl->ep_in1 && wlrdl->ep_in1->address == w_index) {
		return usbclass_get_ep_status((usb_class_t *)wlrdl, wlrdl->ep_in1, setup);
	} else if (wlrdl->ep_in2 && wlrdl->ep_in2->address == w_index)	  {
		return usbclass_get_ep_status((usb_class_t *)wlrdl, wlrdl->ep_in2, setup);
	} else if (wlrdl->ep_out2 && wlrdl->ep_out2->address == w_index)   {
		return usbclass_get_ep_status((usb_class_t *)wlrdl, wlrdl->ep_out2, setup);
	}

	return BDC_STATUS_UNKNOWN;
}

bdc_status_code
wlrdl_feature(wl_rdl *wlrdl, uint8 *setup, int is_set)
{
	uint16 w_index;

	bdc_trace(("%s \n", __FUNCTION__));

	w_index = USB_SETUP_INDEX(setup);
	if (wlrdl->ep_in1 && wlrdl->ep_in1->address == w_index) {
		return usbclass_feature((usb_class_t *)wlrdl, wlrdl->ep_in1, setup, is_set);
	} else if (wlrdl->ep_in2 && wlrdl->ep_in2->address == w_index)	  {
		return usbclass_feature((usb_class_t *)wlrdl, wlrdl->ep_in2, setup, is_set);
	} else if (wlrdl->ep_out2 && wlrdl->ep_out2->address == w_index)   {
		return usbclass_feature((usb_class_t *)wlrdl, wlrdl->ep_out2, setup, is_set);
	}

	return BDC_STATUS_UNKNOWN;
}

bdc_status_code
wlrdl_setup(usb_class_t *cls)
{
	uint8 *setup;

	bdc_trace(("%s \n", __FUNCTION__));

	setup = cls->device->ep0->setup;

	if ((USB_SETUP_BM_REQUEST_TYPE(setup) == 0x82) &&
		(USB_STD_GET_STATUS_REQ_VALUE == USB_SETUP_B_REQUEST(setup)))	{
		return wlrdl_get_ep_status((wl_rdl *)cls, setup);
	} else if ((USB_SETUP_BM_REQUEST_TYPE(setup) == 0x02) &&
		(USB_STD_CLEAR_FEATURE_REQ_VALUE == USB_SETUP_B_REQUEST(setup))) {
		return wlrdl_feature((wl_rdl *)cls, setup, 0);
	} else if ((USB_SETUP_BM_REQUEST_TYPE(setup) == 0x02) &&
		(USB_STD_SET_FEATURE_REQ_VALUE == USB_SETUP_B_REQUEST(setup)))	{
		return wlrdl_feature((wl_rdl *)cls, setup, 1);
	} else {
		return usbclass_setup(cls);
	}
}

void
wlrdl_event_sent(urb_t *urb)
{
	bdc_trace(("%s \n", __FUNCTION__));

	/* Place holder */
	return;
}

void
wlrdl_data_sent(urb_t *urb)
{
	bdc_trace(("%s \n", __FUNCTION__));

	/* Place holder */
	return;
}

void
wlrdl_data_received(urb_t *urb)
{
	bdc_trace(("%s \n", __FUNCTION__));

	/* Place holder */

	return;
}


void
wlrdl_set_class(wl_rdl *wlrdl, usb_device_t *device)
{
	bdc_trace(("%s \n", __FUNCTION__));

	usbdevice_set_class(device, (usb_class_t *)wlrdl);
	device->device_descriptor = usb_rdl_dev_des_buf;

	usbdevice_set_configure_descriptor(device, usb_rdl_cfg_des_buf,
		sizeof(usb_rdl_cfg_des_buf));
	usbdevice_add_string_descriptor(device, usb_manufacturer_id_buf,
		sizeof(usb_manufacturer_id_buf));
	usbdevice_add_string_descriptor(device, usb_product_id_buf, sizeof(usb_product_id_buf));
	usbdevice_add_string_descriptor(device, usb_serial_num_buf, sizeof(usb_serial_num_buf));
}

void
wlrdl_init(wl_rdl *wlrdl, usb_device_t *device)
{
	bdc_trace(("%s \n", __FUNCTION__));

	usbclass_init((usb_class_t *)wlrdl, device);

	((usb_class_t *)wlrdl)->ft = (usb_class_function_table *)&wl_rdl_funcs;

	wlrdl->ep_in1Urb.status = BDC_STATUS_NOT_USED;
	wlrdl->ep_in1Urb.complete_callback = wlrdl_event_sent;

	wlrdl->ep_in2Urb.status = BDC_STATUS_NOT_USED;
	wlrdl->ep_in2Urb.complete_callback = wlrdl_data_sent;

	wlrdl->ep_out2Urb.status = BDC_STATUS_NOT_USED;
	wlrdl->ep_out2Urb.complete_callback = wlrdl_data_received;

}

void *
usb_wl_create(void *device)
{
	wl_rdl *wlrdl;

	bdc_trace(("%s \n", __FUNCTION__));

	if (!(wlrdl = (wl_rdl *)MALLOC_ALIGN(((usb_device_t *)device)->controller->osh,
			sizeof(wl_rdl), UBDC_ALIGN_BITS))) {
		bdc_err(("%s: out of memory", __FUNCTION__));
		ASSERT(0);
		return NULL;
	}

	wlrdl_init(wlrdl, (usb_device_t *)device);
	wlrdl_set_class(wlrdl, (usb_device_t *)device);

	return wlrdl;
}
