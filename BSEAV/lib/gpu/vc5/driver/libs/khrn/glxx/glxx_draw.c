/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../common/khrn_int_common.h"
#include "../common/khrn_int_util.h"
#include "../common/khrn_resource.h"
#include "../common/khrn_counters.h"
#include "../common/khrn_render_state.h"
#include "../common/khrn_fmem.h"
#include "../gl20/gl20_program.h"
#include "glxx_draw.h"
#include "glxx_server.h"
#include "glxx_server_internal.h"
#include "glxx_server_pipeline.h"
#include "glxx_server_texture.h"
#include "glxx_hw.h"
#include "glxx_inner.h"
#include "../egl/egl_image.h"
#include "libs/util/dglenum/dglenum.h"
#include "../gl11/gl11_draw.h"
#include "libs/platform/gmem.h"
#include "libs/util/profile/profile.h"
#include "libs/core/v3d/v3d_shadrec.h"

LOG_DEFAULT_CAT("glxx_draw")

#if !V3D_VER_AT_LEAST(4,1,34,0)
static bool calc_attribs_max(glxx_attribs_max *attribs_max,
      const GLXX_VAO_T *vao, uint32_t attribs_live, unsigned base_instance);
#endif

static bool get_indices(glxx_hw_indices *indices,
   GLXX_HW_RENDER_STATE_T *rs, const GLXX_VAO_T *vao,
   const glxx_hw_draw *draw, const void *raw_indices);

static bool get_vbs(bool *skip, glxx_hw_vb vbs[GLXX_CONFIG_MAX_VERTEX_ATTRIBS],
   GLXX_HW_RENDER_STATE_T *rs, const GLXX_VAO_T *vao,
   const glxx_hw_draw *draw, const void *raw_indices,
   bool prim_restart, uint32_t attribs_live);

static inline GLboolean is_index_type(GLenum type)
{
   return (type == GL_UNSIGNED_BYTE) ||
          (type == GL_UNSIGNED_SHORT) ||
          (type == GL_UNSIGNED_INT);
}

#if GLXX_HAS_TNG
static inline bool is_prim_mode_with_adj(GLenum mode)
{
   switch(mode)
   {
      case GL_LINES_ADJACENCY:
      case GL_LINE_STRIP_ADJACENCY:
      case GL_TRIANGLES_ADJACENCY:
      case GL_TRIANGLE_STRIP_ADJACENCY:
         return true;
      default:
         return false;
   }
}
#endif

static inline bool is_mode(GLenum mode, bool is_gl11)
{
   switch (mode)
   {
      case GL_POINTS:
      case GL_LINES:
      case GL_LINE_LOOP:
      case GL_LINE_STRIP:
      case GL_TRIANGLES:
      case GL_TRIANGLE_STRIP:
      case GL_TRIANGLE_FAN:
         return true;
#if GLXX_HAS_TNG
      case GL_PATCHES:
      case GL_LINES_ADJACENCY:
      case GL_LINE_STRIP_ADJACENCY:
      case GL_TRIANGLES_ADJACENCY:
      case GL_TRIANGLE_STRIP_ADJACENCY:
         return !is_gl11;
#endif
      default:
         return false;
   }
}

static bool check_raw_draw_params(GLXX_SERVER_STATE_T *state, const GLXX_DRAW_RAW_T *draw)
{
   if (!is_mode(draw->mode, IS_GL_11(state)))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return false;
   }

   if (draw->end < draw->start)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      return false;
   }

   if (!draw->is_indirect && (draw->count < 0 || draw->instance_count < 0))
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      return false;
   }

   if (draw->is_draw_arrays)
   {
      if (!draw->is_indirect && (draw->first < 0))
      {
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
         return false;
      }
   }
   else
   {
      if (!is_index_type(draw->index_type))
      {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         return false;
      }

      // Aligned indices are only required if they come from a buffer.
      if(state->vao.bound->element_array_binding.buffer)
      {
         if (!glxx_is_aligned(draw->index_type, (size_t)draw->indices))
         {
            glxx_server_state_set_error(state, GL_INVALID_VALUE);
            return false;
         }
      }
   }

   if (draw->is_indirect)
   {
      if (draw->num_indirect < 0 || draw->indirect_stride < 0)
      {
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
         return false;
      }

      if ((uintptr_t)draw->indirect > (v3d_size_t)-1)
      {
         // Buffer offset is way out of range. Can't even fit in glxx_hw_draw field.
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }
   }

   return true;
}

static void hw_draw_params_from_raw(glxx_hw_draw *draw, const GLXX_DRAW_RAW_T *raw)
{
   draw->mode = (GLXX_PRIMITIVE_T)raw->mode;
   draw->count = raw->count;
   draw->is_draw_arrays = raw->is_draw_arrays;
   draw->index_type = (GLXX_INDEX_T)raw->index_type;
   draw->first = raw->first;
   draw->instance_count = raw->instance_count;
   draw->basevertex = raw->basevertex;
   draw->baseinstance = raw->baseinstance;
   draw->is_indirect = raw->is_indirect;
   draw->num_indirect = raw->num_indirect;
   draw->indirect_stride = raw->indirect_stride;
   draw->indirect_offset = (v3d_size_t)(uintptr_t)raw->indirect;
}

static bool all_enabled_attrs_have_vbo_bound(const GLXX_VAO_T *vao)
{
   for (unsigned i = 0; i != GLXX_CONFIG_MAX_VERTEX_ATTRIBS; ++i)
   {
      const GLXX_ATTRIB_CONFIG_T *attr = &vao->attrib_config[i];
      if (attr->enabled && !vao->vbos[attr->vbo_index].buffer)
         return false;
   }
   return true;
}

static gmem_handle_t alloc_and_sync_dynamic_read(khrn_fmem *fmem,
   size_t size, size_t align,
   v3d_barrier_flags bin_rw_flags, v3d_barrier_flags render_rw_flags,
   const char *desc)
{
   gmem_handle_t handle = gmem_alloc_and_map(size, align,
      GMEM_USAGE_V3D_READ | GMEM_USAGE_HINT_DYNAMIC, desc);
   if (!handle)
      return NULL;

   if (!khrn_fmem_record_handle(fmem, handle))
   {
      gmem_free(handle);
      return NULL;
   }

   khrn_fmem_sync(fmem, handle, bin_rw_flags, render_rw_flags);

   return handle;
}

