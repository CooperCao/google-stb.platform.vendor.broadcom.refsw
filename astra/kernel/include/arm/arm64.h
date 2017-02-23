/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#ifndef _B_TZ_ARM64_H
#define _B_TZ_ARM64_H

#ifndef __ASSEMBLY__
#include <stdint.h>
#include "arch_helpers.h"
#include "smcall.h"
#endif /* __ASSEMBLY__ */

/* Interrupt handling for arm64 */
#define DAIFBIT_FIQ			(1 << 0)
#define DAIFBIT_IRQ			(1 << 1)
#define DAIFBIT_ABT			(1 << 2)
#define DAIFBIT_DBG			(1 << 3)
#define DAIFBIT_ALL			(DAIFBIT_FIQ | DAIFBIT_IRQ | \
					 DAIFBIT_ABT | DAIFBIT_DBG)

#define DAIF_F_SHIFT		6
#define DAIF_F			(1 << 6)
#define DAIF_I			(1 << 7)
#define DAIF_A			(1 << 8)
#define DAIF_D			(1 << 9)
#define DAIF_AIF		(DAIF_A | DAIF_I | DAIF_F)

/* ARMv8 Execution Modes */
#define Mode_EL0t		 0x00
#define Mode_EL1t		 0x04
#define Mode_EL1h		 0x05

/* SPSR mapping */
#define Mode_USR         Mode_EL0t
#define Mode_SVC		 Mode_EL1t

/*
 * ARM CPU core registers
 */
/* TODO: Probably not right in aarch64 */
#define REG_R0		0
#define REG_R1		1
#define REG_R2		2
#define REG_R3		3
#define REG_R4		4
#define REG_R5		5
#define REG_R6		6
#define REG_R7		7
#define REG_R8		8
#define REG_R9		9
#define REG_R10		10
#define REG_R11		11
#define REG_R12		12
#define REG_SP		13
#define REG_LR		14
#define REG_PC		15

#define REG_CPSR	16

#define REG_SVC_SP		17
#define REG_SVC_LR		18
#define REG_SVC_SPSR	19

#define REG_IRQ_SP		20
#define REG_IRQ_LR		21
#define REG_IRQ_SPSR	22

#define REG_ABT_SP		23
#define REG_ABT_LR		24
#define REG_ABT_SPSR	25

#define REG_UND_SP		26
#define REG_UND_LR		27
#define REG_UND_SPSR	28

#define REG_MON_SP		29
#define REG_MON_LR		30
#define REG_MON_SPSR	31

#define REG_HYP_SP		32
#define REG_HYP_LR		33
#define REG_HYP_SPSR	34

#define	REG_FIQ_R8		35
#define REG_FIQ_R9		36
#define REG_FIQ_R10		37
#define REG_FIQ_R11		38
#define REG_FIQ_R12		39
#define REG_FIQ_SP		40
#define REG_FIQ_LR		41
#define REG_FIQ_SPSR	42

#define ARCH_SPECIFIC_NUM_CORE_REGS 43

/*
 * Memory attributes
 */
#define ATTR_DEVICE_NGNRNE	0x00UL
#define ATTR_DEVICE_NGNRE	0x04UL
#define ATTR_DEVICE_NGRE	0x08UL
#define ATTR_DEVICE_GRE		0x0CUL

#define ATTR_MEM_TRANSIENT_WRITE_BACK			0x77UL
#define ATTR_MEM_NON_TRANSIENT_WRITE_BACK		0xFFUL
#define ATTR_MEM_NON_TRANSIENT_WRITE_THROUGH	0xBBUL
#define ATTR_MEM_NON_CACHEABLE					0x44UL

#define MAIR_EL1_VAL	((ATTR_DEVICE_NGNRNE << 56) | (ATTR_DEVICE_NGNRE << 48) | (ATTR_DEVICE_NGRE << 40) | (ATTR_DEVICE_GRE << 32) | (ATTR_MEM_TRANSIENT_WRITE_BACK << 24) | (ATTR_MEM_NON_TRANSIENT_WRITE_BACK << 16) | (ATTR_MEM_NON_TRANSIENT_WRITE_THROUGH << 8) | ATTR_MEM_NON_CACHEABLE)

