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
 * Date:           Generated on               Fri Aug 21 14:43:27 2015
 *                 Full Compile MD5 Checksum  6f40c93fa7adf1b7b596c84d59590a10
 *                     (minus title and desc)
 *                 MD5 Checksum               b1b8c76af39c441b8e9ab1ae2930543d
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     88
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_HDMI_RX_SHARED_H__
#define BCHP_HDMI_RX_SHARED_H__

/***************************************************************************
 *HDMI_RX_SHARED - HDMI Receiver Shared Control Registers
 ***************************************************************************/
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0 0x206f0c00 /* [RW] First HDMI Receiver Digital Front End Configuration Register */
#define BCHP_HDMI_RX_SHARED_HDCP_INTEGRITY_CFG   0x206f0c04 /* [RW] HDCP INTEGRITY Check Configuration Parameters. */
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_0 0x206f0c08 /* [RW] Defines the window position to capture the general control packet */
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_1 0x206f0c0c /* [RW] Defines the window latency delay through BCH block */
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_DELAY_CFG   0x206f0c10 /* [RW] HDCP I<sup><small>2</sup></small>C Configuration */
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG    0x206f0c14 /* [RW] HDCP I<sup><small>2</sup></small>C Configuration */
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG 0x206f0c18 /* [RW] SCDCS I<sup><small>2</sup></small>C Configuration */
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL 0x206f0c1c /* [RW] I2C_GEN_START_STOP_CONTROL */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG 0x206f0c20 /* [RW] I2C_FREQ_DETECTION_CFG */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1 0x206f0c24 /* [RW] I2C_FREQ_DETECTION_CNTRL1 */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL2 0x206f0c28 /* [RW] I2C_FREQ_DETECTION_CNTRL2 */

/***************************************************************************
 *DIGITAL_FRONT_END_CFG_0 - First HDMI Receiver Digital Front End Configuration Register
 ***************************************************************************/
/* HDMI_RX_SHARED :: DIGITAL_FRONT_END_CFG_0 :: PREAMBLE_THRESHOLD [31:28] */
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_PREAMBLE_THRESHOLD_MASK 0xf0000000
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_PREAMBLE_THRESHOLD_SHIFT 28
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_PREAMBLE_THRESHOLD_DEFAULT 0x00000004

/* HDMI_RX_SHARED :: DIGITAL_FRONT_END_CFG_0 :: HYSTERISIS_THRESHOLD [27:24] */
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_HYSTERISIS_THRESHOLD_MASK 0x0f000000
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_HYSTERISIS_THRESHOLD_SHIFT 24
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_HYSTERISIS_THRESHOLD_DEFAULT 0x00000004

/* HDMI_RX_SHARED :: DIGITAL_FRONT_END_CFG_0 :: UPDATE_THRESHOLD [23:20] */
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_UPDATE_THRESHOLD_MASK 0x00f00000
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_UPDATE_THRESHOLD_SHIFT 20
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_UPDATE_THRESHOLD_DEFAULT 0x0000000a

/* HDMI_RX_SHARED :: DIGITAL_FRONT_END_CFG_0 :: reserved0 [19:04] */
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_reserved0_MASK 0x000ffff0
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_reserved0_SHIFT 4

/* HDMI_RX_SHARED :: DIGITAL_FRONT_END_CFG_0 :: SYMBOL_LOCK_THRESHOLD [03:00] */
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_SYMBOL_LOCK_THRESHOLD_MASK 0x0000000f
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_SYMBOL_LOCK_THRESHOLD_SHIFT 0
#define BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0_SYMBOL_LOCK_THRESHOLD_DEFAULT 0x0000000a

/***************************************************************************
 *HDCP_INTEGRITY_CFG - HDCP INTEGRITY Check Configuration Parameters.
 ***************************************************************************/
/* HDMI_RX_SHARED :: HDCP_INTEGRITY_CFG :: reserved0 [31:15] */
#define BCHP_HDMI_RX_SHARED_HDCP_INTEGRITY_CFG_reserved0_MASK      0xffff8000
#define BCHP_HDMI_RX_SHARED_HDCP_INTEGRITY_CFG_reserved0_SHIFT     15

