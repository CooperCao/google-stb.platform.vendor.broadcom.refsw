/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * [File Description:]
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bxdm_pp_xmo.h"

BDBG_MODULE(BXDM_PPXMO);

BDBG_FILE_MODULE(BXDM_XMOCTL);  /* Register software module with debug interface */
BDBG_FILE_MODULE(BXDM_XMODBG);

static uint32_t BXDM_PP_XMO_S_Queue_GetFreeElementCount_isr(
   BXDM_PictureProvider_XMO_Handle hXmo
   );

static uint32_t BXDM_PP_XMO_S_Queue_GetPictureCount_isr(
   BXDM_PictureProvider_XMO_Handle hXmo
   );

static void BXDM_PP_XMO_S_Queue_GetPicture_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_PP_XDM_P_QueueElement ** ppstQueueElement,
   bool bBumpReadOffset
   );

static void BXDM_PP_XMO_S_Queue_GetLastElement_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_PP_XDM_P_QueueElement ** ppstQueueElement,
   bool bBumpReadOffset
   );

static void BXDM_PP_XMO_S_Queue_GetNextFreeElement_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_PP_XDM_P_QueueElement ** ppstQueueElement
   );

static void BXDM_PP_XMO_S_Queue_ReleasePicture_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_PP_XDM_P_QueueElement * pstQueueElement
   );

static void BXDM_PP_XMO_S_Queue_AddPicture_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   const BXDM_Picture * pstBasePicture,
   const BXDM_Picture * pstEnhancedPicture
   );

/*******************************************************************************
 *
 * Begin Local functions
 * Should these be local functions or private functions in another module?
 *
 *******************************************************************************/

/*
 *
 */
static void BXDM_PP_XMO_S_Initialize(
   BXDM_PictureProvider_XMO_Handle hXmo
   )
{
   BXDM_PP_XMO_P_PictureQueue *  pstPictureQueue;

   BDBG_ENTER(BXDM_PP_XMO_S_Initialize);

   /* Initialize the Picture Queue */
   pstPictureQueue = &(hXmo->stPictureQueue);

   BKNI_Memset( pstPictureQueue, 0, sizeof( BXDM_PP_XMO_P_PictureQueue ) );

   pstPictureQueue->pstHeadElement = &(pstPictureQueue->astPicture[0]);
   pstPictureQueue->pstTailElement = &(pstPictureQueue->astPicture[0]);

   BDBG_LEAVE(BXDM_PP_XMO_S_Initialize);
   return;

} /* end of BXDM_PP_XMO_S_ResetCommon() */


/*
 *
 */
static void BXDM_PP_XMO_S_Increment_0BasedOffset_isr(
   uint32_t* pOffset,
   uint32_t  uiCount,
   uint32_t  uiMaxValue
   )
{
   uint32_t i;

   for ( i=0; i < uiCount; i++ )
   {
      /* If the offset is about to go off the end of the queue,
       * set it back to the beginning.  Otherwise just increment it.
       */
      if( *pOffset >= uiMaxValue - 1/*(BXVD_P_MAX_ELEMENTS_IN_DISPLAY_QUEUE - 1)*/ )
      {
         *pOffset = 0;
      }
      else
      {
         (*pOffset)++;
      }
   }

   return;
}

/*
 *
 */
static void BXDM_PP_XMO_S_Queue_InvalidateElement_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_PP_XDM_P_QueueElement * pstQueueElement
   )
{
   BDBG_ENTER(BXDM_PP_XMO_S_Queue_InvalidateElement_isr);

   BDBG_ASSERT(hXmo);
   BDBG_ASSERT(pstQueueElement);

   hXmo->stPictureQueue.iNumUsedElements--;

   /* The element count should never go below 0.
    * If it does, these is a bug in the logic.
    */
   BDBG_ASSERT(hXmo->stPictureQueue.iNumUsedElements >= 0);

   /* TODO: why not memset to 0?
    */
   pstQueueElement->bInUse = false;

   pstQueueElement->pstNextElement = NULL;
   pstQueueElement->pstPreviousElement = NULL;

   BDBG_LEAVE(BXDM_PP_XMO_S_Queue_InvalidateElement_isr);

   return;
}


/*
 * Returns the number of free elements on the picture queue.
 */
static uint32_t BXDM_PP_XMO_S_Queue_GetFreeElementCount_isr(
   BXDM_PictureProvider_XMO_Handle hXmo
   )
{
   uint32_t uiFreeCount;

   BDBG_ENTER( BXDM_PP_XMO_S_Queue_GetFreeElementCount_isr );

   uiFreeCount = BXDM_PP_XMO_P_QUEUE_DEPTH - hXmo->stPictureQueue.iNumUsedElements;

   BDBG_LEAVE( BXDM_PP_XMO_S_Queue_GetFreeElementCount_isr );

   return uiFreeCount;
}

/*
 * Returns the number of picture sets on the picture queue.
 */
static uint32_t BXDM_PP_XMO_S_Queue_GetPictureCount_isr(
   BXDM_PictureProvider_XMO_Handle hXmo
   )
{
   return hXmo->stPictureQueue.iNumPictures;
}

/* Gets the entry at the tail of the picture queue.
 * If the read offset is bumped, the picture is removed from the queue.
 * Will not be bumped for peeks, will be bumped for gets.
 * TODO: return a bool; false if the queue is empty?
 */
static void BXDM_PP_XMO_S_Queue_GetPicture_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_PP_XDM_P_QueueElement ** ppstQueueElement,
   bool bBumpReadOffset
   )
{
   BDBG_ENTER( BXDM_PP_XMO_S_Queue_GetPicture_isr );


   if ( true == bBumpReadOffset )
   {
      hXmo->stPictureQueue.iNumPictures--;
   }

   /* The picture count should never go below 0.
    * If it does, there is a bug in the logic.
    */
   BDBG_ASSERT( hXmo->stPictureQueue.iNumPictures >= 0 );

   BXDM_PP_XMO_S_Queue_GetLastElement_isr( hXmo, ppstQueueElement, bBumpReadOffset );

   BDBG_LEAVE( BXDM_PP_XMO_S_Queue_GetPicture_isr );

   return;

} /* end of BXDM_PP_XMO_S_Queue_GetPicture_isr() */

/*
 *
 */
