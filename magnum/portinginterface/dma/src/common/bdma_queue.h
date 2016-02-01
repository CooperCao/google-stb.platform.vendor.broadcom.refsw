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

#ifndef BDMA_QUEUE_H__
#define BDMA_QUEUE_H__

#include "bdma_priv.h"        /* for enum def and utility MACRO only */

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * {private}
 *
 * queue (private) module overview:
 *
 * This file implements the queue sub-module. It is used by both memory dma,
 * pci dma, and io dma sub-module. Queue sub-module is not a sub-module in
 * the sense of Magnum architecture. It is only a module in the sense of
 * generic programming.
 *
 * It is intended to be used privately, to run fast, and therefore it does
 * not perform parameter validation
 *
 * Refer to BDMA_P_Queue_Create, the queue manages a queue-entry array and
 * a decriptor array. Each queue-entry points to the descriptor of the same
 * index. The descriptor buffer is allocated from BMEM and is contiguous.
 * 
 * When a new transfer is required, BDMA_P_Queue_CreateTran is called 
 * to allocate the next un-used contiguous chunck of queue-entries / 
 * descriptors and initialize the first entry of the chunck, that stores 
 * the state (such as Tran status, NumBlocks of the transfer). The queue is
 * used in a round-robin and wrap-around manner.
 *
 * BDMA_P_Queue_StartTran is called when "start" or "start and call back" is
 * called by high level software. It decides whether the user ordered Tran
 * should be sent to HW at the moment, put as queued, or return error of
 * "engine busy". 
 *
 * BDMA_P_Queue_DestroyTran is called when "destroy" is called by high level
 * software. It free the Tran's entries for future Tran creating request.
 *
 * BDMA_P_Queue_ProcEngineIdle is called to process the engine idle info. 
 * It is ideally for DMA engine/channel's "done" interrupt handler.
 * It ensures HW just finished a transfer and decides which one is just
 * finished. It also schedules a new (async) transfer to be sent to HW if
 * there exists one.
 *
 * Implementaion notes:
 *   1. A Tran occupies a contiguous chunk of entries. Each entry represents
 * a transfer block. The first entry represent the Tran and stores the Tran's
 * status, number of blocks, user call back func, user call back para, ..., 
 * and only fields bBlockInfoSet and pulDesc in the following entries has
 * siginificace (the otherfiels are set fot assersion only).
 *   2. Whether a Tran is active or not, is indicated by whether its status
 * is unknown. The entries of a non-active Tran are free for new Tran
 * creating.
 *   3. The queue module mantains two index: ulIndx_LastEntryIssued, and
 * ulIndx_LastEntryAssigned. If ulIndx_LastEntryIssued is in InProgress state,
 * the HW is busy, otherwise HW is idle. When a "done" interrupt happens, the
 * module tries to find the next async Tran after (wrap-aroundly) the just
 * finished one, to send to HW. And new Tran creating tries to use the next
 * free contiguoys chunk of entries after ulIndx_LastEntryAssigned (wrap-
 * aroundly also). By this way we try to maintan a fairness to all transfer
 * requests.
 *   4. It is assumed that all Tran causes a interrupt, in order to mantain
 * the channel /queue state.
 *   5. Queue module manages the state of the queue, the user of queue module
 * may only know the struct of BDMA_P_QueueEntry.
 */

#define BDMA_P_NUM_QUEUE_ENTRIES      (2<<8)
#define BDMA_P_MAX_NUM_BLOCKS	      (BDMA_P_NUM_QUEUE_ENTRIES >> 1)

#define BDMA_P_QUEUE_GET_CONTEXT(handle, context) \
	BDMA_P_GENERIC_GET_CONTEXT(handle, context, BDMA_P_QueueContext)

#define BDMA_P_QUEUE_SET_BLACK_MAGIC(handle) \
	BDMA_P_GENERIC_SET_BLACK_MAGIC(handle, BDMA_P_QueueContext)
	
#define BDMA_P_QUEUE_DESTROY_CONTEXT(struct_ptr) \
	BDMA_P_GENERIC_DESTROY_CONTEXT(struct_ptr, BDMA_P_QueueContext)

#if BDBG_DEBUG_BUILD 
#define BDMA_P_QUEUE_DESTROY_ENTRY(entry_head, num_entry) \
{ \
	BKNI_Memset((void*)entry_head, 0xA3, num_entry * sizeof(BDMA_P_QueueEntry)); \
	BKNI_Free((void*)entry_head); \
	entry_head = NULL; \
}
#else
#define BDMA_P_QUEUE_DESTROY_ENTRY(entry_head, num_entry) \
{ \
	BKNI_Free((void*)entry_head); \
}
#endif	

#if BDBG_DEBUG_BUILD 
#define BDMA_P_QUEUE_DESTROY_DESC(h_mem, desc_ptr, size) \
{ \
	BKNI_Memset((void*)desc_ptr, 0xA3, size); \
	BMEM_Free(h_mem, desc_ptr); \
}
#else
#define BDMA_P_QUEUE_DESTROY_DESC(h_mem, desc_ptr, size) \
{ \
	BMEM_Free(h_mem, desc_ptr); \
}
#endif	

