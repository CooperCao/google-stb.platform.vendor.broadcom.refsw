/*
 * wirte CID/PackageOtpion to OTP
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
#include <sbchipc.h>
#include <hndcpu.h>
#include <bcmotp.h>
#include <bcmnvram.h>
#include <epivers.h>
#include <hndpmu.h>
#include <bcmutils.h>

#include <rte.h>

void _c_main(unsigned long ra);

static si_t *sih;

void
_c_main(unsigned long ra)
{
	bool vsim, qt;
	int i;
	void *oh;
	uint16 data[3];
	uint words;
	char chn[8];
	uint32 min_rsrc_mask = 0;

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

	if (sih->cccaps & CC_CAP_PMU)
		si_pmu_otp_power(sih, si_osh(sih), TRUE, &min_rsrc_mask);

	oh = otp_init(sih);
	if (oh == NULL)
		printf("otp_init failed.\n");
	else {
		printf("otp_init succedded.\n");

		i = otp_status(oh);
		printf("otp_status: 0x%x\n", i);

		i = otp_size(oh);
		printf("otp size: %d(0x%x)\n", i, i);

		if (sih->ccrev >= 21) {
			data[0] = sih->chip;
			data[1] = 0xa5;
			words = 2;
		}
		else {
			data[0] = 0x4320;
			data[1] = data[2] = 0;
			words = 3;
		}
		i = otp_write_region(oh, OTP_CI_RGN, data, words, 0);
		printf("otp_write_region OTP_CI_RGN returned: %d\n", i);
	}

	printf("That's all folks\n");
}
