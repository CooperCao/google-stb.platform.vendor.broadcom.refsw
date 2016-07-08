/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/

#include "glxx_hw.h"
#include "../common/khrn_counters.h"
#include "../common/khrn_render_state.h"

/*!
   * \brief Clears buffers (glClear)
   *
   * Clears buffers. Chooses either drawing a rectangle or hw clear during tile rendering.
   * - Fast clear sets clear bits in CLE store instructions to clear selected buffers
   * - Optionally we can discard fmem, including all control lists
   *    - This can be done if all buffers are fast cleared or invalidated
   *    - Fast clear without discarding fmem must be used if non-cleared buffers have contents
   * - Using fast clear is preferred, but cannot always be used.
   * - DrawRect is used as fallback when fast clear cannot be used
   * - Scissoring affects clears, and if buffers are only partially cleared, fast clears cannot be used
   *
   * If we are doing a clear after BlitFramebuffer(), we need to flush the blit before
   * we clear. TODO CHECK THIS LOGIC
   *
   * \param color   is the clear colour to be used.
   * \param depth   is the clear Z depth.
   * \param stencil is the clear stencil to be used.
   * \param state   is the OpenGL server state.
   */
bool glxx_hw_clear(GLXX_SERVER_STATE_T *state, GLXX_CLEAR_T *clear)
{
   const GLXX_FRAMEBUFFER_T *fb = state->bound_draw_framebuffer;
   GLXX_HW_FRAMEBUFFER_T   hw_fb;
   GLXX_HW_RENDER_STATE_T  *rs = NULL;
   unsigned x, y, xmax, ymax;
   unsigned b;
   int      scx, scy, scw, sch;
   bool     transpose;
   bool     keep_cl        = false;
   bool     partial_clear  = false;
   bool     use_draw_rect  = false;
   bool     any_hw_clears  = false;
   bool     ok             = true;

   // When enabled, RASTERIZER_DISCARD also causes the Clear and ClearBuffer* commands to be ignored
   if (state->caps.rasterizer_discard)
      return true;

   //  1. Based on GL state, some clears can be disabled.
   //     Note that buffer clearing uses slightly different rules from rendering.
   //     "The clear operation always uses the front stencil write mask when clearing the stencil buffer."
   //     "When Clear is called, the only per-fragment operations that are applied (if
   //      enabled) are the pixel ownership test, the scissor test, sRGB conversion
   //      (see section 4.1.8), and dithering. The masking operations described in section 4.2.2 are
   //      also applied. If a buffer is not present, then a Clear directed at that buffer has no
   //      effect.""
   //     - Setting depth func to never   does NOT disable clearing depth     (this would disable normal depth writes)
   //     - Disabling depth test          does NOT disable clearing depth     (this would disable normal depth writes)
   //     - Disabling stencil test        does NOT disable clearing stencil   (this would disable normal stencil writes)
   //     - Setting front stencil mask to 0 DOES disable clearing stencil     (this would NOT disable normal stencil writes, unless back stencil mask was also 0)
   /* If all colours are masked then we're not clearing colour */
   clear->color   &= state->color_write.r || state->color_write.g ||
                     state->color_write.b || state->color_write.a;
   clear->depth   &= state->depth_mask;                               // Writing to depth
   clear->stencil &= state->stencil_mask.front != 0;                  // Stencil writes not fully disabled

   //  This check can be done before install framebuffer.
   if (!clear->color && !clear->depth && !clear->stencil)
      return true;

   //  create hw_framebuffer
   glxx_init_hw_framebuffer(&hw_fb);
   ok = glxx_build_hw_framebuffer(fb, &hw_fb);
   if (!ok)
      return true;  // We have no buffers to render to. See in glxx_install_framebuffer() if it is an error or not.

   // Could be that framebuffer (object) doesn't have depth / stencil..
   if (hw_fb.depth.image == NULL)
      clear->depth = false;

   if (hw_fb.stencil.image == NULL)
      clear->stencil = false;

   // .. so check again
   if (!clear->color && !clear->depth && !clear->stencil)
      goto end;

   //  3. If we are only clearing color - check if any color render targets is selected by draw buffers
   if (!clear->depth && !clear->stencil)
   {
      bool clearing_any_color = false;
      glxx_att_index_t att_index;
      unsigned i = 0;
      while (glxx_fb_iterate_valid_draw_bufs(fb, &i, &att_index))
      {
         unsigned b = att_index - GLXX_COLOR0_ATT;
         if (clear->color_buffer_mask & (1 << b))
         {
            clearing_any_color = true;
            break;
         }
      }
      if (!clearing_any_color)
         goto end;
   }

   //  4. Figure out if we are doing partial clear, and early out if 0 area to clear
   x = 0;
   y = 0;
   xmax = hw_fb.width;
   ymax = hw_fb.height;

   transpose = false;
   if (transpose)
   {
      scy = state->scissor.x;
      scx = state->scissor.y;
      sch = state->scissor.width;
      scw = state->scissor.height;
   }
   else
   {
      scx = state->scissor.x;
      scy = state->scissor.y;
      scw = state->scissor.width;
      sch = state->scissor.height;
   }

   if (state->caps.scissor_test)
   {
      x = MAX(x, scx);
      y = MAX(y, scy);

      xmax = MIN(xmax, scx + scw);
      ymax = MIN(ymax, scy + sch);
      if (scx + scw < 0 || scy + sch < 0 || x >= xmax || y >= ymax)
      {
         goto end;
      }
   }

   if (x != 0 || y != 0 || xmax != hw_fb.width || ymax != hw_fb.height)
      partial_clear = true;

   // Check the color mask - anything other than all-channels-enabled needs a drawRect clear as
   // the hardware clear ignores the color mask
   if (clear->color && (!state->color_write.r || !state->color_write.g || !state->color_write.b || !state->color_write.a))
      partial_clear = true;

   // Using a stencil mask requires a drawRect based clear as the hardware clear will not do it
   if (clear->stencil && ((state->stencil_mask.front & 0xFF) != 0xFF))
      partial_clear = true;

   {
      rs = glxx_install_rs(state, &hw_fb, false);
      if (!rs)
      {
         ok = false;
         goto end;
      }
      khrn_render_state_disallow_flush((KHRN_RENDER_STATE_T*)rs);
      GLXX_HW_FRAMEBUFFER_T * const hw_fb = &rs->installed_fb;

      // 5.  Check if cl needs to be kept
      khrn_fmem_common_persist* fmem_common = khrn_fmem_get_common_persist(&rs->fmem);
      if (fmem_common->query_list != NULL || rs->tf_used || rs->base.has_buffer_writes)
      {
         keep_cl = true;
      }
      else
      {
         for (b = 0; b < hw_fb->rt_count; ++b)
            {
               if (!glxx_bufstate_can_discard_cl(rs->color_buffer_state[b],
                     clear->color && glxx_fb_is_valid_draw_buf(fb, GLXX_COLOR0_ATT + b),
                     partial_clear))
               {
                  keep_cl = true;
                  break;
               }
            if (hw_fb->ms &&
                !glxx_bufstate_can_discard_cl(rs->ms_color_buffer_state[b],
                   clear->color && glxx_fb_is_valid_draw_buf(fb, GLXX_COLOR0_ATT + b),
                   partial_clear))
            {
               keep_cl = true;
               break;
            }
         }
         if (!glxx_bufstate_can_discard_cl(rs->depth_buffer_state, clear->depth, partial_clear))
            keep_cl = true;
         if (!glxx_bufstate_can_discard_cl(rs->stencil_buffer_state, clear->stencil, partial_clear))
            keep_cl = true;
      }


      // Update bufstates for discarding of CL
      if (!keep_cl)
      {
         for (b = 0; b < hw_fb->rt_count; ++b)
         {
            glxx_bufstate_discard_cl(&rs->color_buffer_state[b]);
            glxx_bufstate_discard_cl(&rs->ms_color_buffer_state[b]);
         }
         glxx_bufstate_discard_cl(&rs->depth_buffer_state);
         glxx_bufstate_discard_cl(&rs->stencil_buffer_state);
      }

      // 6. Update buffer states for cleared buffers and do hw clears
      if (clear->color)
      {
         glxx_att_index_t att_index;
         unsigned i = 0;
         while (glxx_fb_iterate_valid_draw_bufs(fb, &i, &att_index))
         {
            unsigned b = att_index - GLXX_COLOR0_ATT;
            if (clear->color_buffer_mask & (1 << b))
            {
               glxx_bufstate_t col_state = rs->color_buffer_state[b];
               glxx_bufstate_t ms_state = rs->ms_color_buffer_state[b];

               if (glxx_bufstate_try_fast_clear(&col_state, partial_clear) &&
                  (!hw_fb->ms || glxx_bufstate_try_fast_clear(&ms_state, partial_clear)))
               {
                  // Keep both ms an non-ms bufstates in sync.
                  rs->color_buffer_state[b] = col_state;
                  rs->ms_color_buffer_state[b] = ms_state;

                  any_hw_clears = true;
                  memcpy(rs->clear_colors[b], clear->color_value, sizeof(rs->clear_colors[0]));
               }
               else
               {
                  use_draw_rect = true;
               }
            }
         }
      }

      if (clear->depth)
      {
         if (glxx_bufstate_try_fast_clear(&rs->depth_buffer_state, partial_clear))
         {
            rs->depth_value = clear->depth_value;
            any_hw_clears = true;
         }
         else
         {
            use_draw_rect = true;
         }
      }

      if (clear->stencil)
      {
         if (glxx_bufstate_try_fast_clear(&rs->stencil_buffer_state, partial_clear))
         {
            rs->stencil_value = (rs->stencil_value & ~state->stencil_mask.front) | (clear->stencil_value & state->stencil_mask.front);
            any_hw_clears = true;
         }
         else
         {
            use_draw_rect = true;
         }
      }

      if (any_hw_clears)
         khrn_driver_incr_counters(KHRN_PERF_HARD_CLEAR);

      // 7. Throw away cl if possible
      if (!keep_cl)
      {
         ok = glxx_hw_render_state_discard_and_restart(state, rs);
         if (!ok)
            goto end; // TODO set out of memory?
      }

      if (use_draw_rect)
      {
         khrn_driver_incr_counters(KHRN_PERF_SOFT_CLEAR);

         // TODO Single pass clear with shaders
         {
            uint32_t mask = clear->color_buffer_mask;
            if (clear->color)
            {
               glxx_att_index_t att_index;
               unsigned i = 0;
               while (glxx_fb_iterate_valid_draw_bufs(fb, &i, &att_index))
               {
                  unsigned b = att_index - GLXX_COLOR0_ATT;
                  if (mask & (1 << b))
                  {
                     clear->color_buffer_mask = b;
                     if (!glxx_draw_rect(state, rs, clear, x, y, xmax, ymax))
                        ok = false;

                     clear->depth = false;
                     clear->stencil = false;
                  }
               }
            }
            else
            {
               assert(clear->depth || clear->stencil);
               clear->color_buffer_mask = 0;
               if (!glxx_draw_rect(state, rs, clear, x, y, xmax, ymax))
                  ok = false;
            }
         }
      }
   }

   if (!ok)
      goto end;

end:
   glxx_destroy_hw_framebuffer(&hw_fb);
   if (rs != NULL)
   {
      khrn_render_state_allow_flush((KHRN_RENDER_STATE_T*)rs);

      if (!ok)
         glxx_hw_discard_frame(rs);
   }

   return ok;
}
