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
 * Date:           Generated on               Tue Feb 23 15:26:07 2016
 *                 Full Compile MD5 Checksum  4b84f30a4b3665aac5b824a1ed76e56c
 *                     (minus title and desc)
 *                 MD5 Checksum               4894bba0ec078aee10b5b5954262d56e
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     804
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_DS_A_00_FECF_H__
#define BCHP_DS_A_00_FECF_H__

/***************************************************************************
 *DS_A_00_FECF - Downstream FEC Frond End Registers
 ***************************************************************************/
#define BCHP_DS_A_00_FECF_CTL1                   0x04320c00 /* [RW] FEC Control Register 1 */
#define BCHP_DS_A_00_FECF_CTL2                   0x04320c04 /* [RW] FEC Control Register */
#define BCHP_DS_A_00_FECF_LOCK_CTL1              0x04320c08 /* [RW] FEC Lock/Unlock Detection Control Register1 */
#define BCHP_DS_A_00_FECF_LOCK_CTL2              0x04320c0c /* [RW] FEC Lock/Unlock Detection Control Register2 */
#define BCHP_DS_A_00_FECF_LOCK_CTL3              0x04320c10 /* [RW] FEC Lock/Unlock Detection Control Register3 */
#define BCHP_DS_A_00_FECF_STATUS                 0x04320c14 /* [RW] FEC Status Register */

/***************************************************************************
 *CTL1 - FEC Control Register 1
 ***************************************************************************/
/* DS_A_00_FECF :: CTL1 :: NO_DEINT_FIFO_FIX [31:31] */
#define BCHP_DS_A_00_FECF_CTL1_NO_DEINT_FIFO_FIX_MASK              0x80000000
#define BCHP_DS_A_00_FECF_CTL1_NO_DEINT_FIFO_FIX_SHIFT             31
#define BCHP_DS_A_00_FECF_CTL1_NO_DEINT_FIFO_FIX_DEFAULT           0x00000000

/* DS_A_00_FECF :: CTL1 :: ECO_SPARE_2 [30:24] */
#define BCHP_DS_A_00_FECF_CTL1_ECO_SPARE_2_MASK                    0x7f000000
#define BCHP_DS_A_00_FECF_CTL1_ECO_SPARE_2_SHIFT                   24
#define BCHP_DS_A_00_FECF_CTL1_ECO_SPARE_2_DEFAULT                 0x00000000

/* DS_A_00_FECF :: CTL1 :: ECO_SPARE_1 [23:21] */
#define BCHP_DS_A_00_FECF_CTL1_ECO_SPARE_1_MASK                    0x00e00000
#define BCHP_DS_A_00_FECF_CTL1_ECO_SPARE_1_SHIFT                   21
#define BCHP_DS_A_00_FECF_CTL1_ECO_SPARE_1_DEFAULT                 0x00000000

/* DS_A_00_FECF :: CTL1 :: EMAX_A [20:16] */
#define BCHP_DS_A_00_FECF_CTL1_EMAX_A_MASK                         0x001f0000
#define BCHP_DS_A_00_FECF_CTL1_EMAX_A_SHIFT                        16
#define BCHP_DS_A_00_FECF_CTL1_EMAX_A_DEFAULT                      0x00000000

/* DS_A_00_FECF :: CTL1 :: ID_B [15:12] */
#define BCHP_DS_A_00_FECF_CTL1_ID_B_MASK                           0x0000f000
#define BCHP_DS_A_00_FECF_CTL1_ID_B_SHIFT                          12
#define BCHP_DS_A_00_FECF_CTL1_ID_B_DEFAULT                        0x00000000

/* DS_A_00_FECF :: CTL1 :: DEINT_MEM_ADDR_COR [11:11] */
#define BCHP_DS_A_00_FECF_CTL1_DEINT_MEM_ADDR_COR_MASK             0x00000800
#define BCHP_DS_A_00_FECF_CTL1_DEINT_MEM_ADDR_COR_SHIFT            11
#define BCHP_DS_A_00_FECF_CTL1_DEINT_MEM_ADDR_COR_DEFAULT          0x00000000

/* DS_A_00_FECF :: CTL1 :: AUTO_B [10:10] */
#define BCHP_DS_A_00_FECF_CTL1_AUTO_B_MASK                         0x00000400
#define BCHP_DS_A_00_FECF_CTL1_AUTO_B_SHIFT                        10
#define BCHP_DS_A_00_FECF_CTL1_AUTO_B_DEFAULT                      0x00000000

