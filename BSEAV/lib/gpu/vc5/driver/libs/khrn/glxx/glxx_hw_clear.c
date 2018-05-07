/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_hw.h"
#include "../common/khrn_counters.h"
#include "../common/khrn_render_state.h"
#include "libs/util/log/log.h"

LOG_DEFAULT_CAT("glxx_hw_clear")

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
   for (unsigned b = 0; b != V3D_MAX_RENDER_TARGETS; ++b)
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
   if (!glxx_init_hw_framebuffer(fb, &hw_fb, &state->fences))
      return false;

   // Intersect FB rect with scissor rect
   gfx_rect rect = {0, 0, hw_fb.width, hw_fb.height};
   if (state->caps.scissor_test)
   {
      gfx_rect_intersect(&rect, &state->scissor);
      if (rect.width == 0 || rect.height == 0)
      {
         glxx_destroy_hw_framebuffer(&hw_fb);
         return true;
      }
   }

   uint32_t color_full_clear_mask = 0;
   bool depth_full_clear = false;
   bool stencil_full_clear = false;

   // For each buffer, figure out if we are fully clearing
   bool full_rect = rect.x == 0 && rect.y == 0 && rect.width == hw_fb.width && rect.height == hw_fb.height;
   if (full_rect)
   {
      for (uint32_t b = 0, mask = clear->color_buffer_mask; mask; ++b, mask >>= 1)
      {
         if ((mask & 1) && (((state->color_write >> (b * 4)) & 0xf) == 0xf))
            color_full_clear_mask |= 1u << b;
      }
      depth_full_clear = clear->depth;
      stencil_full_clear = clear->stencil && ((state->stencil_mask.front & 0xff) == 0xff);
   }

   // Discard existing render-state if possible.
   bool existing;
   GLXX_HW_RENDER_STATE_T *rs = glxx_install_rs(state, &hw_fb, &existing, /*for_tlb_blit=*/false);
   if (rs && existing)
   {
      if (khrn_fmem_has_queries(&rs->fmem) || rs->tf.used || rs->base.has_buffer_writes)
         goto no_discard;

      for (unsigned b = 0; b < rs->installed_fb.rt_count; ++b)
      {
         if (!(color_full_clear_mask & (1u << b)) &&
            !glxx_bufstate_can_discard_rs(rs->color_buffer_state[b]))
         {
            goto no_discard;
         }
      }
      if (!depth_full_clear && !glxx_bufstate_can_discard_rs(rs->depth_buffer_state))
         goto no_discard;
      if (!stencil_full_clear && !glxx_bufstate_can_discard_rs(rs->stencil_buffer_state))
         goto no_discard;

      glxx_hw_discard_frame(rs);
      rs = glxx_install_rs(state, &hw_fb, &existing, /*for_tlb_blit=*/false);
   }
no_discard:

   glxx_destroy_hw_framebuffer(&hw_fb);

   if (!rs)
      return false;

   khrn_render_state_disallow_flush((khrn_render_state*)rs);

   rs->num_used_layers = rs->installed_fb.layers;
   rs->has_rasterization = true;

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
#if !V3D_HAS_GFXH1461_FIX
   /* Fast-clearing depth & loading stencil (or vice-versa) does not work
    * properly (see GFXH-1461).
    * Fall back to draw-rect clear if we might hit that case... */
   if ((clear->stencil && glxx_bufstate_might_need_load(rs->depth_buffer_state)) ||
         (clear->depth && glxx_bufstate_might_need_load(rs->stencil_buffer_state)))
      use_draw_rect = true;
#endif

   log_trace("%s color_mask=0x%x%s%s", use_draw_rect ? "draw_rect" : "fast",
      clear->color_buffer_mask, clear->depth ? " depth" : "", clear->stencil ? " stencil" : "");

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
               if (!glxx_draw_rect(state, rs, clear, &rect))
                  goto error;
               clear->depth = false;
               clear->stencil = false;
            }
         }
      }
      else if (!glxx_draw_rect(state, rs, clear, &rect))
         goto error;
   }
   else
      /* Managed to do all clears with HW clear */
      khrn_driver_incr_counters(KHRN_PERF_HARD_CLEAR);

   khrn_render_state_allow_flush((khrn_render_state*)rs);
   return true;

error:
   khrn_render_state_allow_flush((khrn_render_state*)rs);
   glxx_hw_discard_frame(rs);
   return false;
}
