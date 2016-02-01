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

#include "bdma_pci_priv.h"
#include "bkni.h"
#include "bchp_pci_dma.h"
#include "bchp_common.h"
#include "bchp_hif_intr2.h"
#include "bchp_int_id_hif_intr2.h"

BDBG_MODULE(BDMA_PCI_PRIV);

#define BDMA_P_PCI_LAST_BLK                 1
#define BDMA_P_PCI_FIRE_INTR                1

/***************************************************************************
 *
 * Utility functions used by BDMA
 *
 ***************************************************************************/
/*--------------------------------------------------------------------------
 * To be called by BDMA API func before taking real action, to get dma mutex
 * WITH block, ideally for user mode
 */
BERR_Code BDMA_P_Pci_AcquireMutex(
	BDMA_Pci_Handle          hPciDma,
	BKNI_MutexHandle        *phMutex )
{
	BDMA_P_Pci_Context  * pPciDma;
	BERR_Code  eResult = BERR_SUCCESS;

	BDMA_P_PCI_GET_CONTEXT(hPciDma, pPciDma);
	if ( NULL == pPciDma )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Pci_AcquireMutex;
	}

	BDBG_ASSERT( NULL != phMutex );
	*phMutex = pPciDma->hDma->hMutex;

	/* hPciDma passed the magic check, we trust every thing pointed by it now */
	eResult = BKNI_AcquireMutex(pPciDma->hDma->hMutex);

  BDMA_P_Done_Pci_AcquireMutex:
	return BERR_TRACE(eResult);
}


/***************************************************************************
 *
 * Static tool functions used in this file
 *
 ***************************************************************************/

/*--------------------------------------------------------------------------
 * To be called to get PciDma context ptr from a Tran ptr. The PciDma handle
 * from the Tran record is validated by black magic, and the Engine Id is
 * checked to make sure the Tran was really created with PciDma handle
 */
static BERR_Code BDMA_P_Pci_GetPciDmaPtrFromTran_isr(
	BDMA_P_QueueEntry *    pTran,
	BDMA_P_Pci_Context **  ppPciDma )
{
	/* assert para from our private code */
	BDBG_ASSERT( NULL != pTran );
	BDBG_ASSERT( NULL != ppPciDma );

	*ppPciDma = (BDMA_P_Pci_Context *) BDMA_P_Queue_GetEngine_isr( pTran->hQueue );

	if (NULL != *ppPciDma)
		return BERR_SUCCESS;
	else
		return BERR_TRACE(BERR_INVALID_PARAMETER);
}

#define BDMA_P_PCI_WORD_ALIGN_MASK          0x3
/*--------------------------------------------------------------------------
 * To be called to set block info for a block, it does validate the block 
 * info provided
 */
static BERR_Code BDMA_P_Pci_SetBlockDesc_isr(
	BDMA_P_Pci_Context *        pPciDma,
	BDMA_P_QueueEntry *         pBlock,
	bool                        bCachedDscrp,
	uint32_t                    ulMemBusAddr,
	uint32_t                    ulPciBusAddr,
	uint32_t                    ulBlockSize,
	BDMA_TranDir                eTranDirection )
{
	BERR_Code  eResult = BERR_SUCCESS;
	uint32_t *pulDesc;
	uint32_t  ulTranDir;
	uint32_t  ulSwapMode;
	
	/* assert para from our private code */
	BDBG_ASSERT( NULL != pPciDma );
	BDBG_ASSERT( NULL != pBlock );

	/* validate the parameters passed in from user */
	if ( eTranDirection >= BDMA_TranDir_eInvalid )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Pci_SetBlockDesc;
	}
	
	if ( (ulPciBusAddr | ulMemBusAddr | ulBlockSize) &
		 BDMA_P_PCI_WORD_ALIGN_MASK )
	{
		eResult = BERR_TRACE(BDMA_ERR_NOT_WORD_ALIGNED);
		goto BDMA_P_Done_Pci_SetBlockDesc;
	}
	if ( (4>ulBlockSize) || (ulBlockSize>BDMA_P_PCI_MAX_BLOCK_SIZE) )
	{
		eResult = BERR_TRACE(BDMA_ERR_SIZE_OUT_RANGE);
		goto BDMA_P_Done_Pci_SetBlockDesc;
	}

	/* calc HW value for tran dir and swap mode */
	ulTranDir = ( (BDMA_TranDir_ePciDeviceToSdram == eTranDirection)? 
				  (BCHP_PCI_DMA_DESC_WORD2_DMA_DIRECTION_PCI_TO_MEMORY) :
				  (BCHP_PCI_DMA_DESC_WORD2_DMA_DIRECTION_MEMORY_TO_PCI) );
	if ( BDMA_SwapMode_eNoSwap == pPciDma->eSwapMode )
	{
		ulSwapMode = BCHP_PCI_DMA_DESC_WORD3_ENDIAN_XLATE_MODE_NO_SWAP;
	}
	else if ( BDMA_SwapMode_e0123To3210 == pPciDma->eSwapMode )
	{
		ulSwapMode = BCHP_PCI_DMA_DESC_WORD3_ENDIAN_XLATE_MODE_SWAP_32;
	}
	else
	{
		ulSwapMode = BCHP_PCI_DMA_DESC_WORD3_ENDIAN_XLATE_MODE_SWAP_16_IN_32;
	}

	pulDesc = (bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;
	
	/* set descriptor: only the last block needs to set INTR_ENABLE and LAST bits,
	 * and the INTR_ENABLE is always set, on matter it is a sync or async Tran. */
	*(pulDesc  ) = BCHP_FIELD_DATA( PCI_DMA_DESC_WORD0, MEMORY_ADDRESS,  ulMemBusAddr );
	*(pulDesc+1) = BCHP_FIELD_DATA( PCI_DMA_DESC_WORD1, PCI_ADDRESS, ulPciBusAddr );
	*(pulDesc+2) =
		BCHP_FIELD_DATA( PCI_DMA_DESC_WORD2, DMA_DIRECTION, ulTranDir ) |
		BCHP_FIELD_DATA( PCI_DMA_DESC_WORD2, DMA_TRANSFER_SIZE, ulBlockSize );
	*(pulDesc+3) =
		BCHP_FIELD_DATA( PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR,
						 (pBlock + 1)->ulDescPhysAddr >>
						 BCHP_PCI_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT ) |
		BCHP_FIELD_DATA( PCI_DMA_DESC_WORD3, ENDIAN_XLATE_MODE, ulSwapMode );

  BDMA_P_Done_Pci_SetBlockDesc:
	return eResult;
}

