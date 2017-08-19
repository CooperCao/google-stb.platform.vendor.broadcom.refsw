/*
 * JTAG access interface for drivers - linux specific (pci only)
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

#define __UNDEF_NO_VERSION__

#include <typedefs.h>
#include <linuxver.h>
#include <bcmdefs.h>

#include <linux/pci.h>
#include <linux/completion.h>
#include <linux/delay.h>

#include <osl.h>
#include <pcicfg.h>
#include <bcmdevs.h>
#include <bcmjtag.h>

#ifndef BCMJTAG_SBCOREID
#include <hndsoc.h>
#endif /* BCMJTAG_SBCOREID */

/* target device info */
typedef struct bcmjtag_device bcmjtag_device_t;
struct bcmjtag_device {
	bcmjtag_device_t *next;
	void *dh;			/* target device handle */
	struct completion poll;		/* device polling thread completion flag */
	bool stop;			/* device polling thread stop flag */
};

/* jtag master info */
typedef struct bcmjtag_master bcmjtag_master_t;
struct bcmjtag_master {
	bcmjtag_master_t *next;
	struct pci_dev *dev;		/* pci device handle */
	osl_t *osh;
	void *regs;			/* jtag master address */
	bcmjtag_info_t *ih;		/* jtag master handle */
	bcmjtag_device_t *dh;		/* target device handle */
};
static bcmjtag_master_t *jtminfo = NULL;

/* driver info, initialized when bcmjtag_register is called */
static bcmjtag_driver_t drvinfo = {NULL, NULL, NULL};

/* debugging macros */
#ifdef BCMDBG_ERR
#define JTLX_MSG(x)	printf x
#else
#define JTLX_MSG(x)
#endif /* BCMDBG_ERR */

/* forward declaraions */
static int bcmjtag_device_poll(void *arg);
static bool bcmjtag_device_attach(void *arg, uint16 venid, uint16 devid, void *devregs);
static int __devinit bcmjtag_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static void __devexit bcmjtag_remove(struct pci_dev *pdev);

/* exported varibless */
#ifndef BCMJTAG_SBCOREID
#define BCMJTAG_SBCOREID	D11_CORE_ID
#endif /* BCMJTAG_SBCOREID */
static uint sbcoreid = BCMJTAG_SBCOREID; /* sbcore type id */
static uint htdiffend = 0;		/* host and target have different endianness */
static uint devpollint = 10;		/* target device polling internal in ms. */
static uint devpollmax = 4;		/* max # frames to poll once */
module_param(sbcoreid, int, 0);
module_param(htdiffend, int, 0);
module_param(devpollint, int, 0);
module_param(devpollmax, int, 0);

/* poll the target device. no interrupt available! */
static int
bcmjtag_device_poll(void *arg)
{
	bcmjtag_device_t *jtdh = (bcmjtag_device_t *)arg;
	long interval = devpollint * HZ / 1000;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
	DECLARE_WAIT_QUEUE_HEAD(wait);

	daemonize();

	do {
		drvinfo.poll(jtdh->dh);
	        init_waitqueue_head(&wait);
		sleep_on_timeout(&wait, interval);
	} while (!jtdh->stop);
#else
	daemonize("bcmjtag");

	do {
		drvinfo.poll(jtdh->dh);
		OSL_SLEEP(interval);
	} while (!jtdh->stop);
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)) */

	complete_and_exit(&jtdh->poll, 0);
	return 0;
}

/* callback function for bcmjtag_devattach */
/*
* attach to the target device identified by devid and devunit, create a
* kernel thread to poll the target device. devregs is host accessible
* memory address. venid and devid are pci ids.
*/
static bool
bcmjtag_device_attach(void *arg, uint16 venid, uint16 devid, void *devregs)
{
	bcmjtag_master_t *jtmh = (bcmjtag_master_t *)arg;
	bcmjtag_device_t *jtdh;

	/* allocate target device state */
	if (!(jtdh = MALLOC(jtmh->osh, sizeof(bcmjtag_device_t)))) {
		JTLX_MSG(("bcmjtag_device_attach: out of memory, allocated %d bytes\n",
			MALLOCED(jtmh->osh)));
		goto err;
	}

	/* try to attach to the target device */
	if (!(jtdh->dh = drvinfo.attach(venid, devid, devregs, (void *)(uintptr)devpollmax))) {
		JTLX_MSG(("bcmjtag_device_attach: device attach failed\n"));
		goto err;
	}

	/* create a thread to poll the target device */
	jtdh->stop = 0;
	init_completion(&jtdh->poll);
	kernel_thread(bcmjtag_device_poll, jtdh, 0);

	/* chain target device info together */
	jtdh->next = jtmh->dh;
	jtmh->dh = jtdh;

	return TRUE;

	/* error handling */
err:
	if (jtdh)
		MFREE(jtmh->osh, jtdh, sizeof(bcmjtag_device_t));
	return FALSE;
}

