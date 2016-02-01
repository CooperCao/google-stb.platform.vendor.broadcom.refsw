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

#include "../common/bdma_mem_priv.h"
#include "bdma_mem_sharf_priv.h"
#include "bkni.h"

#if BDMA_P_SUPPORT_MEM_DMA_ENGINE
#if (((BCHP_CHIP == 35230) && (BCHP_VER == BCHP_VER_C0)) ||(BCHP_CHIP == 7640) ||(BCHP_CHIP == 35233) )
#include "bchp_mem_dma_0.h"
#else
#include "bchp_mem_dma.h"
#endif
#endif

#include "bchp_sharf_mem_dma0.h"
#include "bchp_sharf_mem_dma1.h"
#include "bchp_common.h"
#include "bchp_hif_intr2.h"
#include "bchp_int_id_xpt_bus_if.h"
#if (BCHP_CHIP == 7440) && (BCHP_VER < BCHP_VER_C0)
#include "bchp_fgx_intr2.h"
#include "bchp_int_id_fgx_intr2.h"
#elif (BCHP_CHIP == 7440) && (BCHP_VER >= BCHP_VER_C0)
#include "bchp_fgx_to_hif_intr2.h"
#include "bchp_int_id_fgx_to_hif_intr2.h"
#elif (( BCHP_CHIP == 7630 )||(BCHP_CHIP == 7640)) 
#include "bchp_int_id_hif_intr2.h"
#elif ((BCHP_CHIP == 35230) && (BCHP_VER == BCHP_VER_C0)) 
#include "bchp_int_id_hif_intr2.h"
#include "bchp_int_id_sharf_intr2.h"
#elif (((BCHP_CHIP == 7550) && (BCHP_VER == BCHP_VER_B0)) ||(BCHP_CHIP == 35233))
#include "bchp_sharf_intr2.h"
#else
#include "bchp_shift_intr2.h"
#endif


BDBG_MODULE(BDMA_MEM_PRIV);

/* Added by Bryant and Ali for sharf interrupt */
#if ((BCHP_CHIP == 7440) && (BCHP_VER < BCHP_VER_C0))
#define BCHP_INT_ID_SHARF_MEM_DMA_0_INTR  BCHP_INT_ID_FGX_INTR2_SHARF_MEM_DMA0_DONE
#define BCHP_INT_ID_SHARF_MEM_DMA_1_INTR  BCHP_INT_ID_FGX_INTR2_SHARF_MEM_DMA1_DONE
#elif ((BCHP_CHIP == 7440) &&   (BCHP_VER >= BCHP_VER_C0)) || (BCHP_CHIP == 7630) || (BCHP_CHIP == 7640) \
	||((BCHP_CHIP == 35230) && (BCHP_VER == BCHP_VER_C0))   
#define BCHP_INT_ID_SHARF_MEM_DMA_0_INTR  BCHP_INT_ID_SHARF_MEM_DMA0_DONE
#define BCHP_INT_ID_SHARF_MEM_DMA_1_INTR  BCHP_INT_ID_SHARF_MEM_DMA1_DONE
#elif (((BCHP_CHIP == 7550) && (BCHP_VER == BCHP_VER_B0)) || (BCHP_CHIP == 35233))
#define BCHP_INT_ID_SHARF_MEM_DMA_0_INTR   \
	BCHP_INT_ID_CREATE(BCHP_SHARF_INTR2_CPU_STATUS, BCHP_SHARF_INTR2_CPU_STATUS_SHARF_MEM_DMA0_DONE_SHIFT)
#define BCHP_INT_ID_SHARF_MEM_DMA_1_INTR   \
	BCHP_INT_ID_CREATE(BCHP_SHARF_INTR2_CPU_STATUS, BCHP_SHARF_INTR2_CPU_STATUS_SHARF_MEM_DMA1_DONE_SHIFT)
#else
#define BCHP_INT_ID_SHARF_MEM_DMA_0_INTR   \
	BCHP_INT_ID_CREATE(BCHP_SHIFT_INTR2_CPU_STATUS, BCHP_SHIFT_INTR2_CPU_STATUS_SHARF_MEM_DMA0_DONE_SHIFT)
#define BCHP_INT_ID_SHARF_MEM_DMA_1_INTR   \
	BCHP_INT_ID_CREATE(BCHP_SHIFT_INTR2_CPU_STATUS, BCHP_SHIFT_INTR2_CPU_STATUS_SHARF_MEM_DMA1_DONE_SHIFT)
#endif


#define BDMA_P_SHARF_NUM_WORDS_PER_DESC       8
#define BDMA_P_SHARF_WAKE_FROM_1ST_DESC       1
#define BDMA_P_SHARF_WAKE_FROM_LAST_DESC      0
#define BDMA_P_SHARF_WAKE_UP                  1
#define BDMA_P_SHARF_RUN_START                1


#define BDMA_P_SHARF_MEM_LAST_BLK                 1
#define BDMA_P_SHARF_MEM_FIRE_INTR                1


/***************************************************************************
 * Note: assume parameters are valid. It should be in bdma_priv.c, but we
 * put it here because we want to export sharf concept to outside as less
 * as possible.
 */
static void BDMA_P_SetSharfDmaHandle(
	BDMA_Handle       hDma,
	BDMA_Sharf        eEngine,
	BDMA_Mem_Handle   hMemDma )
{
	BDMA_P_Context *     pDma;

	BDMA_P_MAIN_GET_CONTEXT(hDma, pDma);
	BDBG_ASSERT( NULL != pDma );

	if (eEngine < BDMA_Sharf_eInvalid)
		pDma->hSharfDma[eEngine] = hMemDma;  /* hMemDma will be NULL when destroy */
	else
		BDBG_ASSERT(eEngine < BDMA_Sharf_eInvalid);
}

/***************************************************************************
 * Note: assume parameters are valid. It should be in bdma_priv.c, but we
 * put it here because we want to export sharf concept to outside as less
 * as possible.
 */
static BDMA_Mem_Handle BDMA_P_GetSharfDmaHandle(
	BDMA_Handle        hDma,
	BDMA_Sharf         eEngine )
{
	BDMA_P_Context *     pDma;

	BDMA_P_MAIN_GET_CONTEXT(hDma, pDma);
	BDBG_ASSERT( NULL != pDma );

	if (eEngine < BDMA_Sharf_eInvalid)
		return pDma->hSharfDma[eEngine];  /* hMemDma will be NULL when destroy */
	else
	{
		BDBG_ASSERT(eEngine < BDMA_Sharf_eInvalid);
		return NULL;
	}
}