static v3d_addr_t dynamic_read_copy(khrn_fmem *fmem,
   size_t size, size_t align, const void* data,
   v3d_barrier_flags bin_rw_flags, v3d_barrier_flags render_rw_flags,
   const char *desc)
{
   // Try to put ver small allocations in fmem blocks first.
   if (size < 1024)
   {
      v3d_addr_t addr;
      void* ptr = khrn_fmem_data(&addr, fmem, size, align);
      if (ptr)
      {
         memcpy(ptr, data, size);
         return addr;
      }
   }

   gmem_handle_t handle = alloc_and_sync_dynamic_read(fmem, size, align, bin_rw_flags, render_rw_flags, desc);
   if (!handle)
      return 0;
   memcpy(gmem_get_ptr(handle), data, size);
   gmem_flush_mapped_buffer(handle);
   return gmem_get_addr(handle);
}

static void get_index_range(
   uint32_t *min_out,
   uint32_t *max_out,
   v3d_size_t count,
   unsigned per_index_size,
   const void *indices,
   bool prim_restart)
{
   uint32_t min = UINT32_MAX;
   uint32_t max = 0;

   switch (per_index_size)
   {
   case 1:{
      uint8_t *u = (uint8_t *)indices;
      for (v3d_size_t i = 0; i != count; ++i)
      {
         if (!prim_restart || u[i] != 0xff)
         {
            min = gfx_umin(min, u[i]);
            max = gfx_umax(max, u[i]);
         }
      }
      break;}

   case 2:{
      uint16_t *u = (uint16_t *)indices;
      for (v3d_size_t i = 0; i != count; ++i)
         if (!prim_restart || u[i] != 0xffff)
         {
            min = gfx_umin(min, u[i]);
            max = gfx_umax(max, u[i]);
         }
      break;}

   case 4:{
      uint32_t *u = (uint32_t *)indices;
      for (v3d_size_t i = 0; i != count; ++i)
         if (!prim_restart || u[i] != 0xffffffff)
         {
            min = gfx_umin(min, u[i]);
            max = gfx_umax(max, u[i]);
         }
      break;}

   default:
      unreachable();
   }

   *min_out = min;
   *max_out = max;
}

static bool get_draw_index_range(uint32_t *min, uint32_t *max, const GLXX_VAO_T* vao,
   const glxx_hw_draw *draw, const void *raw_indices, bool prim_restart)
{
   assert(!draw->is_indirect);

   if (draw->is_draw_arrays)
   {
      *max = draw->first + draw->count - 1;
      *min = draw->first;
      return true;
   }

   unsigned per_index_size = glxx_get_index_type_size(draw->index_type);

   // Handle non client-side indices?
   if (vao->element_array_binding.buffer != 0)
   {
      GLXX_BUFFER_T *buffer = vao->element_array_binding.obj;
      assert(buffer != NULL && buffer->enabled);

      // expect higher level API to perform basic alignment and range checks
      v3d_size_t offset = (v3d_size_t)(uintptr_t)raw_indices;
      assert((offset % per_index_size) == 0);
      assert((offset + per_index_size*draw->count) <= buffer->size);

      raw_indices = khrn_resource_read_now(buffer->resource, offset, per_index_size * draw->count);
      if (!raw_indices)
         return false;
   }

   get_index_range(min, max, draw->count, per_index_size, raw_indices, prim_restart);
   return true;
}

static bool get_client_vbs(
   glxx_hw_vb vbs[GLXX_CONFIG_MAX_VERTEX_ATTRIBS],
   khrn_fmem *fmem,
   const GLXX_VAO_T *vao,
   uint32_t attribs_live,
   uint32_t instance_count,
   uint32_t base_instance,
   uint32_t min_index,
   uint32_t max_index)
{
   struct arr_pointer {
      const char *start;
      const char *end;
      const char *orig_start;
      unsigned merged;
   } pointers[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];

   assert(instance_count > 0);

   /* we can have client arrays pointers only when vertex array object is 0 */
   assert(vao->name == 0);

   // Autoclif can't cope with V3D addresses that lie outside a buffer.
 #if KHRN_DEBUG
   if (khrn_options.autoclif_enabled)
      min_index = 0;
 #endif

   unsigned max_attribs = gfx_msb(attribs_live) + 1;
   for (unsigned i = 0; i != max_attribs; ++i)
   {
      if (!(attribs_live & (1 << i)))
         continue;

      struct arr_pointer *curr = &pointers[i];
      const GLXX_ATTRIB_CONFIG_T *attr = &vao->attrib_config[i];
      const GLXX_VBO_BINDING_T *vbo = &vao->vbos[attr->vbo_index];
      assert(attr->enabled && vbo->buffer == NULL);

      /* This attribute has no storage attached to it */
      /* The draw call should be cancelled */
      if (attr->pointer == NULL)
      {
         log_warn("%s: active and enabled attrib without storage or buffer bound", __FUNCTION__);
         return false;
      }

      uint32_t attrib_min = !vbo->divisor ? min_index : base_instance;
      uint32_t attrib_max = !vbo->divisor ? max_index : base_instance + (instance_count - 1)/vbo->divisor;

      const char* start = (const char*)attr->pointer;
      curr->orig_start = start;
      curr->start = start + attrib_min*attr->stride;
      curr->end = start + attrib_max*attr->stride + attr->total_size;
      curr->merged = ~0u;
      vbs[i].stride = attr->stride;
      vbs[i].divisor = vbo->divisor;
    #if V3D_VER_AT_LEAST(4,1,34,0)
      vbs[i].max_index = attrib_max;
    #endif

      /* if some array overlap, we merge them */
      for (unsigned k = 0; k < i; k++)
      {
         if (!(attribs_live & (1 << k)))
            continue;

         struct arr_pointer *existing = &pointers[k];
         if (existing->merged == ~0u && curr->end >= existing->start && curr->start <= existing->end)
         {
            /* set the new start and end and make invalid the existing element so it doesn't matter anymore */
            curr->start = GFX_MIN(curr->start, existing->start);
            curr->end = GFX_MAX(curr->end, existing->end);
            existing->merged = i;
         }
      }
   }

   for (unsigned i = max_attribs; i-- != 0; )
   {
      if (!(attribs_live & (1 << i)))
         continue;

      if (pointers[i].merged == ~0u)
      {
         struct arr_pointer *curr = &pointers[i];

         vbs[i].addr = dynamic_read_copy(fmem,
            curr->end - curr->start, V3D_ATTR_REC_ALIGN, curr->start,
            V3D_BARRIER_VCD_READ, V3D_BARRIER_VCD_READ,
            "client-side vertex data");
         if (!vbs[i].addr)
            return false;

         vbs[i].addr = v3d_addr_offset_wrap(vbs[i].addr, curr->orig_start - curr->start);
      }
      else
      {
         /* update this array that was merged into another array */
         unsigned k = pointers[i].merged;
         while (pointers[k].merged != ~0u)
            k = pointers[k].merged;
         assert(k > i);
         vbs[i].addr = v3d_addr_offset_wrap(vbs[k].addr, pointers[i].orig_start - pointers[k].orig_start);
      }
   }

   return true;
}

