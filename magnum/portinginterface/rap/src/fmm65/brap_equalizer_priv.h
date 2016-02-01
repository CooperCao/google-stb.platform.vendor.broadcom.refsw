/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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
*   Module name: Hardware Equalizer
*   This file contains the definitions and prototypes for the hardware Equalizer
*   abstraction.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef _BRAP_EQUALIZER_PRIV_H__ /*{{{*/
#define _BRAP_EQUALIZER_PRIV_H__

/*{{{ Defines */

#define BRAP_P_MAX_GEQ_FILTERS 5

typedef struct BRAP_EQ_P_FilterCoeffs
{
    int32_t b0[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
    int32_t b1[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
    int32_t b2[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
    int32_t a1[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
    int32_t a2[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
}BRAP_EQ_P_FilterCoeffs;

BERR_Code BRAP_P_ApplyCoefficients (
    BRAP_EqualizerHandle hEqualizer
    );

BERR_Code BRAP_P_ApplyCoefficients_isr(
    BRAP_EqualizerHandle hEqualizer
    );
#endif /*}}} #ifndef _BRAP_EQUALIZER_PRIV_H__ */
	
/* End of File */
	

