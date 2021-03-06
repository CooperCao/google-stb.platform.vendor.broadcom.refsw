/****************************************************************************
 *     Copyright (c) 1999-2015, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Tue May 12 15:31:56 2015
 *                 Full Compile MD5 Checksum  654f5b1025c3f32e1ac79a0158cb9296
 *                     (minus title and desc)
 *                 MD5 Checksum               f5dfcaebcf2741b1de57e2e58f246be2
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     16053
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
 ***************************************************************************/

#ifndef BCHP_AVS_CPU_AUX_REGS_H__
#define BCHP_AVS_CPU_AUX_REGS_H__

/***************************************************************************
 *AVS_CPU_AUX_REGS - CPU Auxiliary Registers
 ***************************************************************************/
#define BCHP_AVS_CPU_AUX_REGS_STATUS             0x0410a000 /* [RO] Auxiliary Register STATUS */
#define BCHP_AVS_CPU_AUX_REGS_SEMAPHORE          0x0410a004 /* [RW] Inter-process/Host semaphore register */
#define BCHP_AVS_CPU_AUX_REGS_LP_START           0x0410a008 /* [RW] Loop start address (32-bit) */
#define BCHP_AVS_CPU_AUX_REGS_LP_END             0x0410a00c /* [RW] Loop end address (32-bit) */
#define BCHP_AVS_CPU_AUX_REGS_IDENTITY           0x0410a010 /* [RO] Processor Identification register */
#define BCHP_AVS_CPU_AUX_REGS_DEBUG_AVS          0x0410a014 /* [RW] Debug register */
#define BCHP_AVS_CPU_AUX_REGS_PC                 0x0410a018 /* [RO] Program Counter register (32-bit) */
#define BCHP_AVS_CPU_AUX_REGS_STATUS32           0x0410a028 /* [RO] Status register (32-bit) */
#define BCHP_AVS_CPU_AUX_REGS_STATUS32_L1        0x0410a02c /* [RW] Status register save for level 1 interrupts */
#define BCHP_AVS_CPU_AUX_REGS_STATUS32_L2        0x0410a030 /* [RW] Status register save for level 2 interrupts */
#define BCHP_AVS_CPU_AUX_REGS_AUX_DCCM           0x0410a060 /* [RW] Address of Local Data RAM */
#define BCHP_AVS_CPU_AUX_REGS_TIMER0_COUNT       0x0410a084 /* [RW] Processor Timer0 Count value */
#define BCHP_AVS_CPU_AUX_REGS_TIMER0_CONTROL     0x0410a088 /* [RW] Processor Timer0 Control value */
#define BCHP_AVS_CPU_AUX_REGS_TIMER0_LIMIT       0x0410a08c /* [RW] Processor Timer0 Limit value */
#define BCHP_AVS_CPU_AUX_REGS_INT_VEC_BASE       0x0410a094 /* [RW] Interrupt Vector Base address */
#define BCHP_AVS_CPU_AUX_REGS_AUX_IRQ_LV12       0x0410a10c /* [RW] Interrupt Level Status */
#define BCHP_AVS_CPU_AUX_REGS_CRC_BUILD_BCR      0x0410a188 /* [RO] Build configuration register for CRC instruction. */
#define BCHP_AVS_CPU_AUX_REGS_DVBF_BUILD         0x0410a190 /* [RO] Build configuration register for dual viterbi butterfly instruction. */
#define BCHP_AVS_CPU_AUX_REGS_EXT_ARITH_BUILD    0x0410a194 /* [RO] Build configuration register to specify that the processor has the extended arithmetic instructions. */
#define BCHP_AVS_CPU_AUX_REGS_DATASPACE          0x0410a198 /* [RO] Build configuration register for dataspace. */
#define BCHP_AVS_CPU_AUX_REGS_MEMSUBSYS          0x0410a19c /* [RO] Build configuration register for memory subsytem. */
#define BCHP_AVS_CPU_AUX_REGS_VECBASE_AC_BUILD   0x0410a1a0 /* [RO] Build configuration register for ARC600 interrupt vector base address. */
#define BCHP_AVS_CPU_AUX_REGS_P_BASE_ADDR        0x0410a1a4 /* [RO] Build configuration register for peripheral base address. */
#define BCHP_AVS_CPU_AUX_REGS_MPU_BUILD          0x0410a1b4 /* [RO] Build configuration register for memory protection unit. */
#define BCHP_AVS_CPU_AUX_REGS_RF_BUILD           0x0410a1b8 /* [RO] Build configuration register for register file. */
#define BCHP_AVS_CPU_AUX_REGS_D_CACHE_BUILD      0x0410a1c8 /* [RO] Build configuration register for data cache. */
#define BCHP_AVS_CPU_AUX_REGS_MADI_BUILD         0x0410a1cc /* [RO] Build configuration register for multiple ARC debug interface. */
#define BCHP_AVS_CPU_AUX_REGS_DCCM_BUILD         0x0410a1d0 /* [RO] Build configuration register for data closely coupled memory. */
#define BCHP_AVS_CPU_AUX_REGS_TIMER_BUILD        0x0410a1d4 /* [RO] Build configuration register for timers. */
#define BCHP_AVS_CPU_AUX_REGS_AP_BUILD           0x0410a1d8 /* [RO] Build configuration register for actionpoints. */
#define BCHP_AVS_CPU_AUX_REGS_I_CACHE_BUILD      0x0410a1dc /* [RO] Build configuration register for instruction cache. */
#define BCHP_AVS_CPU_AUX_REGS_ICCM_BUILD         0x0410a1e0 /* [RO] Build configuration register for instruction closely coupled memory. */
#define BCHP_AVS_CPU_AUX_REGS_DSPRAM_BUILD       0x0410a1e4 /* [RO] Build configuration register for XY memory. */
#define BCHP_AVS_CPU_AUX_REGS_MAC_BUILD          0x0410a1e8 /* [RO] Build configuration register for Xmac. */
#define BCHP_AVS_CPU_AUX_REGS_MULTIPLY_BUILD     0x0410a1ec /* [RO] Build configuration register for instruction closely coupled memory. */
#define BCHP_AVS_CPU_AUX_REGS_SWAP_BUILD         0x0410a1f0 /* [RO] Build configuration register for swap instruction. */
#define BCHP_AVS_CPU_AUX_REGS_NORM_BUILD         0x0410a1f4 /* [RO] Build configuration register for normalise instruction. */
#define BCHP_AVS_CPU_AUX_REGS_MINMAX_BUILD       0x0410a1f8 /* [RO] Build configuration register for min/max instruction. */
#define BCHP_AVS_CPU_AUX_REGS_BARREL_BUILD       0x0410a1fc /* [RO] Build configuration register for barrel shifter. */
#define BCHP_AVS_CPU_AUX_REGS_ARC600_BUILD       0x0410a304 /* [RO] Build configuration register for ARC 600. */
#define BCHP_AVS_CPU_AUX_REGS_AUX_SYSTEM_BUILD   0x0410a308 /* [RW] Build configuration register for AS221BD. */
#define BCHP_AVS_CPU_AUX_REGS_MCD_BCR            0x0410a310 /* [RO] MCD configuration register for AS221BD. */
#define BCHP_AVS_CPU_AUX_REGS_IFETCHQUEUE_BUILD  0x0410a3f8 /* [RO] Build configuration register for the InstructionFetchQueue component. */
#define BCHP_AVS_CPU_AUX_REGS_TIMER1_COUNT       0x0410a400 /* [RW] Processor Timer 1 Count value */
#define BCHP_AVS_CPU_AUX_REGS_TIMER1_CONTROL     0x0410a404 /* [RW] Processor Timer 1 Control value */
#define BCHP_AVS_CPU_AUX_REGS_TIMER1_LIMIT       0x0410a408 /* [RW] Processor Timer 1 Limit value */
#define BCHP_AVS_CPU_AUX_REGS_AUX_IRQ_LEV        0x0410a800 /* [RW] Interrupt Level Programming */
#define BCHP_AVS_CPU_AUX_REGS_AUX_IRQ_HINT       0x0410a804 /* [RW] Software Triggered Interrupt */
#define BCHP_AVS_CPU_AUX_REGS_AUX_ALIGN_CTRL     0x0410a808 /* [RW] Memory Alignment Detection Control */
#define BCHP_AVS_CPU_AUX_REGS_AUX_INTER_CORE_INTERRUPT 0x0410aa00 /* [RO] Inter-core Interrupt Register */
#define BCHP_AVS_CPU_AUX_REGS_AX_IPC_SEM_N       0x0410aa04 /* [RO] Inter-core Sempahore Register */
#define BCHP_AVS_CPU_AUX_REGS_AUX_INTER_CORE_ACK 0x0410aa08 /* [RO] Inter-core Interrupt Acknowledge Register */

