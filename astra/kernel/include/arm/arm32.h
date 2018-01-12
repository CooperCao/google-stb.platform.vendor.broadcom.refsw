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
#ifndef _B_TZ_ARM32_H_
#define _B_TZ_ARM32_H_

#define Mode_USR         0x10
#define Mode_FIQ         0x11
#define Mode_IRQ         0x12
#define Mode_SVC         0x13
#define Mode_ABT         0x17
#define Mode_SYS         0x1F
#define Mode_SVP         0x13
#define Mode_UNDEF       0x1B
#define Mode_HYP         0x1A
#define Mode_MON         0x16

/*
 * ARM CPU core registers
 */
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

#define V7AR_NUM_CORE_REGS 	43

#define V7AR_NUM_NEON64_REGS  32
#define V7AR_NUM_NEON32_REGS  64
#define ARCH_SPECIFIC_NUM_CORE_REGS V7AR_NUM_CORE_REGS
/*
 * Registers saved during TZ OS context switches
 */
#define NUM_SAVED_CPU_REGS   20
#define SAVED_REG_R0   	0
#define SAVED_REG_R1   	1
#define SAVED_REG_R2   	2
#define SAVED_REG_R3   	3
#define SAVED_REG_R4   	4
#define SAVED_REG_R5   	5
#define SAVED_REG_R6   	6
#define SAVED_REG_R7   	7
#define SAVED_REG_R8   	8
#define SAVED_REG_R9   	9
#define SAVED_REG_R10  	10
#define SAVED_REG_R11  	11
#define SAVED_REG_R12  	12
#define SAVED_REG_SPSR 	16
#define SAVED_REG_PC   	15
#define SAVED_REG_LR   	14
#define SAVED_REG_SP   	13
#define SAVED_REG_SP_USR   17
#define SAVED_REG_LR_USR   18

/* neon registers */
#define NUM_SAVED_NEON_REGS 0

/* LPAE table and block descriptors. */
#define PAGE_TABLE_SELECTION_MASK 0x80000000
#define L1_PAGE_TABLE_SLOT(x)  (uint32_t)( (uint32_t)(x) >> 30)
#define L1_PHYS_ADDR_MASK       0xC0000000

#define L1_PAGE_NUM_ENTRIES  8

#define L2_BLOCK_ADDR_MASK      0xfffff000
#define L2_BLOCK_SHIFT           21
#define L2_BLOCK_MASK            0x1ff
#define L2_PHYS_ADDR_MASK        0xFFE00000

#define L2_PAGE_NUM_ENTRIES  512

#define L2_PAGE_TABLE_SLOT(x)  (uint32_t)((uint32_t)(x) >> L2_BLOCK_SHIFT) & L2_BLOCK_MASK
#define L3_BLOCK_ADDR_MASK      0xffffff000
#define L3_BLOCK_SHIFT           12
#define L3_BLOCK_MASK            0x1ff
#define L3_PHYS_ADDR_MASK        0xFFFFF000

#define L3_PAGE_NUM_ENTRIES  512

#define L3_PAGE_TABLE_SLOT(x)  (uint32_t)((uint32_t)(x) >> L3_BLOCK_SHIFT) & L3_BLOCK_MASK

/* Macros for 2MB pages */
#define PAGE_START_2M(x)   (void*)( (uint32_t)(x)  &  0xFFE00000)
#define PAGE_SIZE_2M_BYTES        (2*1024*1024)
#define PAGE_SIZE_2M_WORDS        (512*1024)

/* Macros for 4KB pages */
#define PAGE_START_4K(x)   (void*)( (uint32_t)(x)  &  0xFFFFF000)
#define PAGE_SIZE_4K_BYTES        (4*1024)

#define PAGE_SIZE_4K_WORDS        (1024)

#define PAGE_MASK       0xFFFFF000

/*
 * Page table entry fields
 */

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

#define MAIR_DEVICE         			MAIR_STRONG_ORDERED
#define MAIR_MEMORY         			MAIR_WRITEALLOC

