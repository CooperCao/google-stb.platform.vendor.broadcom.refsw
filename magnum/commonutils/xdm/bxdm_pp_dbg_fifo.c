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
#include "bdbg.h"

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#if 0
#include "bxdm_pp_dbg_fifo.h"
#include "bxdm_pp_dbg_fifo_priv.h"
#endif
#include "bxdm_pp_qm.h"
#include "bxdm_pp_dbg_common.h"


BDBG_MODULE(BXDM_PPDBG_FIFO);
BDBG_FILE_MODULE(BXDM_MFD1);
BDBG_FILE_MODULE(BXDM_MFD2);
BDBG_FILE_MODULE(BXDM_MFD3);
BDBG_FILE_MODULE(BXDM_PPQM);
BDBG_FILE_MODULE(BXDM_CFG);
BDBG_FILE_MODULE(BXDM_PPDBG2);
BDBG_FILE_MODULE(BXDM_PPDBC);
BDBG_FILE_MODULE(BXDM_PPDBG);
BDBG_FILE_MODULE(BXDM_PPFRD);
BDBG_FILE_MODULE(BXDM_PPFIC);
BDBG_FILE_MODULE(BXDM_PPCB);
BDBG_FILE_MODULE(BXDM_PPCLIP);
BDBG_FILE_MODULE(BXDM_PPOUT);
BDBG_FILE_MODULE(BXDM_PPTSM);
BDBG_FILE_MODULE(BXDM_PPVTSM);
BDBG_FILE_MODULE(BXDM_PPV2);

#if BDBG_DEBUG_BUILD && BXDM_DEBUG_FIFO

#define BXDM_PPDF_FORMAT_INSTANCE_ID( _uiInstanceID_ )   \
  _uiInstanceID_ & 0xFF                                     \

/*
 * Functions
 */
static void BXDM_PPDF_S_PrintString_isrsafe(
   BXDM_P_DebugFifo_Entry * pstEntry
   )
{
   BDBG_ENTER( BXDM_PPDF_S_PrintString_isrsafe );

   BDBG_ASSERT( pstEntry );

   switch ( pstEntry->data.stString.eType )
   {
      case BXDM_Debug_MsgType_eQM:   BDBG_MODULE_MSG( BXDM_PPQM, ("%s", pstEntry->data.stString.szString ));    break;
      case BXDM_Debug_MsgType_eDBG:  BDBG_MODULE_MSG( BXDM_PPDBG, ("%s", pstEntry->data.stString.szString ));   break;
      case BXDM_Debug_MsgType_eDBG2: BDBG_MODULE_MSG( BXDM_PPDBG2, ("%s", pstEntry->data.stString.szString ));  break;
      case BXDM_Debug_MsgType_eDBGC: BDBG_MODULE_MSG( BXDM_PPDBC, ("%s", pstEntry->data.stString.szString ));   break;
      case BXDM_Debug_MsgType_eMFD1: BDBG_MODULE_MSG( BXDM_MFD1, ("%s", pstEntry->data.stString.szString ));    break;
      case BXDM_Debug_MsgType_eMFD2: BDBG_MODULE_MSG( BXDM_MFD2, ("%s", pstEntry->data.stString.szString ));    break;
      case BXDM_Debug_MsgType_eMFD3: BDBG_MODULE_MSG( BXDM_MFD3, ("%s", pstEntry->data.stString.szString ));    break;
      case BXDM_Debug_MsgType_eCFG:  BDBG_MODULE_MSG( BXDM_CFG, ("%s", pstEntry->data.stString.szString ));     break;
      case BXDM_Debug_MsgType_eFRD:  BDBG_MODULE_MSG( BXDM_PPFRD, ("%s", pstEntry->data.stString.szString ));   break;
      case BXDM_Debug_MsgType_eFIC:  BDBG_MODULE_MSG( BXDM_PPFIC, ("%s", pstEntry->data.stString.szString ));   break;
      case BXDM_Debug_MsgType_eCB:   BDBG_MODULE_MSG( BXDM_PPCB, ("%s", pstEntry->data.stString.szString ));    break;
      case BXDM_Debug_MsgType_eCLIP: BDBG_MODULE_MSG( BXDM_PPCLIP, ("%s", pstEntry->data.stString.szString ));  break;
      case BXDM_Debug_MsgType_eOUT:  BDBG_MODULE_MSG( BXDM_PPOUT, ("%s", pstEntry->data.stString.szString ));   break;
      case BXDM_Debug_MsgType_eTSM:  BDBG_MODULE_MSG( BXDM_PPTSM, ("%s", pstEntry->data.stString.szString ));   break;
      case BXDM_Debug_MsgType_eVTSM: BDBG_MODULE_MSG( BXDM_PPVTSM, ("%s", pstEntry->data.stString.szString ));  break;
      case BXDM_Debug_MsgType_ePPV2: BDBG_MODULE_MSG( BXDM_PPV2, ("%s", pstEntry->data.stString.szString ));    break;

      case BXDM_Debug_MsgType_eUnKnown:
      default:
         BDBG_MODULE_MSG( BXDM_PPDBG, ("%s: unknown message type", __FUNCTION__ ));
         BDBG_MODULE_MSG( BXDM_PPDBG, ("%s", pstEntry->data.stString.szString ));
         break;

   }

   BDBG_LEAVE( BXDM_PPDF_S_PrintString_isrsafe );

   return;
}