static void BXDM_PP_XMO_S_Queue_GetLastElement_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_PP_XDM_P_QueueElement ** ppstQueueElement,
   bool bBumpReadOffset
   )
{
   BXDM_PP_XMO_P_PictureQueue * pstPictureQueue = &hXmo->stPictureQueue;

   BDBG_ENTER( BXDM_PP_XMO_S_Queue_GetLastElement_isr );

   BDBG_ASSERT( hXmo );
   BDBG_ASSERT( ppstQueueElement );

   /* TODO: what to return if the queue is empty? */

   *ppstQueueElement = pstPictureQueue->pstTailElement;

   if ( true == bBumpReadOffset )
   {
      /*BXDM_PP_XMO_S_Queue_DumpElement( *ppstQueueElement, false );*/

      /* Increment picture context queue read pointer */

      /* Move the tail pointer if there is another picture. */
      if ( NULL != pstPictureQueue->pstTailElement->pstNextElement )
      {
         pstPictureQueue->pstTailElement = pstPictureQueue->pstTailElement->pstNextElement;

         /* Do we need to Break the link between the elements?
          * Note: the element is actually release in the call to BXDM_PP_XMO_S_Queue_ReleasePicture_isr.
          * The picture is actually returned to XVD and the element freed in _ReleasePicture
          */
         pstPictureQueue->pstTailElement->pstPreviousElement->pstNextElement = NULL;
         pstPictureQueue->pstTailElement->pstPreviousElement->pstPreviousElement = NULL;
      }

   }

   BDBG_LEAVE( BXDM_PP_XMO_S_Queue_GetLastElement_isr );

   return;

}  /* end of BXDM_PP_XMO_S_Queue_GetLastElement_isr() */

/*
 * Sets the pointer to the head of the unified picture queue.
 */
static void BXDM_PP_XMO_S_Queue_GetNextFreeElement_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_PP_XDM_P_QueueElement ** ppstQueueElement
   )
{
   uint32_t uiElementsEvaluated=0;
   bool     bFoundFreeElement=false;

   BXDM_PP_XMO_P_PictureQueue * pstPictureQueue = &hXmo->stPictureQueue;

   BDBG_ENTER( BXDM_PP_XMO_S_Queue_GetNextFreeElement_isr );

   /* The call to "BXDM_PP_XMO_S_Queue_GetFreeElementCount_isr()" in "BXDM_PP_XMO_S_Queue_Update()"
    * should ensure that the queue has enough room. Keep this check as a safety valve.
    */
   if ( !BXDM_PP_XMO_S_Queue_GetFreeElementCount_isr( hXmo ) )
   {
      BXVD_DBG_ERR(hXmo, ("BXDM_PP_XMO_S_Queue_GetNextFreeElement_isr:: queue is full."));
      BDBG_ASSERT( 0 );
   }

   /* Set the pointer to the free element. */
   *ppstQueueElement = &(pstPictureQueue->astPicture[ pstPictureQueue->uiSearchIndex ]);

   /* Mark the entry as used. */
   (*ppstQueueElement)->bInUse = true;

   hXmo->stPictureQueue.iNumUsedElements++;
   BDBG_ASSERT( hXmo->stPictureQueue.iNumUsedElements <= BXDM_PP_XMO_P_QUEUE_DEPTH );

   /* uiElementsEvaluated To check if the code has looped around,
    * should not happen if "iNumUsedElements" is correct.
    */

   /* Step through the queue until the next free element is found. */

   while ( uiElementsEvaluated < BXDM_PP_XMO_P_QUEUE_DEPTH )
   {
      BXDM_PP_XMO_S_Increment_0BasedOffset_isr(
               &(pstPictureQueue->uiSearchIndex),
               1,
               BXDM_PP_XMO_P_QUEUE_DEPTH
               );

      if ( false == pstPictureQueue->astPicture[ pstPictureQueue->uiSearchIndex ].bInUse )
      {
         bFoundFreeElement = true;
         break;
      }


      uiElementsEvaluated++;
   }

   if ( true == bFoundFreeElement )
   {
      /* Set the head of the list to this new context. */
      pstPictureQueue->pstHeadElement = &(pstPictureQueue->astPicture[ pstPictureQueue->uiSearchIndex ]);

      /* for debug */
      pstPictureQueue->pstHeadElement->uiIndex = pstPictureQueue->uiSearchIndex;
   }
   else
   {
      /* The preceeding calls to "BXDM_PP_XMO_S_Queue_GetFreeElementCount_isr()"
       * should ensure that the queue has enough room. Keep this check as a safety valve.
       */
      BXVD_DBG_ERR(hXmo, ("BXDM_PP_XMO_S_Queue_GetNextFreeElement_isr:: did not find a free element."));
      BDBG_ASSERT( 0 );
   }

   /* Link this element to the preceeding. */
   pstPictureQueue->pstHeadElement->pstPreviousElement = *ppstQueueElement;

   /* Link the preceeding element to this one. */
   (*ppstQueueElement)->pstNextElement = pstPictureQueue->pstHeadElement;

   BDBG_LEAVE( BXDM_PP_XMO_S_Queue_GetNextFreeElement_isr );

   return;

}  /* end of BXDM_PP_XMO_S_Queue_GetNextFreeElement_isr() */


/*
 *
 */
