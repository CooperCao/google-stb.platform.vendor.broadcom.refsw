/*=============================================================================
Copyright (c) 2011 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Handles calls to glDrawArrays and glDrawElements.
=============================================================================*/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "middleware/khronos/common/khrn_interlock.h"
#include "middleware/khronos/common/khrn_counters.h"
#include "middleware/khronos/common/khrn_render_state.h"
#include "middleware/khronos/common/khrn_fmem.h"
#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/glxx/glxx_draw.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_server_internal.h"
#include "middleware/khronos/glxx/glxx_server_texture.h"
#include "middleware/khronos/glxx/glxx_hw.h"
#include "middleware/khronos/glxx/glxx_log.h"
#include "middleware/khronos/glxx/glxx_inner.h"
#include "middleware/khronos/egl/egl_image.h"
#include "interface/khronos/tools/dglenum/dglenum.h"
#include "middleware/khronos/gl11/gl11_draw.h"
#include "gmem.h"

static bool attrib_handles_and_offsets(GLXX_HW_RENDER_STATE_T *rs, const GLXX_VAO_T *vao,
                                       uint32_t attribs_live,
                                       GLXX_VERTEX_BUFFER_CONFIG_T *vb_config,
                                       GLXX_VERTEX_POINTERS_T *vertex_pointers);
static bool get_server_attribs_max(const GLXX_VAO_T *vao, uint32_t attribs_live,
                            GLXX_ATTRIBS_MAX *attribs_max);

static bool clamp_indices_and_instances(GLXX_DRAW_T *draw,
                                        const GLXX_ATTRIBS_MAX *attribs_max);
static bool index_stuff(GLXX_HW_RENDER_STATE_T *rs, const GLXX_VAO_T * vao,
      const GLXX_DRAW_T *draw, GLXX_STORAGE_T *indices);

static GLboolean is_index_type(GLenum type)
{
   return (type == GL_UNSIGNED_BYTE) ||
          (type == GL_UNSIGNED_SHORT) ||
          (type == GL_UNSIGNED_INT);
}

static bool is_mode(GLenum mode)
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
      default:
         return false;
   }
}

static int calc_length(unsigned max, int size, GLXX_INDEX_T type, int stride)
{
   int type_size = khrn_get_type_size( (int)type, size );
   return type_size + max * (stride ? stride : type_size);
}

static bool check_raw_draw_params(GLXX_SERVER_STATE_T *state, const GLXX_DRAW_RAW_T *draw)
{
   if (!is_mode(draw->mode))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return false;
   }

   if (draw->max_index < draw->min_index)
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
   }

   return true;
}

static void draw_params_from_raw(GLXX_DRAW_T *draw, const GLXX_DRAW_RAW_T *raw)
{
   draw->mode = (GLXX_PRIMITIVE_T)raw->mode;
   draw->min_index = gfx_umin(raw->min_index, GLXX_CONFIG_MAX_ELEMENT_INDEX);
   draw->max_index = gfx_umin(raw->max_index, GLXX_CONFIG_MAX_ELEMENT_INDEX);
   draw->count = raw->count;
   draw->is_draw_arrays = raw->is_draw_arrays;
   draw->index_type = (GLXX_INDEX_T)raw->index_type;
   draw->indices = raw->indices;
   draw->first = raw->first;
   draw->instance_count = raw->instance_count;
   draw->basevertex = raw->basevertex;
   draw->baseinstance = raw->baseinstance;
   draw->is_indirect = raw->is_indirect;
   draw->num_indirect = raw->num_indirect;
   draw->indirect_stride = raw->indirect_stride;
   draw->indirect_offset = (uintptr_t)raw->indirect;
}

/* Return whether any currently active attribute has no buffer bound */
static bool have_client_vertex_pointers(const GLXX_VAO_T *vao, uint32_t attribs_live)
{
   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      if ( attribs_live & (1 << i))
      {
         const GLXX_ATTRIB_CONFIG_T *attr = &vao->attrib_config[i];
         if(attr->enabled && vao->vbos[attr->vbo_index].buffer == NULL)
         {
            return true;
         }
      }
   }
   return false;
}

static gmem_handle_t alloc_and_copy_data(size_t size, size_t align, const void* data)
{
   gmem_handle_t handle = gmem_alloc(
      size,
      align,
      GMEM_USAGE_CPU | GMEM_USAGE_V3D_READ | GMEM_USAGE_HINT_DYNAMIC,
      "client side data");

   if (handle != GMEM_HANDLE_INVALID)
   {
      void *ptr = gmem_map_and_begin_cpu_access(handle, GMEM_SYNC_CPU_WRITE);
      if (ptr)
      {
         memcpy(ptr, data, size);
         gmem_end_cpu_access_and_unmap(handle, GMEM_SYNC_CPU_WRITE);
      }
      else
      {
         gmem_free(handle);
         handle = GMEM_HANDLE_INVALID;
      }
   }
   return handle;
}

