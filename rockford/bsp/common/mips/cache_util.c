/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
/***************************************************************
**
** Broadcom Corp. Confidential
** Copyright 1998-2000 Broadcom Corp. All Rights Reserved.
**
** THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED
** SOFTWARE LICENSE AGREEMENT BETWEEN THE USER AND BROADCOM.
** YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT
** SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
**
** File:		cache_util.c
** Description: 	cache handling utilities.
**
** Created: 06/07/04 by Jeff Fisher
**
**
**
****************************************************************/
#include <stdio.h>
#include "cache_util.h"
#include "bcm_mips_defs.h"

#define CU_DBG(x)	printf x
#define	RAC_CONFIGURATION_REGISTER	0xFF400000
#define	RAC_ADDRESS_RANGE_REGISTER	0xFF400004

#if 1 /* (BCHP_CHIP==7550)*/
unsigned int dcache_size;
unsigned int dcache_linesize;
unsigned int icache_size;
unsigned int icache_linesize;
#endif

/* cache instructions invalidate RAC */
#define invalidate_rac_all() ((void)0)

static void rac_init(void)
{
#if 0 /* (BCHP_CHIP!=7550) */
	unsigned int flags;
#endif
	
	invalidate_rac_all();

#if 0 /* (BCHP_CHIP!=7550) */
	flags = bos_enter_critical();
	*((volatile unsigned int *)RAC_ADDRESS_RANGE_REGISTER) = 0x00FF0000;  /* 0 - 16M */
	/* set to default - C_INV='0'b, PF_D='1'b, PF_I='1'b, RAC_D='1'b, RAC_I='1'b */
	*((volatile unsigned int *)RAC_CONFIGURATION_REGISTER) |= 0x0000000F;

	bos_exit_critical(flags);

	CU_DBG(("after init RAC 0x%08x    0x%08x\n", 
			*((volatile unsigned int *)RAC_CONFIGURATION_REGISTER), 
			*((volatile unsigned int *)RAC_ADDRESS_RANGE_REGISTER)));
#endif
}

void calc_cache_sizes(void)
{
    volatile unsigned int val;
	int da,dl,ds,ia,il,is;
#ifdef GHS
    val = __MFC0(16, 1);
#else
	val = bcm_read_cp0($16, 1);
#endif    
	da = (val & (0x7 << 7)) >> 7;
	ia = (val & (0x7 << 16)) >> 16;
	dl = (val & (0x7 << 10)) >> 10;
	il = (val & (0x7 << 19)) >> 19;
	ds = (val & (0x7 << 13)) >> 13;
	is = (val & (0x7 << 22)) >> 22;

	/*CU_DBG(("D[%d,%d,%d]\n",da,dl,ds));*/
	/*CU_DBG(("I[%d,%d,%d]\n",da,dl,ds));*/
	dcache_linesize =(0x2 << dl);
	dcache_size =  (da + 1) * dcache_linesize * (64 << ds);
	/*CU_DBG(("D[linsize = %d, size = %d]\n",dcache_linesize,dcache_size ));*/
	icache_linesize = (0x2 << il);
	icache_size = (ia + 1) * icache_linesize * (64 << is);
	/*CU_DBG(("I[linsize = %d, size = %d]\n",icache_linesize,icache_size ));*/
}


void print_cache_sizes(void)
{
    unsigned int val;
	unsigned da,dl,ds,ia,il,is;
#ifdef GHS
    val = __MFC0(16, 1);
#else
	val = bcm_read_cp0($16, 1);
#endif    
	da = (val & (0x7 << 7)) >> 7;
	ia = (val & (0x7 << 16)) >> 16;
	dl = (val & (0x7 << 10)) >> 10;
	il = (val & (0x7 << 19)) >> 19;
	ds = (val & (0x7 << 13)) >> 13;
	is = (val & (0x7 << 22)) >> 22;
	CU_DBG(("CONFIG1[0x%08x]\n",val));
	CU_DBG(("D[%d,%d,%d]\n",da,dl,ds));
	CU_DBG(("I[%d,%d,%d]\n",ia,il,is));
	CU_DBG(("D[linesize = %d, size = %d]\n",dcache_linesize,dcache_size ));
	CU_DBG(("I[linesize = %d, size = %d]\n",icache_linesize,icache_size ));

	rac_init();
}

