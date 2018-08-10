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
 *   See code
 *
 ***************************************************************************/
#include "bstd.h"                /* standard types */
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"                /* malloc */
#include "btmr.h"
#include "bxvd_platform.h"
#include "bxvd.h"
#include "bavc.h"
#include "bxvd_priv.h"
#include "bxvd_intr.h"
#include "bxvd_reg.h"
#include "bxvd_decoder.h"

#if BXVD_P_FW_DEBUG_DRAM_LOGGING
#include "bxvd_dbg.h"

#define BXVD_P_LOG_ENTRY_SIZE 80

static char LogBuf[BXVD_P_LOG_ENTRY_SIZE+4];
#endif

BDBG_MODULE(BXVD_INTR);

static void BXVD_S_SetAspectRatio_isr(
   BXVD_P_PPB* pPPB,
   BXVD_StillPictureBuffers *pStillPicBuf
   )
{
   BXDM_Picture_AspectRatio aspectRatio;

   BDBG_ENTER(BXVD_S_SetAspectRatio_isr);

   BXVD_Decoder_P_ComputeAspectRatio_isr( pPPB, &aspectRatio);

   pStillPicBuf->eAspectRatio = aspectRatio.eAspectRatio;
   pStillPicBuf->uiSampleAspectRatioX = aspectRatio.uiSampleAspectRatioX;
   pStillPicBuf->uiSampleAspectRatioY = aspectRatio.uiSampleAspectRatioY;

   if ( false == aspectRatio.bValid )
   {
      BDBG_MSG(("Unknown protocol/aspect ratio: %d/%#x", pPPB->protocol, pPPB->aspect_ratio));
   }

   BDBG_LEAVE(BXVD_S_SetAspectRatio_isr);
   return;

} /* end of BXVD_S_SetAspectRatio() */


void BXVD_P_AVD_MBX_isr(void *pvXvd, int iParam2)
{
   BXVD_DecoderContext  *pDecCntx = pvXvd;

   BDBG_ENTER(BXVD_P_AVD_MBX_isr);
   BSTD_UNUSED(iParam2);

   BXVD_DBG_MSG(pDecCntx->hXvd, ("MBX_isr"));

   BKNI_SetEvent(pDecCntx->hFWCmdDoneEvent);

   BDBG_LEAVE(BXVD_P_AVD_MBX_isr);
}

