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

#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Dbglib */
#include "bfmt.h"

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bxdm_pp_output.h"
#include "bxdm_pp_fic.h"
#include "bxdm_pp_dbg.h"
#include "bxdm_pp_callback_priv.h"
#include "bxdm_pp_qm.h"

BDBG_MODULE(BXDM_PPOUT);
BDBG_FILE_MODULE(BXDM_PPOUT);

/* Console Output:
   T - Top Field Interrupt
   B - Bot Field Interrupt
   F - Frame Interrupt
   c - clipping occurred
   p[x][y] - pan-scan occurred for protocol x on field y
   M - muted
*/

/*******************************************************************************
**
**  Local functions.
**
*******************************************************************************/

static void BXDM_PPOUT_S_UpdatePictureParameters_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BAVC_MFD_Picture* pMFDPicture,
   BXDM_PictureProvider_P_Picture_Context * pPictureContext,
   bool bCachePictureParameters
   )
{
   /* SW7405-4378: The issue was that the coded size/width and source size/width reported
    * by the PictureParameter callback did not match at startup.   This only happened
    * occasionally when playing a live stream.
    *
    * Turned out that the first picture was being dropped since the PCR offset was not valid.
    * As a result the stMFDPicture was being reset to '0' by the call to
    * BXDM_PPQM_P_InvalidatePictureContext_isr.  However stUnifiedPicture was not being reset.
    * The application would see the correct coded size for the stream but 0's for the source size.
    *
    * The fix is to cache the picture parameters in "stCachedPictureParameters" prior to a picture
    * passing the TSM test.  The PictureParameter callback will report these value.  Once a picture
    * is prompted for display, the PictureParameter callback will report the values from the
    * selected picture.
    */
   if ( true == bCachePictureParameters )
   {
      BXDM_PictureProvider_P_PictureParameterInfo * pCachedParams = &(hXdmPP->stDMStatus.stCachedPictureParameters);

      /* Just for debug. */
      pCachedParams->bUseCachedPictureParameters = true;

      pCachedParams->stUnifiedPicture  = *(pPictureContext->pstUnifiedPicture);
      pCachedParams->stMFDPicture      = *pMFDPicture;

      hXdmPP->stDMStatus.stPictureParameterInfo.pstUnifiedPicture = &(pCachedParams->stUnifiedPicture);
      hXdmPP->stDMStatus.stPictureParameterInfo.pstMFDPicture = &(pCachedParams->stMFDPicture);
   }
   else
   {
      /* Just for debug. */
      hXdmPP->stDMStatus.stCachedPictureParameters.bUseCachedPictureParameters = false;

      hXdmPP->stDMStatus.stPictureParameterInfo.pstUnifiedPicture = pPictureContext->pstUnifiedPicture;
      hXdmPP->stDMStatus.stPictureParameterInfo.pstMFDPicture = pMFDPicture;
   }

   BDBG_LEAVE(BXDM_PPOUT_S_UpdatePictureParameters_isr);
   return;
} /* end of BXDM_PPOUT_S_UpdatePictureParameters_isr() */

static void BXDM_PPOUT_S_ConvertSourcePolarityToInterlaced_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState* pLocalState,
   BAVC_Polarity * pPolarity
   )
{
   BDBG_ENTER(BXDM_PPOUT_S_ConvertSourcePolarityToInterlaced_isr);

   if (BAVC_Polarity_eFrame == pLocalState->eVsyncPolarity)
   {
      /* If we're on a progressive display, we need to
       * manually alternate between delivering Top or Bottom
       * fields to the display */
      switch ( hXdmPP->stDMState.stChannel.ePrevSourcePolarity )
      {
         case BAVC_Polarity_eTopField:
            *pPolarity = BAVC_Polarity_eBotField;
            break;

         case BAVC_Polarity_eBotField:
         case BAVC_Polarity_eFrame:
         default:
            *pPolarity = BAVC_Polarity_eTopField;
            break;
      }
   }
   else
   {
      /* If we're on an interlaced display, we just need to
       * set the source polarity to equal the interrupt
       * polarity */
      *pPolarity = pLocalState->eVsyncPolarity;
   }

   BDBG_LEAVE(BXDM_PPOUT_S_ConvertSourcePolarityToInterlaced_isr);
   return;
} /* end of BXDM_PPOUT_S_ConvertSourcePolarityToInterlaced_isr() */

static void BXDM_PPOUT_S_ApplySPOScanMode_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState* pLocalState,
   const BXDM_PictureProvider_P_Picture_Context *pPictureContext,
   BAVC_MFD_Picture* pMFDPicture,
   uint32_t *puiOverrideBits
   )
{
   const BXDM_PictureProvider_P_Picture_Params * pstPicParms= &( pPictureContext->stPicParms );

   BDBG_ENTER(BXDM_PPOUT_S_ApplySPOScanMode_isr);

   switch ( pstPicParms->stDisplay.stStatic.eScanMode )
   {
      case BXDM_PictureProvider_P_ScanMode_eInterlaced:
         /* Handle p->i source override */
         if ( BAVC_Polarity_eFrame == pMFDPicture->eSourcePolarity )
         {
            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eOUT, "%x:[%02x.%03x] Source Polarity Override: p->i",
                                             hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                             BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                             pstPicParms->uiPPBIndex & 0xFFF ));

            *puiOverrideBits |= BXDM_PPDBG_Output_SPO_pToi;

            BXDM_PPOUT_S_ConvertSourcePolarityToInterlaced_isr(
               hXdmPP,
               pLocalState,
               &(pMFDPicture->eSourcePolarity)
               );
         }
         break;

      case BXDM_PictureProvider_P_ScanMode_eProgressive:
         /* Handle i->p source override */
         if ( BAVC_Polarity_eFrame != pMFDPicture->eSourcePolarity )
         {
            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eOUT, "%x:[%02x.%03x] Source Polarity Override: i->p",
                                             hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                             BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                             pstPicParms->uiPPBIndex & 0xFFF ));

            *puiOverrideBits |= BXDM_PPDBG_Output_SPO_iTop;

            pMFDPicture->eSourcePolarity = BAVC_Polarity_eFrame;
         }
         break;

      default:
         break;
   }

   BDBG_LEAVE(BXDM_PPOUT_S_ApplySPOScanMode_isr);
}

static void BXDM_PPOUT_S_ApplySPODisplayFieldMode_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState* pLocalState,
   const BXDM_PictureProvider_P_Picture_Context *pPictureContext,
   BAVC_MFD_Picture* pMFDPicture,
   uint32_t *puiOverrideBits
   )
{
   const BXDM_PictureProvider_P_Picture_Params * pstPicParms= &( pPictureContext->stPicParms );

   BXDM_Picture_PullDown ePulldown;

   BDBG_ENTER(BXDM_PPOUT_S_ApplySPODisplayFieldMode_isr);
   BSTD_UNUSED(pLocalState);

   /* SW7445-586: use accessor method to retrieve pulldwon */
   BXDM_PPQM_P_GetPicturePulldown_isr( pPictureContext, &ePulldown );

   /* Override the source polarity based on the eDisplayFieldMode
    * setting */
   if (BAVC_Polarity_eFrame != pMFDPicture->eSourcePolarity)
   {
      /* We only handle display field mode for interlaced source
       * content */
      switch ( pstPicParms->stDisplay.stDynamic.eDisplayFieldMode )
      {
         case BXDM_PictureProvider_DisplayFieldMode_eTopFieldOnly:
            if ( ePulldown != BXDM_Picture_PullDown_eBottom )
            {
               BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eOUT, "%x:[%02x.%03x] Source Polarity Override: eTopFieldOnly",
                                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                pstPicParms->uiPPBIndex & 0xFFF ));

               pMFDPicture->eSourcePolarity = BAVC_Polarity_eTopField;
               *puiOverrideBits |= BXDM_PPDBG_Output_SPO_TopField;
            }
            break;

         case BXDM_PictureProvider_DisplayFieldMode_eBottomFieldOnly:
            if ( ePulldown != BXDM_Picture_PullDown_eTop )
            {
               BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eOUT, "%x:[%02x.%03x] Source Polarity Override: eBotFieldOnly",
                                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                pstPicParms->uiPPBIndex & 0xFFF ));

               pMFDPicture->eSourcePolarity = BAVC_Polarity_eBotField;
               *puiOverrideBits |= BXDM_PPDBG_Output_SPO_BottomField;
            }
            break;

         case BXDM_PictureProvider_DisplayFieldMode_eSingleField:
            /* We use the previous source polarity */
            if ( ( ePulldown != BXDM_Picture_PullDown_eBottom ) /* PR50158: Not a dangling field */
                 && ( ePulldown != BXDM_Picture_PullDown_eTop )
                 && ( pstPicParms->stDisplay.stDynamic.eTrickPlayRepeatMode == BXDM_PictureProvider_P_RepeatMode_eField ) /* PR50157: Not a progressive-in-nature picture */
                 && ( BAVC_Polarity_eFrame != hXdmPP->stDMState.stChannel.ePrevSourcePolarity ) /* PR56063: Handle switching from frame to field content */
               )
            {
               BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eOUT, "%x:[%02x.%03x] Source Polarity Override: eSingleField",
                                                   hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                   BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                   pstPicParms->uiPPBIndex & 0xFFF ));

               pMFDPicture->eSourcePolarity = hXdmPP->stDMState.stChannel.ePrevSourcePolarity;
               *puiOverrideBits |= BXDM_PPDBG_Output_SPO_SingleField;
            }
            break;

         case BXDM_PictureProvider_DisplayFieldMode_eBothField:
         default:
            {
               bool bPaused;

               /* SW7445-3642: Check for a STC trick mode or decoder trick mode of pause.
                * Don't increment uiDisplayedParityFailureCount when the system is paused. */

               bPaused = ( BXDM_PictureProvider_P_STCTrickMode_ePause == pLocalState->eSTCTrickMode );
               bPaused |= ( 0 == pLocalState->uiSlowMotionRate );

               /* Keep track of display polarity failures for reporting via GetChannelStatus */
               if ( BAVC_Polarity_eFrame != pMFDPicture->eSourcePolarity && false == bPaused )
               {
                  /* SW7405-3586: If source interlaced, and display is progressive, only increment parity
                     failure count if two fields in a row (or more) are the same polarity.
                     So, T0 T0r B0 and T0 T1 B1 should each increment the value by 1, but T0 B0 T1 should not. */
                  if (BAVC_Polarity_eFrame == pMFDPicture->eInterruptPolarity)
                  {
                     /* interlaced content on progressive display ... */
                     if (pMFDPicture->eSourcePolarity == hXdmPP->stDMState.stChannel.ePrevSourcePolarity)
                     {
                        hXdmPP->stDMStatus.stCounters.uiDisplayedParityFailureCount++;
                     }
                  }
                  else
                  {
                     /* if interlaced content on interlaced display, verify interrupt polarity matches source polarity */
                     if (pMFDPicture->eSourcePolarity != pMFDPicture->eInterruptPolarity)
                     {
                        hXdmPP->stDMStatus.stCounters.uiDisplayedParityFailureCount++;
                     }
                  }
               }
            }
            break;
      }
   }

   BDBG_LEAVE(BXDM_PPOUT_S_ApplySPODisplayFieldMode_isr);
}

static void BXDM_PPOUT_S_ApplySPOMPIM_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState* pLocalState,
   const BXDM_PictureProvider_P_Picture_Context *pPictureContext,
   BAVC_MFD_Picture* pMFDPicture,
   uint32_t *puiOverrideBits
   )
{
   BDBG_ENTER(BXDM_PPOUT_S_ApplySPOMPIM_isr);

   BSTD_UNUSED(pPictureContext);

   /* Override the source polarity based on the
    * eMovingContentInterpolationMode setting */
   switch ( hXdmPP->stDMConfig.eScanModeOverride )
   {
      case BXDM_PictureProvider_ScanModeOverride_eInterlaced:
         /* We need to force interlaced scanout */
         if (pMFDPicture->eSourcePolarity == BAVC_Polarity_eFrame)
         {
            /*  We only need to worry about changing the source
             *  polarity for content that is progressive */
            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eOUT, "%x:[%02x.%03x] Source Polarity Override: eInterlacedScanout",
                                                   hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                   BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                   pPictureContext->stPicParms.uiPPBIndex & 0xFFF ));

            *puiOverrideBits |= BXDM_PPDBG_Output_SPO_Interlaced;

            BXDM_PPOUT_S_ConvertSourcePolarityToInterlaced_isr(
               hXdmPP,
               pLocalState,
               &(pMFDPicture->eSourcePolarity)
               );
         }
         break;

      case BXDM_PictureProvider_ScanModeOverride_eProgressive:
         /* We're forcing progressive scanout, so just set source
          * polarity to eFrame */
         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eOUT, "%x:[%02x.%03x] Source Polarity Override: eProgressiveScanout",
                                                   hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                   BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                   pPictureContext->stPicParms.uiPPBIndex & 0xFFF ));

         *puiOverrideBits |= BXDM_PPDBG_Output_SPO_Progressive;

         pMFDPicture->eSourcePolarity = BAVC_Polarity_eFrame;
         break;

      default:
      case BXDM_PictureProvider_ScanModeOverride_eDefault:
         /* Do nothing */
         break;
   }

   BDBG_LEAVE(BXDM_PPOUT_S_ApplySPOMPIM_isr);
   return;
} /* end of BXDM_PPOUT_S_ApplyMPIMOverride() */

