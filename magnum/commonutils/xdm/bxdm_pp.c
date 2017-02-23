/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "btmr.h"

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"

BDBG_MODULE(BXDM_PP);

BERR_Code
BXDM_PictureProvider_SetDecoderInterface_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_Decoder_Interface *pstDecoderInterface,
         void *pPrivateContext
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetDecoderInterface_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstDecoderInterface );

   BDBG_MSG(("BXDM_PictureProvider_SetDecoderInterface_isr(0x%lu, 0x%lu, 0x%lu)",
               (unsigned long)hXdmPP,
               (unsigned long)pstDecoderInterface,
               (unsigned long)pPrivateContext
               ));

   hXdmPP->stDMConfig.stDecoderInterface = *pstDecoderInterface;
   hXdmPP->stDMConfig.pDecoderPrivateContext = pPrivateContext;

   BDBG_LEAVE(BXDM_PictureProvider_SetDecoderInterface_isr);
   return BERR_TRACE( rc );
}

/* SW7425-4630
 * The XDM filter provides a way for Nexus/middleware to fetch pictures from the XVD decoder
 * and process them in some manner.  The pictures can then be passed directly to VDC or sent
 * to XDM. The implementation of the filter itself and the associated picture queues is the
 * responsibility of Nexus/middleware.
 *
 * XDM interacts with the XVD Decoder by means of a number of callbacks that are defined in
 * the BXDM_Decoder_Interface structure. These callbacks allow XDM to get pictures from and
 * return pictures to the XVD Decoder. When an XVD channel is opened, the XVD Decoder registers
 * the callbacks with XDM by calling BXDM_PictureProvider_SetDecoderInterface_isr. In addition
 * to the callbacks, XVD also provides a handle to a private context. This handle is passed
 * back to XVD via the callbacks.
 *
 * The XDM filter uses this same callback mechanism. When a filter is installed, the XVD Decoder
 * callbacks are registered with the filter, the filter's callbacks are registered with XDM.
 * The filter also provides a handle to a private context which is passed in the callbacks.
 *
 * It should be noted that this mechanism supports installing an unlimited number of filters.
 * However with the current design, each new filter is added in front of the preceding filter.
 * The filters cannot be installed in a random order. Also, the filters must be uninstalled
 * in reverse from the order they were installed.
 */
BERR_Code
BXDM_PictureProvider_InstallFilter(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_Decoder_Interface *pstDecoderInterface,
   void *pPrivateFilterContext,
   BXDM_PictureProvider_FilterSettings *pstFilterSettings,
   BXDM_PictureProvider_FilterInterface **pstFilterInterface
   )
{
   BERR_Code rc=BERR_SUCCESS;

   bool bInterfaceError=false;

   BDBG_ENTER( BXDM_PictureProvider_InstallFilter );

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(pstDecoderInterface);
   BDBG_ASSERT(pstFilterInterface);
   BDBG_ASSERT(pstFilterSettings);

   BSTD_UNUSED(pstFilterSettings);

   /* a filter can only be installed when decode is stopped. */
   if ( hXdmPP->stDMState.stChannel.eDecodeState == BXDM_PictureProvider_P_DecodeState_eStarted )
   {
      BDBG_ERR(("BXDM_PictureProvider_InstallFilter_isr:: decoder is active, it must be stopped."));
      rc = BERR_INVALID_PARAMETER;
      goto Done;
   }

   /* Check that the filter supports all the correct methods.
    * TODO: is more validation required?
    */
   bInterfaceError = ( NULL == pstDecoderInterface->getPictureCount_isr );
   bInterfaceError |= ( NULL == pstDecoderInterface->peekAtPicture_isr );
   bInterfaceError |= ( NULL == pstDecoderInterface->getNextPicture_isr );
   bInterfaceError |= ( NULL == pstDecoderInterface->releasePicture_isr );
   bInterfaceError |= ( NULL == pstDecoderInterface->displayInterruptEvent_isr );

   /* TODO: need to support a flush or stop decode? */


   if ( true == bInterfaceError )
   {
      BDBG_ERR(("BXVD_InstallFilter:: filter interface not completely specified."));
      rc = BERR_INVALID_PARAMETER;
      goto Done;
   }


   /* install */
   if ( false == bInterfaceError )
   {
      /* Set the handle to NULL in case the allocation fails */
      BXDM_PictureProvide_P_FilterContext * pstNewFilter = NULL;

      pstNewFilter = ( BXDM_PictureProvide_P_FilterContext * ) BKNI_Malloc( sizeof( BXDM_PictureProvide_P_FilterContext ) );

      if ( NULL == pstNewFilter  )
      {
         return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
      }

      BKNI_Memset( pstNewFilter, 0, sizeof( BXDM_PictureProvide_P_FilterContext ) );

      BKNI_EnterCriticalSection();

      if ( NULL == hXdmPP->stDMConfig.pstFilterContext )
      {

         pstNewFilter->stFilterInterface.stInputInterface = hXdmPP->stDMConfig.stDecoderInterface;
         pstNewFilter->stFilterInterface.pUpStreamPrivateContext = hXdmPP->stDMConfig.pDecoderPrivateContext;
         pstNewFilter->stFilterInterface.hFilter = pstNewFilter;

         hXdmPP->stDMConfig.stDecoderInterface = *pstDecoderInterface;
         hXdmPP->stDMConfig.pDecoderPrivateContext = pPrivateFilterContext;
         hXdmPP->stDMConfig.pstFilterContext = pstNewFilter;

         *pstFilterInterface = &(pstNewFilter->stFilterInterface);


      }
      else
      {
         BXDM_PictureProvide_P_FilterContext * pstInstalledFilter;

         pstInstalledFilter = hXdmPP->stDMConfig.pstFilterContext;

         while ( NULL != pstInstalledFilter->pstUpSteamFilter )
         {
            pstInstalledFilter = pstInstalledFilter->pstUpSteamFilter;
         }

         pstNewFilter->stFilterInterface.stInputInterface = pstInstalledFilter->stFilterInterface.stInputInterface;
         pstNewFilter->stFilterInterface.pUpStreamPrivateContext = pstInstalledFilter->stFilterInterface.pUpStreamPrivateContext;
         pstNewFilter->stFilterInterface.hFilter = pstNewFilter;

         pstNewFilter->pstDownSteamFilter = pstInstalledFilter;

         pstInstalledFilter->stFilterInterface.stInputInterface =  *pstDecoderInterface;
         pstInstalledFilter->stFilterInterface.pUpStreamPrivateContext =  pPrivateFilterContext;
         pstInstalledFilter->pstUpSteamFilter = pstNewFilter;

         *pstFilterInterface = &(pstNewFilter->stFilterInterface);

      }

      BKNI_LeaveCriticalSection();


   }     /* end of if ( true == bInstallXmo ) */

Done:

   BDBG_LEAVE( BXDM_PictureProvider_InstallFilter );

   return  BERR_TRACE(rc);

}

