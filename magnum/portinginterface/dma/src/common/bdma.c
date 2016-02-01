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
#include "bdma.h"             /*  */
#include "bdma_priv.h"        /*  */
#include "bkni.h"

BDBG_MODULE(BDMA);


/***************************************************************************
 *
 */
BERR_Code BDMA_Open(
	BDMA_Handle *         phDma,
	BCHP_Handle           hChip,
	BREG_Handle           hRegister,
	BMEM_Handle           hMemory,
	BINT_Handle           hInterrupt )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Context *  pDma = NULL;
	BKNI_MutexHandle hMutex = NULL;

	if ( (NULL == phDma) ||
		 (0    == hRegister) ||
		 (NULL == hMemory) ) 
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Err_Main_Open;		
	}
	
	pDma = (BDMA_P_Context *)BKNI_Malloc( sizeof(BDMA_P_Context) );
	if ( NULL == pDma )
	{
		eResult = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto BDMA_P_Err_Main_Open;
	}
	BKNI_Memset((void*)pDma, 0x0, sizeof(BDMA_P_Context));

	eResult = BKNI_CreateMutex(&hMutex);
	if ((BERR_SUCCESS != eResult) || (NULL == hMutex))
	{
#if BDBG_DEBUG_BUILD == 1	
		BERR_TRACE(eResult);
#endif		
		goto BDMA_P_Err_Main_Open;		
	}
	
	pDma->hChip = hChip;
	pDma->hRegister = hRegister;
	pDma->hMemory = hMemory;
	pDma->hInterrupt = hInterrupt;
	pDma->hMutex = hMutex;
	/* hMemDma = NULL; */
	/* hPciDma = NULL; */
	BDMA_P_MAIN_SET_BLACK_MAGIC(pDma);

	*phDma = pDma;
	return eResult;

  BDMA_P_Err_Main_Open:
	if ( NULL != pDma )
		BDMA_P_MAIN_DESTROY_CONTEXT(pDma);
	
	if ( NULL != phDma )
		*phDma = NULL;
		
	return eResult;
}

/***************************************************************************
 *
 */
BERR_Code BDMA_Close(
	BDMA_Handle           hDma )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDMA_P_Context *  pDma;
#if BDMA_P_SUPPORT_MEM_DMA_ENGINE
	BDMA_MemDmaEngine  eMem;
#endif

#if BDMA_P_SUPPORT_PCI_DMA_ENGINE
	BDMA_PciDmaEngine  ePci;
#endif
#if BDMA_P_SUPPORT_SHARF_DMA_ENGINE
	BDMA_Sharf         eSharf;
#endif

	BDMA_P_MAIN_GET_CONTEXT( hDma, pDma );
	if ( NULL == pDma )
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BDMA_P_Done_Main_Close;		
	}

	/* sub-modules have to be explicitly destroied first */
#if BDMA_P_SUPPORT_MEM_DMA_ENGINE	
	for (eMem = BDMA_MemDmaEngine_e0; eMem < BDMA_P_SUPPORT_MEM_DMA_ENGINE; eMem ++)
	{
		if (NULL != pDma->hMemDma[eMem])
		{
			eResult = BERR_TRACE(BERR_LEAKED_RESOURCE);
			goto BDMA_P_Done_Main_Close;		
		}
	}
#endif



#if BDMA_P_SUPPORT_PCI_DMA_ENGINE
	for (ePci = BDMA_PciDmaEngine_e0; ePci < BDMA_P_SUPPORT_PCI_DMA_ENGINE; ePci ++)
	{
		if (NULL != pDma->hPciDma[ePci])
		{
			eResult = BERR_TRACE(BERR_LEAKED_RESOURCE);
			goto BDMA_P_Done_Main_Close;		
		}
	}
#endif


#if BDMA_P_SUPPORT_SHARF_DMA_ENGINE
		for (eSharf = BDMA_Sharf_e0; eSharf < BDMA_Sharf_eInvalid; eSharf ++)
		{
			if (NULL != pDma->hSharfDma[eSharf])
		{
			eResult = BERR_TRACE(BERR_LEAKED_RESOURCE);
			goto BDMA_P_Done_Main_Close;		
		}
	}
#endif

	if (pDma->hMutex)
	{
		BKNI_DestroyMutex(pDma->hMutex);
	}
		
	BDMA_P_MAIN_DESTROY_CONTEXT(pDma);

  BDMA_P_Done_Main_Close:
	return eResult;	
}

/* End of File */
