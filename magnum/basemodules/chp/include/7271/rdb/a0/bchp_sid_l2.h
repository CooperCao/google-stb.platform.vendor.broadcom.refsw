/****************************************************************************
 *     Copyright (c) 1999-2015, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Thu Jun 18 10:52:56 2015
 *                 Full Compile MD5 Checksum  32b78c1804e11666b824f2b9450a6228
 *                     (minus title and desc)
 *                 MD5 Checksum               3452ff65b8043c1c458e059705af3b49
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     16265
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_SID_L2_H__
#define BCHP_SID_L2_H__

/***************************************************************************
 *SID_L2 - SID L2 Interrupt Ctl Registers
 ***************************************************************************/
#define BCHP_SID_L2_CPU_STATUS                   0x209a0100 /* [RO] CPU interrupt Status Register */
#define BCHP_SID_L2_CPU_SET                      0x209a0104 /* [WO] CPU interrupt Set Register */
#define BCHP_SID_L2_CPU_CLEAR                    0x209a0108 /* [WO] CPU interrupt Clear Register */
#define BCHP_SID_L2_CPU_MASK_STATUS              0x209a010c /* [RO] CPU interrupt Mask Status Register */
#define BCHP_SID_L2_CPU_MASK_SET                 0x209a0110 /* [WO] CPU interrupt Mask Set Register */
#define BCHP_SID_L2_CPU_MASK_CLEAR               0x209a0114 /* [WO] CPU interrupt Mask Clear Register */
#define BCHP_SID_L2_PCI_STATUS                   0x209a0118 /* [RO] PCI interrupt Status Register */
#define BCHP_SID_L2_PCI_SET                      0x209a011c /* [WO] PCI interrupt Set Register */
#define BCHP_SID_L2_PCI_CLEAR                    0x209a0120 /* [WO] PCI interrupt Clear Register */
#define BCHP_SID_L2_PCI_MASK_STATUS              0x209a0124 /* [RO] PCI interrupt Mask Status Register */
#define BCHP_SID_L2_PCI_MASK_SET                 0x209a0128 /* [WO] PCI interrupt Mask Set Register */
#define BCHP_SID_L2_PCI_MASK_CLEAR               0x209a012c /* [WO] PCI interrupt Mask Clear Register */

/***************************************************************************
 *CPU_STATUS - CPU interrupt Status Register
 ***************************************************************************/
/* SID_L2 :: CPU_STATUS :: reserved0 [31:06] */
#define BCHP_SID_L2_CPU_STATUS_reserved0_MASK                      0xffffffc0
#define BCHP_SID_L2_CPU_STATUS_reserved0_SHIFT                     6

/* SID_L2 :: CPU_STATUS :: SID_INTR [05:05] */
#define BCHP_SID_L2_CPU_STATUS_SID_INTR_MASK                       0x00000020
#define BCHP_SID_L2_CPU_STATUS_SID_INTR_SHIFT                      5
#define BCHP_SID_L2_CPU_STATUS_SID_INTR_DEFAULT                    0x00000000

/* SID_L2 :: CPU_STATUS :: SID_WATCHDOG_INTR [04:04] */
#define BCHP_SID_L2_CPU_STATUS_SID_WATCHDOG_INTR_MASK              0x00000010
#define BCHP_SID_L2_CPU_STATUS_SID_WATCHDOG_INTR_SHIFT             4
#define BCHP_SID_L2_CPU_STATUS_SID_WATCHDOG_INTR_DEFAULT           0x00000000

/* SID_L2 :: CPU_STATUS :: SID_MBOX_INTR [03:03] */
#define BCHP_SID_L2_CPU_STATUS_SID_MBOX_INTR_MASK                  0x00000008
#define BCHP_SID_L2_CPU_STATUS_SID_MBOX_INTR_SHIFT                 3
#define BCHP_SID_L2_CPU_STATUS_SID_MBOX_INTR_DEFAULT               0x00000000