BERR_Code
BXDM_PictureProvider_UninstallFilter(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_FilterInterface * pstFilter
   )
{
   BERR_Code rc=BERR_SUCCESS;

   BDBG_ENTER( BXDM_PictureProvider_UninstallFilter );

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(pstFilter);

   /* a filter can only be uninstalled when decode is stopped. */
   if ( hXdmPP->stDMState.stChannel.eDecodeState == BXDM_PictureProvider_P_DecodeState_eStarted )
   {
      BDBG_ERR(("BXVD_UninstallFilter:: decoder is active, it must be stopped."));
      rc = BERR_INVALID_PARAMETER;
      goto Done;
   }

   /* Bind the decoder to XDM (the Picture Provider)
    * TODO: does BXDM_PictureProvider_SetDecoderInterface_isr
    * need to be an ISR call?
    */
#if 0
   BXDM_PictureProvider_SetDecoderInterface_isr(
               (unsigned long)hXdmPP,
               &(pstFilter->stInputInterface),
               pstFilter->pDecoderPrivateContext
               );
#endif

   if ( NULL == hXdmPP->stDMConfig.pstFilterContext )
   {
      BDBG_ERR(("BXVD_UninstallFilter:: a filter is not installed."));
      rc = BERR_INVALID_PARAMETER;
      goto Done;
   }
   else if ( NULL == pstFilter->hFilter )
   {
      BDBG_ERR(("BXVD_UninstallFilter:: passed a bad filter handle."));
      rc = BERR_INVALID_PARAMETER;
      goto Done;
   }
   else if ( pstFilter->hFilter == hXdmPP->stDMConfig.pstFilterContext )
   {
      /* this is the filter connect to XDM. */

      /* check that this is the only filter installed. */
      if ( NULL != pstFilter->hFilter->pstDownSteamFilter )
      {
         BDBG_ERR(("BXVD_UninstallFilter:: filter is not at the head of the pipe."));
         rc = BERR_INVALID_PARAMETER;
         goto Done;
      }

      BKNI_EnterCriticalSection();

      hXdmPP->stDMConfig.stDecoderInterface = pstFilter->hFilter->stFilterInterface.stInputInterface;
      hXdmPP->stDMConfig.pDecoderPrivateContext = pstFilter->hFilter->stFilterInterface.pUpStreamPrivateContext;
      hXdmPP->stDMConfig.pstFilterContext = NULL;

      pstFilter->hFilter->pstDownSteamFilter = NULL;
      pstFilter->hFilter->pstUpSteamFilter = NULL;
      pstFilter->hFilter->stFilterInterface.pUpStreamPrivateContext = NULL;

      BKNI_LeaveCriticalSection();

      BKNI_Free( pstFilter->hFilter );


   }
   else
   {
      /* currently support uninstalling from the front of the pipe. */
      BXDM_PictureProvide_P_FilterContext * pstInstalledFilter;

      BKNI_EnterCriticalSection();

      /* start with the filter connected to XDM. */
      pstInstalledFilter = hXdmPP->stDMConfig.pstFilterContext;

      /* walk backwards through the pipe to find the filter connected
       * to the decoder.
       */
      while ( NULL != pstInstalledFilter->pstUpSteamFilter )
      {
         pstInstalledFilter = pstInstalledFilter->pstUpSteamFilter;
      }

      /* check that this is the filter that was requested. */
      if ( pstFilter->hFilter != pstInstalledFilter )
      {
         BDBG_ERR(("BXVD_UninstallFilter:: requested filter is not at the head of the pipe."));
         rc = BERR_INVALID_PARAMETER;

         BKNI_LeaveCriticalSection();

         goto Done;
      }

      /* now back up to the preceeding filter. */
      pstInstalledFilter = pstInstalledFilter->pstDownSteamFilter;

      pstInstalledFilter->stFilterInterface.stInputInterface = pstFilter->hFilter->stFilterInterface.stInputInterface;
      pstInstalledFilter->stFilterInterface.pUpStreamPrivateContext = pstFilter->hFilter->stFilterInterface.pUpStreamPrivateContext;
      pstInstalledFilter->pstUpSteamFilter = NULL;

      pstFilter->hFilter->pstDownSteamFilter = NULL;
      pstFilter->hFilter->pstUpSteamFilter = NULL;
      pstFilter->hFilter->stFilterInterface.pUpStreamPrivateContext = NULL;

      BKNI_LeaveCriticalSection();

      BKNI_Free( pstFilter->hFilter );

   }

Done:

   BDBG_LEAVE( BXDM_PictureProvider_UninstallFilter );

   return BERR_TRACE(rc);

}