/*
 * SW7405-4736: conditionally print the MFD structure.
 */
static void BXDM_PPDF_S_PrintMFD_isrsafe(
   BXDM_P_DebugFifo_Entry * pstEntry
   )
{
   BAVC_MFD_Picture * pMFD;
   int32_t  i=0;
   char cBarDataType;
   BFMT_AspectRatio eAspectRatio=BFMT_AspectRatio_eUnknown;

   BDBG_ENTER( BXDM_PPDF_S_PrintMFD_isrsafe );

   BDBG_ASSERT( pstEntry );

   pMFD = &pstEntry->data.stMFD;

   /* Range check */

   if ( pMFD->eAspectRatio <= BFMT_AspectRatio_eSAR )
   {
      eAspectRatio = pMFD->eAspectRatio;
   }

   BDBG_MODULE_MSG( BXDM_MFD1, ("%c%x:[%02x.%03x] id:%03x pts:%08x %s->%s %dx%d->%dx%d %s:%dx%d AQP:%02d fr:%s mr:%s %s%s%s%s%s%s%s%s%s%s%s",
                              ( pMFD->bMute ) ? 'M' : ' ',
                              pstEntry->stMetadata.uiVsyncCount,
                              BXDM_PPDF_FORMAT_INSTANCE_ID( pstEntry->stMetadata.uiInstanceID ),
                              pstEntry->stMetadata.uiPPBIndex & 0xFFF,
                              pMFD->ulDecodePictureId,
                              pMFD->ulOrigPTS,
                              BXDM_P_PolarityToStrLUT[ pMFD->eSourcePolarity ],
                              BXDM_P_PolarityToStrLUT[ pMFD->eInterruptPolarity ],
                              pMFD->ulSourceHorizontalSize,
                              pMFD->ulSourceVerticalSize,
                              pMFD->ulDisplayHorizontalSize,
                              pMFD->ulDisplayVerticalSize,
                              BXDM_P_AspectRatioToStrLUT[ eAspectRatio ],
                              pMFD->uiSampleAspectRatioX,
                              pMFD->uiSampleAspectRatioY,
                              pMFD->ulAdjQp,
                              BXDM_P_BAVCFrameRateToStrLUT[ pMFD->eFrameRateCode ],
                              BXDM_P_BFMTRefreshRateToStrLUT[ pMFD->eInterruptRefreshRate],
                              BXDM_P_OrientationToStrLUT[ pMFD->eOrientation ],
                              ( pMFD->bPictureRepeatFlag ) ? " rp" : " ",
                              ( pMFD->bRepeatField ) ? " rf" : " ",
                              ( pMFD->bIgnoreCadenceMatch ) ? " ic" : " ",
                              ( pMFD->bIgnorePicture ) ? " ip" : " ",
                              ( pMFD->bStallStc ) ? " ss" : " ",
                              ( pMFD->bLast ) ? " lst" : " ",
                              ( pMFD->bChannelChange) ? " chg" : " ",
                              ( pMFD->bPreChargePicture) ? " pcp" : " ",
                              ( pMFD->bEndOfChunk) ? " eoc" : " ",
                              ( 0 != i ) ? " en" : " "
                              ));

   BDBG_MODULE_MSG( BXDM_MFD3, ("%c%x:[%02x.%03x] %s md:%d top::lb:0x%lu lo:%08x cb:0x%lu co:%08x bot::lb:0x%lu lo:%08x cb:0x%lu co:%08x",
                              ( pMFD->bMute ) ? 'M' : ' ',
                              pstEntry->stMetadata.uiVsyncCount,
                              BXDM_PPDF_FORMAT_INSTANCE_ID(  pstEntry->stMetadata.uiInstanceID ),
                              pstEntry->stMetadata.uiPPBIndex & 0xFFF,
                              ( pMFD->eBufferFormat ) ? "fp" : "Fr" ,
                              pMFD->stHdrMetadata.eType,
                              (unsigned long)pMFD->hLuminanceFrameBufferBlock,
                              pMFD->ulLuminanceFrameBufferBlockOffset,
                              (unsigned long)pMFD->hChrominanceFrameBufferBlock,
                              pMFD->ulChrominanceFrameBufferBlockOffset,
                              (unsigned long)pMFD->hLuminanceBotFieldBufferBlock,
                              pMFD->ulLuminanceBotFieldBufferBlockOffset,
                              (unsigned long)pMFD->hChrominanceBotFieldBufferBlock,
                              pMFD->ulChrominanceBotFieldBufferBlockOffset
                       ));


   /* SW7405-4736: Ideally the following would be printed out once per second or when any of the values
    * change, this would save extraneous prints to the console.  The values could be tracked by keeping
    * a local version of the MFD structure, but that would involve copying the data an extra time.
    * The memory pointed to by pMFD is allocated by XDM, i.e. "hXdmPP->astMFDPicture"; essentially XDM
    * already has a local copy.  However the "astMFDPicture" elements are set throughout
    * "BXDM_PPOUT_P_CalculateVdcData_isr".  The code to check for a change in value would have to be added
    * in multiple locations, a task for another day.
    */

   switch( pMFD->eBarDataType )
   {
      case BAVC_BarDataType_eTopBottom:   cBarDataType = 'T';     break;
      case BAVC_BarDataType_eLeftRight:   cBarDataType = 'L';     break;
      default:                            cBarDataType = 'i';     break;
   }

   BDBG_MODULE_MSG( BXDM_MFD2, ("%c%x:[%02x.%03x] clp:%dx%d afd:%d(%d) bar:%dx%d(%c) pan:%dx%d loc:%d ci:%s cp:%d tc:%d mc:%d cr:%d %s dp:%s chk:%08x %s%s",
                                 ( pMFD->bMute ) ? 'M' : ' ',
                                 pstEntry->stMetadata.uiVsyncCount,
                                 BXDM_PPDF_FORMAT_INSTANCE_ID( pstEntry->stMetadata.uiInstanceID ),
                                 pstEntry->stMetadata.uiPPBIndex & 0xFFF,
                                 pMFD->ulSourceClipLeft,
                                 pMFD->ulSourceClipTop,
                                 pMFD->ulAfd,
                                 pMFD->bValidAfd,
                                 pMFD->ulTopLeftBarValue,
                                 pMFD->ulBotRightBarValue,
                                 cBarDataType,
                                 pMFD->i32_HorizontalPanScan,
                                 pMFD->i32_VerticalPanScan,
                                 pMFD->eMpegType,
                                 ( pMFD->eChrominanceInterpolationMode ) ? "Fr" : "Fld",
                                 pMFD->eColorPrimaries,
                                 pMFD->eTransferCharacteristics,
                                 pMFD->eMatrixCoefficients,
                                 pMFD->eColorRange,
                                 BXDM_P_BAVCPictureCodingToStrLUT[ pMFD->ePictureType ],
                                 ( BAVC_VideoBitDepth_e10Bit == pMFD->eBitDepth ) ? "10" : "8",
                                 pMFD->ulChunkId,
                                 ( pMFD->bFrameProgressive ) ? " fp" : " ",
                                 ( pMFD->bStreamProgressive ) ? " sp" : " "
                                 ));


   BDBG_LEAVE( BXDM_PPDF_S_PrintMFD_isrsafe );

   return;

}  /* end of BXDM_PPDF_S_PrintMFD */

