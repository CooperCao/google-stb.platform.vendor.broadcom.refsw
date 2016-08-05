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
 * Module Description:
 *   See code
 *
 ***************************************************************************/
#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd_image.h"
#include "bxvd_image_priv.h"
#include "bxvd_reg.h"

#if BXVD_P_USE_FW_LOAD_CORE_REVK0
#include "bafl.h"
#endif

#include "bchp_common.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

/* ARC AUX register offsets */
#define BXVD_P_ARC_PC 0x18
#define BXVD_P_ARC_STATUS32 0x28

BDBG_MODULE(BXVD_CORE_AVD_REVK0);

#if BXVD_P_USE_FW_LOAD_CORE_REVK0
BERR_Code BXVD_P_FWLoad_RevK0(BXVD_Handle hXvd,
                              uint32_t uiDecoderInstance)
{
   uint32_t uiEndOfCode;
   uint32_t uiILSize;

   void *pvCmdBuffer;

   uint32_t uiDestVirtAddr;

   bool bLoadDataOnly;

   BAFL_FirmwareLoadInfo stBAFLoadInfo;

   BXVD_IMAGE_ContextEntry *pOuterImgContext;
   BXVD_IMAGE_ContextEntry *pInnerImgContext;

#if BXVD_P_SVD_PRESENT
   BXVD_IMAGE_ContextEntry *pBaseImgContext;
#endif

   BERR_Code rc;

   BDBG_ASSERT(hXvd);
   BDBG_ASSERT(hXvd->stSettings.pImgInterface);
   BDBG_ASSERT(hXvd->stSettings.pImgContext);
   BSTD_UNUSED(uiDecoderInstance);

   BDBG_ENTER(BXVD_P_FWLoad_RevK0);

   BXVD_DBG_MSG(hXvd, ("Loading Outer Loop ELF image"));

   /* Get image context entry for Outer loop ARC */
   pOuterImgContext = ((BXVD_IMAGE_ContextEntry **)hXvd->stSettings.pImgContext)[BXVD_IMAGE_RevK_FirmwareID_eOuterELF_AVD];
   pInnerImgContext = ((BXVD_IMAGE_ContextEntry **)hXvd->stSettings.pImgContext)[BXVD_IMAGE_RevK_FirmwareID_eInnerELF_AVD];

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

   uiDestVirtAddr = hXvd->uiFWMemBaseVirtAddr + BXVD_P_FW_INNER_IMAGE_OFFSET;

   BXVD_DBG_MSG(hXvd,  ("Inner Loop FWBase: %08x, Offset: %08x, DestAddr: %08x",
                        hXvd->uiFWMemBaseVirtAddr, BXVD_P_FW_INNER_IMAGE_OFFSET, uiDestVirtAddr ));

#if BXVD_P_SVD_PRESENT
   /* If SVD capable, setup to load BLD ARC */
   if (hXvd->bSVCCapable)
   {
      /* Base Layer image offset minus Inner image offset */
      pBaseImgContext = ((BXVD_IMAGE_ContextEntry **)hXvd->stSettings.pImgContext)[BXVD_IMAGE_RevK_FirmwareID_eBaseELF_SVD];

      uiILSize = BXVD_P_FW_BASELAYER_IMAGE_OFFSET - BXVD_P_FW_INNER_IMAGE_OFFSET;
   }
   else
   {
      uiILSize = BXVD_P_FW_IMAGE_SIZE - BXVD_P_FW_INNER_IMAGE_OFFSET;
   }
#else
   uiILSize = BXVD_P_FW_IMAGE_SIZE - BXVD_P_FW_INNER_IMAGE_OFFSET;
#endif

   rc = BAFL_Load ( BXVD_P_IL,
                    hXvd->stSettings.pImgInterface,
                    hXvd->stSettings.pImgContext,
                    BXVD_IMAGE_FirmwareID_eInnerELF_AVD0,
                    (void *)uiDestVirtAddr,
                    uiILSize,
                    bLoadDataOnly,
                    &stBAFLoadInfo );

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   BMMA_FlushCache(hXvd->hFWMemBlock, (void *)uiDestVirtAddr, uiILSize);

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

   hXvd->stDecoderContext.ulCmdBufferAddr = (uint32_t) pvCmdBuffer;
   hXvd->stDecoderContext.ulRspBufferAddr = (uint32_t) pvCmdBuffer;

#if BXVD_P_SVD_PRESENT
   /* Check to see if BL ARC is present in this decoder */
   if (hXvd->bSVCCapable)
   {
      pBaseImgContext = ((BXVD_IMAGE_ContextEntry **)hXvd->stSettings.pImgContext)[BXVD_IMAGE_RevK_FirmwareID_eBaseELF_SVD];

      uiDestVirtAddr = hXvd->uiFWMemBaseVirtAddr + BXVD_P_FW_BASELAYER_IMAGE_OFFSET;
      BXVD_DBG_MSG(hXvd,  ("Base Layer FWBase: %08x, Offset: %08x, DestAddr: %08x",
                           hXvd->uiFWMemBaseVirtAddr, BXVD_P_FW_BASELAYER_IMAGE_OFFSET, uiDestVirtAddr ));

      rc = BAFL_Load ( BXVD_P_BL,
                       hXvd->stSettings.pImgInterface,
                       hXvd->stSettings.pImgContext,
                       BXVD_IMAGE_RevK_FirmwareID_eBaseELF_SVD,
                       (void *)uiDestVirtAddr,
                       BXVD_P_FW_IMAGE_SIZE - BXVD_P_FW_BASELAYER_IMAGE_OFFSET, /* Start of in FW memory space where FW is loaded */
                       bLoadDataOnly,
                       &stBAFLoadInfo );

      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

      BMMA_FlushCache(hXvd->hFWMemBlock, (void *)uiDestVirtAddr,  BXVD_P_FW_IMAGE_SIZE - BXVD_P_FW_BASELAYER_IMAGE_OFFSET);

      hXvd->astFWBootInfo[2].uiArcInstance = BXVD_IMAGE_RevK_FirmwareID_eBaseELF_SVD;
      hXvd->astFWBootInfo[2].stCode.pStartAddress = stBAFLoadInfo.stCode.pStartAddress;
      hXvd->astFWBootInfo[2].stCode.uiSize = stBAFLoadInfo.stCode.uiSize;

      hXvd->uiBaseInstructionBase = hXvd->uiFWMemBasePhyAddr + BXVD_P_FW_BASELAYER_IMAGE_OFFSET;
      uiEndOfCode = hXvd->uiBaseInstructionBase + stBAFLoadInfo.stCode.uiSize + stBAFLoadInfo.stData.uiSize;

      BXVD_DBG_MSG(hXvd, ("BL Start of Code: %08x", hXvd->uiBaseInstructionBase));
      BXVD_DBG_MSG(hXvd, ("BL Code Size: %08x", stBAFLoadInfo.stCode.uiSize));
      BXVD_DBG_MSG(hXvd, ("BL Data Size: %08x", stBAFLoadInfo.stData.uiSize));
      BXVD_DBG_MSG(hXvd, ("BL End of Code:: %08x", uiEndOfCode));

      hXvd->uiBaseEOC = uiEndOfCode - hXvd->uiBaseInstructionBase;

      BXVD_DBG_MSG(hXvd, ("BaseLayerInstBase: %08x", hXvd->uiBaseInstructionBase));
      BXVD_DBG_MSG(hXvd, ("End of Base Layer code at %x", hXvd->uiBaseEOC));

   }
#endif

   BDBG_LEAVE(BXVD_P_FWLoad_RevK0);

   return BERR_TRACE(BERR_SUCCESS);
}
#endif /* BXVD_P_USE_FW_LOAD_CORE_REVK0 */