BERR_Code BXDM_PPOUT_S_ApplySPOProgressive_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture,
   BAVC_MFD_Picture* pMFDPicture,
   uint32_t *puiOverrideBits
   )
{
   /* Override picture's repeat mode based on SPIM setting */
   switch ( pstPicture->stPicParms.stDisplay.stDynamic.eSourceFormatOverride )
   {
      case BXDM_PictureProvider_SourceFormatOverride_eProgressive:
         if ( pstPicture->stPicParms.stTSM.stStatic.uiNumElements > 1 )
         {
            pstPicture->stPicParms.stDisplay.stDynamic.eRateConversionRepeatMode = BXDM_PictureProvider_P_RepeatMode_eFrame;
            pstPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode = BXDM_PictureProvider_P_RepeatMode_eFrame;
         }
         break;

      case BXDM_PictureProvider_SourceFormatOverride_eInterlaced:
         pstPicture->stPicParms.stDisplay.stDynamic.eRateConversionRepeatMode = BXDM_PictureProvider_P_RepeatMode_eField;
         pstPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode = BXDM_PictureProvider_P_RepeatMode_eField;
         break;

      default:
         break;
   }

   /* Apply progressive override */
   if ( ( pMFDPicture->eSourcePolarity != BAVC_Polarity_eFrame )
        && ( pMFDPicture->eSourcePolarity != pLocalState->eVsyncPolarity )
        && ( false == hXdmPP->stDMConfig.bCRCMode ) )
   {
      if ( BXDM_PictureProvider_P_RepeatMode_eFrame == pstPicture->stPicParms.stDisplay.stDynamic.eRateConversionRepeatMode )
      {
         /* We have inversion but the content can be treated as progressive at all times,
          * so we toggle the source polarity accordingly
          *
          * Some cases where we want this to happen:
          *  - 24i to 60Hz display
          *  - 3:2 to 50Hz display
          */
         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eOUT, "%x:[%02x.%03x] Polarity Override: progressive both field",
                                             hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                             BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                             pstPicture->stPicParms.uiPPBIndex & 0xFFF ));

         *puiOverrideBits |= BXDM_PPDBG_Output_SPO_ProgBothField;

         BXDM_PPOUT_S_ConvertSourcePolarityToInterlaced_isr(
            hXdmPP,
            pLocalState,
            &(pMFDPicture->eSourcePolarity)
            );
      }
      else if ( ( ( BXDM_PictureProvider_P_RepeatMode_eFrame == pstPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode )
                  && ( ( BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE != pLocalState->uiSlowMotionRate ) /* We're in a decoder (pause/play) trick mode, NOTE: this could be FF  */
                       || ( BXDM_PictureProvider_P_STCTrickMode_eSlowMotion == pLocalState->eSTCTrickMode ) /* We're in an STC slow trick mode */
                       || ( BXDM_PictureProvider_P_STCTrickMode_eSlowRewind == pLocalState->eSTCTrickMode ) /* SW7425-1264: using SW STC for slow rewind trick */
                       || ( BXDM_PictureProvider_P_STCTrickMode_ePause == pLocalState->eSTCTrickMode ) /* We're in an STC pause trick mode */
                       || ( ( true == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated ) /* non-rate conversion repeat */
                            && ( pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated > (int32_t) pstPicture->stPicParms.stTSM.stStatic.stPTSDelta.uiWhole ) )
                     )
                 )
              )
      {
         /* We have inversion but the content can be treated as progressive during non-rate conversion repeats,
          * so we toggle the source polarity accordingly
          *
          * Some cases where we want this to happen:
          *  - Pause/Slow motion for content that is progressive-in-nature
          *  - unexpected repeats (errors, missing pictures, hold last picture, etc.)
          *  */

         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eOUT, "%x:[%02x.%03x] Polarity Override: progressive repeat",
                                             hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                             BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                             pstPicture->stPicParms.uiPPBIndex & 0xFFF ));

         *puiOverrideBits |= BXDM_PPDBG_Output_SPO_ProgRepeat;

         BXDM_PPOUT_S_ConvertSourcePolarityToInterlaced_isr(
            hXdmPP,
            pLocalState,
            &(pMFDPicture->eSourcePolarity)
            );
      }
   }

   return BERR_SUCCESS;

}  /* end of BXDM_PPOUT_S_PerformProgressiveOverride() */

static void BXDM_PPOUT_S_SetSourcePolarity_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPictureContext,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BAVC_MFD_Picture* pMFDPicture
   )
{
   uint32_t uiOverrideBits = 0;

   BDBG_ENTER(BXDM_PPOUT_S_SetSourcePolarity_isr);

   /* Set the source polarity to what DM specifies */
   pMFDPicture->eSourcePolarity = pPictureContext->stPicParms.stDisplay.stDynamic.eSourcePolarity;

   /* Override the source polarity based on the special cases as
    * described in rate conversion table */
   if ( false == hXdmPP->stDMConfig.bCRCMode )
   {
      /* The polarity may need be to overridden if the pulldown is interlaced
       * and the source polarity doesn't match the interrupt polarity.
       */
      BXDM_PPOUT_S_ApplySPOProgressive_isr(
               hXdmPP,
               pLocalState,
               pPictureContext,
               pMFDPicture,
               &uiOverrideBits
               );

      BXDM_PPOUT_S_ApplySPOScanMode_isr(
               hXdmPP,
               pLocalState,
               pPictureContext,
               pMFDPicture,
               &uiOverrideBits
               );

      BXDM_PPOUT_S_ApplySPODisplayFieldMode_isr(
               hXdmPP,
               pLocalState,
               pPictureContext,
               pMFDPicture,
               &uiOverrideBits
               );

      BXDM_PPOUT_S_ApplySPOMPIM_isr(
               hXdmPP,
               pLocalState,
               pPictureContext,
               pMFDPicture,
               &uiOverrideBits
               );
   }

   BXDM_PPDBG_P_OutputSPOLog_isr(
      hXdmPP,
      uiOverrideBits
      );

   BDBG_LEAVE(BXDM_PPOUT_S_SetSourcePolarity_isr);
   return;
} /* end of BXDM_PPOUT_S_SetSourcePolarity_isr() */

static void BXDM_PPOUT_S_SetMpegType_isr(
         const BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_P_Picture_Context *pPictureContext,
         BAVC_MFD_Picture* pPicture
         )
{
   const BXDM_Picture* pUnifiedPicture = pPictureContext->pstUnifiedPicture;
   BAVC_MFD_Picture * pDefaultParams = &( hXdmPP->stDMState.stChannel.stDefaultPicParams );

   if ( true == pUnifiedPicture->stBufferInfo.stChromaLocation[pPicture->eSourcePolarity].bValid )
   {
      pPicture->eMpegType = pUnifiedPicture->stBufferInfo.stChromaLocation[pPicture->eSourcePolarity].eMpegType;
   }
   else if ( true == pUnifiedPicture->stBufferInfo.stChromaLocation[BAVC_Polarity_eFrame].bValid )
   {
      pPicture->eMpegType = pUnifiedPicture->stBufferInfo.stChromaLocation[BAVC_Polarity_eFrame].eMpegType;
   }
   else if ( true == pUnifiedPicture->stBufferInfo.stChromaLocation[BAVC_Polarity_eTopField].bValid )
   {
      pPicture->eMpegType = pUnifiedPicture->stBufferInfo.stChromaLocation[BAVC_Polarity_eTopField].eMpegType;
   }
   else if ( true == pUnifiedPicture->stBufferInfo.stChromaLocation[BAVC_Polarity_eBotField].bValid )
   {
      pPicture->eMpegType = pUnifiedPicture->stBufferInfo.stChromaLocation[BAVC_Polarity_eBotField].eMpegType;
   }
   else
   {
      pPicture->eMpegType = pDefaultParams->eMpegType;
   }
}

static void BXDM_PPOUT_S_SetPictureOrderCount_isr(
         const BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_P_Picture_Context *pPictureContext,
         BAVC_MFD_Picture* pPicture
         )
{
   const BXDM_Picture* pUnifiedPicture = pPictureContext->pstUnifiedPicture;
   BAVC_MFD_Picture * pDefaultParams = &( hXdmPP->stDMState.stChannel.stDefaultPicParams );

   /* Set defaults */
   pPicture->ulIdrPicID = pDefaultParams->ulIdrPicID;
   pPicture->int32_PicOrderCnt = pDefaultParams->int32_PicOrderCnt;

   if ( ( NULL != pUnifiedPicture )
        && ( true == pUnifiedPicture->stPOC.bValid ) )
   {
      pPicture->ulIdrPicID = pUnifiedPicture->stPOC.uiPictureId;

      if ( true == pUnifiedPicture->stPOC.stPictureOrderCount[pPicture->eSourcePolarity].bValid )
      {
         pPicture->int32_PicOrderCnt = pUnifiedPicture->stPOC.stPictureOrderCount[pPicture->eSourcePolarity].iValue;
      }
      else if ( true == pUnifiedPicture->stPOC.stPictureOrderCount[BAVC_Polarity_eFrame].bValid )
      {
         pPicture->int32_PicOrderCnt = pUnifiedPicture->stPOC.stPictureOrderCount[BAVC_Polarity_eFrame].iValue;
      }
      else if ( true == pUnifiedPicture->stPOC.stPictureOrderCount[BAVC_Polarity_eTopField].bValid )
      {
         pPicture->int32_PicOrderCnt = pUnifiedPicture->stPOC.stPictureOrderCount[BAVC_Polarity_eTopField].iValue;
      }
      else if ( true == pUnifiedPicture->stPOC.stPictureOrderCount[BAVC_Polarity_eBotField].bValid )
      {
         pPicture->int32_PicOrderCnt = pUnifiedPicture->stPOC.stPictureOrderCount[BAVC_Polarity_eBotField].iValue;
      }
   }
}

static void BXDM_PPOUT_S_SetSourceBufferInformation_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPictureContext,
   BAVC_MFD_Picture* pPicture
   )
{
   const BXDM_Picture* pUnifiedPicture = pPictureContext->pstUnifiedPicture;
   BXDM_PictureProvider_P_Picture_Params* pstPicParms= &( pPictureContext->stPicParms );
   BAVC_MFD_Picture * pDefaultParams = &( hXdmPP->stDMState.stChannel.stDefaultPicParams );

   BDBG_ENTER(BXDM_PPOUT_S_SetSourceBufferInformation_isr);

   BDBG_ASSERT(pPicture);

   if ( pUnifiedPicture )
   {
      const BXDM_Picture_BufferInfo * pstBufferInfo;

      /* bMute */
      pPicture->bMute = false;

      /* SW7445-586: for H265/HEVC interlaced, two linked Unified Pictures may be received.
       * Each Unified Picture will describe a single field.  These two fields need to be
       * treated as one logical picture.
       *
       * The addresses for the top field in the MFD structure need to be filled in with the
       * appropriate values from this logic picture. The same holds true for the addresses
       * for the bottom field. This is achieved by calling BXDM_PPQM_P_GetPictureTopBuffer_isr
       * and BXDM_PPQM_P_GetPictureBottomBuffer_isr.
       *
       * Alternative, if only a single field split interlaced picture is received.  The top
       * and bottom field addresses will be filled in with the same value. Again this is done
       * by calling the preceding routines.
       */

      /* Set the values for the bottom field to the defaults. */
      pPicture->eBufferFormat = pDefaultParams->eBufferFormat;
      pPicture->hLuminanceBotFieldBufferBlock = pDefaultParams->hLuminanceBotFieldBufferBlock;
      pPicture->ulLuminanceBotFieldBufferBlockOffset = pDefaultParams->ulLuminanceBotFieldBufferBlockOffset;
      pPicture->hChrominanceBotFieldBufferBlock = pDefaultParams->hLuminanceBotFieldBufferBlock;
      pPicture->ulChrominanceBotFieldBufferBlockOffset = pDefaultParams->ulLuminanceBotFieldBufferBlockOffset;

      if ( BXDM_Picture_BufferFormat_eSplitInterlaced == pPictureContext->pstUnifiedPicture->stBufferInfo.eBufferFormat )
      {
         BXDM_PPQM_P_GetPictureBottomBuffer_isr( pPictureContext, &pstBufferInfo );

         /* If eFrame(=0), it's frame buffer; else if eFieldPair(=1), it's fields pair. */
         pPicture->eBufferFormat = BAVC_DecodedPictureBuffer_eFieldsPair;

         /* Luminance bottom field buffer (new address) if eBufferFormat is eFieldPair; else, =NULL. */
         pPicture->hLuminanceBotFieldBufferBlock = pstBufferInfo->hLuminanceFrameBufferBlock;
         pPicture->ulLuminanceBotFieldBufferBlockOffset =  pstBufferInfo->ulLuminanceFrameBufferBlockOffset;

         /* Chrominance bottom field buffer (new address) when eBufferFormat is eFieldPair; else, =NULL. */
         pPicture->hChrominanceBotFieldBufferBlock = pstBufferInfo->hChrominanceFrameBufferBlock;
         pPicture->ulChrominanceBotFieldBufferBlockOffset =  pstBufferInfo->ulChrominanceFrameBufferBlockOffset;
      }

      /* SW7445-586: use accessor to get buffer descriptor. */
      BXDM_PPQM_P_GetPictureTopBuffer_isr( pPictureContext, &pstBufferInfo );

      /* pLuminanceFrameBufferAddress */
      pPicture->hLuminanceFrameBufferBlock = pstBufferInfo->hLuminanceFrameBufferBlock;
      pPicture->ulLuminanceFrameBufferBlockOffset =  pstBufferInfo->ulLuminanceFrameBufferBlockOffset;

      /* pChrominanceFrameBufferAddress */
      pPicture->hChrominanceFrameBufferBlock = pstBufferInfo->hChrominanceFrameBufferBlock;
      pPicture->ulChrominanceFrameBufferBlockOffset =  pstBufferInfo->ulChrominanceFrameBufferBlockOffset;

      /* ulLuminanceNMBY */
      /* ulChrominanceNMBY */
      pPicture->ulLuminanceNMBY = pUnifiedPicture->stBufferInfo.uiLumaStripeHeight >> 4;
      pPicture->ulChrominanceNMBY = pUnifiedPicture->stBufferInfo.uiChromaStripeHeight >> 4;

      /* ulSourceHorizontalSize (default) */
      /* ulSourceVerticalSize (default) */
      pPicture->ulSourceHorizontalSize = pUnifiedPicture->stBufferInfo.stSource.uiWidth;
      pPicture->ulSourceVerticalSize = pUnifiedPicture->stBufferInfo.stSource.uiHeight;

      /* eFrameRateCode, we want to send the application set default
       * frame rate if the coded frame rate is unknown, so we use the
       * frame rate value from the picture context parameters instead
       * of the PPB directly */
      pPicture->eFrameRateCode = pstPicParms->stTSM.stStatic.eFrameRate;

      BXDM_PPOUT_S_SetMpegType_isr(
               hXdmPP,
               pPictureContext,
               pPicture);

      /* eStripeWidth */
      pPicture->eStripeWidth = pPictureContext->pstUnifiedPicture->stBufferInfo.eStripeWidth;

      /* YCbCrType */
      pPicture->eYCbCrType = pPictureContext->pstUnifiedPicture->stBufferInfo.eYCbCrType;

      BXDM_PPOUT_S_SetPictureOrderCount_isr(
               hXdmPP,
               pPictureContext,
               pPicture);

      /* SW7425-2181: if specified, set the pixel format. */
      if ( true == pPictureContext->pstUnifiedPicture->stBufferInfo.stPixelFormat.bValid )
      {
         pPicture->ePxlFmt = pPictureContext->pstUnifiedPicture->stBufferInfo.stPixelFormat.ePixelFormat;
      }

      /* SW7425-2181: copy the stride from the unified picture. */
      pPicture->ulRowStride = pPictureContext->pstUnifiedPicture->stBufferInfo.uiRowStride;

   }
   else
   {
      pPicture->bMute = pDefaultParams->bMute;

      pPicture->hLuminanceFrameBufferBlock = pDefaultParams->hLuminanceFrameBufferBlock;
      pPicture->ulLuminanceFrameBufferBlockOffset = pDefaultParams->ulLuminanceFrameBufferBlockOffset;

      pPicture->hChrominanceFrameBufferBlock = pDefaultParams->hChrominanceFrameBufferBlock;
      pPicture->ulChrominanceFrameBufferBlockOffset = pDefaultParams->ulChrominanceFrameBufferBlockOffset;

      /* SW7445-586: H265/HEVC interlaced, default to a single frame buffer. */
      pPicture->eBufferFormat = pDefaultParams->eBufferFormat;
      pPicture->hLuminanceBotFieldBufferBlock = pDefaultParams->hLuminanceBotFieldBufferBlock;
      pPicture->ulLuminanceBotFieldBufferBlockOffset = pDefaultParams->ulLuminanceBotFieldBufferBlockOffset;
      pPicture->hChrominanceBotFieldBufferBlock = pDefaultParams->hLuminanceBotFieldBufferBlock;
      pPicture->ulChrominanceBotFieldBufferBlockOffset = pDefaultParams->ulLuminanceBotFieldBufferBlockOffset;

      pPicture->ulLuminanceNMBY = pDefaultParams->ulLuminanceNMBY;
      pPicture->ulChrominanceNMBY = pDefaultParams->ulChrominanceNMBY;
      pPicture->ulSourceHorizontalSize = pDefaultParams->ulSourceHorizontalSize;
      pPicture->ulSourceVerticalSize = pDefaultParams->ulSourceVerticalSize;
      pPicture->eFrameRateCode = pDefaultParams->eFrameRateCode;
      pPicture->eMpegType = pDefaultParams->eMpegType;
      pPicture->eStripeWidth = pDefaultParams->eStripeWidth;
      pPicture->eYCbCrType = pDefaultParams->eYCbCrType;
   }


   BDBG_LEAVE(BXDM_PPOUT_S_SetSourceBufferInformation_isr);
   return;
} /* BXDM_PPOUT_S_SetSourceBufferInformation_isr() */

