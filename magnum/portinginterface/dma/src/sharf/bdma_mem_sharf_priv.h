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
#ifndef BDMA_MEM_SHARF_PRIV_H__
#define BDMA_MEM_SHARF_PRIV_H__

#include "../common/bdma_priv.h"
#include "../common/bdma_queue.h"
#include "bdma_sharf.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
 * {private}

 * Implementation Overview:
 *
 ****************************************************************************/
#define BDMA_P_SHARF_GET_CONTEXT(handle, context) \
		BDMA_P_GENERIC_GET_CONTEXT((handle), (context), BDMA_P_Mem_Context)
	
#define BDMA_P_SHARF_SET_BLACK_MAGIC(handle) \
		BDMA_P_GENERIC_SET_BLACK_MAGIC((handle), BDMA_P_Mem_Context)
	
#define BDMA_P_SHARF_DESTROY_CONTEXT(struct_ptr) \
		BDMA_P_GENERIC_DESTROY_CONTEXT(struct_ptr, BDMA_P_Mem_Context)


#define BDMA_P_MEM_SHARF_DESTROY_CONTEXT(struct_ptr) \
	BDMA_P_GENERIC_DESTROY_CONTEXT(struct_ptr, BDMA_P_Mem_Sharf_Context)


	
/***************************************************************************
 * Sharf specific info for memory ama channel context
 */
typedef struct BDMA_P_Mem_Sharf_Context
{
	BDMA_Sharf                   eEngine;
	
} BDMA_P_Mem_Sharf_Context;

typedef struct BDMA_P_Mem_Sharf_Context *BDMA_P_Mem_Sharf_Handle;
	

/***************************************************************************
 *
 * API support functions
 *
 ***************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Sharf_Create(
	BDMA_Handle           hDma,
	BDMA_Sharf            eEngine,
	const BDMA_Mem_Settings *pSettings,	
	BDMA_Mem_Handle *     phMemDma );

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Sharf_Destroy(
	BDMA_Mem_Handle       hMemDma );

/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Sharf_Tran_SetCrypto_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_SharfMode           eSharfMode,
	bool                     bSgEnable,
	bool                     bUseBspKey,
	bool                     bCmp8LstByts);
	
/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Sharf_Tran_SetDmaBlockInfo_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	BDMA_Context             eContext,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize );
	
/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Sharf_Tran_SetSgStartEnd_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	bool                     bStartSgScram,
	bool                     bEndSgScram,
	bool                     bKeyPresent,
	bool                     bDigestPresent );	

/***************************************************************************
 *
 */


void BDMA_P_Sharf_SafeSendToHw_isr(
										BDMA_P_QueueHandle      hQueue,
										BDMA_P_HwStatus*        peHwStatus
										);


/***************************************************************************
 *
 */

void BDMA_P_Sharf_ReadHwRegs_isr(
	void  *               pvEngine,
	BDMA_P_HwStatus  *    peHwStatus,
	uint32_t  *           pulCurrDesc,
	uint32_t  *           pulScratchReg );

/***************************************************************************
 *
 */

void BDMA_P_Sharf_AppendTranDesc_isr(
	BDMA_P_QueueEntry *      pCurrTran,
	BDMA_P_QueueEntry *      pNextTran );

/***************************************************************************
 *
 */

void BDMA_P_Sharf_SendToHw_isr(
	BDMA_P_QueueHandle       hQueue,
	BDMA_P_HwStatus          eHwStatus,
	bool                     bAppendInHw );


/***************************************************************************
 *
 */

void BDMA_P_Sharf_CheckNextDesc_isr(
	BDMA_P_QueueEntry *      pCurrTran,
	BDMA_P_QueueEntry *      pNextTran );


/***************************************************************************
 *
 */

void BDMA_P_Sharf_DoneCallBack_isr(
	void *                pParm1,  /* pMemDma */
	int                   parm2 );  /* not used */


/***************************************************************************
 *
 */
BERR_Code BDMA_P_Mem_Sharf_Tran_Start_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,
	BDMA_Mem_CallbackFunc    pCallBackFunc_isr,
	void *                   pUserCallBackParam );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BDMA_MEM_SHARF_PRIV_H__ */

/* end of file */
