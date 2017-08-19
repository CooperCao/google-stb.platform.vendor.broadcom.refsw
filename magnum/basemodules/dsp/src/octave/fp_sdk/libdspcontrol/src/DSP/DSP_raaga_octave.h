/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/

#ifndef _DSP_RAAGA_OCTAVE_H_
#define _DSP_RAAGA_OCTAVE_H_

#include <stdint.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DSP.h"

#include "libfp/src/c_utils_internal.h"

#if IS_TARGET(RaagaFP4015_haps) || IS_TARGET(RaagaFP4015_haps_bm)
#  include "DSP_raaga_octave_haps.h"
#elif IS_TARGET(RaagaFP4015_bm)
#  include "DSP_raaga_octave_bm.h"
#elif IS_TARGET(RaagaFP4015_si_magnum) || IS_TARGET(RaagaFP4015_si_rockford) || IS_TARGET(RaagaFP4015_si_magnum_permissive)
#  include "DSP_raaga_octave_si.h"
#else
#  error "Unknown target - include the right header defining the RDB base addresses"
#endif


/*
 * Raaga Octave blocks memory map
 */

/* RAAGA_DSP_MISC registers */
#define BFPSDK_RAAGA_DSP_MISC_REVISION                             (BFPSDK_RAAGA_DSP_MISC_BASE + 0x0000)                  /* Audio DSP System Revision Register */
#define BFPSDK_RAAGA_DSP_MISC_SOFT_INIT                            (BFPSDK_RAAGA_DSP_MISC_BASE + 0x0004)                  /* Audio DSP System Soft Reset Control */
#define BFPSDK_RAAGA_DSP_MISC_CORE_ID                              (BFPSDK_RAAGA_DSP_MISC_BASE + 0x0008)                  /* DSP Core ID */
#define BFPSDK_RAAGA_DSP_MISC_PROC_ID                              (BFPSDK_RAAGA_DSP_MISC_BASE + 0x000c)                  /* DSP Processor ID */
#define BFPSDK_RAAGA_DSP_MISC_SCB0_BASE_CONFIG                     (BFPSDK_RAAGA_DSP_MISC_BASE + 0x0010)                  /* Raaga SCB0 Base Region Configuration */
#define BFPSDK_RAAGA_DSP_MISC_SCB0_EXT_CONFIG                      (BFPSDK_RAAGA_DSP_MISC_BASE + 0x0014)                  /* Raaga SCB0 Extension Region Configuration */
#define BFPSDK_RAAGA_DSP_MISC_SCB1_BASE_CONFIG                     (BFPSDK_RAAGA_DSP_MISC_BASE + 0x0018)                  /* Raaga SCB1 Base Region Configuration */
#define BFPSDK_RAAGA_DSP_MISC_SCB1_EXT_CONFIG                      (BFPSDK_RAAGA_DSP_MISC_BASE + 0x001c)                  /* Raaga SCB1 Extension Region Configuration */
#define BFPSDK_RAAGA_DSP_MISC_STATUS                               (BFPSDK_RAAGA_DSP_MISC_BASE + 0x0020)                  /* Audio DSP System Status */
#define BFPSDK_RAAGA_DSP_MISC_CLEAR_STATUS                         (BFPSDK_RAAGA_DSP_MISC_BASE + 0x0024)                  /* Audio DSP System Clear Status */

