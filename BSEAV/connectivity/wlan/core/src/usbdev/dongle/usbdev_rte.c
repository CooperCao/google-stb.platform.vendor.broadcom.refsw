/*
 * USB device RTE interface
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

#include <usb.h>
#include <usbdev.h>
#include <usbdev_dbg.h>
#include <osl.h>
#include <osl_ext.h>
#include <sbusbd.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <dngl_bus.h>
#include <dngl_api.h>
#include <rte_isr.h>
#include <rte_dev.h>
#include <rte_cons.h>
#include <rte_trap.h>
#include <rte_ioctl.h>
#include <rte_gpio.h>

typedef struct {
	int unit;		/* Given by hnd_add_device(), always 0 */
	hnd_dev_t *rtedev;	/* Points usbdev_dev */
	void *ch;		/* Points struct usbdev_chip* as returned by xdc_attach/ch_attach */
	osl_t *osh;		/* Points to osl_attach() called from usbdev_probe */
	volatile void *regs;	/* Points to regs for this device core, as obtained by SI utils */
	uint16 device;		/* Points to the device id given in hnd_add_device() */
	struct dngl *dngl;	/* Points pts from dngl_attach(), required for dngl_xxx() */
	hnd_dpc_t *dpc;		/* DPC for calling _usbdev_dpctask() */

	struct usbchip_rte_func {
		int (*dpc)(struct usbdev_chip* ch);
		void (*intrson)(struct usbdev_chip* ch);
		int (*intrsupd)(struct usbdev_chip* ch);
		int (*intrdisp)(struct usbdev_chip* ch);
		void (*intrsoff) (struct usbdev_chip* ch);
		void (*intrs_deassert) (struct usbdev_chip* ch);
		void (*detach) (struct usbdev_chip* ch, bool disable);
		uint32 (*init) (struct usbdev_chip* ch, bool disconnect);
		int (*usbversion)(struct usbdev_chip* ch);
	} ch_rte_func;
} drv_t;

/* Driver entry points */
static void *usbdev_probe(hnd_dev_t *dev, volatile void *regs, uint bus, uint16 device,
                          uint coreid, uint unit);
#ifndef RTE_POLL
static void usbdev_isr(void *cbdata);
#ifdef THREAD_SUPPORT
static void usbdev_dpc(void *cbdata);
#endif	/* THREAD_SUPPORT */
#endif /* !RTE_POLL */
static void usbdev_run(hnd_dev_t *ctx);
static int usbdev_open(hnd_dev_t *dev);
static int usbdev_send(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lb);
static int usbdev_send_on_ep(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lb, uint32 ep_idx);
static int usbdev_send_ctl(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lb);
static int usbdev_ioctl(hnd_dev_t *dev, uint32 cmd, void *buf, int len,
                        int *used, int *needed, int set);
static int usbdev_close(hnd_dev_t *dev);
static void usbdev_txflowcontrol(hnd_dev_t *dev, bool state, int prio);
static void bus_fwhalt(void *ctx, uint32 trap_data);
#ifdef WL_WOWL_MEDIA
static void usbdev_wowldown(hnd_dev_t *dev);
#endif


static hnd_dev_ops_t usbdev_funcs = {
	probe:		usbdev_probe,
	open:		usbdev_open,
	close:		usbdev_close,
	xmit:		usbdev_send,
	ioctl:		usbdev_ioctl,
	txflowcontrol:	usbdev_txflowcontrol,
	poll:		usbdev_run,
	xmit_ctl:	usbdev_send_ctl,
#ifndef WL_WOWL_MEDIA
	xmit2:		usbdev_send_on_ep
#else
	xmit2:		usbdev_send_on_ep,
	wowldown:	usbdev_wowldown
#endif
};

