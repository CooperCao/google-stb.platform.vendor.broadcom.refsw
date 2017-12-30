/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/glxx/glxx_shared.h"

#include "middleware/khronos/gl20/gl20_server.h"
#include "middleware/khronos/glxx/glxx_server_internal.h"

#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/gl20/gl20_shader.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"
#include "interface/khronos/include/GLES2/gl2ext.h"

#include "middleware/khronos/glxx/glxx_texture.h"
#include "middleware/khronos/glxx/glxx_buffer.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "middleware/khronos/common/khrn_interlock.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/glxx/glxx_tweaker.h"

#include <string.h>
#include <math.h>
#include <limits.h>

static int get_uniform_internal(GLuint p, GLint location, const void *v, GLboolean is_float);
static const char *lock_shader_info_log(GL20_SHADER_T *shader);
static void unlock_shader_info_log(GL20_SHADER_T *shader);

bool gl20_server_state_init(GLXX_SERVER_STATE_T *state, uint32_t name, MEM_HANDLE_T shared)
{
   state->type = OPENGL_ES_20;

   //initialise common portions of state
   if(!glxx_server_state_init(state, name, shared))
      return false;

   //gl 2.0 specific parts

   glxx_tweaker_init(&state->tweak_state, true);

   assert(state->mh_program == MEM_HANDLE_INVALID);

   state->point_size = 1.0f;

   return true;
}

void glPointSize_impl_20 (GLfloat size) // S
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   size = clean_float(size);

   if (size > 0.0f)
      state->point_size = size;
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   GL20_UNLOCK_SERVER_STATE();
}

/*
   Get a program pointer from a program name. Optionally retrieve
   the handle to the memory block storing the program structure.
   Gives GL_INVALID_VALUE if no object exists with that name, or
   GL_INVALID_OPERATION if the object is actually a shader and
   returns NULL in either case.
*/

static GL20_PROGRAM_T *get_program(GLXX_SERVER_STATE_T *state, GLuint p, MEM_HANDLE_T *handle)
{
   GL20_PROGRAM_T *program;
   MEM_HANDLE_T phandle = glxx_shared_get_pobject((GLXX_SHARED_T *)mem_lock(state->mh_shared, NULL), p);
   mem_unlock(state->mh_shared);

   if (phandle == MEM_HANDLE_INVALID) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

      return NULL;
   }

   program = (GL20_PROGRAM_T *)mem_lock(phandle, NULL);

   assert(program);

   if (!gl20_is_program(program)) {
      assert(gl20_is_shader((GL20_SHADER_T *)program));

      glxx_server_state_set_error(state, GL_INVALID_OPERATION);

      mem_unlock(phandle);

      return NULL;
   }

   if (handle)
      *handle = phandle;

   return program;
}

/*
   Get a shader pointer from a shader name. Optionally retrieve
   the handle to the memory block storing the shader structure.
   Gives GL_INVALID_VALUE if no object exists with that name, or
   GL_INVALID_OPERATION if the object is actually a program and
   returns NULL in either case.
*/

static GL20_SHADER_T *get_shader(GLXX_SERVER_STATE_T *state, GLuint s, MEM_HANDLE_T *handle)
{
   GL20_SHADER_T *shader;
   MEM_HANDLE_T shandle = glxx_shared_get_pobject((GLXX_SHARED_T *)mem_lock(state->mh_shared, NULL), s);
   mem_unlock(state->mh_shared);

   if (shandle == MEM_HANDLE_INVALID) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

      return NULL;
   }

   shader = (GL20_SHADER_T *)mem_lock(shandle, NULL);

   assert(shader);

   if (!gl20_is_shader(shader)) {
      assert(gl20_is_program((GL20_PROGRAM_T *)shader));

      glxx_server_state_set_error(state, GL_INVALID_OPERATION);

      mem_unlock(shandle);

      return NULL;
   }

   if (handle)
      *handle = shandle;

   return shader;
}


/*
   glAttachShader()

   Attach a fragment or vertex shader to a program object. We make use of the
   ES restriction that a program may only have a single shader of each type
   attached. Gives GL_INVALID_VALUE error if either program or shader is not
   a valid 'program object', or GL_INVALID_OPERATION if both are valid but
   either is of the wrong type.

   Implementation: Done
   Error Checks: Done
*/

void glAttachShader_impl_20 (GLuint p, GLuint s)
{
   MEM_HANDLE_T phandle;
   GL20_PROGRAM_T *program;

   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   assert(state);

   program = get_program(state, p, &phandle);

   if (program) {
      MEM_HANDLE_T shandle;
      GL20_SHADER_T *shader = get_shader(state, s, &shandle);

      if (shader) {
         MEM_HANDLE_T *pmh_shader = NULL;

         switch (shader->type) {
         case GL_VERTEX_SHADER:
            pmh_shader = &program->mh_vertex;
            break;
         case GL_FRAGMENT_SHADER:
            pmh_shader = &program->mh_fragment;
            break;
         default:
            UNREACHABLE();
            break;
         }

         if (*pmh_shader != MEM_HANDLE_INVALID)
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         else {
            gl20_shader_acquire(shader);

            MEM_ASSIGN(*pmh_shader, shandle);
         }

         mem_unlock(shandle);
      }

      mem_unlock(phandle);
   }

   GL20_UNLOCK_SERVER_STATE();
}

void glBindAttribLocation_impl_20 (GLuint p, GLuint index, const char *name)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   if (!state) return;

   if (name)
   {
      if (index < GLXX_CONFIG_MAX_VERTEX_ATTRIBS)
         if (strncmp(name, "gl_", 3)) {
            MEM_HANDLE_T phandle;
            GL20_PROGRAM_T *program = get_program(state, p, &phandle);

            if (program) {
               if (!gl20_program_bind_attrib(program, index, name))
                  glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

               mem_unlock(phandle);
            }
         } else
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
   }

   GL20_UNLOCK_SERVER_STATE();
}





/*
   glBlendColor()

   Sets the constant color for use in blending. All inputs are clamped to the
   range [0.0, 1.0] before being stored. No errors are generated.

   Implementation: Done
   Error Checks: Done
*/

void glBlendColor_impl_20 (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) // S
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   state->blend_color[0] = clampf(red, 0.0f, 1.0f);
   state->blend_color[1] = clampf(green, 0.0f, 1.0f);
   state->blend_color[2] = clampf(blue, 0.0f, 1.0f);
   state->blend_color[3] = clampf(alpha, 0.0f, 1.0f);

   GL20_UNLOCK_SERVER_STATE();
}
/*
void glBlendEquation_impl_20 ( GLenum mode ) // S
{
   UNREACHABLE();
}
*/

