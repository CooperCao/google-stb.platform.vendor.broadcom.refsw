/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "gl_public_api.h"
#include "../common/khrn_int_common.h"
#include "glxx_shared.h"

#include "glxx_server.h"
#include "glxx_server_internal.h"
#include "glxx_server_pipeline.h"

#include "glxx_texture.h"
#include "glxx_buffer.h"
#include "../common/khrn_int_util.h"
#include "../common/khrn_resource.h"
#include "../common/khrn_process.h"

#include "../gl11/gl11_int_config.h"

#include "glxx_hw.h"
#include "../gl20/gl20_program.h"
#include "glxx_renderbuffer.h"
#include "glxx_framebuffer.h"
#include "glxx_translate.h"

#include <string.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "vcos.h"

#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"
#include "libs/util/gfx_util/gfx_util_conv.h"

static uint32_t component_bits(GLXX_FRAMEBUFFER_T *fbo, GLenum component);

static int32_t norm_float_to_int(float f)
{
   int32_t r = (int32_t)floor((4294967295.0f * f - 1.0f) / 2.0f + 0.5f);
   return r >= 0 ? r : 0x7fffffff;
}

#define MAX_GET_ITEM_COUNT 64

GL_API GLenum GL_APIENTRY glGetError(void)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   GLenum result;
   if (!state) return 0;

   result = state->error;

   state->error = GL_NO_ERROR;

   glxx_unlock_server_state();

   return result;
}

typedef enum glxx_get_type
{
   glxx_get_invalid_enum_type,
   glxx_get_invalid_index_type,
   glxx_get_bool_type,
   glxx_get_int_type,
   glxx_get_uint_type,
   glxx_get_int64_type,
   glxx_get_norm_float_type,  // 0.0 to 1.0
   glxx_get_float_type,
} glxx_get_type;

typedef enum glxx_get_type_count
{
   glxx_get_type_count_force_32bit = 0x7fffffff,
   glxx_get_type_shift = 28,

   glxx_get_invalid_enum_0 = glxx_get_invalid_enum_type << glxx_get_type_shift,
   glxx_get_invalid_index_0 = glxx_get_invalid_index_type << glxx_get_type_shift,
   glxx_get_bool_0 = glxx_get_bool_type << glxx_get_type_shift,
   glxx_get_bool_1,
   glxx_get_int_0 = glxx_get_int_type << glxx_get_type_shift,
   glxx_get_int_1,
   glxx_get_uint_0 = glxx_get_uint_type << glxx_get_type_shift,
   glxx_get_uint_1,
   glxx_get_int64_0 = glxx_get_int64_type << glxx_get_type_shift,
   glxx_get_int64_1,
   glxx_get_norm_float_0 = glxx_get_norm_float_type << glxx_get_type_shift,
   glxx_get_norm_float_1,
   glxx_get_float_0 = glxx_get_float_type << glxx_get_type_shift,
   glxx_get_float_1,
}
glxx_get_type_count;

static glxx_get_type_count glxx_get_params_and_type_common(
   GLXX_SERVER_STATE_T const* state,
   GLenum pname,
   GLboolean* booleans,
   GLint* ints,
   GLuint* uints,
   GLfloat* floats)
{
   switch (pname)
   {
   case GL_SAMPLE_COVERAGE_INVERT:
      booleans[0] = state->sample_coverage.invert;
      return glxx_get_bool_1;
#if V3D_VER_AT_LEAST(4,0,2,0) || KHRN_GLES32_DRIVER
   case GL_SAMPLE_SHADING:
      booleans[0] = state->caps.sample_shading;
      return glxx_get_bool_1;
   case GL_MIN_SAMPLE_SHADING_VALUE:
      floats[0] = state->sample_shading_fraction;
      return glxx_get_float_1;
#endif
   case GL_COLOR_WRITEMASK:
      /* Non-indexed glGet functions query the color write mask of buffer 0 */
      for (unsigned i = 0; i != 4; ++i)
         booleans[i] = (state->color_write >> i) & 1;
      return glxx_get_bool_0 + 4;
   case GL_DEPTH_WRITEMASK:
      booleans[0] = state->depth_mask;
      return glxx_get_bool_1;
   case GL_CULL_FACE:
      booleans[0] = state->caps.cull_face;
      return glxx_get_bool_1;
   case GL_POLYGON_OFFSET_FILL:
      booleans[0] = state->caps.polygon_offset_fill;
      return glxx_get_bool_1;
   case GL_SAMPLE_ALPHA_TO_COVERAGE:
      booleans[0] = !!(state->statebits.backend & GLSL_SAMPLE_ALPHA);
      return glxx_get_bool_1;
   case GL_SAMPLE_COVERAGE:
      booleans[0] = state->sample_coverage.enable;
      return glxx_get_bool_1;
   case GL_SCISSOR_TEST:
      booleans[0] = state->caps.scissor_test;
      return glxx_get_bool_1;
   case GL_STENCIL_TEST:
      booleans[0] = state->caps.stencil_test;
      return glxx_get_bool_1;
   case GL_DEPTH_TEST:
      booleans[0] = state->caps.depth_test;
      return glxx_get_bool_1;
   case GL_BLEND:
#if V3D_VER_AT_LEAST(4,0,2,0)
      /* Non-indexed glGet functions query the blend enable for buffer 0 */
      booleans[0] = state->blend.rt_enables & 1;
#else
      booleans[0] = state->blend.enable;
#endif
      return glxx_get_bool_1;
   case GL_DITHER:
      booleans[0] = state->caps.dither;
      return glxx_get_bool_1;
   case GL_DEBUG_OUTPUT_KHR:
      booleans[0] = state->khr_debug.debug_output;
      return glxx_get_bool_1;
   case GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR:
      booleans[0] = state->khr_debug.debug_output_synchronous;
      return glxx_get_bool_1;
   case GL_VIEWPORT:
      ints[0] = state->viewport.x;
      ints[1] = state->viewport.y;
      ints[2] = state->viewport.width;
      ints[3] = state->viewport.height;
      return glxx_get_int_0 + 4;
   case GL_SCISSOR_BOX:
      ints[0] = state->scissor.x;
      ints[1] = state->scissor.y;
      ints[2] = state->scissor.width;
      ints[3] = state->scissor.height;
      return glxx_get_int_0 + 4;
   case GL_STENCIL_CLEAR_VALUE:
      ints[0] = state->clear.stencil_value;
      return glxx_get_int_1;
   case GL_STENCIL_REF:
      ints[0] = gfx_sclamp(state->stencil_func.front.ref, 0, (1 << glxx_get_stencil_size(state)) - 1);
      return glxx_get_int_1;
   case GL_CULL_FACE_MODE:
      uints[0] = state->cull_mode;
      return glxx_get_uint_1;
   case GL_FRONT_FACE:
      uints[0] = state->front_face;
      return glxx_get_uint_1;
   case GL_TEXTURE_BINDING_2D:
      uints[0] = glxx_server_get_active_texture(state, GL_TEXTURE_2D)->name;
      return glxx_get_uint_1;
   case GL_TEXTURE_BINDING_EXTERNAL_OES:
      uints[0] = glxx_server_get_active_texture(state, GL_TEXTURE_EXTERNAL_OES)->name;
      return glxx_get_uint_1;
   case GL_ACTIVE_TEXTURE:
      uints[0] = state->active_texture;
      return glxx_get_uint_1;
   case GL_STENCIL_FUNC:
      uints[0] = state->stencil_func.front.func;
      return glxx_get_uint_1;
   case GL_STENCIL_WRITEMASK:
      uints[0] = state->stencil_mask.front;
      return glxx_get_uint_1;
   case GL_STENCIL_VALUE_MASK:
      uints[0] = state->stencil_func.front.mask;
      return glxx_get_uint_1;
   case GL_STENCIL_FAIL:
      uints[0] = state->stencil_op.front.fail;
      return glxx_get_uint_1;
   case GL_STENCIL_PASS_DEPTH_FAIL:
      uints[0] = state->stencil_op.front.zfail;
      return glxx_get_uint_1;
   case GL_STENCIL_PASS_DEPTH_PASS:
      uints[0] = state->stencil_op.front.zpass;
      return glxx_get_uint_1;
   case GL_DEPTH_FUNC:
      uints[0] = state->depth_func;
      return glxx_get_uint_1;
   case GL_GENERATE_MIPMAP_HINT:
      uints[0] = state->hints.generate_mipmap;
      return glxx_get_uint_1;
   case GL_SUBPIXEL_BITS:
      uints[0] = GLXX_CONFIG_SUBPIXEL_BITS;
      return glxx_get_uint_1;
   case GL_MAX_TEXTURE_SIZE:
      uints[0] = MAX_TEXTURE_SIZE;
      return glxx_get_uint_1;
   case GL_MAX_VIEWPORT_DIMS:
      uints[0] = GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE;
      uints[1] = GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE;
      return glxx_get_uint_0 + 2;
   case GL_SAMPLE_BUFFERS:
      uints[0] = glxx_fb_get_ms_mode(state->bound_draw_framebuffer) == GLXX_NO_MS ? 0 : 1;
      return glxx_get_uint_1;
   case GL_SAMPLES:
      uints[0] = (int)(glxx_fb_get_ms_mode(state->bound_draw_framebuffer));
      return glxx_get_uint_1;
   case GL_COMPRESSED_TEXTURE_FORMATS:
   {
      unsigned num = gfx_compressed_format_enumerate(uints, khrn_get_lfmt_translate_exts());
      assert(num <= MAX_GET_ITEM_COUNT);
      return glxx_get_uint_0 + num;
   }
   case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
      uints[0] = gfx_compressed_format_enumerate(NULL, khrn_get_lfmt_translate_exts());
      return glxx_get_uint_1;
   case GL_RED_BITS:
   case GL_GREEN_BITS:
   case GL_BLUE_BITS:
   case GL_ALPHA_BITS:
   case GL_DEPTH_BITS:
   case GL_STENCIL_BITS:
      uints[0] = component_bits(state->bound_draw_framebuffer, pname);
      return glxx_get_uint_1;
   case GL_FRAMEBUFFER_BINDING:
      // GL_DRAW_FRAMEBUFFER_BINDING is defined to be GL_FRAMEBUFFER_BINDING
      uints[0] = state->bound_draw_framebuffer->name;
      return glxx_get_uint_1;
    case GL_RENDERBUFFER_BINDING:
      uints[0] = state->bound_renderbuffer != NULL ? state->bound_renderbuffer->name : 0u;
      return glxx_get_uint_1;
   case GL_MAX_RENDERBUFFER_SIZE:
      uints[0] = GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE;
      return glxx_get_uint_1;
   case GL_ARRAY_BUFFER_BINDING:
      uints[0] = state->bound_buffer[GLXX_BUFTGT_ARRAY].obj != NULL ? state->bound_buffer[GLXX_BUFTGT_ARRAY].obj->name : 0u;
      return glxx_get_uint_1;
   case GL_ELEMENT_ARRAY_BUFFER_BINDING:
      uints[0] = state->vao.bound->element_array_binding.obj != NULL ? state->vao.bound->element_array_binding.obj->name : 0u;
      return glxx_get_uint_1;
   case GL_UNPACK_ALIGNMENT:
      uints[0] = state->pixel_store_state.unpack.alignment;
      return glxx_get_uint_1;
   case GL_UNPACK_ROW_LENGTH:
      uints[0] = state->pixel_store_state.unpack.row_length;
      return glxx_get_uint_1;
   case GL_UNPACK_SKIP_ROWS:
      uints[0] = state->pixel_store_state.unpack.skip_rows;
      return glxx_get_uint_1;
   case GL_UNPACK_SKIP_PIXELS:
      uints[0] = state->pixel_store_state.unpack.skip_pixels;
      return glxx_get_uint_1;
   case GL_UNPACK_SKIP_IMAGES:
      uints[0] = state->pixel_store_state.unpack_skip_images;
      return glxx_get_uint_1;
   case GL_UNPACK_IMAGE_HEIGHT:
      uints[0] = state->pixel_store_state.unpack_image_height;
      return glxx_get_uint_1;
   case GL_PACK_ALIGNMENT:
      uints[0] = state->pixel_store_state.pack.alignment;
      return glxx_get_uint_1;
   case GL_PACK_ROW_LENGTH:
      uints[0] = state->pixel_store_state.pack.row_length;
      return glxx_get_uint_1;
   case GL_PACK_SKIP_ROWS:
      uints[0] = state->pixel_store_state.pack.skip_rows;
      return glxx_get_uint_1;
   case GL_PACK_SKIP_PIXELS:
      uints[0] = state->pixel_store_state.pack.skip_pixels;
      return glxx_get_uint_1;
   case GL_MAX_DEBUG_MESSAGE_LENGTH_KHR:
      uints[0] = GLXX_CONFIG_MAX_DEBUG_MESSAGE_LENGTH;
      return glxx_get_uint_1;
   case GL_MAX_DEBUG_LOGGED_MESSAGES_KHR:
      uints[0] = GLXX_CONFIG_MAX_DEBUG_LOGGED_MESSAGES;
      return glxx_get_uint_1;
   case GL_MAX_DEBUG_GROUP_STACK_DEPTH_KHR:
      uints[0] = GLXX_CONFIG_MAX_DEBUG_GROUP_STACK_DEPTH;
      return glxx_get_uint_1;
   case GL_MAX_LABEL_LENGTH_KHR:
      uints[0] = GLXX_CONFIG_MAX_LABEL_LENGTH;
      return glxx_get_uint_1;
   case GL_DEBUG_LOGGED_MESSAGES_KHR:
      uints[0] = state->khr_debug.message_log.count;
      return glxx_get_uint_1;
   case GL_DEBUG_GROUP_STACK_DEPTH_KHR:
      uints[0] = state->khr_debug.active_stack_level + 1;
      return glxx_get_uint_1;
   case GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_KHR:
      uints[0] = 0;
      if (state->khr_debug.message_log.count > 0 &&
          state->khr_debug.message_log.head->message != NULL)
      {
         // Includes NULL terminator
         uints[0] = state->khr_debug.message_log.head->message_len + 1;
      }
      return glxx_get_uint_1;

   case GL_IMPLEMENTATION_COLOR_READ_TYPE:
   case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
   {
      GLenum format = GL_NONE;
      GLenum type = GL_NONE;

      const GLXX_ATTACHMENT_T *att = glxx_fb_get_read_buffer(state->bound_read_framebuffer);

      if (att)
      {
         GFX_LFMT_T api_fmt = glxx_attachment_get_api_fmt(att);

         if (api_fmt != GFX_LFMT_NONE)
         {
            GFX_LFMT_T dst_lfmt = GFX_LFMT_NONE;
            glxx_readpixels_impldefined_formats(&format, &type, &dst_lfmt, api_fmt);
         }
      }

      if (pname == GL_IMPLEMENTATION_COLOR_READ_TYPE)
         uints[0] = type;
      else
         uints[0] = format;
      return glxx_get_uint_1;
   }
   case GL_DEPTH_RANGE:
      floats[0] = state->depth_range.z_near;
      floats[1] = state->depth_range.z_far;
      return glxx_get_norm_float_0 + 2;
   case GL_COLOR_CLEAR_VALUE:
      for (unsigned i = 0; i < 4; i++)
         floats[i] = state->clear.color_value[i];
      return glxx_get_norm_float_0 + 4;
   case GL_DEPTH_CLEAR_VALUE:
      floats[0] = state->clear.depth_value;
      return glxx_get_norm_float_1;
   case GL_LINE_WIDTH:
      floats[0] = state->line_width;
      return glxx_get_float_1;
#if V3D_HAS_POLY_OFFSET_CLAMP
   case GL_POLYGON_OFFSET_CLAMP_EXT:
      floats[0] = state->polygon_offset.limit;
      return glxx_get_float_1;
#endif
   case GL_POLYGON_OFFSET_FACTOR:
      floats[0] = state->polygon_offset.factor;
      return glxx_get_float_1;
   case GL_POLYGON_OFFSET_UNITS:
      floats[0] = state->polygon_offset.units;
      return glxx_get_float_1;
   case GL_SAMPLE_COVERAGE_VALUE:
      floats[0] = state->sample_coverage.value;
      return glxx_get_float_1;

#if GL_EXT_texture_filter_anisotropic
   case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
      floats[0] = 16.0f;
      return glxx_get_float_1;
#endif
   case GL_CONTEXT_ROBUST_ACCESS_EXT:
      booleans[0] = egl_context_gl_robustness(state->context);
      return glxx_get_bool_1;
   case GL_RESET_NOTIFICATION_STRATEGY_EXT:
      uints[0] = egl_context_gl_notification(state->context) ?
         GL_LOSE_CONTEXT_ON_RESET_EXT : GL_NO_RESET_NOTIFICATION_EXT;
      return glxx_get_uint_1;
   case GL_CONTEXT_FLAGS:
      ints[0] = (egl_context_gl_secure(state->context) ? GL_CONTEXT_FLAG_PROTECTED_CONTENT_BIT_EXT : 0) |
                (egl_context_gl_robustness(state->context) ? GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT : 0)     |
                (egl_context_gl_debug(state->context)      ? GL_CONTEXT_FLAG_DEBUG_BIT         : 0);
      return glxx_get_int_1;

   default:
      return glxx_get_invalid_enum_0;
   }
}

