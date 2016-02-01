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

#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* Dbglib */
#include "bkni.h"			/* For critical sections */
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"      /* VBI internal data structures */

BDBG_MODULE(BVBI);

/***************************************************************************
* Private data
***************************************************************************/

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

static int P_size_by_type_isr (BVBI_AMOL_Type type);


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVBI_Field_GetAMOLData_isr (
	BVBI_Field_Handle fieldHandle,	
	BVBI_AMOL_Type      *pAmolType,
	uint8_t            *pAMOLData,
	unsigned int*         pLength
)
{
	int count;
	int size;
	uint8_t* fdata;
	BVBI_P_Field_Handle* pVbi_Fld;
	BERR_Code eErr = BERR_SUCCESS;

	BDBG_ENTER(BVBI_Field_GetAMOLData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	if((!pVbi_Fld) || (!pAMOLData) || (!pAmolType) || (!pLength))
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Verify that data is present on this field handle */
	if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_AMOL))
		return (BVBI_ERR_FIELD_NODATA);
	else if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_AMOL_NOENCODE)
		eErr = (BVBI_ERR_FIELD_BADDATA);

	/* Check that field handle was properly sized */
	fdata = pVbi_Fld->pAmolData;
	if (!fdata)
	{
		return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
	}

	/* Return data as requested */
	BDBG_ASSERT (pVbi_Fld->amolType != BVBI_AMOL_Type_None);
	size = P_size_by_type_isr (pVbi_Fld->amolType);
	BDBG_ASSERT (size >= 0);
	*pAmolType = pVbi_Fld->amolType;
	for (count = 0 ; count < size ; ++count)
		*pAMOLData++ = *fdata++;
	*pLength = P_size_by_type_isr (*pAmolType);

	BDBG_LEAVE(BVBI_Field_GetAMOLData_isr);
	return eErr;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SetAMOLData_isr(
	BVBI_Field_Handle fieldHandle,	
	BVBI_AMOL_Type        amolType,
	uint8_t             *pAMOLData,
	unsigned int            length
)
{
	int count;
	int size;
	BVBI_P_Field_Handle* pVbi_Fld;

	BDBG_ENTER(BVBI_Field_SetAMOLData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	if((!pVbi_Fld) || (!pAMOLData))
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	/* TODO: Check amolType more carefully? */
	if (amolType == BVBI_AMOL_Type_None)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	size = P_size_by_type_isr (amolType);
	if (size < 0)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	if ((unsigned int)size > length)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Check that field handle was properly sized */
	if (!pVbi_Fld->pAmolData)
	{
		return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
	}
	if (pVbi_Fld->amolSize < size)
	{
		return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
	}

	/* Store data as requested */
	pVbi_Fld->amolType = amolType;
	for (count = 0 ; count < size ; ++count)
		pVbi_Fld->pAmolData[count] = *pAMOLData++;

	/* Indicate valid data is present */
	pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_AMOL;

	BDBG_LEAVE(BVBI_Field_SetAMOLData_isr);
	return BERR_SUCCESS;
}


/***************************************************************************
* Implementation of supporting AMOL functions that are not in API
***************************************************************************/

BERR_Code BVBI_P_AMOL_Init( BVBI_P_Handle *pVbi )
{
#if (BVBI_NUM_AMOLE > 0) || (BVBI_NUM_AMOLE_656 > 0)
	uint8_t hwIndex;
#endif

	BDBG_ENTER(BVBI_P_AMOL_Init);

#if (BVBI_NUM_AMOLE == 0) && (BVBI_NUM_AMOLE_656 == 0)
	BSTD_UNUSED (pVbi);
#endif

#if (BVBI_NUM_AMOLE > 0)
	for (hwIndex = 0 ; hwIndex < BVBI_NUM_AMOLE ; ++hwIndex)
		BVBI_P_AMOL_Enc_Init (pVbi->hReg, false, hwIndex);
#endif
#if (BVBI_NUM_AMOLE_656 > 0)
	for (hwIndex = 0 ; hwIndex < BVBI_NUM_AMOLE_656 ; ++hwIndex)
		BVBI_P_AMOL_Enc_Init (pVbi->hReg, true, hwIndex);
#endif

	BDBG_LEAVE(BVBI_P_AMOL_Init);
	return BERR_SUCCESS;
}


/***************************************************************************
* Static (private) functions
***************************************************************************/

/***************************************************************************
 *
 */
static int P_size_by_type_isr (BVBI_AMOL_Type type)
{
	int size;

	switch (type)
	{
	case BVBI_AMOL_Type_None:
		size = 0;
		break;
	case BVBI_AMOL_Type_I:
		size = 6;
		break;
	case BVBI_AMOL_Type_II_Lowrate:
		size = 12;
		break;
	case BVBI_AMOL_Type_II_Highrate:
		size = 24;
		break;
	default:
		size = -1;
		break;
	}

	return size;
}

/* End of file */
