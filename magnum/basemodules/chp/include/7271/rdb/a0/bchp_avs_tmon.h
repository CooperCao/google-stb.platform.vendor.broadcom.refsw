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
 * Date:           Generated on               Thu Jun 18 10:52:56 2015
 *                 Full Compile MD5 Checksum  32b78c1804e11666b824f2b9450a6228
 *                     (minus title and desc)
 *                 MD5 Checksum               3452ff65b8043c1c458e059705af3b49
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     16265
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_AVS_TMON_H__
#define BCHP_AVS_TMON_H__

/***************************************************************************
 *AVS_TMON - AVS Temperature Monitor
 ***************************************************************************/
#define BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS 0x204d1500 /* [RO] Indicate temperature measurement data and validity of data */
#define BCHP_AVS_TMON_ENABLE_OVER_TEMPERATURE_RESET 0x204d1504 /* [RW] Enable over temperature reset */
#define BCHP_AVS_TMON_TEMPERATURE_RESET_THRESHOLD 0x204d1508 /* [RW] Represent threshold for over temperature reset */
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_IDLE_TIME 0x204d1510 /* [RW] No new temperature interrupt can be generated for interval defined by this idle time */
#define BCHP_AVS_TMON_ENABLE_TEMPERATURE_INTERRUPT_SOURCES 0x204d1514 /* [RW] Enable high and low temperature interrupts */
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_THRESHOLDS 0x204d1518 /* [RW] Represent thresholds for high and low temperature interrupts */
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_TEMPERATURE 0x204d151c /* [RO] Temperature code associated with temperature interrupt event */
#define BCHP_AVS_TMON_TP_TMON_TEST_ENABLE        0x204d1520 /* [RW] Enabling TP mode to use data from tp_in for testing TMON */
#define BCHP_AVS_TMON_SPARE_0                    0x204d1524 /* [RW] Spare register 0 for AVS TEMPERATURE MONITOR core */

/***************************************************************************
 *TEMPERATURE_MEASUREMENT_STATUS - Indicate temperature measurement data and validity of data
 ***************************************************************************/
/* AVS_TMON :: TEMPERATURE_MEASUREMENT_STATUS :: reserved0 [31:12] */
#define BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS_reserved0_MASK 0xfffff000
#define BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS_reserved0_SHIFT 12

/* AVS_TMON :: TEMPERATURE_MEASUREMENT_STATUS :: valid [11:11] */
#define BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS_valid_MASK    0x00000800
#define BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS_valid_SHIFT   11
#define BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS_valid_DEFAULT 0x00000000

/* AVS_TMON :: TEMPERATURE_MEASUREMENT_STATUS :: data [10:00] */
#define BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS_data_MASK     0x000007ff
#define BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS_data_SHIFT    0
#define BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS_data_DEFAULT  0x00000000

/***************************************************************************
 *ENABLE_OVER_TEMPERATURE_RESET - Enable over temperature reset
 ***************************************************************************/
/* AVS_TMON :: ENABLE_OVER_TEMPERATURE_RESET :: reserved0 [31:01] */
#define BCHP_AVS_TMON_ENABLE_OVER_TEMPERATURE_RESET_reserved0_MASK 0xfffffffe
#define BCHP_AVS_TMON_ENABLE_OVER_TEMPERATURE_RESET_reserved0_SHIFT 1

/* AVS_TMON :: ENABLE_OVER_TEMPERATURE_RESET :: enable [00:00] */
#define BCHP_AVS_TMON_ENABLE_OVER_TEMPERATURE_RESET_enable_MASK    0x00000001
#define BCHP_AVS_TMON_ENABLE_OVER_TEMPERATURE_RESET_enable_SHIFT   0
#define BCHP_AVS_TMON_ENABLE_OVER_TEMPERATURE_RESET_enable_DEFAULT 0x00000000

/***************************************************************************
 *TEMPERATURE_RESET_THRESHOLD - Represent threshold for over temperature reset
 ***************************************************************************/
/* AVS_TMON :: TEMPERATURE_RESET_THRESHOLD :: reserved0 [31:11] */
#define BCHP_AVS_TMON_TEMPERATURE_RESET_THRESHOLD_reserved0_MASK   0xfffff800
#define BCHP_AVS_TMON_TEMPERATURE_RESET_THRESHOLD_reserved0_SHIFT  11

/* AVS_TMON :: TEMPERATURE_RESET_THRESHOLD :: threshold [10:00] */
#define BCHP_AVS_TMON_TEMPERATURE_RESET_THRESHOLD_threshold_MASK   0x000007ff
#define BCHP_AVS_TMON_TEMPERATURE_RESET_THRESHOLD_threshold_SHIFT  0
#define BCHP_AVS_TMON_TEMPERATURE_RESET_THRESHOLD_threshold_DEFAULT 0x0000047e

/***************************************************************************
 *TEMPERATURE_INTERRUPT_IDLE_TIME - No new temperature interrupt can be generated for interval defined by this idle time
 ***************************************************************************/
/* AVS_TMON :: TEMPERATURE_INTERRUPT_IDLE_TIME :: reserved0 [31:08] */
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_IDLE_TIME_reserved0_MASK 0xffffff00
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_IDLE_TIME_reserved0_SHIFT 8

/* AVS_TMON :: TEMPERATURE_INTERRUPT_IDLE_TIME :: idle_time [07:00] */
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_IDLE_TIME_idle_time_MASK 0x000000ff
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_IDLE_TIME_idle_time_SHIFT 0
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_IDLE_TIME_idle_time_DEFAULT 0x00000040

