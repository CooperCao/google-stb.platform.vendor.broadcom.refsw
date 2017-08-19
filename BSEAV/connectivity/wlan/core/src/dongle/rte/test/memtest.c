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
#include <hndcpu.h>
#include <sbhndcpu.h>
#include <epivers.h>
#include <bcmutils.h>

#include <rte_mem.h>
#include <rte_cons.h>
#include <rte.h>

#include <hnd_boot.h>

#include <sbchipc.h>
#include <hndsoc.h>

#ifndef PHYSADDR
#define PHYSADDR(x)	((uintptr)(x))
#define PHYSADDR_MASK	0xffffffff
#endif

uint32 rotate(uint32 val);
uint memtest_data(uint32 *start, uint32 *end);
uint memtest_word(uint32 *start, uint32 *end, uint32 val, int increment);
uint memtest_byte(uint8 *start, uint8 *end, uint32 val, int increment);
uint memtest_addrs(uint32 *start, uint32 *end);
void _c_main(unsigned long ra);

static si_t *sih;

static bool vsim, qt;

static int verbose = 2;
static bool loop = FALSE;

#define	PRINT(args)	do {if (verbose > 0) printf args;} while (0)
#define	VPRINT(args)	do {if (verbose > 1) printf args;} while (0)

uint32
rotate(uint32 val)
{
	uint32 bit;

	bit = ((val & 0x80000000) == 0) ? 0 : 1;

	return (val << 1) | bit;
}

uint
memtest_data(uint32 *start, uint32 *end)
{
	uint32 val, val0 = 1, v;
	uint32 *mem;
	uint err = 0, i;

	for (i = 0; i < 32; i++) {
		mem = start;
		val = val0;

		while (mem < end) {
			*mem++ = val;
			val = rotate(val);
		}

		mem = start;
		val = val0;

		while (mem < end) {
			v = *mem;
			if (v != val) {
				err++;
			}
			mem++;
			val = rotate(val);
		}
		val0 <<= 1;
	}

	return err;
}

uint
memtest_word(uint32 *start, uint32 *end, uint32 val, int increment)
{
	uint32 orig_val, v;
	uint32 *mem;
	uint err = 0;

	orig_val = val;
	mem = start;

	while (mem < end) {
		*mem++ = val;
		val += increment;
	}

	mem = start;
	val = orig_val;

	while (mem < end) {
		v = *mem;
		if (v != val) {
			err++;
		}
		mem++;
		val += increment;
	}

	return err;
}

uint
memtest_byte(uint8 *start, uint8 *end, uint32 val, int increment)
{
	uint32 orig_val, v;
	uint8 *mem, b0, b1, b2, b3;
	uint err = 0;

	orig_val = val;
	mem = start;

	while (mem < end) {
		*mem++ = val & 0xff;
		*mem++ = (val >> 8) & 0xff;
		*mem++ = (val >> 16) & 0xff;
		*mem++ = (val >> 24) & 0xff;
		val += increment;
	}

	mem = start;
	val = orig_val;

	while (mem < end) {
		b0 = *mem++;
		b1 = *mem++;
		b2 = *mem++;
		b3 = *mem++;

		v = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
		if (v != val) {
			err++;
		}
		val += increment;
	}

	return err;
}

