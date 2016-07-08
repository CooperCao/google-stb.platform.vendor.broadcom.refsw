/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/

// See http://confluence.broadcom.com/display/MobileMultimedia/GFX+VC5+BlitFramebuffer

#include "gl_public_api.h"
#include "../common/khrn_int_common.h"

#include "glxx_hw.h"
#include "glxx_server_internal.h"
#include "glxx_tmu_blit.h"
#include "../common/khrn_render_state.h"
#include "../egl/egl_image.h"

#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"

#include "vcos.h"

typedef enum
{
   BLIT_TLB,
   BLIT_TMU // This is in the other blit file: glxx_tmu_blit.c
} blit_backend_t;

static void canonicalise_flip(int *X0, int *X1)
{
   if (*X0 > *X1)
   {
      int tmp = *X0;
      *X0 = *X1;
      *X1 = tmp;
   }
}

// return true if lfmt is compatible with TLB load/store
bool store_general_compatible(GFX_LFMT_T lfmt)
{
   return gfx_lfmt_is_uif(lfmt) || gfx_lfmt_is_uif_xor(lfmt);
}

typedef struct {
   bool       x_y_match;
   int        x0, x1, y0, y1;
} BLIT_INFO_T;

static bool blit_is_full_frame(const BLIT_INFO_T *blit,
                               const GLXX_HW_FRAMEBUFFER_T *src_fb,
                               const GLXX_HW_FRAMEBUFFER_T *dst_fb)
{
   return (blit->x0 == 0) &&
          (blit->y0 == 0) &&
          (blit->x1 == (int)src_fb->width)  && /* Size match with src is important, */
          (blit->y1 == (int)src_fb->height) && /* not sure about dst */
          (blit->x1 == (int)dst_fb->width)  &&
          (blit->y1 == (int)dst_fb->height);
}

static bool same_color_fmt_and_store_general_compatible(const GLXX_FRAMEBUFFER_T *src_fb,
      const GLXX_FRAMEBUFFER_T *dst_fb, const GLXX_HW_FRAMEBUFFER_T *dst_hw_fb)
{
   bool any_draw = false;

   const GLXX_ATTACHMENT_T *src_att = glxx_fb_get_read_buffer(src_fb);
   assert(src_att);
   GFX_LFMT_T src_api_fmt = glxx_attachment_get_api_fmt(src_att);

   glxx_att_index_t att_index;
   unsigned i = 0;
   while (glxx_fb_iterate_valid_draw_bufs(dst_fb, &i, &att_index))
   {
      const GLXX_ATTACHMENT_T *dst_att = &dst_fb->attachment[att_index];
      GFX_LFMT_T dst_api_fmt = glxx_attachment_get_api_fmt(dst_att);
      if (src_api_fmt != dst_api_fmt)
         return false;

      unsigned dst_b = att_index - GLXX_COLOR0_ATT;
      GFX_LFMT_T dst_fmt = khrn_image_plane_lfmt(&dst_hw_fb->color[dst_b]);
      if (!store_general_compatible(dst_fmt))
         return false;

      any_draw = true;
   }
   assert(any_draw);
   vcos_unused_in_release(any_draw);
   return true;
}

// Conditions here must be strict enough so that we can be sure that
// any time later we can use TLB backend if we choose it here.
static blit_backend_t blit_early_backend_selection(BLIT_INFO_T *blit_info,
                                                   GLbitfield mask,
                                                   const GLXX_FRAMEBUFFER_T *src_fb,
                                                   const GLXX_FRAMEBUFFER_T *dst_fb,
                                                   const GLXX_SERVER_STATE_T *state)
{
   /* TODO: It would be really great to not have to build hardware framebuffers here */
   GLXX_HW_FRAMEBUFFER_T src_hw_fb, dst_hw_fb;
   blit_backend_t result = BLIT_TMU;

   glxx_init_hw_framebuffer(&src_hw_fb);
   glxx_init_hw_framebuffer(&dst_hw_fb);

   if (!glxx_build_hw_framebuffer(src_fb, &src_hw_fb)) {
      goto done;
   }

   if (!glxx_build_hw_framebuffer(dst_fb, &dst_hw_fb)) {
      goto done;
   }

   if (!blit_info->x_y_match ||
       !blit_is_full_frame(blit_info, &src_hw_fb, &dst_hw_fb))
      goto done;

   /* Scissored blits cannot use the TLB path */
   if (state->caps.scissor_test)
   {
      if (state->scissor.x != 0 || state->scissor.y != 0 ||
          state->scissor.width != dst_hw_fb.width || state->scissor.height != dst_hw_fb.height)
         goto done;
   }

   if ((mask & GL_COLOR_BUFFER_BIT) &&
       !same_color_fmt_and_store_general_compatible(src_fb, dst_fb, &dst_hw_fb))
      goto done;

   if (mask & GL_STENCIL_BUFFER_BIT)
      goto done;

   if (mask & GL_DEPTH_BUFFER_BIT)
   {
      GFX_LFMT_T dst_lfmt = khrn_image_plane_lfmt(&dst_hw_fb.depth);
      if (!store_general_compatible(dst_lfmt))
         goto done;

      /* Disallow resolving blits of depth buffers. These require store general
       * in raw mode, and these may not work except for F32 targets. See GFXD-36 */
      if (src_hw_fb.ms != dst_hw_fb.ms)
         goto done;
   }

   result = BLIT_TLB;
done:
   glxx_destroy_hw_framebuffer(&src_hw_fb);
   glxx_destroy_hw_framebuffer(&dst_hw_fb);
   return result;
}

