/*
 *  Defines BDC RTE header
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#ifndef _BDC_RTE_H_
#define _BDC_RTE_H_

#include <typedefs.h>
#include <siutils.h>
#include <bdc.h>
#include <usbclass.h>

#define BDC_ALIGN_BITS	4
#define UBDC_ALIGN_BITS	2
#define DL_GO		2

struct usbdev_chip {
	/* Generic defines; required for all usbdev_chip */
	void *drv;			/**< Driver handle */
	uint id;			/**< USB device core ID */
	uint rev;			/**< USB device core revision */
	osl_t *osh;			/**< os (Driver PCI) handle */
	si_t *sih;			/**< Silicon_backplane handle */
	struct dngl_bus *bus;		/**< high level USB bus logic handle */

	/* BDC RTE specific */
	uint8 non_disconnect;	/**< not re-enumerate after FW download */
	bool up;			/** Check whether device is up */

	/* BDC core Specific */
	struct _bdcd *bdc;
	usb_class_t *u_class;
};

int bdci_dpc(struct usbdev_chip *ch);
si_t * bdci_get_sih(struct usbdev_chip *ch);
bool bdci_match(uint vendor, uint device);
int bdci_tx(struct usbdev_chip *ch);
bool bdci_hsic(struct usbdev_chip *chip);
#ifdef USB_HUB
void *rdl_get_hub_drv(void *p);
#endif /* USB_HUB */

struct usbdev_chip * BCMATTACHFN(bdci_attach) (void *drv, uint vendor, uint device, osl_t * osh,
		volatile void *regs, uint bus);
uint32 BCMATTACHFN(bdci_init) (struct usbdev_chip *ch, bool disconnect);
void BCMATTACHFN(bdci_detach) (struct usbdev_chip *ch, bool disable);

#endif /* _BDC_RTE_H_ */