/*
   Check if 'mode' is a valid blend equation enum.
*/

static GLboolean is_blend_equation(GLenum mode)
{
   return mode == GL_FUNC_ADD ||
          mode == GL_FUNC_SUBTRACT ||
          mode == GL_FUNC_REVERSE_SUBTRACT;
}

/*
   glBlendEquationSeparate()

   Sets the RGB and alpha blend equations to one of ADD, SUBTRACT or REVERSE_SUBTRACT.
   Gives GL_INVALID_ENUM error if either equation is not one of these.

   Implementation: Done
   Error Checks: Done
*/

void glBlendEquationSeparate_impl_20 (GLenum modeRGB, GLenum modeAlpha) // S
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   if (is_blend_equation(modeRGB) && is_blend_equation(modeAlpha)) {
      state->changed_backend = true;
      state->blend_equation.rgb = modeRGB;
      state->blend_equation.alpha = modeAlpha;
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   GL20_UNLOCK_SERVER_STATE();
}

/*
   glCreateProgram()

   Creates a new, empty program and returns its name. No errors are generated.

   Implementation: Done
   Error Checks: Done
*/

GLuint glCreateProgram_impl_20 (void)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   GLuint result = glxx_shared_create_program((GLXX_SHARED_T *)mem_lock(state->mh_shared, NULL));
   mem_unlock(state->mh_shared);

   if (result == 0)
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

   GL20_UNLOCK_SERVER_STATE();

   return result;
}

/*
   glCreateShader()

   Creates a new, empty shader and returns its name. Gives GL_INVALID_ENUM if
   type is not one of GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.

   Implementation: Done
   Error Checks: Done
*/

static bool is_shader_type(GLenum type)
{
   return type == GL_VERTEX_SHADER ||
          type == GL_FRAGMENT_SHADER;
}

GLuint glCreateShader_impl_20 (GLenum type)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   GLuint result = 0;

   if (is_shader_type(type)) {
      result = glxx_shared_create_shader((GLXX_SHARED_T *)mem_lock(state->mh_shared, NULL), type);
      mem_unlock(state->mh_shared);

      if (result == 0)
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   GL20_UNLOCK_SERVER_STATE();

   return result;
}

static void try_delete_shader(GLXX_SHARED_T *shared, GL20_SHADER_T *shader)
{
   if (shader->refs == 0 && shader->deleted)
      glxx_shared_delete_pobject(shared, shader->name);
}

static void release_shader(GLXX_SHARED_T *shared, MEM_HANDLE_T handle)
{
   if (handle != MEM_HANDLE_INVALID) {
      GL20_SHADER_T *shader;

      mem_acquire(handle);

      shader = (GL20_SHADER_T *)mem_lock(handle, NULL);

      assert(gl20_is_shader(shader));

      gl20_shader_release(shader);

      try_delete_shader(shared, shader);

      mem_unlock(handle);
      mem_release(handle);
   }
}

static void try_delete_program(GLXX_SHARED_T *shared, GL20_PROGRAM_T *program)
{
   if (program->refs == 0 && program->deleted) {
      release_shader(shared, program->mh_vertex);
      release_shader(shared, program->mh_fragment);

      glxx_shared_delete_pobject(shared, program->name);
   }
}

static void release_program(GLXX_SHARED_T *shared, MEM_HANDLE_T handle)
{
   if (handle != MEM_HANDLE_INVALID) {
      GL20_PROGRAM_T *program;

      mem_acquire(handle);

      program = (GL20_PROGRAM_T *)mem_lock(handle, NULL);

      assert(gl20_is_program(program));

      gl20_program_release(program);

      try_delete_program(shared, program);

      mem_unlock(handle);
      mem_release(handle);
   }
}

/*
   glDeleteProgram()

   Deletes a specified program. If the program is currently active in a
   context, the program is marked as pending deletion. Gives GL_INVALID_VALUE
   if the argument is neither a program nor a shader, or GL_INVALID_OPERATION
   if the argument is a shader.

   Implementation: Done
   Error Checks: Done
*/

void glDeleteProgram_impl_20 (GLuint p)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   GLXX_SHARED_T *shared = (GLXX_SHARED_T *)mem_lock(state->mh_shared, NULL);

   if (p) {
      MEM_HANDLE_T handle = glxx_shared_get_pobject(shared, p);

      if (handle != MEM_HANDLE_INVALID) {
         GL20_PROGRAM_T *program;

         /* wait to make sure noone is using the buffer */
         khrn_hw_common_wait();

         mem_acquire(handle);

         program = (GL20_PROGRAM_T *)mem_lock(handle, NULL);

         if (gl20_is_program(program)) {
            program->deleted = GL_TRUE;

            try_delete_program(shared, program);
         } else
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);

         mem_unlock(handle);
         mem_release(handle);
      } else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
   }

   mem_unlock(state->mh_shared);

   GL20_UNLOCK_SERVER_STATE();
}

/*
   glDeleteShader()

   Deletes a specified shader. If the shader is currently attached to a program,
   the shader is marked as pending deletion. Gives GL_INVALID_VALUE if the argument
   is neither a program nor a shader, or GL_INVALID_OPERATION if the argument is a
   program.

   Implementation: Done
   Error Checks: Done
*/

void glDeleteShader_impl_20 (GLuint s)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   GLXX_SHARED_T *shared = (GLXX_SHARED_T *)mem_lock(state->mh_shared, NULL);

   if (s) {
      MEM_HANDLE_T handle = glxx_shared_get_pobject(shared, s);

      if (handle != MEM_HANDLE_INVALID) {
         GL20_SHADER_T *shader;

         mem_acquire(handle);

         shader = (GL20_SHADER_T *)mem_lock(handle, NULL);

         if (gl20_is_shader(shader)) {
            shader->deleted = true;

            try_delete_shader(shared, shader);
         } else
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);

         mem_unlock(handle);
         mem_release(handle);
      } else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
   }

   mem_unlock(state->mh_shared);

   GL20_UNLOCK_SERVER_STATE();
}

/*
   glDetachShader()

   Detaches a shader from a program. If the shader is marked as pending deletion, and
   is not attached to another program, it is deleted. Gives GL_INVALID_VALUE if the
   program or shader does not exist, or GL_INVALID_OPERATION if the program argument
   is not a program, the shader argument is not a shader, or the shader is not attached
   to the program.

   Implementation: Done
   Error Checks: Done
*/

