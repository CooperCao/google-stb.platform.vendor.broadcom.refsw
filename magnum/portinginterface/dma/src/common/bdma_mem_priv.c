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

#include "bdma_mem_priv.h"
#include "bkni.h"
#if (BDMA_P_SUPPORT_MEM_DMA_ENGINE > 0)
#if (BDMA_P_SUPPORT_MEM_DMA_ENGINE>=2)
#include "bchp_mem_dma_1.h"
#endif
#if (BDMA_P_SUPPORT_MEM_DMA_REG_NAME_i!=0)
#include "bchp_mem_dma_0.h"
#else
#include "bchp_mem_dma.h"
#endif
#endif

#include "bchp_common.h"
#include "bchp_hif_intr2.h"

#if (BCHP_CHIP==7400)
#if (BCHP_VER>=BCHP_VER_B0)
#include "bchp_int_id_memc16_gfx_l2.h"
#else
#include "bchp_int_id_gfx_l2.h"
#endif
#elif (BCHP_CHIP==7420) || (BCHP_CHIP==7125) || (BCHP_CHIP==7340)  || (BCHP_CHIP==7342) || \
      (BCHP_CHIP==7468) || (BCHP_CHIP==7422) || (BCHP_CHIP==7425)  || \
      (BCHP_CHIP==7358) || (BCHP_CHIP==7552) || (BCHP_CHIP==7346)  ||  \
      (BCHP_CHIP==7231) || (BCHP_CHIP==7344) || (BCHP_CHIP==73465) || (BCHP_CHIP==7640)
#include "bchp_int_id_bsp_control_intr2.h"
#elif (BCHP_CHIP==7405)
#include "bchp_int_id_memc16_gfx_l2.h"
#elif (BCHP_CHIP==7325) || (BCHP_CHIP==7335) || (BCHP_CHIP==7336)
#include "bchp_int_id_graphics_l2.h"
#elif (BCHP_CHIP==7440) || (BCHP_CHIP==3563) || (BCHP_CHIP==7408) || (BCHP_CHIP == 35230) || (BCHP_CHIP == 35125) || (BCHP_CHIP == 35233)
#include "bchp_int_id_xpt_bus_if.h"
#elif (BCHP_CHIP == 3548) || (BCHP_CHIP == 3556)
#include "bchp_int_id_gfx_l2.h"
#else
#include "bchp_int_id_hif_intr2.h"
#endif

BDBG_MODULE(BDMA_MEM_PRIV);


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
	BKNI_MutexHandle        *phMutex )
{
	BDMA_P_Mem_Context  * pMemDma;
	BERR_Code  eResult = BERR_SUCCESS;

	BDMA_P_MEM_GET_CONTEXT(hMemDma, pMemDma);
	if ( NULL == pMemDma )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_AcquireMutex;
	}

	BDBG_ASSERT( NULL != phMutex );
	*phMutex = pMemDma->hDma->hMutex;

	/* hMemDma passed the magic check, we trust every thing pointed by it now */
	eResult = BKNI_AcquireMutex(pMemDma->hDma->hMutex);

  BDMA_P_Done_Mem_AcquireMutex:
	return BERR_TRACE(eResult);
}


/***************************************************************************
 *
 * Static tool functions used in this file
 *
 ***************************************************************************/

/*--------------------------------------------------------------------------
 * To be called to get MemDma context ptr from a Tran ptr. The MemDma handle
 * from the Tran record is validated by black magic, and the Engine Id is
 * checked to make sure the Tran was really created with MemDma handle
 */
BERR_Code BDMA_P_Mem_GetMemDmaPtrFromTran_isr(
	BDMA_P_QueueEntry *    pTran,
	BDMA_P_Mem_Context **  ppMemDma )
{
	/* assert para from our private code */
	BDBG_ASSERT( NULL != pTran );
	BDBG_ASSERT( NULL != ppMemDma );

	*ppMemDma = (BDMA_P_Mem_Context *) BDMA_P_Queue_GetEngine_isr(pTran->hQueue);

	if (NULL != *ppMemDma)
		return BERR_SUCCESS;
	else
		return BERR_TRACE(BERR_INVALID_PARAMETER);
}

#if BDMA_P_SUPPORT_MEM_DMA_ENGINE
/*--------------------------------------------------------------------------
 * static array for mapping cryptmode enum to HW value
 */
static uint32_t s_ulCryptMode[] =
{
	0,    /* BDMA_CryptMode_eNoCrypt */
	1,    /* BDMA_CryptMode_eBlock */
	2,    /* BDMA_CryptMode_eMpeg */
	3,    /* BDMA_CryptMode_eDirecTv */
};

#define BDMA_P_MEM_NUM_WORDS_PER_DESC       8
#define BDMA_P_MEM_LAST_BLK                 1
#define BDMA_P_MEM_FIRE_INTR                1
#define BDMA_P_MEM_MODE_SEL_1               2
#define BDMA_P_MEM_MODE_1_DES_SEL           0
#define BDMA_P_MEM_ENC_DEC_INIT_SET         1
#define BDMA_P_MEM_ENC_DEC_INIT_UNSET       0
#define BDMA_P_MEM_SG_ENABLE_SET            1
#define BDMA_P_MEM_SG_ENABLE_UNSET          0
#define BDMA_P_MEM_KEY_UNIT(a)              (a * BDMA_P_MEM_KEY_SIZE)  /* logic -> HW value */
#define BDMA_P_MEM_LITTLE_ENDIAN
#define BDMA_P_MEM_CRYPT_MEM_ALIGN_MASK     0x7
#define BDMA_P_MEM_WORD_ALIGN_MASK          0x3
/*--------------------------------------------------------------------------
 * To be called to set block info for a block, it does validate the block
 * info provided
 */