/* DS_A_00_FECF :: CTL1 :: ECO_SPARE_4 [09:09] */
#define BCHP_DS_A_00_FECF_CTL1_ECO_SPARE_4_MASK                    0x00000200
#define BCHP_DS_A_00_FECF_CTL1_ECO_SPARE_4_SHIFT                   9
#define BCHP_DS_A_00_FECF_CTL1_ECO_SPARE_4_DEFAULT                 0x00000000

/* DS_A_00_FECF :: CTL1 :: SPINV [08:08] */
#define BCHP_DS_A_00_FECF_CTL1_SPINV_MASK                          0x00000100
#define BCHP_DS_A_00_FECF_CTL1_SPINV_SHIFT                         8
#define BCHP_DS_A_00_FECF_CTL1_SPINV_DEFAULT                       0x00000000

/* DS_A_00_FECF :: CTL1 :: BPS [07:05] */
#define BCHP_DS_A_00_FECF_CTL1_BPS_MASK                            0x000000e0
#define BCHP_DS_A_00_FECF_CTL1_BPS_SHIFT                           5
#define BCHP_DS_A_00_FECF_CTL1_BPS_DEFAULT                         0x00000000

/* DS_A_00_FECF :: CTL1 :: ANNEX [04:04] */
#define BCHP_DS_A_00_FECF_CTL1_ANNEX_MASK                          0x00000010
#define BCHP_DS_A_00_FECF_CTL1_ANNEX_SHIFT                         4
#define BCHP_DS_A_00_FECF_CTL1_ANNEX_DEFAULT                       0x00000000

/* DS_A_00_FECF :: CTL1 :: AUTO_SPECTRUM [03:03] */
#define BCHP_DS_A_00_FECF_CTL1_AUTO_SPECTRUM_MASK                  0x00000008
#define BCHP_DS_A_00_FECF_CTL1_AUTO_SPECTRUM_SHIFT                 3
#define BCHP_DS_A_00_FECF_CTL1_AUTO_SPECTRUM_DEFAULT               0x00000000

/* DS_A_00_FECF :: CTL1 :: ECO_SPARE_3 [02:02] */
#define BCHP_DS_A_00_FECF_CTL1_ECO_SPARE_3_MASK                    0x00000004
#define BCHP_DS_A_00_FECF_CTL1_ECO_SPARE_3_SHIFT                   2
#define BCHP_DS_A_00_FECF_CTL1_ECO_SPARE_3_DEFAULT                 0x00000000

/* DS_A_00_FECF :: CTL1 :: ENABLE [01:01] */
#define BCHP_DS_A_00_FECF_CTL1_ENABLE_MASK                         0x00000002
#define BCHP_DS_A_00_FECF_CTL1_ENABLE_SHIFT                        1
#define BCHP_DS_A_00_FECF_CTL1_ENABLE_DEFAULT                      0x00000000

/* DS_A_00_FECF :: CTL1 :: RESET [00:00] */
#define BCHP_DS_A_00_FECF_CTL1_RESET_MASK                          0x00000001
#define BCHP_DS_A_00_FECF_CTL1_RESET_SHIFT                         0
#define BCHP_DS_A_00_FECF_CTL1_RESET_DEFAULT                       0x00000001

/***************************************************************************
 *CTL2 - FEC Control Register
 ***************************************************************************/
/* DS_A_00_FECF :: CTL2 :: ECO_SPARE_0 [31:28] */
#define BCHP_DS_A_00_FECF_CTL2_ECO_SPARE_0_MASK                    0xf0000000
#define BCHP_DS_A_00_FECF_CTL2_ECO_SPARE_0_SHIFT                   28
#define BCHP_DS_A_00_FECF_CTL2_ECO_SPARE_0_DEFAULT                 0x00000000

/* DS_A_00_FECF :: CTL2 :: FRAME_SYNC_TIME_M [27:20] */
#define BCHP_DS_A_00_FECF_CTL2_FRAME_SYNC_TIME_M_MASK              0x0ff00000
#define BCHP_DS_A_00_FECF_CTL2_FRAME_SYNC_TIME_M_SHIFT             20
#define BCHP_DS_A_00_FECF_CTL2_FRAME_SYNC_TIME_M_DEFAULT           0x000000ff

