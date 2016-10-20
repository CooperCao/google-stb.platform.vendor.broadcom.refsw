/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include "vcos.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "egl_surface_base.h"
#include "egl_config.h"
#include "egl_display.h"

/*
 * Set attributes that apply to all kinds of surfaces and validate some
 * others. The attrib_list is also passed into the "derived" init function
 * which is where the more specific attributes will actually be handled.
 *
 * Returns an EGL error code.
 */
static EGLint set_attributes(EGL_SURFACE_T *surface, const void *attrib_list,
      EGL_AttribType attrib_type)
{
   if (!attrib_list)
      return EGL_SUCCESS;

   EGLint name;
   EGLAttribKHR value;
   while (egl_next_attrib(&attrib_list, attrib_type, &name, &value))
   {
      EGLint ret = egl_surface_set_attrib(surface, name, value);
      if (ret != EGL_SUCCESS) return ret;
   }

   return EGL_SUCCESS;
}

EGLint egl_surface_base_init(EGL_SURFACE_T *surface,
      const EGL_SURFACE_METHODS_T *fns,
      EGL_CONFIG_T *config, const void *attrib_list,
      EGL_AttribType attrib_type,
      unsigned width, unsigned height,
      EGLNativeWindowType win, EGLNativePixmapType pix)
{
   EGLint status;

   if (!egl_config_is_valid(config))
      return EGL_BAD_CONFIG;

   if (win != NULL && egl_any_surfaces_using_native_win(win))
      return EGL_BAD_ALLOC;

   if (pix != NULL && egl_any_surfaces_using_native_pixmap(pix))
      return EGL_BAD_ALLOC;

   /* check config has required WINDOW, PIXMAP or PBUFFER bit set */
   EGLint valid_types = egl_config_get_attrib(config, EGL_SURFACE_TYPE, NULL);
   EGLint surface_type;
   if (win != NULL)
      surface_type = EGL_WINDOW_BIT;
   else if (pix != NULL)
      surface_type = EGL_PIXMAP_BIT;
   else
      surface_type = EGL_PBUFFER_BIT;

   if (!(valid_types & surface_type))
      return EGL_BAD_MATCH;

   surface->fns = fns;
   surface->config = config;
   surface->width = width;
   surface->height = height;

   surface->swap_behavior = EGL_BUFFER_DESTROYED;
   surface->multisample_resolve = EGL_MULTISAMPLE_RESOLVE_DEFAULT;

   surface->native_window = win;
   surface->native_pixmap = pix;

   status = set_attributes(surface, attrib_list, attrib_type);
   if (status != EGL_SUCCESS) return status;

   if (surface->width > EGL_CONFIG_MAX_WIDTH
         || surface->height > EGL_CONFIG_MAX_HEIGHT)
   {
      return EGL_BAD_NATIVE_WINDOW;
   }

   if (!egl_surface_base_init_aux_bufs(surface))
      return EGL_BAD_ALLOC;

   if (vcos_mutex_create(&surface->lock, "EGL Surface") != VCOS_SUCCESS)
      return EGL_BAD_ALLOC;

   return EGL_SUCCESS;
}

void egl_surface_base_destroy(EGL_SURFACE_T *surface)
{
   egl_surface_base_delete_aux_bufs(surface);
   vcos_mutex_delete(&surface->lock);
}

bool egl_surface_base_get_attrib(const EGL_SURFACE_T *surface,
      EGLint attrib, EGLAttribKHR *value)
{
   switch (attrib)
   {
   case EGL_GL_COLORSPACE_KHR:
      if (surface->colorspace == SRGB)
         *value = EGL_GL_COLORSPACE_SRGB_KHR;
      else
         *value = EGL_GL_COLORSPACE_LINEAR_KHR;
      return true;

   case EGL_VG_ALPHA_FORMAT:
      if (surface->alpha_format == NONPRE)
         *value = EGL_VG_ALPHA_FORMAT_NONPRE;
      else
         *value = EGL_VG_ALPHA_FORMAT_PRE;
      return true;

   case EGL_VG_COLORSPACE:
      if (surface->colorspace == SRGB)
         *value = EGL_VG_COLORSPACE_sRGB;
      else
         *value = EGL_VG_COLORSPACE_LINEAR;
      return true;

   case EGL_CONFIG_ID:
      *value = egl_config_get_attrib(surface->config, EGL_CONFIG_ID, NULL);
      return true;

   case EGL_HORIZONTAL_RESOLUTION:
   case EGL_VERTICAL_RESOLUTION:
      *value = EGL_UNKNOWN;
      return true;

   case EGL_PIXEL_ASPECT_RATIO:
      *value = EGL_DISPLAY_SCALING;
      return true;

   case EGL_RENDER_BUFFER:
      if (surface == NULL)
         *value = EGL_NONE;
      else if (surface->type == EGL_SURFACE_TYPE_PIXMAP)
         *value = EGL_SINGLE_BUFFER;
      else
         *value = EGL_BACK_BUFFER;
      return true;

   case EGL_SWAP_BEHAVIOR:
      *value = surface->swap_behavior;
      return true;

   case EGL_MULTISAMPLE_RESOLVE:
      *value = surface->multisample_resolve;
      return true;

   case EGL_WIDTH:
      *value = surface->width;
      return true;

   case EGL_HEIGHT:
      *value = surface->height;
      return true;

   default:
      break;
   }
   return false;
}

