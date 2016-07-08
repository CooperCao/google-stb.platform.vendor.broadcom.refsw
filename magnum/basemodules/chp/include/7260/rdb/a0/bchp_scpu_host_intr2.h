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
 * Date:           Generated on               Fri Feb 26 13:24:10 2016
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

#ifndef BCHP_SCPU_HOST_INTR2_H__
#define BCHP_SCPU_HOST_INTR2_H__

/***************************************************************************
 *SCPU_HOST_INTR2 - SCPU to Host Level 2 Interrupt Controller Registers
 ***************************************************************************/
#define BCHP_SCPU_HOST_INTR2_CPU_STATUS          0x20311040 /* [RO] CPU interrupt Status Register */
#define BCHP_SCPU_HOST_INTR2_CPU_SET             0x20311044 /* [WO] CPU interrupt Set Register */
#define BCHP_SCPU_HOST_INTR2_CPU_CLEAR           0x20311048 /* [WO] CPU interrupt Clear Register */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_STATUS     0x2031104c /* [RO] CPU interrupt Mask Status Register */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_SET        0x20311050 /* [WO] CPU interrupt Mask Set Register */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_CLEAR      0x20311054 /* [WO] CPU interrupt Mask Clear Register */
#define BCHP_SCPU_HOST_INTR2_PCI_STATUS          0x20311058 /* [RO] PCI interrupt Status Register */
#define BCHP_SCPU_HOST_INTR2_PCI_SET             0x2031105c /* [WO] PCI interrupt Set Register */
#define BCHP_SCPU_HOST_INTR2_PCI_CLEAR           0x20311060 /* [WO] PCI interrupt Clear Register */
#define BCHP_SCPU_HOST_INTR2_PCI_MASK_STATUS     0x20311064 /* [RO] PCI interrupt Mask Status Register */
#define BCHP_SCPU_HOST_INTR2_PCI_MASK_SET        0x20311068 /* [WO] PCI interrupt Mask Set Register */
#define BCHP_SCPU_HOST_INTR2_PCI_MASK_CLEAR      0x2031106c /* [WO] PCI interrupt Mask Clear Register */

/***************************************************************************
 *CPU_STATUS - CPU interrupt Status Register
 ***************************************************************************/
/* SCPU_HOST_INTR2 :: CPU_STATUS :: SPARE_SCPU_HOST [31:03] */
#define BCHP_SCPU_HOST_INTR2_CPU_STATUS_SPARE_SCPU_HOST_MASK       0xfffffff8
#define BCHP_SCPU_HOST_INTR2_CPU_STATUS_SPARE_SCPU_HOST_SHIFT      3
#define BCHP_SCPU_HOST_INTR2_CPU_STATUS_SPARE_SCPU_HOST_DEFAULT    0x00000000

/* SCPU_HOST_INTR2 :: CPU_STATUS :: SCPU_TIMER [02:02] */
#define BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_TIMER_MASK            0x00000004
#define BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_TIMER_SHIFT           2
#define BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_TIMER_DEFAULT         0x00000000

/* SCPU_HOST_INTR2 :: CPU_STATUS :: SCPU_HOST_OLOAD [01:01] */
#define BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_HOST_OLOAD_MASK       0x00000002
#define BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_HOST_OLOAD_SHIFT      1
#define BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_HOST_OLOAD_DEFAULT    0x00000000

/* SCPU_HOST_INTR2 :: CPU_STATUS :: SCPU_HOST_DRDY [00:00] */
#define BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_HOST_DRDY_MASK        0x00000001
#define BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_HOST_DRDY_SHIFT       0
#define BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_HOST_DRDY_DEFAULT     0x00000000

/***************************************************************************
 *CPU_SET - CPU interrupt Set Register
 ***************************************************************************/
