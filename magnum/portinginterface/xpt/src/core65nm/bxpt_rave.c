/******************************************************************************
 * (c) 2003-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *
 *****************************************************************************/


#include "bstd.h"
#include "bxpt_priv.h"
#include "bxpt.h"
#include "bkni.h"
#if (BCHP_CHIP != 7002)
#include "bmem.h"
#endif
#include "blst_slist.h"
#include "bxpt_rave.h"
#include "bxpt_rave_ihex.h"
#include "bxpt_pcr_offset.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if BXPT_HAS_XCBUF
#include "bxpt_xcbuf.h"
#endif

#if BXPT_HAS_RSBUF
#include "bxpt_rsbuf.h"
#endif

#include "bchp_xpt_rave.h"
#if (BCHP_CHIP == 7118) || (BCHP_CHIP == 7401)
#else
    #include "bchp_xpt_xpu.h"
#endif
#include "bchp_xpt_fe.h"
#include "bchp_sun_top_ctrl.h"

#if (BCHP_CHIP == 35130) || (BCHP_CHIP == 35230) || (BCHP_CHIP == 35125) || (BCHP_CHIP == 35233)
#include "bchp_tvm.h"
#endif

#if (BCHP_CHIP == 7440 && BCHP_VER >= BCHP_VER_B0 )
#define XPT_RAVE_CX0_REC_MISC_CONFIG XPT_RAVE_CX0_MISC_CONFIG
#define XPT_RAVE_CX0_REC_SCD_PIDS_AB XPT_RAVE_CX0_SCD_PIDS_AB
#define XPT_RAVE_CX0_REC_SCD_PIDS_CD XPT_RAVE_CX0_SCD_PIDS_CD
#define XPT_RAVE_CX0_REC_SCD_PIDS_EF XPT_RAVE_CX0_SCD_PIDS_EF
#define XPT_RAVE_CX0_REC_SCD_PIDS_GH XPT_RAVE_CX0_SCD_PIDS_GH
#endif

BDBG_MODULE( xpt_rave );

/* Max number of contexts mapped in XPT_RAVE_CXMEM_LO */
#define CXMEM_LO_MAX_CONTEXT 16
#define CXMEM_HI_MAX_CONTEXT 24

/* Number of bytes to skip when indexing from context X to X+1 */
#define RAVE_CONTEXT_REG_STEP       ( BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#if (BCHP_CHIP == 7440 && BCHP_VER >= BCHP_VER_B0 )
#define REC_MISC_CFG_OFFSET         ( BCHP_XPT_RAVE_CX0_MISC_CONFIG -  BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#else
#define REC_MISC_CFG_OFFSET         ( BCHP_XPT_RAVE_CX0_REC_MISC_CONFIG -  BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#endif

#define REC_MISC_CFG2_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG2 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_MISC_CFG3_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG3 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#if (BCHP_CHIP == 7440 )
#define REC_MISC_CFG4_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG4 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#endif
#define REC_MISC_CFG5_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG5 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#if (BCHP_CHIP == 7440 && BCHP_VER >= BCHP_VER_B0 )
#define REC_SCD_PIDS_AB_OFFSET      ( BCHP_XPT_RAVE_CX0_SCD_PIDS_AB - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_CD_OFFSET      ( BCHP_XPT_RAVE_CX0_SCD_PIDS_CD - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_EF_OFFSET      ( BCHP_XPT_RAVE_CX0_SCD_PIDS_EF - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_GH_OFFSET      ( BCHP_XPT_RAVE_CX0_SCD_PIDS_GH - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#else
#define REC_SCD_PIDS_AB_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_AB - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_CD_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_CD - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_EF_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_EF - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_SCD_PIDS_GH_OFFSET      ( BCHP_XPT_RAVE_CX0_REC_SCD_PIDS_GH - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#endif

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
#if ( BCHP_CHIP == 7325 && BCHP_VER >= BCHP_VER_B0 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || ( BCHP_CHIP == 7405 && BCHP_VER >= BCHP_VER_B0 ) || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || ( BCHP_CHIP == 7002 ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 )
#define SCD_STATE_END_OFFSET        ( BCHP_XPT_RAVE_SCD0_RESERVED_STATE3 - BCHP_XPT_RAVE_SCD0_SCD_MISC_CONFIG )
#else
#define SCD_STATE_END_OFFSET        ( BCHP_XPT_RAVE_SCD0_SCD_STATE5 - BCHP_XPT_RAVE_SCD0_SCD_MISC_CONFIG )
#endif

#define AV_MISC_CFG1_OFFSET         ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG1 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_MISC_CFG2_OFFSET         ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG2 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_MISC_CFG3_OFFSET         ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG3 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_MISC_CFG4_OFFSET         ( BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG4 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define REC_CTRL1_OFFSET            ( BCHP_XPT_RAVE_CX0_REC_CTRL1 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define REC_INIT_TS_OFFSET          ( BCHP_XPT_RAVE_CX0_REC_INIT_TS - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_VID_SC_RANGE_AB_OFFSET   ( BCHP_XPT_RAVE_CX0_AV_VID_SC_RANGE_AB - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define AV_VID_SC_RANGE_CD_OFFSET   ( BCHP_XPT_RAVE_CX0_AV_VID_SC_RANGE_CD - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

/* Haven't done these yet */

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
#define AV_COMP12_FILTER_MODE_OFFSET ( BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#if BXPT_NEXT_GEN_COMPARATORS
    #ifdef BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode                             BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_FUNC_D_MASK      BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_FUNC_D_MASK
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_FUNC_D_SHIFT     BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_FUNC_D_SHIFT
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_FUNC_C_MASK      BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_FUNC_C_MASK
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_FUNC_C_SHIFT     BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_FUNC_C_SHIFT
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_FUNC_B_MASK      BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_FUNC_B_MASK
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_FUNC_B_SHIFT     BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_FUNC_B_SHIFT
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_FUNC_A_MASK      BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_FUNC_A_MASK
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_FUNC_A_SHIFT     BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_FUNC_A_SHIFT

        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_OFFSET_D_MASK    BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_OFFSET_D_MASK
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_OFFSET_D_SHIFT   BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_OFFSET_D_SHIFT
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_OFFSET_C_MASK    BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_OFFSET_C_MASK
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_OFFSET_C_SHIFT   BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_OFFSET_C_SHIFT
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_OFFSET_B_MASK    BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_OFFSET_B_MASK
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_OFFSET_B_SHIFT   BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_OFFSET_B_SHIFT
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_OFFSET_A_MASK    BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_OFFSET_A_MASK
        #define BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_Mode_COMP1_FILT_OFFSET_A_SHIFT   BCHP_XPT_RAVE_CX0_AV_COMP12_FILTER_MODE_COMP1_FILT_OFFSET_A_SHIFT
    #endif
#endif

#if BXPT_HAS_RAVE_SCRAMBLING_CONTROL
#define SCRAMBLE_CTRL_OFFSET        ( BCHP_XPT_RAVE_CX0_SC - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#endif

/* Some chips don't have these defined in their RDB yet */
#ifndef BCHP_XPT_RAVE_CX0_SC_SC_OR_MODE
    #define BCHP_XPT_RAVE_CX0_SC_SC_OR_MODE_MASK          0x00000008
    #define BCHP_XPT_RAVE_CX0_SC_SC_OR_MODE_SHIFT         3
#endif

#if (BCHP_CHIP == 7401)
    #define BCHP_XPT_XPU_IMEMi_ARRAY_BASE   0x00220800
#elif !defined( BCHP_XPT_XPU_IMEMi_ARRAY_BASE )
    #define BCHP_XPT_XPU_IMEMi_ARRAY_BASE   (BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR + 0x10000)
#endif

#if BXPT_HAS_RAVE_AUTO_READ
    /* These definitions aren't in the RDB yet. */
    #define BCHP_XPT_RAVE_CX0_REC_MISC_CONFIG_REC_AUTO_UPDATE_RD_PTR_EN_MASK          0x00000004
    #define BCHP_XPT_RAVE_CX0_REC_MISC_CONFIG_REC_AUTO_UPDATE_RD_PTR_EN_SHIFT         2
#endif

/* BCHP_XPT_RAVE_CX0_RAVE_Reg_0 was renamed to XPT_RAVE_CX0_AV_MISC_CONFIG5 on later chips. */
#ifndef BCHP_XPT_RAVE_CX0_RAVE_Reg_0
    #define BCHP_XPT_RAVE_CX0_RAVE_Reg_0 BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG5
#endif

#define RAVE_CFG_ARRAY_BASE         ( BCHP_XPT_XPU_IMEMi_ARRAY_BASE )
#define RAVE_CFG_MISC_REG           ( BCHP_XPT_XPU_IMEMi_ARRAY_BASE - 0x800 + 4 )

#define CTX_IP_OVERFLOW_THRESH      ( 0x100 )

/* For right now, the first 6 contexts are dedicated AV. The rest are dedicated records. */
#define BXPT_P_MAX_AV_CONTEXT           ( 6 )

/* RAVE only supports buffers aligned on 4-byte boundaries. This alignment is expressed as a power of 2. */
#define RAVE_BUFFER_ALIGNMENT           ( 2 )

/* Base address in data memory and size to be used for context-specific parameters */
/* SCD_DMEM_BASE was 0x540 */
#define SCD_DMEM_BASE  0x360
#define SCD_DMEM_SIZE  0x20

/* An offset into the CTX_DMEM area, where the pointer to the splicing stack is stored. */
#define SPLICE_QUEUE_PTR_OFFSET     ( 0x17 )

/* Offset into CTX_DMEM area, where the actual splicing stacks are stored. Also the size of the stacks */
#define SPLICE_QUEUE_AREA_OFFSET    ( 0x7C0 * 4 )
#define SPLICE_QUEUE_AREA_SIZE      ( 0x10 * 4 )

#define SPLICE_RD_PTR_OFFSET        ( 0 )
#define SPLICE_WR_PTR_OFFSET        ( 1 * 4 )
#define SPLICE_CURR_PID_OFFSET      ( 2 * 4 )
#define SPLICE_QUEUE_LOC_0_OFFSET   ( 3 * 4 )

#define SPLICE_MAX_SLOT         ( 8 )
#define SPLICE_QUEUE_SIZE       ( 8 )
#define MAX_THRESHOLD_GRANULARITY           (4096)
#define MIN_THRESHOLD_GRANULARITY           (32)
#define DEFAULT_THRESHOLD_GRANULARITY       (256)

/* TBD: We don't have RDB defines for this bitfield. These are placeholders until we do. */
#define BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG3_ENABLE_PCR_RTS_ENTRIES_MASK  ( 1 << 15 )
#define BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG3_ENABLE_PCR_RTS_ENTRIES_SHIFT 15

#define BXPT_ITB_SIZE 16

static BERR_Code RaveRamInit( BXPT_Rave_Handle hRave );
static BERR_Code ClearMem( BXPT_Rave_Handle hRave );
static BERR_Code InitScd( StartCodeIndexer *lhScd );
static BERR_Code InitTpit( TpitIndexer *lhTpit );
static BERR_Code InitContext( BXPT_RaveCx_Handle ThisCtx, const BAVC_CdbItbConfig *BufferCfg );
static BERR_Code LoadScRanges( BXPT_RaveCx_Handle Context, const BXPT_Rave_EsRanges *EsRanges );
static BERR_Code GetScRanges( BXPT_RaveCx_Handle Context, BXPT_Rave_EsRanges *EsRanges );
static BERR_Code ConfigureComparators( BXPT_RaveCx_Handle hCtx, BAVC_ItbEsType StreamType );
static BERR_Code AllocateSpliceQueue( BXPT_RaveCx_Handle hCtx, unsigned *QueueNum );
static BERR_Code ResetContextPointers( BXPT_RaveCx_Handle hCtx );
static BERR_Code ChangeScdPid( BXPT_RaveCx_Handle hCtx, uint32_t RegAddr, unsigned WhichScdNum, unsigned WhichScdBlock, unsigned Pid, bool PidValid );
static BERR_Code FlushScds( BXPT_RaveCx_Handle hCtx );
static BERR_Code ClearScdStates( BXPT_RaveCx_Handle hCtx, uint32_t ScdNum );
static BERR_Code FlushTpit( BXPT_RaveCx_Handle hCtx, uint32_t TpitNum );
static int GetNextPicCounter( BXPT_RaveCx_Handle hCtx );
static void FreePicCounter( BXPT_RaveCx_Handle Context );
static void FlushPicCounter( BXPT_RaveCx_Handle hCtx );
static uint32_t GetPictureCounterReg( BXPT_RaveCx_Handle hCtx );
static void SetPictureCounterMode( BXPT_RaveCx_Handle hCtx, BAVC_ItbEsType ItbFormat );
static void FreeScds( unsigned NumScds, StartCodeIndexer **ScdPtr );
static BERR_Code SetExtractionBitCounts( BXPT_RaveCx_Handle hCtx, StartCodeIndexer *hScd, const BXPT_Rave_ScdEntry *ScdConfig, unsigned WhichScd );
static void GetScEnables( BXPT_Rave_EsRanges *Range, unsigned Mode );
static void GetScEnables_Indexer( IndexerScRange *Range, unsigned Mode );
static BERR_Code GetNextFreeScd( BXPT_Rave_Handle hRave, BXPT_RaveCx RequestedType, unsigned *ReturnedScdNum );
static BERR_Code GetNextRaveContext( BXPT_Rave_Handle hRave, BXPT_RaveCx RequestedType, unsigned *ReturnedIndex );
static BERR_Code GetNextSoftRaveContext( BXPT_Rave_Handle hRave, unsigned *ReturnedIndex );
static BERR_Code AllocContext_Priv( BXPT_Rave_Handle hRave, BXPT_RaveCx RequestedType, const BAVC_CdbItbConfig *BufferCfg, unsigned Index, BXPT_RaveCx_Handle *Context );
static BERR_Code AllocSoftContext_Priv( BXPT_RaveCx_Handle SrcContext, BXPT_RaveSoftMode DestContextMode, unsigned index, BXPT_RaveCx_Handle *DestContext );
static BERR_Code SetExtractionBitCounts_ByPid( BXPT_RaveCx_Handle hCtx, StartCodeIndexer *hScd, const BXPT_Rave_ScdEntry *ScdConfig, unsigned WhichScd );

static uint32_t GetGranularityInBytes( BXPT_RaveCx_Handle hCtx );
static BERR_Code MapGranularityToHw( unsigned GranularityInBytes, uint32_t *Wmark );
static size_t GetMaxBufferByGranularity( BXPT_Rave_Handle hRave );

BERR_Code BXPT_Rave_GetTotalChannels(
    BXPT_Handle hXpt,           /* [in] Handle for this transport instance. */
    unsigned *TotalChannels     /* [out] The number of RAVE channels supported */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BSTD_UNUSED( hXpt );
    BDBG_ASSERT( TotalChannels );

    *TotalChannels = BXPT_P_MAX_RAVE_CHANNELS;

    return( ExitCode );
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

#if ( BCHP_CHIP == 7403 )
    RaveDefSettings->TimeTick = 0;
#endif

    RaveDefSettings->chanOpenCB = NULL;
    RaveDefSettings->ThresholdGranularityInBytes = DEFAULT_THRESHOLD_GRANULARITY;

    return( ExitCode );
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
    BXPT_RaveCx_Handle ThisCtx = NULL;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BDBG_ASSERT( hRave );
    BDBG_ASSERT( RaveDefSettings );

    if( ChannelNo >= BXPT_P_MAX_RAVE_CHANNELS )
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

    lhRave->hChip = hXpt->hChip;
    lhRave->hReg = hXpt->hRegister;
    lhRave->hMem = hXpt->hMemory;
    lhRave->hInt = hXpt->hInt;
    lhRave->ChannelNo = ChannelNo;
    lhRave->lvXpt = hXpt;
    lhRave->chanOpenCB = RaveDefSettings->chanOpenCB;

    BDBG_OBJECT_SET(lhRave, bxpt_t);

    for( Index = 0; Index < BXPT_P_MAX_PIC_COUNTER; Index++ )
        lhRave->PicCounterUsed[ Index ] = false;

    /*
    ** Init each of the contexts. This section changes when/if they support
    ** dynamic changing of context type (AV versus record).
    */
    for( Index = 0; Index < BXPT_P_MAX_RAVE_CONTEXTS; Index++ )
    {
        ThisCtx = lhRave->ContextTbl + Index;

        ThisCtx->hChp = lhRave->hChip;
        ThisCtx->hReg = lhRave->hReg;
        ThisCtx->hMem = lhRave->hMem;
        ThisCtx->hInt = lhRave->hInt;
        ThisCtx->hRHeap = hXpt->hRHeap;
        ThisCtx->Allocated = false;
        ThisCtx->Index = Index;     /* Absolute context index, not type dependent. */
        ThisCtx->vhRave = ( void * ) lhRave;
        ThisCtx->PicCounter = -1;    /* Invalid picture counter. */
        ThisCtx->IsSoftContext = false;

        ThisCtx->BaseAddr = BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR + ( Index * RAVE_CONTEXT_REG_STEP );
        InitContext( ThisCtx, NULL );

        BDBG_OBJECT_SET(ThisCtx, bxpt_t);
    }

    for( Index = 0; Index < BXPT_P_NUM_SPLICING_QUEUES; Index++ )
    {
        lhRave->SpliceQueueAllocated[ Index ] = false;
    }

    for( Index = 0; Index < BXPT_P_MAX_SCD; Index++ )
    {
        StartCodeIndexer *lhScd = lhRave->ScdTable + Index;

        lhScd->hReg = lhRave->hReg;
        lhScd->hMem = lhRave->hMem;
        lhScd->hInt = lhRave->hInt;
        lhScd->ChannelNo = Index;
        lhScd->BaseAddr = BCHP_XPT_RAVE_SCD0_SCD_MISC_CONFIG + ( Index * SCD_REG_STEP );
        lhScd->Allocated = false;
        InitScd( lhScd );
    }

    for( Index = 0; Index < BXPT_P_MAX_TPIT; Index++ )
    {
        TpitIndexer *lhTpit = lhRave->TpitTable + Index;

        lhTpit->hReg = lhRave->hReg;
        lhTpit->hMem = lhRave->hMem;
        lhTpit->hInt = lhRave->hInt;
        lhTpit->ChannelNo = Index;
        lhTpit->BaseAddr = BCHP_XPT_RAVE_TPIT0_CTRL1 + ( Index * TPIT_CTRL_REG_STEP );
        lhTpit->PidTableBaseAddr = BCHP_XPT_RAVE_TPIT0_PID_TABLEi_ARRAY_BASE + ( Index * TPIT_PID_TABLE_STEP );
        lhTpit->ParseTableBaseAddr = BCHP_XPT_RAVE_TPIT0_PAR_TABLEi_ARRAY_BASE + ( Index * TPIT_PARSE_TABLE_STEP );
        lhTpit->Allocated = false;
        InitTpit( lhTpit );
    }

    for( Index = 0; Index < BXPT_P_MAX_SCD + BXPT_P_MAX_TPIT; Index++ )
    {
        BXPT_P_IndexerHandle *lhIdx = lhRave->IndexerHandles + Index;

        lhIdx->ChannelType = BXPT_RaveIdx_eScd;
        lhIdx->hChannel = NULL;
        lhIdx->Allocated = false;
    }

    /* Init memory spaces. */
    ClearMem( lhRave );
    RaveRamInit( lhRave );

    /* Load the defaults. */

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
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, NUM_CONTEXTS, BXPT_P_MAX_RAVE_CONTEXTS ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, WMARK_GRANULARITY, WaterMarkGranularity ) |
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL, PACKET_CNT_CLR, 0 )
    );
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_MISC_CONTROL, Reg );

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

#if ( BCHP_CHIP == 7403 )
    Reg = BREG_Read32( lhRave->hReg, BCHP_XPT_RAVE_TPIT_TIME_TICK );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_TPIT_TIME_TICK, TPIT_TIME_TICK )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_TPIT_TIME_TICK, TPIT_TIME_TICK, RaveDefSettings->TimeTick )
    );
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_TPIT_TIME_TICK, Reg );
#endif

#if ( BCHP_CHIP == 7408 )
    Reg = BREG_Read32( lhRave->hReg, BCHP_XPT_RAVE_MISC_CONTROL3 );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_MISC_CONTROL3, DIS_PTS_EXT_BTP_TRIMMED )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_MISC_CONTROL3, DIS_PTS_EXT_BTP_TRIMMED, 1 )
    );
    BREG_Write32( lhRave->hReg, BCHP_XPT_RAVE_MISC_CONTROL3, Reg );
#endif

#ifdef UNIFIED_ITB_SUPPORT
    BDBG_MSG(( "Using unified ITB format" ));
#else
    BDBG_MSG(( "Using legacy ITB format" ));
#endif

    hXpt->RaveChannels[ ChannelNo ].Handle = *hRave = lhRave; /* lhRave is initialized to NULL at the start. */
    return ExitCode;

    Error2:
    BKNI_Free( lhRave );

    Error1:
    *hRave = NULL;
    return ExitCode;
}

BERR_Code BXPT_Rave_CloseChannel(
    BXPT_Rave_Handle hRave     /* [in] Handle for this RAVE instance */
    )
{
    BXPT_Handle hXpt;
    unsigned Index;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRave, bxpt_t);

    hXpt = ( BXPT_Handle ) hRave->lvXpt;
    hXpt->RaveChannels[ hRave->ChannelNo ].Allocated = false;

    /* Stop the XPU? */

    /* Free any context buffers left hanging around */
    for( Index = 0; Index < BXPT_P_MAX_RAVE_CONTEXTS; Index++ ) {
        BXPT_RaveCx_Handle ThisCtx = hRave->ContextTbl + Index;
        if( ThisCtx->Allocated == true )
            BXPT_Rave_FreeContext( ThisCtx);
        BDBG_OBJECT_UNSET(ThisCtx, bxpt_t);
    }

    BDBG_OBJECT_UNSET(hRave, bxpt_t);
    BKNI_Free( hRave );

    return( ExitCode );
}

void BXPT_Rave_GetDefaultAllocCxSettings(
    BXPT_Rave_AllocCxSettings *pSettings /* [out] default settings */
    )
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(BXPT_Rave_AllocCxSettings));
    pSettings->RequestedType = BXPT_RaveCx_eAv;
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

BERR_Code BXPT_Rave_AllocCx(
    BXPT_Rave_Handle hRave,         /* [in] Handle for this RAVE channel */
    const BXPT_Rave_AllocCxSettings *pSettings, /* [in] settings for this RAVE channel allocation */
    BXPT_RaveCx_Handle *Context     /* [out] The allocated context */
    )
{
    unsigned Index;

    BXPT_RaveCx_Handle ThisCtx = NULL;

    BDBG_ASSERT( hRave );
    BDBG_ASSERT( Context );

    if( pSettings->SoftRaveSrcCx )
    {
        if( GetNextSoftRaveContext( hRave, &Index ) != BERR_SUCCESS )
        {
            BDBG_ERR(( "No free contexts!" ));
            return BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        }

        /* Set the local heap handle to the one given at RAVE channel open, as the local handle
        may have been hijacked by BXPT_Rave_AllocSoftContextFromHeap() */
        ThisCtx = hRave->ContextTbl + Index;
        ThisCtx->hMem = hRave->hMem;

        /* this triggers AllocSoftContext_Priv() not to allocate the ITB */
        ThisCtx->ItbBufferAddr = pSettings->ItbBlock ? BMMA_Lock(pSettings->ItbBlock) : NULL;

        return AllocSoftContext_Priv( pSettings->SoftRaveSrcCx, pSettings->SoftRaveMode, Index, Context );
    }
    else {
        if( GetNextRaveContext( hRave, pSettings->RequestedType, &Index ) != BERR_SUCCESS )
        {
            BDBG_ERR(( "No free contexts!" ));
            return BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        }

        /* Set the local heap handle to the one given at RAVE channel open, as the local handle
        may have been hijacked by BXPT_Rave_AllocContextFromHeap() */
        ThisCtx = hRave->ContextTbl + Index;
        ThisCtx->hMem = hRave->hMem;

        /* this triggers InitContext() not to allocate the CDB/ITB */
        ThisCtx->CdbBufferAddr = pSettings->CdbBlock ? BMMA_Lock(pSettings->CdbBlock) : NULL;
        ThisCtx->ItbBufferAddr = pSettings->ItbBlock ? BMMA_Lock(pSettings->ItbBlock) : NULL;

        return AllocContext_Priv( hRave, pSettings->RequestedType, &(pSettings->BufferCfg), Index, Context );
    }
}

BERR_Code BXPT_Rave_AllocContextFromHeap(
    BXPT_Rave_Handle hRave,         /* [in] Handle for this RAVE channel */
    BXPT_RaveCx RequestedType,      /* [in] The type of context to allcoate */
    const BAVC_CdbItbConfig *BufferCfg,   /* [in] Size and alignment for ITB and CDB */
    BMEM_Handle ContextHeap,         /* [in] This context's memory heap handle */
    BXPT_RaveCx_Handle *Context     /* [out] The allocated context */
    )
{
    unsigned Index;

    BXPT_RaveCx_Handle ThisCtx = NULL;

    BDBG_OBJECT_ASSERT(hRave, bxpt_t);
    BDBG_ASSERT( BufferCfg );
    BDBG_ASSERT( Context );
    BDBG_ASSERT( ContextHeap );

    if( GetNextRaveContext( hRave, RequestedType, &Index ) != BERR_SUCCESS )
    {
        BDBG_ERR(( "No free contexts!" ));
        return BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
    }

    /* Same as the normal context alloc, except the we substitute the caller's heap
    for the general handle that was installed when the RAVE channel was openned. */
    ThisCtx = hRave->ContextTbl + Index;
    ThisCtx->hMem = ContextHeap;

    /* InitContext() will do the alloc */
    ThisCtx->CdbBufferAddr = NULL;
    ThisCtx->ItbBufferAddr = NULL;

    return AllocContext_Priv( hRave, RequestedType, BufferCfg, Index, Context );
}

BERR_Code BXPT_Rave_AllocContext(
    BXPT_Rave_Handle hRave,             /* [in] Handle for this RAVE channel */
    BXPT_RaveCx RequestedType,          /* [in] The type of context to allcoate */
    const BAVC_CdbItbConfig *BufferCfg,       /* [in] Size and alignment for ITB and CDB */
    BXPT_RaveCx_Handle *Context     /* [out] The allocated context */
    )
{
    unsigned Index;

    BXPT_RaveCx_Handle ThisCtx = NULL;

    BDBG_OBJECT_ASSERT(hRave, bxpt_t);
    BDBG_ASSERT( BufferCfg );
    BDBG_ASSERT( Context );

    if( GetNextRaveContext( hRave, RequestedType, &Index ) != BERR_SUCCESS )
    {
        BDBG_ERR(( "No free contexts!" ));
        return BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
    }

    /* Set the local heap handle to the one given at RAVE channel open, as the local handle
    may have been hijacked by BXPT_Rave_AllocContextFromHeap() */
    ThisCtx = hRave->ContextTbl + Index;
    ThisCtx->hMem = hRave->hMem;

    /* InitContext() will do the alloc */
    ThisCtx->CdbBufferAddr = NULL;
    ThisCtx->ItbBufferAddr = NULL;

    return AllocContext_Priv( hRave, RequestedType, BufferCfg, Index, Context );
}

static BERR_Code AllocContext_Priv(
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

    /* Check for invalid buffer alignment. */
    if( BufferCfg->Cdb.Alignment < RAVE_BUFFER_ALIGNMENT )
    {
        BDBG_ERR(( "Invalid CDB alignment requested!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto done;
    }
    if( BufferCfg->Cdb.Length > GetMaxBufferByGranularity( hRave ) )
    {
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

    ThisCtx = hRave->ContextTbl + Index;
    ThisCtx->Allocated = true;
    ThisCtx->Type = RequestedType;

    if( RequestedType == BXPT_RaveCx_eVctNull )
    {
        BXPT_RaveCx_Handle VctNeighbor = &hRave->ContextTbl[ Index + 1 ];

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

    if( InitContext( ThisCtx, BufferCfg ) != BERR_SUCCESS )
    {
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        goto done;
    }
    ThisCtx->hAvScd = NULL;
    ThisCtx->hVctScd = NULL;

    /* By default, all SCD mapping should be done by PID channels */
    Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + REC_MISC_CFG_OFFSET );
#if BXPT_7440B0_SCD_MAPPING
    Reg &= ~BCHP_MASK( XPT_RAVE_CX0_MISC_CONFIG, SCD_MAP_MODE );
#else
    Reg &= ~BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE );
    if( RequestedType == BXPT_RaveCx_eIp )
        ScdMapMode = 2;
    Reg |= BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE, ScdMapMode );
#endif
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
        Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET );
        Reg &= ~(
#if BXPT_7440B0_SCD_MAPPING
           BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_NUMA )
#elif BXPT_REC_SCD_MAPPING
           BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA )
