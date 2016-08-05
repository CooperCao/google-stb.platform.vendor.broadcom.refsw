/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  glxx_server_transform_feedback.c

FILE DESCRIPTION
OpenGL ES 3.0 transform feedback object
=============================================================================*/

#include "gl_public_api.h"
#include "../common/khrn_int_common.h"
#include "glxx_shared.h"

#include "glxx_server.h"
#include "glxx_server_internal.h"
#include "libs/util/dglenum/dglenum.h"

#include "glxx_buffer.h"
#include "../common/khrn_int_util.h"

#include "glxx_hw.h"
#include "glxx_translate.h"

#include "../gl20/gl20_program.h"
#include "../gl20/gl20_shader.h"

#include "../common/khrn_render_state.h"

#include <string.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "vcos.h"

#include "libs/platform/gmem.h"

LOG_DEFAULT_CAT("glxx_server_transform_feedback")

#define GLXX_TRANSFORM_FEEDBACK_FLAG_CREATED           (1 << 0)   // set on first call to BeginQuery
#define GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE            (1 << 1)   // set on calls to BeginTransformFeedback, cleared on EndTransformFeedback
#define GLXX_TRANSFORM_FEEDBACK_FLAG_PAUSED            (1 << 2)   // set on calls to PauseTransformFeedback, cleared on ResumeTransformFeedback
#define GLXX_TRANSFORM_FEEDBACK_FLAG_ENDED             (1 << 3)   // set on EndTransformFeedback
#define GLXX_TRANSFORM_FEEDBACK_FLAG_WAITED            (1 << 4)   // set when we read from transform feedback buffer
#define GLXX_TRANSFORM_FEEDBACK_FLAG_RESULT_AVAILABLE  (1 << 4)   // set when result available

static_assrt(V3D_MAX_TF_BUFFERS >= GLXX_CONFIG_MAX_TF_SEPARATE_ATTRIBS);

GLXX_BUFFER_BINDING_T *glxx_tf_get_buffer_binding(GLXX_SERVER_STATE_T *state)
{
   GLXX_TRANSFORM_FEEDBACK_T *tf = state->transform_feedback.binding;
   return &tf->generic_buffer_binding;
}

GLint glxx_tf_get_bound(GLXX_SERVER_STATE_T const* state)
{
   return (GLint) (state->transform_feedback.binding->name);
}

GLuint glxx_tf_get_bound_buffer(GLXX_SERVER_STATE_T const* state)
{
   return (GLuint) (state->transform_feedback.binding->generic_buffer_binding.buffer);
}

bool glxx_tf_bind_buffer_valid(GLXX_SERVER_STATE_T *state) {
   return !state->transform_feedback.in_use;
}

GLXX_INDEXED_BINDING_POINT_T *glxx_tf_get_indexed_bindings(GLXX_SERVER_STATE_T *state) {
   return state->transform_feedback.binding->binding_points;
}

#if !GLXX_HAS_TNG
static bool buffer_has_space(const GLXX_TRANSFORM_FEEDBACK_T *tf, int index, uint32_t additional_bytes)
{
   const GLXX_INDEXED_BINDING_POINT_T *binding = &tf->binding_points[index];

   if (binding->buffer.obj == NULL)
      return false;

   uint32_t write_begin = binding->offset + tf->stream_position[index];
   uint32_t write_end   = write_begin + additional_bytes;
   size_t binding_end = (binding->size == -1)
      ? glxx_buffer_get_size(binding->buffer.obj)   // if binding size == -1, use full buffer size
      : (size_t)(binding->offset + binding->size);

   return (write_end <= binding_end);
}
#endif

