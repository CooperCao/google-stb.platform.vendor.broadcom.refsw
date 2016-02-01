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

BDBG_MODULE(belf);

void BELF_GetDefaultImageSettings(
    BELF_ImageSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

BERR_Code BELF_OpenImage(
    const BELF_ImageSettings *pSettings,
    BELF_ImageHandle *pHandle   /* [out] Image Handle */
    )
{
    BELF_ImageHandle image;
    BERR_Code errCode;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);
    BDBG_ASSERT(NULL != pSettings->pImageData);
    BDBG_ASSERT(pSettings->imageLength > 0);

    /* Alloc handle */
    image = BKNI_Malloc(sizeof(BELF_Image));
    if ( NULL == image )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Init structure */
    BKNI_Memset(image, 0, sizeof(BELF_Image));
    BDBG_OBJECT_SET(image, BELF_Image);
    image->imageSettings = *pSettings;
    image->pElfHeader = (const Elf32_Ehdr *)pSettings->pImageData;
    BLST_SQ_INIT(&image->sectionList);
    BLST_SQ_INIT(&image->commonSymbolList);

    /* Validate Header */
    errCode = BELF_P_ValidateHeader(image);
    if ( errCode )
    {
        BDBG_OBJECT_DESTROY(image, BELF_Image);
        BKNI_Free(image);
        return BERR_TRACE(errCode);
    }

    /* Extract section data */
    errCode = BELF_P_InitSections(image);
    if ( errCode )
    {
        BDBG_OBJECT_DESTROY(image, BELF_Image);
        BKNI_Free(image);
        return BERR_TRACE(errCode);
    }

    /* Resolve common symbols */
    errCode = BELF_P_InitSymbols(image);
    if ( errCode )
    {
        BELF_P_UninitSections(image);
        BDBG_OBJECT_DESTROY(image, BELF_Image);
        BKNI_Free(image);
        return BERR_TRACE(errCode);
    }

    /* Compute image size after loading section and symbol info */
    errCode = BELF_P_ComputeImageSize(image);
    if ( errCode )
    {
        BELF_P_UninitSymbols(image);
        BELF_P_UninitSections(image);
        BDBG_OBJECT_DESTROY(image, BELF_Image);
        BKNI_Free(image);
        return BERR_TRACE(errCode);
    }

    /* Init architecture specifics */
    errCode = BELF_P_ARCH_Init(image);
    if ( errCode )
    {
        BELF_P_UninitSymbols(image);
        BELF_P_UninitSections(image);
        BDBG_OBJECT_DESTROY(image, BELF_Image);
        BKNI_Free(image);
        return BERR_TRACE(errCode);
    }

    /* Success */
    *pHandle = image;
    return BERR_SUCCESS;
}

void BELF_CloseImage(
    BELF_ImageHandle image
    )
{
    BDBG_OBJECT_ASSERT(image, BELF_Image);

    BELF_P_ARCH_Uninit(image);
    BELF_P_UninitSymbols(image);
    BELF_P_UninitSections(image);
    BDBG_OBJECT_DESTROY(image, BELF_Image);
    BKNI_Free(image);
}

BERR_Code BELF_GetImageDetails(
    BELF_ImageHandle image,
    BELF_ImageDetails *pDetails /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(image, BELF_Image);
    BDBG_ASSERT(NULL != pDetails);
    *pDetails = image->imageDetails;
    return BERR_SUCCESS;
}

