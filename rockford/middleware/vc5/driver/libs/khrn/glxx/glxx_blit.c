/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "gl_public_api.h"
#include "glxx_tlb_blit.h"
#include "glxx_tmu_blit.h"
#include "glxx_framebuffer.h"
#include "glxx_server.h"

#include "libs/core/lfmt/lfmt.h"

/* TODO: This int/uint checking is wrong */
static bool is_int_or_uint(GFX_LFMT_T lfmt) {
   return gfx_lfmt_contains_int(lfmt);
}

// Validate source image is not in destination images
// Clamp max src and dst size to used color image sizes
// Check for invalid color format conversion
// Drop color from mask if color is missing in both src and dest
static GLenum validate_blit_color(GLXX_FRAMEBUFFER_T *src_fb,
                                  GLXX_FRAMEBUFFER_T *dst_fb,
                                  bool *color_is_int)
{
   bool src_ms;
   KHRN_IMAGE_T *src_img = NULL, *dst_img = NULL;
   GLenum error = GL_NO_ERROR;

   if (!glxx_fb_acquire_read_image(src_fb, GLXX_PREFER_DOWNSAMPLED, &src_img,
            &src_ms))
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }
   /* we've dropped absent buffers and also we've checked that fbo's are completed */
   assert(src_img);

   *color_is_int = is_int_or_uint(src_img->api_fmt);

   glxx_att_index_t att_index;
   unsigned i = 0;
   while (glxx_fb_iterate_valid_draw_bufs(dst_fb, &i, &att_index))
   {
      const GLXX_ATTACHMENT_T *dst_att =&dst_fb->attachment[att_index];
      if (!glxx_attachment_acquire_image(dst_att, GLXX_DOWNSAMPLED, &dst_img, NULL))
      {
         error = GL_OUT_OF_MEMORY;
         goto end;
      }
      /* we've checked that fbo is complete, so we must have an image */
      assert(dst_img);

      if (khrn_image_equal(dst_img, src_img))
      {
         error = GL_INVALID_OPERATION;
         goto end;
      }

      if (*color_is_int != is_int_or_uint(dst_img->api_fmt))
      {
         error = GL_INVALID_OPERATION;
         goto end;
      }

      if (src_ms && (src_img->api_fmt != dst_img->api_fmt))
      {
         error = GL_INVALID_OPERATION;
         goto end;
      }
      KHRN_MEM_ASSIGN(dst_img, NULL);
   }

end:
   KHRN_MEM_ASSIGN(src_img, NULL);
   KHRN_MEM_ASSIGN(dst_img, NULL);
   return error;
}

// Depth and stencil buffers: Validate they are not the same, but that formats match
static GLenum validate_aux_buffer(const GLXX_ATTACHMENT_T *src_att,
                                  const GLXX_ATTACHMENT_T *dst_att)
{
   KHRN_IMAGE_T *src_img = NULL, *dst_img = NULL;
   GLenum error = GL_NO_ERROR;

   if (!glxx_attachment_acquire_image(src_att, GLXX_PREFER_DOWNSAMPLED, &src_img,
            NULL))
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }
   if (!glxx_attachment_acquire_image(dst_att, GLXX_DOWNSAMPLED, &dst_img, NULL))
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }
   /* we've dropped absent buffers */
   assert(src_img && dst_img);
   if (khrn_image_equal(src_img, dst_img))
   {
      error =  GL_INVALID_OPERATION;
      goto end;
   }

   if (src_img->api_fmt != dst_img->api_fmt)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

end:
   KHRN_MEM_ASSIGN(src_img, NULL);
   KHRN_MEM_ASSIGN(dst_img, NULL);
   return error;
}

static bool no_color_buffers(const GLXX_FRAMEBUFFER_T *fb)
{
   glxx_att_index_t att_index;
   unsigned i = 0;
   if (!glxx_fb_iterate_valid_draw_bufs(fb, &i, &att_index))
      return true;
   return false;
}

