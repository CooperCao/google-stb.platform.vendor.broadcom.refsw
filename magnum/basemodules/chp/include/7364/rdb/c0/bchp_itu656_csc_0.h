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
 * Date:           Generated on               Mon Feb  8 12:53:13 2016
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

#ifndef BCHP_ITU656_CSC_0_H__
#define BCHP_ITU656_CSC_0_H__

/***************************************************************************
 *ITU656_CSC_0 - ITU 656 Color Space Converter
 ***************************************************************************/
#define BCHP_ITU656_CSC_0_CSC_REV_ID             0x006a5600 /* [RO] Revision ID register */
#define BCHP_ITU656_CSC_0_CSC_MODE               0x006a5608 /* [RW] Color space converter mode register */
#define BCHP_ITU656_CSC_0_CSC_MIN_MAX            0x006a560c /* [RW] Color space converter min_max clamp register */
#define BCHP_ITU656_CSC_0_CSC_COEFF_C01_C00      0x006a5610 /* [RW] Video Encoder Color Matrix coefficients c01 and c00 */
#define BCHP_ITU656_CSC_0_CSC_COEFF_C03_C02      0x006a5614 /* [RW] Video Encoder Color Matrix coefficients c03 and c02 */
#define BCHP_ITU656_CSC_0_CSC_COEFF_C11_C10      0x006a5618 /* [RW] Video Encoder Color Matrix coefficients c11 and c10 */
#define BCHP_ITU656_CSC_0_CSC_COEFF_C13_C12      0x006a561c /* [RW] Video Encoder Color Matrix coefficients c13 and c12 */
#define BCHP_ITU656_CSC_0_CSC_COEFF_C21_C20      0x006a5620 /* [RW] Video Encoder Color Matrix coefficients c21 and c20 */
#define BCHP_ITU656_CSC_0_CSC_COEFF_C23_C22      0x006a5624 /* [RW] Video Encoder Color Matrix coefficients c23 and c22 */
#define BCHP_ITU656_CSC_0_DITHER_CONTROL         0x006a5628 /* [RW] Color Space Converter Dither Control */
#define BCHP_ITU656_CSC_0_DITHER_LFSR            0x006a562c /* [RW] Color Space Converter Dither LFSR Control */
#define BCHP_ITU656_CSC_0_DITHER_LFSR_INIT       0x006a5630 /* [RW] Color Space Converter Dither LFSR Init value and control */

#endif /* #ifndef BCHP_ITU656_CSC_0_H__ */

/* End of File */
