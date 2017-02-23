/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of OpenGL ES 2.0 / Open GL ES 1.1 OES_framebuffer_object renderbuffer structure.
=============================================================================*/

#include "../common/khrn_int_common.h"
#include "../egl/egl_display.h"
#include "../egl/egl_image.h"
#include "gl_public_api.h"
#include "glxx_int_config.h"
#include "glxx_server.h"
#include "glxx_server_internal.h"
#include "glxx_renderbuffer.h"
#include "glxx_framebuffer.h"
#include "libs/util/dglenum/dglenum.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"

/*

Implementations are required to support the same internal
formats for renderbuffers as the required formats for
textures enumerated in section 3.8.3, with the exception
of the color formats labelled "texture-only". Requesting
one of these internal formats for a renderbuffer will
allocate at least the internal component sizes and exactly
the component types shown for that format in tables 3.12 - 3.13.

Implementations are also required to support STENCIL_INDEX8.
Requesting this internal format for a renderbuffer will
allocate at least 8 stencil bit planes.

Implementations must support creation of renderbuffers in
these required formats with up to the value of MAX_SAMPLES
multisamples, with the exception of signed and unsigned
integer formats.

*/

static void renderbuffer_init(GLXX_RENDERBUFFER_T *renderbuffer, uint32_t name)
{
   assert(renderbuffer);

   renderbuffer->name = name;

   assert(renderbuffer->image == NULL);

   renderbuffer->debug_label = NULL;
}

static void renderbuffer_term(void *v, size_t size)
{
   GLXX_RENDERBUFFER_T *renderbuffer = (GLXX_RENDERBUFFER_T *)v;

   vcos_unused(size);

   KHRN_MEM_ASSIGN(renderbuffer->image, NULL);
   egl_image_refdec(renderbuffer->source);

   free(renderbuffer->debug_label);
   renderbuffer->debug_label = NULL;
}

bool glxx_renderbuffer_storage(GLXX_RENDERBUFFER_T *renderbuffer,
      glxx_ms_mode ms_mode, GLenum internalformat,
      unsigned width_pixels, unsigned height_pixels,
      bool secure)
{
   unsigned       num_planes;
   GFX_LFMT_T     api_fmt;
   GFX_LFMT_T     lfmts[GFX_BUFFER_MAX_PLANES];
   KHRN_IMAGE_T   *oldimage = renderbuffer->image;
   bool           imagechanged;
   unsigned       width_samples;
   unsigned       height_samples;

   api_fmt = gfx_api_fmt_from_sized_internalformat(internalformat);
   glxx_hw_fmts_from_api_fmt(&num_planes, lfmts, api_fmt);
   glxx_lfmt_add_dim(lfmts, num_planes, 2);
#if !V3D_VER_AT_LEAST(4,0,2,0)
   if (ms_mode != GLXX_NO_MS)
   {
      // Multisample color images must be stored using the TLB raw format.
      if (gfx_lfmt_has_color(lfmts[0]))
      {
         assert(num_planes == 1);
         lfmts[0] = gfx_lfmt_translate_internal_raw_mode(lfmts[0]);
      }
   }
#endif

   unsigned scale = glxx_ms_mode_get_scale(ms_mode);
   width_samples = scale * width_pixels;
   height_samples = scale * height_pixels;

   if (!oldimage)
   {
      imagechanged = true;
   }
   else
   {
      GFX_LFMT_T old_lfmts[GFX_BUFFER_MAX_PLANES];
      unsigned old_num_planes;

      khrn_image_get_lfmts(oldimage, old_lfmts, &old_num_planes);

      /* TODO: what about if we're sharing the image? */
      imagechanged =
         !gfx_lfmt_fmts_equal(old_num_planes, old_lfmts, num_planes, lfmts) ||
         (khrn_image_get_width(oldimage) != width_samples) ||
         (khrn_image_get_height(oldimage) != height_samples) ||
         (renderbuffer->ms_mode != ms_mode);
   }

   assert(lfmts[0] != GFX_LFMT_NONE);

   if (imagechanged)
   {
      renderbuffer->width_pixels = width_pixels;
      renderbuffer->height_pixels = height_pixels;

      if (width_samples == 0 || height_samples == 0) {
         KHRN_MEM_ASSIGN(renderbuffer->image, NULL);
         renderbuffer->ms_mode = GLXX_NO_MS;
         return true;
      }

      renderbuffer->ms_mode = ms_mode;

      gfx_buffer_usage_t flags =
         /* Renderbuffers can end up being used as textures via
          * glBlitFramebuffer. This includes multisample renderbuffers! */
         GFX_BUFFER_USAGE_V3D_TEXTURE |
         /* glBlitFramebuffer may also use depth/stencil buffers as render
          * targets */
         GFX_BUFFER_USAGE_V3D_RENDER_TARGET;
      if (!glxx_is_color_renderable_from_api_fmt(api_fmt))
      {
         /* we've already checked that internal format is color or depth or stencil renderable -->
          * if not color then is one of the others */
         flags |= GFX_BUFFER_USAGE_V3D_DEPTH_STENCIL;
      }
#if !V3D_VER_AT_LEAST(4,0,2,0)
      if (ms_mode != GLXX_NO_MS)
         flags |= GFX_BUFFER_USAGE_V3D_TLB_RAW;
#endif

      KHRN_BLOB_T *blob = khrn_blob_create(width_samples, height_samples, 1,
            1, 1, lfmts, num_planes, flags, secure);
      if (!blob)
         return false;

      KHRN_IMAGE_T *new_image = khrn_image_create(blob, 0, 1, 0, api_fmt);
      KHRN_MEM_ASSIGN(blob, NULL);
      if (!new_image)
         return false;

      KHRN_MEM_ASSIGN(renderbuffer->image, new_image);
      khrn_mem_release(new_image);
   }

   return true;
}