struct dngl_bus_ops usbdev_bus_ops = {
	softreset:	usbdev_bus_softreset,
	binddev:	usbdev_bus_binddev,
	unbinddev:	usbdev_bus_unbinddev,
	tx:		usbdev_bus_tx,
	sendctl:	usbdev_bus_sendctl,
	rxflowcontrol:	usbdev_bus_rxflowcontrol,
	iovar:		usbdev_bus_iovar,
	resume:		usbdev_bus_resume,
	pr46794WAR:	usbdev_bus_pr46794WAR,
#ifdef BCMDBG
	dumpregs:	usbdev_bus_dumpregs,
	loopback:	usbdev_bus_loopback,
	xmit:		usbdev_bus_xmit,
	msgbits:	usbdev_bus_msgbits,
#endif
};

hnd_dev_t usbdev_dev = {
	name:		"usbdev",
	ops:		&usbdev_funcs
};

/* Number of devices found */
static int found = 0;

int usbdev_msglevel = USBDEV_ERROR;

/* thread-safe interrupt enable */
static void
_usbdev_intrson(drv_t *drv)
{
#ifdef THREAD_SUPPORT
	/* critical section */
	osl_ext_interrupt_state_t state = osl_ext_interrupt_disable();
#endif	/* THREAD_SUPPORT */

	drv->ch_rte_func.intrson(drv->ch);

#ifdef THREAD_SUPPORT
	/* critical section */
	osl_ext_interrupt_restore(state);
#endif	/* THREAD_SUPPORT */
}

static int
usbdev_ioctl(hnd_dev_t *rtedev, uint32 cmd, void *buf, int len, int *used, int *needed, int set)
{
	uint32 val = 0;
	drv_t *drv = rtedev->softc;
	BCM_REFERENCE(drv);

	trace("%s: %d", __FUNCTION__, cmd);

	switch (cmd) {
		case HND_RTE_DNGL_IS_SS:
		{
			if (!set) {
				if (drv->ch_rte_func.usbversion(drv->ch) == UD_USB_3_DEV)
					val = TRUE;
				else
					val = FALSE;
			} else
				return BCME_UNSUPPORTED;
		}
		break;
		default:
			return BCME_UNSUPPORTED;
	}

	if (!set) {
		ASSERT(NULL != buf);
		memcpy(buf, &val, sizeof(uint32));
	}
	return BCME_OK;
}

static void
_usbdev_dpc(drv_t *drv)
{
	trace("");

	if (drv->ch_rte_func.dpc(drv->ch)) {
		if (!hnd_dpc_schedule(drv->dpc)) {
			ASSERT(FALSE);
		}
	} else {
		/* re-enable interrupts */
		_usbdev_intrson(drv);
	}
}

static void
_usbdev_dpctask(void *cbdata)
{
	drv_t *drv = (drv_t *)cbdata;

	trace("");
	drv->ch_rte_func.intrsupd(drv->ch);
	_usbdev_dpc(drv);
}

#ifndef RTE_POLL
static void
usbdev_isr(void *cbdata)
{
	hnd_dev_t *rtedev = cbdata;
	drv_t *drv = rtedev->softc;
	BCM_REFERENCE(drv);

	trace("");
	ASSERT(drv->ch);
#ifdef THREAD_SUPPORT
	/* deassert interrupt */
	drv->ch_rte_func.intrs_deassert(drv->ch);
#else
	usbdev_run(rtedev);
#endif	/* THREAD_SUPPORT */
}

#ifdef THREAD_SUPPORT
static void
usbdev_dpc(void *cbdata)
{
	hnd_dev_t *rtedev = cbdata;

	usbdev_run(rtedev);
}
#endif	/* THREAD_SUPPORT */
#endif /* !RTE_POLL */

static void
usbdev_run(hnd_dev_t *rtedev)
{
	drv_t *drv = rtedev->softc;

	trace("");
	ASSERT(drv->ch);
	if (drv->ch_rte_func.intrdisp(drv->ch)) {
		drv->ch_rte_func.intrsoff(drv->ch);
		_usbdev_dpc(drv);
	}
#ifdef THREAD_SUPPORT
	else {
		/* isr turned off interrupts */
		_usbdev_intrson(drv);
	}
#endif	/* THREAD_SUPPORT */
}

