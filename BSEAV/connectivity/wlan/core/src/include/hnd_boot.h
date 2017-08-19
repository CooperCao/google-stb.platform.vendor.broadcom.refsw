/*
 * Device boot/startup definitions.
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
 */

#ifndef _hnd_boot_h_
#define _hnd_boot_h_

#include <typedefs.h>

#ifdef	BCMDBG
#if defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
#define	BCMDBG_TRACE(x)		__watermark = (x)
#else
#define	BCMDBG_TRACE(x)
#endif	/* !__arm__ && !__thumb__ && !__thumb2__ */
#else
#define	BCMDBG_TRACE(x)
#endif	/* BCMDBG */

extern volatile uint __watermark;

#endif /* _hnd_boot_h_ */
