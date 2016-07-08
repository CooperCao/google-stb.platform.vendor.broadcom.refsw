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
 * Date:           Generated on               Fri Feb 26 13:24:09 2016
 *                 Full Compile MD5 Checksum  1560bfee4f086d6e1d49e6bd3406a38d
 *                     (minus title and desc)
 *                 MD5 Checksum               8d7264bb382089f88abd2b1abb2a6340
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     823
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_RAAGA_DSP_MISC_H__
#define BCHP_RAAGA_DSP_MISC_H__

/***************************************************************************
 *RAAGA_DSP_MISC - Raaga DSP MISC Registers
 ***************************************************************************/
#define BCHP_RAAGA_DSP_MISC_REVISION             0x20c20000 /* [RO] Audio DSP System Revision Register */
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT            0x20c20004 /* [RW] Audio DSP System Soft Reset Control */
#define BCHP_RAAGA_DSP_MISC_CORE_ID              0x20c20008 /* [RO] DSP Core ID */
#define BCHP_RAAGA_DSP_MISC_PROC_ID              0x20c2000c /* [RO] DSP Processor ID */
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_0       0x20c20010 /* [RW] FP Integration test register */
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_1       0x20c20014 /* [RW] FP Integration test register */
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_2       0x20c20018 /* [RW] FP Integration test register */
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_3       0x20c2001c /* [RW] FP Integration test register */
#define BCHP_RAAGA_DSP_MISC_SCRATCH_0            0x20c20020 /* [RW] Raaga Scratch register */
#define BCHP_RAAGA_DSP_MISC_SCRATCH_1            0x20c20024 /* [RW] Raaga Scratch register */
#define BCHP_RAAGA_DSP_MISC_SCRATCH_2            0x20c20028 /* [RW] Raaga Scratch register */
#define BCHP_RAAGA_DSP_MISC_SCRATCH_3            0x20c2002c /* [RW] Raaga Scratch register */
#define BCHP_RAAGA_DSP_MISC_MBIST_TM             0x20c20030 /* [RW] Audio DSP System MBIST Control */
#define BCHP_RAAGA_DSP_MISC_MISC_DBG_STATUS      0x20c2040c /* [RO] Misc module status reg */
#define BCHP_RAAGA_DSP_MISC_SOFTBIST_STATUS      0x20c2044c /* [RO] Softbist Status reg */

/***************************************************************************
 *REVISION - Audio DSP System Revision Register
 ***************************************************************************/
/* RAAGA_DSP_MISC :: REVISION :: reserved0 [31:16] */
#define BCHP_RAAGA_DSP_MISC_REVISION_reserved0_MASK                0xffff0000
#define BCHP_RAAGA_DSP_MISC_REVISION_reserved0_SHIFT               16

/* RAAGA_DSP_MISC :: REVISION :: MAJOR [15:08] */
#define BCHP_RAAGA_DSP_MISC_REVISION_MAJOR_MASK                    0x0000ff00
#define BCHP_RAAGA_DSP_MISC_REVISION_MAJOR_SHIFT                   8
#define BCHP_RAAGA_DSP_MISC_REVISION_MAJOR_DEFAULT                 0x00000021

/* RAAGA_DSP_MISC :: REVISION :: MINOR [07:00] */
#define BCHP_RAAGA_DSP_MISC_REVISION_MINOR_MASK                    0x000000ff
#define BCHP_RAAGA_DSP_MISC_REVISION_MINOR_SHIFT                   0
#define BCHP_RAAGA_DSP_MISC_REVISION_MINOR_DEFAULT                 0x00000000

/***************************************************************************
 *SOFT_INIT - Audio DSP System Soft Reset Control
 ***************************************************************************/
/* RAAGA_DSP_MISC :: SOFT_INIT :: reserved0 [31:17] */
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_reserved0_MASK               0xfffe0000
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_reserved0_SHIFT              17

/* RAAGA_DSP_MISC :: SOFT_INIT :: CLIENT_INIT_DONE [16:16] */
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_CLIENT_INIT_DONE_MASK        0x00010000
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_CLIENT_INIT_DONE_SHIFT       16
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_CLIENT_INIT_DONE_DEFAULT     0x00000000

/* RAAGA_DSP_MISC :: SOFT_INIT :: reserved1 [15:10] */
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_reserved1_MASK               0x0000fc00
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_reserved1_SHIFT              10

/* RAAGA_DSP_MISC :: SOFT_INIT :: DO_CLIENT_INIT [09:09] */
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_DO_CLIENT_INIT_MASK          0x00000200
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_DO_CLIENT_INIT_SHIFT         9
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_DO_CLIENT_INIT_DEFAULT       0x00000000