/* SID_L2 :: CPU_STATUS :: GR_BRIDGE_INTR [02:02] */
#define BCHP_SID_L2_CPU_STATUS_GR_BRIDGE_INTR_MASK                 0x00000004
#define BCHP_SID_L2_CPU_STATUS_GR_BRIDGE_INTR_SHIFT                2
#define BCHP_SID_L2_CPU_STATUS_GR_BRIDGE_INTR_DEFAULT              0x00000000

/* SID_L2 :: CPU_STATUS :: reserved1 [01:00] */
#define BCHP_SID_L2_CPU_STATUS_reserved1_MASK                      0x00000003
#define BCHP_SID_L2_CPU_STATUS_reserved1_SHIFT                     0

/***************************************************************************
 *CPU_SET - CPU interrupt Set Register
 ***************************************************************************/
/* SID_L2 :: CPU_SET :: reserved0 [31:06] */
#define BCHP_SID_L2_CPU_SET_reserved0_MASK                         0xffffffc0
#define BCHP_SID_L2_CPU_SET_reserved0_SHIFT                        6

/* SID_L2 :: CPU_SET :: SID_INTR [05:05] */
#define BCHP_SID_L2_CPU_SET_SID_INTR_MASK                          0x00000020
#define BCHP_SID_L2_CPU_SET_SID_INTR_SHIFT                         5
#define BCHP_SID_L2_CPU_SET_SID_INTR_DEFAULT                       0x00000000

/* SID_L2 :: CPU_SET :: SID_WATCHDOG_INTR [04:04] */
#define BCHP_SID_L2_CPU_SET_SID_WATCHDOG_INTR_MASK                 0x00000010
#define BCHP_SID_L2_CPU_SET_SID_WATCHDOG_INTR_SHIFT                4
#define BCHP_SID_L2_CPU_SET_SID_WATCHDOG_INTR_DEFAULT              0x00000000

/* SID_L2 :: CPU_SET :: SID_MBOX_INTR [03:03] */
#define BCHP_SID_L2_CPU_SET_SID_MBOX_INTR_MASK                     0x00000008
#define BCHP_SID_L2_CPU_SET_SID_MBOX_INTR_SHIFT                    3
#define BCHP_SID_L2_CPU_SET_SID_MBOX_INTR_DEFAULT                  0x00000000

/* SID_L2 :: CPU_SET :: GR_BRIDGE_INTR [02:02] */
#define BCHP_SID_L2_CPU_SET_GR_BRIDGE_INTR_MASK                    0x00000004
#define BCHP_SID_L2_CPU_SET_GR_BRIDGE_INTR_SHIFT                   2
#define BCHP_SID_L2_CPU_SET_GR_BRIDGE_INTR_DEFAULT                 0x00000000

/* SID_L2 :: CPU_SET :: reserved1 [01:00] */
#define BCHP_SID_L2_CPU_SET_reserved1_MASK                         0x00000003
#define BCHP_SID_L2_CPU_SET_reserved1_SHIFT                        0

/***************************************************************************
 *CPU_CLEAR - CPU interrupt Clear Register
 ***************************************************************************/
/* SID_L2 :: CPU_CLEAR :: reserved0 [31:06] */
#define BCHP_SID_L2_CPU_CLEAR_reserved0_MASK                       0xffffffc0
#define BCHP_SID_L2_CPU_CLEAR_reserved0_SHIFT                      6

/* SID_L2 :: CPU_CLEAR :: SID_INTR [05:05] */
#define BCHP_SID_L2_CPU_CLEAR_SID_INTR_MASK                        0x00000020
#define BCHP_SID_L2_CPU_CLEAR_SID_INTR_SHIFT                       5
#define BCHP_SID_L2_CPU_CLEAR_SID_INTR_DEFAULT                     0x00000000

/* SID_L2 :: CPU_CLEAR :: SID_WATCHDOG_INTR [04:04] */
#define BCHP_SID_L2_CPU_CLEAR_SID_WATCHDOG_INTR_MASK               0x00000010
#define BCHP_SID_L2_CPU_CLEAR_SID_WATCHDOG_INTR_SHIFT              4
#define BCHP_SID_L2_CPU_CLEAR_SID_WATCHDOG_INTR_DEFAULT            0x00000000

