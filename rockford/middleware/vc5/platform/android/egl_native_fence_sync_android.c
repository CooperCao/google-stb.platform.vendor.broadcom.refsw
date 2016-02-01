/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/
#include "vcos.h"
#include "egl_native_fence_sync_android.h"
#include "middleware/khronos/egl/egl_display.h"
#include "middleware/khronos/egl/egl_context_gl.h"
#include "middleware/khronos/egl/egl_sync.h"
#include "middleware/khronos/egl/egl_attrib_list.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "middleware/khronos/glxx/glxx_server.h"
/*
#include "interface/khronos/include/EGL/eglext.h"
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/egl/egl_thread.h"
*/

#define VCOS_LOG_CATEGORY (&android_egl_native_fence_sync_cat)
static VCOS_LOG_CAT_T android_egl_native_fence_sync_cat =
   VCOS_LOG_INIT("egl_android_native_fence_sync", VCOS_LOG_WARN);


static EGLenum parse_native_fence_fd(const void *attrib_list,
      EGL_AttribType attrib_type, int* fd)
{
   EGLenum error = EGL_SUCCESS;
   *fd = EGL_NO_NATIVE_FENCE_FD_ANDROID;

   EGLint name;
   EGLAttribKHR value;
   while (egl_next_attrib(&attrib_list, attrib_type, &name, &value))
   {
      switch (name)
      {
         case EGL_SYNC_NATIVE_FENCE_FD_ANDROID:
            *fd = (int)value;
            break;
         default:
            error = EGL_BAD_ATTRIBUTE;
            break;
      }
   }
   return error;
}

EGL_SYNC_T *egl_native_fence_sync_android_new(EGL_CONTEXT_T *context,
      EGLenum type, const void *attrib_list, EGL_AttribType attrib_type)
{
   EGLenum error = EGL_BAD_ALLOC;
   EGL_SYNC_T *egl_sync = NULL;
   int fd;
   EGL_GL_CONTEXT_T *gl_context;
   GLXX_SERVER_STATE_T *state;

   assert(context);

   if (type != EGL_SYNC_NATIVE_FENCE_ANDROID)
   {
      error = EGL_BAD_ATTRIBUTE;
      goto end;
   }

   error = parse_native_fence_fd(attrib_list, attrib_type, &fd);
   if (error != EGL_SUCCESS)
      goto end;

   assert(context->api == API_OPENGL);
   gl_context = (EGL_GL_CONTEXT_T *) context;

   if (fd == EGL_NO_NATIVE_FENCE_FD_ANDROID)
   {
      if (!egl_context_gl_lock())
         goto end;
      state = egl_context_gl_server_state(gl_context);
      assert(state);
      egl_sync = egl_sync_create(EGL_SYNC_NATIVE_FENCE_ANDROID,
            EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR, state->fences.fence);
      egl_context_gl_unlock();
   }
   else
   {
      egl_sync = egl_sync_create_from_fd(EGL_SYNC_NATIVE_FENCE_ANDROID,
            EGL_SYNC_NATIVE_FENCE_SIGNALED_ANDROID, fd);
   }

   if (!egl_sync)
   {
      error = EGL_BAD_ALLOC;
      goto end;
   }
   error = EGL_SUCCESS;

end:
   egl_thread_set_error(error);
   return egl_sync;
}

EGLAPI EGLint EGLAPIENTRY eglDupNativeFenceFDANDROID(EGLDisplay dpy, EGLSyncKHR sync)
{
   EGL_SYNC_T *egl_sync = NULL;
   EGLint error = EGL_BAD_ALLOC;
   int fd = EGL_NO_NATIVE_FENCE_FD_ANDROID;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   egl_sync = egl_get_sync_refinc(sync);

   if (egl_sync == NULL)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   if (egl_sync->type != EGL_SYNC_NATIVE_FENCE_ANDROID)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   /* we need a gl lock on all the operations involving a khrn_fence because
    * we are tempering with khrn_fmems by calling a flush*/
   if (!egl_context_gl_lock())
      goto end;
   fd = khrn_fence_get_platform_fence(egl_sync->fence,
         GLXX_FENCESYNC_SIGNALED_DEPS_STATE);
   egl_context_gl_unlock();
   error = EGL_SUCCESS;

end:
   egl_sync_refdec(egl_sync);
   egl_thread_set_error(error);
   return fd;
}
