/****************************************************************************
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
 ****************************************************************************/

#ifndef _DSP_RAAGA_FP2000_H_
#define _DSP_RAAGA_FP2000_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"

#include "fp_sdk_config.h"



#if IS_HOST(DSP_LESS)
#  error "This module is not suitable for DSP-less builds"
#endif
#if defined(__FP4015_ONWARDS__)
#  error "This module is suitable only for FP20xx machines"
#endif


#ifdef RAAGA

#if IS_HOST(SILICON) && (FEATURE_IS(SW_HOST, RAAGA_MAGNUM) || FEATURE_IS(SW_HOST, RAAGA_ROCKFORD))
/* Silicon target */
#  include "bchp_raaga_dsp_rgr.h"
#  include "bchp_raaga_dsp_dma.h"
#  include "bchp_raaga_dsp_mem_subsystem.h"
#  include "bchp_raaga_dsp_peri_dbg_ctrl.h"
#  include "bchp_raaga_dsp_inth.h"
#  include "bchp_raaga_dsp_misc.h"
#  include "bchp_raaga_dsp_fw_inth.h"

/* FIXME: when building in Rockford, we are stuck on a very old commit of the refsw repo,
 * where bdsp_bchp_raaga_dsp_fw_cfg.h doesn't exist; in that case, resort including
 * bchp_raaga_dsp_fw_cfg.h directly. */
#  if FEATURE_IS(SW_HOST, RAAGA_ROCKFORD)
#    include "bchp_raaga_dsp_fw_cfg.h"
#  else
#    include "bdsp_bchp_raaga_dsp_fw_cfg.h"
#  endif
#else
/* BM target. We build this in the SDK build tree, but we do not have an RDB header file */
#  include "fp_sdk_config.h"
/* When talking to the 'system' through the SCP port, we write directly at the addresses in
 * memory where the DSP expect to see these registers. This is why, below, we have register
 * addresses as they are mapped on the DSP. */
#  define BCHP_RAAGA_DSP_MISC_SCRATCH_0                                             0x80020020
#  define BCHP_RAAGA_DSP_MISC_SCRATCH_1                                             0x80020024
#  define BCHP_RAAGA_DSP_MISC_SCRATCH_2                                             0x80020028
#  define BCHP_RAAGA_DSP_MISC_SCRATCH_3                                             0x8002002C
#  define BCHP_RAAGA_DSP_INTH_HOST_STATUS                                           0x80022200
#  define BCHP_RAAGA_DSP_INTH_HOST_SET                                              0x80022204
#  define BCHP_RAAGA_DSP_INTH_HOST_CLEAR                                            0x80022208
#  if !defined(__FP2012_ONWARDS__)  /* pre-rev2000 Raaga */
#    define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS                                      0x80022400
#    define BCHP_RAAGA_DSP_FW_INTH_HOST_SET                                         0x80022404
#    define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR                                       0x80022408
#  else                             /* rev2000 Raaga */
#    define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS                                      0x80022800
#    define BCHP_RAAGA_DSP_FW_INTH_HOST_SET                                         0x80022804
#    define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR                                       0x80022808
#  endif

#  if !defined(__FP2012_ONWARDS__)  /* HW debug console doesn't exist in rev2000 Raaga */
#    define BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_WR_DATA                        0x80021090
#    define BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_RD_DATA                        0x80021094
#    define BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_CTRL                           0x80021098
#    define BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_STATUS                         0x8002109c
#    define BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_CTRL_RD_DATA_DONE_MASK         0x00000001
#    define BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_STATUS_RD_DATA_RDY_MASK        0x00000002
#    define BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_STATUS_WR_DATA_ACCEPT_MASK     0x00000001
#  endif

#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_ID2R_RDATA                   0x80030000
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_ID2R_WDATA                   0x80030004
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_R2ID_CMD                     0x80030008
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_R2ID_ADDR                    0x8003000c
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_R2ID_CMD_MEMSEL_SHIFT        0
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_R2ID_CMD_RD_WR_CMD_SHIFT     4
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_STATUS          0x800300a8
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_CLEAR           0x800300ac
#  define BCHP_RAAGA_DSP_INTH_HOST_STATUS_MEM_PEEK_POKE_SHIFT       25