/* HDMI_RX_SHARED :: HDCP_INTEGRITY_CFG :: J_RATE_6_0 [14:08] */
#define BCHP_HDMI_RX_SHARED_HDCP_INTEGRITY_CFG_J_RATE_6_0_MASK     0x00007f00
#define BCHP_HDMI_RX_SHARED_HDCP_INTEGRITY_CFG_J_RATE_6_0_SHIFT    8
#define BCHP_HDMI_RX_SHARED_HDCP_INTEGRITY_CFG_J_RATE_6_0_DEFAULT  0x00000010

/* HDMI_RX_SHARED :: HDCP_INTEGRITY_CFG :: reserved1 [07:07] */
#define BCHP_HDMI_RX_SHARED_HDCP_INTEGRITY_CFG_reserved1_MASK      0x00000080
#define BCHP_HDMI_RX_SHARED_HDCP_INTEGRITY_CFG_reserved1_SHIFT     7

/* HDMI_RX_SHARED :: HDCP_INTEGRITY_CFG :: I_RATE_6_0 [06:00] */
#define BCHP_HDMI_RX_SHARED_HDCP_INTEGRITY_CFG_I_RATE_6_0_MASK     0x0000007f
#define BCHP_HDMI_RX_SHARED_HDCP_INTEGRITY_CFG_I_RATE_6_0_SHIFT    0
#define BCHP_HDMI_RX_SHARED_HDCP_INTEGRITY_CFG_I_RATE_6_0_DEFAULT  0x00000000

/***************************************************************************
 *HDCP_GCP_WINDOW_CFG_0 - Defines the window position to capture the general control packet
 ***************************************************************************/
/* HDMI_RX_SHARED :: HDCP_GCP_WINDOW_CFG_0 :: reserved0 [31:26] */
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_0_reserved0_MASK   0xfc000000
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_0_reserved0_SHIFT  26

/* HDMI_RX_SHARED :: HDCP_GCP_WINDOW_CFG_0 :: END_GCP_WINDOW [25:16] */
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_0_END_GCP_WINDOW_MASK 0x03ff0000
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_0_END_GCP_WINDOW_SHIFT 16
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_0_END_GCP_WINDOW_DEFAULT 0x0000017f

/* HDMI_RX_SHARED :: HDCP_GCP_WINDOW_CFG_0 :: reserved1 [15:10] */
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_0_reserved1_MASK   0x0000fc00
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_0_reserved1_SHIFT  10

/* HDMI_RX_SHARED :: HDCP_GCP_WINDOW_CFG_0 :: START_GCP_WINDOW [09:00] */
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_0_START_GCP_WINDOW_MASK 0x000003ff
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_0_START_GCP_WINDOW_SHIFT 0
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_0_START_GCP_WINDOW_DEFAULT 0x0000001f

/***************************************************************************
 *HDCP_GCP_WINDOW_CFG_1 - Defines the window latency delay through BCH block
 ***************************************************************************/
/* HDMI_RX_SHARED :: HDCP_GCP_WINDOW_CFG_1 :: reserved0 [31:16] */
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_1_reserved0_MASK   0xffff0000
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_1_reserved0_SHIFT  16

/* HDMI_RX_SHARED :: HDCP_GCP_WINDOW_CFG_1 :: DELAY_GCP_WINDOW_BCH_ENABLED [15:08] */
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_1_DELAY_GCP_WINDOW_BCH_ENABLED_MASK 0x0000ff00
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_1_DELAY_GCP_WINDOW_BCH_ENABLED_SHIFT 8
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_1_DELAY_GCP_WINDOW_BCH_ENABLED_DEFAULT 0x00000050

/* HDMI_RX_SHARED :: HDCP_GCP_WINDOW_CFG_1 :: DELAY_GCP_WINDOW_BCH_DISABLED [07:00] */
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_1_DELAY_GCP_WINDOW_BCH_DISABLED_MASK 0x000000ff
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_1_DELAY_GCP_WINDOW_BCH_DISABLED_SHIFT 0
#define BCHP_HDMI_RX_SHARED_HDCP_GCP_WINDOW_CFG_1_DELAY_GCP_WINDOW_BCH_DISABLED_DEFAULT 0x00000023

/***************************************************************************
 *HDCP_I2C_DELAY_CFG - HDCP I<sup><small>2</sup></small>C Configuration
 ***************************************************************************/
