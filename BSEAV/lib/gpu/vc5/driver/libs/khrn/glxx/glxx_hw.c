/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <limits.h>
#include <assert.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "../common/khrn_int_common.h"
#include "../common/khrn_options.h"
#include "../common/khrn_render_state.h"
#include "../common/khrn_process.h"
#include "gl_public_api.h"
#include "glxx_hw.h"
#include "glxx_bufstate.h"
#include "glxx_inner.h"
#include "glxx_shader.h"
#include "glxx_server.h"
#include "glxx_server_internal.h"
#include "glxx_server_texture.h"
#include "glxx_framebuffer.h"
#include "glxx_translate.h"
#include "glxx_clear_layers.h"
#include "../gl20/gl20_program.h"
#include "../glsl/glsl_tex_params.h"
#include "../glsl/glsl_backend_uniforms.h"
#include "../gl11/gl11_shadercache.h"
#include "../gl11/gl11_shader.h"

#include "libs/core/v3d/v3d_common.h"
#include "libs/core/v3d/v3d_cl.h"
#include "libs/core/v3d/v3d_vpm.h"
#include "libs/core/v3d/v3d_tlb.h"
#include "libs/core/v3d/v3d_clear_shader.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/gfx_buffer/gfx_buffer_uif_config.h"
#include "libs/util/gfx_util/gfx_util_conv.h"

LOG_DEFAULT_CAT("glxx_hw")

/*************************************************************
 Static function forwards
 *************************************************************/

static bool write_gl_shader_record(
   GLXX_HW_RENDER_STATE_T *rs,
   const GLXX_SERVER_STATE_T *state,
   GLXX_LINK_RESULT_DATA_T *link_data, const glxx_hw_image_like_uniforms *image_like_uniforms,
   const GLXX_ATTRIB_CONFIG_T *attribs, const glxx_hw_vb *vbs);

static v3d_prim_mode_t convert_primitive_type(GLXX_SERVER_STATE_T const* state, GLenum mode);
static v3d_index_type_t convert_index_type(GLenum type);

static bool insert_flat_shading_flags(GLXX_HW_RENDER_STATE_T *rs, const GLXX_LINK_RESULT_DATA_T *link_data);
static bool insert_centroid_flags(GLXX_HW_RENDER_STATE_T *rs, const GLXX_LINK_RESULT_DATA_T *link_data);
#if V3D_VER_AT_LEAST(4,1,34,0)
static bool insert_noperspective_flags(GLXX_HW_RENDER_STATE_T *rs, const GLXX_LINK_RESULT_DATA_T *link_data);
#endif

static bool glxx_hw_update_z_prepass_state(
   GLXX_SERVER_STATE_T *state,
   GLXX_LINK_RESULT_DATA_T const *link_data,
   GLXX_HW_RENDER_STATE_T *rs
   );

static bool post_draw_journal(
   GLXX_HW_RENDER_STATE_T *rs,
   const glxx_hw_draw *draw,
   const GLXX_LINK_RESULT_DATA_T *link_data,
   const GLXX_ATTRIB_CONFIG_T *attribs, const glxx_hw_vb *vbs);

static GLXX_HW_RENDER_STATE_T* get_existing_rs(const GLXX_SERVER_STATE_T *state,
      const GLXX_HW_FRAMEBUFFER_T *fb);
static bool setup_dummy_texture_for_texture_unif(GLXX_TEXTURE_UNIF_T *texture_unif,
      bool for_image_unit, khrn_fmem *fmem, const GLSL_SAMPLER_T* sampler);

/*************************************************************
 Global Functions
 *************************************************************/

void glxx_hw_invalidate_framebuffer(
   GLXX_SERVER_STATE_T *state, GLXX_FRAMEBUFFER_T *fb,
   uint32_t rt, bool all_color_ms, bool depth, bool stencil)
{
#if KHRN_DEBUG
   if (khrn_options.no_invalidate_fb)
      return;
#endif

   GLXX_HW_FRAMEBUFFER_T hw_fb;
   if (!glxx_init_hw_framebuffer(fb, &hw_fb, &state->fences))
      return;

   GLXX_HW_RENDER_STATE_T *rs = get_existing_rs(state, &hw_fb);
   if (rs)
   {
      if (all_color_ms && (rt != gfx_mask(hw_fb.rt_count)))
         rs->color_discard_ms = true;
      for (unsigned b = 0; rt; ++b, rt >>= 1)
         if (rt & 1)
            glxx_bufstate_invalidate(&rs->color_buffer_state[b]);

      if (depth)
         glxx_bufstate_invalidate(&rs->depth_buffer_state);
      if (stencil)
         glxx_bufstate_invalidate(&rs->stencil_buffer_state);
   }
   else
   {
      /* Didn't find a matching render state. Just mark the images in the
       * framebuffer as invalid,,, */

      for (unsigned b = 0; rt; ++b, rt >>= 1)
      {
         if (rt & 1)
            khrn_image_plane_invalidate(&hw_fb.color[b], KHRN_CHANGRP_ALL);
         if ((rt & 1) || all_color_ms)
            khrn_image_plane_invalidate(&hw_fb.color_ms[b], KHRN_CHANGRP_ALL);
      }

      if (depth)
         khrn_image_plane_invalidate(&hw_fb.depth, KHRN_CHANGRP_NONSTENCIL);
      if (stencil)
         khrn_image_plane_invalidate(&hw_fb.stencil, KHRN_CHANGRP_STENCIL);
   }

   glxx_destroy_hw_framebuffer(&hw_fb);
}

void glxx_hw_invalidate_default_draw_framebuffer(
   GLXX_SERVER_STATE_T *state,
   bool color, bool color_ms, bool depth, bool stencil)
{
   glxx_hw_invalidate_framebuffer(state, state->default_framebuffer[GLXX_DRAW_SURFACE],
      color ? 1 : 0, color_ms, depth, stencil);
}

