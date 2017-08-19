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

const uint32 dlcode[] = {
	/* 0x1000: */	0xe59f0028, 0xe59f1028, 0xe5810000, 0xe59f0024,
	/* 0x1010: */	0xe5810004, 0xe59f0020, 0xe5810008, 0xe3e00000,
	/* 0x1020: */	0xe581000c, 0xe59f0014, 0xe5810010, 0xeafffffe,
	/* 0x1030: */	0xaa5555aa, 0x00010100, 0x55aaaa55, 0x11111111,
	/* 0x1040: */	0xdeadbeef, 0, 0, 0, 0};

void
_c_main(unsigned long ra)
{
	const uint32 *src;
	uint32	*dst;
	bool vsim, qt;
	char chn[8];

	BCMDBG_TRACE(0x544a0000);

	/* Basic initialization */
	sih = hnd_init();

	BCMDBG_TRACE(0x544a0001);

	vsim = sih->chippkg == HDLSIM_PKG_ID;
	qt = sih->chippkg == HWSIM_PKG_ID;

#ifndef	RTE_UNCACHED
	/* Initialize and turn caches on */
	caches_on();
#endif

	BCMDBG_TRACE(0x544a0002);

	printf("%s ver %s compiled at %s on %s\n",
	       __FILE__, EPI_VERSION_STR, __TIME__, __DATE__);
	printf("Running on a %s%s Rev. %d @ %d/%d/%d/%d Mhz.\n",
	       bcm_chipname(sih->chip, chn, 8), vsim ? "(vsim)" : (qt ? "(qt)" : ""),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       si_clock(sih) / 1000000, si_alp_clock(sih) / 1000000);

	BCMDBG_TRACE(0x544a0003);

	dst = (uint32 *)(64 * 1024);
	src = dlcode;
	while (*src)
		*dst++ = *src++;

	BCMDBG_TRACE(0x544a0004);

	hnd_cpu_jumpto((void *)(64 * 1024));
}
