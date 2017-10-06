/*
 * Broadcom BDC USB device
 *
 * note: although the term 'cdc' is used throughout this file, we only support 'bdc'.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id $
 */

#define __need_wchar_t
#include <stddef.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <osl.h>
#include <siutils.h>
#include <wlioctl.h>
#include <usb.h>
#include <sbusbd.h>
#include <dngl_bus.h>
#include <dngl_api.h>
#include <usbdev.h>
#include <usbdev_dbg.h>
#include <usbrdl.h>
#include <rte_dev.h>
#include <rte_trap.h>

#define DISCONNECT_FROM_BUS

#define WL_BULK_IN_NUM(usbdesc) (usbdesc & 0x30)	/* dongle -> host */
#define WL_BULK_OUT_NUM(usbdesc) (usbdesc & 0xc0)	/* host -> dongle */
#define BT_BULK_OUT_NUM(usbdesc) (usbdesc & 0x8000)
#define BT_BULK_IN_NUM(usbdesc) (usbdesc & 0x08000)


#define ACTION_UNDEFINED		0
#define ACTION_REMOTE_WAKEUP	1
#define ACTION_OOB_RECONNECT	2
#define ACTION_TRIGGER_TRAP		3
#define ACTION_INFINITE_LOOP	4
#define ACTION_OOB_DISCONNECT	5

#ifdef BCMUSB_NODISCONNECT
#ifdef USB_XDCI
#define XDCI_LPM_ENA
#endif
#endif /* BCMUSB_NODISCONNECT */

static usb_device_descriptor_t * get_bcm_device(void);

#include <usbdev_common.h>

extern hnd_dev_t usbdev_dev;

#ifdef BCMUSBDEV_COMPOSITE
static usb_interface_association_descriptor_t* get_bcm_iad(void);
static void usbdev_set_ep_defaults(struct dngl_bus *bus, int int_num);
static void usbdev_set_iso_alt_ep_maxsize(usb_endpoint_descriptor_t* d, int alt_setting);
static usb_interface_descriptor_t* get_bcm_interface(void);
static usb_device_qualifier_t* get_bcm_hs_qualifier(void);
static usb_device_qualifier_t* get_bcm_fs_qualifier(void);
#endif /* BCMUSBDEV_COMPOSITE */

static void usbdev_hostrdy(uint32 stat, void *arg);

#ifdef DISCONNECT_FROM_BUS
static void usbdev_disconnect(struct dngl_bus *bus);
#endif

static void usbdev_gptimer(dngl_timer_t *t);
static void usbdev_wakehost(struct dngl_bus *bus, bool pmu_wake, bool called_from_trap);
static void usbdev_disablehostwake(struct dngl_bus *bus);
static void usbdev_devrdytimer(dngl_timer_t *t);
static void usbdev_generictimer(dngl_timer_t *t);
static void usbdev_reconnect(struct dngl_bus *bus);
static void usbdev_pktdump(uint8 *pdata, uint32 len);
static inline int usbd_tx(struct dngl_bus *bus, int ep, void *p);
static void usbdev_oobreconnect(struct dngl_bus *bus, bool hostrdy);

#define POSTBOOT_ID	0xA123   /* magic to indicate dongle's boot up */

#ifndef BCMUSBDEV_COMPOSITE
#define INT_WLAN  0       /* default WLAN interface */
#else
#define INT_WLAN   (bus->int_wlan) /* WLAN interface on Composite Device */
#endif /*  BCMUSBDEV_COMPOSITE */

/** ROM accessor to avoid struct in shdat */
static usb_config_descriptor_t *
get_bcm_other_config(void)
{
	return &bcm_other_config;
}

#ifdef BCMUSBDEV_COMPOSITE

/**
 * USB2.0 interface association descriptor resolves ambiguity w.r.t. # interfaces per logical
 * function in multi-function devices. It describes associations of interfaces that should be
 * bound to the same device driver instance.
 */
static usb_interface_association_descriptor_t *
get_bcm_iad()
{
	return &bcm_iad;
}

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
		dbg("iso_ep_maxsize: invalid size, setting max ISO mps %d\n",
			USB_MAX_BT_ISO_PACKET);
		d->wMaxPacketSize = USB_MAX_BT_ISO_PACKET;
		break;
	}
}

/** Setup endpoint defaults for WLAN and optional BT interface */
static void
usbdev_set_ep_defaults(struct dngl_bus *bus, int int_num)
{
	int i, j, k;
	int offset = 0;

	/* BT interface #0: intr-in, bulk-in, bulk-out */
	if (int_num > 1) {
		/* mutiple interface and WLAN at interface 0 */
		if (bus->int_wlan == 0) {
			offset = 1;
		}
	}
	j = int_num - 1 + offset;
	if (j > (INTERFACE_BT + offset)) {
		i = INTERFACE_BT + offset;
		bus->ep_ii_num[i] = 1;
		bus->ep_bi_num[i] = 1;
		bus->ep_bo_num[i] = 1;
		bus->ep_isi_num[i] = 0;
		bus->ep_iso_num[i] = 0;
	}

	/* BT interface #1/alt settings: iso-in, iso-out */
	if (j > INTERFACE_ISO + offset) {
		k = INTERFACE_ISO + offset;
		for (i = k; i <= k + ALT_SETTING_MAX; i++) {
			bus->ep_ii_num[i] = 0;
			bus->ep_bi_num[i] = 0;
			bus->ep_bo_num[i] = 0;
			bus->ep_isi_num[i] = 1;
			bus->ep_iso_num[i] = 1;
		}
	}

	/* BT interface #2: DFU/0 endpoints */
	if (j > INTERFACE_DFU + offset) {
		i = INTERFACE_DFU + ALT_SETTING_MAX + offset;
		bus->ep_ii_num[i] = 0;
		bus->ep_bi_num[i] = 0;
		bus->ep_bo_num[i] = 0;
		bus->ep_isi_num[i] = 0;
		bus->ep_iso_num[i] = 0;
	}

	/* WLAN interface #dynamic: intr-in, bulk-in, bulk-out */
	if (j >= INTERFACE_BT) {
		if (j > INTERFACE_ISO)
			i = j + ALT_SETTING_MAX;
		else
			i = j;

		if (bus->int_wlan == 0) {
			i = 0;
		}

		bus->ep_ii_num[i] = 1;
		bus->ep_bi_num[i] = 1;
		bus->ep_bo_num[i] = 1;
		bus->ep_isi_num[i] = 0;
		bus->ep_iso_num[i] = 0;
	}
} /* usbdev_set_ep_defaults */

#endif /* BCMUSBDEV_COMPOSITE */

static const char BCMATTACHDATA(rstr_usbdesc_composite)[] = "usbdesc_composite";
static const char BCMATTACHDATA(rstr_eiD_ebiD_eboD)[] = "ei %d, ebi %d, ebo %d\n";
static const char BCMATTACHDATA(rstr_usbepnum)[] = "usbepnum";
static const char BCMATTACHDATA(rstr_rdlid)[] = "rdlid";
static const char BCMATTACHDATA(rstr_rdlrwu)[] = "rdlrwu";
static const char BCMATTACHDATA(rstr_usbrdy)[] = "usbrdy";
static const char BCMATTACHDATA(rstr_usbrdydelay)[] = "usbrdydelay";
static const char BCMATTACHDATA(rstr_hostrdy)[] = "hostrdy";
static const char BCMATTACHDATA(rstr_S_failed_to_register_host_ready_gpio_interrupt_handler)[] =
	"%s: failed to register host ready gpio interrupt handler\n";
static const char BCMATTACHDATA(rstr_hostwake)[] = "hostwake";
static const char BCMATTACHDATA(rstr_hw_gpdc)[] = "hw_gpdc";
static const char BCMATTACHDATA(rstr_manf)[] = "manf";
static const char BCMATTACHDATA(rstr_productname)[] = "productname";
static const char BCMATTACHDATA(rstr_boardnum)[] = "boardnum";
static const char BCMATTACHDATA(rstr_vendor_id)[] = "vendor_id";
static const char BCMATTACHDATA(rstr_subvendid)[] = "subvendid";
static const char BCMATTACHDATA(rstr_product_id)[] = "product_id";
static const char BCMATTACHDATA(rstr_subdevid)[] = "subdevid";
static const char BCMATTACHDATA(rstr_invalid_gpio_duty_cycle_8x_D_on_D_off)[] =
	"Invalid GPIO duty cycle: 0x%08x (%d on, %d off)\n";

/**
 * Host will read information from the device, such as number of USB configurations and endpoint
 * properties later on. This information is contained within the 'bus' structure, which is
 * initialized during firmware initialization (attach phase).
 */