/* RAAGA_DSP_MISC :: SOFT_INIT :: DO_SW_INIT [08:08] */
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_DO_SW_INIT_MASK              0x00000100
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_DO_SW_INIT_SHIFT             8
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_DO_SW_INIT_DEFAULT           0x00000000

/* RAAGA_DSP_MISC :: SOFT_INIT :: reserved2 [07:02] */
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_reserved2_MASK               0x000000fc
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_reserved2_SHIFT              2

/* RAAGA_DSP_MISC :: SOFT_INIT :: INIT_PROC_B [01:01] */
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_INIT_PROC_B_MASK             0x00000002
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_INIT_PROC_B_SHIFT            1
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_INIT_PROC_B_DEFAULT          0x00000000

/* RAAGA_DSP_MISC :: SOFT_INIT :: INIT_RAAGA_AX [00:00] */
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_INIT_RAAGA_AX_MASK           0x00000001
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_INIT_RAAGA_AX_SHIFT          0
#define BCHP_RAAGA_DSP_MISC_SOFT_INIT_INIT_RAAGA_AX_DEFAULT        0x00000000

/***************************************************************************
 *CORE_ID - DSP Core ID
 ***************************************************************************/
/* RAAGA_DSP_MISC :: CORE_ID :: reserved0 [31:08] */
#define BCHP_RAAGA_DSP_MISC_CORE_ID_reserved0_MASK                 0xffffff00
#define BCHP_RAAGA_DSP_MISC_CORE_ID_reserved0_SHIFT                8

/* RAAGA_DSP_MISC :: CORE_ID :: ID [07:00] */
#define BCHP_RAAGA_DSP_MISC_CORE_ID_ID_MASK                        0x000000ff
#define BCHP_RAAGA_DSP_MISC_CORE_ID_ID_SHIFT                       0

/***************************************************************************
 *PROC_ID - DSP Processor ID
 ***************************************************************************/
/* RAAGA_DSP_MISC :: PROC_ID :: reserved0 [31:08] */
#define BCHP_RAAGA_DSP_MISC_PROC_ID_reserved0_MASK                 0xffffff00
#define BCHP_RAAGA_DSP_MISC_PROC_ID_reserved0_SHIFT                8

/* RAAGA_DSP_MISC :: PROC_ID :: ID [07:00] */
#define BCHP_RAAGA_DSP_MISC_PROC_ID_ID_MASK                        0x000000ff
#define BCHP_RAAGA_DSP_MISC_PROC_ID_ID_SHIFT                       0

/***************************************************************************
 *FP_INTG_TEST_0 - FP Integration test register
 ***************************************************************************/
/* RAAGA_DSP_MISC :: FP_INTG_TEST_0 :: VALUE [31:00] */
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_0_VALUE_MASK              0xffffffff
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_0_VALUE_SHIFT             0
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_0_VALUE_DEFAULT           0x00000000

/***************************************************************************
 *FP_INTG_TEST_1 - FP Integration test register
 ***************************************************************************/
/* RAAGA_DSP_MISC :: FP_INTG_TEST_1 :: VALUE [31:00] */
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_1_VALUE_MASK              0xffffffff
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_1_VALUE_SHIFT             0
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_1_VALUE_DEFAULT           0x00000000

/***************************************************************************
 *FP_INTG_TEST_2 - FP Integration test register
 ***************************************************************************/
/* RAAGA_DSP_MISC :: FP_INTG_TEST_2 :: VALUE [31:00] */
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_2_VALUE_MASK              0xffffffff
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_2_VALUE_SHIFT             0
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_2_VALUE_DEFAULT           0x00000000

/***************************************************************************
 *FP_INTG_TEST_3 - FP Integration test register
 ***************************************************************************/
/* RAAGA_DSP_MISC :: FP_INTG_TEST_3 :: VALUE [31:00] */
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_3_VALUE_MASK              0xffffffff
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_3_VALUE_SHIFT             0
#define BCHP_RAAGA_DSP_MISC_FP_INTG_TEST_3_VALUE_DEFAULT           0x00000000

/***************************************************************************
 *SCRATCH_0 - Raaga Scratch register
 ***************************************************************************/
/* RAAGA_DSP_MISC :: SCRATCH_0 :: VALUE [31:00] */
#define BCHP_RAAGA_DSP_MISC_SCRATCH_0_VALUE_MASK                   0xffffffff
#define BCHP_RAAGA_DSP_MISC_SCRATCH_0_VALUE_SHIFT                  0
#define BCHP_RAAGA_DSP_MISC_SCRATCH_0_VALUE_DEFAULT                0x00000000

/***************************************************************************
 *SCRATCH_1 - Raaga Scratch register
 ***************************************************************************/
