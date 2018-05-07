/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "gl_public_api.h"

#include "../common/khrn_int_common.h"
#include "../common/khrn_int_util.h"
#include "../common/khrn_render_state.h"
#include "../common/khrn_resource.h"

#include "glxx_server.h"
#include "glxx_server_internal.h"
#include "glxx_server_pipeline.h"

#include "glxx_hw.h"
#include "glxx_shared.h"
#include "glxx_texture.h"
#include "glxx_server_texture.h"
#include "glxx_buffer.h"
#include "glxx_renderbuffer.h"
#include "glxx_framebuffer.h"
#include "glxx_translate.h"
#include "../gl20/gl20_program.h"
#include "../gl20/gl20_shader.h"

#include "../gl11/gl11_int_config.h"
#include "../gl11/gl11_shadercache.h"

#include "../egl/egl_surface.h"

#include <string.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "vcos.h"
#include "libs/platform/gmem.h"

#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"
#include "libs/util/profile/profile.h"
#include "libs/util/demand.h"

static void glxx_update_viewport_internal(GLXX_SERVER_STATE_T *state);

#if WANT_PROFILING
profile_timer* glxx_server_locked_timer;
uint32_t glxx_server_lock_time;
#endif

VCOS_MUTEX_T glxx_lock;
static VCOS_ONCE_T glxx_once = VCOS_ONCE_INIT;
static void init_glxx(void)
{
   bool ok;

   ok = (vcos_mutex_create(&glxx_lock, "GLXX") == VCOS_SUCCESS);
   demand_msg(ok, "Fatal: unable to create glxx lock");

   ok = glxx_queries_updates_lock_init();
   demand_msg(ok, "Fatal: unable to create glxx queries updates lock");
}

static void queries_of_type_init(struct glxx_queries_of_type *queries,
      enum glxx_query_type type)
{
   queries->active = NULL;
   khrn_timeline_init(&queries->timeline);
}

static void queries_of_type_deinit(struct glxx_queries_of_type *queries)
{
   KHRN_MEM_ASSIGN(queries->active, NULL);
   khrn_timeline_deinit(&queries->timeline);
}

static bool queries_init(struct glxx_queries *queries)
{
   queries->next_name = 1;

   if (!khrn_map_init(&queries->objects, 256))
      return false;

   for (unsigned i = 0; i < GLXX_Q_COUNT; i++)
      queries_of_type_init(queries->queries + i, i);
   return true;
}

static void queries_deinit(struct glxx_queries *queries)
{
   queries->next_name = 1;
   khrn_map_term(&queries->objects);

   for (unsigned i = 0; i < GLXX_Q_COUNT; i++)
      queries_of_type_deinit(queries->queries + i);
}

static void init_blend_cfg(glxx_blend_cfg *cfg)
{
   cfg->color_eqn  = GLXX_BLEND_EQN_ADD;
   cfg->alpha_eqn  = GLXX_BLEND_EQN_ADD;
   cfg->src_rgb    = V3D_BLEND_MUL_ONE;
   cfg->src_alpha  = V3D_BLEND_MUL_ONE;
   cfg->dst_rgb    = V3D_BLEND_MUL_ZERO;
   cfg->dst_alpha  = V3D_BLEND_MUL_ZERO;
}

/*
   initialises common portions of the GLXX_SERVER_STATE_T state structure
   this function is called by the OpenGL ES version specific functions
   gl11_server_state_init and gl20_server_state_init
*/

bool glxx_server_state_init(GLXX_SERVER_STATE_T *state, GLXX_SHARED_T *shared)
{
   bool ok = false;
   GLXX_FRAMEBUFFER_T *default_framebuffer[2];

   if (vcos_once(&glxx_once, init_glxx))
      return false;

   KHRN_MEM_ASSIGN(state->shared, shared);

   /* create default textures */
   if (!glxx_textures_create(&state->default_textures))
      goto end;

   /* now the rest of the structure */
   state->active_texture = 0;
   state->error = GL_NO_ERROR;

   // Note ELEMENT_ARRAY buffer initialised by glxx_vao_create_default
   for (int i = 0; i < GLXX_BUFTGT_CTX_COUNT; ++i)
      KHRN_MEM_ASSIGN(state->bound_buffer[i].obj, NULL);

   for (int i = 0; i < GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS; ++i)
   {
      KHRN_MEM_ASSIGN(state->uniform_block.binding_points[i].buffer.obj, NULL);
      state->uniform_block.binding_points[i].offset = 0;
      state->uniform_block.binding_points[i].size = 0;
   }

   for (int i = 0; i < GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS; ++i)
   {
      KHRN_MEM_ASSIGN(state->ssbo.binding_points[i].buffer.obj, NULL);
      state->ssbo.binding_points[i].offset = 0;
      state->ssbo.binding_points[i].size = 0;
   }

   // No sampler objects by default
   for (int i = 0; i < GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS; i++)
      KHRN_MEM_ASSIGN(state->bound_sampler[i], NULL);

   // Indexed buffer targets (transform feedback, uniform buffers) are initialized elsewhere

   for (int i = 0; i < GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS; i++)
   {
      glxx_textures_assign(&state->bound_texture[i],
            &state->default_textures);
   }

   state->clear.color_value[0] = 0.0f;
   state->clear.color_value[1] = 0.0f;
   state->clear.color_value[2] = 0.0f;
   state->clear.color_value[3] = 0.0f;

   state->clear.depth_value = 1.0f;

   state->clear.stencil_value = 0;

   state->cull_mode = GL_BACK;
   state->depth_func = GL_LESS;
   state->depth_mask = GL_TRUE;

   state->sample_shading_fraction  = 0.0f;

   state->caps.cull_face            = false;
   state->caps.polygon_offset_fill  = false;
   state->caps.scissor_test         = false;
   state->caps.dither               = true;
   state->caps.stencil_test         = false;
   state->caps.depth_test           = false;
   state->caps.sample_shading       = false;

   // ES 3.0 only
   state->caps.primitive_restart   = false;
   state->caps.rasterizer_discard  = false;

   // ES 1.x only
   state->caps.multisample         = true;

   state->gl11.current_palette_matrix = 0;

   state->front_face = GL_CCW;

   state->hints.generate_mipmap = GL_DONT_CARE;
   state->hints.fshader_derivative = GL_DONT_CARE;

   state->line_width = 1.0f;
#if V3D_VER_AT_LEAST(4,1,34,0)
   state->num_patch_vertices = 3u;
#endif

   static const float pbb[8] = { -1, -1, -1, 1, 1, 1, 1, 1 };
   for (int i=0; i<4; i++) {
      state->primitive_bounding_box.min[i] = pbb[i];
      state->primitive_bounding_box.max[i] = pbb[4+i];
   }

#if V3D_VER_AT_LEAST(4,1,34,0)
   state->polygon_offset.limit = 0.0f;
#endif
   state->polygon_offset.factor = 0.0f;
   state->polygon_offset.units = 0.0f;

   state->sample_coverage.enable = false;
   state->sample_coverage.invert = false;
   state->sample_coverage.value = 1.0f;

   state->sample_mask.enable = false;
   for (int i=0; i<GLXX_CONFIG_MAX_SAMPLE_WORDS; i++)
      state->sample_mask.mask[i] = ~0u;

   state->scissor.x = 0;
   state->scissor.y = 0;
   state->scissor.width = 0;
   state->scissor.height = 0;

   state->stencil_func.front.func = GL_ALWAYS;
   state->stencil_func.front.ref = 0;
   state->stencil_func.front.mask = 0xffffffff;
   state->stencil_mask.front = 0xffffffff;

   state->stencil_op.front.fail = GL_KEEP;
   state->stencil_op.front.zfail = GL_KEEP;
   state->stencil_op.front.zpass = GL_KEEP;

   state->stencil_func.back.func = GL_ALWAYS;
   state->stencil_func.back.ref = 0;
   state->stencil_func.back.mask = 0xffffffff;
   state->stencil_mask.back = 0xffffffff;

   state->stencil_op.back.fail = GL_KEEP;
   state->stencil_op.back.zfail = GL_KEEP;
   state->stencil_op.back.zpass = GL_KEEP;

   state->statebits.backend = GLSL_SAMPLE_MS;

#if V3D_VER_AT_LEAST(4,1,34,0)
   state->blend.rt_enables = 0;
   for (unsigned i = 0; i != V3D_MAX_RENDER_TARGETS; ++i)
      init_blend_cfg(&state->blend.rt_cfgs[i]);
#else
   state->blend.enable = false;
   init_blend_cfg(&state->blend.cfg);
#endif
   state->blend.advanced_coherent = true;

   state->blend_color[0] = 0.0f;
   state->blend_color[1] = 0.0f;
   state->blend_color[2] = 0.0f;
   state->blend_color[3] = 0.0f;

   state->color_write = gfx_mask(V3D_MAX_RENDER_TARGETS * 4);

   state->viewport.x = 0;
   state->viewport.y = 0;
   state->viewport.width = 0;
   state->viewport.height = 0;

   state->depth_range.z_near = 0.0f;
   state->depth_range.z_far = 1.0f;

   glxx_update_viewport_internal(state);

   if (!khrn_map_init(&state->framebuffers, 256))
      goto end;

   default_framebuffer[GLXX_DRAW_SURFACE] = glxx_server_state_get_framebuffer(state, 0, true);
   if (default_framebuffer[GLXX_DRAW_SURFACE] == NULL)
      goto end;

   // Default framebuffer for read surface is a bit special. We put it to framebuffer
   // name map, so that it gets deleted when map is destroyed. However, in the map it
   // has a fake name. In the object we pretend the name to be 0. Value 0 is used
   // in the driver to identify default framebuffers.
   default_framebuffer[GLXX_READ_SURFACE] = glxx_server_state_get_framebuffer(state, UINT32_MAX, true);
   if (default_framebuffer[GLXX_READ_SURFACE] == NULL)
      goto end;

   default_framebuffer[GLXX_READ_SURFACE]->name = 0;

   KHRN_MEM_ASSIGN(state->default_framebuffer[GLXX_READ_SURFACE], default_framebuffer[GLXX_READ_SURFACE]);
   KHRN_MEM_ASSIGN(state->default_framebuffer[GLXX_DRAW_SURFACE], default_framebuffer[GLXX_DRAW_SURFACE]);

   state->next_framebuffer = 1;

   state->current_render_state = NULL;    /* = No render state */
   memset(&state->dirty, 0xff, sizeof(state->dirty));

   state->made_current = GL_FALSE;

   assert(state->bound_renderbuffer == NULL);
   KHRN_MEM_ASSIGN(state->bound_draw_framebuffer, state->default_framebuffer[GLXX_DRAW_SURFACE]);
   KHRN_MEM_ASSIGN(state->bound_read_framebuffer, state->default_framebuffer[GLXX_READ_SURFACE]);

   memset(&(state->pixel_store_state), 0, sizeof(state->pixel_store_state));
   state->pixel_store_state.pack.alignment = 4;
   state->pixel_store_state.unpack.alignment = 4;

   if (!queries_init(&state->queries))
      goto end;

   state->transform_feedback.default_tf = glxx_tf_create(0);
   if (state->transform_feedback.default_tf == NULL)
      goto end;

   state->transform_feedback.next = 1;
   KHRN_MEM_ASSIGN(state->transform_feedback.bound, state->transform_feedback.default_tf);
   if (!khrn_map_init(&state->transform_feedback.objects, 256))
      goto end;

   if (!glxx_vao_initialise(state))
      goto end;

   if (!glxx_pipeline_state_initialise(state))
      goto end;

   // Generic attribute default values.
   for (unsigned i = 0; i < V3D_MAX_ATTR_ARRAYS; i++)
   {
      GLXX_GENERIC_ATTRIBUTE_T* ga = &state->generic_attrib[i];
      ga->f[0] = 0.0f;
      ga->f[1] = 0.0f;
      ga->f[2] = 0.0f;
      ga->f[3] = 1.0f;
      ga->type = V3D_ATTR_TYPE_FLOAT;
      ga->is_signed = false;
   }

   state->fill_mode = GL_FILL_BRCM;

   state->provoking_vtx = GL_LAST_VERTEX_CONVENTION_BRCM;

   state->fences.fence = khrn_fence_create();
   if (!state->fences.fence)
      goto end;
   state->fences.fence_to_depend_on = khrn_fence_create();
   if (!state->fences.fence_to_depend_on)
      goto end;

   state->debug.draw_id = 0;

   if (glxx_init_khr_debug_state(&state->khr_debug) == false)
      goto end;

   for (int i = 0; i < GLXX_CONFIG_MAX_IMAGE_UNITS; ++i)
      glxx_image_unit_init_default(&state->image_unit[i]);

   ok = true;
end:
   if (!ok)
   {
      KHRN_MEM_ASSIGN(state->shared, NULL);
      khrn_fence_refdec(state->fences.fence);
      khrn_fence_refdec(state->fences.fence_to_depend_on);

      /* FIXME: And the rest ... */
   }

   return ok;
}

