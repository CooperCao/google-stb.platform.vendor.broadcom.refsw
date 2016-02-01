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

#include "bdma_queue.h"
#include "bkni.h"

BDBG_MODULE(BDMA);


/***************************************************************************
 *
 * Debug functions
 * 
 *************************************************************************/

#define BDMA_P_QUEUE_WRAP_INDEX(index) \
	((index < pQueue->CmnSettings.ulTotalNumBlocks)? index: 0)

#if ((BDBG_DEBUG_BUILD) && (BDMA_P_CHECK_CORRUPT))
#define BDMA_P_QUEUE_ASSERT_UNCORRUPT(pQueue) \
	BDMA_P_Queue_CheckQCorruption_isr(pQueue)
#else
#define BDMA_P_QUEUE_ASSERT_UNCORRUPT(pQueue)
#endif 

#if ((BDBG_DEBUG_BUILD) && (BDMA_P_CHECK_NEXT_DESC))
#define BDMA_P_QUEUE_CHECK_NEXT_DESC(pQueue) \
	BDMA_P_Queue_CheckNextDescInHw_isr(pQueue)
#else
#define BDMA_P_QUEUE_CHECK_NEXT_DESC(pQueue)
#endif 

/*--------------------------------------------------------------------------
 * {private}
 * BDMA_P_Queue_CheckQCorruption_isr
 *
 * To be called to assert that the queue is not corrupted.
 * It should be called only if we are debugging or if some fetal error happened
*/
static void BDMA_P_Queue_CheckQCorruption_isr
	( BDMA_P_QueueContext *    pQueue )
{
	uint32_t ulLastAssignedId;
	uint32_t ulTranEntry;    /* Entry index of the Tran's 1st Entry */
	uint32_t ulBlockEntry;   /* Entry index of the block */
	BDMA_P_QueueEntry *  pTranEntry;  /* Entry ptr of the Tran's 1st Entry */
	BDMA_P_QueueEntry *  pBlockEntry; /* Entry ptr of the block */
	BDMA_P_QueueEntry *  pHeadTranInHw;
	BDMA_P_QueueEntry *  pTailTranInHw;
	uint32_t ulNumBlocks, ulNumQueueEntries, ulMaxNumBlkPerTran;
	uint32_t ulBlockEntryLimit;
	bool  bSee1stUnFinished = false;
	bool  bSeeLastIssued = false;
	bool  bSeeLastAssigned = false;
	
	BDBG_ASSERT( NULL != pQueue );
	
	ulLastAssignedId = pQueue->ulIndx_LastEntryAssigned;
	ulNumQueueEntries = pQueue->CmnSettings.ulTotalNumBlocks;
	ulMaxNumBlkPerTran = pQueue->CmnSettings.ulMaxNumBlocksPerTran;
	pHeadTranInHw = pQueue->pHeadTranInHw;
	pTailTranInHw = pQueue->pTailTranInHw;
	
	BDBG_ASSERT( ulLastAssignedId < ulNumQueueEntries );

	ulTranEntry = 0;
	while ( ulTranEntry < ulNumQueueEntries )
	{
		/* assert for the first block of the Tran */
		pTranEntry = pQueue->pHeadEntry + ulTranEntry;
		ulNumBlocks = pTranEntry->ulNumTranBlocks;
		BDBG_ASSERT( (0 < ulNumBlocks) && (ulNumBlocks <= ulMaxNumBlkPerTran) );

		/* assert for all remaining blocks of the Tran */
		ulBlockEntryLimit = ulTranEntry + ulNumBlocks; /* 1st block outside of the Tran */
		for ( ulBlockEntry = ulTranEntry+1; ulBlockEntry < ulBlockEntryLimit; ulBlockEntry ++ )
		{
			pBlockEntry = pQueue->pHeadEntry + ulBlockEntry;
			BDBG_ASSERT( 1 == pBlockEntry->ulNumTranBlocks );
			BDBG_ASSERT( BDMA_TranStatus_eUnknown == pBlockEntry->eStatus );
		}

		/* we will assert if pTailTranInHw, pHeadTranInHw
		 * and ulLastAssignedId are found */
		if ( ulTranEntry == ulLastAssignedId )
			bSeeLastAssigned = true;
		if ( pTranEntry == pTailTranInHw )
			bSeeLastIssued = true;
		if ( pTranEntry == pHeadTranInHw ) 
			bSee1stUnFinished = true;

		/* prepare for next pass of the loop */
		ulTranEntry = ulBlockEntry;
	}

	BDBG_ASSERT( ulTranEntry == ulNumQueueEntries );
	BDBG_ASSERT( bSeeLastAssigned );
	BDBG_ASSERT( (bSeeLastIssued)    || (NULL == pTailTranInHw) );
	BDBG_ASSERT( (bSee1stUnFinished) || (NULL == pHeadTranInHw) );
}


/*--------------------------------------------------------------------------
 * {private}
 * BDMA_P_Queue_CheckNextDescInHw_isr
 *
 * To be called to assert that the queue is not corrupted.
 * It should be called only if we are debugging or if some fetal error happened
*/
static void BDMA_P_Queue_CheckNextDescInHw_isr
	( BDMA_P_QueueContext *    pQueue )
{
	uint32_t ulTranEntry;    /* Entry index of the Tran's 1st Entry */
	BDMA_P_QueueEntry *  pTranEntry;  /* Entry ptr of the Tran's 1st Entry */
	BDMA_P_QueueEntry *  pHeadTranInHw;
	BDMA_P_QueueEntry *  pTailTranInHw;
	uint32_t ulNumQueueEntries;
	bool  bSeeLastIssued = false;
	
	BDBG_ASSERT( NULL != pQueue );
	
	ulNumQueueEntries = pQueue->CmnSettings.ulTotalNumBlocks;
	pHeadTranInHw = pQueue->pHeadTranInHw;
	pTailTranInHw = pQueue->pTailTranInHw;
	
	/* assert relation of pHeadTranInHw, and pTailTranInHw */
	BDBG_ASSERT( (NULL != pHeadTranInHw) == (NULL != pTailTranInHw) );
	if ( NULL != pHeadTranInHw )
	{
		/* assert pTailTranInHw IS linked to pHeadTranInHw */
		bSeeLastIssued = false;
		ulTranEntry = 0;
		pTranEntry = pHeadTranInHw;
		while ( ulTranEntry < ulNumQueueEntries )
		{
			if ( pTranEntry == pTailTranInHw )
			{
				(*pQueue->pCheckNextDesc_isr)(pTranEntry, NULL);
				bSeeLastIssued = true;
				break;
			}
			else
			{
				(*pQueue->pCheckNextDesc_isr)(pTranEntry, pTranEntry->pNextTran);
								
				if (NULL == pTranEntry->pNextTran)
				{
					BDBG_ERR(("Fetal error: bad Tran at entry %d between pHeadTranInHw at %d pTailTranInHw at %d",
							  pTranEntry - pQueue->pHeadEntry,
							  pHeadTranInHw - pQueue->pHeadEntry,
							  pTailTranInHw - pQueue->pHeadEntry));
					return;
				}
				ulTranEntry += pTranEntry->ulNumTranBlocks;
				pTranEntry = pTranEntry->pNextTran;
				BDBG_ASSERT(NULL != pTranEntry);
			}
		}
		BDBG_ASSERT(bSeeLastIssued);
	}
}