BERR_Code BELF_LoadImage(
    BELF_ImageHandle image,
    void *pMemory,              /* Required for relocation.  Pass NULL if ignored. */
    uint32_t memoryBase,        /* For relocation, this is the actual address the target will execute from.  If you are either loading
                                   into MMU-translated memory or running from MMU-translated memory, this may not be the same as the
                                   pMemory address.  Pass 0 if relocation is not being performed. */
    void **pEntry               /* [out] Program entry address (relative to pMemory) */
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(image, BELF_Image);
    if ( image->imageDetails.hasRelocations )
    {
        BELF_SectionDetails textSection;
        image->pLoadMemory = pMemory;
        image->memoryBase = memoryBase;
        errCode = BELF_P_CopySections(image);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        errCode = BELF_P_ARCH_Relocate(image);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        errCode = BELF_LookupSectionByName(image, ".text", &textSection);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        *pEntry = (void *)((unsigned)textSection.pAddress + image->pElfHeader->e_entry);
    }
    else
    {
        errCode = BELF_P_LoadImage(image);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        *pEntry = (void *)(image->pElfHeader->e_entry);
    }

    return BERR_SUCCESS;
}

BERR_Code BELF_LookupSymbolByName(
    BELF_ImageHandle image,
    const char *pName,
    BELF_SymbolDetails *pDetails    /* [out] */
    )
{
    const char *pString;
    const Elf32_Sym *pSymbol;
    unsigned i, numSymbols;
    BDBG_OBJECT_ASSERT(image, BELF_Image);
    BDBG_ASSERT(NULL != pName);
    BDBG_ASSERT(NULL != pDetails);

    /* Both the symbol and string tables are required for this to work */
    if ( NULL == image->pSymbolTable )
    {
        return BERR_TRACE(BELF_ERR_INVALID_SYMBOL_TABLE);
    }
    if ( NULL == image->pStringTable )
    {
        return BERR_TRACE(BELF_ERR_INVALID_STRING_TABLE);
    }

    /* Locate the string table data */
    pString = ((const char *)image->imageSettings.pImageData) + image->pStringTable->details.imageOffset;
    /* Locate the symbol table data */
    pSymbol = (const Elf32_Sym *)(((unsigned)image->imageSettings.pImageData) + image->pSymbolTable->details.imageOffset)+1;
    numSymbols = image->pSymbolTable->details.size / sizeof(Elf32_Sym);
    /* Walk through the symbol table looking for a matched symbol name */
    for ( i = 1; i < numSymbols; i++, pSymbol++ )
    {
        if ( !BELF_P_StrCmp(pName, pString+pSymbol->st_name) )
        {
            /* Found a match.  Return details. */
            return BELF_LookupSymbolByIndex(image, i, pDetails);
        }
    }

    return BERR_TRACE(BELF_ERR_SYMBOL_NOT_FOUND);
}

