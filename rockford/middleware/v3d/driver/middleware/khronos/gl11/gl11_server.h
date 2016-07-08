/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 1.1 server-side state structure declaration.
=============================================================================*/

#ifndef GL11_SERVER_H
#define GL11_SERVER_H

#include "interface/khronos/glxx/gl11_int_config.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "interface/khronos/include/GLES/glext.h"

extern bool gl11_server_state_init(GLXX_SERVER_STATE_T *state, uint32_t name, uint64_t pid, MEM_HANDLE_T shared);

/*
   gl11_lock_server_state()

   Returns locked pointer to current GL 1.1 server state.

   Implementation notes:

   Ensures EGL server state has a locked pointer to the current GLES 1.1 server
   state, then returns it.

   TODO make sure this gets reviewed

   Preconditions:

   Valid EGL server state exists
   EGL server state has a current OpenGL ES 1.1 context
   Is being called from a function which _always_ subsequently calls gl11_unlock_server_state()

   Postconditions:

   Return value is a valid pointer
*/

static INLINE GLXX_SERVER_STATE_T *gl11_lock_server_state(void)
{
   EGL_SERVER_STATE_T *egl_state = EGL_GET_SERVER_STATE();
   vcos_assert(egl_state->glversion == EGL_SERVER_GL11);
   if (!egl_state->locked_glcontext) {
      egl_state->locked_glcontext = mem_lock(egl_state->glcontext, NULL);
   }
   return (GLXX_SERVER_STATE_T *)egl_state->locked_glcontext;
}

/*
   gl11_unlock_server_state()

   Releases locked pointer returned by gl11_lock_server_state. This may not
   actually result in the server state being unlocked.

   Implementation notes:

   No-op. Server state is kept locked until gl11_force_unlock_server_state is
   called.

   TODO make sure this gets reviewed

   Preconditions:

   Is being called from a function which has _always_ previously called gl11_lock_server_state()

   Postconditions:
*/

static INLINE void gl11_unlock_server_state(void)
{
#ifndef NDEBUG
   EGL_SERVER_STATE_T *egl_state = EGL_GET_SERVER_STATE();
   vcos_assert(egl_state->glversion == EGL_SERVER_GL11);
   vcos_assert(egl_state->locked_glcontext);
#endif
}

static INLINE void gl11_force_unlock_server_state(void)
{
   EGL_SERVER_STATE_T *egl_state = EGL_GET_SERVER_STATE();
   if ((egl_state->glversion == EGL_SERVER_GL11) && egl_state->locked_glcontext) {
      mem_unlock(egl_state->glcontext);
      egl_state->locked_glcontext = NULL;
   }
}

#define GL11_LOCK_SERVER_STATE()         gl11_lock_server_state()
#define GL11_UNLOCK_SERVER_STATE()       gl11_unlock_server_state()
#define GL11_FORCE_UNLOCK_SERVER_STATE() gl11_force_unlock_server_state()

/*
   Prototypes for server-side implementation functions.

   These are in general very similar to the OpenGL ES 1.1 client
   side functions.
*/
#include "interface/khronos/glxx/gl11_int_impl.h"

#endif