#define SET_MEMORY_ATTR(x,attr)		   x |= ((attr & 7) << 2)
#define GET_MEMORY_ATTR(x)			   ((attr >> 2) & 7)

// LPAE memory access permissions: ARM architecture reference manual Table B3-6 VMSAv7 AP[2:1] access permissions model
#define MEMORY_ACCESS_RW_KERNEL        0
#define MEMORY_ACCESS_RW_USER		   1
#define MEMORY_ACCESS_RO_KERNEL		   2
#define MEMORY_ACCESS_RO_USER	       3

// See ARM Architecture Reference Manual figure B3-16 Memory attribute fields in Long-descriptor stage 1 Block and Page descriptors
#define SET_MEMORY_ACCESS_PERMS(x, ap)  x |= ((ap & 3) << 6)
#define GET_MEMORY_ACCESS_PERMS(x)		((x >> 6) & 3)
#define CLEAR_MEMORY_ACCESS_PERMS(x)	x = x & (~((uint64_t)0x3 << 6))

#define SET_MEMORY_ACCESS_NO_EXEC(x)	x |= ((uint64_t)1 << 54);

#define SET_MEMORY_ACCESS_SW_BITS(x, bits)    x |= ((uint64_t)(bits & 0xf) << 55)
#define GET_MEMORY_ACCESS_SW_BITS(x)	((x >> 55) & 0xf)
#define CLEAR_MEMORY_ACCESS_SW_BITS(x)	x = x & (~((uint64_t)0xf << 55))

/* TTBCR register fields */
#define TTBCR_LPAE_ENABLE                  0x80000000
#define TTBCR_OUTER_SHAREABLE1             0x20000000
#define TTBCR_OUTER_CACHEABLE1_NORMAL      0x04000000
#define TTBCR_INNER_CACHEABLE1_NORMAL      0x01000000
#define TTBCR_OUTER_SHAREABLE0             0x00002000
#define TTBCR_OUTER_CACHEABLE0_NORMAL      0x00000400
#define TTBCR_INNER_CACHEABLE0_NORMAL      0x00000100

/* Secure configuration register (SCR) fields */
#define SCR_SECURE_INSTR_FETCH			   0x00000200
#define SCR_HYP_CALL_ENABLE				   0x00000100
#define SCR_SMC_CALL_DISABLE			   0x00000080
#define SCR_DISABLE_MEM_EARLY_TERM		   0x00000040
#define SCR_NS_ALLOW_DATA_ABORT_MASK       0x00000020
#define SCR_NS_ALLOW_FIQ_MASK       	   0x00000010
#define SCR_EXT_ABORTS_IN_MON_MODE		   0x00000008
#define SCR_FIQ_IN_MON_MODE				   0x00000004
#define SCR_IRQ_IN_MON_MODE				   0x00000002
#define SCR_NS_BIT						   0x00000001

/* Non-secure access configuration register (NSACR) fields */
#define NSACR_DISABLE_CP14_TRACE		   0x00100000
#define NSACR_RESERVE_FIQ_REGISTERS	       0x00080000
#define NSACR_DISABLE_SIMD				   0x00008000
#define NSACR_DISABLE_FP				   0x00004000

/* ARM MIDR register */
#define MIDR_PART_NUM_SHIFT 4
#define MIDR_PART_NUM_MASK  0xFF
#define MPIDR_CPUID_MASK   0x3

/* Configure the generic timer to tick at 27MHz */
#define ARM_GENERIC_TIMER_FREQUENCY  27000000

/* ARM generic counter */
#define CORTEX_A15_PHYS_TIMER_IRQ  30
#define CORTEX_A15_HYP_TIMER_IRQ   26

/* DFSR register in long description table format */
#define IS_CACHE_MAINTENANCE_FAULT(dfsr)  ((dfsr >> 13) & 1)
#define IS_EXT_DATA_ABORT(dfsr)			 ((dfsr >> 12) & 1)
#define IS_WRITE_DATA_ABORT(dfsr)		 ((dfsr >> 11) & 1)
#define IS_LPAE_IN_USE(dfsr)			 ((dfsr >> 9) & 1)
#define FAULT_STATUS(dfsr)				 ((dfsr) & 0x3f)

