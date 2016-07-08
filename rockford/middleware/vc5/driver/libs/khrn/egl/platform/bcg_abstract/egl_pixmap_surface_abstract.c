/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include "vcos.h"
#include "../../egl_display.h"
#include "../../egl_surface.h"
#include "../../egl_surface_base.h"
#include "../../egl_thread.h"
#include "../../egl_context_gl.h"

#include "libs/platform/bcm_sched_api.h"
#include "../../../common/khrn_process.h"
#include "../../../common/khrn_fmem.h"

#include "libs/platform/gmem.h"
#include "libs/platform/v3d_scheduler.h"

#include "egl_platform_abstract.h"
#include "egl_surface_common_abstract.h"

/* Our own representation of an egl_pixmap_surface */
struct egl_pixmap_surface
{
   EGL_SURFACE_T   base;
   KHRN_IMAGE_T   *image;
   void           *native_surface;

   /* Attributes to be used by VG */
   EGLint          vg_alpha_format;
   EGLint          vg_colorspace;
   EGLint          gl_colorspace;
};

static EGL_SURFACE_METHODS_T fns;

static void get_dimensions(EGL_SURFACE_T *surface, unsigned *width, unsigned *height)
{
   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;
   BEGL_SurfaceInfo       surfInfo;

   if (platform->SurfaceGetInfo)
   {
      EGL_PIXMAP_SURFACE_T *surf = (EGL_PIXMAP_SURFACE_T *)surface;

      platform->SurfaceGetInfo(platform->context, surf->native_surface, &surfInfo);

      *width  = surfInfo.width;
      *height = surfInfo.height;
   }
   else
   {
      *width = 0;
      *height = 0;
   }
}

/* Get the buffer to draw to */
static KHRN_IMAGE_T *get_back_buffer(const EGL_SURFACE_T *surface)
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

static bool get_attrib(const EGL_SURFACE_T *surface, EGLint attrib,
      EGLAttribKHR *value)
{
   const EGL_PIXMAP_SURFACE_T *surf = (EGL_PIXMAP_SURFACE_T *) surface;

   switch (attrib)
   {
   case EGL_VG_ALPHA_FORMAT:
      *value = surf->vg_alpha_format;
      return true;

   case EGL_VG_COLORSPACE:
      *value = surf->vg_colorspace;
      return true;

   case EGL_GL_COLORSPACE_KHR:
      *value = surf->gl_colorspace;
      return true;

   default:
      break;
   }

   return egl_surface_base_get_attrib(surface, attrib, value);
}

static EGLint set_attrib(EGL_SURFACE_T *surface, EGLint attrib,
      EGLAttribKHR value)
{
   EGL_PIXMAP_SURFACE_T *surf = (EGL_PIXMAP_SURFACE_T *) surface;

   switch (attrib)
   {
   case EGL_VG_ALPHA_FORMAT:
      if (value != EGL_VG_ALPHA_FORMAT_NONPRE && value != EGL_VG_ALPHA_FORMAT_PRE)
         return EGL_BAD_PARAMETER;

      surf->vg_alpha_format = (EGLint)value;
      return EGL_SUCCESS;

   case EGL_VG_COLORSPACE:
      if (value != EGL_VG_COLORSPACE_sRGB && value != EGL_VG_COLORSPACE_LINEAR)
         return EGL_BAD_PARAMETER;

      surf->vg_colorspace = (EGLint)value;
      return EGL_SUCCESS;

   case EGL_GL_COLORSPACE_KHR:
      if (value != EGL_GL_COLORSPACE_SRGB_KHR && value != EGL_GL_COLORSPACE_LINEAR_KHR)
         return EGL_BAD_PARAMETER;

      surf->gl_colorspace = (EGLint)value;
      return EGL_SUCCESS;

   default:
      break;
   }

   return egl_surface_base_set_attrib(surface, attrib, value);
}

static EGLSurface egl_create_pixmap_surface_impl(
      EGLDisplay dpy, EGLConfig config,
      NativePixmapType pixmap, const void *attrib_list,
      EGL_AttribType attrib_type)
{
   EGLint                error = EGL_BAD_ALLOC;
   EGLSurface            ret   = EGL_NO_SURFACE;
   EGL_PIXMAP_SURFACE_T *surface;
   unsigned int          width, height;
   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;
   BEGL_SurfaceInfo      surfaceInfo;
   GFX_LFMT_T            gfx_format;

   if (!egl_initialized(dpy, true))
      return EGL_NO_SURFACE;

   surface = calloc(1, sizeof(*surface));
   if (!surface)
      goto end; /* BAD ALLOC */

   /* Even though the config will be checked for validity in egl_surface_base_init
    * we check it here first so that dEQP doesn't fail us. We'll return a different
    * error if we leave it until egl_surface_base_init is called. */
   if (!egl_config_is_valid(config))
   {
      error = EGL_BAD_CONFIG;
      goto end;
   }

   /* Validate the pixmap */
   if (!platform->SurfaceGetInfo || platform->SurfaceGetInfo(platform->context, pixmap, &surfaceInfo) != BEGL_Success)
   {
      error = EGL_BAD_NATIVE_PIXMAP;
      goto end;
   }

   surface->base.fns        = &fns;
   surface->base.type       = EGL_SURFACE_TYPE_PIXMAP;
   surface->native_surface  = (void *)pixmap;
   surface->image = image_from_surface_abstract(pixmap, true);

   if (!surface->image)
      goto end;   /* BAD ALLOC */

   /* pixmaps need to be renderable */
   gfx_format = khrn_image_get_lfmt(surface->image, 0);
   if (!egl_can_render_format(gfx_format))
   {
      error = EGL_BAD_NATIVE_PIXMAP;
      goto end;
   }

   /* default settings */
   surface->vg_alpha_format = EGL_VG_ALPHA_FORMAT_NONPRE;
   surface->vg_colorspace   = EGL_VG_COLORSPACE_sRGB;
   surface->gl_colorspace   = EGL_GL_COLORSPACE_LINEAR_KHR;

   /* Determine size of the underlying native pixmap */
   get_dimensions(&surface->base, &width, &height);

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

   egl_surface_invalidate(&surface->base, true);
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
   KHRN_IMAGE_T         *imageDst = NULL;
   GFX_LFMT_T            gfx_format;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   surf = egl_surface_lock(surface);
   if (surf == NULL)
   {
      error = EGL_BAD_SURFACE;
      goto end;
   }

   EGL_CONTEXT_T *context = egl_thread_get_context();
   if (!context || (context->read != surface && context->draw != surface))
   {
      error = EGL_BAD_SURFACE;
      goto end;
   }

   imageDst = image_from_surface_abstract(target, true); /* TODO : Is this right? */
   if (imageDst == NULL)
   {
      error = EGL_BAD_NATIVE_PIXMAP;
      goto end;
   }

   /* pixmaps need to be renderable */
   gfx_format = khrn_image_get_lfmt(imageDst, 0);
   if (!egl_can_render_format(gfx_format))
   {
      error = EGL_BAD_NATIVE_PIXMAP;
      goto end;
   }

   if (egl_surface_copy(surf, imageDst))
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
   get_back_buffer,
   NULL,                /* swap_buffers   */
   NULL,                /* swap_interval  */
   get_dimensions,
   get_attrib,
   set_attrib,
   delete_fn
};