#if GLXX_HAS_TNG
static GLXX_PRIMITIVE_T glxx_get_gs_draw_mode(const GLSL_PROGRAM_T *p, GLXX_PRIMITIVE_T input_mode)
{
   if (glsl_program_has_stage(p, SHADER_TESS_EVALUATION))
   {
      assert(input_mode == GL_PATCHES);

      if      (p->ir->tess_point_mode)                        return GL_POINTS;
      else if (p->ir->tess_mode == V3D_CL_TESS_TYPE_ISOLINES) return GL_LINES;
      else                                                    return GL_TRIANGLES;
   }
   else
      return input_mode;
}
#endif

GLXX_PRIMITIVE_T glxx_get_rast_draw_mode(const GLSL_PROGRAM_T *p, GLXX_PRIMITIVE_T input_mode)
{
#if GLXX_HAS_TNG
   if (glsl_program_has_stage(p, SHADER_GEOMETRY))
   {
      switch(p->ir->gs_out) {
         case V3D_CL_GEOM_PRIM_TYPE_POINTS:         return GL_POINTS;
         case V3D_CL_GEOM_PRIM_TYPE_LINE_STRIP:     return GL_LINES;
         case V3D_CL_GEOM_PRIM_TYPE_TRIANGLE_STRIP: return GL_TRIANGLES;
         default: unreachable();                    return GL_NONE;
      }
   }
   else
   {
      GLXX_PRIMITIVE_T gs_input = glxx_get_gs_draw_mode(p, input_mode);
      switch(gs_input) {
         case GL_LINES_ADJACENCY:          return GL_LINES;
         case GL_LINE_STRIP_ADJACENCY:     return GL_LINE_STRIP;
         case GL_TRIANGLES_ADJACENCY:      return GL_TRIANGLES;
         case GL_TRIANGLE_STRIP_ADJACENCY: return GL_TRIANGLE_STRIP;
         default:                          return gs_input;
      }
   }
#else
   return input_mode;
#endif
}

static bool check_valid_tf_draw(GLXX_SERVER_STATE_T *state, const glxx_hw_draw *draw)
{
   assert(state->transform_feedback.in_use);
   assert(gl20_program_common_get(state)->transform_feedback.varying_count > 0);

#if !GLXX_HAS_TNG
   if(draw->is_indirect || !draw->is_draw_arrays)
      return false;
#endif

   const GLXX_TRANSFORM_FEEDBACK_T *tf = state->transform_feedback.bound;
   const GLSL_PROGRAM_T *program = gl20_program_common_get(state)->linked_glsl_program;
   GLXX_PRIMITIVE_T used_draw_mode = glxx_get_rast_draw_mode(program, draw->mode);

   if (!glxx_tf_draw_mode_allowed(tf, used_draw_mode))
      return false;

#if !V3D_VER_AT_LEAST(4,0,2,0)
   /* we are using draw->count and instance->count; valid only if !is_indirect); */
   assert(!draw->is_indirect);
   const GLXX_PROGRAM_TFF_POST_LINK_T *ptf = &gl20_program_common_get(state)->transform_feedback;
   if (!glxx_tf_capture_to_buffers_no_overflow(tf, ptf, draw->count, draw->instance_count))
      return false;
#endif

   return true;
}

static AdvancedBlendQualifier convert_to_blend_qualifier(uint32_t b)
{
   switch (b)
   {
   case GLSL_ADV_BLEND_MULTIPLY       : return ADV_BLEND_MULTIPLY;
   case GLSL_ADV_BLEND_SCREEN         : return ADV_BLEND_SCREEN;
   case GLSL_ADV_BLEND_OVERLAY        : return ADV_BLEND_OVERLAY;
   case GLSL_ADV_BLEND_DARKEN         : return ADV_BLEND_DARKEN;
   case GLSL_ADV_BLEND_LIGHTEN        : return ADV_BLEND_LIGHTEN;
   case GLSL_ADV_BLEND_COLORDODGE     : return ADV_BLEND_COLORDODGE;
   case GLSL_ADV_BLEND_COLORBURN      : return ADV_BLEND_COLORBURN;
   case GLSL_ADV_BLEND_HARDLIGHT      : return ADV_BLEND_HARDLIGHT;
   case GLSL_ADV_BLEND_SOFTLIGHT      : return ADV_BLEND_SOFTLIGHT;
   case GLSL_ADV_BLEND_DIFFERENCE     : return ADV_BLEND_DIFFERENCE;
   case GLSL_ADV_BLEND_EXCLUSION      : return ADV_BLEND_EXCLUSION;
   case GLSL_ADV_BLEND_HSL_HUE        : return ADV_BLEND_HSL_HUE;
   case GLSL_ADV_BLEND_HSL_SATURATION : return ADV_BLEND_HSL_SATURATION;
   case GLSL_ADV_BLEND_HSL_COLOR      : return ADV_BLEND_HSL_COLOR;
   case GLSL_ADV_BLEND_HSL_LUMINOSITY : return ADV_BLEND_HSL_LUMINOSITY;
   default:
      assert(0); return 0;
   }
}

static bool check_valid_advanced_blend(const GLXX_SERVER_STATE_T *state, const GLSL_PROGRAM_T *program)
{
   if (!glxx_advanced_blend_eqn(state))
      return true;

   GLXX_FRAMEBUFFER_T *fb = state->bound_draw_framebuffer;

   for (uint32_t i = 1; i < GLXX_MAX_RENDER_TARGETS; ++i)
      if (fb->draw_buffer[i] != GL_NONE)
         return false;

   // The layout qualifier in the shader must match the blend mode
   // (it would work for us anyway, but need to conform to spec)
   if ((program->ir->abq & convert_to_blend_qualifier(glxx_advanced_blend_eqn(state))) == 0)
      return false;

   return true;
}

