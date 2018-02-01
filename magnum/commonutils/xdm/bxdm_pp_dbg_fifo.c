/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#define BXDM_PPDF_MESSAGE_SIZE 128

#if BDBG_DEBUG_BUILD

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
         BDBG_MODULE_MSG( BXDM_PPDBG, ("%s: unknown message type", BSTD_FUNCTION ));
         BDBG_MODULE_MSG( BXDM_PPDBG, ("%s", pstEntry->data.stString.szString ));
         break;

   }

   BDBG_LEAVE( BXDM_PPDF_S_PrintString_isrsafe );

   return;
}

/*
 * Routines for formating the MFD debug messages.
 */

int BXDM_PictureProvider_Debug_FormatMsg_MFD1_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   )
{
   const BAVC_MFD_Picture * pMFD;
   BFMT_AspectRatio eAspectRatio=BFMT_AspectRatio_eUnknown;
   int32_t iBytesUsed;

   BDBG_ENTER( BXDM_PictureProvider_Debug_FormatMsg_MFD1_isrsafe );

   BDBG_ASSERT( pstEntry );
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );


   pMFD = &pstEntry->data.stMFD;

   /* Range check */

   if ( pMFD->eAspectRatio <= BFMT_AspectRatio_eSAR )
   {
      eAspectRatio = pMFD->eAspectRatio;
   }

   iBytesUsed = BKNI_Snprintf(
                  szMessage,
                  uiSize,
                  "%c%x:[%02x.%03x] id:%03x pts:%08x %s->%s %dx%d->%dx%d %s:%dx%d AQP:%02d fr:%s mr:%s %s%s%s%s%s%s%s%s%s%s",
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
                  ( pMFD->bEndOfChunk) ? " eoc" : " "
               );

   BDBG_LEAVE( BXDM_PictureProvider_Debug_FormatMsg_MFD1_isrsafe );

   return iBytesUsed;

}

int BXDM_PictureProvider_Debug_FormatMsg_MFD2_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   )
{
   const BAVC_MFD_Picture * pMFD;
   char cBarDataType;
   int32_t iBytesUsed;

   BDBG_ENTER( BXDM_PictureProvider_Debug_FormatMsg_MFD2_isrsafe );

   BDBG_ASSERT( pstEntry );
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );

   pMFD = &pstEntry->data.stMFD;

   switch( pMFD->eBarDataType )
   {
      case BAVC_BarDataType_eTopBottom:   cBarDataType = 'T';     break;
      case BAVC_BarDataType_eLeftRight:   cBarDataType = 'L';     break;
      default:                            cBarDataType = 'i';     break;
   }

   iBytesUsed = BKNI_Snprintf(
                  szMessage,
                  uiSize,
                  "%c%x:[%02x.%03x] clp:%dx%d afd:%d(%d) bar:%dx%d(%c) pan:%dx%d loc:%d ci:%s cp:%d tc:%d mc:%d cr:%d %s dp:%s chk:%08x %s%s",
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
               );

   BDBG_LEAVE( BXDM_PictureProvider_Debug_FormatMsg_MFD2_isrsafe );

   return iBytesUsed;

}



int BXDM_PictureProvider_Debug_FormatMsg_MFD3_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   )
{
   const BAVC_MFD_Picture * pMFD;
   int32_t iBytesUsed;

   BDBG_ENTER( BXDM_PictureProvider_Debug_FormatMsg_MFD3_isrsafe );

   BDBG_ASSERT( pstEntry );
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );

   pMFD = &pstEntry->data.stMFD;

   iBytesUsed = BKNI_Snprintf(
                  szMessage,
                  uiSize,
                  "%c%x:[%02x.%03x] %s md:%d top::lb:0x%lu lo:%08x cb:0x%lu co:%08x bot::lb:0x%lu lo:%08x cb:0x%lu co:%08x",
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
               );

   BDBG_LEAVE( BXDM_PictureProvider_Debug_FormatMsg_MFD3_isrsafe );

   return iBytesUsed;

}

/*
 * SW7405-4736: conditionally print the MFD structure.
 */
