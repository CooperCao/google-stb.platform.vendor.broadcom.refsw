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
#if (BCHP_CHIP != 3548) && (BCHP_CHIP != 3556) && (BCHP_CHIP != 7630) && (BCHP_CHIP != 7640) && (BCHP_CHIP != 7468) \
    && (BCHP_CHIP != 7408) && (BCHP_CHIP != 7422) && (BCHP_CHIP != 7425) \
    && (BCHP_CHIP != 7358) && (BCHP_CHIP != 7552) \
    && (BCHP_CHIP != 7231) && (BCHP_CHIP != 7344) && (BCHP_CHIP != 7346) \
    && (BCHP_CHIP != 7550) && (BCHP_CHIP != 73465)
#include "bdma_pci_priv.h"
#include "bkni.h"

BDBG_MODULE(BDMA_PCI);

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
static const BDMA_Pci_Settings s_stDefaultPciDmaSettings =
{
	BDMA_P_NUM_QUEUE_ENTRIES,
	BDMA_P_MAX_NUM_BLOCKS,

	/* deafult for the case that only one BDMA instance exists in the system */
	NULL, NULL, NULL, 0, true
};

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Create2(
	BDMA_Handle           hDma,
	BDMA_PciDmaEngine     eEngine,
	BDMA_Pci_Settings *   pSettings,
	BDMA_Pci_Handle *     phPciDma )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Context  *pDma;

	BDMA_P_MAIN_GET_CONTEXT( hDma, pDma );
	if (NULL == pDma)
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_Err_Pci_Create;
	}

	/* hDma passed the magic check, we trust every thing pointed by it now */
	eResult = BKNI_AcquireMutex(pDma->hMutex);
	if (BERR_SUCCESS == eResult)
	{
		if (NULL != pSettings)
			eResult = BDMA_P_Pci_Create( hDma, eEngine, pSettings, phPciDma );
		else
			eResult = BDMA_P_Pci_Create( hDma, eEngine, &s_stDefaultPciDmaSettings, phPciDma );
		BKNI_ReleaseMutex(pDma->hMutex);
	}

  BDMA_Err_Pci_Create:
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Destroy(
	BDMA_Pci_Handle       hPciDma )
{
	BKNI_MutexHandle  hMutex;
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Pci_AcquireMutex(hPciDma, &hMutex );
	if (BERR_SUCCESS == eResult)
	{
		eResult = BDMA_P_Pci_Destroy( hPciDma );
		BKNI_ReleaseMutex(hMutex);
	}

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_GetDefaultSettings
	( BDMA_Pci_Settings     *pDefSettings )
{
	if (NULL != pDefSettings)
	{
		*pDefSettings = s_stDefaultPciDmaSettings;
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
BERR_Code BDMA_Pci_SetSwapMode(
	BDMA_Pci_Handle       hPciDma,
	BDMA_SwapMode         eSwapMode )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Pci_SetSwapMode_isr( hPciDma, eSwapMode );
	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_SetMaxBurstSize(
	BDMA_Pci_Handle       hPciDma,
	BDMA_MaxBurstSize     eMaxBurstSize )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Pci_SetMaxBurstSize_isr( hPciDma, eMaxBurstSize );
	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Create2(
	BDMA_Pci_Handle          hPciDma,
	uint32_t                 ulNumBlocks,
	bool                     bCachedDesc,
	BDMA_Pci_Tran_Handle  *  phTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Pci_Tran_Create_isr( hPciDma, ulNumBlocks, bCachedDesc, phTran );

	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_GetMaxNumBlocks(
	BDMA_Pci_Handle          hPciDma,
	uint32_t *               pulMaxNumBlocks )
{
	BERR_Code  eResult = BERR_SUCCESS;

	if ( (NULL == hPciDma) || (NULL == pulMaxNumBlocks) )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_Done_Pci_Tran_GetMaxNumBlocks;
	}
	*pulMaxNumBlocks = BDMA_P_MAX_NUM_BLOCKS;

  BDMA_Done_Pci_Tran_GetMaxNumBlocks:

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_SetBlockInfo(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulMemBusAddr,
	uint32_t                 ulPciBusAddr,
	uint32_t                 ulBlockSize,
	BDMA_TranDir             eTranDirection )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Pci_Tran_SetBlockInfo_isr( hTran, ulBlockId,
											ulMemBusAddr, ulPciBusAddr,
											ulBlockSize,
											eTranDirection );
	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_StartDma(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Pci_Tran_Start_isr( hTran, ulNumActBlocks, NULL, NULL );
	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */

BERR_Code BDMA_Pci_Tran_StartDmaSubset(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulFirstBlock,
	uint32_t                 ulNumActBlocks )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Pci_Tran_StartSubset_isr( hTran, ulFirstBlock, ulNumActBlocks, NULL, NULL );
	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_StartDmaAndCallBack(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,
	BDMA_Pci_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Pci_Tran_Start_isr( hTran, ulNumActBlocks, pCallBackFunc_isr, pUserCallBackParam );
	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_GetStatus(
	BDMA_Pci_Tran_Handle     hTran,
	BDMA_TranStatus *        peTranStatus )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Pci_Tran_GetStatus_isr( hTran, peTranStatus );
	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);

}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Destroy(
	BDMA_Pci_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Pci_Tran_Destroy_isr( hTran );
	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Reset(
	BDMA_Pci_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Pci_Tran_Reset_isr( hTran );
	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Create2_isr(
	BDMA_Pci_Handle          hPciDma,
	uint32_t                 ulNumBlocks,
	bool                     bCachedDesc,
	BDMA_Pci_Tran_Handle  *  phTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Pci_Tran_Create_isr( hPciDma, ulNumBlocks, bCachedDesc, phTran );

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_SetBlockInfo_isr(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulMemBusAddr,
	uint32_t                 ulPciBusAddr,
	uint32_t                 ulBlockSize,
	BDMA_TranDir             eTranDirection )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Pci_Tran_SetBlockInfo_isr( hTran, ulBlockId,
											ulMemBusAddr, ulPciBusAddr,
											ulBlockSize,
											eTranDirection );

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_StartDmaAndCallBack_isr(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,
	BDMA_Pci_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Pci_Tran_Start_isr( hTran, ulNumActBlocks, pCallBackFunc_isr, pUserCallBackParam );

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_GetStatus_isr(
	BDMA_Pci_Tran_Handle     hTran,
	BDMA_TranStatus *        peTranStatus )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Pci_Tran_GetStatus_isr( hTran, peTranStatus );

	return BERR_TRACE(eResult);

}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Destroy_isr(
	BDMA_Pci_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Pci_Tran_Destroy_isr( hTran );

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Reset_isr(
	BDMA_Pci_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Pci_Tran_Reset_isr( hTran );

	return BERR_TRACE(eResult);
}


/***************************************************************************
 * Obsolete Pci Dma API:
 **************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Create(
	BDMA_Handle           hDma,
	BDMA_Pci_Handle *     phPciDma )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Context  *pDma;
	BDMA_P_Pci_Context  *pPciDma = NULL;
	BDMA_PciDmaEngine  eEngine;

	BDMA_P_MAIN_GET_CONTEXT( hDma, pDma );
	if (NULL == pDma)
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_Err_Pci_Create;
	}

	/* hDma passed the magic check, we trust every thing pointed by it now */
	*phPciDma = NULL;
	eResult = BKNI_AcquireMutex(pDma->hMutex);
	if (BERR_SUCCESS == eResult)
	{
		for (eEngine = BDMA_PciDmaEngine_e0; eEngine < BDMA_P_SUPPORT_PCI_DMA_ENGINE; eEngine ++)
		{
			pPciDma = BDMA_P_GetPciDmaHandle( hDma, eEngine );
			if (NULL == pPciDma)
				break;
		}

		if ( NULL == pPciDma )
		{
			/* eEngine is not used yet */
			eResult = BDMA_P_Pci_Create( hDma, eEngine, &s_stDefaultPciDmaSettings, phPciDma );
		}
		else
		{
			/* all pci dma engine are used */
			eResult = BDMA_ERR_ENGINE_OCCUPIED;
		}
		BKNI_ReleaseMutex(pDma->hMutex);
	}

  BDMA_Err_Pci_Create:
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Create(
	BDMA_Pci_Handle          hPciDma,
	uint32_t                 ulNumBlocks,
	BDMA_Pci_Tran_Handle  *  phTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Pci_Tran_Create_isr( hPciDma, ulNumBlocks, false, phTran );
	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Start(
	BDMA_Pci_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Pci_Tran_Start_isr(
		hTran, hTran->QueueEntry.ulNumTranBlocks, NULL, NULL );
	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_StartAndCallBack(
	BDMA_Pci_Tran_Handle     hTran,
	BDMA_Pci_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BKNI_EnterCriticalSection();
	eResult = BDMA_P_Pci_Tran_Start_isr( hTran, hTran->QueueEntry.ulNumTranBlocks,
										 pCallBackFunc_isr, pUserCallBackParam );
	BKNI_LeaveCriticalSection();

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Create_isr(
	BDMA_Pci_Handle          hPciDma,
	uint32_t                 ulNumBlocks,
	BDMA_Pci_Tran_Handle  *  phTran )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Pci_Tran_Create_isr( hPciDma, ulNumBlocks, false, phTran );

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_StartAndCallBack_isr(
	BDMA_Pci_Tran_Handle     hTran,
	BDMA_Pci_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;

	eResult = BDMA_P_Pci_Tran_Start_isr( hTran, hTran->QueueEntry.ulNumTranBlocks,
										 pCallBackFunc_isr, pUserCallBackParam );

	return BERR_TRACE(eResult);
}


#else /*of #if (BCHP_CHIP != 3548) && (BCHP_CHIP != 3556)*/

#include "bdma.h"
#include "bdma_errors.h"
#include "bkni.h"

BDBG_MODULE(BDMA_PCI);

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Create2(
	BDMA_Handle           hDma,
	BDMA_PciDmaEngine     eEngine,
	BDMA_Pci_Settings *   pSettings,
	BDMA_Pci_Handle *     phPciDma )
{
	BSTD_UNUSED(hDma);
	BSTD_UNUSED(eEngine);
	BSTD_UNUSED(pSettings);
	BSTD_UNUSED(phPciDma);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Destroy(
	BDMA_Pci_Handle       hPciDma )
{
	BSTD_UNUSED(hPciDma);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_GetDefaultSettings
	( BDMA_Pci_Settings     *pDefSettings )
{
	BSTD_UNUSED(pDefSettings);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_SetSwapMode(
	BDMA_Pci_Handle       hPciDma,
	BDMA_SwapMode         eSwapMode )
{
	BSTD_UNUSED(hPciDma);
	BSTD_UNUSED(eSwapMode);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_SetMaxBurstSize(
	BDMA_Pci_Handle       hPciDma,
	BDMA_MaxBurstSize     eMaxBurstSize )
{
	BSTD_UNUSED(hPciDma);
	BSTD_UNUSED(eMaxBurstSize);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Create2(
	BDMA_Pci_Handle          hPciDma,
	uint32_t                 ulNumBlocks,
	bool                     bCachedDesc,
	BDMA_Pci_Tran_Handle  *  phTran )
{
	BSTD_UNUSED(hPciDma);
	BSTD_UNUSED(ulNumBlocks);
	BSTD_UNUSED(bCachedDesc);
	BSTD_UNUSED(phTran);

return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_GetMaxNumBlocks(
	BDMA_Pci_Handle          hPciDma,
	uint32_t *               pulMaxNumBlocks )
{
	BSTD_UNUSED(hPciDma);
	BSTD_UNUSED(pulMaxNumBlocks);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_SetBlockInfo(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulMemBusAddr,
	uint32_t                 ulPciBusAddr,
	uint32_t                 ulBlockSize,
	BDMA_TranDir             eTranDirection )
{
	BSTD_UNUSED(hTran);
	BSTD_UNUSED(ulBlockId);
	BSTD_UNUSED(ulMemBusAddr);
	BSTD_UNUSED(ulPciBusAddr);
	BSTD_UNUSED(ulBlockSize);
	BSTD_UNUSED(eTranDirection);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_StartDma(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks )
{
	BSTD_UNUSED(hTran);
	BSTD_UNUSED(ulNumActBlocks);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_StartDmaAndCallBack(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,
	BDMA_Pci_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BSTD_UNUSED(hTran);
	BSTD_UNUSED(ulNumActBlocks);
	BSTD_UNUSED(pCallBackFunc_isr);
	BSTD_UNUSED(pUserCallBackParam);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_GetStatus(
	BDMA_Pci_Tran_Handle     hTran,
	BDMA_TranStatus *        peTranStatus )
{
	BSTD_UNUSED(hTran);
	BSTD_UNUSED(peTranStatus);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Destroy(
	BDMA_Pci_Tran_Handle     hTran )
{
	BSTD_UNUSED(hTran);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Reset(
	BDMA_Pci_Tran_Handle     hTran )
{
	BSTD_UNUSED(hTran);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Create2_isr(
	BDMA_Pci_Handle          hPciDma,
	uint32_t                 ulNumBlocks,
	bool                     bCachedDesc,
	BDMA_Pci_Tran_Handle  *  phTran )
{
	BSTD_UNUSED(hPciDma);
	BSTD_UNUSED(ulNumBlocks);
	BSTD_UNUSED(bCachedDesc);
	BSTD_UNUSED(phTran);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_SetBlockInfo_isr(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulMemBusAddr,
	uint32_t                 ulPciBusAddr,
	uint32_t                 ulBlockSize,
	BDMA_TranDir             eTranDirection )
{
	BSTD_UNUSED(hTran);
	BSTD_UNUSED(ulBlockId);
	BSTD_UNUSED(ulMemBusAddr);
	BSTD_UNUSED(ulPciBusAddr);
	BSTD_UNUSED(ulBlockSize);
	BSTD_UNUSED(eTranDirection);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_StartDmaAndCallBack_isr(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,
	BDMA_Pci_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BSTD_UNUSED(hTran);
	BSTD_UNUSED(ulNumActBlocks);
	BSTD_UNUSED(pCallBackFunc_isr);
	BSTD_UNUSED(pUserCallBackParam);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_GetStatus_isr(
	BDMA_Pci_Tran_Handle     hTran,
	BDMA_TranStatus *        peTranStatus )
{
	BSTD_UNUSED(hTran);
	BSTD_UNUSED(peTranStatus);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);

}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Destroy_isr(
	BDMA_Pci_Tran_Handle     hTran )
{
	BSTD_UNUSED(hTran);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Reset_isr(
	BDMA_Pci_Tran_Handle     hTran )
{
	BSTD_UNUSED(hTran);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}


/***************************************************************************
 * Obsolete Pci Dma API:
 **************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Create(
	BDMA_Handle           hDma,
	BDMA_Pci_Handle *     phPciDma )
{
	BSTD_UNUSED(hDma);
	BSTD_UNUSED(phPciDma);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Create(
	BDMA_Pci_Handle          hPciDma,
	uint32_t                 ulNumBlocks,
	BDMA_Pci_Tran_Handle  *  phTran )
{
	BSTD_UNUSED(hPciDma);
	BSTD_UNUSED(ulNumBlocks);
	BSTD_UNUSED(phTran);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Start(
	BDMA_Pci_Tran_Handle     hTran )
{
	BSTD_UNUSED(hTran);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_StartAndCallBack(
	BDMA_Pci_Tran_Handle     hTran,
	BDMA_Pci_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BSTD_UNUSED(hTran);
	BSTD_UNUSED(pCallBackFunc_isr);
	BSTD_UNUSED(pUserCallBackParam);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_Create_isr(
	BDMA_Pci_Handle          hPciDma,
	uint32_t                 ulNumBlocks,
	BDMA_Pci_Tran_Handle  *  phTran )
{
	BSTD_UNUSED(hPciDma);
	BSTD_UNUSED(ulNumBlocks);
	BSTD_UNUSED(phTran);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Pci_Tran_StartAndCallBack_isr(
	BDMA_Pci_Tran_Handle     hTran,
	BDMA_Pci_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BSTD_UNUSED(hTran);
	BSTD_UNUSED(pCallBackFunc_isr);
	BSTD_UNUSED(pUserCallBackParam);
	return BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
}

#endif

/* End of File */