#if BXVD_P_USE_CORE_CHIP_ENABLE_REVK0

BERR_Code BXVD_P_ChipEnable_RevK0(BXVD_Handle hXvd)
{
   volatile uint32_t uiFWBootStatus;

   BAFL_BootInfo stAVDBootInfo;

   BAFL_FirmwareInfo astFirmwareInfo[3];

   BERR_Code rc = BERR_SUCCESS;

#if BXVD_POLL_FW_MBX
   uint32_t uiVal;
   int loopCount;
#endif

   BDBG_ASSERT(hXvd);
   BDBG_ENTER(BXVD_P_ChipEnable_RevK0);

   /*
    * Write to VectorTB, CpuDbg registers and AuxRegs
    * in THIS ORDER to start Outer Loop ARC
    */

   BDBG_MSG(("OL Ibase: %0x", hXvd->uiOuterLoopInstructionBase));
   BDBG_MSG(("OL EOC: %0x", hXvd->uiOuterLoopEOC));

   BDBG_MSG(("IL Ibase: %0x",hXvd->uiInnerLoopInstructionBase));
   BDBG_MSG(("IL EOC: %0x", hXvd->uiInnerLoopEOC));

   /* Program the relocation base address for outer-loop */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterInstructionBase,
                    hXvd->uiOuterLoopInstructionBase);

   /* Program the outer loop image end of code address */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterEndOfCode,
                    hXvd->uiOuterLoopEOC);

   /* Program the relocation base address for inner-loop */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_InnerInstructionBase,
                    hXvd->uiInnerLoopInstructionBase);

   /* Program the inner loop image end of code address */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_InnerEndOfCode,
                    hXvd->uiInnerLoopEOC);

   /* Program global IO base register - Outer */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterGlobalIOBase,
                    BCHP_PHYSICAL_OFFSET);

   /* Program global IO base register - Inner */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_InnerGlobalIOBase,
                    BCHP_PHYSICAL_OFFSET);

