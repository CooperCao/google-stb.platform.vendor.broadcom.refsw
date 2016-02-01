/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
BCM2708 implementation of hardware abstraction layer.
Functions common to OpenGL ES 1.1 and OpenGL ES 2.0
=============================================================================*/

#include "middleware/khronos/glxx/glxx_hw.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_color.h"
#include "interface/khronos/common/khrn_int_math.h"
#include "interface/khronos/common/khrn_options.h"
#include "interface/khronos/glxx/gl_public_api.h"
#include "middleware/khronos/common/khrn_control_list.h"
#include "middleware/khronos/common/khrn_render_state.h"
#include "middleware/khronos/common/khrn_process.h"
#include "middleware/khronos/glxx/glxx_inner.h"
#include "middleware/khronos/glxx/glxx_shader.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_server_texture.h"
#include "middleware/khronos/glxx/glxx_log.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"
#include "middleware/khronos/glxx/glxx_translate.h"
#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/glsl/glsl_tex_params.h"
#include "middleware/khronos/glsl/glsl_backend_uniforms.h"
#include "middleware/khronos/gl11/gl11_shadercache.h"
#include "middleware/khronos/gl11/gl11_shader.h"

#include "middleware/khronos/glxx/glxx_server_internal.h"

#include "helpers/gfx/gfx_bufstate.h"

#include <limits.h>
#include <assert.h>

#include "helpers/v3d/v3d_common.h"
#include "helpers/v3d/v3d_cl.h"

#define VCOS_LOG_CATEGORY (&glxx_log)

#define IU_MAX 64

/*************************************************************
 Static function forwards
 *************************************************************/

static void create_gl_shader_record(
   KHRN_FMEM_T                    *fmem,
   GLXX_SERVER_STATE_T            *state,
   const GLXX_LINK_RESULT_DATA_T  *data,
   const GLXX_ATTRIB_CONFIG_T     *attrib,
   const GLXX_VERTEX_BUFFER_CONFIG_T *vb_config,
   const GLXX_VERTEX_POINTERS_T   *vertex_pointers,
   const GLXX_GENERIC_ATTRIBUTE_T *generic_attrib,
   GLXX_HW_SHADER_RECORD_INFO_T   *info);

static v3d_prim_mode_t convert_primitive_type(GLenum mode);
static v3d_prim_mode_t convert_primitive_type_with_transform_feedback(GLenum mode);
static v3d_index_type_t convert_index_type(GLenum type);

