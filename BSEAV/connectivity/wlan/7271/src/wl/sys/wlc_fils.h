/*
 * FILS Authentication for OCE declarations/definitions for
 * Broadcom 802.11abgn Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 *
 */

/**
*	OCE uses the FILS shared key authentication 11.11.2.3.1 and
*	FILS Higher Layer Setup with Higher Layer Protocol Encapsulation (10.47.3)
*	from 802.11ai D6.0

*	Place holder for fils authentication module.

*	For latest design proposal, please refer to
*	http://confluence.broadcom.com/display/WLAN/FILS+Authentication+for+OCE

*	The file names are kept as wlc_filsauth.c / wlc_filsauth.h.
*	802.11ai in full is not a requirement for OCE.
*	OCE has a pre-requisite for FILS authentication.

*	In future, if 802.11ai full implementation is needed, we can have a wrapper file
*	like wlc_fils.c / wlc_fils.h which will include wlc_filsauth.c/.h file to cover
*	up the FILS authentication. Having wlc_fils.c/.h for now will be misleading.
*/

#ifndef _wlc_filsauth_h_
#define _wlc_filsauth_h_
wlc_fils_info_t * wlc_fils_attach(wlc_info_t *wlc);
void wlc_fils_detach(wlc_fils_info_t* fils);
void wlc_fils_free_ies(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
#endif	/* _wlc_filsauth_h_ */
