/***************************************************************************
 *     Copyright (c) 1999-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on         Thu Apr 22 17:25:23 2010
 *                 MD5 Checksum         e645cb91be669b614b9400e5da71f4fa
 *
 * Compiled with:  RDB Utility          combo_header.pl
 *                 RDB Parser           3.0
 *                 unknown              unknown
 *                 Perl Interpreter     5.008008
 *                 Operating System     linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_IPI0_INTR2_H__
#define BCHP_IPI0_INTR2_H__

/***************************************************************************
 *IPI0_INTR2 - Inter-Processor Level 2 Interrupt Controller 0 Registers
 ***************************************************************************/
#define BCHP_IPI0_INTR2_CPU_STATUS               0x00441100 /* CPU interrupt Status Register */
#define BCHP_IPI0_INTR2_CPU_SET                  0x00441104 /* CPU interrupt Set Register */
#define BCHP_IPI0_INTR2_CPU_CLEAR                0x00441108 /* CPU interrupt Clear Register */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS          0x0044110c /* CPU interrupt Mask Status Register */
#define BCHP_IPI0_INTR2_CPU_MASK_SET             0x00441110 /* CPU interrupt Mask Set Register */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR           0x00441114 /* CPU interrupt Mask Clear Register */
#define BCHP_IPI0_INTR2_PCI_STATUS               0x00441118 /* PCI interrupt Status Register */
#define BCHP_IPI0_INTR2_PCI_SET                  0x0044111c /* PCI interrupt Set Register */
#define BCHP_IPI0_INTR2_PCI_CLEAR                0x00441120 /* PCI interrupt Clear Register */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS          0x00441124 /* PCI interrupt Mask Status Register */
#define BCHP_IPI0_INTR2_PCI_MASK_SET             0x00441128 /* PCI interrupt Mask Set Register */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR           0x0044112c /* PCI interrupt Mask Clear Register */

/***************************************************************************
 *CPU_STATUS - CPU interrupt Status Register
 ***************************************************************************/
/* IPI0_INTR2 :: CPU_STATUS :: reserved0 [31:12] */
#define BCHP_IPI0_INTR2_CPU_STATUS_reserved0_MASK                  0xfffff000
#define BCHP_IPI0_INTR2_CPU_STATUS_reserved0_SHIFT                 12

/* IPI0_INTR2 :: CPU_STATUS :: SPARE_BIT_11 [11:11] */
#define BCHP_IPI0_INTR2_CPU_STATUS_SPARE_BIT_11_MASK               0x00000800
#define BCHP_IPI0_INTR2_CPU_STATUS_SPARE_BIT_11_SHIFT              11

/* IPI0_INTR2 :: CPU_STATUS :: SPARE_BIT_10 [10:10] */
#define BCHP_IPI0_INTR2_CPU_STATUS_SPARE_BIT_10_MASK               0x00000400
#define BCHP_IPI0_INTR2_CPU_STATUS_SPARE_BIT_10_SHIFT              10

/* IPI0_INTR2 :: CPU_STATUS :: SPARE_BIT_9 [09:09] */
#define BCHP_IPI0_INTR2_CPU_STATUS_SPARE_BIT_9_MASK                0x00000200
#define BCHP_IPI0_INTR2_CPU_STATUS_SPARE_BIT_9_SHIFT               9

/* IPI0_INTR2 :: CPU_STATUS :: SPARE_BIT_8 [08:08] */
#define BCHP_IPI0_INTR2_CPU_STATUS_SPARE_BIT_8_MASK                0x00000100
#define BCHP_IPI0_INTR2_CPU_STATUS_SPARE_BIT_8_SHIFT               8

/* IPI0_INTR2 :: CPU_STATUS :: MOCA [07:07] */
#define BCHP_IPI0_INTR2_CPU_STATUS_MOCA_MASK                       0x00000080
#define BCHP_IPI0_INTR2_CPU_STATUS_MOCA_SHIFT                      7

/* IPI0_INTR2 :: CPU_STATUS :: AVD1 [06:06] */
#define BCHP_IPI0_INTR2_CPU_STATUS_AVD1_MASK                       0x00000040
#define BCHP_IPI0_INTR2_CPU_STATUS_AVD1_SHIFT                      6

/* IPI0_INTR2 :: CPU_STATUS :: AVD0 [05:05] */
#define BCHP_IPI0_INTR2_CPU_STATUS_AVD0_MASK                       0x00000020
#define BCHP_IPI0_INTR2_CPU_STATUS_AVD0_SHIFT                      5

/* IPI0_INTR2 :: CPU_STATUS :: RPDT [04:04] */
#define BCHP_IPI0_INTR2_CPU_STATUS_RPDT_MASK                       0x00000010
#define BCHP_IPI0_INTR2_CPU_STATUS_RPDT_SHIFT                      4

/* IPI0_INTR2 :: CPU_STATUS :: SSP [03:03] */
#define BCHP_IPI0_INTR2_CPU_STATUS_SSP_MASK                        0x00000008
#define BCHP_IPI0_INTR2_CPU_STATUS_SSP_SHIFT                       3

/* IPI0_INTR2 :: CPU_STATUS :: PCI [02:02] */
#define BCHP_IPI0_INTR2_CPU_STATUS_PCI_MASK                        0x00000004
#define BCHP_IPI0_INTR2_CPU_STATUS_PCI_SHIFT                       2

/* IPI0_INTR2 :: CPU_STATUS :: TP1 [01:01] */
#define BCHP_IPI0_INTR2_CPU_STATUS_TP1_MASK                        0x00000002
#define BCHP_IPI0_INTR2_CPU_STATUS_TP1_SHIFT                       1

