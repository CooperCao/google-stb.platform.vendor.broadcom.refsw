/*
 * Initialization and support routines for self-booting
 * compressed image.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id$
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <hndsoc.h>
#include <siutils.h>
#include <wlioctl.h>
#include <hndcpu.h>
#include <bcmdevs.h>
#include <epivers.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <sbconfig.h>
#include <sbchipc.h>
#include <rte_dev.h>
#include <rte.h>

void c_main(unsigned long ra);

static si_t *sih;

/* USB device */
extern hnd_dev_t bcmjtagd;


void
c_main(unsigned long ra)
{
	chipcregs_t *cc;
	char chn[8];

	/* Basic initialization */
	sih = hnd_init();

	/* Initialize and turn caches on */
	caches_on();

	/* clear the watchdog counter which may have been
	 * set by the bootrom download code
	 */
	si_watchdog(sih, 0);
	if (((cc = si_setcore(sih, CC_CORE_ID, 0)) != NULL) &&
	    (R_REG(si_osh(sih), &cc->intstatus) & CI_WDRESET)) {
		printf("Watchdog reset bit set, clearing\n");
		W_REG(si_osh(sih), &cc->intstatus, CI_WDRESET);
	}

#ifdef mips
	if (MFC0(C0_STATUS, 0) & ST0_NMI)
		printf("NMI bit set, ErrorPC=0x%x\n", MFC0(C0_ERREPC, 0));
#else
	/* Check flags in the arm's resetlog */
#endif

	printf("\nRTE (JTAGD%s) %s running on a %s Rev. %d @ %d/%d/%d/%d Mhz.\n",
#ifdef BCMUSB_NODISCONNECT
               " NODISCONNECT",
#else
               "",
#endif
	       EPI_VERSION_STR, bcm_chipname(sih->chip, chn, 8),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       si_clock(sih) / 1000000, si_alp_clock(sih) / 1000000);

	/* Add the USB device */
	if (hnd_add_device(sih, &bcmjtagd, USB20D_CORE_ID, BCM47XX_USB20D_ID) != 0) {
		printf("JTAGD device add failed!!\n");
		hnd_die();
	}
	if (bcmjtagd.ops->open(&bcmjtagd) != 0) {
		printf("JTAGD device open failed!!\n");
		hnd_die();
	}

	hnd_idle(sih);
}