#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetDefaultChannelChangeSettings(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_ChannelChangeSettings *pstChannelChangeSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetDefaultChannelChangeSettings_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstChannelChangeSettings );

   BSTD_UNUSED( hXdmPP );

   BKNI_Memset(pstChannelChangeSettings, 0, sizeof ( BXDM_PictureProvider_ChannelChangeSettings ));

   BDBG_LEAVE(BXDM_PictureProvider_GetDefaultChannelChangeSettings_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetChannelChangeSettings_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         const BXDM_PictureProvider_ChannelChangeSettings *pstChannelChangeSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetChannelChangeSettings_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstChannelChangeSettings );

   BDBG_MSG(("BXDM_PictureProvider_SetChannelChangeSettings_isr(0x%lu, hlp:%d, fpp: %d)",
                  (unsigned long)hXdmPP,
                  pstChannelChangeSettings->bHoldLastPicture,
                  pstChannelChangeSettings->bFirstPicturePreview
                  ));

   hXdmPP->stDMConfig.stChannelChangeSettings = *pstChannelChangeSettings;

   hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_CHANNEL_CHANGE_SETTINGS;

   BDBG_LEAVE(BXDM_PictureProvider_SetChannelChangeSettings_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetChannelChangeSettings_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_ChannelChangeSettings *pstChannelChangeSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetChannelChangeSettings_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstChannelChangeSettings );

   *pstChannelChangeSettings = hXdmPP->stDMConfig.stChannelChangeSettings;

   BDBG_LEAVE(BXDM_PictureProvider_GetChannelChangeSettings_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetMuteMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool bEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetMuteMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetMuteMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  bEnable
                  ));

   hXdmPP->stDMConfig.bMute = bEnable;

   hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_MUTE;

   BDBG_LEAVE(BXDM_PictureProvider_SetMuteMode_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetMuteMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool *pbEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetMuteMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pbEnable );

   *pbEnable = hXdmPP->stDMConfig.bMute;

   BDBG_LEAVE(BXDM_PictureProvider_GetMuteMode_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetDisplayFieldMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_DisplayFieldMode eDisplayFieldMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetDisplayFieldMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetDisplayFieldMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  eDisplayFieldMode
                  ));

   if ( eDisplayFieldMode >= BXDM_PictureProvider_DisplayFieldMode_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetDisplayFieldMode_isr:: eDisplayFieldMode value of %d is out of range", eDisplayFieldMode ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.eDisplayFieldMode = eDisplayFieldMode;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_DISPLAY_FIELD_MODE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetDisplayFieldMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetDisplayFieldMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_DisplayFieldMode *peDisplayFieldMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetDisplayFieldMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( peDisplayFieldMode );

   *peDisplayFieldMode = hXdmPP->stDMConfig.eDisplayFieldMode;

   BDBG_LEAVE(BXDM_PictureProvider_GetDisplayFieldMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetSourceFormatOverride_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_SourceFormatOverride eSourceFormatOverride
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetSourceFormatOverride_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetSourceFormatOverride_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  eSourceFormatOverride
                  ));

   if ( eSourceFormatOverride >= BXDM_PictureProvider_SourceFormatOverride_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetSourceFormatOverride_isr:: eSourceFormatOverride value of %d is out of range", eSourceFormatOverride ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.eSourceFormatOverride = eSourceFormatOverride;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_SRC_FORMAT_OVERRIDE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetSourceFormatOverride_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetSourceFormatOverride_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_SourceFormatOverride *peSourceFormatOverride
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetSourceFormatOverride_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( peSourceFormatOverride );

   *peSourceFormatOverride = hXdmPP->stDMConfig.eSourceFormatOverride;

   BDBG_LEAVE(BXDM_PictureProvider_GetSourceFormatOverride_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetScanModeOverride_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_ScanModeOverride eScanModeOverride
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetScanModeOverride_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetScanModeOverride_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  eScanModeOverride
                  ));

   if ( eScanModeOverride >= BXDM_PictureProvider_ScanModeOverride_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetScanModeOverride_isr:: eScanModeOverride value of %d is out of range", eScanModeOverride ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.eScanModeOverride = eScanModeOverride;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_SCAN_MODE_OVERRIDE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetScanModeOverride_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetScanModeOverride_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_ScanModeOverride *peScanModeOverride
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetScanModeOverride_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( peScanModeOverride );

   *peScanModeOverride = hXdmPP->stDMConfig.eScanModeOverride;

   BDBG_LEAVE(BXDM_PictureProvider_GetScanModeOverride_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetFreeze_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool bEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetFreeze_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetFreeze_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  bEnable
                  ));

   hXdmPP->stDMConfig.bFreeze = bEnable;

   hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_FREEZE;

   BDBG_LEAVE(BXDM_PictureProvider_SetFreeze_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetFreeze_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool *pbEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetFreeze_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pbEnable );

   *pbEnable = hXdmPP->stDMConfig.bFreeze;

   BDBG_LEAVE(BXDM_PictureProvider_GetFreeze_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_Set240iScanMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_240iScanMode e240iScanMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_Set240iScanMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_Set240iScanMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  e240iScanMode
                  ));
   if ( e240iScanMode >= BXDM_PictureProvider_240iScanMode_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_Set240iScanMode_isr:: e240iScanMode value of %d is out of range", e240iScanMode ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.e240iScanMode = e240iScanMode;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_240I_SCAN_MODE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_Set240iScanMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_Get240iScanMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_240iScanMode *pe240iScanMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_Get240iScanMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pe240iScanMode );

   *pe240iScanMode = hXdmPP->stDMConfig.e240iScanMode;

   BDBG_LEAVE(BXDM_PictureProvider_Get240iScanMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_Set480pPulldownMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_PulldownMode ePulldownMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_Set480pPulldownMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_Set480pPulldownMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  ePulldownMode
                  ));

   if ( ePulldownMode >= BXDM_PictureProvider_PulldownMode_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_Set480pPulldownMode_isr:: ePulldownMode value of %d is out of range", ePulldownMode ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.e480pPulldownMode = ePulldownMode;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_480P_PULLDOWN_MODE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_Set480pPulldownMode_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_Get480pPulldownMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_PulldownMode *pePulldownMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_Get480pPulldownMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pePulldownMode );

   *pePulldownMode = hXdmPP->stDMConfig.e480pPulldownMode;

   BDBG_LEAVE(BXDM_PictureProvider_Get480pPulldownMode_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_Set1080pPulldownMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_PulldownMode ePulldownMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_Set1080pPulldownMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_Set1080pPulldownMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  ePulldownMode
                  ));

   if ( ePulldownMode >= BXDM_PictureProvider_PulldownMode_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_Set1080pPulldownMode_isr:: ePulldownMode value of %d is out of range", ePulldownMode ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.e1080pPulldownMode = ePulldownMode;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_1080P_PULLDOWN_MODE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_Set1080pPulldownMode_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_Get1080pPulldownMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_PulldownMode *pePulldownMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_Get1080pPulldownMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pePulldownMode );

   *pePulldownMode = hXdmPP->stDMConfig.e1080pPulldownMode;

   BDBG_LEAVE(BXDM_PictureProvider_Get1080pPulldownMode_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetPTSOffset_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t uiOffset
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetPTSOffset_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetPTSOffset_isr(0x%lu, 0x%08x)",
                  (unsigned long)hXdmPP,
                  uiOffset
                  ));

   hXdmPP->stDMConfig.uiPTSOffset = uiOffset;

   hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_PTS_OFFSET;

   BDBG_LEAVE(BXDM_PictureProvider_SetPTSOffset_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetPTSOffset_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t *puiOffset
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetPTSOffset_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( puiOffset );

   *puiOffset = hXdmPP->stDMConfig.uiPTSOffset;

   BDBG_LEAVE(BXDM_PictureProvider_GetPTSOffset_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetDisplayMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_DisplayMode eDisplayMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetDisplayMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetDisplayMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  eDisplayMode
                  ));

   if ( eDisplayMode >= BXDM_PictureProvider_DisplayMode_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetDisplayMode_isr:: eDisplayMode value of %d is out of range", eDisplayMode ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.eDisplayMode = eDisplayMode;

      hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_DISPLAY_MODE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetDisplayMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetDisplayMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_DisplayMode *peDisplayMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetDisplayMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( peDisplayMode );

   *peDisplayMode = hXdmPP->stDMConfig.eDisplayMode;

   BDBG_LEAVE(BXDM_PictureProvider_GetDisplayMode_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetDefaultTSMThresholdSettings(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_TSMThresholdSettings *pstTSMThresholdSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetDefaultTSMThresholdSettings_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstTSMThresholdSettings );

   BSTD_UNUSED( hXdmPP );

   BKNI_Memset(pstTSMThresholdSettings, 0, sizeof ( BXDM_PictureProvider_TSMThresholdSettings ));

   BDBG_LEAVE(BXDM_PictureProvider_GetDefaultTSMThresholdSettings_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetTSMThresholdSettings_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         const BXDM_PictureProvider_TSMThresholdSettings *pstTSMThresholdSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetTSMThresholdSettings_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstTSMThresholdSettings );

   BDBG_MSG(("BXDM_PictureProvider_SetTSMThresholdSettings_isr(0x%lu, e:0x%08x, l:0x%08x, d:0x%08x)",
                  (unsigned long)hXdmPP,
                  pstTSMThresholdSettings->uiTooEarlyThreshold,
                  pstTSMThresholdSettings->uiTooLateThreshold,
                  pstTSMThresholdSettings->uiDeltaStcPtsDiffThreshold
                  ));

   hXdmPP->stDMConfig.stTSMThresholdSettings = *pstTSMThresholdSettings;

   hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_TSM_THRESHOLD;

   BDBG_LEAVE(BXDM_PictureProvider_SetTSMThresholdSettings_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetTSMThresholdSettings_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_TSMThresholdSettings *pstTSMThresholdSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetTSMThresholdSettings_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstTSMThresholdSettings );

   *pstTSMThresholdSettings = hXdmPP->stDMConfig.stTSMThresholdSettings;

   BDBG_LEAVE(BXDM_PictureProvider_GetTSMThresholdSettings_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetSTCValid_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool bValid
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetSTCValid_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetSTCValid_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  bValid
                  ));

   hXdmPP->stDMConfig.bSTCValid = bValid;

   hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_STC_VALID;

   BDBG_LEAVE(BXDM_PictureProvider_SetSTCValid_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetSTCValid_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool *pbValid
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetSTCValid_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pbValid );

   *pbValid = hXdmPP->stDMConfig.bSTCValid;

   BDBG_LEAVE(BXDM_PictureProvider_GetSTCValid_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetDefaultClipTimeSettings(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_ClipTimeSettings *pstClipTimeSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetDefaultClipTimeSettings_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstClipTimeSettings );

   BSTD_UNUSED( hXdmPP );

   BKNI_Memset(pstClipTimeSettings, 0, sizeof ( BXDM_PictureProvider_ClipTimeSettings ));

   BDBG_LEAVE(BXDM_PictureProvider_GetDefaultClipTimeSettings_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetClipTimeSettings_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         const BXDM_PictureProvider_ClipTimeSettings *pstClipTimeSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetClipTimeSettings_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstClipTimeSettings );

   BDBG_MSG(("BXDM_PictureProvider_SetClipTimeSettings_isr(0x%lu, v: %d, t:%d, 0x%08x-0x%08x, id:0x%08x)",
                  (unsigned long)hXdmPP,
                  pstClipTimeSettings->bValid,
                  pstClipTimeSettings->eType,
                  pstClipTimeSettings->uiStart,
                  pstClipTimeSettings->uiStop,
                  pstClipTimeSettings->uiId
                  ));

   hXdmPP->stDMConfig.stClipTimeSettings = *pstClipTimeSettings;

   hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_CLIP_TIMING;

   BDBG_LEAVE(BXDM_PictureProvider_SetClipTimeSettings_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetSoftwarePCROffset_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t uiOffset
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetSoftwarePCROffset_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetSoftwarePCROffset_isr(0x%lu, 0x%08x)",
                  (unsigned long)hXdmPP,
                  uiOffset
                  ));

   hXdmPP->stDMConfig.uiSoftwarePCROffset = uiOffset;

   hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_SOFTWARE_PCR_OFFSET;

   BDBG_LEAVE(BXDM_PictureProvider_SetSoftwarePCROffset_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetSoftwarePCROffset_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t *puiOffset
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetSoftwarePCROffset_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( puiOffset );

   *puiOffset = hXdmPP->stDMConfig.uiSoftwarePCROffset;

   BDBG_LEAVE(BXDM_PictureProvider_GetSoftwarePCROffset_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetHardwarePCROffsetMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool bEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetHardwarePCROffsetMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetHardwarePCROffsetMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  bEnable
                  ));

   hXdmPP->stDMConfig.bUseHardwarePCROffset = bEnable;

   hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_USE_HW_PCR_OFFSET;

   BDBG_LEAVE(BXDM_PictureProvider_SetHardwarePCROffsetMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetHardwarePCROffsetMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool *pbEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetHardwarePCROffsetMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pbEnable );

   *pbEnable = hXdmPP->stDMConfig.bUseHardwarePCROffset;

   BDBG_LEAVE(BXDM_PictureProvider_GetHardwarePCROffsetMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetPlaybackRate_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         const BXDM_Picture_Rate *pstPlaybackRate
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetPlaybackRate_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstPlaybackRate );

   BDBG_MSG(("BXDM_PictureProvider_SetPlaybackRate_isr(0x%lu, %d/%d)",
                  (unsigned long)hXdmPP,
                  pstPlaybackRate->uiNumerator,
                  pstPlaybackRate->uiDenominator
                  ));

   /* Prevent a divide by '0'.*/
   if ( 0 == pstPlaybackRate->uiDenominator )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetPlaybackRate_isr:: uiDenominator == 0!"));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.stPlaybackRate = *pstPlaybackRate;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_PLAYBACK_RATE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetPlaybackRate_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetPlaybackRate_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_Picture_Rate *pstPlaybackRate
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetPlaybackRate_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstPlaybackRate );

   *pstPlaybackRate = hXdmPP->stDMConfig.stPlaybackRate;

   BDBG_LEAVE(BXDM_PictureProvider_GetPlaybackRate_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetPictureDropMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_PictureDropMode ePictureDropMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetPictureDropMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetPictureDropMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  ePictureDropMode
                  ));
   if ( ePictureDropMode >= BXDM_PictureProvider_PictureDropMode_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetPictureDropMode_isr:: ePictureDropMode value of %d is out of range", ePictureDropMode ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.ePictureDropMode = ePictureDropMode;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_PICTURE_DROP_MODE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetPictureDropMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetPictureDropMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_PictureDropMode *pePictureDropMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetPictureDropMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pePictureDropMode );

   *pePictureDropMode = hXdmPP->stDMConfig.ePictureDropMode;

   BDBG_LEAVE(BXDM_PictureProvider_GetPictureDropMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetCounters_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Counters *pstCounters
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetCounters_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstCounters );

   *pstCounters = hXdmPP->stDMStatus.stCounters;

   BDBG_LEAVE(BXDM_PictureProvider_GetCounters_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetCurrentPTSInfo_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_PTSInfo *pPTSInfo
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetCurrentPTSInfo_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pPTSInfo );

   *pPTSInfo = hXdmPP->stDMStatus.stCurrentPTS;

   BDBG_LEAVE(BXDM_PictureProvider_GetCurrentPTSInfo_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetLastCodedPTSInfo_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_PTSInfo *pPTSInfo
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetLastCodedPTSInfo_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pPTSInfo );

   *pPTSInfo = hXdmPP->stDMStatus.stCodedPTS;

   BDBG_LEAVE(BXDM_PictureProvider_GetLastCodedPTSInfo_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetNextPTSInfo_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_PTSInfo *pPTSInfo
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetNextPTSInfo_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pPTSInfo );

   if ( true == hXdmPP->stDMStatus.bNextPTSInfoValid )
   {
      *pPTSInfo = hXdmPP->stDMStatus.stNextPTS;
      rc = BERR_SUCCESS;
   }
   else
   {
      rc = BERR_UNKNOWN;
   }

   BDBG_LEAVE(BXDM_PictureProvider_GetNextPTSInfo_isr);
   return rc;
}