#  define BCHP_RAAGA_DSP_FW_CFG_SW_DEBUG_SPAREi_ARRAY_BASE          0x80023058
#  define BCHP_RAAGA_DSP_FW_CFG_SW_DEBUG_SPAREi_ARRAY_START         0
#  define BCHP_RAAGA_DSP_FW_CFG_SW_DEBUG_SPAREi_ARRAY_END           225
#  define BCHP_RAAGA_DSP_FW_CFG_SW_DEBUG_SPAREi_ARRAY_ELEMENT_SIZE  32

#  define BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_0    0x800233e0 /* Base Address for FIFO 0 */
#  define BCHP_RAAGA_DSP_FW_CFG_END_ADDR_FIFO_0     0x800233e4 /* End Address for FIFO 0 */
#  define BCHP_RAAGA_DSP_FW_CFG_WRITE_PTR_FIFO_0    0x800233e8 /* Write Pointer for FIFO 0 */
#  define BCHP_RAAGA_DSP_FW_CFG_READ_PTR_FIFO_0     0x800233ec /* Read Pointer for FIFO 0 */
#  define BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_1    0x800233f0 /* Base Address for FIFO 1 */


/* DSP_FW_INTH_HOST_STATUS - Host Interrupt Status Register */
#  define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_HOST_MSG_MASK              0x00000004
#  define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_HOST_MSG_SHIFT             2
#  define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_ASYNC_MSG_MASK             0x00000002
#  define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_ASYNC_MSG_SHIFT            1
#  define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_SYNC_MSG_MASK              0x00000001
#  define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_SYNC_MSG_SHIFT             0

/* DSP_MEMSUBSYTEM - MEMSUB_ERROR_STATUS */
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_STATUS_DMEM_FP_EVENX_OOERROR_MASK   0x00000400
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_STATUS_DMEM_FP_EVENY_OOERROR_MASK   0x00000200
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_STATUS_DMEM_FP_ODDX_OOERROR_MASK    0x00000100
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_STATUS_DMEM_FP_ODDY_OOERROR_MASK    0x00000080
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_STATUS_DMEM_DMA_EVEN_OOERROR_MASK   0x00000040
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_STATUS_DMEM_DMA_ODD_OOERROR_MASK    0x00000020
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_STATUS_DMEM_PP_OOERROR_MASK         0x00000010
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_STATUS_IMEM_FP_OOERROR_MASK         0x00000008
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_STATUS_IMEM_DMA_EVEN_OOERROR_MASK   0x00000004
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_STATUS_IMEM_DMA_ODD_OOERROR_MASK    0x00000002
#  define BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_STATUS_IMEM_PP_OOERROR_MASK         0x00000001

#  if !defined(__FP2012_ONWARDS__)  /* pre-rev2000 Raaga */

/* DSP_INTH_HOST_STATUS - Host Interrupt Status Register */
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_UART_ERROR_MASK               0x80000000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_UART_ERROR_SHIFT              31
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_MEM_SUB_ERROR_MASK            0x40000000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_MEM_SUB_ERROR_SHIFT           30
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_MDMEM_MISS_ERROR_MASK         0x20000000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_MDMEM_MISS_ERROR_SHIFT        29
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_SUB_ERROR0_MASK           0x10000000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_SUB_ERROR0_SHIFT          28
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_GISB_ERROR_MASK               0x08000000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_GISB_ERROR_SHIFT              27
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_PMEM_BRIDGE_ERROR_MASK        0x04000000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_PMEM_BRIDGE_ERROR_SHIFT       26
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_MEM_PEEK_POKE_MASK            0x02000000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_MEM_PEEK_POKE_SHIFT           25
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_RICH_MASK                     0x01000000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_RICH_SHIFT                    24
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_SEC_PROT_VIOL_MASK            0x00800000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_SEC_PROT_VIOL_SHIFT           23
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DAISY_CHAIN_TIMER_INT_MASK    0x00400000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DAISY_CHAIN_TIMER_INT_SHIFT    22
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_SYS_CLK_TIMER_INT3_MASK       0x00200000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_SYS_CLK_TIMER_INT3_SHIFT      21
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_SYS_CLK_TIMER_INT2_MASK       0x00100000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_SYS_CLK_TIMER_INT2_SHIFT      20
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_SYS_CLK_TIMER_INT1_MASK       0x00080000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_SYS_CLK_TIMER_INT1_SHIFT      19
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_SYS_CLK_TIMER_INT0_MASK       0x00040000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_SYS_CLK_TIMER_INT0_SHIFT      18
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DSP_CLK_TIMER_INT_MASK        0x00020000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DSP_CLK_TIMER_INT_SHIFT       17
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_TSM_TIMER_INT_MASK            0x00010000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_TSM_TIMER_INT_SHIFT           16
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_VOM_MISS_INT_MASK             0x00008000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_VOM_MISS_INT_SHIFT            15
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q5_MASK                   0x00000200
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q5_SHIFT                  9
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q4_MASK                   0x00000100
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q4_SHIFT                  8
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q3_MASK                   0x00000080
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q3_SHIFT                  7
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q2_MASK                   0x00000040
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q2_SHIFT                  6
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q1_MASK                   0x00000020
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q1_SHIFT                  5
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q0_MASK                   0x00000010
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q0_SHIFT                  4
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_UART_RX_MASK                  0x00000008
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_UART_RX_SHIFT                 3
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_UART_TX_MASK                  0x00000004
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_UART_TX_SHIFT                 2
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_WATCHDOG_TIMER_ATTN_MASK      0x00000001
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_WATCHDOG_TIMER_ATTN_SHIFT     0