#if BXVD_P_SVD_PRESENT
   if (hXvd->bSVCCapable)
   {

      BDBG_MSG(("BL Ibase: %0x",hXvd->uiBaseInstructionBase));
      BDBG_MSG(("BL EOC: %0x", hXvd->uiBaseEOC));

      /* Program the base layer relocation base address. */
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_BaseInstructionBase,
                       hXvd->uiBaseInstructionBase);

      /* Program the base layer image end of code address */
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_BaseEndOfCode,
                       hXvd->uiBaseEOC);

      /* Program base layer global IO base register */
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_BaseGlobalIOBase,
                       BCHP_PHYSICAL_OFFSET);

   }
#endif

   /* Clear out any previously completed FW command done events */
   BKNI_ResetEvent(hXvd->stDecoderContext.hFWCmdDoneEvent);

   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterHost2CPUMailbox,
                    hXvd->uiAVDCoreFreq);

#if BXVD_POLL_FW_MBX
   /* Initialize MBX to non-zero */
   BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostMailbox, 0xff);
   uiVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostMailbox);
   BDBG_MSG(("Initial CPU2HostMB: %0x", uiVal));
#endif

   if (hXvd->stSettings.pAVDBootCallback)
   {
      /* Set boot mode */
      if ( hXvd->eAVDBootMode == BXVD_AVDBootMode_eNormal)
      {
         stAVDBootInfo.eMode = BAFL_BootMode_eNormal;
      }
      else
      {
         stAVDBootInfo.eMode = BAFL_BootMode_eWatchdog;
      }

      stAVDBootInfo.pstArc = &astFirmwareInfo[0];

      /* Set Outer Loop ARC firmware info */
      astFirmwareInfo[0].uiArcInstance = 0;
      astFirmwareInfo[0].stCode.pStartAddress = hXvd->astFWBootInfo[0].stCode.pStartAddress;
      astFirmwareInfo[0].stCode.uiSize = hXvd->astFWBootInfo[0].stCode.uiSize;
      astFirmwareInfo[0].pNext = &astFirmwareInfo[1];

      /* Set Inner Loop ARC firmware info */
      astFirmwareInfo[1].uiArcInstance = 1;
      astFirmwareInfo[1].stCode.pStartAddress = hXvd->astFWBootInfo[1].stCode.pStartAddress;
      astFirmwareInfo[1].stCode.uiSize = hXvd->astFWBootInfo[1].stCode.uiSize;
      astFirmwareInfo[1].pNext = NULL;

#if BXVD_P_SVD_PRESENT
      if (hXvd->bSVCCapable)
      {
         astFirmwareInfo[1].pNext = &astFirmwareInfo[2];

         /* Set Base Layer ARC firmware info */
         astFirmwareInfo[2].uiArcInstance = 2;
         astFirmwareInfo[2].stCode.pStartAddress = hXvd->astFWBootInfo[2].stCode.pStartAddress;
         astFirmwareInfo[2].stCode.uiSize = hXvd->astFWBootInfo[2].stCode.uiSize;
         astFirmwareInfo[2].pNext = NULL;
      }
#endif
      /* Call external boot callback */
      rc = (*hXvd->stSettings.pAVDBootCallback)(hXvd->stSettings.pAVDBootCallbackData,
                                                &stAVDBootInfo);
      if (rc != BERR_SUCCESS)
      {
         BXVD_DBG_ERR(hXvd, ("Error #%d booting the AVD externally", rc));
         return BERR_TRACE(rc);
      }
   }
   else
   {
      /* Boot the core */

      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUDebug,
                       1);

      /* Write ARC PC start address */
      BXVD_Reg_Write32(hXvd,
                       (hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUAux+BXVD_P_ARC_PC),
                       0);

      /* Start Arc */
      BXVD_Reg_Write32(hXvd,
                       (hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUAux+BXVD_P_ARC_STATUS32),
                       0);
   }