#define MAIR_DEVICE 				7
#define MAIR_MEMORY					2
#define MAIR_MEMORY_NON_CACHEABLE	0

/*
 * 4KB granule size,  4-level page table, 48bit VA to 40 bit IPA:
 *
 *
 *		Virtual Address:
 * 		I--------I--------I--------I--------I-------------------I
 * 		47	   39 38	30 29	 21 20    12 11 			    0
 * 			 |		  |		   |	    |            |
 * 			 |		  |		   |		|		     |
 * 			 |		  |		   |		|			 |
 * 			 V		  V		   V	    V	         V
 * 		Index to	Index     Index     Index       Offset within the page
 * 		L0 table	L1 Table  L2 Table  L3 Table
 *
 */


#define GRANULE_4KB_TABLE_NUM_ENTRIES  64
#define L0_PAGE_NUM_ENTRIES     512
#define L0_PAGE_TABLE_SLOT(x)  (int)(((unsigned long)x >> 39) & 0x1ff)

#define L1_BLOCK_ADDR_MASK      0xfffffffffffff000L
#define L1_BLOCK_SHIFT          30
#define L1_BLOCK_MASK           0x1ff
#define L1_PHYS_ADDR_MASK       0xC0000000
#define L1_PAGE_NUM_ENTRIES     512
#define L1_PAGE_TABLE_SLOT(x)  (int)(((unsigned long)x >> 30) & 0x1ff)

#define L2_BLOCK_ADDR_MASK      0xfffffffffffff000L
#define L2_BLOCK_SHIFT           21
#define L2_BLOCK_MASK            0x1ff
#define L2_PHYS_ADDR_MASK        0xFFE00000
#define L2_PAGE_NUM_ENTRIES  512
#define L2_PAGE_TABLE_SLOT(x)  (int)(((unsigned long)x >> 21) & 0x1ff)

#define L3_BLOCK_ADDR_MASK      0xfffffffffffff000L
#define L3_BLOCK_SHIFT           12
#define L3_BLOCK_MASK            0x1ff
#define L3_PHYS_ADDR_MASK        0xFFFFF000
#define L3_PAGE_NUM_ENTRIES  512
#define L3_PAGE_TABLE_SLOT(x)  (int)(((unsigned long)x >> 12) & 0x1ff)

#define INVALID_TABLE_DESCRIPTOR  0UL
#define MAKE_TABLE_DESCRIPTOR(tablePhysAddr) (((unsigned long)tablePhysAddr & 0x0000fffffffff000UL) | 0x3UL)
#define IS_VALID_TABLE_DESCRIPTOR(tdesc) (((unsigned long)tdesc & 0x03UL) == 0x03UL)
#define IS_VALID_BLOCK_DESCRIPTOR(tdesc) (((unsigned long)tdesc & 0x03UL) == 0x01UL)

#define TABLE_PHYS_ADDR(tdesc) (tdesc & 0x0000fffffffff000UL)

#define TABLE_DESCRIPTOR_NS(tdesc) ((tdesc >> 63) & 0x1UL)
#define CLEAR_TABLE_DESCRIPTOR_NS(tdesc) tdesc &= ~( 0x1UL << 63)
#define CHANGE_TABLE_DESCRIPTOR_NS(tdesc, ns) CLEAR_TABLE_DESCRIPTOR_NS(tdesc); tdesc |= (((unsigned long)ns & 0x1UL) << 63)

#define TABLE_DESCRIPTOR_AP(tdesc) ((tdesc >> 61) & 0x3UL)
#define CLEAR_TABLE_DESCRIPTOR_AP(tdesc) tdesc &= ~( 0x3UL << 61)
#define CHANGE_TABLE_DESCRIPTOR_AP(tdesc, ns) CLEAR_TABLE_DESCRIPTOR_AP(tdesc); tdesc |= (((unsigned long)ap & 0x3UL) << 61)