static bool valid_image(KHRN_IMAGE_T *image)
{
   GFX_LFMT_T lfmt = khrn_image_get_lfmt(image, 0);

   if ((khrn_image_get_width(image) > GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE)
      || (khrn_image_get_height(image) > GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE))
   {
      return false;
   }

   /* TODO This looks a bit bogus... */
   if (!gfx_lfmt_is_2d(lfmt))
      return false;

   if (!gfx_lfmt_has_color(lfmt) && !gfx_lfmt_has_depth(lfmt) && !gfx_lfmt_has_stencil(lfmt))
      return false;

   switch (gfx_lfmt_get_swizzling(&lfmt))
   {
   case GFX_LFMT_SWIZZLING_UIF:
   case GFX_LFMT_SWIZZLING_UIF_XOR:
   case GFX_LFMT_SWIZZLING_UBLINEAR:
   case GFX_LFMT_SWIZZLING_LT:
      break;
   default:
      return false;
   }
   return true;
}

static bool glxx_renderbuffer_bind_image(GLXX_RENDERBUFFER_T *renderbuffer, KHRN_IMAGE_T *image)
{
   bool result;

   if (renderbuffer->ms_mode != GLXX_NO_MS)
      return false;

   if (valid_image(image))
   {
      KHRN_MEM_ASSIGN(renderbuffer->image, image);
      result = true;
   }
   else
      result = false;

   return result;
}

GLXX_RENDERBUFFER_T *glxx_renderbuffer_create(uint32_t name)
{
   GLXX_RENDERBUFFER_T *rb_obj = KHRN_MEM_ALLOC_STRUCT(GLXX_RENDERBUFFER_T);       // check, glxx_renderbuffer_term

   if (rb_obj != NULL)
   {
      renderbuffer_init(rb_obj, name);
      khrn_mem_set_term(rb_obj, renderbuffer_term);
   }
   return rb_obj;
}

GL_APICALL void GL_APIENTRY glEGLImageTargetRenderbufferStorageOES(GLenum target,
      GLeglImageOES image)
{
   GLXX_SERVER_STATE_T *state;
   EGL_IMAGE_T *egl_image = NULL;
   GLenum error = GL_NO_ERROR;
   KHRN_IMAGE_T *khr_image;
   bool locked = false;

   if (!egl_context_gl_lock())
      goto end;
   locked = true;

   state = egl_context_gl_server_state(NULL);
   if (!state)
      goto end;

   if (state->bound_renderbuffer == NULL)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   if (target != GL_RENDERBUFFER)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   egl_image = egl_get_image_refinc((EGLImageKHR) image);
   if (egl_image == NULL)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   khr_image = egl_image_get_image(egl_image);

   if (!glxx_renderbuffer_bind_image(state->bound_renderbuffer, khr_image))
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }
   /* we've already got e ref count of this egl_image; keep it */
   state->bound_renderbuffer->source = egl_image;

end:
   if (error != GL_NO_ERROR)
   {
      egl_image_refdec(egl_image);
      glxx_server_state_set_error(state, error);
   }

   if (locked)
      egl_context_gl_unlock();
}
