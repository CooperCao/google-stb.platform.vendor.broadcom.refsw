/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
 * Module Description: ELF file parser and loader
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "belf.h"
#include "belf_priv.h"

#if BELF_MIPS_SUPPORT
#include "belf_mips.h"
#endif

BDBG_MODULE(belf_priv);

BDBG_OBJECT_ID(BELF_Image);
BDBG_OBJECT_ID(BELF_SectionNode);
BDBG_OBJECT_ID(BELF_CommonSymbolNode);

static BERR_Code BELF_P_ARCH_GetSectionPriority(const BELF_ImageHandle image, const BELF_SectionNode *pNode, unsigned *pPriority);

int BELF_P_StrCmp(const char *pString1, const char *pString2)
{
    int ch1, ch2, diff;

    for(;;) {
       ch1 = *pString1++;
       ch2 = *pString2++;
       diff = ch1 - ch2;
       if (diff) {
          return diff;
       } else if (ch1=='\0') {
          return 0;
       }
    }
}

unsigned BELF_P_GetSymbolOffset(BELF_ImageHandle image, const Elf32_Sym *pSymbol)
{
    /* TODO, there is an arch-specific component to this.  MIPS has SCOMMON and ACOMMON that need to be handled similarly. */

    /* Ordinary symbols have the section offset coded.  Common symbols do not. */
    if ( pSymbol->st_shndx == SHN_COMMON )
    {
        BELF_CommonSymbolNode *pNode;

        for ( pNode = BLST_SQ_FIRST(&image->commonSymbolList); pNode != NULL; pNode = BLST_SQ_NEXT(pNode, node) )
        {
            if ( pNode->pSymbol == pSymbol )
            {
                return pNode->bssOffset;
            }
        }

        /* We should never get here, this is bad. */
        BDBG_ASSERT(false);
        return 0;
    }
    else
    {
        return pSymbol->st_value;
    }
}

const void * BELF_P_GetSymbolAddress(BELF_ImageHandle image, const Elf32_Sym *pSymbol)
{
    /* TODO, there is an arch-specific component to this.  MIPS has SCOMMON and ACOMMON that need to be handled similarly. */

    /* Ordinary symbols have the section offset coded.  Common symbols do not. */
    if ( pSymbol->st_shndx == SHN_COMMON )
    {
        BELF_CommonSymbolNode *pNode;

        for ( pNode = BLST_SQ_FIRST(&image->commonSymbolList); pNode != NULL; pNode = BLST_SQ_NEXT(pNode, node) )
        {
            if ( pNode->pSymbol == pSymbol )
            {
                return (const char *)image->pBss->details.pAddress + pNode->bssOffset;
            }
        }

        /* We should never get here, this is bad. */
        BDBG_ASSERT(false);
        return 0;
    }
    else
    {
        BELF_SectionDetails sectionDetails;
        if ( BELF_LookupSectionByIndex(image, pSymbol->st_shndx, &sectionDetails) != BERR_SUCCESS )
        {
            const char *pString ;
            if ( image->pStringTable )
            {
                pString = ((const char *)image->imageSettings.pImageData) + image->pStringTable->pSectionHeader->sh_offset;
            } else {
                pString = NULL;
            }
            BDBG_WRN(("Can't get section:%u for symbol:'%s'(%u:%#X)", pSymbol->st_shndx, pString?pString+pSymbol->st_name:"", pSymbol->st_name, pSymbol->st_value));
            return (const void *)pSymbol->st_value;
        }
        else
        {
            return (const char *)sectionDetails.pAddress + pSymbol->st_value;
        }
    }
}