static bool prim_mode_valid(GLenum tf_mode, GLenum draw_mode) {
#if GLXX_HAS_TNG
   if (tf_mode == GL_POINTS)    return draw_mode == GL_POINTS;
   if (tf_mode == GL_LINES)     return draw_mode == GL_LINES     || draw_mode == GL_LINE_STRIP     ||
                                       draw_mode == GL_LINE_LOOP;
   if (tf_mode == GL_TRIANGLES) return draw_mode == GL_TRIANGLES || draw_mode == GL_TRIANGLE_STRIP ||
                                       draw_mode == GL_TRIANGLE_FAN;
   unreachable();
   return false;
#else
   return tf_mode == draw_mode;
#endif
}

bool glxx_tf_validate_draw(const GLXX_SERVER_STATE_T *state,
   GLenum primitive_mode, GLsizei count, GLsizei instance_count)
{
   const GLXX_TRANSFORM_FEEDBACK_T *tf = state->transform_feedback.binding;
   bool active = (tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE) == GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE;
   bool paused = (tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_PAUSED) == GLXX_TRANSFORM_FEEDBACK_FLAG_PAUSED;
   bool in_use = active && !paused;

   if (!in_use) return true;

   // When transform feedback is active and not paused, primitive_mode must be
   // compatible with the primitiveMode passed to BeginTransformFeedback.
   if (!prim_mode_valid(tf->primitive_mode, primitive_mode)) return false;

#if !GLXX_HAS_TNG
   // Prior to geometry shading this should fail if the primitives would overflow a buffer

   // Count how many bytes would be written to each buffer by this draw call
   uint32_t tf_vertices;
   if (primitive_mode == GL_POINTS)
      tf_vertices = count;
   else if (primitive_mode == GL_LINES)
      tf_vertices = count - (count & 1);
   else if (primitive_mode == GL_TRIANGLES)
      tf_vertices = count - (count % 3);
   else {
      return false;
   }

   tf_vertices *= instance_count;

   uint32_t bytes_to_write[V3D_MAX_TF_BUFFERS] = { 0, };

   GL20_PROGRAM_COMMON_T        *pc  = gl20_program_common_get(state);
   GLXX_PROGRAM_TFF_POST_LINK_T *ptf = &pc->transform_feedback;

   for (uint32_t i = 0; i < ptf->spec_count; ++i)
   {
      V3D_TF_SPEC_T *spec = &ptf->spec[i];

      bytes_to_write[spec->buffer] += tf_vertices * spec->count * sizeof(uint32_t);
   }

   // Validate we don't exceed buffer
   for (uint32_t i = 0; i < V3D_MAX_TF_BUFFERS; ++i)
   {
      if (bytes_to_write[i])
      {
         bool fits = buffer_has_space(tf, i, bytes_to_write[i]);
         if (!fits)
            return false;
      }
   }
#endif

   return true;
}

