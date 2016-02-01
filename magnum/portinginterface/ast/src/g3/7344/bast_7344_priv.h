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
#ifndef _BAST_7344_PRIV_H__
#define _BAST_7344_PRIV_H__               

#include "bchp_afec.h"
#include "bchp_afec_global.h"
#include "bchp_afec_gr_bridge.h"
#include "bchp_afec_intr_ctrl2.h"

#include "bchp_sds_agc.h"
#include "bchp_sds_bert.h"
#include "bchp_sds_bl.h"
#include "bchp_sds_cg.h"
#include "bchp_sds_cl.h"
#include "bchp_sds_dft.h"
#include "bchp_sds_dsec.h"
#include "bchp_sds_eq.h"
#include "bchp_sds_fe.h"
#include "bchp_sds_fec.h"
#include "bchp_sds_gr_bridge.h"
#include "bchp_sds_hp.h"
#include "bchp_sds_intr2_0.h"
#include "bchp_sds_intr2_1.h"
#include "bchp_sds_misc.h"
#if (BCHP_VER == BCHP_VER_A0)
#include "bchp_sds_ntch.h"
#else
#include "bchp_sds_cwc.h"
#endif
#include "bchp_sds_oi.h"
#include "bchp_sds_snr.h"
#include "bchp_sds_tuner.h"
#include "bchp_sds_vit.h"

#include "bchp_tfec.h"
#include "bchp_tfec_gr_bridge.h"
#include "bchp_tfec_intr2.h"
#include "bchp_tfec_misc.h"

#include "bchp_ftm_gr_bridge.h"
#include "bchp_ftm_intr2.h"
#include "bchp_ftm_phy.h"
#include "bchp_ftm_skit.h"
#include "bchp_ftm_uart.h"
#if (BCHP_VER > BCHP_VER_A0)
#include "bchp_ftm_phy_ana.h"
#define BCHP_FTM_PHY_ANA0_0 BCHP_FTM_PHY_ANA_CTL0_0
#define BCHP_FTM_PHY_ANA0_1 BCHP_FTM_PHY_ANA_CTL0_1
#define BCHP_FTM_PHY_ANA1_0 BCHP_FTM_PHY_ANA_CTL1_0
#define BCHP_FTM_PHY_ANA1_1 BCHP_FTM_PHY_ANA_CTL1_1
#endif


#define BAST_G3_BUILD_VERSION 1 


#define BCHP_INT_ID_SDS_LOCK_IS_0               BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_INTR2_0_CPU_STATUS_rvb_in_lock_SHIFT)
#define BCHP_INT_ID_SDS_NOT_LOCK_IS_0           BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_INTR2_0_CPU_STATUS_rvb_out_lock_SHIFT)
#define BCHP_INT_ID_SDS_BTM_IS_0                BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_INTR2_0_CPU_STATUS_bclktimer_int_SHIFT)
#define BCHP_INT_ID_SDS_BRTM_IS_0               BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_INTR2_0_CPU_STATUS_bertimer_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS1_0             BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_INTR2_0_CPU_STATUS_gentimer1_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS2_0             BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_INTR2_0_CPU_STATUS_gentimer2_int_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS3_0             BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_INTR2_0_CPU_STATUS_gentimer3_int_SHIFT)
#define BCHP_INT_ID_SDS_HP_IS_0                 BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_INTR2_0_CPU_STATUS_hp_state_match_SHIFT)
#define BCHP_INT_ID_MI2C_IS_0                   BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_INTR2_0_CPU_STATUS_mi2c_int_SHIFT)

#define BCHP_INT_ID_SDS_sar_vol_gt_hi_thrsh_0   BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_INTR2_1_CPU_STATUS_dsec_sar_vol_gt_thresh_SHIFT)
#define BCHP_INT_ID_SDS_sar_vol_lt_lo_thrsh_0   BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_INTR2_1_CPU_STATUS_dsec_sar_vol_lt_thresh_SHIFT)
#define BCHP_INT_ID_DSDN_IS_0                   BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_INTR2_1_CPU_STATUS_dsec_done_SHIFT)
#define BCHP_INT_ID_DISEQC_TIMER1_0             BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_INTR2_1_CPU_STATUS_dsec_timer1_SHIFT)
#define BCHP_INT_ID_DISEQC_TIMER2_0             BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_INTR2_1_CPU_STATUS_dsec_timer2_SHIFT)
#define BCHP_INT_ID_DISEQC_tx_fifo_a_empty_0    BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_INTR2_1_CPU_STATUS_dsec_tx_fifo_a_empty_SHIFT)

#define BCHP_INT_ID_TURBO_LOCK_IS_0             BCHP_INT_ID_CREATE(BCHP_TFEC_INTR2_CPU_STATUS, BCHP_TFEC_INTR2_CPU_STATUS_TIL_INTR_SHIFT)
#define BCHP_INT_ID_TURBO_NOT_LOCK_IS_0         BCHP_INT_ID_CREATE(BCHP_TFEC_INTR2_CPU_STATUS, BCHP_TFEC_INTR2_CPU_STATUS_TOL_INTR_SHIFT)
#define BCHP_INT_ID_TURBO_SYNC_0                BCHP_INT_ID_CREATE(BCHP_TFEC_INTR2_CPU_STATUS, BCHP_TFEC_INTR2_CPU_STATUS_TRIL_INTR_SHIFT)

#define BCHP_INT_ID_AFEC_LOCK_IS_0              BCHP_INT_ID_CREATE(BCHP_AFEC_INTR_CTRL2_CPU_STATUS, BCHP_AFEC_INTR_CTRL2_CPU_STATUS_MP_LOCK_INTR_SHIFT)
#define BCHP_INT_ID_AFEC_NOT_LOCK_IS_0          BCHP_INT_ID_CREATE(BCHP_AFEC_INTR_CTRL2_CPU_STATUS, BCHP_AFEC_INTR_CTRL2_CPU_STATUS_MP_LOCK_NEG_INTR_SHIFT)

#define BAST_G3_MAX_CHANNELS 1
#define BAST_G3_XTAL_FREQ    54000000
#define BAST_G3_CHIP         0x7344

#define BAST_DEFAULT_SAMPLE_FREQ 184500000

#endif /* BAST_7344_PRIV_H__ */