/* SCPU_HOST_INTR2 :: CPU_SET :: SPARE_SCPU_HOST [31:03] */
#define BCHP_SCPU_HOST_INTR2_CPU_SET_SPARE_SCPU_HOST_MASK          0xfffffff8
#define BCHP_SCPU_HOST_INTR2_CPU_SET_SPARE_SCPU_HOST_SHIFT         3
#define BCHP_SCPU_HOST_INTR2_CPU_SET_SPARE_SCPU_HOST_DEFAULT       0x00000000

/* SCPU_HOST_INTR2 :: CPU_SET :: SCPU_TIMER [02:02] */
#define BCHP_SCPU_HOST_INTR2_CPU_SET_SCPU_TIMER_MASK               0x00000004
#define BCHP_SCPU_HOST_INTR2_CPU_SET_SCPU_TIMER_SHIFT              2
#define BCHP_SCPU_HOST_INTR2_CPU_SET_SCPU_TIMER_DEFAULT            0x00000000

/* SCPU_HOST_INTR2 :: CPU_SET :: SCPU_HOST_OLOAD [01:01] */
#define BCHP_SCPU_HOST_INTR2_CPU_SET_SCPU_HOST_OLOAD_MASK          0x00000002
#define BCHP_SCPU_HOST_INTR2_CPU_SET_SCPU_HOST_OLOAD_SHIFT         1
#define BCHP_SCPU_HOST_INTR2_CPU_SET_SCPU_HOST_OLOAD_DEFAULT       0x00000000

/* SCPU_HOST_INTR2 :: CPU_SET :: SCPU_HOST_DRDY [00:00] */
#define BCHP_SCPU_HOST_INTR2_CPU_SET_SCPU_HOST_DRDY_MASK           0x00000001
#define BCHP_SCPU_HOST_INTR2_CPU_SET_SCPU_HOST_DRDY_SHIFT          0
#define BCHP_SCPU_HOST_INTR2_CPU_SET_SCPU_HOST_DRDY_DEFAULT        0x00000000

/***************************************************************************
 *CPU_CLEAR - CPU interrupt Clear Register
 ***************************************************************************/
/* SCPU_HOST_INTR2 :: CPU_CLEAR :: SPARE_SCPU_HOST [31:03] */
#define BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SPARE_SCPU_HOST_MASK        0xfffffff8
#define BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SPARE_SCPU_HOST_SHIFT       3
#define BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SPARE_SCPU_HOST_DEFAULT     0x00000000

/* SCPU_HOST_INTR2 :: CPU_CLEAR :: SCPU_TIMER [02:02] */
#define BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SCPU_TIMER_MASK             0x00000004
#define BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SCPU_TIMER_SHIFT            2
#define BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SCPU_TIMER_DEFAULT          0x00000000

/* SCPU_HOST_INTR2 :: CPU_CLEAR :: SCPU_HOST_OLOAD [01:01] */
#define BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SCPU_HOST_OLOAD_MASK        0x00000002
#define BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SCPU_HOST_OLOAD_SHIFT       1
#define BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SCPU_HOST_OLOAD_DEFAULT     0x00000000

/* SCPU_HOST_INTR2 :: CPU_CLEAR :: SCPU_HOST_DRDY [00:00] */
#define BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SCPU_HOST_DRDY_MASK         0x00000001
#define BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SCPU_HOST_DRDY_SHIFT        0
#define BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SCPU_HOST_DRDY_DEFAULT      0x00000000

/***************************************************************************
 *CPU_MASK_STATUS - CPU interrupt Mask Status Register
 ***************************************************************************/
/* SCPU_HOST_INTR2 :: CPU_MASK_STATUS :: SPARE_SCPU_HOST [31:03] */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_STATUS_SPARE_SCPU_HOST_MASK  0xfffffff8
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_STATUS_SPARE_SCPU_HOST_SHIFT 3
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_STATUS_SPARE_SCPU_HOST_DEFAULT 0x1fffffff