#else
           BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMA )
#endif
        );

        Reg |= (
#if BXPT_REC_SCD_MAPPING
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA, ScdNum )
#else
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMA, ScdNum )
#endif
        );
        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET, Reg );
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

#if ( BCHP_CHIP == 7401 ) || ( BCHP_CHIP == 7118 )
    /* Default to non-VOB data for 7401. Other chips don't use this flag */
    ThisCtx->VobMode = false;
#endif

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

    BDBG_OBJECT_ASSERT(Context, bxpt_t);

#ifdef BCHP_PWR_RESOURCE_XPT_RAVE
    Reg = BREG_Read32(Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET);
    wasEnabled = BCHP_GET_FIELD_DATA(Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE);

    /* only increment refcnt if not enabled already */
    if (!wasEnabled) {
        BCHP_PWR_AcquireResource(Context->hChp, BCHP_PWR_RESOURCE_XPT_RAVE);
    }
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

    BDBG_OBJECT_ASSERT(Context, bxpt_t);

#ifdef BCHP_PWR_RESOURCE_XPT_RAVE
    Reg = BREG_Read32(Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET);
    wasEnabled = BCHP_GET_FIELD_DATA(Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE);
#endif

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET, Reg );

    /* Wait for 15uS for any pending DMA transfer to complete */
    BKNI_Delay( 15 );

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
    uint32_t ItbBase;

    unsigned ChannelNum;
    unsigned Index;

    BXPT_RaveIdx_Handle lhIdx = NULL;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRave, bxpt_t);
    BDBG_OBJECT_ASSERT(Context, bxpt_t);
    BDBG_ASSERT( Indexer );

    /* Make sure this context has an ITB allocated. In some cases, it won't */
    ItbBase = BREG_Read32( Context->hReg, Context->BaseAddr + ITB_BASE_PTR_OFFSET );
    if( !ItbBase )
    {
        BDBG_ERR(( "No ITB allocated for this context." ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    for( Index = 0; Index < BXPT_P_MAX_SCD + BXPT_P_MAX_TPIT; Index++ )
        if( hRave->IndexerHandles[ Index ].Allocated == false )
            break;

    if( Index == BXPT_P_MAX_SCD + BXPT_P_MAX_TPIT )
    {
        BDBG_ERR(( "No Indexer handles are available for allocation." ));
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        goto Done;
    }

    lhIdx = hRave->IndexerHandles + Index;
    lhIdx->NumChannels = 0;
    lhIdx->Allocated = true;

    BDBG_OBJECT_SET(lhIdx, bxpt_t);

    if( IndexerType == BXPT_RaveIdx_eTpit )
    {
        TpitIndexer *lhTpit;

        for( ChannelNum = 0; ChannelNum < BXPT_P_MAX_TPIT; ChannelNum++ )
            if( hRave->TpitTable[ ChannelNum ].Allocated == false )
                break;

        if( ChannelNum == BXPT_P_MAX_TPIT )
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
#if (BCHP_CHIP == 7440 && BCHP_VER >= BCHP_VER_B0 )
            BCHP_MASK( XPT_RAVE_CX0_MISC_CONFIG, TPIT_ENABLE ) |
            BCHP_MASK( XPT_RAVE_CX0_MISC_CONFIG, TPIT_CHANNEL )
#else
            BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_ENABLE ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_CHANNEL )
#endif
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
    }

    Done:
    *Indexer = lhIdx;
    return( ExitCode );
}

BERR_Code BXPT_Rave_FreeContext(
    BXPT_RaveCx_Handle Context      /* [in] The context to free. */
    )
{
    uint32_t BufferOffset;
    void *BufferAddr;
    BMEM_Handle hMem;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Context, bxpt_t);

    if( Context->IsSoftContext )
    {
        /* BXPT_RaveSoftMode_ePointersOnly does not allocate any buffers. All other modes do. */
        if( Context->SoftRave.mode != BXPT_RaveSoftMode_ePointersOnly )
        {
            BufferOffset = BREG_Read32( Context->hReg, Context->BaseAddr + ITB_BASE_PTR_OFFSET );
            BMEM_ConvertOffsetToAddress( Context->hMem, BufferOffset, &BufferAddr );
            if( !Context->externalItbAlloc ) {
                BMEM_Free( Context->hMem, BufferAddr );
            }
            Context->ItbBufferAddr = 0;
        }
    }
    else
    {
        /* Make sure it's off before we release the buffers */
        BXPT_Rave_DisableContext( Context );

        /* The CDB may have been allocated from the secure heap. */
        if( Context->Type == BXPT_RaveCx_eAvR && Context->hRHeap )
            hMem = Context->hRHeap;
        else
            hMem = Context->hMem;

        /* Free the CDB */
        BufferOffset = BREG_Read32( Context->hReg, Context->BaseAddr + CDB_BASE_PTR_OFFSET );
        BMEM_ConvertOffsetToAddress( hMem, BufferOffset, &BufferAddr );
        BMEM_ConvertAddressToCached( hMem, BufferAddr, &BufferAddr );
        if(Context->CdbBufferAddr != BufferAddr)
        {
            BDBG_WRN(("Allocated cdb addr %x,actual cdb addr %x",Context->CdbBufferAddr,BufferAddr));
        }
        if( !Context->externalCdbAlloc )
        {
            BMEM_Free( hMem, Context->CdbBufferAddr );
        }
        Context->CdbBufferAddr=0;

        /* Free the ITB, if there was one. The ITB is always allocated from the general heap. */
        BufferOffset = BREG_Read32( Context->hReg, Context->BaseAddr + ITB_BASE_PTR_OFFSET );
        if( BufferOffset )
        {
            BMEM_ConvertOffsetToAddress( Context->hMem, BufferOffset, &BufferAddr );
            BMEM_ConvertAddressToCached( hMem, BufferAddr, &BufferAddr );
            if(Context->ItbBufferAddr != BufferAddr)
            {
                BDBG_WRN(("Allocated itb addr %x,actual itb addr %x",Context->ItbBufferAddr,BufferAddr));
            }
            if( !Context->externalItbAlloc )
            {
                BMEM_Free( Context->hMem, Context->ItbBufferAddr );
            }
            Context->ItbBufferAddr =0;
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
    Context->Type = BXPT_RaveCx_eAv;    /* Default to AV */
    InitContext( Context, NULL );

    Context->allocatedCdbBufferSize = 0;
    Context->usedCdbBufferSize = 0;
    Context->Allocated = false;
    Context->IsSoftContext = false;

    return( ExitCode );
}

BERR_Code BXPT_Rave_FreeIndexer(
    BXPT_RaveIdx_Handle Indexer     /* [in] The indexer to free. */
    )
{
    uint32_t Reg;
    BXPT_RaveCx_Handle hCtx;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Indexer, bxpt_t);
    hCtx = Indexer->vhCtx;

    if( Indexer->ChannelType == BXPT_RaveIdx_eTpit )
    {
        TpitIndexer *lhTpit = Indexer->hChannel;

        /* Disconnect it from the Record channel. */
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET );
        Reg &= ~(
#if (BCHP_CHIP == 7440 && BCHP_VER >= BCHP_VER_B0 )
            BCHP_MASK( XPT_RAVE_CX0_MISC_CONFIG, TPIT_ENABLE ) |
            BCHP_MASK( XPT_RAVE_CX0_MISC_CONFIG, TPIT_CHANNEL )
#else
            BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_ENABLE ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_CHANNEL )
#endif
        );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

        InitTpit( lhTpit );             /* Reset it first. */
        lhTpit->Allocated = false;      /* Free it */
    }
    else
    {
        StartCodeIndexer **lhScd = Indexer->hChannel;
        unsigned ii;

        ChangeScdPid( hCtx, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET, 0, 0, 0x1FFF, false );
        ChangeScdPid( hCtx, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET, 1, 0, 0x1FFF, false );

        ChangeScdPid( hCtx, hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET, 2, 0, 0x1FFF, false );
        ChangeScdPid( hCtx, hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET, 3, 0, 0x1FFF, false );

        ChangeScdPid( hCtx, hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET, 4, 0, 0x1FFF, false );
        ChangeScdPid( hCtx, hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET, 5, 0, 0x1FFF, false );

        ChangeScdPid( hCtx, hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET, 6, 0, 0x1FFF, false );
        ChangeScdPid( hCtx, hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET, 7, 0, 0x1FFF, false );

        for( ii = 0; ii < Indexer->NumChannels; ii++ )
        {
            InitScd( *(lhScd + ii) );
        }
        FreeScds( Indexer->NumChannels, lhScd );

        BKNI_Free( Indexer->hChannel );
    }

    Indexer->Allocated = false;

    BDBG_OBJECT_UNSET(Indexer, bxpt_t);

    return( ExitCode );
}

BERR_Code BXPT_Rave_GetIndexerConfig(
    BXPT_RaveIdx_Handle Indexer,        /* [in] Handle for the allocated indexer */
    BXPT_Rave_IndexerSettings *Config   /* [out] The indexer settings */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Indexer, bxpt_t);
    BDBG_ASSERT( Config );

    if( Indexer->ChannelType == BXPT_RaveIdx_eTpit )
    {
        TpitIndexer *hTpit = ( TpitIndexer * ) Indexer->hChannel;

        Reg = BREG_Read32( hTpit->hReg, hTpit->BaseAddr + TPIT_CTRL1_OFFSET );

#if ( BCHP_CHIP == 7403 )
        Config->Cfg.Tpit.TimeTickEn = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_TPIT0_CTRL1, TPIT_TIME_TICK_EN );
#endif

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

#if BXPT_NEXT_GEN_COMPARATORS

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
        Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_D );
        GetScEnables_Indexer( &Config->Cfg.Scd.ScRange[ 3 ], Mode );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
        Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_C );
        GetScEnables_Indexer( &Config->Cfg.Scd.ScRange[ 2 ], Mode );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
        Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_B );
        GetScEnables_Indexer( &Config->Cfg.Scd.ScRange[ 1 ], Mode );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
        Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_A );
        GetScEnables_Indexer( &Config->Cfg.Scd.ScRange[ 0 ], Mode );
#else
        BSTD_UNUSED( Mode );
        BSTD_UNUSED( GetScEnables_Indexer );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_VID_SC_RANGE_AB_OFFSET );
        Config->Cfg.Scd.ScRange[ 1 ].RangeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEB_HI );
        Config->Cfg.Scd.ScRange[ 1 ].RangeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEB_LO );
        Config->Cfg.Scd.ScRange[ 0 ].RangeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEA_HI );
        Config->Cfg.Scd.ScRange[ 0 ].RangeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEA_LO );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_VID_SC_RANGE_CD_OFFSET );
        Config->Cfg.Scd.ScRange[ 3 ].RangeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGED_HI );
        Config->Cfg.Scd.ScRange[ 3 ].RangeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGED_LO );
        Config->Cfg.Scd.ScRange[ 2 ].RangeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGEC_HI );
        Config->Cfg.Scd.ScRange[ 2 ].RangeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGEC_LO );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG2_OFFSET );
        Config->Cfg.Scd.ScRange[ 3 ].RangeEnable =   BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGED_EN    );
        Config->Cfg.Scd.ScRange[ 3 ].RangeIsASlice = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGED_SLICE );
        Config->Cfg.Scd.ScRange[ 2 ].RangeEnable =   BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEC_EN    );
        Config->Cfg.Scd.ScRange[ 2 ].RangeIsASlice = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEC_SLICE );
        Config->Cfg.Scd.ScRange[ 1 ].RangeEnable =   BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEB_EN    );
        Config->Cfg.Scd.ScRange[ 1 ].RangeIsASlice = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEB_SLICE );
        Config->Cfg.Scd.ScRange[ 0 ].RangeEnable =   BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEA_EN    );
        Config->Cfg.Scd.ScRange[ 0 ].RangeIsASlice = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEA_SLICE );
#endif
    }

    return( ExitCode );
}

BERR_Code BXPT_Rave_SetIndexerConfig(
    BXPT_RaveIdx_Handle Indexer,            /* [in] Handle for the allocated indexer */
    const BXPT_Rave_IndexerSettings *Config /* [in] The indexer settings */
    )
{
    uint32_t Reg;

#if BXPT_NEXT_GEN_COMPARATORS
    int pesSyncMode;
#endif

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Indexer, bxpt_t);
    BDBG_ASSERT( Config );

    if( Indexer->ChannelType == BXPT_RaveIdx_eTpit )
    {
        TpitIndexer *hTpit = ( TpitIndexer * ) Indexer->hChannel;

#if ( BCHP_CHIP == 7401 )
        BXPT_RaveCx_Handle Context = (BXPT_RaveCx_Handle) Indexer->vhCtx;

        Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_RESERVE_CFG2_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_RESERVE_CFG2, REC_EI_DEBUG ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_RESERVE_CFG2, REC_FP_DEBUG )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_RESERVE_CFG2, REC_FP_DEBUG, Config->Cfg.Tpit.FirstPacketEnable ? 1 : 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_RESERVE_CFG2, REC_EI_DEBUG, (Config->Cfg.Tpit.IdleEventEnable || Config->Cfg.Tpit.RecordEventEnable) ? 1 : 0 )
        );
        BREG_Write32( Context->hReg, Context->BaseAddr + AV_RESERVE_CFG2_OFFSET, Reg );
#endif

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

#if BXPT_NEXT_GEN_COMPARATORS

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
        if(pesSyncMode == 1)
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
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_D ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_D ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_C ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_C ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_B ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_B ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_A ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_A )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_D, Config->Cfg.Scd.ScRange[ 3 ].RangeEnable ? ( Config->Cfg.Scd.ScRange[ 3 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_D, 3 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_C, Config->Cfg.Scd.ScRange[ 2 ].RangeEnable ? ( Config->Cfg.Scd.ScRange[ 2 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_C, 3 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_B, Config->Cfg.Scd.ScRange[ 1 ].RangeEnable ? ( Config->Cfg.Scd.ScRange[ 1 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_B, 3 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_A, Config->Cfg.Scd.ScRange[ 0 ].RangeEnable ? ( Config->Cfg.Scd.ScRange[ 0 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_A, 3 )
        );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET, Reg );
#else
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_VID_SC_RANGE_AB_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEB_HI ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEB_LO ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEA_HI ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEA_LO )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEB_HI, Config->Cfg.Scd.ScRange[ 1 ].RangeHi ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEB_LO, Config->Cfg.Scd.ScRange[ 1 ].RangeLo ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEA_HI, Config->Cfg.Scd.ScRange[ 0 ].RangeHi ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEA_LO, Config->Cfg.Scd.ScRange[ 0 ].RangeLo )
        );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_VID_SC_RANGE_AB_OFFSET, Reg );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_VID_SC_RANGE_CD_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGED_HI ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGED_LO ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGEC_HI ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGEC_LO )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGED_HI, Config->Cfg.Scd.ScRange[ 3 ].RangeHi ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGED_LO, Config->Cfg.Scd.ScRange[ 3 ].RangeLo ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGEC_HI, Config->Cfg.Scd.ScRange[ 2 ].RangeHi ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGEC_LO, Config->Cfg.Scd.ScRange[ 2 ].RangeLo )
        );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_VID_SC_RANGE_CD_OFFSET, Reg );

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG2_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGED_EN ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGED_SLICE ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEC_EN ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEC_SLICE ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEB_EN ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEB_SLICE ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEA_EN ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEA_SLICE )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGED_EN,       Config->Cfg.Scd.ScRange[ 3 ].RangeEnable ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGED_SLICE,    Config->Cfg.Scd.ScRange[ 3 ].RangeIsASlice ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEC_EN,       Config->Cfg.Scd.ScRange[ 2 ].RangeEnable ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEC_SLICE,    Config->Cfg.Scd.ScRange[ 2 ].RangeIsASlice ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEB_EN,       Config->Cfg.Scd.ScRange[ 1 ].RangeEnable ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEB_SLICE,    Config->Cfg.Scd.ScRange[ 1 ].RangeIsASlice ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEA_EN,       Config->Cfg.Scd.ScRange[ 0 ].RangeEnable ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEA_SLICE,    Config->Cfg.Scd.ScRange[ 0 ].RangeIsASlice )
        );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG2_OFFSET, Reg );
#endif
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

    BDBG_OBJECT_ASSERT(Context, bxpt_t);

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

    BDBG_OBJECT_ASSERT(Indexer, bxpt_t);
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
#if (BCHP_CHIP == 7440 && BCHP_VER >= BCHP_VER_B0 )
    Reg &= ~BCHP_MASK( XPT_RAVE_CX0_MISC_CONFIG, SCD_MAP_MODE );
#else
    Reg &= ~BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE );
#endif
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

    switch( WhichScd )
    {
        case 0:
        case 1:
        ChangeScdPid( hCtx, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET, WhichScd, hScd->ChannelNo, ScdConfig->PidChannel, true );
        break;

        case 2:
        case 3:
        ChangeScdPid( hCtx, hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET, WhichScd, hScd->ChannelNo, ScdConfig->PidChannel, true );
        break;

        case 4:
        case 5:
        ChangeScdPid( hCtx, hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET, WhichScd, hScd->ChannelNo, ScdConfig->PidChannel, true );
        break;

        case 6:
        case 7:
        ChangeScdPid( hCtx, hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET, WhichScd, hScd->ChannelNo, ScdConfig->PidChannel, true );
        break;

        default:
        BDBG_ERR(( "WhichScd %u not valid!", WhichScd ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        break;
    }

    ExitCode = SetExtractionBitCounts( hCtx, hScd, ScdConfig, WhichScd );

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
    uint32_t Reg;
    BXPT_RaveCx_Handle hCtx;
    StartCodeIndexer *hScd;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Indexer, bxpt_t);
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
#if (BCHP_CHIP == 7440 && BCHP_VER >= BCHP_VER_B0 )
    Reg &= ~BCHP_MASK( XPT_RAVE_CX0_MISC_CONFIG, SCD_MAP_MODE );
    Reg |= BCHP_FIELD_DATA( XPT_RAVE_CX0_MISC_CONFIG, SCD_MAP_MODE, 1 );
#else
    Reg &= ~BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE );
    Reg |= BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE, 1 );
#endif
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

#if (BCHP_CHIP == 3563) || (BCHP_CHIP == 7403) || (BCHP_CHIP == 7405) || ( BCHP_CHIP == 7002 )
    Reg = (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_stream_PID_values_SCD_NUMA, hScd->ChannelNo ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_stream_PID_values_PIDA_VALID, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_stream_PID_values_SCD_PIDA, Pid )
    );
#else
    /* RDB for the other chips hasn't been updated yet. */
    Reg = ( Pid & 0x1FFFF ) | (( hScd->ChannelNo & 0x3F ) << 24 ) | ( 1 << 15 );
#endif

    switch( WhichScd )
    {
        case 0:
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET, Reg );
        break;

        case 1:
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET, Reg );
        break;

        case 2:
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET, Reg );
        break;

        case 3:
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET, Reg );
        break;

        default:
        BDBG_ERR(( "WhichScd %u not valid!", WhichScd ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    ExitCode = SetExtractionBitCounts_ByPid( hCtx, hScd, ScdConfig, WhichScd );

    Done:
    return( ExitCode );
}

#undef BXPT_MAX_ES_COUNT
#if BXPT_HAS_STARTCODE_BUFFER_WORKAROUND
#define BXPT_MAX_ES_COUNT 9
#else
#define BXPT_MAX_ES_COUNT 8
#endif

static BERR_Code SetExtractionBitCounts(
    BXPT_RaveCx_Handle hCtx,
    StartCodeIndexer *hScd,
    const BXPT_Rave_ScdEntry *ScdConfig,     /* [in] The indexer settings */
    unsigned WhichScd
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;
    uint32_t EsCount = 0;

    if( ScdConfig->EsCount > BXPT_MAX_ES_COUNT )
    {
        BDBG_ERR(( "EsCount of %u exceeds limit of %u bytes. Clamping to %u bytes.", ScdConfig->EsCount, BXPT_MAX_ES_COUNT, BXPT_MAX_ES_COUNT ));
        EsCount = BXPT_MAX_ES_COUNT;
    }
    else if( ScdConfig->EsCount < BXPT_MIN_ES_COUNT )
    {
        BDBG_ERR(( "EsCount must be at least %d. %d will be used.", BXPT_MIN_ES_COUNT, BXPT_MIN_ES_COUNT ));
        EsCount = 1;
    }
    else
    {
        EsCount = ScdConfig->EsCount;
    }

    /* Crazy bit mapping in the hardware. See the RDB entry for XPT_RAVE_SCD0_SCD_CTRL1.DATA_EXTRACT_NUM_BITS_B */
    EsCount = ( EsCount * 8 ) - 1;

#if BXPT_PTS_CONTROL_IN_REC_REG

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

    BSTD_UNUSED( hCtx );
    BSTD_UNUSED( WhichScd );
    /*
    ** Number of bits to extract after certain startcode sequences are seen. The number to extract includes the
    ** startcode itself.
    ** DATA_EXTRACT_NUM_BITS_B - number of bits to extract after PES startcode seen
    ** DATA_EXTRACT_NUM_BITS_A - number of bits to extract after ES startcode seen.
    */
    Reg = BREG_Read32( hScd->hReg, hScd->BaseAddr + SCD_CTRL1_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_SCD0_SCD_CTRL1, LOAD_COMP_CONFIG ) |
        BCHP_MASK( XPT_RAVE_SCD0_SCD_CTRL1, DATA_EXTRACT_NUM_BITS_B ) |
        BCHP_MASK( XPT_RAVE_SCD0_SCD_CTRL1, DATA_EXTRACT_NUM_BITS_A ) |
        BCHP_MASK( XPT_RAVE_SCD0_SCD_CTRL1, SCD_PTS_MODE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_SCD0_SCD_CTRL1, LOAD_COMP_CONFIG, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_SCD0_SCD_CTRL1, DATA_EXTRACT_NUM_BITS_B, 23 ) |
        BCHP_FIELD_DATA( XPT_RAVE_SCD0_SCD_CTRL1, DATA_EXTRACT_NUM_BITS_A, EsCount ) |
        BCHP_FIELD_DATA( XPT_RAVE_SCD0_SCD_CTRL1, SCD_PTS_MODE, ScdConfig->ExtractPts == true ? 1 : 0 )
    );
    BREG_Write32( hScd->hReg, hScd->BaseAddr + SCD_CTRL1_OFFSET, Reg );
#endif
    return( ExitCode );
}


static BERR_Code SetExtractionBitCounts_ByPid(
    BXPT_RaveCx_Handle hCtx,
    StartCodeIndexer *hScd,
    const BXPT_Rave_ScdEntry *ScdConfig,     /* [in] The indexer settings */
    unsigned WhichScd
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;
    uint32_t EsCount = 0;

    if( ScdConfig->EsCount > BXPT_MAX_ES_COUNT )
    {
        BDBG_ERR(( "EsCount of %u exceeds limit of %u bytes. Clamping to %u bytes.", ScdConfig->EsCount, BXPT_MAX_ES_COUNT, BXPT_MAX_ES_COUNT ));
        EsCount = BXPT_MAX_ES_COUNT;
    }
    else if( ScdConfig->EsCount < BXPT_MIN_ES_COUNT )
    {
        BDBG_ERR(( "EsCount must be at least %d. %d will be used.", BXPT_MIN_ES_COUNT, BXPT_MIN_ES_COUNT ));
        EsCount = 1;
    }
    else
    {
        EsCount = ScdConfig->EsCount;
    }

    /* Crazy bit mapping in the hardware. See the RDB entry for XPT_RAVE_SCD0_SCD_CTRL1.DATA_EXTRACT_NUM_BITS_B */
    EsCount = ( EsCount * 8 ) - 1;

#if BXPT_PTS_CONTROL_IN_REC_REG

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

    switch( WhichScd )
    {
        case 0:
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET );
        Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB_Mapped_SCD_via_stream_PID_values, SCD_PTS_MODE_PID_A ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB_Mapped_SCD_via_stream_PID_values, SCD_PTS_MODE_PID_A, ScdConfig->ExtractPts == true ? 1 : 0 ) );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET, Reg );
        break;

        case 1:
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET );
        Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_CD_Mapped_SCD_via_stream_PID_values, SCD_PTS_MODE_PID_C ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_CD_Mapped_SCD_via_stream_PID_values, SCD_PTS_MODE_PID_C, ScdConfig->ExtractPts == true ? 1 : 0 ) );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET, Reg );
        break;

        case 2:
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET );
        Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_EF_Mapped_SCD_via_stream_PID_values, SCD_PTS_MODE_PID_E ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_EF_Mapped_SCD_via_stream_PID_values, SCD_PTS_MODE_PID_E, ScdConfig->ExtractPts == true ? 1 : 0 ) );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET, Reg );
        break;

        case 3:
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET );
        Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_GH_Mapped_SCD_via_stream_PID_values, SCD_PTS_MODE_PID_G ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_GH_Mapped_SCD_via_stream_PID_values, SCD_PTS_MODE_PID_G, ScdConfig->ExtractPts == true ? 1 : 0 ) );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET, Reg );
        break;

        default:
        BDBG_ERR(( "Unsupported indexer number" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        break;
    }

#else

    BSTD_UNUSED( hCtx );
    BSTD_UNUSED( WhichScd );
    /*
    ** Number of bits to extract after certain startcode sequences are seen. The number to extract includes the
    ** startcode itself.
    ** DATA_EXTRACT_NUM_BITS_B - number of bits to extract after PES startcode seen
    ** DATA_EXTRACT_NUM_BITS_A - number of bits to extract after ES startcode seen.
    */
    Reg = BREG_Read32( hScd->hReg, hScd->BaseAddr + SCD_CTRL1_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_SCD0_SCD_CTRL1, LOAD_COMP_CONFIG ) |
        BCHP_MASK( XPT_RAVE_SCD0_SCD_CTRL1, DATA_EXTRACT_NUM_BITS_B ) |
        BCHP_MASK( XPT_RAVE_SCD0_SCD_CTRL1, DATA_EXTRACT_NUM_BITS_A ) |
        BCHP_MASK( XPT_RAVE_SCD0_SCD_CTRL1, SCD_PTS_MODE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_SCD0_SCD_CTRL1, LOAD_COMP_CONFIG, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_SCD0_SCD_CTRL1, DATA_EXTRACT_NUM_BITS_B, 23 ) |
        BCHP_FIELD_DATA( XPT_RAVE_SCD0_SCD_CTRL1, DATA_EXTRACT_NUM_BITS_A, EsCount ) |
        BCHP_FIELD_DATA( XPT_RAVE_SCD0_SCD_CTRL1, SCD_PTS_MODE, ScdConfig->ExtractPts == true ? 1 : 0 )
    );
    BREG_Write32( hScd->hReg, hScd->BaseAddr + SCD_CTRL1_OFFSET, Reg );
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

    BDBG_OBJECT_ASSERT(Indexer, bxpt_t);

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
        BDBG_ERR(( "WhichFilter %lu is not valid (valid values are 1, 2, and 3)!", WhichPair ));
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

    BDBG_OBJECT_ASSERT(Indexer, bxpt_t);
    BDBG_ASSERT( Tpit );

    /*
    ** The PR 29691 workaround requires we not use TPIT PID Channel XPT_RAVE_TPIT<i>_PID_TABLE_0
    ** and corresponding PAR Table register XPT_RAVE_TPIT<i>_PAR_TABLE_0 (where i = 0 to 5).
    ** So, the define for BXPT_P_MAX_TPIT_PIDS is reduced from 16 to 15, and WhichFilter is
    ** remapped to 1 greater.
    */

    if( WhichFilter++ >= BXPT_P_MAX_TPIT_PIDS )
    {
        /* Bad table entry number. Complain. */
        BDBG_ERR(( "WhichFilter %lu is out of range!", WhichFilter ));
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

BERR_Code BXPT_Rave_GetBufferInfo(
    BXPT_RaveCx_Handle hCtx,        /* [in] The context. */
    BXPT_Rave_BufferInfo *BufferInfo    /* [out] CDB depth and size */
    )
{
    /* New buffer depth calc. Use READ and VALID */
    uint32_t Read, Valid, Wrap, Base, End;

    size_t ByteCount = 0;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
    BDBG_ASSERT( BufferInfo );

    Read = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_READ_PTR_OFFSET );
    Valid = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_VALID_PTR_OFFSET );
    Wrap = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_WRAP_PTR_OFFSET );
    Base = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET );
    End = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_END_PTR_OFFSET );

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

    BDBG_OBJECT_ASSERT(Context, bxpt_t);
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