BERR_Code BELF_P_ValidateHeader(BELF_ImageHandle image)
{
    const Elf32_Ehdr *pElfHeader = image->pElfHeader;

    if ( pElfHeader->e_ident[EI_MAG0] != ELFMAG0 ||
         pElfHeader->e_ident[EI_MAG1] != ELFMAG1 || 
         pElfHeader->e_ident[EI_MAG2] != ELFMAG2 ||
         pElfHeader->e_ident[EI_MAG3] != ELFMAG3 )
    {
        return BERR_TRACE(BELF_ERR_BAD_MAGIC_NUMBER);
    }

    if ( pElfHeader->e_ident[EI_VERSION] != EV_CURRENT )
    {
        return BERR_TRACE(BELF_ERR_INVALID_VERSION);
    }

    if ( pElfHeader->e_type == ET_REL )
    {
        image->imageDetails.hasRelocations = true;
        image->imageDetails.type = BELF_ImageType_eRelocatable;
    }
    else if ( pElfHeader->e_type == ET_EXEC )
    {
        image->imageDetails.hasRelocations = false;
        image->imageDetails.type = BELF_ImageType_eExecutable;
    }
    else
    {
        return BERR_TRACE(BELF_ERR_UNSUPPORTED_IMAGE_TYPE);
    }

    image->imageDetails.is64Bit = (pElfHeader->e_ident[EI_CLASS] == ELFCLASS64)?true:false;
    image->imageDetails.littleEndian = (pElfHeader->e_ident[EI_DATA] == ELFDATA2LSB)?true:false;
    image->imageDetails.numSections = pElfHeader->e_shnum;

    /* So far so good, now validate the arch specifics */
    switch ( image->pElfHeader->e_machine )
    {
#if BELF_MIPS_SUPPORT
    case EM_MIPS:
        image->imageDetails.machine = BELF_MachineType_eMips;
        return BELF_P_MIPS_ValidateHeader(image);
#endif
    default:
        return BERR_TRACE(BELF_ERR_INVALID_MACHINE);
    }
}

static void BELF_P_AddSection(BELF_ImageHandle image, BELF_SectionNode *pNode)
{
    BELF_SectionNode *pPrev, *pCurrent;
    unsigned priority = pNode->priority;

    /* We insert nodes based on priority.  Nodes of the same priority will appear in the same order as the input file. */

    pCurrent = BLST_SQ_FIRST(&image->sectionList);
    /* Empty list or the first node is lower priority than this one */
    if ( NULL == pCurrent || pCurrent->priority > priority )
    {
        BLST_SQ_INSERT_HEAD(&image->sectionList, pNode, node);
        return;
    }

    for ( ;; )
    {
        /* Step forward */
        pPrev = pCurrent;
        pCurrent = BLST_SQ_NEXT(pPrev, node);

        /* If we've hit the end of the list or a node of lower priority, insert after the previous one. */
        if ( NULL == pCurrent || pCurrent->priority > priority )
        {
            BLST_SQ_INSERT_AFTER(&image->sectionList, pPrev, pNode, node);
            return;
        }
    }
}

