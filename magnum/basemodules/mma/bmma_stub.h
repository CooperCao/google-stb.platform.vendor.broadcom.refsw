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
#ifndef BMMA_STUB_H__
#define BMMA_STUB_H__

#ifndef BMMA_STUB_BMEM_ALIAS
#define BMMA_STUB_BMEM_ALIAS    1
#endif

#if BMMA_STUB_BMEM_ALIAS
#include "bmem.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
#if BMMA_STUB_BMEM_ALIAS
typedef BMEM_Heap_Handle BMMA_Heap_Handle;
#define BMMA_Heap_Handle BMMA_Heap_Handle
#else

#endif

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* #ifndef BMMA_STUB_H__ */