static void BXDM_PP_XMO_S_Queue_ReleasePicture_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_PP_XDM_P_QueueElement * pstQueueElement
   )
{
   BDBG_ENTER( BXDM_PP_XMO_S_Queue_ReleasePicture_isr );

   BDBG_ASSERT( hXmo );
   BDBG_ASSERT( pstQueueElement );

   /*BXDM_PP_XMO_S_Queue_DumpElement( pstQueueElement, false );*/

   /* Release the unified picture(s) as well as the the associated PPB(s).
    * SWBLURAY-23835: Each picture should only be release once.  To
    * accommodate poorly behaving software, check "bInUse" to see if a
    * picture was previously released.
    */
   if ( true == pstQueueElement->bInUse )
   {
      /*
       * TODO: call decoder release routines here!!!!!!!!!!
       * What error checking will be needed?
       */
      /*BXVD_Decoder_S_ReleaseQ_AddPPB( hXmo, pstQueueElement->pPPBPhysical );*/

      /* TODO: does BXDM_Decoder_ReleasePictureInfo need to be anything besides NULL?
       * Will interface '1' ever be installed without interface '0'?
       * Should we only release pictures if the picture pointer is non-NULL?
       */

      if ( NULL != hXmo->astDecoderInterface[ BXDM_PP_XMO_Base_Index ].stInterface.releasePicture_isr
            && NULL != pstQueueElement->pstBasePicture
         )
      {
         hXmo->astDecoderInterface[ BXDM_PP_XMO_Base_Index ].stInterface.releasePicture_isr(
                  hXmo->astDecoderInterface[ BXDM_PP_XMO_Base_Index ].pPrivateContext,
                  pstQueueElement->pstBasePicture,
                  NULL
                  );

         hXmo->astStatus[ BXDM_PP_XMO_Base_Index ].uiNumPicturesReturned++;
      }

      if ( NULL != hXmo->astDecoderInterface[ BXDM_PP_XMO_Enhanced_Index ].stInterface.releasePicture_isr
            && NULL != pstQueueElement->pstEnhancedPicture
         )
      {
         hXmo->astDecoderInterface[ BXDM_PP_XMO_Enhanced_Index ].stInterface.releasePicture_isr(
                  hXmo->astDecoderInterface[ BXDM_PP_XMO_Enhanced_Index ].pPrivateContext,
                  pstQueueElement->pstEnhancedPicture,
                  NULL
                  );

         hXmo->astStatus[ BXDM_PP_XMO_Enhanced_Index ].uiNumPicturesReturned++;
      }


      BXDM_PP_XMO_S_Queue_InvalidateElement_isr( hXmo, pstQueueElement );
   }
   else
   {
      BXVD_DBG_ERR(hXmo,("BXDM_PP_XMO_S_Queue_ReleaseElement:: picture released multiple times."));
   }

   BDBG_LEAVE( BXDM_PP_XMO_S_Queue_ReleasePicture_isr );

   return;
}

static void BXDM_PP_XMO_S_Queue_AddPicture_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   const BXDM_Picture * pstBasePicture,
   const BXDM_Picture * pstEnhancedPicture
   )
{
   BXDM_PP_XDM_P_QueueElement * pstQueueElement=NULL;

   BDBG_ENTER( BXDM_PP_XMO_S_Queue_AddPicture_isr );

   /* Bump the count of sets of pictures on the Unified Queue when
    * the base picture is added.
    */
    hXmo->stPictureQueue.iNumPictures++;

   /* Get the BXDM_Picture element at the head of the unified queue.
    * The call to "BXDM_PP_XMO_S_Queue_GetFreeElementCount_isr()" in "BXDM_PP_XMO_S_Queue_Update()"
    * should ensure that the queue has enough room.
    */
   BXDM_PP_XMO_S_Queue_GetNextFreeElement_isr( hXmo, &pstQueueElement );

   /* This should never happen. We should only get here if there is a free
    * element on the Unified Queue.
    */
   if ( NULL == pstQueueElement )
   {
      BXVD_DBG_ERR(hXmo, ("BXDM_PP_XMO_S_Queue_AddPicture_isr:: failed to get an element."));
      BDBG_ASSERT( 0 );
   }
   else
   {
      /* Save the pointers to the base and enhanced pictures
       * for use in BXDM_PP_XMO_S_Queue_ReleasePicture_isr
       */
      pstQueueElement->pstBasePicture = pstBasePicture;
      pstQueueElement->pstEnhancedPicture = pstEnhancedPicture;

      /* Copy the base picture so that is can be modified.*/
      pstQueueElement->stBasePicture = *pstBasePicture;
      pstQueueElement->stBasePicture.hChannel = hXmo;

      /* Save the pointer to the queue element for use in BXDM_PP_XMO_S_Queue_ReleasePicture_isr */
      pstQueueElement->stBasePicture.pPrivateContext = pstQueueElement;

      /* Bind the enhanced picture to the base. */
      pstQueueElement->stBasePicture.pNextPicture = (BXDM_Picture * )pstEnhancedPicture;
   }

   BDBG_LEAVE( BXDM_PP_XMO_S_Queue_AddPicture_isr );

   return;

} /* end of BXDM_PP_XMO_S_Queue_AddPicture_isr() */


/*******************************************************************************
 *
 * End of local functions.
 *
 *******************************************************************************/


/********************************************************************************
 *
 * XMO Public API's
 *
 ********************************************************************************/

/*
 *
 */
BERR_Code BXDM_PictureProvider_XMO_SetDecoderInterface(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t uiIndex,
   BXDM_Decoder_Interface *pstDecoderInterface,
   void *pPrivateContext
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_XMO_SetDecoderInterface);

   BDBG_ASSERT( hXmo );
   BDBG_ASSERT( pstDecoderInterface );

   BDBG_MODULE_MSG( BXDM_XMOCTL, ("..._XMO_SetDecoderInterface: hXmo:0x%lu index:%d pstDecoderInterface:0x%lu  pPrivateContext:ox%lu",
               (unsigned long)hXmo,
               uiIndex,
               (unsigned long)pstDecoderInterface,
               (unsigned long)pPrivateContext
               ));

   hXmo->astDecoderInterface[ uiIndex ].stInterface = *pstDecoderInterface;
   hXmo->astDecoderInterface[ uiIndex ].pPrivateContext = pPrivateContext;

   BDBG_LEAVE(BXDM_PictureProvider_XMO_SetDecoderInterface);
   return BERR_TRACE( rc );
}

/*
 *
 */
BERR_Code BXDM_PictureProvider_XMO_StartDecode_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t uiIndex
   )
{
   BERR_Code rc=BERR_SUCCESS;

   BDBG_ENTER(BXDM_PictureProvider_XMO_StartDecode_isr);

   BDBG_ASSERT( hXmo );

   /* Note: since the system could be "holding onto the last picture",
    * the queues can not be reset in this routine.
    */

   if ( uiIndex >= BXDM_PP_XMO_P_Inputs_Max )
   {
      BXVD_DBG_ERR(hXmo, ("..._XMO_StartDecode_isr:: uiIndex of %d is greater than the max of %d.", uiIndex, BXDM_PP_XMO_P_Inputs_Max ));
      rc = BERR_INVALID_PARAMETER;
   }
   else if ( BXDM_PP_XMO_P_State_eStarted == hXmo->astStatus[ uiIndex ].eState )
   {
      BXVD_DBG_WRN(hXmo, ("..._XMO_StartDecode_isr:: channel %d has already been started.", uiIndex ));
   }
   else
   {
      BKNI_Memset( &(hXmo->astStatus[ uiIndex ]), 0, sizeof( BXDM_PP_XMO_P_Status ) );
      hXmo->astStatus[ uiIndex ].eState = BXDM_PP_XMO_P_State_eStarted;
      BDBG_MODULE_MSG( BXDM_XMOCTL, ("..._XMO_StartDecode_isr: start channel:%08x", uiIndex ));
   }

   BDBG_LEAVE(BXDM_PictureProvider_XMO_StartDecode_isr);

   return BERR_TRACE(rc);

} /* end of BXVD_P_DisplayManager_StartDecode() */

