/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION

Interface to the V3D Platform. The Platform consists of a scheduler and a hw
platform (or simluator).
=============================================================================*/

#ifndef V3D_PLATFORM_H
#define V3D_PLATFORM_H
#include "vcos.h"
#include <stdbool.h>
#include "libs/core/v3d/v3d_common.h"
#include "libs/core/v3d/v3d_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialize the v3d platform, including the scheduler session
 * (we only use one per process). Return true for success.
 */
extern bool v3d_platform_init(void);
extern void v3d_platform_shutdown(void);

/* Behaves like a fence that has already been signaled, so eg
 * v3d_platform_fence_wait(V3D_PLATFORM_NULL_FENCE) will return immediately.
 * v3d_platform_fence_close(V3D_PLATFORM_NULL_FENCE) is a no-op. */
#define V3D_PLATFORM_NULL_FENCE (-1)

/* Create a fence */
int v3d_platform_fence_create(void);

/* Signal a fence */
void v3d_platform_fence_signal(int fence);

/*Waits on a fence to signal completion */
void v3d_platform_fence_wait(int fence);

enum v3d_fence_status
{
   V3D_FENCE_SIGNALED,
   V3D_FENCE_TIMEOUT
};

/* Waits up to timeout milliseconds for the fence to signal.
 * Returns V3D_FENCE_SIGNALED if fence was signaled;
 * Returns V3D_FENCE_TIMEOUT if the timeout expired;
 */
enum v3d_fence_status v3d_platform_fence_wait_timeout(int fence,
      int timeout);

void v3d_platform_fence_close(int fence);

extern bool v3d_platform_explicit_sync(void);

/* Debug functions. These may be implemented as no-ops! */
extern void v3d_platform_set_debug_callback(v3d_debug_callback_t callback, void *p);
extern void v3d_platform_set_fragment_shader_debug(bool enabled);
extern bool v3d_platform_get_fragment_shader_debug(void);
extern void v3d_platform_set_vertex_shader_debug(bool enabled);
extern bool v3d_platform_get_vertex_shader_debug(void);

#ifdef __cplusplus
}
#endif

#endif /* V3D_PLATFORM_H */