#define TABLE_DESCRIPTOR_XN(tdesc) ((tdesc >> 60) & 0x1UL)
#define CLEAR_TABLE_DESCRIPTOR_XN(tdesc) tdesc &= ~( 0x1UL << 60)
#define CHANGE_TABLE_DESCRIPTOR_XN(tdesc, xn) CLEAR_TABLE_DESCRIPTOR_XN(tdesc); tdesc |= (((unsigned long)xn & 0x1UL) << 60)

#define TABLE_DESCRIPTOR_PXN(tdesc) ((tdesc >> 59) & 0x1UL)
#define CLEAR_TABLE_DESCRIPTOR_PXN(tdesc) tdesc &= ~( 0x1UL << 59)
#define CHANGE_TABLE_DESCRIPTOR_PXN(tdesc, pxn) CLEAR_TABLE_DESCRIPTOR_PXN(tdesc); tdesc |= (((unsigned long)xn & 0x1UL) << 59)

#define MAKE_PAGE_DESCRIPTOR(pagePhysAddr) (((unsigned long)pagePhysAddr & 0x0000fffffffff000UL) | 0x3UL)
#define MAKE_BLOCK_DESCRIPTOR_2M(pagePhysAddr) (((unsigned long)pagePhysAddr & 0x0000ffffffe00000UL) | 0x1UL)

#define PAGE_DESCRIPTOR_SW(pdesc) ((pdesc >> 55) & 0xfUL)
#define CLEAR_PAGE_DESCRIPTOR_SW(pdesc) pdesc &= ~( 0xfUL << 55)
#define CHANGE_PAGE_DESCRIPTOR_SW(pdesc, sw) CLEAR_PAGE_DESCRIPTOR_SW(pdesc); pdesc |= (((unsigned long)sw & 0xfUL) << 55)

#define PAGE_DESCRIPTOR_XN(pdesc) ((pdesc >> 54) & 0x1UL)
#define CLEAR_PAGE_DESCRIPTOR_XN(pdesc) pdesc &= ~( 0x1UL << 54)
#define CHANGE_PAGE_DESCRIPTOR_XN(pdesc, xn) CLEAR_PAGE_DESCRIPTOR_XN(pdesc); pdesc |= (((unsigned long)xn & 0x1UL) << 54)

#define PAGE_DESCRIPTOR_PXN(pdesc) ((pdesc >> 53) & 0x1UL)
#define CLEAR_PAGE_DESCRIPTOR_PXN(pdesc) pdesc &= ~( 0x1UL << 53)
#define CHANGE_PAGE_DESCRIPTOR_PXN(pdesc, pxn) CLEAR_PAGE_DESCRIPTOR_PXN(pdesc); pdesc |= (((unsigned long)pxn & 0x1UL) << 53)

#define PAGE_DESCRIPTOR_NG(pdesc) ((pdesc >> 11) & 0x1UL)
#define CLEAR_PAGE_DESCRIPTOR_NG(pdesc) pdesc &= ~( 0x1UL << 11)
#define CHANGE_PAGE_DESCRIPTOR_NG(pdesc, ng) CLEAR_PAGE_DESCRIPTOR_NG(pdesc); pdesc |= (((unsigned long)ng & 0x1UL) << 11)

#define PAGE_DESCRIPTOR_AF(pdesc) ((pdesc >> 10) & 0x1UL)
#define CLEAR_PAGE_DESCRIPTOR_AF(pdesc) pdesc &= ~( 0x1UL << 10)
#define CHANGE_PAGE_DESCRIPTOR_AF(pdesc, af) CLEAR_PAGE_DESCRIPTOR_AF(pdesc); pdesc |= (((unsigned long)af & 0x1UL) << 10)

#define PAGE_DESCRIPTOR_SH(pdesc) ((pdesc >> 8) & 0x3UL)
#define CLEAR_PAGE_DESCRIPTOR_SH(pdesc) pdesc &= ~( 0x3UL << 8)
#define CHANGE_PAGE_DESCRIPTOR_SH(pdesc, sh) CLEAR_PAGE_DESCRIPTOR_SH(pdesc); pdesc |= (((unsigned long)sh & 0x3UL) << 8)