static void BXDM_PPDF_S_PrintMFD_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry
   )
{
   char szMessage[BXDM_PPDF_MESSAGE_SIZE];

   BDBG_ENTER( BXDM_PPDF_S_PrintMFD_isrsafe );

   BDBG_ASSERT( pstEntry );

   BXDM_PictureProvider_Debug_FormatMsg_MFD1_isrsafe( pstEntry, szMessage, BXDM_PPDF_MESSAGE_SIZE );
   BDBG_MODULE_MSG( BXDM_MFD1, ("%s", szMessage ));

   BXDM_PictureProvider_Debug_FormatMsg_MFD3_isrsafe( pstEntry, szMessage, BXDM_PPDF_MESSAGE_SIZE );
   BDBG_MODULE_MSG( BXDM_MFD3, ("%s", szMessage ));

   /* SW7405-4736: Ideally the following would be printed out once per second or when any of the values
    * change, this would save extraneous prints to the console.  The values could be tracked by keeping
    * a local version of the MFD structure, but that would involve copying the data an extra time.
    * The memory pointed to by pMFD is allocated by XDM, i.e. "hXdmPP->astMFDPicture"; essentially XDM
    * already has a local copy.  However the "astMFDPicture" elements are set throughout
    * "BXDM_PPOUT_P_CalculateVdcData_isr".  The code to check for a change in value would have to be added
    * in multiple locations, a task for another day.
    */

   BXDM_PictureProvider_Debug_FormatMsg_MFD2_isrsafe( pstEntry, szMessage, BXDM_PPDF_MESSAGE_SIZE );
   BDBG_MODULE_MSG( BXDM_MFD2, ("%s", szMessage ));

   BDBG_LEAVE( BXDM_PPDF_S_PrintMFD_isrsafe );

   return;

}  /* end of BXDM_PPDF_S_PrintMFD */

/*
 * Routines for processing the BXDM_PPQM messages.
 */

int BXDM_PictureProvider_Debug_FormatMsg_PPQM_Bonus_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   )
{
   BXDM_DebugFifo_UnifiedPicture * pUniP = (BXDM_DebugFifo_UnifiedPicture *)&pstEntry->data.stUniPic;
   BXDM_Picture * pstUnified = (BXDM_Picture *)&pUniP->stUnifiedPicture;
   bool bProtocolValid;
   BAVC_FrameRateCode eFrameRate = BAVC_FrameRateCode_eUnknown;
   BFMT_AspectRatio eAspectRatio = BFMT_AspectRatio_eUnknown;
   int32_t iBytesUsed;

   BDBG_ENTER( BXDM_PictureProvider_Debug_FormatMsg_PPQM_Bonus_isrsafe );

   BDBG_ASSERT( pstEntry );
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );

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

   iBytesUsed = BKNI_Snprintf(
      szMessage,
      uiSize,
      " %x:[%02x.%03x] %4dx%3d %s%scfr:%d/%d afr:%sHz(%s),ar:%s(%d,%d)(%d) swOff:%x avPts:%d.%u %s%s",
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
   );

   BDBG_LEAVE( BXDM_PictureProvider_Debug_FormatMsg_PPQM_Bonus_isrsafe );

   return iBytesUsed;

}

int BXDM_PictureProvider_Debug_FormatMsg_PPQM_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   )
{
   BXDM_Picture_Coding eCoding;
   char  cSourceFormat, cProgressiveSequence;
   BXDM_DebugFifo_UnifiedPicture * pUniP = (BXDM_DebugFifo_UnifiedPicture *)&pstEntry->data.stUniPic;
   BXDM_Picture * pstUnified = (BXDM_Picture *)&pUniP->stUnifiedPicture;
   int32_t iBytesUsed;

   BDBG_ENTER( BXDM_PictureProvider_Debug_FormatMsg_PPQM_isrsafe );

   BDBG_ASSERT( pstEntry );
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );

   /* The following values from the Unified Picture are set outside of XDM.  Range check the values to avoid
    * stepping off the end of lookup tables, i.e. "s_aXXXToStrLUT[]"  */

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
      iBytesUsed = BKNI_Snprintf(
                     szMessage,
                     uiSize,
                     " %x:[%02x.%03x]%c%c%c%s,%s,fs:%c%c %s,pts:%08x(%d),off:%04x,j:%3d,pcr:%08x(%d,%d),stc:%08x%c,d:%08x",
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
                  );

   }
   else
   {
      iBytesUsed = BKNI_Snprintf(
                     szMessage,
                     uiSize,
                     " %x:[%02x.%03x]%c%c%c%s,%s,fs:%c%c %s,pts:%08x(%d),off:%04x,j:   ,vpts:%08x   ,vstc:%08x ,d:%08x",
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
                  );

   }

   BDBG_LEAVE( BXDM_PictureProvider_Debug_FormatMsg_PPQM_isrsafe );

   return iBytesUsed;

}