/* SID_L2 :: CPU_CLEAR :: SID_MBOX_INTR [03:03] */
#define BCHP_SID_L2_CPU_CLEAR_SID_MBOX_INTR_MASK                   0x00000008
#define BCHP_SID_L2_CPU_CLEAR_SID_MBOX_INTR_SHIFT                  3
#define BCHP_SID_L2_CPU_CLEAR_SID_MBOX_INTR_DEFAULT                0x00000000

/* SID_L2 :: CPU_CLEAR :: GR_BRIDGE_INTR [02:02] */
#define BCHP_SID_L2_CPU_CLEAR_GR_BRIDGE_INTR_MASK                  0x00000004
#define BCHP_SID_L2_CPU_CLEAR_GR_BRIDGE_INTR_SHIFT                 2
#define BCHP_SID_L2_CPU_CLEAR_GR_BRIDGE_INTR_DEFAULT               0x00000000

/* SID_L2 :: CPU_CLEAR :: reserved1 [01:00] */
#define BCHP_SID_L2_CPU_CLEAR_reserved1_MASK                       0x00000003
#define BCHP_SID_L2_CPU_CLEAR_reserved1_SHIFT                      0

/***************************************************************************
 *CPU_MASK_STATUS - CPU interrupt Mask Status Register
 ***************************************************************************/
/* SID_L2 :: CPU_MASK_STATUS :: reserved0 [31:06] */
#define BCHP_SID_L2_CPU_MASK_STATUS_reserved0_MASK                 0xffffffc0
#define BCHP_SID_L2_CPU_MASK_STATUS_reserved0_SHIFT                6

/* SID_L2 :: CPU_MASK_STATUS :: SID_MASK [05:05] */
#define BCHP_SID_L2_CPU_MASK_STATUS_SID_MASK_MASK                  0x00000020
#define BCHP_SID_L2_CPU_MASK_STATUS_SID_MASK_SHIFT                 5
#define BCHP_SID_L2_CPU_MASK_STATUS_SID_MASK_DEFAULT               0x00000001

/* SID_L2 :: CPU_MASK_STATUS :: SID_WATCHDOG_MASK [04:04] */
#define BCHP_SID_L2_CPU_MASK_STATUS_SID_WATCHDOG_MASK_MASK         0x00000010
#define BCHP_SID_L2_CPU_MASK_STATUS_SID_WATCHDOG_MASK_SHIFT        4
#define BCHP_SID_L2_CPU_MASK_STATUS_SID_WATCHDOG_MASK_DEFAULT      0x00000001

/* SID_L2 :: CPU_MASK_STATUS :: SID_MBOX_MASK [03:03] */
#define BCHP_SID_L2_CPU_MASK_STATUS_SID_MBOX_MASK_MASK             0x00000008
#define BCHP_SID_L2_CPU_MASK_STATUS_SID_MBOX_MASK_SHIFT            3
#define BCHP_SID_L2_CPU_MASK_STATUS_SID_MBOX_MASK_DEFAULT          0x00000001

/* SID_L2 :: CPU_MASK_STATUS :: GR_BRIDGE_MASK [02:02] */
#define BCHP_SID_L2_CPU_MASK_STATUS_GR_BRIDGE_MASK_MASK            0x00000004
#define BCHP_SID_L2_CPU_MASK_STATUS_GR_BRIDGE_MASK_SHIFT           2
#define BCHP_SID_L2_CPU_MASK_STATUS_GR_BRIDGE_MASK_DEFAULT         0x00000001

/* SID_L2 :: CPU_MASK_STATUS :: reserved1 [01:00] */
#define BCHP_SID_L2_CPU_MASK_STATUS_reserved1_MASK                 0x00000003
#define BCHP_SID_L2_CPU_MASK_STATUS_reserved1_SHIFT                0

/***************************************************************************
 *CPU_MASK_SET - CPU interrupt Mask Set Register
 ***************************************************************************/
