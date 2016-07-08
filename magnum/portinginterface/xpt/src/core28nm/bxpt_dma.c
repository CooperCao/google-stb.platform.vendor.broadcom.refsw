/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bxpt_capabilities.h"
#include "bxpt_playback.h"
#include "bxpt_priv.h"
#include "bxpt_dma.h"

#if BXPT_DMA_HAS_MEMDMA_MCPB
#include "bchp_xpt_memdma_mcpb.h"
#include "bchp_xpt_memdma_mcpb_ch0.h"
#include "bchp_xpt_memdma_mcpb_ch1.h"
#include "bchp_xpt_memdma_mcpb_cpu_intr_aggregator.h"
#include "bchp_int_id_xpt_memdma_mcpb_desc_done_intr_l2.h"
#else
#include "bchp_xpt_mcpb.h"
#include "bchp_xpt_mcpb_ch0.h"
#include "bchp_xpt_mcpb_ch1.h"
#include "bchp_xpt_mcpb_cpu_intr_aggregator.h"
#include "bchp_int_id_xpt_mcpb_desc_done_intr_l2.h"
#endif

#include "bchp_xpt_wdma_regs.h"
#if BXPT_DMA_HAS_WDMA_CHX
#include "bchp_xpt_wdma_ch0.h"
#include "bchp_xpt_wdma_ch1.h"
#else
#include "bchp_xpt_wdma_rams.h"
#endif

#ifndef BCHP_XPT_WDMA_REGS_PM_INTERVAL
#include "bchp_xpt_wdma_pm_control.h"
#include "bchp_xpt_wdma_pm_results.h"
#endif

#include "bchp_xpt_wdma_cpu_intr_aggregator.h"
#include "bchp_xpt_wdma_pci_intr_aggregator.h"
#include "bchp_xpt_bus_if.h"
#include "bchp_int_id_xpt_wdma_desc_done_intr_l2.h"
#include "bchp_int_id_xpt_wdma_overflow_intr_l2.h"
#include "bchp_int_id_xpt_wdma_btp_intr_l2.h"

#include "blst_squeue.h"
#include "blst_slist.h"
#include "bchp_common.h"

#include "bchp_xpt_pmu.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif
#undef BCHP_PWR_RESOURCE_DMA /* no power management for now */

#if (!BXPT_DMA_HAS_WDMA_CHX) /* for backward-compatibility */
#define BCHP_XPT_WDMA_CH0_FIRST_DESC_ADDR        BCHP_XPT_WDMA_RAMS_FIRST_DESC_ADDR
#define BCHP_XPT_WDMA_CH0_NEXT_DESC_ADDR         BCHP_XPT_WDMA_RAMS_NEXT_DESC_ADDR
#define BCHP_XPT_WDMA_CH0_COMPLETED_DESC_ADDRESS BCHP_XPT_WDMA_RAMS_COMPLETED_DESC_ADDRESS
#define BCHP_XPT_WDMA_CH0_BTP_PACKET_GROUP_ID    BCHP_XPT_WDMA_RAMS_BTP_PACKET_GROUP_ID
#define BCHP_XPT_WDMA_CH0_RUN_VERSION_CONFIG     BCHP_XPT_WDMA_RAMS_RUN_VERSION_CONFIG
#define BCHP_XPT_WDMA_CH0_OVERFLOW_REASONS       BCHP_XPT_WDMA_RAMS_OVERFLOW_REASONS
#define BCHP_XPT_WDMA_CH0_DMQ_CONTROL_STRUCT     BCHP_XPT_WDMA_RAMS_DMQ_CONTROL_STRUCT

#define XPT_WDMA_CH0_FIRST_DESC_ADDR        XPT_WDMA_RAMS_FIRST_DESC_ADDR
#define XPT_WDMA_CH0_NEXT_DESC_ADDR         XPT_WDMA_RAMS_NEXT_DESC_ADDR
#define XPT_WDMA_CH0_COMPLETED_DESC_ADDRESS XPT_WDMA_RAMS_COMPLETED_DESC_ADDRESS
#define XPT_WDMA_CH0_BTP_PACKET_GROUP_ID    XPT_WDMA_RAMS_BTP_PACKET_GROUP_ID
#define XPT_WDMA_CH0_RUN_VERSION_CONFIG     XPT_WDMA_RAMS_RUN_VERSION_CONFIG
#define XPT_WDMA_CH0_OVERFLOW_REASONS       XPT_WDMA_RAMS_OVERFLOW_REASONS
#define XPT_WDMA_CH0_DMQ_CONTROL_STRUCT     XPT_WDMA_RAMS_DMQ_CONTROL_STRUCT

#define BCHP_XPT_WDMA_CH0_RUN_VERSION_CONFIG_RUN_VERSION_MASK  BCHP_XPT_WDMA_RAMS_RUN_VERSION_CONFIG_RUN_VERSION_MASK
#define BCHP_XPT_WDMA_CH0_RUN_VERSION_CONFIG_RUN_VERSION_SHIFT BCHP_XPT_WDMA_RAMS_RUN_VERSION_CONFIG_RUN_VERSION_SHIFT
#endif

#if (!BXPT_DMA_HAS_MEMDMA_MCPB)
#define BCHP_XPT_MEMDMA_MCPB_RUN_SET_CLEAR                          BCHP_XPT_MCPB_RUN_SET_CLEAR
#define BCHP_XPT_MEMDMA_MCPB_WAKE_SET                               BCHP_XPT_MCPB_WAKE_SET
#define BCHP_XPT_MEMDMA_MCPB_PAUSE_AT_DESC_READ_CLEAR               BCHP_XPT_MCPB_PAUSE_AT_DESC_READ_CLEAR
#define BCHP_XPT_MEMDMA_MCPB_PAUSE_AT_DESC_END_CLEAR                BCHP_XPT_MCPB_PAUSE_AT_DESC_END_CLEAR
#define BCHP_XPT_MEMDMA_MCPB_PAUSE_AFTER_GROUP_PACKETS_CLEAR        BCHP_XPT_MCPB_PAUSE_AFTER_GROUP_PACKETS_CLEAR
#define BCHP_XPT_MEMDMA_MCPB_CRC_COMPARE_ERROR_PAUSE_CLEAR          BCHP_XPT_MCPB_CRC_COMPARE_ERROR_PAUSE_CLEAR
#define BCHP_XPT_MEMDMA_MCPB_MICRO_PAUSE_SET_CLEAR                  BCHP_XPT_MCPB_MICRO_PAUSE_SET_CLEAR
#define BCHP_XPT_MEMDMA_MCPB_RUN_STATUS_0_31                        BCHP_XPT_MCPB_RUN_STATUS_0_31
#define BCHP_XPT_MEMDMA_MCPB_WAKE_STATUS_0_31                       BCHP_XPT_MCPB_WAKE_STATUS_0_31
#define BCHP_XPT_MEMDMA_MCPB_PAUSE_AT_DESC_READ_STATUS_0_31         BCHP_XPT_MCPB_PAUSE_AT_DESC_READ_STATUS_0_31
#define BCHP_XPT_MEMDMA_MCPB_PAUSE_AT_DESC_END_STATUS_0_31          BCHP_XPT_MCPB_PAUSE_AT_DESC_END_STATUS_0_31
#define BCHP_XPT_MEMDMA_MCPB_PAUSE_AFTER_GROUP_PACKETS_STATUS_0_31  BCHP_XPT_MCPB_PAUSE_AFTER_GROUP_PACKETS_STATUS_0_31
#define BCHP_XPT_MEMDMA_MCPB_CRC_COMPARE_ERROR_PAUSE_STATUS_0_31    BCHP_XPT_MCPB_CRC_COMPARE_ERROR_PAUSE_STATUS_0_31
#define BCHP_XPT_MEMDMA_MCPB_MICRO_PAUSE_STATUS_0_31                BCHP_XPT_MCPB_MICRO_PAUSE_STATUS_0_31
#define BCHP_XPT_MEMDMA_MCPB_HW_PAUSE_STATUS_0_31                   BCHP_XPT_MCPB_HW_PAUSE_STATUS_0_31
#define BCHP_XPT_MEMDMA_MCPB_DESC_RD_IN_PROGRESS_0_31               BCHP_XPT_MCPB_DESC_RD_IN_PROGRESS_0_31
#define BCHP_XPT_MEMDMA_MCPB_DATA_RD_IN_PROGRESS_0_31               BCHP_XPT_MCPB_DATA_RD_IN_PROGRESS_0_31
#define BCHP_XPT_MEMDMA_MCPB_LAST_DESC_REACHED_0_31                 BCHP_XPT_MCPB_LAST_DESC_REACHED_0_31
#define BCHP_XPT_MEMDMA_MCPB_BUFF_DATA_RDY_0_31                     BCHP_XPT_MCPB_BUFF_DATA_RDY_0_31

#define XPT_MEMDMA_MCPB_RUN_SET_CLEAR                           XPT_MCPB_RUN_SET_CLEAR
#define XPT_MEMDMA_MCPB_WAKE_SET                                XPT_MCPB_WAKE_SET
#define XPT_MEMDMA_MCPB_PAUSE_AT_DESC_READ_CLEAR                XPT_MCPB_PAUSE_AT_DESC_READ_CLEAR
#define XPT_MEMDMA_MCPB_PAUSE_AT_DESC_END_CLEAR                 XPT_MCPB_PAUSE_AT_DESC_END_CLEAR
#define XPT_MEMDMA_MCPB_PAUSE_AFTER_GROUP_PACKETS_CLEAR         XPT_MCPB_PAUSE_AFTER_GROUP_PACKETS_CLEAR
#define XPT_MEMDMA_MCPB_CRC_COMPARE_ERROR_PAUSE_CLEAR           XPT_MCPB_CRC_COMPARE_ERROR_PAUSE_CLEAR
#define XPT_MEMDMA_MCPB_MICRO_PAUSE_SET_CLEAR                   XPT_MCPB_MICRO_PAUSE_SET_CLEAR
#define XPT_MEMDMA_MCPB_RUN_STATUS_0_31                         XPT_MCPB_RUN_STATUS_0_31
#define XPT_MEMDMA_MCPB_WAKE_STATUS_0_31                        XPT_MCPB_WAKE_STATUS_0_31
#define XPT_MEMDMA_MCPB_PAUSE_AT_DESC_READ_STATUS_0_31          XPT_MCPB_PAUSE_AT_DESC_READ_STATUS_0_31
#define XPT_MEMDMA_MCPB_PAUSE_AT_DESC_END_STATUS_0_31           XPT_MCPB_PAUSE_AT_DESC_END_STATUS_0_31
#define XPT_MEMDMA_MCPB_PAUSE_AFTER_GROUP_PACKETS_STATUS_0_31   XPT_MCPB_PAUSE_AFTER_GROUP_PACKETS_STATUS_0_31
#define XPT_MEMDMA_MCPB_CRC_COMPARE_ERROR_PAUSE_STATUS_0_31     XPT_MCPB_CRC_COMPARE_ERROR_PAUSE_STATUS_0_31
#define XPT_MEMDMA_MCPB_MICRO_PAUSE_STATUS_0_31                 XPT_MCPB_MICRO_PAUSE_STATUS_0_31
#define XPT_MEMDMA_MCPB_HW_PAUSE_STATUS_0_31                    XPT_MCPB_HW_PAUSE_STATUS_0_31
#define XPT_MEMDMA_MCPB_DESC_RD_IN_PROGRESS_0_31                XPT_MCPB_DESC_RD_IN_PROGRESS_0_31
#define XPT_MEMDMA_MCPB_DATA_RD_IN_PROGRESS_0_31                XPT_MCPB_DATA_RD_IN_PROGRESS_0_31
#define XPT_MEMDMA_MCPB_LAST_DESC_REACHED_0_31                  XPT_MCPB_LAST_DESC_REACHED_0_31
#define XPT_MEMDMA_MCPB_BUFF_DATA_RDY_0_31                      XPT_MCPB_BUFF_DATA_RDY_0_31

#define BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DESC_CONTROL               BCHP_XPT_MCPB_CH0_DMA_DESC_CONTROL
#define BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_CONTROL               BCHP_XPT_MCPB_CH1_DMA_DESC_CONTROL
#define BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL               BCHP_XPT_MCPB_CH0_DMA_DATA_CONTROL
#define BCHP_XPT_MEMDMA_MCPB_CH0_DMA_CURR_DESC_ADDRESS          BCHP_XPT_MCPB_CH0_DMA_CURR_DESC_ADDRESS
#define BCHP_XPT_MEMDMA_MCPB_CH0_DMA_NEXT_DESC_ADDRESS          BCHP_XPT_MCPB_CH0_DMA_NEXT_DESC_ADDRESS
#define BCHP_XPT_MEMDMA_MCPB_CH0_SP_PKT_LEN                     BCHP_XPT_MCPB_CH0_SP_PKT_LEN
#define BCHP_XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL                 BCHP_XPT_MCPB_CH0_SP_PARSER_CTRL
#define BCHP_XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL1                BCHP_XPT_MCPB_CH0_SP_PARSER_CTRL1
#define BCHP_XPT_MEMDMA_MCPB_CH0_SP_TS_CONFIG                   BCHP_XPT_MCPB_CH0_SP_TS_CONFIG
#define BCHP_XPT_MEMDMA_MCPB_CH0_SP_PES_ES_CONFIG               BCHP_XPT_MCPB_CH0_SP_PES_ES_CONFIG
#define BCHP_XPT_MEMDMA_MCPB_CH0_SP_PES_SYNC_COUNTER            BCHP_XPT_MCPB_CH0_SP_PES_SYNC_COUNTER
#define BCHP_XPT_MEMDMA_MCPB_CH0_DMA_BBUFF_CTRL                 BCHP_XPT_MCPB_CH0_DMA_BBUFF_CTRL
#define BCHP_XPT_MEMDMA_MCPB_CH0_DMA_BBUFF_CRC                  BCHP_XPT_MCPB_CH0_DMA_BBUFF_CRC
#define BCHP_XPT_MEMDMA_MCPB_CH0_TMEU_BLOCKOUT_CTRL             BCHP_XPT_MCPB_CH0_TMEU_BLOCKOUT_CTRL
#define BCHP_XPT_MEMDMA_MCPB_CH0_TMEU_TIMING_CTRL               BCHP_XPT_MCPB_CH0_TMEU_TIMING_CTRL
#define BCHP_XPT_MEMDMA_MCPB_CH0_TMEU_TS_ERR_BOUND_EARLY        BCHP_XPT_MCPB_CH0_TMEU_TS_ERR_BOUND_EARLY
#define BCHP_XPT_MEMDMA_MCPB_CH0_TMEU_TS_ERR_BOUND_LATE         BCHP_XPT_MCPB_CH0_TMEU_TS_ERR_BOUND_LATE
#define BCHP_XPT_MEMDMA_MCPB_CH0_TMEU_PES_PACING_CTRL           BCHP_XPT_MCPB_CH0_TMEU_PES_PACING_CTRL
#define BCHP_XPT_MEMDMA_MCPB_CH0_TMEU_TIMING_INFO_LAST_TIMESTAMP_DELTA BCHP_XPT_MCPB_CH0_TMEU_TIMING_INFO_LAST_TIMESTAMP_DELTA
#define BCHP_XPT_MEMDMA_MCPB_CH0_TMEU_TIMING_INFO_LAST_NEXT_TIMESTAMP  BCHP_XPT_MCPB_CH0_TMEU_TIMING_INFO_LAST_NEXT_TIMESTAMP
#define BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_STATUS                    BCHP_XPT_MCPB_CH0_DCPM_STATUS
#define BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_DESC_ADDR                 BCHP_XPT_MCPB_CH0_DCPM_DESC_ADDR
#define BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_DESC_DONE_INT_ADDR        BCHP_XPT_MCPB_CH0_DCPM_DESC_DONE_INT_ADDR
#define BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_PAUSE_AFTER_GROUP_PACKETS_CTRL BCHP_XPT_MCPB_CH0_DCPM_PAUSE_AFTER_GROUP_PACKETS_CTRL
#define BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER      BCHP_XPT_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER
#define BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_DATA_ADDR_UPPER           BCHP_XPT_MCPB_CH0_DCPM_DATA_ADDR_UPPER
#define BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_DATA_ADDR_LOWER           BCHP_XPT_MCPB_CH0_DCPM_DATA_ADDR_LOWER

#define XPT_MEMDMA_MCPB_CH0_DMA_DESC_CONTROL            XPT_MCPB_CH0_DMA_DESC_CONTROL
#define XPT_MEMDMA_MCPB_CH1_DMA_DESC_CONTROL            XPT_MCPB_CH1_DMA_DESC_CONTROL
#define XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL            XPT_MCPB_CH0_DMA_DATA_CONTROL
#define XPT_MEMDMA_MCPB_CH0_DMA_CURR_DESC_ADDRESS       XPT_MCPB_CH0_DMA_CURR_DESC_ADDRESS
#define XPT_MEMDMA_MCPB_CH0_DMA_NEXT_DESC_ADDRESS       XPT_MCPB_CH0_DMA_NEXT_DESC_ADDRESS
#define XPT_MEMDMA_MCPB_CH0_SP_PKT_LEN                  XPT_MCPB_CH0_SP_PKT_LEN
#define XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL              XPT_MCPB_CH0_SP_PARSER_CTRL
#define XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL1             XPT_MCPB_CH0_SP_PARSER_CTRL1
#define XPT_MEMDMA_MCPB_CH0_SP_TS_CONFIG                XPT_MCPB_CH0_SP_TS_CONFIG
#define XPT_MEMDMA_MCPB_CH0_SP_PES_ES_CONFIG            XPT_MCPB_CH0_SP_PES_ES_CONFIG
#define XPT_MEMDMA_MCPB_CH0_SP_PES_SYNC_COUNTER         XPT_MCPB_CH0_SP_PES_SYNC_COUNTER
#define XPT_MEMDMA_MCPB_CH0_DMA_BBUFF_CTRL              XPT_MCPB_CH0_DMA_BBUFF_CTRL
#define XPT_MEMDMA_MCPB_CH0_DMA_BBUFF_CRC               XPT_MCPB_CH0_DMA_BBUFF_CRC
#define XPT_MEMDMA_MCPB_CH0_TMEU_BLOCKOUT_CTRL          XPT_MCPB_CH0_TMEU_BLOCKOUT_CTRL
#define XPT_MEMDMA_MCPB_CH0_TMEU_TIMING_CTRL            XPT_MCPB_CH0_TMEU_TIMING_CTRL
#define XPT_MEMDMA_MCPB_CH0_TMEU_TS_ERR_BOUND_EARLY     XPT_MCPB_CH0_TMEU_TS_ERR_BOUND_EARLY
#define XPT_MEMDMA_MCPB_CH0_TMEU_TS_ERR_BOUND_LATE      XPT_MCPB_CH0_TMEU_TS_ERR_BOUND_LATE
#define XPT_MEMDMA_MCPB_CH0_TMEU_PES_PACING_CTRL        XPT_MCPB_CH0_TMEU_PES_PACING_CTRL
#define XPT_MEMDMA_MCPB_CH0_TMEU_TIMING_INFO_LAST_TIMESTAMP_DELTA XPT_MCPB_CH0_TMEU_TIMING_INFO_LAST_TIMESTAMP_DELTA
#define XPT_MEMDMA_MCPB_CH0_TMEU_TIMING_INFO_LAST_NEXT_TIMESTAMP  XPT_MCPB_CH0_TMEU_TIMING_INFO_LAST_NEXT_TIMESTAMP
#define XPT_MEMDMA_MCPB_CH0_DCPM_STATUS                 XPT_MCPB_CH0_DCPM_STATUS
#define XPT_MEMDMA_MCPB_CH0_DCPM_DESC_ADDR              XPT_MCPB_CH0_DCPM_DESC_ADDR
#define XPT_MEMDMA_MCPB_CH0_DCPM_DESC_DONE_INT_ADDR     XPT_MCPB_CH0_DCPM_DESC_DONE_INT_ADDR
#define XPT_MEMDMA_MCPB_CH0_DCPM_PAUSE_AFTER_GROUP_PACKETS_CTRL XPT_MCPB_CH0_DCPM_PAUSE_AFTER_GROUP_PACKETS_CTRL
#define XPT_MEMDMA_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER   XPT_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER
#define XPT_MEMDMA_MCPB_CH0_DCPM_DATA_ADDR_UPPER        XPT_MCPB_CH0_DCPM_DATA_ADDR_UPPER
#define XPT_MEMDMA_MCPB_CH0_DCPM_DATA_ADDR_LOWER        XPT_MCPB_CH0_DCPM_DATA_ADDR_LOWER

#define BCHP_XPT_MEMDMA_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_STATUS      BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_STATUS
#define BCHP_XPT_MEMDMA_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS
#define BCHP_XPT_MEMDMA_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET    BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET
#define BCHP_XPT_MEMDMA_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR  BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR

#define XPT_MEMDMA_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_STATUS      XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_STATUS
#define XPT_MEMDMA_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS
#define XPT_MEMDMA_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET    XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET
#define XPT_MEMDMA_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR  XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR
#endif

BDBG_MODULE(bxpt_dma);
BDBG_OBJECT_ID(BXPT_Dma_Handle_Tag);
BDBG_OBJECT_ID(BXPT_Dma_Context);

#define BXPT_DMA_NUM_CHANNELS   BXPT_NUM_PLAYBACKS
#define BXPT_DMA_BAND_ID_OFFSET BXPT_NUM_PLAYBACKS

#define BXPT_DMA_MCPB_DESC_WORDS 8
#define BXPT_DMA_WDMA_DESC_WORDS 4

/* all sizes in bytes */
#define BXPT_DMA_MCPB_DESC_SIZE    (BXPT_DMA_MCPB_DESC_WORDS*4) /* requires 32-byte alignment (eight word descriptor format) */
#define BXPT_DMA_WDMA_DESC_SIZE    (BXPT_DMA_WDMA_DESC_WORDS*4) /* requires 16-byte alignment */
#define BXPT_DMA_MAX_TRANSFER_SIZE (UINT32_C(0xFFFFFFFF)) /* 1 word */

