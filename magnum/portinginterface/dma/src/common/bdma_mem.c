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
 *
 * Revision History:
 * $brcm_Log: $
 * 
 ***************************************************************************/


#include "bdma_mem_priv.h"    /*  */
#include "bkni.h"

BDBG_MODULE(BDMA_MEM);

/***************************************************************************
 * Implementation Overview:
 *
 * The corresponding API functions of both interrupt handler (or critical 
 * session) and non-interrupt handler context version call the same 
 * corresponding private function to do the real job. However, a non-
 * interrupt handler context version API function enters critical session
 * before calling its private function, and leave critical session afterwards.
 * But interrupt handler is already not interruptable, a interrupt handler
 * context version of API functions call their private functions without
 * entering critical session
 ****************************************************************************/

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
BERR_Code BDMA_Mem_GetDefaultSettings
	( BDMA_Mem_Settings     *pDefSettings )
{
	if (NULL != pDefSettings)
	{
		*pDefSettings = s_stDefaultMemDmaSettings;
		return BERR_SUCCESS;
	}
	else
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);		
	}
}


/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_SetByteSwapMode(
	BDMA_Mem_Handle          hMemDma,
	BDMA_Endian              eReadEndian,
	BDMA_SwapMode            eSwapMode )
{
	BERR_Code  eResult = BERR_SUCCESS;

	/*BKNI_EnterCriticalSection();*/
	eResult = BDMA_P_Mem_SetByteSwapMode_isr( hMemDma, eReadEndian, eSwapMode );
	/*BKNI_LeaveCriticalSection();*/
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_Create2(
	BDMA_Mem_Handle          hMemDma,
	uint32_t                 ulNumBlocks,
	bool                     bCachedDesc,
	BDMA_Mem_Tran_Handle  *  phTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();	
	eResult = BDMA_P_Mem_Tran_Create_isr( hMemDma, ulNumBlocks, bCachedDesc, phTran );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_Create2_isr(
	BDMA_Mem_Handle          hMemDma,
	uint32_t                 ulNumBlocks,
	bool                     bCachedDesc,
	BDMA_Mem_Tran_Handle  *  phTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_Tran_Create_isr( hMemDma, ulNumBlocks, bCachedDesc, phTran );
		
	return BERR_TRACE(eResult);
}


/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_Create(
	BDMA_Mem_Handle          hMemDma,
	uint32_t                 ulNumBlocks,
	BDMA_Mem_Tran_Handle  *  phTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();	
	eResult = BDMA_P_Mem_Tran_Create_isr( hMemDma, ulNumBlocks, false, phTran );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_Create_isr(
	BDMA_Mem_Handle          hMemDma,
	uint32_t                 ulNumBlocks,
	BDMA_Mem_Tran_Handle  *  phTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_Tran_Create_isr( hMemDma, ulNumBlocks, false, phTran );
		
	return BERR_TRACE(eResult);
}


/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_GetMaxNumBlocks(
	BDMA_Mem_Handle          hMemDma,
	uint32_t *               pulMaxNumBlocks )
{
	BERR_Code  eResult = BERR_SUCCESS;

	if ( (NULL == hMemDma) || (NULL == pulMaxNumBlocks) )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_Done_Mem_Tran_GetMaxNumBlocks;
	}
	*pulMaxNumBlocks = BDMA_P_MAX_NUM_BLOCKS;

  BDMA_Done_Mem_Tran_GetMaxNumBlocks:
	
	return BERR_TRACE(eResult);
}



/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_GetStatus(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_TranStatus *        peTranStatus )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Mem_Tran_GetStatus_isr( hTran, peTranStatus );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
 
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_Destroy(
	BDMA_Mem_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Mem_Tran_Destroy_isr( hTran );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_Reset(
	BDMA_Mem_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	/*BKNI_EnterCriticalSection();*/
	eResult = BDMA_P_Mem_Tran_Reset_isr( hTran );
	/*BKNI_LeaveCriticalSection();*/
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_GetStatus_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_TranStatus *        peTranStatus )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_Tran_GetStatus_isr( hTran, peTranStatus );
		
	return BERR_TRACE(eResult);
 
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_Destroy_isr(
	BDMA_Mem_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_Tran_Destroy_isr( hTran );
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_Reset_isr(
	BDMA_Mem_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_Tran_Reset_isr( hTran );
		
	return BERR_TRACE(eResult);
}


#if BDMA_P_SUPPORT_MEM_DMA_ENGINE

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Create2(
	BDMA_Handle           hDma,
	BDMA_MemDmaEngine     eEngine,
	BDMA_Mem_Settings *   pSettings,	
	BDMA_Mem_Handle *     phMemDma )
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
			eResult = BDMA_P_Mem_Create( hDma, eEngine, pSettings, phMemDma );
		else
			eResult = BDMA_P_Mem_Create( hDma, eEngine, &s_stDefaultMemDmaSettings, phMemDma );
		BKNI_ReleaseMutex(pDma->hMutex);
	}

  BDMA_Err_Mem_Create:
	return BERR_TRACE(eResult);
}


/***************************************************************************
 * Obsolete Memory Dma API: 
 **************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Create(
	BDMA_Handle           hDma,
	BDMA_Mem_Handle *     phMemDma )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Context  *pDma;
	BDMA_P_Mem_Context  *pMemDma = NULL;
	BDMA_MemDmaEngine  eEngine;
	
	BDMA_P_MAIN_GET_CONTEXT( hDma, pDma );
	if (NULL == pDma)
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_Err_Mem_Create;
	}
	
	/* hDma passed the magic check, we trust every thing pointed by it now */
	*phMemDma = NULL;
	eResult = BKNI_AcquireMutex(pDma->hMutex);
	if (BERR_SUCCESS == eResult)
	{
		for (eEngine = BDMA_MemDmaEngine_e0; eEngine < BDMA_P_SUPPORT_MEM_DMA_ENGINE; eEngine ++)
		{
			pMemDma = BDMA_P_GetMemDmaHandle( hDma, eEngine );
			if (NULL == pMemDma)
				break;
		}
		
		if ( NULL == pMemDma )
		{
			/* eEngine is not used yet */
			eResult = BDMA_P_Mem_Create( hDma, eEngine, &s_stDefaultMemDmaSettings, phMemDma );
		}
		else
		{
			/* all mem dma engine are used */
			eResult = BDMA_ERR_ENGINE_OCCUPIED;
		}
		BKNI_ReleaseMutex(pDma->hMutex);
	}

  BDMA_Err_Mem_Create:
	return BERR_TRACE(eResult);
}



/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Destroy(
	BDMA_Mem_Handle       hMemDma )
{
	BKNI_MutexHandle  hMutex;
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_AcquireMutex( hMemDma, &hMutex );
	if (BERR_SUCCESS == eResult)
	{
		eResult = BDMA_P_Mem_Destroy( hMemDma );
		BKNI_ReleaseMutex(hMutex);
	}
		
	return BERR_TRACE(eResult);
}



/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_SetCrypto(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_CryptMode           eCryptMode,
	uint32_t                 ulKeySlot,
	bool                     bSgEnable )
{
	BERR_Code  eResult = BERR_SUCCESS;

	/*BKNI_EnterCriticalSection();*/
	eResult = BDMA_P_Mem_Tran_SetCrypto_isr( hTran, eCryptMode, ulKeySlot, bSgEnable );
	/*BKNI_LeaveCriticalSection();*/
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_SetCrypto_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_CryptMode           eCryptMode,
	uint32_t                 ulKeySlot,
	bool                     bSgEnable )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_Tran_SetCrypto_isr( hTran, eCryptMode, ulKeySlot, bSgEnable );
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_SetCrypt(
	BDMA_Mem_Handle          hMemDma,
	BDMA_CryptMode           eCryptMode,
	uint32_t                 ulKeySlot )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Mem_SetCrypto_isr( hMemDma, eCryptMode, ulKeySlot, false );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_SetCrypto(
	BDMA_Mem_Handle          hMemDma,
	BDMA_CryptMode           eCryptMode,
	uint32_t                 ulKeySlot,
	bool                     bSgEnable )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Mem_SetCrypto_isr( hMemDma, eCryptMode, ulKeySlot, bSgEnable );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}