void glxx_tf_write_primitives(GLXX_SERVER_STATE_T *state,
   v3d_prim_mode_t primitive_mode, GLsizei count, GLsizei instance_count)
{
   GLXX_TRANSFORM_FEEDBACK_T *tf = state->transform_feedback.binding;
   bool active = (tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE) == GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE;
   bool paused = (tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_PAUSED) == GLXX_TRANSFORM_FEEDBACK_FLAG_PAUSED;
   bool in_use = active && !paused;

   if (!in_use) return;

   // Count how many bytes to write to each buffer by this draw call
   uint32_t primitive_count = 0;
   uint32_t tf_vertices = 0;

#if !V3D_HAS_NEW_TF
   if (primitive_mode == V3D_PRIM_MODE_POINTS_TF)
   {
      tf_vertices = count;
      primitive_count = tf_vertices;
   }
   else if (primitive_mode == V3D_PRIM_MODE_LINES_TF)
   {
      tf_vertices = count - (count & 1);
      primitive_count = tf_vertices / 2;
   }
   else if (primitive_mode == V3D_PRIM_MODE_TRIS_TF)
   {
      tf_vertices = count - (count % 3);
      primitive_count = tf_vertices / 3;
   }
   else
      unreachable();
#else
   /* this code is temporary to get the tf pause in resume passing on versions
    * < es3.1 and no geom extension */
   if (primitive_mode == V3D_PRIM_MODE_POINTS)
   {
      tf_vertices = count;
      primitive_count = tf_vertices;
   }
   else if (primitive_mode == V3D_PRIM_MODE_LINES)
   {
      tf_vertices = count - (count & 1);
      primitive_count = tf_vertices / 2;
   }
   else if (primitive_mode == V3D_PRIM_MODE_TRIS)
   {
      tf_vertices = count - (count % 3);
      primitive_count = tf_vertices / 3;
   }
   else
      unreachable();
#endif

   if (primitive_count == 0)
      return;

   tf_vertices *= instance_count;

   uint32_t bytes_to_write[V3D_MAX_TF_BUFFERS] = { 0, };
   const GL20_PROGRAM_COMMON_T        *pc  = gl20_program_common_get(state);
   const GLXX_PROGRAM_TFF_POST_LINK_T *ptf = &pc->transform_feedback;

   for (unsigned i = 0; i < ptf->spec_count; ++i)
   {
      const V3D_TF_SPEC_T *spec = &ptf->spec[i];
      bytes_to_write[spec->buffer] += tf_vertices * spec->count * sizeof(uint32_t);
   }

   for (unsigned i = 0; i < V3D_MAX_TF_BUFFERS; ++i)
      tf->stream_position[i] += bytes_to_write[i];

   GLXX_QUERY_T *query = state->queries.queries[GLXX_Q_TRANSF_FEEDBACK].active;
   if (query)
      query->result += primitive_count;
}

// Return false if operation is invalid and rs requires flush,
// or we ran out of memory.
bool glxx_tf_add_interlock_writes(
   const GLXX_SERVER_STATE_T *state,
   GLXX_HW_RENDER_STATE_T *rs,
   bool *requires_flush)
{
   GLXX_PROGRAM_TFF_POST_LINK_T *ptf = NULL;
   bool uses_tf = false;

   if (state->transform_feedback.in_use)
   {
      ptf     = &gl20_program_common_get(state)->transform_feedback;
      uses_tf = ptf->varying_count > 0;
   }

   if (!uses_tf)
   {
      *requires_flush = false;
      return true;// No interlock writes needed
   }

   GLXX_TRANSFORM_FEEDBACK_T *tf = state->transform_feedback.binding;
   unsigned num_addr = ptf->addr_count;

   for (unsigned i = 0; i < num_addr; ++i)
   {
      GLXX_BUFFER_T *buffer = tf->binding_points[i].buffer.obj;
      KHRN_RES_INTERLOCK_T *res_i = buffer->resource;

      if (!khrn_fmem_record_res_interlock_self_read_conflicting_write(
            &rs->fmem, res_i, ACTION_BIN, requires_flush))
         return false;
   }

   return true;
}

