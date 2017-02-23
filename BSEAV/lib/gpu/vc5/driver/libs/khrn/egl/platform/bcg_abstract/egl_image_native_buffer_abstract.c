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
#include "../../egl_context_gl.h"
#include <EGL/eglext.h>
#include "../../../common/khrn_image.h"
#include "../../../common/khrn_interlock.h"
#include "sched_abstract.h"

#include "egl_surface_common_abstract.h"

typedef struct native_egl_image
{
   EGL_IMAGE_T base;          /* must be the 1st element to allow type casting */
   void *buffer;              /* some platforms may require acquire/release */
} NATIVE_EGL_IMAGE_T;

static KHRN_IMAGE_T *get_native_egl_image(EGL_IMAGE_T *p)
{
   NATIVE_EGL_IMAGE_T *egl_image = (NATIVE_EGL_IMAGE_T *)p;
   KHRN_IMAGE_T *image = image_from_surface_abstract(egl_image->buffer, false);
   if (!image)
      return NULL;

   /* check for resize/format change.  If the base offset has moved, then replace the image and
      interlocks are invalid */
   if (khrn_image_get_offset(egl_image->base.image, 0) != khrn_image_get_offset(image, 0))
      KHRN_MEM_ASSIGN(egl_image->base.image, image);

   KHRN_MEM_ASSIGN(image, NULL);

   return egl_image->base.image;
}

static void destroy_native_egl_image(EGL_IMAGE_T *p)
{
   NATIVE_EGL_IMAGE_T *egl_image = (NATIVE_EGL_IMAGE_T *)p;
   KHRN_MEM_ASSIGN(egl_image->base.image, NULL);
   free(egl_image);
}

/*
 * Create a new EGL Image type specific to your platform.
 *
 * Returns a valid platform-specific EGL_IMAGE_T * on success. Note that
 * ctx may be NULL, as not all image require contexts. If you do require
 * one, and ctx is NULL, then you should call egl_thread_set_error with
 * EGL_BAD_CONTEXT.
 */
EGL_IMAGE_T *egl_image_native_buffer_abstract_new(EGL_CONTEXT_T *context,
      EGLenum egl_target, EGLClientBuffer egl_buffer, const void *attrib_list,
      EGL_AttribType attrib_type)
{
   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;
   NATIVE_EGL_IMAGE_T *egl_image = NULL;
   EGLenum error = EGL_BAD_ALLOC;
   vcos_unused(context);

   vcos_unused(attrib_list);
   vcos_unused(attrib_type);

   void *buffer;
   if (!platform->GetNativeSurface || platform->GetNativeSurface(
         platform->context, egl_target, egl_buffer, &buffer) != BEGL_Success)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   egl_image = calloc(1, sizeof(*egl_image));
   if (!egl_image)
      goto end;

   KHRN_IMAGE_T *image = image_from_surface_abstract(buffer, false);
   if (!image)
      goto end;
   egl_image->buffer = buffer;

   egl_image_init(&egl_image->base, image, destroy_native_egl_image, get_native_egl_image);
   KHRN_MEM_ASSIGN(image, NULL);

   error = EGL_SUCCESS;
end:
   egl_thread_set_error(error);
   return (EGL_IMAGE_T*)egl_image;
}