#if BXPT_PER_CONTEXT_PIC_COUNTER
    /* Picture Counter Increment/Decrement/Reset Control Register */
    Map->PicIncDecCtrl = Context->BaseAddr + PIC_INC_DEC_CTRL_OFFSET;
#endif

    Map->ContextIdx = Context->Index;

    return( ExitCode );
}

static uint32_t GetPictureCounterReg(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t PictureCounter = 0;

#if BXPT_PER_CONTEXT_PIC_COUNTER
    PictureCounter = hCtx->BaseAddr + PICTURE_CTR_OFFSET;
#else
    /* See if we have one allocated to this context. */
    if( hCtx->PicCounter != -1 )
        PictureCounter = BCHP_XPT_RAVE_PIC_CTR0 + ( 4 * hCtx->PicCounter );
#endif

    return PictureCounter;
}

BERR_Code BXPT_Rave_GetRecordConfig(
    BXPT_RaveCx_Handle Ctx,         /* [in] The context  */
    BXPT_Rave_RecordSettings *Cfg   /* [out] The record settings. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Ctx, bxpt_t);
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

#if BXPT_HAS_RAVE_AUTO_READ
    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG_OFFSET );
    Cfg->AutoReadEn = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_MISC_CONFIG, REC_AUTO_UPDATE_RD_PTR_EN ) ? true : false;
#endif

    return( ExitCode );
}

BERR_Code BXPT_Rave_SetRecordConfig(
    BXPT_RaveCx_Handle Ctx,             /* [in] The context  */
    const BXPT_Rave_RecordSettings *Cfg /* [in] The record settings. */
    )
{
    uint32_t Reg;

    unsigned PesSyncMode = 1;
    unsigned OutputFormat = 2;
    unsigned InputEsFormat = 0;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Ctx, bxpt_t);
    BDBG_ASSERT( Cfg );

    if( Ctx->Type == BXPT_RaveCx_eIp )
    {
        PesSyncMode = 3;
        InputEsFormat = 0x0A;
    }
    else
        PesSyncMode = Cfg->MpegMode == true ? 1 : 3;

    switch( Cfg->OutputFormat )
    {
        case BAVC_StreamType_eTsMpeg:
        OutputFormat = 2;
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
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_LO )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, BAND_HOLD_EN, Cfg->BandHoldEn == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, SHIFT_PTS, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE, PesSyncMode ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, OUTPUT_FORMAT, OutputFormat ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, INPUT_ES_FORMAT, InputEsFormat ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_HI, Cfg->StreamIdHi ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_LO, Cfg->StreamIdLo )
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

    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + CDB_BASE_PTR_OFFSET );
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
            BREG_Write32( Ctx->hReg, Ctx->BaseAddr + CDB_END_PTR_OFFSET, Reg + Ctx->usedCdbBufferSize - 1 );
        }

    }
    else
    {
        /* adjust the cdb end pointer to allocated size */
        Ctx->usedCdbBufferSize = Ctx->allocatedCdbBufferSize;
        BREG_Write32( Ctx->hReg, Ctx->BaseAddr + CDB_END_PTR_OFFSET, Reg + Ctx->usedCdbBufferSize - 1 );
    }

#if BXPT_HAS_RAVE_AUTO_READ
    Reg = BREG_Read32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG_OFFSET );
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, REC_AUTO_UPDATE_RD_PTR_EN ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, REC_AUTO_UPDATE_RD_PTR_EN, Cfg->AutoReadEn == true ? 1 : 0 ) );
    BREG_Write32( Ctx->hReg, Ctx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );
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

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Context, bxpt_t);
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

#if ( BCHP_CHIP == 7401 ) || ( BCHP_CHIP == 7118 )
    Config->VobMode = Context->VobMode;
#endif

    Config->InputFormat = Context->InputFormat;
    Config->ItbFormat = Context->ItbFormat;
    Config->StreamIdHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_HI );
    Config->StreamIdLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_LO );
    Config->BandHoldEn = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, BAND_HOLD_EN ) ? true : false;

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_CDB_THRESHOLD_OFFSET );
    Config->CdbUpperThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_UPPER_THRESHOLD );
    Config->CdbLowerThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_CDB_THRESHOLD_LEVEL, CDB_LOWER_THRESHOLD );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_ITB_THRESHOLD_OFFSET );
    Config->ItbUpperThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_UPPER_THRESHOLD );
    Config->ItbLowerThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_ITB_THRESHOLD_LEVEL, ITB_LOWER_THRESHOLD );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG3_OFFSET );
#if ( BCHP_CHIP == 7403 )
    Config->EnableBPPSearch = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_BPP_SEARCH ) ? true : false;
#endif
    Config->DisableContinuityCheck = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_CC_CHECK ) ? true : false;
    Config->PcrRtsEntryEn = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_PCR_RTS_ENTRIES ) ? true : false;
    Config->DisablePacketErrors = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_PKT_ERRORS ) ? true : false;
    Config->PesSidExcludeHi = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, PES_SID_EXCLUDE_HI);
    Config->PesSidExcludeLo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, PES_SID_EXCLUDE_LO);
    Config->Transcoding = Context->Transcoding;
    Config->ScOrMode = Context->ScOrMode;
    return( ExitCode );
}

BERR_Code BXPT_Rave_SetAvConfig(
    BXPT_RaveCx_Handle Context,         /* [in] The context. */
    const BXPT_Rave_AvSettings *Config  /* [in] The AV settings. */
    )
{
    uint32_t Reg;
    BAVC_ItbEsType ItbFormat;
    BXPT_Rave_Handle lhRave;
    unsigned MaxPatterns;

    unsigned PesType = 0;
    unsigned PesSyncMode = 0;
    bool ShiftPts = true;
    bool ConvertPts = false;
    bool EmulationPrevRemove = false;
    bool EmulationPrevMode = 0;     /* Default to byte removal */
    unsigned OutputFormat = 0;
    BERR_Code ExitCode = BERR_SUCCESS;
    unsigned EsFormat = 0;

    BDBG_OBJECT_ASSERT(Context, bxpt_t);
    BDBG_ASSERT( Config );

    lhRave = Context->vhRave;

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

    /* PR 36908: Remap AC3 to AC3+ for all chips other than these. */
#if ( BCHP_CHIP == 7118 ) || ( BCHP_CHIP == 7401 ) || ( BCHP_CHIP == 7403 ) || ( BCHP_CHIP == 7440 )
    ItbFormat = Config->ItbFormat;
#else
    /*
    ** Raptor engineers request that on the 7400, AC3 audio should be handled in the same
    ** way as AC3+. The XPT PI will honor AC3 audio enum in the Get/Set AV config calls, but
    ** will silently configure for AC3+.
    */
    if( Config->ItbFormat == BAVC_ItbEsType_eAc3gAudio )
        ItbFormat = BAVC_ItbEsType_eAc3Plus;
    else
        ItbFormat = Config->ItbFormat;
#endif

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
#if ( (BCHP_CHIP == 7401  &&  BCHP_VER >= BCHP_VER_B0) || (BCHP_CHIP == 7440) || BCHP_CHIP == 7118 )
            /* WMA data will be put into MPEG TS by the playback channel packetizer */
            case BAVC_ItbEsType_eWma:
            PesType = 0;
            PesSyncMode = 1;
            break;
#endif
#if 0       /* use BAVC_StreamType_eMpeg1System */
            case BAVC_ItbEsType_eMpeg1Video:   /* to do , remove this coz mpeg1 video
                                                  can come inside mpeg2 style pes headers */
            PesType = 1;
            PesSyncMode = 1;
            break;
#endif

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

#if (BCHP_CHIP == 7405 && BCHP_VER >= BCHP_VER_B0 ) || ( BCHP_CHIP == 3548 && BCHP_VER >= BCHP_VER_B1 ) || ( BCHP_CHIP == 7002 )
    EmulationPrevRemove = false;
    EmulationPrevMode = 0;
#endif

    /* Default to disabled, then enable only for certain CODECs */
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
        EmulationPrevRemove = false;
        EsFormat = 0x00;
        break;

        case BAVC_ItbEsType_eAc3Plus:
        EmulationPrevRemove = false;
#if BXPT_RAVE_AUDIO_STARTCODES
        EsFormat = 0x07;
#else
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */
#endif
        break;

        case BAVC_ItbEsType_eAacHe:
        case BAVC_ItbEsType_eAacAudio:
        EmulationPrevRemove = false;
#if BXPT_RAVE_AUDIO_STARTCODES
        EsFormat = 0x03;
#else
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */
#endif
        break;

        case BAVC_ItbEsType_eWma:
        EmulationPrevRemove = false;
#if BXPT_RAVE_AUDIO_STARTCODES
        EsFormat = 0x08;
#else
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */
#endif
        break;

        case BAVC_ItbEsType_eMpegAudio:
        case BAVC_ItbEsType_eMpegAudioLayer3:
        case BAVC_ItbEsType_eMpegAudio2_5:
        EmulationPrevRemove = false;
#if BXPT_RAVE_AUDIO_STARTCODES
        EsFormat = 0x02;
#else
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */
#endif
        /* SW7125-764: Don't put any data into the CDB until a PES header is seen. */
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
        /* SW7125-764 */

        break;

        case BAVC_ItbEsType_eMlpAudio:
#if BXPT_RAVE_AUDIO_STARTCODES
        EsFormat = 0x09;
#else
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */
#endif
        break;

        case BAVC_ItbEsType_eAvsVideo:
        EmulationPrevRemove = true;
        EmulationPrevMode = 1;  /* Emulation prev is a bit in AVS streams */
        EsFormat = 0x00;
        break;

        case BAVC_ItbEsType_eAvsAudio:
        EmulationPrevRemove = true;
        EmulationPrevMode = 1;  /* Emulation prev is a bit in AVS streams */
#if BXPT_RAVE_AUDIO_STARTCODES
        EsFormat = 0x03;    /* Use AAC as the baseline */
#else
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */
#endif
        break;

        case BAVC_ItbEsType_eMpegAudioWithDescriptor:

        /* SW7125-764 */
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
        /* SW7125-764 */
        /* Then fall through to BAVC_ItbEsType_eAudioDescriptor. */

        case BAVC_ItbEsType_eAudioDescriptor:
        EmulationPrevRemove = false;
        EsFormat = 0x0C;
        break;

        case BAVC_ItbEsType_eDra:
        EmulationPrevRemove = false;
#if BXPT_RAVE_AUDIO_STARTCODES
        EsFormat = 0x0B;
#else
        EsFormat = 0x01;    /* All audio codecs get ES FORMAT = 1 */
#endif
        break;

        case BAVC_ItbEsType_eOTFVideo:
        EmulationPrevRemove = false;
        EsFormat = ItbFormat;
        break;

        default:
        /* TODO: Add cases for the rest of the ITB formats. */
        EmulationPrevRemove = false;
#if BXPT_RAVE_AUDIO_STARTCODES
        EsFormat = ItbFormat;
#else
        EsFormat = 0x01;    /* Video doesn't care about the ES type in the ITB, so it's safe to map this for rest of the codecs.  */
#endif
        break;
    }

    BDBG_MSG(( "SetAvConfig for context %u: %s", Context->Index,
        EsFormat ? ( EsFormat == 0x0C ? "Audio Descriptor" : "Audio" ) : "Video" ));

    /* PR 28909: For transcoding applications, we need to keep the emulation prev bytes. */
    if( Config->Transcoding == true )
    {
        /* DEPRECATED: No projects are actively using this feature. It's interfering with
        ** the VCE-based transcoding tests. See SW7425-5829.
        EmulationPrevRemove = false;
        */
        BDBG_WRN(( "%s: Config->Transcoding is no longer supported", __FUNCTION__ ));
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

    LoadScRanges( Context, Config->EsRanges );

    /* No comparator configuration is required for BAVC_ItbEsType_eLpcmAudio.On 7440
       set the AudFrameInfo to appropriate value using BXPT_Rave_SetContextConfig*/
    /* if(!(ItbFormat == BAVC_ItbEsType_eLpcmAudio || ItbFormat == BAVC_ItbEsType_eMlpAudio) || ( ItbFormat == BAVC_ItbEsType_eDtsAudio ))
    {*/
        ConfigureComparators( Context, ItbFormat );
    /*}*/

#if ( BCHP_CHIP == 7401 ) || ( BCHP_CHIP == 7118 )
    /* 7401 needs a very unique config for LPCM */
    Context->VobMode = Config->VobMode; /* Note this setting for the GetAvConfig call */
    if( Config->VobMode )
    {
        Reg = BREG_Read32( Context->hReg, Context->BaseAddr + REC_MISC_CFG_OFFSET );
        Reg &= ~BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE );
        Reg |= BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE, 2 );
        BREG_Write32( Context->hReg, Context->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

        PesSyncMode = 2;
    }
#endif

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG1_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_TYPE_MODE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, SHIFT_PTS ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, CONVERT_PTS ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, EMU_PREV_BYTE_REMOVE ) |
#if (BCHP_CHIP == 7405 && BCHP_VER >= BCHP_VER_B0 ) || ( BCHP_CHIP == 7336 ) || ( BCHP_CHIP == 3548 && BCHP_VER >= BCHP_VER_B1 ) || ( BCHP_CHIP == 7002 ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125)
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, EMU_PREV_MODE ) |
#endif
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, BAND_HOLD_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, INPUT_ES_FORMAT ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_HI ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, STREAM_ID_LO ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, OUTPUT_FORMAT )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_TYPE_MODE, PesType ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, PES_SYNC_MODE, PesSyncMode ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, SHIFT_PTS, ShiftPts ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, CONVERT_PTS, ConvertPts ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, EMU_PREV_BYTE_REMOVE, EmulationPrevRemove ) |
#if BXPT_HAS_AVS
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, EMU_PREV_MODE, EmulationPrevMode ) |
#endif
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, BAND_HOLD_EN, Config->BandHoldEn == true ? 1 : 0 ) |
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
#if ( BCHP_CHIP == 7403 || BCHP_CHIP == 7440 )
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_BPP_SEARCH ) |
#endif
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_CC_CHECK ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_PCR_RTS_ENTRIES ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_PKT_ERRORS )
    );

    Reg |= (
#if ( BCHP_CHIP == 7403 || BCHP_CHIP == 7440 )
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_BPP_SEARCH, Config->EnableBPPSearch == true ? 1 : 0 ) |
#endif
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_CC_CHECK, Config->DisableContinuityCheck == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_PCR_RTS_ENTRIES, Config->PcrRtsEntryEn == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_PKT_ERRORS, Config->DisablePacketErrors == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, PES_SID_EXCLUDE_HI, Config->PesSidExcludeHi ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, PES_SID_EXCLUDE_LO, Config->PesSidExcludeLo )
    );

#if  ( BCHP_CHIP == 7401 ) || ( BCHP_CHIP == 7403 ) || ( BCHP_CHIP == 7118 )
        /*PR46839 : As per the information from hardware team, Playback packetizer Hardware
        bug fix is not present on 7401, 7403, 3563, 7118 chips. This is a SW workaround until fixed*/
        if ( ItbFormat == BAVC_ItbEsType_eWma ) {
            Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG3_OFFSET );
            Reg &= ~(
                BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_CC_CHECK )
            );
            Reg |= (BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG3, DISABLE_CC_CHECK, 0));
            BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG3_OFFSET, Reg );
        }
#endif

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

#ifdef UNIFIED_ITB_SUPPORT
    /* PR 32434: ITB Config spreadsheet ver 1.6 required MaxCompare == 15 for audio */
    switch( ItbFormat )
    {
        case BAVC_ItbEsType_eMpeg2Video:
        case BAVC_ItbEsType_eAvcVideo:
        case BAVC_ItbEsType_eMpeg1Video:
        case BAVC_ItbEsType_eMpeg4Part2:
        case BAVC_ItbEsType_eH263:
        case BAVC_ItbEsType_eVc1Video:
        #if ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || ( BCHP_CHIP == 7405 && BCHP_VER >= BCHP_VER_B0 ) \
        || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || (BCHP_CHIP == 7420 ) || ( BCHP_CHIP == 7635 ) || ( BCHP_CHIP == 7002 ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 ) || ( BCHP_CHIP == 7630 ) || (BCHP_CHIP == 7408)
        MaxPatterns = 52;     /* AV contexts using 1-byte extraction */
        #else
        MaxPatterns =53;
        #endif
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
#if ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || ( BCHP_CHIP == 7405 && BCHP_VER >= BCHP_VER_B0 ) \
    || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || (BCHP_CHIP == 7420 ) || ( BCHP_CHIP == 7635 ) || ( BCHP_CHIP == 7002 ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 ) || ( BCHP_CHIP == 7630 )
        MaxPatterns = 14;   /* For chips supporting AVS. ToDo: add the other chips. */
#else
        MaxPatterns = 15;   /* Contexts using 8-byte extraction format */
#endif
        break;
    }
#else
    /*
    ** PR 27902: Workaround for this PR requires limiting number of startcode extracted
    ** from a single transport packet. We'll set that limit to 16 for records and
    ** 53 for AV (shorter blocks of data are extracted for the decoders). For OTF PVR,
    ** we need to extract 9 bytes of data.
    **
    ** There is similar code to set the MaxPatterns for record contexts in
    ** BXPT_Rave_SetRecConfig().
    */

    /* PR 31547: Smaller MaxPatterns for MPEG audio */
    switch( ItbFormat )
    {
        case BAVC_ItbEsType_eMpegAudio:
        case BAVC_ItbEsType_eMpegAudio2_5:
        case BAVC_ItbEsType_eMpegAudioLayer3:
        MaxPatterns = 38;     /* MPEG audio. */
        break;

        case BAVC_ItbEsType_eOTFVideo:
        MaxPatterns = 14;
        break;

        default:
        MaxPatterns = 53;     /* AV contexts using 1-byte extraction */
        break;
    }
#endif

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
    uint32_t Read, Valid, Wrap, Base;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Context, bxpt_t);
    BDBG_ASSERT( Ptrs );

    Read = BREG_Read32( Context->hReg, Context->BaseAddr + CDB_READ_PTR_OFFSET );
    Valid = BREG_Read32( Context->hReg, Context->BaseAddr + CDB_VALID_PTR_OFFSET );
    Wrap = BREG_Read32( Context->hReg, Context->BaseAddr + CDB_WRAP_PTR_OFFSET );
    Base = BREG_Read32( Context->hReg, Context->BaseAddr + CDB_BASE_PTR_OFFSET );

    if( Read < Valid )
    {
        if( Context->CdbReset == true )
        {
            /*
            ** Checks after a context reset, but before the first update of the READ, are a special case.
            ** We need to do account for the fact that first burst of data written into the buffer
            ** started AT the BASE pointer, not VALID+1.
            */
            BMEM_ConvertOffsetToAddress( Context->hMem, Read, (void *) &(Ptrs->Cdb.DataPtr) );
            Ptrs->Cdb.ByteCount = Valid - Read + 1;
        }
        else
        {
            BMEM_ConvertOffsetToAddress( Context->hMem, Read + 1, (void *) &(Ptrs->Cdb.DataPtr) );
            Ptrs->Cdb.ByteCount = Valid - Read;
        }
        BMEM_ConvertAddressToCached( Context->hMem, Ptrs->Cdb.DataPtr, (void *) &(Ptrs->Cdb.DataPtr) );

        Ptrs->Cdb.WrapDataPtr = NULL;
        Ptrs->Cdb.WrapByteCount = 0;
    }
    else if( Read > Valid )
    {
        /* It did wrap */
        if( Read == Wrap )
        {
            /* They read up to the wraparound point. New data starts at the base */
            BMEM_ConvertOffsetToAddress( Context->hMem, Base, (void *) &(Ptrs->Cdb.DataPtr) );
            BMEM_ConvertAddressToCached( Context->hMem, Ptrs->Cdb.DataPtr, (void *) &(Ptrs->Cdb.DataPtr) );
            Ptrs->Cdb.ByteCount = Valid - Base + 1;

            Ptrs->Cdb.WrapDataPtr = NULL;
            Ptrs->Cdb.WrapByteCount = 0;
        }
        else
        {
            BMEM_ConvertOffsetToAddress( Context->hMem, Read + 1, (void *) &(Ptrs->Cdb.DataPtr) );
            BMEM_ConvertAddressToCached( Context->hMem, Ptrs->Cdb.DataPtr, (void *) &(Ptrs->Cdb.DataPtr) );
            Ptrs->Cdb.ByteCount = Wrap - Read;

            BMEM_ConvertOffsetToAddress( Context->hMem, Base, (void *) &(Ptrs->Cdb.WrapDataPtr) );
            BMEM_ConvertAddressToCached( Context->hMem, Ptrs->Cdb.WrapDataPtr, (void *) &(Ptrs->Cdb.WrapDataPtr) );
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

    Read = BREG_Read32( Context->hReg, Context->BaseAddr + ITB_READ_PTR_OFFSET );
    Valid = BREG_Read32( Context->hReg, Context->BaseAddr + ITB_VALID_PTR_OFFSET );
    Wrap = BREG_Read32( Context->hReg, Context->BaseAddr + ITB_WRAP_PTR_OFFSET );
    Base = BREG_Read32( Context->hReg, Context->BaseAddr + ITB_BASE_PTR_OFFSET );

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
            BMEM_ConvertOffsetToAddress( Context->hMem, Read, (void *) &(Ptrs->Itb.DataPtr) );
            Ptrs->Itb.ByteCount = Valid - Read + 1;
        }
        else
        {
            BMEM_ConvertOffsetToAddress( Context->hMem, Read + 1, (void *) &(Ptrs->Itb.DataPtr) );
            Ptrs->Itb.ByteCount = Valid - Read;
        }
        BMEM_ConvertAddressToCached( Context->hMem, Ptrs->Itb.DataPtr, (void *) &(Ptrs->Itb.DataPtr) );

        Ptrs->Itb.WrapDataPtr = NULL;
        Ptrs->Itb.WrapByteCount = 0;
    }
    else if( Base && ( Read > Valid ) )
    {
        /* It did wrap */
        if( Read == Wrap )
        {
            /* They read up to the wraparound point. New data starts at the base */
            BMEM_ConvertOffsetToAddress( Context->hMem, Base, (void *) &(Ptrs->Itb.DataPtr) );
            BMEM_ConvertAddressToCached( Context->hMem, Ptrs->Itb.DataPtr, (void *) &(Ptrs->Itb.DataPtr) );
            Ptrs->Itb.ByteCount = Valid - Base + 1;

            Ptrs->Itb.WrapDataPtr = NULL;
            Ptrs->Itb.WrapByteCount = 0;
        }
        else
        {
            BMEM_ConvertOffsetToAddress( Context->hMem, Read + 1, (void *) &(Ptrs->Itb.DataPtr) );
            BMEM_ConvertAddressToCached( Context->hMem, Ptrs->Itb.DataPtr, (void *) &(Ptrs->Itb.DataPtr) );
            Ptrs->Itb.ByteCount = Wrap - Read;

            BMEM_ConvertOffsetToAddress( Context->hMem, Base, (void *) &(Ptrs->Itb.WrapDataPtr) );
            BMEM_ConvertAddressToCached( Context->hMem, Ptrs->Itb.WrapDataPtr, (void *) &(Ptrs->Itb.WrapDataPtr) );
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
    uint32_t Read, Valid, Wrap, Base;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Context, bxpt_t);

    if( CdbByteCount )
    {
        uint32_t NewRead = 0;
        Read = BREG_Read32( Context->hReg, Context->BaseAddr + CDB_READ_PTR_OFFSET );
        Valid = BREG_Read32( Context->hReg, Context->BaseAddr + CDB_VALID_PTR_OFFSET );
        Wrap = BREG_Read32( Context->hReg, Context->BaseAddr + CDB_WRAP_PTR_OFFSET );
        Base = BREG_Read32( Context->hReg, Context->BaseAddr + CDB_BASE_PTR_OFFSET );

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
            BREG_Write32( Context->hReg, Context->BaseAddr + CDB_READ_PTR_OFFSET, NewRead );
        }

        /* Check for a wrap-around. Use of Valid is the reccomended way to determine if the buffer has wrapped. */
        else if( Read > Valid )
        {
            /* It did wrap. Will this update move the READ pointer past the wrap point? */
            if( Read + CdbByteCount > Wrap )
            {
                /* Yes, so the READ pointer must 'wrap' too. */
                NewRead = CdbByteCount - ( Wrap - Read ) - 1;
                BREG_Write32( Context->hReg, Context->BaseAddr + CDB_READ_PTR_OFFSET, Base+NewRead );
            }
            else
            {
                /* No, this update only concerns data that didn't wrap. */
                NewRead = Read + CdbByteCount;
                BREG_Write32( Context->hReg, Context->BaseAddr + CDB_READ_PTR_OFFSET, NewRead );
            }
        }

        /* Data didn't wrap. */
        else
        {
            NewRead = Read + CdbByteCount;
            BREG_Write32( Context->hReg, Context->BaseAddr + CDB_READ_PTR_OFFSET, NewRead );
        }
    }

    if( ItbByteCount )
    {
        uint32_t NewRead = 0;
        Read = BREG_Read32( Context->hReg, Context->BaseAddr + ITB_READ_PTR_OFFSET );
        Valid = BREG_Read32( Context->hReg, Context->BaseAddr + ITB_VALID_PTR_OFFSET );
        Wrap = BREG_Read32( Context->hReg, Context->BaseAddr + ITB_WRAP_PTR_OFFSET );
        Base = BREG_Read32( Context->hReg, Context->BaseAddr + ITB_BASE_PTR_OFFSET );

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
                BREG_Write32( Context->hReg, Context->BaseAddr + ITB_READ_PTR_OFFSET, NewRead );
            }

            /* Check for a wrap-around. Use of Valid is the reccomended way to determine if the buffer has wrapped. */
            else if( Read > Valid )
            {
                /* It did wrap. Will this update move the READ pointer past the wrap point? */
                if( Read + ItbByteCount > Wrap )
                {
                    /* Yes, so the READ pointer must 'wrap' too. */
                    NewRead = ItbByteCount - ( Wrap - Read ) - 1;
                    BREG_Write32( Context->hReg, Context->BaseAddr + ITB_READ_PTR_OFFSET, Base + NewRead );
                }
                else
                {
                    /* No, this update only concerns data that didn't wrap. */
                    NewRead = Read + ItbByteCount;
                    BREG_Write32( Context->hReg, Context->BaseAddr + ITB_READ_PTR_OFFSET, NewRead );
                }
            }

            /* Data didn't wrap. */
            else
            {
                NewRead = Read + ItbByteCount;
                BREG_Write32( Context->hReg, Context->BaseAddr + ITB_READ_PTR_OFFSET, NewRead );
            }
        }
    }

    return( ExitCode );
}

BERR_Code BXPT_Rave_AddPidChannel(
    BXPT_RaveCx_Handle Context,     /* [in] The context  */
    unsigned int PidChanNum,        /* [in] Which PID channel to add. */
    bool UseDecrypted               /* [in] Use decrypted versions of packets on this channel */
    )
{
    uint32_t Reg, RegAddr, PipeShift, DummyRead;
    BXPT_Handle lhXpt;
    BXPT_Rave_Handle lhRave;
    bool isPidValid;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Context, bxpt_t);

    /* Sanity check on the arguments. */
    if( PidChanNum >= BXPT_P_MAX_PID_CHANNELS )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChanNum %lu is out of range!", ( unsigned long ) PidChanNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