/* HDMI_RX_SHARED :: HDCP_I2C_DELAY_CFG :: reserved0 [31:24] */
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_DELAY_CFG_reserved0_MASK      0xff000000
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_DELAY_CFG_reserved0_SHIFT     24

/* HDMI_RX_SHARED :: HDCP_I2C_DELAY_CFG :: I2C_WRITE_DELAY [23:16] */
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_DELAY_CFG_I2C_WRITE_DELAY_MASK 0x00ff0000
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_DELAY_CFG_I2C_WRITE_DELAY_SHIFT 16
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_DELAY_CFG_I2C_WRITE_DELAY_DEFAULT 0x00000008

/* HDMI_RX_SHARED :: HDCP_I2C_DELAY_CFG :: I2C_HOLD_DELAY [15:08] */
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_DELAY_CFG_I2C_HOLD_DELAY_MASK 0x0000ff00
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_DELAY_CFG_I2C_HOLD_DELAY_SHIFT 8
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_DELAY_CFG_I2C_HOLD_DELAY_DEFAULT 0x00000008

/* HDMI_RX_SHARED :: HDCP_I2C_DELAY_CFG :: I2C_SAMPLE_DELAY [07:00] */
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_DELAY_CFG_I2C_SAMPLE_DELAY_MASK 0x000000ff
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_DELAY_CFG_I2C_SAMPLE_DELAY_SHIFT 0
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_DELAY_CFG_I2C_SAMPLE_DELAY_DEFAULT 0x00000008

/***************************************************************************
 *HDCP_I2C_MISC_CFG - HDCP I<sup><small>2</sup></small>C Configuration
 ***************************************************************************/
/* HDMI_RX_SHARED :: HDCP_I2C_MISC_CFG :: I2C_LPF_FILTER_THRESHOLD [31:24] */
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_LPF_FILTER_THRESHOLD_MASK 0xff000000
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_LPF_FILTER_THRESHOLD_SHIFT 24
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_LPF_FILTER_THRESHOLD_DEFAULT 0x00000008

/* HDMI_RX_SHARED :: HDCP_I2C_MISC_CFG :: I2C_FAST_ADDRESS [23:16] */
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_FAST_ADDRESS_MASK 0x00ff0000
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_FAST_ADDRESS_SHIFT 16
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_FAST_ADDRESS_DEFAULT 0x00000008

/* HDMI_RX_SHARED :: HDCP_I2C_MISC_CFG :: I2C_NO_INCREMENT_OFFSET [15:08] */
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_NO_INCREMENT_OFFSET_MASK 0x0000ff00
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_NO_INCREMENT_OFFSET_SHIFT 8
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_NO_INCREMENT_OFFSET_DEFAULT 0x00000043

/* HDMI_RX_SHARED :: HDCP_I2C_MISC_CFG :: I2C_NO_INCREMENT_ENABLE [07:07] */
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_NO_INCREMENT_ENABLE_MASK 0x00000080
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_NO_INCREMENT_ENABLE_SHIFT 7
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_NO_INCREMENT_ENABLE_DEFAULT 0x00000001

/* HDMI_RX_SHARED :: HDCP_I2C_MISC_CFG :: I2C_ADDRESS [06:00] */
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_ADDRESS_MASK     0x0000007f
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_ADDRESS_SHIFT    0
#define BCHP_HDMI_RX_SHARED_HDCP_I2C_MISC_CFG_I2C_ADDRESS_DEFAULT  0x0000003a

/***************************************************************************
 *SCDCS_I2C_MISC_CONFIG - SCDCS I<sup><small>2</sup></small>C Configuration
 ***************************************************************************/
/* HDMI_RX_SHARED :: SCDCS_I2C_MISC_CONFIG :: I2C_SCDC_CLEAR_ON_HOTPLUG_IN [31:31] */
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_CLEAR_ON_HOTPLUG_IN_MASK 0x80000000
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_CLEAR_ON_HOTPLUG_IN_SHIFT 31
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_CLEAR_ON_HOTPLUG_IN_DEFAULT 0x00000001

/* HDMI_RX_SHARED :: SCDCS_I2C_MISC_CONFIG :: reserved0 [30:24] */
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_reserved0_MASK   0x7f000000
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_reserved0_SHIFT  24

