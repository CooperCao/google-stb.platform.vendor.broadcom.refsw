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

#ifndef BCHP_RAAGA_DSP_FW_INTH_H__
#define BCHP_RAAGA_DSP_FW_INTH_H__

/***************************************************************************
 *RAAGA_DSP_FW_INTH - Raaga DSP FW Interrupts to External Hosts Registers
 ***************************************************************************/
#define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS       0x20c22800 /* [RO][32] Host Interrupt Status Register */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_SET          0x20c22804 /* [WO][32] Host Interrupt Set Register */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR        0x20c22808 /* [WO][32] Host Interrupt Clear Register */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_STATUS  0x20c2280c /* [RO][32] Host Interrupt Mask Status Register */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_SET     0x20c22810 /* [WO][32] Host Interrupt Mask Set Register */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_CLEAR   0x20c22814 /* [WO][32] Host Interrupt Mask Clear Register */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_STATUS        0x20c22818 /* [RO][32] PCI Interrupt Status Register */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_SET           0x20c2281c /* [WO][32] PCI Interrupt Set Register */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_CLEAR         0x20c22820 /* [WO][32] PCI Interrupt Clear Register */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_STATUS   0x20c22824 /* [RO][32] PCI Interrupt Mask Status Register */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_SET      0x20c22828 /* [WO][32] PCI Interrupt Mask Set Register */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_CLEAR    0x20c2282c /* [WO][32] PCI Interrupt Mask Clear Register */

/***************************************************************************
 *HOST_STATUS - Host Interrupt Status Register
 ***************************************************************************/
/* RAAGA_DSP_FW_INTH :: HOST_STATUS :: reserved0 [31:03] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_reserved0_MASK          0xfffffff8
#define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_reserved0_SHIFT         3

/* RAAGA_DSP_FW_INTH :: HOST_STATUS :: HOST_MSG [02:02] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_HOST_MSG_MASK           0x00000004
#define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_HOST_MSG_SHIFT          2
#define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_HOST_MSG_DEFAULT        0x00000000

/* RAAGA_DSP_FW_INTH :: HOST_STATUS :: ASYNC_MSG [01:01] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_ASYNC_MSG_MASK          0x00000002
#define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_ASYNC_MSG_SHIFT         1
#define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_ASYNC_MSG_DEFAULT       0x00000000

/* RAAGA_DSP_FW_INTH :: HOST_STATUS :: SYNC_MSG [00:00] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_SYNC_MSG_MASK           0x00000001
#define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_SYNC_MSG_SHIFT          0
#define BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS_SYNC_MSG_DEFAULT        0x00000000

/***************************************************************************
 *HOST_SET - Host Interrupt Set Register
 ***************************************************************************/
/* RAAGA_DSP_FW_INTH :: HOST_SET :: reserved0 [31:03] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_SET_reserved0_MASK             0xfffffff8
#define BCHP_RAAGA_DSP_FW_INTH_HOST_SET_reserved0_SHIFT            3

/* RAAGA_DSP_FW_INTH :: HOST_SET :: HOST_MSG [02:02] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_SET_HOST_MSG_MASK              0x00000004
#define BCHP_RAAGA_DSP_FW_INTH_HOST_SET_HOST_MSG_SHIFT             2
#define BCHP_RAAGA_DSP_FW_INTH_HOST_SET_HOST_MSG_DEFAULT           0x00000000

/* RAAGA_DSP_FW_INTH :: HOST_SET :: ASYNC_MSG [01:01] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_SET_ASYNC_MSG_MASK             0x00000002
#define BCHP_RAAGA_DSP_FW_INTH_HOST_SET_ASYNC_MSG_SHIFT            1
#define BCHP_RAAGA_DSP_FW_INTH_HOST_SET_ASYNC_MSG_DEFAULT          0x00000000

/* RAAGA_DSP_FW_INTH :: HOST_SET :: SYNC_MSG [00:00] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_SET_SYNC_MSG_MASK              0x00000001
#define BCHP_RAAGA_DSP_FW_INTH_HOST_SET_SYNC_MSG_SHIFT             0
#define BCHP_RAAGA_DSP_FW_INTH_HOST_SET_SYNC_MSG_DEFAULT           0x00000000

/***************************************************************************
 *HOST_CLEAR - Host Interrupt Clear Register
 ***************************************************************************/