void glDetachShader_impl_20 (GLuint p, GLuint s)
{
   MEM_HANDLE_T phandle;
   GL20_PROGRAM_T *program;

   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   assert(state);

   program = get_program(state, p, &phandle);

   if (program) {
      MEM_HANDLE_T shandle;
      GL20_SHADER_T *shader = get_shader(state, s, &shandle);

      if (shader) {
         MEM_HANDLE_T *pmh_shader = NULL;

         switch (shader->type) {
         case GL_VERTEX_SHADER:
            pmh_shader = &program->mh_vertex;
            break;
         case GL_FRAGMENT_SHADER:
            pmh_shader = &program->mh_fragment;
            break;
         default:
            UNREACHABLE();
            break;
         }

         if (*pmh_shader != shandle)
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         else {
            gl20_shader_release(shader);

            try_delete_shader((GLXX_SHARED_T *)mem_lock(state->mh_shared, NULL), shader);
            mem_unlock(state->mh_shared);

            MEM_ASSIGN(*pmh_shader, MEM_HANDLE_INVALID);
         }

         mem_unlock(shandle);
      }

      mem_unlock(phandle);
   }

   GL20_UNLOCK_SERVER_STATE();
}



/*
void glDisableVertexAttribArray_impl_20 (GLuint index)
{
   UNREACHABLE();
}
*/





/*
void glEnableVertexAttribArray_impl_20 (GLuint index)
{
   UNREACHABLE();
}
*/


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

/*
   glGetActiveAttrib()

   Gets the name, size and type of a specified attribute of a program. Gives
   GL_INVALID_VALUE if the program does not exist, or GL_INVALID_OPERATION
   if the program argument is actually a shader. Also gives GL_INVALID_VALUE
   if the specified index is greater than the number of attributes of the
   linked program.

   Implementation: Done
   Error Checks: Done
*/

void glGetActiveAttrib_impl_20 (GLuint p, GLuint index, GLsizei buf_len, GLsizei *length, GLint *size, GLenum *type, char *buf_ptr)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   if (!state) return;

   MEM_HANDLE_T phandle;
   GL20_PROGRAM_T *program = get_program(state, p, &phandle);

   if (program == NULL) goto end;       /* get_shader will have set error */

   if (index >= program->num_attributes) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      mem_unlock(phandle);
      goto end;
   }

   if (buf_len < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      mem_unlock(phandle);
      goto end;
   }

   GL20_ATTRIB_INFO_T *base = program->attributes;

   size_t chars = strzncpy(buf_ptr, base[index].name, buf_len);

   if (length) *length = (GLsizei)chars;
   if (size) *size = 1;        // no array or structure attributes
   if (type) *type = base[index].type;

   mem_unlock(phandle);

end:
   GL20_UNLOCK_SERVER_STATE();
}

/*
   glGetActiveUniform()

   Gets the name, size and type of a specified uniform of a program. Gives
   GL_INVALID_VALUE if the program does not exist, or GL_INVALID_OPERATION
   if the program argument is actually a shader. Also gives GL_INVALID_VALUE
   if the specified index is greater than the number of uniforms of the
   linked program.

   Implementation: Done
   Error Checks: Done
*/

void glGetActiveUniform_impl_20 (GLuint p, GLuint index, GLsizei buf_len, GLsizei *length, GLint *size, GLenum *type, char *buf_ptr)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   if (!state) return;

   MEM_HANDLE_T phandle;
   GL20_PROGRAM_T *program = get_program(state, p, &phandle);

   if (program == NULL) goto end;

   if (index >= program->num_uniforms) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      mem_unlock(phandle);
      goto end;
   }

   GL20_UNIFORM_INFO_T *base = program->uniforms;
   assert(base);

   size_t chars = strzncpy(buf_ptr, base[index].name, buf_len);
   /* if its an array, append [0] onto the result */
   if (((int)chars > 0) && (base[index].is_array))
   {
      strncat(buf_ptr, "[0]", strlen(buf_ptr));
      /* recalculate the length */
      chars = strlen(buf_ptr);
   }

   if (length) *length = ((int)chars > 0) ? (GLsizei)chars : 0;
   if (size) *size = base[index].size;
   if (type) *type = base[index].type;

   mem_unlock(phandle);
end:
   GL20_UNLOCK_SERVER_STATE();
}

/*
   glGetAttachedShaders()

   Gets the names of the shaders attached to a specified program. Gives
   GL_INVALID_VALUE if the program does not exist, or GL_INVALID_OPERATION
   if the program argument is actually a shader. GL_INVALID_VALUE if
   maxcount < 0

   Implementation: Done
   Error Checks: Done
*/

void glGetAttachedShaders_impl_20 (GLuint p, GLsizei maxcount, GLsizei *pcount, GLuint *shaders)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   assert(state);

   if (maxcount >= 0) {
      MEM_HANDLE_T phandle;
      GL20_PROGRAM_T *program = get_program(state, p, &phandle);

      if (program) {
         int32_t count = 0;

         if (shaders) {
            if (maxcount > 0) {
               if (program->mh_vertex != MEM_HANDLE_INVALID) {
                  GL20_SHADER_T *vertex = (GL20_SHADER_T *)mem_lock(program->mh_vertex, NULL);

                  shaders[count++] = vertex->name;
                  maxcount--;

                  mem_unlock(program->mh_vertex);
               }
            }

            if (maxcount > 0) {
               if (program->mh_fragment != MEM_HANDLE_INVALID) {
                  GL20_SHADER_T *fragment = (GL20_SHADER_T *)mem_lock(program->mh_fragment, NULL);

                  shaders[count++] = fragment->name;
                  maxcount--;

                  mem_unlock(program->mh_fragment);
               }
            }
         }

         if (pcount)
            *pcount = count;

         mem_unlock(phandle);
      }
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   GL20_UNLOCK_SERVER_STATE();
}

int glGetAttribLocation_impl_20 (GLuint p, const char *name)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   if (!state) return 0;

   int result = -1;
   MEM_HANDLE_T phandle;
   GL20_PROGRAM_T *program = get_program(state, p, &phandle);
   if (program == NULL) goto end;

   if (name == NULL) {
      mem_unlock(phandle);
      goto end;
   }

   if (!program->linked) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      mem_unlock(phandle);
      goto end;
   }

   for (unsigned i = 0; i < program->num_attributes; i++) {
      GL20_ATTRIB_INFO_T *base = program->attributes;
      int b = strcmp(base[i].name, name);
      assert((base[i].offset & 3) == 0);
      if (!b) {
         result = base[i].offset >> 2;
         break;
      }
   }

   mem_unlock(phandle);

end:
   GL20_UNLOCK_SERVER_STATE();
   return result;
}

