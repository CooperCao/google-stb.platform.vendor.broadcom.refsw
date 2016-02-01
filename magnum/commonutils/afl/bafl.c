/***************************************************************************
 *     Copyright (c) 2009-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h"
#include "bdbg.h"
#include "berr.h"
#include "bimg.h"
#include "bkni.h"

#include "bafl.h"
#include "bafl_elf.h"

BDBG_MODULE(BAFL);

/* Uncomment the following line to include more detailed debug messages */
/* #define BAFL_DEBUG_DETAIL_MSGS 1 */

#define BAFL_MAX_SHDR          128
#define LSRAM 0x30000000

#define BAFL_IMAGE_FW_BLOCK_SIZE 16384

typedef struct
{
      int32_t    swap_required;
      BAFL_Elf32_Ehdr header;
      BAFL_Elf32_Shdr section_header[BAFL_MAX_SHDR];
} ElfInfo;

typedef struct
{
      uint32_t uiCurImgBlock;
      unsigned long uiCurImgBlockAddr;
} BAFL_P_ImageInfo;

/* Get a 32 bit value and swap it if necessary.	*/
#define BAFL_P_Elf_Swap32(n)            \
         n = ((n & 0xFF000000) >> 24) | \
	     ((n & 0x00FF0000) >>  8) | \
	     ((n & 0x0000FF00) <<  8) | \
             ((n & 0x000000FF) << 24);

/* Get a 16 bit value and swap it if necessary.	*/
#define BAFL_P_Elf_Swap16(n)         \
         n = ((n & 0xFF00) >> 8) |   \
             ((n & 0x00FF) << 8);

static BERR_Code BAFL_P_CopyFWImageToMem( uint8_t* pDestPtr,
                                         uint32_t uiNumBytes,
                                         uint32_t uiOffset,
                                         BAFL_P_ImageInfo *pImageInfo,
                                         BIMG_Method_Next ImageNextFunc,
                                         void *pImage,
                                         bool bByteSwap);