#define BDMA_P_TRAN_GET_CONTEXT(handle, context) \
{ \
	if(!(handle) || \
	   (((BDMA_P_QueueEntry*)(handle))->ulBlackMagic != \
	    (sizeof(BDMA_P_QueueEntry) | 0xbac98700))) \
	{ \
		BDBG_ERR(("NULL context handle\n")); \
		(context) = NULL; \
	} \
	else \
	{ \
		(context) = (BDMA_P_QueueEntry *)(handle); \
	} \
}

#define BDMA_P_TRAN_SET_BLACK_MAGIC(handle) \
{ \
	((BDMA_P_QueueEntry*)(handle))->ulBlackMagic = sizeof(BDMA_P_QueueEntry) | 0xbac98700; \
}

/***************************************************************************
 * {private}
 * Dma queue handle
 */
typedef struct BDMA_P_QueueContext    *BDMA_P_QueueHandle;
	
/***************************************************************************	
 * {private}
 * dma queue entry info
 * Each queue entry corresponds to a dma descriptor, and therefore a tran
 * block. A transfer's state is stored in the queue entry of its first block 
 */
typedef struct BDMA_P_QueueEntry  BDMA_P_Tran;
typedef void (* BDMA_P_CallbackFunc)( \
	void *  pUserCallBackParam,\
	BDMA_P_Tran * hTran, \
	BDMA_TranStatus  eStatus );

typedef struct BDMA_P_QueueEntry
{
	uint32_t   ulBlackMagic;   /* Black magic for handle validation */

	/* created from this handle */
	BDMA_P_QueueHandle   hQueue;

	/* --------------------------------------------------
	 *
	 * context for Tran only
	 */
	struct BDMA_P_QueueEntry *pNextTran;   /* for dynamical link to next Tran */
	
	uint32_t             ulNumTranBlocks;   /* Max number of blocks in this Tran */
	uint32_t             ulNumActBlocks;    /* Number of blocks currently active */
	BDMA_P_CallbackFunc  pCallBackFunc_isr; /* call back function ptr */
	void *               pUserCallBackParam;/* user's info to call back func */
	BDMA_TranStatus      eStatus;           /* status of the transfer */
	
	/* current state of the Tran: applied to future setDmaBlockInfo */
	BDMA_CryptMode       eCryptMode;
	uint32_t             ulKeySlot;
	bool                 bSgEnable;

	bool                 bSgStartStopSet;   /* ever set in any block of this Tran */

	bool                 bCachedDscrp;      /* whether use cached addr to write descrp */

	/* --------------------------------------------------
	 *
	 * context for each block
	 */
	bool                 bBlockInfoSet;     /* whether this block's info is set */
	uint32_t             ulSgStartStop;
	uint32_t *           pulDescAddr;       /* descriptor buffer - non-cached addr */
	uint32_t *           pulCachedDescAddr; /* descriptor buffer - cached addr */
	uint32_t             ulDescPhysAddr;    /* descriptor buffer - phys addr */
	
	uint32_t             ulFirstBlock;      /* offset into contiguous descriptor list */
} BDMA_P_QueueEntry;

/***************************************************************************
 * {private}
 * HW status enum
 */
typedef enum
{
	BDMA_P_HwStatus_eIdle, 
	BDMA_P_HwStatus_eBusy,
	BDMA_P_HwStatus_eSleep,
	BDMA_P_HwStatus_eUnknown 
}
BDMA_P_HwStatus;

/***************************************************************************
 * {private}
 * Common dma engine settings
 */
typedef struct BDMA_P_CmnSettings
{
	uint32_t                           ulTotalNumBlocks;
	uint32_t                           ulMaxNumBlocksPerTran;

	/* settings for the case of multiple BDMA instances */
	BDMA_LockCallbackFunc              pTryLock_isr;
	BDMA_LockCallbackFunc              pReleaseLock_isr;
	void                             * pvParm1;
	int                                iParm2;
	bool                               bSupportCallBack;
}
BDMA_P_CmnSettings;

/***************************************************************************
 * {private}
 * Call back functions to deal with specific HW engine and descriptor formats
 */
typedef void (* BDMA_P_ReadHwRegsFunc) ( \
	void *                   pvEngine, \
	BDMA_P_HwStatus  *       peHwStatus, \
	uint32_t  *              pulCurrDesc, \
	uint32_t  *              pulScratchReg );

typedef void (* BDMA_P_AppendTranDescFunc)(	\
	BDMA_P_QueueEntry *      pCurrTran,  \
	BDMA_P_QueueEntry *      pNextTran );

typedef void (* BDMA_P_SendToHwFunc)( \
	BDMA_P_QueueHandle       hQueue, \
	BDMA_P_HwStatus          eHwStatus, \
  	bool                     bAppendInHw );

