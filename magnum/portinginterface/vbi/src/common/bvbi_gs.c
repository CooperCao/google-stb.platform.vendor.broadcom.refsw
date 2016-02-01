/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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
BERR_Code BVBI_Field_GetGSData_isr( BVBI_Field_Handle fieldHandle,
	BVBI_GSData* pGSData)
{
	BVBI_P_Field_Handle* pVbi_Fld;
	BERR_Code eErr = BERR_SUCCESS;

	BDBG_ENTER(BVBI_Field_GetGSData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	if((!pVbi_Fld) || (!pGSData))
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Verify that data is present on this field handle */
	if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_GS))
		return (BVBI_ERR_FIELD_NODATA);
	else if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_GEMSTAR_NOENCODE)
		eErr = (BVBI_ERR_FIELD_BADDATA);

	/* Check that field handle was properly sized */
	if (!pVbi_Fld->pGSData)
	{
		return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
	}

	/* Return data as requested */
	*pGSData = *(pVbi_Fld->pGSData);

	BDBG_LEAVE(BVBI_Field_GetGSData_isr);
	return eErr;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SetGSData_isr( BVBI_Field_Handle fieldHandle,
	BVBI_GSData* pGSData)
{
	BVBI_P_Field_Handle* pVbi_Fld;

	BDBG_ENTER(BVBI_Field_SetGSData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	if((!pVbi_Fld) || (!pGSData))
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Check that field handle was properly sized */
	if (!pVbi_Fld->pGSData)
	{
		return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
	}

	/* Store data as requested */
	*(pVbi_Fld->pGSData) = *pGSData;

	/* Indicate valid data is present */
	pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_GS;

	BDBG_LEAVE(BVBI_Field_SetGSData_isr);
	return BERR_SUCCESS;
}


/***************************************************************************
* Implementation of supporting GS functions that are not in API
***************************************************************************/

BERR_Code BVBI_P_GS_Init( BVBI_P_Handle *pVbi )
{
#if (BVBI_NUM_GSE > 0) || (BVBI_NUM_GSE_656 > 0)
	uint8_t hwIndex;
#endif

	BDBG_ENTER(BVBI_P_GS_Init);

#if (BVBI_NUM_GSE == 0) && (BVBI_NUM_GSE_656 == 0)
	BSTD_UNUSED (pVbi);
#endif

	/* Initialize CC encoders */
#if (BVBI_NUM_GSE > 0)
	for (hwIndex = 0 ; hwIndex < BVBI_NUM_GSE ; ++hwIndex)
		BVBI_P_GS_Enc_Init (pVbi->hReg, false, hwIndex);
#endif
#if (BVBI_NUM_GSE_656 > 0)
	for (hwIndex = 0 ; hwIndex < BVBI_NUM_GSE_656 ; ++hwIndex)
		BVBI_P_GS_Enc_Init (pVbi->hReg, true, hwIndex);
#endif

	BDBG_LEAVE(BVBI_P_GS_Init);
	return BERR_SUCCESS;
}

/* End of file */
