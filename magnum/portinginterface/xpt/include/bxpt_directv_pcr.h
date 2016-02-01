/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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


#ifndef BXPT_DIRECTV_PCR_H__
#define BXPT_DIRECTV_PCR_H__

#include "bxpt_pcr.h"

#ifdef __cplusplus
extern "C"{
#endif

/*=************************ Module Overview ********************************
<verbatim>
The API module provides the direcTV only APIs for the PCR module. The APIs
in bxpt_pcr.h are required to complete PCR modules configuration in directv mode. 

The DirecTV PCR API operates on a PCR channel handle. 
</verbatim>
***************************************************************************/

/***************************************************************************
Summary:
Enumeration for the different types of channels supported by the record 
module.
****************************************************************************/
typedef enum BXPT_PcrMode 
{
	BXPT_PcrMode_eDirecTv,
	BXPT_PcrMode_eMpeg 
}
BXPT_PcrMode;

/***************************************************************************
Summary:
Set the stream type used by a PCR module.
	 
Description: 
This function sets mode for a given PCR module to DirecTV, or back to the
default MPEG.

Returns:
    BERR_SUCCESS                - Directv mode set 
    BERR_INVALID_PARAMETER      - Bad input parameter  
***************************************************************************/
BERR_Code BXPT_PCR_DirecTv_SetPcrMode( 
	BXPT_PCR_Handle hPcr, 			   /* [In]The Pcr handle */
	BXPT_PcrMode Mode
	);

/***************************************************************************
Summary:
	Gets the last PCR captured in last PCR Hi/Lo registers for DirecTv mode
Description:
	This function reads the values from PCR_LAST_PCR_HI/LO registers.
Returns:
    BERR_SUCCESS                - Retrieved last PCR
    BERR_INVALID_PARAMETER      - Bad input parameter 
***************************************************************************/
BERR_Code	BXPT_PCR_DirecTv_GetLastPcr( 
	BXPT_PCR_Handle hPcr, 			   /* [in]The Pcr handle */
	uint32_t *pPcr      			  /*[out] 32 bits of RTS*/
	);

/***************************************************************************
Summary:
	ISR version of BXPT_PCR_DirecTv_GetLastPcr
***************************************************************************/
BERR_Code	BXPT_PCR_DirecTv_GetLastPcr_isr(
	BXPT_PCR_Handle hPcr, 			   /* [in]The Pcr handle */
	uint32_t *pPcr      			  /*[out] 32 bits of RTS*/
	);

/***************************************************************************
Summary:
	Gets the STC counter values for DirecTv mode
Description: 
	This function read the PCR STC counters from STC_HI/LO registers.
Returns:
    BERR_SUCCESS                - Retrieved STC counter values
    BERR_INVALID_PARAMETER      - Bad input parameter 
***************************************************************************/
BERR_Code	BXPT_PCR_DirecTv_GetStc( 
	BXPT_PCR_Handle hPcr,          /* [in]The Pcr handle */
	uint32_t *pStcHi               /*[out] 32 bits of RTS*/
	);

#ifdef __cplusplus
}
#endif

#endif