#if GLXX_HAS_TNG
static bool check_gs_input_mode(GLXX_PRIMITIVE_T draw_mode, const GLSL_PROGRAM_T *program)
{
   if (!glsl_program_has_stage(program, SHADER_GEOMETRY))
      return true;

   GLXX_PRIMITIVE_T gs_input = glxx_get_gs_draw_mode(program, draw_mode);
   switch (program->ir->gs_in) {
      case GS_IN_POINTS:
         return gs_input == GL_POINTS;
      case GS_IN_LINES:
         return gs_input == GL_LINES || gs_input == GL_LINE_STRIP || gs_input == GL_LINE_LOOP;
      case GS_IN_TRIANGLES:
         return gs_input == GL_TRIANGLES || gs_input == GL_TRIANGLE_STRIP || gs_input == GL_TRIANGLE_FAN;
      case GS_IN_LINES_ADJ:
         return gs_input == GL_LINES_ADJACENCY || gs_input == GL_LINE_STRIP_ADJACENCY;
      case GS_IN_TRIS_ADJ:
         return gs_input == GL_TRIANGLES_ADJACENCY || gs_input == GL_TRIANGLE_STRIP_ADJACENCY;
      default:
         unreachable(); return false;
   }
}
#endif

/* check that we have everything we need to proceed with the draw call
 * (framebuffer complete, vertex attrib enabled for es11, etc) */
static bool check_draw_state(GLXX_SERVER_STATE_T * state, const glxx_hw_draw *draw)
{
   GLXX_VAO_T *vao = state->vao.bound;

   if (IS_GL_11(state))
   {
      if ((state->gl11.statebits.v_enable & GL11_MPAL_M))
      {
         if (vao->attrib_config[GL11_IX_MATRIX_WEIGHT].size !=
             vao->attrib_config[GL11_IX_MATRIX_INDEX].size )
         {
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
            return false;
         }
      }
      if (!(vao->attrib_config[GL11_IX_VERTEX].enabled))
      {
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
         return false;
      }
   }

   if (!glxx_fb_is_complete(state->bound_draw_framebuffer, &state->fences))
   {
      glxx_server_state_set_error(state, GL_INVALID_FRAMEBUFFER_OPERATION);
      return false;
   }

   for (int i=0; i<GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++) {
      if (vao->vbos[i].buffer && vao->vbos[i].buffer->mapped_pointer) {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }
   }

   if (vao->element_array_binding.obj != NULL && vao->element_array_binding.obj->mapped_pointer) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return false;
   }

   if (draw->is_indirect)
   {
      GLXX_VAO_T *vao = state->vao.bound;
      GLXX_BUFFER_T *indirect_buffer = state->bound_buffer[GLXX_BUFTGT_DRAW_INDIRECT].obj;
      if (vao->name == 0 || !all_enabled_attrs_have_vbo_bound(vao) || !indirect_buffer)
      {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }

      v3d_size_t indirect_bufsize = glxx_buffer_get_size(indirect_buffer);
      const size_t sizeof_indirect = (draw->is_draw_arrays ? 4 : 5)*sizeof(uint32_t);
      size_t data_required = (draw->num_indirect == 0) ? 0 :
         ((draw->num_indirect-1)*draw->indirect_stride + sizeof_indirect);
      if (indirect_bufsize < draw->indirect_offset ||
          indirect_bufsize - draw->indirect_offset < data_required)
      {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }

      if (draw->indirect_offset & 3)
      {
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
         return false;
      }
   }

   /* Validate the program last because not having a valid program is not an
    * error and we can't exit before error checks.
    */
   if (IS_GL_11(state))
      return true;

   GLSL_PROGRAM_T const* linked_program = NULL;
   if (state->current_program != NULL)
   {
      /* If we have a current program it must be valid */
      GL20_PROGRAM_COMMON_T* program_common = &state->current_program->common;
      if (!gl20_validate_program(state, program_common))
      {
         log_info("%s: program validate failed", __FUNCTION__);
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }

      if (!glsl_program_has_stage(program_common->linked_glsl_program, SHADER_VERTEX)) {
         if (glsl_program_has_stage(program_common->linked_glsl_program, SHADER_TESS_CONTROL)    ||
             glsl_program_has_stage(program_common->linked_glsl_program, SHADER_TESS_EVALUATION) ||
             glsl_program_has_stage(program_common->linked_glsl_program, SHADER_GEOMETRY)         )
         {
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
            return false;
         }
      } else
         linked_program = program_common->linked_glsl_program;
   }
   else if (state->pipelines.bound)
   {
      /* If we have a bound program pipeline (and no current program), the pipeline must be valid */
      if (!glxx_pipeline_validate(state->pipelines.bound))
      {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }

      if (glxx_pipeline_create_graphics_common(state->pipelines.bound))
         linked_program = state->pipelines.bound->common.linked_glsl_program;
   }

   if (!linked_program)
   {
#if GLXX_HAS_TNG
      if (draw->mode == GL_PATCHES || is_prim_mode_with_adj(draw->mode))
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
#endif
      /* As per the ES2 and ES3 specifications, this is not an error
      condition. glUseProgram(0) does not cause errors when it
      is used, but the result of such uses is undefined.
      ES 2.0.25: p31, para 2; ES 3.0.2: p51, para 1.
      */
      return false;
   }

   if (!check_valid_advanced_blend(state, linked_program))
   {
      log_info("%s: advanced blend validation failed", __FUNCTION__);
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return false;
   }

#if GLXX_HAS_TNG
   // Must have both TCS and TES for patches, otherwise neither.
   unsigned has_ts_mask = (unsigned)glsl_program_has_stage(linked_program, SHADER_TESS_CONTROL) << 0
                        | (unsigned)glsl_program_has_stage(linked_program, SHADER_TESS_EVALUATION) << 1;
   unsigned valid_mask = draw->mode == GL_PATCHES ? 3u : 0u;
   if (has_ts_mask != valid_mask)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return false;
   }

   if (!check_gs_input_mode(draw->mode, linked_program))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return false;
   }
#endif

   if (state->transform_feedback.in_use)
   {
      if (!check_valid_tf_draw(state, draw))
      {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }
   }

   assert(glsl_program_has_stage(linked_program, SHADER_VERTEX));
   return true;
}