/*
 * Routines for dealing with Unified Pictures.
 */
static void BXDM_PPDF_S_PrintUnifiedPicture_isrsafe(
   BXDM_P_DebugFifo_Entry * pstEntry
   )
{
   BXDM_Picture_Coding eCoding;
   char  cSourceFormat, cProgressiveSequence;
   BXDM_DebugFifo_UnifiedPicture * pUniP = (BXDM_DebugFifo_UnifiedPicture *)&pstEntry->data.stUniPic;
   BXDM_Picture * pstUnified = (BXDM_Picture *)&pUniP->stUnifiedPicture;

   BDBG_ENTER( BXDM_PPDF_S_PrintUnifiedPicture_isrsafe );

   BDBG_ASSERT( pstEntry );

   if ( true == pUniP->bPrintAdditional )
   {
      bool bProtocolValid;
      BAVC_FrameRateCode eFrameRate = BAVC_FrameRateCode_eUnknown;
      BFMT_AspectRatio eAspectRatio = BFMT_AspectRatio_eUnknown;

      /* Range check the variables used to index lookup tables. */

      bProtocolValid = ( pstUnified->stProtocol.eProtocol < BXDM_P_MAX_VIDEO_PROTOCOL ) ? true : false ;

      if ( pUniP->eFrameRate < BXDM_PictureProvider_P_MAX_FRAMERATE )
      {
         eFrameRate = pUniP->eFrameRate;
      }

      if (  pstUnified->stAspectRatio.eAspectRatio <= BFMT_AspectRatio_eSAR )
      {
         eAspectRatio =  pstUnified->stAspectRatio.eAspectRatio;
      }

      BDBG_MODULE_MSG( BXDM_PPQM, (" %x:[%02x.%03x] %4dx%3d %s%scfr:%d/%d afr:%sHz(%s),ar:%s(%d,%d)(%d) swOff:%x avPts:%d.%u %s%s",
                             pstEntry->stMetadata.uiVsyncCount,
                             BXDM_PPDF_FORMAT_INSTANCE_ID( pstEntry->stMetadata.uiInstanceID ),
                             pstEntry->stMetadata.uiPPBIndex & 0xFFF,
                             pstUnified->stBufferInfo.stSource.uiWidth,
                             pstUnified->stBufferInfo.stSource.uiHeight,
                             ( bProtocolValid ) ? BXDM_P_VideoCompressionStdToStrLUT[ pstUnified->stProtocol.eProtocol ] : "ukn" ,
                             ( pstUnified->stBufferInfo.eBufferFormat == BXDM_Picture_BufferFormat_eSplitInterlaced ) ?  "-si," : "," ,
                             pstUnified->stFrameRate.stRate.uiNumerator,
                             pstUnified->stFrameRate.stRate.uiDenominator,
                             BXDM_P_BAVCFrameRateToStrLUT[ eFrameRate ],
                             BXDM_P_FrameRateTypeToStrLUT[ pUniP->eFrameRateType ],
                             BXDM_P_AspectRatioToStrLUT[ eAspectRatio ],
                             pstUnified->stAspectRatio.uiSampleAspectRatioX,
                             pstUnified->stAspectRatio.uiSampleAspectRatioY,
                             pstUnified->stAspectRatio.bValid,
                             pUniP->uiSwPcrOffsetUsedForEvaluation,
                             pUniP->stDeltaPTSAvg.uiWhole,
                             pUniP->uiAverageFractionBase10,
                             BXDM_P_DisplayFieldModeToStrLUT[ pUniP->eDisplayFieldMode ],
                             ( pUniP->bForceSingleFieldMode ) ? "-f" :
                                 ( BXDM_PictureProvider_DisplayFieldMode_eAuto == pUniP->eDisplayFieldMode ) ? "-a" : ""
                             ));
   }


   /* The following values from the Unified Picture are set outside of XDM.  Range check the values to avoid
    * stepping off the end of lookup tables, i.e. "s_aXXXToStrLUT[]"
    */
   eCoding = ( pstUnified->stPictureType.eCoding < BXDM_Picture_Coding_eMax ) ? pstUnified->stPictureType.eCoding : BXDM_Picture_Coding_eUnknown ;

   switch ( pstUnified->stBufferInfo.eSourceFormat )
   {
      case BXDM_Picture_SourceFormat_eInterlaced:     cSourceFormat = 'I';    break;
      case BXDM_Picture_SourceFormat_eProgressive:    cSourceFormat = 'P';    break;
      case BXDM_Picture_SourceFormat_eUnknown:
      default:                                        cSourceFormat = 'U';    break;
   }


   switch ( pstUnified->stPictureType.eSequence )
   {
      case BXDM_Picture_Sequence_eInterlaced:   cProgressiveSequence = 'I';    break;
      case BXDM_Picture_Sequence_eProgressive:  cProgressiveSequence = 'P';    break;
      case BXDM_Picture_Sequence_eUnknown:
      default:                                  cProgressiveSequence = 'U';    break;
   }

   if ( BXDM_PictureProvider_DisplayMode_eTSM == pUniP->eSelectionMode )
   {

      BDBG_MODULE_MSG( BXDM_PPQM, (" %x:[%02x.%03x]%c%c%c%s,%s,fs:%c%c %s,pts:%08x(%d),off:%04x,j:%3d,pcr:%08x(%d,%d),stc:%08x%c,d:%08x",
                             pstEntry->stMetadata.uiVsyncCount,
                             BXDM_PPDF_FORMAT_INSTANCE_ID( pstEntry->stMetadata.uiInstanceID ),
                             pstEntry->stMetadata.uiPPBIndex & 0xFFF,
                             pUniP->cErrorOnThisPicture,
                             pUniP->cSelectionMode,
                             ( true == pUniP->bAppendedToPreviousPicture ) ? '^' : ':',
                             BXDM_P_TSMResultToStrLUT[ pUniP->eTsmResult],
                             ( BXDM_Picture_BufferFormat_eSplitInterlaced == pstUnified->stBufferInfo.eBufferFormat ) ?
                                ( BXDM_Picture_BufferHandlingMode_eSiRepeat == pstUnified->stBufferInfo.eBufferHandlingMode ) ?
                                   BXDM_P_SiRepeatPullDownEnumToStrLUT[ pUniP->ePulldown ] : BXDM_P_SiPullDownEnumToStrLUT[ pUniP->ePulldown ] :
                                 BXDM_P_PullDownEnumToStrLUT[ pUniP->ePulldown ],
                             cSourceFormat,
                             cProgressiveSequence,
                             BXDM_P_BXDMPictureCodingToStrLUT[ eCoding ],
                             pstUnified->stPTS.uiValue,
                             pstUnified->stPTS.bValid,
                             pUniP->uiDisplayOffset,
                             pUniP->iPTSJitterCorrection,
                             pUniP->uiPcrOffset,
                             pUniP->bPcrOffsetValid,
                             pUniP->bPcrOffsetDiscontinuity,
                             pUniP->uiStcSnapshot,
                             (pUniP->iStcJitterCorrectionOffset != 0)?'*':' ',
                             pUniP->iStcPtsDifferenceActual
                             ));
   }
   else
   {
      BDBG_MODULE_MSG( BXDM_PPQM, (" %x:[%02x.%03x]%c%c%c%s,%s,fs:%c%c %s,pts:%08x(%d),off:%04x,j:   ,vpts:%08x   ,vstc:%08x ,d:%08x",
                             pstEntry->stMetadata.uiVsyncCount,
                             BXDM_PPDF_FORMAT_INSTANCE_ID( pstEntry->stMetadata.uiInstanceID ),
                             pstEntry->stMetadata.uiPPBIndex & 0xFFF,
                             pUniP->cErrorOnThisPicture,
                             pUniP->cSelectionMode,
                             ( true == pUniP->bAppendedToPreviousPicture ) ? '^' : ':',
                             BXDM_P_TSMResultToStrLUT[ pUniP->eTsmResult],
                             ( BXDM_Picture_BufferFormat_eSplitInterlaced == pstUnified->stBufferInfo.eBufferFormat ) ?
                                ( BXDM_Picture_BufferHandlingMode_eSiRepeat == pstUnified->stBufferInfo.eBufferHandlingMode ) ?
                                   BXDM_P_SiRepeatPullDownEnumToStrLUT[ pUniP->ePulldown ] : BXDM_P_SiPullDownEnumToStrLUT[ pUniP->ePulldown ] :
                                 BXDM_P_PullDownEnumToStrLUT[ pUniP->ePulldown ],
                             cSourceFormat,
                             cProgressiveSequence,
                             BXDM_P_BXDMPictureCodingToStrLUT[ eCoding ],
                             pstUnified->stPTS.uiValue,
                             pstUnified->stPTS.bValid,
                             pUniP->uiDisplayOffset,
                             pUniP->stVirtualPts.uiWhole,
                             pUniP->uiVirtualStc,
                             pUniP->iStcPtsDifferenceActual
                             ));

   }


   BDBG_LEAVE( BXDM_PPDF_S_PrintUnifiedPicture_isrsafe );

   return;

}  /* end of BXDM_PPDF_S_PrintUnifiedPicture */

