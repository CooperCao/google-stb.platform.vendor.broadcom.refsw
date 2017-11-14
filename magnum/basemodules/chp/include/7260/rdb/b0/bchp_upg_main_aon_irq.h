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

#ifndef BCHP_UPG_MAIN_AON_IRQ_H__
#define BCHP_UPG_MAIN_AON_IRQ_H__

/***************************************************************************
 *UPG_MAIN_AON_IRQ - UPG Main AON Level 2 Interrupt Enable/Status
 ***************************************************************************/
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS         0x20417400 /* [RO][32] CPU interrupt Status Register */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS    0x20417404 /* [RO][32] CPU interrupt Mask Status Register */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET       0x20417408 /* [WO][32] CPU interrupt Mask Set Register */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR     0x2041740c /* [WO][32] CPU interrupt Mask Clear Register */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS         0x20417410 /* [RO][32] PCI interrupt Status Register */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS    0x20417414 /* [RO][32] PCI interrupt Mask Status Register */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET       0x20417418 /* [WO][32] PCI interrupt Mask Set Register */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR     0x2041741c /* [WO][32] PCI interrupt Mask Clear Register */

/***************************************************************************
 *CPU_STATUS - CPU interrupt Status Register
 ***************************************************************************/
/* UPG_MAIN_AON_IRQ :: CPU_STATUS :: reserved0 [31:07] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_reserved0_MASK            0xffffff80
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_reserved0_SHIFT           7

/* UPG_MAIN_AON_IRQ :: CPU_STATUS :: spare_00 [06:06] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_spare_00_MASK             0x00000040
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_spare_00_SHIFT            6
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_spare_00_DEFAULT          0x00000000

/* UPG_MAIN_AON_IRQ :: CPU_STATUS :: icap [05:05] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_icap_MASK                 0x00000020
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_icap_SHIFT                5
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_icap_DEFAULT              0x00000000

/* UPG_MAIN_AON_IRQ :: CPU_STATUS :: ldk [04:04] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_ldk_MASK                  0x00000010
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_ldk_SHIFT                 4
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_ldk_DEFAULT               0x00000000

/* UPG_MAIN_AON_IRQ :: CPU_STATUS :: gio [03:03] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_gio_MASK                  0x00000008
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_gio_SHIFT                 3
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_gio_DEFAULT               0x00000000

/* UPG_MAIN_AON_IRQ :: CPU_STATUS :: kbd3 [02:02] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_kbd3_MASK                 0x00000004
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_kbd3_SHIFT                2
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_kbd3_DEFAULT              0x00000000

/* UPG_MAIN_AON_IRQ :: CPU_STATUS :: kbd2 [01:01] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_kbd2_MASK                 0x00000002
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_kbd2_SHIFT                1
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_kbd2_DEFAULT              0x00000000

/* UPG_MAIN_AON_IRQ :: CPU_STATUS :: kbd1 [00:00] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_kbd1_MASK                 0x00000001
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_kbd1_SHIFT                0
#define BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_kbd1_DEFAULT              0x00000000

/***************************************************************************
 *CPU_MASK_STATUS - CPU interrupt Mask Status Register
 ***************************************************************************/
/* UPG_MAIN_AON_IRQ :: CPU_MASK_STATUS :: reserved0 [31:07] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_reserved0_MASK       0xffffff80
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_reserved0_SHIFT      7

/* UPG_MAIN_AON_IRQ :: CPU_MASK_STATUS :: spare_00 [06:06] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_spare_00_MASK        0x00000040
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_spare_00_SHIFT       6
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_spare_00_DEFAULT     0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_STATUS :: icap [05:05] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_icap_MASK            0x00000020
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_icap_SHIFT           5
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_icap_DEFAULT         0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_STATUS :: ldk [04:04] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_ldk_MASK             0x00000010
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_ldk_SHIFT            4
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_ldk_DEFAULT          0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_STATUS :: gio [03:03] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_gio_MASK             0x00000008
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_gio_SHIFT            3
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_gio_DEFAULT          0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_STATUS :: kbd3 [02:02] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_kbd3_MASK            0x00000004
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_kbd3_SHIFT           2
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_kbd3_DEFAULT         0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_STATUS :: kbd2 [01:01] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_kbd2_MASK            0x00000002
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_kbd2_SHIFT           1
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_kbd2_DEFAULT         0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_STATUS :: kbd1 [00:00] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_kbd1_MASK            0x00000001
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_kbd1_SHIFT           0
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_STATUS_kbd1_DEFAULT         0x00000001

/***************************************************************************
 *CPU_MASK_SET - CPU interrupt Mask Set Register
 ***************************************************************************/