#define BDMA_P_PCI_STATUS_IDLE    BCHP_PCI_DMA_STATUS_DMA_STATUS_IDLE_STATE
#define BDMA_P_PCI_STATUS_BUSY    BCHP_PCI_DMA_STATUS_DMA_STATUS_BUSY_STATE
#define BDMA_P_PCI_STATUS_SLEEP   BCHP_PCI_DMA_STATUS_DMA_STATUS_SLEEP_STATE
#define BDMA_P_PCI_WAKE_1ST_DESC  1
#define BDMA_P_PCI_WAKE_LAST_DESC 0

#ifndef BCHP_PCI_DMA_CTRL_DMA_BURST_SIZE_SINGLE
#define BCHP_PCI_DMA_CTRL_DMA_BURST_SIZE_SINGLE BCHP_PCI_DMA_CTRL_DMA_BURST_SIZE_BURST_1
#endif
#ifndef BCHP_PCI_DMA_CTRL_DMA_BURST_SIZE_BURST
#define BCHP_PCI_DMA_CTRL_DMA_BURST_SIZE_BURST  BCHP_PCI_DMA_CTRL_DMA_BURST_SIZE_BURST_16
#endif

/*--------------------------------------------------------------------------
 * To be called to modify the last act block's desc to work as last block,
 * and to flush cached descriptors
 */
static BERR_Code BDMA_P_Pci_Tran_PrepareStart_isr(
	BDMA_P_QueueEntry *      pTran,
	BDMA_P_QueueHandle       hQueue,
	BMEM_Handle              hMemory,
	uint32_t                 ulNumActBlocks )  /* new NumActBlocks */
{
	BDMA_P_QueueContext * pQueue;
	BDMA_P_QueueEntry *pBlock;
	uint32_t *pulDesc;
	uint32_t  ulNextOffset;
	uint32_t  ulWord3;
	volatile uint32_t  ulCheck;
#if BDMA_P_CHECK_NEXT_DESC	
	int iLoopCntr;
#endif
	
	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );
	
	/* in the case of re-use, the previous "last act block" might have wrong NEXT_DESC */
	pBlock = pTran + (pTran->ulNumActBlocks - 1);
	if ((ulNumActBlocks != pTran->ulNumActBlocks) &&
		(0 < pTran->ulNumActBlocks))
	{
		pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;

		*(pulDesc+2) &= ~(
			BCHP_MASK(PCI_DMA_DESC_WORD2, INTERRUPT_ENABLE));
		ulWord3 = *(pulDesc+3) & ~(
			BCHP_MASK(PCI_DMA_DESC_WORD3, LAST_RECORD_INDICATOR) |
			BCHP_MASK(PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR));

		ulNextOffset =
			(pBlock + 1)->ulDescPhysAddr >> BCHP_PCI_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;
	 
		*(pulDesc+3) = ulWord3 |
			BCHP_FIELD_DATA( PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR, ulNextOffset);
	}

	/* modify new "last act block" desc */
	pBlock = pTran + (ulNumActBlocks - 1);
	pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;
	*(pulDesc+2) |= 
		BCHP_FIELD_DATA(PCI_DMA_DESC_WORD2, INTERRUPT_ENABLE, BDMA_P_PCI_FIRE_INTR);
	ulWord3 = *(pulDesc+3) & ~(
		BCHP_MASK(PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR));
	*(pulDesc+3) = ulWord3 |
		BCHP_FIELD_DATA(PCI_DMA_DESC_WORD3, LAST_RECORD_INDICATOR, BDMA_P_PCI_LAST_BLK);
	
	/* check block setting */
#if BDMA_P_CHECK_NEXT_DESC	
	pBlock = pTran;
	for ( iLoopCntr = 0; iLoopCntr < (int) ulNumActBlocks; iLoopCntr++ )
	{
		uint32_t ulSeeNextOffset;
		
		ulNextOffset =
			(pBlock + 1)->ulDescPhysAddr >> BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;
		pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;
		ulSeeNextOffset = BCHP_GET_FIELD_DATA(*(pulDesc+3), PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR);
		if ((false == pBlock->bBlockInfoSet) ||
			(ulNextOffset != ulSeeNextOffset))
		{
			BDBG_ASSERT((false == pBlock->bBlockInfoSet) ||
						(ulNextOffset == ulSeeNextOffset));
			return BERR_TRACE(BDMA_ERR_BLOCK_INFO_UNSET);
		}

		pBlock += 1;
	}
#endif
	
	if (pTran->bCachedDscrp)
		BMEM_FlushCache_isr(hMemory, pTran->pulCachedDescAddr,
							ulNumActBlocks * BDMA_P_PCI_NUM_WORDS_PER_DESC * 4);

	/* read back with un-cached addr to ensure previous writing completes */
	ulCheck = *((pTran + (ulNumActBlocks - 1))->pulDescAddr + 3);
	if ((ulCheck & BCHP_MASK(PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR)) != 0) 	
	{
		BDBG_ERR(("desc writing error with preparing start"));
	}
	
	return BERR_SUCCESS;
}
/*--------------------------------------------------------------------------
 * To be called to modify the last act block's desc to work as last block,
 * and to flush cached descriptors
 */
