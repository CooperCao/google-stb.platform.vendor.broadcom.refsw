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
 * Date:           Generated on               Mon Feb  8 12:53:15 2016
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

#ifndef BCHP_XPT_WDMA_SCPU_INTR_AGGREGATOR_H__
#define BCHP_XPT_WDMA_SCPU_INTR_AGGREGATOR_H__

/***************************************************************************
 *XPT_WDMA_SCPU_INTR_AGGREGATOR - SCPU Interrupt Aggregator
 ***************************************************************************/
#define BCHP_XPT_WDMA_SCPU_INTR_AGGREGATOR_INTR_W0_STATUS 0x00a68040 /* [RO] Interrupt Status Register */
#define BCHP_XPT_WDMA_SCPU_INTR_AGGREGATOR_INTR_W1_STATUS 0x00a68044 /* [RO] Interrupt Status Register */
#define BCHP_XPT_WDMA_SCPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS 0x00a68048 /* [RO] Interrupt Mask Status Register */
#define BCHP_XPT_WDMA_SCPU_INTR_AGGREGATOR_INTR_W1_MASK_STATUS 0x00a6804c /* [RO] Interrupt Mask Status Register */
#define BCHP_XPT_WDMA_SCPU_INTR_AGGREGATOR_INTR_W0_MASK_SET 0x00a68050 /* [WO] Interrupt Mask Set Register */
#define BCHP_XPT_WDMA_SCPU_INTR_AGGREGATOR_INTR_W1_MASK_SET 0x00a68054 /* [WO] Interrupt Mask Set Register */
#define BCHP_XPT_WDMA_SCPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR 0x00a68058 /* [WO] Interrupt Mask Clear Register */
#define BCHP_XPT_WDMA_SCPU_INTR_AGGREGATOR_INTR_W1_MASK_CLEAR 0x00a6805c /* [WO] Interrupt Mask Clear Register */

#endif /* #ifndef BCHP_XPT_WDMA_SCPU_INTR_AGGREGATOR_H__ */

/* End of File */
