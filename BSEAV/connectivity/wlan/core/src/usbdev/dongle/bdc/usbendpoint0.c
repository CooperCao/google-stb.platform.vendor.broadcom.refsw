/*
 *  USB endpoint0 implementation.
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

#include <usbendpoint0.h>
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

void
usbendpoint0_init(usb_endpoint0 *ep, usb_controller *controller)
{
	usbendpoint_init((usb_endpoint *)ep, controller);

	((usb_endpoint *)ep)->address = 0;
	((usb_endpoint *)ep)->type = ENDPOINT_TYPE_CTRL;
	((usb_endpoint *)ep)->max_packet_size = 3;	/* 8 bytes */
}
