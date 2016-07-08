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
 * Date:           Generated on               Mon Aug 24 11:29:34 2015
 *                 Full Compile MD5 Checksum  cecd4eac458fcdc4b77c82d0630f17be
 *                     (minus title and desc)
 *                 MD5 Checksum               c9a18191e1cdbfad4487ef21d91e95fc
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     126
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_UPG_BSC_IRQ_H__
#define BCHP_UPG_BSC_IRQ_H__

/***************************************************************************
 *UPG_BSC_IRQ - UPG BSC Level 2 Interrupt Enable/Status
 ***************************************************************************/
#define BCHP_UPG_BSC_IRQ_CPU_STATUS              0x2040a640 /* [RO] CPU interrupt Status Register */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_STATUS         0x2040a644 /* [RO] CPU interrupt Mask Status Register */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_SET            0x2040a648 /* [WO] CPU interrupt Mask Set Register */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_CLEAR          0x2040a64c /* [WO] CPU interrupt Mask Clear Register */
#define BCHP_UPG_BSC_IRQ_PCI_STATUS              0x2040a650 /* [RO] PCI interrupt Status Register */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_STATUS         0x2040a654 /* [RO] PCI interrupt Mask Status Register */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_SET            0x2040a658 /* [WO] PCI interrupt Mask Set Register */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_CLEAR          0x2040a65c /* [WO] PCI interrupt Mask Clear Register */

/***************************************************************************
 *CPU_STATUS - CPU interrupt Status Register
 ***************************************************************************/
/* UPG_BSC_IRQ :: CPU_STATUS :: reserved0 [31:03] */
#define BCHP_UPG_BSC_IRQ_CPU_STATUS_reserved0_MASK                 0xfffffff8
#define BCHP_UPG_BSC_IRQ_CPU_STATUS_reserved0_SHIFT                3

/* UPG_BSC_IRQ :: CPU_STATUS :: spare_00 [02:02] */
#define BCHP_UPG_BSC_IRQ_CPU_STATUS_spare_00_MASK                  0x00000004
#define BCHP_UPG_BSC_IRQ_CPU_STATUS_spare_00_SHIFT                 2
#define BCHP_UPG_BSC_IRQ_CPU_STATUS_spare_00_DEFAULT               0x00000000

/* UPG_BSC_IRQ :: CPU_STATUS :: iice [01:01] */
#define BCHP_UPG_BSC_IRQ_CPU_STATUS_iice_MASK                      0x00000002
#define BCHP_UPG_BSC_IRQ_CPU_STATUS_iice_SHIFT                     1
#define BCHP_UPG_BSC_IRQ_CPU_STATUS_iice_DEFAULT                   0x00000000

/* UPG_BSC_IRQ :: CPU_STATUS :: iica [00:00] */
#define BCHP_UPG_BSC_IRQ_CPU_STATUS_iica_MASK                      0x00000001
#define BCHP_UPG_BSC_IRQ_CPU_STATUS_iica_SHIFT                     0
#define BCHP_UPG_BSC_IRQ_CPU_STATUS_iica_DEFAULT                   0x00000000

/***************************************************************************
 *CPU_MASK_STATUS - CPU interrupt Mask Status Register
 ***************************************************************************/
/* UPG_BSC_IRQ :: CPU_MASK_STATUS :: reserved0 [31:03] */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_STATUS_reserved0_MASK            0xfffffff8
#define BCHP_UPG_BSC_IRQ_CPU_MASK_STATUS_reserved0_SHIFT           3

/* UPG_BSC_IRQ :: CPU_MASK_STATUS :: spare_00 [02:02] */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_STATUS_spare_00_MASK             0x00000004
#define BCHP_UPG_BSC_IRQ_CPU_MASK_STATUS_spare_00_SHIFT            2
#define BCHP_UPG_BSC_IRQ_CPU_MASK_STATUS_spare_00_DEFAULT          0x00000001

/* UPG_BSC_IRQ :: CPU_MASK_STATUS :: iice [01:01] */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_STATUS_iice_MASK                 0x00000002
#define BCHP_UPG_BSC_IRQ_CPU_MASK_STATUS_iice_SHIFT                1
#define BCHP_UPG_BSC_IRQ_CPU_MASK_STATUS_iice_DEFAULT              0x00000001

/* UPG_BSC_IRQ :: CPU_MASK_STATUS :: iica [00:00] */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_STATUS_iica_MASK                 0x00000001
#define BCHP_UPG_BSC_IRQ_CPU_MASK_STATUS_iica_SHIFT                0
#define BCHP_UPG_BSC_IRQ_CPU_MASK_STATUS_iica_DEFAULT              0x00000001

/***************************************************************************
 *CPU_MASK_SET - CPU interrupt Mask Set Register
 ***************************************************************************/
