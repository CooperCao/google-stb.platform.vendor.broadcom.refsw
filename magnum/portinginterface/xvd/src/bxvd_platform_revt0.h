/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef _BXVD_PLATFORM_REVT0_H_
#define _BXVD_PLATFORM_REVT0_H_

/* Rev N core with dual IL pipe uses 1.79 MB */
#if (BCHP_CHIP == 7278)
#define BXVD_P_FW_IMAGE_SIZE              0x001CA000
#else
#define BXVD_P_FW_IMAGE_SIZE              0x00180000
#endif

#define BXVD_P_FW_INNER_IMAGE_OFFSET      0x00090000
#define BXVD_P_FW_INNER_2_IMAGE_OFFSET    0x0012d000


/* CPU Clock speed for FW UART emulation */
/* UART clock frequency in mhz */

#define  BXVD_P_AVD_IL_CLK_FREQ 450

#define  BXVD_P_CORE_40BIT_ADDRESSABLE 1

#define  BXVD_P_DECODER_REVT 1

#define  BXVD_P_USE_HVD_INTRS 1

#define  BXVD_P_RUL_DONE_MASK_64_BITS 0

#define  BXVD_P_PHY_ADDR BMMA_DeviceOffset

#if (BCHP_CHIP == 7278)
#define  BXVD_P_CORE_REVISION 'T'
#define  BXVD_P_MULTIPLE_HVD_PRESENT 1
#define  BXVD_P_TWO_HVD_DECODERS_PRESENT 1
#define  BXVD_P_PICTURE_DATA_RDY_2_SUPPORTTED  1
#define  BXVD_MAX_INSTANCE_COUNT      2
#define  BXVD_P_RAVE_40BIT_ADDRESSABLE 1
#define  BXVD_P_STB_REG_BASE          0
#define  BXVD_P_AVD_OL_CLK_FREQ 600
#define  BXVD_P_HEVD_DUAL_PIPE_PRESENT 1
#else
#define  BXVD_P_CORE_REVISION 'U'
#define  BXVD_P_FW_HW_BASE_NON_ZERO   1
#define  BXVD_MAX_INSTANCE_COUNT      1
#define  BXVD_P_STB_REG_BASE          BCHP_HEVD_OL_CPU_REGS_0_REG_START
#define  BXVD_P_HEVD_DUAL_PIPE_PRESENT 0
#define  BXVD_P_AVD_OL_CLK_FREQ 648
#endif

#define  BXVD_P_8N3_BUG 1
#define  BXVD_P_USE_REV_N_RESET_METHOD 1

#define  BXVD_P_HEVD_PFRI_DEBUG_PFRI_GROUPING_PRESENT 1

#define  BXVD_P_HVD_PRESENT 1

#include "bchp_common.h"
#include "bchp_bvnf_intr2_0.h"

#include "bchp_memc_ddr_0.h"

#if (BCHP_CHIP == 7278)
#include "bchp_memc_ddr_1.h"
#define BXVD_P_MEMC_1_PRESENT 1
#endif

#include "bchp_hvd_intr2_0.h"
#include "bchp_int_id_hvd_intr2_0.h"

#include "bchp_hevd_ol_ctl_0.h"
#include "bchp_hevd_ol_sint_0.h"
#include "bchp_hevd_pcache_0.h"

#include "bchp_hevd_ol_cpu_regs_0.h"
#include "bchp_hevd_il_cpu_regs_0.h"

#if BXVD_P_MULTIPLE_HVD_PRESENT
#include "bchp_hevd_ol_cpu_regs_1.h"
#include "bchp_hevd_il_cpu_regs_1.h"
#include "bchp_hevd_il_cpu_regs_1.h"
#include "bchp_hevd_ol_cpu_debug_1.h"
#include "bchp_hevd_il_cpu_debug_1.h"

#include "bchp_hvd_intr2_1.h"
#include "bchp_int_id_hvd_intr2_1.h"

#include "bchp_hevd_ol_ctl_1.h"

#include "bchp_hevd_ol_sint_1.h"

#include "bchp_hevd_pcache_1.h"
#include "bchp_rvc_1.h"
#include "bchp_dcd_pipe_ctl_1.h"
#include "bchp_hevd_pfri_1.h"
#endif

#include "bchp_hevd_ol_cpu_debug_0.h"
#include "bchp_hevd_il_cpu_debug_0.h"