static void BXDM_PPDF_S_PrintDebugInfo_isrsafe(
   BXDM_P_DebugFifo_Entry * pstEntry
   )
{
   BXDM_DebugFifo_DebugInfo * pDbgInfo = (BXDM_DebugFifo_DebugInfo *)&pstEntry->data.stDebugInfo;

   BDBG_ENTER( BXDM_PPDF_S_PrintDebugInfo_isrsafe );

   BDBG_MODULE_MSG( BXDM_PPDBG, (" DM Log (ch:%02x stc:%08x%c mr:%sHz edSTC:%u adSTC:%d.%u pbr:%d%c tm:%s %s)",
                          BXDM_PPDF_FORMAT_INSTANCE_ID( pstEntry->stMetadata.uiInstanceID ),
                          pDbgInfo->uiStcSnapshot,
                          (pDbgInfo->iStcJitterCorrectionOffset != 0)?'*':' ',
                          BXDM_P_MonitorRefreshRateToStrLUT[pDbgInfo->eMonitorRefreshRate],
                          pDbgInfo->stSTCDelta.uiWhole,
                          pDbgInfo->iAverageStcDelta,
                          pDbgInfo->uiAverageFractionBase10,
                          ( pDbgInfo->uiSlowMotionRate / BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE_EXTRA_DECIMALS ),
                          '%',
                          ( pDbgInfo->eSTCTrickMode < BXDM_PictureProvider_P_STCTrickMode_eMax) ?
                                    BXDM_P_STCTrickModeToStrLUT[ pDbgInfo->eSTCTrickMode ] : "error",
                          pDbgInfo->bPlayback?"pb":"lv"
                          ));

   BDBG_LEAVE( BXDM_PPDF_S_PrintDebugInfo_isrsafe );

   return;

}