void BXVD_P_AVD_StillPictureRdy_isr(void *pvXvd, int iParam2)
{
   BXVD_DecoderContext   *pDecCntx = pvXvd;
   BXVD_Handle   hXvd = pDecCntx->hXvd;
   BXVD_ChannelHandle hXvdCh = NULL;
   struct BXVD_P_InterruptCallbackInfo *pCallback;
   BXVD_P_PictureDeliveryQueue *pPictureDeliveryQueue;
   BXVD_P_PPB *pPPB = NULL;
   BXVD_StillPictureBuffers stAppStillPictBuffers;
   BXVD_P_PHY_ADDR pDisplayElementOffsetTemp;

#if BXVD_P_PPB_EXTENDED
   uint32_t  uiFlagsExt0;
#endif

   if (hXvd->bStillChannelAllocated)
   {
      /* Find the still channel handle */
      hXvdCh = hXvd->ahChannel[hXvd->uiStillChannelNum];
   }

   BDBG_ENTER(BXVD_P_AVD_StillPictureRdy_isr);
   BSTD_UNUSED(iParam2);

   if (hXvdCh != NULL)
   {
      uint32_t uiDeliveryQReadOffset;

      BXVD_P_DELIVERY_QUEUE_GET_ADDR( hXvdCh, pPictureDeliveryQueue );
      BXVD_P_DELIVERY_QUEUE_GET_READ_OFFSET( hXvdCh, uiDeliveryQReadOffset );

#if BXVD_P_FW_HIM_API
      BMMA_FlushCache(hXvdCh->hFWGenMemBlock, (void *)pPictureDeliveryQueue, sizeof(BXVD_P_PictureDeliveryQueue));
#endif

      /* Get picture off of delivery queue */
      BDBG_ASSERT(hXvdCh->bStillPictureToRelease == false);

      /* Decoder stops after still picture is decoded */
      hXvdCh->eDecoderState = BXVD_P_DecoderState_eNotActive;

      hXvd->auiActiveDecodes[hXvdCh->sChSettings.eChannelMode][hXvdCh->eDisplayInterrupt]--;

      hXvdCh->bStillPictureToRelease = true;
      hXvdCh->uiStillDisplayElementOffset = (uint32_t)pPictureDeliveryQueue->display_elements[uiDeliveryQReadOffset - BXVD_P_INITIAL_OFFSET_DISPLAY_QUEUE];

#if BXVD_P_CORE_40BIT_ADDRESSABLE
      pDisplayElementOffsetTemp = hXvdCh->uiStillDisplayElementOffset + hXvdCh->stDecodeFWBaseAddrs.FWContextBase;
#else
      pDisplayElementOffsetTemp = hXvdCh->uiStillDisplayElementOffset;
#endif

      pPPB = (BXVD_P_PPB *)BXVD_P_OFFSET_TO_VA(hXvdCh, pDisplayElementOffsetTemp);

      BMMA_FlushCache(hXvdCh->hFWGenMemBlock, (void *)pPPB, sizeof(BXVD_P_PPB));

      BXVD_P_INCREMENT_2BASED_OFFSET( uiDeliveryQReadOffset, 1 );

      BXVD_P_DELIVERY_QUEUE_SET_READ_OFFSET( hXvdCh, uiDeliveryQReadOffset );

      pCallback = &hXvd->stDeviceInterruptCallbackInfo[BXVD_DeviceInterrupt_eDecodedStillBufferReady];

      if (pCallback && pCallback->BXVD_P_pAppIntCallbackPtr)
      {
         switch (pPPB->protocol)
         {
            case BAVC_VideoCompressionStd_eMPEG2:
            case BAVC_VideoCompressionStd_eMPEG1:
            case BAVC_VideoCompressionStd_eMPEG2DTV:
            case BAVC_VideoCompressionStd_eMPEG2_DSS_PES:

               stAppStillPictBuffers.ulDisplayWidth = pPPB->other.mpeg.display_horizontal_size;
               stAppStillPictBuffers.ulDisplayHeight = pPPB->other.mpeg.display_vertical_size;
               break;

            case BAVC_VideoCompressionStd_eVC1:
            case BAVC_VideoCompressionStd_eVC1SimpleMain:

               stAppStillPictBuffers.ulDisplayWidth = pPPB->other.vc1.display_horizontal_size;
               stAppStillPictBuffers.ulDisplayHeight = pPPB->other.vc1.display_vertical_size;
               break;

            case BAVC_VideoCompressionStd_eVP8:

               stAppStillPictBuffers.ulDisplayWidth = pPPB->other.vp8.display_horizontal_size;
               stAppStillPictBuffers.ulDisplayHeight = pPPB->other.vp8.display_vertical_size;
               break;

            default:
               stAppStillPictBuffers.ulDisplayWidth = pPPB->video_width;
               stAppStillPictBuffers.ulDisplayHeight = pPPB->video_height;
               break;
         }

         stAppStillPictBuffers.ulImageWidth = pPPB->video_width;
         stAppStillPictBuffers.ulImageHeight = pPPB->video_height;

         switch (pPPB->flags & BXVD_P_PPB_FLAG_SOURCE_TYPE_MASK)
         {
            case BXVD_P_PPB_FLAG_PROGRESSIVE_SRC:
               stAppStillPictBuffers.eSourceType = BXVD_SourceType_eProgressive;
               break;
            case BXVD_P_PPB_FLAG_INTERLACED_SRC:
               stAppStillPictBuffers.eSourceType = BXVD_SourceType_eInterlaced;
               break;
            case BXVD_P_PPB_FLAG_UNKNOWN_SRC:
               stAppStillPictBuffers.eSourceType = BXVD_SourceType_eUnknown;
               break;
         }

         switch (pPPB->flags & BXVD_P_PPB_FLAG_BUFFER_TYPE_MASK)
         {
            case BXVD_P_PPB_FLAG_FRAME:
               stAppStillPictBuffers.eBufferType = BXVD_BufferType_eFrame;
               break;

            case BXVD_P_PPB_FLAG_FIELDPAIR:
               stAppStillPictBuffers.eBufferType = BXVD_BufferType_eFieldPair;
               break;

            case BXVD_P_PPB_FLAG_TOPFIELD:
               stAppStillPictBuffers.eBufferType = BXVD_BufferType_eTopField;
               break;

            case BXVD_P_PPB_FLAG_BOTTOMFIELD:
               stAppStillPictBuffers.eBufferType = BXVD_BufferType_eBotField;
               break;
         }

#if BXVD_P_PPB_EXTENDED
         uiFlagsExt0 = pPPB->flags_ext0;
         if ((uiFlagsExt0 & BXVD_P_PPB_EXT0_FLAG_LUMA_10_BIT_PICTURE) &&
             (uiFlagsExt0 & BXVD_P_PPB_EXT0_FLAG_CHROMA_10_BIT_PICTURE))
         {
            stAppStillPictBuffers.eBitDepth = BAVC_VideoBitDepth_e10Bit;
         }
         else
         {
            stAppStillPictBuffers.eBitDepth = BAVC_VideoBitDepth_e8Bit;
         }
#else
         stAppStillPictBuffers.eBitDepth = BAVC_VideoBitDepth_e8Bit;
#endif

         BXVD_DBG_MSG(hXvdCh, ("Still Pic: BitDepth %s", (stAppStillPictBuffers.eBitDepth ? "10" : "8")));

         if (stAppStillPictBuffers.ulDisplayWidth != 0)
         {
            BXVD_DBG_MSG(hXvdCh, ("Still Pic: Display Width: %ld, Display Height %ld",
                                  stAppStillPictBuffers.ulDisplayWidth,
                                  stAppStillPictBuffers.ulDisplayHeight));
         }

         BXVD_DBG_MSG(hXvdCh, ("Still Pic: Width: %d, Height %d", pPPB->video_width, pPPB->video_height));

         BXVD_DBG_MSG(hXvdCh, ("Still Pic: Source type: %d, Buffer type %d", (pPPB->flags & 0x0c), (pPPB->flags & 0x03)));

         stAppStillPictBuffers.hLuminanceFrameBufferBlock =  hXvdCh->hFWPicMemBlock;
         stAppStillPictBuffers.hChrominanceFrameBufferBlock = hXvdCh->hFWPicChromaMemBlock;

#if BXVD_P_CORE_40BIT_ADDRESSABLE
         stAppStillPictBuffers.ulLuminanceFrameBufferBlockOffset =
         ((BXVD_P_PHY_ADDR) (pPPB->luma_video_address) | ((BXVD_P_PHY_ADDR) pPPB->luma_video_address_hi << 32)) - hXvdCh->FWPicMemBasePhyAddr;

         stAppStillPictBuffers.ulChrominanceFrameBufferBlockOffset =
         ((BXVD_P_PHY_ADDR) (pPPB->chroma_video_address | ((BXVD_P_PHY_ADDR) pPPB->chroma_video_address_hi << 32))) - hXvdCh->FWPicChromaBasePhyAddr;
#else
         stAppStillPictBuffers.ulLuminanceFrameBufferBlockOffset = (uint32_t)(pPPB->luma_video_address) - hXvdCh->FWPicMemBasePhyAddr ;
         stAppStillPictBuffers.ulChrominanceFrameBufferBlockOffset = (uint32_t)(pPPB->chroma_video_address) - hXvdCh->FWPicChromaBasePhyAddr;
#endif

         BXVD_DBG_MSG(hXvdCh, ("PicBasePA: " BDBG_UINT64_FMT " PPB Luma: 0x%0x, Chroma: 0x%0x",
                               BDBG_UINT64_ARG((BMMA_DeviceOffset)hXvdCh->FWPicMemBasePhyAddr),
                               pPPB->luma_video_address, pPPB->chroma_video_address));

         BXVD_DBG_MSG(hXvdCh, ("LumaOffset:0x%0x ChromsOffset:0x%0x",
                               stAppStillPictBuffers.ulLuminanceFrameBufferBlockOffset, stAppStillPictBuffers.ulChrominanceFrameBufferBlockOffset));

         stAppStillPictBuffers.ulStripedWidth = BXVD_P_StripeWidthLUT[hXvd->uiDecode_StripeWidth];

         stAppStillPictBuffers.ulLumaStripedHeight = pPPB->luma_stripe_height;
         stAppStillPictBuffers.ulChromaStripedHeight = pPPB->chroma_stripe_height;

         BXVD_DBG_MSG(hXvdCh, ("Still Pic: LumaStripeHgt: %d, ChromaStripeHgt %d", pPPB->luma_stripe_height, pPPB->chroma_stripe_height));

         BXVD_S_SetAspectRatio_isr(pPPB, &stAppStillPictBuffers);

         /* Set decode error when error bit set in PPB */
         stAppStillPictBuffers.bDecodeError = ((pPPB->flags & BXVD_P_PPB_FLAG_DECODE_ERROR) == BXVD_P_PPB_FLAG_DECODE_ERROR);

         pCallback->BXVD_P_pAppIntCallbackPtr(pCallback->pParm1,
                                              pCallback->parm2,
                                              &stAppStillPictBuffers);

         /* The release of the still picture on the release queue is
          * delayed until the next still picture decode to ensure the
          * contents of this still picture remain intact. See
          * BXVD_StopDecode() for the release code */
      }
   }
   BDBG_LEAVE(BXVD_P_AVD_StillPictureRdy_isr);
}

