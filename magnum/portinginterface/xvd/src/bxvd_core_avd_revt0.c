/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include "bxvd_platform.h"
#include "bxvd_priv.h"

#include "bxvd_image.h"
#include "bxvd_image_priv.h"
#include "bxvd_reg.h"

#if BXVD_P_USE_FW_LOAD_CORE_REVT0
#include "bafl.h"
#endif

BDBG_MODULE(BXVD_CORE_AVD_REVT0);

/* ARC AUX register offsets */
#define BXVD_P_ARC_PC 0x18
#define BXVD_P_ARC_STATUS32 0x28

#if BXVD_P_USE_FW_LOAD_CORE_REVT0
BERR_Code BXVD_P_FWLoad_Revt0(BXVD_Handle hXvd,
                              uint32_t uiDecoderInstance)
{
   uint32_t uiEndOfCode;
   uint32_t uiILSize;

   void *pvCmdBuffer;

   unsigned long ulDestVirtAddr;

   bool bLoadDataOnly;

   BAFL_FirmwareLoadInfo stBAFLoadInfo;

   BERR_Code rc;

   BDBG_ASSERT(hXvd);
   BDBG_ASSERT(hXvd->stSettings.pImgInterface);
   BDBG_ASSERT(hXvd->stSettings.pImgContext);
   BSTD_UNUSED(uiDecoderInstance);

   BDBG_ENTER(BXVD_P_FWLoad_Revt0);

   BXVD_DBG_MSG(hXvd, ("Loading Outer Loop ELF image"));

   if (hXvd->eAVDBootMode == BXVD_AVDBootMode_eNormal)
   {
      bLoadDataOnly = false;
   }
   else
   {
      bLoadDataOnly = true;
   }

   if (hXvd->stSettings.pAVDBootCallback && (bLoadDataOnly == false))
   {
      /* If boot call back set and loading code, zero out FW image.  */
      BKNI_Memset((void*)hXvd->uiFWMemBaseVirtAddr, 0, BXVD_P_FW_IMAGE_SIZE);
   }

   rc = BAFL_Load ( BXVD_P_OL,
                    hXvd->stSettings.pImgInterface,
                    hXvd->stSettings.pImgContext,
                    BXVD_IMAGE_FirmwareID_eOuterELF_AVD0,
                    (void *)hXvd->uiFWMemBaseVirtAddr,
                    BXVD_P_FW_INNER_IMAGE_OFFSET,  /* Size of Outer Loop FW memory is start of Inner Loop FW */
                    bLoadDataOnly,
                    &stBAFLoadInfo );

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   BMMA_FlushCache(hXvd->hFWMemBlock, (void *)hXvd->uiFWMemBaseVirtAddr, BXVD_P_FW_INNER_IMAGE_OFFSET);

   hXvd->astFWBootInfo[0].uiArcInstance = BXVD_IMAGE_FirmwareID_eOuterELF_AVD0;
   hXvd->astFWBootInfo[0].stCode.pStartAddress = stBAFLoadInfo.stCode.pStartAddress;
   hXvd->astFWBootInfo[0].stCode.uiSize = stBAFLoadInfo.stCode.uiSize;

   hXvd->uiOuterLoopInstructionBase = hXvd->uiFWMemBasePhyAddr;

   uiEndOfCode = hXvd->uiOuterLoopInstructionBase + stBAFLoadInfo.stCode.uiSize + stBAFLoadInfo.stData.uiSize;

   hXvd->uiOuterLoopEOC = uiEndOfCode -  hXvd->uiOuterLoopInstructionBase;

   hXvd->uiCmdBufferVector = hXvd->uiOuterLoopInstructionBase + stBAFLoadInfo.stCode.uiSize;
   pvCmdBuffer = (void *)(hXvd->uiFWMemBaseVirtAddr + stBAFLoadInfo.stCode.uiSize);

   BXVD_DBG_MSG(hXvd, ("OL InstrBase: %08x", hXvd->uiOuterLoopInstructionBase));
   BXVD_DBG_MSG(hXvd, ("OL Start of code addr: %0x",    hXvd->uiOuterLoopInstructionBase));

   BXVD_DBG_MSG(hXvd, ("OL Code Size: %08x",  stBAFLoadInfo.stCode.uiSize));
   BXVD_DBG_MSG(hXvd, ("OL Data Size: %08x",  stBAFLoadInfo.stData.uiSize));
   BXVD_DBG_MSG(hXvd, ("OL End of code: %x", hXvd->uiOuterLoopEOC));

   BXVD_DBG_MSG(hXvd, ("Loading Inner Loop ELF image"));

   ulDestVirtAddr = hXvd->uiFWMemBaseVirtAddr + BXVD_P_FW_INNER_IMAGE_OFFSET;

   BXVD_DBG_MSG(hXvd,  ("Inner Loop FWBase: %0lx, Offset: %08x, DestAddr: %0lx",
                        hXvd->uiFWMemBaseVirtAddr, BXVD_P_FW_INNER_IMAGE_OFFSET, ulDestVirtAddr ));

   if (hXvd->bHEVDDualPipe)
   {
      uiILSize = BXVD_P_FW_INNER_2_IMAGE_OFFSET - BXVD_P_FW_INNER_IMAGE_OFFSET;
   }
   else
   {
      uiILSize = BXVD_P_FW_IMAGE_SIZE - BXVD_P_FW_INNER_IMAGE_OFFSET;
   }

   BXVD_DBG_MSG(hXvd, ("IL Image size to load: %08x", uiILSize));

   rc = BAFL_Load ( BXVD_P_IL,
                    hXvd->stSettings.pImgInterface,
                    hXvd->stSettings.pImgContext,
                    BXVD_IMAGE_FirmwareID_eInnerELF_AVD0,
                    (void *)ulDestVirtAddr,
                    uiILSize,
                    bLoadDataOnly,
                    &stBAFLoadInfo );

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   BMMA_FlushCache(hXvd->hFWMemBlock, (void *)ulDestVirtAddr, uiILSize);

   hXvd->astFWBootInfo[1].uiArcInstance=1;
   hXvd->astFWBootInfo[1].stCode.pStartAddress = stBAFLoadInfo.stCode.pStartAddress;
   hXvd->astFWBootInfo[1].stCode.uiSize = stBAFLoadInfo.stCode.uiSize;

   hXvd->uiInnerLoopInstructionBase = hXvd->uiFWMemBasePhyAddr + BXVD_P_FW_INNER_IMAGE_OFFSET;
   uiEndOfCode = hXvd->uiInnerLoopInstructionBase + stBAFLoadInfo.stCode.uiSize + stBAFLoadInfo.stData.uiSize;

   BXVD_DBG_MSG(hXvd, ("IL Start of Code: %08x", hXvd->uiInnerLoopInstructionBase));
   BXVD_DBG_MSG(hXvd, ("IL Code Size: %08x", stBAFLoadInfo.stCode.uiSize));
   BXVD_DBG_MSG(hXvd, ("IL Data Size: %08x", stBAFLoadInfo.stData.uiSize));
   BXVD_DBG_MSG(hXvd, ("IL End of Code:: %08x",  uiEndOfCode));

   hXvd->uiInnerLoopEOC = uiEndOfCode - hXvd->uiInnerLoopInstructionBase;

   BXVD_DBG_MSG(hXvd, ("InnerLoopInstBase: %08x", hXvd->uiInnerLoopInstructionBase));
   BXVD_DBG_MSG(hXvd, ("End of inner loop code at %x", hXvd->uiInnerLoopEOC));

   hXvd->stDecoderContext.ulCmdBufferAddr = (unsigned long) pvCmdBuffer;
   hXvd->stDecoderContext.ulRspBufferAddr = (unsigned long) pvCmdBuffer;

#if BXVD_P_HEVD_DUAL_PIPE_PRESENT
   if (hXvd->bHEVDDualPipe)
   {
      ulDestVirtAddr = hXvd->uiFWMemBaseVirtAddr + BXVD_P_FW_INNER_2_IMAGE_OFFSET;

      BXVD_DBG_MSG(hXvd, ("IL2 Image size to load: %08x", (BXVD_P_FW_IMAGE_SIZE - BXVD_P_FW_INNER_2_IMAGE_OFFSET)));
      rc = BAFL_Load ( BXVD_P_IL,
                       hXvd->stSettings.pImgInterface,
                       hXvd->stSettings.pImgContext,
                       BXVD_IMAGE_FirmwareID_eInnerELF_AVD0,
                       (void *)ulDestVirtAddr,
                       BXVD_P_FW_IMAGE_SIZE - BXVD_P_FW_INNER_2_IMAGE_OFFSET,
                       bLoadDataOnly,
                       &stBAFLoadInfo );

      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

      BMMA_FlushCache(hXvd->hFWMemBlock, (void *)ulDestVirtAddr, uiILSize);

      hXvd->astFWBootInfo[2].uiArcInstance=2;
      hXvd->astFWBootInfo[2].stCode.pStartAddress = stBAFLoadInfo.stCode.pStartAddress;
      hXvd->astFWBootInfo[2].stCode.uiSize = stBAFLoadInfo.stCode.uiSize;

      hXvd->uiInnerLoop2InstructionBase = hXvd->uiFWMemBasePhyAddr + BXVD_P_FW_INNER_2_IMAGE_OFFSET;
      uiEndOfCode = hXvd->uiInnerLoop2InstructionBase + stBAFLoadInfo.stCode.uiSize + stBAFLoadInfo.stData.uiSize;

      BXVD_DBG_MSG(hXvd, ("IL 2 Start of Code: %08x", hXvd->uiInnerLoop2InstructionBase));
      BXVD_DBG_MSG(hXvd, ("IL 2 Code Size: %08x", stBAFLoadInfo.stCode.uiSize));
      BXVD_DBG_MSG(hXvd, ("IL 2 Data Size: %08x", stBAFLoadInfo.stData.uiSize));
      BXVD_DBG_MSG(hXvd, ("IL 2 End of Code: %08x",  uiEndOfCode));

      hXvd->uiInnerLoop2EOC = uiEndOfCode - hXvd->uiInnerLoopInstructionBase;

      BXVD_DBG_MSG(hXvd, ("InnerLoop2InstBase: %08x", hXvd->uiInnerLoop2InstructionBase));
      BXVD_DBG_MSG(hXvd, ("End of inner loop 2 code at %x", hXvd->uiInnerLoop2EOC));
   }
#endif

   BDBG_LEAVE(BXVD_P_FWLoad_Revt0);

   return BERR_TRACE(BERR_SUCCESS);
}
#endif /* BXVD_P_USE_FW_LOAD_CORE_REVT0 */