/* SID_L2 :: CPU_MASK_SET :: reserved0 [31:06] */
#define BCHP_SID_L2_CPU_MASK_SET_reserved0_MASK                    0xffffffc0
#define BCHP_SID_L2_CPU_MASK_SET_reserved0_SHIFT                   6

/* SID_L2 :: CPU_MASK_SET :: SID_MASK [05:05] */
#define BCHP_SID_L2_CPU_MASK_SET_SID_MASK_MASK                     0x00000020
#define BCHP_SID_L2_CPU_MASK_SET_SID_MASK_SHIFT                    5
#define BCHP_SID_L2_CPU_MASK_SET_SID_MASK_DEFAULT                  0x00000001

/* SID_L2 :: CPU_MASK_SET :: SID_WATCHDOG_MASK [04:04] */
#define BCHP_SID_L2_CPU_MASK_SET_SID_WATCHDOG_MASK_MASK            0x00000010
#define BCHP_SID_L2_CPU_MASK_SET_SID_WATCHDOG_MASK_SHIFT           4
#define BCHP_SID_L2_CPU_MASK_SET_SID_WATCHDOG_MASK_DEFAULT         0x00000001

/* SID_L2 :: CPU_MASK_SET :: SID_MBOX_MASK [03:03] */
#define BCHP_SID_L2_CPU_MASK_SET_SID_MBOX_MASK_MASK                0x00000008
#define BCHP_SID_L2_CPU_MASK_SET_SID_MBOX_MASK_SHIFT               3
#define BCHP_SID_L2_CPU_MASK_SET_SID_MBOX_MASK_DEFAULT             0x00000001

/* SID_L2 :: CPU_MASK_SET :: GR_BRIDGE_MASK [02:02] */
#define BCHP_SID_L2_CPU_MASK_SET_GR_BRIDGE_MASK_MASK               0x00000004
#define BCHP_SID_L2_CPU_MASK_SET_GR_BRIDGE_MASK_SHIFT              2
#define BCHP_SID_L2_CPU_MASK_SET_GR_BRIDGE_MASK_DEFAULT            0x00000001

/* SID_L2 :: CPU_MASK_SET :: reserved1 [01:00] */
#define BCHP_SID_L2_CPU_MASK_SET_reserved1_MASK                    0x00000003
#define BCHP_SID_L2_CPU_MASK_SET_reserved1_SHIFT                   0

/***************************************************************************
 *CPU_MASK_CLEAR - CPU interrupt Mask Clear Register
 ***************************************************************************/
/* SID_L2 :: CPU_MASK_CLEAR :: reserved0 [31:06] */
#define BCHP_SID_L2_CPU_MASK_CLEAR_reserved0_MASK                  0xffffffc0
#define BCHP_SID_L2_CPU_MASK_CLEAR_reserved0_SHIFT                 6

/* SID_L2 :: CPU_MASK_CLEAR :: SID_MASK [05:05] */
#define BCHP_SID_L2_CPU_MASK_CLEAR_SID_MASK_MASK                   0x00000020
#define BCHP_SID_L2_CPU_MASK_CLEAR_SID_MASK_SHIFT                  5
#define BCHP_SID_L2_CPU_MASK_CLEAR_SID_MASK_DEFAULT                0x00000001

/* SID_L2 :: CPU_MASK_CLEAR :: SID_WATCHDOG_MASK [04:04] */
#define BCHP_SID_L2_CPU_MASK_CLEAR_SID_WATCHDOG_MASK_MASK          0x00000010
#define BCHP_SID_L2_CPU_MASK_CLEAR_SID_WATCHDOG_MASK_SHIFT         4
#define BCHP_SID_L2_CPU_MASK_CLEAR_SID_WATCHDOG_MASK_DEFAULT       0x00000001

