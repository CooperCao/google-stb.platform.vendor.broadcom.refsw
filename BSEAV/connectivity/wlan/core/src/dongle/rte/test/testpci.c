/*
 * Initialization and support routines for self-booting
 * compressed image.
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
#include <bcmdefs.h>
#include <osl.h>
#include <hndsoc.h>
#include <bcmdevs.h>
#include <hndcpu.h>
#include <siutils.h>
#include <epivers.h>
#include <rte_isr.h>
#include <rte_dev.h>
#include <bcmutils.h>

#include <rte.h>

void _c_main(unsigned long ra);

static si_t *sih;

static si_t *devsih;

static void tst_isr(void *drv);

/* Driver entry points */
static void *
tst_probe(hnd_dev_t *drv, volatile void *regs, uint bus, uint16 device, uint coreid, uint unit)
{
	osl_t *osh;

	printf("Probe called for device 0x%x on bus %d\n", device, bus);

	osh = osl_attach(drv);

	if ((devsih = si_attach(device, osh, regs, bus, drv, NULL, NULL)) == NULL) {
		printf("si_attach failed\n");
		return NULL;
	}
#ifndef RTE_POLL
	if (hnd_isr_register(0, coreid, unit, tst_isr, drv, NULL, NULL, NULL, bus)) {
		printf("hnd_isr_register failed\n");
		return NULL;
	}
#endif	/* !RTE_POLL */
	si_viewall(devsih, TRUE);

	return osh;
}

static void
tst_isr(void *drv)
{
	printf("%s called\n", __FUNCTION__);
}

static int
tst_open(hnd_dev_t *drv)
{
	printf("%s called\n", __FUNCTION__);
	return 0;
}

static int
tst_send(hnd_dev_t *src, hnd_dev_t *drv, struct lbuf *lb)
{
	printf("%s called\n", __FUNCTION__);
	return 0;
}

static int
tst_ioctl(hnd_dev_t *drv, uint32 cmd, void *buf, int len, int *used, int *needed, int set)
{
	printf("%s called\n", __FUNCTION__);
	return 0;
}

static int
tst_close(hnd_dev_t *drv)
{
	printf("%s called\n", __FUNCTION__);
	return 0;
}

static void
tst_poll(hnd_dev_t *dev)
{
	tst_isr(dev);
}

static hnd_dev_ops_t tst_funcs = {
	probe:		tst_probe,
	open:		tst_open,
	close:		tst_close,
	xmit:		tst_send,
	ioctl:		tst_ioctl,
	poll:		tst_poll
};

hnd_dev_t tstdev = {
	name:		"testpci",
	ops:		&tst_funcs
};

void
_c_main(unsigned long ra)
{
	int i;
	bool vsim, qt;
	char chn[8];

	/* Basic initialization */
	sih = hnd_init();

	vsim = sih->chippkg == HDLSIM_PKG_ID;
	qt = sih->chippkg == HWSIM_PKG_ID;

#ifndef	RTE_UNCACHED
	/* Initialize and turn caches on */
	caches_on();
#endif

	printf("%s ver %s compiled at %s on %s\n",
	       __FILE__, EPI_VERSION_STR, __TIME__, __DATE__);
	printf("Running on a %s%s Rev. %d @ %d/%d/%d/%d Mhz.\n",
	       bcm_chipname(sih->chip, chn, 8), vsim ? "(vsim)" : (qt ? "(qt)" : ""),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       si_clock(sih) / 1000000, si_alp_clock(sih) / 1000000);

	hnd_add_device(sih, &tstdev, CC_CORE_ID, 0x4999);

	if (!vsim)
		for (i = 0; TRUE; i++) {
			OSL_DELAY(1000000);
			printf("%d\n", i);
		}
}
