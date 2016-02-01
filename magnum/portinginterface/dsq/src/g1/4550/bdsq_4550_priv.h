/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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

#ifndef _BDSQ_4550_PRIV_H__
#define _BDSQ_4550_PRIV_H__

#include "bchp_leap_l1.h"
#include "bchp_sds_dsec.h"
#include "bchp_sds_dsec_gr_bridge.h"
#include "bchp_sds_dsec_intr2.h"
#include "bchp_ftm_phy_ana.h"

#define BDSQ_G1_BUILD_VERSION 0x01

#define BDSQ_NUM_CHANNELS 1


/* DSEC interrupts */
#define BCHP_INT_ID_DSEC_TIMER1_0         BINT_LEAP_INT_ID_CREATE((BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT), BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_timer2_SHIFT)
#define BCHP_INT_ID_DSEC_TIMER2_0         BINT_LEAP_INT_ID_CREATE((BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT), BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_timer1_SHIFT)
#define BCHP_INT_ID_DSEC_DONE_0           BINT_LEAP_INT_ID_CREATE((BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT), BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_done_SHIFT)
#define BCHP_INT_ID_DSEC_TX_A_EMPTY_0     BINT_LEAP_INT_ID_CREATE((BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT), BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_tx_fifo_a_empty_SHIFT)
#define BCHP_INT_ID_DSEC_RX_A_EMPTY_0     BINT_LEAP_INT_ID_CREATE((BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT), BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_rx_fifo_a_empty_SHIFT)
#define BCHP_INT_ID_DSEC_ACW_DONE_0       BINT_LEAP_INT_ID_CREATE((BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT), BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_acw_done_SHIFT)
#define BCHP_INT_ID_DSEC_VOL_LT_THRESH_0  BINT_LEAP_INT_ID_CREATE((BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT), BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_sar_vol_lt_thresh_SHIFT)
#define BCHP_INT_ID_DSEC_VOL_GT_THRESH_0  BINT_LEAP_INT_ID_CREATE((BCHP_LEAP_L1_INTR_STATUS_DISEC_INTR_SHIFT), BCHP_SDS_DSEC_INTR2_CPU_STATUS_dsec_sar_vol_gt_thresh_SHIFT)
#define BCHP_INT_ID_DSEC_TONE_RISE_0      0
#define BCHP_INT_ID_DSEC_TONE_FALL_0      0
#define BCHP_INT_ID_DSEC_IDLE_TIMEOUT_0   0


#endif /* BDSQ_4550_PRIV_H__ */