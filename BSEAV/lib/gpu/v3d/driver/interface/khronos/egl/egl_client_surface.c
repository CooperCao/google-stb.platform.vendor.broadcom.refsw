/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/egl/egl_client_surface.h"
#include "interface/khronos/egl/egl_client_config.h"
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/egl/egl_int_impl.h"
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/common/khrn_mem.h"

#include <stdlib.h>

EGLint egl_surface_check_attribs(
   EGL_SURFACE_TYPE_T type,
   const EGLint *attrib_list,
   int *width,
   int *height,
   bool *largest_pbuffer,
   EGLenum *texture_format,
   EGLenum *texture_target,
   bool *mipmap_texture,
   bool *secure
   )
{
   if (!attrib_list)
      return EGL_SUCCESS;

   while (*attrib_list != EGL_NONE) {
      int name = *attrib_list++;
      int value; /* read only if we recognise the name */

      switch (name) {

      case EGL_VG_COLORSPACE:
         value = *attrib_list++;
         if (value != EGL_VG_COLORSPACE_sRGB && value != EGL_VG_COLORSPACE_LINEAR)
            return EGL_BAD_ATTRIBUTE;
         break;
      case EGL_VG_ALPHA_FORMAT:
         value = *attrib_list++;
         if (value != EGL_VG_ALPHA_FORMAT_NONPRE && value != EGL_VG_ALPHA_FORMAT_PRE)
            return EGL_BAD_ATTRIBUTE;
         break;

      /* For WINDOW types only */
      case EGL_RENDER_BUFFER:
         value = *attrib_list++;
         if (type != WINDOW || (value != EGL_SINGLE_BUFFER && value != EGL_BACK_BUFFER))
            return EGL_BAD_ATTRIBUTE;
         break;

      /* For PBUFFER types only */
      case EGL_WIDTH:
         value = *attrib_list++;
         if (type != PBUFFER || value < 0)
            return EGL_BAD_PARAMETER;
         if (width != NULL)
            *width = value;
         break;
      case EGL_HEIGHT:
         value = *attrib_list++;
         if (type != PBUFFER || value < 0)
            return EGL_BAD_PARAMETER;
         if (height != NULL)
            *height = value;
         break;
      case EGL_LARGEST_PBUFFER:
         value = *attrib_list++;
         if (type != PBUFFER || (value != EGL_FALSE && value != EGL_TRUE))
            return EGL_BAD_ATTRIBUTE;
         if (largest_pbuffer != NULL)
            *largest_pbuffer = value;
         break;
      case EGL_TEXTURE_FORMAT:
         value = *attrib_list++;
         if (type != PBUFFER || (value != EGL_NO_TEXTURE && value != EGL_TEXTURE_RGB && value != EGL_TEXTURE_RGBA))
            return EGL_BAD_ATTRIBUTE;
         if (texture_format != NULL)
            *texture_format = value;
         break;
      case EGL_TEXTURE_TARGET:
         value = *attrib_list++;
         if (type != PBUFFER || (value != EGL_NO_TEXTURE && value != EGL_TEXTURE_2D))
            return EGL_BAD_ATTRIBUTE;
         if (texture_target != NULL)
            *texture_target = value;
         break;
      case EGL_MIPMAP_TEXTURE:
         value = *attrib_list++;
         if (type != PBUFFER || (value != EGL_FALSE && value != EGL_TRUE))
            return EGL_BAD_ATTRIBUTE;
         if (mipmap_texture != NULL)
            *mipmap_texture = (value == EGL_TRUE);
         break;
#if EGL_EXT_protected_content
      case EGL_PROTECTED_CONTENT_EXT:
         value = *attrib_list++;
         if (value != EGL_FALSE && value != EGL_TRUE)
            return EGL_BAD_ATTRIBUTE;
         if (secure != NULL)
            *secure = (value == EGL_TRUE);
         break;
#endif
      default:
         return EGL_BAD_ATTRIBUTE;
      }
   }

   return EGL_SUCCESS;
}

