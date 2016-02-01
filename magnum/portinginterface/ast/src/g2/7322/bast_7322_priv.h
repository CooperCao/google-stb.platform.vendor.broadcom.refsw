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
#ifndef _BAST_7322_PRIV_H__
#define _BAST_7322_PRIV_H__               

#include "bchp_qpsk.h"
#include "bchp_qpsk_intr2_0.h"
#include "bchp_qpsk_intr2_1.h"
#include "bchp_sds.h"
#include "bchp_sds_intr2_0.h"
#include "bchp_sds_intr2_1.h"
#include "bchp_tuner.h"
#include "bchp_tfec.h"
#include "bchp_afec.h"
#include "bchp_grb.h"
#include "bchp_ftm_intr2.h"
#include "bchp_ftm_phy.h"
#include "bchp_ftm_uart.h"
#include "bchp_ftm_skit.h"


#define BCHP_INT_ID_SDS_LOCK_IS_0               BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS2_RVIL_IS_SHIFT + 8)
#define BCHP_INT_ID_SDS_NOT_LOCK_IS_0           BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS2_RVOL_IS_SHIFT + 8)
#define BCHP_INT_ID_SDS_LOCK_IS_1               BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS2_RVIL_IS_SHIFT + 8)
#define BCHP_INT_ID_SDS_NOT_LOCK_IS_1           BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS2_RVOL_IS_SHIFT + 8)
#define BCHP_INT_ID_SDS_BTM_IS_0                BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS1_BTM_IS_SHIFT)
#define BCHP_INT_ID_SDS_BRTM_IS_0               BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS1_BRTM_IS_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS1_0             BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_IRQSTS5_GENTM_IS1_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS2_0             BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_IRQSTS5_GENTM_IS2_SHIFT)
#define BCHP_INT_ID_SDS_XTLTM_IS_0              BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_IRQSTS5_XTLTM_IS_SHIFT)
#define BCHP_INT_ID_SDS_BTM_IS_1                BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS1_BTM_IS_SHIFT)
#define BCHP_INT_ID_SDS_BRTM_IS_1               BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS1_BRTM_IS_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS1_1             BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_1_CPU_STATUS, BCHP_SDS_IRQSTS5_GENTM_IS1_SHIFT)
#define BCHP_INT_ID_SDS_GENTM_IS2_1             BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_1_CPU_STATUS, BCHP_SDS_IRQSTS5_GENTM_IS2_SHIFT)
#define BCHP_INT_ID_SDS_XTLTM_IS_1              BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_1_CPU_STATUS, BCHP_SDS_IRQSTS5_XTLTM_IS_SHIFT)
#define BCHP_INT_ID_SDS_if_agc_le_thresh_rise_0 BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_IRQMSK5_if_agc_le_thresh_rise_SHIFT)
#define BCHP_INT_ID_SDS_if_agc_le_thresh_fall_0 BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_IRQMSK5_if_agc_le_thresh_fall_SHIFT)
#define BCHP_INT_ID_SDS_rf_agc_le_thresh_rise_0 BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_IRQMSK5_rf_agc_le_thresh_rise_SHIFT)
#define BCHP_INT_ID_SDS_rf_agc_le_thresh_fall_0 BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_IRQMSK5_rf_agc_le_thresh_fall_SHIFT)
#define BCHP_INT_ID_SDS_rf_agc_gt_max_change_0  BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_IRQMSK5_rf_agc_gt_max_change_SHIFT)
#define BCHP_INT_ID_SDS_if_agc_le_thresh_rise_1 BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_1_CPU_STATUS, BCHP_SDS_IRQMSK5_if_agc_le_thresh_rise_SHIFT)
#define BCHP_INT_ID_SDS_if_agc_le_thresh_fall_1 BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_1_CPU_STATUS, BCHP_SDS_IRQMSK5_if_agc_le_thresh_fall_SHIFT)
#define BCHP_INT_ID_SDS_rf_agc_le_thresh_rise_1 BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_1_CPU_STATUS, BCHP_SDS_IRQMSK5_rf_agc_le_thresh_rise_SHIFT)
#define BCHP_INT_ID_SDS_rf_agc_le_thresh_fall_1 BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_1_CPU_STATUS, BCHP_SDS_IRQMSK5_rf_agc_le_thresh_fall_SHIFT)
#define BCHP_INT_ID_SDS_rf_agc_gt_max_change_1  BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_1_CPU_STATUS, BCHP_SDS_IRQMSK5_rf_agc_gt_max_change_SHIFT)
#define BCHP_INT_ID_SDS_sar_vol_gt_hi_thrsh_0   BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_IRQSTS5_sar_vol_gt_hi_thrsh_SHIFT)
#define BCHP_INT_ID_SDS_sar_vol_lt_lo_thrsh_0   BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_1_CPU_STATUS, BCHP_SDS_IRQSTS5_sar_vol_lt_lo_thrsh_SHIFT)
#define BCHP_INT_ID_DSDN_IS_0                   BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS1_DSDN_IS_SHIFT)
#define BCHP_INT_ID_DSDN_IS_1                   BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS1_DSDN_IS_SHIFT)
#define BCHP_INT_ID_SDS_HP_IS_0                 BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS2_HP_IS_SHIFT + 8)
#define BCHP_INT_ID_SDS_HP_IS_1                 BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS2_HP_IS_SHIFT + 8)
#define BCHP_INT_ID_MI2C_IS_0                   BCHP_INT_ID_CREATE(BCHP_SDS_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS1_MIC_IS_SHIFT)
#define BCHP_INT_ID_MI2C_IS_1                   BCHP_INT_ID_CREATE(BCHP_QPSK_INTR2_0_CPU_STATUS, BCHP_SDS_IRQSTS1_MIC_IS_SHIFT)

#define BCHP_INT_ID_TURBO_LOCK_IS_0             0
#define BCHP_INT_ID_TURBO_LOCK_IS_1             0
#define BCHP_INT_ID_TURBO_NOT_LOCK_IS_0         0
#define BCHP_INT_ID_TURBO_NOT_LOCK_IS_1         0


#define BCHP_INT_ID_AFEC_LOCK_IS_0              0
#define BCHP_INT_ID_AFEC_LOCK_IS_1              0
#define BCHP_INT_ID_AFEC_NOT_LOCK_IS_0          0
#define BCHP_INT_ID_AFEC_NOT_LOCK_IS_1          0

#define BAST_G2_MAX_CHANNELS 2
#define BAST_G2_XTAL_FREQ 27000000
#define BAST_G2_CHIP 0x7322
#define BAST_G2_BUILD_VERSION   0x01
#define BAST_G2_TUNER_DAC_DIV_RATIO 0x27

#endif /* BAST_7322_PRIV_H__ */

