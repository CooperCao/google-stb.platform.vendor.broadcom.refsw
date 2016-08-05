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
#include "bxdm_pp_dbg_fifo_priv.h"
#include "bxdm_pp_qm.h"
#include "bxdm_pp_dbg_common.h"


BDBG_MODULE(BXDM_PPDBG_FIFO_PRIV);


#if BDBG_DEBUG_BUILD && BXDM_DEBUG_FIFO

extern uint32_t BXDM_PPTMR_lutVsyncsPersSecond[];

/* To be deleted. */

static void BXDM_PPDBG_S_AppendChar_isrsafe(
   BXDM_PPDBG_P_String * pstString,
   uint32_t uiCount,
   ...
   )
{
   uint32_t i;

   va_list args;

   va_start(args, uiCount);

   for( i=0; i < uiCount; i++ )
   {
      BXDM_PPDBG_P_APPEND_CHAR( pstString, va_arg(args, uint32_t) );
   }

   va_end(args);

   return;
}



/*
 * Functions
 */

BERR_Code BXDM_PPDFIFO_P_OutputLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState *pLocalState,
   const BAVC_MFD_Picture *pMFDPicture
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   uint32_t uiPPBIndex;
   char iInfoFlag;

   /* Gather Stats for this VSYNC */
   uiPPBIndex = hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.uiPPBIndex & 0xFFF;

   if (pMFDPicture->bMute)
   {
      iInfoFlag = 'M';
   }
   else if ((pMFDPicture->eInterruptPolarity != BAVC_Polarity_eFrame) &&
            (pMFDPicture->eSourcePolarity != BAVC_Polarity_eFrame) &&
            (pMFDPicture->eInterruptPolarity != pMFDPicture->eSourcePolarity) &&
            (BXDM_PictureProvider_P_InterruptType_eSingle == pLocalState->eInterruptType)
            )
   {
      iInfoFlag = '*';
   }
   else if (pMFDPicture->bPictureRepeatFlag)
   {
      iInfoFlag = 'R';
   }
   else
   {
      iInfoFlag = ' ';
   }

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( &(pDebugInfo->stInterruptString), 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( &(pDebugInfo->stInterruptString), 4, 'P', 'S', ':', ' ' );
   }

   BXDM_PPDBG_S_AppendChar_isrsafe(
         &(pDebugInfo->stInterruptString),
         8,
         BXDM_P_InterruptPolarityToStrLUT[pMFDPicture->eInterruptPolarity],
         BXDM_P_PicturePolaritytoStrLUT[pLocalState->eInterruptType][pMFDPicture->eSourcePolarity],
         ':',
         BXDM_P_HexToCharLUT[ (uiPPBIndex >> 8) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiPPBIndex >> 4) & 0xF ],
         BXDM_P_HexToCharLUT[ uiPPBIndex & 0xF ],
         iInfoFlag,
         ' ' );

   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDFIFO_P_OutputSPOLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiOverrideBits
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stSourcePolarityOverrideString;

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 4, 'O', 'V', ':', ' ' );
   }

   if ( uiOverrideBits )
   {
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         BXDM_P_HexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         BXDM_P_HexToCharLUT[ (uiOverrideBits >> 16) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiOverrideBits >> 12) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiOverrideBits >> 8) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiOverrideBits >> 4) & 0xF ],
         BXDM_P_HexToCharLUT[ uiOverrideBits & 0xF ],
         ' ' );
   }
   else
   {
      /* If none of the bits are set, print spaces to make it
       * more obvious when something is set.
       */
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         BXDM_P_HexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' );
   }

   if ( 0 != uiOverrideBits )
   {
      pDebugInfo->bPrintSPO = true;
   }

   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDFIFO_P_SelectionLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PPDBG_Selection eSelectionInfo
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stTSMString;

   if (pDebugInfo->abSelectionLogHeader[pDebugInfo->uiVsyncCount] == false)
   {
      if (pDebugInfo->uiVsyncCount == 0)
      {
         BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
         BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 4, 'T', 'M', ':', ' ' );
      }
      else
      {
         BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 1, ' ' );
      }

      BXDM_PPDBG_S_AppendChar_isrsafe(
            pStr,
            2,
            BXDM_P_HexToCharLUT[ pDebugInfo->uiVsyncCount & 0xF ],
            ':' );

      pDebugInfo->abSelectionLogHeader[pDebugInfo->uiVsyncCount] = true;
   }
   if ( BXDM_PPDBG_Selection_PolarityOverride_eSelectPrevious == eSelectionInfo
        || BXDM_PPDBG_Selection_PolarityOverride_eRepeatPrevious == eSelectionInfo
        || BXDM_PPDBG_Selection_PolarityOverride_eFICForceWait == eSelectionInfo
        || BXDM_PPDBG_Selection_PolarityOverride_e2ndSlotNextElement == eSelectionInfo )
   {
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 1, '(' );
   }

   BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 1, BXDM_P_PictureSelectionLUT[eSelectionInfo] );

   if ( BXDM_PPDBG_Selection_PolarityOverride_eSelectPrevious == eSelectionInfo
        || BXDM_PPDBG_Selection_PolarityOverride_eRepeatPrevious == eSelectionInfo
        || BXDM_PPDBG_Selection_PolarityOverride_eFICForceWait == eSelectionInfo
        || BXDM_PPDBG_Selection_PolarityOverride_e2ndSlotNextElement == eSelectionInfo )
   {
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 1, ')' );
   }


   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDFIFO_P_CallbackTriggeredLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiCallbackTriggeredBits
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stCallbackString;

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 4, 'C', 'B', ':', ' ' );
   }


   if ( uiCallbackTriggeredBits )
   {
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         BXDM_P_HexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         BXDM_P_HexToCharLUT[ (uiCallbackTriggeredBits >> 16) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiCallbackTriggeredBits >> 12) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiCallbackTriggeredBits >> 8) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiCallbackTriggeredBits >> 4) & 0xF ],
         BXDM_P_HexToCharLUT[ uiCallbackTriggeredBits & 0xF ],
         ' ' );
   }
   else
   {
      /* If none of the bits are set, print spaces to make
       * it more obvious when something is set.
       */
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         BXDM_P_HexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' );
   }

   if ( 0 != uiCallbackTriggeredBits )
   {
      pDebugInfo->bPrintCallbacks = true;
   }

   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDFIFO_P_StateLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiStateBits
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stStateString;

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 4, 'S', 'T', ':', ' ' );
   }

   BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         BXDM_P_HexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         BXDM_P_HexToCharLUT[ (uiStateBits >> 16) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiStateBits >> 12) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiStateBits >> 8) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiStateBits >> 4) & 0xF ],
         BXDM_P_HexToCharLUT[ uiStateBits & 0xF ],
         ' ' );

   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDFIFO_P_State2Log_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiStateBits
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stState2String;

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 3, 'S', '2', ':' );
   }


   if ( uiStateBits )
   {
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         BXDM_P_HexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         BXDM_P_HexToCharLUT[ (uiStateBits >> 16) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiStateBits >> 12) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiStateBits >> 8) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiStateBits >> 4) & 0xF ],
         BXDM_P_HexToCharLUT[ uiStateBits & 0xF ],
         ' ' );
   }
   else
   {
      /* If none of the bits are set, print spaces to make
       * it more obvious when something is set.
       */
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         BXDM_P_HexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' );
   }

   if ( 0 != uiStateBits )
   {
      pDebugInfo->bPrintState2 = true;
   }

   return BERR_SUCCESS;
}