void glxx_destroy_hw_framebuffer(GLXX_HW_FRAMEBUFFER_T *hw_fb)
{
   for (unsigned b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
   {
      KHRN_MEM_ASSIGN(hw_fb->color[b].image, NULL);
      KHRN_MEM_ASSIGN(hw_fb->color_ms[b].image, NULL);
   }
   KHRN_MEM_ASSIGN(hw_fb->depth.image, NULL);
   KHRN_MEM_ASSIGN(hw_fb->stencil.image, NULL);
}

void glxx_assign_hw_framebuffer(GLXX_HW_FRAMEBUFFER_T *a, const GLXX_HW_FRAMEBUFFER_T *b)
{
   for (unsigned i = 0; i != GLXX_MAX_RENDER_TARGETS; ++i)
   {
      KHRN_MEM_ASSIGN(a->color[i].image, b->color[i].image);
      KHRN_MEM_ASSIGN(a->color_ms[i].image, b->color_ms[i].image);
   }
   KHRN_MEM_ASSIGN(a->depth.image, b->depth.image);
   KHRN_MEM_ASSIGN(a->stencil.image, b->stencil.image);

   /* Copy the rest of the fields (this will also re-copy the image handles but
    * that is harmless) */
   *a = *b;
}

/* Updates max common size in hw_fb based on attached image plane (possibly NULL) */
static void update_hw_fb_with_image(
   GLXX_HW_FRAMEBUFFER_T *hw_fb,
   khrn_image_plane *image_plane,
   bool multisample)
{
   // Not all framebuffers have color / multisample / depth / stencil
   if (image_plane->image == NULL)
      return;

   unsigned w = khrn_image_get_width(image_plane->image);
   unsigned h = khrn_image_get_height(image_plane->image);
   unsigned layers = khrn_image_get_depth(image_plane->image) *
      khrn_image_get_num_elems(image_plane->image);

   if (multisample)
   {
      w /= 2;
      h /= 2;
   }
   assert(w <= GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE);
   assert(h <= GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE);
   assert(layers <= GLXX_CONFIG_MAX_FRAMEBUFFER_LAYERS);

   hw_fb->width = gfx_umin(hw_fb->width, w);
   hw_fb->height = gfx_umin(hw_fb->height, h);
   assert(layers >= 1);
   hw_fb->layers = gfx_umin(hw_fb->layers, layers);
}

bool glxx_init_hw_framebuffer(const GLXX_FRAMEBUFFER_T *fb,
                               GLXX_HW_FRAMEBUFFER_T *hw_fb,
                               glxx_context_fences *fences)
{
   assert(glxx_fb_is_complete(fb, fences));
   memset(hw_fb, 0, sizeof(*hw_fb));

   hw_fb->ms = (glxx_fb_get_ms_mode(fb) != GLXX_NO_MS);

   // we always configure tilebuffer based on all attached images in fb,
   // regardless of any draw buffers setting.
   hw_fb->rt_count = 1;
   for (unsigned b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
   {
      const GLXX_ATTACHMENT_T *att = &fb->attachment[GLXX_COLOR0_ATT + b];

      if (!glxx_attachment_acquire_image(att, GLXX_DOWNSAMPLED, false, fences,
               &hw_fb->color[b].image, NULL))
         goto error_out;

      if (!glxx_attachment_acquire_image(att, GLXX_MULTISAMPLED, false, fences,
               &hw_fb->color_ms[b].image, NULL))
         goto error_out;

      if (hw_fb->color[b].image || hw_fb->color_ms[b].image)
         hw_fb->rt_count = b + 1;
   }

   for (unsigned b = 0; b != hw_fb->rt_count; ++b)
   {
      if (hw_fb->color[b].image)
      {
         gfx_lfmt_translate_rt_format(&hw_fb->color_rt_format[b], khrn_image_plane_lfmt(&hw_fb->color[b]));
#ifndef NDEBUG
         if (hw_fb->color_ms[b].image)
         {
            V3D_RT_FORMAT_T rt_format;
            gfx_lfmt_translate_rt_format(&rt_format, khrn_image_plane_lfmt(&hw_fb->color_ms[b]));
            assert(v3d_rt_formats_equal(&hw_fb->color_rt_format[b], &rt_format));
         }
#endif
      }
      else if (hw_fb->color_ms[b].image)
      {
         gfx_lfmt_translate_rt_format(&hw_fb->color_rt_format[b], khrn_image_plane_lfmt(&hw_fb->color_ms[b]));
      }
      else
      {
         hw_fb->color_rt_format[b].bpp = V3D_RT_BPP_32;
         hw_fb->color_rt_format[b].type = V3D_RT_TYPE_8;
#if V3D_VER_AT_LEAST(4,1,34,0)
         hw_fb->color_rt_format[b].clamp = V3D_RT_CLAMP_NONE;
#endif
      }
   }

   {
      bool depth_ms;
      if (!glxx_attachment_acquire_image(&fb->attachment[GLXX_DEPTH_ATT],
               GLXX_PREFER_DOWNSAMPLED, false, fences, &hw_fb->depth.image, &depth_ms))
         goto error_out;
      if (hw_fb->depth.image)
         assert(hw_fb->ms == depth_ms);
   }

   {
      bool stencil_ms;
      if (!glxx_attachment_acquire_image(&fb->attachment[GLXX_STENCIL_ATT],
               GLXX_PREFER_DOWNSAMPLED, false, fences, &hw_fb->stencil.image, &stencil_ms))
         goto error_out;
      if (hw_fb->stencil.image)
      {
         assert(hw_fb->ms == stencil_ms);
         /* Stencil is always in the last plane */
         hw_fb->stencil.plane_idx = khrn_image_get_num_planes(hw_fb->stencil.image) - 1;
      }
   }

   // Figure out largest common size
   hw_fb->width = ~0u;
   hw_fb->height = ~0u;
   hw_fb->layers = ~0u;
   for (unsigned b = 0; b < hw_fb->rt_count; ++b)
   {
      update_hw_fb_with_image(hw_fb, &hw_fb->color[b], false);
      update_hw_fb_with_image(hw_fb, &hw_fb->color_ms[b], true);
   }
   update_hw_fb_with_image(hw_fb, &hw_fb->depth, hw_fb->ms);
   update_hw_fb_with_image(hw_fb, &hw_fb->stencil, hw_fb->ms);

   /* We do not have any buffer. We must have a default_depth/default_height */
   if (hw_fb->width == ~0u)
   {
      assert(fb->default_width);
      hw_fb->width = fb->default_width;
   }
   if (hw_fb->height == ~0u)
   {
      assert(fb->default_height);
      hw_fb->height = fb->default_height;
   }
   if (hw_fb->layers == ~0u)
   {
      hw_fb->layers = gfx_umax(fb->default_layers, 1);
   }

   return true;

error_out:
   glxx_destroy_hw_framebuffer(hw_fb);
   return false;
}

static bool compare_hw_fb(const GLXX_HW_FRAMEBUFFER_T *a,
                          const GLXX_HW_FRAMEBUFFER_T *b)
{
   if ((a->ms != b->ms) || (a->rt_count != b->rt_count))
      return false;

   for (unsigned i = 0; i < a->rt_count; ++i)
   {
      if (!khrn_image_plane_equal(&a->color[i], &b->color[i]))
         return false;

      if (!khrn_image_plane_equal(&a->color_ms[i], &b->color_ms[i]))
         return false;

      /* These are derived from color[i]/color_ms[i], so if those match, then
       * these should match... */
      assert(v3d_rt_formats_equal(&a->color_rt_format[i], &b->color_rt_format[i]));
   }

   if (!khrn_image_plane_equal(&a->depth, &b->depth))
      return false;

   if (!khrn_image_plane_equal(&a->stencil, &b->stencil))
      return false;

   /* Width/height are derived from the attached images, so if all the
    * attachments match, they should match too... */
   assert(a->width == b->width);
   assert(a->height == b->height);
   assert(a->layers == b->layers);

   return true;
}

struct rs_selection_info {
   const GLXX_HW_FRAMEBUFFER_T   *hw_fb;
   GLXX_HW_RENDER_STATE_T        *rs;
};

static bool try_select_rs(GLXX_HW_RENDER_STATE_T *rs, void *data)
{
   struct rs_selection_info *info = (struct rs_selection_info*)data;

   if (compare_hw_fb(info->hw_fb, &rs->installed_fb))
   {
      info->rs = rs;
      return true;
   }

   return false;
}

GLXX_HW_RENDER_STATE_T *glxx_find_existing_rs_for_fb(const GLXX_HW_FRAMEBUFFER_T *hw_fb)
{
   struct rs_selection_info info;
   info.hw_fb = hw_fb;
   info.rs = NULL;
   glxx_hw_render_state_foreach(try_select_rs, &info);

   return info.rs;
}

static GLXX_HW_RENDER_STATE_T *create_and_start_render_state(const GLXX_HW_FRAMEBUFFER_T *fb, GLXX_SERVER_STATE_T* state)
{
   GLXX_HW_RENDER_STATE_T *rs = khrn_render_state_get_glxx(khrn_render_state_new(KHRN_RENDER_STATE_TYPE_GLXX));

   rs->server_state = state;

   glxx_server_invalidate_for_render_state(state, rs);

   if (!glxx_hw_start_frame_internal(rs, fb))
   {
      glxx_hw_discard_frame(rs);
      rs = NULL;
   }

   khrn_mem_acquire(state->shared->gpu_aborted);
   rs->fmem.persist->gpu_aborted = state->shared->gpu_aborted;

   return rs;
}

static GLXX_HW_RENDER_STATE_T* get_existing_rs(const GLXX_SERVER_STATE_T *state,
      const GLXX_HW_FRAMEBUFFER_T *fb)
{
   if (state->current_render_state != NULL &&
       compare_hw_fb(fb, &state->current_render_state->installed_fb))
      return state->current_render_state;

   return glxx_find_existing_rs_for_fb(fb);
}

GLXX_HW_RENDER_STATE_T* glxx_install_rs(GLXX_SERVER_STATE_T *state,
   const GLXX_HW_FRAMEBUFFER_T *fb, bool* ret_existing, bool for_tlb_blit)
{
   /* we might have users on fence_to_depend_on, flush them now */
   khrn_fence_flush(state->fences.fence_to_depend_on);

   // handle scenarios where an existing render state needs to be flushed
   GLXX_HW_RENDER_STATE_T *rs = get_existing_rs(state, fb);
   if (rs != NULL)
   {
      // Context mismatch, this is undefined behaviour so flush.
      bool flush_rs = rs->server_state != state;

      // If drawing to an existing render-state that has TLB blits then it needs to be flushed.
      flush_rs = flush_rs || (!for_tlb_blit && rs->num_blits != 0);

      // Check to see if we need to flush due to high client fmem use.
      flush_rs = flush_rs || khrn_fmem_should_flush(&rs->fmem);

      if (flush_rs)
      {
         glxx_hw_render_state_flush(rs);
         rs = NULL;
      }
   }

   // create a new render state if required
   *ret_existing = rs != NULL;
   if (rs == NULL)
   {
      rs = create_and_start_render_state(fb, state);
      if (rs == NULL)
         return NULL;
   }

   if (  !khrn_fmem_record_fence_to_signal(&rs->fmem, state->fences.fence)
      || !khrn_fmem_record_fence_to_depend_on(&rs->fmem, state->fences.fence_to_depend_on) )
   {
      glxx_hw_discard_frame(rs);
      return NULL;
   }

   // A render state can only dither if everything was dithered
   // XXX This is not really the right place to put this. glxx_install_rs is
   // called for eg blits.
   if (!state->caps.dither)
      rs->dither = false;

   // cache the current render-state to speedup the lookup next time
   state->current_render_state = rs;
   return rs;
}

/* Returns true if 'mode' does not cull back-facing polygons, false otherwise */
static bool enable_back(GLenum cull_mode)
{
   assert(cull_mode == GL_FRONT || cull_mode == GL_BACK || cull_mode == GL_FRONT_AND_BACK);

   return cull_mode == GL_FRONT;
}

/* Returns true if 'mode' does not cull front-facing polygons, false otherwise */
static bool enable_front(GLenum cull_mode)
{
   assert(cull_mode == GL_FRONT || cull_mode == GL_BACK || cull_mode == GL_FRONT_AND_BACK);

   return cull_mode == GL_BACK;
}


/*!
 * \brief \a Undocumented
 *
 * Returns 1 if front-facing primitives are described CW.
 *
 * \param mode OpenGL mode flag.
 */
static uint32_t glxx_front_facing_is_clockwise(GLenum mode)
{
   assert(mode == GL_CW || mode == GL_CCW);

   return mode == GL_CW;
}

#if !V3D_VER_AT_LEAST(4,0,2,0)
static bool fmt_requires_alpha16(v3d_rt_type_t type, v3d_rt_bpp_t bpp)
{
   switch (type)
   {
      case V3D_RT_TYPE_16I:
      case V3D_RT_TYPE_16UI:
      case V3D_RT_TYPE_16F:
         return bpp == V3D_RT_BPP_64;
      default:
         return false;
   }
}
#endif

uint32_t glxx_get_attribs_live(const GLXX_SERVER_STATE_T *state)
{
   if (IS_GL_11(state))
      return gl11_get_live_attr_set(&state->gl11.shaderkey);
   else {
      IR_PROGRAM_T *ir = gl20_program_common_get(state)->linked_glsl_program->ir;
      return ir->live_attr_set;
   }
}

static unsigned compute_texture_uniforms(GLXX_SERVER_STATE_T *state, glxx_render_state *rs,
   glxx_hw_image_like_uniforms *image_like_uniforms)
{
   GLSL_SAMPLER_T gl11_samplers[GL11_CONFIG_MAX_TEXTURE_UNITS];

   const uint32_t* uniform_data;
   GLSL_SAMPLER_T *samplers;
   unsigned num_samplers;
   bool is_gl11 = IS_GL_11(state);
   if (is_gl11)
   {
      uniform_data = NULL;
      samplers = gl11_samplers;
      num_samplers = GL11_CONFIG_MAX_TEXTURE_UNITS;
      for (unsigned i = 0; i != GL11_CONFIG_MAX_TEXTURE_UNITS; ++i)
      {
         GLSL_SAMPLER_T* sampler = &gl11_samplers[i];
         if (!(state->gl11.shaderkey.texture[i] & GL11_TEX_ENABLE))
         {
            sampler->location = ~0u;
            continue;
         }

         sampler->location = i;
         sampler->texture_type = state->gl11.texunits[i].target_enabled_2D ? GL_TEXTURE_2D : GL_TEXTURE_EXTERNAL_OES;
         sampler->is_32bit = false;
         sampler->in_binning = false;
       #if V3D_VER_AT_LEAST(4,0,2,0)
         sampler->array_size = 0;
         sampler->in_array = false;
       #endif
      }
   }
   else
   {
      GL20_PROGRAM_COMMON_T *program_common = gl20_program_common_get(state);
      uniform_data = program_common->uniform_data;
      samplers = program_common->linked_glsl_program->samplers;
      num_samplers = program_common->linked_glsl_program->num_samplers;
   }

#if V3D_VER_AT_LEAST(4,0,2,0)
   memset(image_like_uniforms->unnorm, 0, sizeof(image_like_uniforms->unnorm));
#endif

   for (unsigned i = 0; i != num_samplers; ++i)
   {
      enum glxx_tex_target tex_target = samplers[i].texture_type;

      unsigned j = samplers[i].location;
      if (!is_gl11)
      {
       #if V3D_VER_AT_LEAST(4,0,2,0)
         unsigned array_size = samplers[i].array_size;
         if (array_size && !khrn_fmem_reserve_tmu_cfg(&rs->fmem, array_size*2))
            return ~0u;
       #endif

         j = uniform_data[j];
      }
      else if (j == ~0u)
         continue;

      if (j < GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
      {
         GLXX_TEXTURE_T* texture = glxx_textures_get_texture(&state->bound_texture[j], tex_target);
         assert(texture->target == tex_target);

         const GLXX_TEXTURE_SAMPLER_STATE_T* sampler = state->bound_sampler[j];
         if (!sampler || glxx_tex_target_is_multisample(tex_target) || tex_target == GL_TEXTURE_BUFFER)
            sampler = &texture->sampler;

         enum glxx_tex_completeness complete = glxx_texture_key_and_uniforms(
            texture, NULL, sampler, &samplers[i], rs, &image_like_uniforms->tex[i], &state->fences);

#if V3D_VER_AT_LEAST(4,0,2,0)
         if (sampler->unnormalised_coords)
            image_like_uniforms->unnorm[i / 16] |= 1 << (i % 16);
#endif

         switch (complete)
         {
         case INCOMPLETE:     break;
         case COMPLETE:       continue;
         case OUT_OF_MEMORY:  return ~0u;
         default:             unreachable();
         }
      }

      assert(!IS_GL_11(state)); /* All gl11 textures should be enabled by now */

      if (!setup_dummy_texture_for_texture_unif(&image_like_uniforms->tex[i], false, &rs->fmem, &samplers[i]))
         return ~0u;
   }

   return num_samplers;
}

static bool setup_dummy_texture_for_texture_unif(GLXX_TEXTURE_UNIF_T *texture_unif,
   bool for_image_unit, khrn_fmem *fmem, const GLSL_SAMPLER_T* glsl_sampler)
{
   gmem_handle_t dummy_texture_handle = get_dummy_texture();
   v3d_addr_t addr_base = khrn_fmem_sync_and_get_addr(fmem,
         dummy_texture_handle,
         glsl_sampler->in_binning ?
         V3D_BARRIER_TMU_DATA_READ : 0,
         V3D_BARRIER_TMU_DATA_READ);
   assert(v3d_addr_aligned(addr_base, V3D_TMU_ML_ALIGN));

   bool is_1d = glxx_tex_target_is_1d(glsl_sampler->texture_type);
   unsigned width  = 4;
   unsigned height = is_1d ? 1 : 4;
   const unsigned depth = 1;
   const v3d_tmu_type_t type = V3D_TMU_TYPE_RGBA8;

#if V3D_VER_AT_LEAST(4,1,34,0)
   /* Override height from above with high bits of width */
   if (is_1d)
      v3d_tmu_get_wh_for_1d_tex_state(&width, &height, width);
#endif

   texture_unif->width = width;
   texture_unif->height = height;
   texture_unif->depth = depth;
#if !V3D_VER_AT_LEAST(4,0,2,0)
   texture_unif->base_level = 0;
#endif

   if (for_image_unit)
   {
      /* by setting w/h/d to 0, the image store will not be done */
      texture_unif->width = 0;
      texture_unif->height = is_1d ? 1 : 0;
      texture_unif->depth = 0;

#if !V3D_VER_AT_LEAST(4,0,2,0)
      texture_unif->arr_stride = 0;
      texture_unif->lx_addr = addr_base;
      texture_unif->lx_pitch = 0;
      texture_unif->lx_slice_pitch = 0;
      texture_unif->lx_swizzling = GLSL_IMGUNIT_SWIZZLING_LT;
#endif
   }

#if V3D_VER_AT_LEAST(4,0,2,0)
   V3D_TMU_TEX_STATE_T tex_state;
   memset(&tex_state, 0, sizeof(tex_state));
   tex_state.l0_addr = addr_base;
   tex_state.arr_str = 4*4*4;// For 3D textures this is used to init the slice_pitch
   tex_state.width = width;
   tex_state.height = height;
   tex_state.depth = depth;
   tex_state.type = type;
   tex_state.swizzles[0] = V3D_TMU_SWIZZLE_R;
   tex_state.swizzles[1] = V3D_TMU_SWIZZLE_G;
   tex_state.swizzles[2] = V3D_TMU_SWIZZLE_B;
   tex_state.swizzles[3] = V3D_TMU_SWIZZLE_A;
   uint8_t hw_tex_state[V3D_TMU_TEX_STATE_PACKED_SIZE];
   v3d_pack_tmu_tex_state(hw_tex_state, &tex_state);
   texture_unif->hw_param[0] = khrn_fmem_add_tmu_tex_state(fmem, hw_tex_state, /*extended=*/false, glsl_sampler->in_array);
   if (!texture_unif->hw_param[0])
      return false;

   if (for_image_unit)
      texture_unif->hw_param[1] = 0; // ignored in this case
   else
   {
      V3D_TMU_SAMPLER_T sampler;
      memset(&sampler, 0, sizeof(sampler));
#if V3D_VER_AT_LEAST(4,1,34,0)
      sampler.magfilt = V3D_TMU_FILTER_NEAREST;
      sampler.minfilt = V3D_TMU_FILTER_NEAREST;
      sampler.mipfilt = V3D_TMU_MIPFILT_NEAREST;
#else
      sampler.filters = V3D_TMU_FILTERS_MIN_NEAR_MAG_NEAR;
#endif
      uint8_t hw_sampler[V3D_TMU_SAMPLER_PACKED_SIZE];
      v3d_pack_tmu_sampler(hw_sampler, &sampler);
      texture_unif->hw_param[1] = khrn_fmem_add_tmu_sampler(fmem, hw_sampler, /*extended=*/false, glsl_sampler->in_array);
      if (!texture_unif->hw_param[1])
         return false;
   }

#else
   V3D_TMU_PARAM0_CFG1_T tmu_param0_cfg1 = { 0, };
   if (for_image_unit)
      tmu_param0_cfg1.wrap_s = tmu_param0_cfg1.wrap_t = tmu_param0_cfg1.wrap_r = V3D_TMU_WRAP_BORDER;
   texture_unif->hw_param[0] = v3d_pack_tmu_param0_cfg1(&tmu_param0_cfg1);

   // setup indirect record
   V3D_TMU_INDIRECT_T tmu_indirect = { 0, };
   tmu_indirect.filters = V3D_TMU_FILTERS_MIN_NEAR_MAG_NEAR;
   tmu_indirect.arr_str = 4*4*4;// For 3D textures this is used to init the slice_pitch
   tmu_indirect.base = addr_base;
   tmu_indirect.width = width;
   tmu_indirect.height = height;
   tmu_indirect.depth = depth;
   tmu_indirect.ttype = type;
#if V3D_VER_AT_LEAST(3,3,0,0)
   tmu_indirect.u.not_child_image.output_type = glsl_sampler->is_32bit ? V3D_TMU_OUTPUT_TYPE_32 : V3D_TMU_OUTPUT_TYPE_16;
#else
   tmu_indirect.u.not_child_image.output_type = V3D_TMU_OUTPUT_TYPE_AUTO;
#endif
   tmu_indirect.swizzles[0] = V3D_TMU_SWIZZLE_R;
   tmu_indirect.swizzles[1] = V3D_TMU_SWIZZLE_G;
   tmu_indirect.swizzles[2] = V3D_TMU_SWIZZLE_B;
   tmu_indirect.swizzles[3] = V3D_TMU_SWIZZLE_A;

   uint32_t hw_indirect[V3D_TMU_INDIRECT_PACKED_SIZE / 4];
   v3d_pack_tmu_indirect_not_child_image(hw_indirect, &tmu_indirect);
   texture_unif->hw_param[1] = khrn_fmem_add_tmu_indirect(fmem, hw_indirect);
   if (!texture_unif->hw_param[1])
      return false;

   texture_unif->hw_param1_no_aniso = texture_unif->hw_param[1];

   tmu_indirect.filters = V3D_TMU_FILTERS_MIN_LIN_MAG_LIN;
   v3d_pack_tmu_indirect_not_child_image(hw_indirect, &tmu_indirect);
   v3d_addr_t gather012_indirect_addr = khrn_fmem_add_tmu_indirect(fmem, hw_indirect);
   if (!gather012_indirect_addr)
      return false;
   for (int i=0; i<3; i++)
      texture_unif->hw_param1_gather[i] = gather012_indirect_addr;

   tmu_indirect.swizzles[0] = tmu_indirect.swizzles[3];
   v3d_pack_tmu_indirect_not_child_image(hw_indirect, &tmu_indirect);
   texture_unif->hw_param1_gather[3] = khrn_fmem_add_tmu_indirect(fmem, hw_indirect);
   if (!texture_unif->hw_param1_gather[3])
      return false;

   texture_unif->hw_param1_fetch = texture_unif->hw_param[1];

#if !V3D_VER_AT_LEAST(3,3,0,0)
   texture_unif->gadgettype = glsl_sampler->is_32bit ? GLSL_GADGETTYPE_SWAP1632 : GLSL_GADGETTYPE_AUTO;
#endif
#endif
   return true;
}

static unsigned compute_image_uniforms(GLXX_SERVER_STATE_T *state, glxx_render_state *rs,
   GLXX_TEXTURE_UNIF_T unifs[GLXX_CONFIG_MAX_IMAGE_UNITS])
{
   if (IS_GL_11(state))
      return 0;

   const GL20_PROGRAM_COMMON_T* program = gl20_program_common_get(state);
   const GLSL_IMAGE_T* images = program->linked_glsl_program->images;
   unsigned num_images = program->linked_glsl_program->num_images;

   for (unsigned i = 0; i != num_images; ++i)
   {
      const GLSL_IMAGE_T* image = &images[i];

      unsigned j = program->uniform_data[image->sampler.location];
      if (j < GLXX_CONFIG_MAX_IMAGE_UNITS)
      {
         const glxx_image_unit *image_unit = &state->image_unit[j];
         assert(images[i].internalformat != GL_NONE);

         glxx_calc_image_unit calc_image_unit;
         if (glxx_get_calc_image_unit(image_unit, image, &calc_image_unit) == GLXX_ACC_OK)
         {
            enum glxx_tex_completeness complete = glxx_texture_key_and_uniforms(
                  image_unit->texture, &calc_image_unit, khrn_get_image_unit_default_sampler(),
                  &image->sampler, rs, &unifs[i], &state->fences);

            switch (complete)
            {
            case COMPLETE:       continue;
            case OUT_OF_MEMORY:  return ~0u;
            default:             unreachable();
            }
         }
      }

      if (!setup_dummy_texture_for_texture_unif(&unifs[i], true, &rs->fmem, &image->sampler))
         return ~0u;
   }

   return num_images;
}

static bool no_depth_updates(const GLXX_SERVER_STATE_T *state, const GLXX_HW_RENDER_STATE_T *rs) {
   if (rs->installed_fb.depth.image == NULL) return true;
   if (!state->depth_mask || !state->caps.depth_test) return true;
   return false;
}

static bool no_stencil_updates(const GLXX_SERVER_STATE_T *state, const GLXX_HW_RENDER_STATE_T *rs) {
   if (rs->installed_fb.stencil.image == NULL) return true;
   if (state->stencil_mask.front == 0 && state->stencil_mask.back == 0) return true;
   if (!state->caps.stencil_test) return true;
   return false;
}

static uint32_t compute_backend_shader_key(const GLXX_SERVER_STATE_T *state,
                                           const GLXX_HW_RENDER_STATE_T *rs,
                                           GLXX_PRIMITIVE_T draw_mode,
                                           GLenum fill_mode)
{
   const GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;
   uint32_t backend_mask = fb->ms ? ~0u : ~(uint32_t)GLSL_SAMPLE_OPS_M;
   uint32_t backend = state->statebits.backend & backend_mask;

   if (!IS_GL_11(state))
   {
      const GLSL_PROGRAM_T *p = gl20_program_common_get(state)->linked_glsl_program;
      draw_mode = glxx_get_rast_draw_mode(p, draw_mode);
   }

   /* Set the primitive type */
   switch (draw_mode)
   {
   case GL_POINTS:
      backend |= GLSL_PRIM_POINT;
      break;
   case GL_LINES:
   case GL_LINE_LOOP:
   case GL_LINE_STRIP:
      backend |= GLSL_PRIM_LINE;
      break;
   default:
      /* Triangles */
      switch (fill_mode)
      {
      case GL_POINT_BRCM:
         backend |= GLSL_PRIM_POINT;
         break;
      case GL_LINE_BRCM:
         backend |= GLSL_PRIM_LINE;
         break;
      case GL_FILL_BRCM:
         /* Do nothing */
         break;
      default:
         unreachable();
      }
   }

   if (rs->z_prepass_allowed) backend |= GLSL_Z_ONLY_WRITE;

   bool activeOcclusion = glxx_server_has_active_query_type(GLXX_Q_OCCLUSION, state, rs);

   if (!activeOcclusion && no_depth_updates(state, rs) && no_stencil_updates(state, rs))
      backend |= GLSL_FEZ_SAFE_WITH_DISCARD;

#if !V3D_VER_AT_LEAST(4,1,34,0)
   if (state->sample_mask.enable && fb->ms) backend |= GLSL_SAMPLE_MASK;
#endif

   for (uint32_t i = 0, cw = state->color_write; i < GLXX_MAX_RENDER_TARGETS; i++, cw >>= 4)
   {
      if ((cw & 0xf) && glxx_fb_is_valid_draw_buf(state->bound_draw_framebuffer, GLXX_COLOR0_ATT + i))
      {
         uint32_t fb_gadget = GLSL_FB_PRESENT;
         if (v3d_tlb_rt_type_use_rw_cfg_16 (fb->color_rt_format[i].type)) fb_gadget |= GLSL_FB_16;
         if (v3d_tlb_rt_type_is_int(fb->color_rt_format[i].type))         fb_gadget |= GLSL_FB_INT;

#if !V3D_VER_AT_LEAST(4,0,2,0)
         const V3D_RT_FORMAT_T *b = &fb->color_rt_format[i];
         if (fmt_requires_alpha16(b->type, b->bpp))
            fb_gadget |= GLSL_FB_ALPHA_16_WORKAROUND;
#endif
         glsl_pack_fb_gadget(&backend, fb_gadget, i);
      }
   }

   backend |= glxx_advanced_blend_eqn(state) << GLSL_ADV_BLEND_S;

#if KHRN_DEBUG
   if (khrn_options.no_ubo_to_unif)
      backend |= GLSL_DISABLE_UBO_FETCH;
#endif

#if !V3D_HAS_SRS_CENTROID_FIX
   if (state->caps.sample_shading)
      backend |= GLSL_SAMPLE_SHADING_ENABLED;
#endif

   return backend;
}

bool glxx_compute_image_like_uniforms(
   GLXX_SERVER_STATE_T *state, glxx_render_state *rs,
   glxx_hw_image_like_uniforms *image_like_uniforms)
{
   unsigned num_tex = compute_texture_uniforms(state, rs, image_like_uniforms);
   if (num_tex == ~0u)
      return false;

   unsigned num_img = compute_image_uniforms(state, rs, image_like_uniforms->img);
   if (num_img == ~0u)
      return false;

   // Zero fill unwritten gadgettype.
 #if !V3D_VER_AT_LEAST(3,3,0,0)
   image_like_uniforms->num_tex = num_tex;
   image_like_uniforms->num_img = num_img;
 #endif
   return true;
}

#if !V3D_VER_AT_LEAST(3,3,0,0)
void glxx_copy_gadgettypes_to_shader_key(GLSL_BACKEND_CFG_T* cfg, const glxx_hw_image_like_uniforms *image_like_uniforms)
{
   static_assrt(countof(cfg->gadgettype) == countof(image_like_uniforms->tex));
   static_assrt(countof(cfg->img_gadgettype) == countof(image_like_uniforms->img));
   memset(cfg->gadgettype, 0, sizeof(cfg->gadgettype));
   memset(cfg->img_gadgettype, 0, sizeof(cfg->img_gadgettype));

   for (unsigned i = 0; i != image_like_uniforms->num_tex; ++i)
      cfg->gadgettype[i] = image_like_uniforms->tex[i].gadgettype;
   for (unsigned i = 0; i != image_like_uniforms->num_img; ++i)
      cfg->img_gadgettype[i] = image_like_uniforms->img[i].gadgettype;
}
#endif

#if V3D_VER_AT_LEAST(4,0,2,0)
static bool blend_cfgs_equal(const glxx_blend_cfg *a, const glxx_blend_cfg *b)
{
   return
      (a->color_eqn == b->color_eqn) &&
      (a->alpha_eqn == b->alpha_eqn) &&
      (a->src_rgb == b->src_rgb) &&
      (a->dst_rgb == b->dst_rgb) &&
      (a->src_alpha == b->src_alpha) &&
      (a->dst_alpha == b->dst_alpha);
}
#endif

static v3d_blend_eqn_t convert_blend_eqn(glxx_blend_eqn_t eq)
{
   assert(eq < GLXX_BLEND_EQN_INVALID);
   return (v3d_blend_eqn_t)eq;
}

static void write_blend_cfg_instr(uint8_t **instr, const glxx_blend_cfg *cfg
#if V3D_VER_AT_LEAST(4,0,2,0)
   , uint32_t rt_mask
#endif
   )
{
   v3d_cl_blend_cfg(instr,
      convert_blend_eqn(cfg->alpha_eqn),
      cfg->src_alpha,
      cfg->dst_alpha,
      convert_blend_eqn(cfg->color_eqn),
      cfg->src_rgb,
      cfg->dst_rgb,
#if V3D_VER_AT_LEAST(4,0,2,0)
      rt_mask,
#endif
      V3D_BLEND_VG_MODE_NORMAL);
}

static void write_blend_cfg(uint8_t **instr,
   const GLXX_HW_RENDER_STATE_T *rs, const GLXX_SERVER_STATE_T *state)
{
#if V3D_VER_AT_LEAST(4,0,2,0)
   uint32_t done_mask = 0;
   for (unsigned i = 0; i != rs->installed_fb.rt_count; ++i)
   {
      if (!(done_mask & (1u << i)))
      {
         uint32_t rt_mask = 1u << i;
         for (unsigned j = i + 1; j != rs->installed_fb.rt_count; ++j)
            if (blend_cfgs_equal(&state->blend.rt_cfgs[i], &state->blend.rt_cfgs[j]))
               rt_mask |= 1u << j;
         if (i == 0)
            /* The blend configs for unused RTs don't really matter, but
             * ensuring they always match the config for the first RT can
             * reduce the number of binned instructions */
            rt_mask |= gfx_mask(V3D_MAX_RENDER_TARGETS - rs->installed_fb.rt_count) << rs->installed_fb.rt_count;

         write_blend_cfg_instr(instr, &state->blend.rt_cfgs[i], rt_mask);
         done_mask |= rt_mask;
      }
   }
   assert(done_mask == gfx_mask(V3D_MAX_RENDER_TARGETS));
#else
   write_blend_cfg_instr(instr, &state->blend.cfg);
#endif
}

static void write_changed_color_write(uint8_t **instr,
   GLXX_HW_RENDER_STATE_T *rs, GLXX_SERVER_STATE_T *state)
{
   if (!khrn_render_state_set_contains(state->dirty.color_write, rs))
      return;

   khrn_render_state_set_remove(&state->dirty.color_write, rs);

   GLXX_CL_RECORD_T *cl_record = glxx_hw_render_state_current_cl_record(rs);
   glxx_cl_record_begin(cl_record, GLXX_CL_STATE_COLOR_WRITE, *instr);

   uint32_t w_disable_mask = ~state->color_write & gfx_mask(V3D_MAX_RENDER_TARGETS * 4);

   /* Disable alpha writes for buffers which don't have alpha channels. We need
    * the alpha in the TLB to be 1 to get correct blending in the case where
    * the buffer doesn't have alpha. The TLB will set alpha to 1 when it loads
    * an alpha-less buffer, but we need to explicitly mask alpha writes after
    * that to prevent it changing. */
   const GLXX_FRAMEBUFFER_T *fb = state->bound_draw_framebuffer;
   unsigned i = 0;
   glxx_att_index_t att_index;
   while (glxx_fb_iterate_valid_draw_bufs(fb, &i, &att_index))
   {
      unsigned b = att_index - GLXX_COLOR0_ATT;
      const GLXX_ATTACHMENT_T *att = glxx_fb_get_attachment_by_index(fb, att_index);
      GFX_LFMT_T api_fmt = glxx_attachment_get_api_fmt(att);
      if (!gfx_lfmt_has_alpha(api_fmt))
         w_disable_mask |= 1u << ((b * 4) + 3);
   }

   v3d_cl_color_wmasks(instr, w_disable_mask);

   glxx_cl_record_end(cl_record, GLXX_CL_STATE_COLOR_WRITE, *instr);
}

/* nothing_to_draw is set to true if the clip region is empty and transform
 * feedback isn't in use.
 * returns false if failed a memory alloc */
static bool do_changed_cfg(
   GLXX_HW_RENDER_STATE_T *rs,
   GLXX_SERVER_STATE_T *state,
   bool *nothing_to_draw,
   GLenum fill_mode)
{
   khrn_fmem *fmem = &rs->fmem;
   const GLXX_HW_FRAMEBUFFER_T *hw_fb = &rs->installed_fb;
   bool line_smooth = false;
   bool rasterizer_discard = state->caps.rasterizer_discard;

   bool depth_test = state->caps.depth_test && (hw_fb->depth.image != NULL);
   v3d_compare_func_t depth_func = depth_test
      ? glxx_hw_convert_test_function(state->depth_func)
      : V3D_COMPARE_FUNC_ALWAYS;
   bool depth_update = depth_test && state->depth_mask;
   bool stencil_test = state->caps.stencil_test && (hw_fb->stencil.image != NULL);

   unsigned max_size = GLXX_CL_STATE_SIZE[GLXX_CL_STATE_VIEWPORT]
                     + GLXX_CL_STATE_SIZE[GLXX_CL_STATE_SAMPLE_STATE]
#if V3D_VER_AT_LEAST(4,0,2,0)
                     + GLXX_CL_STATE_SIZE[GLXX_CL_STATE_BLEND_ENABLES]
#endif
                     + GLXX_CL_STATE_SIZE[GLXX_CL_STATE_BLEND_CFG]
                     + GLXX_CL_STATE_SIZE[GLXX_CL_STATE_BLEND_COLOR]
                     + GLXX_CL_STATE_SIZE[GLXX_CL_STATE_COLOR_WRITE]
                     + GLXX_CL_STATE_SIZE[GLXX_CL_STATE_STENCIL]
                     + GLXX_CL_STATE_SIZE[GLXX_CL_STATE_CFG]
                     + GLXX_CL_STATE_SIZE[GLXX_CL_STATE_POLYGON_OFFSET]
                     + GLXX_CL_STATE_SIZE[GLXX_CL_STATE_LINE_WIDTH];

   // allocate enough space for all the state we're about to change
   uint8_t *instr = khrn_fmem_begin_cle(fmem, max_size);
   if (!instr)
      return false;;

   // grab current CL record
   GLXX_CL_RECORD_T* cl_record = glxx_hw_render_state_current_cl_record(rs);

   *nothing_to_draw = false;

   if (IS_GL_11(state))
      line_smooth = !!(state->gl11.statebits.fragment & GL11_LINESMOOTH);

   if (khrn_render_state_set_contains(state->dirty.viewport, rs))
   {
      khrn_render_state_set_remove(&state->dirty.viewport, rs);

#if V3D_VER_AT_LEAST(4,1,34,0)
      glxx_rect clip = {0, 0, GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE, GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE};
#else
      glxx_rect clip = {0, 0, hw_fb->width, hw_fb->height};
#endif
      glxx_rect_intersect(&clip, &state->viewport);
      if (state->caps.scissor_test)
         glxx_rect_intersect(&clip, &state->scissor);

      bool empty_clip = clip.width == 0 || clip.height == 0;
#if !V3D_VER_AT_LEAST(4,1,34,0)
      if (empty_clip)
      {
         if (!state->transform_feedback.in_use)
         {
            *nothing_to_draw = true;
            goto end;
         }
         else
         {
            /* Empty clip rectangle, but transform feedback in use, so we have to draw anyway.
             * Can't pass an empty clip rectangle to HW, so create a small rectangle and enable
             * both front and back face culling.
            */
            rasterizer_discard = true;
            clip.width = 1;
            clip.height = 1;
         }
      }
#endif

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_VIEWPORT, instr);

      v3d_cl_clip(&instr, clip.x, clip.y, clip.width, clip.height);

      // Don't bother with the rest of the state if clip window is empty --
      // prims won't get far enough down the pipeline for it to matter
      if (!empty_clip)
      {
         v3d_cl_clipper_xy(&instr,
            state->vp_internal.xscale,
            state->vp_internal.yscale);

         v3d_cl_viewport_offset_from_rect(&instr,
            state->viewport.x, state->viewport.y,
            state->viewport.width, state->viewport.height);

         v3d_cl_clipper_z(&instr,
            state->vp_internal.zscale,
            state->vp_internal.zoffset);

         v3d_cl_clipz(&instr,
            fminf(state->depth_range.z_near, state->depth_range.z_far),
            fmaxf(state->depth_range.z_near, state->depth_range.z_far));
      }

      glxx_cl_record_end(cl_record, GLXX_CL_STATE_VIEWPORT, instr);
   }

   if (khrn_render_state_set_contains(state->dirty.sample_state, rs))
   {
      khrn_render_state_set_remove(&state->dirty.sample_state, rs);

      float sample_coverage = 1.0f; /* Disable */
      if (state->sample_coverage.enable && rs->installed_fb.ms)
      {
         /* Invert sets the sign bit. Can't write using floats because -0 == 0 */
         uint32_t cov_val = gfx_float_to_bits(state->sample_coverage.value);
         if (state->sample_coverage.invert) cov_val |= (1<<31);
         sample_coverage = gfx_float_from_bits(cov_val);
      }

#if V3D_VER_AT_LEAST(4,1,34,0)
      uint8_t mask = 0xf;     /* Disable */
      if (state->sample_mask.enable && rs->installed_fb.ms)
         mask = state->sample_mask.mask[0] & 0xf;
#endif

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_SAMPLE_STATE, instr);
#if V3D_VER_AT_LEAST(4,1,34,0)
      v3d_cl_sample_state(&instr, mask, sample_coverage);
#else
      v3d_cl_sample_state(&instr, sample_coverage);
#endif
      glxx_cl_record_end(cl_record, GLXX_CL_STATE_SAMPLE_STATE, instr);
   }

#if V3D_VER_AT_LEAST(4,0,2,0)
   if (khrn_render_state_set_contains(state->dirty.blend_enables, rs))
   {
      khrn_render_state_set_remove(&state->dirty.blend_enables, rs);

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_BLEND_ENABLES, instr);
      v3d_cl_blend_enables(&instr, state->blend.rt_enables);
      glxx_cl_record_end(cl_record, GLXX_CL_STATE_BLEND_ENABLES, instr);
   }
#endif

   if (khrn_render_state_set_contains(state->dirty.blend_cfg, rs))
   {
      khrn_render_state_set_remove(&state->dirty.blend_cfg, rs);

      if (!glxx_advanced_blend_eqn_set(state))
      {
         glxx_cl_record_begin(cl_record, GLXX_CL_STATE_BLEND_CFG, instr);
         write_blend_cfg(&instr, rs, state);
         glxx_cl_record_end(cl_record, GLXX_CL_STATE_BLEND_CFG, instr);
      }
   }

   if (khrn_render_state_set_contains(state->dirty.blend_color, rs))
   {
      khrn_render_state_set_remove(&state->dirty.blend_color, rs);

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_BLEND_COLOR, instr);
      v3d_cl_blend_ccolor(&instr,
         state->blend_color[0],
         state->blend_color[1],
         state->blend_color[2],
         state->blend_color[3]);
      glxx_cl_record_end(cl_record, GLXX_CL_STATE_BLEND_COLOR, instr);
   }

   write_changed_color_write(&instr, rs, state);

   if (stencil_test && khrn_render_state_set_contains(state->dirty.stencil, rs))
   {
      khrn_render_state_set_remove(&state->dirty.stencil, rs);

      int max = ((1 << glxx_get_stencil_size(state)) - 1);
      int fref = gfx_sclamp(state->stencil_func.front.ref, 0, max);
      int bref = gfx_sclamp(state->stencil_func.back.ref, 0, max);

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_STENCIL, instr);

      /* TODO: Could optimise to only write one config when one hasn't changed
       * or when the new settings match */
      v3d_cl_stencil_cfg(&instr, fref, state->stencil_func.front.mask & 0xff,
            translate_stencil_func(state->stencil_func.front.func),
            translate_stencil_op(state->stencil_op.front.fail),
            translate_stencil_op(state->stencil_op.front.zfail),
            translate_stencil_op(state->stencil_op.front.zpass),
            true, false,     /* Only front config */
            state->stencil_mask.front & 0xff);

      v3d_cl_stencil_cfg(&instr, bref, state->stencil_func.back.mask & 0xff,
            translate_stencil_func(state->stencil_func.back.func),
            translate_stencil_op(state->stencil_op.back.fail),
            translate_stencil_op(state->stencil_op.back.zfail),
            translate_stencil_op(state->stencil_op.back.zpass),
            false, true,     /* Only back config */
            state->stencil_mask.back & 0xff);

      glxx_cl_record_end(cl_record, GLXX_CL_STATE_STENCIL, instr);
   }

   /* TODO It might be a worthwhile optimisation to put this in an if (state->changed.ez) */
   if (glxx_ez_update_cfg(&rs->ez,
      depth_func, depth_update,
      stencil_test,
      translate_stencil_op(state->stencil_op.front.fail),
      translate_stencil_op(state->stencil_op.front.zfail),
      translate_stencil_op(state->stencil_op.back.fail),
      translate_stencil_op(state->stencil_op.back.zfail)))
   {
      khrn_render_state_set_add(&state->dirty.cfg, rs);
   }

   /* If we have random forced wireframe, always force out a cfg_bits item;
      it's likely to have changed anyway, and it is too much complexity to
      make it worth tracking if the resultant fill_mode has actually changed */
   if (khrn_render_state_set_contains(state->dirty.cfg, rs) || khrn_options.random_wireframe)
   {
      khrn_render_state_set_remove(&state->dirty.cfg, rs);

      V3D_CL_CFG_BITS_T cfg_bits;
      bool ms = khrn_options.force_multisample || (state->caps.multisample && hw_fb->ms);

      /* Switch blending off if logic_op is enabled */
      cfg_bits.blend = !(state->gl11.statebits.f_enable & GL11_LOGIC_M);
      /* Switch blending off if advanced blending is enabled */
      cfg_bits.blend = cfg_bits.blend && !glxx_advanced_blend_eqn_set(state);
#if !V3D_VER_AT_LEAST(4,0,2,0)
      cfg_bits.blend = cfg_bits.blend && state->blend.enable;
#endif

      cfg_bits.stencil = stencil_test;

      cfg_bits.front_prims = !state->caps.cull_face || enable_front(state->cull_mode);
      cfg_bits.back_prims  = !state->caps.cull_face || enable_back(state->cull_mode);
      if (rasterizer_discard)
         cfg_bits.front_prims = cfg_bits.back_prims = false;

      cfg_bits.cwise_is_front = glxx_front_facing_is_clockwise(state->front_face);

      cfg_bits.depth_offset      = state->caps.polygon_offset_fill;
      cfg_bits.rast_oversample   = ms ? V3D_MS_4X : V3D_MS_1X; // TODO 16x?
      cfg_bits.depth_test        = depth_func;
      cfg_bits.depth_update      = depth_update;
      cfg_bits.ez                = rs->ez.cfg_bits_ez;
      cfg_bits.ez_update         = rs->ez.cfg_bits_ez_update;
      cfg_bits.aa_lines          = cfg_bits.rast_oversample;
      cfg_bits.wireframe_tris    = (fill_mode != GL_FILL_BRCM);
      cfg_bits.wireframe_mode    = (fill_mode == GL_POINT_BRCM) ? V3D_WIREFRAME_MODE_POINTS : V3D_WIREFRAME_MODE_LINES;
      cfg_bits.cov_pipe          = false;
      cfg_bits.cov_update        = V3D_COV_UPDATE_NONZERO;
      cfg_bits.d3d_prov_vtx      = state->provoking_vtx == GL_FIRST_VERTEX_CONVENTION_BRCM;

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_CFG, instr);
      v3d_cl_cfg_bits_indirect(&instr, &cfg_bits);
      glxx_cl_record_end(cl_record, GLXX_CL_STATE_CFG, instr);
   }

   if (khrn_render_state_set_contains(state->dirty.polygon_offset, rs))
   {
      khrn_render_state_set_remove(&state->dirty.polygon_offset, rs);

      GFX_LFMT_T depth_lfmt = glxx_attachment_get_api_fmt(
         &state->bound_draw_framebuffer->attachment[GLXX_DEPTH_ATT]);

      unsigned depth_bits = depth_lfmt != GFX_LFMT_NONE ? gfx_lfmt_depth_bits(depth_lfmt) : 0;

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_POLYGON_OFFSET, instr);
      v3d_cl_set_depth_offset(&instr, state->polygon_offset.factor, state->polygon_offset.units,
#if V3D_VER_AT_LEAST(4,1,34,0)
                                      state->polygon_offset.limit,
#endif
                                      depth_bits);
      glxx_cl_record_end(cl_record, GLXX_CL_STATE_POLYGON_OFFSET, instr);
   }

   if (khrn_render_state_set_contains(state->dirty.linewidth, rs))
   {
      khrn_render_state_set_remove(&state->dirty.linewidth, rs);

      /* For antialiasing we need to rasterise a bigger line because width is measured differently. */
      /* We might need a width of up to floor( sqrt(2) * width ) + 3 */
      float line_width;
      if (line_smooth)
         line_width = (float)floor(M_SQRT2 * state->line_width) + 3;
      else
         line_width = state->line_width;

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_LINE_WIDTH, instr);
      v3d_cl_line_width(&instr, line_width);
      glxx_cl_record_end(cl_record, GLXX_CL_STATE_LINE_WIDTH, instr);
   }

