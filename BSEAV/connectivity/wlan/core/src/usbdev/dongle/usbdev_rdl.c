/*
 * RDownload USB device
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#define __need_wchar_t
#include <stddef.h>
#ifdef ZLIB
#include <zlib.h>
#include <zutil.h>
#endif
#include <typedefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <osl.h>
#include <trxhdr.h>
#include <usb.h>
#include <usbrdl.h>
#include <siutils.h>
#include <usbdev.h>
#include <usbdev_dbg.h>
#include <bcmdevs.h>

#include <rte_dev.h>
#include <rte_mem.h>
#include <rte_cons.h>
#include <usbdev_common.h>

#define MEM_REMAP_MASK 0xff000000

/* OS specifics */
#error "Need to define INV_ICACHE() & FLUSH_DCACHE()"

#define Z_DEFLATED   8		/* Deflated */
#define ASCII_FLAG   0x01	/* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02	/* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04	/* bit 2 set: extra field present */
#define ORIG_NAME    0x08	/* bit 3 set: original file name present */
#define COMMENT      0x10	/* bit 4 set: file comment present */
#define RESERVED     0xE0	/* bits 5..7: reserved */
#define GZIP_TRL_LEN 8		/* 8 bytes, |crc|len| */

extern void rdl_indicate_start(uint32 dlstart);
extern hnd_dev_t bcmrdl;
extern bool validate_sdr_image(struct trx_header *hdr);

#define EP_BULK_MAX	4
#ifdef BCMUSBDEV_COMPOSITE
#define EP_INTR_MAX	2
#define EP_ISO_MAX	2
#define INTERFACE_BT	0	/* default BT interface 0 */
#define INTERFACE_ISO	1	/* BT/ISO interface 1 */
#define INTERFACE_DFU	2	/* BT/DFU interface 2 */
#define INTERFACE_MAX	9	/* composite device: 1 interface for WLAN, rest for BT */
#define ALT_SETTING_MAX	5	/* alternate settings for BT ISO interface */
#define IAD_INT_NUM	0	/* IAD for grouping BT interface 0 onwards */
#endif /* BCMUSBDEV_COMPOSITE */

#define BT_BULK_OUT_NUM(usbdesc) (usbdesc & 0x8000)
#define BT_BULK_IN_NUM(usbdesc) (usbdesc & 0x8000)

#ifdef BCMUSBDEV_COMPOSITE
static void usbdev_set_ep_defaults(struct dngl_bus *bus, int int_num);
static void usbdev_set_iso_alt_ep_maxsize(usb_endpoint_descriptor_t* d, int alt_int);
#endif /* BCMUSBDEV_COMPOSITE */
#ifdef USB_IFTEST
static int usbd_resume(struct dngl_bus *bus);
#endif

typedef void *rdl_t;


/*
 * Remote downloader personality: upto 4 interfaces:
 *   1 WLAN, upto 3 BT interfaces
 */

#ifdef USB_XDCI
void usbdev_attach_xdci(struct dngl_bus *bus)
{
}
void usbdev_detach_xdci(struct dngl_bus *bus)
{
}
#endif /* USB_XDCI */

static int check_headers(struct dngl_bus *dev, unsigned char *headers);
#ifdef ZLIB
static void init_zlib_state(struct dngl_bus *dev);
#endif

#ifdef ZLIB
static void
init_zlib_state(struct dngl_bus *dev)
{
	if (dev->zlibinit) {
		inflateEnd(&(dev->d_stream));
		dev->zlibinit = FALSE;
	}

	dev->zstatus = 0;
	dev->trl_idx = 0;

	/* Initialise the decompression struct */
	dev->d_stream.next_in = NULL;
	dev->d_stream.avail_in = 0;
	dev->d_stream.next_out = (uchar*)0x80001000;
	dev->d_stream.avail_out = 0x4000000;
	dev->d_stream.zalloc = (alloc_func)0;
	dev->d_stream.zfree = (free_func)0;
	if (inflateInit2(&(dev->d_stream), -(DEF_WBITS)) != Z_OK) {
		err("Err: inflateInit2: %d");
		dev->state = DL_START_FAIL;
	} else
		dev->zlibinit = TRUE;
}
#endif /* ZLIB */

typedef struct {
	int index;
	char *name;
} cusstr_t;

#ifdef USB_IFTEST
static void
rdl_initiate_resume(void *dev, int argc, char *argv[])
{
	usbd_resume((struct dngl_bus *)dev);
}
#endif /* USB_IFTEST */

#ifdef BCMUSBDEV_COMPOSITE
static void
usbdev_set_iso_alt_ep_maxsize(usb_endpoint_descriptor_t* d, int alt_setting)
{
	ASSERT(alt_setting >= INTERFACE_ISO);

	switch (alt_setting) {
	case 1:
		d->wMaxPacketSize = USB_MAX_ALT_INT_0_ISO_PACKET;
		break;

	case 2:
		d->wMaxPacketSize = USB_MAX_ALT_INT_1_ISO_PACKET;
		break;

	case 3:
		d->wMaxPacketSize = USB_MAX_ALT_INT_2_ISO_PACKET;
		break;

	case 4:
		d->wMaxPacketSize = USB_MAX_ALT_INT_3_ISO_PACKET;
		break;

	case 5:
		d->wMaxPacketSize = USB_MAX_ALT_INT_4_ISO_PACKET;
		break;

	case 6:
		d->wMaxPacketSize = USB_MAX_ALT_INT_5_ISO_PACKET;
		break;

	default:
		err("invalid size, setting max ISO mps %d", USB_MAX_BT_ISO_PACKET);
		d->wMaxPacketSize = USB_MAX_BT_ISO_PACKET;
		break;
	}
}

/* Setup endpoint defaults for WLAN and optional BT interface */
static void
usbdev_set_ep_defaults(struct dngl_bus *dev, int int_num)
{
	int i, j, k;
	int offset = 0;

	/* BT interface #0: intr-in, bulk-in, bulk-out */
	/* bcm_config.bNumInterface is total interface no, no include ALT */
	if (int_num > 1) {
		/* mutiple interface and WLAN at interface 0 */
		if (dev->int_wlan == 0) {
			offset = 1;
		}
	}
	j = int_num - 1 + offset;
	if (j > (INTERFACE_BT + offset)) {
		i = (INTERFACE_BT + offset);
		dev->ep_ii_num[i] = 1;
		dev->ep_bi_num[i] = 1;
		dev->ep_bo_num[i] = 1;
		dev->ep_isi_num[i] = 0;
		dev->ep_iso_num[i] = 0;
	}

	/* BT interface #1/alt settings: iso-in, iso-out */
	if (j > (INTERFACE_ISO + offset)) {
		k = INTERFACE_ISO + offset;
		for (i = k; i <= k + ALT_SETTING_MAX; i++) {
			dev->ep_ii_num[i] = 0;
			dev->ep_bi_num[i] = 0;
			dev->ep_bo_num[i] = 0;
			dev->ep_isi_num[i] = 1;
			dev->ep_iso_num[i] = 1;
		}
	}

	/* BT interface #2: DFU/0 endpoints */
	if (j > (INTERFACE_DFU + offset)) {
		i = INTERFACE_DFU + ALT_SETTING_MAX + offset;
		dev->ep_ii_num[i] = 0;
		dev->ep_bi_num[i] = 0;
		dev->ep_bo_num[i] = 0;
		dev->ep_isi_num[i] = 0;
		dev->ep_iso_num[i] = 0;
	}

	/* WLAN interface #dynamic: intr-in, bulk-in, bulk-out */
	if (j >= INTERFACE_BT) {
		if (j > INTERFACE_ISO)
			i = j + ALT_SETTING_MAX;
		else
			i = j;

		if (dev->int_wlan == 0) {
			i = 0;
		}

		dev->ep_ii_num[i] = 1;
		dev->ep_bi_num[i] = 1;
		dev->ep_bo_num[i] = 1;
		dev->ep_isi_num[i] = 0;
		dev->ep_iso_num[i] = 0;
	}
}
#endif /* BCMUSBDEV_COMPOSITE */

