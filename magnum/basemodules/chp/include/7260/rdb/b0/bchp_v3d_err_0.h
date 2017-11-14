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
 * Date:           Generated on               Tue Jun 27 10:52:39 2017
 *                 Full Compile MD5 Checksum  de13a1e8011803b5a40ab14e4d71d071
 *                     (minus title and desc)
 *                 MD5 Checksum               b694fcab41780597392ed5a8f558ad3e
 *
 * lock_release:   r_1255
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     1570
 *                 unknown                    unknown
 *                 Perl Interpreter           5.014001
 *                 Operating System           linux
 *                 Script Source              home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   LOCAL home/pntruong/sbin/combo_header.pl
 *
 *
********************************************************************************/

#ifndef BCHP_V3D_ERR_0_H__
#define BCHP_V3D_ERR_0_H__

/***************************************************************************
 *V3D_ERR_0 - V3D Error Registers
 ***************************************************************************/
#define BCHP_V3D_ERR_0_DBGE                      0x21308f00 /* [RO][32] PSE Error Signals */
#define BCHP_V3D_ERR_0_FDBG0                     0x21308f04 /* [RO][32] FEP Overrun Error Signals */
#define BCHP_V3D_ERR_0_FDBGB                     0x21308f08 /* [RO][32] FEP Interface Ready and Stall Signals, FEP Busy Signals */
#define BCHP_V3D_ERR_0_FDBGR                     0x21308f0c /* [RO][32] FEP Internal Ready Signals */
#define BCHP_V3D_ERR_0_FDBGS                     0x21308f10 /* [RO][32] FEP Internal Stall Input Signals */
#define BCHP_V3D_ERR_0_STAT                      0x21308f20 /* [RO][32] Miscellaneous Error Signals (VPM, VDW, VCD, VCM, L2C) */

/***************************************************************************
 *DBGE - PSE Error Signals
 ***************************************************************************/
/* V3D_ERR_0 :: DBGE :: reserved0 [31:21] */
#define BCHP_V3D_ERR_0_DBGE_reserved0_MASK                         0xffe00000
#define BCHP_V3D_ERR_0_DBGE_reserved0_SHIFT                        21

/* V3D_ERR_0 :: DBGE :: IPD2_FPDUSED [20:20] */
#define BCHP_V3D_ERR_0_DBGE_IPD2_FPDUSED_MASK                      0x00100000
#define BCHP_V3D_ERR_0_DBGE_IPD2_FPDUSED_SHIFT                     20
#define BCHP_V3D_ERR_0_DBGE_IPD2_FPDUSED_DEFAULT                   0x00000000

/* V3D_ERR_0 :: DBGE :: IPD2_VALID [19:19] */
#define BCHP_V3D_ERR_0_DBGE_IPD2_VALID_MASK                        0x00080000
#define BCHP_V3D_ERR_0_DBGE_IPD2_VALID_SHIFT                       19
#define BCHP_V3D_ERR_0_DBGE_IPD2_VALID_DEFAULT                     0x00000000

/* V3D_ERR_0 :: DBGE :: MULIP2 [18:18] */
#define BCHP_V3D_ERR_0_DBGE_MULIP2_MASK                            0x00040000
#define BCHP_V3D_ERR_0_DBGE_MULIP2_SHIFT                           18
#define BCHP_V3D_ERR_0_DBGE_MULIP2_DEFAULT                         0x00000000

/* V3D_ERR_0 :: DBGE :: MULIP1 [17:17] */
#define BCHP_V3D_ERR_0_DBGE_MULIP1_MASK                            0x00020000
#define BCHP_V3D_ERR_0_DBGE_MULIP1_SHIFT                           17
#define BCHP_V3D_ERR_0_DBGE_MULIP1_DEFAULT                         0x00000000

/* V3D_ERR_0 :: DBGE :: MULIP0 [16:16] */
#define BCHP_V3D_ERR_0_DBGE_MULIP0_MASK                            0x00010000
#define BCHP_V3D_ERR_0_DBGE_MULIP0_SHIFT                           16
#define BCHP_V3D_ERR_0_DBGE_MULIP0_DEFAULT                         0x00000000

/* V3D_ERR_0 :: DBGE :: reserved1 [15:03] */
#define BCHP_V3D_ERR_0_DBGE_reserved1_MASK                         0x0000fff8
#define BCHP_V3D_ERR_0_DBGE_reserved1_SHIFT                        3

/* V3D_ERR_0 :: DBGE :: VR1_B [02:02] */
#define BCHP_V3D_ERR_0_DBGE_VR1_B_MASK                             0x00000004
#define BCHP_V3D_ERR_0_DBGE_VR1_B_SHIFT                            2
#define BCHP_V3D_ERR_0_DBGE_VR1_B_DEFAULT                          0x00000000

/* V3D_ERR_0 :: DBGE :: VR1_A [01:01] */
#define BCHP_V3D_ERR_0_DBGE_VR1_A_MASK                             0x00000002
#define BCHP_V3D_ERR_0_DBGE_VR1_A_SHIFT                            1
#define BCHP_V3D_ERR_0_DBGE_VR1_A_DEFAULT                          0x00000000

/* V3D_ERR_0 :: DBGE :: reserved2 [00:00] */
#define BCHP_V3D_ERR_0_DBGE_reserved2_MASK                         0x00000001
#define BCHP_V3D_ERR_0_DBGE_reserved2_SHIFT                        0

/***************************************************************************
 *FDBG0 - FEP Overrun Error Signals
 ***************************************************************************/
/* V3D_ERR_0 :: FDBG0 :: reserved0 [31:18] */
#define BCHP_V3D_ERR_0_FDBG0_reserved0_MASK                        0xfffc0000
#define BCHP_V3D_ERR_0_FDBG0_reserved0_SHIFT                       18

