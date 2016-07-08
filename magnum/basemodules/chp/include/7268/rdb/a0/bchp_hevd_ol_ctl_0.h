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
 * Date:           Generated on               Mon Nov 30 09:49:01 2015
 *                 Full Compile MD5 Checksum  1e9e6fafcad3e4be7081dbf62ac498b1
 *                     (minus title and desc)
 *                 MD5 Checksum               e2d1ec86648badd2d3ecac8333ba3ddb
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     535
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
 ***************************************************************************/

#ifndef BCHP_HEVD_OL_CTL_0_H__
#define BCHP_HEVD_OL_CTL_0_H__

/***************************************************************************
 *HEVD_OL_CTL_0
 ***************************************************************************/
#define BCHP_HEVD_OL_CTL_0_CPU_ID                0x20014004 /* [RO] CPU ID Regsiter */
#define BCHP_HEVD_OL_CTL_0_ENDIAN                0x20014008 /* [RO] Stream Endian Control Register */
#define BCHP_HEVD_OL_CTL_0_BVN_INT               0x2001400c /* [RO] BVN Interrupt Register */
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE           0x20014010 /* [CFG] Clock Gate Register */
#define BCHP_HEVD_OL_CTL_0_TIMER_REG             0x20014014 /* [RW] Timer */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL     0x20014018 /* [CFG] Soft Shutdown */
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL        0x2001401c /* [CFG] Shim Debug Control */
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_READ       0x20014020 /* [RO] Shim Debug Read */
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT       0x20014024 /* [CFG] RM2 ARBITER TIMEOUT Register */
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT_SEEN  0x20014028 /* [RW] RM2 Timeout detected */
#define BCHP_HEVD_OL_CTL_0_FIRMWARE_DEBUG        0x20014030 /* [RW] Spare register bits for firmware debug state tracing */
#define BCHP_HEVD_OL_CTL_0_OTP_CTL_REG           0x20014034 /* [RO] OTP Control Bits */
#define BCHP_HEVD_OL_CTL_0_SHIM_ERROR_REG        0x20014038 /* [RO] SCB Shim error register */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS   0x2001403c /* [RO] Soft Shutdown Status */
#define BCHP_HEVD_OL_CTL_0_CLIENT_INIT_CONFIG    0x20014040 /* [CFG] Client Init Config */

/***************************************************************************
 *CPU_ID - CPU ID Regsiter
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: CPU_ID :: reserved0 [31:24] */
#define BCHP_HEVD_OL_CTL_0_CPU_ID_reserved0_MASK                   0xff000000
#define BCHP_HEVD_OL_CTL_0_CPU_ID_reserved0_SHIFT                  24

/* HEVD_OL_CTL_0 :: CPU_ID :: BLCPU_ID [23:16] */
#define BCHP_HEVD_OL_CTL_0_CPU_ID_BLCPU_ID_MASK                    0x00ff0000
#define BCHP_HEVD_OL_CTL_0_CPU_ID_BLCPU_ID_SHIFT                   16

/* HEVD_OL_CTL_0 :: CPU_ID :: ILCPU_ID [15:08] */
#define BCHP_HEVD_OL_CTL_0_CPU_ID_ILCPU_ID_MASK                    0x0000ff00
#define BCHP_HEVD_OL_CTL_0_CPU_ID_ILCPU_ID_SHIFT                   8

/* HEVD_OL_CTL_0 :: CPU_ID :: OLCPU_ID [07:00] */
#define BCHP_HEVD_OL_CTL_0_CPU_ID_OLCPU_ID_MASK                    0x000000ff
#define BCHP_HEVD_OL_CTL_0_CPU_ID_OLCPU_ID_SHIFT                   0

/***************************************************************************
 *ENDIAN - Stream Endian Control Register
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: ENDIAN :: reserved0 [31:02] */
#define BCHP_HEVD_OL_CTL_0_ENDIAN_reserved0_MASK                   0xfffffffc
#define BCHP_HEVD_OL_CTL_0_ENDIAN_reserved0_SHIFT                  2