/* DSP_INTH_HOST_CLEAR - Host Interrupt Clear Register */
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_UART_ERROR_MASK                0x80000000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_UART_ERROR_SHIFT               31
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_MEM_SUB_ERROR_MASK             0x40000000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_MEM_SUB_ERROR_SHIFT            30
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_MDMEM_MISS_ERROR_MASK          0x20000000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_MDMEM_MISS_ERROR_SHIFT         29
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_SUB_ERROR0_MASK            0x10000000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_SUB_ERROR0_SHIFT           28
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_GISB_ERROR_MASK                0x08000000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_GISB_ERROR_SHIFT               27
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_PMEM_BRIDGE_ERROR_MASK         0x04000000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_PMEM_BRIDGE_ERROR_SHIFT        26
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_MEM_PEEK_POKE_MASK             0x02000000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_MEM_PEEK_POKE_SHIFT            25
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_RICH_MASK                      0x01000000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_RICH_SHIFT                     24
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_SEC_PROT_VIOL_MASK             0x00800000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_SEC_PROT_VIOL_SHIFT            23
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DAISY_CHAIN_TIMER_INT_MASK     0x00400000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DAISY_CHAIN_TIMER_INT_SHIFT    22
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_SYS_CLK_TIMER_INT3_MASK        0x00200000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_SYS_CLK_TIMER_INT3_SHIFT       21
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_SYS_CLK_TIMER_INT2_MASK        0x00100000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_SYS_CLK_TIMER_INT2_SHIFT       20
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_SYS_CLK_TIMER_INT1_MASK        0x00080000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_SYS_CLK_TIMER_INT1_SHIFT       19
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_SYS_CLK_TIMER_INT0_MASK        0x00040000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_SYS_CLK_TIMER_INT0_SHIFT       18
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DSP_CLK_TIMER_INT_MASK         0x00020000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DSP_CLK_TIMER_INT_SHIFT        17
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_TSM_TIMER_INT_MASK             0x00010000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_TSM_TIMER_INT_SHIFT            16
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_VOM_MISS_INT_MASK              0x00008000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_VOM_MISS_INT_SHIFT             15
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q5_MASK                    0x00000200
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q5_SHIFT                   9
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q4_MASK                    0x00000100
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q4_SHIFT                   8
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q3_MASK                    0x00000080
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q3_SHIFT                   7
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q2_MASK                    0x00000040
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q2_SHIFT                   6
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q1_MASK                    0x00000020
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q1_SHIFT                   5
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q0_MASK                    0x00000010
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q0_SHIFT                   4
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_UART_RX_MASK                   0x00000008
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_UART_RX_SHIFT                  3
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_UART_TX_MASK                   0x00000004
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_UART_TX_SHIFT                  2
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_WATCHDOG_TIMER_ATTN_MASK       0x00000001
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_WATCHDOG_TIMER_ATTN_SHIFT      0

#  else                             /* rev2000 Raaga */