#if BXVD_P_USE_GET_BUFFER_ATOM_SIZE_REVT0

/* This computes the aligned height */
/* The generic formula is Xn+Y, for example 8n+3, X is 8 and Y 3*/
uint32_t BXVD_P_CalcHeightMultiple(uint32_t uiHeight,
                                    BXVD_P_PFRI_Bank_Height eBankHeight)
{
   uint32_t MbHeight;
   uint32_t n, x, y;

   switch (eBankHeight)
   {
      case BXVD_P_PFRI_Bank_Height_8n3:
         x = 8;
#if BXVD_P_8N3_BUG
         y = 4;
#else
         y = 3;
#endif
         break;

      case BXVD_P_PFRI_Bank_Height_16n6:
         x = 16;
         y = 6;
         break;

      case BXVD_P_PFRI_Bank_Height_32n12:
         x = 32;
         y = 12;
         break;

      default:
         x = 8;
         y = 3;
         break;
   }

   MbHeight = (uiHeight + 15) / 16;

   n = (MbHeight - y + x - 1) / x;

   MbHeight = (x * n) + y;

   return (MbHeight * 16);
}

#define BXVD_P_16K 16384

void BXVD_P_CalcVideoSize( uint32_t  uiWidth,
                           uint32_t  uiHeight,
                           bool      bBitDepth10,
                           uint32_t  uiStripeWidth,
                           BXVD_P_PFRI_Bank_Height eBankHeight,
                           uint32_t *puiYSize,
                           uint32_t *puiUVSize)
{
   uint32_t uiVideoYHeight;
   uint32_t uiVideoYWidth;
   uint32_t uiVideoUVHeight;

   uint32_t uiYVideoSize;
   uint32_t uiUVVideoSize;

   if (bBitDepth10 == true)
   {
      uiWidth = ((uiWidth + 2) / 3) * 4;
   }

   /* Grab selection from the spreadsheet */

   /* Determine actual stripe width from register field value, 0 - 64, 1 - 128, 2 - 256 */
   if ( uiStripeWidth == 0 )
   {
      uiStripeWidth = 64;
   }
   else if ( uiStripeWidth == 1 )
   {
      uiStripeWidth = 128;
   }
   else
   {
      uiStripeWidth = 256;
   }

   /* Making the Luma width multiple of stripe_width */
   uiVideoYWidth = ((uiWidth + uiStripeWidth - 1) / uiStripeWidth) * uiStripeWidth;

   /* finding the correct height based on the formula */
   uiVideoYHeight = BXVD_P_CalcHeightMultiple(uiHeight, eBankHeight);
   uiVideoUVHeight = BXVD_P_CalcHeightMultiple((uiHeight/2), eBankHeight);

   /* round to 16K this is the max page size for ddr4 */
   uiYVideoSize = (((uiVideoYHeight * uiVideoYWidth) + BXVD_P_16K-1) / BXVD_P_16K) * BXVD_P_16K;

   /* Please note Chroma buffer uses the same width as luma as UV are interleaved. This buffer is also made multiple of 16K */
   uiUVVideoSize = (((uiVideoUVHeight * uiVideoYWidth) + BXVD_P_16K-1)/ BXVD_P_16K) * BXVD_P_16K;

   *puiYSize = uiYVideoSize;
   *puiUVSize = uiUVVideoSize;
}