#define PAGE_DESCRIPTOR_AP(pdesc) ((pdesc >> 6) & 0x3UL)
#define CLEAR_PAGE_DESCRIPTOR_AP(pdesc) pdesc &= ~( 0x3UL << 6)
#define CHANGE_PAGE_DESCRIPTOR_AP(pdesc, ap) CLEAR_PAGE_DESCRIPTOR_AP(pdesc); pdesc |= (((unsigned long)ap & 0x3UL) << 6)

#define PAGE_DESCRIPTOR_NS(pdesc) ((pdesc >> 5) & 0x1UL)
#define CLEAR_PAGE_DESCRIPTOR_NS(pdesc) pdesc &= ~( 0x1UL << 5)
#define CHANGE_PAGE_DESCRIPTOR_NS(pdesc, ns) CLEAR_PAGE_DESCRIPTOR_NS(pdesc); pdesc |= (((unsigned long)ns & 0x1UL) << 5)

#define PAGE_DESCRIPTOR_AI(pdesc) ((pdesc >> 2) & 0x7UL)
#define CLEAR_PAGE_DESCRIPTOR_AI(pdesc) pdesc &= ~( 0x7UL << 2)
#define CHANGE_PAGE_DESCRIPTOR_AI(pdesc, ai) CLEAR_PAGE_DESCRIPTOR_AI(pdesc); pdesc |= (((unsigned long)ai & 0x7UL) << 2)

#define PAGE_DESCRIPTOR_AI_STAGE2(pdesc) ((pdesc >> 2) & 0xFUL)
#define CLEAR_PAGE_DESCRIPTOR_AI_STAGE2(pdesc) pdesc &= ~( 0xFUL << 2)
#define CHANGE_PAGE_DESCRIPTOR_AI_STAGE2(pdesc, ai) CLEAR_PAGE_DESCRIPTOR_AI_STAGE2(pdesc); pdesc |= (((unsigned long)ai & 0xfUL) << 2)

#define PAGE_DESCRIPTOR_TLB_NOT_GLOBAL  0x1
#define PAGE_DESCIPTOR_TLB_GLOBAL		0x0

#define PAGE_DESCRIPTOR_PAGE_PRESENT      0x1
#define PAGE_DESCRIPTOR_PAGE_NOT_PRESENT  0x0

#define PAGE_DESCRIPTOR_PAGE_NON_SHAREABLE		0x0
#define PAGE_DESCRIPTOR_PAGE_INNER_SHAREABLE	0x3
#define PAGE_DESCRIPTOR_PAGE_OUTER_SHAREABLE	0x2

#define PAGE_DESCRIPTOR_EL1RW_EL0N	0x0
#define PAGE_DESCRIPTOR_EL1RW_EL0RW	0x1
#define PAGE_DESCRIPTOR_EL1RO_EL0N	0x2
#define PAGE_DESCRIPTOR_EL1RO_EL0RO	0x3

#define PAGE_DESCRIPTOR_STAGE2_NONE	0x0
#define PAGE_DESCRIPTOR_STAGE2_RO	0x1
#define PAGE_DESCRIPTOR_STAGE2_WO	0x2
#define PAGE_DESCRIPTOR_STAGE2_RW	0x3

#define TTBR0_BASE  0x0UL
#define TTBR1_BASE	0xffff000000000000UL

/* Macros for 2MB pages */
#define PAGE_START_2M(x)   (void*)( (uintptr_t)(x)  &  0xFFFFFFFFFFE00000L)
#define PAGE_SIZE_2M_BYTES        (2*1024*1024)
#define PAGE_SIZE_2M_WORDS        (512*1024)

/* Macros for 4KB pages */
#define PAGE_START_4K(x)   (void*)( (uintptr_t)(x)  &  0xFFFFFFFFFFFFF000L)
#define PAGE_SIZE_4K_BYTES        (4*1024)

#define PAGE_SIZE_4K_WORDS        (1024)

#define PAGE_MASK       0xFFFFFFFFFFFFF000L

/*
 * Page table entry fields
 */
 /* TODO: Probably not right in aarch64 */
#define NS_BIT_SECURE                   0
#define NS_BIT_NON_SECURE               1
#define SET_MEMORY_NS_BIT(x, ns)        x |= ((ns & 1) << 5)
#define GET_MEMORY_NS_BIT(x)            ((x >> 5) & 1)