static BERR_Code BDMA_P_Pci_Tran_PrepareStartSubset_isr(
	BDMA_P_QueueEntry *      pTran,
	BDMA_P_QueueHandle       hQueue,
	BMEM_Handle              hMemory,
	uint32_t                 ulFirstBlock,
	uint32_t                 ulNumActBlocks)  /* new NumActBlocks */

{
	BDMA_P_QueueContext * pQueue;
	BDMA_P_QueueEntry *pBlock;
	uint32_t *pulDesc;
	uint32_t  ulNextOffset, i=0;
	uint32_t  ulWord3;
	volatile uint32_t  ulCheck;
#if ((BDMA_P_CHECK_NEXT_DESC) && (BDBG_DEBUG_BUILD)) || \
	((BDMA_P_CHECK_SCATTER_GATHER) && (BDBG_DEBUG_BUILD))
	int iLoopCntr;
#endif
	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );

	/* in the case of re-use, the previous "last act block" might have wrong NEXT_DESC */
	if ((ulNumActBlocks != pTran->ulNumActBlocks) &&
		(0 < pTran->ulNumActBlocks))
	{
		pBlock = pTran + pTran->ulFirstBlock + (pTran->ulNumActBlocks - 1);
		pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;

		*(pulDesc+2) &= ~(
			BCHP_MASK(PCI_DMA_DESC_WORD2, INTERRUPT_ENABLE));
		ulWord3 = *(pulDesc+3) & ~(
			BCHP_MASK(PCI_DMA_DESC_WORD3, LAST_RECORD_INDICATOR) |
			BCHP_MASK(PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR));

		ulNextOffset =
			(pBlock + 1)->ulDescPhysAddr >> BCHP_PCI_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;
	 
		*(pulDesc+3) = ulWord3 |
			BCHP_FIELD_DATA( PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR, ulNextOffset);
	}

    pTran->ulFirstBlock = ulFirstBlock;
	pBlock = pTran + ulFirstBlock;
	while (i<(ulNumActBlocks - 1))
	{
		pBlock = pTran + ulFirstBlock + i;
		pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;

		*(pulDesc+2) &=~
			BCHP_FIELD_DATA( PCI_DMA_DESC_WORD2, INTERRUPT_ENABLE,	BDMA_P_PCI_FIRE_INTR );


		ulNextOffset = (pBlock + 1)->ulDescPhysAddr >> BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;

		*(pulDesc+3) |= BCHP_FIELD_DATA( PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR, ulNextOffset );
		i++;
	}
	/* modify new "last act block" desc */
	pBlock = pTran + ulFirstBlock + (ulNumActBlocks - 1);
	pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;
	*(pulDesc+2) |= 
		BCHP_FIELD_DATA(PCI_DMA_DESC_WORD2, INTERRUPT_ENABLE, BDMA_P_PCI_FIRE_INTR);
	ulWord3 = *(pulDesc+3) & ~(
		BCHP_MASK(PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR));
	*(pulDesc+3) = ulWord3 |
		BCHP_FIELD_DATA(PCI_DMA_DESC_WORD3, LAST_RECORD_INDICATOR, BDMA_P_PCI_LAST_BLK);

#if (BDMA_P_CHECK_NEXT_DESC) && (BDBG_DEBUG_BUILD)
	pBlock = pTran;
	for ( iLoopCntr = 0; iLoopCntr < (int) ulNumActBlocks; iLoopCntr++ )
	{
		uint32_t ulSeeNextOffset;

		ulNextOffset =
			(pBlock + 1)->ulDescPhysAddr >> BCHP_PCI_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;
		pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;
		ulSeeNextOffset = BCHP_GET_FIELD_DATA(*(pulDesc+3), PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR);

		if ((ulNextOffset != ulSeeNextOffset) ||
			(false == pBlock->bBlockInfoSet))
		{
			BDBG_ASSERT((ulNextOffset == ulSeeNextOffset) ||
						(false == pBlock->bBlockInfoSet));
			return BERR_TRACE(BDMA_ERR_BLOCK_INFO_UNSET);
		}
	}
#endif

	/* flush cached desc mem */
	if (pTran->bCachedDscrp)
		BMEM_FlushCache_isr(hMemory, pTran->pulCachedDescAddr,
							ulNumActBlocks * BDMA_P_PCI_NUM_WORDS_PER_DESC * 4);

	/* read back with un-cached addr to ensure previous writing completes */
	ulCheck = *((pTran + ulFirstBlock + (ulNumActBlocks - 1))->pulDescAddr + 3);
	if ((ulCheck & BCHP_MASK(PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR)) != 0)
	{
		BDBG_ERR(("desc writing error with preparing start"));
	}

	return BERR_SUCCESS;
}

#ifndef BCHP_PCI_DMA_SCRATCH0
#define BCHP_PCI_DMA_SCRATCH0 BCHP_PCI_DMA_DMA_SCRATCH0
#endif
/*--------------------------------------------------------------------------
Callback from base-module queue to wait till next descriptor safe to send to HW
*/
static void BDMA_P_Pci_SafeSendToHw_isr(
										BDMA_P_QueueHandle      hQueue,  
										BDMA_P_HwStatus*        peHwStatus
										)
{

	uint32_t  eStatus = (uint32_t)(*peHwStatus);
	BDMA_P_Pci_Context *pPciDma;
	BREG_Handle  hRegister;
	uint32_t   ulRegValue;
	BDMA_P_QueueContext * pQueue;

	

	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );

	if (NULL == pQueue->pHeadTranInQ)
		return;


	pPciDma = (BDMA_P_Pci_Context *)(pQueue->hEngine);
	hRegister = BDMA_P_GetRegisterHandle(pPciDma->hDma);
	while (eStatus ==BDMA_P_PCI_STATUS_BUSY)
	{
		/* read CUR_DESC after STATUS, --> less chance to get wrong CUR_DESC?  */
		ulRegValue = BREG_Read32( hRegister, BCHP_PCI_DMA_STATUS);
		eStatus    = BCHP_GET_FIELD_DATA( ulRegValue, PCI_DMA_STATUS, DMA_STATUS );
	}

	switch (eStatus){
		   case (BDMA_P_PCI_STATUS_IDLE):
			   *peHwStatus = BDMA_P_HwStatus_eIdle;
			   break;
		   case (BDMA_P_PCI_STATUS_SLEEP):
			   *peHwStatus = BDMA_P_HwStatus_eSleep;
			   break;
	}
}