/*
 *
 */
BERR_Code BXDM_PictureProvider_XMO_StopDecode_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t uiIndex
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BXDM_PictureProvider_XMO_StopDecode_isr);

   BDBG_ASSERT( hXmo );

   if ( uiIndex >= BXDM_PP_XMO_P_Inputs_Max )
   {
      BXVD_DBG_ERR(hXmo, ("..._XMO_StopDecode_isr:: uiIndex of %d is greater than the max of %d.", uiIndex, BXDM_PP_XMO_P_Inputs_Max ));
      rc = BERR_INVALID_PARAMETER;
   }
   else if ( uiIndex != 0 )
   {
      BXVD_DBG_WRN(hXmo, ("..._XMO_StopDecode_isr:: called on channel %d, should only be called on channel 0.", uiIndex ));
      rc = BERR_INVALID_PARAMETER;
   }
   else if ( BXDM_PP_XMO_P_State_eStopped == hXmo->astStatus[ 0 ].eState )
   {
      BXVD_DBG_WRN(hXmo, ("..._XMO_StopDecode_isr:: channel %d has already been stopped.", uiIndex ));
   }
   else
   {
      BXDM_PP_XDM_P_QueueElement * pstQueueElement;
      uint32_t i;

      /* Mark all channels as stopped. */
      for ( i=0; i < BXDM_PP_XMO_P_Inputs_Max; i++ )
      {
         hXmo->astStatus[ i ].eState = BXDM_PP_XMO_P_State_eStopped;
         BDBG_MODULE_MSG( BXDM_XMOCTL, ("..._XMO_StopDecode_isr: stop channel:%08x", i ));
      }

      /* Release any pictures on the Picture Queue.  Note: XDM could still be
       * holding onto pictures, don't reset the queue.
       *
       * Should be call between:
       *    BXDM_PictureProvider_StopDecode_isr();
       *    and BXVD_Decoder_StopDecode_isr();
       */
      while ( BXDM_PP_XMO_S_Queue_GetPictureCount_isr( hXmo ) )
      {
         BXDM_PP_XMO_S_Queue_GetPicture_isr( hXmo, &pstQueueElement, true );
         BXDM_PP_XMO_S_Queue_ReleasePicture_isr( hXmo, pstQueueElement );
      }

   }

   BDBG_MODULE_MSG( BXDM_XMODBG, ("Qfree:%d B:(%d/%d) E:(%d/%d) ",
                        BXDM_PP_XMO_S_Queue_GetFreeElementCount_isr( hXmo ),
                        hXmo->astStatus[ BXDM_PP_XMO_Base_Index ].uiNumPicturesReceived,
                        hXmo->astStatus[ BXDM_PP_XMO_Base_Index ].uiNumPicturesReturned,
                        hXmo->astStatus[ BXDM_PP_XMO_Enhanced_Index ].uiNumPicturesReceived,
                        hXmo->astStatus[ BXDM_PP_XMO_Enhanced_Index ].uiNumPicturesReturned
                        ));


   BDBG_LEAVE(BXDM_PictureProvider_XMO_StopDecode_isr);

   return BERR_TRACE( rc );

} /* end of BXDM_PictureProvider_XMO_StopDecode_isr() */


/*
 *
 */
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXDM_PictureProvider_XMO_WatchdogReset_isr(
   BXDM_PictureProvider_XMO_Handle hXmo
   )
{
   BDBG_ENTER(BXDM_PictureProvider_XMO_WatchdogReset_isr);

   BSTD_UNUSED( hXmo );
#if 0
   BDBG_ASSERT( hXmo );

   /* Re-initialize the "stSelectedPicture" structure to be certain we don't try to do use the PPB pointers. */

   BKNI_Memset( &(hXmo->stDMState.stChannel.stSelectedPicture), 0, sizeof ( BXDM_PictureProvider_P_Picture_Context ));

   /* PR53039: also be certain that the PTS type is "invalid" */
   hXmo->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.ePtsType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS;

   hXmo->stDMState.stChannel.eDecodeState = BXDM_PictureProvider_P_DecodeState_eStopped;
#endif
   BDBG_LEAVE(BXDM_PictureProvider_XMO_WatchdogReset_isr);

   return BERR_TRACE( BERR_SUCCESS );
} /* end of BXDM_PictureProvider_XMO_WatchdogReset_isr() */
#endif


#if 0
BERR_Code
BXDM_PictureProvider_XMO_GetDefaultSettings(
         BXDM_PictureProvider_Settings *pstPictureProviderSettings
         )
{
   BDBG_ENTER( BXDM_PictureProvider_XMO_GetDefaultSettings );
   BDBG_ASSERT( pstPictureProviderSettings );
#if 0
   *pstPictureProviderSettings = s_stDefaultPictureProviderSettings;
#endif
   BDBG_LEAVE( BXDM_PictureProvider_XMO_GetDefaultSettings );

   return BERR_TRACE( BERR_SUCCESS );
}
#endif
/*
 * Handle any initialization that only needs to be done once
 * on "OpenChannel"?
 */
BERR_Code BXDM_PictureProvider_XMO_Create(
   BXDM_PictureProvider_XMO_Handle *phXmo
   )
{
   BXDM_PictureProvider_XMO_P_Context *pXmo = NULL;

   BDBG_ENTER( BXDM_PictureProvider_XMO_Create );

   BDBG_ASSERT( phXmo );

   /* Set the handle to NULL in case the allocation fails */
   *phXmo = NULL;

   pXmo = ( BXDM_PictureProvider_XMO_P_Context* ) BKNI_Malloc( sizeof( BXDM_PictureProvider_XMO_P_Context ) );
   if ( NULL == pXmo )
   {
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }

   /* Zero out the newly allocated context */
   BKNI_Memset( ( void * ) pXmo, 0, sizeof( BXDM_PictureProvider_XMO_P_Context ) );

   *phXmo = pXmo;

   BDBG_MODULE_MSG( BXDM_XMOCTL, ("..._XMO_Create: 0x%lu", (unsigned long)pXmo ));

   BDBG_LEAVE( BXDM_PictureProvider_XMO_Create );
   return BERR_TRACE( BERR_SUCCESS );
}