BERR_Code BDMA_P_Mem_SetBlockDesc_isr(
	BDMA_P_Mem_Context *        pMemDma,
	BDMA_P_QueueEntry *         pTran,
	BDMA_P_QueueEntry *         pBlock,
	bool                        bCachedDscrp,
	uint32_t                    ulDstBusAddr,
	uint32_t                    ulSrcBusAddr,
	uint32_t                    ulBlockSize,
	bool                        bCryptInit )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_CryptMode  eCryptMode;
	uint32_t *pulDesc;
	uint32_t  ulCryptMode, ulCryptInit, ulKeyUnit;
	uint32_t  ulSgEnable, ulSgStartEnd;
	uint32_t  ulSwapMode, ulReadEndian, ulNextDescAddr;

	/* assert para from our private code */
	BDBG_ASSERT( NULL != pMemDma );
	BDBG_ASSERT( NULL != pTran );
	BDBG_ASSERT( NULL != pBlock );

	if ( ulBlockSize>BDMA_P_MEM_MAX_BLOCK_SIZE )
	{
		eResult = BERR_TRACE(BDMA_ERR_SIZE_OUT_RANGE);
		goto BDMA_P_Done_Mem_SetBlockDesc;
	}
	if ( (ulSrcBusAddr < ulDstBusAddr) &&
		 (ulDstBusAddr < ulSrcBusAddr+ulBlockSize) )
	{
		eResult = BERR_TRACE(BDMA_ERR_OVERLAP);
		goto BDMA_P_Done_Mem_SetBlockDesc;
	}

	/* set descriptor: only the last block needs to set INTR_ENABLE and LAST bits,
	 * and the INTR_ENABLE is always set, on matter it is a sync or async Tran. */

	pulDesc = (bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;

	/* set desc word 0 */
	*(pulDesc  ) =
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD0, READ_ADDR,    ulSrcBusAddr );

	/* set desc word 1 */
	*(pulDesc+1) =
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD1, WRITE_ADDR,     ulDstBusAddr );

	/* set desc word 2 */
	*(pulDesc+2) =
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD2, TRANSFER_SIZE, ulBlockSize );

	/* set desc word 3 */
	ulNextDescAddr = (pBlock + 1)->ulDescPhysAddr >>
		BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;
	ulReadEndian = (BDMA_Endian_eLittle == pMemDma->eReadEndian)?
		BCHP_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_LITTLE_ENDIAN:
		BCHP_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_BIG_ENDIAN;
	ulSwapMode =
		( (BDMA_SwapMode_eNoSwap == pMemDma->eSwapMode)?
		  (BCHP_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_WORD_ALIGNED):
		  ((BDMA_SwapMode_e0123To2301==pMemDma->eSwapMode)?
		   (BCHP_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_HALF_WORD_ALIGNED):
		   (BCHP_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_BYTE_ALIGNED)) );
	*(pulDesc+3) =
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR,          ulNextDescAddr )      |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD3, READ_ENDIAN_MODE,        ulReadEndian )        |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD3, WRITE_ENDIAN_XLATE_MODE, ulSwapMode );

	/* set desc word 4 */
	ulCryptInit = (bCryptInit) ?
		BDMA_P_MEM_ENC_DEC_INIT_SET : BDMA_P_MEM_ENC_DEC_INIT_UNSET;
	if (pMemDma->bCryptoSetInEngine)
	{
		/* obsolete, for compatibility only */
		eCryptMode = (pMemDma->eCryptMode < BDMA_CryptMode_eInvalid)?
			pMemDma->eCryptMode : BDMA_CryptMode_eNoCrypt;
		ulCryptMode = s_ulCryptMode[eCryptMode];
		ulKeyUnit = BDMA_P_MEM_KEY_UNIT(pMemDma->ulKeySlot);
		ulSgEnable = (pMemDma->bSgEnable) ?
			BDMA_P_MEM_SG_ENABLE_SET : BDMA_P_MEM_SG_ENABLE_UNSET;
	}
	else
	{
		eCryptMode = (pTran->eCryptMode < BDMA_CryptMode_eInvalid)?
			pTran->eCryptMode : BDMA_CryptMode_eNoCrypt;
		ulCryptMode = s_ulCryptMode[eCryptMode];
		ulKeyUnit = BDMA_P_MEM_KEY_UNIT(pTran->ulKeySlot);
		ulSgEnable = (pTran->bSgEnable) ?
			BDMA_P_MEM_SG_ENABLE_SET : BDMA_P_MEM_SG_ENABLE_UNSET;
	}
#if (BDMA_P_SUPPORT_MEM_DMA_SCATTER_GATHER > 0)
	ulSgStartEnd = *(pulDesc+4) & (
		BCHP_MASK( MEM_DMA_DESC_WORD4, SG_SCRAM_START ) |
		BCHP_MASK( MEM_DMA_DESC_WORD4, SG_SCRAM_END ));
	*(pulDesc+4) =
		ulSgStartEnd |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD4, SG_ENABLE,     ulSgEnable )  |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD4, ENC_DEC_INIT,  ulCryptInit ) |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD4, MODE_SEL,      ulCryptMode ) |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD4, KEY_SELECT,    ulKeyUnit );
#else
	BSTD_UNUSED(ulSgStartEnd);
	*(pulDesc+4) =
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD4, ENC_DEC_INIT,  ulCryptInit ) |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD4, MODE_SEL,      ulCryptMode ) |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD4, KEY_SELECT,    ulKeyUnit );
#endif

	/*
	*(pulDesc+5) = 0;
	*(pulDesc+6) = 0;
	*(pulDesc+7) = 0;
	*/

  BDMA_P_Done_Mem_SetBlockDesc:
	return eResult;
}

#define BDMA_P_SG_START_SHIFT   0
#define BDMA_P_SG_END_SHIFT     1
#define BDMA_P_SG_START_MASK    (1<<BDMA_P_SG_START_SHIFT)
#define BDMA_P_SG_END_MASK      (1<<BDMA_P_SG_END_SHIFT)

/*--------------------------------------------------------------------------
 * To be called to mark / unmark a block as scatter-gather start / end point
 */
static BERR_Code BDMA_P_Mem_SetSgStartEndDesc_isr(
	BDMA_P_QueueEntry *         pBlock,
	bool                        bCachedDscrp,
	bool                        bSgStart,
	bool                        bSgEnd )
{
#if (BDMA_P_SUPPORT_MEM_DMA_SCATTER_GATHER > 0)
	uint32_t *pulDesc;
	uint32_t  ulWord4, ulSgStart, ulSgEnd;

	pulDesc = (bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;

	ulWord4 = *(pulDesc+4) & ~(
		BCHP_MASK( MEM_DMA_DESC_WORD4, SG_SCRAM_START ) |
		BCHP_MASK( MEM_DMA_DESC_WORD4, SG_SCRAM_END ));
	ulSgStart = (bSgStart) ? 1 : 0;
	ulSgEnd = (bSgEnd) ? 1 : 0;

	*(pulDesc+4) =
		ulWord4 |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD4, SG_SCRAM_START, ulSgStart ) |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD4, SG_SCRAM_END,   ulSgEnd );

	pBlock->ulSgStartStop =
		(ulSgStart<<BDMA_P_SG_START_SHIFT) | (ulSgEnd << BDMA_P_SG_END_SHIFT);

#else
	BSTD_UNUSED(pBlock);
	BSTD_UNUSED(bCachedDscrp);
	BSTD_UNUSED(bSgStart);
	BSTD_UNUSED(bSgEnd);
#endif
	return BERR_SUCCESS;
}

