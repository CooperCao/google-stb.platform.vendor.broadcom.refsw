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

/************************* Module Overview ********************************
<verbatim>
*
* Our relocation strategy assumes that the program was linked in the
* following fashion:
*
*    +------+
*    |      | <-- code_base
*    | text |   |
*    |      |   | data_offset
*    +------+   V
*    |      |
*    | data |
*    |      |
*    +------+
*    |      |
*    | bss  |
*    |      |
*    +------+
*     ... other
*
* That is, the code (text) and data are linked more-or-less contiguously,
* followed by bss.
*
* The layout is characterized by two values: code_base and data_offset.
* The first gives the base of the link (typically 00000000, but it could
* be anything), and the second gives the offset from code_base to the
* beginning of the data section ( ** NOT ** the address of the data_section).
*
* We also need a third parameter (load_base), which tells us where in
* physical memory you wish the image relocated/loaded.  We relocate the
* image such that code_base --> load_base.
*
* Except...
*
* We relocate anything in code sections as though code_base --> 00000000.
* The code gets loaded to locations starting at load_base, but relocation
* is done as though code were being loaded to 00000000.
*
* When you run the ARC you must load:
*
*   DecCx_RegCodeBase <-- load_base
*   DecCx_RegCodeEnd  <-- data_offset
*
* Load the code at the addresses emitted by RelocateELF and then start the
* ARC at location 00000000.
*
</verbatim>
****************************************************************************/
#include "bstd.h"                /* standard types */
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"                /* malloc, free, snprintf */
#include "bxvd.h"
#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd_errors.h"
#include "bxvd_relf.h"

#if BXVD_P_USE_RELF

BDBG_MODULE(BXVD_RELF);

#ifndef UINT32_C
#define UINT32_C(value) ((uint_least32_t)(value ## UL))
#endif

#define BXVD_MAX_SHDR          128
#define BXVD_OUTPUT_MAX_SIZE   32

/* Set this to 1 if you want tons of debug output */
#define BXVD_RELOC_DEBUG_DETAIL_MSGS 0

/*
 * Set this to 1 if you want BXVD_P_Relf_WriteOutput to use
 * cached memory
 */
#define BXVD_RELF_WRITE_CACHED_MEMORY 0

#define BXVD_RELF_BLOCK_1_SIZE (32 * 1024)
#define BXVD_RELF_BLOCK_2_SIZE (128 * 1024)

/* Working context structure */
typedef struct
{
      int32_t    swap_required;
      uint32_t   code_base;
      uint32_t   load_base;
      Bxvd_Elf32_Ehdr header;
      Bxvd_Elf32_Shdr section_header[BXVD_MAX_SHDR];
      BMMA_Block_Handle hBlock[BXVD_MAX_SHDR];
      uint8_t     *section_data[BXVD_MAX_SHDR];
      uint8_t     *uncached_section_data[BXVD_MAX_SHDR];
      uint32_t   new_section_base[BXVD_MAX_SHDR];
      Bxvd_Elf32_Sym  *symtab;
      int32_t    symcount;
      BMMA_Block_Handle hMemBlock1;
      BMMA_Block_Handle hMemBlock2;
      int32_t    iMemBlk1Used;
      int32_t    iMemBlk2Used;
      uint8_t    *pMemBlk1;
      uint8_t    *pMemBlk2;
      uint8_t    *image;
      uint32_t   image_size;
} ElfInfo;

/* Local prototypes */
static BERR_Code BXVD_P_Relf_ReadHeaders(BXVD_Handle hXvd, ElfInfo *info);

static uint8_t   *BXVD_P_Relf_ReadSection(BXVD_Handle hXvd,
                                          ElfInfo *info,
                                          int32_t MemBlk,
                                          int32_t index);

static void      BXVD_P_Relf_RelocateSections(ElfInfo *info);
static void      BXVD_P_Relf_RelocateSymbols(ElfInfo *info);
static int32_t   BXVD_P_Relf_FindRelocSection(ElfInfo *info,
                                              uint32_t prog_index);
static void      BXVD_P_Relf_DoRelocationA(ElfInfo *info,
                                           int32_t prog_index,
                                           int32_t rel_index);
static void      BXVD_P_Relf_Fixup(uint8_t *data,
                                   unsigned pc,
                                   unsigned targetAddress,
                                   int32_t type,
                                   int32_t addend);
