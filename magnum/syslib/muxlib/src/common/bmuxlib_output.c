/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h" /* also includes berr, bdbg, etc */
#include "bkni.h"

#include "bmuxlib_output.h"

BDBG_MODULE(BMUXLIB_OUTPUT);
BDBG_FILE_MODULE(BMUXLIB_OUTPUT_DESC);    /* enables descriptor diagnostics */
BDBG_FILE_MODULE(BMUXLIB_OUTPUT_OFFSETS); /* enables diagnostics for offsets */
BDBG_FILE_MODULE(BMUXLIB_OUTPUT_USAGE);   /* enables descriptor usage statistics */

/**************/
/* Signatures */
/**************/
#define BMUXLIB_OUTPUT_P_SIGNATURE_CREATESETTINGS   0x79858401

#define OUTPUT_ID(x) ((x)->stCreateSettings.uiOutputID)

typedef struct
{
   BMUXlib_Output_Descriptor stDesc;
   BMUXlib_Output_CompletedCallbackInfo stCallbackInfo;
      /* NOTE: this maintained here so we do not corrupt the original descriptor */
   uint64_t uiOffset;         /* calculated absolute offset for this descriptor */
} BMUXlib_Output_P_MetaDescriptor;

typedef struct BMUXlib_Output_Context
{
   BMUXlib_Output_CreateSettings stCreateSettings;

   /* incoming descriptors ...*/
   BMUXlib_Output_P_MetaDescriptor *astInDescriptors;

   /* outgoing vectored descriptors ...
      NOTE: there is one of these for each incoming descriptor - this ensures
      that we have enough space for the worst-case scenario where all incoming
      descriptors are at discontinuous offsets
      This buffer is accessed using the indexes for the input descriptors */
   BMUXlib_StorageDescriptor *astOutDescriptors;

   size_t uiReadIndex;
   size_t uiQueuedIndex;
   size_t uiWriteIndex;

   uint64_t uiCurrentOffset;  /* current position within the output stream (0 = start of stream) */
   uint64_t uiEndOffset;      /* offset of the end of the output stream (may or may not be the same as uiCurrentOffset) */
   uint64_t uiExpectedOffset; /* expected offset of next outgoing descriptor to process (discontinuity detection) */

   bool bDescWaiting;         /* output descriptor is waiting to be queued to storage */

#if BDBG_DEBUG_BUILD
   uint32_t uiMaxUsage;
   uint32_t uiUsageCount;
#endif
} BMUXlib_Output_Context;

void
BMUXlib_Output_GetDefaultCreateSettings(
   BMUXlib_Output_CreateSettings *pstSettings
   )
{
   BDBG_ENTER( BMUXlib_Output_GetDefaultCreateSettings );

   BDBG_ASSERT( pstSettings );

   BKNI_Memset(
      pstSettings,
      0,
      sizeof ( BMUXlib_Output_CreateSettings )
      );

   pstSettings->uiSignature = BMUXLIB_OUTPUT_P_SIGNATURE_CREATESETTINGS;
   pstSettings->uiCount = 256;

   BDBG_LEAVE( BMUXlib_Output_GetDefaultCreateSettings );
}

BERR_Code
BMUXlib_Output_Create(
   BMUXlib_Output_Handle *phOutput,
   const BMUXlib_Output_CreateSettings *pstSettings
   )
{
   size_t uiAllocationSize;
   BMUXlib_Output_Handle hOutput = NULL;

   BDBG_ENTER( BMUXlib_Output_Create );

   BDBG_ASSERT( phOutput );
   BDBG_ASSERT( pstSettings );
   BDBG_ASSERT( pstSettings->uiSignature == BMUXLIB_OUTPUT_P_SIGNATURE_CREATESETTINGS );

   *phOutput = NULL;

   /* verify settings are valid ...
      (note: ID can have any value) */
   if ((0 == pstSettings->uiCount)
      || (NULL == pstSettings->stStorage.pfAddDescriptors)
      || (NULL == pstSettings->stStorage.pfGetCompleteDescriptors))
   {
      BDBG_LEAVE( BMUXlib_Output_Create );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   hOutput = ( BMUXlib_Output_Handle ) BKNI_Malloc( sizeof( BMUXlib_Output_Context ) );

   if ( NULL == hOutput )
   {
      BDBG_LEAVE( BMUXlib_Output_Create );
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }

   BKNI_Memset(
      hOutput,
      0,
      sizeof( BMUXlib_Output_Context )
      );

   hOutput->stCreateSettings = *pstSettings;

   /* allocate the incoming descriptor and metadata array ... */
   uiAllocationSize = sizeof(BMUXlib_Output_P_MetaDescriptor) * hOutput->stCreateSettings.uiCount;
   hOutput->astInDescriptors = (BMUXlib_Output_P_MetaDescriptor *)BKNI_Malloc(uiAllocationSize);
   if ( NULL == hOutput->astInDescriptors )
   {
      BMUXlib_Output_Destroy( hOutput );
      BDBG_LEAVE( BMUXlib_Output_Create );
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }

   BKNI_Memset(hOutput->astInDescriptors, 0, uiAllocationSize);

   /*
    * Size, allocate and clear the storage descriptor array
    */
   uiAllocationSize = sizeof(BMUXlib_StorageDescriptor) * hOutput->stCreateSettings.uiCount;
   hOutput->astOutDescriptors = (BMUXlib_StorageDescriptor *)BKNI_Malloc(uiAllocationSize);
   if (NULL == hOutput->astOutDescriptors)
   {
      BMUXlib_Output_Destroy( hOutput );
      BDBG_LEAVE( BMUXlib_Output_Create );
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }
   BKNI_Memset(hOutput->astOutDescriptors, 0, uiAllocationSize);

   *phOutput = hOutput;

   BDBG_LEAVE( BMUXlib_Output_Create );
   return BERR_SUCCESS;
}

BERR_Code
BMUXlib_Output_Destroy(
   BMUXlib_Output_Handle hOutput
   )
{
   BDBG_ENTER( BMUXlib_Output_Destroy );

   BDBG_ASSERT( hOutput );

   BDBG_MODULE_MSG(BMUXLIB_OUTPUT_USAGE, ("[%d] Descriptors Used: %d/%d (peak)",
                                          hOutput->stCreateSettings.uiOutputID,
                                          hOutput->uiMaxUsage,
                                          (int)hOutput->stCreateSettings.uiCount));

   if ( NULL != hOutput->astInDescriptors )
   {
      BKNI_Free(hOutput->astInDescriptors);
   }

   if (NULL != hOutput->astOutDescriptors)
   {
      BKNI_Free(hOutput->astOutDescriptors);
   }

   BKNI_Free( hOutput );

   BDBG_LEAVE( BMUXlib_Output_Destroy );

   return BERR_TRACE( BERR_SUCCESS );
}

bool
BMUXlib_Output_IsSpaceAvailable(
   BMUXlib_Output_Handle hOutput
   )
{
   size_t uiTempWriteOffset = hOutput->uiWriteIndex;

   uiTempWriteOffset++;
   if ( uiTempWriteOffset >= hOutput->stCreateSettings.uiCount )
   {
      uiTempWriteOffset -= hOutput->stCreateSettings.uiCount;
   }

   return ( uiTempWriteOffset != hOutput->uiReadIndex );
}

bool
BMUXlib_Output_IsDescriptorPendingCompletion(
   BMUXlib_Output_Handle hOutput
   )
{
   return ( hOutput->uiReadIndex != hOutput->uiWriteIndex );
}

bool
BMUXlib_Output_IsDescriptorPendingQueue(
   BMUXlib_Output_Handle hOutput
   )
{
   return ( hOutput->uiQueuedIndex != hOutput->uiWriteIndex );
}

BERR_Code
BMUXlib_Output_AddNewDescriptor(
         BMUXlib_Output_Handle hOutput,
         BMUXlib_Output_Descriptor *pstDescriptor,
         BMUXlib_Output_CompletedCallbackInfo *pstCompletedCallbackInfo
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BMUXlib_Output_AddNewDescriptor );

   BDBG_ASSERT( hOutput );
   BDBG_ASSERT( pstDescriptor );

   if ( true == BMUXlib_Output_IsSpaceAvailable( hOutput ) )
   {
      BMUXlib_Output_P_MetaDescriptor *pMetaDesc = &hOutput->astInDescriptors[hOutput->uiWriteIndex];

      /* save the original descriptor for supplying back via callback ... */
      pMetaDesc->stDesc = *pstDescriptor;

      switch (pstDescriptor->stStorage.eOffsetFrom)
      {
         case BMUXlib_Output_OffsetReference_eEnd:
            /* offset to use is relative to end offset ... */
            hOutput->uiEndOffset += pstDescriptor->stStorage.uiOffset;
            BDBG_MODULE_MSG(BMUXLIB_OUTPUT_OFFSETS, ("[%2.2d]: %d bytes @ offset: "BDBG_UINT64_FMT" (end), ", OUTPUT_ID(hOutput), (int)pstDescriptor->stStorage.uiLength, BDBG_UINT64_ARG(hOutput->uiEndOffset)));
            pMetaDesc->uiOffset = hOutput->uiEndOffset;
            /* update end offset */
            hOutput->uiEndOffset += pstDescriptor->stStorage.uiLength;
            hOutput->uiCurrentOffset = hOutput->uiEndOffset;
            break;
         case BMUXlib_Output_OffsetReference_eCurrent:
            /* offset to use is relative to current offset ... */
            hOutput->uiCurrentOffset += pstDescriptor->stStorage.uiOffset;
            BDBG_MODULE_MSG(BMUXLIB_OUTPUT_OFFSETS, ("[%2.2d]: %d bytes @ offset: "BDBG_UINT64_FMT" (current), ", OUTPUT_ID(hOutput), (int)pstDescriptor->stStorage.uiLength, BDBG_UINT64_ARG(hOutput->uiCurrentOffset)));
            pMetaDesc->uiOffset = hOutput->uiCurrentOffset;
            hOutput->uiCurrentOffset += pstDescriptor->stStorage.uiLength;
            break;
         case BMUXlib_Output_OffsetReference_eStart:
            BDBG_MODULE_MSG(BMUXLIB_OUTPUT_OFFSETS, ("[%2.2d]: %d bytes @ offset: "BDBG_UINT64_FMT" (start), ", OUTPUT_ID(hOutput), (int)pstDescriptor->stStorage.uiLength, BDBG_UINT64_ARG(pstDescriptor->stStorage.uiOffset)));
            pMetaDesc->uiOffset = pstDescriptor->stStorage.uiOffset;
            hOutput->uiCurrentOffset = pstDescriptor->stStorage.uiOffset + pstDescriptor->stStorage.uiLength;
            break;
         default:
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
            /* NOTE: since write index will not be updated, above changes effectively disappear (i.e. no change to
               queue upon error) */
            break;
      }
      if (BERR_SUCCESS == rc)
      {
         if (hOutput->uiCurrentOffset > hOutput->uiEndOffset)
         {
            hOutput->uiEndOffset = hOutput->uiCurrentOffset;
         }

         BDBG_MODULE_MSG(BMUXLIB_OUTPUT_OFFSETS, ("[%2.2d]: Current Offset = "BDBG_UINT64_FMT", End Offset = "BDBG_UINT64_FMT, OUTPUT_ID(hOutput), BDBG_UINT64_ARG(hOutput->uiCurrentOffset), BDBG_UINT64_ARG(hOutput->uiEndOffset)));

         if ( NULL != pstCompletedCallbackInfo )
         {
            pMetaDesc->stCallbackInfo = *pstCompletedCallbackInfo;
         }
         else
         {
            BKNI_Memset( &pMetaDesc->stCallbackInfo, 0, sizeof( BMUXlib_Output_CompletedCallbackInfo ) );
         }

         hOutput->uiWriteIndex++;
         if ( hOutput->uiWriteIndex >= hOutput->stCreateSettings.uiCount )
         {
            hOutput->uiWriteIndex -= hOutput->stCreateSettings.uiCount;
         }
#if BDBG_DEBUG_BUILD
         hOutput->uiUsageCount++;
         if (hOutput->uiUsageCount > hOutput->uiMaxUsage)
            hOutput->uiMaxUsage = hOutput->uiUsageCount;
#endif
      } /* end: if success */
   }
   else
   {
      BDBG_MSG(("[%2.2d]: Output Queue is Full!", OUTPUT_ID(hOutput)));
      rc = BERR_TRACE(BERR_UNKNOWN);
   }

   BDBG_LEAVE( BMUXlib_Output_AddNewDescriptor );

   return rc;
}

BERR_Code
BMUXlib_Output_ProcessNewDescriptors(
   BMUXlib_Output_Handle hOutput
   )
{
   size_t uiQueuedCount;
   uint64_t uiExpectedOffset = 0;
   bool bExpectedWriteOperation = false;
   BERR_Code rc = BERR_SUCCESS;
   BMUXlib_StorageDescriptor *pSDesc;
   BMUXlib_StorageObjectInterface *pstStorage = &hOutput->stCreateSettings.stStorage;

   BDBG_ENTER( BMUXlib_Output_ProcessNewDescriptors );

   BDBG_ASSERT( hOutput );


   /* if a previously created storage descriptor is waiting to be queued, send it first ...*/
   if (hOutput->bDescWaiting)
   {
      BDBG_MODULE_MSG(BMUXLIB_OUTPUT_DESC, ("[%2.2d]: Re-Queueing waiting storage descriptor", OUTPUT_ID(hOutput)));
      pSDesc = &hOutput->astOutDescriptors[hOutput->uiQueuedIndex];
      rc = pstStorage->pfAddDescriptors(pstStorage->pContext, pSDesc, 1, &uiQueuedCount);
      hOutput->bDescWaiting = (uiQueuedCount != 1);
      if (BERR_SUCCESS == rc && !hOutput->bDescWaiting)
      {
         hOutput->uiQueuedIndex += pSDesc->uiVectorCount;
         if ( hOutput->uiQueuedIndex == hOutput->stCreateSettings.uiCount )
         {
            hOutput->uiQueuedIndex -= hOutput->stCreateSettings.uiCount;
         }
      }
   }
   while (BERR_SUCCESS == rc && !hOutput->bDescWaiting && hOutput->uiQueuedIndex != hOutput->uiWriteIndex)
   {
      /* previous queuing operation completed, and there are new descriptors to process ... */
      uint32_t uiCurrentIndex = hOutput->uiQueuedIndex;    /* index into the waiting output descriptors */
      uint32_t uiCount, uiNewDescCount;
      /* determine how many new descriptors are contiguous ... */
      if ( hOutput->uiQueuedIndex < hOutput->uiWriteIndex )
      {
         uiNewDescCount = hOutput->uiWriteIndex - hOutput->uiQueuedIndex;
      }
      else
      {
         /* new descriptors split around end of buffer, so process those until the end of the buffer ... */
         uiNewDescCount = hOutput->stCreateSettings.uiCount - hOutput->uiQueuedIndex;
      }
      BDBG_MODULE_MSG(BMUXLIB_OUTPUT_DESC, ("[%2.2d]: Processing %d new descriptors", OUTPUT_ID(hOutput), uiNewDescCount));

      for (uiCount = 0; uiCount < uiNewDescCount; uiCount++, uiCurrentIndex++)
      {
         /* NOTE: every storage descriptor is created aligned to a queued index boundary
            Storage descriptors are always output one-at-a-time to storage
            Queued Index is updated throughout this loop */
         BMUXlib_Output_P_MetaDescriptor *pMetaDesc = &hOutput->astInDescriptors[uiCurrentIndex];
         BMUXlib_Output_StorageDescriptor *pInDesc = &pMetaDesc->stDesc.stStorage;
         /* if the offset is discontinuous, or the read/write operation changes,
               then output the current storage descriptor (descriptor is done) */
         if ((0 != uiCount) && ((pMetaDesc->uiOffset != uiExpectedOffset)
               || (pInDesc->bWriteOperation != bExpectedWriteOperation)))
         {
            /* descriptor to send to storage is the one at the queued index */
            pSDesc =  &hOutput->astOutDescriptors[hOutput->uiQueuedIndex];
            rc = pstStorage->pfAddDescriptors(pstStorage->pContext, pSDesc, 1, &uiQueuedCount);
            if (BERR_SUCCESS != rc)
            {
               BDBG_ERR(("[%2.2d]: Queue to storage failed with a return code of %d", OUTPUT_ID(hOutput), rc));
               break;
            }
            hOutput->bDescWaiting = (1 != uiQueuedCount);
            if (!hOutput->bDescWaiting)
            {
               hOutput->uiQueuedIndex += pSDesc->uiVectorCount;
               /* NOTE: no need to wrap the index here - we know these must be contiguous */
            }
            else
               break;      /* done! */
         }
         /* fetch the updated storage descriptor location ... */
         pSDesc =  &hOutput->astOutDescriptors[hOutput->uiQueuedIndex];

         if (uiCurrentIndex == hOutput->uiQueuedIndex)
         {
            /* start a new storage descriptor (i.e. fill in the main
               descriptor information) */
            pSDesc->uiOffset = pMetaDesc->uiOffset;
            pSDesc->bWriteOperation = pInDesc->bWriteOperation;
            bExpectedWriteOperation = pSDesc->bWriteOperation;
            pSDesc->uiVectorCount = 0;
         }

         /* write the vector entry for this incoming descriptor ... */
         if ( ( 0 == pInDesc->uiLength) || ( NULL != pInDesc->pBufferAddress ) )
         {
            pSDesc->iov[pSDesc->uiVectorCount].pBufferAddress = pInDesc->pBufferAddress;
            pInDesc->hBlock = NULL;
         }
         else
         {
            BDBG_ASSERT( NULL != pInDesc->hBlock );

            pSDesc->iov[pSDesc->uiVectorCount].pBufferAddress = (void *) ((uint8_t *)BMMA_Lock( pInDesc->hBlock ) + pInDesc->uiBlockOffset );
         }
         pSDesc->iov[pSDesc->uiVectorCount].uiLength = pInDesc->uiLength;
         uiExpectedOffset = pMetaDesc->uiOffset + pInDesc->uiLength;

         BDBG_MODULE_MSG(BMUXLIB_OUTPUT_DESC, ("[%2.2d]: +Desc[%d]: cb: %p (d: %p), %d bytes %s %p @ "BDBG_UINT64_FMT" (%d) [abs:"BDBG_UINT64_FMT"]: sd: %p, vc:%d",
            OUTPUT_ID(hOutput), uiCurrentIndex, (void *)(unsigned long)(pMetaDesc->stCallbackInfo.pCallback), pMetaDesc->stCallbackInfo.pCallbackData,
            (int)pInDesc->uiLength, (pInDesc->bWriteOperation)?"from":"to", pInDesc->pBufferAddress,
            BDBG_UINT64_ARG(pInDesc->uiOffset), pInDesc->eOffsetFrom, BDBG_UINT64_ARG(pMetaDesc->uiOffset), (void *)pSDesc, pSDesc->uiVectorCount));

         pSDesc->uiVectorCount++;
      } /* end: for each new descriptor */

      if (BERR_SUCCESS == rc && !hOutput->bDescWaiting)
      {
         /* output the last descriptor created */
         pSDesc =  &hOutput->astOutDescriptors[hOutput->uiQueuedIndex];
         rc = pstStorage->pfAddDescriptors(pstStorage->pContext, pSDesc, 1, &uiQueuedCount);
         if (BERR_SUCCESS != rc)
         {
            BDBG_ERR(("[%2.2d]: Queue to storage failed with a return code of %d", OUTPUT_ID(hOutput), rc));
            break;
         }
         hOutput->bDescWaiting = (1 != uiQueuedCount);
         if (!hOutput->bDescWaiting)
         {
            hOutput->uiQueuedIndex += pSDesc->uiVectorCount;
            if ( hOutput->uiQueuedIndex == hOutput->stCreateSettings.uiCount )
            {
               hOutput->uiQueuedIndex -= hOutput->stCreateSettings.uiCount;
            }
         }
      }
   } /* end: while descriptors to process */

   BDBG_LEAVE( BMUXlib_Output_ProcessNewDescriptors );

   return BERR_TRACE( rc );
}

