/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "../../egl_image.h"
#include "../../egl_thread.h"
#include "../../egl_attrib_list.h"
#include "../../egl_context_gl.h"
#include <EGL/eglext.h>
#include "../../../common/khrn_image.h"
#include "../../../common/khrn_resource.h"
#include "sched_abstract.h"

#include "egl_surface_common_abstract.h"

typedef struct native_egl_image
{
   EGL_IMAGE_T base;          /* must be the 1st element to allow type casting */
} NATIVE_EGL_IMAGE_T;

static khrn_image *get_native_egl_image(EGL_IMAGE_T *p)
{
   NATIVE_EGL_IMAGE_T *egl_image = (NATIVE_EGL_IMAGE_T *)p;

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
   NATIVE_EGL_IMAGE_T *egl_image = NULL;
   EGLint error = EGL_BAD_ALLOC;
   unused(context);

   unused(attrib_list);
   unused(attrib_type);

   egl_image = calloc(1, sizeof(*egl_image));
   if (!egl_image)
      goto end;

   unsigned num_mip_levels;
   khrn_image *image = image_from_surface_abstract(egl_target, egl_buffer,
         false, "EGL image", &num_mip_levels, &error);
   if (!image)
   {
      free(egl_image);
      egl_image = NULL;
      goto end;
   }

   egl_image_init(&egl_image->base, image, num_mip_levels, destroy_native_egl_image, get_native_egl_image);
   KHRN_MEM_ASSIGN(image, NULL);

   error = EGL_SUCCESS;
end:
   egl_thread_set_error(error);
   return (EGL_IMAGE_T*)egl_image;
}