/* RAAGA_DSP_FW_INTH :: HOST_CLEAR :: reserved0 [31:03] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR_reserved0_MASK           0xfffffff8
#define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR_reserved0_SHIFT          3

/* RAAGA_DSP_FW_INTH :: HOST_CLEAR :: HOST_MSG [02:02] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR_HOST_MSG_MASK            0x00000004
#define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR_HOST_MSG_SHIFT           2
#define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR_HOST_MSG_DEFAULT         0x00000000

/* RAAGA_DSP_FW_INTH :: HOST_CLEAR :: ASYNC_MSG [01:01] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR_ASYNC_MSG_MASK           0x00000002
#define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR_ASYNC_MSG_SHIFT          1
#define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR_ASYNC_MSG_DEFAULT        0x00000000

/* RAAGA_DSP_FW_INTH :: HOST_CLEAR :: SYNC_MSG [00:00] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR_SYNC_MSG_MASK            0x00000001
#define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR_SYNC_MSG_SHIFT           0
#define BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR_SYNC_MSG_DEFAULT         0x00000000

/***************************************************************************
 *HOST_MASK_STATUS - Host Interrupt Mask Status Register
 ***************************************************************************/
/* RAAGA_DSP_FW_INTH :: HOST_MASK_STATUS :: reserved0 [31:03] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_STATUS_reserved0_MASK     0xfffffff8
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_STATUS_reserved0_SHIFT    3

/* RAAGA_DSP_FW_INTH :: HOST_MASK_STATUS :: HOST_MSG [02:02] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_STATUS_HOST_MSG_MASK      0x00000004
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_STATUS_HOST_MSG_SHIFT     2
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_STATUS_HOST_MSG_DEFAULT   0x00000001

/* RAAGA_DSP_FW_INTH :: HOST_MASK_STATUS :: ASYNC_MSG [01:01] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_STATUS_ASYNC_MSG_MASK     0x00000002
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_STATUS_ASYNC_MSG_SHIFT    1
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_STATUS_ASYNC_MSG_DEFAULT  0x00000001

/* RAAGA_DSP_FW_INTH :: HOST_MASK_STATUS :: SYNC_MSG [00:00] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_STATUS_SYNC_MSG_MASK      0x00000001
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_STATUS_SYNC_MSG_SHIFT     0
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_STATUS_SYNC_MSG_DEFAULT   0x00000001

/***************************************************************************
 *HOST_MASK_SET - Host Interrupt Mask Set Register
 ***************************************************************************/
/* RAAGA_DSP_FW_INTH :: HOST_MASK_SET :: reserved0 [31:03] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_SET_reserved0_MASK        0xfffffff8
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_SET_reserved0_SHIFT       3

/* RAAGA_DSP_FW_INTH :: HOST_MASK_SET :: HOST_MSG [02:02] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_SET_HOST_MSG_MASK         0x00000004
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_SET_HOST_MSG_SHIFT        2
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_SET_HOST_MSG_DEFAULT      0x00000001

/* RAAGA_DSP_FW_INTH :: HOST_MASK_SET :: ASYNC_MSG [01:01] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_SET_ASYNC_MSG_MASK        0x00000002
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_SET_ASYNC_MSG_SHIFT       1
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_SET_ASYNC_MSG_DEFAULT     0x00000001

/* RAAGA_DSP_FW_INTH :: HOST_MASK_SET :: SYNC_MSG [00:00] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_SET_SYNC_MSG_MASK         0x00000001
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_SET_SYNC_MSG_SHIFT        0
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_SET_SYNC_MSG_DEFAULT      0x00000001

/***************************************************************************
 *HOST_MASK_CLEAR - Host Interrupt Mask Clear Register
 ***************************************************************************/
/* RAAGA_DSP_FW_INTH :: HOST_MASK_CLEAR :: reserved0 [31:03] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_CLEAR_reserved0_MASK      0xfffffff8
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_CLEAR_reserved0_SHIFT     3

/* RAAGA_DSP_FW_INTH :: HOST_MASK_CLEAR :: HOST_MSG [02:02] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_CLEAR_HOST_MSG_MASK       0x00000004
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_CLEAR_HOST_MSG_SHIFT      2
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_CLEAR_HOST_MSG_DEFAULT    0x00000001

/* RAAGA_DSP_FW_INTH :: HOST_MASK_CLEAR :: ASYNC_MSG [01:01] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_CLEAR_ASYNC_MSG_MASK      0x00000002
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_CLEAR_ASYNC_MSG_SHIFT     1
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_CLEAR_ASYNC_MSG_DEFAULT   0x00000001

/* RAAGA_DSP_FW_INTH :: HOST_MASK_CLEAR :: SYNC_MSG [00:00] */
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_CLEAR_SYNC_MSG_MASK       0x00000001
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_CLEAR_SYNC_MSG_SHIFT      0
#define BCHP_RAAGA_DSP_FW_INTH_HOST_MASK_CLEAR_SYNC_MSG_DEFAULT    0x00000001

