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
#ifndef BDMA_MEM_PRIV_H__
#define BDMA_MEM_PRIV_H__

#include "bdma_priv.h"
#include "bdma_queue.h"
#ifdef BDMA_P_SUPPORT_SHARF_DMA_ENGINE
#include "../sharf/bdma_mem_sharf_priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
 * {private}

 * Implementation Overview:
 *
 * The implementation uses a private queue module, to manage the active
 * transfers. The queue module is shared by both memory DMA and PCI DMA.
 * It manages the queue entry assigning, freeing, and the transfer status.
 * Refer to queue module overview for more info.
 * 
 * Memory DMA sub-module code should never refer to the member of queue
 * module directly, however it needs to know the def of struct
 * BDMA_P_QueueEntry.
 *
 * By abstracting out the queue module, the readabilty, maintainabilty, and
 * extendabilty (such as to IO DMA) are much improved. Code size is also
 * improved.
 ****************************************************************************/

#define BDMA_P_MEM_GET_CONTEXT(handle, context) \
	BDMA_P_GENERIC_GET_CONTEXT((handle), (context), BDMA_P_Mem_Context)

#define BDMA_P_MEM_SET_BLACK_MAGIC(handle) \
	BDMA_P_GENERIC_SET_BLACK_MAGIC((handle), BDMA_P_Mem_Context)

#define BDMA_P_MEM_DESTROY_CONTEXT(struct_ptr) \
	BDMA_P_GENERIC_DESTROY_CONTEXT(struct_ptr, BDMA_P_Mem_Context)	

#if (BDMA_P_SUPPORT_MEM_DMA_41_KEY_SLOTS)
#define BDMA_P_MEM_KEY_SLOT_MAX               41
#else
#define BDMA_P_MEM_KEY_SLOT_MAX               15
#endif	

#if ((BCHP_CHIP == 35230) || (BCHP_CHIP == 35125) || (BCHP_CHIP == 35233))
#define BDMA_P_MEM_KEY_SIZE                   8
#else
#define BDMA_P_MEM_KEY_SIZE                   6
#endif

#define BDMA_P_MEM_MAX_BLOCK_SIZE             (0x1000000 -1)  /* 16 MBytes */	
	
/***************************************************************************
 * Memory Tran Handle
 */
typedef struct BDMA_P_Mem_TranContext
{
	BDMA_P_QueueEntry  QueueEntry;
	
} BDMA_P_Mem_TranContext;


/***************************************************************************
 * Memory Dma Context
 */
typedef struct BDMA_P_Mem_Context
{
	uint32_t   ulBlackMagic;   /* Black magic for handle validation */

	/* created from this handle */
	BDMA_Handle    hDma;

	/* engine id */
	BDMA_MemDmaEngine eEngine;

	/* register offset */
	uint32_t       ulRegOffset;

	/* configures */
	BDMA_Endian     eReadEndian;/* endian read mode */
	BDMA_SwapMode   eSwapMode;  /* swap mode */
	BDMA_CryptMode  eCryptMode;
	uint32_t        ulKeySlot;
	bool            bSgEnable;
	bool            bCryptoSetInEngine;
	bool            bCryptoSetInTran;
	
	/* tran entry queue */
	BDMA_P_QueueHandle  hQueue;
	
	/* interrupt call back handle */
	BINT_CallbackHandle  hCallBack;	

#ifdef BDMA_P_SUPPORT_SHARF_DMA_ENGINE
	BDMA_P_Mem_Sharf_Handle  hSharf;
#endif
	
} BDMA_P_Mem_Context;


/***************************************************************************
 *
 * Utility functions used by BDMA
 *
 ***************************************************************************/
/*--------------------------------------------------------------------------
 * To be called by BDMA API func before taking real action, to get dma mutex
 * WITH block, ideally for user mode
 */
BERR_Code BDMA_P_Mem_AcquireMutex(
	BDMA_Mem_Handle          hMemDma,
	BKNI_MutexHandle        *phMutex );