void glxx_server_state_flush(GLXX_SERVER_STATE_T *state, bool wait)
{
   if (state->fences.fence == NULL)
      return;

   khrn_fence_flush(state->fences.fence);
   if (wait)
   {
      /* Can't use khrn_fence_wait(state->fences.fence, V3D_SCHED_DEPS_FINALISED);
       * as it doesn't wait for deferred_free_callback()
       */
      v3d_scheduler_wait_all();

#if GMEM_FINISH
      gmem_finish();
#endif
   }
#ifndef NDEBUG
   khrn_fence_allow_change(state->fences.fence);
#endif
}


void glxx_server_attach_surfaces(GLXX_SERVER_STATE_T *state,
      EGL_SURFACE_T *read, EGL_SURFACE_T *draw)
{
   GLXX_FRAMEBUFFER_T *fb_read, *fb_draw;

   if (read && draw)
   {
      fb_read = state->default_framebuffer[GLXX_READ_SURFACE];
      fb_draw = state->default_framebuffer[GLXX_DRAW_SURFACE];

      glxx_fb_attach_egl_surface(fb_read, read);
      glxx_fb_attach_egl_surface(fb_draw, draw);
   }
   else
   {
      glxx_server_detach_surfaces(state);
   }

   if (!state->made_current)
   {
      unsigned width, height;
      if (draw)
      {
         khrn_image *img = egl_surface_get_back_buffer(draw);

         width = khrn_image_get_width(img);
         height = khrn_image_get_height(img);

         assert(width <= GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE &&
            height <= GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE);
      }
      else
      {
         width = height = 0;
      }

      state->scissor.x = state->scissor.y = 0;
      state->viewport.x = state->viewport.y = 0;
      state->scissor.width = state->viewport.width = width;
      state->scissor.height = state->viewport.height = height;
      glxx_update_viewport_internal(state);
      state->made_current = GL_TRUE;
   }
}

void glxx_server_detach_surfaces(GLXX_SERVER_STATE_T *state)
{
   GLXX_FRAMEBUFFER_T *fb_read, *fb_draw;

   fb_read = state->default_framebuffer[GLXX_READ_SURFACE];
   fb_draw = state->default_framebuffer[GLXX_DRAW_SURFACE];

   glxx_fb_detach(fb_read);
   glxx_fb_detach(fb_draw);
}

void glxx_server_state_destroy(GLXX_SERVER_STATE_T *state)
{
   int i;

   if (!state)
      return;

   if (!IS_GL_11(state))
   {
      if (state->current_program != NULL)
      {
         assert(gl20_is_program(state->current_program));
         gl20_program_release(state->current_program);
         gl20_server_try_delete_program(state->shared, state->current_program);
      }
      state->current_program = NULL;
   }

   glxx_destroy_khr_debug_state(&state->khr_debug);

   KHRN_MEM_ASSIGN(state->bound_renderbuffer, NULL);
   KHRN_MEM_ASSIGN(state->bound_draw_framebuffer, NULL);
   KHRN_MEM_ASSIGN(state->bound_read_framebuffer, NULL);

   for (i = 0; i < GLXX_BUFTGT_CTX_COUNT; ++i)
      KHRN_MEM_ASSIGN(state->bound_buffer[i].obj, NULL);

   for (i = 0; i < GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS; ++i)
      KHRN_MEM_ASSIGN(state->uniform_block.binding_points[i].buffer.obj, NULL);

   for (i = 0; i < GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS; ++i)
      KHRN_MEM_ASSIGN(state->ssbo.binding_points[i].buffer.obj, NULL);

   for (i = 0; i < GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS; i++)
      glxx_textures_release(&state->bound_texture[i]);

   glxx_pipeline_state_term(state);

   KHRN_MEM_ASSIGN(state->shared, NULL);
   glxx_textures_release(&state->default_textures);

   /* remove default fb from maps*/
   glxx_server_state_delete_framebuffer(state, 0);
   glxx_server_state_delete_framebuffer(state, UINT32_MAX);
   KHRN_MEM_ASSIGN(state->default_framebuffer[GLXX_DRAW_SURFACE], NULL);
   KHRN_MEM_ASSIGN(state->default_framebuffer[GLXX_READ_SURFACE], NULL);

   if (IS_GL_11(state))
   {
      gl11_server_state_destroy(state);
   }

   KHRN_MEM_ASSIGN(state->transform_feedback.default_tf, NULL);
   KHRN_MEM_ASSIGN(state->transform_feedback.bound, NULL);
   khrn_map_term(&state->transform_feedback.objects);

   glxx_vao_uninitialise(state);

   queries_deinit(&state->queries);

   khrn_map_term(&state->framebuffers);

   for (i = 0; i < GLXX_CONFIG_MAX_IMAGE_UNITS; ++i)
      glxx_image_unit_deinit(&state->image_unit[i]);

   // We want to make sure that any external buffers modified by this context
   // get updated and released.
   glxx_server_state_flush(state, true);

   khrn_fence_refdec(state->fences.fence);
   khrn_fence_refdec(state->fences.fence_to_depend_on);
}

static void set_blend_func(GLXX_SERVER_STATE_T *state, glxx_blend_cfg *cfg,
   v3d_blend_mul_t sc, v3d_blend_mul_t dc, v3d_blend_mul_t sa, v3d_blend_mul_t da)
{
   if (cfg->src_rgb != sc)
   {
      cfg->src_rgb = sc;
      state->dirty.blend_cfg = KHRN_RENDER_STATE_SET_ALL;
   }
   if (cfg->dst_rgb != dc)
   {
      cfg->dst_rgb = dc;
      state->dirty.blend_cfg = KHRN_RENDER_STATE_SET_ALL;
   }
   if (cfg->src_alpha != sa)
   {
      cfg->src_alpha = sa;
      state->dirty.blend_cfg = KHRN_RENDER_STATE_SET_ALL;
   }
   if (cfg->dst_alpha != da)
   {
      cfg->dst_alpha = da;
      state->dirty.blend_cfg = KHRN_RENDER_STATE_SET_ALL;
   }
}

static void set_all_blend_funcs(GLXX_SERVER_STATE_T *state,
   v3d_blend_mul_t sc, v3d_blend_mul_t dc, v3d_blend_mul_t sa, v3d_blend_mul_t da)
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   for (unsigned i = 0; i != V3D_MAX_RENDER_TARGETS; ++i)
      set_blend_func(state, &state->blend.rt_cfgs[i], sc, dc, sa, da);
#else
   set_blend_func(state, &state->blend.cfg, sc, dc, sa, da);
#endif
}

/* Check that 'func' is allowed for es11 */
static inline bool validate_func_es11(GLenum func, bool is_src)
{
   switch(func)
   {
   case GL_ZERO:
   case GL_ONE:
   case GL_SRC_ALPHA:
   case GL_ONE_MINUS_SRC_ALPHA:
   case GL_DST_ALPHA:
   case GL_ONE_MINUS_DST_ALPHA:
      return true;
   /* In es11 these modes are only allowed for dst */
   case GL_SRC_COLOR:
   case GL_ONE_MINUS_SRC_COLOR:
      return !is_src;
   /* These are only allowed for src */
   case GL_SRC_ALPHA_SATURATE:
   case GL_DST_COLOR:
   case GL_ONE_MINUS_DST_COLOR:
      return is_src;
   default:
      return false;
   }

}

bool glxx_advanced_blend_eqn_set(const GLXX_SERVER_STATE_T *state) {
#if V3D_VER_AT_LEAST(4,1,34,0)
   return state->blend.rt_cfgs[0].color_eqn & GLXX_ADV_BLEND_EQN_BIT;
#else
   return state->blend.cfg.color_eqn & GLXX_ADV_BLEND_EQN_BIT;
#endif
}

// Returns encoding for blend equation for backend
// 0 means it is not an advanced blend or blending is disabled
uint32_t glxx_advanced_blend_eqn(const GLXX_SERVER_STATE_T *state)
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   if ((state->blend.rt_enables & 1) && state->blend.rt_cfgs[0].color_eqn >= GLXX_ADV_BLEND_EQN_BIT)
      return state->blend.rt_cfgs[0].color_eqn - GLXX_ADV_BLEND_EQN_BIT;
#else
   if (state->blend.enable && state->blend.cfg.color_eqn >= GLXX_ADV_BLEND_EQN_BIT)
      return state->blend.cfg.color_eqn - GLXX_ADV_BLEND_EQN_BIT;
#endif

   return 0;
}