BERR_Code BXDM_PPDFIFO_P_StcDeltaLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState * pLocalState
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stStcDeltaString;

   /*uint32_t uiStcDelta = pLocalState->uiStcSnapshot - hXdmPP->stDMState.stDecode.uiLastStcSnapshot;*/
   uint32_t uiStcDelta = pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM].uiWhole;

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 4, 'C', 'D', ':', ' ' );
   }

   BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         BXDM_P_HexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         BXDM_P_HexToCharLUT[ (uiStcDelta >> 16) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiStcDelta >> 12) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiStcDelta >> 8) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiStcDelta >> 4) & 0xF ],
         BXDM_P_HexToCharLUT[ uiStcDelta & 0xF ],
         ' ' );

   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDFIFO_P_DecoderDropLog_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiPendingDrop
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_PPDBG_P_String * pStr = &pDebugInfo->stPendingDropString;

   if (pDebugInfo->uiVsyncCount == 0)
   {
      BKNI_Memset( pStr, 0, sizeof( BXDM_PPDBG_P_String ) );
      BXDM_PPDBG_S_AppendChar_isrsafe( pStr, 4, 'D', 'R', ':', ' ' );
   }

   if ( uiPendingDrop )
   {
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         BXDM_P_HexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         BXDM_P_HexToCharLUT[ (uiPendingDrop >> 16) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiPendingDrop >> 12) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiPendingDrop >> 8) & 0xF ],
         BXDM_P_HexToCharLUT[ (uiPendingDrop >> 4) & 0xF ],
         BXDM_P_HexToCharLUT[ uiPendingDrop & 0xF ],
         ' ' );
   }
   else
   {
      /* If none of the bits are set, print spaces to make it
       * more obvious when something is set.
       */
      BXDM_PPDBG_S_AppendChar_isrsafe(
         pStr,
         8,
         BXDM_P_HexToCharLUT[ pDebugInfo->uiVsyncCount ],
         ':',
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' ,
         ' ' );
   }

   if ( 0 != uiPendingDrop )
   {
      pDebugInfo->bPrintDropCount = true;
   }

   return BERR_SUCCESS;
}

void BXDM_PPDFIFO_S_GetDefaultMetaData(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_P_DebugFifo_Entry *pstEntry
   )
{
   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstEntry );

   /* TODO: memset the entire structure? */

   BKNI_Memset( &pstEntry->stMetadata, 0, sizeof( BXDM_DebugFifo_Metadata ));
   pstEntry->stMetadata.uiInstanceID = hXdmPP->stDMConfig.uiInstanceID;
   pstEntry->stMetadata.uiVsyncCount = hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount;

   pstEntry->stMetadata.uiPPBIndex = -1;
   if ( NULL != pstPicture && true == pstPicture->bValidated )
   {
      pstEntry->stMetadata.uiPPBIndex = pstPicture->stPicParms.uiPPBIndex;
   }

   /*
   ( NULL != hVceOutput->hVce->hTimer ) ? BTMR_ReadTimer( hVceOutput->hVce->hTimer, &pstEntry->stMetadata.uiTimestamp ) : 0;
   */
   return;
}


/*
 * routines for writing the queue
 */
void BXDM_PPDFIFO_P_QueString_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_Debug_MsgType eMessageType,
   char * format,
   ...
   )
{
   BXDM_P_DebugFifo_Entry *pstEntry;
   BDBG_Fifo_Token stToken;
   BXDM_PictureProvider_P_DebugFifo * pstDebugFifo = &(hXdmPP->stDMConfig.stDebugFifo);

   BDBG_ENTER( BXDM_PPDFIFO_P_QueString_isr );

   BDBG_ASSERT( hXdmPP );

   if ( NULL == pstDebugFifo->hDebugFifo )
   {
      BDBG_MSG(("%s:: pstDebugFifo->hDebugFifo == NULL", __FUNCTION__ ));
      goto error;
   }

   pstEntry = (BXDM_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( pstDebugFifo->hDebugFifo, &stToken );

   if ( NULL == pstEntry )
   {
      BDBG_MSG(("%s:: pstEntry == NULL", __FUNCTION__ ));
      goto error;
   }

   {
      BXDM_PictureProvider_P_Picture_Context * pstPicture = NULL;
      va_list argList;


      BXDM_PPDFIFO_S_GetDefaultMetaData( hXdmPP, pstPicture, pstEntry );
      pstEntry->stMetadata.eType = BXDM_DebugFifo_EntryType_eString;

      pstEntry->data.stString.eType = ( eMessageType < BXDM_Debug_MsgType_eMax ) ? eMessageType : BXDM_Debug_MsgType_eUnKnown ;

      va_start( argList, format );
      BKNI_Vsnprintf(pstEntry->data.stString.szString, BXDM_P_MAX_DEBUG_FIFO_STRING_LEN, format, argList );
      va_end(argList);

      BDBG_Fifo_CommitBuffer( &stToken );
   }

error:

   BDBG_LEAVE( BXDM_PPDFIFO_P_QueString_isr );

   return;
}


BERR_Code BXDM_PPDFIFO_P_QueDBG_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState* pLocalState,
   bool bForcePrint
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;
   BXDM_P_DebugFifo_Entry *pstEntry;
   BDBG_Fifo_Token stToken;
   BXDM_PictureProvider_P_DebugFifo * pstDebugFifo = &(hXdmPP->stDMConfig.stDebugFifo);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pLocalState );

   /* See if we need to print out stats */
   if (pDebugInfo->uiVsyncCount == (BXDM_PPDBG_P_MAX_VSYNC_DEPTH - 1)
       || true == bForcePrint )
   {
      int32_t iAverageStcDelta;
      uint32_t uiAverageFractionBase10;

      BXDM_PictureProvider_MonitorRefreshRate eMonitorRefreshRate = hXdmPP->stDMConfig.eMonitorRefreshRate;

      if ( NULL == pstDebugFifo->hDebugFifo )
      {
         BDBG_MSG(("%s:: pstDebugFifo->hDebugFifo == NULL", __FUNCTION__ ));
         goto error;
      }

      pstEntry = (BXDM_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( pstDebugFifo->hDebugFifo, &stToken );

      if ( NULL == pstEntry )
      {
         BDBG_MSG(("%s:: pstEntry == NULL", __FUNCTION__ ));
         goto error;
      }

      if ( eMonitorRefreshRate >= BXDM_PictureProvider_MonitorRefreshRate_eMax )
      {
         eMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_eUnknown;
      }

      /* Figure out the STC delta (from the previous vsync) based on assorted state.
       */
      if ( true == hXdmPP->stDMConfig.stClockOverride.bEnableClockOverride )
      {
         iAverageStcDelta = hXdmPP->stDMConfig.stClockOverride.iStcDelta;
         uiAverageFractionBase10 = 0;
      }
      else if ( true == hXdmPP->stDMConfig.bSTCValid )
      {
         iAverageStcDelta = pLocalState->stDeltaSTCAvg.uiWhole;

         /* Convert the fractional part of stDeltaSTCAvg to a base 10 value for display. */
         BXDM_PPFP_P_FixPtBinaryFractionToBase10_isr(
                  (BXDM_PPFP_P_DataType *)&(pLocalState->stDeltaSTCAvg),
                  2,
                  &(uiAverageFractionBase10)
                  );
      }
      else
      {
         iAverageStcDelta = pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eTSM].uiWhole;
         uiAverageFractionBase10 = 0;
      }

      BXDM_PPDFIFO_S_GetDefaultMetaData( hXdmPP, NULL, pstEntry );
      pstEntry->stMetadata.eType = BXDM_DebugFifo_EntryType_eDebugInfo;

      pstEntry->data.stDebugInfo.uiStcSnapshot = pLocalState->uiStcSnapshot;
      pstEntry->data.stDebugInfo.iStcJitterCorrectionOffset = hXdmPP->stDMState.stDecode.iStcJitterCorrectionOffset;
      pstEntry->data.stDebugInfo.eMonitorRefreshRate = eMonitorRefreshRate;
      pstEntry->data.stDebugInfo.stSTCDelta = pLocalState->stSTCDelta;
      pstEntry->data.stDebugInfo.iAverageStcDelta = iAverageStcDelta;
      pstEntry->data.stDebugInfo.uiAverageFractionBase10 = uiAverageFractionBase10;
      pstEntry->data.stDebugInfo.uiSlowMotionRate = pLocalState->uiSlowMotionRate;
      pstEntry->data.stDebugInfo.eSTCTrickMode = pLocalState->eSTCTrickMode;
      pstEntry->data.stDebugInfo.bPlayback = hXdmPP->stDMConfig.bPlayback;

      BDBG_Fifo_CommitBuffer( &stToken );

      /* Print TSM Logs */
      BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, "%s", (char *)&pDebugInfo->stTSMString.szDebugStr );

      /* Print Decoder Drop Logs */
      if ( true ==  pDebugInfo->bPrintDropCount )
      {
         BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, "%s", (char *)&pDebugInfo->stPendingDropString.szDebugStr );
      }

      /* Print picture selecton log */
      BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, "%s", (char *)&pDebugInfo->stInterruptString.szDebugStr );

      /* Print stats for Source Polarity Override */
      if ( true == pDebugInfo->bPrintSPO )
      {
         BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, "%s", (char *)&pDebugInfo->stSourcePolarityOverrideString.szDebugStr );
      }

      /* Print stats for Callbacks */
      if ( true == pDebugInfo->bPrintCallbacks )
      {
         BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, "%s", (char *)&pDebugInfo->stCallbackString.szDebugStr );
      }

      /* Print State Logs: */
      BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, "%s", (char *)&pDebugInfo->stStateString.szDebugStr );

      /* Print stats for state 2 */
      if ( true == pDebugInfo->bPrintState2 )
      {
         BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG2, "%s", (char *)&pDebugInfo->stState2String.szDebugStr );
      }

      BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBGC, "%s", (char *)&pDebugInfo->stStcDeltaString.szDebugStr );

      BKNI_Memset(pDebugInfo, 0, sizeof(BXDM_PPDBG_P_Info));

   }
   else
   {
      pDebugInfo->uiVsyncCount++;
   }

