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
 * Date:           Generated on               Thu Jun 18 10:52:58 2015
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

#ifndef BCHP_CPU_IPI_INTR2_H__
#define BCHP_CPU_IPI_INTR2_H__

/***************************************************************************
 *CPU_IPI_INTR2 - Host to SCPU Inter-Processor Level 2 Interrupt Controller Registers
 ***************************************************************************/
#define BCHP_CPU_IPI_INTR2_CPU_STATUS            0x20311000 /* [RO] CPU interrupt Status Register */
#define BCHP_CPU_IPI_INTR2_CPU_SET               0x20311004 /* [WO] CPU interrupt Set Register */
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR             0x20311008 /* [WO] CPU interrupt Clear Register */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS       0x2031100c /* [RO] CPU interrupt Mask Status Register */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET          0x20311010 /* [WO] CPU interrupt Mask Set Register */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR        0x20311014 /* [WO] CPU interrupt Mask Clear Register */
#define BCHP_CPU_IPI_INTR2_PCI_STATUS            0x20311018 /* [RO] PCI interrupt Status Register */
#define BCHP_CPU_IPI_INTR2_PCI_SET               0x2031101c /* [WO] PCI interrupt Set Register */
#define BCHP_CPU_IPI_INTR2_PCI_CLEAR             0x20311020 /* [WO] PCI interrupt Clear Register */
#define BCHP_CPU_IPI_INTR2_PCI_MASK_STATUS       0x20311024 /* [RO] PCI interrupt Mask Status Register */
#define BCHP_CPU_IPI_INTR2_PCI_MASK_SET          0x20311028 /* [WO] PCI interrupt Mask Set Register */
#define BCHP_CPU_IPI_INTR2_PCI_MASK_CLEAR        0x2031102c /* [WO] PCI interrupt Mask Clear Register */

/***************************************************************************
 *CPU_STATUS - CPU interrupt Status Register
 ***************************************************************************/
/* CPU_IPI_INTR2 :: CPU_STATUS :: SPARE_IPI [31:28] */
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_SPARE_IPI_MASK               0xf0000000
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_SPARE_IPI_SHIFT              28
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_SPARE_IPI_DEFAULT            0x00000000

/* CPU_IPI_INTR2 :: CPU_STATUS :: PCI [27:24] */
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_PCI_MASK                     0x0f000000
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_PCI_SHIFT                    24
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_PCI_DEFAULT                  0x00000000

/* CPU_IPI_INTR2 :: CPU_STATUS :: RAAGA [23:20] */
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_RAAGA_MASK                   0x00f00000
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_RAAGA_SHIFT                  20
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_RAAGA_DEFAULT                0x00000000

/* CPU_IPI_INTR2 :: CPU_STATUS :: SSP [19:16] */
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_SSP_MASK                     0x000f0000
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_SSP_SHIFT                    16
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_SSP_DEFAULT                  0x00000000

/* CPU_IPI_INTR2 :: CPU_STATUS :: VDEC1 [15:12] */
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_VDEC1_MASK                   0x0000f000
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_VDEC1_SHIFT                  12

/* CPU_IPI_INTR2 :: CPU_STATUS :: VDEC0 [11:08] */
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_VDEC0_MASK                   0x00000f00
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_VDEC0_SHIFT                  8
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_VDEC0_DEFAULT                0x00000000

/* CPU_IPI_INTR2 :: CPU_STATUS :: SPARE_HOST_IPI [07:02] */
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_SPARE_HOST_IPI_MASK          0x000000fc
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_SPARE_HOST_IPI_SHIFT         2
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_SPARE_HOST_IPI_DEFAULT       0x00000000

/* CPU_IPI_INTR2 :: CPU_STATUS :: HOST_SCPU_ILOAD [01:01] */
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_HOST_SCPU_ILOAD_MASK         0x00000002
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_HOST_SCPU_ILOAD_SHIFT        1
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_HOST_SCPU_ILOAD_DEFAULT      0x00000000

/* CPU_IPI_INTR2 :: CPU_STATUS :: HOST_SCPU_DDONE [00:00] */
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_HOST_SCPU_DDONE_MASK         0x00000001
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_HOST_SCPU_DDONE_SHIFT        0
#define BCHP_CPU_IPI_INTR2_CPU_STATUS_HOST_SCPU_DDONE_DEFAULT      0x00000000