static void BXDM_PPDF_S_PrintUnifiedPicture_isrsafe(
   BXDM_P_DebugFifo_Entry * pstEntry
   )
{
   char szMessage[BXDM_PPDF_MESSAGE_SIZE];

   BXDM_DebugFifo_UnifiedPicture * pUniP = (BXDM_DebugFifo_UnifiedPicture *)&pstEntry->data.stUniPic;

   BDBG_ENTER( BXDM_PPDF_S_PrintUnifiedPicture_isrsafe );

   BDBG_ASSERT( pstEntry );

   if ( true == pUniP->bPrintAdditional )
   {
      BXDM_PictureProvider_Debug_FormatMsg_PPQM_Bonus_isrsafe( pstEntry, szMessage, BXDM_PPDF_MESSAGE_SIZE );
      BDBG_MODULE_MSG( BXDM_PPQM, ("%s", szMessage ));
   }

   BXDM_PictureProvider_Debug_FormatMsg_PPQM_isrsafe( pstEntry, szMessage, BXDM_PPDF_MESSAGE_SIZE );
   BDBG_MODULE_MSG( BXDM_PPQM, ("%s", szMessage ));

   BDBG_LEAVE( BXDM_PPDF_S_PrintUnifiedPicture_isrsafe );

   return;

}  /* end of BXDM_PPDF_S_PrintUnifiedPicture */

/*
 * Routines for processing the BXDM_PPDBG messages.
 */

int BXDM_PictureProvider_Debug_FormatMsg_PPDBG_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   )
{
   BXDM_DebugFifo_DebugInfo * pDbgInfo = (BXDM_DebugFifo_DebugInfo *)&pstEntry->data.stDebugInfo;
   int32_t iBytesUsed;

   BDBG_ENTER( BXDM_PictureProvider_Debug_FormatMsg_PPDBG_isrsafe );

   BDBG_ASSERT( pstEntry );
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );

   iBytesUsed = BKNI_Snprintf(
                  szMessage,
                  uiSize,
                  " DM Log (ch:%02x stc:%08x%c mr:%sHz edSTC:%u adSTC:%d.%u pbr:%d%c tm:%s %s)",
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
               );

   BDBG_LEAVE( BXDM_PictureProvider_Debug_FormatMsg_PPDBG_isrsafe );

   return iBytesUsed;

}



static void BXDM_PPDF_S_PrintDebugInfo_isrsafe(
   BXDM_P_DebugFifo_Entry * pstEntry
   )
{
   char szMessage[BXDM_PPDF_MESSAGE_SIZE];

   BDBG_ENTER( BXDM_PPDF_S_PrintDebugInfo_isrsafe );

   BXDM_PictureProvider_Debug_FormatMsg_PPDBG_isrsafe( pstEntry, szMessage, BXDM_PPDF_MESSAGE_SIZE );
   BDBG_MODULE_MSG( BXDM_PPDBG, ("%s", szMessage ));

   BDBG_LEAVE( BXDM_PPDF_S_PrintDebugInfo_isrsafe );

   return;

}

/*
 * Routines for processing the BXDM_CFG messages.
 */

