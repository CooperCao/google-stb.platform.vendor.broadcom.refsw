/*
 * SDPCMD device RTE interface
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

#include <sdpcmdev.h>
#include <sdpcmdev_dbg.h>
#include <osl.h>
#include <osl_ext.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <dngl_bus.h>
#include <dngl_api.h>
#include <rte_dev.h>
#include <rte_isr.h>
#include <rte_ioctl.h>

typedef struct {
	int unit;
	hnd_dev_t *rtedev;
	struct dngl_bus *sdpcmd;
	osl_t *osh;
	volatile void *regs;
	uint16 device;
	struct dngl *dngl;
	hnd_dpc_t *dpc;		/* DPC for calling _sdpcmd_dpctask() */
} drv_t;

/* Reclaimable strings */
#ifdef RNDIS
static const char BCMATTACHDATA(rstr_fmt_banner)[] = "%s: Broadcom SDPCMD RNDIS driver\n";
static const char BCMATTACHDATA(rstr_fmt_devname)[] = "sdpcmdrndis%d";
#else
static const char BCMATTACHDATA(rstr_fmt_banner)[] = "%s: Broadcom SDPCMD CDC driver\n";
static const char BCMATTACHDATA(rstr_fmt_devname)[] = "sdpcmdcdc%d";
#endif

/* Driver entry points */
static void *sdpcmd_probe(hnd_dev_t *dev, volatile void *regs, uint bus, uint16 device,
                          uint coreid, uint unit);
#ifndef RTE_POLL
static void sdpcmd_isr(void *cbdata);
#ifdef THREAD_SUPPORT
static void sdpcmd_dpc_thread(void *cbdata);
#endif	/* THREAD_SUPPORT */
#endif /* !RTE_POLL */
static void sdpcmd_run(hnd_dev_t *ctx);
static int sdpcmd_open(hnd_dev_t *dev);
static int sdpcmd_send(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lb);
static int sdpcmd_send_ctl(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lb);
static int sdpcmd_ioctl(hnd_dev_t *dev, uint32 cmd, void *buf, int len,
                        int *used, int *needed, int set);
static int sdpcmd_close(hnd_dev_t *dev);
static void sdpcmd_txflowcontrol(hnd_dev_t *dev, bool state, int prio);

static hnd_dev_ops_t sdpcmd_funcs = {
	probe:		sdpcmd_probe,
	open:		sdpcmd_open,
	close:		sdpcmd_close,
	xmit:		sdpcmd_send,
	ioctl:		sdpcmd_ioctl,
	txflowcontrol:	sdpcmd_txflowcontrol,
	poll:		sdpcmd_run,
	xmit_ctl:	sdpcmd_send_ctl
};

struct dngl_bus_ops sdpcmd_bus_ops = {
	softreset:	sdpcmd_bus_softreset,
	binddev:	sdpcmd_bus_binddev,
	rebinddev:	sdpcmd_bus_rebinddev,
	unbinddev:	sdpcmd_bus_unbinddev,
	tx:		sdpcmd_bus_tx,
	sendctl:	sdpcmd_bus_sendctl,
	rxflowcontrol:	sdpcmd_bus_rxflowcontrol,
	iovar:		sdpcmd_bus_iovar,
	resume:		NULL,
	pr46794WAR:	sdpcmd_bus_pr46794WAR,

#ifdef BCMDBG
	dumpregs:	sdpcmd_bus_dumpregs,
	loopback:	sdpcmd_bus_loopback,
	xmit:		sdpcmd_bus_xmit,
	msgbits:	sdpcmd_bus_msgbits,
#endif
};

hnd_dev_t sdpcmd_dev = {
	name:		"sdpcmdev",
	ops:		&sdpcmd_funcs
};

/* Number of devices found */
static int found = 0;

int sd_msg_level = SD_ERROR_VAL;

/* thread-safe interrupt enable */
static void
_sdpcmd_intrson(drv_t *drv)
{
#ifdef THREAD_SUPPORT
	/* critical section */
	osl_ext_interrupt_state_t state = osl_ext_interrupt_disable();
#endif	/* THREAD_SUPPORT */

	sdpcmd_intrson(drv->sdpcmd);

#ifdef THREAD_SUPPORT
	/* critical section */
	osl_ext_interrupt_restore(state);
#endif	/* THREAD_SUPPORT */
}

