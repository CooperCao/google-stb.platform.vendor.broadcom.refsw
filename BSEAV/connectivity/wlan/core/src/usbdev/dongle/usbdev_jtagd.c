/*
 * Epidiag JTAG Master device
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
#include <typedefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <osl.h>
#include <usbstd.h>
#include <usbrdl.h>
#include <siutils.h>
#include <hndjtagdefs.h>
#include <hndsoc.h>
#include <usbdev.h>
#include <usbdev_dbg.h>
#include <rte_dev.h>
#include <rte_isr.h>
#include <sbchipc.h>

/* OS specifics */
#define INV_ICACHE()	blast_icache()
#define FLUSH_DCACHE()	blast_dcache()

struct dngl_bus {
	osl_t *osh;			/* Driver handle */
	struct usbdev_chip *ch;		/* Chip handle */
	si_t *sih;			/* si handle */
	const usb_interface_descriptor_t *control_interface;
	const usb_interface_descriptor_t *data_interface;
	usb_device_descriptor_t device;
	usb_string_descriptor_t strings[4];
	usb_config_descriptor_t config;
	usb_config_descriptor_t other_config;
	int confignum;
	int interface;
	bool hispeed;
	uint intr_ep;
	uint ep_bi;
	uint ep_bo;
	int state;
	/* Jtag master state */
	chipcregs_t	*jtagm_regs;
	uint32		jtagm_clkd;
	uint32		jtagm_ctrl;
	uint32		jtagm_delay;
	uint32		jtagm_retries;
	uint32		jtagm_dretries;
	uint32		jtagm_disgpio;
	uint32		jtagm_ir_lvbase;
	uint32		jtagm_ir_lv_ro;
	uint		drsz;
	uint		irsz;
	bool		jtag_bigend;
	uint		jtag_mode;
	uint32		ate_write_addr;
	uint32		mips_cmd;
};

/* Keystone needs a different base, we could go further and make
 * shift and mask settable too
 */
#undef	LV_REG_IR

#define IR_FMT_LV_32	0
#define IR_FMT_LV_38	1

static uint lv_IR_fmt = IR_FMT_LV_38;

typedef struct dngl_bus dngl_bus_t;

#define JTAGM_INTTAP(d)	(((d)->jtagm_ctrl & JCTRL_EXT_EN) == 0)
#define JTAGM_FCLK(d)	((d)->jtagm_ctrl & JCTRL_FORCE_CLK)
#define JTAGM_ON(d)	((d)->jtagm_ctrl & JCTRL_EN)

#define JTAG_RETRIES	10000
#define DMA_RETRIES	2500000

/* Instruction & data register sizes */
#define DEF_DATA_SIZE	32		/* Default DR size */
#define DEF_INST_SIZE	32		/* Default IR size */

/* mode of target access */
#define JTAG_MIPS	1
#define JTAG_CHIPC	2
#define JTAG_LV		3
#define JTAG_LVATE      4

#define	TS_DEFAULT	0
#define	TS_PAUSE	1
#define	TS_RTI		2

/* Local prototypes */
static void jtagm_on(dngl_bus_t *dev);
static void jtagm_off(dngl_bus_t *dev);
static void jtagm_reset(dngl_bus_t *dev);
static uint32 jtagm_scmd(dngl_bus_t *dev, uint32 ir, uint irsz, uint32 dr, uint drsz);
static int read_jtag(dngl_bus_t *dev, uint32 addr, uint32 *read_data, uint size);
static int write_jtag(dngl_bus_t *dev, uint32 addr, uint32 write_data, uint size);
static int jtag_scan(dngl_bus_t *dev, uint irsz, uint drsz, int ts, uint32 *in, uint32 *out);

static uint32
LV_REG_IR(uint32 reg, uint32 base)
{
	if (lv_IR_fmt == IR_FMT_LV_32)
		return (base | (((reg) << LV_REG_SHIFT) & LV_REG_MASK));
	else if (lv_IR_fmt == IR_FMT_LV_38)
		return (base | (((reg) << LV_38_REG_SHIFT) & LV_38_REG_MASK));
	return 0;
}

/* none 38 bits IR: The theory is that upper bits are don't cares, but we'll
 * sign-extend the ir value to preserve -2 (id) and -1 (bypass).
 * 38 bits IR: all bits set to 1
 */
static uint32
LV_IR_2(uint32 ir, uint32 irsz)
{
	if (lv_IR_fmt == IR_FMT_LV_32)
		return ((ir & 0x80000000) ? -1 : 0) &
			((irsz == 64) ? -1 : ((1 << (irsz - JRBITS)) - 1));
	else
		return (-1) & ((1 << (irsz - JRBITS)) - 1);
}

/*
 * Initial jtag dongle: 1 interface:
 *   interrupt, bulk-IN, bulk-OUT
 * Later we'll add a second interface for UART.
 */

#define JTAGD_BULK_IN_EP	1
#define JTAGD_BULK_OUT_EP	2

static const usb_device_descriptor_t jtagd_device = {
	bLength: USB_DEVICE_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_DEVICE,
	bcdUSB: 0x0200,
	bDeviceClass: UICLASS_VENDOR,
	bDeviceSubClass: 0,
	bDeviceProtocol: 0,
	bMaxPacketSize: 64,
	idVendor: 0x0a5c,
	idProduct: 0x4a44,
	bcdDevice: 0x0001,
	iManufacturer: 1,
	iProduct: 2,
	iSerialNumber: 3,
	bNumConfigurations: 1
};

#define JTAGD_CONFIGLEN (USB_CONFIG_DESCRIPTOR_SIZE + \
	USB_INTERFACE_DESCRIPTOR_SIZE + \
	(USB_ENDPOINT_DESCRIPTOR_SIZE * 3))

static const usb_config_descriptor_t jtagd_config = {
	bLength: USB_CONFIG_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_CONFIG,
	wTotalLength: JTAGD_CONFIGLEN,
	bNumInterface: 1,
	bConfigurationValue: 1,
	iConfiguration: 0,
	bmAttributes: UC_BUS_POWERED,
	bMaxPower: 200 / UC_POWER_FACTOR
};

static const usb_config_descriptor_t jtagd_other_config = {
	bLength: USB_CONFIG_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_OTHER_SPEED_CONFIGURATION,
	wTotalLength: JTAGD_CONFIGLEN,
	bNumInterface: 1,
	bConfigurationValue: 1,
	iConfiguration: 0,
	bmAttributes: UC_BUS_POWERED,
	bMaxPower: 200 / UC_POWER_FACTOR
};

static const usb_interface_descriptor_t jtagd_interface = {
	bLength: USB_INTERFACE_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_INTERFACE,
	bInterfaceNumber: 0,
	bAlternateSetting: 0,
	bNumEndpoints: 3,
	bInterfaceClass: UICLASS_VENDOR,
	bInterfaceSubClass: UISUBCLASS_ABSTRACT_CONTROL_MODEL,
	bInterfaceProtocol: 0xff,
	iInterface: 0
};

static const usb_endpoint_descriptor_t jtagd_fs_endpoints[] = {
	{
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_IN | 1,
		bmAttributes: UE_INTERRUPT,
		wMaxPacketSize: 16,
		bInterval: 1
	},
	{
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_IN | 2,
		bmAttributes: UE_BULK,
		wMaxPacketSize: 64,
		bInterval: 0
	},
	{
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_OUT | 3,
		bmAttributes: UE_BULK,
		wMaxPacketSize: 64,
		bInterval: 0
	}
};

static const usb_endpoint_descriptor_t jtagd_hs_endpoints[] = {
	{
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_IN | 1,
		bmAttributes: UE_INTERRUPT,
		wMaxPacketSize: 16,
		bInterval: 4
	},
	{
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_IN | 2,
		bmAttributes: UE_BULK,
		wMaxPacketSize: 512,
		bInterval: 0
	},
	{
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_OUT | 3,
		bmAttributes: UE_BULK,
		wMaxPacketSize: 512,
		bInterval: 1
	}
};