static void
BCMATTACHFN(usbdev_attach_interface_ep_init)(struct dngl_bus *bus)
{
	const char *value;
	uint i, j;
	uint NEWCDC_CONFIGLEN;
	uint ep_i_num = 1;      /* input endpoint number */
	uint ep_o_num = 1;	/* output endpoint number */
#ifdef BCMUSBDEV_COMPOSITE
	uint ep_i_num_alt = 1;      /* input endpoint number */
	uint ep_o_num_alt = 1;   /* output endpoint number */

	int ep_val, val;
	int usbdesc = 0;
	int int_wlan;
	int intf_iso;
	int intf_bt;
	uint k, l, iso_mps_idx = 0;
	uint ep_sum = 0;	/* total number of endpoints */

	/* init interface: support multiple interfaces, IAD with OTP override */
	bus->interface_num = 1;
	intf_iso = INTERFACE_ISO;
	intf_bt = INTERFACE_BT;
	if ((value = getvar(NULL, rstr_usbdesc_composite)))
		usbdesc = bcm_strtoul(value, NULL, 0);

	int_wlan = 0;
	if (usbdesc) {
		i = (usbdesc & USB_IF_NUM_MASK);	/* BT interface/s */
		if(i) { /* BT interfaces is present, enable vusbd */
			bus->vusbd_enab = TRUE;
		}
		i++;				/* add WLAN interface */
		if (i > 1) {
			if (i <= (INTERFACE_MAX - ALT_SETTING_MAX)) {
				bus->interface_num = i;
			} else {
				bus->interface_num = INTERFACE_MAX - ALT_SETTING_MAX;
				dbg("invalid OTP interface setting %d, "
					"limiting to %d(max)\n",
					i-1, bus->interface_num);
			}
		}
		if (usbdesc & USB_WL_INTF_MASK) {
			/* to handle WLAN = 0 BT_INTF = 1..... */
			int_wlan = 0;
			intf_iso = INTERFACE_ISO + 1;
			intf_bt = INTERFACE_BT + 1;
			if (bus->interface_num > 1) {
				/* WL interface 0, BT 1, 2, 3 */
				get_bcm_interface()[1].bInterfaceNumber = intf_bt;
				get_bcm_interface()[1].bAlternateSetting = 0;
				get_bcm_interface()[1].bInterfaceClass = UICLASS_WIRELESS;
				get_bcm_interface()[1].bInterfaceSubClass = UISUBCLASS_RF;
				get_bcm_interface()[1].bInterfaceProtocol = UIPROTO_BLUETOOTH;
				get_bcm_interface()[1].iInterface = 6;

				for (i = 2; i < (INTERFACE_MAX-1); i++) {
				        get_bcm_interface()[i].bInterfaceNumber = intf_iso;
					get_bcm_interface()[i].bAlternateSetting = i-2;
					get_bcm_interface()[i].bInterfaceClass = UICLASS_WIRELESS;
					get_bcm_interface()[i].bInterfaceSubClass = UISUBCLASS_RF;
					get_bcm_interface()[i].bInterfaceProtocol =
						UIPROTO_BLUETOOTH;
					get_bcm_interface()[i].iInterface = 6;
				}
				get_bcm_interface()[8].bInterfaceNumber = INTERFACE_DFU + 1;
				get_bcm_interface()[8].bAlternateSetting = 0;
				get_bcm_interface()[8].bInterfaceClass = UICLASS_WIRELESS;
				get_bcm_interface()[8].bInterfaceSubClass = UISUBCLASS_RF;
				get_bcm_interface()[8].bInterfaceProtocol = UIPROTO_BLUETOOTH;
				get_bcm_interface()[8].iInterface = 7;
			}
		} else {
			int_wlan = bus->interface_num - 1;
		}
	}
	bus->int_wlan = int_wlan;
	if (int_wlan > INTERFACE_ISO)
		int_wlan += ALT_SETTING_MAX;
	bus->int_wlan_idx = int_wlan;

	if (int_wlan < (INTERFACE_MAX - 1)) {
		if (int_wlan > INTERFACE_ISO) {
			i = int_wlan;
		} else {
			i = bus->int_wlan;
		}
		get_bcm_interface()[i].bInterfaceNumber = INT_WLAN;
		get_bcm_interface()[i].bInterfaceClass = UICLASS_VENDOR;
		get_bcm_interface()[i].bInterfaceSubClass = UISUBCLASS_ABSTRACT_CONTROL_MODEL;
		get_bcm_interface()[i].bInterfaceProtocol = UIPROTO_DATA_VENDOR;
		get_bcm_interface()[i].iInterface = 2;
	}

	get_bcm_config()->bNumInterface = bus->interface_num;
	get_bcm_other_config()->bNumInterface = bus->interface_num;

	if ((CHIPID(bus->sih->chip) == BCM43242_CHIP_ID) ||
		(CHIPID(bus->sih->chip) == BCM43243_CHIP_ID) ||
		BCM4350_CHIP(sih->chip) ||
		0) {
		get_bcm_config()->bMaxPower = DEV_MAXPOWER_500;
		get_bcm_other_config()->bMaxPower = DEV_MAXPOWER_500;
	}

#ifdef USB_XDCI
	if (usbd_chk_version(bus) > UD_USB_2_1_DEV) {
		get_bcm_config()->bMaxPower = (uByte)DEV_MAXPOWER_900;
		get_bcm_other_config()->bMaxPower = (uByte)DEV_MAXPOWER_900;
	}
#endif /* USB_XDCI */

	/* update device settings if this is a composite device */
	if (bus->interface_num > 1) {
		/* composite without IAD */
		get_bcm_device()->bDeviceClass = UDCLASS_WIRELESS;
		get_bcm_device()->bDeviceSubClass = UDSUBCLASS_RF;
		get_bcm_device()->bDeviceProtocol = UDPROTO_BLUETOOTH;

	        get_bcm_hs_qualifier()->bDeviceClass = UDCLASS_WIRELESS;
		get_bcm_hs_qualifier()->bDeviceSubClass = UDSUBCLASS_RF;
		get_bcm_hs_qualifier()->bDeviceProtocol = UDPROTO_BLUETOOTH;

		get_bcm_fs_qualifier()->bDeviceClass = UDCLASS_WIRELESS;
		get_bcm_fs_qualifier()->bDeviceSubClass = UDSUBCLASS_RF;
		get_bcm_fs_qualifier()->bDeviceProtocol = UDPROTO_BLUETOOTH;

		get_bcm_device()->iProduct = 4;
	}

	if (bus->interface_num > 2) {
		/* composite with IAD */
		if (usbdesc & USB_IAD_MASK) {
			dbg("IAD enabled\n");
			get_bcm_device()->bDeviceClass = UDCLASS_MISC;
			get_bcm_device()->bDeviceSubClass = UDSUBCLASS_COMMON;
			get_bcm_device()->bDeviceProtocol = UDPROTO_IAD;

			get_bcm_hs_qualifier()->bDeviceClass = UDCLASS_MISC;
			get_bcm_hs_qualifier()->bDeviceSubClass = UDSUBCLASS_COMMON;
			get_bcm_hs_qualifier()->bDeviceProtocol = UDPROTO_IAD;

			get_bcm_fs_qualifier()->bDeviceClass = UDCLASS_MISC;
			get_bcm_fs_qualifier()->bDeviceSubClass = UDSUBCLASS_COMMON;
			get_bcm_fs_qualifier()->bDeviceProtocol = UDPROTO_IAD;

			get_bcm_device()->iProduct = 4;
			get_bcm_iad()->bInterfaceCount = get_bcm_config()->bNumInterface - 1;
			if (bus->int_wlan == 0) {
				/* WLAN at interface 0, push rad_iad.bFirstInterface */
				get_bcm_iad()->bFirstInterface = IAD_INT_NUM + 1;
			}
		}
		bus->interface_num += ALT_SETTING_MAX;
	}

	/* Device Class Override */
	if ((val = (usbdesc & USB_DEVCLASS_MASK) >> USB_DEVCLASS_SHIFT)) {
		if (val == USB_DEVCLASS_BT) {
			dbg("devclass BT override\n");
			get_bcm_device()->bDeviceClass = UDCLASS_WIRELESS;
			get_bcm_device()->bDeviceSubClass = UDSUBCLASS_RF;
			get_bcm_device()->bDeviceProtocol = UDPROTO_BLUETOOTH;
			get_bcm_hs_qualifier()->bDeviceClass = UDCLASS_WIRELESS;
			get_bcm_hs_qualifier()->bDeviceSubClass = UDSUBCLASS_RF;
			get_bcm_hs_qualifier()->bDeviceProtocol = UDPROTO_BLUETOOTH;
			get_bcm_fs_qualifier()->bDeviceClass = UDCLASS_WIRELESS;
			get_bcm_fs_qualifier()->bDeviceSubClass = UDSUBCLASS_RF;
			get_bcm_fs_qualifier()->bDeviceProtocol = UDPROTO_BLUETOOTH;

			get_bcm_device()->iProduct = 4;
		} else if (val == USB_DEVCLASS_WLAN) {
			dbg("devclass WLAN override\n");
			get_bcm_device()->bDeviceClass = UDCLASS_VENDOR;
			get_bcm_device()->bDeviceSubClass = 0;
			get_bcm_device()->bDeviceProtocol = 0;
			get_bcm_hs_qualifier()->bDeviceClass = UDCLASS_VENDOR;
			get_bcm_hs_qualifier()->bDeviceSubClass = 0;
			get_bcm_hs_qualifier()->bDeviceProtocol = 0;
			get_bcm_fs_qualifier()->bDeviceClass = UDCLASS_VENDOR;
			get_bcm_fs_qualifier()->bDeviceSubClass = 0;
			get_bcm_fs_qualifier()->bDeviceProtocol = 0;
			get_bcm_device()->iProduct = 2;
		}
	}

	/* init endpoints: support multiple EP and OTP override */
	ASSERT(get_bcm_config()->bNumInterface <= (INTERFACE_MAX - ALT_SETTING_MAX));
	usbdev_set_ep_defaults(bus, get_bcm_config()->bNumInterface);

#ifdef BCMUSBDEV_BULKIN_2EP
	if (!usbdesc)
		bus->ep_bi_num[int_wlan] = 2;
#endif

	if (usbdesc) {
		/* BT bulk endpoints # */
		if (BT_BULK_IN_NUM(usbdesc)) {
			bus->ep_bi_num[intf_bt] ++;
		}
		if (BT_BULK_OUT_NUM(usbdesc)) {
			bus->ep_bo_num[intf_bt] ++;
		}
		ep_val = (usbdesc & USB_WLANIF_INTR_EP_MASK) >> USB_WLANIF_INTR_EP_SHIFT;
		if (ep_val) {
			bus->ep_ii_num[int_wlan] = 0;
			bus->use_intr_ep = FALSE;
		} else {
			bus->ep_ii_num[int_wlan] = 1;
		}

		ep_val = ((usbdesc & USB_BI_EP_MASK) >> USB_BI_EP_SHIFT);
		if (ep_val > 0 && ep_val < EP_BULK_MAX)
			bus->ep_bi_num[int_wlan] += ep_val;

		ep_val = ((usbdesc & USB_B0_EP_MASK) >> USB_B0_EP_SHIFT);
		if (ep_val > 0 && ep_val < EP_BULK_MAX)
			bus->ep_bo_num[int_wlan] += ep_val;

		/* Max 4-in and 4-out endpoints
		 * Adjust endpoint allocation if OTP setting exceeds the limit
		 */
		if (get_bcm_config()->bNumInterface > 2) {
			bus->ep_ii_num[int_wlan] = 0;
			if (bus->ep_bi_num[int_wlan] > 2)
				bus->ep_bi_num[int_wlan] = 2;
			if (bus->ep_bo_num[int_wlan] > 2)
				bus->ep_bo_num[int_wlan] = 2;
		} else if (get_bcm_config()->bNumInterface > 1) {
			if (bus->ep_ii_num[int_wlan]) {
				bus->ep_bi_num[int_wlan] = 1;
			} else if (bus->ep_bi_num[int_wlan] > 2) {
				bus->ep_bi_num[int_wlan] = 2;
			}
			if (bus->ep_bo_num[int_wlan] > 3)
				bus->ep_bo_num[int_wlan] = 3;
		}
	}
	ep_sum = 0;
	for (k = 0; k < bus->interface_num; k++) {
		get_bcm_interface()[k].bNumEndpoints = bus->ep_ii_num[k]
			+ bus->ep_bi_num[k] + bus->ep_bo_num[k]
			+ bus->ep_isi_num[k] + bus->ep_iso_num[k];
		ep_sum += get_bcm_interface()[k].bNumEndpoints;
	}

	NEWCDC_CONFIGLEN = USB_CONFIG_DESCRIPTOR_SIZE +
		(USB_INTERFACE_DESCRIPTOR_SIZE * bus->interface_num) +
		(USB_ENDPOINT_DESCRIPTOR_SIZE * ep_sum);

	if (usbdesc & USB_IAD_MASK) {
		if (get_bcm_iad()->bInterfaceCount > 1) {
			NEWCDC_CONFIGLEN += USB_INTERFACE_ASSOCIATION_DESCRIPTOR_SIZE;
		} else {
			dbg("Invalid OTP IAD setting, interface count is %d(<2)\n",
				get_bcm_iad()->bInterfaceCount);
		}
	}

	if (bus->interface_num == INTERFACE_MAX)
		NEWCDC_CONFIGLEN += USB_DFU_FUNCTIONAL_DESCRIPTOR_SIZE;

	bus->iad = get_bcm_iad();
	bus->bcm_interface = get_bcm_interface();
	bus->interface = INT_WLAN;

	/* init config */
	get_bcm_config()->wTotalLength = NEWCDC_CONFIGLEN;
	get_bcm_other_config()->wTotalLength = NEWCDC_CONFIGLEN;
	bcopy(get_bcm_config(), &bus->bcm_config, sizeof(bus->bcm_config));
	bcopy(get_bcm_other_config(), &bus->bcm_other_config, sizeof(bus->bcm_other_config));

	/* HSIC is self-powered so change the descriptors to reflect this */
	if (usbd_hsic(bus)) {
		bus->bcm_config.bmAttributes &= ~UC_BUS_POWERED;
		bus->bcm_config.bmAttributes |= UC_SELF_POWERED;
		bus->bcm_config.bMaxPower = 0;
		bus->bcm_other_config.bmAttributes &= ~UC_BUS_POWERED;
		bus->bcm_other_config.bmAttributes |= UC_SELF_POWERED;
		bus->bcm_other_config.bMaxPower = 0;
	}

	/* print some cryptic msg unconditionally */
	printf(rstr_eiD_ebiD_eboD, bus->ep_ii_num[int_wlan],
		bus->ep_bi_num[int_wlan], bus->ep_bo_num[int_wlan]);

	/* build EP list for hs and fs:in (dongle->host) EPs first, then out EPs
	 * fixup the ep_num. The DMA channel and ep number will be allocated inside ep_attach()
	 */
	j = 0;
	l = 0;
	if (usbdesc)
		iso_mps_idx = (usbdesc & USB_ISO_MPS_MASK) >> USB_ISO_MPS_SHIFT;
	for (k = 0; k < bus->interface_num; k++) {
		if (bus->bcm_interface[k].bAlternateSetting) {
			ep_i_num = ep_i_num_alt;
			ep_o_num = ep_o_num_alt;
		} else {
			ep_i_num_alt = ep_i_num;
			ep_o_num_alt = ep_o_num;
		}
		for (i = 0; i < bus->ep_ii_num[k]; i++) {
#ifdef USB_XDCI
			bus->intr_endpoints_ss[i+l] = bcm_ss_ep_intr;
			bus->intr_endpoints_cp_ss[i+l] = bcm_ss_cp_intr;
#endif /* USB_XDCI */
			bus->intr_endpoints[i+l] = bcm_hs_ep_intr;
			bus->intr_endpoints_fs[i+l] = bcm_fs_ep_intr;

#ifdef USB_XDCI
			bus->intr_endpoints_ss[i+l].bEndpointAddress =
			UE_SET_ADDR((bus->intr_endpoints_ss[i+l].bEndpointAddress), ep_i_num);
#endif /* USB_XDCI */
			bus->intr_endpoints[i+l].bEndpointAddress =
				UE_SET_ADDR((bus->intr_endpoints[i+l].bEndpointAddress), ep_i_num);
			bus->intr_endpoints_fs[i+l].bEndpointAddress =
				UE_SET_ADDR((bus->intr_endpoints_fs[i+l].bEndpointAddress),
				ep_i_num);
			/* make IN and OUT number start from same after interrupt endpoints */
			ep_i_num ++;
			ep_o_num = ep_i_num;
		}
		l += i;

		for (i = 0; i < bus->ep_bi_num[k]; i++) {
#ifdef USB_XDCI
			bus->data_endpoints_ss[i+j] = bcm_ss_ep_bulkin;
			bus->data_endpoints_cp_ss[i+j] = bcm_ss_cp_bulkin;
#endif /* USB_XDCI */
			bus->data_endpoints[i+j] = bcm_hs_ep_bulkin;
			bus->data_endpoints_fs[i+j] = bcm_fs_ep_bulkin;
#ifdef USB_XDCI
			bus->data_endpoints_ss[i+j].bEndpointAddress =
			UE_SET_ADDR((bus->data_endpoints_ss[i+j].bEndpointAddress), ep_i_num);
#endif /* USB_XDCI */
			bus->data_endpoints[i+j].bEndpointAddress =
				UE_SET_ADDR((bus->data_endpoints[i+j].bEndpointAddress), ep_i_num);
			bus->data_endpoints_fs[i+j].bEndpointAddress =
				UE_SET_ADDR((bus->data_endpoints_fs[i+j].bEndpointAddress),
				ep_i_num);
				ep_i_num++;
		}
		j += i;
		for (i = 0; i < bus->ep_bo_num[k]; i++) {
#ifdef USB_XDCI
			bus->data_endpoints_ss[i + j] = bcm_ss_ep_bulkout;
			bus->data_endpoints_cp_ss[i + j] = bcm_ss_cp_bulkout;
#endif /* USB_XDCI */
			bus->data_endpoints[i+j] = bcm_hs_ep_bulkout;
			bus->data_endpoints_fs[i+j] = bcm_fs_ep_bulkout;
#ifdef USB_XDCI
			bus->data_endpoints_ss[i + j].bEndpointAddress =
				UE_SET_ADDR((bus->data_endpoints_ss[i + j].bEndpointAddress),
				ep_o_num);
#endif /* USB_XDCI */
			bus->data_endpoints[i+j].bEndpointAddress =
				UE_SET_ADDR((bus->data_endpoints[i+j].bEndpointAddress), ep_o_num);
			bus->data_endpoints_fs[i+j].bEndpointAddress =
				UE_SET_ADDR((bus->data_endpoints_fs[i+j].bEndpointAddress),
				ep_o_num);
				ep_o_num ++;
		}
		j += i;
		for (i = 0; i < bus->ep_isi_num[k]; i++) {
#ifdef USB_XDCI
			bus->data_endpoints_ss[i+j] = bcm_ss_ep_isin;
			bus->data_endpoints_cp_ss[i+j] = bcm_ss_cp_isin;
#endif /* USB_XDCI */
			bus->data_endpoints[i+j] = bcm_hs_ep_isin;
			if (iso_mps_idx)
				usbdev_set_iso_alt_ep_maxsize(&bus->data_endpoints[i+j],
					iso_mps_idx);
			else
				usbdev_set_iso_alt_ep_maxsize(&bus->data_endpoints[i+j], k);

			bus->data_endpoints_fs[i+j] = bcm_fs_ep_isin;
			if (iso_mps_idx)
				usbdev_set_iso_alt_ep_maxsize(&bus->data_endpoints_fs[i+j],
					iso_mps_idx);
			else
				usbdev_set_iso_alt_ep_maxsize(&bus->data_endpoints_fs[i+j], k);
#ifdef USB_XDCI
			bus->data_endpoints_ss[i+j].bEndpointAddress =
				UE_SET_ADDR((bus->data_endpoints_ss[i+j].bEndpointAddress),
				ep_i_num);
#endif /* USB_XDCI */
			bus->data_endpoints[i+j].bEndpointAddress =
				UE_SET_ADDR((bus->data_endpoints[i+j].bEndpointAddress), ep_i_num);
			bus->data_endpoints_fs[i+j].bEndpointAddress =
				UE_SET_ADDR((bus->data_endpoints_fs[i+j].bEndpointAddress),
				ep_i_num);
				ep_i_num ++;
		}
		j += i;

		for (i = 0; i < bus->ep_iso_num[k]; i++) {
#ifdef USB_XDCI
			bus->data_endpoints_ss[i+j] = bcm_ss_ep_isout;
			bus->data_endpoints_cp_ss[i+j] = bcm_ss_cp_isout;
#endif /* USB_XDCI */
			bus->data_endpoints[i+j] = bcm_hs_ep_isout;
			if (iso_mps_idx)
				usbdev_set_iso_alt_ep_maxsize(&bus->data_endpoints[i+j],
					iso_mps_idx);
			else
				usbdev_set_iso_alt_ep_maxsize(&bus->data_endpoints[i+j], k);
			bus->data_endpoints_fs[i+j] = bcm_fs_ep_isout;
			if (iso_mps_idx)
				usbdev_set_iso_alt_ep_maxsize(&bus->data_endpoints_fs[i + j],
					iso_mps_idx);
			else
				usbdev_set_iso_alt_ep_maxsize(&bus->data_endpoints_fs[i + j], k);
#ifdef USB_XDCI
			bus->data_endpoints_ss[i + j].bEndpointAddress =
				UE_SET_ADDR((bus->data_endpoints_ss[i + j].bEndpointAddress),
				ep_o_num);
#endif /* USB_XDCI */
			bus->data_endpoints[i+j].bEndpointAddress =
				UE_SET_ADDR((bus->data_endpoints[i+j].bEndpointAddress), ep_o_num);
			bus->data_endpoints_fs[i+j].bEndpointAddress =
				UE_SET_ADDR((bus->data_endpoints_fs[i+j].bEndpointAddress),
				ep_o_num);
				ep_o_num++;
		}
			j += i;
	}
#else
	int val;
	/* init endpoint: support multiple EP, and OTP override */
	bus->ep_ii_num = 1;
	bus->ep_bi_num = 1;
	bus->ep_bo_num = 1;
#ifdef BCMUSBDEV_BULKIN_2EP
	bus->ep_bi_num = 2;
#endif

	if ((CHIPID(bus->sih->chip) == BCM43242_CHIP_ID) ||
		(CHIPID(bus->sih->chip) == BCM43243_CHIP_ID) ||
		BCM4350_CHIP(bus->sih->chip) ||
		0) {
		get_bcm_config()->bMaxPower = DEV_MAXPOWER_500;
		get_bcm_other_config()->bMaxPower = DEV_MAXPOWER_500;
	}

#ifdef USB_XDCI
	if (usbd_chk_version(bus) > UD_USB_2_1_DEV) {
		get_bcm_config()->bMaxPower = (uByte)DEV_MAXPOWER_900;
		get_bcm_other_config()->bMaxPower = (uByte)DEV_MAXPOWER_900;
	}
#endif /* USB_XDCI */

	/* only support multiple BULK EPs for now, only one Interrupt EP */
	if ((value = getvar(NULL, rstr_usbepnum))) {
		i = bcm_strtoul(value, NULL, 0);
		val = (i & 0x0f);
		val = (i & 0x0f);
		if (val > 0 && val <= EP_BULK_MAX) {
			bus->ep_bi_num = val;
		}
		val = (i & 0xf0) >> 4;
		if (val > 0 && val <= EP_BULK_MAX) {
			bus->ep_bo_num = val;
		}
	}
	/*
	 * A USB device supports one or more USB configurations. Host reads USB configuration
	 * descriptor later on to learn about number of interfaces etc.
	 */
	bcopy(get_bcm_config(), &bus->bcm_config, sizeof(bus->bcm_config));
	bcopy(get_bcm_other_config(), &bus->bcm_other_config, sizeof(bus->bcm_other_config));
	bcopy(&bcm_interfaces, &bus->bcm_interface, sizeof(bus->bcm_interface));
	bus->bcm_interface.bNumEndpoints = bus->ep_ii_num + bus->ep_bi_num + bus->ep_bo_num;
	NEWCDC_CONFIGLEN = USB_CONFIG_DESCRIPTOR_SIZE + USB_INTERFACE_DESCRIPTOR_SIZE +
		(USB_ENDPOINT_DESCRIPTOR_SIZE * bus->bcm_interface.bNumEndpoints);

	bus->bcm_config.wTotalLength = NEWCDC_CONFIGLEN;
	bus->bcm_other_config.wTotalLength = NEWCDC_CONFIGLEN;

	/* HSIC is self-powered so change the descriptors to reflect this */
	if (usbd_hsic(bus)) {
		bus->bcm_config.bmAttributes &= ~UC_BUS_POWERED;
		bus->bcm_config.bmAttributes |= UC_SELF_POWERED;
		bus->bcm_config.bMaxPower = 0;
		bus->bcm_other_config.bmAttributes &= ~UC_BUS_POWERED;
		bus->bcm_other_config.bmAttributes |= UC_SELF_POWERED;
		bus->bcm_other_config.bMaxPower = 0;
	}

	/* print some cryptic msg unconditionally */
	printf(rstr_eiD_ebiD_eboD, bus->ep_ii_num, bus->ep_bi_num, bus->ep_bo_num);
	ep_i_num = 2;

	/* build EP list for hs and fs: bulk in (dongle->host) EPs first, then bulk out EPs
	 * fixup the ep_num. The DMA channel and ep number will be allocated inside ep_attach()
	 */
	for (i = 0; i < bus->ep_bi_num; i++) {
#ifdef USB_XDCI
		bus->data_endpoints_ss[i] = bcm_ss_ep_bulkin;
		bus->data_endpoints_cp_ss[i] = bcm_ss_cp_bulkin;
		bus->data_endpoints_ss[i].bEndpointAddress =
			UE_SET_ADDR((bus->data_endpoints_ss[i].bEndpointAddress), ep_i_num);
#endif /* USB_XDCI */
		bus->data_endpoints[i] = bcm_hs_ep_bulkin;
		bus->data_endpoints_fs[i] = bcm_fs_ep_bulkin;
		bus->data_endpoints[i].bEndpointAddress =
			UE_SET_ADDR((bus->data_endpoints[i].bEndpointAddress), ep_i_num);
		bus->data_endpoints_fs[i].bEndpointAddress =
			UE_SET_ADDR((bus->data_endpoints_fs[i].bEndpointAddress), ep_i_num);
		ep_i_num++;
	}

	j = i;
	ep_o_num = ep_i_num;

	for (i = 0; i < bus->ep_bo_num; i++) {
#ifdef USB_XDCI
		bus->data_endpoints_ss[i + j] = bcm_ss_ep_bulkout;
		bus->data_endpoints_cp_ss[i + j] = bcm_ss_cp_bulkout;
		bus->data_endpoints_ss[i + j].bEndpointAddress =
			UE_SET_ADDR((bus->data_endpoints_ss[i + j].bEndpointAddress), ep_o_num);
#endif /* USB_XDCI */
		bus->data_endpoints[i + j] = bcm_hs_ep_bulkout;
		bus->data_endpoints_fs[i + j] = bcm_fs_ep_bulkout;
		bus->data_endpoints[i + j].bEndpointAddress =
			UE_SET_ADDR((bus->data_endpoints[i + j].bEndpointAddress), ep_o_num);
		bus->data_endpoints_fs[i + j].bEndpointAddress =
			UE_SET_ADDR((bus->data_endpoints_fs[i + j].bEndpointAddress), ep_o_num);
		ep_o_num++;
	}
#endif /* BCMUSBDEV_COMPOSITE */
} /* usbdev_attach_interface_ep_init */


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

