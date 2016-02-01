/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Client-side implementation of EGL_KHR_lock_surface extension
=============================================================================*/

#define EGL_EGLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_client_mangle.h"

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/egl/egl_client_config.h"
#include "interface/khronos/egl/egl_int_impl.h"
#include <assert.h>

static bool lock_surface_check_attribs(const EGLint *attrib_list, bool *preserve_pixels, uint32_t *lock_usage_hint)
{
   if (!attrib_list)
      return EGL_TRUE;

   while (1) {
      int name = *attrib_list++;
      if (name == EGL_NONE)
         return EGL_TRUE;
      else {
         int value = *attrib_list++;
         switch (name) {
         case EGL_MAP_PRESERVE_PIXELS_KHR:
            *preserve_pixels = value ? true : false;
            break;
         case EGL_LOCK_USAGE_HINT_KHR:
            if (value & ~(EGL_READ_SURFACE_BIT_KHR | EGL_WRITE_SURFACE_BIT_KHR))
               return EGL_FALSE;

            *lock_usage_hint = value;
            break;
         default:
            return EGL_FALSE;
         }
      }
   }
}

EGLAPI EGLBoolean EGLAPIENTRY eglLockSurfaceKHR (EGLDisplay dpy, EGLSurface surf, const EGLint *attrib_list)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;

   CLIENT_LOCK();

   {
      CLIENT_PROCESS_STATE_T *process = client_egl_get_process_state(thread, dpy, EGL_TRUE);

      if (!process)
         result = 0;
      else {
         EGL_SURFACE_T *surface = client_egl_get_surface(thread, process, surf);

         if (surface) {
            bool preserve_pixels = false;
            uint32_t lock_usage_hint = EGL_READ_SURFACE_BIT_KHR | EGL_WRITE_SURFACE_BIT_KHR;   /* we completely ignore this */

            assert(!surface->is_locked);

            if (!lock_surface_check_attribs(attrib_list, &preserve_pixels, &lock_usage_hint)) {
               thread->error = EGL_BAD_ATTRIBUTE;
               result = EGL_FALSE;
            } else if (!egl_config_is_lockable((int)(intptr_t)surface->config - 1)) {
               /* Only lockable surfaces can be locked (obviously) */
               thread->error = EGL_BAD_ACCESS;
               result = EGL_FALSE;
            } else if (surface->context_binding_count) {
               /* Cannot lock a surface if it is bound to a context */
               thread->error = EGL_BAD_ACCESS;
               result = EGL_FALSE;
            } else if (preserve_pixels) {
               /* TODO: we don't need to support this. What error should we return? */
               thread->error = EGL_BAD_ATTRIBUTE;
               return EGL_FALSE;
            } else {
               /* Don't allocate the buffer here. This happens during "mapping", in eglQuerySurface. */
               surface->mapped_buffer = 0;
               surface->is_locked = true;
               thread->error = EGL_SUCCESS;
               result = EGL_TRUE;
            }
         } else
            result = EGL_FALSE;
      }
   }

   CLIENT_UNLOCK();
   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglUnlockSurfaceKHR (EGLDisplay dpy, EGLSurface surf)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;

   CLIENT_LOCK();

   {
      CLIENT_PROCESS_STATE_T *process = client_egl_get_process_state(thread, dpy, EGL_TRUE);

      if (!process)
         result = 0;
      else {
         EGL_SURFACE_T *surface = client_egl_get_locked_surface(thread, process, surf);

         if (!surface) {
            result = EGL_FALSE;
         } else if (!surface->is_locked) {
            thread->error = EGL_BAD_ACCESS;
            result = EGL_FALSE;
         } else {
            assert(surface->is_locked);
            if (surface->mapped_buffer) {
               KHRN_IMAGE_FORMAT_T format = egl_config_get_mapped_format((int)(intptr_t)surface->config - 1);
               uint32_t stride = khrn_image_get_stride(format, surface->width);
               int lines, offset, height;

               lines = KHDISPATCH_WORKSPACE_SIZE / stride;
               offset = 0;
               height = surface->height;

               while (height > 0) {
                  int batch = _min(lines, height);

                  eglIntSetColorData_impl(
                     surface->serverbuffer,
                     format,
                     surface->width,
                     batch,
                     stride,
                     offset,
                     (const char *)surface->mapped_buffer + offset * stride);

                  offset += batch;
                  height -= batch;
               }

               khrn_platform_free(surface->mapped_buffer);
            }

            surface->mapped_buffer = 0;
            surface->is_locked = false;
            thread->error = EGL_SUCCESS;
            result = EGL_TRUE;
         }
      }
   }

   CLIENT_UNLOCK();
   return result;
}
