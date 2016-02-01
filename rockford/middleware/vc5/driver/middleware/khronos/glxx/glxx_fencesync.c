/*=============================================================================
Copyright (c) 20014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  async queries

FILE DESCRIPTION
fence sync implementation.
=============================================================================*/
#include "middleware/khronos/glxx/glxx_fencesync.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "helpers/v3d/v3d_gen.h"
#include "middleware/khronos/common/khrn_fence.h"
#include "v3d_platform/v3d_platform.h"

static bool fencesync_init(GLXX_FENCESYNC_T* fsync, unsigned name,
      const KHRN_FENCE_T *fence)
{
   memset(fsync, 0, sizeof(GLXX_FENCESYNC_T));
   fsync->name = name;
   fsync->fence = khrn_fence_dup(fence);
   fsync->debug_label = NULL;
   if (!fsync->fence)
      return false;
   return true;
}

static void fencesync_term(void *v, size_t size)
{
   GLXX_FENCESYNC_T *fsync = (GLXX_FENCESYNC_T *)v;

   free(fsync->debug_label);
   fsync->debug_label = NULL;

   UNUSED(size);
   khrn_fence_refdec(fsync->fence);
}

GLXX_FENCESYNC_T* glxx_fencesync_create(unsigned name,
      const KHRN_FENCE_T *fence)
{
   GLXX_FENCESYNC_T *fsync = KHRN_MEM_ALLOC_STRUCT(GLXX_FENCESYNC_T);

   if (!fsync)
      return NULL;

   if (!fencesync_init(fsync, name, fence))
   {
      KHRN_MEM_ASSIGN(fsync, NULL);
      return NULL;
   }

   khrn_mem_set_term(fsync, fencesync_term);
   return fsync;
}

bool glxx_fencesync_is_signaled(GLXX_FENCESYNC_T *fsync)
{
   return khrn_fence_reached_state(fsync->fence, GLXX_FENCESYNC_SIGNALED_DEPS_STATE);
}

void glxx_fencesync_set_signaled(GLXX_FENCESYNC_T *fsync)
{
   khrn_fence_set_known_state(fsync->fence, V3D_SCHED_DEPS_COMPLETED);
}