static const usb_device_qualifier_t jtagd_fs_qualifier = {
	bLength: USB_DEVICE_QUALIFIER_SIZE,
	bDescriptorType: UDESC_DEVICE_QUALIFIER,
	bcdUSB: 0x0110,
	bDeviceClass: UICLASS_VENDOR,
	bDeviceSubClass: 0,
	bDeviceProtocol: 0,
	bMaxPacketSize0: 64,
	bNumConfigurations: 1,
	bReserved: 0
};

static const usb_device_qualifier_t jtagd_hs_qualifier = {
	bLength: USB_DEVICE_QUALIFIER_SIZE,
	bDescriptorType: UDESC_DEVICE_QUALIFIER,
	bcdUSB: 0x0200,
	bDeviceClass: UICLASS_VENDOR,
	bDeviceSubClass: 0,
	bDeviceProtocol: 0,
	bMaxPacketSize0: 64,
	bNumConfigurations: 1,
	bReserved: 0
};


/* UTF16 (not NULL terminated) strings */
static const usb_string_descriptor_t jtagd_strings[] = {
	{
		bLength: 0x04,
		bDescriptorType: UDESC_STRING,
		bString: { 0x409 }
	},
	{
		/* USB spec: bLength is bString size + 2 */
		bLength: sizeof(L"Broadcom"),
		bDescriptorType: UDESC_STRING,
		bString: { 'B', 'r', 'o', 'a', 'd', 'c', 'o', 'm' }
	},
	{
		/* USB spec: bLength is bString size + 2 */
		bLength: sizeof(L"EpiDiag JTAG Master Dongle"),
		bDescriptorType: UDESC_STRING,
		bString: {
			'E', 'p', 'i', 'D', 'i', 'a', 'g', ' ',
			'J', 'T', 'A', 'G', ' ',
			'M', 'a', 's', 't', 'e', 'r', ' ',
			'D', 'o', 'n', 'g', 'l', 'e'
		}
	},
	{
		/* USB spec: bLength is bString size + 2 */
		bLength: sizeof(L"000000000001"),
		bDescriptorType: UDESC_STRING,
		bString: { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1' }
	}
};

dngl_bus_t *
usbdev_attach(osl_t *osh, struct usbdev_chip *ch, si_t *sih)
{
	dngl_bus_t *dev;
	chipcregs_t *cc;
	uint curridx;
	int i;

	trace("JTAGD");

	/* Not interested if there is no jtag master */
	if ((sih->cccaps & CC_CAP_JTAGP) == 0) {
		err("No jtag master");
		return (NULL);
	}

	if (!(dev = MALLOC(osh, sizeof(dngl_bus_t)))) {
		err("out of memory");
		return NULL;
	}

	bzero(dev, sizeof(dngl_bus_t));
	dev->osh = osh;
	dev->ch  = ch;
	dev->sih = sih;

	/* Initialize descriptors */
	dev->control_interface = &jtagd_interface;
	dev->data_interface = &jtagd_interface;
	bcopy(&jtagd_config, &dev->config, sizeof(dev->config));
	bcopy(&jtagd_other_config, &dev->other_config, sizeof(dev->other_config));
	bcopy(&jtagd_device, &dev->device, sizeof(usb_device_descriptor_t));

	for (i = 0; i < ARRAYSIZE(dev->strings); i++)
		bcopy(&jtagd_strings[i], &dev->strings[i], sizeof(usb_string_descriptor_t));

	dev->state = -1;

	/* Initialize jtag master */
	curridx = si_coreidx(sih);
	dev->jtagm_regs = cc = (chipcregs_t *)si_setcoreidx(sih, SI_CC_IDX);
	(void)si_setcoreidx(sih, curridx);
	dev->jtagm_clkd = (R_REG(osh, &cc->clkdiv) & ~CLKD_JTAG) >> CLKD_JTAG_SHIFT;
	dev->jtagm_ctrl = JCTRL_EXT_EN;
	dev->jtagm_retries = JTAG_RETRIES;
	dev->jtagm_dretries = DMA_RETRIES;
	dev->jtagm_ir_lvbase = LV_BASE;
	dev->jtagm_ir_lv_ro = LV_RO;
	dev->drsz = DEF_DATA_SIZE;
	dev->irsz = DEF_INST_SIZE;
	dev->jtag_bigend = FALSE;
	dev->jtag_mode = JTAG_LV;
	dev->ate_write_addr = (uint32)(-1);
	dev->mips_cmd = 0;

	jtagm_off(dev);

	trace("done");
	return dev;
}

void
usbdev_detach(dngl_bus_t *dev)
{
	trace("");

	jtagm_off(dev);

	MFREE(dev->osh, dev, sizeof(dngl_bus_t));
	/* ensure cache state is in sync after code copy */
	FLUSH_DCACHE();
	INV_ICACHE();

	trace("done");
}

void *
usbdev_setup(dngl_bus_t *dev, int ep, void *p, int *errp, int *dir)
{
	usb_device_request_t dr;
	jtagd_id_t *id;
	void *p0 = NULL;
	int i;
	bool hispeed;

	trace("JTAGD");
	ASSERT(ep == 0);

	ltoh_usb_device_request(PKTDATA(dev->osh, p), &dr);

	*dir = dr.bmRequestType & UT_READ;
	/* Get standard descriptor */
	if (dr.bmRequestType == UT_READ_DEVICE) {
		uchar *cur = NULL;

		if (dr.bRequest == UR_GET_DESCRIPTOR) {
			uchar request = (dr.wValue >> 8) & 0xff;

			if (!(p0 = PKTGET(dev->osh, 256, TRUE))) {
				err("ep%d: out of txbufs", ep);
				goto stall;
			}
			cur = PKTDATA(dev->osh, p0);

			switch (request) {
			case UDESC_DEVICE:
				dbg("ep%d: UDESC_DEVICE", ep);
				cur += htol_usb_device_descriptor(&dev->device, cur);
				break;
			case UDESC_CONFIG:
			case UDESC_OTHER_SPEED_CONFIGURATION:
				dbg("ep%d: %s", ep, request == UDESC_CONFIG ? "UDESC_CONFIG" :
					"UDESC_OTHER_SPEED_CONFIGURATION");
				if (request == UDESC_CONFIG) {
					cur += htol_usb_config_descriptor(&dev->config, cur);
					hispeed = dev->hispeed;
				} else {
					cur += htol_usb_config_descriptor(&dev->other_config, cur);
					hispeed = !dev->hispeed;
				}

				if (request == UDESC_CONFIG || ch_usb20(dev->ch)) {
					const usb_endpoint_descriptor_t *data_endpoints;
					if (hispeed)
						data_endpoints = jtagd_hs_endpoints;
					else
						data_endpoints = jtagd_fs_endpoints;
					cur += htol_usb_interface_descriptor(dev->data_interface,
						cur);
					for (i = 0; i < dev->data_interface->bNumEndpoints; i++) {
						cur += htol_usb_endpoint_descriptor(
							&data_endpoints[i], cur);
						dbg("added %d endpoint; cur = %d", i,
						       (uint) cur - (uint) PKTDATA(dev->osh, p0));
					}
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
				if (i == dev->data_interface->bInterfaceNumber)
					cur += htol_usb_interface_descriptor(
						dev->data_interface, cur);
				else {
					err("UDESC_INTERFACE query failed");
					goto stall;
				}
				break;
			case UDESC_STRING:
				i = dr.wValue & 0xff;
				dbg("ep%d: UDESC_STRING %d langid 0x%x", ep, i, dr.wIndex);
				if (i > 3) {
					err("UDESC_STRING: index out of range: i");
					goto stall;
				}
				cur += htol_usb_string_descriptor(&dev->strings[i], cur);
				break;
			case UDESC_DEVICE_QUALIFIER:
				dbg("ep%d: UDESC_DEVICE_QUALIFIER", ep);
				/* return qualifier info for "other" (i.e., not current) speed */
				if (dev->hispeed) {
					cur += htol_usb_device_qualifier(&jtagd_fs_qualifier, cur);
				} else {
					cur += htol_usb_device_qualifier(&jtagd_hs_qualifier, cur);
				}
				break;
			default:
				err("ep%d: unhandled desc type %d\n", ep,
				       (dr.wValue >> 8) & 0xff);
				goto stall;
			}
		} else if (dr.bRequest == UR_GET_CONFIG) {
			if (!(p0 = PKTGET(dev->osh, 4, TRUE))) {
				err("ep%d: out of txbufs\n", ep);
				goto stall;
			}
			cur = PKTDATA(dev->osh, p0);
			*cur++ = (char) dev->confignum;
			dbg("ep%d: get config returning config %d\n", ep, dev->confignum);
		} else if (dr.bRequest == UR_GET_INTERFACE) {
			if (!(p0 = PKTGET(dev->osh, 4, TRUE))) {
				err("ep%d: out of txbufs\n", ep);
				goto stall;
			}
			cur = PKTDATA(dev->osh, p0);

			*cur++ = dev->interface;
			dbg("ep%d: get interface returning interface %d\n", ep, dev->interface);
		} else {
			err("ep%d: unhandled read device bRequest 0x%x, wValue 0x%x\n", ep,
			       dr.bRequest, dr.wValue);
			goto stall;
		}

		PKTSETLEN(dev->osh, p0, cur - PKTDATA(dev->osh, p0));
	} else if ((dr.bmRequestType == UT_WRITE_DEVICE) &&
	           (dr.bRequest == UR_SET_INTERFACE)) {
		/* dummy handler to satisfy WHQL interface query */
		dev->interface = dr.wValue;
		err("ep%d: set interface set interface %d", ep, dev->interface);
	} else if (dr.bmRequestType == UT_WRITE_VENDOR_INTERFACE) {
		/* Send encapsulated command */
		dbg("ep%d: ctrl out %d", ep, dr.bRequest);

		if (dr.bRequest == DL_REBOOT) {
			dbg("DL_REBOOT");
			/* watchdog reset after 2 seconds */
			si_watchdog_ms(dev->sih, 2000);
		} else if ((dr.bRequest & DL_HWCMD_MASK) == DL_WRHW) {
			dbg("DL_WRHW");
		} else if (dr.bRequest == DL_WRJT) {
			dbg("DL_WRJT");
		} else if (dr.bRequest == DL_WRRJT) {
			dbg("DL_WRRJT");
		}
	} else if (dr.bmRequestType == UT_READ_VENDOR_INTERFACE) {
		/* Get encapsulated response */
		dbg("ep%d: ctrl in %d", ep, dr.bRequest);

		if (dr.bRequest == DL_REBOOT) {
			dbg("DL_REBOOT");
			/* watchdog reset after 2 seconds */
			si_watchdog_ms(dev->sih, 2000);
		}

		if (!(p0 = PKTGET(dev->osh, 256, TRUE))) {
			err("ep%d: out of txbufs", ep);
			goto stall;
		}
		bzero(PKTDATA(dev->osh, p0), 256);

		if (dr.bRequest == DL_GETVER) {
			dbg("DL_GETVER");
			id = (jtagd_id_t *)PKTDATA(dev->osh, p0);
			id->chip = dev->sih->chip;
			id->chiprev = dev->sih->chiprev;
			id->ccrev = dev->sih->ccrev;
			id->siclock = si_clock(dev->sih);
			PKTSETLEN(dev->osh, p0, sizeof(jtagd_id_t));
		} else if ((dr.bRequest & DL_HWCMD_MASK) == DL_RDHW) {
			hwacc_t *hwacc;
			uint32 *addr;

			dbg("DL_RDHW");

			hwacc = (hwacc_t *)PKTDATA(dev->osh, p0);
			hwacc->cmd = DL_RDHW;
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
			PKTSETLEN(dev->osh, p0, sizeof(hwacc_t));
			dbg("DL_RDHW: addr 0x%x => 0x%x", hwacc->addr, hwacc->data);
		} else if (dr.bRequest == DL_JTCONF) {
			jtagconf_t *jtconf;

			dbg("DL_JTCONF");

			jtconf = (jtagconf_t *)PKTDATA(dev->osh, p0);
			jtconf->clkd = dev->jtagm_clkd;
			jtconf->disgpio = dev->jtagm_disgpio;
			jtconf->irsz = dev->irsz;
			jtconf->drsz = dev->drsz;
			jtconf->bigend = dev->jtag_bigend;
			jtconf->mode = dev->jtag_mode;
			jtconf->delay = dev->jtagm_delay;
			jtconf->retries = dev->jtagm_retries;
			jtconf->dretries = dev->jtagm_dretries;
			jtconf->ctrl = dev->jtagm_ctrl;
			jtconf->ir_lvbase = dev->jtagm_ir_lvbase;
			PKTSETLEN(dev->osh, p0, sizeof(jtagconf_t));
			dbg("DL_JTCONF: clkd/irsz/drsz/mode/crtl/lvbase %d/%d/%d/%d/0x%x/0x%x",
			    jtconf->clkd, jtconf->irsz, jtconf->drsz, jtconf->mode,
			    jtconf->ctrl, jtconf->ir_lvbase);
		} else if (dr.bRequest == DL_JTON) {
			hwacc_t *hwacc;

			dbg("DL_JTON");

			jtagm_on(dev);

			hwacc = (hwacc_t *)PKTDATA(dev->osh, p0);
			hwacc->cmd = DL_JTON;
			hwacc->addr = 0;
			hwacc->data = dev->jtagm_ctrl;
			PKTSETLEN(dev->osh, p0, sizeof(hwacc_t));
			dbg("DL_JTON: crtl 0x%x", hwacc->data);
		} else if (dr.bRequest == DL_JTOFF) {
			hwacc_t *hwacc;

			dbg("DL_JTOFF");

			jtagm_off(dev);

			hwacc = (hwacc_t *)PKTDATA(dev->osh, p0);
			hwacc->cmd = DL_JTOFF;
			hwacc->addr = 0;
			hwacc->data = dev->jtagm_ctrl;
			PKTSETLEN(dev->osh, p0, sizeof(hwacc_t));
			dbg("DL_JTON: crtl 0x%x", hwacc->data);
		} else if (dr.bRequest == DL_JTRST) {
			hwacc_t *hwacc;

			dbg("DL_JTRST");

			jtagm_reset(dev);

			hwacc = (hwacc_t *)PKTDATA(dev->osh, p0);
			hwacc->cmd = DL_JTRST;
			hwacc->addr = 0;
			hwacc->data = dev->jtagm_ctrl;
			PKTSETLEN(dev->osh, p0, sizeof(hwacc_t));
			dbg("DL_JTON: crtl 0x%x", hwacc->data);
		} else if (dr.bRequest == DL_RDRJT) {
			hwacc_t *hwacc;
			uint32 ir, zeros = 0x00000000;

			dbg("DL_RDRJT");

			hwacc = (hwacc_t *)PKTDATA(dev->osh, p0);
			hwacc->cmd = DL_RDRJT;
			hwacc->addr = ir = (dr.wIndex << 16) | dr.wValue;
			hwacc->data = jtagm_scmd(dev, ir, dev->irsz, zeros, dev->drsz);
			PKTSETLEN(dev->osh, p0, sizeof(hwacc_t));
			dbg("DL_RDRJT: ir 0x%x => 0x%x", ir, hwacc->data);
		} else if ((dr.bRequest & DL_HWCMD_MASK) == DL_RDJT) {
			hwacc_t *hwacc;
			uint32 addr;
			uint size;

			dbg("DL_RDJT");

			hwacc = (hwacc_t *)PKTDATA(dev->osh, p0);
			hwacc->cmd = DL_RDJT;
			hwacc->addr = addr = (dr.wIndex << 16) | dr.wValue;
			size = 4 - (dr.bRequest & ~DL_HWCMD_MASK);
			if (read_jtag(dev, addr, &hwacc->data, size))
				hwacc->cmd |= DL_JTERROR;
			hwacc->len = size;
			PKTSETLEN(dev->osh, p0, sizeof(hwacc_t));
			dbg("DL_RDJT: addr 0x%x => 0x%x", hwacc->addr, hwacc->data);
		}
	} else {
		err("ep%d: unhandled reqType %d req %d index %d", ep, dr.bmRequestType,
		    dr.bRequest, dr.wIndex);
		goto stall;
	}

	/* Trim return packet */
	if (p0) {
		dr.wLength = MIN(dr.wLength, PKTLEN(dev->osh, p0));
		PKTSETLEN(dev->osh, p0, dr.wLength);
		return p0;
	}

	/* No return packet */
	return p;

stall:
	if (p0)
		PKTFREE(dev->osh, p0, TRUE);
	err("ep%d: stall\n", ep);
	return NULL;
}

void
usbdev_rx(dngl_bus_t *dev, int ep, void *p)
{
	uint32 len = PKTLEN(dev->osh, p);
	unsigned char *pdata = PKTDATA(dev->osh, p);
	uint32 cmd = *((unsigned int *) pdata);
	void *p0 = NULL;

	trace("ep %d %d", ep, len);

	if (len < sizeof(unsigned int)) {
		err("ep%d: pkt tossed len %d", ep, len);
		goto toss;
	}

	if ((ep != 0) && (ep != dev->ep_bo)) {
		err("Unexpected ep%d, pkt tossed", ep);
		goto toss;
	}

	dbg("ep%d: %s-out cmd 0x%x; packet len %d\n", ep, (ep == 0) ? "Ctrl" : "Bulk", cmd, len);
	switch (cmd) {
	case DL_WRHW: {
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
		break;
	}
	case DL_JTCONF: {
		chipcregs_t *cc = dev->jtagm_regs;
		jtagconf_t *jtconf;
		uint32 clkd, tmp;

		if (len != sizeof(jtagconf_t)) {
			err("ep%d: JTCONF invalid pkt len %d", ep, len);
			goto toss;
		}
		jtconf = (jtagconf_t *)pdata;
		clkd = jtconf->clkd & 0xe;
		dev->jtagm_disgpio = jtconf->disgpio;
		dev->irsz = jtconf->irsz;
		dev->drsz = jtconf->drsz;
		dev->jtag_bigend = jtconf->bigend;
		dev->jtag_mode = jtconf->mode;
		dev->jtagm_delay = jtconf->delay;
		dev->jtagm_retries = jtconf->retries;
		dev->jtagm_dretries = jtconf->dretries;
		dev->jtagm_ctrl = jtconf->ctrl;

		if (dev->irsz == 38) {
			lv_IR_fmt = IR_FMT_LV_38;
			dev->jtagm_ir_lvbase = LV_38_BASE;
			dev->jtagm_ir_lv_ro = LV_38_RO;
		} else {
			lv_IR_fmt = IR_FMT_LV_32;
			dev->jtagm_ir_lvbase = LV_BASE;
			dev->jtagm_ir_lv_ro = LV_RO;
		}

		dbg("DL_JTCONF: clkd/irsz/drsz/mode/crtl/lvbase %d/%d/%d/%d/0x%x/0x%x",
		    clkd, jtconf->irsz, jtconf->drsz, jtconf->mode, jtconf->ctrl,
		    jtconf->ir_lvbase);
		if (clkd != 0) {
			dev->jtagm_clkd = clkd;
			tmp = R_REG(dev->osh, &cc->clkdiv);
			W_REG(dev->osh, &cc->clkdiv, (tmp & ~CLKD_JTAG) |
			      (clkd << CLKD_JTAG_SHIFT));
		}
		W_REG(dev->osh, &cc->jtagctrl, dev->jtagm_ctrl);
		break;
	}
	case DL_WRRJT: {
		hwacc_t *hwacc;
		uint32 ir, dr;

		hwacc = (hwacc_t *)pdata;
		ir = hwacc->addr;
		dr = hwacc->data;
		dbg("DL_WRHW: ir 0x%x <= 0x%x", ir, dr);
		jtagm_scmd(dev, ir, dev->irsz, dr, dev->drsz);
		break;
	}
	case DL_WRJT: {
		hwacc_t *hwacc;
		uint32 addr, data;
		uint size;

		hwacc = (hwacc_t *)pdata;
		addr = hwacc->addr;
		data = hwacc->data;
		size = hwacc->len;
		dbg("DL_WRJT: addr 0x%x <= 0x%x; len %d", addr, data, size);
		write_jtag(dev, addr, data, size);
		break;
	}
	case DL_SCJT: {
		scjt_t *scin, *scout;
		uint irsz, drsz;
		int ts;

		if (len != sizeof(scjt_t)) {
			err("ep%d: SCJT invalid pkt len %d", ep, len);
			goto toss;
		}
		if (!(p0 = PKTGET(dev->osh, sizeof(scjt_t), TRUE))) {
			err("ep%d: out of txbufs", ep);
			goto toss;
		}
		scout = (scjt_t *)PKTDATA(dev->osh, p0);

		scin = (scjt_t *)pdata;
		scout->cmd = DL_SCJT;
		scout->irsz = irsz = scin->irsz;
		scout->drsz = drsz = scin->drsz;
		scout->ts = ts = scin->ts;
		dbg("DL_SCJT: irsz %d, drsz %d, ts %d", irsz, drsz, ts);
		jtag_scan(dev, irsz, drsz, ts, scin->data, scout->data);
		break;
	}
	default:
		err("ep%d: invalid cmd: %d", ep, cmd);
		break;
	}

	if (p0) {
		int iep;

		if (ep == dev->ep_bo)
			iep = dev->ep_bi;
		else
			iep = ep;

		ch_tx(dev->ch, ep, p0);
	}

toss:
	PKTFREE(dev->osh, p, FALSE);
	trace("done, state %d", dev->state);
}

void
usbdev_txstop(dngl_bus_t *dev, int ep)
{
	trace("");
}

void
usbdev_txstart(dngl_bus_t *dev, int ep)
{
	trace("");
}

uint
usbdev_mps(dngl_bus_t *dev)
{
	trace("");
	return dev->device.bMaxPacketSize;
}

uint
usbdev_setcfg(dngl_bus_t *dev, int config)
{
	const usb_endpoint_descriptor_t *data_endpoints;

	trace("");
	ASSERT(config == dev->config.bConfigurationValue);
	/* Attach configuration endpoints */
	dev->confignum = config;
	if (dev->hispeed)
		data_endpoints = jtagd_hs_endpoints;
	else
		data_endpoints = jtagd_fs_endpoints;
	if (config == 1) {
		/* Attach configuration endpoints */
		if (!dev->ep_bi)
			dev->ep_bi = ep_attach(
				dev->ch, &data_endpoints[JTAGD_BULK_IN_EP],
				dev->config.bConfigurationValue,
				dev->data_interface->bInterfaceNumber,
				dev->data_interface->bAlternateSetting);
		if (!dev->ep_bo)
			dev->ep_bo = ep_attach(
				dev->ch, &data_endpoints[JTAGD_BULK_OUT_EP],
				dev->config.bConfigurationValue,
				dev->data_interface->bInterfaceNumber,
				dev->data_interface->bAlternateSetting);
	}

	trace("done ep_bi %d, ep_bi %d", dev->ep_bi, dev->ep_bo);
	return dev->config.bmAttributes;
}

void
usbdev_speed(dngl_bus_t *dev, int high)
{
	trace("%s", high ? "high" : "full");
	dev->hispeed = high;
}

void
usbdev_reset(dngl_bus_t *dev)
{
	trace("");

	/* Detach configuration endpoints */
	if (dev->ep_bi) {
		ep_detach(dev->ch, dev->ep_bi);
		dev->ep_bi = 0;
	}
	if (dev->ep_bo) {
		ep_detach(dev->ch, dev->ep_bo);
		dev->ep_bo = 0;
	}
}

void
usbdev_sof(dngl_bus_t *dev)
{
	trace("");
}

void
usbdev_suspend(dngl_bus_t *dev)
{
	trace("");
}

void
usbdev_resume(dngl_bus_t *dev)
{
	trace("");
}

static void
jtagm_on(dngl_bus_t *dev)
{
	chipcregs_t *cc = dev->jtagm_regs;

	dev->jtagm_ctrl |= JCTRL_EN;
	W_REG(dev->osh, &cc->jtagctrl, dev->jtagm_ctrl);
	if (dev->jtagm_disgpio) {
		/* Just disable output, the board should have a PD */
		W_REG(dev->osh, &cc->gpioouten, 0);
	}
}

static void
jtagm_off(dngl_bus_t *dev)
{
	chipcregs_t *cc = dev->jtagm_regs;
	uint disgpio = dev->jtagm_disgpio;

	dev->jtagm_ctrl &= ~JCTRL_EN;
	W_REG(dev->osh, &cc->jtagctrl, 0);
	if (disgpio) {
		W_REG(dev->osh, &cc->gpioout, disgpio);
		W_REG(dev->osh, &cc->gpioouten, disgpio);
	}
}

static void
jtagm_reset(dngl_bus_t *dev)
{
	chipcregs_t *cc = dev->jtagm_regs;

	if (dev->mips_cmd != 0) {
		/* 6362 hack: clear mips_cmd */
		dev->mips_cmd = 0;
		err("6362: clearing mips_cmd\n");
	}
	W_REG(dev->osh, &cc->jtagcmd, (JCMD_START | JCMD_ACC_RESET));
	while ((R_REG(dev->osh, &cc->jtagcmd) & JCMD_BUSY) == JCMD_BUSY)
		;
}

static uint32
jtagm_wait(dngl_bus_t *dev, bool readdr)
{
	chipcregs_t *cc = dev->jtagm_regs;
	uint retries = dev->jtagm_retries;
	uint i;

	i = 0;
	while (((R_REG(dev->osh, &cc->jtagcmd) & JCMD_BUSY) == JCMD_BUSY) &&
	       (i < retries)) {
		i++;
	}

	if (readdr && (i < retries))
		return R_REG(dev->osh, &cc->jtagdr);
	else
		return 0xffffffff;
}

static uint32
jtagm_scmd(dngl_bus_t *dev, uint32 ir, uint irsz, uint32 dr, uint drsz)
{
	chipcregs_t *cc = dev->jtagm_regs;
	uint delay = dev->jtagm_delay;
	uint32 data;
	int is62 = 0;

	if (((data = R_REG(dev->osh, &cc->jtagcmd)) & JCMD_BUSY) == JCMD_BUSY) {
		err(" jcmd (0x%x) busy is set before scmd", data);
		return 0xffffffff;
	}

	if (delay)
		OSL_DELAY(delay);

	if ((irsz == 15) && (drsz == 32)) {
		if ((ir == MIPS_CTRL) && (dr == (EJ_PREN | EJ_BREAK))) {
			/* 6362 hack: if we are writing PrEn and EjBreak, keep writing
			 * them plus PrAcc so the CPU hangs forever.
			 */
			dev-> mips_cmd = EJ_PRACC | EJ_PREN | EJ_BREAK;
			err("6362: ir = %d, dr = 0x%x, setting mips_cmd to 0x%x\n",
			    ir, dr, dev->mips_cmd);
		}
		/* 6362 3-daisy chained mips, put two of them in bypass */
		is62 = 1;
		ir <<= 10;
		ir |= (0x1f << 5) | 0x1f;
	}

	W_REG(dev->osh, &cc->jtagir, ir);
	if (irsz > JRBITS) {
		uint32 ir2;

		W_REG(dev->osh, &cc->jtagcmd,
		      (JCMD_START | JCMD_ACC_PIR | ((JRBITS - 1) << JCMD_IRW_SHIFT)));
		jtagm_wait(dev, FALSE);
		/* The theory is that upper bits are don't cares, but we'll
		 * sign-extend the ir value to preserve -2 (id) and -1 (bypass).
		 */
		ir2 = LV_IR_2(ir, irsz);
		W_REG(dev->osh, &cc->jtagir, ir2);
		irsz -= JRBITS;
	}

	if (is62) {
		uint32 d2 = dr >> 30;

		W_REG(dev->osh, &cc->jtagdr, dr << 2);
		W_REG(dev->osh, &cc->jtagcmd,
		      (JCMD_START | JCMD_ACC_IRPDR |
		       ((15 - 1) << JCMD_IRW_SHIFT) | (JRBITS - 1)));
		data = jtagm_wait(dev, TRUE);
		W_REG(dev->osh, &cc->jtagdr, d2);
		W_REG(dev->osh, &cc->jtagcmd, (JCMD_START | JCMD_ACC_DR | 1));
		d2 = jtagm_wait(dev, TRUE);
		data = (data >> 2) | (d2 << 30);
	} else {
		W_REG(dev->osh, &cc->jtagdr, dr);
		W_REG(dev->osh, &cc->jtagcmd, (JCMD_START | JCMD_ACC_IRDR |
		                                    ((irsz - 1) << JCMD_IRW_SHIFT) | (drsz - 1)));

		data = jtagm_wait(dev, TRUE);
	}
	return data;
}

static int
read_jtag(dngl_bus_t *dev, uint32 addr, uint32 *read_data, uint size)
{
	uint32	zeros = 0x00000000;
	uint32	ir_addr, ir_data, ir_data_ro, ir_ctrl, ir_ctrl_ro;
	uint32	be, cmd, mask, shift;
	uint32	ir2, data, dataout;
	uint32	ir_lvbase = dev->jtagm_ir_lvbase;
	uint32	ir_lv_ro = dev->jtagm_ir_lv_ro;
	uint32	mips_cmd = dev->mips_cmd;
	uint	irsz = dev->irsz, drsz = dev->drsz;
	uint	jtag_mode = dev->jtag_mode;
	uint	dretries = dev->jtagm_dretries;
	bool	jtag_bigend = dev->jtag_bigend;
	int	i, error = 0;


	shift = addr & 3;
	switch (size) {
	case 1:
		be = CCC_SZ1 << shift;
		cmd = (jtag_mode == JTAG_MIPS) ?
		        (mips_cmd | DMA_ACC | DMA_READ | DMA_SZ1) :
		        (CCC_START | CCC_READ | be);
		if (jtag_bigend)
			shift = 3 - shift;
		mask = 0x000000ff << (shift * 8);
		break;

	case 2:
		be = CCC_SZ2 << shift;
		cmd = (jtag_mode == JTAG_MIPS) ?
		        (mips_cmd | DMA_ACC | DMA_READ | DMA_SZ2) :
		        (CCC_START | CCC_READ | be);
		if (jtag_bigend)
			shift = 2 - shift;
		mask = 0x0000ffff << (shift * 8);
		if (addr & 1)
			error = -1;
		break;

	case 4:
		be = CCC_SZ4;
		cmd = (jtag_mode == JTAG_MIPS) ?
		        (mips_cmd | DMA_ACC | DMA_READ | DMA_SZ4) :
		        (CCC_START | CCC_READ | CCC_SZ4);
		mask = 0xffffffff;
		if (addr & 3)
			error = -1;
		break;

	default:
		/* Bad size */
		error = -1;
	}

	if (error)
		goto out;

	if (jtag_mode == JTAG_MIPS) {
		ir_addr = MIPS_ADDR;
		ir_data = MIPS_DATA;
		ir_ctrl = MIPS_CTRL;
	} else if (jtag_mode == JTAG_LV) {
		ir_addr = LV_REG_IR(LV_ADDR, ir_lvbase);
		ir_data = LV_REG_IR(LV_DATA, ir_lvbase);
		ir_data_ro = LV_REG_IR(LV_DATA, ir_lvbase) | ir_lv_ro;
		ir_ctrl = LV_REG_IR(LV_CTRL, ir_lvbase);
		ir_ctrl_ro = LV_REG_IR(LV_CTRL, ir_lvbase) | ir_lv_ro;
	} else if (jtag_mode == JTAG_LVATE) {
		/* IR for ATERead */
		ir_addr = LV_REG_IR(LV_ATEREAD, ir_lvbase);
		/* IR for ATEReadData */
		ir_data = LV_REG_IR(LV_ATEREADDATA, ir_lvbase);
		ir_data_ro = LV_REG_IR(LV_ATEREADDATA, ir_lvbase) | ir_lv_ro;
	} else {
		ir_addr = CHIPC_ADDR;
		ir_data = CHIPC_DATA;
		ir_data_ro = CHIPC_DATA | CHIPC_RO;
		ir_ctrl = CHIPC_CTRL;
		ir_ctrl_ro = CHIPC_CTRL | CHIPC_RO;
	}

	/* Do we need to reset? */

	if (jtag_mode == JTAG_MIPS) {
		/* set the dmaacc bit */
		dataout = jtagm_scmd(dev, ir_ctrl, irsz, cmd, drsz);

		/* scan in the address */
		dataout = jtagm_scmd(dev, ir_addr, irsz, addr, drsz);

		/* set the dma start bit, size bit and read/write bit */
		dataout = jtagm_scmd(dev, ir_ctrl, irsz, cmd | DMA_START, drsz);

		/* wait for dma to complete */
		i = 0;
		do {
			dataout = jtagm_scmd(dev, ir_ctrl, irsz, cmd, drsz);
		} while ((dataout & DMA_START) && (++i < dretries));

		if ((dataout & DMA_ERROR) || (i >= dretries)) {
			data = 0xffffffff;
		} else {
			/* read out the data */
			data = jtagm_scmd(dev, ir_data, irsz, zeros, drsz);
		}

		/* clear the dmaacc bit */
		dataout = jtagm_scmd(dev, ir_ctrl, irsz, mips_cmd, drsz);
	} else if (jtag_mode == JTAG_LVATE) {
		chipcregs_t *cc = dev->jtagm_regs;

		/* Use "ATE" backplane access in the LV tap. */

		/* Issue the ATERead until the Reject bit is zero */
		i = 0;
		do {
			W_REG(dev->osh, &cc->jtagir, ir_addr);
			if (irsz > JRBITS) {
				W_REG(dev->osh, &cc->jtagcmd,
				      (JCMD_START | JCMD_ACC_PIR |
				       ((JRBITS - 1) << JCMD_IRW_SHIFT)));
				jtagm_wait(dev, FALSE);
				ir2 = LV_IR_2(ir_addr, irsz);
				W_REG(dev->osh, &cc->jtagir, ir2);
				irsz -= JRBITS;
			}

			/* First 32 bits of DR: address */
			W_REG(dev->osh, &cc->jtagdr, addr);
			W_REG(dev->osh, &cc->jtagcmd,
			      (JCMD_START | JCMD_ACC_IRPDR |
			       ((irsz - 1) << JCMD_IRW_SHIFT) | (JRBITS - 1)));
			/* Don't care about first 32 bits scanned out .. */
			jtagm_wait(dev, FALSE);
			/* Next (and last) 5 bits of DR: byte enables and Reject bit */
			W_REG(dev->osh, &cc->jtagdr, be);
			W_REG(dev->osh, &cc->jtagcmd,
			      (JCMD_START | JCMD_ACC_DR | (5 - 1)));
			dataout = jtagm_wait(dev, TRUE);

		} while ((dataout & ATE_REJECT) && (++i < dretries));

		if (i < dretries) {
			irsz = dev->irsz;

			/* Now read the data until Busy bit is zero */
			i = 0;
			do {
				W_REG(dev->osh, &cc->jtagir, ir_data_ro);
				if (irsz > JRBITS) {
					W_REG(dev->osh, &cc->jtagcmd,
					      (JCMD_START | JCMD_ACC_PIR |
					       ((JRBITS - 1) << JCMD_IRW_SHIFT)));
					jtagm_wait(dev, FALSE);
					ir2 = LV_IR_2(ir_data_ro, irsz);
					W_REG(dev->osh, &cc->jtagir, ir2);
					irsz -= JRBITS;
				}
				/* Scanned in DR contents is don't care since we are
				 * using r/o address
				 */
				W_REG(dev->osh, &cc->jtagcmd,
				      (JCMD_START | JCMD_ACC_IRPDR |
				       ((irsz - 1) << JCMD_IRW_SHIFT) | (JRBITS - 1)));
				/* First 32 bits are the data */
				data = jtagm_wait(dev, TRUE);
				/* Next (and last) 2 bits of DR: Busy and error  */
				W_REG(dev->osh, &cc->jtagcmd,
				      (JCMD_START | JCMD_ACC_DR | (2 - 1)));
				dataout = jtagm_wait(dev, TRUE);
				if (dataout & ATE_RERROR) {
					err("ATEReadData.error set\n");
					data = 0xffffffff;
					error = -2;
					break;
				}
			} while ((dataout & ATE_BUSY) && (++i < dretries));
			if (i >= dretries) {
				err("ATEReadData.busy still on after %d tries\n", dretries);
				data = 0xffffffff;
				error = -3;
			}
		} else {
			err("ATERead.reject still on after %d tries \n", dretries);
			data = 0xffffffff;
			error = -3;
		}
	} else {
		/* scan in the address */
		dataout = jtagm_scmd(dev, ir_addr, irsz, addr, drsz);

		/* set the dma start bit, size bit and read/write bit */
		dataout = jtagm_scmd(dev, ir_ctrl, irsz, (cmd << CCC_WR_SHIFT), drsz);

		/* wait for dma to complete */
		i = 0;
		do {
			dataout = jtagm_scmd(dev, ir_ctrl_ro, irsz, zeros, drsz);
		} while ((dataout & CCC_START) && (++i < dretries));

		if (i < dretries) {
			if (dataout & CCC_ERROR) {
				err("chipc_ctrl.ccc_error set\n");
				data = 0xffffffff;
			} else {
				/* read out the data */
				data = jtagm_scmd(dev, ir_data_ro, irsz, zeros, drsz);
			}
		} else {
			err("chipc_ctrl.ccc_start still on after %d tries \n", dretries);
			data = 0xffffffff;
		}
	}

	/* retrieve the data from the correct bytes */
	data &= mask;
	data >>= shift * 8;

	*read_data = data;

out:
	return error;
}

static int
write_jtag(dngl_bus_t *dev, uint32 addr, uint32 write_data, uint size)
{
	uint32	zeros = 0x00000000;
	uint32	ir_addr, ir_data, ir_data_ro, ir_ctrl, ir_ctrl_ro;
	uint32	ir2, be = 0, cmd, shift;
	uint32	dataout;
	uint32	ir_lvbase = dev->jtagm_ir_lvbase;
	uint32	ir_lv_ro = dev->jtagm_ir_lv_ro;
	uint32	mips_cmd = dev->mips_cmd;
	uint	irsz = dev->irsz, drsz = dev->drsz;
	uint	jtag_mode = dev->jtag_mode;
	uint	dretries = dev->jtagm_dretries;
	bool	jtag_bigend = dev->jtag_bigend;
	int	i, error = 0, ateerr = 0;


	/* move data in to correct byte location, depends on
	 * size, addr and endianess
	 */
	switch (size) {
	case 1:
		if (jtag_bigend)
			shift = ~addr & 3;
		else
			shift = addr & 3;
		write_data <<= (shift * 8);
		if (jtag_mode == JTAG_MIPS)
			cmd = mips_cmd | DMA_ACC | DMA_SZ1;
		else {
			be = CCC_SZ1 << shift;
			cmd = CCC_START | be;
		}
		break;

	case 2:
		/* check alignment */
		if (addr & 1)
			error = -1;

		if (jtag_bigend)
			shift = (addr + 2) & 3;
		else
			shift = addr & 3;

		write_data <<= (shift * 8);

		if (jtag_mode == JTAG_MIPS)
			cmd = mips_cmd | DMA_ACC | DMA_SZ2;
		else {
			be = CCC_SZ2 << shift;
			cmd = CCC_START | be;
		}
		break;

	case 4:
		/* check alignment */
		if (addr & 3)
			error = -1;

		be = CCC_SZ4;
		cmd = (jtag_mode == JTAG_MIPS) ?
		        (mips_cmd | DMA_ACC | DMA_SZ4) :
		        (CCC_START | CCC_SZ4);
		break;

	default:
		/* Bad size */
		error = -1;
	}

	if (error)
		goto out;

	if (jtag_mode == JTAG_MIPS) {
		ir_addr = MIPS_ADDR;
		ir_data = MIPS_DATA;
		ir_ctrl = MIPS_CTRL;
	} else if (jtag_mode == JTAG_LV) {
		ir_addr = LV_REG_IR(LV_ADDR, ir_lvbase);
		ir_data = LV_REG_IR(LV_DATA, ir_lvbase);
		ir_data_ro = LV_REG_IR(LV_DATA, ir_lvbase) | ir_lv_ro;
		ir_ctrl = LV_REG_IR(LV_CTRL, ir_lvbase);
		ir_ctrl_ro = LV_REG_IR(LV_CTRL, ir_lvbase) | ir_lv_ro;
	} else {
		ir_addr = CHIPC_ADDR;
		ir_data = CHIPC_DATA;
		ir_data_ro = CHIPC_DATA | CHIPC_RO;
		ir_ctrl = CHIPC_CTRL;
		ir_ctrl_ro = CHIPC_CTRL | CHIPC_RO;
	}

	/* Do we need to reset? */

	if (jtag_mode == JTAG_MIPS) {
		/* set the dmaacc bit */
		dataout = jtagm_scmd(dev, ir_ctrl, irsz, cmd, drsz);

		/* scan in the address */
		dataout = jtagm_scmd(dev, ir_addr, irsz, addr, drsz);

		/* write the data */
		dataout = jtagm_scmd(dev, ir_data, irsz, write_data, drsz);

		/* set the dma start bit, size bit and read/write bit */
		dataout = jtagm_scmd(dev, ir_ctrl, irsz, cmd | DMA_START, drsz);

		/* wait for dma to complete */
		i = 0;
		do {
			dataout = jtagm_scmd(dev, ir_ctrl, irsz, cmd, drsz);
		} while ((dataout & DMA_START) && (++i < dretries));

		if ((dataout & DMA_ERROR) || (i >= dretries))
			error = -1;

		/* clear the dmaacc bit */
		dataout = jtagm_scmd(dev, ir_ctrl, irsz, mips_cmd, drsz);
	} else if (jtag_mode == JTAG_LVATE) {
		chipcregs_t *cc = dev->jtagm_regs;
		bool sameaddr;

		sameaddr = dev->ate_write_addr == (addr & ~3);
		if (sameaddr) {
			ir_addr = LV_REG_IR(LV_ATEWRITENEXT, ir_lvbase);
			cmd = (JCMD_START | JCMD_ACC_IRPDR |
			       ((irsz - 1) << JCMD_IRW_SHIFT) | (JRBITS - 1));
		} else {
			ir_addr = LV_REG_IR(LV_ATEWRITE, ir_lvbase);
			cmd = (JCMD_START | JCMD_ACC_PDR | (JRBITS - 1));
			dev->ate_write_addr = (addr & ~3);
		}

		/* Issue the ATEWrite until the Reject bit is zero */
		i = 0;
		do {
			W_REG(dev->osh, &cc->jtagir, ir_addr);
			if (irsz > JRBITS) {
				W_REG(dev->osh, &cc->jtagcmd,
				      (JCMD_START | JCMD_ACC_PIR |
				       ((JRBITS - 1) << JCMD_IRW_SHIFT)));
				jtagm_wait(dev, FALSE);
				ir2 = LV_IR_2(ir_addr, irsz);
				W_REG(dev->osh, &cc->jtagir, ir2);
				irsz -= JRBITS;
			}

			if (!sameaddr) {
				/* First 32 bits of DR: address */
				W_REG(dev->osh, &cc->jtagdr, addr);
				W_REG(dev->osh, &cc->jtagcmd,
				      (JCMD_START | JCMD_ACC_IRPDR |
				       ((irsz - 1) << JCMD_IRW_SHIFT) | (JRBITS - 1)));
				/* Don't care about first 32 bits scanned out */
				jtagm_wait(dev, FALSE);
			}

			/* Next 32 bits of DR: data */
			W_REG(dev->osh, &cc->jtagdr, write_data);
			W_REG(dev->osh, &cc->jtagcmd, cmd);
			/* Don't care about next 32 bits scanned out */
			jtagm_wait(dev, FALSE);
			/* Next (and last) 6 bits of DR: byte enables, Reject and Error bits */
			W_REG(dev->osh, &cc->jtagdr, be);
			W_REG(dev->osh, &cc->jtagcmd,
			      (JCMD_START | JCMD_ACC_DR | (6 - 1)));
			dataout = jtagm_wait(dev, TRUE);
			if (dataout & ATE_WERROR)
				ateerr = 1;
		} while ((dataout & ATE_REJECT) && (++i < dretries));
		if (i < dretries) {
			if (ateerr) {
				err("ATEWrite.error set: PREVIOUS write had an error\n");
				error = -2;
			}
		} else {
			err("ATEWrite.reject still on after %d tries\n", dretries);
			error = -3;
		}
	} else {
		/* scan in the address */
		dataout = jtagm_scmd(dev, ir_addr, irsz, addr, drsz);

		/* write the data */
		dataout = jtagm_scmd(dev, ir_data, irsz, write_data, drsz);

		/* set the dma start bit, size bit and read/write bit */
		dataout = jtagm_scmd(dev, ir_ctrl, irsz, (cmd << CCC_WR_SHIFT), drsz);

		/* wait for dma to complete */
		i = 0;
		do {
			dataout = jtagm_scmd(dev, ir_ctrl_ro, irsz, zeros, drsz);
		} while ((dataout & CCC_START) && (++i < dretries));

		if (i >= dretries) {
			err("chipc_ctrl.ccc_start still on after %d tries \n", dretries);
			error = -1;
		}
	}

out:
	return error;
}

static int
jtag_scan(dngl_bus_t *dev, uint irsz, uint drsz, int ts, uint32 *in, uint32 *out)
{
	chipcregs_t *cc = dev->jtagm_regs;
	uint irw, drw, lirsz, ldrsz, w;
	uint cmd, acc_ir, acc_irdr, acc_dr, acc_pdr, acc_irpdr;
	uint32 *ii, *oo;


	irw = (irsz + JRBITS - 1) / JRBITS;
	drw = (drsz + JRBITS - 1) / JRBITS;

	if (ts == TS_RTI) {
		acc_ir = JCMD_ACC_IR;
		acc_irdr = JCMD_ACC_IRDR_I;
		acc_dr = JCMD_ACC_DR_I;
		acc_pdr = JCMD_ACC_PDR;
		acc_irpdr = JCMD_ACC_IRPDR;
	} else if (ts == TS_PAUSE) {
		acc_ir = JCMD_ACC_PIR;
		acc_irdr = JCMD_ACC_IRPDR;
		acc_dr = JCMD_ACC_PDR;
		acc_pdr = JCMD_ACC_PDR;
		acc_irpdr = JCMD_ACC_IRPDR;
	} else {
		acc_ir = JCMD_ACC_IR;
		acc_irdr = JCMD_ACC_IRDR;
		acc_dr = JCMD_ACC_DR;
		acc_pdr = JCMD_ACC_PDR;
		acc_irpdr = JCMD_ACC_IRPDR;
	}

	ii = in;
	oo = out;
	if (irsz == 0) {
		/* scan in the first (or only) DR word with a dr-only command */
		W_REG(dev->osh, &cc->jtagdr, *ii++);
		cmd = (drw == 1) ?
		        (JCMD_START | acc_dr | (drsz - 1)) :
		        (JCMD_START | acc_pdr | (JRBITS - 1));
		W_REG(dev->osh, &cc->jtagcmd, cmd);
	} else {
		lirsz = irsz;

		/* Use Partial IR for all but last IR word */
		for (w = 1; w < irw; w++) {
			W_REG(dev->osh, &cc->jtagir, *ii++);
			W_REG(dev->osh, &cc->jtagcmd,
			      (JCMD_START | JCMD_ACC_PIR | ((JRBITS - 1) << JCMD_IRW_SHIFT)));
			jtagm_wait(dev, FALSE);
			*oo++ = R_REG(dev->osh, &cc->jtagir);
			lirsz -= JRBITS;
		}
		W_REG(dev->osh, &cc->jtagir, *ii++);
		if (drsz == 0) {
			/* If drsz is 0, do an IR-only scan and that's it */
			W_REG(dev->osh, &cc->jtagcmd, (JCMD_START | acc_ir |
			                                    ((lirsz - 1) << JCMD_IRW_SHIFT)));
			jtagm_wait(dev, FALSE);
			*oo++ = R_REG(dev->osh, &cc->jtagir);
			return 0;
		}
		/* Now scan in the last IR word and the first (or only) DR word */
		W_REG(dev->osh, &cc->jtagdr, *ii++);
		cmd = (drw == 1) ?
		        (JCMD_START | acc_irdr | ((irsz - 1) << JCMD_IRW_SHIFT) | (drsz - 1)) :
		        (JCMD_START | acc_irpdr | ((irsz - 1) << JCMD_IRW_SHIFT) | (JRBITS - 1));
		W_REG(dev->osh, &cc->jtagcmd, cmd);
	}

	/* Now scan out the DR and scan in the rest of the DR words */
	w = 1;
	oo = out;
	ldrsz = drsz;
	while (TRUE) {
		*oo++ = jtagm_wait(dev, TRUE);
		if (++w > drw) break;
		ldrsz -= JRBITS;
		W_REG(dev->osh, &cc->jtagdr, *ii++);
		cmd =  (w == drw) ? (JCMD_START | acc_dr | (ldrsz - 1)) :
		        (JCMD_START | acc_pdr | (JRBITS - 1));
		W_REG(dev->osh, &cc->jtagcmd, cmd);
	}

	return 0;
}

bool
usbdev_oobresume(struct dngl_bus *bus, bool active)
{
	return FALSE;
}

/* RTE driver support */

typedef struct jtagd {
	void *drv;		/* Back pointer */
	uint unit;		/* Device index */
	uint device;		/* Chip ID */
	uint bus;		/* bus between device and processor */
	void *regs;		/* Chip registers */
	void *ch;		/* Chip handle */
	struct jtagd *next;
} jtagd_t;

static jtagd_t *jtagd_list = NULL;

#if defined(BCMDBG) || defined(BCMDBG_ERR)
int usbdev_msglevel = USBDEV_ERROR | USBDEV_INFORM;
#endif /* BCMDBG */

/* Number of devices found */
static int found = 0;

static int jtagd_close(hnd_dev_t *dev);

static void
jtagd_isr(void *cbdata)
{
	hnd_dev_t *dev = (hnd_dev_t *)cbdata;
	jtagd_t *jtagd = dev->softc;

	if (jtagd->ch) {
		if (ch_dispatch(jtagd->ch))
			ch_dpc(jtagd->ch);
	}
}

static int
jtagd_write(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *buffer)
{
#ifdef BCMDBG
	jtagd_t *jtagd = dev->softc;
#endif
	trace("jtagd%dwrite", jtagd->unit);

	return 0;
}

static int
jtagd_open(hnd_dev_t *dev)
{
	jtagd_t *jtagd = dev->softc;

	trace("jtagd%d", jtagd->unit);


#ifdef BCMUSB_NODISCONNECT
	/* Initialize chip */
	ch_init(jtagd->ch, FALSE);
#else
	/* delay so that handshake at end of bootloader protocol completes */
	OSL_DELAY(10000);
	/* Initialize chip */
	ch_init(jtagd->ch, TRUE);
#endif

	trace("jtagd exit%d", jtagd->unit);
	return 0;
}

static int
jtagd_close(hnd_dev_t *dev)
{
	jtagd_t *jtagd = dev->softc;

	/* Free chip state */
	if (jtagd->ch) {
		ch_detach(jtagd->ch, FALSE);
		jtagd->ch = NULL;
	}

	return 0;
}

static int
jtagd_ioctl(hnd_dev_t *dev, uint32 cmd, void *buffer, int len, int *used, int *needed, int set)
{
#ifdef BCMDBG
	jtagd_t *jtagd = dev->softc;
#endif
	trace("jtagd%d", jtagd->unit);

	return 0;
}

static void *
jtagd_probe(hnd_dev_t *drv, void *regs, uint bus, uint16 device, uint coreid, uint unit)
{
	jtagd_t *jtagd;
	osl_t *osh;

	if (!ch_match(VENDOR_BROADCOM, device))
		return NULL;
	if (found >= 8) {
		err("too many units");
		return NULL;
	}

	osh = osl_attach(drv);

	if (!(jtagd = (jtagd_t *)MALLOC(osh, sizeof(jtagd_t)))) {
		err("malloc failed");
		return NULL;
	}

	bzero(jtagd, sizeof(jtagd_t));
	jtagd->unit = found;
	jtagd->drv = drv;
	jtagd->device = device;
	jtagd->regs = regs;
	jtagd->bus = bus;

	printf("jtagd%d: EpiDiag JTAG Master Dongle\n", found);

	/* Add us to the global linked list */
	jtagd->next = jtagd_list;
	jtagd_list = jtagd;

	/* Allocate chip state */
	if (!(jtagd->ch = ch_attach(jtagd, VENDOR_BROADCOM, jtagd->device,
	                          osh, jtagd->regs, jtagd->bus))) {
		MFREE(osh, jtagd, sizeof(jtagd_t));
		return NULL;
	}
#ifndef RTE_POLL
	if (hnd_isr_register(0, coreid, unit, jtagd_isr, drv)) {
		ch_detach(jtagd->ch, TRUE);
		MFREE(osh, jtagd, sizeof(jtagd_t));
		return NULL;
	}
#endif	/* !RTE_POLL */
	found++;

	return ((void*)jtagd);
}

static void
jtagd_poll(hnd_dev_t *dev)
{
	jtagd_isr(dev);
}

static hnd_dev_ops_t jtagd_funcs = {
	probe:		jtagd_probe,
	open:		jtagd_open,
	close:		jtagd_close,
	xmit:		jtagd_write,
	ioctl:		jtagd_ioctl,
	poll:		jtagd_poll
};

hnd_dev_t bcmjtagd = {
	name:		"jtagd",
	ops:		&jtagd_funcs
};