#if !V3D_VER_AT_LEAST(4,1,34,0)
end:
#endif
   // end write to cl (pass in actual end point)
   khrn_fmem_end_cle(fmem, instr);

   return true;
}

static v3d_cl_flag_set_func flags_set(glxx_cl_state_t state) {
   switch (state) {
      case GLXX_CL_STATE_FLAT_SHADING_FLAGS:  return v3d_cl_flatshade_flags;
      case GLXX_CL_STATE_CENTROID_FLAGS:      return v3d_cl_centroid_flags;
#if V3D_VER_AT_LEAST(4,1,34,0)
      case GLXX_CL_STATE_NOPERSPECTIVE_FLAGS: return v3d_cl_noperspective_flags;
#endif
      default: unreachable(); return NULL;
   }
}

static v3d_cl_flag_zero_func flags_zero(glxx_cl_state_t state) {
   switch (state) {
      case GLXX_CL_STATE_FLAT_SHADING_FLAGS:  return v3d_cl_zero_all_flatshade_flags;
      case GLXX_CL_STATE_CENTROID_FLAGS:      return v3d_cl_zero_all_centroid_flags;
#if V3D_VER_AT_LEAST(4,1,34,0)
      case GLXX_CL_STATE_NOPERSPECTIVE_FLAGS: return v3d_cl_zero_all_noperspective_flags;
#endif
      default: unreachable(); return NULL;
   }
}