/* IPI0_INTR2 :: CPU_STATUS :: TP0 [00:00] */
#define BCHP_IPI0_INTR2_CPU_STATUS_TP0_MASK                        0x00000001
#define BCHP_IPI0_INTR2_CPU_STATUS_TP0_SHIFT                       0

/***************************************************************************
 *CPU_SET - CPU interrupt Set Register
 ***************************************************************************/
/* IPI0_INTR2 :: CPU_SET :: reserved0 [31:12] */
#define BCHP_IPI0_INTR2_CPU_SET_reserved0_MASK                     0xfffff000
#define BCHP_IPI0_INTR2_CPU_SET_reserved0_SHIFT                    12

/* IPI0_INTR2 :: CPU_SET :: SPARE_BIT_11 [11:11] */
#define BCHP_IPI0_INTR2_CPU_SET_SPARE_BIT_11_MASK                  0x00000800
#define BCHP_IPI0_INTR2_CPU_SET_SPARE_BIT_11_SHIFT                 11

/* IPI0_INTR2 :: CPU_SET :: SPARE_BIT_10 [10:10] */
#define BCHP_IPI0_INTR2_CPU_SET_SPARE_BIT_10_MASK                  0x00000400
#define BCHP_IPI0_INTR2_CPU_SET_SPARE_BIT_10_SHIFT                 10

/* IPI0_INTR2 :: CPU_SET :: SPARE_BIT_9 [09:09] */
#define BCHP_IPI0_INTR2_CPU_SET_SPARE_BIT_9_MASK                   0x00000200
#define BCHP_IPI0_INTR2_CPU_SET_SPARE_BIT_9_SHIFT                  9

/* IPI0_INTR2 :: CPU_SET :: SPARE_BIT_8 [08:08] */
#define BCHP_IPI0_INTR2_CPU_SET_SPARE_BIT_8_MASK                   0x00000100
#define BCHP_IPI0_INTR2_CPU_SET_SPARE_BIT_8_SHIFT                  8

/* IPI0_INTR2 :: CPU_SET :: MOCA [07:07] */
#define BCHP_IPI0_INTR2_CPU_SET_MOCA_MASK                          0x00000080
#define BCHP_IPI0_INTR2_CPU_SET_MOCA_SHIFT                         7

/* IPI0_INTR2 :: CPU_SET :: AVD1 [06:06] */
#define BCHP_IPI0_INTR2_CPU_SET_AVD1_MASK                          0x00000040
#define BCHP_IPI0_INTR2_CPU_SET_AVD1_SHIFT                         6

/* IPI0_INTR2 :: CPU_SET :: AVD0 [05:05] */
#define BCHP_IPI0_INTR2_CPU_SET_AVD0_MASK                          0x00000020
#define BCHP_IPI0_INTR2_CPU_SET_AVD0_SHIFT                         5

/* IPI0_INTR2 :: CPU_SET :: RPDT [04:04] */
#define BCHP_IPI0_INTR2_CPU_SET_RPDT_MASK                          0x00000010
#define BCHP_IPI0_INTR2_CPU_SET_RPDT_SHIFT                         4

/* IPI0_INTR2 :: CPU_SET :: SSP [03:03] */
#define BCHP_IPI0_INTR2_CPU_SET_SSP_MASK                           0x00000008
#define BCHP_IPI0_INTR2_CPU_SET_SSP_SHIFT                          3

/* IPI0_INTR2 :: CPU_SET :: PCI [02:02] */
#define BCHP_IPI0_INTR2_CPU_SET_PCI_MASK                           0x00000004
#define BCHP_IPI0_INTR2_CPU_SET_PCI_SHIFT                          2

/* IPI0_INTR2 :: CPU_SET :: TP1 [01:01] */
#define BCHP_IPI0_INTR2_CPU_SET_TP1_MASK                           0x00000002
#define BCHP_IPI0_INTR2_CPU_SET_TP1_SHIFT                          1

/* IPI0_INTR2 :: CPU_SET :: TP0 [00:00] */
#define BCHP_IPI0_INTR2_CPU_SET_TP0_MASK                           0x00000001
#define BCHP_IPI0_INTR2_CPU_SET_TP0_SHIFT                          0

/***************************************************************************
 *CPU_CLEAR - CPU interrupt Clear Register
 ***************************************************************************/
/* IPI0_INTR2 :: CPU_CLEAR :: reserved0 [31:12] */
#define BCHP_IPI0_INTR2_CPU_CLEAR_reserved0_MASK                   0xfffff000
#define BCHP_IPI0_INTR2_CPU_CLEAR_reserved0_SHIFT                  12

/* IPI0_INTR2 :: CPU_CLEAR :: SPARE_BIT_11 [11:11] */
#define BCHP_IPI0_INTR2_CPU_CLEAR_SPARE_BIT_11_MASK                0x00000800
#define BCHP_IPI0_INTR2_CPU_CLEAR_SPARE_BIT_11_SHIFT               11

/* IPI0_INTR2 :: CPU_CLEAR :: SPARE_BIT_10 [10:10] */
#define BCHP_IPI0_INTR2_CPU_CLEAR_SPARE_BIT_10_MASK                0x00000400
#define BCHP_IPI0_INTR2_CPU_CLEAR_SPARE_BIT_10_SHIFT               10

/* IPI0_INTR2 :: CPU_CLEAR :: SPARE_BIT_9 [09:09] */
#define BCHP_IPI0_INTR2_CPU_CLEAR_SPARE_BIT_9_MASK                 0x00000200
#define BCHP_IPI0_INTR2_CPU_CLEAR_SPARE_BIT_9_SHIFT                9

