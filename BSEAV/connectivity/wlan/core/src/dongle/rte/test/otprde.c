/*
 * wirte redundancy entry 0 to OTP
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

void
_c_main(unsigned long ra)
{
	bool vsim, qt;
	void *oh;
	si_t *sih;
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

	si_otp_power(sih, TRUE, &min_rsrc_mask);

	oh = otp_init(sih);
	if (oh == NULL)
		printf("otp_init failed.\n");
	else {
		printf("otp_init succedded.\n");

		printf("otp_status: 0x%x\n", otp_status(oh));

		printf("otp size: %d\n", otp_size(oh));

		printf("otp_write_rde 0 returned: %d\n", otp_write_rde(oh, 0, 512, 1));
	}

	si_otp_power(sih, OFF, &min_rsrc_mask);

	printf("That's all folks\n");
}
