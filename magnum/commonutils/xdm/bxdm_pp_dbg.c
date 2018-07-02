/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bxdm_pp_dbg.h"
#include "bxdm_pp_qm.h"
#include "bxdm_pp_dbg_common.h"

BDBG_MODULE(BXDM_PPDBG);
BDBG_FILE_MODULE(BXDM_PPDBG);
BDBG_FILE_MODULE(BXDM_MFD1);
BDBG_FILE_MODULE(BXDM_MFD2);
BDBG_FILE_MODULE(BXDM_MFD3);
BDBG_FILE_MODULE(BXDM_PPQM);
BDBG_FILE_MODULE(BXDM_CFG);
BDBG_FILE_MODULE(BXDM_PPDBG2);
BDBG_FILE_MODULE(BXDM_PPDBC);
BDBG_FILE_MODULE(BXDM_PPFRD);
BDBG_FILE_MODULE(BXDM_PPFIC);
BDBG_FILE_MODULE(BXDM_PPCB);
BDBG_FILE_MODULE(BXDM_PPCLIP);
BDBG_FILE_MODULE(BXDM_PPOUT);
BDBG_FILE_MODULE(BXDM_PPTSM);
BDBG_FILE_MODULE(BXDM_PPVTSM);
BDBG_FILE_MODULE(BXDM_PPV2);

#if BDBG_DEBUG_BUILD

/*
 * Functions
 */
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


#if 0
BERR_Code BXDM_PPDBG_P_ResetString(
   BXDM_PPDBG_P_String *pStringInfo
   )
{
   pStringInfo->szDebugStr[0] = '\0';
   pStringInfo->uiDebugStrOffset = 0;

   return BERR_SUCCESS;
}

BERR_Code BXDM_PPDBG_P_AppendString(
   BXDM_PPDBG_P_String *pStringInfo,
   const char *fmt,
   ...
   )
{
   BERR_Code rc = BERR_SUCCESS;

   if (pStringInfo->uiDebugStrOffset < sizeof(pStringInfo->szDebugStr))
   {
      va_list args;

      va_start(args, fmt);
      pStringInfo->uiDebugStrOffset += BKNI_Vsnprintf(pStringInfo->szDebugStr +
                                                     pStringInfo->uiDebugStrOffset,
                                                     sizeof(pStringInfo->szDebugStr) -
                                                     pStringInfo->uiDebugStrOffset,
                                                     fmt,
                                                     args);
      va_end(args);
   }
   else
   {
      rc = BERR_OUT_OF_DEVICE_MEMORY;
   }

   return rc;
}