error:

   return BERR_SUCCESS;

}

void BXDM_PPDFIFO_P_QueMFD_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BAVC_MFD_Picture* pMFDPicture
   )
{
   BXDM_P_DebugFifo_Entry *pstEntry;
   BDBG_Fifo_Token stToken;
   BXDM_PictureProvider_P_DebugFifo * pstDebugFifo = &(hXdmPP->stDMConfig.stDebugFifo);

   BDBG_ENTER( BXDM_PPDFIFO_P_QueMFD_isr );

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pLocalState );
   BDBG_ASSERT( pMFDPicture );

   if ( NULL == pstDebugFifo->hDebugFifo )
   {
      BDBG_MSG(("%s:: pstDebugFifo->hDebugFifo == NULL", __FUNCTION__ ));
      goto error;
   }

   while ( NULL != pMFDPicture )
   {
      pstEntry = (BXDM_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( pstDebugFifo->hDebugFifo, &stToken );

      if ( NULL == pstEntry )
      {
         BDBG_MSG(("%s:: pstEntry == NULL", __FUNCTION__ ));
         goto error;
      }

      BXDM_PPDFIFO_S_GetDefaultMetaData( hXdmPP, &hXdmPP->stDMState.stChannel.stSelectedPicture, pstEntry );
      pstEntry->stMetadata.eType = BXDM_DebugFifo_EntryType_eMFD;

      pstEntry->data.stMFD = *pMFDPicture;
      BDBG_Fifo_CommitBuffer( &stToken );

      pMFDPicture = pMFDPicture->pEnhanced;
   }

error:

   BDBG_LEAVE( BXDM_PPDFIFO_P_QueMFD_isr );

   return;

}

/*
 * Routines for dealing with Unified Pictures.
 */