/* DSP_INTH_HOST_STATUS - Host Interrupt Status Register */
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_MEM_PEEK_POKE_MASK            0x02000000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_MEM_PEEK_POKE_SHIFT           25
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_RICH_MASK                     0x01000000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_RICH_SHIFT                    24
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_SEC_PROT_VIOL_MASK            0x00800000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_SEC_PROT_VIOL_SHIFT           23
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_VOM_MISS_INT_MASK             0x00008000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_VOM_MISS_INT_SHIFT            15
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_AX_DONE_INT_MASK              0x00002000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_AX_DONE_INT_SHIFT             13
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_AX_ERROR_INT_MASK             0x00001000
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_AX_ERROR_INT_SHIFT            12
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_ERROR_INT_MASK                0x00000800
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_ERROR_INT_SHIFT               11
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_TIMER_INT_MASK                0x00000400
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_TIMER_INT_SHIFT               10
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q5_MASK                   0x00000200
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q5_SHIFT                  9
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q4_MASK                   0x00000100
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q4_SHIFT                  8
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q3_MASK                   0x00000080
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q3_SHIFT                  7
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q2_MASK                   0x00000040
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q2_SHIFT                  6
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q1_MASK                   0x00000020
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q1_SHIFT                  5
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q0_MASK                   0x00000010
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_DMA_Q0_SHIFT                  4
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_UART_RX_MASK                  0x00000008
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_UART_RX_SHIFT                 3
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_UART_TX_MASK                  0x00000004
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_UART_TX_SHIFT                 2
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_PRQ_WATERMARK_MASK            0x00000002
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_PRQ_WATERMARK_SHIFT           1
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_WATCHDOG_TIMER_ATTN_MASK      0x00000001
#    define BCHP_RAAGA_DSP_INTH_HOST_STATUS_WATCHDOG_TIMER_ATTN_SHIFT     0

/* DSP_INTH_HOST_CLEAR - Host Interrupt Clear Register */
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_MEM_PEEK_POKE_MASK             0x02000000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_MEM_PEEK_POKE_SHIFT            25
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_RICH_MASK                      0x01000000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_RICH_SHIFT                     24
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_SEC_PROT_VIOL_MASK             0x00800000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_SEC_PROT_VIOL_SHIFT            23
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_VOM_MISS_INT_MASK              0x00008000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_VOM_MISS_INT_SHIFT             15
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_AX_DONE_INT_MASK               0x00002000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_AX_DONE_INT_SHIFT              13
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_AX_ERROR_INT_MASK              0x00001000
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_AX_ERROR_INT_SHIFT             12
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_ERROR_INT_MASK                 0x00000800
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_ERROR_INT_SHIFT                11
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_TIMER_INT_MASK                 0x00000400
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_TIMER_INT_SHIFT                10
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q5_MASK                    0x00000200
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q5_SHIFT                   9
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q4_MASK                    0x00000100
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q4_SHIFT                   8
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q3_MASK                    0x00000080
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q3_SHIFT                   7
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q2_MASK                    0x00000040
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q2_SHIFT                   6
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q1_MASK                    0x00000020
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q1_SHIFT                   5
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q0_MASK                    0x00000010
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_DMA_Q0_SHIFT                   4
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_UART_RX_MASK                   0x00000008
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_UART_RX_SHIFT                  3
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_UART_TX_MASK                   0x00000004
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_UART_TX_SHIFT                  2
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_PRQ_WATERMARK_MASK             0x00000002
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_PRQ_WATERMARK_SHIFT            1
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_WATCHDOG_TIMER_ATTN_MASK       0x00000001
#    define BCHP_RAAGA_DSP_INTH_HOST_CLEAR_WATCHDOG_TIMER_ATTN_SHIFT      0
#  endif
#endif  /* IS_HOST(SILICON) */

/* The FP2012 console emulation registers.  The RX is Host -> FirePath,
   while TX is FP -> HOST, as a result the names appear "wrong" on the host
   side.  */

/* FIXME: when building in Rockford, we are stuck on a very old commit of the refsw repo,
 * with an old RDB naming scheme. */
