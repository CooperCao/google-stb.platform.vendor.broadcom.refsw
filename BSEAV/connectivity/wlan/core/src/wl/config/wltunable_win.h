/*
 * Broadcom 802.11abg Networking Device Driver Configuration file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wltunable_win.h 368001 2013-09-06 18:30:03Z $
 *
 * wl driver tunables for Windows OS
 */

/* max delay for timer interrupt to fire. ~2ms delay noted with Windows
 * when C-states is enabled
 */
#define MCHAN_TIMER_DELAY_US        3000 /* Max delay for timer interrupt. */
#define RXBND                       32   /* max # frames to process in wlc_recv() */