static void
usbdev_attach_interface_ep_init(struct dngl_bus *dev)
{
	const char *value;
	int val;
	uint i, j;
	uint NEWRDL_CONFIGLEN;
	uint ep_i_num = 1;      /* input endpoint number */
	uint ep_o_num = 1;      /* output endpoint number */

#ifdef BCMUSBDEV_COMPOSITE
	uint ep_i_num_alt = 1;	/* input endpoint number */
	uint ep_o_num_alt = 1;	/* output endpoint number */
	int usbdesc = 0;
	int int_wlan;
	int intf_iso;
	int intf_bt;
	uint k, l, iso_mps_idx = 0;
	uint ep_sum = 0;        /* total number of endpoints */

	/* init interface: support multiple interfaces, IAD with OTP override */
	dev->interface_num = 1;
	intf_iso = INTERFACE_ISO;
	intf_bt = INTERFACE_BT;
	if ((value = getvar(NULL, "usbdesc_composite")))
		usbdesc = bcm_strtoul(value, NULL, 0);

	int_wlan = 0;
	if (usbdesc) {
		i = (usbdesc & USB_IF_NUM_MASK);	/* BT interface/s */
		i ++;				/* add WLAN interface */
		if (i > 1) {
			if (i <= (INTERFACE_MAX - ALT_SETTING_MAX)) {
				dev->interface_num = i;
			} else {
				dev->interface_num = INTERFACE_MAX - ALT_SETTING_MAX;
				dbg("invalid OTP interface setting %d, limiting to %d(max)",
					i-1, dev->interface_num);
			}
		}
		if (usbdesc & USB_WL_INTF_MASK) {
			/* to handle WLAN = 0 BT_INTF = 1..... */
			int_wlan = 0;
			intf_iso = INTERFACE_ISO + 1;
			intf_bt = INTERFACE_BT + 1;
			if (dev->interface_num > 1) {
				/* WL at interface 0, BT 0, 1, 2.. */
				bcm_interfaces[1].bInterfaceNumber = intf_bt;
				bcm_interfaces[1].bAlternateSetting = 0;
				bcm_interfaces[1].bInterfaceClass = UICLASS_WIRELESS;
				bcm_interfaces[1].bInterfaceSubClass = UISUBCLASS_RF;
				bcm_interfaces[1].bInterfaceProtocol = UIPROTO_BLUETOOTH;
				bcm_interfaces[1].iInterface = 6;

				for (i = 2; i < (INTERFACE_MAX-1); i++) {
					bcm_interfaces[i].bInterfaceNumber = intf_iso;
					bcm_interfaces[i].bAlternateSetting = i-2;
					bcm_interfaces[i].bInterfaceClass = UICLASS_WIRELESS;
					bcm_interfaces[i].bInterfaceSubClass = UISUBCLASS_RF;
					bcm_interfaces[i].bInterfaceProtocol = UIPROTO_BLUETOOTH;
					bcm_interfaces[i].iInterface = 6;
				}
				bcm_interfaces[8].bInterfaceNumber = INTERFACE_DFU + 1;
				bcm_interfaces[8].bAlternateSetting = 0;
				bcm_interfaces[8].bInterfaceClass = UICLASS_WIRELESS;
				bcm_interfaces[8].bInterfaceSubClass = UISUBCLASS_RF;
				bcm_interfaces[8].bInterfaceProtocol = UIPROTO_BLUETOOTH;
				bcm_interfaces[8].iInterface = 7;
			}
		} else {
			int_wlan = dev->interface_num - 1;
		}
	}
	dev->int_wlan = int_wlan;

	if (int_wlan > INTERFACE_ISO)
		int_wlan += ALT_SETTING_MAX;

	if (int_wlan < (INTERFACE_MAX - 1)) {
		if (int_wlan > INTERFACE_ISO) {
			i = int_wlan;
		} else {
			i = dev->int_wlan;
		}
		bcm_interfaces[i].bInterfaceNumber = dev->int_wlan;
		bcm_interfaces[i].bInterfaceClass = UICLASS_VENDOR;
		bcm_interfaces[i].bInterfaceSubClass = UISUBCLASS_ABSTRACT_CONTROL_MODEL;
		bcm_interfaces[i].bInterfaceProtocol = UIPROTO_DATA_VENDOR;
		bcm_interfaces[i].iInterface = 2;
	}
	bcm_config.bNumInterface = dev->interface_num;
	bcm_other_config.bNumInterface = dev->interface_num;

	/* update settings if this is a composite device */
	if (dev->interface_num > 1) {
		/* composite without IAD */
		bcm_device.bDeviceClass = UDCLASS_WIRELESS;
		bcm_device.bDeviceSubClass = UDSUBCLASS_RF;
		bcm_device.bDeviceProtocol = UDPROTO_BLUETOOTH;
		bcm_hs_qualifier.bDeviceClass = UDCLASS_WIRELESS;
		bcm_hs_qualifier.bDeviceSubClass = UDSUBCLASS_RF;
		bcm_hs_qualifier.bDeviceProtocol = UDPROTO_BLUETOOTH;
		bcm_fs_qualifier.bDeviceClass = UDCLASS_WIRELESS;
		bcm_fs_qualifier.bDeviceSubClass = UDSUBCLASS_RF;
		bcm_fs_qualifier.bDeviceProtocol = UDPROTO_BLUETOOTH;
		bcm_device.iProduct = 4;
	}

	if (dev->interface_num > 2) {
		/* composite with IAD */
		if (usbdesc & USB_IAD_MASK) {
			dbg("IAD enabled");
			bcm_device.bDeviceClass = UDCLASS_MISC;
			bcm_device.bDeviceSubClass = UDSUBCLASS_COMMON;
			bcm_device.bDeviceProtocol = UDPROTO_IAD;
			bcm_hs_qualifier.bDeviceClass = UDCLASS_MISC;
			bcm_hs_qualifier.bDeviceSubClass = UDSUBCLASS_COMMON;
			bcm_hs_qualifier.bDeviceProtocol = UDPROTO_IAD;
			bcm_fs_qualifier.bDeviceClass = UDCLASS_MISC;
			bcm_fs_qualifier.bDeviceSubClass = UDSUBCLASS_COMMON;
			bcm_fs_qualifier.bDeviceProtocol = UDPROTO_IAD;
			bcm_device.iProduct = 4;
			bcm_iad.bInterfaceCount = bcm_config.bNumInterface - 1;
			if (dev->int_wlan == 0) {
				/* WLAN at interface 0, push rad_iad.bFirstInterface */
				bcm_iad.bFirstInterface = IAD_INT_NUM + 1;
			}
		}
		dev->interface_num += ALT_SETTING_MAX;
	}

	/* Device Class Override */
	if ((val = (usbdesc & USB_DEVCLASS_MASK) >> USB_DEVCLASS_SHIFT)) {
		if (val == USB_DEVCLASS_BT) {
			dbg("devclass BT override");
			bcm_device.bDeviceClass = UDCLASS_WIRELESS;
			bcm_device.bDeviceSubClass = UDSUBCLASS_RF;
			bcm_device.bDeviceProtocol = UDPROTO_BLUETOOTH;
			bcm_hs_qualifier.bDeviceClass = UDCLASS_WIRELESS;
			bcm_hs_qualifier.bDeviceSubClass = UDSUBCLASS_RF;
			bcm_hs_qualifier.bDeviceProtocol = UDPROTO_BLUETOOTH;
			bcm_fs_qualifier.bDeviceClass = UDCLASS_WIRELESS;
			bcm_fs_qualifier.bDeviceSubClass = UDSUBCLASS_RF;
			bcm_fs_qualifier.bDeviceProtocol = UDPROTO_BLUETOOTH;
			bcm_device.iProduct = 4;
		} else if (val == USB_DEVCLASS_WLAN) {
			dbg("devclass WLAN override");
			bcm_device.bDeviceClass = UDCLASS_VENDOR;
			bcm_device.bDeviceSubClass = 0;
			bcm_device.bDeviceProtocol = 0;
			bcm_hs_qualifier.bDeviceClass = UDCLASS_VENDOR;
			bcm_hs_qualifier.bDeviceSubClass = 0;
			bcm_hs_qualifier.bDeviceProtocol = 0;
			bcm_fs_qualifier.bDeviceClass = UDCLASS_VENDOR;
			bcm_fs_qualifier.bDeviceSubClass = 0;
			bcm_fs_qualifier.bDeviceProtocol = 0;
			bcm_device.iProduct = 2;
		}
	}

	/* init endpoints: support multiple EP and OTP override */
	ASSERT(bcm_config.bNumInterface <= (INTERFACE_MAX - ALT_SETTING_MAX));
	usbdev_set_ep_defaults(dev, bcm_config.bNumInterface);
#ifdef BCMUSBDEV_BULKIN_2EP
	if (!usbdesc)
		dev->ep_bi_num[int_wlan] = 2;
#endif


	if (usbdesc) {
		/* BT bulk endpoints # */
		if (BT_BULK_IN_NUM(usbdesc)) {
			dev->ep_bi_num[intf_bt] ++;
		}
		if (BT_BULK_OUT_NUM(usbdesc)) {
			dev->ep_bo_num[intf_bt] ++;
		}
		val = (usbdesc & USB_WLANIF_INTR_EP_MASK) >> USB_WLANIF_INTR_EP_SHIFT;
		if (val) {
			dev->ep_ii_num[int_wlan] = 0;
		} else {
			dev->ep_ii_num[int_wlan] = 1;
		}

		val = ((usbdesc & USB_BI_EP_MASK) >> USB_BI_EP_SHIFT);
		if (val > 0 && val < EP_BULK_MAX)
			dev->ep_bi_num[int_wlan] += val;

		val = ((usbdesc & USB_B0_EP_MASK) >> USB_B0_EP_SHIFT);
		if (val > 0 && val < EP_BULK_MAX)
			dev->ep_bo_num[int_wlan] += val;

		/* Max 4-in and 4-out endpoints
		 * Adjust endpoint allocation if OTP setting exceeds the limit
		 */
		if (bcm_config.bNumInterface > 2) {
			dev->ep_ii_num[int_wlan] = 0;
			if (dev->ep_bi_num[int_wlan] > 2)
				dev->ep_bi_num[int_wlan] = 2;
			if (dev->ep_bo_num[int_wlan] > 2)
				dev->ep_bo_num[int_wlan] = 2;
		} else if (bcm_config.bNumInterface > 1) {
			if (dev->ep_ii_num[int_wlan]) {
				dev->ep_bi_num[int_wlan] = 1;
			} else if (dev->ep_bi_num[int_wlan] > 2) {
				dev->ep_bi_num[int_wlan] = 2;
			}
			if (dev->ep_bo_num[int_wlan] > 3)
				dev->ep_bo_num[int_wlan] = 3;
		}
	}

	ep_sum = 0;
	for (k = 0; k < dev->interface_num; k++) {
		bcm_interfaces[k].bNumEndpoints = dev->ep_ii_num[k]
			+ dev->ep_bi_num[k] + dev->ep_bo_num[k]
			+ dev->ep_isi_num[k] + dev->ep_iso_num[k];
		ep_sum += bcm_interfaces[k].bNumEndpoints;
	}

	NEWRDL_CONFIGLEN = USB_CONFIG_DESCRIPTOR_SIZE +
		(USB_INTERFACE_DESCRIPTOR_SIZE * dev->interface_num) +
		(USB_ENDPOINT_DESCRIPTOR_SIZE * ep_sum);

#ifdef USB_XDCI
	if (usbd_chk_version(dev) > UD_USB_2_1_DEV) {
		NEWRDL_CONFIGLEN += USB_ENDPOINT_COMPANION_DESCRIPTOR_SIZE * ep_sum;
	}
#endif /* USB_XDCI */
	if (usbdesc & USB_IAD_MASK) {
		if (bcm_iad.bInterfaceCount > 1) {
			NEWRDL_CONFIGLEN += USB_INTERFACE_ASSOCIATION_DESCRIPTOR_SIZE;
		} else {
			dbg("Invalid OTP IAD setting, interface count is %d(<2)",
				bcm_iad.bInterfaceCount);
		}
	}
	if (dev->interface_num == INTERFACE_MAX)
		NEWRDL_CONFIGLEN += USB_DFU_FUNCTIONAL_DESCRIPTOR_SIZE;

	dev->data_interface = bcm_interfaces;
	dev->iad = &bcm_iad;
	dev->interface = dev->int_wlan;

	dbg("Endpoint: intr %d, bulkin %d, bulkout %d",
		dev->ep_ii_num[int_wlan], dev->ep_bi_num[int_wlan],
		dev->ep_bo_num[int_wlan]);

	trace("ei %d, ebi %d, ebo %d", dev->ep_ii_num[int_wlan],
		dev->ep_bi_num[int_wlan], dev->ep_bo_num[int_wlan]);

	/* init config */
	bcm_config.wTotalLength = NEWRDL_CONFIGLEN;
	bcm_other_config.wTotalLength = NEWRDL_CONFIGLEN;

	if ((CHIPID(dev->sih->chip) == BCM43242_CHIP_ID) ||
		(CHIPID(dev->sih->chip) == BCM43243_CHIP_ID) ||
		BCM4350_CHIP(dev->sih->chip) ||
		0) {
		bcm_config.bMaxPower = DEV_MAXPOWER_500;
		bcm_other_config.bMaxPower = DEV_MAXPOWER_500;
	}

	bcopy(&bcm_config, &dev->config, sizeof(dev->config));
	bcopy(&bcm_other_config, &dev->other_config, sizeof(dev->other_config));

	/* init device */
	bcopy(&bcm_device, &dev->device, sizeof(dev->device));

#ifdef USB_XDCI
		if (usbd_chk_version(dev) == UD_USB_2_1_DEV) {
			dev->device.bcdUSB = UD_USB_2_1;
			dev->device.bMaxPacketSize = USB_2_MAX_CTRL_PACKET;
		} else if (usbd_chk_version(dev) == UD_USB_3_DEV) {
			dev->device.bcdUSB = UD_USB_3_0;
			dev->device.bMaxPacketSize = USB_3_MAX_CTRL_PACKET_FACTORIAL;
			dev->config.bMaxPower = (uByte)DEV_MAXPOWER_900;
		}
#endif /* USB_XDCI */

	/* build EP list for hs and fs:in EPs first, then out EPs
	 * fixup the ep_num. The DMA channel and ep number will be allocated inside ep_attach()
	 */
	j = 0;
	l = 0;

	/* handle WLAN = 0, BT = 1 ...... */

	if (usbdesc)
		iso_mps_idx = (usbdesc & USB_ISO_MPS_MASK) >> USB_ISO_MPS_SHIFT;
	for (k = 0; k < dev->interface_num; k++) {
		if (bcm_interfaces[k].bAlternateSetting) {
			ep_i_num = ep_i_num_alt;
			ep_o_num = ep_o_num_alt;
		} else {
			ep_i_num_alt = ep_i_num;
			ep_o_num_alt = ep_o_num;
		}

		for (i = 0; i < dev->ep_ii_num[k]; i++) {
#ifdef USB_XDCI
			dev->intr_endpoints_ss[i+l] = bcm_ss_ep_intr;
			dev->intr_endpoints_cp_ss[i+l] = bcm_ss_cp_intr;
#endif /* USB_XDCI */
			dev->intr_endpoints[i+l] = bcm_hs_ep_intr;
			dev->intr_endpoints_fs[i+l] = bcm_fs_ep_intr;

#ifdef USB_XDCI
			dev->intr_endpoints_ss[i+l].bEndpointAddress =
			UE_SET_ADDR((dev->intr_endpoints_ss[i+l].bEndpointAddress), ep_i_num);
#endif /* USB_XDCI */
			dev->intr_endpoints[i+l].bEndpointAddress =
			UE_SET_ADDR((dev->intr_endpoints[i+l].bEndpointAddress), ep_i_num);
			dev->intr_endpoints_fs[i+l].bEndpointAddress =
			UE_SET_ADDR((dev->intr_endpoints_fs[i+l].bEndpointAddress), ep_i_num);
			/* make the in and out start from the same after interrupt endpoints */
			ep_i_num ++;
			ep_o_num = ep_i_num;
		}
		l += i;

		for (i = 0; i < dev->ep_bi_num[k]; i++) {
#ifdef USB_XDCI
			dev->data_endpoints_ss[i+j] = bcm_ss_ep_bulkin;
			dev->data_endpoints_cp_ss[i+j] = bcm_ss_cp_bulkin;
#endif /* USB_XDCI */
			dev->data_endpoints[i+j] = bcm_hs_ep_bulkin;
			dev->data_endpoints_fs[i+j] = bcm_fs_ep_bulkin;
#ifdef USB_XDCI
			dev->data_endpoints_ss[i+j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints_ss[i+j].bEndpointAddress), ep_i_num);
#endif /* USB_XDCI */
			dev->data_endpoints[i+j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints[i+j].bEndpointAddress), ep_i_num);
			dev->data_endpoints_fs[i+j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints_fs[i+j].bEndpointAddress), ep_i_num);
			ep_i_num++;
		}
		j += i;

		for (i = 0; i < dev->ep_bo_num[k]; i++) {
#ifdef USB_XDCI
			dev->data_endpoints_ss[i + j] = bcm_ss_ep_bulkout;
			dev->data_endpoints_cp_ss[i + j] = bcm_ss_cp_bulkout;
#endif /* USB_XDCI */
			dev->data_endpoints[i + j] = bcm_hs_ep_bulkout;
			dev->data_endpoints_fs[i + j] = bcm_fs_ep_bulkout;
#ifdef USB_XDCI
			dev->data_endpoints_ss[i + j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints_ss[i + j].bEndpointAddress), ep_o_num);