/***************************************************************************
 *
 * Static Utility Functions for Sub-Module Queue
 *
 **************************************************************************/

/***************************************************************************
 * {private}
 * BDMA_P_Queue_DummyTryLock_isr
 */
static bool BDMA_P_Queue_DummyTryLock_isr
	( void *                   pvParm1,
	  int                      iParm2 )
{
	BSTD_UNUSED(pvParm1);
	BSTD_UNUSED(iParm2);

	return true;
}

/***************************************************************************
 * {private}
 * BDMA_P_Queue_DummyReleaseLock_isr
 */
static bool BDMA_P_Queue_DummyReleaseLock_isr
	( void *                   pvParm1,
	  int                      iParm2 )
{
	BSTD_UNUSED(pvParm1);
	BSTD_UNUSED(iParm2);

	return true;
}

/*--------------------------------------------------------------------------
 * {private}
 * BDMA_P_Queue_ProcDoneStatus_isr
 *
 * Static utility to process dma-done HW status
 */
static void BDMA_P_Queue_ProcDoneStatus_isr
	( BDMA_P_QueueHandle       hQueue )
{
	BDMA_P_QueueContext * pQueue;
	BDMA_P_QueueEntry  *pHeadTranInHw, *pTailTranInHw;	
	BDMA_P_QueueEntry  *pTran;
	bool  bSgOpenInHw;
	
	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );	
	BDBG_ASSERT( NULL != pQueue );	
	
	/* all Trans in HW have completed, but pHeadTranInHw could be NULL */
	bSgOpenInHw = pQueue->bSgOpenInHw;
	pHeadTranInHw = pQueue->pHeadTranInHw;
	pTailTranInHw = pQueue->pTailTranInHw;
	pTran = pHeadTranInHw;
	while (NULL != pTran)
	{
		pHeadTranInHw = pTran->pNextTran;

		pTran->eStatus = BDMA_TranStatus_eSucceeded;
		pTran->pNextTran = NULL;
		
		/* let user's call back func further process done-info.
		 * likely user will check the status of the done Tran
		 * and issue another DMA transfer
		 */
		if (NULL != pTran->pCallBackFunc_isr)
		{
			(*pTran->pCallBackFunc_isr) (
				pTran->pUserCallBackParam, pTran, BDMA_TranStatus_eSucceeded);
		}

		/* continue the loop */
		pTran = pHeadTranInHw;
	}
	
	/* no more Tran progress in HW */
	pQueue->pHeadTranInHw = NULL;
	pQueue->pTailTranInHw = NULL;

	BDBG_MSG(("\t\t\tDone with last Tran in Hw at entry %d, bSgOpenInHw = %d",
			  pTran - pQueue->pHeadEntry, bSgOpenInHw));
}

#if BDMA_P_USE_CURR_DESC_ADDR
/*--------------------------------------------------------------------------
 * {private}
 * BDMA_P_Queue_ProcInProgStatus_isr
 *
 *  Static utility to process dma-in-progress HW status
 */
static void BDMA_P_Queue_ProcInProgStatus_isr
	( BDMA_P_QueueHandle       hQueue,
	  uint32_t                 ulHwActDesc )
{
	BDMA_P_QueueContext * pQueue;
	BDMA_P_QueueEntry  *pTran, *pNextTran;
	
	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );

	/* all Trans before the entry that has desc ulHwActDesc have completed */
	pTran = pQueue->pHeadTranInHw;	
	while (NULL != pTran)
	{
		if ((ulHwActDesc >= pTran->ulDescPhysAddr) &&
			(ulHwActDesc <= (pTran + (pTran->ulNumActBlocks - 1))->ulDescPhysAddr))
		{
			/* hardware working on this transaction.
			 * note: eStatus != eSuccess if this func is called */
			break;
		}

		/* pTran is completed: HW is with next Tran */
		BDBG_MSG(("\t\t\tmark tran at entry %d as success", 
				  pTran - pQueue->pHeadEntry));
		
		pNextTran = pTran->pNextTran;
		pTran->eStatus = BDMA_TranStatus_eSucceeded;
		pTran->pNextTran = NULL;
		
		/* let user's call back func further process done-info.
		 * likely user will check the status of the done Tran
		 * and issue another DMA transfer
		 */
		if (NULL != pTran->pCallBackFunc_isr)
		{
			(*pTran->pCallBackFunc_isr) (
				pTran->pUserCallBackParam, pTran, BDMA_TranStatus_eSucceeded);
		}

		/* continue the loop */
		pTran = pNextTran;
	}
	
	/* the first Tran in HW not completed yet */
	pQueue->pHeadTranInHw = pTran;
	
	BDBG_MSG(("\t\t\t1stUnFinishedTran at entry %d", pTran - pQueue->pHeadEntry));
}
#endif

/*--------------------------------------------------------------------------
 * {private}
 * BDMA_P_Queue_SendToHw_isr
 *
 * Static utility to send to send the Trans queued in SW into HW
 */
static void BDMA_P_Queue_SendToHw_isr
	( BDMA_P_QueueHandle       hQueue,
	  BDMA_P_HwStatus          eStatus,
	  bool                     bAppendInHw)
{
	BDMA_P_QueueContext * pQueue;
	
	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );

#if (1 == BDMA_P_SHOW_DYN_LINK_FAIL)	
	BDMA_P_Queue_CheckQCorruption_isr(pQueue);
	BDMA_P_Queue_CheckNextDescInHw_isr(pQueue);
