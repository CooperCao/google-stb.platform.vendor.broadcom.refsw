/*
 * DHD ioctl interface from user to kernel space.
 * User space application link against this to interface to
 * MacOS DHD driver.
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
 *
 */

#ifndef __DHDIOCTL_USR_H_
#define __DHDIOCTL_USR_H_

/* MacOS DHD driver ioctl interface
 * This uses MacOS's IOUserClient object to enable user apps to
 * interface to kernel drivers
 */
#define kBrcmDhdDriverClassName         "DhdEther"
enum
{
	kFuncIndex_open,
	kFuncIndex_close,
	kFuncIndex_wlioctl_hook,
	kFuncIndex_dhdioctl_hook,
	kNumberOfMethods
};

#ifndef MACOS_DHD_DONGLE
#define dhdioctl_usr_dongle(a, b, c, d, e, f) -1
#else
extern int dhdioctl_usr_dongle(void *dhd, int cmd, void *buf, int len, bool set, bool wl);
#endif

#endif /* __DHDIOCTL_USR_H_ */