/* SID_L2 :: CPU_MASK_CLEAR :: SID_MBOX_MASK [03:03] */
#define BCHP_SID_L2_CPU_MASK_CLEAR_SID_MBOX_MASK_MASK              0x00000008
#define BCHP_SID_L2_CPU_MASK_CLEAR_SID_MBOX_MASK_SHIFT             3
#define BCHP_SID_L2_CPU_MASK_CLEAR_SID_MBOX_MASK_DEFAULT           0x00000001

/* SID_L2 :: CPU_MASK_CLEAR :: GR_BRIDGE_MASK [02:02] */
#define BCHP_SID_L2_CPU_MASK_CLEAR_GR_BRIDGE_MASK_MASK             0x00000004
#define BCHP_SID_L2_CPU_MASK_CLEAR_GR_BRIDGE_MASK_SHIFT            2
#define BCHP_SID_L2_CPU_MASK_CLEAR_GR_BRIDGE_MASK_DEFAULT          0x00000001

/* SID_L2 :: CPU_MASK_CLEAR :: reserved1 [01:00] */
#define BCHP_SID_L2_CPU_MASK_CLEAR_reserved1_MASK                  0x00000003
#define BCHP_SID_L2_CPU_MASK_CLEAR_reserved1_SHIFT                 0

/***************************************************************************
 *PCI_STATUS - PCI interrupt Status Register
 ***************************************************************************/
/* SID_L2 :: PCI_STATUS :: reserved0 [31:06] */
#define BCHP_SID_L2_PCI_STATUS_reserved0_MASK                      0xffffffc0
#define BCHP_SID_L2_PCI_STATUS_reserved0_SHIFT                     6

/* SID_L2 :: PCI_STATUS :: SID_INTR [05:05] */
#define BCHP_SID_L2_PCI_STATUS_SID_INTR_MASK                       0x00000020
#define BCHP_SID_L2_PCI_STATUS_SID_INTR_SHIFT                      5
#define BCHP_SID_L2_PCI_STATUS_SID_INTR_DEFAULT                    0x00000000

/* SID_L2 :: PCI_STATUS :: SID_WATCHDOG_INTR [04:04] */
#define BCHP_SID_L2_PCI_STATUS_SID_WATCHDOG_INTR_MASK              0x00000010
#define BCHP_SID_L2_PCI_STATUS_SID_WATCHDOG_INTR_SHIFT             4
#define BCHP_SID_L2_PCI_STATUS_SID_WATCHDOG_INTR_DEFAULT           0x00000000

/* SID_L2 :: PCI_STATUS :: SID_MBOX_INTR [03:03] */
#define BCHP_SID_L2_PCI_STATUS_SID_MBOX_INTR_MASK                  0x00000008
#define BCHP_SID_L2_PCI_STATUS_SID_MBOX_INTR_SHIFT                 3
#define BCHP_SID_L2_PCI_STATUS_SID_MBOX_INTR_DEFAULT               0x00000000

/* SID_L2 :: PCI_STATUS :: GR_BRIDGE_INTR [02:02] */
#define BCHP_SID_L2_PCI_STATUS_GR_BRIDGE_INTR_MASK                 0x00000004
#define BCHP_SID_L2_PCI_STATUS_GR_BRIDGE_INTR_SHIFT                2
#define BCHP_SID_L2_PCI_STATUS_GR_BRIDGE_INTR_DEFAULT              0x00000000

/* SID_L2 :: PCI_STATUS :: reserved1 [01:00] */
#define BCHP_SID_L2_PCI_STATUS_reserved1_MASK                      0x00000003
#define BCHP_SID_L2_PCI_STATUS_reserved1_SHIFT                     0

/***************************************************************************
 *PCI_SET - PCI interrupt Set Register
 ***************************************************************************/
/* SID_L2 :: PCI_SET :: reserved0 [31:06] */
#define BCHP_SID_L2_PCI_SET_reserved0_MASK                         0xffffffc0
#define BCHP_SID_L2_PCI_SET_reserved0_SHIFT                        6