/*--------------------------------------------------------------------------
 * To be called to uninit mem dma engine: destroy queue and call-back
 */
static BERR_Code BDMA_P_Sharf_UnInit(
	BDMA_P_Mem_Context  * pMemDma )
{
	BDBG_ASSERT( NULL != pMemDma);

	if ( NULL != pMemDma->hCallBack )
	{
		BINT_DisableCallback( pMemDma->hCallBack );
		BINT_DestroyCallback( pMemDma->hCallBack );
	}
	if ( NULL != pMemDma->hQueue )
		BDMA_P_Queue_Destroy( pMemDma->hQueue, BDMA_P_GetMemoryHandle(pMemDma->hDma) );

	return BERR_SUCCESS;
}

/*--------------------------------------------------------------------------
 * To be called to init mem dma engine: 1). init Hw, 2). create queue, 3).
 * create call-back, 4). init crpto setting.
 */
static BERR_Code BDMA_P_Sharf_Init(
	BDMA_P_Mem_Context  * pMemDma,
	const BDMA_Mem_Settings *pSettings,
	uint32_t              ulL2IntrBintId )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_Handle  hDma;
	BMEM_Handle  hMemory = NULL;  /* init to silence linux comipler */
	BDMA_P_QueueHandle  hQueue = NULL;
	BINT_CallbackHandle  hCallBack = NULL;

	BDBG_ASSERT( NULL != pMemDma);

	BDMA_P_MEM_SET_BLACK_MAGIC( pMemDma );

	/* init HW */
	hDma = pMemDma->hDma;

	/* create Tran queue */
	hMemory = BDMA_P_GetMemoryHandle( hDma );
	eResult = BDMA_P_Queue_Create(
		&hQueue, (void *) pMemDma, hMemory, BDMA_P_SHARF_NUM_WORDS_PER_DESC,
		(BDMA_P_CmnSettings *)pSettings, BDMA_P_Sharf_ReadHwRegs_isr,
		BDMA_P_Sharf_AppendTranDesc_isr, BDMA_P_Sharf_SendToHw_isr,
		BDMA_P_Sharf_CheckNextDesc_isr, BDMA_P_Sharf_SafeSendToHw_isr);
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Sharf_Init );
	pMemDma->hQueue = hQueue;

	if (pSettings->bSupportCallBack)
	{
		/* create done-interrupt call back
		 * no need to create call back for BCHP_INT_ID_MEM_DMA_ERR_INTR,
		 * since PI already checks size / error without sending HW ??? */
		/* BCHP_INT_ID_MEM_DMA_0_INTR, or BCHP_INT_ID_MEM_DMA_1_INTR */
		eResult = BINT_CreateCallback(
			&hCallBack, BDMA_P_GetInterruptHandle( hDma ),
			ulL2IntrBintId, BDMA_P_Sharf_DoneCallBack_isr,
			(void *)pMemDma, 0 );
		BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Sharf_Init );
		pMemDma->hCallBack = hCallBack;

		eResult = BINT_ClearCallback( hCallBack );
		BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Sharf_Init );
		eResult = BINT_EnableCallback( hCallBack );
		BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Sharf_Init );
	}

	/* init op */
	pMemDma->eReadEndian = BDMA_Endian_eLittle;
	pMemDma->eSwapMode = BDMA_SwapMode_eNoSwap;
	pMemDma->eCryptMode = BDMA_CryptMode_eNoCrypt;
	pMemDma->ulKeySlot = 0;

	return eResult;

  BDMA_P_Err_Sharf_Init:
	BDMA_P_Sharf_UnInit( pMemDma );

	return BERR_TRACE(eResult);
}





/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Sharf_Create(
	BDMA_Handle           hDma,
	BDMA_Sharf            eEngine,
	const BDMA_Mem_Settings *pSettings,
	BDMA_Mem_Handle *     phMemDma )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Context *  pDma;
	BDMA_P_Mem_Context  *pMemDma = NULL;
	BDMA_P_Mem_Sharf_Context  *pSharf = NULL;
	uint32_t  ulL2IntrBintId;

	if ( NULL != phMemDma )
		*phMemDma = NULL;

	BDMA_P_MAIN_GET_CONTEXT( hDma, pDma );
	if ( (NULL == phMemDma) || (NULL == pDma) || (BDMA_Sharf_eInvalid <= eEngine) )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Err_Mem_Create;
	}

	/* If both MEM DMA modules were created, return error */
	if (NULL != BDMA_P_GetSharfDmaHandle(hDma, eEngine))
	{
		eResult = BERR_TRACE(BDMA_ERR_ENGINE_OCCUPIED);
		goto BDMA_P_Err_Mem_Create;
	}

	/* allocate mem dma sub-module's main context */
	pMemDma = (BDMA_P_Mem_Context *)BKNI_Malloc( sizeof(BDMA_P_Mem_Context) );
	if ( NULL == pMemDma )
	{
		eResult = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto BDMA_P_Err_Mem_Create;
	}
	BKNI_Memset((void*)pMemDma, 0x0, sizeof(BDMA_P_Mem_Context));

	/* allocate sharf specific context */
	pSharf = (BDMA_P_Mem_Sharf_Context *)BKNI_Malloc( sizeof(BDMA_P_Mem_Sharf_Context) );
	if ( NULL == pSharf )
	{
		eResult = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto BDMA_P_Err_Mem_Create;
	}
	BKNI_Memset((void*)pSharf, 0x0, sizeof(BDMA_P_Mem_Sharf_Context));
	pMemDma->hSharf = pSharf;

	/* TODO: find the real intr ID for sharf dma done */
	if (BDMA_Sharf_e0 == eEngine)
	{
		pMemDma->ulRegOffset = 0;
		ulL2IntrBintId = BCHP_INT_ID_SHARF_MEM_DMA_0_INTR;
	}
	else
	{
		pMemDma->ulRegOffset = BCHP_SHARF_MEM_DMA1_FIRST_DESC - BCHP_SHARF_MEM_DMA0_FIRST_DESC;
		ulL2IntrBintId = BCHP_INT_ID_SHARF_MEM_DMA_1_INTR;
	}

	/* init mem dma */
	pMemDma->hDma = hDma;
	pMemDma->eEngine = BDMA_MemDmaEngine_eUnknown;
	eResult = BDMA_P_Sharf_Init( pMemDma, pSettings, ulL2IntrBintId );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Mem_Create );

	/* init for sharf specific */
	pSharf->eEngine = eEngine;

	/* connect sharf dma sub-module to dma module's main context */
	BDMA_P_SetSharfDmaHandle( hDma, eEngine, pMemDma );

	*phMemDma = pMemDma;
	return eResult;

  BDMA_P_Err_Mem_Create:

	if ( NULL != pSharf )
		BDMA_P_MEM_SHARF_DESTROY_CONTEXT( pSharf );
	if ( NULL != pMemDma )
		BDMA_P_MEM_DESTROY_CONTEXT( pMemDma );

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Sharf_Destroy(
	BDMA_Mem_Handle       hMemDma )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;
	BDMA_P_Mem_Sharf_Context  *pSharf;

	BDMA_P_MEM_GET_CONTEXT(hMemDma, pMemDma);
	if ( NULL == pMemDma )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Destroy;
	}

	pSharf = (BDMA_P_Mem_Sharf_Context *) pMemDma->hSharf;
	if ( NULL != pSharf )
	{
		/* disconnect sharf dma sub-module to dma module's main context */
		BDMA_P_SetSharfDmaHandle( pMemDma->hDma, pSharf->eEngine, NULL );

		BDMA_P_MEM_SHARF_DESTROY_CONTEXT( pMemDma->hSharf );
	}
	else
	{
		BDBG_ASSERT( pSharf );
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Destroy;
	}

	/* destroy callBack and queue */
	BDMA_P_Sharf_UnInit( pMemDma );

	BDMA_P_MEM_DESTROY_CONTEXT( pMemDma );

  BDMA_P_Done_Mem_Destroy:
	return eResult;
}