#define cache_op(op,addr)						\
	__asm__ __volatile__(						\
	"	.set	noreorder				\n"	\
	"	.set	mips3\n\t				\n"	\
	"	cache	%0, %1					\n"	\
	"	.set	mips0					\n"	\
	"	.set	reorder"					\
	:								\
	: "i" (op), "m" (*(unsigned char *)(addr)))

#define Index_Invalidate_I      0x00
#define Index_Writeback_Inv_D   0x01
#define Index_Load_Tag_I	0x04
#define Index_Load_Tag_D	0x05
#define Index_Store_Tag_I	0x08
#define Index_Store_Tag_D	0x09
#define Hit_Invalidate_I	0x10
#define Hit_Invalidate_D	0x11
#define Hit_Writeback_Inv_D	0x15

void invalidate_icache_all(void)
{
#ifdef CONFIG_ENABLE_I_CACHE
	unsigned int addr;
	__asm__ (" .set push\n .set mips32\n sync \n .set pop\n");

	__asm__ (" .set push\n .set mips32\n mtc0 $0,$28 \n mtc0 $0,$29 \n .set pop\n");
	for (addr = 0x80000000; addr < (0x80000000 + icache_size); addr += icache_linesize)
	{
		cache_op(Index_Store_Tag_I,addr);
	}
	__asm__ (" nop\n nop\n nop\n nop\n\n");
	invalidate_rac_all();
#endif /* CONFIG_ENABLE_I_CACHE */
}

void flush_dcache_all(void)
{
#ifdef CONFIG_ENABLE_D_CACHE
	unsigned int addr=0x80000000;
	unsigned end = addr+dcache_size;
	unsigned linesize = dcache_linesize;

	for (; addr < end ; addr += linesize)
	{
		cache_op(Index_Writeback_Inv_D,addr); 
	}
	__asm__ (" .set push\n .set mips32\n sync \n .set pop\n");
	invalidate_rac_all();
#endif /* CONFIG_ENABLE_D_CACHE */
}

void flush_dcache(unsigned int start, unsigned int end)
{
#ifdef CONFIG_ENABLE_D_CACHE
	unsigned int addr;
	unsigned linesize = dcache_linesize;

	end += linesize;

	if (end  >= (start + dcache_size))
	{
		flush_dcache_all();
		return;
	}

	for (addr = start; addr < end; addr += linesize)
	{
		cache_op(Hit_Writeback_Inv_D,addr); 
	}
	__asm__ (" .set push\n .set mips32\n sync \n .set pop\n");
	invalidate_rac_all();
#else
	BSTD_UNUSED(start);
	BSTD_UNUSED(end);
#endif /* CONFIG_ENABLE_D_CACHE */
}



void invalidate_dcache_all(void)
{
#ifdef CONFIG_ENABLE_D_CACHE
	unsigned int addr=0x80000000;
	unsigned end = addr+dcache_size;
	unsigned linesize = dcache_linesize;

	for (; addr < end ; addr += linesize)
	{
		cache_op(Index_Writeback_Inv_D,addr); 
	}
	__asm__ (" .set push\n .set mips32\n sync \n .set pop\n");
	invalidate_rac_all();
#endif /* CONFIG_ENABLE_D_CACHE */
}

void invalidate_dcache(unsigned int start, unsigned int end)
{
#ifdef CONFIG_ENABLE_D_CACHE
	unsigned int addr;
	unsigned linesize = dcache_linesize;

	end += linesize;

	if (end  >= (start + dcache_size))
	{
		invalidate_dcache_all();
		return;
	}

	for (addr = start; addr < end; addr += linesize)
	{
		cache_op(Hit_Invalidate_D,addr); 
	}
	__asm__ (" .set push\n .set mips32\n sync \n .set pop\n");
	invalidate_rac_all();
#else
	BSTD_UNUSED(start);
	BSTD_UNUSED(end);
#endif /* CONFIG_ENABLE_D_CACHE */
}