#if (BCHP_CHIP != 7002)
        /* For 7002 we need ability to specify pipe for MMSCRAM function */
        /* PR 28674: Force decode contexts to use the R pipe */
        if( Context->Type == BXPT_RaveCx_eAv || Context->Type == BXPT_RaveCx_eAvR )
        {
            UseDecrypted = true;
        }
#endif

        BDBG_MSG(( "Adding PID channel %u to RAVE context %u", PidChanNum, Context->Index ));

        PipeShift = UseDecrypted == true ? 5 : 4;
        lhRave = ( BXPT_Rave_Handle ) Context->vhRave;
        lhXpt = ( BXPT_Handle ) lhRave->lvXpt;
        BXPT_P_SetPidChannelDestination( lhXpt, PidChanNum, PipeShift, true );

        /* AV context needs this information for filtering as well. */
        if( Context->Index < CXMEM_LO_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = Context->Index * 2;
        }
        else
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = ( Context->Index - CXMEM_LO_MAX_CONTEXT ) * 2;
        }

        /*
        ** Each PID channel entry in the CXMEM array has two bits for each context:
        ** One bit enables the R pipe, and the other enables the G pipe.
        */
        BKNI_EnterCriticalSection();
        Reg = BREG_Read32( Context->hReg, RegAddr );
        DummyRead = BREG_Read32( Context->hReg, BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE );   /* PR 50099 */
        BKNI_LeaveCriticalSection();
        PipeShift += UseDecrypted == true ? 0 : 1;
        Reg |= ( 1ul << PipeShift );
        BREG_Write32( Context->hReg, RegAddr, Reg );

        /* AV contexts need the PID channel mapped to their SCDs */
        if( Context->Type == BXPT_RaveCx_eAv || Context->Type == BXPT_RaveCx_eAvR || Context->Type == BXPT_RaveCx_eIp || Context->Type == BXPT_RaveCx_eVctNull )
        {
            Reg = BREG_Read32( Context->hReg, Context->BaseAddr + REC_SCD_PIDS_AB_OFFSET );

        #if BXPT_REC_SCD_MAPPING
            if ( (isPidValid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID )) == false )
        #elif BXPT_7440B0_SCD_MAPPING
            if ( (isPidValid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_SCD_PIDS_AB, PIDA_VALID )) == false )
        #else
            if ( (isPidValid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDA_VALID )) == false )
        #endif
            {
                Reg &= ~(
            #if BXPT_7440B0_SCD_MAPPING
                    BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, PIDA_VALID ) |
                    BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_PIDA )
            #elif BXPT_REC_SCD_MAPPING
                    BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID ) |
                    BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDA )
            #else
                    BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDA_VALID ) |
                    BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_PIDA )
            #endif
                );

                Reg |= (
             #if BXPT_7440B0_SCD_MAPPING
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_SCD_PIDS_AB, PIDA_VALID, 1 ) |
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_PIDA, PidChanNum )
             #elif BXPT_REC_SCD_MAPPING
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID, 1 ) |
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDA, PidChanNum )
             #else
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDA_VALID, 1 ) |
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_PIDA, PidChanNum )
             #endif
                );
            }
            else /* Extend the case to add to more SCD PID C/D/E/F if the need arise in future. */
            {
                unsigned ScdNum;

                for( ScdNum = 0; ScdNum < BXPT_P_MAX_SCD; ScdNum++ )
                    if( lhRave->ScdTable[ ScdNum ].Allocated == false )
                        break;

                if( ScdNum == BXPT_P_MAX_SCD )
                {
                    BDBG_ERR(( "No SCD channel is available for this AV context" ));
                    ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
                    return( ExitCode );
                }

                lhRave->ScdTable[ ScdNum ].Allocated = true;
                Context->hAvScd = lhRave->ScdTable + ScdNum;
                InitScd( Context->hAvScd );

                Reg &= ~(
            #if BXPT_7440B0_SCD_MAPPING
                    BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, PIDB_VALID ) |
                    BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_PIDB ) |
                    BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_NUMB )
            #elif BXPT_REC_SCD_MAPPING
                    BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDB_VALID ) |
                    BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDB ) |
                    BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMB )
            #else
                    BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDB_VALID ) |
                    BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_PIDB ) |
                    BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMB )
            #endif
                );
                Reg |= (
            #if BXPT_7440B0_SCD_MAPPING
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_SCD_PIDS_AB, PIDB_VALID, 1 ) |
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_PIDB, PidChanNum ) |
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_NUMB, ScdNum )
            #elif BXPT_REC_SCD_MAPPING
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDB_VALID, 1 ) |
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDB, PidChanNum ) |
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMB, ScdNum )
            #else
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDB_VALID, 1 ) |
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_PIDB, PidChanNum ) |
                    BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMB, ScdNum )
            #endif
                );
            }
            BREG_Write32( Context->hReg, Context->BaseAddr + REC_SCD_PIDS_AB_OFFSET, Reg );
        }
    }

    return( ExitCode );
}

BERR_Code BXPT_Rave_AddBppChannel(
    BXPT_RaveCx_Handle Context,     /* [in] The context  */
    unsigned int PidChanNum,        /* [in] Which PID channel to add. */
    bool UseDecrypted               /* [in] Use decrypted versions of packets on this channel */
    )
{
    uint32_t Reg, RegAddr, PipeShift, DummyRead;
    BXPT_Handle lhXpt;
    BXPT_Rave_Handle lhRave;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Context, bxpt_t);

    /* Sanity check on the arguments. */
    if( PidChanNum >= BXPT_P_MAX_PID_CHANNELS )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChanNum %lu is out of range!", ( unsigned long ) PidChanNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        BDBG_MSG(( "Adding BPP PID channel %u to RAVE context %u", PidChanNum, Context->Index ));

        /* PR 28674: Force decode contexts to use the R pipe */
        if( Context->Type == BXPT_RaveCx_eAv || Context->Type == BXPT_RaveCx_eAvR )
        {
            UseDecrypted = true;
        }

        PipeShift = UseDecrypted == true ? 5 : 4;
        lhRave = ( BXPT_Rave_Handle ) Context->vhRave;
        lhXpt = ( BXPT_Handle ) lhRave->lvXpt;
        BXPT_P_SetPidChannelDestination( lhXpt, PidChanNum, PipeShift, true );

        /* AV context needs this information for filtering as well. */
        if( Context->Index < CXMEM_LO_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = Context->Index * 2;
        }
        else
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = ( Context->Index - CXMEM_LO_MAX_CONTEXT ) * 2;
        }

        /*
        ** Each PID channel entry in the CXMEM array has two bits for each context:
        ** One bit enables the R pipe, and the other enables the G pipe.
        */
        BKNI_EnterCriticalSection();
        Reg = BREG_Read32( Context->hReg, RegAddr );
        DummyRead = BREG_Read32( Context->hReg, BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE );   /* PR 50099 */
        BKNI_LeaveCriticalSection();
        PipeShift += UseDecrypted == true ? 0 : 1;
        Reg |= ( 1ul << PipeShift );
        BREG_Write32( Context->hReg, RegAddr, Reg );
    }

    return( ExitCode );
}

BERR_Code BXPT_Rave_RemoveAllPidChannels(
    BXPT_RaveCx_Handle Context
    )
{
    uint32_t Reg, RegAddr, PipeShift, tempReg, DummyRead;
    unsigned int PidChanNum;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Context, bxpt_t);

    /* AV context needs this information for filtering as well. */
    if( Context->Index < CXMEM_LO_MAX_CONTEXT )
    {
        PipeShift = Context->Index * 2;
        tempReg = 0x03 << PipeShift;
        for (PidChanNum=0; PidChanNum < BXPT_P_MAX_PID_CHANNELS; PidChanNum++) {
            RegAddr = BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            Reg = BREG_Read32( Context->hReg, RegAddr );
            /* PR 50099: The dummy read isn't needed if we're reading anywhere in CXMEM_LO. */
            Reg &= tempReg;

            if (Reg)
                ExitCode = BXPT_Rave_RemovePidChannel( Context, PidChanNum );
        }
    }
    else
    {
        PipeShift = ( Context->Index - CXMEM_LO_MAX_CONTEXT ) * 2;
        tempReg = 0x03 << PipeShift;
        for (PidChanNum=0; PidChanNum < BXPT_P_MAX_PID_CHANNELS; PidChanNum++) {
            RegAddr = BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            BKNI_EnterCriticalSection();
            Reg = BREG_Read32( Context->hReg, RegAddr );
            DummyRead = BREG_Read32( Context->hReg, BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE );   /* PR 50099 */
            BKNI_LeaveCriticalSection();
            Reg &= tempReg;

            if (Reg)
                ExitCode = BXPT_Rave_RemovePidChannel( Context, PidChanNum );
        }
    }

    return (ExitCode);
}

BERR_Code BXPT_Rave_RemovePidChannel(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    unsigned int PidChanNum         /* [in] Which PID channel to remove. */
    )
{
    uint32_t Reg, RegAddr, RegAddr1, PipeShift, DummyRead;
    BXPT_Handle lhXpt;
    BXPT_Rave_Handle lhRave;
    uint32_t FieldData;
    uint32_t LoCxMemEnables, HiCxMemEnables;
    uint32_t ScdInUse;
    uint32_t PipeInUse;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Context, bxpt_t);

    /* Sanity check on the arguments. */
    if( PidChanNum >= BXPT_P_MAX_PID_CHANNELS )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChanNum %lu is out of range!", ( unsigned long ) PidChanNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        BDBG_MSG(( "Removing PID channel %u from RAVE context %u", PidChanNum, Context->Index ));

        /* AV context needs this information for filtering as well. */
        if( Context->Index < CXMEM_LO_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = Context->Index * 2;
        }
        else
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
            PipeShift = ( Context->Index - CXMEM_LO_MAX_CONTEXT ) * 2;
        }

        /* Clear the PID VALID bit in SCD */
        Reg = BREG_Read32( Context->hReg, Context->BaseAddr + REC_SCD_PIDS_AB_OFFSET );

#if BXPT_REC_SCD_MAPPING
        FieldData = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDA );
    ScdInUse = BCHP_GET_FIELD_DATA(  Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID );
#elif BXPT_7440B0_SCD_MAPPING
        FieldData = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_SCD_PIDS_AB, SCD_PIDA );
        ScdInUse = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_SCD_PIDS_AB, PIDA_VALID );
#else
        FieldData = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_PIDA );
        ScdInUse = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDA_VALID );
#endif

        if ( FieldData == PidChanNum && ScdInUse ) {
#if BXPT_REC_SCD_MAPPING
            Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID ));
            Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID, 0 ));
#elif BXPT_7440B0_SCD_MAPPING
            Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, PIDA_VALID ));
            Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_SCD_PIDS_AB, PIDA_VALID, 0 ));
#else
            Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDA_VALID ));
            Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDA_VALID, 0 ));
#endif
            BREG_Write32( Context->hReg, Context->BaseAddr + REC_SCD_PIDS_AB_OFFSET, Reg );
        }
        else
        {
#if BXPT_REC_SCD_MAPPING
            FieldData = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDB );
        ScdInUse = BCHP_GET_FIELD_DATA(  Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDB_VALID );
#elif BXPT_7440B0_SCD_MAPPING
            FieldData = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_SCD_PIDS_AB, SCD_PIDB );
            ScdInUse = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_SCD_PIDS_AB, PIDB_VALID );
#else
            FieldData = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_PIDB );
            ScdInUse = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDB_VALID );
#endif
            if ( FieldData == PidChanNum && ScdInUse ) {
                int scdNum;
#if BXPT_REC_SCD_MAPPING
                Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDB_VALID ));
                Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDB_VALID, 0 ));
                scdNum = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMB );
#elif BXPT_7440B0_SCD_MAPPING
                Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, PIDB_VALID ));
                Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_SCD_PIDS_AB, PIDB_VALID, 0 ));
                scdNum = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_SCD_PIDS_AB, SCD_NUMB );
#else
                Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDB_VALID ));
                Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDB_VALID, 0 ));
                scdNum = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMB );
#endif
                BREG_Write32( Context->hReg, Context->BaseAddr + REC_SCD_PIDS_AB_OFFSET, Reg );
                lhRave = ( BXPT_Rave_Handle ) Context->vhRave;
                lhRave->ScdTable[ scdNum ].Allocated = false;
            }
        }

        BKNI_EnterCriticalSection();

        /*
        ** Each PID channel entry in the CXMEM array has two bits for each context:
        ** One bit enables the R pipe, and the other enables the G pipe. When removing
        ** this PID channel, we kill off both pipes.
        */
        Reg = BREG_Read32( Context->hReg, RegAddr );
        DummyRead = BREG_Read32( Context->hReg, BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE );   /* PR 50099 */
        PipeInUse = (Reg >> PipeShift) & 0x03;
        Reg &= ~( 0x03 << PipeShift );
        BREG_Write32( Context->hReg, RegAddr, Reg );

        /*
        ** Turn off the feed from the SPID table when no other contexts are using
        ** this PID channel.
        */
        RegAddr1 = BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
        LoCxMemEnables = BREG_Read32( Context->hReg, RegAddr1 );
        RegAddr1 = BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE + ( PidChanNum * CXMEM_CHNL_STEPSIZE );
        HiCxMemEnables = BREG_Read32( Context->hReg, RegAddr1 );
        DummyRead = BREG_Read32( Context->hReg, BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE );   /* PR 50099 */

        BKNI_LeaveCriticalSection();

        lhRave = ( BXPT_Rave_Handle ) Context->vhRave;
        lhXpt = ( BXPT_Handle ) lhRave->lvXpt;
     /*   if( !BXPT_P_PcrOffset_IsPidChannelInUse( lhXpt, PidChanNum ) ) */
        {
            #define R_MAP   0x55555555UL
            #define G_MAP   0xaaaaaaaaUL

            /* Nobody is using this PID channel, so clear the PID channels enable bit. */
            if( PipeInUse == 0x01 )
            {
                if ( !(LoCxMemEnables & R_MAP) && !(HiCxMemEnables & R_MAP) )
                {
                    BXPT_P_SetPidChannelDestination( lhXpt, PidChanNum, 5, false );
                }
            }
            else
            {
                if( !(LoCxMemEnables & G_MAP) && !(HiCxMemEnables & G_MAP) )
                {
                    BXPT_P_SetPidChannelDestination( lhXpt, PidChanNum, 4, false );
                }
            }
        }
    }

    return( ExitCode );
}

BERR_Code BXPT_Rave_SetTimestampUserBits(
    BXPT_RaveCx_Handle Ctx,         /* [in] The record context  */
    unsigned int Bits                   /* [in] The new value for the user bits. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Ctx, bxpt_t);

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

#if BXPT_HAS_XCBUF && BXPT_HAS_RSBUF
BERR_Code BXPT_Rave_SetRSXCDataRate(
    BXPT_Rave_Handle hRave,     /* [in] Handle for Rave */
    BXPT_ParserType ParserType, /* [in] Input band or playback */
    unsigned BandNum,           /* [in] Which Input band or playback parser to configure */
    unsigned long Rate,         /* [in] Max rate in bps */
    unsigned PacketLen          /* [in] size of mpeg packet */
    )
{
    uint32_t NewBO;
    BXPT_XcBuf_Id Id;
    BXPT_Handle hXpt;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRave, bxpt_t);

    hXpt = ( BXPT_Handle ) hRave->lvXpt;

    switch ( ParserType )
    {
    case BXPT_ParserType_eIb:
        Id = BXPT_XcBuf_Id_RAVE_IBP0;

        NewBO = BXPT_P_RsBuf_ComputeBlockOut( Rate, PacketLen );
        ExitCode = BXPT_P_RsBuf_SetBlockout( hXpt, BandNum, NewBO );
        hXpt->RsBufBO[ BandNum ] = NewBO;

        NewBO = BXPT_P_XcBuf_ComputeBlockOut( Rate, PacketLen );
        ExitCode = BXPT_P_XcBuf_SetBlockout( hXpt, Id + BandNum, NewBO );
        hXpt->XcBufBO[ Id + BandNum ] = NewBO;

        break;
    case BXPT_ParserType_ePb:
        Id = BXPT_XcBuf_Id_RAVE_PBP0;

        NewBO = BXPT_P_RsBuf_ComputeBlockOut( Rate, 188 );
        ExitCode = BXPT_P_RsBuf_PlaybackSetBlockout(hXpt, BandNum, NewBO );

        NewBO = BXPT_P_XcBuf_ComputeBlockOut( Rate, PacketLen );
        ExitCode = BXPT_P_XcBuf_SetBlockout( hXpt, Id + BandNum, NewBO );
        hXpt->XcBufBO[ Id + BandNum ] = NewBO;

        break;
    default:
        BDBG_MSG(("DataRate request for unrecognized ParserType"));
    }

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
    uint32_t RegAddr;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(ThisCtx, bxpt_t);

    RegAddr = BCHP_XPT_RAVE_INT_CX0 + ( ThisCtx->Index * 4 );
    *IntId = BCHP_INT_ID_CREATE( RegAddr, Name );

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

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);

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
        BREG_Write32( hCtx->hReg, SpliceQueueDmemAddr + SPLICE_CURR_PID_OFFSET, PidChannel );
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
    BREG_Write32( hCtx->hReg, SpliceQueueDmemAddr + ( WrPtr * 4 ) + SPLICE_QUEUE_LOC_0_OFFSET, SplicePidChannel );
    WrPtr++;

    /* ... update write pointer. Wrap handled above. */
    BREG_Write32( hCtx->hReg, SpliceQueueDmemAddr + SPLICE_WR_PTR_OFFSET, WrPtr );

    /* ... enable splicing in context. */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_PID_STREAM_ID_OFFSET );
    Reg |= BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_PID_STREAM_ID, SPLICE_EN, 1 );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_PID_STREAM_ID_OFFSET, Reg );

    lhRave = ( BXPT_Rave_Handle ) hCtx->vhRave;
    lhXpt = ( BXPT_Handle ) lhRave->lvXpt;
    BXPT_P_SetPidChannelDestination( lhXpt, SplicePidChannel, 5, true );
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

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);

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

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);

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

BERR_Code BXPT_Rave_GetContextStatus(
    BXPT_RaveCx_Handle hCtx,
    BXPT_RaveCx_Status *CxStatus
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
    BDBG_ASSERT( CxStatus );

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG1_OFFSET );
    CxStatus->ContextEnabled = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1, CONTEXT_ENABLE ) ? true : false;

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_DEPTH_OFFSET );
    CxStatus->CdbOverflow = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_CDB_DEPTH, CDB_OVERFLOW ) ? true : false;

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + ITB_DEPTH_OFFSET );
    CxStatus->ItbOverflow = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_ITB_DEPTH, ITB_OVERFLOW ) ? true : false;

    return( ExitCode );
}

BERR_Code BXPT_Rave_FlushContext(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
    BDBG_MSG(( "Flushing context %u", hCtx->Index ));

    if( hCtx->IsSoftContext )
    {
        hCtx->SoftRave.last_src_itb_valid = hCtx->SoftRave.src_itb_base;
        hCtx->SoftRave.last_dst_itb_valid = hCtx->SoftRave.dest_itb_base;
        hCtx->SoftRave.last_dest_valid = hCtx->SoftRave.last_dst_itb_valid;
    /* hCtx->SoftRave.last_base_address = hCtx->SoftRave.src_itb_base; */
    hCtx->SoftRave.last_base_address = BREG_Read32(hCtx->hReg, CDB_BASE_PTR_OFFSET+hCtx->SoftRave.SrcBaseAddr);

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

#ifdef SW7335_895_WORKAROUND
    /* Use the PID channel version checker to flush the channel. */
    {
        unsigned DummyRead = 0;
        unsigned RegAddr = 0;
        unsigned PipeShift = 0;
        unsigned PidChannel = 0;

        if( hCtx->Index < CXMEM_LO_MAX_CONTEXT )
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE;
            PipeShift = hCtx->Index * 2;
        }
        else
        {
            RegAddr = BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE;
            PipeShift = ( hCtx->Index - CXMEM_LO_MAX_CONTEXT ) * 2;
        }

        BKNI_EnterCriticalSection();
        for( PidChannel = 0; PidChannel < BXPT_P_MAX_PID_CHANNELS; PidChannel++, RegAddr += CXMEM_CHNL_STEPSIZE )
        {
            unsigned Reg;

            Reg = BREG_Read32( hCtx->hReg, RegAddr );
            DummyRead = BREG_Read32( hCtx->hReg, BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE );   /* PR 50099 */
            if( Reg & (0x3 << PipeShift) )
            {
                unsigned PidChannelCfg = BREG_Read32( hCtx->hReg, BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannel * 4 ));
                unsigned IgnorePidVersion = PidChannelCfg & ~( 1 << 28 );   /* Clear the IGNORE_PID_VERSION flag */

#ifdef ENABLE_PLAYBACK_MUX
                {
                    unsigned FeSel = BCHP_GET_FIELD_DATA( PidChannelCfg, XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL );
                    unsigned Band = BCHP_GET_FIELD_DATA( PidChannelCfg, XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT );
                    BXPT_Rave_Handle hRave = (BXPT_Rave_Handle) hCtx->vhRave;
                    BXPT_Handle hXpt = (BXPT_Handle)hRave->lvXpt;

                    /* Only change PID channels that are NOT part of the playback mux */
                    if (Band < BXPT_P_MAX_PLAYBACKS - hXpt->NumPlaybackMuxes && FeSel)
                    {
                        BREG_Write32( hCtx->hReg, BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannel * 4 ), IgnorePidVersion );    /* Once to clear the flag */
                        BREG_Write32( hCtx->hReg, BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannel * 4 ), IgnorePidVersion );    /* Again to be sure the version incremented */
                    }
                }
#else
                /* If playback mux isn't used, do it. */
                BREG_Write32( hCtx->hReg, BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannel * 4 ), IgnorePidVersion );    /* Once to clear the flag */
                BREG_Write32( hCtx->hReg, BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannel * 4 ), IgnorePidVersion );    /* Again to be sure the version incremented */
#endif
            }
        }
        BKNI_LeaveCriticalSection();
        BKNI_Delay( 100 );    /* Need to wait 50uS to flush RAVE's internal FIFO. Play safe and double that. */
    }

#endif /*  SW7335_895_WORKAROUND */

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

    /* Repeat for each SCD??? */
    /* DmemBase = SCD_DMEM_BASE + ( hCtx->Index * SCD_DMEM_SIZE );
    for( ii = 0; ii < SCD_DMEM_SIZE; ii++ )
        BREG_Write32( hCtx->hReg, BCHP_XPT_RAVE_DMEMi_ARRAY_START + DmemBase + ( ii * 4 ), 0 );*/

    /* SW7335-1434: Clear watermarking status during channel change */
    {
        uint32_t RegAddr;

    /* ToDo: Paste into core40nm and core28nm */

        #if (BCHP_CHIP == 7400) || (BCHP_CHIP == 7405)    /* Supported chips before the 7325 */
            RegAddr = hCtx->BaseAddr + AV_MISC_CFG4_OFFSET;
            Reg = BREG_Read32( hCtx->hReg, RegAddr );
            Reg &= ~( 1 << 29 );
            BREG_Write32( hCtx->hReg, RegAddr, Reg );
        #elif (BCHP_CHIP == 7401) || (BCHP_CHIP == 7118)    /* Two special cases */
            #ifdef BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG5
                RegAddr = hCtx->BaseAddr + BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG5 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR;
            #else
                RegAddr = hCtx->BaseAddr + BCHP_XPT_RAVE_CX0_AV_RESERVE_CFG4 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR;
            #endif
            Reg = BREG_Read32( hCtx->hReg, RegAddr );
            Reg &= ~( 1 << 13 );
            BREG_Write32( hCtx->hReg, RegAddr, Reg );
        #else   /* 7325 and all subsequent chips */
            #ifdef BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG5
                RegAddr = hCtx->BaseAddr + BCHP_XPT_RAVE_CX0_AV_MISC_CONFIG5 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR;
            #else
                RegAddr = hCtx->BaseAddr + BCHP_XPT_RAVE_CX0_RAVE_Reg_0 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR;
            #endif
            Reg = BREG_Read32( hCtx->hReg, RegAddr );
            Reg &= ~( 1 << 13 );
            BREG_Write32( hCtx->hReg, RegAddr, Reg );
        #endif
    }

    Done:
    return( ExitCode );
}


BERR_Code BXPT_Rave_GetPictureCount(
    BXPT_RaveCx_Handle hCtx,
    unsigned *PictureCount
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
    BDBG_ASSERT( PictureCount );

#if BXPT_PER_CONTEXT_PIC_COUNTER
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + PICTURE_CTR_OFFSET );
    *PictureCount = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_PIC_CTR, VALUE );
#else
    if( hCtx->PicCounter == -1 )
    {
        BDBG_ERR(( "No picture counter allocated to this context!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        *PictureCount = 0;
    }
    else
    {
        uint32_t RegAddr = BCHP_XPT_RAVE_PIC_CTR0 + ( 4 * hCtx->PicCounter );

        Reg = BREG_Read32( hCtx->hReg, RegAddr );
        *PictureCount = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_PIC_CTR0, VALUE );
    }
#endif

    return ExitCode;
}


static BERR_Code InitContext(
    BXPT_RaveCx_Handle ThisCtx,
    const BAVC_CdbItbConfig *BufferCfg
    )
{
    void *BufferAddr, *Cdb;
    uint32_t BufferOffset, Reg;
    uint32_t Offset;

    BMEM_Handle hMem = ThisCtx->hMem;
    BERR_Code ExitCode = BERR_SUCCESS;
    uint32_t BlockSize = RAVE_CONTEXT_REG_STEP;
    unsigned ItbSize = 0;
    unsigned ItbAlignment = 7;
    unsigned CdbAlignment = 8;

    if( BufferCfg )
    {
        ItbAlignment = BufferCfg->Itb.Alignment;
        CdbAlignment = BufferCfg->Cdb.Alignment;
    }

    /* Clear registers / pointer memory */
    for( Offset = 0; Offset < BlockSize; Offset += 4 )
        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + Offset, 0 );

#if BXPT_HAS_STARTCODE_BUFFER_WORKAROUND
    /*
    ** Workaround for SW7405-4105: RAVE hw doesn't have enough room to store more than 10 startcodes per packet. If you have a TS packet
    ** with more than that, the additional startcodes will be ignored. To workaround this, you'll also need the updated bcmindexer
    ** (and related files) and RAVE ihex for this workaround. See the changeset in the JIRA.
    */
    /* DME: enable full TS packet capture using reserved register */
    Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + BCHP_XPT_RAVE_CX0_RAVE_Reg_0-BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR);
    Reg |= 1<<9;
    BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + BCHP_XPT_RAVE_CX0_RAVE_Reg_0-BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR, Reg );
#endif

    /* Set the context type (AV or REC) */
    Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + REC_MISC_CFG_OFFSET );
#if (BCHP_CHIP == 7440 && BCHP_VER >= BCHP_VER_B0 )
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_MISC_CONFIG, REC_AVN ) );
#else
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, REC_AVN ) );
#endif
    Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_MISC_CONFIG, REC_AVN,
        ThisCtx->Type == BXPT_RaveCx_eRecord || ThisCtx->Type == BXPT_RaveCx_eRecordR ? 1 : 0 ) );
    BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

    /* Watermarking is enabled in fw by default, on all CAT2A parts. For now. it's needed only on 7468, so disable for all others.
    See SW7335-1393
    */
#if ( BCHP_CHIP == 7550 ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 ) || ( BCHP_CHIP == 7420 )
    Reg = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + BCHP_XPT_RAVE_CX0_RAVE_Reg_0 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR);
    Reg |= 1 << 11;     /* Bitfield is DISABLE_ DROP_DATA_ON_PUSI_ERROR. No RDB for this yet.*/
    BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + BCHP_XPT_RAVE_CX0_RAVE_Reg_0 - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR, Reg );