/***************************************************************************
 *STATUS - Auxiliary Register STATUS
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: STATUS :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_STATUS_WORD_MASK                     0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_STATUS_WORD_SHIFT                    0

/***************************************************************************
 *SEMAPHORE - Inter-process/Host semaphore register
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: SEMAPHORE :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_SEMAPHORE_WORD_MASK                  0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_SEMAPHORE_WORD_SHIFT                 0

/***************************************************************************
 *LP_START - Loop start address (32-bit)
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: LP_START :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_LP_START_WORD_MASK                   0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_LP_START_WORD_SHIFT                  0

/***************************************************************************
 *LP_END - Loop end address (32-bit)
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: LP_END :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_LP_END_WORD_MASK                     0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_LP_END_WORD_SHIFT                    0

/***************************************************************************
 *IDENTITY - Processor Identification register
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: IDENTITY :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_IDENTITY_WORD_MASK                   0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_IDENTITY_WORD_SHIFT                  0

/***************************************************************************
 *DEBUG_AVS - Debug register
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: DEBUG_AVS :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_DEBUG_AVS_WORD_MASK                  0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_DEBUG_AVS_WORD_SHIFT                 0

/***************************************************************************
 *PC - Program Counter register (32-bit)
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: PC :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_PC_WORD_MASK                         0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_PC_WORD_SHIFT                        0

/***************************************************************************
 *STATUS32 - Status register (32-bit)
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: STATUS32 :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_STATUS32_WORD_MASK                   0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_STATUS32_WORD_SHIFT                  0

/***************************************************************************
 *STATUS32_L1 - Status register save for level 1 interrupts
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: STATUS32_L1 :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_STATUS32_L1_WORD_MASK                0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_STATUS32_L1_WORD_SHIFT               0

/***************************************************************************
 *STATUS32_L2 - Status register save for level 2 interrupts
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: STATUS32_L2 :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_STATUS32_L2_WORD_MASK                0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_STATUS32_L2_WORD_SHIFT               0

/***************************************************************************
 *AUX_DCCM - Address of Local Data RAM
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: AUX_DCCM :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_AUX_DCCM_WORD_MASK                   0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_AUX_DCCM_WORD_SHIFT                  0

/***************************************************************************
 *TIMER0_COUNT - Processor Timer0 Count value
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: TIMER0_COUNT :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_TIMER0_COUNT_WORD_MASK               0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_TIMER0_COUNT_WORD_SHIFT              0

/***************************************************************************
 *TIMER0_CONTROL - Processor Timer0 Control value
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: TIMER0_CONTROL :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_TIMER0_CONTROL_WORD_MASK             0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_TIMER0_CONTROL_WORD_SHIFT            0

/***************************************************************************
 *TIMER0_LIMIT - Processor Timer0 Limit value
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: TIMER0_LIMIT :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_TIMER0_LIMIT_WORD_MASK               0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_TIMER0_LIMIT_WORD_SHIFT              0

/***************************************************************************
 *INT_VEC_BASE - Interrupt Vector Base address
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: INT_VEC_BASE :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_INT_VEC_BASE_WORD_MASK               0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_INT_VEC_BASE_WORD_SHIFT              0

/***************************************************************************
 *AUX_IRQ_LV12 - Interrupt Level Status
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: AUX_IRQ_LV12 :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_AUX_IRQ_LV12_WORD_MASK               0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_AUX_IRQ_LV12_WORD_SHIFT              0

/***************************************************************************
 *CRC_BUILD_BCR - Build configuration register for CRC instruction.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: CRC_BUILD_BCR :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_CRC_BUILD_BCR_WORD_MASK              0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_CRC_BUILD_BCR_WORD_SHIFT             0

/***************************************************************************
 *DVBF_BUILD - Build configuration register for dual viterbi butterfly instruction.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: DVBF_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_DVBF_BUILD_WORD_MASK                 0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_DVBF_BUILD_WORD_SHIFT                0

/***************************************************************************
 *EXT_ARITH_BUILD - Build configuration register to specify that the processor has the extended arithmetic instructions.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: EXT_ARITH_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_EXT_ARITH_BUILD_WORD_MASK            0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_EXT_ARITH_BUILD_WORD_SHIFT           0

/***************************************************************************
 *DATASPACE - Build configuration register for dataspace.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: DATASPACE :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_DATASPACE_WORD_MASK                  0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_DATASPACE_WORD_SHIFT                 0

/***************************************************************************
 *MEMSUBSYS - Build configuration register for memory subsytem.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: MEMSUBSYS :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_MEMSUBSYS_WORD_MASK                  0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_MEMSUBSYS_WORD_SHIFT                 0

/***************************************************************************
 *VECBASE_AC_BUILD - Build configuration register for ARC600 interrupt vector base address.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: VECBASE_AC_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_VECBASE_AC_BUILD_WORD_MASK           0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_VECBASE_AC_BUILD_WORD_SHIFT          0

/***************************************************************************
 *P_BASE_ADDR - Build configuration register for peripheral base address.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: P_BASE_ADDR :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_P_BASE_ADDR_WORD_MASK                0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_P_BASE_ADDR_WORD_SHIFT               0

/***************************************************************************
 *MPU_BUILD - Build configuration register for memory protection unit.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: MPU_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_MPU_BUILD_WORD_MASK                  0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_MPU_BUILD_WORD_SHIFT                 0

/***************************************************************************
 *RF_BUILD - Build configuration register for register file.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: RF_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_RF_BUILD_WORD_MASK                   0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_RF_BUILD_WORD_SHIFT                  0

/***************************************************************************
 *D_CACHE_BUILD - Build configuration register for data cache.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: D_CACHE_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_D_CACHE_BUILD_WORD_MASK              0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_D_CACHE_BUILD_WORD_SHIFT             0

/***************************************************************************
 *MADI_BUILD - Build configuration register for multiple ARC debug interface.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: MADI_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_MADI_BUILD_WORD_MASK                 0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_MADI_BUILD_WORD_SHIFT                0

/***************************************************************************
 *DCCM_BUILD - Build configuration register for data closely coupled memory.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: DCCM_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_DCCM_BUILD_WORD_MASK                 0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_DCCM_BUILD_WORD_SHIFT                0

/***************************************************************************
 *TIMER_BUILD - Build configuration register for timers.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: TIMER_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_TIMER_BUILD_WORD_MASK                0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_TIMER_BUILD_WORD_SHIFT               0

/***************************************************************************
 *AP_BUILD - Build configuration register for actionpoints.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: AP_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_AP_BUILD_WORD_MASK                   0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_AP_BUILD_WORD_SHIFT                  0

/***************************************************************************
 *I_CACHE_BUILD - Build configuration register for instruction cache.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: I_CACHE_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_I_CACHE_BUILD_WORD_MASK              0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_I_CACHE_BUILD_WORD_SHIFT             0

/***************************************************************************
 *ICCM_BUILD - Build configuration register for instruction closely coupled memory.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: ICCM_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_ICCM_BUILD_WORD_MASK                 0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_ICCM_BUILD_WORD_SHIFT                0

/***************************************************************************
 *DSPRAM_BUILD - Build configuration register for XY memory.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: DSPRAM_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_DSPRAM_BUILD_WORD_MASK               0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_DSPRAM_BUILD_WORD_SHIFT              0

/***************************************************************************
 *MAC_BUILD - Build configuration register for Xmac.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: MAC_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_MAC_BUILD_WORD_MASK                  0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_MAC_BUILD_WORD_SHIFT                 0

/***************************************************************************
 *MULTIPLY_BUILD - Build configuration register for instruction closely coupled memory.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: MULTIPLY_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_MULTIPLY_BUILD_WORD_MASK             0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_MULTIPLY_BUILD_WORD_SHIFT            0

/***************************************************************************
 *SWAP_BUILD - Build configuration register for swap instruction.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: SWAP_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_SWAP_BUILD_WORD_MASK                 0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_SWAP_BUILD_WORD_SHIFT                0

/***************************************************************************
 *NORM_BUILD - Build configuration register for normalise instruction.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: NORM_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_NORM_BUILD_WORD_MASK                 0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_NORM_BUILD_WORD_SHIFT                0

/***************************************************************************
 *MINMAX_BUILD - Build configuration register for min/max instruction.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: MINMAX_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_MINMAX_BUILD_WORD_MASK               0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_MINMAX_BUILD_WORD_SHIFT              0

/***************************************************************************
 *BARREL_BUILD - Build configuration register for barrel shifter.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: BARREL_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_BARREL_BUILD_WORD_MASK               0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_BARREL_BUILD_WORD_SHIFT              0

/***************************************************************************
 *ARC600_BUILD - Build configuration register for ARC 600.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: ARC600_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_ARC600_BUILD_WORD_MASK               0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_ARC600_BUILD_WORD_SHIFT              0

/***************************************************************************
 *AUX_SYSTEM_BUILD - Build configuration register for AS221BD.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: AUX_SYSTEM_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_AUX_SYSTEM_BUILD_WORD_MASK           0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_AUX_SYSTEM_BUILD_WORD_SHIFT          0

/***************************************************************************
 *MCD_BCR - MCD configuration register for AS221BD.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: MCD_BCR :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_MCD_BCR_WORD_MASK                    0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_MCD_BCR_WORD_SHIFT                   0

/***************************************************************************
 *IFETCHQUEUE_BUILD - Build configuration register for the InstructionFetchQueue component.
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: IFETCHQUEUE_BUILD :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_IFETCHQUEUE_BUILD_WORD_MASK          0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_IFETCHQUEUE_BUILD_WORD_SHIFT         0

/***************************************************************************
 *TIMER1_COUNT - Processor Timer 1 Count value
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: TIMER1_COUNT :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_TIMER1_COUNT_WORD_MASK               0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_TIMER1_COUNT_WORD_SHIFT              0

/***************************************************************************
 *TIMER1_CONTROL - Processor Timer 1 Control value
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: TIMER1_CONTROL :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_TIMER1_CONTROL_WORD_MASK             0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_TIMER1_CONTROL_WORD_SHIFT            0

/***************************************************************************
 *TIMER1_LIMIT - Processor Timer 1 Limit value
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: TIMER1_LIMIT :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_TIMER1_LIMIT_WORD_MASK               0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_TIMER1_LIMIT_WORD_SHIFT              0

/***************************************************************************
 *AUX_IRQ_LEV - Interrupt Level Programming
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: AUX_IRQ_LEV :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_AUX_IRQ_LEV_WORD_MASK                0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_AUX_IRQ_LEV_WORD_SHIFT               0

/***************************************************************************
 *AUX_IRQ_HINT - Software Triggered Interrupt
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: AUX_IRQ_HINT :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_AUX_IRQ_HINT_WORD_MASK               0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_AUX_IRQ_HINT_WORD_SHIFT              0

/***************************************************************************
 *AUX_ALIGN_CTRL - Memory Alignment Detection Control
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: AUX_ALIGN_CTRL :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_AUX_ALIGN_CTRL_WORD_MASK             0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_AUX_ALIGN_CTRL_WORD_SHIFT            0

/***************************************************************************
 *AUX_INTER_CORE_INTERRUPT - Inter-core Interrupt Register
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: AUX_INTER_CORE_INTERRUPT :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_AUX_INTER_CORE_INTERRUPT_WORD_MASK   0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_AUX_INTER_CORE_INTERRUPT_WORD_SHIFT  0

/***************************************************************************
 *AX_IPC_SEM_N - Inter-core Sempahore Register
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: AX_IPC_SEM_N :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_AX_IPC_SEM_N_WORD_MASK               0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_AX_IPC_SEM_N_WORD_SHIFT              0

/***************************************************************************
 *AUX_INTER_CORE_ACK - Inter-core Interrupt Acknowledge Register
 ***************************************************************************/
/* AVS_CPU_AUX_REGS :: AUX_INTER_CORE_ACK :: WORD [31:00] */
#define BCHP_AVS_CPU_AUX_REGS_AUX_INTER_CORE_ACK_WORD_MASK         0xffffffff
#define BCHP_AVS_CPU_AUX_REGS_AUX_INTER_CORE_ACK_WORD_SHIFT        0

#endif /* #ifndef BCHP_AVS_CPU_AUX_REGS_H__ */

/* End of File */
