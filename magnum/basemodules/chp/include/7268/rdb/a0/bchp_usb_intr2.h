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

#ifndef BCHP_USB_INTR2_H__
#define BCHP_USB_INTR2_H__

/***************************************************************************
 *USB_INTR2 - USB Level 2 Interrupt Registers
 ***************************************************************************/
#define BCHP_USB_INTR2_CPU_STATUS                0x20b00180 /* [RO] CPU interrupt Status Register */
#define BCHP_USB_INTR2_CPU_SET                   0x20b00184 /* [WO] CPU interrupt Set Register */
#define BCHP_USB_INTR2_CPU_CLEAR                 0x20b00188 /* [WO] CPU interrupt Clear Register */
#define BCHP_USB_INTR2_CPU_MASK_STATUS           0x20b0018c /* [RO] CPU interrupt Mask Status Register */
#define BCHP_USB_INTR2_CPU_MASK_SET              0x20b00190 /* [WO] CPU interrupt Mask Set Register */
#define BCHP_USB_INTR2_CPU_MASK_CLEAR            0x20b00194 /* [WO] CPU interrupt Mask Clear Register */
#define BCHP_USB_INTR2_PCI_STATUS                0x20b00198 /* [RO] PCI interrupt Status Register */
#define BCHP_USB_INTR2_PCI_SET                   0x20b0019c /* [WO] PCI interrupt Set Register */
#define BCHP_USB_INTR2_PCI_CLEAR                 0x20b001a0 /* [WO] PCI interrupt Clear Register */
#define BCHP_USB_INTR2_PCI_MASK_STATUS           0x20b001a4 /* [RO] PCI interrupt Mask Status Register */
#define BCHP_USB_INTR2_PCI_MASK_SET              0x20b001a8 /* [WO] PCI interrupt Mask Set Register */
#define BCHP_USB_INTR2_PCI_MASK_CLEAR            0x20b001ac /* [WO] PCI interrupt Mask Clear Register */

/***************************************************************************
 *CPU_STATUS - CPU interrupt Status Register
 ***************************************************************************/
/* USB_INTR2 :: CPU_STATUS :: reserved0 [31:08] */
#define BCHP_USB_INTR2_CPU_STATUS_reserved0_MASK                   0xffffff00
#define BCHP_USB_INTR2_CPU_STATUS_reserved0_SHIFT                  8

/* USB_INTR2 :: CPU_STATUS :: USB_DRD_STATE_CHG_INTR [07:07] */
#define BCHP_USB_INTR2_CPU_STATUS_USB_DRD_STATE_CHG_INTR_MASK      0x00000080
#define BCHP_USB_INTR2_CPU_STATUS_USB_DRD_STATE_CHG_INTR_SHIFT     7
#define BCHP_USB_INTR2_CPU_STATUS_USB_DRD_STATE_CHG_INTR_DEFAULT   0x00000000

/* USB_INTR2 :: CPU_STATUS :: USB_BDC_HSE_INTR [06:06] */
#define BCHP_USB_INTR2_CPU_STATUS_USB_BDC_HSE_INTR_MASK            0x00000040
#define BCHP_USB_INTR2_CPU_STATUS_USB_BDC_HSE_INTR_SHIFT           6
#define BCHP_USB_INTR2_CPU_STATUS_USB_BDC_HSE_INTR_DEFAULT         0x00000000

/* USB_INTR2 :: CPU_STATUS :: USB_SCB_INVLD_ADDR_INTR [05:05] */
#define BCHP_USB_INTR2_CPU_STATUS_USB_SCB_INVLD_ADDR_INTR_MASK     0x00000020
#define BCHP_USB_INTR2_CPU_STATUS_USB_SCB_INVLD_ADDR_INTR_SHIFT    5
#define BCHP_USB_INTR2_CPU_STATUS_USB_SCB_INVLD_ADDR_INTR_DEFAULT  0x00000000

/* USB_INTR2 :: CPU_STATUS :: USB_XHC_HSE_INTR [04:04] */
#define BCHP_USB_INTR2_CPU_STATUS_USB_XHC_HSE_INTR_MASK            0x00000010
#define BCHP_USB_INTR2_CPU_STATUS_USB_XHC_HSE_INTR_SHIFT           4
#define BCHP_USB_INTR2_CPU_STATUS_USB_XHC_HSE_INTR_DEFAULT         0x00000000

/* USB_INTR2 :: CPU_STATUS :: USB_SOFT_SHUTDOWN_INTR [03:03] */
#define BCHP_USB_INTR2_CPU_STATUS_USB_SOFT_SHUTDOWN_INTR_MASK      0x00000008
#define BCHP_USB_INTR2_CPU_STATUS_USB_SOFT_SHUTDOWN_INTR_SHIFT     3
#define BCHP_USB_INTR2_CPU_STATUS_USB_SOFT_SHUTDOWN_INTR_DEFAULT   0x00000000

/* USB_INTR2 :: CPU_STATUS :: USB_S2_DISCON_INTR [02:02] */
#define BCHP_USB_INTR2_CPU_STATUS_USB_S2_DISCON_INTR_MASK          0x00000004
#define BCHP_USB_INTR2_CPU_STATUS_USB_S2_DISCON_INTR_SHIFT         2
#define BCHP_USB_INTR2_CPU_STATUS_USB_S2_DISCON_INTR_DEFAULT       0x00000000