BERR_Code BXDM_PPDBG_P_PrintString(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PPDBG_P_String *pStringInfo
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_MODULE_MSG(BXDM_PPDBG, ("%s", pStringInfo->szDebugStr));

   if (pStringInfo->uiDebugStrOffset >= sizeof(pStringInfo->szDebugStr))
   {
      BXVD_DBG_ERR(hXdmPP, ("Debug Buffer Overflow!"));
      rc = BERR_OUT_OF_DEVICE_MEMORY;
   }

   BXDM_PPDBG_P_ResetString(pStringInfo);

   return rc;
}
#endif
BERR_Code BXDM_PPDBG_P_OutputLog_isr(
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

BERR_Code BXDM_PPDBG_P_OutputSPOLog_isr(
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

BERR_Code BXDM_PPDBG_P_SelectionLog_isr(
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

BERR_Code BXDM_PPDBG_P_CallbackTriggeredLog_isr(
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

BERR_Code BXDM_PPDBG_P_StateLog_isr(
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

BERR_Code BXDM_PPDBG_P_State2Log_isr(
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


BERR_Code BXDM_PPDBG_P_StcDeltaLog_isr(
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


BERR_Code BXDM_PPDBG_P_DecoderDropLog_isr(
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

BERR_Code BXDM_PPDBG_P_Print_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PictureProvider_P_LocalState* pLocalState,
   bool bForcePrint
   )
{
   BXDM_PPDBG_P_Info *pDebugInfo = &hXdmPP->stDMState.stDecode.stDebug;

   /* See if we need to print out stats */
   if (pDebugInfo->uiVsyncCount == (BXDM_PPDBG_P_MAX_VSYNC_DEPTH - 1)
       || true == bForcePrint )
   {
      int32_t iAverageStcDelta;
      uint32_t uiAverageFractionBase10;

      BXDM_PictureProvider_MonitorRefreshRate eMonitorRefreshRate = hXdmPP->stDMConfig.eMonitorRefreshRate;

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
         BXDM_PPFP_P_FixPtBinaryFractionToBase10_isrsafe(
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


      BDBG_MODULE_MSG(BXDM_PPDBG, (" DM Log (ch:%02x stc:%08x%c mr:%sHz edSTC:%u adSTC:%d.%u pbr:%d%c tm:%s %s)",
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                pLocalState->uiStcSnapshot,
                                (hXdmPP->stDMState.stDecode.iStcJitterCorrectionOffset != 0)?'*':' ',
                                BXDM_P_MonitorRefreshRateToStrLUT[eMonitorRefreshRate],
                                pLocalState->stSTCDelta.uiWhole,
                                iAverageStcDelta,
                                uiAverageFractionBase10,
                                ( pLocalState->uiSlowMotionRate / BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE_EXTRA_DECIMALS ),
                                '%',
                                ( pLocalState->eSTCTrickMode < BXDM_PictureProvider_P_STCTrickMode_eMax) ?
                                          BXDM_P_STCTrickModeToStrLUT[ pLocalState->eSTCTrickMode ] : "error",
                                hXdmPP->stDMConfig.bPlayback?"pb":"lv"
                                ));
      {
         /* Add the info to the debug fifo. */
         BXDM_DebugFifo_DebugInfo stDebugInfo;

         stDebugInfo.uiStcSnapshot = pLocalState->uiStcSnapshot;
         stDebugInfo.iStcJitterCorrectionOffset = hXdmPP->stDMState.stDecode.iStcJitterCorrectionOffset;
         stDebugInfo.eMonitorRefreshRate = eMonitorRefreshRate;
         stDebugInfo.stSTCDelta = pLocalState->stSTCDelta;
         stDebugInfo.iAverageStcDelta = iAverageStcDelta;
         stDebugInfo.uiAverageFractionBase10 = uiAverageFractionBase10;
         stDebugInfo.uiSlowMotionRate = pLocalState->uiSlowMotionRate;
         stDebugInfo.eSTCTrickMode = pLocalState->eSTCTrickMode;
         stDebugInfo.bPlayback = hXdmPP->stDMConfig.bPlayback;

         BXDM_PPDFIFO_P_QueDBG_isr( hXdmPP, &stDebugInfo );
      }

      /* Print TSM Logs */
      BDBG_MODULE_MSG(BXDM_PPDBG, ("%s", (char *)&pDebugInfo->stTSMString.szDebugStr));
      BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, false, "%s", (char *)&pDebugInfo->stTSMString.szDebugStr );

      /* Print Decoder Drop Logs */
      if ( true ==  pDebugInfo->bPrintDropCount )
      {
         BDBG_MODULE_MSG(BXDM_PPDBG, ("%s", (char *)&pDebugInfo->stPendingDropString.szDebugStr));
         BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, false, (char *)&pDebugInfo->stPendingDropString.szDebugStr );
      }

      /* Print picture selecton log */
      BDBG_MODULE_MSG(BXDM_PPDBG, ("%s", (char *)&pDebugInfo->stInterruptString.szDebugStr));
      BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, false, "%s", (char *)&pDebugInfo->stInterruptString.szDebugStr );

      /* Print stats for Source Polarity Override */
      if ( true == pDebugInfo->bPrintSPO )
      {
         BDBG_MODULE_MSG(BXDM_PPDBG, ("%s", (char *)&pDebugInfo->stSourcePolarityOverrideString.szDebugStr));
         BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, false, "%s", (char *)&pDebugInfo->stSourcePolarityOverrideString.szDebugStr );
      }

      /* Print stats for Callbacks */
      if ( true == pDebugInfo->bPrintCallbacks )
      {
         BDBG_MODULE_MSG(BXDM_PPDBG, ("%s", (char *)&pDebugInfo->stCallbackString.szDebugStr));
         BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, false, "%s", (char *)&pDebugInfo->stCallbackString.szDebugStr );
      }

      /* Print State Logs: */
      BDBG_MODULE_MSG(BXDM_PPDBG, ("%s", (char *)&pDebugInfo->stStateString.szDebugStr));
      BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, false, "%s", (char *)&pDebugInfo->stStateString.szDebugStr );

      /* Print stats for state 2 */
      if ( true == pDebugInfo->bPrintState2 )
      {
         BDBG_MODULE_MSG( BXDM_PPDBG2, ("%s", (char *)&pDebugInfo->stState2String.szDebugStr));
         BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG2, false, "%s", (char *)&pDebugInfo->stState2String.szDebugStr );

      }

      BDBG_MODULE_MSG( BXDM_PPDBC, ("%s", (char *)&pDebugInfo->stStcDeltaString.szDebugStr));
      BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBGC, false, "%s", (char *)&pDebugInfo->stStcDeltaString.szDebugStr );

      BKNI_Memset(pDebugInfo, 0, sizeof(BXDM_PPDBG_P_Info));
   }
   else
   {
      pDebugInfo->uiVsyncCount++;
   }

   return BERR_SUCCESS;
}