/* IPI0_INTR2 :: CPU_CLEAR :: SPARE_BIT_8 [08:08] */
#define BCHP_IPI0_INTR2_CPU_CLEAR_SPARE_BIT_8_MASK                 0x00000100
#define BCHP_IPI0_INTR2_CPU_CLEAR_SPARE_BIT_8_SHIFT                8

/* IPI0_INTR2 :: CPU_CLEAR :: MOCA [07:07] */
#define BCHP_IPI0_INTR2_CPU_CLEAR_MOCA_MASK                        0x00000080
#define BCHP_IPI0_INTR2_CPU_CLEAR_MOCA_SHIFT                       7

/* IPI0_INTR2 :: CPU_CLEAR :: AVD1 [06:06] */
#define BCHP_IPI0_INTR2_CPU_CLEAR_AVD1_MASK                        0x00000040
#define BCHP_IPI0_INTR2_CPU_CLEAR_AVD1_SHIFT                       6

/* IPI0_INTR2 :: CPU_CLEAR :: AVD0 [05:05] */
#define BCHP_IPI0_INTR2_CPU_CLEAR_AVD0_MASK                        0x00000020
#define BCHP_IPI0_INTR2_CPU_CLEAR_AVD0_SHIFT                       5

/* IPI0_INTR2 :: CPU_CLEAR :: RPDT [04:04] */
#define BCHP_IPI0_INTR2_CPU_CLEAR_RPDT_MASK                        0x00000010
#define BCHP_IPI0_INTR2_CPU_CLEAR_RPDT_SHIFT                       4

/* IPI0_INTR2 :: CPU_CLEAR :: SSP [03:03] */
#define BCHP_IPI0_INTR2_CPU_CLEAR_SSP_MASK                         0x00000008
#define BCHP_IPI0_INTR2_CPU_CLEAR_SSP_SHIFT                        3

/* IPI0_INTR2 :: CPU_CLEAR :: PCI [02:02] */
#define BCHP_IPI0_INTR2_CPU_CLEAR_PCI_MASK                         0x00000004
#define BCHP_IPI0_INTR2_CPU_CLEAR_PCI_SHIFT                        2

/* IPI0_INTR2 :: CPU_CLEAR :: TP1 [01:01] */
#define BCHP_IPI0_INTR2_CPU_CLEAR_TP1_MASK                         0x00000002
#define BCHP_IPI0_INTR2_CPU_CLEAR_TP1_SHIFT                        1

/* IPI0_INTR2 :: CPU_CLEAR :: TP0 [00:00] */
#define BCHP_IPI0_INTR2_CPU_CLEAR_TP0_MASK                         0x00000001
#define BCHP_IPI0_INTR2_CPU_CLEAR_TP0_SHIFT                        0

/***************************************************************************
 *CPU_MASK_STATUS - CPU interrupt Mask Status Register
 ***************************************************************************/
/* IPI0_INTR2 :: CPU_MASK_STATUS :: reserved0 [31:12] */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_reserved0_MASK             0xfffff000
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_reserved0_SHIFT            12

/* IPI0_INTR2 :: CPU_MASK_STATUS :: SPARE_BIT_11 [11:11] */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_SPARE_BIT_11_MASK          0x00000800
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_SPARE_BIT_11_SHIFT         11

/* IPI0_INTR2 :: CPU_MASK_STATUS :: SPARE_BIT_10 [10:10] */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_SPARE_BIT_10_MASK          0x00000400
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_SPARE_BIT_10_SHIFT         10

/* IPI0_INTR2 :: CPU_MASK_STATUS :: SPARE_BIT_9 [09:09] */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_SPARE_BIT_9_MASK           0x00000200
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_SPARE_BIT_9_SHIFT          9

/* IPI0_INTR2 :: CPU_MASK_STATUS :: SPARE_BIT_8 [08:08] */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_SPARE_BIT_8_MASK           0x00000100
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_SPARE_BIT_8_SHIFT          8

/* IPI0_INTR2 :: CPU_MASK_STATUS :: MoCA [07:07] */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_MoCA_MASK                  0x00000080
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_MoCA_SHIFT                 7

/* IPI0_INTR2 :: CPU_MASK_STATUS :: AVD1 [06:06] */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_AVD1_MASK                  0x00000040
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_AVD1_SHIFT                 6

/* IPI0_INTR2 :: CPU_MASK_STATUS :: AVD0 [05:05] */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_AVD0_MASK                  0x00000020
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_AVD0_SHIFT                 5

/* IPI0_INTR2 :: CPU_MASK_STATUS :: RPDT [04:04] */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_RPDT_MASK                  0x00000010
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_RPDT_SHIFT                 4

/* IPI0_INTR2 :: CPU_MASK_STATUS :: SSP [03:03] */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_SSP_MASK                   0x00000008
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_SSP_SHIFT                  3

/* IPI0_INTR2 :: CPU_MASK_STATUS :: PCI [02:02] */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_PCI_MASK                   0x00000004
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_PCI_SHIFT                  2

/* IPI0_INTR2 :: CPU_MASK_STATUS :: TP1 [01:01] */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_TP1_MASK                   0x00000002
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_TP1_SHIFT                  1

/* IPI0_INTR2 :: CPU_MASK_STATUS :: TP0 [00:00] */
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_TP0_MASK                   0x00000001
#define BCHP_IPI0_INTR2_CPU_MASK_STATUS_TP0_SHIFT                  0

/***************************************************************************
 *CPU_MASK_SET - CPU interrupt Mask Set Register
 ***************************************************************************/
/* IPI0_INTR2 :: CPU_MASK_SET :: reserved0 [31:12] */
#define BCHP_IPI0_INTR2_CPU_MASK_SET_reserved0_MASK                0xfffff000
#define BCHP_IPI0_INTR2_CPU_MASK_SET_reserved0_SHIFT               12

