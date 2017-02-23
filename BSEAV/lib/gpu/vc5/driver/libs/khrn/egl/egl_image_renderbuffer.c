/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "vcos.h"
#include "egl_image_renderbuffer.h"
#include "egl_context_gl.h"
#include "egl_image.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_shared.h"
#include "../glxx/glxx_renderbuffer.h"

EGL_IMAGE_T *egl_image_renderbuffer_new(EGL_CONTEXT_T *context,
      EGLenum target, EGLClientBuffer buffer)
{
   EGLint error = EGL_BAD_ALLOC;
   GLXX_SERVER_STATE_T *state;
   EGL_GL_CONTEXT_T *ctx;
   EGL_IMAGE_T *egl_image = NULL;
   bool locked = false;

   if (context == NULL || context->api != API_OPENGL)
   {
      error = EGL_BAD_MATCH;
      goto end;
   }
   ctx = (EGL_GL_CONTEXT_T *) context;

   if (!egl_context_gl_lock())
   {
      error = EGL_BAD_MATCH;
      goto end;
   }
   locked = true;

   state = egl_context_gl_server_state(ctx);
   if (!state)
      goto end;

   GLXX_RENDERBUFFER_T *rb = ((uintptr_t)buffer <= UINT32_MAX) ?
      glxx_shared_get_renderbuffer(state->shared, (uint32_t)(uintptr_t)buffer, false) :
      NULL;
   if (!rb || rb->ms_mode != GLXX_NO_MS)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   assert(rb->image);

   egl_image = egl_image_create(rb->image);
   if (!egl_image)
      goto end;
   error = EGL_SUCCESS;

end:
   if (locked)
      egl_context_gl_unlock();
   egl_thread_set_error(error);
   return egl_image;
}