/* USB_INTR2 :: CPU_STATUS :: USB_S2_CONNECT_INTR [01:01] */
#define BCHP_USB_INTR2_CPU_STATUS_USB_S2_CONNECT_INTR_MASK         0x00000002
#define BCHP_USB_INTR2_CPU_STATUS_USB_S2_CONNECT_INTR_SHIFT        1
#define BCHP_USB_INTR2_CPU_STATUS_USB_S2_CONNECT_INTR_DEFAULT      0x00000000

/* USB_INTR2 :: CPU_STATUS :: USB_GR_BRIDGE_INTR [00:00] */
#define BCHP_USB_INTR2_CPU_STATUS_USB_GR_BRIDGE_INTR_MASK          0x00000001
#define BCHP_USB_INTR2_CPU_STATUS_USB_GR_BRIDGE_INTR_SHIFT         0
#define BCHP_USB_INTR2_CPU_STATUS_USB_GR_BRIDGE_INTR_DEFAULT       0x00000000

/***************************************************************************
 *CPU_SET - CPU interrupt Set Register
 ***************************************************************************/
/* USB_INTR2 :: CPU_SET :: reserved0 [31:08] */
#define BCHP_USB_INTR2_CPU_SET_reserved0_MASK                      0xffffff00
#define BCHP_USB_INTR2_CPU_SET_reserved0_SHIFT                     8

/* USB_INTR2 :: CPU_SET :: USB_DRD_STATE_CHG_INTR [07:07] */
#define BCHP_USB_INTR2_CPU_SET_USB_DRD_STATE_CHG_INTR_MASK         0x00000080
#define BCHP_USB_INTR2_CPU_SET_USB_DRD_STATE_CHG_INTR_SHIFT        7
#define BCHP_USB_INTR2_CPU_SET_USB_DRD_STATE_CHG_INTR_DEFAULT      0x00000000

/* USB_INTR2 :: CPU_SET :: USB_BDC_HSE_INTR [06:06] */
#define BCHP_USB_INTR2_CPU_SET_USB_BDC_HSE_INTR_MASK               0x00000040
#define BCHP_USB_INTR2_CPU_SET_USB_BDC_HSE_INTR_SHIFT              6
#define BCHP_USB_INTR2_CPU_SET_USB_BDC_HSE_INTR_DEFAULT            0x00000000

/* USB_INTR2 :: CPU_SET :: USB_SCB_INVLD_ADDR_INTR [05:05] */
#define BCHP_USB_INTR2_CPU_SET_USB_SCB_INVLD_ADDR_INTR_MASK        0x00000020
#define BCHP_USB_INTR2_CPU_SET_USB_SCB_INVLD_ADDR_INTR_SHIFT       5
#define BCHP_USB_INTR2_CPU_SET_USB_SCB_INVLD_ADDR_INTR_DEFAULT     0x00000000

/* USB_INTR2 :: CPU_SET :: USB_XHC_HSE_INTR [04:04] */
#define BCHP_USB_INTR2_CPU_SET_USB_XHC_HSE_INTR_MASK               0x00000010
#define BCHP_USB_INTR2_CPU_SET_USB_XHC_HSE_INTR_SHIFT              4
#define BCHP_USB_INTR2_CPU_SET_USB_XHC_HSE_INTR_DEFAULT            0x00000000

/* USB_INTR2 :: CPU_SET :: USB_SOFT_SHUTDOWN_INTR [03:03] */
#define BCHP_USB_INTR2_CPU_SET_USB_SOFT_SHUTDOWN_INTR_MASK         0x00000008
#define BCHP_USB_INTR2_CPU_SET_USB_SOFT_SHUTDOWN_INTR_SHIFT        3
#define BCHP_USB_INTR2_CPU_SET_USB_SOFT_SHUTDOWN_INTR_DEFAULT      0x00000000

/* USB_INTR2 :: CPU_SET :: USB_S2_DISCON_INTR [02:02] */
#define BCHP_USB_INTR2_CPU_SET_USB_S2_DISCON_INTR_MASK             0x00000004
#define BCHP_USB_INTR2_CPU_SET_USB_S2_DISCON_INTR_SHIFT            2
#define BCHP_USB_INTR2_CPU_SET_USB_S2_DISCON_INTR_DEFAULT          0x00000000

/* USB_INTR2 :: CPU_SET :: USB_S2_CONNECT_INTR [01:01] */
#define BCHP_USB_INTR2_CPU_SET_USB_S2_CONNECT_INTR_MASK            0x00000002
#define BCHP_USB_INTR2_CPU_SET_USB_S2_CONNECT_INTR_SHIFT           1
#define BCHP_USB_INTR2_CPU_SET_USB_S2_CONNECT_INTR_DEFAULT         0x00000000

/* USB_INTR2 :: CPU_SET :: USB_GR_BRIDGE_INTR [00:00] */
#define BCHP_USB_INTR2_CPU_SET_USB_GR_BRIDGE_INTR_MASK             0x00000001
#define BCHP_USB_INTR2_CPU_SET_USB_GR_BRIDGE_INTR_SHIFT            0
#define BCHP_USB_INTR2_CPU_SET_USB_GR_BRIDGE_INTR_DEFAULT          0x00000000

/***************************************************************************
 *CPU_CLEAR - CPU interrupt Clear Register
 ***************************************************************************/
/* USB_INTR2 :: CPU_CLEAR :: reserved0 [31:08] */
#define BCHP_USB_INTR2_CPU_CLEAR_reserved0_MASK                    0xffffff00
#define BCHP_USB_INTR2_CPU_CLEAR_reserved0_SHIFT                   8

