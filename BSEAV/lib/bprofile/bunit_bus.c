/***************************************************************************
 *     Copyright (c) 2006, Broadcom Corporation
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

BTRC_MODULE(bunit_bus,ENABLE);

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

static void
b_test_read(volatile uint32_t *ptr, uint32_t val)
{
	unsigned i;
	BSTD_UNUSED(val);

	for(i=0;i<B_TEST_ROUNDS;i++) {
		*ptr;
		*ptr;
		*ptr;
		*ptr;

		*ptr;
		*ptr;
		*ptr;
		*ptr;

		*ptr;
		*ptr;
		*ptr;
		*ptr;

		*ptr;
		*ptr;
		*ptr;
		*ptr;
	}
	return;
}

static void
b_test_write(volatile uint32_t *ptr, uint32_t val)
{
	unsigned i;

	for(i=0;i<B_TEST_ROUNDS;i++) {
		*ptr=val;
		*ptr=val;
		*ptr=val;
		*ptr=val;

		*ptr=val;
		*ptr=val;
		*ptr=val;
		*ptr=val;

		*ptr=val;
		*ptr=val;
		*ptr=val;
		*ptr=val;

		*ptr=val;
		*ptr=val;
		*ptr=val;
		*ptr=val;
	}
	return ;
}

static void
b_test_write_delay(volatile uint32_t *ptr, uint32_t val)
{
	unsigned i;

	for(i=0;i<B_TEST_ROUNDS;i++) {
		*ptr=val;
		b_nop_64();
		*ptr=val;
		b_nop_64();
		*ptr=val;
		b_nop_64();
		*ptr=val;
		b_nop_64();

		*ptr=val;
		b_nop_64();
		*ptr=val;
		b_nop_64();
		*ptr=val;
		b_nop_64();
		*ptr=val;
		b_nop_64();

		*ptr=val;
		b_nop_64();
		*ptr=val;
		b_nop_64();
		*ptr=val;
		b_nop_64();
		*ptr=val;
		b_nop_64();

		*ptr=val;
		b_nop_64();
		*ptr=val;
		b_nop_64();
		*ptr=val;
		b_nop_64();
		*ptr=val;
		b_nop_64();
	}
	return ;
}

static void 
b_unit_bus_run(const char *name, volatile uint32_t *ptr, void (*test)(volatile uint32_t *ptr, uint32_t val))
{
	unsigned i;
	BKNI_EnterCriticalSection();
	for(i=0;i<5;i++) {
		uint32_t val = *ptr;
		BTRC_TRACE(bunit_bus, START);
		test(ptr, val); 
		BTRC_TRACE(bunit_bus, STOP);
	}
	BKNI_LeaveCriticalSection();
	BKNI_Printf("test '%s' addr %#x\n", name, (unsigned)ptr);
	BTRC_Module_Report(BTRC_MODULE_HANDLE(bunit_bus));
	BTRC_Module_Reset(BTRC_MODULE_HANDLE(bunit_bus));
	BKNI_Sleep(50);
	return;
}

void 
bunit_bus(void)
{
	void *addr = BKNI_Malloc(4); /* we use malloced memory because it would go from the KSEG0 (static memory is from KSEG2, and MMU mapped) */
	volatile uint32_t *ptr = addr;
	BDBG_ASSERT(addr);

	b_unit_bus_run("b_test_read/cached", ptr, b_test_read);
	b_unit_bus_run("b_test_write/cached", ptr, b_test_write);
	b_unit_bus_run("b_test_write_delay/cached", ptr, b_test_write_delay);

	ptr = (uint32_t *)(0x20000000|(uint32_t)addr);

	b_unit_bus_run("b_test_read/uncached", ptr, b_test_read);
	b_unit_bus_run("b_test_write/uncached", ptr, b_test_write);
	b_unit_bus_run("b_test_write_delay/uncached", ptr, b_test_write_delay);

	ptr = (uint32_t *)0xB0082ffc;
	b_unit_bus_run("b_test_read/ENET_TOP_Descriptor", ptr, b_test_read);
	b_unit_bus_run("b_test_write/ENET_TOP_Descriptor", ptr, b_test_write);
	b_unit_bus_run("b_test_write_delay/ENET_TOP_Descriptor", ptr, b_test_write_delay);

	BKNI_Free(addr);
	return;
}