#include "bchp_hevd_pfri_0.h"

#include "bchp_dcd_pipe_ctl_0.h"

#include "bchp_rvc_0.h"

#if BXVD_P_RUL_DONE_MASK_64_BITS

#include "bchp_bvnf_intr2_11.h"

#if BXVD_P_USE_HVD_INTRS
#include "bchp_int_id_hvd_intr2_2.h"
#else
#include "bchp_int_id_shvd_intr2_2.h"
#endif

#include "bchp_hevd_ol_ctl_2.h"
#include "bchp_hevd_ol_sint_2.h"

#include "bchp_hevd_pcache_2.h"
#include "bchp_rvc_2.h"

#include "bchp_hevd_ol_cpu_regs_2.h"
#include "bchp_hevd_il_cpu_regs_2.h"

#include "bchp_hevd_ol_cpu_debug_2.h"
#include "bchp_hevd_il_cpu_debug_2.h"

#include "bchp_hevd_pfri_2.h"

#include "bchp_dcd_pipe_ctl_2.h"
#endif

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
#include "bchp_hevd_il_cpu_debug_2_0.h"
#include "bchp_hevd_il_cpu_regs_2_0.h"
#include "bchp_hevd_pfri_2_0.h"
#include "bchp_hevd_pcache_2_0.h"

#if BXVD_P_MULTIPLE_HVD_PRESENT
#include "bchp_hevd_il_cpu_debug_2_1.h"
#include "bchp_hevd_il_cpu_regs_2_1.h"
#include "bchp_hevd_pfri_2_1.h"
#include "bchp_hevd_pcache_2_1.h"
#endif
#endif

/* Common to all flavors */
#include "bchp_sun_top_ctrl.h"
#include "bchp_sun_gisb_arb.h"

#ifdef BCHP_XPT_PCROFFSET_REG_START
#include "bchp_xpt_pcroffset.h"
#endif

#ifdef BCHP_XPT_RAVE_REG_START
#include "bchp_xpt_rave.h"
#else
#include "bchp_ott_xpt_cdb_itb.h"
#endif

#include "bafl.h"

#define BXVD_P_CHIP_PRODUCT_REVISION  BCHP_SUN_TOP_CTRL_PRODUCT_ID

#define  BXVD_P_AVD_ARC600 1
#define  BXVD_P_FW_HIM_API 1
#define  BXVD_P_PLATFORM_STRIPE_WIDTH_NUM 3

#if (BXVD_MAX_INSTANCE_COUNT == 1)

#define  BXVD_P_HVD_INTR2_0_CPU_SET   BCHP_HVD_INTR2_0_CPU_SET

#else /* Multi-decoders */

#if BXVD_P_USE_HVD_INTRS

#define  BXVD_P_SW_INIT_0_SET_xvd0_sw_init_MASK  BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_hvd0_sw_init_MASK
#define  BXVD_P_SW_INIT_0_CLEAR_xvd0_sw_init_MASK BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR_hvd0_sw_init_MASK

#define  BXVD_P_BVNF_INTR2_3_HVD0_STATUS       BCHP_BVNF_INTR2_3_HVD0_STATUS
#define  BXVD_P_BVNF_INTR2_3_HVD0_CLEAR        BCHP_BVNF_INTR2_3_HVD0_CLEAR
#define  BXVD_P_BVNF_INTR2_3_HVD0_MASK_CLEAR   BCHP_BVNF_INTR2_3_HVD0_MASK_CLEAR

#define  BXVD_P_BVNF_INTR2_11_HVD0_STATUS      BCHP_BVNF_INTR2_11_HVD0_STATUS
#define  BXVD_P_BVNF_INTR2_11_HVD0_CLEAR       BCHP_BVNF_INTR2_11_HVD0_CLEAR
#define  BXVD_P_BVNF_INTR2_11_HVD0_MASK_CLEAR  BCHP_BVNF_INTR2_11_HVD0_MASK_CLEAR

#define  BXVD_P_SW_INIT_0_SET_hvd1_sw_init_MASK  BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_hvd1_sw_init_MASK
#define  BXVD_P_SW_INIT_0_CLEAR_hvd1_sw_init_MASK BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR_hvd1_sw_init_MASK