#define ACCESS_FLAG_FAULT_GEN    		0
#define ACCESS_FLAG_NO_FAULT_GEN    	1
#define SET_MEMORY_ACCESS_FLAG(x, af) 	x |= ((af & 1) << 10)
#define GET_MEMORY_ACCESS_FLAG(x)		(( x >> 10) & 1)

#define OUTER_SHAREABLE					2
#define INNER_SHAREABLE					3
#define SET_MEMORY_SH_ATTR(x, sh)		x |= ((sh & 3) << 8)
#define GET_MEMORY_SH_ATTR(x)			((x >> 8) & 1)

#define CORTEX_A53_VIRTUAL_TIMER_INTERRUPT 27
#define CORTEX_A53_SECURE_TIMER_INTERRUPT 29
/*
 * Registers saved during TZ OS context switches
 */
#define NUM_SAVED_CPU_REGS	38
#define SAVED_REG_ELR_EL1	0
#define SAVED_REG_LR		SAVED_REG_ELR_EL1
#define SAVED_REG_SPSR_EL1	1
#define SAVED_REG_SPSR		SAVED_REG_SPSR_EL1
#define SAVED_REG_R30		2
#define SAVED_REG_LR_USR	SAVED_REG_R30
#define SAVED_REG_R31		3
#define SAVED_REG_R28		4
#define SAVED_REG_R29		5
#define SAVED_REG_R26		6
#define SAVED_REG_R27		7
#define SAVED_REG_R24		8
#define SAVED_REG_R25		9
#define SAVED_REG_R22		10
#define SAVED_REG_R23		11
#define SAVED_REG_R20		12
#define SAVED_REG_R21		13
#define SAVED_REG_R18		14
#define SAVED_REG_R19		15
#define SAVED_REG_R16		16
#define SAVED_REG_R17		17
#define SAVED_REG_R14		18
#define SAVED_REG_R15		19
#define SAVED_REG_R12		20
#define SAVED_REG_R13		21
#define SAVED_REG_R10		22
#define SAVED_REG_R11		23
#define SAVED_REG_R8		24
#define SAVED_REG_R9		25
#define SAVED_REG_R6		26
#define SAVED_REG_R7		27
#define SAVED_REG_R4		28
#define SAVED_REG_R5		29
#define SAVED_REG_R2		30
#define SAVED_REG_R3		31
#define SAVED_REG_R0		32
#define SAVED_REG_R1		33
#define SAVED_REG_SP_EL1	34
#define SAVED_REG_SP		SAVED_REG_SP_EL1
#define SAVED_REG_SP_EL0	35
#define SAVED_REG_SP_USR	SAVED_REG_SP_EL0
#define SAVED_REG_TPIDR_EL0 36
#define SAVED_REG_TPIDR_EL1 37

/* ARM MIDR register */
#define MIDR_PART_NUM_SHIFT 4
#define MIDR_PART_NUM_MASK  0xFFF
#define MPIDR_CPUID_MASK   0x3

/* LPAE Stage 1 table memory region attributes
 *
 *                AttrIndex     Encoding (binary)           Comments
 *  STRONG-ORDER    0             0000 0000       Strongly ordered. Use for device registers
 *  BUFFERABLE      1             0100 0100       Weakly ordered (normal memory) but not cached. Use for un-cached memory
 *  WRITETHROUGH    2             1010 1010       Cache write through. Use for pages shared between CPUs.
 *  WRITEBACK       4             1110 1110       Cache write back.                               * NOT SUPPORTED *
 *  DEV_SHARED      5             0000 0100       ??? When should we use this vs. uncached.       * NOT SUPPORTED *
 *  WRITEALLOC      7             1111 1111       Write back and write allocate.  Use for regular memory pages.
 */

#define MAIR0  0xeeaa4400
#define MAIR1  0xff000004

#define MAIR_STRONG_ORDERED           0
#define MAIR_BUFFERABLE               1
#define MAIR_WRITETHROUGH             2

//#define MAIR_WRITEBACK                4   * NOT SUPPORTED
//#define MAIR_DEV_SHARED               5   * NOT SUPPORTED
#define MAIR_WRITEALLOC               7