void BXDM_PPDFIFO_P_QueUnifiedPicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicture
   )
{
   BXDM_PictureProvider_P_Picture_Context *  pstSelectedPicture;
   const BXDM_Picture * pstUnified;
   const BXDM_Picture * pstSelectedUnified;

   bool  bPrintAdditional;

   BXDM_P_DebugFifo_Entry *pstEntry;
   BDBG_Fifo_Token stToken;
   BXDM_PictureProvider_P_DebugFifo * pstDebugFifo = &(hXdmPP->stDMConfig.stDebugFifo);

   BSTD_UNUSED(pLocalState);

   BDBG_ENTER( BXDM_PPDFIFO_P_QueUnifiedPicture_isr );

   pstSelectedPicture = &(hXdmPP->stDMState.stChannel.stSelectedPicture);
   pstSelectedUnified = pstSelectedPicture->pstUnifiedPicture;

   pstUnified = pstPicture->pstUnifiedPicture;

   /* SW7405-4736: now that this routine is called from BXDM_PP_S_SelectPicture, there is a
    * need to verify that the unified picture is still available.  In the scenario where a
    * picture is being redisplayed while subsequent ones are dropped, "pstSelectedUnified"
    * will be valid and "pstUnified" can be NULL.  This can occur when playing a DVD clip
    * where video data continues after the clip stop time.  The picture just prior to the
    * clip stop time will be repeated, the pictures after the clip stop time will be dropped.
    */
   if ( NULL == pstUnified )
   {
      goto Done;
   }

   if ( NULL == pstDebugFifo->hDebugFifo )
   {
      BDBG_MSG(("%s:: pstDebugFifo->hDebugFifo == NULL", __FUNCTION__ ));
      goto Done;
   }

   pstEntry = (BXDM_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( pstDebugFifo->hDebugFifo, &stToken );

   if ( NULL == pstEntry )
   {
      BDBG_MSG(("%s:: pstEntry == NULL", __FUNCTION__ ));
      goto Done;
   }

   {
      BXDM_DebugFifo_UnifiedPicture * pUniP = (BXDM_DebugFifo_UnifiedPicture *)&pstEntry->data.stUniPic;

      BXDM_PPDFIFO_S_GetDefaultMetaData( hXdmPP, pstPicture, pstEntry );
      pstEntry->stMetadata.eType = BXDM_DebugFifo_EntryType_eUnifiedPicture;

      pUniP->stUnifiedPicture = *pstUnified;

      /* Print once per second or when any of the parameters change. */

      bPrintAdditional = ( hXdmPP->stDMState.stDecode.uiVsyncCountQM >= BXDM_PPTMR_lutVsyncsPersSecond[ hXdmPP->stDMConfig.eMonitorRefreshRate ] );

      if ( pstSelectedUnified && true == pstSelectedPicture->bValidated )
      {
         bPrintAdditional |= ( pstUnified->stBufferInfo.stSource.uiWidth != pstSelectedUnified->stBufferInfo.stSource.uiWidth ) ;
         bPrintAdditional |= ( pstUnified->stBufferInfo.stSource.uiHeight != pstSelectedUnified->stBufferInfo.stSource.uiHeight ) ;
         bPrintAdditional |= ( pstUnified->stProtocol.eProtocol != pstSelectedUnified->stProtocol.eProtocol ) ;
         bPrintAdditional |= ( pstUnified->stBufferInfo.eBufferFormat != pstSelectedUnified->stBufferInfo.eBufferFormat ) ;
         bPrintAdditional |= ( pstUnified->stFrameRate.stRate.uiNumerator != pstSelectedUnified->stFrameRate.stRate.uiNumerator ) ;
         bPrintAdditional |= ( pstUnified->stFrameRate.stRate.uiDenominator != pstSelectedUnified->stFrameRate.stRate.uiDenominator ) ;
         bPrintAdditional |= ( pstPicture->stPicParms.stTSM.stStatic.eFrameRateXVD != pstSelectedPicture->stPicParms.stTSM.stStatic.eFrameRateXVD ) ;
         bPrintAdditional |= ( pstUnified->stAspectRatio.eAspectRatio != pstSelectedUnified->stAspectRatio.eAspectRatio ) ;
         bPrintAdditional |= ( pstUnified->stAspectRatio.uiSampleAspectRatioX != pstSelectedUnified->stAspectRatio.uiSampleAspectRatioX ) ;
         bPrintAdditional |= ( pstUnified->stAspectRatio.uiSampleAspectRatioY != pstSelectedUnified->stAspectRatio.uiSampleAspectRatioY ) ;
         bPrintAdditional |= ( hXdmPP->stDMStatus.stCurrentPTS.uiSwPcrOffset != hXdmPP->stDMConfig.uiSoftwarePCROffset );
         bPrintAdditional |= ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_DISPLAY_FIELD_MODE );
      }
      else
      {
         /* Results in printing the parameters for the first picture after start decode. */
         bPrintAdditional = true;
      }

      pUniP->bPrintAdditional = bPrintAdditional;

      if ( true == bPrintAdditional )
      {
         if ( 0 !=  hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount )
         {
            /* determine Average dPTS (using FRD parameters) ... */
            BXDM_PPFP_P_FixPtDiv_isr(
                  &hXdmPP->stDMState.stDecode.stFRDStats.stDeltaPTSRunningSum,
                  hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount,
                  &pUniP->stDeltaPTSAvg
                  );

            /* Convert the fractional part of stDeltaPTSAvg to a base 10 value for display. */
            BXDM_PPFP_P_FixPtBinaryFractionToBase10_isr( &pUniP->stDeltaPTSAvg, 2, &pUniP->uiAverageFractionBase10 );
         }
         else
         {
            pUniP->stDeltaPTSAvg.uiFractional = 0;
            pUniP->stDeltaPTSAvg.uiWhole = 0;
            pUniP->uiAverageFractionBase10 = 0;
         }

         pUniP->eFrameRate = pstPicture->stPicParms.stTSM.stStatic.eFrameRateXVD;
         pUniP->eFrameRateType = pstPicture->stPicParms.stTSM.stStatic.eFrameRateType;
         pUniP->uiSwPcrOffsetUsedForEvaluation = pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation;
         pUniP->eDisplayFieldMode = hXdmPP->stDMConfig.eDisplayFieldMode;
         pUniP->ePictureDisplayFieldMode = pstPicture->stPicParms.stDisplay.stDynamic.eDisplayFieldMode;
         pUniP->bForceSingleFieldMode = pstPicture->stPicParms.stDisplay.stDynamic.bForceSingleFieldMode;

         hXdmPP->stDMState.stDecode.uiVsyncCountQM = 0;

      }     /* end of if ( bPrintAdditional )*/


      /* SW7445-586: use accessor method to retrieve pulldown */
      BXDM_PPQM_P_GetPicturePulldown_isr( pstPicture, &pUniP->ePulldown );

      if ( pUniP->ePulldown >= BXDM_Picture_PullDown_eMax ) pUniP->ePulldown = 0;

      switch ( pstPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode )
      {
         case BXDM_PictureProvider_PictureHandlingMode_eHonorPTS:    pUniP->cSelectionMode = 'h';   break;
         case BXDM_PictureProvider_PictureHandlingMode_eIgnorePTS:   pUniP->cSelectionMode = 'i';   break;
         case BXDM_PictureProvider_PictureHandlingMode_eDrop:        pUniP->cSelectionMode = 'd';   break;
         case BXDM_PictureProvider_PictureHandlingMode_eWait:
            pUniP->cSelectionMode = (BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode) ? 'w' : 'v' ;
            break;

         case BXDM_PictureProvider_PictureHandlingMode_eDefault:
         default:
            if ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode )
            {
               pUniP->cSelectionMode = ( pstPicture->stPicParms.stTSM.stDynamic.bEvaluatedWithSwStc ) ? 's' : 't';
            }
            else
            {
               pUniP->cSelectionMode = ( pstPicture->stPicParms.stTSM.stDynamic.bSelectedInPrerollMode ) ? 'p' : 'v' ;
            }
            break;
      }

      /* A picture-less picture, it will simply be dropped. */
      if ( false == pstPicture->pstUnifiedPicture->stBufferInfo.bValid )
      {
         pUniP->cSelectionMode = 'x';
      }

      /* SWSTB-439: if the error threshold is non-zero, use "uiPercentError" from the picture structure
       * to determine if the picture has an error.  Otherwise use the error flag. */

      if ( 0 != hXdmPP->stDMConfig.uiErrorThreshold )
      {
         pUniP->cErrorOnThisPicture = ( pstUnified->stError.uiPercentError == 0 ) ? ' ' :
                                       ( pstUnified->stError.uiPercentError >= hXdmPP->stDMConfig.uiErrorThreshold ) ? 'e' : 'p';
      }
      else
      {
         pUniP->cErrorOnThisPicture = ( pstUnified->stError.bThisPicture == true )  ? 'E' : ' ';
      }


      pUniP->eSelectionMode = pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode;
      pUniP->eTsmResult = pstPicture->stPicParms.stTSM.stDynamic.eTsmResult;
      pUniP->bAppendedToPreviousPicture = pstPicture->stPicParms.stDisplay.stDynamic.bAppendedToPreviousPicture;

      BXDM_PPQM_P_GetPtsOffset_isr( hXdmPP, pstPicture, &pUniP->uiDisplayOffset );

      pUniP->iStcPtsDifferenceActual = pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceActual;


      if ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode )
      {
         BXDM_PPQM_P_GetHwPcrOffset_isr( hXdmPP, pstPicture, &pUniP->uiPcrOffset );

         pUniP->uiPcrOffset += pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation;


         if (( false == hXdmPP->stDMConfig.bUseHardwarePCROffset )
               || ( true == hXdmPP->stDMConfig.bPlayback))
         {
            pUniP->bPcrOffsetValid = true;
            pUniP->bPcrOffsetDiscontinuity = false;
         }
         else
         {
            pUniP->bPcrOffsetValid = pstPicture->stPicParms.stTSM.stStatic.stPCROffsetXDM.bValidOverloaded;
            pUniP->bPcrOffsetDiscontinuity = pstPicture->stPicParms.stTSM.stStatic.stPCROffsetXDM.bDiscontinuityOverloaded;
         }

         pUniP->iPTSJitterCorrection = pstPicture->stPicParms.stTSM.stStatic.iPTSJitterCorrection;
         pUniP->uiStcSnapshot = pLocalState->uiStcSnapshot;
         pUniP->iStcJitterCorrectionOffset = hXdmPP->stDMState.stDecode.iStcJitterCorrectionOffset;

      }
      else
      {

         BXDM_PPVTSM_P_VirtualStcGet_isr( hXdmPP, &pUniP->uiVirtualStc );

         pUniP->uiVirtualStc += pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation;

         BXDM_PPQM_P_GetPtsWithFrac_isr(
               pstPicture,
               BXDM_PictureProvider_P_PTSIndex_eVirtual,
               pstPicture->stPicParms.stDisplay.stDynamic.uiSelectedElement,
               &pUniP->stVirtualPts
               );
      }

      BDBG_Fifo_CommitBuffer( &stToken );

   }

Done:

   BDBG_LEAVE( BXDM_PPDFIFO_P_QueUnifiedPicture_isr );

   return;

}  /* end of BXDM_PPDFIFO_P_QueUnifiedPicture_isr */



