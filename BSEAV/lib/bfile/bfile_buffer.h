/***************************************************************************
 *     Copyright (c) 2007-2009, Broadcom Corporation
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
#ifndef _BFILE_BUFFER_H__
#define _BFILE_BUFFER_H__

#include "bfile_io.h"
#include "bioatom.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bfile_buffer *bfile_buffer_t;

typedef enum bfile_buffer_overflow_action {
    bfile_buffer_overflow_action_wait,
    bfile_buffer_overflow_action_abort
} bfile_buffer_overflow_action;

typedef struct bfile_buffer_cfg {
	bool async;
	unsigned nsegs;
	size_t buf_len;
	void *buf;
	void *sync_cnxt;
	bfile_io_read_t fd;
	void (*async_read)(void *sync_cnxt, bfile_io_read_t fd, void *buf, size_t length, void (*read_cont)(void *cont, ssize_t size), void *cntx);
    bfile_buffer_overflow_action (*async_overflow)(void *sync_cnxt);
} bfile_buffer_cfg;

typedef enum bfile_buffer_result {
	bfile_buffer_result_ok,
	bfile_buffer_result_read_error,
	bfile_buffer_result_eof,
    bfile_buffer_result_no_data, /* temporary no data in the file */
	bfile_buffer_result_async,
	bfile_buffer_result_buffer_overflow
} bfile_buffer_result;

void bfile_buffer_default_cfg(bfile_buffer_cfg *cfg);
bfile_buffer_t bfile_buffer_create(batom_factory_t factory, bfile_buffer_cfg *cfg);
void bfile_buffer_clear(bfile_buffer_t buf);
void bfile_buffer_destroy(bfile_buffer_t buf);
	
batom_t bfile_buffer_async_read(bfile_buffer_t buf, off_t off, size_t length, bfile_buffer_result *result, void (*read_complete)(void *, batom_t, bfile_buffer_result ), void *cntx);
batom_t bfile_buffer_read(bfile_buffer_t buf, off_t off, size_t length, bfile_buffer_result *result);
int bfile_buffer_get_bounds(bfile_buffer_t buf, off_t *first, off_t *last);

#ifdef __cplusplus
}
#endif


#endif /* _BFILE_BUFFER_H__ */