/* V3D_ERR_0 :: FDBG0 :: EZREQ_FIFO_ORUN [17:17] */
#define BCHP_V3D_ERR_0_FDBG0_EZREQ_FIFO_ORUN_MASK                  0x00020000
#define BCHP_V3D_ERR_0_FDBG0_EZREQ_FIFO_ORUN_SHIFT                 17
#define BCHP_V3D_ERR_0_FDBG0_EZREQ_FIFO_ORUN_DEFAULT               0x00000000

/* V3D_ERR_0 :: FDBG0 :: reserved1 [16:16] */
#define BCHP_V3D_ERR_0_FDBG0_reserved1_MASK                        0x00010000
#define BCHP_V3D_ERR_0_FDBG0_reserved1_SHIFT                       16

/* V3D_ERR_0 :: FDBG0 :: EZVAL_FIFO_ORUN [15:15] */
#define BCHP_V3D_ERR_0_FDBG0_EZVAL_FIFO_ORUN_MASK                  0x00008000
#define BCHP_V3D_ERR_0_FDBG0_EZVAL_FIFO_ORUN_SHIFT                 15
#define BCHP_V3D_ERR_0_FDBG0_EZVAL_FIFO_ORUN_DEFAULT               0x00000000

/* V3D_ERR_0 :: FDBG0 :: DEPTHO_ORUN [14:14] */
#define BCHP_V3D_ERR_0_FDBG0_DEPTHO_ORUN_MASK                      0x00004000
#define BCHP_V3D_ERR_0_FDBG0_DEPTHO_ORUN_SHIFT                     14
#define BCHP_V3D_ERR_0_FDBG0_DEPTHO_ORUN_DEFAULT                   0x00000000

/* V3D_ERR_0 :: FDBG0 :: DEPTHO_FIFO_ORUN [13:13] */
#define BCHP_V3D_ERR_0_FDBG0_DEPTHO_FIFO_ORUN_MASK                 0x00002000
#define BCHP_V3D_ERR_0_FDBG0_DEPTHO_FIFO_ORUN_SHIFT                13
#define BCHP_V3D_ERR_0_FDBG0_DEPTHO_FIFO_ORUN_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBG0 :: REFXY_FIFO_ORUN [12:12] */
#define BCHP_V3D_ERR_0_FDBG0_REFXY_FIFO_ORUN_MASK                  0x00001000
#define BCHP_V3D_ERR_0_FDBG0_REFXY_FIFO_ORUN_SHIFT                 12
#define BCHP_V3D_ERR_0_FDBG0_REFXY_FIFO_ORUN_DEFAULT               0x00000000

/* V3D_ERR_0 :: FDBG0 :: ZCOEFF_FIFO_FULL [11:11] */
#define BCHP_V3D_ERR_0_FDBG0_ZCOEFF_FIFO_FULL_MASK                 0x00000800
#define BCHP_V3D_ERR_0_FDBG0_ZCOEFF_FIFO_FULL_SHIFT                11
#define BCHP_V3D_ERR_0_FDBG0_ZCOEFF_FIFO_FULL_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBG0 :: XYRELW_FIFO_ORUN [10:10] */
#define BCHP_V3D_ERR_0_FDBG0_XYRELW_FIFO_ORUN_MASK                 0x00000400
#define BCHP_V3D_ERR_0_FDBG0_XYRELW_FIFO_ORUN_SHIFT                10
#define BCHP_V3D_ERR_0_FDBG0_XYRELW_FIFO_ORUN_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBG0 :: reserved2 [09:08] */
#define BCHP_V3D_ERR_0_FDBG0_reserved2_MASK                        0x00000300
#define BCHP_V3D_ERR_0_FDBG0_reserved2_SHIFT                       8

/* V3D_ERR_0 :: FDBG0 :: XYRELO_FIFO_ORUN [07:07] */
#define BCHP_V3D_ERR_0_FDBG0_XYRELO_FIFO_ORUN_MASK                 0x00000080
#define BCHP_V3D_ERR_0_FDBG0_XYRELO_FIFO_ORUN_SHIFT                7
#define BCHP_V3D_ERR_0_FDBG0_XYRELO_FIFO_ORUN_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBG0 :: FIXZ_ORUN [06:06] */
#define BCHP_V3D_ERR_0_FDBG0_FIXZ_ORUN_MASK                        0x00000040
#define BCHP_V3D_ERR_0_FDBG0_FIXZ_ORUN_SHIFT                       6
#define BCHP_V3D_ERR_0_FDBG0_FIXZ_ORUN_DEFAULT                     0x00000000

/* V3D_ERR_0 :: FDBG0 :: XYFO_FIFO_ORUN [05:05] */
#define BCHP_V3D_ERR_0_FDBG0_XYFO_FIFO_ORUN_MASK                   0x00000020
#define BCHP_V3D_ERR_0_FDBG0_XYFO_FIFO_ORUN_SHIFT                  5
#define BCHP_V3D_ERR_0_FDBG0_XYFO_FIFO_ORUN_DEFAULT                0x00000000

/* V3D_ERR_0 :: FDBG0 :: QBSZ_FIFO_ORUN [04:04] */
#define BCHP_V3D_ERR_0_FDBG0_QBSZ_FIFO_ORUN_MASK                   0x00000010
#define BCHP_V3D_ERR_0_FDBG0_QBSZ_FIFO_ORUN_SHIFT                  4
#define BCHP_V3D_ERR_0_FDBG0_QBSZ_FIFO_ORUN_DEFAULT                0x00000000

