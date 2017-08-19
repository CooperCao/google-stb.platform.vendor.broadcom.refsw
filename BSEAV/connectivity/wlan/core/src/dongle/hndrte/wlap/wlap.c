/*
 * An hndrte wireless AP.
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
#include <siutils.h>
#include <wlioctl.h>
#include <hndcpu.h>
#include <hndsoc.h>
#include <bcmdevs.h>
#include <epivers.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <rte_dev.h>
#include <rte_cons.h>
#include <rte.h>


void status(void *unused);
void c_main(void);

/* WL device struct */
extern hnd_dev_t bcmwl;

ctimeout_t status_timer;

void
status(void *unused)
{
	uint8 bssid[6];
	int ret;
	static int timeout = 0;

	/* report status */
	ret = bcmwl.ops->ioctl(&bcmwl, WLC_GET_BSSID, &bssid, sizeof(bssid), NULL, NULL, FALSE);

	if (ret && (timeout++ < 10)) {
		printf(".");
		/* Add a timer to check status again in 1 seconds */
		hndrte_timeout(&status_timer, (uint)(1*1000), status, NULL);
	}
	else {
		if (ret)
			printf("ERROR %d; failed to associate\n", ret);
		else
			printf("Associated to 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",
				bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

#ifdef BCMDBG_MEM
		/* display mem usage */
		process_ccmd("mu", 2);
		process_ccmd("ar", 2);
#endif  /* BCMDBG_MEM */
	}
}


void
c_main(void)
{
	wlc_ssid_t ssid;
	int	val = 1;
	const char	*ssid_str;
	uint16 id;
	char chn[8];
	si_t *sih;

	/* Basic initialization */
	sih = hnd_init();

	/* Initialize and turn caches on */
	caches_on();

	printf("\n\nRTE (wlap) v%s running on a %s Rev. %d @ %d/%d\n",
		EPI_VERSION_STR, bcm_chipname(sih->chip, chn, 8),
		sih->chiprev, si_clock(sih), si_cpu_clock(sih));

	/* Add the WL device */
	if ((id = si_d11_devid(sih)) == 0xffff)
		id = BCM4306_D11G_ID;
	hnd_add_device(sih, &bcmwl, D11_CORE_ID, id);

	if ((ssid_str = nvram_get("wl0_ssid"))) {
		ssid.SSID_len = strlen(ssid_str);
		bcopy(ssid_str, ssid.SSID, ssid.SSID_len);

		bcmwl.ops->ioctl(&bcmwl, WLC_SET_INFRA, &val, sizeof(int),
		                   NULL, NULL, TRUE);
		bcmwl.ops->ioctl(&bcmwl, WLC_SET_AP, &val, sizeof(int),
		                   NULL, NULL, TRUE);
#ifdef BCMDBG
		val = WL_ERROR_VAL | WL_INFORM_VAL | WL_ASSOC_VAL;
		bcmwl.ops->ioctl(&bcmwl, WLC_SET_MSGLEVEL, &val, sizeof(int),
		                   NULL, NULL, TRUE);
#endif
		/* ensure interface is up */
		bcmwl.ops->ioctl(&bcmwl, WLC_UP, NULL, 0, NULL, NULL, TRUE);

		/* join/create a BSS */
		bcmwl.ops->ioctl(&bcmwl, WLC_SET_SSID, &ssid, sizeof(wlc_ssid_t),
		                   NULL, NULL, TRUE);

		/* Add a timer to check status in 10 seconds */
		hndrte_timeout(&status_timer, (uint)(2*1000), status, NULL);
	} else
		printf("Test failed: could not find nvram var 'wl0_ssid'\n");

	/* Run the main loop */
	hnd_idle(sih);
}

/* At the end so the whole data segment gets copied */
int input_data = 0xdead;
