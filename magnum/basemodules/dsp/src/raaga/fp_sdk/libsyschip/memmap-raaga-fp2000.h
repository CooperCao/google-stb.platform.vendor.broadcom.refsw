/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

/*
 * This file defines structures describing the layout of hardware
 * register groups, buffers etc for raaga.  It is complemented by
 * the file hwmap-raaga.ld, which defines symbols for key location
 * in the raaga address space.
 *
 * The ASMCPP macro is used to protect C structure declarations, so
 * this file can be #included from assembler.
 *
 * It is laid out in ascending memory order.
 */

#ifndef _MEMMAP_RAAGA_FP2000_H_
#define _MEMMAP_RAAGA_FP2000_H_

#if ! (defined (__FP2008__) || defined (__FP2011__) || \
       defined (__FP2012__) || defined (__COMPILE_HEADER__))
# error "Included memmap-raaga-fp2000.h on unsupported machine architecture"
#endif



/*
 * SMEM & DMEM.
 */
#define SMEM_START       0
#define DMEM_START       0x40000000

#if defined(__FIREPATH__) && !defined(ASMCPP) && !defined(__LINKER_SCRIPT__)
#include <stdint.h>
extern uint32_t tsize_dmem;
extern uint32_t tsize_smem;
#endif

#if !defined(__LINKER_SCRIPT__)
#  define SMEM_SIZE     SYMBOL_ADDR_TO_ADDR (tsize_smem)
#  define SMEM_END      (SMEM_START + SMEM_SIZE)

#  define DMEM_SIZE     SYMBOL_ADDR_TO_ADDR (tsize_dmem)
#  define DMEM_END      (DMEM_START + DMEM_SIZE)
#endif


/*
 *  Shared registers
 *
 *   RAAGA_DSP: Raaga DSP Block                                       0x10a20000 - 0x10a3ffff
 *     RAAGA_DSP_MISC: MISC Regs                                        0x10a20000 - 0x10a2040f
 *     RAAGA_DSP_PERI: Peripheral Regs                                  0x10a21000 - 0x10a213ff
 *       RAAGA_DSP_TIMERS: Timers Control Regs                            0x10a21000 - 0x10a2105b
 *       RAAGA_DSP_PERI_DBG_CTRL: Debug Control Regs                      0x10a21080 - 0x10a2109f
 *       RAAGA_DSP_PERI_SW: SW Control Regs                               0x10a21100 - 0x10a21157
 *       RAAGA_DSP_DMA: Raaga DMA Regs                                    0x10a21200 - 0x10a212ab
 *     RAAGA_DSP_INT: Interrupts                                        0x10a22000 - 0x10a22fff
 *       RAAGA_DSP_ESR_SI: Interrupts Status Reg (Soft Interrupts to DSP) 0x10a22000 - 0x10a22017
 *       RAAGA_DSP_INTH: HW Interrupts to External Hosts Regs             0x10a22200 - 0x10a2222f
 *       RAAGA_DSP_FW_INTH: FW Interrupts to External Hosts Regs          0x10a22400 - 0x10a2242f
 *     RAAGA_DSP_FW_CFG: FW Configuration Regs                          0x10a23000 - 0x10a2357f
 *     RAAGA_DSP_MEM_SUBSYSTEM: Memory Subsystem                        0x10a30000 - 0x10a3bfff
 */

#ifndef ASMCPP

/** Raaga Top is the Base Address of the Raaga System Registers */
#define RAAGA_TOP           0x80000000

/** VOM page table base address */
#define VOM_PAGE_TABLE_BASE (RAAGA_TOP + 0x38000)


#ifndef DOXYGEN

typedef struct {
  volatile uint32_t HOST_STATUS;      /* = 0xXXX22200, Host Interrupt Status Register */
  volatile uint32_t HOST_SET;         /* = 0xXXX22204, Host Interrupt Set Register */
  volatile uint32_t HOST_CLEAR;       /* = 0xXXX22208, Host Interrupt Clear Register */
  volatile uint32_t HOST_MASK_STATUS; /* = 0xXXX2220c, Host Interrupt Mask Status Register */
  volatile uint32_t HOST_MASK_SET;    /* = 0xXXX22210, Host Interrupt Mask Set Register */
  volatile uint32_t HOST_MASK_CLEAR;  /* = 0xXXX22214, Host Interrupt Mask Clear Register */
  volatile uint32_t PCI_STATUS;       /* = 0xXXX22218, PCI Interrupt Status Register */
  volatile uint32_t PCI_SET;          /* = 0xXXX2221c, PCI Interrupt Set Register */
  volatile uint32_t PCI_CLEAR;        /* = 0xXXX22220, PCI Interrupt Clear Register */
  volatile uint32_t PCI_MASK_STATUS;  /* = 0xXXX22224, PCI Interrupt Mask Status Register */
  volatile uint32_t PCI_MASK_SET;     /* = 0xXXX22228, PCI Interrupt Mask Set Register */
  volatile uint32_t PCI_MASK_CLEAR;   /* = 0xXXX2222c, PCI Interrupt Mask Clear Register */
} RaagaRegs_IntH;