/* UPG_BSC_IRQ :: CPU_MASK_SET :: reserved0 [31:03] */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_SET_reserved0_MASK               0xfffffff8
#define BCHP_UPG_BSC_IRQ_CPU_MASK_SET_reserved0_SHIFT              3

/* UPG_BSC_IRQ :: CPU_MASK_SET :: spare_00 [02:02] */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_SET_spare_00_MASK                0x00000004
#define BCHP_UPG_BSC_IRQ_CPU_MASK_SET_spare_00_SHIFT               2
#define BCHP_UPG_BSC_IRQ_CPU_MASK_SET_spare_00_DEFAULT             0x00000001

/* UPG_BSC_IRQ :: CPU_MASK_SET :: iice [01:01] */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_SET_iice_MASK                    0x00000002
#define BCHP_UPG_BSC_IRQ_CPU_MASK_SET_iice_SHIFT                   1
#define BCHP_UPG_BSC_IRQ_CPU_MASK_SET_iice_DEFAULT                 0x00000001

/* UPG_BSC_IRQ :: CPU_MASK_SET :: iica [00:00] */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_SET_iica_MASK                    0x00000001
#define BCHP_UPG_BSC_IRQ_CPU_MASK_SET_iica_SHIFT                   0
#define BCHP_UPG_BSC_IRQ_CPU_MASK_SET_iica_DEFAULT                 0x00000001

/***************************************************************************
 *CPU_MASK_CLEAR - CPU interrupt Mask Clear Register
 ***************************************************************************/
/* UPG_BSC_IRQ :: CPU_MASK_CLEAR :: reserved0 [31:03] */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_CLEAR_reserved0_MASK             0xfffffff8
#define BCHP_UPG_BSC_IRQ_CPU_MASK_CLEAR_reserved0_SHIFT            3

/* UPG_BSC_IRQ :: CPU_MASK_CLEAR :: spare_00 [02:02] */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_CLEAR_spare_00_MASK              0x00000004
#define BCHP_UPG_BSC_IRQ_CPU_MASK_CLEAR_spare_00_SHIFT             2
#define BCHP_UPG_BSC_IRQ_CPU_MASK_CLEAR_spare_00_DEFAULT           0x00000001

/* UPG_BSC_IRQ :: CPU_MASK_CLEAR :: iice [01:01] */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_CLEAR_iice_MASK                  0x00000002
#define BCHP_UPG_BSC_IRQ_CPU_MASK_CLEAR_iice_SHIFT                 1
#define BCHP_UPG_BSC_IRQ_CPU_MASK_CLEAR_iice_DEFAULT               0x00000001

/* UPG_BSC_IRQ :: CPU_MASK_CLEAR :: iica [00:00] */
#define BCHP_UPG_BSC_IRQ_CPU_MASK_CLEAR_iica_MASK                  0x00000001
#define BCHP_UPG_BSC_IRQ_CPU_MASK_CLEAR_iica_SHIFT                 0
#define BCHP_UPG_BSC_IRQ_CPU_MASK_CLEAR_iica_DEFAULT               0x00000001

/***************************************************************************
 *PCI_STATUS - PCI interrupt Status Register
 ***************************************************************************/
/* UPG_BSC_IRQ :: PCI_STATUS :: reserved0 [31:03] */
#define BCHP_UPG_BSC_IRQ_PCI_STATUS_reserved0_MASK                 0xfffffff8
#define BCHP_UPG_BSC_IRQ_PCI_STATUS_reserved0_SHIFT                3

/* UPG_BSC_IRQ :: PCI_STATUS :: spare_00 [02:02] */
#define BCHP_UPG_BSC_IRQ_PCI_STATUS_spare_00_MASK                  0x00000004
#define BCHP_UPG_BSC_IRQ_PCI_STATUS_spare_00_SHIFT                 2
#define BCHP_UPG_BSC_IRQ_PCI_STATUS_spare_00_DEFAULT               0x00000000

/* UPG_BSC_IRQ :: PCI_STATUS :: iice [01:01] */
#define BCHP_UPG_BSC_IRQ_PCI_STATUS_iice_MASK                      0x00000002
#define BCHP_UPG_BSC_IRQ_PCI_STATUS_iice_SHIFT                     1
#define BCHP_UPG_BSC_IRQ_PCI_STATUS_iice_DEFAULT                   0x00000000

/* UPG_BSC_IRQ :: PCI_STATUS :: iica [00:00] */
#define BCHP_UPG_BSC_IRQ_PCI_STATUS_iica_MASK                      0x00000001
#define BCHP_UPG_BSC_IRQ_PCI_STATUS_iica_SHIFT                     0
#define BCHP_UPG_BSC_IRQ_PCI_STATUS_iica_DEFAULT                   0x00000000

