/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.
=============================================================================*/

#include "gl_public_api.h"
#include "../common/khrn_int_common.h"
#include "glxx_shared.h"

#include "glxx_server.h"
#include "glxx_server_internal.h"
#include "libs/util/dglenum/dglenum.h"

#include "glxx_texture.h"
#include "glxx_buffer.h"
#include "../common/khrn_int_util.h"
#include "../common/khrn_interlock.h"

#include "glxx_hw.h"
#include "glxx_renderbuffer.h"
#include "glxx_framebuffer.h"
#include "glxx_translate.h"
#include "libs/core/v3d/v3d_align.h"

#include <string.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "vcos.h"

#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"
#include "libs/util/profile/profile.h"

static bool is_usage(GLXX_SERVER_STATE_T *state, GLenum usage)
{
   switch (usage)
   {
   case GL_STATIC_DRAW:
   case GL_DYNAMIC_DRAW:
      return true;
   case GL_STREAM_DRAW:
   case GL_STREAM_READ:
   case GL_STREAM_COPY:
   case GL_STATIC_READ:
   case GL_STATIC_COPY:
   case GL_DYNAMIC_READ:
   case GL_DYNAMIC_COPY:
      return !IS_GL_11(state);
   default:
      return false;
   }
}
static bool is_access(GLbitfield access)
{
   GLbitfield valid_access =
      GL_MAP_READ_BIT              |
      GL_MAP_WRITE_BIT             |
      GL_MAP_INVALIDATE_RANGE_BIT  |
      GL_MAP_INVALIDATE_BUFFER_BIT |
      GL_MAP_FLUSH_EXPLICIT_BIT    |
      GL_MAP_UNSYNCHRONIZED_BIT;

   return (access & ~valid_access) == 0;
}

static bool gl11_is_buffer_target(GLenum target)
{
   switch (target)
   {
   case GL_ARRAY_BUFFER:
   case GL_ELEMENT_ARRAY_BUFFER:
      return true;
   default:
      return false;
   }
}

static bool gl3_is_buffer_target(GLenum target)
{
   switch (target)
   {
   case GL_ARRAY_BUFFER:
   case GL_ELEMENT_ARRAY_BUFFER:
   case GL_COPY_READ_BUFFER:
   case GL_COPY_WRITE_BUFFER:
   case GL_PIXEL_PACK_BUFFER:
   case GL_PIXEL_UNPACK_BUFFER:
   case GL_TRANSFORM_FEEDBACK_BUFFER:
   case GL_UNIFORM_BUFFER:
      return true;
   case GL_DRAW_INDIRECT_BUFFER:
   case GL_DISPATCH_INDIRECT_BUFFER:
   case GL_ATOMIC_COUNTER_BUFFER:
   case GL_SHADER_STORAGE_BUFFER:
   case GL_TEXTURE_BUFFER:
      return KHRN_GLES31_DRIVER ? true : false;
   default:
      return false;
   }
}

static bool is_indexed_buffer_target(GLenum target) {
   switch(target)
   {
   case GL_TRANSFORM_FEEDBACK_BUFFER:
   case GL_UNIFORM_BUFFER:
      return true;
   case GL_SHADER_STORAGE_BUFFER:
   case GL_ATOMIC_COUNTER_BUFFER:
      return KHRN_GLES31_DRIVER ? true : false;
   default:
      return false;
   }
}

