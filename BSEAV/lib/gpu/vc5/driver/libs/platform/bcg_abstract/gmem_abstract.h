/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __GMEM_ABSTRACT_H__
#define __GMEM_ABSTRACT_H__

#include "../gmem.h"
#include <EGL/begl_memplatform.h>

extern bool gmem_init(void);
extern void gmem_destroy(void);
extern void gmem_mutex_lock_internal(void);
extern void gmem_mutex_unlock_internal(void);
extern uint64_t gmem_get_pagetable_physical_addr(void);
extern v3d_addr_t gmem_get_mmu_max_virtual_addr(void);
extern int64_t gmem_get_mmu_unsecure_bin_translation(void);
extern int64_t gmem_get_mmu_secure_bin_translation(void);
extern uint64_t gmem_get_platform_token(void);

#endif