#endif /* USB_XDCI */
			dev->data_endpoints[i + j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints[i + j].bEndpointAddress), ep_o_num);
			dev->data_endpoints_fs[i + j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints_fs[i + j].bEndpointAddress), ep_o_num);
			ep_o_num ++;
		}
		j += i;

		for (i = 0; i < dev->ep_isi_num[k]; i++) {
#ifdef USB_XDCI
			dev->data_endpoints_ss[i + j] = bcm_ss_ep_isin;
			dev->data_endpoints_cp_ss[i + j] = bcm_ss_cp_isin;
			if (iso_mps_idx)
				usbdev_set_iso_alt_ep_maxsize(&dev->data_endpoints_ss[i + j],
					iso_mps_idx);
			else
				usbdev_set_iso_alt_ep_maxsize(&dev->data_endpoints_ss[i + j], k);
#endif /* USB_XDCI */
			dev->data_endpoints[i + j] = bcm_hs_ep_isin;
			if (iso_mps_idx)
				usbdev_set_iso_alt_ep_maxsize(&dev->data_endpoints[i + j],
					iso_mps_idx);
			else
				usbdev_set_iso_alt_ep_maxsize(&dev->data_endpoints[i + j], k);
			dev->data_endpoints_fs[i + j] = bcm_fs_ep_isin;
			if (iso_mps_idx)
				usbdev_set_iso_alt_ep_maxsize(&dev->data_endpoints_fs[i + j],
					iso_mps_idx);
			else
				usbdev_set_iso_alt_ep_maxsize(&dev->data_endpoints_fs[i + j], k);
#ifdef USB_XDCI
			dev->data_endpoints_ss[i + j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints_ss[i + j].bEndpointAddress), ep_i_num);
#endif /* USB_XDCI */
			dev->data_endpoints[i + j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints[i + j].bEndpointAddress), ep_i_num);
			dev->data_endpoints_fs[i + j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints_fs[i + j].bEndpointAddress), ep_i_num);
			ep_i_num ++;
		}
		j += i;

		for (i = 0; i < dev->ep_iso_num[k]; i++) {
#ifdef USB_XDCI
			dev->data_endpoints_ss[i + j] = bcm_ss_ep_isout;
			dev->data_endpoints_cp_ss[i + j] = bcm_ss_cp_isout;
			if (iso_mps_idx)
				usbdev_set_iso_alt_ep_maxsize(&dev->data_endpoints_ss[i + j],
					iso_mps_idx);
			else
				usbdev_set_iso_alt_ep_maxsize(&dev->data_endpoints_ss[i + j], k);
#endif /* USB_XDCI */
			dev->data_endpoints[i + j] = bcm_hs_ep_isout;
			if (iso_mps_idx)
				usbdev_set_iso_alt_ep_maxsize(&dev->data_endpoints[i + j],
					iso_mps_idx);
			else
				usbdev_set_iso_alt_ep_maxsize(&dev->data_endpoints[i + j], k);
			dev->data_endpoints_fs[i + j] = bcm_fs_ep_isout;
			if (iso_mps_idx)
				usbdev_set_iso_alt_ep_maxsize(&dev->data_endpoints_fs[i + j],
					iso_mps_idx);
			else
				usbdev_set_iso_alt_ep_maxsize(&dev->data_endpoints_fs[i + j], k);
#ifdef USB_XDCI
			dev->data_endpoints_ss[i + j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints_ss[i + j].bEndpointAddress), ep_o_num);