/* IPI0_INTR2 :: CPU_MASK_SET :: SPARE_BIT_11 [11:11] */
#define BCHP_IPI0_INTR2_CPU_MASK_SET_SPARE_BIT_11_MASK             0x00000800
#define BCHP_IPI0_INTR2_CPU_MASK_SET_SPARE_BIT_11_SHIFT            11

/* IPI0_INTR2 :: CPU_MASK_SET :: SPARE_BIT_10 [10:10] */
#define BCHP_IPI0_INTR2_CPU_MASK_SET_SPARE_BIT_10_MASK             0x00000400
#define BCHP_IPI0_INTR2_CPU_MASK_SET_SPARE_BIT_10_SHIFT            10

/* IPI0_INTR2 :: CPU_MASK_SET :: SPARE_BIT_9 [09:09] */
#define BCHP_IPI0_INTR2_CPU_MASK_SET_SPARE_BIT_9_MASK              0x00000200
#define BCHP_IPI0_INTR2_CPU_MASK_SET_SPARE_BIT_9_SHIFT             9

/* IPI0_INTR2 :: CPU_MASK_SET :: SPARE_BIT_8 [08:08] */
#define BCHP_IPI0_INTR2_CPU_MASK_SET_SPARE_BIT_8_MASK              0x00000100
#define BCHP_IPI0_INTR2_CPU_MASK_SET_SPARE_BIT_8_SHIFT             8

/* IPI0_INTR2 :: CPU_MASK_SET :: MoCA [07:07] */
#define BCHP_IPI0_INTR2_CPU_MASK_SET_MoCA_MASK                     0x00000080
#define BCHP_IPI0_INTR2_CPU_MASK_SET_MoCA_SHIFT                    7

/* IPI0_INTR2 :: CPU_MASK_SET :: AVD1 [06:06] */
#define BCHP_IPI0_INTR2_CPU_MASK_SET_AVD1_MASK                     0x00000040
#define BCHP_IPI0_INTR2_CPU_MASK_SET_AVD1_SHIFT                    6

/* IPI0_INTR2 :: CPU_MASK_SET :: AVD0 [05:05] */
#define BCHP_IPI0_INTR2_CPU_MASK_SET_AVD0_MASK                     0x00000020
#define BCHP_IPI0_INTR2_CPU_MASK_SET_AVD0_SHIFT                    5

/* IPI0_INTR2 :: CPU_MASK_SET :: RPDT [04:04] */
#define BCHP_IPI0_INTR2_CPU_MASK_SET_RPDT_MASK                     0x00000010
#define BCHP_IPI0_INTR2_CPU_MASK_SET_RPDT_SHIFT                    4

/* IPI0_INTR2 :: CPU_MASK_SET :: SSP [03:03] */
#define BCHP_IPI0_INTR2_CPU_MASK_SET_SSP_MASK                      0x00000008
#define BCHP_IPI0_INTR2_CPU_MASK_SET_SSP_SHIFT                     3

/* IPI0_INTR2 :: CPU_MASK_SET :: PCI [02:02] */
#define BCHP_IPI0_INTR2_CPU_MASK_SET_PCI_MASK                      0x00000004
#define BCHP_IPI0_INTR2_CPU_MASK_SET_PCI_SHIFT                     2

/* IPI0_INTR2 :: CPU_MASK_SET :: TP1 [01:01] */
#define BCHP_IPI0_INTR2_CPU_MASK_SET_TP1_MASK                      0x00000002
#define BCHP_IPI0_INTR2_CPU_MASK_SET_TP1_SHIFT                     1

/* IPI0_INTR2 :: CPU_MASK_SET :: TP0 [00:00] */
#define BCHP_IPI0_INTR2_CPU_MASK_SET_TP0_MASK                      0x00000001
#define BCHP_IPI0_INTR2_CPU_MASK_SET_TP0_SHIFT                     0

/***************************************************************************
 *CPU_MASK_CLEAR - CPU interrupt Mask Clear Register
 ***************************************************************************/
/* IPI0_INTR2 :: CPU_MASK_CLEAR :: reserved0 [31:12] */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_reserved0_MASK              0xfffff000
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_reserved0_SHIFT             12

/* IPI0_INTR2 :: CPU_MASK_CLEAR :: SPARE_BIT_11 [11:11] */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_SPARE_BIT_11_MASK           0x00000800
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_SPARE_BIT_11_SHIFT          11

/* IPI0_INTR2 :: CPU_MASK_CLEAR :: SPARE_BIT_10 [10:10] */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_SPARE_BIT_10_MASK           0x00000400
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_SPARE_BIT_10_SHIFT          10

/* IPI0_INTR2 :: CPU_MASK_CLEAR :: SPARE_BIT_9 [09:09] */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_SPARE_BIT_9_MASK            0x00000200
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_SPARE_BIT_9_SHIFT           9

/* IPI0_INTR2 :: CPU_MASK_CLEAR :: SPARE_BIT_8 [08:08] */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_SPARE_BIT_8_MASK            0x00000100
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_SPARE_BIT_8_SHIFT           8

/* IPI0_INTR2 :: CPU_MASK_CLEAR :: MoCA [07:07] */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_MoCA_MASK                   0x00000080
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_MoCA_SHIFT                  7

/* IPI0_INTR2 :: CPU_MASK_CLEAR :: AVD1 [06:06] */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_AVD1_MASK                   0x00000040
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_AVD1_SHIFT                  6

/* IPI0_INTR2 :: CPU_MASK_CLEAR :: AVD0 [05:05] */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_AVD0_MASK                   0x00000020
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_AVD0_SHIFT                  5

