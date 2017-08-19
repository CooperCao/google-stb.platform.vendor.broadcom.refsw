/*
 * Polled-mode USB Remote Download driver for CFE
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

#include <typedefs.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <proto/ethernet.h>
#include <bcmendian.h>
#include <dngl_bus.h>
#include <usb.h>
#include <usbdev.h>
#include <hndcpu.h>
#include <rdl_dbg.h>
#include <rte_dev.h>
#include <rte_cons.h>
#include <sbchipc.h>
#ifdef USB_BDCI
#include <bdc_rte.h>
#endif
#include <usbrdl.h>

/* Max number of devices for probe */
#define MAX_DEVICES_FOR_PROBE 8

struct usbchip_rte_func {
	int (*dpc)(struct usbdev_chip* ch);
	si_t * (*get_sih)(struct usbdev_chip* ch);
	int (*intrdisp)(struct usbdev_chip* ch);
	void (*intrsoff) (struct usbdev_chip* ch);
	struct usbdev_chip * (*attach) (void *drv, uint vendor, uint device,
		osl_t *osh, volatile void *regs, uint bus);
	void (*detach) (struct usbdev_chip *ch, bool disable);
	uint32 (*init) (struct usbdev_chip* ch, bool disconnect);
	bool (*is_hsic)(struct usbdev_chip *ch);
};

typedef struct rdl {
	void *rdl_dev;		/* Back pointer */
	uint unit;		/* Given by hnd_add_device(), always 0 */
	uint device;		/* Chip ID - the device id given in hnd_add_device() */
	uint bus;		/* bus between device and processor - set to SI_BUS now */
	volatile void *regs;	/* Chip registers for this device core, as obtained by SI utils */
	void *ch;	/* Points struct usbdev_chip* as returned by xdci_attach/ch_attach */
	osl_t *osh;
#ifdef USB_HUB
	void *hub_drv;
#endif /* USB_HUB */

	struct usbchip_rte_func *ch_rte_func;
	struct rdl *next;
} rdl_t;

static rdl_t *rdl_list = NULL;

#ifdef USB_HUB
void *rdl_get_hub_drv(void *p)
{
	rdl_t *rdl = (rdl_t *)p;

	return rdl->hub_drv;
}
#endif /* USB_HUB */

#if defined(BCMDBG) || defined(BCMDBG_ERR)
int rdl_msglevel = 0x1;
int usbdev_msglevel = RDL_ERROR;
#endif /* BCMDBG */

/* Number of devices found */
static int found = 0;

/* addr to jump to */
static void *jumpto = NULL;
static bool jumpnow = FALSE;

static int rdl_close(hnd_dev_t *dev);

void rdl_addcmd(void);
void rdl_indicate_start(uint32 dlstart);
extern rdl_t * rdl_attach(struct dngl_bus *bus, void *drv);

static uint8 rdl_get_hsic_dev_rdy_details(si_t *sih, uint32 *usbrdy_val);
static uint8 rdl_get_hsic_host_rdy_details(si_t *sih, uint32 *hostrdy_val);

bool rdl_usb_chip_match(hnd_dev_t *drv, uint vendor, uint device);