#endif /* USB_XDCI */
			dev->data_endpoints[i + j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints[i + j].bEndpointAddress), ep_o_num);
			dev->data_endpoints_fs[i + j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints_fs[i + j].bEndpointAddress), ep_o_num);
			ep_o_num++;
		}
		j += i;
	}
#else
	/* init endpoint: support multiple EP, and OTP override */
	dev->ep_ii_num = 1;
	dev->ep_bi_num = 1;
	dev->ep_bo_num = 1;
#ifdef BCMUSBDEV_BULKIN_2EP
	dev->ep_bi_num = 2;
#endif


	/* only support multiple BULK EPs for now, only one Interrupt EP */
	if ((value = getvar(NULL, "usbepnum"))) {
		i = bcm_strtoul(value, NULL, 0);
		val = (i & 0x0f);
		if (val > 0 && val <= EP_BULK_MAX) {
			dev->ep_bi_num = val;
		}
		val = (i & 0xf0) >> 4;
		if (val > 0 && val <= EP_BULK_MAX) {
			dev->ep_bo_num = val;
		}
	}

	bcm_interface.bNumEndpoints = dev->ep_ii_num + dev->ep_bi_num + dev->ep_bo_num;

	NEWRDL_CONFIGLEN = USB_CONFIG_DESCRIPTOR_SIZE + USB_INTERFACE_DESCRIPTOR_SIZE +
		(USB_ENDPOINT_DESCRIPTOR_SIZE * bcm_interface.bNumEndpoints);

#ifdef USB_XDCI
	if (usbd_chk_version(dev) > UD_USB_2_1_DEV) {
		NEWRDL_CONFIGLEN += USB_ENDPOINT_COMPANION_DESCRIPTOR_SIZE
		* bcm_interface.bNumEndpoints;
	}
#endif /* USB_XDCI */

	/* init interface */
	dev->data_interface = &bcm_interface;

	dbg("Endpoint: intr %d, bulkin %d, bulkout %d totconflen %d",
		dev->ep_ii_num, dev->ep_bi_num, dev->ep_bo_num, NEWRDL_CONFIGLEN);

	trace("ei %d, ebi %d, ebo %d", dev->ep_ii_num, dev->ep_bi_num, dev->ep_bo_num);

	/* init config */
	bcm_config.wTotalLength = NEWRDL_CONFIGLEN;
	bcm_other_config.wTotalLength = NEWRDL_CONFIGLEN;
	bcopy(&bcm_config, &dev->config, sizeof(dev->config));
	bcopy(&bcm_other_config, &dev->other_config, sizeof(dev->other_config));

	/* init device */
	bcopy(&bcm_device, &dev->device, sizeof(usb_device_descriptor_t));

#ifdef USB_XDCI
	if (usbd_chk_version(dev) == UD_USB_2_1_DEV) {
		dev->device.bcdUSB = UD_USB_2_1;
		dev->device.bMaxPacketSize = USB_2_MAX_CTRL_PACKET;
	} else if (usbd_chk_version(dev) == UD_USB_3_DEV) {
		dev->device.bcdUSB = UD_USB_3_0;
		dev->device.bMaxPacketSize = USB_3_MAX_CTRL_PACKET_FACTORIAL;
		dev->config.bMaxPower = (uByte)DEV_MAXPOWER_900;
	}
#endif /* USB_XDCI */
	ep_i_num = 2;
	/* build EP list for hs and fs: bulk in EPs first, then bulk out EPs
	 * fixup the ep_num. The DMA channel and ep number will be allocated inside ep_attach()
	 */
	for (i = 0; i < dev->ep_bi_num; i++) {
#ifdef USB_XDCI
		dev->data_endpoints_cp_ss[i] = bcm_ss_cp_bulkin;
		dev->data_endpoints_ss[i] = bcm_ss_ep_bulkin;
		dev->data_endpoints_ss[i].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints_ss[i].bEndpointAddress), ep_i_num);
#endif /* USB_XDCI */
		dev->data_endpoints[i] = bcm_hs_ep_bulkin;
		dev->data_endpoints_fs[i] = bcm_fs_ep_bulkin;
		dev->data_endpoints[i].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints[i].bEndpointAddress), ep_i_num);
		dev->data_endpoints_fs[i].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints_fs[i].bEndpointAddress), ep_i_num);
		ep_i_num++;
	}
	j = i;
	ep_o_num = ep_i_num;

	for (i = 0; i < dev->ep_bo_num; i++) {
#ifdef USB_XDCI
		dev->data_endpoints_cp_ss[i + j] = bcm_ss_cp_bulkout;
		dev->data_endpoints_ss[i + j] = bcm_ss_ep_bulkout;
		dev->data_endpoints_ss[i + j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints_ss[i + j].bEndpointAddress), ep_o_num);
#endif /* USB_XDCI */
		dev->data_endpoints[i + j] = bcm_hs_ep_bulkout;
		dev->data_endpoints_fs[i + j] = bcm_fs_ep_bulkout;
		dev->data_endpoints[i + j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints[i + j].bEndpointAddress), ep_o_num);
		dev->data_endpoints_fs[i + j].bEndpointAddress =
			UE_SET_ADDR((dev->data_endpoints_fs[i + j].bEndpointAddress), ep_o_num);
		ep_o_num++;
	}
#endif /* BCMUSBDEV_COMPOSITE */
}


#ifdef USB_XDCI
/* init usb chip function table for xdci chip */
static void usbdev_init_chip_functable_xdci(struct usbchip_func *func)
{
	func->tx = xdci_tx;
	func->tx_resp = xdci_tx;
	func->rxflowcontrol = xdci_rxflowcontrol;
	func->resume = xdci_resume;
	func->stall_ep = xdci_ep_stall;
	func->usbversion = xdci_usb30;
	func->hsic = xdci_hsic;
	func->ep_attach = xdci_ep_attach;
	func->ep_detach = xdci_ep_detach;
}
#endif
/* init usb chip function table for usb20d chip */
static void usbdev_init_chip_functable_ch(struct usbchip_func *func)
{
	func->tx = ch_tx;
	func->tx_resp = ch_tx_resp;
	func->rxflowcontrol = ch_rxflowcontrol;
	func->resume = ch_resume;
#ifdef BCMUSBDEV_COMPOSITE
	func->stall_ep = ch_stall_ep;
#endif
	func->usbversion = ch_usb20;
	func->hsic = ch_hsic;
	func->ep_attach = ep_attach;
	func->ep_detach = ep_detach;
}


struct dngl_bus *
usbdev_attach(osl_t *osh, struct usbdev_chip *ch, si_t *sih)
{
	struct dngl_bus *dev;
	const char *value;
	int i;
	char **cp;
	int val;
	/* bootloader nvram override options */
	cusstr_t *s, custom_strings[] = {
		{ 1, "manf" },
		{ 2, "productname" },
		{ 3, "rdlsn" },
		{ 0, NULL }
		};
	char *usb_bl_vid_nvoptions[] = {
		"manfid",	/* standard cis tuple */
		"subvendid",	/* Broadcom cis tuple */
		NULL
		};

	char *usb_bl_pid_nvoptions[] = {
		"rdlid",
		NULL
		};

	trace("RDL");

	if (!(dev = MALLOC(osh, sizeof(struct dngl_bus)))) {
		err("out of memory");
		return NULL;
	}
	bzero(dev, sizeof(struct dngl_bus));
	dev->osh = osh;
	dev->ch = ch;
	dev->sih = sih;

#ifdef USB_XDCI
	if (bcmrdl.flags & (1 << RTEDEVFLAG_USB30)) {
		/* xdci core is selected */
		usbdev_init_chip_functable_xdci(&dev->ch_func);
		dev->usb30d = TRUE;
	} else
#endif /* USB_XDCI */
	{
		usbdev_init_chip_functable_ch(&dev->ch_func); /* usb20core chip */
		dev->usb30d = FALSE;
	}

	if ((value = getvar(NULL, "rdlrndis"))) {
		dev->rndis = (bool) bcm_strtoul(value, NULL, 0);
	}

	if (dev->rndis) {
		err("RNDIS de-supported");
		goto fail;
	}

	usbdev_attach_interface_ep_init(dev);

	for (i = 0; i < ARRAYSIZE(dev->strings); i++) {
		bcopy(&bcm_strings[i], &dev->strings[i], sizeof(usb_string_descriptor_t));
	}

	/* Convert custom ASCII strings to UTF16 (not NULL terminated) */
	for (s = custom_strings; s->name; s++) {
		if ((value = getvar(NULL, s->name))) {
			dbg("found %s string %s", value, s->name);
			for (i = 0; i < strlen(value) && i < (USB_MAX_STRING_LEN); i++)
				dev->strings[s->index].bString[i] = (uWord) value[i];
			dev->strings[s->index].bLength = (i * sizeof(uWord)) + 2;
		}
	}

	/* Support custom manf and product IDs for PnP (both CIS-srom & nvram style) */
	for (cp = usb_bl_vid_nvoptions; *cp != NULL; ++cp) {
		if ((value = getvar(NULL, *cp)) && (val = bcm_strtoul(value, NULL, 0))) {
			dbg("found manf string %s, val 0x%x", value, val);
			dev->device.idVendor = val;
			break;
		}
	}

	/* bootloader has a pnp id separate from the downloaded driver */
	for (cp = usb_bl_pid_nvoptions; *cp != NULL; ++cp) {
		if ((value = getvar(NULL, *cp)) && (val = bcm_strtoul(value, NULL, 0))) {
			dbg("found rdlid string %s, val 0x%x", value, val);
			dev->device.idProduct = val;
			break;
		}
	}

	/* remote wakeup capability override */
	if ((value = getvar(NULL, "rdlrwu"))) {
		i = bcm_strtoul(value, NULL, 0);
		dbg("found rdlrwu val %d", i);
		if (i == 0) {	/* honor disable since default is on */
			dev->config.bmAttributes &= ~UC_REMOTE_WAKEUP;
			dev->other_config.bmAttributes &= ~UC_REMOTE_WAKEUP;
		}
	}

#ifdef USB_IFTEST
	if (dev->config.bmAttributes & UC_REMOTE_WAKEUP) {
		if (!hnd_cons_add_cmd("rmwk", rdl_initiate_resume, dev))
			goto fail;
	}
#endif /* USB_IFTEST */

	/* HSIC is self-powered so change the descriptors to reflect this */
	if (usbd_hsic(dev)) {
		dev->config.bmAttributes &= ~UC_BUS_POWERED;
		dev->config.bmAttributes |= UC_SELF_POWERED;
		dev->config.bMaxPower = 0;
		dev->other_config.bmAttributes &= ~UC_BUS_POWERED;
		dev->other_config.bmAttributes |= UC_SELF_POWERED;
		dev->other_config.bMaxPower = 0;
	}

	dev->state = -1;
	usbdev_rdl_dl_state_reset(dev);

	trace("done");
	return dev;
fail:
	if (dev)
		MFREE(osh, dev, sizeof(struct dngl_bus));

	return NULL;
}