#define taddr_RaagaRegs_IntH        ((RaagaRegs_IntH *)(RAAGA_TOP + 0x22200))
#define taddr_RaagaRegs_IntH_ASM    (RAAGA_TOP + 0x22200)

typedef struct {
  volatile uint32_t ID2R_RDATA         ; /* = 0xXXX30000, Peek and Poke IMEM/DMEM RBUS Read Data */
  volatile uint32_t R2ID_WDATA         ; /* = 0xXXX30004, Peek and Poke IMEM/DMEM RBUS Write Data */
  volatile uint32_t R2ID_CMD           ; /* = 0xXXX30008, Peek and Poke Command Register  for IMEM and DMEM */
  volatile uint32_t R2ID_ADDR          ; /* = 0xXXX3000c, IMEM or DMEM Address for peek/poke access */
  volatile uint32_t FP_STALL_ALLOW     ; /* = 0xXXX30010, FP can stall IMEM for this many clock cycles */
  volatile uint32_t DMA_ALLOW          ; /* = 0xXXX30014, DMA priority is raised over FP for this many clock cycles */
  volatile uint32_t IMEM_STATUS        ; /* = 0xXXX30018, IMEM STATUS Register */
  volatile uint32_t DMEM_STATUS        ; /* = 0xXXX3001c, DMEM STATUS Register */
  volatile uint32_t VOM_MISS_STATUS    ; /* = 0xXXX30020, VOM MISS STATUS Register */
  volatile uint32_t MRU_CNTRL          ; /* = 0xXXX30024, MRU Control Register */
  volatile uint32_t MRU_PAGE_0         ; /* = 0xXXX30028, MRU_Page_0 */
  volatile uint32_t MRU_PAGE_1         ; /* = 0xXXX3002c, MRU_Page_1 */
  volatile uint32_t MRU_PAGE_2         ; /* = 0xXXX30030, MRU_Page_2 */
  volatile uint32_t MRU_PAGE_3         ; /* = 0xXXX30034, MRU_Page_3 */
  volatile uint32_t MRU_PAGE_4         ; /* = 0xXXX30038, MRU_Page_4 */
  volatile uint32_t MRU_PAGE_5         ; /* = 0xXXX3003c, MRU_Page_5 */
  volatile uint32_t MRU_PAGE_6         ; /* = 0xXXX30040, MRU_Page_6 */
  volatile uint32_t MRU_PAGE_7         ; /* = 0xXXX30044, MRU_Page_7 */
  volatile uint32_t PC_STATUS          ; /* = 0xXXX30048, FP_PC_STATUS */
  volatile uint32_t INST_OUT_LOWER     ; /* = 0xXXX3004c, INST_OUT_LOWER_HALF */
  volatile uint32_t INST_OUT_UPPER     ; /* = 0xXXX30050, INST_OUT_UPPER_HALF */
  volatile uint32_t INST_OUT_READY     ; /* = 0xXXX30054, INST_OUT_READY */
  volatile uint32_t MEMSUB_ERROR_STATUS; /* = 0xXXX30058, MEMSUB_ERROR_STATUS */
  volatile uint32_t MEMSUB_ERROR_CLEAR ; /* = 0xXXX3005c, MEMSUB_ERROR_CLEAR */
} RaagaRegs_MemSubsystem;

#define taddr_RaagaRegs_MemSubsystem        ((RaagaRegs_MemSubsystem *)(RAAGA_TOP + 0x30000))
#define taddr_RaagaRegs_MemSubsystem_ASM    (RAAGA_TOP + 0x30000)

#endif /*DOXYGEN*/

#endif /*ASMCPP*/


/* IMEM-DMEM peek/poke mechanism defines. */
#define MEM_SUBSYS_MEMSEL_DMEM          0
#define MEM_SUBSYS_READ_BIT             4
#define MEM_SUBSYS_WRITE_BIT            5
#define MEM_SUBSYS_IRQ_PEEK_POKE_BIT    25

#if !defined(__FP2012_ONWARDS__)        /* for FP2012+, memmap.h deals with FIREPATH_NUM definition */
#  define FIREPATH_NUM              0
#  define FIREPATH_NUM_UNCACHED     0
#endif


#endif /* _MEMMAP_RAAGA_FP2000_H_ */
