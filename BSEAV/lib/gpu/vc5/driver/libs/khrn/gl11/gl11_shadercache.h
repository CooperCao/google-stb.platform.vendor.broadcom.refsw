/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Handles vertex and fragment shader caches.
=============================================================================*/
#ifndef GL11_SHADERCACHE_4_H
#define GL11_SHADERCACHE_4_H

#include "../common/khrn_interlock.h"
#include "gl11_shader_cache.h"
#include "../common/khrn_fmem.h"
extern void gl11_hw_shader_cache_reset(GL11_CACHE_ENTRY_T *cache);

/* Find the cache entry corresponding to 'key'. Return the entry with
 * blob and backend_cache valid.
 */
extern GL11_CACHE_ENTRY_T *gl11_get_backend_cache(
   GL11_CACHE_ENTRY_T *cache,
   GL11_CACHE_KEY_T *key);

#endif