/* USB_INTR2 :: CPU_CLEAR :: USB_DRD_STATE_CHG_INTR [07:07] */
#define BCHP_USB_INTR2_CPU_CLEAR_USB_DRD_STATE_CHG_INTR_MASK       0x00000080
#define BCHP_USB_INTR2_CPU_CLEAR_USB_DRD_STATE_CHG_INTR_SHIFT      7
#define BCHP_USB_INTR2_CPU_CLEAR_USB_DRD_STATE_CHG_INTR_DEFAULT    0x00000000

/* USB_INTR2 :: CPU_CLEAR :: USB_BDC_HSE_INTR [06:06] */
#define BCHP_USB_INTR2_CPU_CLEAR_USB_BDC_HSE_INTR_MASK             0x00000040
#define BCHP_USB_INTR2_CPU_CLEAR_USB_BDC_HSE_INTR_SHIFT            6
#define BCHP_USB_INTR2_CPU_CLEAR_USB_BDC_HSE_INTR_DEFAULT          0x00000000

/* USB_INTR2 :: CPU_CLEAR :: USB_SCB_INVLD_ADDR_INTR [05:05] */
#define BCHP_USB_INTR2_CPU_CLEAR_USB_SCB_INVLD_ADDR_INTR_MASK      0x00000020
#define BCHP_USB_INTR2_CPU_CLEAR_USB_SCB_INVLD_ADDR_INTR_SHIFT     5
#define BCHP_USB_INTR2_CPU_CLEAR_USB_SCB_INVLD_ADDR_INTR_DEFAULT   0x00000000

/* USB_INTR2 :: CPU_CLEAR :: USB_XHC_HSE_INTR [04:04] */
#define BCHP_USB_INTR2_CPU_CLEAR_USB_XHC_HSE_INTR_MASK             0x00000010
#define BCHP_USB_INTR2_CPU_CLEAR_USB_XHC_HSE_INTR_SHIFT            4
#define BCHP_USB_INTR2_CPU_CLEAR_USB_XHC_HSE_INTR_DEFAULT          0x00000000

/* USB_INTR2 :: CPU_CLEAR :: USB_SOFT_SHUTDOWN_INTR [03:03] */
#define BCHP_USB_INTR2_CPU_CLEAR_USB_SOFT_SHUTDOWN_INTR_MASK       0x00000008
#define BCHP_USB_INTR2_CPU_CLEAR_USB_SOFT_SHUTDOWN_INTR_SHIFT      3
#define BCHP_USB_INTR2_CPU_CLEAR_USB_SOFT_SHUTDOWN_INTR_DEFAULT    0x00000000

/* USB_INTR2 :: CPU_CLEAR :: USB_S2_DISCON_INTR [02:02] */
#define BCHP_USB_INTR2_CPU_CLEAR_USB_S2_DISCON_INTR_MASK           0x00000004
#define BCHP_USB_INTR2_CPU_CLEAR_USB_S2_DISCON_INTR_SHIFT          2
#define BCHP_USB_INTR2_CPU_CLEAR_USB_S2_DISCON_INTR_DEFAULT        0x00000000

/* USB_INTR2 :: CPU_CLEAR :: USB_S2_CONNECT_INTR [01:01] */
#define BCHP_USB_INTR2_CPU_CLEAR_USB_S2_CONNECT_INTR_MASK          0x00000002
#define BCHP_USB_INTR2_CPU_CLEAR_USB_S2_CONNECT_INTR_SHIFT         1
#define BCHP_USB_INTR2_CPU_CLEAR_USB_S2_CONNECT_INTR_DEFAULT       0x00000000

/* USB_INTR2 :: CPU_CLEAR :: USB_GR_BRIDGE_INTR [00:00] */
#define BCHP_USB_INTR2_CPU_CLEAR_USB_GR_BRIDGE_INTR_MASK           0x00000001
#define BCHP_USB_INTR2_CPU_CLEAR_USB_GR_BRIDGE_INTR_SHIFT          0
#define BCHP_USB_INTR2_CPU_CLEAR_USB_GR_BRIDGE_INTR_DEFAULT        0x00000000

/***************************************************************************
 *CPU_MASK_STATUS - CPU interrupt Mask Status Register
 ***************************************************************************/
/* USB_INTR2 :: CPU_MASK_STATUS :: reserved0 [31:08] */
#define BCHP_USB_INTR2_CPU_MASK_STATUS_reserved0_MASK              0xffffff00
#define BCHP_USB_INTR2_CPU_MASK_STATUS_reserved0_SHIFT             8

/* USB_INTR2 :: CPU_MASK_STATUS :: USB_DRD_STATE_CHG_MASK [07:07] */
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_DRD_STATE_CHG_MASK_MASK 0x00000080
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_DRD_STATE_CHG_MASK_SHIFT 7
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_DRD_STATE_CHG_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_STATUS :: USB_BDC_HSE_MASK [06:06] */
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_BDC_HSE_MASK_MASK       0x00000040
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_BDC_HSE_MASK_SHIFT      6
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_BDC_HSE_MASK_DEFAULT    0x00000001