#define BXPT_DMA_MCPB_LLD_TYPE 0 /* 8-word descriptor format */

#define BXPT_DMA_USE_40BIT_ADDRESSES 1
#if BXPT_DMA_USE_40BIT_ADDRESSES
#define BXPT_DMA_ADDRESS_LIMIT_SHIFT (40)
#else
#define BXPT_DMA_ADDRESS_LIMIT_SHIFT (32)
#endif
#define BXPT_DMA_PWR_BO_COUNT      0x3fffffff
#define BXPT_DMA_DEF_BO_COUNT      0

#define UINT32_V(value) ((uint32_t)(value & 0xFFFFFFFF))

/* RUN_VERSION (aka SEQUENCE_ID) */
#define BXPT_DMA_MAX_RUN_VERSION (BCHP_XPT_WDMA_CH0_RUN_VERSION_CONFIG_RUN_VERSION_MASK >> BCHP_XPT_WDMA_CH0_RUN_VERSION_CONFIG_RUN_VERSION_SHIFT)
#define BXPT_DMA_USE_RUN_VERSION 1

#define BXPT_DMA_DESC_PROTECT      1 /* run-time check for transfers that corrupt descriptor memory location */

#define BXPT_DMA_RESERVE_SAGE_CH 1 /* hard-coded enabled for now */
#define BXPT_DMA_SAGE_CH_NUM     (BXPT_DMA_NUM_CHANNELS-1)

#define BXPT_DMA_NUM_FRONT_SHARED_CH 2 /* assume channels 0 and 1 are shared with others */

#if ((BCHP_CHIP==7445 && BCHP_VER<BCHP_VER_C0) || (BCHP_CHIP==7145 && BCHP_VER<BCHP_VER_B0))
#define BXPT_DMA_WDMA_END_INVERT 1 /* SW7445-96 */
#endif

#if ((BCHP_CHIP==7445 && BCHP_VER<BCHP_VER_C0) || (BCHP_CHIP==7145 && BCHP_VER<BCHP_VER_B0))
#define BXPT_DMA_PREVENT_4BYTE_TRANSFERS 1 /* SW7445-97 */
#endif

#if ((BCHP_CHIP==7445 && BCHP_VER<BCHP_VER_D0) || (BCHP_CHIP==7145 && BCHP_VER<BCHP_VER_B0) || \
     (BCHP_CHIP==7439 && BCHP_VER<BCHP_VER_B0) || (BCHP_CHIP==7366 && BCHP_VER<BCHP_VER_B0) || \
     (BCHP_CHIP==74371 && BCHP_VER<BCHP_VER_B0) )
/* SW7445-198 */
#define BXPT_DMA_SEPARATE_MEMC_FOR_DESC 1
#define BXPT_DMA_LIMIT_TO_SINGLE_CH     1
#endif

#if ((BCHP_CHIP==7445 && BCHP_VER<BCHP_VER_E0) || (BCHP_CHIP==7145 && BCHP_VER<BCHP_VER_C0) || \
     (BCHP_CHIP==7439 && BCHP_VER<BCHP_VER_B0) || (BCHP_CHIP==7366 && BCHP_VER<BCHP_VER_B0) || \
     (BCHP_CHIP==74371 && BCHP_VER<BCHP_VER_B0) )
/* SW7445-941 */
#define BXPT_DMA_WDMA_DESC_CONVERT 1
#if BXPT_DMA_WDMA_DESC_CONVERT
#define BXPT_DMA_AVOID_WAKE        1 /* avoids doing WAKE completely */
#endif
#endif

/* TODO: #if for affected platforms */
#define BXPT_DMA_WDMA_DESC_APPEND_NULL 1 /* CRXPT-985 */

/* TODO: #if for affected platforms */
#if (!BXPT_DMA_HAS_MEMDMA_MCPB)
#define BXPT_DMA_WDMA_ENDIAN_SWAP_CBIT 1
#endif

#define BXPT_DMA_DISABLE_WAKE_ON_OVERLAP 1

/* MCPB descriptor flags (for eight word descriptor format) */
#define MCPB_DW2_LAST_DESC                (1 << 0)

#define MCPB_DW4_INT_EN                   (1 << 31)
#define MCPB_DW4_FORCE_RESYNC             (1 << 30)
#define MCPB_DW4_HOST_DATA_INS_EN         (1 << 29)
#define MCPB_DW4_PUSH_PARTIAL_PKT         (1 << 28)
#define MCPB_DW4_PUSH_PREV_PARTIAL_PKT    (1 << 27)
#define MCPB_DW4_PAUSE_AT_DESC_RD         (1 << 26)
#define MCPB_DW4_DESC_PARSER_SEL          (1 << 9)
#define MCPB_DW4_DESC_BAND_ID_EN          (1 << 8)

#define MCPB_DW5_HOST_INS_DATA_AS_BTP_PKT (1 << 22)
#define MCPB_DW5_RD_ENDIAN_STRAP_INV_CTRL (1 << 21)
#define MCPB_DW5_PAUSE_AT_DESC_END        (1 << 20)
#define MCPB_DW5_PID_CHANNEL_VALID        (1 << 19)
#define MCPB_DW5_SCRAM_END                (1 << 18)
#define MCPB_DW5_SCRAM_START              (1 << 17)
#define MCPB_DW5_SCRAM_INIT               (1 << 16)

#if BXPT_DMA_HAS_WDMA_CHX
#define MCPB_DW5_DATA_UNIT_SIZE_SHIFT     (14) /* size: 2 bits */
#undef MCPB_DW5_RD_ENDIAN_STRAP_INV_CTRL /* no longer valid */
#endif

/* WDMA descriptor flags */
#define WDMA_DW3_DATA_UNIT_SIZE_SHIFT     (2) /* size: 2 bits. was called ENDIAN_SWAP_SHIFT */
#define WDMA_DW3_INTR_EN                  (1 << 1)
#define WDMA_DW3_LAST                     (1 << 0)

/* common desc field */
#define DESC_DATA_UNIT_SIZE_8        0
#define DESC_DATA_UNIT_SIZE_16       1
#define DESC_DATA_UNIT_SIZE_32       2

/* debug functionality */
#define BXPT_DMA_DEBUG_MODE          0 /* extra debug */
#define BXPT_DMA_MCPB_DESC_DONE_IRQ  0
#define BXPT_DMA_MCPB_FALSE_WAKE_IRQ 0
#define BXPT_DMA_MSA_DEBUG_MODE      0
#define BXPT_DMA_TRACE_MSGS          0 /* BDBG_MSG_TRACE(x) -> BDBG_MSG(x) */
#define BXPT_DMA_DESC_DUMP           0 /* dump descriptors */

#define BXPT_DMA_RATE_TEST           0

#if BXPT_DMA_RATE_TEST
#include "bchp_xpt_mcpb_ch0.h"
#include "bchp_int_id_xpt_mcpb_desc_done_intr_l2.h" /* TODO: this header has been removed */
#undef BXPT_DMA_MCPB_DESC_DONE_IRQ
#define BXPT_DMA_MCPB_DESC_DONE_IRQ 1

#include <time.h>
typedef struct timespec BXPT_Dma_Time;
#endif

#if BXPT_DMA_MCPB_FALSE_WAKE_IRQ
#include "bchp_int_id_xpt_memdma_mcpb_misc_false_wake_intr_l2.h" /* TODO: this header has been removed */
#endif

#if BXPT_DMA_TRACE_MSGS
#define BDBG_MSG_TRACE(x) BDBG_MSG(x)
#else
#define BDBG_MSG_TRACE(x)
#endif

/*
Naming changes from MMD:

MMD -> XPT_Dma
BMMD_Handle -> BXPT_Dma_Handle (not BXPT_Dma_ChannelHandle)
BMMD_Open   -> BXPT_Dma_OpenChannel (similar to BXPT_Playback_OpenChannel)
BMMD_ContextHandle -> BXPT_Dma_ContextHandle
*/

typedef struct BXPT_Dma_Context_P_Offsets {
    uint64_t src, dst;
    uint32_t size; /* for WDMA_DESC_CONVERT */
} BXPT_Dma_Context_P_Offsets;

typedef struct BXPT_Dma_Context
{
    BDBG_OBJECT(BXPT_Dma_Context)
    BXPT_Dma_Handle parent;

    /* descriptor prototypes */
    uint32_t mcpbDescW4, mcpbDescW5, mcpbDescW6;
    uint32_t wdmaDescW3;

    BMMA_Block_Handle mcpbBlock[2], wdmaBlock[2];
    BMMA_DeviceOffset firstMcpbDescOffset[2], firstWdmaDescOffset[2];

    /* pingpong scheme: given descriptor memory blocks A and B, we go through a sequence of
       write A -> start A -> complete A -> write B -> start B -> complete B -> write A ... */
    void *firstMcpbDescCached[2], *firstWdmaDescCached[2]; /* cached pointer to first descriptor */
    /* note that these pointers cannot be calculated using numBlocks/numBlockWdma, unless we maintain ping/pong for those variables as well */
    uint32_t *lastMcpbDescCached[2], *lastWdmaDescCached[2]; /* cached pointer to last descriptor. used for linking */

    unsigned pp; /* updated when descriptor completes */
    unsigned lpp; /* updated when HW is started */

    BXPT_Dma_ContextSettings settings;

    BLST_S_ENTRY(BXPT_Dma_Context) ctxNode;
    BLST_SQ_ENTRY(BXPT_Dma_Context) activeNode;
    enum {
        ctx_state_eIdle,
        ctx_state_eQueued,   /* ctx is queued in activeNode and hardware */
        ctx_state_eDestroyed /* Context_Destroy() was called, but destroy deferred */
    } state;

    unsigned dmaLength; /* total length of last transaction */
    unsigned numBlocks; /* total blocks of last transaction */

#if (BXPT_DMA_WDMA_DESC_CONVERT || BXPT_DMA_WDMA_DESC_APPEND_NULL)
    unsigned numBlocksWdma;
#endif
#if BXPT_DMA_WDMA_DESC_CONVERT
    BXPT_Dma_Context_P_Offsets *pOffsets; /* used as scratchpad */
    BXPT_Dma_Context_P_Offsets *pOffsetsWdma;
#endif

    unsigned cntStarted, cntCompleted, cntRun; /* cntStarted - cntRun = cntWake */
    uint32_t tmpDebug;
} BXPT_Dma_Context;

#define MCPB_DESC_OFFSET_LOCK(ctx, idx) (ctx->firstMcpbDescOffset[idx] = BMMA_LockOffset(ctx->mcpbBlock[idx]))
#define MCPB_DESC_OFFSET_UNLOCK(ctx, idx) (BMMA_UnlockOffset(ctx->mcpbBlock[idx], ctx->firstMcpbDescOffset[idx]))
#define WDMA_DESC_OFFSET_LOCK(ctx, idx) (ctx->firstWdmaDescOffset[idx] = BMMA_LockOffset(ctx->wdmaBlock[idx]))
#define WDMA_DESC_OFFSET_UNLOCK(ctx, idx) (BMMA_UnlockOffset(ctx->wdmaBlock[idx], ctx->firstWdmaDescOffset[idx]))

#define MCPB_DESC_CACHED_LOCK(ctx, idx) (ctx->firstMcpbDescCached[idx] = BMMA_Lock(ctx->mcpbBlock[idx]))
#define MCPB_DESC_CACHED_UNLOCK(ctx, idx) (BMMA_Unlock(ctx->mcpbBlock[idx], ctx->firstMcpbDescCached[idx]))
#define WDMA_DESC_CACHED_LOCK(ctx, idx) (ctx->firstWdmaDescCached[idx] = BMMA_Lock(ctx->wdmaBlock[idx]))
#define WDMA_DESC_CACHED_UNLOCK(ctx, idx) (BMMA_Unlock(ctx->wdmaBlock[idx], ctx->firstWdmaDescCached[idx]))

#define FIRST_MCPB_DESC_OFFSET(ctx) (ctx->firstMcpbDescOffset[ctx->pp])
#define FIRST_WDMA_DESC_OFFSET(ctx) (ctx->firstWdmaDescOffset[ctx->pp])
#define LAST_MCPB_DESC_OFFSET(ctx)  (ctx->firstMcpbDescOffset[ctx->pp] + ((ctx->numBlocks-1) * BXPT_DMA_MCPB_DESC_SIZE))
#define LAST_WDMA_DESC_OFFSET(ctx)  (ctx->firstWdmaDescOffset[ctx->pp] + ((ctx->numBlocks-1) * BXPT_DMA_WDMA_DESC_SIZE))

#if (BXPT_DMA_WDMA_DESC_CONVERT || BXPT_DMA_WDMA_DESC_APPEND_NULL)
#undef LAST_WDMA_DESC_OFFSET
#define LAST_WDMA_DESC_OFFSET(ctx)  (ctx->firstWdmaDescOffset[ctx->pp] + ((ctx->numBlocksWdma-1) * BXPT_DMA_WDMA_DESC_SIZE))
#endif

#define FIRST_MCPB_DESC_CACHED(ctx) (ctx->firstMcpbDescCached[ctx->pp])
#define FIRST_WDMA_DESC_CACHED(ctx) (ctx->firstWdmaDescCached[ctx->pp])
#define LAST_MCPB_DESC_CACHED_START(ctx) (ctx->lastMcpbDescCached[ctx->lpp])
#define LAST_WDMA_DESC_CACHED_START(ctx) (ctx->lastWdmaDescCached[ctx->lpp])

#if 0 /* see comment above lastMcpbDescCached/lastWdmaDescCached*/
#define LAST_MCPB_DESC_CACHED_START(ctx) ( (uint32_t*) ((uint8_t*)ctx->firstMcpbDescCached[ctx->lpp] + ((ctx->numBlocks-1) * BXPT_DMA_MCPB_DESC_SIZE)) )
#define LAST_WDMA_DESC_CACHED_START(ctx) ( (uint32_t*) ((uint8_t*)ctx->firstWdmaDescCached[ctx->lpp] + ((ctx->numBlocks-1) * BXPT_DMA_WDMA_DESC_SIZE)) )

#if (BXPT_DMA_WDMA_DESC_CONVERT || BXPT_DMA_WDMA_DESC_APPEND_NULL)
#undef LAST_WDMA_DESC_CACHED_START
#define LAST_WDMA_DESC_CACHED_START(ctx) ( (uint32_t*) ((uint8_t*)ctx->firstWdmaDescCached[ctx->lpp] + ((ctx->numBlocksWdma-1) * BXPT_DMA_WDMA_DESC_SIZE)) )
#endif
#endif

/* context identifier */
#define CTX_ID(ctx) (ctx?ctx->firstMcpbDescOffset[0]:0)

typedef struct BXPT_Dma_Handle_Tag
{
    BDBG_OBJECT(BXPT_Dma_Handle_Tag)
    BXPT_Handle xpt;
    /* store our own */
    BCHP_Handle chp;
    BREG_Handle reg;
    BINT_Handle bint;
    BMMA_Heap_Handle mmaHeap;

    BINT_CallbackHandle irqDescDone, irqOverflow, irqBtp;
    BINT_CallbackHandle irqMcpbFalseWake, irqMcpbDescDone; /* for debug only */

    /* per-channel register offsets */
    uint32_t mcpbRegOffset;
    uint32_t wdmaRamOffset; /* used with XPT_WDMA_CHX. do not use with XPT_WDMA_REGS */

    unsigned channelNum;
    BXPT_Dma_Settings settings;
    BXPT_Dma_ContextHandle lastQueuedCtx;
    BLST_S_HEAD(CtxList, BXPT_Dma_Context) ctxList;
    BLST_SQ_HEAD(ActiveCtxList, BXPT_Dma_Context) activeCtxList;
    bool standby;

    unsigned cntStarted, cntCompleted, cntRun;
    unsigned cntMcpbFalseWake;

    bool wakeDisabled;

#if BXPT_DMA_RATE_TEST
    BXPT_Dma_Time startTime, mcpbEndTime, wdmaEndTime;
    bool usePbMcpb; /* use XPT_MCPB_CHx registers, not XPT_MEMDMA_MCPB_CHx registers */
#endif
} BXPT_Dma_Handle_Tag;

#if BXPT_DMA_MSA_DEBUG_MODE
#include "bchp_memc_gen_0.h"
#include "bchp_memc_gen_1.h"
#define MSA_CMD_LINEAR_1JW_READ  0x01
#define MSA_NUM_DATA_REGS 8
#define MSA_1_REG_OFFSET (BCHP_MEMC_GEN_1_MSA_CMD_TYPE - BCHP_MEMC_GEN_0_MSA_CMD_TYPE)

static void writeMsaCommand(BREG_Handle reg, unsigned cmd, uint32_t addr, uint32_t regoffset)
{
    while ((BREG_Read32(reg, BCHP_MEMC_GEN_0_MSA_STATUS + regoffset) & 0x2) == 0x0); /* wait for T_LOCK=1 */

    BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_CMD_TYPE + regoffset, cmd);
    BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_CMD_ADDR + regoffset, addr);

    while ((BREG_Read32(reg, BCHP_MEMC_GEN_0_MSA_STATUS + regoffset) & 0x1) == 0x1); /* wait for BUSY=0 */
    return;
}

static void readMsaData(BREG_Handle reg, uint32_t addr, uint32_t* read_array, unsigned num, unsigned start)
{
    unsigned i;
    uint32_t regoffset = addr < 0x40000000 ? 0 : MSA_1_REG_OFFSET;

    BDBG_ASSERT((addr & 0x1F) == 0); /* must be 8-word aligned */
    BDBG_ASSERT(num+start<=MSA_NUM_DATA_REGS);
    writeMsaCommand(reg, MSA_CMD_LINEAR_1JW_READ, addr >> 3, regoffset); /* memc wants an o-word (8-byte word) */

    for (i=0; i<num; i++) {
        read_array[i] = BREG_Read32(reg, (BCHP_MEMC_GEN_0_MSA_RD_DATA0 + regoffset) + (MSA_NUM_DATA_REGS-1-(i+start))*sizeof(uint32_t));
    }
    return;
}

static int dataCompare(uint32_t *cached, uint32_t *msa, unsigned numWords)
{
    unsigned i;
    BDBG_ASSERT(numWords<=MSA_NUM_DATA_REGS);
    for (i=0; i<numWords; i++) {
        if (cached[i] != msa[i]) {
            BDBG_ERR(("cached != MSA mismatch"));
            for (i=0; i<numWords; i++) {
                BDBG_ERR(("%08x %08x", cached[i], msa[i]));
            }
            return -1;
        }
    }
    return 0;
}

static void BXPT_Dma_P_AssertMcpbDescMem(BXPT_Dma_ContextHandle ctx, bool checkLastOnly /* if false, check all descriptors */)
{
    uint32_t msaData[MSA_NUM_DATA_REGS], *cached, offset;
    unsigned i, numWords = 8;
    unsigned start = checkLastOnly ? ctx->numBlocks-1 : 0;

    /* MCPB */
    for (i=start; i<ctx->numBlocks; i++) {
        offset = FIRST_MCPB_DESC_OFFSET(ctx) + i*BXPT_DMA_MCPB_DESC_SIZE;
        readMsaData(ctx->parent->reg, offset, msaData, numWords, 0);

        cached = FIRST_MCPB_DESC_CACHED(ctx);
        cached += i*numWords;
        if (dataCompare(cached, msaData, numWords)) {
            BDBG_ERR(("" BDBG_UINT64_FMT ": mem error in MCPB desc %u/%u", BDBG_UINT64_ARG(CTX_ID(ctx)), i, ctx->numBlocks));
            BDBG_ASSERT(0);
        }
    }
}

static void BXPT_Dma_P_AssertWdmaDescMem(BXPT_Dma_ContextHandle ctx, bool checkLastOnly /* if false, check all descriptors */)
{
    uint32_t msaData[MSA_NUM_DATA_REGS], *cached, offset;
    unsigned i, numWords = 4;
    unsigned start = checkLastOnly ? ctx->numBlocks-1 : 0;
    unsigned msaStart;

    /* WDMA */
    for (i=start; i<ctx->numBlocks; i++) {
        offset = FIRST_WDMA_DESC_OFFSET(ctx) + i*BXPT_DMA_WDMA_DESC_SIZE;

        /* WDMA descriptors are 4-word long, so every other desc is not 8-word aligned */
        msaStart = 0;
        if (offset & 0x1F) {
            offset -= BXPT_DMA_WDMA_DESC_SIZE;
            msaStart = 4;
        }
        readMsaData(ctx->parent->reg, offset, msaData, numWords, msaStart);

        cached = FIRST_WDMA_DESC_CACHED(ctx);
        cached += i*numWords;
        if (dataCompare(cached, msaData, numWords)) {
            BDBG_ERR(("" BDBG_UINT64_FMT ": mem error in WDMA desc %u/%u", BDBG_UINT64_ARG(CTX_ID(ctx)), i, ctx->numBlocks));
            BDBG_ASSERT(0);
        }
    }
}
#endif /* BXPT_DMA_MSA_DEBUG_MODE */

#if BXPT_DMA_SAGE_MODE
#include "bxpt_dma_sage_priv.h"
#endif

void BXPT_Dma_Context_P_BlockSettingsDump(BXPT_Dma_ContextHandle ctx, const BXPT_Dma_ContextBlockSettings *pSettings);
void BXPT_Dma_Context_P_DescDump(BXPT_Dma_ContextHandle ctx, bool link_only, bool print_warnings);
void BXPT_Dma_P_RegDumpAll(BXPT_Dma_Handle dma);

unsigned BXPT_Dma_P_GetWdmaRun_isrsafe(BXPT_Dma_Handle dma)
{
    uint32_t reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_RUN_BITS_0_31);
    reg = BCHP_GET_FIELD_DATA(reg, XPT_WDMA_REGS_RUN_BITS_0_31, RUN_BITS);
    return ((reg >> dma->channelNum) & 1);
}