bool glxx_tf_emit_spec(GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs,
   uint8_t **instr, bool point_size_used)
{
   bool           res = true;
   v3d_addr_t     buffer_addrs[V3D_MAX_TF_BUFFERS];
#if V3D_HAS_NEW_TF
   v3d_addr_t     buffer_sizes[V3D_MAX_TF_BUFFERS];
#endif

   GLXX_TRANSFORM_FEEDBACK_T *tf = state->transform_feedback.binding;
   GL20_PROGRAM_COMMON_T *program_common = gl20_program_common_get(state);

   GLXX_PROGRAM_TFF_POST_LINK_T *ptf = &program_common->transform_feedback;
   uint32_t num_specs = ptf->spec_count;
   uint32_t num_addr  = ptf->addr_count;

   rs->tf_used = true;
   rs->tf_started_count = rs->tf_started_count + 1;

   for (unsigned i = 0; i < num_addr; ++i)
   {
      GLintptr             offset;
      GLXX_BUFFER_T        *buffer;
      KHRN_RES_INTERLOCK_T *res_i;
      v3d_addr_t     addr;

      assert(tf->binding_points[i].buffer.obj != NULL);

      buffer  = tf->binding_points[i].buffer.obj;
      offset  = tf->binding_points[i].offset;
      offset += tf->stream_position[i];

      assert(buffer != NULL);

      // We dont need the TF aware version here.
      res_i = glxx_buffer_get_res_interlock(buffer);

      addr = khrn_fmem_lock_and_sync(&rs->fmem, res_i->handle, GMEM_SYNC_CORE_WRITE, 0);
      buffer_addrs[i] = addr + offset;
#if V3D_HAS_NEW_TF
      buffer_sizes[i] = glxx_buffer_get_size(buffer) - offset;
#endif

      // Record the new last TF wait counter that writes to this buffer.
      buffer->last_tf_write_count = rs->tf_started_count;
   }

#if V3D_HAS_NEW_TF
   for(unsigned i = 0; i < num_addr; i += 1)
   {
      v3d_cl_transform_feedback_buffer(instr, i, buffer_sizes[i] / 4, buffer_addrs[i]);
   }
   v3d_cl_transform_feedback_specs(instr, num_specs, true);
#else
   v3d_cl_transform_feedback_enable(instr, 0, num_addr, num_specs);
#endif

   // Emit transform feedback enable specs
   for (unsigned i = 0; i < num_specs; ++i)
   {
      V3D_TF_SPEC_T *spec_src = &ptf->spec[i];
      V3D_TF_SPEC_T  spec     = *spec_src;
      uint8_t        packed_spec[2];

      // At link time, all tf specs use 7 as first user varying.
      // Here at draw call time, we decrement 1 from all user varyings if point size is not used.
      // Additionally, 6 (before decrementing) is not allowed.
      if (!point_size_used && spec.first >= 6)
      {
         assert(spec.first != 6);
         spec.first -= 1;
      }

      v3d_pack_tf_spec(packed_spec, &spec);

      v3d_cl_add_8(instr, packed_spec[0]);
      v3d_cl_add_8(instr, packed_spec[1]);

      log_trace(
         "PACK: tf spec %d buffer = %d, first = %d, count = %d words. bytes: %02x %02x",
         i,
         spec.buffer,
         spec.first,
         spec.count,
         packed_spec[0],
         packed_spec[1]);
   }

#if !V3D_HAS_NEW_TF
   // Emit transform feedback buffer addresses
   for (unsigned i = 0; i < num_addr; ++i)
   {
      v3d_cl_add_addr(instr, buffer_addrs[i]);
   }
#endif

   return res;
}

bool glxx_tf_is_active(const GLXX_SERVER_STATE_T *state)
{
   GLXX_TRANSFORM_FEEDBACK_T *tf = state->transform_feedback.binding;
   return (tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE) == GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE;
}

bool glxx_tf_is_paused(const GLXX_SERVER_STATE_T *state)
{
   GLXX_TRANSFORM_FEEDBACK_T *tf = state->transform_feedback.binding;
   return (tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_PAUSED) == GLXX_TRANSFORM_FEEDBACK_FLAG_PAUSED;
}

void glxx_tf_delete_buffer(GLXX_TRANSFORM_FEEDBACK_T *tf, GLXX_BUFFER_T *buffer_obj, GLuint buffer)
{
   if (tf->generic_buffer_binding.buffer == buffer) {
      KHRN_MEM_ASSIGN(tf->generic_buffer_binding.obj, NULL);
      tf->generic_buffer_binding.buffer = 0;
   }

   for (int i = 0; i < V3D_MAX_TF_BUFFERS; ++i)
   {
      if (tf->binding_points[i].buffer.buffer == buffer) {
         KHRN_MEM_ASSIGN(tf->binding_points[i].buffer.obj, NULL);
         tf->binding_points[i].buffer.buffer = 0;
      }
   }
}

//  The error INVALID_OPERATION is generated by LinkProgram
//  or ProgramBinary if program is the name of a program
//  being used by one or more transform feedback objects,
//  even if the objects are not currently bound or are paused
typedef struct {
   GL20_PROGRAM_T  *program;
   bool             used;
} TF_PROGRAM_USED_INFO_T;

