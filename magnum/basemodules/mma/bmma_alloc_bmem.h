/***************************************************************************
 *     Copyright (c) 2012-2013, Broadcom Corporation
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
#ifndef BMMA_ALLOC_BMEM_H__
#define BMMA_ALLOC_BMEM_H__

#include "bmma.h"

#ifdef __cplusplus
extern "C" {
#endif

void BMMA_Alloc_SetTaint(BMMA_Block_Handle block);
BMMA_Block_Handle BMMA_Alloc_GetTaintByAddress(BMMA_Heap_Handle heap, void *addr);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* #ifndef BMMA_ALLOC_BMEM_H__ */

/* End of File */
