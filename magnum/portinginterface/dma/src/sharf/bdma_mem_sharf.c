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
 *
 * Revision History:
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "../common/bdma_mem_priv.h"    /*  */
#include "bdma_mem_sharf_priv.h"    /*  */
#include "bkni.h"


BDBG_MODULE(BDMA_MEM);

/* Default settings. */
static const BDMA_Mem_Settings s_stDefaultMemDmaSettings =
{
	BDMA_P_NUM_QUEUE_ENTRIES,
	BDMA_P_MAX_NUM_BLOCKS,

	/* deafult for the case that only one BDMA instance exists in the system */
	NULL, NULL, NULL, 0, true
};


/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Sharf_Create(
	BDMA_Handle              hDma,
	BDMA_Sharf               eEngine,
	BDMA_Mem_Settings *      pSettings,	
	BDMA_Mem_Handle *        phMemDma )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Context  *pDma;

	BDMA_P_MAIN_GET_CONTEXT( hDma, pDma );
	if (NULL == pDma)
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_Err_Mem_Create;
	}
	
	/* hDma passed the magic check, we trust every thing pointed by it now */
	eResult = BKNI_AcquireMutex(pDma->hMutex);
	if (BERR_SUCCESS == eResult)
	{
		if (NULL != pSettings)
			eResult = BDMA_P_Mem_Sharf_Create( hDma, eEngine, pSettings, phMemDma );
		else
			eResult = BDMA_P_Mem_Sharf_Create( hDma, eEngine, &s_stDefaultMemDmaSettings, phMemDma );
		BKNI_ReleaseMutex(pDma->hMutex);
	}
	
  BDMA_Err_Mem_Create:
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Sharf_Destroy(
	BDMA_Mem_Handle       hMemDma )
{
	BKNI_MutexHandle  hMutex;
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_AcquireMutex( hMemDma, &hMutex );
	if (BERR_SUCCESS == eResult)
	{
		eResult = BDMA_P_Mem_Sharf_Destroy( hMemDma );
		BKNI_ReleaseMutex(hMutex);
	}
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Sharf_Tran_SetCrypto(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_SharfMode           eSharfMode,
	bool                     bSgEnable,
	bool                     bUseBspKey,
	bool                     bCmp8LstByts)
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Mem_Sharf_Tran_SetCrypto_isr( hTran,
		eSharfMode, bSgEnable, bUseBspKey, bCmp8LstByts );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Sharf_Tran_SetSgStartEnd(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	bool                     bStartSgScram,
	bool                     bEndSgScram,
	bool                     bKeyPresent,
	bool                     bDigestPresent )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();	
	eResult = BDMA_P_Mem_Sharf_Tran_SetSgStartEnd_isr( hTran, ulBlockId,
		bStartSgScram, bEndSgScram, bKeyPresent, bDigestPresent );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Sharf_Tran_SetDmaBlockInfo(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	BDMA_Context             eContext,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();	
	eResult = BDMA_P_Mem_Sharf_Tran_SetDmaBlockInfo_isr( hTran, ulBlockId,
		eContext, ulDstBusAddr, ulSrcBusAddr, ulBlockSize );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Sharf_Tran_SetCrypto_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_SharfMode           eSharfMode,
	bool                     bSgEnable,
	bool                     bUseBspKey,
	bool                     bCmp8LstByts)
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_Sharf_Tran_SetCrypto_isr( hTran,
		eSharfMode, bSgEnable, bUseBspKey, bCmp8LstByts );
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Sharf_Tran_SetSgStartEnd_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	bool                     bStartSgScram,
	bool                     bEndSgScram,
	bool                     bKeyPresent,
	bool                     bDigestPresent )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_Sharf_Tran_SetSgStartEnd_isr( hTran, ulBlockId,
		bStartSgScram, bEndSgScram, bKeyPresent, bDigestPresent );
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Sharf_Tran_SetDmaBlockInfo_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	BDMA_Context             eContext,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_Sharf_Tran_SetDmaBlockInfo_isr( hTran, ulBlockId,
		eContext, ulDstBusAddr, ulSrcBusAddr, ulBlockSize );
		
	return BERR_TRACE(eResult);
}


/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Sharf_Tran_Start(
	BDMA_Mem_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Mem_Sharf_Tran_Start_isr(
		hTran, hTran->QueueEntry.ulNumTranBlocks, NULL, NULL );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}


/* End of File */