/* Non-indexed glGet functions query the blend config of buffer 0 */
static const glxx_blend_cfg *blend_cfg_0(const GLXX_SERVER_STATE_T *state)
{
#if V3D_VER_AT_LEAST(4,0,2,0)
   return &state->blend.rt_cfgs[0];
#else
   return &state->blend.cfg;
#endif
}

static glxx_get_type_count glxx_get_params_and_type_gl11(
   GLXX_SERVER_STATE_T const* state,
   GLenum pname,
   GLboolean* booleans,
   GLint* ints,
   GLuint* uints,
   GLfloat* floats)
{
   static_assrt(GL11_CONFIG_MAX_PLANES == 1);
   static_assrt(GL11_CONFIG_MAX_LIGHTS == 8);
   assert(IS_GL_11(state));

   GLXX_VAO_T const* vao = state->vao.bound;
   assert(vao != NULL);

   switch (pname)
   {
   case GL_LIGHT_MODEL_TWO_SIDE:
      booleans[0] = !!(state->gl11.statebits.vertex & GL11_TWOSIDE);
      return glxx_get_bool_1;
   case GL_NORMALIZE:
      booleans[0] = !(state->gl11.statebits.vertex & GL11_NO_NORMALIZE);
      return glxx_get_bool_1;
   case GL_RESCALE_NORMAL:
      booleans[0] = !(state->gl11.statebits.v_enable & GL11_NO_NORMALIZE);
      return glxx_get_bool_1;
   case GL_CLIP_PLANE0:
      booleans[0] = !!(state->gl11.statebits.f_enable & GL11_UCLIP_M);
      return glxx_get_bool_1;
   case GL_FOG:
      booleans[0] = !!(state->gl11.statebits.f_enable & GL11_FOG_M);
      return glxx_get_bool_1;
   case GL_LIGHTING:
      booleans[0] = !!(state->gl11.statebits.v_enable2 & GL11_LIGHTING_M);
      return glxx_get_bool_1;
   case GL_COLOR_MATERIAL:
      booleans[0] = !!(state->gl11.statebits.vertex & GL11_COLORMAT);
      return glxx_get_bool_1;
   case GL_LIGHT0:
   case GL_LIGHT1:
   case GL_LIGHT2:
   case GL_LIGHT3:
   case GL_LIGHT4:
   case GL_LIGHT5:
   case GL_LIGHT6:
   case GL_LIGHT7:
      booleans[0] = !!(state->gl11.statebits.v_enable & (GL11_LIGHT_M << (pname - GL_LIGHT0)));
      return glxx_get_bool_1;
   case GL_POINT_SMOOTH:
      booleans[0] = !!(state->gl11.statebits.fragment & GL11_POINTSMOOTH);
      return glxx_get_bool_1;
   case GL_POINT_SPRITE_OES:
      booleans[0] = !!(state->gl11.point_sprite);
      return glxx_get_bool_1;
   case GL_LINE_SMOOTH:
      booleans[0] = !!(state->gl11.statebits.fragment & GL11_LINESMOOTH);
      return glxx_get_bool_1;
   case GL_MULTISAMPLE:
      booleans[0] = state->caps.multisample;
      return glxx_get_bool_1;
   case GL_SAMPLE_ALPHA_TO_ONE:
      booleans[0] = !!(state->gl11.statebits.fragment & GL11_SAMPLE_ONE);
      return glxx_get_bool_1;
   case GL_TEXTURE_2D:
      assert(state->active_texture - GL_TEXTURE0 < GL11_CONFIG_MAX_TEXTURE_UNITS);
      booleans[0] = state->gl11.texunits[state->active_texture - GL_TEXTURE0].target_enabled_2D;
      return glxx_get_bool_1;
   case GL_TEXTURE_EXTERNAL_OES:
      assert(state->active_texture - GL_TEXTURE0 < GL11_CONFIG_MAX_TEXTURE_UNITS);
      booleans[0] = state->gl11.texunits[state->active_texture - GL_TEXTURE0].target_enabled_EXTERNAL_OES;
      return glxx_get_bool_1;
   case GL_ALPHA_TEST:
      booleans[0] = !!(state->gl11.statebits.f_enable & GL11_AFUNC_M);
      return glxx_get_bool_1;
   case GL_COLOR_LOGIC_OP:
      booleans[0] = !!(state->gl11.statebits.f_enable & GL11_LOGIC_M);
      return glxx_get_bool_1;
   case GL_VERTEX_ARRAY:
      booleans[0] = vao->attrib_config[GL11_IX_VERTEX].enabled;
      return glxx_get_bool_1;
   case GL_NORMAL_ARRAY:
      booleans[0] = vao->attrib_config[GL11_IX_NORMAL].enabled;
      return glxx_get_bool_1;
   case GL_COLOR_ARRAY:
      booleans[0] = vao->attrib_config[GL11_IX_COLOR].enabled;
      return glxx_get_bool_1;
   case GL_TEXTURE_COORD_ARRAY:
      booleans[0] = vao->attrib_config[state->gl11.client_active_texture - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].enabled;
      return glxx_get_bool_1;
   case GL_POINT_SIZE_ARRAY_OES:
      booleans[0] = vao->attrib_config[GL11_IX_POINT_SIZE].enabled;
      return glxx_get_bool_1;
   case GL_MODELVIEW_MATRIX_FLOAT_AS_INT_BITS_OES:
      gl11_matrix_load((float*)ints, state->gl11.current_modelview);
      return glxx_get_int_0 + 16;
   case GL_PROJECTION_MATRIX_FLOAT_AS_INT_BITS_OES:
      gl11_matrix_load((float*)ints, state->gl11.current_projection);
      return glxx_get_int_0 + 16;
   case GL_TEXTURE_MATRIX_FLOAT_AS_INT_BITS_OES:
      assert(state->active_texture - GL_TEXTURE0 < GL11_CONFIG_MAX_TEXTURE_UNITS);
      gl11_matrix_load((float*)ints, state->gl11.texunits[state->active_texture - GL_TEXTURE0].current_matrix);
      return glxx_get_int_0 + 16;
   case GL_MODELVIEW_STACK_DEPTH:
      uints[0] = state->gl11.modelview.pos + 1;
      return glxx_get_uint_1;
   case GL_PROJECTION_STACK_DEPTH:
      uints[0] = state->gl11.projection.pos + 1;
      return glxx_get_uint_1;
   case GL_TEXTURE_STACK_DEPTH:
      uints[0] = state->gl11.texunits[state->active_texture - GL_TEXTURE0].stack.pos + 1;
      return glxx_get_uint_1;
   case GL_MATRIX_MODE:
      uints[0] = state->gl11.matrix_mode;
      return glxx_get_uint_1;
   case GL_FOG_MODE:
      uints[0] = untranslate_fog_mode((uint32_t) state->gl11.statebits.fragment & GL11_FOG_M);
      return glxx_get_uint_1;
   case GL_SHADE_MODEL:
      uints[0] = state->gl11.shade_model;
      return glxx_get_uint_1;
   case GL_ALPHA_TEST_FUNC:
      uints[0] = untranslate_alpha_func((uint32_t) state->gl11.statebits.fragment & GL11_AFUNC_M);
      return glxx_get_uint_1;
   case GL_BLEND_SRC:
      uints[0] = untranslate_blend_func(blend_cfg_0(state)->src_rgb);
      return glxx_get_uint_1;
   case GL_BLEND_DST:
      uints[0] = untranslate_blend_func(blend_cfg_0(state)->dst_rgb);
      return glxx_get_uint_1;
   case GL_LOGIC_OP_MODE:
      uints[0] = untranslate_logic_op(state->gl11.statebits.fragment & GL11_LOGIC_M);
      return glxx_get_uint_1;
   case GL_PERSPECTIVE_CORRECTION_HINT:
      uints[0] = state->gl11.hints.perspective_correction;
      return glxx_get_uint_1;
   case GL_POINT_SMOOTH_HINT:
      uints[0] = state->gl11.hints.point_smooth;
      return glxx_get_uint_1;
   case GL_LINE_SMOOTH_HINT:
      uints[0] = state->gl11.hints.line_smooth;
      return glxx_get_uint_1;
   case GL_FOG_HINT:
      uints[0] = state->gl11.hints_program.fog;
      return glxx_get_uint_1;
   case GL_MAX_LIGHTS:
      uints[0] = GL11_CONFIG_MAX_LIGHTS;
      return glxx_get_uint_1;
   case GL_MAX_CLIP_PLANES:
      uints[0] = GL11_CONFIG_MAX_PLANES;
      return glxx_get_uint_1;
   case GL_MAX_MODELVIEW_STACK_DEPTH:
   case GL_MAX_PROJECTION_STACK_DEPTH:
   case GL_MAX_TEXTURE_STACK_DEPTH:
      uints[0] = GL11_CONFIG_MAX_STACK_DEPTH;
      return glxx_get_uint_1;
   case GL_MAX_TEXTURE_UNITS:
      uints[0] = GL11_CONFIG_MAX_TEXTURE_UNITS;
      return glxx_get_uint_1;
   case GL_CLIENT_ACTIVE_TEXTURE:
      uints[0] = state->gl11.client_active_texture;
      return glxx_get_uint_1;
   case GL_VERTEX_ARRAY_SIZE:
      uints[0] = vao->attrib_config[GL11_IX_VERTEX].size;
      return glxx_get_uint_1;
   case GL_VERTEX_ARRAY_TYPE:
      uints[0] = vao->attrib_config[GL11_IX_VERTEX].gl_type;
      return glxx_get_uint_1;
   case GL_VERTEX_ARRAY_STRIDE:
      uints[0] = vao->attrib_config[GL11_IX_VERTEX].original_stride;
      return glxx_get_uint_1;
   case GL_NORMAL_ARRAY_TYPE:
      uints[0] = vao->attrib_config[GL11_IX_NORMAL].gl_type;
      return glxx_get_uint_1;
   case GL_NORMAL_ARRAY_STRIDE:
      uints[0] = vao->attrib_config[GL11_IX_NORMAL].original_stride;
      return glxx_get_uint_1;
   case GL_COLOR_ARRAY_SIZE:
      uints[0] = vao->attrib_config[GL11_IX_COLOR].size;
      return glxx_get_uint_1;
   case GL_COLOR_ARRAY_TYPE:
      uints[0] = vao->attrib_config[GL11_IX_COLOR].gl_type;
      return glxx_get_uint_1;
   case GL_COLOR_ARRAY_STRIDE:
      uints[0] = vao->attrib_config[GL11_IX_COLOR].original_stride;
      return glxx_get_uint_1;
   case GL_TEXTURE_COORD_ARRAY_SIZE:
      uints[0] = vao->attrib_config[state->gl11.client_active_texture - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].size;
      return glxx_get_uint_1;
   case GL_TEXTURE_COORD_ARRAY_TYPE:
      uints[0] = vao->attrib_config[state->gl11.client_active_texture - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].gl_type;
      return glxx_get_uint_1;
   case GL_TEXTURE_COORD_ARRAY_STRIDE:
      uints[0] = vao->attrib_config[state->gl11.client_active_texture - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].original_stride;
      return glxx_get_uint_1;
   case GL_POINT_SIZE_ARRAY_TYPE_OES:
      uints[0] = vao->attrib_config[GL11_IX_POINT_SIZE].gl_type;
      return glxx_get_uint_1;
   case GL_POINT_SIZE_ARRAY_STRIDE_OES:
      uints[0] = vao->attrib_config[GL11_IX_POINT_SIZE].original_stride;
      return glxx_get_uint_1;
   case GL_MATRIX_PALETTE_OES:
      booleans[0] = !!(state->gl11.statebits.v_enable & GL11_MPAL_M);
      return glxx_get_bool_1;
   case GL_MATRIX_INDEX_ARRAY_OES:
      booleans[0] = vao->attrib_config[GL11_IX_MATRIX_INDEX].enabled;
      return glxx_get_bool_1;
   case GL_WEIGHT_ARRAY_OES:
      booleans[0] = vao->attrib_config[GL11_IX_MATRIX_WEIGHT].enabled;
      return glxx_get_bool_1;
   case GL_MATRIX_INDEX_ARRAY_SIZE_OES:
      uints[0] = vao->attrib_config[GL11_IX_MATRIX_INDEX].size;
      return glxx_get_uint_1;
   case GL_MATRIX_INDEX_ARRAY_TYPE_OES:
      uints[0] = vao->attrib_config[GL11_IX_MATRIX_INDEX].gl_type;
      return glxx_get_uint_1;
   case GL_MATRIX_INDEX_ARRAY_STRIDE_OES:
      uints[0] = vao->attrib_config[GL11_IX_MATRIX_INDEX].original_stride;
      return glxx_get_uint_1;
   case GL_WEIGHT_ARRAY_SIZE_OES:
      uints[0] = vao->attrib_config[GL11_IX_MATRIX_WEIGHT].size;
      return glxx_get_uint_1;
   case GL_WEIGHT_ARRAY_TYPE_OES:
      uints[0] = vao->attrib_config[GL11_IX_MATRIX_WEIGHT].gl_type;
      return glxx_get_uint_1;
   case GL_WEIGHT_ARRAY_STRIDE_OES:
      uints[0] = vao->attrib_config[GL11_IX_MATRIX_WEIGHT].original_stride;
      return glxx_get_uint_1;
   case GL_MAX_PALETTE_MATRICES_OES:
      uints[0] = GL11_CONFIG_MAX_PALETTE_MATRICES_OES;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_UNITS_OES:
      uints[0] = GL11_CONFIG_MAX_VERTEX_UNITS_OES;
      return glxx_get_uint_1;
   case GL_CURRENT_PALETTE_MATRIX_OES:
      uints[0] = state->gl11.current_palette_matrix;
      return glxx_get_uint_1;
   case GL_VERTEX_ARRAY_BUFFER_BINDING:
   case GL_NORMAL_ARRAY_BUFFER_BINDING:
   case GL_COLOR_ARRAY_BUFFER_BINDING:
   case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
   case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
   case GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES:
   case GL_WEIGHT_ARRAY_BUFFER_BINDING_OES:
   {
      int arr;
      switch (pname)
      {
      case GL_VERTEX_ARRAY_BUFFER_BINDING:            arr = GL11_IX_VERTEX; break;
      case GL_NORMAL_ARRAY_BUFFER_BINDING:            arr = GL11_IX_NORMAL; break;
      case GL_COLOR_ARRAY_BUFFER_BINDING:             arr = GL11_IX_COLOR; break;
      case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:     arr = GL11_IX_TEXTURE_COORD + (state->gl11.client_active_texture - GL_TEXTURE0); break;
      case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:    arr = GL11_IX_POINT_SIZE; break;
      case GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES:  arr = GL11_IX_MATRIX_INDEX; break;
      case GL_WEIGHT_ARRAY_BUFFER_BINDING_OES:        arr = GL11_IX_MATRIX_WEIGHT; break;
      default: unreachable();
      }
      GLXX_VBO_BINDING_T const* vbo = &vao->vbos[vao->attrib_config[arr].vbo_index];
      uints[0] = vbo->buffer != NULL ? vbo->buffer->name : 0;
      return glxx_get_uint_1;
   }
   case GL_FOG_COLOR:
      for (unsigned i = 0; i < 4; i++)
         floats[i] = state->gl11.fog.color[i];
      return glxx_get_norm_float_0 + 4;
   case GL_LIGHT_MODEL_AMBIENT:
      for (unsigned i = 0; i < 4; i++)
         floats[i] = state->gl11.lightmodel.ambient[i];
      return glxx_get_norm_float_0 + 4;
   case GL_ALPHA_TEST_REF:
      floats[0] = state->gl11.alpha_func.ref;
      return glxx_get_norm_float_1;
   case GL_CURRENT_COLOR:
      for (unsigned i = 0; i < 4; i++)
         floats[i] = state->generic_attrib[GL11_IX_COLOR].f[i];
      return glxx_get_norm_float_0 + 4;
   case GL_CURRENT_NORMAL:
      for (unsigned  i = 0; i < 3; i++)
         floats[i] = state->generic_attrib[GL11_IX_NORMAL].f[i];
      return glxx_get_norm_float_0 + 3;

   case GL_MODELVIEW_MATRIX:
      gl11_matrix_load(floats, state->gl11.current_modelview);
      return glxx_get_float_0 + 16;
   case GL_PROJECTION_MATRIX:
      gl11_matrix_load(floats, state->gl11.current_projection);
      return glxx_get_float_0 + 16;
   case GL_TEXTURE_MATRIX:
      assert(state->active_texture - GL_TEXTURE0 < GL11_CONFIG_MAX_TEXTURE_UNITS);
      gl11_matrix_load(floats, state->gl11.texunits[state->active_texture - GL_TEXTURE0].current_matrix);
      return glxx_get_float_0 + 16;
   case GL_FOG_DENSITY:
      floats[0] = state->gl11.fog.density;
      return glxx_get_float_1;
   case GL_FOG_START:
      floats[0] = state->gl11.fog.start;
      return glxx_get_float_1;
   case GL_FOG_END:
      floats[0] = state->gl11.fog.end;
      return glxx_get_float_1;
   case GL_POINT_SIZE_MIN:
      floats[0] = state->gl11.point_params.size_min;
      return glxx_get_float_1;
   case GL_POINT_SIZE_MAX:
      floats[0] = state->gl11.point_params.size_max;
      return glxx_get_float_1;
   case GL_POINT_FADE_THRESHOLD_SIZE:
      floats[0] = state->gl11.point_params.fade_threshold;
      return glxx_get_float_1;
   case GL_POINT_DISTANCE_ATTENUATION:
      for (unsigned i = 0; i < 3; i++)
         floats[i] = state->gl11.point_params.distance_attenuation[i];
      return glxx_get_float_0 + 3;
   case GL_SMOOTH_POINT_SIZE_RANGE:
      floats[0] = GL11_CONFIG_MIN_SMOOTH_POINT_SIZE;
      floats[1] = GL11_CONFIG_MAX_SMOOTH_POINT_SIZE;
      return glxx_get_float_0 + 2;
   case GL_SMOOTH_LINE_WIDTH_RANGE:
      floats[0] = GL11_CONFIG_MIN_SMOOTH_LINE_WIDTH;
      floats[1] = GL11_CONFIG_MAX_SMOOTH_LINE_WIDTH;
      return glxx_get_float_0 + 2;
   case GL_CURRENT_TEXTURE_COORDS:
      /* apparently we need the current texture coordinates for the _server_ active texture unit */
      for (unsigned i = 0; i < 4; i++)
         floats[i] = state->generic_attrib[state->active_texture - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].f[i];
      return glxx_get_float_0 + 4;
   case GL_POINT_SIZE:
      floats[0] = state->generic_attrib[GL11_IX_POINT_SIZE].f[0];
      return glxx_get_float_1;
   case GL_ALIASED_POINT_SIZE_RANGE:
      floats[0] = GL11_CONFIG_MIN_ALIASED_POINT_SIZE;
      floats[1] = GL11_CONFIG_MAX_ALIASED_POINT_SIZE;
      return glxx_get_float_0 + 2;
   case GL_ALIASED_LINE_WIDTH_RANGE:
      floats[0] = GL11_CONFIG_MIN_ALIASED_LINE_WIDTH;
      floats[1] = GL11_CONFIG_MAX_ALIASED_LINE_WIDTH;
      return glxx_get_float_0 + 2;
   default:
      return glxx_get_invalid_enum_0;
   }
}