/* V3D_ERR_0 :: FDBG0 :: QBFR_FIFO_ORUN [03:03] */
#define BCHP_V3D_ERR_0_FDBG0_QBFR_FIFO_ORUN_MASK                   0x00000008
#define BCHP_V3D_ERR_0_FDBG0_QBFR_FIFO_ORUN_SHIFT                  3
#define BCHP_V3D_ERR_0_FDBG0_QBFR_FIFO_ORUN_DEFAULT                0x00000000

/* V3D_ERR_0 :: FDBG0 :: XYRELZ_FIFO_FULL [02:02] */
#define BCHP_V3D_ERR_0_FDBG0_XYRELZ_FIFO_FULL_MASK                 0x00000004
#define BCHP_V3D_ERR_0_FDBG0_XYRELZ_FIFO_FULL_SHIFT                2
#define BCHP_V3D_ERR_0_FDBG0_XYRELZ_FIFO_FULL_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBG0 :: WCOEFF_FIFO_FULL [01:01] */
#define BCHP_V3D_ERR_0_FDBG0_WCOEFF_FIFO_FULL_MASK                 0x00000002
#define BCHP_V3D_ERR_0_FDBG0_WCOEFF_FIFO_FULL_SHIFT                1
#define BCHP_V3D_ERR_0_FDBG0_WCOEFF_FIFO_FULL_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBG0 :: reserved3 [00:00] */
#define BCHP_V3D_ERR_0_FDBG0_reserved3_MASK                        0x00000001
#define BCHP_V3D_ERR_0_FDBG0_reserved3_SHIFT                       0

/***************************************************************************
 *FDBGB - FEP Interface Ready and Stall Signals, FEP Busy Signals
 ***************************************************************************/
/* V3D_ERR_0 :: FDBGB :: reserved0 [31:29] */
#define BCHP_V3D_ERR_0_FDBGB_reserved0_MASK                        0xe0000000
#define BCHP_V3D_ERR_0_FDBGB_reserved0_SHIFT                       29

/* V3D_ERR_0 :: FDBGB :: XYFO_FIFO_OP_READY [28:28] */
#define BCHP_V3D_ERR_0_FDBGB_XYFO_FIFO_OP_READY_MASK               0x10000000
#define BCHP_V3D_ERR_0_FDBGB_XYFO_FIFO_OP_READY_SHIFT              28
#define BCHP_V3D_ERR_0_FDBGB_XYFO_FIFO_OP_READY_DEFAULT            0x00000000

/* V3D_ERR_0 :: FDBGB :: QXYF_FIFO_OP_READY [27:27] */
#define BCHP_V3D_ERR_0_FDBGB_QXYF_FIFO_OP_READY_MASK               0x08000000
#define BCHP_V3D_ERR_0_FDBGB_QXYF_FIFO_OP_READY_SHIFT              27
#define BCHP_V3D_ERR_0_FDBGB_QXYF_FIFO_OP_READY_DEFAULT            0x00000000

/* V3D_ERR_0 :: FDBGB :: RAST_BUSY [26:26] */
#define BCHP_V3D_ERR_0_FDBGB_RAST_BUSY_MASK                        0x04000000
#define BCHP_V3D_ERR_0_FDBGB_RAST_BUSY_SHIFT                       26
#define BCHP_V3D_ERR_0_FDBGB_RAST_BUSY_DEFAULT                     0x00000000

/* V3D_ERR_0 :: FDBGB :: EZ_XY_READY [25:25] */
#define BCHP_V3D_ERR_0_FDBGB_EZ_XY_READY_MASK                      0x02000000
#define BCHP_V3D_ERR_0_FDBGB_EZ_XY_READY_SHIFT                     25
#define BCHP_V3D_ERR_0_FDBGB_EZ_XY_READY_DEFAULT                   0x00000000

/* V3D_ERR_0 :: FDBGB :: reserved1 [24:24] */
#define BCHP_V3D_ERR_0_FDBGB_reserved1_MASK                        0x01000000
#define BCHP_V3D_ERR_0_FDBGB_reserved1_SHIFT                       24

/* V3D_ERR_0 :: FDBGB :: EZ_DATA_READY [23:23] */
#define BCHP_V3D_ERR_0_FDBGB_EZ_DATA_READY_MASK                    0x00800000
#define BCHP_V3D_ERR_0_FDBGB_EZ_DATA_READY_SHIFT                   23
#define BCHP_V3D_ERR_0_FDBGB_EZ_DATA_READY_DEFAULT                 0x00000000

/* V3D_ERR_0 :: FDBGB :: reserved2 [22:08] */
#define BCHP_V3D_ERR_0_FDBGB_reserved2_MASK                        0x007fff00
#define BCHP_V3D_ERR_0_FDBGB_reserved2_SHIFT                       8

/* V3D_ERR_0 :: FDBGB :: ZRWPE_READY [07:07] */
#define BCHP_V3D_ERR_0_FDBGB_ZRWPE_READY_MASK                      0x00000080
#define BCHP_V3D_ERR_0_FDBGB_ZRWPE_READY_SHIFT                     7
#define BCHP_V3D_ERR_0_FDBGB_ZRWPE_READY_DEFAULT                   0x00000000

/* V3D_ERR_0 :: FDBGB :: ZRWPE_STALL [06:06] */
#define BCHP_V3D_ERR_0_FDBGB_ZRWPE_STALL_MASK                      0x00000040
#define BCHP_V3D_ERR_0_FDBGB_ZRWPE_STALL_SHIFT                     6
#define BCHP_V3D_ERR_0_FDBGB_ZRWPE_STALL_DEFAULT                   0x00000000

