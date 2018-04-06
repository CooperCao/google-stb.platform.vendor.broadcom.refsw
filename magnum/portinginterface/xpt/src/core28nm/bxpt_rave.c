/***************************************************************************
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
 *
 * Module Description:
 *
 ***************************************************************************/


#include "bstd.h"
#include "bxpt_priv.h"
#include "bxpt.h"
#include "bkni.h"
#include "blst_slist.h"
#include "bxpt_rave.h"
#include "bxpt_rave_ihex.h"
#include "bxpt_pcr_offset.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#include "bchp_xpt_rave.h"
#include "bchp_xpt_xpu.h"
#include "bchp_xpt_fe.h"
#include "bchp_sun_top_ctrl.h"

#if BXPT_HAS_RAVE_L2
        #include "bchp_xpt_rave_cpu_intr_aggregator.h"
        #include "bchp_xpt_rave_emu_error_cx00_31_l2_intr.h"
        #include "bchp_xpt_rave_cc_error_cx00_31_l2_intr.h"
        #include "bchp_xpt_rave_cdb_lower_thresh_cx00_31_l2_intr.h"
        #include "bchp_xpt_rave_cdb_min_depth_thresh_cx00_31_l2_intr.h"
        #include "bchp_xpt_rave_cdb_overflow_cx00_31_l2_intr.h"
        #include "bchp_xpt_rave_cdb_upper_thresh_cx00_31_l2_intr.h"
        #include "bchp_xpt_rave_itb_lower_thresh_cx00_31_l2_intr.h"
        #include "bchp_xpt_rave_itb_min_depth_thresh_cx00_31_l2_intr.h"
        #include "bchp_xpt_rave_itb_overflow_cx00_31_l2_intr.h"
        #include "bchp_xpt_rave_itb_upper_thresh_cx00_31_l2_intr.h"
        #include "bchp_xpt_rave_last_cmd_cx00_31_l2_intr.h"
        #include "bchp_xpt_rave_pusi_error_cx00_31_l2_intr.h"
        #include "bchp_xpt_rave_splice_cx00_31_l2_intr.h"
        #include "bchp_xpt_rave_tei_error_cx00_31_l2_intr.h"
        #if BXPT_NUM_TSIO
            #include "bchp_xpt_rave_tsio_dma_end_cx00_31_l2_intr.h"
        #endif
    #if BXPT_NUM_RAVE_CONTEXTS > 32
        #include "bchp_xpt_rave_emu_error_cx32_47_l2_intr.h"
        #include "bchp_xpt_rave_cc_error_cx32_47_l2_intr.h"
        #include "bchp_xpt_rave_cdb_lower_thresh_cx32_47_l2_intr.h"
        #include "bchp_xpt_rave_cdb_min_depth_thresh_cx32_47_l2_intr.h"
        #include "bchp_xpt_rave_cdb_overflow_cx32_47_l2_intr.h"
        #include "bchp_xpt_rave_cdb_upper_thresh_cx32_47_l2_intr.h"
        #include "bchp_xpt_rave_itb_lower_thresh_cx32_47_l2_intr.h"
        #include "bchp_xpt_rave_itb_min_depth_thresh_cx32_47_l2_intr.h"
        #include "bchp_xpt_rave_itb_overflow_cx32_47_l2_intr.h"
        #include "bchp_xpt_rave_itb_upper_thresh_cx32_47_l2_intr.h"
        #include "bchp_xpt_rave_last_cmd_cx32_47_l2_intr.h"
        #include "bchp_xpt_rave_pusi_error_cx32_47_l2_intr.h"
        #include "bchp_xpt_rave_splice_cx32_47_l2_intr.h"
        #include "bchp_xpt_rave_tei_error_cx32_47_l2_intr.h"
        #if BXPT_NUM_TSIO
            #include "bchp_xpt_rave_tsio_dma_end_cx32_47_l2_intr.h"
        #endif
    #endif
#endif

#if( BDBG_DEBUG_BUILD == 1 )
    BDBG_MODULE( xpt_rave );
#endif

/* Max number of contexts mapped in XPT_RAVE_CXMEM_LO */
#define CXMEM_LO_MAX_CONTEXT 16
#define CXMEM_HI_MAX_CONTEXT 24

#define CXMEM_A_MAX_CONTEXT 16
#define CXMEM_B_MAX_CONTEXT 32
#define CXMEM_C_MAX_CONTEXT 48

/* Number of bytes to skip when indexing from context X to X+1 */
#define RAVE_CONTEXT_REG_STEP       ( BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define REC_MISC_CFG_OFFSET         ( BCHP_XPT_RAVE_CX0_REC_MISC_CONFIG -  BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_MISC_CFG2_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG2 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_MISC_CFG3_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG3 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_MISC_CFG5_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG5 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define REC_SCD_PIDS_AB_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_AB - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_CD_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_CD - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_EF_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_EF - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_GH_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_GH - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define REC_SCD_PIDS_A_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_A - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_B_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_B - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_C_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_C - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_D_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_D - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_E_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_E - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_F_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_F - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_G_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_G - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_H_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_H - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define AV_INTERRUPT_ENABLES_OFFSET ( BCHP_XPT_RAVE_CX0_AV_INTERRUPT_ENABLES - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define TPIT_CTRL_REG_STEP          ( BCHP_XPT_RAVE_TPIT1_CTRL1 - BCHP_XPT_RAVE_TPIT0_CTRL1 )

/* PID tables aren't adjacent in memory; they have Par tables interleaved. So we need separate STEP and SIZE defines. */
#define TPIT_PID_TABLE_STEP         ( BCHP_XPT_RAVE_TPIT1_PID_TABLEi_ARRAY_BASE - BCHP_XPT_RAVE_TPIT0_PID_TABLEi_ARRAY_BASE )
#define TPIT_PARSE_TABLE_STEP       ( BCHP_XPT_RAVE_TPIT1_PAR_TABLEi_ARRAY_BASE - BCHP_XPT_RAVE_TPIT0_PAR_TABLEi_ARRAY_BASE )
#define TPIT_PID_TABLE_SIZE         (( BCHP_XPT_RAVE_TPIT0_PID_TABLEi_ARRAY_END - BCHP_XPT_RAVE_TPIT0_PID_TABLEi_ARRAY_START + 1 ) * 4 )
#define TPIT_PARSE_TABLE_SIZE       (( BCHP_XPT_RAVE_TPIT0_PAR_TABLEi_ARRAY_END - BCHP_XPT_RAVE_TPIT0_PAR_TABLEi_ARRAY_START + 1 ) * 4 )

#define REC_TS_CTRL_OFFSET          ( BCHP_XPT_RAVE_CX0_REC_TS_CTRL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_TIMER_OFFSET            ( BCHP_XPT_RAVE_CX0_REC_TIMER - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_COUNT_OFFSET            ( BCHP_XPT_RAVE_CX0_REC_COUNT - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define REC_STATE0_OFFSET           ( BCHP_XPT_RAVE_CX0_REC_STATE0 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_STATE1_OFFSET           ( BCHP_XPT_RAVE_CX0_REC_STATE1 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_STATE2_OFFSET           ( BCHP_XPT_RAVE_CX0_REC_STATE2 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_STATE2b_OFFSET          ( BCHP_XPT_RAVE_CX0_REC_STATE2b - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_STATE3_OFFSET           ( BCHP_XPT_RAVE_CX0_REC_STATE3 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define SCD_STATE_START_OFFSET      ( BCHP_XPT_RAVE_SCD0_SCD_COMP_STATE0 - BCHP_XPT_RAVE_SCD0_SCD_MISC_CONFIG )
#ifdef BCHP_XPT_RAVE_SCD0_SCD_STATE11
    #define SCD_STATE_END_OFFSET        ( BCHP_XPT_RAVE_SCD0_SCD_STATE11 - BCHP_XPT_RAVE_SCD0_SCD_MISC_CONFIG )
#else
    #define SCD_STATE_END_OFFSET        ( BCHP_XPT_RAVE_SCD0_RESERVED_STATE3 - BCHP_XPT_RAVE_SCD0_SCD_MISC_CONFIG )
#endif

#define AV_MISC_CFG1_OFFSET         ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG1 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_MISC_CFG2_OFFSET         ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG2 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_MISC_CFG3_OFFSET         ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG3 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_MISC_CFG4_OFFSET         ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG4 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_MISC_CFG5_OFFSET         ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG5 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define ATSOFFSET_CFG_OFFSET        ( BCHP_XPT_RAVE_CX0_ATSOFFSET_CONFIG - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define REC_CTRL1_OFFSET            ( BCHP_XPT_RAVE_CX0_REC_CTRL1 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_INIT_TS_OFFSET          ( BCHP_XPT_RAVE_CX0_REC_INIT_TS - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_VID_SC_RANGE_AB_OFFSET   ( BCHP_XPT_RAVE_CX0_AV_VID_SC_RANGE_AB - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_VID_SC_RANGE_CD_OFFSET   ( BCHP_XPT_RAVE_CX0_AV_VID_SC_RANGE_CD - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define SCD_REG_STEP                ( BCHP_XPT_RAVE_SCD1_SCD_MISC_CONFIG - BCHP_XPT_RAVE_SCD0_SCD_MISC_CONFIG )
#define SPID_CHNL_STEPSIZE          ( 4 )
#define CXMEM_CHNL_STEPSIZE         ( 4 )

/* Offset of context registers from the first register in the block. */
#define CDB_READ_PTR_OFFSET     ( BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define CDB_BASE_PTR_OFFSET     ( BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define CDB_END_PTR_OFFSET      ( BCHP_XPT_RAVE_CX0_AV_CDB_END_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define CDB_WRAP_PTR_OFFSET     ( BCHP_XPT_RAVE_CX0_AV_CDB_WRAPAROUND_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define CDB_VALID_PTR_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_CDB_VALID_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define CDB_WRITE_PTR_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define CDB_DEPTH_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_CDB_DEPTH - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define ITB_READ_PTR_OFFSET     ( BCHP_XPT_RAVE_CX0_AV_ITB_READ_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define ITB_BASE_PTR_OFFSET     ( BCHP_XPT_RAVE_CX0_AV_ITB_BASE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define ITB_END_PTR_OFFSET      ( BCHP_XPT_RAVE_CX0_AV_ITB_END_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define ITB_WRAP_PTR_OFFSET     ( BCHP_XPT_RAVE_CX0_AV_ITB_WRAPAROUND_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define ITB_VALID_PTR_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_ITB_VALID_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define ITB_WRITE_PTR_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_ITB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define ITB_DEPTH_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_ITB_DEPTH - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define AV_COMP1_CTRL_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_COMP1_CONTROL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP1_COMPARE_OFFSET ( BCHP_XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP1_MASK_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_COMP1_MASK_VAL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP2_CTRL_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_COMP2_CONTROL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP2_COMPARE_OFFSET ( BCHP_XPT_RAVE_CX0_AV_COMP2_COMPARE_VAL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP2_MASK_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_COMP2_MASK_VAL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_EXCLUSION_OFFSET     ( BCHP_XPT_RAVE_CX0_AV_EXCLUSION - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_PID_STREAM_ID_OFFSET ( BCHP_XPT_RAVE_CX0_AV_PID_STREAM_ID - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_THRESHOLDS_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_THRESHOLDS - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_CDB_THRESHOLD_OFFSET ( BCHP_XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_ITB_THRESHOLD_OFFSET ( BCHP_XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_CDB_MIN_DEPTH_THRESHOLD_OFFSET ( BCHP_XPT_RAVE_CX0_AV_CDB_MIN_DEPTH_THRESHOLD - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_ITB_MIN_DEPTH_THRESHOLD_OFFSET ( BCHP_XPT_RAVE_CX0_AV_ITB_MIN_DEPTH_THRESHOLD - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_TIME_CONFIG_OFFSET  ( BCHP_XPT_RAVE_CX0_REC_TIME_CONFIG - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_RESERVE_CFG2_OFFSET  ( BCHP_XPT_RAVE_CX0_AV_RESERVE_CFG2 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define AV_COMP_STATE0_OFFSET       ( BCHP_XPT_RAVE_CX0_AV_COMP_STATE0 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP_STATE1_OFFSET       ( BCHP_XPT_RAVE_CX0_AV_COMP_STATE1 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP_STATE2_OFFSET       ( BCHP_XPT_RAVE_CX0_AV_COMP_STATE2 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP_STATE3_OFFSET       ( BCHP_XPT_RAVE_CX0_AV_COMP_STATE3 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP_STATE4_OFFSET       ( BCHP_XPT_RAVE_CX0_AV_COMP_STATE4 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP_STATE5_OFFSET       ( BCHP_XPT_RAVE_CX0_AV_COMP_STATE5 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP_STATE6_OFFSET       ( BCHP_XPT_RAVE_CX0_AV_COMP_STATE6 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_PES_STATE0_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_PES_STATE0 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_PES_STATE1_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_PES_STATE1 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_PES_STATE2_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_PES_STATE2 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_PES_STATE3_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_PES_STATE3 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_PACKET_COUNT_OFFSET      ( BCHP_XPT_RAVE_CX0_AV_PACKET_COUNT - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_RESERVE_STATE0_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_RESERVE_STATE0 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define SCD_CTRL1_OFFSET            ( BCHP_XPT_RAVE_SCD0_SCD_CTRL1 - BCHP_XPT_RAVE_SCD0_SCD_MISC_CONFIG )
#define REC_CDB_THRESHOLD_OFFSET    ( BCHP_XPT_RAVE_CX0_REC_CDB_THRESHOLD_LEVEL - BCHP_XPT_RAVE_CX0_REC_CDB_WRITE_PTR )
#define REC_ITB_THRESHOLD_OFFSET    ( BCHP_XPT_RAVE_CX0_REC_ITB_THRESHOLD_LEVEL - BCHP_XPT_RAVE_CX0_REC_CDB_WRITE_PTR )

#define TPIT_CTRL1_OFFSET           ( BCHP_XPT_RAVE_TPIT0_CTRL1 - BCHP_XPT_RAVE_TPIT0_CTRL1 )
#define TPIT_COR1_OFFSET            ( BCHP_XPT_RAVE_TPIT0_COR1 - BCHP_XPT_RAVE_TPIT0_CTRL1 )
#define TPIT_PID_TABLE_ENTRY_STEP   ( 4 )
#define TPIT_PAR_TABLE_ENTRY_STEP   ( 4 )
#define TPIT_STATE_START_OFFSET     ( BCHP_XPT_RAVE_TPIT0_STATE0 - BCHP_XPT_RAVE_TPIT0_CTRL1 )
#define TPIT_STATE_END_OFFSET       ( BCHP_XPT_RAVE_TPIT0_STATE9 - BCHP_XPT_RAVE_TPIT0_CTRL1 )
#define TPIT_TID_OFFSET             ( BCHP_XPT_RAVE_TPIT0_TID - BCHP_XPT_RAVE_TPIT0_CTRL1 )
#define TPIT_TID2_OFFSET            ( BCHP_XPT_RAVE_TPIT0_TID2 - BCHP_XPT_RAVE_TPIT0_CTRL1 )
#define TPIT_TIME_TICK_OFFSET       ( BCHP_XPT_RAVE_TPIT_TIME_TICK - BCHP_XPT_RAVE_TPIT0_CTRL1 )

#define PICTURE_CTR_OFFSET          ( BCHP_XPT_RAVE_CX0_PIC_CTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define PIC_INC_DEC_CTRL_OFFSET     ( BCHP_XPT_RAVE_CX0_PIC_INC_DEC_CTRL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define PIC_CTR_MODE_OFFSET         ( BCHP_XPT_RAVE_CX0_PIC_CTR_MODE - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define AV_COMP1_COMPARE_VAL_OFFSET ( BCHP_XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP1_MASK_VAL_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_COMP1_MASK_VAL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP2_COMPARE_VAL_OFFSET ( BCHP_XPT_RAVE_CX0_AV_COMP2_COMPARE_VAL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP2_MASK_VAL_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_COMP2_MASK_VAL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP1_FILTER_VAL_OFFSET  ( BCHP_XPT_RAVE_CX0_AV_COMP1_FILTER_VAL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP1_FILTER_VAL_MASK_OFFSET  ( BCHP_XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP2_FILTER_VAL_OFFSET  ( BCHP_XPT_RAVE_CX0_AV_COMP2_FILTER_VAL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP2_FILTER_VAL_MASK_OFFSET  ( BCHP_XPT_RAVE_CX0_AV_COMP2_FILTER_MASK_VAL - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_COMP12_FILTER_MODE_OFFSET ( BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define SCRAMBLE_CTRL_OFFSET        ( BCHP_XPT_RAVE_CX0_SC - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

/* Some chips don't have these defined in their RDB yet */
#define RAVE_CFG_ARRAY_BASE         ( BCHP_XPT_XPU_IMEMi_ARRAY_BASE )
#define RAVE_CFG_MISC_REG           ( BCHP_XPT_XPU_IMEMi_ARRAY_BASE - 0x800 + 4 )

#define CTX_IP_OVERFLOW_THRESH      ( 0x100 )

/* For right now, the first 6 contexts are dedicated AV. The rest are dedicated records. */
#define BXPT_NUM_AV_CONTEXT           ( 6 )

/* RAVE only supports buffers aligned on 4-byte boundaries. This alignment is expressed as a power of 2. */
#define RAVE_BUFFER_ALIGNMENT           ( 2 )

/* Base address in data memory and size to be used for context-specific parameters */
/* SCD_DMEM_BASE was 0x540 */
#if BCHP_XPT_RAVE_DMEMi_ARRAY_END == 4095
    #define SCD_DMEM_BASE  0x800
#else
    #define SCD_DMEM_BASE  0x360
#endif
#define SCD_DMEM_SIZE  0x20

/* An offset into the CTX_DMEM area, where the pointer to the splicing stack is stored. */
#define SPLICE_QUEUE_PTR_OFFSET         ( 0x17 )

/* Offset into CTX_DMEM area, where the actual splicing stacks are stored. Also the size of the stacks */
#define SPLICE_QUEUE_AREA_OFFSET        ( 0x600 * 4 )
#define SPLICE_QUEUE_AREA_SIZE          ( 0x20 * 4 )

#define SPLICE_RD_PTR_OFFSET            ( 0 )
#define SPLICE_WR_PTR_OFFSET            ( 1 * 4 )
#define SPLICE_CURR_PID_MSB_OFFSET      ( 2 * 4 )
#define SPLICE_CURR_PID_LSB_OFFSET      ( 3 * 4 )
#define SPLICE_QUEUE_LOC_0_MSB_OFFSET   ( 4 * 4 )
#define SPLICE_QUEUE_LOC_0_LSB_OFFSET   ( 5 * 4 )

#define SPLICE_MAX_SLOT                 ( 8 )
#define SPLICE_QUEUE_SIZE               ( 8 )
#define MAX_THRESHOLD_GRANULARITY       (4096)
#define MIN_THRESHOLD_GRANULARITY       (32)
#define DEFAULT_THRESHOLD_GRANULARITY   (256)

#define BXPT_ITB_SIZE 16

/* Address delta (in bytes) between consecutive registers in a context. */
#define REG_STEP  (BCHP_XPT_RAVE_CX0_AV_ITB_DEPTH - BCHP_XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL)

typedef enum ScdIdent
{
    ScdIdent_A,
    ScdIdent_B,
    ScdIdent_C,
    ScdIdent_D,
    ScdIdent_E,
    ScdIdent_F,
    ScdIdent_G,
    ScdIdent_H,
    ScdIdent_Max    /* Marks the end of the list */
}
ScdIdent;

static BERR_Code ClearMem( BXPT_Rave_Handle hRave );
static BERR_Code InitScd( StartCodeIndexer *lhScd );
static BERR_Code InitTpit( TpitIndexer *lhTpit );
static BERR_Code InitContext( BXPT_RaveCx_Handle ThisCtx, const BAVC_CdbItbConfig *BufferCfg );
static BERR_Code LoadScRanges( BXPT_RaveCx_Handle Context, const BXPT_Rave_EsRanges *EsRanges );
static BERR_Code GetScRanges( BXPT_RaveCx_Handle Context, BXPT_Rave_EsRanges *EsRanges );
static BERR_Code ConfigureComparators( BXPT_RaveCx_Handle hCtx, BAVC_ItbEsType StreamType );
static BERR_Code AllocateSpliceQueue( BXPT_RaveCx_Handle hCtx, unsigned *QueueNum );
static BERR_Code ResetContextPointers( BXPT_RaveCx_Handle hCtx );
static BERR_Code ChangeScdPid( BXPT_RaveCx_Handle hCtx, unsigned WhichScd, unsigned WhichScdBlock, unsigned Pid, bool PidValid );
static BERR_Code FlushScds( BXPT_RaveCx_Handle hCtx );
static BERR_Code ClearScdStates( BXPT_RaveCx_Handle hCtx, uint32_t ScdNum );
static BERR_Code FlushTpit( BXPT_RaveCx_Handle hCtx, uint32_t TpitNum );
static int GetNextPicCounter( BXPT_RaveCx_Handle hCtx );
static void FreePicCounter( BXPT_RaveCx_Handle Context );
static void FlushPicCounter( BXPT_RaveCx_Handle hCtx );
static uint32_t GetPictureCounterReg( BXPT_RaveCx_Handle hCtx );
static void SetPictureCounterMode( BXPT_RaveCx_Handle hCtx, BAVC_ItbEsType ItbFormat );
static void FreeScds( unsigned NumScds, StartCodeIndexer **ScdPtr );
static BERR_Code SetExtractionBitCounts( BXPT_RaveCx_Handle hCtx, StartCodeIndexer *hScd, const BXPT_Rave_ScdEntry *ScdConfig, unsigned WhichScd, bool SvcMvcMode );
static void GetScEnables( BXPT_Rave_EsRanges *Range, unsigned Mode );
static void GetScEnables_Indexer( IndexerScRange *Range, unsigned Mode );
static BERR_Code GetNextFreeScd( BXPT_Rave_Handle hRave, BXPT_RaveCx RequestedType, unsigned *ReturnedScdNum );
static BERR_Code GetNextRaveContext( BXPT_Rave_Handle hRave, BXPT_RaveCx RequestedType, unsigned *ReturnedIndex );
static BERR_Code GetNextSoftRaveContext( BXPT_Rave_Handle hRave, unsigned *ReturnedIndex );

static BERR_Code AllocContext_Priv( BXPT_Rave_Handle hRave, BXPT_RaveCx RequestedType, const BAVC_CdbItbConfig *BufferCfg, unsigned Index, BXPT_RaveCx_Handle *Context );
static BERR_Code AllocSoftContext_Priv( BXPT_RaveCx_Handle SrcContext, BXPT_RaveSoftMode DestContextMode, unsigned index, BXPT_RaveCx_Handle *DestContext );

static uint32_t GetGranularityInBytes( BXPT_RaveCx_Handle hCtx );
static BERR_Code MapGranularityToHw( unsigned GranularityInBytes, uint32_t *Wmark );
static size_t GetMaxBufferByGranularity( BXPT_Rave_Handle hRave );
static BERR_Code GetScdPid( BXPT_RaveCx_Handle hCtx, unsigned WhichScd, unsigned *WhichScdBlock, unsigned *Pid, bool *PidValid );
static BERR_Code EnableScdByContext( BXPT_RaveCx_Handle Context, unsigned int PidChanNum );
static void ClearSpidTable( BXPT_RaveCx_Handle Context, unsigned int PidChanNum, uint32_t PipeInUse );

static BERR_Code ClearCxHold( BXPT_RaveCx_Handle hCtx );

static void ClearDataDropOnPusiErrState(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t Reg, RegAddr;
    #ifdef BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG5
        RegAddr = hCtx->BaseAddr + BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG5 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR;
    #else
        RegAddr = hCtx->BaseAddr + BCHP_XPT_RAVE_CX0_RAVE_Reg_0 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR;
    #endif
    Reg = BREG_Read32( hCtx->hReg, RegAddr );
    Reg &= ~( 1 << 13 );
    BREG_Write32( hCtx->hReg, RegAddr, Reg );
}

static void SetScdNum(
    BXPT_RaveCx_Handle hCtx,
    unsigned Scd,
    unsigned ScdNum
    )
{
    uint32_t Reg;

    BDBG_ASSERT(Scd == 0); /* Unsupported SCD. Additional code needed. */

#ifdef BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_AB
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET );
    Reg &= ~(
       BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA, ScdNum )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET, Reg );
#else
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_A_OFFSET );
    Reg &= ~(
       BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_PID_channels_SCD_NUMA )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_PID_channels_SCD_NUMA, ScdNum )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_A_OFFSET, Reg );
#endif
    BSTD_UNUSED(Scd);
}

BERR_Code BXPT_Rave_GetChannelDefaultSettings(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport instance */
    unsigned ChannelNo,                         /* [in] Which RAVE instance to get defaults for */
    BXPT_Rave_ChannelSettings *RaveDefSettings  /* [out] The defaults. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BSTD_UNUSED( hXpt );
    BDBG_ASSERT( RaveDefSettings );

    BSTD_UNUSED( ChannelNo );

    RaveDefSettings->TpitEventTimeout = 0;
    RaveDefSettings->TpitPacketTimeout = 0;
    RaveDefSettings->TimeTick = 0;
    RaveDefSettings->chanOpenCB = NULL;
    RaveDefSettings->ThresholdGranularityInBytes = DEFAULT_THRESHOLD_GRANULARITY;

    return( ExitCode );
}

static BXPT_P_ContextHandle * allocContextHandle(
    BXPT_Rave_Handle hRave,
    unsigned Index
    )
{
    BXPT_P_ContextHandle *ThisCtx = hRave->ContextTbl[ Index ] = BKNI_Malloc(sizeof(BXPT_P_ContextHandle));

    if(!ThisCtx)
    {
        BDBG_ERR(("%s malloc failed for context handle", BSTD_FUNCTION));
        goto Done;
    }
    BKNI_Memset(ThisCtx, 0, sizeof(*ThisCtx));

    ThisCtx->hChp = hRave->hChip;
    ThisCtx->hReg = hRave->hReg;
    ThisCtx->hInt = hRave->hInt;
    ThisCtx->mmaHeap = hRave->mmaHeap;
    ThisCtx->mmaRHeap = hRave->mmaRHeap;
    ThisCtx->Allocated = true;
    ThisCtx->Index = Index;     /* Absolute context index, not type dependent. */
    ThisCtx->vhRave = ( void * ) hRave;
    ThisCtx->PicCounter = -1;    /* Invalid picture counter. */
    ThisCtx->IsSoftContext = false;

    ThisCtx->BaseAddr = BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR + ( Index * RAVE_CONTEXT_REG_STEP );
    InitContext( ThisCtx, NULL );
    Done:
    return ThisCtx;
}


static void clearContextRegs(BREG_Handle hRegister, unsigned index)
{
   unsigned reg;
   unsigned BaseAddr = BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR + ( index * RAVE_CONTEXT_REG_STEP );

   BREG_WriteAddr( hRegister, BaseAddr + CDB_WRITE_PTR_OFFSET, 0 );
   BREG_WriteAddr( hRegister, BaseAddr + CDB_READ_PTR_OFFSET, 0 );
   BREG_WriteAddr( hRegister, BaseAddr + CDB_BASE_PTR_OFFSET, 0 );
   BREG_WriteAddr( hRegister, BaseAddr + CDB_END_PTR_OFFSET, 0 );
   BREG_WriteAddr( hRegister, BaseAddr + CDB_VALID_PTR_OFFSET, 0 );
   BREG_WriteAddr( hRegister, BaseAddr + CDB_WRAP_PTR_OFFSET, 0 );
   BREG_Write32( hRegister, BaseAddr + AV_CDB_THRESHOLD_OFFSET, 0 );
   BREG_Write32( hRegister, BaseAddr + CDB_DEPTH_OFFSET, 0 );

   BREG_Write32( hRegister, BaseAddr + AV_THRESHOLDS_OFFSET, 0 );

   BREG_WriteAddr( hRegister, BaseAddr + ITB_WRITE_PTR_OFFSET, 0 );
   BREG_WriteAddr( hRegister, BaseAddr + ITB_READ_PTR_OFFSET, 0 );
   BREG_WriteAddr( hRegister, BaseAddr + ITB_BASE_PTR_OFFSET, 0 );
   BREG_WriteAddr( hRegister, BaseAddr + ITB_END_PTR_OFFSET, 0 );
   BREG_WriteAddr( hRegister, BaseAddr + ITB_VALID_PTR_OFFSET, 0 );
   BREG_WriteAddr( hRegister, BaseAddr + ITB_WRAP_PTR_OFFSET, 0 );

   for (reg = BCHP_XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL; reg < BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR; reg += REG_STEP )
   {
      BREG_Write32(hRegister, BaseAddr + reg - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR, 0);
   }
}

BERR_Code BXPT_Rave_OpenChannel(
    BXPT_Handle hXpt,                                   /* [in] Handle for this transport instance */
    BXPT_Rave_Handle *hRave,                            /* [out] Handle for the RAVE channel */
    unsigned ChannelNo,                                 /* [in] Which RAVE channel to open */
    const BXPT_Rave_ChannelSettings *RaveDefSettings    /* [in] Default settings to use */
    )
{
    uint32_t Reg;
    unsigned Index;
    unsigned WaterMarkGranularity;

    BXPT_Rave_Handle lhRave = NULL;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( hRave );
    BDBG_ASSERT( RaveDefSettings );

    if( ChannelNo >= BXPT_NUM_RAVE_CHANNELS )
    {
        BDBG_ERR(( "ChannelNo %u out of range!", ChannelNo ));
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        goto Error1;
    }

    if( hXpt->RaveChannels[ ChannelNo ].Allocated == true )
    {
        BDBG_ERR(( "ChannelNo %u already allocated!", ChannelNo ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Error1;
    }

#if BXPT_RAVE_AUDIO_STARTCODES
    BDBG_MSG(( "Audio startcode marking ENABLED" ));
#else
    BDBG_MSG(( "Audio startcode marking DISABLED" ));
#endif

#ifdef UNIFIED_ITB_SUPPORT
    BDBG_MSG(( "Using unified ITB format" ));
#else
    BDBG_MSG(( "Using legacy ITB format" ));
#endif

    /* Allocate memory for the handle. */
    lhRave = BKNI_Malloc( sizeof( BXPT_P_RaveHandle ) );
    if( lhRave == NULL )
    {
        BDBG_ERR(( "BKNI_Malloc() failed!" ));
        ExitCode = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        goto Error1;
    }
    if( RaveDefSettings->ThresholdGranularityInBytes > MAX_THRESHOLD_GRANULARITY
    || RaveDefSettings->ThresholdGranularityInBytes < MIN_THRESHOLD_GRANULARITY )
    {
        BDBG_ERR(( "ThresholdGranularityInBytes %u out of range (%u to %u)!",
            RaveDefSettings->ThresholdGranularityInBytes, MIN_THRESHOLD_GRANULARITY, MAX_THRESHOLD_GRANULARITY ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Error2;
    }
    if( MapGranularityToHw( RaveDefSettings->ThresholdGranularityInBytes, &WaterMarkGranularity ) != BERR_SUCCESS )
    {
        BDBG_ERR(( "Invalid ThresholdGranularityInBytes: %d", RaveDefSettings->ThresholdGranularityInBytes ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Error2;
    }

    BXPT_P_AcquireSubmodule(hXpt, BXPT_P_Submodule_eRave);

    lhRave->hChip = hXpt->hChip;
    lhRave->hReg = hXpt->hRegister;
    lhRave->hInt = hXpt->hInt;
    lhRave->ChannelNo = ChannelNo;
    lhRave->lvXpt = hXpt;
    lhRave->chanOpenCB = RaveDefSettings->chanOpenCB;
    lhRave->mmaHeap = hXpt->mmaHeap;
    lhRave->mmaRHeap = hXpt->mmaRHeap;

    /*
    ** Init each of the contexts. This section changes when/if they support
    ** dynamic changing of context type (AV versus record).
    */
    for( Index = 0; Index < BXPT_NUM_RAVE_CONTEXTS; Index++ )
    {
        *(lhRave->ContextTbl + Index) = NULL;
        clearContextRegs(lhRave->hReg, Index);
    }

    for( Index = 0; Index < BXPT_P_NUM_SPLICING_QUEUES; Index++ )
    {
        lhRave->SpliceQueueAllocated[ Index ] = false;
    }

    for( Index = 0; Index < BXPT_NUM_SCD; Index++ )
    {
        StartCodeIndexer *lhScd = lhRave->ScdTable + Index;

        lhScd->hReg = lhRave->hReg;
        lhScd->hInt = lhRave->hInt;
        lhScd->ChannelNo = Index;
        lhScd->BaseAddr = BCHP_XPT_RAVE_SCD0_SCD_MISC_CONFIG + ( Index * SCD_REG_STEP );
        lhScd->Allocated = false;
        InitScd( lhScd );
    }

    for( Index = 0; Index < BXPT_NUM_TPIT; Index++ )
    {
        TpitIndexer *lhTpit = lhRave->TpitTable + Index;

        lhTpit->hReg = lhRave->hReg;
        lhTpit->hInt = lhRave->hInt;
        lhTpit->ChannelNo = Index;
        lhTpit->BaseAddr = BCHP_XPT_RAVE_TPIT0_CTRL1 + ( Index * TPIT_CTRL_REG_STEP );
        lhTpit->PidTableBaseAddr = BCHP_XPT_RAVE_TPIT0_PID_TABLEi_ARRAY_BASE + ( Index * TPIT_PID_TABLE_STEP );
        lhTpit->ParseTableBaseAddr = BCHP_XPT_RAVE_TPIT0_PAR_TABLEi_ARRAY_BASE + ( Index * TPIT_PARSE_TABLE_STEP );
        lhTpit->Allocated = false;
        InitTpit( lhTpit );
    }

    for( Index = 0; Index < BXPT_NUM_SCD + BXPT_NUM_TPIT; Index++ )
    {
        BXPT_P_IndexerHandle *lhIdx = lhRave->IndexerHandles + Index;

        lhIdx->ChannelType = BXPT_RaveIdx_eScd;
        lhIdx->hChannel = NULL;
        lhIdx->Allocated = false;
    }

    /* Init memory spaces. */
    ClearMem( lhRave );
    BXPT_P_RaveRamInit( lhRave );

    /* Load the defaults. */
#if 0
    /* Set starting addresses of Hardware Assist A and Buffer A packet data. */
    Reg = BREG_Read32( lhRave->hReg, BCHP_XPT_RAVE_DATA_START_ADDR_A );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_DATA_START_ADDR_A, HWA_START ) |
        BCHP_MASK( XPT_RAVE_DATA_START_ADDR_A, PKT_START )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_DATA_START_ADDR_A, HWA_START, 0xC0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_DATA_START_ADDR_A, PKT_START, 0 )
    );
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_DATA_START_ADDR_A, Reg );

    /* Set starting addresses of Hardware Assist B and Buffer B packet data. */
    Reg = BREG_Read32( lhRave->hReg, BCHP_XPT_RAVE_DATA_START_ADDR_B );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_DATA_START_ADDR_B, HWA_START ) |
        BCHP_MASK( XPT_RAVE_DATA_START_ADDR_B, PKT_START )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_DATA_START_ADDR_B, HWA_START, 0x240 ) |
        BCHP_FIELD_DATA( XPT_RAVE_DATA_START_ADDR_B, PKT_START, 0x180 )
    );
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_DATA_START_ADDR_B, Reg );
#endif

    /* Misc register setup. */
    Reg = BREG_Read32( lhRave->hReg, BCHP_XPT_RAVE_MISC_CONTROL );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, PS_WAKE ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, AV_ENABLE ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, NUM_DMA_CYCLES ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, COUNTER_MODE  ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, HW_FORCE_SWITCH_EN ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, FORCE_SWITCH  ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, PES_COMPARATOR_RESET ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, INPUT_READ_RATE ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, MUX_BUFFER_SLOT_SIZE ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, DMA_SPEEDUP_EN ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, AV_WMARK_CLEAR ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, EMU_STATE_CLEAR ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, NUM_CONTEXTS ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, WMARK_GRANULARITY ) |
        BCHP_MASK( XPT_RAVE_MISC_CONTROL, PACKET_CNT_CLR )
    );

    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, PS_WAKE, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, AV_ENABLE, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, NUM_DMA_CYCLES, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, COUNTER_MODE, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, HW_FORCE_SWITCH_EN, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, FORCE_SWITCH, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, PES_COMPARATOR_RESET, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, INPUT_READ_RATE, 2 ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, MUX_BUFFER_SLOT_SIZE, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, DMA_SPEEDUP_EN, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, AV_WMARK_CLEAR, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, EMU_STATE_CLEAR, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, NUM_CONTEXTS, BXPT_NUM_RAVE_CONTEXTS ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, WMARK_GRANULARITY, WaterMarkGranularity ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, PACKET_CNT_CLR, 0 )
    );
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_MISC_CONTROL, Reg );

    Reg = BREG_Read32( lhRave->hReg, BCHP_XPT_RAVE_MISC_CONTROL3 );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_MISC_CONTROL3, MIN_DEPTH_THRESHOLD_INTR_MODE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL3, MIN_DEPTH_THRESHOLD_INTR_MODE, 1 )
    );
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_MISC_CONTROL3, Reg );

    /* Mark this channel as allocated and return the handle. */
    hXpt->RaveChannels[ ChannelNo ].Allocated = true;

    /* Set up the TPIT event timeouts. */
    Reg = BREG_Read32( lhRave->hReg, BCHP_XPT_RAVE_TPIT_PKT_TIMEOUT );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_TPIT_PKT_TIMEOUT, TPIT_PKT_TIMEOUT )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_TPIT_PKT_TIMEOUT, TPIT_PKT_TIMEOUT, RaveDefSettings->TpitPacketTimeout )
    );
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_TPIT_PKT_TIMEOUT, Reg );

    Reg = BREG_Read32( lhRave->hReg, BCHP_XPT_RAVE_TPIT_EVE_TIMEOUT );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_TPIT_EVE_TIMEOUT, TPIT_EVE_TIMEOUT )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_TPIT_EVE_TIMEOUT, TPIT_EVE_TIMEOUT, RaveDefSettings->TpitEventTimeout )
    );
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_TPIT_EVE_TIMEOUT, Reg );

    Reg = BREG_Read32( lhRave->hReg, BCHP_XPT_RAVE_TPIT_TIME_TICK );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_TPIT_TIME_TICK, TPIT_TIME_TICK )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_TPIT_TIME_TICK, TPIT_TIME_TICK, RaveDefSettings->TimeTick )
    );
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_TPIT_TIME_TICK, Reg );

    hXpt->RaveChannels[ ChannelNo ].Handle = *hRave = lhRave; /* lhRave is initialized to NULL at the start. */

#ifdef BCHP_PWR_RESOURCE_XPT
    hXpt->vhRave = (void *) lhRave;
#endif

    BXPT_Rave_P_EnableInterrupts( hXpt );

#ifdef BCHP_XPT_RAVE_PUSH_TIMER_EN_CX_0_31
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_PUSH_TIMER_EN_CX_0_31, 0xFFFFFFFF );
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_PUSH_TIMER_EN_CX_32_47, 0xFFFF );
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_PUSH_TIMER_LOAD, 0xD304 );
#endif

#ifdef BCHP_XPT_RAVE_WRMASK_OPTIMIZATION_DIS_CX_0_31
    BREG_Write32(lhRave->hReg, BCHP_XPT_RAVE_WRMASK_OPTIMIZATION_DIS_CX_0_31, 0xFFFFFFFF);
    BREG_Write32(lhRave->hReg, BCHP_XPT_RAVE_WRMASK_OPTIMIZATION_DIS_CX_32_47, 0xFFFFFFFF);
#endif

#ifdef BCHP_XPT_RAVE_MISC_CONTROL3_CRXPT_1164_DIS_MASK
    Reg = BREG_Read32( lhRave->hReg, BCHP_XPT_RAVE_MISC_CONTROL3 );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_MISC_CONTROL3, CRXPT_1164_DIS )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL3, CRXPT_1164_DIS, 1 )
    );
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_MISC_CONTROL3, Reg );
#endif

    BXPT_P_ReleaseSubmodule(hXpt, BXPT_P_Submodule_eRave);

    return ExitCode;

    Error2:
    BKNI_Free( lhRave );

    Error1:
    *hRave = NULL;
    return ExitCode;
}

void BXPT_Rave_P_EnableInterrupts(
    BXPT_Handle hXpt                                /* [in] Handle for this transport instance */
    )
{
#if BXPT_HAS_RAVE_L2
    uint32_t enables = BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_FW_GENERIC_1_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_ITB_MIN_DEPTH_THRESH_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_CDB_MIN_DEPTH_THRESH_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_ITB_UPPER_THRESH_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_ITB_LOWER_THRESH_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_CDB_UPPER_THRESH_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_CDB_LOWER_THRESH_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_LAST_CMD_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_SPLICE_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_ITB_OVERFLOW_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_CDB_OVERFLOW_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_CC_ERROR_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_TEI_ERROR_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_PUSI_ERROR_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_EMU_ERROR_INTR_MASK |
        BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_MISC_INTR_MASK;

#if BXPT_NUM_TSIO
    enables |= BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_RAVE_TSIO_DMA_END_INTR_MASK;
#endif

    BREG_Write32( hXpt->hRegister, BCHP_XPT_RAVE_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR, enables );
#else
    BSTD_UNUSED( hXpt );
#endif
}

BERR_Code BXPT_Rave_CloseChannel(
    BXPT_Rave_Handle hRave     /* [in] Handle for this RAVE instance */
    )
{
    BXPT_Handle hXpt;
    unsigned Index;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hRave );

    hXpt = ( BXPT_Handle ) hRave->lvXpt;
    hXpt->RaveChannels[ hRave->ChannelNo ].Allocated = false;

    /* Stop the XPU? */

    /* Free any context buffers left hanging around */
    for( Index = 0; Index < BXPT_NUM_RAVE_CONTEXTS; Index++ )
        if( hRave->ContextTbl[ Index ] )
            BXPT_Rave_FreeContext( *(hRave->ContextTbl + Index) );

    BKNI_Free( hRave );

    return( ExitCode );
}

BERR_Code BXPT_Rave_AllocAvsCxPair(
    BXPT_Rave_Handle hRave,         /* [in] Handle for this RAVE channel */
    const BXPT_Rave_AllocCxSettings *pDecodeCxSettings, /* [in] settings for this RAVE channel allocation */
    const BXPT_Rave_AllocCxSettings *pReferenceCxSettings, /* [in] settings for this RAVE channel allocation */
    BXPT_RaveCx_Handle *DecodeContext,     /* [out] The allocated decode context */
    BXPT_RaveCx_Handle *ReferenceContext     /* [out] The allocated context */
    )
{
    BSTD_UNUSED(pReferenceCxSettings);
    *ReferenceContext = NULL;
    return BXPT_Rave_AllocCx(hRave, pDecodeCxSettings, DecodeContext);
}

void BXPT_Rave_GetDefaultAllocCxSettings(
    BXPT_Rave_AllocCxSettings *pSettings /* [out] default settings */
    )
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(BXPT_Rave_AllocCxSettings));
    pSettings->RequestedType = BXPT_RaveCx_eAv;
}

BERR_Code BXPT_Rave_AllocCx(
    BXPT_Rave_Handle hRave,         /* [in] Handle for this RAVE channel */
    const BXPT_Rave_AllocCxSettings *pSettings, /* [in] settings for this RAVE channel allocation */
    BXPT_RaveCx_Handle *Context     /* [out] The allocated context */
    )
{
    unsigned Index;
    BERR_Code rc;
    BXPT_RaveCx_Handle ThisCtx = NULL;

    BDBG_ASSERT( hRave );
    BDBG_ASSERT( Context );
    BXPT_P_AcquireSubmodule(hRave->lvXpt, BXPT_P_Submodule_eRave);

    if( pSettings->SoftRaveSrcCx )
    {
        if( GetNextSoftRaveContext( hRave, &Index ) != BERR_SUCCESS )
        {
            BDBG_ERR(( "No free soft contexts!" ));
            rc = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
            goto error;
        }

        if( !(ThisCtx = allocContextHandle(hRave, Index)) )
        {
            BDBG_ERR(( "Can't allocate a soft context handle!" ));
            rc = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
            goto error;
        }
        ThisCtx->externalItbAlloc = pSettings->ItbBlock!=NULL;
        ThisCtx->mma.itbBlock = pSettings->ItbBlock;
        ThisCtx->mma.itbBlockOffset = pSettings->ItbBlock ? pSettings->ItbBlockOffset : 0;

        return AllocSoftContext_Priv( pSettings->SoftRaveSrcCx, pSettings->SoftRaveMode, Index, Context );
    }
    else
    {
        if( GetNextRaveContext( hRave, pSettings->RequestedType, &Index ) != BERR_SUCCESS )
        {
            BDBG_ERR(( "No free contexts!" ));
            rc = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
            goto error;
        }

        if( !(ThisCtx = allocContextHandle(hRave, Index)) )
        {
            BDBG_ERR(( "Can't allocate a context handle!" ));
            rc = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
            goto error;
        }
        ThisCtx->externalCdbAlloc = pSettings->CdbBlock!=NULL;
        ThisCtx->externalItbAlloc = pSettings->ItbBlock!=NULL;

        /* TODO: remove the feature. warning only for now */
        if (!ThisCtx->externalCdbAlloc || (!ThisCtx->externalItbAlloc && pSettings->BufferCfg.Itb.Length) ) {
            BDBG_WRN(("internal RAVE memory allocation is deprecated"));
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        ThisCtx->mma.cdbBlock = pSettings->CdbBlock;
        ThisCtx->mma.itbBlock = pSettings->ItbBlock;
        ThisCtx->mma.cdbBlockOffset = pSettings->CdbBlock ? pSettings->CdbBlockOffset : 0;
        ThisCtx->mma.itbBlockOffset = pSettings->ItbBlock ? pSettings->ItbBlockOffset : 0;

        return AllocContext_Priv( hRave, pSettings->RequestedType, &(pSettings->BufferCfg), Index, Context );
    }

error:
    BXPT_P_ReleaseSubmodule(hRave->lvXpt, BXPT_P_Submodule_eRave);
    return rc;
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Rave_AllocContext(
    BXPT_Rave_Handle hRave,             /* [in] Handle for this RAVE channel */
    BXPT_RaveCx RequestedType,          /* [in] The type of context to allcoate */
    const BAVC_CdbItbConfig *BufferCfg,       /* [in] Size and alignment for ITB and CDB */
    BXPT_RaveCx_Handle *Context     /* [out] The allocated context */
    )
{
    /* deprecated due to MMA conversion */
    BSTD_UNUSED(hRave);
    BSTD_UNUSED(RequestedType);
    BSTD_UNUSED(BufferCfg);
    BSTD_UNUSED(Context);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif

BERR_Code AllocContext_Priv(
    BXPT_Rave_Handle hRave,             /* [in] Handle for this RAVE channel */
    BXPT_RaveCx RequestedType,          /* [in] The type of context to allcoate */
    const BAVC_CdbItbConfig *BufferCfg,       /* [in] Size and alignment for ITB and CDB */
    unsigned Index,
    BXPT_RaveCx_Handle *Context     /* [out] The allocated context */
    )
{
    uint32_t Reg;

    unsigned ScdMapMode = 0;
    BXPT_RaveCx_Handle ThisCtx = NULL;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_MSG(( "AllocContext_Priv: context %u (%s)", Index,
        RequestedType == BXPT_RaveCx_eAv || RequestedType == BXPT_RaveCx_eAvR ? "Decode" :
            RequestedType == BXPT_RaveCx_eRecord || RequestedType == BXPT_RaveCx_eRecordR ? "Record" :
                RequestedType == BXPT_RaveCx_eIp ? "IP" :
                    RequestedType == BXPT_RaveCx_eVctNull ? "VctNull" : "UNKNOWN"
        ));

    /* Check for invalid buffer alignment. */
    if( BufferCfg->Cdb.Alignment < RAVE_BUFFER_ALIGNMENT )
    {
        BDBG_ERR(( "Invalid CDB alignment requested!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto done;
    }
    if( BufferCfg->Cdb.Length > GetMaxBufferByGranularity( hRave ) )
    {
        /* The Upper and Lower Thresholds for both CDB and ITB are specified units of X number of bytes. This is
        the 'granularity', which is set by the ThresholdGranularityInBytes parameter. The threshold values
        themselves are only 16 bits wide, so that we have to limit the buffer size to have a meaningful threshold. */
        BDBG_ERR(( "CDB length can't be supported by ThresholdGranularityInBytes set at BXPT_Rave_OpenChannel" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto done;
    }

    if( BufferCfg->Cdb.Length < 4096 )
    {
        BDBG_ERR(( "CDB length must be at least 4096 bytes" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto done;
    }

    /* A zero-sized/zero-aligned ITB is allowed in some contexts. */
    if( BufferCfg->Itb.Alignment == 1 )
    {
        BDBG_ERR(( "Invalid ITB alignment requested!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto done;
    }

    ThisCtx = *(hRave->ContextTbl + Index);
    ThisCtx->Allocated = true;
    ThisCtx->Type = RequestedType;

    if( RequestedType == BXPT_RaveCx_eVctNull )
    {
        BXPT_RaveCx_Handle VctNeighbor = *(hRave->ContextTbl + Index + 1);

        VctNeighbor->Allocated = true;
        InitContext( VctNeighbor, NULL );
        ThisCtx->VctNeighbor = (void *) VctNeighbor;
    }
    else
    {
        ThisCtx->VctNeighbor = NULL;
    }

    if( BufferCfg->UsePictureCounter == true )
    {
        int PicCounter = GetNextPicCounter( ThisCtx );
        if( PicCounter < 0)
        {
            BDBG_ERR(( "No free picture counters!" ));
            ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
            goto done;
        }

        ThisCtx->PicCounter = PicCounter;
    }

    ThisCtx->HaveSpliceQueue = false;
    ThisCtx->SpliceQueueIdx = 0;
    ThisCtx->InputFormat = BAVC_StreamType_eTsMpeg;
    ThisCtx->ItbFormat = BAVC_ItbEsType_eAvcVideo;
    ExitCode = InitContext( ThisCtx, BufferCfg );
    if( ExitCode != BERR_SUCCESS )
    {
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        goto done;
    }
    ThisCtx->hAvScd = NULL;
    ThisCtx->hVctScd = NULL;

    /* By default, all SCD mapping should be done by PID channels */
    Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + REC_MISC_CFG_OFFSET );
    Reg &= ~BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE );
    if( RequestedType == BXPT_RaveCx_eIp )
        ScdMapMode = 2;
    Reg |= BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE, ScdMapMode );
    BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

    /* Restore the hw defaults, in case this context is being reused */
    Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_MISC_CFG1_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, BAND_HOLD_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, SHIFT_PTS ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, OUTPUT_FORMAT ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_HI ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_LO )
    );
    BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_MISC_CFG1_OFFSET, Reg );

    /* Each AV will need 1 SCD indexer, to build the ITB entries. */
    if( RequestedType == BXPT_RaveCx_eAv || RequestedType == BXPT_RaveCx_eAvR
    || RequestedType == BXPT_RaveCx_eIp || RequestedType == BXPT_RaveCx_eVctNull )
    {
        unsigned ScdNum;

        if( GetNextFreeScd( hRave, RequestedType, &ScdNum ) != BERR_SUCCESS )
        {
            BDBG_ERR(( "No SCD channel is available for this AV context" ));
            ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
            goto done;
        }

        hRave->ScdTable[ ScdNum ].Allocated = true;
        ThisCtx->hAvScd = hRave->ScdTable + ScdNum;
        InitScd( ThisCtx->hAvScd );

        /* For VCT processing, we need the next consecutive SCD as allocated. */
        if( RequestedType == BXPT_RaveCx_eVctNull )
        {
            hRave->ScdTable[ ScdNum + 1 ].Allocated = true;
            ThisCtx->hVctScd = hRave->ScdTable + ScdNum + 1;
            InitScd( ThisCtx->hVctScd );
        }

        /* Now map this SCD to the AV context */
        SetScdNum( ThisCtx, 0, ScdNum );
    }
    else
    {
        /* Some special stuff for record contexts */
        /*
        ** Set PES Sync for MPEG, our initial value for recording. This is used
        ** chiefly for startcode generation.
        ** MPEG -> PES_SYNC = 1
        ** DirecTV -> PES_SYNC = 3
        */
        Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_MISC_CFG1_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE, 1 )
        );
        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_MISC_CFG1_OFFSET, Reg );
    }

    /* For IP contexts, default to MPEG TS. */
    if( RequestedType == BXPT_RaveCx_eIp )
        ThisCtx->IsMpegTs = true;

    /* See SWSTB-2456. This solution is better than introducing another sequence dependency.
    ** 13 is used since it's the default for one of the video formats.
    */
    Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_MISC_CFG3_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, MAX_COMPARE_PATTERNS )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, MAX_COMPARE_PATTERNS, 13 )
    );
    BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_MISC_CFG3_OFFSET, Reg );

    *Context = ThisCtx;

    done:
    return( ExitCode );
}

BERR_Code BXPT_Rave_EnableContext(
    BXPT_RaveCx_Handle Context      /* [in] The context. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;
#ifdef BCHP_PWR_RESOURCE_XPT_RAVE
    unsigned wasEnabled;
#endif

    BDBG_ASSERT( Context );
    BDBG_MSG(( "BXPT_Rave_EnableContext %u", Context->Index ));

#ifdef BCHP_PWR_RESOURCE_XPT_RAVE
    Reg = BREG_Read32(Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET);
    wasEnabled = BCHP_GET_FIELD_DATA(Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE);

    /* only increment refcnt if not enabled already */
    if (!wasEnabled) {
        BCHP_PWR_AcquireResource(Context->hChp, BCHP_PWR_RESOURCE_XPT_RAVE);
    }
#endif

    /* Needs to be done BEFORE enabling the context. The bitfield name changed from 7278A0 to B0. */
#ifdef BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG2_PUSH_AND_RESET_WRMASK_MASK
    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG2_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, PUSH_AND_RESET_WRMASK )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, PUSH_AND_RESET_WRMASK, 1 )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG2_OFFSET, Reg );
#elif defined BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG2_RESET_BEFORE_START_MASK
    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG2_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RESET_BEFORE_START )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RESET_BEFORE_START, 1 )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG2_OFFSET, Reg );
#endif

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE, 1 )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET, Reg );

    return( ExitCode );
}

BERR_Code BXPT_Rave_DisableContext(
    BXPT_RaveCx_Handle Context      /* [in] The context. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;
#ifdef BCHP_PWR_RESOURCE_XPT_RAVE
    unsigned wasEnabled;
#endif

    BDBG_ASSERT( Context );
    BDBG_MSG(( "BXPT_Rave_DisableContext %u", Context->Index ));

#ifdef BCHP_PWR_RESOURCE_XPT_RAVE
    Reg = BREG_Read32(Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET);
    wasEnabled = BCHP_GET_FIELD_DATA(Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE);
#endif

#ifdef BCHP_XPT_RAVE_WRMASK_STOP_VALID_PTR_UPDATE_CX_31_0_WRMASK_STOP_VALID_PTR_UPDATE_CX_31_0_MASK
    if(Context->Index < 32)
        BREG_Write32(Context->hReg, BCHP_XPT_RAVE_WRMASK_STOP_VALID_PTR_UPDATE_CX_31_0, 1 << Context->Index);
    else
        BREG_Write32(Context->hReg, BCHP_XPT_RAVE_WRMASK_STOP_VALID_PTR_UPDATE_CX_47_32, 1 << (Context->Index - 32));
#endif

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET, Reg );

    /* Wait for 15uS for any pending DMA transfer to complete */
    BKNI_Delay( 15 );

    ClearCxHold(Context);

#ifdef BCHP_PWR_RESOURCE_XPT_RAVE
    /* only decrement refcnt if not disabled already */
    if (wasEnabled) {
        BCHP_PWR_ReleaseResource(Context->hChp, BCHP_PWR_RESOURCE_XPT_RAVE);
    }
#endif

    return( ExitCode );
}

BERR_Code BXPT_Rave_AllocIndexer(
    BXPT_Rave_Handle hRave,         /* [in] Handle for this RAVE channel */
    BXPT_RaveIdx IndexerType,       /* [in] Which type of indexer */
    unsigned NumIndexers,           /* [in] Number of indexers requested */
    BXPT_RaveCx_Handle Context,     /* [in] The record context that this indexer should be mapped to */
    BXPT_RaveIdx_Handle *Indexer    /* [out] Handle for the allocated indexer */
    )
{
    uint32_t Reg;
    uint64_t ItbBase;

    unsigned ChannelNum;
    unsigned Index;

    BXPT_RaveIdx_Handle lhIdx = NULL;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hRave );
    BDBG_ASSERT( Context );
    BDBG_ASSERT( Indexer );

    /* Make sure this context has an ITB allocated. In some cases, it won't */
    ItbBase = BREG_ReadAddr( Context->hReg, Context->BaseAddr + ITB_BASE_PTR_OFFSET );
    if( !ItbBase )
    {
        BDBG_ERR(( "No ITB allocated for this context." ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    for( Index = 0; Index < BXPT_NUM_SCD + BXPT_NUM_TPIT; Index++ )
        if( hRave->IndexerHandles[ Index ].Allocated == false )
            break;

    if( Index == BXPT_NUM_SCD + BXPT_NUM_TPIT )
    {
        BDBG_ERR(( "No Indexer handles are available for allocation." ));
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        goto Done;
    }

    lhIdx = hRave->IndexerHandles + Index;
    lhIdx->Allocated = true;
    lhIdx->NumChannels = 0;

    if( IndexerType == BXPT_RaveIdx_eTpit )
    {
        TpitIndexer *lhTpit;

        for( ChannelNum = 0; ChannelNum < BXPT_NUM_TPIT; ChannelNum++ )
            if( hRave->TpitTable[ ChannelNum ].Allocated == false )
                break;

        if( ChannelNum == BXPT_NUM_TPIT )
        {
            BDBG_ERR(( "No TPIT channels are available for allocation." ));
            ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
            goto Done;
        }

        /* We got one, so init it and mark is allocated */
        lhTpit = hRave->TpitTable + ChannelNum;
        InitTpit( lhTpit );
        lhTpit->Allocated = true;

        Reg = BREG_Read32( Context->hReg, Context->BaseAddr + REC_MISC_CFG_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_ENABLE ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_CHANNEL )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_ENABLE, 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_CHANNEL, ChannelNum )
        );
        BREG_Write32( Context->hReg, Context->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

        lhIdx->ChannelType = BXPT_RaveIdx_eTpit;
        lhIdx->hChannel = ( void * ) lhTpit;
        lhIdx->vhCtx = Context;
        lhIdx->NumChannels = 1;
    }
    else
    {
        unsigned ii;
        StartCodeIndexer **ScdPtr;

        ScdPtr = BKNI_Malloc( NumIndexers * sizeof( StartCodeIndexer *));
        if( !ScdPtr )
        {
            BDBG_ERR(( "No memory for indexer array is available" ));
            ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
            lhIdx->Allocated = false;
            goto Done;
        }
        lhIdx->hChannel = ( void * ) ScdPtr;   /* Point to the head of the list */

        for( ii = 0; ii < NumIndexers; ii++ )
        {
            unsigned ScdNum;

            if( GetNextFreeScd( hRave, Context->Type, &ScdNum ) != BERR_SUCCESS )
            {
                BDBG_ERR(( "No memory for indexer array is available" ));

                /* Need to give FreeScds() the head of the list */
                FreeScds( ii, lhIdx->hChannel );
                BKNI_Free( lhIdx->hChannel );

                ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
                lhIdx->Allocated = false;
                goto Done;
            }

            *ScdPtr = hRave->ScdTable + ScdNum;
            InitScd( *ScdPtr );
            ScdPtr++;
            hRave->ScdTable[ ScdNum ].Allocated = true;
        }

        lhIdx->ChannelType = BXPT_RaveIdx_eScd;
        lhIdx->vhCtx = Context;
        lhIdx->NumChannels = NumIndexers;
        lhIdx->SvcMvcMode = false;
    }

    Done:
    *Indexer = lhIdx;
    return( ExitCode );
}

BERR_Code BXPT_Rave_FreeContext(
    BXPT_RaveCx_Handle Context      /* [in] The context to free. */
    )
{
    unsigned Index;
    BERR_Code ExitCode = BERR_SUCCESS;
    BXPT_Rave_Handle hRave;

    BDBG_ASSERT( Context );

    Index = Context->Index;
    hRave = (BXPT_Rave_Handle) Context->vhRave;
    if( Context->IsSoftContext )
    {
        /* BXPT_RaveSoftMode_ePointersOnly does not allocate any buffers. All other modes do. */
        if( Context->SoftRave.mode != BXPT_RaveSoftMode_ePointersOnly )
        {
            BMMA_Unlock(Context->mma.itbBlock, Context->mma.itbPtr);
            BMMA_UnlockOffset(Context->mma.itbBlock, Context->mma.itbOffset);
            if (!Context->externalItbAlloc) {
                BMMA_Free(Context->mma.itbBlock);
            }
        }
    }
    else
    {
        /* Make sure it's off before we release the buffers */
        BXPT_Rave_DisableContext( Context );

        BMMA_Unlock(Context->mma.cdbBlock, Context->mma.cdbPtr);
        BMMA_UnlockOffset(Context->mma.cdbBlock, Context->mma.cdbOffset);
        if (!Context->externalCdbAlloc) {
            BMMA_Free(Context->mma.cdbBlock);
        }

        if (Context->mma.itbBlock) { /* ITB is optional */
            if (Context->mma.itbOffset) {
                BMMA_Unlock(Context->mma.itbBlock, Context->mma.itbPtr);
                BMMA_UnlockOffset(Context->mma.itbBlock, Context->mma.itbOffset);
            }
            if (!Context->externalItbAlloc) {
                BMMA_Free(Context->mma.itbBlock);
            }
        }

        /* free SCD channel */
        if(Context->hAvScd)
        {
            Context->hAvScd->Allocated = false;
        }

        if(Context->hVctScd)
        {
            Context->hVctScd->Allocated = false;
        }

        if( Context->VctNeighbor )
        {
            BXPT_RaveCx_Handle VctNeighbor = ( BXPT_RaveCx_Handle ) Context->VctNeighbor;
            InitContext( VctNeighbor, NULL );
            VctNeighbor->Allocated = false;
        }

        FreePicCounter( Context );
    }

    /* Zero out the registers */
    InitContext( Context, NULL );
    BXPT_P_ReleaseSubmodule(((BXPT_Rave_Handle)(Context->vhRave))->lvXpt, BXPT_P_Submodule_eRave);

    BKNI_Free((void *) Context);
    hRave->ContextTbl[Index] = NULL;
    return( ExitCode );
}

BERR_Code BXPT_Rave_FreeIndexer(
    BXPT_RaveIdx_Handle Indexer     /* [in] The indexer to free. */
    )
{
    uint32_t Reg;
    BXPT_RaveCx_Handle hCtx;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Indexer );
    hCtx = Indexer->vhCtx;

    if( Indexer->ChannelType == BXPT_RaveIdx_eTpit )
    {
        TpitIndexer *lhTpit = Indexer->hChannel;

        /* Disconnect it from the Record channel. */
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_ENABLE ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_CHANNEL )
        );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

        InitTpit( lhTpit );             /* Reset it first. */
        lhTpit->Allocated = false;      /* Free it */
    }
    else
    {
        StartCodeIndexer **lhScd = Indexer->hChannel;
        unsigned ii;

        ChangeScdPid( hCtx, 0, 0, 0x1FFF, false );
        ChangeScdPid( hCtx, 1, 0, 0x1FFF, false );
        ChangeScdPid( hCtx, 2, 0, 0x1FFF, false );
        ChangeScdPid( hCtx, 3, 0, 0x1FFF, false );
        ChangeScdPid( hCtx, 4, 0, 0x1FFF, false );
        ChangeScdPid( hCtx, 5, 0, 0x1FFF, false );
        ChangeScdPid( hCtx, 6, 0, 0x1FFF, false );
        ChangeScdPid( hCtx, 7, 0, 0x1FFF, false );

        for( ii = 0; ii < Indexer->NumChannels; ii++ )
        {
            InitScd( *(lhScd + ii) );
        }
        FreeScds( Indexer->NumChannels, lhScd );

        BKNI_Free( Indexer->hChannel );
    }

    Indexer->Allocated = false;

    return( ExitCode );
}

BERR_Code BXPT_Rave_GetIndexerConfig(
    BXPT_RaveIdx_Handle Indexer,        /* [in] Handle for the allocated indexer */
    BXPT_Rave_IndexerSettings *Config   /* [out] The indexer settings */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Indexer );
    BDBG_ASSERT( Config );

    if( Indexer->ChannelType == BXPT_RaveIdx_eTpit )
    {
        TpitIndexer *hTpit = ( TpitIndexer * ) Indexer->hChannel;

        Reg = BREG_Read32( hTpit->hReg, hTpit->BaseAddr + TPIT_CTRL1_OFFSET );
        Config->Cfg.Tpit.TimeTickEn = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_TPIT0_CTRL1, TPIT_TIME_TICK_EN );
        Config->Cfg.Tpit.StorePcrMsb = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_TPIT0_CTRL1, TPIT_PCR_MODE );
        Config->Cfg.Tpit.FirstPacketEnable = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_TPIT0_CTRL1, TPIT_FIRST_PACKET_EN );
        Config->Cfg.Tpit.IdleEventEnable = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_TPIT0_CTRL1, TPIT_EVENT_IDLE_EN ) ? true : false;
        Config->Cfg.Tpit.RecordEventEnable = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_TPIT0_CTRL1, TPIT_RECORD_IDLE_EN ) ? true : false;

        Reg = BREG_Read32( hTpit->hReg, hTpit->BaseAddr + TPIT_COR1_OFFSET );
        Config->Cfg.Tpit.CorruptionByte = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_TPIT0_COR1, REC_CORRUPT_BYTE );
        Config->Cfg.Tpit.CorruptionStart = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_TPIT0_COR1, REC_CORRUPT_START );
        Config->Cfg.Tpit.CorruptionEnd = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_TPIT0_COR1, REC_CORRUPT_END );

    }
    else
    {
        BXPT_RaveCx_Handle hCtx = ( BXPT_RaveCx_Handle ) Indexer->vhCtx;
        unsigned Mode;

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_CTRL1_OFFSET );
        Config->Cfg.Scd.Atsc01IsUnscrambled = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_CTRL1, ATSC_SCRAM_CTRL ) ? true : false;
        Config->Cfg.Scd.ParseScramblingControl = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_CTRL1, PARSE_SC ) ? true : false;

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET );
        Config->Cfg.Scd.ScRange[ 3 ].RangeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_D, COMP1_RANGED_HI );
        Config->Cfg.Scd.ScRange[ 2 ].RangeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_C, COMP1_RANGEC_HI );
        Config->Cfg.Scd.ScRange[ 1 ].RangeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_B, COMP1_RANGEB_HI );
        Config->Cfg.Scd.ScRange[ 0 ].RangeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_A, COMP1_RANGEA_HI );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET );
        Config->Cfg.Scd.ScRange[ 3 ].RangeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_D, COMP1_RANGED_LO );
        Config->Cfg.Scd.ScRange[ 2 ].RangeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_C, COMP1_RANGEC_LO );
        Config->Cfg.Scd.ScRange[ 1 ].RangeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_B, COMP1_RANGEB_LO );
        Config->Cfg.Scd.ScRange[ 0 ].RangeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_A, COMP1_RANGEA_LO );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
        Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_D );
        GetScEnables_Indexer( &Config->Cfg.Scd.ScRange[ 3 ], Mode );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
        Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_C );
        GetScEnables_Indexer( &Config->Cfg.Scd.ScRange[ 2 ], Mode );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
        Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_B );
        GetScEnables_Indexer( &Config->Cfg.Scd.ScRange[ 1 ], Mode );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
        Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_A );
        GetScEnables_Indexer( &Config->Cfg.Scd.ScRange[ 0 ], Mode );

        Config->Cfg.Scd.SvcMvcMode = Indexer->SvcMvcMode;
    }

    return( ExitCode );
}

BERR_Code BXPT_Rave_SetIndexerConfig(
    BXPT_RaveIdx_Handle Indexer,            /* [in] Handle for the allocated indexer */
    const BXPT_Rave_IndexerSettings *Config /* [in] The indexer settings */
    )
{
    uint32_t Reg;
    int pesSyncMode;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Indexer );
    BDBG_ASSERT( Config );

    if( Indexer->ChannelType == BXPT_RaveIdx_eTpit )
    {
        TpitIndexer *hTpit = ( TpitIndexer * ) Indexer->hChannel;

        Reg = BREG_Read32( hTpit->hReg, hTpit->BaseAddr + TPIT_CTRL1_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_TPIT0_CTRL1, TPIT_PCR_MODE ) |
            BCHP_MASK( XPT_RAVE_TPIT0_CTRL1, TPIT_FIRST_PACKET_EN ) |
            BCHP_MASK( XPT_RAVE_TPIT0_CTRL1, TPIT_EVENT_IDLE_EN ) |
            BCHP_MASK( XPT_RAVE_TPIT0_CTRL1, TPIT_RECORD_IDLE_EN )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_TPIT0_CTRL1, TPIT_PCR_MODE, Config->Cfg.Tpit.StorePcrMsb ) |
            BCHP_FIELD_DATA( XPT_RAVE_TPIT0_CTRL1, TPIT_FIRST_PACKET_EN, Config->Cfg.Tpit.FirstPacketEnable ) |
            BCHP_FIELD_DATA( XPT_RAVE_TPIT0_CTRL1, TPIT_EVENT_IDLE_EN, Config->Cfg.Tpit.IdleEventEnable ) |
            BCHP_FIELD_DATA( XPT_RAVE_TPIT0_CTRL1, TPIT_RECORD_IDLE_EN, Config->Cfg.Tpit.RecordEventEnable )
        );
        BREG_Write32( hTpit->hReg, hTpit->BaseAddr + TPIT_CTRL1_OFFSET, Reg );

        Reg = BREG_Read32( hTpit->hReg, hTpit->BaseAddr + TPIT_COR1_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_TPIT0_COR1, REC_CORRUPT_BYTE ) |
            BCHP_MASK( XPT_RAVE_TPIT0_COR1, REC_CORRUPT_START ) |
            BCHP_MASK( XPT_RAVE_TPIT0_COR1, REC_CORRUPT_END )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_TPIT0_COR1, REC_CORRUPT_BYTE, Config->Cfg.Tpit.CorruptionByte ) |
            BCHP_FIELD_DATA( XPT_RAVE_TPIT0_COR1, REC_CORRUPT_START, Config->Cfg.Tpit.CorruptionStart ) |
            BCHP_FIELD_DATA( XPT_RAVE_TPIT0_COR1, REC_CORRUPT_END, Config->Cfg.Tpit.CorruptionEnd )
        );
        BREG_Write32( hTpit->hReg, hTpit->BaseAddr + TPIT_COR1_OFFSET, Reg );
    }
    else
    {
        BXPT_RaveCx_Handle hCtx = ( BXPT_RaveCx_Handle ) Indexer->vhCtx;

        Indexer->SvcMvcMode = Config->Cfg.Scd.SvcMvcMode;
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_CTRL1_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_REC_CTRL1, ATSC_SCRAM_CTRL ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_CTRL1, PARSE_SC )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_CTRL1, ATSC_SCRAM_CTRL, Config->Cfg.Scd.Atsc01IsUnscrambled == true ? 1 : 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_CTRL1, PARSE_SC, Config->Cfg.Scd.ParseScramblingControl == true ? 1 : 0 )
        );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_CTRL1_OFFSET, Reg );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG1_OFFSET );
        pesSyncMode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE );

        /* Comp1 used for ES startcode */
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_CTRL_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_START_BIT ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_START_BYTE ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMP_ENABLE ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, REPEAT_BYTE ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, NUM_COMPARE_BYTES ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, VALID_BYTE_ENABLE ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, CASCADE_ENABLE ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ALL_DATA ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ADAPTATION_FIELD )|
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_PES_HDR_DATA ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ES_DATA ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, ALIGNMENT_EN )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_START_BIT, 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_START_BYTE, 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMP_ENABLE, 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, REPEAT_BYTE, 4 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, NUM_COMPARE_BYTES, 2 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, VALID_BYTE_ENABLE, 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, CASCADE_ENABLE, 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ALL_DATA, 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ADAPTATION_FIELD, 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ES_DATA, 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, ALIGNMENT_EN, 1 )
            );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_CTRL_OFFSET, Reg );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_VAL_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_0 ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_1 ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_2 ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_3 )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_0, 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_1, 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_2, 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_3, 0 )
            );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_VAL_OFFSET, Reg );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_VAL_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_0 ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_1 ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_2 ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_3 )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_0, 0xff ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_1, 0xff ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_2, 0xff ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_3, 0x00 )
            );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_VAL_OFFSET, Reg );


        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_D, COMP1_RANGED_HI ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_C, COMP1_RANGEC_HI ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_B, COMP1_RANGEB_HI ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_A, COMP1_RANGEA_HI )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_D, COMP1_RANGED_HI, Config->Cfg.Scd.ScRange[ 3 ].RangeHi ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_C, COMP1_RANGEC_HI, Config->Cfg.Scd.ScRange[ 2 ].RangeHi ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_B, COMP1_RANGEB_HI, Config->Cfg.Scd.ScRange[ 1 ].RangeHi ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_A, COMP1_RANGEA_HI, Config->Cfg.Scd.ScRange[ 0 ].RangeHi )
        );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, Reg );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_D, COMP1_RANGED_LO ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_C, COMP1_RANGEC_LO ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_B, COMP1_RANGEB_LO ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_A, COMP1_RANGEA_LO )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_D, COMP1_RANGED_LO, Config->Cfg.Scd.ScRange[ 3 ].RangeLo ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_C, COMP1_RANGEC_LO, Config->Cfg.Scd.ScRange[ 2 ].RangeLo ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_B, COMP1_RANGEB_LO, Config->Cfg.Scd.ScRange[ 1 ].RangeLo ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_A, COMP1_RANGEA_LO, Config->Cfg.Scd.ScRange[ 0 ].RangeLo )
        );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, Reg );

        /*comparator 2*/
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_CTRL_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_START_BIT ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_START_BYTE ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMP_ENABLE ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, REPEAT_BYTE ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, NUM_COMPARE_BYTES ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, VALID_BYTE_ENABLE ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_ALL_DATA ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_ADAPTATION_FIELD )|
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_PES_HDR_DATA ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_ES_DATA ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, ALIGNMENT_EN )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_START_BIT, 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_START_BYTE, 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, REPEAT_BYTE, 4 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, NUM_COMPARE_BYTES, 2 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, VALID_BYTE_ENABLE, 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_ALL_DATA, 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_ADAPTATION_FIELD, 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_ES_DATA, 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, ALIGNMENT_EN, 1 )
            );

        /* for mpeg recording comp is used for es */
        if(pesSyncMode == 1 || Config->Cfg.Scd.SvcMvcMode)
        {
            Reg |= (
                   BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMP_ENABLE, 1 ) |
                   BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_PES_HDR_DATA, 1 )
                   );
        }
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_CTRL_OFFSET, Reg );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_VAL_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_COMPARE_VAL, COMP2_COMPARE_VAL_0 ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_COMPARE_VAL, COMP2_COMPARE_VAL_1 ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_COMPARE_VAL, COMP2_COMPARE_VAL_2 ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_COMPARE_VAL, COMP2_COMPARE_VAL_3 )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_COMPARE_VAL, COMP2_COMPARE_VAL_0, 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_COMPARE_VAL, COMP2_COMPARE_VAL_1, 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_COMPARE_VAL, COMP2_COMPARE_VAL_2, 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_COMPARE_VAL, COMP2_COMPARE_VAL_3, 0 )
            );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_VAL_OFFSET, Reg );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_VAL_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_MASK_VAL, COMP2_MASK_VAL_0 ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_MASK_VAL, COMP2_MASK_VAL_1 ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_MASK_VAL, COMP2_MASK_VAL_2 ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_MASK_VAL, COMP2_MASK_VAL_3 )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_MASK_VAL, COMP2_MASK_VAL_0, 0xff ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_MASK_VAL, COMP2_MASK_VAL_1, 0xff ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_MASK_VAL, COMP2_MASK_VAL_2, 0xff ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_MASK_VAL, COMP2_MASK_VAL_3, 0x00 )
            );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_VAL_OFFSET, Reg );


        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_D ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_D ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_C ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_C ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_B ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_B ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_A ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_A )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_D, Config->Cfg.Scd.ScRange[ 3 ].RangeEnable ? ( Config->Cfg.Scd.ScRange[ 3 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_D, 3 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_C, Config->Cfg.Scd.ScRange[ 2 ].RangeEnable ? ( Config->Cfg.Scd.ScRange[ 2 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_C, 3 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_B, Config->Cfg.Scd.ScRange[ 1 ].RangeEnable ? ( Config->Cfg.Scd.ScRange[ 1 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_B, 3 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_A, Config->Cfg.Scd.ScRange[ 0 ].RangeEnable ? ( Config->Cfg.Scd.ScRange[ 0 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_A, 3 )
        );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET, Reg );

        /* SW7422-115. We don't have updated RDB yet */
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG5_OFFSET );
        Reg &= ~( 1 << 30 );
        Reg |= ( Config->Cfg.Scd. SvcMvcMode ? (1 << 30) : 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG5_OFFSET, Reg );
    }

    return( ExitCode );
}


#ifdef BXPT_VCT_SUPPORT

BERR_Code BXPT_Rave_NullifyVCT(
    BXPT_RaveCx_Handle Context,            /*[in] Context Handle */
    bool ProcessPid,                       /*[in] True(Process PID 0x1ffb) */
    BXPT_RaveVct TableId                   /*[in] Table ID T-VCT C-VCT or non */
    )
{
    uint32_t Reg;
    uint32_t Table_Id;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Context );

    switch ( TableId ) {
    case BXPT_RaveVct_Tvct:
        Table_Id = 0xC8;
        break;
    case BXPT_RaveVct_Cvct:
        Table_Id = 0xC9;
        break;
    default:
        Table_Id = 0;
        break;
    }

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + REC_MISC_CFG5_OFFSET );

    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG5, VCT_PID_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG5, VCT_TABLE_ID )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG5, VCT_PID_EN, ProcessPid ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG5, VCT_TABLE_ID, Table_Id )
    );

    BREG_Write32( Context->hReg, Context->BaseAddr + REC_MISC_CFG5_OFFSET, Reg );

    return ( ExitCode );
}

#endif

BERR_Code BXPT_Rave_SetScdEntry(
    BXPT_RaveIdx_Handle Indexer,            /* [in] Handle for the allocated indexer */
    unsigned WhichScd,                      /* [in] Which of the startcode detectors. */
    const BXPT_Rave_ScdEntry *ScdConfig     /* [in] The indexer settings */
    )
{
    uint32_t Reg;
    BXPT_RaveCx_Handle hCtx;
    StartCodeIndexer *hScd;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Indexer );
    BDBG_ASSERT( ScdConfig );

    hCtx = Indexer->vhCtx;
    if( WhichScd >= Indexer->NumChannels)
    {
        BDBG_ERR(( "Indexer only has %u SCDs mapped. WhichScd %u is out of range!", Indexer->NumChannels, WhichScd ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }
    hScd = *(( StartCodeIndexer ** ) Indexer->hChannel + WhichScd);

    /* By default, all SCD mapping should be done by PID channels */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET );
    Reg &= ~BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

    ChangeScdPid( hCtx, WhichScd, hScd->ChannelNo, ScdConfig->PidChannel, true );

    ExitCode = SetExtractionBitCounts( hCtx, hScd, ScdConfig, WhichScd, Indexer->SvcMvcMode );

    Done:
    return( ExitCode );
}

BERR_Code BXPT_Rave_SetScdUsingPid(
    BXPT_RaveIdx_Handle Indexer,            /* [in] Handle for the allocated indexer */
    unsigned WhichScd,                      /* [in] Which of the startcode detectors. */
    unsigned Pid,                           /* [in] Which PID startcodes will be built for */
    const BXPT_Rave_ScdEntry *ScdConfig     /* [in] The indexer settings */
    )
{
    uint32_t Reg, RegAddr;
    BXPT_RaveCx_Handle hCtx;
    StartCodeIndexer *hScd;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Indexer );
    BDBG_ASSERT( ScdConfig );

    hCtx = Indexer->vhCtx;
    if( WhichScd >= Indexer->NumChannels)
    {
        BDBG_ERR(( "Indexer only has %u SCDs mapped. WhichScd %u is out of range!", Indexer->NumChannels, WhichScd ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }
    hScd = *(( StartCodeIndexer ** ) Indexer->hChannel + WhichScd);

    /* Change the map mode to use the PID rather than the PID channel */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET );
    Reg &= ~BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE );
    Reg |= BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE, 1 );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

#ifdef BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_AB
    Reg = (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_stream_PID_values_SCD_NUMA, hScd->ChannelNo ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_stream_PID_values_PIDA_VALID, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_stream_PID_values_SCD_PIDA, Pid )
    );

    switch( WhichScd )
    {
        case 0:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET;
        break;

        case 1:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET;
        break;

        case 2:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET;
        break;

        case 3:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET;
        break;

        default:
        BDBG_ERR(( "WhichScd %u not valid!", WhichScd ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    BREG_Write32( hCtx->hReg, RegAddr, Reg );
#else
    Reg = (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_stream_PID_values_SCD_NUMA, hScd->ChannelNo ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_stream_PID_values_PIDA_VALID, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_stream_PID_values_SCD_PIDA, Pid )
    );

    switch( WhichScd )
    {
        case 0:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_A_OFFSET;
        break;

        case 1:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_B_OFFSET;
        break;

        case 2:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_C_OFFSET;
        break;

        case 3:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_D_OFFSET;
        break;

        case 4:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_E_OFFSET;
        break;

        case 5:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_F_OFFSET;
        break;

        case 6:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_G_OFFSET;
        break;

        case 7:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_H_OFFSET;
        break;

        default:
        BDBG_ERR(( "Invalid WhichScd: %u", WhichScd ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    BREG_Write32( hCtx->hReg, RegAddr, Reg );
#endif

    ExitCode = SetExtractionBitCounts( hCtx, hScd, ScdConfig, WhichScd, Indexer->SvcMvcMode );

    Done:
    return( ExitCode );
}

BERR_Code SetExtractionBitCounts(
    BXPT_RaveCx_Handle hCtx,
    StartCodeIndexer *hScd,
    const BXPT_Rave_ScdEntry *ScdConfig,     /* [in] The indexer settings */
    unsigned WhichScd,
    bool SvcMvcMode
    )
{
    uint32_t Reg, RegAddr;
#if (!BXPT_HAS_16BYTE_ES_COUNT)
    #undef BXPT_MAX_ES_COUNT
    #define BXPT_MAX_ES_COUNT 9
#endif

    BERR_Code ExitCode = BERR_SUCCESS;
    uint32_t EsCount = 0;

    if( SvcMvcMode )
    {
        EsCount = 71;
    }
    else
    {
        if( ScdConfig->EsCount > BXPT_MAX_ES_COUNT || ScdConfig->EsCount < BXPT_MIN_ES_COUNT ) {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        EsCount = ScdConfig->EsCount;

        /* Crazy bit mapping in the hardware. See the RDB entry for XPT_RAVE_SCD0_SCD_CTRL1.DATA_EXTRACT_NUM_BITS_B */
        EsCount = ( EsCount * 8 ) - 1;
    }

    BSTD_UNUSED( hScd );

    /* Comparator 1 detects startcodes for PES and ES */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_NUM_BITS )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_NUM_BITS, EsCount )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_CTRL_OFFSET, Reg );

    /* Comparator 2 is used for PES only */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_NUM_BITS )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_NUM_BITS, 23 )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_CTRL_OFFSET, Reg );

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG5_OFFSET );
    if( ScdConfig->ExtractDts )
        Reg |= ( 1 << 29 );
    else
        Reg &= ~( 1 << 29 );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG5_OFFSET, Reg );

#ifdef BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_AB
    BSTD_UNUSED( RegAddr );
    switch( WhichScd )
    {
        case 0:
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET );
        Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_A ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_A, ScdConfig->ExtractPts == true ? 1 : 0 ) );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET, Reg );
        break;

        case 1:
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET );
        Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_B ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_B, ScdConfig->ExtractPts == true ? 1 : 0 ) );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET, Reg );
        break;

        case 2:
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET );
        Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_CD_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_C ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_CD_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_C, ScdConfig->ExtractPts == true ? 1 : 0 ) );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET, Reg );
        break;

        case 3:
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET );
        Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_CD_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_D ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_CD_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_D, ScdConfig->ExtractPts == true ? 1 : 0 ) );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET, Reg );
        break;

        case 4:
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET );
        Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_EF_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_E ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_EF_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_E, ScdConfig->ExtractPts == true ? 1 : 0 ) );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET, Reg );
        break;

        case 5:
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET );
        Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_EF_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_F ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_EF_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_F, ScdConfig->ExtractPts == true ? 1 : 0 ) );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET, Reg );
        break;

        case 6:
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET );
        Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_GH_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_G ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_GH_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_G, ScdConfig->ExtractPts == true ? 1 : 0 ) );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET, Reg );
        break;

        case 7:
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET );
        Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_GH_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_H ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_GH_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_H, ScdConfig->ExtractPts == true ? 1 : 0 ) );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET, Reg );
        break;
    }
#else
    switch( WhichScd )
    {
        case 0:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_A_OFFSET;
        break;

        case 1:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_B_OFFSET;
        break;

        case 2:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_C_OFFSET;
        break;

        case 3:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_D_OFFSET;
        break;

        case 4:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_E_OFFSET;
        break;

        case 5:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_F_OFFSET;
        break;

        case 6:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_G_OFFSET;
        break;

        case 7:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_H_OFFSET;
        break;

        default:
        BDBG_ERR(( "Invalid WhichScd: %u", WhichScd ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    Reg = BREG_Read32( hCtx->hReg, RegAddr );
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_A_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_A ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_A_Mapped_SCD_via_PID_channels, SCD_PTS_MODE_PIDCH_A, ScdConfig->ExtractPts == true ? 1 : 0 ) );
    BREG_Write32( hCtx->hReg, RegAddr, Reg );
    Done:
#endif

    return( ExitCode );
}

BERR_Code BXPT_Rave_SetTpitEcms(
    BXPT_RaveIdx_Handle Indexer,        /* [in] Handle for the allocated indexer */
    unsigned WhichPair,                 /* [in] ECM TID pair 0, 1, or 2 */
    unsigned EvenEcmTid,                /* [in] Even ECM TID */
    unsigned OddEcmTid                  /* [in] Odd ECM TID */
    )
{
    uint32_t Reg;
    TpitIndexer *hTpit;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Indexer );

    hTpit = ( TpitIndexer * ) Indexer->hChannel;
    switch( WhichPair )
    {
        case 1:
        Reg = BREG_Read32( hTpit->hReg, hTpit->BaseAddr + TPIT_TID_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_TPIT0_TID, ECM_TID_ODD )|
            BCHP_MASK( XPT_RAVE_TPIT0_TID, ECM_TID_EVEN )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_TPIT0_TID, ECM_TID_ODD, OddEcmTid ) |
            BCHP_FIELD_DATA( XPT_RAVE_TPIT0_TID, ECM_TID_EVEN, EvenEcmTid )
        );
        BREG_Write32( hTpit->hReg, hTpit->BaseAddr + TPIT_TID_OFFSET, Reg );
        break;

        case 2:
        Reg = BREG_Read32( hTpit->hReg, hTpit->BaseAddr + TPIT_TID2_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_TPIT0_TID2, ECM_TID_ODD2 )|
            BCHP_MASK( XPT_RAVE_TPIT0_TID2, ECM_TID_EVEN2 )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_TPIT0_TID2, ECM_TID_ODD2, OddEcmTid ) |
            BCHP_FIELD_DATA( XPT_RAVE_TPIT0_TID2, ECM_TID_EVEN2, EvenEcmTid )
        );
        BREG_Write32( hTpit->hReg, hTpit->BaseAddr + TPIT_TID2_OFFSET, Reg );
        break;

        case 3:
        Reg = BREG_Read32( hTpit->hReg, hTpit->BaseAddr + TPIT_TID2_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_TPIT0_TID2, ECM_TID_ODD3 )|
            BCHP_MASK( XPT_RAVE_TPIT0_TID2, ECM_TID_EVEN3 )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_TPIT0_TID2, ECM_TID_ODD3, OddEcmTid ) |
            BCHP_FIELD_DATA( XPT_RAVE_TPIT0_TID2, ECM_TID_EVEN3, EvenEcmTid )
        );
        BREG_Write32( hTpit->hReg, hTpit->BaseAddr + TPIT_TID2_OFFSET, Reg );
        break;

        default:
        /* Bad table entry number. Complain. */
        BDBG_ERR(( "WhichFilter %u is not valid (valid values are 1, 2, and 3)!", WhichPair ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        break;
    }

    return ExitCode;
}

BERR_Code BXPT_Rave_SetTpitFilter(
    BXPT_RaveIdx_Handle Indexer,            /* [in] Handle for the allocated indexer */
    unsigned WhichFilter,                   /* [in] Which of the 16 filters. */
    const BXPT_Rave_TpitEntry *Tpit         /* [in] The indexer settings */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Indexer );
    BDBG_ASSERT( Tpit );

    /*
    ** The PR 29691 workaround requires we not use TPIT PID Channel XPT_RAVE_TPIT<i>_PID_TABLE_0
    ** and corresponding PAR Table register XPT_RAVE_TPIT<i>_PAR_TABLE_0 (where i = 0 to 5).
    ** So, the define for BXPT_NUM_TPIT_PIDS is reduced from 16 to 15, and WhichFilter is
    ** remapped to 1 greater.
    */

    if( WhichFilter++ >= BXPT_NUM_TPIT_PIDS )
    {
        /* Bad table entry number. Complain. */
        BDBG_ERR(( "WhichFilter %u is out of range!", WhichFilter ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        TpitIndexer *lhTpit = ( TpitIndexer * ) Indexer->hChannel;

        if( Tpit->MpegMode == false )     /* We're handling DirecTV */
        {
            Reg = BREG_Read32( lhTpit->hReg, lhTpit->PidTableBaseAddr + ( WhichFilter * TPIT_PID_TABLE_ENTRY_STEP ) );
            Reg &= ~(
                BCHP_MASK( XPT_RAVE_TPIT0_PID_TABLEi, REC_PID ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PID_TABLEi, REC_PARSE_ENABLE ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PID_TABLEi, REC_CORRUPT_ENABLE )|
                BCHP_MASK( XPT_RAVE_TPIT0_PID_TABLEi, REC_HD )|
                BCHP_MASK( XPT_RAVE_TPIT0_PID_TABLEi, REC_HD_FILT_EN )

            );
            Reg |= (
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PID_TABLEi, REC_PID, Tpit->Pid ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PID_TABLEi, REC_PARSE_ENABLE, 1 ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PID_TABLEi, REC_CORRUPT_ENABLE, Tpit->CorruptionEnable == true ? 1 : 0 ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PID_TABLEi, REC_HD, Tpit->Cfg.DirecTv.HdFilter ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PID_TABLEi, REC_HD_FILT_EN, Tpit->Cfg.DirecTv.FilterHdEnable )
            );
            BREG_Write32( lhTpit->hReg, lhTpit->PidTableBaseAddr + ( WhichFilter * TPIT_PID_TABLE_ENTRY_STEP ), Reg );

            Reg = BREG_Read32( lhTpit->hReg, lhTpit->ParseTableBaseAddr + ( WhichFilter * TPIT_PAR_TABLE_ENTRY_STEP ) );

            /* Clear all the bits we are about to change. */
            Reg &= ~(
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_TC_DET_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_CWP_DET_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_RTS_DET_EN ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_CFF_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_CFF_COMP ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_MF_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_MF_COMP ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_HD_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_HD_MASK ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_CSAUX_CHANGE_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_CS_CHANGE_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_CF_CHANGE_EN ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_BB_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_BB_COMP )
            );

            Reg |= (
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_TC_DET_EN, Tpit->Cfg.DirecTv.TcDetEn ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_CWP_DET_EN, Tpit->Cfg.DirecTv.CwpDetEn ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_RTS_DET_EN, Tpit->Cfg.DirecTv.RtsDetEn ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_CFF_EN, Tpit->Cfg.DirecTv.CffEn ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_CFF_COMP, Tpit->Cfg.DirecTv.CffComp ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_MF_EN, Tpit->Cfg.DirecTv.MfEn ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_MF_COMP, Tpit->Cfg.DirecTv.MfComp ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_HD_EN, Tpit->Cfg.DirecTv.HdEn ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_HD_MASK, Tpit->Cfg.DirecTv.HdMask ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_CSAUX_CHANGE_EN, Tpit->Cfg.DirecTv.CsAuxChangeEn ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_CS_CHANGE_EN, Tpit->Cfg.DirecTv.CsChangeEn ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_CF_CHANGE_EN, Tpit->Cfg.DirecTv.CfChangeEn ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_BB_EN, Tpit->Cfg.DirecTv.BbEn ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_DIRECTV, RP_BB_COMP, Tpit->Cfg.DirecTv.BbComp )

            );

            BREG_Write32( lhTpit->hReg, lhTpit->ParseTableBaseAddr + ( WhichFilter * TPIT_PAR_TABLE_ENTRY_STEP ), Reg );

        }
        else /* mpeg streams */
        {
            Reg = BREG_Read32( lhTpit->hReg, lhTpit->PidTableBaseAddr + ( WhichFilter * TPIT_PID_TABLE_ENTRY_STEP ) );
            Reg &= ~(
                BCHP_MASK( XPT_RAVE_TPIT0_PID_TABLEi, REC_PID ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PID_TABLEi, REC_PARSE_ENABLE ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PID_TABLEi, REC_CORRUPT_ENABLE )
            );
            Reg |= (
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PID_TABLEi, REC_PID, Tpit->Pid ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PID_TABLEi, REC_PARSE_ENABLE, 1 ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PID_TABLEi, REC_CORRUPT_ENABLE, Tpit->CorruptionEnable == true ? 1 : 0 )
            );
            BREG_Write32( lhTpit->hReg, lhTpit->PidTableBaseAddr + ( WhichFilter * TPIT_PID_TABLE_ENTRY_STEP ), Reg );

            Reg = BREG_Read32( lhTpit->hReg, lhTpit->ParseTableBaseAddr + ( WhichFilter * TPIT_PAR_TABLE_ENTRY_STEP ) );

            /* Clear all the bits we are about to change. */
            Reg &= ~(
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_ECM_POLARITY_CHANGE_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_SECTION_FILTER_EN ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_ADAPT_FIELD_EXT_FLAG_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_ADAPT_FIELD_EXT_FLAG_COMP ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_PRIVATE_DATA_FLAG_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_PRIVATE_DATA_FLAG_COMP ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_SPLICING_POINT_FLAG_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_SPLICING_POINT_FLAG_COMP ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_OPCR_FLAG_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_OPCR_FLAG_COMP ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_PCR_FLAG_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_PCR_FLAG_COMP ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_ES_PRIORITY_IND_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_ES_PRIORITY_IND_COMP ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_RANDOM_ACCESS_IND_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_RANDOM_ACCESS_IND_COMP ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_DISCONTINUITY_IND_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_DISCONTINUITY_IND_COMP ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_AFC_CHANGE_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_TSC_CHANGE_EN ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_TRANSPORT_PRIORITY_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_TRANSPORT_PRIORITY_COMP ) |

                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_PUSI_EN ) |
                BCHP_MASK( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_PUSI_COMP )
            );

            Reg |= (
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_ECM_POLARITY_CHANGE_EN, Tpit->Cfg.Mpeg.EcmPolarityChangeEn ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_SECTION_FILTER_EN, Tpit->Cfg.Mpeg.SectionFilterEn ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_ADAPT_FIELD_EXT_FLAG_EN, Tpit->Cfg.Mpeg.AdaptationExtensionFlagEnable ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_ADAPT_FIELD_EXT_FLAG_COMP, Tpit->Cfg.Mpeg.AdaptationExtensionFlagCompValue ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_PRIVATE_DATA_FLAG_EN, Tpit->Cfg.Mpeg.PrivateDataFlagEnable ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_PRIVATE_DATA_FLAG_COMP, Tpit->Cfg.Mpeg.PrivateDataFlagCompValue ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_SPLICING_POINT_FLAG_EN, Tpit->Cfg.Mpeg.SplicingPointFlagEnable ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_SPLICING_POINT_FLAG_COMP, Tpit->Cfg.Mpeg.SplicingPointFlagCompValue ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_OPCR_FLAG_EN, Tpit->Cfg.Mpeg.OpcrFlagEnable ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_OPCR_FLAG_COMP, Tpit->Cfg.Mpeg.OpcrFlagCompValue ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_PCR_FLAG_EN, Tpit->Cfg.Mpeg.PcrFlagEnable ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_PCR_FLAG_COMP, Tpit->Cfg.Mpeg.PcrFlagCompValue ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_ES_PRIORITY_IND_EN, Tpit->Cfg.Mpeg.EsPriorityIndicatorEnable ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_ES_PRIORITY_IND_COMP, Tpit->Cfg.Mpeg.EsPriorityIndicatorCompValue ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_RANDOM_ACCESS_IND_EN, Tpit->Cfg.Mpeg.RandomAccessIndicatorEnable ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_RANDOM_ACCESS_IND_COMP, Tpit->Cfg.Mpeg.RandomAccessIndicatorCompValue ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_DISCONTINUITY_IND_EN, Tpit->Cfg.Mpeg.DiscontinuityIndicatorEnable ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_DISCONTINUITY_IND_COMP, Tpit->Cfg.Mpeg.DiscontinuityIndicatorCompValue ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_AFC_CHANGE_EN, Tpit->Cfg.Mpeg.AdaptationFieldChangeEnable ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_TSC_CHANGE_EN, Tpit->Cfg.Mpeg.ScramblingControlChangeEnable ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_TRANSPORT_PRIORITY_EN, Tpit->Cfg.Mpeg.TransportPriorityEnable ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_TRANSPORT_PRIORITY_COMP, Tpit->Cfg.Mpeg.TransportPriorityCompValue ) |

                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_PUSI_EN, Tpit->Cfg.Mpeg.PayloadUnitStartEnable ) |
                BCHP_FIELD_DATA( XPT_RAVE_TPIT0_PAR_TABLEi_MPEG, RP_PUSI_COMP, Tpit->Cfg.Mpeg.PayloadUnitStartCompValue )
            );

            BREG_Write32( lhTpit->hReg, lhTpit->ParseTableBaseAddr + ( WhichFilter * TPIT_PAR_TABLE_ENTRY_STEP ), Reg );
        }
    }

    return( ExitCode );
}

BERR_Code BXPT_Rave_GetBufferInfo_isrsafe(
    BXPT_RaveCx_Handle hCtx,        /* [in] The context. */
    BXPT_Rave_BufferInfo *BufferInfo    /* [out] CDB depth and size */
    )
{
    /* New buffer depth calc. Use READ and VALID */
    uint64_t Read, Valid, Wrap, Base, End;

    size_t ByteCount = 0;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hCtx );
    BDBG_ASSERT( BufferInfo );

    Read = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_READ_PTR_OFFSET );
    Valid = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_VALID_PTR_OFFSET );
    Wrap = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_WRAP_PTR_OFFSET );
    Base = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET );
    End = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_END_PTR_OFFSET );

    BufferInfo->CdbSize = End - Base + 1;

    if( Read < Valid )
    {
        if( hCtx->CdbReset == true )
        {
            /*
            ** Checks after a context reset, but before the first update of the READ, are a special case.
            ** We need to do account for the fact that first burst of data written into the buffer
            ** started AT the BASE pointer, not VALID+1.
            */
            ByteCount = Valid - Read + 1;
        }
        else
        {
            ByteCount = Valid - Read;
        }
    }
    else if( Read > Valid )
    {
        /* It did wrap */
        if( Read == Wrap )
        {
            /* They read up to the wraparound point. New data starts at the base */
            ByteCount = Valid - Base + 1;
        }
        else
        {
            ByteCount = Wrap - Read;
            ByteCount += Valid - Base + 1;
        }
    }

    BufferInfo->CdbDepth = ByteCount;

    return( ExitCode );
}

BERR_Code BXPT_Rave_GetContextRegisters(
    BXPT_RaveCx_Handle Context,     /* [in] The context  */
    BAVC_XptContextMap *Map         /* [out] Channel info this context uses. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Context );
    BDBG_ASSERT( Map );

    Map->CDB_Read = Context->BaseAddr + CDB_READ_PTR_OFFSET;
    Map->CDB_Base = Context->BaseAddr + CDB_BASE_PTR_OFFSET;
    Map->CDB_Wrap = Context->BaseAddr + CDB_WRAP_PTR_OFFSET;
    Map->CDB_Valid = Context->BaseAddr + CDB_VALID_PTR_OFFSET;
    Map->CDB_End = Context->BaseAddr + CDB_END_PTR_OFFSET;

    Map->ITB_Read = Context->BaseAddr + ITB_READ_PTR_OFFSET;
    Map->ITB_Base = Context->BaseAddr + ITB_BASE_PTR_OFFSET;
    Map->ITB_Wrap = Context->BaseAddr + ITB_WRAP_PTR_OFFSET;
    Map->ITB_Valid = Context->BaseAddr + ITB_VALID_PTR_OFFSET;
    Map->ITB_End = Context->BaseAddr + ITB_END_PTR_OFFSET;

    /* The picture counter */
    Map->PictureCounter = GetPictureCounterReg( Context );

    /* Picture Counter Increment/Decrement/Reset Control Register */
    Map->PicIncDecCtrl = Context->BaseAddr + PIC_INC_DEC_CTRL_OFFSET;

    Map->ContextIdx = Context->Index;

    return( ExitCode );
}

uint32_t GetPictureCounterReg(
    BXPT_RaveCx_Handle hCtx
    )
{
    return hCtx->BaseAddr + PICTURE_CTR_OFFSET;
}

BERR_Code BXPT_Rave_GetRecordConfig(
    BXPT_RaveCx_Handle Ctx,         /* [in] The context  */
    BXPT_Rave_RecordSettings *Cfg   /* [out] The record settings. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Ctx );
    BDBG_ASSERT( Cfg );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG1_OFFSET );
    switch( BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, OUTPUT_FORMAT ) )
    {
        case 0: Cfg->OutputFormat = BAVC_StreamType_eEs; break;
        case 1: Cfg->OutputFormat = BAVC_StreamType_ePes; break;
        case 2: Cfg->OutputFormat = BAVC_StreamType_eTsMpeg; break;

        default:
        BDBG_ERR(( "Unsupported output format!" ));
        Cfg->OutputFormat = BAVC_StreamType_eTsMpeg;        /* This is the most popular. */
        break;
    }

    Cfg->StreamIdHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_HI );
    Cfg->StreamIdLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_LO );
    Cfg->MpegMode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE ) == 1 ? true : false;
    Cfg->BandHoldEn = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1,  BAND_HOLD_EN  ) == 1 ? true : false;
    Cfg->removeEmulationPrevention = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, EMU_PREV_BYTE_REMOVE  ) == 1 ? true : false;

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_CTRL1_OFFSET );
    Cfg->UseTimeStamps = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_CTRL1, REC_TIMESTAMP_ENABLE ) ? true : false;

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_INIT_TS_OFFSET );
    Cfg->InitialTs = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_INIT_TS, INIT_TS );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_TS_CTRL_OFFSET );
    Cfg->TsInitEn = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_TS_CTRL, TS_INIT_EN ) ? true : false;
    Cfg->DisableTimestampParityCheck = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_TS_CTRL, TS_CHECK_DIS ) ? true : false;
    Cfg->TimestampMode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_TS_CTRL, REC_TIMESTAMP_MODE );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_CDB_THRESHOLD_OFFSET );
    Cfg->CdbUpperThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_UPPER_THRESHOLD );
    Cfg->CdbLowerThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_LOWER_THRESHOLD );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_ITB_THRESHOLD_OFFSET );
    Cfg->ItbUpperThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_UPPER_THRESHOLD );
    Cfg->ItbLowerThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_LOWER_THRESHOLD );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_CDB_MIN_DEPTH_THRESHOLD_OFFSET );
    Cfg->CdbMinDepthThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_CDB_MIN_DEPTH_THRESHOLD , CDB_MIN_DEPTH_THRESHOLD );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_ITB_MIN_DEPTH_THRESHOLD_OFFSET );
    Cfg->ItbMinDepthThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_ITB_MIN_DEPTH_THRESHOLD , ITB_MIN_DEPTH_THRESHOLD );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG3_OFFSET );
    Cfg->DisableContinuityCheck = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_CC_CHECK ) ? true : false;

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_TIME_CONFIG_OFFSET );
    Cfg->CountRecordedPackets = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_TIME_CONFIG, REC_COUNT_MODE ) ? true : false;

    if(Ctx->allocatedCdbBufferSize == Ctx->usedCdbBufferSize)
    {
        Cfg->UseCdbSize =0;
    }
    else
    {
        Cfg->UseCdbSize = Ctx->usedCdbBufferSize;
    }

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG_OFFSET );
    Cfg->EmmModeEn = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_MISC_CONFIG, EMM_EN ) ? true : false;

#if BXPT_HAS_RAVE_AUTO_READ
    Cfg->AutoReadEn = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_MISC_CONFIG, REC_AUTO_UPDATE_RD_PTR_EN ) ? true : false;
#endif

#if BXPT_HAS_ATS
    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + ATSOFFSET_CFG_OFFSET );
    Cfg->LockTimestampsToPcrs = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_ATSOFFSET_CONFIG, ATSOFFSET_ENABLE );
    Cfg->PcrPidChannelIndex = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_ATSOFFSET_CONFIG, PCR_PID_CH );
    Cfg->GenerateLocalAts = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_ATSOFFSET_CONFIG, LOCAL_ATS_GEN_ENABLE ) ? true : false;
    Cfg->LocalAtsFormat = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_ATSOFFSET_CONFIG, LOCAL_ATS_FORMAT );
#endif

#if BXPT_NUM_TSIO
    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG5_OFFSET );
    Cfg->BulkMode = Reg & (1 << 10) ? true : false;
#else
    Cfg->BulkMode = false;
#endif

   /* SW7445-888: Enable MCPB MUXlib indexer */
   /* RESERVED_FOR_FW_1 is two bits wide, but only the upper bit is used for the SCT enable. */
   Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG5_OFFSET );
   Cfg->EnableSingleChannelMuxCapableIndexer = (BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG5, RESERVED_FOR_FW_1 ) & 0x02) ? true : false;

    /* GCB */
    #define FW_REG_5 ( BCHP_XPT_RAVE_CX0_RAVE_Reg_5 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
    Reg = BREG_Read32(Ctx->hReg, Ctx->BaseAddr + FW_REG_5);
    Cfg->bipIndexing = (Reg >> 27) & 1;
    Cfg->bipPidChannel = Cfg->bipIndexing ? (Reg >> 16) & 0x7ff : 0;

   /* REC_AVN is 1 for record config, 0 for AV. */
   Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG_OFFSET );
   Cfg->useAvConfig = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_MISC_CONFIG, REC_AVN ) ? false : true;

#if BXPT_HAS_PACKET_PLACEHOLDER
    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_TIME_CONFIG_OFFSET );
    Cfg->packetPlaceholderEn = BCHP_GET_FIELD_DATA(Reg, XPT_RAVE_CX0_REC_TIME_CONFIG, PKT_PLACEHOLDER_EN ) ? true : false;
    Cfg->packetTimeoutItbGenEn = BCHP_GET_FIELD_DATA(Reg, XPT_RAVE_CX0_REC_TIME_CONFIG, REC_TIMEOUT_ITB_EN ) ? true : false;
    Cfg->timeoutVal = BCHP_GET_FIELD_DATA(Reg, XPT_RAVE_CX0_REC_TIME_CONFIG, REC_TIMEOUT_VAL );
#endif

    return( ExitCode );
}

BERR_Code BXPT_Rave_SetRecordConfig(
    BXPT_RaveCx_Handle Ctx,             /* [in] The context  */
    const BXPT_Rave_RecordSettings *Cfg /* [in] The record settings. */
    )
{
    uint32_t Reg;
    BMMA_DeviceOffset offset;
    bool BandHoldEn;

    unsigned PesSyncMode = 1;
    unsigned OutputFormat = 2;
    unsigned InputEsFormat = 0;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Ctx );
    BDBG_ASSERT( Cfg );

    BandHoldEn = Cfg->BandHoldEn;
    if( Ctx->Type == BXPT_RaveCx_eIp )
    {
        PesSyncMode = 3;
        InputEsFormat = 0x0A;
    }
    else
#if BXPT_NUM_TSIO
        if( Cfg->BulkMode )
            PesSyncMode = 3;
        else
            PesSyncMode = Cfg->MpegMode == true ? 1 : 3;
#else
        PesSyncMode = Cfg->MpegMode == true ? 1 : 3;
#endif

    switch( Cfg->OutputFormat )
    {
        case BAVC_StreamType_eTsMpeg:
#if BXPT_NUM_TSIO
        if( Cfg->BulkMode )
            OutputFormat = 3;
        else
            OutputFormat = 2;
#else
        OutputFormat = 2;
#endif
        break;

        case BAVC_StreamType_ePes:
        OutputFormat = 1;
        break;

        case BAVC_StreamType_eEs:
        OutputFormat = 0;
        break;

        default:
        BDBG_ERR(( "Unsupported output format!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto done;
    }

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_CTRL1_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_REC_CTRL1, REC_TIMESTAMP_ENABLE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_CTRL1, REC_TIMESTAMP_ENABLE, Cfg->UseTimeStamps == true ? 1 : 0 )
    );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + REC_CTRL1_OFFSET, Reg );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_INIT_TS_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_REC_INIT_TS, INIT_TS )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_INIT_TS, INIT_TS, Cfg->InitialTs )
    );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + REC_INIT_TS_OFFSET, Reg );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_TS_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_REC_TS_CTRL, TS_INIT_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_REC_TS_CTRL, TS_CHECK_DIS ) |
        BCHP_MASK( XPT_RAVE_CX0_REC_TS_CTRL, REC_TIMESTAMP_MODE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_TS_CTRL, TS_INIT_EN, Cfg->TsInitEn == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_TS_CTRL, TS_CHECK_DIS, Cfg->DisableTimestampParityCheck == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_TS_CTRL, REC_TIMESTAMP_MODE, Cfg->TimestampMode )
    );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + REC_TS_CTRL_OFFSET, Reg );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG1_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, BAND_HOLD_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, SHIFT_PTS ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, OUTPUT_FORMAT ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_HI ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, EMU_PREV_BYTE_REMOVE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, EMU_PREV_MODE )
    );

    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, BAND_HOLD_EN, BandHoldEn == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, SHIFT_PTS, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE, PesSyncMode ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, OUTPUT_FORMAT, OutputFormat ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, INPUT_ES_FORMAT, InputEsFormat ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_HI, Cfg->StreamIdHi ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_LO, Cfg->StreamIdLo ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, EMU_PREV_BYTE_REMOVE, Cfg->removeEmulationPrevention ? 1 : 0 )
    );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG1_OFFSET, Reg );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_CDB_THRESHOLD_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_UPPER_THRESHOLD ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_LOWER_THRESHOLD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_UPPER_THRESHOLD, Cfg->CdbUpperThreshold ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_LOWER_THRESHOLD, Cfg->CdbLowerThreshold )
    );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + AV_CDB_THRESHOLD_OFFSET, Reg );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_ITB_THRESHOLD_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_UPPER_THRESHOLD ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_LOWER_THRESHOLD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_UPPER_THRESHOLD, Cfg->ItbUpperThreshold ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_LOWER_THRESHOLD, Cfg->ItbLowerThreshold )
    );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + AV_ITB_THRESHOLD_OFFSET, Reg );

    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + AV_CDB_MIN_DEPTH_THRESHOLD_OFFSET, Cfg->CdbMinDepthThreshold );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + AV_ITB_MIN_DEPTH_THRESHOLD_OFFSET, Cfg->ItbMinDepthThreshold );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG3_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_CC_CHECK )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_CC_CHECK, Cfg->DisableContinuityCheck == true ? 1 : 0 )
    );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG3_OFFSET, Reg );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_TIME_CONFIG_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_REC_TIME_CONFIG, REC_COUNT_MODE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_TIME_CONFIG, REC_COUNT_MODE, Cfg->CountRecordedPackets == true ? 1 : 0 )
    );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + REC_TIME_CONFIG_OFFSET, Reg );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG3_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_PKT_ERRORS )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_PKT_ERRORS, 1 )
    );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG3_OFFSET, Reg );

    /*
    ** PR 27902: Workaround for this PR requires limiting number of startcode extracted
    ** from a single transport packet. We'll set that limit to 16 for records and
    ** 53 for AV (shorter blocks of data are extracted for the decoders). There is
    ** similar code in BXPT_Rave_SetAvConfig().
    **
    ** Feb 2011: Version 1.21 of the config spreadsheets changes MAX_COMPARE_PATTERNS
    ** to 13 for video. We don't generate index tables for audio during records, so
    ** this shouldn't cause any issues in audio.
    */
    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG3_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, MAX_COMPARE_PATTERNS )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, MAX_COMPARE_PATTERNS, 13 )
    );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG3_OFFSET, Reg );

    offset = BREG_ReadAddr( Ctx->hReg, Ctx->BaseAddr + CDB_BASE_PTR_OFFSET );
    if(Cfg->UseCdbSize)
    {
        if(Cfg->UseCdbSize > Ctx->allocatedCdbBufferSize)
        {
            /* Invalid argument UseCdbSize */
            BDBG_ERR(( "Invalid UseCdbSize=%u arg, should be < %u", Cfg->UseCdbSize,Ctx->allocatedCdbBufferSize ));
        }
        else
        {
            /* adjust the end pointer to the new value */
            Ctx->usedCdbBufferSize = Cfg->UseCdbSize;
            BREG_WriteAddr( Ctx->hReg, Ctx->BaseAddr + CDB_END_PTR_OFFSET, offset + Ctx->usedCdbBufferSize - 1 );
        }

    }
    else
    {
        /* adjust the cdb end pointer to allocated size */
        Ctx->usedCdbBufferSize = Ctx->allocatedCdbBufferSize;
        BREG_WriteAddr( Ctx->hReg, Ctx->BaseAddr + CDB_END_PTR_OFFSET, offset + Ctx->usedCdbBufferSize - 1 );
    }

    if(Cfg->EmmModeEn)
    {
         Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG_OFFSET );
         Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, EMM_EN ) );
         Reg |= BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, EMM_EN,1 ) ;
         BREG_Write32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );
    }

#if BXPT_HAS_RAVE_AUTO_READ
    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG_OFFSET );
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, REC_AUTO_UPDATE_RD_PTR_EN ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, REC_AUTO_UPDATE_RD_PTR_EN, Cfg->AutoReadEn == true ? 1 : 0 ) );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );
#endif

#if BXPT_HAS_ATS
    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + ATSOFFSET_CFG_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_ATSOFFSET_CONFIG, LOCAL_ATS_GEN_ENABLE ) |
        BCHP_MASK( XPT_RAVE_CX0_ATSOFFSET_CONFIG, LOCAL_ATS_FORMAT ) |
        BCHP_MASK( XPT_RAVE_CX0_ATSOFFSET_CONFIG, ATSOFFSET_ENABLE ) |
        BCHP_MASK( XPT_RAVE_CX0_ATSOFFSET_CONFIG, PCR_PID_CH )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_ATSOFFSET_CONFIG, LOCAL_ATS_GEN_ENABLE, Cfg->GenerateLocalAts == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_ATSOFFSET_CONFIG, LOCAL_ATS_FORMAT, Cfg->LocalAtsFormat ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_ATSOFFSET_CONFIG, ATSOFFSET_ENABLE, Cfg->LockTimestampsToPcrs == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_ATSOFFSET_CONFIG, PCR_PID_CH_CHANGED, Cfg->LockTimestampsToPcrs == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_ATSOFFSET_CONFIG, PCR_PID_CH, Cfg->PcrPidChannelIndex )
    );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + ATSOFFSET_CFG_OFFSET, Reg );
#endif

#if BXPT_NUM_TSIO
    /* Disable fix for bad adaptation field length when Bulk mode is selected. */
    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG5_OFFSET );
    Reg &= ~(1 << 10);
    if( Cfg->BulkMode )
        Reg |= (1 << 10);

    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG5_OFFSET, Reg );
    if( Cfg->BulkMode )
    {
        /* Not all chips have the correct RDB defines. */
        Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG5_OFFSET);
        Reg &= ~(
            (1 << 13) |     /* DROP_GARBAGE_DATA_ON_PUSI_ERROR_FLAG */
            (1 << 11 )      /* DROP_GARBAGE_DATA_ON_PUSI_ERR */
        );
        Reg |= (
            (1 << 11 )      /* DROP_GARBAGE_DATA_ON_PUSI_ERR */
        );
        BREG_Write32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG5_OFFSET, Reg );
    }
#endif /* BXPT_NUM_TSIO */

    /* SW7445-888: Enable MCPB MUXlib indexer */
    /* RESERVED_FOR_FW_1 is two bits wide, but only the upper bit is used for the SCT enable.
    Need to leave the lower bit alone. We're forced to make our own mask until there's proper RDB support. */
    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG5_OFFSET );
    Reg &= ~(0x2 << BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG5_RESERVED_FOR_FW_1_SHIFT);
    Reg |= (Cfg->EnableSingleChannelMuxCapableIndexer ? 0x2 : 0x0) << BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG5_RESERVED_FOR_FW_1_SHIFT;
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + AV_MISC_CFG5_OFFSET, Reg );

    /* GCB */
    {
    #define FW_REG_5 ( BCHP_XPT_RAVE_CX0_RAVE_Reg_5 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
    Reg = BREG_Read32(Ctx->hReg, Ctx->BaseAddr + FW_REG_5);
    Reg &= ~(
        (1 << 27) |
        (0x7FF << 16));

    if (Cfg->bipIndexing) {
        if (Cfg->bipPidChannel > 0x3FF) {
            BDBG_ERR(("bipPidChannel %u is out of range", Cfg->bipPidChannel));
            goto done;
        }

        Reg |= (
            (1 << 27) |
            (Cfg->bipPidChannel << 16));
    }
    BREG_Write32(Ctx->hReg, Ctx->BaseAddr + FW_REG_5, Reg);
    }

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG_OFFSET );
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, REC_AVN ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, REC_AVN, Cfg->useAvConfig ? 0 : 1 ));
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

#if BXPT_HAS_PACKET_PLACEHOLDER
    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_TIME_CONFIG_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_REC_TIME_CONFIG, PKT_PLACEHOLDER_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_REC_TIME_CONFIG, REC_TIMEOUT_ITB_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_REC_TIME_CONFIG, REC_TIMEOUT_VAL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_TIME_CONFIG, PKT_PLACEHOLDER_EN, Cfg->packetPlaceholderEn ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_TIME_CONFIG, REC_TIMEOUT_ITB_EN, Cfg->packetTimeoutItbGenEn ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_TIME_CONFIG, REC_TIMEOUT_VAL, Cfg->timeoutVal )
    );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + REC_TIME_CONFIG_OFFSET, Reg );
#endif

    done:
    return( ExitCode );
}

BERR_Code BXPT_Rave_GetAvConfig(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    BXPT_Rave_AvSettings *Config    /* [out] The AV settings. */
    )
{
    uint32_t Reg;
    bool BandHoldEn;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Context );
    BDBG_ASSERT( Config );

    GetScRanges( Context, Config->EsRanges );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET );
    switch( BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, OUTPUT_FORMAT ) )
    {
        case 0: Config->OutputFormat = BAVC_StreamType_eEs; break;
        case 1: Config->OutputFormat = BAVC_StreamType_ePes; break;
        case 2: Config->OutputFormat = BAVC_StreamType_eTsMpeg; break;

        default:
        /* Ooops. Assume ES was used. */
        BDBG_ERR(( "Unsupported output format read from hardware!" ));
        Config->OutputFormat = BAVC_StreamType_eEs;
        break;
    }

    Config->InputFormat = Context->InputFormat;
    Config->ItbFormat = Context->ItbFormat;
    Config->StreamIdHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_HI );
    Config->StreamIdLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_LO );

    BandHoldEn = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, BAND_HOLD_EN ) ? true : false;
    Config->BandHoldEn = BandHoldEn;

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_CDB_THRESHOLD_OFFSET );
    Config->CdbUpperThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_UPPER_THRESHOLD );
    Config->CdbLowerThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_LOWER_THRESHOLD );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_ITB_THRESHOLD_OFFSET );
    Config->ItbUpperThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_UPPER_THRESHOLD );
    Config->ItbLowerThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_LOWER_THRESHOLD );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG3_OFFSET );
    Config->EnableBPPSearch = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_BPP_SEARCH ) ? true : false;
    Config->DisableContinuityCheck = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_CC_CHECK ) ? true : false;
    Config->PcrRtsEntryEn = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, PCR_RTS_EN ) ? true : false;
    Config->DisablePacketErrors = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_PKT_ERRORS ) ? true : false;
    Config->PesSidExcludeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, PES_SID_EXCLUDE_HI);
    Config->PesSidExcludeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, PES_SID_EXCLUDE_LO);
    Config->Transcoding = Context->Transcoding;
    Config->ScOrMode = Context->ScOrMode;
    return( ExitCode );
}

#if BXPT_HAS_HEVD_WORKAROUND
/*
Workaround for SW7439-58, SW7366-48, and SW7445-568 (same issue on different chips). The wraparound
points needs to be fixed for video contexts
*/

static void SetWrapPointerForHEVD(
    BXPT_RaveCx_Handle hCtx         /* [in] The context. */
    )
{
    uint32_t reg;
    BMMA_DeviceOffset endAddr, wrapThreshold, adjustedWrapAddr;

    BDBG_MSG(( "Context %u using fixed wraparound", hCtx->Index ));

    /* Force wraparound pointer to the END minus the wrap threshold. */
    endAddr = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_END_PTR_OFFSET );
    reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_THRESHOLDS_OFFSET );
    wrapThreshold = BCHP_GET_FIELD_DATA( reg, XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_WRAPAROUND_THRESHOLD );
    adjustedWrapAddr = (endAddr - wrapThreshold);
    BDBG_MSG(( "%s: endAddr " BDBG_UINT64_FMT ", wrapThreshold " BDBG_UINT64_FMT ", adjustedWrapAddr " BDBG_UINT64_FMT "", BSTD_FUNCTION,
            BDBG_UINT64_ARG(endAddr), BDBG_UINT64_ARG(wrapThreshold), BDBG_UINT64_ARG(adjustedWrapAddr) ));
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_WRAP_PTR_OFFSET, adjustedWrapAddr );
    BDBG_MSG(( "%s: readback adjustedWrapAddr " BDBG_UINT64_FMT " ", BSTD_FUNCTION, BDBG_UINT64_ARG(BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_WRAP_PTR_OFFSET )) ));

    /*
    Set the enable bit to tell RAVE fw that the workaround should be done. This bit is cleared when the
    context is freed. The AVD fw guys have confirmed that the workaround is supported for all
    video formats, not just HEVC.
    */
    reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG5_OFFSET);
    reg |= 1 << 27;
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG5_OFFSET, reg );
}
#endif

BERR_Code BXPT_Rave_SetAvConfig(
    BXPT_RaveCx_Handle Context,         /* [in] The context. */
    const BXPT_Rave_AvSettings *Config  /* [in] The AV settings. */
    )
{
    uint32_t Reg;
    BAVC_ItbEsType ItbFormat;
    unsigned MaxPatterns;
    bool BandHoldEn;

    unsigned PesType = 0;
    unsigned PesSyncMode = 0;
    bool ShiftPts = true;
    bool ConvertPts = false;
    bool EmulationPrevRemove = false;
    bool EmulationPrevMode = 0;     /* Default to byte removal */
    unsigned OutputFormat = 0;
    BERR_Code ExitCode = BERR_SUCCESS;
    unsigned EsFormat = 0;
    bool disableDataDropOnPusiErr = false;

    BDBG_ASSERT( Context );
    BDBG_ASSERT( Config );

    /* Change stuff ONLY if the context is disabled. */
    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET );
    if ( BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE ) )
    {
        BDBG_ERR(( "Context is enabled! BXPT_Rave_SetAvConfig will have no effect." ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto done;
    }

    /*
    ** Some formats are very similar to earlier formats. Map the ES format as needed,
    ** then do format-specific configuration. We also need to note what the requested
    ** format was, for use by the GetAvConfig() call.
    */
    Context->ItbFormat = Config->ItbFormat;

    /*
    ** PR 36908: Raptor engineers request that on the 7400, AC3 audio should be handled in the same
    ** way as AC3+. The XPT PI will honor AC3 audio enum in the Get/Set AV config calls, but
    ** will silently configure for AC3+.
    */
    if( Config->ItbFormat == BAVC_ItbEsType_eAc3gAudio )
        ItbFormat = BAVC_ItbEsType_eAc3Plus;
    else
        ItbFormat = Config->ItbFormat;

    Context->InputFormat = Config->InputFormat;

    switch( Config->InputFormat )
    {
        /* ES,PES& PS,mpeg1 ss are carried in MPEG transport, unless the DSS enums (below) are used */
        case BAVC_StreamType_eTsMpeg:
        case BAVC_StreamType_eEs:
        case BAVC_StreamType_ePes:
        case BAVC_StreamType_ePS:
        /* case BAVC_StreamType_eVCD: */
        case BAVC_StreamType_eMpeg1System:  /* use this for VCD SS */
        switch( ItbFormat )
        {
            /* WMA data will be put into MPEG TS by the playback channel packetizer */
            case BAVC_ItbEsType_eWma:
            PesType = 0;
            PesSyncMode = 1;
            break;

            case BAVC_ItbEsType_eMpegAudioWithDescriptor:
            case BAVC_ItbEsType_eAudioDescriptor:
            PesType = 0;        /* MPEG2 type PES headers */
            PesSyncMode = 1;    /* Use payload unit start bit in MPEG header. */
            break;

            default:
            PesType = 0;        /* MPEG2 type PES headers */
            PesSyncMode = 1;    /* Use payload unit start bit in MPEG header. */
            break;
        }
        if(Config->InputFormat == BAVC_StreamType_eMpeg1System)
            PesType = 1;        /* mpeg1 pes headers */

        break;

        case BAVC_StreamType_eDssEs:
        disableDataDropOnPusiErr = true;
        ShiftPts = false;
        switch( ItbFormat )
        {
            case BAVC_ItbEsType_eMpeg2Video:
            case BAVC_ItbEsType_eAvcVideo:  /* invalid case,AVC is always in DSSPes format */
            ConvertPts = false; /* no need to convert pr 27445 */
            PesType = 2;        /* User data PTS and DTS */
            PesSyncMode = 0;    /* Continual PES search mode. Video is unbounded PES */
            break;

            default:
            BDBG_ERR(( "Unsupported ItbFormat format for BAVC_StreamType_eDssEs!" ));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
            goto done;
        }
        break;

        case BAVC_StreamType_eDssPes:      /* DSS transport carrying PES ( or PES-like ) data */
        disableDataDropOnPusiErr = true;
        ShiftPts = false;
        switch( ItbFormat )
        {
            case BAVC_ItbEsType_eAc3gAudio:
            case BAVC_ItbEsType_eAc3Plus:
            ConvertPts = true;
            PesType = 0;
            PesSyncMode = 2;        /* Bounded PES sync */
            break;

            case BAVC_ItbEsType_eMpegAudio:
            case BAVC_ItbEsType_eMpegAudioLayer3:
            case BAVC_ItbEsType_eMpegAudio2_5:
            ConvertPts = false;
            PesType = 1;            /* MPEG1 style System Headers (PES-like) */
            PesSyncMode = 2;        /* Bounded PES sync. */
            break;

            case BAVC_ItbEsType_eMpeg2Video:
            case BAVC_ItbEsType_eAvcVideo:
            ConvertPts = true;
            PesType = 0;
            PesSyncMode = 0;        /* Continual PES search mode. Video is unbounded PES */
            break;

            default:
            BDBG_ERR(( "Unsupported ItbFormat format for BAVC_StreamType_eDssPes!" ));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
            goto done;
        }
        break;

        default:
        BDBG_ERR(( "Unsupported input format %d!", Config->InputFormat ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto done;
    }
    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG5_OFFSET );
    if( disableDataDropOnPusiErr )
    {
        Reg |= ( 1 << 11 );
        ClearDataDropOnPusiErrState( Context );
    }
    else
        Reg &= ~( 1 << 11 );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG5_OFFSET, Reg );

    EmulationPrevRemove = false;
    EmulationPrevMode = 0;

    /* Initialize to false, then enable only for certain CODECs */
    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG2_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, DISABLE_BEFORE_PES )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG2_OFFSET, Reg );

    switch( ItbFormat )
    {
        case BAVC_ItbEsType_eMpeg2Video:
        case BAVC_ItbEsType_eMpeg1Video:
        EmulationPrevRemove = false;
        EsFormat = 0x00;
        break;

        case BAVC_ItbEsType_DVD_Subpicture:
        case BAVC_ItbEsType_DVD_HLI:
        EmulationPrevRemove = false;
        EsFormat = 0x00;
        break;

        case BAVC_ItbEsType_eMpeg4Part2:
        EmulationPrevRemove = false;
        EsFormat = 0x00;
        break;

        case BAVC_ItbEsType_eAvcVideo:
        case BAVC_ItbEsType_eVc1Video:
        EmulationPrevRemove = true;
        EsFormat = 0x00;
        break;

        case BAVC_ItbEsType_eH263:
        case BAVC_ItbEsType_eVp6Video:
        EmulationPrevRemove = false;
        EsFormat = 0x00;
        break;

        case BAVC_ItbEsType_eAc3Plus:
        EmulationPrevRemove = false;
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */
        break;

        case BAVC_ItbEsType_eAacHe:
        case BAVC_ItbEsType_eAacAudio:
        EmulationPrevRemove = false;
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */
        break;

        case BAVC_ItbEsType_eWma:
        EmulationPrevRemove = false;
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */
        break;

        case BAVC_ItbEsType_eMpegAudio:
        case BAVC_ItbEsType_eMpegAudioLayer3:
        case BAVC_ItbEsType_eMpegAudio2_5:
        EmulationPrevRemove = false;
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */

        /* SW7422-132: Don't put any data into the CDB until a PES header is seen. */
        if( Config->InputFormat == BAVC_StreamType_eTsMpeg )
        {
            uint32_t Reg;

            Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG2_OFFSET );
            Reg &= ~(
                BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, DISABLE_BEFORE_PES )
            );
            Reg |= (
                BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, DISABLE_BEFORE_PES, 1 )
            );
            BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG2_OFFSET, Reg );
        }
        /* SW7422-132 */

        break;

        case BAVC_ItbEsType_eMlpAudio:
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */
        break;

        case BAVC_ItbEsType_eAvsVideo:
        EmulationPrevRemove = true;
        EmulationPrevMode = 1;  /* Emulation prev is a bit in AVS streams */
        EsFormat = 0x00;
        break;

        case BAVC_ItbEsType_eAvsAudio:
        EmulationPrevRemove = true;
        EmulationPrevMode = 1;  /* Emulation prev is a bit in AVS streams */
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */
        break;

        case BAVC_ItbEsType_eMpegAudioWithDescriptor:
        /* SW7422-132: Don't put any data into the CDB until a PES header is seen. */
        if( Config->InputFormat == BAVC_StreamType_eTsMpeg )
        {
            uint32_t Reg;

            Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG2_OFFSET );
            Reg &= ~(
                BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, DISABLE_BEFORE_PES )
            );
            Reg |= (
                BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, DISABLE_BEFORE_PES, 1 )
            );
            BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG2_OFFSET, Reg );
        }
        /* SW7422-132 */
        /* Then fall through to BAVC_ItbEsType_eAudioDescriptor. */

        case BAVC_ItbEsType_eAudioDescriptor:
        EmulationPrevRemove = false;
        EsFormat = 0x0C;
        break;

        case BAVC_ItbEsType_eDra:
        EmulationPrevRemove = false;
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */
        break;

        case BAVC_ItbEsType_eOTFVideo:
        EmulationPrevRemove = false;
        EsFormat = ItbFormat;
        break;

        default:
        /* TODO: Add cases for the rest of the ITB formats. */
        EmulationPrevRemove = false;
        EsFormat = 0x01;    /* Video doesn't care about the ES type in the ITB, so it's safe to map this for rest of the codecs.  */
        break;
    }

    /* SW7422-20: Audio decoders need one Base entry in the ITB for each TS packet recieved. */
    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + REC_MISC_CFG5_OFFSET );
    Reg &= ~(1 << 31 );
#if 0
/* 21 Mar 2011: Arul Thangaraj requested this feature be disabled for all chips until further notice. */
    if (EsFormat == 1 || EsFormat == 0x0C)
    {
        Reg |= (1 << 31);   /* No RDB for this field yet */
    }
#endif
    BREG_Write32( Context->hReg, Context->BaseAddr + REC_MISC_CFG5_OFFSET, Reg );

    /* PR 28909: For transcoding applications, we need to keep the emulation prev bytes. */
    if( Config->Transcoding == true )
    {
    /* DEPRECATED: No projects are actively using this feature. It's interfering with
        ** the VCE-based transcoding tests. See SW7425-5829.
        EmulationPrevRemove = false;
    */
        BDBG_WRN(( "%s: Config->Transcoding is no longer supported", BSTD_FUNCTION ));
    }

    switch( Config->OutputFormat )
    {
        case BAVC_StreamType_eTsMpeg: OutputFormat = 2; break;
        case BAVC_StreamType_ePes: OutputFormat = 1; break;
        case BAVC_StreamType_eEs: OutputFormat = 0; break;

        default:
        BDBG_ERR(( "Unsupported output format!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto done;
    }

#if BXPT_HAS_HEVD_WORKAROUND
    /* Do this only for video, with ES output to the CDB . */
    if( ( !EsFormat || BAVC_ItbEsType_eOTFVideo == ItbFormat) && 0 == OutputFormat )
    {
        SetWrapPointerForHEVD( Context );
        Context->isHevdVideoContext = true;
    }
    else
        Context->isHevdVideoContext = false;
#endif

    LoadScRanges( Context, Config->EsRanges );

    /* No comparator configuration is required for BAVC_ItbEsType_eLpcmAudio.On 7440
       set the AudFrameInfo to appropriate value using BXPT_Rave_SetContextConfig*/
    /* if(!(ItbFormat == BAVC_ItbEsType_eLpcmAudio || ItbFormat == BAVC_ItbEsType_eMlpAudio) || ( ItbFormat == BAVC_ItbEsType_eDtsAudio ))
    {*/
        ConfigureComparators( Context, ItbFormat );
    /*}*/

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_TYPE_MODE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, SHIFT_PTS ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, CONVERT_PTS ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, EMU_PREV_BYTE_REMOVE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, EMU_PREV_MODE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, BAND_HOLD_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, INPUT_ES_FORMAT ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_HI ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_LO ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, OUTPUT_FORMAT )
    );

    BandHoldEn = Config->BandHoldEn;

    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_TYPE_MODE, PesType ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE, PesSyncMode ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, SHIFT_PTS, ShiftPts ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, CONVERT_PTS, ConvertPts ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, EMU_PREV_BYTE_REMOVE, EmulationPrevRemove ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, EMU_PREV_MODE, EmulationPrevMode ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, BAND_HOLD_EN, BandHoldEn == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, INPUT_ES_FORMAT, EsFormat ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_HI, Config->StreamIdHi ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_LO, Config->StreamIdLo ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, OUTPUT_FORMAT, OutputFormat )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET, Reg );

    ResetContextPointers( Context );

    switch( ItbFormat )
    {
        /* We only want to set picture counters for video streams. */
        case BAVC_ItbEsType_eMpeg2Video:
        case BAVC_ItbEsType_eMpeg1Video:
        case BAVC_ItbEsType_eAvcVideo:
        case BAVC_ItbEsType_eVc1Video:
        SetPictureCounterMode( Context, ItbFormat );
        break;

        case BAVC_ItbEsType_DVD_Subpicture:
        case BAVC_ItbEsType_DVD_HLI:
        SetPictureCounterMode( Context, ItbFormat );
        break;

        /* Ignore audio */
        default:
        break;
    }

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG3_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_BPP_SEARCH ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_CC_CHECK ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, PCR_RTS_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_PKT_ERRORS )
    );

    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_BPP_SEARCH, Config->EnableBPPSearch == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_CC_CHECK, Config->DisableContinuityCheck == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, PCR_RTS_EN, Config->PcrRtsEntryEn == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_PKT_ERRORS, Config->DisablePacketErrors == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, PES_SID_EXCLUDE_HI, Config->PesSidExcludeHi ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, PES_SID_EXCLUDE_LO, Config->PesSidExcludeLo )
    );

    if (Config->DisablePacketErrors)
    {
        BDBG_WRN(( "Packet errors are disabled in ITB,use only for debugging purposes" ));
    }
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG3_OFFSET, Reg );

    if( Config->CdbUpperThreshold == 0 )
    {
        BDBG_WRN(( "Invalid CDB threshold for av context, call BXPT_Rave_GetDefaultThresholds to get default value" ));
    }

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_CDB_THRESHOLD_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_UPPER_THRESHOLD ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_LOWER_THRESHOLD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_UPPER_THRESHOLD, Config->CdbUpperThreshold ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_LOWER_THRESHOLD, Config->CdbLowerThreshold )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_CDB_THRESHOLD_OFFSET, Reg );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_ITB_THRESHOLD_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_UPPER_THRESHOLD ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_LOWER_THRESHOLD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_UPPER_THRESHOLD, Config->ItbUpperThreshold ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_LOWER_THRESHOLD, Config->ItbLowerThreshold )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_ITB_THRESHOLD_OFFSET, Reg );

    /* PR 32434: ITB Config spreadsheet ver 1.6 required MaxCompare == 15 for audio */
    switch( ItbFormat )
    {
        case BAVC_ItbEsType_eMpeg2Video:
        case BAVC_ItbEsType_eAvcVideo:
        case BAVC_ItbEsType_eMpeg1Video:
        case BAVC_ItbEsType_eMpeg4Part2:
        case BAVC_ItbEsType_eH263:
        case BAVC_ItbEsType_eVc1Video:
        MaxPatterns = 52;     /* AV contexts using 1-byte extraction */
        break;

        case BAVC_ItbEsType_DVD_Subpicture:
        case BAVC_ItbEsType_DVD_HLI:
        MaxPatterns = 53;     /* AV contexts using 1-byte extraction */
        break;

        case BAVC_ItbEsType_eOTFVideo:
        case BAVC_ItbEsType_eAvsAudio:
        MaxPatterns = 14;
        break;

        case  BAVC_ItbEsType_eAvsVideo:
        MaxPatterns = 52;
        break;

        default:
        MaxPatterns = 14;
        break;
    }

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG3_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, MAX_COMPARE_PATTERNS )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, MAX_COMPARE_PATTERNS, MaxPatterns )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG3_OFFSET, Reg );


    /* PR57627 :
    ** SC_OR_MODE is used to select the way scramble control bits are reported.
    ** 0 = Disable OR-ing of current and previous scramble control bits (Default).
    ** 1 = Enable OR-ing of current and previous scramble control bits. This is to
    ** support streams which have mixture of scrambled and unscrambled packets within
    ** the same PID. In such case, these PIDs will be treated as scramble PIDs.
    ** By default this is disabled.
    */
    Context->ScOrMode = Config->ScOrMode;

    done:
    return( ExitCode );
}

BERR_Code BXPT_Rave_CheckBuffer(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    BXPT_Rave_ContextPtrs *Ptrs         /* [out] Pointers to the buffer data */
    )
{
    uint64_t Read, Valid, Wrap, Base;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Context );
    BDBG_ASSERT( Ptrs );

    Read = BREG_ReadAddr( Context->hReg, Context->BaseAddr + CDB_READ_PTR_OFFSET );
    Valid = BREG_ReadAddr( Context->hReg, Context->BaseAddr + CDB_VALID_PTR_OFFSET );
    Wrap = BREG_ReadAddr( Context->hReg, Context->BaseAddr + CDB_WRAP_PTR_OFFSET );
    Base = BREG_ReadAddr( Context->hReg, Context->BaseAddr + CDB_BASE_PTR_OFFSET );

    if( Read < Valid )
    {
        if( Context->CdbReset == true )
        {
            /*
            ** Checks after a context reset, but before the first update of the READ, are a special case.
            ** We need to do account for the fact that first burst of data written into the buffer
            ** started AT the BASE pointer, not VALID+1.
            */
            Ptrs->Cdb.DataPtr = (uint8_t*)Context->mma.cdbPtr + (Read - Context->mma.cdbOffset); /* convert Read -> cached ptr */
            Ptrs->Cdb.ByteCount = Valid - Read + 1;
        }
        else
        {
            Ptrs->Cdb.DataPtr = (uint8_t*)Context->mma.cdbPtr + (Read+1 - Context->mma.cdbOffset); /* convert (Read+1) -> cached ptr */
            Ptrs->Cdb.ByteCount = Valid - Read;
        }

        Ptrs->Cdb.WrapDataPtr = NULL;
        Ptrs->Cdb.WrapByteCount = 0;
    }
    else if( Read > Valid )
    {
        /* It did wrap */
        if( Read == Wrap )
        {
            /* They read up to the wraparound point. New data starts at the base */
            Ptrs->Cdb.DataPtr = (uint8_t*)Context->mma.cdbPtr + (Base - Context->mma.cdbOffset); /* convert Base -> cached ptr */
            Ptrs->Cdb.ByteCount = Valid - Base + 1;

            Ptrs->Cdb.WrapDataPtr = NULL;
            Ptrs->Cdb.WrapByteCount = 0;
        }
        else
        {
            Ptrs->Cdb.DataPtr = (uint8_t*)Context->mma.cdbPtr + (Read+1 - Context->mma.cdbOffset); /* convert (Read+1) -> cached ptr */
            Ptrs->Cdb.ByteCount = Wrap - Read;

            Ptrs->Cdb.WrapDataPtr = (uint8_t*)Context->mma.cdbPtr + (Base - Context->mma.cdbOffset); /* convert Base -> cached ptr */
            Ptrs->Cdb.WrapByteCount = Valid - Base + 1;
        }
    }
    else
    {
        /* No new data */
        Ptrs->Cdb.DataPtr = NULL;
        Ptrs->Cdb.ByteCount = 0;

        Ptrs->Cdb.WrapDataPtr = NULL;
        Ptrs->Cdb.WrapByteCount = 0;
    }

    Read = BREG_ReadAddr( Context->hReg, Context->BaseAddr + ITB_READ_PTR_OFFSET );
    Valid = BREG_ReadAddr( Context->hReg, Context->BaseAddr + ITB_VALID_PTR_OFFSET );
    Wrap = BREG_ReadAddr( Context->hReg, Context->BaseAddr + ITB_WRAP_PTR_OFFSET );
    Base = BREG_ReadAddr( Context->hReg, Context->BaseAddr + ITB_BASE_PTR_OFFSET );

    /* Some users don't allocate an ITB, in which case the Base register addr will be 0 */
    if( Base && ( Read < Valid ) )
    {
        if( Context->ItbReset == true )
        {
            /*
            ** Checks after a context reset, but before the first update of the READ, are a special case.
            ** We need to do account for the fact that first burst of data written into the buffer
            ** started AT the BASE pointer, not VALID+1.
            */
            Ptrs->Itb.DataPtr = (uint8_t*)Context->mma.itbPtr + (Read - Context->mma.itbOffset); /* convert Read -> cached ptr */
            Ptrs->Itb.ByteCount = Valid - Read + 1;
        }
        else
        {
            Ptrs->Itb.DataPtr = (uint8_t*)Context->mma.itbPtr + (Read+1 - Context->mma.itbOffset); /* convert Read+1 -> cached ptr */
            Ptrs->Itb.ByteCount = Valid - Read;
        }

        Ptrs->Itb.WrapDataPtr = NULL;
        Ptrs->Itb.WrapByteCount = 0;
    }
    else if( Base && ( Read > Valid ) )
    {
        /* It did wrap */
        if( Read == Wrap )
        {
            /* They read up to the wraparound point. New data starts at the base */
            Ptrs->Itb.DataPtr = (uint8_t*)Context->mma.itbPtr + (Base - Context->mma.itbOffset); /* convert Base -> cached ptr */
            Ptrs->Itb.ByteCount = Valid - Base + 1;

            Ptrs->Itb.WrapDataPtr = NULL;
            Ptrs->Itb.WrapByteCount = 0;
        }
        else
        {
            Ptrs->Itb.DataPtr = (uint8_t*)Context->mma.itbPtr + (Read+1 - Context->mma.itbOffset); /* convert Read+1 -> cached ptr */
            Ptrs->Itb.ByteCount = Wrap - Read;

            Ptrs->Itb.WrapDataPtr = (uint8_t*)Context->mma.itbPtr + (Base - Context->mma.itbOffset); /* convert Base -> cached ptr */
            Ptrs->Itb.WrapByteCount = Valid - Base + 1;
        }
    }
    else
    {
        /* No new data */
        Ptrs->Itb.DataPtr = NULL;
        Ptrs->Itb.ByteCount = 0;

        Ptrs->Itb.WrapDataPtr = NULL;
        Ptrs->Itb.WrapByteCount = 0;
    }

    return( ExitCode );
}

BERR_Code BXPT_Rave_UpdateReadOffset(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    size_t CdbByteCount,            /* [in] Number of bytes read. */
    size_t ItbByteCount             /* [in] Number of bytes read. */
    )
{
    uint64_t Read, Valid, Wrap, Base;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Context );

    if( CdbByteCount )
    {
        uint64_t NewRead = 0;
        Read = BREG_ReadAddr( Context->hReg, Context->BaseAddr + CDB_READ_PTR_OFFSET );
        Valid = BREG_ReadAddr( Context->hReg, Context->BaseAddr + CDB_VALID_PTR_OFFSET );
        Wrap = BREG_ReadAddr( Context->hReg, Context->BaseAddr + CDB_WRAP_PTR_OFFSET );
        Base = BREG_ReadAddr( Context->hReg, Context->BaseAddr + CDB_BASE_PTR_OFFSET );

        /*
        ** If this is the first update since the context was reset, we need to do account
        ** for the fact that the first burst of data written into the buffer started at the
        ** BASE pointer, not VALID+1. Thus, the number of bytes we set to the READ pointer
        ** must be one less than the value the user passed in.
        */
        if( Context->CdbReset == true )
        {
            Context->CdbReset = false;
            NewRead = Base + CdbByteCount - 1;
            BREG_WriteAddr( Context->hReg, Context->BaseAddr + CDB_READ_PTR_OFFSET, NewRead );
        }

        /* Check for a wrap-around. Use of Valid is the reccomended way to determine if the buffer has wrapped. */
        else if( Read > Valid )
        {
            /* It did wrap. Will this update move the READ pointer past the wrap point? */
            if( Read + CdbByteCount > Wrap )
            {
                /* Yes, so the READ pointer must 'wrap' too. */
                NewRead = CdbByteCount - ( Wrap - Read ) - 1;
                BREG_WriteAddr( Context->hReg, Context->BaseAddr + CDB_READ_PTR_OFFSET, Base+NewRead );
            }
            else
            {
                /* No, this update only concerns data that didn't wrap. */
                NewRead = Read + CdbByteCount;
                BREG_WriteAddr( Context->hReg, Context->BaseAddr + CDB_READ_PTR_OFFSET, NewRead );
            }
        }

        /* Data didn't wrap. */
        else
        {
            NewRead = Read + CdbByteCount;
            BREG_WriteAddr( Context->hReg, Context->BaseAddr + CDB_READ_PTR_OFFSET, NewRead );
        }
    }

    if( ItbByteCount )
    {
        uint32_t NewRead = 0;
        Read = BREG_ReadAddr( Context->hReg, Context->BaseAddr + ITB_READ_PTR_OFFSET );
        Valid = BREG_ReadAddr( Context->hReg, Context->BaseAddr + ITB_VALID_PTR_OFFSET );
        Wrap = BREG_ReadAddr( Context->hReg, Context->BaseAddr + ITB_WRAP_PTR_OFFSET );
        Base = BREG_ReadAddr( Context->hReg, Context->BaseAddr + ITB_BASE_PTR_OFFSET );

        /* Some users don't allocate an ITB, so check for a non-zero Base address. */
        if( Base )
        {
            /*
            ** If this is the first update since the context was reset, we need to do account
            ** for the fact that the first burst of data written into the buffer started AT the
            ** BASE pointer, not VALID+1. Thus, the number of bytes we add to the READ pointer
            ** must be one less than the value the user passed in.
            */
            if( Context->ItbReset == true )
            {
                Context->ItbReset = false;
                NewRead = Base + ItbByteCount - 1;
                BREG_WriteAddr( Context->hReg, Context->BaseAddr + ITB_READ_PTR_OFFSET, NewRead );
            }

            /* Check for a wrap-around. Use of Valid is the reccomended way to determine if the buffer has wrapped. */
            else if( Read > Valid )
            {
                /* It did wrap. Will this update move the READ pointer past the wrap point? */
                if( Read + ItbByteCount > Wrap )
                {
                    /* Yes, so the READ pointer must 'wrap' too. */
                    NewRead = ItbByteCount - ( Wrap - Read ) - 1;
                    BREG_WriteAddr( Context->hReg, Context->BaseAddr + ITB_READ_PTR_OFFSET, Base + NewRead );
                }
                else
                {
                    /* No, this update only concerns data that didn't wrap. */
                    NewRead = Read + ItbByteCount;
                    BREG_WriteAddr( Context->hReg, Context->BaseAddr + ITB_READ_PTR_OFFSET, NewRead );
                }
            }

            /* Data didn't wrap. */
            else
            {
                NewRead = Read + ItbByteCount;
                BREG_WriteAddr( Context->hReg, Context->BaseAddr + ITB_READ_PTR_OFFSET, NewRead );
            }
        }
    }

    return( ExitCode );
}

static BERR_Code AddPidChannelToContext(
    BXPT_RaveCx_Handle Context,     /* [in] The context  */
    unsigned int PidChanNum,        /* [in] Which PID channel to add. */
    bool UseDecrypted               /* [in] Use decrypted versions of packets on this channel */
    )
{
    uint32_t Reg, RegAddr, PipeShift;
    BXPT_Handle lhXpt;
    BXPT_Rave_Handle lhRave;

    BERR_Code ExitCode = BERR_SUCCESS;

#if defined BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE && defined BXPT_SW7439_115_WORKAROUND
    bool cxAccess = false;
#endif

    /* Sanity check on the arguments. */
    if( PidChanNum >= BXPT_NUM_PID_CHANNELS )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChanNum %lu is out of range!", ( unsigned long ) PidChanNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        BXPT_PidChannelDestination dest;

        BDBG_MSG(( "AddPidChannelToContext: PidChanNum %u, Context %u", PidChanNum, Context->Index ));

        /* For 7002 we need ability to specify pipe for MMSCRAM function */
        /* PR 28674: Force decode contexts to use the R pipe */
        if( Context->Type == BXPT_RaveCx_eAv || Context->Type == BXPT_RaveCx_eAvR )
        {
            UseDecrypted = true;
        }

        PipeShift = UseDecrypted == true ? 5 : 4;
        dest = UseDecrypted ? BXPT_PidChannelDestination_eRaveRPipe : BXPT_PidChannelDestination_eRaveGPipe;
        lhRave = ( BXPT_Rave_Handle ) Context->vhRave;
        lhXpt = ( BXPT_Handle ) lhRave->lvXpt;
        BXPT_P_SetPidChannelDestination( lhXpt, PidChanNum, dest, true );


#ifdef BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE
        /* AV context needs this information for filtering as well. */
        if( Context->Index < CXMEM_LO_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = Context->Index * 2;
        }
        else if( Context->Index < CXMEM_HI_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = ( Context->Index - CXMEM_LO_MAX_CONTEXT ) * 2;
        }
        else
        {
            BDBG_ERR(( "Internal error: PidChanNum %lu is not mapped", ( unsigned long ) PidChanNum ));
            ExitCode = BERR_TRACE( BERR_UNKNOWN );
            goto Done;
        }
#else
        /* AV context needs this information for filtering as well. */
        if( Context->Index < CXMEM_A_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = Context->Index * 2;
        }
    #ifdef BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE
        else if( Context->Index < CXMEM_B_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = ( Context->Index - CXMEM_A_MAX_CONTEXT ) * 2;
        }
    #endif
    #ifdef BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE
        else if( Context->Index < CXMEM_C_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = ( Context->Index - CXMEM_B_MAX_CONTEXT ) * 2;
    #ifdef BXPT_SW7439_115_WORKAROUND
            cxAccess = true;
    #endif
        }
    #endif
        else
        {
            BDBG_ERR(( "Internal error: PidChanNum %lu is not mapped", ( unsigned long ) PidChanNum ));
            ExitCode = BERR_TRACE( BERR_UNKNOWN );
            goto Done;
        }
#endif

        /*
        ** Each PID channel entry in the CXMEM array has two bits for each context:
        ** One bit enables the R pipe, and the other enables the G pipe.
        */
        Reg = BREG_Read32( Context->hReg, RegAddr );
        PipeShift += UseDecrypted == true ? 0 : 1;
        Reg |= ( 1ul << PipeShift );
        BREG_Write32( Context->hReg, RegAddr, Reg );
    #if defined BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE && defined BXPT_SW7439_115_WORKAROUND
        if ( cxAccess )
            BREG_Read32( Context->hReg, BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE );
    #endif
    }

    Done:
    return( ExitCode );
}

BERR_Code EnableScdByContext(
    BXPT_RaveCx_Handle Context,     /* [in] The context  */
    unsigned int PidChanNum       /* [in] Which PID channel to add. */
    )
{
    unsigned WhichScdBlock;
    unsigned Pid;
    bool PidValid;
    BXPT_Rave_Handle lhRave;
    unsigned ScdBlockNum;
    unsigned ScdSlotNum;

    BERR_Code ExitCode = BERR_SUCCESS;

    lhRave = ( BXPT_Rave_Handle ) Context->vhRave;
    for( ScdSlotNum = 0; ScdSlotNum < 8; ScdSlotNum++ )
    {
        GetScdPid( Context, ScdSlotNum, &WhichScdBlock, &Pid, &PidValid );
        if ( false == PidValid )
        {
            break;
        }
    }

    if( ScdSlotNum == 0 || ScdSlotNum == 1)
    {
        ScdBlockNum = WhichScdBlock;
    }
    else if( ScdSlotNum == 8 )
    {
        BDBG_ERR(( "No available SCD slot in this AV context" ));
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        return( ExitCode );
    }
    else /* Extend the case to add to more SCD PID C/D/E/F if the need arise in future. */
    {

        for( ScdBlockNum = 0; ScdBlockNum < BXPT_NUM_SCD; ScdBlockNum++ )
            if( lhRave->ScdTable[ ScdBlockNum ].Allocated == false )
                break;

        if( ScdBlockNum == BXPT_NUM_SCD )
        {
            BDBG_ERR(( "No SCD channel is available for this AV context" ));
            ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
            return( ExitCode );
        }

        lhRave->ScdTable[ ScdBlockNum ].Allocated = true;
        InitScd( lhRave->ScdTable + ScdBlockNum );
    }
    ChangeScdPid( Context, ScdSlotNum, ScdBlockNum, PidChanNum, true );

    return( ExitCode );
}

BERR_Code BXPT_Rave_AddPidChannel(
    BXPT_RaveCx_Handle Context,     /* [in] The context  */
    unsigned int PidChanNum,        /* [in] Which PID channel to add. */
    bool UseDecrypted               /* [in] Use decrypted versions of packets on this channel */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Context );

    ExitCode = AddPidChannelToContext( Context, PidChanNum, UseDecrypted );

    /* AV contexts need the PID channel mapped to their SCDs */
    if( ExitCode == BERR_SUCCESS )
    {
        if( Context->Type == BXPT_RaveCx_eAv || Context->Type == BXPT_RaveCx_eAvR || Context->Type == BXPT_RaveCx_eIp || Context->Type == BXPT_RaveCx_eVctNull )
        {
            ExitCode = EnableScdByContext( Context, PidChanNum );
        }
    }

    return( ExitCode );
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Rave_AddBppChannel(
    BXPT_RaveCx_Handle Context,     /* [in] The context  */
    unsigned int PidChanNum,        /* [in] Which PID channel to add. */
    bool UseDecrypted               /* [in] Use decrypted versions of packets on this channel */
    )
{
    BDBG_ASSERT( Context );

    return AddPidChannelToContext( Context, PidChanNum, UseDecrypted );
}
#endif

BERR_Code BXPT_Rave_RemovePidChannel(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    unsigned int PidChanNum         /* [in] Which PID channel to remove. */
    )
{
    uint32_t Reg, RegAddr, PipeShift;
    BXPT_Rave_Handle lhRave;
    uint32_t PipeInUse;
    unsigned WhichScdBlock;
    unsigned Pid;
    bool PidValid;
    #if defined BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE && defined BXPT_SW7439_115_WORKAROUND
    bool cxAccess=false;
    #endif

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Context );

    /* Sanity check on the arguments. */
    if( PidChanNum >= BXPT_NUM_PID_CHANNELS )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChanNum %lu is out of range!", ( unsigned long ) PidChanNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        BDBG_MSG(( "BXPT_Rave_RemovePidChannel: PidChanNum %u, Context %u", PidChanNum, Context->Index ));

#ifdef BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE
        if( Context->Index < CXMEM_LO_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = Context->Index * 2;
        }
        else if( Context->Index < CXMEM_HI_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = ( Context->Index - CXMEM_LO_MAX_CONTEXT ) * 2;
        }
        else
        {
            BDBG_ERR(( "Internal error: PidChanNum %lu is not mapped", ( unsigned long ) PidChanNum ));
            ExitCode = BERR_TRACE( BERR_UNKNOWN );
            goto Done;
        }
#else
        /* AV context needs this information for filtering as well. */
        if( Context->Index < CXMEM_A_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = Context->Index * 2;
        }
    #ifdef BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE
        else if( Context->Index < CXMEM_B_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = ( Context->Index - CXMEM_A_MAX_CONTEXT ) * 2;
        }
    #endif
    #ifdef BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE
        else if( Context->Index < CXMEM_C_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = ( Context->Index - CXMEM_B_MAX_CONTEXT ) * 2;
    #ifdef BXPT_SW7439_115_WORKAROUND
            cxAccess = true;
    #endif
        }
    #endif
        else
        {
            BDBG_ERR(( "Internal error: PidChanNum %lu is not mapped", ( unsigned long ) PidChanNum ));
            ExitCode = BERR_TRACE( BERR_UNKNOWN );
            goto Done;
        }
#endif

        lhRave = ( BXPT_Rave_Handle ) Context->vhRave;

        /* Clear the PID VALID bit in SCD */
        if( Context->Type == BXPT_RaveCx_eAv || Context->Type == BXPT_RaveCx_eAvR || Context->Type == BXPT_RaveCx_eIp || Context->Type == BXPT_RaveCx_eVctNull )
        {
            unsigned ScdSlotNum;
            for( ScdSlotNum = 0; ScdSlotNum < 8; ScdSlotNum++ )
            {
                GetScdPid( Context, ScdSlotNum, &WhichScdBlock, &Pid, &PidValid );
                if ( Pid == PidChanNum && PidValid )
                {
                    break;
                }
            }

            if ( ScdSlotNum < 8 )
            {
                ChangeScdPid( Context, ScdSlotNum, WhichScdBlock, Pid, false );

                if (ScdSlotNum != 0 )  /* Deallocate SCD slot unless it's slot zero. */
                {
                    lhRave->ScdTable[ WhichScdBlock ].Allocated = false;
                }
            }
        }

        /*
        ** Each PID channel entry in the CXMEM array has two bits for each context:
        ** One bit enables the R pipe, and the other enables the G pipe. When removing
        ** this PID channel, we kill off both pipes.
        */
        Reg = BREG_Read32( Context->hReg, RegAddr );
        PipeInUse = (Reg >> PipeShift) & 0x03;
        Reg &= ~( 0x03 << PipeShift );
        BREG_Write32( Context->hReg, RegAddr, Reg );

    #if defined BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE && defined BXPT_SW7439_115_WORKAROUND
        if ( cxAccess )
            BREG_Read32( Context->hReg, BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE );
    #endif
        /*
        ** Turn off the feed from the SPID table when no other contexts are using
        ** this PID channel.
        */
        ClearSpidTable( Context, PidChanNum, PipeInUse );
    }

    Done:
    return( ExitCode );
}

void ClearSpidTable(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    unsigned int PidChanNum,         /* [in] Which PID channel to remove. */
    uint32_t PipeInUse
    )
{
    BXPT_Rave_Handle lhRave = ( BXPT_Rave_Handle ) Context->vhRave;
    BXPT_Handle lhXpt = ( BXPT_Handle ) lhRave->lvXpt;

#ifdef BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE
    uint32_t RegAddr1;
    uint32_t LoCxMemEnables, HiCxMemEnables;

    RegAddr1 = BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
    LoCxMemEnables = BREG_Read32( Context->hReg, RegAddr1 );
    RegAddr1 = BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
    HiCxMemEnables = BREG_Read32( Context->hReg, RegAddr1 );

    {
        #define R_MAP   0x55555555UL
        #define G_MAP   0xaaaaaaaaUL
        /* Nobody is using this PID channel, so clear the PID channels enable bit. */
        if( PipeInUse == 0x01 )
        {
            if ( !(LoCxMemEnables & R_MAP) && !(HiCxMemEnables & R_MAP) )
            {
                BXPT_P_SetPidChannelDestination( lhXpt, PidChanNum, BXPT_PidChannelDestination_eRaveRPipe, false );
            }
        }
        else
        {
            if( !(LoCxMemEnables & G_MAP) && !(HiCxMemEnables & G_MAP) )
            {
                BXPT_P_SetPidChannelDestination( lhXpt, PidChanNum, BXPT_PidChannelDestination_eRaveGPipe, false );
            }
        }
    }
#else
    uint32_t RegAddr1;
    uint32_t CxMemAEnables, CxMemBEnables, CxMemCEnables;

    RegAddr1 = BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
    CxMemAEnables = BREG_Read32( Context->hReg, RegAddr1 );

    #ifdef BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE
    RegAddr1 = BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
    CxMemBEnables = BREG_Read32( Context->hReg, RegAddr1 );
    #else
    CxMemBEnables = 0;
    #endif

    #ifdef BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE
    RegAddr1 = BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
    CxMemCEnables = BREG_Read32( Context->hReg, RegAddr1 );
    #else
    CxMemCEnables = 0;
    #endif

    lhRave = ( BXPT_Rave_Handle ) Context->vhRave;
    lhXpt = ( BXPT_Handle ) lhRave->lvXpt;
    {
        #define R_MAP   0x55555555UL
        #define G_MAP   0xaaaaaaaaUL
        /* Nobody is using this PID channel, so clear the PID channels enable bit. */
        if( PipeInUse == 0x01 )
        {
            if ( !(CxMemAEnables & R_MAP) && !(CxMemBEnables & R_MAP) && !(CxMemCEnables & R_MAP) )
            {
                BXPT_P_SetPidChannelDestination( lhXpt, PidChanNum, BXPT_PidChannelDestination_eRaveRPipe, false );
            }
        }
        else
        {
            if( !(CxMemAEnables & G_MAP) && !(CxMemBEnables & G_MAP) && !(CxMemCEnables & G_MAP) )
            {
                BXPT_P_SetPidChannelDestination( lhXpt, PidChanNum, BXPT_PidChannelDestination_eRaveGPipe, false );
            }
        }
    }
#endif
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Rave_SetTimestampUserBits(
    BXPT_RaveCx_Handle Ctx,         /* [in] The record context  */
    unsigned int Bits                   /* [in] The new value for the user bits. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Ctx );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_TS_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_REC_TS_CTRL, TS_USER_BITS )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_TS_CTRL, TS_USER_BITS, Bits )
    );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + REC_TS_CTRL_OFFSET, Reg );

    return( ExitCode );
}
#endif

BERR_Code BXPT_Rave_GetRecordStats(
    BXPT_RaveCx_Handle Ctx,         /* [in] The record context  */
    BXPT_Rave_RecordStats *Stats        /* [out] Record context statistics. */
    )
{
    uint32_t Reg;
    uint64_t ticks;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Ctx );
    BDBG_ASSERT( Stats );

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_TIMER_OFFSET );

    /*
    ** The record timer is a 42-bit counter that runs at 108 MHz. The REC_TIMER register only
    ** exposes the 32 MSBs, which means the value increments at 105,468 Hz. That gives a
    ** resolution of 9.48 uSec. The API header advertises that the value returned is in 1.26
    ** uSec steps, so scale appropriately.
    **
    ** To avoid floating-point while retaining as much precision as possible, compute the ratio
    ** of 9.48/1.26 offline and multply it by 10000. That gives us 75238. After doing the run-time
    ** integer multiplication with the register value, divide the result by 10000. That gives us a
    ** result within +/-2 of the floating point version.
    */
    ticks = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_TIMER, REC_TIMER );
    ticks *= 75238;
    ticks /= 10000;
    Stats->ElapsedTime = (uint32_t) (ticks & 0xFFFFFFFF);

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_COUNT_OFFSET );
    Stats->ByteCount = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_COUNT, REC_COUNT );

    return( ExitCode );
}

BERR_Code BXPT_Rave_GetIntId(
    BXPT_RaveCx_Handle ThisCtx,
    BXPT_RaveIntName Name,
    BINT_Id *IntId
    )
{
    uint32_t RegAddr = 0;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( ThisCtx );
    BDBG_ASSERT( BXPT_RaveIntName_eMax != Name );

#if BXPT_HAS_RAVE_L2
    switch( Name )
    {
    #if BXPT_NUM_RAVE_CONTEXTS > 32
        case BXPT_RaveIntName_eEmuErr: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_EMU_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_EMU_ERROR_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
        case BXPT_RaveIntName_ePusiErr: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_PUSI_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_PUSI_ERROR_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
        case BXPT_RaveIntName_eTeiErr: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_TEI_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_TEI_ERROR_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
        case BXPT_RaveIntName_eCcErr: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_CC_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_CC_ERROR_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
        case BXPT_RaveIntName_eCdbOverflow: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_CDB_OVERFLOW_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_CDB_OVERFLOW_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
        case BXPT_RaveIntName_eItbOverflow: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_ITB_OVERFLOW_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_ITB_OVERFLOW_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
        case BXPT_RaveIntName_eSplice: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_SPLICE_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_SPLICE_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
        case BXPT_RaveIntName_eLastCmd: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_LAST_CMD_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_LAST_CMD_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
        case BXPT_RaveIntName_eCdbLowerThresh: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_CDB_LOWER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_CDB_LOWER_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
        case BXPT_RaveIntName_eCdbUpperThresh: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_CDB_UPPER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_CDB_UPPER_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
        case BXPT_RaveIntName_eItbLowerThresh: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_ITB_LOWER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_ITB_LOWER_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
        case BXPT_RaveIntName_eItbUpperThresh: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_ITB_UPPER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_ITB_UPPER_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
        case BXPT_RaveIntName_eCdbMinDepthThresh: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_CDB_MIN_DEPTH_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_CDB_MIN_DEPTH_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
        case BXPT_RaveIntName_eItbMinDepthThresh: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_ITB_MIN_DEPTH_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_ITB_MIN_DEPTH_THRESH_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
    #else
        case BXPT_RaveIntName_eEmuErr: RegAddr = BCHP_XPT_RAVE_EMU_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
        case BXPT_RaveIntName_ePusiErr: RegAddr = BCHP_XPT_RAVE_PUSI_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
        case BXPT_RaveIntName_eTeiErr: RegAddr = BCHP_XPT_RAVE_TEI_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
        case BXPT_RaveIntName_eCcErr: RegAddr = BCHP_XPT_RAVE_CC_ERROR_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
        case BXPT_RaveIntName_eCdbOverflow: RegAddr = BCHP_XPT_RAVE_CDB_OVERFLOW_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
        case BXPT_RaveIntName_eItbOverflow: RegAddr = BCHP_XPT_RAVE_ITB_OVERFLOW_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
        case BXPT_RaveIntName_eSplice: RegAddr = BCHP_XPT_RAVE_SPLICE_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
        case BXPT_RaveIntName_eLastCmd: RegAddr = BCHP_XPT_RAVE_LAST_CMD_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
        case BXPT_RaveIntName_eCdbLowerThresh: RegAddr = BCHP_XPT_RAVE_CDB_LOWER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
        case BXPT_RaveIntName_eCdbUpperThresh: RegAddr = BCHP_XPT_RAVE_CDB_UPPER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
        case BXPT_RaveIntName_eItbLowerThresh: RegAddr = BCHP_XPT_RAVE_ITB_LOWER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
        case BXPT_RaveIntName_eItbUpperThresh: RegAddr = BCHP_XPT_RAVE_ITB_UPPER_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
        case BXPT_RaveIntName_eCdbMinDepthThresh: RegAddr = BCHP_XPT_RAVE_CDB_MIN_DEPTH_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
        case BXPT_RaveIntName_eItbMinDepthThresh: RegAddr = BCHP_XPT_RAVE_ITB_MIN_DEPTH_THRESH_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
    #endif

#if BXPT_NUM_TSIO
    #if BXPT_NUM_RAVE_CONTEXTS > 32
        case BXPT_RaveIntName_eTsioDmaEnd: RegAddr = ThisCtx->Index < 32 ? BCHP_XPT_RAVE_TSIO_DMA_END_CX00_31_L2_INTR_CPU_STATUS_0_31 : BCHP_XPT_RAVE_TSIO_DMA_END_CX32_47_L2_INTR_CPU_STATUS_32_47; break;
    #else
        case BXPT_RaveIntName_eTsioDmaEnd: RegAddr = BCHP_XPT_RAVE_TSIO_DMA_END_CX00_31_L2_INTR_CPU_STATUS_0_31; break;
    #endif
#endif
        default:
        BDBG_ERR(( "Unknown/unsupported interrupt %u", Name ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        break;
    }

    *IntId = BCHP_INT_ID_CREATE( RegAddr, (ThisCtx->Index % 32) );
#else
    RegAddr = BCHP_XPT_RAVE_INT_CX0 + ( ThisCtx->Index * 4 );
    *IntId = BCHP_INT_ID_CREATE( RegAddr, Name );
#endif
    return( ExitCode );
}

BERR_Code BXPT_Rave_PushPidChannel(
    BXPT_RaveCx_Handle hCtx,        /* [in] The context. */
    unsigned int PidChannel,        /* [in] Current PidChannel  */
    unsigned int SplicePidChannel   /* [in] Channel carrying the PID to splice. */
    )
{
    uint32_t WrPtr=0;
    uint32_t SpliceQueueDmemAddr, Reg;
    BXPT_Handle lhXpt;
    BXPT_Rave_Handle lhRave;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hCtx );


    /* Is there a stack for this context? */
    if( hCtx->HaveSpliceQueue == false )
    {
        /* No... */
        /* ... is there one free? */
        if( AllocateSpliceQueue( hCtx, &hCtx->SpliceQueueIdx ) != BERR_SUCCESS )
        {
            /* No, complain and return. */
            BDBG_ERR(( "No splicing stacks available!" ));
            ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
            goto Done;
        }

        /* Found an unused stack. */
        hCtx->HaveSpliceQueue = true;

        SpliceQueueDmemAddr = BCHP_XPT_RAVE_DMEMi_ARRAY_BASE + SPLICE_QUEUE_AREA_OFFSET + ( hCtx->SpliceQueueIdx * SPLICE_QUEUE_AREA_SIZE );
        BREG_Write32( hCtx->hReg, SpliceQueueDmemAddr + SPLICE_CURR_PID_MSB_OFFSET, (PidChannel>>8)&0xFF );
        BREG_Write32( hCtx->hReg, SpliceQueueDmemAddr + SPLICE_CURR_PID_LSB_OFFSET, PidChannel&0xFF );
    }

    /* Either way, we have a splicing stack now */
    if( BXPT_Rave_GetQueueDepth( hCtx ) >= SPLICE_QUEUE_SIZE )
    {
        BDBG_ERR(( "Splicing stack is full. No slots available!" ));
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        goto Done;
    }

    SpliceQueueDmemAddr = BCHP_XPT_RAVE_DMEMi_ARRAY_BASE + SPLICE_QUEUE_AREA_OFFSET + ( hCtx->SpliceQueueIdx * SPLICE_QUEUE_AREA_SIZE );
    WrPtr = BREG_Read32( hCtx->hReg, SpliceQueueDmemAddr + SPLICE_WR_PTR_OFFSET );

    /* If we're pointing at SPLICE_MAX_SLOT, must wrap *before* we store this PID channel */
    if( WrPtr == SPLICE_MAX_SLOT )
        WrPtr = 0;  /* Wrap around to 0 */

    /*
    ** ... write PID channel onto the stack. The WrPtr is actually an index; it must be
    ** multiplied by 4 to make a 32-bit offset into the queue area.
    */
    BREG_Write32( hCtx->hReg, SpliceQueueDmemAddr + ( WrPtr * 4 ) + SPLICE_QUEUE_LOC_0_MSB_OFFSET, (SplicePidChannel>>8)&0xFF );
    BREG_Write32( hCtx->hReg, SpliceQueueDmemAddr + ( WrPtr * 4 ) + SPLICE_QUEUE_LOC_0_LSB_OFFSET, SplicePidChannel&0xFF );
    WrPtr++;

    /* ... update write pointer. Wrap handled above. */
    BREG_Write32( hCtx->hReg, SpliceQueueDmemAddr + SPLICE_WR_PTR_OFFSET, WrPtr );

    /* ... enable splicing in context. */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_PID_STREAM_ID_OFFSET );
    Reg |= BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_PID_STREAM_ID, SPLICE_EN, 1 );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_PID_STREAM_ID_OFFSET, Reg );

    lhRave = ( BXPT_Rave_Handle ) hCtx->vhRave;
    lhXpt = ( BXPT_Handle ) lhRave->lvXpt;
    BXPT_P_SetPidChannelDestination( lhXpt, SplicePidChannel, BXPT_PidChannelDestination_eRaveRPipe, true );
    Done:
    return( ExitCode );
}

BERR_Code BXPT_Rave_ClearQueue(
    BXPT_RaveCx_Handle hCtx         /* [in] The context. */
    )
{
    unsigned Index;
    uint32_t Reg, SpliceQueueDmemAddr;
    BXPT_Rave_Handle hRave;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hCtx );

    hRave = ( BXPT_Rave_Handle ) hCtx->vhRave;

    /* Disable splicing. Rave shouldn't use this stack while we're clearing it. */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_PID_STREAM_ID_OFFSET );
    Reg &= ~BCHP_MASK( XPT_RAVE_CX0_AV_PID_STREAM_ID, SPLICE_EN );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_PID_STREAM_ID_OFFSET, Reg );

    SpliceQueueDmemAddr = BCHP_XPT_RAVE_DMEMi_ARRAY_BASE + SPLICE_QUEUE_AREA_OFFSET + ( hCtx->SpliceQueueIdx * SPLICE_QUEUE_AREA_SIZE );

    /* Clear the stack area. Includes clearing the RD and WR pointers. */
    for( Index = 0; Index < SPLICE_QUEUE_AREA_SIZE; Index += 4 )
        BREG_Write32( hCtx->hReg, SpliceQueueDmemAddr + Index, 0 );

    /* Also deallocate the stack. */
    if(hCtx->HaveSpliceQueue)
        hRave->SpliceQueueAllocated[ hCtx->SpliceQueueIdx ] = false;

    hCtx->HaveSpliceQueue = false;

    return( ExitCode );
}

unsigned BXPT_Rave_GetQueueDepth(
    BXPT_RaveCx_Handle hCtx         /* [in] The context. */
    )
{
    uint32_t RdPtr, WrPtr, SpliceQueueDmemAddr;

    unsigned Depth = 0;

    BDBG_ASSERT( hCtx );

    /* Context might not have a stack allocated to it. */
    if( hCtx->HaveSpliceQueue == false )
        goto Done;

    SpliceQueueDmemAddr = BCHP_XPT_RAVE_DMEMi_ARRAY_BASE + SPLICE_QUEUE_AREA_OFFSET + ( hCtx->SpliceQueueIdx * SPLICE_QUEUE_AREA_SIZE );
    RdPtr = BREG_Read32( hCtx->hReg, SpliceQueueDmemAddr + SPLICE_RD_PTR_OFFSET );
    WrPtr = BREG_Read32( hCtx->hReg, SpliceQueueDmemAddr + SPLICE_WR_PTR_OFFSET );

    /*
    ** The stack slots is fixed at SPLICE_MAX_SLOT + 1. The last slot can be pointed to
    ** but can't actually be used to hold any info. We take this into account when
    ** computing the wraparound, and we need to consider it here as well.
    */
    if( WrPtr >= RdPtr )
        Depth = WrPtr - RdPtr;
    else
        Depth = SPLICE_MAX_SLOT - RdPtr + WrPtr;

    Done:
    return( Depth );
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Rave_GetContextStatus(
    BXPT_RaveCx_Handle hCtx,
    BXPT_RaveCx_Status *CxStatus
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hCtx );
    BDBG_ASSERT( CxStatus );

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG1_OFFSET );
    CxStatus->ContextEnabled = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE ) ? true : false;

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_DEPTH_OFFSET );
    CxStatus->CdbOverflow = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_CDB_DEPTH, CDB_OVERFLOW ) ? true : false;

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + ITB_DEPTH_OFFSET );
    CxStatus->ItbOverflow = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_ITB_DEPTH, ITB_OVERFLOW ) ? true : false;

    return( ExitCode );
}

void BXPT_Rave_ClearOverflow(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t MiscControlReg;
    uint32_t Reg;

    MiscControlReg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG1_OFFSET );

    /* Disable the context while we do this. */
    Reg = MiscControlReg & ~( BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE ) );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG1_OFFSET, Reg );

    /* Clear the overflow bits for both CDB and ITB */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_DEPTH_OFFSET );
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_AV_CDB_DEPTH, CDB_OVERFLOW ) );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_DEPTH_OFFSET, Reg );

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + ITB_DEPTH_OFFSET );
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_AV_ITB_DEPTH, ITB_OVERFLOW ) );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + ITB_DEPTH_OFFSET, Reg );

    /* Put back the original context enable state (whatever that was). */
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG1_OFFSET, MiscControlReg );
}
#endif

static BERR_Code ClearCxHold(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t MyIndexBit, Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    if( hCtx->Index < 24 )
    {
        MyIndexBit = 1UL << hCtx->Index;
        RegAddr = BCHP_XPT_RAVE_CX_HOLD_CLR_STATUS;
    }
#ifdef BCHP_XPT_RAVE_CX_HOLD_CLR_STATUS_1
    else if( hCtx->Index < 48 )
    {
        MyIndexBit = 1UL << (hCtx->Index - 24);
        RegAddr = BCHP_XPT_RAVE_CX_HOLD_CLR_STATUS_1;
    }
#endif
    else
    {
        BDBG_ERR(( "Unsupported RAVE context %u", hCtx->Index ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    Reg = BREG_Read32( hCtx->hReg, RegAddr );
    BREG_Write32( hCtx->hReg, RegAddr, Reg & MyIndexBit );

    Done:
    return ExitCode;
}

BERR_Code BXPT_Rave_FlushContext(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;
    BDBG_MSG(( "BXPT_Rave_FlushContext %u", hCtx->Index ));
    if( hCtx->IsSoftContext )
    {
        hCtx->SoftRave.last_src_itb_valid = hCtx->SoftRave.src_itb_base;
        hCtx->SoftRave.last_dst_itb_valid = hCtx->SoftRave.dest_itb_base;
        hCtx->SoftRave.last_dest_valid = hCtx->SoftRave.last_dst_itb_valid;
        hCtx->SoftRave.last_base_address = BREG_ReadAddr(hCtx->hReg, CDB_BASE_PTR_OFFSET+hCtx->SoftRave.SrcBaseAddr);

        hCtx->SoftRave.flush_cnt = 0;
        hCtx->SoftRave.b_frame_found = false;
        hCtx->SoftRave.insufficient_itb_info = false;
        hCtx->SoftRave.adjust_pts = false;
        hCtx->SoftRave.sequence_hdr_found = false;
        hCtx->SoftRave.prev_sequence_hdr_found = false;
    }

    /* Flush ONLY if the context is disabled. */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG1_OFFSET );
    if( BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE ) )
    {
        BDBG_ERR(( "Can't flush context because it is enabled" ));
        ExitCode = BXPT_ERR_DEVICE_BUSY;
        goto Done;
    }


    ExitCode = ResetContextPointers( hCtx );
    if( ExitCode != BERR_SUCCESS )
        goto Done;

    hCtx->CdbReset = true;
    hCtx->ItbReset = true;
    hCtx->CdbReadReset = true;
    hCtx->ItbReadReset = true;

    if( hCtx->Type == BXPT_RaveCx_eRecord || hCtx->Type == BXPT_RaveCx_eRecordR )
    {
        uint32_t MiscConfig = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET );

        /* Flush record context */
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_TIMER_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_STATE0_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_STATE1_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_STATE2_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_STATE2b_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_STATE3_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_COUNT_OFFSET, 0 );

        /* Flush any TPIT associated with this context */
        if( BCHP_GET_FIELD_DATA( MiscConfig, XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_ENABLE ) )
        {
            /* We've got a TPIT, flush it. */
            FlushTpit( hCtx, BCHP_GET_FIELD_DATA( MiscConfig, XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_CHANNEL ) );
        }
    }

    /* Flushing SCDs is slightly more complex. */
    FlushScds( hCtx );

    /* Flush the picture counter, if there is one for this context */
    if( hCtx->PicCounter != -1 )
        FlushPicCounter( hCtx );

    /* Clear any holds */
    ExitCode = ClearCxHold( hCtx );

    /* SW7335-1434: Clear watermarking status during channel change */
    ClearDataDropOnPusiErrState( hCtx );



    Done:
    return( ExitCode );
}


BERR_Code InitContext(
    BXPT_RaveCx_Handle ThisCtx,
    const BAVC_CdbItbConfig *BufferCfg
    )
{
    BMMA_DeviceOffset BufferOffset;
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;
    unsigned ItbSize = 0;
    unsigned ItbAlignment = 7;
    unsigned CdbAlignment = 8;

    if( BufferCfg )
    {
        ItbAlignment = BufferCfg->Itb.Alignment;
        CdbAlignment = BufferCfg->Cdb.Alignment;
    }

    /* Clear registers / pointer memory */
    clearContextRegs(ThisCtx->hReg, ThisCtx->Index);

    /* enable full TS packet capture using reserved register. for 28nm, unconditionally enabled */
    Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_MISC_CFG5_OFFSET);
    Reg |= 1<<9;
    BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_MISC_CFG5_OFFSET, Reg );

    /* Set the context type (AV or REC) */
    Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + REC_MISC_CFG_OFFSET );
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, REC_AVN ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, REC_AVN,
        ThisCtx->Type == BXPT_RaveCx_eRecord || ThisCtx->Type == BXPT_RaveCx_eRecordR ? 1 : 0 ) );
    BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

    if( BufferCfg )
    {
        bool ItbEndian, CdbEndian, SysIsBigEndian;

#if (BCHP_CHIP==7145 || BCHP_CHIP==7364 || BCHP_CHIP == 7250 || (BCHP_CHIP==7439 && BCHP_VER >= BCHP_VER_B0)) || (BCHP_CHIP == 7271) || (BCHP_CHIP == 7268) || (BCHP_CHIP == 7260) || (BCHP_CHIP == 7278) || (BCHP_CHIP == 7255)
        BSTD_UNUSED( SysIsBigEndian );

        /* Config the CDB */
        if( BufferCfg->Cdb.LittleEndian == true )
        {
            /* They want little endian... */
            CdbEndian = false;  /* ...and chip is little endian. Don't swap */
        }
        else
        {
            /* They want big endian... */
            CdbEndian = true;   /* ...and chip is little endian. So swap */
        }
#else
        Reg = BREG_Read32( ThisCtx->hReg, BCHP_SUN_TOP_CTRL_STRAP_VALUE_0 );
        SysIsBigEndian = BCHP_GET_FIELD_DATA( Reg, SUN_TOP_CTRL_STRAP_VALUE_0, strap_system_big_endian );

        /* Config the CDB */
        if( BufferCfg->Cdb.LittleEndian == true )
        {
            /* They want little endian... */
            if( SysIsBigEndian )
                CdbEndian = true;   /* ...and chip is big endian. So swap */
            else
                CdbEndian = false;  /* ...and chip is little endian. Don't swap */
        }
        else
        {
            /* They want big endian... */
            if( SysIsBigEndian )
                CdbEndian = false;  /* ...and chip is big endian. Don't swap */
            else
                CdbEndian = true;   /* ...and chip is little endian. So swap */
        }
#endif

        /* Config the ITB - this is not a swap setting. It simply sets the endianness. */
        ItbEndian = BufferCfg->Itb.LittleEndian;

        if (!ThisCtx->externalCdbAlloc) /* XPT does the alloc */
        {
            BDBG_ASSERT(!ThisCtx->mma.cdbBlock);

            /* Special alignment requirements for the decoders. */
            if( ThisCtx->Type == BXPT_RaveCx_eAv || ThisCtx->Type == BXPT_RaveCx_eAvR )
            {
                ItbAlignment = 7;   /* 128-byte alignment */
                CdbAlignment = 8;   /* 256-byte alignment */
            }

            /* Create the CDB. The CDB might be allocated in the R area. */
            switch( ThisCtx->Type )
            {
                case BXPT_RaveCx_eAvR:
                case BXPT_RaveCx_eRecordR:
                /* Decode or record from secure memory. Must use the secure heap */
                if( !ThisCtx->mmaRHeap )
                {
                    BDBG_ERR(( "Decode or record from secure memory is requested, but no secure heap is available." ));
                    ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
                    goto done;
                }
                ThisCtx->mma.cdbBlock = BMMA_Alloc(ThisCtx->mmaRHeap, BufferCfg->Cdb.Length, 1 << CdbAlignment, NULL);
                break;

                case BXPT_RaveCx_eAv:
                case BXPT_RaveCx_eRecord:
                default:
                ThisCtx->mma.cdbBlock = BMMA_Alloc(ThisCtx->mmaHeap, BufferCfg->Cdb.Length, 1 << CdbAlignment, NULL);
                break;
            }

            if (!ThisCtx->mma.cdbBlock)
            {
                ThisCtx->allocatedCdbBufferSize = 0;
                ThisCtx->usedCdbBufferSize = 0;
                BDBG_ERR(( "CDB alloc failed!" ));
                ExitCode = BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
                goto done;
            }
        }
        else /* the app has already done the alloc */
        {
            BDBG_ASSERT(ThisCtx->mma.cdbBlock);
        }

        ThisCtx->allocatedCdbBufferSize = BufferCfg->Cdb.Length;
        ThisCtx->usedCdbBufferSize = BufferCfg->Cdb.Length;
        ThisCtx->mma.cdbPtr = (uint8_t*)BMMA_Lock(ThisCtx->mma.cdbBlock) + ThisCtx->mma.cdbBlockOffset;
        ThisCtx->mma.cdbOffset = BufferOffset = BMMA_LockOffset(ThisCtx->mma.cdbBlock) + ThisCtx->mma.cdbBlockOffset;

        BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_BASE_PTR_OFFSET, BufferOffset );
        BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_WRITE_PTR_OFFSET, BufferOffset );
        BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_VALID_PTR_OFFSET, BufferOffset );
        BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_READ_PTR_OFFSET, BufferOffset );
        BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_END_PTR_OFFSET, BufferOffset + BufferCfg->Cdb.Length - 1 );
        BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_WRAP_PTR_OFFSET, 0 );

#if BXPT_HAS_HEVD_WORKAROUND
        /* Don't do anything to the CDB for video, but update wrap for audio */
        /* BXPT_Rave_SetAvConfig() will come along later and possibly set this flag. */
        if(!ThisCtx->isHevdVideoContext)
            BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_WRAP_PTR_OFFSET, 0 );
#else
        BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_WRAP_PTR_OFFSET, 0 );
#endif

        /* In some cases, the user does not want an ITB. */
        if( BufferCfg->Itb.Length )
        {
            if (!ThisCtx->externalItbAlloc) /* XPT does the alloc */
            {

                ItbSize = BufferCfg->Itb.Length;
                ThisCtx->mma.itbBlock = BMMA_Alloc(ThisCtx->mmaHeap, ItbSize, 1 << ItbAlignment, NULL);
                if (!ThisCtx->mma.itbBlock) {
                    BDBG_ERR(("ITB alloc failed!"));
                    goto error;
                }
            }
            else /* the app has already done the alloc */
            {
                ItbSize = BufferCfg->Itb.Length;
            }

            ThisCtx->mma.itbPtr = (uint8_t*)BMMA_Lock(ThisCtx->mma.itbBlock) + ThisCtx->mma.itbBlockOffset;
            ThisCtx->mma.itbOffset = BufferOffset = BMMA_LockOffset(ThisCtx->mma.itbBlock) + ThisCtx->mma.itbBlockOffset;

            BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_WRITE_PTR_OFFSET, BufferOffset );
            BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_READ_PTR_OFFSET, BufferOffset );
            BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_BASE_PTR_OFFSET, BufferOffset );
            BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_END_PTR_OFFSET, BufferOffset + ItbSize - 1 );
            BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_VALID_PTR_OFFSET, BufferOffset );
            BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_WRAP_PTR_OFFSET, 0 );

           Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_THRESHOLDS_OFFSET );
           Reg &= ~(
               BCHP_MASK( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_OVERFLOW_THRESHOLD ) |
               BCHP_MASK( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_WRAPAROUND_THRESHOLD )
           );
           Reg |= (
               BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_OVERFLOW_THRESHOLD, BXPT_RAVE_OVERFLOW_THRESH ) |
               BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_WRAPAROUND_THRESHOLD, BXPT_RAVE_WRAP_THRESH )
           );
           BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_THRESHOLDS_OFFSET, Reg );
        }
        else
        {
            Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_THRESHOLDS_OFFSET );
            Reg &= ~(
                BCHP_MASK( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_OVERFLOW_THRESHOLD ) |
                BCHP_MASK( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_WRAPAROUND_THRESHOLD )
            );
            Reg |= (
                BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_OVERFLOW_THRESHOLD, 0 ) |
                BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_WRAPAROUND_THRESHOLD, BXPT_RAVE_WRAP_THRESH )
            );
            BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_THRESHOLDS_OFFSET, Reg );
        }

        Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_MISC_CFG2_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, ITB_ENDIAN_CTRL ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, CDB_ENDIAN_CTRL )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, ITB_ENDIAN_CTRL, ItbEndian == true ? 1 : 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, CDB_ENDIAN_CTRL, CdbEndian == true ? 1 : 0 )
        );
        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_MISC_CFG2_OFFSET, Reg );
    }

    BDBG_MSG(( "Context %u: Disabling RUN VERSION checking", ThisCtx->Index ));
    Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_MISC_CFG2_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RUN_VERSION ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, ENABLE_RUN_VERSION )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RUN_VERSION, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, ENABLE_RUN_VERSION, 0 )
    );
    BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + AV_MISC_CFG2_OFFSET, Reg );

    if( ThisCtx->Type == BXPT_RaveCx_eAv || ThisCtx->Type == BXPT_RaveCx_eAvR
        || ThisCtx->Type == BXPT_RaveCx_eRecord || ThisCtx->Type == BXPT_RaveCx_eRecordR
    )
    {
        if( BufferCfg )
        {
            BXPT_Rave_ContextThresholds Thresholds;
            BKNI_Memset( (void *) &Thresholds, 0, sizeof(BXPT_Rave_ContextThresholds));

            BXPT_Rave_ComputeThresholds( ThisCtx, BufferCfg->Cdb.Length, ItbSize, &Thresholds );
            ExitCode = BXPT_Rave_SetThresholds( ThisCtx, &Thresholds );
        }
    }

    ThisCtx->CdbReset = true;
    ThisCtx->ItbReset = true;
    ThisCtx->CdbReadReset = true;
    ThisCtx->ItbReadReset = true;
    ThisCtx->Transcoding = false;
    ThisCtx->ScOrMode = true;
    ClearDataDropOnPusiErrState( ThisCtx );

done:
    return( ExitCode );

error:
    if (ThisCtx->mma.cdbBlock) {
        BMMA_UnlockOffset(ThisCtx->mma.cdbBlock, ThisCtx->mma.cdbOffset);
        BMMA_Unlock(ThisCtx->mma.cdbBlock, ThisCtx->mma.cdbPtr);
        if (!ThisCtx->externalCdbAlloc) {
            BMMA_Free(ThisCtx->mma.cdbBlock);
        }
    }
    if (ThisCtx->mma.itbBlock) {
        if (ThisCtx->mma.itbOffset) {
            BMMA_UnlockOffset(ThisCtx->mma.itbBlock, ThisCtx->mma.itbOffset);
            BMMA_Unlock(ThisCtx->mma.itbBlock, ThisCtx->mma.itbPtr);
        }
        if (!ThisCtx->externalItbAlloc) {
            BMMA_Free(ThisCtx->mma.itbBlock);
        }
    }
    return ExitCode;
}

BERR_Code ClearMem(
    BXPT_Rave_Handle hRave
    )
{
    uint32_t Offset;

    BERR_Code ExitCode = BERR_SUCCESS;

    /* Clear context memory */
#ifdef BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE
    for( Offset = BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_START; Offset <= BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_END; Offset++ )
        BREG_Write32( hRave->hReg, BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE + ( Offset * 4 ), 0 );
    for( Offset = BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_START; Offset <= BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_END; Offset++ )
        BREG_Write32( hRave->hReg, BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE + ( Offset * 4 ), 0 );
#else
    for( Offset = BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_START; Offset <= BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_END; Offset++ )
        BREG_Write32( hRave->hReg, BCHP_XPT_RAVE_CXMEM_Ai_ARRAY_BASE + ( Offset * 4 ), 0 );
    #ifdef BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_START
    for( Offset = BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_START; Offset <= BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_END; Offset++ )
        BREG_Write32( hRave->hReg, BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE + ( Offset * 4 ), 0 );
    #endif

    #ifdef BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_START
        for( Offset = BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_START; Offset <= BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_END; Offset++ )
            BREG_Write32( hRave->hReg, BCHP_XPT_RAVE_CXMEM_Ci_ARRAY_BASE + ( Offset * 4 ), 0 );
#if defined(BXPT_SW7439_115_WORKAROUND) && defined(BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_START)
        BREG_Read32( hRave->hReg, BCHP_XPT_RAVE_CXMEM_Bi_ARRAY_BASE );
    #endif
    #endif
#endif

    /* Clear data memory */
    for( Offset = BCHP_XPT_RAVE_DMEMi_ARRAY_START; Offset <= BCHP_XPT_RAVE_DMEMi_ARRAY_END; Offset++ )
        BREG_Write32( hRave->hReg, BCHP_XPT_RAVE_DMEMi_ARRAY_BASE + ( Offset * 4 ), 0 );

    return( ExitCode );
}

BERR_Code BXPT_P_RaveRamInit(
    BXPT_Rave_Handle hRave
    )
{
    uint32_t Offset;
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

#if BXPT_HAS_RAVE_AUTO_READ
    /*
    ** NOTE: When this option is enabled, bxpt.inc will redirect the build to use the AutoRead ihex image file.
    ** Therefore, no changes are needed in the function.
    */
    BDBG_WRN(( "Using AutoRead ihex" ));
#endif

    /* Unlock IMEM */
    BREG_Write32( hRave->hReg, BCHP_XPT_RAVE_XPU_IMEM_ACCESS_CONTROL, 0x27182818 );
    Reg = BREG_Read32( hRave->hReg, BCHP_XPT_RAVE_XPU_IMEM_ACCESS_STATUS );

    if( BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_XPU_IMEM_ACCESS_STATUS, IMEM_LOCK_STAT) )
    {
        BDBG_ERR(( "RAVE IMEM LOCKED" ));
        BDBG_ASSERT(0);
    }


    for( Offset = 0; Offset < BxptRaveInitDataSize; Offset++ )
        BREG_Write32( hRave->hReg, RAVE_CFG_ARRAY_BASE + ( Offset * 4 ), BxptRaveInitData[ Offset ] );

    if ( hRave->chanOpenCB )
    {
        ExitCode = (hRave->chanOpenCB)();
        if (ExitCode != BERR_SUCCESS )
            return ExitCode;
    }

    BREG_Write32( hRave->hReg, RAVE_CFG_MISC_REG, 0 );

    return( ExitCode );
}

#define SCD_REG_SIZE    (BCHP_XPT_RAVE_SCD0_SCD_COMP_STATE1 - BCHP_XPT_RAVE_SCD0_SCD_COMP_STATE0)

BERR_Code InitScd(
    StartCodeIndexer *lhScd
    )
{
    uint32_t Offset, Reg, CDMemBase;

    BERR_Code ExitCode = BERR_SUCCESS;

    for( Offset = 0; Offset < SCD_REG_STEP; Offset += SCD_REG_SIZE )
        BREG_Write32( lhScd->hReg, lhScd->BaseAddr + Offset, 0 );

    /*
    ** Set base address of the sub-context data. BaseAddr is already pointing to
    ** the correct address, since we set it above.
    */
    CDMemBase = SCD_DMEM_BASE + ( lhScd->ChannelNo * SCD_DMEM_SIZE );
    Reg = BREG_Read32( lhScd->hReg, lhScd->BaseAddr );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_SCD0_SCD_MISC_CONFIG, CONTEXT_DMEM_BASE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_SCD0_SCD_MISC_CONFIG, CONTEXT_DMEM_BASE, CDMemBase )
    );
    BREG_Write32( lhScd->hReg, lhScd->BaseAddr, Reg );

    return( ExitCode );
}

#define TPIT_REG_STEP   (4)

BERR_Code InitTpit(
    TpitIndexer *hTpit
    )
{
    uint32_t Offset;

    BERR_Code ExitCode = BERR_SUCCESS;

    for( Offset = 0; Offset < TPIT_PID_TABLE_SIZE; Offset += TPIT_REG_STEP )
        BREG_Write32( hTpit->hReg, hTpit->PidTableBaseAddr + Offset, 0 );

    for( Offset = 0; Offset < TPIT_PARSE_TABLE_SIZE; Offset += TPIT_REG_STEP )
        BREG_Write32( hTpit->hReg, hTpit->ParseTableBaseAddr + Offset, 0 );

    for( Offset = 0; Offset < TPIT_CTRL_REG_STEP; Offset += TPIT_REG_STEP )
        BREG_Write32( hTpit->hReg, hTpit->BaseAddr + Offset, 0 );

    return( ExitCode );
}

BERR_Code LoadScRanges(
    BXPT_RaveCx_Handle Context,         /* [in] The context. */
    const BXPT_Rave_EsRanges *EsRanges
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_D, COMP1_RANGED_HI ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_C, COMP1_RANGEC_HI ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_B, COMP1_RANGEB_HI ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_A, COMP1_RANGEA_HI )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_D, COMP1_RANGED_HI, EsRanges[ 3 ].RangeHi ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_C, COMP1_RANGEC_HI, EsRanges[ 2 ].RangeHi ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_B, COMP1_RANGEB_HI, EsRanges[ 1 ].RangeHi ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_A, COMP1_RANGEA_HI, EsRanges[ 0 ].RangeHi )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, Reg );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_D, COMP1_RANGED_LO ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_C, COMP1_RANGEC_LO ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_B, COMP1_RANGEB_LO ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_A, COMP1_RANGEA_LO )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_D, COMP1_RANGED_LO, EsRanges[ 3 ].RangeLo ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_C, COMP1_RANGEC_LO, EsRanges[ 2 ].RangeLo ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_B, COMP1_RANGEB_LO, EsRanges[ 1 ].RangeLo ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_A, COMP1_RANGEA_LO, EsRanges[ 0 ].RangeLo )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, Reg );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_D ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_D ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_C ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_C ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_B ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_B ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_A ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_A )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_D, EsRanges[ 3 ].Enable ? ( EsRanges[ 3 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_D, 3 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_C, EsRanges[ 2 ].Enable ? ( EsRanges[ 2 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_C, 3 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_B, EsRanges[ 1 ].Enable ? ( EsRanges[ 1 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_B, 3 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_A, EsRanges[ 0 ].Enable ? ( EsRanges[ 0 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_OFFSET_A, 3 )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET, Reg );
    return( ExitCode );
}

BERR_Code GetScRanges(
    BXPT_RaveCx_Handle Context,         /* [in] The context. */
    BXPT_Rave_EsRanges *EsRanges
    )
{
    uint32_t Reg;
    unsigned Mode;

    BERR_Code ExitCode = BERR_SUCCESS;

    BKNI_Memset( (void *) EsRanges, 0, sizeof( BXPT_Rave_EsRanges ) );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET );
    EsRanges[ 3 ].RangeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_D, COMP1_RANGED_HI );
    EsRanges[ 2 ].RangeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_C, COMP1_RANGEC_HI );
    EsRanges[ 1 ].RangeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_B, COMP1_RANGEB_HI );
    EsRanges[ 0 ].RangeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_VAL_Inclusion_Range_A, COMP1_RANGEA_HI );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET );
    EsRanges[ 3 ].RangeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_D, COMP1_RANGED_LO );
    EsRanges[ 2 ].RangeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_C, COMP1_RANGEC_LO );
    EsRanges[ 1 ].RangeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_B, COMP1_RANGEB_LO );
    EsRanges[ 0 ].RangeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP1_FILTER_MASK_VAL_Inclusion_Range_A, COMP1_RANGEA_LO );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
    Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_D );
    GetScEnables( EsRanges + 3, Mode );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
    Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_C );
    GetScEnables( EsRanges + 2, Mode );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
    Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_B );
    GetScEnables( EsRanges + 1, Mode );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
    Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_MODE, COMP1_FILT_FUNC_A );
    GetScEnables( EsRanges, Mode );

    return( ExitCode );
}

BERR_Code ConfigureComparators(
    BXPT_RaveCx_Handle hCtx,
    BAVC_ItbEsType StreamType
    )
{
    uint32_t Reg;

    unsigned Comp1NumCompareBytes = 0;
    unsigned Comp1RepeatByte = 0;
    unsigned Comp1En = 0;
    unsigned Comp1ExtractStartByte = 0;
    unsigned Comp1ExtractStartBit = 0;
    unsigned Comp1ExtractNumBits = 0;
    unsigned Comp2ExtractNumBits = 0;
    unsigned Comp2ExtractStartBit = 0;
    unsigned Comp2ExtractStartByte = 0;
    unsigned Comp2En = 0;
    unsigned Comp2RepeatByte = 0;
    unsigned Comp2NumCompareBytes = 0;
    unsigned Comp2ValidByteEn = 0;

    unsigned Comp1PesHdrData = 0;
    unsigned Comp1EsData = 1;

    unsigned Comp1CompareVal0 = 0xFF;
    unsigned Comp1CompareVal1 = 0xFF;
    unsigned Comp1CompareVal2 = 0xFF;
    unsigned Comp1CompareVal3 = 0xFF;
    unsigned Comp1MaskVal0 = 0xFF;
    unsigned Comp1MaskVal1 = 0xFF;
    unsigned Comp1MaskVal2 = 0xFF;
    unsigned Comp1MaskVal3 = 0xFF;

    BERR_Code ExitCode = BERR_SUCCESS;

    switch( StreamType )
    {
        case BAVC_ItbEsType_eMpeg2Video:
        case BAVC_ItbEsType_eAvcVideo:
        case BAVC_ItbEsType_eAvsVideo:

        /*
        ** These three cases are suspect. The old ITB configuration
        ** handled them identically to AVC and MPEG2 video. May
        ** not be true when using the unified ITB code.
        */
        case BAVC_ItbEsType_eMpeg1Video:
        case BAVC_ItbEsType_eVc1Video:
        case BAVC_ItbEsType_eMpeg4Part2:

        Comp1ExtractNumBits = 7;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
        Comp1En = 1;
        Comp1NumCompareBytes = 2;

        Comp1RepeatByte = 4;
        Comp1CompareVal0 = 0;
        Comp1CompareVal1 = 0;
        Comp1CompareVal2 = 0x01;
        Comp1CompareVal3 = 0;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0xFF;
        Comp1MaskVal3 = 0;

        Comp2ExtractNumBits = 7;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 1;
        Comp2En = 0;
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 1;
        break;

        case BAVC_ItbEsType_eOTFVideo:
        Comp1ExtractNumBits = 71;
        Comp1RepeatByte = 4;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
        Comp1En = 1;
        Comp1NumCompareBytes = 2;

        Comp1CompareVal0 = 0;
        Comp1CompareVal1 = 0;
        Comp1CompareVal2 = 0x01;
        Comp1CompareVal3 = 0;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0xFF;
        Comp1MaskVal3 = 0;

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 1;
        Comp2En = 0;
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 1;
        break;

        case BAVC_ItbEsType_eH263:
        Comp1ExtractNumBits = 7;
        Comp1ExtractStartBit = 1;
        Comp1ExtractStartByte = 0;
        Comp1En = 1;
        Comp1NumCompareBytes = 2;

        Comp1RepeatByte = 4;
        Comp1CompareVal0 = 0;
        Comp1CompareVal1 = 0;
        Comp1CompareVal2 = 0x80;
        Comp1CompareVal3 = 0;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0x80;
        Comp1MaskVal3 = 0;

        Comp2ExtractNumBits = 7;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 1;
        Comp2En = 0;
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 1;
        break;

        /* For VC1 Simple/Main, just extract PTS values. There aren't any startcodes. */
        /* AMR support needs just the PTSs from the stream. Raptor will do its own CDB-based framesync. */
        case BAVC_ItbEsType_eVC1SimpleMain:
        case BAVC_ItbEsType_eAmr:
        Comp1En = 0;
        Comp2En = 0;
        break;

        case BAVC_ItbEsType_eVp6Video:
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
        Comp1En = 1;
        Comp1NumCompareBytes = 3;

        Comp1RepeatByte = 4;
        Comp1CompareVal0 = 0x42;
        Comp1CompareVal1 = 0x43;
        Comp1CompareVal2 = 0x4d;
        Comp1CompareVal3 = 0x56;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0xFF;
        Comp1MaskVal3 = 0xFF;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0xffffffff );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 1;
        Comp2En = 0;
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 1;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0xffffffff );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET,    0xffffffff );
        break;

        case BAVC_ItbEsType_eAc3gAudio:
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 3;
        Comp1RepeatByte = 0;
        Comp1NumCompareBytes = 1;

        Comp1CompareVal0 = 0x0B;
        Comp1CompareVal1 = 0x77;
        Comp1CompareVal2 = 0x00;
        Comp1CompareVal3 = 0;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0;
        Comp1MaskVal3 = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 1;
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        break;

        case BAVC_ItbEsType_eMpegAudio:
        case BAVC_ItbEsType_eMpegAudio2_5:
        case BAVC_ItbEsType_eMpegAudioLayer3:
        /* Raptor is now using CDB-based framesync all chips except 3563 */
        Comp1En = 0;
        Comp2En = 0;
        break;

        case BAVC_ItbEsType_eAc3Plus:
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
        Comp1RepeatByte = 0;
        Comp1NumCompareBytes = 1;

        Comp1CompareVal0 = 0x0B;
        Comp1CompareVal1 = 0x77;
        Comp1CompareVal2 = 0x00;
        Comp1CompareVal3 = 0x00;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0x00;
        Comp1MaskVal3 = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 1;
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        break;

        case BAVC_ItbEsType_eAacAudio:
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
        Comp1RepeatByte = 2;
        Comp1NumCompareBytes = 2;

        Comp1CompareVal0 = 0;
        Comp1CompareVal1 = 0;
        Comp1CompareVal2 = 0x01;
        Comp1CompareVal3 = 0x39;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0xFF;
        Comp1MaskVal3 = 0xFF;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 0;
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 1;
        Comp2ValidByteEn = 1;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0xFFF90000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET,    0xFFF60000 );
        break;

        case BAVC_ItbEsType_eAacHe:
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 0;
        Comp1RepeatByte = 0;
        Comp1NumCompareBytes = 1;

        Comp1CompareVal0 = 0x56;
        Comp1CompareVal1 = 0xE0;
        Comp1CompareVal2 = 0x00;
        Comp1CompareVal3 = 0;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xE0;
        Comp1MaskVal2 = 0x00;
        Comp1MaskVal3 = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 0;
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 1;
        Comp2ValidByteEn = 1;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0xFFF90000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET,    0xFFF60000 );
        break;

        case BAVC_ItbEsType_eAvsAudio:
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
        Comp1En = 0;
        Comp1RepeatByte = 4;
        Comp1NumCompareBytes = 2;

        Comp1CompareVal0 = 0xFF;
        Comp1CompareVal1 = 0xFF;
        Comp1CompareVal2 = 0xFF;
        Comp1CompareVal3 = 0xFF;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0xFF;
        Comp1MaskVal3 = 0xFF;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 0;
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 1;
        Comp2ValidByteEn = 1;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        break;

        case BAVC_ItbEsType_eWma:
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
        Comp1RepeatByte = 0;
        Comp1NumCompareBytes = 3;

        Comp1CompareVal0 = 0x42;
        Comp1CompareVal1 = 0x43;
        Comp1CompareVal2 = 0x4D;
        Comp1CompareVal3 = 0x41;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0xFF;
        Comp1MaskVal3 = 0xFF;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 1;
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 3;
        Comp2ValidByteEn = 1;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        break;

        case BAVC_ItbEsType_eLpcmAudio:
        case BAVC_ItbEsType_eMlpAudio:
        case BAVC_ItbEsType_eDtsAudio:
        /* diasable both the comparators */
        Comp1En = 0;
        Comp2En = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        break;

        case BAVC_ItbEsType_eMpegAudioWithDescriptor:
        case BAVC_ItbEsType_eAudioDescriptor:
        Comp1ExtractNumBits = 0x3F;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
        Comp1En = 1;

        Comp1RepeatByte = 0;
        Comp1NumCompareBytes = 3;
        Comp1PesHdrData = 1;
        Comp1EsData = 0;

        Comp1CompareVal0 = 0x44;
        Comp1CompareVal1 = 0x54;
        Comp1CompareVal2 = 0x47;
        Comp1CompareVal3 = 0x41;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0xFF;
        Comp1MaskVal3 = 0xFF;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );

        Comp2ExtractNumBits = 0;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 0;
        Comp2En = 0;
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 0;
        Comp2ValidByteEn = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0 );
        break;

        case BAVC_ItbEsType_eDra:
        Comp1ExtractNumBits = 0x3F;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;

        Comp1RepeatByte = 0;
        Comp1NumCompareBytes = 1;
        Comp1PesHdrData = 0;
        Comp1EsData = 1;

        Comp2ExtractNumBits = 0;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 0;
        Comp2En = 0;
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 0;
        Comp2ValidByteEn = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0 );
        break;

        default:
        BDBG_ERR(( "Unsupported BAVC_ItbEsType!" ));
        break;
    }

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, ALIGNMENT_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ES_DATA ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_PES_HDR_DATA ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ADAPTATION_FIELD ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ALL_DATA ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, CASCADE_ENABLE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, VALID_BYTE_ENABLE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, NUM_COMPARE_BYTES ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, REPEAT_BYTE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMP_ENABLE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_START_BYTE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_START_BIT ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_NUM_BITS )
    );

    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, ALIGNMENT_EN, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ES_DATA, Comp1EsData ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_PES_HDR_DATA, Comp1PesHdrData ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ADAPTATION_FIELD, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ALL_DATA, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, CASCADE_ENABLE, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, VALID_BYTE_ENABLE, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, NUM_COMPARE_BYTES, Comp1NumCompareBytes ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, REPEAT_BYTE, Comp1RepeatByte ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMP_ENABLE, Comp1En ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_START_BYTE, Comp1ExtractStartByte ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_START_BIT, Comp1ExtractStartBit ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_NUM_BITS, Comp1ExtractNumBits )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_CTRL_OFFSET, Reg );

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, ALIGNMENT_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_ES_DATA ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_PES_HDR_DATA ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_ADAPTATION_FIELD ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_ALL_DATA ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, VALID_BYTE_ENABLE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, NUM_COMPARE_BYTES ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, REPEAT_BYTE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMP_ENABLE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_START_BYTE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_START_BIT ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_NUM_BITS )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, ALIGNMENT_EN, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_ES_DATA, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_PES_HDR_DATA, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_ADAPTATION_FIELD, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMPARE_ALL_DATA, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, VALID_BYTE_ENABLE, Comp2ValidByteEn ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, NUM_COMPARE_BYTES, Comp2NumCompareBytes ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, REPEAT_BYTE, Comp2RepeatByte ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, COMP_ENABLE, Comp2En ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_START_BYTE, Comp2ExtractStartByte ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_START_BIT, Comp2ExtractStartBit ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_NUM_BITS, Comp2ExtractNumBits )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_CTRL_OFFSET, Reg );

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_VAL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_0 ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_1 ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_2 ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_3 )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_0, Comp1CompareVal0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_1, Comp1CompareVal1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_2, Comp1CompareVal2 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_COMPARE_VAL, COMP1_COMPARE_VAL_3, Comp1CompareVal3 )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_VAL_OFFSET, Reg );

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_VAL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_0 ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_1 ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_2 ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_3 )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_0, Comp1MaskVal0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_1, Comp1MaskVal1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_2, Comp1MaskVal2 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_MASK_VAL, COMP1_MASK_VAL_3, Comp1MaskVal3 )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_VAL_OFFSET, Reg );

    return( ExitCode );
}

BERR_Code AllocateSpliceQueue(
    BXPT_RaveCx_Handle hCtx,
    unsigned *QueueNum
    )
{
    unsigned Index;
    uint32_t SpliceQueueDmemPtr;

    BERR_Code ExitCode = BERR_SUCCESS;
    BXPT_Rave_Handle hRave = ( BXPT_Rave_Handle ) hCtx->vhRave;

    for( Index = 0; Index < BXPT_P_NUM_SPLICING_QUEUES; Index++ )
        if( hRave->SpliceQueueAllocated[ Index ] == false )
            break;

    if( Index == BXPT_P_NUM_SPLICING_QUEUES )
    {
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        goto Done;
    }

    hRave->SpliceQueueAllocated[ Index ] = true;
    *QueueNum = Index;

    /* Get a pointer to the context-specific memory area for this context */
    SpliceQueueDmemPtr = SCD_DMEM_BASE + ( hCtx->Index * SCD_DMEM_SIZE ) + SPLICE_QUEUE_PTR_OFFSET;
    BREG_Write32( hCtx->hReg, BCHP_XPT_RAVE_DMEMi_ARRAY_BASE + (SpliceQueueDmemPtr * 4), Index );

    /* Now clear the stack */
    BXPT_Rave_ClearQueue( hCtx );

    Done:
    return( ExitCode );
}

BERR_Code ResetContextPointers(
    BXPT_RaveCx_Handle hCtx
    )
{
    BMMA_DeviceOffset Offset;

    BERR_Code ExitCode = BERR_SUCCESS;

    Offset = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET );
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_WRITE_PTR_OFFSET, Offset );
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_READ_PTR_OFFSET, Offset );
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_VALID_PTR_OFFSET, Offset );

#if BXPT_HAS_HEVD_WORKAROUND
    /* Do not alter the CDB wrap for video contexts. Allow for audio. */
    if(!hCtx->isHevdVideoContext)
        BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_WRAP_PTR_OFFSET, 0 );
#else
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_WRAP_PTR_OFFSET, 0 );
#endif
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_DEPTH_OFFSET, 0 );

    Offset = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + ITB_BASE_PTR_OFFSET );

    /* Some users don't allocate an ITB, so we check the base ptr is not NULL */
    if( Offset )
    {
        BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + ITB_WRITE_PTR_OFFSET, Offset );
        BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + ITB_READ_PTR_OFFSET, Offset );
        BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + ITB_VALID_PTR_OFFSET, Offset );
        BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + ITB_WRAP_PTR_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + ITB_DEPTH_OFFSET, 0 );
    }

    return( ExitCode );
}

BERR_Code BXPT_Rave_ResetContext(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t Reg, endainCtrl,intrEnable, tpitEnable;
    BMMA_DeviceOffset cdbBasePtr,cdbEndPtr,itbBasePtr,itbEndPtr;

    unsigned WhichScdBlock;
    BXPT_Rave_ContextThresholds Thresholds;
    unsigned enableBppSearch;
    unsigned EmmEn;

    unsigned ScdPidChannel = 0;
    bool ScdPidValid = false;
    BERR_Code ExitCode = BERR_SUCCESS;

    BXPT_Rave_Handle hRave = (BXPT_Rave_Handle)hCtx->vhRave;

    BDBG_MSG(( "Reset context %u", hCtx->Index ));

    if( (ExitCode = BXPT_Rave_FlushContext(hCtx)) != BERR_SUCCESS )
        goto Done;

    /* save endian control values */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG2_OFFSET );
    endainCtrl = Reg &(BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, ITB_ENDIAN_CTRL ) | BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, CDB_ENDIAN_CTRL ));

    /* save all the pointers */
    cdbBasePtr = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET );
    cdbEndPtr =  BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_END_PTR_OFFSET );
    itbBasePtr = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + ITB_BASE_PTR_OFFSET );
    itbEndPtr =  BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + ITB_END_PTR_OFFSET );

    /* save interrupt enable register */
    intrEnable = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_INTERRUPT_ENABLES_OFFSET );

    /* save the TPIT_ENABLE value
       SW7335-531 : Restore TPIT_CHANNEL during context reset */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET );
    tpitEnable = Reg &(BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_ENABLE ) | BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_CHANNEL ));

    /* Save the SCD config for VCT contexts */
    GetScdPid( hCtx, 0, &WhichScdBlock, &ScdPidChannel, &ScdPidValid );

    /* deallocate SCD blocks used by this context */
    {
        unsigned ScdSlotNum, WhichScdBlock, Pid;
        bool PidValid;
        for ( ScdSlotNum=1; ScdSlotNum<8; ScdSlotNum++ ) { /* start at 1 */
            GetScdPid( hCtx, ScdSlotNum, &WhichScdBlock, &Pid, &PidValid );
            if (PidValid) {
                hRave->ScdTable[ WhichScdBlock ].Allocated = false;
            }
        }
    }

    BXPT_Rave_GetThresholds( hCtx, &Thresholds );

    /* Save the BPP search enable */
    enableBppSearch = BCHP_GET_FIELD_DATA( BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG3_OFFSET ),
        XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_BPP_SEARCH );

    EmmEn = BCHP_GET_FIELD_DATA( BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET ),
        XPT_RAVE_CX0_REC_MISC_CONFIG, EMM_EN );

    /* reset the context without allocating the buffers */
    InitContext(hCtx,NULL);

    ExitCode = BXPT_Rave_SetThresholds( hCtx, &Thresholds );
    if( BERR_SUCCESS != ExitCode )
        goto Done;

    /* By default, all SCD mapping should be done by PID channels */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET );
    Reg &= ~BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

    /* Each AV will need 1 SCD indexer, to build the ITB entries. */
    if( hCtx->Type == BXPT_RaveCx_eAv || hCtx->Type == BXPT_RaveCx_eAvR)
    {
        InitScd( hCtx->hAvScd );

        /* Now map this SCD to the AV context */
        ChangeScdPid( hCtx, 0, WhichScdBlock, ScdPidChannel, ScdPidValid );
    }
    else if( hCtx->Type == BXPT_RaveCx_eVctNull )
    {
        /* Same as AV contexts, except we also restore the PID channel and Valid settings */

        InitScd( hCtx->hAvScd );

        /* Now map this SCD to the AV context */
        ChangeScdPid( hCtx, 0, hCtx->hAvScd-hRave->ScdTable, ScdPidChannel, ScdPidValid );
    }
    else
    {
        /* Some special stuff for record contexts */
        /*
        ** Set PES Sync for MPEG, our initial value for recording. This is used
        ** chiefly for startcode generation.
        ** MPEG -> PES_SYNC = 1
        ** DirecTV -> PES_SYNC = 3
        */
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG1_OFFSET );
        Reg &= ~(
                 BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE )
                 );
        Reg |= (
                BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE, 1 )
                );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG1_OFFSET, Reg );
    }

    /* resetore context pointers */
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_WRITE_PTR_OFFSET, cdbBasePtr );
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_READ_PTR_OFFSET, cdbBasePtr );
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET, cdbBasePtr );
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_END_PTR_OFFSET, cdbEndPtr );
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_VALID_PTR_OFFSET, cdbBasePtr );

#if BXPT_HAS_HEVD_WORKAROUND
    /* Do not alter the CDB wrap for HEVD video contexts. All others should the wrap updated. */
    if(!hCtx->isHevdVideoContext)
        BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_WRAP_PTR_OFFSET, 0 );
#else
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_WRAP_PTR_OFFSET, 0 );
#endif

    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + ITB_WRITE_PTR_OFFSET, itbBasePtr );
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + ITB_READ_PTR_OFFSET, itbBasePtr );
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + ITB_BASE_PTR_OFFSET, itbBasePtr );
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + ITB_END_PTR_OFFSET, itbEndPtr );
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + ITB_VALID_PTR_OFFSET, itbBasePtr );
    BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + ITB_WRAP_PTR_OFFSET, 0 );

    /* restore threshold offsets */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_THRESHOLDS_OFFSET );
    Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_OVERFLOW_THRESHOLD ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_WRAPAROUND_THRESHOLD )
        );
    Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_OVERFLOW_THRESHOLD, BXPT_RAVE_OVERFLOW_THRESH ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_WRAPAROUND_THRESHOLD, BXPT_RAVE_WRAP_THRESH )
        );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_THRESHOLDS_OFFSET, Reg );

    /* restore endian control */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG2_OFFSET );
    Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, ITB_ENDIAN_CTRL ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, CDB_ENDIAN_CTRL )
        );
    Reg |= endainCtrl;
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG2_OFFSET, Reg );

    /* restore interrupt enables */
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_INTERRUPT_ENABLES_OFFSET,intrEnable );

    /* restore TPIT_ENABLE
       SW7335-531 : Restore TPIT_CHANNEL during context reset */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET );
    Reg &= ~(
             BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_ENABLE ) |
             BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_CHANNEL )
        );
    Reg |= tpitEnable;
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG3_OFFSET );
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_BPP_SEARCH ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_BPP_SEARCH, enableBppSearch ) );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG3_OFFSET, Reg );

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET );
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, EMM_EN ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, EMM_EN, EmmEn ) );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

    Done:
    return( ExitCode );
}


BERR_Code GetScdPid(
    BXPT_RaveCx_Handle hCtx,
    unsigned WhichScd,
    unsigned *WhichScdBlock,
    unsigned *Pid,
    bool *PidValid
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

#ifdef BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_AB
    switch( WhichScd )
    {
        case 0:
        case 1:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET;
        break;

        case 2:
        case 3:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET;
        break;

        case 4:
        case 5:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET;
        break;

        case 6:
        case 7:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET;
        break;

        default:
        BDBG_ERR(( "Invalid WhichScd: %u", WhichScd ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    Reg = BREG_Read32( hCtx->hReg, RegAddr );
    if( WhichScd % 2 )
    {
        *WhichScdBlock = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMB );
        *Pid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDB );
        *PidValid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDB_VALID );
    }
    else
    {
        *WhichScdBlock = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA );
        *Pid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDA );
        *PidValid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID );
    }
#else
    switch( WhichScd )
    {
        case 0:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_A_OFFSET;
        break;

        case 1:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_B_OFFSET;
        break;

        case 2:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_C_OFFSET;
        break;

        case 3:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_D_OFFSET;
        break;

        case 4:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_E_OFFSET;
        break;

        case 5:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_F_OFFSET;
        break;

        case 6:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_G_OFFSET;
        break;

        case 7:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_H_OFFSET;
        break;

        default:
        BDBG_ERR(( "Invalid WhichScd: %u", WhichScd ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    Reg = BREG_Read32( hCtx->hReg, RegAddr );
    *WhichScdBlock = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_PID_channels_SCD_NUMA );
    *Pid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_PID_channels_SCD_PID_CHA );
    *PidValid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_PID_channels_PID_CHA_VALID );
    #if 0
    BDBG_MSG(("%s : %d : hCtx: %p WhichScd: %u WhichScdBlock: %u Pid: %u PidValid: %u  ",
                 BSTD_FUNCTION, __LINE__, (void*) hCtx, WhichScd, *WhichScdBlock, *Pid, *PidValid   ));
    #endif
#endif

    Done:
    return ExitCode;
}

BERR_Code ChangeScdPid(
    BXPT_RaveCx_Handle hCtx,
    unsigned WhichScd,
    unsigned WhichScdBlock,
    unsigned Pid,
    bool PidValid
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    #if 0
    BDBG_MSG(("%s : %d : Entry: hCtx: %p, WhichScd: %u, WhichScdBlock: %u, Pid: %u, PidValid: %u ",
                           BSTD_FUNCTION, __LINE__ , (void *) hCtx, WhichScd, WhichScdBlock, Pid, PidValid  ));
    #endif

#ifdef BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_AB
    switch( WhichScd )
    {
        case 0:
        case 1:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET;
        break;

        case 2:
        case 3:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET;
        break;

        case 4:
        case 5:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET;
        break;

        case 6:
        case 7:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET;
        break;

        default:
        BDBG_ERR(( "Invalid WhichScd: %u", WhichScd ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    Reg = BREG_Read32( hCtx->hReg, RegAddr );
    if( WhichScd % 2 )
    {
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMB ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDB_VALID ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDB )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMB, WhichScdBlock ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDB_VALID, PidValid == true ? 1 : 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDB, Pid )
        );
    }
    else
    {
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDA )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA, WhichScdBlock ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID, PidValid == true ? 1 : 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDA, Pid )
        );
    }
    BREG_Write32( hCtx->hReg, RegAddr, Reg );
#else
    switch( WhichScd )
    {
        case 0:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_A_OFFSET;
        break;

        case 1:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_B_OFFSET;
        break;

        case 2:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_C_OFFSET;
        break;

        case 3:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_D_OFFSET;
        break;

        case 4:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_E_OFFSET;
        break;

        case 5:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_F_OFFSET;
        break;

        case 6:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_G_OFFSET;
        break;

        case 7:
        RegAddr = hCtx->BaseAddr + REC_SCD_PIDS_H_OFFSET;
        break;

        default:
        BDBG_ERR(( "Invalid WhichScd: %u", WhichScd ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    Reg = BREG_Read32( hCtx->hReg, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_PID_channels_SCD_NUMA ) |
        BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_PID_channels_PID_CHA_VALID ) |
        BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_PID_channels_SCD_PID_CHA )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_PID_channels_SCD_NUMA, WhichScdBlock ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_PID_channels_PID_CHA_VALID, PidValid == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_A, Mapped_SCD_via_PID_channels_SCD_PID_CHA, Pid )
    );
    BREG_Write32( hCtx->hReg, RegAddr, Reg );
#endif

    Done:
    return ExitCode;
}

BERR_Code FlushScds(
    BXPT_RaveCx_Handle hCtx
    )
{
    unsigned ScdIndex;
    unsigned WhichScdBlock;
    unsigned PidChannel;
    bool PidValid;

    for( ScdIndex = 0; ScdIndex < 8; ScdIndex++ )
    {
        GetScdPid( hCtx, ScdIndex, &WhichScdBlock, &PidChannel, &PidValid );
        if( PidValid )
        {
            ClearScdStates( hCtx, WhichScdBlock );
        }
    }

    return BERR_SUCCESS;
}

BERR_Code ClearScdStates(
    BXPT_RaveCx_Handle hCtx,
    uint32_t ScdNum
    )
{
    unsigned ii;

    unsigned BaseAddr = BCHP_XPT_RAVE_SCD0_SCD_MISC_CONFIG + ( ScdNum * SCD_REG_STEP );

    for( ii = SCD_STATE_START_OFFSET; ii <= SCD_STATE_END_OFFSET; ii += REG_STEP )
        BREG_Write32( hCtx->hReg, BaseAddr + ii, 0 );

    /* reset the DMEM area for this SCD */
    BaseAddr = BCHP_XPT_RAVE_DMEMi_ARRAY_BASE + SCD_DMEM_BASE*4 + ScdNum*SCD_DMEM_SIZE*4;

    for( ii = 0; ii < SCD_DMEM_SIZE ; ii++ )
    {
        if(ii==0x15)
        {
            /* Set the BTP tail pointer to 0xFE, per PR 26647 */
            BREG_Write32( hCtx->hReg, BaseAddr + (ii*4), 0xfe );
        }
        else
            BREG_Write32( hCtx->hReg, BaseAddr + (ii*4), 0 );
   }
    return BERR_SUCCESS;
}

#define TPIT_STATE_STEP (BCHP_XPT_RAVE_TPIT0_STATE1 - BCHP_XPT_RAVE_TPIT0_STATE0)

BERR_Code FlushTpit(
    BXPT_RaveCx_Handle hCtx,
    uint32_t TpitNum
    )
{
    unsigned ii;

    unsigned BaseAddr = BCHP_XPT_RAVE_TPIT0_CTRL1 + ( TpitNum * TPIT_CTRL_REG_STEP );

    for( ii = TPIT_STATE_START_OFFSET; ii <= TPIT_STATE_END_OFFSET; ii += TPIT_STATE_STEP )
        BREG_Write32( hCtx->hReg, BaseAddr + ii, 0 );

    return BERR_SUCCESS;
}

int GetNextPicCounter(
    BXPT_RaveCx_Handle hCtx
    )
{
    /* Each context has it's own picture counter, hard-wired. */
    return (int) hCtx->Index;
}

void FreePicCounter(
    BXPT_RaveCx_Handle Context      /* [in] The context to free. */
    )
{
    /* Each context has it's own picture counter, hard-wired. */
    BSTD_UNUSED( Context );
}

void SetPictureCounterMode(
    BXPT_RaveCx_Handle hCtx,        /* [in] The context to free. */
    BAVC_ItbEsType ItbFormat
    )
{
    uint32_t Reg;
    unsigned Mode;
    uint32_t Scv0, Scv1, Scv2;

    Scv0 = Scv1 = Scv2 = 0xFF;  /* Init to an invalid range. */
    switch( ItbFormat )
    {
        case BAVC_ItbEsType_eMpeg2Video:
        case BAVC_ItbEsType_eMpeg1Video:
        Mode = 0;   /* SVC mode. */
        Scv0 = 0x00;    /* MPEG2 picture header */
        Scv1 = 0xB7;    /* Sequence end code. */
        break;

        case BAVC_ItbEsType_eAvcVideo:
        Mode = 2;   /* AVC mode. */
        break;

        case BAVC_ItbEsType_eVc1Video:
        Mode = 0;   /* SVC mode. */
        Scv0 = 0x0D;    /* VC1 frame start */
        break;

        default:
        BDBG_ERR(( "Invalid ITB type for picture counter. Defaulting to MPEG." ));
        Mode = 0;   /* SVC mode. */
        Scv0 = 0x00;    /* MPEG2 picture header */
        break;
    }

    Reg  = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + PIC_CTR_MODE_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_PIC_CTR_MODE, Valid_PIC_CTR_VALUE ) |
        BCHP_MASK( XPT_RAVE_CX0_PIC_CTR_MODE, PIC_CTR_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_PIC_CTR_MODE, PIC_CTR_MODE ) |
        BCHP_MASK( XPT_RAVE_CX0_PIC_CTR_MODE, SCV0 ) |
        BCHP_MASK( XPT_RAVE_CX0_PIC_CTR_MODE, SCV1 ) |
        BCHP_MASK( XPT_RAVE_CX0_PIC_CTR_MODE, SCV2 )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_PIC_CTR_MODE, Valid_PIC_CTR_VALUE, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_PIC_CTR_MODE, PIC_CTR_EN, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_PIC_CTR_MODE, PIC_CTR_MODE, Mode ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_PIC_CTR_MODE, SCV0, Scv0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_PIC_CTR_MODE, SCV1, Scv1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_PIC_CTR_MODE, SCV2, Scv2 )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + PIC_CTR_MODE_OFFSET, Reg );
}

void FlushPicCounter(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t IncDecMode;

    uint32_t Reg  = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + PIC_INC_DEC_CTRL_OFFSET );

    IncDecMode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_PIC_INC_DEC_CTRL, INC_DEC_MODE );

    /* Writing 2 to the INC_DEC_MODE bitfield resets the counter. */
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_PIC_INC_DEC_CTRL, INC_DEC_MODE ) );
    Reg |= BCHP_FIELD_DATA( XPT_RAVE_CX0_PIC_INC_DEC_CTRL, INC_DEC_MODE, 2 );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + PIC_INC_DEC_CTRL_OFFSET, Reg );

    /* Restore the old mode */
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_PIC_INC_DEC_CTRL, INC_DEC_MODE ) );
    Reg |= BCHP_FIELD_DATA( XPT_RAVE_CX0_PIC_INC_DEC_CTRL, INC_DEC_MODE, IncDecMode );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + PIC_INC_DEC_CTRL_OFFSET, Reg );
}


BERR_Code BXPT_Rave_ComputeThresholds(
    BXPT_RaveCx_Handle hCtx,
    size_t CdbLength,
    size_t ItbLength,
    BXPT_Rave_ContextThresholds *Thresholds
    )
{
     BMMA_DeviceOffset Base, End;
     uint32_t GranularityInBytes;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hCtx );
    BDBG_ASSERT( Thresholds );

    GranularityInBytes = GetGranularityInBytes( hCtx );
    if( !CdbLength )
    {
        Base = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET );
        End = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_END_PTR_OFFSET );
        CdbLength = End - Base + 1;
    }

    if( CdbLength <= GranularityInBytes )
    {
        BDBG_WRN(( "Requested CdbLength <= minimum granularity. Lower thresholds will be clamped to 0, upper to granularity" ));
        Thresholds->CdbLower = Thresholds->ItbLower = 0;

        /* Remember that these values are multiplied by the granularity (in hw). So, this is the smallest we can get. */
        Thresholds->CdbUpper = 1;
        Thresholds->ItbUpper = 1;
        return ExitCode;
    }

    /*
    ** The XC buffer and RAVE have some internal buffers that are NOT held off by the pause mechanism. Those packets will
    ** still be transfered to DRAM, even after the upper threshold is crossed. To prevent loosing these packets, we need
    ** to reserve some extra space in the CDB. This is accomplished by computing the number of packets those internal
    ** buffers can hold, coverting that number to bytes, then setting the upper threshold to fire that much earlier.
    ** The math works out as: 2 (XC Buffer read) + 5 (RAVE input) + 2 (RAVE DMEM) = 9 packets, or 9 * 188 bytes.
    */
    Thresholds->CdbUpper = (CdbLength - BXPT_RAVE_OVERFLOW_THRESH - (9 * 188)) / GranularityInBytes;
    Thresholds->CdbLower = (CdbLength - BXPT_RAVE_OVERFLOW_THRESH - (11 * 188)) / GranularityInBytes;
    if( Thresholds->CdbUpper > 0xFFFF )
    {
        BDBG_ERR(( "CdbUpper exceeds 16-bit size limit" ));
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    if( Thresholds->CdbLower > 0xFFFF )
    {
        BDBG_ERR(( "CdbLower exceeds 16-bit size limit" ));
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    if( !ItbLength )
    {
        Base = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + ITB_BASE_PTR_OFFSET );
        End = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + ITB_END_PTR_OFFSET );

        /* Some users don't allocate an ITB. Check the base reg to be sure. */
        if( Base )
        {
            ItbLength = End - Base + 1;
        }
    }

    if( ItbLength )
    {
        if (ItbLength < (size_t) BXPT_RAVE_OVERFLOW_THRESH + (11 * 240) )
        {
            BDBG_ERR(( "ItbLength %u too small", (unsigned ) ItbLength));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        /*
        ** Allow room for 10 ITB entries, at 24 bytes per entry. Anand mentioned this should be the max we'll get
        ** from one TS packet
        */
        Thresholds->ItbUpper = (ItbLength - BXPT_RAVE_OVERFLOW_THRESH - (9 * 240)) / GranularityInBytes;
        Thresholds->ItbLower = (ItbLength - BXPT_RAVE_OVERFLOW_THRESH - (11 * 240)) / GranularityInBytes;

        if( Thresholds->ItbUpper > 0xFFFF )
        {
            BDBG_ERR(( "ItbUpper exceeds 16-bit size limit" ));
            return BERR_INVALID_PARAMETER;
        }
        if( Thresholds->ItbLower > 0xFFFF )
        {
            BDBG_ERR(( "ItbLower exceeds 16-bit size limit" ));
            return BERR_INVALID_PARAMETER;
        }
    }
    else
    {
        Thresholds->ItbUpper = 0;
        Thresholds->ItbLower = 0;
    }

    BDBG_MSG(( "BXPT_Rave_ComputeThresholds CX%u: CdbUpper 0x%08X, CdbLower 0x%08X, ItbUpper 0x%08X, ItbLower 0x%08X",
        hCtx->Index, Thresholds->CdbUpper, Thresholds->CdbLower, Thresholds->ItbUpper, Thresholds->ItbLower ));

    return ExitCode;
}

BERR_Code BXPT_Rave_GetThresholds(
    BXPT_RaveCx_Handle hCtx,
    BXPT_Rave_ContextThresholds *Thresholds
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hCtx );
    BDBG_ASSERT( Thresholds );

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_CDB_THRESHOLD_OFFSET );
    Thresholds->CdbUpper = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_UPPER_THRESHOLD );
    Thresholds->CdbLower = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_LOWER_THRESHOLD );

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_ITB_THRESHOLD_OFFSET );
    Thresholds->ItbUpper = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_UPPER_THRESHOLD );
    Thresholds->ItbLower = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_LOWER_THRESHOLD );

    return ExitCode;
}

BERR_Code BXPT_Rave_SetThresholds(
    BXPT_RaveCx_Handle hCtx,
    const BXPT_Rave_ContextThresholds *Thresholds
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hCtx );
    BDBG_ASSERT( Thresholds );

    BDBG_MSG(( "BXPT_Rave_SetThresholds CX%u: CdbUpper 0x%08X, CdbLower 0x%08X, ItbUpper 0x%08X, ItbLower 0x%08X",
        hCtx->Index, Thresholds->CdbUpper, Thresholds->CdbLower, Thresholds->ItbUpper, Thresholds->ItbLower ));

    if( Thresholds->CdbUpper > 0xFFFF )
    {
        BDBG_ERR(( "CdbUpper exceeds 16-bit size limit" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto done;
    }
    if( Thresholds->CdbLower > 0xFFFF )
    {
        BDBG_ERR(( "CdbLower exceeds 16-bit size limit" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto done;
    }
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_CDB_THRESHOLD_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_UPPER_THRESHOLD ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_LOWER_THRESHOLD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_UPPER_THRESHOLD, Thresholds->CdbUpper ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_LOWER_THRESHOLD, Thresholds->CdbLower )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_CDB_THRESHOLD_OFFSET, Reg );

    /* PR 22634: After updating the CDB thresholds, reset a possible pause. */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG1_OFFSET );
    if ( BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE ) )
    {
        /* Bits in this register are per-context */
        uint32_t MyIndexBit = 1UL << hCtx->Index;

        /* Read Band Hold Status */
        Reg = BREG_Read32( hCtx->hReg, BCHP_XPT_RAVE_CX_HOLD_CLR_STATUS );
        if ( Reg & MyIndexBit )
        {
            /* Grab current CDB Depth */
            Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_DEPTH_OFFSET );
            Reg = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_CDB_DEPTH, CDB_BUFFER_DEPTH );

            /* If depth is < threshold, reset pause (threshold units are 256 bytes) */
            Reg /= GetGranularityInBytes( hCtx );
            if ( Reg < Thresholds->CdbUpper )
            {
                BDBG_WRN(( "Resetting pause on context %u - threshold=%u > depth=%u", hCtx->Index, Thresholds->CdbUpper, Reg ));

                /* Clear only my bit */
                BREG_Write32( hCtx->hReg, BCHP_XPT_RAVE_CX_HOLD_CLR_STATUS, MyIndexBit );
            }
        }
    }
    if( Thresholds->ItbUpper > 0xFFFF )
    {
        BDBG_ERR(( "ItbUpper exceeds 16-bit size limit" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto done;
    }
    if( Thresholds->ItbLower > 0xFFFF )
    {
        BDBG_ERR(( "ItbLower exceeds 16-bit size limit" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto done;
    }

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_ITB_THRESHOLD_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_UPPER_THRESHOLD ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_LOWER_THRESHOLD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_UPPER_THRESHOLD, Thresholds->ItbUpper ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_LOWER_THRESHOLD, Thresholds->ItbLower )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_ITB_THRESHOLD_OFFSET, Reg );

    done:
    return ExitCode;
}

BERR_Code BXPT_Rave_GetScramblingCtrl(
    BXPT_RaveCx_Handle hCtx,
    BXPT_Rave_ScrambleCtrl *ScrambleCtrl
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hCtx );
    BDBG_ASSERT( ScrambleCtrl );

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + SCRAMBLE_CTRL_OFFSET );
    ScrambleCtrl->PusiValid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_SC, SC_PUSI_VALID );
    ScrambleCtrl->Pusi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_SC, SC_PUSI );
    ScrambleCtrl->AllValid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_SC, SC_ALL_VALID );
    ScrambleCtrl->ScAll = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_SC, SC_ALL );

    return ExitCode;
}

BERR_Code BXPT_Rave_ClearSCRegister(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hCtx );

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + SCRAMBLE_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_SC, SC_PUSI_VALID ) |
        BCHP_MASK( XPT_RAVE_CX0_SC, SC_PUSI ) |
        BCHP_MASK( XPT_RAVE_CX0_SC, SC_OR_MODE ) |
        BCHP_MASK( XPT_RAVE_CX0_SC, SC_ALL_VALID ) |
        BCHP_MASK( XPT_RAVE_CX0_SC, SC_ALL )
    );

    /* restore SC_OR_MODE PR57627 :
    ** SC_OR_MODE is used to select the way scramble control bits are reported.
    ** 0 = Disable OR-ing of current and previous scramble control bits (Default).
    ** 1 = Enable OR-ing of current and previous scramble control bits. This is to
    ** support streams which have mixture of scrambled and unscrambled packets within
    ** the same PID. In such case, these PIDs will be treated as scramble PIDs.
    ** By default this is disabled.
    */
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_SC, SC_OR_MODE, hCtx->ScOrMode )
    );

    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + SCRAMBLE_CTRL_OFFSET, Reg );

    return ExitCode;
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Rave_GetContextDefaultSettings(
    BXPT_Rave_ContextSettings *ContextDefSettings   /* [out] The defaults. */
    )
{
     BKNI_Memset( (void *)ContextDefSettings, 0, sizeof(BXPT_Rave_ContextSettings));
     /* ContextDefSettings->PesExtSearchMode = BXPT_Rave_PesExtSearchAlways;*/
     return BERR_SUCCESS;
}
#endif

BERR_Code BXPT_Rave_GetContextConfig(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    BXPT_Rave_ContextSettings *Config   /* [out] The Context settings. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Context );
    BDBG_ASSERT( Config );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG3_OFFSET );
    Config->EnableBppSearch = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_BPP_SEARCH ) ? true : false;
    Config->EnableCpChangeDetect = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, CP_PERM_CHANGE_DETECT ) ? true : false;
    Config->PesExtSearchMode = (BXPT_Rave_PesExtSearchMode)BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, PES_EXT_SEARCH_MODE );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_PID_STREAM_ID_OFFSET );
    Config->EnablePrivateHdrItbEntry = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_PID_STREAM_ID, PRV_HDR_ITB_EN ) ? true : false;
    Config->AudFrameInfo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_PID_STREAM_ID, AUD_FRAME_INFO );
    Config->FilterPidStreamId = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_PID_STREAM_ID, FILTER_PID_STREAM_ID );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG4_OFFSET );
    Config->SidExtDependent = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG4, PES_SID_EXT_DEP );
    Config->SidExtIndependent = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG4, PES_SID_EXT_IND );
    Config->PesSidExtMode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG4, PES_SID_EXT_MODE );

    return ExitCode;
}

BERR_Code BXPT_Rave_SetContextConfig(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    const BXPT_Rave_ContextSettings *Config /* [out] The Context settings. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Context );
    BDBG_ASSERT( Config );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG3_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_BPP_SEARCH ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, CP_PERM_CHANGE_DETECT )|
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, PES_EXT_SEARCH_MODE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_BPP_SEARCH, Config->EnableBppSearch == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, CP_PERM_CHANGE_DETECT, Config->EnableCpChangeDetect == true ? 1 : 0 )|
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, PES_EXT_SEARCH_MODE, Config->PesExtSearchMode )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG3_OFFSET, Reg );

    if(Config->AudFrameInfo > 15)
        BDBG_ERR(( "Invalid Arg AudFrameInfo %d",Config->AudFrameInfo ));

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_PID_STREAM_ID_OFFSET );
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_AV_PID_STREAM_ID, PRV_HDR_ITB_EN ) |
              BCHP_MASK( XPT_RAVE_CX0_AV_PID_STREAM_ID, AUD_FRAME_INFO ) |
              BCHP_MASK( XPT_RAVE_CX0_AV_PID_STREAM_ID, FILTER_PID_STREAM_ID )
            );
    Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_PID_STREAM_ID, PRV_HDR_ITB_EN, Config->EnablePrivateHdrItbEntry == true ? 1 : 0 )|
             BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_PID_STREAM_ID, AUD_FRAME_INFO, Config->AudFrameInfo ) |
             BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_PID_STREAM_ID, FILTER_PID_STREAM_ID, Config->FilterPidStreamId )
             );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_PID_STREAM_ID_OFFSET, Reg );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG4_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG4, PES_SID_EXT_DEP ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG4, PES_SID_EXT_IND ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG4, PES_SID_EXT_MODE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG4, PES_SID_EXT_DEP, Config->SidExtDependent ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG4, PES_SID_EXT_IND, Config->SidExtIndependent)|
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG4, PES_SID_EXT_MODE, Config->PesSidExtMode)
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG4_OFFSET, Reg );

    return ExitCode;
}


void FreeScds(
    unsigned NumScds,
    StartCodeIndexer **ScdPtr
    )
{
    unsigned ii;

    for( ii = 0; ii < NumScds; ii++ )
    {
        ScdPtr[ ii ]->Allocated = false;
    }
}

/*
** Internally, the SCD array is accessed as bytes. However, the host MIPS sees this
** as an array of longs, with each 'byte' mapped to the LSB of each long. For instance,
** byte[ 2 ] is accesed by the MIPS through the LSB of long[ 2 ].
*/
#define GET_SCD_BYTE_ADDR( Base, Offset )   ( Base + 4 * Offset )

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Rave_GetIpConfig(
    BXPT_RaveCx_Handle hCtx,        /* [in] Handle for the IP context */
    BXPT_Rave_IpConfig *IpConfig    /* [out] The IP config params */
    )
{
    uint32_t ScdBytesBase;
    unsigned ScdNum;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hCtx );
    BDBG_ASSERT( IpConfig );

    ScdNum = hCtx->hAvScd->ChannelNo;
    ScdBytesBase = BCHP_XPT_RAVE_DMEMi_ARRAY_BASE + SCD_DMEM_BASE * 4 + ScdNum * SCD_DMEM_SIZE * 4;

    IpConfig->IpHeaderChecksum = ( BREG_Read32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 2 ) ) << 8 ) & 0xFF00;
    IpConfig->IpHeaderChecksum |= BREG_Read32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 1 )) & 0xFF;

    IpConfig->IpHeaderLength = BREG_Read32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 5 )) & 0xFF;

    IpConfig->NumTsPackets = BREG_Read32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 8 )) & 0xFF;

    IpConfig->SequenceNumIncrement = ( BREG_Read32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 10 ) ) << 8 ) & 0xFF00;
    IpConfig->SequenceNumIncrement |= BREG_Read32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 9 )) & 0xFF;

    IpConfig->CurrentSequenceNum = ( BREG_Read32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 14 ) ) << 24 ) & 0xFF000000;
    IpConfig->CurrentSequenceNum |= ( BREG_Read32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 13 ) ) << 16 ) & 0xFF0000;
    IpConfig->CurrentSequenceNum |= ( BREG_Read32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 12 ) ) << 8 )  & 0xFF00;
    IpConfig->CurrentSequenceNum |=   BREG_Read32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 11 ))          & 0xFF;

    IpConfig->IsMpegTs = hCtx->IsMpegTs;

    return ExitCode;
}

BERR_Code BXPT_Rave_SetIpConfig(
    BXPT_RaveCx_Handle hCtx,            /* [in] Handle for the IP context */
    const BXPT_Rave_IpConfig *IpConfig  /* [out] The IP config params */
    )
{
    uint32_t ScdBytesBase;
    uint32_t Reg;
    uint32_t WrapThreshold;
    uint8_t PacketSize;
    unsigned ScdNum;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hCtx );
    BDBG_ASSERT( IpConfig );

    ScdNum = hCtx->hAvScd->ChannelNo;
    ScdBytesBase = BCHP_XPT_RAVE_DMEMi_ARRAY_BASE + SCD_DMEM_BASE * 4 + ScdNum * SCD_DMEM_SIZE * 4;

    BREG_Write32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 1 ), IpConfig->IpHeaderChecksum & 0xFF );
    BREG_Write32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 2 ), ( IpConfig->IpHeaderChecksum >> 8 ) & 0xFF );

    BREG_Write32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 5 ), IpConfig->IpHeaderLength );

    BREG_Write32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 8 ), IpConfig->NumTsPackets );

    BREG_Write32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 9 ), IpConfig->SequenceNumIncrement & 0xFF );
    BREG_Write32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 10 ), ( IpConfig->SequenceNumIncrement >> 8 ) & 0xFF );

    BREG_Write32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 11 ), IpConfig->CurrentSequenceNum & 0xFF );
    BREG_Write32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 12 ), ( IpConfig->CurrentSequenceNum >> 8 ) & 0xFF );
    BREG_Write32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 13 ), ( IpConfig->CurrentSequenceNum >> 16 ) & 0xFF );
    BREG_Write32( hCtx->hReg, GET_SCD_BYTE_ADDR( ScdBytesBase, 14 ), ( IpConfig->CurrentSequenceNum >> 24 ) & 0xFF );

/* TBD: Ask Sanjeev if there's already a define for the IP frame size. */
#define IP_FRAME_SIZE   ( 2048 )

    hCtx->IsMpegTs = IpConfig->IsMpegTs;
    PacketSize = IpConfig->IsMpegTs ? 188 : 130;
    WrapThreshold = IP_FRAME_SIZE - ( IpConfig->IpHeaderLength + IpConfig->NumTsPackets * PacketSize ) + 1;

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_THRESHOLDS_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_OVERFLOW_THRESHOLD ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_WRAPAROUND_THRESHOLD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_OVERFLOW_THRESHOLD, CTX_IP_OVERFLOW_THRESH ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_THRESHOLDS, CONTEXT_WRAPAROUND_THRESHOLD, WrapThreshold )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_THRESHOLDS_OFFSET, Reg );

    return ExitCode;
}

uint8_t *BXPT_Rave_GetCdbBasePtr(
    BXPT_RaveCx_Handle hCtx
    )
{
    BMMA_DeviceOffset Base;
    void *Ptr;

    BDBG_ASSERT( hCtx );

    if (!hCtx->externalCdbAlloc) {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return 0;
    }

    Base = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET );
    Ptr = (uint8_t*)hCtx->mma.cdbPtr + (Base - hCtx->mma.cdbOffset); /* convert Base -> cached ptr */
    return Ptr;
}
#endif

void GetScEnables(
    BXPT_Rave_EsRanges *Range,
    unsigned Mode
    )
{
    switch( Mode )
    {
        case 0:
        case 1:
        Range->Enable = false;
        Range->RangeIsASlice = false;
        break;

        case 2:
        Range->Enable = true;
        Range->RangeIsASlice = false;
        break;

        case 3:
        Range->Enable = true;
        Range->RangeIsASlice = true;
        break;
    }

}

void GetScEnables_Indexer(
    IndexerScRange *Range,
    unsigned Mode
    )
{
    switch( Mode )
    {
        case 0:
        case 1:
        Range->RangeEnable = false;
        Range->RangeIsASlice = false;
        break;

        case 2:
        Range->RangeEnable = true;
        Range->RangeIsASlice = false;
        break;

        case 3:
        Range->RangeEnable = true;
        Range->RangeIsASlice = true;
        break;
    }

}

#define GET_ITB_TYPE(itb) ((itb[0]>>24) & 0xFF)

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Rave_AllocSoftContext(
    BXPT_RaveCx_Handle SrcContext,      /* [in] The source context */
    BXPT_RaveSoftMode DestContextMode,  /* [in] The type of data that the destination should generate. */
    BXPT_RaveCx_Handle *DestContext     /* [out] The destination (soft) context */
    )
{
    /* deprecated due to MMA conversion */
    BSTD_UNUSED(SrcContext);
    BSTD_UNUSED(DestContextMode);
    BSTD_UNUSED(DestContext);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif

BERR_Code AllocSoftContext_Priv(
    BXPT_RaveCx_Handle SrcContext,      /* [in] The source context */
    BXPT_RaveSoftMode DestContextMode,  /* [in] The type of data that the destination should generate. */
    unsigned Index,
    BXPT_RaveCx_Handle *DestContext     /* [out] The destination (soft) context */
    )
{
    BMMA_DeviceOffset Base, End;

    BERR_Code ExitCode = BERR_SUCCESS;
    BXPT_RaveCx_Handle ThisCtx = NULL;
    BXPT_Rave_Handle hRave = (BXPT_Rave_Handle) SrcContext->vhRave;
    BXPT_RaveCx SoftCtxMode = DestContextMode == BXPT_RaveSoftMode_eIndexOnlyRecord ? BXPT_RaveCx_eRecord : BXPT_RaveCx_eAv;

    BDBG_MSG(( "AllocSoftContext_Priv: src %u dst %u", SrcContext->Index, Index ));
    ThisCtx = *(hRave->ContextTbl + Index);
    ThisCtx->Allocated = true;
    ThisCtx->Type = SoftCtxMode;
    ThisCtx->SourceContext = SrcContext;

    ThisCtx->SoftRave.mode = (unsigned) DestContextMode;
    ThisCtx->SoftRave.SrcBaseAddr = SrcContext->BaseAddr;
    ThisCtx->IsSoftContext = true;
    ThisCtx->SoftRave.SrcContextIndex = SrcContext->Index;
    ThisCtx->allocatedCdbBufferSize = SrcContext->allocatedCdbBufferSize;

    /* Make the dest BASE/END point to the src's CDB/ITB */
    Base = BREG_ReadAddr( SrcContext->hReg, SrcContext->BaseAddr + CDB_BASE_PTR_OFFSET );
    BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_BASE_PTR_OFFSET, Base );
    End = BREG_ReadAddr( SrcContext->hReg, SrcContext->BaseAddr + CDB_END_PTR_OFFSET );
    BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_END_PTR_OFFSET, End );
    ThisCtx->mma.cdbPtr = SrcContext->mma.cdbPtr;
    ThisCtx->mma.cdbOffset = SrcContext->mma.cdbOffset;

    Base = BREG_ReadAddr( SrcContext->hReg, SrcContext->BaseAddr + ITB_BASE_PTR_OFFSET );
    End = BREG_ReadAddr( SrcContext->hReg, SrcContext->BaseAddr + ITB_END_PTR_OFFSET );
    ThisCtx->SoftRave.ItbSize = End - Base;
    if( DestContextMode == BXPT_RaveSoftMode_ePointersOnly )
    {
        BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_BASE_PTR_OFFSET, Base );
        BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_END_PTR_OFFSET, End );
        ThisCtx->mma.itbPtr = SrcContext->mma.itbPtr;
        ThisCtx->mma.itbOffset = SrcContext->mma.itbOffset;
    }
    else
    {
        BMMA_DeviceOffset BufferOffset;

        if (!ThisCtx->externalItbAlloc) /* XPT does the alloc */
        {
            /* Allocate a separate ITB for the destination */
            ThisCtx->mma.itbBlock = BMMA_Alloc(ThisCtx->mmaHeap, ThisCtx->SoftRave.ItbSize, 1 << 8, NULL);
            if (!ThisCtx->mma.itbBlock)
            {
                BDBG_ERR(( "ITB alloc failed!" ));
                ExitCode = BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
                goto Done;
            }
        }

        ThisCtx->mma.itbPtr = (uint8_t*)BMMA_Lock(ThisCtx->mma.itbBlock) + ThisCtx->mma.itbBlockOffset;
        ThisCtx->mma.itbOffset = BufferOffset = BMMA_LockOffset(ThisCtx->mma.itbBlock) + ThisCtx->mma.itbBlockOffset;
        BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_BASE_PTR_OFFSET, BufferOffset );
        BREG_WriteAddr( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_END_PTR_OFFSET, BufferOffset + ThisCtx->SoftRave.ItbSize );
    }

    ThisCtx->SoftRave.src_itb_base = BREG_ReadAddr( SrcContext->hReg, SrcContext->BaseAddr + ITB_BASE_PTR_OFFSET );
    ThisCtx->SoftRave.dest_itb_base = BREG_ReadAddr( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_BASE_PTR_OFFSET );
    BXPT_Rave_ResetSoftContext( ThisCtx, DestContextMode );
    BXPT_Rave_FlushContext( SrcContext );

    /* get CACHED pointers to ITB memory for read/write */
    {
        Base = BREG_ReadAddr( SrcContext->hReg, SrcContext->BaseAddr + ITB_BASE_PTR_OFFSET );
        ThisCtx->SoftRave.src_itb_mem = (uint8_t*)SrcContext->mma.itbPtr + (Base - SrcContext->mma.itbOffset); /* convert Base -> cached ptr */

        Base = BREG_ReadAddr( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_BASE_PTR_OFFSET );
        ThisCtx->SoftRave.dest_itb_mem = (uint8_t*)ThisCtx->mma.itbPtr + (Base - ThisCtx->mma.itbOffset); /* convert Base -> cached ptr */
    }

    /* SWSTB-3582. Default to legacy BASE opcode. AdvanceSoftContext will change that if it sees RAVE generating BASE entries with the new opcode. */
    ThisCtx->SoftRave.BaseOpCode = brave_itb_base_address;

    *DestContext = ThisCtx;

    Done:
    return ExitCode;
}

static void check_wrap(
    BXPT_RaveCx_Handle DestCtx,
    BMMA_DeviceOffset *dest_valid,
    BMMA_DeviceOffset *dest_wrap,
    uint32_t **dest_itb_mem
    )
{
    /* use exclusive logic for END and VALID in SW */
    BMMA_DeviceOffset dest_end = BREG_ReadAddr(DestCtx->hReg, DestCtx->BaseAddr + ITB_END_PTR_OFFSET ) + 1;

    /* using BXPT_ITB_SIZE as my wraparound threshold is fine. HW RAVE uses another threshold based on its block size for mem bandwidth */
    if (*dest_valid >= dest_end - 15 * BXPT_ITB_SIZE)
    {
        *dest_wrap = *dest_valid - 1; /* convert to inclusive logic */
        *dest_valid = DestCtx->SoftRave.dest_itb_base; /* this is inclusive */
        *dest_itb_mem = (uint32_t *)DestCtx->SoftRave.dest_itb_mem;
    }
}


/* exact copy of current ITB entry */
#define COPY_ITB() \
    do { \
    dest_itb[0] = src_itb[0]; \
    dest_itb[1] = src_itb[1]; \
    dest_itb[2] = src_itb[2]; \
    dest_itb[3] = src_itb[3]; \
    dest_valid += BXPT_ITB_SIZE; \
    dest_itb += BXPT_ITB_SIZE/4; \
    check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb); \
    DestCtx->SoftRave.SrcIsHeld =  false; \
    } while (0)

#define INSERT_TERMINATION_ITB(offset) \
    do { \
    dest_itb[0] = 0x70000000; \
    dest_itb[1] = (offset) & 0xFF; \
    dest_itb[2] = 0x0; \
    dest_itb[3] = 0x0; \
    dest_valid += BXPT_ITB_SIZE; \
    dest_itb += BXPT_ITB_SIZE/4; \
    check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb); \
    } while (0)

#define INSERT_PTS_ITB(pts) \
    do { \
    dest_itb[0] = 0x21000000; \
    dest_itb[1] = pts; \
    dest_itb[2] = 0x0; \
    dest_itb[3] = 0x0; \
    dest_valid += BXPT_ITB_SIZE; \
    dest_itb += BXPT_ITB_SIZE/4; \
    check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb); \
    } while (0)

#define INSERT_PCROFFSET_ITB(offset) \
    do { \
    dest_itb[0] = 0x22800000; \
    dest_itb[1] = offset; \
    dest_itb[2] = 0x0; \
    dest_itb[3] = 0x0; \
    dest_valid += BXPT_ITB_SIZE; \
    dest_itb += BXPT_ITB_SIZE/4; \
    check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb); \
    } while (0)

#define INSERT_START_MARKER_ITB(offset) \
    do { \
    dest_itb[0] = 0x71000000; \
    dest_itb[1] = offset; \
    dest_itb[2] = 0x0; \
    dest_itb[3] = 0x0; \
    dest_valid += BXPT_ITB_SIZE; \
    dest_itb += BXPT_ITB_SIZE/4; \
    check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb); \
    } while (0)

#define INSERT_START_STOP_PTS_ITB( startFlag, pts ) \
    do { \
    dest_itb[0] = (0x72000000 | (startFlag << 23)); \
    dest_itb[1] = pts; \
    dest_itb[2] = 0x0; \
    dest_itb[3] = 0x0; \
    dest_valid += BXPT_ITB_SIZE; \
    dest_itb += BXPT_ITB_SIZE/4; \
    check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb); \
    } while (0)

#define INSERT_BASE_ENTRY_ITB(base_code, base_address) \
    do { \
    dest_itb[0] = (base_code<<24); \
    dest_itb[1] = base_address; \
    dest_itb[2] = 0x0; \
    dest_itb[3] = 0x0; \
    dest_valid += BXPT_ITB_SIZE; \
    dest_itb += BXPT_ITB_SIZE/4; \
    check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb); \
    } while (0)

#define INSERT_BAND_HOLD_ITB() \
    do { \
    dest_itb[0] = 0x73000000; \
    dest_itb[1] = 0x0; \
    dest_itb[2] = 0x0; \
    dest_itb[3] = 0x0; \
    dest_valid += BXPT_ITB_SIZE; \
    dest_itb += BXPT_ITB_SIZE/4; \
    check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb); \
    } while (0)

static uint32_t * get_next_pts_entry(
    BXPT_RaveCx_Handle DestCtx,
    uint32_t src_valid,
    uint32_t cur_src_itb,
    bool wrap_happened
    )
{
    uint32_t *next_itb;
    uint32_t cnt=0;

    cur_src_itb += BXPT_ITB_SIZE;
    next_itb = (uint32_t*)&DestCtx->SoftRave.src_itb_mem[cur_src_itb - DestCtx->SoftRave.src_itb_base];

    do
    {
        if((cur_src_itb>=src_valid) || (cnt>10))
        {
            if(wrap_happened)
            {
                cur_src_itb = DestCtx->SoftRave.src_itb_base;
                src_valid = BREG_ReadAddr(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + ITB_VALID_PTR_OFFSET );
                wrap_happened = false;
                continue;
            }
            next_itb = NULL;
            break;
        }

        next_itb = (uint32_t*)&DestCtx->SoftRave.src_itb_mem[cur_src_itb - DestCtx->SoftRave.src_itb_base];
        cur_src_itb += BXPT_ITB_SIZE;
        cnt++;
    }
    while(GET_ITB_TYPE(next_itb) != brave_itb_pts_dts);

    return next_itb;
}

static bool GetBandHoldStatus(
    BXPT_RaveCx_Handle DestCtx       /* [in] The destination (soft) context */
    )
{
    uint32_t reg = BREG_Read32( DestCtx->hReg, BCHP_XPT_RAVE_CX_HOLD_CLR_STATUS );
    return reg & ( 1 << DestCtx->SoftRave.SrcContextIndex ) ? true : false;
}

void BXPT_Rave_AdvanceSoftContext(
    BXPT_RaveCx_Handle DestCtx       /* [in] The destination (soft) context */
    )
{
    BMMA_DeviceOffset src_valid,  src_wrap, src_read;
    BMMA_DeviceOffset dest_valid, dest_wrap, dest_read;
    uint32_t reg, overflow;
    BMMA_DeviceOffset src_valid_copy, src_wrap_copy, dest_read_copy;

    BDBG_ASSERT( DestCtx );

    /* Copy over the overflow bits. Decoder needs them. */
    /* CDB */
    reg = BREG_Read32(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + CDB_DEPTH_OFFSET );
    overflow = BCHP_GET_FIELD_DATA( reg, XPT_RAVE_CX0_AV_CDB_DEPTH, CDB_OVERFLOW );
    reg = BREG_Read32(DestCtx->hReg, DestCtx->BaseAddr + CDB_DEPTH_OFFSET );
    reg &= ~( BCHP_MASK( XPT_RAVE_CX0_AV_CDB_DEPTH, CDB_OVERFLOW ) );
    reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_CDB_DEPTH, CDB_OVERFLOW, overflow ) );
    BREG_Write32(DestCtx->hReg, DestCtx->BaseAddr + CDB_DEPTH_OFFSET, reg);

    /* ITB */
    reg = BREG_Read32(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + ITB_DEPTH_OFFSET );
    overflow = BCHP_GET_FIELD_DATA( reg, XPT_RAVE_CX0_AV_ITB_DEPTH, ITB_OVERFLOW );
    reg = BREG_Read32(DestCtx->hReg, DestCtx->BaseAddr + ITB_DEPTH_OFFSET );
    reg &= ~( BCHP_MASK( XPT_RAVE_CX0_AV_ITB_DEPTH, ITB_OVERFLOW ) );
    reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_ITB_DEPTH, ITB_OVERFLOW, overflow ) );
    BREG_Write32(DestCtx->hReg, DestCtx->BaseAddr + ITB_DEPTH_OFFSET, reg);

    /* CDB - always do this before ITB */
    src_valid = BREG_ReadAddr(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + CDB_VALID_PTR_OFFSET );
    src_wrap = BREG_ReadAddr(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + CDB_WRAP_PTR_OFFSET);
    dest_read = BREG_ReadAddr(DestCtx->hReg, DestCtx->BaseAddr + CDB_READ_PTR_OFFSET);

    src_valid_copy =  src_valid;
    src_wrap_copy = src_wrap;
    dest_read_copy = dest_read;

    /* ITB */
    src_valid = BREG_ReadAddr(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + ITB_VALID_PTR_OFFSET );
    src_wrap = BREG_ReadAddr(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + ITB_WRAP_PTR_OFFSET );
    dest_valid = BREG_ReadAddr(DestCtx->hReg, DestCtx->BaseAddr + ITB_VALID_PTR_OFFSET );
    dest_read = BREG_ReadAddr(DestCtx->hReg, DestCtx->BaseAddr + ITB_READ_PTR_OFFSET );
    dest_wrap = BREG_ReadAddr(DestCtx->hReg, DestCtx->BaseAddr + ITB_WRAP_PTR_OFFSET );

    if (DestCtx->SoftRave.mode == BXPT_RaveSoftMode_ePointersOnly)
    {
        /* do nothing - just copy pointers
        this adds host based flow control. useful for debug, peeking into ITB, possibly modifying in-place.
        no ITB entries can be added/removed. */
        BDBG_MSG(("advance rave ITB: " BDBG_UINT64_FMT ", " BDBG_UINT64_FMT ", " BDBG_UINT64_FMT "",
                  BDBG_UINT64_ARG(src_valid), BDBG_UINT64_ARG(src_wrap), BDBG_UINT64_ARG(dest_read) ));
        dest_valid = src_valid;
        dest_wrap = src_wrap;
        src_read = dest_read;
    }
    else
    {
        /* copy ITB */
        uint32_t *dest_itb, *src_itb;
        uint32_t cur_src_itb, dest_valid_actual=dest_valid;
        bool wrap_happened = false;
        BXPT_RaveCx_Handle SrcCtx = DestCtx->SourceContext;

        /* Update the cached data */
        BDBG_ASSERT(SrcCtx);
        BMMA_FlushCache(SrcCtx->mma.itbBlock, DestCtx->SoftRave.src_itb_mem, DestCtx->SoftRave.ItbSize);
        BMMA_FlushCache(DestCtx->mma.itbBlock, DestCtx->SoftRave.dest_itb_mem, DestCtx->SoftRave.ItbSize);

        /* convert from inclusive to exclusive logic before SW processing */
        if (src_valid != DestCtx->SoftRave.src_itb_base)
            src_valid += 1;
        if (dest_valid != DestCtx->SoftRave.dest_itb_base)
            dest_valid += 1;

        /* always work with whole ITB's */
        src_valid -= src_valid % BXPT_ITB_SIZE;
        BDBG_ASSERT(dest_valid % BXPT_ITB_SIZE == 0);


        if(DestCtx->SoftRave.mode == BXPT_RaveSoftMode_eDivX) {
            dest_valid_actual = dest_valid;
            dest_valid= DestCtx->SoftRave.last_dst_itb_valid; /* Use s/w rave cached pointer instead of h/w pointer*/
        }


     /*
        TODO: sw_rave->last_src_itb_valid can be eliminated by using the src READ pointer as the last consumed.
        However, the READ pointer must be treated differently. The definition of READ is that it points
        to the next byte that will be consumed. This creates an ambiguity when READ = VALID (which means
        there is one byte to be read). The AVD decoder avoids this by setting READ to be the last
        byte consumed. This allows the last ITB entry to be read, but avoids READ = VALID.
        This should be done here, but thoroughly tested.
        */
        cur_src_itb = DestCtx->SoftRave.last_src_itb_valid;

        if (src_valid < cur_src_itb) {
            /* our cur_src_itb stopped on the wrap point. just start at base. no wrap.
            or, if we were flushed, src_wrap can go to zero. again, start at base. */
            if (cur_src_itb == src_wrap + 1 || !src_wrap) {
                cur_src_itb = DestCtx->SoftRave.src_itb_base;
            }
            else {
                src_valid = src_wrap + 1;
                wrap_happened = true;
            }
        }

        {
            /* Decoder needs to know if the source context's band-hold is asserted. If it is, insert
            ** a band-hold ITB entry and don't process more data. The band-hold entry is only inserted
            ** once.
            */
            bool SrcBandHold = GetBandHoldStatus( DestCtx );
            if( SrcBandHold && !DestCtx->SoftRave.SrcIsHeld )
            {
                /* Transition into a held state */
                dest_itb = (uint32_t*)&DestCtx->SoftRave.dest_itb_mem[dest_valid - DestCtx->SoftRave.dest_itb_base];
                src_itb = (uint32_t*)&DestCtx->SoftRave.src_itb_mem[cur_src_itb - DestCtx->SoftRave.src_itb_base];
                BDBG_MSG(("Inserting Bandhold "));
                INSERT_BAND_HOLD_ITB();
                DestCtx->SoftRave.SrcIsHeld = true;
            }
             else if ( !SrcBandHold && DestCtx->SoftRave.SrcIsHeld )
            {
                /* Transition out of the held state */
                DestCtx->SoftRave.SrcIsHeld = false;
            }
        }


        BDBG_MSG(("copy ITB: %x --> " BDBG_UINT64_FMT " to " BDBG_UINT64_FMT "", cur_src_itb, BDBG_UINT64_ARG(src_valid), BDBG_UINT64_ARG(dest_valid) ));
        BDBG_MSG(("DestCtx->SoftRave.src_itb_base " BDBG_UINT64_FMT ", DestCtx->SoftRave.dest_itb_base " BDBG_UINT64_FMT "",
                  BDBG_UINT64_ARG(DestCtx->SoftRave.src_itb_base), BDBG_UINT64_ARG(DestCtx->SoftRave.dest_itb_base) ));

        while (cur_src_itb < src_valid)
        {
            dest_itb = (uint32_t*)&DestCtx->SoftRave.dest_itb_mem[dest_valid - DestCtx->SoftRave.dest_itb_base];
            src_itb = (uint32_t*)&DestCtx->SoftRave.src_itb_mem[cur_src_itb - DestCtx->SoftRave.src_itb_base];

            if (DestCtx->SoftRave.mode == BXPT_RaveSoftMode_eCopyItb)
            {
                /* exact copy */
                COPY_ITB();
            }
            else if (DestCtx->SoftRave.mode == BXPT_RaveSoftMode_eIframeTrick)
            {
                uint32_t itb_type;
                itb_type = (src_itb[0]>>24) & 0xFF;

                /* TEMP: This is a hack based on SeqHdr. It macroblocks slightly, but shows the idea. */
                switch (itb_type) {
                case brave_itb_video_start_code:
                    /* TODO: this isn't complete. we should search EVERY startcode in the ITB */
                    switch ((src_itb[0]>>8) & 0xFF) {
                    case 0xB3:
                        /* when we find a SeqHdr, start copying all ITB's */
                        DestCtx->SoftRave.discard_till_next_gop = false;
                        DestCtx->SoftRave.discarding = false;
                        COPY_ITB();
                        break;
                    case 0x00: /* first slice */
                        /* when we find the first 00 slice after a SeqHdr, keep copying,
                        but the second 00 slice should not be copied. */
                        if (!DestCtx->SoftRave.discard_till_next_gop) {
                            /* allow until next 0x00 */
                            COPY_ITB();
                            DestCtx->SoftRave.discard_till_next_gop = true;
                        }
                        else {
                            DestCtx->SoftRave.discarding = true;
                            /* TODO: see note about about EVERY startcode */
                            INSERT_TERMINATION_ITB(src_itb[0] & 0xFF);
                        }
                        break;
                    default:
                        if (!DestCtx->SoftRave.discarding) {
                            COPY_ITB();
                        }
                    }
                    break;
                default:
                    COPY_ITB();
                    break;
                }
            }
            else if (DestCtx->SoftRave.mode == BXPT_RaveSoftMode_eVc1SimpleMain)
            {
                uint32_t itb_type;
                itb_type = (src_itb[0]>>24) & 0xFF;
                switch (itb_type) {
                case brave_itb_base_address: /* base address */
                case brave_itb_base_address_40bit: /* base address, 40-bit support. */
                    DestCtx->SoftRave.BaseOpCode = itb_type;
                    /* cache the last base_address for creating SC entries */
                    DestCtx->SoftRave.last_base_address = src_itb[1];
                    BDBG_MSG(("base_address %x", DestCtx->SoftRave.last_base_address));
                    COPY_ITB();
                    break;
                case brave_itb_pts_dts:
                    /* transform and copy */
                    if (src_itb[1] == 0xFFFFFFFF)
                    {
                      uint32_t *next_itb = get_next_pts_entry(DestCtx, src_valid, cur_src_itb, wrap_happened);

                      if(!next_itb){
                          DestCtx->SoftRave.insufficient_itb_info = true;
                          goto skip_itb_and_restart;
                      }

                      dest_itb[0] = next_itb[0];
                      dest_itb[1] = next_itb[1];
                      dest_itb[2] = next_itb[2];
                      dest_itb[3] = next_itb[3];

                      dest_valid += BXPT_ITB_SIZE;
                      dest_itb += BXPT_ITB_SIZE/4;
                      check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);
                      DestCtx->SoftRave.adjust_pts = true;

                      /* sequence SC - add base address and start code ITB's */
                      BDBG_MSG(("sequence SC PTS %x", DestCtx->SoftRave.last_base_address));

                      dest_itb[0] = 0x00000001; /* 1 Byte Extraction Entry */
                      dest_itb[1] = 0x0F0300FF; /* 0F start code, 03 offset from base_address followed by  end of start codes */

                      dest_valid += BXPT_ITB_SIZE;
                      dest_itb += BXPT_ITB_SIZE/4;
                      check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);
                  }
                else if (((src_itb[0]&0x00008000)>>15) && (src_itb[2] == 0xFFFFFFFF))
                {
                   dest_itb[0] = 0x00000001; /* 1 Byte Extraction Entry */
                   dest_itb[1] = 0x0A0300FF; /* 0F start code, 03 offset from base_address followed by  end of start codes */
                   dest_valid += BXPT_ITB_SIZE;
                   dest_itb += BXPT_ITB_SIZE/4;
                   check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);
                }
                else
                {
                   /* copy first so we get a PTS */
                   if(!DestCtx->SoftRave.adjust_pts)
                     COPY_ITB();
                   else
                     DestCtx->SoftRave.adjust_pts = false;

                   /* frame SC - add base address and start code ITB's */
                   BDBG_MSG(("frame SC PTS %x", DestCtx->SoftRave.last_base_address));
                   dest_itb[0] = 0x00000001; /* 1 Byte Extraction Entry */
                   dest_itb[1] = 0x0D0300FF; /* 0D start code, 03 offset from base_address followed by  end of start codes */
                   dest_valid += BXPT_ITB_SIZE;
                   dest_itb += BXPT_ITB_SIZE/4;
                   check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);
                }
                    break;
                default:
                    /* ignore - don't copy */
                    break;
                }
            }
            else if (DestCtx->SoftRave.mode == BXPT_RaveSoftMode_eDivX_311)
            {
            uint32_t itb_type;
            itb_type = (src_itb[0]>>24) & 0xFF;
            switch (itb_type)
              {
              case brave_itb_base_address: /* base address */
               case brave_itb_base_address_40bit: /* base address, 40-bit support. */
                DestCtx->SoftRave.BaseOpCode = itb_type;

            /* cache the last base_address for creating SC entries */
            DestCtx->SoftRave.last_base_address = src_itb[1];
            BDBG_MSG(("base_address %x", DestCtx->SoftRave.last_base_address));
            COPY_ITB();
            break;

              case brave_itb_pts_dts:

            /* transform and copy */
            if (src_itb[1] == 0xFFFFFFFF)
              {
                  /* sequence SC - add base address and start code ITB's */
                  BDBG_MSG(("sequence SC PTS %x", DestCtx->SoftRave.last_base_address));
                  dest_itb[0] = 0x00000007; /* 3 Byte start code Entry */
                  dest_itb[1] = 0x00032007;
                  dest_itb[2] = 0xB22100FF;
                  dest_itb[3] = 0;

                  dest_valid += BXPT_ITB_SIZE;
                  dest_itb += BXPT_ITB_SIZE/4;
                  check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);
              }
            else if (((src_itb[0]&0x00008000)>>15) && (src_itb[2] == 0xFFFFFFFF))
              {
                  dest_itb[0] = 0x00000001; /* 1 Byte Extraction Entry */
                  dest_itb[1] = 0xB10300FF; /* 0F start code, 03 offset from base_address followed by  end of start codes */
                  dest_itb[2] = 0;
                  dest_itb[3] = 0;

                  dest_valid += BXPT_ITB_SIZE;
                  dest_itb += BXPT_ITB_SIZE/4;
                  check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);
              }
            else
              {
                  /* copy first so we get a PTS */
                  COPY_ITB();

                  /* frame SC - add base address and start code ITB's */
                  BDBG_MSG(("frame SC PTS %x", DestCtx->SoftRave.last_base_address));
                  dest_itb[0] = 0x00000001; /* 1 byte Extraction Entry */
                  dest_itb[1] = 0xB60300FF; /* video start code, B6 start code, 03 offset from base_address, followed by  end of start codes */
                  dest_itb[2] = 0;
                  dest_itb[3] = 0;

                  dest_valid += BXPT_ITB_SIZE;
                  dest_itb += BXPT_ITB_SIZE/4;
                  check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);
              }
            break;
              default:
                    /* ignore - don't copy */
                    break;
                }
            }
            else if (DestCtx->SoftRave.mode == BXPT_RaveSoftMode_eDivX)
            {
               uint32_t itb_type;
               uint64_t prev_frame_size;

               itb_type = (src_itb[0]>>24) & 0xFF;

               if((dest_valid<dest_read) && ((dest_valid+(4*BXPT_ITB_SIZE))>dest_read)){
                   break;
               }

            switch (itb_type) {
            case brave_itb_base_address:
            case brave_itb_base_address_40bit: /* base address, 40-bit support. */
                DestCtx->SoftRave.BaseOpCode = itb_type;
                if(src_itb[1] >= DestCtx->SoftRave.last_base_address){
                    prev_frame_size = src_itb[1] - DestCtx->SoftRave.last_base_address;
                } else {
                    BMMA_DeviceOffset cdb_wrap_ptr, cdb_base_ptr;
                    cdb_base_ptr = BREG_ReadAddr(DestCtx->hReg, CDB_BASE_PTR_OFFSET+DestCtx->SoftRave.SrcBaseAddr);
                    cdb_wrap_ptr = BREG_ReadAddr(DestCtx->hReg, CDB_WRAP_PTR_OFFSET+DestCtx->SoftRave.SrcBaseAddr);
                    prev_frame_size = (cdb_wrap_ptr - DestCtx->SoftRave.last_base_address) + (src_itb[1] - cdb_base_ptr) + 1;
                }
                if(DestCtx->SoftRave.b_frame_found && (prev_frame_size<=8)){
                    dest_valid -= BXPT_ITB_SIZE;
                    if(dest_valid < DestCtx->SoftRave.dest_itb_base)
                        dest_valid = dest_wrap - BXPT_ITB_SIZE + 1;
                    dest_valid -= BXPT_ITB_SIZE;
                    if(dest_valid < DestCtx->SoftRave.dest_itb_base)
                        dest_valid = dest_wrap - BXPT_ITB_SIZE + 1;
                    dest_itb = (uint32_t*)&DestCtx->SoftRave.dest_itb_mem[dest_valid - DestCtx->SoftRave.dest_itb_base];
                    INSERT_TERMINATION_ITB(1);
                    if( DestCtx->SoftRave.P_frame_pts )
                      *(DestCtx->SoftRave.P_frame_pts) = DestCtx->SoftRave.last_pts_dts;
                    DestCtx->SoftRave.b_frame_found = false;
                }
                COPY_ITB();
                DestCtx->SoftRave.last_base_address = src_itb[1];
                DestCtx->SoftRave.last_entry_type = itb_type;
                break;
            case brave_itb_pts_dts:
                if(!DestCtx->SoftRave.b_frame_found){
                    DestCtx->SoftRave.P_frame_pts = &dest_itb[1];
                    DestCtx->SoftRave.flush_cnt++;
                }
                COPY_ITB();

                DestCtx->SoftRave.last_pts_dts = src_itb[1];
                DestCtx->SoftRave.last_entry_type = itb_type;
                break;
            case brave_itb_video_start_code:
                if( (((src_itb[1]>>8) & 0xFF) > 0x00) && (((src_itb[1]>>8) & 0xFF) < 0xb6) ){
                    DestCtx->SoftRave.sequence_hdr_found = true;
                } else {
                    DestCtx->SoftRave.sequence_hdr_found = false;
                }

                if(DestCtx->SoftRave.last_entry_type == brave_itb_base_address && !DestCtx->SoftRave.sequence_hdr_found && !DestCtx->SoftRave.prev_sequence_hdr_found){
                    if(DestCtx->SoftRave.last_pts_dts){
                        dest_itb[0] = (brave_itb_pts_dts<<24);
                        dest_itb[1] = DestCtx->SoftRave.last_pts_dts;
                        dest_itb[2] = 0;
                        dest_itb[3] = 0;
                        dest_valid += BXPT_ITB_SIZE;
                        dest_itb += BXPT_ITB_SIZE/4;
                        check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);

                        DestCtx->SoftRave.last_pts_dts = 0;
                    }
                    COPY_ITB();

                    DestCtx->SoftRave.b_frame_found=true;
                }
                else if(((src_itb[1]>>8) & 0xFF) == 0xb6 && ((src_itb[1]>>24) & 0xFF) == 0xb6){
                    /* Check for entry with 2 start codes*/

                    /* Insert SC Entry*/
                    dest_itb[0] = 0x00000001;
                    dest_itb[1] = (src_itb[1] & 0xffff0000) | 0x00ff;
                    dest_itb[2] = 0;
                    dest_itb[3] = 0;
                    dest_valid += BXPT_ITB_SIZE;
                    dest_itb += BXPT_ITB_SIZE/4;
                    check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);

                    /* Insert Base Address Entry*/
                    dest_itb[0] = (brave_itb_base_address<<24);
                    dest_itb[1] = DestCtx->SoftRave.last_base_address;
                    dest_itb[2] = 0;
                    dest_itb[3] = 0;
                    dest_valid += BXPT_ITB_SIZE;
                    dest_itb += BXPT_ITB_SIZE/4;
                    check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);

                    /* Insert PTS Entry*/
                    dest_itb[0] = (brave_itb_pts_dts<<24);
                    dest_itb[1] = DestCtx->SoftRave.last_pts_dts;
                    dest_itb[2] = 0;
                    dest_itb[3] = 0;
                    dest_valid += BXPT_ITB_SIZE;
                    dest_itb += BXPT_ITB_SIZE/4;
                    check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);

                    /* Insert SC Entry*/
                    dest_itb[0] = 0x00000001;
                    dest_itb[1] =  ((src_itb[1] &0xffff)<<16) | 0x00ff;
                    dest_valid += BXPT_ITB_SIZE;
                    dest_itb += BXPT_ITB_SIZE/4;
                    check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);

                    DestCtx->SoftRave.b_frame_found=true;
                }
                else {
                    COPY_ITB();
                    if(DestCtx->SoftRave.flush_cnt>=2){
                        dest_valid_actual = DestCtx->SoftRave.last_dest_valid;
                        DestCtx->SoftRave.flush_cnt--;
                    }

                    DestCtx->SoftRave.last_dest_valid = dest_valid;
                }

                if( (((src_itb[1]>>8) & 0xFF) > 0x00) && (((src_itb[1]>>8) & 0xFF) < 0xb6) ){
                    DestCtx->SoftRave.prev_sequence_hdr_found = true;
                } else {
                    DestCtx->SoftRave.prev_sequence_hdr_found = false;
                }

                /*Reached EOS. Flush out all ITBs*/
                if(((src_itb[1]>>24) & 0xFF) == 0xb1){
                    dest_valid_actual = dest_valid;
                }
                DestCtx->SoftRave.last_entry_type = itb_type;
                break;
            default:
                break;
            }
            }
            else if (DestCtx->SoftRave.mode == BXPT_RaveSoftMode_eDivX_noReorder) {
                uint32_t itb_type, prev_frame_size;

                itb_type = (src_itb[0]>>24) & 0xFF;

                if((dest_valid<dest_read) && ((dest_valid+(4*BXPT_ITB_SIZE))>dest_read)){
                    break;
                }

                switch (itb_type) {
                case brave_itb_base_address:
                case brave_itb_base_address_40bit: /* base address, 40-bit support. */
                    DestCtx->SoftRave.BaseOpCode = itb_type;
                    if(src_itb[1] >= DestCtx->SoftRave.last_base_address){
                        prev_frame_size = src_itb[1] - DestCtx->SoftRave.last_base_address;
                    } else {
                        uint32_t cdb_wrap_ptr, cdb_base_ptr;
                        cdb_base_ptr = BREG_Read32(DestCtx->hReg, CDB_BASE_PTR_OFFSET+DestCtx->SoftRave.SrcBaseAddr);
                        cdb_wrap_ptr = BREG_Read32(DestCtx->hReg, CDB_WRAP_PTR_OFFSET+DestCtx->SoftRave.SrcBaseAddr);
                        prev_frame_size = (cdb_wrap_ptr - DestCtx->SoftRave.last_base_address) + (src_itb[1] - cdb_base_ptr) + 1;
                    }

                    /* if the prev frame was the not-coded frame */
                    if(DestCtx->SoftRave.b_frame_found && (prev_frame_size<=8)){
                        /* ... back-up two ITB entries (start code and PTS) */
                        dest_valid -= BXPT_ITB_SIZE;
                        if(dest_valid < DestCtx->SoftRave.dest_itb_base)
                            dest_valid = dest_wrap - BXPT_ITB_SIZE + 1;
                        dest_valid -= BXPT_ITB_SIZE;
                        if(dest_valid < DestCtx->SoftRave.dest_itb_base)
                            dest_valid = dest_wrap - BXPT_ITB_SIZE + 1;
                        /* ... and insert the termination ITB */
                        dest_itb = (uint32_t*)&DestCtx->SoftRave.dest_itb_mem[dest_valid - DestCtx->SoftRave.dest_itb_base];
                        INSERT_TERMINATION_ITB(1);
                        DestCtx->SoftRave.b_frame_found = false;
                    }
                    COPY_ITB();
                    DestCtx->SoftRave.last_base_address = src_itb[1];
                    DestCtx->SoftRave.last_entry_type = itb_type;
                    break;
                case brave_itb_pts_dts:
                    if (DestCtx->SoftRave.b_frame_found) {
                        /* write this PTS value to the previous B-frame, and save the address of this PTS entry */
                        if (DestCtx->SoftRave.P_frame_pts) {
                            *DestCtx->SoftRave.P_frame_pts = src_itb[1];
                            DestCtx->SoftRave.P_frame_pts = &dest_itb[1];
                        }
                    }
                    else {
                        DestCtx->SoftRave.flush_cnt++;
                    }

                    COPY_ITB();

                    DestCtx->SoftRave.last_pts_dts = src_itb[1];
                    DestCtx->SoftRave.last_entry_type = itb_type;
                    break;
                case brave_itb_video_start_code:
                    if(DestCtx->SoftRave.last_entry_type == brave_itb_base_address && !DestCtx->SoftRave.sequence_hdr_found){

                        /* this is the packed B frame. insert the PTS entry, and save the address to override the PTS value */
                        dest_itb[0] = (brave_itb_pts_dts<<24);
                        dest_itb[1] = DestCtx->SoftRave.last_pts_dts;
                        DestCtx->SoftRave.P_frame_pts = &dest_itb[1];
                        dest_itb[2] = 0;
                        dest_itb[3] = 0;
                        dest_valid += BXPT_ITB_SIZE;
                        dest_itb += BXPT_ITB_SIZE/4;
                        check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);

                        COPY_ITB();
                        DestCtx->SoftRave.b_frame_found=true;
                    }
                    else if(((src_itb[1]>>8) & 0xFF) == 0xb6 && ((src_itb[1]>>24) & 0xFF) == 0xb6){
                        /* Check for entry with 2 start codes */

                        /* Insert SC Entry */
                        dest_itb[0] = 0x00000001;
                        dest_itb[1] = (src_itb[1] & 0xffff0000) | 0x00ff;
                        dest_itb[2] = 0;
                        dest_itb[3] = 0;
                        dest_valid += BXPT_ITB_SIZE;
                        dest_itb += BXPT_ITB_SIZE/4;
                        check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);

                        /* Insert Base Address Entry - this is the B frame */
                        dest_itb[0] = (brave_itb_base_address<<24);
                        dest_itb[1] = DestCtx->SoftRave.last_base_address;
                        dest_itb[2] = 0;
                        dest_itb[3] = 0;
                        dest_valid += BXPT_ITB_SIZE;
                        dest_itb += BXPT_ITB_SIZE/4;
                        check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);

                        /* Insert PTS Entry */
                        dest_itb[0] = (brave_itb_pts_dts<<24);
                        dest_itb[1] = DestCtx->SoftRave.last_pts_dts;
                        DestCtx->SoftRave.P_frame_pts = &dest_itb[1]; /* save address to override with next PTS */
                        dest_itb[2] = 0;
                        dest_itb[3] = 0;
                        dest_valid += BXPT_ITB_SIZE;
                        dest_itb += BXPT_ITB_SIZE/4;
                        check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);

                        /* Insert SC Entry */
                        dest_itb[0] = 0x00000001;
                        dest_itb[1] =  ((src_itb[1] &0xffff)<<16) | 0x00ff;
                        dest_valid += BXPT_ITB_SIZE;
                        dest_itb += BXPT_ITB_SIZE/4;
                        check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);

                        DestCtx->SoftRave.b_frame_found=true;
                    }
                    else {
                        COPY_ITB();
                        if(DestCtx->SoftRave.flush_cnt>=2){
                            dest_valid_actual = DestCtx->SoftRave.last_dest_valid;
                            DestCtx->SoftRave.flush_cnt--;
                        }

                        DestCtx->SoftRave.last_dest_valid = dest_valid;
                    }

                    if( (((src_itb[1]>>8) & 0xFF) > 0x00) && (((src_itb[1]>>8) & 0xFF) < 0xb6) ){
                        DestCtx->SoftRave.sequence_hdr_found = true;
                    } else {
                        DestCtx->SoftRave.sequence_hdr_found = false;
                    }

                    /*Reached EOS. Flush out all ITBs*/
                    if(((src_itb[1]>>24) & 0xFF) == 0xb1){
                        dest_valid_actual = dest_valid;
                    }
                    DestCtx->SoftRave.last_entry_type = itb_type;
                    break;
                default:
                    break;
                }
            }

            else if ( DestCtx->SoftRave.mode == BXPT_RaveSoftMode_eDynamic_Splice)
            {

                uint32_t itb_type;
                itb_type = (src_itb[0]>>24) & 0xFF;

                if((dest_valid<dest_read) && ((dest_valid+(4*BXPT_ITB_SIZE))>dest_read)){
                    break;
                }

            /*
            ** For audio, insert the Stop PTS marker into the dest as soon as we get the Stop PTS command.
            ** We only want to insert it once, but don't alter the SpliceStopPTSFlag state var below.
            **
            ** - Add a flag to the softrave handle
            ** - Set to true when BXPT_Rave_StopPTS() is called.
            ** - For live, insert the Stop Marker here (before the switch), clear the flag
            */
            if( DestCtx->SoftRave.InsertStopPts )
            {
                DestCtx->SoftRave.InsertStopPts = false;
                BDBG_MSG(("Inserting Stop PTS for context %p, PTS %u", (void *) DestCtx, DestCtx->SoftRave.splice_stop_PTS));
                INSERT_START_STOP_PTS_ITB( 0, DestCtx->SoftRave.splice_stop_PTS );
            }

            /*
            ** Start PTS marker hanlding is similar, except we're using the Start PTS command.
            */
            if( DestCtx->SoftRave.InsertStartPts )
            {
                DestCtx->SoftRave.InsertStartPts = false;
                BDBG_MSG(("Inserting Start PTS for context %p, PTS %u", (void *) DestCtx, DestCtx->SoftRave.splice_start_PTS));
                INSERT_START_STOP_PTS_ITB( 1, DestCtx->SoftRave.splice_start_PTS );
            }

                switch (itb_type) {

                case brave_itb_pts_dts:

                    /*check if a start PTS is programmed*/
                    if (DestCtx->SoftRave.splice_start_PTS != 0 && DestCtx->SoftRave.SpliceStartPTSFlag == true)
                    {
                    if (((DestCtx->SoftRave.splice_start_PTS <= src_itb[1] )
                            && ( src_itb[1] < (DestCtx->SoftRave.splice_start_PTS_tolerance +DestCtx->SoftRave.splice_start_PTS)) )
                                &&(DestCtx->SoftRave.splice_state == SoftRave_SpliceState_Discard))
                        {
                        BDBG_MSG(("Inserting Base Address ITB for context %p, Base Address 0x%08X, ", (void *) DestCtx, DestCtx->SoftRave.last_base_address));
                        INSERT_BASE_ENTRY_ITB(DestCtx->SoftRave.BaseOpCode, DestCtx->SoftRave.last_base_address);
                        INSERT_PCROFFSET_ITB(DestCtx->SoftRave.splice_last_pcr_offset);
                        /* Insert the start marker into the ITB only once. */
                        if( !DestCtx->SoftRave.StartMarkerInserted )
                        {
                            DestCtx->SoftRave.StartMarkerInserted = true;
                            BDBG_MSG(("Inserting Start marker for context %p, PTS %u", (void *) DestCtx, DestCtx->SoftRave.splice_start_PTS));
                            INSERT_START_MARKER_ITB( 0 );
                        }

                            /*Start PTS reached.Change state to Copy and copy the ITB entry*/
                            DestCtx->SoftRave.splice_state = SoftRave_SpliceState_Copy;
                            DestCtx->SoftRave.splice_start_PTS = 0;
                            DestCtx->SoftRave.SpliceStartPTSFlag = false;
                            /*call callback if programmed*/
                            BDBG_MSG(("Start PTS Marker Reached PTS is %u ", src_itb[1]));
                            if (DestCtx->SoftRave.SpliceStartPTSCB != NULL )
                                DestCtx->SoftRave.SpliceStartPTSCB(DestCtx->SoftRave.SpliceStartPTSCBParam, src_itb[1]);
                        }
                    }
                    /*check if a stop PTS is programmed*/
                    if (DestCtx->SoftRave.splice_stop_PTS != 0 && DestCtx->SoftRave.SpliceStopPTSFlag == true)
                    {
                        if (((DestCtx->SoftRave.splice_stop_PTS <= src_itb[1] )
                            && ( src_itb[1] < (DestCtx->SoftRave.splice_stop_PTS_tolerance +DestCtx->SoftRave.splice_stop_PTS)) )
                                &&(DestCtx->SoftRave.splice_state == SoftRave_SpliceState_Copy))
                        {
                        /* Insert the stop marker into the ITB only once. */
                        if( !DestCtx->SoftRave.StopMarkerInserted )
                        {
                            DestCtx->SoftRave.StopMarkerInserted = true;
                            BDBG_MSG(("Inserting Termination ITB for context %p, PTS %u", (void *) DestCtx, DestCtx->SoftRave.splice_stop_PTS));
                            INSERT_TERMINATION_ITB(src_itb[0] & 0xFF);
                        }

                            /*Start PTS reached.Change state to Copy and copy the ITB entry*/
                            DestCtx->SoftRave.splice_state = SoftRave_SpliceState_Discard;
                            DestCtx->SoftRave.splice_stop_PTS = 0;
                            DestCtx->SoftRave.SpliceStopPTSFlag = false;
                            BDBG_MSG(("Stop PTS Marker Reached PTS is %u ", src_itb[1]));
                            /*call callback if programmed*/
                            if (DestCtx->SoftRave.SpliceStopPTSCB != NULL )
                                DestCtx->SoftRave.SpliceStopPTSCB(DestCtx->SoftRave.SpliceStopPTSCBParam, src_itb[1] );
                        }

                    }

                    /*check if monitor PTS is programmed*/
                    if (DestCtx->SoftRave.splice_monitor_PTS != 0 && DestCtx->SoftRave.SpliceMonitorPTSFlag == true)
                    {
                        /*Use monitor PTS compare*/
                        if ((DestCtx->SoftRave.splice_monitor_PTS <= src_itb[1] )
                                && ( src_itb[1] < (DestCtx->SoftRave.splice_monitor_PTS_tolerance +DestCtx->SoftRave.splice_monitor_PTS)) )
                            {
                                /*call back monitor if programmed*/
                                if (DestCtx->SoftRave.SpliceMonitorPTSCB != NULL )
                                        DestCtx->SoftRave.SpliceMonitorPTSCB(DestCtx->SoftRave.SpliceMonitorPTSCBParam, src_itb[1]);
                                DestCtx->SoftRave.splice_monitor_PTS = 0;
                                DestCtx->SoftRave.SpliceMonitorPTSFlag = false;
                            }
                    }

                    if (DestCtx->SoftRave.splice_state == SoftRave_SpliceState_Copy)
                    {

                            /*check if we need to send PCR OFFSET entry (required Only for 7401)*/
                            if (    DestCtx->SoftRave.splice_pcr_offset != 0 &&
                                    DestCtx->SoftRave.SpliceModifyPCROffsetFlag == true)
                            {
/*                          BDBG_MSG(("Inserting PCR offset for context %p, PCR offset %d", (DestCtx->SoftRave.splice_pcr_offset +DestCtx->SoftRave.splice_last_pcr_offset), DestCtx)); */
                                INSERT_PCROFFSET_ITB(DestCtx->SoftRave.splice_pcr_offset +DestCtx->SoftRave.splice_last_pcr_offset );
                            }

                                            /*check if we need to send PCR OFFSET entry (required Only for 7401)*/
                            if (    DestCtx->SoftRave.splice_pts_offset != 0 &&
                                    DestCtx->SoftRave.SpliceModifyPTSFlag == true)
                            {
/*                          BDBG_MSG(("Inserting PCR offset for context %p, PCR offset %d", (DestCtx->SoftRave.splice_pcr_offset +DestCtx->SoftRave.splice_last_pcr_offset), DestCtx)); */
                                INSERT_PTS_ITB(DestCtx->SoftRave.splice_pts_offset + src_itb[1] );
                            }else
                            {

                                COPY_ITB();
                            }

                    }

                    break;
                case brave_itb_pcr_offset:

                    if (DestCtx->SoftRave.splice_state == SoftRave_SpliceState_Copy)
                    {


                        if (    DestCtx->SoftRave.splice_pcr_offset != 0 &&
                                    DestCtx->SoftRave.SpliceModifyPCROffsetFlag == true)
                        {
                            /*This will be required for 7405/7335/etc for 7401 we need to create an entry*/
    #if 0
                            /*Modify and copy */
                            /* src_itb[0] |= ((1 <<23) &0x00800000); */
                            /* src_itb[1] =  src_itb[1]+DestCtx->SoftRave.splice_pcr_offset; */
                            BDBG_ERR((" PCR after offset is %x", src_itb[1]));
    #else
                            ;   /* Placate compiler */
    #endif
                        }
                        else
                        {
                            DestCtx->SoftRave.splice_last_pcr_offset = src_itb[1];
                            COPY_ITB();
                        }
                    }
                    break;
                case brave_itb_btp:
                    {
                    uint32_t sub_command;
                    sub_command = (src_itb[0] >>  8) & 0xff;
                    switch (sub_command)
                    {

                        case brave_itb_splice_start_marker:
                            DestCtx->SoftRave.splice_state = SoftRave_SpliceState_Copy;
                        BDBG_MSG(("Inserting Base Address ITB for context %p, Base Address 0x%08X, ", (void *) DestCtx, DestCtx->SoftRave.last_base_address));
                        INSERT_BASE_ENTRY_ITB(DestCtx->SoftRave.BaseOpCode, DestCtx->SoftRave.last_base_address);
                        BDBG_MSG(("Got brave_itb_splice_start_marker,  Inserting Start PTS ITB & Start Marker for context %p, PTS %u", (void *) DestCtx, src_itb[3]));
                        INSERT_START_STOP_PTS_ITB( 1, src_itb[3]);
                        INSERT_START_MARKER_ITB( 0 );
                            break;

                        case brave_itb_splice_stop_marker:
                            DestCtx->SoftRave.splice_state = SoftRave_SpliceState_Discard;
                        BDBG_MSG(("Got brave_itb_splice_stop_marker,  Inserting Stop PTS ITB & Termination ITB for context %p, PTS %u", (void *) DestCtx, src_itb[3]));
                        INSERT_START_STOP_PTS_ITB( 0, src_itb[3]);
                        INSERT_TERMINATION_ITB(src_itb[0] & 0xFF);
                            INSERT_PCROFFSET_ITB(0);
                            break;

                        case brave_itb_splice_pcr_offset_marker:
                            if (src_itb[3] != 0)
                            {
                                DestCtx->SoftRave.splice_pcr_offset +=  src_itb[3];
                            BDBG_MSG(("Got brave_itb_splice_pcr_offset_marker, Context %p, PCR offset %d", (void *) DestCtx, DestCtx->SoftRave.splice_pcr_offset));
                                DestCtx->SoftRave.SpliceModifyPCROffsetFlag = true;
                            }else
                            {
                                DestCtx->SoftRave.splice_pcr_offset =  src_itb[3];
                            BDBG_MSG(("Got brave_itb_splice_pcr_offset_marker, Context %p, PCR offset %d", (void *) DestCtx, DestCtx->SoftRave.splice_pcr_offset));
                                DestCtx->SoftRave.SpliceModifyPCROffsetFlag = false;
                            }
                            break;

                        case brave_itb_splice_pts_marker:
                            if (src_itb[3] != 0)
                            {
                                DestCtx->SoftRave.splice_pts_offset +=  src_itb[3];
                            BDBG_MSG(("Got brave_itb_splice_pcr_offset_marker, Context %p, PCR offset %d", (void *) DestCtx, DestCtx->SoftRave.splice_pts_offset));
                                DestCtx->SoftRave.SpliceModifyPTSFlag = true;
                            }else
                            {
                                DestCtx->SoftRave.splice_pts_offset =  src_itb[3];
                            BDBG_MSG(("Got brave_itb_splice_pcr_offset_marker, Context %p, PCR offset %d", (void *) DestCtx, DestCtx->SoftRave.splice_pts_offset));
                                DestCtx->SoftRave.SpliceModifyPTSFlag = false;
                            }
                            break;

                        default:
                            break;

                    }
                }
                break;

                default:
                    if (DestCtx->SoftRave.splice_state ==  SoftRave_SpliceState_Copy)
                    {
                        COPY_ITB();
                    }
                   if (itb_type == brave_itb_base_address || itb_type == brave_itb_base_address_40bit)
                   {
                       DestCtx->SoftRave.last_base_address =  src_itb[1];
                       DestCtx->SoftRave.BaseOpCode = itb_type;
                   }
                    break;


                }
            }

            else
            {
                BDBG_ERR(("unknown SW RAVE mode"));
                BDBG_ASSERT(0);
            }

            /* handle wrap */
            cur_src_itb += BXPT_ITB_SIZE;

            if (wrap_happened && cur_src_itb >= src_wrap+1)
            {
                BDBG_ASSERT(cur_src_itb == src_wrap+1);
                cur_src_itb = DestCtx->SoftRave.src_itb_base;
                src_valid = BREG_ReadAddr(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + ITB_VALID_PTR_OFFSET ) + 1;
            }

skip_itb_and_restart:
            if(DestCtx->SoftRave.insufficient_itb_info)
            {
              DestCtx->SoftRave.insufficient_itb_info = false;
              break;
            }
        }

        DestCtx->SoftRave.last_src_itb_valid = src_valid = cur_src_itb;
        if (DestCtx->SoftRave.mode == BXPT_RaveSoftMode_eDivX) {
            DestCtx->SoftRave.last_dst_itb_valid = dest_valid;
            dest_valid = dest_valid_actual;
        }

        /* convert back to inclusive logic before writing to HW */
        if (src_valid != DestCtx->SoftRave.src_itb_base)
            src_valid -= 1;
        if (dest_valid != DestCtx->SoftRave.dest_itb_base)
            dest_valid -= 1;

        /* mark that we've consumed all but one */
        src_read = src_valid + 1 - BXPT_ITB_SIZE; /* TODO: see above, this must be fixed for proper CRC */
        if (src_read < DestCtx->SoftRave.src_itb_base)
            src_read = DestCtx->SoftRave.src_itb_base;
    }

    /* Write back updates to device memory */
    BMMA_FlushCache(DestCtx->mma.itbBlock, DestCtx->SoftRave.dest_itb_mem, DestCtx->SoftRave.ItbSize);

    BKNI_EnterCriticalSection();
    /* produce: advance the dest VALID and WRAP pointers.
    always write WRAP before VALID, always produce on dest before consuming on src */
    BREG_WriteAddr(DestCtx->hReg, DestCtx->BaseAddr + CDB_WRAP_PTR_OFFSET, src_wrap_copy);
    BREG_WriteAddr(DestCtx->hReg, DestCtx->BaseAddr + CDB_VALID_PTR_OFFSET, src_valid_copy);

    /* consume: advance the src READ pointer */
    BREG_WriteAddr(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + CDB_READ_PTR_OFFSET, dest_read_copy);

    /* produce: advance the dest VALID and WRAP pointers */
    BREG_WriteAddr(DestCtx->hReg, DestCtx->BaseAddr + ITB_WRAP_PTR_OFFSET, dest_wrap);
    BREG_WriteAddr(DestCtx->hReg, DestCtx->BaseAddr + ITB_VALID_PTR_OFFSET, dest_valid);

    /* consume: advance the src READ pointer */
    BREG_WriteAddr(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + ITB_READ_PTR_OFFSET, src_read);
    BKNI_LeaveCriticalSection();
}

BERR_Code BXPT_Rave_ResetSoftContext(
    BXPT_RaveCx_Handle hCtx,
    BXPT_RaveSoftMode Mode
    )
{
    /* Save some stuff before we bzero the struct */
    uint32_t SrcBaseAddr = hCtx->SoftRave.SrcBaseAddr;
    uint32_t SrcItbBase = hCtx->SoftRave.src_itb_base;
    uint32_t DestItbBase = hCtx->SoftRave.dest_itb_base;
    uint8_t *src_itb_mem = hCtx->SoftRave.src_itb_mem;
    uint8_t *dest_itb_mem = hCtx->SoftRave.dest_itb_mem;
    uint8_t SourceContextIndex = hCtx->SoftRave.SrcContextIndex;
    size_t ItbSize = hCtx->SoftRave.ItbSize;

    BKNI_Memset( (void *) &hCtx->SoftRave, 0, sizeof( hCtx->SoftRave ));

    hCtx->SoftRave.SrcContextIndex = SourceContextIndex;
    hCtx->SoftRave.ItbSize = ItbSize;
    hCtx->SoftRave.SrcBaseAddr = SrcBaseAddr;
    hCtx->SoftRave.mode = Mode;
    hCtx->SoftRave.last_base_address = BREG_ReadAddr(hCtx->hReg, CDB_BASE_PTR_OFFSET+hCtx->SoftRave.SrcBaseAddr);

    hCtx->SoftRave.b_frame_found = false;
    hCtx->SoftRave.last_dest_valid = DestItbBase;
    hCtx->SoftRave.flush_cnt=0;

    hCtx->SoftRave.insufficient_itb_info = false;
    hCtx->SoftRave.adjust_pts = false;
    hCtx->SoftRave.sequence_hdr_found = false;
    hCtx->SoftRave.prev_sequence_hdr_found = false;

    ResetContextPointers( hCtx );

    hCtx->SoftRave.src_itb_base = SrcItbBase;
    hCtx->SoftRave.dest_itb_base = DestItbBase;
    hCtx->SoftRave.last_src_itb_valid = SrcItbBase;
    hCtx->SoftRave.last_dst_itb_valid = DestItbBase;
    hCtx->SoftRave.src_itb_mem = src_itb_mem;
    hCtx->SoftRave.dest_itb_mem = dest_itb_mem;

    hCtx->SoftRave.discard_till_next_gop = false;
    hCtx->SoftRave.discarding = false;

    hCtx->SoftRave.splice_state = SoftRave_SpliceState_Copy;
    hCtx->SoftRave.splice_start_PTS = 0;
    hCtx->SoftRave.splice_start_PTS_tolerance= 0;
    hCtx->SoftRave.splice_stop_PTS = 0;
    hCtx->SoftRave.splice_stop_PTS_tolerance = 0;
    hCtx->SoftRave.splice_pcr_offset  = 0;
    hCtx->SoftRave.splice_monitor_PTS = 0;
    hCtx->SoftRave.splice_monitor_PTS_tolerance = 0;
    hCtx->SoftRave.splice_last_pcr_offset = 0;
    hCtx->SoftRave.SpliceStartPTSFlag= false;
    hCtx->SoftRave.SpliceStopPTSFlag= false;
    hCtx->SoftRave.SpliceMonitorPTSFlag= false;
    hCtx->SoftRave.SpliceModifyPCROffsetFlag = false;
    hCtx->SoftRave.SpliceStartPTSCB = NULL;
    hCtx->SoftRave.SpliceMonitorPTSCB = NULL;
    hCtx->SoftRave.SpliceStopPTSCB = NULL;
    hCtx->SoftRave.InsertStartPts = false;
    hCtx->SoftRave.InsertStopPts = false;
    hCtx->SoftRave.StartMarkerInserted = false;
    hCtx->SoftRave.StopMarkerInserted = false;

    return BXPT_Rave_FlushContext( hCtx );
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Rave_StopPTS(
    BXPT_RaveCx_Handle hCtx,
    uint32_t StopPTS, uint32_t tolerance,
    void (* StopPTSCb)(void *, uint32_t pts),
    void * param
    )
{
    if (StopPTS == 0 && hCtx->SoftRave.SpliceStopPTSFlag  == true)
    {
    hCtx->SoftRave.splice_stop_PTS = StopPTS;
        hCtx->SoftRave.splice_stop_PTS_tolerance = 0 ;
        hCtx->SoftRave.SpliceStopPTSFlag = false;
        hCtx->SoftRave.SpliceStopPTSCB = NULL;
        hCtx->SoftRave.SpliceStopPTSCBParam =NULL;
        hCtx->SoftRave.InsertStopPts = false;
    }else
    {
        hCtx->SoftRave.splice_stop_PTS = StopPTS;
        hCtx->SoftRave.splice_stop_PTS_tolerance = tolerance ;
    hCtx->SoftRave.SpliceStopPTSFlag = true;
    hCtx->SoftRave.SpliceStopPTSCB = StopPTSCb;
    hCtx->SoftRave.SpliceStopPTSCBParam = param;
    hCtx->SoftRave.InsertStopPts = true;
    hCtx->SoftRave.StopMarkerInserted = false;
    }
    return BERR_SUCCESS;
}

BERR_Code BXPT_Rave_StartPTS(
    BXPT_RaveCx_Handle hCtx,
    uint32_t StartPTS,uint32_t tolerance,
    void (* StartPTSCb)(void *, uint32_t pts),
    void * param
    )
{
    if (StartPTS == 0 && hCtx->SoftRave.SpliceStartPTSFlag  == true)
    {
    hCtx->SoftRave.splice_start_PTS = StartPTS;
    hCtx->SoftRave.splice_start_PTS_tolerance = 0;
    hCtx->SoftRave.SpliceStartPTSFlag = false;
    hCtx->SoftRave.InsertStartPts = false;
    hCtx->SoftRave.SpliceStartPTSCB = NULL;
    hCtx->SoftRave.SpliceStartPTSCBParam = NULL;
    }else
    {
    hCtx->SoftRave.splice_start_PTS = StartPTS;
    hCtx->SoftRave.splice_start_PTS_tolerance = tolerance;
    hCtx->SoftRave.SpliceStartPTSFlag = true;
    hCtx->SoftRave.InsertStartPts = true;
    hCtx->SoftRave.StartMarkerInserted = false;
    hCtx->SoftRave.SpliceStartPTSCB = StartPTSCb;
    hCtx->SoftRave.SpliceStartPTSCBParam = param;
    }
    BDBG_MSG(("Programming Start PTS %u", StartPTS));
    return BERR_SUCCESS;
}
#endif


#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Rave_Cancel_PTS(  BXPT_RaveCx_Handle hCtx  )
{
    hCtx->SoftRave.splice_monitor_PTS = 0;
    hCtx->SoftRave.SpliceMonitorPTSFlag = false;
    hCtx->SoftRave.SpliceMonitorPTSCB = NULL;
    return BERR_SUCCESS;
}

BERR_Code BXPT_Rave_SetPCROffset(BXPT_RaveCx_Handle hCtx  , uint32_t pcr_offset)
{
    hCtx->SoftRave.splice_last_pcr_offset = pcr_offset;
    return BERR_SUCCESS;
}

BERR_Code BXPT_Rave_AdjustCdbLength(
    BXPT_RaveCx_Handle hCtx,
    size_t CdbLength
    )
{
    BMMA_DeviceOffset Base, Read, Write, Valid;
    BXPT_Rave_ContextThresholds Thresholds;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hCtx );

    Base  = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET );
    Read  = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_READ_PTR_OFFSET );
    Write = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_WRITE_PTR_OFFSET );
    Valid = BREG_ReadAddr( hCtx->hReg, hCtx->BaseAddr + CDB_VALID_PTR_OFFSET );

    /* Sanity check; no can do unless pointers are in reset state */
    if ( Base && (Base == Read) && (Base == Write) && (Base == Valid) )
    {
        BREG_WriteAddr( hCtx->hReg, hCtx->BaseAddr + CDB_END_PTR_OFFSET, Base + CdbLength - 1 );

        BXPT_Rave_GetThresholds( hCtx, &Thresholds );
        BXPT_Rave_ComputeThresholds( hCtx, CdbLength, 0, &Thresholds );
        ExitCode = BXPT_Rave_SetThresholds( hCtx, &Thresholds );
    }
    else
    {
      ExitCode = BXPT_ERR_NO_AVAILABLE_RESOURCES;
    }

    return ExitCode;
}
#endif

BERR_Code GetNextFreeScd(
    BXPT_Rave_Handle hRave,
    BXPT_RaveCx RequestedType,
    unsigned *ReturnedScdNum
    )
{
    unsigned ScdNum = 0;
    BERR_Code ExitCode = BERR_SUCCESS;

    if( RequestedType == BXPT_RaveCx_eIp )
        ScdNum = 9;  /* Only context 9 or higher can be used for IP */
    else
        ScdNum = 0;

    if( RequestedType == BXPT_RaveCx_eVctNull )
    {
        /* Need to find two consecutive free SCDs */
        for( ; ScdNum < BXPT_NUM_SCD - 2; ScdNum++ )
        {
            if( hRave->ScdTable[ ScdNum ].Allocated == false
            && hRave->ScdTable[ ScdNum + 1 ].Allocated == false )
                break;
        }

        if( ScdNum == BXPT_NUM_SCD - 1 )
        {
            BDBG_ERR(( "No SCD channel is available for this VCT context" ));
            ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        }
    }
    else
    {
        for( ; ScdNum < BXPT_NUM_SCD; ScdNum++ )
        {
            if( hRave->ScdTable[ ScdNum ].Allocated == false )
                break;
        }

        if( ScdNum == BXPT_NUM_SCD )
        {
            BDBG_ERR(( "No SCD channel is available for this AV context" ));
            ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        }
    }

    *ReturnedScdNum = ScdNum;
    return ExitCode;
}

BERR_Code GetNextRaveContext(
    BXPT_Rave_Handle hRave,
    BXPT_RaveCx RequestedType,
    unsigned *ReturnedIndex
    )
{
    unsigned Index = 0;
    BERR_Code ExitCode = BERR_SUCCESS;

    if( RequestedType == BXPT_RaveCx_eIp )
        Index = 9;  /* Only context 9 or higher can be used for IP */
    else
        Index = 0;

    if( RequestedType == BXPT_RaveCx_eVctNull )
    {
#if 0
        /* Need to find two consecutive free contexts */
        for( ; Index < BXPT_NUM_RAVE_CONTEXTS - 2; Index++ )
        {
            if( hRave->ContextTbl[ Index ].Allocated == false
            && hRave->ContextTbl[ Index + 1 ].Allocated == false )
                break;
        }

        if( Index == BXPT_NUM_RAVE_CONTEXTS - 1 )
        {
            BDBG_ERR(( "No VCT context is available" ));
            ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        }
#else
        BDBG_ERR(( "VCT context is not supported" ));
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
#endif
    }
    else
    {
        for( ; Index < BXPT_NUM_RAVE_CONTEXTS; Index++ )
        {
            if( !hRave->ContextTbl[ Index ] )
                break;
        }

        if( Index == BXPT_NUM_RAVE_CONTEXTS )
        {
            BDBG_ERR(( "No context is available" ));
            ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        }
    }

    *ReturnedIndex = Index;
    return ExitCode;
}

BERR_Code GetNextSoftRaveContext(
    BXPT_Rave_Handle hRave,
    unsigned *ReturnedIndex
    )
{
    unsigned Index = 0;
    BERR_Code ExitCode = BERR_SUCCESS;

    for( Index = 0; Index < BXPT_NUM_RAVE_CONTEXTS; Index++ )
    {
        if( !hRave->ContextTbl[ Index ] )
            break;
    }

    if( Index == BXPT_NUM_RAVE_CONTEXTS )
    {
        BDBG_ERR(( "No context is available" ));
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
    }

    *ReturnedIndex = Index;
    return ExitCode;
}

BERR_Code MapGranularityToHw(
    unsigned GranularityInBytes,
    uint32_t *Wmark
    )
{
    switch( GranularityInBytes )
    {
        case 32: *Wmark = 0; break;
        case 64: *Wmark = 1; break;
        case 128: *Wmark = 2; break;
        case 256: *Wmark = 3; break;
        case 512: *Wmark = 4; break;
        case 1024: *Wmark = 5; break;
        case 2048: *Wmark = 6; break;
        case 4096: *Wmark = 7; break;
        default: *Wmark = 0; return BERR_INVALID_PARAMETER;
    }
    return BERR_SUCCESS;
}
uint32_t GetGranularityInBytes(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t Reg, Watermark;
    Reg = BREG_Read32( hCtx->hReg, BCHP_XPT_RAVE_MISC_CONTROL );
    Watermark = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_MISC_CONTROL, WMARK_GRANULARITY );
    return 32 << Watermark;
}

size_t GetMaxBufferByGranularity(
    BXPT_Rave_Handle hRave
    )
{
    uint32_t Reg = BREG_Read32( hRave->hReg, BCHP_XPT_RAVE_MISC_CONTROL );
    switch( BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_MISC_CONTROL, WMARK_GRANULARITY ) )
    {
        case 0: return 2 * 1024 * 1024;
        case 1: return 4 * 1024 * 1024;
        case 2: return 8 * 1024 * 1024;
        case 3: return 16 * 1024 * 1024;
        case 4: return 32 * 1024 * 1024;
        case 5: return 64 * 1024 * 1024;
        case 6: return 128 * 1024 * 1024;
        case 7: return 256 * 1024 * 1024;
        default: return 0;
    }
}


void BXPT_Rave_GetStatus(
    BXPT_Rave_Handle hRave,
    BXPT_Rave_Status *Status
    )
{
    unsigned Index;

    BDBG_ASSERT( hRave );
    BDBG_ASSERT( Status );

    BKNI_Memset( Status, 0, sizeof(*Status) );
    Status->SupportedContexts = BXPT_NUM_RAVE_CONTEXTS;
    for( Index = 0; Index < BXPT_NUM_RAVE_CONTEXTS; Index++ )
    {
        if( !hRave->ContextTbl[ Index ] )
            Status->AllocatedContexts++;
    }
}


void BXPT_Rave_GetFwRevisionInfo(
    BXPT_Rave_Handle hRave,                 /* [in] Handle for this RAVE channel */
    BXPT_Rave_RevisionInfo *versionInfo     /* [out] Version info */
    )
{
    BDBG_ASSERT( hRave );
    BSTD_UNUSED( hRave );
    BDBG_ASSERT( versionInfo );

    versionInfo->fwRev = BxptRaveInitData[ BxptRaveInitDataSize - 1 ];
    versionInfo->fwCrc = BxptRaveInitData[ BxptRaveInitDataSize - 2 ];
}