#endif

    if( BufferCfg )
    {
        bool ItbEndian, CdbEndian, SysIsBigEndian;

#if ( BCHP_CHIP == 7400 ) && ( BCHP_VER == BCHP_VER_A0 )
        /* 7400A0 XPT was always in Big endian mode, regardless of chip endianxess */
        ItbEndian = BufferCfg->Itb.LittleEndian;
        CdbEndian = BufferCfg->Cdb.LittleEndian;
        BSTD_UNUSED( SysIsBigEndian );
#else
        /* All other parts follow the chip endiannes */

#if ( BCHP_CHIP == 3563 ) || ( BCHP_CHIP == 7400 && BCHP_VER >= BCHP_VER_B0 ) || ( BCHP_CHIP == 7405 ) || ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) \
    || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || ( BCHP_CHIP == 7405 && BCHP_VER >= BCHP_VER_B0 ) || (BCHP_CHIP == 7420 ) || ( BCHP_CHIP == 7635 ) || (BCHP_CHIP == 7002) \
    || ( BCHP_CHIP == 7550 ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 ) || ( BCHP_CHIP == 7630 ) || ( BCHP_CHIP == 7408 ) || ( BCHP_CHIP == 7468 )
        /* Although some insist on changing the register name... */
        Reg = BREG_Read32( ThisCtx->hReg, BCHP_SUN_TOP_CTRL_STRAP_VALUE_0 );
        SysIsBigEndian = BCHP_GET_FIELD_DATA( Reg, SUN_TOP_CTRL_STRAP_VALUE_0, strap_system_big_endian );
#elif (BCHP_CHIP == 35130) || (BCHP_CHIP == 35230) || (BCHP_CHIP == 35125) || (BCHP_CHIP == 35233)
        Reg = BREG_Read32( ThisCtx->hReg, BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_0 );
        if (BCHP_GET_FIELD_DATA( Reg, SUN_TOP_CTRL_OTP_OPTION_STATUS_0, otp_option_strap_override ))
        {
            if( BCHP_GET_FIELD_DATA( Reg, SUN_TOP_CTRL_OTP_OPTION_STATUS_0, otp_option_strap_system_big_endian_override ))
                SysIsBigEndian = true;       /* */
            else
                SysIsBigEndian = false;       /* */
        }
        else
        {
            uint32_t StrapReg = BREG_Read32( ThisCtx->hReg, BCHP_TVM_STRAP_VALUE );
            SysIsBigEndian = BCHP_GET_FIELD_DATA( StrapReg, TVM_STRAP_VALUE, STRAP_SYSTEM_BIG_ENDIAN );
        }
#else
        Reg = BREG_Read32( ThisCtx->hReg, BCHP_SUN_TOP_CTRL_STRAP_VALUE );
        SysIsBigEndian = BCHP_GET_FIELD_DATA( Reg, SUN_TOP_CTRL_STRAP_VALUE, strap_system_big_endian );
#endif

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

        /* Config the ITB - this is not a swap setting. It simply sets the endianness. */
        ItbEndian = BufferCfg->Itb.LittleEndian;
#endif

        if( ThisCtx->CdbBufferAddr==NULL ) /* XPT does the alloc */
        {
            ThisCtx->externalCdbAlloc = false;

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
                if( !ThisCtx->hRHeap )
                {
                    BDBG_ERR(( "Decode or record from secure memory is requested, but no secure heap is available." ));
                    ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
                    goto done;
                }
                Cdb = BufferAddr = BMEM_AllocAligned( ThisCtx->hRHeap, BufferCfg->Cdb.Length, CdbAlignment, 0 );
                break;

                case BXPT_RaveCx_eAv:
                /* Decode from non-secure memory. Use the general heap, but warn them if a secure heap is defined. */
                Cdb = BufferAddr = BMEM_AllocAligned( hMem, BufferCfg->Cdb.Length, CdbAlignment, 0 );
                break;

                case BXPT_RaveCx_eRecord:
                default:
                /* Plain old records, and everything else, comes from the general heap. */
                Cdb = BufferAddr = BMEM_AllocAligned( hMem, BufferCfg->Cdb.Length, CdbAlignment, 0 );
                break;
            }

            if( BufferAddr == NULL )
            {
                ThisCtx->allocatedCdbBufferSize = 0;
                ThisCtx->usedCdbBufferSize = 0;
                BDBG_ERR(( "CDB alloc failed!" ));
                ExitCode = BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
                goto done;
            }
            else
            {
                /* save the initial buffer allocated */
                ThisCtx->allocatedCdbBufferSize = BufferCfg->Cdb.Length;
                ThisCtx->usedCdbBufferSize = BufferCfg->Cdb.Length;
                ThisCtx->CdbBufferAddr=BufferAddr;    /* save the cdb allocated bufffer adddress */
            }
        }
        else /* the app has already done the alloc */
        {
            ThisCtx->externalCdbAlloc = true;

            /* check alignment of buffer pointer */
            if( ThisCtx->Type == BXPT_RaveCx_eAv || ThisCtx->Type == BXPT_RaveCx_eAvR )
            {
                if( ((uint32_t)ThisCtx->CdbBufferAddr)%256!=0 )
                {
                    BDBG_ERR(( "CDB buffer pointer is not 256-Byte aligned "));
                    ThisCtx->CdbBufferAddr = NULL;
                    ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
                    goto done;
                }
            }

            ThisCtx->allocatedCdbBufferSize = BufferCfg->Cdb.Length;
            ThisCtx->usedCdbBufferSize = BufferCfg->Cdb.Length;
            Cdb = NULL;
            BufferAddr = ThisCtx->CdbBufferAddr;
            /*BDBG_MSG(( "InitContext: using pre-allocated CDB %#lx", ThisCtx->CdbBufferAddr ));*/
        }

        if( ThisCtx->Type == BXPT_RaveCx_eAvR && ThisCtx->hRHeap )
            BMEM_ConvertAddressToOffset( ThisCtx->hRHeap, BufferAddr, &BufferOffset );
        else
            BMEM_ConvertAddressToOffset( hMem, BufferAddr, &BufferOffset );

        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_WRITE_PTR_OFFSET, BufferOffset );
        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_READ_PTR_OFFSET, BufferOffset );
        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_BASE_PTR_OFFSET, BufferOffset );
        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_END_PTR_OFFSET, BufferOffset + BufferCfg->Cdb.Length - 1 );
        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_VALID_PTR_OFFSET, BufferOffset );
        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_WRAP_PTR_OFFSET, 0 );

        /* In some cases, the user does not want an ITB. */
        if( BufferCfg->Itb.Length )
        {
            if( ThisCtx->ItbBufferAddr==NULL ) /* XPT does the alloc */
            {
                ThisCtx->externalItbAlloc = false;

                /* SW7420-1459: Insure that the ITB size is a multiple of 16 for AV contexts. */
                if( (ThisCtx->Type == BXPT_RaveCx_eAv || ThisCtx->Type == BXPT_RaveCx_eAvR) && BufferCfg->Itb.Length % BXPT_ITB_SIZE )
                {
                    ItbSize = BufferCfg->Itb.Length - (BufferCfg->Itb.Length % BXPT_ITB_SIZE);
                    BDBG_WRN(( "Invalid ITB size, must be a multiple of %u. Truncating to %u", BXPT_ITB_SIZE, ItbSize ));
                }
                else
                {
                    ItbSize = BufferCfg->Itb.Length;
                }

                /* Create the ITB. ITB is always allocated from the G area. */
                BufferAddr = BMEM_AllocAligned( hMem, ItbSize, ItbAlignment, 0 );
                if( BufferAddr == NULL )
                {
                    BDBG_ERR(( "ITB alloc failed!" ));
                    if ( Cdb )
                    {
                        BMEM_Free( hMem, Cdb );     /* Give back the CDB, since we're bailing out. */
                        ThisCtx->CdbBufferAddr = 0;
                    }
                    ExitCode = BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
                    goto done;
                }
                else
                {
                    ThisCtx->ItbBufferAddr = BufferAddr; /* save the itb allocated buffer address allocated */
                }
            }
            else /* the app has already done the alloc */
            {
                ThisCtx->externalItbAlloc = true;

                /* check alignment of buffer pointer */
                if( ThisCtx->Type == BXPT_RaveCx_eAv || ThisCtx->Type == BXPT_RaveCx_eAvR )
                {
                    if( ((uint32_t)ThisCtx->ItbBufferAddr)%128!=0 )
                    {
                        BDBG_ERR(( "ITB buffer pointer is not 128-Byte aligned "));
                        ThisCtx->ItbBufferAddr = NULL;
                        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
                        goto done;
                    }
                }
                if( (ThisCtx->Type == BXPT_RaveCx_eAv || ThisCtx->Type == BXPT_RaveCx_eAvR) && BufferCfg->Itb.Length % BXPT_ITB_SIZE )
                {
                    BDBG_ERR(( "Invalid ITB size, must be a multiple of %u", BXPT_ITB_SIZE ));
                    ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
                    goto done;
                }

                BufferAddr = ThisCtx->ItbBufferAddr;
                ItbSize = BufferCfg->Itb.Length;
                /* BDBG_MSG(( "InitContext: using pre-allocated ITB %#lx", ThisCtx->ItbBufferAddr )); */
            }

            BMEM_ConvertAddressToOffset( hMem, BufferAddr, &BufferOffset );
            BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_WRITE_PTR_OFFSET, BufferOffset );
            BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_READ_PTR_OFFSET, BufferOffset );
            BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_BASE_PTR_OFFSET, BufferOffset );
            BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_END_PTR_OFFSET, BufferOffset + ItbSize - 1 );
            BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_VALID_PTR_OFFSET, BufferOffset );
            BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_WRAP_PTR_OFFSET, 0 );
        }

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

    if( ThisCtx->Type == BXPT_RaveCx_eAv || ThisCtx->Type == BXPT_RaveCx_eAvR || BXPT_RaveCx_eRecord || BXPT_RaveCx_eRecordR )
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

    done:
    return( ExitCode );
}

static BERR_Code ClearMem(
    BXPT_Rave_Handle hRave
    )
{
    uint32_t Offset;

    BERR_Code ExitCode = BERR_SUCCESS;

    /* Clear context memory */
    for( Offset = BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_START; Offset <= BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_END; Offset++ )
        BREG_Write32( hRave->hReg, BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE + ( Offset * 4 ), 0 );
    for( Offset = BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_START; Offset <= BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_END; Offset++ )
        BREG_Write32( hRave->hReg, BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE + ( Offset * 4 ), 0 );

    /* Clear data memory */
    for( Offset = BCHP_XPT_RAVE_DMEMi_ARRAY_START; Offset < BCHP_XPT_RAVE_DMEMi_ARRAY_END; Offset++ )
        BREG_Write32( hRave->hReg, BCHP_XPT_RAVE_DMEMi_ARRAY_BASE + ( Offset * 4 ), 0 );

    return( ExitCode );
}

static BERR_Code RaveRamInit(
    BXPT_Rave_Handle hRave
    )
{
    uint32_t Offset;

    BERR_Code ExitCode = BERR_SUCCESS;

#if BXPT_HAS_RAVE_AUTO_READ
    /*
    ** NOTE: When this option is enabled, bxpt.inc will redirect the build to use the AutoRead ihex image file.
    ** Therefore, no changes are needed in the function.
    */
    BDBG_WRN(( "Using AutoRead ihex" ));
#endif

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

static BERR_Code InitScd(
    StartCodeIndexer *lhScd
    )
{
    uint32_t Offset, Reg, CDMemBase;

    BERR_Code ExitCode = BERR_SUCCESS;

    for( Offset = 0; Offset < SCD_REG_STEP; Offset += 4 )
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

#if BXPT_NEXT_GEN_COMPARATORS
    /* Not used in the newer arch. */
#else
    /* Use the comparator config loaded into the AV context (which will be set later). */
    Reg = BREG_Read32( lhScd->hReg, lhScd->BaseAddr + SCD_CTRL1_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_SCD0_SCD_CTRL1, LOAD_COMP_CONFIG )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_SCD0_SCD_CTRL1, LOAD_COMP_CONFIG, 1 )
    );
    BREG_Write32( lhScd->hReg, lhScd->BaseAddr + SCD_CTRL1_OFFSET, Reg );
#endif

    return( ExitCode );
}

static BERR_Code InitTpit(
    TpitIndexer *hTpit
    )
{
    uint32_t Offset;

    BERR_Code ExitCode = BERR_SUCCESS;

    for( Offset = 0; Offset < TPIT_PID_TABLE_SIZE; Offset += 4 )
        BREG_Write32( hTpit->hReg, hTpit->PidTableBaseAddr + Offset, 0 );

    for( Offset = 0; Offset < TPIT_PARSE_TABLE_SIZE; Offset += 4 )
        BREG_Write32( hTpit->hReg, hTpit->ParseTableBaseAddr + Offset, 0 );

    for( Offset = 0; Offset < TPIT_CTRL_REG_STEP; Offset += 4 )
        BREG_Write32( hTpit->hReg, hTpit->BaseAddr + Offset, 0 );

    return( ExitCode );
}

static BERR_Code LoadScRanges(
    BXPT_RaveCx_Handle Context,         /* [in] The context. */
    const BXPT_Rave_EsRanges *EsRanges
    )
{
#if BXPT_NEXT_GEN_COMPARATORS

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
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_D ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_D ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_C ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_C ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_B ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_B ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_A ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_A )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_D, EsRanges[ 3 ].Enable ? ( EsRanges[ 3 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_D, 3 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_C, EsRanges[ 2 ].Enable ? ( EsRanges[ 2 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_C, 3 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_B, EsRanges[ 1 ].Enable ? ( EsRanges[ 1 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_B, 3 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_A, EsRanges[ 0 ].Enable ? ( EsRanges[ 0 ].RangeIsASlice == true ? 3 : 2 ) : 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_A, 3 )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET, Reg );
#else
    uint32_t RangeABReg, RangeCDReg, Cfg2Reg;
    BERR_Code ExitCode = BERR_SUCCESS;

    RangeABReg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_VID_SC_RANGE_AB_OFFSET );
    RangeCDReg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_VID_SC_RANGE_CD_OFFSET );

    if( EsRanges[ 0 ].Enable == true )
    {
        RangeABReg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEA_HI ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEA_LO )
        );
        RangeABReg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEA_HI, EsRanges[ 0 ].RangeHi ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEA_LO, EsRanges[ 0 ].RangeLo )
        );
    }

    if( EsRanges[ 1 ].Enable == true )
    {
        RangeABReg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEB_HI ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEB_LO )
        );
        RangeABReg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEB_HI, EsRanges[ 1 ].RangeHi ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEB_LO, EsRanges[ 1 ].RangeLo )
        );
    }

    if( EsRanges[ 2 ].Enable == true )
    {
        RangeCDReg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGEC_HI ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGEC_LO )
        );
        RangeCDReg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGEC_HI, EsRanges[ 2 ].RangeHi ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGEC_LO, EsRanges[ 2 ].RangeLo )
        );
    }

    if( EsRanges[ 3 ].Enable == true )
    {
        RangeCDReg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGED_HI ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGED_LO )
        );
        RangeCDReg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGED_HI, EsRanges[ 3 ].RangeHi ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGED_LO, EsRanges[ 3 ].RangeLo )
        );
    }

    BREG_Write32( Context->hReg, Context->BaseAddr + AV_VID_SC_RANGE_AB_OFFSET, RangeABReg );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_VID_SC_RANGE_CD_OFFSET, RangeCDReg );

    Cfg2Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG2_OFFSET );
    Cfg2Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGED_SLICE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEC_SLICE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEB_SLICE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEA_SLICE ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGED_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEC_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEB_EN ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEA_EN )
    );
    Cfg2Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGED_SLICE, EsRanges[ 3 ].RangeIsASlice == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEC_SLICE, EsRanges[ 2 ].RangeIsASlice == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEB_SLICE, EsRanges[ 1 ].RangeIsASlice == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEA_SLICE, EsRanges[ 0 ].RangeIsASlice == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGED_EN, EsRanges[ 3 ].Enable == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEC_EN, EsRanges[ 2 ].Enable == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEB_EN, EsRanges[ 1 ].Enable == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEA_EN, EsRanges[ 0 ].Enable == true ? 1 : 0 )
    );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_MISC_CFG2_OFFSET, Cfg2Reg );
#endif
    return( ExitCode );
}

static BERR_Code GetScRanges(
    BXPT_RaveCx_Handle Context,         /* [in] The context. */
    BXPT_Rave_EsRanges *EsRanges
    )
{
#if BXPT_NEXT_GEN_COMPARATORS

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
    Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_D );
    GetScEnables( EsRanges + 3, Mode );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
    Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_C );
    GetScEnables( EsRanges + 2, Mode );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
    Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_B );
    GetScEnables( EsRanges + 1, Mode );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
    Mode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_A );
    GetScEnables( EsRanges, Mode );
#else
    uint32_t RangeABReg, RangeCDReg, Cfg2Reg;
    BERR_Code ExitCode = BERR_SUCCESS;

    BSTD_UNUSED( GetScEnables );

    Cfg2Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG2_OFFSET );
    EsRanges[ 3 ].RangeIsASlice = BCHP_GET_FIELD_DATA( Cfg2Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGED_SLICE ) ? true : false;
    EsRanges[ 2 ].RangeIsASlice = BCHP_GET_FIELD_DATA( Cfg2Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEC_SLICE ) ? true : false;
    EsRanges[ 1 ].RangeIsASlice = BCHP_GET_FIELD_DATA( Cfg2Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEB_SLICE ) ? true : false;
    EsRanges[ 0 ].RangeIsASlice = BCHP_GET_FIELD_DATA( Cfg2Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEA_SLICE ) ? true : false;

    EsRanges[ 3 ].Enable = BCHP_GET_FIELD_DATA( Cfg2Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGED_EN ) ? true : false;
    EsRanges[ 2 ].Enable = BCHP_GET_FIELD_DATA( Cfg2Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEC_EN ) ? true : false;
    EsRanges[ 1 ].Enable = BCHP_GET_FIELD_DATA( Cfg2Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEB_EN ) ? true : false;
    EsRanges[ 0 ].Enable = BCHP_GET_FIELD_DATA( Cfg2Reg, XPT_RAVE_CX0_AV_MISC_CONFIG2, RANGEA_EN ) ? true : false;

    RangeCDReg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_VID_SC_RANGE_AB_OFFSET );
    EsRanges[ 3 ].RangeHi = BCHP_GET_FIELD_DATA( RangeCDReg, XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGED_HI );
    EsRanges[ 3 ].RangeLo = BCHP_GET_FIELD_DATA( RangeCDReg, XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGED_LO );
    EsRanges[ 2 ].RangeHi = BCHP_GET_FIELD_DATA( RangeCDReg, XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGEC_HI );
    EsRanges[ 2 ].RangeLo = BCHP_GET_FIELD_DATA( RangeCDReg, XPT_RAVE_CX0_AV_VID_SC_RANGE_CD, RANGEC_LO );

    RangeABReg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_VID_SC_RANGE_AB_OFFSET );
    EsRanges[ 1 ].RangeHi = BCHP_GET_FIELD_DATA( RangeABReg, XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEB_HI );
    EsRanges[ 1 ].RangeLo = BCHP_GET_FIELD_DATA( RangeABReg, XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEB_LO );
    EsRanges[ 0 ].RangeHi = BCHP_GET_FIELD_DATA( RangeABReg, XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEA_HI );
    EsRanges[ 0 ].RangeLo = BCHP_GET_FIELD_DATA( RangeABReg, XPT_RAVE_CX0_AV_VID_SC_RANGE_AB, RANGEA_LO );
#endif
    return( ExitCode );
}

#ifdef UNIFIED_ITB_SUPPORT
/* PR 29614. These bitfields aren't defined in the RDB yet. */
#define BCHP_XPT_RAVE_CX0_AV_COMP1_CONTROL_DATA_EXTRACT_NUM_BITS6_MASK       0x04000000
#define BCHP_XPT_RAVE_CX0_AV_COMP1_CONTROL_DATA_EXTRACT_NUM_BITS6_SHIFT      26
#define BCHP_XPT_RAVE_CX0_AV_COMP2_CONTROL_DATA_EXTRACT_NUM_BITS6_MASK       0x04000000
#define BCHP_XPT_RAVE_CX0_AV_COMP2_CONTROL_DATA_EXTRACT_NUM_BITS6_SHIFT      26
#endif

static BERR_Code ConfigureComparators(
    BXPT_RaveCx_Handle hCtx,
    BAVC_ItbEsType StreamType
    )
{
#ifdef UNIFIED_ITB_SUPPORT
    uint32_t Reg;

    unsigned Comp1NumCompareBytes = 0;
    unsigned Comp1RepeatByte = 0;
    unsigned Comp1En = 0;
    unsigned Comp1ExtractStartByte = 0;
    unsigned Comp1ExtractStartBit = 0;
    unsigned Comp1ExclVal = 0;
    unsigned Comp1ExclMask = 0;
    unsigned Comp2ExclVal = 0;
    unsigned Comp2ExclMask = 0;
    unsigned Comp1ExtractNumBits = 0;
    unsigned Comp2ExtractNumBits = 0;
    unsigned Comp2ExtractStartBit = 0;
    unsigned Comp2ExtractStartByte = 0;
    unsigned Comp2En = 0;
    unsigned Comp2RepeatByte = 0;
    unsigned Comp2NumCompareBytes = 0;
    unsigned Comp2ValidByteEn = 0;

    unsigned Comp1DataExtractNumBits_6 = 0;
    unsigned Comp2DataExtractNumBits_6 = 0;
    unsigned Comp1PesHdrData = 0;
    unsigned Comp1EsData = 1;

#if BXPT_NEXT_GEN_COMPARATORS

    unsigned Comp1CompareVal0 = 0xFF;
    unsigned Comp1CompareVal1 = 0xFF;
    unsigned Comp1CompareVal2 = 0xFF;
    unsigned Comp1CompareVal3 = 0xFF;
    unsigned Comp1MaskVal0 = 0xFF;
    unsigned Comp1MaskVal1 = 0xFF;
    unsigned Comp1MaskVal2 = 0xFF;
    unsigned Comp1MaskVal3 = 0xFF;

    unsigned Comp1FiltFuncD = 0;
    unsigned Comp1FiltOffsetD = 0;
    unsigned Comp1FiltFuncC = 0;
    unsigned Comp1FiltOffsetC = 0;
    unsigned Comp1FiltFuncB = 0;
    unsigned Comp1FiltOffsetB = 0;
    unsigned Comp1FiltFuncA = 0;
    unsigned Comp1FiltOffsetA = 0;

    unsigned Comp2CompareVal0 = 0xFF;
    unsigned Comp2CompareVal1 = 0xFF;
    unsigned Comp2CompareVal2 = 0xFF;
    unsigned Comp2CompareVal3 = 0xFF;
    unsigned Comp2MaskVal0 = 0xFF;
    unsigned Comp2MaskVal1 = 0xFF;
    unsigned Comp2MaskVal2 = 0xFF;
    unsigned Comp2MaskVal3 = 0xFF;

    unsigned Comp2FiltFuncD = 0;
    unsigned Comp2FiltOffsetD = 0;
    unsigned Comp2FiltFuncC = 0;
    unsigned Comp2FiltOffsetC = 0;
    unsigned Comp2FiltFuncB = 0;
    unsigned Comp2FiltOffsetB = 0;
    unsigned Comp2FiltFuncA = 0;
    unsigned Comp2FiltOffsetA = 0;
#endif

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

#if BXPT_NEXT_GEN_COMPARATORS
        Comp1RepeatByte = 4;
        Comp1CompareVal0 = 0;
        Comp1CompareVal1 = 0;
        Comp1CompareVal2 = 0x01;
        Comp1CompareVal3 = 0;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0xFF;
        Comp1MaskVal3 = 0;

        Comp1FiltFuncD = 0;
        Comp1FiltOffsetD = 0;
        Comp1FiltFuncC = 0;
        Comp1FiltOffsetC = 0;
        Comp1FiltFuncB = 0;
        Comp1FiltOffsetB = 0;
        Comp1FiltFuncA = 3;
        Comp1FiltOffsetA = 3;
#else
        Comp1RepeatByte = 2;
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x00000100 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xffffff00 );
#endif

        Comp2ExtractNumBits = 7;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 1;
        Comp2En = 0;
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 1;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp2FiltFuncD = 0;
        Comp2FiltOffsetD = 0;
        Comp2FiltFuncC = 0;
        Comp2FiltOffsetC = 0;
        Comp2FiltFuncB = 0;
        Comp2FiltOffsetB = 0;
        Comp2FiltFuncA = 0;
        Comp2FiltOffsetA = 0;
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0x00000100 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0xffffff00 );
#endif

        /* For video, comparator exclusion val and mask bits are 0 */
        Comp1ExclVal = 0;
        Comp1ExclMask = 0;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;
        break;

        case BAVC_ItbEsType_eOTFVideo:
#if (BCHP_CHIP == 7401) || (BCHP_CHIP == 7118)
        Comp1ExtractNumBits = 63;
        Comp1RepeatByte = 2;
#else
        Comp1ExtractNumBits = 71;
        Comp1RepeatByte = 4;
#endif
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
        Comp1En = 1;
        Comp1NumCompareBytes = 2;
        Comp1DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp1CompareVal0 = 0;
        Comp1CompareVal1 = 0;
        Comp1CompareVal2 = 0x01;
        Comp1CompareVal3 = 0;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0xFF;
        Comp1MaskVal3 = 0;

        Comp1FiltFuncD = 0;
        Comp1FiltOffsetD = 0;
        Comp1FiltFuncC = 0;
        Comp1FiltOffsetC = 0;
        Comp1FiltFuncB = 0;
        Comp1FiltOffsetB = 0;
        Comp1FiltFuncA = 0;
        Comp1FiltOffsetA = 0;
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x00000100 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xffffff00 );
#endif

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 1;
        Comp2En = 0;
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 1;
        Comp2DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp2FiltFuncD = 0;
        Comp2FiltOffsetD = 0;
        Comp2FiltFuncC = 0;
        Comp2FiltOffsetC = 0;
        Comp2FiltFuncB = 0;
        Comp2FiltOffsetB = 0;
        Comp2FiltFuncA = 0;
        Comp2FiltOffsetA = 0;
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0x00000100 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0xffffff00 );
#endif

        Comp1ExclVal = 0;
        Comp1ExclMask = 0;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;
        break;

        case BAVC_ItbEsType_eH263:
        Comp1ExtractNumBits = 7;
        Comp1ExtractStartBit = 1;
        Comp1ExtractStartByte = 0;
        Comp1En = 1;
        Comp1NumCompareBytes = 2;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp1RepeatByte = 4;
        Comp1CompareVal0 = 0;
        Comp1CompareVal1 = 0;
        Comp1CompareVal2 = 0x80;
        Comp1CompareVal3 = 0;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0x80;
        Comp1MaskVal3 = 0;

        Comp1FiltFuncD = 0;
        Comp1FiltOffsetD = 0;
        Comp1FiltFuncC = 0;
        Comp1FiltOffsetC = 0;
        Comp1FiltFuncB = 0;
        Comp1FiltOffsetB = 0;
        Comp1FiltFuncA = 0;
        Comp1FiltOffsetA = 0;
#else
        Comp1RepeatByte = 2;
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x00008000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xFFFF8000 );
#endif

        Comp2ExtractNumBits = 7;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 1;
        Comp2En = 0;
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 1;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp2FiltFuncD = 0;
        Comp2FiltOffsetD = 0;
        Comp2FiltFuncC = 0;
        Comp2FiltOffsetC = 0;
        Comp2FiltFuncB = 0;
        Comp2FiltOffsetB = 0;
        Comp2FiltFuncA = 0;
        Comp2FiltOffsetA = 0;
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0x0000FC00 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0xFFFFFC00 );
#endif

        /* For video, comparator exclusion val and mask bits are 0 */
        Comp1ExclVal = 0;
        Comp1ExclMask = 0;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;
        break;

        /* For VC1 Simple/Main, just extract PTS values. There aren't any startcodes. */
        /* AMR support needs just the PTSs from the stream. Raptor will do its own CDB-based framesync. */
        case BAVC_ItbEsType_eVC1SimpleMain:
        case BAVC_ItbEsType_eAmr:
        Comp1En = 0;
        Comp2En = 0;
        break;

        case BAVC_ItbEsType_eAc3gAudio:
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 3;
#if BXPT_RAVE_AUDIO_STARTCODES
        Comp1En = 1;
#endif
        Comp1RepeatByte = 0;
        Comp1NumCompareBytes = 1;
        Comp1DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp1CompareVal0 = 0x0B;
        Comp1CompareVal1 = 0x77;
        Comp1CompareVal2 = 0x00;
        Comp1CompareVal3 = 0;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0;
        Comp1MaskVal3 = 0;

        Comp1FiltFuncD = 0;
        Comp1FiltOffsetD = 0;
        Comp1FiltFuncC = 0;
        Comp1FiltOffsetC = 0;
        Comp1FiltFuncB = 0;
        Comp1FiltOffsetB = 0;
        Comp1FiltFuncA = 0;
        Comp1FiltOffsetA = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x0B770000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xFFFF0000 );