/*--------------------------------------------------------------------------
Callback from base-module queue to wait till next descriptor safe to send to HW
*/
void BDMA_P_Sharf_SafeSendToHw_isr(
										BDMA_P_QueueHandle      hQueue,
										BDMA_P_HwStatus*        peHwStatus
										)
{
	BDMA_P_QueueEntry * pLastTran;
	bool  bAppendable = true;
	uint32_t   *pulNewDesc;
	uint32_t  ulNewDescPhysAddr;
	uint32_t  *pulLastDesc;
	uint32_t  eStatus = (uint32_t)(*peHwStatus);
	BDMA_P_Mem_Context *pMemDma;
	BREG_Handle  hRegister;
	uint32_t   ulRegValue;
	uint32_t ulNewDesc4   = 0, ulLastDesc4   = 0;
	uint32_t ulNewEncry   = 0, ulLastEncry   = 0;
	uint32_t ulNewModeSel = 0, ulLastModeSel = 0;
	BDMA_P_QueueContext * pQueue;


	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );


	if (NULL == pQueue->pHeadTranInQ)
		return;

	/* 1: Compare whether the descriptor in HW is different from the one to send*/

	/* send Trans queued in SW into HW */
	pLastTran = pQueue->pTailTranInHw;
	ulNewDescPhysAddr = pQueue->pHeadTranInQ->ulDescPhysAddr;

    pulNewDesc = pQueue->pHeadTranInQ->pulDescAddr;

	ulNewDesc4 = *(pulNewDesc+4);

	ulNewEncry	 = BCHP_GET_FIELD_DATA(ulNewDesc4, SHARF_MEM_DMA_DESC_WORD4, SG_SCRAM_START);
	ulNewModeSel = BCHP_GET_FIELD_DATA(ulNewDesc4, SHARF_MEM_DMA_DESC_WORD4, MODE_SEL);

	/*@@@ ToDo: how to decide safe for Sharf
	
	ulNewKeySel	 = BCHP_GET_FIELD_DATA(ulNewDesc4, SHARF_MEM_DMA_DESC_WORD4, KEY_SELECT);
	*/



	if(pLastTran)
	{
		pulLastDesc = (pLastTran + (pLastTran->ulNumActBlocks - 1))->pulDescAddr;
		ulLastDesc4 = *(pulLastDesc + 4);
		ulLastEncry	 = BCHP_GET_FIELD_DATA(ulLastDesc4,  SHARF_MEM_DMA_DESC_WORD4, SG_SCRAM_START);
		ulLastModeSel = BCHP_GET_FIELD_DATA(ulLastDesc4, SHARF_MEM_DMA_DESC_WORD4, MODE_SEL);

		/*@@@ ToDo: how to decide safe for Sharf
		ulLastKeySel  = BCHP_GET_FIELD_DATA(ulLastDesc4, SHARF_MEM_DMA_DESC_WORD4, KEY_SELECT);
		*/
		bAppendable = (ulLastEncry == ulNewEncry)&&
			(ulLastModeSel==ulNewModeSel);

	}

	/* 2: Safe to send the next descriptor if the continuous two are the same*/
	if(bAppendable)
		return;

	/* 3: Safe to send the next descriptor or the previous in HW is done*/

	pMemDma = (BDMA_P_Mem_Context *)(pQueue->hEngine);
	hRegister = BDMA_P_GetRegisterHandle(pMemDma->hDma);
	while (eStatus ==BCHP_SHARF_MEM_DMA0_STATUS_DMA_STATUS_Busy)
	{
		/* read CUR_DESC after STATUS, --> less chance to get wrong CUR_DESC?  */
		ulRegValue = BREG_Read32( hRegister, BCHP_SHARF_MEM_DMA0_STATUS + pMemDma->ulRegOffset );
		eStatus    = BCHP_GET_FIELD_DATA( ulRegValue, SHARF_MEM_DMA0_STATUS, DMA_STATUS );
	}

	switch (eStatus){
		   case (BCHP_SHARF_MEM_DMA0_STATUS_DMA_STATUS_Idle):
			   *peHwStatus = BDMA_P_HwStatus_eIdle;
			   break;
		   case (BCHP_SHARF_MEM_DMA0_STATUS_DMA_STATUS_Sleep):
			   *peHwStatus = BDMA_P_HwStatus_eSleep;
			   break;
	}

}


/*--------------------------------------------------------------------------
* Callback from base-module queue to read the HW status
 */
