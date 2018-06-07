/***************************************************************************
 * Copyright (C) 2003-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"          /* For malloc */
#include "bkni_multi.h"    /* For mutex */

#include "blst_slist.h"

#include "bavc.h"
#include "bxdm.h"
#include "bxdm_dih.h"

BDBG_MODULE(BXDM_DIH);
BDBG_FILE_MODULE(BXDM_DIHCTL);
BDBG_FILE_MODULE(BXDM_DIHDBG);
BDBG_FILE_MODULE(BXDM_MFD1);

typedef struct BXDM_DisplayInterruptHandler_P_PictureProviderCallback
{
   BLST_S_ENTRY(BXDM_DisplayInterruptHandler_P_PictureProviderCallback) link;

   BXDM_DisplayInterruptHandler_PictureProvider_isr fCallback;
   void *pPrivateContext;
   uint32_t uiVDCRectangleNumber;
   BXDM_DisplayInterruptHandler_PictureProviderMode ePictureProviderMode; /* SW7405-3984: enabled or disabled */
} BXDM_DisplayInterruptHandler_P_PictureProviderCallback;

typedef struct BXDM_DisplayInterruptHandler_P_PictureDataReadyHandler
{
      BXDM_DisplayInterruptHandler_PictureDataReady_isr fCallback;
      void *pPrivateContext;
      int32_t iPrivateParam;
} BXDM_DisplayInterruptHandler_P_PictureDataReadyHandler;

typedef struct BXDM_DisplayInterruptHandler_P_DisplayHandler
{
      BXDM_DisplayInterruptHandler_Display_isr fCallback;
      void *pPrivateContext;
      int32_t iPrivateParam;
} BXDM_DisplayInterruptHandler_P_DisplayHandler;

typedef struct BXDM_DisplayInterruptHandler_P_Context
{
      BAVC_MFD_Picture stMFDPicture; /* Used if no picture providers exist */
      BXDM_DisplayInterruptHandler_P_PictureDataReadyHandler stPictureDataReadyHandler;
      BXDM_DisplayInterruptHandler_P_DisplayHandler stDisplayHandler;

      BKNI_MutexHandle hMutex; /* Mutex to lock access to PictureProviderList*/

      /* Picture Provider List */
      BLST_S_HEAD(BXDM_DisplayInterruptHandler_P_PictureProviderList, BXDM_DisplayInterruptHandler_P_PictureProviderCallback) hPictureProviderList;
} BXDM_DisplayInterruptHandler_P_Context;