static void BXDM_PPOUT_S_SetClipping_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPictureContext,
   BAVC_MFD_Picture* pPicture
   )
{
   const BXDM_Picture* pUnifiedPicture = pPictureContext->pstUnifiedPicture;
   bool bCrcMode = hXdmPP->stDMConfig.bCRCMode;
   BAVC_MFD_Picture * pDefaultParams = &( hXdmPP->stDMState.stChannel.stDefaultPicParams );

   BDBG_ENTER(BXDM_PPOUT_S_SetClipping_isr);

   BDBG_ASSERT(pPicture);

   /* ulSourceClipTop */
   /* ulSourceClipLeft */
   pPicture->ulSourceClipTop = pDefaultParams->ulSourceClipTop;
   pPicture->ulSourceClipLeft = pDefaultParams->ulSourceClipLeft;

   if ( pUnifiedPicture )
   {
      if ( ( true == pUnifiedPicture->stClipping.bValid )
           && ( false == bCrcMode ) )
      {
         pPicture->ulSourceHorizontalSize -= (pUnifiedPicture->stClipping.uiLeft + pUnifiedPicture->stClipping.uiRight);
         pPicture->ulSourceVerticalSize -= (pUnifiedPicture->stClipping.uiTop + pUnifiedPicture->stClipping.uiBottom);

         pPicture->ulSourceClipTop = pUnifiedPicture->stClipping.uiTop;
         pPicture->ulSourceClipLeft = pUnifiedPicture->stClipping.uiLeft;
      }
   }

   BDBG_LEAVE(BXDM_PPOUT_S_SetClipping_isr);
   return;
} /* end of BXDM_PPOUT_S_SetClipping_isr() */

static void BXDM_PPOUT_S_SetPanScan_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPictureContext,
   bool bCrcMode,
   BAVC_MFD_Picture* pPicture
   )
{
   const BXDM_Picture* pUnifiedPicture = pPictureContext->pstUnifiedPicture;
   uint32_t uiSelectedElement;

   BDBG_ENTER(BXDM_PPOUT_S_SetPanScan_isr);

   BDBG_ASSERT(pPicture);

   pPicture->i32_HorizontalPanScan = 0;
   pPicture->i32_VerticalPanScan = 0;

   pPicture->ulDisplayHorizontalSize = pPicture->ulSourceHorizontalSize; /* needs to be the post cropped value */
   pPicture->ulDisplayVerticalSize = pPicture->ulSourceVerticalSize; /* needs to be the post cropped value */

   /* Handle Reverse Field Display - The reversal of the source
    * polarity is handled during the PTS calculation done for each
    * element (See BXDM_PPTSM_P_PtsCalculateParameters_isr).  However, we
    * need to reverse the selected element so that the display uses
    * the proper pan-scan vector, if one exists */
   if ( hXdmPP->stDMConfig.bReverseFields )
   {
      uiSelectedElement = pPictureContext->stPicParms.stTSM.stStatic.uiNumElements - (pPictureContext->stPicParms.stDisplay.stDynamic.uiSelectedElement + 1);
   }
   else
   {
      uiSelectedElement = pPictureContext->stPicParms.stDisplay.stDynamic.uiSelectedElement;
   }

   if ( pUnifiedPicture )
   {

      /* SWBLURAY-21461: for VC1, MPEG and AVS a display width/height may be
       * included with the picture data.  Use these values if provided.
       * This check needs to be after the "if (pUnifiedPicture)" to keep coverity happy.
       */
      if ( true == pUnifiedPicture->stBufferInfo.stDisplay.bValid )
      {
         pPicture->ulDisplayHorizontalSize = pUnifiedPicture->stBufferInfo.stDisplay.uiWidth;
         pPicture->ulDisplayVerticalSize = pUnifiedPicture->stBufferInfo.stDisplay.uiHeight;
      }

      if ( ( 0 != pUnifiedPicture->stPanScan.uiCount )
           && ( false == bCrcMode ) )
      {
         uint32_t uiPanScanIndex;

         if ( uiSelectedElement < pUnifiedPicture->stPanScan.uiCount )
         {
            uiPanScanIndex = uiSelectedElement;
         }
         else
         {
            uiPanScanIndex = pUnifiedPicture->stPanScan.uiCount - 1;
         }

         switch ( pUnifiedPicture->stPanScan.stVector[uiPanScanIndex].eType )
         {
            case BXDM_Picture_PanScanVectorType_eSourceWindow:
               pPicture->ulDisplayHorizontalSize = pUnifiedPicture->stPanScan.stVector[uiPanScanIndex].uiWidth;
               pPicture->ulDisplayVerticalSize = pUnifiedPicture->stPanScan.stVector[uiPanScanIndex].uiHeight;

               pPicture->i32_HorizontalPanScan = pUnifiedPicture->stPanScan.stVector[uiPanScanIndex].iHorizontal;
               pPicture->i32_VerticalPanScan = pUnifiedPicture->stPanScan.stVector[uiPanScanIndex].iVertical;
               break;

            case BXDM_Picture_PanScanVectorType_eSourceCrop:
               pPicture->ulDisplayHorizontalSize -= pUnifiedPicture->stPanScan.stVector[uiPanScanIndex].uiWidth;
               pPicture->ulDisplayVerticalSize -= pUnifiedPicture->stPanScan.stVector[uiPanScanIndex].uiHeight;

               pPicture->i32_HorizontalPanScan = pUnifiedPicture->stPanScan.stVector[uiPanScanIndex].iHorizontal;
               pPicture->i32_VerticalPanScan = pUnifiedPicture->stPanScan.stVector[uiPanScanIndex].iVertical;
               break;

            case BXDM_Picture_PanScanVectorType_eDisplayWindow:
               if ( true == pUnifiedPicture->stBufferInfo.stDisplay.bValid )
               {
                  pPicture->ulDisplayHorizontalSize = pUnifiedPicture->stPanScan.stVector[uiPanScanIndex].uiWidth * pPicture->ulSourceHorizontalSize / pUnifiedPicture->stBufferInfo.stDisplay.uiWidth;
                  pPicture->ulDisplayVerticalSize = pUnifiedPicture->stPanScan.stVector[uiPanScanIndex].uiHeight * pPicture->ulSourceVerticalSize / pUnifiedPicture->stBufferInfo.stDisplay.uiHeight;

                  pPicture->i32_HorizontalPanScan = pUnifiedPicture->stPanScan.stVector[uiPanScanIndex].iHorizontal * pPicture->ulSourceHorizontalSize / pUnifiedPicture->stBufferInfo.stDisplay.uiWidth;
                  pPicture->i32_VerticalPanScan = pUnifiedPicture->stPanScan.stVector[uiPanScanIndex].iVertical * pPicture->ulSourceVerticalSize / pUnifiedPicture->stBufferInfo.stDisplay.uiHeight;
               }
               break;

            default:
               break;
         }
      }
   }

   BDBG_LEAVE(BXDM_PPOUT_S_SetPanScan_isr);
   return;
} /* end of BXDM_PPOUT_S_SetPanScan_isr() */

static void BXDM_PPOUT_S_SetDisplayParameters_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPictureContext,
   BAVC_MFD_Picture* pPicture
   )
{
   const BXDM_Picture* pUnifiedPicture = pPictureContext->pstUnifiedPicture;
   BAVC_MFD_Picture * pDefaultParams = &( hXdmPP->stDMState.stChannel.stDefaultPicParams );

   BDBG_ENTER(BXDM_PPOUT_S_SetDisplayParameters_isr);

   BDBG_ASSERT(pPicture);

   /* Set Defaults */
   pPicture->eMatrixCoefficients = pDefaultParams->eMatrixCoefficients;
   pPicture->eColorPrimaries = pDefaultParams->eColorPrimaries;
   pPicture->eTransferCharacteristics = pDefaultParams->eTransferCharacteristics;
   pPicture->ePreferredTransferCharacteristics = pDefaultParams->ePreferredTransferCharacteristics; /* SWSTB-1629 */

   /* SWSTB-3450: initialize the HDR parameters */
   pPicture->ulAvgContentLight      = pDefaultParams->ulAvgContentLight;
   pPicture->ulMaxContentLight      = pDefaultParams->ulMaxContentLight;
   pPicture->stDisplayPrimaries[0]  = pDefaultParams->stDisplayPrimaries[0];
   pPicture->stDisplayPrimaries[1]  = pDefaultParams->stDisplayPrimaries[1];
   pPicture->stDisplayPrimaries[2]  = pDefaultParams->stDisplayPrimaries[2];
   pPicture->stWhitePoint           = pDefaultParams->stWhitePoint;
   pPicture->ulMaxDispMasteringLuma = pDefaultParams->ulMaxDispMasteringLuma;
   pPicture->ulMinDispMasteringLuma = pDefaultParams->ulMinDispMasteringLuma;

   if ( pUnifiedPicture )
   {
      /* SWSTB-1873: support for color range */
      pPicture->eColorRange = pUnifiedPicture->stDisplayInfo.eColorRange;

      if ( true == pUnifiedPicture->stDisplayInfo.bValid )
      {
         BXDM_PictureProvider_ColorOverride * pOverride  = &hXdmPP->stDMConfig.stColorOverride;
         const BXDM_Picture_DisplayInfo * pDisplayInfo = &pUnifiedPicture->stDisplayInfo;
         const BXDM_Picture_HDR * pHDRInfo = &pUnifiedPicture->stHDR;

         bool bUseStreamParameters; /* SWSTB-3450: use the override parameters or the values from the stream? */

         /* Always extract these values from the stream. */
         pPicture->eMatrixCoefficients = pUnifiedPicture->stDisplayInfo.eMatrixCoefficients;
         pPicture->eColorPrimaries = pUnifiedPicture->stDisplayInfo.eColorPrimaries;

#if 0
         pPicture->eTransferCharacteristics = pUnifiedPicture->stDisplayInfo.eTransferCharacteristics;
         pPicture->ePreferredTransferCharacteristics = pUnifiedPicture->stHDR.uiTransferCharacteristics; /* SWSTB-1629 */
#endif

         /* SWSTB-3450: check the HDR transfer characteristic */
         bUseStreamParameters = ( pHDRInfo->uiTransferCharacteristics == BAVC_TransferCharacteristics_eArib_STD_B67 );

         /* SWSTB-3450: check the transfer characteristic */
         bUseStreamParameters |= ( pDisplayInfo->eTransferCharacteristics == BAVC_TransferCharacteristics_eArib_STD_B67  );
         bUseStreamParameters |= ( pDisplayInfo->eTransferCharacteristics == BAVC_TransferCharacteristics_eSmpte_ST_2084 );

         /* SWSTB-3450: check the user specified values. */
         bUseStreamParameters |= ( pOverride->eOverrideMode == BXDM_PictureProvider_ColorOverrideMode_eNone );
         bUseStreamParameters |= (( pOverride->eOverrideMode == BXDM_PictureProvider_ColorOverrideMode_eForce )
                                   && ( pOverride->stSDR.eTransferCharacteristics == BAVC_TransferCharacteristics_eUnknown )
                                   && ( pOverride->stHDR.eTransferCharacteristics == BAVC_TransferCharacteristics_eUnknown ));

         /* SWSTB-3450: select the transfer characterstics based on the preceding. */
         if ( bUseStreamParameters == true )
         {
            pPicture->eTransferCharacteristics          = pDisplayInfo->eTransferCharacteristics;
            pPicture->ePreferredTransferCharacteristics = pHDRInfo->uiTransferCharacteristics;
         }
         else
         {
            pPicture->eTransferCharacteristics          = pOverride->stSDR.eTransferCharacteristics;
            pPicture->ePreferredTransferCharacteristics = pOverride->stHDR.eTransferCharacteristics;
         }

         /* SWSTB-3450: the remaining parameters are handled as follows.
          * If the user specified HDR override parameters copy these into the MFD structure.
          * Else if the content is H265, use the data provided in the stream.
          * Otherwise use the defaults values, these were set up above. */

         if ( BXDM_PictureProvider_ColorOverrideMode_eForce == pOverride->eOverrideMode )
         {
            pPicture->ulAvgContentLight    = pOverride->stHDR.ulAvgContentLight;
            pPicture->ulMaxContentLight    = pOverride->stHDR.ulMaxContentLight;

            pPicture->stDisplayPrimaries[0]    = pOverride->stHDR.stDisplayPrimaries[0];
            pPicture->stDisplayPrimaries[1]    = pOverride->stHDR.stDisplayPrimaries[1];
            pPicture->stDisplayPrimaries[2]    = pOverride->stHDR.stDisplayPrimaries[2];
            pPicture->stWhitePoint             = pOverride->stHDR.stWhitePoint;
            pPicture->ulMaxDispMasteringLuma   = pOverride->stHDR.ulMaxDispMasteringLuma;
            pPicture->ulMinDispMasteringLuma   = pOverride->stHDR.ulMinDispMasteringLuma;
         }
         else if ( BAVC_VideoCompressionStd_eH265 == pUnifiedPicture->stProtocol.eProtocol )
         {
            pPicture->ulAvgContentLight    = pHDRInfo->ulAvgContentLight;
            pPicture->ulMaxContentLight    = pHDRInfo->ulMaxContentLight;

            pPicture->stDisplayPrimaries[0]    = pHDRInfo->stDisplayPrimaries[0];
            pPicture->stDisplayPrimaries[1]    = pHDRInfo->stDisplayPrimaries[1];
            pPicture->stDisplayPrimaries[2]    = pHDRInfo->stDisplayPrimaries[2];
            pPicture->stWhitePoint             = pHDRInfo->stWhitePoint;
            pPicture->ulMaxDispMasteringLuma   = pHDRInfo->ulMaxDispMasteringLuma;
            pPicture->ulMinDispMasteringLuma   = pHDRInfo->ulMinDispMasteringLuma;
         }

      }

   }

   BDBG_LEAVE(BXDM_PPOUT_S_SetDisplayParameters_isr);

   return;

} /* end of BXDM_PPOUT_S_SetDisplayParameters_isr() */