/*--------------------------------------------------------------------------
 * Callback from base-module queue to read the HW status 
 */
static void BDMA_P_Pci_ReadHwRegs_isr(
	void  *               pvEngine,
	BDMA_P_HwStatus  *    peHwStatus,
	uint32_t  *           pulCurrDesc,
	uint32_t  *           pulScratchReg )
{
	BDMA_P_Pci_Context *pPciDma;
	BREG_Handle  hRegister;
	uint32_t   ulRegValue;
	uint32_t   ulStatus;
	
	/* assert para from our private code */
	BDBG_ASSERT( NULL != pvEngine);
	BDBG_ASSERT( NULL != peHwStatus );
	BDBG_ASSERT( NULL != pulCurrDesc );
	BDBG_ASSERT( NULL != pulScratchReg );

	pPciDma = (BDMA_P_Pci_Context *)(pvEngine);
	hRegister = BDMA_P_GetRegisterHandle(pPciDma->hDma);
  
	/* read CUR_DESC after STATUS, --> less chance to get wrong CUR_DESC?  */
	ulRegValue = BREG_Read32( hRegister, BCHP_PCI_DMA_STATUS );
	ulStatus  = BCHP_GET_FIELD_DATA( ulRegValue, PCI_DMA_STATUS, DMA_STATUS );

	ulRegValue = BREG_Read32( hRegister, BCHP_PCI_DMA_CUR_DESC_ADDR );
	*pulCurrDesc =
		BCHP_GET_FIELD_DATA( ulRegValue, PCI_DMA_CUR_DESC_ADDR, DMA_CUR_DESC ) << 4;

	*pulScratchReg = BREG_Read32( hRegister, BCHP_PCI_DMA_SCRATCH0 );

	if ( BDMA_P_PCI_STATUS_IDLE == ulStatus )
	{
		BDBG_MSG(( "Pci DMA engine in idle state!" ));
		*peHwStatus = BDMA_P_HwStatus_eIdle;
	}
	else if ( BDMA_P_PCI_STATUS_BUSY == ulStatus )
	{
		*peHwStatus = BDMA_P_HwStatus_eBusy;
	}
	else if ( BDMA_P_PCI_STATUS_SLEEP == ulStatus )
	{
		*peHwStatus = BDMA_P_HwStatus_eSleep;
	}
	else 
	{
		BDBG_ERR(( "PCI DMA engine in Reservered state!!!" ));
		*peHwStatus = BDMA_P_HwStatus_eUnknown;
		BDBG_ASSERT(0);
	}
}

/*--------------------------------------------------------------------------
 * Callback from base-module queue to append two Trans' descriptors
 */
static void BDMA_P_Pci_AppendTranDesc_isr(
	BDMA_P_QueueEntry *      pCurrTran,
	BDMA_P_QueueEntry *      pNextTran )
{
	uint32_t  *pulLastDesc ;
	uint32_t  ulNewDescPhysAddr;
	uint32_t  ulWord3;
	
	BDBG_ASSERT( NULL != pCurrTran );
	BDBG_ASSERT( NULL != pNextTran );

	ulNewDescPhysAddr = pNextTran->ulDescPhysAddr;	
	pulLastDesc = (pCurrTran + (pCurrTran->ulNumActBlocks - 1))->pulDescAddr;
	ulWord3 = *(pulLastDesc+3) & ~(
			BCHP_MASK(PCI_DMA_DESC_WORD3, LAST_RECORD_INDICATOR) |
			BCHP_MASK(PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR));
	*(pulLastDesc+3) = ulWord3 |
		BCHP_FIELD_DATA( PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR,
			ulNewDescPhysAddr >> BCHP_PCI_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT );
}

/*--------------------------------------------------------------------------
 * Callback from base-module queue to send one Tran to HW. 
 * 
 * Note: It assume block info is already set for all blocks in this Tran
 */