void BDMA_P_Sharf_ReadHwRegs_isr(
	void  *               pvEngine,
	BDMA_P_HwStatus  *    peHwStatus,
	uint32_t  *           pulCurrDesc,
	uint32_t  *           pulScratchReg )
{
	BDMA_P_Mem_Context *pMemDma;
	BREG_Handle  hRegister;
	uint32_t   ulRegValue;
	uint32_t   ulStatus;

	/* assert para from our private code */
	BDBG_ASSERT( NULL != pvEngine);
	BDBG_ASSERT( NULL != peHwStatus );
	BDBG_ASSERT( NULL != pulCurrDesc );
	BDBG_ASSERT( NULL != pulScratchReg );

	pMemDma = (BDMA_P_Mem_Context *)(pvEngine);
	hRegister = BDMA_P_GetRegisterHandle(pMemDma->hDma);

	/* read CUR_DESC after STATUS, --> less chance to get wrong CUR_DESC?  */
	ulRegValue = BREG_Read32( hRegister, BCHP_SHARF_MEM_DMA0_STATUS + pMemDma->ulRegOffset );
	ulStatus  = BCHP_GET_FIELD_DATA( ulRegValue, SHARF_MEM_DMA0_STATUS, DMA_STATUS );

	ulRegValue = BREG_Read32( hRegister, BCHP_SHARF_MEM_DMA0_CUR_DESC + pMemDma->ulRegOffset );
	*pulCurrDesc = BCHP_GET_FIELD_DATA( ulRegValue, SHARF_MEM_DMA0_CUR_DESC, ADDR );



	*pulScratchReg = BREG_Read32( hRegister, BCHP_SHARF_MEM_DMA0_SCRATCH + pMemDma->ulRegOffset );

	if ( BCHP_SHARF_MEM_DMA0_STATUS_DMA_STATUS_Idle == ulStatus )
	{
		BDBG_MSG(( "\nSharf Mem DMA engine in idle state!" ));
		*peHwStatus = BDMA_P_HwStatus_eIdle;
	}
	else if ( BCHP_SHARF_MEM_DMA0_STATUS_DMA_STATUS_Busy== ulStatus )
	{
		*peHwStatus = BDMA_P_HwStatus_eBusy;
	}
	else if ( BCHP_SHARF_MEM_DMA0_STATUS_DMA_STATUS_Sleep== ulStatus )
	{
		BDBG_MSG(( "\nSharf Mem DMA engine in Sleep state!" ));
		*peHwStatus = BDMA_P_HwStatus_eSleep;
	}
	else
	{
		BDBG_ERR(( "Mem DMA engine in Reservered state!!!" ));
		*peHwStatus = BDMA_P_HwStatus_eUnknown;
		BDBG_ASSERT(0);
	}
}

/*--------------------------------------------------------------------------
 * Callback from base-module queue to append two Trans' descriptors
 */
void BDMA_P_Sharf_AppendTranDesc_isr(
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
	
#if BDMA_P_SUPPORT_MEM_DMA_AUTO_APPEND
	*(pulLastDesc+2) |=
		BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD2, AUTO_APPEND, 1);
#else
	*(pulLastDesc+2) &= ~(
		BCHP_MASK(SHARF_MEM_DMA_DESC_WORD2, LAST));
#endif
	ulWord3 = *(pulLastDesc+3) & ~(BCHP_MASK(SHARF_MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR));
	*(pulLastDesc+3) = ulWord3 |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR,
			ulNewDescPhysAddr >> BCHP_SHARF_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT );
}

/*--------------------------------------------------------------------------
 * Callback from base-module queue to send one Tran to HW.
 *
 * Note: It assume block info is already set for all blocks in this Tran
 */
void BDMA_P_Sharf_SendToHw_isr(
	BDMA_P_QueueHandle       hQueue,
	BDMA_P_HwStatus          eHwStatus,
	bool                     bAppendInHw )
{
	BDMA_P_Mem_Context *pMemDma;
	BDMA_P_QueueContext * pQueue;
	BDMA_P_QueueEntry * p1stTran;
	BDMA_P_QueueEntry * pLastTran;
	BREG_Handle  hRegister;
	uint32_t  ulNewDescPhysAddr;
	uint32_t  *pulNewDesc;
	uint32_t  ulDescWord;
	uint32_t  *pulLastDesc;
	uint32_t  ulScratchReg;
	volatile uint32_t  ulCheck;

	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );

	pMemDma = (BDMA_P_Mem_Context *)(pQueue->hEngine);
	hRegister = BDMA_P_GetRegisterHandle(pMemDma->hDma);

	p1stTran = pQueue->pHeadTranInQ;
	if (NULL == p1stTran)
	{
		BDBG_ASSERT( p1stTran );
		return;
	}
	ulNewDescPhysAddr = pQueue->pHeadTranInQ->ulDescPhysAddr;
	pulNewDesc        = pQueue->pHeadTranInQ->pulDescAddr;

	/* write scratch reg so that other BDMA instance could check our status */
	ulScratchReg = ((pQueue->bSgOpenInQ)?
					(pQueue->ul1stDescPhysAddr | BDMA_P_WATER_MARK_SG_OPEN) :
					(pQueue->ul1stDescPhysAddr | BDMA_P_WATER_MARK_SG_CLOSE));
					

	BREG_Write32( hRegister, BCHP_SHARF_MEM_DMA0_SCRATCH + pMemDma->ulRegOffset,
				  ulScratchReg );

	/* send Trans queued in SW into HW */
	pLastTran = pQueue->pTailTranInHw;
	if (bAppendInHw && (NULL != pLastTran))
	{
		pulLastDesc = (pLastTran + (pLastTran->ulNumActBlocks - 1))->pulDescAddr;

		ulDescWord = *(pulLastDesc+3) & ~(BCHP_MASK(SHARF_MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR));
		*(pulLastDesc+3) = ulDescWord |
			BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR,
				ulNewDescPhysAddr >> BCHP_SHARF_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT );

#if BDMA_P_SUPPORT_MEM_DMA_AUTO_APPEND
		/* auto-append to last Tran in hw, which might and might not finished
		 * OK to mix scatter-gather with no-scatter-gather linked-lists  */
		*(pulLastDesc+2) |=
			BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD2, AUTO_APPEND, 1);
#else
		/* sw dynamically link to last Tran in hw, which might and might not finished,
		 * must not mix scatter-gather with no-scatter-gather linked-lists  */
		*(pulLastDesc+2) &= ~(BCHP_MASK(SHARF_MEM_DMA_DESC_WORD2, LAST));
