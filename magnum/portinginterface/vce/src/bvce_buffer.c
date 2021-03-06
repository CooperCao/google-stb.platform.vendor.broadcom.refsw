/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bvce_buffer.h"
#include "bmma_range.h"
#include "bdbg.h"

BDBG_MODULE(BVCE_BUFFER);

/* Contexts */
typedef struct BVCE_P_Allocator
{
   BMMA_Heap_Handle hHeap;
   BMMA_RangeAllocator_Handle hRange;
   BVCE_P_Buffer_AllocSettings stSettings;
   BMMA_Block_Handle hBlock;
   BMMA_DeviceOffset uiDeviceOffset;
   void *pBufferBase;
   unsigned uiBlockRefCount;
} BVCE_P_Allocator;

typedef struct BVCE_P_Buffer
{
   BVCE_P_Allocator_Handle hAllocator;

   struct
   {
      BMMA_Block_Handle hBlock;
      BMMA_RangeAllocator_Block_Handle hRangeBlock;
   } stBlock;

   BVCE_P_Buffer_AllocSettings stSettings;

   BMMA_DeviceOffset uiOffset;
   bool bOffsetValid;
   void *pBuffer;
   bool bBufferValid;
} BVCE_P_Buffer;

/* Functions */
void
BVCE_P_Buffer_GetDefaultAllocSettings(
   BVCE_P_Buffer_AllocSettings *pstAllocSettings
   )
{
   BDBG_ENTER( BVCE_P_Buffer_GetDefaultAllocSettings );

   BDBG_ASSERT( pstAllocSettings );

   BKNI_Memset( pstAllocSettings, 0, sizeof( BVCE_P_Buffer_AllocSettings ) );

   BDBG_LEAVE( BVCE_P_Buffer_GetDefaultAllocSettings );
}

BERR_Code
BVCE_P_Buffer_Alloc(
   BVCE_P_Allocator_Handle hAllocator,
   const BVCE_P_Buffer_AllocSettings *pstAllocSettings,
   BVCE_P_Buffer_Handle *phBuffer
   )
{
   BVCE_P_Buffer_Handle hBuffer = NULL;

   BDBG_ENTER( BVCE_P_Buffer_Alloc );
   BDBG_ASSERT( hAllocator );
   BDBG_ASSERT( pstAllocSettings );
   BDBG_ASSERT( 0 != pstAllocSettings->uiSize );
   BDBG_ASSERT( phBuffer );

   hBuffer = (BVCE_P_Buffer_Handle) BKNI_Malloc( sizeof( BVCE_P_Buffer ) );

   if ( NULL == hBuffer )
   {
      BDBG_ERR(("Error allocating Buffer context"));
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }
   BKNI_Memset( hBuffer, 0, sizeof( BVCE_P_Buffer ) );

   hBuffer->hAllocator = hAllocator;
   hBuffer->stSettings = *pstAllocSettings;

   if ( NULL != hBuffer->hAllocator->hRange )
   {
      /* Allocate using BMMA range allocator */
      BERR_Code rc;
      BMMA_RangeAllocator_BlockSettings stRangeAllocatorBlockSettings;

      BMMA_RangeAllocator_GetDefaultAllocationSettings( &stRangeAllocatorBlockSettings );
      stRangeAllocatorBlockSettings.alignment = hBuffer->stSettings.uiAlignment;

      rc = BMMA_RangeAllocator_Alloc(
         hBuffer->hAllocator->hRange,
         &hBuffer->stBlock.hRangeBlock,
         hBuffer->stSettings.uiSize,
         &stRangeAllocatorBlockSettings
         );

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("Error allocating memory range block"));
         BVCE_P_Buffer_Free( hBuffer );
         return BERR_TRACE( rc );
      }
   }
   else
   {
      /* Allocate using BMMA heap */
      hBuffer->stBlock.hBlock = BMMA_Alloc(
         hBuffer->hAllocator->hHeap,
         hBuffer->stSettings.uiSize,
         hBuffer->stSettings.uiAlignment,
         NULL
         );

      if ( NULL == hBuffer->stBlock.hBlock )
      {
         BDBG_ERR(("Error allocating memory block"));
         BVCE_P_Buffer_Free( hBuffer );
         return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
      }
   }

   if ( ( true == hBuffer->stSettings.bLockOffset )
        || ( true == hBuffer->stSettings.bLockAddress ) )
   {
      BVCE_P_Buffer_LockDeviceOffset( hBuffer );
   }

   if ( true == hBuffer->stSettings.bLockAddress )
   {
      void *pBuffer = BVCE_P_Buffer_LockAddress( hBuffer );

      if ( NULL == pBuffer )
      {
         BDBG_ERR(("Error locking virtual address"));
         BVCE_P_Buffer_Free( hBuffer );
         return BERR_TRACE( BERR_UNKNOWN );
      }
   }

   *phBuffer = hBuffer;

   BDBG_LEAVE( BVCE_P_Buffer_Alloc );

   return BERR_TRACE( BERR_SUCCESS );
}

