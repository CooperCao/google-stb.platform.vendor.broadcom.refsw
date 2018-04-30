/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#define EGL_EGLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/egl/egl_int_impl.h"
#include "middleware/khronos/common/khrn_image.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>
#include <EGL/eglext_wayland.h>
#include <GLES/gl.h>

#if defined(ANDROID)
#include <system/window.h>
#include "gralloc_priv.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#endif

#include "middleware/khronos/egl/egl_server.h"

#ifdef KHRONOS_CLIENT_LOGGING
#include <stdio.h>
extern FILE *xxx_vclog;
#endif

EGLAPI EGLImageKHR EGLAPIENTRY eglCreateImageKHR (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attr_list)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLImageKHR result = EGL_NO_IMAGE_KHR;

   if (!egl_ensure_init_once())
      return EGL_NO_IMAGE_KHR;

   CLIENT_LOCK();

   EGL_SERVER_STATE_T *state = egl_get_process_state(thread, dpy, EGL_TRUE);

   if (state) {
      EGL_CONTEXT_T *context;
      bool ctx_error;
      if (target == EGL_NATIVE_PIXMAP_KHR || target == EGL_NATIVE_BUFFER_ANDROID || target == EGL_WAYLAND_BUFFER_WL) {
         context = NULL;
         ctx_error = ctx != EGL_NO_CONTEXT;
      } else {
         context = egl_get_context(thread, state, ctx);
         ctx_error = (context == NULL);
      }
      if (ctx_error)
         thread->error = EGL_BAD_PARAMETER;
      else {
         EGLint texture_level = 0;
         bool attr_error = false;
         if (attr_list) {
            while (!attr_error && *attr_list != EGL_NONE) {
               switch (*attr_list++) {
               case EGL_GL_TEXTURE_LEVEL_KHR:
                  texture_level = *attr_list++;
                  break;
               case EGL_IMAGE_PRESERVED_KHR:
               {
                  EGLint preserved = *attr_list++;
                  if ((preserved != EGL_FALSE) && (preserved != EGL_TRUE)) {
                     attr_error = true;
                  } /* else: ignore the actual value -- we always preserve */
                  break;
               }
#ifdef WAYLAND
               case EGL_WAYLAND_PLANE_WL:
               {
                  EGLint index = *attr_list++;
                  if (index != 0)
                     attr_error = true;
                  break;
               }
#endif
               default:
                  attr_error = true;
               }
            }
         }
         if (attr_error)
            thread->error = EGL_BAD_PARAMETER;
         else {
            EGLint error;
            result = eglCreateImageKHR_impl(target,
                                    buffer,
                                    texture_level,
                                    &error);
            thread->error = error;
         }
      }
   }

   CLIENT_UNLOCK();

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroyImageKHR (EGLDisplay dpy, EGLImageKHR image)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();

   EGL_SERVER_STATE_T *state = egl_get_process_state(thread, dpy, EGL_TRUE);

   if (!state)
      result = EGL_FALSE;
   else {
      result = eglDestroyImageKHR_impl(image);

      if (!result) {
         thread->error = EGL_BAD_PARAMETER;
      }
   }

   CLIENT_UNLOCK();

   return result;
}