#if FEATURE_IS(SW_HOST, RAAGA_ROCKFORD)
#  define RAAGA_DSP_FP2012_DBG_CONSOLE_BASE          BCHP_RAAGA_DSP_FW_CFG_SW_UNDEFINED_SECTION1_i_ARRAY_BASE
#else
#  define RAAGA_DSP_FP2012_DBG_CONSOLE_BASE          BCHP_RAAGA_DSP_FW_CFG_SW_DEBUG_SPAREi_ARRAY_BASE
#endif
#define RAAGA_DSP_FP2012_DBG_CONSOLE_TX_READ_INDEX   (RAAGA_DSP_FP2012_DBG_CONSOLE_BASE + (4 * 0))
#define RAAGA_DSP_FP2012_DBG_CONSOLE_TX_WRITE_INDEX  (RAAGA_DSP_FP2012_DBG_CONSOLE_BASE + (4 * 1))
#define RAAGA_DSP_FP2012_DBG_CONSOLE_RX_READ_INDEX   (RAAGA_DSP_FP2012_DBG_CONSOLE_BASE + (4 * 2))
#define RAAGA_DSP_FP2012_DBG_CONSOLE_RX_WRITE_INDEX  (RAAGA_DSP_FP2012_DBG_CONSOLE_BASE + (4 * 3))
#define RAAGA_DSP_FP2012_DBG_CONSOLE_TX_BUFF         (RAAGA_DSP_FP2012_DBG_CONSOLE_BASE + (4 * 4))
#define RAAGA_DSP_FP2012_DBG_CONSOLE_RX_BUFF         (RAAGA_DSP_FP2012_DBG_CONSOLE_BASE + (4 * 12))
#define RAAGA_DSP_FP2012_DBG_CONSOLE_TX_BUFF_SIZE    (8)
#define RAAGA_DSP_FP2012_DBG_CONSOLE_RX_BUFF_SIZE    (8)
#define RAAGA_DSP_FP2012_DBG_CONSOLE_IRQ             BCHP_RAAGA_DSP_MISC_SCRATCH_0

/* FIFO address calculation macros */
#define RAAGA_FIFO_SIZE             (BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_1 - BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_0)
#define RAAGA_FIFO_ADDR(FIFO_NUM)   (BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_0 + FIFO_NUM * RAAGA_FIFO_SIZE)

/* Target Buffers legacy assigned FIFOs, no more valid */
/* Retained as a default for internal testing use only */
#define TB_RAAGA_LEGACY_FIFO_COREDUMP   17
#define TB_RAAGA_LEGACY_FIFO_PRINT      18
#define TB_RAAGA_LEGACY_FIFO_STATPROF   19
#define TB_RAAGA_LEGACY_FIFO_INSTR      20

/* Basic DSP memory layout */
#define RAAGA_SMEM_START        0x00000000
#define RAAGA_DMEM_START        0x40000000

#if   defined(__FP2008__)
#  define RAAGA_SMEM_SIZE       (128 * 1024)
#  define RAAGA_DMEM_SIZE       (96 * 1024)
#elif defined(__FP2011__)
#  define RAAGA_SMEM_SIZE       (192 * 1024)
#  define RAAGA_DMEM_SIZE       (128 * 1024)
#elif defined(__FP2012__)
#  define RAAGA_SMEM_SIZE       (192 * 1024)
#  define RAAGA_DMEM_SIZE       (128 * 1024)
#endif

#if RAAGA_SMEM_START != 0
#define isSmem(addr, len) ((addr) >= RAAGA_SMEM_START && ((addr)+(len)) <= RAAGA_SMEM_START+RAAGA_SMEM_SIZE)
#else
#define isSmem(addr, len) (((addr)+(len)) <= RAAGA_SMEM_START+RAAGA_SMEM_SIZE)
#endif

#if RAAGA_DMEM_START != 0
#define isDmem(addr, len) ((addr) >= RAAGA_DMEM_START && ((addr)+(len)) < RAAGA_DMEM_START+RAAGA_DMEM_SIZE)
#else
#define isDmem(addr, len) (((addr)+(len)) <= RAAGA_DMEM_START+RAAGA_DMEM_SIZE)
#endif



#if IS_HOST(SILICON)
#include "libdspcontrol/DSP.h"


void* RAAGA_peek(DSP *dsp, void* dest, uint32_t src, size_t size);
uint32_t RAAGA_malloc_physical(DSP *dsp, size_t size);
void RAAGA_free_physical(DSP *dsp, uint32_t phys_p);
#endif


#endif /* RAAGA */

#endif /* _DSP_RAAGA_FP2000_H_ */