/* HEVD_OL_CTL_0 :: ENDIAN :: avs_plus_present [01:01] */
#define BCHP_HEVD_OL_CTL_0_ENDIAN_avs_plus_present_MASK            0x00000002
#define BCHP_HEVD_OL_CTL_0_ENDIAN_avs_plus_present_SHIFT           1
#define BCHP_HEVD_OL_CTL_0_ENDIAN_avs_plus_present_DEFAULT         0x00000001

/* HEVD_OL_CTL_0 :: ENDIAN :: B1L0 [00:00] */
#define BCHP_HEVD_OL_CTL_0_ENDIAN_B1L0_MASK                        0x00000001
#define BCHP_HEVD_OL_CTL_0_ENDIAN_B1L0_SHIFT                       0

/***************************************************************************
 *BVN_INT - BVN Interrupt Register
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: BVN_INT :: reserved0 [31:04] */
#define BCHP_HEVD_OL_CTL_0_BVN_INT_reserved0_MASK                  0xfffffff0
#define BCHP_HEVD_OL_CTL_0_BVN_INT_reserved0_SHIFT                 4

/* HEVD_OL_CTL_0 :: BVN_INT :: desc2 [03:03] */
#define BCHP_HEVD_OL_CTL_0_BVN_INT_desc2_MASK                      0x00000008
#define BCHP_HEVD_OL_CTL_0_BVN_INT_desc2_SHIFT                     3
#define BCHP_HEVD_OL_CTL_0_BVN_INT_desc2_DEFAULT                   0x00000000

/* HEVD_OL_CTL_0 :: BVN_INT :: reserved1 [02:02] */
#define BCHP_HEVD_OL_CTL_0_BVN_INT_reserved1_MASK                  0x00000004
#define BCHP_HEVD_OL_CTL_0_BVN_INT_reserved1_SHIFT                 2

/* HEVD_OL_CTL_0 :: BVN_INT :: desc [01:01] */
#define BCHP_HEVD_OL_CTL_0_BVN_INT_desc_MASK                       0x00000002
#define BCHP_HEVD_OL_CTL_0_BVN_INT_desc_SHIFT                      1
#define BCHP_HEVD_OL_CTL_0_BVN_INT_desc_DEFAULT                    0x00000000

/* HEVD_OL_CTL_0 :: BVN_INT :: reserved2 [00:00] */
#define BCHP_HEVD_OL_CTL_0_BVN_INT_reserved2_MASK                  0x00000001
#define BCHP_HEVD_OL_CTL_0_BVN_INT_reserved2_SHIFT                 0

/***************************************************************************
 *OL_CLK_GATE - Clock Gate Register
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: OL_CLK_GATE :: reserved0 [31:09] */
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_reserved0_MASK              0xfffffe00
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_reserved0_SHIFT             9

/* HEVD_OL_CTL_0 :: OL_CLK_GATE :: clk_cab_hevd [08:08] */
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_clk_cab_hevd_MASK           0x00000100
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_clk_cab_hevd_SHIFT          8
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_clk_cab_hevd_DEFAULT        0x00000000

/* HEVD_OL_CTL_0 :: OL_CLK_GATE :: clk_cab [07:07] */
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_clk_cab_MASK                0x00000080
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_clk_cab_SHIFT               7
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_clk_cab_DEFAULT             0x00000000

/* HEVD_OL_CTL_0 :: OL_CLK_GATE :: reserved1 [06:00] */
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_reserved1_MASK              0x0000007f
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_reserved1_SHIFT             0

/***************************************************************************
 *TIMER_REG - Timer
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: TIMER_REG :: TimerValue [31:00] */
#define BCHP_HEVD_OL_CTL_0_TIMER_REG_TimerValue_MASK               0xffffffff
#define BCHP_HEVD_OL_CTL_0_TIMER_REG_TimerValue_SHIFT              0

