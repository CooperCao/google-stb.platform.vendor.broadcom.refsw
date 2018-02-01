/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
   GLenum error = GL_NO_ERROR;

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
   {
      /* If the context is lost, we're not going to change it */
      state = glxx_lock_server_state_unchanged_even_if_reset(OPENGL_ES_3X);
      if (!state)
         return;

      /* Error should have been set by the first glxx_lock_server_state */
      assert(state->error == GL_CONTEXT_LOST);
      if (pname == GL_SYNC_STATUS && bufSize > 0)
         values[0] = GL_SIGNALED;

      goto end;
   }

   GLXX_FENCESYNC_T *fsync = glxx_shared_get_fencesync(state->shared, fsync_id);
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

   unsigned val_count = 0;
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
   GLenum error = GL_NO_ERROR;
   GLenum result = GL_WAIT_FAILED;

   GLXX_SERVER_STATE_T* state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return result;

   if (flags & ~GL_SYNC_FLUSH_COMMANDS_BIT)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   GLXX_FENCESYNC_T* fsync = glxx_shared_get_fencesync(state->shared, fsync_id);
   if (!fsync)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   // this function flushes all render states that are users of this fence
   if (glxx_fencesync_is_signaled(fsync))
   {
      result = GL_ALREADY_SIGNALED;
      goto end;
   }

   if (timeout == 0)
   {
      result = GL_TIMEOUT_EXPIRED;
      goto end;
   }

   /* get a reference of the fence so we can wait for it without a gl lock */
   khrn_fence *fence_temp = fsync->fence;
   khrn_fence_refinc(fence_temp);
   glxx_unlock_server_state();

   int timeout_ms = nanosec_to_trunc_ms(timeout);
   bool res = khrn_fence_wait_timeout(fence_temp, GLXX_FENCESYNC_SIGNALED_DEPS_STATE,
         timeout_ms);
   result = res ? GL_CONDITION_SATISFIED : GL_TIMEOUT_EXPIRED;
   khrn_fence_refdec(fence_temp);
   return result;

end:
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