static void BDMA_P_Pci_SendToHw_isr(
	BDMA_P_QueueHandle       hQueue,
	BDMA_P_HwStatus          eHwStatus,
	bool                     bAppendInHw )
{
	BDMA_P_Pci_Context *pPciDma;
	BDMA_P_QueueContext * pQueue;
	BDMA_P_QueueEntry * p1stTran;
	BDMA_P_QueueEntry * pLastTran;
	BREG_Handle  hRegister;
	uint32_t  ulNewDescPhysAddr;
	uint32_t  ulWord3;
	uint32_t  *pulLastDesc;
	uint32_t  ulMaxBurstSize;
	volatile uint32_t  ulCheck;
	
	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );

	pPciDma = (BDMA_P_Pci_Context *)(pQueue->hEngine);
	hRegister = BDMA_P_GetRegisterHandle(pPciDma->hDma);
	
	p1stTran = pQueue->pHeadTranInQ;
	if (NULL == p1stTran)
	{
		BDBG_ASSERT( p1stTran );
		return;
	}
	ulNewDescPhysAddr = pQueue->pHeadTranInQ->ulDescPhysAddr;
		
	/* write scratch reg so that other BDMA instance could check our status */
	pLastTran = pQueue->pTailTranInQ;
	BDBG_ASSERT( pLastTran );
	BREG_Write32( hRegister, BCHP_PCI_DMA_SCRATCH0,
				  pQueue->ul1stDescPhysAddr | BDMA_P_WATER_MARK_SG_CLOSE );

	/* send Trans queued in SW into HW */
	pLastTran = pQueue->pTailTranInHw;
	if (bAppendInHw && (NULL != pLastTran))
	{
		/* dynamically link to last tran in hw, which might and might not finished */
		pulLastDesc = (pLastTran + (pLastTran->ulNumActBlocks - 1))->pulDescAddr;
		ulWord3 = *(pulLastDesc+3) & ~(
			BCHP_MASK( PCI_DMA_DESC_WORD3, LAST_RECORD_INDICATOR) |
			BCHP_MASK( PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR));
		*(pulLastDesc+3) = ulWord3 |
			BCHP_FIELD_DATA( PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR,
				 ulNewDescPhysAddr >> BCHP_PCI_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT);

		/* read back to force that mem writing completes before reg writing */
		ulCheck = *(pulLastDesc+3);
		
		BREG_Write32( hRegister, BCHP_PCI_DMA_LIST_CTRL,
			BCHP_FIELD_DATA(PCI_DMA_LIST_CTRL, WAKE_MODE, BDMA_P_PCI_WAKE_LAST_DESC) |
			BCHP_FIELD_DATA(PCI_DMA_LIST_CTRL, WAKE,      1) );
	}
	else
	{
        ulNewDescPhysAddr = pQueue->pHeadTranInQ->ulDescPhysAddr 
			+ ( pQueue->pHeadTranInQ->ulFirstBlock * 4 * BDMA_P_PCI_NUM_WORDS_PER_DESC);
		/* must start from FIRST_DESC now */
		BREG_Write32( hRegister, BCHP_PCI_DMA_FIRST_DESC_ADRS,
			BCHP_FIELD_DATA(PCI_DMA_FIRST_DESC_ADRS, DESC_ADRS,
			ulNewDescPhysAddr >> BCHP_PCI_DMA_FIRST_DESC_ADRS_DESC_ADRS_SHIFT) );

		ulMaxBurstSize = ( (BDMA_MaxBurstSize_eBurst1 == pPciDma->eMaxBurstSize )?
						   (BCHP_PCI_DMA_CTRL_DMA_BURST_SIZE_SINGLE) :
						   (BCHP_PCI_DMA_CTRL_DMA_BURST_SIZE_BURST) );

		if ( BDMA_P_HwStatus_eIdle != eHwStatus )
		{
			BREG_Write32( hRegister, BCHP_PCI_DMA_CTRL,
				BCHP_FIELD_DATA( PCI_DMA_CTRL, DMA_BURST_SIZE, ulMaxBurstSize ) |
				BCHP_FIELD_DATA( PCI_DMA_CTRL, DMA_RUN_STOP,
					BCHP_PCI_DMA_CTRL_DMA_RUN_STOP_RUN ) );

			BREG_Write32( hRegister, BCHP_PCI_DMA_LIST_CTRL,
				BCHP_FIELD_DATA(PCI_DMA_LIST_CTRL, WAKE_MODE, BDMA_P_PCI_WAKE_1ST_DESC) |
				BCHP_FIELD_DATA(PCI_DMA_LIST_CTRL, WAKE,      1) );		
		}
		else /* never started */
		{
			BREG_Write32( hRegister, BCHP_PCI_DMA_CTRL,
				BCHP_FIELD_DATA( PCI_DMA_CTRL, DMA_BURST_SIZE, ulMaxBurstSize ) |
				BCHP_FIELD_DATA( PCI_DMA_CTRL, DMA_RUN_STOP,
					BCHP_PCI_DMA_CTRL_DMA_RUN_STOP_RUN ) );
		}
	}
}

/*--------------------------------------------------------------------------
 * Callback from base-module queue to check NEXT_DESC_ADDR and print err msg
 * dma engine reg read error happens
 */
