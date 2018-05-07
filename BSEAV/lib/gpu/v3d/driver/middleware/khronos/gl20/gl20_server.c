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
#include <GLES2/gl2ext.h>

#include "middleware/khronos/glxx/glxx_texture.h"
#include "middleware/khronos/glxx/glxx_buffer.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "middleware/khronos/common/khrn_interlock.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/glxx/glxx_tweaker.h"

#include "middleware/khronos/common/khrn_mem.h"

#include <string.h>
#include <math.h>
#include <limits.h>

static int get_uniform_internal(GLuint p, GLint location, const void *v, GLboolean is_float);

bool gl20_server_state_init(GLXX_SERVER_STATE_T *state, GLXX_SHARED_T *shared, bool secure)
{
   state->type = OPENGL_ES_20;

   state->secure = secure;

   //initialise common portions of state
   if(!glxx_server_state_init(state, shared))
      return false;

   //gl 2.0 specific parts

   glxx_tweaker_init(&state->tweak_state, true);

   assert(state->program == NULL);

   state->point_size = 1.0f;

   return true;
}

/*
   Get a program pointer from a program name. Optionally retrieve
   the handle to the memory block storing the program structure.
   Gives GL_INVALID_VALUE if no object exists with that name, or
   GL_INVALID_OPERATION if the object is actually a shader and
   returns NULL in either case.
*/

static GL20_PROGRAM_T *get_program(GLXX_SERVER_STATE_T *state, GLuint p)
{
   GL20_PROGRAM_T *program = glxx_shared_get_pobject(state->shared, p);

   if (program == NULL) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      return NULL;
   }

   assert(program);

   if (!gl20_is_program(program)) {
      assert(gl20_is_shader((GL20_SHADER_T *)program));
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return NULL;
   }

   return program;
}

/*
   Get a shader pointer from a shader name. Optionally retrieve
   the handle to the memory block storing the shader structure.
   Gives GL_INVALID_VALUE if no object exists with that name, or
   GL_INVALID_OPERATION if the object is actually a program and
   returns NULL in either case.
*/

static GL20_SHADER_T *get_shader(GLXX_SERVER_STATE_T *state, GLuint s)
{
   GL20_SHADER_T *shader = glxx_shared_get_pobject(state->shared, s);

   if (shader == NULL) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      return NULL;
   }

   assert(shader);

   if (!gl20_is_shader(shader)) {
      assert(gl20_is_program((GL20_PROGRAM_T *)shader));
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return NULL;
   }

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

GL_API void GL_APIENTRY glAttachShader(GLuint p, GLuint s)
{
   GL20_PROGRAM_T *program;

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   program = get_program(state, p);

   if (program) {
      GL20_SHADER_T *shader = get_shader(state, s);

      if (shader) {
         GL20_SHADER_T **pshader = NULL;

         switch (shader->type) {
         case GL_VERTEX_SHADER:
            pshader = &program->vertex;
            break;
         case GL_FRAGMENT_SHADER:
            pshader = &program->fragment;
            break;
         default:
            UNREACHABLE();
            break;
         }

         if (*pshader != NULL)
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         else {
            gl20_shader_acquire(shader);

            KHRN_MEM_ASSIGN(*pshader, shader);
         }
      }
   }

   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_API void GL_APIENTRY glBindAttribLocation(GLuint p, GLuint index, const char *name)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (name)
   {
      if (index < GLXX_CONFIG_MAX_VERTEX_ATTRIBS)
         if (strncmp(name, "gl_", 3)) {
            GL20_PROGRAM_T *program = get_program(state, p);

            if (program) {
               if (!gl20_program_bind_attrib(program, index, name))
                  glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
            }
         } else
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
   }

   glxx_unlock_server_state(OPENGL_ES_20);
}

/*
   glBlendColor()

   Sets the constant color for use in blending. All inputs are clamped to the
   range [0.0, 1.0] before being stored. No errors are generated.

   Implementation: Done
   Error Checks: Done
*/

GL_API void GL_APIENTRY glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   state->blend_color[0] = clampf(red, 0.0f, 1.0f);
   state->blend_color[1] = clampf(green, 0.0f, 1.0f);
   state->blend_color[2] = clampf(blue, 0.0f, 1.0f);
   state->blend_color[3] = clampf(alpha, 0.0f, 1.0f);

   glxx_unlock_server_state(OPENGL_ES_20);
}

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