/**
 *  Forwards a packet towards the host.
 *
 *  Input parameters:
 *		dev - device context (includes ptr to our softc)
 *		buffer - pointer to buffer descriptor.
 *
 *  Return value:
 *		status, 0 = ok
 */
static int
usbdev_send(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *buffer)
{
	drv_t *drv = dev->softc;

	trace("drv%dwrite", drv->unit);
#ifdef BCMUSBDEV_BMAC
	return dngl_sendpkt((void *)(drv->dngl), src, (void *)buffer, 0);
#else
	return dngl_sendpkt((void *)(drv->dngl), src, (void *)buffer);
#endif /* BCMUSBDEV_BMAC */
}

/**  Forwards a packet towards the host. */
static int
usbdev_send_on_ep(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *buffer, uint32 ep_idx)
{
	drv_t *drv = (drv_t *) dev->softc;

#ifdef BCMUSBDEV_BMAC
	return dngl_sendpkt((void *)(drv->dngl), src, (void *)buffer, ep_idx);
#else
	ASSERT(0);
	err("BCMUSBDEV_BMAC is not defined");
	return dngl_sendpkt((void *)(drv->dngl), src, (void *)buffer);
#endif /* BCMUSBDEV_BMAC */

}

static int
usbdev_send_ctl(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *buffer)
{
	drv_t *drv = dev->softc;

	trace("drv%dwrite", drv->unit);
	return dngl_sendctl((void *)(drv->dngl), src, (void *)buffer);
}

static void
usbdev_txflowcontrol(hnd_dev_t *dev, bool state, int prio)
{
	drv_t *drv = dev->softc;

	/* throttle transmit to slave by pausing USB rx */
	dngl_rxflowcontrol(drv->dngl, state, prio);
}

#ifdef WL_WOWL_MEDIA
static void
usbdev_wowldown(hnd_dev_t *dev)
{
	drv_t *drv = dev->softc;
	ch_wowldown(drv->ch);
}
#endif

/*
 *  Open the USB device.
 *  Prepare to receive and send packets.
 *
 *  Input parameters:
 *		dev - device context (includes ptr to our softc)
 *
 *  Return value:
 *		status, 0 = ok
 */

static int
BCMATTACHFN(usbdev_open)(hnd_dev_t *dev)
{
	drv_t *drv = dev->softc;

	dbg("%s", dev->name);

#ifdef RSOCK
	dngl_init(drv->dngl);
#endif

	drv->dpc = hnd_dpc_register(_usbdev_dpctask, drv, NULL);
	if (drv->dpc == NULL) {
		return BCME_NORESOURCE;
	}

#ifdef BCMUSB_NODISCONNECT
	drv->ch_rte_func.init(drv->ch, FALSE);
#else
	/* delay so that handshake at end of bootloader protocol completes */
	OSL_DELAY(10000);
	drv->ch_rte_func.init(drv->ch, TRUE);
#endif

	return 0;
}

/*
 *  Open the USB device.
 *  Prepare to receive and send packets.
 *
 *  Input parameters:
 *		dev - device context (includes ptr to our softc)
 *
 *  Return value:
 *		status, 0 = ok
 */

static int
BCMATTACHFN(usbdev_close)(hnd_dev_t *dev)
{
#ifndef BCMNODOWN
	drv_t *drv = dev->softc;


	drv->ch_rte_func.detach(drv->ch, TRUE);
	dbg("drv exit%d", drv->unit);
#endif /* BCMNODOWN */
	return 0;
}
static const char BCMATTACHDATA(rstr_usbrndisD)[] = "usbrndis%d";
static const char BCMATTACHDATA(rstr_usbcdcD)[] = "usbcdc%d";

/*
 *  Probe and install driver.
 *  Create a context structure and attach to the  specified network device.
 *
 *  Input parameters:
 *		dev - device descriptor
 *		regs - mapped registers
 *		device - device ID
 *		unit - unit number
 *
 *  Return value:
 *		nothing
 */