static glxx_get_type_count glxx_get_params_and_type_gl3x(
   GLXX_SERVER_STATE_T const* state,
   GLenum pname,
   GLboolean* booleans,
   GLint* ints,
   GLuint* uints,
   GLfloat* floats,
   GLint64* int64s)
{
   assert(!IS_GL_11(state));

   switch (pname)
   {
   case GL_SHADER_COMPILER:
      booleans[0] = GL_TRUE;
      return glxx_get_bool_1;
   case GL_PRIMITIVE_RESTART_FIXED_INDEX:
      booleans[0] = state->caps.primitive_restart;
      return glxx_get_bool_1;
   case GL_TRANSFORM_FEEDBACK_ACTIVE:
      booleans[0] = glxx_tf_is_active(glxx_get_bound_tf(state));
      return glxx_get_bool_1;
   case GL_TRANSFORM_FEEDBACK_PAUSED:
      booleans[0] = glxx_tf_is_paused(glxx_get_bound_tf(state));
      return glxx_get_bool_1;
   case GL_RASTERIZER_DISCARD:
      booleans[0] = state->caps.rasterizer_discard;
      return glxx_get_bool_1;
   case GL_STENCIL_BACK_REF:
      ints[0] = gfx_sclamp(state->stencil_func.back.ref, 0, (1 << glxx_get_stencil_size(state)) - 1);
      return glxx_get_int_1;
   case GL_MIN_PROGRAM_TEXEL_OFFSET:
      ints[0] = GLXX_CONFIG_MIN_TEXEL_OFFSET;
      return glxx_get_int_1;
   case GL_MAX_PROGRAM_TEXEL_OFFSET:
      ints[0] = GLXX_CONFIG_MAX_TEXEL_OFFSET;
      return glxx_get_int_1;
   case GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET:
      ints[0] = GLXX_CONFIG_MAX_TEXEL_OFFSET;
      return glxx_get_int_1;
   case GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET:
      ints[0] = GLXX_CONFIG_MIN_TEXEL_OFFSET;
      return glxx_get_int_1;
   case GL_TEXTURE_BINDING_CUBE_MAP:
      uints[0] = glxx_server_get_active_texture(state, GL_TEXTURE_CUBE_MAP)->name;
      return glxx_get_uint_1;
   case GL_TEXTURE_BINDING_CUBE_MAP_ARRAY:
      uints[0] = glxx_server_get_active_texture(state, GL_TEXTURE_CUBE_MAP_ARRAY)->name;
      return glxx_get_uint_1;
   case GL_STENCIL_BACK_WRITEMASK:
      uints[0] = state->stencil_mask.back;
      return glxx_get_uint_1;
   case GL_STENCIL_BACK_FUNC:
      uints[0] = state->stencil_func.back.func;
      return glxx_get_uint_1;
   case GL_STENCIL_BACK_VALUE_MASK:
      uints[0] = state->stencil_func.back.mask;
      return glxx_get_uint_1;
   case GL_STENCIL_BACK_FAIL:
      uints[0] = state->stencil_op.back.fail;
      return glxx_get_uint_1;
   case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
      uints[0] = state->stencil_op.back.zfail;
      return glxx_get_uint_1;
   case GL_STENCIL_BACK_PASS_DEPTH_PASS:
      uints[0] = state->stencil_op.back.zpass;
      return glxx_get_uint_1;
   case GL_BLEND_SRC_RGB:
      uints[0] = untranslate_blend_func(blend_cfg_0(state)->src_rgb);
      return glxx_get_uint_1;
   case GL_BLEND_SRC_ALPHA:
      uints[0] = untranslate_blend_func(blend_cfg_0(state)->src_alpha);
      return glxx_get_uint_1;
   case GL_BLEND_DST_RGB:
      uints[0] = untranslate_blend_func(blend_cfg_0(state)->dst_rgb);
      return glxx_get_uint_1;
   case GL_BLEND_DST_ALPHA:
      uints[0] = untranslate_blend_func(blend_cfg_0(state)->dst_alpha);
      return glxx_get_uint_1;
   case GL_BLEND_EQUATION_RGB:
      uints[0] = untranslate_blend_equation(blend_cfg_0(state)->color_eqn);
      return glxx_get_uint_1;
   case GL_BLEND_EQUATION_ALPHA:
      uints[0] = untranslate_blend_equation(blend_cfg_0(state)->alpha_eqn);
      return glxx_get_uint_1;
   case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
      uints[0] = MAX_TEXTURE_SIZE;
      return glxx_get_uint_1;
   case GL_SHADER_BINARY_FORMATS:
      return glxx_get_uint_0 + 0;  // No binary formats supported
   case GL_NUM_SHADER_BINARY_FORMATS:
      uints[0] = 0;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_ATTRIBS:
      uints[0] = GLXX_CONFIG_MAX_VERTEX_ATTRIBS;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
   case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_UNIFORM_SCALARS;
      return glxx_get_uint_1;
   case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
   case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
#if KHRN_GLES31_DRIVER
   case GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS:
#endif
      uints[0] = GLXX_CONFIG_MAX_UNIFORM_SCALARS + GLXX_CONFIG_MAX_SHADER_UNIFORM_BLOCKS * GLXX_CONFIG_MAX_UNIFORM_BLOCK_SIZE / 4;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_UNIFORM_VECTORS:
   case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
      uints[0] = GLXX_CONFIG_MAX_UNIFORM_VECTORS;
      return glxx_get_uint_1;
   case GL_MAX_VARYING_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_VARYING_SCALARS;
      return glxx_get_uint_1;
   case GL_MAX_VARYING_VECTORS:
      uints[0] = GLXX_CONFIG_MAX_VARYING_VECTORS;
      return glxx_get_uint_1;
   case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
      uints[0] = GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
   case GL_MAX_TEXTURE_IMAGE_UNITS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_TEXTURE_IMAGE_UNITS;
      return glxx_get_uint_1;
   case GL_CURRENT_PROGRAM:
      uints[0] = state->current_program ? state->current_program->name : 0;
      return glxx_get_uint_1;
   case GL_MAX_ELEMENTS_VERTICES:
      uints[0] = GLXX_CONFIG_RECOMMENDED_ELEMENTS_VERTICES;
      return glxx_get_uint_1;
   case GL_MAX_ELEMENTS_INDICES:
      uints[0] = GLXX_CONFIG_RECOMMENDED_ELEMENTS_INDICES;
      return glxx_get_uint_1;
   case GL_MAX_ELEMENT_INDEX:
      uints[0] = GLXX_CONFIG_MAX_ELEMENT_INDEX;
      return glxx_get_uint_1;
   case GL_MAX_DRAW_BUFFERS:
      uints[0] = GLXX_MAX_RENDER_TARGETS;
      return glxx_get_uint_1;
   case GL_TEXTURE_BINDING_3D:
      uints[0] = glxx_server_get_active_texture(state, GL_TEXTURE_3D)->name;
      return glxx_get_uint_1;
   case GL_TEXTURE_BINDING_2D_ARRAY:
      uints[0] = glxx_server_get_active_texture(state, GL_TEXTURE_2D_ARRAY)->name;
      return glxx_get_uint_1;
   case GL_MAX_COLOR_ATTACHMENTS:
      uints[0] = GLXX_MAX_RENDER_TARGETS;
      return glxx_get_uint_1;
   case GL_MAX_UNIFORM_BUFFER_BINDINGS:
      uints[0] = GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS;
      return glxx_get_uint_1;
   case GL_UNIFORM_BUFFER_BINDING:
      uints[0] = state->bound_buffer[GLXX_BUFTGT_UNIFORM_BUFFER].buffer;
      return glxx_get_uint_1;
   case GL_TRANSFORM_FEEDBACK_BINDING:
      {
         GLXX_TRANSFORM_FEEDBACK_T *tf = glxx_get_bound_tf(state);
         uints[0] = tf->name;
         return glxx_get_uint_1;
      }
   case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
      {
         GLXX_TRANSFORM_FEEDBACK_T *tf = glxx_get_bound_tf(state);
         uints[0] = tf->bound_buffer.buffer;
         return glxx_get_uint_1;
      }
   case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_TF_INTERLEAVED_COMPONENTS;
      return glxx_get_uint_1;
   case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:
      uints[0] = GLXX_CONFIG_MAX_TF_SEPARATE_ATTRIBS;
      return glxx_get_uint_1;
   case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_TF_SEPARATE_COMPONENTS;
      return glxx_get_uint_1;
   case GL_READ_BUFFER:
      {
         GLXX_FRAMEBUFFER_T *fb = state->bound_read_framebuffer;
         uints[0] = GL_NONE;
         if (fb->read_buffer != GLXX_INVALID_ATT)
         {
            if (fb->name == 0)
               uints[0] = GL_BACK;
            else
               uints[0] = GL_COLOR_ATTACHMENT0 + (fb->read_buffer - GLXX_COLOR0_ATT);
         }
         return glxx_get_uint_1;
      }
   case GL_READ_FRAMEBUFFER_BINDING:
      uints[0] = state->bound_read_framebuffer->name;
      return glxx_get_uint_1;
   case GL_MAX_3D_TEXTURE_SIZE:
      uints[0] = MAX_3D_TEXTURE_SIZE;
      return glxx_get_uint_1;
   case GL_MAX_ARRAY_TEXTURE_LAYERS:
      uints[0] = MAX_ARRAY_TEXTURE_LAYERS;
      return glxx_get_uint_1;
   case GL_PIXEL_PACK_BUFFER_BINDING:
      uints[0] = state->bound_buffer[GLXX_BUFTGT_PIXEL_PACK].buffer;
      return glxx_get_uint_1;
   case GL_PIXEL_UNPACK_BUFFER_BINDING:
      uints[0] = state->bound_buffer[GLXX_BUFTGT_PIXEL_UNPACK].buffer;
      return glxx_get_uint_1;
   case GL_COPY_READ_BUFFER:
      uints[0] = state->bound_buffer[GLXX_BUFTGT_COPY_READ].buffer;
      return glxx_get_uint_1;
   case GL_COPY_WRITE_BUFFER:
      uints[0] = state->bound_buffer[GLXX_BUFTGT_COPY_WRITE].buffer;
      return glxx_get_uint_1;
    case GL_VERTEX_ARRAY_BINDING:
      uints[0] = state->vao.bound->name;
      return glxx_get_uint_1;
   case GL_SAMPLER_BINDING:
   {
      GLXX_TEXTURE_SAMPLER_STATE_T *sampler = state->bound_sampler[state->active_texture - GL_TEXTURE0];
      uints[0] = sampler != NULL ? sampler->id : 0u;
      return glxx_get_uint_1;
   }
   case GL_MAX_SAMPLES:
      uints[0] = GLXX_CONFIG_MAX_SAMPLES;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_VARYING_SCALARS;
      return glxx_get_uint_1;
   case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_VARYING_SCALARS;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_UNIFORM_BLOCKS:
   case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_UNIFORM_BLOCKS;
      return glxx_get_uint_1;
   case GL_MAX_COMBINED_UNIFORM_BLOCKS:
      uints[0] = GLXX_CONFIG_MAX_COMBINED_UNIFORM_BLOCKS;
      return glxx_get_uint_1;
   case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
      uints[0] = GLXX_CONFIG_UNIFORM_BUFFER_OFFSET_ALIGNMENT;
      return glxx_get_uint_1;
   case GL_NUM_EXTENSIONS:
      uints[0] = khrn_get_num_gl3x_exts();
      return glxx_get_uint_1;
   case GL_MAJOR_VERSION:
      uints[0] = 3u;
      return glxx_get_uint_1;
   case GL_MINOR_VERSION:
      uints[0] = KHRN_GLES32_DRIVER ? 2u : (KHRN_GLES31_DRIVER ? 1u : 0u);
      return glxx_get_uint_1;
   case GL_NUM_PROGRAM_BINARY_FORMATS:
      uints[0] = 0;
      return glxx_get_uint_1;
   case GL_PROGRAM_BINARY_FORMATS:
      return glxx_get_uint_0;       // No program binary formats supported
   case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
      uints[0] = state->hints.fshader_derivative;
      return glxx_get_uint_1;
   case GL_DRAW_BUFFER0:
   case GL_DRAW_BUFFER1:
   case GL_DRAW_BUFFER2:
   case GL_DRAW_BUFFER3:
   {
      static_assrt(GLXX_MAX_RENDER_TARGETS >= 4);
      unsigned index = pname - GL_DRAW_BUFFER0;
      GLXX_FRAMEBUFFER_T *fb = state->bound_draw_framebuffer;
      if (fb->draw_buffer[index])
      {
         if (fb->name == 0)
         {
            assert(index == 0);
            uints[0] = GL_BACK;
         }
         else
            uints[0] = GL_COLOR_ATTACHMENT0 + index;
      }
      else
      {
         uints[0] = GL_NONE;
      }
      return glxx_get_uint_1;
   }
   case GL_MAX_SERVER_WAIT_TIMEOUT:
      uints[0] = 0; // TODO
      return glxx_get_uint_1;
   case GL_MAX_UNIFORM_BLOCK_SIZE:
      uints[0] = GLXX_CONFIG_MAX_UNIFORM_BLOCK_SIZE;
      return glxx_get_uint_1;
   case GL_BLEND_COLOR:
      for (unsigned i = 0; i < 4; i++)
         floats[i] = state->blend_color[i];
      return glxx_get_norm_float_0 + 4u;
   case GL_ALIASED_POINT_SIZE_RANGE:
      floats[0] = GLXX_CONFIG_MIN_ALIASED_POINT_SIZE;
      floats[1] = GLXX_CONFIG_MAX_ALIASED_POINT_SIZE;
      return glxx_get_float_0 + 2;
   case GL_ALIASED_LINE_WIDTH_RANGE:
      floats[0] = GLXX_CONFIG_MIN_ALIASED_LINE_WIDTH;
      floats[1] = GLXX_CONFIG_MAX_ALIASED_LINE_WIDTH;
      return glxx_get_float_0 + 2;
   case GL_MAX_TEXTURE_LOD_BIAS:
      floats[0] = GLXX_CONFIG_MAX_TEXTURE_LOD_BIAS;
      return glxx_get_float_1;

#if KHRN_GLES31_DRIVER
   case GL_SAMPLE_MASK:
      booleans[0] = state->sample_mask.enable;
      return glxx_get_bool_1;
   case GL_TEXTURE_BINDING_2D_MULTISAMPLE:
      uints[0] = glxx_server_get_active_texture(state, GL_TEXTURE_2D_MULTISAMPLE)->name;
      return glxx_get_uint_1;
   case GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY:
      uints[0] = glxx_server_get_active_texture(state, GL_TEXTURE_2D_MULTISAMPLE_ARRAY)->name;
      return glxx_get_uint_1;
   case GL_DRAW_INDIRECT_BUFFER_BINDING:
      uints[0] = state->bound_buffer[GLXX_BUFTGT_DRAW_INDIRECT].buffer;
      return glxx_get_uint_1;
   case GL_DISPATCH_INDIRECT_BUFFER_BINDING:
      uints[0] = state->bound_buffer[GLXX_BUFTGT_DISPATCH_INDIRECT].buffer;
      return glxx_get_uint_1;
   case GL_ATOMIC_COUNTER_BUFFER_BINDING:
      uints[0] = state->bound_buffer[GLXX_BUFTGT_ATOMIC_COUNTER_BUFFER].buffer;
      return glxx_get_uint_1;
   case GL_SHADER_STORAGE_BUFFER_BINDING:
      uints[0] = state->bound_buffer[GLXX_BUFTGT_SHADER_STORAGE_BUFFER].buffer;
      return glxx_get_uint_1;
   case GL_MAX_FRAMEBUFFER_WIDTH:
   case GL_MAX_FRAMEBUFFER_HEIGHT:
      uints[0] = GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE;
      return glxx_get_uint_1;
   case GL_MAX_FRAMEBUFFER_SAMPLES:
      uints[0] = GLXX_CONFIG_MAX_SAMPLES;
      return glxx_get_uint_1;
   case GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS;
      return glxx_get_uint_1;
   case GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT:
      uints[0] = GLXX_CONFIG_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS:
      uints[0] = GLXX_CONFIG_MAX_VERTEX_STORAGE_BLOCKS;
      return glxx_get_uint_1;
   case GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_STORAGE_BLOCKS;
      return glxx_get_uint_1;
   case GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_STORAGE_BLOCKS;
      return glxx_get_uint_1;
   case GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS:
      uints[0] = GLXX_CONFIG_MAX_COMBINED_STORAGE_BLOCKS;
      return glxx_get_uint_1;
   case GL_MAX_SAMPLE_MASK_WORDS:
      uints[0] = GLXX_CONFIG_MAX_SAMPLE_WORDS;
      return glxx_get_uint_1;
   case GL_MAX_UNIFORM_LOCATIONS:
      uints[0] = GLXX_CONFIG_MAX_UNIFORM_LOCATIONS;
      return glxx_get_uint_1;
   case GL_MAX_IMAGE_UNITS:
      uints[0] = GLXX_CONFIG_MAX_IMAGE_UNITS;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_IMAGE_UNIFORMS:
      uints[0] = GLXX_CONFIG_MAX_VERTEX_IMAGE_UNIFORMS;
      return glxx_get_uint_1;
   case GL_MAX_FRAGMENT_IMAGE_UNIFORMS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_IMAGE_UNIFORMS;
      return glxx_get_uint_1;
   case GL_MAX_COMPUTE_IMAGE_UNIFORMS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_IMAGE_UNIFORMS;
      return glxx_get_uint_1;
   case GL_MAX_COMBINED_IMAGE_UNIFORMS:
      uints[0] = GLXX_CONFIG_MAX_COMBINED_IMAGE_UNIFORMS;
      return glxx_get_uint_1;
   case GL_MAX_COMPUTE_UNIFORM_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_UNIFORM_SCALARS;
      return glxx_get_uint_1;
   case GL_MAX_COMPUTE_UNIFORM_BLOCKS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_UNIFORM_BLOCKS;
      return glxx_get_uint_1;
   case GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_TEXTURE_IMAGE_UNITS;
      return glxx_get_uint_1;
   case GL_MAX_COMPUTE_ATOMIC_COUNTERS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_ATOMIC_COUNTERS;
      return glxx_get_uint_1;
   case GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_ATOMIC_COUNTER_BUFFERS;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_ATOMIC_COUNTERS:
      uints[0] = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTERS;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS:
      uints[0] = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS;
      return glxx_get_uint_1;
   case GL_MAX_FRAGMENT_ATOMIC_COUNTERS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_ATOMIC_COUNTERS;
      return glxx_get_uint_1;
   case GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_ATOMIC_COUNTER_BUFFERS;
      return glxx_get_uint_1;
   case GL_MAX_COMBINED_ATOMIC_COUNTERS:
      uints[0] = GLXX_CONFIG_MAX_COMBINED_ATOMIC_COUNTERS;
      return glxx_get_uint_1;
   case GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS:
      uints[0] = GLXX_CONFIG_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS;
      return glxx_get_uint_1;
   case GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS:
      uints[0] = GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;
      return glxx_get_uint_1;
   case GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE:
      uints[0] = GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_SIZE;
      return glxx_get_uint_1;
   case GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES:
      uints[0] = GLXX_CONFIG_MAX_COMBINED_SHADER_OUTPUTS;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_ATTRIB_BINDINGS:
      uints[0] = GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET:
      uints[0] = GLXX_CONFIG_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET;
      return glxx_get_uint_1;
   case GL_MAX_VERTEX_ATTRIB_STRIDE:
      uints[0] = GLXX_CONFIG_MAX_VERTEX_ATTRIB_STRIDE;
      return glxx_get_uint_1;
   case GL_MAX_COLOR_TEXTURE_SAMPLES:
   case GL_MAX_DEPTH_TEXTURE_SAMPLES:
      uints[0] = GLXX_CONFIG_MAX_SAMPLES;
      return glxx_get_uint_1;
   case GL_MAX_INTEGER_SAMPLES:
      uints[0] = GLXX_CONFIG_MAX_INTEGER_SAMPLES;
      return glxx_get_uint_1;
   case GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS:
      uints[0] = GLXX_CONFIG_MAX_COMPUTE_WORK_GROUP_INVOCATIONS;
      return glxx_get_uint_1;
   case GL_MAX_COMPUTE_SHARED_MEMORY_SIZE:
      uints[0] = GLXX_CONFIG_MAX_COMPUTE_SHARED_MEM_SIZE;
      return glxx_get_uint_1;
   case GL_PROGRAM_PIPELINE_BINDING:
      uints[0] = glxx_pipeline_get_binding(state);
      return glxx_get_uint_1;
   case GL_MAX_SHADER_STORAGE_BLOCK_SIZE:
      int64s[0] = GLXX_CONFIG_MAX_SHADER_STORAGE_BLOCK_SIZE;
      return glxx_get_int64_1;
   case GL_PRIMITIVE_BOUNDING_BOX:
      for (int i=0; i<4; i++) {
         floats[i]   = state->primitive_bounding_box.min[i];
         floats[4+i] = state->primitive_bounding_box.max[i];
      }
      return glxx_get_float_0 + 8;
#endif

#if GLXX_HAS_TNG
   case GL_PATCH_VERTICES:
      uints[0] = state->num_patch_vertices;
      return glxx_get_uint_1;
   case GL_MAX_PATCH_VERTICES:
      uints[0] = V3D_MAX_PATCH_VERTICES;
      return glxx_get_uint_1;
   case GL_MAX_TESS_GEN_LEVEL:
      uints[0] = V3D_MAX_TESS_GEN_LEVEL;
      return glxx_get_uint_1;
   case GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS:
   case GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS:
   case GL_MAX_GEOMETRY_UNIFORM_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_UNIFORM_SCALARS;
      return glxx_get_uint_1;
   case GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS:
   case GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS:
   case GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_TEXTURE_IMAGE_UNITS;
      return glxx_get_uint_1;
   case GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_TESS_CONTROL_OUTPUT_COMPONENTS;
      return glxx_get_uint_1;
   case GL_MAX_TESS_PATCH_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_TESS_PATCH_COMPONENTS;
      return glxx_get_uint_1;
   case GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS;
      return glxx_get_uint_1;
   case GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS;
      return glxx_get_uint_1;
   case GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS:
   case GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS:
   case GL_MAX_GEOMETRY_UNIFORM_BLOCKS:
      uints[0] = GLXX_CONFIG_MAX_SHADER_UNIFORM_BLOCKS;
      return glxx_get_uint_1;
   case GL_MAX_TESS_CONTROL_INPUT_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_TESS_CONTROL_INPUT_COMPONENTS;
      return glxx_get_uint_1;
   case GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_TESS_EVALUATION_INPUT_COMPONENTS;
      return glxx_get_uint_1;
   case GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS:
   case GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS:
   case GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_UNIFORM_SCALARS + GLXX_CONFIG_MAX_SHADER_UNIFORM_BLOCKS * GLXX_CONFIG_MAX_UNIFORM_BLOCK_SIZE / 4;
      return glxx_get_uint_1;
   case GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS:
   case GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS:
   case GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS:
      uints[0] = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS;
      return glxx_get_uint_1;
   case GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS:
   case GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS:
   case GL_MAX_GEOMETRY_ATOMIC_COUNTERS:
      uints[0] = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTERS;
      return glxx_get_uint_1;
   case GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS:
   case GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS:
   case GL_MAX_GEOMETRY_IMAGE_UNIFORMS:
      uints[0] = GLXX_CONFIG_MAX_VERTEX_IMAGE_UNIFORMS;
      return glxx_get_uint_1;
   case GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS:
   case GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS:
   case GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS:
      uints[0] = GLXX_CONFIG_MAX_VERTEX_STORAGE_BLOCKS;
      return glxx_get_uint_1;
   case GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED:
      booleans[0] = false;
      return glxx_get_bool_1;

   case GL_MAX_GEOMETRY_INPUT_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_GEOMETRY_INPUT_COMPONENTS;
      return glxx_get_uint_1;
   case GL_MAX_GEOMETRY_OUTPUT_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_GEOMETRY_OUTPUT_COMPONENTS;
      return glxx_get_uint_1;
   case GL_MAX_GEOMETRY_OUTPUT_VERTICES:
      uints[0] = GLXX_CONFIG_MAX_GEOMETRY_OUTPUT_VERTICES;
      return glxx_get_uint_1;
   case GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
      uints[0] = GLXX_CONFIG_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS;
      return glxx_get_uint_1;
   case GL_MAX_GEOMETRY_SHADER_INVOCATIONS:
      uints[0] = V3D_MAX_GEOMETRY_INVOCATIONS;
      return glxx_get_uint_1;
   case GL_LAYER_PROVOKING_VERTEX:
      ints[0] = GL_UNDEFINED_VERTEX;   /* TODO: It's possible that GL_LAST_VERTEX_CONVENTION is correct here */
      return glxx_get_int_1;

   case GL_MAX_FRAMEBUFFER_LAYERS:
      uints[0] = GLXX_CONFIG_MAX_FRAMEBUFFER_LAYERS;
      return glxx_get_uint_1;
#endif

#if V3D_VER_AT_LEAST(3,3,0,0)
   case GL_PROVOKING_VERTEX_BRCM:
      uints[0] = state->provoking_vtx;
      return glxx_get_uint_1;
#endif
   case GL_TEXTURE_BINDING_1D_BRCM:
      uints[0] = glxx_server_get_active_texture(state, GL_TEXTURE_1D_BRCM)->name;
      return glxx_get_uint_1;
   case GL_TEXTURE_BINDING_1D_ARRAY_BRCM:
      uints[0] = glxx_server_get_active_texture(state, GL_TEXTURE_1D_ARRAY_BRCM)->name;
      return glxx_get_uint_1;

#if KHRN_GLES32_DRIVER
   case GL_MIN_FRAGMENT_INTERPOLATION_OFFSET:
      floats[0] = -0.5f;
      return glxx_get_float_1;
   case GL_MAX_FRAGMENT_INTERPOLATION_OFFSET:
      floats[0] = 0.5f;
      return glxx_get_float_1;
   case GL_FRAGMENT_INTERPOLATION_OFFSET_BITS:
      ints[0] = 4;
      return glxx_get_int_1;

   case GL_MULTISAMPLE_LINE_WIDTH_RANGE:
      floats[0] = GLXX_CONFIG_MIN_MULTISAMPLE_LINE_WIDTH;
      floats[1] = GLXX_CONFIG_MAX_MULTISAMPLE_LINE_WIDTH;
      return glxx_get_float_0 + 2;
   case GL_MULTISAMPLE_LINE_WIDTH_GRANULARITY:
      floats[0] = GLXX_CONFIG_MULTISAMPLE_LINE_WIDTH_GRANULARITY;
      return glxx_get_float_1;
#endif
#if KHRN_GLES31_DRIVER
   case GL_MAX_TEXTURE_BUFFER_SIZE:
      uints[0] = GLXX_CONFIG_MAX_TEXTURE_BUFFER_SIZE;
      return glxx_get_uint_1;
   case GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
      uints[0] = GLXX_CONFIG_TEXTURE_BUFFER_OFFSET_ALIGNMENT;
      return glxx_get_uint_1;
   case GL_TEXTURE_BINDING_BUFFER:
      uints[0] = glxx_server_get_active_texture(state, GL_TEXTURE_BUFFER)->name;
      return glxx_get_uint_1;
   case GL_TEXTURE_BUFFER_BINDING:
      uints[0] = state->bound_buffer[GLXX_BUFTGT_TEXTURE_BUFFER].obj != NULL ? state->bound_buffer[GLXX_BUFTGT_TEXTURE_BUFFER].obj->name : 0u;
      return glxx_get_uint_1;
#endif

   case GL_BLEND_ADVANCED_COHERENT_KHR:
      booleans[0] = state->blend.advanced_coherent ? GL_TRUE : GL_FALSE;
      return glxx_get_bool_1;

   default:
      return glxx_get_invalid_enum_0;
   }
}

