/********************************************************************************
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
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Fri Feb 26 13:25:14 2016
 *                 Full Compile MD5 Checksum  1560bfee4f086d6e1d49e6bd3406a38d
 *                     (minus title and desc)
 *                 MD5 Checksum               8d7264bb382089f88abd2b1abb2a6340
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                unknown
 *                 RDB.pm                     823
 *                 generate_int_id.pl         1.0
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/generate_int_id.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#include "bchp.h"
#include "bchp_hdmi_tx_intr2.h"

#ifndef BCHP_INT_ID_HDMI_TX_INTR2_H__
#define BCHP_INT_ID_HDMI_TX_INTR2_H__

#define BCHP_INT_ID_DRIFT_FIFO_ALMOST_EMPTY_INTR BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_DRIFT_FIFO_ALMOST_EMPTY_INTR_SHIFT)
#define BCHP_INT_ID_DRIFT_FIFO_ALMOST_FULL_INTR BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_DRIFT_FIFO_ALMOST_FULL_INTR_SHIFT)
#define BCHP_INT_ID_DRIFT_FIFO_EMPTY_MINUS_INTR BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_DRIFT_FIFO_EMPTY_MINUS_INTR_SHIFT)
#define BCHP_INT_ID_DRIFT_FIFO_FULL_MINUS_INTR BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_DRIFT_FIFO_FULL_MINUS_INTR_SHIFT)
#define BCHP_INT_ID_DVP_FORMAT_DET_SATURATED_COUNT_INTR BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_DVP_FORMAT_DET_SATURATED_COUNT_INTR_SHIFT)
#define BCHP_INT_ID_DVP_FORMAT_DET_UPDATE_INTR BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_DVP_FORMAT_DET_UPDATE_INTR_SHIFT)
#define BCHP_INT_ID_FD_LINE_TRIGGER_INTR      BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_FD_LINE_TRIGGER_INTR_SHIFT)
#define BCHP_INT_ID_HDCP_AN_READY_INTR        BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_HDCP_AN_READY_INTR_SHIFT)
#define BCHP_INT_ID_HDCP_PJ_INTR              BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_HDCP_PJ_INTR_SHIFT)
#define BCHP_INT_ID_HDCP_REPEATER_ERR_INTR    BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_HDCP_REPEATER_ERR_INTR_SHIFT)
#define BCHP_INT_ID_HDCP_RI_INTR              BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_HDCP_RI_INTR_SHIFT)
#define BCHP_INT_ID_HDCP_V_MATCH_INTR         BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_HDCP_V_MATCH_INTR_SHIFT)
#define BCHP_INT_ID_HDCP_V_MISMATCH_INTR      BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_HDCP_V_MISMATCH_INTR_SHIFT)
#define BCHP_INT_ID_HP_CONNECTED_INTR         BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_HP_CONNECTED_INTR_SHIFT)
#define BCHP_INT_ID_HP_REMOVED_INTR           BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_HP_REMOVED_INTR_SHIFT)
#define BCHP_INT_ID_ILLEGAL_WRITE_TO_ACTIVE_RAM_PACKET_INTR BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_ILLEGAL_WRITE_TO_ACTIVE_RAM_PACKET_INTR_SHIFT)
#define BCHP_INT_ID_MAI_FORMAT_UPDATE_INTR    BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_MAI_FORMAT_UPDATE_INTR_SHIFT)
#define BCHP_INT_ID_PACKET_FIFO_OVERFLOW_INTR BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_PACKET_FIFO_OVERFLOW_INTR_SHIFT)
#define BCHP_INT_ID_PJ_MISMATCH_INTR          BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_PJ_MISMATCH_INTR_SHIFT)
#define BCHP_INT_ID_RGB_UNDER_RANGE_INTR      BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_RGB_UNDER_RANGE_INTR_SHIFT)
#define BCHP_INT_ID_RI_A_MISMATCH_INTR        BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_RI_A_MISMATCH_INTR_SHIFT)
#define BCHP_INT_ID_RI_B_MISMATCH_INTR        BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_RI_B_MISMATCH_INTR_SHIFT)
#define BCHP_INT_ID_RSEN_UPDATE_INTR          BCHP_INT_ID_CREATE(BCHP_HDMI_TX_INTR2_CPU_STATUS, BCHP_HDMI_TX_INTR2_CPU_STATUS_RSEN_UPDATE_INTR_SHIFT)

#endif /* #ifndef BCHP_INT_ID_HDMI_TX_INTR2_H__ */

/* End of File */
