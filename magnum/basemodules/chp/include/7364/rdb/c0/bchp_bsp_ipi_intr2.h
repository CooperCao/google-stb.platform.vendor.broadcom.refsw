/****************************************************************************
 *     Copyright (c) 1999-2016, Broadcom Corporation
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
 * Date:           Generated on               Mon Feb  8 12:53:14 2016
 *                 Full Compile MD5 Checksum  7c463a9180016920b3e03273285ff33d
 *                     (minus title and desc)
 *                 MD5 Checksum               30fed0099690880293569d98807ed1d8
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     749
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_BSP_IPI_INTR2_H__
#define BCHP_BSP_IPI_INTR2_H__

/***************************************************************************
 *BSP_IPI_INTR2 - BSP to SCPU Inter-Processor Level 2 Interrupt Controller Registers
 ***************************************************************************/
#define BCHP_BSP_IPI_INTR2_CPU_STATUS            0x00310500 /* [RO] CPU interrupt Status Register */
#define BCHP_BSP_IPI_INTR2_CPU_SET               0x00310504 /* [WO] CPU interrupt Set Register */
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR             0x00310508 /* [WO] CPU interrupt Clear Register */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS       0x0031050c /* [RO] CPU interrupt Mask Status Register */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET          0x00310510 /* [WO] CPU interrupt Mask Set Register */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR        0x00310514 /* [WO] CPU interrupt Mask Clear Register */
#define BCHP_BSP_IPI_INTR2_PCI_STATUS            0x00310518 /* [RO] PCI interrupt Status Register */
#define BCHP_BSP_IPI_INTR2_PCI_SET               0x0031051c /* [WO] PCI interrupt Set Register */
#define BCHP_BSP_IPI_INTR2_PCI_CLEAR             0x00310520 /* [WO] PCI interrupt Clear Register */
#define BCHP_BSP_IPI_INTR2_PCI_MASK_STATUS       0x00310524 /* [RO] PCI interrupt Mask Status Register */
#define BCHP_BSP_IPI_INTR2_PCI_MASK_SET          0x00310528 /* [WO] PCI interrupt Mask Set Register */
#define BCHP_BSP_IPI_INTR2_PCI_MASK_CLEAR        0x0031052c /* [WO] PCI interrupt Mask Clear Register */

/***************************************************************************
 *CPU_STATUS - CPU interrupt Status Register
 ***************************************************************************/
/* BSP_IPI_INTR2 :: CPU_STATUS :: BSP_SCPU_TIMING_VIOL [31:31] */
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_BSP_SCPU_TIMING_VIOL_MASK    0x80000000
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_BSP_SCPU_TIMING_VIOL_SHIFT   31
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_BSP_SCPU_TIMING_VIOL_DEFAULT 0x00000000

/* BSP_IPI_INTR2 :: CPU_STATUS :: BSP_ZZYZX_INTR [30:30] */
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_BSP_ZZYZX_INTR_MASK          0x40000000
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_BSP_ZZYZX_INTR_SHIFT         30
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_BSP_ZZYZX_INTR_DEFAULT       0x00000000

/* BSP_IPI_INTR2 :: CPU_STATUS :: SPARE_IPI [29:02] */
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_SPARE_IPI_MASK               0x3ffffffc
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_SPARE_IPI_SHIFT              2
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_SPARE_IPI_DEFAULT            0x00000000

/* BSP_IPI_INTR2 :: CPU_STATUS :: BSP_SCPU_OLOAD [01:01] */
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_BSP_SCPU_OLOAD_MASK          0x00000002
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_BSP_SCPU_OLOAD_SHIFT         1
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_BSP_SCPU_OLOAD_DEFAULT       0x00000000

/* BSP_IPI_INTR2 :: CPU_STATUS :: BSP_SCPU_DRDY [00:00] */
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_BSP_SCPU_DRDY_MASK           0x00000001
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_BSP_SCPU_DRDY_SHIFT          0
#define BCHP_BSP_IPI_INTR2_CPU_STATUS_BSP_SCPU_DRDY_DEFAULT        0x00000000

/***************************************************************************
 *CPU_SET - CPU interrupt Set Register
 ***************************************************************************/
