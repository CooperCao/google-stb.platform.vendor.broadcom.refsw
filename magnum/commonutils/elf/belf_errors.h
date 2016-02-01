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
#ifndef BELF_ERRORS_H__
#define BELF_ERRORS_H__

#define BELF_ERR_BAD_MAGIC_NUMBER                   BERR_MAKE_CODE(BERR_ELF_ID, 0x0000)
#define BELF_ERR_NOT32BIT                           BERR_MAKE_CODE(BERR_ELF_ID, 0x0001)
#define BELF_ERR_INVALID_MACHINE                    BERR_MAKE_CODE(BERR_ELF_ID, 0x0002)
#define BELF_ERR_INVALID_ENDIAN                     BERR_MAKE_CODE(BERR_ELF_ID, 0x0003)
#define BELF_ERR_INVALID_VERSION                    BERR_MAKE_CODE(BERR_ELF_ID, 0x0004)
#define BELF_ERR_INVALID_PROGRAM_HEADER             BERR_MAKE_CODE(BERR_ELF_ID, 0x0005)
#define BELF_ERR_INVALID_SECTION_HEADER             BERR_MAKE_CODE(BERR_ELF_ID, 0x0006)
#define BELF_ERR_INVALID_STRING_TABLE               BERR_MAKE_CODE(BERR_ELF_ID, 0x0007)
#define BELF_ERR_INVALID_SYMBOL_TABLE               BERR_MAKE_CODE(BERR_ELF_ID, 0x0008)
#define BELF_ERR_INVALID_SYMBOL                     BERR_MAKE_CODE(BERR_ELF_ID, 0x0009)
#define BELF_ERR_UNDEFINED_SYMBOLS                  BERR_MAKE_CODE(BERR_ELF_ID, 0x000a)
#define BELF_ERR_INVALID_RELOCATION                 BERR_MAKE_CODE(BERR_ELF_ID, 0x000b)
#define BELF_ERR_SYMBOL_NOT_FOUND                   BERR_MAKE_CODE(BERR_ELF_ID, 0x000c)
#define BELF_ERR_SECTION_NOT_FOUND                  BERR_MAKE_CODE(BERR_ELF_ID, 0x000d)
#define BELF_ERR_UNSUPPORTED_IMAGE_TYPE             BERR_MAKE_CODE(BERR_ELF_ID, 0x000e)
#define BELF_ERR_INVALID_SECTION                    BERR_MAKE_CODE(BERR_ELF_ID, 0x000f)

/* Architecture-specific errors below */
#define BELF_ERR_ARCH_BASE                          0x100

#endif /* #ifndef BELF_ERRORS_H__ */