static glxx_get_type_count glxx_get_params_and_type(
   const GLXX_SERVER_STATE_T* state,
   GLenum pname,
   GLboolean* booleans,
   GLint* ints,
   GLuint* uints,
   GLfloat* floats,
   GLint64* int64s)
{
   glxx_get_type_count ret = glxx_get_params_and_type_common(state, pname, booleans, ints, uints, floats);
   if (ret != glxx_get_invalid_enum_0)
      return ret;

   if (IS_GL_11(state))
      return glxx_get_params_and_type_gl11(state, pname, booleans, ints, uints, floats);
   else
      return glxx_get_params_and_type_gl3x(state, pname, booleans, ints, uints, floats, int64s);
}

/* Can use glxx_get_union.ints & glxx_get_union.uints interchangeably, and cast
 * betweeen (GLint *) and (GLuint *) */
static_assrt(sizeof(GLint) == sizeof(GLuint));

typedef union
{
   GLboolean bools[MAX_GET_ITEM_COUNT];
   GLint ints[MAX_GET_ITEM_COUNT];
   GLuint uints[MAX_GET_ITEM_COUNT];
   GLint64 int64s[MAX_GET_ITEM_COUNT];
   GLfloat floats[MAX_GET_ITEM_COUNT];
} glxx_get_union;