#endif
	
	/* start dma in hw: append desc if bAppendInHw */
	(*pQueue->pSendToHw_isr) (hQueue, eStatus, bAppendInHw);

	/* in case last dyn linking fails */
	pQueue->pPreLinkTailTranInHw = (bAppendInHw)? pQueue->pTailTranInHw : NULL;
	pQueue->bPreLinkSgOpenInHw = pQueue->bSgOpenInHw;
 
	pQueue->bSgOpenInHw = pQueue->bSgOpenInQ;

	if ((NULL != pQueue->pHeadTranInHw)&&(pQueue->pTailTranInHw != NULL))
		pQueue->pTailTranInHw->pNextTran = pQueue->pHeadTranInQ;
	else
		pQueue->pHeadTranInHw = pQueue->pHeadTranInQ;
	pQueue->pTailTranInHw = pQueue->pTailTranInQ;

	pQueue->pHeadTranInQ = NULL;
	pQueue->pTailTranInQ = NULL;

	pQueue->bHwStarted = true;
}


/***************************************************************************
 *
 * Exported Queue Functions 
 *
 **************************************************************************/

/***************************************************************************
 * {private}
 * BDMA_P_Queue_Create
 *
 * To be called to create queue for a dma channel. It allocates a queue entry
 * array from system memory, and contiguous descriptor array from BMEM.
 * It then initiate the allocated queue array, including setting each
 * queue-entry to point to the descriptor of the same index. 
 *
 * Note: assume parameters are valid
 */
BERR_Code BDMA_P_Queue_Create
	( BDMA_P_QueueHandle *      phQueue,
	  void *                    hEngine,
	  BMEM_Handle               hMemory,
	  uint32_t                  ulNumWordsPerDesc,
	  const BDMA_P_CmnSettings *pCmnSettings,
	  BDMA_P_ReadHwRegsFunc		pReadHwRegs_isr,
	  BDMA_P_AppendTranDescFunc	pAppendTranDesc_isr,
	  BDMA_P_SendToHwFunc       pSendToHw_isr,
	  BDMA_P_CheckNextDescFunc  pCheckNextDesc_isr,
	  BDMA_P_SafeToSendHwFunc   pSafeToSendHw_isr)
{
	BDMA_P_QueueContext *  pQueue = NULL;
	BDMA_P_QueueEntry *    pQueueEntry = NULL;
	uint32_t *             pulDesc = NULL;
	BDMA_P_QueueEntry * pEntry;
	BERR_Code eResult = BERR_SUCCESS;
	uint32_t  ulNumQueueEntries= 0;
	uint32_t  ulDescBufSize = 0;
	uint32_t  ulId =0;
	void * pCachedAddr = NULL;

	BDBG_ASSERT( NULL != phQueue );
	BDBG_ASSERT( NULL != hMemory );
	BDBG_ASSERT( 0 != ulNumWordsPerDesc );
	BDBG_ASSERT( NULL != pCmnSettings );
	BDBG_ASSERT( NULL != pReadHwRegs_isr );
	BDBG_ASSERT( NULL != pAppendTranDesc_isr );
	BDBG_ASSERT( NULL != pSendToHw_isr );
	BDBG_ASSERT( NULL != pCheckNextDesc_isr );
	BDBG_ASSERT( NULL != pSafeToSendHw_isr);

	/* allocate and clear QueueContext (i.e. queue) */
	pQueue = (BDMA_P_QueueContext *)BKNI_Malloc( sizeof(BDMA_P_QueueContext) );
	if ( NULL == pQueue )
	{
		eResult = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto BDMA_P_Err_Queue_Create;		
	}
	BKNI_Memset((void*)pQueue, 0x0, sizeof(BDMA_P_QueueContext));
	
	/* allocate and clear Queue Entries (i.e. Blocks) */
	ulNumQueueEntries = pCmnSettings->ulTotalNumBlocks;
	pQueueEntry = (BDMA_P_QueueEntry *)BKNI_Malloc( (1 + ulNumQueueEntries) * sizeof(BDMA_P_QueueEntry) );
	if ( NULL == pQueueEntry )
	{
		eResult = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto BDMA_P_Err_Queue_Create;		
	}
	BKNI_Memset((void*)pQueueEntry, 0x0, (1 + ulNumQueueEntries) * sizeof(BDMA_P_QueueEntry));

	/* allocate desc array: it has to be physically present and contiguous
	 * (required by io dma ), therefore should use BMEM */
	ulDescBufSize = ulNumQueueEntries * ulNumWordsPerDesc * 4;
	pulDesc = (uint32_t *) BMEM_AllocAligned(
		hMemory, ulDescBufSize, 6, 0 ); /* 6 ==> 2^6 = 64 bytes alligned */ 
	if ( NULL == pulDesc )
	{
		eResult = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto BDMA_P_Err_Queue_Create;
	}
	BKNI_Memset((void*)pulDesc, 0x0, ulDescBufSize);

	/* initialize queue entries */
	for (ulId=0; ulId<ulNumQueueEntries; ulId++)
	{
		pEntry = pQueueEntry + ulId;
		
		/*pEntry->bBlockInfoSet = false;
		pEntry->bCachedDscrp = false;
		pEntry->ulSgStartStop = 0;
		pEntry->ulNumActBlocks = 0;
		pEntry->bSgStartStopSet = false;
		pEntry->bSgEnable = false;
		pEntry->ulKeySlot = 0;
		pEntry->eCryptMode = BDMA_CryptMode_eNoCrypt;*/
		pEntry->ulNumTranBlocks = 1; /* assert(0!=ulNumTranBlocks)*/
		pEntry->pulDescAddr = pulDesc + ulId * ulNumWordsPerDesc;
		eResult = BMEM_ConvertAddressToCached(hMemory, (void*) pEntry->pulDescAddr,
			&pCachedAddr);
		if (BERR_SUCCESS != eResult)
		{
			goto BDMA_P_Err_Queue_Create;
		}
		/* this extra assignment is to fix a strict-aliasing compile warning */ 
		pEntry->pulCachedDescAddr = (uint32_t *)pCachedAddr;
		BMEM_ConvertAddressToOffset(hMemory, (void*) pEntry->pulDescAddr,
			&pEntry->ulDescPhysAddr);
		pEntry->eStatus = BDMA_TranStatus_eUnknown;
		BDMA_P_TRAN_SET_BLACK_MAGIC( pEntry );

		BDBG_MSG(("Block %d: pulDescAddr 0x%08lx, pulCachedDescAddr 0x%08lx",
				  ulId, pEntry->pulDescAddr, pEntry->pulCachedDescAddr));
	}

	/* this extra struct only means to make (pBlock + 1) valid,
	 * it is to make SetBlockInfo convenient */
	pEntry = pQueueEntry + ulNumQueueEntries;
	pEntry->ulNumTranBlocks = 1;
		pEntry->pulDescAddr = pQueueEntry->pulDescAddr;
	pEntry->pulCachedDescAddr = pQueueEntry->pulCachedDescAddr;
	pEntry->eStatus = BDMA_TranStatus_eUnknown;

	/* initialize BDMA_P_QueueContext */
	/*pQueue->bSgOpenInHw = false;
	pQueue->bSgOpenInQ = false;
	pQueue->pHeadTranInHw = NULL;
	pQueue->pTailTranInHw = NULL;
	pQueue->pHeadTranInQueue = NULL;
	pQueue->pTailTranInQueue = NULL;*/
	pQueue->ulNumWordsPerDesc = ulNumWordsPerDesc;
	pQueue->CmnSettings = *pCmnSettings;
	pQueue->bDmaHwShared = (NULL != pCmnSettings->pTryLock_isr);
	pQueue->pHeadEntry = pQueueEntry;
	pQueue->ul1stDescPhysAddr = pQueueEntry->ulDescPhysAddr;
	pQueue->ulLastDescPhysAddr =
		(pQueueEntry + (ulNumQueueEntries - 1))->ulDescPhysAddr;	
	pQueue->pReadHwRegs_isr = pReadHwRegs_isr;
	pQueue->pAppendTranDesc_isr = pAppendTranDesc_isr;
	pQueue->pSendToHw_isr = pSendToHw_isr;
	pQueue->pCheckNextDesc_isr = pCheckNextDesc_isr;
	pQueue->pSafeToSendHw_isr  = pSafeToSendHw_isr;
	pQueue->ulIndx_LastEntryAssigned = ulNumQueueEntries - 1;
	pQueue->hEngine = hEngine;
	BDMA_P_QUEUE_SET_BLACK_MAGIC( pQueue );

	/* use dummy TryLock/ReleaseLock to avoid "if" in code if it is not provided */
	if (NULL == pCmnSettings->pTryLock_isr)
	{
		pQueue->CmnSettings.pTryLock_isr = BDMA_P_Queue_DummyTryLock_isr;
		pQueue->CmnSettings.pReleaseLock_isr = BDMA_P_Queue_DummyReleaseLock_isr;
	}
	
	*phQueue = pQueue;
	return eResult;

  BDMA_P_Err_Queue_Create:
	/* free allocated buf */
	if ( NULL != pQueue )
		BDMA_P_QUEUE_DESTROY_CONTEXT(pQueue);
	if ( NULL != pQueueEntry )
		BDMA_P_QUEUE_DESTROY_ENTRY(pQueueEntry, ulNumQueueEntries);
	if ( NULL != pulDesc )
		BDMA_P_QUEUE_DESTROY_DESC( hMemory, pulDesc, ulDescBufSize);
	
	*phQueue = NULL;
	return eResult;
}