void BXVD_P_WatchdogInterrupt_isr(void *pvXvd, int param2)
{
   BXVD_Handle hXvd = pvXvd;
   BXVD_ChannelHandle hXvdCh;

   uint32_t  chanNum;
   bool watchdog_fired;

   struct BXVD_P_InterruptCallbackInfo *pCallback;

   BDBG_ENTER(BXVD_P_WatchdogInterrupt_isr);
   BXVD_DBG_MSG(hXvd, ("BXVD Watchdog fired!!!"));

   if (hXvd != NULL)
   {
      watchdog_fired = BXVD_P_VERIFY_WATCHDOG_FIRED(hXvd, param2);

      if (watchdog_fired)
      {
         hXvd->eAVDBootMode = BXVD_AVDBootMode_eWatchdog;

         for ( chanNum = 0; chanNum < BXVD_MAX_VIDEO_CHANNELS; chanNum++)
         {
            hXvdCh = hXvd->ahChannel[chanNum];

            if (hXvdCh != NULL)
            {
               /* Only reset DM on Video channels, not Still Picture channel */
               if (hXvdCh->sChSettings.eChannelMode == BXVD_ChannelMode_eVideo)
               {
                  BXDM_PictureProvider_WatchdogReset_isr(hXvdCh->hPictureProvider);
               }
            }
         }

         hXvd->bWatchdogPending = true;

         /* Notify application if watchdog callback is registered */
         pCallback = &hXvd->stDeviceInterruptCallbackInfo[BXVD_DeviceInterrupt_eWatchdog];

         if (pCallback->BXVD_P_pAppIntCallbackPtr)
         {
            pCallback->BXVD_P_pAppIntCallbackPtr(pCallback->pParm1,
                                                 pCallback->parm2,
                                                 0);
         }
      }
   }
   BDBG_LEAVE(BXVD_P_WatchdogInterrupt_isr);
}