GL_API void GL_APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state) return;

   if (IS_GL_11(state))
   {
      if (!validate_func_es11(sfactor, true) ||
          !validate_func_es11(dfactor, false)   )
      {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         goto end;
      }
   }

   v3d_blend_mul_t s = translate_blend_func(sfactor);
   v3d_blend_mul_t d = translate_blend_func(dfactor);

   if ((s == V3D_BLEND_MUL_INVALID) || (d == V3D_BLEND_MUL_INVALID))
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else
      set_all_blend_funcs(state, s, d, s, d);

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if(!state) return;

   v3d_blend_mul_t sc = translate_blend_func(srcRGB);
   v3d_blend_mul_t dc = translate_blend_func(dstRGB);
   v3d_blend_mul_t sa = translate_blend_func(srcAlpha);
   v3d_blend_mul_t da = translate_blend_func(dstAlpha);

   if ((sc == V3D_BLEND_MUL_INVALID) || (dc == V3D_BLEND_MUL_INVALID) ||
         (sa == V3D_BLEND_MUL_INVALID) || (da == V3D_BLEND_MUL_INVALID))
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else
      set_all_blend_funcs(state, sc, dc, sa, da);

   glxx_unlock_server_state();
}

#if V3D_VER_AT_LEAST(4,1,34,0)

static void blend_func_i(GLuint buf, GLenum src, GLenum dst)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (buf >= V3D_MAX_RENDER_TARGETS)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else
   {
      v3d_blend_mul_t s = translate_blend_func(src);
      v3d_blend_mul_t d = translate_blend_func(dst);

      if ((s == V3D_BLEND_MUL_INVALID) || (d == V3D_BLEND_MUL_INVALID))
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      else
         set_blend_func(state, &state->blend.rt_cfgs[buf], s, d, s, d);
   }

   glxx_unlock_server_state();
}

static void blend_func_separate_i(GLuint buf,
   GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (buf >= V3D_MAX_RENDER_TARGETS)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else
   {
      v3d_blend_mul_t sc = translate_blend_func(srcRGB);
      v3d_blend_mul_t dc = translate_blend_func(dstRGB);
      v3d_blend_mul_t sa = translate_blend_func(srcAlpha);
      v3d_blend_mul_t da = translate_blend_func(dstAlpha);

      if ((sc == V3D_BLEND_MUL_INVALID) || (dc == V3D_BLEND_MUL_INVALID) ||
            (sa == V3D_BLEND_MUL_INVALID) || (da == V3D_BLEND_MUL_INVALID))
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      else
         set_blend_func(state, &state->blend.rt_cfgs[buf], sc, dc, sa, da);
   }

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glBlendFunciEXT(GLuint buf, GLenum src, GLenum dst)
{
   blend_func_i(buf, src, dst);
}

GL_API void GL_APIENTRY glBlendFuncSeparateiEXT(GLuint buf,
   GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
   blend_func_separate_i(buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

GL_API void GL_APIENTRY glBlendFunciOES(GLuint buf, GLenum src, GLenum dst)
{
   blend_func_i(buf, src, dst);
}

GL_API void GL_APIENTRY glBlendFuncSeparateiOES(GLuint buf,
   GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
   blend_func_separate_i(buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

#endif

#if KHRN_GLES32_DRIVER

GL_API void GL_APIENTRY glBlendFunci(GLuint buf, GLenum src, GLenum dst)
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   blend_func_i(buf, src, dst);
#endif
}

GL_API void GL_APIENTRY glBlendFuncSeparatei(GLuint buf,
   GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   blend_func_separate_i(buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
#endif
}

#endif

GL_API void GL_APIENTRY glClear(GLbitfield mask)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   GLenum error = GL_NO_ERROR;

   if (!state) return;

   if (mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (!glxx_fb_is_complete(state->bound_draw_framebuffer, &state->fences))
   {
      error = GL_INVALID_FRAMEBUFFER_OPERATION;
      goto end;
   }

   if (mask == 0)
   {
      goto end;
   }

   GLXX_CLEAR_T clear;
   clear.color_buffer_mask = (mask & GL_COLOR_BUFFER_BIT) ? gfx_mask(V3D_MAX_RENDER_TARGETS) : 0;
   clear.depth = !!(mask & GL_DEPTH_BUFFER_BIT);
   clear.stencil = !!(mask & GL_STENCIL_BUFFER_BIT);
   clear.color_value[0] = gfx_float_to_bits(state->clear.color_value[0]);
   clear.color_value[1] = gfx_float_to_bits(state->clear.color_value[1]);
   clear.color_value[2] = gfx_float_to_bits(state->clear.color_value[2]);
   clear.color_value[3] = gfx_float_to_bits(state->clear.color_value[3]);
   clear.depth_value = state->clear.depth_value;
   clear.stencil_value = state->clear.stencil_value;

   if (!glxx_hw_clear(state, &clear))
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }
end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

static bool clear_buffer_drawbuffer_valid(GLenum buffer, int drawbuffer) {
   switch (buffer)
   {
   case GL_COLOR:
      if ((drawbuffer < 0) || (drawbuffer >= V3D_MAX_RENDER_TARGETS))
         return false;
      break;
   case GL_DEPTH:
   case GL_STENCIL:
   case GL_DEPTH_STENCIL:
      if (drawbuffer != 0) return false;
      break;
   default:
      unreachable();
      return false;
   }

   return true;
}

GL_API void GL_APIENTRY glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (buffer != GL_DEPTH_STENCIL)
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto unlock_out;
   }
   if (!clear_buffer_drawbuffer_valid(buffer, drawbuffer))
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto unlock_out;
   }

   if (!glxx_fb_is_complete(state->bound_draw_framebuffer, &state->fences))
   {
      glxx_server_state_set_error(state, GL_INVALID_FRAMEBUFFER_OPERATION);
      goto unlock_out;
   }

   GLXX_CLEAR_T clear;
   clear.color_buffer_mask = 0;
   clear.depth          = true;
   clear.stencil        = true;;
   clear.depth_value    = depth;
   clear.stencil_value  = stencil;

   if (!glxx_hw_clear(state, &clear))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

unlock_out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (buffer != GL_COLOR && buffer != GL_DEPTH) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (!clear_buffer_drawbuffer_valid(buffer, drawbuffer)) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   GLXX_CLEAR_T clear;
   clear.color_buffer_mask = (buffer == GL_COLOR) ? (1u << drawbuffer) : 0;
   clear.depth             = (buffer == GL_DEPTH);
   clear.stencil           = false;
   clear.color_value[0]    = gfx_float_to_bits(value[0]);
   clear.color_value[1]    = gfx_float_to_bits(value[1]);
   clear.color_value[2]    = gfx_float_to_bits(value[2]);
   clear.color_value[3]    = gfx_float_to_bits(value[3]);
   clear.depth_value       = (buffer == GL_DEPTH ? value[0] : 0);
   clear.stencil_value     = 0;

   if (!glxx_hw_clear(state, &clear))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (buffer != GL_COLOR && buffer != GL_STENCIL) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (!clear_buffer_drawbuffer_valid(buffer, drawbuffer)) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   GLXX_CLEAR_T clear;
   clear.color_buffer_mask = (buffer == GL_COLOR) ? (1u << drawbuffer) : 0;
   clear.depth             = false;
   clear.stencil           = (buffer == GL_STENCIL);
   clear.color_value[0]    = value[0];
   clear.color_value[1]    = value[1];
   clear.color_value[2]    = value[2];
   clear.color_value[3]    = value[3];
   clear.depth_value       = 0.0f;
   clear.stencil_value     = value[0];

   if (!glxx_hw_clear(state, &clear))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (buffer != GL_COLOR) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (!clear_buffer_drawbuffer_valid(buffer, drawbuffer)) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   GLXX_CLEAR_T clear;
   clear.color_buffer_mask = (1u << drawbuffer);
   clear.depth             = false;
   clear.stencil           = false;
   clear.color_value[0]    = value[0];
   clear.color_value[1]    = value[1];
   clear.color_value[2]    = value[2];
   clear.color_value[3]    = value[3];
   clear.depth_value       = 0.0f;
   clear.stencil_value     = 0;

   if (!glxx_hw_clear(state, &clear))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

end:
   glxx_unlock_server_state();
}


/*
   Check if 'func' is a valid depth or stencil test function.
*/

static GLboolean is_func(GLenum func)
{
   return func == GL_NEVER   ||
          func == GL_ALWAYS  ||
          func == GL_LESS    ||
          func == GL_LEQUAL  ||
          func == GL_EQUAL   ||
          func == GL_GREATER ||
          func == GL_GEQUAL  ||
          func == GL_NOTEQUAL;
}

/*
   glDepthFunc()

   Sets the function which determines whether a pixel passes or fails the
   depth test, specifying one of

      GL_NEVER
      GL_ALWAYS
      GL_LESS
      GL_LEQUAL
      GL_EQUAL
      GL_GREATER
      GL_GEQUAL
      GL_NOTEQUAL

   Gives GL_INVALID_ENUM error if the function is not one of these.

   Implementation: Done
   Error Checks: Done
*/

GL_API void GL_APIENTRY glDepthFunc(GLenum func) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (is_func(func))
   {
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
      state->depth_func = func;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

/*
   glDepthMask()

   Sets the write enable for the depth buffer. No errors are generated.

   Implementation: Done
   Error Checks: Done
*/

GL_API void GL_APIENTRY glDepthMask(GLboolean flag) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
   state->depth_mask = !!flag;

   glxx_unlock_server_state();
}

/*
   glDepthRangef (GLclampf zNear, GLclampf zFar)
   glDepthRangex (GLclampx zNear, GLclampx zFar)

   Khronos documentation:

   Each of n and f are clamped to lie within [0, 1], as are all arguments of type clampf
   or clampx. zw is taken to be represented in fixed-point with at least as many bits
   as there are in the depth buffer of the framebuffer. We assume that the fixed-point
   representation used represents each value k/(2^m - 1), where k in {0, 1, . . . , 2^m - 1},
   as k (e.g. 1.0 is represented in binary as a string of all ones).

   Implementation notes:

   We update the z_near and z_far elements of state.depth_range, potentially
   violating the invariant and call glxx_update_viewport_internal() to restore it.

   Preconditions:

   Valid EGL server state exists
   EGL server state has a current OpenGL ES 1.1 or 2.0 context

   Postconditions:

   -

   Invariants preserved:

   0.0 <= state.depth_range.z_near <= 1.0
   0.0 <= state.depth_range.z_far  <= 1.0

   state.vp_internal is consistent with other elements according to glxx_update_viewport_internal() docs
   elements of state.vp_internal are valid
*/

GL_API void GL_APIENTRY glDepthRangef(GLclampf zNear, GLclampf zFar)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   state->depth_range.z_near = gfx_fclamp(zNear, 0.0f, 1.0f);
   state->depth_range.z_far = gfx_fclamp(zFar, 0.0f, 1.0f);
   state->dirty.viewport = KHRN_RENDER_STATE_SET_ALL;

   glxx_update_viewport_internal(state);

   glxx_unlock_server_state();
}
GL_API void GL_APIENTRY glDepthRangex(GLclampx zNear, GLclampx zFar)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   state->depth_range.z_near = gfx_fclamp(fixed_to_float(zNear), 0.0f, 1.0f);
   state->depth_range.z_far = gfx_fclamp(fixed_to_float(zFar), 0.0f, 1.0f);
   state->dirty.viewport = KHRN_RENDER_STATE_SET_ALL;

   glxx_update_viewport_internal(state);

   glxx_unlock_server_state();
}