static BERR_Code BXVD_P_Relf_ReadInput(ElfInfo *info,
                                       uint8_t *buffer,
                                       uint32_t offset,
                                       uint32_t length);

/*
 * Since we don't support the use of inline functions in magnum, the following
 * three functions have been converted to macros. These functions were
 * identified as bottlenecks during execution profiling.
 */

/* Get a 32 bit value and swap it if necessary. */
#define BXVD_P_Relf_Get32(swap, n)      \
do {                                    \
      if (swap)                         \
      {                                 \
         n = ((n & 0xFF000000) >> 24) | \
            ((n & 0x00FF0000) >>  8) |  \
            ((n & 0x0000FF00) <<  8) |  \
            ((n & 0x000000FF) << 24);   \
      }                                 \
} while(0)

/* Get a 16 bit value and swap it if necessary. */
#define BXVD_P_Relf_Get16(swap, n)                      \
do {                                                    \
      if (swap)                                         \
      {                                                 \
         n = ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8); \
      }                                                 \
} while(0)

/* Release a previously allocated section. */
/* LSRAM start address. Nothing >= this address is processed */
#if BXVD_USE_UNCACHED_MEMORY
#define BXVD_P_Relf_ReleaseSection(hXvd, info, index)           \
do {                                                                    \
      if (info->section_data[index])                                    \
      {                                                                 \
         info->section_data[index   ] = 0;                          \
         info->uncached_section_data[index] = 0;                        \
      }                                                                 \
} while(0)
#else

#define BXVD_P_Relf_ReleaseSection(hXvd, info, index)           \
do {                                                                    \
      if (info->section_data[index])                                    \
      {                                                                 \
         info->section_data[index   ] = 0;                          \
         info->uncached_section_data[index] = 0;                        \
      }                                                                 \
} while(0)
#endif

/* LSRAM start address. Nothing >= this address is processed */
#define LSRAM 0x30000000

/*----------------------------------------------------------------------*
 *              Porting layer                                           *
 *----------------------------------------------------------------------*/

/* Format the block into QuickSim format (one word/line; no
 * ranges).  Note that we byte-swap; the ELF files for the ARC
 * appear to be little endian, and we want these 32-bit values
 * to be bit-correct (i.e. bit 31 of a word in memory should be
 * bit 31 of a word in the QuickSim output).
 */
static void BXVD_P_Relf_WriteOutput( BXVD_Handle hXvd,
                                     uint32_t addr,
                                     uint8_t *block,
                                     int32_t size)
{
#if BXVD_RELF_WRITE_CACHED_MEMORY
   void *ctemp;
#endif
   uint32_t virt_addr, word;

   BDBG_MSG(("BXVD_P_Relf_WriteOutput: size is %x at address %x", size, addr));

   virt_addr = hXvd->uiFWMemBaseVirtAddr + (addr - hXvd->uiFWMemBasePhyAddr);

   while ( size > 3 )
   {
      word = ((block[3] & 0xFF) << 24) |
         ((block[2] & 0xFF) << 16) |
         ((block[1] & 0xFF) <<  8) |
         ((block[0] & 0xFF) <<  0);

      *(uint32_t *)virt_addr = word;

      addr  += 4;
      block += 4;
      size  -= 4;
      virt_addr += 4;
   }

   if ( size > 0 )
   {
      word = 0;
      virt_addr += 4;
      switch ( size )
      {
         case 3: word |= (block[2] & 0xFF) << 16;
         case 2: word |= (block[1] & 0xFF) <<  8;
         case 1: word |= (block[0] & 0xFF) <<  0;
            break;
      }
      *(uint32_t *)virt_addr = word;
   }

#if BXVD_RELF_WRITE_CACHED_MEMORY
   BMMA_FlushCache(hXvd->hFWGenMemBlock, (void *)virt_addr, size);
#endif
}

/*----------------------------------------------------------------------*
 *              Entry Point                                             *
 *----------------------------------------------------------------------*/

/* Read and relocate and ELF file.  The ELF file is assumed to
 * already reside in memory at image[]; the total number of bytes
 * in the ELF image is given by image_size.
 *
 * Output is produced via the BXVD_P_Relf_WriteBlock() function.
 *
 * The two parameters (code_base and load_base) define the
 * relocation as described in the comments at the top of the file.
 */
