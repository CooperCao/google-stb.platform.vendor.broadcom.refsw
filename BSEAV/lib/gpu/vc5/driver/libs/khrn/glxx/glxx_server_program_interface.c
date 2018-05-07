/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "libs/util/log/log.h"
#include "../common/khrn_int_common.h"
#include "libs/util/snprintf.h"
#include "glxx_server.h"
#include "../gl20/gl20_program.h"
#include "../gl20/gl20_shader.h"
#include "../glsl/glsl_compiler.h"
#include "glxx_server_program_interface.h"

#include <ctype.h>
#include <math.h> // For log10

/*
   A null-terminating version of strncpy. Copies a string from src
   to dst with a maximum length of len, and forcibly null-terminates
   the result. Returns the number of characters written, not
   including the null terminator, or -1 either dst is NULL or length
   is less than 1 (giving us no space to even write the terminator).
*/
static size_t strzncpy(char *dst, const char *src, size_t len)
{
   // strncpy zero pads to len, but dEQP expects that we don't overrun
   // our src length, so clamp to that too.
   len = gfx_zmin(len, strlen(src) + 1);
   if (dst && len > 0)
   {
      strncpy(dst, src, len);
      dst[len - 1] = '\0';
      return strlen(dst);
   }
   else
      return -1;
}

static size_t strzncpy_with_array(char *dst, const char *src, size_t len, int arr_off)
{
   if (dst == NULL || len <= 0) return -1;

   snprintf(dst, len, "%s[%d]", src, arr_off);
   dst[len-1] = '\0';
   return strlen(dst);
}

static int get_basename_length(const char *name)
{
   int len = strlen(name);

   if (len > 0 && name[len - 1] == ']')
   {
      len--;

      while (len > 0 && isdigit(name[len - 1])) len--;
      if (len > 0)
      {
         if (name[len - 1] != '[')
            return -1;     // Invalid name
         len--;
      }
   }

   return len;
}

static int get_array_offset(const char *name, int32_t pos)
{
   int len = strlen(name);
   int off = 0;

   if (pos < 0 || pos > len)
      return -1;  // Invalid

   if (pos < len)
   {
      if (name[pos++] != '[')
         return -1;

      if (name[pos] == '0' && isdigit(name[pos + 1]))
         return -1;  // Leading zeros are explicitly disallowed by the spec

      while (isdigit(name[pos]))
         off = off * 10 + (name[pos++] - '0');

      if (name[pos++] != ']')
         return -1;

      if (pos < len)
         return -1;
   }

   return off;
}

static const GLSL_BLOCK_MEMBER_T *get_block_member(const GLSL_BLOCK_T *blocks, unsigned block_count, unsigned index, int *block_index)
{
   if (block_index) *block_index = -1;

   for (uint32_t b = 0; b < block_count; b++)
   {
      const GLSL_BLOCK_T *block = &blocks[b];

      if (index < block->num_members) {
         if (block_index) *block_index = block->index;
         return &block->members[index];
      }

      index -= block->num_members;
   }

   return NULL;
}

static const GLSL_BLOCK_MEMBER_T *get_indexed_buffer(const GLSL_PROGRAM_T *p, unsigned index, int *block_index)
{
   return get_block_member(p->buffer_blocks, p->num_buffer_blocks, index, block_index);
}

static const GLSL_BLOCK_MEMBER_T *get_indexed_uniform(const GLSL_PROGRAM_T *p, unsigned index, int *block_index)
{
   if (block_index)
      *block_index = -1;

   if (index < p->default_uniforms.num_members)
      return &p->default_uniforms.members[index];

   index -= p->default_uniforms.num_members;

   return get_block_member(p->uniform_blocks, p->num_uniform_blocks, index, block_index);
}

static bool names_match(const char *name, const char *internal_name, bool is_array, int *array_offset)
{
   /* If the name matches exactly, then this matches the start of the resource */
   if (!strcmp(name, internal_name)) {
      if (array_offset)
         *array_offset = 0;
      return true;
   }

   if (!is_array)
      return false;

   int internal_len = strlen(internal_name);
   int len = get_basename_length(name);
   int off = get_array_offset(name, len);

   if (off < 0 || len < 0)
      return false;
   if (len != internal_len || strncmp(name, internal_name, len))
      return false;

   if (array_offset)
      *array_offset = off;
   return true;
}

static int get_inout_location(const GLSL_INOUT_T *vars, unsigned n_vars, const char *name) {
   for (unsigned i = 0; i < n_vars; i++) {
      const GLSL_INOUT_T *v = &vars[i];
      int off;

      if (names_match(name, v->name, v->is_array, &off) && off < (int)v->array_size)
         return v->index + off;
   }

   return -1;
}

static int get_uniform_location(const GLSL_PROGRAM_T *p, const GLchar *name)
{
   for (unsigned i = 0; i < p->default_uniforms.num_members; i++)
   {
      const GLSL_BLOCK_MEMBER_T *info = &p->default_uniforms.members[i];
      int offset;

      if (names_match(name, info->name, info->is_array, &offset) &&
                      offset < (int)info->array_length)
         return info->offset + offset;
   }

   return -1;
}