BMMA_Block_Handle
BVCE_P_Buffer_GetBlockHandle(
   BVCE_P_Buffer_Handle hBuffer
   )
{
   BMMA_Block_Handle hBlock = NULL;

   BDBG_ENTER( BVCE_P_Buffer_GetBlock );

   hBlock = hBuffer->stBlock.hBlock;
   if ( NULL == hBlock )
   {
      hBlock = hBuffer->hAllocator->hBlock;
   }

   BDBG_ENTER( BVCE_P_Buffer_GetBlock );

   return hBlock;
}

size_t
BVCE_P_Buffer_GetSize(
   BVCE_P_Buffer_Handle hBuffer
   )
{
   BDBG_ASSERT( hBuffer );

   return hBuffer->stSettings.uiSize;
}

BMMA_DeviceOffset
BVCE_P_Buffer_LockDeviceOffset(
   BVCE_P_Buffer_Handle hBuffer
   )
{
   BDBG_ENTER( BVCE_P_Buffer_LockDeviceOffset );

   BDBG_ASSERT( hBuffer );

   if ( false == hBuffer->bOffsetValid )
   {
      if ( NULL != hBuffer->hAllocator->hRange )
      {
         hBuffer->uiOffset = BMMA_RangeAllocator_GetAllocationBase( hBuffer->stBlock.hRangeBlock );
      }
      else
      {
         hBuffer->uiOffset = BMMA_LockOffset( hBuffer->stBlock.hBlock );
      }
      hBuffer->bOffsetValid = true;
   }

   BDBG_LEAVE( BVCE_P_Buffer_LockDeviceOffset );
   return hBuffer->uiOffset;
}

BMMA_DeviceOffset
BVCE_P_Buffer_GetDeviceOffset_isrsafe(
   BVCE_P_Buffer_Handle hBuffer
   )
{
   BDBG_ASSERT( hBuffer );

   if ( true == hBuffer->bOffsetValid )
   {
      return hBuffer->uiOffset;
   }
   else
   {
      BDBG_ERR(("BVCE_P_Buffer_GetDeviceOffset() called for an unlocked buffer"));
      return 0;
   }
}

unsigned
BVCE_P_Buffer_GetDeviceBaseOffset(
   BVCE_P_Buffer_Handle hBuffer
   )
{
   BDBG_ASSERT( hBuffer );

   if ( 0 != BVCE_P_Allocator_GetDeviceOffset( hBuffer->hAllocator ) )
   {
      return ( BVCE_P_Buffer_GetDeviceOffset_isrsafe( hBuffer ) - BVCE_P_Allocator_GetDeviceOffset( hBuffer->hAllocator ) );
   }
   else
   {
      return 0;
   }
}

void
BVCE_P_Buffer_UnlockDeviceOffset(
   BVCE_P_Buffer_Handle hBuffer
   )
{
   BDBG_ENTER( BVCE_P_Buffer_UnlockDeviceOffset );

   BDBG_ASSERT( hBuffer );

   if ( false == hBuffer->stSettings.bLockOffset )
   {
      if ( true == hBuffer->bOffsetValid )
      {
         if ( NULL == hBuffer->hAllocator->hRange )
         {
            BMMA_UnlockOffset( hBuffer->stBlock.hBlock, hBuffer->uiOffset );
         }
         hBuffer->bOffsetValid = false;
         hBuffer->uiOffset = 0;
      }
   }

   BDBG_LEAVE( BVCE_P_Buffer_UnlockDeviceOffset );
}