BERR_Code
BXDM_PictureProvider_GetIPictureFoundStatus_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool *pbFound
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetIPictureFoundStatus_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pbFound );

   *pbFound = hXdmPP->stDMStatus.bIPictureFound;

   BDBG_LEAVE(BXDM_PictureProvider_GetIPictureFoundStatus_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetPictureTag_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t *puiValue
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetPictureTag_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( puiValue );

   *puiValue = hXdmPP->stDMStatus.uiPictureTag;

   BDBG_LEAVE(BXDM_PictureProvider_GetPictureTag_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetCurrentTimeCode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_Picture_GopTimeCode *pstTimeCode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetCurrentTimeCode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstTimeCode );

   *pstTimeCode = hXdmPP->stDMStatus.stGOPTimeCode;

   BDBG_LEAVE(BXDM_PictureProvider_GetCurrentTimeCode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetCurrentPicture_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_Picture *pPicture
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetCurrentPicture_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pPicture );

   if ( true == hXdmPP->stDMStatus.bCurrentPPBParameterInfoValid )
   {
      *pPicture = hXdmPP->stDMStatus.stCurrentPPBParameterInfo;
      rc = BERR_SUCCESS;
   }
   else
   {
      rc = BERR_UNKNOWN;
   }

   BDBG_LEAVE(BXDM_PictureProvider_GetCurrentPicture_isr);
   return rc;
}

/* SW7425-3558: return a pointer to "stCurrentPPBParameterInfo",
 * the picture currently selected for display.
 * In contract to BXDM_PictureProvider_GetCurrentPicture_isr, using
 * this routine saves both a copy operation and the need for the
 * calling routine to have a local BXDM_Picture structure on the stack.
 * It also provides a way to access the linked picture of a
 * pair of pictures (say for MVC).
 * If the data isn't valid, "pPicture" is set to NULL.
 */
BERR_Code
BXDM_PictureProvider_GetCurrentPicturePtr_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         const BXDM_Picture **pPicture
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetCurrentPicture_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pPicture );

   if ( true == hXdmPP->stDMStatus.bCurrentPPBParameterInfoValid )
   {
      *pPicture = &(hXdmPP->stDMStatus.stCurrentPPBParameterInfo);
   }
   else
   {
      *pPicture = NULL;
   }

   BDBG_LEAVE(BXDM_PictureProvider_GetCurrentPicture_isr);
   return rc;
}

BERR_Code
BXDM_PictureProvider_SetFrameAdvanceMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_FrameAdvanceMode eFrameAdvanceMode
      )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetFrameAdvanceMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetFrameAdvanceMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  eFrameAdvanceMode
                  ));
   if ( eFrameAdvanceMode >= BXDM_PictureProvider_FrameAdvanceMode_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetFrameAdvanceMode_isr:: eFrameAdvanceMode value of %d is out of range", eFrameAdvanceMode ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.eFrameAdvanceMode = eFrameAdvanceMode;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_FRAME_ADVANCE_MODE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetFrameAdvanceMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetDefaultPreserveStateSettings_isrsafe(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_PreserveStateSettings *pstPreserveStateSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetDefaultPreserveStateSettings_isrsafe);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstPreserveStateSettings );

   BSTD_UNUSED( hXdmPP );

   BKNI_Memset(pstPreserveStateSettings, 0, sizeof ( BXDM_PictureProvider_PreserveStateSettings ));

   BDBG_LEAVE(BXDM_PictureProvider_GetDefaultPreserveStateSettings_isrsafe);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetPreserveStateSettings_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         const BXDM_PictureProvider_PreserveStateSettings *pstPreserveStateSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetPreserveStateSettings_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstPreserveStateSettings );

   BDBG_MSG(("BXDM_PictureProvider_SetPreserveStateSettings_isr(0x%lu, d:%d, c:%d)",
                  (unsigned long)hXdmPP,
                  pstPreserveStateSettings->bDisplay,
                  pstPreserveStateSettings->bCounters
                  ));

   hXdmPP->stDMConfig.stPreserveStateSettings = *pstPreserveStateSettings;

   hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_PRESERVE_SETTINGS;

   BDBG_LEAVE(BXDM_PictureProvider_SetPreserveStateSettings_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetPreserveStateSettings_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_PreserveStateSettings *pstPreserveStateSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetPreserveStateSettings_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstPreserveStateSettings );

   *pstPreserveStateSettings = hXdmPP->stDMConfig.stPreserveStateSettings;

   BDBG_LEAVE(BXDM_PictureProvider_GetPreserveStateSettings_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetReverseFieldsMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool bEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetReverseFieldsMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetReverseFieldsMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  bEnable
                  ));

   hXdmPP->stDMConfig.bReverseFields = bEnable;

   hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_REVERSE_FIELDS;

   BDBG_LEAVE(BXDM_PictureProvider_SetReverseFieldsMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetReverseFieldsMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool *pbEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetReverseFieldsMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pbEnable );

   *pbEnable = hXdmPP->stDMConfig.bReverseFields;

   BDBG_LEAVE(BXDM_PictureProvider_GetReverseFieldsMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetAutoValidateStcOnPauseMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool bEnable
      )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetAutoValidateStcOnPauseMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetAutoValidateStcOnPauseMode_isr(0x%lu, %d",
                  (unsigned long)hXdmPP,
                  bEnable
                  ));

   hXdmPP->stDMConfig.bAutoValidateStcOnPause = bEnable;

   hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_AUTO_VALIDATE_ON_PAUSE;

   BDBG_LEAVE(BXDM_PictureProvider_SetAutoValidateStcOnPauseMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetAutoValidateStcOnPauseMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool *pbEnable
      )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetAutoValidateStcOnPauseMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pbEnable );

   *pbEnable = hXdmPP->stDMConfig.bAutoValidateStcOnPause;

   BDBG_LEAVE(BXDM_PictureProvider_GetAutoValidateStcOnPauseMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetProtocol_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BAVC_VideoCompressionStd eProtocol
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetProtocol_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetProtocol_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  eProtocol
                  ));

   if ( eProtocol >= BAVC_VideoCompressionStd_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetProtocol_isr:: eProtocol value of %d is out of range", eProtocol ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.eProtocol = eProtocol;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_PROTOCOL;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetProtocol_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetProtocol_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BAVC_VideoCompressionStd *peProtocol
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetProtocol_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( peProtocol );

   *peProtocol = hXdmPP->stDMConfig.eProtocol;

   BDBG_LEAVE(BXDM_PictureProvider_GetProtocol_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetJitterToleranceImprovementMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool bEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetJitterToleranceImprovementMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetJitterToleranceImprovementMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  bEnable
                  ));

   hXdmPP->stDMConfig.bJitterToleranceImprovement = bEnable;

   hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_JITTER_TOLERANCE;

   BDBG_LEAVE(BXDM_PictureProvider_SetJitterToleranceImprovementMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetJitterToleranceImprovementMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool *pbEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetJitterToleranceImprovementMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pbEnable );

   *pbEnable = hXdmPP->stDMConfig.bJitterToleranceImprovement;

   BDBG_LEAVE(BXDM_PictureProvider_GetJitterToleranceImprovementMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetMonitorRefreshRate_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_MonitorRefreshRate eMonitorRefreshRate
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetMonitorRefreshRate_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetMonitorRefreshRate_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  eMonitorRefreshRate
                  ));

   /* Determine the monitor refresh rate */
   if ( eMonitorRefreshRate == BXDM_PictureProvider_MonitorRefreshRate_eUnknown
         || eMonitorRefreshRate >= BXDM_PictureProvider_MonitorRefreshRate_eMax
      )
   {
      BXVD_DBG_WRN(hXdmPP,("Monitor Refresh Rate Override: Unsupported(%d->%d)", eMonitorRefreshRate, BXDM_PictureProvider_MonitorRefreshRate_e59_94Hz));
      hXdmPP->stDMConfig.eMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e59_94Hz;
   }
   else
   {
      hXdmPP->stDMConfig.eMonitorRefreshRate = eMonitorRefreshRate;
   }

   hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_MONITOR_REFRESH_RATE;

   BDBG_LEAVE(BXDM_PictureProvider_SetMonitorRefreshRate_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetMonitorRefreshRate_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_MonitorRefreshRate *peMonitorRefreshRate
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetMonitorRefreshRate_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( peMonitorRefreshRate );

   /* Check for NULL handle */
   if (hXdmPP != NULL)
   {
      *peMonitorRefreshRate = hXdmPP->stDMConfig.eMonitorRefreshRate;
   }
   else
   {
      rc = BERR_INVALID_PARAMETER;
   }

   BDBG_LEAVE(BXDM_PictureProvider_GetMonitorRefreshRate_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_Set1080pScanMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_1080pScanMode eScanMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_Set1080pScanMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_Set1080pScanMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  eScanMode
                  ));

   if ( eScanMode >= BXDM_PictureProvider_1080pScanMode_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_Set1080pScanMode_isr:: eScanMode value of %d is out of range", eScanMode ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.e1080pScanMode = eScanMode;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_1080P_SCAN_MODE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_Set1080pScanMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_Get1080pScanMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_1080pScanMode *peScanMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_Get1080pScanMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( peScanMode );

   *peScanMode = hXdmPP->stDMConfig.e1080pScanMode;

   BDBG_LEAVE(BXDM_PictureProvider_Get1080pScanMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetSTCIndex_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t uiValue
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetSTCIndex_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetSTCIndex_isr(0x%lu, 0x%08x)",
                  (unsigned long)hXdmPP,
                  uiValue
                  ));

   hXdmPP->stDMConfig.uiSTCIndex = uiValue;

   hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_STC_INDEX;

   BDBG_LEAVE(BXDM_PictureProvider_SetSTCIndex_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetSTCIndex_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t *puiValue
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetSTCIndex_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( puiValue );

   *puiValue = hXdmPP->stDMConfig.uiSTCIndex;

   BDBG_LEAVE(BXDM_PictureProvider_GetSTCIndex_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetCRCMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool bEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetCRCMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetCRCMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  bEnable
                  ));

   hXdmPP->stDMConfig.bCRCMode= bEnable;

   BDBG_LEAVE(BXDM_PictureProvider_SetCRCMode_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetCRCMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool *pbEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetCRCMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pbEnable );

   *pbEnable = hXdmPP->stDMConfig.bCRCMode;

   BDBG_LEAVE(BXDM_PictureProvider_GetCRCMode_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetRemovalDelay_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t uiValue
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetRemovalDelay_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetRemovalDelay_isr(0x%lu, 0x%08x)",
                  (unsigned long)hXdmPP,
                  uiValue
                  ));

   hXdmPP->stDMConfig.uiRemovalDelay = uiValue;

   hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_REMOVAL_DELAY;

   BDBG_LEAVE(BXDM_PictureProvider_SetRemovalDelay_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetRemovalDelay_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t *puiValue
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetRemovalDelay_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( puiValue );

   *puiValue = hXdmPP->stDMConfig.uiRemovalDelay;

   BDBG_LEAVE(BXDM_PictureProvider_GetRemovalDelay_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetPreRollRate_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         const BXDM_Picture_Rate *pstPreRollRate
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetPreRollRate_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstPreRollRate );

   BDBG_MSG(("BXDM_PictureProvider_SetPreRollRate_isr(0x%lu, %d/%d)",
                  (unsigned long)hXdmPP,
                  pstPreRollRate->uiNumerator,
                  pstPreRollRate->uiDenominator
                  ));

   if ( 0 == pstPreRollRate->uiDenominator )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetPreRollRate_isr:: uiDenominator == 0!"));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.stPreRollRate = *pstPreRollRate;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_PRE_ROLL_RATE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetPreRollRate_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetPreRollRate_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_Picture_Rate *pstPreRollRate
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetPreRollRate_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstPreRollRate );

   *pstPreRollRate = hXdmPP->stDMConfig.stPreRollRate;

   BDBG_LEAVE(BXDM_PictureProvider_GetPreRollRate_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetPlaybackMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool bEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetPlaybackMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetPlaybackMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  bEnable
                  ));

   hXdmPP->stDMConfig.bPlayback = bEnable;

   hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_PLAYBACK;

   BDBG_LEAVE(BXDM_PictureProvider_SetPlaybackMode_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetPlaybackMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool *pbEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetPlaybackMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pbEnable );

   *pbEnable = hXdmPP->stDMConfig.bPlayback;

   BDBG_LEAVE(BXDM_PictureProvider_GetPlaybackMode_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetDefaultFrameRate_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BAVC_FrameRateCode eFrameRate
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetDefaultFrameRate_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetDefaultFrameRate_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  eFrameRate
                  ));

   /* SW7425-3177: this code could be anywhere in XDM or XVD.  Forces a
    * complie time warning when new BAVC_FrameRateCode enums are added.
    */