BERR_Code BXDM_PPOUT_P_ComputeSARScaling_isr(
         uint32_t uiOriginalSourceSizeX,
         uint32_t uiOriginalSourceSizeY,
         uint16_t uiOriginalSampleAspectRatioX,
         uint16_t uiOriginalSampleAspectRatioY,
         uint32_t uiEffectiveSourceSizeX,
         uint32_t uiEffectiveSourceSizeY,
         uint16_t *puiEffectiveSampleAspectRatioX,
         uint16_t *puiEffectiveSampleAspectRatioY
         )
{
   BDBG_ASSERT(puiEffectiveSampleAspectRatioX);
   BDBG_ASSERT(puiEffectiveSampleAspectRatioY);

   if ( 0 != uiEffectiveSourceSizeX )
   {
      *puiEffectiveSampleAspectRatioX = ( ( ( uiOriginalSampleAspectRatioX * uiOriginalSourceSizeX * 2048 ) + uiEffectiveSourceSizeX/2 ) / uiEffectiveSourceSizeX);
   }

   if ( 0 != uiEffectiveSourceSizeY )
   {
      *puiEffectiveSampleAspectRatioY = ( ( ( uiOriginalSampleAspectRatioY * uiOriginalSourceSizeY * 2048 ) + uiEffectiveSourceSizeY/2 ) / uiEffectiveSourceSizeY);
   }

   return BERR_TRACE(BERR_SUCCESS);
} /* end of BXDM_PPOUT_P_ComputeSARScaling_isr() */

static void BXDM_PPOUT_S_CalculateHorizontalOverscan_isr(
   uint32_t uiSizeX,
   uint32_t uiSizeY,
   uint32_t uiSampleAspectRatioX,
   uint32_t uiSampleAspectRatioY,
   uint32_t *puiOverscan
   )
{
   uint32_t uiIdealWidth16x9;
   uint32_t uiIdealWidth4x3;
   uint32_t uiOverscan16x9;
   uint32_t uiOverscan4x3;
   uint32_t uiOverscan;

   BDBG_ENTER(BXDM_PPOUT_S_CalculateHorizontalOverscan_isr);

   BDBG_ASSERT(puiOverscan);

   *puiOverscan = 0;

   if ( 0 != uiSampleAspectRatioX )
   {
      /* Determine overscan assuming 16x9 display region*/
      uiIdealWidth16x9 = (( uiSizeY * uiSampleAspectRatioY * 16 ) / ( uiSampleAspectRatioX * 9 ));
      if ( uiIdealWidth16x9 > uiSizeX )
      {
         uiOverscan16x9 = uiSizeX;
      }
      else
      {
         uiOverscan16x9 = uiSizeX - uiIdealWidth16x9;
      }

      /* Determine overscan assuming 4x3 display region*/
      uiIdealWidth4x3 = (( uiSizeY * uiSampleAspectRatioY * 4 ) / ( uiSampleAspectRatioX * 3 ));
      if ( uiIdealWidth4x3 > uiSizeX )
      {
         uiOverscan4x3 = uiSizeX;
      }
      else
      {
         uiOverscan4x3 = uiSizeX - uiIdealWidth4x3;
      }

      /* Pick the smallest resulting overscan */
      if ( uiOverscan4x3 < uiOverscan16x9 )
      {
         uiOverscan = uiOverscan4x3;
      }
      else
      {
         uiOverscan = uiOverscan16x9;
      }

      if ( ( uiOverscan > 0 )
            && ( uiOverscan <= 16 ) )
      {
         *puiOverscan = uiOverscan;
      }

   }     /* end of if( 0 != uiSampleAspectRatioX )*/

   BDBG_LEAVE(BXDM_PPOUT_S_CalculateHorizontalOverscan_isr);
   return;
} /* end of BXDM_PPOUT_S_CalculateHorizontalOverscan_isr() */

static void BXDM_PPOUT_S_SetAspectRatio_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPictureContext,
   BAVC_MFD_Picture* pPicture
   )
{
   const BXDM_Picture* pUnifiedPicture = pPictureContext->pstUnifiedPicture;
   BAVC_MFD_Picture * pDefaultParams = &( hXdmPP->stDMState.stChannel.stDefaultPicParams );

   uint32_t uiEffectiveSourceSizeX;
   uint32_t uiEffectiveSourceSizeY;
   uint32_t uiOriginalSourceSizeX;
   uint32_t uiOriginalSourceSizeY;

   bool bResizeRequired = false;

   uint32_t uiEffectiveOverscan;
   uint16_t uiEffectiveSampleAspectRatioX;
   uint16_t uiEffectiveSampleAspectRatioY;

   uint32_t uiOriginalOverscan = 0;
   uint16_t uiOriginalSampleAspectRatioX=0;
   uint16_t uiOriginalSampleAspectRatioY=0;

   BDBG_ENTER(BXDM_PPOUT_S_SetAspectRatio_isr);

   BDBG_ASSERT(pPicture);

   /* EffectiveSourceSize: The dimensions of the actual picture buffer
    * EffectiveOverscan: The overscan of the coded source size ( = (OriginalOverscan * CodedSourceSize + DisplaySourceSize/2) / DisplaySourceSize )
    * EffectiveSAR: The SAR that includes scaling from EffectiveSourceSize to OriginalSourceSize and the OriginalSAR
    *
    * OriginalSourceSize: The dimensions of the original picture buffer
    *    * VC1: display size from vc1 extension
    *    * Others: EffectiveSourceSize
    * OriginalOverscan: The overscan of the OriginalSourceSize
    * OriginalSAR: The SAR that applies to the original source size
    *
    * 1) Compute OriginalOverscan
    * 2) Compute CodedOverscan
    * 3) Apply cropping based on CodedOverscan
    * 4) Compute
    */

   pPicture->eAspectRatio = pDefaultParams->eAspectRatio;
   pPicture->uiSampleAspectRatioX = pDefaultParams->uiSampleAspectRatioX;
   pPicture->uiSampleAspectRatioY = pDefaultParams->uiSampleAspectRatioY;

   if ( ( NULL != pUnifiedPicture )
        && ( true == pUnifiedPicture->stAspectRatio.bValid ) )
   {
      /* Populate Dimensions Parameters */
      uiEffectiveSourceSizeX = pPicture->ulSourceHorizontalSize;
      uiEffectiveSourceSizeY = pPicture->ulSourceVerticalSize;

      uiOriginalSourceSizeX = uiEffectiveSourceSizeX;
      uiOriginalSourceSizeY = uiEffectiveSourceSizeY;

      if ( true == pUnifiedPicture->stBufferInfo.stDisplay.bValid )
      {
         uiOriginalSourceSizeX = pUnifiedPicture->stBufferInfo.stDisplay.uiWidth;
         uiOriginalSourceSizeY = pUnifiedPicture->stBufferInfo.stDisplay.uiHeight;
      }

      if ( ( uiEffectiveSourceSizeX != uiOriginalSourceSizeX )
           || ( uiEffectiveSourceSizeY != uiOriginalSourceSizeY ) )
      {
         bResizeRequired = true;

         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eOUT, "%x:[%02x.%03x] Source Scaling Required: %4dx%4d --> %4dx%4d",
                                          hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                          BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                          pPictureContext->stPicParms.uiPPBIndex & 0xFFF,
                                          uiEffectiveSourceSizeX,
                                          uiEffectiveSourceSizeY,
                                          uiOriginalSourceSizeX,
                                          uiOriginalSourceSizeY ));
      }

      pPicture->eAspectRatio = pUnifiedPicture->stAspectRatio.eAspectRatio;
      uiOriginalSampleAspectRatioX = pUnifiedPicture->stAspectRatio.uiSampleAspectRatioX;
      uiOriginalSampleAspectRatioY = pUnifiedPicture->stAspectRatio.uiSampleAspectRatioY;

      /* Handle SAR Overscan and Rescaling */
      if ( BFMT_AspectRatio_eSAR == pPicture->eAspectRatio )
      {
         /* SW7445-392: add support for H265/HEVC */
         bool bAVCProtocol = ( BAVC_VideoCompressionStd_eH264 == pUnifiedPicture->stProtocol.eProtocol );
         bAVCProtocol |= ( BAVC_VideoCompressionStd_eH265 == pUnifiedPicture->stProtocol.eProtocol );

         /* If applicable, Calculate Original Overscan */
         if ( ( false == hXdmPP->stDMConfig.bCRCMode )
              && ( uiOriginalSampleAspectRatioX != uiOriginalSampleAspectRatioY )  /* Not Square Pixel */
              && ( false == ( ( true == bAVCProtocol ) /* Not AVC protocol with overscan bit indicating no overscan */
                              && ( true == ( pUnifiedPicture->stOverscan.bValid ) )
                              && ( false == ( pUnifiedPicture->stOverscan.bOverscanAppropriate ) ) ) )
              /* SW7405-4703: bypass horizontal overscan calculation if desired */
              && (BXDM_PictureProvider_HorizontalOverscanMode_eDisable != hXdmPP->stDMConfig.eHorizontalOverscanMode)
            )
         {
            BXDM_PPOUT_S_CalculateHorizontalOverscan_isr(
                     uiOriginalSourceSizeX,
                     uiOriginalSourceSizeY,
                     uiOriginalSampleAspectRatioX,
                     uiOriginalSampleAspectRatioY,
                     &uiOriginalOverscan
                     );
         }

         if ( 0 != uiOriginalOverscan )
         {
            /* If applicable, Calculate Effective Overscan */
            if ( false == bResizeRequired )
            {
               uiEffectiveOverscan = uiOriginalOverscan;
            }
            else
            {
               uiEffectiveOverscan = ( ( ( uiOriginalOverscan * uiEffectiveSourceSizeX ) + uiOriginalSourceSizeX/2 ) / uiOriginalSourceSizeX );
            }

            /* Populate Overscan */
            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eOUT, "%x:[%02x.%03x] Applying overscan: %4dx%4d + sar:[%4d:%4d] --> %4dx%4d",
                                                   hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                   BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                   pPictureContext->stPicParms.uiPPBIndex & 0xFFF,
                                                   uiOriginalSourceSizeX,
                                                   uiOriginalSourceSizeY,
                                                   uiOriginalSampleAspectRatioX,
                                                   uiOriginalSampleAspectRatioY,
                                                   uiOriginalSourceSizeX - uiOriginalOverscan,
                                                   uiOriginalSourceSizeY ));

            uiOriginalSourceSizeX -= uiOriginalOverscan;
            uiEffectiveSourceSizeX -= uiEffectiveOverscan;

            pPicture->ulSourceHorizontalSize -= uiEffectiveOverscan;
            pPicture->ulSourceClipLeft += (uiEffectiveOverscan+1)/2;
         }

         /* Calculate Effective SAR */

         uiEffectiveSampleAspectRatioX = uiOriginalSampleAspectRatioX;
         uiEffectiveSampleAspectRatioY = uiOriginalSampleAspectRatioY;

         if ( true == bResizeRequired )
         {
            /* SW7405-2991: Handle VC1 source -> display scaling */
            BXDM_PPOUT_P_ComputeSARScaling_isr(
                     uiOriginalSourceSizeX,
                     uiOriginalSourceSizeY,
                     uiOriginalSampleAspectRatioX,
                     uiOriginalSampleAspectRatioY,
                     uiEffectiveSourceSizeX,
                     uiEffectiveSourceSizeY,
                     &uiEffectiveSampleAspectRatioX,
                     &uiEffectiveSampleAspectRatioY
                     );

            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eOUT, "%x:[%02x.%03x] SAR Based Scaling Conversion: %4dx%4d --> %4dx%4d",
                                          hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                          BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                          pPictureContext->stPicParms.uiPPBIndex & 0xFFF,
                                          uiOriginalSampleAspectRatioX,
                                          uiOriginalSampleAspectRatioY,
                                          uiEffectiveSampleAspectRatioX,
                                          uiEffectiveSampleAspectRatioY ));

         }

         /* Populate SAR */
         pPicture->uiSampleAspectRatioX = uiEffectiveSampleAspectRatioX;
         pPicture->uiSampleAspectRatioY = uiEffectiveSampleAspectRatioY;
      }
   }

   BDBG_LEAVE(BXDM_PPOUT_S_SetAspectRatio_isr);
   return;
} /* end of BXDM_PPOUT_S_SetAspectRatio_isr() */