#if !V3D_VER_AT_LEAST(4,1,34,0)
/* Clamp draw parameters to make sure we never exceed buffer bounds */
static bool clamp_draw_count(glxx_hw_draw *draw, const void *raw_indices,
      const GLXX_VAO_T *vao, unsigned int max_index)
{
   /* We should use V3D_CL_INDIRECT_PRIMITIVE_LIMITS for indirect draws */
   assert(!draw->is_indirect);

   if (draw->is_draw_arrays)
   {
      /* For DrawArrays clamp the (implicit) indices to fit in the array buffers */
      /* Abandon the draw if all the indices are outside the buffer */
      if (draw->first > max_index) return false;

      assert(draw->count > 0);
      if ((draw->count - 1) > (max_index - draw->first)) {
         draw->count = max_index - draw->first + 1;
      }
   }
   else
   {
      /* For DrawElements clamp the count to the size of the element buffer */
      GLXX_BUFFER_T *buffer = vao->element_array_binding.obj;
      if (buffer != NULL) {
         uint32_t type_size = glxx_get_index_type_size(draw->index_type);
         v3d_size_t buffer_size = glxx_buffer_get_size(buffer);
         v3d_size_t offset = (v3d_size_t)(uintptr_t)raw_indices;
         v3d_size_t max_indices = (offset < buffer_size) ?
            ((buffer_size - offset) / type_size) : 0;

         /* Skip the draw call entirely if we can't fetch a single index */
         if (max_indices == 0)
            return false;

         /* Clamp to the last full index that we can fetch */
         if (draw->count > max_indices)
            draw->count = max_indices;
      }
   }

   return true;
}
#endif

static bool has_possible_self_write_conflicting_preprocess_read(
   glxx_hw_render_state const* rs,
   GLXX_SERVER_STATE_T const* state)
{
   if (!rs->tf.used)
      return false;

   GL20_PROGRAM_COMMON_T const* program_common = gl20_program_common_get(state);
   GLSL_PROGRAM_T const* glsl_program = program_common->linked_glsl_program;
   for (unsigned i = 0; i != glsl_program->num_uniform_blocks; ++i)
   {
      unsigned block_index_start = glsl_program->uniform_blocks[i].index;
      unsigned block_index_end = block_index_start + glsl_program->uniform_blocks[i].array_length;
      for (unsigned b = block_index_start; b != block_index_end; ++b)
      {
         unsigned binding_point = program_common->ubo_binding_point[b];
         khrn_resource* res = state->uniform_block.binding_points[binding_point].buffer.obj->resource;
         if ((khrn_resource_get_write_stages(res, (khrn_render_state *)rs) & KHRN_STAGE_BIN) &&
            (res->last_tf_write_count > 0))
         {
            return true;
         }
      }
   }
   return false;
}

/* All GL error have been raised before calling. It is safe to mutate any state
 * but the only error that can be raised is GL_OUT_OF_MEMORY.
 */
static void draw_arrays_or_elements(GLXX_SERVER_STATE_T *state,
   glxx_hw_draw *draw, const void *raw_indices)
{
   if (draw->is_indirect ?
         (draw->num_indirect == 0) :
         (draw->instance_count == 0 || draw->count == 0))
      /* Early out, not an error */
      return;

   khrn_driver_incr_counters(KHRN_PERF_DRAW_CALLS);

   GLXX_HW_FRAMEBUFFER_T hw_fb;
   if (!glxx_init_hw_framebuffer(state->bound_draw_framebuffer, &hw_fb,
            &state->fences))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      return;
   }

   GLXX_HW_RENDER_STATE_T *rs = NULL;
   for (; ;)
   {
      bool existing;
      if (!(rs = glxx_install_rs(state, &hw_fb, &existing, false)))
      {
         log_warn("%s: installing framebuffer failed", __FUNCTION__);
         break;
      }

      if (has_possible_self_write_conflicting_preprocess_read(rs, state))
      {
         glxx_hw_render_state_flush(rs);
         continue;
      }

      if (!state->transform_feedback.in_use)
         break;

      // If this render state is already reading from the buffers written to by TF,
      // then we will flush and get a new one.
      bool requires_flush = false;
      if (glxx_tf_add_resource_writes(state->transform_feedback.bound,
               &gl20_program_common_get(state)->transform_feedback,
               rs, &requires_flush))
         break;

      // If didn't require a flush, then ran out of memory.
      if (!requires_flush)
      {
         glxx_hw_discard_frame(rs);
         rs = NULL;
         break;
      }

      // Flush and try again.
      glxx_hw_render_state_flush(rs);
   }

   glxx_destroy_hw_framebuffer(&hw_fb);

   if (!rs)
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      return;
   }

   khrn_render_state_disallow_flush((khrn_render_state*)rs);

   GLXX_VAO_T *vao = state->vao.bound;

   if (IS_GL_11(state) && khrn_render_state_set_contains(state->dirty.stuff, rs))
   {
      GL11_STATE_T *s = &state->gl11;

      /* TODO: This is pretty terrible here */
      s->statebits.vertex = (s->statebits.vertex & ~GL11_MPAL_M) |
                            (vao->attrib_config[GL11_IX_MATRIX_WEIGHT].size << GL11_MPAL_S);

      /* Install vertex and fragment in shader key. Texture is done elsewhere */
      s->shaderkey.vertex   = gl11_compute_vertex(s, draw->mode);
      s->shaderkey.fragment = gl11_compute_fragment(s, draw->mode, &rs->installed_fb);
      s->shaderkey.points   = gl11_is_points(draw->mode);

      gl11_compute_texture_key(state, gl11_is_points(draw->mode));

      /* If shaderkey says we'll access variables, populate them now */
      gl11_cache_uniforms(state, &rs->fmem);
   }

   uint32_t attribs_live = glxx_get_attribs_live(state);

#if !V3D_VER_AT_LEAST(4,1,34,0)
   glxx_attribs_max attribs_max;
   if (!calc_attribs_max(&attribs_max, vao, attribs_live, draw->baseinstance))
      goto end; //nothing to do

   if (!draw->is_indirect)
   {
      // clamp instance count
      assert(draw->instance_count > 0);
      if (draw->instance_count - 1 > attribs_max.instance)
        draw->instance_count = attribs_max.instance + 1;

      if (!clamp_draw_count(draw, raw_indices, vao , attribs_max.index))
         goto end; //nothing to do
   }
#endif

   glxx_hw_indices indices;
   if (!draw->is_draw_arrays && !get_indices(&indices, rs, vao, draw, raw_indices))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto end;
   }

   bool skip;
   glxx_hw_vb vbs[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   if (!get_vbs(&skip, vbs, rs, vao, draw, raw_indices,
            state->caps.primitive_restart && draw->mode != GL_PATCHES, attribs_live))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto end;
   }
   if (skip)
      goto end;

   if (!glxx_hw_draw_triangles(state, rs, draw, &indices,
            state->vao.bound->attrib_config, vbs
#if !V3D_VER_AT_LEAST(4,1,34,0)
            ,&attribs_max
#endif
      ))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      rs = NULL; // frame was discarded.
      return;
   }

