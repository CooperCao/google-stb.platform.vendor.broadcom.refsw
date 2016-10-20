/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
BCM2708 implementation of hardware abstraction layer.
Functions common to OpenGL ES 1.1 and OpenGL ES 2.0
=============================================================================*/

#include "../common/khrn_int_common.h"
#include "../common/khrn_int_math.h"
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
#include "../gl20/gl20_program.h"
#include "../glsl/glsl_tex_params.h"
#include "../glsl/glsl_backend_uniforms.h"
#include "../gl11/gl11_shadercache.h"
#include "../gl11/gl11_shader.h"

#include <limits.h>
#include <math.h>
#include <assert.h>

#include "libs/core/v3d/v3d_common.h"
#include "libs/core/v3d/v3d_cl.h"
#include "libs/core/v3d/v3d_vpm.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/util/gfx_util/gfx_util_conv.h"

LOG_DEFAULT_CAT("glxx_hw")

/*************************************************************
 Static function forwards
 *************************************************************/

static bool write_gl_shader_record(
   GLXX_HW_RENDER_STATE_T              *rs,
   GLXX_LINK_RESULT_DATA_T             *link_data,
   const GLXX_SERVER_STATE_T           *state,
   const GLXX_ATTRIB_CONFIG_T          *attr_cfgs,
   const GLXX_VERTEX_BUFFER_CONFIG_T   *vb_cfgs,
   const GLXX_VERTEX_POINTERS_T        *vb_pointers);

static v3d_prim_mode_t convert_primitive_type(GLXX_SERVER_STATE_T const* state, GLenum mode);
static v3d_index_type_t convert_index_type(GLenum type);

static bool backend_uniform_address(KHRN_FMEM_T *fmem, uint32_t u1,
   const GL20_PROGRAM_COMMON_T *program,
   GL20_HW_INDEXED_UNIFORM_T *iu,
   uint32_t *location);
static bool insert_flat_shading_flags(GLXX_HW_RENDER_STATE_T *rs,
   GLXX_SERVER_STATE_T *state,
   const GLXX_LINK_RESULT_DATA_T *link_data);
static bool insert_centroid_flags(GLXX_HW_RENDER_STATE_T *rs,
   GLXX_SERVER_STATE_T *state,
   const GLXX_LINK_RESULT_DATA_T *link_data);

static bool glxx_hw_update_z_prepass_state(
   GLXX_SERVER_STATE_T *state,
   GLXX_LINK_RESULT_DATA_T const *link_data,
   GLXX_HW_RENDER_STATE_T *rs
   );

static bool post_draw_journal(
   GLXX_HW_RENDER_STATE_T        *rs,
   const GLXX_ATTRIB_CONFIG_T    *attrib,
   const GLXX_VERTEX_BUFFER_CONFIG_T *vb_config,
   const GLXX_VERTEX_POINTERS_T  *vertex_pointers,
   const GLXX_LINK_RESULT_DATA_T *link_data,
   const GLXX_DRAW_T             *draw
);

static GLXX_HW_RENDER_STATE_T* get_existing_rs(const GLXX_SERVER_STATE_T *state,
      const GLXX_HW_FRAMEBUFFER_T *fb);
static bool setup_dummy_texture_for_texture_unif(GLXX_TEXTURE_UNIF_T *texture_unif,
      bool for_image_unit, KHRN_FMEM_T *fmem, bool used_in_binning, bool is_32bit
#if !V3D_VER_AT_LEAST(3,3,0,0)
      , glsl_gadgettype_t *gadgettype
#endif
      );

/*************************************************************
 Global Functions
 *************************************************************/

void glxx_hw_invalidate_framebuffer(
   GLXX_SERVER_STATE_T *state, GLXX_FRAMEBUFFER_T *fb,
   uint32_t rt, bool all_color_ms, bool depth, bool stencil)
{
   GLXX_HW_FRAMEBUFFER_T hw_fb;
   if (!glxx_init_hw_framebuffer(fb, &hw_fb))
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
            khrn_image_plane_invalidate(&hw_fb.color[b]);
         if ((rt & 1) || all_color_ms)
            khrn_image_plane_invalidate(&hw_fb.color_ms[b]);
      }

      if (depth && stencil)
         khrn_image_plane_invalidate_two(&hw_fb.depth, &hw_fb.stencil);
      else
      {
         if (depth)
            khrn_image_plane_invalidate(&hw_fb.depth);
         if (stencil)
            khrn_image_plane_invalidate(&hw_fb.stencil);
      }
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
   KHRN_IMAGE_PLANE_T *image_plane,
   bool multisample)
{
   // Not all framebuffers have color / multisample / depth / stencil
   if (image_plane->image == NULL)
      return;

   unsigned w = khrn_image_get_width(image_plane->image);
   unsigned h = khrn_image_get_height(image_plane->image);
   if (multisample)
   {
      w /= 2;
      h /= 2;
   }
   assert(w <= GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE);
   assert(h <= GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE);

   hw_fb->width = gfx_umin(hw_fb->width, w);
   hw_fb->height = gfx_umin(hw_fb->height, h);
}

