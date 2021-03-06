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
 * Date:           Generated on               Wed Apr 15 16:00:29 2015
 *                 Full Compile MD5 Checksum  798b3ac22e50cf765b00b72a7779366d
 *                     (minus title and desc)
 *                 MD5 Checksum               ccf80b3ba114a13bf874c64a54245c9a
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     16006
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_SDS_INTR2_0_4_H__
#define BCHP_SDS_INTR2_0_4_H__

/***************************************************************************
 *SDS_INTR2_0_4 - SDS L2 Interrupt Control Registers set 0
 ***************************************************************************/
#define BCHP_SDS_INTR2_0_4_CPU_STATUS            0x04400b00 /* [RO] CPU interrupt Status Register */
#define BCHP_SDS_INTR2_0_4_CPU_SET               0x04400b04 /* [WO] CPU interrupt Set Register */
#define BCHP_SDS_INTR2_0_4_CPU_CLEAR             0x04400b08 /* [WO] CPU interrupt Clear Register */
#define BCHP_SDS_INTR2_0_4_CPU_MASK_STATUS       0x04400b0c /* [RO] CPU interrupt Mask Status Register */
#define BCHP_SDS_INTR2_0_4_CPU_MASK_SET          0x04400b10 /* [WO] CPU interrupt Mask Set Register */
#define BCHP_SDS_INTR2_0_4_CPU_MASK_CLEAR        0x04400b14 /* [WO] CPU interrupt Mask Clear Register */
#define BCHP_SDS_INTR2_0_4_PCI_STATUS            0x04400b18 /* [RO] PCI interrupt Status Register */
#define BCHP_SDS_INTR2_0_4_PCI_SET               0x04400b1c /* [WO] PCI interrupt Set Register */
#define BCHP_SDS_INTR2_0_4_PCI_CLEAR             0x04400b20 /* [WO] PCI interrupt Clear Register */
#define BCHP_SDS_INTR2_0_4_PCI_MASK_STATUS       0x04400b24 /* [RO] PCI interrupt Mask Status Register */
#define BCHP_SDS_INTR2_0_4_PCI_MASK_SET          0x04400b28 /* [WO] PCI interrupt Mask Set Register */
#define BCHP_SDS_INTR2_0_4_PCI_MASK_CLEAR        0x04400b2c /* [WO] PCI interrupt Mask Clear Register */

#endif /* #ifndef BCHP_SDS_INTR2_0_4_H__ */

/* End of File */