#ifdef BDBG_CWARNING
   BDBG_CWARNING( BXDM_PictureProvider_P_MAX_FRAMERATE == BAVC_FrameRateCode_eMax );
#endif

   if ( eFrameRate >= BXDM_PictureProvider_P_MAX_FRAMERATE )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetDefaultFrameRate_isr:: eFrameRate value of %d is out of range", eFrameRate ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.eDefaultFrameRate = eFrameRate;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_DEFAULT_FRAME_RATE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetDefaultFrameRate_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetDefaultFrameRate_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BAVC_FrameRateCode *peFrameRate
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetDefaultFrameRate_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( peFrameRate );

   *peFrameRate = hXdmPP->stDMConfig.eDefaultFrameRate;

   BDBG_LEAVE(BXDM_PictureProvider_GetDefaultFrameRate_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetFrameRateDetectionMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_FrameRateDetectionMode eFrameRateDetectionMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetFrameRateDetectionMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetFrameRateDetectionMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  eFrameRateDetectionMode
                  ));

   if ( eFrameRateDetectionMode >= BXDM_PictureProvider_FrameRateDetectionMode_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetFrameRateDetectionMode_isr:: eFrameRateDetectionMode value of %d is out of range", eFrameRateDetectionMode ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.eFrameRateDetectionMode = eFrameRateDetectionMode;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_FRAME_RATE_DETECTION_MODE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetFrameRateDetectionMode_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetFrameRateDetection_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_FrameRateDetectionMode *peFrameRateDetectionMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetFrameRateDetection_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( peFrameRateDetectionMode );

   *peFrameRateDetectionMode = hXdmPP->stDMConfig.eFrameRateDetectionMode;

   BDBG_LEAVE(BXDM_PictureProvider_GetFrameRateDetection_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetASTMMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool bEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetASTMMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetASTMMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  bEnable
                  ));

   hXdmPP->stDMConfig.bAstmMode = bEnable;

   hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_ATSM_MODE;

   BDBG_LEAVE(BXDM_PictureProvider_SetASTMMode_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetASTMMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool *pbEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetASTMMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pbEnable );

   *pbEnable = hXdmPP->stDMConfig.bAstmMode;

   BDBG_LEAVE(BXDM_PictureProvider_GetASTMMode_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetVirtualTSMOnPCRDiscontinuityMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool bEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetVirtualTSMOnPCRDiscontinuityMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetVirtualTSMOnPCRDiscontinuityMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  bEnable
                  ));

   hXdmPP->stDMConfig.bVirtualTSMOnPCRDiscontinuity = bEnable;

   hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_VTSM_ON_PCR_DISCON;

   BDBG_LEAVE(BXDM_PictureProvider_SetVirtualTSMOnPCRDiscontinuityMode_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetVirtualTSMOnPCRDiscontinuityMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool *pbEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetVirtualTSMOnPCRDiscontinuityMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pbEnable );

   *pbEnable = hXdmPP->stDMConfig.bVirtualTSMOnPCRDiscontinuity;

   BDBG_LEAVE(BXDM_PictureProvider_GetVirtualTSMOnPCRDiscontinuityMode_isr);
   return BERR_TRACE( rc );
}
#endif

/*
 * SWSTB-68: API's for setting/getting the PCR discontinuity modes.
 */