/*
   GetProgramiv

   DELETE STATUS False GetProgramiv
   LINK STATUS False GetProgamiv
   VALIDATE STATUS False GetProgramiv
   ATTACHED SHADERS 0 GetProgramiv
   INFO LOG LENGTH 0 GetProgramiv
   ACTIVE UNIFORMS 0 GetProgamiv
   ACTIVE UNIFORM MAX LENGTH 0 GetProgramiv
   ACTIVE ATTRIBUTES 0 GetProgramiv
   ACTIVE ATTRIBUTE MAX LENGTH 0 GetProgramiv

   If pname is ACTIVE ATTRIBUTE MAX LENGTH, the length of the longest
   active attribute name, including a null terminator, is returned.
*/

int glGetProgramiv_impl_20 (GLuint p, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   MEM_HANDLE_T phandle;

   GL20_PROGRAM_T *program = get_program(state, p, &phandle);

   int result;

   if (program) {
      switch (pname) {
      case GL_DELETE_STATUS:
         params[0] = program->deleted ? 1 : 0;
         result = 1;
         break;
      case GL_LINK_STATUS:
         params[0] = program->linked ? 1 : 0;
         result = 1;
         break;
      case GL_VALIDATE_STATUS:
         params[0] = program->validated ? 1 : 0;
         result = 1;
         break;
      case GL_ATTACHED_SHADERS:
         params[0] = (program->mh_vertex != MEM_HANDLE_INVALID) + (program->mh_fragment != MEM_HANDLE_INVALID);
         result = 1;
         break;
      case GL_INFO_LOG_LENGTH:
         if (program->info_log == NULL)
            params[0] = 0;
         else
            params[0] = strlen(program->info_log) + 1;
         result = 1;
         break;
      case GL_ACTIVE_UNIFORMS:
         params[0] = program->num_uniforms;
         result = 1;
         break;
      case GL_ACTIVE_UNIFORM_MAX_LENGTH:
      {
         GL20_UNIFORM_INFO_T *base = program->uniforms;
         unsigned count = program->num_uniforms;

         assert(base != NULL || count == 0);

         size_t max = 0;
         for (unsigned i = 0; i < count; i++) {
            uint32_t size = strlen(base[i].name) + 1;
            /* if the element is an array it needs [0] appending to it */
            if (base[i].is_array)
               size += 3;

            if (size > max)
               max = size;
         }
         params[0] = max;
         result = 1;
         break;
      }
      case GL_ACTIVE_ATTRIBUTES:
         params[0] = program->num_attributes;
         result = 1;
         break;
      case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
      {
         size_t max = 0;
         GL20_ATTRIB_INFO_T *base = program->attributes;
         unsigned count = program->num_attributes;
         assert(base != NULL || count == 0);
         for (unsigned i = 0; i < count; i++) {
            size_t size = strlen(base[i].name) + 1;

            if (size > max)
               max = size;
         }
         params[0] = max;
         result = 1;
         break;
      }
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
         break;
      }

      mem_unlock(phandle);
   } else
      result = 0;

   GL20_UNLOCK_SERVER_STATE();

   return result;
}

void glGetProgramInfoLog_impl_20 (GLuint p, GLsizei bufsize, GLsizei *length, char *infolog)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   if (!state) return;

   if (bufsize < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   MEM_HANDLE_T phandle;
   GL20_PROGRAM_T *program = get_program(state, p, &phandle);

   if (program == NULL) goto end;

   size_t chars = 0;
   if (program->info_log != NULL)
      chars = strzncpy(infolog, program->info_log, bufsize);

   if (length)
      *length = _max(0, (GLsizei)chars);

   mem_unlock(phandle);

end:
   GL20_UNLOCK_SERVER_STATE();
}

int glGetUniformfv_impl_20 (GLuint p, GLint location, GLfloat *params)
{
   return get_uniform_internal(p, location, params, 1);
}

int glGetUniformiv_impl_20 (GLuint p, GLint location, GLint *params)
{
   return get_uniform_internal(p, location, params, 0);
}

static int32_t get_uniform_length(const char *name)
{
   int32_t len;

   assert(name);

   len = (int32_t)strlen(name);

   if (len > 0 && name[len - 1] == ']') {
      len--;

      while (len > 0 && isdigit(name[len - 1]))
         len--;

      if (name[len - 1] == '[')
         len--;
      else
         len = -1;   /* error, mallformed [] array offset */
   }

   return len;
}

static int32_t get_uniform_offset(const char *name, int32_t pos)
{
   int32_t len;
   int32_t off = 0;

   assert(name);
   assert(pos >= 0);

   len = (int32_t)strlen(name);

   assert(pos <= len);

   if (pos < len) {
      if (name[pos++] != '[')
         return -1;

      while (isdigit(name[pos]))
         off = off * 10 + name[pos++] - '0';

      if (name[pos++] != ']')
         return -1;

      if (pos < len)
         return -1;
   }

   return off;
}

/* The Unity engine (and possibly other apps) expect uniform locations to be packed
 * into minimal values (it uses them for array allocations).
 *
 * We will return the uniform offsets that were calculated during program link, with
 * the addition of the array or structure index that is being requested. Since the
 * calculated offsets already make room for all the array or structure elements, the
 * locations are guaranteed not to overlap.
*/
static GLint encode_location(unsigned index, unsigned offset)
{
   return index + offset;
}

static void location_decode_index(GL20_UNIFORM_INFO_T *info, int32_t count, GLint location, GLint *index, GLint *offset)
{
   int i;
   for (i = 0; i < count; i++) {
      if (info[i].offset > location)
         break;
   }
   if (i > 0)
      i--;

   *index = i;
   *offset = location - info[i].offset;
}

int glGetUniformLocation_impl_20 (GLuint p, const char *name)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   if (!state) return -1;

   MEM_HANDLE_T phandle;
   GL20_PROGRAM_T *program = get_program(state, p, &phandle);

   int result = -1;

   if (program == NULL) goto end;

   if (!program->linked) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      mem_unlock(phandle);
      goto end;
   }

   int32_t off;
   int32_t len = get_uniform_length(name);
   if (len > 0)
      off = get_uniform_offset(name, len);
   else
      off = -1;

   if (off >= 0) {
      GL20_UNIFORM_INFO_T *base = program->uniforms;
      unsigned count = program->num_uniforms;

      //if there are no uniforms, program->mh_uniform_info will point to a zero sized handle
      //so base == 0
      assert(base != NULL || count == 0);

      for (unsigned i = 0; i < count; i++) {
         const char *curr = base[i].name;
         int b = (strlen(curr) == (size_t)len) && !strncmp(curr, name, len) && (off < base[i].size);

         if (b) {
            result = encode_location(base[i].offset, off);
            break;
         }
      }
   }

   mem_unlock(phandle);