/* IPI0_INTR2 :: CPU_MASK_CLEAR :: RPDT [04:04] */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_RPDT_MASK                   0x00000010
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_RPDT_SHIFT                  4

/* IPI0_INTR2 :: CPU_MASK_CLEAR :: SSP [03:03] */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_SSP_MASK                    0x00000008
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_SSP_SHIFT                   3

/* IPI0_INTR2 :: CPU_MASK_CLEAR :: PCI [02:02] */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_PCI_MASK                    0x00000004
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_PCI_SHIFT                   2

/* IPI0_INTR2 :: CPU_MASK_CLEAR :: TP1 [01:01] */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_TP1_MASK                    0x00000002
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_TP1_SHIFT                   1

/* IPI0_INTR2 :: CPU_MASK_CLEAR :: TP0 [00:00] */
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_TP0_MASK                    0x00000001
#define BCHP_IPI0_INTR2_CPU_MASK_CLEAR_TP0_SHIFT                   0

/***************************************************************************
 *PCI_STATUS - PCI interrupt Status Register
 ***************************************************************************/
/* IPI0_INTR2 :: PCI_STATUS :: reserved0 [31:12] */
#define BCHP_IPI0_INTR2_PCI_STATUS_reserved0_MASK                  0xfffff000
#define BCHP_IPI0_INTR2_PCI_STATUS_reserved0_SHIFT                 12

/* IPI0_INTR2 :: PCI_STATUS :: SPARE_BIT_11 [11:11] */
#define BCHP_IPI0_INTR2_PCI_STATUS_SPARE_BIT_11_MASK               0x00000800
#define BCHP_IPI0_INTR2_PCI_STATUS_SPARE_BIT_11_SHIFT              11

/* IPI0_INTR2 :: PCI_STATUS :: SPARE_BIT_10 [10:10] */
#define BCHP_IPI0_INTR2_PCI_STATUS_SPARE_BIT_10_MASK               0x00000400
#define BCHP_IPI0_INTR2_PCI_STATUS_SPARE_BIT_10_SHIFT              10

/* IPI0_INTR2 :: PCI_STATUS :: SPARE_BIT_9 [09:09] */
#define BCHP_IPI0_INTR2_PCI_STATUS_SPARE_BIT_9_MASK                0x00000200
#define BCHP_IPI0_INTR2_PCI_STATUS_SPARE_BIT_9_SHIFT               9

/* IPI0_INTR2 :: PCI_STATUS :: SPARE_BIT_8 [08:08] */
#define BCHP_IPI0_INTR2_PCI_STATUS_SPARE_BIT_8_MASK                0x00000100
#define BCHP_IPI0_INTR2_PCI_STATUS_SPARE_BIT_8_SHIFT               8

/* IPI0_INTR2 :: PCI_STATUS :: MOCA [07:07] */
#define BCHP_IPI0_INTR2_PCI_STATUS_MOCA_MASK                       0x00000080
#define BCHP_IPI0_INTR2_PCI_STATUS_MOCA_SHIFT                      7

/* IPI0_INTR2 :: PCI_STATUS :: AVD1 [06:06] */
#define BCHP_IPI0_INTR2_PCI_STATUS_AVD1_MASK                       0x00000040
#define BCHP_IPI0_INTR2_PCI_STATUS_AVD1_SHIFT                      6

/* IPI0_INTR2 :: PCI_STATUS :: AVD0 [05:05] */
#define BCHP_IPI0_INTR2_PCI_STATUS_AVD0_MASK                       0x00000020
#define BCHP_IPI0_INTR2_PCI_STATUS_AVD0_SHIFT                      5

/* IPI0_INTR2 :: PCI_STATUS :: RPDT [04:04] */
#define BCHP_IPI0_INTR2_PCI_STATUS_RPDT_MASK                       0x00000010
#define BCHP_IPI0_INTR2_PCI_STATUS_RPDT_SHIFT                      4

/* IPI0_INTR2 :: PCI_STATUS :: SSP [03:03] */
#define BCHP_IPI0_INTR2_PCI_STATUS_SSP_MASK                        0x00000008
#define BCHP_IPI0_INTR2_PCI_STATUS_SSP_SHIFT                       3

/* IPI0_INTR2 :: PCI_STATUS :: PCI [02:02] */
#define BCHP_IPI0_INTR2_PCI_STATUS_PCI_MASK                        0x00000004
#define BCHP_IPI0_INTR2_PCI_STATUS_PCI_SHIFT                       2

/* IPI0_INTR2 :: PCI_STATUS :: TP1 [01:01] */
#define BCHP_IPI0_INTR2_PCI_STATUS_TP1_MASK                        0x00000002
#define BCHP_IPI0_INTR2_PCI_STATUS_TP1_SHIFT                       1

/* IPI0_INTR2 :: PCI_STATUS :: TP0 [00:00] */
#define BCHP_IPI0_INTR2_PCI_STATUS_TP0_MASK                        0x00000001
#define BCHP_IPI0_INTR2_PCI_STATUS_TP0_SHIFT                       0

/***************************************************************************
 *PCI_SET - PCI interrupt Set Register
 ***************************************************************************/
/* IPI0_INTR2 :: PCI_SET :: reserved0 [31:12] */
#define BCHP_IPI0_INTR2_PCI_SET_reserved0_MASK                     0xfffff000
#define BCHP_IPI0_INTR2_PCI_SET_reserved0_SHIFT                    12

/* IPI0_INTR2 :: PCI_SET :: SPARE_BIT_11 [11:11] */
#define BCHP_IPI0_INTR2_PCI_SET_SPARE_BIT_11_MASK                  0x00000800
#define BCHP_IPI0_INTR2_PCI_SET_SPARE_BIT_11_SHIFT                 11