void glxx_update_color_material(GLXX_SERVER_STATE_T *state)
{
   if (state->gl11.statebits.vertex & GL11_COLORMAT)
   {
      int i;

      for (i = 0; i < 4; i++)
      {
         state->gl11.material.ambient[i] = state->generic_attrib[GL11_IX_COLOR].f[i];
         state->gl11.material.diffuse[i] = state->generic_attrib[GL11_IX_COLOR].f[i];
      }
   }
}

/* This excludes caps which are set by glEnable/DisableClientState */
static bool is_valid_server_cap(const GLXX_SERVER_STATE_T *state, GLenum cap)
{
   switch (cap)
   {
   case GL_NORMALIZE:
   case GL_RESCALE_NORMAL:
   case GL_FOG:
   case GL_LIGHTING:
   case GL_COLOR_MATERIAL:
   case GL_LIGHT0:
   case GL_LIGHT1:
   case GL_LIGHT2:
   case GL_LIGHT3:
   case GL_LIGHT4:
   case GL_LIGHT5:
   case GL_LIGHT6:
   case GL_LIGHT7:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_EXTERNAL_OES:
   case GL_ALPHA_TEST:
   case GL_CLIP_PLANE0:
   case GL_POINT_SMOOTH:
   case GL_POINT_SPRITE_OES:
   case GL_LINE_SMOOTH:
   case GL_MULTISAMPLE:
   case GL_SAMPLE_ALPHA_TO_ONE:
   case GL_MATRIX_PALETTE_OES:
   case GL_COLOR_LOGIC_OP:
      return IS_GL_11(state);

   case GL_CULL_FACE:
   case GL_POLYGON_OFFSET_FILL:
   case GL_SAMPLE_ALPHA_TO_COVERAGE:
   case GL_SAMPLE_COVERAGE:
   case GL_SCISSOR_TEST:
   case GL_STENCIL_TEST:
   case GL_DEPTH_TEST:
   case GL_BLEND:
   case GL_DITHER:
   case GL_DEBUG_OUTPUT_KHR:
   case GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR:
   case GL_BLEND_ADVANCED_COHERENT_KHR:
      return true;

   case GL_PRIMITIVE_RESTART_FIXED_INDEX:
   case GL_RASTERIZER_DISCARD:
      return !IS_GL_11(state);
#if V3D_VER_AT_LEAST(4,1,34,0)
   case GL_SAMPLE_SHADING:
#endif
   case GL_SAMPLE_MASK:
      return V3D_VER_AT_LEAST(3,3,0,0) && !IS_GL_11(state);

   default:
      return false;
   }
}

static bool is_valid_cap(const GLXX_SERVER_STATE_T *state, GLenum cap)
{
   if (is_valid_server_cap(state, cap))
      return true;

   /* Caps set by glEnable/DisableClientState */
   if (!IS_GL_11(state))
      return false;
   switch (cap)
   {
   case GL_COLOR_ARRAY:
   case GL_NORMAL_ARRAY:
   case GL_MATRIX_INDEX_ARRAY_OES:
   case GL_POINT_SIZE_ARRAY_OES:
   case GL_TEXTURE_COORD_ARRAY:
   case GL_VERTEX_ARRAY:
   case GL_WEIGHT_ARRAY_OES:
      return true;
   default:
      return false;
   }
}

static void set_enabled(GLenum cap, bool enabled)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);

   static_assrt(GL11_CONFIG_MAX_PLANES == 1);
   static_assrt(GL11_CONFIG_MAX_LIGHTS == 8);

   if (!state) return;

   if (!is_valid_server_cap(state, cap)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   switch (cap)
   {
   case GL_NORMALIZE:
      SET_INDIVIDUAL(state->gl11.statebits.vertex, GL11_NO_NORMALIZE, !enabled);
      break;
   case GL_RESCALE_NORMAL:
      SET_INDIVIDUAL(state->gl11.statebits.v_enable, GL11_NO_NORMALIZE, !enabled);
      break;
   case GL_FOG:
      SET_MULTI(state->gl11.statebits.f_enable, GL11_FOG_M, enabled);
      break;
   case GL_LIGHTING:
      SET_MULTI(state->gl11.statebits.v_enable2, GL11_LIGHTING_M, enabled);
      break;
   case GL_COLOR_MATERIAL:
      SET_INDIVIDUAL(state->gl11.statebits.vertex, GL11_COLORMAT, enabled);
      glxx_update_color_material(state);
      break;
   case GL_LIGHT0:
   case GL_LIGHT1:
   case GL_LIGHT2:
   case GL_LIGHT3:
   case GL_LIGHT4:
   case GL_LIGHT5:
   case GL_LIGHT6:
   case GL_LIGHT7:
      SET_INDIVIDUAL(state->gl11.statebits.v_enable, GL11_LIGHT_M << (cap - GL_LIGHT0), enabled);
      break;
   case GL_TEXTURE_2D:
      {
         GL11_TEXUNIT_T *texunit = state->gl11.texunits + state->active_texture;
         texunit->target_enabled_2D = enabled;
      }
      break;
   case GL_TEXTURE_EXTERNAL_OES:
      {
         GL11_TEXUNIT_T *texunit = state->gl11.texunits + state->active_texture;
         texunit->target_enabled_EXTERNAL_OES = enabled;
      }
      break;
   case GL_ALPHA_TEST:
      SET_MULTI(state->gl11.statebits.f_enable, GL11_AFUNC_M, enabled);
      break;
   case GL_CLIP_PLANE0:
      SET_MULTI(state->gl11.statebits.f_enable, GL11_UCLIP_M, enabled);
      break;
   case GL_POINT_SMOOTH:
      SET_INDIVIDUAL(state->gl11.statebits.fragment, GL11_POINTSMOOTH, enabled);
      break;
   case GL_POINT_SPRITE_OES:
      state->gl11.point_sprite = enabled;
      break;
   case GL_LINE_SMOOTH:
      state->dirty.linewidth = KHRN_RENDER_STATE_SET_ALL;
      SET_INDIVIDUAL(state->gl11.statebits.fragment, GL11_LINESMOOTH, enabled);
      break;
   case GL_CULL_FACE:
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
      state->caps.cull_face = enabled;
      break;
   case GL_POLYGON_OFFSET_FILL:
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
      state->caps.polygon_offset_fill = enabled;
      break;
   case GL_MULTISAMPLE:
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
      state->caps.multisample = enabled;
      break;
   case GL_SAMPLE_ALPHA_TO_COVERAGE:
      SET_INDIVIDUAL(state->statebits.backend, GLSL_SAMPLE_ALPHA, enabled);
      break;
   case GL_SAMPLE_ALPHA_TO_ONE:
      SET_INDIVIDUAL(state->gl11.statebits.fragment, GL11_SAMPLE_ONE, enabled);
      break;
   case GL_SAMPLE_COVERAGE:
      state->dirty.sample_state = KHRN_RENDER_STATE_SET_ALL;
      state->sample_coverage.enable = !!enabled;
      break;
   case GL_SCISSOR_TEST:
      state->dirty.viewport = KHRN_RENDER_STATE_SET_ALL;
      state->caps.scissor_test = enabled;
      break;
   case GL_STENCIL_TEST:
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
      state->caps.stencil_test = enabled;
      break;
   case GL_DEPTH_TEST:
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
      state->caps.depth_test = enabled;
      break;
   case GL_BLEND:
#if V3D_VER_AT_LEAST(4,1,34,0)
      state->blend.rt_enables = enabled ? gfx_mask(V3D_MAX_RENDER_TARGETS) : 0;
      state->dirty.blend_enables = KHRN_RENDER_STATE_SET_ALL;
#else
      state->blend.enable = enabled;
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
#endif
      break;
   case GL_DITHER:
      state->caps.dither = enabled;
      break;
   case GL_COLOR_LOGIC_OP:
      SET_MULTI(state->gl11.statebits.f_enable, GL11_LOGIC_M, enabled);
      /* Set dirty.cfg so that blending gets switched off */
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
      break;
   case GL_MATRIX_PALETTE_OES:
      SET_MULTI(state->gl11.statebits.v_enable, GL11_MPAL_M, enabled);
      break;
   case GL_PRIMITIVE_RESTART_FIXED_INDEX:
      state->caps.primitive_restart = enabled;
      break;
   case GL_RASTERIZER_DISCARD:
      state->caps.rasterizer_discard = enabled;
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
      break;
   case GL_SAMPLE_MASK:
#if V3D_VER_AT_LEAST(4,1,34,0)
      state->dirty.sample_state = KHRN_RENDER_STATE_SET_ALL;
#endif
      state->sample_mask.enable = !!enabled;
      break;
   case GL_DEBUG_OUTPUT_KHR:
      state->khr_debug.debug_output = enabled;
      break;
   case GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR:
      state->khr_debug.debug_output_synchronous = enabled;
      break;
   case GL_BLEND_ADVANCED_COHERENT_KHR:
      state->blend.advanced_coherent = enabled;
      break;
#if V3D_VER_AT_LEAST(4,1,34,0)
   case GL_SAMPLE_SHADING:
      state->caps.sample_shading = enabled;
      break;
#endif
   default:
      unreachable();
      break;
   }

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glDisable(GLenum cap)
{
   set_enabled(cap, false);
}

GL_API void GL_APIENTRY glEnable(GLenum cap)
{
   set_enabled(cap, true);
}

#if V3D_VER_AT_LEAST(4,1,34,0)

static void set_enabled_i(GLenum target, GLuint index, bool enabled)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (target != GL_BLEND)
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else if (index >= V3D_MAX_RENDER_TARGETS)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else
   {
      uint32_t bit = 1u << index;
      uint32_t new_enables = enabled ?
         (state->blend.rt_enables | bit) :
         (state->blend.rt_enables & ~bit);
      if (state->blend.rt_enables != new_enables)
      {
         state->blend.rt_enables = new_enables;
         state->dirty.blend_enables = KHRN_RENDER_STATE_SET_ALL;
      }
   }

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glDisableiEXT(GLenum target, GLuint index)
{
   set_enabled_i(target, index, false);
}

GL_API void GL_APIENTRY glEnableiEXT(GLenum target, GLuint index)
{
   set_enabled_i(target, index, true);
}

GL_API void GL_APIENTRY glDisableiOES(GLenum target, GLuint index)
{
   set_enabled_i(target, index, false);
}

GL_API void GL_APIENTRY glEnableiOES(GLenum target, GLuint index)
{
   set_enabled_i(target, index, true);
}

#endif

#if KHRN_GLES32_DRIVER

GL_API void GL_APIENTRY glDisablei(GLenum target, GLuint index)
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   set_enabled_i(target, index, false);
#endif
}

GL_API void GL_APIENTRY glEnablei(GLenum target, GLuint index)
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   set_enabled_i(target, index, true);
#endif
}