#define  BXVD_P_BVNF_INTR2_3_HVD1_STATUS       BCHP_BVNF_INTR2_3_HVD1_STATUS
#define  BXVD_P_BVNF_INTR2_3_HVD1_CLEAR        BCHP_BVNF_INTR2_3_HVD1_CLEAR
#define  BXVD_P_BVNF_INTR2_3_HVD1_MASK_CLEAR   BCHP_BVNF_INTR2_3_HVD1_MASK_CLEAR

#define  BXVD_P_GISB_ARB_REQ_MASK_HVD0_MASK   BCHP_SUN_GISB_ARB_REQ_MASK_hvd_0_MASK
#define  BXVD_P_GISB_ARB_REQ_MASK_HVD1_MASK   BCHP_SUN_GISB_ARB_REQ_MASK_hvd_1_MASK

#define  BXVD_P_HVD_INTR2_0_CPU_SET   BCHP_HVD_INTR2_0_CPU_SET

#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR0          BCHP_INT_ID_HVD_INTR2_0_AVD_SW_INTR0
#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR3          BCHP_INT_ID_HVD_INTR2_0_AVD_SW_INTR3
#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_MBOX_INTR         BCHP_INT_ID_HVD_INTR2_0_AVD_MBOX_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR4          BCHP_INT_ID_HVD_INTR2_0_AVD_SW_INTR4
#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR5          BCHP_INT_ID_HVD_INTR2_0_AVD_SW_INTR5
#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR2          BCHP_INT_ID_HVD_INTR2_0_AVD_SW_INTR2
#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_WATCHDOG_INTR     BCHP_INT_ID_HVD_INTR2_0_AVD_WATCHDOG_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_0_VICH_REG_INTR         BCHP_INT_ID_HVD_INTR2_0_VICH_REG_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_0_VICH_SCB_WR_INTR      BCHP_INT_ID_HVD_INTR2_0_VICH_SCB_WR_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_0_VICH_OL_INST_RD_INTR  BCHP_INT_ID_HVD_INTR2_0_VICH_OL_INST_RD_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_0_VICH_IL_INST_RD_INTR  BCHP_INT_ID_HVD_INTR2_0_VICH_IL_INST_RD_INTR

#ifdef BCHP_HVD_INTR2_1_CPU_SET
#define  BXVD_P_HVD_INTR2_1_CPU_SET   BCHP_HVD_INTR2_1_CPU_SET

#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR0          BCHP_INT_ID_HVD_INTR2_1_AVD_SW_INTR0
#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR3          BCHP_INT_ID_HVD_INTR2_1_AVD_SW_INTR3
#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_MBOX_INTR         BCHP_INT_ID_HVD_INTR2_1_AVD_MBOX_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR4          BCHP_INT_ID_HVD_INTR2_1_AVD_SW_INTR4
#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR5          BCHP_INT_ID_HVD_INTR2_1_AVD_SW_INTR5
#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR2          BCHP_INT_ID_HVD_INTR2_1_AVD_SW_INTR2
#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_WATCHDOG_INTR     BCHP_INT_ID_HVD_INTR2_1_AVD_WATCHDOG_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_1_VICH_REG_INTR         BCHP_INT_ID_HVD_INTR2_1_VICH_REG_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_1_VICH_SCB_WR_INTR      BCHP_INT_ID_HVD_INTR2_1_VICH_SCB_WR_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_1_VICH_OL_INST_RD_INTR  BCHP_INT_ID_HVD_INTR2_1_VICH_OL_INST_RD_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_1_VICH_IL_INST_RD_INTR  BCHP_INT_ID_HVD_INTR2_1_VICH_IL_INST_RD_INTR
#endif

#ifdef BCHP_HVD_INTR2_2_CPU_SET
#define  BXVD_P_HVD_INTR2_2_CPU_SET   BCHP_HVD_INTR2_2_CPU_SET