static bool has_client_side_indices(const GLXX_VAO_T *vao, bool is_draw_arrays)
{
   assert(vao);

   if (is_draw_arrays || vao->element_array_binding.buffer != 0)
      return false;
   return true;
}

static bool store_client_indices(const GLXX_VAO_T *vao,
      const GLXX_DRAW_T *draw, GLXX_STORAGE_T *indices)
{
   int size;

   assert(has_client_side_indices(vao, draw->is_draw_arrays));
   assert(!draw->is_indirect);

   size = draw->count * khrn_get_type_size(draw->index_type, 1);
   indices->handle = alloc_and_copy_data(size, V3D_INDICES_REC_ALIGN, draw->indices);
   if (indices->handle == GMEM_HANDLE_INVALID)
      return false;
   indices->offset = 0;
   indices->needs_freeing = true;
   indices->size = size;
   return true;
}

static int find_max_index(const GLXX_VAO_T * vao, const GLXX_DRAW_T *draw, bool primitive_restart,
                          // Only used for client side indices.
                          GLXX_STORAGE_T *indices)
{
   assert(!draw->is_indirect);

   if (draw->is_draw_arrays)
   {
      return draw->first + draw->count - 1;
   }

   unsigned per_index_size = khrn_get_type_size(draw->index_type, 1);
   int max;
   if (vao->element_array_binding.buffer == 0)
   {
      // This code appears to be making an assumption that as the buffer has previously
      // been mapped then this call can't fail.
      uint32_t sync_flags = GMEM_SYNC_CPU_READ | GMEM_SYNC_RELAXED;
      void* mapped_indices_copied_from_client = gmem_map_and_begin_cpu_access_range(indices->handle, 0, indices->size, sync_flags);
      assert(mapped_indices_copied_from_client);

      /* client side indices */
      max = find_max(draw->count, per_index_size, mapped_indices_copied_from_client,
            primitive_restart);

      gmem_end_cpu_access_range_and_unmap(indices->handle, 0, indices->size, sync_flags);
   }
   else
   {
      GLXX_BUFFER_T *buffer = vao->element_array_binding.obj;
      assert(buffer != NULL);
      max = glxx_buffer_find_max(buffer, draw->count, per_index_size,
            (uintptr_t)draw->indices, primitive_restart);
   }
   return max;
}

static bool store_client_vertex_pointers(const GLXX_VAO_T *vao,
      unsigned int attribs_live, unsigned int instance_count, unsigned max_index,
      GLXX_VERTEX_POINTERS_T *vertex_pointers)
{
   struct arr_pointer
   {
      char *start;
      char *orig_start;
      int length;
      int index_merged;
   };
   struct arr_pointer pointers[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   unsigned int instance_max, i, k;
   bool res = true;

   assert(instance_count > 0);
   instance_max = instance_count - 1;

   /* we can have client arrays pointers only when vertex array object is 0 */
   assert(vao->name == 0);

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      struct arr_pointer *curr = &pointers[i];

      curr->length = 0;
      curr->index_merged = -1;

      if ( attribs_live & (1 << i))
      {
         const GLXX_ATTRIB_CONFIG_T *attr = &vao->attrib_config[i];
         const GLXX_VBO_BINDING_T *vbo = &vao->vbos[attr->vbo_index];
         if (attr->enabled && vbo->buffer == NULL)
         {
            unsigned attrib_max = (vbo->divisor == 0) ? max_index : (instance_max / vbo->divisor);

            /* This attribute has no storage attached to it */
            /* The draw call should be cancelled */
            if (vbo->pointer == NULL)
            {
               vcos_logc_warn((&glxx_attrib_log), "%s: active and enabled attrib without storage or buffer bound", VCOS_FUNCTION);
               return false;
            }

            curr->start = (char*) vbo->pointer;
            curr->orig_start = (char*) vbo->pointer;
            curr->length = calc_length(attrib_max, attr->size, attr->type, vbo->original_stride);

            /* if some array overlap, we merge them */
            for (k = 0; k < i; k++)
            {
               struct arr_pointer *existing = &pointers[k];

               if (existing->length == 0)
                  continue;

               if ((curr->start + curr->length >= existing->start) && (existing->start + existing->length >= curr->start))
               {
                  char *start, *end;
                  start = GFX_MIN(curr->start, existing->start);
                  end = GFX_MAX(curr->start + curr->length, existing->start + existing->length);
                  /* set the new start and end and make invalid the existing element so it doesn't matter anymore */
                  curr->start = start;
                  curr->length = end - start;
                  existing->length = 0;
                  existing->index_merged = i;
               }
            }
         }
      }
   }

   /* we need to store only the merged arrays (length != 0) */
   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      if (pointers[i].length != 0)
      {
         struct arr_pointer *curr = &pointers[i];
         GLXX_STORAGE_T *arr = &vertex_pointers->array[i];

         arr->handle = alloc_and_copy_data(curr->length, V3D_ATTR_REC_ALIGN, curr->start);
         if (arr->handle == GMEM_HANDLE_INVALID)
         {
            res = false;
            break;
         }
         assert(curr->start <= curr->orig_start);
         arr->offset = curr->orig_start - curr->start;
         arr->needs_freeing = true;
         arr->size = curr->length;
      }
   }

   /* update the arrays that where merged into other array */
   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      if (pointers[i].index_merged != -1)
      {
         int j;

         assert(pointers[i].length == 0);

         j = pointers[i].index_merged;
         while (pointers[j].index_merged != -1)
            j = pointers[j].index_merged;

         assert(vertex_pointers->array[j].handle != GMEM_HANDLE_INVALID);
         vertex_pointers->array[i].handle = vertex_pointers->array[j].handle;
         assert(pointers[j].start <= pointers[i].orig_start);
         vertex_pointers->array[i].offset = pointers[i].orig_start - pointers[j].start;
         vertex_pointers->array[i].size = vertex_pointers->array[j].size;
      }
   }
   return res;
}