void BXPT_Dma_P_SetWdmaRun_isrsafe(BXPT_Dma_Handle dma, unsigned run)
{
    uint32_t reg = 0;
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_RUN_SET_CLEAR, CHANNEL_NUM, dma->channelNum);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_RUN_SET_CLEAR, SET_CLEAR, run);
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_REGS_RUN_SET_CLEAR, reg);
}

/* WDMA and MCPB wake bits appear to always be 0 when we query */
#if (!B_REFSW_MINIMAL)
unsigned BXPT_Dma_P_GetWdmaWake_isrsafe(BXPT_Dma_Handle dma)
{
    uint32_t reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_WAKE_BITS_0_31);
    reg = BCHP_GET_FIELD_DATA(reg, XPT_WDMA_REGS_WAKE_BITS_0_31, WAKE_BITS);
    return ((reg >> dma->channelNum) & 1);
}
#endif

void BXPT_Dma_P_SetWdmaWake_isrsafe(BXPT_Dma_Handle dma, unsigned wake)
{
    uint32_t reg = 0;
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_WAKE_SET, CHANNEL_NUM, dma->channelNum);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_WAKE_SET, SET, wake);
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_REGS_WAKE_SET, reg);
}

unsigned BXPT_Dma_P_GetWdmaSleep_isrsafe(BXPT_Dma_Handle dma)
{
    /* WDMA sleep should be used with care: do not assume that a WAKE immediately clears SLEEP */
    uint32_t reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_SLEEP_STATUS_0_31);
    reg = BCHP_GET_FIELD_DATA(reg, XPT_WDMA_REGS_SLEEP_STATUS_0_31, SLEEP_STATUS);
    return ((reg >> dma->channelNum) & 1);
}

unsigned BXPT_Dma_P_GetMcpbRun_isrsafe(BXPT_Dma_Handle dma)
{
    uint32_t reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_RUN_STATUS_0_31);
    reg = BCHP_GET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_RUN_STATUS_0_31, RUN_STATUS);
    return ((reg >> dma->channelNum) & 1);
}

void BXPT_Dma_P_SetMcpbRun_isrsafe(BXPT_Dma_Handle dma, unsigned run)
{
    uint32_t reg = 0;
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_RUN_SET_CLEAR, MCPB_CHANNEL_NUM, dma->channelNum);
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_RUN_SET_CLEAR, SET_CLEAR, run);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_RUN_SET_CLEAR, reg);
}

#if (!B_REFSW_MINIMAL)
unsigned BXPT_Dma_P_GetMcpbWake_isrsafe(BXPT_Dma_Handle dma)
{
    uint32_t reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_WAKE_STATUS_0_31);
    reg = BCHP_GET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_WAKE_STATUS_0_31, WAKE_STATUS);
    return ((reg >> dma->channelNum) & 1);
}
#endif

void BXPT_Dma_P_SetMcpbWake_isrsafe(BXPT_Dma_Handle dma, unsigned wake)
{
    uint32_t reg = 0;
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_WAKE_SET, MCPB_CHANNEL_NUM, dma->channelNum);
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_WAKE_SET, SET, wake);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_WAKE_SET, reg);
}

unsigned BXPT_Dma_P_IncrementRunVersion_isrsafe(BXPT_Dma_Handle dma)
{
    unsigned runVersion;
    uint32_t reg;
    reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL + dma->mcpbRegOffset);
    runVersion = BCHP_GET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL, RUN_VERSION);

    if (++runVersion > BXPT_DMA_MAX_RUN_VERSION) {
        runVersion = 0;
    }

    reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL + dma->mcpbRegOffset);
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL, RUN_VERSION, runVersion);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL + dma->mcpbRegOffset, reg);

    reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_CH0_RUN_VERSION_CONFIG + dma->wdmaRamOffset);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_CH0_RUN_VERSION_CONFIG, RUN_VERSION, runVersion);
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_CH0_RUN_VERSION_CONFIG + dma->wdmaRamOffset, reg);

    return runVersion;
}

static void BXPT_Dma_P_DescDoneCallback_isr(void *pParam1, int parm2);

#if BXPT_DMA_RATE_TEST
/* in microseconds */
static long BXPT_Dma_P_TimeDiff(const BXPT_Dma_Time *future, const BXPT_Dma_Time *past)
{
    return 1000000*(future->tv_sec - past->tv_sec) + (future->tv_nsec - past->tv_nsec)/1000;
}

static void BXPT_Dma_P_RateDump(BXPT_Dma_Handle dma)
{
    /* for now, just measure from first RUN to first interrupt, with size of first context */
    uint64_t size = dma->lastQueuedCtx->dmaLength;
    long mcpbTime = BXPT_Dma_P_TimeDiff(&dma->mcpbEndTime, &dma->startTime);
    long wdmaTime = BXPT_Dma_P_TimeDiff(&dma->wdmaEndTime, &dma->startTime);
    BDBG_WRN(("%u: MCPB %llu bps, WDMA %llu bps", dma->channelNum, (size*8*1000*1000)/mcpbTime, (size*8*1000*1000)/wdmaTime));
}
#endif

#if (!B_REFSW_MINIMAL)
void BXPT_Dma_P_McpbDescDoneCallback_isr(void *pParam1, int parm2)
{
    BXPT_Dma_Handle dma = (BXPT_Dma_Handle)pParam1;
    BSTD_UNUSED(parm2);

    BDBG_OBJECT_ASSERT(dma, BXPT_Dma_Handle_Tag);
    BDBG_MSG_TRACE(("%u: MCPB_DESC_DONE ISR", dma->channelNum));
#if BXPT_DMA_RATE_TEST
{
    int rc = clock_gettime(CLOCK_MONOTONIC, &dma->mcpbEndTime);
    BDBG_ASSERT(rc==0);

    if (dma->usePbMcpb) {
        BXPT_Dma_P_RateDump(dma);
    }
}
#endif
}

void BXPT_Dma_P_McpbFalseWakeCallback_isr(void *pParam1, int parm2)
{
    BXPT_Dma_Handle dma = (BXPT_Dma_Handle)pParam1;
    BSTD_UNUSED(parm2);

    BDBG_OBJECT_ASSERT(dma, BXPT_Dma_Handle_Tag);
    BDBG_ERR(("%u: MCPB_FALSE_WAKE ISR", dma->channelNum));

    #if 0
    BDBG_ASSERT(0);
    #endif

    /* TODO: this callback doesn't get called, even when its FALSE_WAKE_INTR_L2_CPU_STATUS bit gets set.
       note, as prerequisites, we need BXPT_DMA_MCPB_FALSE_WAKE_IRQ=1 and bint_7445[].L2InvalidMask cleared */
}
#endif

void BXPT_Dma_P_OverflowCallback_isr(void *pParam1, int parm2)
{
    BXPT_Dma_Handle dma = (BXPT_Dma_Handle)pParam1;
    void *vdma = dma;
    uint32_t reg;
    BSTD_UNUSED(parm2);

    BDBG_OBJECT_ASSERT(dma, BXPT_Dma_Handle_Tag);
    /* on overflow, data engine drops data on the floor for the duration of the packet */

    reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_CH0_OVERFLOW_REASONS + dma->wdmaRamOffset);
    /* assume that the reasons are not mutually exclusive */
    if (BCHP_GET_FIELD_DATA(reg, XPT_WDMA_CH0_OVERFLOW_REASONS, DATA_STALL_TIMEOUT)) {
        BDBG_ERR(("%p:%u: overflow: DATA_STALL_TIMEOUT", vdma, dma->channelNum));
    }
    if (BCHP_GET_FIELD_DATA(reg, XPT_WDMA_CH0_OVERFLOW_REASONS, SLEEP_NO_WAKE)) {
        BDBG_ERR(("%p:%u: overflow: SLEEP_NO_WAKE", vdma, dma->channelNum));
    }
    if (BCHP_GET_FIELD_DATA(reg, XPT_WDMA_CH0_OVERFLOW_REASONS, RUN_NOT_SET)) {
        BDBG_ERR(("%p:%u: overflow: RUN_NOT_SET", vdma, dma->channelNum));
    }
    if (BCHP_GET_FIELD_DATA(reg, XPT_WDMA_CH0_OVERFLOW_REASONS, RING_BUFFER_FULL)) {
        BDBG_ERR(("%p:%u: overflow: RING_BUFFER_FULL", vdma, dma->channelNum));
    }
    if (BCHP_GET_FIELD_DATA(reg, XPT_WDMA_CH0_OVERFLOW_REASONS, PACKET_SYNC_ERROR)) {
        BDBG_ERR(("%p:%u: overflow: PACKET_SYNC_ERROR", vdma, dma->channelNum));
    }
    if (reg==0) {
        BDBG_ERR(("%p:%u: overflow: unknown reason", vdma, dma->channelNum));
    }

    {
        /* print the last offset that we initiated WAKE against, and the HW status.
           if the HW woke properly and processed the descriptors, the offset will match the HW status
           since overflow likely occurs without completion ISR, check on ctx->pp, not !ctx->pp */
        BXPT_Dma_ContextHandle ctx = dma->lastQueuedCtx;
        BDBG_ERR(("WDMA last wake " BDBG_UINT64_FMT ", NDA %08x, CDA %08x\n", BDBG_UINT64_ARG(FIRST_WDMA_DESC_OFFSET(ctx)),
            BREG_Read32(dma->reg, BCHP_XPT_WDMA_CH0_NEXT_DESC_ADDR + dma->wdmaRamOffset),
            BREG_Read32(dma->reg, BCHP_XPT_WDMA_CH0_COMPLETED_DESC_ADDRESS + dma->wdmaRamOffset)));
        BDBG_ERR(("MCPB last wake " BDBG_UINT64_FMT ", cur %08x, NDA %08x", BDBG_UINT64_ARG(FIRST_MCPB_DESC_OFFSET(ctx)),
            BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_CURR_DESC_ADDRESS + dma->mcpbRegOffset),
            BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_NEXT_DESC_ADDRESS + dma->mcpbRegOffset)));
    }

    #if 0
    BXPT_Dma_P_RegDumpAll(dma);
    #endif

    BDBG_ASSERT(0); /* better to assert and debug issue */

    BREG_Write32(dma->reg, BCHP_XPT_WDMA_CH0_OVERFLOW_REASONS + dma->wdmaRamOffset, 0);
    return;
}

void BXPT_Dma_P_BtpCallback_isr(void *pParam1, int parm2)
{
    BSTD_UNUSED(pParam1);
    BSTD_UNUSED(parm2);
    BDBG_MSG(("BTP callback"));
}

void BXPT_Dma_GetDefaultSettings(
    BXPT_Dma_Settings *pSettings
    )
{
    BDBG_ASSERT(pSettings != NULL);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

BERR_Code BXPT_Dma_P_SetSettings(BXPT_Dma_Handle dma, const BXPT_Dma_Settings *pSettings)
{
    unsigned spPacketLength, spStreamType, spFeedSize;
    uint32_t reg;
    const char* scramMode2String[BXPT_Dma_ScramMode_eMax] = {"block", "mpeg", "dss"};
    BSTD_UNUSED(scramMode2String);

    /* set scramMode-related config */
    switch (pSettings->scramMode) {
        case BXPT_Dma_ScramMode_eBlock:
            spPacketLength = 192;
            spStreamType = 6;
            spFeedSize = 224;
            break;
        case BXPT_Dma_ScramMode_eMpeg:
            spPacketLength = 188;
            spStreamType = 0;
            spFeedSize = 192;
            break;
        case BXPT_Dma_ScramMode_eDss:
            spPacketLength = 130;
            spStreamType = 1;
            spFeedSize = 134;
            break;
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG_TRACE(("BXPT_Dma_P_SetSettings: %u: scram mode %u (%s), ats %u", dma->channelNum, pSettings->scramMode, scramMode2String[pSettings->scramMode], pSettings->timestampEnabled));

    reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_SP_PKT_LEN + dma->mcpbRegOffset);
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_SP_PKT_LEN, PACKET_LENGTH, spPacketLength);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_SP_PKT_LEN + dma->mcpbRegOffset, reg);

    reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL + dma->mcpbRegOffset);
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL, PARSER_STREAM_TYPE, spStreamType);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL + dma->mcpbRegOffset, reg);

    reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_BBUFF_CTRL + dma->mcpbRegOffset);
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_DMA_BBUFF_CTRL, STREAM_PROC_FEED_SIZE, spFeedSize);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_BBUFF_CTRL + dma->mcpbRegOffset, reg);

#ifdef BCHP_XPT_WDMA_REGS_TIMESTAMP_MODE_CONFIG
    reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_SP_TS_CONFIG + dma->mcpbRegOffset);
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_SP_TS_CONFIG, INPUT_HAS_ATS, pSettings->timestampEnabled ? 1:0);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_SP_TS_CONFIG + dma->mcpbRegOffset, reg);

    reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_TIMESTAMP_MODE_CONFIG);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_TIMESTAMP_MODE_CONFIG, CHANNEL_NUM, dma->channelNum);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_TIMESTAMP_MODE_CONFIG, CONFIG, pSettings->timestampEnabled ? 1:0);
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_REGS_TIMESTAMP_MODE_CONFIG, reg);
#else
    if (pSettings->timestampEnabled) {
        return BERR_TRACE((BERR_NOT_SUPPORTED));
    }
#endif

    dma->settings = *pSettings;
    return BERR_SUCCESS;
}

BERR_Code BXPT_Dma_P_Init(BXPT_Handle hXpt)
{
    bool doSoftInit = true;
    unsigned i, softInit = 0;
    BREG_Handle hReg = hXpt->hRegister;

#if (!BXPT_DMA_HAS_MEMDMA_MCPB)
    doSoftInit = false;
#endif

    if (doSoftInit) {
        /* Init all MCPB registers to 0, including RAM-based regs */
        BREG_Write32(hReg, BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DO_MEM_INIT,
            BCHP_FIELD_DATA(XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DO_MEM_INIT, MEMDMA_MCPB_SOFT_INIT_DO_MEM_INIT, 1));
        BREG_Write32(hReg, BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_SET,
            BCHP_FIELD_DATA(XPT_BUS_IF_SUB_MODULE_SOFT_INIT_SET, MEMDMA_MCPB_SOFT_INIT_SET, 1));
        i = 300;
        do {
            softInit = BCHP_GET_FIELD_DATA(BREG_Read32(hReg, BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_STATUS),
                XPT_BUS_IF_SUB_MODULE_SOFT_INIT_STATUS, MEMDMA_MCPB_SOFT_INIT_STATUS);
        } while(--i && softInit);

        if (softInit) {
            BDBG_ERR(("BXPT_Dma_P_Init: MCPB soft init failed"));
            return BERR_TRACE(BERR_UNKNOWN);
        }
        BREG_Write32(hReg, BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_CLEAR,
            BCHP_FIELD_DATA(XPT_BUS_IF_SUB_MODULE_SOFT_INIT_CLEAR, MEMDMA_MCPB_SOFT_INIT_CLEAR, 1));
    }

    /* init BO_COUNT */
    for (i=BXPT_DMA_NUM_FRONT_SHARED_CH; i<BXPT_DMA_NUM_CHANNELS; i++) {
        uint32_t step = BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_CONTROL - BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DESC_CONTROL;
        BREG_Write32(hReg, BCHP_XPT_MEMDMA_MCPB_CH0_TMEU_BLOCKOUT_CTRL + step*i, BXPT_DMA_PWR_BO_COUNT);
    }

    return 0;
}

BERR_Code BXPT_Dma_P_OpenChannel(
    BXPT_Handle hXpt, BCHP_Handle hChp, BREG_Handle hReg, BMMA_Heap_Handle hMmaHeap, BINT_Handle hInt,
    BXPT_Dma_Handle *phDma,
    unsigned channelNum,
    const BXPT_Dma_Settings *pSettings
    )
{
    BXPT_Dma_Handle dma = NULL;
    BERR_Code rc;
    uint32_t intr, reg;
    unsigned runVersion, dramReqSize = 256;
    BXPT_Dma_Settings settings;
    bool isSageCh = (BXPT_DMA_RESERVE_SAGE_CH && channelNum==BXPT_DMA_SAGE_CH_NUM);

    if (channelNum>=BXPT_DMA_NUM_CHANNELS) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto error;
    }
    if (!isSageCh && hXpt->dmaChannels[channelNum]!=NULL) {
        BDBG_ERR(("BXPT_Dma_OpenChannel: %u: Channel already opened", channelNum));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto error;
    }
#if BXPT_DMA_LIMIT_TO_SINGLE_CH
    if (channelNum>0) {
        BDBG_ERR(("BXPT_Dma_OpenChannel: %u: functionality limited to channel 0", channelNum));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto error;
    }
#endif

    if (pSettings==NULL) {
        BXPT_Dma_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    *phDma = NULL;
    dma = BKNI_Malloc(sizeof(*dma));
    if (dma == NULL) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    BKNI_Memset(dma, 0, sizeof(*dma));
    BDBG_OBJECT_SET(dma, BXPT_Dma_Handle_Tag);

    dma->xpt = hXpt;
    dma->chp = hChp;
    dma->reg = hReg;
    dma->mmaHeap = hMmaHeap;
    dma->bint = hInt;
    dma->channelNum = channelNum;
    dma->settings = *pSettings;
#if BXPT_DMA_AVOID_WAKE
    dma->wakeDisabled = true;
#endif
#if BXPT_DMA_SEPARATE_MEMC_FOR_DESC && 0 /* deprecated */
{
    if (pSettings->memExtra) {
        BMEM_HeapInfo heapInfo0, heapInfo1;
        BMEM_Heap_GetInfo(dma->mem0, &heapInfo0);
        BMEM_Heap_GetInfo(pSettings->memExtra, &heapInfo1);
        if ((heapInfo0.ulOffset >= 0x40000000 && heapInfo1.ulOffset >= 0x40000000) ||
            (heapInfo0.ulOffset < 0x40000000 && heapInfo1.ulOffset < 0x40000000))
        {
            BDBG_WRN(("BXPT_Dma_OpenChannel: %u: mem offsets in same SCB: %#x %#x\n", dma->channelNum, heapInfo0.ulOffset, heapInfo1.ulOffset));
        }
        dma->mem1 = pSettings->memExtra;
    }
}
#endif

    dma->mcpbRegOffset = (BCHP_XPT_MEMDMA_MCPB_CH1_DMA_DESC_CONTROL - BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DESC_CONTROL) * channelNum;
#if BXPT_DMA_HAS_WDMA_CHX
    dma->wdmaRamOffset = (BCHP_XPT_WDMA_CH1_FIRST_DESC_ADDR - BCHP_XPT_WDMA_CH0_FIRST_DESC_ADDR) * channelNum;
#else
    dma->wdmaRamOffset = (BCHP_XPT_WDMA_RAMS_CHAN_OFFSET - BCHP_XPT_WDMA_RAMS_FIRST_DESC_ADDR) * channelNum;
#endif

#if BXPT_DMA_RATE_TEST
    /* usePbMcpb doesn't work. needs more time investment if needed */
    if (0) {
        dma->mcpbRegOffset += BCHP_XPT_MCPB_CH0_DMA_DESC_CONTROL - BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DESC_CONTROL;
        dma->usePbMcpb = true;
    }
#endif

    BLST_S_INIT(&dma->ctxList);
    BLST_SQ_INIT(&dma->activeCtxList);

#ifdef BCHP_PWR_RESOURCE_DMA
    BCHP_PWR_AcquireResource(dma->chp, BCHP_PWR_RESOURCE_DMA);
#endif

    /* see BXPT_Dma_P_Init() for global init routine */

    /* enforce RUN=0 for MCPB and WDMA */
    if (BXPT_Dma_P_GetMcpbRun_isrsafe(dma)) {
        BDBG_WRN(("BXPT_Dma_OpenChannel: %u: MCPB RUN_STATUS bit set. Forcing RUN=0...", channelNum));
        BXPT_Dma_P_SetMcpbRun_isrsafe(dma, 0);
    }
    if (BXPT_Dma_P_GetWdmaRun_isrsafe(dma)) {
        BDBG_WRN(("BXPT_Dma_OpenChannel: %u: WDMA RUN_BITS bit set. Forcing RUN=0...", channelNum));
        BXPT_Dma_P_SetWdmaRun_isrsafe(dma, 0);
    }

    /* setup SP_PARSER_CTRL */
    reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL + dma->mcpbRegOffset);
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL, PB_PARSER_SEL, 1);
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL, PB_PARSER_BAND_ID, dma->channelNum + BXPT_DMA_BAND_ID_OFFSET);
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL, PARSER_ENABLE, 1);
#if (!BXPT_DMA_HAS_MEMDMA_MCPB)
    BCHP_SET_FIELD_DATA(reg, XPT_MCPB_CH0_SP_PARSER_CTRL, MEMDMA_PIPE_EN, 1);
#endif
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL + dma->mcpbRegOffset, reg);

    /* setup SP_TS_CONFIG */
    reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_SP_TS_CONFIG + dma->mcpbRegOffset);
#ifdef BCHP_XPT_MEMDMA_MCPB_CH0_SP_TS_CONFIG_PARSER_ACCEPT_NULL_PKT_MASK
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_SP_TS_CONFIG, PARSER_ACCEPT_NULL_PKT, 1);
#else
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_SP_TS_CONFIG, PARSER_ACCEPT_NULL_PKT_PRE_MPOD, 1);
#endif
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_SP_TS_CONFIG, MPEG_TS_AUTO_SYNC_DETECT, 0); /* blind mode: ignore sync byte */
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_SP_TS_CONFIG, INPUT_TEI_IGNORE, 1); /* ignore TEI */
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_SP_TS_CONFIG + dma->mcpbRegOffset, reg);

    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_TMEU_BLOCKOUT_CTRL + dma->mcpbRegOffset, BXPT_DMA_DEF_BO_COUNT);