end:
   khrn_render_state_allow_flush((khrn_render_state*)rs);

#if KHRN_DEBUG
   if (khrn_options.flush_after_draw)
      khrn_render_state_flush((khrn_render_state*)rs);
#endif
}

void glintDrawArraysOrElements(GLXX_SERVER_STATE_T *state, const GLXX_DRAW_RAW_T *draw_raw)
{
   if (!check_raw_draw_params(state, draw_raw))
      return;

   glxx_hw_draw draw;
   hw_draw_params_from_raw(&draw, draw_raw);

   if (!check_draw_state(state, &draw))
      return; /* Error set by check_draw_state */

   draw_arrays_or_elements(state, &draw, draw_raw->indices);
}

GL_API void GL_APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
   {
      return;
   }

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .count = count,
      .is_draw_arrays = true,
      .first = first};
   glintDrawArraysOrElements(state, &draw);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glDrawElements(GLenum mode,
   GLsizei count, GLenum index_type, const GLvoid *indices)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
   {
      return;
   }

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .count = count,
      .index_type = index_type,
      .indices = indices};
   glintDrawArraysOrElements(state, &draw);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glDrawArraysInstanced(GLenum mode,
                                              GLint first,
                                              GLsizei count,
                                              GLsizei instanceCount)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
   {
      return;
   }

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .count = count,
      .is_draw_arrays = true,
      .first = first,
      .instance_count = instanceCount};
   glintDrawArraysOrElements(state, &draw);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glDrawElementsInstanced(GLenum mode,
                                                GLsizei count,
                                                GLenum index_type,
                                                const GLvoid *indices,
                                                GLsizei instanceCount)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
   {
      return;
   }

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .count = count,
      .index_type = index_type,
      .indices = indices,
      .instance_count = instanceCount};
   glintDrawArraysOrElements(state, &draw);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count,
                                            GLenum index_type, const GLvoid* indices)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
   {
      return;
   }

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .start = start,
      .end = end,
      .count = count,
      .index_type = index_type,
      .indices = indices};
   glintDrawArraysOrElements(state, &draw);

   glxx_unlock_server_state();
}

#if V3D_VER_AT_LEAST(3,3,0,0)

GL_APICALL void GL_APIENTRY glDrawArraysIndirect(GLenum mode, const GLvoid *indirect)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
   {
      return;
   }

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .is_draw_arrays = true,
      .is_indirect = true,
      .indirect = indirect};
   glintDrawArraysOrElements(state, &draw);

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glDrawElementsIndirect(GLenum mode, GLenum index_type,
                                                   const GLvoid *indirect)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
   {
      return;
   }

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .index_type = index_type,
      .is_indirect = true,
      .indirect = indirect};
   glintDrawArraysOrElements(state, &draw);

   glxx_unlock_server_state();
}

#endif

/* return false if the buffer does not have enough space to fit one vertex */
static bool get_max_buffer_index(uint32_t *max_index,
      v3d_size_t buffer_size, size_t offset, uint32_t attrib_size, uint32_t actual_stride)
{
   assert(attrib_size > 0);

   if ((attrib_size > buffer_size) || (offset > (buffer_size - attrib_size)))
      return false;    //Not even the first vertex will fit in the buffer
   else if(actual_stride == 0)
   {
#if V3D_VER_AT_LEAST(4,1,34,0)
      //hw ignores max_index if stride is 0
      *max_index = 0;
#else
      /* Any index is valid with a stride of zero */
      *max_index = UINT32_MAX;
#endif
   }
   else
   {
      *max_index = (buffer_size - offset - attrib_size) / actual_stride;
      assert(offset + (*max_index)   * actual_stride + attrib_size <= buffer_size);
      assert(offset + (*max_index+1) * actual_stride + attrib_size >  buffer_size);
   }
   return true;
}

#if !V3D_VER_AT_LEAST(4,1,34,0)
static bool calc_attribs_max(glxx_attribs_max *attribs_max,
      const GLXX_VAO_T *vao, uint32_t attribs_live, unsigned base_instance)
{

   unsigned max_index = GLXX_CONFIG_MAX_ELEMENT_INDEX;
   unsigned max_instance = UINT_MAX;

   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      const GLXX_ATTRIB_CONFIG_T *attr = &vao->attrib_config[i];
      const GLXX_VBO_BINDING_T *vbo = &vao->vbos[attr->vbo_index];

      if ( ! (attribs_live & (1<<i) && attr->enabled) )
         continue;

      /* We only know the sizes of buffers, not client-side attributes */
      GLXX_BUFFER_T *buffer = vbo->buffer;
      if (buffer == NULL) continue;

      uint32_t attr_max_index;
      if (!get_max_buffer_index(&attr_max_index, glxx_buffer_get_size(buffer), vbo->offset +
               attr->relative_offset, attr->total_size, vbo->stride))
         return false;

      if (vbo->divisor == 0)
         max_index = gfx_umin(max_index, attr_max_index);
      else
      {
         if (base_instance > attr_max_index)
            return false;
         unsigned attr_max_instance = (attr_max_index - base_instance + 1) * vbo->divisor - 1;
         max_instance = gfx_umin(max_instance, attr_max_instance);
      }
   }

   attribs_max->index = max_index;
   attribs_max->instance = max_instance;
   attribs_max->base_instance = base_instance;

   return true;
}
#endif

static bool get_indices(glxx_hw_indices *indices,
   GLXX_HW_RENDER_STATE_T *rs, const GLXX_VAO_T *vao,
   const glxx_hw_draw *draw, const void *raw_indices)
{
   assert(!draw->is_draw_arrays);

   GLXX_BUFFER_T *buffer = vao->element_array_binding.obj;
   if (buffer)
   {
      /* Server-side indices */
      if (!glxx_hw_tf_aware_sync_res(rs,
            buffer->resource, V3D_BARRIER_CLE_PRIMIND_READ, V3D_BARRIER_NO_ACCESS))
         return false;
      indices->addr = gmem_get_addr(buffer->resource->handle);
      indices->size = glxx_buffer_get_size(buffer);
      indices->offset = (v3d_size_t)(uintptr_t)raw_indices;
   }
   else
   {
      /* Client-side indices */
      assert(!draw->is_indirect);
      indices->size = draw->count * glxx_get_index_type_size(draw->index_type);
      indices->addr = dynamic_read_copy(&rs->fmem,
         indices->size, V3D_INDICES_REC_ALIGN, raw_indices,
         V3D_BARRIER_CLE_PRIMIND_READ, V3D_BARRIER_NO_ACCESS,
         "client-side indices");
      if (!indices->addr)
         return false;
      indices->offset = 0;
   }

#if !V3D_VER_AT_LEAST(4,1,34,0)
   if (!draw->is_indirect)
      assert(indices->offset < indices->size); //checked by clamp_draw_count
#endif
   return true;
}