void BXDM_PPDBG_P_PrintStartDecode_isr(
   const BXDM_PictureProvider_Handle hXdmPP
   )
{
   /* SW7445-1259: reduce messages printed at start decode time */
   BXDM_PictureProvider_P_State_Decode *pDecode = &hXdmPP->stDMState.stDecode;
   BXDM_PPDBG_P_Info *pDebug = &pDecode->stDebug;

   BDBG_MODULE_MSG(BXDM_PPDBG, ("--- %x:[%02x.xxx] BXDM_PictureProvider_StartDecode_isr has been called (hXdmPP:0x%lu) ---",
                              pDebug->uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              (unsigned long)hXdmPP ));

   BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, true,
            "--- %x:[%02x.xxx] BXDM_PictureProvider_StartDecode_isr has been called (hXdmPP:0x%lu) ---",
            pDebug->uiVsyncCount,
            BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
            (unsigned long)hXdmPP );

   return;

}


void BXDM_PPDBG_P_PrintStopDecode_isr(
   const BXDM_PictureProvider_Handle hXdmPP
   )
{
   BXDM_PictureProvider_P_State_Decode *pDecode = &hXdmPP->stDMState.stDecode;
   BXDM_PPDBG_P_Info *pDebug = &pDecode->stDebug;

   BDBG_MODULE_MSG(BXDM_PPDBG, ("--- %x:[%02x.xxx] BXDM_PictureProvider_StopDecode_isr has been called (hXdmPP:0x%lu) ---",
                              pDebug->uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              (unsigned long)hXdmPP ));

   BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eDBG, true,
            "--- %x:[%02x.xxx] BXDM_PictureProvider_StopDecode_isr has been called (hXdmPP:0x%lu) ---",
            pDebug->uiVsyncCount,
            BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
            (unsigned long)hXdmPP );
   return;
}