static void BDMA_P_Pci_CheckNextDesc_isr(
	BDMA_P_QueueEntry *      pCurrTran,
	BDMA_P_QueueEntry *      pNextTran )
{
	BDMA_P_QueueEntry *pBlock;
	uint32_t  *pulDesc;
	uint32_t  ulSeeNextOffset, ulNextOffset;
	int  iLoopCntr;
	
	BDBG_ASSERT( NULL != pCurrTran );

	pBlock = pCurrTran;			
	for ( iLoopCntr = 0; iLoopCntr < (int) pCurrTran->ulNumActBlocks; iLoopCntr++ )
	{
	{
		if (NULL != pNextTran)
			ulNextOffset = ((iLoopCntr < (int) (pCurrTran->ulNumActBlocks - 1))?
							(pBlock + 1)->ulDescPhysAddr : pNextTran->ulDescPhysAddr);
		else
			ulNextOffset = ((iLoopCntr < (int) (pCurrTran->ulNumActBlocks - 1))?
							(pBlock + 1)->ulDescPhysAddr : 0);
		pulDesc = pBlock->pulDescAddr;
		ulSeeNextOffset = (BCHP_GET_FIELD_DATA(*(pulDesc+3), PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR) <<
						   BCHP_PCI_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT);
		
		if (ulNextOffset != ulSeeNextOffset)
		{
			BDBG_ERR(("Bad next_desc: desc 0x%08lx, next_desc 0x%08lx (should be 0x%08lx)",
					  pBlock->ulDescPhysAddr, ulSeeNextOffset, ulNextOffset));
			BDBG_ERR(("tran_desc 0x%08lx, NumActBlk %d, Blck %d",
					  pCurrTran->ulDescPhysAddr, pCurrTran->ulNumActBlocks, iLoopCntr));

			/*BDBG_ASSERT(ulNextOffset != ulSeeNextOffset);*/
		}
#if 0
		else if (iLoopCntr == (int) (pCurrTran->ulNumActBlocks - 1))
		{
			/* XXX extra print make system busy, and shows HW reg reading problem */
			BDBG_MSG(("good next_desc: desc 0x%08lx (blck %d), next_desc 0x%08lx",
					  pBlock->ulDescPhysAddr, iLoopCntr, ulSeeNextOffset));
		}
#endif
		pBlock++;
	}



		
		ulNextOffset = ((iLoopCntr == (int) (pCurrTran->ulNumActBlocks - 1)) &&
						(NULL != pNextTran))?
			pNextTran->ulDescPhysAddr : (pBlock + 1)->ulDescPhysAddr;
		pulDesc = (pCurrTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;
		ulSeeNextOffset = (BCHP_GET_FIELD_DATA(*(pulDesc+3), PCI_DMA_DESC_WORD3, NEXT_DESC_ADDR) <<
						   BCHP_PCI_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT);
		
		if (ulNextOffset != ulSeeNextOffset)
		{
			BDBG_ERR(("Bad next_desc: desc 0x%80x, next_desc 0x%80x (should be 0x%80x)",
					  pulDesc, ulSeeNextOffset, ulNextOffset));
			/*BDBG_ASSERT(ulNextOffset != ulSeeNextOffset);*/
		}
	}
}


/***************************************************************************
 *
 * "Dma Done" interrupt handler (call back ) function
 *
 ***************************************************************************/

/***************************************************************************
 * To be called by the BINT module as the "dma done" interrupt handler (call
 * back func). It records the just completed Tran's result, calls the user's
 * call back function to futher process the DMA result if the Tran is async,
 * (i.e. if it has a user's call back func).
 */
static void BDMA_P_Pci_DoneCallBack_isr(
	void *                pParm1,  /* pPciDma */
	int                   parm2 )  /* not used */
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Pci_Context *  pPciDma;

	BSTD_UNUSED(parm2);
	BDMA_P_PCI_GET_CONTEXT(pParm1, pPciDma);
	if ( NULL == pPciDma )
	{
		eResult = BERR_TRACE( BERR_INVALID_PARAMETER );
		BDBG_ERR(( "Invaid pParm1 to BDMA_P_Pci_DoneCallBack_isr_isr!" ));
		goto BDMA_P_Done_Pci_DoneCallBack_isr;
	}

	/* read status from HW and update records */
	BDMA_P_Queue_CheckHwAndSendNewReq_isr( pPciDma->hQueue );

  BDMA_P_Done_Pci_DoneCallBack_isr:
	return;
}


/***************************************************************************
 *
 * API support functions
 *
 ***************************************************************************/

/*--------------------------------------------------------------------------
 * To be called to init pci dma engine: 1). init Hw, 2). create queue, 3).
 * create call-back, 4). init crpto setting.
 */
BERR_Code BDMA_P_Pci_Init(
	BDMA_P_Pci_Context  * pPciDma,
	const BDMA_Pci_Settings *pSettings,	
	uint32_t              ulL2IntrBintId )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_Handle  hDma;
	BMEM_Handle  hMemory = NULL;  /* init to silence linux comipler */
	BDMA_P_QueueHandle  hQueue = NULL;
	BINT_CallbackHandle  hCallBack = NULL;

	BDBG_ASSERT( NULL != pPciDma);
	
	BDMA_P_PCI_SET_BLACK_MAGIC( pPciDma );

	/* init HW */
	hDma = pPciDma->hDma;
	
	/* create Tran queue */
	hMemory = BDMA_P_GetMemoryHandle( hDma );
	eResult = BDMA_P_Queue_Create(
		&hQueue, (void *) pPciDma, hMemory, BDMA_P_PCI_NUM_WORDS_PER_DESC,
		(BDMA_P_CmnSettings *)pSettings, BDMA_P_Pci_ReadHwRegs_isr,
		BDMA_P_Pci_AppendTranDesc_isr, BDMA_P_Pci_SendToHw_isr,
		BDMA_P_Pci_CheckNextDesc_isr, BDMA_P_Pci_SafeSendToHw_isr);
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Pci_Init );
	pPciDma->hQueue = hQueue;

	if (pSettings->bSupportCallBack)
	{	
		/* create done-interrupt call back
		 * no need to create call back for BCHP_INT_ID_PCI_DMA_ERR_INTR,
		 * since PI already checks size / error without sending HW ??? */
		/* BCHP_INT_ID_PCI_DMA_0_INTR, or BCHP_INT_ID_PCI_DMA_1_INTR */
		eResult = BINT_CreateCallback(
			&hCallBack, BDMA_P_GetInterruptHandle( hDma ),
			ulL2IntrBintId, BDMA_P_Pci_DoneCallBack_isr,
			(void *)pPciDma, 0 );
		BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Pci_Init );
		pPciDma->hCallBack = hCallBack;
	
		eResult = BINT_ClearCallback( hCallBack );
		BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Pci_Init );
		eResult = BINT_EnableCallback( hCallBack );
		BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Pci_Init );
	}
	
	/* init op */
	pPciDma->eSwapMode = BDMA_SwapMode_eNoSwap;
	pPciDma->eMaxBurstSize = BDMA_MaxBurstSize_eBurst16;

	return eResult;
	
  BDMA_P_Err_Pci_Init:
	BDMA_P_Pci_UnInit( pPciDma );

	return BERR_TRACE(eResult);	
}

/*--------------------------------------------------------------------------
 * To be called to uninit pci dma engine: destroy queue and call-back
 */