static void BXDM_PPOUT_S_SetDigitalNoiseReduction_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPictureContext,
   BAVC_MFD_Picture* pPicture
   )
{
   const BXDM_Picture* pUnifiedPicture = pPictureContext->pstUnifiedPicture;
   BAVC_MFD_Picture * pDefaultParams = &( hXdmPP->stDMState.stChannel.stDefaultPicParams );

   BDBG_ENTER(BXDM_PPOUT_S_SetDigitalNoiseReduction_isr);

   BDBG_ASSERT(pPicture);

   /* Set defaults */
   pPicture->ulAdjQp = pDefaultParams->ulAdjQp;

   if ( ( NULL != pUnifiedPicture )
       && ( pUnifiedPicture->stDigitalNoiseReduction.bValid ) )
   {
      pPicture->ulAdjQp = pUnifiedPicture->stDigitalNoiseReduction.uiAdjustedQuantizationParameter;
   }

   BDBG_LEAVE(BXDM_PPOUT_S_SetDigitalNoiseReduction_isr);
   return;
} /* end of BXDM_PPOUT_S_SetDigitalNoiseReduction_isr() */

static void BXDM_PPOUT_S_SetRangeRemapping_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPictureContext,
   BAVC_MFD_Picture* pPicture
   )
{
   const BXDM_Picture* pUnifiedPicture = pPictureContext->pstUnifiedPicture;
   BAVC_MFD_Picture * pDefaultParams = &( hXdmPP->stDMState.stChannel.stDefaultPicParams );

   BDBG_ENTER(BXDM_PPOUT_S_SetRangeRemapping_isr);

   BDBG_ASSERT(pPicture);

   /* Set defaults */
   pPicture->ulLumaRangeRemapping = pDefaultParams->ulLumaRangeRemapping;
   pPicture->ulChromaRangeRemapping = pDefaultParams->ulChromaRangeRemapping;

   if ( ( NULL != pUnifiedPicture )
        && ( pUnifiedPicture->stRangeRemapping.bValid ) )
   {
      pPicture->ulLumaRangeRemapping = pUnifiedPicture->stRangeRemapping.uiLuma;
      pPicture->ulChromaRangeRemapping = pUnifiedPicture->stRangeRemapping.uiChroma;
   }

   BDBG_LEAVE(BXDM_PPOUT_S_SetRangeRemapping_isr);
   return;
} /* end of BXDM_PPOUT_S_SetRangeRemapping_isr() */


static void BXDM_PPOUT_S_3DFixup_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstSelectedPicture,
   BAVC_MFD_Picture** pMFDPicture
   )
{
   BAVC_MFD_Picture* pBaseMFDPicture;
   BAVC_MFD_Picture* pExtendedMFDPicture;
   const BXDM_Picture* pUnifiedPicture = pstSelectedPicture->pstUnifiedPicture;

   bool bFrame0IsLeft;
   BXDM_Picture_Orientation   eOrientation;

   BDBG_ENTER(BXDM_PPOUT_S_3DFixup_isr);

   /* Handle Dependent Picture Data (e.g. for 3D content) */
   pBaseMFDPicture = *pMFDPicture;
   pBaseMFDPicture->eOrientation = BFMT_Orientation_e2D;
   pBaseMFDPicture->pEnhanced = NULL;
   pBaseMFDPicture->pNext = NULL;    /* TODO: get BD to use "pEnhanced" */


   if ( NULL == pUnifiedPicture )
   {
      return;
   }

   /* SW7422-72: Generate the SEI message callback before filling in the MFD picture structure.
    * The middleware/application can use the contents of the SEI message to determine
    * if BXDM_PictureProvider_Set3D_isr needs to be called to force a specific
    * 3D orientation.
    */
   BXDM_PPCB_P_ExecuteSingleCallbackExt_isr(
               hXdmPP,
               pLocalState,
               pstSelectedPicture,
               BXDM_PictureProvider_Callback_ePictureExtensionData
               );

   /* Determine whether to use the 3D orientation specified by the
    * middleware/application or what has been delivered in-band by AVD.
    */
   if ( true == hXdmPP->stDMConfig.st3DSettings.bOverrideOrientation )
   {
      /* Use the orientation specified by the middleware/application. */

      switch( hXdmPP->stDMConfig.st3DSettings.eOrientation )
      {
         case BXDM_PictureProvider_Orientation_eLeftRightFullFrame:
            eOrientation   = BXDM_Picture_Orientation_e3D_FullFrame;
            bFrame0IsLeft  = true;
            break;

         case BXDM_PictureProvider_Orientation_eLeftRightEnhancedResolution:
            /* SW7425-966: This really means MVC/MRE content, i.e. a pair
             * of pictures where the base picture is SD LeftRight and the
             * dependent picture is enhancement data LeftRight
             */
            eOrientation   = BXDM_Picture_Orientation_e3D_FullFrame;
            bFrame0IsLeft  = true;
            break;

         case BXDM_PictureProvider_Orientation_eRightLeftFullFrame:
            eOrientation   = BXDM_Picture_Orientation_e3D_FullFrame;
            bFrame0IsLeft  = false;
            break;

         case BXDM_PictureProvider_Orientation_eLeftRight:
            eOrientation   = BXDM_Picture_Orientation_e3D_SideBySide;
            bFrame0IsLeft  = true;
            break;

         case BXDM_PictureProvider_Orientation_eOverUnder:
            eOrientation   = BXDM_Picture_Orientation_e3D_TopBottom;
            bFrame0IsLeft  = true;
            break;

         case BXDM_PictureProvider_Orientation_e2D:
         default:
            eOrientation   = BXDM_Picture_Orientation_e2D;
            bFrame0IsLeft  = true;
            break;
      }
   }
   else
   {
      /* Use the orientation delivered in-band by AVD. */

      eOrientation = pUnifiedPicture->st3D.eOrientation;

      if ( BXDM_Picture_FrameRelationship_eFrame0Left == pUnifiedPicture->st3D.eFrameRelationship
          || BXDM_Picture_FrameRelationship_eUnknown == pUnifiedPicture->st3D.eFrameRelationship
         )
      {
         bFrame0IsLeft = true;
      }
      else
      {
         bFrame0IsLeft = false;
      }
   }

   if ( BXDM_Picture_Orientation_e3D_FullFrame == eOrientation )
   {
      if ( BXDM_PICTURE_MAX_3D_BUFFERS > hXdmPP->uiMFDPictureCount )
      {
         BXVD_DBG_ERR(hXdmPP, ("%x:[%02x.%03x] BXDM_PPOUT_S_3DFixup_isr:: only %d buffers created in BXDM_PictureProvider_Create, 3D needs %d",
                                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                pstSelectedPicture->stPicParms.uiPPBIndex & 0xFFF,
                                                hXdmPP->uiMFDPictureCount,
                                                BXDM_PICTURE_MAX_3D_BUFFERS));
         goto Done;
      }

      /* A pair of frames. */
      if ( NULL != pUnifiedPicture->pNextPicture )
      {
         /* Use entry '1' of the astMFDPicture array for the extended picture. */
         pExtendedMFDPicture = &(hXdmPP->astMFDPicture[ 1 ]);

         /* Copy the base MFD Picture into the extended one */
         *pExtendedMFDPicture = *pBaseMFDPicture;

         /* Set the fields that are different for the extended MFD picture. */
         pUnifiedPicture = (BXDM_Picture *) pUnifiedPicture->pNextPicture;

         /* pLuminanceFrameBufferAddress */
         pExtendedMFDPicture->hLuminanceFrameBufferBlock = pUnifiedPicture->stBufferInfo.hLuminanceFrameBufferBlock;
         pExtendedMFDPicture->ulLuminanceFrameBufferBlockOffset =  pUnifiedPicture->stBufferInfo.ulLuminanceFrameBufferBlockOffset;

         /* pChrominanceFrameBufferAddress */
         pExtendedMFDPicture->hChrominanceFrameBufferBlock = pUnifiedPicture->stBufferInfo.hChrominanceFrameBufferBlock;
         pExtendedMFDPicture->ulChrominanceFrameBufferBlockOffset =  pUnifiedPicture->stBufferInfo.ulChrominanceFrameBufferBlockOffset;

         {
            /* SW7445-586: H265/HEVC interlaced doesn't currently support 3D */
            BAVC_MFD_Picture * pDefaultParams = &( hXdmPP->stDMState.stChannel.stDefaultPicParams );

            pExtendedMFDPicture->eBufferFormat = pDefaultParams->eBufferFormat;
            pExtendedMFDPicture->hLuminanceBotFieldBufferBlock = pDefaultParams->hLuminanceBotFieldBufferBlock;
            pExtendedMFDPicture->ulLuminanceBotFieldBufferBlockOffset = pDefaultParams->ulLuminanceBotFieldBufferBlockOffset;
            pExtendedMFDPicture->hChrominanceBotFieldBufferBlock = pDefaultParams->hLuminanceBotFieldBufferBlock;
            pExtendedMFDPicture->ulChrominanceBotFieldBufferBlockOffset = pDefaultParams->ulLuminanceBotFieldBufferBlockOffset;
         }

         /* If frame 0 is the right picture, toggle the pointers.
          * VDC always wants the frame 0 to be the left picture.
          */
         if ( false == bFrame0IsLeft )
         {
            *pMFDPicture = &(hXdmPP->astMFDPicture[1]);
            pBaseMFDPicture = *pMFDPicture;
            pExtendedMFDPicture = &(hXdmPP->astMFDPicture[ 0 ]);
         }

         /* Set the orientations */
         if ( true == hXdmPP->stDMConfig.st3DSettings.bOverrideOrientation
               && BXDM_PictureProvider_Orientation_eLeftRightEnhancedResolution == hXdmPP->stDMConfig.st3DSettings.eOrientation
            )
         {
            /* SW7425-966: This really means MVC/MRE content */
            pBaseMFDPicture->eOrientation = BFMT_Orientation_e3D_LeftRight;
            pExtendedMFDPicture->eOrientation = BFMT_Orientation_eLeftRight_Enhanced;
         }
         else
         {
            pBaseMFDPicture->eOrientation = BFMT_Orientation_e3D_Left;
            pExtendedMFDPicture->eOrientation = BFMT_Orientation_e3D_Right;
         }

         /* Link the extended MFD structure to the base structure.  */
         pBaseMFDPicture->pEnhanced = pExtendedMFDPicture;
         pExtendedMFDPicture->pEnhanced = NULL;

         /* To support the original MVC behavior on the older DVD chips,
          * set "pNext" as well as "pEnhanced"
          */
         if ( true == hXdmPP->stDMConfig.st3DSettings.bSetNextPointer )
         {
            pBaseMFDPicture->pNext = pExtendedMFDPicture;
            pExtendedMFDPicture->pNext = NULL;
         }
      }
      else
      {
         /* We only want to deliver this message once per StartDecode to avoid flooding the console.
          * In addition, we don't want to deliver this message for a picture that was held from the
          * previous decode, i.e. due to "hold last picture".
          */
         if ( false == pstSelectedPicture->stPicParms.stDisplay.stDynamic.bHolding
             && false == hXdmPP->stDMState.stDecode.bMissingSecondFrameMessageDelivered )
         {
            hXdmPP->stDMState.stDecode.bMissingSecondFrameMessageDelivered = true;

            BXVD_DBG_ERR(hXdmPP, ("%x:[%02x.%03x] BXDM_PPOUT_S_3DFixup_isr:: 3D pair does not have the second frame.",
                                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                pstSelectedPicture->stPicParms.uiPPBIndex & 0xFFF));
         }
      }
   }
   else
   {
      /* A Single frame. The only packed 3D formats supported by VDC are SideBySide or TopBottom
       * with frame 0 as the left frame.  Treat all other cases as 2D.
       */
      switch( eOrientation )
      {
         case BXDM_Picture_Orientation_e3D_SideBySide:
            if ( true == bFrame0IsLeft )
            {
               pBaseMFDPicture->eOrientation = BFMT_Orientation_e3D_LeftRight;
            }
            else
            {
               pBaseMFDPicture->eOrientation = BFMT_Orientation_e2D;

               BXVD_DBG_WRN(hXdmPP, ("%x:[%02x.%03x] Unsupported 3D orientation: right/left side-by-side overridden to 2D",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pstSelectedPicture->stPicParms.uiPPBIndex & 0xFFF
                              ));
            }
            break;

         case BXDM_Picture_Orientation_e3D_TopBottom:
            if ( true == bFrame0IsLeft )
            {
               pBaseMFDPicture->eOrientation = BFMT_Orientation_e3D_OverUnder;
            }
            else
            {
               pBaseMFDPicture->eOrientation = BFMT_Orientation_e2D;

               BXVD_DBG_WRN(hXdmPP, ("%x:[%02x.%03x] Unsupported 3D orientation: right/left top-bottom overridden to 2D",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pstSelectedPicture->stPicParms.uiPPBIndex & 0xFFF
                              ));
            }
            break;

         case BXDM_Picture_Orientation_e3D_Checker:
            pBaseMFDPicture->eOrientation = BFMT_Orientation_e2D;

            BXVD_DBG_WRN(hXdmPP, ("%x:[%02x.%03x] Unsupported 3D orientation: checker overridden to 2D",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pstSelectedPicture->stPicParms.uiPPBIndex & 0xFFF
                              ));

            break;

         case BXDM_Picture_Orientation_e3D_Column:
            pBaseMFDPicture->eOrientation = BFMT_Orientation_e2D;

            BXVD_DBG_WRN(hXdmPP, ("%x:[%02x.%03x] Unsupported 3D orientation: column overridden to 2D",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pstSelectedPicture->stPicParms.uiPPBIndex & 0xFFF
                              ));
            break;

         case BXDM_Picture_Orientation_e3D_Row:
            pBaseMFDPicture->eOrientation = BFMT_Orientation_e2D;

            BXVD_DBG_WRN(hXdmPP, ("%x:[%02x.%03x] Unsupported 3D orientation: row overridden to 2D",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pstSelectedPicture->stPicParms.uiPPBIndex & 0xFFF
                              ));
            break;

         case BXDM_Picture_Orientation_e2D:
         default:
            pBaseMFDPicture->eOrientation = BFMT_Orientation_e2D;
            break;
      }
   }

Done:

   BDBG_LEAVE(BXDM_PPOUT_S_3DFixup_isr);

   return;

}     /* end of BXDM_PPOUT_S_3DFixup_isr() */

