/*
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

#ifndef	_h_sdio_linux
#define	_h_sdio_linux

#include <linux/list.h>

struct sdio_dev {
	struct list_head global_list;	/* node in list of all PCI devices */
	struct list_head bus_list;	/* node in per-bus list */

	unsigned short	vendor;
	unsigned short	device;

	unsigned int	rca;		/* device rca */

	struct sdio_driver *driver;	/* which driver has allocated this device */
	void	*driver_data;	/* data private to the driver */
	/*
	 * Instead of touching interrupt line and base address registers
	 * directly, use the values stored here. They might be different!
	 */
	unsigned int	irq;

	char		name[90];	/* device name */
	char		slot_name[8];	/* slot name */
};

struct sdio_device_id {
	unsigned int id;		/* SDIO device ID or SDIO_ANY_ID */
};

struct sdio_driver {
	struct list_head node;
	char *name;
	const struct sdio_device_id *id_table;	/* NULL if wants all devices */
	/* New device inserted */
	int  (*probe)  (struct sdio_dev *dev, const struct sdio_device_id *id);
	/* Device removed (NULL if not a hot-plug capable driver) */
	void (*remove) (struct sdio_dev *dev);
	int  (*suspend) (struct sdio_dev *dev, u32 state);	/* Device suspended */
	int  (*resume) (struct sdio_dev *dev);	                /* Device woken up */
};

#define	SDIO_ANY_ID (~0)

#define sdio_dev_g(n) list_entry(n, struct sdio_dev, global_list)

#define sdio_for_each_dev(dev) \
	for (dev = sdio_dev_g(sdio_devices.next); dev != sdio_dev_g(&sdio_devices); \
	dev = sdio_dev_g(dev->global_list.next))
#define sdio_for_each_dev_reverse(dev) \
	for (dev = sdio_dev_g(sdio_devices.prev); dev != sdio_dev_g(&sdio_devices); \
	dev = sdio_dev_g(dev->global_list.prev))

/* config register r/w */
extern int sdio_read_config_byte(struct sdio_dev *dev, int where, u8 *val);
extern int sdio_write_config_byte(struct sdio_dev *dev, int where, u8 val);

/* init and exit */
extern int sdio_register_driver(struct sdio_driver *drv);
extern void sdio_unregister_driver(struct sdio_driver *drv);

/* error handling */

/* helper functions */

#endif /* _h_sdio_linux */