#endif

		/* read back to force that mem writing completes before reg writing */
		ulCheck = *(pulLastDesc+2) | *(pulLastDesc+3);

#if (1 == BDMA_P_SHOW_DYN_LINK_FAIL)
		BDBG_ERR(("append to 0x%08lx by new tran 0x%08lx",
				  (pLastTran + (pLastTran->ulNumActBlocks - 1))->ulDescPhysAddr,
				  ulNewDescPhysAddr));
#endif

		/* in case HW already in sleep mode */
		BREG_Write32( hRegister, BCHP_SHARF_MEM_DMA0_WAKE_CTRL + pMemDma->ulRegOffset,
			BCHP_FIELD_DATA(SHARF_MEM_DMA0_WAKE_CTRL, WAKE_MODE, BDMA_P_SHARF_WAKE_FROM_LAST_DESC) |
			BCHP_FIELD_DATA(SHARF_MEM_DMA0_WAKE_CTRL, WAKE,      BDMA_P_SHARF_WAKE_UP) );
	}
	else
	{
#if (1 == BDMA_P_SHOW_DYN_LINK_FAIL)
		BDBG_ERR(("start new tran 0x%08lx", ulNewDescPhysAddr));
#endif
		/* must start from FIRST_DESC */
		BREG_Write32( hRegister, BCHP_SHARF_MEM_DMA0_FIRST_DESC + pMemDma->ulRegOffset,
			BCHP_FIELD_DATA(SHARF_MEM_DMA0_FIRST_DESC, ADDR, ulNewDescPhysAddr) );

		if ( BDMA_P_HwStatus_eIdle != eHwStatus )
		{
			/* mem dma started, but last Tran might be destroyed */
			BREG_Write32( hRegister, BCHP_SHARF_MEM_DMA0_WAKE_CTRL + pMemDma->ulRegOffset,
				BCHP_FIELD_DATA(SHARF_MEM_DMA0_WAKE_CTRL, WAKE_MODE, BDMA_P_SHARF_WAKE_FROM_1ST_DESC) |
				BCHP_FIELD_DATA(SHARF_MEM_DMA0_WAKE_CTRL, WAKE,      BDMA_P_SHARF_WAKE_UP) );
		}
		else
		{
			/* mem dma has never started */
			BREG_Write32( hRegister, BCHP_SHARF_MEM_DMA0_CTRL + pMemDma->ulRegOffset,
				BCHP_FIELD_DATA(SHARF_MEM_DMA0_CTRL, RUN, BDMA_P_SHARF_RUN_START) );
		}
	}
}

/*--------------------------------------------------------------------------
 * Callback from base-module queue to check NEXT_DESC_ADDR and print err msg
 * dma engine reg read error happens
 */
void BDMA_P_Sharf_CheckNextDesc_isr(
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
		if (NULL != pNextTran)
			ulNextOffset = ((iLoopCntr < (int) (pCurrTran->ulNumActBlocks - 1))?
							(pBlock + 1)->ulDescPhysAddr : pNextTran->ulDescPhysAddr);
		else
			ulNextOffset = ((iLoopCntr < (int) (pCurrTran->ulNumActBlocks - 1))?
							(pBlock + 1)->ulDescPhysAddr : 0);
		pulDesc = pBlock->pulDescAddr;
		ulSeeNextOffset = (BCHP_GET_FIELD_DATA(*(pulDesc+3), SHARF_MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR) <<
						   BCHP_SHARF_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT);

		if (ulNextOffset != ulSeeNextOffset)
		{
			BDBG_ERR(("Bad next_desc: desc 0x%08lx, next_desc 0x%08lx (should be 0x%08lx)",
					  pBlock->ulDescPhysAddr, ulSeeNextOffset, ulNextOffset));
			BDBG_ERR(("tran_desc 0x%08lx, NumActBlk %d, Blck %d",
					  pCurrTran->ulDescPhysAddr, pCurrTran->ulNumActBlocks, iLoopCntr));

			/*BDBG_ASSERT(ulNextOffset != ulSeeNextOffset);*/
		}

#if (1 == BDMA_P_SHOW_DYN_LINK_FAIL)
		else if (iLoopCntr == (int) (pCurrTran->ulNumActBlocks - 1))
		{
			/* XXX extra print make system busy, and shows HW reg reading problem */
			BDBG_ERR(("good next_desc: desc 0x%08lx (blck %d), next_desc 0x%08lx",
					  pBlock->ulDescPhysAddr, iLoopCntr, ulSeeNextOffset));
		}
#endif

#if (1 == BDMA_P_SHOW_DESC)
		do {
			uint32_t *pulDesc;

			pulDesc = pBlock->pulDescAddr;
			if (iLoopCntr == 0)
				BDBG_ERR(("   first desc 0x%08lx: 0x%08lx 0x%08lx 0x%08lx 0x%08lx 0x%08lx",
					pBlock->ulDescPhysAddr, *pulDesc, *(pulDesc+1), *(pulDesc+2), *(pulDesc+3), *(pulDesc+4)));
			else if (iLoopCntr == (int) (pCurrTran->ulNumActBlocks - 1))
				BDBG_ERR(("   last  desc 0x%08lx: 0x%08lx 0x%08lx 0x%08lx 0x%08lx 0x%08lx",
					pBlock->ulDescPhysAddr, *pulDesc, *(pulDesc+1), *(pulDesc+2), *(pulDesc+3), *(pulDesc+4)));
			else
				BDBG_ERR(("         desc 0x%08lx: 0x%08lx 0x%08lx 0x%08lx 0x%08lx 0x%08lx",
					pBlock->ulDescPhysAddr, *pulDesc, *(pulDesc+1), *(pulDesc+2), *(pulDesc+3), *(pulDesc+4)));
		} while (0);
#endif

		pBlock++;
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
void BDMA_P_Sharf_DoneCallBack_isr(
	void *                pParm1,  /* pMemDma */
	int                   parm2 )  /* not used */
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;

	BSTD_UNUSED(parm2);
	BDMA_P_SHARF_GET_CONTEXT(pParm1, pMemDma);
	if ( NULL == pMemDma )
	{
		eResult = BERR_TRACE( BERR_INVALID_PARAMETER );
		BDBG_ERR(( "Invaid pParm1 to BDMA_P_Mem_DoneCallBack_isr!" ));
		goto BDMA_P_Done_Sharf_DoneCallBack_isr;
	}

	/* read status from HW and update records */
	BDMA_P_Queue_CheckHwAndSendNewReq_isr( pMemDma->hQueue );

  BDMA_P_Done_Sharf_DoneCallBack_isr:
	return;
}

