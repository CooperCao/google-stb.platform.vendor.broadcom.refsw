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

#include "bdma_priv.h"    /*  */

BDBG_MODULE(BDMA);


/***************************************************************************
 * Note: assume parameters are valid
 */
BCHP_Handle BDMA_P_GetChipHandle(
	BDMA_Handle  hDma )
{
	BDMA_P_Context *     pDma;
	
	BDMA_P_MAIN_GET_CONTEXT(hDma, pDma);
	BDBG_ASSERT( NULL != pDma );

	return pDma->hChip;
}

/***************************************************************************
 * Note: assume parameters are valid
 */
BREG_Handle BDMA_P_GetRegisterHandle(
	BDMA_Handle  hDma )
{
	BDMA_P_Context *     pDma;
	
	BDMA_P_MAIN_GET_CONTEXT(hDma, pDma);
	BDBG_ASSERT( NULL != pDma );

	return pDma->hRegister;
}

/***************************************************************************
 * Note: assume parameters are valid
 */
BMEM_Handle BDMA_P_GetMemoryHandle(
	BDMA_Handle  hDma )
{
	BDMA_P_Context *     pDma;
	
	BDMA_P_MAIN_GET_CONTEXT(hDma, pDma);
	BDBG_ASSERT( NULL != pDma );

	return pDma->hMemory;
}

/***************************************************************************
 * Note: assume parameters are valid
 */
BINT_Handle BDMA_P_GetInterruptHandle(
	BDMA_Handle  hDma )
{
	BDMA_P_Context *     pDma;
	
	BDMA_P_MAIN_GET_CONTEXT(hDma, pDma);
	BDBG_ASSERT( NULL != pDma );

	return pDma->hInterrupt;
}

#if BDMA_P_SUPPORT_MEM_DMA_ENGINE
/***************************************************************************
 * Note: assume parameters are valid
 */
void BDMA_P_SetMemDmaHandle(
	BDMA_Handle       hDma,
	BDMA_MemDmaEngine eEngine,
	BDMA_Mem_Handle   hMemDma )
{
	BDMA_P_Context *     pDma;
	
	BDMA_P_MAIN_GET_CONTEXT(hDma, pDma);
	BDBG_ASSERT( NULL != pDma );

	if (eEngine < BDMA_P_SUPPORT_MEM_DMA_ENGINE)
		pDma->hMemDma[eEngine] = hMemDma;  /* hMemDma will be NULL when destroy */
	else
		BDBG_ASSERT(eEngine < BDMA_P_SUPPORT_MEM_DMA_ENGINE);
}

/***************************************************************************
 * Note: assume parameters are valid
 */
BDMA_Mem_Handle BDMA_P_GetMemDmaHandle(
	BDMA_Handle        hDma,
	BDMA_MemDmaEngine eEngine )
{
	BDMA_P_Context *     pDma;
	
	BDMA_P_MAIN_GET_CONTEXT(hDma, pDma);
	BDBG_ASSERT( NULL != pDma );

	if (eEngine < BDMA_P_SUPPORT_MEM_DMA_ENGINE)
		return pDma->hMemDma[eEngine];  /* hMemDma will be NULL when destroy */
	else
	{
		BDBG_ASSERT(eEngine < BDMA_P_SUPPORT_MEM_DMA_ENGINE);
		return NULL;
	}
}
#endif

#if BDMA_P_SUPPORT_PCI_DMA_ENGINE
/***************************************************************************
 * Note: assume parameters are valid
 */
void BDMA_P_SetPciDmaHandle(
	BDMA_Handle       hDma,
	BDMA_PciDmaEngine eEngine,
	BDMA_Pci_Handle   hPciDma )
{
	BDMA_P_Context *     pDma;
	
	BDMA_P_MAIN_GET_CONTEXT(hDma, pDma);
	BDBG_ASSERT( NULL != pDma );

	if (eEngine < BDMA_P_SUPPORT_PCI_DMA_ENGINE)
		pDma->hPciDma[eEngine] = hPciDma;  /* hPciDma will be NULL when destroy */
	else
		BDBG_ASSERT(eEngine < BDMA_P_SUPPORT_PCI_DMA_ENGINE);
}

/***************************************************************************
 * Note: assume parameters are valid
 */
BDMA_Pci_Handle BDMA_P_GetPciDmaHandle(
	BDMA_Handle        hDma,
	BDMA_PciDmaEngine eEngine )
{
	BDMA_P_Context *     pDma;
	
	BDMA_P_MAIN_GET_CONTEXT(hDma, pDma);
	BDBG_ASSERT( NULL != pDma );

	if (eEngine < BDMA_P_SUPPORT_PCI_DMA_ENGINE)
		return pDma->hPciDma[eEngine];  /* hPciDma will be NULL when destroy */
	else
	{
		BDBG_ASSERT(eEngine < BDMA_P_SUPPORT_PCI_DMA_ENGINE);
		return NULL;
	}
}
#endif

#if BDMA_P_SUPPORT_SHARF_DMA_ENGINE
/***************************************************************************
 * Note: assume parameters are valid. It should be in bdma_priv.c, but we
 * put it here because we want to export sharf concept to outside as less
 * as possible.
 */
void BDMA_P_SetSharfDmaHandle(
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
BDMA_Mem_Handle BDMA_P_GetSharfDmaHandle(
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
#endif
/* End of File */
