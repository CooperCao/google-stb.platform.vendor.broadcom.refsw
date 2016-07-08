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
 * Date:           Generated on               Thu Mar 10 13:57:25 2016
 *                 Full Compile MD5 Checksum  628af85e282e26d7aa8cb3039beb3dda
 *                     (minus title and desc)
 *                 MD5 Checksum               4a24f3aa9cf80f1b639f46df03606df9
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     871
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_UFE_GR_BRIDGE_0_H__
#define BCHP_UFE_GR_BRIDGE_0_H__

/***************************************************************************
 *UFE_GR_BRIDGE_0 - UFE GR Bridge Register Set
 ***************************************************************************/
#define BCHP_UFE_GR_BRIDGE_0_REVISION            0x04000700 /* [RO] GR Bridge Revision */
#define BCHP_UFE_GR_BRIDGE_0_CTRL                0x04000704 /* [RW] GR Bridge Control Register */
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_0           0x04000708 /* [RW] GR Bridge Software Init 0 Register */
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_1           0x0400070c /* [RW] GR Bridge Software Init 1 Register */

/***************************************************************************
 *REVISION - GR Bridge Revision
 ***************************************************************************/
/* UFE_GR_BRIDGE_0 :: REVISION :: reserved0 [31:16] */
#define BCHP_UFE_GR_BRIDGE_0_REVISION_reserved0_MASK               0xffff0000
#define BCHP_UFE_GR_BRIDGE_0_REVISION_reserved0_SHIFT              16

/* UFE_GR_BRIDGE_0 :: REVISION :: MAJOR [15:08] */
#define BCHP_UFE_GR_BRIDGE_0_REVISION_MAJOR_MASK                   0x0000ff00
#define BCHP_UFE_GR_BRIDGE_0_REVISION_MAJOR_SHIFT                  8
#define BCHP_UFE_GR_BRIDGE_0_REVISION_MAJOR_DEFAULT                0x00000002

/* UFE_GR_BRIDGE_0 :: REVISION :: MINOR [07:00] */
#define BCHP_UFE_GR_BRIDGE_0_REVISION_MINOR_MASK                   0x000000ff
#define BCHP_UFE_GR_BRIDGE_0_REVISION_MINOR_SHIFT                  0
#define BCHP_UFE_GR_BRIDGE_0_REVISION_MINOR_DEFAULT                0x00000000

/***************************************************************************
 *CTRL - GR Bridge Control Register
 ***************************************************************************/
/* UFE_GR_BRIDGE_0 :: CTRL :: reserved0 [31:01] */
#define BCHP_UFE_GR_BRIDGE_0_CTRL_reserved0_MASK                   0xfffffffe
#define BCHP_UFE_GR_BRIDGE_0_CTRL_reserved0_SHIFT                  1

/* UFE_GR_BRIDGE_0 :: CTRL :: gisb_error_intr [00:00] */
#define BCHP_UFE_GR_BRIDGE_0_CTRL_gisb_error_intr_MASK             0x00000001
#define BCHP_UFE_GR_BRIDGE_0_CTRL_gisb_error_intr_SHIFT            0
#define BCHP_UFE_GR_BRIDGE_0_CTRL_gisb_error_intr_DEFAULT          0x00000000
#define BCHP_UFE_GR_BRIDGE_0_CTRL_gisb_error_intr_INTR_DISABLE     0
#define BCHP_UFE_GR_BRIDGE_0_CTRL_gisb_error_intr_INTR_ENABLE      1

/***************************************************************************
 *SW_INIT_0 - GR Bridge Software Init 0 Register
 ***************************************************************************/
/* UFE_GR_BRIDGE_0 :: SW_INIT_0 :: reserved0 [31:01] */
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_0_reserved0_MASK              0xfffffffe
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_0_reserved0_SHIFT             1

/* UFE_GR_BRIDGE_0 :: SW_INIT_0 :: UFE_SW_INIT [00:00] */
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_0_UFE_SW_INIT_MASK            0x00000001
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_0_UFE_SW_INIT_SHIFT           0
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_0_UFE_SW_INIT_DEFAULT         0x00000000
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_0_UFE_SW_INIT_DEASSERT        0
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_0_UFE_SW_INIT_ASSERT          1

/***************************************************************************
 *SW_INIT_1 - GR Bridge Software Init 1 Register
 ***************************************************************************/
/* UFE_GR_BRIDGE_0 :: SW_INIT_1 :: reserved0 [31:01] */
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_1_reserved0_MASK              0xfffffffe
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_1_reserved0_SHIFT             1

/* UFE_GR_BRIDGE_0 :: SW_INIT_1 :: SPARE_SW_INIT [00:00] */
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_1_SPARE_SW_INIT_MASK          0x00000001
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_1_SPARE_SW_INIT_SHIFT         0
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_1_SPARE_SW_INIT_DEFAULT       0x00000001
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_1_SPARE_SW_INIT_DEASSERT      0
#define BCHP_UFE_GR_BRIDGE_0_SW_INIT_1_SPARE_SW_INIT_ASSERT        1

#endif /* #ifndef BCHP_UFE_GR_BRIDGE_0_H__ */

/* End of File */