/**
 * Data structures need to be allocated and initialized before USB subsystem is operational.
 * Several NVRAM strings can be used to fine tune the behavior.
 */
struct dngl_bus *
BCMATTACHFN(usbdev_attach)(osl_t *osh, struct usbdev_chip *ch, si_t *sih)
{
	struct dngl_bus *bus;
	struct {
		int index;
		const char *name;
	} *s, custom_strings[] = {
		{ 1, rstr_manf },
		{ 2, rstr_productname },
#ifndef WLTEST	/* mfgtest needs unchanging serial # to quell PnP on every insertion */
		{ 3, rstr_boardnum },
#endif
		{ 0, NULL }
	};
	const char *value;
	int i;
#ifndef WLTEST
	int val;
	/* char *manfid_nvoptions[] = {"manfid", "vendor_id", "subvendid", NULL}; */

	const char *manfid_nvoptions[] = { rstr_vendor_id, rstr_subvendid, NULL, NULL};

	/* char *prodid_nvoptions[] = {"prodid", "product_id", "subdevid", NULL};  */
	const char *prodid_nvoptions[] = {rstr_product_id, rstr_subdevid, NULL, NULL};
	const char **cp;
#endif /* WLTEST */
	uint32 gpio;

	trace("usbdev_attach");
	if (!(bus = MALLOC(osh, sizeof(struct dngl_bus)))) {
		err("%s: out of memory, malloced %d bytes", __FUNCTION__, MALLOCED(osh));
		return NULL;
	}
	bzero(bus, sizeof(struct dngl_bus));

	bus->sih = sih;
	bus->osh = osh;
	bus->ch = ch;

#ifdef USB_XDCI
	if (usbdev_dev.flags & (1 << RTEDEVFLAG_USB30)) {
		/* xdci core is selected */
		usbdev_init_chip_functable_xdci(&bus->ch_func);
		bus->usb30d = TRUE;
	} else
#endif
	{
		usbdev_init_chip_functable_ch(&bus->ch_func); /* usb20core chip */
		bus->usb30d = FALSE;
	}

#ifndef NO_USB_INTREP
	bus->use_intr_ep = TRUE;
#endif /* NO_USB_INTREP */
#ifdef WLTEST
	bus->wltest = TRUE;
#endif
#ifdef OOB_WAKE
	bus->oob_wake = TRUE;
#endif
	/* invalidate CPULess write address */
	bus->cpuless_waddr = (uint8 *)0xffffffff;
	pktq_init(&bus->ctrlq, 1, PKTQ_LEN_DEFAULT);
	pktq_init(&bus->txq, 1, PKTQ_LEN_DEFAULT);
	if (!(bus->dngl = dngl_attach(bus, NULL, sih, osh)))
		goto fail;

	/* Initialize descriptors */
	i = usbd_chk_version(bus);
	if (i == UD_USB_3_DEV) {
		get_bcm_device()->bcdUSB = UD_USB_3_0;
		get_bcm_device()->bMaxPacketSize = USB_3_MAX_CTRL_PACKET_FACTORIAL;
	} else if (i == UD_USB_2_1_DEV) {
		get_bcm_device()->bcdUSB = UD_USB_2_1;
		get_bcm_device()->bMaxPacketSize = USB_2_MAX_CTRL_PACKET;
	} else if (i == UD_USB_1_1_DEV) {
		get_bcm_device()->bcdUSB = 0x0110;
	}

	for (i = 0; i < ARRAYSIZE(bus->strings); i++)
		bcopy(&bcm_strings[i], &bus->strings[i], sizeof(usb_string_descriptor_t));

	/* HSIC is self-powered so change the descriptors to reflect this */
	if (usbd_hsic(bus)) {
		get_bcm_config()->bmAttributes &= ~UC_BUS_POWERED;
		get_bcm_config()->bmAttributes |= UC_SELF_POWERED;
		get_bcm_config()->bMaxPower = 0;
		get_bcm_other_config()->bmAttributes &= ~UC_BUS_POWERED;
		get_bcm_other_config()->bmAttributes |= UC_SELF_POWERED;
		get_bcm_other_config()->bMaxPower = 0;
	}

	/* Convert custom ASCII strings to UTF16 (not NULL terminated) */
	for (s = custom_strings; s->name; s++) {
		if ((value = getvar(NULL, s->name))) {
			for (i = 0; i < strlen(value) && i < (USB_MAX_STRING_LEN); i++)
				bus->strings[s->index].bString[i] = (uWord) value[i];

			bus->strings[s->index].bLength = (i * sizeof(uWord)) + 2;
		}
	}

#if defined(BCMUSBDEV_COMPOSITE) && defined(USB_IFTEST)
	bus->ep_config_valid = FALSE;
#endif /* BCMUSBDEV_COMPOSITE && USB_IFTEST */

	usbdev_attach_interface_ep_init(bus);
#ifndef WLTEST
	/* Support custom manf and product IDs for PnP (both CIS-srom & nvram style) */
	for (cp = manfid_nvoptions; *cp != NULL; ++cp) {
		if ((value = getvar(NULL, *cp)) && (val = bcm_strtoul(value, NULL, 0))) {
			get_bcm_device()->idVendor = val;
			break;
		}
	}
	for (cp = prodid_nvoptions; *cp != NULL; ++cp) {
		if ((value = getvar(NULL, *cp)) && (val = bcm_strtoul(value, NULL, 0))) {
#ifdef BCMUSB_NODISCONNECT
			/* Single enumeration disable/enable fix for Vista;
			 * Vista queries device desriptor during enable so
			 * match up with boot loader product id
			 */
			if ((value = getvar(NULL, rstr_rdlid)))
				get_bcm_device()->idProduct = bcm_strtoul(value, NULL, 0);
			else
				get_bcm_device()->idProduct = BCM_DNGL_BL_PID;
#else
			get_bcm_device()->idProduct = val;
#endif /* BCMUSB_NODISCONNECT */
			break;
		}
	}
#endif /* WLTEST */

	bus->generictimer = dngl_init_timer(bus, NULL, usbdev_generictimer);

#ifdef BCMUSB_NODISCONNECT


	/* remote wakeup capability override: retain bootloader attribute */
	if ((value = getvar(NULL, rstr_rdlrwu))) {
		i = bcm_strtoul(value, NULL, 0);
		dbg("found rdlrwu val %d", i);
		if (i == 0) {	/* honor disable since default is on */
			bus->bcm_config.bmAttributes &= ~UC_REMOTE_WAKEUP;
			bus->bcm_other_config.bmAttributes &= ~UC_REMOTE_WAKEUP;
		}
	}
#endif /* BCMUSB_NODISCONNECT */

	if (bus->oob_wake) {
	/* look for "device ready" gpio nvram var */
	if ((value = getvar(NULL, rstr_usbrdy)) != NULL) {
		bus->devrdy_gp = (uint32)bcm_strtoul(value, NULL, 0);
		gpio = (bus->devrdy_gp & USBGPIO_GPIO_MASK);

		bus->devrdy_state = 1;
		bus->devrdydelay = getintvar(NULL, rstr_usbrdydelay);
		if (bus->devrdydelay)
			bus->devrdytimer = dngl_init_timer(bus, NULL, usbdev_devrdytimer);
		if (bus->devrdy_gp & USBGPIO_DEFAULTPULL_BIT) {
				/* add a weak pull in the active direction since
				* we're ready by default
				*/
			si_gpiopull(bus->sih, (bus->devrdy_gp >> USBGPIO_POLARITYBIT_SHIFT),
			            1 << gpio, 1 << gpio);
		}
	} else
		bus->devrdy_gp = USBGPIO_INVALID;

	/* look for "host ready" gpio nvram var */
	if ((value = getvar(NULL, rstr_hostrdy)) != NULL) {
		bus->hostrdy_gp = (uint32)bcm_strtoul(value, NULL, 0);
		gpio = (bus->hostrdy_gp & USBGPIO_GPIO_MASK);

		/* register "host ready" interrupt handler (TRUE=level-detect) */
		bus->hostrdy_h = usbdev_gpio_handler_register(1 << gpio, TRUE,
			usbdev_hostrdy, (void *)bus);
		if (!bus->hostrdy_h)
			printf(rstr_S_failed_to_register_host_ready_gpio_interrupt_handler,
				__FUNCTION__);

		/* make polarity opposite of the current value */
			si_gpiointpolarity(bus->sih, 1 << gpio,
				(si_gpioin(bus->sih) & (1 << gpio)), 0);

		/* always enabled */
		si_gpiointmask(bus->sih, 1 << gpio, 1 << gpio, 0);

		if (bus->hostrdy_gp & USBGPIO_DEFAULTPULL_BIT) {
			/* add a weak pull in the inactive direction */
				si_gpiopull(bus->sih,
					!(bus->hostrdy_gp >> USBGPIO_POLARITYBIT_SHIFT),
					1 << gpio, 1 << gpio);
		}
	} else
		bus->hostrdy_gp = USBGPIO_INVALID;

/* look for "device ready" gpio nvram var */
	if ((value = getvar(NULL, rstr_hostwake)) != NULL) {
		bus->hostwake_gp = (uint32)bcm_strtoul(value, NULL, 0);
		gpio = (bus->hostwake_gp & USBGPIO_GPIO_MASK);

		if ((value = getvar(NULL, rstr_hw_gpdc)) != NULL) {
			uint32 gpdc;
			gpdc = (uint32)bcm_strtoul(value, NULL, 0);
			bus->gpontime = (gpdc >> 16);
			bus->gpofftime = gpdc & 0xffff;
#ifdef BCMDBG
			if (!bus->gpontime != !bus->gpofftime) {
				printf(rstr_invalid_gpio_duty_cycle_8x_D_on_D_off,
					gpdc, bus->gpontime, bus->gpofftime);
				bus->gpofftime = bus->gpontime = 0;
			}
#endif /* BCMDBG */

			if (bus->hostwake_gp & USBGPIO_DEFAULTPULL_BIT) {
				/* add a weak pull in the inactive direction */
				si_gpiopull(bus->sih,
					!(bus->hostwake_gp >> USBGPIO_POLARITYBIT_SHIFT),
					1 << gpio, 1 << gpio);
			}
			if (bus->gpontime)
				bus->gptimer = dngl_init_timer(bus, NULL, usbdev_gptimer);
		}
	} else
		bus->hostwake_gp = USBGPIO_INVALID;

		/* look for "device resume" gpio nvram var */
		if ((value = getvar(NULL, "usbres")) != NULL) {
			bus->resume_gp = (uint32)bcm_strtoul(value, NULL, 0);
			gpio = (bus->resume_gp & USBGPIO_GPIO_MASK);

			if (bus->resume_gp & USBGPIO_DEFAULTPULL_BIT) {
				/* add a weak pull in the inactive direction */
				si_gpiopull(bus->sih,
					!(bus->resume_gp >> USBGPIO_POLARITYBIT_SHIFT),
					1 << gpio, 1 << gpio);
	}
		} else
			bus->resume_gp = USBGPIO_INVALID;
	} /* OOB_WAKE */

#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
	if (VUSBD_ENAB(bus)) {
		bus->vusbd = vusbd_attach(bus->osh, bus, bus->usb30d);
		if (!bus->vusbd) {
			err("fail to attach vusbd!\n");
			bus->vusbd_enab = FALSE;
			goto fail;
		}
	}
#endif /* BCMUSBDEV_COMPOSITE && BCM_VUSBD */

	trace("done");
	return bus;

fail:
	MFREE(bus->osh, bus, sizeof(struct dngl_bus));
	return NULL;
} /* usbdev_attach */