/* V3D_ERR_0 :: FDBGB :: EDGES_CTRLID [05:03] */
#define BCHP_V3D_ERR_0_FDBGB_EDGES_CTRLID_MASK                     0x00000038
#define BCHP_V3D_ERR_0_FDBGB_EDGES_CTRLID_SHIFT                    3
#define BCHP_V3D_ERR_0_FDBGB_EDGES_CTRLID_DEFAULT                  0x00000000

/* V3D_ERR_0 :: FDBGB :: EDGES_ISCTRL [02:02] */
#define BCHP_V3D_ERR_0_FDBGB_EDGES_ISCTRL_MASK                     0x00000004
#define BCHP_V3D_ERR_0_FDBGB_EDGES_ISCTRL_SHIFT                    2
#define BCHP_V3D_ERR_0_FDBGB_EDGES_ISCTRL_DEFAULT                  0x00000000

/* V3D_ERR_0 :: FDBGB :: EDGES_READY [01:01] */
#define BCHP_V3D_ERR_0_FDBGB_EDGES_READY_MASK                      0x00000002
#define BCHP_V3D_ERR_0_FDBGB_EDGES_READY_SHIFT                     1
#define BCHP_V3D_ERR_0_FDBGB_EDGES_READY_DEFAULT                   0x00000000

/* V3D_ERR_0 :: FDBGB :: EDGES_STALL [00:00] */
#define BCHP_V3D_ERR_0_FDBGB_EDGES_STALL_MASK                      0x00000001
#define BCHP_V3D_ERR_0_FDBGB_EDGES_STALL_SHIFT                     0
#define BCHP_V3D_ERR_0_FDBGB_EDGES_STALL_DEFAULT                   0x00000000

/***************************************************************************
 *FDBGR - FEP Internal Ready Signals
 ***************************************************************************/
/* V3D_ERR_0 :: FDBGR :: reserved0 [31:31] */
#define BCHP_V3D_ERR_0_FDBGR_reserved0_MASK                        0x80000000
#define BCHP_V3D_ERR_0_FDBGR_reserved0_SHIFT                       31

/* V3D_ERR_0 :: FDBGR :: FIXZ_READY [30:30] */
#define BCHP_V3D_ERR_0_FDBGR_FIXZ_READY_MASK                       0x40000000
#define BCHP_V3D_ERR_0_FDBGR_FIXZ_READY_SHIFT                      30
#define BCHP_V3D_ERR_0_FDBGR_FIXZ_READY_DEFAULT                    0x00000000

/* V3D_ERR_0 :: FDBGR :: reserved1 [29:29] */
#define BCHP_V3D_ERR_0_FDBGR_reserved1_MASK                        0x20000000
#define BCHP_V3D_ERR_0_FDBGR_reserved1_SHIFT                       29

/* V3D_ERR_0 :: FDBGR :: RECIPW_READY [28:28] */
#define BCHP_V3D_ERR_0_FDBGR_RECIPW_READY_MASK                     0x10000000
#define BCHP_V3D_ERR_0_FDBGR_RECIPW_READY_SHIFT                    28
#define BCHP_V3D_ERR_0_FDBGR_RECIPW_READY_DEFAULT                  0x00000000

/* V3D_ERR_0 :: FDBGR :: INTERPRW_READY [27:27] */
#define BCHP_V3D_ERR_0_FDBGR_INTERPRW_READY_MASK                   0x08000000
#define BCHP_V3D_ERR_0_FDBGR_INTERPRW_READY_SHIFT                  27
#define BCHP_V3D_ERR_0_FDBGR_INTERPRW_READY_DEFAULT                0x00000000

/* V3D_ERR_0 :: FDBGR :: reserved2 [26:25] */
#define BCHP_V3D_ERR_0_FDBGR_reserved2_MASK                        0x06000000
#define BCHP_V3D_ERR_0_FDBGR_reserved2_SHIFT                       25

/* V3D_ERR_0 :: FDBGR :: INTERPZ_READY [24:24] */
#define BCHP_V3D_ERR_0_FDBGR_INTERPZ_READY_MASK                    0x01000000
#define BCHP_V3D_ERR_0_FDBGR_INTERPZ_READY_SHIFT                   24
#define BCHP_V3D_ERR_0_FDBGR_INTERPZ_READY_DEFAULT                 0x00000000

/* V3D_ERR_0 :: FDBGR :: XYRELZ_FIFO_LAST [23:23] */
#define BCHP_V3D_ERR_0_FDBGR_XYRELZ_FIFO_LAST_MASK                 0x00800000
#define BCHP_V3D_ERR_0_FDBGR_XYRELZ_FIFO_LAST_SHIFT                23
#define BCHP_V3D_ERR_0_FDBGR_XYRELZ_FIFO_LAST_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBGR :: XYRELZ_FIFO_READY [22:22] */
#define BCHP_V3D_ERR_0_FDBGR_XYRELZ_FIFO_READY_MASK                0x00400000
#define BCHP_V3D_ERR_0_FDBGR_XYRELZ_FIFO_READY_SHIFT               22
#define BCHP_V3D_ERR_0_FDBGR_XYRELZ_FIFO_READY_DEFAULT             0x00000000

/* V3D_ERR_0 :: FDBGR :: XYNRM_LAST [21:21] */
#define BCHP_V3D_ERR_0_FDBGR_XYNRM_LAST_MASK                       0x00200000
#define BCHP_V3D_ERR_0_FDBGR_XYNRM_LAST_SHIFT                      21
#define BCHP_V3D_ERR_0_FDBGR_XYNRM_LAST_DEFAULT                    0x00000000

