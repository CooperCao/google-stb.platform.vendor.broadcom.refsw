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

#ifndef BCHP_SID_ARC_CORE_H__
#define BCHP_SID_ARC_CORE_H__

/***************************************************************************
 *SID_ARC_CORE - SID_ARC_CORE registers
 ***************************************************************************/
#define BCHP_SID_ARC_CORE_CPU_PC                 0x20995000 /* [RW][32] CPU PC */
#define BCHP_SID_ARC_CORE_CPU_SEM                0x20995004 /* [RW][32] CPU SEMAPHORE */
#define BCHP_SID_ARC_CORE_CPU_LSTART             0x20995008 /* [RW][32] CPU LSTART */
#define BCHP_SID_ARC_CORE_CPU_LEND               0x2099500c /* [RW][32] CPU LEND */
#define BCHP_SID_ARC_CORE_CPU_ID                 0x20995010 /* [RW][32] CPU ID */
#define BCHP_SID_ARC_CORE_CPU_DEBUG              0x20995014 /* [RW][32] CPU DEBUG */

/***************************************************************************
 *CPU_PC - CPU PC
 ***************************************************************************/
/* SID_ARC_CORE :: CPU_PC :: VALUE [31:00] */
#define BCHP_SID_ARC_CORE_CPU_PC_VALUE_MASK                        0xffffffff
#define BCHP_SID_ARC_CORE_CPU_PC_VALUE_SHIFT                       0
#define BCHP_SID_ARC_CORE_CPU_PC_VALUE_DEFAULT                     0x00000000

/***************************************************************************
 *CPU_SEM - CPU SEMAPHORE
 ***************************************************************************/
/* SID_ARC_CORE :: CPU_SEM :: reserved0 [31:04] */
#define BCHP_SID_ARC_CORE_CPU_SEM_reserved0_MASK                   0xfffffff0
#define BCHP_SID_ARC_CORE_CPU_SEM_reserved0_SHIFT                  4

/* SID_ARC_CORE :: CPU_SEM :: SEMA3 [03:03] */
#define BCHP_SID_ARC_CORE_CPU_SEM_SEMA3_MASK                       0x00000008
#define BCHP_SID_ARC_CORE_CPU_SEM_SEMA3_SHIFT                      3
#define BCHP_SID_ARC_CORE_CPU_SEM_SEMA3_DEFAULT                    0x00000000

/* SID_ARC_CORE :: CPU_SEM :: SEMA2 [02:02] */
#define BCHP_SID_ARC_CORE_CPU_SEM_SEMA2_MASK                       0x00000004
#define BCHP_SID_ARC_CORE_CPU_SEM_SEMA2_SHIFT                      2
#define BCHP_SID_ARC_CORE_CPU_SEM_SEMA2_DEFAULT                    0x00000000

/* SID_ARC_CORE :: CPU_SEM :: SEMA1 [01:01] */
#define BCHP_SID_ARC_CORE_CPU_SEM_SEMA1_MASK                       0x00000002
#define BCHP_SID_ARC_CORE_CPU_SEM_SEMA1_SHIFT                      1
#define BCHP_SID_ARC_CORE_CPU_SEM_SEMA1_DEFAULT                    0x00000000

/* SID_ARC_CORE :: CPU_SEM :: SEMA0 [00:00] */
#define BCHP_SID_ARC_CORE_CPU_SEM_SEMA0_MASK                       0x00000001
#define BCHP_SID_ARC_CORE_CPU_SEM_SEMA0_SHIFT                      0
#define BCHP_SID_ARC_CORE_CPU_SEM_SEMA0_DEFAULT                    0x00000000

/***************************************************************************
 *CPU_LSTART - CPU LSTART
 ***************************************************************************/
/* SID_ARC_CORE :: CPU_LSTART :: reserved0 [31:00] */
#define BCHP_SID_ARC_CORE_CPU_LSTART_reserved0_MASK                0xffffffff
#define BCHP_SID_ARC_CORE_CPU_LSTART_reserved0_SHIFT               0