EGLint egl_surface_base_set_attrib(EGL_SURFACE_T *surface,
      EGLint attrib, EGLAttribKHR value)
{
   switch (attrib)
   {
   case EGL_SWAP_BEHAVIOR:
      switch (value)
      {
      case EGL_BUFFER_PRESERVED:
         if (!(egl_config_get_attrib(surface->config, EGL_SURFACE_TYPE, NULL)
             & EGL_SWAP_BEHAVIOR_PRESERVED_BIT))
            return EGL_BAD_MATCH;
      case EGL_BUFFER_DESTROYED:
         break;
      default:
         return EGL_BAD_PARAMETER;
      }

      surface->swap_behavior = (EGLenum)value;
      return EGL_SUCCESS;

   case EGL_MULTISAMPLE_RESOLVE:
      switch (value)
      {
      case EGL_MULTISAMPLE_RESOLVE_BOX:
         if (!(egl_config_get_attrib(surface->config, EGL_SURFACE_TYPE, NULL)
            & EGL_MULTISAMPLE_RESOLVE_BOX_BIT))
            return EGL_BAD_MATCH;
      case EGL_MULTISAMPLE_RESOLVE_DEFAULT:
         break;
      default:
         return EGL_BAD_PARAMETER;
      }

      surface->multisample_resolve = (EGLenum)value;
      return EGL_SUCCESS;

   case EGL_GL_COLORSPACE_KHR:
      switch (value)
      {
      case EGL_GL_COLORSPACE_SRGB_KHR:
         surface->colorspace = SRGB;
         return EGL_SUCCESS;
      case EGL_GL_COLORSPACE_LINEAR_KHR:
         surface->colorspace = LINEAR;
         return EGL_SUCCESS;
      default:
         return EGL_BAD_PARAMETER;
      }
      break;

   case EGL_VG_COLORSPACE:
      switch (value)
      {
      case EGL_VG_COLORSPACE_sRGB:
         surface->colorspace = SRGB;
         return EGL_SUCCESS;
      case EGL_VG_COLORSPACE_LINEAR:
         surface->colorspace = LINEAR;
         return EGL_SUCCESS;
      default:
         return EGL_BAD_PARAMETER;
      }
      break;

   case EGL_VG_ALPHA_FORMAT:
      switch (value)
      {
      case EGL_VG_ALPHA_FORMAT_NONPRE:
         surface->alpha_format = NONPRE;
         return EGL_SUCCESS;
      case EGL_VG_ALPHA_FORMAT_PRE:
         surface->alpha_format = PRE;
         return EGL_SUCCESS;
      default:
         return EGL_BAD_PARAMETER;
      }
      break;

   case EGL_RENDER_BUFFER:
      switch (value)
      {
         /*
          * We only support BACK_BUFFER rendering, but it's ok just to not
          * "respect" any attempt to set this to anything else (EGL 1.4 3.5.1)
          */
      case EGL_BACK_BUFFER:
      case EGL_SINGLE_BUFFER:
         return EGL_SUCCESS;
      default:
         return EGL_BAD_PARAMETER;
      }
      break;

#if EGL_EXT_protected_content
   case EGL_PROTECTED_CONTENT_EXT:
      if (value == EGL_TRUE || value == EGL_FALSE)
      {
         surface->secure = value;
         return EGL_SUCCESS;
      }
      else
         return EGL_BAD_PARAMETER;
      break;
#endif

   default:
      break;
   }

   return EGL_BAD_ATTRIBUTE;
}