#define BDMA_P_1           (1)
#define BDMA_P_0           (0)
#define BDMA_P_SHARF_UseBspKey_SHIFT    0
#define BDMA_P_SHARF_Cmp8LstByts_SHIFT  1
#define BDMA_P_SHARF_UseBspKey_MASK     (1<<BDMA_P_SHARF_UseBspKey_SHIFT)
#define BDMA_P_SHARF_Cmp8LstByts_MASK   (1<<BDMA_P_SHARF_Cmp8LstByts_SHIFT)

/***************************************************************************
 * Note: we must ensure that BDMA_SharfMode_ePassThrough and
 * BDMA_CryptMode_eNoCrypt are all 0.
 */
BERR_Code BDMA_P_Mem_Sharf_Tran_SetCrypto_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_SharfMode           eSharfMode,
	bool                     bSgEnable,
	bool                     bUseBspKey,
	bool                     bCmp8LstByts)
{
	BDMA_P_QueueEntry  *pTran;
	BERR_Code  eResult = BERR_SUCCESS;
	uint32_t  ulUseBspKey, ulCmp8LstByts;

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( (NULL == pTran) ||
		 (eSharfMode >= BDMA_SharfMode_eInvalid) )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_SetCrypt;
	}

	ulUseBspKey   = (bUseBspKey)?   1 : 0;
	ulCmp8LstByts = (bCmp8LstByts)? 1 : 0;

	pTran->bSgEnable = bSgEnable;
	pTran->eCryptMode = (BDMA_CryptMode) eSharfMode;
	pTran->ulKeySlot = ((ulUseBspKey   << BDMA_P_SHARF_UseBspKey_SHIFT) |
						(ulCmp8LstByts << BDMA_P_SHARF_Cmp8LstByts_SHIFT));

  BDMA_P_Done_Mem_SetCrypt:
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * To be called to set the block info for a Tran block. Input block info are
 * validated against the current engine state.
 */
BERR_Code BDMA_P_Mem_Sharf_Tran_SetDmaBlockInfo_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	BDMA_Context             eContext,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_SharfMode  eSharfMode;
	BDMA_P_QueueEntry  *pTran;
	BDMA_P_QueueEntry  *pBlock;
	uint32_t  *pulDesc;
	uint32_t  ulSgStartEnd;
	uint32_t  ulContext, ulSgEnable, ulCmp8LstByts, ulUseBspKey, ulMode;
	uint32_t  ulSwapMode, ulReadEndian, ulNextDescAddr;
	BDMA_P_Mem_Context *  pMemDma;

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	pBlock = pTran + ulBlockId;
	pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;

	/* store the potential setting by BDMA_P_Mem_Sharf_Tran_SetSgStartEnd_isr */
	ulSgStartEnd = *(pulDesc+4) & (
		BCHP_MASK( SHARF_MEM_DMA_DESC_WORD4, SG_SCRAM_START ) |
		BCHP_MASK( SHARF_MEM_DMA_DESC_WORD4, SG_SCRAM_END )   |
		BCHP_MASK( SHARF_MEM_DMA_DESC_WORD4, DIGEST_PRESENT ) |
		BCHP_MASK( SHARF_MEM_DMA_DESC_WORD4, KEY_PRESENT ) );

	/* set the generic stuff first */

	/* set desc word 0 */
	*(pulDesc  ) = 
		BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD0, READ_ADDR, ulSrcBusAddr);

	/* set desc word 1 */
	*(pulDesc+1) = 
		BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD1, WRITE_ADDR, ulDstBusAddr);

	/* set desc word 2 */
	*(pulDesc+2) = 
		BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD2, TRANSFER_SIZE, ulBlockSize);
		
	/* set desc word 3 */
	eResult = BDMA_P_Mem_GetMemDmaPtrFromTran_isr( pTran, &pMemDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Sharf_Tran_SetDmaBlockInfo );
	
	ulNextDescAddr = (pBlock + 1)->ulDescPhysAddr >>
		BCHP_SHARF_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;
	ulReadEndian = (BDMA_Endian_eLittle == pMemDma->eReadEndian)?
		BCHP_SHARF_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_LITTLE_ENDIAN:
		BCHP_SHARF_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_BIG_ENDIAN;
	ulSwapMode =
		( (BDMA_SwapMode_eNoSwap == pMemDma->eSwapMode)?
		  (BCHP_SHARF_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_WORD_ALIGNED):
		  ((BDMA_SwapMode_e0123To2301==pMemDma->eSwapMode)?
		   (BCHP_SHARF_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_HALF_WORD_ALIGNED):
		   (BCHP_SHARF_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_BYTE_ALIGNED)) );
	*(pulDesc+3) =
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR,          ulNextDescAddr )      |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD3, READ_ENDIAN_MODE,        ulReadEndian )        |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD3, WRITE_ENDIAN_XLATE_MODE, ulSwapMode );
	
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Sharf_Tran_SetDmaBlockInfo );

	/* set the sharf specific stuff
	 */

	eSharfMode = (BDMA_SharfMode) pTran->eCryptMode;
	BDBG_ASSERT( BDMA_Context_eInvalid > eContext );
	BDBG_ASSERT( (BDMA_SharfMode_ePassThrough == eSharfMode) ||
				 ((BDMA_SharfMode_eSha1 == eSharfMode) &&
				  (BDMA_Context_eAesOrCmac != eContext))             ||
				 (((BDMA_SharfMode_eAes128CbcDecrypt == eSharfMode) ||
				   (BDMA_SharfMode_eAes128CbcEncrypt == eSharfMode) ||
				   (BDMA_SharfMode_eAes128EcbDecrypt == eSharfMode) ||
				   (BDMA_SharfMode_eAes128EcbEncrypt == eSharfMode) ||
				   (BDMA_SharfMode_eCmac == eSharfMode)) &&
				  (BDMA_Context_eAesOrCmac == eContext)) );

	/* set desc word 4 */
	ulContext = (BDMA_Context_eAesOrCmac != eContext)? (uint32_t) eContext : 0;
	ulSgEnable = (pTran->bSgEnable)? BDMA_P_1: BDMA_P_0;
	ulCmp8LstByts = (pTran->ulKeySlot & BDMA_P_SHARF_Cmp8LstByts_MASK)? BDMA_P_1: BDMA_P_0;
	ulUseBspKey = (pTran->ulKeySlot & BDMA_P_SHARF_UseBspKey_MASK)? BDMA_P_1: BDMA_P_0;
	ulMode = (uint32_t) (eSharfMode);
	*(pulDesc+4) = ulSgStartEnd |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD4, CONTEXT_NUM,             ulContext )     |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD4, SECURE_FAIL_INTR_ENABLE, BDMA_P_1 )      |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD4, FAIL_INTR_ENABLE,        BDMA_P_1 )      |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD4, SG_ENABLE,               ulSgEnable )    |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD4, CMP_8_LSBYTES,           ulCmp8LstByts ) |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD4, USE_BSP_KEY,             ulUseBspKey )   |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD4, MODE_SEL,	            ulMode );

  BDMA_P_Done_Mem_Sharf_Tran_SetDmaBlockInfo:
	return BERR_TRACE( eResult );
}


