/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef _BAST_4528_H__
#define _BAST_4528_H__               

#if (BCHP_CHIP != 4528)
#error "This file is for BCM4528 firmware only (not for host software)"
#endif

#include "bchp_leap_l1.h"
#include "bchp_afec_0.h"
#include "bchp_afec_global_0.h"
#include "bchp_afec_gr_bridge_0.h"
#include "bchp_afec_intr_ctrl2_0.h"
#include "bchp_sds_bert_0.h"
#include "bchp_sds_bl_0.h"
#include "bchp_sds_cg_0.h"
#include "bchp_sds_cl_0.h"
#include "bchp_sds_dft_0.h"
#include "bchp_sds_dsec.h"
#include "bchp_sds_eq_0.h"
#include "bchp_sds_fe_0.h"
#include "bchp_sds_fec_0.h"
#include "bchp_sds_gr_bridge_0.h"
#include "bchp_sds_hp_0.h"
#include "bchp_sds_intr2_0_0.h"
#include "bchp_sds_misc_0.h"
#include "bchp_sds_ntch_0.h"
#include "bchp_sds_oi_0.h"
#include "bchp_sds_snr_0.h"
#include "bchp_sds_vit_0.h"
#include "bchp_ftm_phy_ana.h"
#include "bchp_ftm_intr2.h"
#include "bchp_ftm_phy.h"
#include "bchp_ftm_skit.h"
#include "bchp_ftm_uart.h"
#include "bchp_sds_dsec_intr2.h"


#define BAST_LEAP_INT_ID_CREATE(L1,L2) ((L1<<8)|L2)

#define BCHP_INT_ID_SDS_sar_vol_gt_hi_thrsh_0   BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT, BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_sar_vol_gt_thresh_SHIFT)
#define BCHP_INT_ID_SDS_sar_vol_lt_lo_thrsh_0   BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT, BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_sar_vol_lt_thresh_SHIFT)
#define BCHP_INT_ID_DSDN_IS_0                   BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT, BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_done_SHIFT)
#define BCHP_INT_ID_DISEQC_TIMER1_0             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT, BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_timer2_SHIFT)
#define BCHP_INT_ID_DISEQC_TIMER2_0             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT, BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_timer1_SHIFT)
#define BCHP_INT_ID_DISEQC_tx_fifo_a_empty_0    BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT, BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_tx_fifo_a_empty_SHIFT)

#define BCHP_INT_ID_SDS_LOCK_IS_0               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS0_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_in_lock_SHIFT)
#define BCHP_INT_ID_SDS_NOT_LOCK_IS_0           BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS0_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_out_lock_SHIFT)
#define BCHP_INT_ID_SDS_BTM_IS_0                BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS0_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bclktimer_int_SHIFT)
#define BCHP_INT_ID_SDS_BRTM_IS_0               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS0_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bertimer_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS1_0             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS0_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer1_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS2_0             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS0_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer2_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS3_0             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS0_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer3_int_SHIFT)
#define BCHP_INT_ID_SDS_HP_IS_0                 BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS0_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_state_match_SHIFT)
#define BCHP_INT_ID_MI2C_IS_0                   BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS0_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_mi2c_int_SHIFT)
#define BCHP_INT_ID_HP_FRAME_BOUNDARY_0         BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS0_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_frame_boundary_SHIFT)

#define BCHP_INT_ID_SDS_LOCK_IS_1               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS1_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_in_lock_SHIFT)
#define BCHP_INT_ID_SDS_NOT_LOCK_IS_1           BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS1_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_out_lock_SHIFT)
#define BCHP_INT_ID_SDS_BTM_IS_1                BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS1_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bclktimer_int_SHIFT)
#define BCHP_INT_ID_SDS_BRTM_IS_1               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS1_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bertimer_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS1_1             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS1_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer1_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS2_1             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS1_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer2_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS3_1             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS1_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer3_int_SHIFT)
#define BCHP_INT_ID_SDS_HP_IS_1                 BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS1_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_state_match_SHIFT)
#define BCHP_INT_ID_MI2C_IS_1                   BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS1_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_mi2c_int_SHIFT)
#define BCHP_INT_ID_HP_FRAME_BOUNDARY_1         BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS1_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_frame_boundary_SHIFT)

#define BCHP_INT_ID_SDS_LOCK_IS_2               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS2_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_in_lock_SHIFT)
#define BCHP_INT_ID_SDS_NOT_LOCK_IS_2           BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS2_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_out_lock_SHIFT)
#define BCHP_INT_ID_SDS_BTM_IS_2                BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS2_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bclktimer_int_SHIFT)
#define BCHP_INT_ID_SDS_BRTM_IS_2               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS2_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bertimer_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS1_2             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS2_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer1_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS2_2             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS2_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer2_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS3_2             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS2_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer3_int_SHIFT)
#define BCHP_INT_ID_SDS_HP_IS_2                 BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS2_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_state_match_SHIFT)
#define BCHP_INT_ID_MI2C_IS_2                   BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS2_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_mi2c_int_SHIFT)
#define BCHP_INT_ID_HP_FRAME_BOUNDARY_2         BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS2_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_frame_boundary_SHIFT)

#define BCHP_INT_ID_SDS_LOCK_IS_3               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS3_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_in_lock_SHIFT)
#define BCHP_INT_ID_SDS_NOT_LOCK_IS_3           BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS3_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_out_lock_SHIFT)
#define BCHP_INT_ID_SDS_BTM_IS_3                BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS3_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bclktimer_int_SHIFT)
#define BCHP_INT_ID_SDS_BRTM_IS_3               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS3_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bertimer_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS1_3             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS3_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer1_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS2_3             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS3_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer2_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS3_3             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS3_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer3_int_SHIFT)
#define BCHP_INT_ID_SDS_HP_IS_3                 BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS3_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_state_match_SHIFT)
#define BCHP_INT_ID_MI2C_IS_3                   BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS3_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_mi2c_int_SHIFT)
#define BCHP_INT_ID_HP_FRAME_BOUNDARY_3         BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS3_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_frame_boundary_SHIFT)

#define BCHP_INT_ID_SDS_LOCK_IS_4               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS4_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_in_lock_SHIFT)
#define BCHP_INT_ID_SDS_NOT_LOCK_IS_4           BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS4_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_out_lock_SHIFT)
#define BCHP_INT_ID_SDS_BTM_IS_4                BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS4_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bclktimer_int_SHIFT)
#define BCHP_INT_ID_SDS_BRTM_IS_4               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS4_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bertimer_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS1_4             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS4_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer1_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS2_4             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS4_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer2_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS3_4             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS4_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer3_int_SHIFT)
#define BCHP_INT_ID_SDS_HP_IS_4                 BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS4_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_state_match_SHIFT)
#define BCHP_INT_ID_MI2C_IS_4                   BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS4_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_mi2c_int_SHIFT)
#define BCHP_INT_ID_HP_FRAME_BOUNDARY_4         BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS4_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_frame_boundary_SHIFT)