static bool get_vbs(bool *skip, glxx_hw_vb vbs[GLXX_CONFIG_MAX_VERTEX_ATTRIBS],
   GLXX_HW_RENDER_STATE_T *rs, const GLXX_VAO_T *vao,
   const glxx_hw_draw *draw, const void *raw_indices,
   bool prim_restart, uint32_t attribs_live)
{
   assert(!(attribs_live & ~gfx_mask(GLXX_CONFIG_MAX_VERTEX_ATTRIBS)));
   *skip = false;

   uint32_t client_attribs_live = 0;
   for (unsigned i = 0; attribs_live != 0; ++i, attribs_live >>= 1)
   {
      const GLXX_ATTRIB_CONFIG_T *attr = &vao->attrib_config[i];
      const GLXX_VBO_BINDING_T *vbo = &vao->vbos[attr->vbo_index];

      if (!(attribs_live & 1) || !attr->enabled)
         continue;

      if (!vbo->buffer)
      {
         client_attribs_live |= 1 << i;
         continue;
      }

#if V3D_VER_AT_LEAST(4,1,34,0)
      if (!get_max_buffer_index(&vbs[i].max_index, glxx_buffer_get_size(vbo->buffer), vbo->offset +
               attr->relative_offset, attr->total_size, vbo->stride))
      {
         *skip = true; // No enough data for one attribute, skip the draw call
         return true;
      }
#endif
      khrn_resource* resouce = vbo->buffer->resource;
      if (!glxx_hw_tf_aware_sync_res(rs, resouce, V3D_BARRIER_VCD_READ, V3D_BARRIER_VCD_READ))
         return false;

      vbs[i].addr = gmem_get_addr(resouce->handle) + vbo->offset + attr->relative_offset;
      vbs[i].stride = vbo->stride;
      vbs[i].divisor = vbo->divisor;
   }

   if (client_attribs_live != 0)
   {
      if (vao->name != 0)
      {
         // There is an active and enabled attribute with no associated VBO.
         // The client-side pointer in the VAO should be used instead, however,
         // as it is impossible for any VAO but the default (0) to have
         // client-side pointers, we know it is NULL. Abort the draw call early
         // here.
         log_warn("%s: active and enabled attrib without storage or buffer bound", __FUNCTION__);
         return false;
      }

      assert(!draw->is_indirect);

      uint32_t min_index;
      uint32_t max_index;
      if (!get_draw_index_range(&min_index, &max_index, vao, draw, raw_indices, prim_restart))
         return false;
      if (max_index < min_index)
      {
         *skip = true; // No indices no drawing
         return true;
      }

      if (!get_client_vbs(vbs, &rs->fmem, vao, client_attribs_live,
            draw->instance_count, draw->baseinstance, min_index, max_index))
         return false;
   }

   return true;
}

struct tex_rect
{
   bool valid;
   float s, t, sw, sh;
};
struct screen_rect
{
   float x, y, dw, dh;
};

static bool create_and_record_drawtex_attribs(
      khrn_fmem *fmem,
      const struct screen_rect *screen, float z_w,
      const struct tex_rect tex_rects[GL11_CONFIG_MAX_TEXTURE_UNITS],
      GLXX_ATTRIB_CONFIG_T attribs[GLXX_CONFIG_MAX_VERTEX_ATTRIBS],
      glxx_hw_vb vbs[GLXX_CONFIG_MAX_VERTEX_ATTRIBS])
{
   unsigned n_valid_textures = 0;
   for (unsigned i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if (tex_rects[i].valid)
         n_valid_textures++;
   }

   unsigned v_size = 3 ; /* number of coordinates per vertex */
   unsigned t_size = 2; /* number of coordinates per array element for texture */

   size_t stride = sizeof(float) * (v_size + n_valid_textures * t_size);
   /* we draw a rectangle */
   size_t vertex_count = 4;
   size_t size = vertex_count * stride;

   gmem_handle_t v_handle = alloc_and_sync_dynamic_read(fmem,
      size, V3D_ATTR_REC_ALIGN,
      V3D_BARRIER_VCD_READ, V3D_BARRIER_VCD_READ,
      "drawtex vertices");
   if (!v_handle)
      return false;

   /* glVertexPointer(v_size, GL_FLOAT, stride, v_handle) */
   {
      GLXX_ATTRIB_CONFIG_T *attrib = &attribs[GL11_IX_VERTEX];
      memset(attrib, 0, sizeof(GLXX_ATTRIB_CONFIG_T));
      attrib->gl_type = GL_FLOAT;
      attrib->v3d_type = V3D_ATTR_TYPE_FLOAT;
      attrib->is_signed = false;
      attrib->norm = false;
      attrib->size = v_size;
      attrib->total_size =  sizeof(float) * v_size;
      attrib->enabled = true;

      glxx_hw_vb *vb = &vbs[GL11_IX_VERTEX];
      vb->addr = gmem_get_addr(v_handle);
      vb->stride = stride;
      vb->divisor = 0;
#if V3D_VER_AT_LEAST(4,1,34,0)
      assert(vertex_count > 0);
      vb->max_index = (vertex_count -1);
#endif
   }

   /* glTexCoordPointer(t_size, GL_FLOAT, stride, (float*)v_handle + v_size + t_size * preceding_tex_units_enabled) */
   unsigned prec_txt_enabled = 0;
   for (unsigned i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if (tex_rects[i].valid)
      {
         GLXX_ATTRIB_CONFIG_T *attrib = &attribs[GL11_IX_TEXTURE_COORD + i];
         memset(attrib, 0, sizeof(GLXX_ATTRIB_CONFIG_T));
         attrib->gl_type = GL_FLOAT;
         attrib->v3d_type = V3D_ATTR_TYPE_FLOAT;
         attrib->is_signed = false;
         attrib->norm = false;
         attrib->size = t_size;
         attrib->total_size =  sizeof(float) * t_size;
         attrib->enabled = true;

         glxx_hw_vb *vb = &vbs[GL11_IX_TEXTURE_COORD + i];
         vb->addr = gmem_get_addr(v_handle) + (sizeof(float) * (v_size + prec_txt_enabled * t_size));
         vb->stride = stride;
         vb->divisor = 0;
#if V3D_VER_AT_LEAST(4,1,34,0)
         assert(vertex_count > 0);
         vb->max_index = (vertex_count -1);
#endif

         prec_txt_enabled++;
      }
   }

   /* Just fill in the vertex coordinates intersperesed with the texture
    * coordinates (2 for each texture unit */
   float *p = gmem_get_ptr(v_handle);
   *p++ = screen->x;
   *p++ = screen->y;
   *p++ = z_w;
   for (unsigned i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if (tex_rects[i].valid)
      {
         *p++ = tex_rects[i].s;
         *p++ = tex_rects[i].t;
      }
   }

   *p++ = screen->x + screen->dw;
   *p++ = screen->y;
   *p++ = z_w;
   for (unsigned i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if(tex_rects[i].valid)
      {
         *p++ = tex_rects[i].s + tex_rects[i].sw;
         *p++ = tex_rects[i].t;
      }
   }

   *p++ = screen->x + screen->dw;
   *p++ = screen->y + screen->dh;
   *p++ = z_w;
   for (unsigned i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if(tex_rects[i].valid)
      {
         *p++ = tex_rects[i].s + tex_rects[i].sw;
         *p++ = tex_rects[i].t + tex_rects[i].sh;
      }
   }

   *p++ = screen->x;
   *p++ = screen->y + screen->dh;
   *p++ = z_w;
   for(unsigned i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if(tex_rects[i].valid)
      {
         *p++ = tex_rects[i].s;
         *p++ = tex_rects[i].t + tex_rects[i].sh;
      }
   }

   gmem_flush_mapped_buffer(v_handle);

   return true;
}