static bool check_valid_transform_in_use(GLXX_SERVER_STATE_T *state, const GLXX_DRAW_T *draw)
{
   if (!state->transform_feedback.in_use)
      return true;

   assert(!draw->is_indirect);

   // When transform_feedback is active and not paused:
   // - The error INVALID_OPERATION is also generated by DrawElements,
   //   DrawElementsInstanced, and DrawRangeElements [...] regardless of mode

   // - The error INVALID_OPERATION is generated by DrawArrays and
   //   DrawArraysInstanced if mode is not identical to primitiveMode

   // The error INVALID_OPERATION is generated by DrawArrays and
   // DrawArraysInstanced if recording the vertices of a primitive to the
   // buffer objects being used for transform feedback purposes would result in
   // either exceeding the limits of any buffer object's size, or in exceeding
   // the end position offset + size - 1, as set by BindBufferRange

   if (!draw->is_draw_arrays)
      goto err;

   if (!glxx_tf_validate_draw_arrays(state, draw->mode, draw->count,
            draw->instance_count))
      goto err;

   return true;

err:
   vcos_logc_info((&glxx_draw_log),
      "%s: transform feedback in use, not valid draw arrays mode",
      VCOS_FUNCTION);
   glxx_server_state_set_error(state, GL_INVALID_OPERATION);
   return false;
}

/* check that we have everything we need to proceed with the draw call
 * (framebuffer complete, vertex attrib enabled for es11, etc) */
static bool check_draw_state(GLXX_SERVER_STATE_T * state, const GLXX_DRAW_T *draw)
{
   GLXX_VAO_T *vao;

   vao = state->vao.bound;

   if (IS_GL_11(state))
   {
#if GL_OES_matrix_palette
      if ((state->gl11.statebits.v_enable & GL11_MPAL_M))
      {
         if (vao->attrib_config[GL11_IX_MATRIX_WEIGHT].size !=
             vao->attrib_config[GL11_IX_MATRIX_INDEX].size )
         {
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
            return false;
         }
      }
#endif
      if (!(vao->attrib_config[GL11_IX_VERTEX].enabled))
      {
         vcos_logc_info((&glxx_draw_log),
            "%s: GL11 without position attribute enabled",
            VCOS_FUNCTION);
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
         return false;
      }
   }

   if (!glxx_fb_is_complete(state->bound_draw_framebuffer))
   {
      glxx_server_state_set_error(state, GL_INVALID_FRAMEBUFFER_OPERATION);
      return false;
   }

   if (draw->is_indirect)
   {
      GLXX_BUFFER_T *indirect_buffer = state->bound_buffer[GLXX_BUFTGT_DRAW_INDIRECT].obj;
      GLXX_VAO_T *vao = state->vao.bound;
      const size_t sizeof_indirect = (draw->is_draw_arrays ? 4 : 5)*sizeof(uint32_t);
      size_t data_required = (draw->num_indirect == 0) ? 0 :
         ((draw->num_indirect-1)*draw->indirect_stride + sizeof_indirect);
      size_t indirect_bufsize;
      /* Pass 0xffffffff as the mask because any enabled attrib will give an error */
      /* TODO: This weird masking is a bit shonky */
      bool client_side_vertices = have_client_vertex_pointers(vao, 0xffffffff);

      if (indirect_buffer == NULL) {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }

      indirect_bufsize = glxx_buffer_get_size(indirect_buffer);

      if (vao->name == 0 ||
          indirect_bufsize < draw->indirect_offset ||
          indirect_bufsize - draw->indirect_offset < data_required ||
          client_side_vertices ||
          state->transform_feedback.in_use)
      {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }

      if (draw->indirect_offset & 3) {
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
         return false;
      }
   }

   /* Check for transform feedback errors */
   if (!check_valid_transform_in_use(state, draw))
      return false;  /* This has set its own error */

   /* Validate the program last because not hacing a valid program is not an
    * error and we can't exit before error checks.
    */
   if (!IS_GL_11(state))
   {
      if (state->program == NULL)
      {
         /* As per the ES2 and ES3 specifications, this is not an error
            condition. glUseProgram(0) does not cause errors when it
            is used, but the result of such uses is undefined.
            ES 2.0.25: p31, para 2; ES 3.0.2: p51, para 1.
         */
         return false;
      }

      if (!gl20_validate_program(state, state->program))
      {
         vcos_logc_info((&glxx_draw_log), "%s: program validate failed", VCOS_FUNCTION);
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }
   }

   return true;
}