#define BCHP_INT_ID_SDS_LOCK_IS_5               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS5_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_in_lock_SHIFT)
#define BCHP_INT_ID_SDS_NOT_LOCK_IS_5           BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS5_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_out_lock_SHIFT)
#define BCHP_INT_ID_SDS_BTM_IS_5                BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS5_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bclktimer_int_SHIFT)
#define BCHP_INT_ID_SDS_BRTM_IS_5               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS5_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bertimer_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS1_5             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS5_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer1_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS2_5             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS5_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer2_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS3_5             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS5_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer3_int_SHIFT)
#define BCHP_INT_ID_SDS_HP_IS_5                 BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS5_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_state_match_SHIFT)
#define BCHP_INT_ID_MI2C_IS_5                   BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS5_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_mi2c_int_SHIFT)
#define BCHP_INT_ID_HP_FRAME_BOUNDARY_5         BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS5_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_frame_boundary_SHIFT)

#define BCHP_INT_ID_SDS_LOCK_IS_6               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS6_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_in_lock_SHIFT)
#define BCHP_INT_ID_SDS_NOT_LOCK_IS_6           BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS6_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_out_lock_SHIFT)
#define BCHP_INT_ID_SDS_BTM_IS_6                BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS6_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bclktimer_int_SHIFT)
#define BCHP_INT_ID_SDS_BRTM_IS_6               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS6_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bertimer_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS1_6             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS6_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer1_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS2_6             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS6_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer2_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS3_6             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS6_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer3_int_SHIFT)
#define BCHP_INT_ID_SDS_HP_IS_6                 BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS6_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_state_match_SHIFT)
#define BCHP_INT_ID_MI2C_IS_6                   BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS6_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_mi2c_int_SHIFT)
#define BCHP_INT_ID_HP_FRAME_BOUNDARY_6         BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS6_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_frame_boundary_SHIFT)

#define BCHP_INT_ID_SDS_LOCK_IS_7               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS7_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_in_lock_SHIFT)
#define BCHP_INT_ID_SDS_NOT_LOCK_IS_7           BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS7_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_rvb_out_lock_SHIFT)
#define BCHP_INT_ID_SDS_BTM_IS_7                BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS7_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bclktimer_int_SHIFT)
#define BCHP_INT_ID_SDS_BRTM_IS_7               BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS7_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_bertimer_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS1_7             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS7_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer1_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS2_7             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS7_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer2_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS3_7             BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS7_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_gentimer3_int_SHIFT)
#define BCHP_INT_ID_SDS_HP_IS_7                 BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS7_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_state_match_SHIFT)
#define BCHP_INT_ID_MI2C_IS_7                   BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS7_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_mi2c_int_SHIFT)
#define BCHP_INT_ID_HP_FRAME_BOUNDARY_7         BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS7_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_hp_frame_boundary_SHIFT)

#define BCHP_INT_ID_AFEC_LOCK_IS_0              BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS0_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_in_lock_SHIFT)
#define BCHP_INT_ID_AFEC_NOT_LOCK_IS_0          BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS0_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_out_lock_SHIFT)
#define BCHP_INT_ID_AFEC_LOCK_IS_1              BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS1_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_in_lock_SHIFT)
#define BCHP_INT_ID_AFEC_NOT_LOCK_IS_1          BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS1_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_out_lock_SHIFT)
#define BCHP_INT_ID_AFEC_LOCK_IS_2              BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS2_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_in_lock_SHIFT)
#define BCHP_INT_ID_AFEC_NOT_LOCK_IS_2          BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS2_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_out_lock_SHIFT)
#define BCHP_INT_ID_AFEC_LOCK_IS_3              BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS3_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_in_lock_SHIFT)
#define BCHP_INT_ID_AFEC_NOT_LOCK_IS_3          BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS3_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_out_lock_SHIFT)
#define BCHP_INT_ID_AFEC_LOCK_IS_4              BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS4_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_in_lock_SHIFT)
#define BCHP_INT_ID_AFEC_NOT_LOCK_IS_4          BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS4_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_out_lock_SHIFT)
#define BCHP_INT_ID_AFEC_LOCK_IS_5              BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS5_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_in_lock_SHIFT)
#define BCHP_INT_ID_AFEC_NOT_LOCK_IS_5          BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS5_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_out_lock_SHIFT)
#define BCHP_INT_ID_AFEC_LOCK_IS_6              BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS6_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_in_lock_SHIFT)
#define BCHP_INT_ID_AFEC_NOT_LOCK_IS_6          BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS6_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_out_lock_SHIFT)
#define BCHP_INT_ID_AFEC_LOCK_IS_7              BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS7_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_in_lock_SHIFT)
#define BCHP_INT_ID_AFEC_NOT_LOCK_IS_7          BAST_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_STATUS_SDS7_INTR_SHIFT, BCHP_SDS_INTR2_0_0_CPU_STATUS_ahb_out_lock_SHIFT)

#define BAST_G3_MAX_CHANNELS 8
#define BAST_G3_XTAL_FREQ    54000000
#define BAST_G3_CHIP         0x4528
#define BAST_G3_MAJOR_VERSION 0x00
#define BAST_G3_MINOR_VERSION 0x01

#define BAST_ADC_FREQ_KHZ        4968000 /* this must match BWFE_DEF_FS_ADC_KHZ */
#define BAST_DEFAULT_SAMPLE_FREQ ((BAST_ADC_FREQ_KHZ>>5)*1000)