typedef void (* BDMA_P_CheckNextDescFunc)( \
	BDMA_P_QueueEntry *      pCurrTran,  \
	BDMA_P_QueueEntry *      pNextTran );

typedef void (* BDMA_P_SafeToSendHwFunc)( \
	BDMA_P_QueueHandle       hQueue,      \
	BDMA_P_HwStatus*         peHwStatus);
/***************************************************************************
 * {private}
 * Dma queue Context
 */
typedef struct BDMA_P_QueueContext
{
	uint32_t   ulBlackMagic;   /* Black magic for handle validation */

	/* created from this dma engine */
	void *              hEngine;
	
	uint32_t            ulIndx_LastEntryAssigned;

	bool                bDmaHwShared;
	bool                bHwStarted;
	bool                bSgOpenInHw; /* SgOpen: scatter-gather started, not stopped */
	bool                bPreLinkSgOpenInHw; /* in case dyn link fails */
	bool                bSgOpenInQ;
	BDMA_P_QueueEntry * pHeadTranInHw;
	BDMA_P_QueueEntry * pTailTranInHw;
	BDMA_P_QueueEntry * pHeadTranInQ;
	BDMA_P_QueueEntry * pTailTranInQ;
	BDMA_P_QueueEntry * pPreLinkTailTranInHw; /* in case dyn link fails */
	
	uint32_t            ulNumWordsPerDesc;
	BDMA_P_CmnSettings  CmnSettings;
	
	uint32_t            ulLockMark;
	uint32_t            ul1stDescPhysAddr;
	uint32_t		    ulLastDescPhysAddr;
	BDMA_P_QueueEntry * pHeadEntry;         /* pointer to the entry array */
	
	/* call back funcs from engine to deal with specific HW */
	BDMA_P_ReadHwRegsFunc		pReadHwRegs_isr;
	BDMA_P_AppendTranDescFunc	pAppendTranDesc_isr;
	BDMA_P_SendToHwFunc         pSendToHw_isr;
	BDMA_P_CheckNextDescFunc    pCheckNextDesc_isr;  /* for err msg only */
	BDMA_P_SafeToSendHwFunc     pSafeToSendHw_isr;
	
} BDMA_P_QueueContext;


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
	  BDMA_P_SafeToSendHwFunc   pSafeToSendHw_isr);

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
	  BMEM_Handle               hMemory );

/***************************************************************************
 * {private}
 * BDMA_P_Queue_GetEngine_isr
 *
 * To be called to get the dma engine
 *
 * Note: assume parameters are valid
 */
void * BDMA_P_Queue_GetEngine_isr
	( BDMA_P_QueueHandle        hQueue );

/*--------------------------------------------------------------------------
 * {private}
 * BDMA_P_Queue_CheckHwAndSendNewReq_isr
 *
 * Called to check HW status, process it, and try to send the Trans queued
 * in SW into HW
 */
void BDMA_P_Queue_CheckHwAndSendNewReq_isr
	( BDMA_P_QueueHandle       hQueue );

/***************************************************************************
 * {private}
 * BDMA_P_Queue_CreateTran
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
	  BDMA_P_QueueEntry **     ppNewTran );

/***************************************************************************
 * {private}
 * BDMA_P_Queue_StartTran_isr
 *
 * To be called to process the user calling of Start or StartAndCallBack.
 *
 * pCallBackFunc_isr is assumed to indicate whether the Tran is sync or async.
 *
 * Note: assume parameters are valid
 */
BERR_Code BDMA_P_Queue_StartTran_isr
	( BDMA_P_QueueHandle       hQueue,
	  BDMA_P_QueueEntry *      pTran,
	  uint32_t                 ulNumActBlocks,
	  bool                     bSgOpen,
	  BDMA_P_CallbackFunc      pCallBackFunc_isr,
	  void *                   pUserCallBackParam );

/***************************************************************************
 * {private}
 * BDMA_P_Queue_DestroyTran
 *
 * To be called to destroy a Tran. It reset the Tran's entries to make the
 * chunk of blocks as a destroyed Tran
 *
 * Note: assume parameters are valid
 */
BERR_Code BDMA_P_Queue_DestroyTran_isr
	( BDMA_P_QueueHandle       hQueue,
	  BDMA_P_QueueEntry *      pTran );

/***************************************************************************
 * {private}
 * BDMA_P_Queue_ResetTran
 *
 * To be called to reset a active Tran into intial eCreated state. Without
 * resetting, the Tran could also be used to SetBlockInfo and Start again,
 * but the module will not be able to check whether each block info is set.
 *
 * Note: assume parameters are valid,
 */
BERR_Code BDMA_P_Queue_ResetTran_isr
	( BDMA_P_QueueHandle       hQueue,
	  BDMA_P_QueueEntry *      pTran );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BDMA_QUEUE_H__ */

/* End of File */
