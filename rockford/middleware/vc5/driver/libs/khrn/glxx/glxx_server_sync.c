/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  server_cync

FILE DESCRIPTION
Implementation of GLES 3.0 sync objects and fences.
=============================================================================*/
#include "gl_public_api.h"
#include "../common/khrn_int_common.h"
#include "glxx_server.h"
#include "glxx_shared.h"
#include "glxx_fencesync.h"
#include "libs/platform/v3d_platform.h"
#include "libs/util/profile/profile.h"

GLsync GL_APIENTRY glFenceSync (GLenum condition, GLbitfield flags)
{
   GLXX_SERVER_STATE_T  *state = glxx_lock_server_state(OPENGL_ES_3X);
   GLsync id = 0;
   GLenum error = GL_NO_ERROR;

   if (!state)
      return 0;

   if (condition != GL_SYNC_GPU_COMMANDS_COMPLETE)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }
   if (flags != 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   id = glxx_shared_create_fencesync(state->shared, state->fences.fence);
   if (!id)
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
   return id;
}

void GL_APIENTRY glDeleteSync (GLsync fsync_id)
{
   GLXX_SERVER_STATE_T  *state;
   bool res;
   GLenum error = GL_NO_ERROR;

   if (fsync_id == 0)
      return;

   state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   res = glxx_shared_delete_fencesync(state->shared, fsync_id);
   if (!res)
   {
      /* id not in the map */
      error = GL_INVALID_VALUE;
      goto end;
   }

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

GLboolean GL_APIENTRY glIsSync (GLsync fsync_id)
{
   GLXX_SERVER_STATE_T  *state;
   GLboolean result = GL_FALSE;
   GLXX_FENCESYNC_T *fsync;

   state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return GL_FALSE;

   fsync = glxx_shared_get_fencesync(state->shared, fsync_id);
   if (!fsync)
      goto end;

   result = GL_TRUE;
end:
   glxx_unlock_server_state();
   return result;
}

void GL_APIENTRY glGetSynciv (GLsync fsync_id, GLenum pname, GLsizei bufSize,
      GLsizei *length, GLint *values)
{
   GLXX_SERVER_STATE_T  *state;
   GLXX_FENCESYNC_T *fsync;
   unsigned val_count;
   GLenum error = GL_NO_ERROR;

   state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   fsync = glxx_shared_get_fencesync(state->shared, fsync_id);
   if (!fsync)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (bufSize < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (bufSize == 0)
   {
      if (length)
         *length = 0;
      goto end;
   }

   val_count = 0;
   switch (pname)
   {
      case GL_OBJECT_TYPE:
         values[0] = GL_SYNC_FENCE;
         val_count++;
         break;
      case GL_SYNC_CONDITION:
         values[0] = GL_SYNC_GPU_COMMANDS_COMPLETE;
         val_count++;
         break;
      case GL_SYNC_FLAGS:
         values[0] = 0; /* no flags are currently suypported according to the spec */
         val_count++;
         break;
      case GL_SYNC_STATUS:
         {
            bool signaled;
            signaled = glxx_fencesync_is_signaled(fsync);
            values[0] = signaled ? GL_SIGNALED: GL_UNSIGNALED;
            val_count++;
         }
         break;
      default:
         error = GL_INVALID_ENUM;
         goto end;
   };

   if (length)
      *length = val_count;
end:

   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

static int nanosec_to_trunc_ms(GLuint64 timeout)
{
   GLuint64 timeout_ms = timeout / (1000 * 1000);

   // Round up
   if ((timeout_ms * 1000 * 1000) < timeout)
      ++timeout_ms;

   if (timeout_ms > INT_MAX)
      timeout_ms = INT_MAX ;

   return (int)timeout_ms;
}

/* timeout is in nano seconds */
GLenum GL_APIENTRY glClientWaitSync (GLsync fsync_id, GLbitfield flags,
      GLuint64 timeout)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T  *state;
   GLXX_FENCESYNC_T *fsync;
   GLXX_FENCESYNC_T *fsync_temp = NULL;
   GLenum error = GL_NO_ERROR;
   GLenum result = GL_WAIT_FAILED;
   int platform_fence;
   int timeout_ms;
   enum v3d_fence_status status;

   state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return result;

   if (flags & ~GL_SYNC_FLUSH_COMMANDS_BIT)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   fsync = glxx_shared_get_fencesync(state->shared, fsync_id);
   if (!fsync)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (glxx_fencesync_is_signaled(fsync))
   {
      result = GL_ALREADY_SIGNALED;
      goto end;
   }

   timeout_ms = nanosec_to_trunc_ms(timeout);

   if (timeout_ms == 0)
   {
      result = GL_TIMEOUT_EXPIRED;
      goto end;
   }

   platform_fence = khrn_fence_get_platform_fence(fsync->fence,
         GLXX_FENCESYNC_SIGNALED_DEPS_STATE, /*force_create=*/false);

   /* get a reference to the object in case it gets deleted while we do not
    * hold the lock */
   khrn_mem_acquire(fsync);
   fsync_temp = fsync;

   glxx_unlock_server_state();
   status = v3d_platform_fence_wait_timeout(platform_fence, timeout_ms);
   state = glxx_lock_server_state(OPENGL_ES_3X);

   /* for the moment, close the fence; in the future, it might be useful to
    * keep the platform fence in glxx_fencesync and just close it when the last
    * ref_count of that object get released */
   v3d_platform_fence_close(platform_fence);

   if (status == V3D_FENCE_TIMEOUT)
   {
      result = GL_TIMEOUT_EXPIRED;
      goto end;
   }

   assert(status == V3D_FENCE_SIGNALED);
   glxx_fencesync_set_signaled(fsync_temp);
   result = GL_CONDITION_SATISFIED;

end:
   if (fsync_temp)
      khrn_mem_release(fsync_temp);

   if (error != GL_NO_ERROR)
   {
      glxx_server_state_set_error(state, error);
      result = GL_WAIT_FAILED;
   }
   glxx_unlock_server_state();
   return result;
}

void GL_APIENTRY glWaitSync (GLsync fsync_id, GLbitfield flags, GLuint64 timeout)
{
   GLXX_SERVER_STATE_T *state;
   GLXX_FENCESYNC_T *fsync;
   GLenum error = GL_NO_ERROR;

   state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
       return;

   if (flags != 0 || timeout != GL_TIMEOUT_IGNORED)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   fsync = glxx_shared_get_fencesync(state->shared, fsync_id);
   if (!fsync)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }
   if (glxx_fencesync_is_signaled(fsync))
   {
      /* nothing to do */
      goto end;
   }

   if (!glxx_server_state_add_fence_to_depend_on(state, fsync->fence))
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}
