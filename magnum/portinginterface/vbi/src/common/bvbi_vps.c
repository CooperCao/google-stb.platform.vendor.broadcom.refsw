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
* Forward declarations of static (private) functions
***************************************************************************/


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/


/***************************************************************************
 *
 */
BERR_Code BVBI_Field_GetVPSData_isr( BVBI_Field_Handle fieldHandle,
								     BVBI_VPSData *pVPSData )
{
	BVBI_P_Field_Handle* pVbi_Fld;
	BERR_Code eErr = BERR_SUCCESS;

	BDBG_ENTER(BVBI_Field_GetVPSData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	if((!pVbi_Fld) || (!pVPSData))
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Verify that data is present on this field handle */
	if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_VPS))
		return (BVBI_ERR_FIELD_NODATA);
	if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_VPS_NOENCODE)
		eErr = (BVBI_ERR_FIELD_BADDATA);

	/* Check that field handle was properly sized */
	if (!pVbi_Fld->pVPSData)
	{
		return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
	}

	/* Return data as requested */
    *pVPSData = *pVbi_Fld->pVPSData;

	BDBG_LEAVE(BVBI_Field_GetVPSData_isr);
	return eErr;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SetVPSData_isr( BVBI_Field_Handle fieldHandle,
								     BVBI_VPSData *pVPSData )
{
	BVBI_P_Field_Handle* pVbi_Fld;

	BDBG_ENTER(BVBI_Field_SetVPSData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	if((!pVbi_Fld) || (!pVPSData))
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Check that field handle was properly sized */
	if (!pVbi_Fld->pVPSData)
	{
		return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
	}

	/* Store data as requested */
    *pVbi_Fld->pVPSData = *pVPSData;

	/* Indicate valid data is present */
	pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_VPS;

	BDBG_LEAVE(BVBI_Field_SetVPSData_isr);
	return BERR_SUCCESS;
}


/***************************************************************************
* Implementation of supporting VPS functions that are not in API
***************************************************************************/


BERR_Code BVBI_P_VPS_Init( BVBI_P_Handle *pVbi )
{
	uint8_t hwIndex;
	BERR_Code eErr = BERR_SUCCESS;

	BDBG_ENTER(BVBI_P_VPS_Init);

	/* Initialize VPS encoders */
	for (hwIndex = 0 ; hwIndex < BVBI_NUM_WSE ; ++hwIndex)
		BVBI_P_VPS_Enc_Init (pVbi->hReg, hwIndex);

	BDBG_LEAVE(BVBI_P_VPS_Init);
	return eErr;
}


/***************************************************************************
* Static (private) functions
***************************************************************************/

/* End of file */
