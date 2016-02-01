/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glxx
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "middleware/khronos/glxx/glxx_log.h"
#include "interface/khronos/glxx/glxx_int_config.h"
#include "interface/khronos/tools/dglenum/dglenum.h"
#include "middleware/khronos/common/khrn_image_plane.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_inner.h"

VCOS_LOG_CAT_T glxx_log             = VCOS_LOG_INIT("glxx",                   VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_query_log       = VCOS_LOG_INIT("glxx_query",             VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_fbo_log         = VCOS_LOG_INIT("glxx_fbo",               VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_tf_log          = VCOS_LOG_INIT("glxx_tf",                VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_buffer_log      = VCOS_LOG_INIT("glxx_buffer",            VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_vao_log         = VCOS_LOG_INIT("glxx_vao",               VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_attrib_log      = VCOS_LOG_INIT("glxx_attrib",            VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_error_log       = VCOS_LOG_INIT("glxx_error",             VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_draw_log        = VCOS_LOG_INIT("glxx_draw",              VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_clear_log       = VCOS_LOG_INIT("glxx_clear",             VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_blit_log        = VCOS_LOG_INIT("glxx_blit",              VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_invalidate_log  = VCOS_LOG_INIT("glxx_invalidate",        VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_cle_log         = VCOS_LOG_INIT("glxx_cle",               VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_ubo_log         = VCOS_LOG_INIT("glxx_ubo",               VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_program_log     = VCOS_LOG_INIT("glxx_program",           VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_ldst_log        = VCOS_LOG_INIT("glxx_ldst",              VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_depth_log       = VCOS_LOG_INIT("glxx_depth",             VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_stencil_log     = VCOS_LOG_INIT("glxx_stencil",           VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_scissor_log     = VCOS_LOG_INIT("glxx_scissor",           VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_blend_log       = VCOS_LOG_INIT("glxx_blend",             VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_texture_log     = VCOS_LOG_INIT("glxx_texture",           VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_teximage_log    = VCOS_LOG_INIT("glxx_teximage",          VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_flush_log       = VCOS_LOG_INIT("glxx_flush",             VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_zonly_log       = VCOS_LOG_INIT("glxx_zonly",             VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_bin_log         = VCOS_LOG_INIT("glxx_bin",               VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_render_log      = VCOS_LOG_INIT("glxx_render",            VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_read_log        = VCOS_LOG_INIT("glxx_read",              VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_uniform_log     = VCOS_LOG_INIT("glxx_uniform",           VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_record_log      = VCOS_LOG_INIT("glxx_record",            VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_rs_log          = VCOS_LOG_INIT("glxx_rs",                VCOS_LOG_WARN);
VCOS_LOG_CAT_T glxx_prog_iface_log  = VCOS_LOG_INIT("glxx_program_interface", VCOS_LOG_WARN);


extern unsigned khrn_hw_render_state_allocated_order(const GLXX_HW_RENDER_STATE_T *hw_rs);

void glxx_trace_rs(const VCOS_LOG_CAT_T *cat, const void *rs_v, const char *msg)
{
   GLXX_HW_RENDER_STATE_T *rs = (GLXX_HW_RENDER_STATE_T *)(rs_v);
   GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;
   GFX_BUFSTATE_FLUSH_T depth_flush;
   GFX_BUFSTATE_FLUSH_T stencil_flush;
   GFX_BUFSTATE_FLUSH_T color_flush    [GLXX_MAX_RENDER_TARGETS];
   GFX_BUFSTATE_FLUSH_T color_ms_flush [GLXX_MAX_RENDER_TARGETS];
   unsigned ord = khrn_hw_render_state_allocated_order(rs);
   unsigned b;

   if (rs == NULL)
      return;

   gfx_bufstate_flush(&depth_flush, rs->depth_buffer_state);
   gfx_bufstate_flush(&stencil_flush, rs->stencil_buffer_state);

   for (b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
   {
      gfx_bufstate_flush(&color_flush[b], rs->color_buffer_state[b]);
      gfx_bufstate_flush(&color_ms_flush[b], rs->ms_color_buffer_state[b]);
   }

   vcos_logc_trace(
      cat,
      "%s [rs] ord = %u, label = %s, rt count = %u, fb size %u x %u, %s",
      msg,
      ord,
      "-",
      fb->rt_count,
      fb->width, fb->height,
      fb->ms               ? "multisample "        : "");
   for (b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
      if (rs->color_buffer_state[b] != GFX_BUFSTATE_MISSING)
      {
         GFX_LFMT_SPRINT(lfmt_desc, khrn_image_plane_lfmt(&fb->color[b]));
         vcos_logc_trace(
            cat,
            "[rs] color %u fmt: %s non-ms: %s %s%s ms: %s %s%s",
            b,
            lfmt_desc,
            gfx_bufstate_desc(rs->color_buffer_state[b]),
            color_flush[b].load ?  "load " : "",
            color_flush[b].store ? "store " : "",
            fb->ms ? gfx_bufstate_desc(rs->ms_color_buffer_state[b]) : "-",
            (fb->ms && color_ms_flush[b].load) ? "load " : "",
            (fb->ms && color_ms_flush[b].store) ? "store " : "");
      }

   vcos_logc_trace(
      cat,
      "[rs] depth: %s %s%s",
      gfx_bufstate_desc(rs->depth_buffer_state),
      depth_flush.load ? "load " : "",
      depth_flush.store ? "store " : "");

   vcos_logc_trace(
      cat,
      "[rs] stencil: %s %s%s",
      gfx_bufstate_desc(rs->stencil_buffer_state),
      stencil_flush.load ? "load " : "",
      stencil_flush.store ? "store " : "");
}