BERR_Code BELF_LookupSymbolByIndex(
    BELF_ImageHandle image,
    unsigned index,
    BELF_SymbolDetails *pDetails    /* [out] */
    )
{
    const char *pString;
    const Elf32_Sym *pSymbol;
    const BELF_SectionNode *pNode;
    unsigned sectionNum;

    BDBG_OBJECT_ASSERT(image, BELF_Image);
    BDBG_ASSERT(NULL != pDetails);

    /* The symbol is required for this to work */
    if ( NULL == image->pSymbolTable )
    {
        return BERR_TRACE(BELF_ERR_INVALID_SYMBOL_TABLE);
    }
    if ( index >= image->imageDetails.numSymbols )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Locate the string table data */
    if ( image->pStringTable )
    {
        pString = ((const char *)image->imageSettings.pImageData) + image->pStringTable->details.imageOffset;
    }
    else
    {
        pString = "";
    }

    /* Locate the symbol table data */
    pSymbol = (const Elf32_Sym *)(((unsigned)image->imageSettings.pImageData) + image->pSymbolTable->details.imageOffset);
    pSymbol += index;

    /* Find matching section node */
    sectionNum = pSymbol->st_shndx;
    if ( sectionNum >= SHN_LORESERV )
    {
        pNode = NULL;
    }
    else
    {
        for ( pNode = BLST_SQ_FIRST(&image->sectionList); pNode != NULL; pNode = BLST_SQ_NEXT(pNode, node) )
        {
            if ( pNode->details.index == sectionNum )
            {
                break;
            }
        }
        /* This should never happen, but just for completeness... */
        if ( NULL == pNode )
        {
            return BERR_TRACE(BELF_ERR_INVALID_SYMBOL);
        }
    }

    /* Got the section.  Find the symbol address. */
    if ( !image->imageDetails.hasRelocations || SHN_ABS == sectionNum )
    {
        /* the address will always be stored in the symbol */
        pDetails->pAddress = (void *)pSymbol->st_value;
    }
    else if ( image->pLoadMemory )
    {
        /* The image is loaded, apply the correct offset */
        if ( SHN_COMMON == sectionNum )
        {
            /* Find the address in the common symbol list */
            BELF_CommonSymbolNode *pCommonNode;
            for ( pCommonNode = BLST_SQ_FIRST(&image->commonSymbolList); NULL != pCommonNode; pCommonNode = BLST_SQ_NEXT(pCommonNode, node) )
            {
                if ( pCommonNode->index == index )
                {
                    break;
                }
            }
            /* Should never happen */
            if ( NULL == pCommonNode )
            {
                return BERR_TRACE(BELF_ERR_INVALID_SYMBOL);
            }
            pDetails->pAddress = (uint8_t *)image->pBss->details.pAddress + BELF_P_GetSymbolOffset(image, pSymbol);
            sectionNum = image->pBss->details.index;    /* Common symbols are actually reloated into .bss */
        }
        else if ( sectionNum >= SHN_LORESERV )
        {
            /* TODO: Support arch-specific sections like .sbss */
            pDetails->pAddress = NULL;
        }
        else
        {
            pDetails->pAddress = (uint8_t *)pNode->details.pAddress+ pSymbol->st_value;
        }
    }
    else
    {
        /* Not loaded.  No address. */
        pDetails->pAddress = NULL;
    }
    pDetails->size = pSymbol->st_size;
    pDetails->section = sectionNum;
    switch ( ELF32_ST_BIND(pSymbol->st_info) )
    {
    case STB_LOCAL:
        pDetails->binding = BELF_SymbolBinding_eLocal;
        break;
    case STB_GLOBAL:
        pDetails->binding = BELF_SymbolBinding_eGlobal;
        break;
    case STB_WEAK:
        pDetails->binding = BELF_SymbolBinding_eWeak;
        break;
    default:
        pDetails->binding = BELF_SymbolBinding_eOther;
    }
    switch ( ELF32_ST_TYPE(pSymbol->st_info) )
    {
    case STT_NOTYPE:
        pDetails->type = BELF_SymbolType_eNone;
        break;
    case STT_OBJECT:
        pDetails->type = BELF_SymbolType_eObject;
        break;
    case STT_FUNC:
        pDetails->type = BELF_SymbolType_eFunction;
        break;
    case STT_SECTION:
        pDetails->type = BELF_SymbolType_eSection;
        break;
    case STT_FILE:
        pDetails->type = BELF_SymbolType_eFile;
        break;
    default:
        pDetails->type = BELF_SymbolType_eOther;
        break;
    }
    pDetails->pName = pString + pSymbol->st_name;

    return BERR_SUCCESS;
}

