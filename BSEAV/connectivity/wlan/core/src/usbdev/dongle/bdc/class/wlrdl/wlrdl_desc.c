/*
 * wlan descriptors.
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
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmstdlib.h>
#include <bdc_usb.h>
#include <usbconst.h>
#include <bcmdevs.h>

#ifdef USB_BYPASS_REENUM
/*
 * ===============================================================================================
 * ! ALL usb globals are placed in a separate region.  If no re-enumeration is required on
 * reboot,
 * ! all USB variables will retain their value.	This header must be applied to all USB files
 * ===============================================================================================
 */
#pragma arm section rwdata = "usb_area_rw" zidata = "usb_area_zi"
#endif

uint8 usb_rdl_dev_des_buf[USB_DEVDES_SIZE] =
{
	USB_DEVDES_SIZE,			/* bLength */
	USB_DEVICE_DESC_TYPE,			/* bDescriptorType */
	((USB_UD_2_0) & 0xFF),			/* bcdUSB_LSB */
	((USB_UD_2_0>> 8 ) & 0xFF),		/* bcdUSB_MSB USB 2.0 Spec */
	USB_VDR_CLASS_TYPE,			/* bDeviceClass	 Vendor */
	0x00,					/* bDeviceSubClass */
	0x00,					/* bDeviceProtocol */
	USB_MAX_CTRL_PACKET,			/* bMaxPacketSize */
	((BCM_DNGL_VID) & 0xFF),		/* idVendor_LSB 0x0a5c = Broadcom */
	((BCM_DNGL_VID>>8) & 0xFF),		/* idVendor_MSB */
	((BCM_DNGL_BL_PID_4373)&0xFF),		/* idProduct_LSB */
	((BCM_DNGL_BL_PID_4373>>8)&0xFF),	/* idProduct_MSB */
	0x01,					/* bcdDevice_LSB */
	0x00,					/* bcdDevice_MSB  Device Release Number */
	USB_MANUFACTURER_IDX,			/* iManufacturer */
	USB_PRODUCT_IDX,			/* iProduct */
	USB_SERIAL_NUM_IDX,			/* iSerialNumber */
	0x01,					/* bNumConfigurations */
};

uint8 usb_rdl_cfg_des_buf[USB_DFLT_RDL_CFGDES_SIZE] =
{
/*
 *
 * Configuation Descriptor - 9 bytes,
 *
 */
	USB_CFGDES_SIZE,		/* bLength */
	USB_CONFIG_DESC_TYPE,		/* bDescriptorType */
	USB_DFLT_RDL_CFGDES_SIZE_LSB,	/* wTotalLength (L) */
	USB_DFLT_RDL_CFGDES_SIZE_MSB,	/* wTotalLength (H) */
	0x01,				/* bNumberInterfaces */
	0x01,				/* bConfigurationValue */
	0x00,				/* iConfiguration - index of string descriptor */
	USB_DEV_ATTRIBUTES,		/* bmAttributes, default: bus power, remote wakeup */
	USB_DEV_MAXPOWER, /* MaxPower...Max power used by device from bus in this configuration */
	/*
	 *
	 * Interface 0, Alternate Setting 0 ,
	 * Descriptor - 9 bytes
	 *
	 */
	USB_INTFDES_SIZE,		/* bLength */
	USB_INTERFACE_DESC_TYPE,	/* bDescriptorType */
	0x00,				/* bInterfaceNumber */
	0x00,				/* bAlternateSetting */
	0x03,				/* bNumberEndpoints excluding endpt 0. */
	USB_VDR_CLASS_TYPE,		/* bInterfaceClass */
	USB_SUB_CLASS_ABSTRACT_CONTROL_MODEL,	/* bInterfaceSubClass */
	USB_VDR_CLASS_PROTOCOL_TYPE,	/* bInterfaceProtocol */
	0x00,				/* iInterface - index of string descriptor */
	/*
	 *
	 * Interface 0, Alternate Setting 0,
	 * Interrupt Endpoint Descriptor - 7 bytes
	 *
	 */
	USB_ENDPDES_SIZE,		/* bLength */
	USB_ENDPOINT_DESC_TYPE,		/* bDescriptorType */
	0x81,				/* bEndpointAddress - IN - 1 */
	USB_ENDP_TYPE_INTR,		/* bmAttributes  */
	((USB_MAX_INTR_PACKET) & 0xFF),	/* wMaxPacketSize (L) */
	((USB_MAX_INTR_PACKET >> 8) & 0xFF),	/* wMaxPacketSize (H) */
	0x04,				/* bInterval - Polling interval (ms) */
	/*
	 *
	 * Interface 0, Alternate Setting 0,
	 * BulkIN Endpoint Descriptor - 7 bytes
	 *
	 */
	USB_ENDPDES_SIZE,		/* bLength */
	USB_ENDPOINT_DESC_TYPE,		/* bDescriptorType */
	0x82,				/* bEndpointAddress - IN - 2 */
	USB_ENDP_TYPE_BULK,		/* bmAttributes - Bulk,No sync, Data endpt */
	((USB_MAX_BULK_PACKET) & 0xFF),	/* wMaxPacketSize (L) */
	((USB_MAX_BULK_PACKET >> 8) & 0xFF),	/* wMaxPacketSize (H) */
	0x00,				/* bInterval - Polling interval (ms) */
	/*
	 *
	 * Interface 0, Alternate Setting 0,
	 * BulkOUT Endpoint Descriptor - 7 bytes
	 *
	 */
	USB_ENDPDES_SIZE,		/* bLength */
	USB_ENDPOINT_DESC_TYPE,		/* bDescriptorType */
	0x02,				/* bEndpointAddress - OUT - 2 */
	USB_ENDP_TYPE_BULK,		/* bmAttributes - Bulk,No sync, Data endpt */
	((USB_MAX_BULK_PACKET) & 0xFF),	/* wMaxPacketSize (L) */
	((USB_MAX_BULK_PACKET >> 8) & 0xFF),	/* wMaxPacketSize (H) */
	0x00,				/* bInterval - Polling interval (ms) */
};
