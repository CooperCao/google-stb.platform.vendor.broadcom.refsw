/***************************************************************************
 *     Copyright (c) 2007-2011, Broadcom Corporation
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
 * Block based cached file I/O
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BFILE_UTIL_H__
#define _BFILE_UTIL_H__

#include "bfile_io.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
Summary:
    This function allows to shift read point in the file
Description:
    This function would create new bfile_io_read_t which is identical to the
    original except that all data access is shifted by 'offset' bytes
*/
bfile_io_read_t bfile_read_offset_attach(bfile_io_read_t source, off_t offset);

/*
Summary:
    This function releases resources allocated in the bfile_offset_attach 
*/ 
void bfile_read_offset_detach(bfile_io_read_t file);

/*
Summary:
    This function allows to insert static header into the file
Description:
    This function would create new bfile_io_read_t which is identical to the
    original except that all data access is prefixed by the static header 
*/
bfile_io_read_t bfile_read_header_attach(bfile_io_read_t source, const void *header, size_t header_size);

/*
Summary:
    This function releases resources allocated in the bfile_read_header_attach
*/ 
void bfile_read_header_detach(bfile_io_read_t file);

#ifdef __cplusplus
}
#endif


#endif /* _BFILE_UTIL_H__ */