static void glxx_tf_program_used_callback(KHRN_MAP_T *map, uint32_t key, void *value, void *info_)
{
   TF_PROGRAM_USED_INFO_T *info = info_;
   if (info->used)
      return;

   GLXX_TRANSFORM_FEEDBACK_T *tf = value;
   if (tf->program != NULL && tf->program == info->program)
      info->used = true;
}

// add to header & add to link program
bool glxx_tf_program_used(GLXX_SERVER_STATE_T *state, GL20_PROGRAM_T *program)
{
   TF_PROGRAM_USED_INFO_T info;
   info.program  = program;
   info.used     = false;

   khrn_map_iterate(&state->transform_feedback.objects, glxx_tf_program_used_callback, &info);
   return info.used;
}

static void glxx_transform_feedback_term(void *v, size_t size)
{
   GLXX_TRANSFORM_FEEDBACK_T *tf = (GLXX_TRANSFORM_FEEDBACK_T *)v;

   vcos_unused(size);

   KHRN_MEM_ASSIGN(tf->generic_buffer_binding.obj, NULL);
   for (int i = 0; i < V3D_MAX_TF_BUFFERS; ++i)
   {
      KHRN_MEM_ASSIGN(tf->binding_points[i].buffer.obj, NULL);
   }

   KHRN_MEM_ASSIGN(tf->program, NULL);

   free(tf->debug_label);
   tf->debug_label = NULL;
}

static void glxx_transform_feedback_init(GLXX_TRANSFORM_FEEDBACK_T *tf, int32_t name)
{
   tf->name = name;
   tf->flags = 0;    // created flag is set on first BindTransformFeedback
   tf->primitive_mode = GL_POINTS;
   tf->program = NULL;
   tf->debug_label = NULL;
}

GLXX_TRANSFORM_FEEDBACK_T *glxx_tf_create_default(GLXX_SERVER_STATE_T *state)
{
   GLXX_TRANSFORM_FEEDBACK_T *tf = KHRN_MEM_ALLOC_STRUCT(GLXX_TRANSFORM_FEEDBACK_T);

   if (tf != NULL)
   {
      khrn_mem_set_term(tf, glxx_transform_feedback_term);
      glxx_transform_feedback_init(tf, 0);
   }
   return tf;
}

GLXX_TRANSFORM_FEEDBACK_T *glxx_get_transform_feedback(GLXX_SERVER_STATE_T *state, uint32_t id)
{
   return khrn_map_lookup(&state->transform_feedback.objects, id);
}

static GLXX_TRANSFORM_FEEDBACK_T *new_tf(GLXX_SERVER_STATE_T *state, uint32_t id)
{
   GLXX_TRANSFORM_FEEDBACK_T *tf;

   // return NULL if id was already used
   if (glxx_get_transform_feedback(state, id) != NULL) return NULL;

   tf = KHRN_MEM_ALLOC_STRUCT(GLXX_TRANSFORM_FEEDBACK_T);
   if (tf == NULL) return NULL;

   khrn_mem_set_term(tf, glxx_transform_feedback_term);

   glxx_transform_feedback_init(tf, id);

   if (khrn_map_insert(&state->transform_feedback.objects, id, tf))
      khrn_mem_release(tf);
   else
   {
      khrn_mem_release(tf);
      tf = NULL;
   }

   return tf;
}

static void glxx_delete_transform_feedback(GLXX_SERVER_STATE_T *state, uint32_t id)
{
   assert(id != 0);

   // This makes name unused - map get will now return NULL
   khrn_map_delete(&state->transform_feedback.objects, id);
}

GL_API void GL_APIENTRY glGenTransformFeedbacks(GLsizei n, GLuint *ids)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if (n < 0)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (ids)
   {
      int32_t i = 0;
      while (i < n)
      {
         if (new_tf(state, state->transform_feedback.next) != NULL)
            ids[i++] = state->transform_feedback.next;

         ++state->transform_feedback.next;
      }
   }

   glxx_unlock_server_state();
}

