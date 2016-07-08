/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION

This is just to give a rough idea of how to do an Android-specific EGLImage
source sibling. In VC4 we had various different EGLImage "sources" (target
param to eglCreateImageKHR) for YUV buffers, video, buffers, etc, although
possibly more than we needed. We will probably want something similar, so
probably a few Android-specific EGL_IMAGE_T subclasses.
=============================================================================*/

#include "vcos.h"
#include "../../egl_image.h"
#include "../../egl_thread.h"
#include "../../egl_attrib_list.h"
#include <EGL/eglext.h>
#include "../../../common/khrn_image.h"

#include "egl_surface_common_abstract.h"

/*
 * Create a new EGL Image type specific to your platform.
 *
 * Returns a valid platform-specific EGL_IMAGE_T * on success. Note that
 * ctx may be NULL, as not all image require contexts. If you do require
 * one, and ctx is NULL, then you should call egl_thread_set_error with
 * EGL_BAD_CONTEXT.
 */
EGL_IMAGE_T *egl_image_native_buffer_abstract_new(EGL_CONTEXT_T *context,
      EGLenum target, EGLClientBuffer buffer, const void *attrib_list,
      EGL_AttribType attrib_type)
{
   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;
   EGL_IMAGE_T *egl_image = NULL;
   EGLenum error = EGL_BAD_ALLOC;
   vcos_unused(context);

   UNUSED(attrib_list);
   UNUSED(attrib_type);

   if (!platform->SurfaceVerifyImageTarget
         || platform->SurfaceVerifyImageTarget(platform->context, buffer, target) != BEGL_Success)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   KHRN_IMAGE_T *image = image_from_surface_abstract(buffer, false);
   if (!image)
      goto end;

   egl_image = egl_image_create(image);
   KHRN_MEM_ASSIGN(image, NULL);
   if (!egl_image)
      goto end;
   error = EGL_SUCCESS;
end:
   egl_thread_set_error(error);
   return egl_image;
}