/* IPI0_INTR2 :: PCI_SET :: SPARE_BIT_10 [10:10] */
#define BCHP_IPI0_INTR2_PCI_SET_SPARE_BIT_10_MASK                  0x00000400
#define BCHP_IPI0_INTR2_PCI_SET_SPARE_BIT_10_SHIFT                 10

/* IPI0_INTR2 :: PCI_SET :: SPARE_BIT_9 [09:09] */
#define BCHP_IPI0_INTR2_PCI_SET_SPARE_BIT_9_MASK                   0x00000200
#define BCHP_IPI0_INTR2_PCI_SET_SPARE_BIT_9_SHIFT                  9

/* IPI0_INTR2 :: PCI_SET :: SPARE_BIT_8 [08:08] */
#define BCHP_IPI0_INTR2_PCI_SET_SPARE_BIT_8_MASK                   0x00000100
#define BCHP_IPI0_INTR2_PCI_SET_SPARE_BIT_8_SHIFT                  8

/* IPI0_INTR2 :: PCI_SET :: MOCA [07:07] */
#define BCHP_IPI0_INTR2_PCI_SET_MOCA_MASK                          0x00000080
#define BCHP_IPI0_INTR2_PCI_SET_MOCA_SHIFT                         7

/* IPI0_INTR2 :: PCI_SET :: AVD1 [06:06] */
#define BCHP_IPI0_INTR2_PCI_SET_AVD1_MASK                          0x00000040
#define BCHP_IPI0_INTR2_PCI_SET_AVD1_SHIFT                         6

/* IPI0_INTR2 :: PCI_SET :: AVD0 [05:05] */
#define BCHP_IPI0_INTR2_PCI_SET_AVD0_MASK                          0x00000020
#define BCHP_IPI0_INTR2_PCI_SET_AVD0_SHIFT                         5

/* IPI0_INTR2 :: PCI_SET :: RPDT [04:04] */
#define BCHP_IPI0_INTR2_PCI_SET_RPDT_MASK                          0x00000010
#define BCHP_IPI0_INTR2_PCI_SET_RPDT_SHIFT                         4

/* IPI0_INTR2 :: PCI_SET :: SSP [03:03] */
#define BCHP_IPI0_INTR2_PCI_SET_SSP_MASK                           0x00000008
#define BCHP_IPI0_INTR2_PCI_SET_SSP_SHIFT                          3

/* IPI0_INTR2 :: PCI_SET :: PCI [02:02] */
#define BCHP_IPI0_INTR2_PCI_SET_PCI_MASK                           0x00000004
#define BCHP_IPI0_INTR2_PCI_SET_PCI_SHIFT                          2

/* IPI0_INTR2 :: PCI_SET :: TP1 [01:01] */
#define BCHP_IPI0_INTR2_PCI_SET_TP1_MASK                           0x00000002
#define BCHP_IPI0_INTR2_PCI_SET_TP1_SHIFT                          1

/* IPI0_INTR2 :: PCI_SET :: TP0 [00:00] */
#define BCHP_IPI0_INTR2_PCI_SET_TP0_MASK                           0x00000001
#define BCHP_IPI0_INTR2_PCI_SET_TP0_SHIFT                          0

/***************************************************************************
 *PCI_CLEAR - PCI interrupt Clear Register
 ***************************************************************************/
/* IPI0_INTR2 :: PCI_CLEAR :: reserved0 [31:12] */
#define BCHP_IPI0_INTR2_PCI_CLEAR_reserved0_MASK                   0xfffff000
#define BCHP_IPI0_INTR2_PCI_CLEAR_reserved0_SHIFT                  12

/* IPI0_INTR2 :: PCI_CLEAR :: SPARE_BIT_11 [11:11] */
#define BCHP_IPI0_INTR2_PCI_CLEAR_SPARE_BIT_11_MASK                0x00000800
#define BCHP_IPI0_INTR2_PCI_CLEAR_SPARE_BIT_11_SHIFT               11

/* IPI0_INTR2 :: PCI_CLEAR :: SPARE_BIT_10 [10:10] */
#define BCHP_IPI0_INTR2_PCI_CLEAR_SPARE_BIT_10_MASK                0x00000400
#define BCHP_IPI0_INTR2_PCI_CLEAR_SPARE_BIT_10_SHIFT               10

/* IPI0_INTR2 :: PCI_CLEAR :: SPARE_BIT_9 [09:09] */
#define BCHP_IPI0_INTR2_PCI_CLEAR_SPARE_BIT_9_MASK                 0x00000200
#define BCHP_IPI0_INTR2_PCI_CLEAR_SPARE_BIT_9_SHIFT                9

/* IPI0_INTR2 :: PCI_CLEAR :: SPARE_BIT_8 [08:08] */
#define BCHP_IPI0_INTR2_PCI_CLEAR_SPARE_BIT_8_MASK                 0x00000100
#define BCHP_IPI0_INTR2_PCI_CLEAR_SPARE_BIT_8_SHIFT                8

/* IPI0_INTR2 :: PCI_CLEAR :: MOCA [07:07] */
#define BCHP_IPI0_INTR2_PCI_CLEAR_MOCA_MASK                        0x00000080
#define BCHP_IPI0_INTR2_PCI_CLEAR_MOCA_SHIFT                       7

/* IPI0_INTR2 :: PCI_CLEAR :: AVD1 [06:06] */
#define BCHP_IPI0_INTR2_PCI_CLEAR_AVD1_MASK                        0x00000040
#define BCHP_IPI0_INTR2_PCI_CLEAR_AVD1_SHIFT                       6

/* IPI0_INTR2 :: PCI_CLEAR :: AVD0 [05:05] */
#define BCHP_IPI0_INTR2_PCI_CLEAR_AVD0_MASK                        0x00000020
#define BCHP_IPI0_INTR2_PCI_CLEAR_AVD0_SHIFT                       5