/***************************************************************************
 *SOFTSHUTDOWN_CTRL - Soft Shutdown
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: SOFTSHUTDOWN_CTRL :: reserved0 [31:01] */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL_reserved0_MASK        0xfffffffe
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL_reserved0_SHIFT       1

/* HEVD_OL_CTL_0 :: SOFTSHUTDOWN_CTRL :: Enable [00:00] */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL_Enable_MASK           0x00000001
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL_Enable_SHIFT          0
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL_Enable_DEFAULT        0x00000000

/***************************************************************************
 *SHIM_DEBUG_CTL - Shim Debug Control
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: SHIM_DEBUG_CTL :: reserved0 [31:14] */
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_reserved0_MASK           0xffffc000
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_reserved0_SHIFT          14

/* HEVD_OL_CTL_0 :: SHIM_DEBUG_CTL :: SCB_debug [13:13] */
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_SCB_debug_MASK           0x00002000
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_SCB_debug_SHIFT          13
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_SCB_debug_DEFAULT        0x00000000

/* HEVD_OL_CTL_0 :: SHIM_DEBUG_CTL :: reserved1 [12:10] */
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_reserved1_MASK           0x00001c00
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_reserved1_SHIFT          10

/* HEVD_OL_CTL_0 :: SHIM_DEBUG_CTL :: SCB_debug_select [09:08] */
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_SCB_debug_select_MASK    0x00000300
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_SCB_debug_select_SHIFT   8
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_SCB_debug_select_DEFAULT 0x00000000

/* HEVD_OL_CTL_0 :: SHIM_DEBUG_CTL :: reserved2 [07:03] */
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_reserved2_MASK           0x000000f8
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_reserved2_SHIFT          3

/* HEVD_OL_CTL_0 :: SHIM_DEBUG_CTL :: SCB_status_sel [02:01] */
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_SCB_status_sel_MASK      0x00000006
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_SCB_status_sel_SHIFT     1
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_SCB_status_sel_DEFAULT   0x00000000

/* HEVD_OL_CTL_0 :: SHIM_DEBUG_CTL :: SCB_err_enable [00:00] */
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_SCB_err_enable_MASK      0x00000001
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_SCB_err_enable_SHIFT     0
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL_SCB_err_enable_DEFAULT   0x00000000

/***************************************************************************
 *SHIM_DEBUG_READ - Shim Debug Read
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: SHIM_DEBUG_READ :: SCB_shim_debug_data [31:00] */
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_READ_SCB_shim_debug_data_MASK 0xffffffff
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_READ_SCB_shim_debug_data_SHIFT 0

/***************************************************************************
 *RM2_ARB_TIMEOUT - RM2 ARBITER TIMEOUT Register
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: RM2_ARB_TIMEOUT :: RM2_arbiter_timeout_value [31:08] */
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT_RM2_arbiter_timeout_value_MASK 0xffffff00
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT_RM2_arbiter_timeout_value_SHIFT 8
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT_RM2_arbiter_timeout_value_DEFAULT 0x000003ff

/* HEVD_OL_CTL_0 :: RM2_ARB_TIMEOUT :: reserved0 [07:00] */
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT_reserved0_MASK          0x000000ff
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT_reserved0_SHIFT         0

/***************************************************************************
 *RM2_ARB_TIMEOUT_SEEN - RM2 Timeout detected
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: RM2_ARB_TIMEOUT_SEEN :: reserved0 [31:01] */
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT_SEEN_reserved0_MASK     0xfffffffe
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT_SEEN_reserved0_SHIFT    1

/* HEVD_OL_CTL_0 :: RM2_ARB_TIMEOUT_SEEN :: TimeoutSeen [00:00] */
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT_SEEN_TimeoutSeen_MASK   0x00000001
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT_SEEN_TimeoutSeen_SHIFT  0
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT_SEEN_TimeoutSeen_DEFAULT 0x00000000

