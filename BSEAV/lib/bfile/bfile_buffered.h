/***************************************************************************
 *     Copyright (c) 2007-2008, Broadcom Corporation
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
#ifndef _BFILE_BUFFERED_H__
#define _BFILE_BUFFERED_H__

#include "bfile_io.h"
#include "bioatom.h"
#include "bfile_buffer.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bfile_buffered *bfile_buffered_t;

typedef struct bfile_buffered_cfg {
	unsigned nsegs;
	size_t buf_len;
} bfile_buffered_cfg;


void bfile_buffered_default_cfg(bfile_buffered_cfg *cfg);
bfile_buffered_t bfile_buffered_create(batom_factory_t factory, bfile_buffered_cfg *cfg);
void bfile_buffered_destroy(bfile_buffered_t file);
bfile_io_read_t bfile_buffered_attach(bfile_buffered_t file, bfile_io_read_t fd);
bfile_buffer_t bfile_buffered_get_buffer(bfile_buffered_t file);
void bfile_buffered_detach(bfile_buffered_t file);

#ifdef __cplusplus
}
#endif


#endif /* _BFILE_BUFFERED_H__ */