/* IPI0_INTR2 :: PCI_CLEAR :: RPDT [04:04] */
#define BCHP_IPI0_INTR2_PCI_CLEAR_RPDT_MASK                        0x00000010
#define BCHP_IPI0_INTR2_PCI_CLEAR_RPDT_SHIFT                       4

/* IPI0_INTR2 :: PCI_CLEAR :: SSP [03:03] */
#define BCHP_IPI0_INTR2_PCI_CLEAR_SSP_MASK                         0x00000008
#define BCHP_IPI0_INTR2_PCI_CLEAR_SSP_SHIFT                        3

/* IPI0_INTR2 :: PCI_CLEAR :: PCI [02:02] */
#define BCHP_IPI0_INTR2_PCI_CLEAR_PCI_MASK                         0x00000004
#define BCHP_IPI0_INTR2_PCI_CLEAR_PCI_SHIFT                        2

/* IPI0_INTR2 :: PCI_CLEAR :: TP1 [01:01] */
#define BCHP_IPI0_INTR2_PCI_CLEAR_TP1_MASK                         0x00000002
#define BCHP_IPI0_INTR2_PCI_CLEAR_TP1_SHIFT                        1

/* IPI0_INTR2 :: PCI_CLEAR :: TP0 [00:00] */
#define BCHP_IPI0_INTR2_PCI_CLEAR_TP0_MASK                         0x00000001
#define BCHP_IPI0_INTR2_PCI_CLEAR_TP0_SHIFT                        0

/***************************************************************************
 *PCI_MASK_STATUS - PCI interrupt Mask Status Register
 ***************************************************************************/
/* IPI0_INTR2 :: PCI_MASK_STATUS :: reserved0 [31:12] */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_reserved0_MASK             0xfffff000
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_reserved0_SHIFT            12

/* IPI0_INTR2 :: PCI_MASK_STATUS :: SPARE_BIT_11 [11:11] */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_SPARE_BIT_11_MASK          0x00000800
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_SPARE_BIT_11_SHIFT         11

/* IPI0_INTR2 :: PCI_MASK_STATUS :: SPARE_BIT_10 [10:10] */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_SPARE_BIT_10_MASK          0x00000400
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_SPARE_BIT_10_SHIFT         10

/* IPI0_INTR2 :: PCI_MASK_STATUS :: SPARE_BIT_9 [09:09] */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_SPARE_BIT_9_MASK           0x00000200
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_SPARE_BIT_9_SHIFT          9

/* IPI0_INTR2 :: PCI_MASK_STATUS :: SPARE_BIT_8 [08:08] */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_SPARE_BIT_8_MASK           0x00000100
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_SPARE_BIT_8_SHIFT          8

/* IPI0_INTR2 :: PCI_MASK_STATUS :: MoCA [07:07] */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_MoCA_MASK                  0x00000080
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_MoCA_SHIFT                 7

/* IPI0_INTR2 :: PCI_MASK_STATUS :: AVD1 [06:06] */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_AVD1_MASK                  0x00000040
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_AVD1_SHIFT                 6

/* IPI0_INTR2 :: PCI_MASK_STATUS :: AVD0 [05:05] */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_AVD0_MASK                  0x00000020
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_AVD0_SHIFT                 5

/* IPI0_INTR2 :: PCI_MASK_STATUS :: RPDT [04:04] */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_RPDT_MASK                  0x00000010
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_RPDT_SHIFT                 4

/* IPI0_INTR2 :: PCI_MASK_STATUS :: SSP [03:03] */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_SSP_MASK                   0x00000008
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_SSP_SHIFT                  3

/* IPI0_INTR2 :: PCI_MASK_STATUS :: PCI [02:02] */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_PCI_MASK                   0x00000004
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_PCI_SHIFT                  2

/* IPI0_INTR2 :: PCI_MASK_STATUS :: TP1 [01:01] */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_TP1_MASK                   0x00000002
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_TP1_SHIFT                  1

/* IPI0_INTR2 :: PCI_MASK_STATUS :: TP0 [00:00] */
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_TP0_MASK                   0x00000001
#define BCHP_IPI0_INTR2_PCI_MASK_STATUS_TP0_SHIFT                  0

/***************************************************************************
 *PCI_MASK_SET - PCI interrupt Mask Set Register
 ***************************************************************************/
/* IPI0_INTR2 :: PCI_MASK_SET :: reserved0 [31:12] */
#define BCHP_IPI0_INTR2_PCI_MASK_SET_reserved0_MASK                0xfffff000
#define BCHP_IPI0_INTR2_PCI_MASK_SET_reserved0_SHIFT               12

/* IPI0_INTR2 :: PCI_MASK_SET :: SPARE_BIT_11 [11:11] */
#define BCHP_IPI0_INTR2_PCI_MASK_SET_SPARE_BIT_11_MASK             0x00000800
#define BCHP_IPI0_INTR2_PCI_MASK_SET_SPARE_BIT_11_SHIFT            11

/* IPI0_INTR2 :: PCI_MASK_SET :: SPARE_BIT_10 [10:10] */
#define BCHP_IPI0_INTR2_PCI_MASK_SET_SPARE_BIT_10_MASK             0x00000400
#define BCHP_IPI0_INTR2_PCI_MASK_SET_SPARE_BIT_10_SHIFT            10

/* IPI0_INTR2 :: PCI_MASK_SET :: SPARE_BIT_9 [09:09] */
#define BCHP_IPI0_INTR2_PCI_MASK_SET_SPARE_BIT_9_MASK              0x00000200
#define BCHP_IPI0_INTR2_PCI_MASK_SET_SPARE_BIT_9_SHIFT             9