#define ARM_NEON_NUM_REGS64        32

#define CORTEX_A15_SCTLR_DEFAULT    0x00C50078
#define CORTEX_A15_CPACR_DEFAULT    0x00000000
#define CORTEX_A15_ACTLR_DEFAULT    0x00000000
#define CORTEX_A15_JOSCR_DEFAULT    0x00000000
#define CORTEX_A15_JMCR_DEFAULT     0x00000000
#define CORTEX_A15_VBAR_DEFAULT     0x00000000
#define CORTEX_A15_TTBCR_DEFAULT    0x00000000

#define CORTEX_A15_CACHE_LINE_SIZE   64

#define CORTEX_A15_SECURE_TIMER_INTERRUPT   29

#ifndef __ASSEMBLY__
static inline unsigned int read_mpidr(void)
{
	unsigned int mpidr;

	asm volatile ("mrc	p15, 0, %[mpidr], c0, c0, 5"
			: [mpidr] "=r" (mpidr)
	);

	return mpidr;
}

#define ARCH_SPECIFIC_CACHEFLUSH(startAddr,numBytes) { \
		const uint8_t *currLine = (const uint8_t *)startAddr; \
		const int numLines = numBytes/CORTEX_A15_CACHE_LINE_SIZE + 1; \
		for (int i=0; i<numLines; i++) { \
			asm volatile ("mcr p15, 0, %[rt], c7, c11, 1" : : [rt] "r" (currLine) : "memory"); \
			asm volatile ("mcr p15, 0, %[rt], c7, c6, 1" : : [rt] "r" (currLine) : "memory"); \
			currLine += CORTEX_A15_CACHE_LINE_SIZE; \
		} \
		asm volatile("dsb":::"memory"); \
		asm volatile("isb":::"memory"); \
		}

// printf("%s: switching to page table %p base %p (%p)\n", __FUNCTION__, this, l1Dir, pageTablePA);
/* Invalidate TLB, ICache and branch predictor */
/* Point TTBR0 to the page table */
/* Invalidate TLB, ICache and branch predictor */
#define ARCH_SPECIFIC_ACTIVATE(top) { \
		register TzMem::PhysAddr pageTablePA = TzMem::virtToPhys(top); \
		register unsigned int zero = 0; \
		asm volatile( \
				"mcr p15, 0, %[val], c8, c7, 0\r\n" \
				"mcr p15, 0, %[val], c8, c3, 0\r\n" \
				"mcr p15, 0, %[val], c7, c5, 0\r\n" \
				"mcr p15, 0, %[val], c7, c5, 6\r\n" \
				"dmb\r\n" \
				"isb\r\n" \
				: : [val] "r" (zero)); \
		register uintptr_t ttbr0Low = (uintptr_t)pageTablePA; \
		register uintptr_t ttbr0High = 0; \
		asm volatile("mcrr p15, 0, %[low], %[high], c2" : : [low] "r" (ttbr0Low), [high] "r" (ttbr0High)); \
		asm volatile("isb":::); \
		asm volatile( \
				"mcr p15, 0, %[val], c8, c7, 0\r\n" \
				"mcr p15, 0, %[val], c8, c3, 0\r\n" \
				"mcr p15, 0, %[val], c7, c5, 0\r\n" \
				"mcr p15, 0, %[val], c7, c5, 6\r\n" \
				"dmb\r\n" \
				"isb\r\n" \
				: : [val] "r" (zero)); \
		}

#define ARCH_SPECIFIC_DCCIMVAC(a) asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (a) : "memory")
#define ARCH_SPECIFIC_DCIMVAC(a) asm volatile ("mcr p15, 0, %[rt], c7, c6, 1" : : [rt] "r" (a) : "memory")
#define ARCH_SPECIFIC_DCCMVAC(a) asm volatile ("mcr p15, 0, %[rt], c7, c10, 1" : : [rt] "r" (a) : "memory")