#ifndef BCHP_MEM_DMA_0_FIRST_DESC
#define BCHP_MEM_DMA_0_FIRST_DESC                BCHP_MEM_DMA_FIRST_DESC
#define BCHP_MEM_DMA_0_FIRST_DESC_ADDR_SHIFT     BCHP_MEM_DMA_FIRST_DESC_ADDR_SHIFT
#define BCHP_MEM_DMA_0_CTRL                      BCHP_MEM_DMA_CTRL
#define BCHP_MEM_DMA_0_CTRL_RUN_SHIFT            BCHP_MEM_DMA_CTRL_RUN_SHIFT
#define BCHP_MEM_DMA_0_WAKE_CTRL                 BCHP_MEM_DMA_WAKE_CTRL
#define BCHP_MEM_DMA_0_WAKE_CTRL_WAKE_MODE_SHIFT BCHP_MEM_DMA_WAKE_CTRL_WAKE_MODE_SHIFT
#define BCHP_MEM_DMA_0_WAKE_CTRL_WAKE_SHIFT      BCHP_MEM_DMA_WAKE_CTRL_WAKE_SHIFT
#define BCHP_MEM_DMA_0_STATUS                    BCHP_MEM_DMA_STATUS
#define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_MASK    BCHP_MEM_DMA_STATUS_DMA_STATUS_MASK
#define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_SHIFT   BCHP_MEM_DMA_STATUS_DMA_STATUS_SHIFT
#define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Idle    BCHP_MEM_DMA_STATUS_DMA_STATUS_Idle
#define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Busy    BCHP_MEM_DMA_STATUS_DMA_STATUS_Busy
#define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Sleep   BCHP_MEM_DMA_STATUS_DMA_STATUS_Sleep
#define BCHP_MEM_DMA_0_CUR_DESC                  BCHP_MEM_DMA_CUR_DESC
#define BCHP_MEM_DMA_0_CUR_DESC_ADDR_MASK        BCHP_MEM_DMA_CUR_DESC_ADDR_MASK
#define BCHP_MEM_DMA_0_CUR_DESC_ADDR_SHIFT       BCHP_MEM_DMA_CUR_DESC_ADDR_SHIFT
#define BCHP_MEM_DMA_0_SCRATCH                   BCHP_MEM_DMA_SCRATCH
#endif

#ifndef BCHP_INT_ID_MEM_DMA_0_INTR
#ifdef BCHP_INT_ID_MEM_DMA_INTR
#define BCHP_INT_ID_MEM_DMA_0_INTR               BCHP_INT_ID_MEM_DMA_INTR
#elif defined BCHP_INT_ID_XPT_BUS_IF_MEM_DMA_INTR
#define BCHP_INT_ID_MEM_DMA_0_INTR               BCHP_INT_ID_XPT_BUS_IF_MEM_DMA_INTR
#elif defined BCHP_INT_ID_BSP_CONTROL_INTR2_MEM_DMA_0_INTR
#define BCHP_INT_ID_MEM_DMA_0_INTR               BCHP_INT_ID_BSP_CONTROL_INTR2_MEM_DMA_0_INTR
#else
#define BCHP_INT_ID_MEM_DMA_0_INTR               BCHP_INT_ID_MEM_DMA_INT
#endif
#endif

#define BDMA_P_MEM_STATUS_IDLE            BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Idle
#define BDMA_P_MEM_STATUS_BUSY            BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Busy
#define BDMA_P_MEM_STATUS_SLEEP           BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Sleep
#define BDMA_P_MEM_WAKE_FROM_1ST_DESC     1
#define BDMA_P_MEM_WAKE_FROM_LAST_DESC    0
#define BDMA_P_MEM_WAKE_UP                1
#define BDMA_P_MEM_RUN_START              1

/*--------------------------------------------------------------------------
 * To be called to modify the last act block's desc to work as last block,
 * and to flush cached descriptors
 */
static BERR_Code BDMA_P_Mem_Tran_PrepareStart_isr(
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

	BDMA_P_QUEUE_GET_CONTEXT( hQueue, pQueue );
	BDBG_ASSERT( NULL != pQueue );

	/* in the case of re-use, the previous "last act block" might have wrong NEXT_DESC */
	if ((ulNumActBlocks != pTran->ulNumActBlocks) &&
		(0 < pTran->ulNumActBlocks))
	{
		pBlock = pTran + (pTran->ulNumActBlocks - 1);
		pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;

		*(pulDesc+2) &= ~(
#if BDMA_P_SUPPORT_MEM_DMA_AUTO_APPEND
			BCHP_MASK(MEM_DMA_DESC_WORD2, AUTO_APPEND) |
#endif
			BCHP_MASK(MEM_DMA_DESC_WORD2, INTR_ENABLE) |
			BCHP_MASK(MEM_DMA_DESC_WORD2, LAST));

		ulNextOffset =
			(pBlock + 1)->ulDescPhysAddr >> BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;
		ulWord = *(pulDesc+3) & ~(BCHP_MASK(MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR));
		*(pulDesc+3) = ulWord |
			BCHP_FIELD_DATA( MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR, ulNextOffset );
	}

	/* modify new "last act block" desc */
	pBlock = pTran + (ulNumActBlocks - 1);
	pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;
#if BDMA_P_SUPPORT_MEM_DMA_AUTO_APPEND
	ulWord = *(pulDesc+2) & ~(
		BCHP_MASK(MEM_DMA_DESC_WORD2, AUTO_APPEND));
	*(pulDesc+2) = ulWord | (
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD2, INTR_ENABLE,   BDMA_P_MEM_FIRE_INTR ) |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD2, LAST,          BDMA_P_MEM_LAST_BLK ));
#else
	*(pulDesc+2) |= (
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD2, INTR_ENABLE,   BDMA_P_MEM_FIRE_INTR ) |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD2, LAST,          BDMA_P_MEM_LAST_BLK ));
#endif
	*(pulDesc+3) &= ~(
		BCHP_MASK(MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR));

	*pbSgOpen = ((pTran->bSgEnable) &&
				 (0 == (BDMA_P_SG_END_MASK & pBlock->ulSgStartStop)));

#if (BDMA_P_CHECK_NEXT_DESC) && (BDBG_DEBUG_BUILD)
	pBlock = pTran;
	for ( iLoopCntr = 0; iLoopCntr < (int) ulNumActBlocks; iLoopCntr++ )
	{
		uint32_t ulSeeNextOffset;

		ulNextOffset =
			(pBlock + 1)->ulDescPhysAddr >> BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;
		pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;
		ulSeeNextOffset = BCHP_GET_FIELD_DATA(*(pulDesc+3), MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR);

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
							ulNumActBlocks * BDMA_P_MEM_NUM_WORDS_PER_DESC * 4);

	/* read back with un-cached addr to ensure previous writing completes */
	ulCheck = *((pTran + (ulNumActBlocks - 1))->pulDescAddr + 3);
	if ((ulCheck & BCHP_MASK(MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR)) != 0)
	{
		BDBG_ERR(("desc writing error with preparing start"));
	}

	return BERR_SUCCESS;
}
/*--------------------------------------------------------------------------
 * To be called to modify the last act block's desc to work as last block,
 * and to flush cached descriptors
 */
