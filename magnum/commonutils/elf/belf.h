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
#ifndef BELF_H__
#define BELF_H__

#include "bstd.h"
#include "belf_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	Settings for an ELF image

Description:
    This structure defines the inputs for an ELF image file.  The file should
    be loaded into memory at the address specified.  If reloation is possible,
    a memory handle must be provided as well - otherwise the image will be
    loaded at the addresses within the image.

See Also:
	BELF_GetDefaultImageSettings()

****************************************************************************/
typedef struct BELF_ImageSettings
{
    const void *pImageData;     /* Pointer to image data (required) */
    size_t imageLength;         /* Length of image data (required) */
} BELF_ImageSettings;


/***************************************************************************
Summary:
	Initialize image settings to default values

See Also:
	BELF_OpenImage()

****************************************************************************/
void BELF_GetDefaultImageSettings(
    BELF_ImageSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
	ELF Image Handle

See Also:
	BELF_OpenImage()

****************************************************************************/
typedef struct BELF_Image *BELF_ImageHandle;

/***************************************************************************
Summary:
	Open an ELF file and parse the header information

See Also:
	BELF_CloseImage() BELF_LoadImage()

****************************************************************************/
BERR_Code BELF_OpenImage(
    const BELF_ImageSettings *pSettings,
    BELF_ImageHandle *pHandle   /* [out] Image Handle */
    );

/***************************************************************************
Summary:
	Close an ELF file

See Also:
	BELF_OpenImage()

****************************************************************************/
void BELF_CloseImage(
    BELF_ImageHandle image
    );

/***************************************************************************
Summary:
	ELF Image Types
****************************************************************************/
typedef enum BELF_ImageType
{
    BELF_ImageType_eNone,
    BELF_ImageType_eRelocatable,
    BELF_ImageType_eExecutable,
    BELF_ImageType_eShared,
    BELF_ImageType_eCore,
    BELF_ImageType_eUnknown,
    BELF_ImageType_eMax
} BELF_ImageType;

/***************************************************************************
Summary:
	ELF Image Types
****************************************************************************/
typedef enum BELF_MachineType
{
    BELF_MachineType_eNone,
    BELF_MachineType_eM32,      /* AT&T WE 32100 */
    BELF_MachineType_eSparc,    /* SPARC */
    BELF_MachineType_e386,      /* Intel 80386 */
    BELF_MachineType_e68k,      /* Motorola 68000 */
    BELF_MachineType_e88k,      /* Motorola 88000 */
    BELF_MachineType_e860,      /* Intel 80860 */
    BELF_MachineType_eMips,     /* MIPS R3000 */
    BELF_MachineType_eMax
} BELF_MachineType;

/***************************************************************************
Summary:
	Get information about an ELF file prior to loading

****************************************************************************/
typedef struct BELF_ImageDetails
{
    BELF_ImageType type;            /* File type.  Currently, only Executable and Relocatable are supported */
    BELF_MachineType machine;       /* Machine type.  Currently, only MIPS is supported. */
    bool littleEndian;              /* true=Little Endian, false=Big Endian.  Currently, only formats matching host endian are supported. */
    bool is64Bit;                   /* If true, the file is 64-bit.  Otherwise 32-bit. */
    bool hasRelocations;            /* If true, relocations are present in the file. */
    unsigned numSymbols;            /* Number of symbols in the symbol table if present. 0 if no symbol table exists */
    unsigned numSections;           /* Number of sections in the file */
    unsigned memoryRequired;        /* Amount of memory in bytes required to load the image */
    unsigned memoryAlignment;       /* Memory alignment in bytes required for the image memory */
} BELF_ImageDetails;

/***************************************************************************
Summary:
	Get information about an ELF file prior to loading

****************************************************************************/
BERR_Code BELF_GetImageDetails(
    BELF_ImageHandle image,
    BELF_ImageDetails *pDetails /* [out] */
    );

/***************************************************************************
Summary:
    Load an ELF image into memory, performing relocation if necessary
 
Description:
    This will load the image into memory, performing relocation if required.
    If relocating, the app must provide the memory address and length of
    allocated memory.  Memory requirements are found by calling
    BELF_GetImageDetails().
 
See Also: 
    BELF_GetImageDetails() 
 
****************************************************************************/
BERR_Code BELF_LoadImage(
    BELF_ImageHandle image,
    void *pMemory,              /* Required for relocation.  Pass NULL if ignored. */
    uint32_t memoryBase,        /* For relocation, this is the actual address the target will execute from.  If you are either loading
                                   into MMU-translated memory or running from MMU-translated memory, this may not be the same as the
                                   pMemory address.  Pass 0 if relocation is not being performed. */
    void **pEntry               /* [out] Program entry address (relative to pMemory) */
    );

/***************************************************************************
Summary:
    Symbol Binding
 
****************************************************************************/ 
typedef enum BELF_SymbolBinding
{
    BELF_SymbolBinding_eLocal,
    BELF_SymbolBinding_eGlobal,
    BELF_SymbolBinding_eWeak,
    BELF_SymbolBinding_eOther
} BELF_SymbolBinding;

/***************************************************************************
Summary:
    Symbol Types
 
****************************************************************************/ 
typedef enum BELF_SymbolType
{
    BELF_SymbolType_eNone,
    BELF_SymbolType_eObject,
    BELF_SymbolType_eFunction,
    BELF_SymbolType_eSection,
    BELF_SymbolType_eFile,
    BELF_SymbolType_eOther
} BELF_SymbolType;

/***************************************************************************
Summary:
    Symbol Information
 
****************************************************************************/ 
typedef struct BELF_SymbolDetails
{
    void *pAddress;             /* Address the symbol has been loaded into, requires BELF_LoadImage to be called first if the file is relocatable. */
    unsigned size;              /* Size of the symbol in bytes */
    unsigned section;           /* Section number containing the symbol */
    BELF_SymbolBinding binding; /* Symbol Binding */
    BELF_SymbolType type;       /* Symbol Type */
    const char *pName;          /* Symbol Name.  Pointer will always be valid, but may be an empty string if no string table is present. */
} BELF_SymbolDetails;

/***************************************************************************
Summary:
    Lookup a symbol by name

****************************************************************************/
BERR_Code BELF_LookupSymbolByName(
    BELF_ImageHandle image,
    const char *pName,
    BELF_SymbolDetails *pDetails    /* [out] */
    );

/***************************************************************************
Summary:
    Lookup a symbol by index in the symbol table

****************************************************************************/
BERR_Code BELF_LookupSymbolByIndex(
    BELF_ImageHandle image,
    unsigned index,
    BELF_SymbolDetails *pDetails    /* [out] */
    );

/***************************************************************************
Summary:
    Lookup a symbol by address in the symbol table
 
 Description:
     This function will look for a symbol existing at the address provided.
     For relocatable files, BELF_LoadImage must have been called prior
     to calling this routine.  Addresses are relative to the memory the image
     is loaded into - pMemory in BELF_LoadImage().
****************************************************************************/
BERR_Code BELF_LookupSymbolByAddress(
    BELF_ImageHandle image,
    void *pAddress,                 /* Address of the symbol */
    BELF_SymbolDetails *pDetails    /* [out] */
    );

/***************************************************************************
Summary:
    Lookup the nearest symbol by address in the symbol table
 
 Description:
     This function will look for the symbol closest to the address provided.
     The returned symbol address will always be <= the one provided.
     For relocatable files, BELF_LoadImage must have been called prior
     to calling this routine.  Addresses are relative to the memory the image
     is loaded into - pMemory in BELF_LoadImage().
****************************************************************************/
BERR_Code BELF_LookupNearestSymbolByAddress(
    BELF_ImageHandle image,
    void *pAddress,                 /* Address of the symbol */
    BELF_SymbolDetails *pDetails    /* [out] */
    );

/***************************************************************************
Summary:
    ELF Section Type

****************************************************************************/
typedef enum BELF_SectionType
{
    BELF_SectionType_eNull,
    BELF_SectionType_eProgramBits,
    BELF_SectionType_eSymbolTable,
    BELF_SectionType_eStringTable,
    BELF_SectionType_eRelA,
    BELF_SectionType_eHash,
    BELF_SectionType_eDynamic,
    BELF_SectionType_eNote,
    BELF_SectionType_eNobits,
    BELF_SectionType_eRel,
    BELF_SectionType_eShlib,
    BELF_SectionType_eOsSpecific,
    BELF_SectionType_eProcessorSpecific,
    BELF_SectionType_eUserDefined
} BELF_SectionType;

/***************************************************************************
Summary:
    ELF Section Details

****************************************************************************/
typedef struct BELF_SectionDetails
{
    BELF_SectionType type;      /* Section Type */
    size_t imageOffset;         /* Offset of the section in the image */
    void *pAddress;             /* Set to 0 for relocatable sections until BELF_LoadImage has been called */
    unsigned size;              /* Section length in bytes */
    unsigned alignment;         /* Seection alignment requirement in bytes */
    unsigned index;             /* Which section number this represents */
    const char *pName;          /* Section Name.  Pointer will always be valid, but may be an empty string if no string table is present. */
} BELF_SectionDetails;

/***************************************************************************
Summary:
    Lookup a section by name

****************************************************************************/
BERR_Code BELF_LookupSectionByName(
    BELF_ImageHandle image,
    const char *pName,
    BELF_SectionDetails *pDetails    /* [out] */
    );

/***************************************************************************
Summary:
    Lookup a section by index in the section header

****************************************************************************/
BERR_Code BELF_LookupSectionByIndex(
    BELF_ImageHandle image,
    unsigned index,
    BELF_SectionDetails *pDetails    /* [out] */
    );

/***************************************************************************
Summary:
    Print symbol table to the debug interface (prints at warning level)

****************************************************************************/
BERR_Code BELF_PrintSymbols(
    BELF_ImageHandle image
    );

#ifdef __cplusplus
}
#endif

#endif