bool egl_surface_base_init_aux_bufs(EGL_SURFACE_T *surface)
{
   const EGL_CONFIG_T *config = surface->config;
   GFX_LFMT_T color_format = egl_surface_base_colorformat(surface);
   egl_aux_buf_t i;

   // TODO This currently only handles single plane image formats,
   //      formats like D32 S8 would need some extra work.
   struct
   {
      GFX_LFMT_T           api_fmt;
      GFX_LFMT_T           image_fmt;
      gfx_buffer_usage_t   flags;
      unsigned             multiplier;
      unsigned             bpp;
   }
   params[AUX_MAX] =
   {
      /* depth and stencil */
      {
         config->depth_stencil_api_fmt,
         config->depth_stencil_api_fmt,
         GFX_BUFFER_USAGE_V3D_DEPTH_STENCIL,
         1,
         egl_config_get_attrib(config, EGL_DEPTH_SIZE, NULL),
      },

      /* stencil */
      {
         config->stencil_api_fmt,
         config->stencil_api_fmt,
         GFX_BUFFER_USAGE_V3D_DEPTH_STENCIL,
         1,
         egl_config_get_attrib(config, EGL_STENCIL_SIZE, NULL),
      },

      /* mask */
      {
         config->mask_api_fmt,
         config->mask_api_fmt,
         GFX_BUFFER_USAGE_V3D_DEPTH_STENCIL,
         1,
         0,
      },

      /* multisample */
      {
         color_format,
#if V3D_HAS_NEW_TLB_CFG
         color_format,
         GFX_BUFFER_USAGE_V3D_RENDER_TARGET,
#else
         gfx_lfmt_translate_internal_raw_mode(color_format),
         GFX_BUFFER_USAGE_V3D_RENDER_TARGET | GFX_BUFFER_USAGE_V3D_TLB_RAW,
#endif
         2,
         0,
      }
   };

   if (config->samples == 4)
   {
      for (i = AUX_DEPTH; i <= AUX_STENCIL; i++)
      {
         params[i].multiplier *= 2;
#if !V3D_HAS_NEW_TLB_CFG
         params[i].flags |= GFX_BUFFER_USAGE_V3D_TLB_RAW;
#endif
      }
   }
   else
   {
      assert(config->samples == 0);

      for (i = AUX_DEPTH; i <= AUX_STENCIL; i++)
         /* For glBlitFramebuffer */
         params[i].flags |= GFX_BUFFER_USAGE_V3D_TEXTURE | GFX_BUFFER_USAGE_V3D_RENDER_TARGET;

      params[AUX_MULTISAMPLE].api_fmt = GFX_LFMT_NONE;
      params[AUX_MULTISAMPLE].image_fmt = GFX_LFMT_NONE;
   }

   bool secure = !!(surface->secure);

   for (i = 0; i < AUX_MAX; i++)
   {
      KHRN_BLOB_T *blob;
      KHRN_IMAGE_T *image;
      unsigned k;
      GFX_LFMT_T lfmt;

      if (params[i].api_fmt == GFX_LFMT_NONE)
         continue;

      k = params[i].multiplier;

      lfmt = gfx_lfmt_to_2d(params[i].image_fmt);

      blob = khrn_blob_create_no_storage(
         k * surface->width,
         k * surface->height, 1, 1, 1, &lfmt, 1,
         params[i].flags, secure);
      if (!blob) return false;

      image = khrn_image_create(blob, 0, 1, 0, params[i].api_fmt);
      KHRN_MEM_ASSIGN(blob, NULL);
      if (!image) return false;

      surface->aux_bufs[i].image = image;
   }

   if (surface->aux_bufs[AUX_DEPTH].image)
   {
      if (gfx_lfmt_has_stencil(surface->aux_bufs[AUX_DEPTH].image->api_fmt))
      {
         /* if we have depth and stencil, both buffers should point to the same
          * image */
         assert(surface->aux_bufs[AUX_STENCIL].image == NULL);
         KHRN_MEM_ASSIGN(surface->aux_bufs[AUX_STENCIL].image,
               surface->aux_bufs[AUX_DEPTH].image);
      }
   }

   return true;
}

KHRN_IMAGE_T *egl_surface_base_get_aux_buffer(const EGL_SURFACE_T *surface,
      egl_aux_buf_t which)
{
   assert(which < AUX_MAX);
   return surface->aux_bufs[which].image;
}

GFX_LFMT_T egl_surface_base_colorformat(const EGL_SURFACE_T *surface)
{
   GFX_LFMT_T color_format = egl_config_colorformat(surface->config,
      surface->colorspace,
      surface->alpha_format);
   color_format = gfx_lfmt_to_2d(color_format);
   return color_format;
}

void egl_surface_base_delete_aux_bufs(EGL_SURFACE_T *surface)
{
   egl_aux_buf_t i;

   for (i = 0; i < AUX_MAX; i++)
   {
      KHRN_MEM_ASSIGN(surface->aux_bufs[i].image, NULL);
      memset(surface->aux_bufs + i, 0, sizeof (EGL_AUX_BUF_T));
   }
}
