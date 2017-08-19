/*
 * dump OTP content
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
#include <bcmotp.h>
#include <bcmendian.h>
#include <bcmsrom.h>
#include <epivers.h>
#include <hndpmu.h>
#include <sbchipc.h>
#include <bcmutils.h>

#include <rte.h>

void _c_main(unsigned long ra);

void
_c_main(unsigned long ra)
{
	bool vsim, qt;
	int rc, i;
	uint nw, nb, rsz;
	void *oh;
	char data[2048], *p;
	si_t *sih;
	osl_t *osh;
	char chn[8];
	uint32 min_rsrc_mask = 0;

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

	if (sih->cccaps & CC_CAP_PMU)
		si_pmu_otp_power(sih, osh, TRUE, &min_rsrc_mask);

	oh = otp_init(sih);
	if (oh == NULL) {
		printf("otp_init failed.\n");
		return;
	}

	printf("otp_init succedded.\n");

	printf("otp_status: 0x%x\n", otp_status(oh));

	rsz = nw = otp_size(oh) / 2;

	nb = otp_dump(oh, 1, data, sizeof(data));
	for (p = data, i = 0; i < nb;
	     p += PRINTF_BUFLEN, i += PRINTF_BUFLEN) {
		char c = p[PRINTF_BUFLEN];
		p[PRINTF_BUFLEN] = 0;
		printf("%s", p);
		p[PRINTF_BUFLEN] = c;
	}

	rc = otp_read_region(oh, OTP_SW_RGN, (uint16 *)data, &rsz);
	if (rc == 0) {
		char *v, *vars;
		uint count;
		uint8 *cis;

		htol16_buf((uint8 *)data, nw * 2);

		/* Parse the cis */
		cis = (uint8 *)data;
		if (srom_parsecis(sih, osh, &cis, 1, &vars, &count) == 0) {
			printf("Vars:\n");
			v = vars;
			while (*v) {
				printf("  %s\n", v);
				v += strlen(v) + 1;
			}
		}
	}
	printf("That's all folks\n");
}