uint
memtest_addrs(uint32 *start, uint32 *end)
{
	uint32 lsb, msb, abit, obit;
	uint8 *mem, *omem, v;
	uint err = 0;

	lsb = 1;
	while (lsb < PHYSADDR(start)) lsb <<= 1;
	msb = lsb;
	while (msb < PHYSADDR(end)) msb <<= 1;
	msb >>= 1;

	start = (uint32 *)(((uint32)start & ~PHYSADDR_MASK) | lsb);
	end = (uint32 *)(((uint32)end & ~PHYSADDR_MASK) | msb);
	if (start >= end) {
		return 0;
	}

	mem = (uint8 *)start;
	abit = lsb;

	while (mem <= (uint8 *)end) {
		/* Clear other addrs */
		obit = 1;
		omem = (uint8 *)((uint32)start | obit);
		while (omem <= (uint8 *)end) {
			if (mem != omem) {
				*omem = 0;
			}
			obit <<= 1;
			omem = (uint8 *)((uint32)start | obit);
		}

		/* Write the target address */
		*mem = 0xff;

		/* Verify */
		v = *mem;
		if (v != 0xff) {
			err++;
		}

		obit = 1;
		omem = (uint8 *)((uint32)start | obit);
		while (omem <= (uint8 *)end) {
			if (mem != omem) {

				v = *omem;
				if (v != 0) {
					err++;
				}
			}
			obit <<= 1;
			omem = (uint8 *)((uint32)start | obit);
		}

		mem = (uint8 *)((uint32)start | abit);
		abit <<= 1;
	}

	return err;
}

static uint
word_tests(uint32 *mem_base, uint32 *mem_lim)
{
	uint err = 0, errcnt;

	errcnt = memtest_word(mem_base, mem_lim, 0, 0);
	err += errcnt;
	if (errcnt == 0)
		PRINT(("    -Writing zeros : PASS\n"));
	else
		PRINT(("    -Writing zeros : FAIL, errcnt %d\n", errcnt));
	BCMDBG_TRACE(0x434d0005);

	errcnt = memtest_word(mem_base, mem_lim, 0xffffffff, 0);
	err += errcnt;
	if (errcnt == 0)
		PRINT(("    -Writing ones : PASS\n"));
	else
		PRINT(("    -Writing ones : FAIL, errcnt %d\n", errcnt));
	BCMDBG_TRACE(0x434d0006);

	errcnt = memtest_word(mem_base, mem_lim, 0x55555555, 0);
	err += errcnt;
	if (errcnt == 0)
		PRINT(("    -Writing odd bits : PASS\n"));
	else
		PRINT(("    -Writing odd bits : FAIL, errcnt %d\n", errcnt));
	BCMDBG_TRACE(0x434d0007);

	errcnt = memtest_word(mem_base, mem_lim, 0xaaaaaaaa, 0);
	err += errcnt;
	if (errcnt == 0)
		PRINT(("    -Writing even bits : PASS\n"));
	else
		PRINT(("    -Writing even bits : FAIL, errcnt %d\n", errcnt));
	BCMDBG_TRACE(0x434d0008);

	return err;
}

static void
run_all(void *arg, int argc, char *argv[])
{
	int i, err = 0, err1 = 0, err_cnt;
	uint32 *mem_base, *mem_lim, val;

#if defined(MEMBASE) && defined(MEMSIZE)
	mem_base = (uint32 *)(((uint)_end + 0x1000) & ~4095);
	mem_lim = (uint32 *)((MEMBASE + MEMSIZE) & ~4095);
#else
	void *currall;

	mem_base = (uint32 *)OSL_UNCACHED(((uint)bss_end + 4095) & ~4095);
	currall = hnd_malloc(4);
	mem_lim = (uint32 *)OSL_UNCACHED(((uint)currall - 100) & ~4095);
	hnd_free(currall);
#endif

	BCMDBG_TRACE(0x434d0003);

	PRINT(("***Memory Test Start\n"));
	PRINT(("Testing memory from 0x%08x to 0x%08x (physical 0x%08x to 0x%08x)\n",
	       (uint)mem_base, (uint)mem_lim, (uint)PHYSADDR(mem_base), (uint)PHYSADDR(mem_lim)));

	do {
		/* Do a short data bus test */
		err_cnt = memtest_data(mem_base, mem_base + 1);
		if (err_cnt == 0)
			PRINT(("1. Quick data bus test : PASS\n"));
		else
			PRINT(("1. Quick data bus test : FAIL, errcnt : %d\n", err_cnt));

		BCMDBG_TRACE(0x434d0004);

		/* That's enough in vsim */
		if (vsim)
			return;

		PRINT(("2. Word test : \n"));
		err_cnt = word_tests(mem_base, mem_lim);
		err += err_cnt;

		err_cnt = memtest_addrs(mem_base, mem_lim);
		err += err_cnt;
		if (err_cnt == 0)
			PRINT(("3. Checking for address line shorts : PASS\n"));
		else
			PRINT(("3. Checking for address line shorts : FAIL,"
				"errcnt : %d\n", err_cnt));

		BCMDBG_TRACE(0x434d0009);

		err_cnt = memtest_byte((uint8 *)mem_base, (uint8 *)mem_lim, 0x55aa55aa, 0);
		err += err_cnt;
		if (err_cnt == 0)
			PRINT(("4. Writing even/odd bits a byte at a time 0x55aa55aa : PASS\n"));
		else
			PRINT(("4. Writing even/odd bits a byte at a time 0x55aa55aa : FAIL,"
				"errcnt : %d\n", err_cnt));

		BCMDBG_TRACE(0x434d000a);

		err_cnt = memtest_data(mem_base, mem_lim);
		err += err_cnt;
		if (err_cnt == 0)
			PRINT(("5. Writing walking bit : PASS\n"));
		else
			PRINT(("5. Writing walking bit : FAIL, %d\n", err_cnt));

		BCMDBG_TRACE(0x434d000b);

		PRINT(("6. Increment/Decrement test by word/byte :\n"));

		val = 1;
		for (i = 0; i < 32; i++) {
			err_cnt = memtest_word(mem_base, mem_lim, val, 1);
			err1 += err_cnt;
			if (err_cnt != 0)
				PRINT(("   -Increment test by word, start 0x%08x : FAIL,"
					"errcnt : %d\n", val, err_cnt));

			err_cnt = memtest_byte((uint8 *)mem_base, (uint8 *)mem_lim, val, 1);
			err1 += err_cnt;
			if (err_cnt != 0)
				PRINT(("   -Increment test by byte, start 0x%08x : FAIL,"
					"errcnt : %d\n", val, err_cnt));

			err_cnt = memtest_word(mem_base, mem_lim, val, -1);
			err1 += err_cnt;
			if (err_cnt != 0)
				PRINT(("   -Decrement test by word, start 0x%08x : FAIL,"
					"errcnt : %d\n", val, err_cnt));

			err_cnt = memtest_byte((uint8 *)mem_base, (uint8 *)mem_lim, val, -1);
			err1 += err_cnt;
			if (err_cnt != 0)
				PRINT(("   -Decrement test by byte, start 0x%08x : FAIL,"
					"errcnt : %d\n", val, err_cnt));

			val <<= 1;
		}

		BCMDBG_TRACE(0x434d000c);

		if (err1 == 0)
			PRINT(("		: PASS\n"));
		else
			PRINT(("		: FAIL, errcnt : %d\n", err_cnt));

		if (loop && keypressed())
			break;
	} while (loop);

	BCMDBG_TRACE(0x434d0020);
	PRINT(("***Memory Test Done : total errors is %d\n", err));
}

static void
run_words(void *arg, int argc, char *argv[])
{
	uint32 *mem_base, *mem_lim;
	void *currall;
	int err = 0;

	mem_base = (uint32 *)OSL_UNCACHED(((uint)bss_end + 4095) & ~4095);
	currall = hnd_malloc(4);
	mem_lim = (uint32 *)OSL_UNCACHED(((uint)currall - 100) & ~4095);
	hnd_free(currall);

	BCMDBG_TRACE(0x434d0003);

	PRINT(("Starting Word tests\n"));
	PRINT(("Testing memory from 0x%08x to 0x%08x (physical 0x%08x to 0x%08x)\n",
	       (uint)mem_base, (uint)mem_lim, (uint)PHYSADDR(mem_base), (uint)PHYSADDR(mem_lim)));

	do {
		err += word_tests(mem_base, mem_lim);
		if (loop && keypressed())
			break;
	} while (loop);

	BCMDBG_TRACE(0x434d0020);
	PRINT(("Word tests done, %d total errors\n", err));
}