void
BCMATTACHFN(usbdev_detach)(struct dngl_bus *bus)
{
	trace("usbdev_detach");
	dngl_detach(bus->dngl);

	if (bus->hostrdy_h)
		usbdev_gpio_handler_unregister(bus->hostrdy_h);

#ifdef OOB_WAKE
	if (bus->gptimer) {
		if (bus->gptimer_active)
			dngl_del_timer(bus->gptimer);
		dngl_free_timer(bus->gptimer);
	}
	if (bus->devrdytimer) {
		if (bus->devrdytimer_active)
			dngl_del_timer(bus->devrdytimer);
		dngl_free_timer(bus->devrdytimer);
	}
#endif /* OOB_WAKE */
	if (bus->generictimer) {
		if (bus->generictimer_active)
			dngl_del_timer(bus->generictimer);
		dngl_free_timer(bus->generictimer);
	}

	pktq_deinit(&bus->ctrlq);
	pktq_deinit(&bus->txq);

	MFREE(bus->osh, bus, sizeof(struct dngl_bus));

	trace("done");
} /* usbdev_detach */

/** hide struct details for callers outside of this file, e.g. usbdev_sb.c */
void *
usbdev_dngl(struct dngl_bus *bus)
{
	return bus->dngl;
}


#ifdef BCMUSBDEV_COMPOSITE

static usb_interface_descriptor_t *
get_bcm_interface()
{
	return bcm_interfaces;
}

static usb_device_qualifier_t *
get_bcm_hs_qualifier()
{
	return &bcm_hs_qualifier;
}

static usb_device_qualifier_t *
get_bcm_fs_qualifier()
{
	 return &bcm_fs_qualifier;
}
#endif /* BCMUSBDEV_COMPOSITE */

static void usbdev_pktdump(uint8 *pdata, uint32 len)
{
	int i;
	if (len > 80)
		len = 80;
	printf("setup data %d bytes\n", len);
	for (i = 0; i < len; i += 8) {
		printf("%2x %2x %2x %2x %2x %2x %2x %2x\n",
			pdata[0],  pdata[1],  pdata[2],  pdata[3],
			pdata[4],  pdata[5],  pdata[6],  pdata[7]);
		pdata += 8;
	}
}

/* ! ROM accessor to avoid struct in shdat */
static usb_device_descriptor_t *
get_bcm_device(void)
{
	return &bcm_device;
}

/**
 * Lower USB level (usbdev_sb.c) indicated that a setup transaction was received from the host. The
 * lower level passes the received buffer, containing a usb device request, in parameter 'p'. Setup
 * packets are used for USB standard dictated functionality (like the host interrogating the device
 * about the interfaces it supports) but also for Broadcom specific functionality (e.g. downloading
 * firmware).
 *
 * The USB spec dictates tight timing requirements for setup transactions. Keep this in mind when
 * adding printf's to this code during debugging. Some setup transactions must complete in 50ms.
 *
 * The SETUP transfers are always received on logical endpoint 0.
 */