/* IPI0_INTR2 :: PCI_MASK_SET :: SPARE_BIT_8 [08:08] */
#define BCHP_IPI0_INTR2_PCI_MASK_SET_SPARE_BIT_8_MASK              0x00000100
#define BCHP_IPI0_INTR2_PCI_MASK_SET_SPARE_BIT_8_SHIFT             8

/* IPI0_INTR2 :: PCI_MASK_SET :: MoCA [07:07] */
#define BCHP_IPI0_INTR2_PCI_MASK_SET_MoCA_MASK                     0x00000080
#define BCHP_IPI0_INTR2_PCI_MASK_SET_MoCA_SHIFT                    7

/* IPI0_INTR2 :: PCI_MASK_SET :: AVD1 [06:06] */
#define BCHP_IPI0_INTR2_PCI_MASK_SET_AVD1_MASK                     0x00000040
#define BCHP_IPI0_INTR2_PCI_MASK_SET_AVD1_SHIFT                    6

/* IPI0_INTR2 :: PCI_MASK_SET :: AVD0 [05:05] */
#define BCHP_IPI0_INTR2_PCI_MASK_SET_AVD0_MASK                     0x00000020
#define BCHP_IPI0_INTR2_PCI_MASK_SET_AVD0_SHIFT                    5

/* IPI0_INTR2 :: PCI_MASK_SET :: RPDT [04:04] */
#define BCHP_IPI0_INTR2_PCI_MASK_SET_RPDT_MASK                     0x00000010
#define BCHP_IPI0_INTR2_PCI_MASK_SET_RPDT_SHIFT                    4

/* IPI0_INTR2 :: PCI_MASK_SET :: SSP [03:03] */
#define BCHP_IPI0_INTR2_PCI_MASK_SET_SSP_MASK                      0x00000008
#define BCHP_IPI0_INTR2_PCI_MASK_SET_SSP_SHIFT                     3

/* IPI0_INTR2 :: PCI_MASK_SET :: PCI [02:02] */
#define BCHP_IPI0_INTR2_PCI_MASK_SET_PCI_MASK                      0x00000004
#define BCHP_IPI0_INTR2_PCI_MASK_SET_PCI_SHIFT                     2

/* IPI0_INTR2 :: PCI_MASK_SET :: TP1 [01:01] */
#define BCHP_IPI0_INTR2_PCI_MASK_SET_TP1_MASK                      0x00000002
#define BCHP_IPI0_INTR2_PCI_MASK_SET_TP1_SHIFT                     1

/* IPI0_INTR2 :: PCI_MASK_SET :: TP0 [00:00] */
#define BCHP_IPI0_INTR2_PCI_MASK_SET_TP0_MASK                      0x00000001
#define BCHP_IPI0_INTR2_PCI_MASK_SET_TP0_SHIFT                     0

/***************************************************************************
 *PCI_MASK_CLEAR - PCI interrupt Mask Clear Register
 ***************************************************************************/
/* IPI0_INTR2 :: PCI_MASK_CLEAR :: reserved0 [31:12] */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_reserved0_MASK              0xfffff000
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_reserved0_SHIFT             12

/* IPI0_INTR2 :: PCI_MASK_CLEAR :: SPARE_BIT_11 [11:11] */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_SPARE_BIT_11_MASK           0x00000800
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_SPARE_BIT_11_SHIFT          11

/* IPI0_INTR2 :: PCI_MASK_CLEAR :: SPARE_BIT_10 [10:10] */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_SPARE_BIT_10_MASK           0x00000400
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_SPARE_BIT_10_SHIFT          10

/* IPI0_INTR2 :: PCI_MASK_CLEAR :: SPARE_BIT_9 [09:09] */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_SPARE_BIT_9_MASK            0x00000200
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_SPARE_BIT_9_SHIFT           9

/* IPI0_INTR2 :: PCI_MASK_CLEAR :: SPARE_BIT_8 [08:08] */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_SPARE_BIT_8_MASK            0x00000100
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_SPARE_BIT_8_SHIFT           8

/* IPI0_INTR2 :: PCI_MASK_CLEAR :: MoCA [07:07] */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_MoCA_MASK                   0x00000080
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_MoCA_SHIFT                  7

/* IPI0_INTR2 :: PCI_MASK_CLEAR :: AVD1 [06:06] */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_AVD1_MASK                   0x00000040
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_AVD1_SHIFT                  6

/* IPI0_INTR2 :: PCI_MASK_CLEAR :: AVD0 [05:05] */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_AVD0_MASK                   0x00000020
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_AVD0_SHIFT                  5

/* IPI0_INTR2 :: PCI_MASK_CLEAR :: RPDT [04:04] */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_RPDT_MASK                   0x00000010
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_RPDT_SHIFT                  4

/* IPI0_INTR2 :: PCI_MASK_CLEAR :: SSP [03:03] */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_SSP_MASK                    0x00000008
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_SSP_SHIFT                   3

/* IPI0_INTR2 :: PCI_MASK_CLEAR :: PCI [02:02] */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_PCI_MASK                    0x00000004
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_PCI_SHIFT                   2

/* IPI0_INTR2 :: PCI_MASK_CLEAR :: TP1 [01:01] */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_TP1_MASK                    0x00000002
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_TP1_SHIFT                   1

/* IPI0_INTR2 :: PCI_MASK_CLEAR :: TP0 [00:00] */
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_TP0_MASK                    0x00000001
#define BCHP_IPI0_INTR2_PCI_MASK_CLEAR_TP0_SHIFT                   0

#endif /* #ifndef BCHP_IPI0_INTR2_H__ */

/* End of File */
