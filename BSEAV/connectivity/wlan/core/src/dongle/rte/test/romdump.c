/*
 * Initialization and support routines for dumping ROM
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$:
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <osl.h>
#include <siutils.h>
#include <hndcpu.h>
#include <epivers.h>

void c_main(unsigned long ra);

static si_t *sih;

static void hndrte_romdump(void *ptr, int argc, char **argv);
static void hndrte_memdmp(void *arg, int argc, char **argv);


static uint32 romstart = 0;
static uint32 romend = 0;

static volatile uint hangaround = 1;

void
c_main(unsigned long ra)
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

#ifndef RTE_POLL
	hnd_enable_interrupts();
#endif

	BCMDBG_TRACE(0x434d0003);

	if (sih->chip == BCM4336_CHIP_ID) {
		romstart = 0x800000;
		romend = 0x800000 + (448 * 1024);
	}

	printf("romstart is 0x%04x, romend is 0x%04x\n", romstart, romend);

	if (!hnd_cons_add_cmd("romdump", hndrte_romdump, hnd_time()) ||
		!hnd_cons_add_cmd("d", hndrte_memdmp, hnd_time()))
	   return;

#ifdef RTE_POLL
	while (hangaround)
		hnd_poll(sih);
#endif
}

static void
hndrte_memdmp(void *arg, int argc, char **argv)
{
	uint32 addr, len, nw, i = 0;
	ulong *dw;
	ushort *w;
	uchar *b;

	if (argc != 3) {
		printf("%s: start len\n", argv[0]);
		return;
	}
	addr = bcm_strtoul(argv[1], NULL, 0);
	bytes = bcm_strtoul(argv[2], NULL, 0);
	if (bytes == 1) {
		b = (uchar *)addr;
		printf("add 0x%08x, val: 0x%02x \n", b, *b);
	}
	else if (bytes == 2) {
		w = (ushort *)addr;
		printf("add 0x%08x, val: 0x%04x \n", w, *w);
	}
	else {
		dw = (ulong *)addr;
		printf("add 0x%08x, val: 0x%08x \n", dw, *dw);
	}

}

static void
hndrte_romdump(void *arg, int argc, char **argv)
{
	uint32 start, len, nw, i = 0;
	ulong *ptr, *end;

	if (argc != 3) {
		printf("%s: start len\n", argv[0]);
		return;
	}
	start = bcm_strtoul(argv[1], NULL, 0);
	len = bcm_strtoul(argv[2], NULL, 0);

	if ((start < romstart) || (start > romend) || (start + len > romend)) {
		printf("%s: Outof Range: startaddr 0x%04x len 0x%04x, Range: [0x%04x - 0x%04x]\n",
			__FUNCTION__, start, len, romstart, romend);
		return;
	}
	nw = len >> 2;
	ptr = (ulong *)start;
	end = (ulong *)(start + len);

	/*
	 * reason for this specific format is it is easy to compare with original rom
	 * "od -jx4 -tx4 roml.bin" generates a similar format
	*/
	while (i < nw) {
		printf("\n%06x ", (i* 4) + 0x60000);
		printf("%08x ", ptr[0]);
		if (&ptr[1] < end)
			printf("%08x ", ptr[1]);
		if (&ptr[2] < end)
			printf("%08x ", ptr[2]);
		if (&ptr[3] < end)
			printf("%08x ", ptr[3]);
		ptr += 4;
		i += 4;
	}
	printf("\n");
}