void BXVD_P_VidInstrChkr_isr(void *pvXvd, int param2)
{
   BXVD_Handle hXvd = pvXvd;

   struct BXVD_P_InterruptCallbackInfo *pCallback;

   BDBG_ENTER(BXVD_P_VidInstrChkr_isr);

   BXVD_DBG_MSG(hXvd, ("BXVD Video Instruction Checker triggered!!!"));

   if (hXvd != NULL)
   {
      pCallback = &hXvd->stDeviceInterruptCallbackInfo[BXVD_DeviceInterrupt_eVidInstrChecker];

      if (pCallback->BXVD_P_pAppIntCallbackPtr)
      {
         pCallback->BXVD_P_pAppIntCallbackPtr(pCallback->pParm1,
                                              pCallback->parm2,
                                              (void *)&param2);
      }
   }
   BDBG_LEAVE(BXVD_P_VidInstrChkr_isr);
}


void BXVD_P_StereoSeqError_isr(void *pvXvd,
                               int param2)
{
   BXVD_Handle hXvd = pvXvd;

   struct BXVD_P_InterruptCallbackInfo *pCallback;

   BDBG_ENTER(BXVD_P_StereoSeqError_isr);

   BXVD_DBG_MSG(hXvd, ("BXVD Stereo Sequence Error!!!"));

   if (hXvd != NULL)
   {
      pCallback = &hXvd->stDeviceInterruptCallbackInfo[BXVD_DeviceInterrupt_eStereoSeqError];

      if (pCallback->BXVD_P_pAppIntCallbackPtr)
      {
         pCallback->BXVD_P_pAppIntCallbackPtr(pCallback->pParm1,
                                              pCallback->parm2,
                                              (void *)&param2);
      }
   }

   BDBG_LEAVE(BXVD_P_StereoSeqError_isr);
}

/*
 * SWSTB-9214: handler for the interrupt from HVD, need to change the clock boost mode.
 */
void BXVD_P_ClockBoost_isr(void *pvXvd, int iParam2)
{
   BXVD_Handle hXvd = pvXvd;

   struct BXVD_P_InterruptCallbackInfo *pCallback;

   BDBG_ENTER(BXVD_P_ClockBoost_isr);

   if (hXvd != NULL)
   {
      pCallback = &hXvd->stDeviceInterruptCallbackInfo[BXVD_DeviceInterrupt_eClockBoost];

      if (pCallback->BXVD_P_pAppIntCallbackPtr)
      {

         BDBG_MSG(("%s: calling interrupt handler.", BSTD_FUNCTION ));

         pCallback->BXVD_P_pAppIntCallbackPtr(pCallback->pParm1,
                                              pCallback->parm2,
                                              (void *)&iParam2);
      }
      else
      {
         BDBG_MSG(("%s a NOP, no registered interrupt handler", BSTD_FUNCTION ));
      }
   }

   BDBG_LEAVE(BXVD_P_ClockBoost_isr);

   return;
}