static GLenum glxx_get_union_to_bools(GLboolean *bools, glxx_get_type_count type_count, const glxx_get_union *u)
{
   unsigned count = type_count & gfx_mask(glxx_get_type_shift);
   assert(count <= MAX_GET_ITEM_COUNT);

   switch (type_count >> glxx_get_type_shift)
   {
   case glxx_get_invalid_enum_type:
      return GL_INVALID_ENUM;
   case glxx_get_invalid_index_type:
      return GL_INVALID_VALUE;
   case glxx_get_bool_type:
      /* Assume wrote directly to bools */
      return GL_NO_ERROR;
   case glxx_get_int_type:
   case glxx_get_uint_type:
      for (unsigned i = 0; i != count; ++i)
         bools[i] = u->ints[i] != 0;
      return GL_NO_ERROR;
   case glxx_get_int64_type:
      for (unsigned i = 0; i != count; ++i)
         bools[i] = u->int64s[i] != 0;
      return GL_NO_ERROR;
   case glxx_get_norm_float_type:
   case glxx_get_float_type:
      for (unsigned i = 0; i != count; ++i)
         bools[i] = u->floats[i] != 0.0f;
      return GL_NO_ERROR;
   default:
      unreachable();
      return GL_NO_ERROR;
   }
}

static GLenum glxx_get_union_to_fixeds(GLfixed *fixeds, glxx_get_type_count type_count, const glxx_get_union *u)
{
   unsigned count = type_count & gfx_mask(glxx_get_type_shift);
   assert(count <= MAX_GET_ITEM_COUNT);

   // todo, could probably be smarter about these conversions now.
   switch (type_count >> glxx_get_type_shift)
   {
   case glxx_get_invalid_enum_type:
      return GL_INVALID_ENUM;
   case glxx_get_invalid_index_type:
      return GL_INVALID_VALUE;
   case glxx_get_bool_type:
      for (unsigned i = 0; i != count; ++i)
         fixeds[i] = u->bools[i] ? float_to_fixed(1.0f) : 0;
      return GL_NO_ERROR;
   case glxx_get_int_type:
      for (unsigned i = 0; i != count; ++i)
         fixeds[i] = float_to_fixed((GLfloat)u->ints[i]);
      return GL_NO_ERROR;
   case glxx_get_uint_type:
      for (unsigned i = 0; i != count; ++i)
         fixeds[i] = float_to_fixed((GLfloat)u->uints[i]);
      return GL_NO_ERROR;
   case glxx_get_int64_type:
      for (unsigned i = 0; i != count; ++i)
         fixeds[i] = float_to_fixed((GLfloat)u->int64s[i]);
      return GL_NO_ERROR;
   case glxx_get_norm_float_type:
   case glxx_get_float_type:
      for (unsigned i = 0; i != count; ++i)
         fixeds[i] = float_to_fixed(u->floats[i]);
      return GL_NO_ERROR;
   default:
      unreachable();
      return GL_NO_ERROR;
   }
}

