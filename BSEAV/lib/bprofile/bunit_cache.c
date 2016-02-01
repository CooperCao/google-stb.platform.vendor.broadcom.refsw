/***************************************************************************
 *     Copyright (c) 2006-2008, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Profiler suite
 * 		CPU bus unit tests
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "batomic.h"
#include "btrc.h"

BTRC_MODULE(bunit_cache,ENABLE);

#define  B_TEST_ROUNDS	(16*1024)

#define b_nop() __asm__ __volatile__( \
		".set\tpush\n\t"  \
		".set\tnoreorder\n\t"       \
		"	nop	\n" \
		"	nop	\n"  \
		".set\tpop\n")

#define b_nop_64() \
	b_nop();b_nop();b_nop();b_nop(); /* 8 */ \
	b_nop();b_nop();b_nop();b_nop(); /* 16 */ \
	b_nop();b_nop();b_nop();b_nop(); /* 24 */ \
	b_nop();b_nop();b_nop();b_nop(); /* 32 */ \
	b_nop();b_nop();b_nop();b_nop(); /* 40 */ \
	b_nop();b_nop();b_nop();b_nop(); /* 48 */ \
	b_nop();b_nop();b_nop();b_nop(); /* 56 */ \
	b_nop();b_nop();b_nop();b_nop()  /* 64 */

#define b_nop_4096k() \
	b_nop_64(); b_nop_64(); b_nop_64(); b_nop_64();  /* 1024 */ \
	b_nop_64(); b_nop_64(); b_nop_64(); b_nop_64();  /* 2048 */ \
	b_nop_64(); b_nop_64(); b_nop_64(); b_nop_64();  /* 3072 */ \
	b_nop_64(); b_nop_64(); b_nop_64(); b_nop_64()  /* 4096 */ 

static void
b_test_icache_4k(volatile uint32_t *ptr, uint32_t val)
{
	unsigned i;
	BSTD_UNUSED(val);
	BSTD_UNUSED(ptr);

	for(i=0;i<B_TEST_ROUNDS/4 ;i++) {
		b_nop_4096k();
	}
}

static void
b_test_icache_8k(volatile uint32_t *ptr, uint32_t val)
{
	unsigned i;
	BSTD_UNUSED(val);
	BSTD_UNUSED(ptr);

	for(i=0;i<B_TEST_ROUNDS/8;i++) {
		b_nop_4096k();
		b_nop_4096k();
	}
}

static void
b_test_icache_16k(volatile uint32_t *ptr, uint32_t val)
{
	unsigned i;
	BSTD_UNUSED(val);
	BSTD_UNUSED(ptr);

	for(i=0;i<B_TEST_ROUNDS/16;i++) {
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
	}
}

static void
b_test_icache_32k(volatile uint32_t *ptr, uint32_t val)
{
	unsigned i;
	BSTD_UNUSED(val);
	BSTD_UNUSED(ptr);

	for(i=0;i<B_TEST_ROUNDS/32;i++) {
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
	}
}

static void
b_test_icache_64k(volatile uint32_t *ptr, uint32_t val)
{
	unsigned i;
	BSTD_UNUSED(val);
	BSTD_UNUSED(ptr);

	for(i=0;i<B_TEST_ROUNDS/64;i++) {
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
	}
}

static void
b_test_icache_128k(volatile uint32_t *ptr, uint32_t val)
{
	unsigned i;
	BSTD_UNUSED(val);
	BSTD_UNUSED(ptr);

	for(i=0;i<B_TEST_ROUNDS/128;i++) {
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
	}
}

static void
b_test_icache_256k(volatile uint32_t *ptr, uint32_t val)
{
	unsigned i;
	BSTD_UNUSED(val);
	BSTD_UNUSED(ptr);

	for(i=0;i<B_TEST_ROUNDS/256;i++) {
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();

		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
		b_nop_4096k();
    }
}


static void 
b_unit_bus_run(const char *name, volatile uint32_t *ptr, void (*test)(volatile uint32_t *ptr, uint32_t val))
{
	unsigned i;
	BKNI_EnterCriticalSection();
	for(i=0;i<5;i++) {
		uint32_t val = *ptr;
		BTRC_TRACE(bunit_cache, START);
		test(ptr, val); 
		BTRC_TRACE(bunit_cache, STOP);
	}
	BKNI_LeaveCriticalSection();
	BKNI_Printf("test '%s' addr %#x\n", name, (unsigned)ptr);
	BTRC_Module_Report(BTRC_MODULE_HANDLE(bunit_cache));
	BTRC_Module_Reset(BTRC_MODULE_HANDLE(bunit_cache));
	BKNI_Sleep(50);
	return;
}

void 
bunit_cache(void)
{
	void *addr = BKNI_Malloc(4); /* we use malloced memory because it would go from the KSEG0 (static memory is from KSEG2, and MMU mapped) */
	volatile uint32_t *ptr = addr;
	BDBG_ASSERT(addr);

	b_unit_bus_run("b_test_icache_4k/cached", ptr, b_test_icache_4k);
	b_unit_bus_run("b_test_icache_8k/cached", ptr, b_test_icache_8k);
	b_unit_bus_run("b_test_icache_16k/cached", ptr, b_test_icache_16k);
	b_unit_bus_run("b_test_icache_32k/cached", ptr, b_test_icache_32k);
	b_unit_bus_run("b_test_icache_64k/cached", ptr, b_test_icache_64k);
	b_unit_bus_run("b_test_icache_128k/cached", ptr, b_test_icache_128k);
	b_unit_bus_run("b_test_icache_256k/cached", ptr, b_test_icache_256k);

	BKNI_Free(addr);
	return;
}