/* BSP_IPI_INTR2 :: CPU_SET :: BSP_SCPU_TIMING_VIOL [31:31] */
#define BCHP_BSP_IPI_INTR2_CPU_SET_BSP_SCPU_TIMING_VIOL_MASK       0x80000000
#define BCHP_BSP_IPI_INTR2_CPU_SET_BSP_SCPU_TIMING_VIOL_SHIFT      31
#define BCHP_BSP_IPI_INTR2_CPU_SET_BSP_SCPU_TIMING_VIOL_DEFAULT    0x00000000

/* BSP_IPI_INTR2 :: CPU_SET :: BSP_ZZYZX_INTR [30:30] */
#define BCHP_BSP_IPI_INTR2_CPU_SET_BSP_ZZYZX_INTR_MASK             0x40000000
#define BCHP_BSP_IPI_INTR2_CPU_SET_BSP_ZZYZX_INTR_SHIFT            30
#define BCHP_BSP_IPI_INTR2_CPU_SET_BSP_ZZYZX_INTR_DEFAULT          0x00000000

/* BSP_IPI_INTR2 :: CPU_SET :: SPARE_IPI [29:02] */
#define BCHP_BSP_IPI_INTR2_CPU_SET_SPARE_IPI_MASK                  0x3ffffffc
#define BCHP_BSP_IPI_INTR2_CPU_SET_SPARE_IPI_SHIFT                 2
#define BCHP_BSP_IPI_INTR2_CPU_SET_SPARE_IPI_DEFAULT               0x00000000

/* BSP_IPI_INTR2 :: CPU_SET :: BSP_SCPU_OLOAD [01:01] */
#define BCHP_BSP_IPI_INTR2_CPU_SET_BSP_SCPU_OLOAD_MASK             0x00000002
#define BCHP_BSP_IPI_INTR2_CPU_SET_BSP_SCPU_OLOAD_SHIFT            1
#define BCHP_BSP_IPI_INTR2_CPU_SET_BSP_SCPU_OLOAD_DEFAULT          0x00000000

/* BSP_IPI_INTR2 :: CPU_SET :: BSP_SCPU_DRDY [00:00] */
#define BCHP_BSP_IPI_INTR2_CPU_SET_BSP_SCPU_DRDY_MASK              0x00000001
#define BCHP_BSP_IPI_INTR2_CPU_SET_BSP_SCPU_DRDY_SHIFT             0
#define BCHP_BSP_IPI_INTR2_CPU_SET_BSP_SCPU_DRDY_DEFAULT           0x00000000

/***************************************************************************
 *CPU_CLEAR - CPU interrupt Clear Register
 ***************************************************************************/
/* BSP_IPI_INTR2 :: CPU_CLEAR :: BSP_SCPU_TIMING_VIOL [31:31] */
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_BSP_SCPU_TIMING_VIOL_MASK     0x80000000
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_BSP_SCPU_TIMING_VIOL_SHIFT    31
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_BSP_SCPU_TIMING_VIOL_DEFAULT  0x00000000

/* BSP_IPI_INTR2 :: CPU_CLEAR :: BSP_ZZYZX_INTR [30:30] */
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_BSP_ZZYZX_INTR_MASK           0x40000000
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_BSP_ZZYZX_INTR_SHIFT          30
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_BSP_ZZYZX_INTR_DEFAULT        0x00000000

/* BSP_IPI_INTR2 :: CPU_CLEAR :: SPARE_IPI [29:02] */
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_SPARE_IPI_MASK                0x3ffffffc
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_SPARE_IPI_SHIFT               2
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_SPARE_IPI_DEFAULT             0x00000000

/* BSP_IPI_INTR2 :: CPU_CLEAR :: BSP_SCPU_OLOAD [01:01] */
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_BSP_SCPU_OLOAD_MASK           0x00000002
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_BSP_SCPU_OLOAD_SHIFT          1
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_BSP_SCPU_OLOAD_DEFAULT        0x00000000

/* BSP_IPI_INTR2 :: CPU_CLEAR :: BSP_SCPU_DRDY [00:00] */
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_BSP_SCPU_DRDY_MASK            0x00000001
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_BSP_SCPU_DRDY_SHIFT           0
#define BCHP_BSP_IPI_INTR2_CPU_CLEAR_BSP_SCPU_DRDY_DEFAULT         0x00000000

/***************************************************************************
 *CPU_MASK_STATUS - CPU interrupt Mask Status Register
 ***************************************************************************/