#endif

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 1;
#if BXPT_RAVE_AUDIO_STARTCODES
        Comp2En = 0;
#endif
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 0;
        Comp2DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp2FiltFuncD = 0;
        Comp2FiltOffsetD = 0;
        Comp2FiltFuncC = 0;
        Comp2FiltOffsetC = 0;
        Comp2FiltFuncB = 0;
        Comp2FiltOffsetB = 0;
        Comp2FiltFuncA = 0;
        Comp2FiltOffsetA = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0x00000139 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0xFFFFFFB5 );
#endif

        Comp1ExclVal = 0x00;
        Comp1ExclMask = 0x00;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;
        break;

        case BAVC_ItbEsType_eMpegAudio:
        case BAVC_ItbEsType_eMpegAudio2_5:
        case BAVC_ItbEsType_eMpegAudioLayer3:
        /* Raptor is now using CDB-based framesync all chips except 3563 */
#if ( BCHP_CHIP == 3563 )
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 2;
#if BXPT_RAVE_AUDIO_STARTCODES
        Comp1En = 1;
#endif
        Comp1RepeatByte = 0;
        Comp1NumCompareBytes = 1;
        Comp1DataExtractNumBits_6 = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x7FF90000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xFFF00000 );

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 0;
#if BXPT_RAVE_AUDIO_STARTCODES
        Comp2En = 1;
#endif
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 1;
        Comp2ValidByteEn = 1;
        Comp2DataExtractNumBits_6 = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0xFFE00000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0xFFE00000 );

        Comp1ExclVal = 0x00;
        Comp1ExclMask = 0x00;
        Comp2ExclVal = 0xFF;
        Comp2ExclVal = 0x00;
#else
        Comp1En = 0;
        Comp2En = 0;

        /*
        ** PR 22374: Change back to 0x00. Raptor PI will directly change
        ** this register if/when it detects layer 2 or 3 data.
        */
        Comp2ExclMask = 0;
#endif
        break;

        case BAVC_ItbEsType_eAc3Plus:
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
#if BXPT_RAVE_AUDIO_STARTCODES
        Comp1En = 1;
#endif
        Comp1RepeatByte = 0;
        Comp1NumCompareBytes = 1;
        Comp1DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp1CompareVal0 = 0x0B;
        Comp1CompareVal1 = 0x77;
        Comp1CompareVal2 = 0x00;
        Comp1CompareVal3 = 0x00;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0x00;
        Comp1MaskVal3 = 0;

        Comp1FiltFuncD = 0;
        Comp1FiltOffsetD = 0;
        Comp1FiltFuncC = 0;
        Comp1FiltOffsetC = 0;
        Comp1FiltFuncB = 0;
        Comp1FiltOffsetB = 0;
        Comp1FiltFuncA = 0;
        Comp1FiltOffsetA = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x0B770000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xFFFF0000 );
#endif

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 1;
#if BXPT_RAVE_AUDIO_STARTCODES
        Comp2En = 0;
#endif
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 0;
        Comp2DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp2FiltFuncD = 0;
        Comp2FiltOffsetD = 0;
        Comp2FiltFuncC = 0;
        Comp2FiltOffsetC = 0;
        Comp2FiltFuncB = 0;
        Comp2FiltOffsetB = 0;
        Comp2FiltFuncA = 0;
        Comp2FiltOffsetA = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0x00000139 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0xFFFFFFB5 );
#endif

        Comp1ExclVal = 0;
        Comp1ExclMask = 0;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;
        break;

        case BAVC_ItbEsType_eAacAudio:
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
#if BXPT_RAVE_AUDIO_STARTCODES
        Comp1En = 1;
#endif
        Comp1RepeatByte = 2;
        Comp1NumCompareBytes = 2;
        Comp1DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp1CompareVal0 = 0;
        Comp1CompareVal1 = 0;
        Comp1CompareVal2 = 0x01;
        Comp1CompareVal3 = 0x39;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0xFF;
        Comp1MaskVal3 = 0xFF;

        Comp1FiltFuncD = 0;
        Comp1FiltOffsetD = 0;
        Comp1FiltFuncC = 0;
        Comp1FiltOffsetC = 0;
        Comp1FiltFuncB = 0;
        Comp1FiltOffsetB = 0;
        Comp1FiltFuncA = 0;
        Comp1FiltOffsetA = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x00000139 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xFFFFFFFF );
#endif

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 0;
#if BXPT_RAVE_AUDIO_STARTCODES
        Comp2En = 1;
#endif
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 1;
        Comp2ValidByteEn = 1;
        Comp2DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp2CompareVal0 = 0;
        Comp2CompareVal1 = 0;
        Comp2CompareVal2 = 0x01;
        Comp2CompareVal3 = 0x39;
        Comp2MaskVal0 = 0xFF;
        Comp2MaskVal1 = 0xFF;
        Comp2MaskVal2 = 0xFF;
        Comp2MaskVal3 = 0xFF;

        Comp2FiltFuncD = 0;
        Comp2FiltOffsetD = 0;
        Comp2FiltFuncC = 0;
        Comp2FiltOffsetC = 0;
        Comp2FiltFuncB = 0;
        Comp2FiltOffsetB = 0;
        Comp2FiltFuncA = 0;
        Comp2FiltOffsetA = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0xFFF90000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET,    0xFFF60000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0xFFF90000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET,    0xFFF60000 );
#endif

        Comp1ExclVal = 0;
        Comp1ExclMask = 0;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;
        break;

        case BAVC_ItbEsType_eAacHe:
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 0;
#if BXPT_RAVE_AUDIO_STARTCODES
        Comp1En = 1;
#endif
        Comp1RepeatByte = 0;
        Comp1NumCompareBytes = 1;
        Comp1DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp1CompareVal0 = 0x56;
        Comp1CompareVal1 = 0xE0;
        Comp1CompareVal2 = 0x00;
        Comp1CompareVal3 = 0;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xE0;
        Comp1MaskVal2 = 0x00;
        Comp1MaskVal3 = 0;

        Comp1FiltFuncD = 0;
        Comp1FiltOffsetD = 0;
        Comp1FiltFuncC = 0;
        Comp1FiltOffsetC = 0;
        Comp1FiltFuncB = 0;
        Comp1FiltOffsetB = 0;
        Comp1FiltFuncA = 0;
        Comp1FiltOffsetA = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x56E00000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xFFE00000 );
#endif

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 0;
#if BXPT_RAVE_AUDIO_STARTCODES
        Comp2En = 1;
#endif
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 1;
        Comp2ValidByteEn = 1;
        Comp2DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp2CompareVal0 = 0x56;
        Comp2CompareVal1 = 0xE0;
        Comp2CompareVal2 = 0x00;
        Comp2CompareVal3 = 0;
        Comp2MaskVal0 = 0xFF;
        Comp2MaskVal1 = 0xE0;
        Comp2MaskVal2 = 0x00;
        Comp2MaskVal3 = 0;

        Comp2FiltFuncD = 0;
        Comp2FiltOffsetD = 0;
        Comp2FiltFuncC = 0;
        Comp2FiltOffsetC = 0;
        Comp2FiltFuncB = 0;
        Comp2FiltOffsetB = 0;
        Comp2FiltFuncA = 0;
        Comp2FiltOffsetA = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0xFFF90000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET,    0xFFF60000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0xFFF90000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET,    0xFFF60000 );
#endif

        Comp1ExclVal = 0;
        Comp1ExclMask = 0;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;
        break;

        case BAVC_ItbEsType_eAvsAudio:
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
        Comp1En = 0;
        Comp1RepeatByte = 4;
        Comp1NumCompareBytes = 2;
        Comp1DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp1CompareVal0 = 0xFF;
        Comp1CompareVal1 = 0xFF;
        Comp1CompareVal2 = 0xFF;
        Comp1CompareVal3 = 0xFF;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0xFF;
        Comp1MaskVal3 = 0xFF;

        Comp1FiltFuncD = 0;
        Comp1FiltOffsetD = 0;
        Comp1FiltFuncC = 0;
        Comp1FiltOffsetC = 0;
        Comp1FiltFuncB = 0;
        Comp1FiltOffsetB = 0;
        Comp1FiltFuncA = 0;
        Comp1FiltOffsetA = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x00000139 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xFFFFFFFF );
#endif

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 0;
#if BXPT_RAVE_AUDIO_STARTCODES
        Comp2En = 1;
#endif
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 1;
        Comp2ValidByteEn = 1;
        Comp2DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp2CompareVal0 = 0xFF;
        Comp2CompareVal1 = 0xF9;
        Comp2CompareVal2 = 0x00;
        Comp2CompareVal3 = 0x00;
        Comp2MaskVal0 = 0xFF;
        Comp2MaskVal1 = 0xF6;
        Comp2MaskVal2 = 0x00;
        Comp2MaskVal3 = 0x00;

        Comp2FiltFuncD = 0;
        Comp2FiltOffsetD = 0;
        Comp2FiltFuncC = 0;
        Comp2FiltOffsetC = 0;
        Comp2FiltFuncB = 0;
        Comp2FiltOffsetB = 0;
        Comp2FiltFuncA = 0;
        Comp2FiltOffsetA = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0xFFF90000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET,    0xFFF60000 );
#endif

        Comp1ExclVal = 0;
        Comp1ExclMask = 0;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;
        break;

        case BAVC_ItbEsType_eWma:
        Comp1ExtractNumBits = 63;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
#if BXPT_RAVE_AUDIO_STARTCODES
        Comp1En = 1;
#endif
        Comp1RepeatByte = 0;
        Comp1NumCompareBytes = 3;
        Comp1DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp1CompareVal0 = 0x42;
        Comp1CompareVal1 = 0x43;
        Comp1CompareVal2 = 0x4D;
        Comp1CompareVal3 = 0x41;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0xFF;
        Comp1MaskVal3 = 0xFF;

        Comp1FiltFuncD = 0;
        Comp1FiltOffsetD = 0;
        Comp1FiltFuncC = 0;
        Comp1FiltOffsetC = 0;
        Comp1FiltFuncB = 0;
        Comp1FiltOffsetB = 0;
        Comp1FiltFuncA = 0;
        Comp1FiltOffsetA = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x42434D41 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xFFFFFFFF );
#endif

        Comp2ExtractNumBits = 63;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 1;
#if BXPT_RAVE_AUDIO_STARTCODES
        Comp2En = 0;
#endif
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 3;
        Comp2ValidByteEn = 1;
        Comp2DataExtractNumBits_6 = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp2FiltFuncD = 0;
        Comp2FiltOffsetD = 0;
        Comp2FiltFuncC = 0;
        Comp2FiltOffsetC = 0;
        Comp2FiltFuncB = 0;
        Comp2FiltOffsetB = 0;
        Comp2FiltFuncA = 0;
        Comp2FiltOffsetA = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x42434D41 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xFFFFFFFF );
#endif

        Comp1ExclVal = 0;
        Comp1ExclMask = 0;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;
        break;

        case BAVC_ItbEsType_eLpcmAudio:
        case BAVC_ItbEsType_eMlpAudio:
        case BAVC_ItbEsType_eDtsAudio:
        /* diasable both the comparators */
        Comp1En = 0;
        Comp2En = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xFFFFFFFF );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET,  0xFFFFFFFF );
#endif
        break;

        case BAVC_ItbEsType_eMpegAudioWithDescriptor:
        case BAVC_ItbEsType_eAudioDescriptor:
        Comp1ExtractNumBits = 0x3F;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;
        Comp1En = 1;

        Comp1RepeatByte = 0;
        Comp1NumCompareBytes = 3;
        Comp1DataExtractNumBits_6 = 0;
        Comp1PesHdrData = 1;
        Comp1EsData = 0;

#if BXPT_NEXT_GEN_COMPARATORS
        Comp1CompareVal0 = 0x44;
        Comp1CompareVal1 = 0x54;
        Comp1CompareVal2 = 0x47;
        Comp1CompareVal3 = 0x41;
        Comp1MaskVal0 = 0xFF;
        Comp1MaskVal1 = 0xFF;
        Comp1MaskVal2 = 0xFF;
        Comp1MaskVal3 = 0xFF;

        Comp1FiltFuncD = 0;
        Comp1FiltOffsetD = 0;
        Comp1FiltFuncC = 0;
        Comp1FiltOffsetC = 0;
        Comp1FiltFuncB = 0;
        Comp1FiltOffsetB = 0;
        Comp1FiltFuncA = 0;
        Comp1FiltOffsetA = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_FILTER_VAL_MASK_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_OFFSET, 0x00000000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_FILTER_VAL_MASK_OFFSET, 0x00000000 );
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x44544741 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET,    0xFFFFFFFF );
#endif

        Comp2ExtractNumBits = 0;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 0;
        Comp2En = 0;
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 0;
        Comp2ValidByteEn = 0;
        Comp2DataExtractNumBits_6 = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0 );

        Comp1ExclVal = 0;
        Comp1ExclMask = 0;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;
        break;

        case BAVC_ItbEsType_eDra:
        Comp1ExtractNumBits = 0x3F;
        Comp1ExtractStartBit = 0;
        Comp1ExtractStartByte = 1;

        /* SW7405-3065: Disable comparators for DRA.
        Comp1En = 1;
        */

        Comp1RepeatByte = 0;
        Comp1NumCompareBytes = 1;
        Comp1DataExtractNumBits_6 = 0;
        Comp1PesHdrData = 0;
        Comp1EsData = 1;

#if BXPT_NEXT_GEN_COMPARATORS
        /* No support for DRA on the new comparator chips. */
#else
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x7FFF0000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET,    0xFFFF0000 );
#endif

        Comp2ExtractNumBits = 0;
        Comp2ExtractStartBit = 0;
        Comp2ExtractStartByte = 0;
        Comp2En = 0;
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 0;
        Comp2ValidByteEn = 0;
        Comp2DataExtractNumBits_6 = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0 );

        Comp1ExclVal = 0;
        Comp1ExclMask = 0;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;
        break;

        default:
        BDBG_ERR(( "Unsupported BAVC_ItbEsType!" ));
        break;
    }

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_NUM_BITS6 ) |
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
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_NUM_BITS6, Comp1DataExtractNumBits_6 ) |
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
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_NUM_BITS6 ) |
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
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_NUM_BITS6, Comp2DataExtractNumBits_6 ) |
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

#if BXPT_NEXT_GEN_COMPARATORS
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
#if 0
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_D ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_D ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_C ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_C ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_B ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_B ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_A ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_A ) |

        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_FUNC_D ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_OFFSET_D ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_FUNC_C ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_OFFSET_C ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_FUNC_B ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_OFFSET_B ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_FUNC_A ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_OFFSET_A )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_D, Comp1FiltFuncD ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_D, Comp1FiltOffsetD ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_C, Comp1FiltFuncC ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_C, Comp1FiltOffsetC ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_B, Comp1FiltFuncB ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_B, Comp1FiltOffsetB ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_FUNC_A, Comp1FiltFuncA ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP1_FILT_OFFSET_A, Comp1FiltOffsetA ) |

        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_FUNC_D, Comp2FiltFuncD ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_OFFSET_D, Comp2FiltOffsetD ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_FUNC_C, Comp2FiltFuncC ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_OFFSET_C, Comp2FiltOffsetC ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_FUNC_B, Comp2FiltFuncB ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_OFFSET_B, Comp2FiltOffsetB ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_FUNC_A, Comp2FiltFuncA ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP12_FILTER_Mode, COMP2_FILT_OFFSET_A, Comp2FiltOffsetA )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP12_FILTER_MODE_OFFSET, Reg );
#endif
#else
    /*
    ** PR 22374: The comparator exclusion reg is now shared between the RAVE PI and the Raptor PI.
    ** Our modifications become a critical section.
    */
    BKNI_EnterCriticalSection();
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_EXCLUSION_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_EXCLUSION, COMP2_EXCLUSION_VAL ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_EXCLUSION, COMP2_EXCLUSION_MASK ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_EXCLUSION, COMP1_EXCLUSION_VAL ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_EXCLUSION, COMP1_EXCLUSION_MASK )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_EXCLUSION, COMP2_EXCLUSION_VAL, Comp2ExclVal ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_EXCLUSION, COMP2_EXCLUSION_MASK, Comp2ExclMask ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_EXCLUSION, COMP1_EXCLUSION_VAL, Comp1ExclVal ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_EXCLUSION, COMP1_EXCLUSION_MASK, Comp1ExclMask )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_EXCLUSION_OFFSET, Reg );
    BKNI_LeaveCriticalSection();
#endif

#else
    uint32_t Reg;

    unsigned NumCompareBytes = 0;
    unsigned RepeatByte = 0;
    unsigned CompEn = 0;
    unsigned StartByte = 0;
    unsigned StartBit = 0;
    unsigned Comp1ExclVal = 0;
    unsigned Comp1ExclMask = 0;
    unsigned Comp2ExclVal = 0;
    unsigned Comp2ExclMask = 0;
    unsigned ExtractNumBits = 0;
    unsigned Comp2ExtractNumBits = 0;
    unsigned Comp2StartBit = 0;
    unsigned Comp2StartByte = 0;
    unsigned Comp2En = 0;
    unsigned Comp2RepeatByte = 0;
    unsigned Comp2NumCompareBytes = 0;
    unsigned Comp2ValidByteEn = 0;

    BERR_Code ExitCode = BERR_SUCCESS;

    switch( StreamType )
    {
        case BAVC_ItbEsType_eAvcVideo:
        case BAVC_ItbEsType_eMpeg2Video:
        case BAVC_ItbEsType_eMpeg1Video:
        case BAVC_ItbEsType_eVc1Video:
        case BAVC_ItbEsType_eMpeg4Part2:
        case BAVC_ItbEsType_eOTFVideo:
        NumCompareBytes = 2;
        RepeatByte = 0x02;
        CompEn = 1;
        StartByte = 1;
        StartBit = 0;

        if(StreamType == BAVC_ItbEsType_eOTFVideo)
            ExtractNumBits = Comp2ExtractNumBits = 0x3f;
        else
            ExtractNumBits = Comp2ExtractNumBits = 7;

        Comp2StartBit = 0;
        Comp2StartByte = 1;
        Comp2En = 0;
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 0;

        /* For video, comparator exclusion val and mask bits are 0 */
        Comp1ExclVal = 0;
        Comp1ExclMask = 0;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x00000100 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xffffff00 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0x00000100 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0xffffff00 );
        break;

        /* For VC1 Simple/Main, just extract PTS values. There aren't any startcodes. */
        /* AMR support needs just the PTSs from the stream. Raptor will do its own CDB-based framesync. */
        case BAVC_ItbEsType_eVC1SimpleMain:
        case BAVC_ItbEsType_eAmr:
        CompEn = 0;
        Comp2En = 0;
        break;

        case BAVC_ItbEsType_eH263:
        NumCompareBytes = 2;
        RepeatByte = 0x02;
        CompEn = 1;
        StartByte = 0;
        StartBit = 1;
        ExtractNumBits = 7;
        Comp2ExtractNumBits = 7;
        Comp2StartBit = 0;
        Comp2StartByte = 1;
        Comp2En = 0;
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 0;

        /* For video, comparator exclusion val and mask bits are 0 */
        Comp1ExclVal = 0;
        Comp1ExclMask = 0;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x00008000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET,    0xFFFF8000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0x0000FC00 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET,    0xFFFFFC00 );
        break;

        case BAVC_ItbEsType_eAc3gAudio:
        NumCompareBytes = 1;
        RepeatByte = 0;
        CompEn = 1;
        StartByte = 3;
        StartBit = 0;
        ExtractNumBits = 7;
        Comp2ExtractNumBits = 7;
        Comp2StartBit = 0;
        Comp2StartByte = 1;
        Comp2En = 0;
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 0;

        Comp1ExclVal = 0xB5;
        Comp1ExclMask = 0xFF;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x0b770000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xffff0000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0x00000139 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0xffffffb5 );
        break;

        case BAVC_ItbEsType_eMpegAudio:
        case BAVC_ItbEsType_eMpegAudio2_5:
        case BAVC_ItbEsType_eMpegAudioLayer3:
        NumCompareBytes = 1;
        RepeatByte = 0;
        CompEn = 1;
        StartByte = 2;
        StartBit = 4;
        ExtractNumBits = 0x0A;

        Comp2ExtractNumBits = 0x0b;
        Comp2StartBit = 3;
        Comp2StartByte = 0;
        Comp2En = 1;
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 1;
        Comp2ValidByteEn = 1;

        Comp1ExclVal = 0x00;
        Comp1ExclMask = 0x00;
        Comp2ExclVal = 0xFF;

        /* PR 22374: Changed mask from 0xFF to 0x00 */
        /* PR 27677: needs this value to be 0xFF, as mpeg2 decode
           is higher priority chaging this to 0xFF and let the raptor
           team resolve R 22374. */
        /*
        ** PR 22374: Change back to 0x00. Raptor PI will directly change
        ** this register if/when it detects layer 2 or 3 data.
        */

        /* PR 32568: RAPTOR firmware isn't ready for this yet on the 3563. */
#if BCHP_CHIP == 3563
        if( StreamType == BAVC_ItbEsType_eMpegAudio )
            Comp2ExclMask = 0x00;
        else
            Comp2ExclMask = 0xFF;
#else
        Comp2ExclMask = 0x00;
#endif

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x7ff90000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xfff00000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0xffe00000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0xffe00000 );
        break;

        case BAVC_ItbEsType_eAacHe:
        case BAVC_ItbEsType_eAacAudio:
        /* New AAC-HE config for Steve's new fw. */
        NumCompareBytes = 1;
        RepeatByte = 0;
        CompEn = 1;
        StartByte = 0;
        StartBit = 3;
        ExtractNumBits = 0xC;
        Comp2ExtractNumBits = 0x1F;
        Comp2StartBit = 3;
        Comp2StartByte = 0;
        Comp2En = 1;
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 1;
        Comp2ValidByteEn = 1;

        Comp1ExclVal = 0xB5;
        Comp1ExclMask = 0x00;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x56E00000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET,    0xffe00000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0xfff90000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET,    0xfff60000 );
        break;

        case BAVC_ItbEsType_eAc3Plus:
        NumCompareBytes = 1;
        RepeatByte = 0;
        CompEn = 1;
        StartByte = 1;
        StartBit = 5;
        ExtractNumBits = 0x17;
        Comp2ExtractNumBits = 7;
        Comp2StartBit = 0;
        Comp2StartByte = 1;
        Comp2En = 0;
        Comp2RepeatByte = 2;
        Comp2NumCompareBytes = 2;
        Comp2ValidByteEn = 0;

        Comp1ExclVal = 0x00;
        Comp1ExclMask = 0x00;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;

        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x0b770000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xffff0000 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0x00000139 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0xffffffb5 );
        break;

        case BAVC_ItbEsType_eWma:
        ExtractNumBits = 0x1F;
        StartBit = 0;
        StartByte = 1;
        CompEn = 1;
        RepeatByte = 0;
        NumCompareBytes = 3;
        Comp1ExclMask = 0x00;
        Comp1ExclVal = 0x00;
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0x42434d41 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET,    0xffffffff );

        /* COMP2 isn't used */
        Comp2ExtractNumBits = 0;
        Comp2StartBit = 0;
        Comp2StartByte = 0;
        Comp2En = 0;
        Comp2RepeatByte = 0;
        Comp2NumCompareBytes = 0;
        Comp2ValidByteEn = 0;
        Comp2ExclVal = 0;
        Comp2ExclMask = 0;
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0x00 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET, 0x00 );
        break;
        case BAVC_ItbEsType_eLpcmAudio:
        case BAVC_ItbEsType_eMlpAudio:
        case BAVC_ItbEsType_eDtsAudio:
        /* diasable both the comparators */
        CompEn = 0;
        Comp2En = 0;
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_COMPARE_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP1_MASK_OFFSET, 0xFFFFFFFF );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_COMPARE_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_MASK_OFFSET,  0xFFFFFFFF );
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
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ES_DATA, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_PES_HDR_DATA, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ADAPTATION_FIELD, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMPARE_ALL_DATA, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, CASCADE_ENABLE, 0 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, VALID_BYTE_ENABLE, 1 ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, NUM_COMPARE_BYTES, NumCompareBytes ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, REPEAT_BYTE, RepeatByte ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, COMP_ENABLE, CompEn ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_START_BYTE, StartByte ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_START_BIT, StartBit ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP1_CONTROL, DATA_EXTRACT_NUM_BITS, ExtractNumBits )
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
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_START_BYTE, Comp2StartByte ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_START_BIT, Comp2StartBit ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_COMP2_CONTROL, DATA_EXTRACT_NUM_BITS, Comp2ExtractNumBits )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_COMP2_CTRL_OFFSET, Reg );

    /*
    ** PR 22374: The comparator exclusion reg is now shared between the RAVE PI and the Raptor PI.
    ** Our modifications become a critical section.
    */
    BKNI_EnterCriticalSection();
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_EXCLUSION_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_EXCLUSION, COMP2_EXCLUSION_VAL ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_EXCLUSION, COMP2_EXCLUSION_MASK ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_EXCLUSION, COMP1_EXCLUSION_VAL ) |
        BCHP_MASK( XPT_RAVE_CX0_AV_EXCLUSION, COMP1_EXCLUSION_MASK )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_EXCLUSION, COMP2_EXCLUSION_VAL, Comp2ExclVal ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_EXCLUSION, COMP2_EXCLUSION_MASK, Comp2ExclMask ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_EXCLUSION, COMP1_EXCLUSION_VAL, Comp1ExclVal ) |
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_EXCLUSION, COMP1_EXCLUSION_MASK, Comp1ExclMask )
    );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_EXCLUSION_OFFSET, Reg );
    BKNI_LeaveCriticalSection();
#endif

    return( ExitCode );
}

static BERR_Code AllocateSpliceQueue(
    BXPT_RaveCx_Handle hCtx,
    unsigned *QueueNum
    )
{
    unsigned Index;
    uint32_t SpliceQueueDmemPtr, SpliceQueueDmemAddr;

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
    SpliceQueueDmemAddr = SPLICE_QUEUE_AREA_OFFSET/4 + ( Index * SPLICE_QUEUE_AREA_SIZE/4 );

    /* Get a pointer to the context-specific memory area for this context */
    SpliceQueueDmemPtr = SCD_DMEM_BASE + ( hCtx->Index * SCD_DMEM_SIZE ) + SPLICE_QUEUE_PTR_OFFSET;
    BREG_Write32( hCtx->hReg, BCHP_XPT_RAVE_DMEMi_ARRAY_BASE + (SpliceQueueDmemPtr * 4), SpliceQueueDmemAddr );

    /* Now clear the stack */
    BXPT_Rave_ClearQueue( hCtx );

    Done:
    return( ExitCode );
}

static BERR_Code ResetContextPointers(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t Offset;

    BERR_Code ExitCode = BERR_SUCCESS;

    Offset = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_WRITE_PTR_OFFSET, Offset );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_READ_PTR_OFFSET, Offset );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_VALID_PTR_OFFSET, Offset );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_WRAP_PTR_OFFSET, 0 );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_DEPTH_OFFSET, 0 );

    Offset = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + ITB_BASE_PTR_OFFSET );

    /* Some users don't allocate an ITB, so we check the base ptr is not NULL */
    if( Offset )
    {
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + ITB_WRITE_PTR_OFFSET, Offset );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + ITB_READ_PTR_OFFSET, Offset );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + ITB_VALID_PTR_OFFSET, Offset );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + ITB_WRAP_PTR_OFFSET, 0 );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + ITB_DEPTH_OFFSET, 0 );
    }

    return( ExitCode );
}