/* SCPU_HOST_INTR2 :: CPU_MASK_STATUS :: SCPU_TIMER [02:02] */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_STATUS_SCPU_TIMER_MASK       0x00000004
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_STATUS_SCPU_TIMER_SHIFT      2
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_STATUS_SCPU_TIMER_DEFAULT    0x00000001

/* SCPU_HOST_INTR2 :: CPU_MASK_STATUS :: SCPU_HOST_OLOAD [01:01] */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_STATUS_SCPU_HOST_OLOAD_MASK  0x00000002
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_STATUS_SCPU_HOST_OLOAD_SHIFT 1
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_STATUS_SCPU_HOST_OLOAD_DEFAULT 0x00000001

/* SCPU_HOST_INTR2 :: CPU_MASK_STATUS :: SCPU_HOST_DRDY [00:00] */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_STATUS_SCPU_HOST_DRDY_MASK   0x00000001
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_STATUS_SCPU_HOST_DRDY_SHIFT  0
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_STATUS_SCPU_HOST_DRDY_DEFAULT 0x00000001

/***************************************************************************
 *CPU_MASK_SET - CPU interrupt Mask Set Register
 ***************************************************************************/
/* SCPU_HOST_INTR2 :: CPU_MASK_SET :: SPARE_SCPU_HOST [31:03] */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_SET_SPARE_SCPU_HOST_MASK     0xfffffff8
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_SET_SPARE_SCPU_HOST_SHIFT    3
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_SET_SPARE_SCPU_HOST_DEFAULT  0x1fffffff

/* SCPU_HOST_INTR2 :: CPU_MASK_SET :: SCPU_TIMER [02:02] */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_SET_SCPU_TIMER_MASK          0x00000004
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_SET_SCPU_TIMER_SHIFT         2
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_SET_SCPU_TIMER_DEFAULT       0x00000001

/* SCPU_HOST_INTR2 :: CPU_MASK_SET :: SCPU_HOST_OLOAD [01:01] */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_SET_SCPU_HOST_OLOAD_MASK     0x00000002
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_SET_SCPU_HOST_OLOAD_SHIFT    1
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_SET_SCPU_HOST_OLOAD_DEFAULT  0x00000001

/* SCPU_HOST_INTR2 :: CPU_MASK_SET :: SCPU_HOST_DRDY [00:00] */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_SET_SCPU_HOST_DRDY_MASK      0x00000001
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_SET_SCPU_HOST_DRDY_SHIFT     0
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_SET_SCPU_HOST_DRDY_DEFAULT   0x00000001

/***************************************************************************
 *CPU_MASK_CLEAR - CPU interrupt Mask Clear Register
 ***************************************************************************/
/* SCPU_HOST_INTR2 :: CPU_MASK_CLEAR :: SPARE_SCPU_HOST [31:03] */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_CLEAR_SPARE_SCPU_HOST_MASK   0xfffffff8
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_CLEAR_SPARE_SCPU_HOST_SHIFT  3
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_CLEAR_SPARE_SCPU_HOST_DEFAULT 0x1fffffff

/* SCPU_HOST_INTR2 :: CPU_MASK_CLEAR :: SCPU_TIMER [02:02] */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_CLEAR_SCPU_TIMER_MASK        0x00000004
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_CLEAR_SCPU_TIMER_SHIFT       2
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_CLEAR_SCPU_TIMER_DEFAULT     0x00000001

/* SCPU_HOST_INTR2 :: CPU_MASK_CLEAR :: SCPU_HOST_OLOAD [01:01] */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_CLEAR_SCPU_HOST_OLOAD_MASK   0x00000002
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_CLEAR_SCPU_HOST_OLOAD_SHIFT  1
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_CLEAR_SCPU_HOST_OLOAD_DEFAULT 0x00000001