GL_API GLboolean GL_APIENTRY glIsTransformFeedback(GLuint id)
{
   GLboolean result = GL_FALSE;
   GLXX_TRANSFORM_FEEDBACK_T *tf;
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state) return result;

   if (id == 0) goto end; /* TODO: Looks wrong but was here before */

   tf = glxx_get_transform_feedback(state, id);
   if (tf != NULL)
   {
      if ((tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_CREATED) == GLXX_TRANSFORM_FEEDBACK_FLAG_CREATED)
         result = GL_TRUE;
   }

end:
   glxx_unlock_server_state();
   return result;
}

GL_API void GL_APIENTRY glDeleteTransformFeedbacks(GLsizei n, const GLuint *ids)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if (n < 0)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (ids)
   {
      //  The error INVALID_OPERATION is generated by DeleteTransformFeedbacks
      //  if the transform feedback operation for any object named by ids is
      //  currently active.
      for (int i = 0; i < n; i++)
      {
         if (ids[i])
         {
            GLXX_TRANSFORM_FEEDBACK_T *tf = glxx_get_transform_feedback(state, ids[i]);

            if (tf != NULL)
            {
               if ((tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE) != 0)
               {
                  glxx_server_state_set_error(state, GL_INVALID_OPERATION);
                  goto unlock_out;
               }
            }
         }
      }

      for (int i = 0; i < n; i++)
      {
         if (ids[i] == (GLuint)state->transform_feedback.binding->name) {
            KHRN_MEM_ASSIGN(state->transform_feedback.binding, state->transform_feedback.default_obj);
         }

         if (ids[i])
         {
            glxx_delete_transform_feedback(state, ids[i]);
         }
      }
   }

unlock_out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glBindTransformFeedback(GLenum target, GLuint id)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if (target != GL_TRANSFORM_FEEDBACK)
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto unlock_out;
   }

   if (state->transform_feedback.in_use)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto unlock_out;
   }

   GLXX_TRANSFORM_FEEDBACK_T *tf = NULL;
   if (id == 0)
      tf = state->transform_feedback.default_obj;
   else
      // Set create = false, because name 'id' must have
      // been already created by GenTransformFeedbacks
      tf = glxx_get_transform_feedback(state, id);

   if (tf == NULL)
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
   else
   {
      KHRN_MEM_ASSIGN(state->transform_feedback.binding, tf);

      tf->flags |= GLXX_TRANSFORM_FEEDBACK_FLAG_CREATED;
   }

unlock_out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glBeginTransformFeedback(GLenum primitiveMode)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);

   if (!state)
      return;

   if ((primitiveMode != GL_TRIANGLES) && (primitiveMode != GL_LINES) && (primitiveMode != GL_POINTS))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto unlock_out;
   }

   //  The error INVALID_OPERATION is also generated by BeginTransformFeedback
   //  if no binding points would be used, either because no program object is
   //  active or because the active program object has specified no output
   //  variables to record.
   if (state->current_program == NULL && state->pipelines.bound == NULL)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto unlock_out;
   }

   GL20_PROGRAM_COMMON_T *program_common = gl20_program_common_get(state);
   if (!gl20_validate_program(state, program_common))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto unlock_out;
   }

   if (program_common->transform_feedback.varying_count == 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto unlock_out;
   }

   GLXX_TRANSFORM_FEEDBACK_T  *tf = state->transform_feedback.binding;

   if ((tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE) != 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto unlock_out;
   }

   // The error INVALID_OPERATION is generated by BeginTransformFeedback
   // if any binding point used in transform feedback mode does not
   // have a buffer object bound
   for (uint32_t i = 0; i < V3D_MAX_TF_SPECS; ++i)
   {
      uint32_t buffer = program_common->transform_feedback.spec[i].buffer;
      GLXX_BUFFER_T* bufferObj = tf->binding_points[buffer].buffer.obj;

      if (bufferObj == NULL)
      {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         goto unlock_out;
      }
   }

   // Reset transform feedback writing position for all buffers
   for (uint32_t i = 0; i < V3D_MAX_TF_BUFFERS; ++i)
      tf->stream_position[i] = 0;

   tf->flags |= GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE;
   tf->primitive_mode = primitiveMode;

   // Record the currently program or vertex program in pipeline
   KHRN_MEM_ASSIGN(tf->program,  gl20_get_tf_program(state));

   state->transform_feedback.in_use = true;

