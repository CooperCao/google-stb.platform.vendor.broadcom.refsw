/*=============================================================================
 * Copyright (c) 2015 Broadcom Europe Limited.
 * All rights reserved.
 *
 * Project  :  khronos
 * Module   :  Header file
 *
 * FILE DESCRIPTION
 * Program interface API functions for ES3.1
 * =============================================================================*/

#include "vcos.h"
#include "vcos_logging.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "helpers/snprintf.h"
#include "middleware/khronos/glxx/glxx_log.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/gl20/gl20_shader.h"
#include "middleware/khronos/glsl/glsl_compiler.h"
#include "middleware/khronos/glxx/glxx_server_program_interface.h"

#include <math.h> // For log10

#define NOT_YET_IMPLEMENTED_TODO { vcos_logc_error((&glxx_prog_iface_log), "Not yet implemented in %s : line %d", __FILE__, __LINE__); }

/*
   A null-terminating version of strncpy. Copies a string from src
   to dst with a maximum length of len, and forcibly null-terminates
   the result. Returns the number of characters written, not
   including the null terminator, or -1 either dst is NULL or length
   is less than 1 (giving us no space to even write the terminator).
*/
static size_t strzncpy(char *dst, const char *src, size_t len)
{
   if (dst && len > 0) {
      strncpy(dst, src, len);

      dst[len - 1] = '\0';

      return strlen(dst);
   } else
      return -1;
}

static size_t strzncpy_with_array(char *dst, const char *src, size_t len, int arr_off) {
   if (dst == NULL || len <= 0) return -1;

   snprintf(dst, len, "%s[%d]", src, arr_off);
   dst[len-1] = '\0';
   return strlen(dst);
}

/* The Unity engine (and possibly other apps) expect uniform locations to be packed
 * into minimal values (it uses them for array allocations).
 *
 * We will return the uniform offsets that were calculated during program link, with
 * the addition of the array or structure index that is being requested. Since the
 * calculated offsets already make room for all the array or structure elements, the
 * locations are guaranteed not to overlap.
  */
GLint glxx_encode_location(unsigned index, unsigned offset)
{
   return index + offset;
}