void *
usbdev_setup(struct dngl_bus *bus, int ep, void *p, int *errp, int *dir)
{
	usb_device_request_t dr;
	void *p0 = NULL;
	int i, j;
	bootrom_id_t *id;

	trace("");
	ASSERT(ep == 0);

	ltoh_usb_device_request(PKTDATA(bus->osh, p), &dr);
	bus->ctrlout_wrhw = FALSE;
	*dir = dr.bmRequestType & UT_READ;
	/* Get standard descriptor */
	/* open it for all UT_READ_DEVICE, UT_READ_INTERFACE, UT_READ_ENDPOINT */
	if ((dr.bmRequestType&0xF0) == UT_READ_DEVICE) {
		uchar *cur = NULL;
		int bufsize = 256;
		if (dr.bRequest == UR_GET_DESCRIPTOR) {
#ifdef BCMUSBDEV_COMPOSITE
			if (get_bcm_config()->wTotalLength > bufsize)
				bufsize *= 2;
#endif /* BCMUSBDEV_COMPOSITE */
			if (!(p0 = PKTGET(bus->osh, bufsize, TRUE))) {
				err("ep%d: out of txbufs", ep);
				goto stall;
			}
			cur = PKTDATA(bus->osh, p0);

			switch ((dr.wValue >> 8) & 0xff) {
			case UDESC_DEVICE:
				dbg("ep%d: UDESC_DEVICE", ep);
				cur += htol_usb_device_descriptor(get_bcm_device(), cur);
				break;
			case UDESC_CONFIG:
			{
				const usb_endpoint_descriptor_t *bcm_cur_endpoints;
#ifdef USB_XDCI
				const usb_endpoint_companion_descriptor_t *bcm_cur_ed_cmps = NULL;
#endif /* USB_XDCI */
#ifdef BCMUSBDEV_COMPOSITE
				int k, l;
				cur += usbdev_htol_usb_config_descriptor(
					&bus->bcm_config, cur);
				j = l = 0;
				for (k = 0; k < bus->interface_num; k++) {
					uint32 intf_dfu = INTERFACE_DFU;
					if (bus->int_wlan == 0 && bus->interface_num > 1)
						intf_dfu = INTERFACE_DFU + 1;

					if ((get_bcm_iad()->bInterfaceCount > 1) &&
						(k == get_bcm_iad()->bFirstInterface))
						cur += usbdev_htol_usb_iad(bus->iad, cur);

					cur += usbdev_htol_usb_interface_descriptor(
						&bus->bcm_interface[k], cur);
					if ((get_bcm_interface()[k].bInterfaceNumber == intf_dfu) &&
						(get_bcm_interface()[k].bInterfaceNumber !=
						bus->int_wlan))
					{
						cur += usbdev_htol_usb_dfu_functional_descriptor(
							&bcm_dfu, cur);
					}

#ifdef USB_XDCI
					if (bus->speed == UD_USB_SS) {
						bcm_cur_endpoints = bus->intr_endpoints_ss;
						bcm_cur_ed_cmps = bus->intr_endpoints_cp_ss;
					} else
#endif /* USB_XDCI */
					if (bus->speed == UD_USB_HS)
						bcm_cur_endpoints = bus->intr_endpoints;
					else
						bcm_cur_endpoints = bus->intr_endpoints_fs;
					for (i = 0; i < bus->ep_ii_num[k]; i++) {
						cur += usbdev_htol_usb_endpoint_descriptor(
							&bcm_cur_endpoints[i+l], cur);
#ifdef USB_XDCI
						if (bus->speed == UD_USB_SS)
							cur +=
							usbdev_htol_usb_ed_companion_descriptor(
							&bcm_cur_ed_cmps[i+l], cur);
#endif /* USB_XDCI */
					}
					l += i;

#ifdef USB_XDCI
					if (bus->speed == UD_USB_SS) {
						bcm_cur_endpoints = bus->data_endpoints_ss;
						bcm_cur_ed_cmps = bus->data_endpoints_cp_ss;
					} else
#endif /* USB_XDCI */
					if (bus->speed == UD_USB_HS)
						bcm_cur_endpoints = bus->data_endpoints;
					else
						bcm_cur_endpoints = bus->data_endpoints_fs;
					for (i = 0; i < bus->ep_bi_num[k]; i++) {
						cur += usbdev_htol_usb_endpoint_descriptor(
							&bcm_cur_endpoints[i+j], cur);
#ifdef USB_XDCI
						if (bus->speed == UD_USB_SS)
							cur +=
							usbdev_htol_usb_ed_companion_descriptor(
							&bcm_cur_ed_cmps[i+j], cur);
#endif	/* USB_XDCI */
					}
					j += i;

					for (i = 0; i < bus->ep_bo_num[k]; i++)
					{
						cur += usbdev_htol_usb_endpoint_descriptor(
							&bcm_cur_endpoints[i+j], cur);
#ifdef USB_XDCI
						if (bus->speed == UD_USB_SS)
							cur +=
							usbdev_htol_usb_ed_companion_descriptor(
							&bcm_cur_ed_cmps[i+j], cur);
#endif /* USB_XDCI */
					}
					j += i;

					for (i = 0; i < bus->ep_isi_num[k]; i++) {
						cur += usbdev_htol_usb_endpoint_descriptor(
						&bcm_cur_endpoints[i+j], cur);
#ifdef USB_XDCI
						if (bus->speed == UD_USB_SS)
							cur +=
							usbdev_htol_usb_ed_companion_descriptor(
							&bcm_cur_ed_cmps[i+j], cur);
#endif /* USB_XDCI */

					}
					j += i;

					for (i = 0; i < bus->ep_iso_num[k]; i++) {
						cur += usbdev_htol_usb_endpoint_descriptor(
							&bcm_cur_endpoints[i+j], cur);
#ifdef USB_XDCI
						if (bus->speed == UD_USB_SS)
							cur +=
							usbdev_htol_usb_ed_companion_descriptor(
							&bcm_cur_ed_cmps[i+j], cur);
#endif /* USB_XDCI */
					}
					j += i;
				}
#else

				cur += usbdev_htol_usb_config_descriptor(
					&bus->bcm_config, cur);

				cur += usbdev_htol_usb_interface_descriptor(
						&bus->bcm_interface, cur);
#ifdef USB_XDCI
				if (bus->speed == UD_USB_SS) {
					bcm_cur_endpoints = &bcm_ss_ep_intr;
					bcm_cur_ed_cmps = &bcm_ss_cp_intr;
				} else
#endif /* USB_XDCI */
				if (bus->speed == UD_USB_HS)
					bcm_cur_endpoints = &bcm_hs_ep_intr;
				else
					bcm_cur_endpoints = &bcm_fs_ep_intr;

				cur += usbdev_htol_usb_endpoint_descriptor(
					bcm_cur_endpoints, cur);

#ifdef USB_XDCI
				if (bus->speed == UD_USB_SS)
					cur += usbdev_htol_usb_ed_companion_descriptor(
						bcm_cur_ed_cmps, cur);

				if (bus->speed == UD_USB_SS) {
					bcm_cur_endpoints = bus->data_endpoints_ss;
					bcm_cur_ed_cmps = bus->data_endpoints_cp_ss;
				} else
#endif /* USB_XDCI */
				if (bus->speed == UD_USB_HS)
					bcm_cur_endpoints = bus->data_endpoints;
				else
					bcm_cur_endpoints = bus->data_endpoints_fs;

				for (i = 0; i < bus->ep_bi_num; i++) {
					cur += usbdev_htol_usb_endpoint_descriptor(
						&bcm_cur_endpoints[i], cur);
#ifdef USB_XDCI
					if (bus->speed == UD_USB_SS)
						cur += usbdev_htol_usb_ed_companion_descriptor(
							&bcm_cur_ed_cmps[i], cur);
#endif /* USB_XDCI */
				}
				j = i;

				for (i = 0; i < bus->ep_bo_num; i++) {
					cur += usbdev_htol_usb_endpoint_descriptor(
						&bcm_cur_endpoints[j], cur);
#ifdef USB_XDCI
					if (bus->speed == UD_USB_SS)
						cur += usbdev_htol_usb_ed_companion_descriptor(
							&bcm_cur_ed_cmps[j], cur);
#endif /* USB_XDCI */

					++j;
				}
#endif /* BCMUSBDEV_COMPOSITE */
			}
				break;
			case UDESC_OTHER_SPEED_CONFIGURATION:
				i = usbd_chk_version(bus);
				if ((i == UD_USB_2_0_DEV) || (i == UD_USB_2_1_DEV)) {
					const usb_endpoint_descriptor_t *bcm_other_endpoints;
#ifdef BCMUSBDEV_COMPOSITE
					int k, l;

					cur += usbdev_htol_usb_config_descriptor(
						&bus->bcm_other_config, cur);

					j = l = 0;
					for (k = 0; k < bus->interface_num; k++) {
						uint32 dfu_intf = INTERFACE_DFU;

						if (bus->int_wlan == 0 && bus->interface_num > 1)
							dfu_intf = INTERFACE_DFU + 1;

						if ((get_bcm_iad()->bInterfaceCount > 1) &&
							(k == get_bcm_iad()->bFirstInterface))
							cur += usbdev_htol_usb_iad(bus->iad, cur);

						cur += usbdev_htol_usb_interface_descriptor(
							&bus->bcm_interface[k], cur);

						if (get_bcm_interface()[k].bInterfaceNumber ==
							dfu_intf)
						{
							cur +=
							usbdev_htol_usb_dfu_functional_descriptor(
								&bcm_dfu, cur);
						}

						if (bus->speed == UD_USB_HS)
							bcm_other_endpoints =
								bus->intr_endpoints;
						else
							bcm_other_endpoints =
								bus->intr_endpoints_fs;

						for (i = 0; i < bus->ep_ii_num[k]; i++)
							cur += usbdev_htol_usb_endpoint_descriptor(
								&bcm_other_endpoints[i+l], cur);
						l += i;

						if (bus->speed == UD_USB_HS)
							bcm_other_endpoints =
								bus->data_endpoints;
						else
							bcm_other_endpoints =
								bus->data_endpoints_fs;

						for (i = 0; i < bus->ep_bi_num[k]; i++)
							cur +=
							usbdev_htol_usb_endpoint_descriptor(
								&bcm_other_endpoints[i+j], cur);
						j += i;

						for (i = 0; i < bus->ep_bo_num[k]; i++)
							cur +=
							usbdev_htol_usb_endpoint_descriptor(
								&bcm_other_endpoints[i+j], cur);
						j += i;

						for (i = 0; i < bus->ep_isi_num[k]; i++)
							cur +=
							usbdev_htol_usb_endpoint_descriptor(
								&bcm_other_endpoints[i+j], cur);
						j += i;

						for (i = 0; i < bus->ep_iso_num[k]; i++)
							cur += usbdev_htol_usb_endpoint_descriptor(
								&bcm_other_endpoints[i+j], cur);
						j += i;
					}
#else
					cur += usbdev_htol_usb_config_descriptor(
						&bus->bcm_other_config, cur);

					cur += usbdev_htol_usb_interface_descriptor(
					&bus->bcm_interface, cur);

					if (bus->speed == UD_USB_HS)
						bcm_other_endpoints = bus->data_endpoints;
					else
						bcm_other_endpoints = bus->data_endpoints_fs;

					for (i = 0; i < bus->ep_bi_num; i++)
						cur += usbdev_htol_usb_endpoint_descriptor(
							&bcm_other_endpoints[i], cur);

					j = i;

					for (i = 0; i < bus->ep_bo_num; i++)
						cur += usbdev_htol_usb_endpoint_descriptor(
							&bcm_other_endpoints[j], cur);

#endif /* BCMUSBDEV_COMPOSITE */
					break;
				} else
					goto stall;
			case UDESC_INTERFACE:
				i = dr.wValue & 0xff;
				dbg("ep%d: UDESC_INTERFACE %d", ep, i);
#ifdef BCMUSBDEV_COMPOSITE
				if ((i >= 0) && (i < get_bcm_config()->bNumInterface)) {
					uint32 intf_iso = INTERFACE_ISO;

					if (bus->int_wlan == 0 && bus->interface_num > 1) {
						intf_iso += 1;
					}

					if (i > intf_iso) {
						cur += usbdev_htol_usb_interface_descriptor(
							&(get_bcm_interface()[i+ALT_SETTING_MAX]),
							cur);
					} else {
						cur += usbdev_htol_usb_interface_descriptor(
							&(get_bcm_interface()[i]), cur);
					}
				}
#else
				if (i <= get_bcm_config()->bNumInterface)
					cur += usbdev_htol_usb_interface_descriptor(
						&bcm_interfaces[i], cur);
#endif /* BCMUSBDEV_COMPOSITE */
				else
					goto stall;
				break;
			case UDESC_STRING:
				i = dr.wValue & 0xff;
				dbg("ep%d: UDESC_STRING %d langid 0x%x", ep, i, dr.wIndex);
#ifdef BCMUSBDEV_COMPOSITE
				if (i > 7)
#else
				if (i > 3)
#endif /* BCMUSBDEV_COMPOSITE */
					goto stall;
				cur += htol_usb_string_descriptor(&bus->strings[i], cur);
				break;
			case UDESC_DEVICE_QUALIFIER:
				dbg("ep%d: UDESC_DEVICE_QUALIFIER", ep);
				/* return qualifier info for "other" (i.e., not current) speed */
#ifdef USB_XDCI
				if (bus->speed == UD_USB_SS)
					break;
				else
#endif /* USB_XDCI */
				if (bus->speed == UD_USB_HS)
					cur += usbdev_htol_usb_device_qualifier(
						&bcm_hs_qualifier, cur);
				else
					cur += usbdev_htol_usb_device_qualifier(
					        &bcm_fs_qualifier, cur);
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
				err("ep%d: unhandled desc type %d\n", ep,
				       (dr.wValue >> 8) & 0xff);
				goto stall;
			}
		} else if (dr.bRequest == UR_GET_CONFIG) {
			if (!(p0 = PKTGET(bus->osh, 4, TRUE))) {
				err("ep%d: out of txbufs", ep);
				goto stall;
			}
			cur = PKTDATA(bus->osh, p0);
			*cur++ = (char) bus->confignum;
			err("ep%d: get config returning config %d\n", ep, bus->confignum);
		} else if (dr.bRequest == UR_GET_INTERFACE) {
			if (!(p0 = PKTGET(bus->osh, 4, TRUE))) {
				err("ep%d: out of txbufs", ep);
				goto stall;
			}
			cur = PKTDATA(bus->osh, p0);

			*cur++ = bus->interface;
			err("ep%d: get interface returning interface %d\n", ep, bus->interface);
		} else if (dr.bRequest == UR_GET_STATUS) {
			/* check wValue is 0 and wLength is 2 */
			if (!(p0 = PKTGET(bus->osh, 4, TRUE))) {
				err("ep%d: out of txbufs", ep);
				goto stall;
			}
			cur = PKTDATA(bus->osh, p0);
			*cur++ = (uint16)0;
			err("ep%d: get status, index %d, value %d, length %d\n",
				ep, dr.wIndex, dr.wValue, dr.wLength);
		} else {
			err("ep%d: unhandled read device bRequest 0x%x, wValue 0x%x\n", ep,
			       dr.bRequest, dr.wValue);
			goto stall;
		}

		PKTSETLEN(bus->osh, p0, cur - PKTDATA(bus->osh, p0));

	} else if (dr.bmRequestType == UT_WRITE_INTERFACE && dr.bRequest == UR_SET_INTERFACE) {

		/* dummy handler to satisfy WHQL interface query */
		bus->interface = dr.wValue;
		err("ep%d: set interface set interface %d\n", ep, bus->interface);
#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
		if (VUSBD_ENAB(bus)) {
			if (dr.wIndex != bus->int_wlan) {
				vusbd_process_setalt(bus->vusbd, dr.wIndex, dr.wValue);
			}
		}
#endif /* BCMUSBDEV_COMPOSITE && BCM_VUSBD */

#ifdef BCMUSBDEV_COMPOSITE
	} else if (dr.bmRequestType == UT_WRITE_CLASS_INTERFACE && dr.wIndex <= INT_WLAN) {
#else
	} else if (dr.bmRequestType == UT_WRITE_CLASS_INTERFACE && dr.wIndex == INT_WLAN) {
#endif /* BCMUSBDEV_COMPOSITE */

		/* Send encapsulated command */
		dbg("ep%d: ctrl out", ep);

#ifdef BCMUSBDEV_COMPOSITE
	} else if (dr.bmRequestType == UT_READ_CLASS_INTERFACE && dr.wIndex <= INT_WLAN) {
#else
	} else if (dr.bmRequestType == UT_READ_CLASS_INTERFACE && dr.wIndex == INT_WLAN) {
#endif /* BCMUSBDEV_COMPOSITE */

		/* Get encapsulated response */
		dbg("ep%d: ctrl in", ep);
		if (!(p0 = pktdeq(&bus->ctrlq)) && bus->use_intr_ep) {
			if (!(p0 = PKTGET(bus->osh, 4, TRUE))) {
				err("ep%d: out of txbufs", ep);
				goto stall;
			}

			/* set one byte dummy frame if no response pkt in ctrlq */
			PKTSETLEN(bus->osh, p0, 1);
			((char *) PKTDATA(bus->osh, p0))[0] = 0;
		}

	} else if (dr.bmRequestType == UT_WRITE_VENDOR_INTERFACE) {

		/* Send special low-level command primitive */
		if (dr.bRequest == DL_REBOOT) {
			err("got DL_REBOOT USB primitive\n");
			dngl_schedule_work(bus->dngl, NULL, _dngl_reboot, 500);
		} else if (dr.bRequest == DL_WRHW) {
			bus->ctrlout_wrhw = TRUE;
		} else if (dr.bRequest == DL_ENABLE_U1U2) {
			printf("usbdev_setup DL_ENABLE_U1U2\n");
			bus->enable_u1u2_needed = TRUE;
		}

	} else if ((dr.bmRequestType == UT_READ_VENDOR_INTERFACE) &&
		(dr.bRequest == DL_DEFER_RESP_OK)) {

		/* BMAC: used for call-with-return, where Setup IN are posted with this
		 * Request = DL_DEFER_RESP_OK. Since real data RPC call comes after this
		 * and only one can be outstanding, the below ctrlq should be empty
		 * and setup_resp_pending will trigger bus_sendctl() to send the result over
		 */

		trace("ep%d: ctrl in defer resp", ep);
		if (!(p0 = pktdeq(&bus->ctrlq))) {
			bus->setup_resp_pending = TRUE;
			*errp = dr.bmRequestType & UT_READ;
		}

	} else if (dr.bmRequestType == UT_READ_VENDOR_INTERFACE) {

		if (!(p0 = PKTGET(bus->osh, sizeof(bootrom_id_t), TRUE))) {
			err("ep%d: out of txbufs", ep);
			err("**** Can't get buffer, stall");
			goto stall;
		}

		if (dr.bRequest == DL_GETVER) {
			id = (bootrom_id_t*)PKTDATA(bus->osh, p0);
			id->chip = POSTBOOT_ID;
			id->chiprev = 0x0001;
		} else if (dr.bRequest == DL_RESETCFG) {
			/* reset the device one more time - for single enumeration */
			usbdev_setcfg(bus, 0);
			OSL_DELAY(1000); /* wait 1 ms */
			usbdev_setcfg(bus, 1);
			OSL_DELAY(1000); /* wait 1 ms */
		}
	} else if (bus->wltest && dr.bmRequestType == UT_WRITE_VENDOR_DEVICE &&
		dr.bRequest == CPULESS_INCR_ADDR) {

		/* CPULess access WRITE primitive */
		bus->cpuless_waddr = (uint8 *) ((dr.wIndex << 16) | dr.wValue);
		bus->cpuless_wlen = dr.wLength;
		/* the data will arrive separately, in the usbdev_rx() fn */
	} else if (dr.bmRequestType == UT_READ_VENDOR_DEVICE &&
		dr.bRequest == CPULESS_INCR_ADDR) {
		uint8 *addr = (uint8 *) ((dr.wIndex << 16) | dr.wValue);
		int len = dr.wLength;

		/* CPULess access READ primitive */
		if ((p0 = PKTGET(bus->osh, len, TRUE))) {
			/* need to allow access to any address, even 0 */
			hnd_memtrace_enab(FALSE);
			bcopy(addr, PKTDATA(bus->osh, p0), len);
			hnd_memtrace_enab(TRUE);
			PKTSETLEN(bus->osh, p0, len);
		}
	} else {
#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
		if (VUSBD_ENAB(bus)) {
			if (vusbd_setup(bus->vusbd, &dr, p) == -1)
				goto stall;
			else {
				if (dr.bmRequestType & UT_READ)
					bus->setup_resp_pending = TRUE;
				else
					bus->setup_resp_pending = FALSE;
				return p;
			}
		}
#endif /* BCMUSBDEV_COMPOSITE && BCM_VUSBD */
		goto stall;
	}

	if (p0) {
		/* total len of resp pkt is greater than req size; trim it */
		if (pkttotlen(bus->osh, p0) > dr.wLength) {
			void *p1 = p0;
			int totallen = 0;
			for (p1 = p0; p1; p1 = PKTNEXT(bus->osh, p1)) {
				if (totallen >= 0) {
					totallen += PKTLEN(bus->osh, p1);
					if (totallen > dr.wLength) {
						PKTSETLEN(bus->osh, p1, PKTLEN(bus->osh, p1) -
						          (totallen - dr.wLength));
						totallen = -1;
					}
				} else
					PKTSETLEN(bus->osh, p1, 0);
			}
			dbg("ep%d: host req size %d; trim resp size to %d\n", ep, dr.wLength,
				pkttotlen(bus->osh, p0));
		}
		return p0;
	}

	/* No return packet */
	return p;

stall:
	usbdev_pktdump(PKTDATA(bus->osh, p), PKTLEN(bus->osh, p));
	printf("ep%d: unhandled request type 0x%x, bRequest 0x%x, wValue 0x%x\n", ep,
		dr.bmRequestType, dr.bRequest, dr.wValue);
	if (p0)
		PKTFREE(bus->osh, p0, TRUE);

	*errp = 1;
	return NULL;
} /* usbdev_setup */