/***************************************************************************
 *CPU_SET - CPU interrupt Set Register
 ***************************************************************************/
/* CPU_IPI_INTR2 :: CPU_SET :: SPARE_IPI [31:28] */
#define BCHP_CPU_IPI_INTR2_CPU_SET_SPARE_IPI_MASK                  0xf0000000
#define BCHP_CPU_IPI_INTR2_CPU_SET_SPARE_IPI_SHIFT                 28
#define BCHP_CPU_IPI_INTR2_CPU_SET_SPARE_IPI_DEFAULT               0x00000000

/* CPU_IPI_INTR2 :: CPU_SET :: PCI [27:24] */
#define BCHP_CPU_IPI_INTR2_CPU_SET_PCI_MASK                        0x0f000000
#define BCHP_CPU_IPI_INTR2_CPU_SET_PCI_SHIFT                       24
#define BCHP_CPU_IPI_INTR2_CPU_SET_PCI_DEFAULT                     0x00000000

/* CPU_IPI_INTR2 :: CPU_SET :: RAAGA [23:20] */
#define BCHP_CPU_IPI_INTR2_CPU_SET_RAAGA_MASK                      0x00f00000
#define BCHP_CPU_IPI_INTR2_CPU_SET_RAAGA_SHIFT                     20
#define BCHP_CPU_IPI_INTR2_CPU_SET_RAAGA_DEFAULT                   0x00000000

/* CPU_IPI_INTR2 :: CPU_SET :: SSP [19:16] */
#define BCHP_CPU_IPI_INTR2_CPU_SET_SSP_MASK                        0x000f0000
#define BCHP_CPU_IPI_INTR2_CPU_SET_SSP_SHIFT                       16
#define BCHP_CPU_IPI_INTR2_CPU_SET_SSP_DEFAULT                     0x00000000

/* CPU_IPI_INTR2 :: CPU_SET :: VDEC1 [15:12] */
#define BCHP_CPU_IPI_INTR2_CPU_SET_VDEC1_MASK                      0x0000f000
#define BCHP_CPU_IPI_INTR2_CPU_SET_VDEC1_SHIFT                     12

/* CPU_IPI_INTR2 :: CPU_SET :: VDEC0 [11:08] */
#define BCHP_CPU_IPI_INTR2_CPU_SET_VDEC0_MASK                      0x00000f00
#define BCHP_CPU_IPI_INTR2_CPU_SET_VDEC0_SHIFT                     8
#define BCHP_CPU_IPI_INTR2_CPU_SET_VDEC0_DEFAULT                   0x00000000

/* CPU_IPI_INTR2 :: CPU_SET :: SPARE_HOST_IPI [07:02] */
#define BCHP_CPU_IPI_INTR2_CPU_SET_SPARE_HOST_IPI_MASK             0x000000fc
#define BCHP_CPU_IPI_INTR2_CPU_SET_SPARE_HOST_IPI_SHIFT            2
#define BCHP_CPU_IPI_INTR2_CPU_SET_SPARE_HOST_IPI_DEFAULT          0x00000000

/* CPU_IPI_INTR2 :: CPU_SET :: HOST_SCPU_ILOAD [01:01] */
#define BCHP_CPU_IPI_INTR2_CPU_SET_HOST_SCPU_ILOAD_MASK            0x00000002
#define BCHP_CPU_IPI_INTR2_CPU_SET_HOST_SCPU_ILOAD_SHIFT           1
#define BCHP_CPU_IPI_INTR2_CPU_SET_HOST_SCPU_ILOAD_DEFAULT         0x00000000

/* CPU_IPI_INTR2 :: CPU_SET :: HOST_SCPU_DDONE [00:00] */
#define BCHP_CPU_IPI_INTR2_CPU_SET_HOST_SCPU_DDONE_MASK            0x00000001
#define BCHP_CPU_IPI_INTR2_CPU_SET_HOST_SCPU_DDONE_SHIFT           0
#define BCHP_CPU_IPI_INTR2_CPU_SET_HOST_SCPU_DDONE_DEFAULT         0x00000000

/***************************************************************************
 *CPU_CLEAR - CPU interrupt Clear Register
 ***************************************************************************/
