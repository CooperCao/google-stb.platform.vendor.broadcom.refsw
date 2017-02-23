/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 2.0 server-side state structure declaration.
=============================================================================*/

#ifndef GL20_SERVER_H
#define GL20_SERVER_H

#include "middleware/khronos/gl20/gl20_config.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "interface/khronos/include/GLES2/gl2ext.h"

extern bool gl20_server_state_init(GLXX_SERVER_STATE_T *state, uint32_t name, uint64_t pid, MEM_HANDLE_T handle);

static INLINE GLXX_SERVER_STATE_T *gl20_lock_server_state(void)
{
   EGL_SERVER_STATE_T *egl_state = EGL_GET_SERVER_STATE();
   vcos_assert(egl_state->glversion == EGL_SERVER_GL20);
   if (!egl_state->locked_glcontext) {
      egl_state->locked_glcontext = mem_lock(egl_state->glcontext, NULL);
   }
   return (GLXX_SERVER_STATE_T *)egl_state->locked_glcontext;
}

static INLINE void gl20_unlock_server_state(void)
{
#ifndef NDEBUG
   EGL_SERVER_STATE_T *egl_state = EGL_GET_SERVER_STATE();
   vcos_assert(egl_state->glversion == EGL_SERVER_GL20);
   vcos_assert(egl_state->locked_glcontext);
#endif /* NDEBUG */
}

static INLINE void gl20_force_unlock_server_state(void)
{
   EGL_SERVER_STATE_T *egl_state = EGL_GET_SERVER_STATE();
   if ((egl_state->glversion == EGL_SERVER_GL20) && egl_state->locked_glcontext) {
      mem_unlock(egl_state->glcontext);
      egl_state->locked_glcontext = NULL;
   }
}

#define GL20_LOCK_SERVER_STATE()         gl20_lock_server_state()
#define GL20_UNLOCK_SERVER_STATE()       gl20_unlock_server_state()
#define GL20_FORCE_UNLOCK_SERVER_STATE() gl20_force_unlock_server_state()

#include "interface/khronos/glxx/gl20_int_impl.h"

#endif