static const GLSL_BLOCK_MEMBER_T *get_named_block_var(const GLSL_BLOCK_T *blocks, int num_blocks, const char *name, unsigned *index)
{
   *index = 0;

   for (int b = 0; b < num_blocks; b++)
   {
      for (unsigned u = 0; u < blocks[b].num_members; u++)
      {
         const GLSL_BLOCK_MEMBER_T *unif = &blocks[b].members[u];
         int offset;
         if (names_match(name, unif->name, unif->is_array, &offset))
         {
            if (offset == 0)  // We don't match indexed array items here
               return unif;
            else
               goto end;
         }

         (*index)++;
      }
   }

end:
   *index = GL_INVALID_INDEX;
   return NULL;
}

static const GLSL_BLOCK_MEMBER_T *get_named_uniform(const GLSL_PROGRAM_T *p, const GLchar *name, GLuint *index)
{
   const GLSL_BLOCK_MEMBER_T *ret = get_named_block_var(&p->default_uniforms, 1, name, index);
   if (ret) return ret;

   ret = get_named_block_var(p->uniform_blocks, p->num_uniform_blocks, name, index);
   if (ret) {
      *index += p->default_uniforms.num_members;
      return ret;
   }

   *index = GL_INVALID_INDEX;
   return NULL;
}

static const GLSL_BLOCK_MEMBER_T *get_named_buffer_var(const GLSL_PROGRAM_T *p, const GLchar *name, GLuint *index)
{
   return get_named_block_var(p->buffer_blocks, p->num_buffer_blocks, name, index);
}

static bool is_program_interface(GLenum programInterface)
{
   switch (programInterface)
   {
   case GL_UNIFORM:
   case GL_UNIFORM_BLOCK:
   case GL_ATOMIC_COUNTER_BUFFER:
   case GL_PROGRAM_INPUT:
   case GL_PROGRAM_OUTPUT:
   case GL_TRANSFORM_FEEDBACK_VARYING:
   case GL_BUFFER_VARIABLE:
   case GL_SHADER_STORAGE_BLOCK:
      return true;
   default:
      return false;
   }
}

#if V3D_VER_AT_LEAST(3,3,0,0)

static int get_active_resources(const GLSL_PROGRAM_T *p, GLenum programInterface)
{
   switch (programInterface)
   {
   case GL_UNIFORM:
      {
         int num_uniforms = p->default_uniforms.num_members;
         for (unsigned b = 0; b < p->num_uniform_blocks; b++)
            num_uniforms += p->uniform_blocks[b].num_members;
         return num_uniforms;
      }
   case GL_UNIFORM_BLOCK:
      if (p->num_uniform_blocks == 0)
         return 0;
      else
      {
         GLSL_BLOCK_T *last_block = &p->uniform_blocks[p->num_uniform_blocks - 1];
         return last_block->index + last_block->array_length;
      }
   case GL_SHADER_STORAGE_BLOCK:
      if (p->num_buffer_blocks == 0)
         return 0;
      else
      {
         GLSL_BLOCK_T *last_block = &p->buffer_blocks[p->num_buffer_blocks - 1];
         return last_block->index + last_block->array_length;
      }
   case GL_ATOMIC_COUNTER_BUFFER:
      return p->num_atomic_buffers;
   case GL_PROGRAM_INPUT:
      return p->num_inputs;
   case GL_PROGRAM_OUTPUT:
      return p->num_outputs;
   case GL_TRANSFORM_FEEDBACK_VARYING:
      return p->num_tf_captures;
   case GL_BUFFER_VARIABLE:
      {
         int num_buffers = 0;
         for (unsigned b = 0; b < p->num_buffer_blocks; b++)
            num_buffers += p->buffer_blocks[b].num_members;
         return num_buffers;
      }
   default:
      unreachable();
      return 0;
   }
}

#endif

/* Return name length, adding '[0]' for an array, and including NULL terminator */
static unsigned nonblock_name_len(const char *name, bool is_array) {
   return strlen(name) + (is_array ? 3 : 0) + 1;
}

/* Same as for non-block, but block arrays get '[idx]' appended */
static unsigned block_name_len(const char *name, bool is_array, unsigned idx) {
   return strlen(name) + (is_array ? 3 + (int)log10(idx) : 0) + 1;
}

int glxx_get_max_name_length(const GLSL_PROGRAM_T *p, GLenum interface)
{
   int len = 0;

   switch (interface)
   {
   case GL_UNIFORM:
      for (unsigned i = 0; i < p->default_uniforms.num_members; i++)
         len = gfx_smax(len, (int)nonblock_name_len(p->default_uniforms.members[i].name, p->default_uniforms.members[i].is_array));

      for (unsigned j = 0; j < p->num_uniform_blocks; j++)
         for (unsigned i = 0; i < p->uniform_blocks[j].num_members; i++)
            len = gfx_smax(len, (int)nonblock_name_len(p->uniform_blocks[j].members[i].name, p->uniform_blocks[j].members[i].is_array));
      break;
   case GL_UNIFORM_BLOCK:
   case GL_SHADER_STORAGE_BLOCK:
   {
      unsigned count             = interface == GL_UNIFORM_BLOCK ? p->num_uniform_blocks : p->num_buffer_blocks;
      const GLSL_BLOCK_T *blocks = interface == GL_UNIFORM_BLOCK ? p->uniform_blocks     : p->buffer_blocks;

      for (unsigned i = 0; i < count; i++)
         len = gfx_smax(len, block_name_len(blocks[i].name, blocks[i].is_array, blocks[i].array_length));
      break;
   }
   case GL_PROGRAM_INPUT:
   case GL_PROGRAM_OUTPUT:
   {
      unsigned count        = interface == GL_PROGRAM_INPUT ? p->num_inputs : p->num_outputs;
      const GLSL_INOUT_T *v = interface == GL_PROGRAM_INPUT ? p->inputs     : p->outputs;
      for (unsigned i = 0; i < count; i++)
         len = gfx_smax(len, (int)nonblock_name_len(v[i].name, v[i].is_array));
      break;
   }
   case GL_TRANSFORM_FEEDBACK_VARYING:
      for (unsigned i = 0; i < p->num_tf_captures; i++)
         len = gfx_smax(len, (int)strlen(p->tf_capture[i].name) + 1);
      break;
   case GL_BUFFER_VARIABLE:
      for (unsigned j = 0; j < p->num_buffer_blocks; j++)
         for (unsigned i = 0; i < p->buffer_blocks[j].num_members; i++)
            len = gfx_smax(len, (int)nonblock_name_len(p->buffer_blocks[j].members[i].name, p->buffer_blocks[j].members[i].is_array));
      break;
   default:
      unreachable();
   }

   return len;
}