static void storage_init(GLXX_STORAGE_T *storage)
{
   storage->handle = GMEM_HANDLE_INVALID;
   storage->offset = 0;
   storage->needs_freeing = false;
   storage->size = 0;
}

static void storage_destroy(GLXX_STORAGE_T *storage)
{
   if (storage->needs_freeing)
   {
      assert(storage->handle != GMEM_HANDLE_INVALID);
      gmem_free(storage->handle);
      storage_init(storage);
   }
}

static void vertex_pointers_init(GLXX_VERTEX_POINTERS_T *vertex_pointers)
{
   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      GLXX_STORAGE_T *v = vertex_pointers->array + i;
      storage_init(v);
   }
}

static void vertex_pointers_destroy(GLXX_VERTEX_POINTERS_T *vertex_pointers)
{
   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      GLXX_STORAGE_T *v = vertex_pointers->array + i;
      storage_destroy(v);
   }
}

/* All GL error have been raised before calling. It is safe to mutate any state
 * but the only error that can be raised is GL_OUT_OF_MEMORY.
 */
static void draw_arrays_or_elements(GLXX_SERVER_STATE_T *state, GLXX_DRAW_T *draw)
{
   uint32_t attribs_live;
   GLXX_VAO_T *vao = NULL;
   GLXX_HW_FRAMEBUFFER_T hw_fb;
   GLXX_HW_RENDER_STATE_T *rs = NULL;
   GLXX_VERTEX_BUFFER_CONFIG_T vb_config[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   GLXX_VERTEX_POINTERS_T vertex_pointers;
   GLXX_STORAGE_T indices;

   assert(draw->is_indirect ? (draw->num_indirect > 0) :
      (draw->count > 0 && draw->instance_count > 0));

   khrn_stats_record_start(KHRN_STATS_DRAW_CALL);
   khrn_stats_record_event(KHRN_STATS_DRAW_CALL_COUNT);
   khrn_driver_incr_counters(KHRN_PERF_DRAW_CALLS);

   memset(vb_config, 0, sizeof(vb_config));
   vertex_pointers_init(&vertex_pointers);
   storage_init(&indices);

   vao = state->vao.bound;

   glxx_init_hw_framebuffer(&hw_fb);
   if (!glxx_build_hw_framebuffer(state->bound_draw_framebuffer, &hw_fb))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
   }
   else for (; ;)
   {
      if (!(rs = glxx_install_rs(state, &hw_fb, false)))
      {
         vcos_logc_warn((&glxx_draw_log), "%s: installing framebuffer failed",
               VCOS_FUNCTION);
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
         break;
      }

      // If this render state is already reading from the buffers written to by TF,
      // then we will flush and get a new one.
      bool requires_flush = false;
      if (glxx_tf_add_interlock_writes(state, rs, &requires_flush))
         break;

      // If didn't require a flush, then ran out of memory.
      if (!requires_flush)
      {
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
         glxx_hw_discard_frame(rs);
         rs = NULL;
         break;
      }

      // Flush and try again.
      glxx_hw_render_state_flush(rs);
   }

   glxx_destroy_hw_framebuffer(&hw_fb);

   if(!rs)
      goto end;

   khrn_render_state_disallow_flush((KHRN_RENDER_STATE_T*)rs);

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

   if (has_client_side_indices(vao, draw->is_draw_arrays))
   {
      if (!store_client_indices(vao, draw, &indices))
      {
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
         goto end;
      }
   }

   glxx_get_attribs_live(state, &attribs_live);

   // this is pretty horrible; cached_server_attribs_max is valid if any bits in dirty.stuff are clear
   if (state->dirty.stuff == KHRN_RENDER_STATE_SET_ALL)
   {
      if (!get_server_attribs_max(vao, attribs_live, &state->cached_server_attribs_max))
         goto end;
   }
   GLXX_ATTRIBS_MAX attribs_max = state->cached_server_attribs_max;

   bool have_client_vertices = have_client_vertex_pointers(vao, attribs_live);
   /* We only upload client-side vertices if the default VAO is bound.
    * Otherwise we know the client-side pointers are NULL and the attributes
    * will be ignored anyway.
    */
   if (have_client_vertices && vao->name == 0)
   {
      assert(!draw->is_indirect);

      int max_index = find_max_index(vao, draw, state->caps.primitive_restart, &indices);
      if (max_index < 0)
         // No indices no drawing
         goto end;

      /* Clamp max to client max to avoid wasted data transfer of
       * out-of-range indices for client vertex arrays */
      max_index = MIN((unsigned)max_index, draw->max_index);

      if (!store_client_vertex_pointers(vao, attribs_live,
            draw->instance_count, max_index, &vertex_pointers))
      {
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
         goto end;
      }

      attribs_max.index = MIN(attribs_max.index, (unsigned)max_index);
      attribs_max.instance = MIN(attribs_max.instance, draw->instance_count - 1);
   }

   if(draw->min_index > gfx_umin(draw->max_index, attribs_max.index))
      /* glDrawRangeElements or glDrawRangeElementsBaseVertexEXT:
       * Nothing to draw, early out.
      */
      goto end;

   if (!glxx_calculate_and_hide(state, rs, draw->mode))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto end;
   }

   if (!attrib_handles_and_offsets(rs, vao, attribs_live, vb_config, &vertex_pointers))
   {
      vcos_logc_warn((&glxx_draw_log), "%s: attrib stuff fail", VCOS_FUNCTION);
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto end;
   }

   // If draw indirect is used, HW does limit checking using CLE 44
   if (!draw->is_indirect && !clamp_indices_and_instances(draw, &attribs_max))
      goto end;

   if (!draw->is_draw_arrays && !index_stuff(rs, vao, draw, &indices))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto end;
   }

   if (!glxx_hw_draw_triangles(state, rs, draw,
            state->vao.bound->attrib_config,
            &attribs_max, vb_config,
            &indices, &vertex_pointers))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto end;
   }