/******************************************************************************
 define register names common to all channels
******************************************************************************/
#define BCHP_AFEC_RST BCHP_AFEC_0_RST
#define BCHP_AFEC_CNTR_CTRL BCHP_AFEC_0_CNTR_CTRL
#define BCHP_AFEC_TEST_CONFIG BCHP_AFEC_0_TEST_CONFIG
#define BCHP_AFEC_ACM_MAX_ITER_OVERIDE BCHP_AFEC_0_ACM_MAX_ITER_OVERIDE
#define BCHP_AFEC_ACM_MODCOD_1 BCHP_AFEC_0_ACM_MODCOD_1
#define BCHP_AFEC_ACM_MODCOD_2 BCHP_AFEC_0_ACM_MODCOD_2
#define BCHP_AFEC_ACM_MODCOD_3 BCHP_AFEC_0_ACM_MODCOD_3
#define BCHP_AFEC_ACM_MODCOD_4 BCHP_AFEC_0_ACM_MODCOD_4
#define BCHP_AFEC_ACM_MODCOD_5 BCHP_AFEC_0_ACM_MODCOD_5
#define BCHP_AFEC_ACM_MODCOD_6 BCHP_AFEC_0_ACM_MODCOD_6
#define BCHP_AFEC_ACM_MODCOD_7 BCHP_AFEC_0_ACM_MODCOD_7
#define BCHP_AFEC_ACM_MODCOD_8 BCHP_AFEC_0_ACM_MODCOD_8
#define BCHP_AFEC_ACM_MODCOD_9 BCHP_AFEC_0_ACM_MODCOD_9
#define BCHP_AFEC_ACM_MODCOD_10 BCHP_AFEC_0_ACM_MODCOD_10
#define BCHP_AFEC_ACM_MODCOD_11 BCHP_AFEC_0_ACM_MODCOD_11
#define BCHP_AFEC_ACM_MODCOD_12 BCHP_AFEC_0_ACM_MODCOD_12
#define BCHP_AFEC_ACM_MODCOD_13 BCHP_AFEC_0_ACM_MODCOD_13
#define BCHP_AFEC_ACM_MODCOD_14 BCHP_AFEC_0_ACM_MODCOD_14
#define BCHP_AFEC_ACM_MODCOD_15 BCHP_AFEC_0_ACM_MODCOD_15
#define BCHP_AFEC_ACM_MODCOD_16 BCHP_AFEC_0_ACM_MODCOD_16
#define BCHP_AFEC_ACM_MODCOD_17 BCHP_AFEC_0_ACM_MODCOD_17
#define BCHP_AFEC_ACM_MODCOD_18 BCHP_AFEC_0_ACM_MODCOD_18
#define BCHP_AFEC_ACM_MODCOD_19 BCHP_AFEC_0_ACM_MODCOD_19
#define BCHP_AFEC_ACM_MODCOD_20 BCHP_AFEC_0_ACM_MODCOD_20
#define BCHP_AFEC_ACM_MODCOD_21 BCHP_AFEC_0_ACM_MODCOD_21
#define BCHP_AFEC_ACM_MODCOD_22 BCHP_AFEC_0_ACM_MODCOD_22
#define BCHP_AFEC_ACM_MODCOD_23 BCHP_AFEC_0_ACM_MODCOD_23
#define BCHP_AFEC_ACM_MODCOD_24 BCHP_AFEC_0_ACM_MODCOD_24
#define BCHP_AFEC_ACM_MODCOD_25 BCHP_AFEC_0_ACM_MODCOD_25
#define BCHP_AFEC_ACM_MODCOD_26 BCHP_AFEC_0_ACM_MODCOD_26
#define BCHP_AFEC_ACM_MODCOD_27 BCHP_AFEC_0_ACM_MODCOD_27
#define BCHP_AFEC_ACM_MODCOD_28 BCHP_AFEC_0_ACM_MODCOD_28
#define BCHP_AFEC_ACM_MODCOD_29_EXT BCHP_AFEC_0_ACM_MODCOD_29_EXT
#define BCHP_AFEC_ACM_MODCOD_29_LDPC0_EXT BCHP_AFEC_0_ACM_MODCOD_29_LDPC0_EXT
#define BCHP_AFEC_ACM_MODCOD_30_EXT BCHP_AFEC_0_ACM_MODCOD_30_EXT
#define BCHP_AFEC_ACM_MODCOD_30_LDPC0_EXT BCHP_AFEC_0_ACM_MODCOD_30_LDPC0_EXT
#define BCHP_AFEC_ACM_MODCOD_31_EXT BCHP_AFEC_0_ACM_MODCOD_31_EXT
#define BCHP_AFEC_ACM_MODCOD_31_LDPC0_EXT BCHP_AFEC_0_ACM_MODCOD_31_LDPC0_EXT
#define BCHP_AFEC_ACM_SYM_CNT_0 BCHP_AFEC_0_ACM_SYM_CNT_0
#define BCHP_AFEC_ACM_SYM_CNT_1 BCHP_AFEC_0_ACM_SYM_CNT_1
#define BCHP_AFEC_ACM_SYM_CNT_2 BCHP_AFEC_0_ACM_SYM_CNT_2
#define BCHP_AFEC_ACM_CYCLE_CNT_0 BCHP_AFEC_0_ACM_CYCLE_CNT_0
#define BCHP_AFEC_ACM_CYCLE_CNT_1 BCHP_AFEC_0_ACM_CYCLE_CNT_1
#define BCHP_AFEC_ACM_CYCLE_CNT_2 BCHP_AFEC_0_ACM_CYCLE_CNT_2
#define BCHP_AFEC_ACM_CYCLE_CNT_3 BCHP_AFEC_0_ACM_CYCLE_CNT_3
#define BCHP_AFEC_ACM_CYCLE_CNT_4 BCHP_AFEC_0_ACM_CYCLE_CNT_4
#define BCHP_AFEC_ACM_MISC_0 BCHP_AFEC_0_ACM_MISC_0
#define BCHP_AFEC_ACM_MISC_1 BCHP_AFEC_0_ACM_MISC_1
#define BCHP_AFEC_ACM_MODCOD_OVERIDE BCHP_AFEC_0_ACM_MODCOD_OVERIDE
#define BCHP_AFEC_ACM_MODCOD_STATS_CONFIG BCHP_AFEC_0_ACM_MODCOD_STATS_CONFIG
#define BCHP_AFEC_ACM_DONT_DEC_CNT BCHP_AFEC_0_ACM_DONT_DEC_CNT
#define BCHP_AFEC_ACM_LDPC_ITER_CNT BCHP_AFEC_0_ACM_LDPC_ITER_CNT
#define BCHP_AFEC_ACM_LDPC_FAIL_CNT BCHP_AFEC_0_ACM_LDPC_FAIL_CNT
#define BCHP_AFEC_ACM_LDPC_FRM_CNT BCHP_AFEC_0_ACM_LDPC_FRM_CNT
#define BCHP_AFEC_LDPC_CONFIG_0 BCHP_AFEC_0_LDPC_CONFIG_0
#define BCHP_AFEC_LDPC_STATUS BCHP_AFEC_0_LDPC_STATUS
#define BCHP_AFEC_LDPC_MET_CRC BCHP_AFEC_0_LDPC_MET_CRC
#define BCHP_AFEC_LDPC_EDGE_CRC BCHP_AFEC_0_LDPC_EDGE_CRC
#define BCHP_AFEC_LDPC_PSL_CTL BCHP_AFEC_0_LDPC_PSL_CTL
#define BCHP_AFEC_LDPC_PSL_INT_THRES BCHP_AFEC_0_LDPC_PSL_INT_THRES
#define BCHP_AFEC_LDPC_PSL_INT BCHP_AFEC_0_LDPC_PSL_INT
#define BCHP_AFEC_LDPC_PSL_AVE BCHP_AFEC_0_LDPC_PSL_AVE
#define BCHP_AFEC_LDPC_PSL_XCS BCHP_AFEC_0_LDPC_PSL_XCS
#define BCHP_AFEC_LDPC_PSL_FILTER BCHP_AFEC_0_LDPC_PSL_FILTER
#define BCHP_AFEC_LDPC_MEM_POWER BCHP_AFEC_0_LDPC_MEM_POWER
#define BCHP_AFEC_BCH_TPCTL BCHP_AFEC_0_BCH_TPCTL
#define BCHP_AFEC_BCH_TPSIG BCHP_AFEC_0_BCH_TPSIG
#define BCHP_AFEC_BCH_CTRL BCHP_AFEC_0_BCH_CTRL
#define BCHP_AFEC_BCH_DECNBLK BCHP_AFEC_0_BCH_DECNBLK
#define BCHP_AFEC_BCH_DECCBLK BCHP_AFEC_0_BCH_DECCBLK
#define BCHP_AFEC_BCH_DECBBLK BCHP_AFEC_0_BCH_DECBBLK
#define BCHP_AFEC_BCH_DECCBIT BCHP_AFEC_0_BCH_DECCBIT
#define BCHP_AFEC_BCH_DECMCOR BCHP_AFEC_0_BCH_DECMCOR
#define BCHP_AFEC_BCH_BBHDR0 BCHP_AFEC_0_BCH_BBHDR0
#define BCHP_AFEC_BCH_BBHDR1 BCHP_AFEC_0_BCH_BBHDR1
#define BCHP_AFEC_BCH_BBHDR2 BCHP_AFEC_0_BCH_BBHDR2
#define BCHP_AFEC_BCH_BBHDR3 BCHP_AFEC_0_BCH_BBHDR3
#define BCHP_AFEC_BCH_BBHDR4 BCHP_AFEC_0_BCH_BBHDR4
#define BCHP_AFEC_BCH_BBHDR5 BCHP_AFEC_0_BCH_BBHDR5
#define BCHP_AFEC_BCH_MPLCK BCHP_AFEC_0_BCH_MPLCK
#define BCHP_AFEC_BCH_MPCFG BCHP_AFEC_0_BCH_MPCFG
#define BCHP_AFEC_BCH_SMCFG BCHP_AFEC_0_BCH_SMCFG