/* CDSTRMANA-1083: for field pairs, the vertical size is the height of both fields.
 * This adjustment was originally done in VDC.  It was moved here so that the correct
 * stream size is reported to the application, in particular "ulDisplayVerticalSize". */

static void BXDM_PPOUT_S_HEVCInterlacedFixup_isr(
         const BXDM_PictureProvider_Handle hXdmPP,
         BXDM_PictureProvider_P_Picture_Context *pPictureContext,
         BAVC_MFD_Picture* pPicture
         )
{
   BSTD_UNUSED(hXdmPP);
   BSTD_UNUSED(pPictureContext);

   /* CDSTRMANA-1083: for field pairs, the vertical size is the height of both fields. */
   if ( BAVC_DecodedPictureBuffer_eFieldsPair == pPicture->eBufferFormat )
   {
      pPicture->ulSourceVerticalSize *= 2;
      pPicture->ulDisplayVerticalSize *= 2;
      pPicture->ulSourceClipTop *= 2;

      if ( BAVC_DecodedPictureBuffer_eFieldsPair == pPicture->eBufferFormat
            && BAVC_BarDataType_eTopBottom == pPicture->eBarDataType )
      {
         pPicture->ulTopLeftBarValue  *= 2;
         pPicture->ulBotRightBarValue *= 2;
      }

   }

}


void BXDM_PPOUT_P_CalculateStaticVdcData_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPictureContext
   )
{
   BAVC_MFD_Picture* pMFDPicture = &pPictureContext->stMFDPicture;

   /* Reset local MFD Picture */
   *pMFDPicture = hXdmPP->stDMState.stChannel.stDefaultPicParams;

   /* Source Information (hHeap, pLuminanceFrameBufferAddress,
    * pChrominanceFrameBufferAddress, eStripeWidth, ulLuminanceNMBY,
    * ulChrominanceNMBY, ulSourceHorizontalSize, ulSourceVerticalSize,
    * eFrameRateCode, eMpegType, eYCbCrType) */
   BXDM_PPOUT_S_SetSourceBufferInformation_isr(
      hXdmPP,
      pPictureContext,
      pMFDPicture
      );

#if 0
   BKNI_Printf("[%d/%d - %p]\n", pMFDPicture->eSourcePolarity, pMFDPicture->eInterruptPolarity, pMFDPicture->pLuminanceFrameBufferAddress);
#endif

   /* Clipping (ulSourceHorizontalSize, ulSourceVerticalSize,
    * ulSourceClipTop, ulSourceClipLeft) */
   BXDM_PPOUT_S_SetClipping_isr(
      hXdmPP,
      pPictureContext,
      pMFDPicture
      );

   /* Aspect Ratio (eAspectRatio, uiSampleAspectRatioX,
    * uiSampleAspectRatioY) */
   BXDM_PPOUT_S_SetAspectRatio_isr(
      hXdmPP,
      pPictureContext,
      pMFDPicture
      );

   /* Pan Scan (i32_HorizontalPanScan, i32_VerticalPanScan,
    * ulDisplayHorizontalSize, ulDisplayVerticalSize) */
   BXDM_PPOUT_S_SetPanScan_isr(
      hXdmPP,
      pPictureContext,
      hXdmPP->stDMConfig.bCRCMode,
      pMFDPicture
      );

   /* Display Parameters (eMatrixCoefficients, eColorPrimaries,
    * eTransferCharacteristics) */
   BXDM_PPOUT_S_SetDisplayParameters_isr(
      hXdmPP,
      pPictureContext,
      pMFDPicture
      );

   /* ulAdjQp */
   BXDM_PPOUT_S_SetDigitalNoiseReduction_isr(
      hXdmPP,
      pPictureContext,
      pMFDPicture
      );

   /* Range Re-mapping (ulLumaRangeRemapping,
    * ulChromaRangeRemapping) */
   BXDM_PPOUT_S_SetRangeRemapping_isr(
      hXdmPP,
      pPictureContext,
      pMFDPicture
      );

   /* Progressive Flags (bFrameProgressive, bStreamProgressive) */
   /* SW7405-5549: moved to BXDM_PPOUT_P_CalculateVdcData_isr */
   /*pMFDPicture->bFrameProgressive = ( BXDM_PictureProvider_P_RepeatMode_eFrame == pPictureContext->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode );*/
   pMFDPicture->bStreamProgressive = ( BXDM_Picture_Sequence_eProgressive == pPictureContext->pstUnifiedPicture->stPictureType.eSequence );

   /* SW7425-44: parameters for encoder ... */
   pMFDPicture->ePictureType = pPictureContext->pstUnifiedPicture->stPictureType.eCoding;

   /* SW7425-2247: support for "bar data". */
   pMFDPicture->eBarDataType        = pPictureContext->pstUnifiedPicture->stBarData.eBarDataType;
   pMFDPicture->ulTopLeftBarValue   = pPictureContext->pstUnifiedPicture->stBarData.uiTopLeftBarValue;
   pMFDPicture->ulBotRightBarValue  = pPictureContext->pstUnifiedPicture->stBarData.uiBotRightBarValue;

   /* SW7445-744: add support for 10 bit picture buffers. */
   /* SWSTB-457: specify the depth for both the lumna and chroma buffers. */
   switch ( pPictureContext->pstUnifiedPicture->stBufferInfo.eLumaBitDepth )
   {
      case BXDM_Picture_VideoBitDepth_e10Bit:
         pMFDPicture->eBitDepth = BAVC_VideoBitDepth_e10Bit;
         break;

      case BXDM_Picture_VideoBitDepth_e9Bit:
         pMFDPicture->eBitDepth = BAVC_VideoBitDepth_e9Bit;
         break;

      case BXDM_Picture_VideoBitDepth_e8Bit:
      default:
         pMFDPicture->eBitDepth = BAVC_VideoBitDepth_e8Bit;
         break;
   }

   switch ( pPictureContext->pstUnifiedPicture->stBufferInfo.eChromaBitDepth )
   {
      case BXDM_Picture_VideoBitDepth_e10Bit:
         pMFDPicture->eChromaBitDepth = BAVC_VideoBitDepth_e10Bit;
         break;

      case BXDM_Picture_VideoBitDepth_e9Bit:
         pMFDPicture->eChromaBitDepth = BAVC_VideoBitDepth_e9Bit;
         break;

      case BXDM_Picture_VideoBitDepth_e8Bit:
      default:
         pMFDPicture->eChromaBitDepth = BAVC_VideoBitDepth_e8Bit;
         break;
   }


   /* SWSTB-3950: add support of passing synchronous meta data. */
   {
      const BXDM_Picture_ExtensionInfo * pstMeta = &(pPictureContext->pstUnifiedPicture->stExtensionInfo);

      uint32_t i;

      pMFDPicture->stHdrMetadata.eType = BAVC_HdrMetadataType_eUnknown;
      pMFDPicture->stHdrMetadata.ulSize = 0;
      pMFDPicture->stHdrMetadata.pData = NULL;

      for ( i=0; i < pstMeta->uiCount; i++ )
      {
         if ( pstMeta->astExtensionData[i].eType == BXDM_Picture_ExtensionType_eMetaData )
         {
            switch( pstMeta->astExtensionData[i].data.stMetaDataHdr.eType )
            {
               /* This check is really only needed if the Unified Picture was NOT generated
                * by XVD, i.e. if the picture came from a decoder other than AVD/HVD.
                * XVD will only forward supported meta data packets. */
               case BAVC_HdrMetadataType_eDrpu:
               case BAVC_HdrMetadataType_eTch_Cvri:
               case BAVC_HdrMetadataType_eTch_Cri:
                  pMFDPicture->stHdrMetadata.eType = pstMeta->astExtensionData[i].data.stMetaDataHdr.eType;
                  pMFDPicture->stHdrMetadata.ulSize = pstMeta->astExtensionData[i].data.stMetaDataHdr.uiSize;
                  pMFDPicture->stHdrMetadata.pData = pstMeta->astExtensionData[i].data.stMetaDataHdr.pData;
                  break;

               default:
                  break;
            }
         }
      }

   }

   /* Only update the parameters BEFORE the first PPB has passed TSM */
   /* Update the Picture Parameter structure with the data for this PPB. */
   if (( true == hXdmPP->stDMState.stDecode.bFirstPPBSeen )
       && (false == hXdmPP->stDMState.stDecode.bFirstPPBHasBeenDisplayed))
   {
      BXDM_PPOUT_S_UpdatePictureParameters_isr(
         hXdmPP,
         pMFDPicture,
         pPictureContext,
         true
         );
   }
} /* end of BXDM_PPOUT_P_CalculateStaticVdcData_isr() */