extern void egl_server_surface_term(void *p);

EGL_SURFACE_T *egl_surface_create(
   EGLSurface name,
   EGL_SURFACE_TYPE_T type,
   bool secure,
   uint32_t buffers,
   uint32_t width,
   uint32_t height,
   EGLConfig config,
   EGLNativeWindowType win,
   uintptr_t serverwin,
   bool largest_pbuffer,
   bool mipmap_texture,
   EGLenum texture_format,
   EGLenum texture_target,
   EGLNativePixmapType pixmap,
   int *result)
{
   assert((type == WINDOW) || (width > 0 && height > 0));
   assert((type == PIXMAP) || (width <= EGL_CONFIG_MAX_WIDTH && height <= EGL_CONFIG_MAX_HEIGHT) || largest_pbuffer);

   EGL_SURFACE_T *surface = KHRN_MEM_ALLOC_STRUCT(EGL_SURFACE_T);
   if (surface == NULL)
      return NULL;

   khrn_mem_set_term(surface, egl_server_surface_term);

   surface->name = name;
   surface->type = type;

   surface->config = config;
   surface->win = win;

   surface->largest_pbuffer = largest_pbuffer;
   surface->mipmap_texture = mipmap_texture;
   surface->mipmap_level = 0;
   surface->texture_format = texture_format;
   surface->texture_target = texture_target;
   surface->pixmap = pixmap;
   surface->swap_behavior = EGL_BUFFER_DESTROYED;
   surface->multisample_resolve = EGL_MULTISAMPLE_RESOLVE_DEFAULT;

   surface->buffers = buffers;

   bool create_surface_result = false;
   if (pixmap) {
      assert(type == PIXMAP);
      create_surface_result = egl_create_wrapped_surface(
         surface,
         pixmap);
   } else {
      create_surface_result = egl_create_surface(
         surface,
         serverwin,
         buffers,
         width,
         height,
         secure,
         mipmap_texture,
         NULL,
         type);
   }

   if (create_surface_result) {
      /* update dimensions */
      uint32_t width, height;
      if (egl_back_buffer_dims(surface, &width, &height)) {
         surface->width = width;
         surface->height = height;
         if (result)
            *result = 0;
      } else {
         /* failed as image is not suitible as render target */
         KHRN_MEM_ASSIGN(surface, NULL);
         if (result)
            *result = -1;
      }
   } else {
      /* Server failed to create a surface due to out-of-memory or
         we failed to create the named semaphore object. */
      KHRN_MEM_ASSIGN(surface, NULL);
      if (result)
         *result = 0;
   }
   return surface;
}

EGLint egl_surface_get_render_buffer(EGL_SURFACE_T *surface)
{
   switch (surface->type) {
   case WINDOW:
      if (surface->buffers == 1)
         return EGL_SINGLE_BUFFER;
      else
         return EGL_BACK_BUFFER;
   case PBUFFER:
      return EGL_BACK_BUFFER;
   case PIXMAP:
      return EGL_SINGLE_BUFFER;
   default:
      UNREACHABLE();
      return EGL_NONE;
   }
}