int BXDM_PictureProvider_Debug_FormatMsg_Config1_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   )
{
   BXDM_DebugFifo_Config * pstConfig;
   int32_t iBytesUsed=0;

   BDBG_ENTER( BXDM_PictureProvider_Debug_FormatMsg_Config1_isrsafe );

   BDBG_ASSERT( pstEntry );
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );

   pstConfig = (BXDM_DebugFifo_Config *)&pstEntry->data.stConfigInfo;

   /* Print when any of the parameters change. */

   if (  pstConfig->stConfig.uiDirtyBits_1 || pstConfig->stConfig.uiDirtyBits_2 )
   {
      uint32_t uiDirtyBitsGroup_1 = pstConfig->stConfig.uiDirtyBits_1;
      uint32_t uiDirtyBitsGroup_2 = pstConfig->stConfig.uiDirtyBits_2;

      uiDirtyBitsGroup_1 &= BXDM_PictureProvider_P_DIRTY_1_STC_VALID
                              | BXDM_PictureProvider_P_DIRTY_1_PTS_OFFSET
                              | BXDM_PictureProvider_P_DIRTY_1_SOFTWARE_PCR_OFFSET
                              | BXDM_PictureProvider_P_DIRTY_1_USE_HW_PCR_OFFSET
                              | BXDM_PictureProvider_P_DIRTY_1_PLAYBACK_RATE
                              | BXDM_PictureProvider_P_DIRTY_1_DISPLAY_FIELD_MODE;

      uiDirtyBitsGroup_2 &= BXDM_PictureProvider_P_DIRTY_2_MUTE;

      if ( uiDirtyBitsGroup_1 || uiDirtyBitsGroup_2 )
      {

         iBytesUsed = BKNI_Snprintf(
                        szMessage,
                        uiSize,
                        "%c%x:[%02x.xxx]1:%c stc:%c(%d) PTSoff:%08x(%d) STCoff:%08x(%d) usePCR:%c(%d) pbr:%d/%d(%d) dfm:%s(%d)",
                        ( pstConfig->bLastCall ) ? '*' : ' ',
                        pstEntry->stMetadata.uiVsyncCount,
                        BXDM_PPDF_FORMAT_INSTANCE_ID( pstEntry->stMetadata.uiInstanceID ),
                        ( pstConfig->stConfig.bMute == true ) ? 'M' : ' ',
            /*                  ( pstConfig->stConfig.uiDirtyBits_2 & BXDM_PictureProvider_P_DIRTY_2_MUTE ) ? 1 : 0,*/
                        ( pstConfig->stConfig.bSTCValid == true ) ? 'v' : 'i',
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_STC_VALID ) ? 1 : 0,
                        pstConfig->stConfig.uiPTSOffset,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_PTS_OFFSET ) ? 1 : 0,
                        pstConfig->stConfig.uiSoftwarePCROffset,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_SOFTWARE_PCR_OFFSET ) ? 1 : 0,
                        ( pstConfig->stConfig.bUseHardwarePCROffset == true) ? 't' : 'f',
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_USE_HW_PCR_OFFSET ) ? 1 : 0,
                        pstConfig->stConfig.stPlaybackRate.uiNumerator,
                        pstConfig->stConfig.stPlaybackRate.uiDenominator,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_PLAYBACK_RATE ) ? 1 : 0,
                        BXDM_P_DisplayFieldModeToStrLUT[ pstConfig->stConfig.eDisplayFieldMode ],
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_DISPLAY_FIELD_MODE ) ? 1 : 0
                     );


      }     /* end of  if ( uiDirtyBitsGroup_1_1 || uiDirtyBitsGroup_2_1 ) */



   }       /* end of  if ( pstConfig->stConfig.uiDirtyBits_1 || pstConfig->stConfig.uiDirtyBits_2 ) */


   BDBG_LEAVE( BXDM_PictureProvider_Debug_FormatMsg_Config1_isrsafe );

   return iBytesUsed;

}