/***************************************************************************
 * {private}
 * BDMA_P_Queue_Destroy
 *
 * To be called to destroy the queue.
 *
 * Note: assume parameters are valid
 */
BERR_Code BDMA_P_Queue_Destroy
	( BDMA_P_QueueHandle        hQueue,
	  BMEM_Handle               hMemory )
{
	BDMA_P_QueueContext * pQueue;	
	BDMA_P_QueueEntry * pFirstEntry;
	uint32_t  ulDescBufSize;
	
	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );
	BDBG_ASSERT( NULL != hMemory );
	BDMA_P_QUEUE_ASSERT_UNCORRUPT( pQueue );
	
	/* TODO: make sure HW is idle ??? */
	
	/* free allocated buf */
	if ( NULL != pQueue )
	{
		pFirstEntry = pQueue->pHeadEntry;
		if ( NULL != pFirstEntry )
		{
			if ( NULL != pFirstEntry->pulDescAddr )
			{
				ulDescBufSize = pQueue->CmnSettings.ulTotalNumBlocks *
					pQueue->ulNumWordsPerDesc * 4;
				BDMA_P_QUEUE_DESTROY_DESC( hMemory,
					pFirstEntry->pulDescAddr, ulDescBufSize );
			}
			BDMA_P_QUEUE_DESTROY_ENTRY(
				pFirstEntry, pQueue->CmnSettings.ulTotalNumBlocks);
		}
		BDMA_P_QUEUE_DESTROY_CONTEXT( pQueue );
	}

	return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 * BDMA_P_Queue_GetEngine_isr
 *
 * To be called to get the dma engine
 *
 * Note: assume parameters are valid
 */
void * BDMA_P_Queue_GetEngine_isr
	( BDMA_P_QueueHandle        hQueue )
{
	BDMA_P_QueueContext * pQueue;	

	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );

	return pQueue->hEngine;
}

#define BDMA_P_NUM_TRY_READ            (5)
/*--------------------------------------------------------------------------
 * {private}
 * BDMA_P_Queue_ReadHwReg_isr
 *
 *  Static utility to check whether the values returned by HW register 
 *  reading looks good
 */