static GLbitfield drop_absent_buffers(GLbitfield mask,
                                      GLXX_FRAMEBUFFER_T *src,
                                      GLXX_FRAMEBUFFER_T *dst)
{
   const GLXX_ATTACHMENT_T *src_att;

   src_att = glxx_fb_get_read_buffer(src);
   if (!src_att || src_att->obj_type == GL_NONE || no_color_buffers(dst))
      mask &= ~GL_COLOR_BUFFER_BIT;

   if (src->attachment[GLXX_DEPTH_ATT].obj_type == GL_NONE ||
       dst->attachment[GLXX_DEPTH_ATT].obj_type == GL_NONE   )
      mask &= ~GL_DEPTH_BUFFER_BIT;

   if (src->attachment[GLXX_STENCIL_ATT].obj_type == GL_NONE ||
       dst->attachment[GLXX_STENCIL_ATT].obj_type == GL_NONE)
      mask &= ~GL_STENCIL_BUFFER_BIT;

   return mask;
}

static GLenum validate_blit(
   GLXX_SERVER_STATE_T *state,
   GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
   GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
   GLbitfield mask, GLenum filter)
{
   GLXX_FRAMEBUFFER_T *src_fb = state->bound_read_framebuffer;
   GLXX_FRAMEBUFFER_T *dst_fb = state->bound_draw_framebuffer;
   GLenum              error;

   if (!glxx_fb_is_complete(src_fb) || !glxx_fb_is_complete(dst_fb))
      return GL_INVALID_FRAMEBUFFER_OPERATION;

   bool ms_src = glxx_fb_get_ms_mode(src_fb) != GLXX_NO_MS;
   bool ms_dst = glxx_fb_get_ms_mode(dst_fb) != GLXX_NO_MS;

   if (ms_src)
   {
      if (srcX0 != dstX0 || srcX1 != dstX1 || srcY0 != dstY0 || srcY1 != dstY1)
         return GL_INVALID_OPERATION;
   }

   if (ms_dst)
      return GL_INVALID_OPERATION;

   if (mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
      return GL_INVALID_VALUE;

   if (filter != GL_NEAREST && filter != GL_LINEAR)
      return GL_INVALID_ENUM;

   /* XXX Should we be silently ignoring bits before this error, or after? */
   if (filter == GL_LINEAR && mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
      return GL_INVALID_OPERATION;

   mask = drop_absent_buffers(mask, src_fb, dst_fb);

   if (mask & GL_COLOR_BUFFER_BIT)
   {
      bool color_is_int;
      error = validate_blit_color(src_fb, dst_fb, &color_is_int);
      if (error != GL_NO_ERROR)
         return error;

      if (color_is_int && filter == GL_LINEAR)
         return GL_INVALID_OPERATION;
   }

   if (mask & GL_DEPTH_BUFFER_BIT)
   {
      error = validate_aux_buffer(&src_fb->attachment[GLXX_DEPTH_ATT],
            &dst_fb->attachment[GLXX_DEPTH_ATT]);
      if (error != GL_NO_ERROR)
         return error;
   }

   if (mask & GL_STENCIL_BUFFER_BIT)
   {
      error = validate_aux_buffer(&src_fb->attachment[GLXX_STENCIL_ATT],
            &dst_fb->attachment[GLXX_STENCIL_ATT]);
      if (error != GL_NO_ERROR)
         return error;
   }


   return GL_NO_ERROR;
}

GL_API void GL_APIENTRY glBlitFramebuffer(
      GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
      GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
      GLbitfield mask, GLenum filter)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state)
      return;

   GLenum error = validate_blit( state, srcX0, srcY0, srcX1, srcY1,
                                 dstX0, dstY0, dstX1, dstY1,
                                 mask, filter);
   if(error != GL_NO_ERROR) {
      glxx_server_state_set_error(state, error);
      goto out;
   }

   mask = drop_absent_buffers(mask, state->bound_read_framebuffer, state->bound_draw_framebuffer);
   if (mask == 0) goto out;

   if (srcX0 == srcX1 || srcY0 == srcY1 ||
       dstX0 == dstX1 || dstY0 == dstY1   )
      goto out;

   if (!glxx_try_tlb_blit_framebuffer(state,
      srcX0, srcY0, srcX1, srcY1,
      dstX0, dstY0, dstX1, dstY1,
      mask))
   {
      glxx_tmu_blit_framebuffer(srcX0, srcY0, srcX1, srcY1,
                                dstX0, dstY0, dstX1, dstY1,
                                mask, filter);
   }

out:
   glxx_unlock_server_state();
}
