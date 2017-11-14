/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 * The launch point for all information concerning RDB is found at:
 *   http://bcgbu.broadcom.com/RDB/SitePages/Home.aspx
 *
 * Date:           Generated on               Thu Apr 13 10:09:30 2017
 *                 Full Compile MD5 Checksum  7f180d7646477bba2bae1a701efd9ef5
 *                     (minus title and desc)
 *                 MD5 Checksum               a2a4a53aa20c0c2f46073b879159b85d
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     1395
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   LOCAL tools/dvtsw/current/Linux/combo_header.pl
 *
 *
********************************************************************************/

#ifndef BCHP_HEVD_OL_CTL_0_H__
#define BCHP_HEVD_OL_CTL_0_H__

/***************************************************************************
 *HEVD_OL_CTL_0
 ***************************************************************************/
#define BCHP_HEVD_OL_CTL_0_CPU_ID                0x00c14004 /* [RO][32] CPU ID Regsiter */
#define BCHP_HEVD_OL_CTL_0_ENDIAN                0x00c14008 /* [RO][32] Stream Endian Control Register */
#define BCHP_HEVD_OL_CTL_0_BVN_INT               0x00c1400c /* [RO][32] BVN Interrupt Register */
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE           0x00c14010 /* [CFG][32] Clock Gate Register */
#define BCHP_HEVD_OL_CTL_0_TIMER_REG             0x00c14014 /* [RW][32] Timer */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL     0x00c14018 /* [CFG][32] Soft Shutdown */
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_CTL        0x00c1401c /* [CFG][32] Shim Debug Control */
#define BCHP_HEVD_OL_CTL_0_SHIM_DEBUG_READ       0x00c14020 /* [RO][32] Shim Debug Read */
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT       0x00c14024 /* [CFG][32] RM2 ARBITER TIMEOUT Register */
#define BCHP_HEVD_OL_CTL_0_RM2_ARB_TIMEOUT_SEEN  0x00c14028 /* [RW][32] RM2 Timeout detected */
#define BCHP_HEVD_OL_CTL_0_FIRMWARE_DEBUG        0x00c14030 /* [RW][32] Spare register bits for firmware debug state tracing */
#define BCHP_HEVD_OL_CTL_0_OTP_CTL_REG           0x00c14034 /* [RO][32] OTP Control Bits */
#define BCHP_HEVD_OL_CTL_0_SHIM_ERROR_REG        0x00c14038 /* [RO][32] SCB Shim error register */
#define BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_STATUS   0x00c1403c /* [RO][32] Soft Shutdown Status */
#define BCHP_HEVD_OL_CTL_0_CLIENT_INIT_CONFIG    0x00c14040 /* [CFG][32] Client Init Config */
#define BCHP_HEVD_OL_CTL_0_HVD_REG_BASE          0x00c14060 /* [RO][64] Global I/O Base for HVD registers */
#define BCHP_HEVD_OL_CTL_0_MSAT_BASE             0x00c14068 /* [RO][64] Global I/O Base for MSAT registers */
#define BCHP_HEVD_OL_CTL_0_PMU_BASE              0x00c14070 /* [RO][64] Global I/O Base for PMU registers */

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

/* HEVD_OL_CTL_0 :: OL_CLK_GATE :: reserved1 [06:01] */
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_reserved1_MASK              0x0000007e
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_reserved1_SHIFT             1

/* HEVD_OL_CTL_0 :: OL_CLK_GATE :: clk_lp_mode [00:00] */
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_clk_lp_mode_MASK            0x00000001
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_clk_lp_mode_SHIFT           0
#define BCHP_HEVD_OL_CTL_0_OL_CLK_GATE_clk_lp_mode_DEFAULT         0x00000000

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
 *HVD_REG_BASE - Global I/O Base for HVD registers
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: HVD_REG_BASE :: reserved0 [63:40] */
#define BCHP_HEVD_OL_CTL_0_HVD_REG_BASE_reserved0_MASK             BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_HEVD_OL_CTL_0_HVD_REG_BASE_reserved0_SHIFT            40

/* HEVD_OL_CTL_0 :: HVD_REG_BASE :: BaseAddr [39:00] */
#define BCHP_HEVD_OL_CTL_0_HVD_REG_BASE_BaseAddr_MASK              BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_HEVD_OL_CTL_0_HVD_REG_BASE_BaseAddr_SHIFT             0
#define BCHP_HEVD_OL_CTL_0_HVD_REG_BASE_BaseAddr_DEFAULT           0

/***************************************************************************
 *MSAT_BASE - Global I/O Base for MSAT registers
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: MSAT_BASE :: reserved0 [63:40] */
#define BCHP_HEVD_OL_CTL_0_MSAT_BASE_reserved0_MASK                BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_HEVD_OL_CTL_0_MSAT_BASE_reserved0_SHIFT               40

/* HEVD_OL_CTL_0 :: MSAT_BASE :: BaseAddr [39:00] */
#define BCHP_HEVD_OL_CTL_0_MSAT_BASE_BaseAddr_MASK                 BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_HEVD_OL_CTL_0_MSAT_BASE_BaseAddr_SHIFT                0

/***************************************************************************
 *PMU_BASE - Global I/O Base for PMU registers
 ***************************************************************************/
/* HEVD_OL_CTL_0 :: PMU_BASE :: reserved0 [63:40] */
#define BCHP_HEVD_OL_CTL_0_PMU_BASE_reserved0_MASK                 BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_HEVD_OL_CTL_0_PMU_BASE_reserved0_SHIFT                40

/* HEVD_OL_CTL_0 :: PMU_BASE :: BaseAddr [39:00] */
#define BCHP_HEVD_OL_CTL_0_PMU_BASE_BaseAddr_MASK                  BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_HEVD_OL_CTL_0_PMU_BASE_BaseAddr_SHIFT                 0

/***************************************************************************
 *STC_REG%i - Serial Time Stamp PTS
 ***************************************************************************/
#define BCHP_HEVD_OL_CTL_0_STC_REGi_ARRAY_BASE                     0x00c14100
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
#define BCHP_HEVD_OL_CTL_0_ENTRYi_ARRAY_BASE                       0x00c15000
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
