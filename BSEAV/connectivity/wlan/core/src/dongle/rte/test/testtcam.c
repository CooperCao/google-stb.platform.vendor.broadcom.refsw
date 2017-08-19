/*
 * Test routine to validate TCAM hardware
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: testtcam.c 241182 2013-12-11 21:50:03Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <osl.h>
#include <siutils.h>
#include <hndcpu.h>
#include <epivers.h>
#include <hndtcam.h>
#if !defined(__ARM_ARCH_7R__)
#if defined(__ARM_ARCH_7A__)
#include <sbsysmem.h>
#else
#include <sbsocram.h>
#endif
#endif	/* !defined(__ARM_ARCH_7R__) */

#include <rte.h>

/* This defines should be updated per chip */

#define KB							1024

/* size in KB */
#define ROMSIZE						640
#define ROMBANKSIZE					64

#define TCAM_MAX_ENTRY				256
#define ROM_BANKS					10

/* exclude flops section 32 bytes offset */
#define ROM_START					0x1e000000

/* check that PATCHCOUNT is 1, config it in hndtcam.h */
#define TCAM_PATCHBYTES				8

#define ROMSEGMENT_SIZE				((ROMSIZE*KB)/TCAM_MAX_ENTRY)
#define ROMPTR_SEGMENTINCR			(ROMSEGMENT_SIZE/TCAM_PATCHBYTES)

/* gets rounded off - for ex., 42 per bank in 4349 chip */
#define TCAMENTRIES_PERBANK			((ROMBANKSIZE*KB)/ROMSEGMENT_SIZE)
#define ROMPTR_BANKINCR				((ROMBANKSIZE*KB)/TCAM_PATCHBYTES)

/* Enable flag for individual tcam entry */
#define TCAM_ENTRY_ENABLE			1

/* Enable define below incase debug data needs to be gathered */
#define TCAM_DBGDATA

static si_t *sih;

/* original function hnd_tcam_reset is static so duplicating defn here */
void hnd_tcam_reset_duplicated(void *regs_p, void *ptbl);

void _c_main(unsigned long ra);

/* patch table i.e. data being patched */
/* The base address aligned on table size */
/* each entry is 8 bytes as expected by tcam on cr4 */
/* ensure this patch table is in ATCM only incase the test pgm is too big */
uint64 rambuf[TCAM_MAX_ENTRY] __attribute__ ((aligned(2048))) = {
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83,
84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112,
113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125,
126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151,
152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177,
178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190,
191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203,
204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216,
217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229,
230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242,
243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256 };

/* Buffer holding ROM data after tcam patching is done */
uint64 postpatchrombuf[TCAM_MAX_ENTRY] = {0, };

extern void *socram_regs;
extern void *arm_regs;
extern void *sysmem_regs;