BERR_Code
BXDM_PictureProvider_SetPCRMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         const BXDM_PictureProvider_PCRModes * pstPCRModes
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetPCRMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstPCRModes );

   BDBG_MSG(("%s(0x%lu) ePCRDiscontinuityModeAtStartup:%d",
                     __FUNCTION__,
                     (unsigned long)hXdmPP,
                     pstPCRModes->ePCRDiscontinuityModeAtStartup ));

   if ( pstPCRModes->ePCRDiscontinuityModeAtStartup >= BXDM_PictureProvider_PCRDiscontinuityMode_eMax )
   {
      BDBG_WRN(("%s:: ePCRDiscontinuityModeAtStartup value of %d is out of range", __FUNCTION__, pstPCRModes->ePCRDiscontinuityModeAtStartup ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.ePCRDiscontinuityModeAtStartup = pstPCRModes->ePCRDiscontinuityModeAtStartup;
      hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_PCR_ON_STARTUP;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetPCRMode_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetPCRMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_PCRModes * pstPCRModes
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetPCRMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstPCRModes );

   pstPCRModes->ePCRDiscontinuityModeAtStartup = hXdmPP->stDMConfig.ePCRDiscontinuityModeAtStartup;

   BDBG_LEAVE(BXDM_PictureProvider_GetPCRMode_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetErrorHandlingMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_ErrorHandlingMode eErrorHandlingMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetErrorHandlingMode_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetErrorHandlingMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  eErrorHandlingMode
                  ));

   if ( eErrorHandlingMode >= BXDM_PictureProvider_ErrorHandlingMode_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetErrorHandlingMode_isr:: eErrorHandlingMode value of %d is out of range", eErrorHandlingMode ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.eErrorHandlingMode = eErrorHandlingMode;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_ERROR_HANDLING_MODE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetErrorHandlingMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetErrorHandlingMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_ErrorHandlingMode *peErrorHandlingMode
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetErrorHandlingMode_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( peErrorHandlingMode );

   *peErrorHandlingMode = hXdmPP->stDMConfig.eErrorHandlingMode;

   BDBG_LEAVE(BXDM_PictureProvider_GetErrorHandlingMode_isr);
   return BERR_TRACE( rc );
}

/* SWSTB-439: adds a new error threshold with a range of 0:100, i.e. 0% to 100%.
 * When the error threshold is non-zero, the drop logic will use "uiPercentError"
 * from the Unified Picture structure to determine if the picture should be dropped.
 * Pictures will be dropped when the percentage of macro blocks with an error is
 * greater than or equal to "uiErrorThreshold".
 * When the error threshold is zero, the drop logic will use the error flags
 * "bThisPicture" and "bPreviousRefPic", this is the original behavior.  In this
 * mode, a single bad macro block can cause a picture to be dropped.
 * In both cases, the picture's error state is evaluated when "eErrorHandlingMode"
 * is set to either "ePicture" or "ePrognostic". */

BERR_Code
BXDM_PictureProvider_SetErrorThreshold_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t uiErrorThreshold
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetErrorThreshold_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetErrorThreshold_isr(0x%lu, %d)", (unsigned long)hXdmPP, uiErrorThreshold ));

   if ( uiErrorThreshold > 100 )
   {
      BDBG_WRN(("%s:: uiErrorThreshold of %d is outside the expected range of 0:100 ", __FUNCTION__, uiErrorThreshold ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.uiErrorThreshold = uiErrorThreshold;
      hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_ERROR_THRESHOLD;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetErrorThreshold_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetErrorThreshold_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t * puiErrorThreshold
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetErrorThreshold_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( puiErrorThreshold );

   *puiErrorThreshold = hXdmPP->stDMConfig.uiErrorThreshold;

   BDBG_LEAVE(BXDM_PictureProvider_GetErrorThreshold_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_SetTimerHandle_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BTMR_TimerHandle hTimer
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetTimerHandle_isr);

   BDBG_ASSERT(hXdmPP);

   BDBG_MSG(("BXDM_PictureProvider_SetTimerHandle_isr(0x%lu, h:0x%lu)",
             (unsigned long)hXdmPP,
             (unsigned long)hTimer
               ));

   hXdmPP->hTimer = hTimer;

   BDBG_LEAVE(BXDM_PictureProvider_SetTimerHandle_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetTimerHandle_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BTMR_TimerHandle *phTimer
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetTimerHandle_isr);

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(phTimer);

   *phTimer = hXdmPP->hTimer;

   BDBG_LEAVE(BXDM_PictureProvider_GetTimerHandle_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetChannelSyncMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool bEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetChannelSyncMode_isr);

   BDBG_ASSERT(hXdmPP);

   BDBG_MSG(("BXDM_PictureProvider_SetChannelSyncMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  bEnable
                  ));

   hXdmPP->stDMConfig.bChannelSyncMode = bEnable;

   hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_CHANNEL_SYNC_MODE;

   BDBG_LEAVE(BXDM_PictureProvider_SetChannelSyncMode_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetChannelSyncMode_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         bool *pbEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetChannelSyncMode_isr);

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(pbEnable);

   *pbEnable = hXdmPP->stDMConfig.bChannelSyncMode;

   BDBG_LEAVE(BXDM_PictureProvider_GetChannelSyncMode_isr);
   return BERR_TRACE( rc );
}
#endif

BERR_Code
BXDM_PictureProvider_SetDIH(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_DisplayInterruptHandler_Handle hXdmDih
         )
{
   BDBG_ENTER( BXDM_PictureProvider_SetDIH );

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(hXdmDih);

   hXdmPP->stDMState.stChannel.hXdmDih = hXdmDih;

   BDBG_LEAVE( BXDM_PictureProvider_SetDIH );

   return BERR_TRACE( BERR_SUCCESS );
}


BERR_Code
BXDM_PictureProvider_GetDIH_isrsafe(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_DisplayInterruptHandler_Handle *phXdmDih
         )
{
   BDBG_ENTER( BXDM_PictureProvider_GetDIH );

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(phXdmDih);

   *phXdmDih = hXdmPP->stDMState.stChannel.hXdmDih;

   BDBG_LEAVE( BXDM_PictureProvider_GetDIH );

   return BERR_TRACE( BERR_SUCCESS );
}

/* SW7405-4117: uiMaxHeightSupportedByDeinterlacer is used in conjuction with
 * BXDM_PictureProvider_DisplayFieldMode_eAuto to choose either eSingleField or eBothField
 * based on the steam height during slow motion (and preroll).
 */
BERR_Code
BXDM_PictureProvider_SetDeinterlacerMaxHeight_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t uiMaxHeight
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetDeinterlacerMaxHeight_isr);

   BDBG_ASSERT(hXdmPP);

   BDBG_MSG(("BXDM_PictureProvider_SetDeinterlacerMaxHeight_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  uiMaxHeight
                  ));

   hXdmPP->stDMConfig.uiMaxHeightSupportedByDeinterlacer = uiMaxHeight;

   hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_MAX_DEINT_HEIGHT;

   BDBG_LEAVE(BXDM_PictureProvider_SetDeinterlacerMaxHeight_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetDeinterlacerMaxHeight_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t * puiMaxHeight
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetDeinterlacerMaxHeight_isr);

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(puiMaxHeight);

   *puiMaxHeight = hXdmPP->stDMConfig.uiMaxHeightSupportedByDeinterlacer;

   BDBG_LEAVE(BXDM_PictureProvider_GetDeinterlacerMaxHeight_isr);
   return BERR_TRACE( rc );
}

/* SW7405-4703: API to allow bypass of BXDM_PPOUT_S_CalculateHorizontalOverscan */
BERR_Code BXDM_PictureProvider_SetHorizontalOverscanMode_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_HorizontalOverscanMode eHorizOverscanMode)
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetHorizontalOverscanMode_isr);

   BDBG_ASSERT(hXdmPP);

   BDBG_MSG(("BXDM_PictureProvider_SetHorizontalOverscanMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  eHorizOverscanMode
                  ));

   if ( eHorizOverscanMode >= BXDM_PictureProvider_HorizontalOverscanMode_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetHorizontalOverscanMode_isr:: eHorizOverscanMode value of %d is out of range", eHorizOverscanMode ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.eHorizontalOverscanMode = eHorizOverscanMode;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_HORIZON_OVERSCAN_MODE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetHorizontalOverscanMode_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetHorizontalOverscanMode_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_HorizontalOverscanMode *peHorizOverscanMode)
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetHorizontalOverscanMode_isr);

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(peHorizOverscanMode);

   *peHorizOverscanMode = hXdmPP->stDMConfig.eHorizontalOverscanMode;

   BDBG_LEAVE(BXDM_PictureProvider_GetHorizontalOverscanMode_isr);
   return BERR_TRACE( rc );
}

/* SWDEPRECATED-1003: needed to turn off the FIC logic during certain trick modes
 */
