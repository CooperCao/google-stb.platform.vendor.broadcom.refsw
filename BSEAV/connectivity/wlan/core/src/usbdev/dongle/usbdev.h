/*
 * USB device(target) driver API
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

#ifndef	_usbdev_h_
#define	_usbdev_h_

#include <typedefs.h>
#include <usbstd.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>

struct usbdev_chip;

/* gpioX interrupt handler */
typedef void (*usbdev_gpio_handler_t)(uint32 stat, void *arg);
typedef struct usbdev_gpioh usbdev_gpioh_t;

/* Device operations */
extern struct dngl_bus * usbdev_attach(osl_t *osh, struct usbdev_chip *ch, si_t *sih);
extern void usbdev_detach(struct dngl_bus *bus);
extern uint usbdev_mps(struct dngl_bus *bus);
extern void *usbdev_setup(struct dngl_bus *bus, int ep, void *p, int *errp, int *dir);
extern uint usbdev_setcfg(struct dngl_bus *bus, int n);
extern int usbdev_getcfg(struct dngl_bus *bus);
extern void usbdev_rx(struct dngl_bus *bus, int ep, void *p);
extern void usbdev_txstop(struct dngl_bus *bus, int ep);
extern void usbdev_txstart(struct dngl_bus *bus, int ep);
extern void usbdev_reset(struct dngl_bus *bus);
extern void usbdev_sof(struct dngl_bus *bus);
extern void usbdev_suspend(struct dngl_bus *bus);
extern void usbdev_resume(struct dngl_bus *bus);
extern void usbdev_speed(struct dngl_bus *bus, int high);
extern uint usbdev_getwlbulkout(struct dngl_bus *bus);

extern usbdev_gpioh_t * usbdev_gpio_handler_register(uint32 event, bool level,
	usbdev_gpio_handler_t cb, void *arg);
extern void usbdev_gpio_handler_unregister(usbdev_gpioh_t *gi);

/* return dngl handle */
extern void *usbdev_dngl(struct dngl_bus *bus);
extern bool usbdev_oobresume(struct dngl_bus *bus, bool active);
#ifdef OOB_WAKE
extern bool usbdev_oob_connected(struct dngl_bus *bus);
#else
#define usbdev_oob_connected(bus) 1
#endif

/* Chip operations */
#ifdef USB_XDCI
extern void usbdev_attach_xdci(struct dngl_bus *bus);
extern void usbdev_detach_xdci(struct dngl_bus *bus);
extern bool xdci_match(uint vendor, uint device);
extern int xdci_usb30(struct usbdev_chip *ch);
extern int xdci_tx(struct usbdev_chip *ch, int ep_index, void *p);
extern int xdci_resume(struct usbdev_chip *ch);
extern int xdci2wlan_isr(struct usbdev_chip *ch);
extern void xdci2wlan_irq_on(struct usbdev_chip *ch);
extern void xdci2wlan_irq_off(struct usbdev_chip *ch);
extern int xdci_dpc(struct usbdev_chip *ch);
extern	void xdci_ep_stall(struct usbdev_chip *ch, int ep_index);
extern void xdci_ep_detach(struct usbdev_chip *ch, int ep_index);
extern uint32 xdci_ep_attach(struct usbdev_chip *ch, const usb_endpoint_descriptor_t *endpoint,
	const usb_endpoint_companion_descriptor_t *sscmp,
	int config, int interface, int alternate);
extern uint32
BCMATTACHFN(xdci_init)(struct usbdev_chip *ch, bool disconnect);
extern struct usbdev_chip *
BCMATTACHFN(xdci_attach)(void *drv, uint vendor, uint device, osl_t *osh,
	volatile void *regs, uint bus);
extern void
BCMATTACHFN(xdci_detach)(struct usbdev_chip *ch, bool disable);
extern si_t *xdci_get_sih(struct usbdev_chip *ch);
extern void xdci_rxflowcontrol(struct usbdev_chip *ch, int ep, bool state);
extern	void xdci_indicate_start(void *ch, uint32 start, uint32 no_dis);
extern int xdci_indicate_speed(void *ch, uint32 speed);
extern void xdci_enable_u1u2(void *ch, bool enable_u1u2);
extern bool xdci_hsic(struct usbdev_chip *ch);
#endif /* USB_XDCI */

