/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Client-side implementation of the EGLImage extensions for EGL:
   EGL_KHR_image
   EGL_KHR_vg_parent_image
   EGL_KHR_gl_texture_2D_image
   EGL_KHR_gl_texture_cubemap_image
=============================================================================*/

#define EGL_EGLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_client_mangle.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/egl/egl_int_impl.h"
#include "middleware/khronos/common/khrn_image.h"

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/include/EGL/eglext_brcm.h"
#include "interface/khronos/include/GLES/gl.h"

#if defined(ANDROID)
#include <system/window.h>
#include "gralloc_priv.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#endif

#ifdef KHRONOS_CLIENT_LOGGING
#include <stdio.h>
extern FILE *xxx_vclog;
#endif

EGLAPI EGLImageKHR EGLAPIENTRY eglCreateImageKHR (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attr_list)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLImageKHR result = EGL_NO_IMAGE_KHR;

   CLIENT_LOCK();

   KHRONOS_CLIENT_LOG("eglCreateImageKHR !!!?\n");

   {
      CLIENT_PROCESS_STATE_T *process = client_egl_get_process_state(thread, dpy, EGL_TRUE);

      if (process) {
         EGL_CONTEXT_T *context;
         bool ctx_error;
         if (target == EGL_NATIVE_PIXMAP_KHR
#ifdef EGL_BRCM_image_wrap_bcg
            || target == EGL_IMAGE_WRAP_BRCM_BCG
#endif
#ifdef ANDROID
            || target == EGL_NATIVE_BUFFER_ANDROID
#endif
            ) {
            context = NULL;
            ctx_error = ctx != EGL_NO_CONTEXT;
         } else {
            context = client_egl_get_context(thread, process, ctx);
            ctx_error = !context;
         }
         if (ctx_error)
            thread->error = EGL_BAD_PARAMETER;
         else {
            KHRN_IMAGE_FORMAT_T buffer_format = IMAGE_FORMAT_INVALID;
            bool buf_error = false;
#if EGL_BRCM_image_wrap_bcg
            if (target == EGL_IMAGE_WRAP_BRCM_BCG) {
               EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *wrap_buffer = (EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *)buffer;
               switch (wrap_buffer->format) {
               case BEGL_BufferFormat_eA8B8G8R8_TFormat:
               case BEGL_BufferFormat_eX8B8G8R8_TFormat:
               case BEGL_BufferFormat_eR5G6B5_TFormat:
               case BEGL_BufferFormat_eR5G5B5A1_TFormat:
               case BEGL_BufferFormat_eR4G4B4A4_TFormat: break;
               default:
                  buf_error = true;
                  break;
               }
#endif
#ifdef ANDROID
            } else if (target == EGL_NATIVE_BUFFER_ANDROID) {
               android_native_buffer_t *android_buffer = (android_native_buffer_t *)buffer;

               if (android_buffer->common.magic != ANDROID_NATIVE_BUFFER_MAGIC)
                  buf_error = true;

               /* not one of the supported color formats */
               switch (((struct private_handle_t *)android_buffer->handle)->oglFormat)
               {
               case BEGL_BufferFormat_eA8B8G8R8:
               case BEGL_BufferFormat_eX8B8G8R8:
               case BEGL_BufferFormat_eR5G6B5:
               case BEGL_BufferFormat_eYV12_Texture: break;
               default:
                  buf_error = true;
                  break;
               }
#endif
            }

            if (buf_error)
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
                     default:
                        attr_error = true;
                     }
                  }
               }
               if (attr_error)
                  thread->error = EGL_BAD_PARAMETER;
               else {
                  EGLint results[2];
                  eglCreateImageKHR_impl(context ? (context->type == OPENGL_ES_20 ? 2 : 1) : 0,
                                          context ? context->servercontext : 0,
                                          target,
                                          buffer,
                                          texture_level,
                                          results);

                  result = (EGLImageKHR)(intptr_t)results[0];
                  thread->error = results[1];
               }
            }
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

   KHRONOS_CLIENT_LOG("eglDestroyImageKHR image=%d.\n", (int)image);

   CLIENT_LOCK();

   {
      CLIENT_PROCESS_STATE_T *process = client_egl_get_process_state(thread, dpy, EGL_TRUE);

      if (!process)
         result = EGL_FALSE;
      else {
         result = eglDestroyImageKHR_impl(image);

         if (!result) {
            thread->error = EGL_BAD_PARAMETER;
         }
      }
   }

   CLIENT_UNLOCK();

   return result;
}