static void BXDM_PPOUT_S_SetStallStcAndIgnorePicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture,
   BAVC_MFD_Picture* pMFDPicture
   )
{
   BXDM_PictureProvider_P_State_NonRealTime * pstNonRealTime = &(hXdmPP->stDMState.stDecode.stNonRealTime);

   /* SW7425-44: Set the bIgnorePicture flag if we are repeating because of a stall scenario.
    * Set the bStallStc flag if video decoder is underflowing
    * See: http://twiki-01.broadcom.com/bin/view/Bsema/DisplayManagerV2TSM#Transcode_meta_data_sent_to_VDC
    */

   /* Calculate STC Stall */
   if ( BXDM_PictureProvider_P_DecodeState_eStopped == hXdmPP->stDMState.stChannel.eDecodeState )
   {
      /* SW7425-5929: between stop/start decode, "bIgnorePicture" and "bStallStc" should be
       * set to "true".  Set "bStallStc" here, "bIgnorePicture" will be set below.
       * This path gets exercised when the channel change mode is "hold last picture" and
       * the decoder is stopped.  When the CCM is "mute", "bStallStc" and "bIgnorePicture"
       * are set to "true" in BXDM_PPOUT_P_CalculateVdcData_isr.
       */
      pMFDPicture->bStallStc = true;
   }
   else
   {
      pMFDPicture->bStallStc = false;

      /* SW7425-2270: remain in the "ignore" state until there are pictues on the queue. */
      if ( true == pstNonRealTime->bIgnoringUnderflow )
      {
         pstNonRealTime->bIgnoringUnderflow = ( true == pLocalState->bPictureQueueIsEmpty );
      }

      /* SW7425-2270: only look to set bStallStc if the system is NOT ignoring underflows. */
      if ( false == pstNonRealTime->bIgnoringUnderflow )
      {
         /* This encoder stalled (underflow) */
         if ( ( pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated >= (int32_t) pstPicture->stPicParms.stTSM.stStatic.stPTSDelta.uiWhole ) /* next picture not available */
                && ( true == pLocalState->bPictureQueueIsEmpty ) ) /* decoder underflow */
         {
            if ( true == hXdmPP->stDMConfig.bIgnoreNRTUnderflow )
            {
               BDBG_MSG(("%x:[%02x.%03x] Ignoring Underflow",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pstPicture->stPicParms.uiPPBIndex & 0xFFF
                              ));

               /* SW7425-2270: "bIgnoreNRTUnderflow" is a one shot, clear the flag once
                * the system enters this state.
                */
               hXdmPP->stDMConfig.bIgnoreNRTUnderflow = false;
               pstNonRealTime->bIgnoringUnderflow = true;
            }
            else
            {
               BDBG_MSG(("%x:[%02x.%03x] Underflow STC Stall",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pstPicture->stPicParms.uiPPBIndex & 0xFFF
                              ));

               pMFDPicture->bStallStc = true;
            }
         }

#if 0 /* SW7445-224: Remove the FIC bStallStc logic from XDM. */

         /* SW7425-767: Prevent field inversion when STC is stalled, by stalling until the T/B polarity matches */
         else if ( ( ( true == pstNonRealTime->bLastStcValid ) /* dSTC == 0 */
                     && ( pstNonRealTime->uiLastStc == pLocalState->uiStcSnapshot ) )
                   && ( pstNonRealTime->ePolarityWhenDeltaStcWasNonZero != pLocalState->eVsyncPolarity ) ) /* Polarity mismatch */
         {
            BDBG_MSG(("%x:[%02x.%03x] FIC STC Stall: last STC:%08x current STC:%08x last polarity:%d current polarity:%d",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                              pstNonRealTime->uiLastStc,
                              pLocalState->uiStcSnapshot,
                              pstNonRealTime->ePolarityWhenDeltaStcWasNonZero,
                              pLocalState->eVsyncPolarity
                              ));

            pMFDPicture->bStallStc = true;
         }
#endif
      }
   }

   /* Calculate Ignore Picture */
   {
      /* SWSTB-2914: revamp the ignore picture logic.   Need to enure that a picture is
       * encoded for each unique value of the STC.  This fixes a bug that occured when
       * transitioning out of a delivery queue underflow AND the new picture on the
       * queue did NOT pass TSM. */

      /* If the STC has advanced, then a picture has not been encoded on this STC value. */

      if ( pstNonRealTime->uiLastStc != pLocalState->uiStcSnapshot )
      {
         pstNonRealTime->bPictureEncodedOnThisSTC = false;
      }

      /* Set bIgnorePicture if:
       * - XDM is requesting that the STC be stalled
       * - OR a picture has already been encoded on this STC value. */

      pMFDPicture->bIgnorePicture = pMFDPicture->bStallStc;
      pMFDPicture->bIgnorePicture |= pstNonRealTime->bPictureEncodedOnThisSTC;

      /* Set bPictureEncodedOnThisSTC if:
       * - a picture has not already been encoded on this STC
       * - AND this picture is not being ignored. */

      if ( false == pstNonRealTime->bPictureEncodedOnThisSTC )
      {
         pstNonRealTime->bPictureEncodedOnThisSTC = !pMFDPicture->bIgnorePicture;
      }
   }



#if 0
   /* Calculate Ignore Picture */
   {
      /* By default, if we're stalling the STC, we also want to ignore this picture */
      pMFDPicture->bIgnorePicture = pMFDPicture->bStallStc;

      /* This encoder stalled (underflow) */
      if ( ( pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated >= (int32_t) pstPicture->stPicParms.stTSM.stStatic.stPTSDelta.uiWhole ) /* next picture not available */
                && ( true == pLocalState->bPictureQueueIsEmpty )  /* decoder underflow */
                && ( false == pstNonRealTime->bIgnoringUnderflow ) /* not ignoring the underflow */
         )
      {
         BDBG_MSG(("%x:[%02x.%03x] Underflow Ignore Pic",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pstPicture->stPicParms.uiPPBIndex & 0xFFF
                              ));

         pMFDPicture->bIgnorePicture = true;
      }

      /* The STC is stalled and this field has already been encoded, then don't encode the same field again */
      if ( ( ( true == pstNonRealTime->bLastStcValid ) /* dSTC == 0 */
             && ( pstNonRealTime->uiLastStc == pLocalState->uiStcSnapshot ) )
           && ( true == pstPicture->stPicParms.stDisplay.stDynamic.bPPBEncoded ) )
      {
         BDBG_MSG(("%x:[%02x.%03x] Repeated Ignore Pic",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pstPicture->stPicParms.uiPPBIndex & 0xFFF
                              ));

         pMFDPicture->bIgnorePicture = true;
      }
   }

   if ( false == pMFDPicture->bIgnorePicture )
   {
      pstPicture->stPicParms.stDisplay.stDynamic.bPPBEncoded = true;
   }
#endif

#if 0 /* SW7445-224: Remove the FIC bStallStc logic from XDM. */

   /* Keep track of interrupt polarity to aid FIC STC Stall logic
    * SW7425-2424: "bPolarityFieldValid" forces "ePolarityWhenDeltaStcWasNonZero"
    * to be set the first time through this code.
    */
   if ( (( true == pstNonRealTime->bLastStcValid )
               && ( pstNonRealTime->uiLastStc != pLocalState->uiStcSnapshot ))
        || ( false == pstNonRealTime->bPolarityFieldValid )
      )
   {
      /* dStc != 0 */
      /* STC advanced since last display interrupt,
       * so just record the current polarity
       */
      pstNonRealTime->ePolarityWhenDeltaStcWasNonZero = pLocalState->eVsyncPolarity;
      pstNonRealTime->bPolarityFieldValid = true;
   }
#endif

   pstNonRealTime->uiLastStc = pLocalState->uiStcSnapshot;
   pstNonRealTime->bLastStcValid = true;
   pstNonRealTime->bLastStallStc = pMFDPicture->bStallStc;
   pstNonRealTime->bLastStallStcValid = true;

   /*
    * SW7425-3358: support for FNRT
    */
   if ( true == hXdmPP->stDMConfig.stFNRTSettings.bEnabled )
   {
      /* If this is the first picture after start decode
       * has been called, save the chunkId.
       */
      if ( true == hXdmPP->stDMState.stDecode.bThisIsTheFirstPPB )
      {
         pstNonRealTime->uiChunkId = pstPicture->pstUnifiedPicture->uiChunkId;
      }

      pMFDPicture->ulChunkId = pstNonRealTime->uiChunkId;

      /* Set pMFDPicture->bPreChargePicture if
       * - the precharge count for this picture is less than the specified precharge count
       * - AND this is NOT chunk '0'
       */
      if ( pstNonRealTime->uiFNRTPreChargeCnt < hXdmPP->stDMConfig.stFNRTSettings.uiPreChargeCount
            && 0 != pstNonRealTime->uiChunkId )
      {
         pMFDPicture->bPreChargePicture = true;
      }

      pstNonRealTime->uiFNRTPreChargeCnt += ( false == pMFDPicture->bIgnorePicture ) ? 1 : 0;

      /* If the chunkId in the Unified Picture is different from the saved value,
       * then XDM is beginning to process the start of the next chunk.
       */
      if ( pstNonRealTime->uiChunkId != pstPicture->pstUnifiedPicture->uiChunkId )
      {
         pstNonRealTime->bNextChunkReceived = true;
      }

      /* Set bIgnorePicture the vsync after the end of the chunk has been reached.
       * This line of code needs to precede the logic which sets bChunkDone.
       */
      pMFDPicture->bIgnorePicture |= ( true == pstNonRealTime->bChunkDone );

      /* If the next chunck has been received, start counting the "post" chunk pictures.
       * Set "bChunkDone" once the number delivered equals the precharge count.
       * Don't increment the "post charge count" if the picture is being ignored.
       */
      if ( true == pstNonRealTime->bNextChunkReceived )
      {
         pstNonRealTime->uiFNRTPostChargeCnt += ( false == pMFDPicture->bIgnorePicture ) ? 1 : 0;

         if ( pstNonRealTime->uiFNRTPostChargeCnt >= hXdmPP->stDMConfig.stFNRTSettings.uiPreChargeCount )
         {
            pstNonRealTime->bChunkDone = true;
         }
      }

      pMFDPicture->bEndOfChunk = pstNonRealTime->bChunkDone;


   }     /* end of if ( true == hXdmPP->stDMConfig.stFNRTSettings.bEnabled )  */
   else
   {
      /* If FNRT is not enabled, simply copy the chunk ID from the unified picture.
       */
      pMFDPicture->ulChunkId = pstPicture->pstUnifiedPicture->uiChunkId;
   }

}        /* end of BXDM_PPOUT_S_SetStallStcAndIgnorePicture_isr()                 */