/***************************************************************************
 *CPU_LEND - CPU LEND
 ***************************************************************************/
/* SID_ARC_CORE :: CPU_LEND :: reserved0 [31:00] */
#define BCHP_SID_ARC_CORE_CPU_LEND_reserved0_MASK                  0xffffffff
#define BCHP_SID_ARC_CORE_CPU_LEND_reserved0_SHIFT                 0

/***************************************************************************
 *CPU_ID - CPU ID
 ***************************************************************************/
/* SID_ARC_CORE :: CPU_ID :: mancode [31:24] */
#define BCHP_SID_ARC_CORE_CPU_ID_mancode_MASK                      0xff000000
#define BCHP_SID_ARC_CORE_CPU_ID_mancode_SHIFT                     24
#define BCHP_SID_ARC_CORE_CPU_ID_mancode_DEFAULT                   0x00000001

/* SID_ARC_CORE :: CPU_ID :: manver [23:16] */
#define BCHP_SID_ARC_CORE_CPU_ID_manver_MASK                       0x00ff0000
#define BCHP_SID_ARC_CORE_CPU_ID_manver_SHIFT                      16
#define BCHP_SID_ARC_CORE_CPU_ID_manver_DEFAULT                    0x00000001

/* SID_ARC_CORE :: CPU_ID :: id_arcnum [15:08] */
#define BCHP_SID_ARC_CORE_CPU_ID_id_arcnum_MASK                    0x0000ff00
#define BCHP_SID_ARC_CORE_CPU_ID_id_arcnum_SHIFT                   8
#define BCHP_SID_ARC_CORE_CPU_ID_id_arcnum_DEFAULT                 0x00000000

/* SID_ARC_CORE :: CPU_ID :: arcver [07:00] */
#define BCHP_SID_ARC_CORE_CPU_ID_arcver_MASK                       0x000000ff
#define BCHP_SID_ARC_CORE_CPU_ID_arcver_SHIFT                      0
#define BCHP_SID_ARC_CORE_CPU_ID_arcver_DEFAULT                    0x00000008

/***************************************************************************
 *CPU_DEBUG - CPU DEBUG
 ***************************************************************************/
/* SID_ARC_CORE :: CPU_DEBUG :: reserved0 [31:12] */
#define BCHP_SID_ARC_CORE_CPU_DEBUG_reserved0_MASK                 0xfffff000
#define BCHP_SID_ARC_CORE_CPU_DEBUG_reserved0_SHIFT                12

/* SID_ARC_CORE :: CPU_DEBUG :: db_inst_step [11:11] */
#define BCHP_SID_ARC_CORE_CPU_DEBUG_db_inst_step_MASK              0x00000800
#define BCHP_SID_ARC_CORE_CPU_DEBUG_db_inst_step_SHIFT             11
#define BCHP_SID_ARC_CORE_CPU_DEBUG_db_inst_step_DEFAULT           0x00000000

/* SID_ARC_CORE :: CPU_DEBUG :: reserved1 [10:01] */
#define BCHP_SID_ARC_CORE_CPU_DEBUG_reserved1_MASK                 0x000007fe
#define BCHP_SID_ARC_CORE_CPU_DEBUG_reserved1_SHIFT                1

/* SID_ARC_CORE :: CPU_DEBUG :: db_step [00:00] */
#define BCHP_SID_ARC_CORE_CPU_DEBUG_db_step_MASK                   0x00000001
#define BCHP_SID_ARC_CORE_CPU_DEBUG_db_step_SHIFT                  0
#define BCHP_SID_ARC_CORE_CPU_DEBUG_db_step_DEFAULT                0x00000000

#endif /* #ifndef BCHP_SID_ARC_CORE_H__ */

/* End of File */