#if BXPT_DMA_RATE_TEST
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_TMEU_BLOCKOUT_CTRL + dma->mcpbRegOffset, 0);
#endif

    /* set channel settings */
    rc = BXPT_Dma_P_SetSettings(dma, pSettings);
    if (rc!=BERR_SUCCESS) {
        goto error;
    }

    /* clear FIRST_DESC_ADDR */
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_CH0_FIRST_DESC_ADDR + dma->wdmaRamOffset, 0);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DESC_CONTROL + dma->mcpbRegOffset, 0);

    /* clear other state */
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_CH0_COMPLETED_DESC_ADDRESS + dma->wdmaRamOffset, 0);
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_CH0_BTP_PACKET_GROUP_ID + dma->wdmaRamOffset, 0);
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_CH0_RUN_VERSION_CONFIG + dma->wdmaRamOffset, 0);
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_CH0_OVERFLOW_REASONS + dma->wdmaRamOffset, 0);

    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_STATUS + dma->mcpbRegOffset, 0);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_DESC_ADDR + dma->mcpbRegOffset, 0);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_DESC_DONE_INT_ADDR + dma->mcpbRegOffset, 0);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_DATA_ADDR_LOWER + dma->mcpbRegOffset, 0);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_DATA_ADDR_UPPER + dma->mcpbRegOffset, 0);

    reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_DATA_STALL_TIMEOUT);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_DATA_STALL_TIMEOUT, TIMEOUT_CLOCKS, 0xA4CB80); /* 108M = 100ms */
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_REGS_DATA_STALL_TIMEOUT, reg);

#ifdef BCHP_XPT_WDMA_REGS_ADDRESS_MODE_CONFIG
    reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_ADDRESS_MODE_CONFIG);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_ADDRESS_MODE_CONFIG, CHANNEL_NUM, channelNum);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_ADDRESS_MODE_CONFIG, CONFIG, 0); /* Indirect Address Mode (use descriptors) */
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_REGS_ADDRESS_MODE_CONFIG, reg);
#else
    reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_RBUF_MODE_CONFIG);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_RBUF_MODE_CONFIG, CHANNEL_NUM, channelNum);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_RBUF_MODE_CONFIG, CONFIG, 0); /* Indirect Address Mode (use descriptors) */
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_REGS_RBUF_MODE_CONFIG, reg);
#endif

#ifdef BCHP_XPT_WDMA_PM_CONTROL_MODE
    reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_PM_CONTROL_MODE);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_PM_CONTROL_MODE, MODE, 0); /* normal mode */
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_PM_CONTROL_MODE, reg);
#endif

    /* RUN_VERSION is used to do a clean start */
#if BXPT_DMA_USE_RUN_VERSION
    reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_MATCH_RUN_VERSION_CONFIG);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_MATCH_RUN_VERSION_CONFIG, CHANNEL_NUM, channelNum);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_MATCH_RUN_VERSION_CONFIG, CONFIG, 1); /* enable RUN_VERSION_MATCH */
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_REGS_MATCH_RUN_VERSION_CONFIG, reg);

    runVersion = BXPT_Dma_P_IncrementRunVersion_isrsafe(dma);
    BDBG_MSG(("BXPT_Dma_OpenChannel: %u: RUN_VERSION %u", channelNum, runVersion));
#else
    BSTD_UNUSED(runVersion);
    reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL + dma->mcpbRegOffset);
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL, RUN_VERSION, 0);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL + dma->mcpbRegOffset, reg);

    reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_MATCH_RUN_VERSION_CONFIG);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_MATCH_RUN_VERSION_CONFIG, CHANNEL_NUM, channelNum);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_MATCH_RUN_VERSION_CONFIG, CONFIG, 0);
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_REGS_MATCH_RUN_VERSION_CONFIG, reg);
#endif

    reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL + dma->mcpbRegOffset);
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL, DRAM_REQ_SIZE, dramReqSize);
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL + dma->mcpbRegOffset, reg);

    /* coverity[dead_error_line: FALSE] */
    if (isSageCh && hInt==NULL) {
        goto postint;
    }

    BXPT_Dma_P_EnableInterrupts(hXpt);

    BDBG_CASSERT(31 ==
        BCHP_INT_ID_XPT_WDMA_DESC_DONE_INTR_L2_WDMA_CHAN_31_INTR -
        BCHP_INT_ID_XPT_WDMA_DESC_DONE_INTR_L2_WDMA_CHAN_00_INTR);
    BDBG_CASSERT(31 ==
        BCHP_INT_ID_XPT_WDMA_OVERFLOW_INTR_L2_WDMA_CHAN_31_INTR -
        BCHP_INT_ID_XPT_WDMA_OVERFLOW_INTR_L2_WDMA_CHAN_00_INTR);
    BDBG_CASSERT(31 ==
        BCHP_INT_ID_XPT_WDMA_BTP_INTR_L2_WDMA_CHAN_31_INTR -
        BCHP_INT_ID_XPT_WDMA_BTP_INTR_L2_WDMA_CHAN_00_INTR);

    intr = BCHP_INT_ID_XPT_WDMA_DESC_DONE_INTR_L2_WDMA_CHAN_00_INTR + channelNum;
    rc = BINT_CreateCallback(&dma->irqDescDone, dma->bint, intr, BXPT_Dma_P_DescDoneCallback_isr, dma, 0);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }
    rc = BINT_EnableCallback(dma->irqDescDone);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }

    intr = BCHP_INT_ID_XPT_WDMA_OVERFLOW_INTR_L2_WDMA_CHAN_00_INTR + channelNum;
    rc = BINT_CreateCallback(&dma->irqOverflow, dma->bint, intr, BXPT_Dma_P_OverflowCallback_isr, dma, 0);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }
    rc = BINT_EnableCallback(dma->irqOverflow);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }

    intr = BCHP_INT_ID_XPT_WDMA_BTP_INTR_L2_WDMA_CHAN_00_INTR + channelNum;
    rc = BINT_CreateCallback(&dma->irqBtp, dma->bint, intr, BXPT_Dma_P_BtpCallback_isr, dma, 0);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }
    rc = BINT_EnableCallback(dma->irqBtp);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }

#if BXPT_DMA_MCPB_DESC_DONE_IRQ
    BDBG_CASSERT(31 ==
        BCHP_INT_ID_XPT_MEMDMA_MCPB_DESC_DONE_INTR_L2_MCPB_CHAN_31_INTR -
        BCHP_INT_ID_XPT_MEMDMA_MCPB_DESC_DONE_INTR_L2_MCPB_CHAN_00_INTR);

    intr = BCHP_INT_ID_XPT_MEMDMA_MCPB_DESC_DONE_INTR_L2_MCPB_CHAN_00_INTR + channelNum;
#if BXPT_DMA_RATE_TEST
    if (dma->usePbMcpb) {
        intr = BCHP_INT_ID_XPT_MCPB_DESC_DONE_INTR_L2_MCPB_CHAN_00_INTR + channelNum;
    }
#endif
    rc = BINT_CreateCallback(&dma->irqMcpbDescDone, dma->bint, intr, BXPT_Dma_P_McpbDescDoneCallback_isr, dma, 0);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }
    rc = BINT_EnableCallback(dma->irqMcpbDescDone);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }
#endif

#if BXPT_DMA_MCPB_FALSE_WAKE_IRQ
    BDBG_CASSERT(31 ==
        BCHP_INT_ID_XPT_MEMDMA_MCPB_MISC_FALSE_WAKE_INTR_L2_MCPB_CHAN_31_INTR -
        BCHP_INT_ID_XPT_MEMDMA_MCPB_MISC_FALSE_WAKE_INTR_L2_MCPB_CHAN_00_INTR);

    intr = BCHP_INT_ID_XPT_MEMDMA_MCPB_MISC_FALSE_WAKE_INTR_L2_MCPB_CHAN_00_INTR + channelNum;
    rc = BINT_CreateCallback(&dma->irqMcpbFalseWake, dma->bint, intr, BXPT_Dma_P_McpbFalseWakeCallback_isr, dma, 0);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }
    rc = BINT_EnableCallback(dma->irqMcpbFalseWake);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }

#if 0
    /* make sure it's cleared so that we can manually check when it gets set */
    BDBG_ASSERT(BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_MISC_FALSE_WAKE_INTR_L2_CPU_STATUS)==0);
#endif
#endif

postint:

#ifdef BCHP_PWR_RESOURCE_DMA
    BCHP_PWR_ReleaseResource(dma->chp, BCHP_PWR_RESOURCE_DMA);
#endif

    if (!isSageCh) {
        hXpt->dmaChannels[channelNum] = dma;
    }
    *phDma = dma;
    return BERR_SUCCESS;

error:
    if (dma != NULL) {
        if (dma->irqDescDone) {
            BINT_DestroyCallback(dma->irqDescDone);
        }
        if (dma->irqOverflow) {
            BINT_DestroyCallback(dma->irqOverflow);
        }
        if (dma->irqBtp) {
            BINT_DestroyCallback(dma->irqBtp);
        }
        if (dma->irqMcpbDescDone) {
            BINT_DestroyCallback(dma->irqMcpbDescDone);
        }
        if (dma->irqMcpbFalseWake) {
            BINT_DestroyCallback(dma->irqMcpbFalseWake);
        }
        BKNI_Free(dma);
        *phDma = NULL;
    }
    return rc;
}

void BXPT_Dma_P_EnableInterrupts(BXPT_Handle hXpt)
{
    uint32_t reg;

    /* aggregator handling */
    reg = BREG_Read32(hXpt->hRegister, BCHP_XPT_MEMDMA_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS);
    BDBG_MSG_TRACE(("  MCPB aggregator mask status: %#x", reg));
    reg = 0;
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR, MCPB_DESC_DONE_INTR, 1);
    BREG_Write32(hXpt->hRegister, BCHP_XPT_MEMDMA_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR, reg);
    reg = BREG_Read32(hXpt->hRegister, BCHP_XPT_MEMDMA_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS);
    BDBG_MSG_TRACE(("  MCPB aggregator mask status: %#x", reg));

    reg = BREG_Read32(hXpt->hRegister, BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS);
    BDBG_MSG_TRACE(("  WDMA aggregator mask status: %#x", reg));
    reg = 0;
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR, DESC_DONE_INTR, 1);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR, OVERFLOW_INTR, 1);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR, BTP_INTR, 1);
    BREG_Write32(hXpt->hRegister, BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR, reg);
    reg = BREG_Read32(hXpt->hRegister, BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS);
    BDBG_MSG_TRACE(("  WDMA aggregator mask status: %#x", reg));
}

BERR_Code BXPT_Dma_OpenChannel(
    BXPT_Handle hXpt,
    BXPT_Dma_Handle *phDma,
    unsigned channelNum,
    const BXPT_Dma_Settings *pSettings
    )
{
    BDBG_ASSERT(hXpt);
    BDBG_ASSERT(hXpt->hChip);
    BDBG_ASSERT(hXpt->hRegister);
    BDBG_ASSERT(hXpt->mmaHeap);
    BDBG_ASSERT(hXpt->hInt);
#if BXPT_DMA_RESERVE_SAGE_CH
    if (channelNum==BXPT_DMA_SAGE_CH_NUM) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
#endif
    return BXPT_Dma_P_OpenChannel(hXpt, hXpt->hChip, hXpt->hRegister, hXpt->mmaHeap, hXpt->hInt, phDma, channelNum, pSettings);
}

BERR_Code BXPT_Dma_GetSettings(
    BXPT_Dma_Handle hDma,
    BXPT_Dma_Settings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(hDma, BXPT_Dma_Handle_Tag);
    BDBG_ASSERT(pSettings);
    *pSettings = hDma->settings;
    return BERR_SUCCESS;
}

BERR_Code BXPT_Dma_SetSettings(
    BXPT_Dma_Handle hDma,
    const BXPT_Dma_Settings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(hDma, BXPT_Dma_Handle_Tag);
    BDBG_ASSERT(pSettings);
    return BXPT_Dma_P_SetSettings(hDma, pSettings);
}

void BXPT_Dma_Context_GetDefaultSettings(
    BXPT_Dma_ContextSettings *pSettings
    )
{
    BDBG_ASSERT(pSettings != NULL);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->maxNumBlocks = 1;
    pSettings->pidChannelNum = BXPT_DMA_PID_CHANNEL_NUM_START;
}

void BXPT_Dma_Context_GetDefaultBlockSettings(
    BXPT_Dma_ContextBlockSettings *pSettings
    )
{
    BDBG_ASSERT(pSettings != NULL);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static BERR_Code BXPT_Dma_Context_P_SetSettings(BXPT_Dma_ContextHandle ctx, const BXPT_Dma_ContextSettings *pSettings)
{
    bool readSwap;
    unsigned readSwapMode, writeSwapMode, prevReadSwapMode, prevWriteSwapMode;
    BXPT_Dma_Handle dma = ctx->parent;
    uint32_t reg;

    ctx->mcpbDescW4 = 0;

    if (pSettings->endianMode==BXPT_Dma_EndianMode_eLittle) {
        readSwap = (BSTD_CPU_ENDIAN!=BSTD_ENDIAN_LITTLE);
    }
    else if (pSettings->endianMode==BXPT_Dma_EndianMode_eBig) {
        readSwap = (BSTD_CPU_ENDIAN!=BSTD_ENDIAN_BIG);
    }
    else {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

#if (!BXPT_DMA_HAS_WDMA_CHX)
    prevReadSwapMode = 0;
    readSwapMode = 0; /* readSwapMode doesn't exist */
    ctx->mcpbDescW5 = readSwap ? MCPB_DW5_RD_ENDIAN_STRAP_INV_CTRL : 0;
#else
    prevReadSwapMode = (ctx->mcpbDescW5 >> MCPB_DW5_DATA_UNIT_SIZE_SHIFT) & 3;
    readSwapMode = readSwap ? DESC_DATA_UNIT_SIZE_32 : DESC_DATA_UNIT_SIZE_8; /* no swap means treat the data as 8 bits */
    ctx->mcpbDescW5 = readSwapMode << MCPB_DW5_DATA_UNIT_SIZE_SHIFT;
#endif

    ctx->mcpbDescW5 |= MCPB_DW5_PID_CHANNEL_VALID;

    if (pSettings->pidChannelNum < BXPT_DMA_PID_CHANNEL_NUM_START || pSettings->pidChannelNum >= BXPT_P_PID_TABLE_SIZE) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    ctx->mcpbDescW6 = pSettings->pidChannelNum; /* PID_CHANNEL [9:0] */
    BDBG_MSG_TRACE(("" BDBG_UINT64_FMT " pid channel: %u", BDBG_UINT64_ARG(CTX_ID(ctx)), pSettings->pidChannelNum));

    /* descriptor specification.
       note that BMMD_SwapMode_eWord = 1 and eByte = 2,
       but BXPT_Dma_SwapMode_eWord = 2 and eByte = 1 */
#if (!BXPT_DMA_HAS_WDMA_CHX)
    #define SWAP_NONE 0
    #define SWAP_BYTE 1
    #define SWAP_WORD 2
#else
    #define SWAP_NONE DESC_DATA_UNIT_SIZE_8
    #define SWAP_BYTE DESC_DATA_UNIT_SIZE_32
    #define SWAP_WORD DESC_DATA_UNIT_SIZE_16
#endif

    switch (pSettings->swapMode) {
        case BXPT_Dma_SwapMode_eNone:
            writeSwapMode = SWAP_NONE;
            break;
        case BXPT_Dma_SwapMode_eByte:
            writeSwapMode = SWAP_BYTE;
            break;
        case BXPT_Dma_SwapMode_eWord:
            writeSwapMode = SWAP_WORD;
            break;
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

#if BXPT_DMA_WDMA_END_INVERT
    switch (pSettings->swapMode) {
        case BXPT_Dma_SwapMode_eNone:
            writeSwapMode = SWAP_BYTE;
            break;
        case BXPT_Dma_SwapMode_eByte:
            writeSwapMode = SWAP_NONE;
            break;
        case BXPT_Dma_SwapMode_eWord:
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
#endif

#if BXPT_DMA_WDMA_ENDIAN_SWAP_CBIT
    /* swapMode is per-Context, but the setting is per-channel. set it for the entire channel on first trigger */
    if (pSettings->swapMode!=BXPT_Dma_SwapMode_eNone) {
        unsigned setCbit = 1;
        reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL + dma->mcpbRegOffset);
        BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL, ENDIAN_CONTROL, setCbit);
        BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL + dma->mcpbRegOffset, reg);

        reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_CH0_DATA_CONTROL + dma->wdmaRamOffset);
        BCHP_SET_FIELD_DATA(reg, XPT_WDMA_CH0_DATA_CONTROL, INV_STRAP_ENDIAN_CTRL, setCbit);
        BREG_Write32(dma->reg, BCHP_XPT_WDMA_CH0_DATA_CONTROL + dma->wdmaRamOffset, reg);
    }
#endif

    prevWriteSwapMode = (ctx->wdmaDescW3 >> WDMA_DW3_DATA_UNIT_SIZE_SHIFT) & 3;
    ctx->wdmaDescW3 = writeSwapMode << WDMA_DW3_DATA_UNIT_SIZE_SHIFT;

    if ((prevReadSwapMode != readSwapMode) || (prevWriteSwapMode != writeSwapMode)) {
    BDBG_MSG_TRACE(("" BDBG_UINT64_FMT " endian swap: (%s:%s) read %u, write %u", BDBG_UINT64_ARG(CTX_ID(ctx)),
        BSTD_CPU_ENDIAN==BSTD_ENDIAN_LITTLE?"LE":"BE", pSettings->endianMode==BXPT_Dma_EndianMode_eLittle?"LE":"BE", readSwapMode, writeSwapMode));
    }

    /* configure the pid channel */
    reg = 0;
    BCHP_SET_FIELD_DATA(reg, XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE, 1);
    BCHP_SET_FIELD_DATA(reg, XPT_FE_PID_TABLE_i, PLAYBACK_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, dma->channelNum + BXPT_DMA_BAND_ID_OFFSET);
    BCHP_SET_FIELD_DATA(reg, XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, 1); /* PB parser */
    BCHP_SET_FIELD_DATA(reg, XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL, 1); /* direct to XPT security */
    BCHP_SET_FIELD_DATA(reg, XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID, 0); /* the PID does not matter when PID channel is specified via descriptor */
    BREG_Write32(dma->reg, BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + 4 * pSettings->pidChannelNum, reg);

    reg = 0;
    BREG_Write32(dma->reg, BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + 4 * pSettings->pidChannelNum, reg);

    BCHP_SET_FIELD_DATA(reg, XPT_FE_SPID_EXT_TABLE_i, PID_DESTINATION_EXT, 1 << (!pSettings->useRPipe ? 0 : 1));
    BREG_Write32(dma->reg, BCHP_XPT_FE_SPID_EXT_TABLE_i_ARRAY_BASE + 4 * pSettings->pidChannelNum, reg);

    ctx->settings = *pSettings;
    return BERR_SUCCESS;
}

BXPT_Dma_ContextHandle BXPT_Dma_Context_Create(
    BXPT_Dma_Handle hDma,
    const BXPT_Dma_ContextSettings *pSettings
    )
{
    BXPT_Dma_Context *pCtx;
    BXPT_Dma_ContextSettings ctxSettings;
    BERR_Code rc;
    unsigned i, maxNumBlocksMcpb, maxNumBlocksWdma;

    BDBG_OBJECT_ASSERT(hDma, BXPT_Dma_Handle_Tag);

    if (pSettings == NULL) {
        BXPT_Dma_Context_GetDefaultSettings(&ctxSettings);
        pSettings = &ctxSettings;
    }
    if (pSettings->maxNumBlocks == 0) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_settings;
    }
    BDBG_ASSERT(pSettings->endianMode < BXPT_Dma_EndianMode_eMax);
    BDBG_ASSERT(pSettings->swapMode < BXPT_Dma_SwapMode_eMax);

    maxNumBlocksMcpb = pSettings->maxNumBlocks;
    maxNumBlocksWdma = pSettings->maxNumBlocks;
#if BXPT_DMA_WDMA_DESC_CONVERT
    maxNumBlocksWdma *= 2;
#endif
#if BXPT_DMA_WDMA_DESC_APPEND_NULL
    maxNumBlocksWdma += 1;
#endif

    pCtx = BKNI_Malloc(sizeof(BXPT_Dma_Context));
    if (pCtx==NULL) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_settings; }

    BKNI_Memset(pCtx, 0, sizeof(BXPT_Dma_Context));
    BDBG_OBJECT_SET(pCtx, BXPT_Dma_Context);
    pCtx->parent = hDma;
    pCtx->lpp = 1;

#if BXPT_DMA_WDMA_DESC_CONVERT
    pCtx->pOffsets = BKNI_Malloc(sizeof(BXPT_Dma_Context_P_Offsets)*maxNumBlocksMcpb);
    if (pCtx->pOffsets==NULL) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_mem;
    }
    pCtx->pOffsetsWdma = BKNI_Malloc(sizeof(BXPT_Dma_Context_P_Offsets)*maxNumBlocksWdma);
    if (pCtx->pOffsetsWdma==NULL) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_mem;
    }
