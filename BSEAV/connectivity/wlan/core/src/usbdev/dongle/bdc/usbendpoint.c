/*
 *  USB endpoint implementation.
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
#include <usbtypes.h>
#include <usbendpoint.h>
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

bdc_status_code usbendpoint_transfer_start(urb_t *urb);
void usbendpoint_transfer_cancel(urb_t *urb);
void usbendpoint_clear_or_set_stall(usb_endpoint *ep, int is_set);
void usbendpoint_set_alt_setting(usb_endpoint *ep, int altsetting, int packet_size);

bdc_status_code
usbendpoint_transfer_start(urb_t *urb)
{
	/* ASSERT(0); */
	return BDC_STATUS_IMPLEMENTED_BY_SUBCLASS;
}

void
usbendpoint_transfer_cancel(urb_t *urb)
{
	/* ASSERT(0); */
}

void
usbendpoint_clear_or_set_stall(usb_endpoint *ep, int is_set)
{
}

void
usbendpoint_set_alt_setting(usb_endpoint *ep, int altsetting, int packet_size)
{
}

void
usbendpoint_init(usb_endpoint *ep, usb_controller *controller)
{
	ep->device = controller->device;
	ep->transfer_start = usbendpoint_transfer_start;
	ep->transfer_cancel = usbendpoint_transfer_cancel;
	ep->clear_or_set_stall = usbendpoint_clear_or_set_stall;
	ep->set_alt_setting = usbendpoint_set_alt_setting;
}
