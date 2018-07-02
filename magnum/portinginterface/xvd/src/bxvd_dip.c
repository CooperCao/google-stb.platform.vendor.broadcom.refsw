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
 * [File Description:]
 *
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"          /* For malloc */
#include "bxvd.h"
#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd_dip.h"
#include "bxvd_reg.h"

#if (BXVD_CHIP != 'N') && (BXVD_CHIP != 'T') && (BXVD_CHIP != 'V')
#include "bchp_decode_ip_shim_0.h"
#endif

BDBG_MODULE(BXVD_DIP);
BDBG_FILE_MODULE(BXVD_DIPCTL);
BDBG_FILE_MODULE(BXVD_DIPDBG);

#if !BXVD_P_RUL_DONE_MASK_64_BITS

/* 32 bit VDC RUL Done interrupt mask support */

#define BXVD_DIP_TEST_MULTI_INTERRUPT_BITS_SET( stDisplayInfo, stIntrSettings)  \
{                                                                               \
   uint32_t uiTempBits = stDisplayInfo.vsync_parity &                           \
      ( stIntrSettings.stRULIDMasks_0.ui32BottomFieldRULIDMask |                \
        stIntrSettings.stRULIDMasks_0.ui32TopFieldRULIDMask |                   \
        stIntrSettings.stRULIDMasks_0.ui32ProgressiveFieldRULIDMask );          \
                                                                                \
   /* Check if more than one vsync polarity bit is set */                       \
   if (uiTempBits & (uiTempBits - 1))                                           \
   {                                                                            \
      BDBG_WRN(("Multiple bits set in the vsync polarity bit field (0x%08x)",   \
                stDisplayInfo.vsync_parity));                                   \
   }                                                                            \
}

#define BXVD_DIP_IS_POLARITY_BOTTOM(stDisplayInfo, stIntrSettings)                       \
   (stDisplayInfo.vsync_parity & stIntrSettings.stRULIDMasks_0.ui32BottomFieldRULIDMask)

#define BXVD_DIP_IS_POLARITY_TOP(stDisplayInfo, stIntrSettings)                       \
   (stDisplayInfo.vsync_parity & stIntrSettings.stRULIDMasks_0.ui32TopFieldRULIDMask)

#define BXVD_DIP_IS_POLARITY_FRAME(stDisplayInfo, stIntrSettings)                             \
   (stDisplayInfo.vsync_parity & stIntrSettings.stRULIDMasks_0.ui32ProgressiveFieldRULIDMask)

#define BXVD_DIP_DBG_WRN_BAD_VSYNC(stDisplayInfo)                                       \
   BDBG_WRN(("Unrecognized VSYNC polarity type (0x%08x)", stDisplayInfo.vsync_parity));

#else

/* 64 bit VDC RUL Done interrupt mask support */

#define BXVD_DIP_TEST_MULTI_INTERRUPT_BITS_SET( stDisplayInfo, stIntrSettings)          \
{                                                                                       \
   uint32_t uiTempBits = stDisplayInfo.vsync_parity &                                   \
            ( stIntrSettings.stRULIDMasks_0.ui32BottomFieldRULIDMask |                  \
              stIntrSettings.stRULIDMasks_0.ui32TopFieldRULIDMask |                     \
              stIntrSettings.stRULIDMasks_0.ui32ProgressiveFieldRULIDMask );            \
                                                                                        \
   uint32_t uiTempBits1 = stDisplayInfo.vsync_parity_1 &                                \
      ( stIntrSettings.stRULIDMasks_1.ui32BottomFieldRULIDMask |                        \
        stIntrSettings.stRULIDMasks_1.ui32TopFieldRULIDMask |                           \
        stIntrSettings.stRULIDMasks_1.ui32ProgressiveFieldRULIDMask );                  \
                                                                                        \
   /* Check if more than one vsync polarity bit is set */                               \
   if ((uiTempBits & (uiTempBits - 1)) ||                                               \
       (uiTempBits1 & (uiTempBits1 - 1)) ||                                             \
       ((uiTempBits != 0 ) && (uiTempBits1 != 0)))                                      \
   {                                                                                    \
      BDBG_WRN(("Multiple bits set in the vsync polarity bit field (0x%08x), (0x%08x)", \
                stDisplayInfo.vsync_parity, stDisplayInfo.vsync_parity_1));             \
   }                                                                                    \
}

#define BXVD_DIP_IS_POLARITY_BOTTOM(stDisplayInfo, stIntrSettings)                           \
   ((stDisplayInfo.vsync_parity & stIntrSettings.stRULIDMasks_0.ui32BottomFieldRULIDMask) || \
    (stDisplayInfo.vsync_parity_1 & stIntrSettings.stRULIDMasks_1.ui32BottomFieldRULIDMask))

#define BXVD_DIP_IS_POLARITY_TOP(stDisplayInfo, stIntrSettings)                           \
   ((stDisplayInfo.vsync_parity & stIntrSettings.stRULIDMasks_0.ui32TopFieldRULIDMask) || \
    (stDisplayInfo.vsync_parity_1 & stIntrSettings.stRULIDMasks_1.ui32TopFieldRULIDMask))

