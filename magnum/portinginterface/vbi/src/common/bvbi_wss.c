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
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bkni.h"			/* For critical sections */
#include "bvbi_priv.h"      /* VBI internal data structures */

BDBG_MODULE(BVBI);

/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_GetWSSData_isr(
	BVBI_Field_Handle vbiData,
	uint16_t *pusWSSData
)
{
	BVBI_P_Field_Handle* pVbi_Fld;
	BERR_Code eErr = BERR_SUCCESS;

	BDBG_ENTER(BVBI_Field_GetWSSData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(vbiData, pVbi_Fld);
	if((!pVbi_Fld) || (!pusWSSData))
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Verify that data is present on this field handle */
	if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_WSS))
		return (BVBI_ERR_FIELD_NODATA);
	else if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_WSS_NOENCODE)
		eErr = (BVBI_ERR_FIELD_BADDATA);

	/* Return data as requested */
    *pusWSSData = pVbi_Fld->usWSSData;
	BDBG_LEAVE(BVBI_Field_GetWSSData_isr);
	return eErr;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SetWSSData_isr(
	BVBI_Field_Handle vbiData, uint16_t usWSSData )
{
	BVBI_P_Field_Handle* pVbi_Fld;

	BDBG_ENTER(BVBI_Field_SetWSSData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(vbiData, pVbi_Fld);
	if(!pVbi_Fld)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Store data as requested */
    pVbi_Fld->usWSSData = usWSSData;

	/* Indicate valid data is present */
	pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_WSS;

	BDBG_LEAVE(BVBI_Field_SetWSSData_isr);
	return BERR_SUCCESS;
}


/***************************************************************************
* Implementation of supporting WSS functions that are not in API
***************************************************************************/


BERR_Code BVBI_P_WSS_Init( BVBI_P_Handle *pVbi )
{
	uint8_t hwIndex;

	BDBG_ENTER(BVBI_P_WSS_Init);

	/* Initialize WSS encoders */
	for (hwIndex = 0 ; hwIndex < BVBI_NUM_WSE ; ++hwIndex)
		BVBI_P_WSS_Enc_Init (pVbi->hReg, hwIndex);
#if (BVBI_NUM_WSE_656 > 0)
	for (hwIndex = 0 ; hwIndex < BVBI_NUM_WSE_656 ; ++hwIndex)
		BVBI_P_WSS_656_Enc_Init (pVbi->hReg, hwIndex);
#endif

	/* This line of code only serves to shut up a compiler warning: */
	(void) BVBI_P_AddWSSparity (0);

	BDBG_LEAVE(BVBI_P_WSS_Init);
	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
uint16_t BVBI_P_AddWSSparity (uint16_t usData)
{
    uint16_t usOriginalData = usData;
    uint8_t uchParity       = 0;
	static const uint16_t mask = 0x0008;

	/* The computation only depends on the 3 LSbits */
	uchParity += (usData & 0x1);
	usData >>= 1;
	uchParity += (usData & 0x1);
	usData >>= 1;
	uchParity += (usData & 0x1);

	/* Debug code
	printf ("%04x (p%d) -> %04x\n",
		usOriginalData, uchParity,
		(uchParity & 0x1) ?
			(usOriginalData & ~mask) : (usOriginalData | mask));
	*/

	return
		(uchParity & 0x1) ?
			(usOriginalData & ~mask) : (usOriginalData | mask);
}


/***************************************************************************
* Static (private) functions
***************************************************************************/

/* End of file */