#endif

GL_API void GL_APIENTRY glFinish(void)
{
   {
      PROFILE_FUNCTION_MT("GL");

      GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);

      if (!state) return;

      glxx_server_state_flush(state, true);

      glxx_unlock_server_state();
   }

   profile_on_finish();
}

GL_API void GL_APIENTRY glFlush(void)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);

   if (!state) return;

   glxx_server_state_flush(state, false);
   glxx_unlock_server_state();
}

/*
   glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
   glClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha)

   Khronos documentation:

   Each of the specified components is clamped to [0, 1] and converted to fixed-point
   according to the rules of section 2.12.8.

   Implementation notes:

   Conversion to fixed point is deferred to color buffer clear time.

   Preconditions:

   Valid EGL server state exists
   EGL server state has a current OpenGL ES 1.1 or 2.0 context

   Postconditions:

   Invariants preserved:

   state.clear_color is valid
*/

GL_API void GL_APIENTRY glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   state->clear.color_value[0] = gfx_fclamp(red, 0.0f, 1.0f);
   state->clear.color_value[1] = gfx_fclamp(green, 0.0f, 1.0f);
   state->clear.color_value[2] = gfx_fclamp(blue, 0.0f, 1.0f);
   state->clear.color_value[3] = gfx_fclamp(alpha, 0.0f, 1.0f);

   glxx_unlock_server_state();
}
GL_API void GL_APIENTRY glClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   state->clear.color_value[0] = gfx_fclamp(fixed_to_float(red), 0.0f, 1.0f);
   state->clear.color_value[1] = gfx_fclamp(fixed_to_float(green), 0.0f, 1.0f);
   state->clear.color_value[2] = gfx_fclamp(fixed_to_float(blue), 0.0f, 1.0f);
   state->clear.color_value[3] = gfx_fclamp(fixed_to_float(alpha), 0.0f, 1.0f);

   glxx_unlock_server_state();
}

/*
   glClearDepth(GLclampf depth)
   glClearDepthx(GLclampx depth)

   Khronos documentation:

   Takes a value that is clamped to the range [0, 1] and converted to fixed-point according
   to the rules for a window z value given in section 2.10.1, which says:

   zw is taken to be represented in fixed-point with at least as many bits
   as there are in the depth buffer of the framebuffer. We assume that the fixed-point
   representation used represents each value k/(2^m - 1), where k in {0, 1, . . . , 2^m - 1},
   as k (e.g. 1.0 is represented in binary as a string of all ones).

   Implementation notes:

   Conversion to fixed point is deferred to depth buffer clear time.

   Preconditions:

   Valid EGL server state exists
   EGL server state has a current OpenGL ES 1.1 or 2.0 context

   Postconditions:

   Invariants preserved:

   state.clear_depth is valid
*/

GL_API void GL_APIENTRY glClearDepthf(GLclampf depth)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   state->clear.depth_value = gfx_fclamp(depth, 0.0f, 1.0f);

   glxx_unlock_server_state();
}
GL_API void GL_APIENTRY glClearDepthx(GLclampx depth)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   state->clear.depth_value = gfx_fclamp(fixed_to_float(depth), 0.0f, 1.0f);

   glxx_unlock_server_state();
}


static GLboolean is_front_face(GLenum mode)
{
   return mode == GL_CW ||
          mode == GL_CCW;
}

/*
   glFrontFace()

   Sets which winding order is considered to be front facing, specifying
   one of CW (clockwise) or CCW (counterclockwise). Gives GL_INVALID_ENUM
   error if the mode is not one of these.

   Implementation: Done
   Error Checks: Done
*/

GL_API void GL_APIENTRY glFrontFace(GLenum mode) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (is_front_face(mode))
   {
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
      state->front_face = mode;
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGenBuffers(GLsizei n, GLuint *buffers)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   int32_t i = 0;
   if (!state) return;

   if (n < 0)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);   // The conformance tests insist...
   else if (buffers) {
      GLXX_SHARED_T *shared = state->shared;
      while (i < n)
      {
         bool out_of_memory;
         GLXX_BUFFER_T *buf = glxx_shared_allocate_buffer(shared, shared->next_buffer, &out_of_memory);

         if (out_of_memory)
         {
            glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
            break;
         }

         if (buf != NULL)
            buffers[i++] = shared->next_buffer;

         shared->next_buffer++;
      }
   }

   glxx_unlock_server_state();
}


static GLboolean is_hint(GLenum mode)
{
   return mode == GL_FASTEST ||
          mode == GL_NICEST  ||
          mode == GL_DONT_CARE;
}

GL_API void GL_APIENTRY glHint(GLenum target, GLenum mode)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (is_hint(mode))
   {
      switch (target)
      {
      case GL_PERSPECTIVE_CORRECTION_HINT:
         if (IS_GL_11(state))
            state->gl11.hints.perspective_correction = mode;
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      case GL_POINT_SMOOTH_HINT:
         if (IS_GL_11(state))
            state->gl11.hints.point_smooth = mode;
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      case GL_LINE_SMOOTH_HINT:
         if (IS_GL_11(state))
            state->gl11.hints.line_smooth = mode;
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      case GL_FOG_HINT:
         if (IS_GL_11(state))
            state->gl11.hints_program.fog = mode;
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      case GL_GENERATE_MIPMAP_HINT:
         state->hints.generate_mipmap = mode;
         break;
      case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
         state->hints.fshader_derivative = mode;
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

GL_API GLboolean GL_APIENTRY glIsEnabled(GLenum cap)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   if (!state)
      return GL_FALSE;

   GLboolean result = GL_FALSE;
   if (is_valid_cap(state, cap))
      verif(glxx_get_booleans(state, cap, &result) == GL_NO_ERROR);
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();

   return result;
}

#if V3D_VER_AT_LEAST(4,1,34,0)

static GLboolean is_enabled_i(GLenum target, GLuint index)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   if (!state)
      return GL_FALSE;

   GLboolean result = GL_FALSE;
   GLenum error;
   if (target == GL_BLEND)
      error = glxx_get_booleans_i(state, target, index, &result);
   else
      error = GL_INVALID_ENUM;
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();

   return result;
}

GL_API GLboolean GL_APIENTRY glIsEnablediEXT(GLenum target, GLuint index)
{
   return is_enabled_i(target, index);
}

GL_API GLboolean GL_APIENTRY glIsEnablediOES(GLenum target, GLuint index)
{
   return is_enabled_i(target, index);
}

#endif

#if KHRN_GLES32_DRIVER

GL_API GLboolean GL_APIENTRY glIsEnabledi(GLenum target, GLuint index)
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   return is_enabled_i(target, index);
#else
   return GL_FALSE;
#endif
}

#endif

/*
   void glLineWidth (GLfloat width)
   void glLineWidthx (GLfixed width)

   Khronos documentation:

   Line width may be set by calling

      void LineWidth( float width );
      void LineWidthx( fixed width );

   with an appropriate positive width, controls the width of rasterized line segments.
   The default width is 1.0. Values less than or equal to 0.0 generate the error
   INVALID VALUE.

   Implementation notes:

   Preconditions:

   Valid EGL server state exists
   EGL server state has a current OpenGL ES 1.1 or 2.0 context

   Postconditions:

   If !(width > 0) and no current error, error becomes GL_INVALID_ENUM

   Invariants preserved:

   state.line_width > 0.0
*/

GL_API void GL_APIENTRY glLineWidth(GLfloat width) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (width > 0.0f)
   {
      state->dirty.linewidth = KHRN_RENDER_STATE_SET_ALL;
      state->line_width = width;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   glxx_unlock_server_state();
}
GL_API void GL_APIENTRY glLineWidthx(GLfixed width)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   float w = fixed_to_float(width);
   if (!state)
      return;

   if (w > 0.0f)
   {
      state->dirty.linewidth = KHRN_RENDER_STATE_SET_ALL;
      state->line_width = w;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   glxx_unlock_server_state();
}

#if V3D_VER_AT_LEAST(4,1,34,0)

static void glxx_patch_parameter(GLenum pname, GLint value)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (pname != GL_PATCH_VERTICES)
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }
   else if (value <= 0 || value > V3D_MAX_PATCH_VERTICES)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   }
   else
   {
      state->num_patch_vertices = value;
   }

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glPatchParameteriOES(GLenum pname, GLint value)
{
   glxx_patch_parameter(pname, value);
}

GL_API void GL_APIENTRY glPatchParameteriEXT(GLenum pname, GLint value)
{
   glxx_patch_parameter(pname, value);
}

#endif

#if KHRN_GLES32_DRIVER

GL_API void GL_APIENTRY glPatchParameteri(GLenum pname, GLint value)
{
   glxx_patch_parameter(pname, value);
}

#endif


#if V3D_VER_AT_LEAST(4,1,34,0)
GL_API void GL_APIENTRY glPolygonOffsetClampEXT(GLfloat factor, GLfloat units, GLfloat limit)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   state->dirty.polygon_offset  = KHRN_RENDER_STATE_SET_ALL;
   state->polygon_offset.limit  = limit;
   state->polygon_offset.factor = factor;
   state->polygon_offset.units  = units;

   glxx_unlock_server_state();
}
#endif

GL_API void GL_APIENTRY glPolygonOffset(GLfloat factor, GLfloat units)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   state->dirty.polygon_offset = KHRN_RENDER_STATE_SET_ALL;
#if V3D_VER_AT_LEAST(4,1,34,0)
   state->polygon_offset.limit = 0.0f;
#endif
   state->polygon_offset.factor = factor;
   state->polygon_offset.units = units;

   glxx_unlock_server_state();
}
GL_API void GL_APIENTRY glPolygonOffsetx(GLfixed factor, GLfixed units)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   state->dirty.polygon_offset = KHRN_RENDER_STATE_SET_ALL;
#if V3D_VER_AT_LEAST(4,1,34,0)
   state->polygon_offset.limit = 0.0f;
#endif
   state->polygon_offset.factor = fixed_to_float(factor);
   state->polygon_offset.units = fixed_to_float(units);

   glxx_unlock_server_state();
}

static GLenum readpixels_check_internals(const GLXX_FRAMEBUFFER_T *fb, int x,
      int y, GLsizei width, GLsizei height, GLenum format, GLenum type,
      GLsizei buf_size, const GLXX_BUFFER_T *pixel_buffer, const void *pixels,
      glxx_context_fences *fences)
{
   if (format == GL_DEPTH_COMPONENT || format == GL_DEPTH_STENCIL)
      return GL_INVALID_OPERATION;

   if (!glxx_fb_is_complete(fb, fences))
      return GL_INVALID_FRAMEBUFFER_OPERATION;

   if (fb->name != 0 && (glxx_fb_get_ms_mode(fb) != GLXX_NO_MS))
      return GL_INVALID_OPERATION;

   if (width < 0 || height < 0 || (!pixel_buffer && !pixels))
      return GL_INVALID_VALUE;

   if (buf_size < 0)
      return GL_INVALID_OPERATION;

   return GL_NO_ERROR;
}