BERR_Code BELF_P_InitSections(BELF_ImageHandle image)
{
    unsigned i;
    const unsigned numSections = image->imageDetails.numSections;
    const Elf32_Shdr *pSectionHeader = (const Elf32_Shdr *)((unsigned)image->imageSettings.pImageData + image->pElfHeader->e_shoff);
    const char *pString;
    BELF_SectionNode *pNode;

    /* Make sure there is a section header */
    if ( 0 == image->pElfHeader->e_shoff )
    {
        if ( image->pElfHeader->e_type == ET_EXEC )
        {
            /* This is okay, we don't need it for programs. */
            return BERR_SUCCESS;
        }
        else
        {
            return BERR_TRACE(BELF_ERR_INVALID_SECTION_HEADER);
        }
    }
    
    /* Cache section header base */
    image->pSectionHeader = pSectionHeader;

    /* Use empty string table if none exists.  All name offsets will be 0 in that case, so it's safe. */
    if ( image->pElfHeader->e_shstrndx == 0 )
    {
        pString = "";
    }
    else
    {
        /* Locate section header string table data */
        pString = ((const char *)image->imageSettings.pImageData + pSectionHeader[image->pElfHeader->e_shstrndx].sh_offset);
    }

    /* Skip the first section, it's always invalid */
    pSectionHeader++;
    for ( i = 1; i < numSections; i++, pSectionHeader++ )
    {
        unsigned type;
        pNode = BKNI_Malloc(sizeof(BELF_SectionNode));

        if ( NULL == pNode )
        {
            BELF_P_UninitSections(image);
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }

        /* Converrt type */
        type = pSectionHeader->sh_type;
        if ( type >= SHT_LOPROC && type <= SHT_HIPROC )
        {
            pNode->details.type = BELF_SectionType_eProcessorSpecific;
        }
        else if ( type >= SHT_LOUSER && type <= SHT_HIUSER )
        {
            pNode->details.type = BELF_SectionType_eUserDefined;
        }
        else if ( type >= SHT_LOOS && type <= SHT_HIOS )
        {
            pNode->details.type = BELF_SectionType_eOsSpecific;
        }
        else if ( type <= SHT_DYNSYM )
        {
            pNode->details.type = type;
            if ( type == SHT_SYMTAB )
            {
                image->pSymbolTable = pNode;
                image->imageDetails.numSymbols = pSectionHeader->sh_size / pSectionHeader->sh_entsize;
            }
            else if ( type == SHT_NOBITS )
            {
                if ( !BELF_P_StrCmp(pString + pSectionHeader->sh_name, ".bss") )
                {
                    /* Common symbols need to be moved here.  Save a link. */
                    image->pBss = pNode;
                }
            }
        }
        else
        {
            BKNI_Free(pNode);
            BDBG_ERR(("Invalid section type %u (%#x) found at section index %u", type, type, i));
            return BERR_TRACE(BELF_ERR_INVALID_SECTION);
        }

        pNode->details.imageOffset = pSectionHeader->sh_offset;
        pNode->details.pAddress = (void *)pSectionHeader->sh_addr;  /* Relocatable sections already set this to 0 */
        pNode->details.size = pSectionHeader->sh_size;
        pNode->details.alignment = pSectionHeader->sh_addralign;
        pNode->details.index = i;
        pNode->details.pName = pString + pSectionHeader->sh_name;
        BDBG_OBJECT_SET(pNode, BELF_SectionNode);
        pNode->pSectionHeader = pSectionHeader;
        (void)BELF_P_ARCH_GetSectionPriority(image, pNode, &pNode->priority);
        /* We insert nodes based on priority.  Nodes of the same priority will appear in the same order as the input file. */
        BELF_P_AddSection(image, pNode);
        BDBG_MSG(("Found section %u (%s)", i, pNode->details.pName));
    }

    /* Find link to string table */
    if ( image->pSymbolTable )
    {
        unsigned link;
        /* Grab link to string table, the symbol table references the correct one as a link. */
        pSectionHeader = image->pSectionHeader;
        link = pSectionHeader[image->pSymbolTable->details.index].sh_link;
        if ( link > 0 )
        {
            for ( pNode = BLST_SQ_FIRST(&image->sectionList); pNode != NULL; pNode = BLST_SQ_NEXT(pNode, node) )
            {
                if ( pNode->details.index == link )
                {
                    image->pStringTable = pNode;
                    break;
                }
            }
        }
    }

    return BERR_SUCCESS;
}

void BELF_P_UninitSections(BELF_ImageHandle image)
{
    BELF_SectionNode *pNode;

    for ( pNode = BLST_SQ_FIRST(&image->sectionList); pNode != NULL; pNode = BLST_SQ_FIRST(&image->sectionList) )
    {
        BDBG_OBJECT_ASSERT(pNode, BELF_SectionNode);
        BLST_SQ_REMOVE_HEAD(&image->sectionList, node);
        BDBG_OBJECT_DESTROY(pNode, BELF_SectionNode);
        BKNI_Free(pNode);
    }
}