#define BCHP_AFEC_GLOBAL_CLK_CNTRL BCHP_AFEC_GLOBAL_0_CLK_CNTRL
#define BCHP_AFEC_GLOBAL_PWR_CNTRL BCHP_AFEC_GLOBAL_0_PWR_CNTRL
#define BCHP_AFEC_GLOBAL_SW_SPARE0 BCHP_AFEC_GLOBAL_0_SW_SPARE0
#define BCHP_AFEC_GLOBAL_SW_SPARE1 BCHP_AFEC_GLOBAL_0_SW_SPARE1

#define BCHP_AFEC_GR_BRIDGE_REVISION BCHP_AFEC_GR_BRIDGE_0_REVISION
#define BCHP_AFEC_GR_BRIDGE_CTRL BCHP_AFEC_GR_BRIDGE_0_CTRL
#define BCHP_AFEC_GR_BRIDGE_SW_RESET_0 BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0
#define BCHP_AFEC_GR_BRIDGE_SW_RESET_1 BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1

#define BCHP_AFEC_INTR_CTRL2_CPU_STATUS BCHP_AFEC_INTR_CTRL2_0_CPU_STATUS
#define BCHP_AFEC_INTR_CTRL2_CPU_SET BCHP_AFEC_INTR_CTRL2_0_CPU_SET
#define BCHP_AFEC_INTR_CTRL2_CPU_CLEAR BCHP_AFEC_INTR_CTRL2_0_CPU_CLEAR
#define BCHP_AFEC_INTR_CTRL2_CPU_MASK_STATUS BCHP_AFEC_INTR_CTRL2_0_CPU_MASK_STATUS
#define BCHP_AFEC_INTR_CTRL2_CPU_MASK_SET BCHP_AFEC_INTR_CTRL2_0_CPU_MASK_SET
#define BCHP_AFEC_INTR_CTRL2_CPU_MASK_CLEAR BCHP_AFEC_INTR_CTRL2_0_CPU_MASK_CLEAR
#define BCHP_AFEC_INTR_CTRL2_PCI_STATUS BCHP_AFEC_INTR_CTRL2_0_PCI_STATUS
#define BCHP_AFEC_INTR_CTRL2_PCI_SET BCHP_AFEC_INTR_CTRL2_0_PCI_SET
#define BCHP_AFEC_INTR_CTRL2_PCI_CLEAR BCHP_AFEC_INTR_CTRL2_0_PCI_CLEAR
#define BCHP_AFEC_INTR_CTRL2_PCI_MASK_STATUS BCHP_AFEC_INTR_CTRL2_0_PCI_MASK_STATUS
#define BCHP_AFEC_INTR_CTRL2_PCI_MASK_SET BCHP_AFEC_INTR_CTRL2_0_PCI_MASK_SET
#define BCHP_AFEC_INTR_CTRL2_PCI_MASK_CLEAR BCHP_AFEC_INTR_CTRL2_0_PCI_MASK_CLEAR

#define BCHP_SDS_BERT_BERCTL BCHP_SDS_BERT_0_BERCTL
#define BCHP_SDS_BERT_BEIT BCHP_SDS_BERT_0_BEIT
#define BCHP_SDS_BERT_BERC BCHP_SDS_BERT_0_BERC
#define BCHP_SDS_BERT_BEM1 BCHP_SDS_BERT_0_BEM1
#define BCHP_SDS_BERT_BEM2 BCHP_SDS_BERT_0_BEM2
#define BCHP_SDS_BERT_BEM3 BCHP_SDS_BERT_0_BEM3
#define BCHP_SDS_BERT_BEST BCHP_SDS_BERT_0_BEST
#define BCHP_SDS_BERT_ACMCTL BCHP_SDS_BERT_0_ACMCTL

#define BCHP_SDS_BL_BLPCTL BCHP_SDS_BL_0_BLPCTL
#define BCHP_SDS_BL_PFCTL BCHP_SDS_BL_0_PFCTL
#define BCHP_SDS_BL_BRSW BCHP_SDS_BL_0_BRSW
#define BCHP_SDS_BL_BRLC BCHP_SDS_BL_0_BRLC
#define BCHP_SDS_BL_BRIC BCHP_SDS_BL_0_BRIC
#define BCHP_SDS_BL_BRI BCHP_SDS_BL_0_BRI
#define BCHP_SDS_BL_BFOS BCHP_SDS_BL_0_BFOS
#define BCHP_SDS_BL_BRFO BCHP_SDS_BL_0_BRFO
#define BCHP_SDS_BL_BNCO BCHP_SDS_BL_0_BNCO