static GLenum glxx_get_union_to_floats(GLfloat *floats, glxx_get_type_count type_count, const glxx_get_union *u)
{
   unsigned count = type_count & gfx_mask(glxx_get_type_shift);
   assert(count <= MAX_GET_ITEM_COUNT);

   switch (type_count >> glxx_get_type_shift)
   {
   case glxx_get_invalid_enum_type:
      return GL_INVALID_ENUM;
   case glxx_get_invalid_index_type:
      return GL_INVALID_VALUE;
   case glxx_get_bool_type:
      for (unsigned i = 0; i != count; ++i)
         floats[i] = u->bools[i] ? 1.0f : 0.0f;
      return GL_NO_ERROR;
   case glxx_get_int_type:
      for (unsigned i = 0; i != count; ++i)
         floats[i] = (GLfloat)u->ints[i];
      return GL_NO_ERROR;
   case glxx_get_uint_type:
      for (unsigned i = 0; i != count; ++i)
         floats[i] = (GLfloat)u->uints[i];
      return GL_NO_ERROR;
   case glxx_get_int64_type:
      for (unsigned i = 0; i != count; ++i)
         floats[i] = (GLfloat)u->int64s[i];
      return GL_NO_ERROR;
   case glxx_get_norm_float_type:
   case glxx_get_float_type:
      /* Assume wrote directly to floats */
      return GL_NO_ERROR;
   default:
      unreachable();
      return GL_NO_ERROR;
   }
}

static GLenum glxx_get_union_to_ints(GLint *ints, glxx_get_type_count type_count, const glxx_get_union *u)
{
   unsigned count = type_count & gfx_mask(glxx_get_type_shift);
   assert(count <= MAX_GET_ITEM_COUNT);

   switch (type_count >> glxx_get_type_shift)
   {
   case glxx_get_invalid_enum_type:
      return GL_INVALID_ENUM;
   case glxx_get_invalid_index_type:
      return GL_INVALID_VALUE;
   case glxx_get_bool_type:
      for (unsigned i = 0; i != count; ++i)
         ints[i] = (GLint)u->bools[i];
      return GL_NO_ERROR;
   case glxx_get_int_type:
      /* Assume wrote directly to ints */
      return GL_NO_ERROR;
   case glxx_get_uint_type:
      for (unsigned i = 0; i != count; ++i)
         ints[i] = gfx_uclamp(u->uints[i], 0u, (unsigned)INT_MAX);
      return GL_NO_ERROR;
   case glxx_get_int64_type:
      for (unsigned i = 0; i != count; ++i)
         ints[i] = (GLint)u->int64s[i];
      return GL_NO_ERROR;
   case glxx_get_norm_float_type:
      for (unsigned i = 0; i != count; ++i)
         ints[i] = norm_float_to_int(u->floats[i]);
      return GL_NO_ERROR;
   case glxx_get_float_type:
      for (unsigned i = 0; i != count; ++i)
         ints[i] = gfx_float_to_int32(u->floats[i]);
      return GL_NO_ERROR;
   default:
      unreachable();
      return GL_NO_ERROR;
   }
}

static GLenum glxx_get_union_to_int64s(GLint64 *int64s, glxx_get_type_count type_count, const glxx_get_union *u)
{
   unsigned count = type_count & gfx_mask(glxx_get_type_shift);
   assert(count <= MAX_GET_ITEM_COUNT);

   switch (type_count >> glxx_get_type_shift)
   {
   case glxx_get_invalid_enum_type:
      return GL_INVALID_ENUM;
   case glxx_get_invalid_index_type:
      return GL_INVALID_VALUE;
   case glxx_get_bool_type:
      for (unsigned i = 0; i != count; ++i)
         int64s[i] = (GLint64)u->bools[i];
      return GL_NO_ERROR;
   case glxx_get_int_type:
      for (unsigned i = 0; i != count; ++i)
         int64s[i] = (GLint64)u->ints[i];
      return GL_NO_ERROR;
   case glxx_get_uint_type:
      for (unsigned i = 0; i != count; ++i)
         int64s[i] = (GLint64)u->uints[i];
      return GL_NO_ERROR;
   case glxx_get_int64_type:
      /* Assume wrote directly to int64s */
      return GL_NO_ERROR;
   case glxx_get_norm_float_type:
      for (unsigned i = 0; i != count; ++i)
         int64s[i] = (GLint64)norm_float_to_int(u->floats[i]); // todo, 64-bit handling?
      return GL_NO_ERROR;
   case glxx_get_float_type:
      for (unsigned i = 0; i != count; ++i)
         int64s[i] = gfx_float_to_int64(u->floats[i]);
      return GL_NO_ERROR;
   default:
      unreachable();
      return GL_NO_ERROR;
   }
}

GLenum glxx_get_booleans(const GLXX_SERVER_STATE_T* state, GLenum pname, GLboolean* params)
{
   glxx_get_union u;
   glxx_get_type_count type_count = glxx_get_params_and_type(state, pname, params, u.ints, u.uints, u.floats, u.int64s);
   return glxx_get_union_to_bools(params, type_count, &u);
}

GLenum glxx_get_fixeds(const GLXX_SERVER_STATE_T* state, GLenum pname, GLfixed* params)
{
   glxx_get_union u;
   glxx_get_type_count type_count = glxx_get_params_and_type(state, pname, u.bools, u.ints, u.uints, u.floats, u.int64s);
   return glxx_get_union_to_fixeds(params, type_count, &u);
}