BERR_Code BELF_P_InitSymbols(BELF_ImageHandle image)
{
    /* This only applies to relocatable files, programs should never have common symbols - the linker resolved them */
    if ( image->imageDetails.hasRelocations )
    {
        const Elf32_Sym *pSymbol;
        const char *pString;
        unsigned i, num;
        
        /* Relocatable files must have a symbol table */               
        if ( NULL == image->pSymbolTable )
        {
            BDBG_ERR(("No symbol table found.  Symbol table required for relocation"));
            return BERR_TRACE(BELF_ERR_INVALID_SYMBOL_TABLE);
        }

        /* If there is no string table, use an empty string */
        if ( NULL == image->pStringTable )
        {
            pString = "";
        }
        else
        {
            pString = ((const char *)image->imageSettings.pImageData + image->pStringTable->details.imageOffset);
        }

        pSymbol = (const Elf32_Sym *)((unsigned)image->imageSettings.pImageData + image->pSymbolTable->details.imageOffset);
        num = image->imageDetails.numSymbols;
        for ( i = 0; i < num; i++ )
        {
            if ( SHN_COMMON == pSymbol[i].st_shndx )
            {
                /* Need to allocate space in .bss */
                BELF_CommonSymbolNode *pSymbolNode;
                BELF_SectionNode *pBss = image->pBss;
                unsigned offset, align, mod;
                if ( NULL == pBss )
                {
                    BDBG_ERR(("Common symbols require a .bss section, but one was not found"));
                    BELF_P_UninitSymbols(image);
                    return BERR_TRACE(BELF_ERR_INVALID_SYMBOL);
                }                
                pSymbolNode = BKNI_Malloc(sizeof(BELF_CommonSymbolNode));
                if ( NULL == pSymbolNode )
                {
                    BELF_P_UninitSymbols(image);
                    return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                }
                BDBG_OBJECT_SET(pSymbolNode, BELF_CommonSymbolNode);
                pSymbolNode->index = i;
                pSymbolNode->pSymbol = pSymbol+i;
                offset = pBss->details.size;
                align = pSymbol[i].st_value;
                mod = offset % align;
                if ( 0 != mod )
                {
                    offset += align - mod;
                }
                pSymbolNode->bssOffset = offset;
                pBss->details.size = offset + pSymbol[i].st_size;
                BLST_SQ_INSERT_TAIL(&image->commonSymbolList, pSymbolNode, node);
                BDBG_MSG(("Allocated space for common symbol %u '%s' at offset %#x of .bss", i, pString+pSymbol[i].st_name, pSymbolNode->bssOffset));
            }

            /* TODO, there is an arch-specific component to this.  MIPS has SCOMMON and ACOMMON that need to be handled similarly. */

        }
    }
    return BERR_SUCCESS;
}

BERR_Code BELF_P_ComputeImageSize(BELF_ImageHandle image)
{
    /* Compute the image size after the sections have been sorted and the symbols resolved */
    if ( image->imageDetails.type == BELF_ImageType_eExecutable )
    {
        /* If we're an executable image, walk the program header */
        const Elf32_Phdr *pProgramHeader = (const Elf32_Phdr *)((unsigned)image->imageSettings.pImageData + image->pElfHeader->e_phoff);
        unsigned int i, num;
        unsigned memoryRequired=0;
        
        num = image->pElfHeader->e_phnum;
        for ( i = 0; i < num; i++ )
        {
            /* We only require memory for loadable segments */
            if ( pProgramHeader[i].p_type == PT_LOAD )
            {
                memoryRequired += pProgramHeader[i].p_memsz;
            }
        }    
        
        image->imageDetails.memoryRequired = memoryRequired;
        image->imageDetails.memoryAlignment = 0;    
    }
    else if ( image->imageDetails.type == BELF_ImageType_eRelocatable )
    {
        /* Walk the section list */
        const Elf32_Shdr *pSectionHeader = (const Elf32_Shdr *)((unsigned)image->imageSettings.pImageData + image->pElfHeader->e_shoff);
        const BELF_SectionNode *pNode;
        unsigned memoryRequired, memoryAlignment;
        
        memoryAlignment = 256;       /* Default to 256-byte alignment by default (typical RAC linesize) */
        /* Get the largest overall alignment in case it's larger */
        for ( pNode = BLST_SQ_FIRST(&image->sectionList); pNode != NULL; pNode = BLST_SQ_NEXT(pNode, node) )
        {
            const Elf32_Shdr *pSection = pSectionHeader + pNode->details.index;
            if ( pSection->sh_flags & SHF_ALLOC )
            {
                unsigned align = pSection->sh_addralign;
                if ( align > memoryAlignment )
                {
                    memoryAlignment = align;
                }
            }
        }

        /* Sum up the image size */
        memoryRequired = memoryAlignment;
        for ( pNode = BLST_SQ_FIRST(&image->sectionList); NULL != pNode; pNode = BLST_SQ_NEXT(pNode, node) )
        {
            const Elf32_Shdr *pSection = pSectionHeader + pNode->details.index;
            if ( pSection->sh_flags & SHF_ALLOC )
            {
                unsigned mod = memoryRequired % pSection->sh_addralign;
                if ( mod )
                {
                    memoryRequired += (pSection->sh_addralign - mod);
                }
                memoryRequired += pSection->sh_size;
            }
        }

        image->imageDetails.memoryRequired = memoryRequired;
        image->imageDetails.memoryAlignment = memoryAlignment;
    }
    else
    {
        /* Invalid */
        return BERR_TRACE(BELF_ERR_UNSUPPORTED_IMAGE_TYPE);
    }

    return BERR_SUCCESS;
}

