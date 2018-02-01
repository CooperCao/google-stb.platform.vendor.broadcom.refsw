/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#define EGL_EGLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/egl/egl_client_config.h"
#include "interface/khronos/egl/egl_int_impl.h"
#include "middleware/khronos/common/khrn_mem.h"
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

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();

   {
      EGL_SERVER_STATE_T *state = egl_get_process_state(thread, dpy, EGL_TRUE);

      if (!state)
         result = 0;
      else {
         EGL_SURFACE_T *surface = egl_get_surface(thread, state, surf);

         if (surface) {
            bool preserve_pixels = false;
            uint32_t lock_usage_hint = EGL_READ_SURFACE_BIT_KHR | EGL_WRITE_SURFACE_BIT_KHR;   /* we completely ignore this */

            assert(!surface->is_locked);

            if (!lock_surface_check_attribs(attrib_list, &preserve_pixels, &lock_usage_hint)) {
               thread->error = EGL_BAD_ATTRIBUTE;
               result = EGL_FALSE;
            } else if (!egl_config_is_lockable(egl_config_to_id(surface->config))) {
               /* Only lockable surfaces can be locked (obviously) */
               thread->error = EGL_BAD_ACCESS;
               result = EGL_FALSE;
            } else if (khrn_mem_get_ref_count(surface) > 1) {
               /* Cannot lock a surface if it is bound to a context */
               thread->error = EGL_BAD_ACCESS;
               result = EGL_FALSE;
            } else if (preserve_pixels) {
               /* TODO: we don't need to support this. What error should we return? */
               thread->error = EGL_BAD_ATTRIBUTE;
               result = EGL_FALSE;
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
   EGLBoolean result = EGL_FALSE;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();

   EGL_SERVER_STATE_T *state = egl_get_process_state(thread, dpy, EGL_TRUE);

   if (!state)
      goto end;

   EGL_SURFACE_T *surface = egl_get_locked_surface(thread, state, surf);

   if (surface == NULL)
      goto end;

   if (!surface->is_locked) {
      thread->error = EGL_BAD_ACCESS;
      result = EGL_FALSE;
   } else {
      assert(surface->is_locked);
      if (surface->mapped_buffer) {
         KHRN_IMAGE_FORMAT_T format = egl_config_get_mapped_format(egl_config_to_id(surface->config));
         uint32_t stride = khrn_image_get_stride(format, surface->width);

         egl_set_color_data(
            surface,
            format,
            surface->width,
            surface->height,
            stride,
            0,
            (const char *)surface->mapped_buffer);

         free(surface->mapped_buffer);
      }

      surface->mapped_buffer = 0;
      surface->is_locked = false;
      thread->error = EGL_SUCCESS;
      result = EGL_TRUE;
   }

end:
   CLIENT_UNLOCK();
   return result;
}