static bool BDMA_P_Queue_ReadHwReg_isr
	( BDMA_P_QueueContext     *pQueue,
	  BDMA_P_HwStatus         *peStatus,
	  uint32_t                *pulHwActDesc,
	  uint32_t                *pulScratchReg)
{
	BDMA_P_QueueEntry  *pLastTranInHw, *pLastEntryInHw;
	BDMA_P_QueueEntry  *pNewHeadTranInQ, *pPreLinkTailTranInHw, *pPreLinkLastEntryInHw;
	BDMA_P_HwStatus  eStatus;
	uint32_t  ulHwActDesc, ulHw1stDesc, ulWaterMark, ulScratchReg, ulLastDesc;
	uint32_t  ul1stDescInQ, ulLastDescInQ;
	bool  bBadHwDescAddr, bBadWaterMark, bMisMatch, bHwEndDescMisMatch = true;
	uint32_t  ulOff, ulOffMask;
	int  ii;

	BDBG_ASSERT( NULL != pQueue );
	BDBG_ASSERT( NULL != peStatus );
	BDBG_ASSERT( NULL != pulHwActDesc );
	BDBG_ASSERT( NULL != pulScratchReg );
	BDBG_ASSERT((NULL == pQueue->pHeadTranInHw) == (NULL == pQueue->pTailTranInHw));

	/* some customer team found HW regsiter reading might return junk value */
	ulOffMask = (8 == pQueue->ulNumWordsPerDesc)? 0xFFF0001F: 0xFFF0000F;
	ul1stDescInQ  = pQueue->ul1stDescPhysAddr;
	ulLastDescInQ = pQueue->ulLastDescPhysAddr;
	pLastTranInHw = pQueue->pTailTranInHw;
	pLastEntryInHw = (NULL != pLastTranInHw)?
		pLastTranInHw + (pLastTranInHw->ulNumActBlocks - 1) : NULL;
	for (ii=0; ii<BDMA_P_NUM_TRY_READ; ii++)
	{
		/* read HW registers */
		(*pQueue->pReadHwRegs_isr)(
			pQueue->hEngine, &eStatus, &ulHwActDesc, &ulScratchReg);

		/* validate the reg reading */
		
		ulHw1stDesc = ulScratchReg & ~BDMA_P_WATER_MARK_MASK;
		ulWaterMark = ulScratchReg & BDMA_P_WATER_MARK_MASK;
		ulOff = ulHwActDesc - ulHw1stDesc;

	
		bBadWaterMark = ((ulWaterMark != BDMA_P_WATER_MARK_SG_OPEN) &&
						 (ulWaterMark != BDMA_P_WATER_MARK_SG_CLOSE));
		bBadHwDescAddr = (0 != (ulOff & ulOffMask));
        if (( pLastTranInHw != NULL) &&(pLastTranInHw->ulFirstBlock))
        {
			ulLastDesc = pLastTranInHw->ulDescPhysAddr + (((pLastTranInHw->ulNumActBlocks-1)+pLastTranInHw->ulFirstBlock)*4*8/*BDMA_P_MEM_NUM_WORDS_PER_DESC*/);
			bHwEndDescMisMatch = ( ulLastDesc != ulHwActDesc );
        }
		
		bMisMatch = (((BDMA_P_HwStatus_eSleep == eStatus)             &&
					  (NULL != pLastTranInHw)                         &&
					  (pLastEntryInHw->ulDescPhysAddr != ulHwActDesc) &&
					  bHwEndDescMisMatch )                            ||
					 ((BDMA_P_HwStatus_eIdle == eStatus)              &&
					  (pQueue->bHwStarted)));
		
		*peStatus = eStatus;
		*pulHwActDesc = ulHwActDesc;
		*pulScratchReg = ulScratchReg;
		if ((BDMA_P_HwStatus_eUnknown == eStatus) ||
			((BDMA_P_HwStatus_eIdle != eStatus) &&
			 (bBadWaterMark || bBadHwDescAddr || bMisMatch)))
		{
			/* note: we occasionally see dynamic linking failure when HW is working on the
			 * last desc as we perform the dyn linking. (why XXX ???)
			 * re-wind Q record if it is only because that the last dynamic linking failed */
			pPreLinkTailTranInHw = pQueue->pPreLinkTailTranInHw;
			pPreLinkLastEntryInHw = (NULL != pPreLinkTailTranInHw)?
				pPreLinkTailTranInHw + (pPreLinkTailTranInHw->ulNumActBlocks - 1) : NULL;
			if ((NULL != pPreLinkTailTranInHw) &&
				(BDMA_TranStatus_eInProgress == pPreLinkTailTranInHw->eStatus) &&
				(pPreLinkLastEntryInHw->ulDescPhysAddr == ulHwActDesc) &&
				(BDMA_P_HwStatus_eSleep == eStatus) &&
				(false == bBadWaterMark) &&
				(false == bBadHwDescAddr))
			{
#if (1 == BDMA_P_SHOW_DYN_LINK_FAIL)	
				BDBG_ERR(("last dyn link failed, rewind. now LastDescInHw 0x%08lx",
						  pPreLinkLastEntryInHw->ulDescPhysAddr));
#endif
				pNewHeadTranInQ = pPreLinkTailTranInHw->pNextTran;
				if (NULL != pQueue->pHeadTranInQ)
				{
					(*pQueue->pAppendTranDesc_isr)(pNewHeadTranInQ, pQueue->pHeadTranInQ);
					pNewHeadTranInQ->pNextTran = pQueue->pHeadTranInQ;
				}
				else
				{
					pQueue->pTailTranInQ = pNewHeadTranInQ;
					pNewHeadTranInQ->pNextTran = NULL;
				}
				pQueue->pHeadTranInQ = pNewHeadTranInQ;
				
				pPreLinkTailTranInHw->pNextTran = NULL;
				pQueue->pTailTranInHw = pPreLinkTailTranInHw;
				pQueue->bSgOpenInHw = pQueue->bPreLinkSgOpenInHw;
				
				pQueue->pPreLinkTailTranInHw = NULL;
				return true;
			}

			/* this read is invalid */
			BDBG_ERR(("see bad HW reg read: eStatus %d, HwActDesc 0x%08lx, Scratch 0x%08lx",
					  eStatus, ulHwActDesc, ulScratchReg));
			BDBG_ERR(("bBadWaterMark %d bBadHwDescAddr %d bMisMatch %d, ulHwLastDesc 0x%08lx",
					  bBadWaterMark? 1: 0, bBadHwDescAddr? 1 : 0, bMisMatch? 1: 0,
					  (pLastEntryInHw)? pLastEntryInHw->ulDescPhysAddr : 0));
		}
		else
		{
			/*BDBG_MSG(("good HW reg read: eStatus %d, HwActDesc 0x%08lx, ulHwLastDesc 0x%08lx",
			  eStatus, ulHwActDesc, (pLastEntryInHw)? pLastEntryInHw->ulDescPhysAddr : 0));*/
			pQueue->pPreLinkTailTranInHw = NULL;
			return true;
		}
	}
	pQueue->pPreLinkTailTranInHw = NULL;
	
	/* we read BDMA_P_NUM_TRY_READ times and failed */
	if (pQueue->bHwStarted)
	{
		BDBG_ERR(("bad HW reg read: eStatus %d, HwActDesc 0x%08lx, Scratch 0x%08lx",
				  eStatus, ulHwActDesc, ulScratchReg));
		BDBG_ERR(("Queue 1st desc 0x%08lx, last desc 0x%08lx", ul1stDescInQ, ulLastDescInQ));

		BDMA_P_Queue_CheckQCorruption_isr(pQueue);
		BDMA_P_Queue_CheckNextDescInHw_isr(pQueue);
	}
	
	/* if app exit and re-run, we might have mismatched status at beginning.
	 * XXX TODO: how to handle app re-start when reg has mismatched values, and 
	 * not break multiple bdma instances algorithm? */
	return false;
}