static BERR_Code BDMA_P_Mem_Tran_PrepareStartSubset_isr(
	BDMA_P_QueueEntry *      pTran,
	BDMA_P_QueueHandle       hQueue,
	BMEM_Handle              hMemory,
	uint32_t                 ulFirstBlock,
	uint32_t                 ulNumActBlocks,  /* new NumActBlocks */
	bool  *                  pbSgOpen )
{
	BDMA_P_QueueContext * pQueue;
	BDMA_P_QueueEntry *pBlock;
	uint32_t *pulDesc;
	uint32_t  ulNextOffset, i=0;
	uint32_t  ulWord;
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
#if BDMA_P_SUPPORT_MEM_DMA_AUTO_APPEND
			BCHP_MASK(MEM_DMA_DESC_WORD2, AUTO_APPEND) |
#endif
			BCHP_MASK(MEM_DMA_DESC_WORD2, INTR_ENABLE) |
			BCHP_MASK(MEM_DMA_DESC_WORD2, LAST));

		ulNextOffset =
			(pBlock + 1)->ulDescPhysAddr >> BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;
		ulWord = *(pulDesc+3) & ~(BCHP_MASK(MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR));
		*(pulDesc+3) = ulWord |
			BCHP_FIELD_DATA( MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR, ulNextOffset );
	}

    pTran->ulFirstBlock = ulFirstBlock;
	/* if re-use, the previous descriptor might have wrong setting */
	pBlock = pTran + ulFirstBlock;
	while (i<(ulNumActBlocks - 1))
	{
		pBlock = pTran + ulFirstBlock + i;
		pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;

#if BDMA_P_SUPPORT_MEM_DMA_AUTO_APPEND
	ulWord = *(pulDesc+2) & ~(
		BCHP_MASK(MEM_DMA_DESC_WORD2, AUTO_APPEND));

		*(pulDesc+2) = ulWord & (
			BCHP_FIELD_DATA( MEM_DMA_DESC_WORD2, INTR_ENABLE,	BDMA_P_MEM_FIRE_INTR ) |
			BCHP_FIELD_DATA( MEM_DMA_DESC_WORD2, LAST,			BDMA_P_MEM_LAST_BLK ));
#else
		*(pulDesc+2) &=~(
			BCHP_FIELD_DATA( MEM_DMA_DESC_WORD2, INTR_ENABLE,	BDMA_P_MEM_FIRE_INTR ) |
			BCHP_FIELD_DATA( MEM_DMA_DESC_WORD2, LAST,			BDMA_P_MEM_LAST_BLK ));
#endif

		ulNextOffset = (pBlock + 1)->ulDescPhysAddr >> BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;

		*(pulDesc+3) |= BCHP_FIELD_DATA( MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR, ulNextOffset );
		i++;
	}

	/* modify new "last act block" desc */
	pBlock = pTran + ulFirstBlock + (ulNumActBlocks - 1);
	pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;
#if BDMA_P_SUPPORT_MEM_DMA_AUTO_APPEND
	ulWord = *(pulDesc+2) & ~(
		BCHP_MASK(MEM_DMA_DESC_WORD2, AUTO_APPEND));
	*(pulDesc+2) = ulWord | (
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD2, INTR_ENABLE,   BDMA_P_MEM_FIRE_INTR ) |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD2, LAST,          BDMA_P_MEM_LAST_BLK ));
#else
	*(pulDesc+2) |= (
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD2, INTR_ENABLE,   BDMA_P_MEM_FIRE_INTR ) |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD2, LAST,          BDMA_P_MEM_LAST_BLK ));
#endif
	*(pulDesc+3) &= ~(
		BCHP_MASK(MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR));

	*pbSgOpen = ((pTran->bSgEnable) &&
				 (0 == (BDMA_P_SG_END_MASK & pBlock->ulSgStartStop)));

#if (BDMA_P_CHECK_NEXT_DESC) && (BDBG_DEBUG_BUILD)
	pBlock = pTran;
	for ( iLoopCntr = 0; iLoopCntr < (int) ulNumActBlocks; iLoopCntr++ )
	{
		uint32_t ulSeeNextOffset;

		ulNextOffset =
			(pBlock + 1)->ulDescPhysAddr >> BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT;
		pulDesc = (pTran->bCachedDscrp)? pBlock->pulCachedDescAddr : pBlock->pulDescAddr;
		ulSeeNextOffset = BCHP_GET_FIELD_DATA(*(pulDesc+3), MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR);

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
							ulNumActBlocks * BDMA_P_MEM_NUM_WORDS_PER_DESC * 4);

	/* read back with un-cached addr to ensure previous writing completes */
	ulCheck = *((pTran + ulFirstBlock + (ulNumActBlocks - 1))->pulDescAddr + 3);
	if ((ulCheck & BCHP_MASK(MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR)) != 0)
	{
		BDBG_ERR(("desc writing error with preparing start"));
	}

	return BERR_SUCCESS;
}
/*--------------------------------------------------------------------------
Callback from base-module queue to wait till next descriptor safe to send to HW
*/
static void BDMA_P_Mem_SafeSendToHw_isr(
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
	uint32_t ulNewKeySel  = 0, ulLastKeySel  = 0;
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

	ulNewEncry	= BCHP_GET_FIELD_DATA(ulNewDesc4, MEM_DMA_DESC_WORD4, SG_SCRAM_START);
	ulNewModeSel = BCHP_GET_FIELD_DATA(ulNewDesc4, MEM_DMA_DESC_WORD4, MODE_SEL);
	ulNewKeySel	= BCHP_GET_FIELD_DATA(ulNewDesc4, MEM_DMA_DESC_WORD4, KEY_SELECT);





	if(pLastTran)
	{
		pulLastDesc = (pLastTran + (pLastTran->ulNumActBlocks - 1))->pulDescAddr;
		ulLastDesc4 = *(pulLastDesc + 4);
		ulLastEncry	 = BCHP_GET_FIELD_DATA(ulLastDesc4, MEM_DMA_DESC_WORD4, SG_SCRAM_START);
		ulLastModeSel = BCHP_GET_FIELD_DATA(ulLastDesc4, MEM_DMA_DESC_WORD4, MODE_SEL);
		ulLastKeySel  = BCHP_GET_FIELD_DATA(ulLastDesc4, MEM_DMA_DESC_WORD4, KEY_SELECT);
		bAppendable = (ulLastEncry == ulNewEncry)&&
			(ulLastModeSel==ulNewModeSel)&&
			(ulLastKeySel==ulNewKeySel);

	}

	/* 2: Safe to send the next descriptor if the continuous two are the same*/
	if(bAppendable)
		return;

	/* 3: Safe to send the next descriptor or the previous in HW is done*/

	pMemDma = (BDMA_P_Mem_Context *)(pQueue->hEngine);
	hRegister = BDMA_P_GetRegisterHandle(pMemDma->hDma);
	while (eStatus ==BDMA_P_MEM_STATUS_BUSY)
	{
		/* read CUR_DESC after STATUS, --> less chance to get wrong CUR_DESC?  */
		ulRegValue = BREG_Read32( hRegister, BCHP_MEM_DMA_0_STATUS + pMemDma->ulRegOffset );
		eStatus    = BCHP_GET_FIELD_DATA( ulRegValue, MEM_DMA_0_STATUS, DMA_STATUS );
	}

	switch (eStatus){
		   case (BDMA_P_MEM_STATUS_IDLE):
			   *peHwStatus = BDMA_P_HwStatus_eIdle;
			   break;
		   case (BDMA_P_MEM_STATUS_SLEEP):
			   *peHwStatus = BDMA_P_HwStatus_eSleep;
			   break;
	}

}