static void read_pixels(int x, int y, GLsizei width, GLsizei height,
   GLenum format, GLenum type, GLsizei buf_size, void *pixels)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state) return;

   khrn_image *src = NULL;     /* Used in error handler at 'end' */

   GLXX_BUFFER_T *pixel_buffer = state->bound_buffer[GLXX_BUFTGT_PIXEL_PACK].obj;
   GLenum error = readpixels_check_internals(state->bound_read_framebuffer, x,
         y, width, height, format, type, buf_size, pixel_buffer, pixels,
         &state->fences);
   if (error != GL_NO_ERROR)
      goto end;

   if (!glxx_fb_acquire_read_image(state->bound_read_framebuffer,
            GLXX_DOWNSAMPLED, &src, NULL))
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }

   if (src == NULL)
   {
      error = GL_INVALID_FRAMEBUFFER_OPERATION;
      goto end;
   }

   GFX_LFMT_T src_lfmt = khrn_image_get_lfmt(src, 0);
   GFX_LFMT_T dst_lfmt;
   GLenum dst_format, dst_type;

   /* ES3 spec: "Only two combinations of format and type are accepted in
      most cases [...]" */
   glxx_readpixels_std_formats(&dst_format, &dst_type, &dst_lfmt, src_lfmt);
   if (dst_format != format || dst_type != type)
   {
      glxx_readpixels_impldefined_formats(&dst_format, &dst_type, &dst_lfmt,
         src->api_fmt);
      if (dst_format != format || dst_type != type)
      {
         error = GL_INVALID_OPERATION;
         goto end;
      }
   }

   /* The size is zero, do nothing or we don't have where to copy data */
   if (width == 0 || height == 0 ||
      (pixel_buffer == NULL && pixels == NULL))
      goto end;

   struct glxx_pixels_info dst_info;
   glxx_get_pack_unpack_info(&state->pixel_store_state, true, width,
         height, format, type, &dst_info);

   GFX_BUFFER_DESC_T dst_desc = {0};
   dst_desc.width = width;
   dst_desc.height = height;
   dst_desc.depth = 1;
   dst_desc.num_planes = 1;
   dst_desc.planes[0].lfmt = dst_lfmt;
   dst_desc.planes[0].pitch = dst_info.stride;

   size_t buf_offset = 0;
   void *dst_ptr;
   if (pixel_buffer)
   {
      /* in this case, pixels are just an offset in the pixel buffer object */

      if (!glxx_check_buffer_valid_access(pixel_buffer,
               &dst_desc, (uintptr_t)pixels, dst_info.offset, 1, 0))
      {
         error = GL_INVALID_OPERATION;
         goto end;
      }

      buf_offset = dst_info.offset + (uintptr_t)pixels;
      dst_ptr = glxx_buffer_map(pixel_buffer, buf_offset, GL_MAP_WRITE_BIT);
      if (!dst_ptr)
      {
         error = GL_OUT_OF_MEMORY;
         goto end;
      }
   }
   else
   {
      if ((dst_desc.height * dst_desc.planes[0].pitch) +
         dst_info.offset > (unsigned)buf_size)
      {
         error = GL_INVALID_OPERATION;
         goto end;
      }

      dst_ptr = (uint8_t*)pixels + dst_info.offset;
   }

   struct v3d_imgconv_ptr_tgt dst;
   v3d_imgconv_init_ptr_tgt(&dst, dst_ptr, &dst_desc, 0, 0, 0, 0, dst_info.slice_pitch);

   /* Values for pixels that lie outside the window connected to the current GL
      context are undefined. */
   unsigned src_width = khrn_image_get_width(src);
   unsigned src_height = khrn_image_get_height(src);
   int dst_x = 0, dst_y = 0;
   glxx_clamp_rect(src_width, src_height, &x, &y, &width, &height, &dst_x, &dst_y);

   assert(dst_x >= 0);
   assert(dst_y >= 0);

   bool ok = true;
   if (width > 0 && height > 0)
   {
      assert(khrn_image_get_num_elems(src) == 1 &&
             khrn_image_get_depth(src) == 1);

      bool secure_context = egl_context_gl_secure(state->context);
      ok = khrn_image_convert_to_ptr_tgt(src, x, y, (unsigned)dst_x, (unsigned)dst_y,
                           &dst, width, height, 1, 1, &state->fences, secure_context);
   }

   if (pixel_buffer)
      glxx_buffer_unmap(pixel_buffer, buf_offset, GL_MAP_WRITE_BIT);

   if (!ok)
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }

end:
   KHRN_MEM_ASSIGN(src, NULL);
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height,
   GLenum format, GLenum type, void *pixels)
{
   read_pixels(x, y, width, height, format, type, INT_MAX, pixels);
}

GL_API void GL_APIENTRY glReadnPixelsEXT(GLint x, GLint y, GLsizei width, GLsizei height,
   GLenum format, GLenum type, GLsizei buf_size, void *pixels)
{
   read_pixels(x, y, width, height, format, type, buf_size, pixels);
}

GL_API void GL_APIENTRY glReadnPixelsKHR(GLint x, GLint y, GLsizei width, GLsizei height,
   GLenum format, GLenum type, GLsizei buf_size, void *pixels)
{
   read_pixels(x, y, width, height, format, type, buf_size, pixels);
}

#if KHRN_GLES32_DRIVER
GL_API void GL_APIENTRY glReadnPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                                      GLenum format, GLenum type, GLsizei buf_size, void *pixels)
{
   read_pixels(x, y, width, height, format, type, buf_size, pixels);
}
#endif

GL_API void GL_APIENTRY glSampleCoverage(GLclampf value, GLboolean invert)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state) return;

   state->dirty.sample_state = KHRN_RENDER_STATE_SET_ALL;
   state->sample_coverage.invert = !!invert;
   state->sample_coverage.value = gfx_fclamp(value, 0.0f, 1.0f);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glSampleCoveragex(GLclampx value, GLboolean invert)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state) return;

   state->dirty.sample_state = KHRN_RENDER_STATE_SET_ALL;
   state->sample_coverage.invert = !!invert;
   state->sample_coverage.value = gfx_fclamp(fixed_to_float(value), 0.0f, 1.0f);

   glxx_unlock_server_state();
}

#if V3D_VER_AT_LEAST(3,3,0,0)

GL_API void GL_APIENTRY glSampleMaski(GLuint maskNumber, GLbitfield mask)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if(maskNumber >= GLXX_CONFIG_MAX_SAMPLE_WORDS) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

#if V3D_VER_AT_LEAST(4,1,34,0)
   state->dirty.sample_state = KHRN_RENDER_STATE_SET_ALL;
#endif
   state->sample_mask.mask[maskNumber] = mask;

end:
   glxx_unlock_server_state();
}

#endif

GL_API void GL_APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (width >= 0 && height >= 0)
   {
      state->dirty.viewport = KHRN_RENDER_STATE_SET_ALL;
      state->scissor.x = x;
      state->scissor.y = y;
      state->scissor.width = width;
      state->scissor.height = height;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   glxx_unlock_server_state();
}

static GLboolean is_face(GLenum face)
{
   return face == GL_FRONT ||
          face == GL_BACK  ||
          face == GL_FRONT_AND_BACK;
}