static bool update_vary_flags(GLXX_HW_RENDER_STATE_T *rs, const uint32_t *flags, uint32_t *prev_flags,
                              bool *set, glxx_cl_state_t flag_type)
{
   if (!*set || memcmp(prev_flags, flags, V3D_MAX_VARY_FLAG_WORDS * sizeof(uint32_t)))
   {
      uint8_t *instr = glxx_hw_render_state_begin_cle(rs, flag_type);
      if (!instr)
         return false;;

      *set = true;
      memcpy(prev_flags, flags, V3D_MAX_VARY_FLAG_WORDS * sizeof(uint32_t));

      v3d_cl_write_vary_flags(&instr, flags, flags_set(flag_type), flags_zero(flag_type));

      glxx_hw_render_state_end_cle(rs, flag_type, instr);
   }
   return true;
}

static bool insert_flat_shading_flags(GLXX_HW_RENDER_STATE_T *rs, const GLXX_LINK_RESULT_DATA_T *link_data)
{
   return update_vary_flags(rs, link_data->varying_flat, rs->prev_flat_shading_flags,
                            &rs->flat_shading_flags_set, GLXX_CL_STATE_FLAT_SHADING_FLAGS);
}

#if V3D_VER_AT_LEAST(4,1,34,0)
static bool insert_noperspective_flags(GLXX_HW_RENDER_STATE_T *rs, const GLXX_LINK_RESULT_DATA_T *link_data)
{
   return update_vary_flags(rs, link_data->varying_noperspective, rs->prev_noperspective_flags,
                            &rs->noperspective_flags_set, GLXX_CL_STATE_NOPERSPECTIVE_FLAGS);
}
#endif

static bool insert_centroid_flags(GLXX_HW_RENDER_STATE_T *rs, const GLXX_LINK_RESULT_DATA_T *link_data)
{
   return update_vary_flags(rs, link_data->varying_centroid, rs->prev_centroid_flags,
                            &rs->centroid_flags_set, GLXX_CL_STATE_CENTROID_FLAGS);
}

bool glxx_is_stencil_updated(GLXX_SERVER_STATE_T *state) {
   bool update_front = state->stencil_mask.front != 0 && ( state->stencil_op.front.fail  != GL_KEEP ||
                                                           state->stencil_op.front.zfail != GL_KEEP ||
                                                           state->stencil_op.front.zpass != GL_KEEP   );
   bool update_back = state->stencil_mask.back != 0 && ( state->stencil_op.back.fail   != GL_KEEP ||
                                                         state->stencil_op.back.zfail  != GL_KEEP ||
                                                         state->stencil_op.back.zpass  != GL_KEEP   );
   return update_front || update_back;
}

static bool glxx_hw_draw_arrays_record(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs,
      const glxx_hw_draw *draw,
      const GLXX_LINK_RESULT_DATA_T *link_data)
{
   int size = (draw->instance_count > 1) ?
      V3D_CL_VERTEX_ARRAY_INSTANCED_PRIMS_SIZE :
      V3D_CL_VERTEX_ARRAY_PRIMS_SIZE;
   v3d_prim_mode_t mode = convert_primitive_type(state, draw->mode);

   assert(draw->is_draw_arrays);
   assert(draw->instance_count >= 1);

   if (draw->baseinstance)
      size += V3D_CL_BASE_VERTEX_BASE_INSTANCE_SIZE;

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, size);
   if (!instr) return false;

   if (draw->baseinstance)
      v3d_cl_base_vertex_base_instance(&instr, 0, draw->baseinstance);

   if (draw->instance_count > 1)
      v3d_cl_vertex_array_instanced_prims(&instr, mode, draw->count,
            draw->instance_count, draw->first);
   else
      v3d_cl_vertex_array_prims(&instr, mode, draw->count, draw->first);

   khrn_fmem_end_cle_exact(&rs->fmem, instr);

   return true;
}

static bool glxx_hw_draw_elements_record(GLXX_SERVER_STATE_T *state,
   GLXX_HW_RENDER_STATE_T *rs,
   const glxx_hw_draw *draw,
#if !V3D_VER_AT_LEAST(4,1,34,0)
   const glxx_attribs_max *attribs_max,
#endif
   const glxx_hw_indices *indices)
{
   v3d_prim_mode_t mode = convert_primitive_type(state, draw->mode);
   v3d_index_type_t index_type = convert_index_type(draw->index_type);
   int size = (draw->instance_count > 1) ?
      V3D_CL_INDEXED_INSTANCED_PRIM_LIST_SIZE : V3D_CL_INDEXED_PRIM_LIST_SIZE;

   if (draw->basevertex || draw->baseinstance)
      size += V3D_CL_BASE_VERTEX_BASE_INSTANCE_SIZE;

   assert(draw->instance_count >= 1);

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, size);
   if (!instr)
      return false;

   if (draw->basevertex || draw->baseinstance)
      v3d_cl_base_vertex_base_instance(&instr,
#if V3D_VER_AT_LEAST(4,0,2,0)
         draw->basevertex,
#else
         // Partial workaround for GFXH-1692
         gfx_sext(draw->basevertex, 25),
#endif
         draw->baseinstance);

   if (draw->instance_count > 1)
   {
      V3D_CL_INDEXED_INSTANCED_PRIM_LIST_T indexed_instanced_prim_list = {
         .prim_mode = mode,
         .index_type = index_type,
         .num_indices = draw->count,
         .prim_restart = state->caps.primitive_restart && draw->mode != GL_PATCHES,
         .num_instances = draw->instance_count,
#if V3D_VER_AT_LEAST(4,1,34,0)
         .indices_offset = indices->offset,
#else
         .indices_addr = indices->addr + indices->offset,
         .max_index = attribs_max->index
#endif
      };
      v3d_cl_indexed_instanced_prim_list_indirect(&instr, &indexed_instanced_prim_list);
   }
   else
   {
      V3D_CL_INDEXED_PRIM_LIST_T indexed_prim_list = {
         .prim_mode = mode,
         .index_type = index_type,
         .num_indices = draw->count,
         .prim_restart = state->caps.primitive_restart && draw->mode != GL_PATCHES,
#if V3D_VER_AT_LEAST(4,1,34,0)
         .indices_offset = indices->offset,
#else
         .indices_addr = indices->addr + indices->offset,
         .max_index = attribs_max->index
#endif
      };
      v3d_cl_indexed_prim_list_indirect(&instr, &indexed_prim_list);
   }

   khrn_fmem_end_cle_exact(&rs->fmem, instr);

   return true;
}

static bool glxx_hw_draw_indirect_record(
   GLXX_SERVER_STATE_T *state,
   GLXX_HW_RENDER_STATE_T *rs,
   const glxx_hw_draw *draw,
#if !V3D_VER_AT_LEAST(4,1,34,0)
   const glxx_attribs_max *attribs_max,
#endif
   const glxx_hw_indices *indices,
   v3d_addr_t indirect_addr)
{
   assert(draw->num_indirect > 0); /* Should not get this far if num_indirect is 0! */

   v3d_prim_mode_t mode = convert_primitive_type(state, draw->mode);
   unsigned size = 0;

   // TODO v3d_cl_indirect_primitive_limits() can be omitted if buffers have not changed
#if !V3D_VER_AT_LEAST(4,1,34,0)
   size += V3D_CL_INDIRECT_PRIMITIVE_LIMITS_SIZE;
#endif

   if (draw->is_draw_arrays)
      size += V3D_CL_INDIRECT_VERTEX_ARRAY_PRIMS_SIZE;
   else
      size += V3D_CL_INDIRECT_INDEXED_PRIM_LIST_SIZE;

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, size);
   if (!instr)
      return false;

   if (!draw->is_draw_arrays)
   {
      assert(indices->offset == 0); //gl api drawelementsindirect does not specify offset in index buffer
      v3d_index_type_t index_type = convert_index_type(draw->index_type);
#if !V3D_VER_AT_LEAST(4,1,34,0)
      unsigned type_size = glxx_get_index_type_size(draw->index_type);
      v3d_cl_indirect_primitive_limits(&instr, attribs_max->index,
            attribs_max->instance, indices->size / type_size);
#endif
      v3d_cl_indirect_indexed_prim_list(
         &instr, mode, index_type, draw->num_indirect, state->caps.primitive_restart && draw->mode != GL_PATCHES,
         indirect_addr,
#if !V3D_VER_AT_LEAST(4,1,34,0)
         indices->addr,
#endif
         draw->indirect_stride);

   }
   else
   {
#if !V3D_VER_AT_LEAST(4,1,34,0)
      v3d_cl_indirect_primitive_limits(&instr, attribs_max->index,
            attribs_max->instance, 0);
#endif
      v3d_cl_indirect_vertex_array_prims(&instr, mode, draw->num_indirect, indirect_addr, draw->indirect_stride);
   }

   khrn_fmem_end_cle_exact(&rs->fmem, instr);

   return true;
}

/*!
 * \brief Processes a batch of primitives and bins them.
 *
 * This function is called for each batch of primitives in the scene. It first performs
 * per-batch tasks like setting up the shaders and uniforms and then runs the binning
 * pipeline, binning the primitives into the according tile lists.
 */
bool glxx_hw_draw_triangles(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs,
      const glxx_hw_draw *draw,
      const glxx_hw_indices *indices,
      const GLXX_ATTRIB_CONFIG_T *attribs, const glxx_hw_vb *vbs
#if !V3D_VER_AT_LEAST(4,1,34,0)
      , const glxx_attribs_max *attribs_max
#endif
      )
{
   if (!rs->clist_start && !(rs->clist_start = khrn_fmem_begin_clist(&rs->fmem)))
      return false;

   /* compute texture/image unifs, create or retrieve shaders from cache */
   glxx_hw_image_like_uniforms image_like_uniforms;
   GLXX_LINK_RESULT_DATA_T* link_data;
   GLenum fill_mode = state->fill_mode;
   /* modify (locally) fill_mode with khrn_options_make_wireframe() */
   if (khrn_options_make_wireframe() && (fill_mode == GL_FILL_BRCM))
      fill_mode = khrn_options_make_wireframe_points() ? GL_POINT_BRCM : GL_LINE_BRCM;
   {
      if (!glxx_compute_image_like_uniforms(state, &rs->base, &image_like_uniforms))
         goto fail;

      GLSL_BACKEND_CFG_T key;
      key.backend = compute_backend_shader_key(state, rs, draw->mode, fill_mode);
    #if !V3D_VER_AT_LEAST(3,3,0,0)
      glxx_copy_gadgettypes_to_shader_key(&key, &image_like_uniforms);
    #endif
      link_data = glxx_get_shaders(state, &key);
      if (!link_data)
         goto fail;
   }

#if !V3D_VER_AT_LEAST(3,3,0,0)
   /* If using flow control, then driver needs to work around GFXH-1181. */
   rs->fmem.br_info.details.bin_workaround_gfxh_1181 |= link_data->bin_uses_control_flow;
   rs->fmem.br_info.details.render_workaround_gfxh_1181 |= link_data->render_uses_control_flow;

   /* For z-prepass the bin-mode shaders are also used during rendering */
   if (rs->z_prepass_allowed) rs->fmem.br_info.details.render_workaround_gfxh_1181 |= link_data->bin_uses_control_flow;
#endif

   /* Write the shader record */
   bool ok = write_gl_shader_record(rs, state, link_data, &image_like_uniforms, attribs, vbs);
   if (!ok)
   {
      log_warn("%s: creating shader record failed", __FUNCTION__);
      goto fail;
   }

   // update z-prepass state
   if (!glxx_hw_update_z_prepass_state(state, link_data, rs))
      goto fail;

   /* emit any necessary config change instructions */
   {
      bool nothing_to_draw = false;
      ok = do_changed_cfg(rs, state, &nothing_to_draw, fill_mode);
      ok &= insert_flat_shading_flags(rs, link_data);
      ok &= insert_centroid_flags(rs, link_data);
#if V3D_VER_AT_LEAST(4,1,34,0)
      ok &= insert_noperspective_flags(rs, link_data);
#endif
      if (!ok)
      {
         log_warn("%s: applying state changes failed", __FUNCTION__);
         goto fail;
      }
      if (nothing_to_draw)
      {
         log_trace("%s: nothing to draw", __FUNCTION__);
         return true;
      }
   }

   if (!glxx_server_queries_install(state, rs))
      goto fail;

   bool point_size_used = (link_data->flags & GLXX_SHADER_FLAGS_POINT_SIZE_SHADED_VERTEX_DATA) ==
      GLXX_SHADER_FLAGS_POINT_SIZE_SHADED_VERTEX_DATA;
   if (!glxx_server_tf_install(state, rs, point_size_used))
      goto fail;

#if !V3D_VER_AT_LEAST(4,0,2,0)
   if (state->transform_feedback.in_use)
   {
      GLXX_TRANSFORM_FEEDBACK_T *tf = state->transform_feedback.bound;
      const GLXX_PROGRAM_TFF_POST_LINK_T *ptf = &gl20_program_common_get(state)->transform_feedback;
      unsigned prim_count = glxx_tf_update_stream_pos(tf, ptf,
            draw->count, draw->instance_count);

      GLXX_QUERY_T *query = state->queries.queries[GLXX_Q_PRIM_WRITTEN].active;
      if (query)
         query->result += prim_count;
   }
#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
   if (!draw->is_draw_arrays)
   {
      //setup index buffer
      uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, V3D_CL_INDEX_BUFFER_SETUP_SIZE);
      if (!instr)
         return false;
      v3d_cl_index_buffer_setup(&instr, indices->addr, indices->size);
      khrn_fmem_end_cle_exact(&rs->fmem, instr);
   }
#endif

#if !V3D_VER_AT_LEAST(3,3,0,0)
   // Instanced renders on 3.2 might encounter GFXH-1313.
   if (draw->instance_count > 1 || draw->is_indirect)
      rs->workaround_gfxh_1313 = true;
#endif

#if GLXX_HAS_TNG
   if (!IS_GL_11(state))
   {
      IR_PROGRAM_T *ir = gl20_program_common_get(state)->linked_glsl_program->ir;
      rs->num_used_layers =  gfx_umax(rs->num_used_layers,
            gfx_umin(rs->installed_fb.layers, ir->max_known_layers));
   }
#endif

   if (!draw->is_indirect)
   {
      if (!(draw->is_draw_arrays ?
            glxx_hw_draw_arrays_record(state, rs, draw, link_data) :
            glxx_hw_draw_elements_record(state, rs, draw,
#if !V3D_VER_AT_LEAST(4,1,34,0)
               attribs_max,
#endif
               indices)))
         goto fail;
   }
   else
   {
      khrn_resource *res = state->bound_buffer[GLXX_BUFTGT_DRAW_INDIRECT].obj->resource;
      if (!glxx_hw_tf_aware_sync_res(rs, res, V3D_BARRIER_CLE_DRAWREC_READ, V3D_BARRIER_NO_ACCESS))
         goto fail;
      if (!glxx_hw_draw_indirect_record(
            state, rs, draw,
#if !V3D_VER_AT_LEAST(4,1,34,0)
            attribs_max,
#endif
            indices, gmem_get_addr(res->handle) + draw->indirect_offset))
         goto fail;
   }

   glxx_server_tf_install_post_draw(state, rs);

   /* Update buffer states with rw operation */
   glxx_hw_render_state_rw(state, rs);

   if (draw->mode == GL_TRIANGLES || draw->mode == GL_TRIANGLE_STRIP ||
         draw->mode == GL_TRIANGLE_FAN)
   {
      khrn_render_state_set_remove(&state->dirty.stuff, rs);
   }

   // If potentially doing multicore binning, save current GPU state following
   // this draw. CL may be split at this point for multicore binning.
   if ((glxx_hw_render_state_max_bin_subjobs(rs) > 1) &&
      // Do *not* do this between glBeginTransformFeedback &
      // glEndTransformFeedback as transform feedback state is not saved by
      // post_draw_journal
      !glxx_tf_is_active(glxx_get_bound_tf(state)) &&
#if V3D_VER_AT_LEAST(4,0,2,0)
      // Similarly, do not do this if any prim count queries are active -- this
      // state is not saved either
      !state->queries.queries[GLXX_Q_PRIM_GEN].active &&
      !state->queries.queries[GLXX_Q_PRIM_WRITTEN].active &&
#endif
      !post_draw_journal(rs, draw, link_data, attribs, vbs))
      goto fail;

   return true;

fail:
   glxx_hw_discard_frame(rs);
   return false;
}

/*************************************************************
 Static Functions
 *************************************************************/

static uint64_t estimate_bin_cost(
   const glxx_hw_draw *draw,
   const GLXX_LINK_RESULT_DATA_T *link_data,
   const GLXX_ATTRIB_CONFIG_T *attribs, const glxx_hw_vb *vbs)
{
   // Hardware limits
   uint32_t const c_instr_per_cycle = khrn_get_num_qpus_per_core()*4;
   uint32_t const c_bytes_per_cycle = 16;
   // VCD is also limited (per-cycle) to
   // - 2 attribute-vectors
   // - 1 cache-line check (but can reuse check from previous attribute)
   // But we don't take these limitations into account here.

   // estimate bandwidth used with following caveats:
   // - assume whole vertex is fetched in each buffer (even unused attributes)
   // - ignore effect of VCD cache and cache-line size (so we assume fetching whole vertex for each vertex processed)
   uint32_t vertex_size = 0;
   uint32_t attrib_remaining = (1 << link_data->attr_count) - 1;
   while (attrib_remaining)
   {
      uint32_t this_vbo = UINT32_MAX;
      uint32_t this_stride = 0;

      for (unsigned n = 0; n != link_data->attr_count; ++n)
      {
         if (link_data->attr[n].c_scalars_used > 0 && attrib_remaining & (1 << n))
         {
            const int i = link_data->attr[n].idx;
            if (attribs[i].enabled)
            {
               if (this_vbo == UINT32_MAX)
               {
                  this_vbo = attribs[i].vbo_index;
                  this_stride = vbs[i].stride;
               }
               else if (this_vbo != attribs[i].vbo_index || this_stride != vbs[i].stride)
               {
                  // leave till next pass
                  continue;
               }
            }
         }
         // handled this attribute
         attrib_remaining &= ~(1 << n);
      }
      vertex_size += this_stride;
   }

   // consider size of index data
   uint32_t index_size;
   if (draw->is_draw_arrays)
      index_size = 0;
   else switch (draw->index_type)
   {
   case GL_UNSIGNED_BYTE:  index_size = 1; break;
   case GL_UNSIGNED_SHORT: index_size = 2; break;
   case GL_UNSIGNED_INT:   index_size = 4; break;
   default:                unreachable();
   }

   // estimate number of vertices processed
   uint64_t num_indices = draw->is_indirect ? 1000 : /* TODO See SWVC5-186 */
      ((uint64_t)draw->count * draw->instance_count);
   uint64_t est_num_vertices = num_indices;
   if (!draw->is_draw_arrays)
   {
      if (draw->mode == GL_TRIANGLES)
      {
         // for indexed tri-list, assume index to vertex ratio of 0.625
         est_num_vertices = (est_num_vertices * 5) / 8;
      }
      else if (draw->mode == GL_TRIANGLE_STRIP)
      {
         // for indexed tri-strip, strip is typically 60-75% (use ~70%) size of equivalent tri-list
         est_num_vertices = (est_num_vertices * 7) / 8;
      }
   }

   // estimate number of cycles for draw
   uint64_t bandwidth_cycles = (vertex_size*est_num_vertices + index_size*num_indices) / c_bytes_per_cycle;
   uint64_t instr_cycles = (link_data->num_bin_qpu_instructions * est_num_vertices) / c_instr_per_cycle;
   uint64_t bin_cost = gfx_umax64(bandwidth_cycles, instr_cycles);
   return bin_cost;
}