/* BSP_IPI_INTR2 :: CPU_MASK_STATUS :: BSP_SCPU_TIMING_VIOL [31:31] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_BSP_SCPU_TIMING_VIOL_MASK 0x80000000
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_BSP_SCPU_TIMING_VIOL_SHIFT 31
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_BSP_SCPU_TIMING_VIOL_DEFAULT 0x00000001

/* BSP_IPI_INTR2 :: CPU_MASK_STATUS :: BSP_ZZYZX_INTR [30:30] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_BSP_ZZYZX_INTR_MASK     0x40000000
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_BSP_ZZYZX_INTR_SHIFT    30
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_BSP_ZZYZX_INTR_DEFAULT  0x00000001

/* BSP_IPI_INTR2 :: CPU_MASK_STATUS :: SPARE_IPI [29:02] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_SPARE_IPI_MASK          0x3ffffffc
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_SPARE_IPI_SHIFT         2
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_SPARE_IPI_DEFAULT       0x0fffffff

/* BSP_IPI_INTR2 :: CPU_MASK_STATUS :: BSP_SCPU_OLOAD [01:01] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_BSP_SCPU_OLOAD_MASK     0x00000002
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_BSP_SCPU_OLOAD_SHIFT    1
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_BSP_SCPU_OLOAD_DEFAULT  0x00000001

/* BSP_IPI_INTR2 :: CPU_MASK_STATUS :: BSP_SCPU_DRDY [00:00] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_BSP_SCPU_DRDY_MASK      0x00000001
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_BSP_SCPU_DRDY_SHIFT     0
#define BCHP_BSP_IPI_INTR2_CPU_MASK_STATUS_BSP_SCPU_DRDY_DEFAULT   0x00000001

/***************************************************************************
 *CPU_MASK_SET - CPU interrupt Mask Set Register
 ***************************************************************************/
/* BSP_IPI_INTR2 :: CPU_MASK_SET :: BSP_SCPU_TIMING_VIOL [31:31] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_BSP_SCPU_TIMING_VIOL_MASK  0x80000000
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_BSP_SCPU_TIMING_VIOL_SHIFT 31
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_BSP_SCPU_TIMING_VIOL_DEFAULT 0x00000001

/* BSP_IPI_INTR2 :: CPU_MASK_SET :: BSP_ZZYZX_INTR [30:30] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_BSP_ZZYZX_INTR_MASK        0x40000000
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_BSP_ZZYZX_INTR_SHIFT       30
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_BSP_ZZYZX_INTR_DEFAULT     0x00000001

/* BSP_IPI_INTR2 :: CPU_MASK_SET :: SPARE_IPI [29:02] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_SPARE_IPI_MASK             0x3ffffffc
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_SPARE_IPI_SHIFT            2
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_SPARE_IPI_DEFAULT          0x0fffffff

/* BSP_IPI_INTR2 :: CPU_MASK_SET :: BSP_SCPU_OLOAD [01:01] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_BSP_SCPU_OLOAD_MASK        0x00000002
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_BSP_SCPU_OLOAD_SHIFT       1
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_BSP_SCPU_OLOAD_DEFAULT     0x00000001

/* BSP_IPI_INTR2 :: CPU_MASK_SET :: BSP_SCPU_DRDY [00:00] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_BSP_SCPU_DRDY_MASK         0x00000001
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_BSP_SCPU_DRDY_SHIFT        0
#define BCHP_BSP_IPI_INTR2_CPU_MASK_SET_BSP_SCPU_DRDY_DEFAULT      0x00000001

/***************************************************************************
 *CPU_MASK_CLEAR - CPU interrupt Mask Clear Register
 ***************************************************************************/
/* BSP_IPI_INTR2 :: CPU_MASK_CLEAR :: BSP_SCPU_TIMING_VIOL [31:31] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_BSP_SCPU_TIMING_VIOL_MASK 0x80000000
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_BSP_SCPU_TIMING_VIOL_SHIFT 31
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_BSP_SCPU_TIMING_VIOL_DEFAULT 0x00000001

/* BSP_IPI_INTR2 :: CPU_MASK_CLEAR :: BSP_ZZYZX_INTR [30:30] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_BSP_ZZYZX_INTR_MASK      0x40000000
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_BSP_ZZYZX_INTR_SHIFT     30
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_BSP_ZZYZX_INTR_DEFAULT   0x00000001

/* BSP_IPI_INTR2 :: CPU_MASK_CLEAR :: SPARE_IPI [29:02] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_SPARE_IPI_MASK           0x3ffffffc
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_SPARE_IPI_SHIFT          2
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_SPARE_IPI_DEFAULT        0x0fffffff

/* BSP_IPI_INTR2 :: CPU_MASK_CLEAR :: BSP_SCPU_OLOAD [01:01] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_BSP_SCPU_OLOAD_MASK      0x00000002
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_BSP_SCPU_OLOAD_SHIFT     1
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_BSP_SCPU_OLOAD_DEFAULT   0x00000001

/* BSP_IPI_INTR2 :: CPU_MASK_CLEAR :: BSP_SCPU_DRDY [00:00] */
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_BSP_SCPU_DRDY_MASK       0x00000001
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_BSP_SCPU_DRDY_SHIFT      0
#define BCHP_BSP_IPI_INTR2_CPU_MASK_CLEAR_BSP_SCPU_DRDY_DEFAULT    0x00000001