BERR_Code BELF_LookupSymbolByAddress(
    BELF_ImageHandle image,
    void *pAddress,                 /* Address of the symbol */
    BELF_SymbolDetails *pDetails    /* [out] */
    )
{
    const Elf32_Sym *pSymbol;
    unsigned i, numSymbols;
    BDBG_OBJECT_ASSERT(image, BELF_Image);
    BDBG_ASSERT(NULL != pDetails);

    /* Both the symbol and string tables are required for this to work */
    if ( NULL == image->pSymbolTable )
    {
        return BERR_TRACE(BELF_ERR_INVALID_SYMBOL_TABLE);
    }

    /* Locate the symbol table data */
    pSymbol = (const Elf32_Sym *)(((unsigned)image->imageSettings.pImageData) + image->pSymbolTable->details.imageOffset)+1;
    numSymbols = image->pSymbolTable->details.size / sizeof(Elf32_Sym);
    /* Walk through the symbol table looking for a matched symbol name */
    for ( i = 1; i < numSymbols; i++, pSymbol++ )
    {
        const char *pSymAddress = BELF_P_GetSymbolAddress(image, pSymbol);
        if ( (const char *)pAddress >= pSymAddress && (const char *)pAddress <= (pSymAddress + pSymbol->st_size) )
        {
            return BELF_LookupSymbolByIndex(image, i, pDetails);
        }
    }

    return BERR_TRACE(BELF_ERR_SYMBOL_NOT_FOUND);    
}

BERR_Code BELF_LookupNearestSymbolByAddress(
    BELF_ImageHandle image,
    void *pAddress,                 /* Address of the symbol */
    BELF_SymbolDetails *pDetails    /* [out] */
    )
{
    const Elf32_Sym *pSymbol;
    unsigned i, numSymbols, nearest=0, nearestOffset=0xffffffff;
    BDBG_OBJECT_ASSERT(image, BELF_Image);
    BDBG_ASSERT(NULL != pDetails);

    /* Both the symbol and string tables are required for this to work */
    if ( NULL == image->pSymbolTable )
    {
        return BERR_TRACE(BELF_ERR_INVALID_SYMBOL_TABLE);
    }

    /* Locate the symbol table data */
    pSymbol = (const Elf32_Sym *)(((unsigned)image->imageSettings.pImageData) + image->pSymbolTable->details.imageOffset)+1;
    numSymbols = image->pSymbolTable->details.size / sizeof(Elf32_Sym);
    /* Walk through the symbol table looking for a matched symbol name */
    for ( i = 1; i < numSymbols; i++, pSymbol++ )
    {
        const char *pSymAddress = BELF_P_GetSymbolAddress(image, pSymbol);
        if ( (const char *)pAddress >= pSymAddress && (const char *)pAddress <= (pSymAddress + pSymbol->st_size) )
        {
            return BELF_LookupSymbolByIndex(image, i, pDetails);
        }
        else
        {
            unsigned offset;
            if ( pSymAddress > (const char *)pAddress )
            {
                offset = pSymAddress - (const char *)pAddress;
                if ( offset < nearestOffset )
                {
                    nearestOffset = offset;
                    nearest = i;
                }
            }
        }
    }

    if ( nearest > 0 )
    {
        return BELF_LookupSymbolByIndex(image, nearest, pDetails);
    }

    return BERR_TRACE(BELF_ERR_SYMBOL_NOT_FOUND);
}

BERR_Code BELF_LookupSectionByName(
    BELF_ImageHandle image,
    const char *pName,
    BELF_SectionDetails *pDetails    /* [out] */
    )
{
    BELF_SectionNode *pNode;

    BDBG_OBJECT_ASSERT(image, BELF_Image);
    BDBG_ASSERT(NULL != pName);
    BDBG_ASSERT(NULL != pDetails);

    for ( pNode = BLST_SQ_FIRST(&image->sectionList); pNode != NULL; pNode = BLST_SQ_NEXT(pNode, node) )
    {
        if ( !BELF_P_StrCmp(pName, pNode->details.pName) )
        {
            *pDetails = pNode->details;
            return BERR_SUCCESS;
        }
    }

    /* Section not found, initialize details structure to defaults */
    BKNI_Memset(pDetails, 0, sizeof(BELF_SectionDetails));
    return BERR_TRACE(BELF_ERR_SECTION_NOT_FOUND);
}