void
_c_main(unsigned long ra)
{
	uint i = 0, j = 0;
	uint64* romptr = (uint64*)ROM_START;
	uint result = 1, sanity = 1;
#if defined(mips)
	void *regs_p = NULL;
#else
#if defined(__ARM_ARCH_7R__)
	cr4regs_t *regs_p = (cr4regs_t *)arm_regs;
#elif defined(__ARM_ARCH_7A__)
	sysmemregs_t *regs_p = (sysmemregs_t *)sysmem_regs;
#else
	sbsocramregs_t *regs_p = (sbsocramregs_t *)socram_regs;
#endif	/* defined(__ARM_ARCH_7R__) */
#endif	/* !mips */

	/* Basic initialization */
	sih = hnd_init();

	/* first reset the tcam HW */
	hnd_tcam_reset_duplicated((void *)regs_p, (void *)(&rambuf));

	/* setup hw tcam entries */
	/* entries are first double word locations in every ROM segment */
	/* along with address program valid bit in LSB */
	romptr = (uint64*)ROM_START;

	for (i = 0; i < TCAM_MAX_ENTRY; i++)
	{
		hnd_tcam_write((void*)regs_p, i, ((uint32)romptr | TCAM_ENTRY_ENABLE));
		romptr += ROMPTR_SEGMENTINCR;
	}

	/* enable TCAM hardware */
	hnd_tcam_enablepatch((void *)regs_p);

	/* now read from patched ROM locations */
	romptr = (uint64*)ROM_START;

	for (i = 0; i < TCAM_MAX_ENTRY; i++)
	{
		postpatchrombuf[i] = *romptr;
		romptr += ROMPTR_SEGMENTINCR;
	}

#ifdef TCAM_DBGDATA
	/* print data read from patched locations */
	/* format specifier %llu or %lld not supported in this gcc version */
	/* hence printing as two uint32 value .. needs to be interpreted */
	/* read back the TCAM hardware for verification */
	uint32 *postpatchbufptr, *rambufptr;
	postpatchbufptr = (uint32*)postpatchrombuf;
	rambufptr = (uint32*)rambuf;

	printf("postpatchufffer and rambuf\n");

	for (i = 0; i < TCAM_MAX_ENTRY; )
	{
		printf("\t%u\t%u\t%u\t%u\n", *(postpatchbufptr), *(postpatchbufptr+1),
			*(rambufptr), *(rambufptr+1));
		i++;
		postpatchbufptr += 2;
		rambufptr += 2;
	}
#endif /* TCAM_DBGDATA */

	/* compare data read against patch table data */
	for (i = 0; i < TCAM_MAX_ENTRY; i++)
	{
		if (postpatchrombuf[i] != rambuf[i])
		{
			result = 0;
			break;
		}
	}

	/* check for same offset different bank corruption - 4335a0 issues */
	romptr = (uint64*)ROM_START;

	/* parse thru all patched locations of bank 0 */
	for (i = 0; i < TCAMENTRIES_PERBANK; i++)
	{
		/* compare against corresponding offsets in rest of the banks */
		for (j = 1; j < ROM_BANKS; j++)
		{
			if (*romptr == *(romptr+(j*ROMPTR_BANKINCR)))
				{
					/* exit now as corruption is observed */
					result = 0;
					sanity = 0;
					break;
				}
		}
		/* complete exit even if single failure */
		if (sanity == 0) {
			break;
		}
		/* continue with the remaining patch entries */
		romptr += ROMPTR_SEGMENTINCR;
	}

	printf("\n Comparing rom data after patching against patch table\n");
	printf("\n 0:failed 1:passed\n");
	printf("\n tcam test result is %d and sanity check is %d\n", result, sanity);
	while (1);
}

#if defined(__ARM_ARCH_7R__)
void *armp = NULL;
#endif

/* original defn in hndtcam.c is static so duplicating here */
void hnd_tcam_reset_duplicated(void *regs_p, void *ptbl)
{
	int i, tot;
#if defined(__ARM_ARCH_7R__)
	uint32 patchctrl;
#elif defined(__ARM_ARCH_7A__)
	sysmemregs_t *sr = (sysmemregs_t *)regs_p;
#else
	sbsocramregs_t *sr = (sbsocramregs_t *)regs_p;
#endif	/* defined(__ARM_ARCH_7R__) */

	tot = TCAM_MAX_ENTRY;

	hnd_tcam_disablepatch(regs_p);

#if defined(__ARM_ARCH_7R__)
	/* Set the clock and patch count */
	ASSERT(ARMCR4_TCAMPATCHCOUNT > 0);
	ASSERT(ARMCR4_TCAMPATCHCOUNT < 5);

	/* Write patch count and enable clock */
	patchctrl = R_REG(SI_OSH, ARMREG(armp, tcampatchctrl));
	patchctrl &= ~(ARMCR4_TCAM_PATCHCNT_MASK);
	patchctrl |= (ARMCR4_TCAM_CLKENAB | ARMCR4_TCAMPATCHCOUNT);

	W_REG(SI_OSH, ARMREG(armp, tcampatchctrl), patchctrl);
	W_REG(SI_OSH, ARMREG(armp, tcampatchtblbaseaddr), (uint32) ptbl);
#else
	/* Write patch count and enable clock */
	W_REG(SI_OSH, &sr->cambankpatchctrl, (SRPC_PATCHCOUNT));
	W_REG(SI_OSH, &sr->cambankpatchtblbaseaddr, (uint32)ptbl);
#endif	/* !defined(__ARM_ARCH_7R__) */

	/* Initialize cam entries to zero */
	for (i = 0; i < tot; i++)
		hnd_tcam_write(regs_p, i, 0);
}