static GLXX_BUFFER_BINDING_T *binding_from_target(GLXX_SERVER_STATE_T *state,
                                                  GLenum target)
{
   switch (target)
   {
   case GL_ARRAY_BUFFER:
      return &state->bound_buffer[GLXX_BUFTGT_ARRAY];
   case GL_ELEMENT_ARRAY_BUFFER:
   {
      GLXX_VAO_T *vao = state->vao.bound;
      return &vao->element_array_binding;
   }
   case GL_COPY_READ_BUFFER:
      return &state->bound_buffer[GLXX_BUFTGT_COPY_READ];
   case GL_COPY_WRITE_BUFFER:
      return &state->bound_buffer[GLXX_BUFTGT_COPY_WRITE];
   case GL_PIXEL_PACK_BUFFER:
      return &state->bound_buffer[GLXX_BUFTGT_PIXEL_PACK];
   case GL_PIXEL_UNPACK_BUFFER:
      return &state->bound_buffer[GLXX_BUFTGT_PIXEL_UNPACK];
   case GL_DRAW_INDIRECT_BUFFER:
      return &state->bound_buffer[GLXX_BUFTGT_DRAW_INDIRECT];
   case GL_DISPATCH_INDIRECT_BUFFER:
      return &state->bound_buffer[GLXX_BUFTGT_DISPATCH_INDIRECT];
   case GL_UNIFORM_BUFFER:
      return &state->bound_buffer[GLXX_BUFTGT_UNIFORM_BUFFER];
   case GL_ATOMIC_COUNTER_BUFFER:
      return &state->bound_buffer[GLXX_BUFTGT_ATOMIC_COUNTER_BUFFER];
   case GL_SHADER_STORAGE_BUFFER:
      return &state->bound_buffer[GLXX_BUFTGT_SHADER_STORAGE_BUFFER];
   case GL_TEXTURE_BUFFER:
      return &state->bound_buffer[GLXX_BUFTGT_TEXTURE_BUFFER];
   case GL_TRANSFORM_FEEDBACK_BUFFER:
      return &(glxx_get_bound_tf(state)->bound_buffer);
   default:
      unreachable();
   }
}

static GLXX_INDEXED_BINDING_POINT_T *indexed_binding_from_target(GLXX_SERVER_STATE_T *state,
                                                                 GLenum target)
{
   switch (target) {
      case GL_UNIFORM_BUFFER:            return state->uniform_block.binding_points;
      case GL_TRANSFORM_FEEDBACK_BUFFER: return state->transform_feedback.bound->binding_points;
      case GL_SHADER_STORAGE_BUFFER:     return state->ssbo.binding_points;
      case GL_ATOMIC_COUNTER_BUFFER:     return state->atomic_counter.binding_points;
      default:       unreachable(); return NULL;
   }
}

/* Returns GL_NO_ERROR if we can find a bound buffer for that target and the
 * bounding buffer is valid. Fills in dst with the the bounding buffer.
 * Returns a GL error otherwise;
 */
static GLenum get_buffer_binding(GLXX_SERVER_STATE_T *state, GLenum target,
      GLXX_BUFFER_BINDING_T *dst)
{
   GLXX_BUFFER_BINDING_T *b;

   if (IS_GL_11(state) && !gl11_is_buffer_target(target))
      return GL_INVALID_ENUM;

   if (!IS_GL_11(state) && !gl3_is_buffer_target(target))
      return GL_INVALID_ENUM;

   b = binding_from_target(state, target);

   dst->buffer = b->buffer;
   dst->obj    = b->obj;

   /* client attempts to modify or query buffer object state for a target
    * bound to zero generate an INVALID_OPERATION error */
   if (dst->buffer == 0 || dst->obj == NULL)
      return GL_INVALID_OPERATION;

   return GL_NO_ERROR;
}

static void bind_buffer(GLXX_SERVER_STATE_T *state, GLenum target, GLXX_BUFFER_T *buffer)
{
   GLXX_BUFFER_BINDING_T *buffer_binding = binding_from_target(state, target);

   buffer_binding->buffer = buffer ? buffer->name : 0;
   KHRN_MEM_ASSIGN(buffer_binding->obj, buffer);
}

// If size == SIZE_MAX, use full buffer size, dynamically evaluated each time binding is used
static void bind_indexed_buffer(GLXX_SERVER_STATE_T *state, GLenum target, GLuint index,
   GLXX_BUFFER_T *buffer, size_t offset, size_t size)
{
   GLXX_INDEXED_BINDING_POINT_T *binding_points;

   assert(is_indexed_buffer_target(target));

   binding_points = indexed_binding_from_target(state, target);

   binding_points[index].buffer.buffer = buffer ? buffer->name : 0;
   binding_points[index].offset        = offset;
   binding_points[index].size          = size;
   KHRN_MEM_ASSIGN(binding_points[index].buffer.obj, buffer);
}

