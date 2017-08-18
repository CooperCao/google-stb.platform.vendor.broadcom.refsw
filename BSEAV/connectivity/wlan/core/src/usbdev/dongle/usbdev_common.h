/*
 * USB common data structure shared by bootloader and firmware
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id$
 */

#define __need_wchar_t
#include <stddef.h>
#ifdef ZLIB
#include <zlib.h>
#include <zutil.h>
#endif
#include <typedefs.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <osl.h>
#include <trxhdr.h>
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
#include <usbrdl.h>


static inline int usbd_chk_version(struct dngl_bus *bus);
static inline bool usbd_hsic(struct dngl_bus *bus);
static inline int usbd_resume(struct dngl_bus *bus);
static inline uint usbd_ep_attach(struct dngl_bus *bus, const usb_endpoint_descriptor_t *endpoint,
        const usb_endpoint_companion_descriptor_t *sscmp,
        int config, int interface, int alternate);
static inline void usbd_ep_detach(struct dngl_bus *bus, int ep);


#define EP_BULK_MAX     4
#define ACTION_REMOTE_WAKEUP    1

#ifdef BCMUSBDEV_COMPOSITE
#include <vusbd_pub.h>
#include <vusbd.h>
#define INTERFACE_BT    0       /* default BT interface 0 */
#define INTERFACE_ISO   1       /* BT/ISO interface 1 */
#define INTERFACE_DFU   2       /* BT/DFU interface 2 */
#define INTERFACE_MAX   9
#define ALT_SETTING_MAX 5       /* alternate settings for BT ISO interface */
#define IAD_INT_NUM     0       /* IAD for grouping BT interface 0 onwards */
#endif /* BCMUSBDEV_COMPOSITE */

/* default values for config descriptors */
#define DEV_ATTRIBUTES (UC_BUS_POWERED | UC_REMOTE_WAKEUP)
#define DEV_MAXPOWER (200 / UC_POWER_FACTOR)
#define DEV_MAXPOWER_500 (500 / UC_POWER_FACTOR)
#define DEV_MAXPOWER_900 (900 / UC_SSPOWER_FACTOR)


#define RDL_CONFIGLEN           (USB_CONFIG_DESCRIPTOR_SIZE + \
				USB_INTERFACE_DESCRIPTOR_SIZE + \
				(USB_ENDPOINT_DESCRIPTOR_SIZE * 3))

#ifndef BCMUSBDEV_COMPOSITE
#define INT_WLAN  0       /* default WLAN interface */
#else
#define INT_WLAN   (bus->int_wlan) /* WLAN interface on Composite Device */
#endif /*  BCMUSBDEV_COMPOSITE */


struct dngl_bus {
	osl_t *osh;			/* os handle */
	struct usbdev_chip *ch;		/* Chip handle */
	void *dngl;			/* dongle handle */
	si_t *sih;			/* sonics bus handle */
	struct pktq ctrlq;	/* to send and receive USB packets on endpoint 0 (setup/control) */
	usb_device_descriptor_t device; /* specific item used in bootloader */
	uint speed;
	bool ctrlout_wrhw;
	bool setup_resp_pending;	/* BMAC Control IN wait state */
	bool loopback;			/* 1=loop back packets to the host */
	bool disconnected;		/* 1=host shut down its controller */
	bool host_pmuwake;		/* 1=device asserted "host pmu wake" */
	bool wakehost;	/* 1=got packet; wake host (facilitates delayed DEVICE_READY signaling) */
	bool gpon;			/* 1="host wake" signal is in active state */
	bool gptimer_active;		/* 1=gpio timer is running */
	bool suspended;			/* 1=device is in Suspend state */
	bool devrdy_state;		/* 1=devrdy asserted, 0=devrdy de-asserted */
	bool devrdytimer_active;	/* 1=devrdy delay timer is running */
	bool pmuwake_host;		/* 1=wake host via pmu on packet indication */
	uint16 gpontime;		/* On time for duty cycle when toggling gpio */
	uint16 gpofftime;		/* Off time for duty cycle when toggling gpio */
	dngl_timer_t *gptimer;		/* Timer for toggling gpio output */
	uint32 devrdy_gp;		/* OOB "device ready" gpio information */
	uint32 hostrdy_gp;		/* OOB "host ready" gpio information */
	uint32 hostwake_gp;		/* OOB "host wake" gpio information */
	usbdev_gpioh_t *hostrdy_h;	/* handle to "host ready" interrupt object */
	dngl_timer_t *devrdytimer;	/* Timer for delay asserting DEVICE_READY */
	struct pktq txq;		/* backup q if device is Suspend'd or disconnected */
	uint16 devrdydelay;		/* ms delay asserting DEVICE_READY again */
	bool use_intr_ep;		/* 1=use interrupt ep */
	bool wltest;			/* 1=firmware was built for mfgtest */
	bool oob_wake;			/* 1=firmware was built for OOB signaling */
	uint8 *cpuless_waddr;		/* address at which to issue a CPULess access */
	int cpuless_wlen;		/* length of CPULess write */
	int enable_u1u2_needed;
#ifdef BCM_FD_AGGR
	bool fdaggr;			/* 1=aggregation enabled */
#endif
	int confignum;
	usb_config_descriptor_t bcm_config;
	usb_config_descriptor_t bcm_other_config;
	int interface;
	uint ep_bi[EP_BULK_MAX];	/* bulk in  (dongle->host) */
	uint ep_bo[EP_BULK_MAX];	/* bulk out (host->dongle) */
#ifdef BCMUSBDEV_COMPOSITE
#ifdef USB_IFTEST
	bool ep_config_valid;
#endif /* USB_IFTEST */
	usb_string_descriptor_t strings[8];
	usb_interface_association_descriptor_t* iad;
	usb_interface_descriptor_t* bcm_interface;
	uint intr_ep[INTERFACE_MAX];		/* interrupt in */
	uint ep_ii_num[INTERFACE_MAX];		/* intr ep per interface */
	uint ep_bi_num[INTERFACE_MAX];		/* bulk-in (dongle->host) ep per interface */
	uint ep_bo_num[INTERFACE_MAX];		/* bulk-out (host->dongle) ep per interface */
	uint ep_isi;				/* iso-in ep hw address */
	uint ep_isi_num[INTERFACE_MAX];		/* iso-in ep per interface */
	uint ep_iso;				/* iso-out ep hw address */
	uint ep_iso_num[INTERFACE_MAX];		/* iso-out ep per interface */
#ifdef USB_XDCI
	usb_endpoint_descriptor_t data_endpoints_ss[EP_BULK_MAX*INTERFACE_MAX];
	usb_endpoint_companion_descriptor_t data_endpoints_cp_ss[EP_BULK_MAX*INTERFACE_MAX];
	usb_endpoint_descriptor_t intr_endpoints_ss[INTERFACE_MAX];
	usb_endpoint_companion_descriptor_t intr_endpoints_cp_ss[INTERFACE_MAX];
#endif /* USB_XDCI */

	/* high speed endpoints */
	usb_endpoint_descriptor_t data_endpoints[EP_BULK_MAX*INTERFACE_MAX];
	/* full speed endpoints */
	usb_endpoint_descriptor_t data_endpoints_fs[EP_BULK_MAX*INTERFACE_MAX];
	uint interface_num;			/* interface count */
	usb_endpoint_descriptor_t intr_endpoints[INTERFACE_MAX];
	usb_endpoint_descriptor_t intr_endpoints_fs[INTERFACE_MAX];
	int int_wlan;			/* WLAN interface number used for Enumeration */
	int int_wlan_idx;		/* WLAN index in interface array */
#else
	usb_string_descriptor_t strings[4];
	usb_interface_descriptor_t bcm_interface;
	uint ep_ii_num;                 /* interrupt In */
	uint intr_ep;
	uint ep_bi_num; /* bulk in  (dongle->host) */
	uint ep_bo_num; /* bulk out (host->dongle) */
#ifdef USB_XDCI
	usb_endpoint_descriptor_t data_endpoints_ss[EP_BULK_MAX*2]; /* super speed endpoints */
	usb_endpoint_companion_descriptor_t data_endpoints_cp_ss[EP_BULK_MAX*2];
#endif /* USB_XDCI */
	usb_endpoint_descriptor_t data_endpoints[EP_BULK_MAX*2]; 	/* high speed endpoints */
	usb_endpoint_descriptor_t data_endpoints_fs[EP_BULK_MAX*2];     /* full speed endpoints */
#endif /* BCMUSBDEV_COMPOSITE */
	bool usb30d;
	uint32 resume_gp;		/* OOB "remote wake-up" gpio information */
	dngl_timer_t *generictimer;	/* Timer for delay of misc operations */
	uint32 generictimer_action;	/* generic timer action */
	uint32 generictimer_arg;	/* argument for generic timer actions */
	bool generictimer_active;	/* 1=generic delay timer is running */
	bool disconnect_upon_reset;	/* 1=wait for Reset before disconnecting from bus */
#ifdef BCMUSBDEV_COMPOSITE
	bool vusbd_enab;
	uint ep_bi_BT;				/* BT bulk in ep hw addr */
	uint ep_bo_BT;				/* BT bulk out ep hw addr */
	struct vusbd *vusbd;
#endif
	struct usbchip_func {
		/* Interface function call table to usb chip */
		int (*tx)(struct usbdev_chip* ch, int ep_index, void *p);
		int (*tx_resp)(struct usbdev_chip* ch, int ep_index, void *p);
		void (*rxflowcontrol)(struct usbdev_chip* ch, int ep_index, bool state);
		int (*resume)(struct usbdev_chip* ch);
		void (*stall_ep)(struct usbdev_chip* ch, int ep_index);
		int (*usbversion)(struct usbdev_chip* ch);
		bool (*hsic)(struct usbdev_chip* ch);
		uint32 (*ep_attach)(struct usbdev_chip* ch,
			const usb_endpoint_descriptor_t *endpoint,
			const usb_endpoint_companion_descriptor_t *sscmp,
			int config, int interface, int alternate);
		void (*ep_detach)(struct usbdev_chip* ch, int ep_index);
	} ch_func;
	/* Following memeber is bootloader specific */
};

/*
 * BDC personality: upto 4 interfaces:
 *   1 WLAN, upto 3 BT interfaces
 */
static usb_device_descriptor_t bcm_device = {
	bLength: USB_DEVICE_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_DEVICE,
	bcdUSB: UD_USB_2_0,
	bDeviceClass: UDCLASS_VENDOR,
	bDeviceSubClass: 0,
	bDeviceProtocol: 0,
	bMaxPacketSize: USB_2_MAX_CTRL_PACKET,
	idVendor: BCM_DNGL_VID,
	idProduct: BCM_DNGL_BL_PID,
	bcdDevice: 0x0001,
	iManufacturer: 1,
	iProduct: 2,
	iSerialNumber: 3,
	bNumConfigurations: 1
};

static usb_config_descriptor_t bcm_config = {
	bLength: USB_CONFIG_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_CONFIG,
	bNumInterface: 1,
	bConfigurationValue: 1,
	iConfiguration: 0,
	bmAttributes: DEV_ATTRIBUTES,
	bMaxPower: DEV_MAXPOWER
};


static usb_config_descriptor_t bcm_other_config = {
	bLength: USB_CONFIG_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_OTHER_SPEED_CONFIGURATION,
	bNumInterface: 1,
	bConfigurationValue: 1,
	iConfiguration: 0,
	bmAttributes: DEV_ATTRIBUTES,
	bMaxPower: DEV_MAXPOWER
};


#ifdef BCMUSBDEV_COMPOSITE
static usb_interface_association_descriptor_t bcm_iad = {
	bLength: USB_INTERFACE_ASSOCIATION_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_IAD,
	bFirstInterface: IAD_INT_NUM,
	bInterfaceCount: 0,
	bFunctionClass: UICLASS_WIRELESS,
	bFunctionSubClass: UISUBCLASS_RF,
	bFunctionProtocol: UIPROTO_BLUETOOTH,
	iFunction: 5
};