/* USB_INTR2 :: CPU_MASK_STATUS :: USB_SCB_INVLD_ADDR_MASK [05:05] */
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_SCB_INVLD_ADDR_MASK_MASK 0x00000020
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_SCB_INVLD_ADDR_MASK_SHIFT 5
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_SCB_INVLD_ADDR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_STATUS :: USB_XHC_HSE_INTR_MASK [04:04] */
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_XHC_HSE_INTR_MASK_MASK  0x00000010
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_XHC_HSE_INTR_MASK_SHIFT 4
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_XHC_HSE_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_STATUS :: USB_SOFT_SHUTDOWN_INTR_MASK [03:03] */
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_SOFT_SHUTDOWN_INTR_MASK_MASK 0x00000008
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_SOFT_SHUTDOWN_INTR_MASK_SHIFT 3
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_SOFT_SHUTDOWN_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_STATUS :: USB_S2_DISCON_INTR_MASK [02:02] */
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_S2_DISCON_INTR_MASK_MASK 0x00000004
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_S2_DISCON_INTR_MASK_SHIFT 2
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_S2_DISCON_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_STATUS :: USB_S2_CONNECT_INTR_MASK [01:01] */
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_S2_CONNECT_INTR_MASK_MASK 0x00000002
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_S2_CONNECT_INTR_MASK_SHIFT 1
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_S2_CONNECT_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_STATUS :: USB_GR_BRIDGE_INTR_MASK [00:00] */
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_GR_BRIDGE_INTR_MASK_MASK 0x00000001
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_GR_BRIDGE_INTR_MASK_SHIFT 0
#define BCHP_USB_INTR2_CPU_MASK_STATUS_USB_GR_BRIDGE_INTR_MASK_DEFAULT 0x00000001

/***************************************************************************
 *CPU_MASK_SET - CPU interrupt Mask Set Register
 ***************************************************************************/
/* USB_INTR2 :: CPU_MASK_SET :: reserved0 [31:08] */
#define BCHP_USB_INTR2_CPU_MASK_SET_reserved0_MASK                 0xffffff00
#define BCHP_USB_INTR2_CPU_MASK_SET_reserved0_SHIFT                8

/* USB_INTR2 :: CPU_MASK_SET :: USB_DRD_STATE_CHG_MASK [07:07] */
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_DRD_STATE_CHG_MASK_MASK    0x00000080
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_DRD_STATE_CHG_MASK_SHIFT   7
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_DRD_STATE_CHG_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_SET :: USB_BDC_HSE_MASK [06:06] */
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_BDC_HSE_MASK_MASK          0x00000040
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_BDC_HSE_MASK_SHIFT         6
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_BDC_HSE_MASK_DEFAULT       0x00000001

/* USB_INTR2 :: CPU_MASK_SET :: USB_SCB_INVLD_ADDR_MASK [05:05] */
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_SCB_INVLD_ADDR_MASK_MASK   0x00000020
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_SCB_INVLD_ADDR_MASK_SHIFT  5
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_SCB_INVLD_ADDR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_SET :: USB_XHC_HSE_INTR_MASK [04:04] */
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_XHC_HSE_INTR_MASK_MASK     0x00000010
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_XHC_HSE_INTR_MASK_SHIFT    4
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_XHC_HSE_INTR_MASK_DEFAULT  0x00000001

/* USB_INTR2 :: CPU_MASK_SET :: USB_SOFT_SHUTDOWN_INTR_MASK [03:03] */
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_SOFT_SHUTDOWN_INTR_MASK_MASK 0x00000008
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_SOFT_SHUTDOWN_INTR_MASK_SHIFT 3
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_SOFT_SHUTDOWN_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_SET :: USB_S2_DISCON_INTR_MASK [02:02] */
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_S2_DISCON_INTR_MASK_MASK   0x00000004
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_S2_DISCON_INTR_MASK_SHIFT  2
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_S2_DISCON_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_SET :: USB_S2_CONNECT_INTR_MASK [01:01] */
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_S2_CONNECT_INTR_MASK_MASK  0x00000002
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_S2_CONNECT_INTR_MASK_SHIFT 1
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_S2_CONNECT_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_SET :: USB_GR_BRIDGE_INTR_MASK [00:00] */
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_GR_BRIDGE_INTR_MASK_MASK   0x00000001
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_GR_BRIDGE_INTR_MASK_SHIFT  0
#define BCHP_USB_INTR2_CPU_MASK_SET_USB_GR_BRIDGE_INTR_MASK_DEFAULT 0x00000001

/***************************************************************************
 *CPU_MASK_CLEAR - CPU interrupt Mask Clear Register
 ***************************************************************************/
/* USB_INTR2 :: CPU_MASK_CLEAR :: reserved0 [31:08] */
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_reserved0_MASK               0xffffff00
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_reserved0_SHIFT              8

/* USB_INTR2 :: CPU_MASK_CLEAR :: USB_DRD_STATE_CHG_MASK [07:07] */
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_DRD_STATE_CHG_MASK_MASK  0x00000080
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_DRD_STATE_CHG_MASK_SHIFT 7
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_DRD_STATE_CHG_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_CLEAR :: USB_BDC_HSE_MASK [06:06] */
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_BDC_HSE_MASK_MASK        0x00000040
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_BDC_HSE_MASK_SHIFT       6
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_BDC_HSE_MASK_DEFAULT     0x00000001

/* USB_INTR2 :: CPU_MASK_CLEAR :: USB_SCB_INVLD_ADDR_MASK [05:05] */
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_SCB_INVLD_ADDR_MASK_MASK 0x00000020
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_SCB_INVLD_ADDR_MASK_SHIFT 5
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_SCB_INVLD_ADDR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_CLEAR :: USB_XHC_HSE_INTR_MASK [04:04] */
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_XHC_HSE_INTR_MASK_MASK   0x00000010
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_XHC_HSE_INTR_MASK_SHIFT  4
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_XHC_HSE_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_CLEAR :: USB_SOFT_SHUTDOWN_INTR_MASK [03:03] */
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_SOFT_SHUTDOWN_INTR_MASK_MASK 0x00000008
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_SOFT_SHUTDOWN_INTR_MASK_SHIFT 3
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_SOFT_SHUTDOWN_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_CLEAR :: USB_S2_DISCON_INTR_MASK [02:02] */
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_S2_DISCON_INTR_MASK_MASK 0x00000004
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_S2_DISCON_INTR_MASK_SHIFT 2
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_S2_DISCON_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_CLEAR :: USB_S2_CONNECT_INTR_MASK [01:01] */
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_S2_CONNECT_INTR_MASK_MASK 0x00000002
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_S2_CONNECT_INTR_MASK_SHIFT 1
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_S2_CONNECT_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: CPU_MASK_CLEAR :: USB_GR_BRIDGE_INTR_MASK [00:00] */
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_GR_BRIDGE_INTR_MASK_MASK 0x00000001
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_GR_BRIDGE_INTR_MASK_SHIFT 0
#define BCHP_USB_INTR2_CPU_MASK_CLEAR_USB_GR_BRIDGE_INTR_MASK_DEFAULT 0x00000001

