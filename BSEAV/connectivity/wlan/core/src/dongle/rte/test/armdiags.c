/*
 * Standalone CPU diags to be run by the tcl diags scripts.
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
 * $Id: diags.c,v 1.4 2008/08/12 04:53:57 Exp $
 */

#include <typedefs.h>
#include <osl.h>
#include <sbhndcpu.h>
#include <standdiags.h>

#include <hnd_boot.h>

#ifdef	__arm__

static INLINE uint32
arm_cpsr(void)
{
	uint32 res;

	__asm__ __volatile__("mrs	%0, cpsr" :
	                     "=r" (res));
	return res;
}

static INLINE uint32
arm_spsr(void)
{
	uint32 res;

	__asm__ __volatile__("mrs	%0, spsr" :
	                     "=r" (res));
	return res;
}

#if defined(__ARM_ARCH_7M__)
static INLINE uint32
armcm3_apsr(void)
{
	uint32 res;

	__asm__ __volatile__("mrs	%0, apsr" :
	                     "=r" (res));
	return res;
}
#endif	/* ARM7M */
#endif	/* __arm__ */


void _c_main(uint32 ra);

void
_c_main(uint32 ra)
{
	uint32 base;
	statarea_t *st;
	uint32 *w, *ww, cid;
#ifdef	mips
	uint32 ar;
#endif
	uint16 *h, *wh, h0, h1;
	uint8 *b, *wb, b0, b1, b2, b3;


	BCMDBG_TRACE(0x434d4142);

	/* Get our base (for running from SOCRAM not at zero) */
	base = ra & 0xff000000;

	st = (statarea_t *)OSL_UNCACHED(base + stat_off);

	st->wm = 0x12345678;

#if defined(mips)
	st->prid = cid = MFC0(C0_PRID, 0);
	st->st = MFC0(C0_STATUS, 0);
	st->cf = MFC0(C0_CONFIG, 0);
	ar = (st->cf & CONF_AR) >> CONF_AR_SHIFT;
	st->cf1 = MFC0(C0_CONFIG, 1);
	if (st->cf1 & CONF_M)
		st->cf2 = MFC0(C0_CONFIG, 2);
	if (st->cf2 & CONF_M)
		st->cf3 = MFC0(C0_CONFIG, 2);
	if (((cid & PRID_COMP_MASK) == PRID_COMP_MIPS) &&
	    ((cid & PRID_IMP_MASK) == PRID_IMP_74K)) {
		st->cf7 = MFC0(C0_CONFIG, 7);
	}
	if (ar > 0) {
		st->intctl = MFC0(C0_STATUS, 1);
		st->srsctl = MFC0(C0_STATUS, 2);
		st->srsmap = MFC0(C0_STATUS, 3);
		st->ebase = MFC0(C0_PRID, 1);
		st->hwren = MFC0(C0_INFO, 0);
	}
	if (((cid & PRID_COMP_MASK) == PRID_COMP_BROADCOM) &&
	    ((cid & PRID_IMP_MASK) == PRID_IMP_BCM3302)) {
		st->brcm0b = st->brcm0 = MFC0(C0_BROADCOM, 0);
		st->pll1 = MFC0(C0_BROADCOM, 1);
		st->pll2 = MFC0(C0_BROADCOM, 2);
		st->clksync = MFC0(C0_BROADCOM, 3);
		st->pll3 = MFC0(C0_BROADCOM, 4);
		st->rst = MFC0(C0_BROADCOM, 5);
	}
#elif defined(__ARM_ARCH_7M__)

	st->idcode = cid = *(uint32 *)CM3_CPUID;
	st->cpsr = armcm3_apsr();
	st->freetimer = *(uint32 *)0x18000000;
	st->ctrl = *(uint32 *)CM3_SYSCTRL;
	st->auxctrl = *(uint32 *)CM3_CFGCTRL;
	st->TCMstat = *(uint32 *)CM3_MPUCTRL;
	st->MPUtype = *(uint32 *)CM3_MPUTYPE;
	st->procf0 = *(uint32 *)CM3_PFR0;
	st->procf1 = *(uint32 *)CM3_PFR1;
	st->debugf0 = *(uint32 *)CM3_DFR0;
	st->auxf0 = *(uint32 *)CM3_AFR0;
	st->mmf0 = *(uint32 *)CM3_MMFR0;
	st->mmf1 = *(uint32 *)CM3_MMFR1;
	st->mmf2 = *(uint32 *)CM3_MMFR2;
	st->mmf3 = *(uint32 *)CM3_MMFR3;
	st->isf0 = *(uint32 *)CM3_ISAR0;
	st->isf1 = *(uint32 *)CM3_ISAR1;
	st->isf2 = *(uint32 *)CM3_ISAR2;
	st->isf3 = *(uint32 *)CM3_ISAR3;
	st->isf4 = *(uint32 *)CM3_ISAR4;
	st->isf5 = *(uint32 *)CM3_ISAR5;
#elif defined(__ARM_ARCH_7R__)
	st->cpsr = arm_cpsr();
	st->spsr = arm_spsr();

	st->idcode = cid = mrc(15, 0, 0, 0);
	st->cachetype = mrc(15, 0, 0, 1);
	st->TCMstat = mrc(15, 0, 0, 2); /* TCM type */
	st->MPUtype = mrc(15, 0, 0, 4);

	st->procf0 = mrc(15, 0, 1, 0);
	st->procf1 = mrc(15, 0, 1, 1);
	st->debugf0 = mrc(15, 0, 1, 2);
	st->auxf0 = mrc(15, 0, 1, 3);

	st->mmf0 = mrc(15, 0, 1, 4);
	st->mmf1 = mrc(15, 0, 1, 5);
	st->mmf2 = mrc(15, 0, 1, 6);
	st->mmf3 = mrc(15, 0, 1, 7);

	st->isf0 = mrc(15, 0, 2, 0);
	st->isf1 = mrc(15, 0, 2, 1);
	st->isf2 = mrc(15, 0, 2, 2);
	st->isf3 = mrc(15, 0, 2, 3);
	st->isf4 = mrc(15, 0, 2, 4);
	st->isf5 = mrc(15, 0, 2, 5);
#elif defined(__arm__)

	st->idcode = cid = mrc(15, 0, 0, 0);
	st->cpsr = arm_cpsr();
	st->spsr = arm_spsr();
	st->freetimer = *(uint32 *)0x18000000;

	st->ctrl = mrc(15, 1, 0, 0);
	st->auxctrl = mrc(15, 1, 0, 1);
	st->cpacc = mrc(15, 1, 0, 2);
	st->pid = mrc(15, 13, 0, 1);
	st->cachetype = mrc(15, 0, 0, 1);
	st->TCMstat = mrc(15, 0, 0, 2);
	st->MPUtype = mrc(15, 0, 0, 4);
	st->procf0 = mrc(15, 0, 1, 0);
	st->procf1 = mrc(15, 0, 1, 1);
	st->debugf0 = mrc(15, 0, 1, 2);
	st->auxf0 = mrc(15, 0, 1, 3);
	st->mmf0 = mrc(15, 0, 1, 4);
	st->mmf1 = mrc(15, 0, 1, 5);
	st->mmf2 = mrc(15, 0, 1, 6);
	st->mmf3 = mrc(15, 0, 1, 7);
	st->isf0 = mrc(15, 0, 2, 0);
	st->isf1 = mrc(15, 0, 2, 1);
	st->isf2 = mrc(15, 0, 2, 2);
	st->isf3 = mrc(15, 0, 2, 3);
	st->isf4 = mrc(15, 0, 2, 4);
	st->isf5 = mrc(15, 0, 2, 5);

#else	/* !mips && !arm */
#error	"Unrecognized architecture"
#endif	/* mips vs arm */

	w = (uint32 *)OSL_UNCACHED(base + swaptest_off);
	h = (uint16 *)w;
	b = (uint8 *)w;
	ww = w + 1;
	wh = (uint16 *)((uint32)w + 0x20);
	wb = (uint8 *)((uint32)w + 0x28);

	*w = 0x33221100;
	h0 = *h++;
	*ww++ = (uint32)h0;
	*wh++ = h0;
	*wh++ = ~h0;

	h1 = *h++;
	*ww++ = (uint32)h1;
	*wh++ = h1;
	*wh++ = ~h1;

	b0 = *b++;
	ww++;	/* Skip to 0x10 */
	*ww++ = (uint32)b0;
	*wb++ = b0++;
	*wb++ = b0++;
	*wb++ = b0++;
	*wb++ = b0++;

	b1 = *b++;
	*ww++ = (uint32)b1;
	*wb++ = b1++;
	*wb++ = b1++;
	*wb++ = b1++;
	*wb++ = b1++;

	b2 = *b++;
	*ww++ = (uint32)b2;
	*wb++ = b2++;
	*wb++ = b2++;
	*wb++ = b2++;
	*wb++ = b2++;

	b3 = *b++;
	*ww++ = (uint32)b3;
	*wb++ = b3++;
	*wb++ = b3++;
	*wb++ = b3++;
	*wb++ = b3++;

	st = (statarea_t *)OSL_UNCACHED(base + stat_off);

	st->wm = 0xdeadbeef;

	return;
}