/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_SetDmaBlockInfo(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize,
	bool                     bCryptInit )
{
	BERR_Code  eResult = BERR_SUCCESS;

	/*BKNI_EnterCriticalSection();*/
	eResult = BDMA_P_Mem_Tran_SetDmaBlockInfo_isr( hTran, ulBlockId,
											ulDstBusAddr, ulSrcBusAddr,
											ulBlockSize, bCryptInit );
	/*BKNI_LeaveCriticalSection();*/
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_SetDmaBlockInfo_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize,
	bool                     bCryptInit )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_Tran_SetDmaBlockInfo_isr( hTran, ulBlockId,
											ulDstBusAddr, ulSrcBusAddr,
											ulBlockSize, bCryptInit );
		
	return BERR_TRACE(eResult);
}


/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_SetSgStartEnd(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	bool                     bSgStart,
	bool                     bSgEnd )
{
	BERR_Code  eResult = BERR_SUCCESS;

	/*BKNI_EnterCriticalSection();*/
	eResult = BDMA_P_Mem_Tran_SetSgStartEnd_isr( hTran, ulBlockId,
												 bSgStart, bSgEnd );
	/*BKNI_LeaveCriticalSection();*/
		
	return BERR_TRACE(eResult);
}


/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_SetSgStartEnd_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	bool                     bSgStart,
	bool                     bSgEnd )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_Tran_SetSgStartEnd_isr( hTran, ulBlockId,
												 bSgStart, bSgEnd );
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Transfer(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize,
	bool                     bCryptInit,
	BDMA_Mem_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Mem_Transfer_isr( hTran, 
		ulBlockId, ulDstBusAddr, ulSrcBusAddr, ulBlockSize, bCryptInit, 
		pCallBackFunc_isr, pUserCallBackParam );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);

}