/***************************************************************************
 *PCI_STATUS - PCI interrupt Status Register
 ***************************************************************************/
/* BSP_IPI_INTR2 :: PCI_STATUS :: UNUSED [31:00] */
#define BCHP_BSP_IPI_INTR2_PCI_STATUS_UNUSED_MASK                  0xffffffff
#define BCHP_BSP_IPI_INTR2_PCI_STATUS_UNUSED_SHIFT                 0
#define BCHP_BSP_IPI_INTR2_PCI_STATUS_UNUSED_DEFAULT               0x00000000

/***************************************************************************
 *PCI_SET - PCI interrupt Set Register
 ***************************************************************************/
/* BSP_IPI_INTR2 :: PCI_SET :: UNUSED [31:00] */
#define BCHP_BSP_IPI_INTR2_PCI_SET_UNUSED_MASK                     0xffffffff
#define BCHP_BSP_IPI_INTR2_PCI_SET_UNUSED_SHIFT                    0
#define BCHP_BSP_IPI_INTR2_PCI_SET_UNUSED_DEFAULT                  0x00000000

/***************************************************************************
 *PCI_CLEAR - PCI interrupt Clear Register
 ***************************************************************************/
/* BSP_IPI_INTR2 :: PCI_CLEAR :: UNUSED [31:00] */
#define BCHP_BSP_IPI_INTR2_PCI_CLEAR_UNUSED_MASK                   0xffffffff
#define BCHP_BSP_IPI_INTR2_PCI_CLEAR_UNUSED_SHIFT                  0
#define BCHP_BSP_IPI_INTR2_PCI_CLEAR_UNUSED_DEFAULT                0x00000000

/***************************************************************************
 *PCI_MASK_STATUS - PCI interrupt Mask Status Register
 ***************************************************************************/
/* BSP_IPI_INTR2 :: PCI_MASK_STATUS :: UNUSED [31:00] */
#define BCHP_BSP_IPI_INTR2_PCI_MASK_STATUS_UNUSED_MASK             0xffffffff
#define BCHP_BSP_IPI_INTR2_PCI_MASK_STATUS_UNUSED_SHIFT            0
#define BCHP_BSP_IPI_INTR2_PCI_MASK_STATUS_UNUSED_DEFAULT          0xffffffff

/***************************************************************************
 *PCI_MASK_SET - PCI interrupt Mask Set Register
 ***************************************************************************/
/* BSP_IPI_INTR2 :: PCI_MASK_SET :: UNUSED [31:00] */
#define BCHP_BSP_IPI_INTR2_PCI_MASK_SET_UNUSED_MASK                0xffffffff
#define BCHP_BSP_IPI_INTR2_PCI_MASK_SET_UNUSED_SHIFT               0
#define BCHP_BSP_IPI_INTR2_PCI_MASK_SET_UNUSED_DEFAULT             0xffffffff

/***************************************************************************
 *PCI_MASK_CLEAR - PCI interrupt Mask Clear Register
 ***************************************************************************/
/* BSP_IPI_INTR2 :: PCI_MASK_CLEAR :: UNUSED [31:00] */
#define BCHP_BSP_IPI_INTR2_PCI_MASK_CLEAR_UNUSED_MASK              0xffffffff
#define BCHP_BSP_IPI_INTR2_PCI_MASK_CLEAR_UNUSED_SHIFT             0
#define BCHP_BSP_IPI_INTR2_PCI_MASK_CLEAR_UNUSED_DEFAULT           0xffffffff

#endif /* #ifndef BCHP_BSP_IPI_INTR2_H__ */

/* End of File */