void glxx_decode_location(const GLSL_BLOCK_T *block, unsigned location, unsigned *index, unsigned *offset)
{
   unsigned i;
   for (i = 1; i < block->num_members; i++)
   {
      if (block->members[i].offset > location)
         break;
   }

   i--;

   *index = i;
   *offset = location - block->members[i].offset;
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

static int count_indices(const char *name)
{
   int len = strlen(name) - 1;
   int count = 0;

   if (len >= 0)
      if (name[len] != ']')
         return 0;

   while (len >= 0)
   {
      if (name[len] == '[')
      {
         count++;

         // Early exit if possible
         if (len > 0 && name[len - 1] != ']')
            return count;
      }
      len--;
   }

   return count;
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

static GLSL_BLOCK_MEMBER_T *get_block_member(GLSL_BLOCK_T *blocks, unsigned block_count, unsigned index, int *block_index)
{
   if (block_index) *block_index = -1;

   for (uint32_t b = 0; b < block_count; b++)
   {
      GLSL_BLOCK_T *block = &blocks[b];

      if (index < block->num_members) {
         if (block_index) *block_index = block->index;
         return &block->members[index];
      }

      index -= block->num_members;
   }

   return NULL;
}

static GLSL_BLOCK_MEMBER_T *get_indexed_buffer(GLSL_PROGRAM_T *p, GLuint index, GLint *blockIndex)
{
   return get_block_member(p->buffer_blocks, p->num_buffer_blocks, index, blockIndex);
}

static GLSL_BLOCK_MEMBER_T *get_indexed_uniform(GLSL_PROGRAM_T *p, GLuint index, GLint *blockIndex)
{
   if (blockIndex)
      *blockIndex = -1;

   if (index < p->default_uniforms.num_members)
      return &p->default_uniforms.members[index];

   index -= p->default_uniforms.num_members;

   return get_block_member(p->uniform_blocks, p->num_uniform_blocks, index, blockIndex);
}

static bool inner_names_match(const char *in_name, const char *unif_name)
{
   /* Names match if they're the same, or if appending '[0]' to in_name would make them the same */
   if (!strcmp(in_name, unif_name))
      return true; // Exact match

   int unif_name_len = strlen(unif_name);
   int in_name_len = strlen(in_name);

   if (unif_name_len == in_name_len + 3)
   {
      if (!strncmp(in_name, unif_name, in_name_len) &&
          !strncmp(unif_name + in_name_len, "[0]", 3))
          return true;
   }

   return false;
}

typedef enum
{
   ARRAY,
   NOT_ARRAY,
   INTERNAL_NAME_IMPLIES_ARRAY
} ARRAY_TYPE;

// name is the name passed via the APIs
// internal_name is the name stored internally
// array_offset is filled with the array offset given in name
//   if array_offset is NULL, array offsets other than 0 will not match
//
// array_type determines how we detect if internal_name is an array type:
//    ARRAY     = internal_name is an array type, but won't have [0] as a suffix
//    NOT_ARRAY = internal_name is not an array type
//    INTERNAL_NAME_IMPLIES_ARRAY = if name ends with [0], it is an array, otherwise not
//
static bool names_match(const char *name, const char *internal_name, ARRAY_TYPE array_type, int *array_offset)
{
   const char *match_internal_name = NULL;
   char *extended = NULL;
   bool ret = false;

   if (array_type == ARRAY)
   {
      // internal_name is an array type, but doesn't have [0] at the end.
      // Add the [0] to make the remaining matching code the same for all cases.
      char *extended = malloc(strlen(internal_name) + 3 + 1);
      if (extended == NULL)
         goto end2;
      strcpy(extended, internal_name);
      strcat(extended, "[0]");
      match_internal_name = extended;
   }
   else
      match_internal_name = internal_name;

   // NOT_ARRAY and INTERNAL_NAME_IMPLIES_ARRAY are both covered by this code
   int len = get_basename_length(name);
   int off = get_array_offset(name, len);

   if (off < 0 || len < 0)
   {
      if (array_offset)
         *array_offset = -1;
      ret = false;  /* Invalid array offset specified */
      goto end;
   }

   // name may or may not have a final array index attached
   // a[0] or a[0][0] should both match with internal name a[0][0] for example
   // a[0][1] should match with uniform a[0][0] and give an array_offset of 1

   // So, we actually need to count the number of indices to decide what to do
   int unif_index_count = count_indices(match_internal_name);
   int name_index_count = count_indices(name);

   if (array_offset)
   {
      if (unif_index_count == name_index_count)
         *array_offset = off;
      else
         *array_offset = 0;
   }

   // If neither name has array indexing, or if our name has one more element
   // than the given name, do a standard name match.
   if ((unif_index_count == 0 && name_index_count == 0) ||
        unif_index_count == name_index_count + 1)
   {
      ret = inner_names_match(name, match_internal_name);
   }
   else if (unif_index_count == name_index_count)
   {
      // Both names have the same number of array indices.
      // Strip the last index from the passed name and compare with our internal
      // uniform. This effectively zeros the last index during the compare so it
      // will match the internal name correctly.
      GLchar *stripped = strdup(name);
      if (stripped != NULL)
      {
         GLchar *s = stripped + strlen(stripped) - 1;
         while (s > stripped && *s != '[')
            s--;
         *s = '\0';

         ret = inner_names_match(stripped, match_internal_name);
         free(stripped);
      }
   }

end:
   if (array_offset == NULL && off != 0)
      ret = false;

end2:
   if (extended != NULL)
      free(extended);

   return ret;
}

static int get_inout_location(GLSL_INOUT_T *vars, unsigned n_vars, const char *name) {
   for (unsigned i = 0; i < n_vars; i++) {
      GLSL_INOUT_T *v = &vars[i];
      int           off;

      if (names_match(name, v->name, v->is_array ? ARRAY : NOT_ARRAY, &off) &&
                      off < (int)v->array_size)
         return v->index + off;
   }

   return -1;
}

static GLint get_attribute_location(GLSL_PROGRAM_T *p, const GLchar *name)
{
   return get_inout_location(p->attributes, p->num_attributes, name);
}

static GLint get_frag_data_location(GLSL_PROGRAM_T *p, const GLchar *name)
{
   return get_inout_location(p->frag_out, p->num_frag_outputs, name);
}

static GLint get_uniform_location(GLSL_PROGRAM_T *p, const GLchar *name)
{
   for (unsigned i = 0; i < p->default_uniforms.num_members; i++)
   {
      GLSL_BLOCK_MEMBER_T *info = &p->default_uniforms.members[i];
      int offset;

      if (names_match(name, info->name, INTERNAL_NAME_IMPLIES_ARRAY, &offset) &&
                      offset < (int)info->array_length)
         return glxx_encode_location(info->offset, offset);
   }

   return -1;
}

static GLSL_BLOCK_MEMBER_T *get_named_block_var(GLSL_BLOCK_T *blocks, int num_blocks, const char *name, unsigned *index)
{
   *index = 0;

   for (int b = 0; b < num_blocks; b++)
   {
      for (unsigned u = 0; u < blocks[b].num_members; u++)
      {
         GLSL_BLOCK_MEMBER_T *unif = &blocks[b].members[u];
         int offset;
         if (names_match(name, unif->name, INTERNAL_NAME_IMPLIES_ARRAY, &offset))
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

static GLSL_BLOCK_MEMBER_T *get_named_uniform(GL20_PROGRAM_T *prog, const GLchar *name, GLuint *index)
{
   GLSL_PROGRAM_T *p = prog->linked_glsl_program;
   GLSL_BLOCK_MEMBER_T *ret = get_named_block_var(&p->default_uniforms, 1, name, index);
   if (ret) return ret;

   ret = get_named_block_var(p->uniform_blocks, p->num_uniform_blocks, name, index);
   if (ret) {
      *index += p->default_uniforms.num_members;
      return ret;
   }

   *index = GL_INVALID_INDEX;
   return NULL;
}

static GLSL_BLOCK_MEMBER_T *get_named_buffer_var(GL20_PROGRAM_T *prog, const GLchar *name, GLuint *index)
{
   GLSL_PROGRAM_T *p = prog->linked_glsl_program;
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

static int get_active_resources(GLSL_PROGRAM_T *p, GLenum programInterface)
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
      return p->num_attributes;
   case GL_PROGRAM_OUTPUT:
      return p->num_frag_outputs;
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

static GLenum get_max_name_length(GLSL_PROGRAM_T *p, GLenum programInterface, GLint *len)
{
   unsigned i, j;

   *len = 0;

   switch (programInterface)
   {
   case GL_UNIFORM:
      for (i = 0; i < p->default_uniforms.num_members; i++)
         *len = vcos_max(*len, (GLint)strlen(p->default_uniforms.members[i].name));

      for (j = 0; j < p->num_uniform_blocks; j++)
         for (i = 0; i < p->uniform_blocks[j].num_members; i++)
            *len = vcos_max(*len, (GLint)strlen(p->uniform_blocks[j].members[i].name));
      break;
   case GL_UNIFORM_BLOCK:
      for (i = 0; i < p->num_uniform_blocks; i++)
      {
         GLSL_BLOCK_T *block = &p->uniform_blocks[i];
         GLint l = strlen(block->name);
         if (block->is_array)
            l += 3 + (GLint)log10(block->array_length);
         *len = vcos_max(*len, l);
      }
      break;
   case GL_SHADER_STORAGE_BLOCK:
      for (i = 0; i < p->num_buffer_blocks; i++)
      {
         GLSL_BLOCK_T *block = &p->buffer_blocks[i];
         GLint l = strlen(block->name);
         if (block->is_array)
            l += 3 + (GLint)log10(block->array_length);
         *len = vcos_max(*len, l);
      }
      break;
   case GL_PROGRAM_INPUT:
      for (i = 0; i < p->num_attributes; i++)
         *len = vcos_max(*len, (GLint)strlen(p->attributes[i].name));
      break;
   case GL_PROGRAM_OUTPUT:
      for (i = 0; i < p->num_frag_outputs; i++)
      {
         GLSL_INOUT_T *frag = &p->frag_out[i];

         GLint l = strlen(frag->name);
         if (frag->is_array)
            l += 3 + (GLint)log10(frag->array_size);
         *len = vcos_max(*len, l);
      }
      break;
   case GL_TRANSFORM_FEEDBACK_VARYING:
      for (i = 0; i < p->num_tf_captures; i++)
         *len = vcos_max(*len, (GLint)strlen(p->tf_capture[i].name));
      break;
   case GL_BUFFER_VARIABLE:
      for (j = 0; j < p->num_buffer_blocks; j++)
         for (i = 0; i < p->buffer_blocks[j].num_members; i++)
            *len = vcos_max(*len, (int)strlen(p->buffer_blocks[j].members[i].name));
      break;
   default:
      return GL_INVALID_OPERATION;
   }

   if (*len > 0)
      *len += 1;  // One extra for the null terminator, which strlen doesn't count

   return GL_NO_ERROR;
}

static int get_atomic_num_active_vars(GLSL_PROGRAM_T *p, unsigned index)
{
   unsigned count = 0;
   for (unsigned i=0; i<p->default_uniforms.num_members; i++)
   {
      if (p->default_uniforms.members[i].atomic_idx == index)
         count++;
   }
   return count;
}

static GLenum get_max_num_active_variables(GL20_PROGRAM_T *prog, GLenum programInterface, GLint *len)
{
   *len = 0;

   switch (programInterface)
   {
   case GL_UNIFORM_BLOCK:
      for (unsigned i = 0; i < prog->linked_glsl_program->num_uniform_blocks; i++)
         *len = vcos_max(*len, (GLint)prog->linked_glsl_program->uniform_blocks[i].num_members);
      break;
   case GL_SHADER_STORAGE_BLOCK:
      for (unsigned i = 0; i < prog->linked_glsl_program->num_buffer_blocks; i++)
         *len = vcos_max(*len, (GLint)prog->linked_glsl_program->buffer_blocks[i].num_members);
      break;
   case GL_ATOMIC_COUNTER_BUFFER:
      for (unsigned i=0; i<prog->linked_glsl_program->num_atomic_buffers; i++)
         *len = vcos_max(*len, get_atomic_num_active_vars(prog->linked_glsl_program, i));
      break;
   default:
      return GL_INVALID_OPERATION;
   }

   return GL_NO_ERROR;
}

static GLuint get_program_resource_index(GL20_PROGRAM_T *prog, GLenum programInterface, const GLchar *name)
{
   GLuint i = GL_INVALID_INDEX;

   switch (programInterface)
   {
   case GL_UNIFORM:
      if (get_named_uniform(prog, name, &i) != NULL)
         return i;
      break;
   case GL_UNIFORM_BLOCK:
      for (unsigned i = 0; i < prog->linked_glsl_program->num_uniform_blocks; i++)
      {
         GLSL_BLOCK_T *block = &prog->linked_glsl_program->uniform_blocks[i];
         int           offset;

         if (names_match(name, block->name, block->is_array ? ARRAY : NOT_ARRAY, &offset))
         {
            if (offset < block->array_length)
               return block->index + offset;
         }
      }
      break;
   case GL_SHADER_STORAGE_BLOCK:
      for (unsigned i = 0; i < prog->linked_glsl_program->num_buffer_blocks; i++)
      {
         GLSL_BLOCK_T *block = &prog->linked_glsl_program->buffer_blocks[i];
         int           offset;

         if (names_match(name, block->name, block->is_array ? ARRAY : NOT_ARRAY, &offset))
         {
            if (offset < block->array_length)
               return block->index + offset;
         }
      }
      break;
   case GL_PROGRAM_INPUT:
      for (i = 0; i < prog->linked_glsl_program->num_attributes; i++)
         if (names_match(name, prog->linked_glsl_program->attributes[i].name, NOT_ARRAY, NULL))
            return i;
      break;
   case GL_PROGRAM_OUTPUT:
      for (i = 0; i < prog->linked_glsl_program->num_frag_outputs; i++)
      {
         GLSL_INOUT_T *out = &prog->linked_glsl_program->frag_out[i];
         if (names_match(name, out->name, out->is_array ? ARRAY : NOT_ARRAY, NULL))
            return i;
      }
      break;
   case GL_TRANSFORM_FEEDBACK_VARYING:
      for (i = 0; i < prog->linked_glsl_program->num_tf_captures; i++)
         if (names_match(name, prog->linked_glsl_program->tf_capture[i].name, INTERNAL_NAME_IMPLIES_ARRAY, NULL))
            return i;
      break;
   case GL_BUFFER_VARIABLE:
      if (get_named_buffer_var(prog, name, &i) != NULL)
         return i;
      break;
   default:
      unreachable();
   }

   return GL_INVALID_INDEX;
}

static bool index_valid_for_interface(GLSL_PROGRAM_T *p, GLenum program_interface, unsigned index)
{
   switch (program_interface)
   {
   case GL_UNIFORM:
      return (get_indexed_uniform(p, index, NULL) != NULL);
   case GL_UNIFORM_BLOCK:
      return (gl20_get_ubo_from_index(p, index) != NULL);
   case GL_SHADER_STORAGE_BLOCK:
      return (gl20_get_ssbo_from_index(p, index) != NULL);
   case GL_PROGRAM_INPUT:
      return (index < p->num_attributes);
   case GL_PROGRAM_OUTPUT:
      return (index < p->num_frag_outputs);
   case GL_TRANSFORM_FEEDBACK_VARYING:
      return (index < p->num_tf_captures);
   case GL_BUFFER_VARIABLE:
      return (get_indexed_buffer(p, index, NULL) != NULL);
   case GL_ATOMIC_COUNTER_BUFFER:
      return (index < p->num_atomic_buffers);
   default:
      unreachable();
      return false;
   }
}

static void get_program_resource_name(GL20_PROGRAM_T *prog, GLenum programInterface, GLuint index,
                                      GLsizei bufSize, GLsizei *length, GLchar *name)
{
   switch (programInterface)
   {
   case GL_UNIFORM:
      {
         GLSL_BLOCK_MEMBER_T *uniform = get_indexed_uniform(prog->linked_glsl_program, index, NULL);
         strncpy(name, uniform->name, bufSize);
      }
      break;
   case GL_UNIFORM_BLOCK:
      {
         GLSL_BLOCK_T *block = gl20_get_ubo_from_index(prog->linked_glsl_program, index);
         if (block->is_array)
            strzncpy_with_array(name, block->name, bufSize, index - block->index);
         else
            strzncpy(name, block->name, bufSize);
      }
      break;
   case GL_SHADER_STORAGE_BLOCK:
      {
         GLSL_BLOCK_T *block = gl20_get_ssbo_from_index(prog->linked_glsl_program, index);
         if (block->is_array)
            strzncpy_with_array(name, block->name, bufSize, index - block->index);
         else
            strzncpy(name, block->name, bufSize);
      }
      break;
   case GL_PROGRAM_INPUT:
      strncpy(name, prog->linked_glsl_program->attributes[index].name, bufSize);
      break;
   case GL_PROGRAM_OUTPUT:
      {
         GLSL_INOUT_T *frag = &prog->linked_glsl_program->frag_out[index];
         if (frag->is_array)
            strzncpy_with_array(name, frag->name, bufSize, 0);
         else
            strzncpy(name, frag->name, bufSize);
      }
      break;
   case GL_TRANSFORM_FEEDBACK_VARYING:
      strncpy(name, prog->linked_glsl_program->tf_capture[index].name, bufSize);
      break;
   case GL_BUFFER_VARIABLE:
      {
         GLSL_BLOCK_MEMBER_T *buffer = get_indexed_buffer(prog->linked_glsl_program, index, NULL);
         strncpy(name, buffer->name, bufSize);
      }
      break;
   default:
      unreachable();
   }

   if (bufSize > 0)
      name[bufSize - 1] = '\0';

   if (length != NULL)
      *length = strlen(name);
}

static GLint get_program_resource_location(GLSL_PROGRAM_T *p, GLenum programInterface, const GLchar *name)
{
   switch (programInterface)
   {
   case GL_UNIFORM:
      return get_uniform_location(p, name);
   case GL_PROGRAM_INPUT:
      return get_attribute_location(p, name);
   case GL_PROGRAM_OUTPUT:
      return get_frag_data_location(p, name);
   default:
      unreachable();
   }

   return -1;
}

// Generated from Table 7.2 in ES3.1 spec (April 29, 2015)
static GLenum valid_uniform_props[] =
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
   GL_TYPE
};

static GLenum valid_uniform_block_props[] =
{
   GL_NAME_LENGTH,
   GL_ACTIVE_VARIABLES,
   GL_BUFFER_BINDING,
   GL_NUM_ACTIVE_VARIABLES,
   GL_BUFFER_DATA_SIZE,
   GL_REFERENCED_BY_VERTEX_SHADER,
   GL_REFERENCED_BY_FRAGMENT_SHADER,
   GL_REFERENCED_BY_COMPUTE_SHADER
};

static GLenum valid_atomic_props[] =
{
   GL_ACTIVE_VARIABLES,
   GL_BUFFER_BINDING,
   GL_NUM_ACTIVE_VARIABLES,
   GL_BUFFER_DATA_SIZE,
   GL_REFERENCED_BY_VERTEX_SHADER,
   GL_REFERENCED_BY_FRAGMENT_SHADER,
   GL_REFERENCED_BY_COMPUTE_SHADER
};

static GLenum valid_input_props[] =
{
   GL_NAME_LENGTH,
   GL_ARRAY_SIZE,
   GL_LOCATION,
   GL_REFERENCED_BY_VERTEX_SHADER,
   GL_REFERENCED_BY_FRAGMENT_SHADER,
   GL_REFERENCED_BY_COMPUTE_SHADER,
   GL_TYPE
};

static GLenum valid_output_props[] =
{
   GL_NAME_LENGTH,
   GL_ARRAY_SIZE,
   GL_LOCATION,
   GL_REFERENCED_BY_VERTEX_SHADER,
   GL_REFERENCED_BY_FRAGMENT_SHADER,
   GL_REFERENCED_BY_COMPUTE_SHADER,
   GL_TYPE
};

static GLenum valid_tfv_props[] =
{
   GL_NAME_LENGTH,
   GL_ARRAY_SIZE,
   GL_TYPE
};

static GLenum valid_buffer_var_props[] =
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
   GL_TYPE
};

static GLenum valid_ssb_props[] =
{
   GL_NAME_LENGTH,
   GL_ACTIVE_VARIABLES,
   GL_BUFFER_BINDING,
   GL_NUM_ACTIVE_VARIABLES,
   GL_BUFFER_DATA_SIZE,
   GL_REFERENCED_BY_VERTEX_SHADER,
   GL_REFERENCED_BY_FRAGMENT_SHADER,
   GL_REFERENCED_BY_COMPUTE_SHADER
};

#define USE_ARRAY(a)\
   propsArray = (a);\
   num = sizeof(a) / sizeof(GLenum);

static GLenum valid_props_combination(GLenum programInterface, GLenum prop)
{
   GLenum   *propsArray = NULL;
   unsigned num;

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
      break;
   default:
      return GL_INVALID_ENUM;
   }

   switch (programInterface)
   {
   case GL_UNIFORM:                     USE_ARRAY(valid_uniform_props); break;
   case GL_UNIFORM_BLOCK:               USE_ARRAY(valid_uniform_block_props); break;
   case GL_ATOMIC_COUNTER_BUFFER:       USE_ARRAY(valid_atomic_props); break;
   case GL_PROGRAM_INPUT:               USE_ARRAY(valid_input_props); break;
   case GL_PROGRAM_OUTPUT:              USE_ARRAY(valid_output_props); break;
   case GL_TRANSFORM_FEEDBACK_VARYING:  USE_ARRAY(valid_tfv_props); break;
   case GL_BUFFER_VARIABLE:             USE_ARRAY(valid_buffer_var_props); break;
   case GL_SHADER_STORAGE_BLOCK:        USE_ARRAY(valid_ssb_props); break;
   default:
      return GL_INVALID_ENUM;
   }

   for (unsigned i = 0; i < num; i++)
   {
      if (propsArray[i] == prop)
         return GL_NO_ERROR;
   }

   // Must be an invalid combination
   return GL_INVALID_OPERATION;
}

// Returns how many entries it wrote into params. Must never write more than 'space' entries.
static GLint get_uniform_resource_prop(GL20_PROGRAM_T *prog, GLuint index,
                                       GLenum prop, GLint *params, GLint space)
{
   GLint block_index = -1;
   assert(space > 0);

   GLSL_BLOCK_MEMBER_T *uniform = get_indexed_uniform(prog->linked_glsl_program, index, &block_index);
   if (uniform == NULL)
      return 0;

   switch (prop)
   {
   case GL_NAME_LENGTH:
      params[0] = strlen(uniform->name) + 1;
      break;
   case GL_ARRAY_SIZE:
      params[0] = uniform->array_length;
      break;
   case GL_ARRAY_STRIDE:
      params[0] = uniform->array_stride;
      break;
   case GL_BLOCK_INDEX:
      params[0] = block_index;
      break;
   case GL_IS_ROW_MAJOR:
      params[0] = uniform->column_major ? 0 : 1;
      break;
   case GL_MATRIX_STRIDE:
      params[0] = uniform->matrix_stride;
      break;
   case GL_ATOMIC_COUNTER_BUFFER_INDEX:
      params[0] = uniform->atomic_idx;
      break;
   case GL_LOCATION:
      if (block_index == -1 && uniform->atomic_idx == -1)
         params[0] = glxx_encode_location(uniform->offset, 0);
      else
         params[0] = -1;
      break;
   case GL_OFFSET:
      if (block_index == -1 && uniform->atomic_idx == -1)
         params[0] = -1;
      else
         params[0] = uniform->offset;
      break;
   case GL_REFERENCED_BY_VERTEX_SHADER:
      params[0] = uniform->used_in_vs ? 1 : 0;
      break;
   case GL_REFERENCED_BY_FRAGMENT_SHADER:
      params[0] = uniform->used_in_fs ? 1 : 0;
      break;
   case GL_REFERENCED_BY_COMPUTE_SHADER:
      params[0] = uniform->used_in_cs ? 1 : 0;
      break;
   case GL_TYPE:
      params[0] = uniform->type;
      break;
   default:
      unreachable();
   }

   return 1;   // All the props above will have filled in one entry in params
}

// Common properties of UBO and SSBOs.
static void get_block_resource_prop(const GLSL_BLOCK_T *block, GLenum prop, int *params)
{
   switch (prop)
   {
   case GL_NAME_LENGTH:
      params[0] = strlen(block->name) + 1;
      if (block->is_array)
         params[0] += 3 + (int)log10(block->array_length);
      break;
   case GL_NUM_ACTIVE_VARIABLES:
      params[0] = block->num_members;
      break;
   case GL_BUFFER_DATA_SIZE:
      params[0] = block->size;
      break;
   case GL_REFERENCED_BY_VERTEX_SHADER:
      params[0] = block->used_in_vs ? 1 : 0;
      break;
   case GL_REFERENCED_BY_FRAGMENT_SHADER:
      params[0] = block->used_in_fs ? 1 : 0;
      break;
   case GL_REFERENCED_BY_COMPUTE_SHADER:
      params[0] = block->used_in_cs ? 1 : 0;
      break;
   default:
      unreachable();
   }
}

// Returns how many entries it wrote into params. Must never write more than 'space' entries.
static unsigned get_uniform_block_resource_prop(GL20_PROGRAM_T *prog, unsigned index,
                                                GLenum prop, int *params, unsigned space)
{
   const GLSL_BLOCK_T *block = gl20_get_ubo_from_index(prog->linked_glsl_program, index);
   assert(block != NULL);

   switch (prop)
   {
   case GL_ACTIVE_VARIABLES:
      {
         GLSL_BLOCK_T *base = prog->linked_glsl_program->uniform_blocks;
         unsigned      uniform_count = block->num_members;
         unsigned      uniform_index;

         /* Loop over previous blocks */
         uniform_index = prog->linked_glsl_program->default_uniforms.num_members;
         for (unsigned j = 0; base[j].index < block->index; j++)
            uniform_index += base[j].num_members;

         if (space < uniform_count)
            uniform_count = space;

         /* Write out the index array */
         for (unsigned j = 0; j < uniform_count; j++)
            params[j] = uniform_index++;

         return uniform_count;
      }
   case GL_BUFFER_BINDING:
      params[0] = prog->ubo_binding_point[index];
      break;
   default:
      get_block_resource_prop(block, prop, params);
      break;
   }

   return 1;
}

// Returns how many entries it wrote into params. Must never write more than 'space' entries.
static GLint get_prog_input_resource_prop(GL20_PROGRAM_T *prog, GLuint index,
                                          GLenum prop, GLint *params, GLint space)
{
   assert(space > 0);

   GLSL_INOUT_T *attrib = &prog->linked_glsl_program->attributes[index];

   switch (prop)
   {
   case GL_NAME_LENGTH:
      params[0] = strlen(attrib->name) + 1;
      break;
   case GL_ARRAY_SIZE:
      params[0] = 1;   // Attributes cannot be arrays
      break;
   case GL_LOCATION:
      params[0] = get_attribute_location(prog->linked_glsl_program, attrib->name);
      break;
   case GL_REFERENCED_BY_VERTEX_SHADER:
      params[0] = 1; // Must be referenced by vertex shader
      break;
   case GL_REFERENCED_BY_FRAGMENT_SHADER:
      params[0] = 0; // Input resources cannot be referenced by the fragment shader
      break;
   case GL_REFERENCED_BY_COMPUTE_SHADER:
      NOT_YET_IMPLEMENTED_TODO;
      params[0] = 0;
      break;
   case GL_TYPE:
      params[0] = attrib->type;
      break;
   default:
      unreachable();
   }

   return 1;
}

// Returns how many entries it wrote into params. Must never write more than 'space' entries.
static GLint get_prog_output_resource_prop(GL20_PROGRAM_T *prog, GLuint index,
                                           GLenum prop, GLint *params, GLint space)
{
   assert(space > 0);

   GLSL_INOUT_T *output = &prog->linked_glsl_program->frag_out[index];

   switch (prop)
   {
   case GL_NAME_LENGTH:
      params[0] = strlen(output->name) + 1;
      if (output->is_array)
         params[0] += 3 + (GLint)log10(output->array_size);
      break;
   case GL_ARRAY_SIZE:
      params[0] = output->array_size;
      break;
   case GL_LOCATION:
      params[0] = get_frag_data_location(prog->linked_glsl_program, output->name);
      break;
   case GL_REFERENCED_BY_VERTEX_SHADER:
      params[0] = 0; // Output resources cannot be referenced by the vertex shader
      break;
   case GL_REFERENCED_BY_FRAGMENT_SHADER:
      params[0] = 1; // Must be referenced by fragment shader
      break;
   case GL_REFERENCED_BY_COMPUTE_SHADER:
      params[0] = 0; // Output resources cannot be referenced by the compute shader
      break;
   case GL_TYPE:
      params[0] = output->type;
      break;
   default:
      unreachable();
   }

   return 1;
}

// Returns how many entries it wrote into params. Must never write more than 'space' entries.
static GLint get_atomic_counter_buffer_resource_prop(GLSL_PROGRAM_T *p, GLuint index,
                                                     GLenum prop, GLint *params, GLint space)
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
   default:
      unreachable();
      return 0;
   }
}

// Returns how many entries it wrote into params. Must never write more than 'space' entries.
static GLint get_transform_feedback_resource_prop(GLSL_PROGRAM_T *p, GLuint index,
                                                  GLenum prop, GLint *params, GLint space)
{
   assert(space > 0);

   const GLSL_TF_CAPTURE_T *tff = &p->tf_capture[index];

   switch (prop)
   {
   case GL_NAME_LENGTH:
      params[0] = strlen(tff->name) + 1;
      break;
   case GL_ARRAY_SIZE:
      params[0] = tff->array_length;
      break;
   case GL_TYPE:
      params[0] = tff->type;
      break;
   default:
      unreachable();
   }

   return 1;
}

// Returns how many entries it wrote into params. Must never write more than 'space' entries.
static GLint get_buffer_variable_resource_prop(GLSL_PROGRAM_T *p, GLuint index,
                                               GLenum prop, GLint *params, GLint space)
{
   GLint block_index = -1;
   assert(space > 0);

   const GLSL_BLOCK_MEMBER_T *v = get_indexed_buffer(p, index, &block_index);
   if (v == NULL)
      return 0;

   switch (prop)
   {
   case GL_NAME_LENGTH:
      params[0] = strlen(v->name) + 1;
      break;
   case GL_ARRAY_SIZE:
      params[0] = v->array_length;
      break;
   case GL_ARRAY_STRIDE:
      params[0] = v->array_stride;
      break;
   case GL_BLOCK_INDEX:
      params[0] = block_index;
      break;
   case GL_IS_ROW_MAJOR:
      params[0] = v->column_major ? 0 : 1;
      break;
   case GL_MATRIX_STRIDE:
      params[0] = v->matrix_stride;
      break;
   case GL_OFFSET:
      params[0] = v->offset;
      break;
   case GL_REFERENCED_BY_VERTEX_SHADER:
      params[0] = v->used_in_vs ? 1 : 0;
      break;
   case GL_REFERENCED_BY_FRAGMENT_SHADER:
      params[0] = v->used_in_fs ? 1 : 0;
      break;
   case GL_REFERENCED_BY_COMPUTE_SHADER:
      params[0] = v->used_in_cs ? 1 : 0;
      break;
   case GL_TOP_LEVEL_ARRAY_SIZE:
      params[0] = v->top_level_size;
      break;
   case GL_TOP_LEVEL_ARRAY_STRIDE:
      params[0] = v->top_level_stride;
      break;
   case GL_TYPE:
      params[0] = v->type;
      break;
   default:
      unreachable();
   }

   return 1;   // All the props above will have filled in one entry in params
}

// Returns how many entries it wrote into params. Must never write more than 'space' entries.
static unsigned get_shader_storage_block_resource_prop(GL20_PROGRAM_T *prog, unsigned index,
                                                       GLenum prop, int *params, unsigned space)
{
   const GLSL_BLOCK_T *block = gl20_get_ssbo_from_index(prog->linked_glsl_program, index);
   assert(block != NULL);

   switch (prop)
   {
   case GL_ACTIVE_VARIABLES:
      {
         GLSL_BLOCK_T *base = prog->linked_glsl_program->buffer_blocks;
         unsigned      count = block->num_members;
         unsigned      index = 0;

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
   case GL_BUFFER_BINDING:
      params[0] = prog->ssbo_binding_point[index];
      break;
   default:
      get_block_resource_prop(block, prop, params);
      break;
   }

   return 1;
}

// Returns how many entries it wrote into params. Must never write more than 'space' entries.
static unsigned get_program_resource_prop(GL20_PROGRAM_T *program, GLenum programInterface, unsigned index,
                                          GLenum prop, int *params, unsigned space)
{
   unsigned added = 0;

   switch (programInterface)
   {
   case GL_UNIFORM:
      added = get_uniform_resource_prop(program, index, prop, params, space);
      break;
   case GL_UNIFORM_BLOCK:
      added = get_uniform_block_resource_prop(program, index, prop, params, space);
      break;
   case GL_PROGRAM_INPUT:
      added = get_prog_input_resource_prop(program, index, prop, params, space);
      break;
   case GL_PROGRAM_OUTPUT:
      added = get_prog_output_resource_prop(program, index, prop, params, space);
      break;
   case GL_ATOMIC_COUNTER_BUFFER:
      added = get_atomic_counter_buffer_resource_prop(program->linked_glsl_program, index, prop, params, space);
      break;
   case GL_TRANSFORM_FEEDBACK_VARYING:
      added = get_transform_feedback_resource_prop(program->linked_glsl_program, index, prop, params, space);
      break;
   case GL_BUFFER_VARIABLE:
      added = get_buffer_variable_resource_prop(program->linked_glsl_program, index, prop, params, space);
      break;
   case GL_SHADER_STORAGE_BLOCK:
      added = get_shader_storage_block_resource_prop(program, index, prop, params, space);
      break;
   default:
      unreachable();
   }

   assert(added <= space);
   return added;
}

GL_APICALL void GL_APIENTRY glGetProgramInterfaceiv(GLuint p, GLenum programInterface, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state;
   GLenum error = GL_NO_ERROR;

   state = GL31_LOCK_SERVER_STATE_UNCHANGED();
   if (!state)
      return;

   if (!is_program_interface(programInterface))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   GL20_PROGRAM_T *program = gl20_get_program(state, p);  // takes care of setting correct error
   if (!program)
      goto end;

   if (program->linked_glsl_program == NULL)
   {
      // Failed or unlinked programs can be assumed to have no resources
      params[0] = 0;
      goto end;
   }

   switch (pname)
   {
   case GL_ACTIVE_RESOURCES:
      if (params)
         params[0] = get_active_resources(program->linked_glsl_program, programInterface);
      break;
   case GL_MAX_NAME_LENGTH:
      error = get_max_name_length(program->linked_glsl_program, programInterface, params);
      if (error != GL_NO_ERROR)
         goto end;
      break;
   case GL_MAX_NUM_ACTIVE_VARIABLES:
      error = get_max_num_active_variables(program, programInterface, params);
      if (error != GL_NO_ERROR)
         goto end;
      break;
   default:
      error = GL_INVALID_ENUM;
      goto end;
   }

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   GL31_UNLOCK_SERVER_STATE();
}

GLuint glxx_get_program_resource_index(GLXX_SERVER_STATE_T *state,
                                       GLuint p, GLenum programInterface, const GLchar *name)
{
   GLenum error = GL_NO_ERROR;
   GLuint indx  = GL_INVALID_INDEX;

   if (!is_program_interface(programInterface) || programInterface == GL_ATOMIC_COUNTER_BUFFER)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   GL20_PROGRAM_T *program = gl20_get_program(state, p);  // takes care of setting correct error
   if (!program)
      goto end;

   if (program->linked_glsl_program == NULL)
   {
      // Failed or unlinked programs can be assumed to have no resources
      goto end;
   }

   if (name == NULL || name[0] == '\0')
      goto end;

   indx = get_program_resource_index(program, programInterface, name);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   return indx;
}

GL_APICALL GLuint GL_APIENTRY glGetProgramResourceIndex(GLuint p, GLenum programInterface, const GLchar *name)
{
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE_UNCHANGED();
   GLuint              ret = GL_INVALID_INDEX;

   if (!state)
      return ret;

   ret = glxx_get_program_resource_index(state, p, programInterface, name);

   GL31_UNLOCK_SERVER_STATE();

   return ret;
}

void glxx_get_program_resource_name(GLXX_SERVER_STATE_T *state,
                                    GLuint p, GLenum programInterface, GLuint index, GLsizei bufSize,
                                    GLsizei *length, GLchar *name)
{
   GLenum error = GL_NO_ERROR;

   if (!is_program_interface(programInterface) || programInterface == GL_ATOMIC_COUNTER_BUFFER)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (bufSize < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   GL20_PROGRAM_T *program = gl20_get_program(state, p);  // takes care of setting correct error
   if (!program)
      goto end;

   if (program->linked_glsl_program == NULL)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (!index_valid_for_interface(program->linked_glsl_program, programInterface, index))
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   get_program_resource_name(program, programInterface, index, bufSize, length, name);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
}

GL_APICALL void GL_APIENTRY glGetProgramResourceName(GLuint p, GLenum programInterface, GLuint index, GLsizei bufSize,
                                                     GLsizei *length, GLchar *name)
{
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE_UNCHANGED();
   if (!state)
      return;

   glxx_get_program_resource_name(state, p, programInterface, index, bufSize, length, name);

   GL31_UNLOCK_SERVER_STATE();
}

bool glxx_get_program_resourceiv(GLXX_SERVER_STATE_T *state,
                                 GLuint p, GLenum programInterface, GLuint index, GLsizei propCount,
                                 const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params)
{
   GLenum error = GL_NO_ERROR;

   if (!is_program_interface(programInterface))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (propCount <= 0 || bufSize < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   GL20_PROGRAM_T *program = gl20_get_program(state, p);  // takes care of setting correct error
   if (!program)
      goto end;

   if (program->linked_glsl_program == NULL)
   {
      // Failed or unlinked programs can be assumed to have no resources
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (!index_valid_for_interface(program->linked_glsl_program, programInterface, index))
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   // Check for more argument error conditions
   for (GLsizei p = 0; p < propCount; p++)
   {
      error = valid_props_combination(programInterface, props[p]);
      if (error != GL_NO_ERROR)
         goto end;
   }

   unsigned space = bufSize;

   for (GLsizei p = 0; p < propCount && space > 0; p++)
   {
      unsigned added = get_program_resource_prop(program, programInterface, index, props[p], params, space);
      space = space - added;
      params += added;
   }

   if (length)
      *length = bufSize - space;

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   return error == GL_NO_ERROR;
}

GL_APICALL void GL_APIENTRY glGetProgramResourceiv(GLuint p, GLenum programInterface, GLuint index, GLsizei propCount,
                                                   const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params)
{
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE_UNCHANGED();
   if (!state)
      return;

   glxx_get_program_resourceiv(state, p, programInterface, index, propCount, props, bufSize, length, params);

   GL31_UNLOCK_SERVER_STATE();
}

GLint glxx_get_program_resource_location(GLXX_SERVER_STATE_T *state,
                                         GLuint p, GLenum programInterface, const GLchar *name)
{
   GLenum error = GL_NO_ERROR;
   GLint  location = -1;

   switch (programInterface)
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

   if (program->linked_glsl_program == NULL)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   if (name == NULL || name[0] == '\0')
      goto end;

   location = get_program_resource_location(program->linked_glsl_program, programInterface, name);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   return location;
}

GL_APICALL GLint GL_APIENTRY glGetProgramResourceLocation(GLuint p, GLenum programInterface, const GLchar *name)
{
   GLXX_SERVER_STATE_T *state;
   GLint  location = -1;

   state = GL31_LOCK_SERVER_STATE_UNCHANGED();
   if (!state)
      return location;

   location = glxx_get_program_resource_location(state, p, programInterface, name);

   GL31_UNLOCK_SERVER_STATE();

   return location;
}