static bool post_draw_journal(
   GLXX_HW_RENDER_STATE_T *rs,
   const glxx_hw_draw *draw,
   const GLXX_LINK_RESULT_DATA_T *link_data,
   const GLXX_ATTRIB_CONFIG_T *attribs, const glxx_hw_vb *vbs)
{
   // should only call this if multi-core bin is compiled in
   assert(GLXX_MULTICORE_BIN_ENABLED);

   // get estimated cost of bin in cycles
   uint64_t const bin_cost = estimate_bin_cost(draw, link_data, attribs, vbs);

   // increment bin-cost in current draw record
   GLXX_CL_RECORD_T* cl_record = &rs->cl_records[rs->num_cl_records];
   cl_record->bin_cost_cumulative += bin_cost;

   // validate current record
   assert(glxx_cl_record_validate(cl_record));

   if (rs->cl_record_remaining <= bin_cost)
   {
      // add nop we can patch later
      static_assrt(V3D_CL_NOP_SIZE == V3D_CL_RETURN_SIZE);
      uint8_t* instr = khrn_fmem_begin_cle(&rs->fmem, V3D_CL_NOP_SIZE);
      if (!instr)
         return false;
      cl_record->ret_sub_ptr = instr;
      v3d_cl_nop(&instr);
      khrn_fmem_end_cle_exact(&rs->fmem, instr);

      cl_record->start_sub_addr = khrn_fmem_cle_addr(&rs->fmem);

      // start new record, compact and adjust record threshold if out of space
      unsigned int num_cl_records = rs->num_cl_records + 1;
      if (num_cl_records == GLXX_MAX_CL_RECORDS)
      {
         static_assrt(!GLXX_MULTICORE_BIN_ENABLED || (GLXX_MAX_CL_RECORDS & 1) == 0);
         for (unsigned int i = 0; i != GLXX_MAX_CL_RECORDS/2; ++i)
         {
            rs->cl_records[i] = rs->cl_records[i*2 + 1];
         }
         num_cl_records /= 2;
         rs->cl_record_threshold *= 2;
      }
      rs->num_cl_records = num_cl_records;

      // copy state from last record to upcoming record
      rs->cl_records[num_cl_records] = rs->cl_records[num_cl_records-1];
      rs->cl_record_remaining = rs->cl_record_threshold;

      // capture z-only state at the beginning of this record
      rs->cl_records[num_cl_records].z_prepass_stopped = rs->z_prepass_stopped;
   }
   else
   {
      rs->cl_record_remaining -= bin_cost;
   }
   return true;
}

GLXX_LINK_RESULT_DATA_T *glxx_get_shaders(
   GLXX_SERVER_STATE_T *state, const GLSL_BACKEND_CFG_T *key)
{
   GLXX_BINARY_CACHE_T *cache;
   IR_PROGRAM_T *ir;
   if (IS_GL_11(state))
   {
      /* Get the ES1 equivalent of a program from the cache */
      GL11_CACHE_ENTRY_T *e = gl11_get_backend_cache(state->gl11.shader_cache, &state->gl11.shaderkey);
      cache = &e->backend_cache;
      ir = e->blob;
   }
   else
   {
      GL20_PROGRAM_COMMON_T *program_common = gl20_program_common_get(state);
      cache = &program_common->cache;
      ir = program_common->linked_glsl_program->ir;
   }
   assert(ir);

   /* Try to get a previous binary from the cache */
   GLXX_LINK_RESULT_DATA_T *result = glxx_binary_cache_get_shaders(cache, key);

   if (result == NULL)
   {
      /* If the cache lookup failed, make a new shader and put it in the cache */
      result = glxx_get_shaders_and_cache(cache, ir, key);
      if (result == NULL)
         log_warn("[%s] shader compilation failed for %p", __FUNCTION__, state);
   }

   return result;
}

static uint32_t assemble_tmu_param(uint32_t param, uint32_t u_value,
                                   const GLXX_TEXTURE_UNIF_T *tus)
{
   assert(param == 0 || param == 1);

#if V3D_VER_AT_LEAST(4,0,2,0)
   uint32_t sampler    = backend_uniform_get_sampler(u_value);
   uint32_t extra_bits = backend_uniform_get_extra(u_value);
   uint32_t bits       = tus[sampler].hw_param[param];

   assert(!(bits & extra_bits));
   return (bits | extra_bits);
#else
   uint32_t sampler = backend_uniform_get_sampler(u_value);
   uint32_t bits;
   uint32_t extra_bits;

   if (param == 0) {
      bits = tus[sampler].hw_param[0];

# if !V3D_VER_AT_LEAST(3,3,1,0)
      // Workaround GFXH-1371 if required by clearing the pix_mask bit
      if (tus[sampler].force_no_pixmask)
         u_value &= ~GLSL_TEXPARAM0_PIX_MASK;
# endif

      extra_bits = backend_uniform_get_extra_param0(u_value);
   } else {
      if (u_value & GLSL_TEXPARAM1_FETCH) {
         assert(!(u_value & GLSL_TEXPARAM1_GATHER));
         bits = tus[sampler].hw_param1_fetch;
      } else if (u_value & GLSL_TEXPARAM1_GATHER) {
         int comp = (u_value & GLSL_TEXPARAM1_GATHER_COMP_MASK) >> GLSL_TEXPARAM1_GATHER_COMP_SHIFT;
         bits = tus[sampler].hw_param1_gather[comp];
      } else if (u_value & GLSL_TEXPARAM1_NO_ANISO) {
         bits = tus[sampler].hw_param1_no_aniso;
      } else
         bits = tus[sampler].hw_param[1];

      extra_bits = backend_uniform_get_extra_param1(u_value);
   }

   assert(!(bits & extra_bits));
   return (bits | extra_bits);
#endif
}

static uint32_t get_dynamic_array_length(
   const GLXX_INDEXED_BINDING_POINT_T *binding,
   const gl20_program_dynamic_array* array)
{
   size_t binding_size = glxx_indexed_binding_point_get_size(binding);
   binding_size = binding_size > binding->offset ? binding_size - binding->offset : 0;

   // Should be using v3d_size_t internally...
   assert(binding_size <= ~(v3d_size_t)0u);
   v3d_size_t size = (v3d_size_t)binding_size;

   // Get size of buffer from dynamic array offset.
   assert(array->stride != 0);
   v3d_size_t length = size > array->offset ? (size - array->offset) / array->stride : 0;

   // We should check the block fits in the buffer earlier.
   assert(length != 0);

   static_assrt(sizeof(v3d_size_t) == sizeof(uint32_t));
   return length;
}

v3d_addr_t glxx_hw_install_uniforms(
   glxx_render_state                 *rs,
   const GLXX_SERVER_STATE_T         *state,
   enum shader_mode                   mode,
   const GLXX_UNIFORM_MAP_T          *map,
   GL20_HW_INDEXED_UNIFORM_T         *iu,
   const glxx_hw_image_like_uniforms *image_like_uniforms,
   const glxx_hw_compute_uniforms    *compute_uniforms)
{
   khrn_fmem* fmem = &rs->fmem;

   v3d_addr_t uniforms_addr;
   uint32_t* uniforms_ptr = khrn_fmem_data(&uniforms_addr, fmem, sizeof(uint32_t) * map->num_uniforms, V3D_QPU_UNIFS_ALIGN);
   if (!uniforms_ptr)
      return 0;

   GL20_PROGRAM_COMMON_T* program_common = NULL;
   uint32_t *data;
   if (!IS_GL_11(state))
   {
      program_common = gl20_program_common_get(state);
      data = program_common->uniform_data;
   }
   else
   {
      data = (uint32_t *)&state->gl11;
   }

   /* Install uniforms */
   uint32_t* ptr = uniforms_ptr;
   for (unsigned i = 0; i < map->num_general_uniforms; i++)
   {
      const BackendUniformFlavour u_type  = map->general_uniforms[i].type;
      const uint32_t              u_value = map->general_uniforms[i].value;

      assert(u_type >= 0 && u_type <= BACKEND_UNIFORM_LAST_ELEMENT);

      switch(u_type) {

      case BACKEND_UNIFORM_LITERAL:
         *ptr = u_value;
         break;

      case BACKEND_UNIFORM_PLAIN:
         *ptr = data[u_value];
         break;

      case BACKEND_UNIFORM_PLAIN_FPACK:
      {
         unsigned idx1 = u_value >> 16;
         unsigned idx2 = u_value & 0xFFFF;
         *ptr = (gfx_floatbits_to_float16(data[idx1]) << 16) | gfx_floatbits_to_float16(data[idx2]);
         break;
      }

      case BACKEND_UNIFORM_ADDRESS:
         if (!iu->valid) {
            /* Copy the whole default uniform block into fmem */
            uint32_t *tex = khrn_fmem_data(&iu->block_addr, fmem, 4*program_common->num_scalar_uniforms, 16);
            if (tex == NULL) return 0;

            memcpy(tex, program_common->uniform_data, 4*program_common->num_scalar_uniforms);
            iu->valid = true;
         }

         *ptr = iu->block_addr + 4*(u_value & 0xffff) + (u_value >> 16);
         break;

      case BACKEND_UNIFORM_UBO_ADDRESS:
      case BACKEND_UNIFORM_SSBO_ADDRESS:
      case BACKEND_UNIFORM_ATOMIC_ADDRESS:
      {
         /* Not having a buffer bound gives undefined behaviour - we crash */
         const GLXX_INDEXED_BINDING_POINT_T *binding;
         bool write;
         uint32_t offset;

         if (u_type == BACKEND_UNIFORM_SSBO_ADDRESS) {
            uint32_t id = u_value & 0x1F;
            uint32_t binding_point = program_common->ssbo_binding_point[id];
            binding = &state->ssbo.binding_points[binding_point];

            offset = u_value >> 5;

            // If accessing past the first element of the dynamic array in an SSBO,
            // clamp the element being addressed to the last one. This allows the compiler
            // to fold static accesses to a single address uniform.
            // We should have checked earlier that the binding is large enough for the block
            // (which includes one dynamic array element).
            const gl20_program_dynamic_array* dynamic_array = &program_common->ssbo_dynamic_arrays[id];
            if (offset >= (dynamic_array->offset + dynamic_array->stride))
            {
               uint32_t offset_in_array = offset - dynamic_array->offset;
               uint32_t array_length = get_dynamic_array_length(binding, dynamic_array);
               uint32_t array_size = array_length * dynamic_array->stride;
               if (offset_in_array >= array_size)
                  offset -= ((offset_in_array - array_size) / dynamic_array->stride) + 1;
            }
            write = true;
         } else if (u_type == BACKEND_UNIFORM_UBO_ADDRESS) {
            uint32_t binding_point = program_common->ubo_binding_point[u_value & 0x1F];
            binding = &state->uniform_block.binding_points[binding_point];
            offset = u_value >> 5;
            write = false;
         } else { // u_type == BACKEND_UNIFORM_ATOMIC_ADDRESS
            uint32_t binding_point = u_value >> 16;
            binding = &state->atomic_counter.binding_points[binding_point];
            offset = u_value & 0xFFFF;
            write = true;
         }
         if (write)
            rs->has_buffer_writes = true;

         khrn_resource *res = binding->buffer.obj->resource;

         v3d_barrier_flags rw_flags = V3D_BARRIER_TMU_DATA_READ | (write ? V3D_BARRIER_TMU_DATA_WRITE : 0);
         v3d_barrier_flags bin_rw_flags = mode == MODE_BIN ? rw_flags : V3D_BARRIER_NO_ACCESS;
         if (((khrn_render_state *)rs)->type == KHRN_RENDER_STATE_TYPE_GLXX)
         {
            if (!glxx_hw_tf_aware_sync_res((GLXX_HW_RENDER_STATE_T *)rs, res, bin_rw_flags, rw_flags))
               return 0;
         }
         else
         {
            if (!khrn_fmem_sync_res(&rs->fmem, res, bin_rw_flags, rw_flags))
               return 0;
         }

         *ptr = gmem_get_addr(res->handle) + binding->offset + offset;
         break;
      }

      case BACKEND_UNIFORM_UBO_LOAD:
         // UBO load handled outside this loop, value is how many words to skip.
         ptr += u_value;
         continue;

         // glsl_dataflow_builder doesn't generate these.
      case BACKEND_UNIFORM_SSBO_SIZE:
      case BACKEND_UNIFORM_UBO_SIZE:
      case BACKEND_UNIFORM_UBO_ARRAY_LENGTH:
         unreachable();

      case BACKEND_UNIFORM_SSBO_ARRAY_LENGTH:
      {
         uint32_t buffer_id = u_value & 0x1f;
         uint32_t binding_point = program_common->ssbo_binding_point[buffer_id];
         const GLXX_INDEXED_BINDING_POINT_T *binding =  &state->ssbo.binding_points[binding_point];
         const gl20_program_dynamic_array* dynamic_array = &program_common->ssbo_dynamic_arrays[buffer_id];
         uint32_t length = get_dynamic_array_length(binding, dynamic_array);
         *ptr = length - (u_value >> 5);
         break;
      }

      case BACKEND_UNIFORM_TEX_PARAM0:
         *ptr = assemble_tmu_param(0, u_value, image_like_uniforms->tex);
         break;
      case BACKEND_UNIFORM_TEX_PARAM1:
         *ptr = assemble_tmu_param(1, u_value, image_like_uniforms->tex);
         break;
#if V3D_VER_AT_LEAST(4,0,2,0)
      case BACKEND_UNIFORM_TEX_PARAM1_UNNORMS:
      {
         // Unaligned load of unnorm mask beginning at given sampler index.
         uint32_t unnorm = ((uint32_t)image_like_uniforms->unnorm[u_value/16 + 0] >> (u_value % 16))
                         | ((uint32_t)image_like_uniforms->unnorm[u_value/16 + 1] << (16 - (u_value % 16)));

         // Xor mask with first bit (which will be set in TEX_PARAM1).
         uint32_t unnorm_xor = (0 - (unnorm & 1)) ^ unnorm;

         // Left rotate mask so that bit0 is in the correct position. Shader will rotate
         // right by the sampler index.
         *ptr = unnorm_xor << 1;
         break;
      }
#endif
      case BACKEND_UNIFORM_TEX_SIZE_X:
         *ptr = image_like_uniforms->tex[u_value].width;
         break;
      case BACKEND_UNIFORM_TEX_SIZE_Y:
         *ptr = image_like_uniforms->tex[u_value].height;
         break;
      case BACKEND_UNIFORM_TEX_SIZE_Z:
         *ptr = image_like_uniforms->tex[u_value].depth;
         break;
      case BACKEND_UNIFORM_TEX_LEVELS:
         assert(0); // Not used in GL
         break;

#if !V3D_VER_AT_LEAST(4,0,2,0)
      case BACKEND_UNIFORM_TEX_BASE_LEVEL:
         *ptr = image_like_uniforms->tex[u_value].base_level;
         break;
      case BACKEND_UNIFORM_TEX_BASE_LEVEL_FLOAT:
         *ptr = gfx_float_to_bits((float)image_like_uniforms->tex[u_value].base_level);
         break;
#endif
      case BACKEND_UNIFORM_IMG_PARAM0:
         *ptr = assemble_tmu_param(0, u_value, image_like_uniforms->img);
         break;
#if !V3D_VER_AT_LEAST(4,0,2,0)
      case BACKEND_UNIFORM_IMG_PARAM1:
         *ptr = assemble_tmu_param(1, u_value, image_like_uniforms->img);
         break;
      case BACKEND_UNIFORM_IMAGE_SWIZZLING:
         *ptr = (uint32_t)image_like_uniforms->img[u_value].lx_swizzling;
         break;
      case BACKEND_UNIFORM_IMAGE_XOR_ADDR:
         *ptr = image_like_uniforms->img[u_value].lx_swizzling == GLSL_IMGUNIT_SWIZZLING_UIF_XOR ? GFX_UIF_XOR_ADDR : 0;
         break;
      case BACKEND_UNIFORM_IMAGE_LX_ADDR:
         *ptr = image_like_uniforms->img[u_value].lx_addr;
         break;
      case BACKEND_UNIFORM_IMAGE_LX_PITCH:
         *ptr = image_like_uniforms->img[u_value].lx_pitch;
         break;
      case BACKEND_UNIFORM_IMAGE_LX_SLICE_PITCH:
         *ptr = image_like_uniforms->img[u_value].lx_slice_pitch;
         break;
      case BACKEND_UNIFORM_IMAGE_ARR_STRIDE:
         *ptr = image_like_uniforms->img[u_value].arr_stride;
         break;
#endif
      case BACKEND_UNIFORM_IMG_SIZE_X:
         *ptr = image_like_uniforms->img[u_value].width;
         break;
      case BACKEND_UNIFORM_IMG_SIZE_Y:
         *ptr = image_like_uniforms->img[u_value].height;
         break;
      case BACKEND_UNIFORM_IMG_SIZE_Z:
         *ptr = image_like_uniforms->img[u_value].depth;
         break;

      case BACKEND_UNIFORM_SPECIAL:
      {
         const BackendSpecialUniformFlavour u_special = u_value;

         assert(u_special >= 0 && u_special <= BACKEND_SPECIAL_UNIFORM_LAST_ELEMENT);

         switch (u_special) {

         case BACKEND_SPECIAL_UNIFORM_VP_SCALE_X:
            *ptr = gfx_float_to_bits(state->vp_internal.xscale);
            break;

         case BACKEND_SPECIAL_UNIFORM_VP_SCALE_Y:
            *ptr = gfx_float_to_bits(state->vp_internal.yscale);
            break;

         case BACKEND_SPECIAL_UNIFORM_VP_SCALE_Z:
            *ptr = gfx_float_to_bits(state->vp_internal.zscale);
            break;

         case BACKEND_SPECIAL_UNIFORM_VP_OFFSET_Z:
            *ptr = gfx_float_to_bits(state->vp_internal.zoffset);
            break;

         case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_NEAR:
            *ptr = gfx_float_to_bits(state->depth_range.z_near);
            break;

         case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_FAR:
            *ptr = gfx_float_to_bits(state->depth_range.z_far);
            break;

         case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_DIFF:
            *ptr = gfx_float_to_bits(state->vp_internal.dr_diff);
            break;

#if !V3D_VER_AT_LEAST(4,1,34,0)
         case BACKEND_SPECIAL_UNIFORM_SAMPLE_MASK:
            *ptr = state->sample_mask.mask[0];
            break;
#endif
         case BACKEND_SPECIAL_UNIFORM_SHARED_PTR:
            *ptr = compute_uniforms->shared_ptr;
            break;
         case BACKEND_SPECIAL_UNIFORM_FB_MAX_LAYER:
            {
               assert(((khrn_render_state *)rs)->type == KHRN_RENDER_STATE_TYPE_GLXX);
               const GLXX_HW_FRAMEBUFFER_T *fb = &((GLXX_HW_RENDER_STATE_T *)rs)->installed_fb;
               assert(fb->layers > 0);
               assert(fb->layers <= V3D_MAX_LAYERS);
               *ptr = fb->layers -1 ;
            }
            break;
         default:
            unreachable();
         }
         break;
      }

      case BACKEND_UNIFORM_UNASSIGNED:
         unreachable();
         break;
      }

      ptr++;
   }

   unsigned load_start = 0;
   for (unsigned b = 0; b != map->num_ubo_load_batches; ++b)
   {
      glxx_shader_ubo_load_batch const* load_batch = &map->ubo_load_batches[b];

      bool read_now;
      void const* src;
      if (load_batch->index != GLXX_SHADER_COMPUTE_INDIRECT_BUFFER)
      {
         uint32_t binding_point = program_common->ubo_binding_point[load_batch->index];
         const GLXX_INDEXED_BINDING_POINT_T* binding = &state->uniform_block.binding_points[binding_point];
         khrn_resource* res = binding->buffer.obj->resource;
         assert((binding->offset & 3) == 0);

         v3d_size_t res_offset = binding->offset + load_batch->range_start;
         src = khrn_fmem_resource_read_now_or_in_preprocess(&rs->fmem, res, res_offset, load_batch->range_size, &read_now);
         if (!src)
            return 0;
      }
      else
      {
         read_now = !compute_uniforms->read_in_preprocess;
         src = (char const*)compute_uniforms->num_work_groups + load_batch->range_start;
      }

      const glxx_hw_ubo_load_batch batch =
      {
         .uniform_map = map,
         .dst_ptr = uniforms_ptr,
         .src_ptr = src,
         .loads = map->ubo_loads + load_start,
         .num_loads = load_batch->load_end - load_start
      };

      if (read_now)
         glxx_hw_process_ubo_load_batch(&batch, false);
      else
      {
         if (!khrn_vector_push_back(glxx_hw_ubo_load_batch, &fmem->persist->preprocess_ubo_loads, batch))
            return 0;
         khrn_mem_acquire(map);
      }

      load_start = load_batch->load_end;
   }

   return uniforms_addr;
}

