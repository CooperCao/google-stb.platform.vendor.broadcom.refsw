/***************************************************************************
 *    Copyright (c) 2004-2013, Broadcom Corporation
 *    All Rights Reserved
 *    Confidential Property of Broadcom Corporation
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
 *	 This module contains the code for the XVD memory sub-manager.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ****************************************************************************/
#include "bxvd_memory_priv.h"
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bxvd_platform.h"
#include "bxvd_priv.h"

/* To print out allocation define the following.
#define BXVD_P_DBG_MEM 1
 */

BDBG_MODULE(BXVD_MEMORY);

/***************************************************************************
BXVD_P_Memory_Open: Opens and initializes an XVD memory instance
******************
********************************************************/
BERR_Code BXVD_P_Memory_Open(BXVD_Handle hXvd,
                             BXVD_P_MemoryHandle *phXvdMem,
                             void *pvAddr,
                             uint32_t uiSize,
                             BXVD_P_Memory_Protection eProtect)
{
   BERR_Code rc = BERR_SUCCESS;
   BXVD_P_Memory *pMem = (BXVD_P_Memory *)NULL;

   BMMA_RangeAllocator_CreateSettings RangeAllocatorSettings;

   BXVD_P_Memory_RegionIndex region;

   BDBG_ENTER(BXVD_P_Memory_Open);
   BDBG_ASSERT(hXvd);
   BDBG_ASSERT(phXvdMem);
   BDBG_ASSERT(pvAddr);

   if (eProtect != BXVD_P_Memory_Protection_eProtect &&
       eProtect != BXVD_P_Memory_Protection_eDontProtect)
   {
      BXVD_DBG_ERR(hXvd, ("Invalid memory protection parameter"));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Set handle to NULL in case the allocation fails */
   *phXvdMem = NULL;

   pMem = (BXVD_P_Memory*)(BKNI_Malloc(sizeof(BXVD_P_Memory)));
   if (!pMem)
   {
      BXVD_DBG_ERR(hXvd, ("Could not allocate context structure memory"));
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   /* Zero out the newly allocated context */
   BKNI_Memset((void*)pMem, BXVD_P_MEM_ZERO, sizeof(BXVD_P_Memory));

   pMem->hXvd = hXvd;
   pMem->uiBlockSize = uiSize;
   pMem->uiRegionCount = 0;
   pMem->uiBlockBaseAddr = pvAddr;

   /* Set the free flag to true for all the new, unallocated regions */
   for (region = 0; region < BXVD_P_MEMORY_MAX_ALLOCATIONS; region++)
   {
      (pMem->aAllocatedRegions[region]).bRegionFree = true;
      (pMem->aAllocatedRegions[region]).iRegionIndex =
         BXVD_P_MEMORY_FREE_REGION_INDEX;
   }

   BMMA_RangeAllocator_GetDefaultCreateSettings(&RangeAllocatorSettings);

   RangeAllocatorSettings.base = (unsigned long) pvAddr;
   RangeAllocatorSettings.size = uiSize;

   if ((rc = BMMA_RangeAllocator_Create(&(pMem->hBMMA_RangeAllocator), &RangeAllocatorSettings )) != BERR_SUCCESS)
   {
      BXVD_DBG_ERR(hXvd, ("Could not create BMMA_RangeAllocator instance"));
      BKNI_Free(pMem);
      return BERR_TRACE(rc);
   }

   *phXvdMem = (BXVD_P_MemoryHandle)pMem;

   pMem->uiFreeSpace = uiSize;

#if BXVD_P_DBG_MEM
   BDBG_ERR(("\tMemory_Handle_Open: size %d\n", uiSize));
#endif

   BDBG_LEAVE(BXVD_P_Memory_Open);
   return BERR_TRACE(rc);
}

/***************************************************************************
BXVD_P_Memory_Close: Closes an XVD memory instance
**************************************************************************/
BERR_Code BXVD_P_Memory_Close(BXVD_P_MemoryHandle hXvdMem)
{
   BDBG_ENTER(BXVD_P_Memory_Close);

   BDBG_ASSERT(hXvdMem);

   BMMA_RangeAllocator_Destroy(hXvdMem->hBMMA_RangeAllocator);

   /* Release the memory context */
   BKNI_Free(hXvdMem);

   /* Null the handle */
   hXvdMem = NULL;

   BDBG_LEAVE(BXVD_P_Memory_Close);
   return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
BXVD_P_Memory_Allocate: Allocate a region of sub-managed memory
**************************************************************************/
void *BXVD_P_Memory_Allocate(BXVD_P_MemoryHandle hXvdMem,
                             uint32_t uiAllocationSize,
                             uint32_t uiAlignment,
                             uint32_t uiRegionTag)
{
   bool bAlloc, bRegionFound;

   unsigned long ulAddr;
   int32_t iIndex;

   BXVD_P_AllocatedRegion stAllocReg;

   BMMA_RangeAllocator_BlockSettings stRA_BlockSettings;

   BDBG_ENTER(BXVD_P_Memory_Allocate);
   BDBG_ASSERT(hXvdMem);

#if BXVD_P_DBG_MEM
   BDBG_ERR(("\t\tMemAlloc FreeSpace: %0x (%d) size %d align: %d \n", hXvdMem->uiFreeSpace, hXvdMem->uiFreeSpace, uiAllocationSize, uiAlignment));
#endif

   /*
    * Make sure we don't try to allocate > than BXVD_P_MEMORY_MAX_ALLOCATIONS
    * since that is the maximum number we can track. The default for maximum
    * is currently 17 regions but this can easily be changed.
    */
   if (((hXvdMem->uiRegionCount) + 1) > BXVD_P_MEMORY_MAX_ALLOCATIONS)
   {
      BDBG_ERR(("No more available regions. Maximum is %d",
                BXVD_P_MEMORY_MAX_ALLOCATIONS));
      return NULL;
   }

   BMMA_RangeAllocator_GetDefaultAllocationSettings( &stRA_BlockSettings );

   stRA_BlockSettings.alignment = 1 << uiAlignment;

   if ( BMMA_RangeAllocator_Alloc( hXvdMem->hBMMA_RangeAllocator, &stAllocReg.hBMMA_RangeAllocBlock,
                                   uiAllocationSize, &stRA_BlockSettings ) !=  BERR_SUCCESS )
   {
      BDBG_ERR(("Could not allocate requested memory block"));
      return NULL;
   }

   ulAddr = BMMA_RangeAllocator_GetAllocationBase( stAllocReg.hBMMA_RangeAllocBlock );

   /*
    * Store bookkeeping information in the allocated regions array.
    */

   bAlloc = false;
   bRegionFound = false;
   stAllocReg.bRegionFree = false;

   /* Find the first unallocated region slot */
   for (iIndex = 0; iIndex < BXVD_P_MEMORY_MAX_ALLOCATIONS-1; iIndex++)
   {
      BXVD_P_Memory_IsRegionAllocated(hXvdMem, iIndex, &bAlloc);
      if (bAlloc == false)
      {
	 BDBG_MSG(("<<<< found a free allocation slot at %d >>>>", iIndex));
	 bRegionFound = true;
         break;
      }
   }

   /* Make sure we actually found a free allocation slot */
   if (bRegionFound == false)
   {
      BDBG_ERR(("No allocation region slots available"));
      return NULL;
   }

   /* Initialize allocation region members before copy */
   stAllocReg.iRegionIndex = (BXVD_P_Memory_RegionIndex)iIndex;
   stAllocReg.uiRegionAlignment = uiAlignment;
   stAllocReg.pvRegionAddr = (void *)ulAddr;
   stAllocReg.uiRegionSize = uiAllocationSize;
   stAllocReg.uiRegionTag = uiRegionTag;

   /* Move it from found slot to allocated regions array */
   BKNI_Memcpy(&(hXvdMem->aAllocatedRegions[iIndex]),
               &stAllocReg,
               sizeof(BXVD_P_AllocatedRegion));

   hXvdMem->uiFreeSpace -= uiAllocationSize;
   /* Increment the region count */
   (hXvdMem->uiRegionCount)++;

   BDBG_LEAVE(BXVD_P_Memory_Allocate);

   return (void *) ulAddr;
}

/***************************************************************************
BXVD_P_Memory_Free: Free a region of memory by address
**************************************************************************/
BERR_Code BXVD_P_Memory_Free(BXVD_P_MemoryHandle hXvdMem,
                             void *pvAddr)
{
   BERR_Code rc = BERR_INVALID_PARAMETER;
   BXVD_P_Memory_RegionIndex region;

   BDBG_ENTER(BXVD_P_Memory_Free);
   BDBG_ASSERT(hXvdMem);
   BDBG_ASSERT(pvAddr);


   /*
    * Look up the region about to be freed and do some house keeping.
    */
   for (region = 0; region < BXVD_P_MEMORY_MAX_ALLOCATIONS; region++)
   {
      if (hXvdMem->aAllocatedRegions[region].pvRegionAddr == pvAddr)
      {
         hXvdMem->uiFreeSpace += hXvdMem->aAllocatedRegions[region].uiRegionSize;

         hXvdMem->aAllocatedRegions[region].bRegionFree = true;
         hXvdMem->aAllocatedRegions[region].iRegionIndex = BXVD_P_MEMORY_FREE_REGION_INDEX;
         hXvdMem->aAllocatedRegions[region].pvRegionAddr = NULL;
         hXvdMem->aAllocatedRegions[region].uiRegionSize = 0;
         BMMA_RangeAllocator_Free(hXvdMem->hBMMA_RangeAllocator, hXvdMem->aAllocatedRegions[region].hBMMA_RangeAllocBlock);
         (hXvdMem->uiRegionCount)--;
         rc = BERR_SUCCESS;
         break;
      }
   }
#if BXVD_P_DBG_MEM
   BDBG_ERR(("\t\tMemFree, FreeSpace: %0x\n", hXvdMem->uiFreeSpace));
#endif
   BDBG_LEAVE(BXVD_P_Memory_Free);
   return BERR_TRACE(rc);
}

/***************************************************************************
BXVD_P_Memory_IsRegionAllocated: Return a region's allocation status given
its region index.
****************************************************************************/
BERR_Code BXVD_P_Memory_IsRegionAllocated(BXVD_P_MemoryHandle hXvdMem,
                                          BXVD_P_Memory_RegionIndex regionIndex,
                                          bool *pbAllocated)
{
   BDBG_ENTER(BXVD_P_Memory_IsRegionAllocated);

   BDBG_ASSERT(hXvdMem);
   BDBG_ASSERT(pbAllocated);

   if (regionIndex < 0 || regionIndex > BXVD_P_MEMORY_MAX_ALLOCATIONS-1)
   {
      *pbAllocated = false;
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   if ((hXvdMem->aAllocatedRegions[regionIndex].bRegionFree) == true)
      *pbAllocated = false;
   else
      *pbAllocated = true;

   BDBG_LEAVE(BXVD_P_Memory_IsRegionAllocated);
   return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
BXVD_P_Memory_DumpRegion: Diagnostic dump of memory regions
**************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
void BXVD_P_Memory_Dump(BXVD_P_MemoryHandle hXvdMem,
                        BXVD_P_Memory_DumpFlag eDumpFlag)
{
   BXVD_P_Memory_RegionIndex region;

   BDBG_ENTER(BXVD_P_Memory_Dump);
   BDBG_ASSERT(hXvdMem);

   BKNI_Printf("--------------------------------------------------\n");
   if (hXvdMem->uiRegionCount > 1)
      BKNI_Printf("- 0x%2.2x (%2.2d) regions have been allocated          -\n",
                  hXvdMem->uiRegionCount, hXvdMem->uiRegionCount);
   else
      BKNI_Printf("- One region has been allocated                  -\n");
   BKNI_Printf("--------------------------------------------------\n");

   for (region = 0; region < BXVD_P_MEMORY_MAX_ALLOCATIONS; region++)
   {
      if (eDumpFlag == BXVD_P_Memory_DumpFlag_eAllocatedOnly &&
          hXvdMem->aAllocatedRegions[region].bRegionFree == true)
         continue;
      BKNI_Printf("Allocated/Free flag: %s\n",
                  (char *)(hXvdMem->aAllocatedRegions[region].bRegionFree ==
                           true ? "free" : "allocated"));
      BKNI_Printf("         Region tag: 0x%x (%d)\n",
                  hXvdMem->aAllocatedRegions[region].uiRegionTag,
                  hXvdMem->aAllocatedRegions[region].uiRegionTag);
      BKNI_Printf("       Region index: %d\n",
                  hXvdMem->aAllocatedRegions[region].iRegionIndex);
      BKNI_Printf("   Region alignment: %d\n",
                  hXvdMem->aAllocatedRegions[region].uiRegionAlignment);

      BKNI_Printf("   Region address: 0x%0*lx\n",
                  BXVD_P_DIGITS_IN_LONG, (long)hXvdMem->aAllocatedRegions[region].pvRegionAddr);

      BKNI_Printf("        Region size: 0x%x (%d)\n",
                  hXvdMem->aAllocatedRegions[region].uiRegionSize,
                  hXvdMem->aAllocatedRegions[region].uiRegionSize);
      BKNI_Printf("--------------------------------------------------\n");
   }

   BDBG_LEAVE(BXVD_P_Memory_Dump);
   return;
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */
