/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
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

#include "bmuxlib_pqueue.h"
#include "bkni.h"

BDBG_MODULE(BMUXLIB_PQUEUE);

typedef struct BMUXlib_P_PriorityQueue
{
   BMUXlib_PriorityQueue_CreateSettings stSettings;

   unsigned uiCount;
   void *astQueue;
} BMUXlib_P_PriorityQueue;

#define BMUXlib_P_PriorityQueue_Parent( index ) ( ( (index) == 1 ) ? 0 : (index)/2 )
#define BMUXlib_P_PriorityQueue_YoungChild( index ) ( 2 * (index) )
#define BMUXlib_P_PriorityQueue_Entry( q, index ) ( (void*) (((uint8_t*) (q)->astQueue) + ( (index) * (q)->stSettings.uiEntrySize ) ) )
#define BMUXlib_P_PriorityQueue_Copy( q, entryDst, entrySrc ) BKNI_Memcpy( entryDst, entrySrc, (q)->stSettings.uiEntrySize )
#define BMUXlib_P_PriorityQueue_Swap( q, indexA, indexB ) { BMUXlib_P_PriorityQueue_Copy( q, BMUXlib_P_PriorityQueue_Entry( q, 0 ), BMUXlib_P_PriorityQueue_Entry( q, indexA ) );\
                                                            BMUXlib_P_PriorityQueue_Copy( q, BMUXlib_P_PriorityQueue_Entry( q, indexA ), BMUXlib_P_PriorityQueue_Entry( q, indexB ) );\
                                                            BMUXlib_P_PriorityQueue_Copy( q, BMUXlib_P_PriorityQueue_Entry( q, indexB ), BMUXlib_P_PriorityQueue_Entry( q, 0 ) ); }

void
BMUXlib_PriorityQueue_GetDefaultCreateSettings(
   BMUXlib_PriorityQueue_CreateSettings *pstSettings
   )
{
   BKNI_Memset( pstSettings, 0, sizeof( BMUXlib_PriorityQueue_CreateSettings ) );
}

BERR_Code
BMUXlib_PriorityQueue_Create(
   BMUXlib_PriorityQueue_Handle *phQueue,
   const BMUXlib_PriorityQueue_CreateSettings *pstSettings
   )
{
   BMUXlib_PriorityQueue_Handle hQueue = NULL;

   BDBG_ENTER( BMUXlib_PriorityQueue_Create );

   BDBG_ASSERT( phQueue );
   BDBG_ASSERT( pstSettings );
   BDBG_ASSERT( pstSettings->fCmp );
   BDBG_ASSERT( pstSettings->uiSize > 0 );
   BDBG_ASSERT( pstSettings->uiEntrySize > 0 );

   hQueue = ( BMUXlib_PriorityQueue_Handle ) BKNI_Malloc( sizeof( BMUXlib_P_PriorityQueue ) );
   if ( NULL == hQueue )
   {
      BDBG_LEAVE( BMUXlib_PriorityQueue_Create );
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }

   BKNI_Memset( hQueue, 0, sizeof( BMUXlib_P_PriorityQueue ) );

   hQueue->stSettings = *pstSettings;

   hQueue->astQueue = BKNI_Malloc( ( hQueue->stSettings.uiSize + 1 ) * hQueue->stSettings.uiEntrySize ); /* Always leave room for an extra element to support SWAP */
   if ( NULL == hQueue->astQueue )
   {
      BMUXlib_PriorityQueue_Destroy( hQueue );
      BDBG_LEAVE( BMUXlib_PriorityQueue_Create );
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }

   *phQueue = hQueue;

   BDBG_LEAVE( BMUXlib_PriorityQueue_Create );

   return BERR_TRACE( BERR_SUCCESS );
}

void
BMUXlib_PriorityQueue_Destroy(
   BMUXlib_PriorityQueue_Handle hQueue
   )
{
   BDBG_ENTER( BMUXlib_PriorityQueue_Destroy );

   BDBG_ASSERT( hQueue );

   if ( NULL != hQueue->astQueue )
   {
      BKNI_Free( hQueue->astQueue );
      hQueue->astQueue = NULL;
   }

   BKNI_Free( hQueue );

   BDBG_LEAVE( BMUXlib_PriorityQueue_Destroy );
}

void
BMUXlib_P_PriorityQueue_BubbleUp(
   BMUXlib_PriorityQueue_Handle hQueue,
   unsigned uiIndex
   )
{
   if ( 0 == BMUXlib_P_PriorityQueue_Parent( uiIndex ) ) return; /* At the root of the heap, no parent */

   if ( hQueue->stSettings.fCmp( BMUXlib_P_PriorityQueue_Entry( hQueue, BMUXlib_P_PriorityQueue_Parent( uiIndex ) ), BMUXlib_P_PriorityQueue_Entry( hQueue, uiIndex ) ) > 0 )
   {
      BMUXlib_P_PriorityQueue_Swap( hQueue, uiIndex, BMUXlib_P_PriorityQueue_Parent( uiIndex ) );
      BMUXlib_P_PriorityQueue_BubbleUp( hQueue, BMUXlib_P_PriorityQueue_Parent( uiIndex ) );
   }
}

void
BMUXlib_P_PriorityQueue_BubbleDown(
   BMUXlib_PriorityQueue_Handle hQueue,
   unsigned uiIndex
   )
{
   unsigned i;
   unsigned uiChildIndex;
   unsigned uiLightestChildIndex;

   uiChildIndex = BMUXlib_P_PriorityQueue_YoungChild( uiIndex );
   uiLightestChildIndex = uiIndex;

   for ( i = 0; i <= 1; i++ )
   {
      if ( ( uiChildIndex + i ) <= hQueue->uiCount )
      {
         if ( hQueue->stSettings.fCmp( BMUXlib_P_PriorityQueue_Entry( hQueue, uiLightestChildIndex ), BMUXlib_P_PriorityQueue_Entry( hQueue, uiChildIndex + i ) ) > 0 )
         {
            uiLightestChildIndex = uiChildIndex + i;
         }
      }
   }

   if ( uiLightestChildIndex != uiIndex )
   {
      BMUXlib_P_PriorityQueue_Swap( hQueue, uiIndex, uiLightestChildIndex );
      BMUXlib_P_PriorityQueue_BubbleDown( hQueue, uiLightestChildIndex );
   }
}

BERR_Code
BMUXlib_PriorityQueue_Insert(
   BMUXlib_PriorityQueue_Handle hQueue,
   void *pEntry
   )
{
   BDBG_ENTER( BMUXlib_PriorityQueue_Insert );
   BDBG_ASSERT( hQueue );
   BDBG_ASSERT( pEntry );

   if ( hQueue->uiCount >= hQueue->stSettings.uiSize )
   {
      BDBG_ERR(("Priority Queue is Full"));
      BDBG_LEAVE( BMUXlib_PriorityQueue_Insert );
      return BERR_TRACE( BERR_UNKNOWN );
   }
   else
   {
      hQueue->uiCount++;
      BMUXlib_P_PriorityQueue_Copy( hQueue, BMUXlib_P_PriorityQueue_Entry( hQueue, hQueue->uiCount ), pEntry );
      BMUXlib_P_PriorityQueue_BubbleUp( hQueue, hQueue->uiCount );
   }

   BDBG_LEAVE( BMUXlib_PriorityQueue_Insert );
   return BERR_TRACE( BERR_SUCCESS );
}

bool
BMUXlib_PriorityQueue_PeekMin(
   BMUXlib_PriorityQueue_Handle hQueue,
   void *pEntry
   )
{
   BDBG_ENTER( BMUXlib_PriorityQueue_PeekMin);
   BDBG_ASSERT( hQueue );
   BDBG_ASSERT( pEntry );

   if ( hQueue->uiCount == 0 )
   {
      return false;
   }
   else
   {
      BMUXlib_P_PriorityQueue_Copy( hQueue, pEntry, BMUXlib_P_PriorityQueue_Entry( hQueue, 1 ) );
   }

   BDBG_LEAVE( BMUXlib_PriorityQueue_PeekMin);
   return true;
}

BERR_Code
BMUXlib_PriorityQueue_ExtractMin(
   BMUXlib_PriorityQueue_Handle hQueue,
   void *pEntry
   )
{
   BERR_Code rc = BERR_UNKNOWN;

   BDBG_ENTER( BMUXlib_PriorityQueue_ExtractMin);
   BDBG_ASSERT( hQueue );
   BDBG_ASSERT( pEntry );

   if ( true == BMUXlib_PriorityQueue_PeekMin( hQueue, pEntry ) )
   {
      BMUXlib_P_PriorityQueue_Copy( hQueue, BMUXlib_P_PriorityQueue_Entry( hQueue, 1 ), BMUXlib_P_PriorityQueue_Entry( hQueue, hQueue->uiCount ) );
      hQueue->uiCount--;
      BMUXlib_P_PriorityQueue_BubbleDown( hQueue, 1 );
      rc = BERR_SUCCESS;
   }
   else
   {
      BDBG_ASSERT(0);
   }

   BDBG_LEAVE( BMUXlib_PriorityQueue_PeekMin);
   return BERR_TRACE( rc );
}

unsigned
BMUXlib_PriorityQueue_GetCount(
   BMUXlib_PriorityQueue_Handle hQueue
   )
{
   BDBG_ENTER( BMUXlib_PriorityQueue_GetCount );
   BDBG_LEAVE( BMUXlib_PriorityQueue_GetCount );
   return hQueue->uiCount;
}