BERR_Code BMUXlib_Output_ProcessCompletedDescriptors( BMUXlib_Output_Handle hOutput )
{
   size_t uiCompletedCount = 0;
   uint32_t i;
   BERR_Code rc = BERR_SUCCESS;
   BMUXlib_StorageObjectInterface *pstStorage = &hOutput->stCreateSettings.stStorage;

   BDBG_ENTER( BMUXlib_Output_ProcessCompletedDescriptors );

   BDBG_ASSERT( hOutput );

   rc = pstStorage->pfGetCompleteDescriptors( pstStorage->pContext, &uiCompletedCount );

   BDBG_MODULE_MSG(BMUXLIB_OUTPUT_DESC, ("[%2.2d]: %d Descriptors Completed", OUTPUT_ID(hOutput), (int)uiCompletedCount));

   if ( BERR_SUCCESS == rc )
   {
      /* for each descriptor completed, release the corresponding incoming descriptors */
      while ( 0 != uiCompletedCount )
      {
         /* each new descriptor is aligned to an incoming descriptor boundary ... */
         BMUXlib_StorageDescriptor *pDesc = &hOutput->astOutDescriptors[hOutput->uiReadIndex];
         /* for the number of iov in the descriptor, iterate over the input descriptors
               and invoke the callbacks, and free the required out descriptors memory */
         /* NOTE: We know these must be contiguous, so no need to check for wrap */

         for (i = 0; i < pDesc->uiVectorCount; i++)
         {
            BMUXlib_Output_P_MetaDescriptor *pMetaDesc = &hOutput->astInDescriptors[hOutput->uiReadIndex + i];
            BMUXlib_Output_StorageDescriptor *pInDesc = &pMetaDesc->stDesc.stStorage;

            BDBG_MODULE_MSG(BMUXLIB_OUTPUT_DESC, ("[%2.2d]: -Desc[%d]: cb: %p (d: %p), %d bytes %s %p @ "BDBG_UINT64_FMT" (%d) [abs:"BDBG_UINT64_FMT"]: sd:%p, vc:%d",
            OUTPUT_ID(hOutput), (int)(hOutput->uiReadIndex + i), (void *)(unsigned long)pMetaDesc->stCallbackInfo.pCallback, pMetaDesc->stCallbackInfo.pCallbackData,
            (int)pInDesc->uiLength, (pInDesc->bWriteOperation)?"from":"to", pInDesc->pBufferAddress,
            BDBG_UINT64_ARG(pInDesc->uiOffset), pInDesc->eOffsetFrom, BDBG_UINT64_ARG(pMetaDesc->uiOffset), (void *)pDesc, i));

            if ( NULL != pInDesc->hBlock )
            {
               BMMA_Unlock( pInDesc->hBlock, (void *) ( (uint8_t *) pDesc->iov[i].pBufferAddress - pInDesc->uiBlockOffset ) );
            }

            if ( NULL != pMetaDesc->stCallbackInfo.pCallback )
            {
               pMetaDesc->stCallbackInfo.pCallback(pMetaDesc->stCallbackInfo.pCallbackData, &pMetaDesc->stDesc);
            }
         }
         /* account for the incoming descriptors that made up this output descriptor ... */
         hOutput->uiReadIndex += pDesc->uiVectorCount;
         BDBG_ASSERT(hOutput->uiReadIndex <= hOutput->stCreateSettings.uiCount);
         if ( hOutput->uiReadIndex >= hOutput->stCreateSettings.uiCount )
         {
            hOutput->uiReadIndex -= hOutput->stCreateSettings.uiCount;
         }

         uiCompletedCount--;
#if BDBG_DEBUG_BUILD
         hOutput->uiUsageCount--;
#endif
      } /* end: while not completed */
   }
   BDBG_LEAVE( BMUXlib_Output_ProcessCompletedDescriptors );

   return BERR_TRACE( rc );
}