end:
   GL20_UNLOCK_SERVER_STATE();
   return result;
}
/*
void glGetVertexAttribfv_impl_20 (GLuint index, GLenum pname, GLfloat *params)
{
   UNREACHABLE();
}

void glGetVertexAttribiv_impl_20 (GLuint index, GLenum pname, GLint *params)
{
   UNREACHABLE();
}

void glGetVertexAttribPointerv_impl_20 (GLuint index, GLenum pname, void **pointer)
{
   UNREACHABLE();
}
*/


/*
   glIsProgram()

   Returns TRUE if program is the name of a program object. If program is zero,
   or a non-zero value that is not the name of a program object, IsProgram returns
   FALSE. No error is generated if program is not a valid program object name.

   Implementation: Done
   Error Checks: Done
*/

GLboolean glIsProgram_impl_20 (GLuint p)
{
   GLboolean result;
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   MEM_HANDLE_T handle = glxx_shared_get_pobject((GLXX_SHARED_T *)mem_lock(state->mh_shared, NULL), p);
   mem_unlock(state->mh_shared);

   if (handle == MEM_HANDLE_INVALID)
      result = GL_FALSE;
   else {
      result = gl20_is_program((GL20_PROGRAM_T *)mem_lock(handle, NULL));
      mem_unlock(handle);
   }

   GL20_UNLOCK_SERVER_STATE();

   return result;
}

/*
   glIsShader()

   Returns TRUE if shader is the name of a shader object. If shader is zero,
   or a non-zero value that is not the name of a shader object, IsShader returns
   FALSE. No error is generated if shader is not a valid shader object name.

   Implementation: Done
   Error Checks: Done
*/

GLboolean glIsShader_impl_20 (GLuint s)
{
   GLboolean result;
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   MEM_HANDLE_T handle = glxx_shared_get_pobject((GLXX_SHARED_T *)mem_lock(state->mh_shared, NULL), s);
   mem_unlock(state->mh_shared);

   if (handle == MEM_HANDLE_INVALID)
      result = GL_FALSE;
   else {
      result = gl20_is_shader((GL20_SHADER_T *)mem_lock(handle, NULL));
      mem_unlock(handle);
   }

   GL20_UNLOCK_SERVER_STATE();

   return result;
}




void glLinkProgram_impl_20 (GLuint p)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   MEM_HANDLE_T phandle;

   GL20_PROGRAM_T *program = get_program(state, p, &phandle);

   if (program) {
      /* wait to make sure noone is using the buffer */
      khrn_hw_common_wait();

      gl20_program_link(program);

#define TEST_SCHED_AT_LINK_TIME
#ifdef TEST_SCHED_AT_LINK_TIME
      glxx_schedule_during_link(state, program);
#endif

      mem_unlock(phandle);
   }

   GL20_UNLOCK_SERVER_STATE();
}

#define COERCE_ID 0
#define COERCE_INT_TO_FLOAT 1
#define COERCE_INT_TO_BOOL 2
#define COERCE_FLOAT_TO_BOOL 3
#define COERCE_FLOAT_TO_INT 4
static GLboolean is_compatible_uniform(GL20_UNIFORM_INFO_T *info, GLint stride, GLboolean is_float, GLboolean is_matrix, uint32_t *coercion)
{
   switch (info->type) {
   case GL_FLOAT:
      *coercion = COERCE_ID;
      return stride == 1 && is_float && !is_matrix;
   case GL_FLOAT_VEC2:
      *coercion = COERCE_ID;
      return stride == 2 && is_float && !is_matrix;
   case GL_FLOAT_VEC3:
      *coercion = COERCE_ID;
      return stride == 3 && is_float && !is_matrix;
   case GL_FLOAT_VEC4:
      *coercion = COERCE_ID;
      return stride == 4 && is_float && !is_matrix;
   case GL_INT:
      *coercion = COERCE_INT_TO_FLOAT;
      return stride == 1 && !is_float && !is_matrix;
   case GL_INT_VEC2:
      *coercion = COERCE_INT_TO_FLOAT;
      return stride == 2 && !is_float && !is_matrix;
   case GL_INT_VEC3:
      *coercion = COERCE_INT_TO_FLOAT;
      return stride == 3 && !is_float && !is_matrix;
   case GL_INT_VEC4:
      *coercion = COERCE_INT_TO_FLOAT;
      return stride == 4 && !is_float && !is_matrix;
   case GL_BOOL:
      *coercion = is_float ? COERCE_FLOAT_TO_BOOL : COERCE_INT_TO_BOOL;
      return stride == 1 && !is_matrix;
   case GL_BOOL_VEC2:
      *coercion = is_float ? COERCE_FLOAT_TO_BOOL : COERCE_INT_TO_BOOL;
      return stride == 2 && !is_matrix;
   case GL_BOOL_VEC3:
      *coercion = is_float ? COERCE_FLOAT_TO_BOOL : COERCE_INT_TO_BOOL;
      return stride == 3 && !is_matrix;
   case GL_BOOL_VEC4:
      *coercion = is_float ? COERCE_FLOAT_TO_BOOL : COERCE_INT_TO_BOOL;
      return stride == 4 && !is_matrix;
   case GL_FLOAT_MAT2:
      *coercion = COERCE_ID;
      return stride == 4 && is_float && is_matrix;
   case GL_FLOAT_MAT3:
      *coercion = COERCE_ID;
      return stride == 9 && is_float && is_matrix;
   case GL_FLOAT_MAT4:
      *coercion = COERCE_ID;
      return stride == 16 && is_float && is_matrix;
   case GL_SAMPLER_2D:
   case GL_SAMPLER_EXTERNAL_OES:
   case GL_SAMPLER_CUBE:
      *coercion = COERCE_ID;
      return stride == 1 && !is_float && !is_matrix;
   default:
      UNREACHABLE();
      break;
   }

   return GL_FALSE;
}