int BXDM_PictureProvider_Debug_FormatMsg_Config2_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   )
{
   BXDM_DebugFifo_Config * pstConfig;
   int32_t iBytesUsed=0;

   BDBG_ENTER( BXDM_PictureProvider_Debug_FormatMsg_Config2_isrsafe );

   BDBG_ASSERT( pstEntry );
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );

   pstConfig = (BXDM_DebugFifo_Config *)&pstEntry->data.stConfigInfo;

   /* Print when any of the parameters change. */

   if (  pstConfig->stConfig.uiDirtyBits_1 || pstConfig->stConfig.uiDirtyBits_2 )
   {
      uint32_t uiDirtyBitsGroup_1 = pstConfig->stConfig.uiDirtyBits_1;
      uint32_t uiDirtyBitsGroup_2 = pstConfig->stConfig.uiDirtyBits_2;

      uiDirtyBitsGroup_1 &= BXDM_PictureProvider_P_DIRTY_1_PLAYBACK
                              | BXDM_PictureProvider_P_DIRTY_1_PROTOCOL
                              | BXDM_PictureProvider_P_DIRTY_1_MONITOR_REFRESH_RATE
                              | BXDM_PictureProvider_P_DIRTY_1_DEFAULT_FRAME_RATE
                              | BXDM_PictureProvider_P_DIRTY_1_FRAMERATE_OVERRIDE
                              | BXDM_PictureProvider_P_DIRTY_1_TRICK_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_FREEZE ;

      uiDirtyBitsGroup_2 &= BXDM_PictureProvider_P_DIRTY_2_DISPLAY_MODE;

      if ( uiDirtyBitsGroup_1 || uiDirtyBitsGroup_2 )
      {
         iBytesUsed = BKNI_Snprintf(
                        szMessage,
                        uiSize,
                        "%c%x:[%02x.xxx]2:%s(%d) %s(%d) %s(%d) mr:%s(%d) dfr:%s(%d) fro:%d/%d(%d,%d)(%d) tm:%s(%d) frz:%c(%d)",
                        ( pstConfig->bLastCall ) ? '*' : ' ',
                        pstEntry->stMetadata.uiVsyncCount,
                        BXDM_PPDF_FORMAT_INSTANCE_ID( pstEntry->stMetadata.uiInstanceID ),
                        ( pstConfig->stConfig.eDisplayMode == BXDM_PictureProvider_DisplayMode_eTSM ) ? "TSM" : "vsync",
                        ( pstConfig->stConfig.uiDirtyBits_2 & BXDM_PictureProvider_P_DIRTY_2_DISPLAY_MODE ) ? 1 : 0,
                        ( pstConfig->stConfig.bPlayback == true ) ? "pb" : "lv",
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_PLAYBACK ) ? 1 : 0,
                        BXDM_P_VideoCompressionStdToStrLUT[ pstConfig->stConfig.eProtocol ],
                        ( pstConfig->stConfig.uiDirtyBits_2 & BXDM_PictureProvider_P_DIRTY_1_PROTOCOL ) ? 1 : 0,
                        BXDM_P_MonitorRefreshRateToStrLUT[ pstConfig->stConfig.eMonitorRefreshRate ],
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_MONITOR_REFRESH_RATE ) ? 1 : 0,
                        BXDM_P_BAVCFrameRateToStrLUT[ pstConfig->stConfig.eDefaultFrameRate ],
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_DEFAULT_FRAME_RATE ) ? 1 : 0,
                        pstConfig->stConfig.stFrameRateOverride.stRate.uiNumerator,
                        pstConfig->stConfig.stFrameRateOverride.stRate.uiDenominator,
                        pstConfig->stConfig.stFrameRateOverride.bValid,
                        pstConfig->stConfig.stFrameRateOverride.bTreatAsSingleElement,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_FRAMERATE_OVERRIDE ) ? 1 : 0,
                        BXDM_P_TrickModeToStrLUT[ pstConfig->stConfig.eTrickMode ],
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_TRICK_MODE ) ? 1 : 0,
                        ( pstConfig->stConfig.bFreeze == true ) ? 't' : 'f',
                        ( pstConfig->stConfig.uiDirtyBits_2 & BXDM_PictureProvider_P_DIRTY_1_FREEZE ) ? 1 : 0
                     );

      } /* end of if ( uiDirtyBitsGroup_1_2 || uiDirtyBitsGroup_2_2 ) */

   }       /* end of  if ( pstConfig->stConfig.uiDirtyBits_1 || pstConfig->stConfig.uiDirtyBits_2 ) */


   BDBG_LEAVE( BXDM_PictureProvider_Debug_FormatMsg_Config2_isrsafe );

   return iBytesUsed;

}