#if defined(USB_BDCI)
static void rdl_bdc_function_init(rdl_t *rdl)
{
	dbg("rdl_bdc_function_init");

	rdl->ch_rte_func->dpc = bdci_dpc;
	rdl->ch_rte_func->get_sih = bdci_get_sih;
	rdl->ch_rte_func->intrdisp = bdci2wlan_isr;
	rdl->ch_rte_func->init = bdci_init;
	rdl->ch_rte_func->attach = bdci_attach;
	rdl->ch_rte_func->detach = bdci_detach;
	rdl->ch_rte_func->is_hsic = bdci_hsic;
}
#elif defined(USB_XDCI)
static void rdl_xdc_function_init(rdl_t *rdl)
{
	dbg("rdl_xdc_function_init");

	rdl->ch_rte_func->dpc = xdci_dpc;
	rdl->ch_rte_func->get_sih = xdci_get_sih;
	rdl->ch_rte_func->intrdisp = xdci2wlan_isr;
	rdl->ch_rte_func->init = xdci_init;
	rdl->ch_rte_func->attach = xdci_attach;
	rdl->ch_rte_func->detach = xdci_detach;
	rdl->ch_rte_func->is_hsic = xdci_hsic;
}
#else
static void rdl_ch_function_init(rdl_t *rdl)
{
	dbg("rdl_ch_function_init");

	rdl->ch_rte_func->dpc = ch_dpc;
	rdl->ch_rte_func->get_sih = ch_get_sih;
	rdl->ch_rte_func->intrdisp = ch_dispatch;
	rdl->ch_rte_func->init = ch_init;
	rdl->ch_rte_func->attach = ch_attach;
	rdl->ch_rte_func->detach = ch_detach;
	rdl->ch_rte_func->is_hsic = ch_hsic;
}
#endif /* defined(USB_BDCI) */
#ifdef USB_HUB
void usb_bind(void *rdl_drv, void *hub_drv);
void usb_bind(void *rdl_drv, void *hub_drv)
{
	rdl_t *rdl = (rdl_t *)rdl_drv;
	rdl->hub_drv = hub_drv;
}
#endif /* USB_HUB */
static void
rdl_isr(void *cbdata)
{
	hnd_dev_t *dev = (hnd_dev_t *)cbdata;
	rdl_t *rdl = dev->softc;

	if (rdl->ch) {
		if (rdl->ch_rte_func->intrdisp(rdl->ch))
			rdl->ch_rte_func->dpc(rdl->ch);
	}

	/* Jump to dl image ? */
	if (jumpnow) {
#if defined(BCMQT)
#if defined(RTE_CONS)
		/* print memuse after dl for bootloader sizing: DATA_START, stack size, etc. */
		process_ccmd("mu", 2);
		/* OSL_DELAY(100000); */
#endif
#else
		/* wait 10ms to give the host time to handshake the DL_GO Setup packet */
		OSL_DELAY(10000);
#endif 

		rdl_close(dev);
		dbg("rdl_go : 0x%x", (uint32)jumpto);
		hnd_cpu_jumpto(jumpto);
	}
}

static void
rdl_poll(hnd_dev_t *dev)
{
	rdl_isr(dev);
}

void
rdl_indicate_start(uint32 dlstart)
{
	jumpto = (void*)dlstart;
	jumpnow = TRUE;
}


/*
 *  ETHER_WRITE(dev,buffer)
 *
 *  Write a packet to the Ethernet device.
 *
 *  Input parameters:
 *		dev - device context (includes ptr to our softc)
 *		buffer - pointer to buffer descriptor.
 *
 *  Return value:
 *		status, 0 = ok
 */

static int
rdl_write(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *buffer)
{
#ifdef BCMDBG
	rdl_t *rdl = dev->softc;
#endif
	trace("rdl%dwrite", rdl->unit);

	return 0;
}

/*
 *  ETHER_OPEN(dev)
 *
 *  Open the Ethernet device.  The MAC is reset, initialized, and
 *  prepared to receive and send packets.
 *
 *  Input parameters:
 *		dev - device context (includes ptr to our softc)
 *
 *  Return value:
 *		status, 0 = ok
 */