#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR0          BCHP_INT_ID_HVD_INTR2_2_AVD_SW_INTR0
#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR3          BCHP_INT_ID_HVD_INTR2_2_AVD_SW_INTR3
#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_MBOX_INTR         BCHP_INT_ID_HVD_INTR2_2_AVD_MBOX_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR4          BCHP_INT_ID_HVD_INTR2_2_AVD_SW_INTR4
#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR5          BCHP_INT_ID_HVD_INTR2_2_AVD_SW_INTR5
#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR2          BCHP_INT_ID_HVD_INTR2_2_AVD_SW_INTR2
#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_WATCHDOG_INTR     BCHP_INT_ID_HVD_INTR2_2_AVD_WATCHDOG_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_2_VICH_REG_INTR         BCHP_INT_ID_HVD_INTR2_2_VICH_REG_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_2_VICH_SCB_WR_INTR      BCHP_INT_ID_HVD_INTR2_2_VICH_SCB_WR_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_2_VICH_OL_INST_RD_INTR  BCHP_INT_ID_HVD_INTR2_2_VICH_OL_INST_RD_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_2_VICH_IL_INST_RD_INTR  BCHP_INT_ID_HVD_INTR2_2_VICH_IL_INST_RD_INTR
#endif

#else

#define  BXVD_P_SW_INIT_0_SET_hvd1_sw_init_MASK  BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_avd1_sw_init_MASK
#define  BXVD_P_SW_INIT_0_CLEAR_hvd1_sw_init_MASK BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR_avd1_sw_init_MASK

#define  BXVD_P_BVNF_INTR2_3_HVD1_STATUS       BCHP_BVNF_INTR2_3_AVD1_STATUS
#define  BXVD_P_BVNF_INTR2_3_HVD1_CLEAR        BCHP_BVNF_INTR2_3_AVD1_CLEAR
#define  BXVD_P_BVNF_INTR2_3_HVD1_MASK_CLEAR   BCHP_BVNF_INTR2_3_AVD1_MASK_CLEAR

#define  BXVD_P_GISB_ARB_REQ_MASK_HVD0_MASK   BCHP_SUN_GISB_ARB_REQ_MASK_avd_0_MASK
#define  BXVD_P_GISB_ARB_REQ_MASK_HVD1_MASK   BCHP_SUN_GISB_ARB_REQ_MASK_avd_1_MASK

#define  BXVD_P_HVD_INTR2_0_CPU_SET   BCHP_SHVD_INTR2_0_CPU_SET

#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR0          BCHP_INT_ID_SHVD_INTR2_0_AVD_SW_INTR0
#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR3          BCHP_INT_ID_SHVD_INTR2_0_AVD_SW_INTR3
#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_MBOX_INTR         BCHP_INT_ID_SHVD_INTR2_0_AVD_MBOX_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR4          BCHP_INT_ID_SHVD_INTR2_0_AVD_SW_INTR4
#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR5          BCHP_INT_ID_SHVD_INTR2_0_AVD_SW_INTR5
#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_SW_INTR2          BCHP_INT_ID_SHVD_INTR2_0_AVD_SW_INTR2
#define  BXVD_P_INT_ID_HVD_INTR2_0_AVD_WATCHDOG_INTR     BCHP_INT_ID_SHVD_INTR2_0_AVD_WATCHDOG_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_0_VICH_REG_INTR         BCHP_INT_ID_SHVD_INTR2_0_VICH_REG_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_0_VICH_SCB_WR_INTR      BCHP_INT_ID_SHVD_INTR2_0_VICH_SCB_WR_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_0_VICH_OL_INST_RD_INTR  BCHP_INT_ID_SHVD_INTR2_0_VICH_OL_INST_RD_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_0_VICH_IL_INST_RD_INTR  BCHP_INT_ID_SHVD_INTR2_0_VICH_IL_INST_RD_INTR

#ifdef BCHP_SHVD_INTR2_1_CPU_SET
#define  BXVD_P_HVD_INTR2_1_CPU_SET   BCHP_SHVD_INTR2_1_CPU_SET

#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR0          BCHP_INT_ID_SHVD_INTR2_1_AVD_SW_INTR0
#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR3          BCHP_INT_ID_SHVD_INTR2_1_AVD_SW_INTR3
#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_MBOX_INTR         BCHP_INT_ID_SHVD_INTR2_1_AVD_MBOX_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR4          BCHP_INT_ID_SHVD_INTR2_1_AVD_SW_INTR4
#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR5          BCHP_INT_ID_SHVD_INTR2_1_AVD_SW_INTR5
#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_SW_INTR2          BCHP_INT_ID_SHVD_INTR2_1_AVD_SW_INTR2
#define  BXVD_P_INT_ID_HVD_INTR2_1_AVD_WATCHDOG_INTR     BCHP_INT_ID_SHVD_INTR2_1_AVD_WATCHDOG_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_1_VICH_REG_INTR         BCHP_INT_ID_SHVD_INTR2_1_VICH_REG_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_1_VICH_SCB_WR_INTR      BCHP_INT_ID_SHVD_INTR2_1_VICH_SCB_WR_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_1_VICH_OL_INST_RD_INTR  BCHP_INT_ID_SHVD_INTR2_1_VICH_OL_INST_RD_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_1_VICH_IL_INST_RD_INTR  BCHP_INT_ID_SHVD_INTR2_1_VICH_IL_INST_RD_INTR
#endif