/* UPG_MAIN_AON_IRQ :: CPU_MASK_SET :: reserved0 [31:07] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_reserved0_MASK          0xffffff80
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_reserved0_SHIFT         7

/* UPG_MAIN_AON_IRQ :: CPU_MASK_SET :: spare_00 [06:06] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_spare_00_MASK           0x00000040
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_spare_00_SHIFT          6
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_spare_00_DEFAULT        0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_SET :: icap [05:05] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_icap_MASK               0x00000020
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_icap_SHIFT              5
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_icap_DEFAULT            0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_SET :: ldk [04:04] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_ldk_MASK                0x00000010
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_ldk_SHIFT               4
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_ldk_DEFAULT             0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_SET :: gio [03:03] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_gio_MASK                0x00000008
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_gio_SHIFT               3
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_gio_DEFAULT             0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_SET :: kbd3 [02:02] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_kbd3_MASK               0x00000004
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_kbd3_SHIFT              2
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_kbd3_DEFAULT            0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_SET :: kbd2 [01:01] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_kbd2_MASK               0x00000002
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_kbd2_SHIFT              1
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_kbd2_DEFAULT            0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_SET :: kbd1 [00:00] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_kbd1_MASK               0x00000001
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_kbd1_SHIFT              0
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_SET_kbd1_DEFAULT            0x00000001

/***************************************************************************
 *CPU_MASK_CLEAR - CPU interrupt Mask Clear Register
 ***************************************************************************/
/* UPG_MAIN_AON_IRQ :: CPU_MASK_CLEAR :: reserved0 [31:07] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_reserved0_MASK        0xffffff80
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_reserved0_SHIFT       7

/* UPG_MAIN_AON_IRQ :: CPU_MASK_CLEAR :: spare_00 [06:06] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_spare_00_MASK         0x00000040
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_spare_00_SHIFT        6
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_spare_00_DEFAULT      0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_CLEAR :: icap [05:05] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_icap_MASK             0x00000020
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_icap_SHIFT            5
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_icap_DEFAULT          0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_CLEAR :: ldk [04:04] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_ldk_MASK              0x00000010
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_ldk_SHIFT             4
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_ldk_DEFAULT           0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_CLEAR :: gio [03:03] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_gio_MASK              0x00000008
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_gio_SHIFT             3
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_gio_DEFAULT           0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_CLEAR :: kbd3 [02:02] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_kbd3_MASK             0x00000004
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_kbd3_SHIFT            2
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_kbd3_DEFAULT          0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_CLEAR :: kbd2 [01:01] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_kbd2_MASK             0x00000002
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_kbd2_SHIFT            1
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_kbd2_DEFAULT          0x00000001

/* UPG_MAIN_AON_IRQ :: CPU_MASK_CLEAR :: kbd1 [00:00] */
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_kbd1_MASK             0x00000001
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_kbd1_SHIFT            0
#define BCHP_UPG_MAIN_AON_IRQ_CPU_MASK_CLEAR_kbd1_DEFAULT          0x00000001

/***************************************************************************
 *PCI_STATUS - PCI interrupt Status Register
 ***************************************************************************/
/* UPG_MAIN_AON_IRQ :: PCI_STATUS :: reserved0 [31:07] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_reserved0_MASK            0xffffff80
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_reserved0_SHIFT           7

/* UPG_MAIN_AON_IRQ :: PCI_STATUS :: spare_00 [06:06] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_spare_00_MASK             0x00000040
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_spare_00_SHIFT            6
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_spare_00_DEFAULT          0x00000000

/* UPG_MAIN_AON_IRQ :: PCI_STATUS :: icap [05:05] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_icap_MASK                 0x00000020
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_icap_SHIFT                5
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_icap_DEFAULT              0x00000000

/* UPG_MAIN_AON_IRQ :: PCI_STATUS :: ldk [04:04] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_ldk_MASK                  0x00000010
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_ldk_SHIFT                 4
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_ldk_DEFAULT               0x00000000

/* UPG_MAIN_AON_IRQ :: PCI_STATUS :: gio [03:03] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_gio_MASK                  0x00000008
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_gio_SHIFT                 3
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_gio_DEFAULT               0x00000000

/* UPG_MAIN_AON_IRQ :: PCI_STATUS :: kbd3 [02:02] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_kbd3_MASK                 0x00000004
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_kbd3_SHIFT                2
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_kbd3_DEFAULT              0x00000000

/* UPG_MAIN_AON_IRQ :: PCI_STATUS :: kbd2 [01:01] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_kbd2_MASK                 0x00000002
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_kbd2_SHIFT                1
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_kbd2_DEFAULT              0x00000000

/* UPG_MAIN_AON_IRQ :: PCI_STATUS :: kbd1 [00:00] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_kbd1_MASK                 0x00000001
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_kbd1_SHIFT                0
#define BCHP_UPG_MAIN_AON_IRQ_PCI_STATUS_kbd1_DEFAULT              0x00000000

/***************************************************************************
 *PCI_MASK_STATUS - PCI interrupt Mask Status Register
 ***************************************************************************/
