/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 *  Power management API for Broadcom STB/DTV peripherals
 *
 *   when        who         what
 *   -----       ---         ----
 *   20071030    cernekee    initial version
 *   20080303    cernekee    add TP1 shutdown
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef _H_PMLIB_
#define _H_PMLIB_

#include <stdbool.h>

/**
NOTE: This file is copied from the linux 2.6.18 source tree.
It's original location is Ref_Linux_Kernel/uclinux-rootfs/user/brcm-pm.
This file will be refactored in a non-backward-compatible way for the linux 2.6.31 release.
**/

/*
 * For all read/write fields:
 *
 * BRCM_PM_UNDEF from brcm_pm_get_status() means Not Available
 * BRCM_PM_UNDEF to brcm_pm_set_status() means Don't Touch
 */

#define BRCM_PM_UNDEF       -1

struct brcm_pm_state
{
	int usb_status;		/* 1=on, 0=off */
	int enet_status;	/* 1=on, 0=off */
	int moca_status;	/* 1=on, 0=off */
	int sata_status;	/* 1=on, 0=off */
	int tp1_status;		/* 1=on, 0=off */

	int cpu_base;		/* current base frequency, in Hz */
	int cpu_divisor;	/* 1, 2, 4, or 8 */
	int cpu_pll;		/* 0=normal, >0 = power save modes */

	int ddr_timeout;	/* 0=no PM, >0 = timeout for self-refresh */

	int standby_flags;
};

struct brcm_pm_cfg
{
	int use_dhcp;		/* stop/start dhcpcd */
};
void *brcm_pm_init(void);
void brcm_pm_close(void *);

int brcm_pm_get_cfg(void *ctx, struct brcm_pm_cfg *);
int brcm_pm_set_cfg(void *ctx, struct brcm_pm_cfg *);

int brcm_pm_get_status(void *ctx, struct brcm_pm_state *);
int brcm_pm_set_status(void *ctx, struct brcm_pm_state *);

#define BRCM_PM_STANDBY		1
#define BRCM_PM_SUSPEND		3
#define BRCM_PM_HIBERNATE	4
#define BRCM_PM_IRW_HALT	5

int brcm_pm_suspend(void *ctx, int suspend_mode);

/*************************************************

The following brcm_pm_standby and related functions only apply to 2.6.18.
A new technique will be supported in future 2.6.31 releases.

*************************************************/

/**
Summary:
Input to brcm_pm_standby
**/
struct brcm_pm_standby_settings
{
    unsigned timeout; /* in seconds. 0 = no timeout. */
    bool ir;          /* allow wakeup by IR input interrupt */
    bool keypad;      /* allow wakeup by keypad interrupt */
    bool gpio;        /* allow wakeup by GPIO interrupt */
};

/**
Summary:
Output from brcm_pm_standby about what ended standby state
**/
struct brcm_pm_standby_result
{
    bool timeout;
    bool ir;
    bool keypad;
    bool gpio;
};

/**
Summary:
Get default settings
**/
void brcm_pm_get_default_standby_settings(
    struct brcm_pm_standby_settings *p_settings /* [out] */
    );

/**
Summary:
Put linux into a low-power standby state
**/
int brcm_pm_standby(
    void *ctx,
    const struct brcm_pm_standby_settings *p_settings,
    struct brcm_pm_standby_result *p_result /* [out] */
    );

#endif /* _H_PMLIB_ */