GL_API void GL_APIENTRY glGetBooleanv(GLenum pname, GLboolean *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   if (!state) return;

   GLenum error = glxx_get_booleans(state, pname, params);
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetFloatv(GLenum pname, GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   if (!state) return;

   glxx_get_union u;
   glxx_get_type_count type_count = glxx_get_params_and_type(state, pname, u.bools, u.ints, u.uints, params, u.int64s);
   GLenum error = glxx_get_union_to_floats(params, type_count, &u);
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetIntegerv(GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   if (!state) return;

   glxx_get_union u;
   glxx_get_type_count type_count = glxx_get_params_and_type(state, pname, u.bools, params, u.uints, u.floats, u.int64s);
   GLenum error = glxx_get_union_to_ints(params, type_count, &u);
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetInteger64v(GLenum pname, GLint64* params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state) return;

   glxx_get_union u;
   glxx_get_type_count type_count = glxx_get_params_and_type(state, pname, u.bools, u.ints, u.uints, u.floats, params);
   GLenum error = glxx_get_union_to_int64s(params, type_count, &u);
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

bool glxx_is_int_sampler_texparam(GLXX_SERVER_STATE_T *state, GLenum pname)
{
   switch(pname)
   {
      // Sampler parameters
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
         return true;

      case GL_TEXTURE_WRAP_R:
      case GL_TEXTURE_COMPARE_MODE:
      case GL_TEXTURE_COMPARE_FUNC:
      case GL_TEXTURE_BORDER_COLOR:
      case GL_TEXTURE_SRGB_DECODE_EXT:
         return !IS_GL_11(state);

#if V3D_VER_AT_LEAST(3,3,0,0)
      case GL_TEXTURE_UNNORMALISED_COORDS_BRCM:
         return !IS_GL_11(state);
#endif

      default:
         return false;
   }
}

bool glxx_is_int_texparam(GLXX_SERVER_STATE_T *state, GLenum target, GLenum pname)
{
   if (glxx_is_int_sampler_texparam(state, pname))
      return true;

   switch (pname)
   {
      case GL_GENERATE_MIPMAP:
         return IS_GL_11(state) && target != GL_TEXTURE_EXTERNAL_OES;

      case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
         return true;
      case GL_TEXTURE_SWIZZLE_R:
      case GL_TEXTURE_SWIZZLE_G:
      case GL_TEXTURE_SWIZZLE_B:
      case GL_TEXTURE_SWIZZLE_A:
      case GL_TEXTURE_BASE_LEVEL:
      case GL_TEXTURE_MAX_LEVEL:
      case GL_TEXTURE_IMMUTABLE_FORMAT:
      case GL_TEXTURE_IMMUTABLE_LEVELS:
      case GL_TEXTURE_PROTECTED_EXT:
         return !IS_GL_11(state);
      case GL_DEPTH_STENCIL_TEXTURE_MODE:
      case GL_IMAGE_FORMAT_COMPATIBILITY_TYPE:
         return KHRN_GLES31_DRIVER ? !IS_GL_11(state) : false;
      default:
         return false;
   }
}

uint32_t glxx_get_texparameter_sampler_internal(GLXX_SERVER_STATE_T *state, GLXX_TEXTURE_SAMPLER_STATE_T *so, GLenum pname, GLint *params)
{
   uint32_t result;

   assert(glxx_is_int_sampler_texparam(state, pname));

   switch(pname)
   {
      case GL_TEXTURE_MIN_FILTER:
         params[0] = so->filter.min;
         result = 1;
         break;
      case GL_TEXTURE_MAG_FILTER:
         params[0] = so->filter.mag;
         result = 1;
         break;
      case GL_TEXTURE_WRAP_S:
         params[0] = so->wrap.s;
         result = 1;
         break;
      case GL_TEXTURE_WRAP_T:
         params[0] = so->wrap.t;
         result = 1;
         break;
      case GL_TEXTURE_WRAP_R:
         params[0] = so->wrap.r;
         result = 1;
         break;
      case GL_TEXTURE_COMPARE_MODE:
         params[0] = so->compare_mode;
         result = 1;
         break;
      case GL_TEXTURE_COMPARE_FUNC:
         params[0] = so->compare_func;
         result = 1;
         break;
      case GL_TEXTURE_UNNORMALISED_COORDS_BRCM:
         params[0] = so->unnormalised_coords ? GL_TRUE : GL_FALSE;
         result = 1;
         break;
      case GL_TEXTURE_SRGB_DECODE_EXT:
         params[0] = so->skip_srgb_decode ? GL_SKIP_DECODE_EXT : GL_DECODE_EXT;
         result = 1;
         break;
      case GL_TEXTURE_BORDER_COLOR:
         for (uint32_t i = 0; i < 4; i++)
            params[i] = so->border_color[i];
         result = 4;
         break;
      default:
         unreachable();
   }
   return result;
}

uint32_t glxx_get_texparameter_internal(GLXX_SERVER_STATE_T *state, GLenum target, GLenum pname, GLint *params)
{
   if (target == GL_TEXTURE_BUFFER)
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return 0;
   }

   assert(glxx_is_int_texparam(state, target, pname));

   int result = 0;
   GLXX_TEXTURE_T *texture = glxx_server_state_get_texture(state, target);

   if (texture) {
      switch (pname) {
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_WRAP_R:
      case GL_TEXTURE_COMPARE_MODE:
      case GL_TEXTURE_COMPARE_FUNC:
      case GL_TEXTURE_BORDER_COLOR:
      case GL_TEXTURE_UNNORMALISED_COORDS_BRCM:
      case GL_TEXTURE_SRGB_DECODE_EXT:
         result = glxx_get_texparameter_sampler_internal(state, &texture->sampler, pname, params);
         break;
      case GL_GENERATE_MIPMAP:
         params[0] = texture->generate_mipmap;
         result = 1;
         break;
      case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
         params[0] = 1;
         result = 1;
         break;
      case GL_TEXTURE_SWIZZLE_R:
      case GL_TEXTURE_SWIZZLE_G:
      case GL_TEXTURE_SWIZZLE_B:
      case GL_TEXTURE_SWIZZLE_A:
         switch (texture->swizzle[pname - GL_TEXTURE_SWIZZLE_R]) {
            case 0: params[0] = GL_ZERO; break;
            case 1: params[0] = GL_ONE; break;
            case 2: params[0] = GL_RED; break;
            case 3: params[0] = GL_GREEN; break;
            case 4: params[0] = GL_BLUE; break;
            case 5: params[0] = GL_ALPHA; break;
            default:
               unreachable();
         }
         result = 1;
         break;
      case GL_TEXTURE_BASE_LEVEL:
         params[0] = texture->base_level;
         result = 1;
         break;
      case GL_TEXTURE_MAX_LEVEL:
         params[0] = texture->max_level;
         result = 1;
         break;
      case GL_TEXTURE_IMMUTABLE_FORMAT:
         params[0] = (texture->immutable_format == GFX_LFMT_NONE) ? GL_FALSE : GL_TRUE;
         result = 1;
         break;
      case GL_TEXTURE_IMMUTABLE_LEVELS:
         params[0] = texture->immutable_levels;
         result = 1;
         break;
      case GL_DEPTH_STENCIL_TEXTURE_MODE:
         params[0] = texture->ds_texture_mode;
         result = 1;
         break;
      case GL_IMAGE_FORMAT_COMPATIBILITY_TYPE:
         params[0] = GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE;
         result = 1;
         break;
      case GL_TEXTURE_PROTECTED_EXT:
         params[0] = texture->create_secure;
         result = 1;
         break;

      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
         break;
      }
   }

   return result;
}

uint32_t glxx_get_texparameterf_sampler_internal(GLXX_SERVER_STATE_T *state, GLXX_TEXTURE_SAMPLER_STATE_T *sampler, GLenum pname, GLfloat *params)
{
   uint32_t result;

   switch (pname)
   {
#if GL_EXT_texture_filter_anisotropic
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      if (!IS_GL_11(state)) {
         params[0] = sampler->anisotropy;
         result = 1;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
      }
      break;
#endif
   case GL_TEXTURE_MIN_LOD:
      if (!IS_GL_11(state)) {
         params[0] = sampler->min_lod;
         result = 1;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
      }
      break;
   case GL_TEXTURE_MAX_LOD:
      if (!IS_GL_11(state)) {
         params[0] = sampler->max_lod;
         result = 1;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
      }
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      result = 0;
   }

   return result;
}

static uint32_t glxx_get_texparameterf_internal(GLXX_SERVER_STATE_T *state, GLenum target, GLenum pname, GLfloat *params)
{
   if (target == GL_TEXTURE_BUFFER)
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return 0;
   }

   int result = 0;
   GLXX_TEXTURE_T *texture = glxx_server_state_get_texture(state, target);

   if (texture) {
      switch (pname) {
#if GL_EXT_texture_filter_anisotropic
      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
#endif
      case GL_TEXTURE_MIN_LOD:
      case GL_TEXTURE_MAX_LOD:
         result = glxx_get_texparameterf_sampler_internal(state, &texture->sampler, pname, params);
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
         break;
      }
   }

   return result;
}

GL_API void GL_APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   if (!state)
      return;

   if (glxx_is_float_texparam(pname)) {
      glxx_get_texparameterf_internal(state, target, pname, params);
   } else if (glxx_is_int_texparam(state, target, pname)) {
      GLint temp[4];
      uint32_t count = glxx_get_texparameter_internal(state, target, pname, temp);
      if (count) {
         assert(count == 1 || count == 4);
         for (uint32_t i = 0; i < count; i++)
            if (pname == GL_TEXTURE_BORDER_COLOR)
               params[i] = gfx_float_from_bits(temp[i]);
            else
               params[i] = (GLfloat)temp[i];
      }
   } else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

   glxx_unlock_server_state();

}

void glxx_get_texparameter_iv_common(GLenum target, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   if (!state) return;

   if (glxx_is_float_texparam(pname)) {
      GLfloat temp;
      uint32_t count = glxx_get_texparameterf_internal(state, target, pname, &temp);
      if (count)
      {
         assert(count == 1);
         params[0] = gfx_float_to_int32(temp);
      }
   } else if (glxx_is_int_texparam(state, target, pname)) {
      glxx_get_texparameter_internal(state, target, pname, params);
   } else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
   glxx_get_texparameter_iv_common(target, pname, params);
}

#if KHRN_GLES32_DRIVER
GL_APICALL void GL_APIENTRY glGetTexParameterIiv(GLenum target, GLenum pname, GLint *params)
{
   glxx_get_texparameter_iv_common(target, pname, params);
}

GL_APICALL void GL_APIENTRY glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint *params)
{
   glxx_get_texparameter_iv_common(target, pname, (GLint *) params);
}
#endif

static int get_integer_vertex_attrib_internal(GLXX_SERVER_STATE_T *state,
                                              GLint index, GLenum pname)
{
   GLXX_ATTRIB_CONFIG_T *attr;
   GLXX_VBO_BINDING_T *vbo;
   assert(index < GLXX_CONFIG_MAX_VERTEX_ATTRIBS);

   attr = &state->vao.bound->attrib_config[index];
   vbo = &state->vao.bound->vbos[attr->vbo_index];
   switch (pname) {
   case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
      return attr->enabled ? GL_TRUE : GL_FALSE;
   case GL_VERTEX_ATTRIB_ARRAY_SIZE:
      return attr->size;
   case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
      return attr->original_stride;
   case GL_VERTEX_ATTRIB_ARRAY_TYPE:
      return attr->gl_type;
   case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
      return attr->norm ? GL_TRUE : GL_FALSE;
   case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
      if (vbo->buffer == NULL) return 0;
      else return vbo->buffer->name;
   case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
      return vbo->divisor;
   case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
      return attr->is_int;
   case GL_VERTEX_ATTRIB_BINDING:
      return attr->vbo_index;
   case GL_VERTEX_ATTRIB_RELATIVE_OFFSET:
      return attr->relative_offset;
   default:
      unreachable(); return 0;
   }
}

static bool is_integer_attrib_param(GLenum pname) {
   switch(pname) {
      case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
      case GL_VERTEX_ATTRIB_ARRAY_SIZE:
      case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
      case GL_VERTEX_ATTRIB_ARRAY_TYPE:
      case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
      case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
      case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
      case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
         return true;
      case GL_VERTEX_ATTRIB_BINDING:
      case GL_VERTEX_ATTRIB_RELATIVE_OFFSET:
         return KHRN_GLES31_DRIVER ? true : false;
      default:
         return false;
   }
}