static void blend_equation_separate(GLenum modeRGB, GLenum modeAlpha) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (is_blend_equation(modeRGB) && is_blend_equation(modeAlpha)) {
      state->changed_backend = true;
      state->blend_equation.rgb = modeRGB;
      state->blend_equation.alpha = modeAlpha;
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_API void GL_APIENTRY glBlendEquation(GLenum mode) // S
{
   blend_equation_separate(mode, mode);
}

GL_API void GL_APIENTRY glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) // S
{
   blend_equation_separate(modeRGB, modeAlpha);
}

GL_API void GL_APIENTRY glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   glxx_server_set_blend_func(state, srcRGB, dstRGB, srcAlpha, dstAlpha);

   glxx_unlock_server_state(OPENGL_ES_20);
}

/*
   glCreateProgram()

   Creates a new, empty program and returns its name. No errors are generated.

   Implementation: Done
   Error Checks: Done
*/

GL_API GLuint GL_APIENTRY glCreateProgram(void)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return 0;

   GLuint result = glxx_shared_create_program(state->shared);

   if (result == 0)
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

   glxx_unlock_server_state(OPENGL_ES_20);

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

GL_API GLuint GL_APIENTRY glCreateShader(GLenum type)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return 0;

   GLuint result = 0;

   if (is_shader_type(type)) {
      result = glxx_shared_create_shader(state->shared, type);
      if (result == 0)
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_20);

   return result;
}

static void try_delete_shader(GLXX_SHARED_T *shared, GL20_SHADER_T *shader)
{
   if (shader->refs == 0 && shader->deleted)
      glxx_shared_delete_pobject(shared, shader->name);
}

static void release_shader(GLXX_SHARED_T *shared, GL20_SHADER_T *shader)
{
   if (shader != NULL) {

      khrn_mem_acquire(shader);

      assert(gl20_is_shader(shader));

      gl20_shader_release(shader);

      try_delete_shader(shared, shader);

      KHRN_MEM_ASSIGN(shader, NULL);
   }
}

static void try_delete_program(GLXX_SHARED_T *shared, GL20_PROGRAM_T *program)
{
   if (program->refs == 0 && program->deleted) {
      release_shader(shared, program->vertex);
      release_shader(shared, program->fragment);

      glxx_shared_delete_pobject(shared, program->name);
   }
}

static void release_program(GLXX_SHARED_T *shared, GL20_PROGRAM_T *program)
{
   if (program != NULL) {

      khrn_mem_acquire(program);

      assert(gl20_is_program(program));

      gl20_program_release(program);

      try_delete_program(shared, program);

      KHRN_MEM_ASSIGN(program, NULL);
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

GL_API void GL_APIENTRY glDeleteProgram(GLuint p)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   GLXX_SHARED_T *shared = state->shared;

   if (p) {
      GL20_PROGRAM_T *program = glxx_shared_get_pobject(shared, p);

      if (program != NULL) {

         /* wait to make sure noone is using the buffer */
         khrn_hw_common_wait();

         khrn_mem_acquire(program);

         if (gl20_is_program(program)) {
            program->deleted = GL_TRUE;

            try_delete_program(shared, program);
         } else
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);

         KHRN_MEM_ASSIGN(program, NULL);
      } else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
   }

   glxx_unlock_server_state(OPENGL_ES_20);
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

GL_API void GL_APIENTRY glDeleteShader(GLuint s)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   GLXX_SHARED_T *shared = state->shared;

   if (s) {
      GL20_SHADER_T *shader = glxx_shared_get_pobject(shared, s);

      if (shader != NULL) {

         khrn_mem_acquire(shader);

         if (gl20_is_shader(shader)) {
            shader->deleted = true;

            try_delete_shader(shared, shader);
         } else
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);

         KHRN_MEM_ASSIGN(shader, NULL);
      } else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
   }

   glxx_unlock_server_state(OPENGL_ES_20);
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

