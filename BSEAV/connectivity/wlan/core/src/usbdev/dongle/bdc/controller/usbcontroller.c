/*
 * USB device controller implementation.
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
#include <urb.h>
#include <usbdevice.h>
#include <usbclass.h>
#include <usbcontroller.h>

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

void usbcontroller_remote_wake(usb_controller *controller);
void usbcontroller_interrupt_handler(usb_controller *controller);
bool usbcontroller_is_remote_wake_enabled(usb_controller *controller);
void usbcontroller_add_to_pending_list(usb_controller *controller, urb_t *urb);

#if defined(MPAF_ENABLE)
extern usb_controller *g_hub_controller;
#endif

usb_endpoint *
usbcontroller_ep_create(usb_controller *controller, endpoint_usage usage)
{
	return NULL;
}

void
usbcontroller_ep_destroy(usb_controller *controller, usb_endpoint *ep)
{
	controller->ft->ep_cancel_all_transfer(controller, ep);
}

void
usbcontroller_ep_cancel_all_transfer(usb_controller *controller, usb_endpoint *ep)
{
	while (NULL != ep->urb_list) {
		controller->ft->transfer_complete(controller, ep, BDC_STATUS_BUS_RESET);
	}
}

void
usbcontroller_start(usb_controller *controller)
{
}

void
usbcontroller_stop(usb_controller *controller)
{
}

void
usbcontroller_reset(usb_controller *controller)
{
}

void
usbcontroller_bus_reset(usb_controller *controller)
{
	usb_device_t *device = controller->device;

	controller->ft->reset(controller);
	/* controller->sof_call_back_list = NULL; */
	device->ft->bus_reset(device);
}

void
usbcontroller_remote_wake(usb_controller *controller)
{
}