void
usbdev_detach(struct dngl_bus *dev)
{
	trace("");

	MFREE(dev->osh, dev, sizeof(struct dngl_bus));
	/* ensure cache state is in sync after code copy */
	FLUSH_DCACHE();
	INV_ICACHE();

	trace("done");
}

static void
usbdev_setup_ep_report(struct dngl_bus *dev, uchar **cur, uint speed)
{
	uint i;
	usb_endpoint_descriptor_t *data_endpoints;
#ifdef USB_XDCI
	usb_endpoint_companion_descriptor_t *data_endpoints_cp = NULL;
#endif /* USB_XDCI */

	trace("cur %p", *cur);

#ifdef BCMUSBDEV_COMPOSITE
	uint j, k, l;
	usb_endpoint_descriptor_t *intr_endpoints;
#ifdef USB_XDCI
	usb_endpoint_companion_descriptor_t *intr_endpoints_cp = NULL;
#endif

	j = 0;
	l = 0;
#ifdef USB_XDCI
	if (speed == UD_USB_SS) {
		intr_endpoints = dev->intr_endpoints_ss;
		data_endpoints = dev->data_endpoints_ss;
		intr_endpoints_cp = dev->intr_endpoints_cp_ss;
		data_endpoints_cp = dev->data_endpoints_cp_ss;

	} else
#endif /* USB_XDCI */
	{
		intr_endpoints = (speed) ? dev->intr_endpoints : dev->intr_endpoints_fs;
		data_endpoints = (speed) ? dev->data_endpoints : dev->data_endpoints_fs;
	}

	for (k = 0; k < dev->interface_num; k++) {
		uint32 intf_dfu = INTERFACE_DFU;

		if (dev->int_wlan == 0 && dev->interface_num > 1)
			intf_dfu = INTERFACE_DFU + 1;

		if ((bcm_iad.bInterfaceCount > 1) && (k == bcm_iad.bFirstInterface))
			*cur += usbdev_htol_usb_iad(dev->iad, *cur);
		*cur += usbdev_htol_usb_interface_descriptor(&dev->data_interface[k], *cur);

		if (bcm_interfaces[k].bInterfaceNumber == intf_dfu &&
			bcm_interfaces[k].bInterfaceNumber != dev->int_wlan) {
			*cur += usbdev_htol_usb_dfu_functional_descriptor(&bcm_dfu, *cur);
		}

		for (i = 0; i < dev->ep_ii_num[k]; i++) {
			*cur += usbdev_htol_usb_endpoint_descriptor(&intr_endpoints[i+l], *cur);
#ifdef USB_XDCI
			if (speed == UD_USB_SS)
				*cur += usbdev_htol_usb_ed_companion_descriptor(
					&intr_endpoints_cp[i+l], *cur);
#endif /* USB_XDCI */
		}
		l += i;

		for (i = 0; i < dev->ep_bi_num[k]; i++) {
			*cur += usbdev_htol_usb_endpoint_descriptor(&data_endpoints[i+j], *cur);
#ifdef USB_XDCI
			if (speed == UD_USB_SS)
				*cur += usbdev_htol_usb_ed_companion_descriptor(
					&data_endpoints_cp[i+j], *cur);
#endif /* USB_XDCI */
		}
		j += i;

		for (i = 0; i < dev->ep_bo_num[k]; i++) {
			*cur += usbdev_htol_usb_endpoint_descriptor(&data_endpoints[i+j], *cur);
#ifdef USB_XDCI
			if (speed == UD_USB_SS)
				*cur += usbdev_htol_usb_ed_companion_descriptor(
					&data_endpoints_cp[i+j], *cur);
#endif /* USB_XDCI */
		}
		j += i;

		for (i = 0; i < dev->ep_isi_num[k]; i++) {
			*cur += usbdev_htol_usb_endpoint_descriptor(&data_endpoints[i+j], *cur);
#ifdef USB_XDCI
			if (speed == UD_USB_SS)
				*cur += usbdev_htol_usb_ed_companion_descriptor(
					&data_endpoints_cp[i+j], *cur);
#endif /* USB_XDCI */

		}
		j += i;

		for (i = 0; i < dev->ep_iso_num[k]; i++) {
			*cur += usbdev_htol_usb_endpoint_descriptor(&data_endpoints[i+j], *cur);
#ifdef USB_XDCI
			if (speed == UD_USB_SS)
				*cur += usbdev_htol_usb_ed_companion_descriptor(
					&data_endpoints_cp[i+j], *cur);
#endif
		}
		j += i;
	}
#else
	*cur += usbdev_htol_usb_interface_descriptor(dev->data_interface, *cur);

	for (i = 0; i < dev->ep_ii_num; i++) {
#ifdef USB_XDCI
		if (speed == UD_USB_SS) {
			data_endpoints = &bcm_ss_ep_intr;
			data_endpoints_cp = &bcm_ss_cp_intr;
		} else
#endif /* USB_XDCI */
		data_endpoints = (speed) ? &bcm_hs_ep_intr : &bcm_fs_ep_intr;
		*cur += usbdev_htol_usb_endpoint_descriptor(data_endpoints, *cur);
#ifdef USB_XDCI
		if (speed == UD_USB_SS) {
			*cur += usbdev_htol_usb_ed_companion_descriptor(data_endpoints_cp, *cur);
		}
#endif /* USB_XDCI */
	}

	for (i = 0; i < dev->ep_bi_num + dev->ep_bo_num; i++) {
#ifdef USB_XDCI
		if (speed == UD_USB_SS) {
			data_endpoints_cp = &dev->data_endpoints_cp_ss[i];
			data_endpoints = &dev->data_endpoints_ss[i];
		} else
#endif /* USB_XDCI */
		data_endpoints = (speed) ? &dev->data_endpoints[i] : &dev->data_endpoints_fs[i];
		*cur += usbdev_htol_usb_endpoint_descriptor(data_endpoints, *cur);
#ifdef USB_XDCI
		if (speed == UD_USB_SS)
			*cur += usbdev_htol_usb_ed_companion_descriptor(data_endpoints_cp, *cur);
#endif /* USB_XDCI */

	}
#endif /* BCMUSBDEV_COMPOSITE */

	trace("cur %p after adding EPs", *cur);
}