#ifdef BCHP_SHVD_INTR2_2_CPU_SET
#define  BXVD_P_HVD_INTR2_2_CPU_SET   BCHP_SHVD_INTR2_2_CPU_SET

#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR0          BCHP_INT_ID_SHVD_INTR2_2_AVD_SW_INTR0
#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR3          BCHP_INT_ID_SHVD_INTR2_2_AVD_SW_INTR3
#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_MBOX_INTR         BCHP_INT_ID_SHVD_INTR2_2_AVD_MBOX_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR4          BCHP_INT_ID_SHVD_INTR2_2_AVD_SW_INTR4
#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR5          BCHP_INT_ID_SHVD_INTR2_2_AVD_SW_INTR5
#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_SW_INTR2          BCHP_INT_ID_SHVD_INTR2_2_AVD_SW_INTR2
#define  BXVD_P_INT_ID_HVD_INTR2_2_AVD_WATCHDOG_INTR     BCHP_INT_ID_SHVD_INTR2_2_AVD_WATCHDOG_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_2_VICH_REG_INTR         BCHP_INT_ID_SHVD_INTR2_2_VICH_REG_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_2_VICH_SCB_WR_INTR      BCHP_INT_ID_SHVD_INTR2_2_VICH_SCB_WR_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_2_VICH_OL_INST_RD_INTR  BCHP_INT_ID_SHVD_INTR2_2_VICH_OL_INST_RD_INTR
#define  BXVD_P_INT_ID_HVD_INTR2_2_VICH_IL_INST_RD_INTR  BCHP_INT_ID_SHVD_INTR2_2_VICH_IL_INST_RD_INTR
#endif

#endif /* #if BXVD_P_USE_HVD_INTRS  */
#endif /* #if BXVD_P_MAX_INSTANCE_COUNT == 1 */

#include "bxvd_vdec_info.h"
#include "bxvd_priv.h"

#ifdef BCHP_MEMC_SENTINEL_0_0_REG_START
#define BXVD_P_MEMC_SENTINEL_0_REG_START BCHP_MEMC_SENTINEL_0_0_REG_START
#endif

#define BXVD_P_GET_MEMORY_CONFIG(hXvd, eMemCfgMode)   \
{                                                     \
   eMemCfgMode = BXVD_P_MemCfgMode_eUNKNOWN;          \
}

#if 0
/* Bug in REV U core, AVS2 is no longer supported */
/* #if BXVD_P_CORE_REVISION_NUM >= 21    / * Core revision U or later */
#define BXVD_P_PLATFORM_SUPPORTED_PROTOCOLS  BXVD_P_REVU_DECODE_PROTOCOLS_MASK
#else
/* HVD Rev R */
#define BXVD_P_PLATFORM_SUPPORTED_PROTOCOLS  BXVD_P_REVS_DECODE_PROTOCOLS_MASK
#endif

#define BXVD_P_VP9_CAPABLE 1
#define BXVD_P_NO_LEGACY_PROTOCOL_SUPPORT 1

/* Get picture buffer atom size */
#define BXVD_P_USE_GET_BUFFER_ATOM_SIZE_REVT0 1
#define BXVD_P_GET_BUFFER_ATOM_SIZE BXVD_P_GetBufferAtomSize_RevT0
BXVD_P_SETUP_GET_BUFFER_ATOM_SIZE_PROTOTYPE(RevT0);

/* Platform Stripe Info */
#define BXVD_P_USE_DETERMINE_STRIPE_INFO_REVT0 1
#define BXVD_P_DETERMINE_STRIPE_INFO BXVD_P_DetermineStripeInfo_RevT0
BXVD_P_DETERMINE_STRIPE_INFO_PROTOTYPE(RevT0);