int BXDM_PictureProvider_Debug_FormatMsg_Config3_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   )
{
   BXDM_DebugFifo_Config * pstConfig;
   int32_t iBytesUsed=0;

   BDBG_ENTER( BXDM_PictureProvider_Debug_FormatMsg_Config3_isrsafe );

   BDBG_ASSERT( pstEntry );
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );

   pstConfig = (BXDM_DebugFifo_Config *)&pstEntry->data.stConfigInfo;

   /* Print when any of the parameters change. */

   if (  pstConfig->stConfig.uiDirtyBits_1 || pstConfig->stConfig.uiDirtyBits_2 )
   {
      uint32_t uiDirtyBitsGroup_1 = pstConfig->stConfig.uiDirtyBits_1;

      uiDirtyBitsGroup_1 &= BXDM_PictureProvider_P_DIRTY_1_SRC_FORMAT_OVERRIDE
                              | BXDM_PictureProvider_P_DIRTY_1_SCAN_MODE_OVERRIDE
                              | BXDM_PictureProvider_P_DIRTY_1_1080P_SCAN_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_PICTURE_DROP_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_480P_PULLDOWN_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_1080P_PULLDOWN_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_240I_SCAN_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_HORIZON_OVERSCAN_MODE;


      if ( uiDirtyBitsGroup_1 )
      {
         char cSFO, cSMO;

         switch( pstConfig->stConfig.eSourceFormatOverride )
         {
            case BXDM_PictureProvider_SourceFormatOverride_eInterlaced:    cSFO = 'P';    break;
            case BXDM_PictureProvider_SourceFormatOverride_eProgressive:   cSFO = 'I';    break;
            case BXDM_PictureProvider_SourceFormatOverride_eDefault:
            default:                                                       cSFO = 'D';    break;
         }

         switch( pstConfig->stConfig.eScanModeOverride )
         {
            case BXDM_PictureProvider_ScanModeOverride_eInterlaced:     cSMO = 'P';    break;
            case BXDM_PictureProvider_ScanModeOverride_eProgressive:    cSMO = 'I';    break;
            case BXDM_PictureProvider_ScanModeOverride_eDefault:
            default:                                                    cSMO = 'D';    break;
         }

         iBytesUsed = BKNI_Snprintf(
                        szMessage,
                        uiSize,
                        "%c%x:[%02x.xxx]3:sfo:%c(%d) smo:%c(%d) 1080sm:%c(%d) pdm:%s(%d) 480pdm:%s(%d) 1080pdm:%s(%d) 240sm:%s(%d) hom:%s(%d)",
                        ( pstConfig->bLastCall ) ? '*' : ' ',
                        pstEntry->stMetadata.uiVsyncCount,
                        BXDM_PPDF_FORMAT_INSTANCE_ID( pstEntry->stMetadata.uiInstanceID ),
                        cSFO,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_SRC_FORMAT_OVERRIDE ) ? 1 : 0,
                        cSMO,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_SCAN_MODE_OVERRIDE ) ? 1 : 0,
                        ( pstConfig->stConfig.e1080pScanMode == BXDM_PictureProvider_1080pScanMode_eDefault ) ? 'D' : 'A' ,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_1080P_SCAN_MODE ) ? 1 : 0,
                        ( pstConfig->stConfig.ePictureDropMode == BXDM_PictureProvider_PictureDropMode_eField ) ? "fld" : "Frm" ,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_PICTURE_DROP_MODE ) ? 1 : 0,
                        BXDM_P_PulldownModeToStrLUT[ pstConfig->stConfig.e480pPulldownMode ],
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_480P_PULLDOWN_MODE ) ? 1 : 0,
                        BXDM_P_PulldownModeToStrLUT[ pstConfig->stConfig.e1080pPulldownMode ],
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_1080P_PULLDOWN_MODE ) ? 1 : 0,
                        ( pstConfig->stConfig.e240iScanMode == BXDM_PictureProvider_240iScanMode_eForceProgressive ) ? "fp" : "en" ,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_240I_SCAN_MODE ) ? 1 : 0,
                        ( pstConfig->stConfig.eHorizontalOverscanMode == BXDM_PictureProvider_HorizontalOverscanMode_eAuto ) ? "auto" : "dis" ,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_HORIZON_OVERSCAN_MODE ) ? 1 : 0
                     );

      }     /* end of if ( uiDirtyBitsGroup_1 ) */
   }       /* end of  if ( pstConfig->stConfig.uiDirtyBits_1 || pstConfig->stConfig.uiDirtyBits_2 ) */


   BDBG_LEAVE( BXDM_PictureProvider_Debug_FormatMsg_Config3_isrsafe );

   return iBytesUsed;

}

