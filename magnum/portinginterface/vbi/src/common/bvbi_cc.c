/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
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

#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* Dbglib */
#include "bkni.h"			/* For critical sections */
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"      /* VBI internal data structures */

BDBG_MODULE(BVBI);

/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_GetCCData_isr (
	BVBI_Field_Handle fieldHandle,
	uint8_t           *pucLowByte,	
	uint8_t          *pucHighByte
)
{
	BVBI_P_Field_Handle* pVbi_Fld;
	BERR_Code eErr = BERR_SUCCESS;

	BDBG_ENTER(BVBI_Field_GetCCData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	if((!pVbi_Fld) || (!pucLowByte) || (!pucHighByte))
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Verify that data is present on this field handle */
	if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_CC))
		return (BVBI_ERR_FIELD_NODATA);
	else if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_CC_NOENCODE)
		eErr = (BVBI_ERR_FIELD_BADDATA);

	/* Return data as requested */
	*pucLowByte  = pVbi_Fld->usCCData;
	*pucHighByte = (pVbi_Fld->usCCData >> 8);

	BDBG_LEAVE(BVBI_Field_GetCCData_isr);
	return eErr;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SetCCData_isr(
	BVBI_Field_Handle fieldHandle,
	 uint8_t             cLowByte,	
	 uint8_t            cHighByte
)
{
	BVBI_P_Field_Handle* pVbi_Fld;

	BDBG_ENTER(BVBI_SetCCData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	if(!pVbi_Fld)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Store data as requested */
    pVbi_Fld->usCCData = ((uint16_t)cHighByte << 8) | (uint16_t)cLowByte;

	/* Indicate valid data is present */
	pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_CC;

	BDBG_LEAVE(BVBI_SetCCData_isr);
	return BERR_SUCCESS;
}


/***************************************************************************
* Implementation supporting closed caption functions that are not in API
***************************************************************************/


BERR_Code BVBI_P_CC_Init( BVBI_P_Handle *pVbi )
{
	uint8_t hwIndex;

	BDBG_ENTER(BVBI_P_CC_Init);

	/* Initialize CC encoders */
	for (hwIndex = 0 ; hwIndex < BVBI_NUM_CCE ; ++hwIndex)
		BVBI_P_CC_Enc_Init (pVbi->hReg, false, hwIndex);
#if (BVBI_NUM_CCE_656 > 0)
	for (hwIndex = 0 ; hwIndex < BVBI_NUM_CCE_656 ; ++hwIndex)
		BVBI_P_CC_Enc_Init (pVbi->hReg, true, hwIndex);
#endif

	BDBG_LEAVE(BVBI_P_CC_Init);
	return BERR_SUCCESS;
}

/* End of file */