/* UPG_MAIN_AON_IRQ :: PCI_MASK_STATUS :: reserved0 [31:07] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_reserved0_MASK       0xffffff80
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_reserved0_SHIFT      7

/* UPG_MAIN_AON_IRQ :: PCI_MASK_STATUS :: spare_00 [06:06] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_spare_00_MASK        0x00000040
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_spare_00_SHIFT       6
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_spare_00_DEFAULT     0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_STATUS :: icap [05:05] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_icap_MASK            0x00000020
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_icap_SHIFT           5
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_icap_DEFAULT         0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_STATUS :: ldk [04:04] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_ldk_MASK             0x00000010
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_ldk_SHIFT            4
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_ldk_DEFAULT          0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_STATUS :: gio [03:03] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_gio_MASK             0x00000008
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_gio_SHIFT            3
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_gio_DEFAULT          0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_STATUS :: kbd3 [02:02] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_kbd3_MASK            0x00000004
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_kbd3_SHIFT           2
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_kbd3_DEFAULT         0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_STATUS :: kbd2 [01:01] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_kbd2_MASK            0x00000002
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_kbd2_SHIFT           1
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_kbd2_DEFAULT         0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_STATUS :: kbd1 [00:00] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_kbd1_MASK            0x00000001
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_kbd1_SHIFT           0
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_STATUS_kbd1_DEFAULT         0x00000001

/***************************************************************************
 *PCI_MASK_SET - PCI interrupt Mask Set Register
 ***************************************************************************/
/* UPG_MAIN_AON_IRQ :: PCI_MASK_SET :: reserved0 [31:07] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_reserved0_MASK          0xffffff80
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_reserved0_SHIFT         7

/* UPG_MAIN_AON_IRQ :: PCI_MASK_SET :: spare_00 [06:06] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_spare_00_MASK           0x00000040
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_spare_00_SHIFT          6
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_spare_00_DEFAULT        0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_SET :: icap [05:05] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_icap_MASK               0x00000020
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_icap_SHIFT              5
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_icap_DEFAULT            0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_SET :: ldk [04:04] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_ldk_MASK                0x00000010
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_ldk_SHIFT               4
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_ldk_DEFAULT             0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_SET :: gio [03:03] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_gio_MASK                0x00000008
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_gio_SHIFT               3
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_gio_DEFAULT             0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_SET :: kbd3 [02:02] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_kbd3_MASK               0x00000004
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_kbd3_SHIFT              2
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_kbd3_DEFAULT            0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_SET :: kbd2 [01:01] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_kbd2_MASK               0x00000002
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_kbd2_SHIFT              1
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_kbd2_DEFAULT            0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_SET :: kbd1 [00:00] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_kbd1_MASK               0x00000001
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_kbd1_SHIFT              0
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_SET_kbd1_DEFAULT            0x00000001

/***************************************************************************
 *PCI_MASK_CLEAR - PCI interrupt Mask Clear Register
 ***************************************************************************/
/* UPG_MAIN_AON_IRQ :: PCI_MASK_CLEAR :: reserved0 [31:07] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_reserved0_MASK        0xffffff80
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_reserved0_SHIFT       7

/* UPG_MAIN_AON_IRQ :: PCI_MASK_CLEAR :: spare_00 [06:06] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_spare_00_MASK         0x00000040
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_spare_00_SHIFT        6
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_spare_00_DEFAULT      0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_CLEAR :: icap [05:05] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_icap_MASK             0x00000020
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_icap_SHIFT            5
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_icap_DEFAULT          0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_CLEAR :: ldk [04:04] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_ldk_MASK              0x00000010
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_ldk_SHIFT             4
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_ldk_DEFAULT           0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_CLEAR :: gio [03:03] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_gio_MASK              0x00000008
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_gio_SHIFT             3
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_gio_DEFAULT           0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_CLEAR :: kbd3 [02:02] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_kbd3_MASK             0x00000004
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_kbd3_SHIFT            2
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_kbd3_DEFAULT          0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_CLEAR :: kbd2 [01:01] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_kbd2_MASK             0x00000002
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_kbd2_SHIFT            1
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_kbd2_DEFAULT          0x00000001

/* UPG_MAIN_AON_IRQ :: PCI_MASK_CLEAR :: kbd1 [00:00] */
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_kbd1_MASK             0x00000001
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_kbd1_SHIFT            0
#define BCHP_UPG_MAIN_AON_IRQ_PCI_MASK_CLEAR_kbd1_DEFAULT          0x00000001

#endif /* #ifndef BCHP_UPG_MAIN_AON_IRQ_H__ */

/* End of File */