#define ARCH_SPECIFIC_GET_TTBR0(l,h) asm volatile("mrrc p15, 0, %[low], %[high], c2" : [low] "=r" (l), [high] "=r" (h) : :)
#define ARCH_SPECIFIC_GET_TTBR1(l,h) asm volatile("mrrc p15, 1, %[low], %[high], c2" : [low] "=r" (l), [high] "=r" (h) : :)

#define ARCH_SPECIFIC_TLB_FLUSH \
	asm volatile("dsb\r\n" \
	                 "mov r4, #0\r\n" \
	                 "mcr p15, 0, r4, c8, c7, 0\r\n" \
	                 "mcr p15, 0, r4, c7, c5, 6\r\n" \
	                 "dsb\r\n" \
	                 "isb\r\n" \
	                 : : : "r4")

#define ARCH_SPECIFIC_DMB \
	asm volatile("dmb":::"memory")


#define ARCH_SPECIFIC_MEMORY_BARRIER \
	asm volatile("dmb":::"memory"); \
	asm volatile("isb":::"memory")

#define ARCH_SPECIFIC_GET_GIC_BASEADDR(a)	asm volatile("mrc p15, 4, %[rt], c15, c0, 0" : [rt] "=r" (a) : :)

#define ARCH_SPECIFIC_TIMER_INTERRUPT CORTEX_A15_SECURE_TIMER_INTERRUPT

#define ARCH_SPECIFIC_GET_SECURE_TIMER_CURRENT_TIME(timeNow) { \
			register uint32_t timeHigh, timeLow; \
			asm volatile("MRRC p15, 0, %[low], %[high], c14" : [low] "=r" (timeLow), [high] "=r" (timeHigh) : : ); \
			timeNow = ((uint64_t) timeHigh << 32) | timeLow; \
}

#define ARCH_SPECIFIC_GET_SECURE_TIMER_FREQUENCY(rv) asm volatile("MRC p15, 0, %[rt], c14, c0, 0": [rt] "=r" (rv) : :)

#define ARCH_SPECIFIC_NSWTASK { \
	asm volatile("cpsie af":::); \
	while (true) { \
		asm volatile("smc #0": : :); \
	} \
	}

#define ARCH_SPECIFIC_ENABLE_INTERRUPTS asm volatile("cpsie aif":::)

#define ARCH_SPECIFIC_GET_SPSR(spsr) asm volatile("mrs %[rt], cpsr" : [rt] "=r" (spsr) : :)

#define ARCH_SPECIFIC_SAVE_STATE(neonReg, cpuReg) \
	asm volatile ( \
			"cpsid if\r\n" \
			"mov r0, %[rt0]\r\n" \
			"mov r0, %[rt1]\r\n" \
			"str lr, [r0]\r\n" \
			"str r1, [r0, #4]\r\n" \
			"str r2, [r0, #8]\r\n" \
			"str r3, [r0, #12]\r\n" \
			"str r4, [r0, #16]\r\n" \
			"str r5, [r0, #20]\r\n" \
			"str r6, [r0, #24]\r\n" \
			"str r7, [r0, #28]\r\n" \
			"str r8, [r0, #32]\r\n" \
			"str r9, [r0, #36]\r\n" \
			"str r10, [r0, #40]\r\n" \
			"str r11, [r0, #44]\r\n" \
			"str r12, [r0, #48]\r\n" \
			"str sp,  [r0, #52]\r\n" \
			"clrex\r\n" \
			"adr r1, resumption\r\n" \
			"str r1, [r0, #56]\r\n" \
			"mrs r1, cpsr \r\n" \
			"str r1, [r0, #64]\r\n" \
			"b schedule\r\n" \
			"resumption:\r\n" \
			"mov lr, r0\r\n" \
			: :[rt0] "r" (neonReg), [rt1] "r" (cpuReg) :)

