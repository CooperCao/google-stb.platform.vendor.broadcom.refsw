/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
#include "belf_mips.h"

BDBG_MODULE(belf_mips);

BERR_Code BELF_P_MIPS_ValidateHeader(BELF_ImageHandle image)
{
    unsigned data;

    /* We only support 32-bit data with the same endian as the host */
    if ( image->pElfHeader->e_ident[EI_CLASS] != ELFCLASS32 )
    {
        BDBG_ERR(("Only 32-bit MIPS targets are supported"));
        return BERR_TRACE(BELF_ERR_NOT32BIT);
    }

    #if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
    data = ELFDATA2LSB;
    #else
    data = ELFDATA2MSB;
    #endif

    if ( image->pElfHeader->e_ident[EI_DATA] != data )
    {
        BDBG_ERR(("File Endian Mismatch"));
        return BERR_TRACE(BELF_ERR_INVALID_ENDIAN);
    }

    /* If we're here, everything is good. */
    return BERR_SUCCESS;
}

BERR_Code BELF_P_MIPS_Init(BELF_ImageHandle image)
{
    if ( image->imageDetails.hasRelocations )
    {   
        BELF_MipsData *pData;
        pData = BKNI_Malloc(sizeof(BELF_MipsData));
        if ( NULL == pData )
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
        image->pArchData = pData;
    }

    return BERR_SUCCESS;
}

#if BDBG_DEBUG_BUILD
static const char *g_pRelNames[] = 
{
    "R_MIPS_NONE",
    "R_MIPS_16",
    "R_MIPS_32",
    "R_MIPS_REL32",
    "R_MIPS_26",
    "R_MIPS_HI16",
    "R_MIPS_LO16",
    "R_MIPS_GPREL16",
    "R_MIPS_LITERAL",
    "R_MIPS_GOT16",
    "R_MIPS_PC16",
    "R_MIPS_CALL16",
    "R_MIPS_GPREL32",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "R_MIPS_GOTHI16",
    "R_MIPS_GOTLO16",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "R_MIPS_CALLHI16",
    "R_MIPS_CALLLO16"
};
#endif

typedef struct BELF_MipsHi16Node
{
    BLST_SQ_ENTRY(BELF_MipsHi16Node) node;
    Elf32_Word *pAddress;
    Elf32_Word value;
} BELF_MipsHi16Node;

