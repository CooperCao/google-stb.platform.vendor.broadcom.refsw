/****************************************************************************
 *     Copyright (c) 1999-2015, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Wed Apr  1 11:25:04 2015
 *                 Full Compile MD5 Checksum  267f8e92d9b43928c0a06f1ab29c511c
 *                     (minus title and desc)
 *                 MD5 Checksum               0548f7f0a8e20364fd383a7aa29c0b86
 *
 * Compiled with:  RDB Utility                unknown
 *                 RDB.pm                     15956
 *                 generate_int_id.pl         1.0
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/generate_int_id.pl
 *                 DVTSWVER                   n/a
 *
 *
 ***************************************************************************/

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