static void uniformv_internal(GLint location, GLsizei num, const void *v, GLint stride, GLboolean is_float, GLboolean is_matrix)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   if (!state) return;

   if (location == -1) {
      // check the program object
      if (state->mh_program == MEM_HANDLE_INVALID) {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      }
      goto end;
   }

   if (state->mh_program == MEM_HANDLE_INVALID) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   GL20_PROGRAM_T *program = (GL20_PROGRAM_T *)mem_lock(state->mh_program, NULL);

   if (!program->linked) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      mem_unlock(state->mh_program);
      goto end;
   }

   int32_t index, offset;
   uint32_t coercion = 0;
   GL20_UNIFORM_INFO_T *info = program->uniforms;
   int count = program->num_uniforms;

   location_decode_index(info, count, location, &index, &offset);

   if ((index < 0) ||
       (index >= count) ||
       (offset < 0) ||
       (offset >= info[index].size) ||
       !((info[index].is_array) || (num == 1)) ||
      !is_compatible_uniform(&info[index], stride, is_float, is_matrix, &coercion))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      mem_unlock(state->mh_program);
      goto end;
   }

   GLint *dst = program->uniform_data;
   assert(dst);

   if (offset + num > info[index].size)
      num = info[index].size - offset;

   dst += info[index].offset + offset * stride;
   GLint *src = (GLint*)v;

   switch (coercion)
   {
   case COERCE_ID:
      for (int i = 0; i < (num * stride); i++)
         *(dst++) = *(src++);
      break;
   case COERCE_INT_TO_FLOAT:
      //TODO: is it acceptable to lose precision by converting ints to floats?
      for (int i = 0; i < (num * stride); i++)
         *(float*)(dst++) = (float)*(src++);
      break;
   case COERCE_INT_TO_BOOL:
      for (int i = 0; i < (num * stride); i++)
         *(dst++) = *(src++) ? 1 : 0;
      break;
   case COERCE_FLOAT_TO_BOOL:
      for (int i = 0; i < (num * stride); i++)
         *(dst++) = (*(float*)(src++) != 0.0f) ? 1 : 0;
      break;
   default:
      //Unreachable
      UNREACHABLE();
   }

   mem_unlock(state->mh_program);

end:
   GL20_UNLOCK_SERVER_STATE();
}

static void uniform_coercion_for_get(GL20_UNIFORM_INFO_T *info, GLboolean is_float, uint32_t *coercion, int *num_elements)
{
   switch (info->type) {
   case GL_FLOAT:
   case GL_INT:
      *num_elements = 1;
      *coercion = is_float ? COERCE_ID : COERCE_FLOAT_TO_INT;
      break;
   case GL_FLOAT_VEC2:
   case GL_INT_VEC2:
      *num_elements = 2;
      *coercion = is_float ? COERCE_ID : COERCE_FLOAT_TO_INT;
      break;
   case GL_FLOAT_VEC3:
   case GL_INT_VEC3:
      *num_elements = 3;
      *coercion = is_float ? COERCE_ID : COERCE_FLOAT_TO_INT;
      break;
   case GL_FLOAT_VEC4:
   case GL_INT_VEC4:
      *num_elements = 4;
      *coercion = is_float ? COERCE_ID : COERCE_FLOAT_TO_INT;
      break;
   case GL_BOOL:
   case GL_SAMPLER_2D:
   case GL_SAMPLER_CUBE:
      *num_elements = 1;
      *coercion = is_float ? COERCE_INT_TO_FLOAT : COERCE_ID;
      break;
   case GL_BOOL_VEC2:
      *num_elements = 2;
      *coercion = is_float ? COERCE_INT_TO_FLOAT : COERCE_ID;
      break;
   case GL_BOOL_VEC3:
      *num_elements = 3;
      *coercion = is_float ? COERCE_INT_TO_FLOAT : COERCE_ID;
      break;
   case GL_BOOL_VEC4:
      *num_elements = 4;
      *coercion = is_float ? COERCE_INT_TO_FLOAT : COERCE_ID;
      break;
   case GL_FLOAT_MAT2:
      *num_elements = 4;
      *coercion = is_float ? COERCE_ID : COERCE_FLOAT_TO_INT;
      break;
   case GL_FLOAT_MAT3:
      *num_elements = 9;
      *coercion = is_float ? COERCE_ID : COERCE_FLOAT_TO_INT;
      break;
   case GL_FLOAT_MAT4:
      *num_elements = 16;
      *coercion = is_float ? COERCE_ID : COERCE_FLOAT_TO_INT;
      break;
   default:
      //Unreachable
      UNREACHABLE();
      break;
   }
}

static int get_uniform_internal(GLuint p, GLint location, const void *v, GLboolean is_float)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   if (!state) return 0;

   MEM_HANDLE_T phandle;
   GL20_PROGRAM_T *program = get_program(state, p, &phandle);

   int num_elements = 0;
   if (program == NULL) goto end;

   if (!program->linked) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      mem_unlock(phandle);
      goto end;
   }

   int32_t index, offset;
   uint32_t coercion = 0;
   GL20_UNIFORM_INFO_T *info = program->uniforms;
   int count = program->num_uniforms;

   assert(info != NULL || count == 0);

   location_decode_index(info, count, location, &index, &offset);

   if ((index < 0) ||
      (index >= count) ||
      (offset < 0) ||
      (offset >= info[index].size)) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      mem_unlock(phandle);
      goto end;
   }

   uniform_coercion_for_get(&info[index], is_float, &coercion, &num_elements);

   GLint *src = (GLint *)program->uniform_data;
   assert(src);

   src += info[index].offset + offset * num_elements;
   GLint *dst = (GLint*)v;

   switch (coercion)
   {
   case COERCE_ID:
      for (int i = 0; i < num_elements; i++)
         *(dst++) = *(src++);
      break;
   case COERCE_INT_TO_FLOAT:
      for (int i = 0; i < num_elements; i++)
         *(float*)(dst++) = (float)*(src++);
      break;
   case COERCE_FLOAT_TO_INT:
      for (int i = 0; i < num_elements; i++)
         *(dst++) = (GLint)*(float*)(src++);
      break;
   default:
      //Unreachable
      UNREACHABLE();
   }

   mem_unlock(phandle);

end:
   GL20_UNLOCK_SERVER_STATE();
   return num_elements;
}

void glUniform1i_impl_20 (GLint location, GLint x)
{
   uniformv_internal(location, 1, &x, 1, GL_FALSE, GL_FALSE);
}

void glUniform2i_impl_20 (GLint location, GLint x, GLint y)
{
   GLint v[2];

   v[0] = x; v[1] = y;

   uniformv_internal(location, 1, v, 2, GL_FALSE, GL_FALSE);
}

void glUniform3i_impl_20 (GLint location, GLint x, GLint y, GLint z)
{
   GLint v[3];

   v[0] = x; v[1] = y; v[2] = z;

   uniformv_internal(location, 1, v, 3, GL_FALSE, GL_FALSE);
}