int BXDM_PictureProvider_Debug_FormatMsg_Config4_isrsafe(
   const BXDM_P_DebugFifo_Entry * pstEntry,
   char *szMessage,
   size_t uiSize
   )
{
   BXDM_DebugFifo_Config * pstConfig;
   int32_t iBytesUsed=0;

   BDBG_ENTER( BXDM_PictureProvider_Debug_FormatMsg_Config4_isrsafe );

   BDBG_ASSERT( pstEntry );
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );

   pstConfig = (BXDM_DebugFifo_Config *)&pstEntry->data.stConfigInfo;

   /* Print when any of the parameters change. */

   if (  pstConfig->stConfig.uiDirtyBits_1 || pstConfig->stConfig.uiDirtyBits_2 )
   {
      uint32_t uiDirtyBitsGroup_1 = pstConfig->stConfig.uiDirtyBits_1;
      uint32_t uiDirtyBitsGroup_2 = pstConfig->stConfig.uiDirtyBits_2;

      uiDirtyBitsGroup_1 &= BXDM_PictureProvider_P_DIRTY_1_FRAME_ADVANCE_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_REVERSE_FIELDS
                              | BXDM_PictureProvider_P_DIRTY_1_AUTO_VALIDATE_ON_PAUSE
                              | BXDM_PictureProvider_P_DIRTY_1_3D_SETTINGS
                              | BXDM_PictureProvider_P_DIRTY_1_JITTER_TOLERANCE
                              | BXDM_PictureProvider_P_DIRTY_1_FRAME_RATE_DETECTION_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_ERROR_HANDLING_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_ATSM_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_VTSM_ON_PCR_DISCON ;


      uiDirtyBitsGroup_2 &= BXDM_PictureProvider_P_DIRTY_2_ERROR_THRESHOLD;

      if ( uiDirtyBitsGroup_1 || uiDirtyBitsGroup_2 )
      {
         iBytesUsed = BKNI_Snprintf(
                        szMessage,
                        uiSize,
                        "%c%x:[%02x.xxx]4:fra:%s(%d) rvf:%c(%d) avop:%c(%d) 3Do:%s(%d)(%d) jti:%s(%d) frd:%s(%d) ehm:%s(%d) el:%d(%d) astm:%c(%d) avsync:%c(%d) ",
                        ( pstConfig->bLastCall ) ? '*' : ' ',
                        pstEntry->stMetadata.uiVsyncCount,
                        BXDM_PPDF_FORMAT_INSTANCE_ID( pstEntry->stMetadata.uiInstanceID ),
                        BXDM_P_FrameAdvanceModeToStrLUT[pstConfig->stConfig.eFrameAdvanceMode],
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_FRAME_ADVANCE_MODE ) ? 1 : 0,
                        ( pstConfig->stConfig.bReverseFields == true ) ? 't' : 'f' ,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_REVERSE_FIELDS ) ? 1 : 0,
                        ( pstConfig->stConfig.bAutoValidateStcOnPause == true ) ? 't' : 'f' ,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_AUTO_VALIDATE_ON_PAUSE ) ? 1 : 0,
                        BXDM_P_PPOrientationToStrLUT[ pstConfig->stConfig.st3DSettings.eOrientation ],
                        pstConfig->stConfig.st3DSettings.bOverrideOrientation,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_3D_SETTINGS ) ? 1 : 0,
                        ( pstConfig->stConfig.bJitterToleranceImprovement == true ) ? "on" : "off" ,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_JITTER_TOLERANCE ) ? 1 : 0,
                        BXDM_P_FrameRateDetectionModeToStrLUT[ pstConfig->stConfig.eFrameRateDetectionMode ],
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_FRAME_RATE_DETECTION_MODE ) ? 1 : 0,
                        BXDM_P_ErrorHandlingModeToStrLUT[ pstConfig->stConfig.eErrorHandlingMode ],
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_ERROR_HANDLING_MODE ) ? 1 : 0,
                        pstConfig->stConfig.uiErrorThreshold,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_2_ERROR_THRESHOLD ) ? 1 : 0,
                        ( pstConfig->stConfig.bAstmMode == true ) ? 't' : 'f' ,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_ATSM_MODE ) ? 1 : 0,
                        ( pstConfig->stConfig.bVirtualTSMOnPCRDiscontinuity == true ) ? 't' : 'f' ,
                        ( pstConfig->stConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_VTSM_ON_PCR_DISCON ) ? 1 : 0
                     );

      }     /* end of if ( uiDirtyBitsGroup_1 || uiDirtyBitsGroup_2 ) */


   }       /* end of  if ( pstConfig->stConfig.uiDirtyBits_1 || pstConfig->stConfig.uiDirtyBits_2 ) */

   BDBG_LEAVE( BXDM_PictureProvider_Debug_FormatMsg_Config4_isrsafe );

   return iBytesUsed;

}