#define ARCH_SPECIFIC_DATA_ABORT_EXCEPTION(dfsr, dfar, align) \
	asm volatile("MRC p15, 0, %[rt], c5, c0, 0": [rt] "=r" (dfsr)::); \
	asm volatile("MRC p15, 0, %[rt], c6, c0, 0":[rt] "=r" (dfar)::); \
			asm volatile("MRC p15, 0, %[rt], c1, c0, 0":[rt] "=r" (align)::)

#define ARCH_SPECIFIC_SET_TPIDRURO(ti) asm volatile("MCR p15, 0, %[rt], c13, c0, 3"::[rt] "r" (ti):)

#define ARCH_SPECIFIC_GET_DFAR(dfar) asm volatile("MRC p15, 0, %[rt], c6, c0, 0":[rt] "=r" (dfar)::)

#define ARCH_SPECIFIC_DISABLE_INTERRUPTS asm volatile("cpsid if":::)

#define ARCH_SPECIFIC_ENABLE_USER_PERF_MON asm volatile("mcr p15, 0, %[val], c9, c14, 0" : :[val] "r"(0x1))

#define ARCH_SPECIFIC_CPU_ID(id) { \
			asm volatile("mrc p15, 0, %[result], c0, c0, 5" : [result] "=r" (id) : :); \
			id &= MPIDR_CPUID_MASK; \
}

#define ARCH_SPECIFIC_SMP_CPU_NUM(num) { \
			asm volatile("mrc p15, 0, %[result], c0, c0, 5" : [result] "=r" (num) : :); \
			num &= MPIDR_CPUID_MASK; \
}

#define ARCH_SPECIFIC_SYSCALL_NUM_REGISTER TzTask::UserRegs::r7

#define ARCH_SPECIFIC_GET_CTR(ctr) asm volatile ("mrc p15, 0, %0, c0, c0, 1" : "=r" (ctr))

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
	r12 = SAVED_REG_R12, \
	spsr = SAVED_REG_SPSR, \
	pc = SAVED_REG_PC, \
	lr = SAVED_REG_LR, \
	sp = SAVED_REG_SP, \
	sp_usr = SAVED_REG_SP_USR, \
	lr_usr = SAVED_REG_LR_USR }

#define ARCH_SPECIFIC_CHUNK_MASK (0xFFFFC000L)

/*
 * HWCAP flags - AT_HWCAP auxialary
 */

#define HWCAP_SWP       (1 << 0)
#define HWCAP_HALF      (1 << 1)
#define HWCAP_THUMB     (1 << 2)
#define HWCAP_26BIT     (1 << 3)
#define HWCAP_FAST_MULT (1 << 4)
#define HWCAP_FPA       (1 << 5)
#define HWCAP_VFP       (1 << 6)
#define HWCAP_EDSP      (1 << 7)
#define HWCAP_JAVA      (1 << 8)
#define HWCAP_IWMMXT    (1 << 9)
#define HWCAP_CRUNCH    (1 << 10)
#define HWCAP_THUMBEE   (1 << 11)
#define HWCAP_NEON      (1 << 12)
#define HWCAP_VFPv3     (1 << 13)
#define HWCAP_VFPv3D16  (1 << 14)
#define HWCAP_TLS       (1 << 15)
#define HWCAP_VFPv4     (1 << 16)
#define HWCAP_IDIVA     (1 << 17)
#define HWCAP_IDIVT     (1 << 18)
#define HWCAP_VFPD32    (1 << 19)
#define HWCAP_IDIV      (HWCAP_IDIVA | HWCAP_IDIVT)
#define HWCAP_LPAE      (1 << 20)
#define HWCAP_EVTSTRM   (1 << 21)

/*
 * HWCAP2 flags - AT_HWCAP2 auxilaray
 */
#define HWCAP2_AES      (1 << 0)
#define HWCAP2_PMULL    (1 << 1)
#define HWCAP2_SHA1     (1 << 2)
#define HWCAP2_SHA2     (1 << 3)
#define HWCAP2_CRC32    (1 << 4)

#define HWCAPS_AUXVAL   (HWCAP_SWP | HWCAP_VFP | HWCAP_FAST_MULT | HWCAP_TLS)

#endif /* __ASSEMBLY__ */

#endif
