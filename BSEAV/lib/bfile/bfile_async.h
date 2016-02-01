/***************************************************************************
 *     Copyright (c) 2007, Broadcom Corporation
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
 * Single thread asynchronous I/O
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BFILE_ASYNC_H__
#define _BFILE_ASYNC_H__

#include "bfile_io.h"

#ifdef __cplusplus
extern "C"
{
#endif

void bfile_async_init(void);
void bfile_async_shutdown(void);
void bfile_async_write(void *sync_cnxt, bfile_io_write_t fd, const void *buf, size_t length, void (*cont)(void *, ssize_t ), void *cntx);
void bfile_async_read(void *sync_cnxt, bfile_io_read_t fd, void *buf, size_t length, void (*cont)(void *, ssize_t ), void *cntx);
void bfile_async_process(void);


#ifdef __cplusplus
}
#endif


#endif /* _BFILE_ASYNC_H__ */