static bool backend_uniform_address(KHRN_FMEM_T *fmem, uint32_t u1,
   const GL20_PROGRAM_T *program,
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

/*************************************************************
 Global Functions
 *************************************************************/
bool glxx_hw_invalidate_frame(GLXX_SERVER_STATE_T *state,
                              GLXX_FRAMEBUFFER_T *fbo,
                              bool rt[GLXX_MAX_RENDER_TARGETS],
                              bool color, bool multisample,
                              bool depth, bool stencil,
                              bool *have_rs)
{
   GLXX_HW_FRAMEBUFFER_T hw_fb;
   GLXX_HW_RENDER_STATE_T *rs = NULL;
   bool ok = false;

   glxx_init_hw_framebuffer(&hw_fb);
   if (!glxx_build_hw_framebuffer(fbo, &hw_fb))
      return false;

   // ok false means we ran out memory; ok true and rs == NULL, it means our rs
   // has already been flushed
   ok = glxx_install_existing_rs(state, &hw_fb, &rs);
   if (!ok || !rs)
      goto end;

   glxx_hw_invalidate_internal(rs, rt, color, multisample, depth, stencil);
   ok = true;
end:
   if (have_rs)
      *have_rs = (rs != NULL);
   glxx_destroy_hw_framebuffer(&hw_fb);
   return ok;
}

bool glxx_invalidate_default_draw_framebuffer(GLXX_SERVER_STATE_T *state,
                                              bool color, bool multisample,
                                              bool depth, bool stencil,
                                              bool *have_rs)
{
   /* default frame buffer has only one color rt */
   bool rt[GLXX_MAX_RENDER_TARGETS];
   rt[0] = true;
   for (unsigned b = 1; b < GLXX_MAX_RENDER_TARGETS; ++b)
      rt[b] = false;

   return glxx_hw_invalidate_frame(state, state->default_framebuffer[GLXX_DRAW_SURFACE],
                                   rt, color, multisample, depth, stencil, have_rs);
}

void glxx_init_hw_framebuffer(GLXX_HW_FRAMEBUFFER_T *hw_fb)
{
   memset(hw_fb, 0, sizeof(GLXX_HW_FRAMEBUFFER_T));
   hw_fb->width   = ~0u;
   hw_fb->height  = ~0u;
   hw_fb->max_bpp = V3D_RT_BPP_32;
}

void glxx_destroy_hw_framebuffer(GLXX_HW_FRAMEBUFFER_T *hw_fb)
{
   unsigned b;
   for (b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
   {
      khrn_image_plane_assign(&hw_fb->color[b], NULL);
      khrn_image_plane_assign(&hw_fb->color_ms[b], NULL);
   }
   khrn_image_plane_assign(&hw_fb->depth, NULL);
   khrn_image_plane_assign(&hw_fb->stencil, NULL);
}

/* Updates max common size and max_bpp in hw_fb based on attached image plane (possibly NULL) */
static void update_hw_fb_with_image(
   GLXX_HW_FRAMEBUFFER_T *hw_fb,
   KHRN_IMAGE_PLANE_T *image_plane,
   bool multisample,
   bool is_color_buffer)
{
   unsigned w;
   unsigned h;
   GFX_LFMT_T image_lfmt;

   // Not all framebuffers have color / multisample / depth / stencil
   if (image_plane->image == NULL)
      return;

   w = khrn_image_get_width(image_plane->image);
   h = khrn_image_get_height(image_plane->image);
   image_lfmt = khrn_image_plane_lfmt(image_plane);

   assert(image_lfmt != GFX_LFMT_NONE);

   if (is_color_buffer)
   {
      v3d_pixel_format_t pixel_format = gfx_lfmt_translate_pixel_format(image_lfmt);
      v3d_rt_bpp_t internal_bpp = v3d_pixel_format_internal_bpp(pixel_format);

      hw_fb->max_bpp = gfx_umax(hw_fb->max_bpp, internal_bpp);
   }

   if (multisample)
   {
      w /= 2;
      h /= 2;
   }

   hw_fb->width = gfx_umin(hw_fb->width, w);
   hw_fb->height = gfx_umin(hw_fb->height, h);

   assert(w <= GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE);
   assert(h <= GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE);
}

bool glxx_build_hw_framebuffer(const GLXX_FRAMEBUFFER_T *fb,
                               GLXX_HW_FRAMEBUFFER_T *hw_fb)
{
   bool     ms;
   unsigned b;
   bool     ok;
   bool depth_ms, stencil_ms;

   assert(glxx_fb_is_complete(fb));

   ms = (glxx_fb_get_ms_mode(fb) != GLXX_NO_MS);

   ok = glxx_attachment_acquire_image(&fb->attachment[GLXX_DEPTH_ATT],
            GLXX_PREFER_DOWNSAMPLED, &hw_fb->depth.image, &depth_ms);
   if (!ok)
      goto error_out;

   ok = glxx_attachment_acquire_image(&fb->attachment[GLXX_STENCIL_ATT],
            GLXX_PREFER_DOWNSAMPLED, &hw_fb->stencil.image, &stencil_ms);
   if (!ok)
      goto error_out;


   /* Stencil is always in the last plane */
   if (hw_fb->stencil.image)
      hw_fb->stencil.plane_idx = khrn_image_get_num_planes(hw_fb->stencil.image) - 1;

   assert(!hw_fb->depth.image || ms == depth_ms);
   assert(!hw_fb->stencil.image || ms == stencil_ms);

   // we always configure tilebuffer based on all attached images in fb,
   // regardless of any draw buffers setting.
   for (b = 0; b < GLXX_MAX_RENDER_TARGETS; ++b)
   {
      const GLXX_ATTACHMENT_T *att;

      att = &fb->attachment[GLXX_COLOR0_ATT + b];

      ok = glxx_attachment_acquire_image(att, GLXX_DOWNSAMPLED,
            &hw_fb->color[b].image, NULL);
      if (!ok)
         goto error_out;

      ok = glxx_attachment_acquire_image(att, GLXX_MULTISAMPLED,
            &hw_fb->color_ms[b].image, NULL);
      if (!ok)
         goto error_out;

      if (hw_fb->color[b].image || hw_fb->color_ms[b].image)
         hw_fb->rt_count = b + 1;
   }

   // Figure out framebuffer max_bpp and largest common size
   for (b = 0; b < hw_fb->rt_count; ++b)
   {
      update_hw_fb_with_image(hw_fb, &hw_fb->color[b], false, true);
      update_hw_fb_with_image(hw_fb, &hw_fb->color_ms[b], true, true);
   }
   update_hw_fb_with_image(hw_fb, &hw_fb->depth, depth_ms, false);
   update_hw_fb_with_image(hw_fb, &hw_fb->stencil, stencil_ms, false);

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

   hw_fb->ms = ms;

   return true;

error_out:
   glxx_destroy_hw_framebuffer(hw_fb);
   return false;
}

static bool compare_hw_fb(const GLXX_HW_FRAMEBUFFER_T *a,
                          const GLXX_HW_FRAMEBUFFER_T *b)
{
   unsigned i;

   if ((a->rt_count      != b->rt_count    ) ||
       (a->max_bpp       != b->max_bpp     ) ||
       (a->ms            != b->ms          ) ||
       (a->width         != b->width       ) ||
       (a->height        != b->height      )  )
      return false;

   for (i = 0; i < a->rt_count; ++i)
   {
      if (!khrn_image_plane_equal(&a->color[i], &b->color[i]))
         return false;

      if (!khrn_image_plane_equal(&a->color_ms[i], &b->color_ms[i]))
         return false;
   }

   if (!khrn_image_plane_equal(&a->depth, &b->depth))
      return false;

   if (!khrn_image_plane_equal(&a->stencil, &b->stencil))
      return false;

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

GLXX_HW_RENDER_STATE_T* glxx_install_framebuffer_renderstate(GLXX_SERVER_STATE_T *state)
{
   GLXX_HW_RENDER_STATE_T* rs = NULL;

   GLXX_HW_FRAMEBUFFER_T hw_fb;
   glxx_init_hw_framebuffer(&hw_fb);

   if (glxx_build_hw_framebuffer(state->bound_draw_framebuffer, &hw_fb))
   {
      rs = glxx_install_rs(state, &hw_fb, false);
   }

   glxx_destroy_hw_framebuffer(&hw_fb);
   return rs;
}

static GLXX_HW_RENDER_STATE_T *create_and_start_render_state(GLXX_HW_FRAMEBUFFER_T *fb)
{
   GLXX_HW_RENDER_STATE_T *rs;

   rs = khrn_render_state_get_glxx(khrn_render_state_new(KHRN_RENDER_STATE_TYPE_GLXX));

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
      return  state->current_render_state;

   return glxx_find_existing_rs_for_fb(fb);
}

static bool install_rs_as_current(GLXX_SERVER_STATE_T* state,
      GLXX_HW_RENDER_STATE_T *rs)
{
   if (  !khrn_fmem_record_fence_to_signal(&rs->fmem, state->fences.fence)
      || !khrn_fmem_record_fence_to_depend_on(&rs->fmem, state->fences.fence_to_depend_on) )
      return false;

   // if render-state is new, or last driven by different server-state
   if (rs->current_server_state != state)
   {
      // only allow one server-state to point to this render-state
      if (rs->current_server_state != NULL)
      {
         rs->current_server_state->current_render_state = NULL;
      }
      rs->current_server_state = state;

      // set all dirty flags for the server-state so they are flushed through
      glxx_server_invalidate_for_render_state(state, rs);
   }

   // A render state can only dither if everything was dithered
   rs->dither = rs->dither && state->caps.dither;

   // cache the current render-state to speedup the lookup next time
   state->current_render_state = rs;
   return true;
}

bool glxx_install_existing_rs(GLXX_SERVER_STATE_T *state,
   GLXX_HW_FRAMEBUFFER_T *fb, GLXX_HW_RENDER_STATE_T **rs)
{
   bool ok = true;

   /* we might have users on fence_to_depend_on, flush them now */
   khrn_fence_flush(state->fences.fence_to_depend_on);

   *rs = get_existing_rs(state, fb);
   if (*rs == NULL)
      goto end;

   if (khrn_fmem_check_flush(&(*rs)->fmem))
      goto end;

   if (!install_rs_as_current(state, *rs))
   {
      glxx_hw_discard_frame(*rs);
      *rs = NULL;
      ok = false;
      goto end;
   }

end:
   return ok;
}

GLXX_HW_RENDER_STATE_T* glxx_install_rs(GLXX_SERVER_STATE_T *state,
   GLXX_HW_FRAMEBUFFER_T *fb, bool do_not_flush_existing)
{
   /* we might have users on fence_to_depend_on, flush them now */
   khrn_fence_flush(state->fences.fence_to_depend_on);

   // handle scenarios where an existing render state needs to be flushed
   GLXX_HW_RENDER_STATE_T *rs = get_existing_rs(state, fb);
   if (rs != NULL && !do_not_flush_existing)
   {
      // If we're drawing to an existing render-state that has TLB blits then it needs to be flushed
      // in case this draw call is using the TLB store target as a texture.
      if (rs->num_blits != 0)
      {
         glxx_hw_render_state_flush(rs);
         rs = NULL;
      }
      // check to see if we need to flush due to high client fmem use
      // if this returns true we flushed the given render-state
      else if (khrn_fmem_check_flush(&rs->fmem))
      {
         rs = NULL;
      }
   }

   // create a new render state if required
   if (rs == NULL)
   {
      rs = create_and_start_render_state(fb);
      if (rs == NULL)
         return NULL;
   }

   if (!install_rs_as_current(state, rs))
   {
      glxx_hw_discard_frame(rs);
      return NULL;
   }
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

static int get_tlb_write_type_from_lfmt(GFX_LFMT_T lfmt)
{
   v3d_rt_type_t rt = v3d_pixel_format_internal_type(gfx_lfmt_translate_pixel_format(lfmt));
   return get_tlb_write_type_from_rt_type(rt);
}

void glxx_get_attribs_live(const GLXX_SERVER_STATE_T *state,
                           uint32_t *attribs_live)
{
   if (IS_GL_11(state)) {
      *attribs_live = gl11_get_live_attr_set(&state->gl11.shaderkey);
   } else {
      IR_PROGRAM_T   *ir      = state->program->linked_glsl_program->ir;
      *attribs_live = ir->live_attr_set;
   }
}

bool glxx_compute_texture_uniforms(GLXX_SERVER_STATE_T *state, KHRN_FMEM_T *fmem)
{
   glxx_texture_iterator_t iterate = glxx_server_texture_iterator(state);
   struct glxx_texture_info info;

   for (unsigned i = 0; i < GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS; i++)
      state->shaderkey_common.gadgettype[i] = 0;

   for (unsigned i = 0; iterate(&info, state, &i); )
   {
      unsigned index = info.index;
      GLXX_TEXTURE_UNIF_T *texture_unif = &state->texture_unif[index];

      enum glxx_tex_completeness complete = glxx_texture_key_and_uniforms(
            info.texture, info.sampler,
            info.used_in_binning, info.is_32bit, fmem, texture_unif,
            &state->shaderkey_common.gadgettype[index],
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

      gmem_handle_t dummy_texture_handle = get_dummy_texture();
      v3d_addr_t addr_base = khrn_fmem_lock_and_sync(fmem,
            dummy_texture_handle,
            info.used_in_binning ?
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

#if V3D_HAS_NEW_TMU_CFG
      V3D_TMU_TEX_STATE_T tex_state;
      memset(&tex_state, 0, sizeof(tex_state));
      tex_state.l0_addr = addr_base;
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
      texture_unif->hw_param0 = khrn_fmem_add_tmu_tex_state(fmem, hw_tex_state, /*extended=*/false);
      if (!texture_unif->hw_param0)
         return false;

      V3D_TMU_SAMPLER_T sampler;
      memset(&sampler, 0, sizeof(sampler));
      sampler.filter = V3D_TMU_FILTER_MIN_LIN_MAG_LIN;
      uint8_t hw_sampler[V3D_TMU_SAMPLER_PACKED_SIZE];
      v3d_pack_tmu_sampler(hw_sampler, &sampler);
      texture_unif->hw_param1 = khrn_fmem_add_tmu_sampler(fmem, hw_sampler, /*extended=*/false);
      if (!texture_unif->hw_param1)
         return false;

      state->shaderkey_common.gadgettype[index] = GLSL_GADGETTYPE_AUTO;
#else
      V3D_TMU_PARAM0_CFG1_T tmu_param0_cfg1 = { 0, };
      texture_unif->hw_param0 = v3d_pack_tmu_param0_cfg1(&tmu_param0_cfg1);

      // setup indirect record
      V3D_TMU_INDIRECT_T tmu_indirect = { 0, };
      tmu_indirect.arr_str = 4*4*4;// For 3D textures this is used to init the slice_pitch
      tmu_indirect.base = addr_base;
      tmu_indirect.width = width;
      tmu_indirect.height = height;
      tmu_indirect.depth = depth;
      tmu_indirect.ttype = type;
      tmu_indirect.u.not_child_image.output_type = khrn_get_hw_misccfg()->ovrtmuout ?
         (info.is_32bit ? V3D_TMU_OUTPUT_TYPE_32 : V3D_TMU_OUTPUT_TYPE_16) :
         V3D_TMU_OUTPUT_TYPE_AUTO;
      tmu_indirect.swizzles[0] = V3D_TMU_SWIZZLE_R;
      tmu_indirect.swizzles[1] = V3D_TMU_SWIZZLE_G;
      tmu_indirect.swizzles[2] = V3D_TMU_SWIZZLE_B;
      tmu_indirect.swizzles[3] = V3D_TMU_SWIZZLE_A;
      tmu_indirect.etcflip = true;

      uint32_t hw_indirect[V3D_TMU_INDIRECT_PACKED_SIZE / 4];
      v3d_pack_tmu_indirect_not_child_image(hw_indirect, &tmu_indirect);
      texture_unif->hw_param1 = khrn_fmem_add_tmu_indirect(fmem, hw_indirect);
      if (!texture_unif->hw_param1)
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

      state->shaderkey_common.gadgettype[index] =
         (khrn_get_hw_misccfg()->ovrtmuout || !info.is_32bit) ?
         GLSL_GADGETTYPE_AUTO : GLSL_GADGETTYPE_SWAP1632;
#endif
   }

   return true;
}

static void compute_backend_shader_key(GLXX_SERVER_STATE_T *state,
                                       const GLXX_HW_RENDER_STATE_T *rs,
                                       GLXX_PRIMITIVE_T draw_mode)
{
   const GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;
   uint32_t backend_mask = fb->ms ? ~0u : ~(uint32_t)GLXX_SAMPLE_OPS_M;
   uint32_t backend = state->statebits.backend & backend_mask;

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

   if (rs->z_prepass_allowed)            backend |= GLXX_Z_ONLY_WRITE;

   if (!state->depth_mask) backend |= GLXX_SKIP_DISCARD;
   if (state->sample_mask.enable && fb->ms) backend |= GLXX_SAMPLE_MASK;

   for (int i = 0; i < GLXX_MAX_RENDER_TARGETS; i++)
   {
      int fb_gadget;
      uint32_t shift;
      if (glxx_fb_is_valid_draw_buf(state->bound_draw_framebuffer, GLXX_COLOR0_ATT + i)) {
         const KHRN_IMAGE_PLANE_T *col = fb->color_ms[i].image ? &fb->color_ms[i] : &fb->color[i];
         fb_gadget =
            get_tlb_write_type_from_lfmt(khrn_image_plane_lfmt(col));
      } else
         fb_gadget = GLXX_FB_NOT_PRESENT;

      assert((fb_gadget & (~GLXX_FB_GADGET_M)) == 0);

      shift = GLXX_FB_GADGET_S + 2*i;
      backend &= ~(GLXX_FB_GADGET_M << shift);
      backend |= (fb_gadget << shift);
   }

   state->shaderkey_common.backend = backend;
}

bool glxx_calculate_and_hide(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs,
      GLXX_PRIMITIVE_T draw_mode)
{
   /* These are order dependent because they read/write the shaderkey */
   compute_backend_shader_key(state, rs, draw_mode);

   return glxx_compute_texture_uniforms(state, &rs->fmem);
}

/* disable all the color write masks */
static void disable_color_write_masks(V3D_CL_COLOR_WMASKS_T masks)
{
   for (unsigned i = 0; i < V3D_CL_COLOR_WMASKS_NUM; i++)
   {
      masks[i].disable_r = masks[i].disable_g =
        masks[i].disable_b = masks[i].disable_a = true;
   }
}

static void set_color_write_masks(const GLXX_FRAMEBUFFER_T *fb,
      const glxx_color_write_t *color_write,
      V3D_CL_COLOR_WMASKS_T masks)
{
   /* start by disabling all of them */
   disable_color_write_masks(masks);

   glxx_att_index_t att_index;
   unsigned i = 0;
   while (glxx_fb_iterate_valid_draw_bufs(fb, &i, &att_index))
   {
      unsigned b = att_index - GLXX_COLOR0_ATT;

      masks[b].disable_r = !color_write->r;
      masks[b].disable_g = !color_write->g;
      masks[b].disable_b = !color_write->b;
      masks[b].disable_a = !color_write->a;

      /* If the buffer doesn't have an alpha channel, we need the alpha in
       * the TLB to be 1 to get correct blending. The TLB will set alpha to
       * 1 when it loads an alpha-less buffer, but we need to explicitly
       * mask alpha writes after that to prevent it changing */
      const GLXX_ATTACHMENT_T *att;
      att = glxx_fb_get_attachment_by_index(fb, att_index);
      GFX_LFMT_T api_fmt =  glxx_attachment_get_api_fmt (att);
      if (api_fmt != GFX_LFMT_NONE && !gfx_lfmt_has_alpha(api_fmt))
         masks[b].disable_a = true;
   }

}

/* insert clip instruction. nothing_to_draw is set to true if the region is empty and transform feedback
 * isn't in use.
   returns false if failed a memory alloc
*/
static bool do_changed_cfg(
   GLXX_HW_RENDER_STATE_T *rs,
   GLXX_SERVER_STATE_T *state,
   bool * nothing_to_draw)
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
                     + GLXX_CL_STATE_SIZE[GLXX_CL_STATE_BLEND_MODE]
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
      int x = MAX(0, vpx);
      int y = MAX(0, vpy);
      int xmax = MIN(hw_fb->width, vpx + vpw);
      int ymax = MIN(hw_fb->height, vpy + vph);

      if (state->caps.scissor_test)
      {
         x = MAX(x, scx);
         y = MAX(y, scy);
         xmax = MIN(xmax, scx + scw);
         ymax = MIN(ymax, scy + sch);
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
         gfx_fmin(state->viewport.vp_near, state->viewport.vp_far),
         gfx_fmax(state->viewport.vp_near, state->viewport.vp_far));

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

   if (khrn_render_state_set_contains(state->dirty.blend_mode | state->dirty.blend_color, rs))
   {
      if (IS_GL_11(state))
      {
         assert(state->blend.src_alpha == state->blend.src_rgb);
         assert(state->blend.dst_alpha == state->blend.dst_rgb);
         assert(state->blend.color_eqn == V3D_BLEND_EQN_ADD);
         assert(state->blend.alpha_eqn == V3D_BLEND_EQN_ADD);
      }

      if (khrn_render_state_set_contains(state->dirty.blend_mode, rs))
      {
         khrn_render_state_set_remove(&state->dirty.blend_mode, rs);

         glxx_cl_record_begin(cl_record, GLXX_CL_STATE_BLEND_MODE, instr);
         v3d_cl_blend_cfg(
              &instr,
            state->blend.alpha_eqn,
            state->blend.src_alpha,
            state->blend.dst_alpha,
            state->blend.color_eqn,
            state->blend.src_rgb,
            state->blend.dst_rgb,
            V3D_BLEND_VG_MODE_NORMAL
            );
         glxx_cl_record_end(cl_record, GLXX_CL_STATE_BLEND_MODE, instr);
      }

      if (  khrn_render_state_set_contains(state->dirty.blend_color, rs)
         && (  v3d_blend_needs_cc(state->blend.src_rgb)
            || v3d_blend_needs_cc(state->blend.dst_rgb)
            || v3d_blend_needs_cc(state->blend.src_alpha)
            || v3d_blend_needs_cc(state->blend.dst_alpha)
            )
         )
      {
         khrn_render_state_set_remove(&state->dirty.blend_color, rs);

         glxx_cl_record_begin(cl_record, GLXX_CL_STATE_BLEND_COLOR, instr);
         v3d_cl_blend_ccolor(
            &instr,
            state->blend_color[0],
            state->blend_color[1],
            state->blend_color[2],
            state->blend_color[3]
            );
         glxx_cl_record_end(cl_record, GLXX_CL_STATE_BLEND_COLOR, instr);
      }
   }

   if (khrn_render_state_set_contains(state->dirty.color_write, rs))
   {
      khrn_render_state_set_remove(&state->dirty.color_write, rs);

      V3D_CL_COLOR_WMASKS_T masks;
      GLXX_FRAMEBUFFER_T *fb = state->bound_draw_framebuffer;

      set_color_write_masks(fb, &state->color_write, masks);

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_COLOR_WRITE, instr);
      v3d_cl_color_wmasks_indirect(&instr, (const V3D_CL_COLOR_WMASKS_T *)&masks);
      glxx_cl_record_end(cl_record, GLXX_CL_STATE_COLOR_WRITE, instr);
   }

   if (stencil_test && khrn_render_state_set_contains(state->dirty.stencil, rs))
   {
      khrn_render_state_set_remove(&state->dirty.stencil, rs);

      int max = ((1 << glxx_get_stencil_size(state)) - 1);
      int fref = gfx_sclamp(state->stencil_func.front.ref, 0, max);
      int bref = gfx_sclamp(state->stencil_func.back.ref, 0, max);

      glxx_cl_record_begin(cl_record, GLXX_CL_STATE_STENCIL, instr);

      uint32_t stencil_cfg = fref;
      stencil_cfg |= (state->stencil_func.front.mask & 0xff) << 8;
      stencil_cfg |= translate_stencil_func(state->stencil_func.front.func) << 16;
      stencil_cfg |= translate_stencil_op(state->stencil_op.front.fail) << 19;
      stencil_cfg |= translate_stencil_op(state->stencil_op.front.zfail) << 22;
      stencil_cfg |= translate_stencil_op(state->stencil_op.front.zpass) << 25;
      stencil_cfg |= 1 << 28;

      add_byte(&instr, V3D_CL_STENCIL_CFG);
      add_word(&instr, stencil_cfg);
      add_byte(&instr, state->stencil_mask.front & 0xff);

      stencil_cfg = bref;
      stencil_cfg |= (state->stencil_func.back.mask & 0xff) << 8;
      stencil_cfg |= translate_stencil_func(state->stencil_func.back.func) << 16;
      stencil_cfg |= translate_stencil_op(state->stencil_op.back.fail) << 19;
      stencil_cfg |= translate_stencil_op(state->stencil_op.back.zfail) << 22;
      stencil_cfg |= translate_stencil_op(state->stencil_op.back.zpass) << 25;
      stencil_cfg |= 1 << 29;

      add_byte(&instr, V3D_CL_STENCIL_CFG);
      add_word(&instr, stencil_cfg);
      add_byte(&instr, state->stencil_mask.back & 0xff);

      glxx_cl_record_end(cl_record, GLXX_CL_STATE_STENCIL, instr);
   }

   // timh-todo: can we delete this code now?
#if 0
   if (state->changed_stencil.front || state->changed_stencil.back) // TODO Could check if both changed and if only one of them changed
   {
      V3D_CL_STENCIL_CFG_T f_cfg;
      V3D_CL_STENCIL_CFG_T b_cfg;
      f_cfg.reference      = state->stencil_func.front.ref;
      f_cfg.mask           = state->stencil_func.front.mask & 0xff;
      f_cfg.func           = translate_stencil_func(state->stencil_func.front.func);
      f_cfg.fail_op        = translate_stencil_op(state->stencil_op.front.fail);
      f_cfg.depth_fail_op  = translate_stencil_op(state->stencil_op.front.zfail);
      f_cfg.pass_op        = translate_stencil_op(state->stencil_op.front.zpass);
      f_cfg.front_cfg      = true;
      f_cfg.write_mask     = state->stencil_mask.front & 0xff;

      if (!IS_GL_11(state))
      {
         b_cfg.reference      = state->stencil_func.back.ref;
         b_cfg.mask           = state->stencil_func.back.mask & 0xff;
         b_cfg.func           = translate_stencil_func(state->stencil_func.back.func);
         b_cfg.fail_op        = translate_stencil_op(state->stencil_op.back.fail);
         b_cfg.depth_fail_op  = translate_stencil_op(state->stencil_op.back.zfail);
         b_cfg.pass_op        = translate_stencil_op(state->stencil_op.back.zpass);
         b_cfg.write_mask     = state->stencil_mask.back & 0xff;
         f_cfg.back_cfg =
            (b_cfg.reference     == f_cfg.reference    ) &&
            (b_cfg.mask          == f_cfg.mask         ) &&
            (b_cfg.func          == f_cfg.func         ) &&
            (b_cfg.fail_op       == f_cfg.fail_op      ) &&
            (b_cfg.depth_fail_op == f_cfg.depth_fail_op) &&
            (b_cfg.pass_op       == f_cfg.pass_op      ) &&
            (b_cfg.write_mask    == f_cfg.write_mask   );
      }
      else
      {
         f_cfg.back_cfg = true;
      }
      instr = khrn_fmem_cle(fmem, V3D_CL_STENCIL_CFG_SIZE * (f_cfg.back_cfg ? 1 : 2));
      v3d_cl_stencil_cfg_indirect(&instr, &f_cfg);
      if (!f_cfg.back_cfg)
      {
         b_cfg.front_cfg   = false;
         b_cfg.back_cfg    = true;
         v3d_cl_stencil_cfg_indirect(&instr, &b_cfg);
      }
      state->changed_stencil.front = false;
      state->changed_stencil.back = false;
   }
#endif

   /* TODO It might be a worthwhile optimisation to put this in an if (state->changed.ez) */
   if (gfx_ez_update_cfg(&rs->ez,
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

      cfg_bits.blend = !!state->blend.enable;
      /* Switch blending off if logic_op is enabled */
      cfg_bits.blend = cfg_bits.blend && !(state->gl11.statebits.f_enable & GL11_LOGIC_M);
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
#if GL_BRCM_provoking_vertex
      cfg_bits.d3d_prov_vtx      = state->provoking_vtx == GL_FIRST_VERTEX_CONVENTION_BRCM;
#else
      cfg_bits.d3d_prov_vtx      = false;
#endif

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


static bool record_shader_record(KHRN_FMEM_T *fmem,
                                 const GLXX_HW_SHADER_RECORD_INFO_T *record_info)
{
   uint8_t *instr = khrn_fmem_cle(fmem, V3D_CL_GL_SHADER_SIZE);
   if (!instr)
      return false;

   assert(record_info->shader_record);
   // See create_gl_shader_record() and GFXH-930 for min 1 attributes
   assert(record_info->num_attributes_hw > 0);

   v3d_cl_gl_shader(&instr, record_info->num_attributes_hw, record_info->shader_record);

   return true;
}


///////////

static bool glxx_hw_draw_arrays_record(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs,
      const GLXX_DRAW_T *draw,
      const GLXX_LINK_RESULT_DATA_T *link_data)
{
   uint8_t *instr;
   int size = (draw->instance_count > 1) ?
      V3D_CL_VERTEX_ARRAY_INSTANCED_PRIMS_SIZE :
      V3D_CL_VERTEX_ARRAY_PRIMS_SIZE;
   v3d_prim_mode_t mode = state->transform_feedback.in_use
      ? convert_primitive_type_with_transform_feedback(draw->mode)
      : convert_primitive_type(draw->mode);

   assert(draw->is_draw_arrays);
   assert(draw->instance_count >= 1);

   bool use_tf = false;

   if (state->transform_feedback.in_use)
   {
      GLXX_PROGRAM_TRANSFORM_FEEDBACK_T *ptf = &state->program->transform_feedback;

      if (ptf->post_link.varying_count > 0)
      {
         use_tf = true;

         uint32_t spec_count = ptf->post_link.spec_count;
         uint32_t stream_count = ptf->post_link.addr_count;

         // Enable tf streams
         size += V3D_CL_TRANSFORM_FEEDBACK_ENABLE_SIZE;
         size += V3D_TF_SPEC_PACKED_SIZE * spec_count;
         size += sizeof(v3d_addr_t) * stream_count;

         // Disable streams
         size += V3D_CL_TRANSFORM_FEEDBACK_ENABLE_SIZE;
         size += 2; // Empty spec
      }
   }

   if (draw->baseinstance)
      size += V3D_CL_BASE_VERTEX_BASE_INSTANCE_SIZE;

   instr = khrn_fmem_cle(&rs->fmem, size);
   if (!instr)
      return false;

   if (use_tf)
   {
      bool point_size_used = (link_data->flags & 1) == 1;
      glxx_tf_emit_spec(state, rs, &instr, point_size_used);
   }

   if (draw->baseinstance)
      v3d_cl_base_vertex_base_instance(&instr, 0, draw->baseinstance);

   if (draw->instance_count > 1)
      v3d_cl_vertex_array_instanced_prims(&instr, mode, draw->count,
            draw->instance_count, draw->first);
   else
      v3d_cl_vertex_array_prims(&instr, mode, draw->count, draw->first);

   if (use_tf)
   {
      glxx_tf_write_primitives(state, mode, draw->count, draw->instance_count);
      v3d_cl_transform_feedback_enable(&instr, 0, 0, 1);
      v3d_cl_add_16(&instr, 0);
   }

   return true;
}

static bool glxx_hw_draw_elements_record(GLXX_SERVER_STATE_T *state,
   GLXX_HW_RENDER_STATE_T *rs,
   const GLXX_DRAW_T *draw,
   const GLXX_ATTRIBS_MAX *attribs_max,
   const GLXX_STORAGE_T *indices)
{
   uint8_t *instr;
   unsigned int max_index;
   v3d_addr_t indices_addr;
   v3d_prim_mode_t mode = convert_primitive_type(draw->mode);
   v3d_index_type_t index_type = convert_index_type(draw->index_type);
   int size = (draw->instance_count > 1) ?
      V3D_CL_INDEXED_INSTANCED_PRIM_LIST_SIZE : V3D_CL_INDEXED_PRIM_LIST_SIZE;

   if (draw->basevertex || draw->baseinstance)
      size += V3D_CL_BASE_VERTEX_BASE_INSTANCE_SIZE;

   assert(draw->instance_count >= 1);

   instr = khrn_fmem_cle(&rs->fmem, size);
   if (!instr)
      return false;

   max_index = MIN(draw->max_index, attribs_max->index);

   if (draw->basevertex || draw->baseinstance)
      v3d_cl_base_vertex_base_instance(&instr, draw->basevertex, draw->baseinstance);

   indices_addr = khrn_fmem_lock_and_sync(&rs->fmem, indices->handle, GMEM_SYNC_CORE_READ, GMEM_SYNC_CORE_READ);
   indices_addr += indices->offset;

   if (draw->instance_count > 1)
   {
      v3d_cl_indexed_instanced_prim_list(&instr, mode, index_type,
            draw->count, draw->instance_count, indices_addr, max_index,
            state->caps.primitive_restart);
   }
   else
   {
      v3d_cl_indexed_prim_list(&instr, mode, index_type, draw->count, indices_addr,
            max_index, state->caps.primitive_restart, draw->min_index);
   }

   return true;
}

static v3d_addr_t core_read_storage(GLXX_HW_RENDER_STATE_T *rs, const GLXX_STORAGE_T *storage)
{
   v3d_addr_t addr = khrn_fmem_lock_and_sync(
      &rs->fmem, storage->handle, GMEM_SYNC_CORE_READ, GMEM_SYNC_CORE_READ);
   addr += storage->offset;
   return addr;
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

   v3d_prim_mode_t mode = convert_primitive_type(draw->mode);
   unsigned size = V3D_CL_INDIRECT_PRIMITIVE_LIMITS_SIZE;
   v3d_addr_t indirect_addr = core_read_storage(rs, indirect);

   // TODO v3d_cl_indirect_primitive_limits() can be omitted if buffers have not changed
   if (draw->is_draw_arrays)
      size += V3D_CL_INDIRECT_VERTEX_ARRAY_PRIMS_SIZE;
   else
      size += V3D_CL_INDIRECT_INDEXED_PRIM_LIST_SIZE;

   uint8_t *instr = khrn_fmem_cle(&rs->fmem, size);
   if (!instr)
      return false;

   if (!draw->is_draw_arrays)
   {
      v3d_index_type_t index_type = convert_index_type(draw->index_type);
      unsigned max_index = MIN(draw->max_index, attribs_max->index);
      unsigned type_size = khrn_get_type_size(draw->index_type, 1);
      unsigned num_indices = (indices->size - indices->offset) / type_size;
      v3d_addr_t indices_addr = core_read_storage(rs, indices);

      // draw->max_index is based on index type (0xff, 0xfff, 0x00ffffff)
      // attribs_max->index is maximum index allowed across all enabled attribs
      v3d_cl_indirect_primitive_limits(&instr, max_index, attribs_max->instance, num_indices);
      v3d_cl_indirect_indexed_prim_list(
         &instr, mode, index_type, draw->num_indirect, state->caps.primitive_restart,
         indirect_addr, indices_addr, draw->indirect_stride);
   }
   else
   {
      // draw->max_index is vertex buffer size - 1 and we cannot index past it
      v3d_cl_indirect_primitive_limits(&instr, attribs_max->index, attribs_max->instance, 0);
      v3d_cl_indirect_vertex_array_prims(&instr, mode, draw->num_indirect, indirect_addr, draw->indirect_stride);
   }

   return true;
}

static bool update_vcm_cache_size(GLXX_LINK_RESULT_DATA_T *link_data, GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs)
{
   // The hardware vertex cache manager (VCM) receives a stream of indices and produces
   // batches of unique vertices and a remapped index stream. Each thread's VCM
   // (bin/render) can be configured to generate references to vertices in the last
   // 1-4 batches. A thread must not require more than half the total VPM memory at any
   // one time since a deadlock can occur if this happens in conjunction with the other
   // thread.

   // The VPM has khrn_get_vpm_size() bytes of memory, split into blocks of 512 bytes.
   // For the purpose of configuring the VCM, we allocate half to each thread.
   // An individual thread can use more than this if available, but is always
   // guaranteed to make progress with only half.

   const unsigned vpm_size_in_bytes = khrn_get_vpm_size();
   const unsigned max_blocks_per_thread = vpm_size_in_bytes / (512 * 2);

   // compute VCM cache size for bin then render
   unsigned vcm_cache_sizes[2];
   for (unsigned i = 0; i != 2; ++i)
   {
      // The sizes in link_data are given in 512 byte blocks
      unsigned output_size = (i == 0 ? link_data->cs_output_size : link_data->vs_output_size);
      unsigned input_size = 0;
      if (link_data->flags & (i == 0 ? GLXX_SHADER_FLAGS_CS_SEPARATE_I_O_VPM_BLOCKS : GLXX_SHADER_FLAGS_VS_SEPARATE_I_O_VPM_BLOCKS))
      {
         input_size = output_size & 15;
         output_size = output_size >> 4;
      }
      assert(output_size > 0);

      // We need space for an additional input and output segment, on top of the span
      // of segments referenced by the remapped indices from the VCM as a new segment
      // is required to displace the oldest one in the PTB/PSE (see v3d_vbt.v).
      vcm_cache_sizes[i] = gfx_umin((max_blocks_per_thread - input_size - output_size) / output_size, 4u);

      // Since the minimum VPM configurations is 16kb, and the maximum batch size is 4kb
      // this shouldn't ever happen unless we start using separate_io as well
      // (which currently we don't). If we want to use separate_io for anything other than
      // testing then it would be necessary to disable concurrent bin/render for control
      // lists that cannot work with half the available VPM memory.
      assert((input_size + output_size*2) <= max_blocks_per_thread);
   }

   // if using z-prepass, then account for running the coordinate shader in rendering
   if (rs->z_prepass_started && !rs->z_prepass_stopped)
   {
      vcm_cache_sizes[1] = gfx_umin(vcm_cache_sizes[0], vcm_cache_sizes[1]);
   }

   // be conservative on 7250 and platforms with a small VCM for now
   if (vpm_size_in_bytes <= (16*1024))
   {
      vcm_cache_sizes[0] = 1;
      vcm_cache_sizes[1] = 1;
   }

   // write the vcm cache size if changed.
   // the initial values in rs are zero, so this will always execute the first time.
   if (rs->vcm_cache_size_bin != vcm_cache_sizes[0] || rs->vcm_cache_size_render != vcm_cache_sizes[1])
   {
      uint8_t* instr = glxx_hw_render_state_begin_cle(rs, GLXX_CL_STATE_VCM_CACHE_SIZE);
      if (!instr)
         return false;

      rs->vcm_cache_size_bin = (uint8_t)vcm_cache_sizes[0];
      rs->vcm_cache_size_render = (uint8_t)vcm_cache_sizes[1];
      v3d_cl_vcm_cache_size(&instr, vcm_cache_sizes[0], vcm_cache_sizes[1]);

      glxx_hw_render_state_end_cle(rs, GLXX_CL_STATE_VCM_CACHE_SIZE, instr);
   }
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
   GLXX_LINK_RESULT_DATA_T       *link_data;
   bool                          record;
   GLXX_HW_SHADER_RECORD_INFO_T  record_info;

   /* create or retrieve shaders from cache and setup attribs_live */
   link_data = glxx_get_shaders(state);
   if (!link_data)
   {
      vcos_log_warn("[%s] shader compilation failed for %p", VCOS_FUNCTION, state);
      goto fail;
   }

   /* If using flow control, then driver needs to work around GFXH-1181. */
   rs->fmem.br_info.bin_workaround_gfxh_1181 |= link_data->bin_uses_control_flow;
   rs->fmem.br_info.render_workaround_gfxh_1181 |= link_data->render_uses_control_flow;

   /* Build the shader record */
   create_gl_shader_record(
      &rs->fmem,
      state,
      link_data,
      attrib_config,
      vb_config,
      vertex_pointers,
      state->generic_attrib,
      &record_info
   );
   if (!record_info.shader_record)
   {
      vcos_logc_warn((&glxx_draw_log), "%s: creating shader record failed", VCOS_FUNCTION);
      goto fail;
   }

   // update z-prepass state
   if (!glxx_hw_update_z_prepass_state(state, link_data, rs))
      goto fail;

#ifdef KHRN_GEOMD
   /* It would be nice to use address of the draw instruction. However, this
    * would only be useful during binning. We use shader record address
    * instead, as this gets binned to tile list and thus will be useful during
    * rendering as well. This does mean we must have unique shader record for
    * each draw call when using debug info. */
   fmem_debug_info_insert(
      &rs->fmem, record_info.shader_record, state->debug.draw_id++, khrn_hw_render_state_allocated_order(rs));
#endif

   /* emit any necessary config change instructions */
   {
      bool nothing_to_draw = false, ok;
      ok = do_changed_cfg(rs, state, &nothing_to_draw);
      ok &= insert_flat_shading_flags(rs, state, link_data);
      ok &= insert_centroid_flags(rs, state, link_data);
      if (!ok)
      {
         vcos_logc_warn((&glxx_draw_log), "%s: applying state changes failed",
               VCOS_FUNCTION);
         goto fail;
      }
      if (nothing_to_draw)
      {
         vcos_logc_info((&glxx_draw_log), "%s: nothing to draw", VCOS_FUNCTION);
         return true;
      }
   }

   if (!update_vcm_cache_size(link_data, state, rs))
      goto fail;

   glxx_server_active_queries_install(state, rs);

   if (!record_shader_record(&rs->fmem, &record_info))
      goto fail;

   // Instanced renders on 3.2 might encounter GFXH-1313.
   if ((draw->instance_count > 1 || draw->is_indirect) && khrn_get_v3d_version() == V3D_MAKE_VER(3,2))
      rs->workaround_gfxh_1313 = true;

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
      res_i = glxx_buffer_get_res_interlock(buffer);
      indirect_storage.handle = res_i->handle;
      assert(draw->indirect_offset <= UINT_MAX);
      indirect_storage.offset = (unsigned int)draw->indirect_offset;

      khrn_fmem_record_res_interlock(&rs->fmem, res_i, true, ACTION_BOTH);

      record = glxx_hw_draw_indirect_record(
         state, rs, draw, attribs_max, indices, &indirect_storage);
   }

   if (record == false)
   {
      vcos_logc_warn((&glxx_draw_log),"%s: draw record failed", VCOS_FUNCTION);
      goto fail;
   }

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
   if (GLXX_MULTICORE_BIN_ENABLED && rs->do_multicore_bin && rs->tf_waited_count == 0)
   {
      if (!post_draw_journal(rs, attrib_config, vb_config, vertex_pointers, link_data, draw))
      {
         return false;
      }
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
      uint8_t* instr = khrn_fmem_cle(&rs->fmem, V3D_CL_NOP_SIZE);
      if (!instr)
      {
         return false;
      }

      v3d_cl_nop(&instr);
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
   GLXX_BINARY_CACHE_T *cache;
   IR_PROGRAM_T *ir;

   assert(IS_GL_11(state) || state->program != NULL);

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
      cache = &state->program->cache;
      ir    = state->program->linked_glsl_program->ir;
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
   const GLXX_TEXTURE_UNIF_T *tu = tus + backend_uniform_get_sampler(u_value);

   uint32_t bits;
   uint32_t extra_bits;
   switch (param)
   {
   case 0:
      bits = tu->hw_param0;
#if V3D_HAS_NEW_TMU_CFG
      extra_bits = backend_uniform_get_extra(u_value);
#else
      extra_bits = backend_uniform_get_extra_param0(u_value);
#endif
      break;
   case 1:
#if V3D_HAS_NEW_TMU_CFG
      bits = tu->hw_param1;
      extra_bits = backend_uniform_get_extra(u_value);
#else
      if (u_value & GLSL_TEXBITS_GATHER) {
         int comp = (u_value & GLSL_TEXBITS_GATHER_COMP_MASK) >> GLSL_TEXBITS_GATHER_COMP_SHIFT;
         bits = tu->hw_param1_gather[comp];
      } else
         bits = tu->hw_param1;
      extra_bits = backend_uniform_get_extra_param1(u_value);
#endif
      break;
   default:
      unreachable();
   }

   assert(!(bits & extra_bits));
   bits |= extra_bits;

   return bits;
}

v3d_addr_t glxx_hw_install_uniforms(
   KHRN_FMEM_T                      *fmem,
   const GLXX_SERVER_STATE_T        *state,
   const GLXX_UNIFORM_MAP_T         *map,
   GL20_HW_INDEXED_UNIFORM_T        *iu,
   const glxx_hw_compute_uniforms   *compute_uniforms)
{
   uint32_t *ptr = khrn_fmem_data(fmem, 4 * map->count, V3D_QPU_UNIFS_ALIGN);
   if (!ptr)
      return 0;
   v3d_addr_t result = khrn_fmem_hw_address(fmem, ptr);

   uint32_t *data;
   if (!IS_GL_11(state)) data = state->program->uniform_data;
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

      case BACKEND_UNIFORM_BLOCK_ADDRESS:
      case BACKEND_UNIFORM_SSBO_ADDRESS:
      {
         /* Not having a buffer bound gives undefined behaviour - we crash */
         const GLXX_INDEXED_BINDING_POINT_T *binding;
         bool write = false;
         if (u_type == BACKEND_UNIFORM_SSBO_ADDRESS) {
            uint32_t binding_point = state->program->ssbo_binding_point[u_value];
            binding                = &state->ssbo.binding_points[binding_point];
            write = true;
         } else {
            uint32_t binding_point = state->program->ubo_binding_point[u_value];
            binding                = &state->uniform_block.binding_points[binding_point];
         }
         const GLXX_BUFFER_T *buffer = binding->buffer.obj;
         KHRN_RES_INTERLOCK_T *res_i = glxx_buffer_get_res_interlock(buffer);

         khrn_fmem_record_res_interlock(fmem, res_i, write, ACTION_BOTH);

         v3d_addr_t uniform_addr;
         uint32_t action = GMEM_SYNC_TMU_DATA_READ | (write ? GMEM_SYNC_TMU_DATA_WRITE : 0);
         uniform_addr = khrn_fmem_lock_and_sync(fmem, res_i->handle, action, action);
         put_word((uint8_t*)ptr, uniform_addr + binding->offset);
         break;
      }

#if V3D_HAS_NEW_TMU_CFG
      case BACKEND_UNIFORM_TEX_PARAM1:
         put_word(ptr, assemble_tmu_param(1, u_value, state->texture_unif));
         break;
#else
      case BACKEND_UNIFORM_TEX_PARAM1:
      {
         uint32_t p1_bits = assemble_tmu_param(1, u_value, state->texture_unif);
         uint32_t p0_bits = assemble_tmu_param(0, u_value, state->texture_unif);

         V3D_TMU_PARAM0_T p0;
         v3d_unpack_tmu_param0(&p0, p0_bits);
         assert(p0.cfg == 1);

         if (p0.u.cfg1.fetch)
         {
            V3D_TMU_PARAM1_CFG1_T p1;
            v3d_unpack_tmu_param1_cfg1(&p1, p1_bits);
            p1.unnorm = false; /* Must not set both fetch & unnorm! */
            p1_bits = v3d_pack_tmu_param1_cfg1(&p1);
         }

         put_word(ptr, p1_bits);
         break;
      }
#endif

      case BACKEND_UNIFORM_TEX_PARAM0:
         put_word(ptr, assemble_tmu_param(0, u_value, state->texture_unif));
         break;

      case BACKEND_UNIFORM_TEX_SIZE_X:
         put_word(ptr, state->texture_unif[u_value].width);
         break;

      case BACKEND_UNIFORM_TEX_SIZE_Y:
         put_word(ptr, state->texture_unif[u_value].height);
         break;

      case BACKEND_UNIFORM_TEX_SIZE_Z:
         put_word(ptr, state->texture_unif[u_value].depth);
         break;

      case BACKEND_UNIFORM_SPECIAL:
      {
         const BackendSpecialUniformFlavour u_special = u_value;

         assert(u_special >= 0 && u_special <= BACKEND_SPECIAL_UNIFORM_LAST_ELEMENT);

         switch (u_special) {

         case BACKEND_SPECIAL_UNIFORM_VP_SCALE_X:
            put_word(ptr, float_to_bits(state->viewport.internal_xscale));
            break;

         case BACKEND_SPECIAL_UNIFORM_VP_SCALE_Y:
            put_word(ptr, float_to_bits(state->viewport.internal_yscale));
            break;

         case BACKEND_SPECIAL_UNIFORM_VP_SCALE_Z:
            put_word(ptr, float_to_bits(state->viewport.internal_zscale));
            break;

         case BACKEND_SPECIAL_UNIFORM_VP_SCALE_W:
            put_word(ptr, float_to_bits(state->viewport.internal_wscale));
            break;

         case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_NEAR:
            put_word(ptr, float_to_bits(state->viewport.internal_dr_near));
            break;

         case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_FAR:
            put_word(ptr, float_to_bits(state->viewport.internal_dr_far));
            break;

         case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_DIFF:
            put_word(ptr, float_to_bits(state->viewport.internal_dr_diff));
            break;

         case BACKEND_SPECIAL_UNIFORM_SAMPLE_MASK:
            put_word(ptr, state->sample_mask.mask[0]);
            break;

         case BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_X:
            put_word(ptr, compute_uniforms->num_work_groups[0]);
            break;

         case BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_Y:
            put_word(ptr, compute_uniforms->num_work_groups[1]);
            break;

         case BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_Z:
            put_word(ptr, compute_uniforms->num_work_groups[2]);
            break;

         case BACKEND_SPECIAL_UNIFORM_SHARED_PTR:
            put_word(ptr, compute_uniforms->shared_ptr);
            break;
         }
         break;
      }

      case BACKEND_UNIFORM_ADDRESS:
         if (!backend_uniform_address(fmem, u_value, state->program, iu, ptr)) {
            return 0;
         }
         break;

      case BACKEND_UNIFORM_UNASSIGNED:
         UNREACHABLE();
         break;
      }

      ptr++;
   }

   return result;
}

static bool backend_uniform_address(
   KHRN_FMEM_T *fmem,
   uint32_t u_value,
   const GL20_PROGRAM_T *program,
   GL20_HW_INDEXED_UNIFORM_T *iu,
   uint32_t *location)
{
   uint32_t index = u_value & 0xffff;

   if (!iu->valid) {
      v3d_addr_t texa;
      uint32_t *tex;
      /* Copy the whole default uniform block into fmem */
      tex = khrn_fmem_data(fmem, 4*program->num_scalar_uniforms, 16);
      if (tex == NULL) return false;

      memcpy(tex, program->uniform_data, 4*program->num_scalar_uniforms);
      texa = khrn_fmem_hw_address(fmem, tex);
      iu->block_addr = texa;
      iu->valid = true;
   }

   *location = iu->block_addr + 4*index;    /* XXX Am I allowed fmem-pointer arithmetic? */
   return true;
}

/*!
 * \brief Converts the primitive type from its GLenum representation
 *        to the representation used in the simulator (unsigned int).
 *
 * Also asserts that the mode is within the valid/implemented range.
 *
 * \param mode is the GL primitive type.
 */
static v3d_prim_mode_t convert_primitive_type(GLenum mode)
{
   static_assrt(GL_POINTS         == V3D_PRIM_MODE_POINTS    );
   static_assrt(GL_LINES          == V3D_PRIM_MODE_LINES     );
   static_assrt(GL_LINE_LOOP      == V3D_PRIM_MODE_LINE_LOOP );
   static_assrt(GL_LINE_STRIP     == V3D_PRIM_MODE_LINE_STRIP);
   static_assrt(GL_TRIANGLES      == V3D_PRIM_MODE_TRIS      );
   static_assrt(GL_TRIANGLE_STRIP == V3D_PRIM_MODE_TRI_STRIP );
   static_assrt(GL_TRIANGLE_FAN   == V3D_PRIM_MODE_TRI_FAN   );

   assert(mode <= GL_TRIANGLE_FAN);

   return (v3d_prim_mode_t)mode;
}

static v3d_prim_mode_t convert_primitive_type_with_transform_feedback(GLenum mode)
{
   switch (mode)
   {
   case GL_POINTS   : return V3D_PRIM_MODE_POINTS_TF;
   case GL_LINES    : return V3D_PRIM_MODE_LINES_TF;
   case GL_TRIANGLES: return V3D_PRIM_MODE_TRIS_TF;
   }

   unreachable();

   return V3D_PRIM_MODE_INVALID;
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
      UNREACHABLE(); // unsupported index type
      return 0;
   }
}

static v3d_attr_type_t translate_attrib_type(GLenum type)
{
   switch (type)
   {
   case GL_NONE:                          return V3D_ATTR_TYPE_DISABLED; // GL 1.x can use this
   case GL_BYTE:                          return V3D_ATTR_TYPE_BYTE;
   case GL_UNSIGNED_BYTE:                 return V3D_ATTR_TYPE_BYTE;
   case GL_SHORT:                         return V3D_ATTR_TYPE_SHORT;
   case GL_UNSIGNED_SHORT:                return V3D_ATTR_TYPE_SHORT;
   case GL_INT:                           return V3D_ATTR_TYPE_INT;
   case GL_UNSIGNED_INT:                  return V3D_ATTR_TYPE_INT;
   case GL_FIXED:                         return V3D_ATTR_TYPE_FIXED;
   case GL_FLOAT:                         return V3D_ATTR_TYPE_FLOAT;
   case GL_HALF_FLOAT_OES:                return V3D_ATTR_TYPE_HALF_FLOAT;
   case GL_HALF_FLOAT:                    return V3D_ATTR_TYPE_HALF_FLOAT;
   case GL_INT_2_10_10_10_REV:            return V3D_ATTR_TYPE_INT2_10_10_10;
   case GL_UNSIGNED_INT_2_10_10_10_REV:   return V3D_ATTR_TYPE_INT2_10_10_10;
   default:
      UNREACHABLE();
      return 0;
   }
}

static bool translate_attrib_signed_int(GLenum type)
{
   switch (type)
   {
   case GL_NONE:                          return false; // GL 1.x can use this
   case GL_BYTE:                          return true;
   case GL_UNSIGNED_BYTE:                 return false;
   case GL_SHORT:                         return true;
   case GL_UNSIGNED_SHORT:                return false;
   case GL_INT:                           return true;
   case GL_UNSIGNED_INT:                  return false;
   case GL_FIXED:                         return false;
   case GL_FLOAT:                         return false;
   case GL_HALF_FLOAT_OES:                return false;
   case GL_HALF_FLOAT:                    return false;
   case GL_INT_2_10_10_10_REV:            return true;
   case GL_UNSIGNED_INT_2_10_10_10_REV:   return false;
   default:
      UNREACHABLE();
      return 0;
   }
}

// Packs a float attribute with given size and number of cs/vs reads into
// ith attribute in shader record. Packed base points to first attribute.
static void make_simple_float_shader_attr(V3D_SHADREC_GL_ATTR_T *attr,
   int size, int cs_reads, int vs_reads, v3d_addr_t addr, int stride)
{
   attr->addr            = addr;
   attr->size            = size;
   attr->type            = V3D_ATTR_TYPE_FLOAT;
   attr->signed_int      = false;
   attr->normalised_int  = false;
   attr->read_as_int     = false;
   attr->cs_num_reads    = cs_reads;
   attr->vs_num_reads    = vs_reads;
   attr->divisor         = 0;
   attr->stride          = stride;
}

/* Returns true if b was succesfully merged to a */
static bool maybe_merge_attr(V3D_SHADREC_GL_ATTR_T *a, V3D_SHADREC_GL_ATTR_T *b)
{
   unsigned a_size = v3d_attr_get_size_in_memory(a);
   v3d_addr_t a_end = a->addr + a_size;
   if (
      (a->type == b->type) &&
      (a->stride == b->stride) &&
      (a->divisor == b->divisor) &&
      (a->signed_int == b->signed_int) &&
      (a->normalised_int == b->normalised_int) &&
      (a->read_as_int == b->read_as_int) &&
      (a->size + b->size <= 4) &&
      (a->cs_num_reads <= a->size) &&
      (a->vs_num_reads <= a->size) &&
      (b->cs_num_reads <= b->size) &&
      (b->vs_num_reads <= b->size) &&
      (a_end == b->addr)
   )
   {
      a->size += b->size;
      a->cs_num_reads += b->cs_num_reads;
      a->vs_num_reads += b->vs_num_reads;
      assert(a->size <= 4);
      assert(a->cs_num_reads <= 4);
      assert(a->vs_num_reads <= 4);
      return true;
   }
   return false;
}

/* Work around GFXH-1276. 0 varyings causes memory corruption. */
static void workaround_gfxh_1276(V3D_SHADREC_GL_MAIN_T *record) {
   /* This is more complicated for points, so for now skip the workaround */
   if (record->num_varys == 0 && !record->point_size_included)
      record->num_varys = 1;
}

static bool create_gl_shader_record_inner(
   V3D_SHADREC_GL_MAIN_T            *shader_record,
   V3D_SHADREC_GL_ATTR_T            *attr,
   KHRN_FMEM_T                      *fmem,
   const GLXX_SERVER_STATE_T        *state,
   const GLXX_LINK_RESULT_DATA_T    *data,
   const GLXX_ATTRIB_CONFIG_T       *attrib,
   const GLXX_VERTEX_BUFFER_CONFIG_T *vb_config,
   const GLXX_VERTEX_POINTERS_T     *vertex_pointers,
   const GLXX_GENERIC_ATTRIBUTE_T   *generic_attrib)
{
   unsigned   num_default_block_attributes = 0;
   v3d_addr_t defaults_hw;
   uint32_t  *defaults = NULL;

   // Note that defaults memory is used for three distinct purposes:
   // 0. No defaults used, data is sourced for vertex attribute pointers
   // 1. Disabled attributes store generic attribute values
   //    here, attributes point directly inside defaults with stride = 0.
   //    All attribute setup information comes from generic_attrib.
   // 2. Implicit values for components not specified by attribute
   //    setup in GL get their values (0, 0, 0, 1) through the
   //    defaults block, which pointer is in shader record when
   //    GFXH-967 is not implemented.
   // 3. Working around GFXH-930. See the workaround function elsewhere.

   // Count how many attibutes are needed in shader record and how
   // much memory is needed for defaults
   for (unsigned n = 0; n < data->attr_count; n++)
   {
      unsigned shader_reads = gfx_umax(data->attr[n].c_scalars_used, data->attr[n].v_scalars_used);
      const GLXX_ATTRIB_CONFIG_T *attrib_cfg = &attrib[data->attr[n].idx];

      if (attrib_cfg->enabled)
      {
         if (shader_reads > attrib_cfg->size)
            // Need default values for all attributes due to
            // current Simpenrose implementation.
            num_default_block_attributes = data->attr_count;
      }
      else
      {
         // Need generic attribute values up to this attribute at least
         num_default_block_attributes = gfx_umax(
            num_default_block_attributes,
            n + 1);
      }
   }

   if (num_default_block_attributes > 0)
   {
      defaults = khrn_fmem_data(fmem, num_default_block_attributes * 16, V3D_ATTR_DEFAULTS_ALIGN);
      if (!defaults)
      {
         vcos_logc_warn(
            (&glxx_draw_log), "%s: no memory for defaults block", VCOS_FUNCTION);
         return false;
      }
      defaults_hw = khrn_fmem_hw_address(fmem, defaults);
   }
   else
   {
      defaults = NULL;
      defaults_hw = 0;
   }

   shader_record->point_size_included  = !!(data->flags & GLXX_SHADER_FLAGS_POINT_SIZE_SHADED_VERTEX_DATA);
   shader_record->clipping             = !!(data->flags & GLXX_SHADER_FLAGS_ENABLE_CLIPPING);
   shader_record->cs_vertex_id         = !!(data->flags & GLXX_SHADER_FLAGS_CS_READS_VERTEX_ID);
   shader_record->cs_instance_id       = !!(data->flags & GLXX_SHADER_FLAGS_CS_READS_INSTANCE_ID);
   shader_record->vs_vertex_id         = !!(data->flags & GLXX_SHADER_FLAGS_VS_READS_VERTEX_ID);
   shader_record->vs_instance_id       = !!(data->flags & GLXX_SHADER_FLAGS_VS_READS_INSTANCE_ID);
   shader_record->z_write              = !!(data->flags & GLXX_SHADER_FLAGS_FS_WRITES_Z);
   shader_record->no_ez                = !!(data->flags & GLXX_SHADER_FLAGS_FS_NOT_EARLY_Z_COMPATIBLE);
   shader_record->cs_separate_blocks   = !!(data->flags & GLXX_SHADER_FLAGS_CS_SEPARATE_I_O_VPM_BLOCKS);
   shader_record->vs_separate_blocks   = !!(data->flags & GLXX_SHADER_FLAGS_VS_SEPARATE_I_O_VPM_BLOCKS);
   shader_record->scb_wait_on_first_thrsw = false;
   shader_record->disable_scb          = false;
   // TODO centroid flag

   shader_record->num_varys         = data->num_varys;
   shader_record->fs_working_size   = 0;
   shader_record->cs_output_size    = data->cs_output_size;
   shader_record->cs_working_size   = 0;
   shader_record->vs_output_size    = data->vs_output_size;
   shader_record->vs_working_size   = 0;
   shader_record->defaults          = defaults_hw;
   shader_record->fs.threading      = data->f.threading;
   shader_record->vs.threading      = data->v.threading;
   shader_record->cs.threading      = data->c.threading;
   shader_record->fs.propagate_nans = true;
   shader_record->vs.propagate_nans = true;
   shader_record->cs.propagate_nans = true;

   /* Modify the shader record to avoid specifying 0 varyings */
   if (khrn_get_v3d_version() < V3D_MAKE_VER(3,3))
      workaround_gfxh_1276(shader_record);

   {
      bool ok = true;

      shader_record->fs.addr = khrn_fmem_lock_and_sync(fmem, data->f.res_i->handle, 0, GMEM_SYNC_QPU_IU_READ);
      shader_record->vs.addr = khrn_fmem_lock_and_sync(fmem, data->v.res_i->handle, 0, GMEM_SYNC_QPU_IU_READ);
      shader_record->cs.addr = khrn_fmem_lock_and_sync(fmem, data->c.res_i->handle, GMEM_SYNC_QPU_IU_READ, GMEM_SYNC_QPU_IU_READ);
      ok &= khrn_fmem_record_res_interlock(fmem, data->v.res_i, false, ACTION_RENDER);
      ok &= khrn_fmem_record_res_interlock(fmem, data->c.res_i, false, ACTION_BOTH);
      ok &= khrn_fmem_record_res_interlock(fmem, data->f.res_i, false, ACTION_RENDER);

      if (!ok)
      {
         vcos_logc_warn((&glxx_draw_log), "%s: khrn_fmem_record_res_interlock() for shader code failed", VCOS_FUNCTION);
         return false;
      }
   }

   {
      GL20_HW_INDEXED_UNIFORM_T  iu;
      iu.valid = false;

      shader_record->cs.unifs_addr = glxx_hw_install_uniforms(fmem, state, data->c.uniform_map, &iu, NULL);
      shader_record->vs.unifs_addr = glxx_hw_install_uniforms(fmem, state, data->v.uniform_map, &iu, NULL);
      shader_record->fs.unifs_addr = glxx_hw_install_uniforms(fmem, state, data->f.uniform_map, &iu, NULL);
      if (!shader_record->cs.unifs_addr || !shader_record->vs.unifs_addr || !shader_record->fs.unifs_addr)
      {
         vcos_logc_warn((&glxx_draw_log), "%s: install_uniforms() failed", VCOS_FUNCTION);
         return false;
      }
   }

   assert(data->attr_count <= GLXX_CONFIG_MAX_VERTEX_ATTRIBS);

   for (unsigned n = 0; n < data->attr_count; n++)
   {
      unsigned shader_reads = gfx_umax(data->attr[n].c_scalars_used, data->attr[n].v_scalars_used);
      const GLXX_ATTRIB_CONFIG_T *attrib_cfg = &attrib[data->attr[n].idx];

      if (attrib_cfg->enabled)
      {
         // Cases 0 and 2
         const GLXX_STORAGE_T *vertex_p = &vertex_pointers->array[data->attr[n].idx];
         v3d_addr_t attrib_addr;

         assert(vertex_p->handle != GMEM_HANDLE_INVALID);

         attrib_addr = khrn_fmem_lock_and_sync(
            fmem, vertex_p->handle, GMEM_SYNC_VCD_READ, GMEM_SYNC_VCD_READ);

         attr[n].addr            = attrib_addr + vertex_p->offset;
         attr[n].size            = attrib_cfg->size;
         attr[n].type            = translate_attrib_type(attrib_cfg->type);
         attr[n].signed_int      = translate_attrib_signed_int(attrib_cfg->type);
         attr[n].normalised_int  = attrib_cfg->norm;
         attr[n].read_as_int     = attrib_cfg->is_int;
         attr[n].cs_num_reads    = data->attr[n].c_scalars_used;
         attr[n].vs_num_reads    = data->attr[n].v_scalars_used;
         attr[n].divisor         = vb_config[data->attr[n].idx].divisor;
         attr[n].stride          = vb_config[data->attr[n].idx].stride;

         // Case 2
         if (shader_reads > attrib_cfg->size)
         {
            for (int i = 0; i < 4; ++i)
            {
               if (attrib_cfg->is_int)
                  defaults[n * 4 + i] = (i == 3) ? 1 : 0;
               else
                  defaults[n * 4 + i] = gfx_float_to_bits((i == 3) ? 1.0f : 0.0f);
            }
         }
      }
      else
      {
         const GLXX_GENERIC_ATTRIBUTE_T *generic_cfg = &generic_attrib[data->attr[n].idx];
         // Case 1
         if (generic_cfg->type == GL_FLOAT)
         {
            for (int i = 0; i < 4; ++i)
            {
               defaults[n * 4 + i] = gfx_float_to_bits(generic_cfg->value[i]);
            }
         }
         else
         {
            for (int i = 0; i < 4; ++i)
            {
               defaults[n * 4 + i] = generic_cfg->value_int[i];
            }
         }

         attr[n].addr            = defaults_hw + n * 16;
         attr[n].size            = shader_reads;
         attr[n].type            = translate_attrib_type(generic_cfg->type);
         attr[n].signed_int      = translate_attrib_signed_int(generic_cfg->type);
         attr[n].normalised_int  = false;
         attr[n].read_as_int     = generic_cfg->type == GL_FLOAT ? false:true;
         attr[n].cs_num_reads    = data->attr[n].c_scalars_used;
         attr[n].vs_num_reads    = data->attr[n].v_scalars_used;
         attr[n].divisor         = 0;
         attr[n].stride          = 0;
      }
   }

   return true;
}

static bool workaround_GFXH_930(
   KHRN_FMEM_T             *fmem,
   V3D_SHADREC_GL_ATTR_T   *attr,
   unsigned                *num_attributes
)
{
   unsigned total_cs_reads = 0;
   unsigned total_vs_reads = 0;
   unsigned i;

   // Hardware needs at least one attribute read in both coordinate
   // shader, and one attribute read by the vertex shader. They don't
   // have to be the same attribute
   //
   // There are three cases to consider
   // 1.  No attributes are read by either CS or VS.
   //     In this case we pretend there is one attribute, and
   //     set it to be read by both CS and VS.
   // 2.  Attributes are read by VS but not CS.
   //     In this case we pick one attribute read by VS and
   //     pretend it is also read by CS.
   // 3.  Attirbutes are read by CS but not VS.
   //     In this case we pick one attribute read by CS and
   //     pretend it is also read by VS.

   for (i = 0; i < *num_attributes; ++i)
   {
      total_cs_reads += attr[i].cs_num_reads;
      total_vs_reads += attr[i].vs_num_reads;
   }

   // Case 1.
   // This also covers case when *num_attributes == 0
   if ((total_cs_reads == 0) && (total_vs_reads == 0))
   {
      uint32_t *defaults = khrn_fmem_data(fmem, 16, V3D_ATTR_DEFAULTS_ALIGN);
      v3d_addr_t defaults_hw;
      if (!defaults)
         return false;

      defaults_hw = khrn_fmem_hw_address(fmem, defaults);
      for (i = 0; i < 4; ++i)
         defaults[i] = gfx_float_to_bits((i == 3) ? 1.0f : 0.0f);

      make_simple_float_shader_attr(&attr[0], 1, 1, 1, defaults_hw, 0);

      *num_attributes = 1;
   }
   else
   {
      // We read some attributes either in CS or VS, but not necessarily both
      // Pretend that at least one attribute is read by both CS and VS
      // Case 2
      if (total_cs_reads == 0)
      {
         for (i = 0; i < *num_attributes; ++i)
         {
            if (attr[i].vs_num_reads > 0)
            {
               attr[i].cs_num_reads = 1;
               break;
            }
         }
      }

      // Case 3
      if (total_vs_reads == 0)
      {
         for (i = 0; i < *num_attributes; ++i)
         {
            if (attr[i].cs_num_reads > 0)
            {
               attr[i].vs_num_reads = 1;
               break;
            }
         }
      }
   }

   return true;
}

static void workaround_large_attribute_divisor(
   KHRN_FMEM_T             *fmem,
   V3D_SHADREC_GL_ATTR_T   *attr,
   unsigned                *num_attributes
)
{
   unsigned i;

   // If the divisor was set to 0x10000, the app expects the 2^16'th instance to still
   // get its attrib data from the base of the buffer, i.e. without a stride added on.
   // Our hardware would fail in this case because it only allows divisor values up to
   // 0xFFFF. We put an assertion here to make a note of this limitation.
   // In the mean time we do allow the special case of a divisor of 0xFFFFFFFF, which
   // implies that the app never wants the attribute to advance, and wants the same
   // data to be used for every single instance. We force the stride to 0 so that if
   // we do have more than 0xFFFF instances, those ones after will still use the same
   // attribs at the beginning of the buffer. It then doesnt matter what the divisor
   // is set to, but we use 0xFFFF for illustrative purposes.

   for (i = 0; i < *num_attributes; ++i)
   {
      assert(attr[i].divisor <= 0xFFFF || attr[i].divisor == 0xFFFFFFFF);
      if (attr[i].divisor == 0xFFFFFFFF)
      {
         attr[i].divisor = 0xFFFF;
         attr[i].stride = 0;
      }
   }
}

static void merge_attributes(
   KHRN_FMEM_T             *fmem,
   V3D_SHADREC_GL_ATTR_T   *attr,
   unsigned                *num_attributes
)
{
   unsigned out_num_attributes = 1;
   unsigned w = 0; // write pos, merge destination
   unsigned r = 1; // read pos, merge source

   // See if we can merge attributes

   if (*num_attributes == 0)
      return;

   while (r < *num_attributes)
   {
      if (!maybe_merge_attr(&attr[w], &attr[r]))
      {
         // Merge failed, add new attribute
         ++out_num_attributes;
         ++w;
      }

      ++r;
   }

   *num_attributes = out_num_attributes;
}

static v3d_addr_t pack_shader_record(
   KHRN_FMEM_T             *fmem,
   V3D_SHADREC_GL_MAIN_T   *shader_record,
   V3D_SHADREC_GL_ATTR_T   *attr,
   unsigned                num_attributes
)
{
   uint32_t *shader_record_packed;
   unsigned size;
   unsigned attr_offset;
   unsigned i;

   // This allocation contains both main GL shader record,
   // followed by variable number of attribute records.
   size = V3D_SHADREC_GL_MAIN_PACKED_SIZE;
   attr_offset = size;
   size += num_attributes * V3D_SHADREC_GL_ATTR_PACKED_SIZE;
   shader_record_packed = khrn_fmem_data(fmem, size, V3D_SHADREC_ALIGN);
   if (!shader_record_packed)
      return 0;

   for (i = 0; i < num_attributes; ++i)
   {
      uint32_t *attr_packed = (uint32_t *)&((char*)shader_record_packed)[attr_offset + i * V3D_SHADREC_GL_ATTR_PACKED_SIZE];
      v3d_pack_shadrec_gl_attr(attr_packed, &attr[i]);
   }

   v3d_pack_shadrec_gl_main(shader_record_packed, shader_record);

   return khrn_fmem_hw_address(fmem, shader_record_packed);
}

static void create_gl_shader_record(
   KHRN_FMEM_T                    *fmem,
   GLXX_SERVER_STATE_T            *state,
   const GLXX_LINK_RESULT_DATA_T  *data,
   const GLXX_ATTRIB_CONFIG_T     *attrib,
   const GLXX_VERTEX_BUFFER_CONFIG_T *vb_config,
   const GLXX_VERTEX_POINTERS_T   *vertex_pointers,
   const GLXX_GENERIC_ATTRIBUTE_T *generic_attrib,
   GLXX_HW_SHADER_RECORD_INFO_T   *info
)
{
   V3D_SHADREC_GL_MAIN_T shader_record;
   V3D_SHADREC_GL_ATTR_T attr[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];

   info->shader_record = 0;
   info->num_attributes_hw = data->attr_count;
   if ( !create_gl_shader_record_inner( &shader_record, &attr[0],
                                        fmem, state, data, attrib, vb_config,
                                        vertex_pointers, generic_attrib)
   )
      return;

   if (!workaround_GFXH_930(fmem, &attr[0], &info->num_attributes_hw))
      return;

   workaround_large_attribute_divisor(fmem, &attr[0], &info->num_attributes_hw);

   if (khrn_options.merge_attributes)
      merge_attributes(fmem, &attr[0], &info->num_attributes_hw);

   info->shader_record = pack_shader_record(
      fmem, &shader_record, &attr[0], info->num_attributes_hw);
}

static v3d_addr_t create_nv_shader_record(
   KHRN_FMEM_T       *fmem,
   uint32_t          *fshader_ptr,
   uint32_t          *funif_ptr,
   uint32_t          *vdata_ptr,
   bool              does_z_writes,
   v3d_threading_t   threading)
{
   V3D_SHADREC_GL_MAIN_T shader_record;
   V3D_SHADREC_GL_ATTR_T attr[3];
   v3d_addr_t vdata = khrn_fmem_hw_address(fmem, vdata_ptr);
   uint32_t *defaults;
   int num_attr = 2;       // Standard shaded vertex format
   int vdata_stride = 12;  // Xs, Ys, Zs (clip header and 1/Wc from defaults)

   defaults = khrn_fmem_data(fmem, 2 * 16, V3D_ATTR_DEFAULTS_ALIGN);
   if (!defaults)
      return 0;

   // First attribute contains clip header (dummy values).
   // Last value is is 1/Wc for second attribute.
   for (int i = 0; i < 8; ++i)
      defaults[i] = gfx_float_to_bits((i == 7) ? 1.0f : 0.0f);

   shader_record.point_size_included  = false;
   shader_record.clipping             = false;
   shader_record.cs_vertex_id         = false; // N/A in NV shader record
   shader_record.cs_instance_id       = false; // N/A in NV shader record
   shader_record.vs_vertex_id         = false; // N/A in NV shader record
   shader_record.vs_instance_id       = false; // N/A in NV shader record
   shader_record.z_write              = does_z_writes;
   shader_record.no_ez                = false; // TODO Set this if needed
   shader_record.cs_separate_blocks   = false; // N/A in NV shader record
   shader_record.vs_separate_blocks   = false; // N/A in NV shader record
   shader_record.scb_wait_on_first_thrsw = false;
   shader_record.disable_scb          = false;
   // TODO centroid flag

   shader_record.num_varys            = 0;
   shader_record.fs_working_size      = 0;
   shader_record.cs_output_size       = 1; // Coord and IO VPM segment, measured in units of 8 words (we only need 3 words)
   shader_record.cs_working_size      = 0; // Ignored in NV shader
   shader_record.vs_output_size       = 1; // Vertex IO VPM segment, measured in units of 8 words (we only need 3 words)
   shader_record.vs_working_size      = 0; // Ignored in NV shader
   shader_record.defaults             = khrn_fmem_hw_address(fmem, defaults);
   shader_record.fs.threading         = threading;
   shader_record.vs.threading         = V3D_THREADING_T1; // Ignored in NV shader
   shader_record.cs.threading         = V3D_THREADING_T1; // Ignored in NV shader
   shader_record.fs.propagate_nans    = false;
   shader_record.vs.propagate_nans    = false; // Ignored in NV shader
   shader_record.cs.propagate_nans    = false; // Ignored in NV shader
   shader_record.fs.addr              = khrn_fmem_hw_address(fmem, fshader_ptr);
   shader_record.vs.addr              = 0;
   shader_record.cs.addr              = 0;
   shader_record.cs.unifs_addr        = 0;
   shader_record.vs.unifs_addr        = 0;
   shader_record.fs.unifs_addr        = khrn_fmem_hw_address(fmem, funif_ptr);

   /* Modify the shader record to avoid specifying 0 varyings */
   if (khrn_get_v3d_version() < V3D_MAKE_VER(3,3))
      workaround_gfxh_1276(&shader_record);

   /* Xc, Yc, Zc, Wc - clipping is disabled so we just use dummy default values */
   make_simple_float_shader_attr(&attr[0], 4, 4, 0, shader_record.defaults, 0);

   /* Xs, Ys, Zs from attribute data - 1/Wc from default values (1.0) */
   make_simple_float_shader_attr(&attr[1], 3, 4, 4, vdata, vdata_stride);

   return pack_shader_record(fmem, &shader_record, &attr[0], num_attr);
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

/* The semi-reusable part of draw_rect. Create a control list */
bool glxx_draw_alternate_cle(
      GLXX_HW_RENDER_STATE_T *rs,
      glxx_dirty_set_t *dirty,
      const GLXX_FRAMEBUFFER_T *fb,
      bool change_color, const glxx_color_write_t *color_write,
      bool change_depth,
      bool change_stencil, uint8_t stencil_value, const struct stencil_mask *stencil_mask,
      int x, int y, int xmax, int ymax,
      bool front_prims, bool back_prims, bool cwise_is_front)
{
   V3D_CL_COLOR_WMASKS_T masks;
   uint32_t size = 0;
   uint8_t *instr;

   size += V3D_CL_CLIP_SIZE;
   size += V3D_CL_CLIPZ_SIZE;
   size += V3D_CL_CFG_BITS_SIZE;
   if (change_stencil)
      size += V3D_CL_STENCIL_CFG_SIZE;
   size += V3D_CL_COLOR_WMASKS_SIZE + V3D_CL_VIEWPORT_OFFSET_SIZE;
   // nv shader and vertex array prims need separate khrn_fmem_cle() call due to
   // khrn_fmem_data() call in the middle

   // Set up control list
   instr = khrn_fmem_cle(&rs->fmem, size);
   if (!instr)
      return false;

   //  Emit scissor/clipper/viewport instructions
   v3d_cl_clip(&instr, x, y, xmax - x, ymax - y);
   v3d_cl_clipz(&instr, 0.0f, 1.0f);
   khrn_render_state_set_add(&dirty->viewport, rs);  /* Clear and render might end up with different clip rectangles - clear doesn't clip to viewport */

   //  Emit a Configuration record
   gfx_ez_update_cfg(&rs->ez,
      V3D_COMPARE_FUNC_ALWAYS, change_depth,
      change_stencil,
      V3D_STENCIL_OP_ZERO, V3D_STENCIL_OP_ZERO,
      V3D_STENCIL_OP_ZERO, V3D_STENCIL_OP_ZERO);
   v3d_cl_cfg_bits(&instr,
      front_prims,
      back_prims,
      cwise_is_front,
      false,                     /* depth offset */
      false,                     /* aa_lines */
      V3D_MS_1X,                 /* rast_oversample */
      false,                     /* cov_pipe */
      V3D_COV_UPDATE_NONZERO,    /* cov_update */
      false,                     /* wireframe_tris */
      V3D_COMPARE_FUNC_ALWAYS,   /* depth_test - zfunc */
      change_depth,              /* depth_update - enzu   */
      rs->ez.cfg_bits_ez,
      rs->ez.cfg_bits_ez_update,
      change_stencil,            /* stencil */
      false,                     /* blend */
      V3D_WIREFRAME_MODE_LINES,
      false);                    /* d3d_prov_vtx */

   khrn_render_state_set_add(&dirty->cfg, rs);       /* Clear and render probably use different configs */

   /* Emit a stencil config record */
   if (change_stencil)
   {
      v3d_cl_stencil_cfg(&instr,
         stencil_value,                      /* ref value */
         0xffU,                              /* test mask */
         V3D_COMPARE_FUNC_ALWAYS,            /* test function */
         V3D_STENCIL_OP_ZERO,                /* stencil fail op = don't care */
         V3D_STENCIL_OP_ZERO,                /* depth fail op = don't care */
         V3D_STENCIL_OP_REPLACE,             /* pass op = replace */
         true,                               /* back config */
         true,                               /* front config */
         stencil_mask->front & 0xffU); /* stencil write mask */

      khrn_render_state_set_add(&dirty->stencil, rs);
   }

   if (change_color)
      set_color_write_masks(fb, color_write, masks);
   else
      disable_color_write_masks(masks);
   v3d_cl_color_wmasks_indirect(&instr, (const V3D_CL_COLOR_WMASKS_T *)&masks);

   khrn_render_state_set_add(&dirty->color_write, rs);

   v3d_cl_viewport_offset(&instr, 0, 0);

   //TODO: other miscellaneous pieces of state

   return true;
}

uint32_t *glxx_draw_alternate_install_nvshader(KHRN_FMEM_T* fmem, uint32_t shaderSize, uint32_t *shaderCode)
{
   uint32_t *fshader_ptr = khrn_fmem_data(fmem, shaderSize, V3D_SHADREC_ALIGN);
   if (fshader_ptr)
      memcpy(fshader_ptr, shaderCode, shaderSize);
   return fshader_ptr;
}

/* Pack the colours into a shader uniform block, skipping uniform 1, which is used for configuration. */
static void pack_colors_in_uniforms(const uint32_t color_value[4],
   uint32_t *uniform, unsigned uniform_count)
{
   switch (uniform_count)
   {
   case 3:
      uniform[2] = gfx_pack_1616(gfx_bits_to_float16(color_value[2]), gfx_bits_to_float16(color_value[3]));
      uniform[0] = gfx_pack_1616(gfx_bits_to_float16(color_value[0]), gfx_bits_to_float16(color_value[1]));
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
   0xbb800000, 0x3c403186, // nop;           ldunif
   0xb682d000, 0x3c003188, // mov tlbu, r5
   0xbb800000, 0x3c403186, // nop;           ldunif
   0xb682d000, 0x3c203187, // mov tlb, r5;   thrsw
   0xbb800000, 0x3c003186, // nop
   0xbb800000, 0x3c003186, // nop
};

static uint32_t clear_shader_4[] =
{
   0xbb800000, 0x3c403186, // nop; ldunif
   0xb682d000, 0x3c003188, // mov tlbu, r5
   0xbb800000, 0x3c403186, // nop; ldunif
   0xb682d000, 0x3c403187, // mov tlb, r5; ldunif
   0xb682d000, 0x3c403187, // mov tlb, r5; ldunif
   0xb682d000, 0x3c203187, // mov tlb, r5; thrsw
   0xbb800000, 0x3c003186, // nop
   0xbb800000, 0x3c003186, // nop
};

static uint32_t clear_shader_no_color[] =
{
   0x00000000, 0x030010c6, // [0x00000000] mov rf3, 0
   0x00000000, 0x03003206, // [0x00000008] mov tlbu, 0
   0xb68360c0, 0x3c203187, // [0x00000010] mov tlb, rf3 ; thrsw
   0x00000000, 0x030031c6, // [0x00000018] mov tlb, 0
   0x00000000, 0x030031c6, // [0x00000020] mov tlb, 0
};

static uint32_t clear_shader_no_color_point[] =
{
   0xbb800000, 0x3d003186, // [0x00000000] nop; ldvary
   0xbb800000, 0x3d003186, // [0x00000008] nop; ldvary
   0xbb800000, 0x3d003186, // [0x00000010] nop; ldvary
   0x00000000, 0x030010c6, // [0x00000018] mov rf3, 0
   0x00000000, 0x03003206, // [0x00000020] mov tlbu, 0
   0xb68360c0, 0x3c203187, // [0x00000028] mov tlb, rf3 ; thrsw
   0x00000000, 0x030031c6, // [0x00000030] mov tlb, 0
   0x00000000, 0x030031c6, // [0x00000038] mov tlb, 0
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
   uint32_t          z;
   uint32_t          *shaderCode;
   uint32_t          shaderSize;
   uint32_t          uniformCount;
   unsigned int      rt = 0;
   bool              front_prims     = true;     /* enfwd */
   bool              back_prims      = true;     /* enrev */
   bool              cwise_is_front  = false;    /* cwise */
   uint32_t          *funif_ptr;
   v3d_addr_t        nv_shadrec_hw;
   uint32_t          *vdata_ptr;
   uint32_t          *fshader_ptr;
   bool              does_z_writes = false;
   const GLXX_HW_FRAMEBUFFER_T *hw_fb = &rs->installed_fb;
   const GLXX_FRAMEBUFFER_T *fb = state->bound_draw_framebuffer;

   assert(clear->color || clear->depth || clear->stencil);

   //TODO: MRT
   if (clear->color)
   {
      rt = clear->color_buffer_mask;      // TODO mask really abused as clear index for now
      assert(glxx_fb_is_valid_draw_buf(state->bound_draw_framebuffer,
               GLXX_COLOR0_ATT + rt));
      assert(hw_fb->color[rt].image != NULL || hw_fb->color_ms[rt].image != NULL);
      gfx_bufstate_rw(&rs->ms_color_buffer_state[rt]);
      gfx_bufstate_rw(&rs->color_buffer_state[rt]);
   }

   if (clear->depth)
      gfx_bufstate_rw(&rs->depth_buffer_state);

   if (clear->stencil)
   {
      gfx_bufstate_rw(&rs->stencil_buffer_state);
   }

   uint8_t cfg;
   unsigned cfg_unif_pos;

   if (clear->color)
   {
      const KHRN_IMAGE_PLANE_T *plane = hw_fb->color_ms[rt].image ?
                                        &hw_fb->color_ms[rt]: &hw_fb->color[rt];
      int tlb_write_type = get_tlb_write_type_from_lfmt(khrn_image_plane_lfmt(plane));

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

   funif_ptr = khrn_fmem_data(&rs->fmem, uniformCount * 4, V3D_SHADREC_ALIGN);
   if (funif_ptr == NULL)
      return false;

   if (clear->color)
   {
      /* fill in uniforms, skipping uniform 1 which is used as a configuration byte */
      pack_colors_in_uniforms(clear->color_value, funif_ptr, uniformCount);
   }

   /* set up the config uniform; unused config entries must be all 1s */
   funif_ptr[cfg_unif_pos] = 0xffffff00 | cfg;

   z = gfx_float_to_bits(clear->depth_value);

   /* Use x, y, z to create vertex data to be used in the clear */
   vdata_ptr = draw_rect_vertex_data(&rs->fmem, x, xmax, y, ymax, z);
   if (vdata_ptr == NULL)
      return false;

   /* Install the shader and shader record. */
   fshader_ptr = glxx_draw_alternate_install_nvshader(&rs->fmem, shaderSize, shaderCode);
   if (fshader_ptr == NULL)
      return false;

   nv_shadrec_hw = create_nv_shader_record(&rs->fmem, fshader_ptr, funif_ptr, vdata_ptr,
      does_z_writes, V3D_THREADING_T1);
   if (!nv_shadrec_hw)
      return false;

   /* Install the control list. */
   if (glxx_draw_alternate_cle(rs, &state->dirty, fb,
      clear->color, &state->color_write, clear->depth,
      clear->stencil, clear->stencil_value, &state->stencil_mask,
      x, y, xmax, ymax,
      front_prims, back_prims, cwise_is_front) == false)
   {
      return false;
   }

   uint8_t *instr = khrn_fmem_cle(&rs->fmem, V3D_CL_GL_SHADER_SIZE + V3D_CL_VERTEX_ARRAY_PRIMS_SIZE);
   if (!instr) return false;

   v3d_cl_nv_shader(&instr, 2, nv_shadrec_hw);
   v3d_cl_vertex_array_prims(&instr, V3D_PRIM_MODE_TRI_FAN, 4, 0);

#ifdef SIMPENROSE_WRITE_LOG
   // Increment batch count
   log_batchcount++;
#endif

   return true;
}

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
   uint32_t* fshader_ptr = glxx_draw_alternate_install_nvshader(fmem, sizeof(clear_shader_no_color_point), clear_shader_no_color_point);
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
   v3d_cl_prim_list_format(&instr, V3D_PRIM_TYPE_POINT, false, false);

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

   V3D_CL_COLOR_WMASKS_T color_wmasks;
   disable_color_write_masks(color_wmasks);
   v3d_cl_color_wmasks_indirect(&instr, (const V3D_CL_COLOR_WMASKS_T *)&color_wmasks);

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
   v3d_addr_t scratch_addr = khrn_fmem_lock_and_sync(fmem, scratch_mem, 0, GMEM_SYNC_CORE_READ | GMEM_SYNC_CORE_WRITE);
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
      v3d_cl_occlusion_query_counter_enable(&instr, scratch_addr + V3D_QUERY_COUNTER_FIRST_CORE_CACHE_LINE_ALIGN * i);
      v3d_cl_nv_shader(&instr, 2, shader_addr);
      render_dummy_point(&instr);
   }

   assert((instr - *instr_ptr) == glxx_workaround_gfxh_1320_size());
   *instr_ptr = instr;
   return true;
}

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

   // if depth writes are enabled, then we must test the following conditions to
   // ensure that performing a z-prepass is valid
   if (state->caps.depth_test && state->depth_mask)
   {
      // disable z-only if using blending (currently a conservative test)
      if (state->blend.enable)
      {
         goto disallowed;
      }

      // disable z-prepass if using colour write masks
      // timh-todo: should only care if this changes
      if (!(state->color_write.r && state->color_write.g && state->color_write.b && state->color_write.a))
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