#define BCHP_SDS_CG_RSTCTL BCHP_SDS_CG_0_RSTCTL
#define BCHP_SDS_CG_CGDIV00 BCHP_SDS_CG_0_CGDIV00
#define BCHP_SDS_CG_CGDIV01 BCHP_SDS_CG_0_CGDIV01
#define BCHP_SDS_CG_SPLL_NPDIV BCHP_SDS_CG_0_SPLL_NPDIV
#define BCHP_SDS_CG_SPLL_MDIV_CTRL BCHP_SDS_CG_0_SPLL_MDIV_CTRL
#define BCHP_SDS_CG_SPLL_CTRL BCHP_SDS_CG_0_SPLL_CTRL
#define BCHP_SDS_CG_SPLL_SSC_CTRL1 BCHP_SDS_CG_0_SPLL_SSC_CTRL1
#define BCHP_SDS_CG_SPLL_SSC_CTRL0 BCHP_SDS_CG_0_SPLL_SSC_CTRL0
#define BCHP_SDS_CG_SPLL_STATUS BCHP_SDS_CG_0_SPLL_STATUS
#define BCHP_SDS_CG_SPLL_PWRDN_RST BCHP_SDS_CG_0_SPLL_PWRDN_RST

#define BCHP_SDS_CL_CLCTL1 BCHP_SDS_CL_0_CLCTL1
#define BCHP_SDS_CL_CLCTL2 BCHP_SDS_CL_0_CLCTL2
#define BCHP_SDS_CL_FLLC BCHP_SDS_CL_0_FLLC
#define BCHP_SDS_CL_FLLC1 BCHP_SDS_CL_0_FLLC1
#define BCHP_SDS_CL_FLIC BCHP_SDS_CL_0_FLIC
#define BCHP_SDS_CL_FLIC1 BCHP_SDS_CL_0_FLIC1
#define BCHP_SDS_CL_FLSW BCHP_SDS_CL_0_FLSW
#define BCHP_SDS_CL_FLI BCHP_SDS_CL_0_FLI
#define BCHP_SDS_CL_FLIF BCHP_SDS_CL_0_FLIF
#define BCHP_SDS_CL_FLPA BCHP_SDS_CL_0_FLPA
#define BCHP_SDS_CL_FLTD BCHP_SDS_CL_0_FLTD
#define BCHP_SDS_CL_PEEST BCHP_SDS_CL_0_PEEST
#define BCHP_SDS_CL_PLTD BCHP_SDS_CL_0_PLTD
#define BCHP_SDS_CL_PLC BCHP_SDS_CL_0_PLC
#define BCHP_SDS_CL_PLC1 BCHP_SDS_CL_0_PLC1
#define BCHP_SDS_CL_PLSW BCHP_SDS_CL_0_PLSW
#define BCHP_SDS_CL_PLI BCHP_SDS_CL_0_PLI
#define BCHP_SDS_CL_PLPA BCHP_SDS_CL_0_PLPA
#define BCHP_SDS_CL_CRBFD BCHP_SDS_CL_0_CRBFD
#define BCHP_SDS_CL_CLHT BCHP_SDS_CL_0_CLHT
#define BCHP_SDS_CL_CLLT BCHP_SDS_CL_0_CLLT
#define BCHP_SDS_CL_CLLA BCHP_SDS_CL_0_CLLA
#define BCHP_SDS_CL_CLCT BCHP_SDS_CL_0_CLCT
#define BCHP_SDS_CL_CLFFCTL BCHP_SDS_CL_0_CLFFCTL
#define BCHP_SDS_CL_FFLPA BCHP_SDS_CL_0_FFLPA
#define BCHP_SDS_CL_CLFBCTL BCHP_SDS_CL_0_CLFBCTL
#define BCHP_SDS_CL_FBLC BCHP_SDS_CL_0_FBLC
#define BCHP_SDS_CL_FBLI BCHP_SDS_CL_0_FBLI
#define BCHP_SDS_CL_FBPA BCHP_SDS_CL_0_FBPA
#define BCHP_SDS_CL_CLDAFECTL BCHP_SDS_CL_0_CLDAFECTL
#define BCHP_SDS_CL_DAFELI BCHP_SDS_CL_0_DAFELI
#define BCHP_SDS_CL_DAFEINT BCHP_SDS_CL_0_DAFEINT

#define BCHP_SDS_DFT_CTRL0 BCHP_SDS_DFT_0_CTRL0
#define BCHP_SDS_DFT_CTRL1 BCHP_SDS_DFT_0_CTRL1
#define BCHP_SDS_DFT_STATUS BCHP_SDS_DFT_0_STATUS
#define BCHP_SDS_DFT_RANGE_START BCHP_SDS_DFT_0_RANGE_START
#define BCHP_SDS_DFT_RANGE_END BCHP_SDS_DFT_0_RANGE_END
#define BCHP_SDS_DFT_DDFS_FCW BCHP_SDS_DFT_0_DDFS_FCW
#define BCHP_SDS_DFT_PEAK_POW BCHP_SDS_DFT_0_PEAK_POW
#define BCHP_SDS_DFT_PEAK_BIN BCHP_SDS_DFT_0_PEAK_BIN
#define BCHP_SDS_DFT_TOTAL_POW BCHP_SDS_DFT_0_TOTAL_POW
#define BCHP_SDS_DFT_MEM_RADDR BCHP_SDS_DFT_0_MEM_RADDR
#define BCHP_SDS_DFT_MEM_RDATA BCHP_SDS_DFT_0_MEM_RDATA

#define BCHP_SDS_EQ_EQMISCCTL BCHP_SDS_EQ_0_EQMISCCTL
#define BCHP_SDS_EQ_EQFFECTL BCHP_SDS_EQ_0_EQFFECTL
#define BCHP_SDS_EQ_EQCFAD BCHP_SDS_EQ_0_EQCFAD
#define BCHP_SDS_EQ_EQFRZCTL BCHP_SDS_EQ_0_EQFRZCTL
#define BCHP_SDS_EQ_F0B BCHP_SDS_EQ_0_F0B
#define BCHP_SDS_EQ_HD8PSK1 BCHP_SDS_EQ_0_HD8PSK1
#define BCHP_SDS_EQ_HD8PSK2 BCHP_SDS_EQ_0_HD8PSK2
#define BCHP_SDS_EQ_HDQPSK BCHP_SDS_EQ_0_HDQPSK
#define BCHP_SDS_EQ_HD16QAM BCHP_SDS_EQ_0_HD16QAM
#define BCHP_SDS_EQ_CMA BCHP_SDS_EQ_0_CMA
#define BCHP_SDS_EQ_CMATH BCHP_SDS_EQ_0_CMATH
#define BCHP_SDS_EQ_VLCTL BCHP_SDS_EQ_0_VLCTL
#define BCHP_SDS_EQ_VLCI BCHP_SDS_EQ_0_VLCI
#define BCHP_SDS_EQ_VLCQ BCHP_SDS_EQ_0_VLCQ
#define BCHP_SDS_EQ_VCOS BCHP_SDS_EQ_0_VCOS
#define BCHP_SDS_EQ_TSFT BCHP_SDS_EQ_0_TSFT
#define BCHP_SDS_EQ_EQSFT BCHP_SDS_EQ_0_EQSFT
#define BCHP_SDS_EQ_PILOTCTL BCHP_SDS_EQ_0_PILOTCTL
#define BCHP_SDS_EQ_PLDCTL BCHP_SDS_EQ_0_PLDCTL
#define BCHP_SDS_EQ_HDRD BCHP_SDS_EQ_0_HDRD
#define BCHP_SDS_EQ_HDRA BCHP_SDS_EQ_0_HDRA
#define BCHP_SDS_EQ_XSEED BCHP_SDS_EQ_0_XSEED
#define BCHP_SDS_EQ_XTAP1 BCHP_SDS_EQ_0_XTAP1
#define BCHP_SDS_EQ_XTAP2 BCHP_SDS_EQ_0_XTAP2
#define BCHP_SDS_EQ_LUPD BCHP_SDS_EQ_0_LUPD
#define BCHP_SDS_EQ_LUPA BCHP_SDS_EQ_0_LUPA
#define BCHP_SDS_EQ_SDSLEN BCHP_SDS_EQ_0_SDSLEN
#define BCHP_SDS_EQ_SDSIG BCHP_SDS_EQ_0_SDSIG
#define BCHP_SDS_EQ_MGAIND BCHP_SDS_EQ_0_MGAIND
#define BCHP_SDS_EQ_MGAINA BCHP_SDS_EQ_0_MGAINA
#define BCHP_SDS_EQ_CWC_CTRL BCHP_SDS_EQ_0_CWC_CTRL
#define BCHP_SDS_EQ_CWC_SPUR_FREQ BCHP_SDS_EQ_0_CWC_SPUR_FREQ
#define BCHP_SDS_EQ_CWC_LFC BCHP_SDS_EQ_0_CWC_LFC
#define BCHP_SDS_EQ_CWC_LF_INT BCHP_SDS_EQ_0_CWC_LF_INT