/*
 *
 */
BERR_Code BXDM_PictureProvider_XMO_Destroy(
   BXDM_PictureProvider_XMO_Handle hXmo
   )
{
   BERR_Code rc=BERR_SUCCESS;

   BDBG_ENTER( BXDM_PictureProvider_XMO_Destroy );

   BDBG_ASSERT(hXmo);

   if ( NULL != hXmo )
   {
      BDBG_MODULE_MSG( BXDM_XMOCTL, ("..._XMO_Destroy: 0x%lu", (unsigned long)hXmo ));
      BKNI_Free( hXmo );
   }
   else
   {
      BDBG_ERR(("..._XMO_Destroy:: hXmo == NULL."));
      rc = BERR_INVALID_PARAMETER;
   }

   BDBG_LEAVE( BXDM_PictureProvider_XMO_Destroy );

   return BERR_TRACE(rc);
}

/*
 * This is effectively "XMO_Open_Channel".
 */
BERR_Code BXDM_PictureProvider_XMO_GetDMInterface(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_Decoder_Interface *pstDecoderInterface,
   void **pPrivateContext
   )
{
   BDBG_ENTER( BXDM_PictureProvider_XMO_GetDMInterface );

   BDBG_ASSERT( hXmo );
   BDBG_ASSERT( pstDecoderInterface );
   BDBG_ASSERT( pPrivateContext );

   BDBG_MODULE_MSG( BXDM_XMOCTL, ("..._XMO_GetDMInterface: hXmo:0x%lu  pstDecoderInterface:0x%lu  pPrivateContext:0x%lu",
                                          (unsigned long)hXmo,
                                          (unsigned long)pstDecoderInterface,
                                          (unsigned long)pPrivateContext
                                          ));

   pstDecoderInterface->getPictureCount_isr = (BXDM_Decoder_GetPictureCount_isr) BXDM_PictureProvider_XMO_GetPictureCount_isr;
   pstDecoderInterface->peekAtPicture_isr = (BXDM_Decoder_PeekAtPicture_isr) BXDM_PictureProvider_XMO_PeekAtPicture_isr;
   pstDecoderInterface->getNextPicture_isr = (BXDM_Decoder_GetNextPicture_isr) BXDM_PictureProvider_XMO_GetNextPicture_isr;
   pstDecoderInterface->releasePicture_isr = (BXDM_Decoder_ReleasePicture_isr) BXDM_PictureProvider_XMO_ReleasePicture_isr;
   pstDecoderInterface->displayInterruptEvent_isr = (BXDM_Decoder_DisplayInterruptEvent_isr) BXDM_PictureProvider_XMO_DisplayInterruptEvent_isr;

   /*
    * TODO: how will drop requests be handled?
    */
#if 1
   pstDecoderInterface->requestPictureDrop_isr = NULL;
   pstDecoderInterface->getPictureDropPendingCount_isr = NULL;
#else
   pstDecoderInterface->requestPictureDrop_isr = (BXDM_Decoder_RequestPictureDrop_isr) BXDM_PictureProvider_XMO_RequestPictureDrop_isr;
   pstDecoderInterface->getPictureDropPendingCount_isr = (BXDM_Decoder_GetPictureDropPendingCount_isr) BXDM_PictureProvider_XMO_GetPictureDropPendingCount_isr;
#endif

   *pPrivateContext = hXmo;

   /* Should the queue be initialized at this time, in the create routine or in
    * BXDM_PictureProvider_XMO_SetDecoderInterface?   Does it matter?
    * This routine will be called just prior to registering the XMO with XDM.
    */
   BXDM_PP_XMO_S_Initialize( hXmo );

   BDBG_LEAVE( BXDM_PictureProvider_XMO_GetDMInterface );
   return BERR_TRACE( BERR_SUCCESS );

}  /* end of BXDM_PictureProvider_XMO_GetDMInterface */


/*******************************************************************************
 *
 * XMO BXDM_Decoder_Interface functions
 *
 *******************************************************************************/

/*
 *
 */
