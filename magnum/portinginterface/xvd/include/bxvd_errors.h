/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BXVD_ERRORS_H__
#define BXVD_ERRORS_H__

#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes unique to BXVD.  Error has been reserved by berr module.
 * Each module has 0xfffff (64k) of errors. 
	*/

/* Video decoder subsystem related errors */
#define BXVD_ERR_QUEUE_CORRUPTED         BERR_MAKE_CODE(BERR_XVD_ID, 0x000)
#define BXVD_ERR_QUEUE_EMPTY             BERR_MAKE_CODE(BERR_XVD_ID, 0x001)
#define BXVD_ERR_QUEUE_FULL              BERR_MAKE_CODE(BERR_XVD_ID, 0x002)
#define BXVD_ERR_USERDATA_NONE           BERR_MAKE_CODE(BERR_XVD_ID, 0x003)
#define BXVD_ERR_USERDATA_INVALID        BERR_MAKE_CODE(BERR_XVD_ID, 0x004)
#define BXVD_ERR_USERDATA_DISABLED       BERR_MAKE_CODE(BERR_XVD_ID, 0x005)
#define BXVD_ERR_USERDATA_UNINITED       BERR_MAKE_CODE(BERR_XVD_ID, 0x006)
#define BXVD_ERR_USERDATA_USRBFROFL      BERR_MAKE_CODE(BERR_XVD_ID, 0x007)
#define BXVD_ERR_USERDATA_ITEM_TOO_LARGE BERR_MAKE_CODE(BERR_XVD_ID, 0x008)

/* FW image handling errors */
#define BXVD_ERR_EOF                 BERR_MAKE_CODE(BERR_XVD_ID, 0x008)

/* FW communication errors */
#define BXVD_ERR_FW_IS_BUSY          BERR_MAKE_CODE(BERR_XVD_ID, 0x009)

/* State machine errors */
#define BXVD_ERR_DECODER_INACTIVE    BERR_MAKE_CODE(BERR_XVD_ID, 0x00A)
#define BXVD_ERR_DECODER_ACTIVE      BERR_MAKE_CODE(BERR_XVD_ID, 0x00B)

/* Relocation engine errors */
#define BXVD_ERR_RELF_BAD_INPUT      BERR_MAKE_CODE(BERR_XVD_ID, 0x00C)
#define BXVD_ERR_RELF_BAD_HEADER     BERR_MAKE_CODE(BERR_XVD_ID, 0x00D)
#define BXVD_ERR_RELF_BAD_SECTION    BERR_MAKE_CODE(BERR_XVD_ID, 0x00E)
#define BXVD_ERR_RELF_BAD_RELOC_TYPE BERR_MAKE_CODE(BERR_XVD_ID, 0x00F)
#define BXVD_ERR_RELF_NO_EOC_FOUND   BERR_MAKE_CODE(BERR_XVD_ID, 0x010)
#define BXVD_ERR_MULT_SYM_TABLE_REFS BERR_MAKE_CODE(BERR_XVD_ID, 0x011)
#define BXVD_ERR_CANT_READ_SYMTAB    BERR_MAKE_CODE(BERR_XVD_ID, 0x012)

/* Decoder debug logging errors */
#define BXVD_ERR_DEBUG_LOG_NOBUFFER  BERR_MAKE_CODE(BERR_XVD_ID, 0x013)
#define BXVD_ERR_DEBUG_LOG_OVERFLOW  BERR_MAKE_CODE(BERR_XVD_ID, 0x014)

/* Invalid handle error code */
#define BXVD_ERR_INVALID_HANDLE      BERR_MAKE_CODE(BERR_XVD_ID, 0x015)

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXVD_ERRORS_H__ */
/* End of file. */