static void
run_addr(void *arg, int argc, char *argv[])
{
	uint32 *mem_base, *mem_lim;
	void *currall;
	int err = 0;

	mem_base = (uint32 *)OSL_UNCACHED(((uint)bss_end + 4095) & ~4095);
	currall = hnd_malloc(4);
	mem_lim = (uint32 *)OSL_UNCACHED(((uint)currall - 100) & ~4095);
	hnd_free(currall);

	BCMDBG_TRACE(0x434d0003);

	PRINT(("Starting Addr test\n"));
	PRINT(("Testing memory from 0x%08x to 0x%08x (physical 0x%08x to 0x%08x)\n",
	       (uint)mem_base, (uint)mem_lim, (uint)PHYSADDR(mem_base), (uint)PHYSADDR(mem_lim)));

	do {
		PRINT(("Checking for address line shorts\n"));
		err += memtest_addrs(mem_base, mem_lim);
		BCMDBG_TRACE(0x434d0009);

		if (loop && keypressed())
			break;
	} while (loop);

	BCMDBG_TRACE(0x434d0020);
	PRINT(("Addr test done, %d total errors\n", err));
}

static void
do_loop(void *arg, int argc, char *argv[])
{
	if (loop)
		loop = FALSE;
	else
		loop = TRUE;

	printf("Loop now: %s\n", loop ? "TRUE" : "FALSE");
}

static void
do_verbose(void *arg, int argc, char *argv[])
{
	int step;

	if (arg == 0)
		step = 1;
	else
		step = -1;

	verbose += step;
	if (verbose > 2) verbose = 2;
	if (verbose < 0) verbose = 0;

	printf("verbose now: %d\n", verbose);
}

void
_c_main(unsigned long ra)
{
	int i;
	char chn[8];

	BCMDBG_TRACE(0xaaaa5550);

	/* Basic initialization */
	sih = hnd_init();

	BCMDBG_TRACE(0xaaaa5551);

	vsim = sih->chippkg == HDLSIM_PKG_ID;
	qt = sih->chippkg == HWSIM_PKG_ID;

#ifndef	RTE_UNCACHED
	/* Initialize and turn caches on */
	caches_on();
#endif

	BCMDBG_TRACE(0xaaaa5552);

	printf("%s ver %s compiled at %s on %s\n",
	       __FILE__, EPI_VERSION_STR, __TIME__, __DATE__);
	printf("Running on a %s%s Rev. %d @ %d/%d/%d/%d Mhz.\n",
	       bcm_chipname(sih->chip, chn, 8), vsim ? "(vsim)" : (qt ? "(qt)" : ""),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       si_clock(sih) / 1000000, si_alp_clock(sih) / 1000000);

	BCMDBG_TRACE(0xaaaa5553);

	/* Flush input */
	while (keypressed()) i = getc();

	BCMDBG_TRACE(0xaaaa5554);

	printf("Hit a key to enter interactive mode...");
	if (vsim || qt)
		i = 0;
	else
		i = 300;	/* 300 x 10ms => 3 seconds */
	while (i) {
		if (keypressed())
			break;
		OSL_DELAY(10000);
		i--;
	}

	BCMDBG_TRACE(0xaaaa5555);

	printf("\n");

	if (i == 0) {
		run_all(NULL, 0, NULL);
		BCMDBG_TRACE(0xaaaa5556);
		return;
	}

	BCMDBG_TRACE(0xaaaa5557);

	if (!hnd_cons_add_cmd("all", run_all, 0) ||
		!hnd_cons_add_cmd("words", run_words, 0) ||
		!hnd_cons_add_cmd("addr", run_addr, 0) ||
		!hnd_cons_add_cmd("loop", do_loop, 0) ||
		!hnd_cons_add_cmd("ve", do_verbose, 0) ||
		!hnd_cons_add_cmd("qu", do_verbose, (void *)1)) {
	   return;
	}
}
