/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef _GMEM_TALLOC_H
#define _GMEM_TALLOC_H

extern void *talloc_initialize(void);
extern void talloc_term(void *h);
extern bool talloc_alloc(void *h, size_t size, v3d_size_t align, void **cur_map_ptr, v3d_addr_t *cur_lock_phys);
extern void talloc_free(void *h, void *p);

#endif // _GMEM_TALLOC_H