/* V3D_ERR_0 :: FDBGR :: XYNRM_READY [20:20] */
#define BCHP_V3D_ERR_0_FDBGR_XYNRM_READY_MASK                      0x00100000
#define BCHP_V3D_ERR_0_FDBGR_XYNRM_READY_SHIFT                     20
#define BCHP_V3D_ERR_0_FDBGR_XYNRM_READY_DEFAULT                   0x00000000

/* V3D_ERR_0 :: FDBGR :: EZLIM_READY [19:19] */
#define BCHP_V3D_ERR_0_FDBGR_EZLIM_READY_MASK                      0x00080000
#define BCHP_V3D_ERR_0_FDBGR_EZLIM_READY_SHIFT                     19
#define BCHP_V3D_ERR_0_FDBGR_EZLIM_READY_DEFAULT                   0x00000000

/* V3D_ERR_0 :: FDBGR :: DEPTHO_READY [18:18] */
#define BCHP_V3D_ERR_0_FDBGR_DEPTHO_READY_MASK                     0x00040000
#define BCHP_V3D_ERR_0_FDBGR_DEPTHO_READY_SHIFT                    18
#define BCHP_V3D_ERR_0_FDBGR_DEPTHO_READY_DEFAULT                  0x00000000

/* V3D_ERR_0 :: FDBGR :: RAST_LAST [17:17] */
#define BCHP_V3D_ERR_0_FDBGR_RAST_LAST_MASK                        0x00020000
#define BCHP_V3D_ERR_0_FDBGR_RAST_LAST_SHIFT                       17
#define BCHP_V3D_ERR_0_FDBGR_RAST_LAST_DEFAULT                     0x00000000

/* V3D_ERR_0 :: FDBGR :: RAST_READY [16:16] */
#define BCHP_V3D_ERR_0_FDBGR_RAST_READY_MASK                       0x00010000
#define BCHP_V3D_ERR_0_FDBGR_RAST_READY_SHIFT                      16
#define BCHP_V3D_ERR_0_FDBGR_RAST_READY_DEFAULT                    0x00000000

/* V3D_ERR_0 :: FDBGR :: reserved3 [15:15] */
#define BCHP_V3D_ERR_0_FDBGR_reserved3_MASK                        0x00008000
#define BCHP_V3D_ERR_0_FDBGR_reserved3_SHIFT                       15

/* V3D_ERR_0 :: FDBGR :: XYFO_FIFO_READY [14:14] */
#define BCHP_V3D_ERR_0_FDBGR_XYFO_FIFO_READY_MASK                  0x00004000
#define BCHP_V3D_ERR_0_FDBGR_XYFO_FIFO_READY_SHIFT                 14
#define BCHP_V3D_ERR_0_FDBGR_XYFO_FIFO_READY_DEFAULT               0x00000000

/* V3D_ERR_0 :: FDBGR :: ZO_FIFO_READY [13:13] */
#define BCHP_V3D_ERR_0_FDBGR_ZO_FIFO_READY_MASK                    0x00002000
#define BCHP_V3D_ERR_0_FDBGR_ZO_FIFO_READY_SHIFT                   13
#define BCHP_V3D_ERR_0_FDBGR_ZO_FIFO_READY_DEFAULT                 0x00000000

/* V3D_ERR_0 :: FDBGR :: reserved4 [12:12] */
#define BCHP_V3D_ERR_0_FDBGR_reserved4_MASK                        0x00001000
#define BCHP_V3D_ERR_0_FDBGR_reserved4_SHIFT                       12

/* V3D_ERR_0 :: FDBGR :: XYRELO_FIFO_READY [11:11] */
#define BCHP_V3D_ERR_0_FDBGR_XYRELO_FIFO_READY_MASK                0x00000800
#define BCHP_V3D_ERR_0_FDBGR_XYRELO_FIFO_READY_SHIFT               11
#define BCHP_V3D_ERR_0_FDBGR_XYRELO_FIFO_READY_DEFAULT             0x00000000

/* V3D_ERR_0 :: FDBGR :: reserved5 [10:08] */
#define BCHP_V3D_ERR_0_FDBGR_reserved5_MASK                        0x00000700
#define BCHP_V3D_ERR_0_FDBGR_reserved5_SHIFT                       8

/* V3D_ERR_0 :: FDBGR :: WCOEFF_FIFO_READY [07:07] */
#define BCHP_V3D_ERR_0_FDBGR_WCOEFF_FIFO_READY_MASK                0x00000080
#define BCHP_V3D_ERR_0_FDBGR_WCOEFF_FIFO_READY_SHIFT               7
#define BCHP_V3D_ERR_0_FDBGR_WCOEFF_FIFO_READY_DEFAULT             0x00000000

/* V3D_ERR_0 :: FDBGR :: XYRELW_FIFO_READY [06:06] */
#define BCHP_V3D_ERR_0_FDBGR_XYRELW_FIFO_READY_MASK                0x00000040
#define BCHP_V3D_ERR_0_FDBGR_XYRELW_FIFO_READY_SHIFT               6
#define BCHP_V3D_ERR_0_FDBGR_XYRELW_FIFO_READY_DEFAULT             0x00000000

/* V3D_ERR_0 :: FDBGR :: ZCOEFF_FIFO_READY [05:05] */
#define BCHP_V3D_ERR_0_FDBGR_ZCOEFF_FIFO_READY_MASK                0x00000020
#define BCHP_V3D_ERR_0_FDBGR_ZCOEFF_FIFO_READY_SHIFT               5
#define BCHP_V3D_ERR_0_FDBGR_ZCOEFF_FIFO_READY_DEFAULT             0x00000000