BERR_Code
BXDM_DisplayInterruptHandler_Create(
         BXDM_DisplayInterruptHandler_Handle *phXdmDih
         )
{
   BXDM_DisplayInterruptHandler_P_Context *pXdmDih = NULL;

   BERR_Code rc;

   BDBG_ENTER( BXDM_DisplayInterruptHandler_Create );

   BDBG_ASSERT( phXdmDih );

   /* Set the handle to NULL in case the allocation fails */
   *phXdmDih = NULL;

   pXdmDih = ( BXDM_DisplayInterruptHandler_P_Context* ) BKNI_Malloc( sizeof( BXDM_DisplayInterruptHandler_P_Context ) );
   if ( NULL == pXdmDih )
   {
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }

   /* Zero out the newly allocated context */
   BKNI_Memset( ( void * ) pXdmDih, 0, sizeof( BXDM_DisplayInterruptHandler_P_Context ) );

   rc = BKNI_CreateMutex( &(pXdmDih->hMutex) );

   if ( BERR_SUCCESS != rc )
   {
      BKNI_Free( pXdmDih );
      return BERR_TRACE( rc );
   }


   BKNI_AcquireMutex( pXdmDih->hMutex );

   BLST_S_INIT( &pXdmDih->hPictureProviderList );

   BKNI_ReleaseMutex( pXdmDih->hMutex );

   *phXdmDih = pXdmDih;

   BDBG_MODULE_MSG( BXDM_DIHCTL, ("Create: hDih:0x%lu", (unsigned long) pXdmDih));

   BDBG_LEAVE( BXDM_DisplayInterruptHandler_Create );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BXDM_DisplayInterruptHandler_Destroy(
         BXDM_DisplayInterruptHandler_Handle hXdmDih
         )
{
   BXDM_DisplayInterruptHandler_P_PictureProviderCallback *pPictureProvider;

   BDBG_ENTER( BXDM_DisplayInterruptHandler_Destroy );

   if ( NULL != hXdmDih )
   {
      pPictureProvider = BLST_S_FIRST( &hXdmDih->hPictureProviderList );

      BKNI_AcquireMutex(  hXdmDih->hMutex );

      while ( NULL != pPictureProvider )
      {
         BLST_S_REMOVE_HEAD(
                  &hXdmDih->hPictureProviderList,
                  link);

         BKNI_Free(pPictureProvider);

         pPictureProvider = BLST_S_FIRST( &hXdmDih->hPictureProviderList );
      }

      BLST_S_INIT( &hXdmDih->hPictureProviderList );

      BKNI_ReleaseMutex( hXdmDih->hMutex );

      BKNI_DestroyMutex( hXdmDih->hMutex );

      BDBG_MODULE_MSG( BXDM_DIHCTL, ("Destroy: hDih:0x%lu", (unsigned long) hXdmDih));

      BKNI_Free( hXdmDih );
   }

   BDBG_LEAVE( BXDM_DisplayInterruptHandler_Destroy );

   return BERR_TRACE( BERR_SUCCESS );
}

static BXDM_DisplayInterruptHandler_P_PictureProviderCallback* BXDM_DisplayInterruptHandler_S_FindPictureProvider_isr(
         BXDM_DisplayInterruptHandler_Handle hXdmDih,
         const BXDM_DisplayInterruptHandler_PictureProvider_isr fCallback,
         const void *pPrivateContext
         )
{
   BXDM_DisplayInterruptHandler_P_PictureProviderCallback *pPictureProvider;

   BDBG_ENTER( BXDM_DisplayInterruptHandler_S_FindPictureProvider_isr );

   pPictureProvider = BLST_S_FIRST( &hXdmDih->hPictureProviderList );
   while ( NULL != pPictureProvider )
   {
      if ( ( pPictureProvider->fCallback == fCallback )
           && ( pPictureProvider->pPrivateContext == pPrivateContext ) )
      {
         return pPictureProvider;
      }

      pPictureProvider = BLST_S_NEXT( pPictureProvider, link );
   }

   BDBG_LEAVE( BXDM_DisplayInterruptHandler_S_FindPictureProvider_isr );

   return NULL;
}


BERR_Code
BXDM_DIH_AddPictureProviderInterface_GetDefaultSettings (
   BXDM_DisplayInterruptHandler_AddPictureProviderInterface_Settings *pstDefSettings
   )
{
   BDBG_ENTER( BXDM_DIH_AddPictureProviderInterface_GetDefaultSettings );

   BDBG_ASSERT(pstDefSettings);

   pstDefSettings->uiVDCRectangleNumber = 0;

   BDBG_LEAVE( BXDM_DIH_AddPictureProviderInterface_GetDefaultSettings );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BXDM_DisplayInterruptHandler_AddPictureProviderInterface(
         BXDM_DisplayInterruptHandler_Handle hXdmDih,
         BXDM_DisplayInterruptHandler_PictureProvider_isr fCallback_isr,
         void *pPrivateContext,
         BXDM_DisplayInterruptHandler_AddPictureProviderInterface_Settings *pDefSettings
         )
{
   BXDM_DisplayInterruptHandler_P_PictureProviderCallback *pPictureProvider;

   BDBG_ENTER( BXDM_DisplayInterruptHandler_AddPictureProviderInterface );

   BDBG_ASSERT( fCallback_isr );

   BKNI_EnterCriticalSection();

   pPictureProvider = BXDM_DisplayInterruptHandler_S_FindPictureProvider_isr(
               hXdmDih,
               fCallback_isr,
               pPrivateContext);

   BKNI_LeaveCriticalSection();

   if ( NULL == pPictureProvider )
   {
      pPictureProvider = ( BXDM_DisplayInterruptHandler_P_PictureProviderCallback* ) BKNI_Malloc( sizeof ( BXDM_DisplayInterruptHandler_P_PictureProviderCallback ) );
      if ( NULL == pPictureProvider )
      {
         return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
      }

      /* Zero out the newly allocated context */
      BKNI_Memset( ( void * ) pPictureProvider, 0, sizeof( BXDM_DisplayInterruptHandler_P_PictureProviderCallback) );

      pPictureProvider->fCallback = fCallback_isr;
      pPictureProvider->pPrivateContext = pPrivateContext;
      pPictureProvider->uiVDCRectangleNumber = pDefSettings->uiVDCRectangleNumber;

      /* SW7405-3984: mark the PictureProvider as enabled. */
      pPictureProvider->ePictureProviderMode = BXDM_DisplayInterruptHandler_PictureProviderMode_eEnabled;

      /* BLST list key's in descending order, needed to negate to get in ascending order */
      pPictureProvider->uiVDCRectangleNumber = (pPictureProvider->uiVDCRectangleNumber + 1) * (-1);

      BKNI_AcquireMutex( hXdmDih->hMutex );

      BDBG_MODULE_MSG( BXDM_DIHCTL, ("hDih:0x%lu pp:0x%lu add hXdm:0x%lu ch:%d",
                  (unsigned long)hXdmDih,
                  (unsigned long)pPictureProvider,
                  (unsigned long)pPrivateContext,
                  pDefSettings->uiVDCRectangleNumber
                  ));

      BLST_S_DICT_ADD(
               &hXdmDih->hPictureProviderList,
               pPictureProvider,
               BXDM_DisplayInterruptHandler_P_PictureProviderCallback,
               uiVDCRectangleNumber,
               link,
               err_duplicate);

      BKNI_ReleaseMutex( hXdmDih->hMutex );

   }
   else
   {
      BDBG_ERR(("Specified Picture Provider already exists"));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   BDBG_LEAVE( BXDM_DisplayInterruptHandler_AddPictureProviderInterface );

   return BERR_TRACE( BERR_SUCCESS );

err_duplicate:
   BKNI_Free( pPictureProvider );
   BKNI_ReleaseMutex( hXdmDih->hMutex );
   BDBG_ERR(("Specified Picture Provider already exists"));
   return BERR_TRACE( BERR_INVALID_PARAMETER );
}

BERR_Code
BXDM_DisplayInterruptHandler_RemovePictureProviderInterface(
         BXDM_DisplayInterruptHandler_Handle hXdmDih,
         BXDM_DisplayInterruptHandler_PictureProvider_isr fCallback_isr,
         void *pPrivateContext
         )
{
   BXDM_DisplayInterruptHandler_P_PictureProviderCallback *pPictureProvider;

   BDBG_ENTER( BXDM_DisplayInterruptHandler_RemovePictureProviderInterface );

   BKNI_EnterCriticalSection();

   /* Find the picture provider */
   pPictureProvider = BXDM_DisplayInterruptHandler_S_FindPictureProvider_isr(
            hXdmDih,
            fCallback_isr,
            pPrivateContext);

   BKNI_LeaveCriticalSection();

   if ( NULL != pPictureProvider )
   {
      BKNI_AcquireMutex( hXdmDih->hMutex );

      BDBG_MODULE_MSG( BXDM_DIHCTL, ("hDih:0x%lu pp:0x%lu remove hXdm:0x%lu",
                  (unsigned long)hXdmDih,
                  (unsigned long)pPictureProvider,
                  (unsigned long)pPrivateContext
                  ));

      BLST_S_REMOVE(
               &hXdmDih->hPictureProviderList,
               pPictureProvider,
               BXDM_DisplayInterruptHandler_P_PictureProviderCallback,
               link);

      BKNI_ReleaseMutex( hXdmDih->hMutex );

      BKNI_Free( pPictureProvider );
   }
   else
   {
      BDBG_ERR(("Specified Picture Provider could not be found"));
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   BDBG_LEAVE( BXDM_DisplayInterruptHandler_RemovePictureProviderInterface );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BXDM_DisplayInterruptHandler_Callback_isr(
         void *pPrivateContext,
         BXDM_DisplayInterruptInfo *pstDisplayInterruptInfo
         )
{
   BXDM_DisplayInterruptHandler_Handle hXdmDih = ( BXDM_DisplayInterruptHandler_Handle ) pPrivateContext;
   BAVC_MFD_Picture *pMFDPictureList = NULL;
   BAVC_MFD_Picture *pMFDPicture = NULL;
   BXDM_DisplayInterruptHandler_P_PictureProviderCallback *pPictureProvider;

   BDBG_ENTER( BXDM_DisplayInterruptHandler_Callback_isr );

   BDBG_ASSERT( hXdmDih );

   pPictureProvider = BLST_S_FIRST( &hXdmDih->hPictureProviderList );

   /* SW7405-3085: If we only have one picture provider outputting to this display,
    * we set the interrupt count to invalid so that the picture provider
    * does not use it to synchronize interlaced output on interlaced displays.
    *
    * For non-mosaic display, this will result in the best cadence output on
    * progressive displays.
    *
    * For mosaic display, we leave the interrupt count valid to ensure all
    * picture providers synchronize to the correct target polarity when on a
    * progressive display
    */
   if ( ( NULL != pPictureProvider )
        && ( NULL == BLST_S_NEXT( pPictureProvider, link ) )
      )
   {
      pstDisplayInterruptInfo->stInterruptCount.bValid = false;
   }

   while ( NULL != pPictureProvider )
   {

      BDBG_MODULE_MSG( BXDM_DIHDBG, ("hDih:0x%lu pp:0x%lu hXdm:0x%lu(%d) ch:%d %d",
                  (unsigned long)hXdmDih,
                  (unsigned long)pPictureProvider,
                  (unsigned long)pPictureProvider->pPrivateContext,
                  pPictureProvider->ePictureProviderMode,
                  ((pPictureProvider->uiVDCRectangleNumber * (-1)) - 1),
                  pstDisplayInterruptInfo->stInterruptCount.uiValue
                  ));

      /* SW7405-3984: only execute the callback if the PictureProvider is enabled.
       */
      if ( BXDM_DisplayInterruptHandler_PictureProviderMode_eEnabled == pPictureProvider->ePictureProviderMode )
      {
         pPictureProvider->fCallback(
               pPictureProvider->pPrivateContext,
               pstDisplayInterruptInfo,
               &pMFDPicture );

         /* Needed to negate to get in ascending order, now restore actual value */
         pMFDPicture->ulChannelId = (pPictureProvider->uiVDCRectangleNumber * (-1)) - 1;

         /* Append the returned picture list to the previous MFD Pictures */
         if ( NULL == pMFDPictureList )
         {
            pMFDPictureList = pMFDPicture;
         }
         else
         {
            BAVC_MFD_Picture *pMFDPictureLast = pMFDPictureList;

            /* Add this picture to the end of the current pMFDPictureList */
            while ( pMFDPictureLast->pNext != NULL )
            {
               pMFDPictureLast = pMFDPictureLast->pNext;
            }

            pMFDPictureLast->pNext = pMFDPicture;
            pMFDPicture->pNext = NULL;
         }
      }

      pPictureProvider = BLST_S_NEXT( pPictureProvider, link );
   }

   if ( NULL == pMFDPictureList )
   {
      hXdmDih->stMFDPicture.eInterruptPolarity = pstDisplayInterruptInfo->eInterruptPolarity;
      hXdmDih->stMFDPicture.bIgnorePicture = true;
      hXdmDih->stMFDPicture.bStallStc = true;
      hXdmDih->stMFDPicture.bMute = true;
      hXdmDih->stMFDPicture.bChannelChange = true; /* for SW7425-2253: */
      hXdmDih->stMFDPicture.pNext = NULL;

      pMFDPictureList = &hXdmDih->stMFDPicture;

#if BDBG_DEBUG_BUILD /* SW7445-1329: only compile for debug builds */
      {
         char cInterrtupPolarity;

         switch( pstDisplayInterruptInfo->eInterruptPolarity )
         {
            case 0:  cInterrtupPolarity = 'T';  break;
            case 1:  cInterrtupPolarity = 'B';  break;
            case 2:  cInterrtupPolarity = 'F';  break;
            default: cInterrtupPolarity = 'x';  break;
         }

         BDBG_MODULE_MSG( BXDM_MFD1, ("%c :[x.%03x] int:%c %s %s %s",
                                    ( hXdmDih->stMFDPicture.bMute ) ? 'M' : ' ',
                                    hXdmDih->stMFDPicture.ulDecodePictureId,
                                    cInterrtupPolarity,
                                    ( hXdmDih->stMFDPicture.bIgnorePicture ) ? "ip" : " ",
                                    ( hXdmDih->stMFDPicture.bStallStc ) ? "ss" : " ",
                                    ( hXdmDih->stMFDPicture.bChannelChange) ? "chg" : " "
                                    ));
      }
#endif

   }

   /* Execute the application's Picture Data Ready callback */
   if( NULL != hXdmDih->stPictureDataReadyHandler.fCallback )
   {
      hXdmDih->stPictureDataReadyHandler.fCallback(
               hXdmDih->stPictureDataReadyHandler.pPrivateContext,
               hXdmDih->stPictureDataReadyHandler.iPrivateParam,
               pMFDPictureList
               );
   }

   /* Execute the application's Display Interrupt Callback */
   if ( NULL != hXdmDih->stDisplayHandler.fCallback )
   {
      hXdmDih->stDisplayHandler.fCallback(
               hXdmDih->stDisplayHandler.pPrivateContext,
               hXdmDih->stDisplayHandler.iPrivateParam,
               pMFDPictureList
               );
   }

   BDBG_LEAVE( BXDM_DisplayInterruptHandler_Callback_isr );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BXDM_DisplayInterruptHandler_InstallCallback_PictureDataReadyInterrupt(
         BXDM_DisplayInterruptHandler_Handle hXdmDih,
         BXDM_DisplayInterruptHandler_PictureDataReady_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         )
{
   BDBG_ENTER( BXDM_DisplayInterruptHandler_InstallCallback_PictureDataReadyInterrupt );

   BDBG_ASSERT( hXdmDih );

   hXdmDih->stPictureDataReadyHandler.fCallback = fCallback_isr;
   hXdmDih->stPictureDataReadyHandler.pPrivateContext = pPrivateContext;
   hXdmDih->stPictureDataReadyHandler.iPrivateParam = iPrivateParam;

   BDBG_MODULE_MSG( BXDM_DIHCTL, ("hDih:0x%lu Install PDR:0x%lu context:0x%lu param:%d",
                  (unsigned long)hXdmDih,
                  (unsigned long)hXdmDih->stPictureDataReadyHandler.fCallback,
                  (unsigned long)hXdmDih->stPictureDataReadyHandler.pPrivateContext,
                  hXdmDih->stPictureDataReadyHandler.iPrivateParam
                  ));

   BDBG_LEAVE( BXDM_DisplayInterruptHandler_InstallCallback_PictureDataReadyInterrupt );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BXDM_DisplayInterruptHandler_UnInstallCallback_PictureDataReadyInterrupt(
         BXDM_DisplayInterruptHandler_Handle hXdmDih
         )
{
   BDBG_ENTER( BXDM_DisplayInterruptHandler_UnInstallCallback_PictureDataReadyInterrupt );

   BDBG_ASSERT( hXdmDih );

   BDBG_MODULE_MSG( BXDM_DIHCTL, ("hDih:0x%lu UnInstall PDR:0x%lu context:0x%lu param:%d",
                  (unsigned long)hXdmDih,
                  (unsigned long)hXdmDih->stPictureDataReadyHandler.fCallback,
                  (unsigned long)hXdmDih->stPictureDataReadyHandler.pPrivateContext,
                  hXdmDih->stPictureDataReadyHandler.iPrivateParam
                  ));

   hXdmDih->stPictureDataReadyHandler.fCallback = NULL;
   hXdmDih->stPictureDataReadyHandler.pPrivateContext = NULL;
   hXdmDih->stPictureDataReadyHandler.iPrivateParam = 0;

   BDBG_LEAVE( BXDM_DisplayInterruptHandler_UnInstallCallback_PictureDataReadyInterrupt );

   return BERR_TRACE( BERR_SUCCESS );
}


BERR_Code
BXDM_DisplayInterruptHandler_InstallCallback_DisplayInterrupt(
         BXDM_DisplayInterruptHandler_Handle hXdmDih,
         BXDM_DisplayInterruptHandler_Display_isr fCallback,
         void *pPrivateContext,
         int32_t iPrivateParam
         )
{
   BDBG_ENTER( BXDM_DisplayInterruptHandler_InstallCallback_DisplayInterrupt );

   BDBG_ASSERT( hXdmDih );

   hXdmDih->stDisplayHandler.fCallback = fCallback;
   hXdmDih->stDisplayHandler.pPrivateContext = pPrivateContext;
   hXdmDih->stDisplayHandler.iPrivateParam = iPrivateParam;

   BDBG_MODULE_MSG( BXDM_DIHCTL, ("hDih:0x%lu Install DisplayInterupt:0x%lu context:0x%lu param:%d",
                  (unsigned long)hXdmDih,
                  (unsigned long)hXdmDih->stDisplayHandler.fCallback,
                  (unsigned long)hXdmDih->stDisplayHandler.pPrivateContext,
                  hXdmDih->stDisplayHandler.iPrivateParam
                  ));

   BDBG_LEAVE( BXDM_DisplayInterruptHandler_InstallCallback_DisplayInterrupt );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BXDM_DisplayInterruptHandler_UnInstallCallback_DisplayInterrupt(
         BXDM_DisplayInterruptHandler_Handle hXdmDih
         )
{
   BDBG_ENTER( BXDM_DisplayInterruptHandler_UnInstallCallback_DisplayInterrupt );

   BDBG_ASSERT( hXdmDih );

   BDBG_MODULE_MSG( BXDM_DIHCTL, ("hDih:0x%lu UnInstall DisplayInterupt:0x%lu context:0x%lu param:%d",
                  (unsigned long)hXdmDih,
                  (unsigned long)hXdmDih->stDisplayHandler.fCallback,
                  (unsigned long)hXdmDih->stDisplayHandler.pPrivateContext,
                  hXdmDih->stDisplayHandler.iPrivateParam
                  ));

   hXdmDih->stDisplayHandler.fCallback = NULL;
   hXdmDih->stDisplayHandler.pPrivateContext = NULL;
   hXdmDih->stDisplayHandler.iPrivateParam = 0;

   BDBG_LEAVE( BXDM_DisplayInterruptHandler_UnInstallCallback_DisplayInterrupt );

   return BERR_TRACE( BERR_SUCCESS );
}

/* SW7405-3984: the Set/Get mode API's provide a way to disable
 * a PictureProvide (XDM) without unregistering it from the DIH.
 * When a PictureProvider is "disabled", the associated callback
 * will not be executed by in BXDM_DisplayInterruptHandler_Callback_isr.
 */
BERR_Code
BXDM_DisplayInterruptHandler_SetPictureProviderMode_isr(
   BXDM_DisplayInterruptHandler_Handle hXdmDih,
   BXDM_DisplayInterruptHandler_PictureProvider_isr fCallback_isr,
   void *pPrivateContext,
   BXDM_DisplayInterruptHandler_PictureProviderMode eMode
   )
{
   BXDM_DisplayInterruptHandler_P_PictureProviderCallback *pPictureProvider;

   BERR_Code rc=BERR_SUCCESS;

   BDBG_ENTER( BXDM_DisplayInterruptHandler_SetPictureProviderMode_isr );

   BDBG_ASSERT( hXdmDih );

   /* Find the picture provider */
   pPictureProvider = BXDM_DisplayInterruptHandler_S_FindPictureProvider_isr(
            hXdmDih,
            fCallback_isr,
            pPrivateContext);

   if ( NULL != pPictureProvider )
   {
      /* Set the mode. */
      pPictureProvider->ePictureProviderMode = eMode;

      BDBG_MODULE_MSG( BXDM_DIHCTL, ("SetPictureProviderMode:: hDih:0x%lu pp:0x%lu hXdm:0x%lu %s",
                  (unsigned long)hXdmDih,
                  (unsigned long)pPictureProvider,
                  (unsigned long)pPrivateContext,
                  ( pPictureProvider->ePictureProviderMode ) ? "enabled" : "disabled"
                  ));
   }
   else
   {
      BDBG_ERR(("BXDM_DisplayInterruptHandler_SetPictureProviderMode_isr:: Specified Picture Provider could not be found"));
      rc = BERR_INVALID_PARAMETER;
   }

   BDBG_LEAVE( BXDM_DisplayInterruptHandler_SetPictureProviderMode_isr );

   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_DisplayInterruptHandler_GetPictureProviderMode_isr(
   BXDM_DisplayInterruptHandler_Handle hXdmDih,
   BXDM_DisplayInterruptHandler_PictureProvider_isr fCallback,
   void *pPrivateContext,
   BXDM_DisplayInterruptHandler_PictureProviderMode * peMode
   )
{
   BXDM_DisplayInterruptHandler_P_PictureProviderCallback *pPictureProvider;

   BERR_Code rc=BERR_SUCCESS;

   BDBG_ENTER( BXDM_DisplayInterruptHandler_GetPictureProviderMode_isr );

   BDBG_ASSERT( hXdmDih );

   /* Find the picture provider */
   pPictureProvider = BXDM_DisplayInterruptHandler_S_FindPictureProvider_isr(
            hXdmDih,
            fCallback,
            pPrivateContext);

   if ( NULL != pPictureProvider )
   {
      /* Return the current mode. */
      *peMode = pPictureProvider->ePictureProviderMode;

      BDBG_MODULE_MSG( BXDM_DIHCTL, ("GetPictureProviderMode:: hDih:0x%lu pp:0x%lu hXdm:0x%lu mode:%d",
                  (unsigned long)hXdmDih,
                  (unsigned long)pPictureProvider,
                  (unsigned long)pPrivateContext,
                  pPictureProvider->ePictureProviderMode
                  ));
   }
   else
   {
      BDBG_ERR(("BXDM_DisplayInterruptHandler_GetPictureProviderMode_isr:: Specified Picture Provider could not be found"));
      rc = BERR_INVALID_PARAMETER;
   }

   BDBG_LEAVE( BXDM_DisplayInterruptHandler_GetPictureProviderMode_isr );

   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */
