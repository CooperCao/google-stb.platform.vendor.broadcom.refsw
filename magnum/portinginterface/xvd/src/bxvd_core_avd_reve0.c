/******************************************************************************
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
 *****************************************************************************/

#include "bxvd.h"
#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd_intr.h"
#include "bxvd_reg.h"
#include "bxvd_userdata.h"
#include "bxvd_platform.h"
#include "bmma.h"
#include "bchp_common.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif
/* #define BXVD_DM_ENABLE_YUV_GRAB_MODE 1 */

/* Local constants */
#define BXVD_REVE0_OL_INIT_TIMEOUT 1000         /* One second init timeout */
#define BXVD_PRIV_USE_CACHED_MEMORY 0           /* Turn caching on or off */

/* Define BXVD_POLL_FW_MBX to bring up decoder witout the use in interrupts */
/* #define BXVD_POLL_FW_MBX 1 */

BDBG_MODULE(BXVD_CORE_AVD_REVE0);


/* AVD Memory requirements based on "AVD Core Mosaic Mode API", Rev
 * 2.19, March 3, 2008 */

#if BXVD_P_USE_GET_BUFFER_ATOM_SIZE_REVE0

#define BXVD_P_STRIPE_VIDEO_ATOM_NUM 13

static const uint32_t sVideoAtomSize[BXVD_P_STRIPE_WIDTH_NUM][BXVD_P_STRIPE_MULTIPLE_NUM][BXVD_P_STRIPE_VIDEO_ATOM_NUM] =
{
   {
      {3194880,  688128,  172032,  159744, 2064384, 6389760, 5898240, 12779520, 6389760, 3207168, 13631488, 7127040}, /* 0/0 (64/32) */
      {3276800,  688128,  196608,  221184, 2064384, 6389760, 6553600, 12779520, 6881280, 3203072, 13631488, 7372800}, /* 0/1 (64/64) */
      {3440640,  786432,  294912,  294912, 2359296, 6881280, 6553600, 13762560, 7864320, 3342336, 13631488, 7864320}  /* 0/2 (64/128) */
   },
   {
      {3194880,  688128,  172032,  196608, 2064384, 6389760, 5898240, 12779520, 6389760, 3391488, 13631488, 7127040}, /* 1/0 (128/32) */
      {3276800,  688128,  196608,  294912, 2064384, 6389760, 6553600, 12779520, 6881280, 3391488, 13631488, 7372800}, /* 1/1 (128/64) */
      {3440640,  786432,  294912,  393216, 2359296, 6881280, 6553600, 13762560, 7864320, 3538944, 13631488, 7864320}  /* 1/2 (128/128) */
   }
};

void BXVD_P_GetBufferAtomSize_RevE0(BXVD_Handle hXvd,
                                    const BXVD_ChannelSettings *pChSettings,
                                    BXVD_P_VideoAtomIndex  vidBlkIndex,
                                    uint32_t *puiLumaAtomSize,
                                    uint32_t *puiChromaAtomSize)
{
   BSTD_UNUSED(puiChromaAtomSize);
   BSTD_UNUSED(pChSettings);

   if (vidBlkIndex == BXVD_P_VideoAtomIndex_eM)
   {
      vidBlkIndex = BXVD_P_VideoAtomIndex_eA;
   }

   *puiLumaAtomSize = sVideoAtomSize[hXvd->uiDecode_StripeWidth][hXvd->uiDecode_StripeMultiple][vidBlkIndex];
}

#endif


#ifdef BXVD_P_USE_DETERMINE_STRIPE_INFO_REVE0
void BXVD_P_DetermineStripeInfo_RevE0( BCHP_DramType ddrType,
                                       uint32_t uiMemPartSize,
                                       uint32_t uiMemBusWidth,
                                       uint32_t uiMemDeviceWidth,
                                       bool     bDDRGroupageEnabled,
                                       uint32_t *puiStripeWidth,
                                       uint32_t *puiBankHeight)
{
   BSTD_UNUSED(ddrType);
   BSTD_UNUSED(uiMemDeviceWidth);
   BSTD_UNUSED(bDDRGroupageEnabled);

   /* Set stripe width and bank height base on bus width and memory size */
   switch(uiMemBusWidth)
   {
      /* Dram bus width 64 */
      case 64:

         switch(uiMemPartSize)
         {
            case 256: /* 256 Mbits Device Tech*/
               *puiStripeWidth =1; /* 64 bytes */
               *puiBankHeight = 1; /* 2 Mblks */
               break;

            case 512:  /* 512 Mbits Device Tech*/
            case 1024: /* 1024 Mbits Device Tech*/
            case 2048: /* 2048 Mbits Device Tech*/
            case 4096: /* 4096 Mbits Device Tech*/
            case 8192: /* 8192 Mbits Device Tech*/
               *puiStripeWidth =1; /* 128 bytes */
               *puiBankHeight = 2; /* 2 Mblks */
               break;

            default:
               *puiStripeWidth = 0;
               *puiBankHeight  = 1; /* 2 Mblks */
               BDBG_ERR(("Unknown MemPartSize: %d", uiMemPartSize));
               break;
         }

         break;

      case 32:

         switch(uiMemPartSize)
         {
            case 256: /* 256 Mbits Device Tech*/
               *puiStripeWidth =0; /* 64 bytes */
               *puiBankHeight = 1; /* 2 Mblks */
               break;

            case 512:  /* 512 Mbits Device Tech*/
            case 1024: /* 1024 Mbits Device Tech*/
            case 2048: /* 2048 Mbits Device Tech*/
            case 4096: /* 4096 Mbits Device Tech*/
            case 8192: /* 8192 Mbits Device Tech*/
               *puiStripeWidth =1; /* 128 bytes */
               *puiBankHeight = 1; /* 2 Mblks */
               break;

            default:
               *puiStripeWidth = 0;
               *puiBankHeight  = 1; /* 2 Mblks */
               BDBG_ERR(("Unknown MemPartSize: %d", uiMemPartSize));
               break;
         }

         break;

      /* Dram bus width 16 */
      case 16:

         switch(uiMemPartSize)
         {
            case 256: /* 256 Mbits Device Tech*/
               *puiStripeWidth =0; /* 64 bytes */
               *puiBankHeight = 0; /* 1 Mblks */
               break;

            case 512:  /* 512 Mbits Device Tech*/
            case 1024: /* 1024 Mbits Device Tech*/
            case 2048: /* 2048 Mbits Device Tech*/
            case 4096: /* 4096 Mbits Device Tech*/
            case 8192: /* 8192 Mbits Device Tech*/
               *puiStripeWidth =0; /* 64 bytes */
               *puiBankHeight = 1; /* 2 Mblks */
               break;

            default:
               *puiStripeWidth = 0; /* 64 bytes */
               *puiBankHeight  = 0; /* 1 Mblks */
               BDBG_ERR(("Unknown MemPartSize: %d", uiMemPartSize));
               break;
         }

         break;

      default:
         *puiStripeWidth = 0;
         *puiBankHeight  = 0;
         BDBG_ERR(("Unknown MemBusWidth: %d", uiMemBusWidth));
         break;
   }
}
#endif

extern BERR_Code BXVD_P_Relf_RelocateELF(BXVD_Handle hXvd,
                     uint8_t  *image,
                     uint32_t image_size,
                     uint32_t code_base,
                     uint32_t load_base,
                     uint32_t *end_of_code);

#if BXVD_P_USE_FW_LOAD_CORE_REVE0
#define BXVD_IMAGE_FW_CHUNK_SIZE 16384

#define BXVD_MSG_HEADER_FIELD(ptr, field) BXVD_DBG_MSG(hXvd, ("Image Header: %s = %d", #field, ((BXVD_AvdImgHdr *) ptr)->field))
#define BXVD_MSG_HEADER_FIELD_HEX(ptr, field) BXVD_DBG_MSG(hXvd, ("Image Header: %s = 0x%x", #field, ((BXVD_AvdImgHdr *) ptr)->field))

BERR_Code BXVD_P_ChunkFWDataToMem(uint8_t* pDestPtr,
                  uint32_t uiNumBytes,
                  uint32_t uiOffset,
                  BIMG_Method_Next ImageNextFunc,
                  void *pImage)
{
   BERR_Code rc;
   uint32_t uiStartChunk, uiEndChunk, i;
   const void *pImageDataChunk;
   uint32_t uiCopyOffset = 0;
   uint32_t uiBytesToCopy;
   uint32_t uiBytesLeft = uiNumBytes;

   BDBG_ENTER(BXVD_P_ChunkFWDataToMem);

   if (uiNumBytes == 0){

      BDBG_MSG(("Zero size image, FW image NOT loaded."));

      return BERR_TRACE(BERR_SUCCESS);
   }

   /* We read the firmware image in chunks of
    * BXVD_IMAGE_FW_CHUNK_SIZE.  This function allows us to copy
    * any byte offset and size from the image to memory.  In
    * order to do this, we need to handle the various cases:
    *
    * - Start Condition
    *  - uiOffset is 0 or a multiple of a chunk (We can start
    *    copying the chunk from the beginning)
    *  - uiOffset is not a multiple of a chunk (We need to start
    *    copying from an offset into the first chunk)
    * - Middle Condition
    *  - If the bytes left to copy is greater than the chunk size,
    *    then we can just copy the entire chunk
    * - End Condition
    *  - (uiOffset + uiNumBytes) ends on a chunk boundary (We can
    *    copy the last chunk until the end)
    *  - (uiOffset + uiNumBytes) does NOT end on a chunk boundary
    *    (We copy only a portion of the last chunk)
    */

   uiStartChunk = (uiOffset / BXVD_IMAGE_FW_CHUNK_SIZE) + 1;
   uiEndChunk = ((uiOffset + uiNumBytes - 1) / BXVD_IMAGE_FW_CHUNK_SIZE) + 1;
   if (uiOffset % BXVD_IMAGE_FW_CHUNK_SIZE) {
      uiCopyOffset = (uiOffset % BXVD_IMAGE_FW_CHUNK_SIZE);
   }

   for (i = uiStartChunk; i <= uiEndChunk; i++) {
      /* Retrieve next chunk */
      rc = ImageNextFunc(pImage,
                         i, /* The firmware chunks
                             * start from 1 */
                         &pImageDataChunk,
                         BXVD_IMAGE_FW_CHUNK_SIZE);
      if (rc != BERR_SUCCESS)
      {
         BDBG_ERR(("Error reading image chunk: %d", i));
         return BERR_TRACE(rc);
      }

      /* Determine how many bytes of this chunk we need to
       * copy */
      if (uiBytesLeft > BXVD_IMAGE_FW_CHUNK_SIZE - uiCopyOffset) {
         uiBytesToCopy = BXVD_IMAGE_FW_CHUNK_SIZE - uiCopyOffset;
      } else {
         uiBytesToCopy = uiBytesLeft;
      }


      /* Copy chunk to temporary region */
      BKNI_Memcpy(pDestPtr,
                  (uint8_t*)pImageDataChunk + uiCopyOffset,
                  uiBytesToCopy);

      pDestPtr += uiBytesToCopy;
      uiBytesLeft -= uiBytesToCopy;
      uiCopyOffset = 0;
   }

   BDBG_LEAVE(BXVD_P_ChunkFWDataToMem);

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code BXVD_P_LoadFWImageToMem(BXVD_Handle hXvd,
                                  BXVD_IMAGE_FirmwareID firmwareID,
                                  uint32_t uiDestPhysAddr,
                                  uint32_t *puiStartOfCode,
                                  uint32_t *puiEndOfCode)
{
   BERR_Code rc = BERR_SUCCESS;
   bool bTempAlloc = false;
   const BIMG_Interface *pImgInterface;
   void *pImgContext;
   void *pImage;
   void *pLoadAddress;
   uint32_t uiTempImageSize;
   uint8_t *pTempImageData;
   const void *pImageDataChunk;
   uint32_t uiImageOffset;

   BDBG_ENTER(BXVD_P_LoadFWImageToMem);
   BDBG_ASSERT(hXvd);
   BDBG_ASSERT(puiStartOfCode);
   BDBG_ASSERT(puiEndOfCode);

   pImgInterface = hXvd->stSettings.pImgInterface;
   pImgContext = hXvd->stSettings.pImgContext;

   BDBG_ASSERT(pImgInterface);
   BDBG_ASSERT(pImgContext);

   if (hXvd->uiFWMemSize < BXVD_P_FW_IMAGE_SIZE)
   {
      BXVD_DBG_ERR(hXvd, ("Error: insufficient firmware memory. Need at least %d bytes", BXVD_P_FW_IMAGE_SIZE));
      return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
   }

   /* Open Firmware Image */
   BXVD_DBG_MSG(hXvd, ("Opening firmware image ID: %d", firmwareID));
   rc = pImgInterface->open(pImgContext,
                            &pImage,
                            firmwareID);
   if (rc != BERR_SUCCESS)
   {
      BXVD_DBG_ERR(hXvd, ("Error opening image"));
      return BERR_TRACE(rc);
   }

   BXVD_DBG_MSG(hXvd, ("Reading firmware image header: %d bytes",
             sizeof(BXVD_AvdImgHdr)));

   /* Read Firmware Image Header */
   rc = pImgInterface->next(pImage,
                            0, /* Header is always chunk #0 */
                            &pImageDataChunk,
                            sizeof(BXVD_AvdImgHdr));
   if (rc != BERR_SUCCESS)
   {
      BXVD_DBG_ERR(hXvd, ("Error reading image header"));
      pImgInterface->close(pImage);
      return BERR_TRACE(rc);
   }

   /* Debug messages to print header */
   BXVD_MSG_HEADER_FIELD(pImageDataChunk, uiHeaderVersion);
   BXVD_MSG_HEADER_FIELD(pImageDataChunk, uiHeaderSize);
   BXVD_MSG_HEADER_FIELD(pImageDataChunk, uiImageSize);
   BXVD_MSG_HEADER_FIELD(pImageDataChunk, uiRelocationBase);

   /* Set up temporary memory for ELF image */
   uiTempImageSize = ((BXVD_AvdImgHdr *) pImageDataChunk)->uiImageSize;
   uiImageOffset = ((BXVD_AvdImgHdr *) pImageDataChunk)->uiRelocationBase;

   if (uiTempImageSize > hXvd->uiFWGenMemSize)
   {
      BXVD_DBG_MSG(hXvd, ("Heap too small, allocating temporary FW reloc memory"));
      pTempImageData = (uint8_t*)BKNI_Malloc(uiTempImageSize);
      if (pTempImageData == NULL)
      {
         BXVD_DBG_ERR(hXvd, ("Error: Insufficient temp memory for loading image"));
         pImgInterface->close(pImage);
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }
      BKNI_Memset((void *)pTempImageData, 0, uiTempImageSize);
      bTempAlloc = true;
      pLoadAddress = pTempImageData;
   }
   else
   {
      BXVD_DBG_MSG(hXvd, ("Temporarily using exising heap for FW relocation"));
      pTempImageData = (uint8_t*) hXvd->uiFWGenMemBaseVirtAddr;
      bTempAlloc = false;

      pLoadAddress = pTempImageData;
   }

   /* Get the firmware chunks */
   BXVD_DBG_MSG(hXvd, ("Reading firmware image data: %d bytes", uiTempImageSize));
   rc = BXVD_P_ChunkFWDataToMem(pLoadAddress,
                                uiTempImageSize,
                                0,
                                pImgInterface->next,
                                pImage);

   if (rc != BERR_SUCCESS) {
      pImgInterface->close(pImage);
      if (bTempAlloc == true)
         BKNI_Free(pLoadAddress);
      return BERR_TRACE(rc);
   }

#if BXVD_PRIV_USE_CACHED_MEMORY
   BMMA_FlushCache(hXvd->hFWGenMemBlock, (void *)pLoadAddress, uiNumBytes);
#endif

   BXVD_DBG_MSG(hXvd, ("Closing firmware image"));
   /* Close Firmware Image */
   pImgInterface->close(pImage);

   *puiStartOfCode = uiDestPhysAddr + uiImageOffset;
   BXVD_DBG_MSG(hXvd, ("Relocating ELF firmware image to 0x%x", *puiStartOfCode));
   /* Relocate the firmware image to specified memory address */
   rc = BXVD_P_Relf_RelocateELF(hXvd,
                                pLoadAddress,
                                uiTempImageSize,
                                BXVD_P_RELF_DEFAULT_CODE_BASE,
                                *puiStartOfCode,
                                puiEndOfCode);

   if (bTempAlloc == true)
   {
      BXVD_DBG_MSG(hXvd, ("Freeing temporary FW memory"));
      BKNI_Free(pLoadAddress);
   }


   BDBG_LEAVE(BXVD_P_LoadFWImageToMem);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_P_LoadAuthenticatedFWImageToMem(BXVD_Handle hXvd,
                           uint32_t *puiCmdBufferVector,
                           uint32_t *puiInstructionBaseOL,
                           uint32_t *puiEndOfCodeOL,
                           uint32_t *puiInstructionBaseIL,
                           uint32_t *puiEndOfCodeIL)
{
   BERR_Code rc = BERR_SUCCESS;
   void *pImage;
   BXVD_AvdImgHdr imageHeader;
   const void *pImageDataChunk;
   const BIMG_Interface *pImgInterface = NULL;
   void *pImgContext = NULL;
   BXVD_IMAGE_FirmwareID firmwareID;
   uint32_t uiDestPhyAddr;
   uint32_t uiDestVirtAddr;
   uint32_t uiDestSize;

   BDBG_ENTER(BXVD_P_LoadAuthenticatedFWImageToMem);
   BDBG_ASSERT(hXvd);

   pImgInterface = hXvd->stSettings.pImgInterface;
   pImgContext = hXvd->stSettings.pImgContext;
   firmwareID = hXvd->stPlatformInfo.stFirmware.authenticatedFirmwareID;
   uiDestPhyAddr = hXvd->FWMemBasePhyAddr;
   uiDestVirtAddr = hXvd->uiFWMemBaseVirtAddr;
   uiDestSize = hXvd->uiFWMemSize;

   BDBG_ASSERT(pImgInterface);
   BDBG_ASSERT(pImgContext);
   BDBG_ASSERT(puiCmdBufferVector);
   BDBG_ASSERT(puiInstructionBaseOL);
   BDBG_ASSERT(puiEndOfCodeOL);
   BDBG_ASSERT(puiInstructionBaseIL);
   BDBG_ASSERT(puiEndOfCodeIL);

   /* Open Firmware Image */
   BXVD_DBG_MSG(hXvd, ("Opening firmware image ID: %d", firmwareID));
   rc = pImgInterface->open(pImgContext,
                            &pImage,
                            firmwareID);
   if (rc == BERR_INVALID_PARAMETER) {
      /* An invalid parameter means that authenticated
       * firmware does not exist.  We fail silently here and
       * expect this error to be reported by the caller
       * depending on the situation */
      return rc;
   } else if (rc != BERR_SUCCESS) {
      BXVD_DBG_ERR(hXvd, ("Error opening image"));
      return BERR_TRACE(rc);
   }

   /* Read header */
   BXVD_DBG_MSG(hXvd, ("Reading authenticated firmware image header: %d bytes",
             sizeof(BXVD_AvdImgHdr)));

   /* Read Firmware Image Header */
   rc = pImgInterface->next(pImage,
                            0, /* Header is always chunk #0 */
                            &pImageDataChunk,
                            sizeof(BXVD_AvdImgHdr));
   if (rc != BERR_SUCCESS)
   {
      BXVD_DBG_ERR(hXvd, ("Error reading image header"));
      pImgInterface->close(pImage);
      return BERR_TRACE(rc);
   }

   BKNI_Memcpy(&imageHeader, pImageDataChunk, sizeof(BXVD_AvdImgHdr));
   /* Debug messages to print header */
   BXVD_MSG_HEADER_FIELD((&imageHeader), uiHeaderVersion);
   BXVD_MSG_HEADER_FIELD((&imageHeader), uiHeaderSize);
   BXVD_MSG_HEADER_FIELD((&imageHeader), uiImageSize);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), uiRelocationBase);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), outerCode.uiOffset);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), outerCode.uiAddress);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), outerCode.uiSize);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), innerCode.uiOffset);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), innerCode.uiAddress);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), innerCode.uiSize);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), commandBuffer.uiOffset);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), commandBuffer.uiAddress);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), commandBuffer.uiSize);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), outerData.uiOffset);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), outerData.uiAddress);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), outerData.uiSize);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), innerData.uiOffset);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), innerData.uiAddress);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), innerData.uiSize);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), signature.uiOffset);
   BXVD_MSG_HEADER_FIELD_HEX((&imageHeader), signature.uiAddress);
   BXVD_MSG_HEADER_FIELD((&imageHeader), signature.uiSize);

   /* Verify that the specified destination address matches the
    * firmware's relocated base address */
   if (uiDestPhyAddr != imageHeader.uiRelocationBase) {
      BXVD_DBG_ERR(hXvd, ("Error: FW relocation address (0x%x) does not match FW memory heap (0x%x)", imageHeader.uiRelocationBase, uiDestPhyAddr));
      pImgInterface->close(pImage);
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Verify that the image header is one we understand
    * firmware's relocated base address */
   if ((imageHeader.uiHeaderSize != sizeof(BXVD_AvdImgHdr)) ||
       (imageHeader.uiHeaderVersion != BXVD_AVD_IMG_HDR_VERSION)) {
      BXVD_DBG_ERR(hXvd, ("Error: incompatible firmware image header version/size"));
      pImgInterface->close(pImage);
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Verify there's enough firmware memory to load the image */
   if (imageHeader.uiImageSize > uiDestSize) {
      BXVD_DBG_ERR(hXvd, ("Error: insufficient firmware memory. Need at least %d bytes", imageHeader.uiImageSize));
      pImgInterface->close(pImage);
      return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
   }

   /* Handle normal vs. watchdog boot.  If we are booted
    * externally and are in watchdog, we should just reload the
    * data region, since we have to assume the code region is
    * locked down */
   if ((hXvd->stSettings.pAVDBootCallback != NULL) &&
       (hXvd->eAVDBootMode != BXVD_AVDBootMode_eNormal)) {
      /* Re-load just the data regions of authenticated image */
      BXVD_DBG_MSG(hXvd, ("Authenticated Firmware Load: Watchdog - data regions only"));
      /* Load Outer Loop Data */
      rc = BXVD_P_ChunkFWDataToMem((uint8_t*)(hXvd->uiFWMemBaseVirtAddr +
                                              (imageHeader.outerData.uiAddress - hXvd->FWMemBasePhyAddr)),
                                   imageHeader.outerData.uiSize,
                                   imageHeader.outerData.uiOffset,
                                   pImgInterface->next,
                                   pImage);
      if (rc != BERR_SUCCESS) {
         pImgInterface->close(pImage);
         return BERR_TRACE(rc);
      }

      /* Load Inner Loop Data */
      rc = BXVD_P_ChunkFWDataToMem((uint8_t*) (hXvd->uiFWMemBaseVirtAddr +
                                               (imageHeader.innerData.uiAddress - hXvd->FWMemBasePhyAddr)),
                                   imageHeader.innerData.uiSize,
                                   imageHeader.innerData.uiOffset,
                                   pImgInterface->next,
                                   pImage);
      if (rc != BERR_SUCCESS) {
         pImgInterface->close(pImage);
         return BERR_TRACE(rc);
      }
   } else {
      BXVD_DBG_MSG(hXvd, ("Authenticated Firmware Load: Normal"));
      /* Get the firmware chunks */
      rc = BXVD_P_ChunkFWDataToMem((uint8_t*) uiDestVirtAddr,
                                   imageHeader.uiImageSize,
                                   0,
                                   pImgInterface->next,
                                   pImage);

      if (rc != BERR_SUCCESS) {
         pImgInterface->close(pImage);
         return BERR_TRACE(rc);
      }
   }
   BXVD_DBG_MSG(hXvd, ("Closing firmware image"));
   /* Close Firmware Image */
   pImgInterface->close(pImage);

   /* Set CmdBufferAddr, OL EOC, and IL EOC */
   *puiCmdBufferVector = imageHeader.commandBuffer.uiAddress;

   *puiInstructionBaseOL = imageHeader.outerCode.uiAddress;
   *puiEndOfCodeOL = imageHeader.outerCode.uiSize;

   *puiInstructionBaseIL = imageHeader.innerCode.uiAddress;
   *puiEndOfCodeIL = imageHeader.innerCode.uiSize;

   BDBG_LEAVE(BXVD_P_LoadAuthenticatedFWImageToMem);

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code BXVD_P_FWLoad_RevE0(BXVD_Handle hXvd,
                  uint32_t uiDecoderInstance)
{
   uint32_t uiStartOfOLCode, uiStartOfILCode;
   uint32_t uiEndOfOLCode, uiEndOfILCode;
   void *pvCmdBuffer;

   BERR_Code rc;

   BDBG_ASSERT(hXvd);
   BDBG_ASSERT(hXvd->stSettings.pImgInterface);
   BDBG_ASSERT(hXvd->stSettings.pImgContext);
   BSTD_UNUSED(uiDecoderInstance);

   BDBG_ENTER(BXVD_P_FWLoad_RevE0);

   uiEndOfOLCode = 0;
   uiEndOfILCode = 0;

   BXVD_DBG_MSG(hXvd, ("Attempting load of authenticated firmware"));
   rc = BXVD_P_LoadAuthenticatedFWImageToMem(hXvd,
                         &hXvd->uiCmdBufferVector,
                         &hXvd->uiOuterLoopInstructionBase,
                         &hXvd->uiOuterLoopEOC,
                         &hXvd->uiInnerLoopInstructionBase,
                         &hXvd->uiInnerLoopEOC);
   if (rc == BERR_SUCCESS) {
      BXVD_DBG_MSG(hXvd, ("Authenticated firmware loaded"));
   } else {
      BXVD_DBG_MSG(hXvd, ("Authenticated firmware NOT loaded"));
      BXVD_DBG_MSG(hXvd, ("Attempting load of normal firmware"));

      BXVD_DBG_MSG(hXvd, ("Loading and relocating outer loop ELF image"));
      rc = BXVD_P_LoadFWImageToMem(hXvd,
                                   hXvd->stPlatformInfo.stFirmware.outerELFFirmwareID,
                                   hXvd->FWMemBasePhyAddr,
                                   &uiStartOfOLCode,
                                   &uiEndOfOLCode);

      if (rc != BERR_SUCCESS || uiEndOfOLCode == 0 || uiStartOfOLCode == 0)
      {
         BXVD_DBG_ERR(hXvd, ("Error loading outer loop"));
         return BERR_TRACE(rc);
      }

      hXvd->uiOuterLoopInstructionBase = uiStartOfOLCode;
      hXvd->uiCmdBufferVector = uiEndOfOLCode;
      hXvd->uiOuterLoopEOC = uiEndOfOLCode -  hXvd->uiOuterLoopInstructionBase;

      BXVD_DBG_MSG(hXvd, ("End of outer loop code at %x", hXvd->uiOuterLoopEOC));

      BXVD_DBG_MSG(hXvd, ("Loading and relocating inner loop ELF image"));
      rc = BXVD_P_LoadFWImageToMem(hXvd,
                                   hXvd->stPlatformInfo.stFirmware.innerELFFirmwareID,
                                   hXvd->FWMemBasePhyAddr,
                                   &uiStartOfILCode,
                                   &uiEndOfILCode);

      if (rc != BERR_SUCCESS || uiEndOfILCode == 0 || uiStartOfILCode == 0)
      {
         BXVD_DBG_ERR(hXvd, ("Error loading inner loop"));
         return BERR_TRACE(rc);
      }

      hXvd->uiInnerLoopInstructionBase = uiStartOfILCode;
      hXvd->uiInnerLoopEOC = uiEndOfILCode - hXvd->uiInnerLoopInstructionBase;
      BXVD_DBG_MSG(hXvd, ("End of inner loop code at %x", hXvd->uiInnerLoopEOC));
   }


   BMMA_FlushCache(hXvd->hFWMemBlock, (void *) hXvd->uiFWMemBaseVirtAddr, hXvd->uiFWMemSize);

   pvCmdBuffer = (void *)(hXvd->uiFWMemBaseVirtAddr + ((uint32_t)hXvd->uiCmdBufferVector - hXvd->FWMemBasePhyAddr));

   hXvd->stDecoderContext.ulCmdBufferAddr = (uint32_t) pvCmdBuffer;
   hXvd->stDecoderContext.ulRspBufferAddr = (uint32_t) pvCmdBuffer;

   BDBG_LEAVE(BXVD_P_FWLoad_RevE0);

   return BERR_TRACE(BERR_SUCCESS);
}
#endif /* BXVD_P_USE_FW_LOAD_CORE_REVE0 */

#ifdef BXVD_P_USE_SETUP_FW_MEMORY_REVE0

BERR_Code BXVD_P_SetupFWMemory_RevE0(BXVD_Handle hXvd)
{
   BMMA_AllocationSettings *pBMMASettings;
#if !BXVD_P_FW_HIM_API
   BMMA_AllocationSettings sBMMASettings;
#endif

   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BXVD_P_SetupFWMemory_RevE0);

   BDBG_ASSERT(hXvd);

   /* Allocate FW programs memory */
   if (hXvd->hFirmwareHeap != hXvd->hGeneralHeap)
   {
      hXvd->uiFWMemSize = BXVD_P_FW_IMAGE_SIZE + BXVD_P_FW_IMAGE_SIGN_SIZE;
   }
   else
   {
      hXvd->uiFWMemSize = BXVD_P_FW_IMAGE_SIZE;
   }

#if BXVD_P_FW_HIM_API
      pBMMASettings = NULL;
#else
      /* Request uncached mapping too */
      BMMA_GetDefaultAllocationSettings( &sBMMASettings);

      sBMMASettings.uncached = true;
      pBMMASettings = &sBMMASettings;
#endif

   hXvd->hFWMemBlock = BMMA_Alloc(hXvd->hFirmwareHeap, hXvd->uiFWMemSize, 4096, pBMMASettings);

   if (hXvd->hFWMemBlock == 0)
   {
      BXVD_DBG_ERR(hXvd, ("Insufficient device memory for FW program"));
      return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
   }

   /* Allocate FW programs memory */
   hXvd->uiFWMemBaseVirtAddr = (unsigned long) BMMA_Lock(hXvd->hFWMemBlock);

   if (hXvd->uiFWMemBaseVirtAddr == 0)
   {
      BXVD_DBG_ERR(hXvd, ("Insufficient device memory, FW memory can not be locked!"));
      return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
   }

   hXvd->FWMemBasePhyAddr = (BXVD_P_PHY_ADDR) BMMA_LockOffset(hXvd->hFWMemBlock);

   if (hXvd->FWMemBasePhyAddr == 0)
   {
      BXVD_DBG_ERR(hXvd, ("Insufficient device memory, FW memory can not be locked!"));
      return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
   }

#if !BXVD_P_FW_HIM_API
   hXvd->uiFWMemBaseUncachedVirtAddr = (uint32_t) BMMA_GetUncached(hXvd->hFWMemBlock);
#endif

   /* We set the Allocated flag to true, so that we know to free
    * this region */
   hXvd->bFWMemAllocated = true;

#if !BXVD_P_AVD_ARC600
   /* Pre rev K cores require FW code loaded below 768 MB */
   if ((hXvd->FWMemBasePhyAddr + hXvd->uiFWMemSize) >= BXVD_P_ARC300_RAM_LIMIT)
   {
      BXVD_DBG_ERR(hXvd, ("AVD Firmware code memory (%0x) allocated in region greater than 768MB!", hXvd->FWMemBasePhyAddr));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
#endif

   BXVD_DBG_MSG(hXvd, ("FWMem VA:0x%0lx, PA:" BDBG_UINT64_FMT,
                       hXvd->uiFWMemBaseVirtAddr, BDBG_UINT64_ARG ((BMMA_DeviceOffset)hXvd->FWMemBasePhyAddr)));

   /* Need to pad a bit since sub-allocations will need to be on 4k
    * boundaries. We need to account for 4K padding per channel
    * that we intend to have open.  Since we are in custom mode, we
    * will have at most BXVD_MAX_VIDEO_CHANNELS channels */
   hXvd->uiFWGenMemSize = hXvd->stSettings.stFWMemConfig.uiGeneralHeapSize;
   if (hXvd->uiFWGenMemSize > 0)
   {
      hXvd->uiFWGenMemSize +=  (BXVD_MAX_VIDEO_CHANNELS * 4096);
   }
   hXvd->uiFWCabacMemSize = hXvd->stSettings.stFWMemConfig.uiCabacHeapSize;
   hXvd->uiFWPicMemSize = hXvd->stSettings.stFWMemConfig.uiPictureHeapSize;
   hXvd->uiFWPicMem1Size = hXvd->stSettings.stFWMemConfig.uiPictureHeap1Size;

   /* Allocate context memory */
   if (hXvd->uiFWGenMemSize != 0)
   {
      /* The allocated buffer needs to be a multiple of the alignment size.
       * Get heap info to determine heap alignment. Adjust size accordingly. */
#if 0
      BMEM_Heap_GetInfo( hXvd->hGeneralHeap, &heapInfo);

      uiAlignment = heapInfo.uiAlignment;

      if ((hXvd->uiFWGenMemSize % (1<<uiAlignment)) != 0)
      {
         hXvd->uiFWGenMemSize = (((hXvd->uiFWGenMemSize / (1<<uiAlignment)) + 1) * (1<<uiAlignment));
      }
#endif

#if BXVD_P_FW_HIM_API
      pBMMASettings = NULL;
#else
      /* Request uncached mapping too */
      BMMA_GetDefaultAllocationSettings( &sBMMASettings);

      sBMMASettings.uncached = true;
      pBMMASettings = &sBMMASettings;
#endif

#if BXVD_P_CAPTURE_RVC
      hXvd->hFWRvcBlock =  BMMA_Alloc(hXvd->hGeneralHeap, (4 * 1024 * 1024), 1<<12, NULL);

      hXvd->uiFWRvcBaseVirtAddr = (uint32_t) BMMA_Lock(hXvd->hFWRvcBlock);
      hXvd->uiFWRvcBasePhyAddr = (uint32_t) BMMA_LockOffset(hXvd->hFWRvcBlock);

      BKNI_Memset((void *)hXvd->uiFWRvcBaseVirtAddr, 0,  (4 * 1024 * 1024));
#endif

      hXvd->hFWGenMemBlock =  BMMA_Alloc(hXvd->hGeneralHeap, hXvd->uiFWGenMemSize, 1<<12, pBMMASettings);

      if (hXvd->hFWGenMemBlock == 0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient context memory FW General heap"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      hXvd->uiFWGenMemBaseVirtAddr =  (unsigned long) BMMA_Lock(hXvd->hFWGenMemBlock);

      if (hXvd->uiFWGenMemBaseVirtAddr == 0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient device memory, general memory can not be locked!"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      hXvd->FWGenMemBasePhyAddr = (BXVD_P_PHY_ADDR) BMMA_LockOffset(hXvd->hFWGenMemBlock);

      if (hXvd->FWGenMemBasePhyAddr == 0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient device memory, general memory can not be locked!"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

#if BXVD_P_FW_HIM_API
      BXVD_DBG_MSG(hXvd, ("FWGenMem VA:0x%lx, PA:" BDBG_UINT64_FMT ", Size:0x%x",
                          hXvd->uiFWGenMemBaseVirtAddr,
                          BDBG_UINT64_ARG((BMMA_DeviceOffset)hXvd->FWGenMemBasePhyAddr),
                          hXvd->uiFWGenMemSize));
#else
      hXvd->uiFWGenMemBaseUncachedVirtAddr = (uint32_t) BMMA_GetUncached(hXvd->hFWGenMemBlock);

      BXVD_DBG_MSG(hXvd, ("FWGenMem VA:0x%x, UCVA: 0x%x PA:0x%x, Size:0x%x",
                          hXvd->uiFWGenMemBaseVirtAddr,
                          hXvd->uiFWGenMemBaseUncachedVirtAddr,
                          hXvd->FWGenMemBasePhyAddr,
                          hXvd->uiFWGenMemSize));
#endif
   }

   /* Allocate cabac/secure memory */
   if (hXvd->uiFWCabacMemSize != 0)
   {
#if 0
      /* The allocated buffer needs to be a multiple of the alignment size.
       * Get heap info to determine heap alignment. Adjust size accordingly. */

      BMEM_Heap_GetInfo( hXvd->hCabacHeap, &heapInfo);

      uiAlignment = heapInfo.uiAlignment;

      if ((hXvd->uiFWCabacMemSize % (1<<uiAlignment)) != 0)
      {
         hXvd->uiFWCabacMemSize = (((hXvd->uiFWCabacMemSize / (1<<uiAlignment)) + 1) * (1<<uiAlignment));
      }
#endif

      hXvd->hFWCabacMemBlock = BMMA_Alloc(hXvd->hCabacHeap, hXvd->uiFWCabacMemSize, 1<<8, 0);

      if (hXvd->hFWCabacMemBlock == 0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient FW cabac memory data heap"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      hXvd->FWCabacMemBasePhyAddr = (BXVD_P_PHY_ADDR) BMMA_LockOffset(hXvd->hFWCabacMemBlock);

      BXVD_DBG_MSG(hXvd, ("FWCabacMem PA:" BDBG_UINT64_FMT ", Size:0x%x",
                          BDBG_UINT64_ARG((BMMA_DeviceOffset)hXvd->FWCabacMemBasePhyAddr),
                          hXvd->uiFWCabacMemSize));

      if (hXvd->FWCabacMemBasePhyAddr == 0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient device memory, CABAC memory can not be locked!"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }
   }
   else
   {
      /* Cabac is coming out of general heap */

      hXvd->hFWCabacMemBlock = hXvd->hFWGenMemBlock;
      hXvd->FWCabacMemBasePhyAddr =  hXvd->FWGenMemBasePhyAddr;
   }

   /* Allocate picture memory */
   if (hXvd->uiFWPicMemSize != 0)
   {
      hXvd->hFWPicMemBlock = BMMA_Alloc(hXvd->hPictureHeap, hXvd->uiFWPicMemSize, 1<<12, 0);

      if (hXvd->hFWPicMemBlock == 0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient FW picture memory heap space"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      hXvd->FWPicMemBasePhyAddr = (BXVD_P_PHY_ADDR) BMMA_LockOffset(hXvd->hFWPicMemBlock);

      BXVD_DBG_MSG(hXvd, ("FWPictMem PA: " BDBG_UINT64_FMT ", FWPictMemSize:0x%08x",
                          BDBG_UINT64_ARG((BMMA_DeviceOffset)hXvd->FWPicMemBasePhyAddr), hXvd->uiFWPicMemSize));

      if (hXvd->FWPicMemBasePhyAddr == 0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient device memory, picture memory can not be locked!"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

#if BXVD_DM_ENABLE_YUV_GRAB_MODE
      if (hXvd->uDecoderInstance== 0)
      {
         hXvd->uiFWPicMemBaseVirtAddr = (unsigned long) BMMA_Lock(hXvd->hFWPicMemBlock);

         BXVD_DBG_MSG(hXvd, ("FWPictMem VA:0x%lx, Size:0x%x",
                             hXvd->uiFWPicMemBaseVirtAddr,
                             hXvd->uiFWPicMemSize));
      }
#endif

   }
   if (hXvd->uiFWPicMem1Size != 0)
   {
      hXvd->hFWPicMem1Block = BMMA_Alloc(hXvd->hPictureHeap1, hXvd->uiFWPicMem1Size, 1<<12, 0);

      if (hXvd->hFWPicMem1Block == 0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient FW picture memory 1 heap space"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      hXvd->FWPicMem1BasePhyAddr = (BXVD_P_PHY_ADDR) BMMA_LockOffset(hXvd->hFWPicMem1Block);

      BXVD_DBG_MSG(hXvd, ("FWPictMem PA:" BDBG_UINT64_FMT ", Size:0x%x",
                          BDBG_UINT64_ARG((BMMA_DeviceOffset)hXvd->FWPicMem1BasePhyAddr),
                          hXvd->uiFWPicMem1Size));

      if (hXvd->FWPicMem1BasePhyAddr == 0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient device memory, picture memory 1 can not be locked!"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }


#if BXVD_DM_ENABLE_YUV_GRAB_MODE
      hXvd->uiFWPicMem1BaseVirtAddr = (unsigned long) BMMA_Lock(hXvd->hFWPicMem1Block);

      BXVD_DBG_MSG(hXvd, ("FWPictMem VA:0x%x, Size:0x%x",
                          hXvd->uiFWPicMem1BaseVirtAddr,
                          hXvd->uiFWPicMem1Size));
#endif

   }

   if (hXvd->stSettings.uiDecoderDebugLogBufferSize != 0)
   {
      hXvd->hFWDbgBuf_MemBlock = BMMA_Alloc( hXvd->hGeneralHeap,
                                             hXvd->stSettings.uiDecoderDebugLogBufferSize,
                                             1<<12, 0);

      if (hXvd->hFWDbgBuf_MemBlock == 0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient context memory FW Debug Log"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      hXvd->uiDecoderDbgBufVirtAddr = (unsigned long) BMMA_Lock( hXvd->hFWDbgBuf_MemBlock);

      if (hXvd->uiDecoderDbgBufVirtAddr == 0)
      {
         BXVD_DBG_ERR(hXvd, ("Insufficient device memory, video decoder debug memory can not be locked!"));
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      hXvd->DecoderDbgBufPhyAddr = (BXVD_P_PHY_ADDR) BMMA_LockOffset( hXvd->hFWDbgBuf_MemBlock);

      BXVD_DBG_MSG(hXvd, ("Decoder Debug Log VA:0x%lx, PA:"BDBG_UINT64_FMT" , Size:0x%x",
                          hXvd->uiDecoderDbgBufVirtAddr,
                          BDBG_UINT64_ARG((BMMA_DeviceOffset)hXvd->DecoderDbgBufPhyAddr),
                          hXvd->stSettings.uiDecoderDebugLogBufferSize));
   }

   BDBG_LEAVE(BXVD_P_SetupFWMemory_RevE0);

   return BERR_TRACE(rc);
}
#endif /* BXVD_P_USE_SETUP_FW_MEMORY_REVE0 */




#ifdef BXVD_P_USE_TEAR_DOWN_FW_MEMORY_REVE0
BERR_Code BXVD_P_TearDownFWMemory_RevE0(BXVD_Handle hXvd)
{
   BDBG_ENTER(BXVD_P_TearDownFWMemory_RevE0);

   BDBG_ASSERT(hXvd);

   if ((hXvd->hFWCabacMemBlock != NULL) && (hXvd->hFWGenMemBlock != hXvd->hFWCabacMemBlock))
   {
      BMMA_UnlockOffset(hXvd->hFWCabacMemBlock, hXvd->FWCabacMemBasePhyAddr);
      BMMA_Free(hXvd->hFWCabacMemBlock);
   }

   if ( hXvd->hFWGenMemBlock != NULL)
   {
      BMMA_Unlock(hXvd->hFWGenMemBlock, (void *)hXvd->uiFWGenMemBaseVirtAddr);
      BMMA_UnlockOffset(hXvd->hFWGenMemBlock, hXvd->FWGenMemBasePhyAddr);
      BMMA_Free(hXvd->hFWGenMemBlock);
   }

#if BXVD_P_CAPTURE_RVC
   if ( hXvd->hFWRvcBlock != NULL)
   {
      BMMA_Unlock(hXvd->hFWRvcBlock, (void *)hXvd->uiFWRvcBaseVirtAddr);
      BMMA_UnlockOffset(hXvd->hFWRvcBlock, hXvd->uiFWRvcBasePhyAddr);
      BMMA_Free(hXvd->hFWRvcBlock);
   }
#endif

   if (hXvd->hFWPicMemBlock != NULL)
   {
      BMMA_UnlockOffset(hXvd->hFWPicMemBlock, hXvd->FWPicMemBasePhyAddr);
#if BXVD_DM_ENABLE_YUV_GRAB_MODE
      BMMA_Unlock(hXvd->hFWPicMemBlock, (void *)hXvd->uiFWPicMemBaseVirtAddr);
#endif
      BMMA_Free(hXvd->hFWPicMemBlock);
   }

   if (hXvd->hFWPicMem1Block != NULL)
   {
      BMMA_UnlockOffset(hXvd->hFWPicMem1Block, hXvd->FWPicMem1BasePhyAddr);
#if BXVD_DM_ENABLE_YUV_GRAB_MODE
      BMMA_Unlock(hXvd->hFWPicMem1Block, (void *)hXvd->FWPicMem1BaseVirtAddr);
#endif
      BMMA_Free(hXvd->hFWPicMem1Block);
   }

   if (hXvd->hFWMemBlock != NULL)
   {
      BMMA_Unlock(hXvd->hFWMemBlock, (void *)hXvd->uiFWMemBaseVirtAddr);
      BMMA_UnlockOffset(hXvd->hFWMemBlock, hXvd->FWMemBasePhyAddr);
      BMMA_Free(hXvd->hFWMemBlock);
   }

   if (hXvd->hFWDbgBuf_MemBlock != NULL)
   {
      BMMA_Unlock( hXvd->hFWDbgBuf_MemBlock, (void *)hXvd->uiDecoderDbgBufVirtAddr);
      BMMA_UnlockOffset( hXvd->hFWDbgBuf_MemBlock, hXvd->DecoderDbgBufPhyAddr);
      BMMA_Free(hXvd->hFWDbgBuf_MemBlock);
   }

   BDBG_LEAVE(BXVD_P_TearDownFWMemory_RevE0);

   return BERR_TRACE(BERR_SUCCESS);
}
#endif /* BXVD_P_USE_TEAR_DOWN_FW_MEMORY_REVE0 */

#if BXVD_P_USE_CORE_CHIP_ENABLE_REVE0

BERR_Code BXVD_P_ChipEnable_RevE0(BXVD_Handle hXvd)
{
   volatile uint32_t uiFWBootStatus;

   BERR_Code rc = BERR_SUCCESS;

#if BXVD_POLL_FW_MBX
   uint32_t uiVal;
   int loopCount;
#endif

   BDBG_ASSERT(hXvd);
   BDBG_ENTER(BXVD_P_ChipEnable_RevE0);

   /* Write to VectorTB, CpuDbg registers and AuxRegs
    * in THIS ORDER to begin 7401 operation
    */

   /* program the relocation base address for outer-loop */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterInstructionBase,
                    hXvd->uiOuterLoopInstructionBase);

   /* program the outer loop image end of code address */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterEndOfCode,
                    hXvd->uiOuterLoopEOC);

   /* program the relocation base address for inner-loop */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_InnerInstructionBase,
                    hXvd->uiInnerLoopInstructionBase);

   /* program the inner loop image end of code address */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_InnerEndOfCode,
                    hXvd->uiInnerLoopEOC);

   /* program global IO base register - Outer */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterGlobalIOBase,
                    BCHP_PHYSICAL_OFFSET);

   /* program global IO base register - Inner */
   BXVD_Reg_Write32(hXvd,
            hXvd->stPlatformInfo.stReg.uiDecode_InnerGlobalIOBase,
            BCHP_PHYSICAL_OFFSET);

   /* Clear out any previously completed FW command done events */
   BKNI_ResetEvent(hXvd->stDecoderContext.hFWCmdDoneEvent);

   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterHost2CPUMailbox,
                    BXVD_P_AVD_CORE_UART_FREQ);

#if BXVD_POLL_FW_MBX

   /* Initialize MBX to non-zero */
   BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPU2HostMailbox, 0xff);

#endif

   /* Boot the core */
   if (hXvd->stSettings.pAVDBootCallback)
   {
      /* Call external boot callback */
      rc = (*hXvd->stSettings.pAVDBootCallback)(hXvd->stSettings.pAVDBootCallbackData,
                                                hXvd->eAVDBootMode);
      if (rc != BERR_SUCCESS)
      {
         BXVD_DBG_ERR(hXvd, ("Error #%d booting the AVD externally", rc));
         return BERR_TRACE(rc);
      }
   }
   else
   {
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUDebug,
                       1);
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUAux,
                       0 );
      BXVD_Reg_Write32(hXvd,
                       hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUDebug,
                       0);
   }

#define FW_CMD_TIMEOUT 1000

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
         BKNI_Sleep(10);

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

   BDBG_LEAVE(BXVD_P_ChipEnable_RevE0);
   return BERR_TRACE(rc);
}

#endif /* BXVD_P_USE_CORE_CHIP_ENABLE_REVE0 */

#ifdef BXVD_P_USE_CORE_RESET_CHIP_REVE0

BERR_Code BXVD_P_ChipReset_RevE0(BXVD_Handle hXvd)
{
   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_ChipReset_RevE0);

   /* Stop OL AVD ARC */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUDebug,
                    1);
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUAux,
                    0x02000000);

   /* Stop IL AVD ARC */
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_InnerCPUDebug,
                    1);
   BXVD_Reg_Write32(hXvd,
                    hXvd->stPlatformInfo.stReg.uiDecode_InnerCPUAux,
                    0x02000000);

   /* Reset AVD HW blocks */
   BXVD_P_RESET_CORE( hXvd, hXvd->stPlatformInfo.stReg.uiDecode_CabacBinCtl,
                      hXvd->stPlatformInfo.stReg.uiDecode_CabacBinCtl_ResetMask,
                      "CABAC");

   BXVD_P_RESET_CORE( hXvd, hXvd->stPlatformInfo.stReg.uiDecode_SintStrmSts,
                      hXvd->stPlatformInfo.stReg.uiDecode_SintStrmSts_ResetMask,
                      "Stream");

   BXVD_P_RESET_CORE( hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OLSintStrmSts,
                      hXvd->stPlatformInfo.stReg.uiDecode_OLSintStrmSts_ResetMask,
                      "OLoopStream");

   BXVD_P_RESET_CORE( hXvd, hXvd->stPlatformInfo.stReg.uiDecode_MainCtl,
                      hXvd->stPlatformInfo.stReg.uiDecode_MainCtl_ResetMask,
                      "Backend" )

   /* Reset Video Decoder */
   BREG_AtomicUpdate32(hXvd->hReg,
                       hXvd->stPlatformInfo.stReg.uiSun_SWReset,
                       hXvd->stPlatformInfo.stReg.uiSun_SWResetAVDMask,
                       hXvd->stPlatformInfo.stReg.uiSun_SWResetAVDMask);

   BREG_AtomicUpdate32(hXvd->hReg,
                       hXvd->stPlatformInfo.stReg.uiSun_SWReset,
                       hXvd->stPlatformInfo.stReg.uiSun_SWResetAVDMask,
                       0);

   /* Clear interrupt mask (Enable) register to prevent interrupts before ISR is setup */
   BXVD_Reg_Write32(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptMask, 0);

   BDBG_LEAVE(BXVD_P_ChipReset_RevE0);

   return BERR_TRACE(BERR_SUCCESS);
}