/*--------------------------------------------------------------------------
* Callback from base-module queue to read the HW status
 */
static void BDMA_P_Mem_ReadHwRegs_isr(
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
	ulRegValue = BREG_Read32( hRegister, BCHP_MEM_DMA_0_STATUS + pMemDma->ulRegOffset );
	ulStatus  = BCHP_GET_FIELD_DATA( ulRegValue, MEM_DMA_0_STATUS, DMA_STATUS );

	ulRegValue = BREG_Read32( hRegister, BCHP_MEM_DMA_0_CUR_DESC + pMemDma->ulRegOffset );
	*pulCurrDesc = BCHP_GET_FIELD_DATA( ulRegValue, MEM_DMA_0_CUR_DESC, ADDR );

#ifdef BDMA_P_SUPPORT_SHARF_DMA_ENGINE
	if (NULL != pMemDma->hSharf)
		*pulScratchReg = BREG_Read32( hRegister, BCHP_MEM_DMA_0_SCRATCH + pMemDma->ulRegOffset - 4);
	else
		*pulScratchReg = BREG_Read32( hRegister, BCHP_MEM_DMA_0_SCRATCH + pMemDma->ulRegOffset );
#else
	*pulScratchReg = BREG_Read32( hRegister, BCHP_MEM_DMA_0_SCRATCH + pMemDma->ulRegOffset );
#endif

	if ( BDMA_P_MEM_STATUS_IDLE == ulStatus )
	{
		BDBG_MSG(( "Mem DMA engine in idle state!" ));
		*peHwStatus = BDMA_P_HwStatus_eIdle;
	}
	else if ( BDMA_P_MEM_STATUS_BUSY == ulStatus )
	{
		*peHwStatus = BDMA_P_HwStatus_eBusy;
	}
	else if ( BDMA_P_MEM_STATUS_SLEEP == ulStatus )
	{
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
static void BDMA_P_Mem_AppendTranDesc_isr(
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
		BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, AUTO_APPEND, 1);
#else
	*(pulLastDesc+2) &= ~(
		BCHP_MASK(MEM_DMA_DESC_WORD2, LAST));
#endif
	ulWord3 = *(pulLastDesc+3) & ~(BCHP_MASK(MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR));
	*(pulLastDesc+3) = ulWord3 |
		BCHP_FIELD_DATA( MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR,
			ulNewDescPhysAddr >> BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT );
}

/*--------------------------------------------------------------------------
 * Callback from base-module queue to send one Tran to HW.
 *
 * Note: It assume block info is already set for all blocks in this Tran
 */
static void BDMA_P_Mem_SendToHw_isr(
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
#ifdef BDMA_P_SUPPORT_SHARF_DMA_ENGINE
	if (NULL != pMemDma->hSharf)
		BREG_Write32( hRegister, BCHP_MEM_DMA_0_SCRATCH + pMemDma->ulRegOffset - 4,
					  ulScratchReg );
	else
		BREG_Write32( hRegister, BCHP_MEM_DMA_0_SCRATCH + pMemDma->ulRegOffset,
					  ulScratchReg );
#else
	BREG_Write32( hRegister, BCHP_MEM_DMA_0_SCRATCH + pMemDma->ulRegOffset,
				  ulScratchReg );
#endif

	/* send Trans queued in SW into HW */
	pLastTran = pQueue->pTailTranInHw;
	if (bAppendInHw && (NULL != pLastTran))
	{
		pulLastDesc = (pLastTran + (pLastTran->ulNumActBlocks - 1))->pulDescAddr;

		ulDescWord = *(pulLastDesc+3) & ~(BCHP_MASK(MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR));
		*(pulLastDesc+3) = ulDescWord |
			BCHP_FIELD_DATA( MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR,
				ulNewDescPhysAddr >> BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT );

#if BDMA_P_SUPPORT_MEM_DMA_AUTO_APPEND
		/* auto-append to last Tran in hw, which might and might not finished
		 * OK to mix scatter-gather with no-scatter-gather linked-lists  */
		*(pulLastDesc+2) |=
			BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, AUTO_APPEND, 1);
#else
		/* sw dynamically link to last Tran in hw, which might and might not finished,
		 * must not mix scatter-gather with no-scatter-gather linked-lists  */
		*(pulLastDesc+2) &= ~(BCHP_MASK(MEM_DMA_DESC_WORD2, LAST));
#endif

		/* read back to force that mem writing completes before reg writing */
		ulCheck = *(pulLastDesc+2) | *(pulLastDesc+3);

#if (1 == BDMA_P_SHOW_DYN_LINK_FAIL)
		BDBG_ERR(("append to 0x%08lx by new tran 0x%08lx",
				  (pLastTran + (pLastTran->ulNumActBlocks - 1))->ulDescPhysAddr,
				  ulNewDescPhysAddr));
#endif

		/* in case HW already in sleep mode */
		BREG_Write32( hRegister, BCHP_MEM_DMA_0_WAKE_CTRL + pMemDma->ulRegOffset,
			BCHP_FIELD_DATA(MEM_DMA_0_WAKE_CTRL, WAKE_MODE, BDMA_P_MEM_WAKE_FROM_LAST_DESC) |
			BCHP_FIELD_DATA(MEM_DMA_0_WAKE_CTRL, WAKE,      BDMA_P_MEM_WAKE_UP) );
	}
	else
	{
        ulNewDescPhysAddr = pQueue->pHeadTranInQ->ulDescPhysAddr 
			+ ( pQueue->pHeadTranInQ->ulFirstBlock * 4 * BDMA_P_MEM_NUM_WORDS_PER_DESC);
#if (1 == BDMA_P_SHOW_DYN_LINK_FAIL)
		BDBG_ERR(("start new tran 0x%08lx", ulNewDescPhysAddr));
#endif
		/* must start from FIRST_DESC */
		BREG_Write32( hRegister, BCHP_MEM_DMA_0_FIRST_DESC + pMemDma->ulRegOffset,
			BCHP_FIELD_DATA(MEM_DMA_0_FIRST_DESC, ADDR, ulNewDescPhysAddr) );

		if ( BDMA_P_HwStatus_eIdle != eHwStatus )
		{
			/* mem dma started, but last Tran might be destroyed */
			BREG_Write32( hRegister, BCHP_MEM_DMA_0_WAKE_CTRL + pMemDma->ulRegOffset,
				BCHP_FIELD_DATA(MEM_DMA_0_WAKE_CTRL, WAKE_MODE, BDMA_P_MEM_WAKE_FROM_1ST_DESC) |
				BCHP_FIELD_DATA(MEM_DMA_0_WAKE_CTRL, WAKE,      BDMA_P_MEM_WAKE_UP) );
		}
		else
		{
			/* mem dma has never started */
			BREG_Write32( hRegister, BCHP_MEM_DMA_0_CTRL + pMemDma->ulRegOffset,
				BCHP_FIELD_DATA(MEM_DMA_0_CTRL, RUN, BDMA_P_MEM_RUN_START) );
		}
	}
}