BERR_Code BDMA_P_Pci_UnInit(
	BDMA_P_Pci_Context  * pPciDma )
{
	BDBG_ASSERT( NULL != pPciDma);

	if ( NULL != pPciDma->hCallBack )
	{
		BINT_DisableCallback( pPciDma->hCallBack );
		BINT_DestroyCallback( pPciDma->hCallBack );
	}
	if ( NULL != pPciDma->hQueue )
		BDMA_P_Queue_Destroy( pPciDma->hQueue, BDMA_P_GetMemoryHandle(pPciDma->hDma) );

	return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BDMA_P_Pci_Create(
	BDMA_Handle           hDma,
	BDMA_PciDmaEngine     eEngine,
	const BDMA_Pci_Settings *pSettings,	
	BDMA_Pci_Handle *     phPciDma )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Context *  pDma;
	BDMA_P_Pci_Context  *pPciDma = NULL;

	uint32_t  ulL2IntrBintId;

	if ( NULL != phPciDma )
		*phPciDma = NULL;
	
	BDMA_P_MAIN_GET_CONTEXT( hDma, pDma );
	if ( (NULL == phPciDma) || (NULL == pDma) || (eEngine >= BDMA_PciDmaEngine_eUnknown))
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Err_Pci_Create;		
	}
	
	if ( NULL != BDMA_P_GetPciDmaHandle(hDma, eEngine) )
	{
		/* memeory dma engine sub-module is already created */
		eResult = BERR_TRACE(BDMA_ERR_ENGINE_OCCUPIED);
		goto BDMA_P_Err_Pci_Create;		
	}
	
	/* allocate pci dma sub-module's main context */
	pPciDma = (BDMA_P_Pci_Context *)BKNI_Malloc( sizeof(BDMA_P_Pci_Context) );
	if ( NULL == pPciDma )
	{
		eResult = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto BDMA_P_Err_Pci_Create;
	}
	BKNI_Memset((void*)pPciDma, 0x0, sizeof(BDMA_P_Pci_Context));

	ulL2IntrBintId = BCHP_INT_ID_PCI_DMA_DONE;

	/* init pci dma */
	pPciDma->hDma = hDma;
	pPciDma->eEngine = eEngine;
	eResult = BDMA_P_Pci_Init( pPciDma, pSettings, ulL2IntrBintId );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Pci_Create );

	/* connect pci dma sub-module to dma module's main context */
	BDMA_P_SetPciDmaHandle( hDma, eEngine, pPciDma );

	*phPciDma = pPciDma;
	return eResult;

  BDMA_P_Err_Pci_Create:

	if ( NULL != pPciDma )
		BDMA_P_PCI_DESTROY_CONTEXT( pPciDma );

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Pci_Destroy(
	BDMA_Pci_Handle       hPciDma )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Pci_Context *  pPciDma;
	
	BDMA_P_PCI_GET_CONTEXT(hPciDma, pPciDma);
	if ( NULL == pPciDma )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Pci_Destroy;
	}

	/* disconnect pci dma sub-module to dma module's main context */
	BDMA_P_SetPciDmaHandle( pPciDma->hDma, pPciDma->eEngine, NULL );

	/* destroy callBack and queue */
	BDMA_P_Pci_UnInit( pPciDma );
	BDMA_P_PCI_DESTROY_CONTEXT( pPciDma );

  BDMA_P_Done_Pci_Destroy:
	return eResult;	
}

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Pci_SetSwapMode_isr(
	BDMA_Pci_Handle       hPciDma,
	BDMA_SwapMode         eSwapMode )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Pci_Context *  pPciDma;
	
	BDMA_P_PCI_GET_CONTEXT(hPciDma, pPciDma);
	if ( (NULL == pPciDma) ||
		 (eSwapMode >= BDMA_SwapMode_eInvalid) )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Pci_SetSwapMode;		
	}

	pPciDma->eSwapMode = eSwapMode;

  BDMA_P_Done_Pci_SetSwapMode:
	return eResult;
}

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Pci_SetMaxBurstSize_isr(
	BDMA_Pci_Handle       hPciDma,
	BDMA_MaxBurstSize     eMaxBurstSize )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Pci_Context *  pPciDma;
	
	BDMA_P_PCI_GET_CONTEXT(hPciDma, pPciDma);
	if ( (NULL == pPciDma) ||
		 (eMaxBurstSize >= BDMA_MaxBurstSize_eInvalid) )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Pci_SetMaxBurstSize;		
	}

	pPciDma->eMaxBurstSize = eMaxBurstSize;

  BDMA_P_Done_Pci_SetMaxBurstSize:
	return eResult;
}

/***************************************************************************
 * To be called to created a new Tran
 */
BERR_Code BDMA_P_Pci_Tran_Create_isr(
	BDMA_Pci_Handle          hPciDma,
	uint32_t                 ulNumBlocks,
	bool                     bCachedDesc,
	BDMA_Pci_Tran_Handle  *  phTran )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Pci_Context *  pPciDma;
	BDMA_P_QueueEntry * pTran;

	if ( NULL != phTran )
		*phTran = NULL;
	
	BDMA_P_PCI_GET_CONTEXT( hPciDma, pPciDma );
	if ( (NULL == pPciDma) || (NULL == phTran) )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Pci_Tran_Create;
	}

	eResult = BDMA_P_Queue_CreateTran_isr(
		pPciDma->hQueue, ulNumBlocks, bCachedDesc, &pTran );
	if ( BERR_SUCCESS == eResult )
		*phTran = (BDMA_Pci_Tran_Handle ) pTran;

  BDMA_P_Done_Pci_Tran_Create:
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * To be called to set the block info for a Tran block. Input block info are
 * validated against the current engine state.
 */
BERR_Code BDMA_P_Pci_Tran_SetBlockInfo_isr(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulMemBusAddr,
	uint32_t                 ulPciBusAddr,
	uint32_t                 ulBlockSize,
	BDMA_TranDir             eTranDirection )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Pci_Context *  pPciDma;
	BDMA_P_QueueEntry * pTran;
	BDMA_P_QueueEntry * pBlock;
	bool bLastBlock;	

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Pci_Tran_SetBlockInfo;
	}

	if ( pTran->ulNumTranBlocks <= ulBlockId )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Pci_Tran_SetBlockInfo;
	}

	eResult = BDMA_P_Pci_GetPciDmaPtrFromTran_isr( pTran, &pPciDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Pci_Tran_SetBlockInfo );

	/* validate addr, size and set desc */
	bLastBlock = (pTran->ulNumTranBlocks == (ulBlockId+1));	
	pBlock = pTran + ulBlockId;
	eResult = BDMA_P_Pci_SetBlockDesc_isr( pPciDma,
										   pBlock,
										   pTran->bCachedDscrp,
										   ulMemBusAddr, ulPciBusAddr,
										   ulBlockSize,
										   eTranDirection );
	if ( BERR_SUCCESS == eResult )
		pBlock->bBlockInfoSet = true;

  BDMA_P_Done_Pci_Tran_SetBlockInfo:
	return BERR_TRACE( eResult );	
}