static void calculate_tex_rect(const GLXX_SERVER_STATE_T *state, unsigned
      tex_unit, struct tex_rect* rect)
{
   const GL11_TEXUNIT_T *texunit = &state->gl11.texunits[tex_unit];
   GLXX_TEXTURE_T *texture;
   khrn_image *img;
   unsigned img_width, img_height;
   enum glxx_tex_target tex_target;

   assert(texunit->target_enabled_2D ||
         texunit->target_enabled_EXTERNAL_OES);

   if (texunit->target_enabled_EXTERNAL_OES)
      tex_target = GL_TEXTURE_EXTERNAL_OES;
   else
      tex_target = GL_TEXTURE_2D;

   texture =  glxx_textures_get_texture(&state->bound_texture[tex_unit],
         tex_target);

   img  = texture->img[0][0];
   assert(img);
   img_width = khrn_image_get_width(img);
   img_height = khrn_image_get_height(img);

   rect->valid = true;
   rect->s = ((float)texture->crop_rect.Ucr) / (float)img_width;
   rect->t = ((float)texture->crop_rect.Vcr) / (float)img_height;
   rect->sw = ((float)texture->crop_rect.Wcr) / (float) img_width;
   rect->sh = ((float)texture->crop_rect.Hcr)/ (float) img_height;
}

bool glxx_drawtex(GLXX_SERVER_STATE_T *state, float x_s, float y_s, float z_w,
                  float w_s, float h_s)
{
   assert(IS_GL_11(state));

   GLXX_HW_RENDER_STATE_T* rs = NULL;
   {
      GLXX_HW_FRAMEBUFFER_T hw_fb;
      if (glxx_init_hw_framebuffer(state->bound_draw_framebuffer, &hw_fb, &state->fences))
      {
         bool existing;
         rs = glxx_install_rs(state, &hw_fb, &existing, false);
         glxx_destroy_hw_framebuffer(&hw_fb);
      }
   }
   if (rs == NULL)
      return false;

   khrn_render_state_disallow_flush((khrn_render_state*)rs);

   GLXX_DRAW_RAW_T draw_raw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = GL_TRIANGLE_FAN,
      .count = 4,
      .is_draw_arrays = true};
   glxx_hw_draw draw;
   hw_draw_params_from_raw(&draw, &draw_raw);

   {
      GL11_STATE_T *s = &state->gl11;

      /* Install vertex and fragment in shader key. Texture is done elsewhere */
      s->shaderkey.vertex   = GL11_DRAW_TEX;
      s->shaderkey.fragment = gl11_compute_fragment(s, draw.mode, &rs->installed_fb);
      s->shaderkey.points   = gl11_is_points(draw.mode);

      gl11_compute_texture_key(state, gl11_is_points(draw.mode));

      /* If shaderkey says we'll access variables, populate them now */
      gl11_cache_uniforms(state, &rs->fmem);
   }

#if !V3D_VER_AT_LEAST(4,1,34,0)
   glxx_attribs_max attribs_max = {
      .index = GLXX_CONFIG_MAX_ELEMENT_INDEX,
      .instance = INT_MAX };
#endif

   struct tex_rect tex_rects[GL11_CONFIG_MAX_TEXTURE_UNITS];
   memset(tex_rects, 0, sizeof(tex_rects));
   for (unsigned i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if (state->gl11.shaderkey.texture[i] & GL11_TEX_ENABLE)
      {
         calculate_tex_rect(state, i, tex_rects + i);
      }
   }

   struct screen_rect s_rect = { .x = 2.0f * x_s/state->viewport.width - 1.0f,
                                 .y = 2.0f * y_s/state->viewport.height - 1.0f,
                                 .dw = 2.0f * w_s/state->viewport.width,
                                 .dh = 2.0f * h_s/state->viewport.height };

   GLXX_ATTRIB_CONFIG_T attribs[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   glxx_hw_vb vbs[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   memset(attribs, 0, sizeof(attribs));
   memset(vbs, 0, sizeof(vbs));
   if (!create_and_record_drawtex_attribs(&rs->fmem, &s_rect, z_w, tex_rects, attribs, vbs))
   {
      khrn_render_state_allow_flush((khrn_render_state*)rs);
      return false;
   }

   if (!glxx_hw_draw_triangles(state, rs, &draw, NULL, attribs, vbs
#if !V3D_VER_AT_LEAST(4,1,34,0)
         , &attribs_max
#endif
         ))
      return false; // frame was discarded, rs no longer valid, so no cleanup to do...

   /* The shaderkey will be set up wrong, if nothing else */
   khrn_render_state_set_add(&state->dirty.stuff, rs);

   khrn_render_state_allow_flush((khrn_render_state*)rs);

   return true;
}