void BXDM_PPOUT_P_CalculateVdcData_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BAVC_MFD_Picture** pMFDPicture
   )
{
   const BXDM_Picture* pUnifiedPicture = NULL;
   BAVC_MFD_Picture * pDefaultParams = &( hXdmPP->stDMState.stChannel.stDefaultPicParams );
   BAVC_MFD_Picture *pCurrentMFDPicture;

   BXDM_PictureProvider_P_Picture_Context * pstSelectedPicture = &( hXdmPP->stDMState.stChannel.stSelectedPicture );

   BDBG_ENTER(BXDM_PPOUT_P_CalculateVdcData_isr);

   BDBG_ASSERT(pMFDPicture);
   *pMFDPicture = &(hXdmPP->astMFDPicture[0]);
   pCurrentMFDPicture = *pMFDPicture;

   /* If a PPB is not available, simply return without changing the "pCurrentMFDPicture" structure.
    * This leaves all the information intact from the last picture sent to XVD.
    * However, the "polarity" does need to be set, don't move this statement.
    * Also be certain that the picture is muted.
    */
   if ( NULL == pstSelectedPicture->pstUnifiedPicture )
   {
      /* eInterruptPolarity */
      pCurrentMFDPicture->eInterruptPolarity = pLocalState->eVsyncPolarity;

      if ( BXDM_PictureProvider_P_DecodeState_eStopped == hXdmPP->stDMState.stChannel.eDecodeState )
      {
         /* SW7425-3358: between stop/start decode, bIgnorePicture
          * and bStallStc should be set to the defaults (true).
          */
         pCurrentMFDPicture->bIgnorePicture = pDefaultParams->bIgnorePicture;
         pCurrentMFDPicture->bStallStc = pDefaultParams->bStallStc;
      }
      else if ( ( true == hXdmPP->stDMConfig.bPlayback )
               && ( true == hXdmPP->stDMConfig.bSTCValid ) )
      {
         /* If in playback mode and the STC is valid, but we don't have a picture to display, yet,
          * then we want the encoder to encode the muted frames and increment the STC to ensure all
          * encoders start at the same time.  This would happen when the video decoder is the slave
          * and TSM has not passed, yet.
          * See: http://twiki-01.broadcom.com/bin/view/Arch/TranscodeMiscellaneous#Revisiting_Time_Zero_Approach_of */

         /* SW7425-2915: At startup when in NRT mode, the action of the FIC needs to be taken
          * into consideration.  Specifically the case where displaying the first picture is delayed
          * for one vsync when the FIC logics pushes the comparison into the second slot of the
          * pass window. In this situation, XDM should deliver a muted picture on this vsync.
          * The question is how to set the "bIgnorePicture" flag. If " bIgnorePicture" was set on
          * the previous vsync, it should be set on this one as well. If it was not set,
          * it should be set this time either.
          */
         if ( true == pLocalState->bWaitDueTo2ndSlotTransition
             && true == hXdmPP->stDMState.stDecode.stNonRealTime.blastIgnorePicture )
         {
            pCurrentMFDPicture->bIgnorePicture = true;

            BDBG_MSG(("%x:[%02x.000] Setting Ignore Picture flag as a result of bWaitDueTo2ndSlotTransition",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP )
                              ));
         }
         else
         {
            /* SW7425-4205: Once the STC is validated, if the STC is stalled, then XDM should
             * set bIgnorePicture regardless of whether it is a muted or decoded picture.
             * This handles the case of a muted picture at startup.
             */
            pCurrentMFDPicture->bIgnorePicture = ( true == hXdmPP->stDMState.stDecode.bStcStalled ) ? true : false;
         }
         pCurrentMFDPicture->bStallStc = false;
      }
      else
      {
         pCurrentMFDPicture->bIgnorePicture = pDefaultParams->bIgnorePicture;
         pCurrentMFDPicture->bStallStc = pDefaultParams->bStallStc;
      }

      pCurrentMFDPicture->bMute=true;
      pCurrentMFDPicture->bChannelChange = true; /* for SW7425-2253: */

      /* SW7425-3358: set EOC and PCP to the default (false) if there isn't a picture available. */
      pCurrentMFDPicture->bEndOfChunk = pDefaultParams->bEndOfChunk;
      pCurrentMFDPicture->bPreChargePicture = pDefaultParams->bPreChargePicture;

      /* SW7425-4201: bLast should not span a stop/start decode, force bLast to false. */
      pCurrentMFDPicture->bLast = false;

   }
   else
   {
      pUnifiedPicture = pstSelectedPicture->pstUnifiedPicture;

      /* Copy static MFD Picture info from the selected picture context */
      *pCurrentMFDPicture = pstSelectedPicture->stMFDPicture;

      /* SW7405-5549: there is a remote chance that eTrickPlayRepeatMode could change after
       * BXDM_PPOUT_P_CalculateStaticVdcData_isr has been called. This could come about if the
       * monitor refresh rate is changed while a stream is playing.  Hence the assignment needs
       * to be done here.
       */
      pCurrentMFDPicture->bFrameProgressive = ( BXDM_PictureProvider_P_RepeatMode_eFrame == pstSelectedPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode );

      /* eInterruptPolarity */
      pCurrentMFDPicture->eInterruptPolarity = pLocalState->eVsyncPolarity;

      /* pPictureInfo->eSourcePolarity */
      BXDM_PPOUT_S_SetSourcePolarity_isr(
         hXdmPP,
         pstSelectedPicture,
         pLocalState,
         pCurrentMFDPicture
         );

      /* Pan Scan (i32_HorizontalPanScan, i32_VerticalPanScan,
       * ulDisplayHorizontalSize, ulDisplayVerticalSize) */
      BXDM_PPOUT_S_SetPanScan_isr(
         hXdmPP,
         pstSelectedPicture,
         hXdmPP->stDMConfig.bCRCMode,
         pCurrentMFDPicture
         );

      /* eChrominanceInterpolationMode */
      if ( ( BAVC_Polarity_eFrame == pCurrentMFDPicture->eSourcePolarity )
           || ( BXDM_PictureProvider_P_RepeatMode_eFrame == pstSelectedPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode )
         )
      {
         pCurrentMFDPicture->eChrominanceInterpolationMode = BAVC_InterpolationMode_eFrame;
      }
      else
      {
         pCurrentMFDPicture->eChrominanceInterpolationMode = pDefaultParams->eChrominanceInterpolationMode;
      }

      /* bPictureRepeatFlag - We set this to true if we're repeating the
       * same field from the same PPB */
      if ( ( true == pstSelectedPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
           && ( pstSelectedPicture->pstUnifiedPicture == hXdmPP->stDMState.stChannel.stPreviousPicture.pstUnifiedPicture )
           && ( pCurrentMFDPicture->eSourcePolarity == hXdmPP->stDMState.stChannel.ePrevSourcePolarity )
           && ( false == hXdmPP->stDMState.stChannel.bPrevMute )
         )
      {
         pCurrentMFDPicture->bPictureRepeatFlag = true;
      }
      else
      {
         pCurrentMFDPicture->bPictureRepeatFlag = pDefaultParams->bPictureRepeatFlag;
      }

      /* SW7425-104: Set the MFD bRepeatField flag based on how the picture was previously
       * delivered to VDC.  If an element is being repeated, set the flag.  Track which
       * elements are delivered for subsequent vsyncs.
       */
      if ( false == hXdmPP->stDMState.stChannel.bPrevMute )
      {
         switch ( pCurrentMFDPicture->eSourcePolarity )
         {
            case BAVC_Polarity_eFrame:
               pCurrentMFDPicture->bRepeatField = ( pstSelectedPicture->stPicParms.stDisplay.stDynamic.bFrameDelivered ) ? true : false ;
               pstSelectedPicture->stPicParms.stDisplay.stDynamic.bFrameDelivered = true;
               break;

            case BAVC_Polarity_eBotField:
               pCurrentMFDPicture->bRepeatField = ( pstSelectedPicture->stPicParms.stDisplay.stDynamic.bBottomFieldDelivered ) ? true : false ;
               pstSelectedPicture->stPicParms.stDisplay.stDynamic.bBottomFieldDelivered = true;
               break;

            case BAVC_Polarity_eTopField:
            default:
               pCurrentMFDPicture->bRepeatField = ( pstSelectedPicture->stPicParms.stDisplay.stDynamic.bTopFieldDelivered ) ? true : false ;
               pstSelectedPicture->stPicParms.stDisplay.stDynamic.bTopFieldDelivered = true;
               break;

         }
      }


      /* PR49634: bIgnoreCadenceMatch - We set this to true if we're
       * not expecting polarity match so that we can turn off VDC's
       * cadence matching */
      BXDM_PPFIC_P_CalculateIgnoreCadenceMatch_isr(
         hXdmPP,
         pLocalState,
         pstSelectedPicture
         );
      pCurrentMFDPicture->bIgnoreCadenceMatch = pstSelectedPicture->stPicParms.stDisplay.stDynamic.bIgnoreCadenceMatch;

      BXDM_PPOUT_S_SetStallStcAndIgnorePicture_isr(
         hXdmPP,
         pLocalState,
         pstSelectedPicture,
         pCurrentMFDPicture
         );

      /* Remember the mute state from this vsync */
      hXdmPP->stDMState.stChannel.bPrevMute = pCurrentMFDPicture->bMute;

      /* Keep track of the source polarity to help us decide what to set
       * the polarity of progressive content on a progressive display
       * when MPIM is set to eInterlaced.  We also use this value when
       * handling the eSingleField display field type */
      hXdmPP->stDMState.stChannel.ePrevSourcePolarity = pCurrentMFDPicture->eSourcePolarity;

      BXDM_PPOUT_S_SetMpegType_isr(
               hXdmPP,
               pstSelectedPicture,
               pCurrentMFDPicture
               );

      BXDM_PPOUT_S_SetPictureOrderCount_isr(
                hXdmPP,
                pstSelectedPicture,
                pCurrentMFDPicture);

      /* Set the AFD (Active Format Description) based on the contents of the PPB.
       */
      pCurrentMFDPicture->bValidAfd = pUnifiedPicture->stActiveFormatDescription.bValid;
      if ( true == pCurrentMFDPicture->bValidAfd )
      {
         pCurrentMFDPicture->ulAfd = pUnifiedPicture->stActiveFormatDescription.uiValue;
      }
      else
      {
         pCurrentMFDPicture->ulAfd = 0;
      }

      /* If the picture is NOT a repeat, bump the count of pictures delivered to VDC. */
      if ( false == pstSelectedPicture->stPicParms.stDisplay.stDynamic.bElementRepeated )
      {
         hXdmPP->stDMStatus.stCounters.uiDisplayedCount++;
      }

      /* CDSTRMANA-1083: BXDM_PPOUT_P_CalculateStaticVdcData_isr is called multiple
       * times on each sync. First for the current picture and then for the picture at
       * the head of the display queue.  This adjustment needs to be made here in
       * BXDM_PPOUT_P_CalculateVdcData_isr so that it isn't applied twice.*/

      BXDM_PPOUT_S_HEVCInterlacedFixup_isr( hXdmPP, pstSelectedPicture, pCurrentMFDPicture );

      /* Only update the parameters AFTER the first PPB has passed TSM */
      /* Update the Picture Parameter structure with the data for this PPB. */
      if (true == hXdmPP->stDMState.stDecode.bFirstPPBHasBeenDisplayed)
      {
         BXDM_PPOUT_S_UpdatePictureParameters_isr(
            hXdmPP,
            pCurrentMFDPicture,
            pstSelectedPicture,
            false
            );
      }

      /* If CRC mode is enabled AND the element is not being repeated
       * then instruct VDC to generate the CRC.
       */
      if ( ( false == pstSelectedPicture->stPicParms.stDisplay.stDynamic.bElementRepeated )
           && ( true == hXdmPP->stDMConfig.bCRCMode )
         )
      {
         pCurrentMFDPicture->bCaptureCrc = true;
      }
      else
      {
         pCurrentMFDPicture->bCaptureCrc = false;
      }

      /* SW7425-44: parameters for encoder ... */
      /* set original PTS for encoder use. Note: original PTS is either the original coded
         PTS of the selected picture or the interpolated PTS for this picture BEFORE any jitter
         correction or adjustments are made. */

      {
         /* SWSTB-5027: use "eTrickPlayRepeatMode" to infer if the content is progressive.
          * If the content is progressive, send the coded PTS for the frame.  When the frame is repeated,
          * the same PTS will be sent every time.  If the content is interlaced, send the per field PTS.
          * The PTS for the first field will be the coded PTS.  The PTS for subsequent fields will be an
          * interpolated PTS. When a field is repeated, the PTS associated with that field will be sent again. */

         uint32_t uiIndex;

         if ( BXDM_PictureProvider_P_RepeatMode_eFrame == pstSelectedPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode )
         {
            uiIndex = 0;
         }
         else
         {
            uiIndex =  pstSelectedPicture->stPicParms.stTSM.stDynamic.uiTSMSelectedElement;
         }

         BXDM_PPQM_P_GetPtsUnfiltered_isr(
                        pstSelectedPicture,
                        BXDM_PictureProvider_P_PTSIndex_eActual,
                        uiIndex,
                        &pCurrentMFDPicture->ulOrigPTS );
      }

      /* SW7425-764: copy the monitor refresh rate into the MFD structure. */
      pCurrentMFDPicture->eInterruptRefreshRate = pstSelectedPicture->stPicParms.stDisplay.stStatic.eBFMTRefreshRate;

      /* SW7425-1781: copy the picture's serial number into uiDecoderPictureId */
      pCurrentMFDPicture->ulDecodePictureId = pUnifiedPicture->uiSerialNumber;

      /* SW7425-2253: set "bChannelChange" if new pictures are NOT being decoded and delivered to VDC, i.e.
       * - BXVD_StopDecode has been called
       * - OR BXVD_StartDecode has been, but the first picture has not been sent to VDC.
       */
      pCurrentMFDPicture->bChannelChange = ( BXDM_PictureProvider_P_DecodeState_eStopped == hXdmPP->stDMState.stChannel.eDecodeState );
      pCurrentMFDPicture->bChannelChange |= ( false == hXdmPP->stDMState.stDecode.bFirstPPBHasBeenDisplayed );

#if 0 /* SWSTB-3450: add display/HDR parameter override. */
      /* Copy HDR info */
      pCurrentMFDPicture->ulAvgContentLight = pUnifiedPicture->stHDR.ulAvgContentLight;
      pCurrentMFDPicture->ulMaxContentLight = pUnifiedPicture->stHDR.ulMaxContentLight;

      pCurrentMFDPicture->stDisplayPrimaries[0] = pUnifiedPicture->stHDR.stDisplayPrimaries[0];
      pCurrentMFDPicture->stDisplayPrimaries[1] = pUnifiedPicture->stHDR.stDisplayPrimaries[1];
      pCurrentMFDPicture->stDisplayPrimaries[2] = pUnifiedPicture->stHDR.stDisplayPrimaries[2];

      pCurrentMFDPicture->stWhitePoint = pUnifiedPicture->stHDR.stWhitePoint;

      pCurrentMFDPicture->ulMaxDispMasteringLuma = pUnifiedPicture->stHDR.ulMaxDispMasteringLuma;
      pCurrentMFDPicture->ulMinDispMasteringLuma = pUnifiedPicture->stHDR.ulMinDispMasteringLuma;
#endif
   }

   /* SW7425-2915: added for issues at startup in NRT mode. */
   hXdmPP->stDMState.stDecode.stNonRealTime.blastIgnorePicture = pCurrentMFDPicture->bIgnorePicture;


   /* SW7425-1001: set "MFD.bLast" if
    * - "bLastPictureSeen" is true
    * - AND and the last field of the last picture has been delivered to VDC
    *   (as indicated by value of "iStcPtsDifferenceEvaluated")
    *
    * The use of "bSetLastPictureFlag" may be overkill, but it ensures that
    * once "MFD.bLast" is set, it stays set.
    */
   if ( ( pstSelectedPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated >= (int32_t) pstSelectedPicture->stPicParms.stTSM.stStatic.stPTSDelta.uiWhole )
         && ( true == pstSelectedPicture->stPicParms.stDisplay.stDynamic.bLastPictureSeen )
      )
   {
      pstSelectedPicture->stPicParms.stDisplay.stDynamic.bSetLastPictureFlag = true;
   }

   pCurrentMFDPicture->bLast = pstSelectedPicture->stPicParms.stDisplay.stDynamic.bSetLastPictureFlag;

   BXDM_PPDBG_P_OutputLog_isr(
      hXdmPP,
      pLocalState,
      pCurrentMFDPicture
      );


   BXDM_PPOUT_S_3DFixup_isr(
      hXdmPP,
      pLocalState,
      pstSelectedPicture,
      pMFDPicture
      );

#if 0
   BKNI_Printf("m:%d,ip:%d,sp:%d,ar:(%d,%d:%d),fr:%d,mt:%d,yt:%d,%4dx%4d,nm:%3dx%3d,sw:%d,poc:(%2d,%2d),cl:%2dx%2d\n",
               pCurrentMFDPicture->bMute,
               pCurrentMFDPicture->eInterruptPolarity,
               pCurrentMFDPicture->eSourcePolarity,
               pCurrentMFDPicture->eAspectRatio,
               pCurrentMFDPicture->uiSampleAspectRatioX,
               pCurrentMFDPicture->uiSampleAspectRatioY,
               pCurrentMFDPicture->eFrameRateCode,
               pCurrentMFDPicture->eMpegType,
               pCurrentMFDPicture->eYCbCrType,
               pCurrentMFDPicture->ulSourceHorizontalSize,
               pCurrentMFDPicture->ulSourceVerticalSize,
               pCurrentMFDPicture->ulLuminanceNMBY,
               pCurrentMFDPicture->ulChrominanceNMBY,
               pCurrentMFDPicture->eStripeWidth,
               pCurrentMFDPicture->ulIdrPicID,
               pCurrentMFDPicture->int32_PicOrderCnt,
               pCurrentMFDPicture->ulSourceClipTop,
               pCurrentMFDPicture->ulSourceClipLeft
               );
#endif

   BDBG_LEAVE(BXDM_PPOUT_P_CalculateVdcData_isr);
   return;
} /* end of BXDM_PPOUT_P_CalculateVdcData_isr() */

/*
 * This routine will be called at "channel open" time to
 * handle initialization that only needs to occur once.
 * This is intend to reduce the initialization overhead
 * at "start decode" time.
 */
void BXDM_PPOUT_P_OpenChannel(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BAVC_MFD_Picture * pDefaultParams = &( hXdmPP->stDMState.stChannel.stDefaultPicParams );

   /*
    * Set the default picture parameters.
    */
   BKNI_Memset(
      &(hXdmPP->stDMState.stChannel.stDefaultPicParams),
      0,
      sizeof( hXdmPP->stDMState.stChannel.stDefaultPicParams )
      );

   pDefaultParams->eInterruptPolarity = BAVC_Polarity_eTopField;
   pDefaultParams->bIgnorePicture = true;
   pDefaultParams->bStallStc = true;
   pDefaultParams->bMute = true;
   pDefaultParams->eSourcePolarity = BAVC_Polarity_eTopField;
   pDefaultParams->eMatrixCoefficients = BAVC_MatrixCoefficients_eUnknown;
   pDefaultParams->eAspectRatio = BFMT_AspectRatio_eUnknown;
   pDefaultParams->eFrameRateCode = BAVC_FrameRateCode_eUnknown;
   pDefaultParams->bStreamProgressive = false;
   pDefaultParams->bFrameProgressive = false;
   pDefaultParams->eMpegType = BAVC_MpegType_eMpeg2;
   pDefaultParams->eYCbCrType = BAVC_YCbCrType_e4_2_0;
   pDefaultParams->eChrominanceInterpolationMode = BAVC_InterpolationMode_eField;
   pDefaultParams->eColorPrimaries = BAVC_ColorPrimaries_eUnknown;
   pDefaultParams->eTransferCharacteristics = BAVC_TransferCharacteristics_eUnknown;
   pDefaultParams->ePreferredTransferCharacteristics = BAVC_TransferCharacteristics_eUnknown; /* SWSTB-1629 */

   pDefaultParams->hLuminanceFrameBufferBlock = NULL;
   pDefaultParams->ulLuminanceFrameBufferBlockOffset = 0;
   pDefaultParams->hChrominanceFrameBufferBlock = NULL;
   pDefaultParams->ulChrominanceFrameBufferBlockOffset = 0;

   /* SW7445-586: H265/HEVC interlaced, default to a single frame buffer. */
   pDefaultParams->eBufferFormat = BAVC_DecodedPictureBuffer_eFrame;
   pDefaultParams->hLuminanceBotFieldBufferBlock = NULL;
   pDefaultParams->ulLuminanceBotFieldBufferBlockOffset = 0;
   pDefaultParams->hChrominanceBotFieldBufferBlock = NULL;
   pDefaultParams->ulChrominanceBotFieldBufferBlockOffset = 0;

   pDefaultParams->bPictureRepeatFlag = false;
   pDefaultParams->bIgnoreCadenceMatch = false;
   pDefaultParams->pNext = NULL;
   pDefaultParams->bCaptureCrc = false;
   pDefaultParams->eStripeWidth = BAVC_StripeWidth_e64Byte;
   pDefaultParams->ulLumaRangeRemapping = 0x08;
   pDefaultParams->ulChromaRangeRemapping = 0x08;

   pDefaultParams->ePictureType = BAVC_PictureCoding_eUnknown;
   pDefaultParams->ePxlFmt = BPXL_INVALID;
   pDefaultParams->eBarDataType = BAVC_BarDataType_eInvalid;

   /* SW7445-744: add support for 10 bit picture buffers. */
   pDefaultParams->eBitDepth = BAVC_VideoBitDepth_e8Bit;
   pDefaultParams->eChromaBitDepth = BAVC_VideoBitDepth_e8Bit;

   /* SWSTB-3450: HDR parameters */
   pDefaultParams->ulAvgContentLight = 0;
   pDefaultParams->ulMaxContentLight = 0;
   pDefaultParams->stDisplayPrimaries[0].ulX = 0xFFFFFFFF;
   pDefaultParams->stDisplayPrimaries[0].ulY = 0xFFFFFFFF;
   pDefaultParams->stDisplayPrimaries[1].ulX = 0xFFFFFFFF;
   pDefaultParams->stDisplayPrimaries[1].ulY = 0xFFFFFFFF;
   pDefaultParams->stDisplayPrimaries[2].ulX = 0xFFFFFFFF;
   pDefaultParams->stDisplayPrimaries[2].ulY = 0xFFFFFFFF;
   pDefaultParams->stWhitePoint.ulX          = 0xFFFFFFFF;
   pDefaultParams->stWhitePoint.ulY          = 0xFFFFFFFF;
   pDefaultParams->ulMaxDispMasteringLuma    = 0xFFFFFFFF;
   pDefaultParams->ulMinDispMasteringLuma    = 0xFFFFFFFF;

   return;

}  /* end of BXDM_PPOUT_P_OpenChannel() */