/*--------------------------------------------------------------------------
 * Callback from base-module queue to check NEXT_DESC_ADDR and print err msg
 * dma engine reg read error happens
 */
static void BDMA_P_Mem_CheckNextDesc_isr(
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
		ulSeeNextOffset = (BCHP_GET_FIELD_DATA(*(pulDesc+3), MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR) <<
						   BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT);

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
static void BDMA_P_Mem_DoneCallBack_isr(
	void *                pParm1,  /* pMemDma */
	int                   parm2 )  /* not used */
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;

	BSTD_UNUSED(parm2);
	BDMA_P_MEM_GET_CONTEXT(pParm1, pMemDma);
	if ( NULL == pMemDma )
	{
		eResult = BERR_TRACE( BERR_INVALID_PARAMETER );
		BDBG_ERR(( "Invaid pParm1 to BDMA_P_Mem_DoneCallBack_isr!" ));
		goto BDMA_P_Done_Mem_DoneCallBack_isr;
	}

	/* read status from HW and update records */
	BDMA_P_Queue_CheckHwAndSendNewReq_isr( pMemDma->hQueue );

  BDMA_P_Done_Mem_DoneCallBack_isr:
	return;
}


/***************************************************************************
 *
 * API support functions
 *
 ***************************************************************************/

/*--------------------------------------------------------------------------
 * To be called to init mem dma engine: 1). init Hw, 2). create queue, 3).
 * create call-back, 4). init crpto setting.
 */
BERR_Code BDMA_P_Mem_Init(
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
		&hQueue, (void *) pMemDma, hMemory, BDMA_P_MEM_NUM_WORDS_PER_DESC,
		(BDMA_P_CmnSettings *)pSettings, BDMA_P_Mem_ReadHwRegs_isr,
		BDMA_P_Mem_AppendTranDesc_isr, BDMA_P_Mem_SendToHw_isr,
		BDMA_P_Mem_CheckNextDesc_isr, BDMA_P_Mem_SafeSendToHw_isr);
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Mem_Init );
	pMemDma->hQueue = hQueue;

	if (pSettings->bSupportCallBack)
	{
		/* create done-interrupt call back
		 * no need to create call back for BCHP_INT_ID_MEM_DMA_ERR_INTR,
		 * since PI already checks size / error without sending HW ??? */
		/* BCHP_INT_ID_MEM_DMA_0_INTR, or BCHP_INT_ID_MEM_DMA_1_INTR */
		eResult = BINT_CreateCallback(
			&hCallBack, BDMA_P_GetInterruptHandle( hDma ),
			ulL2IntrBintId, BDMA_P_Mem_DoneCallBack_isr,
			(void *)pMemDma, 0 );
		BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Mem_Init );
		pMemDma->hCallBack = hCallBack;

		eResult = BINT_ClearCallback( hCallBack );
		BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Mem_Init );
		eResult = BINT_EnableCallback( hCallBack );
		BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Mem_Init );
	}

	/* init op */
	pMemDma->eReadEndian = BDMA_Endian_eLittle;
	pMemDma->eSwapMode = BDMA_SwapMode_eNoSwap;
	pMemDma->eCryptMode = BDMA_CryptMode_eNoCrypt;
	pMemDma->ulKeySlot = 0;

	return eResult;

  BDMA_P_Err_Mem_Init:
	BDMA_P_Mem_UnInit( pMemDma );

	return BERR_TRACE(eResult);
}


/*--------------------------------------------------------------------------
 * To be called to uninit mem dma engine: destroy queue and call-back
 */
BERR_Code BDMA_P_Mem_UnInit(
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

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Create(
	BDMA_Handle           hDma,
	BDMA_MemDmaEngine     eEngine,
	const BDMA_Mem_Settings *pSettings,
	BDMA_Mem_Handle *     phMemDma )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Context *  pDma;
	BDMA_P_Mem_Context  *pMemDma = NULL;
	uint32_t  ulL2IntrBintId;

	if ( NULL != phMemDma )
		*phMemDma = NULL;

	BDMA_P_MAIN_GET_CONTEXT( hDma, pDma );
	if ( (NULL == phMemDma) || (NULL == pDma) || (eEngine >= BDMA_MemDmaEngine_eUnknown))
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Err_Mem_Create;
	}
	if (BDMA_P_SUPPORT_MEM_DMA_ENGINE <= eEngine)
	{
		eResult = BERR_TRACE(BDMA_ERR_ENGINE_NOT_SUPPORTED);
		goto BDMA_P_Err_Mem_Create;
	}
	if ( NULL != BDMA_P_GetMemDmaHandle(hDma, eEngine) )
	{
		/* memeory dma engine sub-module is already created */
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

#if (BDMA_P_SUPPORT_MEM_DMA_ENGINE>=2)
	if (BDMA_MemDmaEngine_e0 == eEngine)
	{
		ulL2IntrBintId = BCHP_INT_ID_MEM_DMA_0_INTR;
		pMemDma->ulRegOffset = 0;
	}
	else
	{
		ulL2IntrBintId = BCHP_INT_ID_MEM_DMA_1_INTR;
		pMemDma->ulRegOffset = BCHP_MEM_DMA_1_REG_START - BCHP_MEM_DMA_0_REG_START;
	}
#else
		ulL2IntrBintId = BCHP_INT_ID_MEM_DMA_0_INTR;
		pMemDma->ulRegOffset = 0;
#endif

	/* init mem dma */
	pMemDma->hDma = hDma;
	pMemDma->eEngine = eEngine;
	eResult = BDMA_P_Mem_Init( pMemDma, pSettings, ulL2IntrBintId );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Err_Mem_Create );

	/* connect mem dma sub-module to dma module's main context */
	BDMA_P_SetMemDmaHandle( hDma, eEngine, pMemDma );

	*phMemDma = pMemDma;
	return eResult;

  BDMA_P_Err_Mem_Create:

	if ( NULL != pMemDma )
		BDMA_P_MEM_DESTROY_CONTEXT( pMemDma );

	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Destroy(
	BDMA_Mem_Handle       hMemDma )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;

	BDMA_P_MEM_GET_CONTEXT(hMemDma, pMemDma);
	if ( NULL == pMemDma )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Destroy;
	}

	/* disconnect mem dma sub-module to dma module's main context */
	BDMA_P_SetMemDmaHandle( pMemDma->hDma, pMemDma->eEngine, NULL );

	/* destroy callBack and queue */
	BDMA_P_Mem_UnInit( pMemDma );

	BDMA_P_MEM_DESTROY_CONTEXT( pMemDma );

  BDMA_P_Done_Mem_Destroy:
	return eResult;
}

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Tran_SetCrypto_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_CryptMode           eCryptMode,
	uint32_t                 ulKeySlot,
	bool                     bSgEnable )
{
	BDMA_P_QueueEntry * pTran;
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ((NULL == pTran) ||
		(eCryptMode >= BDMA_CryptMode_eInvalid) ||
		(ulKeySlot > BDMA_P_MEM_KEY_SLOT_MAX))
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Tran_SetCrypt;
	}

	eResult = BDMA_P_Mem_GetMemDmaPtrFromTran_isr( pTran, &pMemDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Tran_SetCrypt );
	if (pMemDma->bCryptoSetInEngine)
	{
		eResult = BERR_TRACE(BDMA_ERR_OBSOLETE_API_MIX);
		goto BDMA_P_Done_Mem_Tran_SetCrypt;
	}

	pMemDma->bCryptoSetInTran = true;
	pTran->eCryptMode = eCryptMode;
	pTran->ulKeySlot = ulKeySlot;
	pTran->bSgEnable = bSgEnable;

  BDMA_P_Done_Mem_Tran_SetCrypt:
	return eResult;
}

