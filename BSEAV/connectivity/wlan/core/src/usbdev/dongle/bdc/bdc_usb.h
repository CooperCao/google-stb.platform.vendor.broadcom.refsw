/*
 *  Misc BDC core header functions
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

#ifndef _USB_H_
#define _USB_H_

#include <typedefs.h>
#include <bcmdefs.h>
#include <usbconst.h>

/* Number of ports configuration we track. */
#define USB_NUM_PORTS_CFG 3

enum {
	USB_PORT_BT     = 0,
	USB_PORT_KB     = 1,
	USB_PORT_MS     = 2,
	USB_PORT_RSVD   = 3,
	USB_PORT_HUB    = 4,
	};

typedef uint8 usb_port;

extern uint8 usb_serial_num_buf[USB_WLAN_SERIAL_NUMBER_SIZE];
extern uint8 usb_rdl_cfg_des_buf[USB_DFLT_RDL_CFGDES_SIZE];

extern uint8 usb_language_id_buf[USB_ONE_LANGUAGE_ID_SIZE];
extern uint8 usb_manufacturer_id_buf[USB_MANUF_ID_BUF_SIZE];
extern uint8 usb_product_id_buf[USB_PRODUCT_ID_BUF_SIZE];

typedef void (*usb_enum_done_cb)(usb_port port_id);

extern usb_enum_done_cb usb_ptr_to_enum_done_callback;

#endif	/* _USB_H_ */