BERR_Code BXPT_Rave_ResetContext(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t Reg, endainCtrl,cdbBasePtr,cdbEndPtr,itbBasePtr,itbEndPtr,intrEnable, tpitEnable;
    uint32_t ScdPidChannel = 0, ScdPidValid = false;
    BERR_Code ExitCode = BERR_SUCCESS;
    BXPT_Rave_Handle hRave;
    BXPT_Rave_ContextThresholds Thresholds;

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);

    hRave = (BXPT_Rave_Handle)hCtx->vhRave;

    BXPT_Rave_FlushContext(hCtx);

    /* save endian control values */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG2_OFFSET );
    endainCtrl = Reg &(BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, ITB_ENDIAN_CTRL ) | BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, CDB_ENDIAN_CTRL ));

    /* save all the pointers */
    cdbBasePtr = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET );
    cdbEndPtr =  BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_END_PTR_OFFSET );
    itbBasePtr = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + ITB_BASE_PTR_OFFSET );
    itbEndPtr =  BREG_Read32( hCtx->hReg, hCtx->BaseAddr + ITB_END_PTR_OFFSET );

    /* save interrupt enable register */
    intrEnable = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_INTERRUPT_ENABLES_OFFSET );

    /* save the TPIT_ENABLE value
       SW7335-531 : Restore TPIT_CHANNEL during context reset */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET );
    tpitEnable = Reg &(BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_ENABLE ) | BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, TPIT_CHANNEL ));

    /* Save the SCD config for VCT contexts */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET );
#if BXPT_7440B0_SCD_MAPPING
    ScdPidChannel = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_SCD_PIDS_AB, SCD_PIDA );
    ScdPidValid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_SCD_PIDS_AB, PIDA_VALID );
#elif BXPT_REC_SCD_MAPPING
    ScdPidChannel = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDA );
    ScdPidValid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID );
#else
    ScdPidChannel = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_PIDA );
    ScdPidValid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDA_VALID );
#endif
    BXPT_Rave_GetThresholds( hCtx, &Thresholds );

    /* reset the context without allocating the buffers */
    InitContext(hCtx,NULL);

    ExitCode = BXPT_Rave_SetThresholds( hCtx, &Thresholds );
    if( BERR_SUCCESS != ExitCode )
        goto Done;

    /* By default, all SCD mapping should be done by PID channels */
    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET );
#if BXPT_7440B0_SCD_MAPPING
    Reg &= ~BCHP_MASK( XPT_RAVE_CX0_MISC_CONFIG, SCD_MAP_MODE );
#else
    Reg &= ~BCHP_MASK( XPT_RAVE_CX0_REC_MISC_CONFIG, SCD_MAP_MODE );
#endif
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_MISC_CFG_OFFSET, Reg );

    /* Each AV will need 1 SCD indexer, to build the ITB entries. */
    if( hCtx->Type == BXPT_RaveCx_eAv || hCtx->Type == BXPT_RaveCx_eAvR)
    {
        InitScd( hCtx->hAvScd );
        /* Now map this SCD to the AV context */
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET );
        Reg &= ~(
#if BXPT_7440B0_SCD_MAPPING
           BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_NUMA )
#elif BXPT_REC_SCD_MAPPING
           BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA )
#else
           BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMA )
#endif
        );
        Reg |= (
#if BXPT_REC_SCD_MAPPING
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA, hCtx->hAvScd-hRave->ScdTable )
#else
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMA, hCtx->hAvScd-hRave->ScdTable )
#endif
        );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET, Reg );
    }
    else if( hCtx->Type == BXPT_RaveCx_eVctNull )
    {
        /* Same as AV contexts, except we also restore the PID channel and Valid settings */

        InitScd( hCtx->hAvScd );

        /* Now map this SCD to the AV context */
        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET );
        Reg &= ~(
#if BXPT_7440B0_SCD_MAPPING
           BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_NUMA ) |
           BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_PIDA ) |
           BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, PIDA_VALID )
#elif BXPT_REC_SCD_MAPPING
           BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA ) |
           BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDA ) |
           BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID )
#else
           BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMA ) |
           BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_PIDA ) |
           BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDA_VALID )
#endif
        );

        Reg |= (
#if BXPT_7440B0_SCD_MAPPING
           BCHP_FIELD_DATA( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_NUMA, hCtx->hAvScd-hRave->ScdTable ) |
           BCHP_FIELD_DATA( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_PIDA, ScdPidChannel ) |
           BCHP_FIELD_DATA( XPT_RAVE_CX0_SCD_PIDS_AB, PIDA_VALID, ScdPidValid )
#elif BXPT_REC_SCD_MAPPING
           BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA, hCtx->hAvScd-hRave->ScdTable ) |
           BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDA, ScdPidChannel ) |
           BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID, ScdPidValid )
#else
           BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMA, hCtx->hAvScd-hRave->ScdTable ) |
           BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_PIDA, ScdPidChannel ) |
           BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDA_VALID, ScdPidValid )
#endif
        );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET, Reg );
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
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_WRITE_PTR_OFFSET, cdbBasePtr );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_READ_PTR_OFFSET, cdbBasePtr );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET, cdbBasePtr );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_END_PTR_OFFSET, cdbEndPtr );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_VALID_PTR_OFFSET, cdbBasePtr );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_WRAP_PTR_OFFSET, 0 );

    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + ITB_WRITE_PTR_OFFSET, itbBasePtr );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + ITB_READ_PTR_OFFSET, itbBasePtr );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + ITB_BASE_PTR_OFFSET, itbBasePtr );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + ITB_END_PTR_OFFSET, itbEndPtr );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + ITB_VALID_PTR_OFFSET, itbBasePtr );
    BREG_Write32( hCtx->hReg, hCtx->BaseAddr + ITB_WRAP_PTR_OFFSET, 0 );

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

    Done:
    return( ExitCode );
}

static BERR_Code ChangeScdPid(
    BXPT_RaveCx_Handle hCtx,
    uint32_t RegAddr,
    unsigned WhichScdNum,
    unsigned WhichScdBlock,
    unsigned Pid,
    bool PidValid
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    Reg = BREG_Read32( hCtx->hReg, RegAddr );

    if( WhichScdNum % 2 )
    {
        /* The macro for SCD_PIDB is used to load the odd-numbered SCD PIDs */
        Reg &= ~(
#if BXPT_7440B0_SCD_MAPPING
            BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_NUMB ) |
            BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, PIDB_VALID ) |
            BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_PIDB )
#elif BXPT_REC_SCD_MAPPING
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMB ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDB_VALID ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDB )
#else
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMB ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDB_VALID ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_PIDB )
#endif
        );

        Reg |= (
#if BXPT_REC_SCD_MAPPING
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMB, WhichScdBlock ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDB_VALID, PidValid == true ? 1 : 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDB, Pid )
#else
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMB, WhichScdBlock ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDB_VALID, PidValid == true ? 1 : 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_PIDB, Pid )
#endif
        );
    }
    else
    {
        /* The macro for SCD_PIDA is used to load the even-numbered SCD PIDs */
        Reg &= ~(
#if BXPT_7440B0_SCD_MAPPING
            BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_NUMA ) |
            BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, PIDA_VALID ) |
            BCHP_MASK( XPT_RAVE_CX0_SCD_PIDS_AB, SCD_PIDA )
#elif BXPT_REC_SCD_MAPPING
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDA )
#else
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMA ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDA_VALID ) |
            BCHP_MASK( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_PIDA )
#endif
        );

        Reg |= (
#if BXPT_REC_SCD_MAPPING
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA, WhichScdBlock ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID, PidValid == true ? 1 : 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_PIDA, Pid )
#else
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMA, WhichScdBlock ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDA_VALID, PidValid == true ? 1 : 0 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_PIDA, Pid )
#endif
        );
    }

    BREG_Write32( hCtx->hReg, RegAddr, Reg );

    return ExitCode;
}

static BERR_Code FlushScds(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t ScdEnables;
#if BXPT_REC_SCD_MAPPING

    ScdEnables = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDA_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMA ) );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_PIDB_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_AB, Mapped_SCD_via_PID_channels_SCD_NUMB ) );

    ScdEnables = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_CD, Mapped_SCD_via_PID_channels_PIDC_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_CD, Mapped_SCD_via_PID_channels_SCD_NUMC ) );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_CD, Mapped_SCD_via_PID_channels_PIDD_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_CD, Mapped_SCD_via_PID_channels_SCD_NUMD ) );

    ScdEnables = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_EF, Mapped_SCD_via_PID_channels_PIDE_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_EF, Mapped_SCD_via_PID_channels_SCD_NUME ) );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_EF, Mapped_SCD_via_PID_channels_PIDF_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_EF, Mapped_SCD_via_PID_channels_SCD_NUMF ) );

    ScdEnables = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_GH, Mapped_SCD_via_PID_channels_PIDG_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_GH, Mapped_SCD_via_PID_channels_SCD_NUMG ) );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_GH, Mapped_SCD_via_PID_channels_PIDH_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_GH, Mapped_SCD_via_PID_channels_SCD_NUMH ) );
#else
    ScdEnables = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_AB_OFFSET );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDA_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMA ) );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_AB, PIDB_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_AB, SCD_NUMB ) );

    ScdEnables = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_CD_OFFSET );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_CD, PIDC_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_CD, SCD_NUMC ) );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_CD, PIDD_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_CD, SCD_NUMD ) );

    ScdEnables = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_EF_OFFSET );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_EF, PIDE_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_EF, SCD_NUME ) );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_EF, PIDF_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_EF, SCD_NUMF ) );

    ScdEnables = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + REC_SCD_PIDS_GH_OFFSET );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_GH, PIDG_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_GH, SCD_NUMG ) );
    if( BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_GH, PIDH_VALID ) )
        ClearScdStates( hCtx, BCHP_GET_FIELD_DATA( ScdEnables, XPT_RAVE_CX0_REC_SCD_PIDS_GH, SCD_NUMH ) );
#endif

    return BERR_SUCCESS;
}

static BERR_Code ClearScdStates(
    BXPT_RaveCx_Handle hCtx,
    uint32_t ScdNum
    )
{
    unsigned ii;

    unsigned BaseAddr = BCHP_XPT_RAVE_SCD0_SCD_MISC_CONFIG + ( ScdNum * SCD_REG_STEP );

    for( ii = SCD_STATE_START_OFFSET; ii <= SCD_STATE_END_OFFSET; ii += 4 )
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

static BERR_Code FlushTpit(
    BXPT_RaveCx_Handle hCtx,
    uint32_t TpitNum
    )
{
    unsigned ii;

    unsigned BaseAddr = BCHP_XPT_RAVE_TPIT0_CTRL1 + ( TpitNum * TPIT_CTRL_REG_STEP );

    for( ii = TPIT_STATE_START_OFFSET; ii <= TPIT_STATE_END_OFFSET; ii += 4 )
        BREG_Write32( hCtx->hReg, BaseAddr + ii, 0 );

    return BERR_SUCCESS;
}

static int GetNextPicCounter(
    BXPT_RaveCx_Handle hCtx
    )
{
#if BXPT_PER_CONTEXT_PIC_COUNTER
    /* Each context has it's own picture counter, hard-wired. */
    return (int) hCtx->Index;
#else
    unsigned Index;
    BXPT_Rave_Handle hRave;

    hRave = hCtx->vhRave;

    for( Index = 0; Index < BXPT_P_MAX_PIC_COUNTER; Index++ )
        if( hRave->PicCounterUsed[ Index ] == false )
            return Index;

    return -1;
#endif
}

static void FreePicCounter(
    BXPT_RaveCx_Handle Context      /* [in] The context to free. */
    )
{
#if BXPT_PER_CONTEXT_PIC_COUNTER
    /* Each context has it's own picture counter, hard-wired. */
    BSTD_UNUSED( Context );
#else
    BXPT_Rave_Handle lhRave = Context->vhRave;

    if( Context->PicCounter == -1 )
        return;

    lhRave->PicCounterUsed[ Context->PicCounter ] = false;
    BREG_Write32( Context->hReg, BCHP_XPT_RAVE_PIC_CTR0 + ( 4 * Context->PicCounter ), 0 );
#endif
}

static void SetPictureCounterMode(
    BXPT_RaveCx_Handle hCtx,        /* [in] The context to free. */
    BAVC_ItbEsType ItbFormat
    )
{
    uint32_t Reg;
    unsigned Mode;

#if BXPT_PER_CONTEXT_PIC_COUNTER

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
#else
    Mode = 0;
    if( hCtx->PicCounter != -1 )
    {
        switch( ItbFormat )
        {
            case BAVC_ItbEsType_eMpeg2Video:
            case BAVC_ItbEsType_eMpeg1Video:
            Mode = 0;
            break;

            case BAVC_ItbEsType_eAvcVideo: Mode = 1; break;
            case BAVC_ItbEsType_eVc1Video: Mode = 2; break;

            default:
            BDBG_ERR(( "Invalid ITB type for picture counter. Defaulting to MPEG." ));
            break;
        }

        Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG2_OFFSET );
        Reg &= ~(
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, PIC_CTR_SEL ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, PIC_CTR_EN ) |
            BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG2, PIC_CTR_MODE )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, PIC_CTR_SEL, hCtx->PicCounter ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, PIC_CTR_EN, 1 ) |
            BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG2, PIC_CTR_MODE, Mode )
        );
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + AV_MISC_CFG2_OFFSET, Reg );
    }
#endif
}

static void FlushPicCounter(
    BXPT_RaveCx_Handle hCtx
    )
{
#if BXPT_PER_CONTEXT_PIC_COUNTER

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
#else
    unsigned ClearVal = BREG_Read32( hCtx->hReg, BCHP_XPT_RAVE_PIC_CTR0 + ( 4 * hCtx->PicCounter ) );

    BREG_Write32( hCtx->hReg, BCHP_XPT_RAVE_PIC_CTR0 + ( 4 * hCtx->PicCounter ), ClearVal );
#endif
}


BERR_Code BXPT_Rave_GetDefaultThresholds(
    BXPT_RaveCx_Handle hCtx,
    BXPT_Rave_ContextThresholds *Thresholds
    )
{
    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
    /* A size of 0 will return the default thresholds. */
    return BXPT_Rave_ComputeThresholds( hCtx, 0, 0, Thresholds );
}

BERR_Code BXPT_Rave_ComputeThresholds(
    BXPT_RaveCx_Handle hCtx,
    size_t CdbLength,
    size_t ItbLength,
    BXPT_Rave_ContextThresholds *Thresholds
    )
{
    uint32_t Base, End, GranularityInBytes;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
    BDBG_ASSERT( Thresholds );

    #define BXPT_P_MIN_CDB_LEN (BXPT_RAVE_OVERFLOW_THRESH + (11 * 188))
    #define BXPT_P_MIN_ITB_LEN (BXPT_RAVE_OVERFLOW_THRESH + (11 * 240))

    GranularityInBytes = GetGranularityInBytes( hCtx );

    /* if they didn't give us one, get one from the context. Since they have a handle to it, it's been initialized */
    if( !CdbLength )
    {
        Base = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET );
        End = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_END_PTR_OFFSET );
        CdbLength = End - Base + 1;
    }

    if( CdbLength < BXPT_P_MIN_CDB_LEN )
    {
        BDBG_WRN(( "Requested CdbLength < minimum (%u). Upper and lower thresholds will be clamped to %u", BXPT_P_MIN_CDB_LEN, GranularityInBytes ));

        /* Remember that these values are multiplied by the granularity (in hw). So, this is the smallest we can get. */
        Thresholds->CdbUpper = Thresholds->CdbLower = 1;
    }
    else
    {
        /* CDB Upper Threshold = [CDB Length - Overflow Threshold - (9 * 188)] / X, where X is the Watermark Granularity in bytes. */
        Thresholds->CdbUpper = (CdbLength - BXPT_RAVE_OVERFLOW_THRESH - (9 * 188)) / GranularityInBytes;

        if( Thresholds->CdbUpper > 0xFFFF )
        {
            BDBG_ERR(( "CdbUpper exceeds 16-bit size limit" ));
            return BERR_TRACE( BERR_INVALID_PARAMETER );
        }

        /* CDB Lower Threshold = [CDB Length - Overflow Threshold - (11 * 188)] / X, where X is the watermark granularity */
        Thresholds->CdbLower = (CdbLength - BXPT_P_MIN_CDB_LEN) / GranularityInBytes;

        if( Thresholds->CdbLower > 0xFFFF )
        {
            BDBG_ERR(( "CdbLower exceeds 16-bit size limit" ));
            return BERR_TRACE( BERR_INVALID_PARAMETER );
        }
    }

    if( !ItbLength )
    {
        Base = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + ITB_BASE_PTR_OFFSET );
        End = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + ITB_END_PTR_OFFSET );

        /* Some users don't allocate an ITB. Check the base reg to be sure. */
        if( Base )
        {
            ItbLength = End - Base + 1;
        }
    }

    if( ItbLength )
    {
        if ( ItbLength < BXPT_P_MIN_ITB_LEN )
        {
            BDBG_ERR(( "ItbLength %d too small", ItbLength));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        /* ITB Upper Threshold = [ITB length - Overflow Threshold - (9 * 240)] / X, where X is the watermark granularity */
        Thresholds->ItbUpper = (ItbLength - BXPT_RAVE_OVERFLOW_THRESH - (9 * 240)) / GranularityInBytes;

        /* ITB Lower Threshold = [ ITB length - Overflow Threshold - (11 * 240)] / X, where X is the watermark granularity */
        Thresholds->ItbLower = (ItbLength - BXPT_P_MIN_ITB_LEN) / GranularityInBytes;

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
        Thresholds->ItbUpper = 1;
        Thresholds->ItbLower = 1;
    }

    return ExitCode;
}


BERR_Code BXPT_Rave_GetThresholds(
    BXPT_RaveCx_Handle hCtx,
    BXPT_Rave_ContextThresholds *Thresholds
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
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

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
    BDBG_ASSERT( Thresholds );

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

#if BXPT_HAS_RAVE_SCRAMBLING_CONTROL

BERR_Code BXPT_Rave_GetScramblingCtrl(
    BXPT_RaveCx_Handle hCtx,
    BXPT_Rave_ScrambleCtrl *ScrambleCtrl
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
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

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);

    Reg = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + SCRAMBLE_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_RAVE_CX0_SC, SC_PUSI_VALID ) |
        BCHP_MASK( XPT_RAVE_CX0_SC, SC_PUSI ) |
        BCHP_MASK( XPT_RAVE_CX0_SC, SC_OR_MODE ) |
        BCHP_MASK( XPT_RAVE_CX0_SC, SC_ALL_VALID ) |
        BCHP_MASK( XPT_RAVE_CX0_SC, SC_ALL )
    );

    /* restore SC_OR_MODE
     PR57627 :
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
#endif

#if BXPT_HAS_RAVE_PES_EXTENSIONS

BERR_Code BXPT_Rave_GetContextDefaultSettings(
    BXPT_Rave_ContextSettings *ContextDefSettings   /* [out] The defaults. */
    )
{
     BKNI_Memset( (void *)ContextDefSettings, 0, sizeof(BXPT_Rave_ContextSettings));
     /* ContextDefSettings->PesExtSearchMode = BXPT_Rave_PesExtSearchAlways;*/
     return BERR_SUCCESS;
}

BERR_Code BXPT_Rave_GetContextConfig(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    BXPT_Rave_ContextSettings *Config   /* [out] The Context settings. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Context, bxpt_t);
    BDBG_ASSERT( Config );

#if ( BCHP_CHIP == 7401 ) || ( BCHP_CHIP == 7118 )
    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_PID_STREAM_ID_OFFSET );
    Config->AudFrameInfo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_PID_STREAM_ID, AUD_FRAME_INFO );
    Config->SsidEn = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_PID_STREAM_ID, SSID_ENABLE );
    Config->FilterPidStreamId = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_PID_STREAM_ID, FILTER_PID_STREAM_ID );
    Config->BandNum = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_PID_STREAM_ID, BAND_NUM );
#if ((BCHP_CHP == 7401) || (BCHP_CHIP == 7118 && BCHP_VER >= BCHP_VER_C0))
    Config->GenericPsEnable = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_PID_STREAM_ID, GENERIC_PS_ENABLE );
#endif
    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_RESERVE_CFG2_OFFSET );

#if ((BCHP_CHP == 7401) || (BCHP_CHIP == 7118 && BCHP_VER >= BCHP_VER_C0))
    Config->GenericPsSsid = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_RESERVE_CFG2, GENERIC_PS_SSID );
#endif

#else
    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG3_OFFSET );
    Config->EnableBppSearch = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, ENABLE_BPP_SEARCH ) ? true : false;
    Config->EnableCpChangeDetect = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, CP_PERM_CHANGE_DETECT ) ? true : false;
    Config->PesExtSearchMode = (BXPT_Rave_PesExtSearchMode)BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG3, PES_EXT_SEARCH_MODE );


    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_PID_STREAM_ID_OFFSET );
    Config->EnablePrivateHdrItbEntry = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_PID_STREAM_ID, PRV_HDR_ITB_EN ) ? true : false;
    Config->AudFrameInfo = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_PID_STREAM_ID, AUD_FRAME_INFO );

    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_MISC_CFG4_OFFSET );
    Config->SidExtDependent = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG4, PES_SID_EXT_DEP );
    Config->SidExtIndependent = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG4, PES_SID_EXT_IND );
    Config->PesSidExtMode = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG4, PES_SID_EXT_MODE );
#endif
    return ExitCode;
}

BERR_Code BXPT_Rave_SetContextConfig(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    const BXPT_Rave_ContextSettings *Config /* [out] The Context settings. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(Context, bxpt_t);
    BDBG_ASSERT( Config );

#if ( BCHP_CHIP == 7401 ) || ( BCHP_CHIP == 7118 )
    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_PID_STREAM_ID_OFFSET );
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_AV_PID_STREAM_ID, SSID_ENABLE ) |
              BCHP_MASK( XPT_RAVE_CX0_AV_PID_STREAM_ID, AUD_FRAME_INFO ) |
              BCHP_MASK( XPT_RAVE_CX0_AV_PID_STREAM_ID, FILTER_PID_STREAM_ID ) |
#if ((BCHP_CHP == 7401) || (BCHP_CHIP == 7118 && BCHP_VER >= BCHP_VER_C0))
              BCHP_MASK( XPT_RAVE_CX0_AV_PID_STREAM_ID, GENERIC_PS_ENABLE ) |
#endif
              BCHP_MASK( XPT_RAVE_CX0_AV_PID_STREAM_ID, BAND_NUM )
            );
    Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_PID_STREAM_ID, SSID_ENABLE, Config->SsidEn == true ? 1 : 0 ) |
             BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_PID_STREAM_ID, AUD_FRAME_INFO, Config->AudFrameInfo ) |
             BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_PID_STREAM_ID, FILTER_PID_STREAM_ID, Config->FilterPidStreamId ) |
#if ((BCHP_CHP == 7401) || (BCHP_CHIP == 7118 && BCHP_VER >= BCHP_VER_C0))
             BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_PID_STREAM_ID, GENERIC_PS_ENABLE, Config->GenericPsEnable ) |
#endif
             BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_PID_STREAM_ID, BAND_NUM, Config->BandNum )
             );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_PID_STREAM_ID_OFFSET, Reg );

#if ((BCHP_CHP == 7401) || (BCHP_CHIP == 7118 && BCHP_VER >= BCHP_VER_C0))
    Reg = BREG_Read32( Context->hReg, Context->BaseAddr + AV_RESERVE_CFG2_OFFSET );
    Reg &= ~( BCHP_MASK( XPT_RAVE_CX0_AV_RESERVE_CFG2, GENERIC_PS_SSID ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_RESERVE_CFG2, GENERIC_PS_SSID, Config->GenericPsSsid ) );
    BREG_Write32( Context->hReg, Context->BaseAddr + AV_RESERVE_CFG2_OFFSET, Reg );
#endif

#else
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
              BCHP_MASK( XPT_RAVE_CX0_AV_PID_STREAM_ID, AUD_FRAME_INFO )
            );
    Reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_PID_STREAM_ID, PRV_HDR_ITB_EN, Config->EnablePrivateHdrItbEntry == true ? 1 : 0 )|
             BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_PID_STREAM_ID, AUD_FRAME_INFO, Config->AudFrameInfo )
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
#endif

    return ExitCode;
}

#endif /* BCHP_CHIP == 7440 */

#if 0
Old version. Newer version added to support VCT contexts
static BERR_Code GetNextFreeScd(
    BXPT_Rave_Handle hRave,
    StartCodeIndexer **ScdPtr
    )
{
    unsigned ii;

    for( ii = 0; ii < BXPT_P_MAX_SCD; ii++ )
    {
        if( hRave->ScdTable[ ii ].Allocated == false )
        {
            hRave->ScdTable[ ii ].Allocated = true;
            *ScdPtr = &hRave->ScdTable[ ii ];
            return BERR_SUCCESS;
        }
    }

    /* Didn't find a free SCD if we got this far. */
    return BXPT_ERR_NO_AVAILABLE_RESOURCES;
}
#endif

static void FreeScds(
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

BERR_Code BXPT_Rave_GetIpConfig(
    BXPT_RaveCx_Handle hCtx,        /* [in] Handle for the IP context */
    BXPT_Rave_IpConfig *IpConfig    /* [out] The IP config params */
    )
{
    uint32_t ScdBytesBase;
    unsigned ScdNum;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
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

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
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
    uint32_t Base;
    void *Ptr;

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);

    Base = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET );
    if( BMEM_ConvertOffsetToAddress( hCtx->hMem, Base, &Ptr ) != BERR_SUCCESS )
    {
        BDBG_ERR(( "Offset to address conversion failed, context %lu", (unsigned long) hCtx ));
        Ptr = NULL;
    }

    return Ptr;
}