/*
** SW7335-781: Output warning when forcing picture selection override
** Output Selection Mode override message only once, if:
** an override to vTSM mode occurred
** the override was not the same as the previous Override
** function assumes new selection mode is VSYNC (vTSM) mode
*/
void BXDM_PPDBG_P_PrintSelectionModeOverride_isr(
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
void BXDM_PPDBG_P_PrintEndSelectionModeOverride_isr(
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
 * SW7405-4736: conditionally print the MFD structure.
 */
void BXDM_PPDBG_P_PrintMFD_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BAVC_MFD_Picture* pMFDPicture
   )
{
   BAVC_MFD_Picture * pMFD;
   BXDM_PPDBG_P_Info * pDebug;
   BXDM_PictureProvider_P_Picture_Context * pstSelectedPicture;
   int32_t  i=0;
   char cBarDataType;

   BDBG_ENTER( BXDM_PPDBG_P_PrintMFD_isr );

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pLocalState );
   BDBG_ASSERT( pMFDPicture );

   pMFD = pMFDPicture;
   pDebug = &(hXdmPP->stDMState.stDecode.stDebug);
   pstSelectedPicture = &(hXdmPP->stDMState.stChannel.stSelectedPicture);


   while ( NULL != pMFD )
   {
      BFMT_AspectRatio eAspectRatio=BFMT_AspectRatio_eUnknown;

      /* Range check */

      if ( pMFD->eAspectRatio <= BFMT_AspectRatio_eSAR )
      {
         eAspectRatio = pMFD->eAspectRatio;
      }

      BDBG_MODULE_MSG( BXDM_MFD1, ("%c%x:[%02x.%03x] id:%03x pts:%08x %s->%s %dx%d->%dx%d %s:%dx%d AQP:%02d fr:%s mr:%s %s%s%s%s%s%s%s%s%s%s%s",
                                 ( pMFD->bMute ) ? 'M' : ' ',
                                 pDebug->uiVsyncCount,
                                 BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                 pstSelectedPicture->stPicParms.uiPPBIndex & 0xFFF,
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
                                 pDebug->uiVsyncCount,
                                 BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                 pstSelectedPicture->stPicParms.uiPPBIndex & 0xFFF,
                                 ( pMFD->eBufferFormat ) ? "fp" : "Fr" ,
                                 pMFD->stHdrMetadata.stDynamic.eType,
                                 (unsigned long)pMFD->hLuminanceFrameBufferBlock,
                                 pMFD->ulLuminanceFrameBufferBlockOffset,
                                 (unsigned long)pMFD->hChrominanceFrameBufferBlock,
                                 pMFD->ulChrominanceFrameBufferBlockOffset,
                                 (unsigned long)pMFD->hLuminanceBotFieldBufferBlock,
                                 pMFD->ulLuminanceBotFieldBufferBlockOffset,
                                 (unsigned long)pMFD->hChrominanceBotFieldBufferBlock,
                                 pMFD->ulChrominanceBotFieldBufferBlockOffset
                          ));

      pMFD = pMFD->pEnhanced;
      i++;
   }

   pMFD = pMFDPicture; /* reset to the orignal value. */

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
                                 pDebug->uiVsyncCount,
                                 BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                 pstSelectedPicture->stPicParms.uiPPBIndex & 0xFFF,
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
                                 ( BAVC_VideoBitDepth_e10Bit == pMFDPicture->eBitDepth ) ? "10" : "8",
                                 pMFD->ulChunkId,
                                 ( pMFD->bFrameProgressive ) ? " fp" : " ",
                                 ( pMFD->bStreamProgressive ) ? " sp" : " "
                                 ));


   /* Add the data to the debug fifo. */
   BXDM_PPDFIFO_P_QueMFD_isr( hXdmPP, pLocalState, pMFDPicture );

   BDBG_LEAVE( BXDM_PPDBG_P_PrintMFD_isr );

   return;

}  /* end of BXDM_PPDBG_P_PrintMFD */