BERR_Code BXDM_PictureProvider_XMO_DisplayInterruptEvent_isr(
   BXDM_PictureProvider_XMO_Handle hXmo
   )
{
   uint32_t i;
   BERR_Code rc=BERR_SUCCESS;

   bool bInterface0Installed=false;
   bool bInterface1Installed=false;

   uint32_t uiChan0NumPictures=0;
   uint32_t uiChan1NumPictures=0;
   uint32_t uiNumFreeElements;

   BXDM_PP_XMO_DecoderInterface * pDecoder0=&(hXmo->astDecoderInterface[ BXDM_PP_XMO_Base_Index ]);
   BXDM_PP_XMO_DecoderInterface * pDecoder1=&(hXmo->astDecoderInterface[ BXDM_PP_XMO_Enhanced_Index ]);

   BDBG_ENTER( BXDM_PictureProvider_XMO_DisplayInterruptEvent_isr );

   BDBG_ASSERT(hXmo);

   /*
    * Initialize any per vsync local state.
    */

   /* Are all the required callbacks installed?
    * "releasePicture_isr" is not used in this routine, but it will be needed later.
    * Don't take any pictures if that can't be given back.
    */

   bInterface0Installed = ( NULL != pDecoder0->stInterface.getPictureCount_isr );
   bInterface0Installed |= ( NULL != pDecoder0->stInterface.getNextPicture_isr );
   bInterface0Installed |= ( NULL != pDecoder0->stInterface.peekAtPicture_isr );
   bInterface0Installed |= ( NULL != pDecoder0->stInterface.releasePicture_isr );
   bInterface0Installed |= ( NULL != pDecoder0->stInterface.displayInterruptEvent_isr );

   if ( false == bInterface0Installed )
   {
      BXVD_DBG_ERR(hXmo, ("..._XMO_DisplayInterruptEvent_isr:: interface 0 is not installed."));
      rc = BERR_UNKNOWN;
      goto Done;
   }

   bInterface1Installed = ( NULL != pDecoder1->stInterface.getPictureCount_isr );
   bInterface1Installed |= ( NULL != pDecoder1->stInterface.getNextPicture_isr );
   bInterface1Installed |= ( NULL != pDecoder1->stInterface.peekAtPicture_isr );
   bInterface1Installed |= ( NULL != pDecoder1->stInterface.releasePicture_isr );
   bInterface1Installed |= ( NULL != pDecoder1->stInterface.displayInterruptEvent_isr );

   /*
    * Turn the crank on any attached decoders.
    * Note: checked that displayInterruptEvent_isr was installed up above, reconcile this
    * duplicate code.
    */
   for ( i=0; i < BXDM_PP_XMO_P_Inputs_Max ; i++ )
   {
      if ( NULL != hXmo->astDecoderInterface[i].stInterface.displayInterruptEvent_isr )
      {
         hXmo->astDecoderInterface[i].stInterface.displayInterruptEvent_isr(
                     hXmo->astDecoderInterface[i].pPrivateContext
                     );
      }
   }

   /* Call each decoder and bind the pictures together.
    * Decoders will report zero pictures until they are ready.
    */

   /* A simple approach for now, just stick the pictues together. */
   /* If only one decoder registered, simply act as a pass through?
    * Might this be needed at startup or shutdown?
    */

   /* Call DisplayInterrupt routines for attached decoders. */

   /* Hold off processing until the queues have been intialized. */

   pDecoder0->stInterface.getPictureCount_isr(
                     pDecoder0->pPrivateContext,
                     &uiChan0NumPictures
                     );

   if ( NULL != pDecoder1->stInterface.getPictureCount_isr )
   {
      pDecoder1->stInterface.getPictureCount_isr(
                     pDecoder1->pPrivateContext,
                     &uiChan1NumPictures
                     );
   }

   uiNumFreeElements = BXDM_PP_XMO_S_Queue_GetFreeElementCount_isr( hXmo );

   BDBG_MODULE_MSG( BXDM_XMODBG, ("I:%d/%d Qfree:%d B:%d (%d/%d) E:%d (%d/%d) ",
                                       bInterface0Installed,
                                       bInterface1Installed,
                                       uiNumFreeElements,
                                       uiChan0NumPictures,
                                       hXmo->astStatus[ BXDM_PP_XMO_Base_Index ].uiNumPicturesReceived,
                                       hXmo->astStatus[ BXDM_PP_XMO_Base_Index ].uiNumPicturesReturned,
                                       uiChan1NumPictures,
                                       hXmo->astStatus[ BXDM_PP_XMO_Enhanced_Index ].uiNumPicturesReceived,
                                       hXmo->astStatus[ BXDM_PP_XMO_Enhanced_Index ].uiNumPicturesReturned
                                       ));

   /* TODO: add a warning if there are fewer free elements
    * than available pictures?
    */
   for ( i=0; i < uiChan0NumPictures && i < uiChan1NumPictures && i < uiNumFreeElements; i++ )
   {
      const BXDM_Picture * pstBasePicture;
      const BXDM_Picture * pstEnhancedPicture;

      uint32_t uiBasePts;
      uint32_t uiEnhancedPts;
      bool     bDropBase=false;
      bool     bDropEnhanced=false;
      int32_t  iPtsDiff=0;

      pDecoder0->stInterface.peekAtPicture_isr(
                     pDecoder0->pPrivateContext,
                     1,
                     &pstBasePicture
                     );

      pDecoder1->stInterface.peekAtPicture_isr(
                     pDecoder1->pPrivateContext,
                     1,
                     &pstEnhancedPicture
                     );

      uiBasePts = ( true== pstBasePicture->stPTS.bValid ) ? pstBasePicture->stPTS.uiValue : 0;
      uiEnhancedPts = ( true== pstEnhancedPicture->stPTS.bValid ) ? pstEnhancedPicture->stPTS.uiValue : 0;

      if ( uiBasePts < uiEnhancedPts )
      {
         iPtsDiff = uiEnhancedPts - uiBasePts;
         bDropBase = ( iPtsDiff >  375 ) ? true : false;
      }
      else if ( uiEnhancedPts < uiBasePts )
      {
         iPtsDiff = uiBasePts - uiEnhancedPts;
         bDropEnhanced = ( iPtsDiff >  375 ) ? true : false;
      }

      if ( true == bDropBase )
      {
         pDecoder0->stInterface.getNextPicture_isr(
                     pDecoder0->pPrivateContext,
                     &pstBasePicture
                     );

         pDecoder0->stInterface.releasePicture_isr(
                  pDecoder0->pPrivateContext,
                  pstBasePicture,
                  NULL
                  );

         hXmo->astStatus[ BXDM_PP_XMO_Base_Index ].uiNumPicturesReceived++;
         hXmo->astStatus[ BXDM_PP_XMO_Base_Index ].uiNumPicturesReturned++;

         BDBG_MODULE_MSG( BXDM_XMODBG, ("   Drop base: uiBasePts: %08x uiEnhancedPts: %08x",
                                       uiBasePts,
                                       uiEnhancedPts
                                       ));

      }
      else if (  true == bDropEnhanced )
      {
         pDecoder1->stInterface.getNextPicture_isr(
                     pDecoder1->pPrivateContext,
                     &pstEnhancedPicture
                     );


         pDecoder1->stInterface.releasePicture_isr(
                  pDecoder1->pPrivateContext,
                  pstEnhancedPicture,
                  NULL
                  );

         hXmo->astStatus[ BXDM_PP_XMO_Enhanced_Index ].uiNumPicturesReceived++;
         hXmo->astStatus[ BXDM_PP_XMO_Enhanced_Index ].uiNumPicturesReturned++;


         BDBG_MODULE_MSG( BXDM_XMODBG, ("   Drop enhanced: uiBasePts: %08x uiEnhancedPts: %08x",
                                       uiBasePts,
                                       uiEnhancedPts
                                       ));

      }
      else /* ( uiBasePts == uiEnhancedPts )*/
      {

         BDBG_MODULE_MSG( BXDM_XMODBG, ("   PTS match: iPtsDiff:%d  base:%08x  enhanced:%08x", iPtsDiff, uiBasePts, uiEnhancedPts ));

         pDecoder0->stInterface.getNextPicture_isr(
                     pDecoder0->pPrivateContext,
                     &pstBasePicture
                     );


         pDecoder1->stInterface.getNextPicture_isr(
                     pDecoder1->pPrivateContext,
                     &pstEnhancedPicture
                     );


         hXmo->astStatus[ BXDM_PP_XMO_Base_Index ].uiNumPicturesReceived++;
         hXmo->astStatus[ BXDM_PP_XMO_Enhanced_Index ].uiNumPicturesReceived++;

         BXDM_PP_XMO_S_Queue_AddPicture_isr(
                  hXmo,
                  pstBasePicture,
                  pstEnhancedPicture
                  );
      }
   }

Done:

   BDBG_LEAVE( BXDM_PictureProvider_XMO_DisplayInterruptEvent_isr );
   return BERR_TRACE( rc );

}  /* end of BXDM_PictureProvider_XMO_DisplayInterruptEvent_isr */