/***************************************************************************
 *PCI_MASK_STATUS - PCI interrupt Mask Status Register
 ***************************************************************************/
/* UPG_BSC_IRQ :: PCI_MASK_STATUS :: reserved0 [31:03] */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_STATUS_reserved0_MASK            0xfffffff8
#define BCHP_UPG_BSC_IRQ_PCI_MASK_STATUS_reserved0_SHIFT           3

/* UPG_BSC_IRQ :: PCI_MASK_STATUS :: spare_00 [02:02] */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_STATUS_spare_00_MASK             0x00000004
#define BCHP_UPG_BSC_IRQ_PCI_MASK_STATUS_spare_00_SHIFT            2
#define BCHP_UPG_BSC_IRQ_PCI_MASK_STATUS_spare_00_DEFAULT          0x00000001

/* UPG_BSC_IRQ :: PCI_MASK_STATUS :: iice [01:01] */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_STATUS_iice_MASK                 0x00000002
#define BCHP_UPG_BSC_IRQ_PCI_MASK_STATUS_iice_SHIFT                1
#define BCHP_UPG_BSC_IRQ_PCI_MASK_STATUS_iice_DEFAULT              0x00000001

/* UPG_BSC_IRQ :: PCI_MASK_STATUS :: iica [00:00] */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_STATUS_iica_MASK                 0x00000001
#define BCHP_UPG_BSC_IRQ_PCI_MASK_STATUS_iica_SHIFT                0
#define BCHP_UPG_BSC_IRQ_PCI_MASK_STATUS_iica_DEFAULT              0x00000001

/***************************************************************************
 *PCI_MASK_SET - PCI interrupt Mask Set Register
 ***************************************************************************/
/* UPG_BSC_IRQ :: PCI_MASK_SET :: reserved0 [31:03] */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_SET_reserved0_MASK               0xfffffff8
#define BCHP_UPG_BSC_IRQ_PCI_MASK_SET_reserved0_SHIFT              3

/* UPG_BSC_IRQ :: PCI_MASK_SET :: spare_00 [02:02] */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_SET_spare_00_MASK                0x00000004
#define BCHP_UPG_BSC_IRQ_PCI_MASK_SET_spare_00_SHIFT               2
#define BCHP_UPG_BSC_IRQ_PCI_MASK_SET_spare_00_DEFAULT             0x00000001

/* UPG_BSC_IRQ :: PCI_MASK_SET :: iice [01:01] */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_SET_iice_MASK                    0x00000002
#define BCHP_UPG_BSC_IRQ_PCI_MASK_SET_iice_SHIFT                   1
#define BCHP_UPG_BSC_IRQ_PCI_MASK_SET_iice_DEFAULT                 0x00000001

/* UPG_BSC_IRQ :: PCI_MASK_SET :: iica [00:00] */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_SET_iica_MASK                    0x00000001
#define BCHP_UPG_BSC_IRQ_PCI_MASK_SET_iica_SHIFT                   0
#define BCHP_UPG_BSC_IRQ_PCI_MASK_SET_iica_DEFAULT                 0x00000001

/***************************************************************************
 *PCI_MASK_CLEAR - PCI interrupt Mask Clear Register
 ***************************************************************************/
/* UPG_BSC_IRQ :: PCI_MASK_CLEAR :: reserved0 [31:03] */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_CLEAR_reserved0_MASK             0xfffffff8
#define BCHP_UPG_BSC_IRQ_PCI_MASK_CLEAR_reserved0_SHIFT            3

/* UPG_BSC_IRQ :: PCI_MASK_CLEAR :: spare_00 [02:02] */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_CLEAR_spare_00_MASK              0x00000004
#define BCHP_UPG_BSC_IRQ_PCI_MASK_CLEAR_spare_00_SHIFT             2
#define BCHP_UPG_BSC_IRQ_PCI_MASK_CLEAR_spare_00_DEFAULT           0x00000001

/* UPG_BSC_IRQ :: PCI_MASK_CLEAR :: iice [01:01] */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_CLEAR_iice_MASK                  0x00000002
#define BCHP_UPG_BSC_IRQ_PCI_MASK_CLEAR_iice_SHIFT                 1
#define BCHP_UPG_BSC_IRQ_PCI_MASK_CLEAR_iice_DEFAULT               0x00000001

/* UPG_BSC_IRQ :: PCI_MASK_CLEAR :: iica [00:00] */
#define BCHP_UPG_BSC_IRQ_PCI_MASK_CLEAR_iica_MASK                  0x00000001
#define BCHP_UPG_BSC_IRQ_PCI_MASK_CLEAR_iica_SHIFT                 0
#define BCHP_UPG_BSC_IRQ_PCI_MASK_CLEAR_iica_DEFAULT               0x00000001

#endif /* #ifndef BCHP_UPG_BSC_IRQ_H__ */

/* End of File */
