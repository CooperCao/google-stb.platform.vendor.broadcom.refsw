/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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

/*= Module Overview *********************************************************
<verbatim>

Overview
This common utility module provides the required math functions.


Design
none


Usage
none


Sample Code
none


</verbatim>
***************************************************************************/


#ifndef BMTH_H__
#define BMTH_H__


#ifdef __cplusplus
extern "C" {
#endif


void BMTH_HILO_64TO64_Neg(uint32_t xhi, uint32_t xlo, uint32_t *pouthi, uint32_t *poutlo);
void BMTH_HILO_32TO64_Mul(uint32_t x, uint32_t y, uint32_t *pouthi, uint32_t *poutlo);
void BMTH_HILO_64TO64_Mul(uint32_t xhi, uint32_t xlo, uint32_t yhi, uint32_t ylo, uint32_t *pouthi, uint32_t *poutlo);
void BMTH_HILO_64TO64_Add(uint32_t xhi, uint32_t xlo, uint32_t yhi, uint32_t ylo, uint32_t *pouthi, uint32_t *poutlo);
void BMTH_HILO_64TO64_Div32(uint32_t xhi, uint32_t xlo, uint32_t y, uint32_t *pouthi, uint32_t *poutlo);
uint32_t BMTH_2560log10(uint32_t x);

#ifdef __cplusplus
}
#endif
 
#endif