/***************************************************************************
 *FIRMWARE_DEBUG - Spare register bits for firmware debug state tracing
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: FIRMWARE_DEBUG :: Reserved [31:04] */
#define BCHP_HEVD_OL_CTL_0_FIRMWARE_DEBUG_Reserved_MASK            0xfffffff0
#define BCHP_HEVD_OL_CTL_0_FIRMWARE_DEBUG_Reserved_SHIFT           4

/* HEVD_OL_CTL_0 :: FIRMWARE_DEBUG :: STATE [03:00] */
#define BCHP_HEVD_OL_CTL_0_FIRMWARE_DEBUG_STATE_MASK               0x0000000f
#define BCHP_HEVD_OL_CTL_0_FIRMWARE_DEBUG_STATE_SHIFT              0
#define BCHP_HEVD_OL_CTL_0_FIRMWARE_DEBUG_STATE_DEFAULT            0x00000000

/***************************************************************************
 *OTP_CTL_REG - OTP Control Bits
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: OTP_CTL_REG :: reserved0 [31:03] */
#define BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_reserved0_MASK              0xfffffff8
#define BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_reserved0_SHIFT             3

/* HEVD_OL_CTL_0 :: OTP_CTL_REG :: disable_Spark [02:02] */
#define BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_disable_Spark_MASK          0x00000004
#define BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_disable_Spark_SHIFT         2

/* HEVD_OL_CTL_0 :: OTP_CTL_REG :: disable_VP6 [01:01] */
#define BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_disable_VP6_MASK            0x00000002
#define BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_disable_VP6_SHIFT           1

/* HEVD_OL_CTL_0 :: OTP_CTL_REG :: disable_RV9 [00:00] */
#define BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_disable_RV9_MASK            0x00000001
#define BCHP_HEVD_OL_CTL_0_OTP_CTL_REG_disable_RV9_SHIFT           0

/***************************************************************************
 *SHIM_ERROR_REG - SCB Shim error register
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: SHIM_ERROR_REG :: SCB_shim_error [31:00] */
#define BCHP_HEVD_OL_CTL_0_SHIM_ERROR_REG_SCB_shim_error_MASK      0xffffffff
#define BCHP_HEVD_OL_CTL_0_SHIM_ERROR_REG_SCB_shim_error_SHIFT     0

/***************************************************************************
 *SOFTSHUTDOWN_STATUS - Soft Shutdown Status
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: SOFTSHUTDOWN_STATUS :: reserved0 [31:10] */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_reserved0_MASK      0xfffffc00
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_reserved0_SHIFT     10

/* HEVD_OL_CTL_0 :: SOFTSHUTDOWN_STATUS :: Client_init_ack_DB2 [09:09] */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_DB2_MASK 0x00000200
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_DB2_SHIFT 9
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_DB2_DEFAULT 0x00000000

/* HEVD_OL_CTL_0 :: SOFTSHUTDOWN_STATUS :: Client_init_ack_SI2 [08:08] */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_SI2_MASK 0x00000100
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_SI2_SHIFT 8
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_SI2_DEFAULT 0x00000000

/* HEVD_OL_CTL_0 :: SOFTSHUTDOWN_STATUS :: Client_init_ack_IL2 [07:07] */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_IL2_MASK 0x00000080
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_IL2_SHIFT 7
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_IL2_DEFAULT 0x00000000

/* HEVD_OL_CTL_0 :: SOFTSHUTDOWN_STATUS :: Client_init_ack_pfri [06:06] */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_pfri_MASK 0x00000040
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_pfri_SHIFT 6
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_pfri_DEFAULT 0x00000000

/* HEVD_OL_CTL_0 :: SOFTSHUTDOWN_STATUS :: Client_init_ack_DB [05:05] */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_DB_MASK 0x00000020
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_DB_SHIFT 5
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_DB_DEFAULT 0x00000000

/* HEVD_OL_CTL_0 :: SOFTSHUTDOWN_STATUS :: Client_init_ack_SI [04:04] */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_SI_MASK 0x00000010
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_SI_SHIFT 4
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_SI_DEFAULT 0x00000000