/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_StartAndCallBack_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_Mem_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_Tran_Start_isr( hTran, hTran->QueueEntry.ulNumTranBlocks,
										 pCallBackFunc_isr, pUserCallBackParam );
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_Start(
	BDMA_Mem_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Mem_Tran_Start_isr(
		hTran, hTran->QueueEntry.ulNumTranBlocks, NULL, NULL );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_StartAndCallBack(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_Mem_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Mem_Tran_Start_isr( hTran, hTran->QueueEntry.ulNumTranBlocks,
										 pCallBackFunc_isr, pUserCallBackParam );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_StartDma(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Mem_Tran_Start_isr( hTran, ulNumActBlocks, NULL, NULL );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */

BERR_Code BDMA_Mem_Tran_StartDmaSubset(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulFirstBlock,
	uint32_t                 ulNumActBlocks )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Mem_Tran_StartSubset_isr( hTran, ulFirstBlock, ulNumActBlocks, NULL, NULL );
	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);
}
/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_StartDmaAndCallBack(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,
	BDMA_Mem_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Mem_Tran_Start_isr( hTran, ulNumActBlocks, pCallBackFunc_isr, pUserCallBackParam );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_StartDmaSubsetAndCallBack(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulFirstBlock,
	uint32_t                 ulNumActBlocks,
	BDMA_Mem_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Mem_Tran_StartSubset_isr( hTran, ulFirstBlock, ulNumActBlocks, pCallBackFunc_isr, pUserCallBackParam );
	BKNI_LeaveCriticalSection();
		
	return BERR_TRACE(eResult);
}


/***************************************************************************
 *
 */
BERR_Code BDMA_Mem_Tran_StartDmaAndCallBack_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,
	BDMA_Mem_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Mem_Tran_Start_isr( hTran, ulNumActBlocks, pCallBackFunc_isr, pUserCallBackParam );
		
	return BERR_TRACE(eResult);
}

#endif