/* RAAGA_DSP_MISC :: SCRATCH_1 :: VALUE [31:00] */
#define BCHP_RAAGA_DSP_MISC_SCRATCH_1_VALUE_MASK                   0xffffffff
#define BCHP_RAAGA_DSP_MISC_SCRATCH_1_VALUE_SHIFT                  0
#define BCHP_RAAGA_DSP_MISC_SCRATCH_1_VALUE_DEFAULT                0x00000000

/***************************************************************************
 *SCRATCH_2 - Raaga Scratch register
 ***************************************************************************/
/* RAAGA_DSP_MISC :: SCRATCH_2 :: VALUE [31:00] */
#define BCHP_RAAGA_DSP_MISC_SCRATCH_2_VALUE_MASK                   0xffffffff
#define BCHP_RAAGA_DSP_MISC_SCRATCH_2_VALUE_SHIFT                  0
#define BCHP_RAAGA_DSP_MISC_SCRATCH_2_VALUE_DEFAULT                0x00000000

/***************************************************************************
 *SCRATCH_3 - Raaga Scratch register
 ***************************************************************************/
/* RAAGA_DSP_MISC :: SCRATCH_3 :: VALUE [31:00] */
#define BCHP_RAAGA_DSP_MISC_SCRATCH_3_VALUE_MASK                   0xffffffff
#define BCHP_RAAGA_DSP_MISC_SCRATCH_3_VALUE_SHIFT                  0
#define BCHP_RAAGA_DSP_MISC_SCRATCH_3_VALUE_DEFAULT                0x00000000

/***************************************************************************
 *MBIST_TM - Audio DSP System MBIST Control
 ***************************************************************************/
/* RAAGA_DSP_MISC :: MBIST_TM :: reserved0 [31:02] */
#define BCHP_RAAGA_DSP_MISC_MBIST_TM_reserved0_MASK                0xfffffffc
#define BCHP_RAAGA_DSP_MISC_MBIST_TM_reserved0_SHIFT               2

/* RAAGA_DSP_MISC :: MBIST_TM :: FP_TST_TM [01:00] */
#define BCHP_RAAGA_DSP_MISC_MBIST_TM_FP_TST_TM_MASK                0x00000003
#define BCHP_RAAGA_DSP_MISC_MBIST_TM_FP_TST_TM_SHIFT               0
#define BCHP_RAAGA_DSP_MISC_MBIST_TM_FP_TST_TM_DEFAULT             0x00000000

/***************************************************************************
 *MISC_DBG_STATUS - Misc module status reg
 ***************************************************************************/
/* RAAGA_DSP_MISC :: MISC_DBG_STATUS :: reserved0 [31:04] */
#define BCHP_RAAGA_DSP_MISC_MISC_DBG_STATUS_reserved0_MASK         0xfffffff0
#define BCHP_RAAGA_DSP_MISC_MISC_DBG_STATUS_reserved0_SHIFT        4

/* RAAGA_DSP_MISC :: MISC_DBG_STATUS :: PMEM_STATE [03:00] */
#define BCHP_RAAGA_DSP_MISC_MISC_DBG_STATUS_PMEM_STATE_MASK        0x0000000f
#define BCHP_RAAGA_DSP_MISC_MISC_DBG_STATUS_PMEM_STATE_SHIFT       0

/***************************************************************************
 *SOFTBIST_STATUS - Softbist Status reg
 ***************************************************************************/
/* RAAGA_DSP_MISC :: SOFTBIST_STATUS :: reserved0 [31:02] */
#define BCHP_RAAGA_DSP_MISC_SOFTBIST_STATUS_reserved0_MASK         0xfffffffc
#define BCHP_RAAGA_DSP_MISC_SOFTBIST_STATUS_reserved0_SHIFT        2

/* RAAGA_DSP_MISC :: SOFTBIST_STATUS :: SOFTBIST_PASS [01:01] */
#define BCHP_RAAGA_DSP_MISC_SOFTBIST_STATUS_SOFTBIST_PASS_MASK     0x00000002
#define BCHP_RAAGA_DSP_MISC_SOFTBIST_STATUS_SOFTBIST_PASS_SHIFT    1

/* RAAGA_DSP_MISC :: SOFTBIST_STATUS :: SOFTBIST_DONE [00:00] */
#define BCHP_RAAGA_DSP_MISC_SOFTBIST_STATUS_SOFTBIST_DONE_MASK     0x00000001
#define BCHP_RAAGA_DSP_MISC_SOFTBIST_STATUS_SOFTBIST_DONE_SHIFT    0

#endif /* #ifndef BCHP_RAAGA_DSP_MISC_H__ */

/* End of File */