#define BCHP_SDS_FE_ADCPCTL BCHP_SDS_FE_0_ADCPCTL
#define BCHP_SDS_FE_DCOCTL BCHP_SDS_FE_0_DCOCTL
#define BCHP_SDS_FE_DCOI BCHP_SDS_FE_0_DCOI
#define BCHP_SDS_FE_IQCTL BCHP_SDS_FE_0_IQCTL
#define BCHP_SDS_FE_IQAEST BCHP_SDS_FE_0_IQAEST
#define BCHP_SDS_FE_IQPEST BCHP_SDS_FE_0_IQPEST
#define BCHP_SDS_FE_MIXCTL BCHP_SDS_FE_0_MIXCTL
#define BCHP_SDS_FE_DSTGCTL BCHP_SDS_FE_0_DSTGCTL
#define BCHP_SDS_FE_FILTCTL BCHP_SDS_FE_0_FILTCTL
#define BCHP_SDS_FE_DFCTL BCHP_SDS_FE_0_DFCTL
#define BCHP_SDS_FE_AGFCTL BCHP_SDS_FE_0_AGFCTL
#define BCHP_SDS_FE_AGF BCHP_SDS_FE_0_AGF
#define BCHP_SDS_FE_AIF BCHP_SDS_FE_0_AIF
#define BCHP_SDS_FE_NVCTL BCHP_SDS_FE_0_NVCTL

#define BCHP_SDS_FEC_FECTL BCHP_SDS_FEC_0_FECTL
#define BCHP_SDS_FEC_FSYN BCHP_SDS_FEC_0_FSYN
#define BCHP_SDS_FEC_FRS BCHP_SDS_FEC_0_FRS
#define BCHP_SDS_FEC_FMOD BCHP_SDS_FEC_0_FMOD
#define BCHP_SDS_FEC_FERR BCHP_SDS_FEC_0_FERR
#define BCHP_SDS_FEC_FRSV BCHP_SDS_FEC_0_FRSV

#define BCHP_SDS_GR_BRIDGE_REVISION BCHP_SDS_GR_BRIDGE_0_REVISION
#define BCHP_SDS_GR_BRIDGE_CTRL BCHP_SDS_GR_BRIDGE_0_CTRL
#define BCHP_SDS_GR_BRIDGE_SW_INIT_0 BCHP_SDS_GR_BRIDGE_0_SW_INIT_0
#define BCHP_SDS_GR_BRIDGE_SW_INIT_1 BCHP_SDS_GR_BRIDGE_0_SW_INIT_1

#define BCHP_SDS_HP_HPCONTROL BCHP_SDS_HP_0_HPCONTROL
#define BCHP_SDS_HP_HPCONFIG BCHP_SDS_HP_0_HPCONFIG
#define BCHP_SDS_HP_FNORM BCHP_SDS_HP_0_FNORM
#define BCHP_SDS_HP_HPOVERRIDE BCHP_SDS_HP_0_HPOVERRIDE
#define BCHP_SDS_HP_FROF1 BCHP_SDS_HP_0_FROF1
#define BCHP_SDS_HP_FROF2 BCHP_SDS_HP_0_FROF2
#define BCHP_SDS_HP_FROF3 BCHP_SDS_HP_0_FROF3
#define BCHP_SDS_HP_FROF1_SW BCHP_SDS_HP_0_FROF1_SW
#define BCHP_SDS_HP_FROF2_SW BCHP_SDS_HP_0_FROF2_SW
#define BCHP_SDS_HP_FROF3_SW BCHP_SDS_HP_0_FROF3_SW
#define BCHP_SDS_HP_M_N_PEAK_VERIFY BCHP_SDS_HP_0_M_N_PEAK_VERIFY
#define BCHP_SDS_HP_M_N_RECEIVER_VERIFY BCHP_SDS_HP_0_M_N_RECEIVER_VERIFY
#define BCHP_SDS_HP_M_N_RECEIVER_LOCK BCHP_SDS_HP_0_M_N_RECEIVER_LOCK
#define BCHP_SDS_HP_DCORR_THRESHOLD BCHP_SDS_HP_0_DCORR_THRESHOLD
#define BCHP_SDS_HP_PLHDRSCR1 BCHP_SDS_HP_0_PLHDRSCR1
#define BCHP_SDS_HP_PLHDRSCR2 BCHP_SDS_HP_0_PLHDRSCR2
#define BCHP_SDS_HP_PLHDRSCR3 BCHP_SDS_HP_0_PLHDRSCR3
#define BCHP_SDS_HP_ACM_CHECK BCHP_SDS_HP_0_ACM_CHECK
#define BCHP_SDS_HP_FRAME_LENGTH_INITIAL BCHP_SDS_HP_0_FRAME_LENGTH_INITIAL
#define BCHP_SDS_HP_FRAME_LENGTH_DUMMY_NORMAL BCHP_SDS_HP_0_FRAME_LENGTH_DUMMY_NORMAL
#define BCHP_SDS_HP_FRAME_LENGTH_QPSK_NORMAL BCHP_SDS_HP_0_FRAME_LENGTH_QPSK_NORMAL
#define BCHP_SDS_HP_FRAME_LENGTH_8PSK_NORMAL BCHP_SDS_HP_0_FRAME_LENGTH_8PSK_NORMAL
#define BCHP_SDS_HP_FRAME_LENGTH_16APSK_NORMAL BCHP_SDS_HP_0_FRAME_LENGTH_16APSK_NORMAL
#define BCHP_SDS_HP_FRAME_LENGTH_32APSK_NORMAL BCHP_SDS_HP_0_FRAME_LENGTH_32APSK_NORMAL
#define BCHP_SDS_HP_FRAME_LENGTH_RESERVED_29_NORMAL BCHP_SDS_HP_0_FRAME_LENGTH_RESERVED_29_NORMAL
#define BCHP_SDS_HP_FRAME_LENGTH_RESERVED_30_NORMAL BCHP_SDS_HP_0_FRAME_LENGTH_RESERVED_30_NORMAL
#define BCHP_SDS_HP_FRAME_LENGTH_RESERVED_31_NORMAL BCHP_SDS_HP_0_FRAME_LENGTH_RESERVED_31_NORMAL
#define BCHP_SDS_HP_FRAME_LENGTH_DUMMY_SHORT BCHP_SDS_HP_0_FRAME_LENGTH_DUMMY_SHORT
#define BCHP_SDS_HP_FRAME_LENGTH_QPSK_SHORT BCHP_SDS_HP_0_FRAME_LENGTH_QPSK_SHORT
#define BCHP_SDS_HP_FRAME_LENGTH_8PSK_SHORT BCHP_SDS_HP_0_FRAME_LENGTH_8PSK_SHORT
#define BCHP_SDS_HP_FRAME_LENGTH_16APSK_SHORT BCHP_SDS_HP_0_FRAME_LENGTH_16APSK_SHORT
#define BCHP_SDS_HP_FRAME_LENGTH_32APSK_SHORT BCHP_SDS_HP_0_FRAME_LENGTH_32APSK_SHORT
#define BCHP_SDS_HP_FRAME_LENGTH_RESERVED_29_SHORT BCHP_SDS_HP_0_FRAME_LENGTH_RESERVED_29_SHORT
#define BCHP_SDS_HP_FRAME_LENGTH_RESERVED_30_SHORT BCHP_SDS_HP_0_FRAME_LENGTH_RESERVED_30_SHORT
#define BCHP_SDS_HP_FRAME_LENGTH_RESERVED_31_SHORT BCHP_SDS_HP_0_FRAME_LENGTH_RESERVED_31_SHORT
#define BCHP_SDS_HP_FRAME_LENGTH_SAMPLE BCHP_SDS_HP_0_FRAME_LENGTH_SAMPLE
#define BCHP_SDS_HP_PEAK_SAMPLE_1_0 BCHP_SDS_HP_0_PEAK_SAMPLE_1_0
#define BCHP_SDS_HP_PEAK_SAMPLE_3_2 BCHP_SDS_HP_0_PEAK_SAMPLE_3_2