static void GetScEnables(
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

static void GetScEnables_Indexer(
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

BERR_Code BXPT_Rave_AllocSoftContext(
    BXPT_RaveCx_Handle SrcContext,      /* [in] The source context */
    BXPT_RaveSoftMode DestContextMode,  /* [in] The type of data that the destination should generate. */
    BXPT_RaveCx_Handle *DestContext     /* [out] The destination (soft) context */
    )
{
    BDBG_OBJECT_ASSERT(SrcContext, bxpt_t);
    return( BXPT_Rave_AllocSoftContextFromHeap( SrcContext, DestContextMode, NULL, DestContext ));
}

BERR_Code BXPT_Rave_AllocSoftContextFromHeap(
    BXPT_RaveCx_Handle SrcContext,      /* [in] The source context */
    BXPT_RaveSoftMode DestContextMode,  /* [in] The type of data that the destination should generate. */
    BMEM_Handle ContextHeap,            /* [in] The soft-context's memory heap handle */
    BXPT_RaveCx_Handle *DestContext     /* [out] The destination (soft) context */
    )
{
    unsigned Index;

    BXPT_RaveCx_Handle ThisCtx = NULL;
    BXPT_Rave_Handle hRave;

    BDBG_ASSERT( SrcContext );
    BDBG_ASSERT( DestContext );

    hRave = (BXPT_Rave_Handle) SrcContext->vhRave;

    if( GetNextSoftRaveContext( hRave, &Index ) != BERR_SUCCESS )
    {
        BDBG_ERR(( "No free contexts!" ));
        return BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
    }

    /* Same as the normal context alloc, except the we substitute the caller's heap
    for the general handle that was installed when the RAVE channel was openned. */
    ThisCtx = hRave->ContextTbl + Index;
    ThisCtx->hMem = ContextHeap ? ContextHeap : hRave->hMem;

    /* InitContext() will do the alloc */
    ThisCtx->CdbBufferAddr = NULL;
    ThisCtx->ItbBufferAddr = NULL;

    return AllocSoftContext_Priv( SrcContext, DestContextMode, Index, DestContext );
}

BERR_Code AllocSoftContext_Priv(
    BXPT_RaveCx_Handle SrcContext,      /* [in] The source context */
    BXPT_RaveSoftMode DestContextMode,  /* [in] The type of data that the destination should generate. */
    unsigned Index,
    BXPT_RaveCx_Handle *DestContext     /* [out] The destination (soft) context */
    )
{
    uint32_t Base, End;

    BERR_Code ExitCode = BERR_SUCCESS;
    BXPT_RaveCx_Handle ThisCtx = NULL;
    BXPT_Rave_Handle hRave;
    BXPT_RaveCx SoftCtxMode = DestContextMode == BXPT_RaveSoftMode_eIndexOnlyRecord ? BXPT_RaveCx_eRecord : BXPT_RaveCx_eAv;

    BDBG_OBJECT_ASSERT(SrcContext, bxpt_t);
    hRave = (BXPT_Rave_Handle) SrcContext->vhRave;

    ThisCtx = hRave->ContextTbl + Index;
    ThisCtx->Allocated = true;
    ThisCtx->Type = SoftCtxMode;

    ThisCtx->SoftRave.mode = (unsigned) DestContextMode;
    ThisCtx->SoftRave.SrcBaseAddr = SrcContext->BaseAddr;
    ThisCtx->IsSoftContext = true;
    ThisCtx->SoftRave.SrcContextIndex = SrcContext->Index;
    ThisCtx->allocatedCdbBufferSize = SrcContext->allocatedCdbBufferSize;

    /* Make the dest BASE/END point to the src's CDB/ITB */
    Base = BREG_Read32( SrcContext->hReg, SrcContext->BaseAddr + CDB_BASE_PTR_OFFSET );
    BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_BASE_PTR_OFFSET, Base );
    End = BREG_Read32( SrcContext->hReg, SrcContext->BaseAddr + CDB_END_PTR_OFFSET );
    BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + CDB_END_PTR_OFFSET, End );
    ThisCtx->SoftRave.ItbSize = End - Base;

    Base = BREG_Read32( SrcContext->hReg, SrcContext->BaseAddr + ITB_BASE_PTR_OFFSET );
    End = BREG_Read32( SrcContext->hReg, SrcContext->BaseAddr + ITB_END_PTR_OFFSET );
    if( DestContextMode == BXPT_RaveSoftMode_ePointersOnly )
    {
        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_BASE_PTR_OFFSET, Base );
        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_END_PTR_OFFSET, End );
    }
    else
    {
        uint32_t BufferOffset;

        void *BufferAddr = NULL;

        if( ThisCtx->ItbBufferAddr==NULL ) /* XPT does the alloc */
        {
            ThisCtx->externalItbAlloc = false;

            /* Allocate a separate ITB for the destination */
            BufferAddr = BMEM_AllocAligned( ThisCtx->hMem, ThisCtx->SoftRave.ItbSize, 8, 0 );
            if( BufferAddr == NULL )
            {
                BDBG_ERR(( "ITB alloc failed!" ));
                ExitCode = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
                goto Done;
            }
            else
            {
                ThisCtx->ItbBufferAddr = BufferAddr; /* save the itb allocated buffer address allocated */
            }
        }
        else /* the app has already done the alloc */
        {
            ThisCtx->externalItbAlloc = true;

            BufferAddr = ThisCtx->ItbBufferAddr;
            /*BDBG_MSG(( "BXPT_Rave_AllocSoftContext: using pre-allocated ITB %#lx", ThisCtx->ItbBufferAddr ));*/
        }

        BMEM_ConvertAddressToOffset( ThisCtx->hMem, BufferAddr, &BufferOffset );
        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_BASE_PTR_OFFSET, BufferOffset );
        BREG_Write32( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_END_PTR_OFFSET, BufferOffset + ThisCtx->SoftRave.ItbSize );
    }

    ThisCtx->SoftRave.src_itb_base = BREG_Read32( SrcContext->hReg, SrcContext->BaseAddr + ITB_BASE_PTR_OFFSET );
    ThisCtx->SoftRave.dest_itb_base = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_BASE_PTR_OFFSET );
    BXPT_Rave_ResetSoftContext( ThisCtx, DestContextMode );
    BXPT_Rave_FlushContext( SrcContext );

    /* get CACHED pointers to ITB memory for read/write */
    {
        void *UncachedAddr = NULL;

        Base = BREG_Read32( SrcContext->hReg, SrcContext->BaseAddr + ITB_BASE_PTR_OFFSET );
        BMEM_Heap_ConvertOffsetToAddress( SrcContext->hMem, Base, &UncachedAddr );
        BMEM_ConvertAddressToCached(SrcContext->hMem, UncachedAddr, (void *) &(ThisCtx->SoftRave.src_itb_mem));

        Base = BREG_Read32( ThisCtx->hReg, ThisCtx->BaseAddr + ITB_BASE_PTR_OFFSET );
        BMEM_Heap_ConvertOffsetToAddress( ThisCtx->hMem, Base, &UncachedAddr );
        BMEM_ConvertAddressToCached(ThisCtx->hMem, UncachedAddr, (void *) &(ThisCtx->SoftRave.dest_itb_mem));
    }

    *DestContext = ThisCtx;

    BDBG_MSG(( "AllocSoftContext: Src %u, Dst %u",
               SrcContext->Index,
               ThisCtx->Index ));

    Done:
    return ExitCode;
}

static void check_wrap(
    BXPT_RaveCx_Handle DestCtx,
    uint32_t *dest_valid,
    uint32_t *dest_wrap,
    uint32_t **dest_itb_mem
    )
{
    /* use exclusive logic for END and VALID in SW */
    uint32_t dest_end = BREG_Read32(DestCtx->hReg, DestCtx->BaseAddr + ITB_END_PTR_OFFSET ) + 1;

    /* using BXPT_ITB_SIZE as my wraparound threshold is fine. HW RAVE uses another threshold based on its block size for mem bandwidth */
    if (*dest_valid >= dest_end - BXPT_ITB_SIZE)
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

#define INSERT_BASE_ENTRY_ITB(base_address) \
    do { \
    dest_itb[0] = (brave_itb_base_address<<24); \
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
                src_valid = BREG_Read32(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + ITB_VALID_PTR_OFFSET );
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
    uint32_t src_valid,  src_wrap, src_read;
    uint32_t dest_valid, dest_wrap, dest_read;
    uint32_t reg, overflow;

    BDBG_OBJECT_ASSERT(DestCtx, bxpt_t);
    /*BDBG_MSG(( "Advance soft_rave dest context %u", DestCtx->Index ));
      */

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
    src_valid = BREG_Read32(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + CDB_VALID_PTR_OFFSET );
    src_wrap = BREG_Read32(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + CDB_WRAP_PTR_OFFSET);
    dest_read = BREG_Read32(DestCtx->hReg, DestCtx->BaseAddr + CDB_READ_PTR_OFFSET);

    /* produce: advance the dest VALID and WRAP pointers.
    always write WRAP before VALID, always produce on dest before consuming on src */
    BREG_Write32(DestCtx->hReg, DestCtx->BaseAddr + CDB_WRAP_PTR_OFFSET, src_wrap);
    BREG_Write32(DestCtx->hReg, DestCtx->BaseAddr + CDB_VALID_PTR_OFFSET, src_valid);

    /* consume: advance the src READ pointer */
    BREG_Write32(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + CDB_READ_PTR_OFFSET, dest_read);

    /* ITB */
    src_valid = BREG_Read32(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + ITB_VALID_PTR_OFFSET );
    src_wrap = BREG_Read32(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + ITB_WRAP_PTR_OFFSET );
    dest_valid = BREG_Read32(DestCtx->hReg, DestCtx->BaseAddr + ITB_VALID_PTR_OFFSET );
    dest_read = BREG_Read32(DestCtx->hReg, DestCtx->BaseAddr + ITB_READ_PTR_OFFSET );
    dest_wrap = BREG_Read32(DestCtx->hReg, DestCtx->BaseAddr + ITB_WRAP_PTR_OFFSET );

    if (DestCtx->SoftRave.mode == BXPT_RaveSoftMode_ePointersOnly)
    {
        /* do nothing - just copy pointers
        this adds host based flow control. useful for debug, peeking into ITB, possibly modifying in-place.
        no ITB entries can be added/removed. */
        BDBG_MSG(("advance rave ITB: %x, %x, %x, %x", src_valid, src_wrap, dest_read));
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

        /* Update the cached data */
        BMEM_FlushCache(DestCtx->hMem, DestCtx->SoftRave.src_itb_mem, DestCtx->SoftRave.ItbSize );
        BMEM_FlushCache(DestCtx->hMem, DestCtx->SoftRave.dest_itb_mem, DestCtx->SoftRave.ItbSize );

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

#if 0
      BDBG_MSG(("copy ITB: %x --> %x to %x", cur_src_itb, src_valid, dest_valid));
        BDBG_MSG(("DestCtx->SoftRave.src_itb_base %x, DestCtx->SoftRave.dest_itb_base %x",DestCtx->SoftRave.src_itb_base, DestCtx->SoftRave.dest_itb_base));
#endif

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
                    else if (((src_itb[0]&0x00008000)>>15) && (src_itb[2] == 0xFFFFFFFF)){
                dest_itb[0] = 0x00000001; /* 1 Byte Extraction Entry */
                dest_itb[1] = 0x0A0300FF; /* 0F start code, 03 offset from base_address followed by  end of start codes */
                dest_valid += BXPT_ITB_SIZE;
                dest_itb += BXPT_ITB_SIZE/4;
                check_wrap(DestCtx, &dest_valid, &dest_wrap, &dest_itb);
                    }
                    else {
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
            uint32_t itb_type, prev_frame_size;

            itb_type = (src_itb[0]>>24) & 0xFF;

            if((dest_valid<dest_read) && ((dest_valid+(4*BXPT_ITB_SIZE))>dest_read)){
                break;
            }

            switch (itb_type) {
            case brave_itb_base_address:
                if(src_itb[1] >= DestCtx->SoftRave.last_base_address){
                    prev_frame_size = src_itb[1] - DestCtx->SoftRave.last_base_address;
                } else {
                    uint32_t cdb_wrap_ptr, cdb_base_ptr;
                    cdb_base_ptr = BREG_Read32(DestCtx->hReg, CDB_BASE_PTR_OFFSET+DestCtx->SoftRave.SrcBaseAddr);
                    cdb_wrap_ptr = BREG_Read32(DestCtx->hReg, CDB_WRAP_PTR_OFFSET+DestCtx->SoftRave.SrcBaseAddr);
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
                BDBG_MSG(("Inserting Stop PTS for context %p, PTS %u", DestCtx, DestCtx->SoftRave.splice_stop_PTS));
                INSERT_START_STOP_PTS_ITB( 0, DestCtx->SoftRave.splice_stop_PTS );
            }

            /*
            ** Start PTS marker hanlding is similar, except we're using the Start PTS command.
            */
            if( DestCtx->SoftRave.InsertStartPts )
            {
                DestCtx->SoftRave.InsertStartPts = false;
                BDBG_MSG(("Inserting Start PTS for context %p, PTS %u", DestCtx, DestCtx->SoftRave.splice_start_PTS));
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
                        BDBG_MSG(("Inserting Base Address ITB for context %p, Base Address %p, ", DestCtx, DestCtx->SoftRave.last_base_address));
                        INSERT_BASE_ENTRY_ITB(DestCtx->SoftRave.last_base_address);
                        INSERT_PCROFFSET_ITB(DestCtx->SoftRave.splice_last_pcr_offset);
                        /* Insert the start marker into the ITB only once. */
                        if( !DestCtx->SoftRave.StartMarkerInserted )
                        {
                            DestCtx->SoftRave.StartMarkerInserted = true;
                            BDBG_MSG(("Inserting Start marker for context %p, PTS %u", DestCtx, DestCtx->SoftRave.splice_start_PTS));
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
                            BDBG_MSG(("Inserting Termination ITB for context %p, PTS %u", DestCtx, DestCtx->SoftRave.splice_stop_PTS));
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
                        BDBG_MSG(("Inserting Base Address ITB for context %p, Base Address %p, ", DestCtx, DestCtx->SoftRave.last_base_address));
                        INSERT_BASE_ENTRY_ITB(DestCtx->SoftRave.last_base_address);
                        BDBG_MSG(("Got brave_itb_splice_start_marker,  Inserting Start PTS ITB & Start Marker for context %p, PTS %u", DestCtx, src_itb[3]));
                        INSERT_START_STOP_PTS_ITB( 1, src_itb[3]);
                        INSERT_START_MARKER_ITB( 0 );
                            break;

                        case brave_itb_splice_stop_marker:
                            DestCtx->SoftRave.splice_state = SoftRave_SpliceState_Discard;
                        BDBG_MSG(("Got brave_itb_splice_stop_marker,  Inserting Stop PTS ITB & Termination ITB for context %p, PTS %u", DestCtx, src_itb[3]));
                        INSERT_START_STOP_PTS_ITB( 0, src_itb[3]);
                        INSERT_TERMINATION_ITB(src_itb[0] & 0xFF);
                            INSERT_PCROFFSET_ITB(0);
                            break;

                        case brave_itb_splice_pcr_offset_marker:
                            if (src_itb[3] != 0)
                            {
                                DestCtx->SoftRave.splice_pcr_offset +=  src_itb[3];
                            BDBG_MSG(("Got brave_itb_splice_pcr_offset_marker, Context %p, PCR offset %d", DestCtx, DestCtx->SoftRave.splice_pcr_offset));
                                DestCtx->SoftRave.SpliceModifyPCROffsetFlag = true;
                            }else
                            {
                                DestCtx->SoftRave.splice_pcr_offset =  src_itb[3];
                            BDBG_MSG(("Got brave_itb_splice_pcr_offset_marker, Context %p, PCR offset %d", DestCtx, DestCtx->SoftRave.splice_pcr_offset));
                                DestCtx->SoftRave.SpliceModifyPCROffsetFlag = false;
                            }
                            break;

                        case brave_itb_splice_pts_marker:
                            if (src_itb[3] != 0)
                            {
                                DestCtx->SoftRave.splice_pts_offset +=  src_itb[3];
                            BDBG_MSG(("Got brave_itb_splice_pcr_offset_marker, Context %p, PCR offset %d", DestCtx, DestCtx->SoftRave.splice_pts_offset));
                                DestCtx->SoftRave.SpliceModifyPTSFlag = true;
                            }else
                            {
                                DestCtx->SoftRave.splice_pts_offset =  src_itb[3];
                            BDBG_MSG(("Got brave_itb_splice_pcr_offset_marker, Context %p, PCR offset %d", DestCtx, DestCtx->SoftRave.splice_pts_offset));
                                DestCtx->SoftRave.SpliceModifyPTSFlag = false;
                            }
                            break;

                        default:
                            COPY_ITB();
                            break;

                    }
                }
                break;

                default:
                    if (DestCtx->SoftRave.splice_state ==  SoftRave_SpliceState_Copy)
                    {
                        COPY_ITB();
                    }
                if (itb_type == brave_itb_base_address )
                {
                    DestCtx->SoftRave.last_base_address =  src_itb[1];
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
                src_valid = BREG_Read32(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + ITB_VALID_PTR_OFFSET ) + 1;
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
    BMEM_FlushCache(DestCtx->hMem, DestCtx->SoftRave.dest_itb_mem, DestCtx->SoftRave.ItbSize );

    /* produce: advance the dest VALID and WRAP pointers */
    BREG_Write32(DestCtx->hReg, DestCtx->BaseAddr + ITB_WRAP_PTR_OFFSET, dest_wrap);
    BREG_Write32(DestCtx->hReg, DestCtx->BaseAddr + ITB_VALID_PTR_OFFSET, dest_valid);

    /* consume: advance the src READ pointer */
    BREG_Write32(DestCtx->hReg, DestCtx->SoftRave.SrcBaseAddr + ITB_READ_PTR_OFFSET, src_read);
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

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);

    BKNI_Memset( (void *) &hCtx->SoftRave, 0, sizeof( hCtx->SoftRave ));

    hCtx->SoftRave.SrcContextIndex = SourceContextIndex;
    hCtx->SoftRave.ItbSize = ItbSize;
    hCtx->SoftRave.SrcBaseAddr = SrcBaseAddr;
    hCtx->SoftRave.mode = Mode;
    hCtx->SoftRave.last_base_address = BREG_Read32(hCtx->hReg, CDB_BASE_PTR_OFFSET+hCtx->SoftRave.SrcBaseAddr);

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
    hCtx->SoftRave.SpliceModifyPTSFlag = false;
    hCtx->SoftRave.SpliceStartPTSCB = NULL;
    hCtx->SoftRave.SpliceMonitorPTSCB = NULL;
    hCtx->SoftRave.SpliceStopPTSCB = NULL;
    hCtx->SoftRave.InsertStartPts = false;
    hCtx->SoftRave.InsertStopPts = false;
    hCtx->SoftRave.StartMarkerInserted = false;
    hCtx->SoftRave.StopMarkerInserted = false;

    return BXPT_Rave_FlushContext( hCtx );
}

BERR_Code BXPT_Rave_StopPTS(
    BXPT_RaveCx_Handle hCtx,
    uint32_t StopPTS, uint32_t tolerance,
    void  StopPTSCb( void *, uint32_t pts),
    void * param
    )
{
    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);

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
    void  StartPTSCb(void *,uint32_t pts),
    void * param
    )
{
    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);

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
BERR_Code BXPT_Rave_Monitor_PTS(
    BXPT_RaveCx_Handle hCtx,
    uint32_t PTS,uint32_t tolerance,
    void  SpliceMonitorPTSCB(void *, uint32_t pts),
    void * param
    )
{
    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
    BSTD_UNUSED( param );

    if (PTS == 0 && (hCtx->SoftRave.SpliceMonitorPTSFlag == true) )
    {
    hCtx->SoftRave.splice_monitor_PTS = PTS;
        hCtx->SoftRave.splice_monitor_PTS_tolerance = 0;
        hCtx->SoftRave.SpliceMonitorPTSFlag = false;
        hCtx->SoftRave.SpliceMonitorPTSCBParam = NULL;
        hCtx->SoftRave.SpliceMonitorPTSCB = NULL;
    }else
    {
        hCtx->SoftRave.splice_monitor_PTS = PTS;
        hCtx->SoftRave.splice_monitor_PTS_tolerance = tolerance;
    hCtx->SoftRave.SpliceMonitorPTSFlag = true;
    hCtx->SoftRave.SpliceMonitorPTSCBParam = param;
    hCtx->SoftRave.SpliceMonitorPTSCB = SpliceMonitorPTSCB;
    }
    return BERR_SUCCESS;
}
BERR_Code BXPT_Rave_Cancel_PTS(  BXPT_RaveCx_Handle hCtx  )
{
    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
    hCtx->SoftRave.splice_monitor_PTS = 0;
    hCtx->SoftRave.SpliceMonitorPTSFlag = false;
    hCtx->SoftRave.SpliceMonitorPTSCB = NULL;
    return BERR_SUCCESS;
}
BERR_Code BXPT_Rave_SetPCROffset(BXPT_RaveCx_Handle hCtx  , uint32_t pcr_offset)
{
    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);
    hCtx->SoftRave.splice_last_pcr_offset = pcr_offset;
    return BERR_SUCCESS;
}

static void SetBandEnBit(
    BREG_Handle hReg,
    unsigned RaveContextNo,
    bool BandEn
    )
{
    uint32_t RegVal;

    uint32_t BaseAddr = BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR + ( RaveContextNo * RAVE_CONTEXT_REG_STEP );

    RegVal = BREG_Read32( hReg, BaseAddr + AV_MISC_CFG1_OFFSET );
    RegVal &= ~(
        BCHP_MASK( XPT_RAVE_CX0_AV_MISC_CONFIG1, BAND_HOLD_EN )
    );
    RegVal |= (
        BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_MISC_CONFIG1, BAND_HOLD_EN, BandEn == true ? 1 : 0 )
    );
    BREG_Write32( hReg, BaseAddr + AV_MISC_CFG1_OFFSET, RegVal );

    /* If clearing the hold enable, clear the hold as well. */
    if( !BandEn )
    {
        uint32_t MyIndexBit = 1UL << RaveContextNo;

        BREG_Write32( hReg, BCHP_XPT_RAVE_CX_HOLD_CLR_STATUS, MyIndexBit );
    }
}

void BXPT_Rave_P_SetBandHold(
    BXPT_Handle hXpt,
    unsigned PbChannelNo,
    uint32_t BandHoldStatus,
    bool SetHoldEn
    )
{
    unsigned PidChannel;
    uint32_t BandHold;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Walk through the PID channel table. */
    for( PidChannel = 0; PidChannel < hXpt->MaxPidChannels; PidChannel++ )
    {
        uint32_t FeSel, DestSel, SourceBand;

        uint32_t PidCfg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannel * 4 ) );
        uint32_t SpidCfg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( PidChannel * 4 ) );

        FeSel = BCHP_GET_FIELD_DATA( PidCfg, XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL );
#if (BCHP_CHIP == 7630 && BCHP_VER >= BCHP_VER_B2 )
        SourceBand = BCHP_GET_FIELD_DATA( PidCfg, XPT_FE_PID_TABLE_i, PID_CHANNEL_INPUT_SELECT );
#else
        SourceBand = BCHP_GET_FIELD_DATA( PidCfg, XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT );
#endif
        DestSel = BCHP_GET_FIELD_DATA( SpidCfg, XPT_FE_SPID_TABLE_i, PID_DESTINATION );

        /* Is this PID channel going to the RAVE, and is it mapped to the given PB channel? */
        if( (DestSel & 0x30 ) && FeSel && SourceBand == PbChannelNo )
        {
            uint32_t LoCxMemEnables, HiCxMemEnables, RegAddr1, DummyRead;
            unsigned RaveContextNo, EnableMask;

            /* Get the CXMEM HI and LO for this PID channel */
            BKNI_EnterCriticalSection();
            RegAddr1 = BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE + ( PidChannel * CXMEM_CHNL_STEPSIZE );
            LoCxMemEnables = BREG_Read32( hXpt->hRegister, RegAddr1 );
            RegAddr1 = BCHP_XPT_RAVE_CXMEM_HIi_ARRAY_BASE + ( PidChannel * CXMEM_CHNL_STEPSIZE );
            HiCxMemEnables = BREG_Read32( hXpt->hRegister, RegAddr1 );
            DummyRead = BREG_Read32( hXpt->hRegister, BCHP_XPT_RAVE_CXMEM_LOi_ARRAY_BASE );   /* PR 50099 */
            BKNI_LeaveCriticalSection();

            /* Walk through the number of rave contexts, building the enable bitmask for each context */
            for( RaveContextNo = 0; RaveContextNo < CXMEM_LO_MAX_CONTEXT; RaveContextNo++ )
            {
                EnableMask = 0x3 << (RaveContextNo * 2);          /* 0x3 since it could be either the R or G pipe. */

                /* Is this rave context using this PID channel ? */
                if( EnableMask & LoCxMemEnables )
                {
                    /*If BAND_HOLD_EN was previously false, keep it false. SW7335-664*/
                    BandHold = BandHoldStatus >> RaveContextNo;
                    if(BandHold & 0x1)
                        SetBandEnBit( hXpt->hRegister, RaveContextNo, SetHoldEn );
                    else
                        SetBandEnBit( hXpt->hRegister, RaveContextNo, false );
                }
            }

            for( RaveContextNo = CXMEM_LO_MAX_CONTEXT; RaveContextNo < CXMEM_HI_MAX_CONTEXT; RaveContextNo++ )
            {
                EnableMask = 0x3 << ((RaveContextNo - CXMEM_LO_MAX_CONTEXT) * 2);          /* 0x3 since it could be either the R or G pipe. */

                /* Is this rave context using this PID channel ? */
                if( EnableMask & HiCxMemEnables )
                {
                    /*If BAND_HOLD_EN was previously false, keep it false. SW7335-664*/
                    BandHold = BandHoldStatus >> RaveContextNo;
                    if(BandHold & 0x1)
                        SetBandEnBit( hXpt->hRegister, RaveContextNo, SetHoldEn );
                    else
                        SetBandEnBit( hXpt->hRegister, RaveContextNo, false );
                }
            }
        }
    }
}

void BXPT_Rave_P_GetBandHold(
    BXPT_Handle hXpt,
    uint32_t *BandHoldStatus
    )
{
    uint32_t Reg;
    uint32_t BaseAddr;
    unsigned RaveContextNo;
    uint32_t BandHold = 0;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    for (RaveContextNo = 0; RaveContextNo < CXMEM_HI_MAX_CONTEXT; RaveContextNo++) {
        BaseAddr = BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR + ( RaveContextNo * RAVE_CONTEXT_REG_STEP );
        Reg = BREG_Read32( hXpt->hRegister, BaseAddr + AV_MISC_CFG1_OFFSET );
        BandHold = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_CX0_AV_MISC_CONFIG1,  BAND_HOLD_EN );
        *BandHoldStatus |= BandHold << RaveContextNo;
    }
}

BERR_Code BXPT_Rave_AdjustCdbLength(
    BXPT_RaveCx_Handle hCtx,
    size_t CdbLength
    )
{
    uint32_t Base, Read, Write, Valid;
    BXPT_Rave_ContextThresholds Thresholds;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hCtx, bxpt_t);

    Base  = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_BASE_PTR_OFFSET );
    Read  = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_READ_PTR_OFFSET );
    Write = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_WRITE_PTR_OFFSET );
    Valid = BREG_Read32( hCtx->hReg, hCtx->BaseAddr + CDB_VALID_PTR_OFFSET );

    /* Sanity check; no can do unless pointers are in reset state */
    if ( Base && (Base == Read) && (Base == Write) && (Base == Valid) )
    {
        BREG_Write32( hCtx->hReg, hCtx->BaseAddr + CDB_END_PTR_OFFSET, Base + CdbLength - 1 );

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

static BERR_Code GetNextFreeScd(
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
        for( ; ScdNum < BXPT_P_MAX_SCD - 2; ScdNum++ )
        {
            if( hRave->ScdTable[ ScdNum ].Allocated == false
            && hRave->ScdTable[ ScdNum + 1 ].Allocated == false )
                break;
        }

        if( ScdNum == BXPT_P_MAX_SCD - 1 )
        {
            BDBG_ERR(( "No SCD channel is available for this VCT context" ));
            ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        }
    }
    else
    {
        for( ; ScdNum < BXPT_P_MAX_SCD; ScdNum++ )
        {
            if( hRave->ScdTable[ ScdNum ].Allocated == false )
                break;
        }

        if( ScdNum == BXPT_P_MAX_SCD )
        {
            BDBG_ERR(( "No SCD channel is available for this AV context" ));
            ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        }
    }

    *ReturnedScdNum = ScdNum;
    return ExitCode;
}

static BERR_Code GetNextRaveContext(
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
        /* Need to find two consecutive free contexts */
        for( ; Index < BXPT_P_MAX_RAVE_CONTEXTS - 2; Index++ )
        {
            if( hRave->ContextTbl[ Index ].Allocated == false
            && hRave->ContextTbl[ Index + 1 ].Allocated == false )
                break;
        }

        if( Index == BXPT_P_MAX_RAVE_CONTEXTS - 1 )
        {
            BDBG_ERR(( "No VCT context is available" ));
            ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        }
    }
    else
    {
        for( ; Index < BXPT_P_MAX_RAVE_CONTEXTS; Index++ )
        {
            if( hRave->ContextTbl[ Index ].Allocated == false )
                break;
        }

        if( Index == BXPT_P_MAX_RAVE_CONTEXTS )
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

    for( Index = 0; Index < BXPT_P_MAX_RAVE_CONTEXTS; Index++ )
    {
        if( hRave->ContextTbl[ Index ].Allocated == false )
            break;
    }

    if( Index == BXPT_P_MAX_RAVE_CONTEXTS )
    {
        BDBG_ERR(( "No context is available" ));
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
    }

    *ReturnedIndex = Index;
    return ExitCode;
}

static BERR_Code MapGranularityToHw(
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
static uint32_t GetGranularityInBytes(
    BXPT_RaveCx_Handle hCtx
    )
{
    uint32_t Reg, Watermark;
    Reg = BREG_Read32( hCtx->hReg, BCHP_XPT_RAVE_MISC_CONTROL );
    Watermark = BCHP_GET_FIELD_DATA( Reg, XPT_RAVE_MISC_CONTROL, WMARK_GRANULARITY );
    return 32 << Watermark;
}
static size_t GetMaxBufferByGranularity(
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
    Status->SupportedContexts = BXPT_P_MAX_RAVE_CONTEXTS;
    for( Index = 0; Index < BXPT_P_MAX_RAVE_CONTEXTS; Index++ )
    {
        if( hRave->ContextTbl[ Index ].Allocated )
            Status->AllocatedContexts++;
    }
}


void BXPT_Rave_GetFwRevisionInfo(
    BXPT_Rave_Handle hRave,                 /* [in] Handle for this RAVE channel */
    BXPT_Rave_RevisionInfo *versionInfo     /* [out] Version info */
    )
{
    BSTD_UNUSED( hRave );
    BDBG_ASSERT( versionInfo );

    versionInfo->fwRev = BxptRaveInitData[ BxptRaveInitDataSize - 1 ];
    versionInfo->fwCrc = BxptRaveInitData[ BxptRaveInitDataSize - 2 ];
}