/***************************************************************************
 *ENABLE_TEMPERATURE_INTERRUPT_SOURCES - Enable high and low temperature interrupts
 ***************************************************************************/
/* AVS_TMON :: ENABLE_TEMPERATURE_INTERRUPT_SOURCES :: reserved0 [31:02] */
#define BCHP_AVS_TMON_ENABLE_TEMPERATURE_INTERRUPT_SOURCES_reserved0_MASK 0xfffffffc
#define BCHP_AVS_TMON_ENABLE_TEMPERATURE_INTERRUPT_SOURCES_reserved0_SHIFT 2

/* AVS_TMON :: ENABLE_TEMPERATURE_INTERRUPT_SOURCES :: enable_high [01:01] */
#define BCHP_AVS_TMON_ENABLE_TEMPERATURE_INTERRUPT_SOURCES_enable_high_MASK 0x00000002
#define BCHP_AVS_TMON_ENABLE_TEMPERATURE_INTERRUPT_SOURCES_enable_high_SHIFT 1
#define BCHP_AVS_TMON_ENABLE_TEMPERATURE_INTERRUPT_SOURCES_enable_high_DEFAULT 0x00000000

/* AVS_TMON :: ENABLE_TEMPERATURE_INTERRUPT_SOURCES :: enable_low [00:00] */
#define BCHP_AVS_TMON_ENABLE_TEMPERATURE_INTERRUPT_SOURCES_enable_low_MASK 0x00000001
#define BCHP_AVS_TMON_ENABLE_TEMPERATURE_INTERRUPT_SOURCES_enable_low_SHIFT 0
#define BCHP_AVS_TMON_ENABLE_TEMPERATURE_INTERRUPT_SOURCES_enable_low_DEFAULT 0x00000000

/***************************************************************************
 *TEMPERATURE_INTERRUPT_THRESHOLDS - Represent thresholds for high and low temperature interrupts
 ***************************************************************************/
/* AVS_TMON :: TEMPERATURE_INTERRUPT_THRESHOLDS :: reserved0 [31:27] */
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_THRESHOLDS_reserved0_MASK 0xf8000000
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_THRESHOLDS_reserved0_SHIFT 27

/* AVS_TMON :: TEMPERATURE_INTERRUPT_THRESHOLDS :: high_threshold [26:16] */
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_THRESHOLDS_high_threshold_MASK 0x07ff0000
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_THRESHOLDS_high_threshold_SHIFT 16
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_THRESHOLDS_high_threshold_DEFAULT 0x0000049a

/* AVS_TMON :: TEMPERATURE_INTERRUPT_THRESHOLDS :: reserved1 [15:11] */
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_THRESHOLDS_reserved1_MASK 0x0000f800
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_THRESHOLDS_reserved1_SHIFT 11

/* AVS_TMON :: TEMPERATURE_INTERRUPT_THRESHOLDS :: low_threshold [10:00] */
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_THRESHOLDS_low_threshold_MASK 0x000007ff
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_THRESHOLDS_low_threshold_SHIFT 0
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_THRESHOLDS_low_threshold_DEFAULT 0x000004a8

/***************************************************************************
 *TEMPERATURE_INTERRUPT_TEMPERATURE - Temperature code associated with temperature interrupt event
 ***************************************************************************/
/* AVS_TMON :: TEMPERATURE_INTERRUPT_TEMPERATURE :: reserved0 [31:10] */
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_TEMPERATURE_reserved0_MASK 0xfffffc00
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_TEMPERATURE_reserved0_SHIFT 10

/* AVS_TMON :: TEMPERATURE_INTERRUPT_TEMPERATURE :: code [09:00] */
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_TEMPERATURE_code_MASK  0x000003ff
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_TEMPERATURE_code_SHIFT 0
#define BCHP_AVS_TMON_TEMPERATURE_INTERRUPT_TEMPERATURE_code_DEFAULT 0x00000000

/***************************************************************************
 *TP_TMON_TEST_ENABLE - Enabling TP mode to use data from tp_in for testing TMON
 ***************************************************************************/
/* AVS_TMON :: TP_TMON_TEST_ENABLE :: reserved0 [31:01] */
#define BCHP_AVS_TMON_TP_TMON_TEST_ENABLE_reserved0_MASK           0xfffffffe
#define BCHP_AVS_TMON_TP_TMON_TEST_ENABLE_reserved0_SHIFT          1

/* AVS_TMON :: TP_TMON_TEST_ENABLE :: tp_mode_en [00:00] */
#define BCHP_AVS_TMON_TP_TMON_TEST_ENABLE_tp_mode_en_MASK          0x00000001
#define BCHP_AVS_TMON_TP_TMON_TEST_ENABLE_tp_mode_en_SHIFT         0
#define BCHP_AVS_TMON_TP_TMON_TEST_ENABLE_tp_mode_en_DEFAULT       0x00000000

/***************************************************************************
 *SPARE_0 - Spare register 0 for AVS TEMPERATURE MONITOR core
 ***************************************************************************/
/* AVS_TMON :: SPARE_0 :: spare [31:00] */
#define BCHP_AVS_TMON_SPARE_0_spare_MASK                           0xffffffff
#define BCHP_AVS_TMON_SPARE_0_spare_SHIFT                          0
#define BCHP_AVS_TMON_SPARE_0_spare_DEFAULT                        0x00000000

#endif /* #ifndef BCHP_AVS_TMON_H__ */

/* End of File */