static BERR_Code BELF_P_MIPS_ProcessRel(BELF_ImageHandle image, BELF_SectionNode *pRelSection, BELF_SectionNode *pOrigSection)
{
    const Elf32_Rel *pRel = (const Elf32_Rel *)(((unsigned)image->imageSettings.pImageData) + pRelSection->pSectionHeader->sh_offset);
    const int numRel = pRelSection->pSectionHeader->sh_size / pRelSection->pSectionHeader->sh_entsize;
    unsigned sectionBase = (unsigned)pOrigSection->details.pAddress;
    const Elf32_Sym *pSymbols = (const Elf32_Sym *)(((unsigned)image->imageSettings.pImageData) + image->pSymbolTable->pSectionHeader->sh_offset);
    const char *pString;
    BELF_MipsHi16Node *pNode;
    BLST_SQ_HEAD(MipsHiList, BELF_MipsHi16Node) hiList;
    BERR_Code errCode = BERR_SUCCESS;
    unsigned AHI=0;
    int i;

    BLST_SQ_INIT(&hiList);

    /* Grab string table */
    if ( image->pStringTable )
    {
        pString = ((const char *)image->imageSettings.pImageData) + image->pStringTable->pSectionHeader->sh_offset;
    }
    else
    {
        pString = NULL;
    }

    for ( i = 0; i < numRel && errCode == BERR_SUCCESS; i++, pRel++ )
    {
        const Elf32_Word info = pRel->r_info;
        Elf32_Word sym, type, value, scratch;


        sym = ELF32_R_SYM(info);
        type = ELF32_R_TYPE(info);
        if ( type > R_MIPS_CALLLO16 )
        {
            BDBG_ERR(("Invalid relocation type %d", type));
            return BERR_TRACE(BELF_ERR_INVALID_RELOCATION);
        }
        BDBG_MSG(("Processing %s on symbol '%s'", g_pRelNames[type], pString?pString + pSymbols[sym].st_name:""));

        value = (Elf32_Word)BELF_P_GetSymbolAddress(image, pSymbols+sym);
        value -= (Elf32_Word)image->pLoadMemory;
        value += image->memoryBase;
        


        switch ( type )
        {
        case R_MIPS_NONE:
            break;
        case R_MIPS_32:
            /* The entire word is the addend here */
            scratch = *((Elf32_Word *)(sectionBase + pRel->r_offset));
            scratch += value;
            *((Elf32_Word *)(sectionBase + pRel->r_offset)) = scratch;
            break;
        case R_MIPS_26:
            {
                Elf32_Word P;
                if ( value & 0x3 )
                {
                    BDBG_ERR(("Unaligned relocation R_MIPS_26 on symbol %s", pString+pSymbols[sym].st_name));
                    return BERR_TRACE(BELF_ERR_INVALID_RELOCATION);
                }
                P = (sectionBase + pRel->r_offset) - (Elf32_Word)image->pLoadMemory + image->memoryBase;
                if ( (value & 0xf0000000) != ((P + 4) & 0xf0000000) )
                {
                    BDBG_ERR(("R_MIPS_26 relocation overflow on symbol %s", pString+pSymbols[sym].st_name));
                    return BERR_TRACE(BELF_ERR_INVALID_RELOCATION);
                }
                scratch = *(Elf32_Word *)(sectionBase + pRel->r_offset);
                scratch = (scratch & ~0x03ffffff) |
                            ((scratch + (value >> 2)) & 0x03ffffff);
                *(Elf32_Word *)(sectionBase + pRel->r_offset) = scratch;
            }
            break;
        case R_MIPS_HI16:
            /* Though the spec indicates that a HI16 should always be followed immediately by a LO16, this doesn't always happen.
               Add the HI16 entries found to a list and process them when the next LO16 is encountered. */
            pNode = BKNI_Malloc(sizeof(BELF_MipsHi16Node));
            if ( NULL == pNode )
            {
                errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                break;
            }
            pNode->pAddress = (Elf32_Word *)(sectionBase + pRel->r_offset);
            pNode->value = value;
            BLST_SQ_INSERT_HEAD(&hiList, pNode, node); 
            break;
        case R_MIPS_LO16:
            {
                Elf32_Word AHL, ALO;
                Elf32_Word scratchLo;

                scratchLo = *(Elf32_Word *)(sectionBase + pRel->r_offset);
                ALO = scratchLo & 0xffff;
                while ( NULL != (pNode = BLST_SQ_FIRST(&hiList)) )
                {
                    if ( value != pNode->value )
                    {
                        BDBG_ERR(("Invalid R_MIPS_HI16/LO16 pair encountered [value mismatch]"));
                        errCode = BERR_TRACE(BELF_ERR_INVALID_RELOCATION);
                        break;
                    }
                    scratch = *pNode->pAddress;
                    AHI = scratch & 0xffff;
                    AHL = (AHI << 16) + (int16_t)ALO;
                    scratch = scratch & 0xffff0000;
                    scratch |= ((AHL + value) - (int16_t)(AHL + value)) >> 16;
                    *pNode->pAddress = scratch;
                    BLST_SQ_REMOVE_HEAD(&hiList, node); 
                    BKNI_Free(pNode);
                }
                /* recompute this in case we didn't find any high nodes and have an orphan LO */
                AHL = (AHI << 16) + (int16_t)ALO;
                scratchLo = (scratchLo & 0xffff0000) | ((AHL+value) & 0xffff);
                *(Elf32_Word *)(sectionBase + pRel->r_offset) = scratchLo;
            }
            break;
        case R_MIPS_PC16:
            {
                int iScratch;                
                /* PC16 relocations are always in the same section, so just compare offsets */
                iScratch = (unsigned)pSymbols[sym].st_value - (unsigned)pRel->r_offset;
                iScratch /= 4;  /* Convert to instructions */
                iScratch--;     /* Account for delay slot */
                if ( iScratch > 32768 || iScratch < -32767 )
                {
                    BDBG_ERR(("PC16 Relocation out of range %d (value=%#x addr=%#x)", iScratch, value, sectionBase + pRel->r_offset));
                    errCode = BERR_TRACE(BELF_ERR_INVALID_RELOCATION);
                    break;
                }
                scratch = *(Elf32_Word *)(sectionBase + pRel->r_offset);
                scratch = (scratch & 0xffff0000) | (iScratch&0xffff);
                *(Elf32_Word *)(sectionBase + pRel->r_offset) = scratch;
            }
            break;
        default:
            BDBG_ERR(("Unhandled relocation type %d (%s) encountered", type, g_pRelNames[type]));
            errCode = BERR_TRACE(BELF_ERR_INVALID_RELOCATION);
            break;
        }
    }


    while ( NULL != (pNode = BLST_SQ_FIRST(&hiList)) )
    {
        BLST_SQ_REMOVE_HEAD(&hiList, node);
        BKNI_Free(pNode);
    }

    return errCode;
}