void BXDM_PPDBG_P_PrintUnifiedPicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicture
   )
{
   BXDM_PictureProvider_P_Picture_Context *  pstSelectedPicture;
   const BXDM_Picture * pstUnified;
   const BXDM_Picture * pstSelectedUnified;

   bool  bPrintAdditional;
   BXDM_Picture_Coding eCoding;
   BXDM_Picture_PullDown ePulldown;
   char  cSourceFormat, cProgressiveSequence, cSelectionMode, cErrorOnThisPicture;

   BDBG_ENTER( BXDM_PPDBG_P_PrintUnifiedPicture_isr );

   BSTD_UNUSED(pLocalState);

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

   /* Add the data to the debug fifo. Note: BXDM_PPDFIFO_P_QueUnifiedPicture_isr checks uiVsyncCountQM,
   * it needs to be called before uiVsyncCountQM is reset later in this routine. */

   BXDM_PPDFIFO_P_QueUnifiedPicture_isr( hXdmPP, pLocalState, pstPicture );

   /* Print once per second or when any of the parameters change. */

   bPrintAdditional = ( hXdmPP->stDMState.stDecode.uiVsyncCountQM >= BXDM_P_VsyncsPerSecondLUT[ hXdmPP->stDMConfig.eMonitorRefreshRate ] );

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

   if ( true == bPrintAdditional )
   {
      bool bProtocolValid;
      BAVC_FrameRateCode eFrameRate = BAVC_FrameRateCode_eUnknown;
      BFMT_AspectRatio eAspectRatio = BFMT_AspectRatio_eUnknown;
      BXDM_PPFP_P_DataType stDeltaPTSAvg;
      uint32_t uiAverageFractionBase10;

      if ( 0 !=  hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount )
      {
         /* determine Average dPTS (using FRD parameters) ... */
         BXDM_PPFP_P_FixPtDiv_isrsafe(
               &hXdmPP->stDMState.stDecode.stFRDStats.stDeltaPTSRunningSum,
               hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount,
               &stDeltaPTSAvg
               );

         /* Convert the fractional part of stDeltaPTSAvg to a base 10 value for display. */
         BXDM_PPFP_P_FixPtBinaryFractionToBase10_isrsafe( &stDeltaPTSAvg, 2, &uiAverageFractionBase10 );
      }
      else
      {
         stDeltaPTSAvg.uiFractional = 0;
         stDeltaPTSAvg.uiWhole = 0;
         uiAverageFractionBase10 = 0;
      }

      /* Range check the variables used to index lookup tables. */

      bProtocolValid = ( pstUnified->stProtocol.eProtocol < BXDM_P_MAX_VIDEO_PROTOCOL ) ? true : false ;

      if ( pstPicture->stPicParms.stTSM.stStatic.eFrameRateXVD < BXDM_PictureProvider_P_MAX_FRAMERATE )
      {
         eFrameRate = pstPicture->stPicParms.stTSM.stStatic.eFrameRateXVD;
      }

      if (  pstUnified->stAspectRatio.eAspectRatio <= BFMT_AspectRatio_eSAR )
      {
         eAspectRatio =  pstUnified->stAspectRatio.eAspectRatio;
      }

      hXdmPP->stDMState.stDecode.uiVsyncCountQM = 0;

      BDBG_MODULE_MSG( BXDM_PPQM, (" %x:[%02x.%03x] %4dx%3d %s%scfr:%d/%d afr:%sHz(%s),ar:%s(%d,%d)(%d) swOff:%x avPts:%d.%u %s%s",
                             hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                             BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                             pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                             pstUnified->stBufferInfo.stSource.uiWidth,
                             pstUnified->stBufferInfo.stSource.uiHeight,
                             ( bProtocolValid ) ? BXDM_P_VideoCompressionStdToStrLUT[ pstUnified->stProtocol.eProtocol ] : "ukn" ,
                             ( pstUnified->stBufferInfo.eBufferFormat == BXDM_Picture_BufferFormat_eSplitInterlaced ) ?  "-si," : "," ,
                             pstUnified->stFrameRate.stRate.uiNumerator,
                             pstUnified->stFrameRate.stRate.uiDenominator,
                             BXDM_P_BAVCFrameRateToStrLUT[ eFrameRate ],
                             BXDM_P_FrameRateTypeToStrLUT[ pstPicture->stPicParms.stTSM.stStatic.eFrameRateType ],
                             BXDM_P_AspectRatioToStrLUT[ eAspectRatio ],
                             pstUnified->stAspectRatio.uiSampleAspectRatioX,
                             pstUnified->stAspectRatio.uiSampleAspectRatioY,
                             pstUnified->stAspectRatio.bValid,
                             pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation,
                             stDeltaPTSAvg.uiWhole,
                             uiAverageFractionBase10,
                             BXDM_P_DisplayFieldModeToStrLUT[ pstPicture->stPicParms.stDisplay.stDynamic.eDisplayFieldMode ],
                             ( pstPicture->stPicParms.stDisplay.stDynamic.bForceSingleFieldMode ) ? "-f" :
                                 ( BXDM_PictureProvider_DisplayFieldMode_eAuto == hXdmPP->stDMConfig.eDisplayFieldMode ) ? "-a" : ""
                             ));
   }


   /* The following values from the Unified Picture are set outside of XDM.  Range check the values to avoid
    * stepping off the end of lookup tables, i.e. "s_aXXXToStrLUT[]"
    */
   eCoding = ( pstUnified->stPictureType.eCoding < BXDM_Picture_Coding_eMax ) ? pstUnified->stPictureType.eCoding : BXDM_Picture_Coding_eUnknown ;

   /* SW7445-586: use accessor method to retrieve pulldown */
   BXDM_PPQM_P_GetPicturePulldown_isr( pstPicture, &ePulldown );

   if ( ePulldown >= BXDM_Picture_PullDown_eMax ) ePulldown = 0;

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

   switch ( pstPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode )
   {
      case BXDM_PictureProvider_PictureHandlingMode_eHonorPTS:    cSelectionMode = 'h';   break;
      case BXDM_PictureProvider_PictureHandlingMode_eIgnorePTS:   cSelectionMode = 'i';   break;
      case BXDM_PictureProvider_PictureHandlingMode_eDrop:        cSelectionMode = 'd';   break;
      case BXDM_PictureProvider_PictureHandlingMode_eWait:
         cSelectionMode = (BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode) ? 'w' : 'v' ;
         break;

      case BXDM_PictureProvider_PictureHandlingMode_eDefault:
      default:
         if ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode )
         {
            cSelectionMode = ( pstPicture->stPicParms.stTSM.stDynamic.bEvaluatedWithSwStc ) ? 's' : 't';
         }
         else
         {
            cSelectionMode = ( pstPicture->stPicParms.stTSM.stDynamic.bSelectedInPrerollMode ) ? 'p' : 'v' ;
         }
         break;
   }

   /* A picture-less picture, it will simply be dropped. */
   if ( false == pstPicture->pstUnifiedPicture->stBufferInfo.bValid )
   {
      cSelectionMode = 'x';
   }

   /* SWSTB-439: if the error threshold is non-zero, use "uiPercentError" from the picture structure
    * to determine if the picture has an error.  Otherwise use the error flag. */

   if ( 0 != hXdmPP->stDMConfig.uiErrorThreshold )
   {
      cErrorOnThisPicture = ( pstUnified->stError.uiPercentError == 0 ) ? ' ' :
                                    ( pstUnified->stError.uiPercentError >= hXdmPP->stDMConfig.uiErrorThreshold ) ? 'e' : 'p';
   }
   else
   {
      cErrorOnThisPicture = ( pstUnified->stError.bThisPicture == true )  ? 'E' : ' ';
   }


   if ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode )
   {
      uint32_t uiPcrOffset;
      uint32_t uiDisplayOffset;
      bool     bPcrOffsetValid;
      bool     bPcrOffsetDiscontinuity;

      BXDM_PPQM_P_GetHwPcrOffset_isr( hXdmPP, pstPicture, &uiPcrOffset );

      uiPcrOffset += pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation;

      BXDM_PPQM_P_GetPtsOffset_isr( hXdmPP, pstPicture, &uiDisplayOffset );

      if (( false == hXdmPP->stDMConfig.bUseHardwarePCROffset )
            || ( true == hXdmPP->stDMConfig.bPlayback))
      {
         bPcrOffsetValid = true;
         bPcrOffsetDiscontinuity = false;
      }
      else
      {
         bPcrOffsetValid = pstPicture->stPicParms.stTSM.stStatic.stPCROffsetXDM.bValidOverloaded;
         bPcrOffsetDiscontinuity = pstPicture->stPicParms.stTSM.stStatic.stPCROffsetXDM.bDiscontinuityOverloaded;
      }

      BDBG_MODULE_MSG( BXDM_PPQM, (" %x:[%02x.%03x]%c%c%c%s,%s,fs:%c%c %s,pts:%08x(%d),off:%04x,j:%3d,pcr:%08x(%d,%d),%s:%08x%c,d:%08x",
                             hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                             BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                             pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                             cErrorOnThisPicture,
                             cSelectionMode,
                             ( true == pstPicture->stPicParms.stDisplay.stDynamic.bAppendedToPreviousPicture ) ? '^' : ':',
                             BXDM_P_TSMResultToStrLUT[ pstPicture->stPicParms.stTSM.stDynamic.eTsmResult],
                             ( BXDM_Picture_BufferFormat_eSplitInterlaced == pstUnified->stBufferInfo.eBufferFormat ) ?
                                ( BXDM_Picture_BufferHandlingMode_eSiRepeat == pstUnified->stBufferInfo.eBufferHandlingMode ) ?
                                   BXDM_P_SiRepeatPullDownEnumToStrLUT[ ePulldown ] : BXDM_P_SiPullDownEnumToStrLUT[ ePulldown ] :
                                 BXDM_P_PullDownEnumToStrLUT[ ePulldown ],
                             cSourceFormat,
                             cProgressiveSequence,
                             BXDM_P_BXDMPictureCodingToStrLUT[ eCoding ],
                             pstUnified->stPTS.uiValue,
                             pstUnified->stPTS.bValid,
                             uiDisplayOffset,
                             pstPicture->stPicParms.stTSM.stStatic.iPTSJitterCorrection,
                             uiPcrOffset,
                             bPcrOffsetValid,
                             bPcrOffsetDiscontinuity,
                             ( true == pLocalState->bUsingXPTSTC ) ? "xstc" : "stc",
                             pLocalState->uiStcSnapshot,
                             (hXdmPP->stDMState.stDecode.iStcJitterCorrectionOffset != 0)?'*':' ',
                             pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceActual
                             ));
   }
   else
   {
      uint32_t    uiVirtualStc;
      uint32_t    uiDisplayOffset;

      BXDM_PPFP_P_DataType stVirtualPts;

      BXDM_PPQM_P_GetPtsOffset_isr( hXdmPP, pstPicture, &uiDisplayOffset );

      BXDM_PPVTSM_P_VirtualStcGet_isr( hXdmPP, &uiVirtualStc );

      uiVirtualStc += pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation;

      BXDM_PPQM_P_GetPtsWithFrac_isr(
            pstPicture,
            BXDM_PictureProvider_P_PTSIndex_eVirtual,
            pstPicture->stPicParms.stDisplay.stDynamic.uiSelectedElement,
            &stVirtualPts
            );

      BDBG_MODULE_MSG( BXDM_PPQM, (" %x:[%02x.%03x]%c%c%c%s,%s,fs:%c%c %s,pts:%08x(%d),off:%04x,j:   ,vpts:%08x   ,vstc:%08x ,d:%08x",
                             hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                             BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                             pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                             cErrorOnThisPicture,
                             cSelectionMode,
                             ( true == pstPicture->stPicParms.stDisplay.stDynamic.bAppendedToPreviousPicture ) ? '^' : ':',
                             BXDM_P_TSMResultToStrLUT[ pstPicture->stPicParms.stTSM.stDynamic.eTsmResult],
                             ( BXDM_Picture_BufferFormat_eSplitInterlaced == pstUnified->stBufferInfo.eBufferFormat ) ?
                                ( BXDM_Picture_BufferHandlingMode_eSiRepeat == pstUnified->stBufferInfo.eBufferHandlingMode ) ?
                                   BXDM_P_SiRepeatPullDownEnumToStrLUT[ ePulldown ] : BXDM_P_SiPullDownEnumToStrLUT[ ePulldown ] :
                                 BXDM_P_PullDownEnumToStrLUT[ ePulldown ],
                             cSourceFormat,
                             cProgressiveSequence,
                             BXDM_P_BXDMPictureCodingToStrLUT[ eCoding ],
                             pstUnified->stPTS.uiValue,
                             pstUnified->stPTS.bValid,
                             uiDisplayOffset,
                             stVirtualPts.uiWhole,
                             uiVirtualStc,
                             pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceActual
                             ));

   }