/** bulk transaction received from host on arbitrary OUT (host->dongle) endpoint. */
void
usbdev_rx(struct dngl_bus *bus, int ep, void *p)
{
	unsigned char *pdata = PKTDATA(bus->osh, p);
	uint32 len = PKTLEN(bus->osh, p);

	trace("");

	/* Control OUT (host->dongle) */
	if (ep == 0) {
		/* handle control OUT data */
		if (TRUE == bus->ctrlout_wrhw) {

			unsigned int cmd = *((unsigned int *) pdata);

			if (len < sizeof(unsigned int)) {
				err("ep%d: pkt tossed len %d", ep, len);
				goto toss;
			}

			dbg("ep%d: Ctrl-out cmd %d; packet len %d\n", ep, cmd, len);
			switch (cmd) {
			case DL_CMD_WRHW:
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
						err("ep%d: invalid WRHW cmd len: %d", ep,
							hwacc->len);
						break;
					}
				} else {
					/* toss other pkts from the ctrl e/p */
					err("ep%d: WRHW invalid pkt len %d", ep, len);
				}
				break;
			default:
				err("ep%d: invalid cmd: %d", ep, cmd);
				break;
			}
			goto toss;

		} else if (bus->wltest && bus->cpuless_waddr != (uint8 *)0xffffffff) {
			int len = PKTLEN(bus->osh, p);

			if (bus->cpuless_wlen == len) {
				/* need to allow access to any address, even 0 */
				hnd_memtrace_enab(FALSE);
				bcopy(PKTDATA(bus->osh, p), bus->cpuless_waddr, len);
				hnd_memtrace_enab(TRUE);
			}
			PKTFREE(bus->osh, p, FALSE);
			bus->cpuless_waddr = (uint8 *)0xffffffff;
			bus->cpuless_wlen = 0;
#ifdef USB_XDCI
		} else if (bus->enable_u1u2_needed) {
			printf("usbdev_rx DL_ENABLE_U1U2 data:%d PKTLEN(bus->osh, p):%d\n",
				(uint8)*pdata, PKTLEN(bus->osh, p));
			bus->enable_u1u2_needed = FALSE;
			if ((uint8)*pdata == 1) {
				printf("usbdev_rx xdci_enable_u1u2(bus->ch, TRUE)\n");
				xdci_enable_u1u2(bus->ch, TRUE);
			} else if ((uint8)*pdata == 0) {
				printf("usbdev_rx xdci_enable_u1u2(bus->ch, FALSE)\n");
				xdci_enable_u1u2(bus->ch, FALSE);
			} else {
				printf("usbdev_rx wrong data\n");
			}
#endif
		} else {
#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
			if (VUSBD_ENAB(bus)) {
				if (vusbd_ctrl_dispatch(bus->vusbd, p)) {
					dbg("ctrl OUT, claimed by vusbd");
					return;
				}
			}
#endif /* BCMUSBDEV_COMPOSITE */
		dngl_ctrldispatch(bus->dngl, p, NULL);
		}

	} else { /* ep != 0 */
		/* Data (bulk) OUT (host->dongle) */
#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
		if (VUSBD_ENAB(bus)) {
			/* check if this pkt belongs to BT */
			if (usbdev_rx_dispatch(bus, ep, p))
				return;
		}
#endif /* defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD) */
		ASSERT(ep == bus->ep_bo[0]);
		if (bus->wltest && bus->loopback)
			usbd_tx(bus, bus->ep_bi[0], p);
		else {
			dngl_sendup(bus->dngl, p); /* forward host pkt towards wireless subsystem */
		}
	}

	trace("done");
	return;

toss:
	PKTFREE(bus->osh, p, FALSE);
} /* usbdev_rx */

/*
 * Generic bus interface routines
 */

/**
 * Called by higher layer when it wants to transmit data to the host. This function queues the
 * caller supplied packet (p) for sending on a BULK IN (dongle->host) endpoint.
 */
int
#ifdef BCMUSBDEV_BMAC
usbdev_bus_tx(struct dngl_bus *bus, void *p, uint32 ep_index)
#else
usbdev_bus_tx(struct dngl_bus *bus, void *p)
#endif /* BCMUSBDEV_BMAC */
{
	int ret = TRUE;

	trace("");
#ifdef BCMUSBDEV_BMAC
#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
	if ((VUSBD_ENAB(bus)) && (ep_index >= 32) && !bus->suspended) {
		/* BT endpoint */
		ret = usbd_tx(bus, vusbd_get_hw_ep(bus->vusbd, ep_index), p);
	} else
#endif /* BCMUSBDEV_COMPOSITE && BCM_VUSBD */
		if (bus->ep_bi[ep_index] && !bus->suspended)
		ret = usbd_tx(bus, bus->ep_bi[ep_index], p);

#else
	if (bus->ep_bi[0] && !bus->suspended) {
		ret = usbd_tx(bus, bus->ep_bi[0], p);
	}
#endif /* BCMUSBDEV_BMAC */
	else if (bus->disconnected || bus->suspended || bus->confignum == 0) {
		/* put it on a backup q until the bus comes back up */
		if (!pktq_full(&bus->txq)) {
			pktenq(&bus->txq, p);
		} else {
			printf("%s: txq full; dropping pkt\n", __FUNCTION__);
			PKTFREE(bus->osh, p, TRUE);
			ret = FALSE;
		}

		if (bus->suspended && !bus->disconnected) {
			if (usbd_resume(bus) != BCME_OK)
				printf("%s: failed to Resume bus\n", __FUNCTION__);
		}
		/* if disconnected, wake host */
		else if (bus->oob_wake && bus->disconnected) {
			bus->wakehost = TRUE;
			if (bus->pmuwake_host) {
				if (!bus->host_pmuwake) {
					usbdev_wakehost(bus, TRUE, FALSE);
					bus->host_pmuwake = TRUE;
				}
			} else
				usbdev_wakehost(bus, FALSE, FALSE);
		}
	} else {
		printf("%s: no tx endpt\n", __FUNCTION__);
		PKTFREE(bus->osh, p, TRUE);
		ret = FALSE;
	}

	return ret;
} /* usbdev_bus_tx */

/**
 * Called when RTE wants to transmit a bus type independent control packet (bdc format) to the host.
 * pkt on interrupt endpoint tells host to query for data on the the control endpoint.
 */
void
usbdev_bus_sendctl(struct dngl_bus *bus, void *p)
{
	trace("");

	/* Q return packet - critical data: don't observe queue limit */
	if (bus->setup_resp_pending) {
	bus->ch_func.tx_resp(bus->ch, 0, p);

		bus->setup_resp_pending = FALSE;
	} else
		pktenq(&bus->ctrlq, p);

	if (bus->use_intr_ep) {
		/* Notify host that data is ready via interrupt endpoint */
		if (!(p = PKTGET(bus->osh, 8, TRUE))) {
#ifdef BCMUSBDEV_COMPOSITE
			err("ep%d: out of txbufs", bus->intr_ep[INT_WLAN]);
#else
			err("ep%d: out of txbufs", bus->intr_ep);
#endif /* BCMUSBDEV_COMPOSITE */
			return;
		}

		bzero((PKTDATA(bus->osh, p)), 8);
		((uint32 *) PKTDATA(bus->osh, p))[0] = htol32(1);
#ifdef BCMUSBDEV_COMPOSITE
		usbd_tx(bus, bus->intr_ep[INT_WLAN], p);
#else
		usbd_tx(bus, bus->intr_ep, p);
#endif /* BCMUSBDEV_COMPOSITE */
	}
}

/**
 * Makes more buffers available for other (non-rx) purposes by not refilling the rx DMA chain in
 * case state==TRUE. Called by higher USB layer.
 *
 * note: parameter 'prio' is unused. Shouldn't this be fixed ?
 */
void
usbdev_bus_rxflowcontrol(struct dngl_bus *bus, bool state, int prio)
{

	bus->ch_func.rxflowcontrol(bus->ch, bus->ep_bo[0], state);
}

/**
 * For test purposes, the user can force the device to wake up using the serial console. This
 * function is called when the RTOS (RTE) calls the 'resume' handler of the attached device.
 */