static unsigned get_atomic_num_active_vars(const GLSL_PROGRAM_T *p, unsigned index)
{
   unsigned count = 0;
   for (unsigned i=0; i<p->default_uniforms.num_members; i++)
   {
      if (p->default_uniforms.members[i].atomic_idx == index)
         count++;
   }
   return count;
}

#if V3D_VER_AT_LEAST(3,3,0,0)

static bool interface_valid_for_max_num_active(GLenum interface) {
   return interface == GL_UNIFORM_BLOCK || interface == GL_SHADER_STORAGE_BLOCK || interface == GL_ATOMIC_COUNTER_BUFFER;
}

static int get_max_num_active_variables(const GL20_PROGRAM_T *prog, GLenum interface)
{
   unsigned n = 0;

   switch (interface)
   {
   case GL_UNIFORM_BLOCK:
      for (unsigned i = 0; i < prog->common.linked_glsl_program->num_uniform_blocks; i++)
         n = gfx_umax(n, prog->common.linked_glsl_program->uniform_blocks[i].num_members);
      break;
   case GL_SHADER_STORAGE_BLOCK:
      for (unsigned i = 0; i < prog->common.linked_glsl_program->num_buffer_blocks; i++)
         n = gfx_umax(n, prog->common.linked_glsl_program->buffer_blocks[i].num_members);
      break;
   case GL_ATOMIC_COUNTER_BUFFER:
      for (unsigned i=0; i<prog->common.linked_glsl_program->num_atomic_buffers; i++)
         n = gfx_umax(n, get_atomic_num_active_vars(prog->common.linked_glsl_program, i));
      break;
   default:
      unreachable();
   }

   return n;
}

#endif

static unsigned get_program_resource_index(const GLSL_PROGRAM_T *p, GLenum interface, const char *name)
{
   switch (interface)
   {
   case GL_UNIFORM:
   {
      unsigned i;
      get_named_uniform(p, name, &i);
      return i;
   }
   case GL_BUFFER_VARIABLE:
   {
      unsigned i;
      get_named_buffer_var(p, name, &i);
      return i;
   }
   case GL_UNIFORM_BLOCK:
   case GL_SHADER_STORAGE_BLOCK:
   {
      unsigned count           = interface == GL_UNIFORM_BLOCK ? p->num_uniform_blocks : p->num_buffer_blocks;
      const GLSL_BLOCK_T *base = interface == GL_UNIFORM_BLOCK ? p->uniform_blocks     : p->buffer_blocks;

      for (unsigned i = 0; i < count; i++)
      {
         const GLSL_BLOCK_T *block = &base[i];
         int                 offset;

         if (names_match(name, block->name, block->is_array, &offset))
         {
            if (offset < block->array_length)
               return block->index + offset;
         }
      }
      break;
   }
   case GL_PROGRAM_INPUT:
   case GL_PROGRAM_OUTPUT:
   {
      unsigned count        = interface == GL_PROGRAM_INPUT ? p->num_inputs : p->num_outputs;
      const GLSL_INOUT_T *v = interface == GL_PROGRAM_INPUT ? p->inputs     : p->outputs;

      for (unsigned i = 0; i < count; i++) {
         if (names_match(name, v[i].name, v[i].is_array, NULL))
            return i;
      }
      break;
   }
   case GL_TRANSFORM_FEEDBACK_VARYING:
      /* TF Varyings are an exception to the name matching rules. Names must match exactly */
      for (unsigned i = 0; i < p->num_tf_captures; i++)
         if (strcmp(name, p->tf_capture[i].name) == 0)
            return i;
      break;
   default:
      unreachable();
   }

   return GL_INVALID_INDEX;
}

static bool index_valid_for_interface(const GLSL_PROGRAM_T *p, GLenum interface, unsigned index)
{
   switch (interface)
   {
   case GL_UNIFORM:
      return (get_indexed_uniform(p, index, NULL) != NULL);
   case GL_BUFFER_VARIABLE:
      return (get_indexed_buffer(p, index, NULL) != NULL);
   case GL_UNIFORM_BLOCK:
      return (gl20_get_ubo_from_index(p, index) != NULL);
   case GL_SHADER_STORAGE_BLOCK:
      return (gl20_get_ssbo_from_index(p, index) != NULL);
   case GL_PROGRAM_INPUT:
      return (index < p->num_inputs);
   case GL_PROGRAM_OUTPUT:
      return (index < p->num_outputs);
   case GL_TRANSFORM_FEEDBACK_VARYING:
      return (index < p->num_tf_captures);
   case GL_ATOMIC_COUNTER_BUFFER:
      return (index < p->num_atomic_buffers);
   default:
      unreachable();
      return false;
   }
}