static void BXDM_PPDF_S_PrintConfigInfo_isrsafe(
   BXDM_P_DebugFifo_Entry * pstEntry
   )
{
   char szMessage[BXDM_PPDF_MESSAGE_SIZE];
   int32_t iBytesUsed;
   uint32_t i;

   BDBG_ENTER( BXDM_PPDF_S_PrintConfigInfo_isrsafe );

   for ( i=0 ; i < 4; i++ )
   {
      switch( i )
      {
         case 0:
            iBytesUsed = BXDM_PictureProvider_Debug_FormatMsg_Config1_isrsafe( pstEntry, szMessage, BXDM_PPDF_MESSAGE_SIZE );
            break;

         case 1:
            iBytesUsed = BXDM_PictureProvider_Debug_FormatMsg_Config2_isrsafe( pstEntry, szMessage, BXDM_PPDF_MESSAGE_SIZE );
            break;

         case 2:
            iBytesUsed = BXDM_PictureProvider_Debug_FormatMsg_Config3_isrsafe( pstEntry, szMessage, BXDM_PPDF_MESSAGE_SIZE );
            break;

         case 3:
         default:
            iBytesUsed = BXDM_PictureProvider_Debug_FormatMsg_Config4_isrsafe( pstEntry, szMessage, BXDM_PPDF_MESSAGE_SIZE );
            break;
      }

      if ( iBytesUsed )
      {
         BDBG_MODULE_MSG( BXDM_CFG, ("%s", szMessage ));
      }

   }

   BDBG_LEAVE( BXDM_PPDF_S_PrintConfigInfo_isrsafe );

   return;

}


/*
 * Public API's.
 */

void BXDM_PictureProvider_Debug_PrintFifoEntry_isrsafe(
   BXDM_P_DebugFifo_Entry * pstEntry
   )
{
   BDBG_ENTER(BXDM_PictureProvider_Debug_PrintFifoEntry_isrsafe);

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

      case BXDM_DebugFifo_EntryType_eConfig:
         BXDM_PPDF_S_PrintConfigInfo_isrsafe( pstEntry );
         break;

      default:
         BDBG_MSG(("%s: unknown message type:%d.", BSTD_FUNCTION, pstEntry->stMetadata.eType ));
         break;

   }

   BDBG_LEAVE(BXDM_PictureProvider_Debug_PrintFifoEntry_isrsafe);

   return;

}


void BXDM_PictureProvider_Debug_DumpFifo_isrsafe(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BDBG_ENTER(BXDM_PictureProvider_Debug_DumpFifo_isrsafe);

   if ( NULL ==  hXdmPP->stDMConfig.stDebugReader.hDebugReader)
   {
      BDBG_ERR(("%s::  hXdmPP->stDMConfig.stDebugReader.hDebugReader== NULL", BSTD_FUNCTION ));
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

         BXDM_PictureProvider_Debug_PrintFifoEntry_isrsafe( pstEntry );

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
         break;
      }
   }

error:

   BDBG_LEAVE(BXDM_PictureProvider_Debug_DumpFifo_isrsafe);

   return;

}

#endif