#define SET_MEMORY_ATTR(x,attr)		   x |= ((attr & 7) << 2)
#define GET_MEMORY_ATTR(x)			   ((x >> 2) & 7)

// LPAE memory access permissions: ARM architecture reference manual Table B3-6 VMSAv7 AP[2:1] access permissions model
#define MEMORY_ACCESS_RW_KERNEL        0
#define MEMORY_ACCESS_RW_USER		   1
#define MEMORY_ACCESS_RO_KERNEL		   2
#define MEMORY_ACCESS_RO_USER	       3

// See ARM Architecture Reference Manual figure B3-16 Memory attribute fields in Long-descriptor stage 1 Block and Page descriptors
#define SET_MEMORY_ACCESS_PERMS(x, ap)  x |= ((ap & 3) << 6)
#define GET_MEMORY_ACCESS_PERMS(x)		((x >> 6) & 3)
#define CLEAR_MEMORY_ACCESS_PERMS(x)	x = x & (~((uint64_t)0x3 << 6))

#define SET_MEMORY_ACCESS_NO_EXEC(x)	x |= ((uint64_t)1 << 54)

#define SET_MEMORY_ACCESS_SW_BITS(x, bits)    x |= ((uint64_t)(bits & 0xf) << 55)
#define GET_MEMORY_ACCESS_SW_BITS(x)	((x >> 55) & 0xf)
#define CLEAR_MEMORY_ACCESS_SW_BITS(x)	x = x & (~((uint64_t)0xf << 55))

/* DFSR (aka ESR_EL1) register in long description table format */
#define IS_CACHE_MAINTENANCE_FAULT(dfsr)  ((dfsr >> 8) & 1)
#define IS_EXT_DATA_ABORT(dfsr)			 ((dfsr >> 9) & 1)
#define IS_WRITE_DATA_ABORT(dfsr)		 ((dfsr >> 6) & 1)
#define FAULT_STATUS(dfsr)				 ((dfsr) & 0x3f)

#define CORTEX_A53_CACHE_LINE_SIZE   64

#ifndef __ASSEMBLY__

#define ARCH_SPECIFIC_CACHEFLUSH(startAddr,numBytes) { \
		const uint8_t *currLine = (const uint8_t *)startAddr; \
		const int numLines = numBytes/CORTEX_A53_CACHE_LINE_SIZE + 1; \
		for (int i=0; i<numLines; i++) { \
			asm volatile ("dc cvau, %[rt]" : : [rt] "r" (currLine) : "memory"); \
			asm volatile ("dc ivac, %[rt]" : : [rt] "r" (currLine) : "memory"); \
			currLine += CORTEX_A53_CACHE_LINE_SIZE; \
		} \
		asm volatile("dsb sy":::"memory"); \
		asm volatile("isb":::"memory"); \
		}

#define ARCH_SPECIFIC_ACTIVATE(top) { \
		register TzMem::PhysAddr pageTablePA = TzMem::virtToPhys(top); \
		register uintptr_t ttbr0_el1 = (uintptr_t)pageTablePA; \
		asm volatile("msr TTBR0_EL1, %[xt]"::[xt] "r" (ttbr0_el1)); \
		asm volatile("tlbi VMALLE1":::"memory"); \
		asm volatile("ic IALLU":::"memory"); \
}

#define ARCH_SPECIFIC_DCCIMVAC(a) { \
	asm volatile("dsb ishst":::"memory"); \
	asm volatile("isb":::"memory"); \
	asm volatile("dc cvac, %[xt]"::[xt] "r" (a):"memory"); \
	asm volatile("dsb ish":::"memory"); \
}

#define ARCH_SPECIFIC_DCIMVAC(a) { \
	asm volatile("dsb ishst":::"memory"); \
	asm volatile("isb":::"memory"); \
	asm volatile("dc ivac, %[xt]"::[xt] "r" (a):"memory"); \
	asm volatile("dsb ish":::"memory"); \
}

