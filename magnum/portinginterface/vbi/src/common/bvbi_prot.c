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
#include "bstd.h"                /* standard types */
#include "bdbg.h"                /* Dbglib */
#include "bvbi.h"                /* VBI processing, this module. */
#include "bvbi_priv.h"           /* VBI internal data structures */
#include "bvbi_prot.h"           /* VBI protected functions */

BDBG_MODULE(BVBI);

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/


/***************************************************************************
* Implementation of "BVBI_" protected functions
***************************************************************************/


/***************************************************************************
 *
 */
void BVBI_Field_Zero_UsageCount_isr (BVBI_Field_Handle fieldHandle)
{
	BVBI_P_Field_Handle *pVbi_Fld;

	BDBG_ENTER(BVBI_Field_Zero_UsageCount_isr);

	/* check parameter */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	BDBG_ASSERT (pVbi_Fld != NULL);

	pVbi_Fld->inUseCount = 0;

	BDBG_LEAVE(BVBI_Field_Zero_UsageCount_isr);
}


/***************************************************************************
 *
 */
void BVBI_Field_Increment_UsageCount_isr (BVBI_Field_Handle fieldHandle)
{
	BVBI_P_Field_Handle *pVbi_Fld;

	BDBG_ENTER(BVBI_Field_Increment_UsageCount_isr);

	/* check parameter */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	BDBG_ASSERT (pVbi_Fld != NULL);

	++(pVbi_Fld->inUseCount);

	BDBG_LEAVE(BVBI_Field_Increment_UsageCount_isr);
}


/***************************************************************************
 *
 */
void BVBI_Field_Decrement_UsageCount_isr (BVBI_Field_Handle fieldHandle)
{
	BVBI_P_Field_Handle *pVbi_Fld;

	BDBG_ENTER(BVBI_Field_Decrement_UsageCount_isr);

	/* check parameter */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	BDBG_ASSERT (pVbi_Fld != NULL);

	BDBG_ASSERT (pVbi_Fld->inUseCount > 0);
	--(pVbi_Fld->inUseCount);

	BDBG_LEAVE(BVBI_Field_Decrement_UsageCount_isr);
}


/***************************************************************************
 *
 */
int  BVBI_Field_Get_UsageCount_isr (BVBI_Field_Handle fieldHandle)
{
	BVBI_P_Field_Handle *pVbi_Fld;

	BDBG_ENTER(BVBI_Field_Get_UsageCount_isr);

	/* check parameter */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	BDBG_ASSERT (pVbi_Fld != NULL);

	BDBG_LEAVE(BVBI_Field_Get_UsageCount_isr);
	return pVbi_Fld->inUseCount;
}


/***************************************************************************
 *
 */
void BVBI_Field_ClearState_isr (BVBI_Field_Handle fieldHandle)
{
	BVBI_P_Field_Handle *pVbi_Fld;

	BDBG_ENTER(BVBI_Field_ClearState_isr);

	/* check parameter */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	BDBG_ASSERT (pVbi_Fld != NULL);

	/* Clear out simple attributes */
	pVbi_Fld->ulWhichPresent = 0x0;
	pVbi_Fld->ulErrInfo      = 0x0;
	pVbi_Fld->polarityMask   = 0x0;

	BDBG_LEAVE(BVBI_Field_ClearState_isr);
}

/* End of File */