/* DS_A_00_FECF :: CTL2 :: ECO_SPARE_1 [19:18] */
#define BCHP_DS_A_00_FECF_CTL2_ECO_SPARE_1_MASK                    0x000c0000
#define BCHP_DS_A_00_FECF_CTL2_ECO_SPARE_1_SHIFT                   18
#define BCHP_DS_A_00_FECF_CTL2_ECO_SPARE_1_DEFAULT                 0x00000000

/* DS_A_00_FECF :: CTL2 :: ALIGN [17:17] */
#define BCHP_DS_A_00_FECF_CTL2_ALIGN_MASK                          0x00020000
#define BCHP_DS_A_00_FECF_CTL2_ALIGN_SHIFT                         17
#define BCHP_DS_A_00_FECF_CTL2_ALIGN_DEFAULT                       0x00000001

/* DS_A_00_FECF :: CTL2 :: MEMORY_NUMBER [16:15] */
#define BCHP_DS_A_00_FECF_CTL2_MEMORY_NUMBER_MASK                  0x00018000
#define BCHP_DS_A_00_FECF_CTL2_MEMORY_NUMBER_SHIFT                 15
#define BCHP_DS_A_00_FECF_CTL2_MEMORY_NUMBER_DEFAULT               0x00000000

/* DS_A_00_FECF :: CTL2 :: BASE_ADDRESS [14:00] */
#define BCHP_DS_A_00_FECF_CTL2_BASE_ADDRESS_MASK                   0x00007fff
#define BCHP_DS_A_00_FECF_CTL2_BASE_ADDRESS_SHIFT                  0
#define BCHP_DS_A_00_FECF_CTL2_BASE_ADDRESS_DEFAULT                0x00000000

/***************************************************************************
 *LOCK_CTL1 - FEC Lock/Unlock Detection Control Register1
 ***************************************************************************/
/* DS_A_00_FECF :: LOCK_CTL1 :: ECO_SPARE_0 [31:18] */
#define BCHP_DS_A_00_FECF_LOCK_CTL1_ECO_SPARE_0_MASK               0xfffc0000
#define BCHP_DS_A_00_FECF_LOCK_CTL1_ECO_SPARE_0_SHIFT              18
#define BCHP_DS_A_00_FECF_LOCK_CTL1_ECO_SPARE_0_DEFAULT            0x00000000

/* DS_A_00_FECF :: LOCK_CTL1 :: N_Nsp_B [17:08] */
#define BCHP_DS_A_00_FECF_LOCK_CTL1_N_Nsp_B_MASK                   0x0003ff00
#define BCHP_DS_A_00_FECF_LOCK_CTL1_N_Nsp_B_SHIFT                  8
#define BCHP_DS_A_00_FECF_LOCK_CTL1_N_Nsp_B_DEFAULT                0x00000000

/* DS_A_00_FECF :: LOCK_CTL1 :: ECO_SPARE_1 [07:00] */
#define BCHP_DS_A_00_FECF_LOCK_CTL1_ECO_SPARE_1_MASK               0x000000ff
#define BCHP_DS_A_00_FECF_LOCK_CTL1_ECO_SPARE_1_SHIFT              0
#define BCHP_DS_A_00_FECF_LOCK_CTL1_ECO_SPARE_1_DEFAULT            0x00000000

/***************************************************************************
 *LOCK_CTL2 - FEC Lock/Unlock Detection Control Register2
 ***************************************************************************/
/* DS_A_00_FECF :: LOCK_CTL2 :: F_Hg [31:24] */
#define BCHP_DS_A_00_FECF_LOCK_CTL2_F_Hg_MASK                      0xff000000
#define BCHP_DS_A_00_FECF_LOCK_CTL2_F_Hg_SHIFT                     24
#define BCHP_DS_A_00_FECF_LOCK_CTL2_F_Hg_DEFAULT                   0x00000000

/* DS_A_00_FECF :: LOCK_CTL2 :: F_Hscan [23:16] */
#define BCHP_DS_A_00_FECF_LOCK_CTL2_F_Hscan_MASK                   0x00ff0000
#define BCHP_DS_A_00_FECF_LOCK_CTL2_F_Hscan_SHIFT                  16
#define BCHP_DS_A_00_FECF_LOCK_CTL2_F_Hscan_DEFAULT                0x00000000

/* DS_A_00_FECF :: LOCK_CTL2 :: F_Hb [15:08] */
#define BCHP_DS_A_00_FECF_LOCK_CTL2_F_Hb_MASK                      0x0000ff00
#define BCHP_DS_A_00_FECF_LOCK_CTL2_F_Hb_SHIFT                     8
#define BCHP_DS_A_00_FECF_LOCK_CTL2_F_Hb_DEFAULT                   0x00000000