static v3d_prim_mode_t convert_primitive_type(GLXX_SERVER_STATE_T const* state, GLenum mode)
{
   static_assrt(GL_POINTS         == V3D_PRIM_MODE_POINTS    );
   static_assrt(GL_LINES          == V3D_PRIM_MODE_LINES     );
   static_assrt(GL_LINE_LOOP      == V3D_PRIM_MODE_LINE_LOOP );
   static_assrt(GL_LINE_STRIP     == V3D_PRIM_MODE_LINE_STRIP);
   static_assrt(GL_TRIANGLES      == V3D_PRIM_MODE_TRIS      );
   static_assrt(GL_TRIANGLE_STRIP == V3D_PRIM_MODE_TRI_STRIP );
   static_assrt(GL_TRIANGLE_FAN   == V3D_PRIM_MODE_TRI_FAN   );

#if V3D_VER_AT_LEAST(4,0,2,0)
   switch (mode) {
      case GL_LINES_ADJACENCY:          return V3D_PRIM_MODE_LINES_ADJ;
      case GL_LINE_STRIP_ADJACENCY:     return V3D_PRIM_MODE_LINE_STRIP_ADJ;
      case GL_TRIANGLES_ADJACENCY:      return V3D_PRIM_MODE_TRIS_ADJ;
      case GL_TRIANGLE_STRIP_ADJACENCY: return V3D_PRIM_MODE_TRI_STRIP_ADJ;
#if GLXX_HAS_TNG
      case GL_PATCHES:
         assert((state->num_patch_vertices - 1u) < 32);
         return V3D_PRIM_MODE_PATCH1-1u + state->num_patch_vertices;
#endif

      default:
          assert(mode <= GL_TRIANGLE_FAN);
          return mode;
   }
#else
   assert(mode <= GL_TRIANGLE_FAN);

   static_assrt((V3D_PRIM_MODE_LINES_TF - V3D_PRIM_MODE_POINTS_TF) == (GL_LINES - GL_POINTS));
   static_assrt((V3D_PRIM_MODE_TRIS_TF - V3D_PRIM_MODE_POINTS_TF) == (GL_TRIANGLES - GL_POINTS));
   return state->transform_feedback.in_use ? V3D_PRIM_MODE_POINTS_TF + mode : mode;
#endif
}

/*!
 * \brief Converts the index type from the GLenum
 *        representation to the internal one used
 *        by control list instructions.
 *
 * \param type is the GL index type specifier.
 */
static v3d_index_type_t convert_index_type(GLenum type)
{
   // Index type 0,1,2 = 8-bit, 16-bit, 32-bit (max 2^24-1)
   switch (type)
   {
   case GL_UNSIGNED_BYTE:  return V3D_INDEX_TYPE_8BIT;
   case GL_UNSIGNED_SHORT: return V3D_INDEX_TYPE_16BIT;
   case GL_UNSIGNED_INT:   return V3D_INDEX_TYPE_32BIT;;
   default:
      unreachable(); // unsupported index type
      return 0;
   }
}

static bool write_gl_shader_record_defaults(
   v3d_addr_t*                      defaults_addr,
   khrn_fmem*                       fmem,
   const GLXX_LINK_RESULT_DATA_T*   link_data,
   const GLXX_ATTRIB_CONFIG_T*      attribs)
{
   uint32_t *defaults = NULL;
   *defaults_addr = 0;

   assert(link_data->attr_count <= GLXX_CONFIG_MAX_VERTEX_ATTRIBS);

   // Iterate backwards, we'll allocate space for all defaults
   // from the last default down when required.
   for (unsigned n = link_data->attr_count; n-- != 0; )
   {
      const GLXX_ATTRIB_CONFIG_T *attrib = &attribs[link_data->attr[n].idx];
      if (attrib->enabled)
      {
         if (  link_data->attr[n].c_scalars_used > attrib->size
            || link_data->attr[n].v_scalars_used > attrib->size )
         {
            if (!defaults)
            {
               defaults = khrn_fmem_data(defaults_addr, fmem, (n + 1)*16, V3D_ATTR_DEFAULTS_ALIGN);
               if (!defaults)
                  return false;
            }

            defaults[n*4 + 0] = 0u; // or 0.0f
            defaults[n*4 + 1] = 0u; // or 0.0f
            defaults[n*4 + 2] = 0u; // or 0.0f
            defaults[n*4 + 3] = attrib->is_int ? 1u : 0x3f800000; // or 1.0f
         }
      }
   }
   return true;
}

static bool write_gl_shader_record_attribs(
   uint32_t*                           packed_attrs,
   v3d_addr_t                          defaults_addr,
   khrn_fmem*                          fmem,
   const GLXX_LINK_RESULT_DATA_T*      link_data,
   const GLXX_ATTRIB_CONFIG_T*         attribs,
   const glxx_hw_vb*                   vbs,
   const GLXX_GENERIC_ATTRIBUTE_T*     generic_attrs)
{
   // Workaround GFXH-930, must have at least 1 attribute:
   if (link_data->attr_count == 0)
   {
      v3d_addr_t dummy_addr;
      uint32_t *dummy_data = khrn_fmem_data(&dummy_addr, fmem, sizeof(uint32_t), V3D_ATTR_ALIGN);
      if (!dummy_data)
         return false;
      dummy_data[0] = 0;

      V3D_SHADREC_GL_ATTR_T attr = {
         .addr            = dummy_addr,
         .size            = 1,
         .type            = V3D_ATTR_TYPE_FLOAT,
         .signed_int      = false,
         .normalised_int  = false,
         .read_as_int     = false,
         .cs_num_reads    = 1,
         .vs_num_reads    = 1,
         .divisor         = 0,
         .stride          = 0,
#if V3D_VER_AT_LEAST(4,1,34,0)
         .max_index        = 0,
#endif
      };
      v3d_pack_shadrec_gl_attr(packed_attrs, &attr);
      return true;
   }

   // Workaround GFXH-930: First count the total number of reads in VS and CS
   unsigned cs_total_reads = 0;
   unsigned vs_total_reads = 0;
   for (unsigned n = 0; n < link_data->attr_count; n++)
   {
      cs_total_reads += link_data->attr[n].c_scalars_used;
      vs_total_reads += link_data->attr[n].v_scalars_used;
   }

   for (unsigned n = 0; n < link_data->attr_count; n++)
   {
      unsigned cs_num_reads = link_data->attr[n].c_scalars_used;
      unsigned vs_num_reads = link_data->attr[n].v_scalars_used;
      assert(cs_num_reads > 0 || vs_num_reads > 0); // attribute entry shouldn't exist if there are no reads.

      // Workaround GFXH-930:
      if (n == 0 && (cs_total_reads == 0 || vs_total_reads == 0))
      {
         // We read all attributes either in CS or VS, bodge the first attribute
         // to be read by both. It must be the first to work around GFXH-1602
         assert(cs_num_reads == 0 || vs_num_reads == 0);
         cs_num_reads |= vs_num_reads;
         vs_num_reads = cs_num_reads;
      }

      const unsigned i = link_data->attr[n].idx;
      const GLXX_ATTRIB_CONFIG_T *attrib = &attribs[i];
      if (attrib->enabled)
      {
         uint32_t divisor = vbs[i].divisor;
         uint32_t stride = vbs[i].stride;
         if (divisor == 0xffffffff)
         {
            // Handle special divisor case.
            divisor = 1u;
            stride = 0u;
         }

         V3D_SHADREC_GL_ATTR_T attr = {
            .addr             = vbs[i].addr,
            .size             = attrib->size,
            .type             = attrib->v3d_type,
            .signed_int       = attrib->is_signed,
            .normalised_int   = attrib->norm,
            .read_as_int      = attrib->is_int,
            .cs_num_reads     = cs_num_reads,
            .vs_num_reads     = vs_num_reads,
            .divisor          = divisor,
            .stride           = stride,
#if V3D_VER_AT_LEAST(4,1,34,0)
            .max_index        = gfx_umin(vbs[i].max_index, V3D_VCD_MAX_INDEX),
#endif
         };
         v3d_pack_shadrec_gl_attr(packed_attrs + V3D_SHADREC_GL_ATTR_PACKED_SIZE/sizeof(uint32_t)*n, &attr);
      }
      else
      {
         const GLXX_GENERIC_ATTRIBUTE_T *generic = &generic_attrs[i];
         v3d_addr_t generic_addr;
         uint32_t* generic_data = khrn_fmem_data(&generic_addr, fmem, sizeof(uint32_t)*4u, V3D_ATTR_ALIGN);
         if (!generic_data)
            return false;
         generic_data[0] = generic->u[0];
         generic_data[1] = generic->u[1];
         generic_data[2] = generic->u[2];
         generic_data[3] = generic->u[3];

         V3D_SHADREC_GL_ATTR_T attr = {
            .addr            = generic_addr,
            .size            = 4,
            .type            = generic->type,
            .signed_int      = generic->is_signed,
            .normalised_int  = false,
            .read_as_int     = generic->type != V3D_ATTR_TYPE_FLOAT,
            .cs_num_reads    = cs_num_reads,
            .vs_num_reads    = vs_num_reads,
            .divisor         = 0,
            .stride          = 0,
#if V3D_VER_AT_LEAST(4,1,34,0)
            .max_index        = 0,
#endif
         };
         v3d_pack_shadrec_gl_attr(packed_attrs + V3D_SHADREC_GL_ATTR_PACKED_SIZE/sizeof(uint32_t)*n, &attr);
      }
   }

   return true;
}

#if GLXX_HAS_TNG
static void compute_tng_vpm_cfg(
   v3d_vpm_cfg_v cfg_v[2],
   uint32_t shadrec_tg_packed[],
   GLXX_LINK_RESULT_DATA_T const* link_data,
   unsigned num_patch_vertices)
{
   V3D_VPM_CFG_TG_T cfg_tg[2];
   bool ok = v3d_vpm_compute_cfg_tg(
      cfg_v, cfg_tg,
      link_data->has_tess,
      link_data->has_geom,
      khrn_get_vpm_size() / 512,
      link_data->vs_input_words,
      link_data->vs_output_words,
      num_patch_vertices,
      link_data->tcs_output_words_per_patch,
      link_data->tcs_output_words,
      !!(link_data->flags & GLXX_SHADER_FLAGS_TCS_BARRIERS),
      link_data->tcs_output_vertices_per_patch,
      link_data->tes_output_words,
      link_data->tess_type,
      6u, // maximum number of vertices per GS primitive
      link_data->gs_output_words);
   assert(ok); // GL requirements mean this should always succeed.

   V3D_SHADREC_GL_TESS_OR_GEOM_T shadrec_tg =
   {
      .tess_type           = link_data->tess_type,
      .tess_point_mode     = link_data->tess_point_mode,
      .tess_edge_spacing   = link_data->tess_edge_spacing,
      .tess_clockwise      = link_data->tess_clockwise,
      //.tcs_bypass        = todo_not_implemented,
      //.tcs_bypass_render = todo_not_implemented,
      .tes_no_inp_verts    = true, // todo_not_implemented
      .num_tcs_invocations = gfx_umax(link_data->tcs_output_vertices_per_patch, 1u),
      .geom_output         = link_data->geom_prim_type,
      .geom_num_instances  = gfx_umax(link_data->geom_invocations, 1u),
   };
   v3d_shadrec_gl_tg_set_vpm_cfg(&shadrec_tg, cfg_tg);

   v3d_pack_shadrec_gl_tess_or_geom(shadrec_tg_packed, &shadrec_tg);
}
#endif

static bool write_gl_shader_record(
   GLXX_HW_RENDER_STATE_T *rs,
   const GLXX_SERVER_STATE_T *state,
   GLXX_LINK_RESULT_DATA_T *link_data, const glxx_hw_image_like_uniforms *image_like_uniforms,
   const GLXX_ATTRIB_CONFIG_T *attribs, const glxx_hw_vb *vbs)
{
   v3d_addr_t defaults_addr;
   if (!write_gl_shader_record_defaults(&defaults_addr, &rs->fmem, link_data, attribs))
      return false;

   if (!khrn_fmem_sync_res(&rs->fmem, link_data->res, V3D_BARRIER_QPU_INSTR_READ, V3D_BARRIER_QPU_INSTR_READ))
      return false;
   v3d_addr_t code_addr = gmem_get_addr(link_data->res->handle);

   GL20_HW_INDEXED_UNIFORM_T iu;
   iu.valid = false;

   // Bin/Render vertex pipeline uniforms.
   v3d_addr_t vp_unifs_addrs[GLXX_SHADER_VPS_COUNT][MODE_COUNT];
   for (enum shader_mode m = 0; m != MODE_COUNT; ++m)
   {
      for (unsigned s = 0; s != GLXX_SHADER_VPS_COUNT; ++s)
      {
         if (link_data->vps[s][m].uniform_map != NULL)
         {
            vp_unifs_addrs[s][m] = glxx_hw_install_uniforms(&rs->base, state, m, link_data->vps[s][m].uniform_map, &iu, image_like_uniforms, NULL);
            if (!vp_unifs_addrs[s][m])
               return false;
         }
      }
   }

#if V3D_HAS_NULL_FS
   const bool null_fs = link_data->fs.code_offset == ~(v3d_size_t)0;
#else
   assert(link_data->fs.code_offset != ~(v3d_size_t)0);
   const bool null_fs = false;
#endif

   // Fragment shader uniforms.
   v3d_addr_t frag_unifs_addr = 0;
   if (!null_fs)
   {
      frag_unifs_addr = glxx_hw_install_uniforms(&rs->base, state, MODE_RENDER, link_data->fs.uniform_map, &iu, image_like_uniforms, NULL);
      if (!frag_unifs_addr)
         return false;
   }

   // Separate shader-records from uniforms when clif recording.
#if KHRN_DEBUG
   if (khrn_options.autoclif_enabled)
   {
      v3d_addr_t pad_addr;
      khrn_fmem_data(&pad_addr, &rs->fmem, V3D_MAX_QPU_UNIFS_READAHEAD + 1, 1);
   }
#endif

#if GLXX_HAS_TNG
   // Treat tessellation part of record as two geometry shaders.
   static_assrt(V3D_SHADREC_GL_GEOM_PACKED_SIZE*2 == V3D_SHADREC_GL_TESS_PACKED_SIZE);
#endif

   unsigned num_attrs = gfx_umax(1u, link_data->attr_count);
   unsigned shadrec_size =
#if GLXX_HAS_TNG
      +  V3D_SHADREC_GL_GEOM_PACKED_SIZE
      +  V3D_SHADREC_GL_TESS_PACKED_SIZE
      +  V3D_SHADREC_GL_TESS_OR_GEOM_PACKED_SIZE
#endif
      +  V3D_SHADREC_GL_MAIN_PACKED_SIZE
      +  V3D_SHADREC_GL_ATTR_PACKED_SIZE*num_attrs;

   v3d_addr_t shadrec_addr;
   uint32_t* shadrec_data = khrn_fmem_begin_data(&shadrec_addr, &rs->fmem, shadrec_size, V3D_SHADREC_ALIGN);
   if (!shadrec_data)
      return false;

#ifdef KHRN_GEOMD
   /* It would be nice to use address of the draw instruction. However, this
    * would only be useful during binning. We use shader record address
    * instead, as this gets binned to tile list and thus will be useful during
    * rendering as well. This does mean we must have unique shader record for
    * each draw call when using debug info. */
   fmem_debug_info_insert(&rs->fmem, shadrec_addr, ((GLXX_SERVER_STATE_T*)state)->debug.draw_id++, khrn_hw_render_state_allocated_order(rs));
#endif

#if GLXX_HAS_TNG
   static_assrt(GLXX_SHADER_VPS_GS == (GLXX_SHADER_VPS_VS + 1));
   static_assrt(GLXX_SHADER_VPS_TCS  == (GLXX_SHADER_VPS_VS + 2));
   static_assrt(GLXX_SHADER_VPS_TES  == (GLXX_SHADER_VPS_VS + 3));
#endif

#if GLXX_HAS_TNG
   // Write out shader record parts for each T+G stage.
   for (unsigned s = GLXX_SHADER_VPS_VS+1; s <= GLXX_SHADER_VPS_TES; ++s)
   {
      if (link_data->vps[s][MODE_RENDER].uniform_map != NULL)
      {
         V3D_SHADREC_GL_GEOM_T stage_shadrec =
         {
            .gs_bin.threading          = link_data->vps[s][MODE_BIN].threading,
#if V3D_VER_AT_LEAST(4,1,34,0)
            .gs_bin.single_seg         = link_data->vps[s][MODE_BIN].single_seg,
#endif
            .gs_bin.propagate_nans     = true,
            .gs_bin.addr               = code_addr + link_data->vps[s][MODE_BIN].code_offset,
            .gs_bin.unifs_addr         = vp_unifs_addrs[s][MODE_BIN],
            .gs_render.threading       = link_data->vps[s][MODE_RENDER].threading,
#if V3D_VER_AT_LEAST(4,1,34,0)
            .gs_render.single_seg      = link_data->vps[s][MODE_RENDER].single_seg,
#endif
            .gs_render.propagate_nans  = true,
            .gs_render.addr            = code_addr + link_data->vps[s][MODE_RENDER].code_offset,
            .gs_render.unifs_addr      = vp_unifs_addrs[s][MODE_RENDER],
         };
         v3d_pack_shadrec_gl_geom(shadrec_data, &stage_shadrec);
         shadrec_data += V3D_SHADREC_GL_GEOM_PACKED_SIZE / sizeof(*shadrec_data);
      }
   }

   // Write out tess or geom part.
   if (link_data->has_tng)
   {
      unsigned num_patch_vertices = link_data->has_tess ? state->num_patch_vertices : 0u;

      if (!link_data->cached_vpm_cfg.key.valid || link_data->cached_vpm_cfg.key.tg.num_patch_vertices != num_patch_vertices)
      {
         link_data->cached_vpm_cfg.key.valid = true;
         link_data->cached_vpm_cfg.key.tg.num_patch_vertices = num_patch_vertices;

         compute_tng_vpm_cfg(
            link_data->cached_vpm_cfg.vpm_cfg_v,
            link_data->cached_vpm_cfg.shadrec_tg_packed,
            link_data,
            state->num_patch_vertices);
      }


      unsigned num_words = V3D_SHADREC_GL_TESS_OR_GEOM_PACKED_SIZE/sizeof(*shadrec_data);
      for (unsigned w = 0; w != num_words; ++w)
      {
         shadrec_data[w] = link_data->cached_vpm_cfg.shadrec_tg_packed[w];
      }
      shadrec_data += num_words;

      /* When T+G is enabled, not writing a point size should have it default to 1.0 */
      uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, V3D_CL_POINT_SIZE_SIZE);
      v3d_cl_point_size(&instr, 1.0f);
      khrn_fmem_end_cle_exact(&rs->fmem, instr);
   }
   else