static int
sdpcmd_ioctl(hnd_dev_t *rtedev, uint32 cmd, void *buf, int len, int *used, int *needed, int set)
{
	drv_t *drv = (drv_t *) rtedev->softc;
	int ret = BCME_OK;
	uint32 outlen = 0;

	switch (cmd) {
	case SDPCMDEV_SET_MAXTXPKTGLOM:
		sdpcmd_set_maxtxpktglom(drv->sdpcmd, *((uint8 *)buf));
		break;
	case BUS_GET_VAR:
	case BUS_SET_VAR:
		ASSERT((cmd == BUS_GET_VAR) == !set);
		if (strncmp((char *)buf, "bus:", strlen("bus:"))) {
			ret = BCME_ERROR;
			break;
		}

		ret = sdpcmd_bus_iovar(drv->sdpcmd, (char *)buf, len, &outlen, set);
		break;
	default:
		ret = BCME_ERROR;
		break;
	}

	if (used)
		*used = outlen;
	return ret;
}

static void
_sdpcmd_dpc(drv_t *drv)
{
	if (sdpcmd_dpc(drv->sdpcmd)) {
		if (!hnd_dpc_schedule(drv->dpc)) {
			ASSERT(FALSE);
		}
	/* re-enable interrupts */
	} else {
		_sdpcmd_intrson(drv);
	}
}

static void
_sdpcmd_dpctask(void *cbdata)
{
	drv_t *drv = (drv_t *)cbdata;

	sdpcmd_intrsupd(drv->sdpcmd);
	_sdpcmd_dpc(drv);
}

#ifndef RTE_POLL
static void
sdpcmd_isr(void *cbdata)
{
	hnd_dev_t *rtedev = cbdata;
#ifdef THREAD_SUPPORT
	drv_t *drv = rtedev->softc;

	/* deassert interrupt */
	sdpcmd_intrsoff(drv->sdpcmd);
#else
	sdpcmd_run(rtedev);
#endif	/* THREAD_SUPPORT */
}

#ifdef THREAD_SUPPORT
static void
sdpcmd_dpc_thread(void *cbdata)
{
	hnd_dev_t *rtedev = cbdata;

	sdpcmd_run(rtedev);
}
#endif	/* THREAD_SUPPORT */
#endif /* !RTE_POLL */

static void
sdpcmd_run(hnd_dev_t *rtedev)
{
	drv_t *drv = rtedev->softc;

	ASSERT(drv->sdpcmd);

#ifndef THREAD_SUPPORT
	sdpcmd_intrsoff(drv->sdpcmd);
#endif	/* THREAD_SUPPORT */

	/* call common first level interrupt handler */
	if (sdpcmd_dispatch(drv->sdpcmd)) {
		/* if more to do... */
		_sdpcmd_dpc(drv);
	}
#ifdef THREAD_SUPPORT
	else {
		/* isr turned off interrupts */
		_sdpcmd_intrson(drv);
	}
#endif	/* THREAD_SUPPORT */
}

/**
 *  Forwards a packet towards the host (over the SDPCMD bus).
 *
 *  Input parameters:
 *		dev - device context (includes ptr to our softc)
 *		buffer - pointer to buffer descriptor.
 *
 *  Return value:
 *		status, 0 = ok
 */
static int
sdpcmd_send(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lb)
{
	drv_t *drv = dev->softc;

	SD_TRACE(("sdpcmd_send: drv%dwrite\n", drv->unit));
	return dngl_sendpkt((void *)(drv->dngl), src, (void *)lb);
}

static int
sdpcmd_send_ctl(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lb)
{
	drv_t *drv = dev->softc;

	SD_TRACE(("sdpcmd_send: drv%dwrite\n", drv->unit));
	return dngl_sendctl((void *)(drv->dngl), src, (void *)lb);
}

static void
sdpcmd_txflowcontrol(hnd_dev_t *dev, bool state, int prio)
{
	drv_t *drv = dev->softc;

	/* throttle transmit to slave by pausing SDPCMD rx */
	dngl_rxflowcontrol(drv->dngl, state, prio);
}