static void get_program_resource_name(const GLSL_PROGRAM_T *prog, GLenum interface, unsigned index,
                                      unsigned buf_size, GLsizei *length, char *name)
{
   switch (interface)
   {
   case GL_UNIFORM:
   case GL_BUFFER_VARIABLE:
      {
         const GLSL_BLOCK_MEMBER_T *v = interface == GL_UNIFORM ? get_indexed_uniform(prog, index, NULL) :
                                                                  get_indexed_buffer (prog, index, NULL);
         if (v->is_array)
            strzncpy_with_array(name, v->name, buf_size, 0);
         else
            strzncpy(name, v->name, buf_size);
      }
      break;
   case GL_UNIFORM_BLOCK:
   case GL_SHADER_STORAGE_BLOCK:
      {
         const GLSL_BLOCK_T *block = (interface == GL_UNIFORM_BLOCK) ? gl20_get_ubo_from_index(prog, index) :
                                                                       gl20_get_ssbo_from_index(prog, index);
         if (block->is_array)
            strzncpy_with_array(name, block->name, buf_size, index - block->index);
         else
            strzncpy(name, block->name, buf_size);
      }
      break;
   case GL_PROGRAM_INPUT:
   case GL_PROGRAM_OUTPUT:
      {
         const GLSL_INOUT_T *v = (interface == GL_PROGRAM_INPUT) ? &prog->inputs[index] :
                                                                   &prog->outputs[index];
         if (v->is_array)
            strzncpy_with_array(name, v->name, buf_size, 0);
         else
            strzncpy(name, v->name, buf_size);
      }
      break;
   case GL_TRANSFORM_FEEDBACK_VARYING:
      strzncpy(name, prog->tf_capture[index].name, buf_size);
      break;
   default:
      unreachable();
   }

   if (length != NULL)
   {
      if (buf_size <= 0)
         *length = 0;
      else
         *length = strlen(name);
   }
}

static int get_program_resource_location(const GLSL_PROGRAM_T *p, GLenum interface, const GLchar *name)
{
   switch (interface)
   {
   case GL_UNIFORM:
      return get_uniform_location(p, name);
   case GL_PROGRAM_INPUT:
      return get_inout_location(p->inputs, p->num_inputs, name);
   case GL_PROGRAM_OUTPUT:
      return get_inout_location(p->outputs, p->num_outputs, name);
   default:
      unreachable();
   }

   return -1;
}

// Generated from Table 7.2 in ES3.1 spec (April 29, 2015)
const static GLenum valid_uniform_props[] =
{
   GL_NAME_LENGTH,
   GL_ARRAY_SIZE,
   GL_ARRAY_STRIDE,
   GL_BLOCK_INDEX,
   GL_IS_ROW_MAJOR,
   GL_MATRIX_STRIDE,
   GL_ATOMIC_COUNTER_BUFFER_INDEX,
   GL_LOCATION,
   GL_OFFSET,
   GL_REFERENCED_BY_VERTEX_SHADER,
   GL_REFERENCED_BY_FRAGMENT_SHADER,
   GL_REFERENCED_BY_COMPUTE_SHADER,
   GL_TYPE,
#if V3D_VER_AT_LEAST(4,1,34,0)
   GL_REFERENCED_BY_TESS_CONTROL_SHADER,
   GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
   GL_REFERENCED_BY_GEOMETRY_SHADER,
#endif
};

const static GLenum valid_buffer_props[] =
{
   GL_NAME_LENGTH,
   GL_ACTIVE_VARIABLES,
   GL_BUFFER_BINDING,
   GL_NUM_ACTIVE_VARIABLES,
   GL_BUFFER_DATA_SIZE,
   GL_REFERENCED_BY_VERTEX_SHADER,
   GL_REFERENCED_BY_FRAGMENT_SHADER,
   GL_REFERENCED_BY_COMPUTE_SHADER,
#if V3D_VER_AT_LEAST(4,1,34,0)
   GL_REFERENCED_BY_TESS_CONTROL_SHADER,
   GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
   GL_REFERENCED_BY_GEOMETRY_SHADER,
#endif
};

const static GLenum valid_atomic_props[] =
{
   GL_ACTIVE_VARIABLES,
   GL_BUFFER_BINDING,
   GL_NUM_ACTIVE_VARIABLES,
   GL_BUFFER_DATA_SIZE,
   GL_REFERENCED_BY_VERTEX_SHADER,
   GL_REFERENCED_BY_FRAGMENT_SHADER,
   GL_REFERENCED_BY_COMPUTE_SHADER,
#if V3D_VER_AT_LEAST(4,1,34,0)
   GL_REFERENCED_BY_TESS_CONTROL_SHADER,
   GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
   GL_REFERENCED_BY_GEOMETRY_SHADER,
#endif
};

const static GLenum valid_inout_props[] =
{
   GL_NAME_LENGTH,
   GL_ARRAY_SIZE,
   GL_LOCATION,
   GL_REFERENCED_BY_VERTEX_SHADER,
   GL_REFERENCED_BY_FRAGMENT_SHADER,
   GL_REFERENCED_BY_COMPUTE_SHADER,
   GL_TYPE,
#if V3D_VER_AT_LEAST(4,1,34,0)
   GL_IS_PER_PATCH,
   GL_REFERENCED_BY_TESS_CONTROL_SHADER,
   GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
   GL_REFERENCED_BY_GEOMETRY_SHADER,
#endif
};