extern bool ch_match(uint vendor, uint device);
extern struct usbdev_chip * ch_attach(void *drv, uint vendor, uint device, osl_t *osh,
                                    volatile void *regs, uint bus);
extern void ch_detach(struct usbdev_chip *ch, bool disable);
extern int ch_intrsupd(struct usbdev_chip *ch);
extern void ch_intrsoff(struct usbdev_chip *ch);
extern void ch_intrs_deassert(struct usbdev_chip* ch);
extern void ch_intrson(struct usbdev_chip* ch);
extern uint32 ch_init(struct usbdev_chip* ch, bool disconnect);
extern void ch_dumpregs(struct usbdev_chip *ch, struct bcmstrbuf *b);
extern void ch_rxflowcontrol(struct usbdev_chip* ch, int ep, bool state);
extern int ch_dispatch(struct usbdev_chip* ch);
extern int ch_dpc(struct usbdev_chip* ch);
extern int ch_tx(struct usbdev_chip* ch, int ep, void *p);
extern int ch_tx_resp(struct usbdev_chip* ch, int ep, void *p);
extern uint ep_attach(struct usbdev_chip* ch,
	const usb_endpoint_descriptor_t *endpoint,
	const usb_endpoint_companion_descriptor_t *sscmp,
	int config, int interface, int alternate);
extern void ep_detach(struct usbdev_chip* ch, int ep);
extern int ch_loopback(struct usbdev_chip *ch, char *buf, uint count);
extern int ch_usb20(struct usbdev_chip* ch);
extern int ch_resume(struct usbdev_chip* ch);
extern void ch_pr46794WAR(struct usbdev_chip *ch);
/* return dngl handle */
extern void *ch_dngl(struct usbdev_chip *ch);
struct dngl_bus *ch_bus(struct usbdev_chip *ch);
/* return sih handle */
extern si_t * ch_get_sih(struct usbdev_chip *ch);
extern bool ch_hsic(struct usbdev_chip *ch);
#ifdef BCMUSB_NODISCONNECT
extern bool ch_hispeed(struct usbdev_chip *ch);
#endif
extern void ch_disconnect(struct usbdev_chip *ch);
extern void ch_devready(struct usbdev_chip *ch, uint32 delay);
extern void ch_leave_suspend_wrapper(struct usbdev_chip *ch);
extern void usbdev_devready(struct dngl_bus *bus, bool active, bool called_from_trap);
extern bool ch_txpending(struct usbdev_chip *ch);
#ifdef BCMDBG
extern void do_usbdevdump_cmd(void *arg, int argc, char *argv[]);
#endif

extern void ch_mdio_wreg(struct usbdev_chip *ch, uint16 addr, uint16 data);
extern uint16 ch_mdio_rreg(struct usbdev_chip *ch, uint16 addr);
extern int process_lpm_settings(struct usbdev_chip *ch);
extern void enter_lpm_sleep(struct usbdev_chip *ch);
extern void leave_lpm_sleep(struct usbdev_chip *ch);

/* Bus API operations */
void usbdev_bus_softreset(struct dngl_bus *bus);
int usbdev_bus_binddev(void *bus, void *dev, uint numslaves);
int usbdev_bus_unbinddev(void *bus, void *dev);
#ifdef BCMUSBDEV_BMAC
int usbdev_bus_tx(struct dngl_bus *bus, void *pi, uint32 ep_index);
#else
int usbdev_bus_tx(struct dngl_bus *bus, void *p);
#endif /* BCMUSBDEV_BMAC */

#ifdef BCMUSBDEV_COMPOSITE
void usbd_stall_ep(struct dngl_bus *bus, int ep);
void ch_stall_ep(struct usbdev_chip *ch, int hw_ep);
#endif /* BCMUSBDEV_COMPOSITE */

void usbdev_bus_sendctl(struct dngl_bus *bus, void *p);
void usbdev_bus_rxflowcontrol(struct dngl_bus *bus, bool state, int prio);
uint32 usbdev_bus_iovar(struct dngl_bus *usbdev, char *buf,
                        uint32 inlen, uint32 *outlen, bool set);
