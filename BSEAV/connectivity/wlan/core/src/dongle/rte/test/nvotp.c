/*
 * Test the writing of nvram into otp.
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
#include <bcmutils.h>

#include <rte.h>

void _c_main(unsigned long ra);

static si_t *sih;

static char *vars[] = {
	"boardtype=0x45a",	"boardnum=666",
	"boardrev=0x22",	"boardflags=0x648",
	"sromrev=2",		"wl0id=0x4318",
	"il0macaddr=00:90:4c:82:02:9a",
	"aa0=3",		"ag0=5",
	"pa0maxpwr=80",		"pa0itssit=62",
	"pa0b0=0x15e3",		"pa0b1=0xfa7f",
	"pa0b2=0xfea8",		"opo=8",
	"cctl=0",		"ccode=0",
	NULL
};

void
_c_main(unsigned long ra)
{
	bool vsim, qt;
	int st, i, len, s_len;
	void *oh;
	char *v, *d, **s;
	char chn[8];

	/* Basic initialization */
	sih = hnd_init();

	vsim = sih->chippkg == HDLSIM_PKG_ID;
	qt = sih->chippkg == HWSIM_PKG_ID;

	/* Initialize and turn caches on */
	caches_on();

	printf("%s ver %s compiled at %s on %s\n",
	       __FILE__, EPI_VERSION_STR, __TIME__, __DATE__);
#ifdef	OTP_FORCEFAIL
	printf("  compiled with OTP_FORCEFAIL = %d\n", OTP_FORCEFAIL);
#endif
	printf("Running on a %s%s Rev. %d @ %d/%d/%d/%d Mhz.\n",
	       bcm_chipname(sih->chip, chn, 8), vsim ? "(vsim)" : (qt ? "(qt)" : ""),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       si_clock(sih) / 1000000, si_alp_clock(sih) / 1000000);

	d = v = hnd_malloc(NVRAM_SPACE);
	if (v == NULL) {
		printf("nvram buffer allocation failed, aborting test.\n");
		goto fail;
	}

	/* copy all the nvram vars from the vars[] array to the new memory block */
	len = 0;
	for (s = vars; *s != NULL; s++) {

		/* get the length of next string & null termination */
		s_len = strlen(*s) + 1;

		/* if the next string & null termination do not fit in dest buffer, bail out */
		if (len + s_len > NVRAM_SPACE) {
			printf("nvram buffer overflowed with var string \"%s\".\n", *s);
			goto fail;
		}

		memcpy(d, *s, s_len);
		d += s_len;
		len += s_len;
	}

	oh = otp_init(sih);
	if (oh == NULL)
		printf("otp_init failed.\n");
	else {
		printf("otp_init succedded.\n");

		st = otp_status(oh);
		printf("otp_status: 0x%x\n", st);

		i = otp_size(oh);
		printf("otp size: %d(0x%x)\n", i, i);

		if (st & 1) {
			/* Not the first time, just add a value */
			*v = '\0';

			/* len of string & null termination */
			len = snprintf(v, NVRAM_SPACE, "newvalue=0x%x", osl_getcycles()) + 1;

			/* check for truncation on snprintf call */
			if (len > NVRAM_SPACE) {
				printf("nvram buffer written with truncated string \"%s\".\n", v);
				/* snprintf null terminates to the limit it was given,
				 * adjust len to indicate actual buffer size.
				 */
				len = NVRAM_SPACE;
			}

			printf("Adding ");
		} else {
			printf("Writing ");
		}
		printf("%d bytes to otp\n", len);
		if (len & 1) {
			v[len++] = '\0';
		}
		i = otp_nvwrite(oh, (uint16 *)v, len / 2);
		printf("otp_nvwrite returned: %d\n", i);
	}

fail:
	if (v != NULL) {
		hnd_free(v);
	}

	printf("That's all folks\n");
}