/***************************************************************************
 * To be called by both BDMA_P_Pci_Tran_Start_isrAndCallBack and
 * BDMA_P_Pci_Tran_Start_isr. BDMA_P_Pci_Tran_Start_isr should pass NULL for
 * pCallBackFunc_isr and pUserCallBackParam
 *
 * Note: when a Tran is sent to HW, interrupt is alwayas enabled. If the 
 * Tran is async, the module's interrupt handler BDMA_P_Pci_DoneCallBack_isr 
 * will call user's callback function. In the sync case, there is no user's 
 * call back function to call, BDMA_P_Pci_DoneCallBack_isr_isr only sets the 
 * internal record of the status, the ideal is that the user will check the 
 * Tran's status in a waiting loop and process the Tran result synchronously.
 */
BERR_Code BDMA_P_Pci_Tran_Start_isr(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,
	BDMA_Pci_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Pci_Context *  pPciDma;
	BDMA_P_QueueEntry * pTran;
	
	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Pci_Tran_Start;
	}

	eResult = BDMA_P_Pci_GetPciDmaPtrFromTran_isr( pTran, &pPciDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Pci_Tran_Start );

	eResult = BDMA_P_Pci_Tran_PrepareStart_isr( pTran, pPciDma->hQueue,
		BDMA_P_GetMemoryHandle(pPciDma->hDma), ulNumActBlocks);
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Pci_Tran_Start );

	eResult = BDMA_P_Queue_StartTran_isr (
		pPciDma->hQueue, pTran, ulNumActBlocks, false,
		(BDMA_P_CallbackFunc) pCallBackFunc_isr, pUserCallBackParam );

  BDMA_P_Done_Pci_Tran_Start:
	return BERR_TRACE(eResult);
}

/***************************************************************************
 */

BERR_Code BDMA_P_Pci_Tran_StartSubset_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulFirstBlock,
	uint32_t                 ulNumActBlocks,
	BDMA_Mem_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Pci_Context *  pPciDma;
	BDMA_P_QueueEntry * pTran;


	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Pci_Tran_Start;
	}

	eResult = BDMA_P_Pci_GetPciDmaPtrFromTran_isr( pTran, &pPciDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Pci_Tran_Start );

	eResult = BDMA_P_Pci_Tran_PrepareStartSubset_isr( pTran, pPciDma->hQueue,
		BDMA_P_GetMemoryHandle(pPciDma->hDma), ulFirstBlock, ulNumActBlocks);
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Pci_Tran_Start );

	eResult = BDMA_P_Queue_StartTran_isr (
		pPciDma->hQueue, pTran, ulNumActBlocks, false,
		(BDMA_P_CallbackFunc) pCallBackFunc_isr, pUserCallBackParam );

  BDMA_P_Done_Pci_Tran_Start:
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * To be called by BDMA_Pci_Tran_GetStatus. It outputs the internal records
 * of the Tran directly.
 */
BERR_Code BDMA_P_Pci_Tran_GetStatus_isr(
	BDMA_Pci_Tran_Handle     hTran,
	BDMA_TranStatus *        peTranStatus )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Pci_Context *  pPciDma;
	BDMA_P_QueueEntry * pTran;

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Pci_Tran_GetStatus;
	}

	eResult = BDMA_P_Pci_GetPciDmaPtrFromTran_isr( pTran, &pPciDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Pci_Tran_GetStatus );

	/* read status from HW and update Trans' records */
	BDMA_P_Queue_CheckHwAndSendNewReq_isr( pPciDma->hQueue );
	*peTranStatus = pTran->eStatus;	
	return BERR_SUCCESS;

  BDMA_P_Done_Pci_Tran_GetStatus:
	return BERR_TRACE(eResult);
}


/***************************************************************************
 * To be called by BDMA_Pci_Tran_Destroy. It free the queue entries occupied
 * this Tran
 */
BERR_Code BDMA_P_Pci_Tran_Destroy_isr(
	BDMA_Pci_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Pci_Context *  pPciDma;
	BDMA_P_QueueEntry * pTran;

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Pci_Tran_Destroy;
	}

	eResult = BDMA_P_Pci_GetPciDmaPtrFromTran_isr( pTran, &pPciDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Pci_Tran_Destroy );

	/* tell queue module to free the queue entries of this */
	eResult = BDMA_P_Queue_DestroyTran_isr ( pPciDma->hQueue,
											 pTran );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Pci_Tran_Destroy );
	
	return BERR_SUCCESS;

  BDMA_P_Done_Pci_Tran_Destroy:
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * To be called by BDMA_Pci_Tran_Reset. It resets the Tran into its 
 * initially created state
 */
BERR_Code BDMA_P_Pci_Tran_Reset_isr(
	BDMA_Pci_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Pci_Context *  pPciDma;
	BDMA_P_QueueEntry * pTran;

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Pci_Tran_Reset;
	}

	eResult = BDMA_P_Pci_GetPciDmaPtrFromTran_isr( pTran, &pPciDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Pci_Tran_Reset );

	/* tell queue module to reset this Tran into initially created state */
	eResult = BDMA_P_Queue_ResetTran_isr ( pPciDma->hQueue,
										   pTran );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Pci_Tran_Reset );
	
	return BERR_SUCCESS;

  BDMA_P_Done_Pci_Tran_Reset:
	return BERR_TRACE(eResult);
}


/* End of File */