/*
 *
 */
BERR_Code BXDM_PictureProvider_XMO_GetPictureCount_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t *puiPictureCount
   )
{
   BDBG_ENTER( BXDM_PictureProvider_XMO_GetPictureCount_isr );

   /* The XVD decoder logic check "bDecoderHasBeenInitialized".
    * XMO should not need to do this, it will be implicit in the
    * count returned by the attached decoders.
    */

   *puiPictureCount = BXDM_PP_XMO_S_Queue_GetPictureCount_isr( hXmo );

   BDBG_LEAVE( BXDM_PictureProvider_XMO_GetPictureCount_isr );
   return BERR_TRACE( BERR_SUCCESS );

}  /* end of BXDM_PictureProvider_XMO_GetPictureCount_isr */


/*
 *
 */
BERR_Code BXDM_PictureProvider_XMO_PeekAtPicture_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t uiIndex,
   const BXDM_Picture **pUnifiedPicture
   )
{
   BXDM_PP_XDM_P_QueueElement * pstQueueElement;

   BDBG_ENTER( BXDM_PictureProvider_XMO_PeekAtPicture_isr );

   BSTD_UNUSED( uiIndex );

   BXDM_PP_XMO_S_Queue_GetPicture_isr( hXmo, &pstQueueElement, false );

   *pUnifiedPicture = &(pstQueueElement->stBasePicture);

   BDBG_LEAVE( BXDM_PictureProvider_XMO_PeekAtPicture_isr );
   return BERR_TRACE( BERR_SUCCESS );

}  /* end of BXDM_PictureProvider_XMO_PeekAtPicture_isr */

/*
 *
 */
BERR_Code BXDM_PictureProvider_XMO_GetNextPicture_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   const BXDM_Picture **pUnifiedPicture
   )
{
   BXDM_PP_XDM_P_QueueElement * pstQueueElement;

   BDBG_ENTER( BXDM_PictureProvider_XMO_GetNextPicture_isr );

   /*BXVD_Decoder_S_Printf("_GetNextPicture_isr:: ", NULL );*/

   BXDM_PP_XMO_S_Queue_GetPicture_isr( hXmo, &pstQueueElement, true );

   *pUnifiedPicture = &(pstQueueElement->stBasePicture);

   /*
    * Now that the PPB is being "processed", enqueue any user data.
    */

   /* For debug. */
   /*hXmo->stDecoderContext.stLogData.uiPicturesToPP++;*/

   BDBG_LEAVE( BXDM_PictureProvider_XMO_GetNextPicture_isr );
   return BERR_TRACE( BERR_SUCCESS );

}  /* end of BXDM_PictureProvider_XMO_GetNextPicture_isr */

/*
 *
 */
BERR_Code BXDM_PictureProvider_XMO_ReleasePicture_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_Picture *pUnifiedPicture,
   const BXDM_Decoder_ReleasePictureInfo *pReleasePictureInfo
   )
{
   BXDM_PP_XDM_P_QueueElement * pstQueueElement;

   BDBG_ENTER( BXDM_PictureProvider_XMO_ReleasePicture_isr );

   BSTD_UNUSED(pReleasePictureInfo);

   /*BXVD_Decoder_S_Printf("_ReleasePicture_isr:: ", NULL );*/

   pstQueueElement = pUnifiedPicture->pPrivateContext;

   if ( hXmo == pUnifiedPicture->hChannel )
   {
      BXDM_PP_XMO_S_Queue_ReleasePicture_isr( hXmo, pstQueueElement );
   }
   else if ( hXmo->astDecoderInterface[ BXDM_PP_XMO_Base_Index ].pPrivateContext == pUnifiedPicture->hChannel )
   {
      if ( NULL != hXmo->astDecoderInterface[ BXDM_PP_XMO_Base_Index ].stInterface.releasePicture_isr )
      {
         hXmo->astDecoderInterface[ BXDM_PP_XMO_Base_Index ].stInterface.releasePicture_isr(
                  hXmo->astDecoderInterface[ BXDM_PP_XMO_Base_Index ].pPrivateContext,
                  pUnifiedPicture,
                  NULL
                  );
      }
   }

   /* For debug. */
   /*hXmo->stDecoderContext.stLogData.uiPicturesFromPP++;*/

   BDBG_LEAVE( BXDM_PictureProvider_XMO_ReleasePicture_isr );
   return BERR_TRACE( BERR_SUCCESS );

}  /* end of BXDM_PictureProvider_XMO_ReleasePicture_isr */


/*
 * TODO: how to handle drop counts and requests?
 */
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXDM_PictureProvider_XMO_GetPictureDropPendingCount_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t *puiPictureDropPendingCount
   )
{
#if 0
   BXDM_PP_XDM_P_QueueElement * pstQueueElement;

   uint32_t uiNumToBeDropped;

   BXVD_AVD_P_GET_DROP_COUNT( hXmo, uiNumToBeDropped );
#endif
   BDBG_ENTER( BXDM_PictureProvider_XMO_GetPictureDropPendingCount_isr );

   BSTD_UNUSED( hXmo );
   BSTD_UNUSED( puiPictureDropPendingCount );
#if 0
   BDBG_ASSERT( hXmo );
   BDBG_ASSERT( puiPictureDropPendingCount );

   /* The "pending drop count" is difference between the number of pictures
    * that the Picture Provider has requested to be dropped and the number
    * that AVD has actually dropped.
    * When this value transistions from non-zero to zero, the Picture Provider
    * will transistion from VSYNC to TSM mode.  At this point, the PP will need
    * a valid coded PTS.  For this reason, "uiDropCount" is a sampled version of
    * "n_drop" which is only updated when a PPB contains a coded PTS.
    *
    * Never allow the pending drop count to be less than '0'.  If more pictures have been
    * dropped than was requested, return a pending drop of '0'.
    *
    * SW7405-3929/-3925: Check that there are pictures in the queue, before asking for the context
    * (contents of the returned context are invalid if no pictures available)
    * If this is not done, pending drops can "ping-pong" between current value
    * and zero, causing selection mode to ping-pong between vTSM and TSM mode
    */
   if ( BXDM_PP_XMO_S_Queue_GetPictureCount_isr(hXmo) )
   {
      BXDM_PP_XMO_S_Queue_GetPicture_isr( hXmo, &pstQueueElement, false );

      if ( uiNumToBeDropped < pstQueueElement->uiDropCount )
      {
         *puiPictureDropPendingCount = 0;
      }
      else
      {
         *puiPictureDropPendingCount = uiNumToBeDropped - pstQueueElement->uiDropCount;
      }
   }
   else
   {
      /* No pictures available in the unified Q, so we need to use previous data to supply the drop count ... */
      if ( uiNumToBeDropped < hXmo->stDecoderContext.uiDropCountSnapshot)
      {
         *puiPictureDropPendingCount = 0;
      }
      else
      {
         *puiPictureDropPendingCount = uiNumToBeDropped - hXmo->stDecoderContext.uiDropCountSnapshot;
      }
   }
#endif
   BDBG_LEAVE( BXDM_PictureProvider_XMO_GetPictureDropPendingCount_isr );
   return BERR_TRACE( BERR_SUCCESS );

}  /* end of BXDM_PictureProvider_XMO_GetPictureDropPendingCount_isr */
#endif /* !B_REFSW_MINIMAL SWSTB-461 */