void glUniform4i_impl_20 (GLint location, GLint x, GLint y, GLint z, GLint w)
{
   GLint v[4];

   v[0] = x; v[1] = y; v[2] = z; v[3] = w;

   uniformv_internal(location, 1, v, 4, GL_FALSE, GL_FALSE);
}

void glUniform1f_impl_20 (GLint location, GLfloat x)
{
   uniformv_internal(location, 1, &x, 1, GL_TRUE, GL_FALSE);
}

void glUniform2f_impl_20 (GLint location, GLfloat x, GLfloat y)
{
   GLfloat v[2];

   v[0] = x; v[1] = y;

   uniformv_internal(location, 1, v, 2, GL_TRUE, GL_FALSE);
}

void glUniform3f_impl_20 (GLint location, GLfloat x, GLfloat y, GLfloat z)
{
   GLfloat v[3];

   v[0] = x; v[1] = y; v[2] = z;

   uniformv_internal(location, 1, v, 3, GL_TRUE, GL_FALSE);
}

void glUniform4f_impl_20 (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GLfloat v[4];

   v[0] = x; v[1] = y; v[2] = z; v[3] = w;

   uniformv_internal(location, 1, v, 4, GL_TRUE, GL_FALSE);
}

void glUniform1iv_impl_20 (GLint location, GLsizei num, int size, const GLint *v)
{
   UNUSED(size);

   uniformv_internal(location, num, v, 1, GL_FALSE, GL_FALSE);
}

void glUniform2iv_impl_20 (GLint location, GLsizei num, int size, const GLint *v)
{
   UNUSED(size);

   uniformv_internal(location, num, v, 2, GL_FALSE, GL_FALSE);
}

void glUniform3iv_impl_20 (GLint location, GLsizei num, int size, const GLint *v)
{
   UNUSED(size);

   uniformv_internal(location, num, v, 3, GL_FALSE, GL_FALSE);
}

void glUniform4iv_impl_20 (GLint location, GLsizei num, int size, const GLint *v)
{
   UNUSED(size);

   uniformv_internal(location, num, v, 4, GL_FALSE, GL_FALSE);
}

void glUniform1fv_impl_20 (GLint location, GLsizei num, int size, const GLfloat *v)
{
   UNUSED(size);

   uniformv_internal(location, num, v, 1, GL_TRUE, GL_FALSE);
}

void glUniform2fv_impl_20 (GLint location, GLsizei num, int size, const GLfloat *v)
{
   UNUSED(size);

   uniformv_internal(location, num, v, 2, GL_TRUE, GL_FALSE);
}

void glUniform3fv_impl_20 (GLint location, GLsizei num, int size, const GLfloat *v)
{
   UNUSED(size);

   uniformv_internal(location, num, v, 3, GL_TRUE, GL_FALSE);
}

void glUniform4fv_impl_20 (GLint location, GLsizei num, int size, const GLfloat *v)
{
   UNUSED(size);

   uniformv_internal(location, num, v, 4, GL_TRUE, GL_FALSE);
}

/*
   The transpose parameter in the UniformMatrix API call can only be FALSE in
   OpenGL ES 2.0. The transpose field was added to UniformMatrix as OpenGL 2.0
   supports both column major and row major matrices. OpenGL ES 1.0 and 1.1 do
   not support row major matrices because there was no real demand for it. There
   is no reason to support both column major and row major matrices in OpenGL ES
   2.0, so the default matrix type used in OpenGL (i.e. column major) is the
   only one supported. An INVALID VALUE error will be generated if tranpose is
   not FALSE.
*/

void glUniformMatrix2fv_impl_20 (GLint location, GLsizei num, GLboolean transpose, int size, const GLfloat *v)
{
   UNUSED(size);

   if (!transpose)
      uniformv_internal(location, num, v, 4, GL_TRUE, GL_TRUE);
   else {
      GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

      glxx_server_state_set_error(state, GL_INVALID_VALUE);

      GL20_UNLOCK_SERVER_STATE();
   }
}

void glUniformMatrix3fv_impl_20 (GLint location, GLsizei num, GLboolean transpose, int size, const GLfloat *v)
{
   UNUSED(size);

   if (!transpose)
      uniformv_internal(location, num, v, 9, GL_TRUE, GL_TRUE);
   else {
      GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

      glxx_server_state_set_error(state, GL_INVALID_VALUE);

      GL20_UNLOCK_SERVER_STATE();
   }
}

void glUniformMatrix4fv_impl_20 (GLint location, GLsizei num, GLboolean transpose, int size, const GLfloat *v)
{
   UNUSED(size);

   if (!transpose)
      uniformv_internal(location, num, v, 16, GL_TRUE, GL_TRUE);
   else {
      GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

      glxx_server_state_set_error(state, GL_INVALID_VALUE);

      GL20_UNLOCK_SERVER_STATE();
   }
}

void glUseProgram_impl_20 (GLuint p) // S
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   assert(state);

   if (p) {
      MEM_HANDLE_T phandle;

      GL20_PROGRAM_T *program = get_program(state, p, &phandle);

      if (program) {
         if (program->linked) {
            gl20_program_acquire(program);

            release_program((GLXX_SHARED_T *)mem_lock(state->mh_shared, NULL), state->mh_program);
            mem_unlock(state->mh_shared);

            MEM_ASSIGN(state->mh_program, phandle);
         } else {
            //TODO have I got the reference counts right? I'm new to all this.
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         }
         mem_unlock(phandle);
      }
   } else {
      release_program((GLXX_SHARED_T *)mem_lock(state->mh_shared, NULL), state->mh_program);
      mem_unlock(state->mh_shared);

      MEM_ASSIGN(state->mh_program, MEM_HANDLE_INVALID);
   }

   GL20_UNLOCK_SERVER_STATE();
}

void glValidateProgram_impl_20 (GLuint p)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   MEM_HANDLE_T phandle;

   GL20_PROGRAM_T *program = get_program(state, p, &phandle);

   if (program) {
      program->validated = gl20_validate_program(state, program);
      free(program->info_log);
      program->info_log = NULL;
      mem_unlock(phandle);
   }

   GL20_UNLOCK_SERVER_STATE();
}
/*
void glVertexAttrib1f_impl_20 (GLuint indx, GLfloat x)
{
  UNREACHABLE();
}

void glVertexAttrib2f_impl_20 (GLuint indx, GLfloat x, GLfloat y)
{
  UNREACHABLE();
}

void glVertexAttrib3f_impl_20 (GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
  UNREACHABLE();
}

void glVertexAttrib4f_impl_20 (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  UNREACHABLE();
}

void glVertexAttrib1fv_impl_20 (GLuint indx, const GLfloat *values)
{
  UNREACHABLE();
}

void glVertexAttrib2fv_impl_20 (GLuint indx, const GLfloat *values)
{
  UNREACHABLE();
}

void glVertexAttrib3fv_impl_20 (GLuint indx, const GLfloat *values)
{
  UNREACHABLE();
}

void glVertexAttrib4fv_impl_20 (GLuint indx, const GLfloat *values)
{
  UNREACHABLE();
}
*/
void glVertexAttribPointer_impl_20 (GLuint indx)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   assert(indx < GLXX_CONFIG_MAX_VERTEX_ATTRIBS);

   MEM_ASSIGN(state->bound_buffer.mh_attrib_array[indx], state->bound_buffer.mh_array);

   GL20_UNLOCK_SERVER_STATE();
}

