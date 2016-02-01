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
#ifndef BMMA_POOL_H__
#define BMMA_POOL_H__

#include "bmma_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=Module Overview: ********************************************************
The purpose of this module is to provide API to efficiently allocate
large number of the same size memory blocks from the OS heap.

And efficiency is attained by allocating memory blocks in larger chunks
and then subdivide them internally to fixed size sub allocations.
****************************************************************************/

/***************************************************************************
Summary:
    A handle representing one instance of pool memory allocator
****************************************************************************/
typedef struct BMMA_PoolAllocator *BMMA_PoolAllocator_Handle;

/***************************************************************************
Summary:
    Settings structure for allocator
****************************************************************************/
typedef struct BMMA_PoolAllocator_CreateSettings {
    size_t allocationSize; /* size of allocation, all sub-allocations would be of this size */
    size_t maxBlockSize; /* cap to the block size, in bytes */
} BMMA_PoolAllocator_CreateSettings;

/***************************************************************************
Summary:
    Initializes structure fields with default valued
****************************************************************************/
void BMMA_PoolAllocator_GetDefaultCreateSettings(BMMA_PoolAllocator_CreateSettings *settings);

/***************************************************************************
Summary:
    Create new instance of allocator
****************************************************************************/
#if BKNI_TRACK_MALLOCS
BERR_Code BMMA_PoolAllocator_Create_tagged(BMMA_PoolAllocator_Handle *allocator, const BMMA_PoolAllocator_CreateSettings *settings, const char *file, unsigned line);
#define BMMA_PoolAllocator_Create(allocator, settings) BMMA_PoolAllocator_Create_tagged(allocator, settings, BSTD_FILE, BSTD_LINE)
#else
BERR_Code BMMA_PoolAllocator_Create(BMMA_PoolAllocator_Handle *allocator, const BMMA_PoolAllocator_CreateSettings *settings);
#endif
/***************************************************************************
Summary:
    Destroys instance of allocator
****************************************************************************/
void BMMA_PoolAllocator_Destroy(BMMA_PoolAllocator_Handle allocator);
/***************************************************************************
Summary:
    Allocated memory of previously specified size
****************************************************************************/
void *BMMA_PoolAllocator_Alloc(BMMA_PoolAllocator_Handle allocator);
/***************************************************************************
Summary:
    Frees previously allocated memory
****************************************************************************/
void BMMA_PoolAllocator_Free(BMMA_PoolAllocator_Handle allocator, void *block);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* #ifndef BMMA_POOL_H__ */

/* End of File */