#define ARCH_SPECIFIC_DCCMVAC(a) { \
	asm volatile("dsb ishst":::"memory"); \
	asm volatile("isb":::"memory"); \
	asm volatile("dc cvac, %[xt]"::[xt] "r" (a):"memory"); \
	asm volatile("dsb ish":::"memory"); \
}

#define ARCH_SPECIFIC_GET_TTBR0(l,h) { \
	h = 0; \
	asm volatile("mrs %[xt], TTBR0_EL1":[xt] "=r" (l)::); \
}

#define ARCH_SPECIFIC_GET_TTBR1(l,h) { \
	h = 0; \
	asm volatile("mrs %[xt], TTBR1_EL1":[xt] "=r" (l)::); \
}

#define ARCH_SPECIFIC_TLB_FLUSH asm volatile("tlbi VMALLE1":::"memory")

#define ARCH_SPECIFIC_DMB \
	asm volatile("dmb sy":::"memory")

#define ARCH_SPECIFIC_MEMORY_BARRIER \
	asm volatile("isb":::"memory")

#define ARCH_SPECIFIC_GET_GIC_BASEADDR(a) asm volatile("mrs %[xt], s3_1_c15_c3_0" : [xt] "=r" (a) : :)

#define ARCH_SPECIFIC_TIMER_INTERRUPT CORTEX_A53_SECURE_TIMER_INTERRUPT

#define ARCH_SPECIFIC_SECURE_TIMER_ENABLE(enable) asm volatile("msr cntp_ctl_el0, %[xt]" : : [xt] "r" (enable) :)

#define ARCH_SPECIFIC_GET_SECURE_TIMER_CNTP_CTL(cntpctl) asm volatile("mrs %[xt],cntp_ctl_el0" : [xt] "=r" (cntpctl) : :)

#define ARCH_SPECIFIC_GET_SECURE_TIMER_CNTP_CVAL(hwTimerFiresAt) asm volatile("mrs %[xt], cntp_cval_el0" : [xt] "=r" (hwTimerFiresAt) : : )

#define ARCH_SPECIFIC_SECURE_TIMER_FIRE_AT(headTime)   asm volatile("msr cntp_cval_el0, %[xt]" :  : [xt] "r" (headTime) : )

#define ARCH_SPECIFIC_GET_SECURE_TIMER_CURRENT_TIME(timeNow) asm volatile("mrs %[xt], cntpct_el0" : [xt] "=r" (timeNow) : : )

#define ARCH_SPECIFIC_GET_SECURE_TIMER_FREQUENCY(rv) asm volatile("mrs %[xt],cntfrq_el0" : [xt] "=r" (rv) : :)


#define ARCH_SPECIFIC_NSWTASK \
	while (true) { \
		enable_fiq(); \
		enable_serror(); \
		asm volatile("mov	x0, #0x3c000000":::"x0"); \
		asm volatile("smc #0": : :"x0"); \
		disable_irq(); \
	}

#define ARCH_SPECIFIC_ENABLE_INTERRUPTS { \
	enable_irq(); \
	enable_fiq(); \
	enable_serror(); \
}

#define ARCH_SPECIFIC_GET_SPSR(spsr) asm volatile("mrs %[xt],spsr_el1": [xt] "=r" (spsr) : :)

/*  "mrs x29, spsr_el1\r\n" TODO : read from current pstate */
#define ARCH_SPECIFIC_SAVE_STATE(idx) \
	asm volatile ( \
			"msr daifset,#3\r\n" \
			"mov x0, %[xt]\r\n" \
			"clrex\r\n" \
			"stp x30, x1, [sp, #-16]!\r\n" \
			"adr x30, resumption\r\n" \
			"stp x30, x1, [x0, #16]\r\n" \
			"stp x28, x29,[x0, #32]\r\n" \
			"mov x28, x30\r\n" \
			"mov x29, #0x3e5 \r\n" \
			"stp x28, x29, [x0], #48\r\n" \
			"stp x26, x27, [x0], #16\r\n" \
			"stp x24, x25, [x0], #16\r\n" \
			"stp x22, x23, [x0], #16\r\n" \
			"stp x20, x21, [x0], #16\r\n" \
			"stp x18, x19, [x0], #16\r\n" \
			"stp x16, x17, [x0], #16\r\n" \
			"stp x14, x15, [x0], #16\r\n" \
			"stp x12, x13, [x0], #16\r\n" \
			"stp x10, x11, [x0], #16\r\n" \
			"stp x8, x9, [x0], #16\r\n" \
			"stp x6, x7, [x0], #16\r\n" \
			"stp x4, x5, [x0], #16\r\n" \
			"stp x2, x3, [x0], #16\r\n" \
			"mov x2, x0\r\n" \
			"stp x0, x1, [x2], #16\r\n" \
			"mov x0, sp\r\n" \
			"mrs x3, sp_el0\r\n" \
			"stp x0, x3, [x2], #16\r\n" \
			"mrs x0, tpidr_el0\r\n" \
			"mrs x3, tpidr_el1\r\n" \
			"stp x0, x3, [x2], #16\r\n" \
			"b schedule\r\n" \
			"resumption:\r\n" \
			"ldp x30, xzr, [sp], #16\r\n" \
			: :[xt] "r" (idx) :)