#endif

    for (i=0; i<2; i++) {
        unsigned alignment = 32;

        pCtx->mcpbBlock[i] = BMMA_Alloc(hDma->mmaHeap, maxNumBlocksMcpb * BXPT_DMA_MCPB_DESC_SIZE, alignment, NULL);
        if (pCtx->mcpbBlock[i] == NULL) { rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY); goto err_mem; }
        pCtx->wdmaBlock[i] = BMMA_Alloc(hDma->mmaHeap, maxNumBlocksWdma * BXPT_DMA_WDMA_DESC_SIZE, alignment, NULL);
        if (pCtx->wdmaBlock[i] == NULL) { rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY); goto err_mem; }

        /* lock all pointers and offsets */
        MCPB_DESC_OFFSET_LOCK(pCtx, i);
        MCPB_DESC_CACHED_LOCK(pCtx, i);
        WDMA_DESC_OFFSET_LOCK(pCtx, i);
        WDMA_DESC_CACHED_LOCK(pCtx, i);
    }

    BDBG_MSG(("" BDBG_UINT64_FMT " create: maxblocks %u", BDBG_UINT64_ARG(CTX_ID(pCtx)), pSettings->maxNumBlocks));
    BDBG_MSG(("  MCPB offset " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT ", cached %p:%p", BDBG_UINT64_ARG(pCtx->firstMcpbDescOffset[0]), BDBG_UINT64_ARG(pCtx->firstMcpbDescOffset[1]), pCtx->firstMcpbDescCached[0], pCtx->firstMcpbDescCached[1]));
    BDBG_MSG(("  WDMA offset " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT ", cached %p:%p", BDBG_UINT64_ARG(pCtx->firstWdmaDescOffset[0]), BDBG_UINT64_ARG(pCtx->firstWdmaDescOffset[1]), pCtx->firstWdmaDescCached[0], pCtx->firstWdmaDescCached[1]));

    rc = BXPT_Dma_Context_P_SetSettings(pCtx, pSettings);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_set_settings; }

    BLST_S_INSERT_HEAD(&hDma->ctxList, pCtx, ctxNode);

#ifdef BCHP_PWR_RESOURCE_DMA
    BCHP_PWR_AcquireResource(pCtx->parent->chp, BCHP_PWR_RESOURCE_DMA);
#endif
    return pCtx;

err_set_settings:
err_mem:
    for (i=0; i<2; i++) {
        if (pCtx->mcpbBlock[i]) {
            BMMA_Free(pCtx->mcpbBlock[i]);
        }
        if (pCtx->wdmaBlock[i]) {
            BMMA_Free(pCtx->wdmaBlock[i]);
        }
    }
#if BXPT_DMA_WDMA_DESC_CONVERT
    if (pCtx->pOffsets) {
        BKNI_Free(pCtx->pOffsets);
    }
    if (pCtx->pOffsetsWdma) {
        BKNI_Free(pCtx->pOffsetsWdma);
    }
#endif
    BKNI_Free(pCtx);
err_settings:
    return NULL;
}

BERR_Code BXPT_Dma_Context_GetSettings(
    BXPT_Dma_ContextHandle hCtx,
    BXPT_Dma_ContextSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(hCtx, BXPT_Dma_Context);
    BDBG_ASSERT(pSettings);

    if (hCtx->state == ctx_state_eDestroyed) {
        return BERR_NOT_SUPPORTED;
    }

    *pSettings = hCtx->settings;
    return BERR_SUCCESS;
}

BERR_Code BXPT_Dma_Context_SetSettings(
    BXPT_Dma_ContextHandle hCtx,
    const BXPT_Dma_ContextSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(hCtx, BXPT_Dma_Context);
    BDBG_ASSERT(pSettings);

    if (hCtx->state != ctx_state_eIdle) {
        return BERR_NOT_SUPPORTED;
    }

    if (pSettings->maxNumBlocks != hCtx->settings.maxNumBlocks) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (pSettings->memoryBounds.offset != hCtx->settings.memoryBounds.offset ||
        pSettings->memoryBounds.size != hCtx->settings.memoryBounds.size) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return BXPT_Dma_Context_P_SetSettings(hCtx, pSettings);
}


static void
BXPT_Dma_Context_P_NotifyFirst_isr(BXPT_Dma_Handle dma, BXPT_Dma_ContextHandle ctx)
{
    BDBG_ASSERT(ctx==BLST_SQ_FIRST(&dma->activeCtxList));

    if (ctx->state == ctx_state_eDestroyed) {
        BLST_SQ_REMOVE_HEAD(&dma->activeCtxList, activeNode);
        return;
    }

    BDBG_ASSERT(ctx->state == ctx_state_eQueued);
    ctx->state = ctx_state_eIdle;
    ctx->cntCompleted++;
    ctx->parent->cntCompleted++;
    ctx->pp = ctx->pp ? 0 : 1;
    BLST_SQ_REMOVE_HEAD(&dma->activeCtxList, activeNode);

    if (ctx->settings.callback_isr) {
        ctx->settings.callback_isr(ctx->settings.pParm1, ctx->settings.pParm2);
    }
    return;
}

static void
BXPT_Dma_P_UpdateStatus_isr(BXPT_Dma_Handle dma)
{
    BXPT_Dma_ContextHandle ctx;
    uint32_t reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_CH0_COMPLETED_DESC_ADDRESS + dma->wdmaRamOffset);
    bool sleep = BXPT_Dma_P_GetWdmaSleep_isrsafe(dma);
    int completedIdx = -1, idx;
    BSTD_UNUSED(sleep);

    /* check for invalid PAUSE states */
{
    uint32_t regs[5];
    regs[0] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_PAUSE_AT_DESC_READ_STATUS_0_31);
    regs[1] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_PAUSE_AT_DESC_END_STATUS_0_31);
    regs[2] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_PAUSE_AFTER_GROUP_PACKETS_STATUS_0_31);
    regs[3] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_MICRO_PAUSE_STATUS_0_31);
    regs[4] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_HW_PAUSE_STATUS_0_31);
    if (regs[0] || regs[1] || regs[2] || regs[3] || regs[4]) {
        BDBG_WRN(("%u: invalid PAUSE state %x %x %x %x %x", dma->channelNum, regs[0], regs[1], regs[2], regs[3], regs[4]));
    }
}

    BDBG_MSG_TRACE(("%u: start update (CDA %#lx)", dma->channelNum, reg));
    for (ctx=BLST_SQ_FIRST(&dma->activeCtxList), idx=0; ctx; ctx=BLST_SQ_NEXT(ctx, activeNode), idx++) {
        uint32_t LAST_WDMA_DESC_OFFSET_INT = LAST_WDMA_DESC_OFFSET(ctx); /* desc with INT set */
#if BXPT_DMA_WDMA_DESC_APPEND_NULL
        LAST_WDMA_DESC_OFFSET_INT -= BXPT_DMA_WDMA_DESC_SIZE;
#endif
        if (reg == LAST_WDMA_DESC_OFFSET_INT) {
            completedIdx = idx;
            break;
        }
        else if (FIRST_WDMA_DESC_OFFSET(ctx)<=reg && reg<LAST_WDMA_DESC_OFFSET_INT) {
            if (idx > 0) {
                completedIdx = idx - 1;
            }
            break;
        }
    }

    if (completedIdx>-1) {
        for (ctx=BLST_SQ_FIRST(&dma->activeCtxList), idx=0; ctx; ctx=BLST_SQ_NEXT(ctx, activeNode), idx++) {
            BDBG_MSG_TRACE(("" BDBG_UINT64_FMT " notify from %s", BDBG_UINT64_ARG(CTX_ID(ctx)), sleep?"sleep":"busy"));
            BXPT_Dma_Context_P_NotifyFirst_isr(dma, ctx);
            if (idx==completedIdx) {
                break;
            }
        }
    }
    else {
        BDBG_MSG_TRACE(("%u: no completions", dma->channelNum, reg));
    }

    if (dma->wakeDisabled) {
        if (!BLST_SQ_EMPTY(&dma->activeCtxList) &&
            (completedIdx!=-1)) /* do not attempt to start unless we've completed the last job */
        {
            BDBG_ASSERT(sleep);
            ctx = BLST_SQ_FIRST(&dma->activeCtxList);

            BXPT_Dma_P_SetWdmaRun_isrsafe(dma, 0);
            BXPT_Dma_P_SetMcpbRun_isrsafe(dma, 0);

            BXPT_Dma_P_IncrementRunVersion_isrsafe(dma);

            BREG_Write32(dma->reg, BCHP_XPT_WDMA_CH0_COMPLETED_DESC_ADDRESS + dma->wdmaRamOffset, 0);
            BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_STATUS + dma->mcpbRegOffset, 0); /* clear DCPM status before RUN */
            BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER + dma->mcpbRegOffset, 0); /* can be used to check if packets were released from MCPB */

            /* always start WDMA before MCPB */
            BREG_Write32(dma->reg, BCHP_XPT_WDMA_CH0_FIRST_DESC_ADDR + dma->wdmaRamOffset, FIRST_WDMA_DESC_OFFSET(ctx));
            BXPT_Dma_P_SetWdmaRun_isrsafe(dma, 1);

            BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DESC_CONTROL + dma->mcpbRegOffset, FIRST_MCPB_DESC_OFFSET(ctx) | BXPT_DMA_MCPB_LLD_TYPE);
            BXPT_Dma_P_SetMcpbRun_isrsafe(dma, 1);

            BDBG_MSG_TRACE(("" BDBG_UINT64_FMT " restart (%u blocks, WDMA desc %#lx)", BDBG_UINT64_ARG(CTX_ID(ctx)), ctx->numBlocks, FIRST_WDMA_DESC_OFFSET(ctx)));

            ctx->cntRun++;
            ctx->parent->cntRun++;
            ctx->lpp = ctx->pp;
            ctx->cntStarted++;
            ctx->parent->cntStarted++;
        }

#if (!BXPT_DMA_AVOID_WAKE)
        if (BLST_SQ_EMPTY(&dma->activeCtxList)) { /* if no more contexts are pending, then we can turn off wake_disable */
            BDBG_MSG(("%u: WAKE enabled", dma->channelNum));
            dma->wakeDisabled = false;
        }
#endif
    }

    BDBG_MSG_TRACE(("%u: end update", dma->channelNum));
    return;
}

static void BXPT_Dma_P_DescDoneCallback_isr(void *pParam1, int parm2)
{
    BXPT_Dma_Handle dma = (BXPT_Dma_Handle)pParam1;
    BSTD_UNUSED(parm2);

    BDBG_OBJECT_ASSERT(dma, BXPT_Dma_Handle_Tag);
    BDBG_MSG_TRACE(("%u: WDMA ISR", dma->channelNum));

#if BXPT_DMA_RATE_TEST
{
    int rc = clock_gettime(CLOCK_MONOTONIC, &dma->wdmaEndTime);
    BDBG_ASSERT(rc==0);

    if (!dma->usePbMcpb) {
        BXPT_Dma_P_RateDump(dma);
    }
}
#endif

    /* ISR could fire after all contexts have been destroyed. short-circuit to avoid register access */
    if (BLST_S_EMPTY(&dma->ctxList)) { return; }

    BXPT_Dma_P_UpdateStatus_isr(dma);
}

static BERR_Code
BXPT_Dma_Context_P_Start_isr(BXPT_Dma_Handle dma, BXPT_Dma_ContextHandle ctx)
{
    BXPT_Dma_ContextHandle lastCtx = dma->lastQueuedCtx;
    BERR_Code rc;
    bool startWithRun = false;

    dma->lastQueuedCtx = ctx;
    BDBG_ASSERT(ctx->lpp!=ctx->pp);

    /* workaround to monitor MCPB FALSE_WAKE interrupt manually */
#if BXPT_DMA_MCPB_FALSE_WAKE_IRQ && 0
{
    uint32_t reg;
    reg = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_MISC_FALSE_WAKE_INTR_L2_CPU_STATUS);
    if ((reg >> dma->channelNum) & 0x1) {
        BDBG_ERR(("%u: " BDBG_UINT64_FMT ": MCPB FALSE_WAKE bit set (%u)", dma->channelNum, BDBG_UINT64_ARG(CTX_ID(ctx)), dma->cntMcpbFalseWake++));
    }
    reg = 1 << dma->channelNum;
    BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_MISC_FALSE_WAKE_INTR_L2_CPU_CLEAR, reg);
}
#endif

    if (dma->wakeDisabled) {
        /* if no active contexts AND in sleep, then we have to start the HW here, since there won't be any ISRs */
        if (BLST_SQ_EMPTY(&dma->activeCtxList) && BXPT_Dma_P_GetWdmaSleep_isrsafe(dma)) {
            startWithRun = true;
        }
    }

    if (lastCtx==NULL || startWithRun) /* no descriptor to append to: first time to start OR last ctx was destroyed */
    {
        if (BXPT_Dma_P_GetWdmaRun_isrsafe(dma)) {
            unsigned runVersion;
            if (BXPT_Dma_P_GetWdmaSleep_isrsafe(dma)) { /* in SLEEP, but don't have a context to wake from */
                BDBG_MSG(("" BDBG_UINT64_FMT " restart from sleep", BDBG_UINT64_ARG(CTX_ID(ctx))));
            }
            else { /* still running. should never occur */
                BDBG_WRN(("" BDBG_UINT64_FMT " restart while busy", BDBG_UINT64_ARG(CTX_ID(ctx))));
            }

            BXPT_Dma_P_SetWdmaRun_isrsafe(dma, 0);
            BXPT_Dma_P_SetMcpbRun_isrsafe(dma, 0);

            runVersion = BXPT_Dma_P_IncrementRunVersion_isrsafe(dma);
            BDBG_MSG(("" BDBG_UINT64_FMT " RUN_VERSION %u", BDBG_UINT64_ARG(CTX_ID(ctx)), runVersion));
        }

        BREG_Write32(dma->reg, BCHP_XPT_WDMA_CH0_COMPLETED_DESC_ADDRESS + dma->wdmaRamOffset, 0);
        BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_STATUS + dma->mcpbRegOffset, 0); /* clear DCPM status before RUN */
        BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER + dma->mcpbRegOffset, 0); /* can be used to check if packets were released from MCPB */

        /* always start WDMA before MCPB */
        BREG_Write32(dma->reg, BCHP_XPT_WDMA_CH0_FIRST_DESC_ADDR + dma->wdmaRamOffset, FIRST_WDMA_DESC_OFFSET(ctx));
        BXPT_Dma_P_SetWdmaRun_isrsafe(dma, 1);

        BREG_Write32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DESC_CONTROL + dma->mcpbRegOffset, FIRST_MCPB_DESC_OFFSET(ctx) | BXPT_DMA_MCPB_LLD_TYPE);
        BXPT_Dma_P_SetMcpbRun_isrsafe(dma, 1);

#if BXPT_DMA_RATE_TEST
        clock_gettime(CLOCK_MONOTONIC, &dma->startTime);
#endif

        BDBG_MSG_TRACE(("" BDBG_UINT64_FMT " start   (%u blocks, WDMA desc " BDBG_UINT64_FMT ")", BDBG_UINT64_ARG(CTX_ID(ctx)), ctx->numBlocks, BDBG_UINT64_ARG(FIRST_WDMA_DESC_OFFSET(ctx))));
        rc = BERR_SUCCESS;
        ctx->cntRun++;
        ctx->parent->cntRun++;
        goto done;
    }
    else {
        /* we can WAKE even if ctx==lastCtx since we have a pingpong scheme */
        uint32_t reg;
        if (dma->wakeDisabled) {
            return BXPT_DMA_QUEUED;
        }

#if BXPT_DMA_DEBUG_MODE
        /* test that LAST flags are still set in the last descriptor */
        BMMA_FlushCache_isr(lastCtx->mcpbBlock[lastCtx->lpp], LAST_MCPB_DESC_CACHED_START(lastCtx), BXPT_DMA_MCPB_DESC_SIZE);
        BMMA_FlushCache_isr(lastCtx->wdmaBlock[lastCtx->lpp], LAST_WDMA_DESC_CACHED_START(lastCtx), BXPT_DMA_WDMA_DESC_SIZE);
        BDBG_ASSERT(LAST_MCPB_DESC_CACHED_START(lastCtx)[2] & MCPB_DW2_LAST_DESC);
        BDBG_ASSERT(LAST_WDMA_DESC_CACHED_START(lastCtx)[3] & WDMA_DW3_LAST);
#endif

        /* sanity check on CDA */
        /* coverity[unreachable] */
        reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_CH0_COMPLETED_DESC_ADDRESS + dma->wdmaRamOffset);
        if (FIRST_WDMA_DESC_OFFSET(ctx) <= reg && reg <= LAST_WDMA_DESC_OFFSET(ctx)) { /* note that LAST_WDMA_DESC_OFFSET is based on numBlocks, not maxNumBlocks */
            BDBG_ERR(("" BDBG_UINT64_FMT ": CDA (%#x) points to descriptor range " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT "", BDBG_UINT64_ARG(CTX_ID(ctx)), reg, BDBG_UINT64_ARG(FIRST_WDMA_DESC_OFFSET(ctx)), BDBG_UINT64_ARG(LAST_WDMA_DESC_OFFSET(ctx))));
            BREG_Write32(dma->reg, BCHP_XPT_WDMA_CH0_COMPLETED_DESC_ADDRESS + dma->wdmaRamOffset, 0);
        }

        /* update NEXT_DESC_ADDR, clear LAST flag, then WAKE. do it for WDMA first */
        LAST_WDMA_DESC_CACHED_START(lastCtx)[3] = FIRST_WDMA_DESC_OFFSET(ctx) | lastCtx->wdmaDescW3;
        BMMA_FlushCache_isr(lastCtx->wdmaBlock[lastCtx->lpp], LAST_WDMA_DESC_CACHED_START(lastCtx), BXPT_DMA_WDMA_DESC_SIZE);
#if BXPT_DMA_MSA_DEBUG_MODE
        BXPT_Dma_P_AssertWdmaDescMem(ctx, true);
#endif
        BXPT_Dma_P_SetWdmaWake_isrsafe(dma, 1);

        /* do MCPB next */
        LAST_MCPB_DESC_CACHED_START(lastCtx)[2] = FIRST_MCPB_DESC_OFFSET(ctx);
        BMMA_FlushCache_isr(lastCtx->mcpbBlock[lastCtx->lpp], LAST_MCPB_DESC_CACHED_START(lastCtx), BXPT_DMA_MCPB_DESC_SIZE);
#if BXPT_DMA_MSA_DEBUG_MODE
        BXPT_Dma_P_AssertMcpbDescMem(ctx, true);
#endif
        BXPT_Dma_P_SetMcpbWake_isrsafe(dma, 1);

        BDBG_MSG_TRACE(("" BDBG_UINT64_FMT " append (%u blocks, WDMA desc " BDBG_UINT64_FMT ")", BDBG_UINT64_ARG(CTX_ID(ctx)), ctx->numBlocks, BDBG_UINT64_ARG(FIRST_WDMA_DESC_OFFSET(ctx))));

#if BXPT_DMA_DESC_DUMP
        BXPT_Dma_Context_P_DescDump(lastCtx, true, false);
#endif
        rc = BXPT_DMA_QUEUED;
        goto done;
    }
done:
    ctx->lpp = ctx->pp;
    ctx->cntStarted++;
    ctx->parent->cntStarted++;
    return rc;
}

