/*
 * Initialization and support routines for self-booting
 * compressed image.
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <hndsoc.h>
#include <bcmdevs.h>
#include <osl.h>
#include <siutils.h>
#include <hndcpu.h>
#include <epivers.h>
#include <bcmutils.h>

#include <rte.h>

#include <hnd_boot.h>

void _c_main(unsigned long ra);

static si_t *sih;

void
_c_main(unsigned long ra)
{
	uint32	*base, *toaddr;
	bool vsim, qt;
	char chn[8];

	BCMDBG_TRACE(0x4a4a0000);

	/* Basic initialization */
	sih = hnd_init();

	BCMDBG_TRACE(0x4a4a0001);

	vsim = sih->chippkg == HDLSIM_PKG_ID;
	qt = sih->chippkg == HWSIM_PKG_ID;

#ifndef	RTE_UNCACHED
	/* Initialize and turn caches on */
	caches_on();
#endif

	BCMDBG_TRACE(0x4a4a0002);

	printf("%s ver %s compiled at %s on %s\n",
	       __FILE__, EPI_VERSION_STR, __TIME__, __DATE__);
	printf("Running on a %s%s Rev. %d @ %d/%d/%d/%d Mhz.\n",
	       bcm_chipname(sih->chip, chn, 8), vsim ? "(vsim)" : (qt ? "(qt)" : ""),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       si_clock(sih) / 1000000, si_alp_clock(sih) / 1000000);

	BCMDBG_TRACE(0x4a4a0003);

	toaddr = (uint32 *)NULL;
	base = toaddr;
	while (*base != 0xea000012)
		;

	BCMDBG_TRACE(0x4a4a0004);

	hnd_cpu_jumpto((void *)NULL);
}