end:
   storage_destroy(&indices);
   vertex_pointers_destroy(&vertex_pointers);
   if (rs) khrn_render_state_allow_flush((KHRN_RENDER_STATE_T*)rs);
   khrn_stats_record_end(KHRN_STATS_DRAW_CALL);
}

void glintDrawArraysOrElements(GLXX_SERVER_STATE_T *state, const GLXX_DRAW_RAW_T *draw_raw)
{
   GLXX_DRAW_T draw;

   if (!check_raw_draw_params(state, draw_raw))
      return;

   draw_params_from_raw(&draw, draw_raw);

   if (!check_draw_state(state, &draw))
      return; /* Error set by check_draw_state */

   if (draw.is_indirect ?
         (draw.num_indirect == 0) :
         (draw.instance_count == 0 || draw.count == 0))
      /* Early out, not an error */
      return;

   draw_arrays_or_elements(state, &draw);
}

GL_API void GL_APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
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

   GLXX_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glDrawElements(GLenum mode,
   GLsizei count, GLenum index_type, const GLvoid *indices)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
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

   GLXX_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glDrawArraysInstanced(GLenum mode,
                                              GLint first,
                                              GLsizei count,
                                              GLsizei instanceCount)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
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

   GL30_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glDrawElementsInstanced(GLenum mode,
                                                GLsizei count,
                                                GLenum index_type,
                                                const GLvoid *indices,
                                                GLsizei instanceCount)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
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

   GL30_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count,
                                            GLenum index_type, const GLvoid* indices)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   if (!state)
   {
      return;
   }

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .min_index = start,
      .max_index = end,
      .count = count,
      .index_type = index_type,
      .indices = indices};
   glintDrawArraysOrElements(state, &draw);

   GL30_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glDrawArraysIndirect(GLenum mode, const GLvoid *indirect)
{
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE();
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

   GL31_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glDrawElementsIndirect(GLenum mode, GLenum index_type,
                                                   const GLvoid *indirect)
{
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE();
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

   GL31_UNLOCK_SERVER_STATE();
}

static int get_max_buffer_index(GLXX_BUFFER_T *buffer, size_t offset, uint32_t attrib_size, uint32_t actual_stride)
{
   size_t buffer_size = glxx_buffer_get_size(buffer);

   assert(attrib_size > 0);

   int result;
   if ((attrib_size > buffer_size) || (offset > (buffer_size - attrib_size)))
      result = -1;    //Not even the first vertex will fit in the buffer
   else if(actual_stride == 0)
   {
      /* Any index is valid with a stride of zero */
      return INT_MAX;
   }
   else
   {
      size_t max_index = (buffer_size - offset - attrib_size) / actual_stride;
      assert(offset +  max_index    * actual_stride + attrib_size <= buffer_size);
      assert(offset + (max_index+1) * actual_stride + attrib_size >  buffer_size);

      assert(max_index <= INT_MAX);
      result = (int)max_index;
   }

   return result;
}

static bool attrib_handles_and_offsets(GLXX_HW_RENDER_STATE_T *rs, const GLXX_VAO_T *vao,
                                       uint32_t attribs_live,
                                       GLXX_VERTEX_BUFFER_CONFIG_T* vb_config,
                                       GLXX_VERTEX_POINTERS_T *vertex_pointers)
{
   KHRN_FMEM_T *fmem = &rs->fmem;

   unsigned int i;

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      const GLXX_ATTRIB_CONFIG_T *attr = &vao->attrib_config[i];
      const GLXX_VBO_BINDING_T *vbo = &vao->vbos[attr->vbo_index];
      GLXX_STORAGE_T *vertex_p = &vertex_pointers->array[i];
      GLXX_BUFFER_T *buffer;

      if ( ! (attribs_live & (1<<i) && attr->enabled) )
         continue;

      buffer = vbo->buffer;
      vb_config[i].stride = vbo->stride;
      vb_config[i].divisor = vbo->divisor;

      if (buffer != NULL)
      {
         KHRN_RES_INTERLOCK_T *res_i;

         res_i = glxx_buffer_get_tf_aware_res_interlock(rs, buffer);
         if (res_i == NULL) return false;

         if (!khrn_fmem_record_res_interlock(fmem, res_i, false, ACTION_BOTH))
            return false;

         vertex_p->handle = res_i->handle;
         vertex_p->offset = vbo->offset + attr->relative_offset;
         vertex_p->size = glxx_buffer_get_size(buffer);
      }
      else
      {
         /* this is some client side vertex that we have already stored in
          * vertex_attribs.array[i] */
         assert(vertex_p->handle != GMEM_HANDLE_INVALID);

         if (vertex_p->needs_freeing)
         {
            /* we need to add this buffer to the fmem so it gets freed when we are
             * done with this fmem */
            if (!khrn_fmem_record_handle(fmem, vertex_p->handle))
               return false;

            /* this handle is not ours to free anymore */
            vertex_p->needs_freeing = false;
         }
      }
   }

   return true;
}

