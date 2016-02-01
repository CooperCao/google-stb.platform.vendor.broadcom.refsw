/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
/***************************************************************
**
** Broadcom Corp. Confidential
** Copyright 1998-2006 Broadcom Corp. All Rights Reserved.
**
** THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED
** SOFTWARE LICENSE AGREEMENT BETWEEN THE USER AND BROADCOM.
** YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT
** SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
**
** File:		cache_util.h
** Description: 	cache handling utilities.
**
** Created: 06/07/04 by Jeff Fisher
**
**
**
****************************************************************/

#ifndef __CACHEUTIL_H__
#define __CACHEUTIL_H__

#include "bstd.h"

#if 0 /* (BCHP_CHIP!=7550) */
#define dcache_size			g_p_dsp->cache.d_size
#define dcache_linesize		g_p_dsp->cache.d_linesize
#define icache_size			g_p_dsp->cache.i_size
#define icache_linesize		g_p_dsp->cache.i_linesize
#endif
/* function prototypes */
#ifdef __cplusplus
extern "C" {
#endif

void calc_cache_sizes(void);
void print_cache_sizes(void);
void invalidate_icache_all(void);
void flush_dcache(unsigned int start, unsigned int end);
void flush_dcache_all(void);
void invalidate_dcache(unsigned int start, unsigned int end);
void invalidate_dcache_all(void);


#ifdef __cplusplus
}
#endif

#endif /* __CACHEUTIL_H__ */