void BELF_P_UninitSymbols(BELF_ImageHandle image)
{
    BELF_CommonSymbolNode *pNode;

    for ( pNode = BLST_SQ_FIRST(&image->commonSymbolList); pNode != NULL; pNode = BLST_SQ_FIRST(&image->commonSymbolList) )
    {
        BDBG_OBJECT_ASSERT(pNode, BELF_CommonSymbolNode);
        BLST_SQ_REMOVE_HEAD(&image->commonSymbolList, node);
        BDBG_OBJECT_DESTROY(pNode, BELF_CommonSymbolNode);
        BKNI_Free(pNode);
    }
}

BERR_Code BELF_P_LoadImage(BELF_ImageHandle image)
{
    const Elf32_Phdr *pProgramHeader = (const Elf32_Phdr *)((unsigned)image->imageSettings.pImageData + image->pElfHeader->e_phoff);
    unsigned int i, num;

    /* Loading an executable walks the program header and loads on a segment basis. */

    if ( image->pElfHeader->e_type != ET_EXEC )
    {
        /* This is only supported for executables. */
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( image->pElfHeader->e_phoff == 0 )
    {
        /* Bad program header */
        return BERR_TRACE(BELF_ERR_INVALID_PROGRAM_HEADER);
    }

    num = image->pElfHeader->e_phnum;
    for ( i = 0; i < num; i++ )
    {
        /* We only deal with loadable segments */
        if ( pProgramHeader[i].p_type == PT_LOAD )
        {
            if ( pProgramHeader[i].p_filesz > 0 )
            {
                BDBG_MSG(("Loading program segment %d at virtual address %#x file offset %d bytes length %d bytes",
                          i, pProgramHeader[i].p_vaddr, pProgramHeader[i].p_offset, pProgramHeader[i].p_filesz));
                /* Actual data from the file */
                BKNI_Memcpy((void *)pProgramHeader[i].p_vaddr, ((const char *)image->imageSettings.pImageData) + pProgramHeader[i].p_offset, pProgramHeader[i].p_filesz);
            }
            if ( pProgramHeader[i].p_memsz > pProgramHeader[i].p_filesz )
            {
                BDBG_MSG(("Zeroing extra bytes in program segment %d at virtual address %#x length %d bytes",
                          i, pProgramHeader[i].p_vaddr + pProgramHeader[i].p_filesz, pProgramHeader[i].p_memsz - pProgramHeader[i].p_filesz));
                /* Unitialized "extra" data is set to 0 (e.g. .bss) */
                BKNI_Memset(((char *)pProgramHeader[i].p_vaddr) + pProgramHeader[i].p_filesz, 0, pProgramHeader[i].p_memsz - pProgramHeader[i].p_filesz);
            }
        }
    }

    return BERR_SUCCESS;
}

BERR_Code BELF_P_CopySections(BELF_ImageHandle image)
{
    const Elf32_Shdr *pSectionHeader = image->pSectionHeader;
    char *pTargetAddr = image->pLoadMemory;
    BELF_SectionNode *pNode;

    /* Loading a relocatable image copies the sections into the target address prior to relocation by walking the section headers.  */
    /* These were sorted initially so they should be loaded in the sorted order from the linked list, not the section header order. */
    for ( pNode = BLST_SQ_FIRST(&image->sectionList); pNode != NULL; pNode = BLST_SQ_NEXT(pNode, node) )
    {
        const Elf32_Shdr *pSection = pSectionHeader + pNode->details.index;
        
        if ( pSection->sh_flags & SHF_ALLOC )
        {
            unsigned mod;
            mod = (unsigned)pTargetAddr % pSection->sh_addralign; 
            /* Fixup alignment */
            if ( mod != 0 )
            {
                BDBG_MSG(("Aligning section %d to %d bytes", pSection->sh_addralign));
                pTargetAddr += pSection->sh_addralign - mod;
            }
            pNode->details.pAddress = pTargetAddr;
            if ( pSection->sh_type == SHT_NOBITS )
            {
                /* .bss type section */
                BDBG_MSG(("Zeroing section %d (file offset %#x RAM address %#x)", pNode->details.index, pSection->sh_offset, pTargetAddr));
                BKNI_Memset(pTargetAddr, 0, pSection->sh_size);
            }
            else
            {
                /* Data to be loaded */
                BDBG_MSG(("Loading section %d (file offset %#x RAM address %#x)", pNode->details.index, pSection->sh_offset, pTargetAddr));
                BKNI_Memcpy(pTargetAddr, ((const char *)image->imageSettings.pImageData) + pSection->sh_offset, pSection->sh_size);
            }
            pTargetAddr += pSection->sh_size;
        }
    }

    return BERR_SUCCESS;
}

BELF_SectionNode *BELF_P_GetSectionByIndex(BELF_ImageHandle image, unsigned index)
{
    BELF_SectionNode *pNode;

    for ( pNode = BLST_SQ_FIRST(&image->sectionList); pNode != NULL; pNode = BLST_SQ_NEXT(pNode, node) )
    {
        if ( index == pNode->details.index )
        {
            return pNode;
        }
    }
    return NULL;
}

BERR_Code BELF_P_ARCH_Init(BELF_ImageHandle image)
{
    switch ( image->pElfHeader->e_machine )
    {
#if BELF_MIPS_SUPPORT
    case EM_MIPS:
        return BELF_P_MIPS_Init(image);
#endif
    default:
        return BERR_TRACE(BELF_ERR_INVALID_MACHINE);
    }
}

BERR_Code BELF_P_ARCH_Relocate(BELF_ImageHandle image)
{
    switch ( image->pElfHeader->e_machine )
    {
#if BELF_MIPS_SUPPORT
    case EM_MIPS:
        return BELF_P_MIPS_Relocate(image);
#endif
    default:
        return BERR_TRACE(BELF_ERR_INVALID_MACHINE);
    }
}

void BELF_P_ARCH_Uninit(BELF_ImageHandle image)
{
    switch ( image->pElfHeader->e_machine )
    {
#if BELF_MIPS_SUPPORT
    case EM_MIPS:
        BELF_P_MIPS_Uninit(image);
        break;
#endif
    default:
        (void)BERR_TRACE(BELF_ERR_INVALID_MACHINE);
        break;
    }
}

static BERR_Code BELF_P_ARCH_GetSectionPriority(const BELF_ImageHandle image, const BELF_SectionNode *pNode, unsigned *pPriority)
{
    /* Because some section types are architecture-specific, we allow the architecture to set the sort order of sections */
    switch ( image->pElfHeader->e_machine )
    {
#if BELF_MIPS_SUPPORT
    case EM_MIPS:
        return BELF_P_MIPS_GetSectionPriority(image, pNode, pPriority);
#endif
    default:
        return BERR_TRACE(BELF_ERR_INVALID_MACHINE);
    }
}