void
usbdev_bus_resume(struct dngl_bus *bus)
{
	bus->ch_func.resume(bus->ch);
}

/** purge int and ctrl pkts in response to protocol reset msg */
void
usbdev_bus_softreset(struct dngl_bus *bus)
{
	void *p;
	const usb_endpoint_descriptor_t *bcm_cur_endpoints;
	const usb_endpoint_companion_descriptor_t *sscmp = NULL;
#ifdef BCMUSBDEV_COMPOSITE
	int ep_idx = 0;
#endif /* ep_idx */
	trace("usbdev_softreset");

#ifdef BCMUSBDEV_COMPOSITE
	if (bus->interface_num > 1)
		ep_idx = bus->ep_ii_num[INTERFACE_BT];

#ifdef USB_XDCI
	if (bus->speed == UD_USB_SS) {
		bcm_cur_endpoints = &bus->intr_endpoints_ss[ep_idx];
		sscmp = &bus->intr_endpoints_cp_ss[ep_idx];
	} else
#endif /* USB_XDCI */
	if (bus->speed == UD_USB_HS)
		bcm_cur_endpoints = &bus->intr_endpoints[ep_idx];
	else
		bcm_cur_endpoints = &bus->intr_endpoints_fs[ep_idx];
#else
#ifdef USB_XDCI
	if (bus->speed == UD_USB_SS) {
		bcm_cur_endpoints = &bcm_ss_ep_intr;
		sscmp = &bcm_ss_cp_intr;
	} else
#endif /* USB_XDCI */
	if (bus->speed == UD_USB_HS)
		bcm_cur_endpoints = &bcm_hs_ep_intr;
	else
		bcm_cur_endpoints = &bcm_fs_ep_intr;
#endif /* BCMUSBDEV_COMPOSITE */

	/* purge stale ctrl response packets */
	while ((p = pktdeq(&bus->ctrlq)))
		PKTFREE(bus->osh, p, TRUE);

	/* purge associated interrupt packets */
#ifdef BCMUSBDEV_COMPOSITE
	if (bus->use_intr_ep && bus->intr_ep[INT_WLAN]) {
		usbd_ep_detach(bus, bus->intr_ep[INT_WLAN]);
		bus->intr_ep[INT_WLAN] = usbd_ep_attach(bus, bcm_cur_endpoints,
			sscmp,
			bus->bcm_config.bConfigurationValue,
			bus->bcm_interface[bus->int_wlan_idx].bInterfaceNumber,
			bus->bcm_interface[bus->int_wlan_idx].bAlternateSetting);
	}
#else
	if (bus->use_intr_ep && bus->intr_ep) {
		usbd_ep_detach(bus, bus->intr_ep);
		bus->intr_ep = usbd_ep_attach(bus, bcm_cur_endpoints,
			sscmp,
			bus->bcm_config.bConfigurationValue,
			bus->bcm_interface.bInterfaceNumber,
			bus->bcm_interface.bAlternateSetting);
	}
#endif /* BCMUSBDEV_COMPOSITE */
} /* usbdev_bus_softreset */

static void
usbdev_drive_gpio(struct dngl_bus *bus, uint32 gpio, uint32 polarity, bool active, char *name)
{
	if (active) {
		printf("%s->1\n", name);
		si_gpioout(bus->sih, 1 << gpio, !polarity << gpio,
		           GPIO_HI_PRIORITY);
		/* gpioout disable assumes external pull in the active direction */
		si_gpioouten(bus->sih, 1 << gpio, 1 << gpio, GPIO_HI_PRIORITY);
	} else {
		printf("%s->0\n", name);
		si_gpioout(bus->sih, 1 << gpio, polarity << gpio,
		           GPIO_HI_PRIORITY);
		/* pull in the inactive direction */
		si_gpioouten(bus->sih, 1 << gpio, 1 << gpio, GPIO_HI_PRIORITY);
	}
}

/** RTE calls this function to forward an IOVAR to the dongle's USB subsystem */
uint32
usbdev_bus_iovar(struct dngl_bus *bus, char *buf, uint32 inlen, uint32 *outlen, bool set)
{
	char *cmd = buf + 4;
	int offset;
	uint32 val = 0;

	trace("%s: %s", __FUNCTION__, cmd);

	for (offset = 0; offset < inlen; ++offset) {
		if (buf[offset] == '\0')
			break;
	}

	if (buf[offset] != '\0')
		return BCME_BADARG;

	++offset;

	if (set && (offset + sizeof(uint32)) > inlen)
		return BCME_BUFTOOSHORT;

	if (set)
		memcpy(&val, buf + offset, sizeof(uint32));

	if (bus->oob_wake && !strcmp(cmd, "disconnect")) {
		if (set) {
			if (inlen >= 4) {
				/*
				 * host is disconnecting from us;
				 * listen on gpio for reconnect signal
				 */
				if (val == 1 || val == 0)
					bus->pmuwake_host = (bool)val;
				else if (val == 99)
					hnd_die();
				else if (val == 100)
					/* test mode: drive it low */
					usbdev_devready(bus, FALSE, FALSE);
				else if (val == 101)
					/* test mode: drive it high */
					usbdev_devready(bus, TRUE, FALSE);
				else if (val == 102) {
					if (!bus->generictimer_active) {
						bus->generictimer_action = ACTION_INFINITE_LOOP;
						dngl_add_timer(bus->generictimer, 50, FALSE);
						bus->generictimer_active = TRUE;
					} else
						return BCME_NOTREADY;
				} else
					return BCME_BADARG;
			} else
				return BCME_BUFTOOSHORT;
		} else
			val = (uint32)bus->pmuwake_host;
	} else if (bus->oob_wake && !strcmp(cmd, "deviceready")) {
		uint32 gpio = (bus->devrdy_gp & USBGPIO_GPIO_MASK);
		uint32 polarity = (bus->devrdy_gp >> USBGPIO_POLARITYBIT_SHIFT);

		if (set)
			usbdev_drive_gpio(bus, gpio, polarity, val != 0, "deviceready");
		else
			val = ((si_gpioin(bus->sih) & (1 << gpio)) >> gpio) & 0x1;
	} else if (bus->oob_wake && !strcmp(cmd, "hostready")) {
		uint32 gpio = (bus->hostrdy_gp & USBGPIO_GPIO_MASK);
		uint32 polarity = (bus->hostrdy_gp >> USBGPIO_POLARITYBIT_SHIFT);

		if (set)
			usbdev_drive_gpio(bus, gpio, polarity, val != 0, "hostready");
		else
			val = ((si_gpioin(bus->sih) & (1 << gpio)) >> gpio) & 0x1;
	} else if (bus->oob_wake && !strcmp(cmd, "hostwake")) {
		uint32 gpio = (bus->hostwake_gp & USBGPIO_GPIO_MASK);
		uint32 polarity = (bus->hostwake_gp >> USBGPIO_POLARITYBIT_SHIFT);

		if (set)
			usbdev_drive_gpio(bus, gpio, polarity, val != 0, "hostwake");
		else
			val = ((si_gpioin(bus->sih) & (1 << gpio)) >> gpio) & 0x1;
	} else if (bus->oob_wake && !strcmp(cmd, "reconnect")) {
		if (set) {
			if (inlen >= 4) {
				bus->generictimer_action = ACTION_OOB_RECONNECT;
				bus->generictimer_arg = val;
			} else
				return BCME_BUFTOOSHORT;
		} else
			return BCME_BADARG;
	} else if (!strcmp(cmd, "resume")) {
		if (set) {
			if (inlen >= 4) {
				bus->generictimer_action = ACTION_REMOTE_WAKEUP;
				bus->generictimer_arg = val;
			} else
				return BCME_BUFTOOSHORT;
		} else
			return BCME_BADARG;
	} else if (bus->wltest && !strcmp(cmd, "loopback")) {
		if (set) {
			if (inlen >= 4)
				bus->loopback = (bool) val;
			else
				return BCME_BUFTOOSHORT;
		} else
			val = (uint32)bus->loopback;
	}
	/* direct backplane access: may help when JTAG is not available */
	else if (bus->wltest && !strcmp(cmd, "hwacc")) {
		hwacc_t hwacc;
		uint32 *addr;

		val = 0xffffffff;

		if (inlen < sizeof(hwacc_t))
			return BCME_BUFTOOSHORT;

		bcopy(&buf[offset], &hwacc, sizeof(hwacc_t));
		addr = (uint32 *)OSL_UNCACHED(hwacc.addr);
		switch (hwacc.len) {
		case 1:
			if (set)
				*((uint8 *)addr) = (uint8)(hwacc.data & 0xff);
			else
				val = *(uint8 *)addr;
			break;
		case 2:
			if (set)
				*((uint16 *)addr) = (uint16)(hwacc.data & 0xffff);
			else
				val = *(uint16 *)addr;
			break;
		case 4:
			if (set)
				*addr = hwacc.data;
			else
				val = *addr;
			break;
		default:
			err("hwacc: invalid rdhw cmd len: %d", hwacc.len);
			return BCME_BADARG;
		}
	}
	else {
		return BCME_UNSUPPORTED;
	}

	if (!set) {
		memcpy(buf, &val, sizeof(uint32));
		ASSERT(NULL != outlen);
		*outlen = sizeof(uint32);
	}
	return BCME_OK;
} /* usbdev_bus_iovar */

void
usbdev_bus_pr46794WAR(struct dngl_bus *bus)
{
	if (bus->ch_func.usbversion(bus->ch) > UD_USB_2_0_DEV) {
		return;
	}
	ch_pr46794WAR(bus->ch);
}


static void
usbdev_oobreconnect(struct dngl_bus *bus, bool hostrdy)
{
	if (!hostrdy && bus->pmuwake_host) {
		usbdev_wakehost(bus, TRUE, FALSE);
		return;
	}

#ifdef DISCONNECT_FROM_BUS
	/* tell USB to issue CONNECT when it sees Idle on the bus */
	ch_devready(bus->ch, 10 * 1000);
#endif /* DISCONNECT_FROM_BUS */

	/* clear host wake, if necessary */
	if (bus->host_pmuwake)
		usbdev_disablehostwake(bus);
	bus->pmuwake_host = FALSE;
	usbdev_devready(bus, TRUE, FALSE);

	bus->disconnected = FALSE;
}