void *
usbdev_setup(struct dngl_bus *dev, int ep, void *p, int *errp, int *dir)
{
	usb_device_request_t dr;
	rdl_state_t *rdl_state;
	bootrom_id_t *id;
	void *p0 = NULL;
	int i;
	uint speed;

	trace("RDL");
	ASSERT(ep == 0);

	ltoh_usb_device_request(PKTDATA(dev->osh, p), &dr);

	*dir = dr.bmRequestType & UT_READ;
	/* Get standard descriptor */
	/* open it for all UT_READ_DEVICE, UT_READ_INTERFACE, UT_READ_ENDPOINT */
	if ((dr.bmRequestType&0xF0) == UT_READ_DEVICE) {
		uchar *cur = NULL;
		int bufsize = 256;
		if (dr.bRequest == UR_GET_DESCRIPTOR) {
			uchar request = (dr.wValue >> 8) & 0xff;
#ifdef BCMUSBDEV_COMPOSITE
			if (bcm_config.wTotalLength > bufsize)
				bufsize *= 2;
#endif /* BCMUSBDEV_COMPOSITE */
			if (!(p0 = PKTGET(dev->osh, bufsize, TRUE))) {
				err("ep%d: out of txbufs", ep);
				goto stall;
			}
			cur = PKTDATA(dev->osh, p0);

			switch (request) {
			case UDESC_DEVICE:
				{
					const char *otp_val;
					dbg("ep%d: UDESC_DEVICE", ep);
					if ((otp_val = getvar(NULL, "bldr_to")) != NULL) {
						si_watchdog_ms(dev->sih, 0);
					}
					cur += htol_usb_device_descriptor(&dev->device, cur);
				}
				break;
			case UDESC_CONFIG:
			case UDESC_OTHER_SPEED_CONFIGURATION:
				dbg("ep%d: %s", ep, request == UDESC_CONFIG ? "UDESC_CONFIG" :
					"UDESC_OTHER_SPEED_CONFIGURATION");
				if (request == UDESC_CONFIG) {
					cur += usbdev_htol_usb_config_descriptor(
						&dev->config, cur);
					speed = dev->speed;
				} else {
					cur += usbdev_htol_usb_config_descriptor(
						&dev->other_config, cur);
					speed = !dev->speed;
				}
				if (request == UDESC_CONFIG ||
					usbd_chk_version(dev) >= UD_USB_2_0_DEV) {
					if (dr.wLength > 9)
						usbdev_setup_ep_report(dev, &cur, speed);
					break;
				} else {
					err("ep%d: %s failed", ep,
					    request == UDESC_CONFIG ? "UDESC_CONFIG" :
					    "UDESC_OTHER_SPEED_CONFIGURATION");
					goto stall;
				}
			case UDESC_INTERFACE:
				i = dr.wValue & 0xff;
				dbg("ep%d: UDESC_INTERFACE %d", ep, i);
#ifdef BCMUSBDEV_COMPOSITE
				if ((i >= 0) && (i < bcm_config.bNumInterface)) {
					uint32 intf_iso = INTERFACE_ISO;
					if (dev->int_wlan == 0) {
						intf_iso += 1;
					}
					if (i > intf_iso)
						cur += usbdev_htol_usb_interface_descriptor(
						&dev->data_interface[i + ALT_SETTING_MAX], cur);
					else
						cur += usbdev_htol_usb_interface_descriptor(
							&dev->data_interface[i], cur);
				}

#else
				if (i == dev->data_interface->bInterfaceNumber)
					cur += usbdev_htol_usb_interface_descriptor(
						dev->data_interface, cur);
#endif /* BCMUSBDEV_COMPOSITE */
				else {
					err("UDESC_INTERFACE query failed");
					goto stall;
				}
				break;
			case UDESC_STRING:
				i = dr.wValue & 0xff;
				dbg("ep%d: UDESC_STRING %d langid 0x%x", ep, i, dr.wIndex);
#ifdef BCMUSBDEV_COMPOSITE
				if (i > 7) {
#else
				if (i > 3) {
#endif /* BCMUSBDEV_COMPOSITE */
					err("UDESC_STRING: index %d out of range", i);
					goto stall;
				}
				cur += htol_usb_string_descriptor(&dev->strings[i], cur);
				break;
			case UDESC_DEVICE_QUALIFIER:
				dbg("ep%d: UDESC_DEVICE_QUALIFIER", ep);
				/* return qualifier info for "other" (i.e., not current) speed */
				if (dev->speed) {
					cur += usbdev_htol_usb_device_qualifier(
							&bcm_hs_qualifier, cur);
				} else {
					cur += usbdev_htol_usb_device_qualifier(
							&bcm_fs_qualifier, cur);
				}
				break;
#ifdef BCMUSBDEV_COMPOSITE
			case UDESC_DFU:
				dbg("ep%d: UDESC_DFU", ep);
				cur += usbdev_htol_usb_dfu_functional_descriptor(&bcm_dfu, cur);
				break;
			case UDESC_DEBUG:
				dbg("ep%d: UDESC_DEBUG, not supported", ep);
				goto stall;
				break;
#endif /* BCMUSBDEV_COMPOSITE */
#ifdef USB_XDCI
			case UDESC_BOS:
				cur += usbdev_htol_usb_bos_descriptor(&bcm_bos_desc, cur);
				cur += usbdev_htol_usb_2_extension_descritor(&bcm_usb2_ext, cur);
				cur += usbdev_htol_usb_ss_device_capacity_descritor(&bcm_ss_cap,
					cur);
				break;
#endif /* USB_XDCI */
			default:
				err("ep%d: unhandled desc type %d", ep, (dr.wValue >> 8) & 0xff);
				goto stall;
			}
		} else if (dr.bRequest == UR_GET_CONFIG) {
			if (!(p0 = PKTGET(dev->osh, 4, TRUE))) {
				err("ep%d: out of txbufs", ep);
				goto stall;
			}
			cur = PKTDATA(dev->osh, p0);
			*cur++ = (char) dev->confignum;
			dbg("ep%d: get config returning config %d", ep, dev->confignum);
		} else if (dr.bRequest == UR_GET_INTERFACE) {
			if (!(p0 = PKTGET(dev->osh, 4, TRUE))) {
				err("ep%d: out of txbufs", ep);
				goto stall;
			}
			cur = PKTDATA(dev->osh, p0);

			*cur++ = dev->interface;
			dbg("ep%d: get interface returning interface %d", ep, dev->interface);
		} else {
			err("ep%d: unhandled read device bRequest 0x%x, wValue 0x%x", ep,
			       dr.bRequest, dr.wValue);
			goto stall;
		}

		PKTSETLEN(dev->osh, p0, cur - PKTDATA(dev->osh, p0));
	} else if (dr.bmRequestType == UT_WRITE_INTERFACE &&
	         dr.bRequest == UR_SET_INTERFACE) {
		/* dummy handler to satisfy WHQL interface query */
		dev->interface = dr.wValue;
		err("ep%d: set interface set interface %d", ep, dev->interface);
	}

	/* Send encapsulated command */
	else if (dr.bmRequestType == UT_WRITE_VENDOR_INTERFACE) {
		dbg("ep%d: ctrl out %d", ep, dr.bRequest);

		if (dr.bRequest == DL_GO) {
			dbg("DFU GO 0x%x", (uint32)dev->jumpto);
			rdl_indicate_start(dev->jumpto);
		} else if (dr.bRequest == DL_GO_PROTECTED) {
			dbg("DFU GO PROTECTED @ 0x%x", (uint32)dev->jumpto);
			/* watchdog reset after 2 seconds */
			si_watchdog_ms(dev->sih, 2000);
			rdl_indicate_start(dev->jumpto);
		} else if (dr.bRequest == DL_REBOOT) {
			dbg("DL_REBOOT");
			/* watchdog reset after 2 seconds */
			si_watchdog_ms(dev->sih, 2000);
		} else if (dr.bRequest == DL_EXEC) {
			uint32 *addr;
			exec_fn_t exec_fn;

			dbg("DL_EXEC");
			addr = (uint32 *) ((dr.wIndex << 16) | dr.wValue);
			exec_fn = (exec_fn_t)OSL_UNCACHED(addr);
			exec_fn((void *)(dev->sih));
		}
#ifdef USB_XDCI
		else if (dr.bRequest == DL_CHGSPD) {
				if (dev->usb30d) {
					xdci_indicate_speed(dev->ch, dr.wIndex);
				}
		}
#endif /* USB_XDCI */
	}
	/* Get encapsulated response */
	else if (dr.bmRequestType == UT_READ_VENDOR_INTERFACE) {
		dbg("ep%d: ctrl in %d", ep, dr.bRequest);

		if (!(p0 = PKTGET(dev->osh, 256, TRUE))) {
			err("ep%d: out of txbufs", ep);
			goto stall;
		}
		bzero(PKTDATA(dev->osh, p0), 256);

		if (dr.bRequest == DL_CHECK_CRC) {
			/* check the crc of the DL image */
			dbg("DFU CHECK CRC");
		} else if (dr.bRequest == DL_GO) {
			dbg("DFU GO 0x%x", (uint32)dev->jumpto);
#ifdef BCM_SDRBL
			/* In SDR case if image isgnature is not valid,
			 * then dont allow to run the image.
			 */
			if (dev->state == DL_RUNNABLE)
#endif /* BCM_SDRBL */
#ifdef USB_XDCI
				if (dev->usb30d) {
					xdci_indicate_start(dev->ch, dev->jumpto, dr.wIndex);
				} else
#endif
					rdl_indicate_start(dev->jumpto);
		} else if (dr.bRequest == DL_GO_PROTECTED) {
			dbg("DFU GO PROTECTED 0x%x", (uint32)dev->jumpto);
			/* watchdog reset after 2 seconds */
			si_watchdog_ms(dev->sih, 2000);
#ifdef BCM_SDRBL
			/* In SDR case if image isgnature is not valid,
			 * then dont allow to run the image.
			 */
			if (dev->state == DL_RUNNABLE)
#endif /* BCM_SDRBL */
			rdl_indicate_start(dev->jumpto);
		} else if (dr.bRequest == DL_START) {
			dbg("DL_START");
			/* Go to start state */
			usbdev_rdl_dl_state_reset(dev);
		} else if (dr.bRequest == DL_REBOOT) {
			dbg("DL_REBOOT");
			/* watchdog reset after 2 seconds */
			si_watchdog_ms(dev->sih, 2000);
		}

		if (dr.bRequest == DL_GETVER) {
			dbg("DL_GETVER");
			id = (bootrom_id_t*)PKTDATA(dev->osh, p0);
			id->chip = dev->sih->chip;
			id->chiprev = dev->sih->chiprev;
			id->ramsize = hnd_get_memsize();
			id->remapbase = DL_BASE;
			id->boardtype = getintvar(NULL, "boardtype");
			id->boardrev = getintvar(NULL, "boardrev");
			PKTSETLEN(dev->osh, p0, sizeof(bootrom_id_t));
		} else if (dr.bRequest == DL_GETSTATE) {
			dbg("DL_GETSTATE");
			/* return only the state struct */
			rdl_state = (rdl_state_t*)PKTDATA(dev->osh, p0);
			rdl_state->state = dev->state;
			rdl_state->bytes = dev->dlcurrentlen;
			PKTSETLEN(dev->osh, p0, sizeof(rdl_state_t));
		} else if ((dr.bRequest & DL_HWCMD_MASK) == DL_RDHW) {
			hwacc_t *hwacc;
			uint32 *addr;

			dbg("DL_RDHW");

			hwacc = (hwacc_t *)PKTDATA(dev->osh, p0);
			hwacc->addr = (dr.wIndex << 16) | dr.wValue;
			addr = (uint32 *)OSL_UNCACHED(hwacc->addr);
			if (dr.bRequest == DL_RDHW32) {
				hwacc->data = *addr;
				hwacc->len = 4;
			} else if (dr.bRequest == DL_RDHW16) {
				hwacc->data = *(uint16 *)addr;
				hwacc->len = 2;
			} else if (dr.bRequest == DL_RDHW8) {
				hwacc->data = *(uint8 *)addr;
				hwacc->len = 1;
			}
			hwacc->cmd = DL_RDHW;
			PKTSETLEN(dev->osh, p0, sizeof(hwacc_t));
			dbg("DL_RDHW: addr 0x%x => 0x%x", hwacc->addr, hwacc->data);
		} else if (dr.bRequest == DL_GET_NVRAM) {
			uint16 len = 1;
			if (*(dev->var)) {
				const char *var = getvar(NULL, dev->var);
				if (var) {
					len = strlen(var) + 1;
					if (PKTLEN(dev->osh, p0) < len) {
						PKTFREE(dev->osh, p0, TRUE);
						if (!(p0 = PKTGET(dev->osh, len, TRUE))) {
							err("ep%d: out of txbufs", ep);
							goto stall;
						}
						bzero(PKTDATA(dev->osh, p0), len);
					}
					strncpy((char*)PKTDATA(dev->osh, p0), var, len);
				}
			}
			PKTSETLEN(dev->osh, p0, len);
		}
	} else {
		goto stall;
	}

	/* Free request packet and trim return packet */
	if (p0) {
		dr.wLength = MIN(dr.wLength, PKTLEN(dev->osh, p0));
		PKTSETLEN(dev->osh, p0, dr.wLength);
		return p0;
	}

	/* No return packet */
	return p;

stall:
	printf("Stall: type %x req %x val %x idx %x len %x\n", dr.bmRequestType,
	       dr.bRequest, dr.wValue, dr.wIndex, dr.wLength);

	if (p0)
		PKTFREE(dev->osh, p0, TRUE);
	err("ep%d: stall\n", ep);

	*errp = 1;
	return NULL;
}

#if defined(DL_NVRAM) && !defined(BCMTRXV2)
/**
 * points at location where downloaded nvram should be copied to. This location is determined
 * dynamically in the bootloader start up assembly code, but is fixed for a specific bootloader.
 */
extern uchar *_dlvars;
extern uint *_dlvarsz;

static void
usbdev_nvramdl(struct dngl_bus *dev, unsigned char *pdata, uint32 len, unsigned char *dlnvramp)
{
	int nvram_len;

	nvram_len = MIN(dev->nvramsz, (pdata + len) - dlnvramp);

	dbg("len %d, nvram_len %d", len, nvram_len);

	if (nvram_len) {
		bcopy(dlnvramp, _dlvars + dev->nvramoff, nvram_len);
		dev->nvramsz -= nvram_len;
	}
	dev->nvramoff += nvram_len;
	if (dev->nvramsz == 0) {
		int memorygap = (int)((hnd_get_memsize() + (dev->dlbase & MEM_REMAP_MASK)) - 4) -
		                      ((int)_dlvars + dev->nvramoff);
		dbg("memorygap %d; _memsize 0x%x; nvramoff %d; _dlvars 0x%x",
		       memorygap, hnd_get_memsize(), dev->nvramoff, (uint32)_dlvars);
		/* shift the vars up in memory to eliminate memory waste */
		if (memorygap > 0)
			memmove(_dlvars + memorygap, _dlvars, dev->nvramoff);
		dbg("DL_RUNNABLE");
		dev->state = DL_RUNNABLE;
	}
}
#endif /* DL_NVRAM && !BCMTRXV2 */

#ifdef ZLIB
/* decompress data */
static int
copy_decompress(struct dngl_bus *dev, unsigned char *pdata, uint32 len)
{
	int trl_frag, i;
	uint32 uncmp_len = 0, dec_crc;

	dev->d_stream.avail_in = len;
	dev->d_stream.next_in = pdata;
	if (dev->zstatus != Z_STREAM_END)
		dev->zstatus = inflate(&(dev->d_stream), Z_SYNC_FLUSH);
	if (dev->zstatus == Z_STREAM_END) {
		dbg("dev->zstatus == Z_STREAM_END");
		/* If the decompression completed then collate */
		/* the trailer info */
		trl_frag = MIN((GZIP_TRL_LEN - dev->trl_idx),
		               dev->d_stream.avail_in);
		for (i = 0; i < trl_frag; i++)
			dev->trl[dev->trl_idx++] = *dev->d_stream.next_in++;
		if (dev->trl_idx != GZIP_TRL_LEN)
			dbg("trl_idx %d, stream %d", dev->trl_idx,
			    dev->d_stream.avail_in);
		/* Once we have all the trailer info then check the crc */
		if (dev->trl_idx == GZIP_TRL_LEN) {
			/* Get the orig. files len and crc32 value */
			uncmp_len = dev->trl[4];
			uncmp_len |= dev->trl[5]<<8;
			uncmp_len |= dev->trl[6]<<16;
			uncmp_len |= dev->trl[7]<<24;
			/* Do a CRC32 on the uncompressed data */
			printf("chip ver %x, rev %d\n", dev->sih->chip, dev->sih->chiprev);
			dec_crc = hndcrc32((uchar *)dev->dlbase, uncmp_len,
				CRC32_INIT_VALUE);
			if (hndcrc32(dev->trl, 4, dec_crc) != CRC32_GOOD_VALUE) {
					printf("decompression: bad crc check\n");
				dev->state = DL_BAD_CRC;
#if defined(DL_NVRAM) && !defined(BCMTRXV2)
			} else if (dev->nvramsz) {
				if (dev->nvramsz > DL_NVRAM) {
					dev->state = DL_NVRAM_TOOBIG;
					return -1;
				} else {
					uint varsz;
					dbg("accepting nvram download");
					/* size is 32-bit word count */
					varsz = ROUNDUP(dev->nvramsz, 4)/4;
					/* upper 16 bits is ~size */
					*_dlvarsz = (~varsz << 16) | varsz;
					usbdev_nvramdl(dev, pdata, len, dev->d_stream.next_in);
				}
#endif /* DL_NVRAM && !BCMTRXV2 */
			} else
				dev->state = DL_RUNNABLE;
		}
	} else {
		dbg("%d ", dev->zstatus);
	}

	return 0;
}
#else
#define copy_decompress(dev, pdata, len) (-1)
#endif /* ZLIB */

/* uncompressed image: copy the data to dlcurrentbase directly */
static int
copy_direct(struct dngl_bus *dev, unsigned char *pdata, uint32 len)
{
	int downloaded = dev->dlcurrentlen - dev->hdrlen;
#ifdef BCM_SDRBL
	struct trx_header *hdr = (struct trx_header *) (_rambottom
				 - sizeof(struct trx_header) - 4);
#endif /* BCM_SDRBL */

	if ((downloaded + len) >= dev->dlfwlen) {
		bcopy(pdata, dev->dlcurrentbase, dev->dlfwlen - downloaded);
#if defined(DL_NVRAM) && !defined(BCMTRXV2)
		if (dev->nvramsz) {
			if (dev->nvramsz > DL_NVRAM) {
				dev->state = DL_NVRAM_TOOBIG;
				return -1;
			} else {
				uint varsz;
				/* size is 32-bit word count */
				varsz = ROUNDUP(dev->nvramsz, 4)/4;
				/* upper 16 bits is ~size */
				*_dlvarsz = (~varsz << 16) | varsz;
				usbdev_nvramdl(dev, pdata, len, pdata +
				               (dev->dlfwlen - downloaded));
			}
		} else
			dev->state = DL_RUNNABLE;
#else
#ifdef BCM_SDRBL
		if (validate_sdr_image(hdr)) {
			dev->state = DL_RUNNABLE;
		} else {
			dev->state = DL_BAD_CRC;
			dev->jumpto = 0xFFFFFFFF;
		}
#else
		dev->state = DL_RUNNABLE;
#endif /* BCM_SDRBL */
#endif /* DL_NVRAM && !BCMTRXV2 */
	} else
		bcopy(pdata, dev->dlcurrentbase, len);

	return 0;
}

void
usbdev_rx(struct dngl_bus *dev, int ep, void *p)
{
	unsigned char *pdata = PKTDATA(dev->osh, p);
	uint32 len = PKTLEN(dev->osh, p);
	int hdrlen, rv;

	trace("ep %d %d", ep, len);

	/* handle control OUT data */
	if (ep == 0) {
		unsigned int cmd = *((unsigned int *) pdata);

		if (len < sizeof(unsigned int)) {
			err("ep%d: pkt tossed len %d", ep, len);
			goto toss;
		}

		dbg("ep%d: Ctrl-out cmd %d; packet len %d", ep, cmd, len);
		switch (cmd) {
		case DL_WRHW:
			if (len == sizeof(hwacc_t)) {
				hwacc_t *hwacc;
				uint32 *addr;

				hwacc = (hwacc_t *)pdata;
				dbg("DL_WRHW: addr 0x%x <= 0x%x; len %d", hwacc->addr,
				    hwacc->data, hwacc->len);
				addr = (uint32 *)OSL_UNCACHED(hwacc->addr);
				switch (hwacc->len) {
				case 1:
					*((uint8 *)addr) = (uint8)(hwacc->data & 0xff);
					break;
				case 2:
					*((uint16 *)addr) = (uint16)(hwacc->data & 0xffff);
					break;
				case 4:
					*addr = hwacc->data;
					break;
				default:
					err("ep%d: invalid WRHW cmd len: %d", ep, hwacc->len);
					break;
				}
			} else {
				/* toss other pkts from the ctrl e/p */
				err("ep%d: WRHW invalid pkt len %d", ep, len);
			}
			break;
		 case DL_WRHW_BLK:
			{
				hwacc_blk_t *hwacc;
				uint32 *addr;

				hwacc = (hwacc_blk_t *)pdata;
				dbg("DL_WRHW_BLK: addr 0x%x <= 0x%x...; len %d", hwacc->addr,
				     hwacc->data[0], hwacc->len);
				addr = (uint32 *)OSL_UNCACHED(hwacc->addr);
				memcpy(addr, hwacc->data, hwacc->len);
			}
			break;

		case DL_GET_NVRAM:
			{
				/* save the nvram query string
				 * response will be sent in usbdev_setup
				 */
				nvparam_t *nv = (nvparam_t *)pdata;
				uint8 len = strlen(nv->var);

				if (len <= QUERY_STRING_MAX)
					strncpy(dev->var, nv->var, len+1);

				else
					bzero(dev->var, QUERY_STRING_MAX);
			}
			break;

		default:
			err("ep%d: invalid cmd: %d", ep, cmd);
			break;
		}
		goto toss;
	}

	switch (dev->state) {

	case DL_WAITING:
		/* Process trx/gzip header */
		hdrlen = check_headers(dev, pdata);
		dbg("DL_WAITING: process headers");
#ifdef ZLIB
		if (dev->comp_image)
			init_zlib_state(dev);
#endif
		if (hdrlen < 0) {
			err("Error checking headers");
			dev->state = DL_BAD_HDR;
		} else {
			dev->state = DL_READY;
			/* Skip the headers */
			pdata += hdrlen;
			dev->dlcurrentlen += hdrlen;
			len -= hdrlen;
			dev->hdrlen = hdrlen;
#ifdef ZLIB
			/* Set the decompression base addr */
			dev->d_stream.next_out = dev->dlcurrentbase;
#endif
		}
		/* deliberate fall through */
	case DL_READY:
		/* if decompression is done simply download
		 * the rest of the file (if its padded).
		 */
		dbg("DL_READY: got %d bytes", len);
		if (dev->state == DL_READY) {
			if (!dev->nvramoff) {
				if (dev->comp_image)
					rv = copy_decompress(dev, pdata, len);
				else
					rv = copy_direct(dev, pdata, len);
				if (rv < 0)
					break;
			}
#if defined(DL_NVRAM) && !defined(BCMTRXV2)
			else
				usbdev_nvramdl(dev, pdata, len, pdata);
#endif /* DL_NVRAM && !BCMTRXV2 */
		}
	/* deliberate fall through */
	default:
		/* update status of dl data */
		dev->dlcurrentbase += len;
		dev->dlcurrentlen += len;
		if (dev->dlcurrentlen == dev->dllen) {
			if (dev->state != DL_RUNNABLE) {
				err("inflate incomplete");
				if (dev->state != DL_NVRAM_TOOBIG)
					dev->state = DL_BAD_CRC;
			}
		}
	}

	dbg(" pkt len 0x%x loaded 0x%x", PKTLEN(dev->osh, p), dev->dlcurrentlen);
toss:
	PKTFREE(dev->osh, p, FALSE);
	trace("done, state %d", dev->state);
}

/* check_headers:
 *		parse and verify the trx and gzip headers.
 *
 *		RETURN: the length of the header scetion
 *
 */

static int check_headers(struct dngl_bus *dev, unsigned char *headers)
{
	struct trx_header *trx;
	unsigned char *pdata = headers;
	uint32	trxhdr_size;

	/* Extract trx header and init data */
	trx = (struct trx_header *)headers;
	trxhdr_size = SIZEOF_TRX(trx);

	if (trx->magic == TRX_MAGIC) {
		/* get download address from trx header */
		dev->dlbase = DL_BASE;
		dev->dlcurrentbase = (uchar*)dev->dlbase;
		/* get the firmware len */
		dev->dlfwlen = trx->offsets[TRX_OFFSETS_DLFWLEN_IDX];

		dbg("trx flags=0x%04x, version=0x%04x",
		    trx->flag_version & 0xFFFF, trx->flag_version >> 16);
#ifndef BCM_SDRBL
#ifdef DATA_START
		if ((trx->flag_version & TRX_UNCOMP_IMAGE) &&
		    dev->dlfwlen > (DATA_START & ~(MEM_REMAP_MASK))) {
			dev->state = DL_IMAGE_TOOBIG;
			printf("rdl: image len %d greater than max image size %d\n",
			    dev->dlfwlen, (DATA_START & ~(MEM_REMAP_MASK)));
			return -1;
		}
#endif /* DATA_START */
#endif /* ! BCM_SDRBL */
		/* get start address from trx header */
		dev->jumpto = trx->offsets[TRX_OFFSETS_JUMPTO_IDX];
		dev->dlcrc = trx->crc32;
		dev->dllen = trx->len;
		dev->state = DL_READY;

#ifndef BCMTRXV2
		dev->nvramsz = trx->offsets[TRX_OFFSETS_NVM_LEN_IDX];
#else /* BCMTRXV2 */
		/* With trxv2 header, we will copy the TRXV2 header
		 * to top of RAM. TRX header should provide the FW
		 * with required size/offset information for NVRAM.
		 * No plans to copy nvram to top of RAM, FW on bootup
		 * is expected to detect the TRX header from top of RAM
		 * and thereby figureout the offsets of nvram region.
		 * In simple terms DL_NVRAM mechanism will be discontinued
		 * with TRXV2 header.
		 * In new image format, we download entire trx file,
		 * as is. Bootloader does not handle nvram specially,
		 * It is upto the downloaded FW to detect and
		 * treat various regions from TRX header placed at top
		 * of RAM.
		 */
		if (ISTRX_V1(trx)) {
			dev->nvramsz = trx->offsets[TRX_OFFSETS_NVM_LEN_IDX];
		} else {
			dev->dlfwlen += trx->offsets[TRX_OFFSETS_NVM_LEN_IDX]
					+ trx->offsets[TRX_OFFSETS_DSG_LEN_IDX]
					+ trx->offsets[TRX_OFFSETS_CFG_LEN_IDX];
			/* Copy the TRX header to top of RAM */
			memcpy((void *)(_rambottom - trxhdr_size - 4),
				trx, trxhdr_size);
		}
#endif /* BCMTRXV2 */
		dbg("usbdev_rx, dlbase 0x%x", dev->dlbase);
		dbg("usbdev_rx, jumpto 0x%x", dev->jumpto);
		dbg("usbdev_rx, dllen 0x%x", dev->dllen);
		dbg("usbdev_rx, dlcrc 0x%x", dev->dlcrc);
	} else {
		dev->state = DL_BAD_HDR;
		err("rdl: trx bad hdr");
		return -1;
	}

	headers += trxhdr_size;

	if (!(trx->flag_version & TRX_UNCOMP_IMAGE)) {
#ifdef ZLIB
		uint32 len;
		int method; /* method byte */
		int flags;  /* flags byte */
		uint8 gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */

		dev->comp_image = 1;
		/* Extract the gzip header info */
		if ((*headers++ != gz_magic[0]) || (*headers++ != gz_magic[1]))
			return -1;

		method = (int) *headers++;
		flags = (int) *headers++;

		if (method != Z_DEFLATED || (flags & RESERVED) != 0)
			return -1;

		/* Discard time, xflags and OS code: */
		for (len = 0; len < 6; len++)
			headers++;

		if ((flags & EXTRA_FIELD) != 0) { /* skip the extra field */
			len = (uint32) *headers++;
			len += ((uint32)*headers++) << 8;
			/* len is garbage if EOF but the loop below will quit anyway */
			while (len-- != 0)
				headers++;
		}
		if ((flags & ORIG_NAME) != 0) { /* skip the original file name */
			while (*headers++ && (*headers != 0));
		}
		if ((flags & COMMENT) != 0) {   /* skip the .gz file comment */
			while (*headers++ && (*headers != 0));
		}
		if ((flags & HEAD_CRC) != 0) {  /* skip the header crc */
			for (len = 0; len < 2; len++)
				headers++;
		}
		headers++;
#else
		err("USB downloader doesn't support compressed images");
		return -1;
#endif /* ZLIB */
	}

	/* return the length of the headers */
	return (int)((int)headers - (int)pdata);
}