#define BCHP_SDS_INTR2_0_CPU_STATUS BCHP_SDS_INTR2_0_0_CPU_STATUS
#define BCHP_SDS_INTR2_0_CPU_SET BCHP_SDS_INTR2_0_0_CPU_SET
#define BCHP_SDS_INTR2_0_CPU_CLEAR BCHP_SDS_INTR2_0_0_CPU_CLEAR
#define BCHP_SDS_INTR2_0_CPU_MASK_STATUS BCHP_SDS_INTR2_0_0_CPU_MASK_STATUS
#define BCHP_SDS_INTR2_0_CPU_MASK_SET BCHP_SDS_INTR2_0_0_CPU_MASK_SET
#define BCHP_SDS_INTR2_0_CPU_MASK_CLEAR BCHP_SDS_INTR2_0_0_CPU_MASK_CLEAR
#define BCHP_SDS_INTR2_0_PCI_STATUS BCHP_SDS_INTR2_0_0_PCI_STATUS
#define BCHP_SDS_INTR2_0_PCI_SET BCHP_SDS_INTR2_0_0_PCI_SET
#define BCHP_SDS_INTR2_0_PCI_CLEAR BCHP_SDS_INTR2_0_0_PCI_CLEAR
#define BCHP_SDS_INTR2_0_PCI_MASK_STATUS BCHP_SDS_INTR2_0_0_PCI_MASK_STATUS
#define BCHP_SDS_INTR2_0_PCI_MASK_SET BCHP_SDS_INTR2_0_0_PCI_MASK_SET
#define BCHP_SDS_INTR2_0_PCI_MASK_CLEAR BCHP_SDS_INTR2_0_0_PCI_MASK_CLEAR

#define BCHP_SDS_MISC_REVID BCHP_SDS_MISC_0_REVID
#define BCHP_SDS_MISC_IICTL1 BCHP_SDS_MISC_0_IICTL1
#define BCHP_SDS_MISC_IICTL2 BCHP_SDS_MISC_0_IICTL2
#define BCHP_SDS_MISC_IICCNT BCHP_SDS_MISC_0_IICCNT
#define BCHP_SDS_MISC_IICHPA BCHP_SDS_MISC_0_IICHPA
#define BCHP_SDS_MISC_MIICTX1 BCHP_SDS_MISC_0_MIICTX1
#define BCHP_SDS_MISC_MIICTX2 BCHP_SDS_MISC_0_MIICTX2
#define BCHP_SDS_MISC_MIICRX1 BCHP_SDS_MISC_0_MIICRX1
#define BCHP_SDS_MISC_MIICRX2 BCHP_SDS_MISC_0_MIICRX2
#define BCHP_SDS_MISC_MI2CSA BCHP_SDS_MISC_0_MI2CSA
#define BCHP_SDS_MISC_TMRCTL BCHP_SDS_MISC_0_TMRCTL
#define BCHP_SDS_MISC_GENTMR3 BCHP_SDS_MISC_0_GENTMR3
#define BCHP_SDS_MISC_GENTMR2 BCHP_SDS_MISC_0_GENTMR2
#define BCHP_SDS_MISC_GENTMR1 BCHP_SDS_MISC_0_GENTMR1
#define BCHP_SDS_MISC_BERTMR BCHP_SDS_MISC_0_BERTMR
#define BCHP_SDS_MISC_BTMR BCHP_SDS_MISC_0_BTMR
#define BCHP_SDS_MISC_TPDIR BCHP_SDS_MISC_0_TPDIR
#define BCHP_SDS_MISC_TPODS BCHP_SDS_MISC_0_TPODS
#define BCHP_SDS_MISC_TPDS BCHP_SDS_MISC_0_TPDS
#define BCHP_SDS_MISC_TPCTL1 BCHP_SDS_MISC_0_TPCTL1
#define BCHP_SDS_MISC_TPCTL2 BCHP_SDS_MISC_0_TPCTL2
#define BCHP_SDS_MISC_TPCTL3 BCHP_SDS_MISC_0_TPCTL3
#define BCHP_SDS_MISC_TPOUT BCHP_SDS_MISC_0_TPOUT
#define BCHP_SDS_MISC_MISCTL BCHP_SDS_MISC_0_MISCTL
#define BCHP_SDS_MISC_INTR_RAW_STS0 BCHP_SDS_MISC_0_INTR_RAW_STS0
#define BCHP_SDS_MISC_INTR_RAW_STS1 BCHP_SDS_MISC_0_INTR_RAW_STS1