// If offset == -1, do not bind indexed binding point
static GLenum set_bound_buffer(GLXX_SERVER_STATE_T *state, GLenum target,
                               GLuint buffer, GLuint index,
                               GLintptr offset, GLsizeiptr size)
{
   GLXX_BUFFER_T *buffer_obj;

   if (target == GL_TRANSFORM_FEEDBACK_BUFFER)
   {
      GLXX_TRANSFORM_FEEDBACK_T *tf = glxx_get_bound_tf(state);
      if (glxx_tf_is_active(tf))
         return GL_INVALID_OPERATION;
   }

   if (buffer)
   {
      buffer_obj = glxx_shared_get_buffer(state->shared, buffer);

      // Buffer not found, create one from the provided name
      if (buffer_obj == NULL)
      {
         bool out_of_memory;
         buffer_obj = glxx_shared_allocate_buffer(state->shared, buffer, &out_of_memory);

         if (out_of_memory)
            return GL_OUT_OF_MEMORY;

         // We know that the name didn't exist before the allocate, so if
         // there was no buffer returned and no out of memory error set by the
         // allocation then something is badly wrong with the buffer map.
         assert(buffer_obj != NULL);
      }

      if (!buffer_obj->enabled)
         glxx_buffer_enable(buffer_obj);
   }
   else
      buffer_obj = NULL;

   bind_buffer(state, target, buffer_obj);

   if (offset != -1)
      bind_indexed_buffer(state, target, index, buffer_obj, offset, size);

   return GL_NO_ERROR;
}

GL_API GLboolean GL_APIENTRY glIsBuffer(GLuint buffer)
{
   GLXX_SERVER_STATE_T  *state = glxx_lock_server_state(OPENGL_ES_ANY);
   GLboolean            result;
   GLXX_BUFFER_T        *buffer_obj;

   if (!state) return GL_FALSE;

   buffer_obj = glxx_shared_get_buffer(state->shared, buffer);
   result = (buffer_obj != NULL && buffer_obj->enabled);

   glxx_unlock_server_state();

   return result;
}

GL_API void GL_APIENTRY glBindBuffer(GLenum target, GLuint buffer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   GLenum error;

   if (!state) return;

   if (IS_GL_11(state) && !gl11_is_buffer_target(target)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto unlock_out;
   }

   if (!IS_GL_11(state) && !gl3_is_buffer_target(target)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto unlock_out;
   }

   error = set_bound_buffer(state, target, buffer, 0, -1, 0);
   if (error != GL_NO_ERROR)
   {
      glxx_server_state_set_error(state, error);
      goto unlock_out;
   }

unlock_out:
   glxx_unlock_server_state();
}

#if KHRN_GLES31_DRIVER

GL_API void GL_APIENTRY glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);

   if(!state) return;

   if(bindingindex >= GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS
      || stride < 0 || offset < 0 || stride > GLXX_CONFIG_MAX_VERTEX_ATTRIB_STRIDE)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if(state->vao.bound == state->vao.default_vao)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   GLXX_BUFFER_T *buffer_obj = NULL;
   if(buffer > 0)
   {
      buffer_obj = glxx_shared_get_buffer(state->shared, buffer);

      // NOTE: This ignores the case where the buffer object already exists
      //       but was created via BindBuffer using a non-GenBuffers generated
      //       name. Khronos Bug#15252 has been raised to clarify the
      //       BindVertexBuffer error returns for this situation.
      if(buffer_obj == NULL)
      {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         goto end;
      }

      if(!buffer_obj->enabled)
         glxx_buffer_enable(buffer_obj);
   }

   GLXX_VBO_BINDING_T *vbo = &state->vao.bound->vbos[bindingindex];
   vbo->stride = stride;
   vbo->offset = offset;
   KHRN_MEM_ASSIGN(vbo->buffer, buffer_obj);

end:
   glxx_unlock_server_state();
}

#endif

static const struct buf_restrictions {
   unsigned int num_bindings;
   int offset_alignment;
   int size_alignment;
} gl_buf_restrictions[] = {
   { GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS, GLXX_CONFIG_UNIFORM_BUFFER_OFFSET_ALIGNMENT, 1 },
   { V3D_MAX_TF_BUFFERS, 4, 4 },
   { GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS, GLXX_CONFIG_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, 1 },
   { GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS, 4, 1}
};

static const struct buf_restrictions *get_buf_restrictions(GLenum target) {
   switch (target) {
      case GL_UNIFORM_BUFFER:            return &gl_buf_restrictions[0];
      case GL_TRANSFORM_FEEDBACK_BUFFER: return &gl_buf_restrictions[1];
      case GL_SHADER_STORAGE_BUFFER:     return &gl_buf_restrictions[2];
      case GL_ATOMIC_COUNTER_BUFFER:     return &gl_buf_restrictions[3];
      default: unreachable();       return NULL;
   }
}

