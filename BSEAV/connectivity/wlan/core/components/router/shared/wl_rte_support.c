/*
 * Wireless network adapter utilities (RTE-specific)
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

#include <typedefs.h>
#include <osl.h>
#include <wlioctl.h>
#include <wlutils.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <hnd_dev.h>

int
wl_ioctl(char *pDevName, int cmd, void *buf, int len)
{
	hnd_dev_t *dev = NULL;

	if (pDevName == NULL || *pDevName == NULL)
		return NULL;

	if (!(dev = hnd_get_dev(pDevName)))
		return NULL;

	return dev->ops->ioctl(dev, cmd, buf, len, NULL, NULL, 0);
}

int
wl_hwaddr(char *pDevName, unsigned char *hwaddr)
{
	return wl_ioctl(pDevName, RTEGHWADDR, hwaddr, ETHER_ADDR_LEN);
}