/* CPU_IPI_INTR2 :: CPU_CLEAR :: SPARE_IPI [31:28] */
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_SPARE_IPI_MASK                0xf0000000
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_SPARE_IPI_SHIFT               28
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_SPARE_IPI_DEFAULT             0x00000000

/* CPU_IPI_INTR2 :: CPU_CLEAR :: PCI [27:24] */
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_PCI_MASK                      0x0f000000
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_PCI_SHIFT                     24
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_PCI_DEFAULT                   0x00000000

/* CPU_IPI_INTR2 :: CPU_CLEAR :: RAAGA [23:20] */
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_RAAGA_MASK                    0x00f00000
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_RAAGA_SHIFT                   20
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_RAAGA_DEFAULT                 0x00000000

/* CPU_IPI_INTR2 :: CPU_CLEAR :: SSP [19:16] */
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_SSP_MASK                      0x000f0000
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_SSP_SHIFT                     16
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_SSP_DEFAULT                   0x00000000

/* CPU_IPI_INTR2 :: CPU_CLEAR :: VDEC1 [15:12] */
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_VDEC1_MASK                    0x0000f000
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_VDEC1_SHIFT                   12

/* CPU_IPI_INTR2 :: CPU_CLEAR :: VDEC0 [11:08] */
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_VDEC0_MASK                    0x00000f00
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_VDEC0_SHIFT                   8
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_VDEC0_DEFAULT                 0x00000000

/* CPU_IPI_INTR2 :: CPU_CLEAR :: SPARE_HOST_IPI [07:02] */
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_SPARE_HOST_IPI_MASK           0x000000fc
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_SPARE_HOST_IPI_SHIFT          2
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_SPARE_HOST_IPI_DEFAULT        0x00000000

/* CPU_IPI_INTR2 :: CPU_CLEAR :: HOST_SCPU_ILOAD [01:01] */
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_HOST_SCPU_ILOAD_MASK          0x00000002
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_HOST_SCPU_ILOAD_SHIFT         1
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_HOST_SCPU_ILOAD_DEFAULT       0x00000000

/* CPU_IPI_INTR2 :: CPU_CLEAR :: HOST_SCPU_DDONE [00:00] */
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_HOST_SCPU_DDONE_MASK          0x00000001
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_HOST_SCPU_DDONE_SHIFT         0
#define BCHP_CPU_IPI_INTR2_CPU_CLEAR_HOST_SCPU_DDONE_DEFAULT       0x00000000

/***************************************************************************
 *CPU_MASK_STATUS - CPU interrupt Mask Status Register
 ***************************************************************************/
/* CPU_IPI_INTR2 :: CPU_MASK_STATUS :: SPARE_IPI [31:28] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_SPARE_IPI_MASK          0xf0000000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_SPARE_IPI_SHIFT         28
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_SPARE_IPI_DEFAULT       0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_STATUS :: PCI [27:24] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_PCI_MASK                0x0f000000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_PCI_SHIFT               24
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_PCI_DEFAULT             0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_STATUS :: RAAGA [23:20] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_RAAGA_MASK              0x00f00000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_RAAGA_SHIFT             20
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_RAAGA_DEFAULT           0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_STATUS :: SSP [19:16] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_SSP_MASK                0x000f0000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_SSP_SHIFT               16
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_SSP_DEFAULT             0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_STATUS :: VDEC1 [15:12] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_VDEC1_MASK              0x0000f000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_VDEC1_SHIFT             12
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_VDEC1_DEFAULT           0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_STATUS :: VDEC0 [11:08] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_VDEC0_MASK              0x00000f00
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_VDEC0_SHIFT             8
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_VDEC0_DEFAULT           0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_STATUS :: SPARE_HOST_IPI [07:02] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_SPARE_HOST_IPI_MASK     0x000000fc
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_SPARE_HOST_IPI_SHIFT    2
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_SPARE_HOST_IPI_DEFAULT  0x0000003f

/* CPU_IPI_INTR2 :: CPU_MASK_STATUS :: HOST_SCPU_ILOAD [01:01] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_HOST_SCPU_ILOAD_MASK    0x00000002
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_HOST_SCPU_ILOAD_SHIFT   1
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_HOST_SCPU_ILOAD_DEFAULT 0x00000001

/* CPU_IPI_INTR2 :: CPU_MASK_STATUS :: HOST_SCPU_DDONE [00:00] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_HOST_SCPU_DDONE_MASK    0x00000001
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_HOST_SCPU_DDONE_SHIFT   0
#define BCHP_CPU_IPI_INTR2_CPU_MASK_STATUS_HOST_SCPU_DDONE_DEFAULT 0x00000001

/***************************************************************************
 *CPU_MASK_SET - CPU interrupt Mask Set Register
 ***************************************************************************/