/* Return the max index and instance before reading attributes would overrun
 * a buffer
 */
static bool get_server_attribs_max(const GLXX_VAO_T *vao, uint32_t attribs_live,
                            GLXX_ATTRIBS_MAX *attribs_max)
{
   unsigned int max_index = GLXX_CONFIG_MAX_ELEMENT_INDEX;
   unsigned int max_instance = INT_MAX;
   int i;

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      const GLXX_ATTRIB_CONFIG_T *attr = &vao->attrib_config[i];
      const GLXX_VBO_BINDING_T *vbo = &vao->vbos[attr->vbo_index];
      GLXX_BUFFER_T *buffer;

      if ( ! (attribs_live & (1<<i) && attr->enabled) )
         continue;

      buffer = vbo->buffer;

      /* We only know the sizes of buffers, not client-side attributes */
      if (buffer != NULL)
      {
         int buffer_max_index;

         buffer_max_index = get_max_buffer_index(
            buffer, vbo->offset + attr->relative_offset, attr->total_size, vbo->stride);

         if (buffer_max_index == -1) return false;

         assert(buffer_max_index >= 0);
         if (vbo->divisor == 0)
            max_index = MIN(max_index, buffer_max_index);
         else
         {
            unsigned int buffer_max_instance = ((unsigned int)buffer_max_index + 1) * vbo->divisor - 1;
            max_instance = MIN(max_instance, buffer_max_instance);
         }
      }
   }

   attribs_max->index = max_index;
   attribs_max->instance = max_instance;

   return true;
}

/* Clamp draw parameters to make sure we never exceed buffer bounds */
static bool clamp_indices_and_instances(GLXX_DRAW_T *draw,
                                        const GLXX_ATTRIBS_MAX *attribs_max)
{
   /* We should use V3D_CL_INDIRECT_PRIMITIVE_LIMITS for indirect draws */
   assert(!draw->is_indirect);

   assert(draw->instance_count > 0);
   if (draw->instance_count - 1 > attribs_max->instance)
      draw->instance_count = attribs_max->instance + 1;

   /* HW will do the clamping for DrawElements, so clamp only for DrawArrays */
   if (draw->is_draw_arrays)
   {
      /* Abandon the draw if all the indices are outside the buffer */
      if (draw->first > attribs_max->index) return false;

      assert(draw->count > 0);
      if ((draw->count - 1) > (attribs_max->index - draw->first)) {
         draw->count = attribs_max->index - draw->first + 1;
      }
   }

   return true;
}