/*
 * Public API's.
 */

void BXDM_PictureProvider_PrintEntry_isrsafe(
   BXDM_P_DebugFifo_Entry * pstEntry
   )
{
   BDBG_ENTER(BXDM_PictureProvider_PrintEntry_isrsafe);

   switch( pstEntry->stMetadata.eType )
   {
      case BXDM_DebugFifo_EntryType_eMFD:
         BXDM_PPDF_S_PrintMFD_isrsafe( pstEntry );
         break;

      case BXDM_DebugFifo_EntryType_eUnifiedPicture:
         BXDM_PPDF_S_PrintUnifiedPicture_isrsafe( pstEntry );
         break;

      case BXDM_DebugFifo_EntryType_eString:
         BXDM_PPDF_S_PrintString_isrsafe( pstEntry );
         break;

      case BXDM_DebugFifo_EntryType_eDebugInfo:
         BXDM_PPDF_S_PrintDebugInfo_isrsafe( pstEntry );
         break;

      default:
         BDBG_MSG(("%s: unknown message type:%d.", __FUNCTION__, pstEntry->stMetadata.eType ));
         break;

   }

   BDBG_LEAVE(BXDM_PictureProvider_PrintEntry_isrsafe);

   return;

}


void BXDM_PictureProvider_ReadFifo_isrsafe(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BDBG_ENTER(BXDM_PictureProvider_ReadFifo_isrsafe);

   if ( NULL ==  hXdmPP->stDMConfig.stDebugReader.hDebugReader)
   {
      BDBG_MSG(("%s::  hXdmPP->stDMConfig.stDebugReader.hDebugReader== NULL", __FUNCTION__ ));
      goto error;
   }

   for(;;)
   {
      BERR_Code rc = BDBG_FifoReader_Read(
                           hXdmPP->stDMConfig.stDebugReader.hDebugReader,
                           hXdmPP->stDMConfig.stDebugReader.pEntry,
                           hXdmPP->stDMConfig.stDebugReader.stFifoInfo.uiElementSize );

      if( rc == BERR_SUCCESS )
      {
         BXDM_P_DebugFifo_Entry * pstEntry;

         pstEntry = (BXDM_P_DebugFifo_Entry *)hXdmPP->stDMConfig.stDebugReader.pEntry;

         BXDM_PictureProvider_PrintEntry_isrsafe( pstEntry );

      }
      else if(rc==BERR_FIFO_OVERFLOW)
      {
         BDBG_LOG(("DebugLog FIFO OVERFLOW"));
         BDBG_FifoReader_Resync(hXdmPP->stDMConfig.stDebugReader.hDebugReader);
      }
      else if (rc==BERR_FIFO_NO_DATA)
      {
         /*BKNI_Printf("element size %d\n", hXdmPP->stDMConfig.stDebugReader.stFifoInfo.uiElementSize );*/
         break;
      }
      else
      {
         BKNI_Printf(" . ");
         break;
      }
   }

error:

   BDBG_LEAVE(BXDM_PictureProvider_ReadFifo_isrsafe);

   return;

}

#endif