GL_API void GL_APIENTRY glDetachShader(GLuint p, GLuint s)
{
   GL20_PROGRAM_T *program;

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   program = get_program(state, p);

   if (program) {
      GL20_SHADER_T *shader = get_shader(state, s);

      if (shader) {
         GL20_SHADER_T **pshader = NULL;

         switch (shader->type) {
         case GL_VERTEX_SHADER:
            pshader = &program->vertex;
            break;
         case GL_FRAGMENT_SHADER:
            pshader = &program->fragment;
            break;
         default:
            UNREACHABLE();
            break;
         }

         if (*pshader != shader)
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         else {
            gl20_shader_release(shader);

            try_delete_shader(state->shared, shader);

            KHRN_MEM_ASSIGN(*pshader, NULL);
         }
      }
   }

   glxx_unlock_server_state(OPENGL_ES_20);
}


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

GL_APICALL void GL_APIENTRY glGetActiveAttrib(GLuint p, GLuint index, GLsizei buf_len, GLsizei *length, GLint *size, GLenum *type, char *buf_ptr)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   GL20_PROGRAM_T *program = get_program(state, p);

   if (program == NULL) goto end;       /* get_shader will have set error */

   if (index >= program->num_attributes) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (buf_len < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   GL20_ATTRIB_INFO_T *base = program->attributes;

   size_t chars = strzncpy(buf_ptr, base[index].name, buf_len);

   if (length) *length = (GLsizei)chars;
   if (size) *size = 1;        // no array or structure attributes
   if (type) *type = base[index].type;

end:
   glxx_unlock_server_state(OPENGL_ES_20);
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

GL_APICALL void GL_APIENTRY glGetActiveUniform(GLuint p, GLuint index, GLsizei buf_len, GLsizei *length, GLint *size, GLenum *type, char *buf_ptr)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   GL20_PROGRAM_T *program = get_program(state, p);

   if (program == NULL) goto end;

   if (index >= program->num_uniforms) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
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

end:
   glxx_unlock_server_state(OPENGL_ES_20);
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

GL_APICALL void GL_APIENTRY glGetAttachedShaders(GLuint p, GLsizei maxcount, GLsizei *pcount, GLuint *shaders)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (maxcount >= 0) {
      GL20_PROGRAM_T *program = get_program(state, p);

      if (program) {
         int32_t count = 0;

         if (shaders) {
            if (maxcount > 0) {
               if (program->vertex != NULL) {
                  GL20_SHADER_T *vertex = program->vertex;
                  shaders[count++] = vertex->name;
                  maxcount--;
               }
            }

            if (maxcount > 0) {
               if (program->fragment != NULL) {
                  GL20_SHADER_T *fragment = program->fragment;
                  shaders[count++] = fragment->name;
                  maxcount--;
               }
            }
         }

         if (pcount)
            *pcount = count;
      }
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_APICALL int GL_APIENTRY glGetAttribLocation(GLuint p, const char *name)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return 0;

   int result = -1;
   GL20_PROGRAM_T *program = get_program(state, p);
   if (program == NULL) goto end;

   if (name == NULL)
      goto end;

   if (!program->linked) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
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

end:
   glxx_unlock_server_state(OPENGL_ES_20);
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

GL_APICALL void GL_APIENTRY glGetProgramiv(GLuint p, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   GL20_PROGRAM_T *program = get_program(state, p);

   if (program) {
      switch (pname) {
      case GL_DELETE_STATUS:
         params[0] = program->deleted ? 1 : 0;
         break;
      case GL_LINK_STATUS:
         params[0] = program->linked ? 1 : 0;
         break;
      case GL_VALIDATE_STATUS:
         params[0] = program->validated ? 1 : 0;
         break;
      case GL_ATTACHED_SHADERS:
         params[0] = (program->vertex != NULL) + (program->fragment != NULL);
         break;
      case GL_INFO_LOG_LENGTH:
         if (program->info_log == NULL)
            params[0] = 0;
         else
            params[0] = strlen(program->info_log) + 1;
         break;
      case GL_ACTIVE_UNIFORMS:
         params[0] = program->num_uniforms;
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
         break;
      }
      case GL_ACTIVE_ATTRIBUTES:
         params[0] = program->num_attributes;
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
         break;
      }
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
   }

   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_APICALL void GL_APIENTRY glGetProgramInfoLog(GLuint p, GLsizei bufsize, GLsizei *length, char *infolog)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (bufsize < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   GL20_PROGRAM_T *program = get_program(state, p);

   if (program == NULL) goto end;

   size_t chars = 0;
   if (program->info_log != NULL)
      chars = strzncpy(infolog, program->info_log, bufsize);

   if (length)
      *length = _max(0, (GLsizei)chars);

end:
   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_APICALL void GL_APIENTRY glGetUniformfv(GLuint p, GLint location, GLfloat *params)
{
   get_uniform_internal(p, location, params, 1);
}

GL_APICALL void GL_APIENTRY glGetUniformiv(GLuint p, GLint location, GLint *params)
{
   get_uniform_internal(p, location, params, 0);
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

GL_APICALL int GL_APIENTRY glGetUniformLocation(GLuint p, const char *name)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return -1;

   GL20_PROGRAM_T *program = get_program(state, p);

   int result = -1;

   if (program == NULL) goto end;

   if (!program->linked) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
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

end:
   glxx_unlock_server_state(OPENGL_ES_20);
   return result;
}

/*
   glIsProgram()

   Returns TRUE if program is the name of a program object. If program is zero,
   or a non-zero value that is not the name of a program object, IsProgram returns
   FALSE. No error is generated if program is not a valid program object name.

   Implementation: Done
   Error Checks: Done
*/

GL_API GLboolean GL_APIENTRY glIsProgram(GLuint p)
{
   GLboolean result;
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return GL_FALSE;

   GL20_PROGRAM_T *program = glxx_shared_get_pobject(state->shared, p);

   if (program == NULL)
      result = GL_FALSE;
   else
      result = gl20_is_program(program);

   glxx_unlock_server_state(OPENGL_ES_20);

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

GL_API GLboolean GL_APIENTRY glIsShader(GLuint s)
{
   GLboolean result;
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return GL_FALSE;

   GL20_SHADER_T *shader = glxx_shared_get_pobject(state->shared, s);

   if (shader == NULL)
      result = GL_FALSE;
   else
      result = gl20_is_shader(shader);

   glxx_unlock_server_state(OPENGL_ES_20);

   return result;
}

GL_API void GL_APIENTRY glLinkProgram(GLuint p)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   GL20_PROGRAM_T *program = get_program(state, p);

   if (program) {
      /* wait to make sure noone is using the buffer */
      khrn_hw_common_wait();

      gl20_program_link(program);

#define TEST_SCHED_AT_LINK_TIME
#ifdef TEST_SCHED_AT_LINK_TIME
      glxx_schedule_during_link(state, program);
#endif

   }

   glxx_unlock_server_state(OPENGL_ES_20);
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

static void uniformv(GLXX_SERVER_STATE_T *state, GLint location, GLsizei num, const void *v, GLint stride, GLboolean is_float, GLboolean is_matrix)
{
   if (location == -1) {
      // check the program object
      if (state->program == NULL)
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return;
   }

   if (state->program == NULL) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return;
   }

   GL20_PROGRAM_T *program = state->program;

   if (!program->linked) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
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

end:
   return;
}

static void uniformv_internal(GLint location, GLsizei num, const void *v, GLint stride, GLboolean is_float, GLboolean is_matrix)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   uniformv(state, location, num, v, stride, is_float, is_matrix);

   glxx_unlock_server_state(OPENGL_ES_20);
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

static int get_uniform(GLXX_SERVER_STATE_T *state, GLuint p, GLint location, const void *v, GLboolean is_float)
{
   GL20_PROGRAM_T *program = get_program(state, p);

   if (program == NULL)
      return 0;

   int num_elements = 0;
   if (!program->linked) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
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

end:
   return num_elements;
}

static int get_uniform_internal(GLuint p, GLint location, const void *v, GLboolean is_float)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state) return 0;

   int num_elements = get_uniform(state, p, location, v, is_float);

   glxx_unlock_server_state(OPENGL_ES_20);
   return num_elements;
}

GL_API void GL_APIENTRY glUniform1i(GLint location, GLint x)
{
   uniformv_internal(location, 1, &x, 1, GL_FALSE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform2i(GLint location, GLint x, GLint y)
{
   GLint v[2];

   v[0] = x; v[1] = y;

   uniformv_internal(location, 1, v, 2, GL_FALSE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform3i(GLint location, GLint x, GLint y, GLint z)
{
   GLint v[3];

   v[0] = x; v[1] = y; v[2] = z;

   uniformv_internal(location, 1, v, 3, GL_FALSE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
   GLint v[4];

   v[0] = x; v[1] = y; v[2] = z; v[3] = w;

   uniformv_internal(location, 1, v, 4, GL_FALSE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform1f(GLint location, GLfloat x)
{
   uniformv_internal(location, 1, &x, 1, GL_TRUE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform2f(GLint location, GLfloat x, GLfloat y)
{
   GLfloat v[2];

   v[0] = x; v[1] = y;

   uniformv_internal(location, 1, v, 2, GL_TRUE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
   GLfloat v[3];

   v[0] = x; v[1] = y; v[2] = z;

   uniformv_internal(location, 1, v, 3, GL_TRUE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GLfloat v[4];

   v[0] = x; v[1] = y; v[2] = z; v[3] = w;

   uniformv_internal(location, 1, v, 4, GL_TRUE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform1iv(GLint location, GLsizei num, const GLint *v)
{
   uniformv_internal(location, num, v, 1, GL_FALSE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform2iv(GLint location, GLsizei num, const GLint *v)
{
   uniformv_internal(location, num, v, 2, GL_FALSE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform3iv(GLint location, GLsizei num, const GLint *v)
{
   uniformv_internal(location, num, v, 3, GL_FALSE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform4iv(GLint location, GLsizei num, const GLint *v)
{
   uniformv_internal(location, num, v, 4, GL_FALSE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform1fv(GLint location, GLsizei num, const GLfloat *v)
{
   uniformv_internal(location, num, v, 1, GL_TRUE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform2fv(GLint location, GLsizei num, const GLfloat *v)
{
   uniformv_internal(location, num, v, 2, GL_TRUE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform3fv(GLint location, GLsizei num, const GLfloat *v)
{
   uniformv_internal(location, num, v, 3, GL_TRUE, GL_FALSE);
}

GL_API void GL_APIENTRY glUniform4fv(GLint location, GLsizei num, const GLfloat *v)
{
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

GL_API void GL_APIENTRY glUniformMatrix2fv(GLint location, GLsizei num, GLboolean transpose, const GLfloat *v)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (!transpose)
      uniformv(state, location, num, v, 4, GL_TRUE, GL_TRUE);
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_API void GL_APIENTRY glUniformMatrix3fv(GLint location, GLsizei num, GLboolean transpose, const GLfloat *v)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (!transpose)
      uniformv(state, location, num, v, 9, GL_TRUE, GL_TRUE);
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_API void GL_APIENTRY glUniformMatrix4fv(GLint location, GLsizei num, GLboolean transpose, const GLfloat *v)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (!transpose)
      uniformv(state, location, num, v, 16, GL_TRUE, GL_TRUE);
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_API void GL_APIENTRY glUseProgram(GLuint p)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (p) {
      GL20_PROGRAM_T *program = get_program(state, p);

      if (program) {
         if (program->linked) {
            gl20_program_acquire(program);

            release_program(state->shared, state->program);

            KHRN_MEM_ASSIGN(state->program, program);
         } else {
            //TODO have I got the reference counts right? I'm new to all this.
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         }
      }
   } else {
      release_program(state->shared, state->program);

      KHRN_MEM_ASSIGN(state->program, NULL);
   }

   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_API void GL_APIENTRY glValidateProgram(GLuint p)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   GL20_PROGRAM_T *program = get_program(state, p);

   if (program) {
      program->validated = gl20_validate_program(state, program);
      free(program->info_log);
      program->info_log = NULL;
   }

   glxx_unlock_server_state(OPENGL_ES_20);
}

static GLboolean is_vertex_attrib_size(GLint size)
{
   return size >= 1 && size <= 4;
}

static GLboolean is_vertex_attrib_type(GLenum type)
{
   return type == GL_BYTE ||
      type == GL_UNSIGNED_BYTE ||
      type == GL_SHORT ||
      type == GL_UNSIGNED_SHORT ||
      type == GL_FLOAT ||
      type == GL_FIXED;
}

GL_APICALL void GL_APIENTRY glVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (indx >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS || !is_vertex_attrib_size(size) || stride < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (is_vertex_attrib_type(type) || type == GL_HALF_FLOAT_OES) {
      state->attrib[indx].size = size;
      state->attrib[indx].type = type;
      state->attrib[indx].normalized = normalized;
      state->attrib[indx].stride = stride;
      state->attrib[indx].pointer = pointer;
      state->attrib[indx].offset = (uintptr_t)pointer;
      state->attrib[indx].buffer = state->bound_buffer.array_name;

      assert(indx < GLXX_CONFIG_MAX_VERTEX_ATTRIBS);
      KHRN_MEM_ASSIGN(state->attrib[indx].attrib, state->bound_buffer.array_buffer);
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

end:
   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_APICALL void GL_APIENTRY glCompileShader(GLuint s)
{
   GL20_SHADER_T *shader;

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   shader = get_shader(state, s);

   if (shader)
      gl20_shader_compile(shader);

   glxx_unlock_server_state(OPENGL_ES_20);
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

GL_APICALL void GL_APIENTRY glGetShaderiv(GLuint s, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   GL20_SHADER_T *shader = get_shader(state, s);

   if (shader) {
      switch (pname) {
      case GL_SHADER_TYPE:
         params[0] = shader->type;
         break;
      case GL_DELETE_STATUS:
         params[0] = shader->deleted ? 1 : 0;
         break;
      case GL_COMPILE_STATUS:
         params[0] = shader->compiled ? 1 : 0;
         break;
      case GL_INFO_LOG_LENGTH:
      {
         if (shader->info_log == NULL)
            params[0] = 0;
         else
            params[0] = strlen(shader->info_log) + 1;
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
         break;
      }
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
   }

   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_APICALL void GL_APIENTRY glGetShaderInfoLog(GLuint s, GLsizei bufsize, GLsizei *length, char *infolog)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   GL20_SHADER_T *shader = get_shader(state, s);
   if (shader == NULL) goto end;

   if (bufsize < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   size_t chars = 0;
   if (shader->info_log != NULL)
      chars = strzncpy(infolog, shader->info_log, bufsize);

   if (length)
      *length = _max(0, (GLsizei)chars);

end:
   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_APICALL void GL_APIENTRY glGetShaderSource(GLuint s, GLsizei bufsize, GLsizei *length, char *source)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   uint32_t charswritten = 0;

   GL20_SHADER_T *shader = get_shader(state, s);
   if (shader == NULL) goto end;       /* get_shader will have set error */

   if (bufsize < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
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

   if (length) {
      *length = charswritten;
   }
   if (bufsize > 0) {
      assert(charswritten < (uint32_t)bufsize);
      source[charswritten] = 0;
   }

end:
   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_APICALL void GL_APIENTRY glShaderSource(GLuint s, GLsizei count, const char *const*string, const GLint *length)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (count < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   GL20_SHADER_T *shader = get_shader(state, s);
   if (shader == NULL) goto end;       /* get_shader will have set error */

   if (string == NULL)
      goto end;

   if (!gl20_shader_set_source(shader, count, string, length))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

   glxx_tweaker_setshadersource(&state->tweak_state, count, string, length);

end:
   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_APICALL void GL_APIENTRY glReleaseShaderCompiler(void)
{
}

GL_APICALL void GL_APIENTRY glShaderBinary(GLint n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLint length)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   UNUSED(n);
   UNUSED(shaders);
   UNUSED(binaryformat);
   UNUSED(binary);
   UNUSED(length);

   glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_20);
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

GL_APICALL void GL_APIENTRY glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (is_shader_type(shadertype) && is_precision_type(precisiontype)) {
      if (is_float_type(precisiontype)) {
         if (range) {
            range[0] = 127;
            range[1] = 127;
         }
         if (precision)
            *precision = 23;
      }
      else {
         if (range) {
            range[0] = 31;
            range[1] = 30;
         }
         if (precision)
            *precision = 0;
      }
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_20);
}

static void enable_vertex_attrib_array(GLuint index, bool enabled)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (index < GLXX_CONFIG_MAX_VERTEX_ATTRIBS)
      state->attrib[index].enabled = enabled;
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_APICALL void GL_APIENTRY glEnableVertexAttribArray(GLuint index)
{
   enable_vertex_attrib_array(index, true);
}

GL_APICALL void GL_APIENTRY glDisableVertexAttribArray(GLuint index)
{
   enable_vertex_attrib_array(index, false);
}

GL_APICALL void GL_APIENTRY glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   switch (pname) {
   case GL_CURRENT_VERTEX_ATTRIB:
      params[0] = state->attrib[index].value[0];
      params[1] = state->attrib[index].value[1];
      params[2] = state->attrib[index].value[2];
      params[3] = state->attrib[index].value[3];
      break;

      //TODO: is this the best way to handle conversions? We duplicate
      //the entire switch statement.
   case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
      params[0] = state->attrib[index].enabled ? 1.0f : 0.0f;
      break;
   case GL_VERTEX_ATTRIB_ARRAY_SIZE:
      params[0] = (GLfloat)state->attrib[index].size;
      break;
   case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
      params[0] = (GLfloat)state->attrib[index].stride;
      break;
   case GL_VERTEX_ATTRIB_ARRAY_TYPE:
      params[0] = (GLfloat)state->attrib[index].type;
      break;
   case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
      params[0] = state->attrib[index].normalized ? 1.0f : 0.0f;
      break;
   case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
      params[0] = (GLfloat)state->attrib[index].buffer;
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

end:
   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_APICALL void GL_APIENTRY glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   switch (pname) {
   case GL_CURRENT_VERTEX_ATTRIB:
      params[0] = (GLint)state->attrib[index].value[0];
      params[1] = (GLint)state->attrib[index].value[1];
      params[2] = (GLint)state->attrib[index].value[2];
      params[3] = (GLint)state->attrib[index].value[3];
      break;
   case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
      params[0] = (GLint)state->attrib[index].enabled ? GL_TRUE : GL_FALSE;
      break;
   case GL_VERTEX_ATTRIB_ARRAY_SIZE:
      params[0] = (GLint)state->attrib[index].size;
      break;
   case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
      params[0] = (GLint)state->attrib[index].stride;
      break;
   case GL_VERTEX_ATTRIB_ARRAY_TYPE:
      params[0] = (GLint)state->attrib[index].type;
      break;
   case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
      params[0] = (GLint)state->attrib[index].normalized ? GL_TRUE : GL_FALSE;
      break;
   case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
      params[0] = (GLint)state->attrib[index].buffer;
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

end:
   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_APICALL void GL_APIENTRY glGetVertexAttribPointerv(GLuint index, GLenum pname, void **pointer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   switch (pname) {
   case GL_VERTEX_ATTRIB_ARRAY_POINTER:
      pointer[0] = (void *)state->attrib[index].pointer;
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

end:
   glxx_unlock_server_state(OPENGL_ES_20);
}

static void vertex_attrib_4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   if (indx >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

      state->attrib[indx].value[0] = clean_float(x);
      state->attrib[indx].value[1] = clean_float(y);
      state->attrib[indx].value[2] = clean_float(z);
      state->attrib[indx].value[3] = clean_float(w);

end:
   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_APICALL void GL_APIENTRY glVertexAttrib1f(GLuint indx, GLfloat x)
{
   vertex_attrib_4f(indx, x, 0.0f, 0.0f, 1.0f);
}

GL_APICALL void GL_APIENTRY glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y)
{
   vertex_attrib_4f(indx, x, y, 0.0f, 1.0f);
}

GL_APICALL void GL_APIENTRY glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
   vertex_attrib_4f(indx, x, y, z, 1.0f);
}

GL_APICALL void GL_APIENTRY glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   vertex_attrib_4f(indx, x, y, z, w);
}

GL_APICALL void GL_APIENTRY glVertexAttrib1fv(GLuint indx, const GLfloat *values)
{
   vertex_attrib_4f(indx, values[0], 0.0f, 0.0f, 1.0f);
}

GL_APICALL void GL_APIENTRY glVertexAttrib2fv(GLuint indx, const GLfloat *values)
{
   vertex_attrib_4f(indx, values[0], values[1], 0.0f, 1.0f);
}

GL_APICALL void GL_APIENTRY glVertexAttrib3fv(GLuint indx, const GLfloat *values)
{
   vertex_attrib_4f(indx, values[0], values[1], values[2], 1.0f);
}

GL_APICALL void GL_APIENTRY glVertexAttrib4fv(GLuint indx, const GLfloat *values)
{
   vertex_attrib_4f(indx, values[0], values[1], values[2], values[3]);
}

GL_API void GL_APIENTRY glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   glxx_server_set_stencil_func(state, face, func, ref, mask);

   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_API void GL_APIENTRY glStencilMaskSeparate(GLenum face, GLuint mask) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   glxx_server_set_stencil_mask(state, face, mask);

   glxx_unlock_server_state(OPENGL_ES_20);
}

GL_API void GL_APIENTRY glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_20);
   if (!state)
      return;

   glxx_server_set_stencil_op(state, face, fail, zfail, zpass);

   glxx_unlock_server_state(OPENGL_ES_20);
}
