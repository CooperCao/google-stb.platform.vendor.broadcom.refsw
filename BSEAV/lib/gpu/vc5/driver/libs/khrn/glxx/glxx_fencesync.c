/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_fencesync.h"
#include "../common/khrn_mem.h"
#include "libs/core/v3d/v3d_gen.h"
#include "../common/khrn_fence.h"
#include "libs/platform/v3d_platform.h"

static bool fencesync_init(GLXX_FENCESYNC_T* fsync, unsigned name,
      const khrn_fence *fence)
{
   memset(fsync, 0, sizeof(GLXX_FENCESYNC_T));
   fsync->name = name;
   fsync->fence = khrn_fence_dup(fence);
   fsync->debug_label = NULL;
   if (!fsync->fence)
      return false;
   return true;
}

static void fencesync_term(void *v)
{
   GLXX_FENCESYNC_T *fsync = (GLXX_FENCESYNC_T *)v;

   free(fsync->debug_label);
   fsync->debug_label = NULL;

   khrn_fence_refdec(fsync->fence);
}

GLXX_FENCESYNC_T* glxx_fencesync_create(unsigned name,
      const khrn_fence *fence)
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