/* HDMI_RX_SHARED :: SCDCS_I2C_MISC_CONFIG :: I2C_SCDC_FAST_ADDRESS [23:16] */
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_FAST_ADDRESS_MASK 0x00ff0000
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_FAST_ADDRESS_SHIFT 16
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_FAST_ADDRESS_DEFAULT 0x00000010

/* HDMI_RX_SHARED :: SCDCS_I2C_MISC_CONFIG :: I2C_SCDC_NO_INCREMENT_OFFSET [15:08] */
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_NO_INCREMENT_OFFSET_MASK 0x0000ff00
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_NO_INCREMENT_OFFSET_SHIFT 8
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_NO_INCREMENT_OFFSET_DEFAULT 0x00000000

/* HDMI_RX_SHARED :: SCDCS_I2C_MISC_CONFIG :: I2C_SCDC_NO_INCREMENT_ENABLE [07:07] */
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_NO_INCREMENT_ENABLE_MASK 0x00000080
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_NO_INCREMENT_ENABLE_SHIFT 7
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_NO_INCREMENT_ENABLE_DEFAULT 0x00000000

/* HDMI_RX_SHARED :: SCDCS_I2C_MISC_CONFIG :: I2C_SCDC_ADDRESS [06:00] */
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_ADDRESS_MASK 0x0000007f
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_ADDRESS_SHIFT 0
#define BCHP_HDMI_RX_SHARED_SCDCS_I2C_MISC_CONFIG_I2C_SCDC_ADDRESS_DEFAULT 0x00000054

/***************************************************************************
 *I2C_GEN_START_STOP_CONTROL - I2C_GEN_START_STOP_CONTROL
 ***************************************************************************/
/* HDMI_RX_SHARED :: I2C_GEN_START_STOP_CONTROL :: reserved0 [31:30] */
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_reserved0_MASK 0xc0000000
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_reserved0_SHIFT 30

/* HDMI_RX_SHARED :: I2C_GEN_START_STOP_CONTROL :: SDA_DN_DELAY [29:24] */
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_SDA_DN_DELAY_MASK 0x3f000000
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_SDA_DN_DELAY_SHIFT 24
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_SDA_DN_DELAY_DEFAULT 0x00000000

/* HDMI_RX_SHARED :: I2C_GEN_START_STOP_CONTROL :: reserved1 [23:22] */
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_reserved1_MASK 0x00c00000
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_reserved1_SHIFT 22

/* HDMI_RX_SHARED :: I2C_GEN_START_STOP_CONTROL :: SDA_UP_DELAY [21:16] */
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_SDA_UP_DELAY_MASK 0x003f0000
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_SDA_UP_DELAY_SHIFT 16
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_SDA_UP_DELAY_DEFAULT 0x00000000

/* HDMI_RX_SHARED :: I2C_GEN_START_STOP_CONTROL :: reserved2 [15:14] */
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_reserved2_MASK 0x0000c000
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_reserved2_SHIFT 14

/* HDMI_RX_SHARED :: I2C_GEN_START_STOP_CONTROL :: SCL_DELAY_STOP [13:08] */
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_SCL_DELAY_STOP_MASK 0x00003f00
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_SCL_DELAY_STOP_SHIFT 8
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_SCL_DELAY_STOP_DEFAULT 0x00000000

/* HDMI_RX_SHARED :: I2C_GEN_START_STOP_CONTROL :: reserved3 [07:06] */
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_reserved3_MASK 0x000000c0
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_reserved3_SHIFT 6

/* HDMI_RX_SHARED :: I2C_GEN_START_STOP_CONTROL :: SCL_DELAY_START [05:00] */
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_SCL_DELAY_START_MASK 0x0000003f
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_SCL_DELAY_START_SHIFT 0
#define BCHP_HDMI_RX_SHARED_I2C_GEN_START_STOP_CONTROL_SCL_DELAY_START_DEFAULT 0x00000000

/***************************************************************************
 *I2C_FREQ_DETECTION_CFG - I2C_FREQ_DETECTION_CFG
 ***************************************************************************/
/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CFG :: reserved0 [31:08] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_reserved0_MASK  0xffffff00
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_reserved0_SHIFT 8