void* BVCE_P_Buffer_LockAddress(
   BVCE_P_Buffer_Handle hBuffer
   )
{
   BDBG_ENTER( BVCE_P_Buffer_LockAddress );

   BDBG_ASSERT( hBuffer );

   if ( false == hBuffer->bBufferValid )
   {
      if ( NULL != hBuffer->hAllocator->hRange )
      {
         hBuffer->hAllocator->pBufferBase = BMMA_Lock( hBuffer->hAllocator->hBlock );
         if ( NULL != hBuffer->hAllocator->pBufferBase )
         {
            hBuffer->pBuffer = (void*) ((unsigned) hBuffer->hAllocator->pBufferBase + (unsigned) ( BVCE_P_Buffer_GetDeviceOffset_isrsafe( hBuffer ) - hBuffer->hAllocator->uiDeviceOffset ));
            hBuffer->bBufferValid = true;
            hBuffer->hAllocator->uiBlockRefCount++;
         }
         else
         {
            BDBG_ERR(("Error locking address for memory range block"));
            hBuffer->pBuffer = NULL;
         }
      }
      else
      {
         hBuffer->pBuffer = BMMA_Lock( hBuffer->stBlock.hBlock );

         if ( NULL != hBuffer->pBuffer )
         {
            hBuffer->bBufferValid = true;
         }
         else
         {
            BDBG_ERR(("Error locking address for memory block"));
         }
      }
   }

   BDBG_LEAVE( BVCE_P_Buffer_LockAddress );

   return hBuffer->pBuffer;
}

void*
BVCE_P_Buffer_GetAddress_isrsafe(
   BVCE_P_Buffer_Handle hBuffer
   )
{
   BDBG_ASSERT( hBuffer );

   if ( true == hBuffer->bBufferValid )
   {
      return hBuffer->pBuffer;
   }
   else
   {
      BDBG_ERR(("BVCE_P_Buffer_GetAddress() called for an unlocked buffer"));
      return NULL;
   }
}

void
BVCE_P_Buffer_FlushCache_isr(
   BVCE_P_Buffer_Handle hBuffer,
   void *pBuffer,
   size_t uiSize
   )
{
   BDBG_ASSERT( hBuffer );
   BDBG_ASSERT( pBuffer );

   BDBG_ENTER( BVCE_P_Buffer_FlushCache_isr );

   if ( true == hBuffer->bBufferValid )
   {
      if ( NULL != hBuffer->hAllocator->hRange )
      {
         BMMA_FlushCache_isr(
            hBuffer->hAllocator->hBlock,
            pBuffer,
            uiSize
            );
      }
      else
      {
         BMMA_FlushCache_isr(
            hBuffer->stBlock.hBlock,
            pBuffer,
            uiSize
            );
      }
   }
   else
   {
      BDBG_ERR(("BVCE_P_Buffer_FlushCache_isr() called for an unlocked buffer"));
   }

   BDBG_LEAVE( BVCE_P_Buffer_FlushCache_isr );
}

void BVCE_P_Buffer_UnlockAddress(
   BVCE_P_Buffer_Handle hBuffer
   )
{
   BDBG_ENTER( BVCE_P_Buffer_UnlockAddress );

   BDBG_ASSERT( hBuffer );

   if ( false == hBuffer->stSettings.bLockAddress )
   {
      if ( true == hBuffer->bBufferValid )
      {
         if ( NULL == hBuffer->hAllocator->hRange )
         {
            BMMA_Unlock( hBuffer->stBlock.hBlock, hBuffer->pBuffer );
         }
         else
         {
            BMMA_Unlock( hBuffer->hAllocator->hBlock, hBuffer->hAllocator->pBufferBase );
            if (--hBuffer->hAllocator->uiBlockRefCount == 0)
            {
               hBuffer->hAllocator->pBufferBase = NULL;
            }
         }
         hBuffer->bBufferValid = false;
         hBuffer->pBuffer = NULL;
      }
   }

   BDBG_LEAVE( BVCE_P_Buffer_UnlockAddress );
}

void
BVCE_P_Buffer_Free(
   BVCE_P_Buffer_Handle hBuffer
   )
{
   BDBG_ENTER( BVCE_P_Buffer_Free );
   if ( NULL != hBuffer )
   {
      hBuffer->stSettings.bLockAddress = false;
      BVCE_P_Buffer_UnlockAddress( hBuffer );

      hBuffer->stSettings.bLockOffset= false;
      BVCE_P_Buffer_UnlockDeviceOffset( hBuffer );

      if ( NULL != hBuffer->hAllocator->hRange )
      {
         if ( NULL != hBuffer->stBlock.hRangeBlock )
         {
            BMMA_RangeAllocator_Free(
               hBuffer->hAllocator->hRange,
               hBuffer->stBlock.hRangeBlock
               );
            hBuffer->stBlock.hRangeBlock = NULL;
         }
      }
      else
      {
         if ( NULL != hBuffer->stBlock.hBlock )
         {
            BMMA_Free( hBuffer->stBlock.hBlock );
            hBuffer->stBlock.hBlock = NULL;
         }
      }

      BKNI_Free( hBuffer );
   }

   BDBG_LEAVE( BVCE_P_Buffer_Free );
}