bool
usbdev_oobresume(struct dngl_bus *bus, bool active)
{
	if (bus->oob_wake && bus->resume_gp != USBGPIO_INVALID) {
		/* resume signaling is done via OOB GPIO */
		uint32 gpio = (bus->resume_gp & USBGPIO_GPIO_MASK);
		uint32 polarity = (bus->resume_gp >> USBGPIO_POLARITYBIT_SHIFT);

		/* only toggle if GPIO is not in desired state */
		if (active) {
			if (!((si_gpioin(bus->sih) & (1 << gpio)) >> gpio)) {
				printf("res->1\n");
				si_gpioout(bus->sih, 1 << gpio, !polarity << gpio,
				           GPIO_HI_PRIORITY);
				si_gpioouten(bus->sih, 1 << gpio, 1 << gpio, GPIO_HI_PRIORITY);
			}
		} else {
			if ((si_gpioin(bus->sih) & (1 << gpio)) >> gpio) {
				printf("res->0\n");
				si_gpioout(bus->sih, 1 << gpio, polarity << gpio,
				           GPIO_HI_PRIORITY);
				/* assumes a weak pull in the inactive direction */
				si_gpioouten(bus->sih, 1 << gpio, 0, GPIO_HI_PRIORITY);
			}
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

/* interrupt handler for "host ready" gpio event */
static void
usbdev_hostrdy(uint32 stat, void *arg)
{
	struct dngl_bus *bus = (struct dngl_bus *) arg;
	uint32 gpio = (bus->hostrdy_gp & USBGPIO_GPIO_MASK);
	bool hostrdy = (si_gpioin(bus->sih) & (1 << gpio)) >> gpio;

	trace("");

	if (bus->disconnected && hostrdy) {
		printf("hr->1\n");
		usbdev_oobreconnect(bus, TRUE);
	} else if (!bus->disconnected && !hostrdy) {
		printf("hr->0\n");

		/* make sure clocks are up so Reset is detected */
		ch_leave_suspend_wrapper(bus->ch);

		/* drive "device ready" inactive */
		usbdev_devready(bus, FALSE, FALSE);

		bus->disconnected = TRUE;
		bus->disconnect_upon_reset = 1;

		/* cancel OOB resume signaling, if necessary */
		usbdev_oobresume(bus, FALSE);

		/* test mode: host requested OOB reconnect after specified time */
		if (bus->generictimer_action == ACTION_OOB_RECONNECT) {
			dngl_add_timer(bus->generictimer, bus->generictimer_arg, FALSE);
			bus->generictimer_active = TRUE;
		}
	} else {
		printf("!!! dis %d; hr %d\n", bus->disconnected, hostrdy);
	}

	/* make polarity opposite the current value */
	si_gpiointpolarity(bus->sih, 1 << gpio, (hostrdy << gpio), 0);
} /* usbdev_hostrdy */

#ifdef OOB_WAKE
bool
usbdev_oob_connected(struct dngl_bus *bus)
{
	uint32 gpio = (bus->hostrdy_gp & USBGPIO_GPIO_MASK);

	if (!bus->oob_wake || bus->hostrdy_gp == USBGPIO_INVALID)
		return TRUE;

	return (si_gpioin(bus->sih) & (1 << gpio)) >> gpio;
}
#endif /* OOB_WAKE */

#ifdef DISCONNECT_FROM_BUS
static void
usbdev_disconnect(struct dngl_bus *bus)
{
	trace("");
	/* bus-level disconnect (disable USB/HSIC core) */
	ch_disconnect(bus->ch);
}
#endif /* DISCONNECT_FROM_BUS */

static void
usbdev_gptimer(dngl_timer_t *t)
{
	struct dngl_bus *bus = (struct dngl_bus *)hnd_timer_get_ctx(t);
	uint32 gpio = (bus->hostwake_gp & USBGPIO_GPIO_MASK);

	/* Toggle the bit and set the appropriate timer value */
	if (bus->gpon) {
		dbg("hw0 ");
		si_gpioout(bus->sih, 1 << gpio,
		           (bus->hostwake_gp >> USBGPIO_POLARITYBIT_SHIFT) << gpio,
		           GPIO_HI_PRIORITY);
		dngl_add_timer(bus->gptimer, bus->gpofftime, FALSE);
		bus->gpon = FALSE;
	} else {
		dbg("hw1 ");
		si_gpioout(bus->sih, 1 << gpio,
		           !(bus->hostwake_gp >> USBGPIO_POLARITYBIT_SHIFT) << gpio,
		           GPIO_HI_PRIORITY);
		dngl_add_timer(bus->gptimer, bus->gpontime, FALSE);
		bus->gpon = TRUE;
	}
}

void
usbdev_devready(struct dngl_bus *bus, bool active, bool called_from_trap)
{
	uint32 hr_gpio = (1 << (bus->hostrdy_gp & USBGPIO_GPIO_MASK));

	trace("");

	/* tell the host the device is ready to connect */
	if (bus->devrdy_gp != USBGPIO_INVALID) {
		uint32 gpio = (bus->devrdy_gp & USBGPIO_GPIO_MASK);
		uint32 polarity = (bus->devrdy_gp >> USBGPIO_POLARITYBIT_SHIFT);

		if (active) {
			if (bus->devrdy_state == 1)
				return;
			dbg("dr->1\n");
			si_gpioout(bus->sih, 1 << gpio, !polarity << gpio,
			           GPIO_HI_PRIORITY);
			/* gpioout disable assumes external pull in the active direction */
			si_gpioouten(bus->sih, 1 << gpio, 0, GPIO_HI_PRIORITY);
			bus->devrdy_state = 1;
			if (bus->devrdytimer_active) {
				dngl_del_timer(bus->devrdytimer);
				bus->devrdytimer_active = FALSE;
			}

			if (called_from_trap && !(si_gpioin(bus->sih) & hr_gpio)) {
				/* host has disabled the port: spinwait for host to reenable it */
				while (!(si_gpioin(bus->sih) & hr_gpio))
					OSL_DELAY(100);
			}
			return;
		}

		/* need to wake the host via its pmu? */
		if (called_from_trap && bus->pmuwake_host && !(si_gpioin(bus->sih) & hr_gpio)) {
				bool on = TRUE;
				uint32 polarity;
				uint32 hw_gpio = (bus->hostwake_gp & USBGPIO_GPIO_MASK);

				/* host is asleep: do pmu wake & spinwait for host to wake up */
				usbdev_wakehost(bus, TRUE, TRUE);
				while (!(si_gpioin(bus->sih) & hr_gpio)) {
					if (bus->gpontime) {
						/* drive signal on/off while testing HOST_READY */
						SPINWAIT(!(si_gpioin(bus->sih) & hr_gpio),
						         on ? (bus->gpontime * 1000) :
						         (bus->gpofftime * 1000));
						if (on)
							polarity = (bus->hostwake_gp >>
							            USBGPIO_POLARITYBIT_SHIFT)
							        << hw_gpio;
						else
							polarity = !(bus->hostwake_gp >>
							             USBGPIO_POLARITYBIT_SHIFT)
							        << hw_gpio;
						si_gpioout(bus->sih, 1 << hw_gpio,
						           polarity, GPIO_HI_PRIORITY);
						on = !on;
					} else
						SPINWAIT(!(si_gpioin(bus->sih) & hr_gpio), 3000);
				}
				usbdev_disablehostwake(bus);
			}

		if (bus->devrdy_state == 0 && !called_from_trap)
			return;

		dbg("dr->0\n");
		si_gpioout(bus->sih, 1 << gpio, polarity << gpio,
		           GPIO_HI_PRIORITY);
		/* pull in the inactive direction */
		si_gpioouten(bus->sih, 1 << gpio, 1 << gpio, GPIO_HI_PRIORITY);
		bus->devrdy_state = 0;
		if (bus->devrdydelay) {
			if (called_from_trap) {
				if (!bus->pmuwake_host) {
					/* host is awake: just re-enumerate with standard delay */
					OSL_DELAY(bus->devrdydelay * 1000);
				}
			} else {
				if (bus->devrdytimer_active)
					dngl_del_timer(bus->devrdytimer);
				dngl_add_timer(bus->devrdytimer, bus->devrdydelay, FALSE);
				bus->devrdytimer_active = TRUE;
			}
		}
		/* immediately exit disconnected state if there are any packets pending */
		if (ch_txpending(bus->ch))
			usbdev_wakehost(bus, bus->pmuwake_host, FALSE);
	}
} /* usbdev_devready */

static void
usbdev_wakehost(struct dngl_bus *bus, bool pmu_wake, bool called_from_trap)
{
	trace("");

	if (pmu_wake) {
		uint32 gpio = (bus->hostwake_gp & USBGPIO_GPIO_MASK);

		/* If the duty-cycle timer is already running, nothing more to do */
		if (bus->gptimer_active && !called_from_trap)
			return;

		/* remove weak pull in the inactive direction */
		si_gpiopull(bus->sih, !(bus->hostwake_gp >> USBGPIO_POLARITYBIT_SHIFT),
		            1 << gpio, 0);

		/* Drive "host wake" GPIO active */
		(void) si_gpioout(bus->sih, 1 << gpio,
		                 !(bus->hostwake_gp >> USBGPIO_POLARITYBIT_SHIFT) << gpio,
		                 GPIO_HI_PRIORITY);
		(void) si_gpioouten(bus->sih, 1 << gpio, 1 << gpio, GPIO_HI_PRIORITY);

		bus->gpon = TRUE;

		/* If we have toggling configured, start the duty-cycle timer */
		if (bus->gpontime && !called_from_trap) {
			dngl_add_timer(bus->gptimer, bus->gpontime, FALSE);
			bus->gptimer_active = TRUE;
		}
	} else if (bus->devrdy_state == 0) {
		if (bus->devrdytimer_active)
			return;
		usbdev_devready(bus, TRUE, FALSE);
	}

	bus->wakehost = FALSE;
} /* usbdev_wakehost */

static void
usbdev_disablehostwake(struct dngl_bus *bus)
{
	/* deassert host wake gpio */
	uint32 gpio = (bus->hostwake_gp & USBGPIO_GPIO_MASK);

	trace("");

	bus->gpon = FALSE;
	if (bus->gptimer_active) {
		dngl_del_timer(bus->gptimer);
		bus->gptimer_active = FALSE;
	}

	/* Drive "host wake" GPIO output inactive */
	si_gpioout(bus->sih, 1 << gpio,
	           (bus->hostwake_gp >> USBGPIO_POLARITYBIT_SHIFT) << gpio,
	           GPIO_HI_PRIORITY);

	/* add a weak pull in the inactive direction */
	si_gpiopull(bus->sih, !(bus->hostwake_gp >> USBGPIO_POLARITYBIT_SHIFT),
	            1 << gpio, 1 << gpio);

	si_gpioouten(bus->sih, 1 << gpio, 0, GPIO_HI_PRIORITY);

	bus->host_pmuwake = FALSE;
}

/* must wait at least some number of ms before it's ok to drive DEVICE_READY active */
static void
usbdev_devrdytimer(dngl_timer_t *t)
{
	struct dngl_bus *bus = (struct dngl_bus *)hnd_timer_get_ctx(t);

	bus->devrdytimer_active = FALSE;

	if (bus->wakehost)
		usbdev_wakehost(bus, FALSE, FALSE);
}

#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
/*
 * return 1 if this pkt belongs to BT
 * return 0 if this pkt belongs to WLAN
 */
int usbdev_rx_dispatch(struct dngl_bus *bus, int ep, void *p)
{
	if (bus->vusbd->bt_dl_inprogress) {
		vusbd_recv(bus->vusbd, ep, p);
		return 1;
	}
	if (bus->ep_bo_num[INTERFACE_BT] && (ep == bus->ep_bo_BT)) {
		vusbd_recv(bus->vusbd, ep, p);
		/* ch_rxfill(bus->vusbd->usbdev, EP2DMA(ep)); */
		return 1;
	}
	if (bus->ep_iso_num[INTERFACE_ISO] && (ep == bus->ep_iso)) {
		vusbd_recv(bus->vusbd, ep, p);
		/* ch_rxfill(bus->vusbd->usbdev, EP2DMA(ep)); */
		return 1;
	}
	return 0;
}
#endif /* BCMUSBDEV_COMPOSITE  && BCM_VUSBD */

#ifdef USB_XDCI
void usbdev_attach_xdci(struct dngl_bus *bus)
{
	int i, sscmp_len;
	int ep_sum;

	/* Initialize descriptors */
	i = usbd_chk_version(bus);
	if (i == UD_USB_3_DEV) {
		get_bcm_device()->bcdUSB = UD_USB_3_0;
		get_bcm_device()->bMaxPacketSize = USB_3_MAX_CTRL_PACKET_FACTORIAL;
	} else {
		return;
	}

	get_bcm_config()->bMaxPower = (uByte)DEV_MAXPOWER_900;
	get_bcm_other_config()->bMaxPower = (uByte)DEV_MAXPOWER_900;

	bus->bcm_config.bMaxPower = (uByte)DEV_MAXPOWER_900;
	bus->bcm_other_config.bMaxPower = (uByte)DEV_MAXPOWER_900;

	ep_sum = 0;
#ifdef BCMUSBDEV_COMPOSITE
	for (i = 0; i < bus->interface_num; i++) {
		get_bcm_interface()[i].bNumEndpoints = bus->ep_ii_num[i]
			+ bus->ep_bi_num[i] + bus->ep_bo_num[i]
			+ bus->ep_isi_num[i] + bus->ep_iso_num[i];
		ep_sum += get_bcm_interface()[i].bNumEndpoints;
	}
#else
	ep_sum = bus->bcm_interface.bNumEndpoints;
#endif

	sscmp_len = USB_ENDPOINT_COMPANION_DESCRIPTOR_SIZE * ep_sum;

#ifdef BCMUSBDEV_COMPOSITE
	get_bcm_config()->wTotalLength += sscmp_len;
	get_bcm_other_config()->wTotalLength += sscmp_len;
#endif

	bus->bcm_config.wTotalLength += sscmp_len;
	bus->bcm_other_config.wTotalLength += sscmp_len;
} /* usbdev_attach_xdci */

void usbdev_detach_xdci(struct dngl_bus *bus)
{
	int i, sscmp_len;
	int ep_sum;

	get_bcm_device()->bcdUSB = UD_USB_2_1;
	get_bcm_device()->bMaxPacketSize = USB_2_MAX_CTRL_PACKET;

	/* Initialize descriptors */
	i = usbd_chk_version(bus);
	if (i != UD_USB_3_DEV) {
		return;
	}

	ep_sum = 0;
#ifdef BCMUSBDEV_COMPOSITE
	for (i = 0; i < bus->interface_num; i++) {
		get_bcm_interface()[i].bNumEndpoints = bus->ep_ii_num[i]
			+ bus->ep_bi_num[i] + bus->ep_bo_num[i]
			+ bus->ep_isi_num[i] + bus->ep_iso_num[i];
		ep_sum += get_bcm_interface()[i].bNumEndpoints;
	}
#else
	ep_sum = bus->bcm_interface.bNumEndpoints;
#endif

	sscmp_len = USB_ENDPOINT_COMPANION_DESCRIPTOR_SIZE * ep_sum;

#ifdef BCMUSBDEV_COMPOSITE
	get_bcm_config()->wTotalLength -= sscmp_len;
	get_bcm_other_config()->wTotalLength -= sscmp_len;
#endif

	bus->bcm_config.wTotalLength -= sscmp_len;
	bus->bcm_other_config.wTotalLength -= sscmp_len;
} /* usbdev_detach_xdci */

#endif /* USB_XDCI */


#ifdef BCMUSBDEV_COMPOSITE
void usbd_stall_ep(struct dngl_bus *bus, int ep)
{
	bus->ch_func.stall_ep(bus->ch, ep);
}
#endif /* BCMUSBDEV_COMPOSITE */


/** generic timer for doing misc stuff after a delay */
static void
usbdev_generictimer(dngl_timer_t *t)
{
	struct dngl_bus *bus = (struct dngl_bus *)hnd_timer_get_ctx(t);

	bus->generictimer_active = FALSE;
	switch (bus->generictimer_action) {
	case ACTION_TRIGGER_TRAP:
		/* test mode: induce switch to CPULess mode via trap */
		hnd_die();
	case ACTION_INFINITE_LOOP:
		/* test mode: go into infinite loop */
		while (1);
	case ACTION_OOB_RECONNECT:
		usbdev_oobreconnect(bus, FALSE);
		break;
	case ACTION_REMOTE_WAKEUP:
		ch_resume(bus->ch);
		break;
	case ACTION_OOB_DISCONNECT:
#ifdef DISCONNECT_FROM_BUS
		usbdev_disconnect(bus);
#endif
		break;
	default:
		printf("%s: expired with unhandled action %d\n", __FUNCTION__,
		       bus->generictimer_action);
		break;
	}
	bus->generictimer_action = ACTION_UNDEFINED;
}