/*
 *
 */
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXDM_PictureProvider_XMO_RequestPictureDrop_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   uint32_t *puiPictureDropRequestCount
   )
{
#if 0
   uint32_t uiDropsRequested = 0;

   uint32_t uiDropCount;

   BXVD_AVD_P_GET_DROP_COUNT( hXmo, uiDropCount );
#endif
   BDBG_ENTER( BXDM_PictureProvider_XMO_RequestPictureDrop_isr );

   BSTD_UNUSED( hXmo );
   BSTD_UNUSED( puiPictureDropRequestCount );

#if 0

   BDBG_ASSERT( hXmo );
   BDBG_ASSERT( puiPictureDropRequestCount );

   /* We only request more drops if the decoder has caught up with the previous drop request.
    */
   if ( uiDropCount <= hXmo->stDecoderContext.uiDropCountSnapshot )
   {
      if ( *puiPictureDropRequestCount > BXVD_DECODER_MAX_FIELDS_TO_DROP )
      {
         /* PR47521: We need to cap the max requested drop count to prevent scenarious where a
          * large drop@decode request would occur that would normally recover on it's own.
          * (e.g. a unmarked stream discontinuity or stream wrap.).
          * We really only want to do a drop@decode if the stream is muxed poorly and/or we're
          * trying to do STC fast forward trick modes.  In such scenarioes, if we still need
          * more drops to catch up, the Picture Provider can always request more after the
          * initial drop has occurred.
          */
         uiDropsRequested = BXVD_DECODER_MAX_FIELDS_TO_DROP;

      }
      else
      {
         /* The decoder drops 2 fields at a time; ensure that "uiPictureDropRequestCount"
          * is an even value.
          */
         uiDropsRequested = *puiPictureDropRequestCount & ~0x01;
      }

      uiDropCount += uiDropsRequested;
      BXVD_AVD_P_SET_DROP_COUNT( hXmo, uiDropCount );

   }

   /* SE7405-3925: Update the actual number of drops requested from the decoder, to allow the caller
      to accurately update the drop request count status ... */
   *puiPictureDropRequestCount = uiDropsRequested;
#endif
   BDBG_LEAVE( BXDM_PictureProvider_XMO_RequestPictureDrop_isr );
   return BERR_TRACE( BERR_SUCCESS );

}  /* end of BXDM_PictureProvider_XMO_RequestPictureDrop_isr */
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/*
 * Used to save the channel change settings at StartDecode time.
 */
BERR_Code BXDM_PictureProvider_XMO_SaveChannelChangeSettings_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_PictureProvider_ChannelChangeSettings * pstChannelChangeSettings
   )
{
   BERR_Code rc=BERR_SUCCESS;

   BDBG_ENTER( BXDM_PictureProvider_XMO_SaveChannelChangeSettings_isr );

   BDBG_ASSERT( hXmo );
   BDBG_ASSERT( pstChannelChangeSettings );

   hXmo->stSavedChannelChangeSettings = *pstChannelChangeSettings;
   BDBG_MODULE_MSG( BXDM_XMOCTL, ("..._XMO_SaveChannelChangeSettings_isr: hXmo:0x%lu  bHoldLastPicture:%d  bFirstPicturePreview:%d",
                                       (unsigned long)hXmo,
                                       hXmo->stSavedChannelChangeSettings.bHoldLastPicture,
                                       hXmo->stSavedChannelChangeSettings.bFirstPicturePreview
                                       ));

   BDBG_LEAVE( BXDM_PictureProvider_XMO_SaveChannelChangeSettings_isr );

   return BERR_TRACE( rc );
}

/*
 * Used to restore the channel change settings at StopDecode time.
 */
BERR_Code BXDM_PictureProvider_XMO_RetrieveChannelChangeSettings_isr(
   BXDM_PictureProvider_XMO_Handle hXmo,
   BXDM_PictureProvider_ChannelChangeSettings * pstChannelChangeSettings
   )
{
   BERR_Code rc=BERR_SUCCESS;
   BDBG_ENTER( BXDM_PictureProvider_XMO_RetrieveChannelChangeSettings_isr );

   BDBG_ASSERT( hXmo );
   BDBG_ASSERT( pstChannelChangeSettings );

   *pstChannelChangeSettings = hXmo->stSavedChannelChangeSettings;
   BDBG_MODULE_MSG( BXDM_XMOCTL, ("..._XMO_RetrieveChannelChangeSettings_isr: hXmo:0x%lu  bHoldLastPicture:%d  bFirstPicturePreview:%d",
                                       (unsigned long)hXmo,
                                       hXmo->stSavedChannelChangeSettings.bHoldLastPicture,
                                       hXmo->stSavedChannelChangeSettings.bFirstPicturePreview
                                       ));

   BDBG_LEAVE( BXDM_PictureProvider_XMO_RetrieveChannelChangeSettings_isr );

   return BERR_TRACE( rc );
}


/*******************************************************************************
 *
 * End BXDM_Decoder_Interface functions
 *
 *******************************************************************************/
