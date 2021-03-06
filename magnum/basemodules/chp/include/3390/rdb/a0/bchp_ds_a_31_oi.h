/****************************************************************************
 *     Copyright (c) 1999-2014, Broadcom Corporation
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
 * Date:           Generated on               Mon Sep 15 10:12:20 2014
 *                 Full Compile MD5 Checksum  ef22086ebd4065e4fea50dbc64f17e5e
 *                     (minus title and desc)
 *                 MD5 Checksum               39fcae49037a6337517df43bfc24b21f
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     14796
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_DS_A_31_OI_H__
#define BCHP_DS_A_31_OI_H__

/***************************************************************************
 *DS_A_31_OI - Downstream Output Interface Registers
 ***************************************************************************/
#define BCHP_DS_A_31_OI_STATUS                   0x0225f200 /* [RO] OI status */
#define BCHP_DS_A_31_OI_NCON                     0x0225f204 /* [RW] OI Clock Rate NCO Numerator */
#define BCHP_DS_A_31_OI_NCODL                    0x0225f208 /* [RW] OI Clock Rate NCO Delta */
#define BCHP_DS_A_31_OI_FIFO_CTL                 0x0225f20c /* [RW] FIFO control */
#define BCHP_DS_A_31_OI_CTL                      0x0225f210 /* [RW] OI Control */
#define BCHP_DS_A_31_OI_OUT                      0x0225f214 /* [RW] OI Output override Control */
#define BCHP_DS_A_31_OI_FRAME_CNT                0x0225f218 /* [RO] OI Frame Count */
#define BCHP_DS_A_31_OI_BERT_CTL                 0x0225f224 /* [RW] OI BERT Control Register */
#define BCHP_DS_A_31_OI_BERT_IT                  0x0225f228 /* [RW] OI BERT integration period and threshold */
#define BCHP_DS_A_31_OI_BERT_BIT_CNT_H           0x0225f22c /* [RO] OI BERT bit Counter snapshot value, higher bits */
#define BCHP_DS_A_31_OI_BERT_BIT_CNT_L           0x0225f230 /* [RO] OI BERT bit Counter snapshot value, lower bits */
#define BCHP_DS_A_31_OI_BERT_ERR_CNT_H           0x0225f234 /* [RO] OI BERT error Counter snapshot value, higher bits */
#define BCHP_DS_A_31_OI_BERT_ERR_CNT_L           0x0225f238 /* [RO] OI BERT error Counter snapshot value, lower bits */
#define BCHP_DS_A_31_OI_BERT_ST                  0x0225f23c /* [RO] OI BERT status register */

#endif /* #ifndef BCHP_DS_A_31_OI_H__ */

/* End of File */