static int
rdl_open(hnd_dev_t *dev)
{
	rdl_t *rdl = dev->softc;
	char *otp_val;
	uint32 val;
	si_t *sih;

	uint32 hostrdy_val = 0;
	uint8 hostrdy_gpio;

	uint32 usbrdy_val = USBGPIO_INVALID;
	uint8 usbrdy_gpio = CC4335_PIN_GPIO_01;
	uint32 usbrdy_pol = 0; /* 0=active high; 1=active low */

	dbg("rdl%d", rdl->unit);

	sih = rdl->ch_rte_func->get_sih(rdl->ch);

	if (rdl->ch_rte_func->is_hsic(rdl->ch)) {
		/* [HOST-DEV-Handshake-1 of 3] setup the dev rdy and make it low. */
		usbrdy_gpio = rdl_get_hsic_dev_rdy_details(sih, &usbrdy_val);

		if ((usbrdy_val != USBGPIO_INVALID)) {
			/* Note: usbrdy_pol 0=active high; 1=active low. So, adding ! below
			* makes usbrdy_pol inverted so that it gives a right picture which level
			* we want after ch_init()
			*/
			usbrdy_pol = !(usbrdy_val >> USBGPIO_POLARITYBIT_SHIFT);

			/* drive !(usbrdy_pol) to usbrdy_gpio */
			si_gpioout(sih, 1 << usbrdy_gpio, !(usbrdy_pol) << usbrdy_gpio, 0);
			si_gpioouten(sih, 1 << usbrdy_gpio, 1 << usbrdy_gpio, 0);

			switch (CHIPID(sih->chip)) {
			case BCM4335_CHIP_ID:
			case BCM4345_CHIP_ID:
			case BCM4350_CHIP_ID:
			case BCM4354_CHIP_ID:
			case BCM43556_CHIP_ID:
			case BCM43558_CHIP_ID:
			case BCM43566_CHIP_ID:
			case BCM43568_CHIP_ID:
			case BCM43569_CHIP_ID:
			case BCM43570_CHIP_ID:
			case BCM4358_CHIP_ID:
				/* setup the default OUT pin HSIC_DEV_RDY
				 * a) drive chipc as what was required. [bcos it is an out,
				 *		so no pull down reqd]: already done above
				 *	[both these overridden by otp]
				 * b) then route using fn-sel to avoid glitches. : done below
				 * Note: all chips use the same Function Sel (same as pin)
				 */
				si_gci_set_functionsel(sih, usbrdy_gpio, CC4335_FNSEL_SAMEASPIN);
			default:
				;
			}

			/* remove pulls if any on usbrdy so that there is no leakage. */
			si_gpiopull(sih, 1, 1 << usbrdy_gpio, 0);
			si_gpiopull(sih, 0, 1 << usbrdy_gpio, 0);
		} else
			goto skip_wait;

		/* [HOST-DEV-Handshake-2 of 3] setup host rdy and if enabled, wait */
		hostrdy_gpio = rdl_get_hsic_host_rdy_details(sih, &hostrdy_val);

		if (hostrdy_val & (1 << USBFL_HOSTRDY_WAIT_S)) {
			uint32 bit;
			uint16 cnt = 0;
			uint16 htimeout = (hostrdy_val >> USBFL_HOSTRDY_TO_S) &
				USBFL_HOSTRDY_TO_M;
			uint32 hostrdy_pol = !((hostrdy_val >> USBFL_HOSTRDY_GPIOPOL_S)	&
				USBFL_HOSTRDY_GPIOPOL_M);
			int glitchfiltercnt;

			/* add weak pull. hostrdy_pol: 1 means looking for transition from 0 to 1 */
			si_gpiopull(sih, hostrdy_pol, 1 << hostrdy_gpio, 1 << hostrdy_gpio);

			/* clear the opposite polarity pull */
			si_gpiopull(sih, !hostrdy_pol, 1 << hostrdy_gpio, 0);

			/* make sure that we are NOT driving hostrdy_gpio  */
			si_gpioout(sih, 1 << hostrdy_gpio, 0, 0);
			si_gpioouten(sih, 1 << hostrdy_gpio, 0, 0);

			/* for 4335 route GPIO_1 in fn-sel to chipc ("same as pin name") */
			switch (CHIPID(sih->chip)) {
			case BCM4335_CHIP_ID:
			case BCM4345_CHIP_ID:
			case BCM4350_CHIP_ID:
			case BCM4354_CHIP_ID:
			case BCM43556_CHIP_ID:
			case BCM43558_CHIP_ID:
			case BCM43566_CHIP_ID:
			case BCM43568_CHIP_ID:
			case BCM43569_CHIP_ID:
			case BCM43570_CHIP_ID:
			case BCM4358_CHIP_ID:
				si_gci_set_functionsel(sih, hostrdy_gpio, CC4335_FNSEL_SAMEASPIN);
				OSL_DELAY(1000); /* 1ms */
			default:
				;
			}

			/* unit of htimeout: seconds. so htimeout and OSL_DELAY() at the
			* end of the next while() has to take care of 1s.
			*/
			if (htimeout != USBFL_HOSTRDY_TO_INFINITE)
				htimeout *= 1000;

			while ((htimeout == USBFL_HOSTRDY_TO_INFINITE) || (cnt < htimeout)) {
				bit = (si_gpioin(sih) >> hostrdy_gpio) & 0x1;

				if (bit == hostrdy_pol) {
					/* glitch handling code, for 1 millisec, checks each 100uS
					* for 10 times. [it is possible that, hostRDY can glitch.
					* So following code makes sure that, hostrdy stays in the
					* required polarity for more than 1 millisecond]
					*/
					for (glitchfiltercnt = 0;
						glitchfiltercnt < USB_GLITCHFILTER_CNT_MAX;
						glitchfiltercnt++) {
						OSL_DELAY(100); /* 100 us */
						bit = (si_gpioin(sih) >>
								hostrdy_gpio) & 0x1;
						if (bit != hostrdy_pol) {
							break; /* inner loop */
						}
					}

					if (glitchfiltercnt >= USB_GLITCHFILTER_CNT_MAX) {
						printf("Got HRDY: %d\n", cnt);
						break;
					}
				}


				cnt++;
				OSL_DELAY(1000); /* 1 ms */
			}

			/* remove a weak pull in the inactive direction */
			si_gpiopull(sih, hostrdy_pol, 1 << hostrdy_gpio, 0);
		}
	}

skip_wait:


	if ((val = (uint32)getintvar(NULL, "usbpredly")) != 0) {
		OSL_DELAY(val*1000);
	}

	if ((otp_val = getvar(NULL, "bldr_to")) != NULL) {
		val = (uint32)bcm_strtoul(otp_val, NULL, 0);
		/* The value in the OTP is in terms of millisecs */
		si_watchdog_ms(sih, val);
	}

	/* Initialize chip */
	rdl->ch_rte_func->init(rdl->ch, TRUE);

	if ((val = (uint32)getintvar(NULL, "usbpostdly")) != 0) {
		OSL_DELAY(val*1000);
	}


	if (rdl->ch_rte_func->is_hsic(rdl->ch)) {
		/* [HOST-DEV-Handshake-3 of 3] respond to host with dev rdy */
		if (usbrdy_val != USBGPIO_INVALID) {
			/* The required GPIO line is asserted with 0 or 1 as obtained from OTP */
			si_gpioout(sih, 1 << usbrdy_gpio, usbrdy_pol << usbrdy_gpio, 0);
		}
	}

	dbg("rdl exit%d", rdl->unit);
	return 0;
}