bool
usbcontroller_update_resume(usb_controller *controller)
{
	if (controller->device->usbd_current_status & USBDEVICE_STATUS_RESUMING) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void
usbcontroller_connect_to_host(usb_controller *controller)
{
}

void
usbcontroller_disconnect_from_host(usb_controller *controller)
{
}

void
usbcontroller_setup_complete(usb_controller *controller, bdc_status_code status)
{
}

void
usbcontroller_interrupt_handler(usb_controller *controller)
{
}

void
usbcontroller_enable(usb_controller *controller)
{
}

void
usbcontroller_disable(usb_controller *controller)
{
}

void
usbcontroller_polling(usb_controller *controller)
{
}

bool
usbcontroller_is_remote_wake_enabled(usb_controller *controller)
{
	return FALSE;
}

void
usbcontroller_set_wakeup_source(usb_controller *controller)
{
}
void
usbcontroller_send_raw_event(usb_controller *controller, uint8 *data, uint32 length)
{
}
void
usbcontroller_suspended(usb_controller *controller)
{
}

void
usbcontroller_transfer_complete(usb_controller *controller, usb_endpoint *ep,
	bdc_status_code status)
{
	urb_t *urb;
	urb_t *next_urb;

	if (ep->urb_list == NULL) {
		return;
	}

	urb = ep->urb_list;
	next_urb = urb->next;
	ep->urb_list = next_urb;

	urb->status = status;
	if (urb->complete_callback) {
		urb->complete_callback(urb);
	}

	if (NULL != next_urb) {
		ep->transfer_start(next_urb);
	}
}

void
usbcontroller_data_sent(usb_controller *controller, usb_endpoint *ep)
{
	urb_t *urb;
	int length;

	urb = ep->urb_list;

	if (urb->attribute & URB_ATTRIBUTE_UNLIMITED) {
		ep->transfer_start(urb);
		return;
	}

	urb->finished_length += ep->pending_size;

	length = urb->expected_length - urb->finished_length;
	if (length) {
		ep->transfer_start(urb);
		return;
	}

	controller->ft->transfer_complete(controller, ep, BDC_STATUS_SUCCESS);
}

void
usbcontroller_data_received(usb_controller *controller, usb_endpoint *ep)
{
	urb_t *urb;
	int length;

	urb = ep->urb_list;
	if (urb == NULL) {
		return;
	}

	ep->transfer_start(urb);

	if (urb->attribute & URB_ATTRIBUTE_UNLIMITED) {
		return;
	}

	length = urb->expected_length - urb->finished_length;
	if (length == 0 || (urb->finished_length % (1 << ep->max_packet_size))) {
		controller->ft->transfer_complete(controller, ep, BDC_STATUS_SUCCESS);
	}
}


void
usbcontroller_add_to_pending_list(usb_controller *controller, urb_t *urb)
{
	urb_t *urb_list;

	if (controller->pending_urbs_list == NULL) {
		controller->pending_urbs_list = urb;
	} else {
		urb_list = controller->pending_urbs_list;
		while (NULL != urb_list->next) {
			urb_list = urb_list->next;
		}

		urb_list->next = urb;
	}
}

void
usbcontroller_resume_completed(usb_controller *controller)
{
	urb_t *urb;

	controller->device->usbd_current_status &= ~(USBDEVICE_STATUS_RESUMING);
	controller->device->usbd_current_status &= ~(USBDEVICE_STATUS_SUSPENDED);

	/* usb_send_message_to_thread(USB_MSG_WORK);	//TODO */

	/* If this controller has pending urbs, send them */
	while (controller->pending_urbs_list != NULL) {
		urb = controller->pending_urbs_list;
		controller->pending_urbs_list = controller->pending_urbs_list->next;
		controller->ft->transfer_start(controller, urb);
	}

	/* Call resumed for USB Class */
	controller->device->usb_class->ft->resumed(controller->device->usb_class);

}

void
usbcontroller_transfer_start(usb_controller *controller, urb_t *urb)
{
	usb_endpoint *ep;
	urb_t *urb_list;

	bdc_fn(("%s\n", __FUNCTION__));

	urb->next = NULL;
	urb->status = BDC_STATUS_PENDING;

	if (controller->device->usbd_current_status & USBDEVICE_STATUS_SUSPENDED) {
		usbcontroller_add_to_pending_list(controller, urb);

		if (controller->ft->is_remote_wake_enabled(controller)) {
			controller->ft->remote_wake(controller);
		}
	} else {
		/* !USBDEVICE_STATUS_SUSPENDED */
		ep = urb->ep;
		urb->finished_length = 0;
		ep->pending_size = 0;

		if (ep->urb_list == NULL) {
			ep->urb_list = urb;

			ep->transfer_start(urb);

			if (urb->attribute & URB_ATTRIBUTE_UNLIMITED) {
				return;
			}

			if ((ep->address & USB_DIR_DEVICE_TO_HOST) == 0) {
				if (!(urb->expected_length - urb->finished_length)) {
					controller->ft->transfer_complete(controller, ep,
							BDC_STATUS_SUCCESS);
				}
			}
		} else {
			urb_list = ep->urb_list;
			while (NULL != urb_list->next) {
				urb_list = urb_list->next;
			}

			urb_list->next = urb;
		}
	}
}

int
usbcontroller_get_port(usb_controller *controller)
{
	return USB_PORT_1;
}

bdc_status_code
usbcontroller_feature(usb_controller *controller, uint16 feature, int is_set)
{
	if (USBDEVICE_FEATURE_REMOTE_WAKEUP == feature) {
		if (is_set) {
			controller->attribute |= USBDEVICE_FEATURE_REMOTE_WAKEUP;
		} else {
			controller->attribute &= ~USBDEVICE_FEATURE_REMOTE_WAKEUP;
		}
	}

	return BDC_STATUS_SUCCESS;
}

bool
usbcontroller_is_enumerated(usb_controller *controller)
{
	return controller->device->usbd_current_status & USBDEVICE_STATUS_ENUMERATED;
}

void
usbcontroller_init(usb_controller *controller)
{
	controller->pending_urbs_list = NULL;
}