/* Firmware loading */
#define BXVD_P_USE_FW_LOAD_CORE_REVN0 1
#define BXVD_P_FW_LOAD BXVD_P_FWLoad_RevN0
BXVD_P_FW_LOAD_PROTOTYPE(RevN0);

/* Chip enable */
#define BXVD_P_USE_CORE_CHIP_ENABLE_REVT0 1
#define BXVD_P_CHIP_ENABLE BXVD_P_ChipEnable_RevT0
BXVD_P_CHIP_ENABLE_PROTOTYPE(RevT0);

/* Chip Reset */
#define BXVD_P_USE_CORE_RESET_CHIP_REVT0 1
#define BXVD_P_RESET_CHIP BXVD_P_ChipReset_RevT0
BXVD_P_CHIP_RESET_PROTOTYPE(RevT0);

/* Init Register Pointers/Masks */

#define BXVD_P_USE_INIT_REG_PTRS_REVT0 1
#define BXVD_P_INIT_REG_PTRS BXVD_P_InitRegPtrs_RevT0
BXVD_P_INIT_REG_PTRS_PROTOTYPE(RevT0);

/* Setup FW Memory */
#define BXVD_P_USE_SETUP_FW_MEMORY_REVE0 1
#define BXVD_P_SETUP_FW_MEMORY BXVD_P_SetupFWMemory_RevE0
BXVD_P_SETUP_FW_MEMORY_PROTOTYPE(RevE0);

/* Teardown FW Memory */
#define BXVD_P_USE_TEAR_DOWN_FW_MEMORY_REVE0 1
#define BXVD_P_TEAR_DOWN_FW_MEMORY BXVD_P_TearDownFWMemory_RevE0
BXVD_P_TEAR_DOWN_FW_MEMORY_PROTOTYPE(RevE0);

/* Verify Watchdog Fired */
#define BXVD_P_USE_VERIFY_WATCHDOG_FIRED_REVK0 1
#define BXVD_P_VERIFY_WATCHDOG_FIRED BXVD_P_VerifyWatchdogFired_RevK0_isr
BXVD_P_VERIFY_WATCHDOG_FIRED_PROTOTYPE(RevK0);

/* Rave Context Register info needed by FW */
#ifdef BCHP_XPT_RAVE_REG_START
#define BXVD_P_RAVE_CONTEXT_SIZE        (BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR)
#define BXVD_P_RAVE_CX_HOLD_CLR_STATUS  BCHP_XPT_RAVE_CX_HOLD_CLR_STATUS
#define BXVD_P_RAVE_PACKET_COUNT        BCHP_XPT_RAVE_PACKET_COUNT
#else
#define BXVD_P_RAVE_CONTEXT_SIZE        (BCHP_OTT_XPT_CDB_ITB_CX1_CDB_WRITE_PTR - BCHP_OTT_XPT_CDB_ITB_CX0_CDB_WRITE_PTR)
#define BXVD_P_RAVE_CX_HOLD_CLR_STATUS   BCHP_OTT_XPT_CDB_ITB_CX_HOLD_CLR_STATUS
#define BXVD_P_RAVE_PACKET_COUNT        0
#endif

/* Use Host Interface Memory routines */
#define BXVD_P_USE_READ_DISPLAY_INFO_HIM_API_REVT0 1

#define BXVD_P_IS_DISPLAY_INFO_EQUAL BXVD_P_IsDisplayInfoEqual_HIM_API_isr

void BXVD_P_ReadDisplayInfo_HIM_API_isr(BXVD_Handle hXvd,
                                        uint32_t uiDisplayInfoOffset,
                                        BXVD_P_DisplayInfo *pstDisplayInfo);

bool BXVD_P_IsDisplayInfoEqual_HIM_API_isr(BXVD_P_DisplayInfo stDisplayInfo,
                                           BXVD_P_DisplayInfo stDisplayInfo1);

#if BXVD_P_POWER_MANAGEMENT

#define BXVD_P_USE_SET_POWER_STATE_REVK0 1
#define BXVD_P_SET_POWER_STATE BXVD_P_SetPowerState_RevK0

void BXVD_P_SetPowerState_RevK0(BXVD_Handle hXvd,
                                BXVD_P_PowerState PowerState);

#endif /* BXVD_P_POWER_MANAGEMENT */

#endif /* _BXVD_PLATFORM_REVN0_H_ */