GL_API void GL_APIENTRY glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   GLenum error;
   const struct buf_restrictions *r;

   if (!state) return;

   if (!is_indexed_buffer_target(target)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto unlock_out;
   }

   r = get_buf_restrictions(target);
   if (index >= r->num_bindings) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto unlock_out;
   }

   error = set_bound_buffer(state, target, buffer, index, 0, SIZE_MAX);
   if (error != GL_NO_ERROR)
   {
      glxx_server_state_set_error(state, error);
      goto unlock_out;
   }

unlock_out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
   GLXX_SERVER_STATE_T *state;
   GLenum error;
   const struct buf_restrictions *r;

   state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if (!is_indexed_buffer_target(target)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto unlock_out;
   }

   r = get_buf_restrictions(target);
   if (index >= r->num_bindings) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto unlock_out;
   }

   /* When buffer == 0, we would not perform error checks for size and offset;
    * The conformance test uses size == 0, so allow that. See Khronos bugzilla
    * 9765
    */
   if (buffer != 0)
   {
      /* TODO: Is it allowed to bind UBOs that are bigger than the max size? */
      if (size <= 0) {
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
         goto unlock_out;
      }

      if ((offset % r->offset_alignment) != 0) {
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
         goto unlock_out;
      }

      if ((size % r->size_alignment) != 0) {
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
         goto unlock_out;
      }
   }

   // This will call glxx_tf_set_bound_buffer() for example which will
   // check if we have transform feedback active
   error = set_bound_buffer(state, target, buffer, index, offset, size);
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

unlock_out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T     *state = glxx_lock_server_state(OPENGL_ES_ANY);
   GLXX_BUFFER_BINDING_T   buffer_binding;
   GLXX_BUFFER_T           *buffer;
   GLenum error;

   if (!state) return;

   if (size < 0)
   {
      // OpenGL GL 4.3:  "An INVALID_VALUE error is generated if size is negative."
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto unlock_out;
   }

   if(!is_usage(state, usage))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto unlock_out;
   }

   error  = get_buffer_binding(state, target, &buffer_binding);
   if (error != GL_NO_ERROR)
   {
      glxx_server_state_set_error(state, error);
      goto unlock_out;
   }

   buffer = buffer_binding.obj;
   if (buffer->mapped_pointer != 0)
   {
      //  If any portion of the buffer object is mapped in the
      //  current context or any context current to another thread,
      //  it is as though UnmapBuffer (see section 2.9.3) is executed
      //  in each such context prior to deleting the existing data store.
      if (!glxx_unmap_buffer(state, target))
         goto unlock_out;
   }

   if (!glxx_buffer_data(buffer, (size_t)size, data, usage))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

unlock_out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLXX_BUFFER_BINDING_T buffer_binding;
   GLenum error = get_buffer_binding(state, target, &buffer_binding);
   if (error != GL_NO_ERROR)
   {
      glxx_server_state_set_error(state, error);
      goto unlock_out;
   }

   GLXX_BUFFER_T *buffer = buffer_binding.obj;

   int32_t buffer_size = glxx_buffer_get_size(buffer);
   if ((offset < 0) || (size < 0) || (offset + size > buffer_size))
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto unlock_out;
   }

   if (buffer->mapped_pointer != NULL)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto unlock_out;
   }

   if (data && size > 0)
   {
      if (!glxx_buffer_subdata(buffer, (size_t)offset, (size_t)size, data))
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
   }