/* CPU_IPI_INTR2 :: CPU_MASK_SET :: SPARE_IPI [31:28] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_SPARE_IPI_MASK             0xf0000000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_SPARE_IPI_SHIFT            28
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_SPARE_IPI_DEFAULT          0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_SET :: PCI [27:24] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_PCI_MASK                   0x0f000000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_PCI_SHIFT                  24
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_PCI_DEFAULT                0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_SET :: RAAGA [23:20] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_RAAGA_MASK                 0x00f00000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_RAAGA_SHIFT                20
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_RAAGA_DEFAULT              0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_SET :: SSP [19:16] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_SSP_MASK                   0x000f0000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_SSP_SHIFT                  16
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_SSP_DEFAULT                0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_SET :: VDEC1 [15:12] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_VDEC1_MASK                 0x0000f000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_VDEC1_SHIFT                12
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_VDEC1_DEFAULT              0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_SET :: VDEC0 [11:08] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_VDEC0_MASK                 0x00000f00
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_VDEC0_SHIFT                8
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_VDEC0_DEFAULT              0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_SET :: SPARE_HOST_IPI [07:02] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_SPARE_HOST_IPI_MASK        0x000000fc
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_SPARE_HOST_IPI_SHIFT       2
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_SPARE_HOST_IPI_DEFAULT     0x0000003f

/* CPU_IPI_INTR2 :: CPU_MASK_SET :: HOST_SCPU_ILOAD [01:01] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_HOST_SCPU_ILOAD_MASK       0x00000002
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_HOST_SCPU_ILOAD_SHIFT      1
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_HOST_SCPU_ILOAD_DEFAULT    0x00000001

/* CPU_IPI_INTR2 :: CPU_MASK_SET :: HOST_SCPU_DDONE [00:00] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_HOST_SCPU_DDONE_MASK       0x00000001
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_HOST_SCPU_DDONE_SHIFT      0
#define BCHP_CPU_IPI_INTR2_CPU_MASK_SET_HOST_SCPU_DDONE_DEFAULT    0x00000001

/***************************************************************************
 *CPU_MASK_CLEAR - CPU interrupt Mask Clear Register
 ***************************************************************************/
/* CPU_IPI_INTR2 :: CPU_MASK_CLEAR :: SPARE_IPI [31:28] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_SPARE_IPI_MASK           0xf0000000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_SPARE_IPI_SHIFT          28
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_SPARE_IPI_DEFAULT        0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_CLEAR :: PCI [27:24] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_PCI_MASK                 0x0f000000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_PCI_SHIFT                24
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_PCI_DEFAULT              0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_CLEAR :: RAAGA [23:20] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_RAAGA_MASK               0x00f00000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_RAAGA_SHIFT              20
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_RAAGA_DEFAULT            0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_CLEAR :: SSP [19:16] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_SSP_MASK                 0x000f0000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_SSP_SHIFT                16
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_SSP_DEFAULT              0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_CLEAR :: VDEC1 [15:12] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_VDEC1_MASK               0x0000f000
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_VDEC1_SHIFT              12
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_VDEC1_DEFAULT            0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_CLEAR :: VDEC0 [11:08] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_VDEC0_MASK               0x00000f00
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_VDEC0_SHIFT              8
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_VDEC0_DEFAULT            0x0000000f

/* CPU_IPI_INTR2 :: CPU_MASK_CLEAR :: SPARE_HOST_IPI [07:02] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_SPARE_HOST_IPI_MASK      0x000000fc
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_SPARE_HOST_IPI_SHIFT     2
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_SPARE_HOST_IPI_DEFAULT   0x0000003f

/* CPU_IPI_INTR2 :: CPU_MASK_CLEAR :: HOST_SCPU_ILOAD [01:01] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_HOST_SCPU_ILOAD_MASK     0x00000002
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_HOST_SCPU_ILOAD_SHIFT    1
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_HOST_SCPU_ILOAD_DEFAULT  0x00000001

/* CPU_IPI_INTR2 :: CPU_MASK_CLEAR :: HOST_SCPU_DDONE [00:00] */
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_HOST_SCPU_DDONE_MASK     0x00000001
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_HOST_SCPU_DDONE_SHIFT    0
#define BCHP_CPU_IPI_INTR2_CPU_MASK_CLEAR_HOST_SCPU_DDONE_DEFAULT  0x00000001