/* V3D_ERR_0 :: FDBGR :: REFXY_FIFO_READY [04:04] */
#define BCHP_V3D_ERR_0_FDBGR_REFXY_FIFO_READY_MASK                 0x00000010
#define BCHP_V3D_ERR_0_FDBGR_REFXY_FIFO_READY_SHIFT                4
#define BCHP_V3D_ERR_0_FDBGR_REFXY_FIFO_READY_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBGR :: DEPTHO_FIFO_READY [03:03] */
#define BCHP_V3D_ERR_0_FDBGR_DEPTHO_FIFO_READY_MASK                0x00000008
#define BCHP_V3D_ERR_0_FDBGR_DEPTHO_FIFO_READY_SHIFT               3
#define BCHP_V3D_ERR_0_FDBGR_DEPTHO_FIFO_READY_DEFAULT             0x00000000

/* V3D_ERR_0 :: FDBGR :: EZVAL_FIFO_READY [02:02] */
#define BCHP_V3D_ERR_0_FDBGR_EZVAL_FIFO_READY_MASK                 0x00000004
#define BCHP_V3D_ERR_0_FDBGR_EZVAL_FIFO_READY_SHIFT                2
#define BCHP_V3D_ERR_0_FDBGR_EZVAL_FIFO_READY_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBGR :: EZREQ_FIFO_READY [01:01] */
#define BCHP_V3D_ERR_0_FDBGR_EZREQ_FIFO_READY_MASK                 0x00000002
#define BCHP_V3D_ERR_0_FDBGR_EZREQ_FIFO_READY_SHIFT                1
#define BCHP_V3D_ERR_0_FDBGR_EZREQ_FIFO_READY_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBGR :: QXYF_FIFO_READY [00:00] */
#define BCHP_V3D_ERR_0_FDBGR_QXYF_FIFO_READY_MASK                  0x00000001
#define BCHP_V3D_ERR_0_FDBGR_QXYF_FIFO_READY_SHIFT                 0
#define BCHP_V3D_ERR_0_FDBGR_QXYF_FIFO_READY_DEFAULT               0x00000000

/***************************************************************************
 *FDBGS - FEP Internal Stall Input Signals
 ***************************************************************************/
/* V3D_ERR_0 :: FDBGS :: reserved0 [31:29] */
#define BCHP_V3D_ERR_0_FDBGS_reserved0_MASK                        0xe0000000
#define BCHP_V3D_ERR_0_FDBGS_reserved0_SHIFT                       29

/* V3D_ERR_0 :: FDBGS :: ZO_FIFO_IP_STALL [28:28] */
#define BCHP_V3D_ERR_0_FDBGS_ZO_FIFO_IP_STALL_MASK                 0x10000000
#define BCHP_V3D_ERR_0_FDBGS_ZO_FIFO_IP_STALL_SHIFT                28
#define BCHP_V3D_ERR_0_FDBGS_ZO_FIFO_IP_STALL_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBGS :: reserved1 [27:26] */
#define BCHP_V3D_ERR_0_FDBGS_reserved1_MASK                        0x0c000000
#define BCHP_V3D_ERR_0_FDBGS_reserved1_SHIFT                       26

/* V3D_ERR_0 :: FDBGS :: RECIPW_IP_STALL [25:25] */
#define BCHP_V3D_ERR_0_FDBGS_RECIPW_IP_STALL_MASK                  0x02000000
#define BCHP_V3D_ERR_0_FDBGS_RECIPW_IP_STALL_SHIFT                 25
#define BCHP_V3D_ERR_0_FDBGS_RECIPW_IP_STALL_DEFAULT               0x00000000

/* V3D_ERR_0 :: FDBGS :: reserved2 [24:23] */
#define BCHP_V3D_ERR_0_FDBGS_reserved2_MASK                        0x01800000
#define BCHP_V3D_ERR_0_FDBGS_reserved2_SHIFT                       23

/* V3D_ERR_0 :: FDBGS :: INTERPW_IP_STALL [22:22] */
#define BCHP_V3D_ERR_0_FDBGS_INTERPW_IP_STALL_MASK                 0x00400000
#define BCHP_V3D_ERR_0_FDBGS_INTERPW_IP_STALL_SHIFT                22
#define BCHP_V3D_ERR_0_FDBGS_INTERPW_IP_STALL_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBGS :: reserved3 [21:19] */
#define BCHP_V3D_ERR_0_FDBGS_reserved3_MASK                        0x00380000
#define BCHP_V3D_ERR_0_FDBGS_reserved3_SHIFT                       19

/* V3D_ERR_0 :: FDBGS :: XYRELZ_FIFO_IP_STALL [18:18] */
#define BCHP_V3D_ERR_0_FDBGS_XYRELZ_FIFO_IP_STALL_MASK             0x00040000
#define BCHP_V3D_ERR_0_FDBGS_XYRELZ_FIFO_IP_STALL_SHIFT            18
#define BCHP_V3D_ERR_0_FDBGS_XYRELZ_FIFO_IP_STALL_DEFAULT          0x00000000

/* V3D_ERR_0 :: FDBGS :: INTERPZ_IP_STALL [17:17] */
#define BCHP_V3D_ERR_0_FDBGS_INTERPZ_IP_STALL_MASK                 0x00020000
#define BCHP_V3D_ERR_0_FDBGS_INTERPZ_IP_STALL_SHIFT                17
#define BCHP_V3D_ERR_0_FDBGS_INTERPZ_IP_STALL_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBGS :: DEPTHO_FIFO_IP_STALL [16:16] */
#define BCHP_V3D_ERR_0_FDBGS_DEPTHO_FIFO_IP_STALL_MASK             0x00010000
#define BCHP_V3D_ERR_0_FDBGS_DEPTHO_FIFO_IP_STALL_SHIFT            16
#define BCHP_V3D_ERR_0_FDBGS_DEPTHO_FIFO_IP_STALL_DEFAULT          0x00000000