Done:

   BDBG_LEAVE( BXDM_PPDBG_P_PrintUnifiedPicture_isr );

   return;

}  /* end of BXDM_PPDBG_P_PrintUnifiedPicture */

void BXDM_PPDBG_P_PrintDMConfig_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   bool bLastCall
   )
{
   BDBG_ENTER( BXDM_PPDBG_P_PrintDMConfig_isr );

   BSTD_UNUSED(pLocalState);

   /* Print when any of the parameters change. */

   if (  hXdmPP->stDMConfig.uiDirtyBits_1 || hXdmPP->stDMConfig.uiDirtyBits_2 )
   {
      uint32_t uiDirtyBitsGroup_1_1, uiDirtyBitsGroup_1_2, uiDirtyBitsGroup_1_3, uiDirtyBitsGroup_1_4;
      uint32_t uiDirtyBitsGroup_2_1, uiDirtyBitsGroup_2_2, uiDirtyBitsGroup_2_3, uiDirtyBitsGroup_2_4 ;

      /* Add the data to the debug fifo. */
      BXDM_PPDFIFO_P_QueDMConfig_isr( hXdmPP, pLocalState, bLastCall );

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
         BDBG_MODULE_MSG( BXDM_CFG, ("%c%x:[%02x.xxx]1:%c stc:%c(%d) PTSoff:%08x(%d) STCoff:%08x(%d) usePCR:%c(%d) pbr:%d/%d(%d) dfm:%s(%d)",
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
                  ));

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

         BDBG_MODULE_MSG( BXDM_CFG, ("%c%x:[%02x.xxx]2:%s(%d) %s(%d) %s(%d) mr:%s(%d) dfr:%s(%d) fro:%d/%d(%d,%d)(%d) tm:%s(%d) frz:%c(%d)",
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
                  ));

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

         BDBG_MODULE_MSG( BXDM_CFG, ("%c%x:[%02x.xxx]3:sfo:%c(%d) smo:%c(%d) 1080sm:%c(%d) pdm:%s(%d) 480pdm:%s(%d) 1080pdm:%s(%d) 240sm:%s(%d) hom:%s(%d)",
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
                  ));

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
         BDBG_MODULE_MSG( BXDM_CFG, ("%c%x:[%02x.xxx]4:fra:%s(%d) rvf:%c(%d) avop:%c(%d) 3Do:%s(%d)(%d) jti:%s(%d) frd:%s(%d) ehm:%s(%d) el:%d(%d) astm:%c(%d) avsync:%c(%d) ",
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
                  ));

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

   BDBG_LEAVE( BXDM_PPDBG_P_PrintDMConfig_isr );

   return;

}  /* end of BXDM_PPDBG_P_PrintDMConfig */


#endif
