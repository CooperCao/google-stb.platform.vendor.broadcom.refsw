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

#include "bstd.h"             /* standard types */
#include "bdbg.h"             /* Dbglib */
#include "bkni.h"			  /* For critical sections */
#include "bvbi.h"             /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"        /* VBI internal data structures */

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
BERR_Code
BVBI_Field_GetCGMSAData_isr( BVBI_Field_Handle vbiData, uint32_t *pulCGMSData )
{
	BVBI_P_Field_Handle* pVbi_Fld;
	BERR_Code eErr = BERR_SUCCESS;

	BDBG_ENTER(BVBI_Field_GetCGMSAData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(vbiData, pVbi_Fld);
	if((!pVbi_Fld) || (!pulCGMSData))
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Verify that data is present on this field handle */
	if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_CGMSA))
		return (BVBI_ERR_FIELD_NODATA);
	else if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_CGMS_NOENCODE)
		eErr = (BVBI_ERR_FIELD_BADDATA);

	/* Return data as requested */
    *pulCGMSData = pVbi_Fld->ulCGMSData;
	BDBG_LEAVE(BVBI_Field_GetCGMSAData_isr);
	return eErr;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SetCGMSAData_isr(
	BVBI_Field_Handle vbiData, uint32_t ulCGMSData )
{
	BVBI_P_Field_Handle* pVbi_Fld;

	BDBG_ENTER(BVBI_Field_SetCGMSAData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(vbiData, pVbi_Fld);
	if(!pVbi_Fld)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Store data as requested */
    pVbi_Fld->ulCGMSData = ulCGMSData;

	/* Indicate valid data is present */
	pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_CGMSA;

	BDBG_LEAVE(BVBI_Field_SetCGMSAData_isr);
	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code
BVBI_Field_GetCGMSBData_isr(
	BVBI_Field_Handle vbiData,  BVBI_CGMSB_Datum* pDatum)
{
	int subIndex;
	BVBI_P_Field_Handle* pVbi_Fld;
	BERR_Code eErr = BERR_SUCCESS;

	BDBG_ENTER(BVBI_Field_GetCGMSBData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(vbiData, pVbi_Fld);
	if((!pVbi_Fld) || (!pDatum))
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Verify that data is present on this field handle */
	if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_CGMSB))
		return (BVBI_ERR_FIELD_NODATA);
	else if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_CGMS_NOENCODE)
		eErr = (BVBI_ERR_FIELD_BADDATA);

	/* Check that field handle was properly sized */
	if (!pVbi_Fld->pCgmsbDatum)
	{
		return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
	}

	/* Return data as requested */
	for (subIndex = 0 ; subIndex < 5 ; ++subIndex)
		(*pDatum)[subIndex] = (*pVbi_Fld->pCgmsbDatum)[subIndex];
	BDBG_LEAVE(BVBI_Field_GetCGMSBData_isr);
	return eErr;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SetCGMSBData_isr(
	BVBI_Field_Handle vbiData, BVBI_CGMSB_Datum* pDatum)
{
	BVBI_P_Field_Handle* pVbi_Fld;
	int subIndex;

	BDBG_ENTER(BVBI_Field_SetCGMSBData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(vbiData, pVbi_Fld);
	if(!pVbi_Fld)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Check that field handle was properly sized */
	if (!pVbi_Fld->pCgmsbDatum)
	{
		return BERR_TRACE (BVBI_ERR_FLDH_CONFLICT);
	}

	/* Store data as requested */
	for (subIndex = 0 ; subIndex < 5 ; ++subIndex)
		(*pVbi_Fld->pCgmsbDatum)[subIndex] = (*pDatum)[subIndex];

	/* Indicate valid data is present */
	pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_CGMSB;

	BDBG_LEAVE(BVBI_Field_SetCGMSBData_isr);
	return BERR_SUCCESS;
}


/***************************************************************************
* Implementation of supporting CGMS functions that are not in API
***************************************************************************/


BERR_Code BVBI_P_CGMS_Init( BVBI_P_Handle *pVbi )
{
	uint8_t hwIndex;

	BDBG_ENTER(BVBI_P_CGMS_Init);

	/* Initialize CGMS encoders */
	for (hwIndex = 0 ; hwIndex < BVBI_NUM_CGMSAE ; ++hwIndex)
		BVBI_P_CGMS_Enc_Init (pVbi->hReg, false, hwIndex);
#if (BVBI_NUM_CGMSAE_656 > 0)
	for (hwIndex = 0 ; hwIndex < BVBI_NUM_CGMSAE_656 ; ++hwIndex)
		BVBI_P_CGMS_Enc_Init (pVbi->hReg, true, hwIndex);
#endif

	BDBG_LEAVE(BVBI_P_CGMS_Init);
	return BERR_SUCCESS;
}

uint32_t BVPI_P_CGMS_format_data_isr (uint32_t userdata)
{
	/* Do CRC calculation if hardware is broken */
	/* Otherwise, just transform from decoder format to encoder format. */
#ifdef P_CGMS_SOFTWARE_CRC
	userdata &= 0x00003fff;
	userdata = P_CalculateCRC (userdata);
	userdata <<= 2;
	userdata &= 0x003ffffc;
	userdata |= 0x00000001;
#else
	userdata <<= 2;
	userdata &= 0x0000fffc;
#endif

	return userdata;
}


/***************************************************************************
* Static (private) functions
***************************************************************************/


/* End of file */
