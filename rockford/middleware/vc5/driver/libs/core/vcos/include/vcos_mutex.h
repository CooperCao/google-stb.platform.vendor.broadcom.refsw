/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once
#include "vcos_types.h"
#include <stdbool.h>

#if defined(__unix__)
#include "../posix/vcos_mutex_posix.h"
#else
#include "../cpp/vcos_mutex_cpp.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

VCOS_STATUS_T vcos_mutex_create(VCOS_MUTEX_T *mutex, const char *name);
void vcos_mutex_delete(VCOS_MUTEX_T *mutex);
void vcos_mutex_lock(VCOS_MUTEX_T *mutex);
void vcos_mutex_unlock(VCOS_MUTEX_T *mutex);
bool vcos_mutex_trylock(VCOS_MUTEX_T *mutex);
bool vcos_mutex_is_locked(VCOS_MUTEX_T *mutex);

VCOS_STATUS_T vcos_reentrant_mutex_create(VCOS_REENTRANT_MUTEX_T *mutex, const char *name);
void vcos_reentrant_mutex_delete(VCOS_REENTRANT_MUTEX_T *mutex);
void vcos_reentrant_mutex_lock(VCOS_REENTRANT_MUTEX_T *mutex);
void vcos_reentrant_mutex_unlock(VCOS_REENTRANT_MUTEX_T *mutex);

#ifdef __cplusplus
}
#endif