/***************************************************************************
 *
 * API support functions
 *
 ***************************************************************************/

/*--------------------------------------------------------------------------
 */
BERR_Code BDMA_P_Mem_GetMemDmaPtrFromTran_isr(
	BDMA_P_QueueEntry *    pTran,
	BDMA_P_Mem_Context **  ppMemDma );

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Init(
	BDMA_P_Mem_Context  * pMemDma,
	const BDMA_Mem_Settings *pSettings,
	uint32_t              ulL2IntrBintId );

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_UnInit(
	BDMA_P_Mem_Context  * pMemDma );

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Create(
	BDMA_Handle           hDma,
	BDMA_MemDmaEngine     eEngine,
	const BDMA_Mem_Settings *pSettings,
	BDMA_Mem_Handle *     phMemDma );

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Destroy(
	BDMA_Mem_Handle          hMemDma );

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_SetSwapMode_isr(
	BDMA_Mem_Handle          hMemDma,
	BDMA_SwapMode            eSwapMode );

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_SetByteSwapMode_isr(
	BDMA_Mem_Handle          hMemDma,
	BDMA_Endian              eReadEndian,
	BDMA_SwapMode            eSwapMode );

BERR_Code BDMA_P_Mem_Tran_SetCrypto_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_CryptMode           eCryptMode,
	uint32_t                 ulKeySlot,
	bool                     bSgEnable );

/***************************************************************************
 * obsolete
 */
BERR_Code BDMA_P_Mem_SetCrypto_isr(
	BDMA_Mem_Handle          hMemDma,
	BDMA_CryptMode           eCryptMode,	
	uint32_t                 ulKeySlot,
	bool                     bSgEnable );

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Tran_Create_isr(
	BDMA_Mem_Handle          hMemDma,
	uint32_t                 ulNumBlocks,
	bool                     bCachedDesc,
	BDMA_Mem_Tran_Handle  *  phTran );

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Tran_SetDmaBlockInfo_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize,
	bool                     bCryptInit );

/***************************************************************************
 * To be called to mark / unmark the block as scatter-gather start /end point
 * validated against the current engine state.
 */
BERR_Code BDMA_P_Mem_Tran_SetSgStartEnd_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	bool                     bSgStart,
	bool                     bSgEnd );

/***************************************************************************
 * To be called by both BDMA_P_Mem_Tran_Start and
 * BDMA_P_Mem_Tran_StartAndCallBack, BDMA_P_Mem_Tran_Start should pass NULL
 * for pCallBackFunc_isr and pUserCallBackParam
 */
BERR_Code BDMA_P_Mem_Tran_Start_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,
	BDMA_Mem_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam );

/***************************************************************************
 * To be called by both BDMA_Mem_Tran_StartDmaSubset and
 * BDMA_Mem_Tran_StartDmaSubsetAndCallBack. 
 */

BERR_Code BDMA_P_Mem_Tran_StartSubset_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulFirstBlock,
	uint32_t                 ulNumActBlocks,
	BDMA_Mem_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam );

/***************************************************************************
 */
BERR_Code BDMA_P_Mem_Transfer_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize,
	bool                     bCryptInit,
	BDMA_Mem_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam );

/***************************************************************************
 * To be called by BDMA_Mem_Tran_GetStatus. 
 */
BERR_Code BDMA_P_Mem_Tran_GetStatus_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_TranStatus *        peTranStatus );

/***************************************************************************
 * To be called by BDMA_Mem_Tran_Destroy. It free the queue entries occupied
 * this Tran
 */
BERR_Code BDMA_P_Mem_Tran_Destroy_isr(
	BDMA_Mem_Tran_Handle     hTran );


/***************************************************************************
 * To be called by BDMA_Mem_Tran_Reset. It resets the Tran into its 
 * initially created state
 */
BERR_Code BDMA_P_Mem_Tran_Reset_isr(
	BDMA_Mem_Tran_Handle     hTran );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BDMA_MEM_PRIV_H__ */

/* end of file */