/***************************************************************************
 *PCI_STATUS - PCI Interrupt Status Register
 ***************************************************************************/
/* RAAGA_DSP_FW_INTH :: PCI_STATUS :: reserved0 [31:03] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_STATUS_reserved0_MASK           0xfffffff8
#define BCHP_RAAGA_DSP_FW_INTH_PCI_STATUS_reserved0_SHIFT          3

/* RAAGA_DSP_FW_INTH :: PCI_STATUS :: HOST_MSG [02:02] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_STATUS_HOST_MSG_MASK            0x00000004
#define BCHP_RAAGA_DSP_FW_INTH_PCI_STATUS_HOST_MSG_SHIFT           2
#define BCHP_RAAGA_DSP_FW_INTH_PCI_STATUS_HOST_MSG_DEFAULT         0x00000000

/* RAAGA_DSP_FW_INTH :: PCI_STATUS :: ASYNC_MSG [01:01] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_STATUS_ASYNC_MSG_MASK           0x00000002
#define BCHP_RAAGA_DSP_FW_INTH_PCI_STATUS_ASYNC_MSG_SHIFT          1
#define BCHP_RAAGA_DSP_FW_INTH_PCI_STATUS_ASYNC_MSG_DEFAULT        0x00000000

/* RAAGA_DSP_FW_INTH :: PCI_STATUS :: SYNC_MSG [00:00] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_STATUS_SYNC_MSG_MASK            0x00000001
#define BCHP_RAAGA_DSP_FW_INTH_PCI_STATUS_SYNC_MSG_SHIFT           0
#define BCHP_RAAGA_DSP_FW_INTH_PCI_STATUS_SYNC_MSG_DEFAULT         0x00000000

/***************************************************************************
 *PCI_SET - PCI Interrupt Set Register
 ***************************************************************************/
/* RAAGA_DSP_FW_INTH :: PCI_SET :: reserved0 [31:03] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_SET_reserved0_MASK              0xfffffff8
#define BCHP_RAAGA_DSP_FW_INTH_PCI_SET_reserved0_SHIFT             3

/* RAAGA_DSP_FW_INTH :: PCI_SET :: HOST_MSG [02:02] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_SET_HOST_MSG_MASK               0x00000004
#define BCHP_RAAGA_DSP_FW_INTH_PCI_SET_HOST_MSG_SHIFT              2
#define BCHP_RAAGA_DSP_FW_INTH_PCI_SET_HOST_MSG_DEFAULT            0x00000000

/* RAAGA_DSP_FW_INTH :: PCI_SET :: ASYNC_MSG [01:01] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_SET_ASYNC_MSG_MASK              0x00000002
#define BCHP_RAAGA_DSP_FW_INTH_PCI_SET_ASYNC_MSG_SHIFT             1
#define BCHP_RAAGA_DSP_FW_INTH_PCI_SET_ASYNC_MSG_DEFAULT           0x00000000

/* RAAGA_DSP_FW_INTH :: PCI_SET :: SYNC_MSG [00:00] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_SET_SYNC_MSG_MASK               0x00000001
#define BCHP_RAAGA_DSP_FW_INTH_PCI_SET_SYNC_MSG_SHIFT              0
#define BCHP_RAAGA_DSP_FW_INTH_PCI_SET_SYNC_MSG_DEFAULT            0x00000000

/***************************************************************************
 *PCI_CLEAR - PCI Interrupt Clear Register
 ***************************************************************************/
/* RAAGA_DSP_FW_INTH :: PCI_CLEAR :: reserved0 [31:03] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_CLEAR_reserved0_MASK            0xfffffff8
#define BCHP_RAAGA_DSP_FW_INTH_PCI_CLEAR_reserved0_SHIFT           3

/* RAAGA_DSP_FW_INTH :: PCI_CLEAR :: HOST_MSG [02:02] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_CLEAR_HOST_MSG_MASK             0x00000004
#define BCHP_RAAGA_DSP_FW_INTH_PCI_CLEAR_HOST_MSG_SHIFT            2
#define BCHP_RAAGA_DSP_FW_INTH_PCI_CLEAR_HOST_MSG_DEFAULT          0x00000000

/* RAAGA_DSP_FW_INTH :: PCI_CLEAR :: ASYNC_MSG [01:01] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_CLEAR_ASYNC_MSG_MASK            0x00000002
#define BCHP_RAAGA_DSP_FW_INTH_PCI_CLEAR_ASYNC_MSG_SHIFT           1
#define BCHP_RAAGA_DSP_FW_INTH_PCI_CLEAR_ASYNC_MSG_DEFAULT         0x00000000

/* RAAGA_DSP_FW_INTH :: PCI_CLEAR :: SYNC_MSG [00:00] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_CLEAR_SYNC_MSG_MASK             0x00000001
#define BCHP_RAAGA_DSP_FW_INTH_PCI_CLEAR_SYNC_MSG_SHIFT            0
#define BCHP_RAAGA_DSP_FW_INTH_PCI_CLEAR_SYNC_MSG_DEFAULT          0x00000000

/***************************************************************************
 *PCI_MASK_STATUS - PCI Interrupt Mask Status Register
 ***************************************************************************/
