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
#include <osl.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <hndchipc.h>
#include <hndcpu.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <siutils.h>
#include <epivers.h>
#include <bcmutils.h>
#include <rte_cons.h>
#include <rte.h>


#define	PAUSE		OSL_DELAY(500000);

#ifdef BCMDBG_MEM
#define malloc_align(n, a) hnd_malloc_align(n, a, __FILE__, __LINE__)
#else
#define malloc_align(n, a) hnd_malloc_align(n, a)
#endif /* BCMDBG_MEM */

void c_main(unsigned long ra);

static si_t *sih;
static uint32 clock;

#define	NUB	512

static char notused[NUB];	/* To test reclaim */

#define	NUM_ALLOC	16

static ctimeout_t hb_timer, timer, timers[NUM_ALLOC];

static uint buf[NUM_ALLOC * 2];

static uint mark_calls = 0;

static void
mark(void *arg)
{
	uint32 *m = (uint32 *)arg;
	uint32 now = hnd_time();

	if (now == 0)
		now = 1;

	*m = now;
	 mark_calls++;
}

static uint mnr_calls = 0;

static void
marknrearm(void *arg)
{
	uint32 i, d, *m = (uint32 *)arg;
	uint32 now = hnd_time();

	i = mnr_calls++;
	m[(2 * i) + 1] = now;

	if (++i < NUM_ALLOC) {
		d = 8 * i;
		m[2 * i] = d;
		m[(2 * i) + 1] = 0;
		if (!hndrte_timeout(&timer, d, marknrearm, (void *)m)) {
			printf("%s: Failed to add %d timeout\n", __FUNCTION__, i);
			mnr_calls = NUM_ALLOC;
		}
	}
}

static void
hello(void *arg, int argc, char *argv[])
{
	uint i = 0;
	printf("You say \"");
	for (; i < argc; i++)
		printf("%s", argv[i]);
	printf("\"\n");
	printf("I say: Hello!!!\n");
	printf("(born at %d ms)\n", (uint32)(uintptr)arg);
}

static volatile uint hangaround = 1;

static void
bye(void *arg, int argc, char *argv[])
{
	uint i = 0;
	printf("You say \"");
	for (; i < argc; i++)
		printf("%s", argv[i]);
	printf("\"\n");
	printf("I say: Good Bye!!!\n");
	hangaround = 0;
}

static void
heartbeat(void *arg)
{
	uint32 now = hnd_time();
	uint32 sec = now / 1000;
	uint32 mil = now - (sec * 1000);

	printf("%d.%d\n", sec, mil);

	if (hangaround)
		if (!hndrte_timeout(&hb_timer, 5000, heartbeat, NULL))
			printf("%s: Failed to add heartbeat timeout\n", __FUNCTION__);
}