GL_API void GL_APIENTRY glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (is_integer_attrib_param(pname)) {
      params[0] = (GLfloat)get_integer_vertex_attrib_internal(state, index, pname);
   } else {
      switch (pname) {
         case GL_CURRENT_VERTEX_ATTRIB:
            params[0] = state->generic_attrib[index].f[0];
            params[1] = state->generic_attrib[index].f[1];
            params[2] = state->generic_attrib[index].f[2];
            params[3] = state->generic_attrib[index].f[3];
            break;
         default:
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
            break;
      }
   }

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (is_integer_attrib_param(pname)) {
      params[0] = get_integer_vertex_attrib_internal(state, index, pname);
   } else {
      switch(pname) {
         case GL_CURRENT_VERTEX_ATTRIB:
            params[0] = (GLint)state->generic_attrib[index].f[0];
            params[1] = (GLint)state->generic_attrib[index].f[1];
            params[2] = (GLint)state->generic_attrib[index].f[2];
            params[3] = (GLint)state->generic_attrib[index].f[3];
            break;
         default:
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
            break;
      }
   }

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetVertexAttribIiv(GLuint index, GLenum pname, GLint* params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (is_integer_attrib_param(pname)) {
      params[0] = get_integer_vertex_attrib_internal(state, index, pname);
   } else {
      switch (pname)
      {
         case GL_CURRENT_VERTEX_ATTRIB:
            params[0] = state->generic_attrib[index].i[0];
            params[1] = state->generic_attrib[index].i[1];
            params[2] = state->generic_attrib[index].i[2];
            params[3] = state->generic_attrib[index].i[3];
            break;

         default:
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
            break;
      }
   }

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (is_integer_attrib_param(pname)) {
      params[0] = get_integer_vertex_attrib_internal(state, index, pname);
   } else {
      switch (pname) {
      case GL_CURRENT_VERTEX_ATTRIB:
         params[0] = state->generic_attrib[index].u[0];
         params[1] = state->generic_attrib[index].u[1];
         params[2] = state->generic_attrib[index].u[2];
         params[3] = state->generic_attrib[index].u[3];
         break;

      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
   }

end:
   glxx_unlock_server_state();
}

typedef uint32_t (*comp_size_fct_t)(GFX_LFMT_T lfmt);
static uint32_t component_bits(GLXX_FRAMEBUFFER_T *fbo, GLenum component)
{
   glxx_attachment_point_t att_point;
   comp_size_fct_t fct = NULL;
   uint32_t bits = 0;

   switch(component)
   {
      case GL_RED_BITS:
         att_point = GL_COLOR_ATTACHMENT0;
         fct = gfx_lfmt_red_bits;
         break;
      case GL_GREEN_BITS:
         att_point = GL_COLOR_ATTACHMENT0;
         fct = gfx_lfmt_green_bits;
         break;
      case GL_BLUE_BITS:
         att_point = GL_COLOR_ATTACHMENT0;
         fct = gfx_lfmt_blue_bits;
         break;
      case GL_ALPHA_BITS:
         att_point = GL_COLOR_ATTACHMENT0;
         fct = gfx_lfmt_alpha_bits;
         break;
      case GL_DEPTH_BITS:
         att_point = GL_DEPTH_ATTACHMENT;
         fct = gfx_lfmt_depth_bits;
         break;
      case GL_STENCIL_BITS:
         att_point = GL_STENCIL_ATTACHMENT;
         fct = gfx_lfmt_stencil_bits;
         break;
      default:
         unreachable();
   }

   const GLXX_ATTACHMENT_T *att = glxx_fb_get_attachment(fbo, att_point);
   GFX_LFMT_T api_fmt = glxx_attachment_get_api_fmt(att);
   if (api_fmt != GFX_LFMT_NONE)
      bits = fct(api_fmt);

   return bits;
}

static glxx_get_type_count glxx_get_params_and_type_gl3x_indexed(
   const GLXX_SERVER_STATE_T *state,
   GLenum target, GLuint index,
   GLboolean *booleans,
   GLint *ints,
   GLint64 *int64s)
{
   switch (target)
   {
   case GL_UNIFORM_BUFFER_BINDING:
   case GL_UNIFORM_BUFFER_START:
   case GL_UNIFORM_BUFFER_SIZE:
      if (index >= GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS)
         return glxx_get_invalid_index_0;
      switch (target)
      {
      case GL_UNIFORM_BUFFER_BINDING:
         ints[0] = state->uniform_block.binding_points[index].buffer.buffer;
         return glxx_get_int_1;
      case GL_UNIFORM_BUFFER_START:
      case GL_UNIFORM_BUFFER_SIZE:
         if (state->uniform_block.binding_points[index].buffer.obj != NULL)
         {
            if (target == GL_UNIFORM_BUFFER_START)
               int64s[0] = state->uniform_block.binding_points[index].offset;
            else
               // Internally -1 means full size. In GL API 0 means full size
               int64s[0] = (state->uniform_block.binding_points[index].size == -1)
                  ? 0
                  : state->uniform_block.binding_points[index].size;
         }
         else
         {
            /* If no buffer is bound the return should be 0 */
            int64s[0] = 0;
         }
         return glxx_get_int64_1;
      default:
         unreachable();
      }

   case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
   case GL_TRANSFORM_FEEDBACK_BUFFER_START:
   case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
   {
      if (index >= GLXX_CONFIG_MAX_TF_SEPARATE_ATTRIBS)
         return glxx_get_invalid_index_0;
      const GLXX_TRANSFORM_FEEDBACK_T *tf = glxx_get_bound_tf(state);
      switch (target)
      {
      case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
         ints[0] = tf->binding_points[index].buffer.buffer;
         return glxx_get_int_1;
      case GL_TRANSFORM_FEEDBACK_BUFFER_START:
         int64s[0] = tf->binding_points[index].offset;
         return glxx_get_int64_1;
      case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
         // Internally -1 means full size. In GL API 0 means full size
         int64s[0] = (tf->binding_points[index].size == -1) ? 0 : tf->binding_points[index].size;
         return glxx_get_int64_1;
      default:
         unreachable();
      }
   }

#if KHRN_GLES31_DRIVER
   case GL_SHADER_STORAGE_BUFFER_BINDING:
   case GL_SHADER_STORAGE_BUFFER_START:
   case GL_SHADER_STORAGE_BUFFER_SIZE:
      if (index >= GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS)
         return glxx_get_invalid_index_0;
      switch (target)
      {
      case GL_SHADER_STORAGE_BUFFER_BINDING:
         ints[0] = state->ssbo.binding_points[index].buffer.buffer;
         return glxx_get_int_1;
      case GL_SHADER_STORAGE_BUFFER_START:
      case GL_SHADER_STORAGE_BUFFER_SIZE:
         if (state->ssbo.binding_points[index].buffer.obj != NULL)
         {
            if (target == GL_SHADER_STORAGE_BUFFER_START)
               int64s[0] = state->ssbo.binding_points[index].offset;
            else
               // Internally -1 means full size. In GL API 0 means full size
               int64s[0] = (state->ssbo.binding_points[index].size == -1)
                  ? 0
                  : state->ssbo.binding_points[index].size;
         }
         else
         {
            /* If no buffer is bound the return should be 0 */
            int64s[0] = 0;
         }
         return glxx_get_int64_1;
      default:
         unreachable();
      }

   case GL_ATOMIC_COUNTER_BUFFER_BINDING:
   case GL_ATOMIC_COUNTER_BUFFER_START:
   case GL_ATOMIC_COUNTER_BUFFER_SIZE:
      if (index >= GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS)
         return glxx_get_invalid_index_0;
      switch (target)
      {
      case GL_ATOMIC_COUNTER_BUFFER_BINDING:
         ints[0] = state->atomic_counter.binding_points[index].buffer.buffer;
         return glxx_get_int_1;
      case GL_ATOMIC_COUNTER_BUFFER_START:
      case GL_ATOMIC_COUNTER_BUFFER_SIZE:
         if (state->atomic_counter.binding_points[index].buffer.obj != NULL)
         {
            if (target == GL_ATOMIC_COUNTER_BUFFER_START)
               int64s[0] = state->atomic_counter.binding_points[index].offset;
            else
               // Internally -1 means full size. In GL API 0 means full size
               int64s[0] = (state->atomic_counter.binding_points[index].size == -1)
                  ? 0
                  : state->atomic_counter.binding_points[index].size;
         }
         else
         {
            /* If no buffer is bound the return should be 0 */
            int64s[0] = 0;
         }
         return glxx_get_int64_1;
      default:
         unreachable();
      }

   case GL_IMAGE_BINDING_NAME:
   case GL_IMAGE_BINDING_LEVEL:
   case GL_IMAGE_BINDING_LAYER:
   case GL_IMAGE_BINDING_LAYERED:
   case GL_IMAGE_BINDING_ACCESS:
   case GL_IMAGE_BINDING_FORMAT:
      if (index >= GLXX_CONFIG_MAX_IMAGE_UNITS)
         return glxx_get_invalid_index_0;
      switch (target)
      {
      case GL_IMAGE_BINDING_NAME:
         ints[0] = state->image_unit[index].texture ? state->image_unit[index].texture->name : 0;
         return glxx_get_int_1;
      case GL_IMAGE_BINDING_LEVEL:
         ints[0] = state->image_unit[index].level;
         return glxx_get_int_1;
      case GL_IMAGE_BINDING_LAYER:
         ints[0] = state->image_unit[index].layer;
         return glxx_get_int_1;
      case GL_IMAGE_BINDING_LAYERED:
         booleans[0] = state->image_unit[index].layered;
         return glxx_get_bool_1;
      case GL_IMAGE_BINDING_ACCESS:
         ints[0] = state->image_unit[index].access;
         return glxx_get_int_1;
      case GL_IMAGE_BINDING_FORMAT:
         ints[0] = state->image_unit[index].internalformat;
         return glxx_get_int_1;
      default:
         unreachable();
      }

   case GL_VERTEX_BINDING_OFFSET:
   case GL_VERTEX_BINDING_DIVISOR:
   case GL_VERTEX_BINDING_STRIDE:
   case GL_VERTEX_BINDING_BUFFER:
      if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS)
         return glxx_get_invalid_index_0;
      switch (target)
      {
      case GL_VERTEX_BINDING_OFFSET:
         int64s[0] = state->vao.bound->vbos[index].offset;
         return glxx_get_int64_1;
      case GL_VERTEX_BINDING_DIVISOR:
         ints[0] = state->vao.bound->vbos[index].divisor;
         return glxx_get_int_1;
      case GL_VERTEX_BINDING_STRIDE:
         ints[0] = state->vao.bound->vbos[index].stride;
         return glxx_get_int_1;
      case GL_VERTEX_BINDING_BUFFER:
         ints[0] = state->vao.bound->vbos[index].buffer ? state->vao.bound->vbos[index].buffer->name : 0;
         return glxx_get_int_1;
      default:
         unreachable();
      }

   case GL_SAMPLE_MASK_VALUE:
      if (index >= GLXX_CONFIG_MAX_SAMPLE_WORDS)
         return glxx_get_invalid_index_0;
      ints[0] = state->sample_mask.mask[index];
      return glxx_get_int_1;

   case GL_MAX_COMPUTE_WORK_GROUP_COUNT:
   case GL_MAX_COMPUTE_WORK_GROUP_SIZE:
      if (index >= 3)
         return glxx_get_invalid_index_0;
      switch (target)
      {
      case GL_MAX_COMPUTE_WORK_GROUP_COUNT:
         ints[0] = GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT;
         return glxx_get_int_1;
      case GL_MAX_COMPUTE_WORK_GROUP_SIZE:
      {
         static const int v[3] = { GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_X,
                                   GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_Y,
                                   GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_Z };
         ints[0] = v[index];
         return glxx_get_int_1;
      }
      default:
         unreachable();
      }
#endif

#if V3D_VER_AT_LEAST(4,0,2,0)
   case GL_BLEND:
   case GL_BLEND_EQUATION_RGB:
   case GL_BLEND_EQUATION_ALPHA:
   case GL_BLEND_SRC_RGB:
   case GL_BLEND_SRC_ALPHA:
   case GL_BLEND_DST_RGB:
   case GL_BLEND_DST_ALPHA:
   case GL_COLOR_WRITEMASK:
      if (index >= GLXX_MAX_RENDER_TARGETS)
         return glxx_get_invalid_index_0;
      switch (target)
      {
      case GL_BLEND:
         booleans[0] = (state->blend.rt_enables >> index) & 1;
         return glxx_get_bool_1;
      case GL_BLEND_EQUATION_RGB:
         ints[0] = untranslate_blend_equation(state->blend.rt_cfgs[index].color_eqn);
         return glxx_get_int_1;
      case GL_BLEND_EQUATION_ALPHA:
         ints[0] = untranslate_blend_equation(state->blend.rt_cfgs[index].alpha_eqn);
         return glxx_get_int_1;
      case GL_BLEND_SRC_RGB:
         ints[0] = untranslate_blend_func(state->blend.rt_cfgs[index].src_rgb);
         return glxx_get_int_1;
      case GL_BLEND_SRC_ALPHA:
         ints[0] = untranslate_blend_func(state->blend.rt_cfgs[index].src_alpha);
         return glxx_get_int_1;
      case GL_BLEND_DST_RGB:
         ints[0] = untranslate_blend_func(state->blend.rt_cfgs[index].dst_rgb);
         return glxx_get_int_1;
      case GL_BLEND_DST_ALPHA:
         ints[0] = untranslate_blend_func(state->blend.rt_cfgs[index].dst_alpha);
         return glxx_get_int_1;
      case GL_COLOR_WRITEMASK:
      {
         uint32_t mask = state->color_write >> (index * 4);
         for (unsigned i = 0; i != 4; ++i)
            booleans[i] = (mask >> i) & 1;
         return glxx_get_bool_0 + 4;
      }
      default:
         unreachable();
      }
#endif

   default:
      return glxx_get_invalid_enum_0;
   }
}

#if KHRN_GLES31_DRIVER

GLenum glxx_get_booleans_i(const GLXX_SERVER_STATE_T *state, GLenum pname, GLuint index, GLboolean *params)
{
   glxx_get_union u;
   glxx_get_type_count type_count = glxx_get_params_and_type_gl3x_indexed(state, pname, index, params, u.ints, u.int64s);
   return glxx_get_union_to_bools(params, type_count, &u);
}

GL_API void GL_APIENTRY glGetBooleani_v(GLenum target, GLuint index, GLboolean *data)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   GLenum error = glxx_get_booleans_i(state, target, index, data);
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

#endif

GL_API void GL_APIENTRY glGetIntegeri_v(GLenum target, GLuint index, GLint* data)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   glxx_get_union u;
   glxx_get_type_count type_count = glxx_get_params_and_type_gl3x_indexed(state, target, index, u.bools, data, u.int64s);
   GLenum error = glxx_get_union_to_ints(data, type_count, &u);
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetInteger64i_v(GLenum target, GLuint index, GLint64* data)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   glxx_get_union u;
   glxx_get_type_count type_count = glxx_get_params_and_type_gl3x_indexed(state, target, index, u.bools, u.ints, data);
   GLenum error = glxx_get_union_to_int64s(data, type_count, &u);
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetInternalformativ (GLenum target,
                                               GLenum internalformat,
                                               GLenum pname,
                                               GLsizei bufSize,
                                               GLint* params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state) return;

   bool renderable = glxx_is_color_renderable_internalformat(internalformat) ||
                     glxx_is_depth_renderable_internalformat(internalformat) ||
                     glxx_is_stencil_renderable_internalformat(internalformat);

   if (!renderable || (target != GL_RENDERBUFFER && (!KHRN_GLES31_DRIVER || !glxx_tex_target_is_multisample(target))))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (bufSize < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (bufSize == 0)
   {
      goto end;   /* No space to write anything, so done. */
   }

   glxx_ms_mode max_ms_mode = glxx_max_ms_mode_for_internalformat(internalformat);

   switch (pname) {
      case GL_NUM_SAMPLE_COUNTS:
         if (max_ms_mode == GLXX_NO_MS) params[0] = 0;
         else             params[0] = 1;
         break;
      case GL_SAMPLES:
         params[0] = max_ms_mode;
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         goto end;
   }

end:
   glxx_unlock_server_state();
}

#if KHRN_GLES31_DRIVER

/* the order and location of samples in TLB is (in 1/8s from bottom-left):
 *      1 2 3 4 5 6 7
 *     _______________
 *  7  |_|_|3|_|_|_|_|
 *  6  |_|_|_|_|_|_|_|
 *  5  |_|_|_|_|_|_|2|
 *  4  |_|_|_|_|_|_|_|
 *  3  |1| |_|_|_|_|_|
 *  2  |_|_|_|_|_|_|_|
 *  1  |_|_|_|_|0|_|_|
 *
 */
GL_APICALL void GL_APIENTRY glGetMultisamplefv (GLenum pname, GLuint index,
      GLfloat *val)
{
   GLenum error = GL_NO_ERROR;
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (pname != GL_SAMPLE_POSITION)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   /* if fb is not complete, samples(ms_mode) is undefined */
   glxx_ms_mode ms_mode = glxx_fb_get_ms_mode(state->bound_draw_framebuffer);
   if (index >= (unsigned) ms_mode)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }
   assert(ms_mode == GLXX_4X_MS);

   switch(index)
   {
      case 0:
         val[0] = 5; val[1] = 1;
         break;
      case 1:
         val[0] = 1; val[1] = 3;
         break;
      case 2:
         val[0] = 7; val[1] = 5;
         break;
      case 3:
         val[0] = 3; val[1] = 7;
         break;
      default:
         unreachable();
   }
   val[0] *= 1.0/8;
   val[1] *= 1.0/8;
end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   glxx_unlock_server_state();
}

#endif