BERR_Code BELF_LookupSectionByIndex(
    BELF_ImageHandle image,
    unsigned index,
    BELF_SectionDetails *pDetails    /* [out] */
    )
{
    BELF_SectionNode *pNode;

    BDBG_OBJECT_ASSERT(image, BELF_Image);
    BDBG_ASSERT(NULL != pDetails);

    pNode = BELF_P_GetSectionByIndex(image, index);
    if ( NULL == pNode )
    {
        BKNI_Memset(pDetails, 0, sizeof(BELF_SectionDetails));
        return BERR_TRACE(BELF_ERR_SECTION_NOT_FOUND);
    }
    *pDetails = pNode->details;
    return BERR_SUCCESS;
}

BERR_Code BELF_PrintSymbols(
    BELF_ImageHandle image
    )
{
    const char *pString, *pSecString;
    const Elf32_Sym *pSymbol;
    unsigned numSymbols, i;

    BDBG_OBJECT_ASSERT(image, BELF_Image);

    /* Both the symbol and string tables are required for this to work */
    if ( NULL == image->pSymbolTable )
    {
        return BERR_TRACE(BELF_ERR_INVALID_SYMBOL_TABLE);
    }
    if ( NULL == image->pStringTable )
    {
        return BERR_TRACE(BELF_ERR_INVALID_STRING_TABLE);
    }

    /* Locate the string table data */
    pString = ((const char *)image->imageSettings.pImageData) + image->pStringTable->details.imageOffset;
    pSecString = ((const char *)image->imageSettings.pImageData) + image->pSectionHeader[image->pElfHeader->e_shstrndx].sh_offset;
    /* Locate the symbol table data */
    pSymbol = (const Elf32_Sym *)(((unsigned)image->imageSettings.pImageData) + image->pSymbolTable->details.imageOffset)+1;
    numSymbols = image->pSymbolTable->details.size / sizeof(Elf32_Sym);
    for ( i = 1; i < numSymbols; i++,pSymbol++ )
    {
        char secType;
        const char *pSecName;
        unsigned offset, address=0;

        offset = BELF_P_GetSymbolOffset(image, pSymbol);

        switch ( ELF32_ST_TYPE(pSymbol->st_info) )
        {
        case STT_OBJECT:
        case STT_FUNC:
            /* Only print info for code and data */
            break;
        default:
            continue;
        }

        if ( pSymbol->st_shndx == SHN_COMMON )
        {
            secType = 'C';
            pSecName = ".bss";
            address = (unsigned)image->pBss->details.pAddress;
            if ( address )
            {
                address += offset;
            }
        }
        else if ( pSymbol->st_shndx == SHN_UNDEF )
        {
            secType = 'U';
            pSecName = "Unresolved";
        }
        else if ( pSymbol->st_shndx == SHN_ABS )
        {
            secType = 'A';
            pSecName = "Absolute";
            address = pSymbol->st_value;
        }
        else if ( pSymbol->st_shndx >= SHN_LORESERV )
        {
            /* TODO: Add arch specific part of this to handle processor specific sections */
            secType = 'R';
            pSecName = "Reserved";
            address = pSymbol->st_value;
            BDBG_WRN(("Reserved section %d (%#x)", pSymbol->st_shndx, pSymbol->st_shndx));
        }
        else
        {
            BELF_SectionDetails secDetails;

            pSecName = pSecString + image->pSectionHeader[pSymbol->st_shndx].sh_name;
            if ( pSecName[0] && pSecName[1] )
            {
                secType = (ELF32_ST_BIND(pSymbol->st_info) == STB_LOCAL)?pSecName[1]:pSecName[1]-('a'-'A');
            }
            else
            {
                secType = '?';
            }
            if ( BERR_SUCCESS == BELF_LookupSectionByIndex(image, pSymbol->st_shndx, &secDetails) )
            {
                address  = (unsigned)secDetails.pAddress;
                if ( address )
                {
                    address += offset;
                }
            } 
        }

        BDBG_WRN(("%#x %c %s (%s+%#x)", address, secType, pString+pSymbol->st_name, pSecName, offset));
    }

    return BERR_SUCCESS;
}