#define BCHP_SDS_NTCH_CTRL BCHP_SDS_NTCH_0_CTRL
#define BCHP_SDS_NTCH_FCWADJ_SCL BCHP_SDS_NTCH_0_FCWADJ_SCL
#define BCHP_SDS_NTCH_FCW0 BCHP_SDS_NTCH_0_FCW0
#define BCHP_SDS_NTCH_FCW1 BCHP_SDS_NTCH_0_FCW1
#define BCHP_SDS_NTCH_FCW2 BCHP_SDS_NTCH_0_FCW2
#define BCHP_SDS_NTCH_FCW3 BCHP_SDS_NTCH_0_FCW3
#define BCHP_SDS_NTCH_DCO0_INT BCHP_SDS_NTCH_0_DCO0_INT
#define BCHP_SDS_NTCH_DCO1_INT BCHP_SDS_NTCH_0_DCO1_INT
#define BCHP_SDS_NTCH_DCO2_INT BCHP_SDS_NTCH_0_DCO2_INT

#define BCHP_SDS_OI_OIFCTL00 BCHP_SDS_OI_0_OIFCTL00
#define BCHP_SDS_OI_OIFCTL01 BCHP_SDS_OI_0_OIFCTL01
#define BCHP_SDS_OI_OIFCTL02 BCHP_SDS_OI_0_OIFCTL02
#define BCHP_SDS_OI_OPLL BCHP_SDS_OI_0_OPLL
#define BCHP_SDS_OI_OPLL2 BCHP_SDS_OI_0_OPLL2
#define BCHP_SDS_OI_FERC BCHP_SDS_OI_0_FERC
#define BCHP_SDS_OI_FRC BCHP_SDS_OI_0_FRC
#define BCHP_SDS_OI_OSIGPN BCHP_SDS_OI_0_OSIGPN
#define BCHP_SDS_OI_OSUBD BCHP_SDS_OI_0_OSUBD
#define BCHP_SDS_OI_OCOEF BCHP_SDS_OI_0_OCOEF
#define BCHP_SDS_OI_OFI BCHP_SDS_OI_0_OFI
#define BCHP_SDS_OI_OPLL_NPDIV BCHP_SDS_OI_0_OPLL_NPDIV
#define BCHP_SDS_OI_OPLL_MDIV_CTRL BCHP_SDS_OI_0_OPLL_MDIV_CTRL
#define BCHP_SDS_OI_OPLL_CTRL BCHP_SDS_OI_0_OPLL_CTRL
#define BCHP_SDS_OI_OPLL_SSC_CTRL1 BCHP_SDS_OI_0_OPLL_SSC_CTRL1
#define BCHP_SDS_OI_OPLL_SSC_CTRL0 BCHP_SDS_OI_0_OPLL_SSC_CTRL0
#define BCHP_SDS_OI_OPLL_STATUS BCHP_SDS_OI_0_OPLL_STATUS
#define BCHP_SDS_OI_OPLL_PWRDN_RST BCHP_SDS_OI_0_OPLL_PWRDN_RST
#define BCHP_SDS_OI_OPLL_MDIV_CTRL_channel_load_en_MASK BCHP_SDS_OI_0_OPLL_MDIV_CTRL_channel_load_en_MASK
#define BCHP_SDS_OI_OIFCTL01_loop_en_MASK BCHP_SDS_OI_0_OIFCTL01_loop_en_MASK

#define BCHP_SDS_SNR_SNRCTL BCHP_SDS_SNR_0_SNRCTL
#define BCHP_SDS_SNR_SNRHT BCHP_SDS_SNR_0_SNRHT
#define BCHP_SDS_SNR_SNRLT BCHP_SDS_SNR_0_SNRLT
#define BCHP_SDS_SNR_SNRE BCHP_SDS_SNR_0_SNRE
#define BCHP_SDS_SNR_SNORETP BCHP_SDS_SNR_0_SNORETP
#define BCHP_SDS_SNR_SNORESP BCHP_SDS_SNR_0_SNORESP
#define BCHP_SDS_SNR_SNORECTL BCHP_SDS_SNR_0_SNORECTL

#define BCHP_SDS_VIT_VTCTL BCHP_SDS_VIT_0_VTCTL
#define BCHP_SDS_VIT_V10 BCHP_SDS_VIT_0_V10
#define BCHP_SDS_VIT_V32 BCHP_SDS_VIT_0_V32
#define BCHP_SDS_VIT_V54 BCHP_SDS_VIT_0_V54
#define BCHP_SDS_VIT_V76 BCHP_SDS_VIT_0_V76
#define BCHP_SDS_VIT_VINT BCHP_SDS_VIT_0_VINT
#define BCHP_SDS_VIT_VCNT BCHP_SDS_VIT_0_VCNT
#define BCHP_SDS_VIT_VSTC BCHP_SDS_VIT_0_VSTC
#define BCHP_SDS_VIT_VST BCHP_SDS_VIT_0_VST
#define BCHP_SDS_VIT_VREC BCHP_SDS_VIT_0_VREC
#define BCHP_SDS_VIT_VRCV BCHP_SDS_VIT_0_VRCV

/* other registers/names that changed */
#define BCHP_FTM_PHY_ANA0_0 BCHP_FTM_PHY_ANA_0_0
#define BCHP_FTM_PHY_ANA0_1 BCHP_FTM_PHY_ANA_0_1
#define BCHP_FTM_PHY_ANA1_0 BCHP_FTM_PHY_ANA_1_0
#define BCHP_FTM_PHY_ANA1_1 BCHP_FTM_PHY_ANA_1_1
#define BCHP_FTM_PHY_IRQ_MSK_RSSI_FALL_IM_SHIFT BCHP_FTM_INTR2_CPU_MASK_STATUS_RSSI_FALL_IM_SHIFT
#define BCHP_FTM_PHY_IRQ_MSK_RX_TYPE1_CNT_IM_SHIFT BCHP_FTM_INTR2_CPU_MASK_STATUS_RX_TYPE1_CNT_IM_SHIFT
#define BCHP_FTM_PHY_IRQ_MSK_RX_TYPE2_CNT_IM_SHIFT BCHP_FTM_INTR2_CPU_MASK_STATUS_RX_TYPE2_CNT_IM_SHIFT
#define BCHP_SDS_INTR2_0_CPU_STATUS_vit_in_sync_MASK BCHP_SDS_INTR2_0_0_CPU_STATUS_vit_in_sync_MASK

/* BCM4528-specific functions */
BERR_Code BAST_4528_SetSampleFreq(BAST_Handle h, uint32_t Fs);

#endif /* BAST_4528_H__ */

