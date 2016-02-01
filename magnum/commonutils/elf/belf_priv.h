/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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
#ifndef BELF_PRIV_H__
#define BELF_PRIV_H__

#include "bkni.h"
#include "bdbg.h"
#include "berr.h"
#include "berr_ids.h"
#include "belf.h"
#include "blst_squeue.h"
#include "belf_types.h"

BDBG_OBJECT_ID_DECLARE(BELF_SectionNode);
typedef struct BELF_SectionNode
{
    BDBG_OBJECT(BELF_SectionNode)
    BLST_SQ_ENTRY(BELF_SectionNode) node;
    unsigned priority;
    const Elf32_Shdr *pSectionHeader;
    BELF_SectionDetails details;
} BELF_SectionNode;

BDBG_OBJECT_ID_DECLARE(BELF_CommonSymbolNode);
typedef struct BELF_CommonSymbolNode
{
    BDBG_OBJECT(BELF_CommonSymbolNode)
    BLST_SQ_ENTRY(BELF_CommonSymbolNode) node;
    const Elf32_Sym *pSymbol;
    unsigned index;
    unsigned bssOffset;
} BELF_CommonSymbolNode;

BDBG_OBJECT_ID_DECLARE(BELF_Image);
typedef struct BELF_Image
{
    BDBG_OBJECT(BELF_Image)
    BELF_ImageSettings imageSettings;
    BELF_ImageDetails imageDetails;
    void *pArchData;
    const Elf32_Ehdr *pElfHeader;
    const Elf32_Shdr *pSectionHeader;
    const BELF_SectionNode *pSymbolTable;
    const BELF_SectionNode *pStringTable;
    BELF_SectionNode *pBss;
    char *pLoadMemory;
    uint32_t memoryBase;
    BLST_SQ_HEAD(BELF_SectionList, BELF_SectionNode) sectionList;
    BLST_SQ_HEAD(BELF_CommonSymbolList, BELF_CommonSymbolNode) commonSymbolList;
} BELF_Image;

/* Helper functions */
int BELF_P_StrCmp(const char *pString1, const char *pString2);
unsigned BELF_P_GetSymbolOffset(BELF_ImageHandle image, const Elf32_Sym *pSymbol);
const void * BELF_P_GetSymbolAddress(BELF_ImageHandle image, const Elf32_Sym *pSymbol);

/* Generic routines */
BERR_Code BELF_P_ValidateHeader(BELF_ImageHandle image);
BERR_Code BELF_P_InitSections(BELF_ImageHandle image);
void BELF_P_UninitSections(BELF_ImageHandle image);
BERR_Code BELF_P_InitSymbols(BELF_ImageHandle image);
void BELF_P_UninitSymbols(BELF_ImageHandle image);
BERR_Code BELF_P_LoadImage(BELF_ImageHandle image);
BERR_Code BELF_P_CopySections(BELF_ImageHandle image);
BELF_SectionNode *BELF_P_GetSectionByIndex(BELF_ImageHandle image, unsigned index);
BERR_Code BELF_P_ComputeImageSize(BELF_ImageHandle image);

/* Architecture-specific routines */
BERR_Code BELF_P_ARCH_Init(BELF_ImageHandle image);
BERR_Code BELF_P_ARCH_Relocate(BELF_ImageHandle image);
void BELF_P_ARCH_Uninit(BELF_ImageHandle image);

#endif /* #ifndef BELF_PRIV_H__ */