/* DS_A_00_FECF :: LOCK_CTL2 :: F_Hbt [07:00] */
#define BCHP_DS_A_00_FECF_LOCK_CTL2_F_Hbt_MASK                     0x000000ff
#define BCHP_DS_A_00_FECF_LOCK_CTL2_F_Hbt_SHIFT                    0
#define BCHP_DS_A_00_FECF_LOCK_CTL2_F_Hbt_DEFAULT                  0x00000000

/***************************************************************************
 *LOCK_CTL3 - FEC Lock/Unlock Detection Control Register3
 ***************************************************************************/
/* DS_A_00_FECF :: LOCK_CTL3 :: M_MPEGgc [31:24] */
#define BCHP_DS_A_00_FECF_LOCK_CTL3_M_MPEGgc_MASK                  0xff000000
#define BCHP_DS_A_00_FECF_LOCK_CTL3_M_MPEGgc_SHIFT                 24
#define BCHP_DS_A_00_FECF_LOCK_CTL3_M_MPEGgc_DEFAULT               0x00000000

/* DS_A_00_FECF :: LOCK_CTL3 :: M_MPEGb [23:12] */
#define BCHP_DS_A_00_FECF_LOCK_CTL3_M_MPEGb_MASK                   0x00fff000
#define BCHP_DS_A_00_FECF_LOCK_CTL3_M_MPEGb_SHIFT                  12
#define BCHP_DS_A_00_FECF_LOCK_CTL3_M_MPEGb_DEFAULT                0x00000000

/* DS_A_00_FECF :: LOCK_CTL3 :: M_MPEGt [11:00] */
#define BCHP_DS_A_00_FECF_LOCK_CTL3_M_MPEGt_MASK                   0x00000fff
#define BCHP_DS_A_00_FECF_LOCK_CTL3_M_MPEGt_SHIFT                  0
#define BCHP_DS_A_00_FECF_LOCK_CTL3_M_MPEGt_DEFAULT                0x00000000

/***************************************************************************
 *STATUS - FEC Status Register
 ***************************************************************************/
/* DS_A_00_FECF :: STATUS :: ECO_SPARE_3 [31:29] */
#define BCHP_DS_A_00_FECF_STATUS_ECO_SPARE_3_MASK                  0xe0000000
#define BCHP_DS_A_00_FECF_STATUS_ECO_SPARE_3_SHIFT                 29
#define BCHP_DS_A_00_FECF_STATUS_ECO_SPARE_3_DEFAULT               0x00000000

/* DS_A_00_FECF :: STATUS :: FRAME_PEAK_STATUS [28:21] */
#define BCHP_DS_A_00_FECF_STATUS_FRAME_PEAK_STATUS_MASK            0x1fe00000
#define BCHP_DS_A_00_FECF_STATUS_FRAME_PEAK_STATUS_SHIFT           21
#define BCHP_DS_A_00_FECF_STATUS_FRAME_PEAK_STATUS_DEFAULT         0x00000000

/* DS_A_00_FECF :: STATUS :: SPECTRUM_LOCKED [20:20] */
#define BCHP_DS_A_00_FECF_STATUS_SPECTRUM_LOCKED_MASK              0x00100000
#define BCHP_DS_A_00_FECF_STATUS_SPECTRUM_LOCKED_SHIFT             20
#define BCHP_DS_A_00_FECF_STATUS_SPECTRUM_LOCKED_DEFAULT           0x00000000

/* DS_A_00_FECF :: STATUS :: IDEXT_B [19:16] */
#define BCHP_DS_A_00_FECF_STATUS_IDEXT_B_MASK                      0x000f0000
#define BCHP_DS_A_00_FECF_STATUS_IDEXT_B_SHIFT                     16
#define BCHP_DS_A_00_FECF_STATUS_IDEXT_B_DEFAULT                   0x00000000

/* DS_A_00_FECF :: STATUS :: ECO_SPARE_2 [15:15] */
#define BCHP_DS_A_00_FECF_STATUS_ECO_SPARE_2_MASK                  0x00008000
#define BCHP_DS_A_00_FECF_STATUS_ECO_SPARE_2_SHIFT                 15
#define BCHP_DS_A_00_FECF_STATUS_ECO_SPARE_2_DEFAULT               0x00000000