const static GLenum valid_tfv_props[] =
{
   GL_NAME_LENGTH,
   GL_ARRAY_SIZE,
   GL_TYPE
};

const static GLenum valid_buffer_var_props[] =
{
   GL_NAME_LENGTH,
   GL_ARRAY_SIZE,
   GL_ARRAY_STRIDE,
   GL_BLOCK_INDEX,
   GL_IS_ROW_MAJOR,
   GL_MATRIX_STRIDE,
   GL_OFFSET,
   GL_REFERENCED_BY_VERTEX_SHADER,
   GL_REFERENCED_BY_FRAGMENT_SHADER,
   GL_REFERENCED_BY_COMPUTE_SHADER,
   GL_TOP_LEVEL_ARRAY_SIZE,
   GL_TOP_LEVEL_ARRAY_STRIDE,
   GL_TYPE,
#if V3D_VER_AT_LEAST(4,1,34,0)
   GL_REFERENCED_BY_TESS_CONTROL_SHADER,
   GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
   GL_REFERENCED_BY_GEOMETRY_SHADER,
#endif
};

static GLenum valid_props_combination(GLenum interface, GLenum prop)
{
   // Check for valid prop
   switch (prop)
   {
   case GL_NAME_LENGTH:
   case GL_ARRAY_SIZE:
   case GL_ARRAY_STRIDE:
   case GL_BLOCK_INDEX:
   case GL_IS_ROW_MAJOR:
   case GL_MATRIX_STRIDE:
   case GL_ATOMIC_COUNTER_BUFFER_INDEX:
   case GL_LOCATION:
   case GL_OFFSET:
   case GL_REFERENCED_BY_VERTEX_SHADER:
   case GL_REFERENCED_BY_FRAGMENT_SHADER:
   case GL_REFERENCED_BY_COMPUTE_SHADER:
   case GL_TYPE:
   case GL_ACTIVE_VARIABLES:
   case GL_BUFFER_BINDING:
   case GL_NUM_ACTIVE_VARIABLES:
   case GL_BUFFER_DATA_SIZE:
   case GL_TOP_LEVEL_ARRAY_SIZE:
   case GL_TOP_LEVEL_ARRAY_STRIDE:
#if V3D_VER_AT_LEAST(4,1,34,0)
   case GL_IS_PER_PATCH:
   case GL_REFERENCED_BY_TESS_CONTROL_SHADER:
   case GL_REFERENCED_BY_TESS_EVALUATION_SHADER:
   case GL_REFERENCED_BY_GEOMETRY_SHADER:
#endif
      break;
   default:
      return GL_INVALID_ENUM;
   }

#define USE_ARRAY(a)\
   propsArray = (a);\
   num = sizeof(a) / sizeof(GLenum);

   const GLenum *propsArray = NULL;
   unsigned num;
   switch (interface)
   {
   case GL_UNIFORM:                     USE_ARRAY(valid_uniform_props); break;
   case GL_UNIFORM_BLOCK:               USE_ARRAY(valid_buffer_props); break;
   case GL_SHADER_STORAGE_BLOCK:        USE_ARRAY(valid_buffer_props); break;
   case GL_ATOMIC_COUNTER_BUFFER:       USE_ARRAY(valid_atomic_props); break;
   case GL_PROGRAM_INPUT:               USE_ARRAY(valid_inout_props); break;
   case GL_PROGRAM_OUTPUT:              USE_ARRAY(valid_inout_props); break;
   case GL_TRANSFORM_FEEDBACK_VARYING:  USE_ARRAY(valid_tfv_props); break;
   case GL_BUFFER_VARIABLE:             USE_ARRAY(valid_buffer_var_props); break;
   default:
      return GL_INVALID_ENUM;
   }

#undef USE_ARRAY

   for (unsigned i = 0; i < num; i++)
   {
      if (propsArray[i] == prop)
         return GL_NO_ERROR;
   }

   // Must be an invalid combination
   return GL_INVALID_OPERATION;
}

static int get_block_member_prop(const GLSL_BLOCK_MEMBER_T *v, int block_index, GLenum prop)
{
   switch (prop)
   {
   case GL_NAME_LENGTH:                   return nonblock_name_len(v->name, v->is_array);
   case GL_TYPE:                          return v->type;
   case GL_ARRAY_SIZE:                    return v->array_length;
   case GL_ARRAY_STRIDE:                  return v->array_stride;
   case GL_TOP_LEVEL_ARRAY_SIZE:          return v->top_level_size;
   case GL_TOP_LEVEL_ARRAY_STRIDE:        return v->top_level_stride;
   case GL_BLOCK_INDEX:                   return block_index;
   case GL_IS_ROW_MAJOR:                  return v->column_major ? 0 : 1;
   case GL_MATRIX_STRIDE:                 return v->matrix_stride;
   case GL_ATOMIC_COUNTER_BUFFER_INDEX:   return v->atomic_idx;

   case GL_LOCATION:
      if (block_index == -1 && v->atomic_idx == -1)
         return v->offset;
      else
         return -1;
   case GL_OFFSET:
      if (block_index == -1 && v->atomic_idx == -1)
         return -1;
      else
         return v->offset;

   case GL_REFERENCED_BY_VERTEX_SHADER:          return v->used_in_vs;
   case GL_REFERENCED_BY_FRAGMENT_SHADER:        return v->used_in_fs;
   case GL_REFERENCED_BY_COMPUTE_SHADER:         return v->used_in_cs;
#if V3D_VER_AT_LEAST(4,1,34,0)
   case GL_REFERENCED_BY_TESS_CONTROL_SHADER:    return v->used_in_tcs;
   case GL_REFERENCED_BY_TESS_EVALUATION_SHADER: return v->used_in_tes;
   case GL_REFERENCED_BY_GEOMETRY_SHADER:        return v->used_in_gs;
#endif

   default:
      unreachable();
   }
}

