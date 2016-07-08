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

#endif