#define ARCH_SPECIFIC_DATA_ABORT_EXCEPTION(dfsr, dfar, align) \
	asm volatile("mrs %[xt],esr_el1": [xt] "=r" (dfsr)::); \
	asm volatile("mrs %[xt],far_el1":[xt] "=r" (dfar)::); \
	asm volatile("mrs %[xt],sctlr_el1":[xt] "=r" (align)::)


#define ARCH_SPECIFIC_SET_TPIDRURO(ti) 	\
	asm volatile("mov x2, %[xt]": :[xt] "r" (ti) :); \
	asm volatile("msr tpidr_el0, x2":::); \
	asm volatile("msr tpidrro_el0, x2":::)

#define ARCH_SPECIFIC_GET_DFAR(dfar) asm volatile("mrs %[xt],far_el1": [xt] "=r" (dfar)::)

#define ARCH_SPECIFIC_DISABLE_INTERRUPTS write_daifset(DAIFBIT_ALL)

#define ARCH_SPECIFIC_ENABLE_USER_PERF_MON asm volatile("msr pmuserenr_el0,%[xt]" : : [xt] "r" (0x1))

#define ARCH_SPECIFIC_CPU_ID(id) id = (read_midr()>>MIDR_PART_NUM_SHIFT)&MIDR_PART_NUM_MASK

#define ARCH_SPECIFIC_SMP_CPU_NUM(num) num = read_mpidr() & MPIDR_CPUID_MASK

#define ARCH_SPECIFIC_SYSCALL_NUM_REGISTER TzTask::UserRegs::r8

#define ARCH_SPECIFIC_GET_CTR(ctr) asm volatile("mrs %[xt],ctr_el0": [xt] "=r" (ctr)::)

#define ARCH_SPECIFIC_USER_REG_LIST \
	enum UserRegs { \
	r0 = SAVED_REG_R0, \
	r1 = SAVED_REG_R1, \
	r2 = SAVED_REG_R2, \
	r3 = SAVED_REG_R3, \
	r4 = SAVED_REG_R4, \
	r5 = SAVED_REG_R5, \
	r6 = SAVED_REG_R6, \
	r7 = SAVED_REG_R7, \
	r8 = SAVED_REG_R8, \
	r9 = SAVED_REG_R9, \
	r10= SAVED_REG_R10, \
	r11= SAVED_REG_R11, \
	r12= SAVED_REG_R12, \
	sp = SAVED_REG_SP, \
	lr = SAVED_REG_LR, \
	pc_unused = SAVED_REG_LR, \
	spsr = SAVED_REG_SPSR }

#define ARCH_SPECIFIC_CHUNK_MASK (0xFFFFFFFFFFFFC000)

#define ARCH_SPECIFIC_GET_TPIDRRO(tpidrro) asm volatile("mrs %[xt],tpidrro_el0": [xt] "=r" (tpidrro)::)
#define ARCH_SPECIFIC_GET_TPIDR(tpidr) asm volatile("mrs %[xt],tpidr_el0": [xt] "=r" (tpidr)::)

#endif /* __ASSEMBLY__ */
#endif /*_B_TZ_ARM64_H*/