void BXDM_PPDFIFO_P_QueDMConfig_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   bool bLastCall
   )
{
   BDBG_ENTER( BXDM_PPDFIFO_P_QueDMConfig_isr );

   BSTD_UNUSED(pLocalState);
   BSTD_UNUSED(hXdmPP);
   BSTD_UNUSED(bLastCall);

   /* Print when any of the parameters change. */

   if (  hXdmPP->stDMConfig.uiDirtyBits_1 || hXdmPP->stDMConfig.uiDirtyBits_2 )
   {
      uint32_t uiDirtyBitsGroup_1_1, uiDirtyBitsGroup_1_2, uiDirtyBitsGroup_1_3, uiDirtyBitsGroup_1_4;
      uint32_t uiDirtyBitsGroup_2_1, uiDirtyBitsGroup_2_2, uiDirtyBitsGroup_2_3, uiDirtyBitsGroup_2_4 ;

      uiDirtyBitsGroup_1_1 = uiDirtyBitsGroup_1_2 = uiDirtyBitsGroup_1_3 = uiDirtyBitsGroup_1_4 = hXdmPP->stDMConfig.uiDirtyBits_1;

      uiDirtyBitsGroup_2_1 = uiDirtyBitsGroup_2_2 = uiDirtyBitsGroup_2_3 = uiDirtyBitsGroup_2_4 = hXdmPP->stDMConfig.uiDirtyBits_2;

      uiDirtyBitsGroup_1_1 &= BXDM_PictureProvider_P_DIRTY_1_STC_VALID
                              | BXDM_PictureProvider_P_DIRTY_1_PTS_OFFSET
                              | BXDM_PictureProvider_P_DIRTY_1_SOFTWARE_PCR_OFFSET
                              | BXDM_PictureProvider_P_DIRTY_1_USE_HW_PCR_OFFSET
                              | BXDM_PictureProvider_P_DIRTY_1_PLAYBACK_RATE
                              | BXDM_PictureProvider_P_DIRTY_1_DISPLAY_FIELD_MODE;

      uiDirtyBitsGroup_2_1 &= BXDM_PictureProvider_P_DIRTY_2_MUTE;

      if ( uiDirtyBitsGroup_1_1 || uiDirtyBitsGroup_2_1 )
      {

         BXDM_PPDFIFO_P_QueString_isr(
            hXdmPP,
            BXDM_Debug_MsgType_eCFG,
            "%c%x:[%02x.xxx]1:%c stc:%c(%d) PTSoff:%08x(%d) STCoff:%08x(%d) usePCR:%c(%d) pbr:%d/%d(%d) dfm:%s(%d)",
            ( bLastCall ) ? '*' : ' ',
            hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
            BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
            ( hXdmPP->stDMConfig.bMute == true ) ? 'M' : ' ',
/*                  ( hXdmPP->stDMConfig.uiDirtyBits_2 & BXDM_PictureProvider_P_DIRTY_2_MUTE ) ? 1 : 0,*/
            ( hXdmPP->stDMConfig.bSTCValid == true ) ? 'v' : 'i',
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_STC_VALID ) ? 1 : 0,
            hXdmPP->stDMConfig.uiPTSOffset,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_PTS_OFFSET ) ? 1 : 0,
            hXdmPP->stDMConfig.uiSoftwarePCROffset,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_SOFTWARE_PCR_OFFSET ) ? 1 : 0,
            ( hXdmPP->stDMConfig.bUseHardwarePCROffset == true) ? 't' : 'f',
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_USE_HW_PCR_OFFSET ) ? 1 : 0,
            hXdmPP->stDMConfig.stPlaybackRate.uiNumerator,
            hXdmPP->stDMConfig.stPlaybackRate.uiDenominator,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_PLAYBACK_RATE ) ? 1 : 0,
            BXDM_P_DisplayFieldModeToStrLUT[ hXdmPP->stDMConfig.eDisplayFieldMode ],
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_DISPLAY_FIELD_MODE ) ? 1 : 0
         );

      }     /* end of  if ( uiDirtyBitsGroup_1_1 || uiDirtyBitsGroup_2_1 ) */


      uiDirtyBitsGroup_1_2 &= BXDM_PictureProvider_P_DIRTY_1_PLAYBACK
                              | BXDM_PictureProvider_P_DIRTY_1_PROTOCOL
                              | BXDM_PictureProvider_P_DIRTY_1_MONITOR_REFRESH_RATE
                              | BXDM_PictureProvider_P_DIRTY_1_DEFAULT_FRAME_RATE
                              | BXDM_PictureProvider_P_DIRTY_1_FRAMERATE_OVERRIDE
                              | BXDM_PictureProvider_P_DIRTY_1_TRICK_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_FREEZE ;

      uiDirtyBitsGroup_2_2 &= BXDM_PictureProvider_P_DIRTY_2_DISPLAY_MODE;

      if ( uiDirtyBitsGroup_1_2 || uiDirtyBitsGroup_2_2 )
      {
         BXDM_PPDFIFO_P_QueString_isr(
            hXdmPP,
            BXDM_Debug_MsgType_eCFG,
            "%c%x:[%02x.xxx]2:%s(%d) %s(%d) %s(%d) mr:%s(%d) dfr:%s(%d) fro:%d/%d(%d,%d)(%d) tm:%s(%d) frz:%c(%d)",
            ( bLastCall ) ? '*' : ' ',
            hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
            BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
            ( hXdmPP->stDMConfig.eDisplayMode == BXDM_PictureProvider_DisplayMode_eTSM ) ? "TSM" : "vsync",
            ( hXdmPP->stDMConfig.uiDirtyBits_2 & BXDM_PictureProvider_P_DIRTY_2_DISPLAY_MODE ) ? 1 : 0,
            ( hXdmPP->stDMConfig.bPlayback == true ) ? "pb" : "lv",
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_PLAYBACK ) ? 1 : 0,
            BXDM_P_VideoCompressionStdToStrLUT[ hXdmPP->stDMConfig.eProtocol ],
            ( hXdmPP->stDMConfig.uiDirtyBits_2 & BXDM_PictureProvider_P_DIRTY_1_PROTOCOL ) ? 1 : 0,
            BXDM_P_MonitorRefreshRateToStrLUT[ hXdmPP->stDMConfig.eMonitorRefreshRate ],
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_MONITOR_REFRESH_RATE ) ? 1 : 0,
            BXDM_P_BAVCFrameRateToStrLUT[ hXdmPP->stDMConfig.eDefaultFrameRate ],
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_DEFAULT_FRAME_RATE ) ? 1 : 0,
            hXdmPP->stDMConfig.stFrameRateOverride.stRate.uiNumerator,
            hXdmPP->stDMConfig.stFrameRateOverride.stRate.uiDenominator,
            hXdmPP->stDMConfig.stFrameRateOverride.bValid,
            hXdmPP->stDMConfig.stFrameRateOverride.bTreatAsSingleElement,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_FRAMERATE_OVERRIDE ) ? 1 : 0,
            BXDM_P_TrickModeToStrLUT[ hXdmPP->stDMConfig.eTrickMode ],
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_TRICK_MODE ) ? 1 : 0,
            ( hXdmPP->stDMConfig.bFreeze == true ) ? 't' : 'f',
            ( hXdmPP->stDMConfig.uiDirtyBits_2 & BXDM_PictureProvider_P_DIRTY_1_FREEZE ) ? 1 : 0
         );

      } /* end of if ( uiDirtyBitsGroup_1_2 || uiDirtyBitsGroup_2_2 ) */


      uiDirtyBitsGroup_1_3 &= BXDM_PictureProvider_P_DIRTY_1_SRC_FORMAT_OVERRIDE
                              | BXDM_PictureProvider_P_DIRTY_1_SCAN_MODE_OVERRIDE
                              | BXDM_PictureProvider_P_DIRTY_1_1080P_SCAN_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_PICTURE_DROP_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_480P_PULLDOWN_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_1080P_PULLDOWN_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_240I_SCAN_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_HORIZON_OVERSCAN_MODE;


      if ( uiDirtyBitsGroup_1_3 )
      {
         char cSFO, cSMO;

         switch( hXdmPP->stDMConfig.eSourceFormatOverride )
         {
            case BXDM_PictureProvider_SourceFormatOverride_eInterlaced:    cSFO = 'P';    break;
            case BXDM_PictureProvider_SourceFormatOverride_eProgressive:   cSFO = 'I';    break;
            case BXDM_PictureProvider_SourceFormatOverride_eDefault:
            default:                                                       cSFO = 'D';    break;
         }

         switch( hXdmPP->stDMConfig.eScanModeOverride )
         {
            case BXDM_PictureProvider_ScanModeOverride_eInterlaced:     cSMO = 'P';    break;
            case BXDM_PictureProvider_ScanModeOverride_eProgressive:    cSMO = 'I';    break;
            case BXDM_PictureProvider_ScanModeOverride_eDefault:
            default:                                                    cSMO = 'D';    break;
         }

         BXDM_PPDFIFO_P_QueString_isr(
            hXdmPP,
            BXDM_Debug_MsgType_eCFG,
            "%c%x:[%02x.xxx]3:sfo:%c(%d) smo:%c(%d) 1080sm:%c(%d) pdm:%s(%d) 480pdm:%s(%d) 1080pdm:%s(%d) 240sm:%s(%d) hom:%s(%d)",
            ( bLastCall ) ? '*' : ' ',
            hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
            BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
            cSFO,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_SRC_FORMAT_OVERRIDE ) ? 1 : 0,
            cSMO,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_SCAN_MODE_OVERRIDE ) ? 1 : 0,
            ( hXdmPP->stDMConfig.e1080pScanMode == BXDM_PictureProvider_1080pScanMode_eDefault ) ? 'D' : 'A' ,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_1080P_SCAN_MODE ) ? 1 : 0,
            ( hXdmPP->stDMConfig.ePictureDropMode == BXDM_PictureProvider_PictureDropMode_eField ) ? "fld" : "Frm" ,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_PICTURE_DROP_MODE ) ? 1 : 0,
            BXDM_P_PulldownModeToStrLUT[ hXdmPP->stDMConfig.e480pPulldownMode ],
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_480P_PULLDOWN_MODE ) ? 1 : 0,
            BXDM_P_PulldownModeToStrLUT[ hXdmPP->stDMConfig.e1080pPulldownMode ],
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_1080P_PULLDOWN_MODE ) ? 1 : 0,
            ( hXdmPP->stDMConfig.e240iScanMode == BXDM_PictureProvider_240iScanMode_eForceProgressive ) ? "fp" : "en" ,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_240I_SCAN_MODE ) ? 1 : 0,
            ( hXdmPP->stDMConfig.eHorizontalOverscanMode == BXDM_PictureProvider_HorizontalOverscanMode_eAuto ) ? "auto" : "dis" ,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_HORIZON_OVERSCAN_MODE ) ? 1 : 0
         );

      }     /* end of if ( uiDirtyBitsGroup_1_3 ) */


      uiDirtyBitsGroup_1_4 &= BXDM_PictureProvider_P_DIRTY_1_FRAME_ADVANCE_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_REVERSE_FIELDS
                              | BXDM_PictureProvider_P_DIRTY_1_AUTO_VALIDATE_ON_PAUSE
                              | BXDM_PictureProvider_P_DIRTY_1_3D_SETTINGS
                              | BXDM_PictureProvider_P_DIRTY_1_JITTER_TOLERANCE
                              | BXDM_PictureProvider_P_DIRTY_1_FRAME_RATE_DETECTION_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_ERROR_HANDLING_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_ATSM_MODE
                              | BXDM_PictureProvider_P_DIRTY_1_VTSM_ON_PCR_DISCON ;


      uiDirtyBitsGroup_2_4 &= BXDM_PictureProvider_P_DIRTY_2_ERROR_THRESHOLD;

      if ( uiDirtyBitsGroup_1_4 || uiDirtyBitsGroup_2_4 )
      {
         BXDM_PPDFIFO_P_QueString_isr(
            hXdmPP,
            BXDM_Debug_MsgType_eCFG,
            "%c%x:[%02x.xxx]4:fra:%s(%d) rvf:%c(%d) avop:%c(%d) 3Do:%s(%d)(%d) jti:%s(%d) frd:%s(%d) ehm:%s(%d) el:%d(%d) astm:%c(%d) avsync:%c(%d) ",
            ( bLastCall ) ? '*' : ' ',
            hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
            BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
            BXDM_P_FrameAdvanceModeToStrLUT[hXdmPP->stDMConfig.eFrameAdvanceMode],
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_FRAME_ADVANCE_MODE ) ? 1 : 0,
            ( hXdmPP->stDMConfig.bReverseFields == true ) ? 't' : 'f' ,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_REVERSE_FIELDS ) ? 1 : 0,
            ( hXdmPP->stDMConfig.bAutoValidateStcOnPause == true ) ? 't' : 'f' ,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_AUTO_VALIDATE_ON_PAUSE ) ? 1 : 0,
            BXDM_P_PPOrientationToStrLUT[ hXdmPP->stDMConfig.st3DSettings.eOrientation ],
            hXdmPP->stDMConfig.st3DSettings.bOverrideOrientation,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_3D_SETTINGS ) ? 1 : 0,
            ( hXdmPP->stDMConfig.bJitterToleranceImprovement == true ) ? "on" : "off" ,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_JITTER_TOLERANCE ) ? 1 : 0,
            BXDM_P_FrameRateDetectionModeToStrLUT[ hXdmPP->stDMConfig.eFrameRateDetectionMode ],
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_FRAME_RATE_DETECTION_MODE ) ? 1 : 0,
            BXDM_P_ErrorHandlingModeToStrLUT[ hXdmPP->stDMConfig.eErrorHandlingMode ],
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_ERROR_HANDLING_MODE ) ? 1 : 0,
            hXdmPP->stDMConfig.uiErrorThreshold,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_2_ERROR_THRESHOLD ) ? 1 : 0,
            ( hXdmPP->stDMConfig.bAstmMode == true ) ? 't' : 'f' ,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_ATSM_MODE ) ? 1 : 0,
            ( hXdmPP->stDMConfig.bVirtualTSMOnPCRDiscontinuity == true ) ? 't' : 'f' ,
            ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_VTSM_ON_PCR_DISCON ) ? 1 : 0
         );

      }     /* end of if ( uiDirtyBitsGroup_1_4 || uiDirtyBitsGroup_2_4 ) */

      /* The following are not currently printed.
       *
       * uint32_t uiRemovalDelay;
       * BXDM_Picture_Rate stPreRollRate;
       * uint32_t uiMaxHeightSupportedByDeinterlacer;
       * BXDM_PictureProvider_TSMThresholdSettings stTSMThresholdSettings;
       * uint32_t uiSTCIndex;
       * BXDM_PictureProvider_ClipTimeSettings stClipTimeSettings;
       */

      hXdmPP->stDMConfig.uiDirtyBits_1 = BXDM_PictureProvider_P_DIRTY_NONE;
      hXdmPP->stDMConfig.uiDirtyBits_2 = BXDM_PictureProvider_P_DIRTY_NONE;

   }       /* end of  if ( hXdmPP->stDMConfig.uiDirtyBits_1 || hXdmPP->stDMConfig.uiDirtyBits_2 ) */

   BDBG_LEAVE( BXDM_PPDFIFO_P_QueDMConfig_isr );

   return;

}  /* end of BXDM_PPDFIFO_P_PrintDMConfig */