static BERR_Code BXPT_Dma_Context_P_PrepareBlocks(BXPT_Dma_ContextHandle ctx, const BXPT_Dma_ContextBlockSettings *pSettings, unsigned numBlocks)
{
    unsigned i, dmaLength;
    uint32_t *desc;
    uint32_t nextDescOffset;
    BXPT_Dma_ContextHandle context;
#if (!BXPT_DMA_AVOID_WAKE)
    bool wakeDisabled = false;
#endif
#if 0
    bool sgStart = false;
#endif
    unsigned numBlocksMcpb, numBlocksWdma;
#if BXPT_DMA_WDMA_DESC_CONVERT
    BXPT_Dma_Context_P_Offsets *pOffsets = ctx->pOffsets;
    BXPT_Dma_Context_P_Offsets *pOffsetsWdma = ctx->pOffsetsWdma;
#endif

    BDBG_ASSERT(numBlocks <= ctx->settings.maxNumBlocks); /* checked prior */

    /* do checks and tests first */
    for (i=0, dmaLength=0; i<numBlocks; i++, pSettings++)
    {
        if (pSettings->size==0) {
            BDBG_ERR(("" BDBG_UINT64_FMT ": transfer size of 0 is not allowed", BDBG_UINT64_ARG(CTX_ID(ctx))));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        if ((pSettings->src >> BXPT_DMA_ADDRESS_LIMIT_SHIFT) || (pSettings->dst >> BXPT_DMA_ADDRESS_LIMIT_SHIFT)) {
            BDBG_ERR(("" BDBG_UINT64_FMT ":%u invalid src address " BDBG_UINT64_FMT " or dst address " BDBG_UINT64_FMT "", BDBG_UINT64_ARG(CTX_ID(ctx)), i, BDBG_UINT64_ARG(pSettings->src), BDBG_UINT64_ARG(pSettings->dst)));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

#if BXPT_DMA_DESC_PROTECT
    {
        /* check if queued descriptor reads from or writes to memory locations where contexts' descriptors are located.
           only the descriptor memories for this channel is tested */
        for (context=BLST_S_FIRST(&ctx->parent->ctxList); context; context=BLST_S_NEXT(context, ctxNode)) {
            uint64_t mcpb_beg, mcpb_end, wdma_beg, wdma_end; /* beg is inclusive, end is not */
            mcpb_beg = FIRST_MCPB_DESC_OFFSET(context);
            mcpb_end = FIRST_MCPB_DESC_OFFSET(context) + (context->numBlocks * BXPT_DMA_MCPB_DESC_SIZE);
            wdma_beg = FIRST_WDMA_DESC_OFFSET(context);
            wdma_end = FIRST_WDMA_DESC_OFFSET(context) + (context->numBlocks * BXPT_DMA_WDMA_DESC_SIZE);

            if ((pSettings->src >= mcpb_beg && pSettings->src < mcpb_end) ||
                (pSettings->src >= wdma_beg && pSettings->src < wdma_end))
            {
                BDBG_WRN(("" BDBG_UINT64_FMT ":%u src address " BDBG_UINT64_FMT " READS from BXPT_DMA private descriptor memory location", BDBG_UINT64_ARG(CTX_ID(ctx)), i, BDBG_UINT64_ARG(pSettings->src)));
                /* warn, but continue */
            }

            if ((pSettings->dst >= mcpb_beg && pSettings->dst < mcpb_end) ||
                (pSettings->dst >= wdma_beg && pSettings->dst < wdma_end))
            {
                BDBG_ERR(("" BDBG_UINT64_FMT ":%u dst address " BDBG_UINT64_FMT " WRITES to BXPT_DMA private descriptor memory location", BDBG_UINT64_ARG(CTX_ID(ctx)), i, BDBG_UINT64_ARG(pSettings->dst)));
                return BERR_TRACE(BERR_INVALID_PARAMETER); /* don't allow */
            }
        }
    }
#endif

#if BXPT_DMA_DISABLE_WAKE_ON_OVERLAP
    {
        unsigned j;
        for (context=BLST_SQ_FIRST(&ctx->parent->activeCtxList); context; context=BLST_SQ_NEXT(context, activeNode)) {
            /* is this context trying to read from an unfinished context's dest location? */
            unsigned numBlocks = context->numBlocks;
#if (BXPT_DMA_WDMA_DESC_CONVERT || BXPT_DMA_WDMA_DESC_APPEND_NULL)
            numBlocks = context->numBlocksWdma;
#endif

            for (j=0; j<numBlocks; j++) {
                uint64_t word0 = ((uint32_t*)(FIRST_WDMA_DESC_CACHED(context)))[BXPT_DMA_WDMA_DESC_WORDS*j + 0];
                uint64_t word1 = ((uint32_t*)(FIRST_WDMA_DESC_CACHED(context)))[BXPT_DMA_WDMA_DESC_WORDS*j + 1];
                uint64_t word2 = ((uint32_t*)(FIRST_WDMA_DESC_CACHED(context)))[BXPT_DMA_WDMA_DESC_WORDS*j + 2];
                uint64_t dst = (word0 & 0xff) << 32 | word1;
                uint64_t size = word2;
                /* beg is inclusive, end is not */
                uint64_t dst_beg = dst;
                uint64_t dst_end = (dst + size) ? (dst + size - 1) : 0;
                if ((pSettings->src >= dst_beg && pSettings->src < dst_end) ||
                    (pSettings->src+pSettings->size-1 >= dst_beg && pSettings->src+pSettings->size-1 < dst_end))
                {
                    BDBG_MSG(("Overlap between queued ctx dest (" BDBG_UINT64_FMT " - " BDBG_UINT64_FMT ") and current ctx src (" BDBG_UINT64_FMT " - " BDBG_UINT64_FMT ")",
                        BDBG_UINT64_ARG(dst_beg), BDBG_UINT64_ARG(dst_end), BDBG_UINT64_ARG(pSettings->src), BDBG_UINT64_ARG(pSettings->src+pSettings->size-1)));
#if (!BXPT_DMA_AVOID_WAKE)
                    /* if there's overlap, then we turn off WAKE for this
                     * channel until it's idle again */
                    wakeDisabled = true;
#endif
                }
            }
        }
    }
#endif

        if (ctx->settings.memoryBounds.offset && ctx->settings.memoryBounds.size) {
            uint64_t b_offset = ctx->settings.memoryBounds.offset;
            unsigned b_size = ctx->settings.memoryBounds.size;

            if (pSettings->src < b_offset || (pSettings->src+pSettings->size > b_offset+b_size)) {
                BDBG_ERR(("" BDBG_UINT64_FMT ":%u src address violates memory bounds " BDBG_UINT64_FMT ":%#x " BDBG_UINT64_FMT ":%#x", BDBG_UINT64_ARG(CTX_ID(ctx)), i,
                    BDBG_UINT64_ARG(pSettings->src), pSettings->size, BDBG_UINT64_ARG(b_offset), b_size));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }

            if (pSettings->dst < b_offset || (pSettings->dst+pSettings->size > b_offset+b_size)) {
                BDBG_ERR(("" BDBG_UINT64_FMT ":%u dst address violates memory bounds " BDBG_UINT64_FMT ":%#x " BDBG_UINT64_FMT ":%#x", BDBG_UINT64_ARG(CTX_ID(ctx)), i,
                    BDBG_UINT64_ARG(pSettings->dst), pSettings->size, BDBG_UINT64_ARG(b_offset), b_size));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        }

        if (pSettings->size==0 && (pSettings->resetCrypto || pSettings->sgScramStart || pSettings->sgScramEnd)) {
            BDBG_ERR(("" BDBG_UINT64_FMT ":%u resetCrypto, sgScramStart and sgScramEnd flags not allowed with size 0", BDBG_UINT64_ARG(CTX_ID(ctx)), i));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
#if 0
        if (pSettings->resetCrypto && !pSettings->sgScramStart) {
            BDBG_WRN(("" BDBG_UINT64_FMT ":%u resetCrypto flag ignored when not paired with sgScramStart", BDBG_UINT64_ARG(CTX_ID(ctx)), i));
            /* continue but don't set the flag in the descriptor */
        }
#endif
#if 0
        if (pSettings->sgScramStart) {
            if (sgStart) {
                BDBG_ERR(("" BDBG_UINT64_FMT ":%u unpaired sgScramStart flag", BDBG_UINT64_ARG(CTX_ID(ctx)), i));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            sgStart = true;
        }
        if (pSettings->sgScramEnd) {
            if (!sgStart) {
                BDBG_ERR(("" BDBG_UINT64_FMT ":%u unpaired sgScramEnd flag", BDBG_UINT64_ARG(CTX_ID(ctx)), i));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            sgStart = false;
        }

        if (i==numBlocks-1 && sgStart) {
            BDBG_ERR(("" BDBG_UINT64_FMT ":%u unpaired sgScramStart flag", BDBG_UINT64_ARG(CTX_ID(ctx)), i));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
#endif
        dmaLength += pSettings->size;
    }

#if BXPT_DMA_PREVENT_4BYTE_TRANSFERS
    if (dmaLength<=4) {
        BDBG_ERR(("" BDBG_UINT64_FMT ": total transfer size must be larger than 4 bytes", BDBG_UINT64_ARG(CTX_ID(ctx))));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
#endif

    if (ctx->parent->settings.scramMode!=BXPT_Dma_ScramMode_eBlock) {
        unsigned validTransferSize = 0;
        switch (ctx->parent->settings.scramMode) {
            case BXPT_Dma_ScramMode_eMpeg: validTransferSize = 188; break;
            case BXPT_Dma_ScramMode_eDss: validTransferSize = 130; break;
            default: BDBG_ASSERT(0);
        }
        if (ctx->parent->settings.timestampEnabled) {
            validTransferSize += 4;
        }
        if (dmaLength % validTransferSize) {
            BDBG_ERR(("" BDBG_UINT64_FMT ": invalid transfer size %u, expected multiple of %u (mode %u, timestamp %u)", BDBG_UINT64_ARG(CTX_ID(ctx)), dmaLength, validTransferSize, ctx->parent->settings.scramMode, ctx->parent->settings.timestampEnabled));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    ctx->dmaLength = dmaLength;
    ctx->numBlocks = numBlocks;
    pSettings -= numBlocks;

    /* setup MCPB descriptors */
    numBlocksMcpb = numBlocks;
    for (i=0, desc=FIRST_MCPB_DESC_CACHED(ctx), nextDescOffset=FIRST_MCPB_DESC_OFFSET(ctx); i<numBlocksMcpb; i++, pSettings++)
    {
        nextDescOffset += BXPT_DMA_MCPB_DESC_SIZE;
        BDBG_ASSERT((nextDescOffset & 0x1F) == 0); /* 5 LSBs must be 0 */

#if BXPT_DMA_USE_40BIT_ADDRESSES
        desc[0] = (pSettings->src >> 32) & 0xFF; /* BUFF_ST_RD_ADDR [39:32] */
        desc[1] = pSettings->src & 0xFFFFFFFF; /* BUFF_ST_RD_ADDR [31:0] */
#else
        desc[0] = 0; /* BUFF_ST_RD_ADDR [39:32] */
        desc[1] = pSettings->src & 0xFFFFFFFF; /* BUFF_ST_RD_ADDR [31:0] */
#endif
        desc[2] = nextDescOffset; /* NEXT_DESC_ADDR [31:5] */
        desc[3] = pSettings->size; /* BUFF_SIZE [31:0] */
        desc[4] = ctx->mcpbDescW4;
        desc[5] = ctx->mcpbDescW5;
        desc[6] = ctx->mcpbDescW6;
        desc[7] = 0;

        if (pSettings->resetCrypto && pSettings->sgScramStart) { /* ignore resetCrypto when not paired with sgScramStart */
            desc[5] |= MCPB_DW5_SCRAM_INIT;
        }
        if (pSettings->sgScramStart) {
            desc[5] |= MCPB_DW5_SCRAM_START;
        }
        if (pSettings->sgScramEnd) {
            desc[5] |= MCPB_DW5_SCRAM_END;
        }
        if (pSettings->securityBtp) {
            desc[4] |= MCPB_DW4_HOST_DATA_INS_EN; /* this is generic */
            desc[5] |= MCPB_DW5_HOST_INS_DATA_AS_BTP_PKT; /* the scope of this is very specific to BTPs processed by security engine */
        }

        if (i==numBlocksMcpb-1) {
            desc[2] = 0; /* set nextDescOffset to 0 */
            desc[2] |= MCPB_DW2_LAST_DESC;
            if (!pSettings->sgScramEnd) { /* PUSH_PARTIAL_PKT not supported with sgScram */
                desc[4] |= MCPB_DW4_PUSH_PARTIAL_PKT;
            }

#if BXPT_DMA_MCPB_DESC_DONE_IRQ
            desc[4] |= MCPB_DW4_INT_EN; /* normally, we're only interested in the WDMA interrupt, not the MCPB interrupt */
#endif

            ctx->lastMcpbDescCached[ctx->pp] = desc;
        }

        desc += BXPT_DMA_MCPB_DESC_WORDS;
    }

    pSettings -= numBlocksMcpb;

    numBlocksWdma = numBlocks;
#if BXPT_DMA_WDMA_DESC_CONVERT
{
    unsigned j, remainder, deferredSum = 0;

    for (i=0; i<numBlocks; i++) {
        pOffsets[i].size = pSettings[i].size;
        pOffsets[i].dst = pSettings[i].dst;
    }

    for (i=0, j=0; i<numBlocks; i++, j++) {
        pOffsetsWdma[j].size = pOffsets[i].size;
        pOffsetsWdma[j].dst = pOffsets[i].dst;
        if (i==numBlocks-1) {
            j++;
            break;
        }

        remainder = (deferredSum + pOffsets[i].size) % 4;
        if (remainder) {
            unsigned fill = 4 - remainder;
            BDBG_ASSERT(pOffsets[i+1].size==pSettings[i+1].size);
            if (pOffsets[i+1].size >= fill) { /* borrow from next descriptor to create an extra, intermediate desriptor */
                pOffsetsWdma[j+1].size = fill;
                pOffsetsWdma[j+1].dst = pOffsets[i+1].dst;
                if (pOffsets[i+1].size > fill) {
                    pOffsets[i+1].size -= fill;
                    pOffsets[i+1].dst += fill;
                }
                else { /* avoid creating a zero-sized intermediate descriptor */
                    i++;
                }
                j++;
                deferredSum = 0;
            }
            else {
                deferredSum += pOffsets[i].size;
            }
        }
    }

    if (deferredSum) {
        BDBG_WRN(("Unable to convert descriptor sizes for WDMA alignment"));
        for (i=0; i<numBlocks; i++) {
            BDBG_WRN(("%u", UINT32_V(pSettings[i].size)));
        }
    }

    numBlocksWdma = ctx->numBlocksWdma = j;

#if 0 /* debug print */
    for (i=0; i<numBlocks; i++) {
        BKNI_Printf("%u ", UINT32_V(pSettings[i].size));
    }
    BKNI_Printf("=> ");
    for (i=0; i<numBlocksWdma; i++) {
        BKNI_Printf("%u ", UINT32_V(pOffsetsWdma[i].size));
    }
    BKNI_Printf("\n");
#endif
}
#endif /* BXPT_DMA_WDMA_DESC_CONVERT */

    /* setup WDMA descriptors */
    for (i=0, desc=FIRST_WDMA_DESC_CACHED(ctx), nextDescOffset=FIRST_WDMA_DESC_OFFSET(ctx); i<numBlocksWdma; i++, pSettings++)
    {
        uint64_t dst;
        uint32_t size;
#if BXPT_DMA_WDMA_DESC_CONVERT
        dst = pOffsetsWdma->dst;
        size = pOffsetsWdma->size;
        pOffsetsWdma++;
#else
        BDBG_ASSERT(i<numBlocksMcpb);
        dst = pSettings->dst;
        size = pSettings->size;
#endif
        nextDescOffset += BXPT_DMA_WDMA_DESC_SIZE;
        BDBG_ASSERT((nextDescOffset & 0xF) == 0); /* 4 LSBs must be 0 */

#if BXPT_DMA_USE_40BIT_ADDRESSES
        desc[0] = (dst >> 32) & 0xFF; /* write address [39:32] */
        desc[1] = dst & 0xFFFFFFFF; /* write address [31:0] */
#else
        desc[0] = 0; /* write address [39:32] */
        desc[1] = dst & 0xFFFFFFFF; /* write address [31:0] */
#endif
        desc[2] = size; /* transfer size [31:0] */

        BDBG_ASSERT((ctx->wdmaDescW3 & 0xFFFFFFF0) == 0); /* sanity check on prototype */
        desc[3] = nextDescOffset | ctx->wdmaDescW3;

        if (i==numBlocksWdma-1) {
#if (!BXPT_DMA_WDMA_DESC_APPEND_NULL)
            desc[3] = ctx->wdmaDescW3;
            desc[3] |= WDMA_DW3_INTR_EN | WDMA_DW3_LAST;

            ctx->lastWdmaDescCached[ctx->pp] = desc;
#else
            desc[3] |= WDMA_DW3_INTR_EN; /* keep address to nextDesc and only set INT flag */
#endif
        }

        desc += BXPT_DMA_WDMA_DESC_WORDS;
    }

#if BXPT_DMA_WDMA_DESC_APPEND_NULL
    ctx->numBlocksWdma = ++numBlocksWdma;

    desc[0] = 0;
    desc[1] = 0;
    desc[2] = 0; /* size 0 */
    desc[3] = ctx->wdmaDescW3 | WDMA_DW3_LAST;

    ctx->lastWdmaDescCached[ctx->pp] = desc;
#endif

    BMMA_FlushCache(ctx->mcpbBlock[ctx->pp], FIRST_MCPB_DESC_CACHED(ctx), numBlocksMcpb*BXPT_DMA_MCPB_DESC_SIZE);
    BMMA_FlushCache(ctx->wdmaBlock[ctx->pp], FIRST_WDMA_DESC_CACHED(ctx), numBlocksWdma*BXPT_DMA_WDMA_DESC_SIZE);
#if BXPT_DMA_MSA_DEBUG_MODE
    BXPT_Dma_P_AssertMcpbDescMem(ctx, false);
    BXPT_Dma_P_AssertWdmaDescMem(ctx, false);
#endif

#if (!BXPT_DMA_AVOID_WAKE)
    if (wakeDisabled && !ctx->parent->wakeDisabled) {
        ctx->parent->wakeDisabled = true;
        BDBG_MSG(("%u: WAKE disabled", ctx->parent->channelNum));
    }
#endif

    return BERR_SUCCESS;
}

BERR_Code BXPT_Dma_Context_Enqueue(
    BXPT_Dma_ContextHandle hCtx,
    const BXPT_Dma_ContextBlockSettings *pSettings,
    unsigned numBlocks
    )
{
    BERR_Code rc;
    BXPT_Dma_Handle dma;
    BDBG_ASSERT(hCtx);
    BDBG_ASSERT(hCtx->parent);
    BDBG_OBJECT_ASSERT(hCtx, BXPT_Dma_Context);

    if (hCtx->parent->standby) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if (numBlocks==0 || numBlocks > hCtx->settings.maxNumBlocks ) {
        BDBG_ERR(("numBlocks %u out of range. max=%u", numBlocks, hCtx->settings.maxNumBlocks));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* a context can only be enqueued once at a time */
    if (hCtx->state != ctx_state_eIdle) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if (pSettings==NULL) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    rc = BXPT_Dma_Context_P_PrepareBlocks(hCtx, pSettings, numBlocks);
    if (rc!=BERR_SUCCESS) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

#if BXPT_DMA_DESC_DUMP
    #if 0
    BXPT_Dma_Context_P_BlockSettingsDump(hCtx, pSettings);
    #endif
    BXPT_Dma_Context_P_DescDump(hCtx, false, false);
#endif

    dma = hCtx->parent;

    BKNI_EnterCriticalSection();
    BXPT_Dma_Context_P_Start_isr(dma, hCtx);
    hCtx->state = ctx_state_eQueued;

    /* only if there are no other queued contexts, can we possibly short-circuit. otherwise we have to traverse through activeCtxList */
    if (BLST_SQ_FIRST(&dma->activeCtxList)==NULL) {
        uint32_t reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_CH0_COMPLETED_DESC_ADDRESS + dma->wdmaRamOffset);
        if (reg == LAST_WDMA_DESC_OFFSET(hCtx)) {
            BDBG_MSG_TRACE(("" BDBG_UINT64_FMT " fast complete", CTX_ID(hCtx)));
            hCtx->state = ctx_state_eIdle; /* no need to add to list */
            hCtx->cntCompleted++;
            hCtx->parent->cntCompleted++;
            hCtx->pp = hCtx->pp ? 0 : 1;
            BKNI_LeaveCriticalSection();
            return BERR_SUCCESS;
        }
    }

    BLST_SQ_INSERT_TAIL(&dma->activeCtxList, hCtx, activeNode);
    BKNI_LeaveCriticalSection();

    return BXPT_DMA_QUEUED;
}

BERR_Code BXPT_Dma_Context_GetStatus(
    BXPT_Dma_ContextHandle hCtx,
    BXPT_Dma_ContextStatus *pStatus
    )
{
    BDBG_OBJECT_ASSERT(hCtx, BXPT_Dma_Context);
    BDBG_ASSERT(NULL != pStatus);

    if (hCtx->state == ctx_state_eDestroyed) {
        return BERR_NOT_SUPPORTED;
    }

    if (hCtx->state != ctx_state_eIdle) {
        BKNI_EnterCriticalSection();
        BDBG_MSG_TRACE(("" BDBG_UINT64_FMT " GetStatus", CTX_ID(hCtx)));
        BXPT_Dma_P_UpdateStatus_isr(hCtx->parent);
        BKNI_LeaveCriticalSection();
    }

    if (hCtx->state == ctx_state_eIdle) {
        pStatus->state = BXPT_Dma_ContextState_eIdle;
    }
    else {
        pStatus->state = BXPT_Dma_ContextState_eInProgress;
    }
    return BERR_SUCCESS;
}

void BXPT_Dma_Context_P_Destroy(BXPT_Dma_ContextHandle ctx)
{
    unsigned i;
    if (ctx->parent->lastQueuedCtx == ctx) { /* P_Start_isr() cannot use this context to wake any more */
        ctx->parent->lastQueuedCtx = NULL;
    }
    BDBG_MSG_TRACE(("" BDBG_UINT64_FMT " destroy (start %u, complete %u, run %u)", BDBG_UINT64_ARG(CTX_ID(ctx)), ctx->cntStarted, ctx->cntCompleted, ctx->cntRun));
    BLST_S_REMOVE(&(ctx->parent->ctxList), ctx, BXPT_Dma_Context, ctxNode);

    for (i=0; i<2; i++) {
        MCPB_DESC_OFFSET_UNLOCK(ctx, i);
        MCPB_DESC_CACHED_UNLOCK(ctx, i);
        WDMA_DESC_OFFSET_UNLOCK(ctx, i);
        WDMA_DESC_CACHED_UNLOCK(ctx, i);
        BMMA_Free(ctx->mcpbBlock[i]);
        BMMA_Free(ctx->wdmaBlock[i]);
    }
#ifdef BCHP_PWR_RESOURCE_DMA
    BCHP_PWR_ReleaseResource(ctx->parent->chp, BCHP_PWR_RESOURCE_DMA);
#endif
#if BXPT_DMA_WDMA_DESC_CONVERT
    BKNI_Free(ctx->pOffsets);
    BKNI_Free(ctx->pOffsetsWdma);
#endif

    BDBG_OBJECT_DESTROY(ctx, BXPT_Dma_Context);
    BKNI_Free(ctx);
}

void BXPT_Dma_Context_Destroy(
    BXPT_Dma_ContextHandle hCtx
    )
{
    BXPT_Dma_ContextStatus status;
    BDBG_OBJECT_ASSERT(hCtx, BXPT_Dma_Context);
    BXPT_Dma_Context_GetStatus(hCtx, &status);

    /* if not idle, then mark it as destroyed, but defer the actual destroy to BXPT_Dma_CloseChannel() */
    if (hCtx->state!=ctx_state_eIdle) {
        BDBG_ERR(("BXPT_Dma_Context_Destroy: " BDBG_UINT64_FMT " is busy, leaking context", BDBG_UINT64_ARG(CTX_ID(hCtx))));
        hCtx->state = ctx_state_eDestroyed;
    }
    else {
        /* otherwise, really destroy */
        BXPT_Dma_Context_P_Destroy(hCtx);
    }
}

void BXPT_Dma_CloseChannel(
    BXPT_Dma_Handle hDma
    )
{
    BXPT_Dma_ContextHandle ctx;
    uint32_t reg;
    bool isSageCh = (BXPT_DMA_RESERVE_SAGE_CH && hDma->channelNum==BXPT_DMA_SAGE_CH_NUM);
    BDBG_OBJECT_ASSERT(hDma, BXPT_Dma_Handle_Tag);

    /* wait for pending operations to finish */
    if (BLST_SQ_FIRST(&hDma->activeCtxList)) { /* Context_Destroy() never actually destroys any contexts that are part of activeCtxList */
        BDBG_WRN(("%u: XPT_Dma still busy. Waiting before forced stop...", hDma->channelNum));
        for (ctx=BLST_SQ_FIRST(&hDma->activeCtxList); ctx; ctx=BLST_SQ_NEXT(ctx, activeNode)) {
            BDBG_WRN(("active context " BDBG_UINT64_FMT "", BDBG_UINT64_ARG(CTX_ID(ctx))));
        }

        BKNI_Sleep(100);
    }

    /* both RUN bits are still 1, so clear both */
    BXPT_Dma_P_SetMcpbRun_isrsafe(hDma, 0);
    BXPT_Dma_P_SetWdmaRun_isrsafe(hDma, 0);

#if BXPT_DMA_WDMA_ENDIAN_SWAP_CBIT
    {
        unsigned setCbit = 0;
        reg = BREG_Read32(hDma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL + hDma->mcpbRegOffset);
        BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL, ENDIAN_CONTROL, setCbit);
        BREG_Write32(hDma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL + hDma->mcpbRegOffset, reg);

        reg = BREG_Read32(hDma->reg, BCHP_XPT_WDMA_CH0_DATA_CONTROL + hDma->wdmaRamOffset);
        BCHP_SET_FIELD_DATA(reg, XPT_WDMA_CH0_DATA_CONTROL, INV_STRAP_ENDIAN_CTRL, setCbit);
        BREG_Write32(hDma->reg, BCHP_XPT_WDMA_CH0_DATA_CONTROL + hDma->wdmaRamOffset, reg);
    }
#endif

    if (isSageCh && hDma->bint==NULL) {
        goto postint;
    }

    BINT_DestroyCallback(hDma->irqDescDone);
    BINT_DestroyCallback(hDma->irqOverflow);
    BINT_DestroyCallback(hDma->irqBtp);
    if (hDma->irqMcpbDescDone) {
        BINT_DestroyCallback(hDma->irqMcpbDescDone);
    }
    if (hDma->irqMcpbFalseWake) {
        BINT_DestroyCallback(hDma->irqMcpbFalseWake);
    }

postint:
    /* all pending operations are done. shutdown */
    while ((ctx = BLST_S_FIRST(&hDma->ctxList))) {
        BDBG_WRN(("BXPT_Dma_CloseChannel: stale context " BDBG_UINT64_FMT " (%s)", BDBG_UINT64_ARG(CTX_ID(ctx)), ctx->state==ctx_state_eIdle?"idle":"active"));
        BXPT_Dma_Context_P_Destroy(ctx);
    }

    /* disable after all ops are done */
    reg = BREG_Read32(hDma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL + hDma->mcpbRegOffset);
    BCHP_SET_FIELD_DATA(reg, XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL, PARSER_ENABLE, 0);
#if (!BXPT_DMA_HAS_MEMDMA_MCPB)
    BCHP_SET_FIELD_DATA(reg, XPT_MCPB_CH0_SP_PARSER_CTRL, MEMDMA_PIPE_EN, 0);
#endif
    BREG_Write32(hDma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL + hDma->mcpbRegOffset, reg);
    if (hDma->channelNum >= BXPT_DMA_NUM_FRONT_SHARED_CH) {
        BREG_Write32(hDma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_TMEU_BLOCKOUT_CTRL + hDma->mcpbRegOffset, BXPT_DMA_PWR_BO_COUNT);
    }

    if (!isSageCh) {
        hDma->xpt->dmaChannels[hDma->channelNum] = NULL;
    }

    BDBG_OBJECT_UNSET(hDma, BXPT_Dma_Handle_Tag);
    BKNI_Free(hDma);
}

BERR_Code BXPT_P_Dma_Standby(BXPT_Handle hXpt)
{
    BERR_Code rc = BERR_SUCCESS;
    unsigned i;
    BXPT_Dma_Handle dma;

    for (i=0; i<BXPT_DMA_NUM_CHANNELS; i++) {
        dma = hXpt->dmaChannels[i];
        if (!dma) { continue; }

        /* cannot enter standby while dma is busy */
        if (BLST_SQ_FIRST(&dma->activeCtxList)) {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }

    for (i=0; i<BXPT_DMA_NUM_CHANNELS; i++) {
        dma = hXpt->dmaChannels[i];
        if (!dma) { continue; }

        rc = BINT_DisableCallback(dma->irqDescDone);
        if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); }
        rc = BINT_DisableCallback(dma->irqOverflow);
        if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); }
        rc = BINT_DisableCallback(dma->irqBtp);
        if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); }
    }

    return rc;
}

void BXPT_P_Dma_Resume(BXPT_Handle hXpt)
{
    BERR_Code rc = BERR_SUCCESS;
    unsigned i;
    BXPT_Dma_Handle dma;
    uint32_t reg;

    /* re-apply some common, fixed settings */
    reg = BREG_Read32(hXpt->hRegister, BCHP_XPT_WDMA_REGS_DATA_STALL_TIMEOUT);
    BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_DATA_STALL_TIMEOUT, TIMEOUT_CLOCKS, 0xA4CB80); /* 108M = 100ms */
    BREG_Write32(hXpt->hRegister, BCHP_XPT_WDMA_REGS_DATA_STALL_TIMEOUT, reg);

    for (i=0; i<BXPT_DMA_NUM_CHANNELS; i++) {
        /* clear any RUNs set by BOLT / linux */
        reg = 0;
        BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_RUN_SET_CLEAR, CHANNEL_NUM, i);
        BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_RUN_SET_CLEAR, SET_CLEAR, 0);
        BREG_Write32(hXpt->hRegister, BCHP_XPT_WDMA_REGS_RUN_SET_CLEAR, reg);

        dma = hXpt->dmaChannels[i];
        if (!dma) { continue; }

        (void) BXPT_Dma_P_SetSettings(dma, &dma->settings);

#if BXPT_DMA_USE_RUN_VERSION
        reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_MATCH_RUN_VERSION_CONFIG);
        BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_MATCH_RUN_VERSION_CONFIG, CHANNEL_NUM, i);
        BCHP_SET_FIELD_DATA(reg, XPT_WDMA_REGS_MATCH_RUN_VERSION_CONFIG, CONFIG, 1); /* enable RUN_VERSION_MATCH */
        BREG_Write32(dma->reg, BCHP_XPT_WDMA_REGS_MATCH_RUN_VERSION_CONFIG, reg);
        BXPT_Dma_P_IncrementRunVersion_isrsafe(dma);
#endif
    }

    for (i=0; i<BXPT_DMA_NUM_CHANNELS; i++) {
        dma = hXpt->dmaChannels[i];
        if (!dma) { continue; }

        rc = BINT_EnableCallback(dma->irqDescDone);
        if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); }
        rc = BINT_EnableCallback(dma->irqOverflow);
        if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); }
        rc = BINT_EnableCallback(dma->irqBtp);
        if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); }
    }

    return;
}