/***************************************************************************
 *PCI_STATUS - PCI interrupt Status Register
 ***************************************************************************/
/* CPU_IPI_INTR2 :: PCI_STATUS :: UNUSED [31:00] */
#define BCHP_CPU_IPI_INTR2_PCI_STATUS_UNUSED_MASK                  0xffffffff
#define BCHP_CPU_IPI_INTR2_PCI_STATUS_UNUSED_SHIFT                 0
#define BCHP_CPU_IPI_INTR2_PCI_STATUS_UNUSED_DEFAULT               0x00000000

/***************************************************************************
 *PCI_SET - PCI interrupt Set Register
 ***************************************************************************/
/* CPU_IPI_INTR2 :: PCI_SET :: UNUSED [31:00] */
#define BCHP_CPU_IPI_INTR2_PCI_SET_UNUSED_MASK                     0xffffffff
#define BCHP_CPU_IPI_INTR2_PCI_SET_UNUSED_SHIFT                    0
#define BCHP_CPU_IPI_INTR2_PCI_SET_UNUSED_DEFAULT                  0x00000000

/***************************************************************************
 *PCI_CLEAR - PCI interrupt Clear Register
 ***************************************************************************/
/* CPU_IPI_INTR2 :: PCI_CLEAR :: UNUSED [31:00] */
#define BCHP_CPU_IPI_INTR2_PCI_CLEAR_UNUSED_MASK                   0xffffffff
#define BCHP_CPU_IPI_INTR2_PCI_CLEAR_UNUSED_SHIFT                  0
#define BCHP_CPU_IPI_INTR2_PCI_CLEAR_UNUSED_DEFAULT                0x00000000

/***************************************************************************
 *PCI_MASK_STATUS - PCI interrupt Mask Status Register
 ***************************************************************************/
/* CPU_IPI_INTR2 :: PCI_MASK_STATUS :: UNUSED [31:00] */
#define BCHP_CPU_IPI_INTR2_PCI_MASK_STATUS_UNUSED_MASK             0xffffffff
#define BCHP_CPU_IPI_INTR2_PCI_MASK_STATUS_UNUSED_SHIFT            0
#define BCHP_CPU_IPI_INTR2_PCI_MASK_STATUS_UNUSED_DEFAULT          0xffffffff

/***************************************************************************
 *PCI_MASK_SET - PCI interrupt Mask Set Register
 ***************************************************************************/
/* CPU_IPI_INTR2 :: PCI_MASK_SET :: UNUSED [31:00] */
#define BCHP_CPU_IPI_INTR2_PCI_MASK_SET_UNUSED_MASK                0xffffffff
#define BCHP_CPU_IPI_INTR2_PCI_MASK_SET_UNUSED_SHIFT               0
#define BCHP_CPU_IPI_INTR2_PCI_MASK_SET_UNUSED_DEFAULT             0xffffffff

/***************************************************************************
 *PCI_MASK_CLEAR - PCI interrupt Mask Clear Register
 ***************************************************************************/
/* CPU_IPI_INTR2 :: PCI_MASK_CLEAR :: UNUSED [31:00] */
#define BCHP_CPU_IPI_INTR2_PCI_MASK_CLEAR_UNUSED_MASK              0xffffffff
#define BCHP_CPU_IPI_INTR2_PCI_MASK_CLEAR_UNUSED_SHIFT             0
#define BCHP_CPU_IPI_INTR2_PCI_MASK_CLEAR_UNUSED_DEFAULT           0xffffffff

#endif /* #ifndef BCHP_CPU_IPI_INTR2_H__ */

/* End of File */
