/*
 * Broadcom Dongle Host Driver (DHD), pcie-specific functionality *
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
 /* FILE-CSTYLED */


/* include files */
#include <typedefs.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <siutils.h>
#include <hndsoc.h>
#include <hndpmu.h>
#include <sbchipc.h>
#if defined(DHD_DEBUG)
#include <hnd_armtrap.h>
#include <hnd_cons.h>
#endif /* defined(DHD_DEBUG) */
#include <dngl_stats.h>
#include <pcie_core.h>
#include <dhd.h>
#include <dhd_bus.h>
#include <dhd_proto.h>
#include <dhd_dbg.h>
#include <dhdioctl.h>
#include <bcmmsgbuf.h>
#include <pcicfg.h>
#include <circularbuf.h>
#include <dhd_pcie.h>

int
dhdpcie_bus_probe(shared_info_t *sh, void* bar, void** regsva, uint irq, void ** dhd, void * wl)
{

	int				ret = -1;
	void			* dhd_bus = NULL;
	void			* pci_dev = NULL;

	/* try to attach to the target device */
	if (!(dhd_bus = dhdpcie_bus_attach(sh->osh, *regsva, bar, pci_dev))) {
		printf("%s: device attach failed\n", __FUNCTION__);
		goto err;
	}

	ret = 0;

	/* error handling */
err:
	* dhd = dhd_bus;
	return ret;
}

void
dhdpcie_bus_remove(osl_t *osh, void *instance, shared_info_t *sh)
{
	if (sh) {
		dhdpcie_bus_release(instance);
	}
}

int
dhdpcie_bus_register(void)
{
	return 0;
}

void
dhdpcie_bus_unregister(void)
{
}

/* Free Linux irq */
void
dhdpcie_free_irq(dhd_bus_t *bus)
{
	return;
}

int
dhdpcie_disable_irq_nosync(dhd_bus_t *bus)
{
	return 0;
}

int
dhdpcie_disable_irq(dhd_bus_t *bus)
{
	return 0;
}

int
dhdpcie_enable_irq(dhd_bus_t *bus)
{
	return 0;
}

int dhdpcie_pci_suspend_resume(dhd_bus_t *bus, bool state)
{
	if (state) {
		/* Make sure Interrpt Mask is disabled whenever Entering D3 */
		dhdpcie_bus_intr_disable(bus);
	}
	else {
		/* Make sure Interrpt Mask is enabled whenever Entering D0 */
		dhdpcie_bus_intr_enable(bus);
	}
	return 0;
}
