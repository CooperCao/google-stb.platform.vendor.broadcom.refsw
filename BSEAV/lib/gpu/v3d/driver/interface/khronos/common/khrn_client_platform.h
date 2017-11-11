/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_image.h"
#include "vcfw/rtos/abstract/rtos_abstract_mem.h"
#include <stdlib.h> // for size_t
#include <stdint.h>

/* Per-platform types are defined in here. Most platforms can be supported
 * via vcos, but 'direct' has its own header and types, which is why
 * the indirection is required.
 */
#include "interface/khronos/common/abstract/khrn_client_platform_filler_abstract.h"

/*
   workaround for broken platforms which don't detect threads exiting
*/
extern void platform_hint_thread_finished(void);

#define PLATFORM_WIN_NONE     UINTPTR_MAX

static INLINE uintptr_t platform_get_handle(EGLNativeWindowType win)
{
   return (uintptr_t)win;
}

extern bool platform_match_pixmap_api_support(EGLNativePixmapType pixmap, uint32_t api_support);

/* Platform optimised versions of memcpy and memcmp */
extern uint32_t platform_memcmp(const void * aLeft, const void * aRight, size_t aLen);
extern void platform_memcpy(void * aTrg, const void * aSrc, size_t aLength);