/***************************************************************************
 *PCI_STATUS - PCI interrupt Status Register
 ***************************************************************************/
/* USB_INTR2 :: PCI_STATUS :: reserved0 [31:08] */
#define BCHP_USB_INTR2_PCI_STATUS_reserved0_MASK                   0xffffff00
#define BCHP_USB_INTR2_PCI_STATUS_reserved0_SHIFT                  8

/* USB_INTR2 :: PCI_STATUS :: USB_DRD_STATE_CHG_INTR [07:07] */
#define BCHP_USB_INTR2_PCI_STATUS_USB_DRD_STATE_CHG_INTR_MASK      0x00000080
#define BCHP_USB_INTR2_PCI_STATUS_USB_DRD_STATE_CHG_INTR_SHIFT     7
#define BCHP_USB_INTR2_PCI_STATUS_USB_DRD_STATE_CHG_INTR_DEFAULT   0x00000000

/* USB_INTR2 :: PCI_STATUS :: USB_BDC_HSE_INTR [06:06] */
#define BCHP_USB_INTR2_PCI_STATUS_USB_BDC_HSE_INTR_MASK            0x00000040
#define BCHP_USB_INTR2_PCI_STATUS_USB_BDC_HSE_INTR_SHIFT           6
#define BCHP_USB_INTR2_PCI_STATUS_USB_BDC_HSE_INTR_DEFAULT         0x00000000

/* USB_INTR2 :: PCI_STATUS :: USB_SCB_INVLD_ADDR_INTR [05:05] */
#define BCHP_USB_INTR2_PCI_STATUS_USB_SCB_INVLD_ADDR_INTR_MASK     0x00000020
#define BCHP_USB_INTR2_PCI_STATUS_USB_SCB_INVLD_ADDR_INTR_SHIFT    5
#define BCHP_USB_INTR2_PCI_STATUS_USB_SCB_INVLD_ADDR_INTR_DEFAULT  0x00000000

/* USB_INTR2 :: PCI_STATUS :: USB_XHC_HSE_INTR [04:04] */
#define BCHP_USB_INTR2_PCI_STATUS_USB_XHC_HSE_INTR_MASK            0x00000010
#define BCHP_USB_INTR2_PCI_STATUS_USB_XHC_HSE_INTR_SHIFT           4
#define BCHP_USB_INTR2_PCI_STATUS_USB_XHC_HSE_INTR_DEFAULT         0x00000000

/* USB_INTR2 :: PCI_STATUS :: USB_SOFT_SHUTDOWN_INTR [03:03] */
#define BCHP_USB_INTR2_PCI_STATUS_USB_SOFT_SHUTDOWN_INTR_MASK      0x00000008
#define BCHP_USB_INTR2_PCI_STATUS_USB_SOFT_SHUTDOWN_INTR_SHIFT     3
#define BCHP_USB_INTR2_PCI_STATUS_USB_SOFT_SHUTDOWN_INTR_DEFAULT   0x00000000

/* USB_INTR2 :: PCI_STATUS :: USB_S2_DISCON_INTR [02:02] */
#define BCHP_USB_INTR2_PCI_STATUS_USB_S2_DISCON_INTR_MASK          0x00000004
#define BCHP_USB_INTR2_PCI_STATUS_USB_S2_DISCON_INTR_SHIFT         2
#define BCHP_USB_INTR2_PCI_STATUS_USB_S2_DISCON_INTR_DEFAULT       0x00000000

/* USB_INTR2 :: PCI_STATUS :: USB_S2_CONNECT_INTR [01:01] */
#define BCHP_USB_INTR2_PCI_STATUS_USB_S2_CONNECT_INTR_MASK         0x00000002
#define BCHP_USB_INTR2_PCI_STATUS_USB_S2_CONNECT_INTR_SHIFT        1
#define BCHP_USB_INTR2_PCI_STATUS_USB_S2_CONNECT_INTR_DEFAULT      0x00000000

/* USB_INTR2 :: PCI_STATUS :: USB_GR_BRIDGE_INTR [00:00] */
#define BCHP_USB_INTR2_PCI_STATUS_USB_GR_BRIDGE_INTR_MASK          0x00000001
#define BCHP_USB_INTR2_PCI_STATUS_USB_GR_BRIDGE_INTR_SHIFT         0
#define BCHP_USB_INTR2_PCI_STATUS_USB_GR_BRIDGE_INTR_DEFAULT       0x00000000

/***************************************************************************
 *PCI_SET - PCI interrupt Set Register
 ***************************************************************************/
/* USB_INTR2 :: PCI_SET :: reserved0 [31:08] */
#define BCHP_USB_INTR2_PCI_SET_reserved0_MASK                      0xffffff00
#define BCHP_USB_INTR2_PCI_SET_reserved0_SHIFT                     8