/* SID_L2 :: PCI_SET :: SID_INTR [05:05] */
#define BCHP_SID_L2_PCI_SET_SID_INTR_MASK                          0x00000020
#define BCHP_SID_L2_PCI_SET_SID_INTR_SHIFT                         5
#define BCHP_SID_L2_PCI_SET_SID_INTR_DEFAULT                       0x00000000

/* SID_L2 :: PCI_SET :: SID_WATCHDOG_INTR [04:04] */
#define BCHP_SID_L2_PCI_SET_SID_WATCHDOG_INTR_MASK                 0x00000010
#define BCHP_SID_L2_PCI_SET_SID_WATCHDOG_INTR_SHIFT                4
#define BCHP_SID_L2_PCI_SET_SID_WATCHDOG_INTR_DEFAULT              0x00000000

/* SID_L2 :: PCI_SET :: SID_MBOX_INTR [03:03] */
#define BCHP_SID_L2_PCI_SET_SID_MBOX_INTR_MASK                     0x00000008
#define BCHP_SID_L2_PCI_SET_SID_MBOX_INTR_SHIFT                    3
#define BCHP_SID_L2_PCI_SET_SID_MBOX_INTR_DEFAULT                  0x00000000

/* SID_L2 :: PCI_SET :: GR_BRIDGE_INTR [02:02] */
#define BCHP_SID_L2_PCI_SET_GR_BRIDGE_INTR_MASK                    0x00000004
#define BCHP_SID_L2_PCI_SET_GR_BRIDGE_INTR_SHIFT                   2
#define BCHP_SID_L2_PCI_SET_GR_BRIDGE_INTR_DEFAULT                 0x00000000

/* SID_L2 :: PCI_SET :: reserved1 [01:00] */
#define BCHP_SID_L2_PCI_SET_reserved1_MASK                         0x00000003
#define BCHP_SID_L2_PCI_SET_reserved1_SHIFT                        0

/***************************************************************************
 *PCI_CLEAR - PCI interrupt Clear Register
 ***************************************************************************/
/* SID_L2 :: PCI_CLEAR :: reserved0 [31:06] */
#define BCHP_SID_L2_PCI_CLEAR_reserved0_MASK                       0xffffffc0
#define BCHP_SID_L2_PCI_CLEAR_reserved0_SHIFT                      6

/* SID_L2 :: PCI_CLEAR :: SID_INTR [05:05] */
#define BCHP_SID_L2_PCI_CLEAR_SID_INTR_MASK                        0x00000020
#define BCHP_SID_L2_PCI_CLEAR_SID_INTR_SHIFT                       5
#define BCHP_SID_L2_PCI_CLEAR_SID_INTR_DEFAULT                     0x00000000

/* SID_L2 :: PCI_CLEAR :: SID_WATCHDOG_INTR [04:04] */
#define BCHP_SID_L2_PCI_CLEAR_SID_WATCHDOG_INTR_MASK               0x00000010
#define BCHP_SID_L2_PCI_CLEAR_SID_WATCHDOG_INTR_SHIFT              4
#define BCHP_SID_L2_PCI_CLEAR_SID_WATCHDOG_INTR_DEFAULT            0x00000000

/* SID_L2 :: PCI_CLEAR :: SID_MBOX_INTR [03:03] */
#define BCHP_SID_L2_PCI_CLEAR_SID_MBOX_INTR_MASK                   0x00000008
#define BCHP_SID_L2_PCI_CLEAR_SID_MBOX_INTR_SHIFT                  3
#define BCHP_SID_L2_PCI_CLEAR_SID_MBOX_INTR_DEFAULT                0x00000000

/* SID_L2 :: PCI_CLEAR :: GR_BRIDGE_INTR [02:02] */
#define BCHP_SID_L2_PCI_CLEAR_GR_BRIDGE_INTR_MASK                  0x00000004
#define BCHP_SID_L2_PCI_CLEAR_GR_BRIDGE_INTR_SHIFT                 2
#define BCHP_SID_L2_PCI_CLEAR_GR_BRIDGE_INTR_DEFAULT               0x00000000