BERR_Code BXVD_P_Relf_RelocateELF(BXVD_Handle hXvd,
                                  uint8_t  *image,
                                  uint32_t image_size,
                                  uint32_t code_base,
                                  uint32_t load_base,
                                  uint32_t *end_of_code)
{
   bool     found_data_section;
   int32_t  i;
   int32_t  r;

   BMMA_Block_Handle hMemBlock1, hMemBlock2;
   uint8_t *pMemBlock1, *pMemBlock2;
   ElfInfo  *info;

   uint32_t base;
   BERR_Code status = BERR_SUCCESS;

   BDBG_ENTER(BXVD_P_Relf_RelocateELF);

#if BXVD_USE_UNCACHED_MEMORY
   BDBG_MSG(("using uncached memory"));
#else
   BDBG_MSG(("using cached memory"));
#endif

   /* Allocate and initialize our local context. */

   hMemBlock1 =  BMMA_Alloc(hXvd->hGeneralHeap,
                            BXVD_RELF_BLOCK_1_SIZE, /*sizeof(ElfInfo), */
                            1<<4,
                            0);

   if (!hMemBlock1)
   {
      BDBG_ERR(("unable to allocate Relf MemBlock1"));
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   hMemBlock2 =  BMMA_Alloc(hXvd->hGeneralHeap,
                            BXVD_RELF_BLOCK_2_SIZE, /* sizeof(ElfInfo), */
                            1<<4,
                            0);

   if (!hMemBlock2)
   {
      BDBG_ERR(("unable to allocate Relf MemBlock2"));
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   pMemBlock1 = (uint8_t *)BMMA_Lock(hMemBlock1);
   pMemBlock2 = (uint8_t *)BMMA_Lock(hMemBlock2);

   info = (ElfInfo *)pMemBlock1;

   BKNI_Memset(pMemBlock1, 0, BXVD_RELF_BLOCK_1_SIZE);
   BKNI_Memset(pMemBlock2, 0, BXVD_RELF_BLOCK_2_SIZE);

   found_data_section = false;

   BDBG_MSG(("image = 0x%0*lx image size = %d(%x), code_base = %x, load_base = %x",
             BXVD_P_DIGITS_IN_LONG, (long)image, image_size, image_size, code_base, load_base));

   info->code_base   = code_base;
   info->load_base   = load_base;
   info->image       = image;
   info->image_size  = image_size;

   info->pMemBlk1 = pMemBlock1;
   info->pMemBlk2 = pMemBlock2;

   info->iMemBlk1Used = sizeof(ElfInfo);

   /* Read the control information from the ELF image. */
   BDBG_MSG(("Reading ELF image headers"));
   if ( BXVD_P_Relf_ReadHeaders(hXvd, info) != 0 )
   {
      BDBG_ERR(("failed reading header information"));
      BKNI_Free(info);
      return BERR_TRACE(BXVD_ERR_RELF_BAD_HEADER);
   }

   /* Do part 1 of the relocation: move the sections and symbols. */
   BDBG_MSG(("Relocating sections"));
   BXVD_P_Relf_RelocateSections(info);
   BDBG_MSG(("Relocating symbols"));
   BXVD_P_Relf_RelocateSymbols (info);

   /* Do part 2 of the relocation: relocate and emit each loadable
    *   PROGBITS section.
    */
   for (i = 1; i < info->header.e_shnum; i++)
   {
#if BXVD_RELOC_DEBUG_DETAIL_MSGS
      BDBG_MSG(("Relocating PROGBITS section %d", i));
#endif
      if ( info->section_header[i].sh_size == 0 )
      {
#if BXVD_RELOC_DEBUG_DETAIL_MSGS
         BDBG_MSG(("section size = 0... skipping"));
#endif
         continue;
      }
      if ((info->section_header[i].sh_flags & BXVD_SHF_ALLOC) == 0)
      {
#if BXVD_RELOC_DEBUG_DETAIL_MSGS
         BDBG_MSG(("sh_flags & SHF_ALLOC... skipping"));
#endif
         continue;
      }
      if ( info->section_header[i].sh_type != BXVD_SHT_PROGBITS )
      {
#if BXVD_RELOC_DEBUG_DETAIL_MSGS
         BDBG_MSG(("not a PROGBITS section... skipping"));
#endif
         continue;
      }
      if ( info->section_header[i].sh_addr >= LSRAM )
      {
#if BXVD_RELOC_DEBUG_DETAIL_MSGS
         BDBG_MSG(("section addr >= 0x30000000... done"));
#endif
         break;
      }

      info->iMemBlk2Used = 0;

      /* Read sections */
      if ( ! BXVD_P_Relf_ReadSection(hXvd, info, 2, i))
      {
         BDBG_MSG(("unable to read program data in RelocateELF()"));
         status = BXVD_ERR_RELF_BAD_SECTION;
         break;
      }

      /* See if we can find a relocation section for this program section.  If
       *    there isn't one then we don't need to modify this section.
       */
      r = BXVD_P_Relf_FindRelocSection(info, i);
      if (r)
      {
         if (info->section_header[r].sh_type == BXVD_SHT_REL)
         {
            BDBG_ERR(("relocation type REL unsupported for ARC"));
            status = BXVD_ERR_RELF_BAD_RELOC_TYPE;
            break;
         }

         if ( ! BXVD_P_Relf_ReadSection(hXvd, info, 2, r))
            {
               BDBG_ERR(("unable to read relocation data in RelocateELF()"));
               status = BXVD_ERR_RELF_BAD_SECTION;
               break;
            }

            BXVD_P_Relf_DoRelocationA(info, i, r);
            BXVD_P_Relf_ReleaseSection(hXvd, info, r);
            }


         /* Determine the base address of this section in the output.
          * This is different for code vs. data because code is always
          * relocated to zero, but loaded somewhere else (load_base).
          * Data is relocated to its absolute address.
          */
         if ( info->section_header[i].sh_flags & BXVD_SHF_EXECINSTR )
         {
            base = info->new_section_base[i] + info->load_base;
         }
         else
         {
            base = info->new_section_base[i];
            if (found_data_section == false)
            {
               *end_of_code = base;
               found_data_section = true;
            }
         }

         /* Move relocated code to FW memory one section at a time */
         BXVD_P_Relf_WriteOutput(hXvd,
                                 base,
                                 info->section_data[i],
                                 info->section_header[i].sh_size);

         BXVD_P_Relf_ReleaseSection(hXvd, info, i);
      }


      /* Release anything that we allocated along the way. */
      for (i = 0; i < info->header.e_shnum; i++)
      {
         if ( info->section_data[i] )
            BXVD_P_Relf_ReleaseSection(hXvd, info, i);
      }

      /* If we didn't detect EOC, signal an error since the image is
       * probably corrupt
       */
      if (*end_of_code == 0)
      {
         BDBG_ERR(("EOC == 0!"));
         status = BXVD_ERR_RELF_NO_EOC_FOUND;
      }

      /* Free the heap we used for the info structure  */
      BMMA_Unlock(hMemBlock2, pMemBlock2);
      BMMA_Unlock(hMemBlock1, pMemBlock1);

      BMMA_Free(hMemBlock2);
      BMMA_Free(hMemBlock1);

      /* Get out cleanly or signal an error if we picked one up along the way */
      BDBG_LEAVE(BXVD_P_Relf_RelocateELF);

      if (status == 0)
         return status;
      else
         return BERR_TRACE(status);
   }

/*
 * Read the ELF image into the supplied local buffer
 */
   static BERR_Code BXVD_P_Relf_ReadInput (ElfInfo *info,
                                           uint8_t *buffer,
                                           uint32_t offset,
                                           uint32_t length)
   {
      if ( offset >= info->image_size )
      {
         BDBG_ERR(("BXVD_P_Relf_ReadInput error: offset (%x) >= info->image_size (%x)",
                   offset, info->image_size));
         return BERR_TRACE(BXVD_ERR_RELF_BAD_INPUT);
      }

      if ( (offset + length) > info->image_size )
      {
         BDBG_ERR(("BXVD_P_Relf_ReadInput error:(offset + length)(%x) > info->image_size (%x)",
                   (offset + length), info->image_size));
         return BERR_TRACE(BXVD_ERR_RELF_BAD_INPUT);
      }

      BKNI_Memcpy(buffer, info->image + offset, length);

      return BERR_TRACE(BERR_SUCCESS);
   }

/*----------------------------------------------------------------------*
 *              ELF input and parsing                                   *
 *----------------------------------------------------------------------*/

/* Read the header information from the ELF image.  This function
 * reads the following sections:
 *
 * - the ELF header
 * - the section headers
 * - the dynamic symbol table
 */
   static BERR_Code BXVD_P_Relf_ReadHeaders( BXVD_Handle hXvd, ElfInfo * info)
   {
      int32_t    i;
      uint32_t   offset;
      uint32_t   n;
      Bxvd_Elf32_Shdr *hdr;

      /* Read the ELF header. */
      if (BXVD_P_Relf_ReadInput(info,
                                (uint8_t *)&(info->header),
                                0,
                                sizeof(Bxvd_Elf32_Ehdr)) != 0)
      {
         BDBG_ERR(("unable to read ELF header"));
         return BERR_TRACE(BXVD_ERR_RELF_BAD_HEADER);
      }

#if (BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE)
      info->swap_required = (info->header.e_ident[BXVD_EI_DATA] == BXVD_ELFDATA2LSB) ? 0 : 1;
#else
      info->swap_required = (info->header.e_ident[BXVD_EI_DATA] == BXVD_ELFDATA2LSB) ? 1 : 0;
#endif

      BDBG_MSG(("Header %s endianess of system - byteswapping %s occur (data=%d)",
                info->swap_required?"does not match":"matches",
                info->swap_required?"will":"will not",
                info->header.e_ident[BXVD_EI_DATA]));

      BXVD_P_Relf_Get16(info->swap_required, info->header.e_type);
      BXVD_P_Relf_Get16(info->swap_required, info->header.e_machine);
      BXVD_P_Relf_Get32(info->swap_required, info->header.e_version);
      BXVD_P_Relf_Get32(info->swap_required, info->header.e_entry);
      BXVD_P_Relf_Get32(info->swap_required, info->header.e_phoff);
      BXVD_P_Relf_Get32(info->swap_required, info->header.e_shoff);
      BXVD_P_Relf_Get32(info->swap_required, info->header.e_flags);
      BXVD_P_Relf_Get16(info->swap_required, info->header.e_ehsize);
      BXVD_P_Relf_Get16(info->swap_required, info->header.e_phentsize);
      BXVD_P_Relf_Get16(info->swap_required, info->header.e_phnum);
      BXVD_P_Relf_Get16(info->swap_required, info->header.e_shentsize);
      BXVD_P_Relf_Get16(info->swap_required, info->header.e_shnum);
      BXVD_P_Relf_Get16(info->swap_required, info->header.e_shstrndx);


      /* Read the section headers. */
      if ( info->header.e_shoff )
      {
         offset = info->header.e_shoff;
         hdr    = info->section_header;

         for (i = 0; i < info->header.e_shnum; i++)
         {
            if ( BXVD_P_Relf_ReadInput(info,
                                       (uint8_t *)hdr,
                                       offset,
                                       sizeof(Bxvd_Elf32_Shdr)) != 0 )
            {
               BDBG_ERR(("unable to read section header"));
               return BERR_TRACE(BXVD_ERR_RELF_BAD_INPUT);
            }

            offset += info->header.e_shentsize;

            BXVD_P_Relf_Get32(info->swap_required, hdr->sh_name);
            BXVD_P_Relf_Get32(info->swap_required, hdr->sh_type);
            BXVD_P_Relf_Get32(info->swap_required, hdr->sh_flags);
            BXVD_P_Relf_Get32(info->swap_required, hdr->sh_addr);
            BXVD_P_Relf_Get32(info->swap_required, hdr->sh_offset);
            BXVD_P_Relf_Get32(info->swap_required, hdr->sh_size);
            BXVD_P_Relf_Get32(info->swap_required, hdr->sh_link);
            BXVD_P_Relf_Get32(info->swap_required, hdr->sh_info);
            BXVD_P_Relf_Get32(info->swap_required, hdr->sh_addralign);
            BXVD_P_Relf_Get32(info->swap_required, hdr->sh_entsize);

            hdr++;
         }
      }

      /*
       * Any REL or RELA sections will point to the dynamic symbol table, which
       * must be a single section (so they'd better all point to the same place).
       * Find it and read in that symbol table section.
       */
      n = 0;
      for (i = 0; i < info->header.e_shnum; i++)
      {
         if ( info->section_header[i].sh_type == BXVD_SHT_REL ||
              info->section_header[i].sh_type == BXVD_SHT_RELA )
         {
            if ( n == 0 )
               n = info->section_header[i].sh_link;
            else if ( n != info->section_header[i].sh_link )
            {
               BDBG_ERR(("multiple dynamic symbol table sections referenced"));
               return BERR_TRACE(BXVD_ERR_MULT_SYM_TABLE_REFS);
            }
         }
      }
      if ( ! n )
      {
         for (i = 0; i < info->header.e_shnum; i++)
         {
            if ( info->section_header[i].sh_type == BXVD_SHT_SYMTAB )
            {
               n = i;
               break;
            }
         }
      }

      if ( n )
      {
         info->symtab   = (Bxvd_Elf32_Sym *) BXVD_P_Relf_ReadSection(hXvd, info, 1, n);
         info->symcount = info->section_header[n].sh_size / sizeof(Bxvd_Elf32_Sym);
         if ( ! info->symtab )
         {
            BDBG_ERR(("unable to read symbol table"));
            return BERR_TRACE(BXVD_ERR_CANT_READ_SYMTAB);
         }

         /* Byte-swap the symbol table entries. */
         for (i = 0; i < info->symcount; i++)
         {
            BXVD_P_Relf_Get32(info->swap_required, info->symtab[i].st_name);
            BXVD_P_Relf_Get32(info->swap_required, info->symtab[i].st_value);
            BXVD_P_Relf_Get32(info->swap_required, info->symtab[i].st_size);
            BXVD_P_Relf_Get16(info->swap_required, info->symtab[i].st_shndx);
         }
      }

      return BERR_TRACE(BERR_SUCCESS);
   }

/*
 * Read the section information from the ELF image
 */
static uint8_t *BXVD_P_Relf_ReadSection( BXVD_Handle hXvd,
                                         ElfInfo * info,
                                         int32_t MemBlk,
                                         int32_t index)
{
   BMMA_Block_Handle hBlock;

   uint8_t   *ptr;
   unsigned offset;
   unsigned size;

   BSTD_UNUSED(hXvd);

   if ( info->section_data[index] )
      return info->section_data[index];

   size   = info->section_header[index].sh_size;
   offset = info->section_header[index].sh_offset;


   if ( size == 0 || offset == 0 )
   {
      return NULL;
   }

   if ( MemBlk == 1 )
   {
      if ( info->iMemBlk1Used + size <= BXVD_RELF_BLOCK_1_SIZE )
      {
         ptr = info->pMemBlk1 + info->iMemBlk1Used;
         info->iMemBlk1Used += size;
         hBlock = info->hMemBlock1;
      }
      else
      {
         BDBG_WRN(("Temp Mem block 1 too small)"));
         return NULL;
      }
   }
   else
   {
      if ( info->iMemBlk2Used + size <= BXVD_RELF_BLOCK_2_SIZE )
      {
         ptr = info->pMemBlk2 + info->iMemBlk2Used;
         info->iMemBlk2Used += size;
         hBlock = info->hMemBlock2;
      }
      else
      {
         BDBG_WRN(("Temp Mem block 2 too small)"));
         return NULL;
      }
   }

   if ( BXVD_P_Relf_ReadInput(info, ptr, offset, size) != 0 )
   {
      BMMA_FlushCache(hBlock, ptr, size);
      return NULL;
   }

   info->section_data[index] = ptr;

   return ptr;
}

/*----------------------------------------------------------------------*
 *              Relocation                                              *
 *----------------------------------------------------------------------*/

/* Compute new base addresses for each section header. These get written into
 * the array new_section_base[] (rather than overwriting the sh_addr members)
 * because we need the original section base addresses later during
 * relocation.
 */
   static void BXVD_P_Relf_RelocateSections( ElfInfo * info )
   {
      int32_t i;
      uint32_t base;

      for (i = 1; i < info->header.e_shnum; i++)
      {
         info->new_section_base[i] = 0;

         base = info->section_header[i].sh_addr;
         if ( base == 0 && info->section_header[i].sh_type != BXVD_SHT_PROGBITS )
            continue;

         /* Skip anything in LSRAM -- that stays where it is. */
         if ( base >= LSRAM )
            continue;

         /* Skip anything below the stated code base -- we don't know
          * what it is.
          */
         if ( base < info->code_base )
            continue;

         if ( info->section_header[i].sh_flags & BXVD_SHF_EXECINSTR )
         {
            /* This is a code section -- relocate it to zero. */
            base -= info->code_base;
#if BXVD_RELOC_DEBUG_DETAIL_MSGS
            BDBG_MSG(("Code section relocation. base = %x", base));
#endif
         }
         else
         {
            /* This is a data section -- relocate it to the stated
             * load position (same offset as in the current image).
             */
            base = (base - info->code_base) + info->load_base;
#if BXVD_RELOC_DEBUG_DETAIL_MSGS
            BDBG_MSG(("Data section relocation. base = %x", base));
#endif
         }

         info->new_section_base[i] = base;
      }
   }

/* Go through the symbol table and relocate all symbols.  We just overwrite
 * the st_addr field, since we won't be needing the original address after
 * this.
 */
   static void BXVD_P_Relf_RelocateSymbols( ElfInfo * info )
   {
      int32_t    i;
      uint32_t   offset;     /* Offset of symbol within section */
      Bxvd_Elf32_Sym  *symbol;    /* Symbol being relocated */
      Bxvd_Elf32_Shdr *section;   /* Section containing symbol */

      symbol = info->symtab;
      for (i = 1; i < info->symcount; i++)
      {
         symbol++;

         if ( symbol->st_shndx == 0 || symbol->st_shndx >= BXVD_MAX_SHDR )
            continue;

         if ( symbol->st_value >= LSRAM )
            continue;

         section          = info->section_header + symbol->st_shndx;
         offset           = symbol->st_value - section->sh_addr;
         symbol->st_value = info->new_section_base[ symbol->st_shndx ] + offset;
      }
   }

/* Determine which REL or RELA section holds relocation enties for the
 * program section indexed by 'prog_index'.
 *
 * Returns the index of the REL(A) section, or 0 if there is none.
 */
   static int32_t BXVD_P_Relf_FindRelocSection(ElfInfo * info,
                                               uint32_t prog_index )
   {
      int32_t     i;

      for (i = 1; i < info->header.e_shnum; i++)
      {
         if ( info->section_header[i].sh_info != prog_index )
            continue;

         if ( info->section_header[i].sh_type == BXVD_SHT_RELA ||
              info->section_header[i].sh_type == BXVD_SHT_REL )
            return i;
      }

      return 0;
   }

/* Process a single RELA section.  This function processes
 * (applies) all records in the specified RELA section.
 *
 * Assumes: the sections referenced by prog_index and rel_index have
 * been read into memory (and are in section_data[]).
 *
 */
   static void BXVD_P_Relf_DoRelocationA(ElfInfo *info,
                                         int32_t prog_index,
                                         int32_t rel_index)
   {
      uint8_t    *data;
      int32_t    count;        /* Entries in RELA section         */
      int32_t    type;         /* RELA entry type                    */
      int32_t    sym;          /* RELA entry symbol index         */
      int32_t    i;
      uint32_t   offset;
      uint32_t   new_pc;
      Bxvd_Elf32_Sym  *symbol; /* Symbol referenced by entry      */
      Bxvd_Elf32_Shdr *prog;   /* Program section being relocated */
      Bxvd_Elf32_Shdr *rel;    /* RELA section being interpreted  */
      Bxvd_Elf32_Rela *entry;  /* RELA entry                      */

      prog  = info->section_header + prog_index;
      rel   = info->section_header + rel_index;
      count = rel->sh_size / sizeof(Bxvd_Elf32_Rela);

      entry = (Bxvd_Elf32_Rela *)(info->section_data[rel_index]);
      for (i = 0; i < count; i++)
      {
         BXVD_P_Relf_Get32(info->swap_required, entry->r_offset);
         BXVD_P_Relf_Get32(info->swap_required, entry->r_info);
         BXVD_P_Relf_Get32(info->swap_required, entry->r_addend);

         type = BXVD_ELF32_R_TYPE(entry->r_info);
         sym  = BXVD_ELF32_R_SYM (entry->r_info);

         /* Compute the offset into the 'prog' section at which
          * the fixup occurs.  We also need to know the new
          *  (relocated) PC of this location, as well as a pointer
          *  to the actual data to be changed.
          */
         offset = entry->r_offset - prog->sh_addr;
         data    = info->section_data    [ prog_index ] + offset;
         new_pc =   info->new_section_base[ prog_index ] + offset;

         /*  Determine which symbol is being referenced. */
         symbol = info->symtab + sym;

         BXVD_P_Relf_Fixup(data,
                           new_pc,
                           symbol->st_value,
                           type,
                           entry->r_addend);

         entry++;
      }
   }

/* Perform a single relocation fixup.  This is a modified copy of
 * the original function from Metaware, and handles the ARC-specific
 * types from RELA section entries.
 *
 * I have no idea whether any of this is correct or not; all questions
 * should be refered to Metaware.
 */
   static void BXVD_P_Relf_Fixup(uint8_t *data,
                                 unsigned pc,
                                 unsigned targetAddress,
                                 int32_t type,
                                 int32_t addend)
   {
      int32_t  t = targetAddress + addend;
      int32_t  disp;
      int32_t  I0 = 0, I1 = 1, I2 = 2, I3 = 3;
      int32_t  W0 = 0, W1 = 1, W2 = 2;
      int32_t  S0 = 0, S1 = 1;
      uint32_t w = 0;

      switch ( type )
      {
         case R_ARC_8:
            *data = (uint8_t)t;
            break;
         case R_ARC_16:
            data[S0] = (uint8_t)(t & 0xFF);
            data[S1] = (uint8_t)(t >> 8);
            break;
         case R_ARC_24:
            data[W0] = (uint8_t)(t & 0xFF);
            data[W1] = (uint8_t)(t >> 8);
            data[W2] = (uint8_t)(t >> 16);
            break;
         case R_ARC_32:
            data[I0] = (uint8_t)(t & 0xFF);
            data[I1] = (uint8_t)(t >> 8);
            data[I2] = (uint8_t)(t >> 16);
            data[I3] = (uint8_t)(t >> 24);
            break;
         case R_ARC_B26:
            if (t > 0x03FFFFFF || t < -0x04000000)
               BDBG_ERR(("target out of range at pc=%08X", pc));
            else if ( t & 3 )
               BDBG_ERR(("target needs to be word-aligned at pc=%08X", pc));
            w = data[3] << 24;
            w |= (t >> 2) & 0x00FFFFFF;
            data[I0] = (uint8_t)(w & 0xFF);
            data[I1] = (uint8_t)(w >> 8);
            data[I2] = (uint8_t)(w >> 16);
            data[I3] = (uint8_t)(w >> 24);
            break;
         case R_ARC_B22_PCREL:
            disp = t - pc - 4;
            if ( (disp & 3) != 0 )
               BDBG_ERR(("fixup at PC 0x%08X requires a word-aligned displacement",
                         pc));
            if (disp >> 21 != 0 && disp >> 21 != -1)
               BDBG_ERR(("PC-relative fixup at 0x%08X is out of range", pc));
            disp <<= 5;
            disp &= 0x07FFFF80;
            w = ((data[3] & 0xF8) << 24) + (data[0] & 0x7F);
            w |= disp;
            data[I0] = (uint8_t)(w & 0xFF);
            data[I1] = (uint8_t)(w >> 8);
            data[I2] = (uint8_t)(w >> 16);
            data[I3] = (uint8_t)(w >> 24);
            break;
         case R_ARC_H30:
            w = t >> 2;
            data[I0] = (uint8_t)(w & 0xFF);
            data[I1] = (uint8_t)(w >> 8);
            data[I2] = (uint8_t)(w >> 16);
            data[I3] = (uint8_t)(w >> 24);
            break;
         case R_ARC_N8:
            *data = (uint8_t)(addend - targetAddress);
            break;
         case R_ARC_N16:
            w = addend - targetAddress;
            data[S0] = (uint8_t)(w & 0xFF);
            data[S1] = (uint8_t)(w >> 8);
            break;
         case R_ARC_N24:
            w = addend - targetAddress;
            data[W0] = (uint8_t)(w & 0xFF);
            data[W1] = (uint8_t)(w >> 8);
            data[W2] = (uint8_t)(w >> 16);
            break;
         case R_ARC_N32:
            w = addend - targetAddress;
            data[I0] = (uint8_t)(w & 0xFF);
            data[I1] = (uint8_t)(w >> 8);
            data[I2] = (uint8_t)(w >> 16);
            data[I3] = (uint8_t)(w >> 24);
            break;
         case R_ARC_SDA:
            /* 9-bit SDA. Should require no change */
            break;
         case R_ARC_SECTOFF:
            /* 32-bit SDA. Should require no change */
            break;
         case R_ARC_NONE:
            /* It happens in __xcheck. Don't understand why.*/
            break;
         default:
            BDBG_ERR(("unexpected fixup type %d (0x%08X) at PC 0x%08X", type,type,pc));
            break;
      }
   }
#endif /* If BXVD_P_USE_RELF */
