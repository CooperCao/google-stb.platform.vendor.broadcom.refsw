/*
 *  USB Internal configuration
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

#ifndef _USBPLATFORM_H_
#define _USBPLATFORM_H_

#include <typedefs.h>
#include <usbcontroller.h>

/* Defines & Macros */

/* Register definition for cr_pwr_ctl_adr */
#define BT_USB_48M_CLK_ENA	0x1
#define KEYBOARD_48M_CLK_ENA	0x2
#define MOUSE_48M_CLK_ENA	0x4
#define USB3_IP_CLK_ENA		0x10
#define USB11_IP_CLK_ENA	0x20
#define SEL_USB11_IP		0x40


/* Type Definitions */

typedef enum {
	USB_FUNCTION_NONE     = 0x00,
	USB_FUNCTION_BT       = 0x01,
	USB_FUNCTION_KB       = 0x02,
	USB_FUNCTION_MS       = 0x04,
	USB_FUNCTION_HUB      = 0x08,
	USB_FUNCTION_AUDIO    = 0x10,

	USB_FUNCTION_UHE      = 0x07

} USB_FUNCTION_MASK;

typedef struct {
	/* Hidden feature bits */
	uint32 isoc_buffer : 1;
	uint32 isoc_word_align_wa : 1;
	uint32 l1rwe_status_wa : 1;
	uint32 l1rwe_enable_wa : 1;
	uint32 pending_dma_at_bus_reset_wa : 1;
	uint32 default_save_restore_en : 1;
	uint32 bus_reset_hw_reset : 1;
	uint32 unused1 : 25;
} usb_internal_config_t;

extern usb_internal_config_t usb_internal_config;

#endif /* _USBPLATFORM_H_ */