/* V3D_ERR_0 :: FDBGS :: EZLIM_IP_STALL [15:15] */
#define BCHP_V3D_ERR_0_FDBGS_EZLIM_IP_STALL_MASK                   0x00008000
#define BCHP_V3D_ERR_0_FDBGS_EZLIM_IP_STALL_SHIFT                  15
#define BCHP_V3D_ERR_0_FDBGS_EZLIM_IP_STALL_DEFAULT                0x00000000

/* V3D_ERR_0 :: FDBGS :: XYNRM_IP_STALL [14:14] */
#define BCHP_V3D_ERR_0_FDBGS_XYNRM_IP_STALL_MASK                   0x00004000
#define BCHP_V3D_ERR_0_FDBGS_XYNRM_IP_STALL_SHIFT                  14
#define BCHP_V3D_ERR_0_FDBGS_XYNRM_IP_STALL_DEFAULT                0x00000000

/* V3D_ERR_0 :: FDBGS :: EZREQ_FIFO_OP_VALID [13:13] */
#define BCHP_V3D_ERR_0_FDBGS_EZREQ_FIFO_OP_VALID_MASK              0x00002000
#define BCHP_V3D_ERR_0_FDBGS_EZREQ_FIFO_OP_VALID_SHIFT             13
#define BCHP_V3D_ERR_0_FDBGS_EZREQ_FIFO_OP_VALID_DEFAULT           0x00000000

/* V3D_ERR_0 :: FDBGS :: QXYF_FIFO_OP_VALID [12:12] */
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP_VALID_MASK               0x00001000
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP_VALID_SHIFT              12
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP_VALID_DEFAULT            0x00000000

/* V3D_ERR_0 :: FDBGS :: QXYF_FIFO_OP_LAST [11:11] */
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP_LAST_MASK                0x00000800
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP_LAST_SHIFT               11
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP_LAST_DEFAULT             0x00000000

/* V3D_ERR_0 :: FDBGS :: QXYF_FIFO_OP1_DUMMY [10:10] */
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP1_DUMMY_MASK              0x00000400
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP1_DUMMY_SHIFT             10
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP1_DUMMY_DEFAULT           0x00000000

/* V3D_ERR_0 :: FDBGS :: QXYF_FIFO_OP1_LAST [09:09] */
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP1_LAST_MASK               0x00000200
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP1_LAST_SHIFT              9
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP1_LAST_DEFAULT            0x00000000

/* V3D_ERR_0 :: FDBGS :: QXYF_FIFO_OP1_VALID [08:08] */
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP1_VALID_MASK              0x00000100
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP1_VALID_SHIFT             8
#define BCHP_V3D_ERR_0_FDBGS_QXYF_FIFO_OP1_VALID_DEFAULT           0x00000000

/* V3D_ERR_0 :: FDBGS :: EZTEST_ANYQVALID [07:07] */
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_ANYQVALID_MASK                 0x00000080
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_ANYQVALID_SHIFT                7
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_ANYQVALID_DEFAULT              0x00000000

/* V3D_ERR_0 :: FDBGS :: EZTEST_ANYQF [06:06] */
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_ANYQF_MASK                     0x00000040
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_ANYQF_SHIFT                    6
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_ANYQF_DEFAULT                  0x00000000

/* V3D_ERR_0 :: FDBGS :: EZTEST_QREADY [05:05] */
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_QREADY_MASK                    0x00000020
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_QREADY_SHIFT                   5
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_QREADY_DEFAULT                 0x00000000

/* V3D_ERR_0 :: FDBGS :: EZTEST_VLF_OKNOVALID [04:04] */
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_VLF_OKNOVALID_MASK             0x00000010
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_VLF_OKNOVALID_SHIFT            4
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_VLF_OKNOVALID_DEFAULT          0x00000000

/* V3D_ERR_0 :: FDBGS :: EZTEST_STALL [03:03] */
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_STALL_MASK                     0x00000008
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_STALL_SHIFT                    3
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_STALL_DEFAULT                  0x00000000

/* V3D_ERR_0 :: FDBGS :: EZTEST_IP_VLFSTALL [02:02] */
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_IP_VLFSTALL_MASK               0x00000004
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_IP_VLFSTALL_SHIFT              2
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_IP_VLFSTALL_DEFAULT            0x00000001

/* V3D_ERR_0 :: FDBGS :: EZTEST_IP_PRSTALL [01:01] */
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_IP_PRSTALL_MASK                0x00000002
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_IP_PRSTALL_SHIFT               1
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_IP_PRSTALL_DEFAULT             0x00000001

/* V3D_ERR_0 :: FDBGS :: EZTEST_IP_QSTALL [00:00] */
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_IP_QSTALL_MASK                 0x00000001
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_IP_QSTALL_SHIFT                0
#define BCHP_V3D_ERR_0_FDBGS_EZTEST_IP_QSTALL_DEFAULT              0x00000001

/***************************************************************************
 *STAT - Miscellaneous Error Signals (VPM, VDW, VCD, VCM, L2C)
 ***************************************************************************/
/* V3D_ERR_0 :: STAT :: reserved0 [31:16] */
#define BCHP_V3D_ERR_0_STAT_reserved0_MASK                         0xffff0000
#define BCHP_V3D_ERR_0_STAT_reserved0_SHIFT                        16

/* V3D_ERR_0 :: STAT :: L2CARE [15:15] */
#define BCHP_V3D_ERR_0_STAT_L2CARE_MASK                            0x00008000
#define BCHP_V3D_ERR_0_STAT_L2CARE_SHIFT                           15
#define BCHP_V3D_ERR_0_STAT_L2CARE_DEFAULT                         0x00000000

