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
#ifndef BELF_MIPS_H__
#define BELF_MIPS_H__

/* MIPS specific data */
typedef struct BELF_MipsData
{
    void *pGpAddress;
    unsigned gpOffset;
} BELF_MipsData;

/* MIPS relocation types */
#define R_MIPS_NONE          0
#define R_MIPS_16            1
#define R_MIPS_32            2
#define R_MIPS_REL32         3
#define R_MIPS_26            4
#define R_MIPS_HI16          5
#define R_MIPS_LO16          6
#define R_MIPS_GPREL16       7
#define R_MIPS_LITERAL       8
#define R_MIPS_GOT16         9
#define R_MIPS_PC16         10
#define R_MIPS_CALL16       11
#define R_MIPS_GPREL32      12
#define R_MIPS_GOTHI16      21
#define R_MIPS_GOTLO16      22
#define R_MIPS_CALLHI16     30
#define R_MIPS_CALLLO16     31

/* MIPS section indexes */
#define SHN_MIPS_ACOMMON    (SHN_LOPROC + 0)
#define SHN_MIPS_TEXT       (SHN_LOPROC + 1)
#define SHN_MIPS_DATA       (SHN_LOPROC + 2)
#define SHN_MIPS_SCOMMON    (SHN_LOPROC + 3)
#define SHN_MIPS_SUNDEFINED (SHN_LOPROC + 4)

/* MIPS section types */
#define SHT_MIPS_LIBLIST    (SHT_LOPROC + 0)
#define SHT_MIPS_CONFLICT   (SHT_LOPROC + 2)
#define SHT_MIPS_GPTAB      (SHT_LOPROC + 3)
#define SHT_MIPS_UCODE      (SHT_LOPROC + 4)
#define SHT_MIPS_DEBUG      (SHT_LOPROC + 5)
#define SHT_MIPS_REGINFO    (SHT_LOPROC + 6)

/* MIPS section flags */
#define SHF_MIPS_GPREL     0x10000000

/* MIPS specific routines */
BERR_Code BELF_P_MIPS_ValidateHeader(BELF_ImageHandle image);
BERR_Code BELF_P_MIPS_Init(BELF_ImageHandle image);
BERR_Code BELF_P_MIPS_Relocate(BELF_ImageHandle image);
void BELF_P_MIPS_Uninit(BELF_ImageHandle image);
BERR_Code BELF_P_MIPS_GetSectionPriority(const BELF_ImageHandle image, const BELF_SectionNode *pNode, unsigned *pPriority);

#endif /* #ifndef BELF_MIPS_H__ */

