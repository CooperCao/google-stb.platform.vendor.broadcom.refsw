/***************************************************************************
 *     Copyright (c) 2003-2007, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BCSC_H__
#define BCSC_H__

#ifdef __cplusplus
extern "C" {
#endif

/*  Given source and display (xR, yR), (xG, yG), (xB, yB), (xW, yW), derive CMP matrix;
    Output: CMP matrix in array of 12 fixed point coefficients;
    inputs: 
        pul_SrcRGBW       - Source gamut in (xR, yR), (xG, yG), (xB, yB), (xW, yW) array, 8 values;
        pul_DispRGBW:     - Display gamut in (xR, yR), (xG, yG), (xB, yB), (xW, yW) array, 8 values;
        ulXyFractBits     - Fixed-point fractional bits of input gamut values in (x, y) array;
        ulMatrixFractBits - Fixed-point fractional bits of output matrix coefficients; */
BERR_Code BCSC_RGBW2CmpMatrix
	( uint32_t                              *pul_SrcRGBW,
	  uint32_t                              *pul_DispRGBW,
	  uint32_t                               ulXyFractBits,
	  uint32_t                               ulMatrixFractBits,
	  uint32_t                               ulN,
	  int32_t                               *plCmpMatrix,
	  int32_t                               *plDvoMatrix);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BCSC_H__*/

/* End of file. */