static uint8
rdl_get_hsic_dev_rdy_details(si_t *sih, uint32 *usbrdy_val)
{
	char *otp_str;
	uint32 usbrdy_gpio = USBGPIO_INVALID;
	uint32 otp_val;

#ifdef DEFAULT_USBRDY
	otp_val = DEFAULT_USBRDY;
#else /* DEFAULT_USBRDY */
	otp_val = USBGPIO_INVALID;
#endif /* DEFAULT_USBRDY */

	/* For 4335, default is done in code, not in Makefile.
	* get the default OUT pin for HSIC_DEV_RDY
	* a) check pkg.
	* b) for WLBGA pkg, GPIO_8 should be routed to chipc
	* c) for WLCSP pkg, GPIO_9 should be routed to chipc
	*/
	if (CHIPID(sih->chip) == BCM4335_CHIP_ID) {
		if ((sih->chippkg & BCM4335_PKG_MASK) == BCM4335_WLBGA_PKG_ID)
			otp_val = CC4335_PIN_GPIO_08;
		else /* implicit: BCM4335_WLCSP_PKG_ID */
			otp_val = CC4335_PIN_GPIO_09;

		/* note: default pull is active high: 0 */
	}

	if ((otp_str = getvar(NULL, "usbrdy")) != NULL)
		otp_val = (uint32)bcm_strtoul(otp_str, NULL, 0);

	if (otp_val != USBGPIO_INVALID)
		usbrdy_gpio = otp_val & USBGPIO_GPIO_MASK;

	*usbrdy_val = otp_val;

	return usbrdy_gpio;
}

static uint8
rdl_get_hsic_host_rdy_details(si_t *sih, uint32 *hostrdy_val)
{
	char *otp_str;
	uint32 hostrdy_gpio = 0;
	uint32 otp_val;

#ifdef DEFAULT_USBFLAGS
	otp_val = DEFAULT_USBFLAGS;
#else
	otp_val = 0;
#endif

	if ((otp_str = getvar(NULL, "usbflags")) != NULL)
		otp_val = (uint32)bcm_strtoul(otp_str, NULL, 0);

	if (otp_val & (1 << USBFL_HOSTRDY_WAIT_S))
		hostrdy_gpio = (otp_val >> USBFL_HOSTRDY_GPIO_S) & USBFL_HOSTRDY_GPIO_M;

	*hostrdy_val = otp_val;

	return hostrdy_gpio;
}

/*
 *  ETHER_CLOSE(dev)
 *
 *  Close the Ethernet device.
 *
 *  Input parameters:
 *		dev - device context (includes ptr to our softc)
 *
 *  Return value:
 *		status, 0 = ok
 */

static int
rdl_close(hnd_dev_t *dev)
{
	rdl_t *rdl = dev->softc;

	/* Free chip state */
	if (rdl->ch) {
		rdl->ch_rte_func->detach(rdl->ch, FALSE);
	}
	rdl->ch = NULL;

	MFREE(rdl->osh, rdl->ch_rte_func, sizeof(struct usbchip_rte_func));
	rdl->ch_rte_func = NULL;

	return 0;
}