static usb_interface_descriptor_t bcm_interfaces[] = {
	{
		bLength: USB_INTERFACE_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_INTERFACE,
		bInterfaceNumber: 0,
		bAlternateSetting: 0,
		bInterfaceClass: UICLASS_WIRELESS,
		bInterfaceSubClass: UISUBCLASS_RF,
		bInterfaceProtocol: UIPROTO_BLUETOOTH,
		iInterface: 6
	},
	{
		bLength: USB_INTERFACE_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_INTERFACE,
		bInterfaceNumber: 1,
		bAlternateSetting: 0,
		bInterfaceClass: UICLASS_WIRELESS,
		bInterfaceSubClass: UISUBCLASS_RF,
		bInterfaceProtocol: UIPROTO_BLUETOOTH,
		iInterface: 6
	},
	{
		bLength: USB_INTERFACE_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_INTERFACE,
		bInterfaceNumber: 1,
		bAlternateSetting: 1,
		bInterfaceClass: UICLASS_WIRELESS,
		bInterfaceSubClass: UISUBCLASS_RF,
		bInterfaceProtocol: UIPROTO_BLUETOOTH,
		iInterface: 6
	},
	{
		bLength: USB_INTERFACE_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_INTERFACE,
		bInterfaceNumber: 1,
		bAlternateSetting: 2,
		bInterfaceClass: UICLASS_WIRELESS,
		bInterfaceSubClass: UISUBCLASS_RF,
		bInterfaceProtocol: UIPROTO_BLUETOOTH,
		iInterface: 6
	},
	{
		bLength: USB_INTERFACE_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_INTERFACE,
		bInterfaceNumber: 1,
		bAlternateSetting: 3,
		bInterfaceClass: UICLASS_WIRELESS,
		bInterfaceSubClass: UISUBCLASS_RF,
		bInterfaceProtocol: UIPROTO_BLUETOOTH,
		iInterface: 6
	},
	{
		bLength: USB_INTERFACE_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_INTERFACE,
		bInterfaceNumber: 1,
		bAlternateSetting: 4,
		bInterfaceClass: UICLASS_WIRELESS,
		bInterfaceSubClass: UISUBCLASS_RF,
		bInterfaceProtocol: UIPROTO_BLUETOOTH,
		iInterface: 6
	},
	{
		bLength: USB_INTERFACE_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_INTERFACE,
		bInterfaceNumber: 1,
		bAlternateSetting: 5,
		bInterfaceClass: UICLASS_WIRELESS,
		bInterfaceSubClass: UISUBCLASS_RF,
		bInterfaceProtocol: UIPROTO_BLUETOOTH,
		iInterface: 6
	},
	{
		bLength: USB_INTERFACE_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_INTERFACE,
		bInterfaceNumber: 2,
		bAlternateSetting: 0,
		bInterfaceClass: UICLASS_APPL_SPEC,
		bInterfaceSubClass: UISUBCLASS_FIRMWARE_DOWNLOAD,
		bInterfaceProtocol: UIPROTO_DFU,
		iInterface: 7
	},
	{
		bLength: USB_INTERFACE_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_INTERFACE,
		bInterfaceNumber: 3,
		bAlternateSetting: 0,
		bInterfaceClass: UICLASS_VENDOR,
		bInterfaceSubClass: UISUBCLASS_ABSTRACT_CONTROL_MODEL,
		bInterfaceProtocol: UIPROTO_DATA_VENDOR,
		iInterface: 2
	}
};
#else
static const usb_interface_descriptor_t bcm_interfaces[] = {
	{
		bLength: USB_INTERFACE_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_INTERFACE,
		bInterfaceNumber: 0,
		bAlternateSetting: 0,
		bInterfaceClass: UICLASS_VENDOR,
		bInterfaceSubClass: UISUBCLASS_ABSTRACT_CONTROL_MODEL,
		bInterfaceProtocol: 0xff,
		iInterface: 0
	}
};
#endif /* BCMUSBDEV_COMPOSITE */

#ifdef USB_XDCI
/* SS mode supports 4 types of descriptor: device, configuration, BOS and string */
static const usb_bos_descriptor_t bcm_bos_desc = {
		bLength: USB_BOS_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_BOS,
		wTotalLength: USB_BOS_DESCRIPTOR_LENGTH,
		bNumDeviceCaps: 0x02
};

static const usb_2_extension_descriptor_t bcm_usb2_ext = {
		bLength: USB_2_EXTENSION_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_CAP,
		bDevCapabilityType: 0x02,
#if defined(XDCI_LPM_ENA)
		bmAttributes: 0x02
#else
		bmAttributes: 0x00
#endif /* XDCI_LPM_ENA */
};

static const usb_ss_device_capability_descriptor_t bcm_ss_cap = {
		bLength: USB_SS_DEVICE_CAPABILITY_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_CAP,
		bDevCapabilityType: 0x03,
		bmAttributes: 0x00,
		wSpeedsSupport: 0x0e,  /* supoort SS/HS/FS speed */
		bFunctionalitySupport: 0x01,
		bU1DevExitLat: 0x0a,
		wU2DevExitLat: 0x07FF
};

/**
 * SS mode reports the other speed it support via the BOS descriptor and shall not support the
 * device_qualifier and other_speed_configuration descriptor
 */
static const usb_device_qualifier_t bcm_ss_qualifier = {
	bLength: USB_DEVICE_QUALIFIER_SIZE,
	bDescriptorType: UDESC_DEVICE_QUALIFIER,
	bcdUSB: UD_USB_3_0,
	bDeviceClass: UDCLASS_VENDOR,
	bDeviceSubClass: 0,
	bDeviceProtocol: 0,
	bMaxPacketSize0: USB_2_MAX_CTRL_PACKET,
	bNumConfigurations: 1,
	bReserved: 0
};

static const usb_endpoint_descriptor_t bcm_ss_ep_intr = {
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_IN | 1, /* dongle -> host */
		bmAttributes: UE_INTERRUPT,
		wMaxPacketSize: 64,
		bInterval: 4
	};

static const usb_endpoint_companion_descriptor_t bcm_ss_cp_intr = {
		bLength: USB_ENDPOINT_COMPANION_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT_COMPANION,
		bMaxBurst: 0,
		bmAttributes: 0,
		wBytesPerInterVal: 64,
	};

static const usb_endpoint_descriptor_t bcm_ss_ep_bulkin = {
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_IN | 2, /* dongle -> host */
		bmAttributes: UE_BULK,
		wMaxPacketSize: USB_3_MAX_BULK_PACKET,
		bInterval: 0
	};

static const usb_endpoint_companion_descriptor_t bcm_ss_cp_bulkin = {
		bLength: USB_ENDPOINT_COMPANION_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT_COMPANION,
		bMaxBurst: 1,
		bmAttributes: 0,
		wBytesPerInterVal: 0,
	};

static const usb_endpoint_descriptor_t bcm_ss_ep_bulkout = {
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_OUT | 3, /* host -> dongle */
		bmAttributes: UE_BULK,
		wMaxPacketSize: USB_3_MAX_BULK_PACKET,
		bInterval: 0
	};

