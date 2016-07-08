/****************************************************************************
 *     Copyright (c) 1999-2016, Broadcom Corporation
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
 * Date:           Generated on               Mon Feb  8 12:54:55 2016
 *                 Full Compile MD5 Checksum  7c463a9180016920b3e03273285ff33d
 *                     (minus title and desc)
 *                 MD5 Checksum               30fed0099690880293569d98807ed1d8
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                unknown
 *                 RDB.pm                     749
 *                 generate_int_id.pl         1.0
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/generate_int_id.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#include "bchp.h"
#include "bchp_hdmi_tx_scdc_intr2_0.h"

#ifndef BCHP_INT_ID_HDMI_TX_SCDC_INTR2_0_H__
#define BCHP_INT_ID_HDMI_TX_SCDC_INTR2_0_H__

#define BCHP_INT_ID_I2C_CH0_DONE_INTR         BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_I2C_CH0_DONE_INTR_SHIFT)
#define BCHP_INT_ID_I2C_CH1_DONE_INTR         BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_I2C_CH1_DONE_INTR_SHIFT)
#define BCHP_INT_ID_I2C_CH2_DONE_INTR         BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_I2C_CH2_DONE_INTR_SHIFT)
#define BCHP_INT_ID_I2C_CH3_DONE_INTR         BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_I2C_CH3_DONE_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_00_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_00_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_01_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_01_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_02_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_02_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_03_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_03_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_04_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_04_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_05_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_05_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_06_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_06_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_07_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_07_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_08_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_08_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_09_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_09_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_10_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_10_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_11_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_11_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_12_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_12_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_13_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_13_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_14_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_14_INTR_SHIFT)
#define BCHP_INT_ID_UPDATE_15_INTR            BCHP_INT_ID_CREATE(BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS, BCHP_HDMI_TX_SCDC_INTR2_0_CPU_STATUS_UPDATE_15_INTR_SHIFT)

#endif /* #ifndef BCHP_INT_ID_HDMI_TX_SCDC_INTR2_0_H__ */

/* End of File */