void
c_main(unsigned long ra)
{
	bool vsim, qt;
	int i, n;
	int a, pass;
	uint32 start;
	void *mem[NUM_ALLOC];
	volatile uint32 *p;
	chipcregs_t *cc;
#ifndef	RTE_UNCACHED
	volatile uint32 *r;
#endif
	char chn[8];

	/* Basic initialization */
	sih = hnd_init();

	vsim = sih->chippkg == HDLSIM_PKG_ID;
	qt = sih->chippkg == HWSIM_PKG_ID;

	/* clear the watchdog counter which may have been
	 * set by the bootrom download code
	 */

	si_watchdog(sih, 0);

	if (((cc = si_setcoreidx(sih, SI_CC_IDX)) != NULL) &&
	    (R_REG(si_osh(sih), &cc->intstatus) & CI_WDRESET)) {
		printf("Watchdog reset bit set, clearing\n");
		W_REG(si_osh(sih), &cc->intstatus, CI_WDRESET);
	}

#ifndef	RTE_UNCACHED
	/* Initialize and turn caches on */
	caches_on();
#endif

	/* Initialize all timers */
	hndrte_init_timeout((ctimeout_t*)&timer);
	hndrte_init_timeout((ctimeout_t*)&hb_timer);
	for (i = 0; i < NUM_ALLOC; i++)
		hndrte_init_timeout((ctimeout_t*)&timers[i]);

	clock = si_clock(sih);
	printf("%s ver %s compiled at %s on %s\n",
	       __FILE__, EPI_VERSION_STR, __TIME__, __DATE__);
	printf("Running on a %s%s Rev. %d @ %d/%d/%d/%d Mhz.\n",
	       bcm_chipname(sih->chip, chn, 8), vsim ? "(vsim)" : (qt ? "(qt)" : ""),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       clock / 1000000, si_alp_clock(sih) / 1000000);

	printf("\nhndrtetest: Alloc test\n");
	process_ccmd("mu", 2);
	for (i = 0; i < NUM_ALLOC; i++) {
		n =  8 + (16 * i);
		mem[i] = hnd_malloc(n);
		printf("mem[%d] = %d @ 0x%x\n", i, n, (uint32)mem[i]);
	}
#ifdef	BCMDBG_MEM
	hnd_print_malloc();
#endif

#ifndef	RTE_UNCACHED
	printf("\nhndrtetest: Simple cache test\n");
	p = (volatile uint32 *)mem[NUM_ALLOC - 1];
	r = (volatile uint32 *)OSL_UNCACHED(p);
	flush_dcache((uint32)p, n);
	*r = 0xaa55aa55;
	printf("Cached: 0x%x, Uncached: 0x%x\n", *p, *r);
	*p = 0xdeadbeef;
	printf("Cached: 0x%x, Uncached: 0x%x\n", *p, *r);
	flush_dcache((uint32)p, n);
	printf("Cached: 0x%x, Uncached: 0x%x\n", *p, *r);
#endif

#ifndef RTE_POLL
	hnd_enable_interrupts();
#endif

	printf("\nhndrtetest: Simple timer test\n");
	start = hnd_time();
	for (i = 0; i < NUM_ALLOC; i++) {
		volatile uint32 *m = (volatile uint32 *)mem[i];
		uint32 d = 7 * i;

		*m++ = d;
		*m = 0;
		if (!hndrte_timeout(&timer, d, mark, DISCARD_QUAL(m, uint32))) {
			printf("%s: Failed to add simple %d timeout\n", __FUNCTION__, i);
			goto sttend;
		}
		while (*m == 0)
			hnd_poll(sih);
	}
	printf("\tstart:\t%u\n", start);
	for (i = 0; i < NUM_ALLOC; i++) {
		uint32 *mw = (uint32 *)mem[i];
		uint32 *nw = mw + 1;

		printf("\t+ %d =>\t%u\n", *mw, *nw);
	}
sttend:

	printf("\nhndrtetest: Simple rearming timer test\n");
	mnr_calls = 0;
	start = hnd_time();
	buf[0] = 0;
	if (hndrte_timeout(&timer, 0, marknrearm, (void *)buf)) {
		p = (volatile uint32 *)(&mnr_calls);
		while (*p != NUM_ALLOC) {
			hnd_poll(sih);
			printf(" %d", *p);
		}
		printf("\tstart:\t%u\n", start);
		for (i = 0; i < NUM_ALLOC; i++) {
			printf("\t+ %d =>\t%u\n", buf[2 * i], buf[(2 * i) + 1]);
		}
	} else
		printf("%s: Failed to add first rearming timeout\n", __FUNCTION__);

	printf("\nhndrtetest: Concurrent timer test\n");
	start = hnd_time();
	mark_calls = 0;
	for (i = 0; i < NUM_ALLOC; i++) {
		uint32 *m = (uint32 *)mem[i];
		uint32 d = 8 * i;

		*m++ = d;
		*m = 0;
		if (!hndrte_timeout(&timers[i], d, mark, (void *)m)) {
			printf("%s: Failed to add concurrent %d timeout\n", __FUNCTION__, i);
			goto cttend;
		}
	}
	p = (volatile uint32 *)(&mark_calls);
	while (*p != NUM_ALLOC) {
		hnd_poll(sih);
		printf(" %d", *p);
	}
	printf("\tstart:\t%u\n", start);
	for (i = 0; i < NUM_ALLOC; i++) {
		uint32 *mw = (uint32 *)mem[i];
		uint32 *nw = mw + 1;

		printf("\t+ %d =>\t%u\n", *mw, *nw);
	}
cttend:

	printf("\nhndrtetest: Simultaneous timer test\n");
	start = hnd_time();
	mark_calls = 0;
	for (i = 0; i < NUM_ALLOC; i++) {
		uint32 *m = (uint32 *)mem[i];
		uint32 d = 8 * (i >> 2);

		*m++ = d;
		*m = 0;
		if (!hndrte_timeout(&timers[i], d, mark, (void *)m)) {
			printf("%s: Failed to add concurrent %d timeout\n", __FUNCTION__, i);
			goto simultttend;
		}
	}
	p = (volatile uint32 *)(&mark_calls);
	while (*p != NUM_ALLOC) {
		hnd_poll(sih);
		printf(" %d", *p);
	}
	printf("\tstart:\t%u\n", start);
	for (i = 0; i < NUM_ALLOC; i++) {
		uint32 *mw = (uint32 *)mem[i];
		uint32 *nw = mw + 1;

		printf("\t+ %d =>\t%u\n", *mw, *nw);
	}
simultttend:

	printf("\nhndrtetest: Alloc/free test\n");
	for (pass = 0; pass < 2; pass++) {
		for (i = 0; i < NUM_ALLOC; i++) {
			if (mem[i])
				hnd_free(mem[i]);
			printf("[%d] freed 0x%p\n", i, mem[i]);
#ifdef	BCMDBG_MEM
			hnd_print_malloc();
#endif
			if (pass & 1)
				n = 8 + (12 * i);
			else
				n = 8 + (20 * i);
			mem[i] = hnd_malloc(n);
			printf("[%d] allocated %d @ 0x%p\n", i, n, mem[i]);
#ifdef	BCMDBG_MEM
			hnd_print_malloc();
			PAUSE;
#endif
		}
	}

	printf("\nhndrtetest: Aligned alloc test\n");
	for (i = 0; i < NUM_ALLOC; i++) {
		if (mem[i])
			hnd_free(mem[i]);
	}
#ifdef	BCMDBG_MEM
	hnd_print_malloc();
#endif
	for (a = 0, i = 0; i < NUM_ALLOC; i++, a++) {
		n = 8 + (16 * i);
		mem[i] = malloc_align(n, a);
		printf("mem[%d] = %d/%d @ 0x%x\n", i, n, (1 << a), (uint32)mem[i]);
	}
#ifdef	BCMDBG_MEM
	hnd_print_malloc();
#endif

	for (pass = 0; pass < 2; pass++) {
		if (pass & 1)
			a = NUM_ALLOC;
		else
			a = 0;
		for (i = 0; i < NUM_ALLOC; i++) {
			if (mem[i])
				hnd_free(mem[i]);
			printf("[%d] freed 0x%p\n", i, mem[i]);
#ifdef	BCMDBG_MEM
			hnd_print_malloc();
			PAUSE;
#endif
			if (pass & 1) {
				a--;
				n = 8 + (12 * i);
			} else {
				n = 8 + (20 * i);
				a++;
			}
			mem[i] = malloc_align(n, a);
			printf("[%d] allocated %d/%d @ 0x%p\n", i, n, (1 << a), mem[i]);
#ifdef	BCMDBG_MEM
			hnd_print_malloc();
			PAUSE;
#endif
		}
	}

	printf("\nhndrtetest: Large alloc test\n");
	for (i = 0; i < NUM_ALLOC; i++) {
		if (mem[i])
			hnd_free(mem[i]);
		n = 8 << i;
		if ((mem[i] = hnd_malloc(n)) != NULL) {
			printf("mem[%d] = %d @ 0x%p\n", i, n, mem[i]);
		} else {
			printf("Failed to allocate %d bytes\n", n);
			break;
		}
	}
#ifdef	BCMDBG_MEM
	hnd_print_malloc();
#endif
	process_ccmd("mu", 2);
	a = (int)hnd_arena_add((uint32)notused, NUB);
	printf("Adding %d bytes, %d useable; @ 0x%x to the arena\n", NUB, a,
	       (uint32)notused);
	process_ccmd("mu", 2);
	for (i = -2; i < 2; i++) {
		n = a + (i * 4);
		p = hnd_malloc(n);
		printf("Allocated %d bytes @ 0x%p\n", n, p);
#ifdef	BCMDBG_MEM
		process_ccmd("ar", 2);
#endif
		hnd_free(DISCARD_QUAL(p, uint32));
	}

	if (!hnd_cons_add_cmd("hello", hello, (void *)hnd_time()) ||
		!hnd_cons_add_cmd("bye", bye, (void *)0))
		printf("%s: Failed to add cons command\n", __FUNCTION__);

	if (!hndrte_timeout(&hb_timer, 5000, heartbeat, NULL))
		printf("%s: Failed to add initial heartbeat timeout\n", __FUNCTION__);

#ifdef RTE_POLL
	while (hangaround)
		hnd_poll(sih);
#endif

	printf("\nhndrtetest: Done in %d ms.\n", hnd_time());
	printf("\n");
}