#endif /* BXVD_P_USE_CORE_CHIP_RESET_REVE0 */

#ifdef BXVD_P_USE_VERIFY_WATCHDOG_FIRED_REVE0

bool BXVD_P_VerifyWatchdogFired_RevE0_isr( BXVD_Handle hXvd,
                                           int param2 )
{
   volatile uint32_t     uIntrVal;
   volatile uint32_t     uiOL_pc;
   int i;

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_P_VerifyWatchdogFired_RevE0_isr);

   if ( param2 == BXVD_P_OUTER_WATCHDOG)
   {
      uIntrVal = BXVD_Reg_Read32_isr(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterInterruptClear);

      BXVD_DBG_MSG(hXvd, ("L3IntClr:%08x", uIntrVal));

      if ( uIntrVal & BXVD_P_INTR_OL_MASK)
      {
         BXVD_DBG_MSG(hXvd, ("Outer Loop Watchdog"));
         BXVD_DBG_ERR(hXvd, ("OL watchdog - firmware revision: %x", hXvd->sRevisionInfo.ulDecoderFwRev));

         BXVD_Reg_Write32_isr(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUDebug, 1);

         /* Read FW OL PC to help with debugging watchdog timeouts */
         for (i = 0; i < 8; i++)
         {
             /* read the AVD OL PC */
             uiOL_pc = BXVD_Reg_Read32_isr(hXvd, hXvd->stPlatformInfo.stReg.uiDecode_OuterCPUAux);

             BXVD_DBG_ERR(hXvd, ("[%d] AVD_%d: OL PC=%08x", i, hXvd->uDecoderInstance, uiOL_pc));
         }

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

   BDBG_LEAVE(BXVD_P_VerifyWatchdogFired_RevE0_isr);

   return true;
}

#endif /*  BXVD_P_USE_VERIFY_WATCHDOG_FIRED_REVE0 */

#if BXVD_P_USE_SET_POWER_STATE_REVE0

void BXVD_P_SetPowerState_RevE0(BXVD_Handle hXvd,
                                BXVD_P_PowerState PowerStateNew)
{

   /* Check to if PM reg pointers are NULL */
   if ( (hXvd->stPlatformInfo.stReg.uiClkGen_CoreClkCtrl == 0) &&
        (hXvd->stPlatformInfo.stReg.uiVCXO_AVDCtrl == 0) &&
        (hXvd->stPlatformInfo.stReg.uiClkGen_SCBClkCtrl == 0) &&
        (hXvd->stPlatformInfo.stReg.uiClkGen_GISBClkCtrl == 0) )
   {
      BXVD_DBG_WRN(hXvd, ("XVD Powermanagement is not supported on this platform!"));

      return;
   }

   if (PowerStateNew == BXVD_P_PowerState_ePwrOff)
   {
      /* PwrOff is not supported in this platform */
      PowerStateNew = BXVD_P_PowerState_eClkOff;
   }

   if ((hXvd->PowerStateCurrent == BXVD_P_PowerState_eOn) &&
       (PowerStateNew == BXVD_P_PowerState_eClkOff))
   {
      BXVD_DBG_MSG(hXvd, ("Turn off clocks"));

      /* AVD Coreclock */
      BREG_AtomicUpdate32(hXvd->hReg,
                          hXvd->stPlatformInfo.stReg.uiClkGen_CoreClkCtrl,
                          hXvd->stPlatformInfo.stReg.uiClkGen_CoreClkCtrl_PwrDnMask,
                          hXvd->stPlatformInfo.stReg.uiClkGen_CoreClkCtrl_PwrDnMask);

      if (hXvd->stPlatformInfo.stReg.uiVCXO_AVDCtrl != (uint32_t)NULL)
      {
         /* Powerdown control for AVD PLL */
         BREG_AtomicUpdate32(hXvd->hReg,
                             hXvd->stPlatformInfo.stReg.uiVCXO_AVDCtrl,
                             hXvd->stPlatformInfo.stReg.uiVCXO_AVDCtrl_PwrDnMask,
                             hXvd->stPlatformInfo.stReg.uiVCXO_AVDCtrl_PwrDnMask);
      }

      /* PWRDN_CLOCK_216_CG_AVD SCB Memory clock */
      BREG_AtomicUpdate32(hXvd->hReg,
                          hXvd->stPlatformInfo.stReg.uiClkGen_SCBClkCtrl,
                          hXvd->stPlatformInfo.stReg.uiClkGen_SCBClkCtrl_PwrDnMask,
                          hXvd->stPlatformInfo.stReg.uiClkGen_SCBClkCtrl_PwrDnMask);

      /* PWRDN_CLOCK_108_CG_AVD GISB clock*/
      BREG_AtomicUpdate32(hXvd->hReg,
                          hXvd->stPlatformInfo.stReg.uiClkGen_GISBClkCtrl,
                          hXvd->stPlatformInfo.stReg.uiClkGen_GISBClkCtrl_PwrDnMask,
                          hXvd->stPlatformInfo.stReg.uiClkGen_GISBClkCtrl_PwrDnMask);

      hXvd->PowerStateCurrent = PowerStateNew;
   }
   else if ((hXvd->PowerStateCurrent == BXVD_P_PowerState_eClkOff) &&
            (PowerStateNew == BXVD_P_PowerState_eOn))
   {
      BXVD_DBG_MSG(hXvd, ("Turn on clocks"));

      /* PWRDN_CLOCK_108_CG_AVD */
      BREG_AtomicUpdate32(hXvd->hReg,
                          hXvd->stPlatformInfo.stReg.uiClkGen_GISBClkCtrl,
                          hXvd->stPlatformInfo.stReg.uiClkGen_GISBClkCtrl_PwrDnMask,
                          0);

      /* PWRDN_CLOCK_216_CG_AVD */
      BREG_AtomicUpdate32(hXvd->hReg,
                          hXvd->stPlatformInfo.stReg.uiClkGen_SCBClkCtrl,
                          hXvd->stPlatformInfo.stReg.uiClkGen_SCBClkCtrl_PwrDnMask,
                          0);

      if (hXvd->stPlatformInfo.stReg.uiVCXO_AVDCtrl != (uint32_t)NULL)
      {
         /* Powerdown control for AVD PLL */
         BREG_AtomicUpdate32(hXvd->hReg,
                             hXvd->stPlatformInfo.stReg.uiVCXO_AVDCtrl,
                             hXvd->stPlatformInfo.stReg.uiVCXO_AVDCtrl_PwrDnMask,
                             0);
      }

      /* Wait for PLLs to lock */
      BKNI_Sleep(1);

      /* CLKGEN_AVD_CTRL, Powerdown control for AVD PLL */
      BREG_AtomicUpdate32(hXvd->hReg,
                          hXvd->stPlatformInfo.stReg.uiClkGen_CoreClkCtrl,
                          hXvd->stPlatformInfo.stReg.uiClkGen_CoreClkCtrl_PwrDnMask,
                          0);

      hXvd->PowerStateCurrent = PowerStateNew;
   }
}
#endif

#if BXVD_P_USE_SET_POWER_STATE_REVE0_PM2

void BXVD_P_SetPowerState_RevE0_PM2(BXVD_Handle hXvd,
                                    BXVD_P_PowerState PowerStateNew)
{
   BCHP_PWR_ResourceId clkResourceId = BCHP_PWR_RESOURCE_AVD0;

#if BCHP_PWR_RESOURCE_AVD1
   if (hXvd->uDecoderInstance == 1)
   {
      clkResourceId = BCHP_PWR_RESOURCE_AVD1;
   }
#endif

   BDBG_MSG(("BXVD_P_SetPowerState, Decoder:%d Cur: %d New: %d",
             hXvd->uDecoderInstance, hXvd->PowerStateCurrent , PowerStateNew));

   if (PowerStateNew == BXVD_P_PowerState_ePwrOff)
   {
      /* PwrOff is not supported in this platform */
      PowerStateNew = BXVD_P_PowerState_eClkOff;
   }

   if (hXvd->PowerStateCurrent != PowerStateNew)
   {
      if ((hXvd->PowerStateCurrent == BXVD_P_PowerState_eOn) &&
          (PowerStateNew == BXVD_P_PowerState_eClkOff))
      {
         BDBG_MSG(("Release AVD_Clk Resource"));
         BCHP_PWR_ReleaseResource(hXvd->hChip, clkResourceId);
      }

      if ((hXvd->PowerStateCurrent != BXVD_P_PowerState_eOn) &&
          (PowerStateNew == BXVD_P_PowerState_eOn))
      {
         BDBG_MSG(("Acquire AVD_CLK Resource"));
         BCHP_PWR_AcquireResource(hXvd->hChip, clkResourceId);
      }

      hXvd->PowerStateCurrent = PowerStateNew;
   }
}

#endif /* BXVD_P_USE_SET_POWER_STATE_7420 */