bool glxx_init_hw_framebuffer(const GLXX_FRAMEBUFFER_T *fb,
                               GLXX_HW_FRAMEBUFFER_T *hw_fb)
{
   assert(glxx_fb_is_complete(fb));

   memset(hw_fb, 0, sizeof(*hw_fb));

   hw_fb->ms = (glxx_fb_get_ms_mode(fb) != GLXX_NO_MS);

   // we always configure tilebuffer based on all attached images in fb,
   // regardless of any draw buffers setting.
   hw_fb->rt_count = 1;
   for (unsigned b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
   {
      const GLXX_ATTACHMENT_T *att = &fb->attachment[GLXX_COLOR0_ATT + b];

      if (!glxx_attachment_acquire_image(att, GLXX_DOWNSAMPLED, &hw_fb->color[b].image, NULL))
         goto error_out;

      if (!glxx_attachment_acquire_image(att, GLXX_MULTISAMPLED, &hw_fb->color_ms[b].image, NULL))
         goto error_out;

      if (hw_fb->color[b].image || hw_fb->color_ms[b].image)
         hw_fb->rt_count = b + 1;
   }

   for (unsigned b = 0; b != hw_fb->rt_count; ++b)
   {
      if (hw_fb->color[b].image)
      {
         v3d_pixel_format_internal_type_and_bpp(
            &hw_fb->color_internal_type[b], &hw_fb->color_internal_bpp[b],
            gfx_lfmt_translate_pixel_format(khrn_image_plane_lfmt(&hw_fb->color[b])));
#ifndef NDEBUG
         if (hw_fb->color_ms[b].image)
         {
            v3d_rt_type_t type;
            v3d_rt_bpp_t bpp;
            v3d_pixel_format_internal_type_and_bpp(&type, &bpp,
               gfx_lfmt_translate_pixel_format(khrn_image_plane_lfmt(&hw_fb->color_ms[b])));
            assert(hw_fb->color_internal_type[b] == type);
            assert(hw_fb->color_internal_bpp[b] == bpp);
         }
#endif
      }
      else if (hw_fb->color_ms[b].image)
         v3d_pixel_format_internal_type_and_bpp(
            &hw_fb->color_internal_type[b], &hw_fb->color_internal_bpp[b],
            gfx_lfmt_translate_pixel_format(khrn_image_plane_lfmt(&hw_fb->color_ms[b])));
      else
      {
         hw_fb->color_internal_bpp[b] = V3D_RT_BPP_32;
         hw_fb->color_internal_type[b] = V3D_RT_TYPE_8;
      }
   }

   {
      bool depth_ms;
      if (!glxx_attachment_acquire_image(&fb->attachment[GLXX_DEPTH_ATT],
               GLXX_PREFER_DOWNSAMPLED, &hw_fb->depth.image, &depth_ms))
         goto error_out;
      if (hw_fb->depth.image)
         assert(hw_fb->ms == depth_ms);
   }

   {
      bool stencil_ms;
      if (!glxx_attachment_acquire_image(&fb->attachment[GLXX_STENCIL_ATT],
               GLXX_PREFER_DOWNSAMPLED, &hw_fb->stencil.image, &stencil_ms))
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
      assert(a->color_internal_bpp[i] == b->color_internal_bpp[i]);
      assert(a->color_internal_type[i] == b->color_internal_type[i]);
   }

   if (!khrn_image_plane_equal(&a->depth, &b->depth))
      return false;

   if (!khrn_image_plane_equal(&a->stencil, &b->stencil))
      return false;

   /* Width/height are derived from the attached images, so if all the
    * attachments match, they should match too... */
   assert(a->width == b->width);
   assert(a->height == b->height);

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
   const GLXX_HW_FRAMEBUFFER_T *fb, bool for_tlb_blit)
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

static int get_tlb_write_type_from_rt_type(v3d_rt_type_t rt)
{
   switch (rt)
   {
   case V3D_RT_TYPE_8I:
   case V3D_RT_TYPE_8UI:
   case V3D_RT_TYPE_16I:
   case V3D_RT_TYPE_16UI:
   case V3D_RT_TYPE_32I:
   case V3D_RT_TYPE_32UI:
      return GLXX_FB_I32;
   case V3D_RT_TYPE_8:
   case V3D_RT_TYPE_16F:
      return GLXX_FB_F16;
   case V3D_RT_TYPE_32F:
      return GLXX_FB_F32;
   default:
      unreachable();
      return 0;
   }
}

#if !V3D_HAS_GFXH1212_FIX
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

static bool compute_texture_uniforms(GLXX_SERVER_STATE_T *state, glxx_render_state *rs)
{
   glxx_texture_iterator_t iterate = glxx_server_texture_iterator(state);
   struct glxx_texture_info info;

#if !V3D_VER_AT_LEAST(3,3,0,0)
   for (unsigned i = 0; i < GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS; i++)
      state->shaderkey_common.gadgettype[i] = 0;
#endif

   for (unsigned i = 0; iterate(&info, state, &i); )
   {
      unsigned index = info.index;
      GLXX_TEXTURE_UNIF_T *texture_unif = &state->texture_unif[index];

      const GLXX_TEXTURE_SAMPLER_STATE_T *sampler = info.sampler;
      if (!sampler || glxx_tex_target_is_multisample(info.texture->target))
         sampler = &info.texture->sampler;

      enum glxx_tex_completeness complete = glxx_texture_key_and_uniforms(
            info.texture, NULL, sampler,
            info.used_in_binning, info.is_32bit, rs, texture_unif,
#if !V3D_VER_AT_LEAST(3,3,0,0)
            &state->shaderkey_common.gadgettype[index],
#endif
            &state->fences);

      switch (complete)
      {
      case INCOMPLETE:
         /* All gl11 textures should be enabled by now */
         assert(!IS_GL_11(state));
         break;
      case COMPLETE:
         continue;
      case OUT_OF_MEMORY:
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
         return false;
      default:
         unreachable();
      }

      if (!setup_dummy_texture_for_texture_unif(texture_unif, false, &rs->fmem,
               info.used_in_binning, info.is_32bit
#if !V3D_VER_AT_LEAST(3,3,0,0)
               , &state->shaderkey_common.gadgettype[index]
#endif
               ))
         return false;

   }

   return true;
}

static bool glxx_server_iterate_glslimages(
      const glxx_image_unit **image_unit,
      const GLSL_IMAGE_T **glslimage_info,
      unsigned *index,
      const GLXX_SERVER_STATE_T *state, unsigned *i)
{
   const GL20_PROGRAM_COMMON_T *program = gl20_program_common_get(state);
   unsigned num_images = program->linked_glsl_program->num_images;
   GLSL_IMAGE_T *images = program->linked_glsl_program->images;

   while (*i < num_images)
   {
      int j = program->uniform_data[images[*i].location];

      if (j >= 0 && j < GLXX_CONFIG_MAX_IMAGE_UNITS)
      {
         *index = *i;
         *image_unit = &state->image_unit[j];
         assert(images[*i].internalformat != GL_NONE);
         *glslimage_info= &images[*i];
         (*i)++;
         return true;
      }
      (*i)++;
   }

   return false;
}

static bool setup_dummy_texture_for_texture_unif(GLXX_TEXTURE_UNIF_T *texture_unif,
      bool for_image_unit, KHRN_FMEM_T *fmem, bool used_in_binning, bool is_32bit
#if !V3D_VER_AT_LEAST(3,3,0,0)
      , glsl_gadgettype_t *gadgettype
#endif
      )
{
   gmem_handle_t dummy_texture_handle = get_dummy_texture();
   v3d_addr_t addr_base = khrn_fmem_lock_and_sync(fmem,
         dummy_texture_handle,
         used_in_binning ?
         GMEM_SYNC_TMU_DATA_READ : 0,
         GMEM_SYNC_TMU_DATA_READ);

   assert(v3d_addr_aligned(addr_base, V3D_TMU_ML_ALIGN));

   const unsigned width = 4;
   const unsigned height = 4;
   const unsigned depth = 1;
   const v3d_tmu_type_t type = V3D_TMU_TYPE_RGBA8;

   texture_unif->width = width;
   texture_unif->height = height;
   texture_unif->depth = depth;
#if !V3D_HAS_NEW_TMU_CFG
   texture_unif->base_level = 0;
#endif

   if (for_image_unit)
   {
      /* by setting w/h/d to 0, the image store will not be done */
      texture_unif->width = 0;
      texture_unif->height = 0;
      texture_unif->depth = 0;

#if !V3D_HAS_TMU_TEX_WRITE
      texture_unif->arr_stride = 0;
      texture_unif->lx_addr = addr_base;
      texture_unif->lx_pitch = 0;
      texture_unif->lx_slice_pitch = 0;
      texture_unif->lx_swizzling = GLSL_IMGUNIT_SWIZZLING_LT;
#endif
   }

#if V3D_HAS_NEW_TMU_CFG
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
   texture_unif->hw_param[0] = khrn_fmem_add_tmu_tex_state(fmem, hw_tex_state, /*extended=*/false);
   if (!texture_unif->hw_param[0])
      return false;

   if (for_image_unit)
      texture_unif->hw_param[1] = 0; // ignored in this case
   else
   {
      V3D_TMU_SAMPLER_T sampler;
      memset(&sampler, 0, sizeof(sampler));
      sampler.filter = V3D_TMU_FILTER_MIN_NEAR_MAG_NEAR;
      uint8_t hw_sampler[V3D_TMU_SAMPLER_PACKED_SIZE];
      v3d_pack_tmu_sampler(hw_sampler, &sampler);
      texture_unif->hw_param[1] = khrn_fmem_add_tmu_sampler(fmem, hw_sampler, /*extended=*/false);
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
   tmu_indirect.filter = V3D_TMU_FILTER_MIN_NEAR_MAG_NEAR;
   tmu_indirect.arr_str = 4*4*4;// For 3D textures this is used to init the slice_pitch
   tmu_indirect.base = addr_base;
   tmu_indirect.width = width;
   tmu_indirect.height = height;
   tmu_indirect.depth = depth;
   tmu_indirect.ttype = type;
#if V3D_VER_AT_LEAST(3,3,0,0)
   tmu_indirect.u.not_child_image.output_type = is_32bit ? V3D_TMU_OUTPUT_TYPE_32 : V3D_TMU_OUTPUT_TYPE_16;
#else
   tmu_indirect.u.not_child_image.output_type = V3D_TMU_OUTPUT_TYPE_AUTO;
#endif
   tmu_indirect.swizzles[0] = V3D_TMU_SWIZZLE_R;
   tmu_indirect.swizzles[1] = V3D_TMU_SWIZZLE_G;
   tmu_indirect.swizzles[2] = V3D_TMU_SWIZZLE_B;
   tmu_indirect.swizzles[3] = V3D_TMU_SWIZZLE_A;
   tmu_indirect.etcflip = true;

   uint32_t hw_indirect[V3D_TMU_INDIRECT_PACKED_SIZE / 4];
   v3d_pack_tmu_indirect_not_child_image(hw_indirect, &tmu_indirect);
   texture_unif->hw_param[1] = khrn_fmem_add_tmu_indirect(fmem, hw_indirect);
   if (!texture_unif->hw_param[1])
      return false;

   tmu_indirect.filter = V3D_TMU_FILTER_MIN_LIN_MAG_LIN;
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
   *gadgettype = is_32bit ? GLSL_GADGETTYPE_SWAP1632 : GLSL_GADGETTYPE_AUTO;
#endif
#endif
   return true;
}

static bool compute_image_uniforms(GLXX_SERVER_STATE_T *state,
                                   glxx_render_state *rs)
{
   const glxx_image_unit *image_unit = NULL;
   const GLSL_IMAGE_T *info = NULL;
   unsigned index;

#if !V3D_VER_AT_LEAST(3,3,0,0)
   for (unsigned i = 0; i < GLXX_CONFIG_MAX_IMAGE_UNITS; i++)
      state->shaderkey_common.img_gadgettype[i] = 0;
#endif

   if (IS_GL_11(state))
      return true;

   for (unsigned i = 0; glxx_server_iterate_glslimages(&image_unit, &info, &index, state, &i); )
   {
      GLXX_TEXTURE_UNIF_T *texture_unif = &state->image_unif[index];

      glxx_calc_image_unit calc_image_unit;
      if (glxx_get_calc_image_unit(image_unit, info, &calc_image_unit) == GLXX_ACC_OK)
      {
         enum glxx_tex_completeness complete = glxx_texture_key_and_uniforms(
               image_unit->texture, &calc_image_unit,
               khrn_get_image_unit_default_sampler(), info->in_binning, info->is_32bit,
               rs, texture_unif,
#if !V3D_VER_AT_LEAST(3,3,0,0)
               &state->shaderkey_common.img_gadgettype[index],
#endif
               &state->fences);

         switch (complete)
         {
         case COMPLETE:
            continue;
         case OUT_OF_MEMORY:
            glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
            break;
         default:
            unreachable();
            break;
         }
      }

      if (!setup_dummy_texture_for_texture_unif(texture_unif, true, &rs->fmem,
               info->in_binning, info->is_32bit
#if !V3D_VER_AT_LEAST(3,3,0,0)
               , &state->shaderkey_common.img_gadgettype[index]
#endif
            ))
         return false;

   }

   return true;
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
                                           GLXX_PRIMITIVE_T draw_mode)
{
   const GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;
   uint32_t backend_mask = fb->ms ? ~0u : ~(uint32_t)GLXX_SAMPLE_OPS_M;
   uint32_t backend = state->statebits.backend & backend_mask;

   if (!IS_GL_11(state))
   {
      const GLSL_PROGRAM_T *p = gl20_program_common_get(state)->linked_glsl_program;
      draw_mode = glxx_get_used_draw_mode(p, draw_mode);
   }

   /* Set the primitive type */
   switch (draw_mode)
   {
   case GL_POINTS:
      backend |= GLXX_PRIM_POINT;
      break;
   case GL_LINES:
   case GL_LINE_LOOP:
   case GL_LINE_STRIP:
      backend |= GLXX_PRIM_LINE;
      break;
   default:
      /* Triangles */
      switch (state->fill_mode)
      {
      case GL_POINT_BRCM:
         backend |= GLXX_PRIM_POINT;
         break;
      case GL_LINE_BRCM:
         backend |= GLXX_PRIM_LINE;
         break;
      case GL_FILL_BRCM:
         /* Do nothing */
         break;
      default:
         unreachable();
      }
   }

   if (rs->z_prepass_allowed) backend |= GLXX_Z_ONLY_WRITE;

   bool activeOcclusion = glxx_server_has_active_query_type(GLXX_Q_OCCLUSION, state, rs);

   if (!activeOcclusion && no_depth_updates(state, rs) && no_stencil_updates(state, rs))
      backend |= GLXX_FEZ_SAFE_WITH_DISCARD;

   if (state->sample_mask.enable && fb->ms) backend |= GLXX_SAMPLE_MASK;

   for (int i = 0; i < GLXX_MAX_RENDER_TARGETS; i++)
   {
      int fb_gadget;
      if (glxx_fb_is_valid_draw_buf(state->bound_draw_framebuffer, GLXX_COLOR0_ATT + i))
      {
         v3d_rt_type_t type = fb->color_internal_type[i];
         fb_gadget = get_tlb_write_type_from_rt_type(type);
#if !V3D_HAS_GFXH1212_FIX
         v3d_rt_bpp_t bpp = fb->color_internal_bpp[i];
         if (fmt_requires_alpha16(type, bpp))
            fb_gadget |= GLXX_FB_ALPHA_16_WORKAROUND;
#endif
      }
      else
         fb_gadget = GLXX_FB_NOT_PRESENT;

      assert((fb_gadget & (~GLXX_FB_GADGET_M)) == 0);

      uint32_t shift = GLXX_FB_GADGET_S + 3*i;
      backend &= ~(GLXX_FB_GADGET_M << shift);
      backend |= (fb_gadget << shift);
   }

   backend |= glxx_advanced_blend_eqn(state) << GLXX_ADV_BLEND_S;

   return backend;
}

bool glxx_compute_image_like_uniforms(GLXX_SERVER_STATE_T *state,
                                      glxx_render_state *rs)
{
   bool res = compute_texture_uniforms(state, rs);
   if (res)
      res = compute_image_uniforms(state, rs);
   return res;
}

bool glxx_calculate_and_hide(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs,
      GLXX_PRIMITIVE_T draw_mode)
{
   state->shaderkey_common.backend = compute_backend_shader_key(state, rs, draw_mode);

   return glxx_compute_image_like_uniforms(state, &rs->base);
}

#if V3D_HAS_PER_RT_BLEND
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
#if V3D_HAS_PER_RT_BLEND
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
#if V3D_HAS_PER_RT_BLEND
      rt_mask,
#endif
      V3D_BLEND_VG_MODE_NORMAL);
}

static void write_blend_cfg(uint8_t **instr,
   const GLXX_HW_RENDER_STATE_T *rs, const GLXX_SERVER_STATE_T *state)
{
   static glxx_blend_cfg   dummy = { 0, };

#if V3D_HAS_PER_RT_BLEND
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

         bool not_advanced = state->blend.rt_cfgs[i].color_eqn < GLXX_ADV_BLEND_EQN_BIT;

         write_blend_cfg_instr(instr,  not_advanced ? &state->blend.rt_cfgs[i] : &dummy, rt_mask);
         done_mask |= rt_mask;
      }
   }
   assert(done_mask == gfx_mask(V3D_MAX_RENDER_TARGETS));