GL_API void GL_APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (is_func(func))
   {
      state->stencil_func.front.func = func;
      state->stencil_func.front.ref = ref;
      state->stencil_func.front.mask = mask;

      state->stencil_func.back.func = func;
      state->stencil_func.back.ref = ref;
      state->stencil_func.back.mask = mask;

      state->dirty.stencil = KHRN_RENDER_STATE_SET_ALL;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (is_face(face) && is_func(func))
   {
      if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
      {
         state->stencil_func.front.func = func;
         state->stencil_func.front.ref = ref;
         state->stencil_func.front.mask = mask;
         state->dirty.stencil = KHRN_RENDER_STATE_SET_ALL;
      }

      if (face == GL_BACK || face == GL_FRONT_AND_BACK)
      {
         state->stencil_func.back.func = func;
         state->stencil_func.back.ref = ref;
         state->stencil_func.back.mask = mask;
         state->dirty.stencil = KHRN_RENDER_STATE_SET_ALL;
      }
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

static GLboolean is_op(GLenum op)
{
   return op == GL_KEEP    ||
          op == GL_ZERO    ||
          op == GL_REPLACE ||
          op == GL_INCR    ||
          op == GL_DECR    ||
          op == GL_INVERT  ||
          op == GL_INCR_WRAP ||
          op == GL_DECR_WRAP;
}

GL_API void GL_APIENTRY glStencilMask(GLuint mask) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   state->stencil_mask.front = mask;
   state->stencil_mask.back = mask;
   state->dirty.stencil = KHRN_RENDER_STATE_SET_ALL;

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glStencilMaskSeparate(GLenum face, GLuint mask) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (is_face(face))
   {
      if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
      {
         state->stencil_mask.front = mask;
         state->dirty.stencil = KHRN_RENDER_STATE_SET_ALL;
      }

      if (face == GL_BACK || face == GL_FRONT_AND_BACK)
      {
         state->stencil_mask.back = mask;
         state->dirty.stencil = KHRN_RENDER_STATE_SET_ALL;
      }
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (is_op(fail) && is_op(zfail) && is_op(zpass))
   {
      state->stencil_op.front.fail = fail;
      state->stencil_op.front.zfail = zfail;
      state->stencil_op.front.zpass = zpass;

      state->stencil_op.back.fail = fail;
      state->stencil_op.back.zfail = zfail;
      state->stencil_op.back.zpass = zpass;

      state->dirty.stencil = KHRN_RENDER_STATE_SET_ALL;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (is_face(face) && is_op(fail) && is_op(zfail) && is_op(zpass))
   {
      if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
      {
         state->stencil_op.front.fail = fail;
         state->stencil_op.front.zfail = zfail;
         state->stencil_op.front.zpass = zpass;
         state->dirty.stencil = KHRN_RENDER_STATE_SET_ALL;
      }

      if (face == GL_BACK || face == GL_FRONT_AND_BACK)
      {
         state->stencil_op.back.fail = fail;
         state->stencil_op.back.zfail = zfail;
         state->stencil_op.back.zpass = zpass;
         state->dirty.stencil = KHRN_RENDER_STATE_SET_ALL;
      }
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

static bool is_vertex_attrib_size(GLint size)
{
   return size >= 1 && size <= 4;
}

static bool is_vertex_attrib_type_size_valid(GLenum type, GLint size)
{
   switch (type)
   {
   case GL_UNSIGNED_INT_2_10_10_10_REV:
   case GL_INT_2_10_10_10_REV:
      return size == 4;
   default:
      return size >= 1 && size <= 4;
   }
}

static bool is_vertex_attrib_type(GLenum type)
{
   if (type == GL_BYTE               ||
       type == GL_UNSIGNED_BYTE      ||
       type == GL_SHORT              ||
       type == GL_UNSIGNED_SHORT     ||
       type == GL_FLOAT              ||
       type == GL_FIXED              ||
       type == GL_HALF_FLOAT_OES     ||
       type == GL_HALF_FLOAT         ||
       type == GL_INT                ||
       type == GL_UNSIGNED_INT       ||
       type == GL_INT_2_10_10_10_REV ||
       type == GL_UNSIGNED_INT_2_10_10_10_REV)
   {
      return true;
   }

   return false;
}

static bool is_vertex_attrib_int_type(GLenum type)
{
   if (type == GL_BYTE           ||
       type == GL_SHORT          ||
       type == GL_INT            ||
       type == GL_UNSIGNED_BYTE  ||
       type == GL_UNSIGNED_SHORT ||
       type == GL_UNSIGNED_INT)
   {
      return true;
   }
   return false;
}

static bool attrib_index_valid(GLXX_SERVER_STATE_T *state, uint32_t indx)
{
   if (indx < V3D_MAX_ATTR_ARRAYS)
      return true;
   else {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      return false;
   }
}

static v3d_attr_type_t translate_attrib_type(bool* is_signed, GLenum type)
{
   switch (type)
   {
   case GL_BYTE:                          *is_signed = true;   return V3D_ATTR_TYPE_BYTE;
   case GL_UNSIGNED_BYTE:                 *is_signed = false;  return V3D_ATTR_TYPE_BYTE;
   case GL_SHORT:                         *is_signed = true;   return V3D_ATTR_TYPE_SHORT;
   case GL_UNSIGNED_SHORT:                *is_signed = false;  return V3D_ATTR_TYPE_SHORT;
   case GL_INT:                           *is_signed = true;   return V3D_ATTR_TYPE_INT;
   case GL_UNSIGNED_INT:                  *is_signed = false;  return V3D_ATTR_TYPE_INT;
   case GL_FLOAT:                         *is_signed = false;  return V3D_ATTR_TYPE_FLOAT;
   case GL_HALF_FLOAT:                    *is_signed = false;  return V3D_ATTR_TYPE_HALF_FLOAT;
   case GL_FIXED:                         *is_signed = false;  return V3D_ATTR_TYPE_FIXED;
   case GL_HALF_FLOAT_OES:                *is_signed = false;  return V3D_ATTR_TYPE_HALF_FLOAT;
   case GL_INT_2_10_10_10_REV:            *is_signed = true;   return V3D_ATTR_TYPE_INT2_10_10_10;
   case GL_UNSIGNED_INT_2_10_10_10_REV:   *is_signed = false;  return V3D_ATTR_TYPE_INT2_10_10_10;
   default:
      unreachable();
   }
}

static void vertex_attrib_pointer(GLXX_SERVER_STATE_T *state, GLuint index, GLint size,
   GLenum type, GLboolean normalized, GLsizei stride, GLintptr ptr, bool integer)
{
   // Can't create a vertex array object containing client array pointers.
   if (state->vao.bound != state->vao.default_vao && ptr != 0 &&
         state->bound_buffer[GLXX_BUFTGT_ARRAY].obj == NULL)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return;
   }

   GLXX_ATTRIB_CONFIG_T *attr = &state->vao.bound->attrib_config[index];
   GLXX_VBO_BINDING_T *vbo = &state->vao.bound->vbos[index];

   attr->gl_type = type;
   attr->v3d_type = translate_attrib_type(&attr->is_signed, type);
   attr->norm = normalized;
   attr->size = size;
   attr->vbo_index = index;
   attr->total_size = v3d_attr_type_get_size_in_memory(attr->v3d_type, size);
   attr->is_int = integer;
   attr->relative_offset = 0;

   if (stride == 0)
      vbo->stride = attr->total_size;
   else
      vbo->stride = stride;

   attr->original_stride = stride;
   attr->stride = vbo->stride;
   attr->pointer = (const void *)ptr;

   if(state->bound_buffer[GLXX_BUFTGT_ARRAY].obj != NULL)
      vbo->offset = ptr;

   KHRN_MEM_ASSIGN(vbo->buffer, state->bound_buffer[GLXX_BUFTGT_ARRAY].obj);
}

static void vertex_attrib_pointer_chk(GLXX_SERVER_STATE_T *state, GLuint index, GLint size,
   GLenum type, GLboolean normalized, GLsizei stride, GLintptr ptr, bool integer)
{
   if(!is_vertex_attrib_size(size) || index >= V3D_MAX_ATTR_ARRAYS)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      return;
   }

   if(integer && !is_vertex_attrib_int_type(type))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return;
   }
   if(!integer && !is_vertex_attrib_type(type))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return;
   }

   if(!is_vertex_attrib_type_size_valid(type, size))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return;
   }

   if (stride < 0 || stride > GLXX_CONFIG_MAX_VERTEX_ATTRIB_STRIDE) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      return;
   }

   vertex_attrib_pointer(state, index, size, type, normalized, stride, ptr, integer);
}

void glintAttribPointer_GL11(GLXX_SERVER_STATE_T *state, GLuint index, GLint size,
   GLenum type, GLboolean normalized, GLsizei stride, GLintptr ptr)
{
   vertex_attrib_pointer(state, index, size, type, normalized, stride, ptr, false);
}

#if V3D_VER_AT_LEAST(3,3,0,0)

static void vertex_attrib_format(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset, bool integer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if(attribindex >= V3D_MAX_ATTR_ARRAYS
      || relativeoffset > GLXX_CONFIG_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET
      || !is_vertex_attrib_size(size))
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if(integer && !is_vertex_attrib_int_type(type))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }
   if(!integer && !is_vertex_attrib_type(type))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if(state->vao.bound == state->vao.default_vao
      || !is_vertex_attrib_type_size_valid(type, size))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   GLXX_ATTRIB_CONFIG_T *attr = &state->vao.bound->attrib_config[attribindex];

   attr->gl_type = type;
   attr->v3d_type = translate_attrib_type(&attr->is_signed, type);
   attr->norm = normalized;
   attr->size = size;
   attr->total_size = v3d_attr_type_get_size_in_memory(attr->v3d_type, size);
   attr->is_int = integer;
   attr->relative_offset = relativeoffset;

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
   vertex_attrib_format(attribindex, size, type, normalized, relativeoffset, false);
}

GL_API void GL_APIENTRY glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
   vertex_attrib_format(attribindex, size, type, GL_FALSE, relativeoffset, true);
}

#endif

GL_API void GL_APIENTRY glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *ptr)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   vertex_attrib_pointer_chk(state, index, size, type, normalized, stride, (GLintptr)ptr, false);
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* ptr)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   vertex_attrib_pointer_chk(state, index, size, type, GL_FALSE, stride, (GLintptr)ptr, true);
   glxx_unlock_server_state();
}

#if V3D_VER_AT_LEAST(3,3,0,0)

GL_API void GL_APIENTRY glVertexAttribBinding (GLuint attribindex, GLuint bindingindex)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;


   if(attribindex >= V3D_MAX_ATTR_ARRAYS
      || bindingindex >= GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if(state->vao.bound == state->vao.default_vao)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   state->vao.bound->attrib_config[attribindex].vbo_index = bindingindex;

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glVertexBindingDivisor(GLuint bindingindex, GLuint divisor)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if(bindingindex >= GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if(state->vao.bound == state->vao.default_vao)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   state->vao.bound->vbos[bindingindex].divisor = divisor;

end:
   glxx_unlock_server_state();
}

#endif

void glintAttrib(uint32_t api, uint32_t indx, float x, float y, float z, float w)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(api);
   if (!state)
      return;

   if (attrib_index_valid(state, indx))
   {
      GLXX_GENERIC_ATTRIBUTE_T* ga = &state->generic_attrib[indx];
      ga->f[0] = x;
      ga->f[1] = y;
      ga->f[2] = z;
      ga->f[3] = w;
      ga->type = V3D_ATTR_TYPE_FLOAT;
      ga->is_signed = false;
   }

   glxx_unlock_server_state();
}

void glintAttribI(uint32_t api, uint32_t indx, uint32_t x, uint32_t y, uint32_t z, uint32_t w, bool is_signed)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(api);
   if (!state)
      return;

   if (attrib_index_valid(state, indx))
   {
      GLXX_GENERIC_ATTRIBUTE_T* ga = &state->generic_attrib[indx];
      ga->u[0] = x;
      ga->u[1] = y;
      ga->u[2] = z;
      ga->u[3] = w;
      ga->type = V3D_ATTR_TYPE_INT;
      ga->is_signed = is_signed;
   }

   glxx_unlock_server_state();
}

void glintAttribEnable(GLXX_SERVER_STATE_T *state, uint32_t indx, bool enabled)
{
   if (attrib_index_valid(state, indx)) {
      GLXX_VAO_T *vao = state->vao.bound;
      vao->attrib_config[indx].enabled = enabled;
   }
}

GL_API void GL_APIENTRY glPixelStorei(GLenum pname, GLint param)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if ((pname != GL_PACK_ALIGNMENT && pname != GL_UNPACK_ALIGNMENT) && IS_GL_11(state))
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else if ((pname == GL_PACK_ALIGNMENT || pname == GL_UNPACK_ALIGNMENT) && (!glxx_is_alignment(param)))
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (param < 0)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else
   {
      GLXX_PIXEL_STORE_STATE_T *pb = &(state->pixel_store_state);
      switch (pname)
      {
      case GL_PACK_ALIGNMENT:
         pb->pack.alignment = param;
         break;
      case GL_PACK_ROW_LENGTH:
         pb->pack.row_length = (uint32_t)param;
         break;
      case GL_PACK_SKIP_ROWS:
         pb->pack.skip_rows = (uint32_t)param;
         break;
      case GL_PACK_SKIP_PIXELS:
         pb->pack.skip_pixels = (uint32_t)param;
         break;
      case GL_UNPACK_ALIGNMENT:
         pb->unpack.alignment = param;
         break;
      case GL_UNPACK_ROW_LENGTH:
         pb->unpack.row_length = (uint32_t)param;
         break;
      case GL_UNPACK_SKIP_ROWS:
         pb->unpack.skip_rows = (uint32_t)param;
         break;
      case GL_UNPACK_SKIP_PIXELS:
         pb->unpack.skip_pixels = (uint32_t)param;
         break;
      case GL_UNPACK_IMAGE_HEIGHT:
         pb->unpack_image_height = (uint32_t)param;
         break;
      case GL_UNPACK_SKIP_IMAGES:
         pb->unpack_skip_images = (uint32_t)param;
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
   }

   glxx_unlock_server_state();
}

/*
   "It is possible to attach the ambient and diffuse material properties to the current
color, so that they continuously track its component values.
Color material tracking is enabled and disabled by calling Enable or Disable
with the symbolic value COLOR MATERIAL. When enabled, both the ambient (acm)
and diffuse (dcm) properties of both the front and back material are immediately
set to the value of the current color, and will track changes to the current color
resulting from either the Color commands or drawing vertex arrays with the color
array enabled.
The replacements made to material properties are permanent; the replaced values
remain until changed by either sending a new color or by setting a new material
value when COLOR MATERIAL is not currently enabled, to override that particular
value."

   TODO: it is irritating that we have to do it this way
*/