/* USB_INTR2 :: PCI_SET :: USB_DRD_STATE_CHG_INTR [07:07] */
#define BCHP_USB_INTR2_PCI_SET_USB_DRD_STATE_CHG_INTR_MASK         0x00000080
#define BCHP_USB_INTR2_PCI_SET_USB_DRD_STATE_CHG_INTR_SHIFT        7
#define BCHP_USB_INTR2_PCI_SET_USB_DRD_STATE_CHG_INTR_DEFAULT      0x00000000

/* USB_INTR2 :: PCI_SET :: USB_BDC_HSE_INTR [06:06] */
#define BCHP_USB_INTR2_PCI_SET_USB_BDC_HSE_INTR_MASK               0x00000040
#define BCHP_USB_INTR2_PCI_SET_USB_BDC_HSE_INTR_SHIFT              6
#define BCHP_USB_INTR2_PCI_SET_USB_BDC_HSE_INTR_DEFAULT            0x00000000

/* USB_INTR2 :: PCI_SET :: USB_SCB_INVLD_ADDR_INTR [05:05] */
#define BCHP_USB_INTR2_PCI_SET_USB_SCB_INVLD_ADDR_INTR_MASK        0x00000020
#define BCHP_USB_INTR2_PCI_SET_USB_SCB_INVLD_ADDR_INTR_SHIFT       5
#define BCHP_USB_INTR2_PCI_SET_USB_SCB_INVLD_ADDR_INTR_DEFAULT     0x00000000

/* USB_INTR2 :: PCI_SET :: USB_XHC_HSE_INTR [04:04] */
#define BCHP_USB_INTR2_PCI_SET_USB_XHC_HSE_INTR_MASK               0x00000010
#define BCHP_USB_INTR2_PCI_SET_USB_XHC_HSE_INTR_SHIFT              4
#define BCHP_USB_INTR2_PCI_SET_USB_XHC_HSE_INTR_DEFAULT            0x00000000

/* USB_INTR2 :: PCI_SET :: USB_SOFT_SHUTDOWN_INTR [03:03] */
#define BCHP_USB_INTR2_PCI_SET_USB_SOFT_SHUTDOWN_INTR_MASK         0x00000008
#define BCHP_USB_INTR2_PCI_SET_USB_SOFT_SHUTDOWN_INTR_SHIFT        3
#define BCHP_USB_INTR2_PCI_SET_USB_SOFT_SHUTDOWN_INTR_DEFAULT      0x00000000

/* USB_INTR2 :: PCI_SET :: USB_S2_DISCON_INTR [02:02] */
#define BCHP_USB_INTR2_PCI_SET_USB_S2_DISCON_INTR_MASK             0x00000004
#define BCHP_USB_INTR2_PCI_SET_USB_S2_DISCON_INTR_SHIFT            2
#define BCHP_USB_INTR2_PCI_SET_USB_S2_DISCON_INTR_DEFAULT          0x00000000

/* USB_INTR2 :: PCI_SET :: USB_S2_CONNECT_INTR [01:01] */
#define BCHP_USB_INTR2_PCI_SET_USB_S2_CONNECT_INTR_MASK            0x00000002
#define BCHP_USB_INTR2_PCI_SET_USB_S2_CONNECT_INTR_SHIFT           1
#define BCHP_USB_INTR2_PCI_SET_USB_S2_CONNECT_INTR_DEFAULT         0x00000000

/* USB_INTR2 :: PCI_SET :: USB_GR_BRIDGE_INTR [00:00] */
#define BCHP_USB_INTR2_PCI_SET_USB_GR_BRIDGE_INTR_MASK             0x00000001
#define BCHP_USB_INTR2_PCI_SET_USB_GR_BRIDGE_INTR_SHIFT            0
#define BCHP_USB_INTR2_PCI_SET_USB_GR_BRIDGE_INTR_DEFAULT          0x00000000

/***************************************************************************
 *PCI_CLEAR - PCI interrupt Clear Register
 ***************************************************************************/
/* USB_INTR2 :: PCI_CLEAR :: reserved0 [31:08] */
#define BCHP_USB_INTR2_PCI_CLEAR_reserved0_MASK                    0xffffff00
#define BCHP_USB_INTR2_PCI_CLEAR_reserved0_SHIFT                   8

/* USB_INTR2 :: PCI_CLEAR :: USB_DRD_STATE_CHG_INTR [07:07] */
#define BCHP_USB_INTR2_PCI_CLEAR_USB_DRD_STATE_CHG_INTR_MASK       0x00000080
#define BCHP_USB_INTR2_PCI_CLEAR_USB_DRD_STATE_CHG_INTR_SHIFT      7
#define BCHP_USB_INTR2_PCI_CLEAR_USB_DRD_STATE_CHG_INTR_DEFAULT    0x00000000

/* USB_INTR2 :: PCI_CLEAR :: USB_BDC_HSE_INTR [06:06] */
#define BCHP_USB_INTR2_PCI_CLEAR_USB_BDC_HSE_INTR_MASK             0x00000040
#define BCHP_USB_INTR2_PCI_CLEAR_USB_BDC_HSE_INTR_SHIFT            6
#define BCHP_USB_INTR2_PCI_CLEAR_USB_BDC_HSE_INTR_DEFAULT          0x00000000