void usbdev_bus_resume(struct dngl_bus *bus);
void usbdev_bus_pr46794WAR(struct dngl_bus *bus);

#ifdef BCMDBG
void usbdev_bus_dumpregs(void);
void usbdev_bus_loopback(void);
void usbdev_bus_xmit(int len, int clen, bool ctl);
uint usbdev_bus_msgbits(uint *newbits);
#endif

/* defines for host handshake gpios taken from nvram */
#define USBGPIO_INVALID		0xFF
#define USBGPIO_GPIO_MASK	0x1F
#define USBGPIO_DEFAULTPULL_BIT	0x40
#define USBGPIO_POLARITYBIT_SHIFT 7

/* General USB Flags (to save OTP overhead) */
#define USBFL_HOSTRDY_TO_M		BITFIELD_MASK(8)
#define USBFL_HOSTRDY_TO_S		0
#define USBFL_HOSTRDY_TO_INFINITE	0xff		/* Wait 0-254 seconds or 0xff infinite */
#define USBFL_HOSTRDY_GPIO_M		BITFIELD_MASK(3)
#define USBFL_HOSTRDY_GPIO_S		8
#define USBFL_HOSTRDY_GPIOPOL_M	BITFIELD_MASK(1)
#define USBFL_HOSTRDY_GPIOPOL_S	11
#define USBFL_HOSTRDY_WAIT_M		BITFIELD_MASK(1)
#define USBFL_HOSTRDY_WAIT_S		12
#define USBFL_PULLENABLE_M		BITFIELD_MASK(1)
#define USBFL_PULLENABLE_S		13

#define USB_GLITCHFILTER_CNT_MAX        (10)

#define USB_HOSTRDY_WAIT_ENAB(flags)	(GFIELD((flags), USBFL_HOSTRDY_WAIT))
#define USB_HOSTRDY_GPULL_ENAB(flags)	(GFIELD((flags), USBFL_HOSTRDY_GPIOPULL))
#define USB_PULLENABLE_ENAB(flags)	(GFIELD((flags), USBFL_PULLENABLE))

#ifdef WL_WOWL_MEDIA
extern void ch_wowldown(struct usbdev_chip *ch);
#endif

#ifdef BCMUSBDEV_COMPOSITE
/* USB-BT Interface/Endpoint control
 * bits 2:0 	- Number of BT interfaces in addition to default WLAN
 * bit  3   	- disable WLAN intr ep
 * bits 5:4 	- additional WLAN bi eps
 * bits 7:6 	- additional WLAN bo eps
 * bit  8	- enable IAD
 * bits 11:9  	- SCO/ISO ep mps
 * bits 13:12 	- Device Class: BT, WLAN
 * bit 14 		- WLAN at interface 0
 * bits 15 	- Reserved
 */
#define USB_IF_NUM_MASK			0x07
#define USB_WLANIF_INTR_EP_MASK		0x08
#define USB_WLANIF_INTR_EP_SHIFT	3
#define USB_BI_EP_MASK			0x30
#define USB_BI_EP_SHIFT			4
#define USB_B0_EP_MASK			0xc0
#define USB_B0_EP_SHIFT			6
#define USB_IAD_MASK			0x100
#define USB_ISO_MPS_MASK		0xe00
#define USB_ISO_MPS_SHIFT		9
#define USB_DEVCLASS_MASK		0x3000
#define USB_DEVCLASS_SHIFT		12
#define USB_DEVCLASS_BT			0x01
#define USB_DEVCLASS_WLAN		0x02
#define USB_WL_INTF_MASK		0x4000
#define USB_WL_INTF_SHIFT		14

extern void ep_config_reset(struct usbdev_chip *ch);
extern void ep_config(struct usbdev_chip *ch, const usb_endpoint_descriptor_t *endpoint,
	int config, int interface, int alternate);
#endif /* BCMUSBDEV_COMPOSITE */

#ifndef BCMUSBDEV_COMPOSITE
#define VUSBD_ENAB(bus) (0)
#else
#if defined(BCM_VUSBD)
#define VUSBD_ENAB(bus) (bus->vusbd_enab)
#else
#define VUSBD_ENAB(bus)	(0)
#endif
#endif /* BCM_VUSB */


#endif	/* _usbdev_h_ */