// Common properties of UBO and SSBOs.
static int get_block_resource_prop(const GLSL_BLOCK_T *block, GLenum prop)
{
   switch (prop)
   {
   case GL_NAME_LENGTH:                          return block_name_len(block->name, block->is_array, block->array_length);
   case GL_NUM_ACTIVE_VARIABLES:                 return block->num_members;
   case GL_BUFFER_DATA_SIZE:                     return block->size;
   case GL_REFERENCED_BY_VERTEX_SHADER:          return block->used_in_vs;
   case GL_REFERENCED_BY_FRAGMENT_SHADER:        return block->used_in_fs;
   case GL_REFERENCED_BY_COMPUTE_SHADER:         return block->used_in_cs;
#if V3D_VER_AT_LEAST(4,1,34,0)
   case GL_REFERENCED_BY_TESS_CONTROL_SHADER:    return block->used_in_tcs;
   case GL_REFERENCED_BY_TESS_EVALUATION_SHADER: return block->used_in_tes;
   case GL_REFERENCED_BY_GEOMETRY_SHADER:        return block->used_in_gs;
#endif
   default:
      unreachable();
   }
}

static int active_variables(const GLSL_BLOCK_T *base, unsigned idx_offset, const GLSL_BLOCK_T *block, int *params, unsigned space)
{
   unsigned count = block->num_members;
   unsigned index = idx_offset;

   /* Loop over previous blocks */
   for (unsigned j = 0; base[j].index < block->index; j++)
      index += base[j].num_members;

   if (space < count)
      count = space;

   /* Write out the index array */
   for (unsigned j = 0; j < count; j++)
      params[j] = index++;

   return count;
}

// Returns how many entries it wrote into params. Must never write more than 'space' entries.
static unsigned get_uniform_block_resource_prop(const GL20_PROGRAM_T *prog, unsigned index,
                                                GLenum prop, int *params, unsigned space)
{
   const GLSL_BLOCK_T *block = gl20_get_ubo_from_index(prog->common.linked_glsl_program, index);
   assert(block != NULL);

   switch (prop)
   {
   case GL_ACTIVE_VARIABLES:
      return active_variables(prog->common.linked_glsl_program->uniform_blocks,
                              prog->common.linked_glsl_program->default_uniforms.num_members,
                              block, params, space);
   case GL_BUFFER_BINDING:
      params[0] = prog->common.ubo_binding_point[index];
      return 1;
   default:
      params[0] = get_block_resource_prop(block, prop);
      return 1;
   }
}

// Returns how many entries it wrote into params. Must never write more than 'space' entries.
static unsigned get_shader_storage_block_resource_prop(const GL20_PROGRAM_T *prog, unsigned index,
                                                       GLenum prop, int *params, unsigned space)
{
   const GLSL_BLOCK_T *block = gl20_get_ssbo_from_index(prog->common.linked_glsl_program, index);
   assert(block != NULL);

   switch (prop)
   {
   case GL_ACTIVE_VARIABLES:
      return active_variables(prog->common.linked_glsl_program->buffer_blocks, 0, block, params, space);
   case GL_BUFFER_BINDING:
      params[0] = prog->common.ssbo_binding_point[index];
      return 1;
   default:
      params[0] = get_block_resource_prop(block, prop);
      return 1;
   }
}

static int inout_var_resource_prop(GLSL_INOUT_T *v, GLenum prop) {
   switch (prop) {
   case GL_NAME_LENGTH:                   return nonblock_name_len(v->name, v->is_array);
   case GL_ARRAY_SIZE:                    return v->array_size;
   case GL_LOCATION:                      return v->index;
   case GL_TYPE:                          return v->type;
   case GL_REFERENCED_BY_VERTEX_SHADER:   return v->used_in_vs;
   case GL_REFERENCED_BY_FRAGMENT_SHADER: return v->used_in_fs;
   case GL_REFERENCED_BY_COMPUTE_SHADER:  return v->used_in_cs;
#if V3D_VER_AT_LEAST(4,1,34,0)
   case GL_REFERENCED_BY_TESS_CONTROL_SHADER:    return v->used_in_tcs;
   case GL_REFERENCED_BY_TESS_EVALUATION_SHADER: return v->used_in_tes;
   case GL_REFERENCED_BY_GEOMETRY_SHADER:        return v->used_in_gs;
   case GL_IS_PER_PATCH:                         return v->is_per_patch;
#endif
   default:
      unreachable();
      return 0;
   }
}

