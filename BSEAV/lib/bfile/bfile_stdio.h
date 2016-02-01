/***************************************************************************
 *     Copyright (c) 2006-2011, Broadcom Corporation
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
#ifndef __BFILE_STDIO_H__
#define __BFILE_STDIO_H__
#include "bfile_io.h"
#include <stdio.h>

#if defined(_WIN32) || defined(__vxworks) || defined (B_REFSW_ANDROID)
#define fopen64 fopen
#define fseeko fseek
#define ftello ftell
#endif

#ifdef __cplusplus
extern "C"
{
#endif

bfile_io_read_t bfile_stdio_read_attach(FILE *fin);
void bfile_stdio_read_detach(bfile_io_read_t file);

bfile_io_write_t bfile_stdio_write_attach(FILE *fout);
void bfile_stdio_write_detach(bfile_io_read_t file);

#ifdef __cplusplus
}
#endif

#endif /* __BFILE_STDIO_H__ */