static GLXX_HW_RENDER_STATE_T *install_rs_for_tlb_blit(
                                      GLXX_SERVER_STATE_T *state,
                                      const GLXX_FRAMEBUFFER_T *fb)
{
   GLXX_HW_FRAMEBUFFER_T   src_fb;
   GLXX_HW_RENDER_STATE_T *src_rs = NULL;

   glxx_init_hw_framebuffer(&src_fb);
   if (!glxx_build_hw_framebuffer(fb, &src_fb))
      goto end;

   for(;;)
   {
      src_rs = glxx_install_rs(state, &src_fb, true);
      if (src_rs == NULL) break;

      if(src_rs->num_blits == GLXX_RS_MAX_BLITS)
         // Blit slots we already in use. Flush are try again
         glxx_hw_render_state_flush(src_rs);
      else
         break;
   }

end:
   glxx_destroy_hw_framebuffer(&src_fb);
   return src_rs;
}

static bool blit_record_in_fmem(GLXX_BLIT_T *blit, KHRN_FMEM_T *fmem)
{
   GLXX_HW_FRAMEBUFFER_T *dst_fb = &blit->dst_fb;

   // Record the interlocks of the written images in the fmem
   if (blit->color)
   {
      for (unsigned b = 0; b < dst_fb->rt_count; ++b)
      {
         if (blit->color_draw_to_buf[b])
         {
            KHRN_RES_INTERLOCK_T *res_i = khrn_image_get_res_interlock(dst_fb->color[b].image);
            if (!khrn_fmem_record_res_interlock(fmem, res_i, true, ACTION_RENDER))
               return false;
         }
      }
   }

   if (blit->depth)
   {
      KHRN_RES_INTERLOCK_T *res_i = khrn_image_get_res_interlock(dst_fb->depth.image);
      if (!khrn_fmem_record_res_interlock(fmem, res_i, true, ACTION_RENDER))
         return false;
   }

   return true;
}

static bool glxx_tlb_blit_framebuffer(GLXX_SERVER_STATE_T *state,
                                      const GLXX_FRAMEBUFFER_T *src,
                                      const GLXX_FRAMEBUFFER_T *dst,
                                      GLbitfield mask)
{
   GLXX_HW_RENDER_STATE_T *src_rs;
   GLXX_BLIT_T *blit;

   src_rs = install_rs_for_tlb_blit(state, src);
   if (src_rs == NULL)
      return false;

   assert(src_rs->num_blits < GLXX_RS_MAX_BLITS);
   blit = &src_rs->tlb_blits[src_rs->num_blits++];

   glxx_init_hw_framebuffer(&blit->dst_fb);
   if (!glxx_build_hw_framebuffer(dst, &blit->dst_fb)) {
      goto fail;
   }

   blit->color = !!(mask & GL_COLOR_BUFFER_BIT);
   if (blit->color)
   {
      assert(src->read_buffer != GLXX_INVALID_ATT);
      blit->color_read_buffer = src->read_buffer - GLXX_COLOR0_ATT;

      glxx_att_index_t att_index;
      unsigned i = 0;
      while (glxx_fb_iterate_valid_draw_bufs(dst, &i, &att_index))
      {
         unsigned b = att_index - GLXX_COLOR0_ATT;
         blit->color_draw_to_buf[b] = true;
      }
   }

   blit->depth   = !!(mask & GL_DEPTH_BUFFER_BIT);
   blit->stencil = !!(mask & GL_STENCIL_BUFFER_BIT);

   if (!blit_record_in_fmem(blit, &src_rs->fmem))
      goto fail;

   return true;

fail:
   glxx_destroy_hw_framebuffer(&blit->dst_fb);
   src_rs->num_blits--;
   return false;
}

/* TODO: This int/uint checking is wrong */
static bool is_int_or_uint(GFX_LFMT_T lfmt) {
   return gfx_lfmt_contains_int_signed(lfmt) || gfx_lfmt_contains_int_unsigned(lfmt);
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

static void prepare_blit_info(BLIT_INFO_T *blit,
                              GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                              GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1)
{
   blit->x_y_match = (srcX0 == dstX0 && srcX1 == dstX1 &&
                      srcY0 == dstY0 && srcY1 == dstY1   );

   blit->x0 = dstX0;
   blit->y0 = dstY0;
   blit->x1 = dstX1;
   blit->y1 = dstY1;

   // If rectangle has N1 < N0 flip the end-points
   canonicalise_flip(&blit->x0, &blit->x1);
   canonicalise_flip(&blit->y0, &blit->y1);
}

GL_API void GL_APIENTRY glBlitFramebuffer(
      GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
      GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
      GLbitfield mask, GLenum filter)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE_UNCHANGED();
   BLIT_INFO_T blit_info;
   GLenum error;

   if (!state)
      return;

   error = validate_blit( state, srcX0, srcY0, srcX1, srcY1,
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

   prepare_blit_info(&blit_info, srcX0, srcY0, srcX1, srcY1,
                                 dstX0, dstY0, dstX1, dstY1);

   // Choose between TLB and draw rect backend here
   // Just use TMU path for now.
   if (blit_early_backend_selection(&blit_info, mask,
                                   state->bound_read_framebuffer,
                                   state->bound_draw_framebuffer, state) == BLIT_TLB)
   {
      if (!glxx_tlb_blit_framebuffer(state, state->bound_read_framebuffer,
                                            state->bound_draw_framebuffer,
                                            mask))
      {
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
         goto out;
      }
   }
   else // BLIT_TMU
   {
      // Call the TMU backend.
      glxx_tmu_blit_framebuffer(srcX0, srcY0, srcX1, srcY1,
                                dstX0, dstY0, dstX1, dstY1,
                                mask, filter);
   }

out:
   GLXX_UNLOCK_SERVER_STATE();
}
