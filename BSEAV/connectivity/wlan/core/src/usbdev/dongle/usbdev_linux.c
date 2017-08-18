/*
 * USB device Linux interface
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

#define __UNDEF_NO_VERSION__

#include <typedefs.h>

#include <linux/module.h>

#include <osl.h>
#include <linuxver.h>
#include <epivers.h>
#include <bcmutils.h>

#include <linux/pci.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

#include <asm/irq.h>
#include <asm/uaccess.h>

typedef const struct si_pub si_t;

#include <usbdev.h>
#include <usbdev_dbg.h>

#include <dngl_bus.h>
#include <dngl_api.h>

typedef struct {
	struct net_device_stats stats;
	struct net_device dev;
	struct usbdev_chip *ch;
	void *regs;
	osl_t *osh;
	int irq;
	spinlock_t lock;
	struct dngl *dngl;
	struct tasklet_struct tasklet;	/* dpc tasklet */
	int unit;
} drv_t;

#define DRV_INFO(dev)	(drv_t *)(dev->priv)

#ifdef BCMDBG
int usbdev_msglevel = USBDEV_ERROR;
MODULE_PARM(usbdev_msglevel, "i");
#endif

static void
usbdev_dpc(ulong data)
{
	drv_t *drv = (drv_t *)data;
	if (ch_dpc(drv->ch))
		tasklet_schedule(&drv->tasklet);
	else
		ch_intrson(drv->ch);
}

static irqreturn_t
usbdev_isr(int irq, void *devid, struct pt_regs *ptregs)
{
	drv_t *drv = (drv_t *)devid;
	if (ch_dispatch(drv->ch)) {
		ch_intrsoff(drv->ch);
		tasklet_schedule(&drv->tasklet);
		return IRQ_RETVAL(TRUE);
	}
	return IRQ_RETVAL(FALSE);
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
BCMATTACHFN(usbdev_open)(struct net_device *dev)
{
	drv_t *drv = DRV_INFO(dev);

	err("init chip");

	ch_init(drv->ch);

	dngl_opendev(drv->dngl);

	OLD_MOD_INC_USE_COUNT;

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
BCMUNINITFN(usbdev_close)(struct net_device *dev)
{
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	drv_t *drv = DRV_INFO(dev);
#endif

	netif_down(dev);
	netif_stop_queue(dev);

	OLD_MOD_DEC_USE_COUNT;

	dbg("drv exit%d", drv->unit);
	return 0;
}

/*
 *
 *  Write a packet to the USB bus.
 *
 *  Input parameters:
 *		dev - device context (includes ptr to our softc)
 *		buffer - pointer to buffer descriptor.
 *
 *  Return value:
 *		status, 0 = ok
 */

static int
usbdev_start(struct sk_buff *skb, struct net_device *dev)
{
	drv_t *drv = DRV_INFO(dev);

	trace("drv%dwrite", drv->unit);

	return dngl_sendpkt((void *)(drv->dngl), (void *)skb);
}

/* Get device stats */
static struct net_device_stats *
usbdev_get_stats(struct net_device *dev)
{
	drv_t *drv = DRV_INFO(dev);

	return &drv->stats;
}

static int
usbdev_set_mac_address(struct net_device *dev, void *addr)
{
	struct sockaddr *sa = (struct sockaddr *)addr;

	bcopy(sa->sa_data, dev->dev_addr, 6);

	return 0;
}

static void
usbdev_set_multicast_list(struct net_device *dev)
{
}

static int
usbdev_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	return 0;
}

static struct pci_device_id usbdev_pci_id_table[] __devinitdata = {
	{
		vendor: PCI_ANY_ID,
		device: PCI_ANY_ID,
		subvendor: PCI_ANY_ID,
		subdevice: PCI_ANY_ID,
		class: PCI_CLASS_SERIAL_USB << 8,
		class_mask: 0xffff00,
		driver_data: 0
	},
	{ }
};
MODULE_DEVICE_TABLE(pci, usbdev_pci_id_table);

#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *proc_usbdev = NULL;

static int
usbdev_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	drv_t *drv = (drv_t *) data;
	unsigned long flags;

	trace("usbdev_read_proc");
	spin_lock_irqsave(&drv->lock, flags);
	count = (int)(ch_dumpregs(drv->ch, page) - page);
	spin_unlock_irqrestore(&drv->lock, flags);

	return count;
}

static int
usbdev_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	drv_t *drv = (drv_t *) data;
	char *buf;
	unsigned long flags;

	trace("usbdev_write_proc");

	if (!(buf = MALLOC(drv->osh, count))) {
		printf("usbdev_write_proc: out of memory, malloced %d bytes\n", MALLOCED(drv->osh));
		return -ENOMEM;
	}

	if (copy_from_user(buf, buffer, count)) {
		MFREE(drv->osh, buf, count);
		return -EFAULT;
	}

	spin_lock_irqsave(&drv->lock, flags);
	count = (unsigned long) ch_loopback(drv->ch, buf, count);
	spin_unlock_irqrestore(&drv->lock, flags);

	MFREE(drv->osh, buf, count);
	return (int) count;
}
#endif	/* CONFIG_PROC_FS */