#else
   bool not_advanced = state->blend.cfg.color_eqn < GLXX_ADV_BLEND_EQN_BIT;

   write_blend_cfg_instr(instr, not_advanced ? &state->blend.cfg : &dummy);
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

   uint32_t w_disable_mask = ~state->color_write;

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
   bool *nothing_to_draw)
{
   KHRN_FMEM_T *fmem = &rs->fmem;
   const GLXX_HW_FRAMEBUFFER_T *hw_fb = &rs->installed_fb;
   bool depth_test;
   v3d_compare_func_t depth_func;
   bool depth_update;
   bool stencil_test;
   bool line_smooth = false;
   bool rasterizer_discard = false;
   int scx, scy, scw, sch;
   int vpx, vpy, vpw, vph;
   unsigned viewport_offset_scale;
   float clipper_xy_scale;

   depth_test = state->caps.depth_test && (hw_fb->depth.image != NULL);
   depth_func = depth_test
      ? glxx_hw_convert_test_function(state->depth_func)
      : V3D_COMPARE_FUNC_ALWAYS;
   depth_update = depth_test && state->depth_mask;
   stencil_test = state->caps.stencil_test && (hw_fb->stencil.image != NULL);

   viewport_offset_scale = 128;
   clipper_xy_scale = 128.0f;

   unsigned max_size = GLXX_CL_STATE_SIZE[GLXX_CL_STATE_VIEWPORT]
                     + GLXX_CL_STATE_SIZE[GLXX_CL_STATE_SAMPLE_COVERAGE]
#if V3D_HAS_PER_RT_BLEND
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

   scx = state->scissor.x;
   scy = state->scissor.y;
   scw = state->scissor.width;
   sch = state->scissor.height;
   vpx = state->viewport.x;
   vpy = state->viewport.y;
   vpw = state->viewport.width;
   vph = state->viewport.height;

   *nothing_to_draw = false;

   if (IS_GL_11(state))
      line_smooth = !!(state->gl11.statebits.fragment & GL11_LINESMOOTH);

   if (khrn_render_state_set_contains(state->dirty.viewport, rs))
   {
      int x = gfx_smax(0, vpx);
      int y = gfx_smax(0, vpy);
      int xmax = gfx_smin(hw_fb->width, vpx + vpw);
      int ymax = gfx_smin(hw_fb->height, vpy + vph);

      if (state->caps.scissor_test)
      {
         x = gfx_smax(x, scx);
         y = gfx_smax(y, scy);
         xmax = gfx_smin(xmax, scx + scw);
         ymax = gfx_smin(ymax, scy + sch);
      }

      if (x >= xmax || y >= ymax)
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
            xmax = x + 1;
            ymax = y + 1;
         }
      }

      khrn_render_state_set_remove(&state->dirty.viewport, rs);

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_VIEWPORT, instr);

      v3d_cl_clip(&instr, x, y, xmax - x, ymax - y);

      v3d_cl_clipper_xy(&instr,
        clipper_xy_scale * (float)vpw,
        clipper_xy_scale * (float)vph);

      v3d_cl_viewport_offset(&instr,
         viewport_offset_scale * (2*vpx + vpw),
         (viewport_offset_scale * (2*vpy + vph)));

      v3d_cl_clipper_z(&instr,
         0.5f * (state->viewport.vp_far - state->viewport.vp_near),
         0.5f * (state->viewport.vp_far + state->viewport.vp_near));

      v3d_cl_clipz(&instr,
         fminf(state->viewport.vp_near, state->viewport.vp_far),
         fmaxf(state->viewport.vp_near, state->viewport.vp_far));

      glxx_cl_record_end(cl_record, GLXX_CL_STATE_VIEWPORT, instr);
   }

   if (khrn_render_state_set_contains(state->dirty.sample_coverage, rs))
   {
      khrn_render_state_set_remove(&state->dirty.sample_coverage, rs);

      float sample_coverage = 1.0f; /* Disable */
      if (state->sample_coverage.enable)
      {
         /* Invert sets the sign bit. Can't write using floats because -0 == 0 */
         uint32_t cov_val = gfx_float_to_bits(state->sample_coverage.value);
         if (state->sample_coverage.invert) cov_val |= (1<<31);
         sample_coverage = gfx_float_from_bits(cov_val);
      }

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_SAMPLE_COVERAGE, instr);
      v3d_cl_sample_coverage(&instr, sample_coverage);
      glxx_cl_record_end(cl_record, GLXX_CL_STATE_SAMPLE_COVERAGE, instr);
   }

#if V3D_HAS_PER_RT_BLEND
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

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_BLEND_CFG, instr);
      write_blend_cfg(&instr, rs, state);
      glxx_cl_record_end(cl_record, GLXX_CL_STATE_BLEND_CFG, instr);
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

   if (khrn_render_state_set_contains(state->dirty.cfg, rs))
   {
      khrn_render_state_set_remove(&state->dirty.cfg, rs);

      V3D_CL_CFG_BITS_T cfg_bits;
      bool ms = khrn_options.force_multisample || (state->caps.multisample && hw_fb->ms);

      /* Switch blending off if logic_op is enabled */
      cfg_bits.blend = !(state->gl11.statebits.f_enable & GL11_LOGIC_M);
      /* Switch blending off if advanced blending is enabled */
      cfg_bits.blend = cfg_bits.blend && !glxx_advanced_blend_eqn_set(state);
#if !V3D_HAS_PER_RT_BLEND
      cfg_bits.blend = cfg_bits.blend && state->blend.enable;
#endif

      cfg_bits.stencil = stencil_test;

      cfg_bits.front_prims = !state->caps.cull_face || enable_front(state->cull_mode);
      cfg_bits.back_prims  = !state->caps.cull_face || enable_back(state->cull_mode);
      if (state->caps.rasterizer_discard || rasterizer_discard)
         cfg_bits.front_prims = cfg_bits.back_prims = false;

      cfg_bits.cwise_is_front = glxx_front_facing_is_clockwise(state->front_face);

      cfg_bits.depth_offset      = state->caps.polygon_offset_fill;
      cfg_bits.rast_oversample   = ms ? V3D_MS_4X : V3D_MS_1X; // TODO 16x?
      cfg_bits.depth_test        = depth_func;
      cfg_bits.depth_update      = depth_update;
      cfg_bits.ez                = rs->ez.cfg_bits_ez;
      cfg_bits.ez_update         = rs->ez.cfg_bits_ez_update;
      cfg_bits.aa_lines          = cfg_bits.rast_oversample;
      cfg_bits.wireframe_tris    = state->fill_mode != GL_FILL_BRCM;
      cfg_bits.wireframe_mode    = state->fill_mode == GL_POINT_BRCM ? V3D_WIREFRAME_MODE_POINTS : V3D_WIREFRAME_MODE_LINES;
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
      GLfloat units = state->polygon_offset.units;

      if(depth_lfmt != GFX_LFMT_NONE && gfx_lfmt_depth_bits(depth_lfmt) == 16)
      {
         /* The hardware always applies a constant amount of unit depth offset (2^-24),
            regardless of how much precission the depth buffer has. Therefore if we have
            a 16-bit depth buffer increase the amount of units to compensate. */
         units *= 1 << 8;
      }

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_POLYGON_OFFSET, instr);
      v3d_cl_depth_offset(&instr, state->polygon_offset.factor, units);
      glxx_cl_record_end(cl_record, GLXX_CL_STATE_POLYGON_OFFSET, instr);
   }

   if (khrn_render_state_set_contains(state->dirty.linewidth, rs))
   {
      khrn_render_state_set_remove(&state->dirty.linewidth, rs);

      /* For antialiasing we need to rasterise a bigger line because width is measured differently. */
      /* We might need a width of up to floor( sqrt(2) * width ) + 3 */
      float line_width;
      if (line_smooth)
         line_width = (float)floor(SQRT_2 * state->line_width) + 3;
      else
         line_width = state->line_width;

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_LINE_WIDTH, instr);
      v3d_cl_line_width(&instr, line_width);
      glxx_cl_record_end(cl_record, GLXX_CL_STATE_LINE_WIDTH, instr);
   }

end:
   // end write to cl (pass in actual end point)
   khrn_fmem_end_cle(fmem, instr);

   return true;
}

static bool insert_flat_shading_flags(GLXX_HW_RENDER_STATE_T *rs, GLXX_SERVER_STATE_T *state, const GLXX_LINK_RESULT_DATA_T *link_data)
{
   const uint32_t *flags = link_data->varying_flat;

   if (  !rs->flat_shading_flags_set
      || memcmp(rs->prev_flat_shading_flags, flags, sizeof(rs->prev_flat_shading_flags))
      )
   {
      uint8_t *instr = glxx_hw_render_state_begin_cle(rs, GLXX_CL_STATE_FLAT_SHADING_FLAGS);
      if (!instr)
         return false;;

      rs->flat_shading_flags_set = true;
      memcpy(rs->prev_flat_shading_flags, flags, sizeof(rs->prev_flat_shading_flags));

      v3d_flags_action_t lower_action = V3D_FLAGS_ACTION_ZERO;
      for (uint32_t i = 0; i < GLXX_NUM_SHADING_FLAG_WORDS; i++)
      {
         if (flags[i] != 0)
         {
            v3d_cl_flatshade_flags(&instr, i, lower_action, V3D_FLAGS_ACTION_ZERO, flags[i]);
            lower_action = V3D_FLAGS_ACTION_KEEP;
         }
      }
      if (lower_action != V3D_FLAGS_ACTION_KEEP)
      {
         v3d_cl_zero_all_flatshade_flags(&instr);
      }

      glxx_hw_render_state_end_cle(rs, GLXX_CL_STATE_FLAT_SHADING_FLAGS, instr);
   }
   return true;
}

static bool insert_centroid_flags(GLXX_HW_RENDER_STATE_T *rs, GLXX_SERVER_STATE_T *state, const GLXX_LINK_RESULT_DATA_T *link_data)
{
   const uint32_t *flags = link_data->varying_centroid;

   if (  !rs->centroid_flags_set
      || memcmp(rs->prev_centroid_flags, flags, sizeof(rs->prev_centroid_flags))
      )
   {
      uint8_t *instr = glxx_hw_render_state_begin_cle(rs, GLXX_CL_STATE_CENTROID_FLAGS);
      if (!instr)
         return false;;

      rs->centroid_flags_set = true;
      memcpy(rs->prev_centroid_flags, flags, sizeof(rs->prev_centroid_flags));

      v3d_flags_action_t lower_action = V3D_FLAGS_ACTION_ZERO;
      for (uint32_t i = 0; i < GLXX_NUM_SHADING_FLAG_WORDS; i++)
      {
         if (flags[i] != 0)
         {
            v3d_cl_centroid_flags(&instr, i, lower_action, V3D_FLAGS_ACTION_ZERO, flags[i]);
            lower_action = V3D_FLAGS_ACTION_KEEP;
         }
      }
      if (lower_action != V3D_FLAGS_ACTION_KEEP)
      {
         v3d_cl_zero_all_centroid_flags(&instr);
      }

      glxx_hw_render_state_end_cle(rs, GLXX_CL_STATE_CENTROID_FLAGS, instr);
   }
   return true;
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
      const GLXX_DRAW_T *draw,
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
   const GLXX_DRAW_T *draw,
   const GLXX_ATTRIBS_MAX *attribs_max,
   const GLXX_STORAGE_T *indices)
{
   v3d_addr_t indices_addr;
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
      v3d_cl_base_vertex_base_instance(&instr, draw->basevertex, draw->baseinstance);

   indices_addr = khrn_fmem_lock_and_sync(&rs->fmem, indices->handle, GMEM_SYNC_CLE_PRIMIND_READ, GMEM_SYNC_NONE);
   indices_addr += indices->offset;

   if (draw->instance_count > 1)
   {
      V3D_CL_INDEXED_INSTANCED_PRIM_LIST_T indexed_instanced_prim_list = {
         .prim_mode = mode,
         .index_type = index_type,
         .num_indices = draw->count,
         .prim_restart = state->caps.primitive_restart,
         .num_instances = draw->instance_count,
         .indices_addr = indices_addr,
         .max_index = attribs_max->index};
      v3d_cl_indexed_instanced_prim_list_indirect(&instr, &indexed_instanced_prim_list);
   }
   else
   {
      V3D_CL_INDEXED_PRIM_LIST_T indexed_prim_list = {
         .prim_mode = mode,
         .index_type = index_type,
         .num_indices = draw->count,
         .prim_restart = state->caps.primitive_restart,
         .indices_addr = indices_addr,
         .max_index = attribs_max->index};
      v3d_cl_indexed_prim_list_indirect(&instr, &indexed_prim_list);
   }

   khrn_fmem_end_cle_exact(&rs->fmem, instr);

   return true;
}