/*
 *
 *  Open the SDPCMD device.
 *  Prepare to receive and send packets.
 *
 *  Input parameters:
 *		dev - device context (includes ptr to our softc)
 *
 *  Return value:
 *		status, 0 = ok
 */

static int
BCMATTACHFN(sdpcmd_open)(hnd_dev_t *dev)
{
	drv_t *drv = dev->softc;

	SD_TRACE(("sdpcmd_open: %s\n", dev->name));

#ifdef RSOCK
	/* init the dongle state */
	dngl_init(drv->dngl);
#endif

	drv->dpc = hnd_dpc_register(_sdpcmd_dpctask, drv, NULL);
	if (drv->dpc == NULL) {
		return BCME_NORESOURCE;
	}

	/* Initialize chip */
	sdpcmd_init(drv->sdpcmd);

	return 0;
}

/*
 *  Close the SDPCMD device.
 *
 *  Input parameters:
 *		dev - device context (includes ptr to our softc)
 *
 *  Return value:
 *		status, 0 = ok
 */

int
BCMATTACHFN(sdpcmd_close)(hnd_dev_t *dev)
{
#ifndef BCMNODOWN
	drv_t *drv = dev->softc;

	SD_TRACE(("sdpcmd_close: drv exit%d\n", drv->unit));


	sdpcmd_detach(drv->sdpcmd);
	osl_detach(drv->osh);
#endif /* BCMNODOWN */

	return 0;
}

/*
 *
 *  Probe and install driver.
 *  Create a context structure and attach to the
 *  specified network device.
 *
 *  Input parameters:
 *		rtedev - device descriptor
 *		regs - mapped registers
 *		device - device ID
 *		unit - unit number
 *
 *  Return value:
 *		pointer to context structure
 */

static void *
BCMATTACHFN(sdpcmd_probe)(hnd_dev_t *rtedev, volatile void *regs, uint bus, uint16 device,
                        uint coreid, uint unit)
{
	drv_t *drv;
	osl_t *osh;

	SD_TRACE(("sdpcmd_probe\n"));

	if (found >= 8) {
		SD_ERROR(("sdpcmd_probe: too many units\n"));
		goto fail;
	}
#ifdef SHARED_OSL_CMN
	osh = osl_attach(rtedev, NULL);
#else
	osh = osl_attach(rtedev);
#endif /* SHARED_OSL_CMN */
	if (!(drv = (drv_t *)MALLOC(osh, sizeof(drv_t)))) {
		SD_ERROR(("sdpcmd_probe: malloc failed\n"));
		goto fail;
	}

	bzero(drv, sizeof(drv_t));

	drv->unit = found;
	drv->rtedev = rtedev;
	drv->device = device;
	drv->regs = regs;

	drv->osh = osh;

	/* Allocate chip state */
	if (!(drv->sdpcmd = sdpcmd_attach(drv, VENDOR_BROADCOM, device, osh, regs, bus))) {
		SD_ERROR(("sdpcmd_probe: sdpcmd_attach failed\n"));
		goto fail;
	}

#ifndef RTE_POLL
	if (hnd_isr_register(0, coreid, unit, sdpcmd_isr, rtedev,
	    sdpcmd_dpc_thread, rtedev, NULL, bus) != BCME_OK) {
		SD_ERROR(("sdpcmd_probe: hnd_isr_register failed\n"));
		sdpcmd_detach(drv->sdpcmd);
		goto fail;
	}
#endif	/* !RTE_POLL */

	drv->dngl = sdpcmd_dngl(drv->sdpcmd);

	(void)snprintf(rtedev->name, sizeof(rtedev->name), rstr_fmt_devname, found);
	printf(rstr_fmt_banner, rtedev->name);

	found++;

	return (void*)drv;

fail:
	return NULL;
}

/* bind/enslave to virtual device */
/* Return > 0 to indicate success and I/F index carried in BDC header.
 * Return < 0 to indicate failure.
 */
int
sdpcmd_bus_binddev(void *bus, void *dev, uint numslaves)
{
	drv_t *drv = ((hnd_dev_t *)bus)->softc;
	return dngl_binddev(drv->dngl, bus, dev, numslaves);
}

