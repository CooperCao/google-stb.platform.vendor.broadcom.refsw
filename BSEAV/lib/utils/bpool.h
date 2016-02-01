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
 * Pool alloc library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef __BPOOL_H__
#define __BPOOL_H__ 
#include "balloc.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bpool *bpool_t;
typedef struct bpool_status {
	unsigned pools;
	unsigned free;
	unsigned allocated;
} bpool_status;

bpool_t bpool_create(balloc_iface_t alloc, size_t nelem, size_t elem_size);
void bpool_destroy(bpool_t pool);

void *bpool_alloc(bpool_t pool, size_t size);
void bpool_free(bpool_t pool, void *ptr);
bool bpool_test_block(bpool_t pool, void *ptr);
void bpool_get_status(bpool_t pool, bpool_status *status);
balloc_iface_t bpool_alloc_iface(bpool_t pool);

void bpool_join(bpool_t parent, bpool_t child);
bool bpool_is_empty(bpool_t pool);
void bpool_detach(bpool_t parent, bpool_t child);
bpool_t bpool_last_child(bpool_t pool);
void bpool_dump(bpool_t pool);

#ifdef __cplusplus
}
#endif

#endif /* __BPOOL_H__ */

