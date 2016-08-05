/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/

#include "glxx_hw.h"
#include "../common/khrn_counters.h"
#include "../common/khrn_render_state.h"

/* Clears buffers (glClear). Chooses either drawing a rectangle or hw clear
 * during tile rendering.
 * - Fast clear sets clear bits in CLE store instructions to clear selected
 *   buffers
 * - Optionally we can discard fmem, including all control lists
 *    - This can be done if all buffers are fast cleared or invalidated
 *    - Fast clear without discarding fmem must be used if non-cleared buffers
 *      have contents
 * - Using fast clear is preferred, but cannot always be used.
 * - DrawRect is used as fallback when fast clear cannot be used
 * - Scissoring affects clears, and if buffers are only partially cleared, fast
 *   clears may not be able to be used
 */
bool glxx_hw_clear(GLXX_SERVER_STATE_T *state, GLXX_CLEAR_T *clear)
{
   // When enabled, RASTERIZER_DISCARD also causes the Clear and ClearBuffer* commands to be ignored
   if (state->caps.rasterizer_discard)
      return true;

   // Buffer clearing uses slightly different rules from rendering:
   // - Clears always uses the front stencil write mask when clearing the
   //   stencil buffer.
   // - The only per-fragment operations that are applied (if enabled) are the
   //   pixel ownership test, the scissor test, sRGB conversion, and dithering.
   //   Masking operations are also applied.
   // - Setting depth func to never or disabling depth test does NOT disable
   //   clearing depth (this would disable normal depth writes)
   // - Disabling stencil test does NOT disable clearing stencil (this would
   //   disable normal stencil writes)
   // - Setting front stencil mask to 0 DOES disable clearing stencil (this
   //   would NOT disable normal stencil writes, unless back stencil mask was
   //   also 0)

   const GLXX_FRAMEBUFFER_T *fb = state->bound_draw_framebuffer;

   // Disable clears of not-present buffers
   clear->color_buffer_mask &= glxx_fb_get_valid_draw_buf_mask(fb);
   if (fb->attachment[GLXX_DEPTH_ATT].obj_type == GL_NONE)
      clear->depth = false;
   if (fb->attachment[GLXX_STENCIL_ATT].obj_type == GL_NONE)
      clear->stencil = false;

   // Disable clears of buffers if writes to them are entirely masked
   for (unsigned b = 0; b != GLXX_MAX_RENDER_TARGETS; ++b)
      if (!(state->color_write & (0xf << (b * 4))))
         clear->color_buffer_mask &= ~(1u << b);
   if (!state->depth_mask)
      clear->depth = false;
   if (!state->stencil_mask.front)
      clear->stencil = false;

   // Early out if no clears
   if (!clear->color_buffer_mask && !clear->depth && !clear->stencil)
      return true;

   GLXX_HW_FRAMEBUFFER_T hw_fb;
   if (!glxx_init_hw_framebuffer(fb, &hw_fb))
      return false;

   // Intersect FB rect with scissor rect
   int x = 0;
   int y = 0;
   int xmax = hw_fb.width;
   int ymax = hw_fb.height;
   if (state->caps.scissor_test)
   {
      x = gfx_smax(x, state->scissor.x);
      y = gfx_smax(y, state->scissor.y);
      xmax = gfx_smin(xmax, state->scissor.x + state->scissor.width);
      ymax = gfx_smin(ymax, state->scissor.y + state->scissor.height);
      if (x >= xmax || y >= ymax)
      {
         glxx_destroy_hw_framebuffer(&hw_fb);
         return true;
      }
   }
   bool full_rect = x == 0 && y == 0 && xmax == hw_fb.width && ymax == hw_fb.height;

   GLXX_HW_RENDER_STATE_T *rs = glxx_install_rs(state, &hw_fb, /*for_tlb_blit=*/false);

   glxx_destroy_hw_framebuffer(&hw_fb);

   if (!rs)
      return false;

   khrn_render_state_disallow_flush((KHRN_RENDER_STATE_T*)rs);

   // For each buffer, figure out if we are fully clearing
   uint32_t color_full_clear_mask = 0;
   if (full_rect)
      for (uint32_t b = 0, mask = clear->color_buffer_mask; mask; ++b, mask >>= 1)
         if ((mask & 1) && (((state->color_write >> (b * 4)) & 0xf) == 0xf))
            color_full_clear_mask |= 1u << b;
   bool depth_full_clear = full_rect && clear->depth;
   bool stencil_full_clear = full_rect && clear->stencil && ((state->stencil_mask.front & 0xff) == 0xff);

   // Discard CL if possible
   bool keep_cl = khrn_fmem_get_common_persist(&rs->fmem)->occlusion_query_list != NULL || rs->tf_used || rs->base.has_buffer_writes;
   if (!keep_cl)
   {
      for (unsigned b = 0; b < rs->installed_fb.rt_count; ++b)
      {
         if (!(color_full_clear_mask & (1u << b)) &&
            !glxx_bufstate_can_discard_cl(rs->color_buffer_state[b]))
         {
            keep_cl = true;
            break;
         }
      }
      if (!depth_full_clear && !glxx_bufstate_can_discard_cl(rs->depth_buffer_state))
         keep_cl = true;
      if (!stencil_full_clear && !glxx_bufstate_can_discard_cl(rs->stencil_buffer_state))
         keep_cl = true;
   }
   if (!keep_cl)
   {
      for (unsigned b = 0; b < rs->installed_fb.rt_count; ++b)
         glxx_bufstate_discard_cl(&rs->color_buffer_state[b]);
      glxx_bufstate_discard_cl(&rs->depth_buffer_state);
      glxx_bufstate_discard_cl(&rs->stencil_buffer_state);
      if (!glxx_hw_render_state_discard_and_restart(state, rs))
         goto error; // TODO set out of memory?
   }

   // Try doing HW clears
   bool use_draw_rect = false;
   for (uint32_t b = 0, mask = clear->color_buffer_mask; mask; ++b, mask >>= 1)
   {
      if (mask & 1)
      {
         if (glxx_bufstate_try_fast_clear(&rs->color_buffer_state[b], !(color_full_clear_mask & (1u << b))))
            memcpy(rs->clear_colors[b], clear->color_value, sizeof(rs->clear_colors[0]));
         else
            use_draw_rect = true;
      }
   }
   if (clear->depth)
   {
      if (glxx_bufstate_try_fast_clear(&rs->depth_buffer_state, !depth_full_clear))
         rs->depth_value = clear->depth_value;
      else
         use_draw_rect = true;
   }
   if (clear->stencil)
   {
      if (glxx_bufstate_try_fast_clear(&rs->stencil_buffer_state, !stencil_full_clear))
         rs->stencil_value = clear->stencil_value;
      else
         use_draw_rect = true;
   }

   if (use_draw_rect)
   {
      khrn_driver_incr_counters(KHRN_PERF_SOFT_CLEAR);

      // TODO Single pass clear with shaders
      if (clear->color_buffer_mask)
      {
         for (uint32_t b = 0, mask = clear->color_buffer_mask; mask; ++b, mask >>= 1)
         {
            if (mask & 1)
            {
               clear->color_buffer_mask = 1u << b;
               if (!glxx_draw_rect(state, rs, clear, x, y, xmax, ymax))
                  goto error;
               clear->depth = false;
               clear->stencil = false;
            }
         }
      }
      else if (!glxx_draw_rect(state, rs, clear, x, y, xmax, ymax))
         goto error;
   }
   else
      /* Managed to do all clears with HW clear */
      khrn_driver_incr_counters(KHRN_PERF_HARD_CLEAR);

   khrn_render_state_allow_flush((KHRN_RENDER_STATE_T*)rs);
   return true;

error:
   khrn_render_state_allow_flush((KHRN_RENDER_STATE_T*)rs);
   glxx_hw_discard_frame(rs);
   return false;
}