void
sdpcmd_bus_rebinddev(void *bus, void *dev, int ifindex)
{
	drv_t *drv = ((hnd_dev_t *)bus)->softc;
	dngl_rebinddev(drv->dngl, bus, dev, ifindex);
}

/* unbind virtual device */
/* Return > 0 to indicate success and I/F index carried in BDC header.
 * Return < 0 to indicate failure.
 */
int
sdpcmd_bus_unbinddev(void *bus, void *dev)
{
	drv_t *drv = ((hnd_dev_t *)bus)->softc;
	return dngl_unbinddev(drv->dngl, bus, dev);
}

#ifdef NOT_YET
static int
BCMATTACHFN(sdpcmd_module_init)(si_t *sih)
{
	SD_TRACE(("sdpcmd_module_init: add SDIO device\n"));

	if (hnd_add_device(sih, &sdpcmd_dev, PCMCIA_CORE_ID, SDIOD_FPGA_ID) == 0 ||
	    hnd_add_device(sih, &sdpcmd_dev, SDIOD_CORE_ID, SDIOD_FPGA_ID) == 0) {
		return BCME_OK;
	}

	return BCME_ERROR;
}

RTE_MODULE_INIT(sdpcmd_module_init);
#endif /* NOT_YET */

void
sdpcmd_bus_pr46794WAR(struct dngl_bus *bus)
{
}

#ifdef BCMDBG
static char bigbuf[BUS_DBG_BUFSZ] = { 0 };

void
sdpcmd_bus_xmit(int len, int clen, bool ctl)
{
	drv_t *drv = sdpcmd_dev.softc;
	char *buf = "abcdefghijklmnopqrstuvwxyz0123456789A123";

	if (len == -1) {
		printf("\ntransmit of single %s packet containing %s\n",
		       (ctl ? "ctl" : "data"), buf);
		clen = len = strlen(buf) + 1;
		sdpcmd_transmit(drv->sdpcmd, buf, len, clen, ctl);
	} else if ((len < 0) || (len > BUS_DBG_BUFSZ)) {
		printf("\nInvalid length %d\n", len);
	} else {
		if (!bigbuf[0]) {
			int i;
			for (i = 0; i < BUS_DBG_BUFSZ; i++)
				bigbuf[i] = (char)~i;
		}
		if (clen)
			printf("\ntransmit single %s packet of %d bytes (%d-byte DMA chunks)\n",
			       (ctl ? "ctl" : "data"), len, clen);
		else
			printf("\ntransmit single %s packet of %d bytes",
			       (ctl ? "ctl" : "data"), len);
		sdpcmd_transmit(drv->sdpcmd, bigbuf, (uint)len,
		                (clen ? (uint)clen : (uint)len), ctl);
	}

	printf("transmit done\n");
}

void
sdpcmd_bus_dumpregs(void)
{
	drv_t *drv = sdpcmd_dev.softc;
	struct bcmstrbuf b;
	char buf[BUS_DBG_BUFSZ];

	bcm_binit(&b, buf, BUS_DBG_BUFSZ);
	sdpcmd_dumpregs(drv->sdpcmd, &b);
	printbig(b.origbuf);
}

void
sdpcmd_bus_loopback(void)
{
	drv_t *drv = sdpcmd_dev.softc;
	char *buf = "abcdefghijklmnopqrstuvwxyz0123456789A123";

	printf("\nloopback of single packet containing ");
	printf(buf);
	printf("\n");
	sdpcmd_loopback(drv->sdpcmd, buf, strlen(buf) + 1);
	printf("loopback done\n");
}

uint
sdpcmd_bus_msgbits(uint *newbits)
{
	if (newbits)
		sd_msg_level = (int)*newbits;
	return (uint)sd_msg_level;
}
#endif /* BCMDBG */

#ifdef DS_PROT
#ifdef BCMDBG
/* Public sdpcmd debug log interface */
extern struct dngl_bus *g_sdpcmd;

void
sdpcmd_ds_log(uint16 type, uint8 a, uint8 b, uint8 c)
{
	if (g_sdpcmd) {
		_sdpcmd_ds_log(g_sdpcmd, type, a, b, c);
	}
}
#endif /* BCMDBG */
#endif /* DS_PROT */