BERR_Code BXDM_PictureProvider_SetTrickMode_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_TrickMode eTrickMode
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetTrickMode_isr);

   BDBG_ASSERT(hXdmPP);

   BDBG_MSG(("BXDM_PictureProvider_SetTrickMode_isr(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  eTrickMode
                  ));
   if ( eTrickMode >= BXDM_PictureProvider_TrickMode_eMax )
   {
      BDBG_WRN(("BXDM_PictureProvider_SetTrickMode_isr:: eTrickMode value of %d is out of range", eTrickMode ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.eTrickMode = eTrickMode;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_TRICK_MODE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetTrickMode_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetTrickMode_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_TrickMode *peTrickMode
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetTrickMode_isr);

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(peTrickMode);

   *peTrickMode = hXdmPP->stDMConfig.eTrickMode;

   BDBG_LEAVE(BXDM_PictureProvider_GetTrickMode_isr);
   return BERR_TRACE( rc );
}
#endif

/* SWDEPRECATED-1003: provide an API to override the frame rate during trick modes.
*/
BERR_Code
BXDM_PictureProvider_SetFrameRateOverride_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         const BXDM_Picture_FrameRateOverride *pstFrameRateOverride
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetFrameRateOverride_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstFrameRateOverride );

   BDBG_MSG(("BXDM_PictureProvider_SetFrameRateOverride_isr(0x%lu, valid:%d %d/%d)",
                  (unsigned long)hXdmPP,
                  pstFrameRateOverride->bValid,
                  pstFrameRateOverride->stRate.uiNumerator,
                  pstFrameRateOverride->stRate.uiDenominator
                  ));

   /* Prevent a divide by '0'.*/
   if ( ( true == pstFrameRateOverride->bValid )
         && ( 0 == pstFrameRateOverride->stRate.uiDenominator ) )
   {
      hXdmPP->stDMConfig.stFrameRateOverride.bValid = false;
      BDBG_WRN(("BXDM_PictureProvider_SetFrameRateOverride_isr:: uiDenominator == 0!"));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.stFrameRateOverride = *pstFrameRateOverride;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_FRAMERATE_OVERRIDE;
   }

   BDBG_LEAVE(BXDM_PictureProvider_SetFrameRateOverride_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetFrameRateOverride_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_Picture_FrameRateOverride *pstFrameRateOverride
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetFrameRateOverride_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstFrameRateOverride );

   *pstFrameRateOverride = hXdmPP->stDMConfig.stFrameRateOverride;

   BDBG_LEAVE(BXDM_PictureProvider_GetFrameRateOverride_isr);
   return BERR_TRACE( rc );
}

/* SW7405-4736: Set the XDM instance ID. This is to help debug multi channel issues,
 * i.e. PIP and mosaic mode.  Many of the BXVD_DBG_MSG statements will print this ID
 * to help associate messages with a particular channel.  This can be set to any value,
 * perhaps what makes the most sense is to set it equal to the VDC rectangle number.
 */
BERR_Code
BXDM_PictureProvider_SetInstanceID_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t uiInstanceID
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetInstanceID_isr);

   BDBG_ASSERT(hXdmPP);

   BDBG_MSG(("BXDM_PictureProvider_SetInstanceID(0x%lu, %d)",
                  (unsigned long)hXdmPP,
                  uiInstanceID
                  ));

   hXdmPP->stDMConfig.uiInstanceID = uiInstanceID;

   BDBG_LEAVE(BXDM_PictureProvider_SetInstanceID_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetInstanceID_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         uint32_t * puiInstanceID
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetInstanceID_isr);

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(puiInstanceID);

   *puiInstanceID = hXdmPP->stDMConfig.uiInstanceID;

   BDBG_LEAVE(BXDM_PictureProvider_GetInstanceID_isr);
   return BERR_TRACE( rc );
}
#endif

/* SW7422-72: API's to allow the middleware/application to specify an
 * orientation for each picture.
 */

BERR_Code
BXDM_PictureProvider_Set3D_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_3DSettings * pst3DSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_Set3D_isr);

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(pst3DSettings);

   if ( true == pst3DSettings->bOverrideOrientation
         && pst3DSettings->eOrientation >= BXDM_PictureProvider_Orientation_eMax
      )
   {
      BDBG_WRN(("BXDM_PictureProvider_Set3D_isr:: bOverrideOrientation is out of range %d", pst3DSettings->bOverrideOrientation ));
      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXdmPP->stDMConfig.st3DSettings = *pst3DSettings;

      hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_3D_SETTINGS;
   }

   BDBG_LEAVE(BXDM_PictureProvider_Set3D_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_Get3D_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_3DSettings * pst3DSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_Get3D_isr);

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(pst3DSettings);

   *pst3DSettings = hXdmPP->stDMConfig.st3DSettings;

   BDBG_LEAVE(BXDM_PictureProvider_Get3D_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetDefault3D(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_3DSettings * pst3DSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetDefault3D);

   BSTD_UNUSED(hXdmPP);
   BDBG_ASSERT(pst3DSettings);

   BKNI_Memset( pst3DSettings, 0, sizeof( BXDM_PictureProvider_3DSettings ));

   BDBG_LEAVE(BXDM_PictureProvider_GetDefault3D);
   return BERR_TRACE( rc );
}

/*
 * SW7425-1264: support for a synthesized STC; can create a clock that run backwards.
 */
BERR_Code
BXDM_PictureProvider_SetClockOverride_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_ClockOverride * pstClockOverride
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetClockOverride_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstClockOverride );

   BDBG_MSG(("BXDM_PictureProvider_SetClockOverride_isr(0x%lu) Enable:%d  initialValue:%08x  load:%d stcDelta:%d)",
                  (unsigned long)hXdmPP,
                  pstClockOverride->bEnableClockOverride,
                  pstClockOverride->uiStcValue,
                  pstClockOverride->bLoadSwStc,
                  pstClockOverride->iStcDelta
                  ));

   hXdmPP->stDMConfig.stClockOverride = *pstClockOverride;

   /* Effectively a one-shot load.  Whenever called with this flag set,
    * SW STC will be reloaded.
    */
   hXdmPP->stDMConfig.bInitializeSwStc = pstClockOverride->bLoadSwStc;

   hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_CLOCK_OVERRIDE;

   BDBG_LEAVE(BXDM_PictureProvider_SetClockOverride_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetClockOverride_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_ClockOverride * pstClockOverride
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetClockOverride_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstClockOverride );

   *pstClockOverride = hXdmPP->stDMConfig.stClockOverride;

   BDBG_LEAVE(BXDM_PictureProvider_GetClockOverride_isr);
   return BERR_TRACE( rc );
}

/* SW7425-2270:
 * The application will call SetIgnoreNRTUnderflow when it determines that an NRT underflow
 * is actually a gap in the content (e.g. slideshow or end of stream) and the repeated picture
 * should actually be encoded.
 *
 * When SetIgnoreNRTUnderflow=true, the "decoder underflow" scenario should be ignored until either:
 * - the underflow condition ends
 * - the app explicitly sets SetIgnoreNRTUnderflow=false
 *
 * Note: only the "decoder underflow" condition is ignored. All other NRT scenarios
 * (e.g. "Other Transcode Stalled", "FIC Stall", etc) are still in effect.
 */
BERR_Code
BXDM_PictureProvider_SetIgnoreNRTUnderflow_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   bool bIgnoreNRTUnderflow
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetIgnoreNRTUnderflow_isr);

   BDBG_ASSERT( hXdmPP );

   BDBG_MSG(("BXDM_PictureProvider_SetIgnoreNRTUnderflow_isr(0x%lu) bIgnoreNRTUnderflow:%d",
                  (unsigned long)hXdmPP,
                  bIgnoreNRTUnderflow
                  ));

   hXdmPP->stDMConfig.bIgnoreNRTUnderflow = bIgnoreNRTUnderflow;

   /* If "bIgnoreNRTUnderflow" has been disabled, blindly clear "bIgnoringUnderflow"
    * just in case the system was currently ignoring underflows.
    */
   if ( false == hXdmPP->stDMConfig.bIgnoreNRTUnderflow )
   {
      hXdmPP->stDMState.stDecode.stNonRealTime.bIgnoringUnderflow = false;
   }

   hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_IGNORE_NRT_UNDERFLOW;

   BDBG_LEAVE(BXDM_PictureProvider_SetIgnoreNRTUnderflow_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_GetIgnoreNRTUnderflow_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   bool * pbIgnoreNRTUnderflow
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetIgnoreNRTUnderflow_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pbIgnoreNRTUnderflow );

   *pbIgnoreNRTUnderflow = hXdmPP->stDMConfig.bIgnoreNRTUnderflow;

   BDBG_LEAVE(BXDM_PictureProvider_GetIgnoreNRTUnderflow_isr);
   return BERR_TRACE( rc );
}
#endif

/*
 * SW7425-3358: support for FNRT.
 */
BERR_Code
BXDM_PictureProvider_SetFNRTSettings_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_FNRTSettings * pstFNRTSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_SetFNRTSettings_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstFNRTSettings );

   BDBG_MSG(("BXDM_PictureProvider_SetFNRTSettings_isr(0x%lu) bEnable:%d  uiPreChargeCount:%08x",
                  (unsigned long)hXdmPP,
                  pstFNRTSettings->bEnabled,
                  pstFNRTSettings->uiPreChargeCount
                  ));

   hXdmPP->stDMConfig.stFNRTSettings = *pstFNRTSettings;

   hXdmPP->stDMConfig.uiDirtyBits_2 |= BXDM_PictureProvider_P_DIRTY_2_FNRT_SETTINGS;

   BDBG_LEAVE(BXDM_PictureProvider_SetFNRTSettings_isr);
   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PictureProvider_GetFNRTSettings_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_FNRTSettings * pstFNRTSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetFNRTSettings_isr);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstFNRTSettings );

   *pstFNRTSettings = hXdmPP->stDMConfig.stFNRTSettings;

   BDBG_LEAVE(BXDM_PictureProvider_GetFNRTSettings_isr);
   return BERR_TRACE( rc );
}