/* V3D_ERR_0 :: STAT :: VCMBE [14:14] */
#define BCHP_V3D_ERR_0_STAT_VCMBE_MASK                             0x00004000
#define BCHP_V3D_ERR_0_STAT_VCMBE_SHIFT                            14
#define BCHP_V3D_ERR_0_STAT_VCMBE_DEFAULT                          0x00000000

/* V3D_ERR_0 :: STAT :: VCMRE [13:13] */
#define BCHP_V3D_ERR_0_STAT_VCMRE_MASK                             0x00002000
#define BCHP_V3D_ERR_0_STAT_VCMRE_SHIFT                            13
#define BCHP_V3D_ERR_0_STAT_VCMRE_DEFAULT                          0x00000000

/* V3D_ERR_0 :: STAT :: VCDI [12:12] */
#define BCHP_V3D_ERR_0_STAT_VCDI_MASK                              0x00001000
#define BCHP_V3D_ERR_0_STAT_VCDI_SHIFT                             12
#define BCHP_V3D_ERR_0_STAT_VCDI_DEFAULT                           0x00000001

/* V3D_ERR_0 :: STAT :: VCDE [11:11] */
#define BCHP_V3D_ERR_0_STAT_VCDE_MASK                              0x00000800
#define BCHP_V3D_ERR_0_STAT_VCDE_SHIFT                             11
#define BCHP_V3D_ERR_0_STAT_VCDE_DEFAULT                           0x00000000

/* V3D_ERR_0 :: STAT :: VDWE [10:10] */
#define BCHP_V3D_ERR_0_STAT_VDWE_MASK                              0x00000400
#define BCHP_V3D_ERR_0_STAT_VDWE_SHIFT                             10
#define BCHP_V3D_ERR_0_STAT_VDWE_DEFAULT                           0x00000000

/* V3D_ERR_0 :: STAT :: VPMEAS [09:09] */
#define BCHP_V3D_ERR_0_STAT_VPMEAS_MASK                            0x00000200
#define BCHP_V3D_ERR_0_STAT_VPMEAS_SHIFT                           9
#define BCHP_V3D_ERR_0_STAT_VPMEAS_DEFAULT                         0x00000000

/* V3D_ERR_0 :: STAT :: VPMEFNA [08:08] */
#define BCHP_V3D_ERR_0_STAT_VPMEFNA_MASK                           0x00000100
#define BCHP_V3D_ERR_0_STAT_VPMEFNA_SHIFT                          8
#define BCHP_V3D_ERR_0_STAT_VPMEFNA_DEFAULT                        0x00000000

/* V3D_ERR_0 :: STAT :: VPMEWNA [07:07] */
#define BCHP_V3D_ERR_0_STAT_VPMEWNA_MASK                           0x00000080
#define BCHP_V3D_ERR_0_STAT_VPMEWNA_SHIFT                          7
#define BCHP_V3D_ERR_0_STAT_VPMEWNA_DEFAULT                        0x00000000

/* V3D_ERR_0 :: STAT :: VPMERNA [06:06] */
#define BCHP_V3D_ERR_0_STAT_VPMERNA_MASK                           0x00000040
#define BCHP_V3D_ERR_0_STAT_VPMERNA_SHIFT                          6
#define BCHP_V3D_ERR_0_STAT_VPMERNA_DEFAULT                        0x00000000

/* V3D_ERR_0 :: STAT :: VPMERR [05:05] */
#define BCHP_V3D_ERR_0_STAT_VPMERR_MASK                            0x00000020
#define BCHP_V3D_ERR_0_STAT_VPMERR_SHIFT                           5
#define BCHP_V3D_ERR_0_STAT_VPMERR_DEFAULT                         0x00000000

/* V3D_ERR_0 :: STAT :: VPMEWR [04:04] */
#define BCHP_V3D_ERR_0_STAT_VPMEWR_MASK                            0x00000010
#define BCHP_V3D_ERR_0_STAT_VPMEWR_SHIFT                           4
#define BCHP_V3D_ERR_0_STAT_VPMEWR_DEFAULT                         0x00000000

/* V3D_ERR_0 :: STAT :: VPAERRGL [03:03] */
#define BCHP_V3D_ERR_0_STAT_VPAERRGL_MASK                          0x00000008
#define BCHP_V3D_ERR_0_STAT_VPAERRGL_SHIFT                         3
#define BCHP_V3D_ERR_0_STAT_VPAERRGL_DEFAULT                       0x00000000

/* V3D_ERR_0 :: STAT :: VPAEBRGL [02:02] */
#define BCHP_V3D_ERR_0_STAT_VPAEBRGL_MASK                          0x00000004
#define BCHP_V3D_ERR_0_STAT_VPAEBRGL_SHIFT                         2
#define BCHP_V3D_ERR_0_STAT_VPAEBRGL_DEFAULT                       0x00000000

/* V3D_ERR_0 :: STAT :: VPAERGS [01:01] */
#define BCHP_V3D_ERR_0_STAT_VPAERGS_MASK                           0x00000002
#define BCHP_V3D_ERR_0_STAT_VPAERGS_SHIFT                          1
#define BCHP_V3D_ERR_0_STAT_VPAERGS_DEFAULT                        0x00000000

/* V3D_ERR_0 :: STAT :: VPAEABB [00:00] */
#define BCHP_V3D_ERR_0_STAT_VPAEABB_MASK                           0x00000001
#define BCHP_V3D_ERR_0_STAT_VPAEABB_SHIFT                          0
#define BCHP_V3D_ERR_0_STAT_VPAEABB_DEFAULT                        0x00000000

#endif /* #ifndef BCHP_V3D_ERR_0_H__ */

/* End of File */