#define FW_CMD_TIMEOUT 10000

#if !BXVD_POLL_FW_MBX

   rc = BERR_TRACE(BKNI_WaitForEvent(hXvd->stDecoderContext.hFWCmdDoneEvent, FW_CMD_TIMEOUT));

#else

   uiVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostMailbox);

   loopCount = 0;
   rc = BERR_TIMEOUT;

   while (loopCount < FW_CMD_TIMEOUT)
   {
      if (uiVal != 0)
      {
         uiFWBootStatus = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostStatus);

         BDBG_MSG(("ARC FW Boot Status = %d", uiFWBootStatus));

         BDBG_MSG(("loopCount:%d Calling BKNI_Sleep(1), MBX:%d", loopCount, uiVal));
         BKNI_Sleep(1);

         loopCount++;
         uiVal = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostMailbox);
      }
      else
      {
         rc = BERR_SUCCESS;
         break;
      }
   }

#endif

   /* Read FW boot progress/status from CPU2HostStatus register that was written by FW */
   uiFWBootStatus = BXVD_Reg_Read32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostStatus);

   if(BERR_TIMEOUT == rc)
   {
      BXVD_DBG_ERR(hXvd, ("ARC FW command response timed out, FW Boot Status = %d", uiFWBootStatus));

      return BERR_TRACE(rc);
   }
   else
   {
      BXVD_DBG_MSG(hXvd, ("FW boot successful, FW Boot Status = %d", uiFWBootStatus));
   }

   BKNI_ResetEvent(hXvd->stDecoderContext.hFWCmdDoneEvent);

   BDBG_LEAVE(BXVD_P_ChipEnable_RevK0);
   return BERR_TRACE(rc);
}

#endif /* BXVD_P_USE_CORE_CHIP_ENABLE_REVK0 */


#if BXVD_P_USE_READ_DISPLAY_INFO_HIM_API_REVK0

void BXVD_P_ReadDisplayInfo_HIM_API_isr(BXVD_Handle hXvd,
uint32_t uiDisplayInfoOffset,
BXVD_P_DisplayInfo *pstDisplayInfo)
{
uint32_t i, wordOffset;

wordOffset = uiDisplayInfoOffset;

   for (i = 0; i < BXVD_P_STC_MAX; i++)
   {
      BXVD_P_READ_HIM(hXvd, wordOffset, pstDisplayInfo->stc_snapshot[i]);
      wordOffset++;
   }

   BXVD_P_READ_HIM(hXvd, wordOffset, pstDisplayInfo->vsync_parity);
}

bool BXVD_P_IsDisplayInfoEqual_HIM_API_isr(BXVD_P_DisplayInfo stDisplayInfo,
                                       BXVD_P_DisplayInfo stDisplayInfo1)
{
   uint32_t i;
   bool bDisplayInfoEqual = true;

   for (i=0; i<8; i++)
   {
      if (stDisplayInfo.stc_snapshot[i] != stDisplayInfo1.stc_snapshot[i])
      {
         bDisplayInfoEqual = false;
         break;
      }
   }

   if (stDisplayInfo.vsync_parity != stDisplayInfo1.vsync_parity)
   {
      bDisplayInfoEqual = false;
   }

   return bDisplayInfoEqual;
}

#endif  /* BXVD_P_USE_READ_DISPLAY_INFO_HIM_API_REVK0 */

#if BXVD_P_USE_SET_POWER_STATE_REVK0