void BXPT_Dma_Context_P_DescCheckMcpb(BXPT_Dma_ContextHandle ctx, unsigned idx, bool link_only, uint32_t *desc)
{
    unsigned numBlocks = ctx->numBlocks;
    if (desc[0] & 0xFFFFFF00) { /* [31:8] RESERVED */
        BDBG_ERR(("MCPB DW0: non-zero value in reserved field"));
    }
    if (idx==numBlocks-1 && !link_only) {
        if ((desc[2] & MCPB_DW2_LAST_DESC) == 0) {
            BDBG_ERR(("MCPB DW2: LAST_DESC flag not set in last descriptor"));
        }
    }
    else {
        if ((desc[2] & MCPB_DW2_LAST_DESC) == MCPB_DW2_LAST_DESC) {
            BDBG_ERR(("MCPB DW2: LAST_DESC flag set in non-last descriptor"));
        }
#if (!BXPT_DMA_MCPB_DESC_DONE_IRQ)
        if ((desc[4] & MCPB_DW4_INT_EN) == (unsigned)MCPB_DW4_INT_EN) {
            BDBG_ERR(("MCPB DW4: INT_EN flag set in non-last descriptor, in non-debug mode"));
        }
#endif
    }
    if (desc[4] & 0x03FE0000) { /* [25:17] RESERVED */
        BDBG_ERR(("MCPB DW4: non-zero value in reserved field"));
    }
    if (desc[4] & 0x0001FFFF) { /* [16:0] unused */
        BDBG_ERR(("MCPB DW4: non-zero value in unused field"));
    }
    if (desc[5] & 0xFFC00000) { /* [31:22] RESERVED */
        BDBG_ERR(("MCPB DW5: non-zero value in reserved field"));
    }
    if ((desc[5] & MCPB_DW5_PID_CHANNEL_VALID) == 0) {
        BDBG_ERR(("MCPB DW5: PID_CHANNEL_VALID flag not set"));
    }

    if (desc[6] & 0xFFFFFC00) { /* [31:10] RESERVED */
        BDBG_ERR(("MCPB DW6: non-zero value in reserved field"));
    }
    if ((desc[6] & 0x3FF) < BXPT_DMA_PID_CHANNEL_NUM_START) { /* [9:0] PID_CHANNEL */
        BDBG_ERR(("MCPB DW6: PID_CHANNEL invalid"));
    }

    /* other things that could be tested:
       - same PID_CHANNEL value for all descs
       - same RD_ENDIAN_STRAP_INV_CTRL flag for all descs
       - SCRAM_INIT, SCRAM_START, SCRAM_END correctness (but already tested in P_PrepareBlocks())
    */

}

void BXPT_Dma_Context_P_DescCheckWdma(BXPT_Dma_ContextHandle ctx, unsigned idx, bool link_only, uint32_t *desc)
{
    unsigned numBlocks = ctx->numBlocks;

    if (desc[0] & 0xFFFFFF00) { /* [31:8] RESERVED */
        BDBG_ERR(("WDMA DW0: non-zero value in reserved field"));
    }
    if (idx==numBlocks-1 && !link_only) {
        if ((desc[3] & WDMA_DW3_INTR_EN)==0 || (desc[3] & WDMA_DW3_LAST)==0) {
            BDBG_ERR(("WDMA DW3: INTR_EN and LAST flags not set in last descriptor"));
        }
    }
    else {
        if ((desc[3] & WDMA_DW3_INTR_EN)==WDMA_DW3_INTR_EN || (desc[3] & WDMA_DW3_LAST)==WDMA_DW3_LAST) {
            BDBG_ERR(("WDMA DW3: INTR_EN and LAST flags set in non-last descriptor"));
        }
    }
}

#if (!B_REFSW_MINIMAL)
void BXPT_Dma_Context_P_BlockSettingsDump(BXPT_Dma_ContextHandle ctx, const BXPT_Dma_ContextBlockSettings *pSettings)
{
    BSTD_UNUSED(ctx);
    BKNI_Printf("" BDBG_UINT64_FMT " -> " BDBG_UINT64_FMT " (size %#x:%u). flags: %u%u%u%u\n", BDBG_UINT64_ARG(pSettings->src), BDBG_UINT64_ARG(pSettings->dst), pSettings->size, pSettings->size,
        pSettings->resetCrypto, pSettings->sgScramStart, pSettings->sgScramEnd, pSettings->securityBtp);
}

void BXPT_Dma_Context_P_DescDump(BXPT_Dma_ContextHandle ctx, bool link_only, bool print_warnings)
{
    BXPT_Dma_Handle dma = ctx->parent;
    unsigned i, p;
    uint32_t *desc;
    BMMA_DeviceOffset descOffset;
    unsigned numBlocksMcpb = ctx->numBlocks, numBlocksWdma = ctx->numBlocks;

#if (BXPT_DMA_WDMA_DESC_CONVERT || BXPT_DMA_WDMA_DESC_APPEND_NULL)
    numBlocksWdma = ctx->numBlocksWdma;
#endif
    BMMA_FlushCache(ctx->mcpbBlock[0], ctx->firstMcpbDescCached[0], numBlocksMcpb*BXPT_DMA_MCPB_DESC_SIZE);
    BMMA_FlushCache(ctx->mcpbBlock[1], ctx->firstMcpbDescCached[1], numBlocksMcpb*BXPT_DMA_MCPB_DESC_SIZE);
    BMMA_FlushCache(ctx->wdmaBlock[0], ctx->firstWdmaDescCached[0], numBlocksWdma*BXPT_DMA_WDMA_DESC_SIZE);
    BMMA_FlushCache(ctx->wdmaBlock[1], ctx->firstWdmaDescCached[1], numBlocksWdma*BXPT_DMA_WDMA_DESC_SIZE);

    #define PRINT_SIMPLE 1

    BKNI_Printf("dma %2u: %s started %u, completed %u, run %u\n", dma->channelNum, "              ", dma->cntStarted, dma->cntCompleted, dma->cntRun);
    BKNI_Printf("ctx " BDBG_UINT64_FMT ": %s: started %u, completed %u, run %u. pp %u, lpp %u\n", BDBG_UINT64_ARG(CTX_ID(ctx)), link_only ? "LINK ":"START",
        ctx->cntStarted, ctx->cntCompleted, ctx->cntRun, ctx->pp, ctx->lpp);
    BKNI_Printf("  MCPB offset " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT ", cached %#x:%#x\n", ctx->firstMcpbDescOffset[0], ctx->firstMcpbDescOffset[1], ctx->firstMcpbDescCached[0], ctx->firstMcpbDescCached[1]);
    BKNI_Printf("  WDMA offset " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT ", cached %#x:%#x\n", ctx->firstWdmaDescOffset[0], ctx->firstWdmaDescOffset[1], ctx->firstWdmaDescCached[0], ctx->firstWdmaDescCached[1]);

    for (p=0; p<2; p++) {
        /* don't print the other ping/pong that we're not interested in */
        if ( (!link_only && p!=ctx->pp) ||
             (link_only && p!=ctx->lpp))
        {
            continue;
        }

        desc = ctx->firstMcpbDescCached[p];
        descOffset = ctx->firstMcpbDescOffset[p];

        for (i=0; i<numBlocksMcpb; i++, desc+=BXPT_DMA_MCPB_DESC_WORDS, descOffset+=BXPT_DMA_MCPB_DESC_SIZE) {
            if (link_only && i<numBlocksMcpb-1) { continue; }
#if PRINT_SIMPLE
            BKNI_Printf("MCPB desc%2u:" BDBG_UINT64_FMT ": %08x %08x %08x %08x %08x %08x %08x %08x\n",
                i, BDBG_UINT64_ARG(descOffset), desc[0], desc[1], desc[2], desc[3], desc[4], desc[5], desc[6], desc[7]);
#else
            BKNI_Printf("MCPB %s[%2u:" BDBG_UINT64_FMT ":%s] %08x %08x %08x %08x %08x %08x %08x %08x\n",
                !link_only ? "desc" : "link",
                i, BDBG_UINT64_ARG(descOffset), p==0?"ping":"pong",
                desc[0], desc[1], desc[2], desc[3], desc[4], desc[5], desc[6], desc[7]);
#endif
            if (print_warnings) {
                BXPT_Dma_Context_P_DescCheckMcpb(ctx, i, link_only, desc);
            }
        }

        desc = ctx->firstWdmaDescCached[p];
        descOffset = ctx->firstWdmaDescOffset[p];

        for (i=0; i<numBlocksWdma; i++, desc+=BXPT_DMA_WDMA_DESC_WORDS, descOffset+=BXPT_DMA_WDMA_DESC_SIZE) {
            if (link_only && i<numBlocksWdma-1) { continue; }
#if PRINT_SIMPLE
            BKNI_Printf("WDMA desc%2u:" BDBG_UINT64_FMT ": %08x %08x %08x %08x\n",
                i, BDBG_UINT64_ARG(descOffset), desc[0], desc[1], desc[2], desc[3]);
#else
            BKNI_Printf("WDMA %s[%2u:" BDBG_UINT64_FMT ":%s] %08x %08x %08x %08x\n",
                !link_only ? "desc" : "link",
                i, BDBG_UINT64_ARG(descOffset), p==0?"ping":"pong",
                desc[0], desc[1], desc[2], desc[3]);
#endif
            if (print_warnings) {
                BXPT_Dma_Context_P_DescCheckWdma(ctx, i, link_only, desc);
            }
        }
    }
}

void BXPT_Dma_P_StatusDump(BXPT_Dma_Handle dma)
{
    /* dump before Enqueue should be all 0s */
    BXPT_Dma_ContextHandle ctx;
    uint32_t reg[8];
    BKNI_Printf("Status: channel %u\n", dma->channelNum);
    reg[0] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_CH0_COMPLETED_DESC_ADDRESS + dma->wdmaRamOffset); /* should match LAST_WDMA_DESC_OFFSET(ctx) after done */
    reg[1] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_CH0_FIRST_DESC_ADDR + dma->wdmaRamOffset); /* should match FIRST_WDMA_DESC_OFFSET(ctx) after Enqueue() */
    reg[2] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_RUN_BITS_0_31);
    reg[3] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_SLEEP_STATUS_0_31);
    reg[4] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_WAKE_BITS_0_31);
    BKNI_Printf("  WDMA: CDA %#lx, FDA %#lx, RUN %#lx, SLEEP %#lx, WAKE %#lx\n", reg[0], reg[1], reg[2], reg[3], reg[4]);

    /* after done, both READ_PTR and WRITE_PTR should be the (number of descriptors) % 4 */
    reg[0] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_CH0_DMQ_CONTROL_STRUCT + dma->wdmaRamOffset);
    reg[1] = BCHP_GET_FIELD_DATA(reg[0], XPT_WDMA_CH0_DMQ_CONTROL_STRUCT, READ_PTR);
    reg[2] = BCHP_GET_FIELD_DATA(reg[0], XPT_WDMA_CH0_DMQ_CONTROL_STRUCT, WRITE_PTR);
    BKNI_Printf("  WDMA: DMQ: READ %u, WRITE %u\n", reg[1], reg[2]);

    reg[0] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DMA_DESC_CONTROL + dma->mcpbRegOffset); /* should match FIRST_MCPB_DESC_OFFSET(ctx) after Enqueue() */
    reg[1] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_RUN_STATUS_0_31);
    reg[2] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_WAKE_STATUS_0_31);
    BKNI_Printf("  MCPB: FDA %#lx, RUN %#lx, WAKE %#lx\n", reg[0], reg[1], reg[2]);

    reg[0] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_STATUS + dma->mcpbRegOffset);
    reg[1] = BCHP_GET_FIELD_DATA(reg[0], XPT_MEMDMA_MCPB_CH0_DCPM_STATUS, DESC_ADDRESS_STATUS);
    reg[2] = BCHP_GET_FIELD_DATA(reg[0], XPT_MEMDMA_MCPB_CH0_DCPM_STATUS, DESC_DONE_INT_ADDRESS_STATUS);
    reg[3] = BCHP_GET_FIELD_DATA(reg[0], XPT_MEMDMA_MCPB_CH0_DCPM_STATUS, DATA_ADDR_CUR_DESC_ADDR_STATUS);
    reg[4] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_DESC_ADDR + dma->mcpbRegOffset); /* should match LAST_MCPB_DESC_OFFSET(ctx) after done */
    reg[5] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_DESC_DONE_INT_ADDR + dma->mcpbRegOffset);
    reg[6] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_CH0_DCPM_DATA_ADDR_LOWER + dma->mcpbRegOffset);
    BKNI_Printf("  MCPB: DCPM_DESC_ADDR          %#lx (valid %u)\n", reg[4], reg[1]);
    BKNI_Printf("  MCPB: DCPM_DESC_DONE_INT_ADDR %#lx (valid %u)\n", reg[5], reg[2]);
    BKNI_Printf("  MCPB: DCPM_DATA_ADDR          %#lx (valid %u)\n", reg[6], reg[3]);

    reg[0] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_DESC_RD_IN_PROGRESS_0_31); /* should be 0 after done */
    reg[1] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_DATA_RD_IN_PROGRESS_0_31); /* should be 0 after done */
    reg[2] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_LAST_DESC_REACHED_0_31); /* should be set with bit for this channel after done */
    reg[3] = BREG_Read32(dma->reg, BCHP_XPT_MEMDMA_MCPB_BUFF_DATA_RDY_0_31); /* should be 0 after done */
    BKNI_Printf("  MCPB: DESC_RD %#lx, DATA_RD %#lx, LAST_DESC %#lx, BUFF_DATA %#lx\n", reg[0], reg[1], reg[2], reg[3]);

    /* print out activeCtx's and their WDMA desc ranges */
    for (ctx=BLST_SQ_FIRST(&dma->activeCtxList); ctx; ctx=BLST_SQ_NEXT(ctx, activeNode)) {
        BKNI_Printf("  ctx " BDBG_UINT64_FMT " MCPB_DESC " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT "\n", BDBG_UINT64_ARG(CTX_ID(ctx)), BDBG_UINT64_ARG(FIRST_MCPB_DESC_OFFSET(ctx)), BDBG_UINT64_ARG(LAST_MCPB_DESC_OFFSET(ctx)));
        BKNI_Printf("  ctx " BDBG_UINT64_FMT " WDMA_DESC " BDBG_UINT64_FMT ":" BDBG_UINT64_FMT "\n", BDBG_UINT64_ARG(CTX_ID(ctx)), BDBG_UINT64_ARG(FIRST_WDMA_DESC_OFFSET(ctx)), BDBG_UINT64_ARG(LAST_WDMA_DESC_OFFSET(ctx)));
    }
}