// Returns how many entries it wrote into params. Must never write more than 'space' entries.
static int get_atomic_counter_buffer_resource_prop(const GLSL_PROGRAM_T *p, unsigned index,
                                                   GLenum prop, GLint *params, int space)
{
   assert(space > 0);

   const GLSL_ATOMIC_BUF_T *buf = &p->atomic_buffers[index];

   switch (prop)
   {
   case GL_ACTIVE_VARIABLES:
      {
         int count = 0;
         for (unsigned i=0; i<p->default_uniforms.num_members && count < space; i++)
         {
            if (p->default_uniforms.members[i].atomic_idx == index)
               params[count++] = i;
         }
         return count;
      }
   case GL_BUFFER_BINDING:
      params[0] = buf->binding;
      return 1;
   case GL_NUM_ACTIVE_VARIABLES:
      params[0] = get_atomic_num_active_vars(p, index);
      return 1;
   case GL_BUFFER_DATA_SIZE:
      params[0] = buf->size;
      return 1;
   case GL_REFERENCED_BY_VERTEX_SHADER:
      params[0] = buf->used_in_vs;
      return 1;
   case GL_REFERENCED_BY_FRAGMENT_SHADER:
      params[0] = buf->used_in_fs;
      return 1;
   case GL_REFERENCED_BY_COMPUTE_SHADER:
      params[0] = buf->used_in_cs;
      return 1;
#if V3D_VER_AT_LEAST(4,1,34,0)
   case GL_REFERENCED_BY_TESS_CONTROL_SHADER:
      params[0] = buf->used_in_tcs;
      return 1;
   case GL_REFERENCED_BY_TESS_EVALUATION_SHADER:
      params[0] = buf->used_in_tes;
      return 1;
   case GL_REFERENCED_BY_GEOMETRY_SHADER:
      params[0] = buf->used_in_gs;
      return 1;
#endif
   default:
      unreachable();
      return 0;
   }
}

static int get_transform_feedback_resource_prop(const GLSL_TF_CAPTURE_T *tff, GLenum prop)
{
   switch (prop)
   {
   case GL_NAME_LENGTH:    return strlen(tff->name) + 1;
   case GL_ARRAY_SIZE:     return tff->array_length;
   case GL_TYPE:           return tff->type;
   default: unreachable(); return 0;
   }
}

// Returns how many entries it wrote into params. Must never write more than 'space' entries.
static unsigned get_program_resource_prop(const GL20_PROGRAM_T *program, GLenum interface, unsigned index,
                                          GLenum prop, int *params, unsigned space)
{
   unsigned added = 0;

   switch (interface)
   {
   case GL_UNIFORM:
   case GL_BUFFER_VARIABLE:
   {
      int block_index = -1;
      const GLSL_PROGRAM_T *p = program->common.linked_glsl_program;
      const GLSL_BLOCK_MEMBER_T *v = interface == GL_UNIFORM ? get_indexed_uniform(p, index, &block_index) :
                                                               get_indexed_buffer (p, index, &block_index);
      params[0] = get_block_member_prop(v, block_index, prop);
      return 1;
   }
   case GL_UNIFORM_BLOCK:
      added = get_uniform_block_resource_prop(program, index, prop, params, space);
      break;
   case GL_SHADER_STORAGE_BLOCK:
      added = get_shader_storage_block_resource_prop(program, index, prop, params, space);
      break;
   case GL_PROGRAM_INPUT:
      params[0] = inout_var_resource_prop(&program->common.linked_glsl_program->inputs[index], prop);
      return 1;
   case GL_PROGRAM_OUTPUT:
      params[0] = inout_var_resource_prop(&program->common.linked_glsl_program->outputs[index], prop);
      return 1;
   case GL_TRANSFORM_FEEDBACK_VARYING:
      params[0] = get_transform_feedback_resource_prop(&program->common.linked_glsl_program->tf_capture[index], prop);
      return 1;
   case GL_ATOMIC_COUNTER_BUFFER:
      added = get_atomic_counter_buffer_resource_prop(program->common.linked_glsl_program, index, prop, params, space);
      break;
   default:
      unreachable();
   }

   assert(added <= space);
   return added;
}

#if V3D_VER_AT_LEAST(3,3,0,0)

static bool interface_valid_for_max_name_length(GLenum interface) {
   return interface == GL_UNIFORM       || interface == GL_BUFFER_VARIABLE      ||
          interface == GL_UNIFORM_BLOCK || interface == GL_SHADER_STORAGE_BLOCK ||
          interface == GL_PROGRAM_INPUT || interface == GL_PROGRAM_OUTPUT       ||
          interface == GL_TRANSFORM_FEEDBACK_VARYING;
}

GL_APICALL void GL_APIENTRY glGetProgramInterfaceiv(GLuint p, GLenum programInterface, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state) return;

   if (!is_program_interface(programInterface))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   switch (pname)
   {
   case GL_ACTIVE_RESOURCES:
      break;
   case GL_MAX_NAME_LENGTH:
      if (!interface_valid_for_max_name_length(programInterface)) {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         goto end;
      }
      break;
   case GL_MAX_NUM_ACTIVE_VARIABLES:
      if (!interface_valid_for_max_num_active(programInterface)) {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         goto end;
      }
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   GL20_PROGRAM_T *program = gl20_get_program(state, p);  // takes care of setting correct error
   if (!program)
      goto end;

   if (!program->common.link_status)
   {
      // Failed or unlinked programs can be assumed to have no resources
      params[0] = 0;
      goto end;
   }

   switch (pname)
   {
   case GL_ACTIVE_RESOURCES:
      params[0] = get_active_resources(program->common.linked_glsl_program, programInterface);
      break;
   case GL_MAX_NAME_LENGTH:
      params[0] = glxx_get_max_name_length(program->common.linked_glsl_program, programInterface);
      break;
   case GL_MAX_NUM_ACTIVE_VARIABLES:
      params[0] = get_max_num_active_variables(program, programInterface);
      break;
   default:
      unreachable();
   }

end:
   glxx_unlock_server_state();
}