static const usb_endpoint_companion_descriptor_t bcm_ss_cp_bulkout = {
		bLength: USB_ENDPOINT_COMPANION_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT_COMPANION,
		bMaxBurst: 1,
		bmAttributes: 0,
		wBytesPerInterVal: 0,
	};

#endif /* USB_XDCI */

static usb_endpoint_descriptor_t bcm_hs_ep_intr = {
	bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_ENDPOINT,
	bEndpointAddress: UE_DIR_IN | 1,	/* dongle -> host */
	bmAttributes: UE_INTERRUPT,
	wMaxPacketSize: USB_2_MAX_INTR_PACKET,
	bInterval: 4
        };

static const usb_endpoint_descriptor_t bcm_hs_ep_bulkin = {
	bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_ENDPOINT,
	bEndpointAddress: UE_DIR_IN | 2,	/* dongle -> host */
	bmAttributes: UE_BULK,
	wMaxPacketSize: USB_2_MAX_BULK_PACKET,
	bInterval: 0
        };

static usb_endpoint_descriptor_t bcm_hs_ep_bulkout = {
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_OUT | 3,
		bmAttributes: UE_BULK,
		wMaxPacketSize: 512,
		bInterval: 1
};

static usb_endpoint_descriptor_t bcm_fs_ep_intr = {
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_IN | 1,	/* dongle -> host */
		bmAttributes: UE_INTERRUPT,
		wMaxPacketSize: 16,
		bInterval: 1
        };


static usb_endpoint_descriptor_t bcm_fs_ep_bulkin = {
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_IN | 2,	/* dongle -> host */
		bmAttributes: UE_BULK,
		wMaxPacketSize: 64,
		bInterval: 0
        };

static const usb_endpoint_descriptor_t bcm_fs_ep_bulkout = {
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_OUT | 3,
		bmAttributes: UE_BULK,
		wMaxPacketSize: 64,
		bInterval: 0
        };

#ifdef BCMUSBDEV_COMPOSITE
#ifdef USB_XDCI

static const usb_endpoint_descriptor_t bcm_ss_ep_isin = {
	bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_ENDPOINT,
	bEndpointAddress: UE_DIR_IN | 4,	/* dongle -> host */
	bmAttributes: UE_ISOCHRONOUS,
	wMaxPacketSize: USB_3_MAX_ISO_PACKET,
	bInterval: 1
};

static usb_endpoint_companion_descriptor_t bcm_ss_cp_isin = {
	bLength: USB_ENDPOINT_COMPANION_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_ENDPOINT_COMPANION,
	bMaxBurst: 0,
	bmAttributes: 0, /* B[0,1]:MULT=>Max packets = (bMasBurst+1)*(Mult+1) */
	wBytesPerInterVal: 1024,
};

static const usb_endpoint_descriptor_t bcm_ss_ep_isout = {
	bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_ENDPOINT,
	bEndpointAddress: UE_DIR_OUT | 5,
	bmAttributes: UE_ISOCHRONOUS,
	wMaxPacketSize: USB_3_MAX_ISO_PACKET,
	bInterval: 1
};
static usb_endpoint_companion_descriptor_t bcm_ss_cp_isout = {
	bLength: USB_ENDPOINT_COMPANION_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_ENDPOINT_COMPANION,
	bMaxBurst: 0,
	bmAttributes: 0,
	wBytesPerInterVal: 1024,
};
#endif /* USB_XDCI */


#endif /* BCMUSBDEV_COMPOSITE */

#ifdef BCMUSBDEV_COMPOSITE
static const usb_endpoint_descriptor_t bcm_hs_ep_isin = {
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_IN | 4,	/* dongle -> host */
		bmAttributes: UE_ISOCHRONOUS,
		wMaxPacketSize: USB_MAX_BT_ISO_PACKET,
		bInterval: 1
        };

static const usb_endpoint_descriptor_t bcm_hs_ep_isout = {
		bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
		bDescriptorType: UDESC_ENDPOINT,
		bEndpointAddress: UE_DIR_OUT | 5,
		bmAttributes: UE_ISOCHRONOUS,
		wMaxPacketSize: USB_MAX_BT_ISO_PACKET,
		bInterval: 1
        };
#endif /* BCMUSBDEV_COMPOSITE */


#ifdef BCMUSBDEV_COMPOSITE
static const usb_endpoint_descriptor_t bcm_fs_ep_isin = {
	bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_ENDPOINT,
	bEndpointAddress: UE_DIR_IN | 4,	/* dongle -> host */
	bmAttributes: UE_ISOCHRONOUS,
	wMaxPacketSize: USB_1_MAX_ISO_PACKET,
	bInterval: 1
};

static const usb_endpoint_descriptor_t bcm_fs_ep_isout = {
	bLength: USB_ENDPOINT_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_ENDPOINT,
	bEndpointAddress: UE_DIR_OUT | 5,
	bmAttributes: UE_ISOCHRONOUS,
	wMaxPacketSize: USB_1_MAX_ISO_PACKET,
	bInterval: 1
};

static const usb_dfu_functional_descriptor_t bcm_dfu = {
	bLength: USB_DFU_FUNCTIONAL_DESCRIPTOR_SIZE,
	bDescriptorType: UDESC_DFU,
	/* download, execute device requests after FW update without reboot */
	bmAttributes: 0x05,
	wDetachTimeOut: 0x1388,         /* 5 ms timeout */
	wTransferSize: USB_2_MAX_CTRL_PACKET,
	bcdDFUVersion: 0x0100
};
#endif /* BCMUSBDEV_COMPOSITE */