/* obsolete, for back compatibility only */
BERR_Code BDMA_P_Mem_SetCrypto_isr(
	BDMA_Mem_Handle          hMemDma,
	BDMA_CryptMode           eCryptMode,
	uint32_t                 ulKeySlot,
	bool                     bSgEnable )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;

	BDMA_P_MEM_GET_CONTEXT(hMemDma, pMemDma);
	if ( (NULL == pMemDma) ||
		 (eCryptMode >= BDMA_CryptMode_eInvalid) ||
		 (ulKeySlot > BDMA_P_MEM_KEY_SLOT_MAX))
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_SetCrypt;
	}

	if (pMemDma->bCryptoSetInTran)
	{
		eResult = BERR_TRACE(BDMA_ERR_OBSOLETE_API_MIX);
		goto BDMA_P_Done_Mem_SetCrypt;
	}

	pMemDma->bCryptoSetInEngine = true;
	pMemDma->eCryptMode = eCryptMode;
	pMemDma->ulKeySlot = ulKeySlot;
	pMemDma->bSgEnable = bSgEnable;

  BDMA_P_Done_Mem_SetCrypt:
	return eResult;
}

/***************************************************************************
 * To be called to set the block info for a Tran block. Input block info are
 * validated against the current engine state.
 */
BERR_Code BDMA_P_Mem_Tran_SetDmaBlockInfo_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize,
	bool                     bCryptInit )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;
	BDMA_P_QueueEntry * pTran;
	BDMA_P_QueueEntry * pBlock;

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Tran_SetDmaBlockInfo;
	}

	if ( pTran->ulNumTranBlocks <= ulBlockId )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Tran_SetDmaBlockInfo;
	}

	eResult = BDMA_P_Mem_GetMemDmaPtrFromTran_isr( pTran, &pMemDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Tran_SetDmaBlockInfo );

	/* validate addr, size and set desc */
	pBlock = pTran + ulBlockId;
	eResult = BDMA_P_Mem_SetBlockDesc_isr( pMemDma,
										   pTran, pBlock,
										   pTran->bCachedDscrp,
										   ulDstBusAddr, ulSrcBusAddr,
										   ulBlockSize, bCryptInit );
	if ( BERR_SUCCESS == eResult )
		pBlock->bBlockInfoSet = true;

  BDMA_P_Done_Mem_Tran_SetDmaBlockInfo:
	return BERR_TRACE( eResult );
}


/***************************************************************************
 * To be called to mark / unmark the block as scatter-gather start /end point
 * validated against the current engine state.
 */
BERR_Code BDMA_P_Mem_Tran_SetSgStartEnd_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	bool                     bSgStart,
	bool                     bSgEnd )
{
#if (BDMA_P_SUPPORT_MEM_DMA_SCATTER_GATHER)
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_QueueEntry * pTran;
	BDMA_P_QueueEntry * pBlock;

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Tran_SetSgStartEnd;
	}

	if ( pTran->ulNumTranBlocks <= ulBlockId )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Tran_SetSgStartEnd;
	}

	pTran->bSgStartStopSet = true;

	pBlock = pTran + ulBlockId;
	eResult = BDMA_P_Mem_SetSgStartEndDesc_isr(
		pBlock, pTran->bCachedDscrp, bSgStart, bSgEnd );

  BDMA_P_Done_Mem_Tran_SetSgStartEnd:
	return BERR_TRACE( eResult );
#else
	BSTD_UNUSED(hTran);
	BSTD_UNUSED(ulBlockId);
	BSTD_UNUSED(bSgStart);
	BSTD_UNUSED(bSgEnd);

	return BERR_TRACE(BDMA_ERR_FEATURE_NOT_SUPPORTED);
#endif
}

/***************************************************************************
 */

BERR_Code BDMA_P_Mem_Tran_StartSubset_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulFirstBlock,
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
		goto BDMA_P_Done_Mem_Tran_Start;
	}

	eResult = BDMA_P_Mem_GetMemDmaPtrFromTran_isr( pTran, &pMemDma );
	BDBG_ERR(("\n BDMA_P_Mem_GetMemDmaPtrFromTran_isr"));
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Tran_Start );

	eResult = BDMA_P_Mem_Tran_PrepareStartSubset_isr( pTran, pMemDma->hQueue,
		BDMA_P_GetMemoryHandle(pMemDma->hDma), ulFirstBlock, ulNumActBlocks, &bSgOpen );
	BDBG_ERR(("\n BDMA_P_Mem_Tran_PrepareStartSubset_isr"));
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Tran_Start );

	eResult = BDMA_P_Queue_StartTran_isr (
		pMemDma->hQueue, pTran, ulNumActBlocks, bSgOpen,
		(BDMA_P_CallbackFunc) pCallBackFunc_isr, pUserCallBackParam );

  BDMA_P_Done_Mem_Tran_Start:
	return BERR_TRACE(eResult);
}

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
	void *                   pUserCallBackParam )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;
	BDMA_P_QueueEntry * pTran;
	BDMA_P_QueueEntry * pBlock;
	bool bLastBlock;
	bool  bSgOpen;

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Transfer;
	}

	if ( pTran->ulNumTranBlocks <= ulBlockId )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Transfer;
	}

	/* get mem dma pointer */
	eResult = BDMA_P_Mem_GetMemDmaPtrFromTran_isr( pTran, &pMemDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Transfer );

	/* tell queue module to reset this Tran into initially created state */
	eResult = BDMA_P_Queue_ResetTran_isr ( pMemDma->hQueue, pTran );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Transfer );

	/* set transfer information */
	/* validate addr, size and set desc */
	bLastBlock = (pTran->ulNumTranBlocks == (ulBlockId+1));
	pBlock = pTran + ulBlockId;
	eResult = BDMA_P_Mem_SetBlockDesc_isr( pMemDma,
										   pTran, pBlock,
										   bLastBlock,
										   ulDstBusAddr, ulSrcBusAddr,
										   ulBlockSize, bCryptInit );
	if ( BERR_SUCCESS == eResult )
		pBlock->bBlockInfoSet = true;

	eResult = BDMA_P_Mem_Tran_PrepareStart_isr( pTran, pMemDma->hQueue,
		BDMA_P_GetMemoryHandle(pMemDma->hDma), 1, &bSgOpen );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Transfer );

	eResult = BDMA_P_Queue_StartTran_isr ( pMemDma->hQueue,
										   pTran, 1, bSgOpen,
										   (BDMA_P_CallbackFunc) pCallBackFunc_isr,
										   pUserCallBackParam );

  BDMA_P_Done_Mem_Transfer:
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * To be called by both BDMA_P_Mem_Tran_Start_isrAndCallBack and
 * BDMA_P_Mem_Tran_Start_isr. BDMA_P_Mem_Tran_Start_isr should pass NULL for
 * pCallBackFunc_isr and pUserCallBackParam
 *
 * Note: when a Tran is sent to HW, interrupt is alwayas enabled. If the
 * Tran is async, the module's interrupt handler BDMA_P_Mem_DoneCallBack_isr
 * will call user's callback function. In the sync case, there is no user's
 * call back function to call, BDMA_P_Mem_DoneCallBack_isr only sets the
 * internal record of the status, the ideal is that the user will check the
 * Tran's status in a waiting loop and process the Tran result synchronously.
 */