#endif
   {
      bool z_pre_pass = rs->z_prepass_started && !rs->z_prepass_stopped;
      if (!link_data->cached_vpm_cfg.key.valid || link_data->cached_vpm_cfg.key.v.z_pre_pass != z_pre_pass)
      {
         link_data->cached_vpm_cfg.key.valid = true;
         link_data->cached_vpm_cfg.key.v.z_pre_pass = z_pre_pass;

         v3d_vpm_compute_cfg(
            link_data->cached_vpm_cfg.vpm_cfg_v,
            khrn_get_vpm_size() / 512,
            link_data->vs_input_words,
            link_data->vs_output_words,
            z_pre_pass);
      }
   }

   v3d_vpm_cfg_v const* vpm_cfg_v = link_data->cached_vpm_cfg.vpm_cfg_v;

   V3D_SHADREC_GL_MAIN_T main_shadrec =
   {
      .point_size_included       = !!(link_data->flags & GLXX_SHADER_FLAGS_POINT_SIZE_SHADED_VERTEX_DATA),
      .clipping                  = !state->caps.rasterizer_discard,     /* No need to clip if everything is thrown away */
      .cs_vertex_id              = !!(link_data->flags & (GLXX_SHADER_FLAGS_VS_READS_VERTEX_ID << MODE_BIN)),
      .cs_instance_id            = !!(link_data->flags & (GLXX_SHADER_FLAGS_VS_READS_INSTANCE_ID << MODE_BIN)),
      .vs_vertex_id              = !!(link_data->flags & (GLXX_SHADER_FLAGS_VS_READS_VERTEX_ID << MODE_RENDER)),
      .vs_instance_id            = !!(link_data->flags & (GLXX_SHADER_FLAGS_VS_READS_INSTANCE_ID << MODE_RENDER)),
      .z_write                   = !!(link_data->flags & GLXX_SHADER_FLAGS_FS_WRITES_Z),
      .no_ez                     = !!(link_data->flags & GLXX_SHADER_FLAGS_FS_EARLY_Z_DISABLE),
      .cs_separate_blocks        = !!(link_data->flags & (GLXX_SHADER_FLAGS_VS_SEPARATE_I_O_VPM_BLOCKS << MODE_BIN)),
      .vs_separate_blocks        = !!(link_data->flags & (GLXX_SHADER_FLAGS_VS_SEPARATE_I_O_VPM_BLOCKS << MODE_RENDER)),
      .fs_needs_w                = !!(link_data->flags & GLXX_SHADER_FLAGS_FS_NEEDS_W),
      .scb_wait_on_first_thrsw   = !!(link_data->flags & GLXX_SHADER_FLAGS_TLB_WAIT_FIRST_THRSW),
      .disable_scb               = false,
#if V3D_VER_AT_LEAST(4,0,2,0)
      .disable_implicit_varys    = !!(link_data->flags & GLXX_SHADER_FLAGS_DISABLE_IMPLICIT_VARYS),
      .cs_baseinstance = !!(link_data->flags & (GLXX_SHADER_FLAGS_VS_READS_BASE_INSTANCE << MODE_BIN)),
      .vs_baseinstance = !!(link_data->flags & (GLXX_SHADER_FLAGS_VS_READS_BASE_INSTANCE << MODE_RENDER)),
      .prim_id_used  = !!(link_data->flags & GLXX_SHADER_FLAGS_PRIM_ID_USED),
      .prim_id_to_fs = !!(link_data->flags & GLXX_SHADER_FLAGS_PRIM_ID_TO_FS),
      /* We must only set sample_rate_shading if multisample is on */
      .sample_rate_shading = ( (link_data->flags & GLXX_SHADER_FLAGS_PER_SAMPLE) ||
                               (state->caps.sample_shading && state->sample_shading_fraction >= 0.25f) ||
                               khrn_options_make_sample_rate_shaded() ) &&
                             (khrn_options.force_multisample || rs->installed_fb.ms),
#endif
      .num_varys = link_data->num_varys,
      .cs_output_size = vpm_cfg_v[MODE_BIN].output_size,
      .cs_input_size = vpm_cfg_v[MODE_BIN].input_size,
      .vs_output_size = vpm_cfg_v[MODE_RENDER].output_size,
      .vs_input_size = vpm_cfg_v[MODE_RENDER].input_size,
      .defaults = defaults_addr,
      .fs.threading = link_data->fs.threading,
#if V3D_VER_AT_LEAST(4,1,34,0)
      .fs.single_seg  = link_data->fs.single_seg,
#endif
      .fs.propagate_nans = true,
      .fs.addr = null_fs ? 0 : code_addr + link_data->fs.code_offset,
      .fs.unifs_addr = frag_unifs_addr,
      .vs.threading = link_data->vps[GLXX_SHADER_VPS_VS][MODE_RENDER].threading,
#if V3D_VER_AT_LEAST(4,1,34,0)
      .vs.single_seg  = link_data->vps[GLXX_SHADER_VPS_VS][MODE_RENDER].single_seg,
#endif
      .vs.propagate_nans = true,
      .vs.addr = code_addr + link_data->vps[GLXX_SHADER_VPS_VS][MODE_RENDER].code_offset,
      .vs.unifs_addr = vp_unifs_addrs[GLXX_SHADER_VPS_VS][MODE_RENDER],
      .cs.threading = link_data->vps[GLXX_SHADER_VPS_VS][MODE_BIN].threading,
#if V3D_VER_AT_LEAST(4,1,34,0)
      .cs.single_seg  = link_data->vps[GLXX_SHADER_VPS_VS][MODE_BIN].single_seg,
#endif
      .cs.propagate_nans = true,
      .cs.addr = code_addr + link_data->vps[GLXX_SHADER_VPS_VS][MODE_BIN].code_offset,
      .cs.unifs_addr = vp_unifs_addrs[GLXX_SHADER_VPS_VS][MODE_BIN],
   };
   // Modify the shader record to avoid specifying 0 varyings.
   #if !V3D_VER_AT_LEAST(3,3,0,0)
      v3d_workaround_gfxh_1276(&main_shadrec);
   #endif
   v3d_pack_shadrec_gl_main(shadrec_data, &main_shadrec);
   shadrec_data += V3D_SHADREC_GL_MAIN_PACKED_SIZE / sizeof(*shadrec_data);

   // write_gl_shader_record_attribs needs to allocate fmem_data, so end this block early.
   uint32_t* shadrec_attr_data = shadrec_data;
   shadrec_data += V3D_SHADREC_GL_ATTR_PACKED_SIZE/sizeof(uint32_t) * num_attrs;
   khrn_fmem_end_data(&rs->fmem, shadrec_data);

   // Write shader record attributes.
   bool ok = write_gl_shader_record_attribs(
      shadrec_attr_data,
      defaults_addr,
      &rs->fmem,
      link_data,
      attribs,
      vbs,
      state->generic_attrib);
   if (!ok)
      return false;

   // Convert bits from active stage mask to CL opcode.
   unsigned tng_stage_mask = 0;
#if GLXX_HAS_TNG
   tng_stage_mask |= link_data->has_tess;
   tng_stage_mask |= link_data->has_geom << 1;
   static_assrt(V3D_CL_GL_T_SHADER == V3D_CL_GL_SHADER+1);
   static_assrt(V3D_CL_GL_G_SHADER == V3D_CL_GL_SHADER+2);
   static_assrt(V3D_CL_GL_TG_SHADER == V3D_CL_GL_SHADER+3);
#endif
   uint32_t shader_cl_opcode = V3D_CL_GL_SHADER + tng_stage_mask;

   // Finally, write shader-record control list item.
   uint8_t* cl = khrn_fmem_begin_cle(&rs->fmem, V3D_CL_VCM_CACHE_SIZE_SIZE + V3D_CL_GL_SHADER_SIZE);
   if (!cl)
      return false;
   v3d_cl_vcm_cache_size(&cl, vpm_cfg_v[0].vcm_cache_size, vpm_cfg_v[1].vcm_cache_size);
   // todo, replace with generalised v3d_cl_gl_shader
   v3d_cl_add_8(&cl, shader_cl_opcode);
   cl[0] = (uint8_t)gfx_pack_uint_0_is_max(num_attrs, 5) | (uint8_t)(gfx_exact_lsr(shadrec_addr, 5) << 5);
   cl[1] = (uint8_t)(gfx_exact_lsr(shadrec_addr, 5) >> 3);
   cl[2] = (uint8_t)(gfx_exact_lsr(shadrec_addr, 5) >> 11);
   cl[3] = (uint8_t)(gfx_exact_lsr(shadrec_addr, 5) >> 19);
   cl += 4;
   khrn_fmem_end_cle_exact(&rs->fmem, cl);
   return true;
}

static v3d_addr_t create_nv_shader_record(
   khrn_fmem*        fmem,
   v3d_addr_t        fshader_addr,
   v3d_addr_t        funif_addr,
   v3d_addr_t        vdata_addr,
   uint32_t          vdata_max_index,
   bool              does_z_writes,
   v3d_threading_t   threading)
{
   // Find what sized allocations we need
   V3D_NV_SHADER_RECORD_ALLOC_SIZES_T  sizes;
   v3d_get_nv_shader_record_alloc_sizes(&sizes);

   // Allocate the defaults fmem
   v3d_addr_t defaults_addr;
   uint32_t *defaults_ptr = khrn_fmem_data(&defaults_addr, fmem, sizes.defaults_size, sizes.defaults_align);
   if (!defaults_ptr)
      return 0;

   // Allocate the shader record fmem
   v3d_addr_t shader_record_packed_addr;
   uint32_t *shader_record_packed_ptr = khrn_fmem_data(
      &shader_record_packed_addr,
      fmem,
      sizes.packed_shader_rec_size,
      sizes.packed_shader_rec_align);
   if (!shader_record_packed_ptr)
      return 0;

   // Call the common creation function
   v3d_create_nv_shader_record(
      shader_record_packed_ptr,
      shader_record_packed_addr,
      defaults_ptr,
      defaults_addr,
      fshader_addr,
      funif_addr,
      vdata_addr,
      vdata_max_index,
      does_z_writes,
      threading);

   return shader_record_packed_addr;
}

static void nv_vertex(uint32_t *addr, uint32_t x, uint32_t y, uint32_t z)
{
   // Clipping and 1/Wc come from defaults
   addr[0] = x << 8;
   addr[1] = y << 8;
   addr[2] = z;
}

/* Allocate vertex data for draw_rect in fmem */
v3d_addr_t glxx_draw_rect_vertex_data(uint32_t *vdata_max_index, khrn_fmem *fmem, const glxx_rect *rect, uint32_t z)
{
   *vdata_max_index = 3;
   unsigned attr_components= 3 ; /* x, y, z */
   v3d_addr_t addr;

   uint32_t size = attr_components * sizeof(uint32_t) * (*vdata_max_index + 1);
   uint32_t *ptr = khrn_fmem_data(&addr, fmem, size, V3D_ATTR_ALIGN);
   if (ptr == NULL)
      return 0;

   int xmax = rect->x + rect->width;
   int ymax = rect->y + rect->height;
   nv_vertex(ptr, rect->x, rect->y, z);
   nv_vertex(ptr + attr_components, xmax, rect->y, z);
   nv_vertex(ptr + 2 * attr_components, xmax, ymax, z);
   nv_vertex(ptr + 3 * attr_components, rect->x, ymax, z);

   return addr;
}

static bool glxx_draw_rect_write_state_changes(
   GLXX_SERVER_STATE_T *state,
   GLXX_HW_RENDER_STATE_T *rs,
   const GLXX_CLEAR_T *clear,
   const glxx_rect *rect)
{
   // Set up control list
   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem,
      V3D_CL_CLIP_SIZE +
      V3D_CL_CLIPZ_SIZE +
      V3D_CL_CFG_BITS_SIZE +
      V3D_CL_STENCIL_CFG_SIZE +
      V3D_CL_COLOR_WMASKS_SIZE +
      V3D_CL_VIEWPORT_OFFSET_SIZE);
   if (!instr)
      return false;

   //  Emit scissor/clipper/viewport instructions
   v3d_cl_clip(&instr, rect->x, rect->y, rect->width, rect->height);
   v3d_cl_clipz(&instr, 0.0f, 1.0f);
   khrn_render_state_set_add(&state->dirty.viewport, rs); /* Clear and render might end up with different clip rectangles - clear doesn't clip to viewport */

   //  Emit a Configuration record
   glxx_ez_update_cfg(&rs->ez,
      V3D_COMPARE_FUNC_ALWAYS, clear->depth,
      clear->stencil,
      V3D_STENCIL_OP_ZERO, V3D_STENCIL_OP_ZERO,
      V3D_STENCIL_OP_ZERO, V3D_STENCIL_OP_ZERO);
   v3d_cl_cfg_bits(&instr,
      true,                      /* front_prims */
      true,                      /* back_prims */
      false,                     /* cwise_is_front */
      false,                     /* depth offset */
      false,                     /* aa_lines */
      V3D_MS_1X,                 /* rast_oversample */
      false,                     /* cov_pipe */
      V3D_COV_UPDATE_NONZERO,    /* cov_update */
      false,                     /* wireframe_tris */
      V3D_COMPARE_FUNC_ALWAYS,   /* depth_test - zfunc */
      clear->depth,              /* depth_update - enzu   */
      rs->ez.cfg_bits_ez,
      rs->ez.cfg_bits_ez_update,
      clear->stencil,            /* stencil */
      false,                     /* blend */
      V3D_WIREFRAME_MODE_LINES,
      false);                    /* d3d_prov_vtx */

   khrn_render_state_set_add(&state->dirty.cfg, rs); /* Clear and render probably use different configs */

   /* Emit a stencil config record */
   if (clear->stencil)
   {
      v3d_cl_stencil_cfg(&instr,
         clear->stencil_value,               /* ref value */
         0xffU,                              /* test mask */
         V3D_COMPARE_FUNC_ALWAYS,            /* test function */
         V3D_STENCIL_OP_ZERO,                /* stencil fail op = don't care */
         V3D_STENCIL_OP_ZERO,                /* depth fail op = don't care */
         V3D_STENCIL_OP_REPLACE,             /* pass op = replace */
         true,                               /* back config */
         true,                               /* front config */
         state->stencil_mask.front & 0xffU); /* stencil write mask */

      khrn_render_state_set_add(&state->dirty.stencil, rs);
   }

   if (clear->color_buffer_mask)
      write_changed_color_write(&instr, rs, state);
   else
   {
      /* Disable all color writes */
      v3d_cl_color_wmasks(&instr, gfx_mask(V3D_MAX_RENDER_TARGETS * 4));
      khrn_render_state_set_add(&state->dirty.color_write, rs);
   }

#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_cl_viewport_offset(&instr, 0, 0, 0, 0);
#else
   v3d_cl_viewport_offset(&instr, 0, 0);
#endif

   //TODO: other miscellaneous pieces of state

   khrn_fmem_end_cle(&rs->fmem, instr);

   return true;
}

static v3d_addr_t copy_shader_to_fmem(khrn_fmem* fmem, uint32_t shaderSize, uint32_t *shaderCode)
{
   v3d_addr_t addr;
   uint32_t *ptr = khrn_fmem_data(&addr, fmem, shaderSize, V3D_QPU_INSTR_ALIGN);
   if (!ptr)
      return 0;
   memcpy(ptr, shaderCode, shaderSize);
   return addr;
}

/* Draw a rectangle quickly using a non-vertex shader. Currently only used as
 * the slow path of a clear operation,
 */
