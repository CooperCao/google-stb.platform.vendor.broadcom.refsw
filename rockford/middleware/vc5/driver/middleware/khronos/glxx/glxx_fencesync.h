/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
fence sync structure declaration + API.
=============================================================================*/
#ifndef GLXX_FENCESYNC_H
#define GLXX_FENCESYNC_H

#include "interface/khronos/glxx/gl_public_api.h"
#include "interface/khronos/glxx/glxx_enum_types.h"
#include "middleware/khronos/common/khrn_types.h"
#include "middleware/khronos/common/khrn_fence.h"
#include <stdbool.h>

typedef struct glxx_fencesync
{
   uint32_t name;
   KHRN_FENCE_T *fence;
   char *debug_label;

} GLXX_FENCESYNC_T;

extern GLXX_FENCESYNC_T* glxx_fencesync_create(unsigned name,
      const KHRN_FENCE_T *fence);

/* a gl fencesync object is signaled when the dependencies specified in fence
 * reach state = completed */
#define GLXX_FENCESYNC_SIGNALED_DEPS_STATE V3D_SCHED_DEPS_COMPLETED

/* the caller must hold the gl lock when calling this */
extern bool glxx_fencesync_is_signaled(GLXX_FENCESYNC_T *fsync);
extern void glxx_fencesync_set_signaled(GLXX_FENCESYNC_T *fsync);

#endif