static void *
BCMATTACHFN(usbdev_probe)(hnd_dev_t *rtedev, volatile void *regs, uint bus, uint16 device,
                        uint coreid, uint unit)
{
	drv_t *drv;
	osl_t *osh;

	if (found >= 8) {
		err("too many units");
		goto fail;
	}

	osh = osl_attach(rtedev);

	if (!(drv = (drv_t *)MALLOCZ(osh, sizeof(drv_t)))) {
		err("malloc failed");
		goto fail;
	}

	drv->unit = found;
	drv->rtedev = rtedev;
	drv->device = device;
	drv->regs = regs;
	drv->osh = osh;

	/* Allocate chip state */
#ifdef USB_XDCI
	if (rtedev->flags & (1 << RTEDEVFLAG_USB30)) {
		if (!(drv->ch = xdci_attach(drv, VENDOR_BROADCOM,
			drv->device, osh, drv->regs, bus)))
			goto fail;
		drv->ch_rte_func.dpc = xdci_dpc;
		drv->ch_rte_func.intrson = xdci2wlan_irq_on;
		drv->ch_rte_func.intrsoff = xdci2wlan_irq_off;
		drv->ch_rte_func.intrsupd = xdci2wlan_isr;
		drv->ch_rte_func.intrdisp = xdci2wlan_isr;
		drv->ch_rte_func.intrs_deassert = xdci2wlan_irq_off;
		drv->ch_rte_func.init = xdci_init;
		drv->ch_rte_func.usbversion = xdci_usb30;

	} else
#endif
	{
		if (!(drv->ch = ch_attach(drv, VENDOR_BROADCOM, drv->device, osh, drv->regs, bus)))
			goto fail;
		drv->ch_rte_func.dpc = ch_dpc;
		drv->ch_rte_func.intrson = ch_intrson;
		drv->ch_rte_func.intrsoff = ch_intrsoff;
		drv->ch_rte_func.intrsupd = ch_intrsupd;
		drv->ch_rte_func.intrdisp = ch_dispatch;
		drv->ch_rte_func.intrs_deassert = ch_intrs_deassert;
		drv->ch_rte_func.init = ch_init;
		drv->ch_rte_func.usbversion = ch_usb20;
	}

#ifdef BCMDBG
	if (!hnd_cons_add_cmd("usbsbdump", do_usbdevdump_cmd, drv->ch))
		goto fail;
#endif

#ifndef RTE_POLL
	if (hnd_isr_register(0, coreid, unit, usbdev_isr, rtedev,
	    usbdev_dpc, rtedev, NULL, bus) != BCME_OK)
	{
#ifndef BCMNODOWN
		drv->ch_rte_func.detach(drv->ch, TRUE);
#endif /* BCMNODOWN */
		goto fail;
	}
#endif	/* !RTE_POLL */
	drv->dngl = ch_dngl(drv->ch);

#ifdef RNDIS
	snprintf(rtedev->name, HND_DEV_NAME_MAX, rstr_usbrndisD, found);
	dbg("%s: Broadcom USB RNDIS driver", rtedev->name);
#else
	snprintf(rtedev->name, HND_DEV_NAME_MAX, rstr_usbcdcD, found);
	dbg("%s: Broadcom USB CDC driver", rtedev->name);
#endif

	/* Set halt handler */
	hnd_set_fwhalt(bus_fwhalt, drv);

	found++;

	return (void*)drv;

fail:
	return NULL;
}

/* bind/enslave to device */
/* Return > 0 to indicate success and I/F index carried in BDC header.
 * Return < 0 to indicate failure.
 */
int
usbdev_bus_binddev(void *bus, void *dev, uint numslaves)
{
	drv_t *drv = ((hnd_dev_t *)bus)->softc;
	return dngl_binddev(drv->dngl, bus, dev, numslaves);
}

/* unbind virtual device */
/* Return > 0 to indicate success and I/F index carried in BDC header.
 * Return < 0 to indicate failure.
 */
int
usbdev_bus_unbinddev(void *bus, void *dev)
{
	drv_t *drv = ((hnd_dev_t *)bus)->softc;
	return dngl_unbinddev(drv->dngl, bus, dev);
}

