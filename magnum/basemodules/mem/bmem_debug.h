/***************************************************************************
 *     Copyright (c) 2001-2010, Broadcom Corporation
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
#ifndef BMEM_DEBUG_H__
#define BMEM_DEBUG_H__

#include "bmem.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * These functions print out various info from the heap for debugging
 * purposes.
 */
void BMEM_Dbg_Map(BMEM_Handle pheap);
void BMEM_Dbg_DumpHeap(BMEM_Handle pheap);

void BMEM_Dbg_ClearErrorCount(BMEM_Handle pheap);
uint32_t BMEM_Dbg_GetErrorCount(BMEM_Handle pheap);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* #ifndef BMEM_DEBUG_H__ */

/* End of File */