/* Fills in the indices struct with the indices handle and offset*/
static bool index_stuff(GLXX_HW_RENDER_STATE_T *rs, const GLXX_VAO_T * vao,
                        const GLXX_DRAW_T *draw, GLXX_STORAGE_T *indices)
{
   KHRN_FMEM_T *fmem = &rs->fmem;

   assert(!draw->is_draw_arrays);

   GLXX_BUFFER_T *buffer = vao->element_array_binding.obj;

   if (buffer != NULL)
   {
      int type_size;
      uint32_t buffer_size;
      KHRN_RES_INTERLOCK_T *res_i;

      res_i = glxx_buffer_get_tf_aware_res_interlock(rs, buffer);
      if (res_i == NULL)
         return false;

      indices->handle = res_i->handle;
      indices->offset = (size_t)draw->indices;

      type_size = khrn_get_type_size(draw->index_type, 1);
      buffer_size = glxx_buffer_get_size(buffer);
      indices->size = buffer_size;

      if (indices->offset > buffer_size)
         return false;
      /* This is handled by V3D_CL_INDIRECT_PRIMITIVE_LIMITS for indirect draws */
      if (!draw->is_indirect && ((indices->offset + draw->count * type_size) > buffer_size))
         return false;

      khrn_fmem_record_res_interlock(fmem, res_i, false, ACTION_BOTH);
   }
   else
   {
      /* this was filled in by store_client_indices */
      assert(indices->handle != GMEM_HANDLE_INVALID &&
                  indices->needs_freeing == true);

      /* we need to add this handle to the fmem so it gets freed when we are
       * done with this fmem */
     if (!khrn_fmem_record_handle(fmem, indices->handle))
        return false;

     /* this handle is not ours to free anymore */
     indices->needs_freeing = false;
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
      KHRN_FMEM_T *fmem,
      const struct screen_rect *screen, float z_w,
      const struct tex_rect tex_rects[GL11_CONFIG_MAX_TEXTURE_UNITS],
      GLXX_VERTEX_POINTERS_T *vertex_pointers,
      GLXX_VERTEX_BUFFER_CONFIG_T *vb_config,
      GLXX_ATTRIB_CONFIG_T attrib_config[GLXX_CONFIG_MAX_VERTEX_ATTRIBS])
{
   void *storage = NULL;
   float *p;
   bool ok = false;
   GLXX_ATTRIB_CONFIG_T *v_cfg;
   GLXX_STORAGE_T *v_arr = NULL;
   unsigned n_valid_textures, prec_txt_enabled, t_size, v_size, i;
   size_t size, stride;

   n_valid_textures = 0;
   for (i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if (tex_rects[i].valid)
         n_valid_textures++;
   }

   v_size = 3 ; /* number of coordinates per vertex */
   t_size = 2; /* number of coordinates per array element for texture */

   stride = sizeof(float) * (v_size + n_valid_textures * t_size);
   /* we draw a rectangle */
   size = 4 * stride;

   v_arr = &vertex_pointers->array[GL11_IX_VERTEX];
   v_arr->handle = gmem_alloc(size, V3D_ATTR_REC_ALIGN, GMEM_USAGE_CPU | GMEM_USAGE_V3D_READ | GMEM_USAGE_HINT_DYNAMIC, "drawtex vertices");
   if (v_arr->handle == GMEM_HANDLE_INVALID)
      goto end;
   v_arr->offset = 0;
   v_arr->needs_freeing = true;
   v_arr->size = size;
   vb_config[GL11_IX_VERTEX].stride = stride;
   vb_config[GL11_IX_VERTEX].divisor = 0;

   /* we need to add the vertex handle to the fmem so it gets freed when we are
    * done with this fmem */
   if (!khrn_fmem_record_handle(fmem, v_arr->handle))
      goto end;
   /* this handle is not ours to free anymore */
   v_arr->needs_freeing = false;

   storage = gmem_map_and_begin_cpu_access(v_arr->handle, GMEM_SYNC_CPU_WRITE);
   if (storage == NULL)
      goto end;

   v_cfg = &attrib_config[GL11_IX_VERTEX];
   /* glVertexPointer(3, GL_FLOAT, (3 + n_valid_texture * 2) *sizeof(GL_FLOAT), storage) */
   memset(v_cfg, 0, sizeof(GLXX_ATTRIB_CONFIG_T));
   v_cfg->type = GL_FLOAT;
   v_cfg->norm = false;
   v_cfg->size = v_size;
   v_cfg->total_size =  sizeof(float) * v_size;
   v_cfg->enabled = true;

   /* glTexCoordPointer(size, GL_FLOAT,(3 + n_valid_texture * 2)
    * *sizeof(GL_FLOAT), (float*)storage + 3 + 2 * preceding_tex_units_enabled)
    * */
   prec_txt_enabled = 0;
   for (i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if (tex_rects[i].valid)
      {
         GLXX_STORAGE_T *t_arr;
         GLXX_ATTRIB_CONFIG_T *t_cfg;
         GLXX_VERTEX_BUFFER_CONFIG_T *b_cfg;
         t_cfg = &attrib_config[GL11_IX_TEXTURE_COORD + i];
         b_cfg = &vb_config[GL11_IX_TEXTURE_COORD + i];
         memset(t_cfg, 0, sizeof(GLXX_ATTRIB_CONFIG_T));
         t_cfg->type = GL_FLOAT;
         t_cfg->norm = false;
         t_cfg->size = t_size;
         t_cfg->total_size =  sizeof(float) * t_size;
         t_cfg->enabled = true;

         b_cfg->stride = stride;
         b_cfg->divisor = 0;

         t_arr = &vertex_pointers->array[GL11_IX_TEXTURE_COORD + i];
         t_arr->handle = v_arr->handle;
         t_arr->offset = sizeof(float) * (v_size + prec_txt_enabled * t_size);
         t_arr->size = size;
         t_arr->needs_freeing = false;
         prec_txt_enabled++;
      }
   }

   /* Just fill in the vertex coordinates intersperesed with the texture
    * coordinates (2 for each texture unit */
   p = storage;
   *p++ = screen->x;
   *p++ = screen->y;
   *p++ = z_w;
   for (i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
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
   for (i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
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
   for (i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
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
   for(i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if(tex_rects[i].valid)
      {
         *p++ = tex_rects[i].s;
         *p++ = tex_rects[i].t + tex_rects[i].sh;
      }
   }

   ok = true;

end:
   if (storage != NULL)
      gmem_end_cpu_access_and_unmap(v_arr->handle, GMEM_SYNC_CPU_WRITE);
   return ok;
}

static void calculate_tex_rect(const GLXX_SERVER_STATE_T *state, unsigned
      tex_unit, struct tex_rect* rect)
{
   const GL11_TEXUNIT_T *texunit = &state->gl11.texunits[tex_unit];
   GLXX_TEXTURE_T *texture;
   KHRN_IMAGE_T *img;
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
   GLXX_HW_RENDER_STATE_T *rs = NULL;
   bool ok = false;
   unsigned i;
   GLXX_ATTRIBS_MAX attribs_max;
   GLXX_VERTEX_POINTERS_T vertex_pointers;
   GLXX_VERTEX_BUFFER_CONFIG_T vb_config[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   GLXX_ATTRIB_CONFIG_T attrib_config[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   struct tex_rect tex_rects[GL11_CONFIG_MAX_TEXTURE_UNITS];
   struct screen_rect s_rect;

   assert(IS_GL_11(state));

   vertex_pointers_init(&vertex_pointers);
   memset(vb_config, 0, sizeof(vb_config));
   memset(attrib_config, 0, sizeof attrib_config);
   memset(tex_rects, 0, sizeof(struct tex_rect) * GL11_CONFIG_MAX_TEXTURE_UNITS);

   rs = glxx_install_framebuffer_renderstate(state);
   if (rs == NULL)
   {
      goto end;
   }
   khrn_render_state_disallow_flush((KHRN_RENDER_STATE_T*)rs);

   GLXX_DRAW_RAW_T draw_raw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = GL_TRIANGLE_FAN,
      .count = 4,
      .is_draw_arrays = true};
   GLXX_DRAW_T draw;
   draw_params_from_raw(&draw, &draw_raw);

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

   if (!glxx_calculate_and_hide(state, rs, draw.mode))
      goto end;

   attribs_max.index = GLXX_CONFIG_MAX_ELEMENT_INDEX;
   attribs_max.instance = INT_MAX;

   for (i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if (state->gl11.shaderkey.texture[i] & GL11_TEX_ENABLE)
      {
         calculate_tex_rect(state, i, tex_rects + i);
      }
   }

   s_rect.x = 2.0f * x_s/state->viewport.width - 1.0f;
   s_rect.y = 2.0f * y_s/state->viewport.height - 1.0f;
   s_rect.dw = 2.0f * w_s/state->viewport.width;
   s_rect.dh = 2.0f * h_s/state->viewport.height;

   ok = create_and_record_drawtex_attribs(&rs->fmem, &s_rect, z_w, tex_rects,
         &vertex_pointers, vb_config, attrib_config);
   if (!ok)
      goto end;

   ok = glxx_hw_draw_triangles(state, rs, &draw, attrib_config, &attribs_max,
        vb_config, NULL, &vertex_pointers);
   if (!ok)
      goto end;

   /* The shaderkey will be set up wrong, if nothing else */
   khrn_render_state_set_add(&state->dirty.stuff, rs);

end:
   if (rs) khrn_render_state_allow_flush((KHRN_RENDER_STATE_T*)rs);
   vertex_pointers_destroy(&vertex_pointers);
   return ok;
}