/* SID_L2 :: PCI_CLEAR :: reserved1 [01:00] */
#define BCHP_SID_L2_PCI_CLEAR_reserved1_MASK                       0x00000003
#define BCHP_SID_L2_PCI_CLEAR_reserved1_SHIFT                      0

/***************************************************************************
 *PCI_MASK_STATUS - PCI interrupt Mask Status Register
 ***************************************************************************/
/* SID_L2 :: PCI_MASK_STATUS :: reserved0 [31:06] */
#define BCHP_SID_L2_PCI_MASK_STATUS_reserved0_MASK                 0xffffffc0
#define BCHP_SID_L2_PCI_MASK_STATUS_reserved0_SHIFT                6

/* SID_L2 :: PCI_MASK_STATUS :: SID_MASK [05:05] */
#define BCHP_SID_L2_PCI_MASK_STATUS_SID_MASK_MASK                  0x00000020
#define BCHP_SID_L2_PCI_MASK_STATUS_SID_MASK_SHIFT                 5
#define BCHP_SID_L2_PCI_MASK_STATUS_SID_MASK_DEFAULT               0x00000001

/* SID_L2 :: PCI_MASK_STATUS :: SID_WATCHDOG_MASK [04:04] */
#define BCHP_SID_L2_PCI_MASK_STATUS_SID_WATCHDOG_MASK_MASK         0x00000010
#define BCHP_SID_L2_PCI_MASK_STATUS_SID_WATCHDOG_MASK_SHIFT        4
#define BCHP_SID_L2_PCI_MASK_STATUS_SID_WATCHDOG_MASK_DEFAULT      0x00000001

/* SID_L2 :: PCI_MASK_STATUS :: SID_MBOX_MASK [03:03] */
#define BCHP_SID_L2_PCI_MASK_STATUS_SID_MBOX_MASK_MASK             0x00000008
#define BCHP_SID_L2_PCI_MASK_STATUS_SID_MBOX_MASK_SHIFT            3
#define BCHP_SID_L2_PCI_MASK_STATUS_SID_MBOX_MASK_DEFAULT          0x00000001

/* SID_L2 :: PCI_MASK_STATUS :: GR_BRIDGE_MASK [02:02] */
#define BCHP_SID_L2_PCI_MASK_STATUS_GR_BRIDGE_MASK_MASK            0x00000004
#define BCHP_SID_L2_PCI_MASK_STATUS_GR_BRIDGE_MASK_SHIFT           2
#define BCHP_SID_L2_PCI_MASK_STATUS_GR_BRIDGE_MASK_DEFAULT         0x00000001

/* SID_L2 :: PCI_MASK_STATUS :: reserved1 [01:00] */
#define BCHP_SID_L2_PCI_MASK_STATUS_reserved1_MASK                 0x00000003
#define BCHP_SID_L2_PCI_MASK_STATUS_reserved1_SHIFT                0

/***************************************************************************
 *PCI_MASK_SET - PCI interrupt Mask Set Register
 ***************************************************************************/
/* SID_L2 :: PCI_MASK_SET :: reserved0 [31:06] */
#define BCHP_SID_L2_PCI_MASK_SET_reserved0_MASK                    0xffffffc0
#define BCHP_SID_L2_PCI_MASK_SET_reserved0_SHIFT                   6

/* SID_L2 :: PCI_MASK_SET :: SID_MASK [05:05] */
#define BCHP_SID_L2_PCI_MASK_SET_SID_MASK_MASK                     0x00000020
#define BCHP_SID_L2_PCI_MASK_SET_SID_MASK_SHIFT                    5
#define BCHP_SID_L2_PCI_MASK_SET_SID_MASK_DEFAULT                  0x00000001

/* SID_L2 :: PCI_MASK_SET :: SID_WATCHDOG_MASK [04:04] */
#define BCHP_SID_L2_PCI_MASK_SET_SID_WATCHDOG_MASK_MASK            0x00000010
#define BCHP_SID_L2_PCI_MASK_SET_SID_WATCHDOG_MASK_SHIFT           4
#define BCHP_SID_L2_PCI_MASK_SET_SID_WATCHDOG_MASK_DEFAULT         0x00000001