uint64_t
BMUXlib_Output_GetCurrentOffset(
   BMUXlib_Output_Handle hOutput
   )
{
   BDBG_ASSERT(hOutput);
   BDBG_MODULE_MSG(BMUXLIB_OUTPUT_OFFSETS,("[%2.2d]: GetCurrentOffset: "BDBG_UINT64_FMT, OUTPUT_ID(hOutput), BDBG_UINT64_ARG(hOutput->uiCurrentOffset)));
   return hOutput->uiCurrentOffset;
}

uint64_t
BMUXlib_Output_GetEndOffset(
   BMUXlib_Output_Handle hOutput
   )
{
   BDBG_ASSERT(hOutput);
   BDBG_MODULE_MSG(BMUXLIB_OUTPUT_OFFSETS,("[%2.2d]: GetEndOffset: "BDBG_UINT64_FMT, OUTPUT_ID(hOutput), BDBG_UINT64_ARG(hOutput->uiEndOffset)));
   return hOutput->uiEndOffset;
}

BERR_Code
BMUXlib_Output_SetCurrentOffset(
   BMUXlib_Output_Handle hOutput,
   uint64_t uiOffset,
   BMUXlib_Output_OffsetReference eOffsetFrom
   )
{
   BDBG_ASSERT(hOutput);
   switch (eOffsetFrom)
   {
      case BMUXlib_Output_OffsetReference_eEnd:
         /* offset is relative to end ... */
         hOutput->uiCurrentOffset = hOutput->uiEndOffset + uiOffset;
         break;
      case BMUXlib_Output_OffsetReference_eCurrent:
         /* offset is relative to current offset ... */
         hOutput->uiCurrentOffset += uiOffset;
         break;
      case BMUXlib_Output_OffsetReference_eStart:
         hOutput->uiCurrentOffset = uiOffset;
         break;
      default:
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   if (hOutput->uiCurrentOffset > hOutput->uiEndOffset)
      hOutput->uiEndOffset = hOutput->uiCurrentOffset;
   BDBG_MODULE_MSG(BMUXLIB_OUTPUT_OFFSETS,("[%2.2d]: Setting Offset: "BDBG_UINT64_FMT, OUTPUT_ID(hOutput), BDBG_UINT64_ARG(hOutput->uiCurrentOffset)));
   return BERR_SUCCESS;
}

/*****************************************************************************
* EOF
******************************************************************************/