void BXDM_PPDFIFO_P_QueStartDecode_isr(
   const BXDM_PictureProvider_Handle hXdmPP
   )
{
   /* SW7445-1259: reduce messages printed at start decode time */
   BXDM_PictureProvider_P_State_Decode *pDecode = &hXdmPP->stDMState.stDecode;
   BXDM_PPDBG_P_Info *pDebug = &pDecode->stDebug;

   BXDM_PPDFIFO_P_QueString_isr(
            hXdmPP,
            BXDM_Debug_MsgType_eDBG,
            "--- %x:[%02x.xxx] BXDM_PictureProvider_StartDecode_isr has been called ---",
            pDebug->uiVsyncCount,
            BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP )
            );
   return;
}

void BXDM_PPDFIFO_P_QueStopDecode_isr(
   const BXDM_PictureProvider_Handle hXdmPP
   )
{
   BXDM_PictureProvider_P_State_Decode *pDecode = &hXdmPP->stDMState.stDecode;
   BXDM_PPDBG_P_Info *pDebug = &pDecode->stDebug;

   BXDM_PPDFIFO_P_QueString_isr(
            hXdmPP,
            BXDM_Debug_MsgType_eDBG,
            "--- %x:[%02x.xxx] BXDM_PictureProvider_StopDecode_isr has been called ---",
            pDebug->uiVsyncCount,
            BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP )
            );

   return;
}