/* SID_L2 :: PCI_MASK_SET :: SID_MBOX_MASK [03:03] */
#define BCHP_SID_L2_PCI_MASK_SET_SID_MBOX_MASK_MASK                0x00000008
#define BCHP_SID_L2_PCI_MASK_SET_SID_MBOX_MASK_SHIFT               3
#define BCHP_SID_L2_PCI_MASK_SET_SID_MBOX_MASK_DEFAULT             0x00000001

/* SID_L2 :: PCI_MASK_SET :: GR_BRIDGE_MASK [02:02] */
#define BCHP_SID_L2_PCI_MASK_SET_GR_BRIDGE_MASK_MASK               0x00000004
#define BCHP_SID_L2_PCI_MASK_SET_GR_BRIDGE_MASK_SHIFT              2
#define BCHP_SID_L2_PCI_MASK_SET_GR_BRIDGE_MASK_DEFAULT            0x00000001

/* SID_L2 :: PCI_MASK_SET :: reserved1 [01:00] */
#define BCHP_SID_L2_PCI_MASK_SET_reserved1_MASK                    0x00000003
#define BCHP_SID_L2_PCI_MASK_SET_reserved1_SHIFT                   0

/***************************************************************************
 *PCI_MASK_CLEAR - PCI interrupt Mask Clear Register
 ***************************************************************************/
/* SID_L2 :: PCI_MASK_CLEAR :: reserved0 [31:06] */
#define BCHP_SID_L2_PCI_MASK_CLEAR_reserved0_MASK                  0xffffffc0
#define BCHP_SID_L2_PCI_MASK_CLEAR_reserved0_SHIFT                 6

/* SID_L2 :: PCI_MASK_CLEAR :: SID_MASK [05:05] */
#define BCHP_SID_L2_PCI_MASK_CLEAR_SID_MASK_MASK                   0x00000020
#define BCHP_SID_L2_PCI_MASK_CLEAR_SID_MASK_SHIFT                  5
#define BCHP_SID_L2_PCI_MASK_CLEAR_SID_MASK_DEFAULT                0x00000001

/* SID_L2 :: PCI_MASK_CLEAR :: SID_WATCHDOG_MASK [04:04] */
#define BCHP_SID_L2_PCI_MASK_CLEAR_SID_WATCHDOG_MASK_MASK          0x00000010
#define BCHP_SID_L2_PCI_MASK_CLEAR_SID_WATCHDOG_MASK_SHIFT         4
#define BCHP_SID_L2_PCI_MASK_CLEAR_SID_WATCHDOG_MASK_DEFAULT       0x00000001

/* SID_L2 :: PCI_MASK_CLEAR :: SID_MBOX_MASK [03:03] */
#define BCHP_SID_L2_PCI_MASK_CLEAR_SID_MBOX_MASK_MASK              0x00000008
#define BCHP_SID_L2_PCI_MASK_CLEAR_SID_MBOX_MASK_SHIFT             3
#define BCHP_SID_L2_PCI_MASK_CLEAR_SID_MBOX_MASK_DEFAULT           0x00000001

/* SID_L2 :: PCI_MASK_CLEAR :: GR_BRIDGE_MASK [02:02] */
#define BCHP_SID_L2_PCI_MASK_CLEAR_GR_BRIDGE_MASK_MASK             0x00000004
#define BCHP_SID_L2_PCI_MASK_CLEAR_GR_BRIDGE_MASK_SHIFT            2
#define BCHP_SID_L2_PCI_MASK_CLEAR_GR_BRIDGE_MASK_DEFAULT          0x00000001

/* SID_L2 :: PCI_MASK_CLEAR :: reserved1 [01:00] */
#define BCHP_SID_L2_PCI_MASK_CLEAR_reserved1_MASK                  0x00000003
#define BCHP_SID_L2_PCI_MASK_CLEAR_reserved1_SHIFT                 0

#endif /* #ifndef BCHP_SID_L2_H__ */

/* End of File */
