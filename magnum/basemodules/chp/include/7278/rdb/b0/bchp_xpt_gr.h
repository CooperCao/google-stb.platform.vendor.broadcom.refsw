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

#ifndef BCHP_XPT_GR_H__
#define BCHP_XPT_GR_H__

/***************************************************************************
 *XPT_GR - XPT GR_BRIDGE Control Registers
 ***************************************************************************/
#define BCHP_XPT_GR_REVISION                     0x02200000 /* [RO][32] GR Bridge Revision */
#define BCHP_XPT_GR_CTRL                         0x02200004 /* [CFG][32] GR Bridge Control Register */
#define BCHP_XPT_GR_PD                           0x02200008 /* [CFG][32] GR Bridge Power Down Register */
#define BCHP_XPT_GR_SW_INIT_0                    0x0220000c /* [CFG][32] GR Bridge Software Init 0 Register */

/***************************************************************************
 *REVISION - GR Bridge Revision
 ***************************************************************************/
/* XPT_GR :: REVISION :: reserved0 [31:16] */
#define BCHP_XPT_GR_REVISION_reserved0_MASK                        0xffff0000
#define BCHP_XPT_GR_REVISION_reserved0_SHIFT                       16

/* XPT_GR :: REVISION :: MAJOR [15:08] */
#define BCHP_XPT_GR_REVISION_MAJOR_MASK                            0x0000ff00
#define BCHP_XPT_GR_REVISION_MAJOR_SHIFT                           8
#define BCHP_XPT_GR_REVISION_MAJOR_DEFAULT                         0x00000007

/* XPT_GR :: REVISION :: MINOR [07:00] */
#define BCHP_XPT_GR_REVISION_MINOR_MASK                            0x000000ff
#define BCHP_XPT_GR_REVISION_MINOR_SHIFT                           0
#define BCHP_XPT_GR_REVISION_MINOR_DEFAULT                         0x00000000

/***************************************************************************
 *CTRL - GR Bridge Control Register
 ***************************************************************************/
/* XPT_GR :: CTRL :: reserved0 [31:01] */
#define BCHP_XPT_GR_CTRL_reserved0_MASK                            0xfffffffe
#define BCHP_XPT_GR_CTRL_reserved0_SHIFT                           1

/* XPT_GR :: CTRL :: gisb_error_intr [00:00] */
#define BCHP_XPT_GR_CTRL_gisb_error_intr_MASK                      0x00000001
#define BCHP_XPT_GR_CTRL_gisb_error_intr_SHIFT                     0
#define BCHP_XPT_GR_CTRL_gisb_error_intr_INTR_DISABLE              0
#define BCHP_XPT_GR_CTRL_gisb_error_intr_INTR_ENABLE               1

/***************************************************************************
 *PD - GR Bridge Power Down Register
 ***************************************************************************/
/* XPT_GR :: PD :: reserved0 [31:01] */
#define BCHP_XPT_GR_PD_reserved0_MASK                              0xfffffffe
#define BCHP_XPT_GR_PD_reserved0_SHIFT                             1

/* XPT_GR :: PD :: power_down [00:00] */
#define BCHP_XPT_GR_PD_power_down_MASK                             0x00000001
#define BCHP_XPT_GR_PD_power_down_SHIFT                            0
#define BCHP_XPT_GR_PD_power_down_DEFAULT                          0x00000000

/***************************************************************************
 *SW_INIT_0 - GR Bridge Software Init 0 Register
 ***************************************************************************/
/* XPT_GR :: SW_INIT_0 :: reserved0 [31:01] */
#define BCHP_XPT_GR_SW_INIT_0_reserved0_MASK                       0xfffffffe
#define BCHP_XPT_GR_SW_INIT_0_reserved0_SHIFT                      1

/* XPT_GR :: SW_INIT_0 :: SPARE_SW_INIT [00:00] */
#define BCHP_XPT_GR_SW_INIT_0_SPARE_SW_INIT_MASK                   0x00000001
#define BCHP_XPT_GR_SW_INIT_0_SPARE_SW_INIT_SHIFT                  0
#define BCHP_XPT_GR_SW_INIT_0_SPARE_SW_INIT_DEFAULT                0x00000000
#define BCHP_XPT_GR_SW_INIT_0_SPARE_SW_INIT_DEASSERT               0
#define BCHP_XPT_GR_SW_INIT_0_SPARE_SW_INIT_ASSERT                 1

#endif /* #ifndef BCHP_XPT_GR_H__ */

/* End of File */