/* RAAGA_DSP_L2C registers in revision 1.0 ("R0") */
#define BFPSDK_RAAGA_DSP_L2C_R0_REVISION                           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0000)                   /* L2 Cache Revision Register */
#define BFPSDK_RAAGA_DSP_L2C_R0_CTRL0                              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0004)                   /* L2C Control 0 */
#define BFPSDK_RAAGA_DSP_L2C_R0_CTRL1                              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0008)                   /* L2C Control 1 */
#define BFPSDK_RAAGA_DSP_L2C_R0_CTRL2                              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x000c)                   /* L2C Control 2 */
#define BFPSDK_RAAGA_DSP_L2C_R0_CTRL3                              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0010)                   /* L2C Control 3 */
#define BFPSDK_RAAGA_DSP_L2C_R0_CTRL4                              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0014)                   /* L2C Control 4 */
#define BFPSDK_RAAGA_DSP_L2C_R0_CTRL5                              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0018)                   /* L2C Control 5 */
#define BFPSDK_RAAGA_DSP_L2C_R0_RRB_CTRL_0                         (BFPSDK_RAAGA_DSP_L2C_BASE + 0x001c)                   /* RRB Control */
#define BFPSDK_RAAGA_DSP_L2C_R0_RRB_CTRL(n)                        (BFPSDK_RAAGA_DSP_L2C_R0_RRB_CTRL_0 + 4 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R0_RRB_CTRL_COUNT                     4
#define BFPSDK_RAAGA_DSP_L2C_R0_AUTO_WB_AGEING_COUNT               (BFPSDK_RAAGA_DSP_L2C_BASE + 0x002c)                   /* Automatic Write Back Ageing Count */
#define BFPSDK_RAAGA_DSP_L2C_R0_PRIORITY_CFG                       (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0030)                   /* CBUS Priority Configuration */
#define BFPSDK_RAAGA_DSP_L2C_R0_QUEUE_PRIORITY_CFG                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0034)                   /* Queue Priority Configuration */
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_RELEASE_WAY                    (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0038)                   /* Release Way Command Controls */
#define BFPSDK_RAAGA_DSP_L2C_R0_RELEASE_WAY_DONE                   (BFPSDK_RAAGA_DSP_L2C_BASE + 0x003c)                   /* Release Way Done Status */
#define BFPSDK_RAAGA_DSP_L2C_R0_RELEASE_WAY_CLEAR                  (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0040)                   /* Release Way Clear */
#define BFPSDK_RAAGA_DSP_L2C_R0_CACHE_WAY_ENABLE                   (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0044)                   /* Cache Way Enable Controls */
#define BFPSDK_RAAGA_DSP_L2C_R0_POWERDN_WAY_ENABLE                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0048)                   /* Power-Down Way Enable Controls */
#define BFPSDK_RAAGA_DSP_L2C_R0_SMEM_ACCESS_WAY_ENABLE             (BFPSDK_RAAGA_DSP_L2C_BASE + 0x004c)                   /* SMEM Simulation Mode Way Enable Controls */
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_CTRL                           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0050)                   /* Command Controls */
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_SEQID                          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0058)                   /* Command Sequence ID */
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_DONE_STATUS_0                  (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0060)                   /* Queue Done Status */
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_DONE_STATUS(n)                 (BFPSDK_RAAGA_DSP_L2C_R0_CMD_DONE_STATUS_0 + 8 *(n))
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_DONE_STATUS_COUNT              8
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_ERROR_STATUS_0                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x00a0)                   /* Queue Error Status */
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_ERROR_STATUS(n)                (BFPSDK_RAAGA_DSP_L2C_R0_CMD_ERROR_STATUS_0 + 8 *(n))
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_ERROR_STATUS_COUNT             8
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_DONE_CLEAR_0                   (BFPSDK_RAAGA_DSP_L2C_BASE + 0x00e0)                   /* Queue Done/Error Status Clear Control */
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_DONE_CLEAR(n)                  (BFPSDK_RAAGA_DSP_L2C_R0_CMD_DONE_CLEAR_0 + 8 *(n))
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_DONE_STATUS_COUNT              8
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_QUEUE_ABORT                    (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0120)                   /* Abort Command for Queue */
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_ID_ABORT                       (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0128)                   /* Abort Command for Command ID */
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_ABORT_STATUS                   (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0130)                   /* Queue Abort Status */
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_ABORT_CLEAR                    (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0138)                   /* Queue Abort Status Clear Control */
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_QUEUE_MIGRATE                  (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0140)                   /* Migrate Command for Queue */
#define BFPSDK_RAAGA_DSP_L2C_R0_PCQ_QUEUE_CONTROL_0                (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0150)                   /* Pooled Command Queue Controls */
#define BFPSDK_RAAGA_DSP_L2C_R0_PCQ_QUEUE_CONTROL(n)               (BFPSDK_RAAGA_DSP_L2C_R0_PCQ_QUEUE_CONTROL_0 + 4 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R0_PCQ_QUEUE_CONTROL_COUNT            8
#define BFPSDK_RAAGA_DSP_L2C_R0_PCQ_QUEUE_STATUS_0                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0170)                   /* Pooled Command Queue Status */
#define BFPSDK_RAAGA_DSP_L2C_R0_PCQ_QUEUE_STATUS(n)                (BFPSDK_RAAGA_DSP_L2C_R0_PCQ_QUEUE_STATUS_0 + 4 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R0_PCQ_QUEUE_STATUS_COUNT             8
#define BFPSDK_RAAGA_DSP_L2C_R0_PCQ_CONTROL                        (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0190)                   /* Pooled Command Queue Controls */
#define BFPSDK_RAAGA_DSP_L2C_R0_PCQ_SW_INIT                        (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0194)                   /* Pooled Command Queue Soft Init */
#define BFPSDK_RAAGA_DSP_L2C_R0_PCQ_STATUS                         (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0198)                   /* Pooled Command Queue Status */
#define BFPSDK_RAAGA_DSP_L2C_R0_ADDR_TRANSLATION_TABLE_0           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x01a0)                   /* Physical Address translation Table */
#define BFPSDK_RAAGA_DSP_L2C_R0_ADDR_TRANSLATION_TABLE(n)          (BFPSDK_RAAGA_DSP_L2C_R0_ADDR_TRANSLATION_TABLE_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R0_ADDR_TRANSLATION_TABLE_COUNT       16
#define BFPSDK_RAAGA_DSP_L2C_R0_PHYSICAL_40B_BASE_ADDR_0           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0220)                   /* Physical Base Address */
#define BFPSDK_RAAGA_DSP_L2C_R0_PHYSICAL_40B_BASE_ADDR(n)          (BFPSDK_RAAGA_DSP_L2C_R0_PHYSICAL_40B_BASE_ADDR_0 + 4 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R0_PHYSICAL_40B_BASE_ADDR_COUNT       16
#define BFPSDK_RAAGA_DSP_L2C_R0_PAGE_40B_BASE_ADDR_0               (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0260)                   /* Physical Base Address */
#define BFPSDK_RAAGA_DSP_L2C_R0_PAGE_40B_BASE_ADDR(n)              (BFPSDK_RAAGA_DSP_L2C_R0_PAGE_40B_BASE_ADDR_0 + 4 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R0_PAGE_40B_BASE_ADDR_COUNT           128
#define BFPSDK_RAAGA_DSP_L2C_R0_PREFETCH_SCRATCH_CTRL_0            (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0460)                   /* Prefetch Scratch Control */
#define BFPSDK_RAAGA_DSP_L2C_R0_PREFETCH_SCRATCH_CTRL_1            (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0464)
#define BFPSDK_RAAGA_DSP_L2C_R0_PREFETCH_SCRATCH_CORE0_REF_BITS_0  (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0468)                   /* Prefetch Scratch Reference Bits for Core-0 */
#define BFPSDK_RAAGA_DSP_L2C_R0_PREFETCH_SCRATCH_CORE0_REF_BITS(n) (BFPSDK_RAAGA_DSP_L2C_R0_PREFETCH_SCRATCH_CORE0_REF_BITS_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R0_PREFETCH_SCRATCH_CORE1_REF_BITS_0  (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0568)                   /* Prefetch Scratch Reference Bits for Core-1 */
#define BFPSDK_RAAGA_DSP_L2C_R0_PREFETCH_SCRATCH_CORE1_REF_BITS(n) (BFPSDK_RAAGA_DSP_L2C_R0_PREFETCH_SCRATCH_CORE1_REF_BITS_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R0_PP_R2TD_ADDR                       (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0670)                   /* TAG or DATARAM Address for peek/poke access */
#define BFPSDK_RAAGA_DSP_L2C_R0_PP_R2TD_CMD                        (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0674)                   /* Peek and Poke Command Register for TAG and DATARAM */
#define BFPSDK_RAAGA_DSP_L2C_R0_PP_R2TD_AUTO                       (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0678)                   /* Set Peek/poke in Auto increment mode */
#define BFPSDK_RAAGA_DSP_L2C_R0_PP_R2TD_AUTO_RD_TRIG               (BFPSDK_RAAGA_DSP_L2C_BASE + 0x067c)                   /* Peek trigger in auto increment mode */
#define BFPSDK_RAAGA_DSP_L2C_R0_PP_R2TD_WDATA                      (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0680)                   /* Peek and Poke TAG/DATARAM RBUS Write Data */
#define BFPSDK_RAAGA_DSP_L2C_R0_PP_TD2R_RDATA                      (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0688)                   /* Peek and Poke TAG/DATARAM RBUS Read Data */
#define BFPSDK_RAAGA_DSP_L2C_R0_PP_R2TD_AUTO_ADDR_STATUS           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0690)                   /* Current address for peek/poke operation in auto increment mode. */
#define BFPSDK_RAAGA_DSP_L2C_R0_PP_STATUS                          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0694)                   /* Peek and Poke Interface Status */
#define BFPSDK_RAAGA_DSP_L2C_R0_ALLOCATION_ERROR_STATUS            (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0698)                   /* Allocation Error Status */
#define BFPSDK_RAAGA_DSP_L2C_R0_CMD_LOCK_ERROR_STATUS              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x069c)                   /* Command Lock Error Status */
#define BFPSDK_RAAGA_DSP_L2C_R0_SCB_ADDR_ERROR_STATUS              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06a0)                   /* SCB Address Error Status */
#define BFPSDK_RAAGA_DSP_L2C_R0_ERROR_STATUS_CLEAR                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06a8)                   /* Error Status Clear Control */
#define BFPSDK_RAAGA_DSP_L2C_R0_ERROR_INTERRUPT                    (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06b0)                   /* Error Interrupt Register */
#define BFPSDK_RAAGA_DSP_L2C_R0_ERROR_INTERRUPT_SET                (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06b4)                   /* Error Interrupt Set Register */
#define BFPSDK_RAAGA_DSP_L2C_R0_ERROR_INTERRUPT_CLEAR              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06b8)                   /* Error Interrupt Clear Register */
#define BFPSDK_RAAGA_DSP_L2C_R0_ERROR_INTERRUPT_MASK               (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06bc)                   /* Error Interrupt Mask Register */
#define BFPSDK_RAAGA_DSP_L2C_R0_ERROR_INTERRUPT_MASK_SET           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06c0)                   /* Error Interrupt Mask Set Register */
#define BFPSDK_RAAGA_DSP_L2C_R0_ERROR_INTERRUPT_MASK_CLEAR         (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06c4)                   /* Error Interrupt Mask Clear Register */
#define BFPSDK_RAAGA_DSP_L2C_R0_STATUS_INTERRUPT                   (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06d0)                   /* Status Interrupt Register */
#define BFPSDK_RAAGA_DSP_L2C_R0_STATUS_INTERRUPT_SET               (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06d4)                   /* Status Interrupt Set Register */
#define BFPSDK_RAAGA_DSP_L2C_R0_STATUS_INTERRUPT_CLEAR             (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06d8)                   /* Status Interrupt Clear Register */
#define BFPSDK_RAAGA_DSP_L2C_R0_STATUS_INTERRUPT_MASK              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06dc)                   /* Status Interrupt Mask Register */
#define BFPSDK_RAAGA_DSP_L2C_R0_STATUS_INTERRUPT_MASK_SET          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06e0)                   /* Status Interrupt Mask Set Register */
#define BFPSDK_RAAGA_DSP_L2C_R0_STATUS_INTERRUPT_MASK_CLEAR        (BFPSDK_RAAGA_DSP_L2C_BASE + 0x06e4)                   /* Status Interrupt Mask Clear Register */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_CTRL0                        (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0700)                   /* Debug Control-0 */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_CTRL1                        (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0704)                   /* Debug Control-1 */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_CTRL2                        (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0708)                   /* Debug Control-2 */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_FW_DATA                      (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0710)                   /* Debug FW Data */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_STATUS                       (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0718)                   /* Debug Status */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_TIMESTAMP                    (BFPSDK_RAAGA_DSP_L2C_BASE + 0x071c)                   /* Debug Timestamp */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_ICACHE_HIT_COUNT_0           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0720)                   /* Debug I-Cache Hit Count */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_ICACHE_HIT_COUNT_1           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0724)
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_DCACHE_HIT_COUNT_0           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0728)                   /* Debug D-Cache Hit Count */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_DCACHE_HIT_COUNT_1           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x072c)
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_ICACHE_MISS_COUNT_0          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0730)                   /* Debug I-Cache Miss Count */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_ICACHE_MISS_COUNT_1          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0734)
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_DCACHE_MISS_COUNT_0          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0738)                   /* Debug D-Cache Miss Count */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_DCACHE_MISS_COUNT_1          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x073c)
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_ICACHE_STALL_COUNT_0         (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0740)                   /* Debug I-Cache Stall Count */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_ICACHE_STALL_COUNT_1         (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0744)
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_DCACHE_STALL_COUNT_0         (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0748)                   /* Debug D-Cache Stall Count */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_DCACHE_STALL_COUNT_1         (BFPSDK_RAAGA_DSP_L2C_BASE + 0x074c)
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_WITHOUT_SCB_COUNT_0          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0750)                   /* Debug Allocate/Flush Without SCB Count */
#define BFPSDK_RAAGA_DSP_L2C_R0_DEBUG_WITHOUT_SCB_COUNT_1          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0754)
#define BFPSDK_RAAGA_DSP_L2C_R0_SCRATCH_REG                        (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0ff8)                   /* Scratch Register */