EGLBoolean egl_surface_get_attrib(EGL_SURFACE_T *surface, EGLint attrib, EGLint *value)
{
   switch (attrib) {
   case EGL_VG_ALPHA_FORMAT:
      *value = EGL_VG_ALPHA_FORMAT_NONPRE;
      return EGL_TRUE;
   case EGL_VG_COLORSPACE:
      *value = EGL_VG_COLORSPACE_sRGB;
      return EGL_TRUE;
   case EGL_CONFIG_ID:
      *value = (EGLint)(intptr_t)surface->config;
      return EGL_TRUE;
   case EGL_HEIGHT:
      if (!egl_back_buffer_dims(surface, NULL, (uint32_t *)value))
         *value = surface->height;
      return EGL_TRUE;
   case EGL_HORIZONTAL_RESOLUTION:
   case EGL_VERTICAL_RESOLUTION:
      *value = EGL_UNKNOWN;
      return EGL_TRUE;
   case EGL_LARGEST_PBUFFER:
      // For a window or pixmap surface, the contents of value are not modified.
      if (surface->type == PBUFFER)
         *value = surface->largest_pbuffer;
      return EGL_TRUE;
   case EGL_MIPMAP_TEXTURE:
      // Querying EGL_MIPMAP_TEXTURE for a non-pbuffer surface is not
      // an error, but value is not modified.
      if (surface->type == PBUFFER)
         *value = surface->mipmap_texture;
      return EGL_TRUE;
   case EGL_MIPMAP_LEVEL:
      // Querying EGL_MIPMAP_LEVEL for a non-pbuffer surface is not
      // an error, but value is not modified.
      if (surface->type == PBUFFER)
         *value = surface->mipmap_level;
      return EGL_TRUE;
   case EGL_PIXEL_ASPECT_RATIO:
      *value = EGL_DISPLAY_SCALING;
      return EGL_TRUE;
   case EGL_RENDER_BUFFER:
      *value = egl_surface_get_render_buffer(surface);
      return EGL_TRUE;
   case EGL_SWAP_BEHAVIOR:
      *value = surface->swap_behavior;
      return EGL_TRUE;
   case EGL_MULTISAMPLE_RESOLVE:
      *value = surface->multisample_resolve;
      return EGL_TRUE;
   case EGL_TEXTURE_FORMAT:
      // Querying EGL_TEXTURE_FORMAT for a non-pbuffer surface is not
      // an error, but value is not modified.
      if (surface->type == PBUFFER)
         *value = surface->texture_format;
      return EGL_TRUE;
   case EGL_TEXTURE_TARGET:
      // Querying EGL_TEXTURE_TARGET for a non-pbuffer surface is not
      // an error, but value is not modified.
      if (surface->type == PBUFFER)
         *value = surface->texture_target;
      return EGL_TRUE;
   case EGL_WIDTH:
      if (!egl_back_buffer_dims(surface, (uint32_t *)value, NULL))
         *value = surface->width;
      return EGL_TRUE;
   default:
      return EGL_FALSE;
   }
}

EGLint egl_surface_set_attrib(EGL_SURFACE_T *surface, EGLint attrib, EGLint value)
{
   switch (attrib) {
   case EGL_MIPMAP_LEVEL:
      /* If the value of pbuffer attribute EGL_TEXTURE_FORMAT is EGL_NO_TEXTURE,
         if the value of attribute EGL_TEXTURE_TARGET is EGL_NO_TEXTURE, or if surface
         is not a pbuffer, then attribute EGL_MIPMAP_LEVEL may be set, but has no effect.*/
      if (surface->type == PBUFFER) {
         if ((surface->texture_format != EGL_NO_TEXTURE) &&
             (surface->texture_target != EGL_NO_TEXTURE))
            egl_select_mipmap(surface, value);
      }
      surface->mipmap_level = value;
      return EGL_SUCCESS;
   case EGL_SWAP_BEHAVIOR:
      switch (value) {
      case EGL_BUFFER_PRESERVED:
      {
         EGLint value = 0;
         egl_config_get_attrib(egl_config_to_id(surface->config), EGL_SURFACE_TYPE, &value);
         if (!(value & EGL_SWAP_BEHAVIOR_PRESERVED_BIT))
            return EGL_BAD_MATCH;
      }
      case EGL_BUFFER_DESTROYED:
         break;
      default:
         return EGL_BAD_PARAMETER;
      }

      surface->swap_behavior = value;
      return EGL_SUCCESS;
   case EGL_MULTISAMPLE_RESOLVE:
      switch (value) {
      case EGL_MULTISAMPLE_RESOLVE_DEFAULT:
      case EGL_MULTISAMPLE_RESOLVE_BOX:
         surface->multisample_resolve = value;
         return EGL_SUCCESS;
      default:
         return EGL_BAD_PARAMETER;
      }
   default:
      return EGL_BAD_ATTRIBUTE;
   }
}