#define BXVD_DIP_IS_POLARITY_FRAME(stDisplayInfo, stIntrSettings)                                  \
   ((stDisplayInfo.vsync_parity & stIntrSettings.stRULIDMasks_0.ui32ProgressiveFieldRULIDMask ) || \
    (stDisplayInfo.vsync_parity_1 & stIntrSettings.stRULIDMasks_1.ui32ProgressiveFieldRULIDMask ))

#define BXVD_DIP_DBG_WRN_BAD_VSYNC(stDisplayInfo)                        \
   BDBG_WRN(("Unrecognized VSYNC polarity type (0x%08x), (0x%08x)",      \
             stDisplayInfo.vsync_parity, stDisplayInfo.vsync_parity_1));

#endif

#define BXVD_DIP_P_MEASURE_LATENCY 0
#define LATENCY_FILE_OUTPUT 1

#if BXVD_DIP_P_MEASURE_LATENCY
#include <stdio.h>
#endif

typedef struct BXVD_DisplayInterruptProvider_P_DisplayInterruptHandler
{
      BXDM_DisplayInterruptHandler_isr fCallback;
      void *pPrivateContext;
} BXVD_DisplayInterruptProvider_P_DisplayInterruptHandler;

typedef struct BXVD_DisplayInterruptProvider_P_ChannelContext
{
      BXVD_DisplayInterruptProvider_P_ChannelSettings stChannelSettings;

      BXVD_DisplayInterruptProvider_P_InterruptSettings stInterruptSettings;
      uint32_t uiRegMaskCurrent_0;
      uint32_t uiRegMaskCurrent_1;
      bool bInterruptSettingsValid;

      /* BINT Callback Handle */
      BINT_CallbackHandle hPictureDataReadyInterruptCallback;
      BXDM_DisplayInterruptInfo stDisplayInterruptInfo;

      /* Application Callback */
      BXVD_DisplayInterruptProvider_P_DisplayInterruptHandler stDisplayInterruptHandler;
      union {
          struct {
              BXVD_P_DisplayInfo stDisplayInfo;
              BXVD_P_DisplayInfo stUpdateDisplayInfo;
          } BXVD_DisplayInterruptProvider_S_PictureDataReady_isr;
      } functionData;

} BXVD_DisplayInterruptProvider_P_ChannelContext;

#if BXVD_DIP_P_MEASURE_LATENCY

#define BXVD_DIP_MAX_TIME_SAMPLES 2048

/* DM's persistent state between BXVD_StartDecode and
 * BXVD_StopDecode */

static uint32_t guiSampleIndex;
static uint32_t gauiTimeSample[BXVD_DIP_MAX_TIME_SAMPLES];


void BXVD_DIP_P_SampleTime(BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh)
{
   uint32_t uiTemp;
   BXVD_Handle hXvd = hXvdDipCh->stChannelSettings.hXvd;
   BXVD_ChannelHandle hCh = hXvd->ahChannel[0];
   BXVD_STC eStc;

   if (guiSampleIndex >= BXVD_DIP_MAX_TIME_SAMPLES)
   {
      BKNI_Printf("\nBXVD_DIP_P_SampleTime too many samples\n");
   }
   else
   {
#if (BCHP_CHIP == 7445)
      BDBG_ERR(("NEED TO ADD STC Read Macro in BXVD_DIP_P_SampleTime"));
#else
      if (hXvd->uDecoderInstance == 0)
      {
         BXVD_GetSTCSource(hCh, &eStc);
         if (BXVD_STC_eZero == eStc  )
         {
            uiTemp = BREG_Read32(hXvd->hReg, BCHP_DECODE_IP_SHIM_0_STC0_REG);
         }
         else
         {
            uiTemp = BREG_Read32(hXvd->hReg, BCHP_DECODE_IP_SHIM_0_STC1_REG);
         }
      }
      else
#endif
      {
         uiTemp = 0xFFFFFFFF;
      }

      gauiTimeSample[guiSampleIndex] = uiTemp;
      guiSampleIndex++;
   }
}

static void BXVD_DIP_S_PrintSampleTimes(void *pXvdDipCh, uint32_t uiStcFromDecoder)
{
#if LATENCY_FILE_OUTPUT
   FILE *outfd;
#endif
   int32_t iNum45kTicks;
   int32_t iUsecs;

   BSTD_UNUSED(pXvdDipCh);

#if LATENCY_FILE_OUTPUT
   outfd = fopen("timeresults.txt", "a+");
   if (outfd == NULL)
   {
      BKNI_Printf("BXVD_DIP_S_PrintSampleTimes is unable to open output file\n");
      return;
   }
#endif
   /* Sampled STC, AVD STC snapshot and latency between the two. */
   iNum45kTicks = (int)gauiTimeSample[0] - (int)uiStcFromDecoder;
   iUsecs = ( iNum45kTicks * 1000 * 1000 ) / 45000;

#if !LATENCY_FILE_OUTPUT
   BKNI_Printf("%u,%u,%d,",
         uiStcFromDecoder,
         gauiTimeSample[0],
         iUsecs
         );
#else
   fprintf(outfd, "%u,%u,%d,", uiStcFromDecoder, gauiTimeSample[0], iUsecs);
#endif

   /* The elapse time for the DM ISR routine. */
   iNum45kTicks = gauiTimeSample[2] - gauiTimeSample[0];
   iUsecs = ( iNum45kTicks * 1000 * 1000 ) / 45000;
#if !LATENCY_FILE_OUTPUT
   BKNI_Printf("%u,", iUsecs );
#else
   fprintf(outfd, "%u,", iUsecs );
#endif
   /* Execution time for callbacks. */
   iNum45kTicks = gauiTimeSample[2] - gauiTimeSample[1];
   iUsecs = ( iNum45kTicks * 1000 * 1000 ) / 45000;
#if !LATENCY_FILE_OUTPUT
   BKNI_Printf("%u ", iUsecs );
   BKNI_Printf("\n");
#else
   fprintf(outfd, "%u\n", iUsecs );
   fflush(outfd);
   fclose(outfd);
#endif
}
#endif