void glCompileShader_impl_20 (GLuint s)
{
   MEM_HANDLE_T shandle;
   GL20_SHADER_T *shader;

   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   assert(state);

   shader = get_shader(state, s, &shandle);

   if (shader) {
      gl20_shader_compile(shader);

      mem_unlock(shandle);
   }

   GL20_UNLOCK_SERVER_STATE();
}

/*
   GetShaderiv

   SHADER TYPE   GetShaderiv
   DELETE STATUS False GetShaderiv
   COMPILE STATUS   False GetShaderiv
   INFO LOG LENGTH   0 GetShaderiv
   SHADER SOURCE LENGTH   0 GetShaderiv

   If pname is SHADER SOURCE LENGTH, the length of the concatenation
   of the source strings making up the shader source, including a null
   terminator, is returned. If no source has been defined, zero is
   returned.
*/

int glGetShaderiv_impl_20 (GLuint s, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   MEM_HANDLE_T shandle;
   GL20_SHADER_T *shader = get_shader(state, s, &shandle);
   int result;

   if (shader) {
      switch (pname) {
      case GL_SHADER_TYPE:
         params[0] = shader->type;
         result = 1;
         break;
      case GL_DELETE_STATUS:
         params[0] = shader->deleted ? 1 : 0;
         result = 1;
         break;
      case GL_COMPILE_STATUS:
         params[0] = shader->compiled ? 1 : 0;
         result = 1;
         break;
      case GL_INFO_LOG_LENGTH:
      {
         if (shader->info_log == NULL)
            params[0] = 0;
         else
            params[0] = strlen(shader->info_log) + 1;
         result = 1;
         break;
      }
      case GL_SHADER_SOURCE_LENGTH:
      {
         size_t total = 0;

         for (unsigned i = 0; i < shader->sourcec; i++)
            total += strlen(shader->sourcev[i]);

         /* If there's any source at all, count 1 for the NULL terminator */
         if (shader->sourcec > 0) total++;

         params[0] = total;
         result = 1;
         break;
      }
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
         break;
      }

      mem_unlock(shandle);
   } else
      result = 0;

   GL20_UNLOCK_SERVER_STATE();

   return result;
}

void glGetShaderInfoLog_impl_20 (GLuint s, GLsizei bufsize, GLsizei *length, char *infolog)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   if (!state) return;

   MEM_HANDLE_T shandle;
   GL20_SHADER_T *shader = get_shader(state, s, &shandle);
   if (shader == NULL) goto end;

   if (bufsize < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      mem_unlock(shandle);
      goto end;
   }

   size_t chars = 0;
   if (shader->info_log != NULL)
      chars = strzncpy(infolog, shader->info_log, bufsize);

   if (length)
      *length = _max(0, (GLsizei)chars);

   mem_unlock(shandle);
end:
   GL20_UNLOCK_SERVER_STATE();
}

void glGetShaderSource_impl_20 (GLuint s, GLsizei bufsize, GLsizei *length, char *source)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   MEM_HANDLE_T shandle;
   uint32_t charswritten = 0;
   if (!state) return;

   GL20_SHADER_T *shader = get_shader(state, s, &shandle);
   if (shader == NULL) goto end;       /* get_shader will have set error */

   if (bufsize < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      mem_unlock(shandle);
      goto end;
   }

   if (bufsize > 1) {//need 1 byte for NULL terminator below
      for (unsigned i = 0; i < shader->sourcec; i++) {
         const char *str = shader->sourcev[i];
         int32_t len = strlen(str);
         assert(len >= 0);
         assert(str[len] == 0);

         if (charswritten + len >(uint32_t)bufsize - 1)
         {
            assert((int)bufsize - 1 - (int)charswritten >= 0);
            memcpy(source + charswritten, str, bufsize - 1 - charswritten);

            charswritten = bufsize - 1;
            break;
         }
         else
         {
            memcpy(source + charswritten, str, len);
            charswritten += len;
         }
      }
   }
   mem_unlock(shandle);

   if (length) {
      *length = charswritten;
   }
   if (bufsize > 0) {
      assert(charswritten < (uint32_t)bufsize);
      source[charswritten] = 0;
   }

end:
   GL20_UNLOCK_SERVER_STATE();
}

void glShaderSource_impl_20 (GLuint s, GLsizei count, const char **string, const GLint *length)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   if (!state) return;

   if (count < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   MEM_HANDLE_T shandle;
   GL20_SHADER_T *shader = get_shader(state, s, &shandle);
   if (shader == NULL) goto end;       /* get_shader will have set error */

   if (string == NULL) {
      mem_unlock(shandle);
      goto end;
   }

   if (!gl20_shader_set_source(shader, count, string, length))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

   mem_unlock(shandle);

end:
   GL20_UNLOCK_SERVER_STATE();
}

/* OES_shader_source + OES_shader_binary */

static bool is_precision_type(GLenum type)
{
   return type == GL_LOW_FLOAT ||
          type == GL_MEDIUM_FLOAT ||
          type == GL_HIGH_FLOAT ||
          type == GL_LOW_INT ||
          type == GL_MEDIUM_INT ||
          type == GL_HIGH_INT;
}

static bool is_float_type(GLenum type)
{
   return type == GL_LOW_FLOAT ||
          type == GL_MEDIUM_FLOAT ||
          type == GL_HIGH_FLOAT;
}

/*
   TODO: is this right? very poorly specified
*/

void glGetShaderPrecisionFormat_impl_20 (GLenum shadertype, GLenum precisiontype, GLint *result)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();

   if (is_shader_type(shadertype) && is_precision_type(precisiontype)) {
      if (is_float_type(precisiontype)) {
         result[0] = 127;
         result[1] = 127;
         result[2] = 23;
      }
      else {
         result[0] = 31;
         result[1] = 30;
         result[2] = 0;
      }
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   GL20_UNLOCK_SERVER_STATE();
}