unlock_out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
   GLXX_SERVER_STATE_T  *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state) return;

   if (n < 0)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (buffers)
   {
      //  We believe it is not necessary to call khrn_fifo_finish() here because
      //  the buffer contents are not being modified.
      //  If the client requires more predictable memory usage, glFinish() should
      //  be called after deleting objects and before allocating new ones.

      for (int i = 0; i < n; i++)
      {
         if (buffers[i])
         {
            GLXX_BUFFER_T *buffer = glxx_shared_get_buffer(state->shared, buffers[i]);

            if (buffer != NULL) {
               for (int j = 0; j < GLXX_BUFTGT_CTX_COUNT; ++j)
               {
                  if (state->bound_buffer[j].obj == buffer) {
                     KHRN_MEM_ASSIGN(state->bound_buffer[j].obj, NULL);
                     state->bound_buffer[j].buffer = 0;
                  }
               }

               GLXX_VAO_T *vao = state->vao.bound;
               if (vao->element_array_binding.obj == buffer) {
                  KHRN_MEM_ASSIGN(vao->element_array_binding.obj, NULL);
                  vao->element_array_binding.buffer = 0;
               }

               // Disconnect from transform feedback
               glxx_tf_delete_buffer(state->transform_feedback.bound, buffer, buffers[i]);

               // Disconnect from uniform buffers
               for (int j = 0; j < GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS; ++j)
               {
                  if (state->uniform_block.binding_points[j].buffer.obj == buffer) {
                     KHRN_MEM_ASSIGN(state->uniform_block.binding_points[j].buffer.obj, NULL);
                     state->uniform_block.binding_points[j].buffer.buffer = 0;
                  }
               }

               for (int j = 0; j < GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS; j++)
               {
                  if (vao->vbos[j].buffer == buffer)
                     KHRN_MEM_ASSIGN(vao->vbos[j].buffer, NULL);
               }

               for (int j=0; j<GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS; j++) {
                  if (state->atomic_counter.binding_points[j].buffer.obj == buffer) {
                     KHRN_MEM_ASSIGN(state->atomic_counter.binding_points[j].buffer.obj, NULL);
                     state->atomic_counter.binding_points[j].buffer.buffer = 0;
                  }
               }

               for (int j=0; j<GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS; j++) {
                  if (state->ssbo.binding_points[j].buffer.obj == buffer) {
                     KHRN_MEM_ASSIGN(state->ssbo.binding_points[j].buffer.obj, NULL);
                     state->ssbo.binding_points[j].buffer.buffer = 0;
                  }
               }

               glxx_shared_delete_buffer(state->shared, buffers[i]);
            }
         }
      }
   }

   glxx_unlock_server_state();
}

void glxx_get_buffer_pointerv(GLenum target, GLenum pname, GLvoid ** params)
{
   GLXX_SERVER_STATE_T     *state = glxx_lock_server_state(OPENGL_ES_ANY);
   GLXX_BUFFER_BINDING_T   buffer_binding;
   GLXX_BUFFER_T           *buffer;
   GLenum error;

   if (!state) return;

   if (pname != GL_BUFFER_MAP_POINTER || pname != GL_BUFFER_MAP_POINTER_OES)
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto unlock_out;
   }

   error = get_buffer_binding(state, target, &buffer_binding);
   if (error != GL_NO_ERROR)
   {
      glxx_server_state_set_error(state, error);
      goto unlock_out;
   }

   buffer = buffer_binding.obj;
   params[0] = buffer->mapped_pointer;