/* USB_INTR2 :: PCI_CLEAR :: USB_SCB_INVLD_ADDR_INTR [05:05] */
#define BCHP_USB_INTR2_PCI_CLEAR_USB_SCB_INVLD_ADDR_INTR_MASK      0x00000020
#define BCHP_USB_INTR2_PCI_CLEAR_USB_SCB_INVLD_ADDR_INTR_SHIFT     5
#define BCHP_USB_INTR2_PCI_CLEAR_USB_SCB_INVLD_ADDR_INTR_DEFAULT   0x00000000

/* USB_INTR2 :: PCI_CLEAR :: USB_XHC_HSE_INTR [04:04] */
#define BCHP_USB_INTR2_PCI_CLEAR_USB_XHC_HSE_INTR_MASK             0x00000010
#define BCHP_USB_INTR2_PCI_CLEAR_USB_XHC_HSE_INTR_SHIFT            4
#define BCHP_USB_INTR2_PCI_CLEAR_USB_XHC_HSE_INTR_DEFAULT          0x00000000

/* USB_INTR2 :: PCI_CLEAR :: USB_SOFT_SHUTDOWN_INTR [03:03] */
#define BCHP_USB_INTR2_PCI_CLEAR_USB_SOFT_SHUTDOWN_INTR_MASK       0x00000008
#define BCHP_USB_INTR2_PCI_CLEAR_USB_SOFT_SHUTDOWN_INTR_SHIFT      3
#define BCHP_USB_INTR2_PCI_CLEAR_USB_SOFT_SHUTDOWN_INTR_DEFAULT    0x00000000

/* USB_INTR2 :: PCI_CLEAR :: USB_S2_DISCON_INTR [02:02] */
#define BCHP_USB_INTR2_PCI_CLEAR_USB_S2_DISCON_INTR_MASK           0x00000004
#define BCHP_USB_INTR2_PCI_CLEAR_USB_S2_DISCON_INTR_SHIFT          2
#define BCHP_USB_INTR2_PCI_CLEAR_USB_S2_DISCON_INTR_DEFAULT        0x00000000

/* USB_INTR2 :: PCI_CLEAR :: USB_S2_CONNECT_INTR [01:01] */
#define BCHP_USB_INTR2_PCI_CLEAR_USB_S2_CONNECT_INTR_MASK          0x00000002
#define BCHP_USB_INTR2_PCI_CLEAR_USB_S2_CONNECT_INTR_SHIFT         1
#define BCHP_USB_INTR2_PCI_CLEAR_USB_S2_CONNECT_INTR_DEFAULT       0x00000000

/* USB_INTR2 :: PCI_CLEAR :: USB_GR_BRIDGE_INTR [00:00] */
#define BCHP_USB_INTR2_PCI_CLEAR_USB_GR_BRIDGE_INTR_MASK           0x00000001
#define BCHP_USB_INTR2_PCI_CLEAR_USB_GR_BRIDGE_INTR_SHIFT          0
#define BCHP_USB_INTR2_PCI_CLEAR_USB_GR_BRIDGE_INTR_DEFAULT        0x00000000

/***************************************************************************
 *PCI_MASK_STATUS - PCI interrupt Mask Status Register
 ***************************************************************************/
/* USB_INTR2 :: PCI_MASK_STATUS :: reserved0 [31:08] */
#define BCHP_USB_INTR2_PCI_MASK_STATUS_reserved0_MASK              0xffffff00
#define BCHP_USB_INTR2_PCI_MASK_STATUS_reserved0_SHIFT             8

/* USB_INTR2 :: PCI_MASK_STATUS :: USB_DRD_STATE_CHG_MASK [07:07] */
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_DRD_STATE_CHG_MASK_MASK 0x00000080
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_DRD_STATE_CHG_MASK_SHIFT 7
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_DRD_STATE_CHG_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_STATUS :: USB_BDC_HSE_MASK [06:06] */
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_BDC_HSE_MASK_MASK       0x00000040
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_BDC_HSE_MASK_SHIFT      6
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_BDC_HSE_MASK_DEFAULT    0x00000001

/* USB_INTR2 :: PCI_MASK_STATUS :: USB_SCB_INVLD_ADDR_MASK [05:05] */
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_SCB_INVLD_ADDR_MASK_MASK 0x00000020
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_SCB_INVLD_ADDR_MASK_SHIFT 5
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_SCB_INVLD_ADDR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_STATUS :: USB_XHC_HSE_INTR_MASK [04:04] */
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_XHC_HSE_INTR_MASK_MASK  0x00000010
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_XHC_HSE_INTR_MASK_SHIFT 4
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_XHC_HSE_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_STATUS :: USB_SOFT_SHUTDOWN_INTR_MASK [03:03] */
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_SOFT_SHUTDOWN_INTR_MASK_MASK 0x00000008
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_SOFT_SHUTDOWN_INTR_MASK_SHIFT 3
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_SOFT_SHUTDOWN_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_STATUS :: USB_S2_DISCON_INTR_MASK [02:02] */
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_S2_DISCON_INTR_MASK_MASK 0x00000004
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_S2_DISCON_INTR_MASK_SHIFT 2
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_S2_DISCON_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_STATUS :: USB_S2_CONNECT_INTR_MASK [01:01] */
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_S2_CONNECT_INTR_MASK_MASK 0x00000002
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_S2_CONNECT_INTR_MASK_SHIFT 1
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_S2_CONNECT_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_STATUS :: USB_GR_BRIDGE_INTR_MASK [00:00] */
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_GR_BRIDGE_INTR_MASK_MASK 0x00000001
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_GR_BRIDGE_INTR_MASK_SHIFT 0
#define BCHP_USB_INTR2_PCI_MASK_STATUS_USB_GR_BRIDGE_INTR_MASK_DEFAULT 0x00000001

