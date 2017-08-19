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
#include <bcmnvram.h>
#include <hndcpu.h>
#include <epivers.h>
#include <bcmutils.h>

#include <rte.h>

void _c_main(unsigned long ra);

static si_t *sih;

static char *vars[] = {
	"boardtype",	"boardnum",	"boardrev",	"boardflags",
	"xtalfreq",
	"sromrev",	"wl0id",	"il0macaddr",	"aa0",
	"ag0",		"pa0maxpwr",	"notthere",	"newvalue",
	"et0macaddr",	"et1macaddr",
	NULL
};

void
_c_main(unsigned long ra)
{
	int i;
	char *v, *s;
	bool vsim, qt;
	char chn[8];

	/* Basic initialization */
	sih = hnd_init();

	vsim = sih->chippkg == HDLSIM_PKG_ID;
	qt = sih->chippkg == HWSIM_PKG_ID;

	/* Initialize and turn caches on */
	caches_on();

	printf("%s ver %s compiled at %s on %s\n",
	       __FILE__, EPI_VERSION_STR, __TIME__, __DATE__);
	printf("Running on a %s%s Rev. %d @ %d/%d/%d/%d Mhz.\n",
	       bcm_chipname(sih->chip, chn, 8), vsim ? "(vsim)" : (qt ? "(qt)" : ""),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       si_clock(sih) / 1000000, si_alp_clock(sih) / 1000000);

	for (i = 0; (v = vars[i]) != NULL; i++) {
		s = nvram_get(v);
		if (s == NULL)
			printf("%s not found\n", v);
		else
			printf("%s=%s\n", v, s);
	}

	v = hnd_malloc(NVRAM_SPACE);
	if (nvram_getall(v, NVRAM_SPACE) != 0) {
		printf("nvram_getall failed\n");
	} else {
		printf("nvram_getall:\n");
		s = v;
		while (*s) {
			printf("  %s\n", s);
			s += strlen(s) + 1;
		}
	}

	hnd_free(v);
	printf("That's all folks\n");
}
