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
#ifndef BMMA_BMEM_H__
#define BMMA_BMEM_H__

#include "bmem.h"
#include "bmma.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=Module Overview: ********************************************************
The purpose of this module is to provide BMEM compatible API to allocate memory
****************************************************************************/

/***************************************************************************
Summary:
    Settings structure to create BMEM heap
See Also:
    BMMA_Bmem_Attach
****************************************************************************/
typedef struct BMMA_Bmem_Settings {
    BMMA_DeviceOffset base;
    size_t length;
    void *cached;
    void *uncached;
    void (*flush_cache)(const void *addr, size_t size);
    bool dummy; /* if set then this is just a dummy heap, which would fail all allocations */
} BMMA_Bmem_Settings;

/***************************************************************************
Summary:
    Fills in BMMA_Bmem_Settings structure with default settings.
****************************************************************************/
void BMMA_Bmem_GetDefaultSettings(BMMA_Bmem_Settings *settings);

/***************************************************************************
Summary:
    Creates BMEM_Handle instance  for existing heap
****************************************************************************/
BERR_Code BMMA_Bmem_Attach(BMMA_Heap_Handle heap, BMEM_ModuleHandle parent, BMEM_Handle *bmemHeap, const BMMA_Bmem_Settings *settings);

/***************************************************************************
Summary:
    Releases resources used by the BMEM_Handle
****************************************************************************/
void BMMA_Bmem_Detach(BMEM_Handle bmemHeap);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* #ifndef BMMA_BMEM_H__ */

/* End of File */
