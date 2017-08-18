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
#include <hndsoc.h>
#include <sbpcmcia.h>
#include <bcmsrom.h>
#include <hndcpu.h>
#include <epivers.h>
#include <siutils.h>
#include <bcmendian.h>
#include <bcmutils.h>

#include <rte.h>

void _c_main(unsigned long ra);

static si_t *sih;

void
_c_main(unsigned long ra)
{
	bool vsim, qt;
	int i, rc;
	char *v, *vars;
	uint nw, nb, count;
	uint16 *srom;
	uint8 *cis, *pcmregs;
	osl_t	*osh;
	char chn[8];


	/* Basic initialization */
	sih = hnd_init();
	osh = si_osh(sih);

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

	if ((pcmregs = (uint8 *)si_setcore(sih, PCMCIA_CORE_ID, 0)) == NULL) {
		printf("Cannot find the pcmcia core\n");
		return;
	}

	if ((i = si_corerev(sih)) < 8) {
		printf("Pcmcia core revision (%d) is less than 8\n", i);
		return;
	}

	nw = 256;	/* SROM_INFO == 1 means 4Kbit i.e. 512 bytes, 256 words */
	nb = 2 * nw;

	if ((srom = (uint16 *)hnd_malloc(nb)) == NULL) {
		printf("Cannot malloc memory for srom image\n");
		return;
	}

	rc = srom_read(sih, SI_BUS, pcmregs, osh, 0, nb, srom, FALSE);
	if (rc != 0) {
		printf("Could not read pcmcia/srom over sb\n");
		return;
	}
	for (i = 0; i < nw; i++) {
		if ((i & 7) == 0)
			printf("\n0x%04x:", i);
		printf(" 0x%04x", srom[i]);
	}
	printf("\n");

	htol16_buf(srom, nw * 2);
	if (hndcrc8((uint8 *)srom, nw * 2, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE) {
		printf("Bad crc\n");
		return;
	}

	/* Parse the cis */
	cis = (uint8 *)&srom[4];
	if (srom_parsecis(sih, osh, &cis, 1, &vars, &count) != 0) {
		printf("Parse CIS failed\n");
		return;
	}

	printf("Vars:\n");
	v = vars;
	while (*v) {
		printf("  %s\n", v);
		v += strlen(v) + 1;
	}
	printf("\nThat's all folks\n");
}