BERR_Code
BXVD_DisplayInterruptProvider_P_GetDefaultChannelSettings(
         BXVD_DisplayInterruptProvider_P_ChannelSettings *pstXvdDipChSettings
         )
{
   BDBG_ENTER( BXVD_DisplayInterruptProvider_P_GetDefaultChannelSettings );

   BDBG_ASSERT( pstXvdDipChSettings );

   BKNI_Memset( pstXvdDipChSettings, 0, sizeof( BXVD_DisplayInterruptProvider_P_ChannelSettings ) );

   BDBG_LEAVE( BXVD_DisplayInterruptProvider_P_GetDefaultChannelSettings );
   return BERR_TRACE(BERR_SUCCESS);
}

void
BXVD_DisplayInterruptProvider_S_PictureDataReady_isr(
         void* pXvdDipCh,
         int iParam2
         )
{
   BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh = (BXVD_DisplayInterruptProvider_P_ChannelHandle) pXvdDipCh;

   BXVD_P_DisplayInfo *displayInfo = &hXvdDipCh->functionData.BXVD_DisplayInterruptProvider_S_PictureDataReady_isr.stDisplayInfo;
   BXVD_P_DisplayInfo *updateDisplayInfo = &hXvdDipCh->functionData.BXVD_DisplayInterruptProvider_S_PictureDataReady_isr.stUpdateDisplayInfo;

   uint32_t i;

#if BDBG_DEBUG_BUILD
   BXVD_DisplayInterrupt eDisplayInterrupt = ( BXVD_DisplayInterrupt ) iParam2;
#else
   BSTD_UNUSED(iParam2);
#endif

   BDBG_ASSERT( hXvdDipCh );

#if BDBG_DEBUG_BUILD
   BDBG_ASSERT( eDisplayInterrupt == hXvdDipCh->stChannelSettings.eDisplayInterrupt );
#endif

#if BXVD_DIP_P_MEASURE_LATENCY
   guiSampleIndex = 0;
   BKNI_Memset( gauiTimeSample, 0, (sizeof( uint32_t) * BXVD_DIP_MAX_TIME_SAMPLES) );

   BXVD_DIP_P_SampleTime(hXvdDipCh);
#endif

   /*************************************/
   /* Update the Display Interrupt Info */
   /*************************************/

   for ( i = 0; i < 2; i++ )
   {
      BXVD_P_READ_DISPLAY_INFO(hXvdDipCh, (*displayInfo));

      /* Extract Interrupt Polarity */
      BXVD_DIP_TEST_MULTI_INTERRUPT_BITS_SET((*displayInfo), hXvdDipCh->stInterruptSettings);

      if (BXVD_DIP_IS_POLARITY_BOTTOM((*displayInfo), hXvdDipCh->stInterruptSettings))
      {
         hXvdDipCh->stDisplayInterruptInfo.eInterruptPolarity = BAVC_Polarity_eBotField ;
      }
      else if (BXVD_DIP_IS_POLARITY_TOP((*displayInfo), hXvdDipCh->stInterruptSettings))
      {
         hXvdDipCh->stDisplayInterruptInfo.eInterruptPolarity = BAVC_Polarity_eTopField ;
      }
      else if (BXVD_DIP_IS_POLARITY_FRAME((*displayInfo), hXvdDipCh->stInterruptSettings))
      {
         hXvdDipCh->stDisplayInterruptInfo.eInterruptPolarity = BAVC_Polarity_eFrame ;
      }
      else
      {
         /* We default to TopField for the vsync polarity if we don't
          * recognize the polarity type */
         BXVD_DIP_DBG_WRN_BAD_VSYNC((*displayInfo));

         hXvdDipCh->stDisplayInterruptInfo.eInterruptPolarity = BAVC_Polarity_eTopField ;
      }

      /* Verify that the AVD FW hasn't updated the STC and Vsync Parity, meaning something is keeping the
         PDR ISR from being serviced in a timely manner */

      BXVD_P_READ_DISPLAY_INFO(hXvdDipCh, (*updateDisplayInfo));

      if (!BXVD_P_IS_DISPLAY_INFO_EQUAL((*updateDisplayInfo), (*displayInfo)))
      {
#if !B_REFSW_SYSTEM_MODE_CLIENT
         BDBG_ERR(("AVD Picture Data Ready interrupt not processed in time!"));
#else
         BDBG_MSG(("AVD Picture Data Ready interrupt not processed in time!"));
#endif
      }
      else
      {
         break;
      }
   }

   /* Extract STC snapshot(s) */
   BDBG_ASSERT( BXVD_P_STC_MAX == hXvdDipCh->stDisplayInterruptInfo.uiSTCCount );

   BXVD_P_SAVE_DIP_INFO_STC(hXvdDipCh, (*displayInfo));


#if defined(BCHP_XPT_PCROFFSET_STC_SNAPSHOT0) || defined(BCHP_OTT_XPT_TSM_STC_SNAPSHOT0)
   {
      /*
       * SWSTB-3955: STC's read directly from XPT.
       */
      BXVD_Handle hXvd;
      BXVD_ChannelHandle hXvdCh;
      BXDM_PictureProvider_DisplayMode eXDMDisplayMode;

      for ( i=0; i < hXvdDipCh->stDisplayInterruptInfo.uiSTCCount; i++ )
      {
         hXvdDipCh->stDisplayInterruptInfo.astSTCFromXPT[i].bValid = false;
      }

      /* Walk through the list of XVD channels.  Only read
       * the STC snapshot for the active channels. */

      hXvd = hXvdDipCh->stChannelSettings.hXvd;

      if ( hXvd )
      {
         for ( i=0; i < BXVD_MAX_VIDEO_CHANNELS; i++ )
         {
            uint32_t uiOffset = 0;

            hXvdCh = hXvd->ahChannel[i];

            if ( hXvdCh && hXvdCh->eDecoderState == BXVD_P_DecoderState_eActive )
            {
               BXDM_PictureProvider_GetDisplayMode_isr( hXvdCh->hPictureProvider, &eXDMDisplayMode);

               if ( eXDMDisplayMode == BXDM_PictureProvider_DisplayMode_eTSM )
               {
                  switch( hXvdCh->sDecodeSettings.eSTC )
                  {
#ifdef BCHP_XPT_PCROFFSET_STC_SNAPSHOT0
                     case 0: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT0; break;
                     case 1: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT1; break;
                     case 2: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT2; break;
                     case 3: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT3; break;
#ifdef BCHP_XPT_PCROFFSET_STC_SNAPSHOT4
                     case 4: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT4; break;
#ifdef BCHP_XPT_PCROFFSET_STC_SNAPSHOT5
                     case 5: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT5; break;
#ifdef BCHP_XPT_PCROFFSET_STC_SNAPSHOT6
                     case 6: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT6; break;
                     case 7: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT7; break;
#ifdef BCHP_XPT_PCROFFSET_STC_SNAPSHOT8
                     case 8: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT8; break;
                     case 9: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT9; break;
                     case 10: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT10; break;
                     case 11: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT11; break;
#ifdef BCHP_XPT_PCROFFSET_STC_SNAPSHOT12
                     case 12: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT12; break;
                     case 13: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT13; break;
                     case 14: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT14; break;
                     case 15: uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT15; break;
#endif
#endif
#endif
#endif
#endif
                     default:
                        uiOffset = BCHP_XPT_PCROFFSET_STC_SNAPSHOT0;
                        BERR_TRACE(BERR_INVALID_PARAMETER);
                        break;
#else
#ifdef BCHP_OTT_XPT_TSM_STC_SNAPSHOT0
                     case 0: uiOffset = BCHP_OTT_XPT_TSM_STC_SNAPSHOT0; break;
                     case 1: uiOffset = BCHP_OTT_XPT_TSM_STC_SNAPSHOT1; break;
                     case 2: uiOffset = BCHP_OTT_XPT_TSM_STC_SNAPSHOT2; break;
                     case 3: uiOffset = BCHP_OTT_XPT_TSM_STC_SNAPSHOT3; break;
                     case 4: uiOffset = BCHP_OTT_XPT_TSM_STC_SNAPSHOT4; break;
                     case 5: uiOffset = BCHP_OTT_XPT_TSM_STC_SNAPSHOT5; break;
#endif
                     default:
                        uiOffset = BCHP_OTT_XPT_TSM_STC_SNAPSHOT0;
                        BERR_TRACE(BERR_INVALID_PARAMETER);
                        break;
#endif
                  }

                  hXvdDipCh->stDisplayInterruptInfo.astSTCFromXPT[hXvdCh->sDecodeSettings.eSTC].uiValue = BXVD_Reg_Read32_isr( hXvdDipCh->stChannelSettings.hXvd, uiOffset );
                  hXvdDipCh->stDisplayInterruptInfo.astSTCFromXPT[hXvdCh->sDecodeSettings.eSTC].bValid = true;
               }
            }
         }
      }
   }
#endif

#if 0
   /* Enable to see the STCs that are being captured by FW */
   {
      int ii;

      for (ii=0; ii< BXVD_P_STC_MAX; ii++)
      {
         BKNI_Printf("stc %d, %0x\n", ii, stDisplayInfo.stc_snapshot[ii]);
      }
   }
#endif

   /* Increment Interrupt Count */
   hXvdDipCh->stDisplayInterruptInfo.stInterruptCount.uiValue++;
   hXvdDipCh->stDisplayInterruptInfo.stInterruptCount.bValid = true;

   /* Execute the application's Picture Data Ready callback */
   if( NULL != hXvdDipCh->stDisplayInterruptHandler.fCallback )
   {
#if BXVD_DIP_P_MEASURE_LATENCY
      BXVD_DIP_P_SampleTime(hXvdDipCh);
#endif

#if BDBG_DEBUG_BUILD
      {
         /*
          * If enabled, print out a debug message every vsync.
          */
         char cPolarity;
         switch( hXvdDipCh->stDisplayInterruptInfo.eInterruptPolarity )
         {
            case BAVC_Polarity_eTopField:    cPolarity='T';    break;
            case BAVC_Polarity_eBotField:    cPolarity='B';    break;
            case BAVC_Polarity_eFrame:       cPolarity='F';    break;
            default:                         cPolarity='u';    break;
         }

         BDBG_MODULE_MSG( BXVD_DIPDBG, ("0x%0*lx %d.%d %c stc:%08x(%d) %d",
                  BXVD_P_DIGITS_IN_LONG, (long)hXvdDipCh,
                  hXvdDipCh->stChannelSettings.hXvd->uDecoderInstance,
                  hXvdDipCh->stChannelSettings.eDisplayInterrupt,
                  cPolarity,
                  hXvdDipCh->stDisplayInterruptInfo.astSTC[0].uiValue,
                  hXvdDipCh->stDisplayInterruptInfo.astSTC[0].bValid,
                  hXvdDipCh->stDisplayInterruptInfo.stInterruptCount.uiValue
                  ));
      }
#endif

      hXvdDipCh->stDisplayInterruptHandler.fCallback(
               hXvdDipCh->stDisplayInterruptHandler.pPrivateContext,
               &hXvdDipCh->stDisplayInterruptInfo
               );
#if BXVD_DIP_P_MEASURE_LATENCY
      BXVD_DIP_P_SampleTime(hXvdDipCh);
#endif
   }

#if BXVD_DIP_P_MEASURE_LATENCY
#if !BXVD_P_FW_HIM_API

   BXVD_DIP_S_PrintSampleTimes(hXvdDipCh, stDisplayInfo.stc_snapshot);
#else
   BXVD_DIP_S_PrintSampleTimes(hXvdDipCh, stDisplayInfo.stc_snapshot[0]);
#endif

#endif

   return;
}