BERR_Code BAFL_Load ( const uint32_t uiDevice,
                      const BIMG_Interface *pImgInterface,
                      void **pImageContext,
                      uint32_t uiInstance,
                      void *pStartAddress, /* Virtual Address */
                      size_t uiMemSize,
                      bool bDataOnly,
                      BAFL_FirmwareLoadInfo *pstFWLoadInfo )
{
   ElfInfo *pElfInfo;
   BAFL_P_ImageInfo stImageInfo;

   BAFL_ImageHeader *pImageHeader;
   void       *pImageData;

   BAFL_Elf32_Ehdr *pElfHdr;   /* ELF header */
   BAFL_Elf32_Shdr *pSHdr;     /* Section table header */

   uint32_t uiTransSize;

   uint32_t uiSHdr;
   uint32_t i;

   unsigned long uiDestBase;

   bool bFoundDataSection = false;

   void *pImage;

   BERR_Code rc = BERR_SUCCESS;

   BDBG_ASSERT(pImgInterface);

   BDBG_MSG(("Opening firmware image ID: %d", uiInstance));

   rc = pImgInterface->open(pImageContext,
                            &pImage,
                            uiInstance);

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   pImageData = (void *) &pImageHeader;
   rc = pImgInterface->next(pImage,
                            0,
                            pImageData,
                            BAFL_IMAGE_HDR_SIZE);
   if (rc != BERR_SUCCESS)
   {
      BDBG_ERR(("unable to read image header"));
      return BERR_TRACE(rc);
   }

   if (pImageHeader->uiDevice != uiDevice)
   {
      BDBG_ERR(("Incorrect image load device:%08x for intended device: %08x ", pImageHeader->uiDevice, uiDevice));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   stImageInfo.uiCurImgBlock = 0xffff;
   stImageInfo.uiCurImgBlockAddr = 0;

   pElfInfo = (ElfInfo *) BKNI_Malloc(sizeof(ElfInfo));

   if (!pElfInfo)
   {
      BDBG_ERR(("unable to allocate Elf Info structure"));
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   BKNI_Memset((void *)pElfInfo, 0, sizeof(ElfInfo));

   rc = BAFL_P_CopyFWImageToMem((uint8_t *)&(pElfInfo->header), /* pDestPtr */
                                sizeof(BAFL_Elf32_Ehdr),        /* uiNumBytes */
                                0,                              /* uiOffset */
                                &stImageInfo,
                                pImgInterface->next,
                                pImage,
                                false); /* No byte swap */

   if (rc != BERR_SUCCESS)
   {
      BKNI_Free(pElfInfo);
      return rc;
   }

#if (BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE)
   pElfInfo->swap_required = (pElfInfo->header.e_ident[BAFL_EI_DATA] == BAFL_ELFDATA2LSB) ? 0 : 1;
#else
   pElfInfo->swap_required = (pElfInfo->header.e_ident[BAFL_EI_DATA] == BAFL_ELFDATA2LSB) ? 1 : 0;
#endif

   BDBG_MSG(("Header %s endianess of system - byteswapping %s occur (data=%d)",
             pElfInfo->swap_required?"does not match":"matches",
             pElfInfo->swap_required?"will":"will not",
             pElfInfo->header.e_ident[BAFL_EI_DATA]));

   if (pElfInfo->swap_required)
   {
      BAFL_P_Elf_Swap16(pElfInfo->header.e_type);
      BAFL_P_Elf_Swap16(pElfInfo->header.e_machine);
      BAFL_P_Elf_Swap32(pElfInfo->header.e_version);
      BAFL_P_Elf_Swap32(pElfInfo->header.e_entry);
      BAFL_P_Elf_Swap32(pElfInfo->header.e_phoff);
      BAFL_P_Elf_Swap32(pElfInfo->header.e_shoff);
      BAFL_P_Elf_Swap32(pElfInfo->header.e_flags);
      BAFL_P_Elf_Swap16(pElfInfo->header.e_ehsize);
      BAFL_P_Elf_Swap16(pElfInfo->header.e_phentsize);
      BAFL_P_Elf_Swap16(pElfInfo->header.e_phnum);
      BAFL_P_Elf_Swap16(pElfInfo->header.e_shentsize);
      BAFL_P_Elf_Swap16(pElfInfo->header.e_shnum);
      BAFL_P_Elf_Swap16(pElfInfo->header.e_shstrndx);
   }

   pElfHdr = (BAFL_Elf32_Ehdr *) &(pElfInfo->header);

#if BAFL_DEBUG_DETAIL_MSGS
   BDBG_MSG(("Elf32_Ehdr"));
   BDBG_MSG(("\te_type: %0x", (uint32_t) pElfHdr->e_type));
   BDBG_MSG(("\te_machine: %d", (uint32_t) pElfHdr->e_machine));
   BDBG_MSG(("\te_version: %d", (uint32_t) pElfHdr->e_version));
   BDBG_MSG(("\te_entry: %0x", (uint32_t) pElfHdr->e_entry));
   BDBG_MSG(("\te_phoff: 0x%0x", (uint32_t) pElfHdr->e_phoff));
   BDBG_MSG(("\te_shoff: 0x%0x", (uint32_t) pElfHdr->e_shoff));
   BDBG_MSG(("\te_flags: 0x%0x", (uint32_t) pElfHdr->e_flags));
   BDBG_MSG(("\te_ehsize: %d", (uint32_t) pElfHdr->e_ehsize));
   BDBG_MSG(("\te_phentsize: %d", (uint32_t) pElfHdr->e_phentsize));
   BDBG_MSG(("\te_phnum: %d", (uint32_t) pElfHdr->e_phnum));
   BDBG_MSG(("\te_shentsize: %d", (uint32_t) pElfHdr->e_shentsize));
   BDBG_MSG(("\te_shnum: %d", (uint32_t) pElfHdr->e_shnum));
   BDBG_MSG(("\te_shstrndx: %d", (uint32_t) pElfHdr->e_shstrndx));
#endif

   /* Read in ELF Section Table */

   uiSHdr = pElfHdr->e_shoff; /* Section table offset */
   pSHdr    = &pElfInfo->section_header[0];

   uiTransSize = sizeof(BAFL_Elf32_Shdr) * pElfInfo->header.e_shnum;

   rc = BAFL_P_CopyFWImageToMem((uint8_t *)pSHdr,     /* pDestPtr */
                                uiTransSize,          /* uiNumBytes */
                                uiSHdr,               /* uiOffset */
                                &stImageInfo,
                                pImgInterface->next,
                                pImage,
                                false); /* No byte swap */
   if (rc != BERR_SUCCESS)
   {
      BKNI_Free(pElfInfo);
      return rc;
   }

   for (i = 0; i < pElfInfo->header.e_shnum; i++)
   {
      if (pElfInfo->swap_required)
      {
         BAFL_P_Elf_Swap32(pSHdr->sh_name);
         BAFL_P_Elf_Swap32(pSHdr->sh_type);
         BAFL_P_Elf_Swap32(pSHdr->sh_flags);
         BAFL_P_Elf_Swap32(pSHdr->sh_addr);
         BAFL_P_Elf_Swap32(pSHdr->sh_offset);
         BAFL_P_Elf_Swap32(pSHdr->sh_size);
         BAFL_P_Elf_Swap32(pSHdr->sh_link);
         BAFL_P_Elf_Swap32(pSHdr->sh_info);
         BAFL_P_Elf_Swap32(pSHdr->sh_addralign);
         BAFL_P_Elf_Swap32(pSHdr->sh_entsize);
      }

#if BAFL_DEBUG_DETAIL_MSGS
      {
         BDBG_MSG((" "));
         BDBG_MSG(("Sect hdr: %d", i));
         BDBG_MSG(( "pSHdr->sh_name: %0x", (uint32_t) pSHdr->sh_name));
         BDBG_MSG(( "pSHdr->sh_type: %0x", (uint32_t) pSHdr->sh_type));
         BDBG_MSG(( "pSHdr->sh_flags: %0x", (uint32_t) pSHdr->sh_flags));
         BDBG_MSG(( "pSHdr->sh_addr: %0x", (uint32_t) pSHdr->sh_addr));
         BDBG_MSG(( "pSHdr->sh_offset: %0x", (uint32_t) pSHdr->sh_offset));
         BDBG_MSG(( "pSHdr->sh_size: %0x", (uint32_t) pSHdr->sh_size));
         BDBG_MSG(( "pSHdr->sh_link: %0x", (uint32_t) pSHdr->sh_link));
         BDBG_MSG(( "pSHdr->sh_info: %0x", (uint32_t) pSHdr->sh_info));
         BDBG_MSG(( "pSHdr->sh_addralign: %0x", (uint32_t) pSHdr->sh_addralign));
         BDBG_MSG(( "pSHdr->sh_entsize: %0x", (uint32_t) pSHdr->sh_entsize));
      }
#endif
      pSHdr++;
   }

/* Copy section data */

   BDBG_MSG(("Number of sections: %d", pElfInfo->header.e_shnum));

   for (i = 1; i < pElfInfo->header.e_shnum; i++)
   {
      if ( pElfInfo->section_header[i].sh_size == 0 )
      {
#if BAFL_DEBUG_DETAIL_MSGS
         BDBG_MSG(("section size = 0... skipping"));
#endif
         continue;
      }

      if ((pElfInfo->section_header[i].sh_flags & BAFL_SHF_ALLOC) == 0)
      {
#if BAFL_DEBUG_DETAIL_MSGS
         BDBG_MSG(("sh_flags & SHF_ALLOC... skipping"));
#endif
         continue;
      }

      if ( pElfInfo->section_header[i].sh_type != BAFL_SHT_PROGBITS )
      {
#if BAFL_DEBUG_DETAIL_MSGS
         BDBG_MSG(("not a PROGBITS section... skipping"));
#endif
         continue;
      }

      if ( pElfInfo->section_header[i].sh_addr >= LSRAM )
      {
#if BAFL_DEBUG_DETAIL_MSGS
         BDBG_MSG(("section addr >= 0x30000000... done"));
#endif
         break;
      }

      if ( !(pElfInfo->section_header[i].sh_flags & BAFL_SHF_EXECINSTR ) )
      {
         BDBG_MSG(("Load Data section"));
         if (bFoundDataSection == false)
         {
            bFoundDataSection = true;

            pstFWLoadInfo->stCode.pStartAddress = pStartAddress;
            pstFWLoadInfo->stCode.uiSize = pElfInfo->section_header[i].sh_addr;

            pstFWLoadInfo->stData.pStartAddress = (uint8_t*)pStartAddress + pElfInfo->section_header[i].sh_addr;
            pstFWLoadInfo->stData.uiSize = uiMemSize - pElfInfo->section_header[i].sh_addr;
         }
      }
      else if (bDataOnly)
      {
         BDBG_MSG(("Load data only... Skip executable section"));
         continue;
      }
      else
      {
         BDBG_MSG(("Load executable section"));
      }

      /* Calculate the base address of this section where the image is being copied to. */
      uiDestBase = (unsigned long)(pElfInfo->section_header[i].sh_addr) + (unsigned long)pStartAddress;

#if BAFL_DEBUG_DETAIL_MSGS
      BDBG_MSG(("Load Section %d", i));
      BDBG_MSG(("\tFW Base Address: %0x", pStartAddress));
      BDBG_MSG(("\tSection Base: %08x", uiDestBase));
      BDBG_MSG(("\tDest Offset: %08x", pElfInfo->section_header[i].sh_addr));
      BDBG_MSG(("\tImageSectoffset: %0x", pElfInfo->section_header[i].sh_offset));
#endif

      if (pElfInfo->section_header[i].sh_size +  pElfInfo->section_header[i].sh_offset > uiMemSize)
      {
         BDBG_ERR(("FW memory size too small for image"));
         return (BERR_INVALID_PARAMETER);
      }

      rc = BAFL_P_CopyFWImageToMem((uint8_t *)uiDestBase,                 /* pDestPtr */
                                   pElfInfo->section_header[i].sh_size,   /* uiNumBytes */
                                   pElfInfo->section_header[i].sh_offset, /* uiOffset */
                                   &stImageInfo,
                                   pImgInterface->next,
                                   pImage,
                                   pElfInfo->swap_required);

      if (rc != BERR_SUCCESS)
      {
         BKNI_Free(pElfInfo);
         return rc;
      }
   }

   pImgInterface->close(pImage);

   BDBG_MSG(("LoadInfo Code Start Address: %0lx, Size: %08x",
             (unsigned long) pstFWLoadInfo->stCode.pStartAddress, pstFWLoadInfo->stCode.uiSize));

   BDBG_MSG(("LoadInfo Data Start Address: %0lx, Size: %08x",
             (unsigned long) pstFWLoadInfo->stData.pStartAddress, pstFWLoadInfo->stData.uiSize));

   BKNI_Free(pElfInfo);

   return (BERR_SUCCESS);
}

static void BAFL_ByteSwapMemcpy(uint8_t *pDestPtr,
                                uint8_t *pSrcPtr,
                                uint32_t uiBytesToCopy)
{
   unsigned long  word;
   unsigned long  *pTmp;
   unsigned long  uiBytePos;

   /* Check to see if starting destination ptr on word boundary */
   if ( ((unsigned long)pDestPtr & 0x00000003) != 0x00)
   {
      pTmp = (unsigned long*) ((unsigned long)pDestPtr & 0xfffffffc);

      word = *pTmp;

      uiBytePos = (unsigned long) pDestPtr - (unsigned long) pTmp;

      switch (uiBytePos)
      {
         case 1:
            word &= 0xffff00ff;
            word |= ((*pSrcPtr & 0xff) <<  8);
            pSrcPtr++;
            /* coverity[unterminated_case] */
         case 2:
            word &= 0xff00ffff;
            word |= ((*pSrcPtr & 0xff) <<  16);
            pSrcPtr++;
            /* coverity[unterminated_case] */
         case 3:
            word &= 0x00ffffff;
            word |= ((*pSrcPtr & 0xff) <<  24);
            pSrcPtr++;
      }

      *pTmp = word;

      pDestPtr += (4 - uiBytePos);
      uiBytesToCopy -= (4 - uiBytePos);
   }

   /* Copy long word aligned word length data */
   while (uiBytesToCopy > 3)
   {
      word = 0;

      word = ((pSrcPtr[3] & 0xff) << 24) |
             ((pSrcPtr[2] & 0xff) << 16) |
             ((pSrcPtr[1] & 0xff) <<  8) |
             ((pSrcPtr[0] & 0xff) <<  0);

      *(unsigned long *)pDestPtr = word;

      pSrcPtr += 4;
      pDestPtr += 4;
      uiBytesToCopy  -= 4;
   }

   /* Copy last non word data if exists */
   if (uiBytesToCopy > 0)
   {
      word = 0;
      switch (uiBytesToCopy)
      {
         case 3: word |= (pSrcPtr[2] & 0xff) << 16;
         /* coverity[unterminated_case] */
         case 2: word |= (pSrcPtr[1] & 0xff) <<  8;
         /* coverity[unterminated_case] */
         case 1: word |= (pSrcPtr[0] & 0xff) <<  0;
            break;
      }

      *(unsigned long *)pDestPtr = word;
   }
}


/*
 * BAFL_P_CoptFWImageToMem
 *
 * This routine copies a number of bytes from the specified offset of the image to
 * the destination address. This routine uses the bimg interface to access the appropriate
 * block(s) of the image. The block that is currently open is saved from call to call.
 */

BERR_Code BAFL_P_CopyFWImageToMem(uint8_t* pDestPtr,
                                  uint32_t uiNumBytes,
                                  uint32_t uiOffset,
                                  BAFL_P_ImageInfo *pImageInfo,
                                  BIMG_Method_Next ImageNextFunc,
                                  void *pImage,
                                  bool bByteSwap)
{
   BERR_Code rc;
   uint32_t uiStartBlock, uiEndBlock, i;
   const void *pImageDataBlock;
   uint32_t uiCopyOffset = 0;
   uint32_t uiBytesToCopy;
   uint32_t uiBytesLeft = uiNumBytes;

   BDBG_ENTER(BAFL_P_CopyFWImageToMem);

   if (uiNumBytes == 0){

      BDBG_MSG(("Zero size image, FW image NOT loaded."));

      return BERR_TRACE(BERR_SUCCESS);
   }

#if BAFL_DEBUG_DETAIL_MSGS
   BDBG_MSG(("CopyFWImageToMem: size is %x at address %x", uiNumBytes, pDestPtr));
#endif

   /* Read the firmware image in blocks of BAFL_IMAGE_FW_BLOCK_SIZE.
    * This function will copy any byte offset and size from the
    * image to memory. In order to do this, this routine needs to
    * handle the various cases:
    *
    * - Start Condition
    *  o uiOffset is 0 or a multiple of a block (We can start
    *    copying the block from the beginning)
    *  o uiOffset is not a multiple of a block (We need to start
    *    copying from an offset into the first block)
    * - Middle Condition
    *  o If the bytes left to copy is greater than the block size,
    *    then we can just copy the entire block
    * - End Condition
    *  o (uiOffset + uiNumBytes) ends on a block boundary (We can
    *    copy the last block until the end)
    *  o (uiOffset + uiNumBytes) does NOT end on a block boundary
    *    (We copy only a portion of the last block)
    */

   uiStartBlock = (uiOffset / BAFL_IMAGE_FW_BLOCK_SIZE) + 1;
   uiEndBlock = ((uiOffset + uiNumBytes - 1) / BAFL_IMAGE_FW_BLOCK_SIZE) + 1;

   if (uiOffset % BAFL_IMAGE_FW_BLOCK_SIZE)
   {
      uiCopyOffset = (uiOffset % BAFL_IMAGE_FW_BLOCK_SIZE);
   }

   for (i = uiStartBlock; i <= uiEndBlock; i++)
   {
      if (i == pImageInfo->uiCurImgBlock)
      {
         pImageDataBlock = (void *)pImageInfo->uiCurImgBlockAddr;
      }
      else
      {
         /* Retrieve next block */
         rc = ImageNextFunc(pImage,
                            i,
                            &pImageDataBlock,
                            BAFL_IMAGE_FW_BLOCK_SIZE);

         if (rc != BERR_SUCCESS)
         {
            BDBG_ERR(("Error reading image block: %d", i));
            return BERR_TRACE(rc);
         }

         pImageInfo->uiCurImgBlock = i;
         pImageInfo->uiCurImgBlockAddr = (unsigned long)pImageDataBlock;
      }

      /* Determine how many bytes of this block we need to
       * copy */
      if (uiBytesLeft > (BAFL_IMAGE_FW_BLOCK_SIZE - uiCopyOffset))
      {
         uiBytesToCopy = BAFL_IMAGE_FW_BLOCK_SIZE - uiCopyOffset;
      }
      else
      {
         uiBytesToCopy = uiBytesLeft;
      }

#if BAFL_DEBUG_DETAIL_MSGS
      BDBG_MSG(("pDestPtr: %08x Bytes: %d", pDestPtr, uiBytesToCopy));
#endif

      if (bByteSwap == true)
      {
         BAFL_ByteSwapMemcpy(pDestPtr,
                             (uint8_t*)pImageDataBlock + uiCopyOffset,
                             uiBytesToCopy);
      }
      else
      {
         /* Copy image block to FW memory region */
         BKNI_Memcpy(pDestPtr,
                     (uint8_t*)pImageDataBlock + uiCopyOffset,
                     uiBytesToCopy);
      }

      pDestPtr += uiBytesToCopy;
      uiBytesLeft -= uiBytesToCopy;
      uiCopyOffset = 0;
   }

   BDBG_LEAVE(BAFL_P_CopyFWImageToMem);

   return BERR_TRACE(BERR_SUCCESS);
}