/* these must be the same as defined in bdma_mem_priv.c */
#define BDMA_P_SG_START_SHIFT   0
#define BDMA_P_SG_END_SHIFT     1
#define BDMA_P_SG_START_MASK    (1<<BDMA_P_SG_START_SHIFT)
#define BDMA_P_SG_END_MASK      (1<<BDMA_P_SG_END_SHIFT)

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Sharf_Tran_SetSgStartEnd_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	bool                     bStartSgScram,
	bool                     bEndSgScram,
	bool                     bKeyPresent,
	bool                     bDigestPresent )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_QueueEntry * pTran;
	BDMA_P_QueueEntry * pBlock;
	uint32_t  *pulDesc;
	uint32_t  ulWord4;
	uint32_t  ulSgStart, ulSgEnd, ulKeyPresent, ulDigestPresent;

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		BDBG_ERR(( "Bad hTran 0x%x", (uint32_t) hTran ));
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Tran_SetSgStartEnd;
	}
	if ( pTran->ulNumTranBlocks <= ulBlockId )
	{
		BDBG_ERR(( "Bad ulBlockId %d", ulBlockId ));
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Tran_SetSgStartEnd;
	}
	if ((!bStartSgScram) && (bDigestPresent || bKeyPresent))
	{
		BDBG_ERR(( "Key/Digest present at non-SgStart block Id %d", ulBlockId ));
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Tran_SetSgStartEnd;
	}

	pBlock = pTran + ulBlockId;
	pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;
	ulWord4 = *(pulDesc+4) & ~(
		BCHP_MASK( SHARF_MEM_DMA_DESC_WORD4, SG_SCRAM_START ) |
		BCHP_MASK( SHARF_MEM_DMA_DESC_WORD4, SG_SCRAM_END )   |
		BCHP_MASK( SHARF_MEM_DMA_DESC_WORD4, DIGEST_PRESENT ) |
		BCHP_MASK( SHARF_MEM_DMA_DESC_WORD4, KEY_PRESENT ) );

	ulSgStart = (bStartSgScram) ? BDMA_P_1 : BDMA_P_0;
	ulSgEnd = (bEndSgScram) ? BDMA_P_1 : BDMA_P_0;
	ulKeyPresent = (bKeyPresent) ? BDMA_P_1 : BDMA_P_0;
	ulDigestPresent = (bDigestPresent) ? BDMA_P_1 : BDMA_P_0;

	*(pulDesc+4) = ulWord4 |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD4, SG_SCRAM_START, ulSgStart ) |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD4, SG_SCRAM_END,   ulSgEnd )   |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD4, DIGEST_PRESENT, ulDigestPresent ) |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD4, KEY_PRESENT,    ulKeyPresent );

	/* for optimization in BDMA_P_Mem_Tran_PrepareStart_isr */
	pTran->bSgStartStopSet = true;
	pBlock->ulSgStartStop =
		(ulSgStart<<BDMA_P_SG_START_SHIFT) | (ulSgEnd << BDMA_P_SG_END_SHIFT);

  BDMA_P_Done_Mem_Tran_SetSgStartEnd:
	return BERR_TRACE(eResult);
}

/*--------------------------------------------------------------------------
 * To be called to modify the last act block's desc to work as last block,
 * and to flush cached descriptors
 */
static BERR_Code BDMA_P_Mem_Sharf_Tran_PrepareStart_isr(
	BDMA_P_QueueEntry *      pTran,
	BDMA_P_QueueHandle       hQueue,
	BMEM_Handle              hMemory,
	uint32_t                 ulNumActBlocks,  /* new NumActBlocks */
	bool  *                  pbSgOpen )
{
	BDMA_P_QueueContext * pQueue;
	BDMA_P_QueueEntry *pBlock;
	uint32_t *pulDesc;
	uint32_t  ulNextOffset;
	uint32_t  ulWord;
	volatile uint32_t  ulCheck;
#if ((BDMA_P_CHECK_NEXT_DESC) && (BDBG_DEBUG_BUILD)) || \
	((BDMA_P_CHECK_SCATTER_GATHER) && (BDBG_DEBUG_BUILD))
	int iLoopCntr;
#endif

	BDBG_ENTER(BDMA_P_Mem_Sharf_Tran_PrepareStart_isr);

	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );

	BDBG_ERR(("\n NumActBlocks %d %d", ulNumActBlocks, pTran->ulNumActBlocks));
	/* in the case of re-use, the previous "last act block" might have wrong NEXT_DESC */
	if ((ulNumActBlocks != pTran->ulNumActBlocks) &&
		(0 < pTran->ulNumActBlocks))
	{
		pBlock = pTran + (pTran->ulNumActBlocks - 1);
		pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;

		*(pulDesc+2) &= ~(
#if BDMA_P_SUPPORT_MEM_DMA_AUTO_APPEND
			BCHP_MASK(SHARF_MEM_DMA_DESC_WORD2, AUTO_APPEND) |
#endif
			BCHP_MASK(SHARF_MEM_DMA_DESC_WORD2, INTR_ENABLE) |
			BCHP_MASK(SHARF_MEM_DMA_DESC_WORD2, LAST));

		ulNextOffset =
			(pBlock + 1)->ulDescPhysAddr >> BCHP_SHARF_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;
		ulWord = *(pulDesc+3) & ~(BCHP_MASK(SHARF_MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR));
		*(pulDesc+3) = ulWord |
			BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR, ulNextOffset );
	}

	/* modify new "last act block" desc */
	pBlock = pTran + (ulNumActBlocks - 1);
	pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;