static BERR_Code
BXVD_DisplayInterruptProvider_S_SetupInterrupts(
         BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh
         )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_DisplayInterruptProvider_S_SetupInterrupts );

   BDBG_ASSERT( hXvdDipCh );

#if !BXVD_POLL_FW_MBX
   if ( hXvdDipCh->hPictureDataReadyInterruptCallback ) {
      rc = BINT_DisableCallback( hXvdDipCh->hPictureDataReadyInterruptCallback );
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   } else {
      rc = BINT_CreateCallback(
               &hXvdDipCh->hPictureDataReadyInterruptCallback,
               hXvdDipCh->stChannelSettings.hInterrupt,
               hXvdDipCh->stChannelSettings.interruptId,
               BXVD_DisplayInterruptProvider_S_PictureDataReady_isr,
               ( void* ) hXvdDipCh,
               hXvdDipCh->stChannelSettings.eDisplayInterrupt
               );

      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   rc = BINT_EnableCallback( hXvdDipCh->hPictureDataReadyInterruptCallback );
   if (rc != BERR_SUCCESS )
   {
      return BERR_TRACE(rc);
   }
#endif
   BDBG_LEAVE( BXVD_DisplayInterruptProvider_S_SetupInterrupts );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code BXVD_DisplayInterruptProvider_P_EnableInterrupts(
   BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER( BXVD_DisplayInterruptProvider_P_EnableInterrupts );

   BDBG_ASSERT( hXvdDipCh );

   if ( hXvdDipCh->hPictureDataReadyInterruptCallback )
   {
      rc = BINT_EnableCallback( hXvdDipCh->hPictureDataReadyInterruptCallback );
   }

   BDBG_LEAVE( BXVD_DisplayInterruptProvider_P_SetupInterrupts );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_DisplayInterruptProvider_P_DisableInterrupts(
   BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER( BXVD_DisplayInterruptProvider_P_DisaableInterrupts );

   BDBG_ASSERT( hXvdDipCh );

   if ( hXvdDipCh->hPictureDataReadyInterruptCallback )
   {
      rc = BINT_DisableCallback( hXvdDipCh->hPictureDataReadyInterruptCallback );
   }

   BDBG_LEAVE( BXVD_DisplayInterruptProvider_P_DisableInterrupts );

   return BERR_TRACE( rc );
}

static BERR_Code
BXVD_DisplayInterruptProvider_S_TeardownInterrupts(
         BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh
         )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_DisplayInterruptProvider_S_TeardownInterrupts );

   BDBG_ASSERT( hXvdDipCh );

   if ( hXvdDipCh->hPictureDataReadyInterruptCallback ) {
      rc = BINT_DisableCallback( hXvdDipCh->hPictureDataReadyInterruptCallback );
      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

      rc = BINT_DestroyCallback( hXvdDipCh->hPictureDataReadyInterruptCallback );
      if (rc != BERR_SUCCESS )
      {
         return BERR_TRACE(rc);
      }

      hXvdDipCh->hPictureDataReadyInterruptCallback = NULL;
   }

   BDBG_LEAVE( BXVD_DisplayInterruptProvider_S_TeardownInterrupts );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BXVD_DisplayInterruptProvider_P_OpenChannel(
         BXVD_DisplayInterruptProvider_P_ChannelHandle *phXvdDipCh,
         const BXVD_DisplayInterruptProvider_P_ChannelSettings *pstXvdDipChSettings
         )
{
   BERR_Code rc;
   BXVD_DisplayInterruptProvider_P_ChannelContext *pXvdDipCh = NULL;

   BDBG_ENTER( BXVD_DisplayInterruptProvider_P_OpenChannel );

   BDBG_ASSERT( phXvdDipCh );
   BDBG_ASSERT( pstXvdDipChSettings );

   /* Set the handle to NULL in case the allocation fails */
   *phXvdDipCh = NULL;

   pXvdDipCh = ( BXVD_DisplayInterruptProvider_P_ChannelContext* ) BKNI_Malloc( sizeof( BXVD_DisplayInterruptProvider_P_ChannelContext ) );
   if ( NULL == pXvdDipCh )
   {
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }

   /* Zero out the newly allocated context */
   BKNI_Memset( ( void * ) pXvdDipCh, 0, sizeof( BXVD_DisplayInterruptProvider_P_ChannelContext ) );

   /* Allocate STC snapshot array */
   pXvdDipCh->stDisplayInterruptInfo.uiSTCCount = BXVD_P_STC_MAX;
   pXvdDipCh->stDisplayInterruptInfo.astSTC = ( BXDM_QualifiedValue * ) BKNI_Malloc( sizeof ( BXDM_QualifiedValue ) * pXvdDipCh->stDisplayInterruptInfo.uiSTCCount );
   if ( NULL == pXvdDipCh->stDisplayInterruptInfo.astSTC )
   {
      BXVD_DisplayInterruptProvider_P_CloseChannel( pXvdDipCh );
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }


   /* SWSTB-3955: STC's read directly from XPT. */

   pXvdDipCh->stDisplayInterruptInfo.astSTCFromXPT = ( BXDM_QualifiedValue * ) BKNI_Malloc( sizeof ( BXDM_QualifiedValue ) * pXvdDipCh->stDisplayInterruptInfo.uiSTCCount );
   if ( NULL == pXvdDipCh->stDisplayInterruptInfo.astSTCFromXPT )
   {
      BXVD_DisplayInterruptProvider_P_CloseChannel( pXvdDipCh );
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }
   else
   {
      BKNI_Memset( pXvdDipCh->stDisplayInterruptInfo.astSTCFromXPT, 0, sizeof( BXDM_QualifiedValue ) * pXvdDipCh->stDisplayInterruptInfo.uiSTCCount );
   }

   /* TODO: Validate the channel settings */
   pXvdDipCh->stChannelSettings = *pstXvdDipChSettings;

   /* Install Picture Data Ready Callback Handler */
   rc = BXVD_DisplayInterruptProvider_S_SetupInterrupts( pXvdDipCh );
   if ( BERR_SUCCESS != rc )
   {
      BXVD_DisplayInterruptProvider_P_CloseChannel( pXvdDipCh );
      return BERR_TRACE(rc);
   }

   *phXvdDipCh = pXvdDipCh;

   BDBG_MODULE_MSG( BXVD_DIPCTL, ("Create: hDip:0x%0*lx Decoder:%d hXvd:0x%0*lx eDisplayInterrupt:%d interruptId:%08x",
                  BXVD_P_DIGITS_IN_LONG, (long)pXvdDipCh,
                  pXvdDipCh->stChannelSettings.hXvd->uDecoderInstance,
                  BXVD_P_DIGITS_IN_LONG, (long)pXvdDipCh->stChannelSettings.hXvd,
                  pXvdDipCh->stChannelSettings.eDisplayInterrupt,
                  pXvdDipCh->stChannelSettings.interruptId
                  ));

   BDBG_LEAVE( BXVD_DisplayInterruptProvider_P_OpenChannel );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BXVD_DisplayInterruptProvider_P_CloseChannel(
         BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh
         )
{
   BDBG_ENTER( BXVD_DisplayInterruptProvider_P_CloseChannel );

   if ( NULL != hXvdDipCh )
   {
      BDBG_MODULE_MSG( BXVD_DIPCTL, ("Destroy: h0x%0*lx Decoder:%d",
                           BXVD_P_DIGITS_IN_LONG, (long)hXvdDipCh,
                           hXvdDipCh->stChannelSettings.hXvd->uDecoderInstance ));

      BXVD_DisplayInterruptProvider_S_TeardownInterrupts( hXvdDipCh );

      BKNI_Free( hXvdDipCh->stDisplayInterruptInfo.astSTC );

      /* SWSTB-3955: STC's read directly from XPT. */
      if ( hXvdDipCh->stDisplayInterruptInfo.astSTCFromXPT )
         BKNI_Free( hXvdDipCh->stDisplayInterruptInfo.astSTCFromXPT );

      BKNI_Free( hXvdDipCh );
   }

   BDBG_LEAVE( BXVD_DisplayInterruptProvider_P_CloseChannel );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BXVD_DisplayInterruptProvider_P_ProcessWatchdog(
         BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh
         )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_DisplayInterruptProvider_P_ProcessWatchdog );

   BDBG_ASSERT( hXvdDipCh );

   rc = BXVD_DisplayInterruptProvider_S_SetupInterrupts( hXvdDipCh );
   if ( BERR_SUCCESS != rc )
   {
      return BERR_TRACE( rc );
   }

   if ( true == hXvdDipCh->bInterruptSettingsValid )
   {
      /* clear pending BVNF interrupts since they are edge-triggered */
      BREG_Write32(hXvdDipCh->stChannelSettings.hRegister,
                   hXvdDipCh->stChannelSettings.uiInterruptClearRegister,
                   hXvdDipCh->uiRegMaskCurrent_0);

      /* Restore mask register */
      BREG_Write32(hXvdDipCh->stChannelSettings.hRegister,
                   hXvdDipCh->stChannelSettings.uiInterruptMaskRegister,
                   hXvdDipCh->uiRegMaskCurrent_0);

      if ( hXvdDipCh->stChannelSettings.uiInterruptClearRegister_1 != 0)
      {
         /* clear pending BVNF interrupts since they are edge-triggered */
         BREG_Write32(hXvdDipCh->stChannelSettings.hRegister,
                      hXvdDipCh->stChannelSettings.uiInterruptClearRegister_1,
                      hXvdDipCh->uiRegMaskCurrent_1);

         BREG_Write32(hXvdDipCh->stChannelSettings.hRegister,
                      hXvdDipCh->stChannelSettings.uiInterruptMaskRegister_1,
                      hXvdDipCh->uiRegMaskCurrent_1);
      }

      rc = BXVD_P_HostCmdSendConfig(hXvdDipCh->stChannelSettings.hXvd,
                                    hXvdDipCh->stChannelSettings.eDisplayInterrupt,
                                    hXvdDipCh->uiRegMaskCurrent_0,
                                    hXvdDipCh->uiRegMaskCurrent_1);
      if ( BERR_SUCCESS != rc )
      {
         return BERR_TRACE( rc );
      }
   }

   BDBG_LEAVE( BXVD_DisplayInterruptProvider_P_ProcessWatchdog );

   return BERR_TRACE( BERR_SUCCESS );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXVD_DisplayInterruptProvider_P_GetDefaultInterruptSettings(
         BXVD_DisplayInterruptProvider_P_InterruptSettings *pstXvdDipIntSettings
         )
{
   BDBG_ENTER( BXVD_DisplayInterruptProvider_P_GetDefaultInterruptSettings );

   BDBG_ASSERT( pstXvdDipIntSettings );

   BKNI_Memset( pstXvdDipIntSettings, 0, sizeof( BXVD_DisplayInterruptProvider_P_InterruptSettings ) );

   BDBG_LEAVE( BXVD_DisplayInterruptProvider_P_GetDefaultInterruptSettings );

   return BERR_TRACE(BERR_SUCCESS);
}
#endif

BERR_Code
BXVD_DisplayInterruptProvider_P_SetInterruptConfiguration(
         BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh,
         const BXVD_DisplayInterruptProvider_P_InterruptSettings *pstXvdDipIntSettings
         )
{
   BERR_Code rc;

   uint32_t uiRegMaskNew_0, uiRegMaskNew_1;

   BDBG_ENTER( BXVD_DisplayInterruptProvider_P_SetInterruptConfiguration );

   BDBG_ASSERT( hXvdDipCh );
   BDBG_ASSERT( pstXvdDipIntSettings );

   uiRegMaskNew_0 = ( pstXvdDipIntSettings->stRULIDMasks_0.ui32ProgressiveFieldRULIDMask |
                    pstXvdDipIntSettings->stRULIDMasks_0.ui32TopFieldRULIDMask |
                    pstXvdDipIntSettings->stRULIDMasks_0.ui32BottomFieldRULIDMask );

   uiRegMaskNew_1 = ( pstXvdDipIntSettings->stRULIDMasks_1.ui32ProgressiveFieldRULIDMask |
                    pstXvdDipIntSettings->stRULIDMasks_1.ui32TopFieldRULIDMask |
                    pstXvdDipIntSettings->stRULIDMasks_1.ui32BottomFieldRULIDMask );

   if ( ( false == hXvdDipCh->bInterruptSettingsValid )
        || ( hXvdDipCh->uiRegMaskCurrent_0 != uiRegMaskNew_0 )
        || ( hXvdDipCh->uiRegMaskCurrent_1 != uiRegMaskNew_1 ))
   {
      hXvdDipCh->bInterruptSettingsValid = true;
      hXvdDipCh->stInterruptSettings = *pstXvdDipIntSettings;
      hXvdDipCh->uiRegMaskCurrent_0 = uiRegMaskNew_0;
      hXvdDipCh->uiRegMaskCurrent_1 = uiRegMaskNew_1;

      /* clear pending BVNF interrupts since they are edge-triggered */
      BREG_Write32(hXvdDipCh->stChannelSettings.hRegister,
                   hXvdDipCh->stChannelSettings.uiInterruptClearRegister,
                   hXvdDipCh->uiRegMaskCurrent_0);

      BREG_Write32(hXvdDipCh->stChannelSettings.hRegister,
                   hXvdDipCh->stChannelSettings.uiInterruptMaskRegister,
                   hXvdDipCh->uiRegMaskCurrent_0);

      BDBG_MODULE_MSG( BXVD_DIPCTL, ("hDip:0x%0*lx SetInterrupt Decoder:%d hXvd:0x%0*lx eDisplayInterrupt:%d mask0:%08x mask1:%08x",
                  BXVD_P_DIGITS_IN_LONG, (long)hXvdDipCh,
                  hXvdDipCh->stChannelSettings.hXvd->uDecoderInstance,
                  BXVD_P_DIGITS_IN_LONG, (long)hXvdDipCh->stChannelSettings.hXvd,
                  hXvdDipCh->stChannelSettings.eDisplayInterrupt,
                  hXvdDipCh->uiRegMaskCurrent_0,
                  hXvdDipCh->uiRegMaskCurrent_0));

      if ( hXvdDipCh->stChannelSettings.uiInterruptClearRegister_1 != 0 )
      {
         /* clear pending BVNF interrupts since they are edge-triggered */
         BREG_Write32(hXvdDipCh->stChannelSettings.hRegister,
                      hXvdDipCh->stChannelSettings.uiInterruptClearRegister_1,
                      hXvdDipCh->uiRegMaskCurrent_1);

         BREG_Write32(hXvdDipCh->stChannelSettings.hRegister,
                      hXvdDipCh->stChannelSettings.uiInterruptMaskRegister_1,
                      hXvdDipCh->uiRegMaskCurrent_1);
      }

      rc = BXVD_P_HostCmdSendConfig(hXvdDipCh->stChannelSettings.hXvd,
                                    hXvdDipCh->stChannelSettings.eDisplayInterrupt,
                                    hXvdDipCh->uiRegMaskCurrent_0,
                                    hXvdDipCh->uiRegMaskCurrent_1);
      if ( BERR_SUCCESS != rc )
      {
         return BERR_TRACE( rc );
      }
   }

   BDBG_LEAVE( BXVD_DisplayInterruptProvider_P_SetInterruptConfiguration );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BXVD_DisplayInterruptProvider_P_GetInterruptConfiguration(
         BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh,
         BXVD_DisplayInterruptProvider_P_InterruptSettings *pstXvdDipIntSettings
         )
{
   BDBG_ENTER( BXVD_DisplayInterruptProvider_P_GetInterruptConfiguration );

   BDBG_ASSERT( hXvdDipCh );
   BDBG_ASSERT( pstXvdDipIntSettings );

   *pstXvdDipIntSettings = hXvdDipCh->stInterruptSettings;

   BDBG_LEAVE( BXVD_DisplayInterruptProvider_P_GetInterruptConfiguration );
   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code
BXVD_DisplayInterruptProvider_InstallCallback_DisplayInterrupt(
         BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh,
         BXDM_DisplayInterruptHandler_isr fCallback_isr,
         void *pPrivateContext
         )
{
   BDBG_ENTER( BXVD_DisplayInterruptProvider_InstallCallback_DisplayInterrupt );

   BDBG_ASSERT( hXvdDipCh );

   hXvdDipCh->stDisplayInterruptHandler.fCallback = fCallback_isr;
   hXvdDipCh->stDisplayInterruptHandler.pPrivateContext = pPrivateContext;

   BDBG_MODULE_MSG( BXVD_DIPCTL, ("hDip:0x%0*lx Install DIH Decoder:%d hXvd:0x%0*lx eDisplayInterrupt:%d hDih:0x%0*lx cb:0x%0*lx",
                  BXVD_P_DIGITS_IN_LONG, (long)hXvdDipCh,
                  hXvdDipCh->stChannelSettings.hXvd->uDecoderInstance,
                  BXVD_P_DIGITS_IN_LONG, (long)hXvdDipCh->stChannelSettings.hXvd,
                  hXvdDipCh->stChannelSettings.eDisplayInterrupt,
                  BXVD_P_DIGITS_IN_LONG, (long)hXvdDipCh->stDisplayInterruptHandler.pPrivateContext,
                  BXVD_P_DIGITS_IN_LONG, (long)hXvdDipCh->stDisplayInterruptHandler.fCallback
                  ));

   BDBG_LEAVE( BXVD_DisplayInterruptProvider_InstallCallback_DisplayInterrupt );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BXVD_DisplayInterruptProvider_GetDisplayInterruptInfo_isr(
         BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh,
         BXDM_DisplayInterruptInfo *pstXvdDisplayInterruptInfo
         )
{
   BDBG_ENTER( BXVD_DisplayInterruptProvider_GetDisplayInterruptInfo_isr );

   BDBG_ASSERT( hXvdDipCh );
   BDBG_ASSERT( pstXvdDisplayInterruptInfo );

   *pstXvdDisplayInterruptInfo = hXvdDipCh->stDisplayInterruptInfo;

   BDBG_LEAVE( BXVD_DisplayInterruptProvider_GetDisplayInterruptInfo_isr );

   return BERR_TRACE( BERR_SUCCESS );
}