/*--------------------------------------------------------------------------
 * {private}
 * BDMA_P_Queue_CheckHwAndSendNewReq_isr
 *
 * Called to check HW status, process it, and try to send the Trans queued
 * in SW into HW
 */
void BDMA_P_Queue_CheckHwAndSendNewReq_isr
	( BDMA_P_QueueHandle       hQueue )
{
	BDMA_P_QueueContext * pQueue;

	BDMA_P_CmnSettings  *pSettings;
	BDMA_P_QueueEntry  *pTailTranInHw;
	uint32_t  ulHwActDesc = 0;
	uint32_t  ulScratchReg = 0;
	BDMA_P_HwStatus  eStatus;
	bool  bGotLock, bRegReadOk;
	bool  bHwSleep, bSgOpenInHw, bOwnHw, bAppendInHw, bSendToHw;
		
	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );

	/* ensure dma done _isr is quarded by critical section */
	BDBG_ASSERT(0 == pQueue->ulLockMark);
	pQueue->ulLockMark++;

	BDMA_P_QUEUE_ASSERT_UNCORRUPT( pQueue );
	BDMA_P_QUEUE_CHECK_NEXT_DESC( pQueue );

	bGotLock = false;
    if ((NULL == pQueue->pHeadTranInQ) && (NULL == pQueue->pHeadTranInHw))
		/* nothing to do */
		goto CleanUp;

	pSettings = &(pQueue->CmnSettings);
	bSgOpenInHw = true;  /* worst senario */
	bHwSleep = false;
	bOwnHw = ! pQueue->bDmaHwShared;
	eStatus = BDMA_P_HwStatus_eUnknown;
	bGotLock =
		(*pSettings->pTryLock_isr) (pSettings->pvParm1, pSettings->iParm2);
	
	bRegReadOk = BDMA_P_Queue_ReadHwReg_isr(
		pQueue, &eStatus, &ulHwActDesc, &ulScratchReg);
	pQueue->pSafeToSendHw_isr(hQueue, &eStatus);
	
	if ((NULL != pQueue->pHeadTranInHw) &&
		(bRegReadOk))
	{
		/* this BDMA instance has sent its transfer list into
		 * hardware, now the result is checked and processed */

		if (/* some other BDMA instance already grab HW from us */
#if BDMA_P_USE_CURR_DESC_ADDR
			((ulHwActDesc < pQueue->ul1stDescPhysAddr) &&
			 (ulHwActDesc > pQueue->ulLastDescPhysAddr)) ||
#endif
			/* still owned by this BDMA instance, but dma done */
			(BDMA_P_HwStatus_eSleep == eStatus))
			/* ReadHwReg_isr already check the case that dma cmd still in bus */
		{
			/* the request of this BDMA instance have completed in HW,			
			 * mark success and call user call-back for all trans done-in-Hw,
			 * and update pHead/TrailTranInHw */
			BDMA_P_Queue_ProcDoneStatus_isr(hQueue);
		}
#if BDMA_P_USE_CURR_DESC_ADDR
		else 
		{
			/* this BDMA still own the HW, the dma req might-not completed yet 
			 * mark success and call user call-back for done-trans */
			BDMA_P_Queue_ProcInProgStatus_isr(hQueue, ulHwActDesc);
		}
#endif
	}
	/* else this instance has no req in HW, no need to check result */

    if (NULL != pQueue->pHeadTranInQ)
    {
		/* there are Tran queued in sw, try to send it to hw */
		if (bGotLock && (bRegReadOk || !pQueue->bHwStarted))
		{
            /* if current HW owner's last request in HW completed, we can grab
			 * the HW now. 
			 * Note: check eStatus alone might not be enough if we read while
			 *       a new dma was sent to HW but status not changed yet, but
			 *       this case has been checked by ReadHwReg_isr. It will read
			 *       one more time in this case.
			 * note: we use (scrach & BDMA_P_WATER_MARK_MASK) to indicate 
			 *       bSgOpenInHw */
			bSgOpenInHw = 
				(BDMA_P_WATER_MARK_SG_OPEN == (ulScratchReg & BDMA_P_WATER_MARK_MASK));
            bHwSleep = ((BDMA_P_HwStatus_eIdle  == eStatus) ||
						(BDMA_P_HwStatus_eSleep == eStatus));
			bOwnHw = ((ulHwActDesc >= pQueue->ul1stDescPhysAddr) &&
					  (ulHwActDesc <= pQueue->ulLastDescPhysAddr));
		}

		pTailTranInHw = pQueue->pTailTranInHw;
#if BDMA_P_SUPPORT_MEM_DMA_AUTO_APPEND
		/* if pQueue->bSgOpenInHw, we can always dynamically link new dma req
		 * in HW, even if we did not get Lock. When we failed to get Lock, this 
		 * is also the only case we will send new dma req to HW */
		bSendToHw = (bOwnHw || pQueue->bSgOpenInHw || (bHwSleep && !bSgOpenInHw));
#else
		/* for those chips that don't support auto_append there is a HW bug
		 * with dynamic append when scater-gather is active */
		bSendToHw = ((bOwnHw && !pQueue->bSgOpenInHw) ||
					 (bHwSleep && (!bSgOpenInHw || pQueue->bSgOpenInHw)));
#endif
		bAppendInHw = ((NULL != pTailTranInHw) && (!bHwSleep));
		
		if (bSendToHw)
		{
			BDMA_P_Queue_SendToHw_isr(hQueue, eStatus, bAppendInHw);
		}
	}
	
  CleanUp:		
	if (bGotLock)
	{
		(*pSettings->pReleaseLock_isr)(
			pSettings->pvParm1, pSettings->iParm2);
	}	
	pQueue->ulLockMark--;
}

/*--------------------------------------------------------------------------
 * {private}
 * BDMA_P_Queue_ReInitBlocks_isr
 *
 * static utility used in this file. No validation. It makes the chunk of
 * blocks following ulFirstEntryId as one destroyed Tran
 */