unlock_out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params)
{
   // TODO: This is currently identical to glGetBufferParameteriv
   GLXX_SERVER_STATE_T     *state = glxx_lock_server_state(OPENGL_ES_ANY);
   GLXX_BUFFER_BINDING_T   buffer_binding;
   GLXX_BUFFER_T           *buffer;
   GLenum error;

   if (!state) return;

   error = get_buffer_binding(state, target, &buffer_binding);
   if (error != GL_NO_ERROR)
   {
      glxx_server_state_set_error(state, error);
      goto unlock_out;
   }

   buffer = buffer_binding.obj;

   switch (pname)
   {
   case GL_BUFFER_SIZE:
      params[0] = glxx_buffer_get_size(buffer);
      break;
   case GL_BUFFER_USAGE:
      params[0] = buffer->usage;
      break;
   case GL_BUFFER_ACCESS_OES:
      if ((buffer->mapped_access_flags & ~GL_MAP_WRITE_BIT) != 0)
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      else
         params[0] = GL_WRITE_ONLY;
      break;
   case GL_BUFFER_MAPPED: // == GL_BUFFER_MAPPED_OES
      params[0] = (buffer->mapped_pointer != 0);
      break;
   case GL_BUFFER_MAP_OFFSET:
      params[0] = buffer->mapped_offset;
      break;
   case GL_BUFFER_MAP_LENGTH:
      params[0] = buffer->mapped_size;
      break;
   case GL_BUFFER_ACCESS_FLAGS:
      params[0] = buffer->mapped_access_flags;
      break;
   case GL_BUFFER_MAP_POINTER:
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

unlock_out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetBufferPointerv(GLenum target, GLenum pname, GLvoid** params)
{
   GLXX_SERVER_STATE_T     *state = glxx_lock_server_state(OPENGL_ES_ANY);
   GLXX_BUFFER_BINDING_T   buffer_binding;
   GLXX_BUFFER_T           *buffer;
   GLenum error;

   if (!state) return;

   error = get_buffer_binding(state, target, &buffer_binding);
   if (error != GL_NO_ERROR)
   {
      glxx_server_state_set_error(state, error);
      goto unlock_out;
   }

   buffer = buffer_binding.obj;

   switch (pname)
   {
   case GL_BUFFER_MAP_POINTER:
      params[0] = buffer->mapped_pointer;
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

unlock_out:
   glxx_unlock_server_state();
}

GL_API GLvoid* GL_APIENTRY glMapBufferRange(
   GLenum      target,
   GLintptr    offset,
   GLsizeiptr  length,
   GLbitfield  access)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return NULL;

   void *p = NULL;

   if ((offset < 0) || (length < 0))
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   p = glxx_map_buffer_range(state, target, offset, length, access);

end:
   glxx_unlock_server_state();
   return p;
}

/* if size == SIZE_MAX, the buffer is mapped starting from offset till the end
 * of the buffer */
void* glxx_map_buffer_range(GLXX_SERVER_STATE_T *state, GLenum target,
      size_t offset, size_t size, GLbitfield access)
{
   bool read = (access & GL_MAP_READ_BIT) == GL_MAP_READ_BIT;
   bool write = (access & GL_MAP_WRITE_BIT) == GL_MAP_WRITE_BIT;
   bool inv_buffer = (access & GL_MAP_INVALIDATE_BUFFER_BIT) == GL_MAP_INVALIDATE_BUFFER_BIT;
   bool inv_range = (access & GL_MAP_INVALIDATE_RANGE_BIT) == GL_MAP_INVALIDATE_RANGE_BIT;
   bool unsync = (access & GL_MAP_UNSYNCHRONIZED_BIT) == GL_MAP_UNSYNCHRONIZED_BIT;
   bool flush_exp = (access & GL_MAP_FLUSH_EXPLICIT_BIT) == GL_MAP_FLUSH_EXPLICIT_BIT;

   GLXX_BUFFER_BINDING_T buffer_binding;
   GLenum error = get_buffer_binding(state, target, &buffer_binding);
   if (error != GL_NO_ERROR)
   {
      glxx_server_state_set_error(state, error);
      return NULL;
   }
   assert(buffer_binding.obj != NULL);

   if (!is_access(access))
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      return NULL;
   }

   if ((size == 0)                                   ||
      (!read && !write)                              ||
      (read && (inv_buffer || inv_range || unsync))  ||
      (flush_exp && !write))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return NULL;
   }

   GLXX_BUFFER_T *buffer = buffer_binding.obj;
   if (buffer->mapped_pointer != NULL)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION); /* already mapped */
      return NULL;
   }

   size_t buffer_size = glxx_buffer_get_size(buffer);
   if (size == SIZE_MAX)
   {
      if (offset > buffer_size)
      {
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
         return NULL;
      }
      size = buffer_size - offset;
   }
   else if (offset + size > buffer_size)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      return NULL;
   }

   buffer->mapped_pointer = glxx_buffer_map_range(buffer, offset, size, access);
   if (buffer->mapped_pointer == NULL)
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      return NULL;
   }

   buffer->mapped_size         = size;
   buffer->mapped_offset       = offset;
   buffer->mapped_access_flags = access;

   return buffer->mapped_pointer;
}

GL_API GLboolean GL_APIENTRY glUnmapBuffer(GLenum target)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return GL_FALSE;

   GLboolean res = glxx_unmap_buffer(state, target);

   glxx_unlock_server_state();
   return res;
}

GLboolean glxx_unmap_buffer(GLXX_SERVER_STATE_T *state, GLenum target)
{
   GLXX_BUFFER_BINDING_T   buffer_binding;
   GLXX_BUFFER_T           *buffer;
   GLenum   error;

   // Value of access needs to be validated by the caller
   error = get_buffer_binding(state, target, &buffer_binding);
   if (error != GL_NO_ERROR)
   {
      glxx_server_state_set_error(state, error);
      return false;
   }

   buffer = buffer_binding.obj;


   if (buffer->mapped_pointer == NULL)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION); /* not mapped */
      return false;
   }

   glxx_buffer_unmap_range(
      buffer,
      buffer->mapped_offset,
      buffer->mapped_size,
      buffer->mapped_access_flags);

   buffer->mapped_pointer      = NULL;
   buffer->mapped_offset       = 0;
   buffer->mapped_size         = 0;
   buffer->mapped_access_flags = 0;

   return true;
}