#ifdef BCMUSBDEV_COMPOSITE
static usb_device_qualifier_t bcm_fs_qualifier = {
#else
static const usb_device_qualifier_t bcm_fs_qualifier = {
#endif /* BCMUSBDEV_COMPOSITE */
	bLength: USB_DEVICE_QUALIFIER_SIZE,
	bDescriptorType: UDESC_DEVICE_QUALIFIER,
	bcdUSB: UD_USB_2_0,
	bDeviceClass: UDCLASS_VENDOR,
	bDeviceSubClass: 0,
	bDeviceProtocol: 0,
	bMaxPacketSize0: USB_2_MAX_CTRL_PACKET,
	bNumConfigurations: 1,
	bReserved: 0
};

#ifdef BCMUSBDEV_COMPOSITE
static usb_device_qualifier_t bcm_hs_qualifier = {
#else
static const usb_device_qualifier_t bcm_hs_qualifier = {
#endif /* BCMUSBDEV_COMPOSITE */
	bLength: USB_DEVICE_QUALIFIER_SIZE,
	bDescriptorType: UDESC_DEVICE_QUALIFIER,
	bcdUSB: UD_USB_2_0,
	bDeviceClass: UDCLASS_VENDOR,
	bDeviceSubClass: 0,
	bDeviceProtocol: 0,
	bMaxPacketSize0: USB_2_MAX_CTRL_PACKET,
	bNumConfigurations: 1,
	bReserved: 0
};


/* UTF16 (not NULL terminated) strings */
static const usb_string_descriptor_t BCMATTACHDATA(bcm_strings)[] =
{
	{
		/* USB spec: bLength is bString size + 2 */
#ifdef BCMUSBDEV_COMPOSITE
		bLength: 0x08,
#else
		bLength: 0x04,
#endif /* BCMUSBDEV_COMPOSITE */
		bDescriptorType: UDESC_STRING,
		bString: { 0x0409 }
	},
	{
		/* USB spec: bLength is bString size + 2 */
		bLength: sizeof(L"Broadcom"),
		bDescriptorType: UDESC_STRING,
		bString: { 'B', 'r', 'o', 'a', 'd', 'c', 'o', 'm' }
	},
	{
		/* USB spec: bLength is bString size + 2 */
		bLength: sizeof(L"BCMUSB 802.11 Wireless Adapter"),
		bDescriptorType: UDESC_STRING,
		bString: {
			'B', 'C', 'M', 'U', 'S', 'B', ' ',
			'8', '0', '2', '.', '1', '1', ' ',
			'W', 'i', 'r', 'e', 'l', 'e', 's', 's', ' ',
			'A', 'd', 'a', 'p', 't', 'e', 'r'
		}
	},
	{
		/* USB spec: bLength is bString size + 2 */
		bLength: sizeof(L"000000000001"),
		bDescriptorType: UDESC_STRING,
		bString: { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1' }
	},
#ifdef BCMUSBDEV_COMPOSITE
	{
		/* USB spec: bLength is bString size + 2 */
		bLength: sizeof(L"Composite Wireless Adapter"),
		bDescriptorType: UDESC_STRING,
		bString: {
			'C', 'o', 'm', 'p', 'o', 's', 'i', 't', 'e', ' ',
			'W', 'i', 'r', 'e', 'l', 'e', 's', 's', ' ',
			'A', 'd', 'a', 'p', 't', 'e', 'r'
		}
	},
	{
		/* USB spec: bLength is bString size + 2 */
		bLength: sizeof(L"Bluetooth MI"),
		bDescriptorType: UDESC_STRING,
		bString: {
			'B', 'l', 'u', 'e', 't', 'o', 'o', 't', 'h', ' ', 'M', 'I'
		}
	},
	{
		/* USB spec: bLength is bString size + 2 */
		bLength: sizeof(L"Bluetooth Interface"),
		bDescriptorType: UDESC_STRING,
		bString: {
			'B', 'l', 'u', 'e', 't', 'o', 'o', 't', 'h', ' ',
			'I', 'n', 't', 'e', 'r', 'f', 'a', 'c', 'e'
		}
	},
	{
		/* USB spec: bLength is bString size + 2 */
		bLength: sizeof(L"DFU Interface"),
		bDescriptorType: UDESC_STRING,
		bString: {
			'D', 'F', 'U', ' ', 'I', 'n', 't', 'e', 'r', 'f', 'a', 'c', 'e'
		}
	}
#endif /* BCMUSBDEV_COMPOSITE */
};

static inline int usbd_tx(struct dngl_bus *bus, int ep, void *p)
{
	return (bus->ch_func.tx(bus->ch, ep, p));
}


/**
 * Flow control to avoid tx (dongle->host) packet drop. Lower USB level requests this level to
 * pause traffic from wireless subsystem (wl/wlc) flowing towards this USB stack.
 */
void
usbdev_txstop(struct dngl_bus *bus, int ep)
{
	trace("");
	if (ep == bus->ep_bi[0])
		dngl_txstop(bus->dngl);

	trace("done");
}

void
usbdev_txstart(struct dngl_bus *bus, int ep)
{
	trace("");
	if (ep == bus->ep_bi[0])
		dngl_txstart(bus->dngl);

	trace("done");
}

void
usbdev_reset(struct dngl_bus *bus)
{
	void *p;
	int i;

	trace("ep_detach");

	/* Free queued ctrl packets */
	while ((p = pktdeq(&bus->ctrlq)))
		PKTFREE(bus->osh, p, TRUE);

	/* Detach configuration endpoints */
#ifdef BCMUSBDEV_COMPOSITE
	if (bus->use_intr_ep && bus->intr_ep[INT_WLAN]) {
		usbd_ep_detach(bus, bus->intr_ep[INT_WLAN]);
		bus->intr_ep[INT_WLAN] = 0;
	}

	for (i = 0; i < bus->ep_bo_num[bus->int_wlan_idx]; i++) {
		if (bus->ep_bo[i])
			usbd_ep_detach(bus, bus->ep_bo[i]);
		bus->ep_bo[i] = 0;
	}

	for (i = 0; i < bus->ep_bi_num[bus->int_wlan_idx]; i++) {
		if (bus->ep_bi[i])
			usbd_ep_detach(bus, bus->ep_bi[i]);
		bus->ep_bi[i] = 0;
	}

	if (bus->ep_bo_num[INTERFACE_BT] && bus->ep_bo_BT) {
		usbd_ep_detach(bus, bus->ep_bo_BT);
		bus->ep_bo_BT = 0;
	}

	if (bus->ep_bi_num[INTERFACE_BT] && bus->ep_bi_BT) {
		usbd_ep_detach(bus, bus->ep_bi_BT);
		bus->ep_bi_BT = 0;
	}

	if (bus->ep_isi_num[INTERFACE_ISO] && (bus->ep_isi)) {
		usbd_ep_detach(bus, bus->ep_isi);
		bus->ep_isi = 0;
	}

	if (bus->ep_iso_num[INTERFACE_ISO] && (bus->ep_iso)) {
		usbd_ep_detach(bus, bus->ep_iso);
		bus->ep_iso = 0;
	}

#ifdef USB_IFTEST
	ep_config_reset(bus->ch);
	bus->ep_config_valid = FALSE;
#endif /* USB_IFTEST */
#else
	if (bus->use_intr_ep && bus->intr_ep) {
		usbd_ep_detach(bus, bus->intr_ep);
		bus->intr_ep = 0;
	}

	for (i = 0; i < bus->ep_bo_num; i++) {
		if (bus->ep_bo[i])
			usbd_ep_detach(bus, bus->ep_bo[i]);
		bus->ep_bo[i] = 0;
	}
	for (i = 0; i < bus->ep_bi_num; i++) {
		if (bus->ep_bi[i])
			usbd_ep_detach(bus, bus->ep_bi[i]);
		bus->ep_bi[i] = 0;
	}
#endif /* BCMUSBDEV_COMPOSITE */

	bus->confignum = 0;
	if (bus->disconnect_upon_reset) {
		bus->disconnect_upon_reset = FALSE;
#ifdef DISCONNECT_FROM_BUS
		/* can't whack USB core in its ISR context: defer the operation */
		if (bus->generictimer_active)
			dngl_del_timer(bus->generictimer);
		bus->generictimer_action = ACTION_OOB_DISCONNECT;
		dngl_add_timer(bus->generictimer, 0, FALSE);
		bus->generictimer_active = TRUE;
#endif /* DISCONNECT_FROM_BUS */
	}

	bus->suspended = FALSE;

	dbg("  done");
}


/* bus is connected again; send q'd packets */
static void
usbdev_reconnect(struct dngl_bus *bus)
{
	if (bus->ep_bi[0]) {
		void *p;
		while ((p = pktdeq(&bus->txq)))
			usbd_tx(bus, bus->ep_bi[0], p);
	}
}

/** ROM accessor to avoid struct in shdat */
static usb_config_descriptor_t *
get_bcm_config(void)
{
	return &bcm_config;
}


/** get the index of wl bulkout ep */
uint
usbdev_getwlbulkout(struct dngl_bus *bus)
{
	return bus->ep_bo[0];
}

static inline int usbd_chk_version(struct dngl_bus *bus)
{
	return bus->ch_func.usbversion(bus->ch);
}

static inline bool usbd_hsic(struct dngl_bus *bus)
{
	return bus->ch_func.hsic(bus->ch);
}


/**
 * Called when higher layer wants to send something to the host, but the bus is suspended. Also
 * called on an elapsed time after a 'remote wakeup' iovar.
 */
static inline int usbd_resume(struct dngl_bus *bus)
{
	return bus->ch_func.resume(bus->ch);
}

static inline uint usbd_ep_attach(struct dngl_bus *bus, const usb_endpoint_descriptor_t *endpoint,
        const usb_endpoint_companion_descriptor_t *sscmp,
        int config, int interface, int alternate)
{
	return bus->ch_func.ep_attach(bus->ch, endpoint, sscmp, config, interface, alternate);
}


static inline void usbd_ep_detach(struct dngl_bus *bus, int ep)
{
	 bus->ch_func.ep_detach(bus->ch, ep);
}


/**
 * Called by lower level USB layer when a Start Of Frame is received. SOF indication is normally
 * disabled in lower level USB layer since it occurs very frequently.
 */
void
usbdev_sof(struct dngl_bus *dev)
{
	trace("");
}

/** lower USB level (usbdev_sb.c) caller sets device speed */
void
usbdev_speed(struct dngl_bus *bus, int high)
{
	trace("%s", high ? "high" : "full");
	bus->speed = high;
}

/** hide struct details for lower USB level (usbdev_sb.c) caller */
int
usbdev_getcfg(struct dngl_bus *bus)
{
	return bus->confignum;
}

/** hide details for callers outside of this file (being usbdev_sb.c) */
uint
usbdev_mps(struct dngl_bus *dev)
{
	trace("");
	return get_bcm_device()->bMaxPacketSize;
}

/** populates caller supplied buffer with caller supplied config descriptor */
static int
usbdev_htol_usb_config_descriptor(const usb_config_descriptor_t *d, uchar *buf)
{
	return htol_usb_config_descriptor(d, buf);
}


static int
usbdev_htol_usb_interface_descriptor(const usb_interface_descriptor_t *d, uchar *buf)
{
	return htol_usb_interface_descriptor(d, buf);
}

static int
usbdev_htol_usb_endpoint_descriptor(const usb_endpoint_descriptor_t *d, uchar *buf)
{
	return htol_usb_endpoint_descriptor(d, buf);
}

static int
usbdev_htol_usb_device_qualifier(const usb_device_qualifier_t *d, uchar *buf)
{
	return htol_usb_device_qualifier(d, buf);
}

#ifdef BCMUSBDEV_COMPOSITE
static int
usbdev_htol_usb_dfu_functional_descriptor(
	const usb_dfu_functional_descriptor_t *d, uchar *buf)
{
	return htol_usb_dfu_functional_descriptor(d, buf);
}

static int
usbdev_htol_usb_iad(usb_interface_association_descriptor_t *d, uchar *buf)
{
	return htol_usb_interface_association_descriptor(d, buf);
}

#endif /* BCMUSBDEV_COMPOSITE */

#ifdef USB_XDCI
static int
usbdev_htol_usb_ed_companion_descriptor(const usb_endpoint_companion_descriptor_t *d, uchar *buf)
{
	return htol_usb_endpoint_companion_descriptor(d, buf);
}

static int
usbdev_htol_usb_bos_descriptor(const usb_bos_descriptor_t *d, uchar *buf)
{
	return htol_usb_bos_descriptor(d, buf);
}
static int
usbdev_htol_usb_ss_device_capacity_descritor(
	const usb_ss_device_capability_descriptor_t *d, uchar *buf)
{
	return htol_usb_ss_device_capability_descriptor(d, buf);
}

static int
usbdev_htol_usb_2_extension_descritor(const usb_2_extension_descriptor_t *d, uchar *buf)
{
	return htol_usb_2_extension_descriptor(d, buf);
}
#endif /* USB_XDCI */

/**
 * Host indicates to dongle what USB configuration it selects. Several code paths lead up to this
 * function. It configures hardware and firmware to present the desired configuration to the host.
 *
 * This function, somehow, only does something when parameter config==1 (?)
 */
uint
usbdev_setcfg(struct dngl_bus *bus, int config)
{
	const usb_endpoint_descriptor_t *bcm_cur_endpoint;
	const usb_endpoint_companion_descriptor_t *sscmp = NULL;
	int i, j;
#ifdef BCMUSBDEV_COMPOSITE
	int k, ep_idx;
	BCM_REFERENCE(j);
#endif /* BCMUSBDEV_COMPOSITE */
	trace("");

	if ((config == get_bcm_config()->bConfigurationValue) || config == 0)
	{
		/* Attach configuration endpoints */
		bus->confignum = config;
	}

	if (config == 1) {
#ifdef BCMUSBDEV_COMPOSITE
#ifdef USB_XDCI
		if (bus->speed == UD_USB_SS) {
			bcm_cur_endpoint = bus->intr_endpoints_ss;
			sscmp = bus->intr_endpoints_cp_ss;
		}
		else
#endif /* USB_XDCI */
		if (bus->speed == UD_USB_HS)
			bcm_cur_endpoint = bus->intr_endpoints;
		else
			bcm_cur_endpoint = bus->intr_endpoints_fs;
		if (bus->use_intr_ep && !bus->intr_ep[INT_WLAN]) {
			bus->intr_ep[INT_WLAN] = usbd_ep_attach(bus,
				&bcm_cur_endpoint[0], &sscmp[0],
				get_bcm_config()->bConfigurationValue,
				bus->bcm_interface[bus->int_wlan_idx].bInterfaceNumber,
				bus->bcm_interface[bus->int_wlan_idx].bAlternateSetting);
			if (bus->interface_num > 1) {
				if (INT_WLAN == 0) {
					bus->intr_ep[INTERFACE_BT + 1] = bus->intr_ep[INT_WLAN];
				}
			}
		}
#ifdef BCM_VUSBD
		/* attach EP for BT interrupt */
		if (VUSBD_ENAB(bus)) {
			if (bus->ep_ii_num[INTERFACE_BT] &&	!bus->intr_ep[INTERFACE_BT]) {
				bus->intr_ep[INTERFACE_BT] = usbd_ep_attach(bus,
					&bcm_cur_endpoint[0], &sscmp[0],
					get_bcm_config()->bConfigurationValue,
					bus->bcm_interface[INTERFACE_BT].bInterfaceNumber,
					bus->bcm_interface[INTERFACE_BT].bAlternateSetting);
				vusbd_ep_attach(bus->vusbd, &bcm_cur_endpoint[0],
					bus->intr_ep[INTERFACE_BT]);
				dbg("vusbd BT IF attach intr ep %d\n", bus->intr_ep[INTERFACE_BT]);
			}
		}
#endif /* BCM_VUSBD */
#else
#ifdef USB_XDCI
		if (bus->speed == UD_USB_SS) {
			bcm_cur_endpoint = &bcm_ss_ep_intr;
			sscmp = &bcm_ss_cp_intr;
		}
		else
#endif /* USB_XDCI */
		if (bus->speed == UD_USB_HS)
			bcm_cur_endpoint = &bcm_hs_ep_intr;
		else
			bcm_cur_endpoint = &bcm_fs_ep_intr;

		if (bus->use_intr_ep && !bus->intr_ep)
			bus->intr_ep = usbd_ep_attach(bus, bcm_cur_endpoint,
				sscmp,
				get_bcm_config()->bConfigurationValue,
				bus->bcm_interface.bInterfaceNumber,
				bus->bcm_interface.bAlternateSetting);
#endif /* BCMUSBDEV_COMPOSITE */

#ifdef USB_XDCI
		if (bus->speed == UD_USB_SS) {
			bcm_cur_endpoint = bus->data_endpoints_ss;
			sscmp = bus->data_endpoints_cp_ss;
		}
		else
#endif /* USB_XDCI */
		if (bus->speed == UD_USB_HS)
			bcm_cur_endpoint = bus->data_endpoints;
		else
			bcm_cur_endpoint = bus->data_endpoints_fs;

#ifdef BCMUSBDEV_COMPOSITE

#ifdef BCM_VUSBD
		if (VUSBD_ENAB(bus)) {

			j = 0;

			/* attach endpoints for BT's interfaces */
			for (k = 0; k < bus->interface_num; k++) {

				if (k == bus->int_wlan)
					continue;

				for (i = 0; i < bus->ep_bi_num[k]; i++) {
					if (!bus->ep_bi_BT && (k == INTERFACE_BT)) {
						bus->ep_bi_BT = usbd_ep_attach(bus,
							&bcm_cur_endpoint[i+j],
							&sscmp[i+j],
							get_bcm_config()->bConfigurationValue,
							bus->bcm_interface[k].bInterfaceNumber,
							bus->bcm_interface[k].bAlternateSetting);

						vusbd_ep_attach(bus->vusbd, &bcm_cur_endpoint[i+j],
							bus->ep_bi_BT);

						dbg("BT EP attach %d BI hw ep %d\n", i+j,
							bus->ep_bi_BT);
					}
				}
				j += i;

				for (i = 0; i < bus->ep_bo_num[k]; i++) {
					if (!bus->ep_bo_BT && (k == INTERFACE_BT)) {

						bus->ep_bo_BT = usbd_ep_attach(bus,
							&bcm_cur_endpoint[i+j],
							&sscmp[i+j],
							get_bcm_config()->bConfigurationValue,
							bus->bcm_interface[k].bInterfaceNumber,
							bus->bcm_interface[k].bAlternateSetting);

						vusbd_ep_attach(bus->vusbd, &bcm_cur_endpoint[i+j],
							bus->ep_bo_BT);

						dbg("BT EP attach %d BO hw ep %x\n", i+j,
							bus->ep_bo_BT);
					}
				}
				j += i;

				/* attach iso interface */
				for (i = 0; i < bus->ep_isi_num[k]; i++) {
					if (!bus->ep_isi && (k == INTERFACE_ISO)) {
						bus->ep_isi = usbd_ep_attach(bus,
							&bcm_cur_endpoint[i+j], &sscmp[i+j],
							get_bcm_config()->bConfigurationValue,
							bus->bcm_interface[k].bInterfaceNumber,
							bus->bcm_interface[k].bAlternateSetting);
						vusbd_ep_attach(bus->vusbd, &bcm_cur_endpoint[i+j],
							bus->ep_isi);

						dbg("EP attach  %d ISO IN  hw ep %d\n", i+j,
							bus->ep_isi);
					}
				}
				j += i;

				for (i = 0; i < bus->ep_iso_num[k]; i++) {
					if (!bus->ep_iso && (k == INTERFACE_ISO)) {
						bus->ep_iso = usbd_ep_attach(bus,
							&bcm_cur_endpoint[i+j], &sscmp[i+j],
							get_bcm_config()->bConfigurationValue,
							bus->bcm_interface[k].bInterfaceNumber,
							bus->bcm_interface[k].bAlternateSetting);
						vusbd_ep_attach(bus->vusbd, &bcm_cur_endpoint[i+j],
							bus->ep_iso);
						dbg("EP attach %d ISO OUT %d\n", i+j, bus->ep_iso);
					}
				}
				j += i;

			}
			vusbd_enable(bus->vusbd, 0);
		}
#endif /* BCM_VUSBD */
		/* attach wlan interface */
		ep_idx = 0;

		for (k = 0; k < bus->int_wlan_idx; k++) {
			ep_idx += bus->ep_bi_num[k] + bus->ep_bo_num[k]
				+ bus->ep_isi_num[k] + bus->ep_iso_num[k];
		}

		for (i = 0; i < bus->ep_bi_num[bus->int_wlan_idx]; i++) {
			if (!bus->ep_bi[i]) {
				bus->ep_bi[i] = usbd_ep_attach(bus, &bcm_cur_endpoint[ep_idx],
					&sscmp[ep_idx],
					get_bcm_config()->bConfigurationValue,
					bus->bcm_interface[bus->int_wlan_idx].bInterfaceNumber,
					bus->bcm_interface[bus->int_wlan_idx].bAlternateSetting);
					ep_idx++;
					dbg("WLAN EP attach %d Bi ep %x\n", ep_idx, bus->ep_bi[i]);
			}
		}

		for (i = 0; i < bus->ep_bo_num[bus->int_wlan_idx]; i++) {
			if (!bus->ep_bo[i]) {
				bus->ep_bo[i] = usbd_ep_attach(bus, &bcm_cur_endpoint[ep_idx],
					&sscmp[ep_idx],
					get_bcm_config()->bConfigurationValue,
					bus->bcm_interface[bus->int_wlan_idx].bInterfaceNumber,
					bus->bcm_interface[bus->int_wlan_idx].bAlternateSetting);
					ep_idx++;
					dbg("WLAN EP attach %d Bo ep %x\n", ep_idx, bus->ep_bo[i]);
			}
		}
#ifdef USB_IFTEST
		/* set BT bulk, iso ep-config regs */
		ep_idx = 0;
		for (k = 0; k < bus->interface_num; k++) {
			if (k == bus->int_wlan_idx)
				continue;

			for (i = 0; i < bus->ep_bi_num[k]; i++) {
				if (!bus->bcm_interface[k].bAlternateSetting) {
					if (!bus->ep_config_valid)
						ep_config(bus->ch, &bcm_cur_endpoint[ep_idx+i],
						get_bcm_config()->bConfigurationValue,
						bus->bcm_interface[k].bInterfaceNumber,
						bus->bcm_interface[k].bAlternateSetting);
				}
			}
			ep_idx += bus->ep_bi_num[k] + bus->ep_bo_num[k];

			for (i = 0; i < bus->ep_isi_num[k]; i++) {
				if (!bus->bcm_interface[k].bAlternateSetting) {
					if (!bus->ep_config_valid)
						ep_config(bus->ch, &bcm_cur_endpoint[ep_idx+i],
						get_bcm_config()->bConfigurationValue,
						bus->bcm_interface[k].bInterfaceNumber,
						bus->bcm_interface[k].bAlternateSetting);
				}
			}
			ep_idx += bus->ep_isi_num[k] + bus->ep_iso_num[k];
		}
		bus->ep_config_valid = TRUE;
#endif /* USB_IFTEST */
#else /* !BCMUSBDEV_COMPOSITE */
		for (i = 0; i < bus->ep_bi_num; i++) {
			if (!bus->ep_bi[i])
				bus->ep_bi[i] = usbd_ep_attach(bus, &bcm_cur_endpoint[i],
					&sscmp[i],
					get_bcm_config()->bConfigurationValue,
					bus->bcm_interface.bInterfaceNumber,
					bus->bcm_interface.bAlternateSetting);
		}

		j = i;

		for (i = 0; i < bus->ep_bo_num; i++) {
			if (!bus->ep_bo[i])
				bus->ep_bo[i] = usbd_ep_attach(bus, &bcm_cur_endpoint[j],
					&sscmp[j],
					get_bcm_config()->bConfigurationValue,
					bus->bcm_interface.bInterfaceNumber,
					bus->bcm_interface.bAlternateSetting);
		}
#endif /* BCMUSBDEV_COMPOSITE */
		/* transmit any packets that were waiting for tx_ep */
		usbdev_reconnect(bus);
	}
	trace("done");

#ifdef USB_XDCI
		if (bus->usb30d) {
			return (get_bcm_config()->bConfigurationValue);
		}
#endif

	return get_bcm_config()->bmAttributes;
} /* usbdev_setcfg */

/**
 * Called when the usb20d core signals it suspended *and*  the lower USB level (usbdev_sb.c)
 * concluded that it is safe to stop requesting the HT clock. The wireless subsystem (wl/wlc) is
 * notified that there is an opportunity to transition to suspend mode. It should be noted that at
 * the moment (Jan 2013), the wireless subsystem does nothing with this notification (function
 * dngl_suspend() is an empty shell).
 */
void
usbdev_suspend(struct dngl_bus *bus)
{
	trace("");
	bus->suspended = TRUE;
	dngl_suspend(bus->dngl);

	/* test mode: host requested OOB reconnect after specified time */
	if (bus->generictimer_action == ACTION_REMOTE_WAKEUP) {
		dngl_add_timer(bus->generictimer, bus->generictimer_arg, FALSE);
		bus->generictimer_active = TRUE;
	}
}

/**
 * called when the usb20d core signals a 'resume' event, or prior to this device signalling
 * a USB disconnect to the host.
 */
void
usbdev_resume(struct dngl_bus *bus)
{
	trace("");
	if (bus->suspended) {
		bus->suspended = FALSE;
		if (!bus->disconnected) {
			usbdev_oobresume(bus, FALSE);
			usbdev_reconnect(bus);
			dngl_resume(bus->dngl);
		}
	}
} /* usbdev_resume */
