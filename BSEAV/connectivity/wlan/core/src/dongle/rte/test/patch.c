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
#include <bcmdevs.h>
#include <osl.h>
#include <siutils.h>
#include <hndcpu.h>
#include <epivers.h>
#include <bcmutils.h>


#include <rte.h>

#include <hnd_boot.h>

void _c_main(unsigned long ra);

void test_patch(void);

static si_t *sih;

void
_c_main(unsigned long ra)
{
	bool vsim, qt;
	char chn[8];

	BCMDBG_TRACE(0x434d0000);


	/* Basic initialization */
	sih = hnd_init();


	BCMDBG_TRACE(0x434d0001);

	vsim = sih->chippkg == HDLSIM_PKG_ID;
	qt = sih->chippkg == HWSIM_PKG_ID;

#ifndef	RTE_UNCACHED
	/* Initialize and turn caches on */
	caches_on();
#endif

	BCMDBG_TRACE(0x434d0002);

	printf("%s ver %s compiled at %s on %s\n",
	       __FILE__, EPI_VERSION_STR, __TIME__, __DATE__);
	printf("Running on a %s%s Rev. %d @ %d/%d/%d/%d Mhz.\n",
	       bcm_chipname(sih->chip, chn, 8), vsim ? "(vsim)" : (qt ? "(qt)" : ""),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       si_clock(sih) / 1000000, si_alp_clock(sih) / 1000000);

	test_patch();

	BCMDBG_TRACE(0x434d0003);
}

void
test_patch(void)
{
	char a = '1';
	printf("atoi(%c) is %d and ASCII number is %d\n", a, bcm_atoi(&a), a);
}