/***************************************************************************
 *PCI_MASK_SET - PCI interrupt Mask Set Register
 ***************************************************************************/
/* USB_INTR2 :: PCI_MASK_SET :: reserved0 [31:08] */
#define BCHP_USB_INTR2_PCI_MASK_SET_reserved0_MASK                 0xffffff00
#define BCHP_USB_INTR2_PCI_MASK_SET_reserved0_SHIFT                8

/* USB_INTR2 :: PCI_MASK_SET :: USB_DRD_STATE_CHG_MASK [07:07] */
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_DRD_STATE_CHG_MASK_MASK    0x00000080
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_DRD_STATE_CHG_MASK_SHIFT   7
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_DRD_STATE_CHG_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_SET :: USB_BDC_HSE_MASK [06:06] */
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_BDC_HSE_MASK_MASK          0x00000040
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_BDC_HSE_MASK_SHIFT         6
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_BDC_HSE_MASK_DEFAULT       0x00000001

/* USB_INTR2 :: PCI_MASK_SET :: USB_SCB_INVLD_ADDR_MASK [05:05] */
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_SCB_INVLD_ADDR_MASK_MASK   0x00000020
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_SCB_INVLD_ADDR_MASK_SHIFT  5
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_SCB_INVLD_ADDR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_SET :: USB_XHC_HSE_INTR_MASK [04:04] */
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_XHC_HSE_INTR_MASK_MASK     0x00000010
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_XHC_HSE_INTR_MASK_SHIFT    4
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_XHC_HSE_INTR_MASK_DEFAULT  0x00000001

/* USB_INTR2 :: PCI_MASK_SET :: USB_SOFT_SHUTDOWN_INTR_MASK [03:03] */
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_SOFT_SHUTDOWN_INTR_MASK_MASK 0x00000008
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_SOFT_SHUTDOWN_INTR_MASK_SHIFT 3
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_SOFT_SHUTDOWN_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_SET :: USB_S2_DISCON_INTR_MASK [02:02] */
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_S2_DISCON_INTR_MASK_MASK   0x00000004
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_S2_DISCON_INTR_MASK_SHIFT  2
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_S2_DISCON_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_SET :: USB_S2_CONNECT_INTR_MASK [01:01] */
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_S2_CONNECT_INTR_MASK_MASK  0x00000002
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_S2_CONNECT_INTR_MASK_SHIFT 1
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_S2_CONNECT_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_SET :: USB_GR_BRIDGE_INTR_MASK [00:00] */
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_GR_BRIDGE_INTR_MASK_MASK   0x00000001
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_GR_BRIDGE_INTR_MASK_SHIFT  0
#define BCHP_USB_INTR2_PCI_MASK_SET_USB_GR_BRIDGE_INTR_MASK_DEFAULT 0x00000001

/***************************************************************************
 *PCI_MASK_CLEAR - PCI interrupt Mask Clear Register
 ***************************************************************************/
/* USB_INTR2 :: PCI_MASK_CLEAR :: reserved0 [31:08] */
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_reserved0_MASK               0xffffff00
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_reserved0_SHIFT              8

/* USB_INTR2 :: PCI_MASK_CLEAR :: USB_DRD_STATE_CHG_MASK [07:07] */
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_DRD_STATE_CHG_MASK_MASK  0x00000080
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_DRD_STATE_CHG_MASK_SHIFT 7
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_DRD_STATE_CHG_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_CLEAR :: USB_BDC_HSE_MASK [06:06] */
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_BDC_HSE_MASK_MASK        0x00000040
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_BDC_HSE_MASK_SHIFT       6
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_BDC_HSE_MASK_DEFAULT     0x00000001

/* USB_INTR2 :: PCI_MASK_CLEAR :: USB_SCB_INVLD_ADDR_MASK [05:05] */
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_SCB_INVLD_ADDR_MASK_MASK 0x00000020
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_SCB_INVLD_ADDR_MASK_SHIFT 5
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_SCB_INVLD_ADDR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_CLEAR :: USB_XHC_HSE_INTR_MASK [04:04] */
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_XHC_HSE_INTR_MASK_MASK   0x00000010
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_XHC_HSE_INTR_MASK_SHIFT  4
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_XHC_HSE_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_CLEAR :: USB_SOFT_SHUTDOWN_INTR_MASK [03:03] */
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_SOFT_SHUTDOWN_INTR_MASK_MASK 0x00000008
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_SOFT_SHUTDOWN_INTR_MASK_SHIFT 3
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_SOFT_SHUTDOWN_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_CLEAR :: USB_S2_DISCON_INTR_MASK [02:02] */
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_S2_DISCON_INTR_MASK_MASK 0x00000004
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_S2_DISCON_INTR_MASK_SHIFT 2
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_S2_DISCON_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_CLEAR :: USB_S2_CONNECT_INTR_MASK [01:01] */
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_S2_CONNECT_INTR_MASK_MASK 0x00000002
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_S2_CONNECT_INTR_MASK_SHIFT 1
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_S2_CONNECT_INTR_MASK_DEFAULT 0x00000001

/* USB_INTR2 :: PCI_MASK_CLEAR :: USB_GR_BRIDGE_INTR_MASK [00:00] */
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_GR_BRIDGE_INTR_MASK_MASK 0x00000001
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_GR_BRIDGE_INTR_MASK_SHIFT 0
#define BCHP_USB_INTR2_PCI_MASK_CLEAR_USB_GR_BRIDGE_INTR_MASK_DEFAULT 0x00000001

#endif /* #ifndef BCHP_USB_INTR2_H__ */

/* End of File */