/* RAAGA_DSP_L2C registers in revision 1.1 ("R1") */
#define BFPSDK_RAAGA_DSP_L2C_R1_REVISION                              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0000)             /* L2 Cache Revision Register */
#define BFPSDK_RAAGA_DSP_L2C_R1_CTRL0                                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0004)             /* L2C Control 0 */
#define BFPSDK_RAAGA_DSP_L2C_R1_CTRL1                                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0008)             /* L2C Control 1 */
#define BFPSDK_RAAGA_DSP_L2C_R1_CTRL2                                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x000c)             /* L2C Control 2 */
#define BFPSDK_RAAGA_DSP_L2C_R1_CTRL3                                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0010)             /* L2C Control 3 */
#define BFPSDK_RAAGA_DSP_L2C_R1_CTRL4                                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0014)             /* L2C Control 4 */
#define BFPSDK_RAAGA_DSP_L2C_R1_CTRL5                                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0018)             /* L2C Control 5 */
#define BFPSDK_RAAGA_DSP_L2C_R1_HW_PREFETCH_CTRL                      (BFPSDK_RAAGA_DSP_L2C_BASE + 0x001c)             /* L2C HW Prefetch Control */
#define BFPSDK_RAAGA_DSP_L2C_R1_RRB_CTRL_0                            (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0020)             /* RRB Control */
#define BFPSDK_RAAGA_DSP_L2C_R1_RRB_CTRL(n)                           (BFPSDK_RAAGA_DSP_L2C_R1_RRB_CTRL_0 + 4 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_RRB_CTRL_COUNT                        4
#define BFPSDK_RAAGA_DSP_L2C_R1_AUTO_WB_AGEING_COUNT                  (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0030)             /* Automatic Write Back Ageing Count */
#define BFPSDK_RAAGA_DSP_L2C_R1_PRIORITY_CFG                          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0034)             /* CBUS Priority Configuration */
#define BFPSDK_RAAGA_DSP_L2C_R1_QUEUE_PRIORITY_CFG                    (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0038)             /* Queue Priority Configuration */
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_RELEASE_WAY                       (BFPSDK_RAAGA_DSP_L2C_BASE + 0x003c)             /* Release Way Command Controls */
#define BFPSDK_RAAGA_DSP_L2C_R1_RELEASE_WAY_DONE                      (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0040)             /* Release Way Done Status */
#define BFPSDK_RAAGA_DSP_L2C_R1_RELEASE_WAY_CLEAR                     (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0044)             /* Release Way Clear */
#define BFPSDK_RAAGA_DSP_L2C_R1_CACHE_WAY_ENABLE                      (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0048)             /* Cache Way Enable Controls */
#define BFPSDK_RAAGA_DSP_L2C_R1_POWERDN_WAY_ENABLE                    (BFPSDK_RAAGA_DSP_L2C_BASE + 0x004c)             /* Power-Down Way Enable Controls */
#define BFPSDK_RAAGA_DSP_L2C_R1_SMEM_ACCESS_WAY_ENABLE                (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0050)             /* SMEM Simulation Mode Way Enable Controls */
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_CTRL                              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0060)             /* Command Controls */
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_SEQID                             (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0068)             /* Command Sequence ID */
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_DONE_STATUS_0                     (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0070)             /* Queue Done Status */
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_DONE_STATUS(n)                    (BFPSDK_RAAGA_DSP_L2C_R1_CMD_DONE_STATUS_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_DONE_STATUS_COUNT                 8
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_ERROR_STATUS_0                    (BFPSDK_RAAGA_DSP_L2C_BASE + 0x00b0)             /* Queue Error Status */
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_ERROR_STATUS(n)                   (BFPSDK_RAAGA_DSP_L2C_R1_CMD_ERROR_STATUS_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_ERROR_STATUS_COUNT                8
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_DONE_CLEAR_0                      (BFPSDK_RAAGA_DSP_L2C_BASE + 0x00f0)             /* Queue Done/Error Status Clear Control */
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_DONE_CLEAR(n)                     (BFPSDK_RAAGA_DSP_L2C_R1_CMD_DONE_CLEAR_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_DONE_CLEAR_COUNT                  8
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_QUEUE_ABORT                       (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0130)             /* Abort Command for Queue */
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_ID_ABORT                          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0138)             /* Abort Command for Command ID */
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_ABORT_STATUS                      (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0140)             /* Queue Abort Status */
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_ABORT_CLEAR                       (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0148)             /* Queue Abort Status Clear Control */
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_QUEUE_MIGRATE                     (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0150)             /* Migrate Command for Queue */
#define BFPSDK_RAAGA_DSP_L2C_R1_PCQ_QUEUE_0                           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0160)             /* Pooled Command Queue Controls */
#define BFPSDK_RAAGA_DSP_L2C_R1_PCQ_QUEUE_CONTROL(n)                  (BFPSDK_RAAGA_DSP_L2C_R1_PCQ_QUEUE_0 + 4 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_PCQ_QUEUE_COUNT                       8
#define BFPSDK_RAAGA_DSP_L2C_R1_PCQ_QUEUE_STATUS_0                    (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0190)             /* Pooled Command Queue Status */
#define BFPSDK_RAAGA_DSP_L2C_R1_PCQ_QUEUE_STATUS(n)                   (BFPSDK_RAAGA_DSP_L2C_R1_PCQ_QUEUE_STATUS_0 + 4 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_PCQ_QUEUE_STATUS_COUNT                8
#define BFPSDK_RAAGA_DSP_L2C_R1_PCQ_CONTROL                           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x01b0)             /* Pooled Command Queue Controls */
#define BFPSDK_RAAGA_DSP_L2C_R1_PCQ_SW_INIT                           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x01b4)             /* Pooled Command Queue Soft Init */
#define BFPSDK_RAAGA_DSP_L2C_R1_PCQ_STATUS                            (BFPSDK_RAAGA_DSP_L2C_BASE + 0x01b8)             /* Pooled Command Queue Status */
#define BFPSDK_RAAGA_DSP_L2C_R1_ADDR_TRANSLATION_TABLE_0              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x01c0)             /* Physical Address translation Table */
#define BFPSDK_RAAGA_DSP_L2C_R1_ADDR_TRANSLATION_TABLE(n)             (BFPSDK_RAAGA_DSP_L2C_R1_ADDR_TRANSLATION_TABLE_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_ADDR_TRANSLATION_TABLE_COUNT          128
#define BFPSDK_RAAGA_DSP_L2C_R1_PHYSICAL_40B_BASE_ADDR_0              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x05c0)             /* Physical Base Address */
#define BFPSDK_RAAGA_DSP_L2C_R1_PHYSICAL_40B_BASE_ADDR(n)             (BFPSDK_RAAGA_DSP_L2C_R1_PHYSICAL_40B_BASE_ADDR_0 + 4 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_PHYSICAL_40B_BASE_ADDR_COUNT          128
#define BFPSDK_RAAGA_DSP_L2C_R1_PREFETCH_SCRATCH_CTRL_0               (BFPSDK_RAAGA_DSP_L2C_BASE + 0x07c0)             /* Prefetch Scratch Control */
#define BFPSDK_RAAGA_DSP_L2C_R1_PREFETCH_SCRATCH_CTRL(n)              (BFPSDK_RAAGA_DSP_L2C_R1_PREFETCH_SCRATCH_CTRL_0 + 4 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_PREFETCH_SCRATCH_CTRL_COUNT           2
#define BFPSDK_RAAGA_DSP_L2C_R1_PREFETCH_SCRATCH_CORE0_REF_BITS_0     (BFPSDK_RAAGA_DSP_L2C_BASE + 0x07c8)             /* Prefetch Scratch Reference Bits for Core-0 */
#define BFPSDK_RAAGA_DSP_L2C_R1_PREFETCH_SCRATCH_CORE0_REF_BITS(n)    (BFPSDK_RAAGA_DSP_L2C_R1_PREFETCH_SCRATCH_CORE0_REF_BITS_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_PREFETCH_SCRATCH_CORE0_REF_BITS_COUNT 32
#define BFPSDK_RAAGA_DSP_L2C_R1_PREFETCH_SCRATCH_CORE1_REF_BITS_0     (BFPSDK_RAAGA_DSP_L2C_BASE + 0x08c8)             /* Prefetch Scratch Reference Bits for Core-1 */
#define BFPSDK_RAAGA_DSP_L2C_R1_PREFETCH_SCRATCH_CORE1_REF_BITS(n)    (BFPSDK_RAAGA_DSP_L2C_R1_PREFETCH_SCRATCH_CORE1_REF_BITS_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_PREFETCH_SCRATCH_CORE1_REF_BITS_COUNT 32
#define BFPSDK_RAAGA_DSP_L2C_R1_PP_R2TD_ADDR                          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x09d0)             /* TAG or DATARAM Address for peek/poke access */
#define BFPSDK_RAAGA_DSP_L2C_R1_PP_R2TD_CMD                           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x09d4)             /* Peek and Poke Command Register for TAG and DATARAM */
#define BFPSDK_RAAGA_DSP_L2C_R1_PP_R2TD_AUTO                          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x09d8)             /* Set Peek/poke in Auto increment mode */
#define BFPSDK_RAAGA_DSP_L2C_R1_PP_R2TD_AUTO_RD_TRIG                  (BFPSDK_RAAGA_DSP_L2C_BASE + 0x09dc)             /* Peek trigger in auto increment mode */
#define BFPSDK_RAAGA_DSP_L2C_R1_PP_R2TD_WDATA                         (BFPSDK_RAAGA_DSP_L2C_BASE + 0x09e0)             /* Peek and Poke TAG/DATARAM RBUS Write Data */
#define BFPSDK_RAAGA_DSP_L2C_R1_PP_TD2R_RDATA                         (BFPSDK_RAAGA_DSP_L2C_BASE + 0x09e8)             /* Peek and Poke TAG/DATARAM RBUS Read Data */
#define BFPSDK_RAAGA_DSP_L2C_R1_PP_R2TD_AUTO_ADDR_STATUS              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x09f0)             /* Current address for peek/poke operation in auto increment mode. */
#define BFPSDK_RAAGA_DSP_L2C_R1_PP_STATUS                             (BFPSDK_RAAGA_DSP_L2C_BASE + 0x09f4)             /* Peek and Poke Interface Status */
#define BFPSDK_RAAGA_DSP_L2C_R1_ICA_RD_DCA_ERROR_STATUS               (BFPSDK_RAAGA_DSP_L2C_BASE + 0x09f8)             /* Allocation Error Status */
#define BFPSDK_RAAGA_DSP_L2C_R1_DCA_WR_ICA_ERROR_STATUS               (BFPSDK_RAAGA_DSP_L2C_BASE + 0x09fc)             /* Allocation Error Status */
#define BFPSDK_RAAGA_DSP_L2C_R1_ALLOCATION_ERROR_STATUS               (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a00)             /* Allocation Error Status */
#define BFPSDK_RAAGA_DSP_L2C_R1_CMD_LOCK_ERROR_STATUS                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a04)             /* Command Lock Error Status */
#define BFPSDK_RAAGA_DSP_L2C_R1_SCB_ADDR_ERROR_STATUS                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a08)             /* SCB Address Error Status */
#define BFPSDK_RAAGA_DSP_L2C_R1_ERROR_STATUS_CLEAR                    (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a10)             /* Error Status Clear Control */
#define BFPSDK_RAAGA_DSP_L2C_R1_ERROR_INTERRUPT                       (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a20)             /* Error Interrupt Register */
#define BFPSDK_RAAGA_DSP_L2C_R1_ERROR_INTERRUPT_SET                   (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a24)             /* Error Interrupt Set Register */
#define BFPSDK_RAAGA_DSP_L2C_R1_ERROR_INTERRUPT_CLEAR                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a28)             /* Error Interrupt Clear Register */
#define BFPSDK_RAAGA_DSP_L2C_R1_ERROR_INTERRUPT_MASK                  (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a2c)             /* Error Interrupt Mask Register */
#define BFPSDK_RAAGA_DSP_L2C_R1_ERROR_INTERRUPT_MASK_SET              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a30)             /* Error Interrupt Mask Set Register */
#define BFPSDK_RAAGA_DSP_L2C_R1_ERROR_INTERRUPT_MASK_CLEAR            (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a34)             /* Error Interrupt Mask Clear Register */
#define BFPSDK_RAAGA_DSP_L2C_R1_STATUS_INTERRUPT                      (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a38)             /* Status Interrupt Register */
#define BFPSDK_RAAGA_DSP_L2C_R1_STATUS_INTERRUPT_SET                  (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a3c)             /* Status Interrupt Set Register */
#define BFPSDK_RAAGA_DSP_L2C_R1_STATUS_INTERRUPT_CLEAR                (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a40)             /* Status Interrupt Clear Register */
#define BFPSDK_RAAGA_DSP_L2C_R1_STATUS_INTERRUPT_MASK                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a44)             /* Status Interrupt Mask Register */
#define BFPSDK_RAAGA_DSP_L2C_R1_STATUS_INTERRUPT_MASK_SET             (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a48)             /* Status Interrupt Mask Set Register */
#define BFPSDK_RAAGA_DSP_L2C_R1_STATUS_INTERRUPT_MASK_CLEAR           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a4c)             /* Status Interrupt Mask Clear Register */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_CTRL0                           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a50)             /* Debug Control-0 */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_CTRL1                           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a54)             /* Debug Control-1 */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_CTRL2                           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a58)             /* Debug Control-2 */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_FW_DATA                         (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a60)             /* Debug FW Data */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_STATUS                          (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a68)             /* Debug Status */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_TIMESTAMP                       (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a6c)             /* Debug Timestamp */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_ICACHE_HIT_COUNT_0              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a70)             /* Debug I-Cache Hit Count */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_ICACHE_HIT_COUNT_1              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a74)
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_DCACHE_HIT_COUNT_0              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a78)             /* Debug D-Cache Hit Count */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_DCACHE_HIT_COUNT_1              (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a7c)
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_ICACHE_MISS_COUNT_0             (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a80)             /* Debug I-Cache Miss Count */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_ICACHE_MISS_COUNT_1             (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a84)
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_DCACHE_MISS_COUNT_0             (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a88)             /* Debug D-Cache Miss Count */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_DCACHE_MISS_COUNT_1             (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a8c)
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_ICACHE_STALL_COUNT_0            (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a90)             /* Debug I-Cache Stall Count */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_ICACHE_STALL_COUNT_1            (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a94)
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_DCACHE_STALL_COUNT_0            (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a98)             /* Debug D-Cache Stall Count */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_DCACHE_STALL_COUNT_1            (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0a9c)
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_WITHOUT_SCB_COUNT_0             (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0aa0)             /* Debug Allocate/Flush Without SCB Count */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_WITHOUT_SCB_COUNT_1             (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0aa4)
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_L2C_HW_STATUS0                  (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0b00)             /* Debug L2C HW Status0 */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_L2C_HW_STATUS1                  (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0b08)             /* Debug L2C HW Status1 */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB3_BUF_CTRL_0                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0b10)             /* Debug L2C RRB3 Stored Control */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB3_BUF_CTRL(n)                (BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB3_BUF_CTRL_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB3_BUF_CTRL_COUNT 8
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB2_BUF_CTRL_0                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0b50)             /* Debug L2C RRB2 Stored Control */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB2_BUF_CTRL(n)                (BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB2_BUF_CTRL_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB2_BUF_CTRL_COUNT 8
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB1_BUF_CTRL_0                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0b90)             /* Debug L2C RRB1 Stored Control */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB1_BUF_CTRL(n)                (BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB1_BUF_CTRL_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB1_BUF_CTRL_COUNT 8
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB0_BUF_CTRL_0                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0bd0)             /* Debug L2C RRB0 Stored Control */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB0_BUF_CTRL(n)                (BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB0_BUF_CTRL_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_RRB0_BUF_CTRL_COUNT 8
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_WBB_BUF_CTRL_0                  (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0c10)             /* Debug L2C WBB Stored Control */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_WBB_BUF_CTRL(n)                 (BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_WBB_BUF_CTRL_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_WBB_BUF_CTRL_COUNT 16
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_PREFETCH_BUF_CTRL_0             (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0c90)             /* Debug L2C Prefetch Buffer Stored Control */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_PREFETCH_BUF_CTRL(n)            (BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_PREFETCH_BUF_CTRL_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_PREFETCH_BUF_CTRL_COUNT 8
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_MISS_BUF_CTRL_0                 (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0cd0)             /* Debug L2C Miss Buffer Stored Control */
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_MISS_BUF_CTRL(n)                (BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_MISS_BUF_CTRL_0 + 8 * (n))
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_MISS_BUF_CTRL_COUNT 8
#define BFPSDK_RAAGA_DSP_L2C_R1_DEBUG_L2C_CONFIG                      (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0d10)             /* Debug L2C Configuration */
#define BFPSDK_RAAGA_DSP_L2C_R1_SCRATCH_REG                           (BFPSDK_RAAGA_DSP_L2C_BASE + 0x0ff8)             /* Scratch Register */

/* RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE fields - shared with both R0 and R1 versions.
 * In R1 PAGEING_EN always reads as 0. */
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_TRANSLATION_VALID_MASK      BITMASK_64(0, 0)
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_TRANSLATION_VALID_ALIGN     0
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_TRANSLATION_VALID_BITS      1
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_TRANSLATION_VALID_SHIFT     0
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_PAGEING_EN_MASK             BITMASK_64(1, 1)
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_PAGEING_EN_ALIGN            0
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_PAGEING_EN_BITS             1
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_PAGEING_EN_SHIFT            1
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_VIRTUAL_START_MASK          BITMASK_64(31, 12)
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_VIRTUAL_START_ALIGN         0
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_VIRTUAL_START_BITS          20
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_VIRTUAL_START_SHIFT         12
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_VIRTUAL_END_MASK            BITMASK_64(63, 44)
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_VIRTUAL_END_ALIGN           0
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_VIRTUAL_END_BITS            20
#define BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE_VIRTUAL_END_SHIFT           44

/* RAAGA_DSP_L2C_PHYSICAL_40B_BASE_ADDR fields - shared with both R0 and R1 versions */
#define BFPSDK_RAAGA_DSP_L2C_PHYSICAL_40B_BASE_ADDR_BASE_ADDR_MASK      BITMASK_32(30, 0)
#define BFPSDK_RAAGA_DSP_L2C_PHYSICAL_40B_BASE_ADDR_BASE_ADDR_ALIGN     0
#define BFPSDK_RAAGA_DSP_L2C_PHYSICAL_40B_BASE_ADDR_BASE_ADDR_BITS      31
#define BFPSDK_RAAGA_DSP_L2C_PHYSICAL_40B_BASE_ADDR_BASE_ADDR_SHIFT     0


/*
 * 64 bit RDB access methods.
 */
uint64_t DSP_readShared64BitRegister(DSP *dsp, SHARED_ADDR reg_addr);
void DSP_writeShared64BitRegister(DSP *dsp, SHARED_ADDR reg_addr, uint64_t value);


#endif  /* _DSP_RAAGA_OCTAVE_H_ */