/* SCPU_HOST_INTR2 :: CPU_MASK_CLEAR :: SCPU_HOST_DRDY [00:00] */
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_CLEAR_SCPU_HOST_DRDY_MASK    0x00000001
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_CLEAR_SCPU_HOST_DRDY_SHIFT   0
#define BCHP_SCPU_HOST_INTR2_CPU_MASK_CLEAR_SCPU_HOST_DRDY_DEFAULT 0x00000001

/***************************************************************************
 *PCI_STATUS - PCI interrupt Status Register
 ***************************************************************************/
/* SCPU_HOST_INTR2 :: PCI_STATUS :: UNUSED [31:00] */
#define BCHP_SCPU_HOST_INTR2_PCI_STATUS_UNUSED_MASK                0xffffffff
#define BCHP_SCPU_HOST_INTR2_PCI_STATUS_UNUSED_SHIFT               0
#define BCHP_SCPU_HOST_INTR2_PCI_STATUS_UNUSED_DEFAULT             0x00000000

/***************************************************************************
 *PCI_SET - PCI interrupt Set Register
 ***************************************************************************/
/* SCPU_HOST_INTR2 :: PCI_SET :: UNUSED [31:00] */
#define BCHP_SCPU_HOST_INTR2_PCI_SET_UNUSED_MASK                   0xffffffff
#define BCHP_SCPU_HOST_INTR2_PCI_SET_UNUSED_SHIFT                  0
#define BCHP_SCPU_HOST_INTR2_PCI_SET_UNUSED_DEFAULT                0x00000000

/***************************************************************************
 *PCI_CLEAR - PCI interrupt Clear Register
 ***************************************************************************/
/* SCPU_HOST_INTR2 :: PCI_CLEAR :: UNUSED [31:00] */
#define BCHP_SCPU_HOST_INTR2_PCI_CLEAR_UNUSED_MASK                 0xffffffff
#define BCHP_SCPU_HOST_INTR2_PCI_CLEAR_UNUSED_SHIFT                0
#define BCHP_SCPU_HOST_INTR2_PCI_CLEAR_UNUSED_DEFAULT              0x00000000

/***************************************************************************
 *PCI_MASK_STATUS - PCI interrupt Mask Status Register
 ***************************************************************************/
/* SCPU_HOST_INTR2 :: PCI_MASK_STATUS :: UNUSED [31:00] */
#define BCHP_SCPU_HOST_INTR2_PCI_MASK_STATUS_UNUSED_MASK           0xffffffff
#define BCHP_SCPU_HOST_INTR2_PCI_MASK_STATUS_UNUSED_SHIFT          0
#define BCHP_SCPU_HOST_INTR2_PCI_MASK_STATUS_UNUSED_DEFAULT        0xffffffff

/***************************************************************************
 *PCI_MASK_SET - PCI interrupt Mask Set Register
 ***************************************************************************/
/* SCPU_HOST_INTR2 :: PCI_MASK_SET :: UNUSED [31:00] */
#define BCHP_SCPU_HOST_INTR2_PCI_MASK_SET_UNUSED_MASK              0xffffffff
#define BCHP_SCPU_HOST_INTR2_PCI_MASK_SET_UNUSED_SHIFT             0
#define BCHP_SCPU_HOST_INTR2_PCI_MASK_SET_UNUSED_DEFAULT           0xffffffff

/***************************************************************************
 *PCI_MASK_CLEAR - PCI interrupt Mask Clear Register
 ***************************************************************************/
/* SCPU_HOST_INTR2 :: PCI_MASK_CLEAR :: UNUSED [31:00] */
#define BCHP_SCPU_HOST_INTR2_PCI_MASK_CLEAR_UNUSED_MASK            0xffffffff
#define BCHP_SCPU_HOST_INTR2_PCI_MASK_CLEAR_UNUSED_SHIFT           0
#define BCHP_SCPU_HOST_INTR2_PCI_MASK_CLEAR_UNUSED_DEFAULT         0xffffffff

#endif /* #ifndef BCHP_SCPU_HOST_INTR2_H__ */

/* End of File */