void BXVD_P_GetBufferAtomSize_RevT0(BXVD_Handle hXvd,
                                    const BXVD_ChannelSettings *pChSettings,
                                    BXVD_P_VideoAtomIndex vidAtomIndex,
                                    uint32_t *puiLumaAtomSize,
                                    uint32_t *puiChromaAtomSize)
{
   uint32_t uiWidth = 0;
   uint32_t uiHeight = 0;

   uint32_t uiLumaAtomSizeTemp=0;
   uint32_t uiChromaAtomSizeTemp=0;

   if ((vidAtomIndex == BXVD_P_VideoAtomIndex_eA) ||
       (vidAtomIndex == BXVD_P_VideoAtomIndex_eF) ||
       (vidAtomIndex == BXVD_P_VideoAtomIndex_eH))
   {
      uiWidth = 1920;
      uiHeight = 1088;
   }

   else if ((vidAtomIndex == BXVD_P_VideoAtomIndex_eB) ||
            (vidAtomIndex == BXVD_P_VideoAtomIndex_eE))
   {
      uiWidth = 720;
      uiHeight = 576;
   }

   else if (vidAtomIndex == BXVD_P_VideoAtomIndex_eC)
   {
      uiWidth = 352;
      uiHeight = 288;
   }

   else if (vidAtomIndex == BXVD_P_VideoAtomIndex_eD)
   {
      uiWidth = 176;
      uiHeight = 144;
   }

   else if ((vidAtomIndex == BXVD_P_VideoAtomIndex_eG) ||
            (vidAtomIndex == BXVD_P_VideoAtomIndex_eM))
   {
      uiWidth = 1280;
      uiHeight = 720;
   }

   else if (vidAtomIndex == BXVD_P_VideoAtomIndex_eI)
   {
      uiWidth = 1920;
      uiHeight = 544;
   }

   else if (vidAtomIndex == BXVD_P_VideoAtomIndex_eJ)
   {
      uiWidth = 4096;
#if BXVD_P_VP9_CAPABLE
      uiHeight = 2304;
#else
      uiHeight = 2160;
#endif
   }

   else if (vidAtomIndex == BXVD_P_VideoAtomIndex_eAT)
   {
      uiWidth = 1088;
      uiHeight = 1920;
   }

   else if (vidAtomIndex == BXVD_P_VideoAtomIndex_eK)
   {
      uiWidth = 1920;
      uiHeight = 1200;
   }

   BXVD_DBG_MSG(hXvd,("Width: %d, Hgt: %d, StripeWid: %d, StripeMultiple: %d",
                      uiWidth, uiHeight, hXvd->uiDecode_StripeWidth, hXvd->uiDecode_StripeMultiple));

   BXVD_P_CalcVideoSize( uiWidth, uiHeight,
                         pChSettings->b10BitBuffersEnable, hXvd->uiDecode_StripeWidth, hXvd->uiDecode_StripeMultiple,
                         puiLumaAtomSize, puiChromaAtomSize);

   if (vidAtomIndex == BXVD_P_VideoAtomIndex_eM)
   {
      /* 2 720 pictures need to fit in 1 1088 buffers, so make sure the HD atoms are big enough */
      uiWidth = 1920;
      uiHeight = 1088;

      BXVD_P_CalcVideoSize( uiWidth, uiHeight,
                            pChSettings->b10BitBuffersEnable, hXvd->uiDecode_StripeWidth, hXvd->uiDecode_StripeMultiple,
                            &uiLumaAtomSizeTemp, &uiChromaAtomSizeTemp);

      if ((uiChromaAtomSizeTemp+uiLumaAtomSizeTemp) > (2*(*puiLumaAtomSize+*puiChromaAtomSize)))
      {
         *puiLumaAtomSize = uiLumaAtomSizeTemp;
         *puiChromaAtomSize = uiChromaAtomSizeTemp;
      }
      else
      {
         *puiLumaAtomSize = (*puiLumaAtomSize)*2;
         *puiChromaAtomSize = (*puiChromaAtomSize)*2;
      }
   }

   else if (vidAtomIndex == BXVD_P_VideoAtomIndex_eJ)
   {
      /* Compare 2 times the field size with frame size, use the larger */
      uiWidth = 4096;
      uiHeight = 1088;

      BXVD_P_CalcVideoSize( uiWidth, uiHeight,
                            pChSettings->b10BitBuffersEnable, hXvd->uiDecode_StripeWidth, hXvd->uiDecode_StripeMultiple,
                            &uiLumaAtomSizeTemp, &uiChromaAtomSizeTemp);

      if ((uiLumaAtomSizeTemp*2) > (*puiLumaAtomSize))
      {
         *puiLumaAtomSize = uiLumaAtomSizeTemp*2;
      }

      if ((uiChromaAtomSizeTemp*2) > (*puiChromaAtomSize))
      {
         *puiChromaAtomSize = uiChromaAtomSizeTemp*2;
      }
   }

   else if (vidAtomIndex == BXVD_P_VideoAtomIndex_eF)
   {
      *puiLumaAtomSize = (*puiLumaAtomSize)*2;
      *puiChromaAtomSize = (*puiChromaAtomSize)*2;
   }

   else if ((vidAtomIndex == BXVD_P_VideoAtomIndex_eH) ||
            (vidAtomIndex == BXVD_P_VideoAtomIndex_eI))
   {
      *puiLumaAtomSize = (*puiLumaAtomSize)*4;
      *puiChromaAtomSize = (*puiChromaAtomSize)*4;
   }

   else if ((vidAtomIndex == BXVD_P_VideoAtomIndex_eE) ||
            (vidAtomIndex == BXVD_P_VideoAtomIndex_eD))
   {
      *puiLumaAtomSize = (*puiLumaAtomSize)*3;
      *puiChromaAtomSize = (*puiChromaAtomSize)*3;
   }

   BXVD_DBG_MSG(hXvd,("LumaAtomSize: %d, ChromaAtomSize: %d", *puiLumaAtomSize, *puiChromaAtomSize));
}
#endif