static int __devinit
usbdev_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	drv_t *drv;
	osl_t *osh;

	trace("usbdev_pci_probe");

	if (!ch_match(pdev->vendor, pdev->device))
		return -ENODEV;

	/* Enable device */
	pci_set_master(pdev);
	pci_enable_device(pdev);

	/* Attach to osl */
	osh = osl_attach(pdev, PCI_BUS, FALSE);
	ASSERT(osh);

	/* Allocate driver structure */
	if (!(drv = MALLOC(osh, sizeof(drv_t)))) {
		printf("usbdev_pci_probe: out of memory, malloced %d bytes\n", MALLOCED(osh));
		osl_detach(osh);
		return -ENOMEM;
	}
	bzero(drv, sizeof(drv_t));
	pci_set_drvdata(pdev, drv);
	drv->osh = osh;

	tasklet_init(&drv->tasklet, usbdev_dpc, (ulong)drv);
	spin_lock_init(&drv->lock);

	/* Map chip registers */
	if (!(drv->regs = ioremap_nocache(pci_resource_start(pdev, 0),
	                                  pci_resource_len(pdev, 0)))) {
		printf("error mapping registers\n");
		goto fail;
	}

	/* Allocate chip state */
	if (!(drv->ch = ch_attach(drv, pdev->vendor, pdev->device, osh, drv->regs, PCI_BUS))) {
		printf("ch_attach() failed\n");
		goto fail;
	}
	drv->dngl = ch_dngl(drv->ch);

	/* Register interrupt handler */
	if (request_irq(pdev->irq, usbdev_isr, SA_SHIRQ, "usbdev", drv)) {
		printf("error requesting IRQ %d\n", pdev->irq);
		goto fail;
	}
	drv->irq = pdev->irq;

	/* Enable our entry points */
	drv->dev.open = usbdev_open;
	drv->dev.stop = usbdev_close;
	drv->dev.hard_start_xmit = usbdev_start;
	drv->dev.get_stats = usbdev_get_stats;
	drv->dev.set_mac_address = usbdev_set_mac_address;
	drv->dev.set_multicast_list = usbdev_set_multicast_list;
	drv->dev.do_ioctl = usbdev_ioctl;
	if (register_netdev(&drv->dev)) {
		printf("register_netdev() failed\n");
		goto fail;
	}
	ether_setup(&drv->dev);
	if (netif_queue_stopped(&drv->dev))
		netif_stop_queue(&drv->dev);
	drv->dev.priv = drv;

	/* print hello string */
	printf("%s: Broadcom BCM%04X USB Controller " EPI_VERSION_STR,
	       drv->dev.name, pdev->device);
#ifdef BCMDBG
	printf(" (Compiled in " SRCBASE " at " __TIME__ " on " __DATE__ ")");
#endif /* BCMDBG */
	printf("\n");

#ifdef CONFIG_PROC_FS
{
	struct proc_dir_entry *res;

	/* Create /proc/usbdev */
	res = create_proc_read_entry(pdev->slot_name, S_IFREG | 0444,
	                             proc_usbdev, usbdev_read_proc, drv);
	if (!res) {
		printf("error creating /proc/usbdev/%s\n", pdev->slot_name);
		goto fail;
	}
	res->write_proc = usbdev_write_proc;
}
#endif	/* CONFIG_PROC_FS */

	return 0;

fail:
	if (drv->ch)
		ch_detach(drv->ch, TRUE);
	if (drv->regs)
		iounmap(drv->regs);
	if (drv)
		MFREE(osh, drv, sizeof(drv_t));
	osl_detach(osh);
	return -ENODEV;
}

static void __devexit
usbdev_pci_remove(struct pci_dev *pdev)
{
	drv_t *drv;
	osl_t *osh;

	trace("usbdev_pci_remove");
	if (!ch_match(pdev->vendor, pdev->device))
		return;

	if (!(drv = (drv_t *) pci_get_drvdata(pdev)))
		return;

#ifdef CONFIG_PROC_FS
	remove_proc_entry(pdev->slot_name, proc_usbdev);
#endif
	unregister_netdev(&drv->dev);
	ch_detach(drv->ch, TRUE);
	free_irq(drv->irq, drv);
	iounmap(drv->regs);
	tasklet_kill(&drv->tasklet);
	pci_set_drvdata(pdev, NULL);
	osh = drv->osh;
	MFREE(osh, drv, sizeof(drv_t));
	ASSERT(MALLOCED(osh) == 0);
	osl_detach(osh);
}

static struct pci_driver usbdev_pci_driver = {
	name:		"usbdev",
	probe:		usbdev_pci_probe,
	remove:		__devexit_p(usbdev_pci_remove),
	id_table:	usbdev_pci_id_table
};

static int __init
usbdev_module_init(void)
{
	trace("usbdev_module_init");
#ifdef CONFIG_PROC_FS
	proc_usbdev = proc_mkdir("usbdev", &proc_root);
#endif
	return pci_module_init(&usbdev_pci_driver);
}

static void __exit
usbdev_module_exit(void)
{
	trace("usbdev_module_exit");
	pci_unregister_driver(&usbdev_pci_driver);
#ifdef CONFIG_PROC_FS
	if (proc_usbdev)
		remove_proc_entry("usbdev", &proc_root);
#endif
}

module_init(usbdev_module_init);
module_exit(usbdev_module_exit);