void BXPT_Dma_P_PerfMeterDump(BXPT_Dma_Handle dma)
{
    uint32_t val[8];
#ifdef BCHP_XPT_WDMA_REGS_PM_INTERVAL
    val[0] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_INTERVAL);
    BKNI_Printf("WDMA PerfMeter: channel %u, interval %u\n", dma->channelNum, val[0]);

    val[0] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_INPUT_DATA_MOVED);
    val[1] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_INPUT_DATA_NOT_READY);
    val[2] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_INPUT_DATA_NOT_ACCEPTED);
    BKNI_Printf("  INPUT_DATA:  MOVED %u, NOT_READY %u, NOT_ACCEPTED %u\n", val[0], val[1], val[2]);

    val[0] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_OUTPUT_DATA_MOVED);
    val[1] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_OUTPUT_DATA_NOT_READY);
    val[2] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_OUTPUT_DATA_NOT_ACCEPTED);
    BKNI_Printf("  OUTPUT_DATA: MOVED %u, NOT_READY %u, NOT_ACCEPTED %u\n", val[0], val[1], val[2]);

    val[0] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_NUM_DESCRIPTORS);
    val[1] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_DESCR_READS);
    val[2] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_DATA_STALL_CLOCKS);
    BKNI_Printf("  NUM_DESCS %u, DESCR_READS %u, DATA_STALL_CLOCKS %u\n", val[0], val[1], val[2]);
#else
    BREG_Write32(dma->reg, BCHP_XPT_WDMA_PM_CONTROL_STATS_GUARD, 0x1);

    val[0] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_PM_CONTROL_INTERVAL);
    BKNI_Printf("WDMA PerfMeter: channel %u, interval %u\n", dma->channelNum, val[0]);

    val[0] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_PM_RESULTS_INPUT_DATA_MOVED);
    val[1] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_PM_RESULTS_INPUT_DATA_NOT_READY);
    val[2] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_PM_RESULTS_INPUT_DATA_NOT_ACCEPTED);
    BKNI_Printf("  INPUT_DATA:  MOVED %u, NOT_READY %u, NOT_ACCEPTED %u\n", val[0], val[1], val[2]);

    val[0] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_DATA_MOVED);
    val[1] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_DATA_NOT_READY);
    val[2] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_PM_RESULTS_OUTPUT_DATA_NOT_ACCEPTED);
    BKNI_Printf("  OUTPUT_DATA: MOVED %u, NOT_READY %u, NOT_ACCEPTED %u\n", val[0], val[1], val[2]);

    val[0] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_PM_RESULTS_NUM_DESCRIPTORS);
    val[1] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_PM_RESULTS_DESCR_READS);
    val[2] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_PM_RESULTS_DATA_STALL_CLOCKS);
    BKNI_Printf("  NUM_DESCS %u, DESCR_READS %u, DATA_STALL_CLOCKS %u\n", val[0], val[1], val[2]);

    BREG_Write32(dma->reg, BCHP_XPT_WDMA_PM_CONTROL_STATS_GUARD, 0x0);
#endif

#if 0
    val[0] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_CHAN_SELECT);
    val[0] = BCHP_GET_FIELD_DATA(val[0], XPT_WDMA_REGS_PM_CHAN_SELECT, CHAN_NUM);
    if (val[0] != dma->channelNum) {
        BDBG_WRN(("CHAN_SELECT %u -> %u", val[0], dma->channelNum));
        BCHP_SET_FIELD_DATA(val[0], XPT_WDMA_REGS_PM_CHAN_SELECT, CHAN_NUM, dma->channelNum);
        BREG_Write32(dma->reg, BCHP_XPT_WDMA_REGS_PM_CHAN_SELECT, val[0]);
    }
    val[0] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_CHAN_BYTES_TXFERED);
    val[1] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_CHAN_DATA_STALL_CLOCKS);
    val[2] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_CHAN_PACKETS_ACCEPTED);
    val[3] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_CHAN_PACKETS_REJECTED_OVERFLOW);
    val[4] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_CHAN_PACKETS_REJECTED_RUN_VERSION);
    val[5] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_CHAN_READING_DESCRIPTORS);
    val[6] = BREG_Read32(dma->reg, BCHP_XPT_WDMA_REGS_PM_CHAN_SLEEP);
    BKNI_Printf("CHAN: BYTES_TXFERED %u, DATA_STALL_CLOCKS %u\n", val[0], val[1]);
    BKNI_Printf("CHAN_PACKETS: ACCEPTED %u, REJECTED_OVERFLOW %u, REJECTED_RUN_VERSION %u\n", val[2], val[3], val[4]);
    BKNI_Printf("CHAN: READING_DESCRIPTORS %u, SLEEP %u\n", val[5], val[6]);
#endif
}
#endif

#define MAKE_REG(name) BCHP_##name
#define MAKE_STR(name) #name
#define MAKE_WDMA_CH_REG(name) BCHP_XPT_WDMA_RAMS_##name
#define PRINT_REG_MCPB_TOP(name)  BKNI_Printf("%-s = %08x\n", MAKE_STR(name), BREG_Read32(reg, MAKE_REG(name) + 0))
#define PRINT_REG_MCPB_CH(name)   BKNI_Printf("%-s = %08x\n", MAKE_STR(name), BREG_Read32(reg, MAKE_REG(name) + dma->mcpbRegOffset))
#define PRINT_REG_WDMA_REGS(name) BKNI_Printf("%-s = %08x\n", MAKE_STR(name), BREG_Read32(reg, MAKE_REG(name) + 0))
#define PRINT_REG_WDMA_CH(name)   BKNI_Printf("XPT_WDMA_CHx_%-s = %08x\n", MAKE_STR(name), BREG_Read32(reg, MAKE_WDMA_CH_REG(name) + dma->wdmaRamOffset))
#define PRINT_REG_OTHERS(name)    BKNI_Printf("%-s = %08x\n", MAKE_STR(name), BREG_Read32(reg, MAKE_REG(name) + 0))

#if 0
void BXPT_Dma_P_RegDumpMcpbTop(BXPT_Dma_Handle dma)
{
    BREG_Handle reg = dma->reg;
    BKNI_Printf("[XPT_MEMDMA_MCPB]\n");
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_RUN_SET_CLEAR);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_WAKE_SET);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_WAKE_MODE_SET_CLEAR);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_PAUSE_AT_DESC_READ_CLEAR);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_PAUSE_AT_DESC_END_CLEAR);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_PAUSE_AFTER_GROUP_PACKETS_CLEAR);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_CRC_COMPARE_ERROR_PAUSE_CLEAR);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_MICRO_PAUSE_SET_CLEAR);
#if BCHP_CHIP==7445
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_INTERNAL_SLOTS_CLEAR);
#endif
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_RUN_STATUS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_WAKE_STATUS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_WAKE_MODE_STATUS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_PAUSE_AT_DESC_READ_STATUS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_PAUSE_AT_DESC_END_STATUS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_PAUSE_AFTER_GROUP_PACKETS_STATUS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_CRC_COMPARE_ERROR_PAUSE_STATUS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_MICRO_PAUSE_STATUS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_FINAL_PAUSE_STATUS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_HW_PAUSE_STATUS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DESC_RD_IN_PROGRESS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DATA_RD_IN_PROGRESS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BBUFF_RD_IN_PROGRESS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_LAST_DESC_REACHED_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BUFF_DATA_RDY_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_FIRST_DATA_RD_OPN_PRIORITY_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_RUN_INVALID_SET_CLEAR);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_RUN_INVALID_STATUS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_CHANNEL_PRIORITY_SET);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_CHANNEL_PRIORITY_STATUS_0_15);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_CHANNEL_PRIORITY_STATUS_16_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_0);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_1);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_2);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_3);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_4);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_5);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_6);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_7);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_8);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_9);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_10);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_11);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_12);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_13);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_14);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_15);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_16);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_17);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_18);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_19);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_20);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_21);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_22);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_23);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_24);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_25);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_26);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_27);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_28);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_29);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_30);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BAND_PAUSE_MAPPING_VECTOR_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_SP_GLOBAL_CFG_REG0);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_SP_GLOBAL_CFG_REG1);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_TMEU_CHANNEL_BYPASS_SET_CLEAR);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_TMEU_CHANNEL_BYPASS_STATUS_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC0_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC0_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC0_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC1_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC1_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC1_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC2_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC2_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC2_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC3_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC3_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC3_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC4_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC4_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC4_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC5_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC5_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC5_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC6_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC6_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC6_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC7_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC7_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC7_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC8_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC8_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC8_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC9_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC9_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC9_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC10_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC10_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC10_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC11_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC11_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC11_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC12_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC12_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC12_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC13_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC13_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC13_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC14_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC14_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC14_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC15_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC15_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC15_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC16_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC16_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC16_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC17_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC17_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC17_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC18_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC18_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC18_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC19_CTRL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC19_CURR_COUNTER_VAL);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GPC19_PAUSE_0_31);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_TMEU_ARB_DELAY);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DMA_MEM_ADDR_START);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_BBUFF_MEM_ADDR_START);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_TMEU_MEM_ADDR_START);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DCPM_MEM_ADDR_START);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_PER_CH_MEM_ADDR_END);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_GLOBAL_CTRL_SIGNALS);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_0);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_1);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_2);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_3);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_4);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_5);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_6);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_7);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_8);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_9);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_10);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_11);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_12);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_13);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_14);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_15);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_16);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_17);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_18);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_19);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_20);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_21);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_22);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_23);
    PRINT_REG_MCPB_TOP(XPT_MEMDMA_MCPB_DEBUG_24);
    BKNI_Printf("\n");
}

void BXPT_Dma_P_RegDumpMcpbCh(BXPT_Dma_Handle dma)
{
    BREG_Handle reg = dma->reg;
    BKNI_Printf("[XPT_MEMDMA_MCPB_CH%u]\n", dma->channelNum);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_CONTROL);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DATA_CONTROL);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_CURR_DESC_ADDRESS);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_NEXT_DESC_ADDRESS);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BUFF_BASE_ADDRESS_UPPER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BUFF_BASE_ADDRESS_LOWER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BUFF_END_ADDRESS_UPPER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BUFF_END_ADDRESS_LOWER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BUFF_CURR_RD_ADDRESS_UPPER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BUFF_CURR_RD_ADDRESS_LOWER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BUFF_WRITE_ADDRESS_UPPER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BUFF_WRITE_ADDRESS_LOWER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_STATUS_0);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_STATUS_1);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT0_STATUS_0);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT0_STATUS_1);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT0_CURR_DESC_ADDR);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT0_CURR_DATA_ADDR_UPPER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT0_CURR_DATA_ADDR_LOWER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT0_NEXT_TIMESTAMP);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT0_PKT2PKT_TIMESTAMP_DELTA);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT1_STATUS_0);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT1_STATUS_1);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT1_CURR_DESC_ADDR);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT1_CURR_DATA_ADDR_UPPER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT1_CURR_DATA_ADDR_LOWER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT1_NEXT_TIMESTAMP);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_DESC_SLOT1_PKT2PKT_TIMESTAMP_DELTA);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_PKT_LEN);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_PARSER_CTRL);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_TS_CONFIG);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_PES_ES_CONFIG);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_PES_SYNC_COUNTER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_ASF_CONFIG);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_STATE_REG_0);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_STATE_REG_1);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_STATE_REG_2);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_STATE_REG_3);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_STATE_REG_4);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_STATE_REG_5);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_STATE_REG_6);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_STATE_REG_7);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_STATE_REG_8);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_STATE_REG_9);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_SP_STATE_REG_10);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BBUFF_CTRL);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BBUFF_CRC);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BBUFF0_RW_STATUS);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BBUFF0_RO_STATUS);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BBUFF1_RW_STATUS);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DMA_BBUFF1_RO_STATUS);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_BLOCKOUT_CTRL);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_NEXT_BO_MON);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_TIMING_CTRL);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_REF_DIFF_VALUE_TS_MBOX);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_TS_ERR_BOUND_EARLY);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_TS_ERR_BOUND_LATE);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_NEXT_GPC_MON);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_REF_DIFF_VALUE_SIGN);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_PES_PACING_CTRL);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_SLOT_STATUS);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_TIMING_INFO_SLOT0_REG1);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_TIMING_INFO_SLOT0_REG2);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_TIMING_INFO_SLOT1_REG1);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_TIMING_INFO_SLOT1_REG2);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_TIMING_INFO_LAST_TIMESTAMP_DELTA);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_TMEU_TIMING_INFO_LAST_NEXT_TIMESTAMP);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_STATUS);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_DESC_ADDR);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_DESC_DONE_INT_ADDR);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_PAUSE_AFTER_GROUP_PACKETS_CTRL);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_PAUSE_AFTER_GROUP_PACKETS_PKT_COUNTER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_DATA_ADDR_UPPER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_DATA_ADDR_LOWER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_CURR_DESC_ADDR);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_SLOT_STATUS);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_DESC_ADDR_SLOT_0);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_DATA_ADDR_SLOT_0_UPPER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_DATA_ADDR_SLOT_0_LOWER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_DESC_ADDR_SLOT_1);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_DATA_ADDR_SLOT_1_UPPER);
    PRINT_REG_MCPB_CH(XPT_MEMDMA_MCPB_CH0_DCPM_DATA_ADDR_SLOT_1_LOWER);
    BKNI_Printf("\n");
}

void BXPT_Dma_P_RegDumpWdmaRegs(BXPT_Dma_Handle dma)
{
    BREG_Handle reg = dma->reg;
    BKNI_Printf("[XPT_WDMA_REGS]\n");
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_RUN_SET_CLEAR);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_WAKE_SET);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_RBUF_MODE_CONFIG);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_TIMESTAMP_MODE_CONFIG);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_MATCH_RUN_VERSION_CONFIG);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_LDL_MODE_CONFIG);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_DMQ_RESET_CMD);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_DMQ_PUSH_CMD);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_RUN_BITS_0_31);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_WAKE_BITS_0_31);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_RBUF_MODE_BITS_0_31);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_TIMESTAMP_MODE_BITS_0_31);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_LDL_MODE_BITS_0_31);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_MATCH_RUN_VERSION_BITS_0_31);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_SLEEP_STATUS_0_31);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_CONTEXT_VALID_0_31);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_DATA_STALL_TIMEOUT);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_SPECULATIVE_READ_ENABLE);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_OUTSTANDING_READ_FLAG_0_31);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_OUTSTANDING_WRITE_FLAG_0_31);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_OUTSTANDING_READ_FLAG_CLEAR);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_OUTSTANDING_WRITE_FLAG_CLEAR);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_DRR_STATE);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_DRC_STATE);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_DAP_STATE);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_RPT_STATE);
    PRINT_REG_WDMA_REGS(XPT_WDMA_REGS_READY_ACCEPT_PROBE);
    BKNI_Printf("\n");
}

void BXPT_Dma_P_RegDumpWdmaRams(BXPT_Dma_Handle dma)
{
    BREG_Handle reg = dma->reg;
    BKNI_Printf("[XPT_WDMA_CH%u]\n", dma->channelNum);
    PRINT_REG_WDMA_CH(FIRST_DESC_ADDR);
    PRINT_REG_WDMA_CH(NEXT_DESC_ADDR);
    PRINT_REG_WDMA_CH(COMPLETED_DESC_ADDRESS);
    PRINT_REG_WDMA_CH(BTP_PACKET_GROUP_ID);
    PRINT_REG_WDMA_CH(RUN_VERSION_CONFIG);
    PRINT_REG_WDMA_CH(OVERFLOW_REASONS);
    PRINT_REG_WDMA_CH(DMQ_CONTROL_STRUCT);
    PRINT_REG_WDMA_CH(DATA_CONTROL);
    PRINT_REG_WDMA_CH(DRAM_BUFF_BASE_PTR_HI);
    PRINT_REG_WDMA_CH(DRAM_BUFF_BASE_PTR);
    PRINT_REG_WDMA_CH(DRAM_BUFF_END_PTR_HI);
    PRINT_REG_WDMA_CH(DRAM_BUFF_END_PTR);
    PRINT_REG_WDMA_CH(DRAM_BUFF_RD_PTR_HI);
    PRINT_REG_WDMA_CH(DRAM_BUFF_RD_PTR);
    PRINT_REG_WDMA_CH(DRAM_BUFF_WR_PTR_HI);
    PRINT_REG_WDMA_CH(DRAM_BUFF_WR_PTR);
    PRINT_REG_WDMA_CH(DRAM_BUFF_VALID_PTR_HI);
    PRINT_REG_WDMA_CH(DRAM_BUFF_VALID_PTR);
    PRINT_REG_WDMA_CH(DRAM_BUFF_LOWER_THRESHOLD_HI);
    PRINT_REG_WDMA_CH(DRAM_BUFF_LOWER_THRESHOLD);
    PRINT_REG_WDMA_CH(DRAM_BUFF_UPPER_THRESHOLD_HI);
    PRINT_REG_WDMA_CH(DRAM_BUFF_UPPER_THRESHOLD);
    PRINT_REG_WDMA_CH(DRAM_BUFF_STATUS);
    PRINT_REG_WDMA_CH(DRAM_BUFF_CONTROL);
    PRINT_REG_WDMA_CH(DMQ_0_0);
    PRINT_REG_WDMA_CH(DMQ_0_1);
    PRINT_REG_WDMA_CH(DMQ_0_2);
    PRINT_REG_WDMA_CH(DMQ_0_3);
    PRINT_REG_WDMA_CH(DMQ_1_0);
    PRINT_REG_WDMA_CH(DMQ_1_1);
    PRINT_REG_WDMA_CH(DMQ_1_2);
    PRINT_REG_WDMA_CH(DMQ_1_3);
    PRINT_REG_WDMA_CH(DMQ_2_0);
    PRINT_REG_WDMA_CH(DMQ_2_1);
    PRINT_REG_WDMA_CH(DMQ_2_2);
    PRINT_REG_WDMA_CH(DMQ_2_3);
    PRINT_REG_WDMA_CH(DMQ_3_0);
    PRINT_REG_WDMA_CH(DMQ_3_1);
    PRINT_REG_WDMA_CH(DMQ_3_2);
    PRINT_REG_WDMA_CH(DMQ_3_3);
    BKNI_Printf("\n");
}

void BXPT_Dma_P_RegDumpOthers(BXPT_Dma_Handle dma)
{
    BREG_Handle reg = dma->reg;
    BKNI_Printf("[OTHERS]\n");
#if BXPT_DMA_MCPB_FALSE_WAKE_IRQ
    PRINT_REG_OTHERS(XPT_MEMDMA_MCPB_MISC_FALSE_WAKE_INTR_L2_CPU_STATUS);
    BKNI_Printf("MCPB_FALSE_WAKE accum %u\n", dma->cntMcpbFalseWake);
#endif
    BKNI_Printf("\n");
}

void BXPT_Dma_P_RegDumpAll(BXPT_Dma_Handle dma)
{
    BXPT_Dma_ContextHandle context;

    /* dump OVERFLOW_REASONS register for all channels first */
#if 0
{
    unsigned i, offset;
    uint32_t reg;
    BKNI_Printf("[XPT_WDMA_CHx_OVERFLOW_REASONS]\n");
    for (i=0; i<BXPT_DMA_NUM_CHANNELS; i++) {
        offset = (BCHP_XPT_WDMA_CH0_CHAN_OFFSET - BCHP_XPT_WDMA_CH0_FIRST_DESC_ADDR) * i;
        reg = BREG_Read32(dma->reg, BCHP_XPT_WDMA_CH0_OVERFLOW_REASONS + offset);
        BKNI_Printf("XPT_WDMA_CHx_OVERFLOW_REASONS[%2u] = %#x\n", i, reg);    }
    BKNI_Printf("\n");
}
#endif

    BXPT_Dma_P_RegDumpMcpbTop(dma);
    BXPT_Dma_P_RegDumpMcpbCh(dma);
    BXPT_Dma_P_RegDumpWdmaRegs(dma);
    BXPT_Dma_P_RegDumpWdmaRams(dma);
    BXPT_Dma_P_RegDumpOthers(dma);

    BKNI_Printf("CH%u: last_queued_ctx=" BDBG_UINT64_FMT "\n", dma->channelNum, CTX_ID(dma->lastQueuedCtx));

    for (context=BLST_S_FIRST(&dma->ctxList); context; context=BLST_S_NEXT(context, ctxNode)) {
        BXPT_Dma_Context_P_DescDump(context, false, false);
    }
}

void BXPT_Dma_P_RegDumpPidTable(BXPT_Dma_Handle dma)
{
    BREG_Handle reg = dma->reg;
    BXPT_Dma_ContextHandle ctx;
    unsigned pidChannelNum;
    uint32_t PID_TABLE_ADDR, SPID_TABLE_ADDR, SPID_EXT_TABLE_ADDR, PID_TABLE_VAL, SPID_TABLE_VAL, SPID_EXT_TABLE_VAL;

    for (ctx=BLST_S_FIRST(&dma->ctxList); ctx; ctx=BLST_S_NEXT(ctx, ctxNode)) {
        BKNI_Printf("[context " BDBG_UINT64_FMT "]\n", BDBG_UINT64_ARG(CTX_ID(ctx)));
        pidChannelNum = ctx->settings.pidChannelNum;
        PID_TABLE_ADDR      = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + 4 * pidChannelNum;
        SPID_TABLE_ADDR     = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + 4 * pidChannelNum;
        SPID_EXT_TABLE_ADDR = BCHP_XPT_FE_SPID_EXT_TABLE_i_ARRAY_BASE + 4 * pidChannelNum;
        PID_TABLE_VAL      = BREG_Read32(reg, PID_TABLE_ADDR);
        SPID_TABLE_VAL     = BREG_Read32(reg, SPID_TABLE_ADDR);
        SPID_EXT_TABLE_VAL = BREG_Read32(reg, SPID_EXT_TABLE_ADDR);

        BKNI_Printf("XPT_FE_PID_TABLE_%u      (addr %08x) = %08x\n", pidChannelNum, PID_TABLE_ADDR, PID_TABLE_VAL);
        BKNI_Printf("XPT_FE_SPID_TABLE_%u     (addr %08x) = %08x\n", pidChannelNum, SPID_TABLE_ADDR, SPID_TABLE_VAL);
        BKNI_Printf("XPT_FE_SPID_EXT_TABLE_%u (addr %08x) = %08x\n", pidChannelNum, SPID_EXT_TABLE_ADDR, SPID_EXT_TABLE_VAL);
    }
    BKNI_Printf("\n");
}
#endif