BERR_Code BDMA_P_Mem_Tran_Start_isr(
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
		goto BDMA_P_Done_Mem_Tran_Start;
	}

	eResult = BDMA_P_Mem_GetMemDmaPtrFromTran_isr( pTran, &pMemDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Tran_Start );

	eResult = BDMA_P_Mem_Tran_PrepareStart_isr( pTran, pMemDma->hQueue,
		BDMA_P_GetMemoryHandle(pMemDma->hDma), ulNumActBlocks, &bSgOpen );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Tran_Start );

	eResult = BDMA_P_Queue_StartTran_isr (
		pMemDma->hQueue, pTran, ulNumActBlocks, bSgOpen,
		(BDMA_P_CallbackFunc) pCallBackFunc_isr, pUserCallBackParam );

  BDMA_P_Done_Mem_Tran_Start:
	return BERR_TRACE(eResult);
}


#endif

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_SetByteSwapMode_isr(
	BDMA_Mem_Handle          hMemDma,
	BDMA_Endian              eReadEndian,
	BDMA_SwapMode            eSwapMode )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;

	BDMA_P_MEM_GET_CONTEXT(hMemDma, pMemDma);
	if ( (NULL == pMemDma) ||
		 (eReadEndian >= BDMA_Endian_eInvalid) ||
		 (eSwapMode >= BDMA_SwapMode_eInvalid) )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_SetByteSwapMode;
	}

	pMemDma->eReadEndian = eReadEndian;
	pMemDma->eSwapMode = eSwapMode;

  BDMA_P_Done_Mem_SetByteSwapMode:
	return eResult;
}

/***************************************************************************
 * To be called to created a new Tran
 */
BERR_Code BDMA_P_Mem_Tran_Create_isr(
	BDMA_Mem_Handle          hMemDma,
	uint32_t                 ulNumBlocks,
	bool                     bCachedDesc,
	BDMA_Mem_Tran_Handle  *  phTran )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;
	BDMA_P_QueueEntry * pTran;

	if ( NULL != phTran )
		*phTran = NULL;

	BDMA_P_MEM_GET_CONTEXT( hMemDma, pMemDma );
	if ( (NULL == pMemDma) || (NULL == phTran) )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Tran_Create;
	}

	eResult = BDMA_P_Queue_CreateTran_isr(
		pMemDma->hQueue, ulNumBlocks, bCachedDesc, &pTran );
	if ( BERR_SUCCESS == eResult )
		*phTran = (BDMA_Mem_Tran_Handle ) pTran;

  BDMA_P_Done_Mem_Tran_Create:
	return BERR_TRACE(eResult);
}






/***************************************************************************
 * To be called by BDMA_Mem_Tran_GetStatus. It outputs the internal records
 * of the Tran directly.
 */
BERR_Code BDMA_P_Mem_Tran_GetStatus_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_TranStatus *        peTranStatus )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;
	BDMA_P_QueueEntry * pTran;

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Tran_GetStatus;
	}

	eResult = BDMA_P_Mem_GetMemDmaPtrFromTran_isr( pTran, &pMemDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Tran_GetStatus );

	/* read status from HW and update Trans' records */
	BDMA_P_Queue_CheckHwAndSendNewReq_isr( pMemDma->hQueue );

	*peTranStatus = pTran->eStatus;
	return BERR_SUCCESS;

  BDMA_P_Done_Mem_Tran_GetStatus:
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * To be called by BDMA_Mem_Tran_Destroy. It free the queue entries occupied
 * this Tran
 */
BERR_Code BDMA_P_Mem_Tran_Destroy_isr(
	BDMA_Mem_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;
	BDMA_P_QueueEntry * pTran;

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Tran_Destroy;
	}

	eResult = BDMA_P_Mem_GetMemDmaPtrFromTran_isr( pTran, &pMemDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Tran_Destroy );

	/* tell queue module to free the queue entries of this */
	eResult = BDMA_P_Queue_DestroyTran_isr ( pMemDma->hQueue,
										 pTran );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Tran_Destroy );

	return BERR_SUCCESS;

  BDMA_P_Done_Mem_Tran_Destroy:
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * To be called by BDMA_Mem_Tran_Reset. It resets the Tran into its
 * initially created state
 */
BERR_Code BDMA_P_Mem_Tran_Reset_isr(
	BDMA_Mem_Tran_Handle     hTran )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Mem_Context *  pMemDma;
	BDMA_P_QueueEntry * pTran;

	BDMA_P_TRAN_GET_CONTEXT( hTran, pTran );
	if ( NULL == pTran )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Mem_Tran_Reset;
	}

	eResult = BDMA_P_Mem_GetMemDmaPtrFromTran_isr( pTran, &pMemDma );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Tran_Reset );

	/* tell queue module to reset this Tran into initially created state */
	eResult = BDMA_P_Queue_ResetTran_isr ( pMemDma->hQueue,
									   pTran );
	BDMA_P_END_IF_ERR( eResult, BDMA_P_Done_Mem_Tran_Reset );

	return BERR_SUCCESS;

  BDMA_P_Done_Mem_Tran_Reset:
	return BERR_TRACE(eResult);
}

/* End of File */