BERR_Code
BXDM_PictureProvider_GetDefaultFNRTSettings_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_FNRTSettings * pstFNRTSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_GetDefaultFNRTSettings_isr);

   BSTD_UNUSED(hXdmPP);
   BDBG_ASSERT( pstFNRTSettings );

   BKNI_Memset( pstFNRTSettings, 0, sizeof( BXDM_PictureProvider_FNRTSettings ));

   BDBG_LEAVE(BXDM_PictureProvider_GetDefaultFNRTSettings_isr);
   return BERR_TRACE( rc );
}

/*
 * SWSTB-1380: returns a descriptor of the debug fifo.  This information
 * can be used to create a fifo reader.
 */
BERR_Code
BXDM_PictureProvider_GetDebugFifo(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_DebugFifoInfo *pstDebugFifoInfo
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstDebugFifoInfo );

   BKNI_Memset( pstDebugFifoInfo, 0, sizeof( BXDM_PictureProvider_DebugFifoInfo) );

   if ( NULL == hXdmPP->stDMConfig.stDebugFifo.hBMMABlock )
   {
      /* TODO: Print message that the fifo has not been created? */
      rc = BERR_NOT_SUPPORTED;
   }
   else
   {
      pstDebugFifoInfo->uiElementSize = hXdmPP->stDMConfig.stDebugFifo.uiElementSize;
      pstDebugFifoInfo->hBlock = hXdmPP->stDMConfig.stDebugFifo.hBMMABlock;
      pstDebugFifoInfo->uiOffset = 0;
   }

   return BERR_TRACE( rc );
}

/*
 * SWSTB-3450: support for passing start parameters directly to BXDM_PictureProvider_Start_isr
 */
BERR_Code
BXDM_PictureProvider_GetDefaultStartSettings_isrsafe(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_StartSettings * pstStartSettings
)
{
   BSTD_UNUSED(hXdmPP);
   BDBG_ASSERT(pstStartSettings);

   BKNI_Memset( pstStartSettings , 0, sizeof( BXDM_PictureProvider_StartSettings ));

   /* Disabled by the default. */
   pstStartSettings->stColorOverride.eOverrideMode = BXDM_PictureProvider_ColorOverrideMode_eNone;

   /* initialize the color override SDR parameters. */
   pstStartSettings->stColorOverride.stSDR.eTransferCharacteristics = BAVC_TransferCharacteristics_eUnknown;

   /* initialize the color override HDR parameters. */
   pstStartSettings->stColorOverride.stHDR.eTransferCharacteristics = BAVC_TransferCharacteristics_eUnknown;
   pstStartSettings->stColorOverride.stHDR.ulAvgContentLight    = 0;
   pstStartSettings->stColorOverride.stHDR.ulMaxContentLight    = 0;
   pstStartSettings->stColorOverride.stHDR.stDisplayPrimaries[0].ulX    = 0xFFFFFFFF;
   pstStartSettings->stColorOverride.stHDR.stDisplayPrimaries[0].ulY    = 0xFFFFFFFF;
   pstStartSettings->stColorOverride.stHDR.stDisplayPrimaries[1].ulX    = 0xFFFFFFFF;
   pstStartSettings->stColorOverride.stHDR.stDisplayPrimaries[1].ulY    = 0xFFFFFFFF;
   pstStartSettings->stColorOverride.stHDR.stDisplayPrimaries[2].ulX    = 0xFFFFFFFF;
   pstStartSettings->stColorOverride.stHDR.stDisplayPrimaries[2].ulY    = 0xFFFFFFFF;
   pstStartSettings->stColorOverride.stHDR.stWhitePoint.ulX             = 0xFFFFFFFF;
   pstStartSettings->stColorOverride.stHDR.stWhitePoint.ulY             = 0xFFFFFFFF;
   pstStartSettings->stColorOverride.stHDR.ulMaxDispMasteringLuma       = 0xFFFFFFFF;
   pstStartSettings->stColorOverride.stHDR.ulMinDispMasteringLuma       = 0xFFFFFFFF;

   return BERR_SUCCESS;

}

/*
 * SWSTB-3450: support for passing stop parameters directly to BXDM_PictureProvider_Stop_isr
 */
BERR_Code
BXDM_PictureProvider_GetDefaultStopSettings_isrsafe(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_StopSettings * pstStopSettings
)
{
   BSTD_UNUSED(hXdmPP);
   BDBG_ASSERT(pstStopSettings);

   BKNI_Memset( pstStopSettings , 0, sizeof( BXDM_PictureProvider_StopSettings ));

   return BERR_SUCCESS;
}

/*
 * Callback
 */
BERR_Code
BXDM_PictureProvider_Callback_SetEnable_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback ePictureProviderCallback,
         bool bEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_Callback_SetEnable_isr);

   BDBG_ASSERT(hXdmPP);

   BDBG_MSG(("BXDM_PictureProvider_Callback_SetEnable_isr(0x%lu, cb:%d, e:%d)",
                  (unsigned long)hXdmPP,
                  ePictureProviderCallback,
                  bEnable
                  ));

   hXdmPP->stCallbacks[ePictureProviderCallback].bEnable = bEnable;

   BDBG_LEAVE(BXDM_PictureProvider_Callback_SetEnable_isr);
   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_PictureProvider_Callback_GetEnable_isr(
         BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_Callback ePictureProviderCallback,
         bool *pbEnable
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BXDM_PictureProvider_Callback_GetEnable_isr);

   BDBG_ASSERT(hXdmPP);
   BDBG_ASSERT(pbEnable);

   *pbEnable = hXdmPP->stCallbacks[ePictureProviderCallback].bEnable;

   BDBG_LEAVE(BXDM_PictureProvider_Callback_GetEnable_isr);
   return BERR_TRACE( rc );
}
#endif

#define BXDM_PP_CALLBACK_ENTRY(_name)\
BERR_Code \
BXDM_PictureProvider_Callback_Install_##_name##_isr(\
         BXDM_PictureProvider_Handle hXdmPP,\
         BXDM_PictureProvider_Callback_##_name##_isr fCallback,\
         void *pPrivateContext,\
         int32_t iPrivateParam\
)\
{\
   BERR_Code rc = BERR_SUCCESS;\
   BXDM_PictureProvider_Callback ePictureProviderCallback = BXDM_PictureProvider_Callback_e##_name;\
   BDBG_ENTER(BXDM_PictureProvider_Callback_Install_##_name##_isr);\
   BDBG_ASSERT(hXdmPP);\
   BDBG_MSG(("BXDM_PictureProvider_Callback_Install_"#_name"_isr(0x%lu, f:0x%lu, pp:0x%lu, pi:%d)",\
                  (unsigned long)hXdmPP,\
                  (unsigned long)fCallback,\
                  (unsigned long)pPrivateContext,\
                  iPrivateParam\
                  ));\
   hXdmPP->stCallbacks[ePictureProviderCallback].stFunction.f##_name = fCallback;\
   hXdmPP->stCallbacks[ePictureProviderCallback].pPrivateContext = pPrivateContext;\
   hXdmPP->stCallbacks[ePictureProviderCallback].iPrivateParam = iPrivateParam;\
   BDBG_LEAVE(BXDM_PictureProvider_Callback_Install_##_name##_isr);\
   return BERR_TRACE( rc );\
}
#include "bxdm_pp_callback.def"

#define BXDM_PP_CALLBACK_ENTRY(_name)\
BERR_Code \
BXDM_PictureProvider_Callback_UnInstall_##_name##_isr(\
         BXDM_PictureProvider_Handle hXdmPP\
         )\
{\
   BERR_Code rc = BERR_SUCCESS;\
   BXDM_PictureProvider_Callback ePictureProviderCallback = BXDM_PictureProvider_Callback_e##_name;\
   BDBG_ENTER(BXDM_PictureProvider_Callback_Install_##_name##_isr);\
   BDBG_ASSERT(hXdmPP);\
   BDBG_MSG(("BXDM_PictureProvider_Callback_UnInstall_"#_name"_isr(0x%lu)",\
                  (unsigned long)hXdmPP\
                  ));\
   hXdmPP->stCallbacks[ePictureProviderCallback].stFunction.f##_name = NULL;\
   hXdmPP->stCallbacks[ePictureProviderCallback].pPrivateContext = NULL;\
   hXdmPP->stCallbacks[ePictureProviderCallback].iPrivateParam = 0;\
   BDBG_LEAVE(BXDM_PictureProvider_Callback_Install_##_name##_isr);\
   return BERR_TRACE( rc );\
}

#include "bxdm_pp_callback.def"