#endif

unsigned glxx_get_program_resource_index(GLXX_SERVER_STATE_T *state,
                                         unsigned p, GLenum interface, const char *name)
{
   GLenum   error = GL_NO_ERROR;
   unsigned indx  = GL_INVALID_INDEX;

   if (!is_program_interface(interface) || interface == GL_ATOMIC_COUNTER_BUFFER)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   GL20_PROGRAM_T *program = gl20_get_program(state, p);  // takes care of setting correct error
   if (!program)
      goto end;

   if (program->common.linked_glsl_program == NULL)
   {
      // Failed or unlinked programs can be assumed to have no resources
      goto end;
   }

   if (name == NULL || name[0] == '\0')
      goto end;

   indx = get_program_resource_index(program->common.linked_glsl_program, interface, name);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   return indx;
}

#if V3D_VER_AT_LEAST(3,3,0,0)

GL_APICALL GLuint GL_APIENTRY glGetProgramResourceIndex(GLuint p, GLenum programInterface, const GLchar *name)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   GLuint              ret = GL_INVALID_INDEX;

   if (!state)
      return ret;

   ret = glxx_get_program_resource_index(state, p, programInterface, name);

   glxx_unlock_server_state();

   return ret;
}

#endif

void glxx_get_program_resource_name(GLXX_SERVER_STATE_T *state,
                                    unsigned p, GLenum interface, unsigned index, GLsizei buf_size,
                                    GLsizei *length, char *name)
{
   GLenum error = GL_NO_ERROR;

   if (!is_program_interface(interface) || interface == GL_ATOMIC_COUNTER_BUFFER)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (buf_size < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   GL20_PROGRAM_T *program = gl20_get_program(state, p);  // takes care of setting correct error
   if (!program)
      goto end;

   if (program->common.linked_glsl_program == NULL)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (!index_valid_for_interface(program->common.linked_glsl_program, interface, index))
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   get_program_resource_name(program->common.linked_glsl_program, interface, index, buf_size, length, name);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
}

#if V3D_VER_AT_LEAST(3,3,0,0)

GL_APICALL void GL_APIENTRY glGetProgramResourceName(GLuint p, GLenum programInterface, GLuint index, GLsizei bufSize,
                                                     GLsizei *length, GLchar *name)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state)
      return;

   glxx_get_program_resource_name(state, p, programInterface, index, bufSize, length, name);

   glxx_unlock_server_state();
}

#endif

bool glxx_get_program_resourceiv(GLXX_SERVER_STATE_T *state, unsigned p, GLenum interface,
                                 unsigned index, GLsizei propCount, const GLenum *props,
                                 GLsizei buf_size, GLsizei *length, int *params)
{
   GLenum error = GL_NO_ERROR;

   if (!is_program_interface(interface))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (propCount <= 0 || buf_size < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   GL20_PROGRAM_T *program = gl20_get_program(state, p);  // takes care of setting correct error
   if (!program)
      goto end;

   if (program->common.linked_glsl_program == NULL)
   {
      // Failed or unlinked programs can be assumed to have no resources
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (!index_valid_for_interface(program->common.linked_glsl_program, interface, index))
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   // Check for more argument error conditions
   for (GLsizei p = 0; p < propCount; p++)
   {
      error = valid_props_combination(interface, props[p]);
      if (error != GL_NO_ERROR)
         goto end;
   }

   unsigned space = buf_size;

   for (GLsizei p = 0; p < propCount && space > 0; p++)
   {
      unsigned added = get_program_resource_prop(program, interface, index, props[p], params, space);
      space = space - added;
      params += added;
   }

   if (length)
      *length = buf_size - space;

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   return error == GL_NO_ERROR;
}

#if V3D_VER_AT_LEAST(3,3,0,0)

GL_APICALL void GL_APIENTRY glGetProgramResourceiv(GLuint p, GLenum programInterface, GLuint index, GLsizei propCount,
                                                   const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state)
      return;

   glxx_get_program_resourceiv(state, p, programInterface, index, propCount, props, bufSize, length, params);

   glxx_unlock_server_state();
}

#endif

int glxx_get_program_resource_location(GLXX_SERVER_STATE_T *state, unsigned p,
                                       GLenum interface, const char *name)
{
   GLenum error = GL_NO_ERROR;
   GLint  location = -1;

   switch (interface)
   {
   case GL_UNIFORM:
   case GL_PROGRAM_INPUT:
   case GL_PROGRAM_OUTPUT:
      break;
   default:
      error = GL_INVALID_ENUM;
      goto end;
   }

   GL20_PROGRAM_T *program = gl20_get_program(state, p);  // takes care of setting correct error
   if (!program)
      goto end;

   if (!program->common.link_status)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   if (name == NULL || name[0] == '\0')
      goto end;

   location = get_program_resource_location(program->common.linked_glsl_program, interface, name);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   return location;
}

#if V3D_VER_AT_LEAST(3,3,0,0)

GL_APICALL GLint GL_APIENTRY glGetProgramResourceLocation(GLuint p, GLenum programInterface, const GLchar *name)
{
   GLXX_SERVER_STATE_T *state;
   GLint  location = -1;

   state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state)
      return location;

   location = glxx_get_program_resource_location(state, p, programInterface, name);

   glxx_unlock_server_state();

   return location;
}

#endif
