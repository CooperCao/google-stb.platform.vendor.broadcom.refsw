/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "../../egl_display.h"
#include "../../egl_surface.h"
#include "../../egl_surface_base.h"
#include "../../egl_thread.h"
#include "../../egl_context_gl.h"

#include "libs/platform/bcm_sched_api.h"
#include "../../../common/khrn_process.h"
#include "../../../common/khrn_fmem.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"

#include "libs/platform/gmem.h"
#include "libs/platform/v3d_scheduler.h"

#include "egl_platform_abstract.h"
#include "egl_surface_common_abstract.h"

/* Our own representation of an egl_pixmap_surface */
struct egl_pixmap_surface
{
   EGL_SURFACE_T   base;
   khrn_image   *image;
   void           *native_surface;
};

static EGL_SURFACE_METHODS_T fns;

/* Get the buffer to draw to */
static khrn_image *get_back_buffer(EGL_SURFACE_T *surface)
{
   EGL_PIXMAP_SURFACE_T *surf = (EGL_PIXMAP_SURFACE_T *) surface;

   return surf->image;
}

static void delete_fn(EGL_SURFACE_T *surface)
{
   EGL_PIXMAP_SURFACE_T *surf = (EGL_PIXMAP_SURFACE_T *) surface;

   if (!surface)
      return;

   KHRN_MEM_ASSIGN(surf->image, NULL);

   egl_surface_base_destroy(surface);
   free(surface);
}

static EGLSurface egl_create_pixmap_surface_impl(
      EGLDisplay dpy, EGLConfig config_in,
      NativePixmapType pixmap, const void *attrib_list,
      EGL_AttribType attrib_type)
{
   EGLint                error = EGL_BAD_ALLOC;
   EGLSurface            ret   = EGL_NO_SURFACE;
   EGL_PIXMAP_SURFACE_T *surface;
   unsigned int          width, height;
   GFX_LFMT_T            gfx_format;

   if (!egl_initialized(dpy, true))
      return EGL_NO_SURFACE;

   surface = calloc(1, sizeof(*surface));
   if (!surface)
      goto end; /* BAD ALLOC */

   const EGL_CONFIG_T *config = egl_config_validate(config_in);
   if (!config)
   {
      error = EGL_BAD_CONFIG;
      goto end;
   }

   surface->base.fns        = &fns;
   surface->base.type       = EGL_SURFACE_TYPE_PIXMAP;
   surface->native_surface  = (void *)pixmap;
   surface->image = image_from_surface_abstract(BEGL_PIXMAP_BUFFER, pixmap,
         true, "EGL pixmap", /*num_mip_levels=*/NULL, &error);

   if (!surface->image)
      goto end;   /* error set by image_from_surface_abstract() */

   khrn_image_invalidate(surface->image);

   /* pixmaps need to be renderable */
   gfx_format = khrn_image_get_lfmt(surface->image, 0);
   if (!gfx_lfmt_can_render_format(gfx_format))
   {
      error = EGL_BAD_NATIVE_PIXMAP;
      goto end;
   }

   /* Determine size of the underlying native pixmap */
   khrn_image_get_dimensions(surface->image, &width, &height, NULL, NULL);

   if (width == 0 || height == 0)
   {
      error = EGL_BAD_MATCH;
      goto end;
   }

   error = egl_surface_base_init(&surface->base, &fns, config, attrib_list,
         attrib_type, width, height, NULL, pixmap);
   if (error != EGL_SUCCESS)
      goto end;

   ret = egl_map_surface((EGL_SURFACE_T *) surface);
   if (!ret || ret == EGL_NO_SURFACE)
   {
      ret = EGL_NO_SURFACE;
      goto end;
   }

   error = EGL_SUCCESS;

end:
   if (error != EGL_SUCCESS)
   {
      if (ret != EGL_NO_SURFACE)
         egl_unmap_surface(ret);

      KHRN_MEM_ASSIGN(surface->image, NULL);

      if (surface != NULL)
         delete_fn((EGL_SURFACE_T *) surface);

      ret = EGL_NO_SURFACE;
   }

   egl_thread_set_error(error);

   return ret;
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePixmapSurface(
      EGLDisplay dpy, EGLConfig config,
      NativePixmapType pixmap, const EGLint *attrib_list)
{
   return egl_create_pixmap_surface_impl(dpy, config, pixmap, attrib_list,
         attrib_EGLint);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurface (EGLDisplay dpy,
      EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list)
{
   return egl_create_pixmap_surface_impl(dpy, config,
         (NativePixmapType)native_pixmap, attrib_list, attrib_EGLAttrib);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurfaceEXT(EGLDisplay dpy,
      EGLConfig config, void *native_pixmap, const EGLint *attrib_list)
{
   return egl_create_pixmap_surface_impl(dpy, config,
         (NativePixmapType)native_pixmap, attrib_list, attrib_EGLint);
}

EGLAPI EGLBoolean EGLAPIENTRY eglCopyBuffers(EGLDisplay dpy, EGLSurface
      surface, EGLNativePixmapType target)
{
   EGLint                error    = EGL_SUCCESS;
   EGL_SURFACE_T        *surf     = NULL;
   khrn_image         *imageDst = NULL;
   GFX_LFMT_T            gfx_format;
   unsigned              num_mip_levels;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   surf = egl_surface_lock(surface);
   if (surf == NULL)
   {
      error = EGL_BAD_SURFACE;
      goto end;
   }

   EGL_CONTEXT_T *context = egl_thread_get_context();
   if (!context || (context->read != surf && context->draw != surf))
   {
      error = EGL_BAD_SURFACE;
      goto end;
   }

   imageDst = image_from_surface_abstract(BEGL_PIXMAP_BUFFER, target, true,
         "EGL pixmap", &num_mip_levels, &error); /* TODO : Is this right? */
   if (imageDst == NULL)
   {
      goto end;
   }

   if (num_mip_levels != 1)
   {
      error = EGL_BAD_NATIVE_PIXMAP;
      goto end;
   }

   /* pixmaps need to be renderable */
   gfx_format = khrn_image_get_lfmt(imageDst, 0);
   if (!gfx_lfmt_can_render_format(gfx_format))
   {
      error = EGL_BAD_NATIVE_PIXMAP;
      goto end;
   }

   // TODO Invalidate aux buffers here??

   if (!egl_context_convert_image(context, imageDst, egl_surface_get_back_buffer(surf)))
   {
      error = EGL_BAD_MATCH;
      goto end;
   }

end:
   egl_surface_unlock(surf);

   KHRN_MEM_ASSIGN(imageDst, NULL);

   egl_thread_set_error(error);

   return error == EGL_SUCCESS;
}

static EGL_SURFACE_METHODS_T fns =
{
   .get_back_buffer = get_back_buffer,
   .delete_fn = delete_fn,
};