void BXVD_P_SetPowerState_RevK0(BXVD_Handle hXvd,
                                BXVD_P_PowerState PowerStateNew)
{
#if BCHP_PWR_RESOURCE_AVD

   BCHP_PWR_ResourceId clkResourceId = BCHP_PWR_RESOURCE_AVD0_CLK;
   BCHP_PWR_ResourceId pwrResourceId = BCHP_PWR_RESOURCE_AVD0_PWR;

#if BCHP_PWR_RESOURCE_AVD1
   if (hXvd->uDecoderInstance == 1)
   {
      clkResourceId = BCHP_PWR_RESOURCE_AVD1_CLK;
      pwrResourceId = BCHP_PWR_RESOURCE_AVD1_PWR;
   }
#endif

#if BCHP_PWR_RESOURCE_AVD2
   if (hXvd->uDecoderInstance == 2)
   {
      clkResourceId = BCHP_PWR_RESOURCE_AVD2_CLK;
      pwrResourceId = BCHP_PWR_RESOURCE_AVD2_PWR;
   }
#endif

   BDBG_MSG(("BXVD_P_SetPowerState, Decoder:%d Cur: %d New: %d",
             hXvd->uDecoderInstance, hXvd->PowerStateCurrent , PowerStateNew));

   if (hXvd->PowerStateCurrent != PowerStateNew)
   {
      if ((hXvd->PowerStateCurrent == BXVD_P_PowerState_eOn) &&
          ((PowerStateNew == BXVD_P_PowerState_eClkOff) ||
           (PowerStateNew == BXVD_P_PowerState_ePwrOff)))
      {
         BDBG_MSG(("Release AVD_Clk Resource"));
         BCHP_PWR_ReleaseResource(hXvd->hChip, clkResourceId);
      }

      if (((hXvd->PowerStateCurrent == BXVD_P_PowerState_eOn) ||
           (hXvd->PowerStateCurrent == BXVD_P_PowerState_eClkOff)) &&
          (PowerStateNew == BXVD_P_PowerState_ePwrOff))
      {
         BDBG_MSG(("Release AVD_PWR Resource"));
         BCHP_PWR_ReleaseResource(hXvd->hChip, pwrResourceId);
      }

      if ((hXvd->PowerStateCurrent == BXVD_P_PowerState_ePwrOff) &&
          ((PowerStateNew == BXVD_P_PowerState_eClkOff) ||
           (PowerStateNew == BXVD_P_PowerState_eOn)))
      {
         BDBG_MSG(("Acquire AVD_PWR Resource"));

         BCHP_PWR_AcquireResource(hXvd->hChip, pwrResourceId);
      }

      if (((hXvd->PowerStateCurrent == BXVD_P_PowerState_eClkOff) ||
           (hXvd->PowerStateCurrent == BXVD_P_PowerState_ePwrOff)) &&
          (PowerStateNew == BXVD_P_PowerState_eOn))
      {
         BDBG_MSG(("Acquire AVD_CLK Resource"));
         BCHP_PWR_AcquireResource(hXvd->hChip, clkResourceId);
      }

      hXvd->PowerStateCurrent = PowerStateNew;
   }
#else  /* !BCHP_PWR_RESOURCE_AVD0 so PM not supported */

   hXvd->PowerStateCurrent = PowerStateNew;

#endif
}

#endif /* BXVD_P_USE_SET_POWER_STATE_REVK0 */


#ifdef BXVD_P_USE_VERIFY_WATCHDOG_FIRED_REVK0

bool BXVD_P_VerifyWatchdogFired_RevK0_isr(BXVD_Handle hXvd,
                                          int param2)
{
   volatile uint32_t     uIntrVal;

#if BDBG_DEBUG_BUILD
   volatile uint32_t     uiOL_pc;
   int i;
#endif

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_VerifyWatchdogFired_RevK0_isr);
   if ( param2 == BXVD_P_OUTER_WATCHDOG)
   {
      uIntrVal = BXVD_Reg_Read32_isr(hXvd,
                                     hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptClear);

      BXVD_DBG_MSG(hXvd, ("L3IntClr:%08x", uIntrVal));

      if ( uIntrVal & BXVD_P_INTR_OL_MASK)
      {
         BXVD_DBG_MSG(hXvd, ("Outer Loop Watchdog"));
         BXVD_DBG_ERR(hXvd, ("OL watchdog - firmware revision: 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hXvd->sRevisionInfo.ulDecoderFwRev));

#if BDBG_DEBUG_BUILD
         BXVD_Reg_Write32_isr(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUDebug, 1);

         /* Read FW OL PC to help with debugging watchdog timeouts */
         for (i = 0; i < 8; i++)
         {
            /* read the AVD OL PC */
            uiOL_pc = BXVD_Reg_Read32_isr(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUAux+BXVD_P_ARC_PC);

            BXVD_DBG_ERR(hXvd, ("[%d] AVD_%d: OL PC=%08x", i, hXvd->uDecoderInstance, uiOL_pc));
         }
#endif
         BXVD_Reg_Write32_isr(hXvd,
                              hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptClear,
                              uIntrVal);
      }

      if (!(uIntrVal & BXVD_P_INTR_OL_MASK))
      {
         return false;
      }
   }
   else
   {
      BXVD_DBG_MSG(hXvd, ("Inner Loop Watchdog"));
   }

   BDBG_LEAVE(BXVD_P_VerifyWatchdogFired_RevK0_isr);

   return true;
}

#endif /*  BXVD_P_USE_VERIFY_WATCHDOG_FIRED_REVK0 */