BERR_Code
BVCE_P_Allocator_Create(
   BMMA_Heap_Handle hHeap,
   const BVCE_P_Buffer_AllocSettings *pstAllocSettings, /* Non-Null Indicate Create a Sub-Heap Allocator */
   BVCE_P_Allocator_Handle *phAllocator
   )
{
   BVCE_P_Allocator_Handle hAllocator = NULL;

   BDBG_ENTER( BVCE_P_Allocator_Create );
   BDBG_ASSERT( hHeap );
   BDBG_ASSERT( phAllocator );

   hAllocator = (BVCE_P_Allocator_Handle) BKNI_Malloc( sizeof( BVCE_P_Allocator ) );
   if ( NULL == hAllocator )
   {
      BDBG_ERR(("Error allocating Allocator context"));
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }
   BKNI_Memset( hAllocator, 0, sizeof( BVCE_P_Allocator ) );

   hAllocator->hHeap = hHeap;

   if ( ( NULL != pstAllocSettings ) && ( 0 != pstAllocSettings->uiSize ) )
   {
      hAllocator->stSettings = *pstAllocSettings;
      /* Allocate sub-heap */
      hAllocator->hBlock = BMMA_Alloc(
         hAllocator->hHeap,
         hAllocator->stSettings.uiSize,
         hAllocator->stSettings.uiAlignment,
         0
         );

      if ( NULL == hAllocator->hBlock )
      {
         BDBG_ERR(("Error allocating memory"));
         BVCE_P_Allocator_Destroy( hAllocator );
         return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
      }

      hAllocator->uiDeviceOffset = BMMA_LockOffset( hAllocator->hBlock );

      /* Create BMMA Range Allocator */
      {
         BERR_Code rc;
         BMMA_RangeAllocator_CreateSettings stRangeAllocatorCreateSettings;

         BMMA_RangeAllocator_GetDefaultCreateSettings(
                  &stRangeAllocatorCreateSettings
                  );

         stRangeAllocatorCreateSettings.base = hAllocator->uiDeviceOffset;
         stRangeAllocatorCreateSettings.size = hAllocator->stSettings.uiSize;

         rc = BMMA_RangeAllocator_Create(
            &hAllocator->hRange,
            &stRangeAllocatorCreateSettings
            );

         if ( BERR_SUCCESS != rc )
         {
            BDBG_ERR(("Error creating range allocator"));
            BVCE_P_Allocator_Destroy( hAllocator );
            return BERR_TRACE( rc );
         }
      }
   }

   *phAllocator = hAllocator;

   BDBG_LEAVE( BVCE_P_Allocator_Create );

   return BERR_TRACE( BERR_SUCCESS );
}

size_t
BVCE_P_Allocator_GetSize(
   BVCE_P_Allocator_Handle hAllocator
   )
{
   BDBG_ASSERT( hAllocator );

   if ( NULL != hAllocator->hRange )
   {
      return hAllocator->stSettings.uiSize;
   }
   else
   {
      return 0;
   }
}

BMMA_DeviceOffset
BVCE_P_Allocator_GetDeviceOffset(
   BVCE_P_Allocator_Handle hAllocator
   )
{
   BDBG_ASSERT( hAllocator );

   if ( NULL != hAllocator->hRange )
   {
      return hAllocator->uiDeviceOffset;
   }
   else
   {
      return 0;
   }
}

void
BVCE_P_Allocator_Destroy(
   BVCE_P_Allocator_Handle hAllocator
   )
{
   BDBG_ENTER( BVCE_P_Allocator_Destroy );

   if ( NULL != hAllocator )
   {
      if ( NULL != hAllocator->hRange )
      {
         BMMA_RangeAllocator_Destroy( hAllocator->hRange );
         hAllocator->hRange = NULL;
      }

      if ( NULL != hAllocator->hBlock )
      {
         BDBG_ASSERT(hAllocator->uiBlockRefCount == 0);
         BMMA_UnlockOffset( hAllocator->hBlock, hAllocator->uiDeviceOffset );
         BMMA_Free( hAllocator->hBlock );
         hAllocator->hBlock = NULL;
      }

      BKNI_Free( hAllocator );
   }

   BDBG_LEAVE( BVCE_P_Allocator_Destroy );
}