/* attach to jtag master and the target devices with type specified in 'sbcoreid' */
static int __devinit
bcmjtag_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	osl_t *osh = NULL;
	int rc;
	bcmjtag_master_t *jtmh = NULL;
	void *regs = NULL;
	bcmjtag_info_t *jtih = NULL;

	/* match this pci device with what we support */
	/* we can't solely rely on this to believe it is our jtag master! */
	if (!bcmjtag_chipmatch(pdev->vendor, pdev->device))
		return -ENODEV;

	/* this is a pci device we support! */
	JTLX_MSG(("bcmjtag_pci_probe: bus %d slot %d func %d irq %d\n",
	          pdev->bus->number, PCI_SLOT(pdev->devfn),
	          PCI_FUNC(pdev->devfn), pdev->irq));

	/* allocate jtag master state info */
	if (!(osh = osl_attach(pdev, JTAG_BUS, FALSE))) {
		JTLX_MSG(("bcmjtag_pci_probe: osl_attach failed\n"));
		goto err;
	}
	if (!(jtmh = MALLOC(osh, sizeof(bcmjtag_master_t)))) {
		JTLX_MSG(("bcmjtag_pci_probe: out of memory, allocated %d bytes\n",
			MALLOCED(osh)));
		goto err;
	}
	bzero(jtmh, sizeof(bcmjtag_master_t));
	jtmh->osh = osh;

	jtmh->dev = pdev;

	/* map to address where host can access */
	pci_set_master(pdev);
	rc = pci_enable_device(pdev);
	if (rc) {
		JTLX_MSG(("%s: Cannot enable PCI device\n", __FUNCTION__));
		goto err;
	}
	if (!(regs = (void *)(uintptr)pci_resource_start(pdev, 0))) {
		JTLX_MSG(("bcmjtag_pci_probe: pci_resource_start failed\n"));
		goto err;
	}
	if (!(regs = ioremap_nocache((unsigned long)regs, PCI_BAR0_WINSZ))) {
		JTLX_MSG(("bcmjtag_pci_probe: ioremap_nocache failed\n"));
		goto err;
	}
	jtmh->regs = regs;

	/* attach to the pci device. will get a handle if it is our jtag master */
	if (!(jtih = bcmjtag_attach(osh, pdev->vendor, pdev->device,
	                regs, PCI_BUS, pdev, (bool)htdiffend))) {
		JTLX_MSG(("bcmjtag_pci_probe: bcmjtag_attach failed\n"));
		goto err;
	}
	jtmh->ih = jtih;

	/* attach to the target devices that jtag can reach */
	if (!bcmjtag_devattach(jtih, SB_VEND_BCM, (uint16)sbcoreid, bcmjtag_device_attach, jtmh)) {
		JTLX_MSG(("bcmjtag_pci_probe: no device attached\n"));
		goto err;
	}

	/* chain jtag master info together */
	jtmh->next = jtminfo;
	jtminfo = jtmh;

	return 0;

	/* error handling */
err:
	if (jtih)
		bcmjtag_detach(jtih);
	if (regs)
		iounmap(regs);
	if (jtmh)
		MFREE(osh, jtmh, sizeof(bcmjtag_master_t *));
	if (osh)
		osl_detach(osh);
	return -ENODEV;
}

/* stop polling threads, detach from target devices and jtag master */
static void __devexit
bcmjtag_remove(struct pci_dev *pdev)
{
	bcmjtag_master_t *jtmh, *prev;
	bcmjtag_device_t *jtdh, *next;
	osl_t *osh;

	/* find the jtag master state for this pdev and take it out from the list */
	for (jtmh = jtminfo, prev = NULL; jtmh; jtmh = jtmh->next) {
		if (jtmh->dev == pdev) {
			if (prev)
				prev->next = jtmh->next;
			else
				jtminfo = NULL;
			break;
		}
		prev = jtmh;
	}
	if (!jtmh)
		return;

	/* kill all polling threads and detach from all target devices */
	for (jtdh = jtmh->dh, next = NULL; jtdh; jtdh = next) {
		/* kill the polling thread and wait till it is indeed dead */
		jtdh->stop = 1;
		wait_for_completion(&jtdh->poll);
		/* detach from the target device */
		drvinfo.detach(jtdh->dh);
		/* free the memory */
		next = jtdh->next;
		MFREE(jtmh->osh, jtdh, sizeof(bcmjtag_device_t));
	}

	/* detach from the jtag master */
	bcmjtag_detach(jtmh->ih);

	/* unmap memory */
	iounmap(jtmh->regs);

	/* release jtag master info */
	osh = jtmh->osh;
	MFREE(osh, jtmh, sizeof(bcmjtag_master_t));
	osl_detach(osh);
}

/* pci id table */
static struct pci_device_id bcmjtag_pci_devid[] __devinitdata = {
	{ vendor: PCI_ANY_ID,
	device: PCI_ANY_ID,
	subvendor: PCI_ANY_ID,
	subdevice: PCI_ANY_ID,
	class: 0,
	class_mask: 0,
	driver_data: 0,
	},
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, bcmjtag_pci_devid);

/* jtag master pci driver info */
static struct pci_driver bcmjtag_pci_driver = {
	node:		{},
	name:		"bcmjtag",
	id_table:	bcmjtag_pci_devid,
	probe:		bcmjtag_pci_probe,
	remove:		bcmjtag_remove,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
	save_state:	NULL,
#endif
	suspend:	NULL,
	resume:		NULL,
	enable_wake:	NULL,
	};

/* attach to jtag master and target devices */
int
bcmjtag_register(bcmjtag_driver_t *driver)
{
	int error;

	drvinfo = *driver;
	if (!(error = pci_module_init(&bcmjtag_pci_driver)))
		return 0;

	JTLX_MSG(("bcmjtag_register: pci_module_init failed\n"));
	return error;
}

/* detach from target devices and jtag master */
void
bcmjtag_unregister(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
	if (bcmjtag_pci_driver.node.next)
#endif
		pci_unregister_driver(&bcmjtag_pci_driver);
}