static bool glxx_hw_draw_indirect_record(
   GLXX_SERVER_STATE_T *state,
   GLXX_HW_RENDER_STATE_T *rs,
   const GLXX_DRAW_T *draw,
   const GLXX_ATTRIBS_MAX *attribs_max,
   const GLXX_STORAGE_T *indices,
   const GLXX_STORAGE_T *indirect)
{
   assert(draw->num_indirect > 0); /* Should not get this far if num_indirect is 0! */

   v3d_prim_mode_t mode = convert_primitive_type(state, draw->mode);
   unsigned size = V3D_CL_INDIRECT_PRIMITIVE_LIMITS_SIZE;
   v3d_addr_t indirect_addr = khrn_fmem_lock_and_sync(
      &rs->fmem, indirect->handle, GMEM_SYNC_CLE_DRAWREC_READ, GMEM_SYNC_NONE);
   indirect_addr += indirect->offset;

   // TODO v3d_cl_indirect_primitive_limits() can be omitted if buffers have not changed
   if (draw->is_draw_arrays)
      size += V3D_CL_INDIRECT_VERTEX_ARRAY_PRIMS_SIZE;
   else
      size += V3D_CL_INDIRECT_INDEXED_PRIM_LIST_SIZE;

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, size);
   if (!instr)
      return false;

   if (!draw->is_draw_arrays)
   {
      v3d_index_type_t index_type = convert_index_type(draw->index_type);
      unsigned type_size = glxx_get_index_type_size(draw->index_type);
      unsigned num_indices = (indices->size - indices->offset) / type_size;
      v3d_addr_t indices_addr = khrn_fmem_lock_and_sync(
         &rs->fmem, indices->handle, GMEM_SYNC_CLE_PRIMIND_READ, GMEM_SYNC_NONE);
      indices_addr += indices->offset;

      v3d_cl_indirect_primitive_limits(&instr, attribs_max->index, attribs_max->instance, num_indices);
      v3d_cl_indirect_indexed_prim_list(
         &instr, mode, index_type, draw->num_indirect, state->caps.primitive_restart,
         indirect_addr, indices_addr, draw->indirect_stride);
   }
   else
   {
      v3d_cl_indirect_primitive_limits(&instr, attribs_max->index, attribs_max->instance, 0);
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
bool glxx_hw_draw_triangles(
      GLXX_SERVER_STATE_T          *state,
      GLXX_HW_RENDER_STATE_T       *rs,
      const GLXX_DRAW_T            *draw,
      GLXX_ATTRIB_CONFIG_T          attrib_config[GLXX_CONFIG_MAX_VERTEX_ATTRIBS],
      const GLXX_ATTRIBS_MAX       *attribs_max,
      const GLXX_VERTEX_BUFFER_CONFIG_T *vb_config,
      const GLXX_STORAGE_T         *indices,
      const GLXX_VERTEX_POINTERS_T *vertex_pointers)
{
   /* create or retrieve shaders from cache and setup attribs_live */
   GLXX_LINK_RESULT_DATA_T* link_data = glxx_get_shaders(state);
   if (!link_data)
   {
      log_warn("[%s] shader compilation failed for %p", VCOS_FUNCTION, state);
      goto fail;
   }

#if V3D_VER_AT_LEAST(3,3,0,0)
   rs->fmem.br_info.bin_workaround_gfxh_1181    = false;
   rs->fmem.br_info.render_workaround_gfxh_1181 = false;
#else
   /* If using flow control, then driver needs to work around GFXH-1181. */
   rs->fmem.br_info.bin_workaround_gfxh_1181 |= link_data->bin_uses_control_flow;
   rs->fmem.br_info.render_workaround_gfxh_1181 |= link_data->render_uses_control_flow;

   /* For z-prepass the bin-mode shaders are also used during rendering */
   if (rs->z_prepass_allowed) rs->fmem.br_info.render_workaround_gfxh_1181 |= link_data->bin_uses_control_flow;
#endif

   /* Write the shader record */
   bool ok = write_gl_shader_record(
      rs,
      link_data,
      state,
      attrib_config,
      vb_config,
      vertex_pointers);
   if (!ok)
   {
      log_warn("%s: creating shader record failed", VCOS_FUNCTION);
      goto fail;
   }

   // update z-prepass state
   if (!glxx_hw_update_z_prepass_state(state, link_data, rs))
      goto fail;

   /* emit any necessary config change instructions */
   {
      bool nothing_to_draw = false;
      ok = do_changed_cfg(rs, state, &nothing_to_draw);
      ok &= insert_flat_shading_flags(rs, state, link_data);
      ok &= insert_centroid_flags(rs, state, link_data);
      if (!ok)
      {
         log_warn("%s: applying state changes failed",
               VCOS_FUNCTION);
         goto fail;
      }
      if (nothing_to_draw)
      {
         log_trace("%s: nothing to draw", VCOS_FUNCTION);
         return true;
      }
   }

   if (!glxx_server_queries_install(state, rs))
      goto fail;

   bool point_size_used = (link_data->flags & GLXX_SHADER_FLAGS_POINT_SIZE_SHADED_VERTEX_DATA) ==
      GLXX_SHADER_FLAGS_POINT_SIZE_SHADED_VERTEX_DATA;
   if (!glxx_server_tf_install(state, rs, point_size_used))
      goto fail;

#if !V3D_HAS_NEW_TF
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

#if !V3D_VER_AT_LEAST(3,3,0,0)
   // Instanced renders on 3.2 might encounter GFXH-1313.
   if (draw->instance_count > 1 || draw->is_indirect)
      rs->workaround_gfxh_1313 = true;
#endif


   bool record;
   if (!draw->is_indirect)
   {
      if (draw->is_draw_arrays)
         record = glxx_hw_draw_arrays_record(state, rs, draw, link_data);
      else
         record = glxx_hw_draw_elements_record(state, rs, draw, attribs_max, indices);
   }
   else
   {
      GLXX_BUFFER_T        *buffer;
      GLXX_STORAGE_T       indirect_storage;
      KHRN_RES_INTERLOCK_T *res_i;

      buffer = state->bound_buffer[GLXX_BUFTGT_DRAW_INDIRECT].obj;
      res_i = glxx_buffer_get_tf_aware_res_interlock(rs, buffer);
      indirect_storage.handle = res_i->handle;
      assert(draw->indirect_offset <= UINT_MAX);
      indirect_storage.offset = (unsigned int)draw->indirect_offset;

      khrn_fmem_record_res_interlock(&rs->fmem, res_i, false, ACTION_BOTH);

      record = glxx_hw_draw_indirect_record(
         state, rs, draw, attribs_max, indices, &indirect_storage);
   }

   if (!record)
   {
      log_warn("%s: draw record failed", VCOS_FUNCTION);
      goto fail;
   }

   glxx_server_tf_install_post_draw(state, rs);

#ifdef SIMPENROSE_WRITE_LOG
   // Increment batch count
   log_batchcount++;
#endif

   /* Update buffer states with rw operation */
   glxx_hw_render_state_rw(state, rs);

   if (draw->mode == GL_TRIANGLES || draw->mode == GL_TRIANGLE_STRIP ||
         draw->mode == GL_TRIANGLE_FAN)
   {
      khrn_render_state_set_remove(&state->dirty.stuff, rs);
   }

   // save current GPU state following this draw
   if (GLXX_MULTICORE_BIN_ENABLED && rs->do_multicore_bin && rs->tf.waited_count == 0
#if V3D_HAS_NEW_TF
         && (khrn_fmem_get_common_persist(&rs->fmem)->prim_counts_query_list == NULL)
#endif
       )
   {
      if (!post_draw_journal(rs, attrib_config, vb_config, vertex_pointers, link_data, draw))
         goto fail;
   }

   return true;

fail:
   glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
   glxx_hw_discard_frame(rs);

   return false;
}

/*************************************************************
 Static Functions
 *************************************************************/

static uint64_t estimate_bin_cost(
   const GLXX_ATTRIB_CONFIG_T    *attrib,
   const GLXX_VERTEX_BUFFER_CONFIG_T *vb_config,
   const GLXX_VERTEX_POINTERS_T *vertex_pointers,
   const GLXX_LINK_RESULT_DATA_T *link_data,
   const GLXX_DRAW_T             *draw
   )
{
    // hardware SoL figures
   uint32_t const c_instr_per_cycle = khrn_get_num_qpus_per_core()*4;
   uint32_t const c_bytes_per_cycle = 16;

   // VCD is also limited (per-cycle) to
   // - 16 bytes per cycle peak bandwidth
   // - 2 attribute-vectors
   // - 1 cache-line check (but can reuse check from previous attribute)

   // estimate bandwidth used with following caveats:
   // - assume whole vertex is fetched in each stream (even unused attributes)
   // - ignore effect of VCD cache and cache-line size (so we assume fetching whole vertex for each vertex processed)
   uint32_t vertex_size = 0;
   uint32_t attrib_remaining = (1 << link_data->attr_count) - 1;
   while (attrib_remaining)
   {
      gmem_handle_t this_buffer = GMEM_HANDLE_INVALID;
      uint32_t this_stride = 0;

      for (unsigned n = 0; n != link_data->attr_count; ++n)
      {
         if (link_data->attr[n].c_scalars_used > 0 && attrib_remaining & (1 << n))
         {
            const int i = link_data->attr[n].idx;
            if (attrib[i].enabled)
            {
               gmem_handle_t buffern = vertex_pointers->array[i].handle;
               if (this_buffer == GMEM_HANDLE_INVALID)
               {
                  this_buffer = buffern;
                  this_stride = vb_config[i].stride;
               }
               else if (this_buffer != buffern && this_stride != vb_config[i].stride)
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
   uint64_t bin_cost = vcos_max(bandwidth_cycles, instr_cycles);
   return bin_cost;
}

static bool post_draw_journal(
   GLXX_HW_RENDER_STATE_T        *rs,
   const GLXX_ATTRIB_CONFIG_T    *attrib,
   const GLXX_VERTEX_BUFFER_CONFIG_T *vb_config,
   const GLXX_VERTEX_POINTERS_T  *vertex_pointers,
   const GLXX_LINK_RESULT_DATA_T *link_data,
   const GLXX_DRAW_T             *draw
   )
{
   // should only call this if multi-core bin is compiled in
   assert(GLXX_MULTICORE_BIN_ENABLED && rs->do_multicore_bin);

   // get estimated cost of bin in cycles
   uint64_t const bin_cost = estimate_bin_cost(attrib, vb_config, vertex_pointers, link_data, draw);

   // increment bin-cost in current draw record
   GLXX_CL_RECORD_T* cl_record = &rs->cl_records[rs->num_cl_records];
   cl_record->bin_cost_cumulative += bin_cost;

   // validate current record
   assert(glxx_cl_record_validate(cl_record));

   if (rs->cl_record_remaining <= bin_cost)
   {
      // add nop we can patch later
      uint8_t* instr = khrn_fmem_begin_cle(&rs->fmem, V3D_CL_NOP_SIZE);
      if (!instr)
         return false;
      v3d_cl_nop(&instr);
      khrn_fmem_end_cle_exact(&rs->fmem, instr);

      cl_record->start_sub_addr = instr;

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

GLXX_LINK_RESULT_DATA_T *glxx_get_shaders(GLXX_SERVER_STATE_T *state)
{
   GLXX_LINK_RESULT_DATA_T *result;
   GLXX_BINARY_CACHE_T     *cache;
   IR_PROGRAM_T            *ir;

   assert(IS_GL_11(state) || gl20_program_common_get(state)->linked_glsl_program->ir != NULL);

   if (IS_GL_11(state))
   {
      GL11_CACHE_ENTRY_T *e;

      /* Get the ES1 equivalent of a program from the cache */
      e = gl11_get_backend_cache(state->gl11.shader_cache, &state->gl11.shaderkey);
      cache = &e->backend_cache;
      ir =  e->blob;
   }
   else
   {
      GL20_PROGRAM_COMMON_T *program_common = gl20_program_common_get(state);
      cache = &program_common->cache;
      ir    = program_common->linked_glsl_program->ir;
   }

   /* Try to get a previous binary from the cache */
   result = glxx_binary_cache_get_shaders(cache, &state->shaderkey_common);

   if (result == NULL) {
      khrn_stats_record_event(KHRN_STATS_SHADER_CACHE_MISS);
      /* If the cache lookup failed, make a new shader and put it in the cache */
      result = glxx_get_shaders_and_cache(cache, ir, &state->shaderkey_common);
   }

   return result;
}

static uint32_t assemble_tmu_param(uint32_t param, uint32_t u_value,
                                   const GLXX_TEXTURE_UNIF_T *tus)
{
   assert(param == 0 || param == 1);

#if V3D_HAS_NEW_TMU_CFG
   uint32_t sampler = backend_uniform_get_sampler(u_value);
   uint32_t bits = (sampler == GLSL_SAMPLER_NONE) ? 0 : tus[sampler].hw_param[param];
   uint32_t extra_bits = backend_uniform_get_extra(u_value);

   assert(param == 1 || sampler != GLSL_SAMPLER_NONE);
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
      } else
         bits = tus[sampler].hw_param[1];

      extra_bits = backend_uniform_get_extra_param1(u_value);
   }

   assert(!(bits & extra_bits));
   return (bits | extra_bits);
#endif
}

v3d_addr_t glxx_hw_install_uniforms(
   glxx_render_state                *rs,
   const GLXX_SERVER_STATE_T        *state,
   const GLXX_UNIFORM_MAP_T         *map,
   GL20_HW_INDEXED_UNIFORM_T        *iu,
   const glxx_hw_compute_uniforms   *compute_uniforms)
{
   khrn_fmem* fmem = &rs->fmem;
   uint32_t *ptr = khrn_fmem_data(fmem, 4 * map->count, V3D_QPU_UNIFS_ALIGN);
   if (!ptr)
      return 0;
   v3d_addr_t result = khrn_fmem_hw_address(fmem, ptr);

   uint32_t *data;
   if (!IS_GL_11(state)) data = gl20_program_common_get(state)->uniform_data;
   else                  data = (uint32_t *)&state->gl11;

   /* Install uniforms */
   for (unsigned i = 0; i < map->count; i++)
   {
      const BackendUniformFlavour u_type  = map->entry[2 * i];
      const uint32_t              u_value = map->entry[2 * i + 1];

      assert(u_type >= 0 && u_type <= BACKEND_UNIFORM_LAST_ELEMENT);

      switch(u_type) {

      case BACKEND_UNIFORM_LITERAL:
         *ptr = u_value;
         break;

      case BACKEND_UNIFORM_PLAIN:
         *ptr = data[u_value];
         break;

      case BACKEND_UNIFORM_UBO_ADDRESS:
      case BACKEND_UNIFORM_SSBO_ADDRESS:
      case BACKEND_UNIFORM_ATOMIC_ADDRESS:
      {
         /* Not having a buffer bound gives undefined behaviour - we crash */
         const GLXX_INDEXED_BINDING_POINT_T *binding;
         bool write = false;
         int offset = 0;
         if (u_type == BACKEND_UNIFORM_SSBO_ADDRESS) {
            uint32_t binding_point = gl20_program_common_get(state)->ssbo_binding_point[u_value];
            binding                = &state->ssbo.binding_points[binding_point];
            write = true;
         } else if (u_type == BACKEND_UNIFORM_ATOMIC_ADDRESS) {
            uint32_t binding_point = u_value >> 16;
            binding = &state->atomic_counter.binding_points[binding_point];
            offset = u_value & 0xFFFF;
            write = true;
         } else {
            uint32_t binding_point = gl20_program_common_get(state)->ubo_binding_point[u_value];
            binding                = &state->uniform_block.binding_points[binding_point];
         }
         if (write)
            rs->has_buffer_writes = true;
         GLXX_BUFFER_T *buffer = binding->buffer.obj;
         KHRN_RES_INTERLOCK_T *res_i =
            (((KHRN_RENDER_STATE_T *)rs)->type == KHRN_RENDER_STATE_TYPE_GLXX) ?
            glxx_buffer_get_tf_aware_res_interlock((GLXX_HW_RENDER_STATE_T *)rs, buffer) :
            glxx_buffer_get_res_interlock(buffer);

         khrn_fmem_record_res_interlock(fmem, res_i, write, ACTION_BOTH);

         v3d_addr_t uniform_addr;
         uint32_t action = GMEM_SYNC_TMU_DATA_READ | (write ? GMEM_SYNC_TMU_DATA_WRITE : 0);
         uniform_addr = khrn_fmem_lock_and_sync(fmem, res_i->handle, action, action);
         *ptr = uniform_addr + binding->offset + offset;
         break;
      }

      case BACKEND_UNIFORM_SSBO_SIZE:
      {
         /* Not having a buffer bound gives undefined behaviour - we crash */
         const GLXX_INDEXED_BINDING_POINT_T *binding;
         uint32_t binding_point = gl20_program_common_get(state)->ssbo_binding_point[u_value];
         binding = &state->ssbo.binding_points[binding_point];
         size_t size = glxx_indexed_binding_point_get_size(binding);
         if (binding->offset < size)
            *ptr = size - binding->offset;
         else
         {
            /* if offset is beyond binding or buffer size, use 0 as size */
            *ptr = 0;
         }
         break;
      }

      case BACKEND_UNIFORM_TEX_PARAM1:
         *ptr = assemble_tmu_param(1, u_value, state->texture_unif);
         break;
      case BACKEND_UNIFORM_TEX_PARAM0:
         *ptr = assemble_tmu_param(0, u_value, state->texture_unif);
         break;
      case BACKEND_UNIFORM_TEX_SIZE_X:
         *ptr = state->texture_unif[u_value].width;
         break;
      case BACKEND_UNIFORM_TEX_SIZE_Y:
         *ptr = state->texture_unif[u_value].height;
         break;
      case BACKEND_UNIFORM_TEX_SIZE_Z:
         *ptr = state->texture_unif[u_value].depth;
         break;
#if !V3D_HAS_NEW_TMU_CFG
      case BACKEND_UNIFORM_TEX_BASE_LEVEL:
         *ptr = state->texture_unif[u_value].base_level;
         break;
      case BACKEND_UNIFORM_TEX_BASE_LEVEL_FLOAT:
         *ptr = gfx_float_to_bits((float)state->texture_unif[u_value].base_level);
         break;
#endif

      case BACKEND_UNIFORM_IMAGE_PARAM1:
         *ptr = assemble_tmu_param(1, u_value, state->image_unif);
         break;
      case BACKEND_UNIFORM_IMAGE_PARAM0:
         *ptr = assemble_tmu_param(0, u_value, state->image_unif);
         break;
#if !V3D_HAS_TMU_TEX_WRITE
      case BACKEND_UNIFORM_IMAGE_SWIZZLING:
         *ptr = (uint32_t)state->image_unif[u_value].lx_swizzling;
         break;
      case BACKEND_UNIFORM_IMAGE_LX_ADDR:
         *ptr = state->image_unif[u_value].lx_addr;
         break;
      case BACKEND_UNIFORM_IMAGE_LX_PITCH:
         *ptr = state->image_unif[u_value].lx_pitch;
         break;
      case BACKEND_UNIFORM_IMAGE_LX_SLICE_PITCH:
         *ptr = state->image_unif[u_value].lx_slice_pitch;
         break;
      case BACKEND_UNIFORM_IMAGE_ARR_STRIDE:
         *ptr = state->image_unif[u_value].arr_stride;
         break;
#endif
      case BACKEND_UNIFORM_IMAGE_LX_WIDTH:
         *ptr = state->image_unif[u_value].width;
         break;
      case BACKEND_UNIFORM_IMAGE_LX_HEIGHT:
         *ptr = state->image_unif[u_value].height;
         break;
      case BACKEND_UNIFORM_IMAGE_LX_DEPTH:
         *ptr = state->image_unif[u_value].depth;
         break;

      case BACKEND_UNIFORM_SPECIAL:
      {
         const BackendSpecialUniformFlavour u_special = u_value;

         assert(u_special >= 0 && u_special <= BACKEND_SPECIAL_UNIFORM_LAST_ELEMENT);

         switch (u_special) {

         case BACKEND_SPECIAL_UNIFORM_VP_SCALE_X:
            *ptr = gfx_float_to_bits(state->viewport.internal_xscale);
            break;

         case BACKEND_SPECIAL_UNIFORM_VP_SCALE_Y:
            *ptr = gfx_float_to_bits(state->viewport.internal_yscale);
            break;

         case BACKEND_SPECIAL_UNIFORM_VP_SCALE_Z:
            *ptr = gfx_float_to_bits(state->viewport.internal_zscale);
            break;

         case BACKEND_SPECIAL_UNIFORM_VP_OFFSET_Z:
            *ptr = gfx_float_to_bits(state->viewport.internal_zoffset);
            break;

         case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_NEAR:
            *ptr = gfx_float_to_bits(state->viewport.internal_dr_near);
            break;

         case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_FAR:
            *ptr = gfx_float_to_bits(state->viewport.internal_dr_far);
            break;

         case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_DIFF:
            *ptr = gfx_float_to_bits(state->viewport.internal_dr_diff);
            break;

         case BACKEND_SPECIAL_UNIFORM_SAMPLE_MASK:
            *ptr = state->sample_mask.mask[0];
            break;

         case BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_X:
            *ptr = compute_uniforms->num_work_groups[0];
            break;

         case BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_Y:
            *ptr = compute_uniforms->num_work_groups[1];
            break;

         case BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_Z:
            *ptr = compute_uniforms->num_work_groups[2];
            break;

         case BACKEND_SPECIAL_UNIFORM_QUORUM:
            *ptr = compute_uniforms->quorum;
            break;

         case BACKEND_SPECIAL_UNIFORM_SHARED_PTR:
            *ptr = compute_uniforms->shared_ptr;
            break;
         }
         break;
      }

      case BACKEND_UNIFORM_ADDRESS:
         if (!backend_uniform_address(fmem, u_value, gl20_program_common_get(state), iu, ptr)) {
            return 0;
         }
         break;

      case BACKEND_UNIFORM_UNASSIGNED:
         unreachable();
         break;
      }

      ptr++;
   }

   return result;
}

static bool backend_uniform_address(
   KHRN_FMEM_T *fmem,
   uint32_t u_value,
   const GL20_PROGRAM_COMMON_T *program_common,
   GL20_HW_INDEXED_UNIFORM_T *iu,
   uint32_t *location)
{
   uint32_t index = u_value & 0xffff;

   if (!iu->valid) {
      v3d_addr_t texa;
      uint32_t *tex;
      /* Copy the whole default uniform block into fmem */
      tex = khrn_fmem_data(fmem, 4*program_common->num_scalar_uniforms, 16);
      if (tex == NULL) return false;

      memcpy(tex, program_common->uniform_data, 4*program_common->num_scalar_uniforms);
      texa = khrn_fmem_hw_address(fmem, tex);
      iu->block_addr = texa;
      iu->valid = true;
   }

   *location = iu->block_addr + 4*index;    /* XXX Am I allowed fmem-pointer arithmetic? */
   return true;
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

#if V3D_HAS_NEW_TF
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
   KHRN_FMEM_T*                     fmem,
   const GLXX_LINK_RESULT_DATA_T*   link_data,
   const GLXX_ATTRIB_CONFIG_T*      attr_cfgs)
{
   uint32_t *defaults = NULL;
   *defaults_addr = 0;

   assert(link_data->attr_count <= GLXX_CONFIG_MAX_VERTEX_ATTRIBS);

   // Iterate backwards, we'll allocate space for all defaults
   // from the last default down when required.
   for (unsigned n = link_data->attr_count; n-- != 0; )
   {
      const GLXX_ATTRIB_CONFIG_T *attr_cfg = &attr_cfgs[link_data->attr[n].idx];
      if (attr_cfg->enabled)
      {
         if (  link_data->attr[n].c_scalars_used > attr_cfg->size
            || link_data->attr[n].v_scalars_used > attr_cfg->size )
         {
            if (!defaults)
            {
               defaults = khrn_fmem_data(fmem, (n + 1)*16, V3D_ATTR_DEFAULTS_ALIGN);
               if (!defaults)
                  return false;
               *defaults_addr = khrn_fmem_hw_address(fmem, defaults);
            }

            defaults[n*4 + 0] = 0u; // or 0.0f
            defaults[n*4 + 1] = 0u; // or 0.0f
            defaults[n*4 + 2] = 0u; // or 0.0f
            defaults[n*4 + 3] = attr_cfg->is_int ? 1u : 0x3f800000; // or 1.0f
         }
      }
   }
   return true;
}

static bool write_gl_shader_record_attribs(
   uint32_t*                           packed_attrs,
   v3d_addr_t                          defaults_addr,
   KHRN_FMEM_T*                        fmem,
   const GLXX_LINK_RESULT_DATA_T*      link_data,
   const GLXX_ATTRIB_CONFIG_T*         attr_cfgs,
   const GLXX_VERTEX_BUFFER_CONFIG_T*  vb_cfgs,
   const GLXX_VERTEX_POINTERS_T*       vb_pointers,
   const GLXX_GENERIC_ATTRIBUTE_T*     generic_attrs)
{
   // Workaround GFXH-930, must have at least 1 attribute:
   if (link_data->attr_count == 0)
   {
      uint32_t *dummy_data = khrn_fmem_data(fmem, sizeof(uint32_t), V3D_ATTR_ALIGN);
      if (!dummy_data)
         return false;
      dummy_data[0] = 0;

      V3D_SHADREC_GL_ATTR_T attr = {
         .addr            = khrn_fmem_hw_address(fmem, dummy_data),
         .size            = 1,
         .type            = V3D_ATTR_TYPE_FLOAT,
         .signed_int      = false,
         .normalised_int  = false,
         .read_as_int     = false,
         .cs_num_reads    = 1,
         .vs_num_reads    = 1,
         .divisor         = 0,
         .stride          = 0,
      };
      v3d_pack_shadrec_gl_attr(packed_attrs, &attr);
      return true;
   }

   unsigned cs_total_reads = 0;
   unsigned vs_total_reads = 0;
   for (unsigned n = 0; n < link_data->attr_count; n++)
   {
      unsigned cs_num_reads = link_data->attr[n].c_scalars_used;
      unsigned vs_num_reads = link_data->attr[n].v_scalars_used;
      assert(cs_num_reads > 0 || vs_num_reads > 0); // attribute entry shouldn't exist if there are no reads.
      cs_total_reads += cs_num_reads;
      vs_total_reads += vs_num_reads;

      // Workaround GFXH-930:
      if (n == (link_data->attr_count-1) && (cs_total_reads == 0 || vs_total_reads == 0))
      {
         // We read all attributes either in CS or VS, bodge the last attribute
         // to be read by both.
         assert(cs_num_reads == 0 || vs_num_reads == 0);
         cs_num_reads |= vs_num_reads;
         vs_num_reads = cs_num_reads;
      }

      const unsigned vb_idx = link_data->attr[n].idx;
      const GLXX_ATTRIB_CONFIG_T *attr_cfg = &attr_cfgs[vb_idx];
      if (attr_cfg->enabled)
      {
         const GLXX_STORAGE_T *vertex_p = &vb_pointers->array[vb_idx];
         assert(vertex_p->handle != GMEM_HANDLE_INVALID);

         v3d_addr_t attrib_addr = khrn_fmem_lock_and_sync(
            fmem, vertex_p->handle, GMEM_SYNC_VCD_READ, GMEM_SYNC_VCD_READ);

         uint32_t divisor = vb_cfgs[vb_idx].divisor;
         uint32_t stride = vb_cfgs[vb_idx].stride;
         if (divisor == 0xffffffff)
         {
            // Handle special divisor case.
            divisor = 1u;
            stride = 0u;
         }

         V3D_SHADREC_GL_ATTR_T attr = {
            .addr             = attrib_addr + vertex_p->offset,
            .size             = attr_cfg->size,
            .type             = attr_cfg->v3d_type,
            .signed_int       = attr_cfg->is_signed,
            .normalised_int   = attr_cfg->norm,
            .read_as_int      = attr_cfg->is_int,
            .cs_num_reads     = cs_num_reads,
            .vs_num_reads     = vs_num_reads,
            .divisor          = divisor,
            .stride           = stride,
         };
         v3d_pack_shadrec_gl_attr(packed_attrs + V3D_SHADREC_GL_ATTR_PACKED_SIZE/sizeof(uint32_t)*n, &attr);
      }
      else
      {
         const GLXX_GENERIC_ATTRIBUTE_T *generic = &generic_attrs[vb_idx];
         uint32_t* generic_data = khrn_fmem_data(fmem, sizeof(uint32_t)*4u, V3D_ATTR_ALIGN);
         if (!generic_data)
            return false;
         generic_data[0] = generic->u[0];
         generic_data[1] = generic->u[1];
         generic_data[2] = generic->u[2];
         generic_data[3] = generic->u[3];

         V3D_SHADREC_GL_ATTR_T attr = {
            .addr            = khrn_fmem_hw_address(fmem, generic_data),
            .size            = 4,
            .type            = generic->type,
            .signed_int      = generic->is_signed,
            .normalised_int  = false,
            .read_as_int     = generic->type != V3D_ATTR_TYPE_FLOAT,
            .cs_num_reads    = cs_num_reads,
            .vs_num_reads    = vs_num_reads,
            .divisor         = 0,
            .stride          = 0,
         };
         v3d_pack_shadrec_gl_attr(packed_attrs + V3D_SHADREC_GL_ATTR_PACKED_SIZE/sizeof(uint32_t)*n, &attr);
      }
   }

   return true;
}

#if GLXX_HAS_TNG
static bool compute_tng_vpm_cfg(
   v3d_vpm_cfg_v* cfg_v,
   uint32_t shadrec_tg_packed[],
   GLXX_LINK_RESULT_DATA_T const* link_data,
   unsigned num_patch_vertices)
{
   v3d_vpm_cfg_t vpm_t;
   v3d_vpm_cfg_g vpm_g;
   if (!link_data->has_tess) v3d_vpm_default_cfg_t(&vpm_t);
   if (!link_data->has_geom) v3d_vpm_default_cfg_g(&vpm_g);

   bool ok = v3d_vpm_compute_cfg_tg(
      cfg_v,
      link_data->has_tess ? &vpm_t : NULL,
      link_data->has_geom ? &vpm_g : NULL,
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
   if (!ok)
      return false;

   V3D_SHADREC_GL_TESS_OR_GEOM_T shadrec_tog =
   {
      .tess_type                                = link_data->tess_type,
      .tess_point_mode                          = link_data->tess_point_mode,
      .tess_edge_spacing                        = link_data->tess_edge_spacing,
      .tess_clockwise                           = link_data->tess_clockwise,
      //.tcs_bypass                             = todo_not_implemented,
      //.tcs_bypass_render                      = todo_not_implemented,
      .tcs_batch_flush                          = vpm_t.tcs_batch_flush,
      .num_tcs_invocations                      = gfx_umax(link_data->tcs_output_vertices_per_patch, 1u),
      .geom_output                              = link_data->geom_prim_type,
      .geom_num_instances                       = gfx_umax(link_data->geom_invocations, 1u),
      .per_patch_depth_bin                      = vpm_t.per_patch_depth[0],
      .per_patch_depth_render                   = vpm_t.per_patch_depth[1],
      .tcs_output_bin                           = vpm_t.tcs_output[0],
      .tcs_output_render                        = vpm_t.tcs_output[1],
      .tes_output_bin                           = vpm_t.tes_output[0],
      .tes_output_render                        = vpm_t.tes_output[1],
      .geom_output_bin                          = vpm_g.geom_output[0],
      .geom_output_render                       = vpm_g.geom_output[1],
      .max_patches_per_tcs_batch                = vpm_t.max_patches_per_tcs_batch,
      .max_extra_vert_segs_per_tcs_batch_bin    = vpm_t.max_extra_vert_segs_per_tcs_batch[0],
      .max_extra_vert_segs_per_tcs_batch_render = vpm_t.max_extra_vert_segs_per_tcs_batch[1],
      .min_tcs_segs_bin                         = vpm_t.min_tcs_segs[0],
      .min_tcs_segs_render                      = vpm_t.min_tcs_segs[1],
      .min_per_patch_segs_bin                   = vpm_t.min_per_patch_segs[0],
      .min_per_patch_segs_render                = vpm_t.min_per_patch_segs[1],
      .max_patches_per_tes_batch                = vpm_t.max_patches_per_tes_batch,
      .max_extra_vert_segs_per_tes_batch_bin    = vpm_t.max_extra_vert_segs_per_tes_batch[0],
      .max_extra_vert_segs_per_tes_batch_render = vpm_t.max_extra_vert_segs_per_tes_batch[1],
      .max_tcs_segs_per_tes_batch_bin           = vpm_t.max_tcs_segs_per_tes_batch[0],
      .max_tcs_segs_per_tes_batch_render        = vpm_t.max_tcs_segs_per_tes_batch[1],
      .min_tes_segs_bin                         = vpm_t.min_tes_segs[0],
      .min_tes_segs_render                      = vpm_t.min_tes_segs[1],
      .max_extra_vert_segs_per_gs_batch_bin     = vpm_g.max_extra_vert_segs_per_gs_batch[0],
      .max_extra_vert_segs_per_gs_batch_render  = vpm_g.max_extra_vert_segs_per_gs_batch[1],
      .min_gs_segs_bin                          = vpm_g.min_gs_segs[0],
      .min_gs_segs_render                       = vpm_g.min_gs_segs[1],
   };

   v3d_pack_shadrec_gl_tess_or_geom(shadrec_tg_packed, &shadrec_tog);
   return true;
}
#endif

static bool write_gl_shader_record(
   GLXX_HW_RENDER_STATE_T              *rs,
   GLXX_LINK_RESULT_DATA_T             *link_data,
   const GLXX_SERVER_STATE_T           *state,
   const GLXX_ATTRIB_CONFIG_T          *attr_cfgs,
   const GLXX_VERTEX_BUFFER_CONFIG_T   *vb_cfgs,
   const GLXX_VERTEX_POINTERS_T        *vb_pointers)
{
   v3d_addr_t defaults_addr;
   if (!write_gl_shader_record_defaults(&defaults_addr, &rs->fmem, link_data, attr_cfgs))
      return false;

   rs->has_tcs_barriers |= !!(link_data->flags & GLXX_SHADER_FLAGS_TCS_BARRIERS);

   // Bin/Render vertex pipeline shaders.
   v3d_addr_t vp_shader_addrs[GLXX_SHADER_VPS_COUNT][MODE_COUNT];
   for (unsigned m = 0; m != MODE_COUNT; ++m)
   {
      gmem_sync_flags_t rdr_flags = GMEM_SYNC_QPU_INSTR_READ;
      gmem_sync_flags_t bin_flags = (m == MODE_BIN) ? GMEM_SYNC_QPU_INSTR_READ : 0u;
      khrn_interlock_action_t action = (m == MODE_BIN) ? ACTION_BOTH : ACTION_RENDER;

      for (unsigned s = 0; s != GLXX_SHADER_VPS_COUNT; ++s)
      {
         if (link_data->vps[s][m].res_i != NULL)
         {
            vp_shader_addrs[s][m] = khrn_fmem_lock_and_sync(&rs->fmem, link_data->vps[s][m].res_i->handle, bin_flags, rdr_flags);
            if (!khrn_fmem_record_res_interlock(&rs->fmem, link_data->vps[s][m].res_i, false, action))
               return false;
         }
      }
   }

   // Fragment shader.
   v3d_addr_t frag_shader_addr = khrn_fmem_lock_and_sync(&rs->fmem, link_data->fs.res_i->handle, 0, GMEM_SYNC_QPU_INSTR_READ);
   if (!khrn_fmem_record_res_interlock(&rs->fmem, link_data->fs.res_i, false, ACTION_RENDER))
      return false;

   GL20_HW_INDEXED_UNIFORM_T iu;
   iu.valid = false;

   // Bin/Render vertex pipeline uniforms.
   v3d_addr_t vp_unifs_addrs[GLXX_SHADER_VPS_COUNT][MODE_COUNT];
   for (unsigned m = 0; m != MODE_COUNT; ++m)
   {
      for (unsigned s = 0; s != GLXX_SHADER_VPS_COUNT; ++s)
      {
         if (link_data->vps[s][m].res_i != NULL)
         {
            vp_unifs_addrs[s][m] = glxx_hw_install_uniforms(&rs->base, state, link_data->vps[s][m].uniform_map, &iu, NULL);
            if (!vp_unifs_addrs[s][m])
               return false;
         }
      }
   }

   // Fragment shader uniforms.
   v3d_addr_t frag_unifs_addr = glxx_hw_install_uniforms(&rs->base, state, link_data->fs.uniform_map, &iu, NULL);
   if (!frag_unifs_addr)
      return false;

   // Treat tessellation part of record as two geometry shaders.
   static_assrt(V3D_SHADREC_GL_GEOM_PACKED_SIZE*2 == V3D_SHADREC_GL_TESS_PACKED_SIZE);

   unsigned num_attrs = gfx_umax(1u, link_data->attr_count);
   unsigned shadrec_size =
#if GLXX_HAS_TNG
      +  V3D_SHADREC_GL_GEOM_PACKED_SIZE
      +  V3D_SHADREC_GL_TESS_PACKED_SIZE
      +  V3D_SHADREC_GL_TESS_OR_GEOM_PACKED_SIZE
#endif
      +  V3D_SHADREC_GL_MAIN_PACKED_SIZE
      +  V3D_SHADREC_GL_ATTR_PACKED_SIZE*num_attrs;

   uint32_t* shadrec_data = khrn_fmem_begin_data(&rs->fmem, shadrec_size, V3D_SHADREC_ALIGN);
   if (!shadrec_data)
      return false;
   v3d_addr_t shadrec_addr = khrn_fmem_hw_address(&rs->fmem, shadrec_data);

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
      if (link_data->vps[s][MODE_RENDER].res_i != NULL)
      {
         V3D_SHADREC_GL_GEOM_T stage_shadrec =
         {
            .gs_bin.threading          = link_data->vps[s][MODE_BIN].threading,
            .gs_bin.propagate_nans     = true,
            .gs_bin.addr               = vp_shader_addrs[s][MODE_BIN],
            .gs_bin.unifs_addr         = vp_unifs_addrs[s][MODE_BIN],
            .gs_render.threading       = link_data->vps[s][MODE_RENDER].threading,
            .gs_render.propagate_nans  = true,
            .gs_render.addr            = vp_shader_addrs[s][MODE_RENDER],
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

         bool ok = compute_tng_vpm_cfg(
            &link_data->cached_vpm_cfg.vpm_cfg_v,
            link_data->cached_vpm_cfg.shadrec_tg_packed,
            link_data,
            state->num_patch_vertices);
         assert(ok); // GL requirements mean this should always succeed.
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
            &link_data->cached_vpm_cfg.vpm_cfg_v,
            khrn_get_vpm_size() / 512,
            link_data->vs_input_words,
            link_data->vs_output_words,
            z_pre_pass);
      }
   }

   v3d_vpm_cfg_v const* vpm_cfg_v = &link_data->cached_vpm_cfg.vpm_cfg_v;

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
      .scb_wait_on_first_thrsw   = !!(link_data->flags & GLXX_SHADER_FLAGS_TLB_WAIT_FIRST_THRSW),
      .disable_scb               = false,
#if V3D_HAS_VARY_DISABLE
      .disable_implicit_varys    = false,
#endif
#if V3D_HAS_TNG      /* TNG and BASEINSTANCE are the same hw revision */
      .cs_baseinstance = !!(link_data->flags & (GLXX_SHADER_FLAGS_VS_READS_BASE_INSTANCE << MODE_BIN)),
      .vs_baseinstance = !!(link_data->flags & (GLXX_SHADER_FLAGS_VS_READS_BASE_INSTANCE << MODE_RENDER)),
#endif
#if V3D_HAS_INLINE_CLIP
      .prim_id_used  = !!(link_data->flags & GLXX_SHADER_FLAGS_PRIM_ID_USED),
      .prim_id_to_fs = !!(link_data->flags & GLXX_SHADER_FLAGS_PRIM_ID_TO_FS),
#endif
   // TODO centroid flag
#if V3D_HAS_SRS
      .sample_rate_shading = (link_data->flags & GLXX_SHADER_FLAGS_PER_SAMPLE) ||
                             (state->caps.sample_shading && state->sample_shading_fraction >= 0.25f) ||
         ((khrn_options.force_multisample || rs->installed_fb.ms) && khrn_options_make_sample_rate_shaded()),
#endif
      .num_varys = link_data->num_varys,
      .cs_output_size = vpm_cfg_v->output_size[MODE_BIN],
      .cs_input_size = vpm_cfg_v->input_size[MODE_BIN],
      .vs_output_size = vpm_cfg_v->output_size[MODE_RENDER],
      .vs_input_size = vpm_cfg_v->input_size[MODE_RENDER],
      .defaults = defaults_addr,
      .fs.threading = link_data->fs.threading,
      .fs.propagate_nans = true,
      .fs.addr = frag_shader_addr,
      .fs.unifs_addr = frag_unifs_addr,
      .vs.threading = link_data->vps[GLXX_SHADER_VPS_VS][MODE_RENDER].threading,
      .vs.propagate_nans = true,
      .vs.addr = vp_shader_addrs[GLXX_SHADER_VPS_VS][MODE_RENDER],
      .vs.unifs_addr = vp_unifs_addrs[GLXX_SHADER_VPS_VS][MODE_RENDER],
      .cs.threading = link_data->vps[GLXX_SHADER_VPS_VS][MODE_BIN].threading,
      .cs.propagate_nans = true,
      .cs.addr = vp_shader_addrs[GLXX_SHADER_VPS_VS][MODE_BIN],
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
      attr_cfgs,
      vb_cfgs,
      vb_pointers,
      state->generic_attrib);
   if (!ok)
      return false;

   // Convert bits from active stage mask to CL opcode.
   unsigned tng_stage_mask = 0;
#if GLXX_HAS_TNG
   tng_stage_mask |= link_data->has_tess;
   tng_stage_mask |= link_data->has_geom << 1;
#endif
   static_assrt(V3D_CL_GL_T_SHADER == V3D_CL_GL_SHADER+1);
   static_assrt(V3D_CL_GL_G_SHADER == V3D_CL_GL_SHADER+2);
   static_assrt(V3D_CL_GL_TG_SHADER == V3D_CL_GL_SHADER+3);
   uint32_t shader_cl_opcode = V3D_CL_GL_SHADER + tng_stage_mask;

   // Finally, write shader-record control list item.
   uint8_t* cl = khrn_fmem_begin_cle(&rs->fmem, V3D_CL_VCM_CACHE_SIZE_SIZE + V3D_CL_GL_SHADER_SIZE);
   if (!cl)
      return false;
   v3d_cl_vcm_cache_size(&cl, vpm_cfg_v->vcm_cache_size[0], vpm_cfg_v->vcm_cache_size[1]);
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
   KHRN_FMEM_T       *fmem,
   uint32_t          *fshader_ptr,
   uint32_t          *funif_ptr,
   uint32_t          *vdata_ptr,
   bool              does_z_writes,
   v3d_threading_t   threading)
{
   // Find what sized allocations we need
   V3D_NV_SHADER_RECORD_ALLOC_SIZES_T  sizes;
   v3d_get_nv_shader_record_alloc_sizes(&sizes);

   // Allocate the defaults fmem
   uint32_t *defaults_ptr = khrn_fmem_data(fmem, sizes.defaults_size, sizes.defaults_align);
   if (!defaults_ptr)
      return 0;

   // Allocate the shader record fmem
   uint32_t *shader_record_packed_ptr = khrn_fmem_data(fmem, sizes.packed_shader_rec_size,
                                                       sizes.packed_shader_rec_align);
   if (!shader_record_packed_ptr)
      return 0;

   v3d_addr_t shader_record_packed_addr = khrn_fmem_hw_address(fmem, shader_record_packed_ptr);

   // Call the common creation function
   v3d_create_nv_shader_record(
      shader_record_packed_ptr,
      shader_record_packed_addr,
      defaults_ptr,
      khrn_fmem_hw_address(fmem, defaults_ptr),
      khrn_fmem_hw_address(fmem, fshader_ptr),
      khrn_fmem_hw_address(fmem, funif_ptr),
      khrn_fmem_hw_address(fmem, vdata_ptr),
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
static uint32_t *draw_rect_vertex_data(KHRN_FMEM_T *fmem, int x, int xmax, int y, int ymax, uint32_t z)
{
   uint32_t *addr = khrn_fmem_data(fmem, 4 * 3 * 4, V3D_ATTR_ALIGN);
   if (addr == NULL)
      return NULL;

   nv_vertex(addr + 0, x, y, z);
   nv_vertex(addr + 3, xmax, y, z);
   nv_vertex(addr + 6, xmax, ymax, z);
   nv_vertex(addr + 9, x, ymax, z);

   return addr;
}

static bool glxx_draw_rect_write_state_changes(
   GLXX_SERVER_STATE_T *state,
   GLXX_HW_RENDER_STATE_T *rs,
   const GLXX_CLEAR_T *clear,
   int x, int y, int xmax, int ymax)
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
   v3d_cl_clip(&instr, x, y, xmax - x, ymax - y);
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

   v3d_cl_viewport_offset(&instr, 0, 0);

   //TODO: other miscellaneous pieces of state

   khrn_fmem_end_cle(&rs->fmem, instr);

   return true;
}

static uint32_t *copy_shader_to_fmem(KHRN_FMEM_T* fmem, uint32_t shaderSize, uint32_t *shaderCode)
{
   uint32_t *fshader_ptr = khrn_fmem_data(fmem, shaderSize, V3D_QPU_INSTR_ALIGN);
   if (fshader_ptr)
      memcpy(fshader_ptr, shaderCode, shaderSize);
   return fshader_ptr;
}

/* Pack the colours into a shader uniform block, skipping uniform 1, which is used for configuration. */
static void pack_colors_in_uniforms(const uint32_t color_value[4],
   uint32_t *uniform, unsigned uniform_count, v3d_rt_type_t rt_type)
{
   switch (uniform_count)
   {
   case 3:
      {
         uint32_t color_f16[4];
         if (rt_type == V3D_RT_TYPE_8)
         {
            for (unsigned i = 0; i < 4; i++)
            {
               float f = gfx_float_from_bits(color_value[i]);
               uint32_t u = gfx_float_to_unorm8(f);
               color_f16[i] = gfx_unorm_to_float16(u, 8);
            }
         }
         else
         {
            for (unsigned i = 0; i < 4; i++)
               color_f16[i] = gfx_floatbits_to_float16(color_value[i]);
         }
         uniform[2] = gfx_pack_1616(color_f16[2], color_f16[3]);
         uniform[0] = gfx_pack_1616(color_f16[0], color_f16[1]);
      }
      break;
   case 5:
      uniform[4] = color_value[3];
      uniform[3] = color_value[2];
      uniform[2] = color_value[1];
      uniform[0] = color_value[0];
      break;
   default:
      unreachable();
   }
}

/* TODO: MRT and other fanciness? */
static uint32_t clear_shader_2[] =
{
   0xbb800000, 0x3c403186, // nop            ; ldunif
#if V3D_VER_HAS_LATE_UNIF
   0xb682d000, 0x3c203188, // mov tlbu, r5   ; thrsw
   0xbb800000, 0x3c403186, // nop            ; ldunif
   0xb682d000, 0x3c003187, // mov tlb, r5
#else
   0xb682d000, 0x3c003188, // mov tlbu, r5
   0xbb800000, 0x3c403186, // nop            ; ldunif
   0xb682d000, 0x3c203187, // mov tlb, r5    ; thrsw
   0xbb800000, 0x3c003186, // nop
   0xbb800000, 0x3c003186, // nop
#endif
};

static uint32_t clear_shader_4[] =
{
   0xbb800000, 0x3c403186, // nop; ldunif
   0xb682d000, 0x3c003188, // mov tlbu, r5
   0xbb800000, 0x3c403186, // nop; ldunif
#if V3D_VER_HAS_LATE_UNIF
   0xb682d000, 0x3c603187, // mov tlb, r5; ldunif ; thrsw
   0xb682d000, 0x3c403187, // mov tlb, r5; ldunif
   0xb682d000, 0x3c003187, // mov tlb, r5
#else
   0xb682d000, 0x3c403187, // mov tlb, r5; ldunif
   0xb682d000, 0x3c403187, // mov tlb, r5; ldunif
   0xb682d000, 0x3c203187, // mov tlb, r5; thrsw
   0xbb800000, 0x3c003186, // nop
   0xbb800000, 0x3c003186, // nop
#endif
};

static uint32_t clear_shader_no_color[] =
{
   0xbb800000, 0x3c003186, // nop
   0x00000000, 0x03003206, // mov tlbu, 0
   0xb7800000, 0x3c203187, // xor tlb, r0, r0 ; thrsw
   0x00000000, 0x030031c6, // mov tlb, 0
   0x00000000, 0x030031c6, // mov tlb, 0
};

/* Draw a rectangle quickly using a non-vertex shader. Currently only used as
 * the slow path of a clear operation,
 */
/* TODO Should really share more code with the non-draw_rect case */
bool glxx_draw_rect(
   GLXX_SERVER_STATE_T     *state,
   GLXX_HW_RENDER_STATE_T  *rs,
   const GLXX_CLEAR_T      *clear,
   int x, int y,
   int xmax, int ymax)
{
   assert(clear->color_buffer_mask || clear->depth || clear->stencil);

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

   uint32_t *shaderCode;
   uint32_t shaderSize;
   uint32_t uniformCount;
   uint8_t cfg;
   unsigned cfg_unif_pos;
   if (clear->color_buffer_mask)
   {
      int tlb_write_type = get_tlb_write_type_from_rt_type(hw_fb->color_internal_type[rt]);

      /* Allocate memory and copy the shader to the fmem */
      /* TODO: select shader based on MRT etc. */
      /* If we write f16 then we have 2 writes , f32 and i32 have 4 */
      if (tlb_write_type == GLXX_FB_F16)
      {
         shaderCode = clear_shader_2;
         shaderSize = sizeof(clear_shader_2);   /* NB: sizeof(array) */
         uniformCount = 3;
      }
      else
      {
         shaderCode = clear_shader_4;
         shaderSize = sizeof(clear_shader_4);
         uniformCount = 5;
      }
      // Set the configuration byte
      cfg = tlb_write_type << 6 |  (7 - rt) << 3 | 7;
      cfg_unif_pos = 1;
   }
   else
   {
      shaderCode = clear_shader_no_color;
      shaderSize = sizeof(clear_shader_no_color);
      uniformCount = 1;

      /* Configure tlb for f32 color writes, but disable the write masks later */
      /* GFXH-1212 means we must write 4 values to prevent a lockup */
      cfg = 0x3F;
      cfg_unif_pos = 0;
   }

   uint32_t *funif_ptr = khrn_fmem_data(&rs->fmem, uniformCount * 4, V3D_SHADREC_ALIGN);
   if (funif_ptr == NULL)
      return false;

   if (clear->color_buffer_mask)
      /* fill in uniforms, skipping uniform 1 which is used as a configuration byte */
      pack_colors_in_uniforms(clear->color_value, funif_ptr, uniformCount,
            hw_fb->color_internal_type[rt]);

   /* set up the config uniform; unused config entries must be all 1s */
   funif_ptr[cfg_unif_pos] = 0xffffff00 | cfg;

   /* Use x, y, z to create vertex data to be used in the clear */
   uint32_t *vdata_ptr = draw_rect_vertex_data(&rs->fmem, x, xmax, y, ymax, gfx_float_to_bits(clear->depth_value));
   if (vdata_ptr == NULL)
      return false;

   /* Install the shader and shader record. */
   uint32_t *fshader_ptr = copy_shader_to_fmem(&rs->fmem, shaderSize, shaderCode);
   if (fshader_ptr == NULL)
      return false;
   v3d_addr_t nv_shadrec_hw = create_nv_shader_record(&rs->fmem, fshader_ptr, funif_ptr, vdata_ptr,
      /*does_z_writes=*/false, V3D_THREADING_T1);
   if (!nv_shadrec_hw)
      return false;

   if (!glxx_draw_rect_write_state_changes(state, rs, clear, x, y, xmax, ymax))
      return false;

#if V3D_HAS_NEW_TF
   glxx_prim_drawn_by_us_record(rs, 2);
   if (!glxx_tf_record_disable(rs))
      return false;
#endif

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, V3D_CL_GL_SHADER_SIZE + V3D_CL_VERTEX_ARRAY_PRIMS_SIZE);
   if (!instr) return false;

   v3d_cl_nv_shader(&instr, 2, nv_shadrec_hw);
   v3d_cl_vertex_array_prims(&instr, V3D_PRIM_MODE_TRI_FAN, 4, 0);

   khrn_fmem_end_cle_exact(&rs->fmem, instr);

#ifdef SIMPENROSE_WRITE_LOG
   // Increment batch count
   log_batchcount++;
#endif

   return true;
}

#if !V3D_VER_AT_LEAST(3,3,0,0)

static uint32_t setup_render_dummy_point_no_shader_no_query_addr_size(void)
{
   return V3D_CL_VIEWPORT_OFFSET_SIZE
      + V3D_CL_CLIP_SIZE
      + V3D_CL_POINT_SIZE_SIZE
      + V3D_CL_PRIM_LIST_FORMAT_SIZE
      + V3D_CL_SAMPLE_COVERAGE_SIZE
      + V3D_CL_CLIPZ_SIZE
      + V3D_CL_CFG_BITS_SIZE
      + V3D_CL_COLOR_WMASKS_SIZE;
}

static uint32_t clear_shader_no_color_point[] =
{
   0xbb800000, 0x3d003186, // nop; ldvary
   0xbb800000, 0x3d003186, // nop; ldvary
   0xb683f000, 0x3dc03188, // mov tlbu, 0     ; ldvary
   0xb7800000, 0x3c203187, // xor tlb, r0, r0 ; thrsw
   0x00000000, 0x030031c6, // mov tlb, 0
   0x00000000, 0x030031c6, // mov tlb, 0
};

/* The center of the dummy point will be at (0,0).
 * The pixels (if there are any) will pass the Z tests but nothing will be
 * written to the tilebuffer. */

static bool setup_render_dummy_point_no_shader_no_query_addr(uint8_t** instr_ptr, v3d_addr_t* shader_addr, KHRN_FMEM_T* fmem, float size)
{
   /* set up the config uniform; unused config entries must be all 1s */
   /* Configure tlb for f32 color writes, but disable the write masks later */
   /* GFXH-1212 means we must write 4 values to prevent a lockup */
   uint32_t* funif_ptr = khrn_fmem_data(fmem, 4, V3D_SHADREC_ALIGN);
   if (funif_ptr == NULL)
      return false;
   funif_ptr[0] = 0xffffff00 | 0x3F;

   /* create single point vertex */
   uint32_t* vdata_ptr = khrn_fmem_data(fmem, sizeof(uint32_t) * 3, V3D_ATTR_ALIGN);
   if (!vdata_ptr)
      return false;
   nv_vertex(vdata_ptr, 0, 0, 0);

   /* Install the shader and shader record. */
   uint32_t* fshader_ptr = copy_shader_to_fmem(fmem, sizeof(clear_shader_no_color_point), clear_shader_no_color_point);
   if (!fshader_ptr)
      return false;
   *shader_addr = create_nv_shader_record(fmem, fshader_ptr, funif_ptr, vdata_ptr, false, V3D_THREADING_T1);
   if (!*shader_addr)
      return false;

   // Items written to instr begin here.
   uint8_t* instr = *instr_ptr;

   // Emit scissor/clipper/viewport instructions
   v3d_cl_viewport_offset(&instr, 0, 0);
   v3d_cl_clip(&instr, 0, 0, 0x1000, 0x1000);

   v3d_cl_point_size(&instr, size);
   v3d_cl_prim_list_format(&instr, 1, false, false);

   // Emit enough configuration to ensure we pass the Z tests.
   v3d_cl_sample_coverage(&instr, 1.0f);
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

static uint32_t setup_render_dummy_point_size()
{
   return setup_render_dummy_point_no_shader_no_query_addr_size()
      + V3D_CL_OCCLUSION_QUERY_COUNTER_ENABLE_SIZE
      + V3D_CL_GL_SHADER_SIZE;
}

static bool setup_render_dummy_point(uint8_t** instr_ptr, KHRN_FMEM_T* fmem, float size)
{
   // Items written to instr begin here.
   uint8_t* instr = *instr_ptr;

   v3d_addr_t shader_addr;
   if (!setup_render_dummy_point_no_shader_no_query_addr(&instr, &shader_addr, fmem, size))
      return false;

   v3d_cl_occlusion_query_counter_enable(&instr, 0);

   // Emit NV shader instruction.
   v3d_cl_nv_shader(&instr, 2, shader_addr);

   assert((*instr_ptr + setup_render_dummy_point_size()) == instr);
   *instr_ptr = instr;
   return true;
}

static uint32_t render_dummy_point_size()
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

uint32_t glxx_workaround_gfxh_1313_size(void)
{
   return setup_render_dummy_point_size() + render_dummy_point_size();
}

bool glxx_workaround_gfxh_1313(uint8_t** instr_ptr, KHRN_FMEM_T* fmem)
{
   if (!setup_render_dummy_point(instr_ptr, fmem, 0.0f))
      return false;

   render_dummy_point(instr_ptr);
   return true;
}

uint32_t glxx_workaround_gfxh_1320_size(void)
{
   return setup_render_dummy_point_no_shader_no_query_addr_size()
      + (V3D_CL_OCCLUSION_QUERY_COUNTER_ENABLE_SIZE + V3D_CL_GL_SHADER_SIZE + render_dummy_point_size()) * 8;
}

bool glxx_workaround_gfxh_1320(uint8_t** instr_ptr, KHRN_FMEM_T* fmem)
{
   uint8_t* instr = *instr_ptr;

   // Get address of scratch buffer for TLB to spray writes to.
   gmem_handle_t scratch_mem = khrn_get_gfxh_1320_buffer();
   v3d_addr_t scratch_addr = khrn_fmem_lock_and_sync(fmem, scratch_mem, 0, GMEM_SYNC_TLB_OQ_READ | GMEM_SYNC_TLB_OQ_WRITE);
   if (!scratch_addr)
      return false;

   // Setup for rendering a point of size 1 pixel.
   v3d_addr_t shader_addr;
   if (!setup_render_dummy_point_no_shader_no_query_addr(&instr, &shader_addr, fmem, 1.5f))
      return false;

   // Draw a pixel for each way in the occlusion query cache.
   for (unsigned i = 0; i != 8; ++i)
   {
      // GFXH-1330 requires a shader change for every stencil/query change.
      v3d_cl_occlusion_query_counter_enable(&instr, scratch_addr + V3D_OCCLUSION_QUERY_COUNTER_FIRST_CORE_CACHE_LINE_ALIGN * i);
      v3d_cl_nv_shader(&instr, 2, shader_addr);
      render_dummy_point(&instr);
   }

   assert((instr - *instr_ptr) == glxx_workaround_gfxh_1320_size());
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
#if V3D_HAS_PER_RT_BLEND
      if (state->blend.rt_enables)
#else
      if (state->blend.enable)
#endif
      {
         goto disallowed;
      }

      // disable z-prepass if using colour write masks (currently a conservative test)
      // timh-todo: should only care if this changes
      if (state->color_write != gfx_mask(rs->installed_fb.rt_count * 4))
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