/* RAAGA_DSP_FW_INTH :: PCI_MASK_STATUS :: reserved0 [31:03] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_STATUS_reserved0_MASK      0xfffffff8
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_STATUS_reserved0_SHIFT     3

/* RAAGA_DSP_FW_INTH :: PCI_MASK_STATUS :: HOST_MSG [02:02] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_STATUS_HOST_MSG_MASK       0x00000004
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_STATUS_HOST_MSG_SHIFT      2
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_STATUS_HOST_MSG_DEFAULT    0x00000001

/* RAAGA_DSP_FW_INTH :: PCI_MASK_STATUS :: ASYNC_MSG [01:01] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_STATUS_ASYNC_MSG_MASK      0x00000002
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_STATUS_ASYNC_MSG_SHIFT     1
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_STATUS_ASYNC_MSG_DEFAULT   0x00000001

/* RAAGA_DSP_FW_INTH :: PCI_MASK_STATUS :: SYNC_MSG [00:00] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_STATUS_SYNC_MSG_MASK       0x00000001
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_STATUS_SYNC_MSG_SHIFT      0
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_STATUS_SYNC_MSG_DEFAULT    0x00000001

/***************************************************************************
 *PCI_MASK_SET - PCI Interrupt Mask Set Register
 ***************************************************************************/
/* RAAGA_DSP_FW_INTH :: PCI_MASK_SET :: reserved0 [31:03] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_SET_reserved0_MASK         0xfffffff8
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_SET_reserved0_SHIFT        3

/* RAAGA_DSP_FW_INTH :: PCI_MASK_SET :: HOST_MSG [02:02] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_SET_HOST_MSG_MASK          0x00000004
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_SET_HOST_MSG_SHIFT         2
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_SET_HOST_MSG_DEFAULT       0x00000001

/* RAAGA_DSP_FW_INTH :: PCI_MASK_SET :: ASYNC_MSG [01:01] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_SET_ASYNC_MSG_MASK         0x00000002
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_SET_ASYNC_MSG_SHIFT        1
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_SET_ASYNC_MSG_DEFAULT      0x00000001

/* RAAGA_DSP_FW_INTH :: PCI_MASK_SET :: SYNC_MSG [00:00] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_SET_SYNC_MSG_MASK          0x00000001
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_SET_SYNC_MSG_SHIFT         0
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_SET_SYNC_MSG_DEFAULT       0x00000001

/***************************************************************************
 *PCI_MASK_CLEAR - PCI Interrupt Mask Clear Register
 ***************************************************************************/
/* RAAGA_DSP_FW_INTH :: PCI_MASK_CLEAR :: reserved0 [31:03] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_CLEAR_reserved0_MASK       0xfffffff8
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_CLEAR_reserved0_SHIFT      3

/* RAAGA_DSP_FW_INTH :: PCI_MASK_CLEAR :: HOST_MSG [02:02] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_CLEAR_HOST_MSG_MASK        0x00000004
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_CLEAR_HOST_MSG_SHIFT       2
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_CLEAR_HOST_MSG_DEFAULT     0x00000001

/* RAAGA_DSP_FW_INTH :: PCI_MASK_CLEAR :: ASYNC_MSG [01:01] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_CLEAR_ASYNC_MSG_MASK       0x00000002
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_CLEAR_ASYNC_MSG_SHIFT      1
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_CLEAR_ASYNC_MSG_DEFAULT    0x00000001

/* RAAGA_DSP_FW_INTH :: PCI_MASK_CLEAR :: SYNC_MSG [00:00] */
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_CLEAR_SYNC_MSG_MASK        0x00000001
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_CLEAR_SYNC_MSG_SHIFT       0
#define BCHP_RAAGA_DSP_FW_INTH_PCI_MASK_CLEAR_SYNC_MSG_DEFAULT     0x00000001

#endif /* #ifndef BCHP_RAAGA_DSP_FW_INTH_H__ */

/* End of File */