/*
** SW7335-781: Output warning when forcing picture selection override
** Output Selection Mode override message only once, if:
** an override to vTSM mode occurred
** the override was not the same as the previous Override
** function assumes new selection mode is VSYNC (vTSM) mode
*/
void BXDM_PPDFIFO_P_PrintSelectionModeOverride_isr(
   char *pMsg,
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPicture)
{
   BXDM_PictureProvider_P_State_Decode *pDecode = &hXdmPP->stDMState.stDecode;
   BXDM_PPDBG_P_Info *pDebug = &pDecode->stDebug;

   if ((BXDM_PictureProvider_DisplayMode_eVirtualTSM != pPicture->stPicParms.stTSM.stDynamic.eSelectionMode)
     && (BXDM_PictureProvider_DisplayMode_eVirtualTSM != pDecode->eLastSelectionModeOverride))
   {
      BXVD_DBG_WRN(hXdmPP, (" %x:[%02x.%03x] %s: Selection Mode Override (TSM -> vTSM)",
                              pDebug->uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pPicture->stPicParms.uiPPBIndex & 0xFFF,
                              pMsg));

      /* prevent repeated override messages */
      pDecode->eLastSelectionModeOverride = BXDM_PictureProvider_DisplayMode_eVirtualTSM;
   }
}

/*
** SW7335-781: Output warning when forcing picture selection override
** Indicate when the override transitions back to TSM mode
*/
void BXDM_PPDFIFO_P_PrintEndSelectionModeOverride_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPicture)
{
   BXDM_PictureProvider_P_State_Decode *pDecode = &hXdmPP->stDMState.stDecode;
   BXDM_PPDBG_P_Info *pDebug = &pDecode->stDebug;

   /* if selected picture's selection mode was TSM and last selection mode override was VSYNC
      then output the selection mode change message */
   if ((BXDM_PictureProvider_DisplayMode_eTSM == pPicture->stPicParms.stTSM.stDynamic.eSelectionMode)
      && (BXDM_PictureProvider_DisplayMode_eVirtualTSM == pDecode->eLastSelectionModeOverride))
   {
      BXVD_DBG_WRN(hXdmPP, (" %x:[%02x.%03x] Selection Mode Override: Return to TSM",
                                 pDebug->uiVsyncCount,
                                 BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                 pPicture->stPicParms.uiPPBIndex & 0xFFF
                                 ));

      pDecode->eLastSelectionModeOverride = BXDM_PictureProvider_DisplayMode_eTSM;
   }
}



/*
 * SWSTB-1380: routines for managing the debug fifo's
 */

BERR_Code BXDM_PPDFIFO_P_Fifo_Create(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BXDM_PictureProvider_P_DebugFifo * pstDebugFifo = &(hXdmPP->stDMConfig.stDebugFifo);

   BERR_Code rc = BERR_SUCCESS;

   bool bAllocationError = false;

   BDBG_Fifo_CreateSettings stCreateSettings;

   BDBG_ENTER( BXDM_PPDFIFO_P_Fifo_Create );

   pstDebugFifo->uiElementSize = sizeof( BXDM_P_DebugFifo_Entry );
   pstDebugFifo->uiFifoSize = BXDM_P_MAX_DEBUG_FIFO_COUNT * sizeof( BXDM_P_DebugFifo_Entry );

   /*
    * Allocate the buffer for the debug fifo.
    */
   pstDebugFifo->hBMMABlock = BMMA_Alloc(
      pstDebugFifo->hBMMAHeap,
      pstDebugFifo->uiFifoSize,
      1024,                               /* what was done for VCE, i.e. BVCE_P_DEFAULT_ALIGNMENT */
      NULL
      );

   if ( NULL == pstDebugFifo->hBMMABlock )
   {
      BDBG_ERR(("%s:: BMMA_Alloc failure", __FUNCTION__ ));
      bAllocationError = true;
      rc = BERR_OUT_OF_DEVICE_MEMORY;
      goto error;
   }

   /*
    * Get the physical address.
    */
   pstDebugFifo->bmmaOffset = BMMA_LockOffset( pstDebugFifo->hBMMABlock );

   /* will this be needed?
     * hBuffer->bOffsetValid = true; */

   if ( 0 == pstDebugFifo->bmmaOffset )
   {
      BDBG_ERR(("%s:: BMMA_LockOffset failure", __FUNCTION__ ));
      bAllocationError = true;
      rc = BERR_OUT_OF_DEVICE_MEMORY;
      goto error;
   }

   /*
    * Get the virtual address.
    */
   pstDebugFifo->pBuffer = BMMA_Lock( pstDebugFifo->hBMMABlock );

   if ( 0 == pstDebugFifo->pBuffer )
   {
      BDBG_ERR(("%s:: BMMA_Lock failure", __FUNCTION__ ));
      bAllocationError = true;
      rc = BERR_OUT_OF_DEVICE_MEMORY;
      goto error;
   }

   pstDebugFifo->bBufferValid = true;

   /*
    * Create the debug FIFO
    */
   BDBG_Fifo_GetDefaultCreateSettings( &stCreateSettings );

   stCreateSettings.elementSize = pstDebugFifo->uiElementSize;
   stCreateSettings.bufferSize = pstDebugFifo->uiFifoSize;
   stCreateSettings.buffer = pstDebugFifo->pBuffer;

   rc = BDBG_Fifo_Create( &(pstDebugFifo->hDebugFifo), &stCreateSettings );

   if ( BERR_SUCCESS != rc )
   {
      BDBG_ERR(("%s:: BDBG_Fifo_Create failure: rc:%08x", __FUNCTION__, rc ));
      bAllocationError = true;
   }
   BDBG_MSG(("%s:: ", __FUNCTION__ ));
   BDBG_MSG(("    pstDebugFifo->hBMMAHeap: %08x", (unsigned)pstDebugFifo->hBMMAHeap ));
   BDBG_MSG(("    pstDebugFifo->hDebugFifo: %08x", (unsigned)pstDebugFifo->hDebugFifo ));
   BDBG_MSG(("    pstDebugFifo->pBuffer: %08x", (unsigned)pstDebugFifo->pBuffer ));
   BDBG_MSG(("    pstDebugFifo->bmmaOffset: %08x", (unsigned)pstDebugFifo->bmmaOffset ));
   BDBG_MSG(("    pstDebugFifo->hBMMABlock: %08x", (unsigned)pstDebugFifo->hBMMABlock ));
   BDBG_MSG(("    pstDebugFifo->uiElementSize: %d", pstDebugFifo->uiElementSize ));
   BDBG_MSG(("    pstDebugFifo->uiFifoSize: %d", pstDebugFifo->uiFifoSize ));


error:

   if ( true == bAllocationError )
   {
      BXDM_PPDFIFO_P_Fifo_Destroy( hXdmPP );
   }

   BDBG_LEAVE( BXDM_PPDFIFO_P_Fifo_Create );

   return BERR_TRACE( rc );

}