/* HEVD_OL_CTL_0 :: SOFTSHUTDOWN_STATUS :: Client_init_ack_CAB [03:03] */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_CAB_MASK 0x00000008
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_CAB_SHIFT 3
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_CAB_DEFAULT 0x00000000

/* HEVD_OL_CTL_0 :: SOFTSHUTDOWN_STATUS :: Client_init_ack_IL [02:02] */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_IL_MASK 0x00000004
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_IL_SHIFT 2
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_IL_DEFAULT 0x00000000

/* HEVD_OL_CTL_0 :: SOFTSHUTDOWN_STATUS :: Client_init_ack_OL [01:01] */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_OL_MASK 0x00000002
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_OL_SHIFT 1
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Client_init_ack_OL_DEFAULT 0x00000000

/* HEVD_OL_CTL_0 :: SOFTSHUTDOWN_STATUS :: Done [00:00] */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Done_MASK           0x00000001
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Done_SHIFT          0
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS_Done_DEFAULT        0x00000000

/***************************************************************************
 *CLIENT_INIT_CONFIG - Client Init Config
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: CLIENT_INIT_CONFIG :: reserved0 [31:01] */
#define BCHP_HEVD_OL_CTL_0_CLIENT_INIT_CONFIG_reserved0_MASK       0xfffffffe
#define BCHP_HEVD_OL_CTL_0_CLIENT_INIT_CONFIG_reserved0_SHIFT      1

/* HEVD_OL_CTL_0 :: CLIENT_INIT_CONFIG :: client_init_disable [00:00] */
#define BCHP_HEVD_OL_CTL_0_CLIENT_INIT_CONFIG_client_init_disable_MASK 0x00000001
#define BCHP_HEVD_OL_CTL_0_CLIENT_INIT_CONFIG_client_init_disable_SHIFT 0
#define BCHP_HEVD_OL_CTL_0_CLIENT_INIT_CONFIG_client_init_disable_DEFAULT 0x00000000

/***************************************************************************
 *STC_REG%i - Serial Time Stamp PTS
 ***************************************************************************/
#define BCHP_HEVD_OL_CTL_0_STC_REGi_ARRAY_BASE                     0x20014100
#define BCHP_HEVD_OL_CTL_0_STC_REGi_ARRAY_START                    0
#define BCHP_HEVD_OL_CTL_0_STC_REGi_ARRAY_END                      15
#define BCHP_HEVD_OL_CTL_0_STC_REGi_ARRAY_ELEMENT_SIZE             32

/***************************************************************************
 *STC_REG%i - Serial Time Stamp PTS
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: STC_REGi :: STC [31:00] */
#define BCHP_HEVD_OL_CTL_0_STC_REGi_STC_MASK                       0xffffffff
#define BCHP_HEVD_OL_CTL_0_STC_REGi_STC_SHIFT                      0


/***************************************************************************
 *ENTRY%i - Shared scratch RAM
 ***************************************************************************/
#define BCHP_HEVD_OL_CTL_0_ENTRYi_ARRAY_BASE                       0x20015000
#define BCHP_HEVD_OL_CTL_0_ENTRYi_ARRAY_START                      0
#define BCHP_HEVD_OL_CTL_0_ENTRYi_ARRAY_END                        255
#define BCHP_HEVD_OL_CTL_0_ENTRYi_ARRAY_ELEMENT_SIZE               32

/***************************************************************************
 *ENTRY%i - Shared scratch RAM
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: ENTRYi :: ENTRY [31:00] */
#define BCHP_HEVD_OL_CTL_0_ENTRYi_ENTRY_MASK                       0xffffffff
#define BCHP_HEVD_OL_CTL_0_ENTRYi_ENTRY_SHIFT                      0


#endif /* #ifndef BCHP_HEVD_OL_CTL_0_H__ */

/* End of File */