/* TODO Should really share more code with the non-draw_rect case */
bool glxx_draw_rect(
   GLXX_SERVER_STATE_T     *state,
   GLXX_HW_RENDER_STATE_T  *rs,
   const GLXX_CLEAR_T      *clear,
   const glxx_rect         *rect)
{
   assert(clear->color_buffer_mask || clear->depth || clear->stencil);

   if (!rs->clist_start && !(rs->clist_start = khrn_fmem_begin_clist(&rs->fmem)))
      return false;

   const GLXX_HW_FRAMEBUFFER_T *hw_fb = &rs->installed_fb;

   //TODO: MRT
   unsigned int rt = 0;
   if (clear->color_buffer_mask)
   {
      assert(gfx_is_power_of_2(clear->color_buffer_mask));
      rt = gfx_msb(clear->color_buffer_mask);
      assert(glxx_fb_is_valid_draw_buf(state->bound_draw_framebuffer, GLXX_COLOR0_ATT + rt));
      assert(hw_fb->color[rt].image != NULL || hw_fb->color_ms[rt].image != NULL);
      glxx_bufstate_rw(&rs->color_buffer_state[rt]);
   }

   if (clear->depth)
      glxx_bufstate_rw(&rs->depth_buffer_state);

   if (clear->stencil)
      glxx_bufstate_rw(&rs->stencil_buffer_state);

   v3d_addr_t funif_addr;
   uint32_t *funif_ptr = khrn_fmem_data(&funif_addr, &rs->fmem, V3D_CLEAR_SHADER_MAX_UNIFS * sizeof(uint32_t), V3D_QPU_UNIFS_ALIGN);
   if (funif_ptr == NULL)
      return false;

   uint32_t *shader_code;
   uint32_t shader_size;
   if (clear->color_buffer_mask)
      v3d_clear_shader_color(&shader_code, &shader_size, funif_ptr, hw_fb->color_rt_format[rt].type, rt, clear->color_value);
   else
      v3d_clear_shader_no_color(&shader_code, &shader_size, funif_ptr);


   /* Install the shader and shader record. */
   v3d_addr_t fshader_addr = copy_shader_to_fmem(&rs->fmem, shader_size, shader_code);
   if (!fshader_addr)
      return false;
   v3d_addr_t sh_rec_addr = 0;
   if (rs->num_used_layers == 1)
   {
      uint32_t vdata_max_index;
      v3d_addr_t vdata_addr = glxx_draw_rect_vertex_data( &vdata_max_index, &rs->fmem, rect,
         gfx_float_to_bits(clear->depth_value));
      if (!vdata_addr)
         return false;

      sh_rec_addr = create_nv_shader_record(&rs->fmem, fshader_addr, funif_addr,
            vdata_addr, vdata_max_index, /*does_z_writes=*/false,
#if V3D_VER_AT_LEAST(4,1,34,0)
            V3D_THREADING_4
#else
            V3D_THREADING_1
#endif
            );
   }
#if V3D_VER_AT_LEAST(4,0,2,0)
   else
   {
      //layered_fb, we need a vs + gs for clear
      sh_rec_addr = glxx_create_clear_gl_g_shader_record(&rs->fmem, fshader_addr, funif_addr,
            rect, clear->depth_value);
   }
#else
   assert(rs->num_used_layers == 1);
#endif

   if (!sh_rec_addr)
      return false;

   if (!glxx_draw_rect_write_state_changes(state, rs, clear, rect))
      return false;

#if V3D_VER_AT_LEAST(4,0,2,0)
   glxx_prim_drawn_by_us_record(rs, 2 * rs->num_used_layers);
   if (!glxx_tf_record_disable(rs))
      return false;
#endif

   unsigned size = V3D_CL_GL_SHADER_SIZE + V3D_CL_VCM_CACHE_SIZE_SIZE;
   if (rs->num_used_layers == 1)
      size += V3D_CL_VERTEX_ARRAY_PRIMS_SIZE;
   else
      size += V3D_CL_VERTEX_ARRAY_INSTANCED_PRIMS_SIZE;

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, size);
   if (!instr) return false;

   v3d_cl_vcm_cache_size(&instr, 4, 4);
   if (rs->num_used_layers == 1)
   {
      v3d_cl_nv_shader(&instr, 2, sh_rec_addr);
      v3d_cl_vertex_array_prims(&instr, V3D_PRIM_MODE_TRI_FAN, 4, 0);
   }
#if V3D_VER_AT_LEAST(4,0,2,0)
   else
   {
      v3d_cl_gl_g_shader(&instr, 1, sh_rec_addr);
      v3d_cl_vertex_array_instanced_prims(&instr, V3D_PRIM_MODE_TRI_FAN, 4, rs->num_used_layers, 0);
   }
#else
   assert(rs->num_used_layers == 1);
#endif

   khrn_fmem_end_cle_exact(&rs->fmem, instr);

   return true;
}

#if !V3D_VER_AT_LEAST(4,2,13,0)

static uint32_t setup_render_dummy_point_no_shader_no_query_addr_size(void)
{
   return V3D_CL_VIEWPORT_OFFSET_SIZE
      + V3D_CL_CLIP_SIZE
      + V3D_CL_POINT_SIZE_SIZE
      + V3D_CL_PRIM_LIST_FORMAT_SIZE
      + V3D_CL_SAMPLE_STATE_SIZE
      + V3D_CL_CLIPZ_SIZE
      + V3D_CL_CFG_BITS_SIZE
      + V3D_CL_COLOR_WMASKS_SIZE;
}

static uint32_t clear_shader_no_color_point[] =
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   0xbb800000, 0x3c203186, // nop ; thrsw
   0xbb800000, 0x3d203186, // nop ; thrsw ; ldvary[.rf0]
   0xbb800000, 0x3c003186, // nop
#else
#if !V3D_VER_AT_LEAST(3,3,0,0)
   // Extra varying from working around GFXH-1276
   0xbb800000, 0x3d003186, // nop; ldvary
#endif
   0xbb800000, 0x3d003186, // nop; ldvary
#endif
   0xb683f000, 0x3dc03188, // mov tlbu, 0     ; ldvary[.rf0]
   0xb7800000, 0x3c203187, // xor tlb, r0, r0 ; thrsw
   0x00000000, 0x030031c6, // mov tlb, 0
   0x00000000, 0x030031c6, // mov tlb, 0
};

/* The center of the dummy point will be at (0,0).
 * The pixels (if there are any) will pass the Z tests but nothing will be
 * written to the tilebuffer. */

static bool setup_render_dummy_point_no_shader_no_query_addr(uint8_t** instr_ptr, v3d_addr_t* shader_addr, khrn_fmem* fmem, float size, uint32_t fb_width, uint32_t fb_height)
{
   v3d_addr_t funif_addr;
   uint32_t* funif_ptr = khrn_fmem_data(&funif_addr, fmem, 4, V3D_SHADREC_ALIGN);
   if (funif_ptr == NULL)
      return false;
   {
      /* Set up the TLB config uniform */
      /* Configure tlb for f32 color writes, but disable the write masks later */
      /* GFXH-1212 means we must write 4 values to prevent a lockup (if !V3D_VER_AT_LEAST(4,0,2,0) at least) */
      V3D_TLB_CONFIG_COLOR_32_T cfg = {
         .num_words = 4,
         .all_samples_same_data = true,
         .rt = 0,
#if !V3D_VER_AT_LEAST(4,2,13,0)
         .as_int = false
#endif
         };
      /* Unused config entries must be all 1s */
      funif_ptr[0] = 0xffffff00 | v3d_pack_tlb_config_color_32(&cfg);
   }

   /* create single point vertex */
   uint32_t vdata_max_index = 0;
   v3d_addr_t vdata_addr;
   uint32_t* vdata_ptr = khrn_fmem_data(&vdata_addr, fmem, sizeof(uint32_t) * 3, V3D_ATTR_ALIGN);
   if (!vdata_ptr)
      return false;
   nv_vertex(vdata_ptr, 0, 0, 0);

   /* Install the shader and shader record. */
   v3d_addr_t fshader_addr = copy_shader_to_fmem(fmem, sizeof(clear_shader_no_color_point), clear_shader_no_color_point);
   if (!fshader_addr)
      return false;
   *shader_addr = create_nv_shader_record(fmem, fshader_addr, funif_addr,
         vdata_addr, vdata_max_index, false,
#if V3D_VER_AT_LEAST(4,1,34,0)
         V3D_THREADING_4
#else
         V3D_THREADING_1
#endif
         );
   if (!*shader_addr)
      return false;

   // Items written to instr begin here.
   uint8_t* instr = *instr_ptr;

   // Emit scissor/clipper/viewport instructions
#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_cl_viewport_offset(&instr, 0, 0, 0, 0);
#else
   v3d_cl_viewport_offset(&instr, 0, 0);
#endif
   v3d_cl_clip(&instr, 0, 0, fb_width, fb_height);

   v3d_cl_point_size(&instr, size);
   v3d_cl_prim_list_format(&instr, 1, false, false);

   // Emit enough configuration to ensure we pass the Z tests.
#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_cl_sample_state(&instr, 0xf, 1.0f);
#else
   v3d_cl_sample_state(&instr, 1.0f);
#endif
   v3d_cl_clipz(&instr, 0.0f, 1.0f);
   v3d_cl_cfg_bits(&instr,
      true,                      /* front_prims */
      true,                      /* back_prims */
      false,                     /* cwise_is_front */
      false,                     /* depth offset */
      false,                     /* aa_lines */
      V3D_MS_1X,                 /* rast_oversample */
      false,                     /* cov_pipe */
      V3D_COV_UPDATE_NONZERO,    /* cov_update */
      false,                     /* wireframe_tris */
      V3D_COMPARE_FUNC_ALWAYS,   /* depth_test - zfunc */
      false,                     /* depth_update - enzu   */
      false,                     /* ez */
      false,                     /* ez_update */
      false,                     /* stencil */
      false,                     /* blend */
      V3D_WIREFRAME_MODE_LINES,  /* wireframe_mode */
      false                      /* d3d_prov_vtx */
      );

   /* Disable all color writes */
   v3d_cl_color_wmasks(&instr, gfx_mask(V3D_MAX_RENDER_TARGETS * 4));

   assert((*instr_ptr + setup_render_dummy_point_no_shader_no_query_addr_size()) == instr);
   *instr_ptr = instr;
   return true;
}

static uint32_t render_dummy_point_size(void)
{
   return V3D_CL_COMPRESSED_PRIM_LIST_IID_ZERO_SIZE + 4; /* compressed point list */
}

static void render_dummy_point(uint8_t** instr_ptr)
{
   uint8_t* instr = *instr_ptr;
   v3d_cl_compressed_prim_list_iid_zero(&instr);
   instr[0] = 131;   // select coding 4
   instr[1] = 0;     // vertex 0 (16 bit index)
   instr[2] = 0;     //
   instr[3] = 227;   // escape
   instr += 4;
   assert((*instr_ptr + render_dummy_point_size()) == instr);
   *instr_ptr = instr;
}

#if !V3D_VER_AT_LEAST(3,3,0,0)

static uint32_t setup_render_dummy_point_size(void)
{
   return setup_render_dummy_point_no_shader_no_query_addr_size()
      + V3D_CL_OCCLUSION_QUERY_COUNTER_ENABLE_SIZE
      + V3D_CL_GL_SHADER_SIZE;
}

static bool setup_render_dummy_point(uint8_t** instr_ptr, khrn_fmem* fmem, float size,
   uint32_t fb_width, uint32_t fb_height)
{
   // Items written to instr begin here.
   uint8_t* instr = *instr_ptr;

   v3d_addr_t shader_addr;
   if (!setup_render_dummy_point_no_shader_no_query_addr(&instr, &shader_addr, fmem, size, fb_width, fb_height))
      return false;

   v3d_cl_occlusion_query_counter_enable(&instr, 0);

   // Emit NV shader instruction.
   v3d_cl_nv_shader(&instr, 2, shader_addr);

   assert((*instr_ptr + setup_render_dummy_point_size()) == instr);
   *instr_ptr = instr;
   return true;
}

uint32_t glxx_workaround_gfxh_1313_size(void)
{
   return setup_render_dummy_point_size() + render_dummy_point_size();
}

bool glxx_workaround_gfxh_1313(uint8_t** instr_ptr, khrn_fmem* fmem,
   uint32_t fb_width, uint32_t fb_height)
{
   if (!setup_render_dummy_point(instr_ptr, fmem, 0.0f, fb_width, fb_height))
      return false;

   render_dummy_point(instr_ptr);
   return true;
}

#endif

uint32_t glxx_fill_ocq_cache_size(void)
{
   return setup_render_dummy_point_no_shader_no_query_addr_size()
#if V3D_VER_AT_LEAST(3,3,0,0)
      + V3D_CL_GL_SHADER_SIZE
#endif
      + (V3D_CL_OCCLUSION_QUERY_COUNTER_ENABLE_SIZE
#if !V3D_VER_AT_LEAST(3,3,0,0)
         + V3D_CL_GL_SHADER_SIZE
#endif
         + render_dummy_point_size()) * 8;
}

bool glxx_fill_ocq_cache(uint8_t** instr_ptr, khrn_fmem* fmem,
   uint32_t fb_width, uint32_t fb_height)
{
   uint8_t* instr = *instr_ptr;

   // Get address of scratch buffer for TLB to spray writes to.
   gmem_handle_t scratch_mem = khrn_get_dummy_ocq_buffer();
   v3d_addr_t scratch_addr = khrn_fmem_sync_and_get_addr(fmem, scratch_mem, 0, V3D_BARRIER_TLB_OQ_READ | V3D_BARRIER_TLB_OQ_WRITE);
   if (!scratch_addr)
      return false;

   // Setup for rendering a point of size 1 pixel.
   v3d_addr_t shader_addr;
   if (!setup_render_dummy_point_no_shader_no_query_addr(&instr, &shader_addr, fmem, 1.5f, fb_width, fb_height))
      return false;

#if V3D_VER_AT_LEAST(3,3,0,0)
   v3d_cl_nv_shader(&instr, 2, shader_addr);
#endif

   // Draw a pixel for each way in the occlusion query cache.
   for (unsigned i = 0; i != 8; ++i)
   {
      v3d_cl_occlusion_query_counter_enable(&instr, scratch_addr + V3D_OCCLUSION_QUERY_COUNTER_FIRST_CORE_CACHE_LINE_ALIGN * i);
#if !V3D_VER_AT_LEAST(3,3,0,0)
      // GFXH-1330 requires a shader change for every stencil/query change.
      v3d_cl_nv_shader(&instr, 2, shader_addr);
#endif
      render_dummy_point(&instr);
   }

   assert((instr - *instr_ptr) == glxx_fill_ocq_cache_size());
   *instr_ptr = instr;
   return true;
}

#endif

static bool glxx_hw_update_z_prepass_state(
   GLXX_SERVER_STATE_T *state,
   GLXX_LINK_RESULT_DATA_T const *link_data,
   GLXX_HW_RENDER_STATE_T *rs
   )
{
   // early out if already disallowed
   if (!rs->z_prepass_allowed)
   {
      return true;
   }

   // disable z-only if using stencil
   if (     state->caps.stencil_test
      && (  state->stencil_op.back.fail != GL_KEEP
         || state->stencil_op.back.zfail != GL_KEEP
         || state->stencil_op.back.zpass != GL_KEEP
         || state->stencil_op.front.fail != GL_KEEP
         || state->stencil_op.front.zfail != GL_KEEP
         || state->stencil_op.front.zpass != GL_KEEP
         )
      )
   {
      goto disallowed;
   }

#if GLXX_HAS_TNG
   // disable z-only for TnG
   if (link_data->has_tng)
      goto disallowed;
#endif

   // if depth writes are enabled, then we must test the following conditions to
   // ensure that performing a z-prepass is valid
   if (state->caps.depth_test && state->depth_mask)
   {
      // disable z-only if using blending (currently a conservative test)
#if V3D_VER_AT_LEAST(4,0,2,0)
      if (state->blend.rt_enables & gfx_mask(rs->installed_fb.rt_count))
#else
      if (state->blend.enable)
#endif
      {
         goto disallowed;
      }

      // disable z-prepass if using colour write masks (currently a conservative test)
      // timh-todo: should only care if this changes
      uint32_t relevant_channels = gfx_mask(rs->installed_fb.rt_count * 4);
      if ((state->color_write & relevant_channels) != relevant_channels)
      {
         goto disallowed;
      }

      // disable z-prepass if using discard or shader z write
      if (state->depth_mask && (link_data->flags & GLXX_SHADER_FLAGS_FS_WRITES_Z) != 0)
      {
         goto disallowed;
      }

      // take z-direction
      int8_t z_dir = 0;
      switch (state->depth_func)
      {
      case GL_LESS:
      case GL_LEQUAL:
         z_dir = -1;
         break;
      case GL_GREATER:
      case GL_GEQUAL:
         z_dir = 1;
         break;
      }

      // disable z-prepass if z-func is not compatible
      if (z_dir == 0)
      {
         goto disallowed;
      }

      // if we haven't started z-only yet, then capture z-direction now
      // this must subsequently remain constant for all z enabled draws
      if (!rs->z_prepass_started)
      {
         rs->z_prepass_dir = z_dir;
         rs->z_prepass_started = true;
      }
      else if (z_dir != rs->z_prepass_dir)
      {
         goto disallowed;
      }
   }
   else if (rs->z_prepass_started)
   {
      // As a heuristic, if a z-prepass was started but we are no longer writing to z,
      // then assume future batches are also of no use in a the z-prepass and disable it.
      // If an application is sensible and renders all opaque geometry first, then this
      // results in optimal behaviour.
      goto disallowed;
   }

   return true;

disallowed:
   // end z-prepass rendering if started
   if (rs->z_prepass_started)
   {
      uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, V3D_CL_END_Z_ONLY_SIZE);
      if (!instr)
         return false;
      v3d_cl_end_z_only(&instr);
      khrn_fmem_end_cle_exact(&rs->fmem, instr);

      rs->z_prepass_stopped = true;
   }
   rs->z_prepass_allowed = false;
   return true;
}

bool glxx_hw_tf_aware_sync_res(GLXX_HW_RENDER_STATE_T *rs,
   khrn_resource *res, v3d_barrier_flags bin_rw_flags, v3d_barrier_flags render_rw_flags)
{
   unsigned wait_count;
   if ((khrn_resource_get_write_stages(res, (khrn_render_state *)rs) & KHRN_STAGE_BIN) &&
      (wait_count = res->last_tf_write_count))
   {
      // We have written to this buffer with transform feedback in this render
      // state

      if (bin_rw_flags)
      {
         if (rs->tf.waited_count < wait_count)
         {
            uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem,
               V3D_CL_FLUSH_TRANSFORM_FEEDBACK_DATA_SIZE +
               V3D_CL_WAIT_TRANSFORM_FEEDBACK_SIZE);
            if (!instr)
               return false;
            v3d_cl_flush_transform_feedback_data(&instr);
            v3d_cl_wait_transform_feedback(&instr, wait_count);
            khrn_fmem_end_cle_exact(&rs->fmem, instr);

            rs->tf.waited_count = wait_count;
            rs->tf.done_cache_ops = V3D_CACHE_OPS_NONE;
         }

         khrn_fmem_cle_barrier_flush(&rs->fmem, V3D_BARRIER_PTB_TF_WRITE, bin_rw_flags, &rs->tf.done_cache_ops);
      }

      if (render_rw_flags)
         rs->fmem.br_info.details.render_depends_on_bin = true;
   }

   return khrn_fmem_sync_res(&rs->fmem, res, bin_rw_flags, render_rw_flags);
}