BERR_Code BXDM_PPDFIFO_P_Fifo_Destroy(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BXDM_PictureProvider_P_DebugFifo * pstDebugFifo = &(hXdmPP->stDMConfig.stDebugFifo);

   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER( BXDM_PPDFIFO_P_Fifo_Destroy );

   /* Release the debug fifo. */

   if ( NULL == pstDebugFifo )
   {
      BDBG_ERR(("%s  pstDebugFifo is NULL", __FUNCTION__ ));
      goto error;
   }

   BDBG_MSG(("%s::", __FUNCTION__ ));
   BDBG_MSG(("    pstDebugFifo->hBMMAHeap: %08x", (unsigned)pstDebugFifo->hBMMAHeap ));
   BDBG_MSG(("    hDebugFifo: %08x", (unsigned)pstDebugFifo->hDebugFifo ));
   BDBG_MSG(("    pBuffer: %08x", (unsigned)pstDebugFifo->pBuffer ));
   BDBG_MSG(("    bmmaOffset: %08x", (unsigned)pstDebugFifo->bmmaOffset ));
   BDBG_MSG(("    hBMMABlock: %08x", (unsigned)pstDebugFifo->hBMMABlock ));

   if ( 0 != pstDebugFifo->pBuffer )
   {
      BMMA_Unlock( pstDebugFifo->hBMMABlock, pstDebugFifo->pBuffer );
      pstDebugFifo->pBuffer = NULL;
      pstDebugFifo->bBufferValid = false;
   }

   if ( 0 != pstDebugFifo->bmmaOffset )
   {
      BMMA_UnlockOffset( pstDebugFifo->hBMMABlock, pstDebugFifo->bmmaOffset );
      pstDebugFifo->bmmaOffset = 0;
   }

   if ( NULL != pstDebugFifo->hBMMABlock )
   {
      BMMA_Free( pstDebugFifo->hBMMABlock );
      pstDebugFifo->hBMMABlock = NULL;
   }

   if ( NULL != pstDebugFifo->hDebugFifo )
   {
      BDBG_Fifo_Destroy( pstDebugFifo->hDebugFifo );
      pstDebugFifo->hDebugFifo = NULL;
   }

error:

   BDBG_LEAVE( BXDM_PPDFIFO_P_Fifo_Destroy );

   return BERR_TRACE( rc );

}

BERR_Code BXDM_PPDFIFO_P_Reader_Create(
   BXDM_PictureProvider_Handle hXdmPP
   )
{

   BERR_Code rc = BERR_SUCCESS;
   bool bAllocationError = false;
   BXDM_Debug_ReaderInfo * pstReaderInfo;

   BDBG_ENTER( BXDM_PPDFIFO_P_Reader_Create );

   BDBG_ASSERT( hXdmPP );

   pstReaderInfo = &hXdmPP->stDMConfig.stDebugReader;

   rc = BXDM_PictureProvider_GetDebugFifo( hXdmPP, &(pstReaderInfo->stFifoInfo) );

   if ( BERR_SUCCESS != rc )
   {
      BDBG_ERR(("%s:: BXDM_PictureProvider_GetDebugFifo failure: rc:%08x", __FUNCTION__, rc ));
      bAllocationError = true;
      goto error;
   }

   pstReaderInfo->pEntry = BKNI_Malloc( pstReaderInfo->stFifoInfo.uiElementSize );

   if ( NULL == pstReaderInfo->pEntry )
   {
      BDBG_ERR(("%s:: BKNI_Malloc failure", __FUNCTION__ ));
      rc = BERR_OUT_OF_SYSTEM_MEMORY;
      bAllocationError = true;
      goto error;
   }

   pstReaderInfo->pDebugFifo = BMMA_Lock( pstReaderInfo->stFifoInfo.hBlock );

   if ( 0 == pstReaderInfo->pDebugFifo )
   {
      BDBG_ERR(("%s:: BMMA_Lock failure:", __FUNCTION__ ));
      rc = BERR_OUT_OF_SYSTEM_MEMORY;
      bAllocationError = true;
      goto error;
   }

   pstReaderInfo->pDebugFifo = (uint8_t *)pstReaderInfo->pDebugFifo + pstReaderInfo->stFifoInfo.uiOffset;

   rc = BDBG_FifoReader_Create(&(pstReaderInfo->hDebugReader), pstReaderInfo->pDebugFifo);

   if (  BERR_SUCCESS != rc  )
   {
      BDBG_ERR(("%s:: BDBG_FifoReader_Create failure: rc:%08x", __FUNCTION__, rc ));
      bAllocationError = true;
      goto error;
   }

   BDBG_MSG(("%s::", __FUNCTION__ ));
   BDBG_MSG(("    pstReaderInfo->hDebugReader: %08x", (unsigned)pstReaderInfo->hDebugReader ));
   BDBG_MSG(("    pstReaderInfo->pEntry: %08x", (unsigned)pstReaderInfo->pEntry ));
   BDBG_MSG(("    pstReaderInfo->pDebugFifo: %08x", (unsigned)pstReaderInfo->pDebugFifo ));

error:

   if ( true == bAllocationError )
   {
      BXDM_PPDFIFO_P_Reader_Destroy( hXdmPP );
   }

   BDBG_LEAVE( BXDM_PPDFIFO_P_Reader_Create );

   return BERR_TRACE( rc );

}


BERR_Code BXDM_PPDFIFO_P_Reader_Destroy(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BXDM_Debug_ReaderInfo * pstReaderInfo;

   BDBG_ASSERT( hXdmPP );

   BDBG_ENTER( BXDM_PPDFIFO_P_Reader_Destroy );

   pstReaderInfo = &hXdmPP->stDMConfig.stDebugReader;


   BDBG_MSG(("%s::", __FUNCTION__ ));
   BDBG_MSG(("    pstReaderInfo->hDebugReader: %08x", (unsigned)pstReaderInfo->hDebugReader ));
   BDBG_MSG(("    pstReaderInfo->pEntry: %08x", (unsigned)pstReaderInfo->pEntry ));
   BDBG_MSG(("    pstReaderInfo->pDebugFifo: %08x", (unsigned)pstReaderInfo->pDebugFifo ));


   if ( NULL != pstReaderInfo->pDebugFifo )
   {
      /* TODO: should this come into play, i.e. is a subtract needed?
       * pstReaderInfo->pDebugFifo = (uint8_t *)pstReaderInfo->pDebugFifo + pstReaderInfo->stFifoInfo.uiOffset;
       */
      BMMA_Unlock( pstReaderInfo->stFifoInfo.hBlock, pstReaderInfo->pDebugFifo );
      pstReaderInfo->pDebugFifo = NULL;
   }

   if ( NULL != pstReaderInfo->pEntry )
   {
      BKNI_Free( pstReaderInfo->pEntry );
      pstReaderInfo->pEntry = NULL;
   }

   if ( NULL != pstReaderInfo->hDebugReader )
   {
      BDBG_FifoReader_Destroy( pstReaderInfo->hDebugReader );
      pstReaderInfo->hDebugReader = NULL;
   }

   BDBG_LEAVE( BXDM_PPDFIFO_P_Reader_Destroy );

   return BERR_TRACE( rc );

}

/*
 * SWSTB-1380: end of prototype code for the BDBG_Fifo
 */


#endif