/*
 *  ETHER_IOCTL(dev,buffer)
 *
 *  Do device-specific I/O control operations for the device
 *
 *  Input parameters:
 *		dev - device context (includes ptr to our softc)
 *		buffer - pointer to buffer descriptor.
 *
 *  Return value:
 *		status, 0 = ok
 */

static int
rdl_ioctl(hnd_dev_t *dev, uint32 cmd, void *buffer, int len, int *used, int *needed, int set)
{
#ifdef BCMDBG
	rdl_t *rdl = dev->softc;
#endif
	trace("rdl%d", rdl->unit);

	return 0;
}

bool
rdl_usb_chip_match(hnd_dev_t *drv, uint vendor, uint device)
{
	trace("Vendor 0x%x device 0x%x \n", vendor, device);

	if (vendor != VENDOR_BROADCOM) {
		dbg("rdl: vendor id didn't match\n");
		return FALSE;
	}

#ifdef USB_XDCI
	if (drv->flags & (1 << RTEDEVFLAG_USB30)) {
		switch (device) {
		case BCM47XX_USBD_ID:
		case BCM47XX_USB20D_ID:
		case BCM47XX_USB30D_ID:
			return TRUE;
		}
	}
#endif
	if (device == BCM47XX_USB30D_ID)
		return TRUE;

	dbg("rdl: Device id didn't match\n");
	return FALSE;
}
/*
 *  ETHER_PROBE(drv,probe_a,probe_b,probe_ptr)
 *
 *  Probe and install driver.
 *  Create a context structure and attach to the
 *  specified network device.
 *
 *  Input parameters:
 *		drv - driver descriptor
 *		probe_a - device ID
 *		probe_b - unit number
 *		probe_ptr - mapped registers
 *
 *  Return value:
 *		nothing
 */

static void *
rdl_probe(hnd_dev_t *rdl_dev, volatile void *regs, uint bus, uint16 device, uint coreid, uint unit)
{
	rdl_t *rdl;
	osl_t *osh;

	if (!rdl_usb_chip_match(rdl_dev, VENDOR_BROADCOM, device))
		return NULL;

	if (found >= MAX_DEVICES_FOR_PROBE) {
		err("too many units");
		return NULL;
	}

	osh = osl_attach(rdl_dev);
	if (!(rdl = (rdl_t *)MALLOC(osh, sizeof(*rdl)))) {
		err("malloc failed");
		return NULL;
	}
	bzero(rdl, sizeof(rdl_t));

	rdl->osh = osh;

	if (!(rdl->ch_rte_func =
		(struct usbchip_rte_func *)MALLOC(osh, sizeof(*(rdl->ch_rte_func))))) {
		err("usbchip_rte_func malloc failed");
		return NULL;
	}

	rdl->unit = found;
	rdl->rdl_dev = rdl_dev;
	rdl->device = device;
	rdl->regs = regs;
	rdl->bus = bus;

	printf("rdl%d: Broadcom USB Remote Download Adapter\n", found);

	/* Add us to the global linked list */
	rdl->next = rdl_list;
	rdl_list = rdl;

#if defined(USB_BDCI)
	rdl_bdc_function_init(rdl);
#elif defined(USB_XDCI)
	rdl_xdc_function_init(rdl);
#else
	rdl_ch_function_init(rdl);
#endif
	if (!(rdl->ch = rdl->ch_rte_func->attach(rdl, VENDOR_BROADCOM, rdl->device,
			osh, rdl->regs, rdl->bus))) {
			err("USB Device attach failed");
			MFREE(osh, rdl, sizeof(rdl_t));
			return NULL;
		}

#ifndef RTE_POLL
	if (hnd_isr_register(0, coreid, unit, rdl_isr, rdl_dev, bus) != BCME_OK) {
			rdl.ch_rte_func->detach(rdl->ch, TRUE);
		err("RDL ISR could not be registered ");
		MFREE(osh, rdl, sizeof(rdl_t));
		return NULL;
	}
#endif	/* !RTE_POLL */
	found++;

	return ((void*)rdl);
}

static hnd_dev_ops_t rdl_funcs = {
	probe:		rdl_probe,
	open:		rdl_open,
	close:		rdl_close,
	xmit:		rdl_write,
	ioctl:		rdl_ioctl,
	poll:		rdl_poll
};

hnd_dev_t bcmrdl = {
	name:		"bcmrdl",
	ops:		&rdl_funcs
};