void glintColor(float red, float green, float blue, float alpha)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   state->generic_attrib[GL11_IX_COLOR].f[0] = red;
   state->generic_attrib[GL11_IX_COLOR].f[1] = green;
   state->generic_attrib[GL11_IX_COLOR].f[2] = blue;
   state->generic_attrib[GL11_IX_COLOR].f[3] = alpha;

   glxx_update_color_material(state);

   glxx_unlock_server_state();
}

void *glintAttribGetPointer(GLXX_SERVER_STATE_T *state, uint32_t indx)
{
   assert(indx < V3D_MAX_ATTR_ARRAYS);
   return (void *)state->vao.bound->attrib_config[indx].pointer;
}

/* Spec: s is masked to the number of bitplanes in the stencil buffer.  */
GL_API void GL_APIENTRY glClearStencil(GLint s)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   if (!state)
      return;

   state->clear.stencil_value = s & ((1 << glxx_get_stencil_size(state)) - 1);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   state->color_write = GLXX_SERVER_COLOR_WRITE_RED_BITS *
      ((r ? 0x1 : 0) | (g ? 0x2 : 0) | (b ? 0x4 : 0) | (a ? 0x8 : 0));
   state->dirty.color_write = KHRN_RENDER_STATE_SET_ALL;

   glxx_unlock_server_state();
}

#if V3D_VER_AT_LEAST(4,1,34,0)

static void color_mask_i(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (buf >= V3D_MAX_RENDER_TARGETS)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else
   {
      uint32_t new_color_write = (state->color_write & ~(0xf << (buf * 4))) |
         (((r ? 0x1 : 0) | (g ? 0x2 : 0) | (b ? 0x4 : 0) | (a ? 0x8 : 0)) << (buf * 4));
      if (state->color_write != new_color_write)
      {
         state->color_write = new_color_write;
         state->dirty.color_write = KHRN_RENDER_STATE_SET_ALL;
      }
   }

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glColorMaskiEXT(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
   color_mask_i(buf, r, g, b, a);
}

GL_API void GL_APIENTRY glColorMaskiOES(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
   color_mask_i(buf, r, g, b, a);
}

#endif

#if KHRN_GLES32_DRIVER

GL_API void GL_APIENTRY glColorMaski(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   color_mask_i(buf, r, g, b, a);
#endif
}

#endif

static GLboolean is_cull_face(GLenum mode)
{
   return mode == GL_FRONT ||
          mode == GL_BACK ||
          mode == GL_FRONT_AND_BACK;
}

GL_API void GL_APIENTRY glCullFace(GLenum mode)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (is_cull_face(mode))
   {
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
      state->cull_mode = mode;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}


/*
   glxx_update_viewport_internal(GLXX_SERVER_STATE_T *state)

   Fills in viewport internal so that install_uniforms gets the cached values
*/
static void glxx_update_viewport_internal(GLXX_SERVER_STATE_T *state)
{
   /* Builtin uniforms */
   state->vp_internal.dr_diff = state->depth_range.z_far - state->depth_range.z_near;

   /* Ones that the hardware always requires */
   state->vp_internal.xscale = 128.0f * (float)state->viewport.width;
   state->vp_internal.yscale = 128.0f * (float)state->viewport.height;
   state->vp_internal.zscale = (state->depth_range.z_far - state->depth_range.z_near) / 2.0f;
   state->vp_internal.zoffset = (state->depth_range.z_far + state->depth_range.z_near) / 2.0f;
}

/*
   glxx_server_state_set_error()

   set GL server-side error state if none already set
*/

void glxx_server_state_set_error_ex(GLXX_SERVER_STATE_T *state, GLenum error, const char *func, const char *file, int line)
{
   glxx_debug_log_error(&state->khr_debug, error, func, file, line);

   if (state->error == GL_NO_ERROR)
      state->error = error;
}

/*
   glViewport (GLint x, GLint y, GLsizei width, GLsizei height)

   Implementation notes:

   We check that width and height are non-negative, update the x, y, width and height
   elements of state.viewport, potentially violating the invariant and call
   glxx_update_viewport_internal() to restore it.

   Invariants preserved:

   state.vp_internal is consistent with other elements according to glxx_update_viewport_internal() docs
   elements of state.vp_internal are valid
*/

GL_API void GL_APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (width < 0 || height < 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   state->dirty.viewport = KHRN_RENDER_STATE_SET_ALL;
   state->viewport.x = x;
   state->viewport.y = y;
   state->viewport.width = gfx_smin(width, GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE);
   state->viewport.height = gfx_smin(height, GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE);

   glxx_update_viewport_internal(state);

end:
   glxx_unlock_server_state();
}

void glxx_server_invalidate_for_render_state(GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs)
{
   khrn_render_state_set_add(&state->dirty.blend_color, rs);
#if V3D_VER_AT_LEAST(4,1,34,0)
   khrn_render_state_set_add(&state->dirty.blend_enables, rs);
#endif
   khrn_render_state_set_add(&state->dirty.blend_cfg, rs);
   khrn_render_state_set_add(&state->dirty.color_write, rs);
   khrn_render_state_set_add(&state->dirty.cfg, rs);
   khrn_render_state_set_add(&state->dirty.linewidth, rs);
   khrn_render_state_set_add(&state->dirty.polygon_offset, rs);
   khrn_render_state_set_add(&state->dirty.viewport, rs);
   khrn_render_state_set_add(&state->dirty.sample_state, rs);
   khrn_render_state_set_add(&state->dirty.stuff, rs);
   khrn_render_state_set_add(&state->dirty.stencil, rs);
}

bool glxx_server_state_add_fence_to_depend_on(GLXX_SERVER_STATE_T *state, const khrn_fence *fence)
{
   khrn_fence *new_fence;

   new_fence = khrn_fence_dup(state->fences.fence_to_depend_on);
   if (!new_fence)
      return false;

   khrn_fence_refdec(state->fences.fence_to_depend_on);
   state->fences.fence_to_depend_on = new_fence;

   return khrn_fence_merge(state->fences.fence_to_depend_on, fence);
}


GLXX_FRAMEBUFFER_T* glxx_server_get_bound_fb(const GLXX_SERVER_STATE_T *state,
      glxx_fb_target_t target)
{
   switch(target)
   {
   case GL_READ_FRAMEBUFFER:
      return state->bound_read_framebuffer;
   case GL_DRAW_FRAMEBUFFER:
   case GL_FRAMEBUFFER:
      return state->bound_draw_framebuffer;
   default:
      unreachable();
      return NULL;
   }
}

GL20_PROGRAM_T *glxx_server_get_active_program(GLXX_SERVER_STATE_T *state)
{
   if (state->current_program != NULL)
      return state->current_program;

   if (state->pipelines.bound != NULL)
      return glxx_pipeline_get_active_program(state);

   return NULL;
}

#if V3D_VER_AT_LEAST(3,3,0,0)
static void primitive_bounding_box(float minX, float minY, float minZ, float minW,
                                   float maxX, float maxY, float maxZ, float maxW)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   float min[4] = { minX, minY, minZ, minW };
   float max[4] = { maxX, maxY, maxZ, maxW };
   for (int i=0; i<4; i++) {
      state->primitive_bounding_box.min[i] = min[i];
      state->primitive_bounding_box.max[i] = max[i];
   }

   glxx_unlock_server_state();
}
#endif

#if KHRN_GLES32_DRIVER
GL_API void GL_APIENTRY glPrimitiveBoundingBox(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW,
                                               GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)
{
   primitive_bounding_box(minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
}
#endif
#if V3D_VER_AT_LEAST(3,3,0,0)
GL_API void GL_APIENTRY glPrimitiveBoundingBoxEXT(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW,
                                                  GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)
{
   primitive_bounding_box(minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
}
GL_API void GL_APIENTRY glPrimitiveBoundingBoxOES(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW,
                                                  GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)
{
   primitive_bounding_box(minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
}
#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
static void min_sample_shading(float value) {
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   state->sample_shading_fraction = gfx_fclamp(value, 0.0f, 1.0f);

   glxx_unlock_server_state();
}
#endif

#if KHRN_GLES32_DRIVER
GL_API void GL_APIENTRY glMinSampleShading(GLfloat value) {
   min_sample_shading(value);
}
#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
GL_API void GL_APIENTRY glMinSampleShadingOES(GLfloat value) {
   min_sample_shading(value);
}
#endif

bool glxx_get_pointerv(GLXX_SERVER_STATE_T *state, GLenum pname, void **params)
{
   switch (pname)
   {
   case GL_DEBUG_CALLBACK_FUNCTION_KHR:
      params[0] = state->khr_debug.callback;
      return true;
   case GL_DEBUG_CALLBACK_USER_PARAM_KHR:
      params[0] = (GLvoid*)state->khr_debug.user_param;
      return true;
   default:
      return false;
   }
}

GL_API void GL_APIENTRY glGetPointerv(GLenum pname, GLvoid **params)
{
   uint32_t validApis = OPENGL_ES_11;
#if KHRN_GLES32_DRIVER
   validApis = OPENGL_ES_ANY; // Valid in ES1.1 and also after ES3.2
#endif

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(validApis);
   if (!state) return;

   bool found = false;

   if (IS_GL_11(state))
      found = gl11_get_pointerv(state, pname, params);
   else
      found = glxx_get_pointerv(state, pname, params);

   if (!found)
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

static GLenum glxx_get_graphics_reset_status(void)
{
   uint32_t validApis = OPENGL_ES_ANY;
   GLenum error = GL_NO_ERROR;

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged_even_if_reset(validApis);

   if (!state)
      return GL_NO_ERROR;

   if (egl_context_gl_notification(NULL))
   {
      bool gpu_aborted = vcos_atomic_load_bool(state->shared->gpu_aborted, VCOS_MEMORY_ORDER_RELAXED);
      if (gpu_aborted && !egl_context_gl_reset_notified())
      {
         /* only returns the error once then the application knows
          * that the reset is finished and the contexts can be deleted
          */
         egl_context_gl_set_reset_notified();
         state->error = GL_CONTEXT_LOST;
         error = GL_UNKNOWN_CONTEXT_RESET_EXT;
      }
   }

   glxx_unlock_server_state();
   return error;
}

GL_APICALL GLenum GL_APIENTRY glGetGraphicsResetStatusEXT(void)
{
   return glxx_get_graphics_reset_status();
}

GL_APICALL GLenum GL_APIENTRY glGetGraphicsResetStatusKHR(void)
{
   return glxx_get_graphics_reset_status();
}

#if KHRN_GLES32_DRIVER
GL_API GLenum GL_APIENTRY glGetGraphicsResetStatus(void)
{
   return glxx_get_graphics_reset_status();
}
#endif