static void BDMA_P_Queue_ReInitBlocks_isr(
	BDMA_P_QueueContext *pQueue,
	BDMA_P_QueueEntry *  pFirstEntry,
	uint32_t             ulNumBlocks )
{
	BDMA_P_QueueEntry *  pEntry;
	uint32_t  ulLoopCntr;
	uint32_t  ulDescNumWords;

	ulDescNumWords = pQueue->ulNumWordsPerDesc;
	for ( ulLoopCntr = 0; ulLoopCntr < ulNumBlocks; ulLoopCntr++ )
	{
		pEntry = pFirstEntry + ulLoopCntr;
		
		pEntry->eStatus = BDMA_TranStatus_eUnknown;
		pEntry->bBlockInfoSet = false;
		pEntry->bCachedDscrp = false;
		pEntry->ulSgStartStop = 0;
		pEntry->ulNumActBlocks = 0;
		pEntry->ulNumTranBlocks = 1; /* assert(0!=ulNumTranBlocks)*/
		pEntry->pCallBackFunc_isr = NULL;
		pEntry->pUserCallBackParam = NULL;
		pEntry->bSgStartStopSet = false;
		pEntry->bSgEnable = false;
		pEntry->ulKeySlot = 0;
		pEntry->eCryptMode = BDMA_CryptMode_eNoCrypt;
	
		/* clean bSgStart/bSgEnd bits
		 * todo: use a call back func for this */
		if (ulDescNumWords > 4)
			*(pEntry->pulDescAddr + 4) = 0; 
	}

	/* re-init first block */
	pFirstEntry->ulNumTranBlocks = ulNumBlocks;
}

/*--------------------------------------------------------------------------
 * {private}
 * BDMA_P_Queue_TryMakeTranHere_isr(
 *
 * static utility used by BDMA_P_Queue_CreateTran only. No validation.
 * It checks whether ulFirstEntryId has enough free following contiguous
 * blocks. Return ulNumBlocksNeeded if yes, return the real number of 
 * free contiguous blocks if not. In the yes case, it also make the remaining
 * tail as a destroyed Tran
 */
static uint32_t BDMA_P_Queue_TryMakeTranHere_isr(
	BDMA_P_QueueContext * pQueue,
	uint32_t              ulFirstEntryId,
	uint32_t              ulNumBlocksNeeded )
{
	BDMA_P_QueueEntry *  pEntry;
	uint32_t ulNumFreeBlocks;
	uint32_t ulEntryId;
	uint32_t ulNumRemainBlocks;
	uint32_t ulNumQueueEntries;
	
	ulNumFreeBlocks = 0;
	ulEntryId = ulFirstEntryId;
	ulNumQueueEntries = pQueue->CmnSettings.ulTotalNumBlocks;
	while ( ulEntryId < ulNumQueueEntries )
	{
		pEntry = pQueue->pHeadEntry + ulEntryId;
		if ( BDMA_TranStatus_eUnknown == pEntry->eStatus )
		{
			/* the chunk of blocks lead by this "FirstEntry" is free */
			ulNumFreeBlocks += pEntry->ulNumTranBlocks;
			if ( ulNumFreeBlocks >=  ulNumBlocksNeeded )
			{
				/* we got enough contiguous free blocks, init the new Tran */
				BDMA_P_Queue_ReInitBlocks_isr(
					pQueue, pQueue->pHeadEntry + ulFirstEntryId, ulNumBlocksNeeded );
				pEntry = pQueue->pHeadEntry + ulFirstEntryId;
				pEntry->eStatus = BDMA_TranStatus_eCreated;
				
				/* make the Tran's remaining tail as a destroyed Tran */
				ulNumRemainBlocks = ulNumFreeBlocks - ulNumBlocksNeeded;				
				if ( ulNumRemainBlocks > 0 )
				{
					BDMA_P_Queue_ReInitBlocks_isr(pQueue,
						pQueue->pHeadEntry + ulFirstEntryId+ulNumBlocksNeeded,
						ulNumRemainBlocks );
				}

				return ulNumBlocksNeeded;				
			}
			else
			{
				/* need to continue to look at next Tran: one more loop pass */
				ulEntryId += pEntry->ulNumTranBlocks;
			}
		}
		else /* we hit an active Tran, still not got enough free blocks yet */
		{
			break;
		}
	}

	return ulNumFreeBlocks;
}

/***************************************************************************
 * {private}
 * BDMA_P_Queue_CreateTran_isr
 *
 * To be called when a new transfer is created. It finds a free chunck of
 * contiguous queue entries (that corresponds to a set of contiguous
 * descriptors too) in the dma channel queue, and outputs the first entry 
 * of the chunck. 
 *
 * Note: assume parameters are valid,
 *       ulNumBlocks can not be 0
 */
BERR_Code BDMA_P_Queue_CreateTran_isr
	( BDMA_P_QueueHandle       hQueue,
	  uint32_t                 ulNumBlocks,
	  bool                     bCachedDesc,
	  BDMA_P_QueueEntry **     ppNewTran )
{
	BDMA_P_QueueContext * pQueue;
	uint32_t ulPreAssignedId;
	uint32_t ulNewAssignedId;
	BDMA_P_QueueEntry * pNewAssigned;		
	BDMA_P_QueueEntry * pPreAssigned;
	uint32_t ulNumBlockSeen, ulMaxNumBlockToSee;
	uint32_t ulNumFreeBlocks;
	uint32_t ulNumQueueEntries;

	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );
	BDBG_ASSERT( NULL != ppNewTran );
	BDBG_ASSERT( 0 != ulNumBlocks );
	BDBG_ASSERT( ulNumBlocks <= pQueue->CmnSettings.ulMaxNumBlocksPerTran );
	BDMA_P_QUEUE_ASSERT_UNCORRUPT( pQueue );
	
	*ppNewTran = NULL;
	
	/* find free contiguous entries */
	ulPreAssignedId = pQueue->ulIndx_LastEntryAssigned;
	pPreAssigned = pQueue->pHeadEntry + ulPreAssignedId;

	/* The lastAssignedId might be free also, so should look one more time */
	ulNumQueueEntries = pQueue->CmnSettings.ulTotalNumBlocks;
	ulMaxNumBlockToSee = ulNumQueueEntries + pPreAssigned->ulNumTranBlocks;
	ulNumBlockSeen = 0;
	while ( ulNumBlockSeen < ulNumQueueEntries )
	{
		ulNewAssignedId = BDMA_P_QUEUE_WRAP_INDEX(
			ulPreAssignedId + pPreAssigned->ulNumTranBlocks );
		pNewAssigned = pQueue->pHeadEntry + ulNewAssignedId;

		/* try to get ulNumBlocks contiguous free blocks following
		 * ulNewAssignedId */
		ulNumFreeBlocks = BDMA_P_Queue_TryMakeTranHere_isr(
			pQueue, ulNewAssignedId, ulNumBlocks );

		if ( ulNumBlocks == ulNumFreeBlocks )
		{
			/* This entry got enough contiguous following free blocks */
			pQueue->ulIndx_LastEntryAssigned = ulNewAssignedId;
			pNewAssigned->hQueue = hQueue;
			pNewAssigned->bCachedDscrp = bCachedDesc;
			*ppNewTran = pNewAssigned;			
			return BERR_SUCCESS;
		}
		else
		{
			/* This entry does NOT have enough contiguous following free blocks
			 * skip this chunk and try another loop pass */
			ulNumBlockSeen += (ulNumFreeBlocks + pPreAssigned->ulNumTranBlocks);
			ulPreAssignedId = BDMA_P_QUEUE_WRAP_INDEX(
				ulNewAssignedId + ulNumFreeBlocks );
			pPreAssigned = pQueue->pHeadEntry + ulPreAssignedId;
		}
	}

	/* we did not find a big enough contiguous free block chunk */
	return BERR_TRACE(BDMA_ERR_OUT_OF_QUEUE);
}