/* DS_A_00_FECF :: STATUS :: FRAME_U_TO_L [14:14] */
#define BCHP_DS_A_00_FECF_STATUS_FRAME_U_TO_L_MASK                 0x00004000
#define BCHP_DS_A_00_FECF_STATUS_FRAME_U_TO_L_SHIFT                14
#define BCHP_DS_A_00_FECF_STATUS_FRAME_U_TO_L_DEFAULT              0x00000000

/* DS_A_00_FECF :: STATUS :: FRAME_L_TO_U [13:13] */
#define BCHP_DS_A_00_FECF_STATUS_FRAME_L_TO_U_MASK                 0x00002000
#define BCHP_DS_A_00_FECF_STATUS_FRAME_L_TO_U_SHIFT                13
#define BCHP_DS_A_00_FECF_STATUS_FRAME_L_TO_U_DEFAULT              0x00000000

/* DS_A_00_FECF :: STATUS :: FRAME_LOCK [12:12] */
#define BCHP_DS_A_00_FECF_STATUS_FRAME_LOCK_MASK                   0x00001000
#define BCHP_DS_A_00_FECF_STATUS_FRAME_LOCK_SHIFT                  12
#define BCHP_DS_A_00_FECF_STATUS_FRAME_LOCK_DEFAULT                0x00000000

/* DS_A_00_FECF :: STATUS :: DEINT_MEM_ADDR_OOR [11:11] */
#define BCHP_DS_A_00_FECF_STATUS_DEINT_MEM_ADDR_OOR_MASK           0x00000800
#define BCHP_DS_A_00_FECF_STATUS_DEINT_MEM_ADDR_OOR_SHIFT          11
#define BCHP_DS_A_00_FECF_STATUS_DEINT_MEM_ADDR_OOR_DEFAULT        0x00000000

/* DS_A_00_FECF :: STATUS :: NODE_U_TO_L [10:10] */
#define BCHP_DS_A_00_FECF_STATUS_NODE_U_TO_L_MASK                  0x00000400
#define BCHP_DS_A_00_FECF_STATUS_NODE_U_TO_L_SHIFT                 10
#define BCHP_DS_A_00_FECF_STATUS_NODE_U_TO_L_DEFAULT               0x00000000

/* DS_A_00_FECF :: STATUS :: NODE_L_TO_U [09:09] */
#define BCHP_DS_A_00_FECF_STATUS_NODE_L_TO_U_MASK                  0x00000200
#define BCHP_DS_A_00_FECF_STATUS_NODE_L_TO_U_SHIFT                 9
#define BCHP_DS_A_00_FECF_STATUS_NODE_L_TO_U_DEFAULT               0x00000000

/* DS_A_00_FECF :: STATUS :: NODE_LOCK [08:08] */
#define BCHP_DS_A_00_FECF_STATUS_NODE_LOCK_MASK                    0x00000100
#define BCHP_DS_A_00_FECF_STATUS_NODE_LOCK_SHIFT                   8
#define BCHP_DS_A_00_FECF_STATUS_NODE_LOCK_DEFAULT                 0x00000000

/* DS_A_00_FECF :: STATUS :: ECO_SPARE_0 [07:00] */
#define BCHP_DS_A_00_FECF_STATUS_ECO_SPARE_0_MASK                  0x000000ff
#define BCHP_DS_A_00_FECF_STATUS_ECO_SPARE_0_SHIFT                 0
#define BCHP_DS_A_00_FECF_STATUS_ECO_SPARE_0_DEFAULT               0x00000000

/***************************************************************************
 *HIST_%i - History memory content
 ***************************************************************************/
#define BCHP_DS_A_00_FECF_HIST_i_ARRAY_BASE                        0x04320c18
#define BCHP_DS_A_00_FECF_HIST_i_ARRAY_START                       0
#define BCHP_DS_A_00_FECF_HIST_i_ARRAY_END                         89
#define BCHP_DS_A_00_FECF_HIST_i_ARRAY_ELEMENT_SIZE                32

/***************************************************************************
 *HIST_%i - History memory content
 ***************************************************************************/
/* DS_A_00_FECF :: HIST_i :: DATA [31:00] */
#define BCHP_DS_A_00_FECF_HIST_i_DATA_MASK                         0xffffffff
#define BCHP_DS_A_00_FECF_HIST_i_DATA_SHIFT                        0
#define BCHP_DS_A_00_FECF_HIST_i_DATA_DEFAULT                      0x00000000


#endif /* #ifndef BCHP_DS_A_00_FECF_H__ */

/* End of File */