GL_API void GL_APIENTRY glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if ((offset < 0) || (length < 0))
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto unlock_out;
   }

   GLXX_BUFFER_BINDING_T buffer_binding;
   GLenum error = get_buffer_binding(state, target, &buffer_binding);
   if (error != GL_NO_ERROR)
   {
      glxx_server_state_set_error(state, error);
      goto unlock_out;
   }

   GLXX_BUFFER_T *buffer = buffer_binding.obj;

   bool flush_exp = (buffer->mapped_access_flags & GL_MAP_FLUSH_EXPLICIT_BIT) == GL_MAP_FLUSH_EXPLICIT_BIT;
   if (!flush_exp || (buffer->mapped_pointer == NULL))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto unlock_out;
   }

   if (offset + length > buffer->mapped_size)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto unlock_out;
   }

unlock_out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glCopyBufferSubData(GLenum readTarget, GLenum writeTarget,
   GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T     *state = glxx_lock_server_state(OPENGL_ES_3X);
   GLXX_BUFFER_BINDING_T   read_buffer_binding;
   GLXX_BUFFER_BINDING_T   write_buffer_binding;
   GLXX_BUFFER_T           *read_buffer;
   GLXX_BUFFER_T           *write_buffer;
   GLenum error;
   int32_t                 read_buffer_size;
   int32_t                 write_buffer_size;

   if (!state) return;

   error = get_buffer_binding(state, readTarget,  &read_buffer_binding);
   if (error == GL_NO_ERROR)
      error = get_buffer_binding(state, writeTarget, &write_buffer_binding);
   if (error != GL_NO_ERROR)
   {
      glxx_server_state_set_error(state, error);
      goto unlock_out;
   }

   read_buffer = read_buffer_binding.obj;
   write_buffer = write_buffer_binding.obj;

   read_buffer_size = glxx_buffer_get_size(read_buffer);
   write_buffer_size = glxx_buffer_get_size(write_buffer);
   if ( size < 0 || readOffset < 0 || writeOffset < 0 ||
        (readOffset + size > read_buffer_size) ||
        (writeOffset + size > write_buffer_size)  )
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto unlock_out;
   }

   // Test for overlapping ranges when the read and write buffers are the same.
   if((read_buffer == write_buffer) &&
      (readOffset + size > writeOffset) &&
      (writeOffset + size > readOffset))
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto unlock_out;
   }

   if ((read_buffer->mapped_pointer != NULL) || (write_buffer->mapped_pointer != NULL))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto unlock_out;
   }

   if (size > 0)
   {
      if (!glxx_buffer_copy_subdata(read_buffer, write_buffer, (size_t)readOffset, (size_t)writeOffset, (size_t)size))
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
   }

unlock_out:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
   // TODO: This is currently identical to glGetBufferParameteri64v
   GLXX_SERVER_STATE_T     *state = glxx_lock_server_state(OPENGL_ES_ANY);
   GLXX_BUFFER_BINDING_T   buffer_binding;
   GLXX_BUFFER_T           *buffer;
   GLenum error;

   if (!state) return;

   error = get_buffer_binding(state, target, &buffer_binding);
   if (error != GL_NO_ERROR)
   {
      glxx_server_state_set_error(state, error);
      goto unlock_out;
   }

   buffer = buffer_binding.obj;

   switch (pname)
   {
   case GL_BUFFER_SIZE:
      params[0] = glxx_buffer_get_size(buffer);
      break;
   case GL_BUFFER_USAGE:
      params[0] = buffer->usage;
      break;
   case GL_BUFFER_ACCESS_OES:
      if ((buffer->mapped_access_flags & ~GL_MAP_WRITE_BIT)!=0)
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      else
         params[0] = GL_WRITE_ONLY;
      break;
   case GL_BUFFER_MAPPED: // == GL_BUFFER_MAPPED_OES
      params[0] = (buffer->mapped_pointer != 0);
      break;
   case GL_BUFFER_MAP_OFFSET:
      params[0] = buffer->mapped_offset;
      break;
   case GL_BUFFER_MAP_LENGTH:
      params[0] = buffer->mapped_size;
      break;
   case GL_BUFFER_ACCESS_FLAGS:
      params[0] = buffer->mapped_access_flags;
      break;
   case GL_BUFFER_MAP_POINTER:
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

unlock_out:
   glxx_unlock_server_state();
}
