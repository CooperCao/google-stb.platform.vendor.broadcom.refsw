/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#include "bkni.h"                /* malloc */

#include "bmem.h"
#include "bvdc.h"                /* Video display */
#include "bvdc_bufferheap_priv.h"
#include "bvdc_priv.h"

BDBG_MODULE(BVDC_HEAP);

/***************************************************************************
 *
 */
BERR_Code BVDC_Heap_Create
	( BVDC_Handle                       hVdc,
	  BVDC_Heap_Handle                 *phHeap,
	  BMMA_Heap_Handle                  hMem,
	  const BVDC_Heap_Settings         *pSettings )
{
	BERR_Code err = BERR_SUCCESS;

	BDBG_ENTER(BVDC_Heap_Create);
	BDBG_ASSERT(hVdc);
	BDBG_ASSERT(hMem);
	BDBG_ASSERT(pSettings);

	err = BVDC_P_CheckHeapSettings(pSettings);
	if( err != BERR_SUCCESS )
	{
		return BERR_TRACE(err);
	}
	err = BVDC_P_BufferHeap_Create(hVdc, phHeap, hMem, pSettings);
	if (err != BERR_SUCCESS)
		return BERR_TRACE(err);

	BDBG_LEAVE(BVDC_Heap_Create);
	return BERR_SUCCESS;

}

/***************************************************************************
 *
 */
BERR_Code BVDC_Heap_Destroy
	( BVDC_Heap_Handle                 hHeap)
{
	BERR_Code err = BERR_SUCCESS;

	BDBG_ENTER(BVDC_Heap_Destroy);
	BDBG_ASSERT(hHeap);

	err = BVDC_P_BufferHeap_Destroy(hHeap);
	if (err != BERR_SUCCESS)
		return err;

	BDBG_LEAVE(BVDC_Heap_Destroy);
	return BERR_SUCCESS;
}