unlock_out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glPauseTransformFeedback(void)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   GLXX_TRANSFORM_FEEDBACK_T *tf = state->transform_feedback.binding;

   //  The error INVALID_OPERATION is generated by PauseTransformFeedback
   //  if the currently bound transform feedback is not active or is paused.
   if ((tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE) == 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto out;
   }
   if ((tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_PAUSED) != 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto out;
   }

   tf->flags |= GLXX_TRANSFORM_FEEDBACK_FLAG_PAUSED;
   state->transform_feedback.in_use = false;

out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glResumeTransformFeedback(void)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   GLXX_TRANSFORM_FEEDBACK_T *tf = state->transform_feedback.binding;

   //  The error INVALID_OPERATION is generated by ResumeTransformFeedback
   //  if the currently bound transform feedback is not active or is not paused
   if ((tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE) == 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto out;
   }
   if ((tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_PAUSED) == 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto out;
   }

   tf->flags &= ~GLXX_TRANSFORM_FEEDBACK_FLAG_PAUSED;
   state->transform_feedback.in_use = true;

out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glEndTransformFeedback(void)
{
   GLXX_TRANSFORM_FEEDBACK_T *tf;
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   tf = state->transform_feedback.binding;

   if ((tf->flags & GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE) == 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto out;
   }

   tf->flags &= ~GLXX_TRANSFORM_FEEDBACK_FLAG_ACTIVE;
   tf->flags &= ~GLXX_TRANSFORM_FEEDBACK_FLAG_PAUSED;

   tf->flags |= GLXX_TRANSFORM_FEEDBACK_FLAG_ENDED;

   KHRN_MEM_ASSIGN(tf->program, NULL);

   state->transform_feedback.in_use = false;

out:
   glxx_unlock_server_state();
}

//  glGetTransformFeedbackVarying() is in gl20_program.c

GL_API void GL_APIENTRY glTransformFeedbackVaryings(
   GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
   GLXX_SERVER_STATE_T *state;
   GL20_PROGRAM_T *p;
   int i;

   state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if (bufferMode != GL_INTERLEAVED_ATTRIBS && bufferMode != GL_SEPARATE_ATTRIBS)
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto unlock_out;
   }
   if (count < 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto unlock_out;
   }
   if (bufferMode == GL_SEPARATE_ATTRIBS &&
       count > GLXX_CONFIG_MAX_TF_SEPARATE_ATTRIBS)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto unlock_out;
   }

   p = gl20_get_program(state, program);

   if (p == NULL)
      /* error is set by gl20_get_program */
      goto unlock_out;

   p->transform_feedback.buffer_mode = bufferMode;
   p->transform_feedback.varying_count = count;

   for (i = 0; i < GLXX_CONFIG_MAX_TF_INTERLEAVED_COMPONENTS; ++i) {
      // In case we are re-assigning this program's TF varyings.
      free(p->transform_feedback.name[i]);
   }

   for (i = 0; i < count; ++i) {
      p->transform_feedback.name[i] = strdup(varyings[i]);
      if (p->transform_feedback.name[i] == NULL) {
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      }
   }

   for (; i < GLXX_CONFIG_MAX_TF_INTERLEAVED_COMPONENTS; ++i)
      p->transform_feedback.name[i] = NULL;

unlock_out:
   glxx_unlock_server_state();
}