/***************************************************************************
 * {private}
 * BDMA_P_Queue_StartTran_isr
 *
 * To be called to process the user calling of Start or StartAndCallBack. It
 * always returns eSuccess.
 *
 * pCallBackFunc_isr is assumed to indicate whether the Tran is sync or async.
 * No matter its is aync or async, it is always queued in sw first. Then the
 * Trans in sw Q is sent to HW if we can.
 * 
 * Note: assume parameters are valid
 */
BERR_Code BDMA_P_Queue_StartTran_isr
	( BDMA_P_QueueHandle       hQueue,
	  BDMA_P_QueueEntry *      pTran,
	  uint32_t                 ulNumActBlocks,
	  bool                     bSgOpen, 
	  BDMA_P_CallbackFunc      pCallBackFunc_isr,
	  void *                   pUserCallBackParam )
{
	BDMA_P_QueueContext * pQueue;
	
	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );
	BDBG_ASSERT( NULL != pTran );

	/* can not re-start this Tran if it is still in prgress */
	if (BDMA_TranStatus_eInProgress == pTran->eStatus)
	{
		BDBG_ERR(("re-start in prog tran at entry %d", pTran - pQueue->pHeadEntry));
		return BERR_TRACE(BDMA_ERR_TRAN_IN_PROGRESS);
	}

#if (1 == BDMA_P_SHOW_SG_OPEN)
	if (bSgOpen)
	{
		BDBG_ERR(("!!!!!!!! Scatter-gather still open at end with Tran %d",
				  pTran - pQueue->pHeadEntry));
	}
#endif
	
	/* put this Tran into sw queue */
	pTran->eStatus = BDMA_TranStatus_eInProgress;
	pTran->pNextTran = NULL; /* Proc*Status rely on this to get out of loop */
	pTran->ulNumActBlocks = ulNumActBlocks;
	pTran->pCallBackFunc_isr = pCallBackFunc_isr;		
	pTran->pUserCallBackParam = pUserCallBackParam;
	pQueue->bSgOpenInQ = bSgOpen;
	if (pQueue->pHeadTranInQ)
	{
		/* append this Tran into sw queue */
		(*pQueue->pAppendTranDesc_isr)(pQueue->pTailTranInQ, pTran);
		pQueue->pTailTranInQ->pNextTran = pTran;
	}
	else
	{
		pQueue->pHeadTranInQ = pTran;
	}
	pQueue->pTailTranInQ = pTran;
	
	/* check and try to setup the tran in hw */
	BDMA_P_Queue_CheckHwAndSendNewReq_isr(hQueue);
	
	return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 * BDMA_P_Queue_DestroyTran_isr
 *
 * To be called to destroy a Tran. It reset the Tran's entries to make the
 * chunk of blocks as a destroyed Tran
 *
 * Note: assume parameters are valid
 */
BERR_Code BDMA_P_Queue_DestroyTran_isr
	( BDMA_P_QueueHandle       hQueue,
	  BDMA_P_QueueEntry *      pTran )
{
	BDMA_P_QueueContext * pQueue;

	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );
	BDBG_ASSERT( NULL != pTran );
	BDMA_P_QUEUE_ASSERT_UNCORRUPT( pQueue );

	/* if pTran is pTailTranInHw in the case (true==pQueue->bSgOpenInHw),
	 * we can not destroy it yet, it is needed for future desc link */
	if ( (BDMA_TranStatus_eInProgress != pTran->eStatus) &&
		 ((!pQueue->bSgOpenInHw) || (pTran != pQueue->pTailTranInHw)) )
	{
		/* reset the blocks in this Tran */
		BDMA_P_Queue_ReInitBlocks_isr( pQueue, pTran, pTran->ulNumTranBlocks );
		return BERR_SUCCESS;
	}
	else
	{
		BDBG_ERR(("destroy in prog tran at entry %d", pTran - pQueue->pHeadEntry));
		return BERR_TRACE(BDMA_ERR_TRAN_IN_PROGRESS);
	}
}

/***************************************************************************
 * {private}
 * BDMA_P_Queue_ResetTran_isr
 *
 * To be called to reset a active Tran into intial eCreated state. Without
 * resetting, the Tran could also be used to SetBlockInfo and Start again,
 * but the module will not be able to check whether each block info is set.
 *
 * Note: assume parameters are valid,
 */
BERR_Code BDMA_P_Queue_ResetTran_isr
	( BDMA_P_QueueHandle       hQueue,
	  BDMA_P_QueueEntry *      pTran )
{
	BDMA_P_QueueContext * pQueue;
	uint32_t  ulNumTranBlocks;
	bool  bCachedDscrp;

	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );
	BDBG_ASSERT( NULL != pTran );
	BDMA_P_QUEUE_ASSERT_UNCORRUPT( pQueue );

	if ( BDMA_TranStatus_eInProgress != pTran->eStatus )
	{
		bCachedDscrp = pTran->bCachedDscrp; /* save for restore back */
		ulNumTranBlocks = pTran->ulNumTranBlocks;
		BDMA_P_Queue_ReInitBlocks_isr( pQueue, pTran, pTran->ulNumTranBlocks );
		pTran->bCachedDscrp = bCachedDscrp; /* restore back  */
		pTran->ulNumTranBlocks = ulNumTranBlocks;
		pTran->eStatus = BDMA_TranStatus_eCreated;
		return BERR_SUCCESS;
	}
	else
	{
		return BERR_TRACE(BDMA_ERR_TRAN_IN_PROGRESS);
	}
}

/* End of File */