/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CFG :: FD_SEL_BIT_COUNT_TRB [07:07] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_SEL_BIT_COUNT_TRB_MASK 0x00000080
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_SEL_BIT_COUNT_TRB_SHIFT 7
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_SEL_BIT_COUNT_TRB_DEFAULT 0x00000000

/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CFG :: FD_SEL_SCL_CYCLE [06:03] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_SEL_SCL_CYCLE_MASK 0x00000078
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_SEL_SCL_CYCLE_SHIFT 3
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_SEL_SCL_CYCLE_DEFAULT 0x00000004

/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CFG :: FD_ENABLE_TIMEOUT [02:02] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_ENABLE_TIMEOUT_MASK 0x00000004
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_ENABLE_TIMEOUT_SHIFT 2
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_ENABLE_TIMEOUT_DEFAULT 0x00000000

/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CFG :: FD_ENABLE_FREQ_CHECK [01:01] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_ENABLE_FREQ_CHECK_MASK 0x00000002
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_ENABLE_FREQ_CHECK_SHIFT 1
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_ENABLE_FREQ_CHECK_DEFAULT 0x00000001

/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CFG :: FD_ENABLE_FREQ_DETECTION [00:00] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_ENABLE_FREQ_DETECTION_MASK 0x00000001
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_ENABLE_FREQ_DETECTION_SHIFT 0
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CFG_FD_ENABLE_FREQ_DETECTION_DEFAULT 0x00000001

/***************************************************************************
 *I2C_FREQ_DETECTION_CNTRL1 - I2C_FREQ_DETECTION_CNTRL1
 ***************************************************************************/
/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CNTRL1 :: reserved0 [31:24] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1_reserved0_MASK 0xff000000
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1_reserved0_SHIFT 24

/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CNTRL1 :: FD_SCL_TIMEOUT_THRESHOLD [23:08] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1_FD_SCL_TIMEOUT_THRESHOLD_MASK 0x00ffff00
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1_FD_SCL_TIMEOUT_THRESHOLD_SHIFT 8
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1_FD_SCL_TIMEOUT_THRESHOLD_DEFAULT 0x0000ffff

/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CNTRL1 :: reserved1 [07:02] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1_reserved1_MASK 0x000000fc
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1_reserved1_SHIFT 2

/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CNTRL1 :: FD_RST_TIMEOUT_STAT [01:01] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1_FD_RST_TIMEOUT_STAT_MASK 0x00000002
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1_FD_RST_TIMEOUT_STAT_SHIFT 1
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1_FD_RST_TIMEOUT_STAT_DEFAULT 0x00000000

/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CNTRL1 :: FD_RST_TRANSACTION_COUNT [00:00] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1_FD_RST_TRANSACTION_COUNT_MASK 0x00000001
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1_FD_RST_TRANSACTION_COUNT_SHIFT 0
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL1_FD_RST_TRANSACTION_COUNT_DEFAULT 0x00000000

/***************************************************************************
 *I2C_FREQ_DETECTION_CNTRL2 - I2C_FREQ_DETECTION_CNTRL2
 ***************************************************************************/
/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CNTRL2 :: reserved0 [31:25] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL2_reserved0_MASK 0xfe000000
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL2_reserved0_SHIFT 25

/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CNTRL2 :: FD_SCL_FREQ_HIGH_LIMIT [24:16] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL2_FD_SCL_FREQ_HIGH_LIMIT_MASK 0x01ff0000
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL2_FD_SCL_FREQ_HIGH_LIMIT_SHIFT 16
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL2_FD_SCL_FREQ_HIGH_LIMIT_DEFAULT 0x000000ff

/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CNTRL2 :: reserved1 [15:13] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL2_reserved1_MASK 0x0000e000
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL2_reserved1_SHIFT 13

/* HDMI_RX_SHARED :: I2C_FREQ_DETECTION_CNTRL2 :: FD_SCL_FREQ_LOW_LIMIT [12:00] */
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL2_FD_SCL_FREQ_LOW_LIMIT_MASK 0x00001fff
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL2_FD_SCL_FREQ_LOW_LIMIT_SHIFT 0
#define BCHP_HDMI_RX_SHARED_I2C_FREQ_DETECTION_CNTRL2_FD_SCL_FREQ_LOW_LIMIT_DEFAULT 0x00001fff

#endif /* #ifndef BCHP_HDMI_RX_SHARED_H__ */

/* End of File */