#ifdef NOT_YET
static int
BCMATTACHFN(usbdev_module_init)(si_t *sih)
{
	trace("add USB device");

	if (hnd_add_device(sih, &usbdev_dev, USB_CORE_ID, BCM47XX_USBD_ID) == 0 ||
	    hnd_add_device(sih, &usbdev_dev, USB20D_CORE_ID, BCM47XX_USB20D_ID) == 0 ||
	    hnd_add_device(sih, &usbdev_dev, USB11D_CORE_ID, BCM47XX_USBD_ID) == 0) {
		return BCME_OK;
	}

	return BCME_ERROR;
}

RTE_MODULE_INIT(usbdev_module_init);
#endif /* NOT_YET */

#ifdef BCMDBG
void
usbdev_bus_dumpregs()
{
	drv_t *drv = usbdev_dev.softc;
	struct bcmstrbuf b;
	char buf[BUS_DBG_BUFSZ];

	bcm_binit(&b, buf, BUS_DBG_BUFSZ);
#ifndef USB_XDCI
	ch_dumpregs(drv->ch, &b);
#endif
	printbig(buf);
}

void
usbdev_bus_loopback()
{
	drv_t *drv = usbdev_dev.softc;
	char *buf = "abcdefghijklmnopqrstuvwxyz0123456789";

	printf("\nloopback of single packet containing ");
	printf(buf);
	printf("\n");
#ifndef USB_XDCI
	ch_loopback(drv->ch, buf, strlen(buf) + 1);
#endif
	printf("loopback done\n");
}

void
usbdev_bus_xmit(int len, int clen, bool ctl)
{
	printf("\nNot implemented.\n");
}

uint
usbdev_bus_msgbits(uint *newbits)
{
	if (newbits)
		usbdev_msglevel = (int)*newbits;
	return (uint)usbdev_msglevel;
}
#endif /* BCMDBG */

/* called from startarm.S hnd_die() */
static void
bus_fwhalt(void *ctx, uint32 trap_data)
{
	drv_t *drv = (drv_t *)ctx;
	uint currid;
#ifdef OOB_WAKE
	struct dngl_bus *bus = NULL;
#endif /* OOB_WAKE */
	si_t *sih = ch_get_sih(drv->ch);

	currid = si_coreid(sih);
#ifdef USB_XDCI
	if (currid != USB30D_CORE_ID)
		si_setcore(sih, USB30D_CORE_ID, 0);
	else
#endif
	if (currid != USB20D_CORE_ID)
		si_setcore(sih, USB20D_CORE_ID, 0);

#ifdef OOB_WAKE
	if (drv && (bus = ch_bus(drv->ch))) {
		/*
		 * drive DEVICE_READY inactive, if connected, to trigger host to reenumerate device
		 *
		 */
		usbdev_devready(bus, FALSE, TRUE);
	}
#endif /* OOB_WAKE */

	/* switch to CPULess mode */
	/* reset core: will sample the bit for CPULess */
	si_core_reset(sih, DMPIOC_CPULESS, 0);

	/* Must wait at least 400 ns after core reset */
	OSL_DELAY(1);

#ifdef OOB_WAKE
	/* drop off and then appear the bus so host will re-enumerate us */
	if (bus) {
		/* Drive DEVICE_READY active */
		usbdev_devready(bus, TRUE, TRUE);
	}
#endif /* OOB_WAKE */

#ifdef USB_XDCI
	if (currid != USB30D_CORE_ID)
		si_setcore(sih, USB30D_CORE_ID, 0);
	else
#endif

	if (currid != USB20D_CORE_ID)
		si_setcore(sih, currid, 0);

	printf("bus_fwhalt done\n");
}

usbdev_gpioh_t *
usbdev_gpio_handler_register(uint32 event, bool level,
	usbdev_gpio_handler_t cb, void *arg)
{
	return (usbdev_gpioh_t *)rte_gpio_handler_register(event, level, cb, arg);
}

void
usbdev_gpio_handler_unregister(usbdev_gpioh_t *gi)
{
	rte_gpio_handler_unregister(gi);
}