static BERR_Code BELF_P_MIPS_ProcessRelA(BELF_ImageHandle image, BELF_SectionNode *pRelSection, BELF_SectionNode *pOrigSection)
{
    BSTD_UNUSED(image);
    BSTD_UNUSED(pRelSection);
    BSTD_UNUSED(pOrigSection);
    /* Not supported yet */
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BELF_P_MIPS_Relocate(BELF_ImageHandle image)
{
    BERR_Code errCode;
    const Elf32_Shdr *pSectionHeader = image->pSectionHeader;
    unsigned i, num;

    num = image->pElfHeader->e_shnum;
    /* Walk through the section headers in the file, looking for REL or RELA types. */
    for ( i = 0; i < num; i++ )
    {
        unsigned type = pSectionHeader[i].sh_type;
        if ( type == SHT_REL || type == SHT_RELA )
        {
            BELF_SectionNode *pRel, *pOrig;
            pRel = BELF_P_GetSectionByIndex(image, i);
            BDBG_ASSERT(NULL != pRel);
            pOrig = BELF_P_GetSectionByIndex(image, pSectionHeader[i].sh_info);
            BDBG_ASSERT(NULL != pOrig);

            /* If the original section is not allocated, skip it. */
            if ( pOrig->pSectionHeader->sh_flags & SHF_ALLOC )
            {
                BDBG_MSG(("Processing %s section %s referring to original %s", (type==SHT_REL)?"REL":"RELA", pRel->details.pName, pOrig->details.pName));
                if ( type == SHT_REL )
                {
                    errCode = BELF_P_MIPS_ProcessRel(image, pRel, pOrig);
                    if ( errCode )
                    {
                        return BERR_TRACE(errCode);
                    }
                }
                else
                {
                    errCode = BELF_P_MIPS_ProcessRelA(image, pRel, pOrig);
                    if ( errCode )
                    {
                        return BERR_TRACE(errCode);
                    }
                }
            }
        }
    }

    return BERR_SUCCESS;
}

void BELF_P_MIPS_Uninit(BELF_ImageHandle image)
{
    if ( image->pArchData )
    {
        BKNI_Free(image->pArchData);
    }
}

BERR_Code BELF_P_MIPS_GetSectionPriority(const BELF_ImageHandle image, const BELF_SectionNode *pNode, unsigned *pPriority)
{
    const Elf32_Shdr *pSectionHeader = image->pSectionHeader;

    pSectionHeader += pNode->details.index;

    *pPriority = -1;

    /* Only prioritize allocated sections, others will fall to the back of the list */
    if ( pSectionHeader->sh_flags & SHF_ALLOC )
    {
        /* Executable is highest priority */
        if ( pSectionHeader->sh_flags & SHF_EXECINSTR )
        {
            *pPriority = 0;
        }
        /* Read-only is next (not small) */
        else if ( 0 == (pSectionHeader->sh_flags & (SHF_WRITE|SHF_MIPS_GPREL)) )
        {
            *pPriority = 1;
        }
        /* Read-only small is next */
        else if ( 0 == (pSectionHeader->sh_flags & SHF_WRITE) )
        {
            *pPriority = 2;
        }
        /* Read-write small is next */
        else if ( pSectionHeader->sh_flags & SHF_MIPS_GPREL )
        {
            *pPriority = 3;
        }
        /* .bss is last */
        else if ( pSectionHeader->sh_flags & SHF_MIPS_GPREL )
        {
            *pPriority = 4;
        }
    }

    return BERR_SUCCESS;
}

