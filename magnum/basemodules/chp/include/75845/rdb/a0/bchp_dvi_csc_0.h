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
 * Date:           Generated on               Tue Mar 10 17:57:38 2015
 *                 Full Compile MD5 Checksum  cac1d82e0ea60ac191e5eb0deb00af85
 *                     (minus title and desc)
 *                 MD5 Checksum               e839fde353a1ae191e2e40126f6846c9
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     15839
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_DVI_CSC_0_H__
#define BCHP_DVI_CSC_0_H__

/***************************************************************************
 *DVI_CSC_0 - DVI Frontend Color Space Converter 0
 ***************************************************************************/
#define BCHP_DVI_CSC_0_CSC_REV_ID                0x00685100 /* [RO] Revision ID register */
#define BCHP_DVI_CSC_0_CSC_MODE                  0x00685108 /* [RW] Color space converter mode register */
#define BCHP_DVI_CSC_0_CSC_MIN_MAX               0x0068510c /* [RW] Color space converter min_max clamp register */
#define BCHP_DVI_CSC_0_CSC_COEFF_C01_C00         0x00685110 /* [RW] Video Encoder Color Matrix coefficients c01 and c00 */
#define BCHP_DVI_CSC_0_CSC_COEFF_C03_C02         0x00685114 /* [RW] Video Encoder Color Matrix coefficients c03 and c02 */
#define BCHP_DVI_CSC_0_CSC_COEFF_C11_C10         0x00685118 /* [RW] Video Encoder Color Matrix coefficients c11 and c10 */
#define BCHP_DVI_CSC_0_CSC_COEFF_C13_C12         0x0068511c /* [RW] Video Encoder Color Matrix coefficients c13 and c12 */
#define BCHP_DVI_CSC_0_CSC_COEFF_C21_C20         0x00685120 /* [RW] Video Encoder Color Matrix coefficients c21 and c20 */
#define BCHP_DVI_CSC_0_CSC_COEFF_C23_C22         0x00685124 /* [RW] Video Encoder Color Matrix coefficients c23 and c22 */
#define BCHP_DVI_CSC_0_DITHER_CONTROL            0x00685128 /* [RW] Color Space Converter Dither Control */
#define BCHP_DVI_CSC_0_DITHER_LFSR               0x0068512c /* [RW] Color Space Converter Dither LFSR Control */
#define BCHP_DVI_CSC_0_DITHER_LFSR_INIT          0x00685130 /* [RW] Color Space Converter Dither LFSR Init value and control */

#endif /* #ifndef BCHP_DVI_CSC_0_H__ */

/* End of File */