#if BDMA_P_SUPPORT_MEM_DMA_AUTO_APPEND
	ulWord = *(pulDesc+2) & ~(
		BCHP_MASK(SHARF_MEM_DMA_DESC_WORD2, AUTO_APPEND));
	*(pulDesc+2) = ulWord | (
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD2, INTR_ENABLE,   BDMA_P_SHARF_MEM_FIRE_INTR ) |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD2, LAST,          BDMA_P_SHARF_MEM_LAST_BLK ));
#else
	*(pulDesc+2) |= (
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD2, INTR_ENABLE,   BDMA_P_SHARF_MEM_FIRE_INTR ) |
		BCHP_FIELD_DATA( SHARF_MEM_DMA_DESC_WORD2, LAST,          BDMA_P_SHARF_MEM_LAST_BLK ));
#endif
	*(pulDesc+3) &= ~(
		BCHP_MASK(SHARF_MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR));

	*pbSgOpen = ((pTran->bSgEnable) &&
				 (0 == (BDMA_P_SG_END_MASK & pBlock->ulSgStartStop)));

#if (BDMA_P_CHECK_NEXT_DESC) && (BDBG_DEBUG_BUILD)
	pBlock = pTran;
	for ( iLoopCntr = 0; iLoopCntr < (int) ulNumActBlocks; iLoopCntr++ )
	{
		uint32_t ulSeeNextOffset;

		ulNextOffset =
			(pBlock + 1)->ulDescPhysAddr >> BCHP_SHARF_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;
		pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;
		ulSeeNextOffset = BCHP_GET_FIELD_DATA(*(pulDesc+3), SHARF_MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR);

		if ((ulNextOffset != ulSeeNextOffset) ||
			(false == pBlock->bBlockInfoSet))
		{
			BDBG_ASSERT((ulNextOffset == ulSeeNextOffset) ||
						(false == pBlock->bBlockInfoSet));
			return BERR_TRACE(BDMA_ERR_BLOCK_INFO_UNSET);
		}
	}
#endif

#if (BDMA_P_CHECK_SCATTER_GATHER) && (BDBG_DEBUG_BUILD)
	/* check scatter-gather setting */
	if (pQueue->bSgOpenInQ || pTran->bSgEnable)
	{
		bool  bSgOpen;

		/* we must be doing scatter-gather if we get in here */
		bSgOpen = pQueue->bSgOpenInQ;
		if ((false == pTran->bSgEnable) ||
			((false == bSgOpen) && (false == pTran->bSgStartStopSet)))
		{
			BDBG_ASSERT((false == pTran->bSgEnable) ||
						((false == bSgOpen) && (false == pTran->bSgStartStopSet)));
			return BERR_TRACE(BDMA_ERR_SG_MISMATCH);
		}

		pBlock = pTran;
		for ( iLoopCntr = 0; iLoopCntr < (int) ulNumActBlocks; iLoopCntr++ )
		{
			if (((false == bSgOpen) && (0 == (BDMA_P_SG_START_MASK & pBlock->ulSgStartStop))) ||
				((true  == bSgOpen) && (0 != (BDMA_P_SG_START_MASK & pBlock->ulSgStartStop))))
			{
				BDBG_ASSERT(((false == bSgOpen) && (0 == (BDMA_P_SG_START_MASK & pBlock->ulSgStartStop))) ||
							((true  == bSgOpen) && (0 != (BDMA_P_SG_START_MASK & pBlock->ulSgStartStop))));
				return BERR_TRACE(BDMA_ERR_SG_MISMATCH);
			}
			bSgOpen = (0 == (BDMA_P_SG_END_MASK & pBlock->ulSgStartStop));
			pBlock += 1;
		}
	}
#endif

	/* flush cached desc mem */
	if (pTran->bCachedDscrp)
		BMEM_FlushCache_isr(hMemory, pTran->pulCachedDescAddr,
							ulNumActBlocks * BDMA_P_SHARF_NUM_WORDS_PER_DESC * 4);

	/* read back with un-cached addr to ensure previous writing completes */
	ulCheck = *((pTran + (ulNumActBlocks - 1))->pulDescAddr + 3);
	if ((ulCheck & BCHP_MASK(SHARF_MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR)) != 0)
	{
		BDBG_ERR(("desc writing error with preparing start"));
	}

	BDBG_LEAVE(BDMA_P_Mem_Sharf_Tran_PrepareStart_isr);
	return BERR_SUCCESS;
}

/***************************************************************************
 * To be called by both BDMA_P_Mem_Sharf_Tran_Start_isrAndCallBack and
 * BDMA_P_Mem_Sharf_Tran_Start_isr. BDMA_P_Mem_Tran_Start_isr should pass NULL for
 * pCallBackFunc_isr and pUserCallBackParam
 *
 * Note: when a Tran is sent to HW, interrupt is alwayas enabled. If the
 * Tran is async, the module's interrupt handler BDMA_P_Mem_DoneCallBack_isr
 * will call user's callback function. In the sync case, there is no user's
 * call back function to call, BDMA_P_Mem_DoneCallBack_isr only sets the
 * internal record of the status, the ideal is that the user will check the
 * Tran's status in a waiting loop and process the Tran result synchronously.
 */
BERR_Code BDMA_P_Mem_Sharf_Tran_Start_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,
	BDMA_Mem_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;
	BDMA_P_QueueEntry * pTran;
	bool  bSgOpen;

	

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Sharf_Tran_Start;
	}

	eResult = BDMA_P_Mem_GetMemDmaPtrFromTran_isr( pTran, &pMemDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Sharf_Tran_Start );

	eResult = BDMA_P_Mem_Sharf_Tran_PrepareStart_isr( pTran, pMemDma->hQueue,
		BDMA_P_GetMemoryHandle(pMemDma->hDma), ulNumActBlocks, &bSgOpen );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Sharf_Tran_Start );

	eResult = BDMA_P_Queue_StartTran_isr (
		pMemDma->hQueue, pTran, ulNumActBlocks, bSgOpen,
		(BDMA_P_CallbackFunc) pCallBackFunc_isr, pUserCallBackParam );

  BDMA_P_Done_Mem_Sharf_Tran_Start:
  	
	return BERR_TRACE(eResult);
}
/* End of File */
