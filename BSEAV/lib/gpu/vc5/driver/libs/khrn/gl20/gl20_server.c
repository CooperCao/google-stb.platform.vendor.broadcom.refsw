/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../common/khrn_int_common.h"
#include "../common/khrn_int_util.h"

#include "libs/util/dglenum/dglenum.h"

#include "../glxx/glxx_server.h"
#include "../glxx/glxx_server_internal.h"
#include "../glxx/glxx_shared.h"
#include "../glxx/glxx_translate.h"

#include "gl20_program.h"
#include "gl20_shader.h"

#include "../glxx/glxx_server_program_interface.h"

#include "libs/util/snprintf.h"

#include <string.h>
#include <math.h>
#include <limits.h>

LOG_DEFAULT_CAT("gl20_server")

static void get_uniform_internal(GLuint p, GLint location, GLsizei buf_size, void *v, GLenum to_type);

bool gl20_server_state_init(GLXX_SERVER_STATE_T *state, GLXX_SHARED_T *shared)
{
   //initialise common portions of state
   if(!glxx_server_state_init(state, shared))
      return false;

   //gl 2.0 specific parts
   assert(state->current_program == NULL);

   return true;
}


////

/*
   Get a program pointer from a program name.  Gives GL_INVALID_VALUE if no
   object exists with that name, or GL_INVALID_OPERATION if the object is
   actually a shader and returns NULL in either case.
*/

GL20_PROGRAM_T *gl20_get_program(GLXX_SERVER_STATE_T *state, GLuint p)
{
   GL20_PROGRAM_T *program = glxx_shared_get_pobject(state->shared, p);

   if (program == NULL) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

      return NULL;
   }

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
*/
GL_APICALL void GL_APIENTRY glAttachShader(GLuint p, GLuint s)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state) return;

   GL20_PROGRAM_T *program = gl20_get_program(state, p);
   if (!program) goto end;

   GL20_SHADER_T *shader = get_shader(state, s);
   if (!shader) goto end;

   if (!gl20_program_attach_shader(program, shader))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

end:
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glBindAttribLocation(GLuint p, GLuint index, const char *name)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state) return;

   if (!name) goto end;

   if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (!strncmp(name, "gl_", 3)) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   GL20_PROGRAM_T *program = gl20_get_program(state, p);

   if (program) {
      if (!gl20_program_bind_attrib(program, index, name))
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
   }

end:
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   state->blend_color[0] = gfx_fclamp(red, 0.0f, 1.0f);
   state->blend_color[1] = gfx_fclamp(green, 0.0f, 1.0f);
   state->blend_color[2] = gfx_fclamp(blue, 0.0f, 1.0f);
   state->blend_color[3] = gfx_fclamp(alpha, 0.0f, 1.0f);

   state->dirty.blend_color = KHRN_RENDER_STATE_SET_ALL;

   glxx_unlock_server_state();
}

static void set_blend_eqn(GLXX_SERVER_STATE_T *state, glxx_blend_cfg *cfg,
                          glxx_blend_eqn_t ec, glxx_blend_eqn_t ea)
{
   if (cfg->color_eqn != ec) {
      state->dirty.blend_cfg = KHRN_RENDER_STATE_SET_ALL;
      /* Changing into/out of advanced blend means changing enable/disable state
       * for blending, so we need to set dirty.cfg as well as .blend_cfg */
      if ((cfg->color_eqn & GLXX_ADV_BLEND_EQN_BIT) != (ec & GLXX_ADV_BLEND_EQN_BIT))
         state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;

      cfg->color_eqn = ec;
   }
   if (cfg->alpha_eqn != ea)
   {
      state->dirty.blend_cfg = KHRN_RENDER_STATE_SET_ALL;
      if ((cfg->alpha_eqn & GLXX_ADV_BLEND_EQN_BIT) != (ea & GLXX_ADV_BLEND_EQN_BIT))
         state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;

      cfg->alpha_eqn = ea;
   }
}

static void set_all_blend_eqns(GLXX_SERVER_STATE_T *state, glxx_blend_eqn_t ec, glxx_blend_eqn_t ea)
{
#if V3D_VER_AT_LEAST(4,0,2,0)
   for (unsigned i = 0; i != GLXX_MAX_RENDER_TARGETS; ++i)
      set_blend_eqn(state, &state->blend.rt_cfgs[i], ec, ea);
#else
   set_blend_eqn(state, &state->blend.cfg, ec, ea);
#endif
}

static bool is_valid_separate_blend(glxx_blend_eqn_t e)
{
   return e != GLXX_BLEND_EQN_INVALID && !(e & GLXX_ADV_BLEND_EQN_BIT);
}

GL_APICALL void GL_APIENTRY glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   glxx_blend_eqn_t ec = translate_blend_equation(modeRGB);
   glxx_blend_eqn_t ea = translate_blend_equation(modeAlpha);

   if (is_valid_separate_blend(ec) && is_valid_separate_blend(ea))
      set_all_blend_eqns(state, ec, ea);
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glBlendBarrier(void)
{
   /* Nothing to do */
}

GL_APICALL void GL_APIENTRY glBlendBarrierKHR(void)
{
   /* Nothing to do */
}

GL_APICALL void GL_APIENTRY glBlendEquation(GLenum mode) // S
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   glxx_blend_eqn_t eq = translate_blend_equation(mode);

   if (eq != GLXX_BLEND_EQN_INVALID)
      set_all_blend_eqns(state, eq, eq);
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

#if V3D_VER_AT_LEAST(4,0,2,0)

static void blend_equation_separate_i(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (buf >= GLXX_MAX_RENDER_TARGETS)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else
   {
      glxx_blend_eqn_t ec = translate_blend_equation(modeRGB);
      glxx_blend_eqn_t ea = translate_blend_equation(modeAlpha);

      if (is_valid_separate_blend(ec) && is_valid_separate_blend(ea))
         set_blend_eqn(state, &state->blend.rt_cfgs[buf], ec, ea);
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

   glxx_unlock_server_state();
}

static void blend_equation_i(GLuint buf, GLenum mode)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (buf >= GLXX_MAX_RENDER_TARGETS)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else
   {
      glxx_blend_eqn_t eq = translate_blend_equation(mode);

      if (eq != GLXX_BLEND_EQN_INVALID)
         set_blend_eqn(state, &state->blend.rt_cfgs[buf], eq, eq);
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glBlendEquationSeparateiEXT(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
   blend_equation_separate_i(buf, modeRGB, modeAlpha);
}

GL_APICALL void GL_APIENTRY glBlendEquationiEXT(GLuint buf, GLenum mode)
{
   blend_equation_i(buf, mode);
}

GL_APICALL void GL_APIENTRY glBlendEquationSeparateiOES(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
   blend_equation_separate_i(buf, modeRGB, modeAlpha);
}

GL_APICALL void GL_APIENTRY glBlendEquationiOES(GLuint buf, GLenum mode)
{
   blend_equation_i(buf, mode);
}

#endif

#if KHRN_GLES32_DRIVER

GL_APICALL void GL_APIENTRY glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
#if V3D_VER_AT_LEAST(4,0,2,0)
   blend_equation_separate_i(buf, modeRGB, modeAlpha);
#endif
}

GL_APICALL void GL_APIENTRY glBlendEquationi(GLuint buf, GLenum mode)
{
#if V3D_VER_AT_LEAST(4,0,2,0)
   blend_equation_i(buf, mode);
#endif
}

#endif

/*
   glCreateProgram()

   Creates a new, empty program and returns its name. No errors are generated.

   Implementation: Done
   Error Checks: Done
*/

GL_APICALL GLuint GL_APIENTRY glCreateProgram(void)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state) return 0;

   GLuint result = glxx_shared_create_program(state->shared);

   if (result == 0)
   {
      log_warn("glCreateProgram() = 0 fail. Setting GL_OUT_OF_MEMORY");
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
   }

   glxx_unlock_server_state();

   return result;
}

/*
   glCreateShader()

   Creates a new, empty shader and returns its name. Gives GL_INVALID_ENUM if
   type is not a valid shader type.
*/

GL_APICALL GLuint GL_APIENTRY glCreateShader(GLenum type)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   GLuint result = 0;
   if (!state) return 0;

   if (!glxx_is_shader_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   result = glxx_shared_create_shader(state->shared, type);

   if (result == 0)
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

end:
   glxx_unlock_server_state();
   return result;
}



void gl20_server_try_delete_shader(GLXX_SHARED_T *shared, GL20_SHADER_T *shader)
{
   if (shader->refs == 0 && shader->deleted)
      glxx_shared_delete_pobject(shared, shader->name);
}

static void release_shader(GLXX_SHARED_T *shared, GL20_SHADER_T *shader)
{
   if (shader == NULL) return;

   assert(gl20_is_shader(shader));

   gl20_shader_release(shader);

   gl20_server_try_delete_shader(shared, shader);
}

void gl20_server_try_delete_program(GLXX_SHARED_T *shared, GL20_PROGRAM_T *program)
{
   if (program->refs == 0 && program->deleted) {
      release_shader(shared, program->vertex);
      release_shader(shared, program->fragment);
      release_shader(shared, program->compute);
   #if GLXX_HAS_TNG
      release_shader(shared, program->tess_control);
      release_shader(shared, program->tess_evaluation);
      release_shader(shared, program->geometry);
   #endif

      glxx_shared_delete_pobject(shared, program->name);
   }
}

static void release_program(GLXX_SHARED_T *shared, khrn_mem_handle_t handle)
{
   if (handle != KHRN_MEM_HANDLE_INVALID) {
      GL20_PROGRAM_T *program;

      khrn_mem_acquire(handle);

      program = (GL20_PROGRAM_T *)khrn_mem_lock(handle);

      assert(gl20_is_program(program));

      gl20_program_release(program);

      gl20_server_try_delete_program(shared, program);

      khrn_mem_unlock(handle);
      khrn_mem_release(handle);
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

GL_APICALL void GL_APIENTRY glDeleteProgram(GLuint p)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (p == 0) goto end;

   khrn_mem_handle_t h_program = glxx_shared_get_pobject(state->shared, p);
   if (h_program == KHRN_MEM_HANDLE_INVALID) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   khrn_mem_acquire(h_program);
   GL20_PROGRAM_T *program = (GL20_PROGRAM_T *)khrn_mem_lock(h_program);

   if (gl20_is_program(program)) {
      program->deleted = GL_TRUE;

      gl20_server_try_delete_program(state->shared, program);
   } else
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);

   khrn_mem_unlock(h_program);
   khrn_mem_release(h_program);

end:
   glxx_unlock_server_state();
}

/*
   glDeleteShader()

   Deletes a specified shader. If the shader is currently attached to a program,
   the shader is marked as pending deletion. Gives GL_INVALID_VALUE if the argument
   is neither a program nor a shader, or GL_INVALID_OPERATION if the argument is a
   program.
*/
GL_APICALL void GL_APIENTRY glDeleteShader(GLuint s)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if (s) {
      khrn_mem_handle_t handle = glxx_shared_get_pobject(state->shared, s);

      if (handle != KHRN_MEM_HANDLE_INVALID) {
         GL20_SHADER_T *shader;

         khrn_mem_acquire(handle);

         shader = (GL20_SHADER_T *)khrn_mem_lock(handle);

         if (gl20_is_shader(shader)) {
            shader->deleted = GL_TRUE;

            gl20_server_try_delete_shader(state->shared, shader);
         } else
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);

         khrn_mem_unlock(handle);
         khrn_mem_release(handle);
      } else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
   }

   glxx_unlock_server_state();
}

/*
   glDetachShader()

   Detaches a shader from a program. If the shader is marked as pending deletion, and
   is not attached to another program, it is deleted. Gives GL_INVALID_VALUE if the
   program or shader does not exist, or GL_INVALID_OPERATION if the program argument
   is not a program, the shader argument is not a shader, or the shader is not attached
   to the program.
*/
GL_APICALL void GL_APIENTRY glDetachShader(GLuint p, GLuint s)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   GL20_PROGRAM_T *program = gl20_get_program(state, p);
   if (!program) goto end;

   GL20_SHADER_T *shader = get_shader(state, s);
   if (!shader) goto end;

   if (!gl20_program_detach_shader(program, shader)) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   gl20_server_try_delete_shader(state->shared, shader);

end:
   glxx_unlock_server_state();
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
   Gets the name, size and type of a specified attribute of a program. Gives
   GL_INVALID_VALUE if the program does not exist, or GL_INVALID_OPERATION
   if the program argument is actually a shader. Also gives GL_INVALID_VALUE
   if the specified index is greater than the number of attributes of the
   linked program.
*/
GL_APICALL void GL_APIENTRY glGetActiveAttrib(GLuint p, GLuint index, GLsizei buf_len, GLsizei *length, GLint *size, GLenum *type, char *buf_ptr)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state) return;

   GL20_PROGRAM_T *program = gl20_get_program(state, p);
   if (!program) goto end;

   if (!program->common.linked_glsl_program ||
       program->common.linked_glsl_program->ir->stage[SHADER_VERTEX].ir == NULL ||
       index >= program->common.linked_glsl_program->num_inputs)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (buf_len < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   GLSL_INOUT_T *attr = &program->common.linked_glsl_program->inputs[index];
   assert(attr->name);

   size_t chars = strzncpy(buf_ptr, attr->name, buf_len);

   if (length) *length = (GLsizei)chars;
   if (size) *size = 1;        // no array or structure attributes
   if (type) *type = attr->type;

end:
   glxx_unlock_server_state();
}

const GLSL_BLOCK_T *gl20_get_ubo_from_index(const GLSL_PROGRAM_T *p, unsigned int index)
{
   for (unsigned i=0; i < p->num_uniform_blocks; i++) {
      const GLSL_BLOCK_T *b = &p->uniform_blocks[i];

      if (index < (unsigned int)(b->index + b->array_length))
         return b;
   }

   return NULL;
}

const GLSL_BLOCK_T *gl20_get_ssbo_from_index(const GLSL_PROGRAM_T *p, unsigned int index)
{
   for (unsigned i=0; i < p->num_buffer_blocks; i++) {
      const GLSL_BLOCK_T *b = &p->buffer_blocks[i];

      if (index < (unsigned int)(b->index + b->array_length))
         return b;
   }

   return NULL;
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
GL_APICALL void GL_APIENTRY glGetActiveUniform(GLuint p, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state)
      return;

   // Section 7.6 of the spec says this is equivalent to:
   const GLenum props[] = { GL_ARRAY_SIZE, GL_TYPE };
   glxx_get_program_resource_name(state, p, GL_UNIFORM, index, bufSize, length, name);
   if (size != NULL)
      glxx_get_program_resourceiv(state, p, GL_UNIFORM, index, 1,  &props[0], 1, NULL, size);
   if (type != NULL)
      glxx_get_program_resourceiv(state, p, GL_UNIFORM, index, 1,  &props[1], 1, NULL, (int*)type);

   glxx_unlock_server_state();
}

/*
   glGetAttachedShaders()

   Gets the names of the shaders attached to a specified program. Gives
   GL_INVALID_VALUE if the program does not exist, or GL_INVALID_OPERATION
   if the program argument is actually a shader.
*/

GL_APICALL void GL_APIENTRY glGetAttachedShaders(GLuint p, GLsizei maxcount, GLsizei *pcount, GLuint *shaders)
{
   GL20_PROGRAM_T *program;
   int32_t count = 0;
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state) return;

   program = gl20_get_program(state, p);
   if (program == NULL)
      goto end;      /* get_program will have set the correct error */

   if (maxcount < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (shaders) {
      GL20_SHADER_T *attached[] = { program->vertex,
#if GLXX_HAS_TNG
                                    program->tess_control, program->tess_evaluation, program->geometry,
#endif
                                    program->fragment, program->compute };

      for (int i=0; i<vcos_countof(attached); i++) {
         if (maxcount > 0) {
            if (attached[i] != NULL) {
               shaders[count++] = attached[i]->name;
               maxcount--;
            }
         }
      }
   }

   if (pcount)
      *pcount = count;

end:
   glxx_unlock_server_state();
}

GL_APICALL GLint GL_APIENTRY glGetAttribLocation(GLuint p, const char *name)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   GL20_PROGRAM_T *program = NULL;
   int result = -1;
   if (!state) return 0;

   program = gl20_get_program(state, p);
   if (!program) goto end;

   if (name == NULL) goto end;

   if (!program->common.linked_glsl_program) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   // Section 11.1 of the spec says this is now equivalent to:
   result = glxx_get_program_resource_location(state, p, GL_PROGRAM_INPUT, name);

end:
   glxx_unlock_server_state();
   return result;
}

GL_APICALL GLint GL_APIENTRY glGetFragDataLocation(GLuint p, const char *name) {
   GLXX_SERVER_STATE_T *state;
   GL20_PROGRAM_T *program = NULL;
   int result = -1;

   state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state) return result;

   program = gl20_get_program(state, p);
   if (!program) goto end;

   if (name == NULL) goto end;

   if (!program->common.linked_glsl_program) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   // Section 14.2 of the spec says this is now equivalent to:
   if (program->common.linked_glsl_program->ir->stage[SHADER_FRAGMENT].ir == NULL)
      result = -1;
   else
      result = glxx_get_program_resource_location(state, p, GL_PROGRAM_OUTPUT, name);

end:
   glxx_unlock_server_state();
   return result;
}

GL_APICALL void GL_APIENTRY glGetProgramiv(GLuint p, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state) return;

   GL20_PROGRAM_T *program = gl20_get_program(state, p);
   if (!program) goto end;                /* get_program will set the error */

   GLSL_PROGRAM_T *linked_program = program->common.linked_glsl_program;
   switch (pname)
   {
   case GL_DELETE_STATUS:
      params[0] = program->deleted ? 1 : 0;
      break;
   case GL_LINK_STATUS:
      params[0] = program->common.link_status ? 1 : 0;
      break;
   case GL_VALIDATE_STATUS:
      params[0] = program->validated ? 1 : 0;
      break;
   case GL_ATTACHED_SHADERS:
      params[0] = (program->vertex != NULL) + (program->fragment != NULL) + (program->compute != NULL);
#if GLXX_HAS_TNG
      params[0] += (program->tess_control != NULL) + (program->tess_evaluation != NULL) + (program->geometry != NULL);
#endif
      break;
   case GL_INFO_LOG_LENGTH:
      params[0] = khrn_mem_get_size(program->mh_info);
      break;
   case GL_ACTIVE_UNIFORM_BLOCKS:
      if (!linked_program || linked_program->num_uniform_blocks == 0) {
         params[0] = 0;
      } else {
         GLSL_BLOCK_T *last_block;
         last_block = &linked_program->uniform_blocks[linked_program->num_uniform_blocks-1];
         params[0] = last_block->index + last_block->array_length;
      }
      break;
   case GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH:
      if (!linked_program) params[0] = 0;
      else                 params[0] = glxx_get_max_name_length(linked_program, GL_UNIFORM_BLOCK);
      break;
   case GL_ACTIVE_UNIFORMS:
      if(!linked_program) {
         params[0] = 0;
      } else {
         unsigned count;
         unsigned int i;
         count = linked_program->default_uniforms.num_members;
         for (i=0; i<linked_program->num_uniform_blocks; i++) {
            count += linked_program->uniform_blocks[i].num_members;
         }
         params[0] = count;
      }
      break;
   case GL_ACTIVE_UNIFORM_MAX_LENGTH:
      if(!linked_program) {
         params[0] = 0;
      } else {
         params[0] = glxx_get_max_name_length(linked_program, GL_UNIFORM);
      }
      break;
   case GL_ACTIVE_ATTRIBUTES:
      if (!linked_program || !glsl_program_has_stage(linked_program, SHADER_VERTEX)) {
         params[0] = 0;
      } else {
         params[0] = linked_program->num_inputs;
      }
      break;
   case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
      if (!linked_program || !glsl_program_has_stage(linked_program, SHADER_VERTEX))
         params[0] = 0;
      else
         params[0] = glxx_get_max_name_length(linked_program, GL_PROGRAM_INPUT);
      break;
   case GL_TRANSFORM_FEEDBACK_BUFFER_MODE:
      params[0] = program->transform_feedback.buffer_mode;
      break;
   case GL_TRANSFORM_FEEDBACK_VARYINGS:
      params[0] = linked_program ? program->common.transform_feedback.varying_count : 0u;
      break;
   case GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH:
      if (!linked_program)
         params[0] = 0;
      else
         params[0] = glxx_get_max_name_length(linked_program, GL_TRANSFORM_FEEDBACK_VARYING);
      break;
   case GL_PROGRAM_BINARY_RETRIEVABLE_HINT:
      params[0] = program->binary_hint;
      break;
   case GL_PROGRAM_BINARY_LENGTH:
      params[0] = 0;
      break;

#if KHRN_GLES31_DRIVER
   case GL_ACTIVE_ATOMIC_COUNTER_BUFFERS:
      params[0] = linked_program ? linked_program->num_atomic_buffers : 0u;
      break;
   case GL_PROGRAM_SEPARABLE:
      params[0] = program->common.separable ? GL_TRUE : GL_FALSE;
      break;
   case GL_COMPUTE_WORK_GROUP_SIZE:
      if (!linked_program || !glsl_program_has_stage(linked_program, SHADER_COMPUTE))
         goto invalid_op;
      params[0] = linked_program->ir->cs_wg_size[0];
      params[1] = linked_program->ir->cs_wg_size[1];
      params[2] = linked_program->ir->cs_wg_size[2];
      break;
#endif

#if GLXX_HAS_TNG
   case GL_TESS_CONTROL_OUTPUT_VERTICES:
      if (!linked_program || !glsl_program_has_stage(linked_program, SHADER_TESS_CONTROL))
         goto invalid_op;
      params[0] = linked_program->ir->tess_vertices;
      break;
   case GL_TESS_GEN_MODE:
      if (!linked_program || !glsl_program_has_stage(linked_program, SHADER_TESS_EVALUATION))
         goto invalid_op;
      switch (linked_program->ir->tess_mode)
      {
      case TESS_ISOLINES:  params[0] = GL_ISOLINES; break;
      case TESS_TRIANGLES: params[0] = GL_TRIANGLES; break;
      case TESS_QUADS:     params[0] = GL_QUADS; break;
      default: unreachable();
      }
      break;
   case GL_TESS_GEN_SPACING:
      if (!linked_program || !glsl_program_has_stage(linked_program, SHADER_TESS_EVALUATION))
         goto invalid_op;
      switch (linked_program->ir->tess_spacing)
      {
      case TESS_SPACING_EQUAL:      params[0] = GL_EQUAL; break;
      case TESS_SPACING_FRACT_EVEN: params[0] = GL_FRACTIONAL_EVEN; break;
      case TESS_SPACING_FRACT_ODD:  params[0] = GL_FRACTIONAL_ODD; break;
      default: unreachable();
      }
      break;
   case GL_TESS_GEN_VERTEX_ORDER:
      if (!linked_program || !glsl_program_has_stage(linked_program, SHADER_TESS_EVALUATION))
         goto invalid_op;
      params[0] = linked_program->ir->tess_cw ? GL_CW : GL_CCW;
      break;
   case GL_TESS_GEN_POINT_MODE:
      if (!linked_program || !glsl_program_has_stage(linked_program, SHADER_TESS_EVALUATION))
         goto invalid_op;
      params[0] = linked_program->ir->tess_point_mode;
      break;
   case GL_GEOMETRY_VERTICES_OUT:
      if (!linked_program || !glsl_program_has_stage(linked_program, SHADER_GEOMETRY))
         goto invalid_op;
      params[0] = linked_program->ir->gs_max_vertices;
      break;
   case GL_GEOMETRY_INPUT_TYPE:
      if (!linked_program || !glsl_program_has_stage(linked_program, SHADER_GEOMETRY))
         goto invalid_op;
      switch (linked_program->ir->gs_in) {
         case GS_IN_POINTS:    params[0] = GL_POINTS;              break;
         case GS_IN_LINES:     params[0] = GL_LINES;               break;
         case GS_IN_TRIANGLES: params[0] = GL_TRIANGLES;           break;
         case GS_IN_LINES_ADJ: params[0] = GL_LINES_ADJACENCY;     break;
         case GS_IN_TRIS_ADJ:  params[0] = GL_TRIANGLES_ADJACENCY; break;
         default: unreachable();
      }
      break;
   case GL_GEOMETRY_OUTPUT_TYPE:
      if (!linked_program || !glsl_program_has_stage(linked_program, SHADER_GEOMETRY))
         goto invalid_op;
      switch (linked_program->ir->gs_out) {
         case GS_OUT_POINTS:     params[0] = GL_POINTS;         break;
         case GS_OUT_LINE_STRIP: params[0] = GL_LINE_STRIP;     break;
         case GS_OUT_TRI_STRIP:  params[0] = GL_TRIANGLE_STRIP; break;
         default: unreachable();
      }
      break;
   case GL_GEOMETRY_SHADER_INVOCATIONS:
      if (!linked_program || !glsl_program_has_stage(linked_program, SHADER_GEOMETRY))
         goto invalid_op;
      params[0] = linked_program->ir->gs_n_invocations;
      break;
#endif

   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }
   goto end;

#if KHRN_GLES31_DRIVER
invalid_op:
   glxx_server_state_set_error(state, GL_INVALID_OPERATION);
#endif
end:
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glGetProgramInfoLog(GLuint p, GLsizei bufsize, GLsizei *length, char *infolog)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   GL20_PROGRAM_T *program;
   if (!state) return;

   program = gl20_get_program(state, p);
   if (program == NULL) goto end;   /* get_program will have set error */

   if (bufsize < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   {
      size_t chars = strzncpy(infolog, (const char *)khrn_mem_lock(program->mh_info), bufsize);
      khrn_mem_unlock(program->mh_info);

      if (length)
         *length = gfx_smax(0, (GLsizei)chars);
   }

end:
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glGetUniformfv(GLuint p, GLint location, GLfloat *params)
{
   get_uniform_internal(p, location, INT_MAX, params, GL_FLOAT);
}

GL_APICALL void GL_APIENTRY glGetnUniformfvEXT(GLuint p, GLint location, GLsizei bufSize, GLfloat *params)
{
   get_uniform_internal(p, location, bufSize, params, GL_FLOAT);
}

GL_APICALL void GL_APIENTRY glGetUniformiv(GLuint p, GLint location, GLint *params)
{
   get_uniform_internal(p, location, INT_MAX, params, GL_INT);
}

GL_APICALL void GL_APIENTRY glGetnUniformivEXT(GLuint p, GLint location, GLsizei bufSize, GLint *params)
{
   get_uniform_internal(p, location, bufSize, params, GL_INT);
}

GL_APICALL void GL_APIENTRY glGetUniformuiv(GLuint p, GLint location, GLuint *params)
{
   get_uniform_internal(p, location, INT_MAX, params, GL_UNSIGNED_INT);
}

GL_APICALL void GL_APIENTRY glGetnUniformuivEXT(GLuint p, GLint location, GLsizei bufSize, GLuint *params)
{
   get_uniform_internal(p, location, bufSize, params, GL_UNSIGNED_INT);
}

#if KHRN_GLES32_DRIVER
GL_APICALL void GL_APIENTRY glGetnUniformfv(GLuint p, GLint location, GLsizei bufSize, GLfloat *params)
{
   get_uniform_internal(p, location, bufSize, params, GL_FLOAT);
}

GL_APICALL void GL_APIENTRY glGetnUniformiv(GLuint p, GLint location, GLsizei bufSize, GLint *params)
{
   get_uniform_internal(p, location, bufSize, params, GL_INT);
}

GL_APICALL void GL_APIENTRY glGetnUniformuiv(GLuint p, GLint location, GLsizei bufSize, GLuint *params)
{
   get_uniform_internal(p, location, bufSize, params, GL_UNSIGNED_INT);
}
#endif

GL_APICALL GLint GL_APIENTRY glGetUniformLocation(GLuint p, const char *name)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   GLint               loc = -1;
   if (!state)
      return loc;

   // Section 7.6 of the spec says this is equivalent to:
   loc = glxx_get_program_resource_location(state, p, GL_UNIFORM, name);

   glxx_unlock_server_state();

   return loc;
}

/*
   glIsProgram()

   Returns TRUE if program is the name of a program object. If program is zero,
   or a non-zero value that is not the name of a program object, IsProgram returns
   FALSE. No error is generated if program is not a valid program object name.

   Implementation: Done
   Error Checks: Done
*/

GL_APICALL GLboolean GL_APIENTRY glIsProgram(GLuint p)
{
   GLboolean result;
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   khrn_mem_handle_t handle;
   if (!state) return 0;

   handle = glxx_shared_get_pobject(state->shared, p);

   if (handle == KHRN_MEM_HANDLE_INVALID)
      result = GL_FALSE;
   else {
      result = gl20_is_program((GL20_PROGRAM_T *)khrn_mem_lock(handle));
      khrn_mem_unlock(handle);
   }

   glxx_unlock_server_state();

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

GL_APICALL GLboolean GL_APIENTRY glIsShader(GLuint s)
{
   GLboolean result;
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   khrn_mem_handle_t handle;
   if (!state) return 0;

   handle  = glxx_shared_get_pobject(state->shared, s);

   if (handle == KHRN_MEM_HANDLE_INVALID)
      result = GL_FALSE;
   else {
      result = gl20_is_shader((GL20_SHADER_T *)khrn_mem_lock(handle));
      khrn_mem_unlock(handle);
   }

   glxx_unlock_server_state();

   return result;
}




GL_APICALL void GL_APIENTRY glLinkProgram(GLuint p)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   GL20_PROGRAM_T *program;

   if (!state) return;

   program = gl20_get_program(state, p);
   if (!program) goto end;

   if (glxx_server_program_used_by_any_tf(state, program)) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
   } else {
      gl20_program_link(program);

      /* Invalidate the program cache because the source has changed */
      glxx_binary_cache_invalidate(&program->common.cache);
   }

end:
   glxx_unlock_server_state();
}

static bool is_bool_type(GLenum type)
{
   switch (type) {
   case GL_BOOL:
   case GL_BOOL_VEC2:
   case GL_BOOL_VEC3:
   case GL_BOOL_VEC4:
      return true;
   default:
      return false;
   }
}

static bool is_float_type(GLenum type)
{
   switch (type) {
   case GL_FLOAT:
   case GL_FLOAT_VEC2:
   case GL_FLOAT_VEC3:
   case GL_FLOAT_VEC4:
   case GL_FLOAT_MAT2:
   case GL_FLOAT_MAT3:
   case GL_FLOAT_MAT4:
   case GL_FLOAT_MAT2x3:
   case GL_FLOAT_MAT2x4:
   case GL_FLOAT_MAT3x2:
   case GL_FLOAT_MAT3x4:
   case GL_FLOAT_MAT4x2:
   case GL_FLOAT_MAT4x3:
      return true;
   default:
      return false;
   }
}

static bool is_signed_int_type(GLenum type)
{
   switch (type)
   {
   case GL_INT:
   case GL_INT_VEC2:
   case GL_INT_VEC3:
   case GL_INT_VEC4:
   case GL_SAMPLER_2D:
      return true;
   default:
      return false;
   }
}

static bool is_unsigned_int_type(GLenum type)
{
   switch (type)
   {
   case GL_UNSIGNED_INT:
   case GL_UNSIGNED_INT_VEC2:
   case GL_UNSIGNED_INT_VEC3:
   case GL_UNSIGNED_INT_VEC4:
      return true;
   default:
      return false;
   }
}

#define COERCE_ID 0
#define COERCE_INT_TO_FLOAT 1
#define COERCE_INT_TO_BOOL 2
#define COERCE_FLOAT_TO_BOOL 3
#define COERCE_FLOAT_TO_INT 4
#define COERCE_SIGN 5

static unsigned get_coercion(GLenum from_type, GLenum to_type)
{
   if (is_float_type(from_type)) {
      if (is_bool_type(to_type))
         return COERCE_FLOAT_TO_BOOL;
      if (!is_float_type(to_type))
         return COERCE_FLOAT_TO_INT;
      return COERCE_ID;
   }
   if (is_bool_type(to_type))
      return COERCE_INT_TO_BOOL;
   if (is_float_type(to_type))
      return COERCE_INT_TO_FLOAT;

   if (is_signed_int_type(from_type) && is_unsigned_int_type(to_type))
      return COERCE_SIGN;

   if (is_unsigned_int_type(from_type) && is_signed_int_type(to_type))
      return COERCE_SIGN;

   return COERCE_ID;
}

uint32_t glxx_get_element_count(GLenum type)
{
   switch (type) {
   case GL_FLOAT:
   case GL_INT:
   case GL_UNSIGNED_INT:
   case GL_BOOL:
   case GL_SAMPLER_2D:
   case GL_SAMPLER_CUBE:
   case GL_SAMPLER_CUBE_MAP_ARRAY:
   case GL_SAMPLER_2D_MULTISAMPLE:
   case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
   case GL_SAMPLER_EXTERNAL_OES:
   case GL_SAMPLER_3D:
   case GL_SAMPLER_2D_ARRAY:
   case GL_SAMPLER_1D_BRCM:
   case GL_SAMPLER_1D_ARRAY_BRCM:
   case GL_SAMPLER_2D_SHADOW:
   case GL_SAMPLER_CUBE_SHADOW:
   case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
   case GL_SAMPLER_2D_ARRAY_SHADOW:
   case GL_SAMPLER_BUFFER:
   case GL_INT_SAMPLER_1D_BRCM:
   case GL_INT_SAMPLER_1D_ARRAY_BRCM:
   case GL_INT_SAMPLER_2D:
   case GL_INT_SAMPLER_3D:
   case GL_INT_SAMPLER_CUBE:
   case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
   case GL_INT_SAMPLER_2D_MULTISAMPLE:
   case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
   case GL_INT_SAMPLER_2D_ARRAY:
   case GL_INT_SAMPLER_BUFFER:
   case GL_UNSIGNED_INT_SAMPLER_1D_BRCM:
   case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY_BRCM:
   case GL_UNSIGNED_INT_SAMPLER_2D:
   case GL_UNSIGNED_INT_SAMPLER_3D:
   case GL_UNSIGNED_INT_SAMPLER_CUBE:
   case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
   case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
   case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
   case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
   case GL_UNSIGNED_INT_SAMPLER_BUFFER:
   case GL_IMAGE_2D:
   case GL_IMAGE_3D:
   case GL_IMAGE_CUBE:
   case GL_IMAGE_2D_ARRAY:
   case GL_IMAGE_BUFFER:
   case GL_INT_IMAGE_2D:
   case GL_INT_IMAGE_3D:
   case GL_INT_IMAGE_CUBE:
   case GL_INT_IMAGE_2D_ARRAY:
   case GL_INT_IMAGE_BUFFER:
   case GL_UNSIGNED_INT_IMAGE_2D:
   case GL_UNSIGNED_INT_IMAGE_3D:
   case GL_UNSIGNED_INT_IMAGE_CUBE:
   case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
   case GL_UNSIGNED_INT_IMAGE_BUFFER:
      return 1;
   case GL_FLOAT_VEC2:
   case GL_INT_VEC2:
   case GL_UNSIGNED_INT_VEC2:
   case GL_BOOL_VEC2:
      return 2;
   case GL_FLOAT_VEC3:
   case GL_INT_VEC3:
   case GL_UNSIGNED_INT_VEC3:
   case GL_BOOL_VEC3:
      return 3;
   case GL_FLOAT_VEC4:
   case GL_INT_VEC4:
   case GL_UNSIGNED_INT_VEC4:
   case GL_BOOL_VEC4:
   case GL_FLOAT_MAT2:
      return 4;
   case GL_FLOAT_MAT3:
      return 9;
   case GL_FLOAT_MAT4:
      return 16;
   case GL_FLOAT_MAT2x3:
   case GL_FLOAT_MAT3x2:
      return 6;
   case GL_FLOAT_MAT4x2:
   case GL_FLOAT_MAT2x4:
      return 8;
   case GL_FLOAT_MAT3x4:
   case GL_FLOAT_MAT4x3:
      return 12;
   case GL_UNSIGNED_INT_ATOMIC_COUNTER:
      return 0;
   default:
      unreachable();
   }
   return 0;
}

static int get_column_height(GLenum type)
{
   switch (type) {
      case GL_FLOAT_MAT2:
      case GL_FLOAT_MAT3x2:
      case GL_FLOAT_MAT4x2:
         return 2;
      case GL_FLOAT_MAT2x3:
      case GL_FLOAT_MAT3:
      case GL_FLOAT_MAT4x3:
         return 3;
      case GL_FLOAT_MAT2x4:
      case GL_FLOAT_MAT3x4:
      case GL_FLOAT_MAT4:
         return 4;
      default:
         unreachable();
         return 0;
   }
}

static bool is_matrix_type(GLenum type)
{
   switch (type) {
      case GL_FLOAT_MAT2:
      case GL_FLOAT_MAT3x2:
      case GL_FLOAT_MAT4x2:
      case GL_FLOAT_MAT2x3:
      case GL_FLOAT_MAT3:
      case GL_FLOAT_MAT4x3:
      case GL_FLOAT_MAT2x4:
      case GL_FLOAT_MAT3x4:
      case GL_FLOAT_MAT4:
         return true;
      default:
         return false;
   }
}

/* Transpose a matrix. All matrices are floats */
static void transpose(GLfloat *v_out, const GLfloat *v_in, GLenum type) {
   int col_height = get_column_height(type);
   int row_length = glxx_get_element_count(type)/col_height;
   int i, j;
   for (i=0; i<row_length; i++) {
      for (j=0; j<col_height; j++) {
         v_out[col_height * i + j] = v_in[row_length * j + i];
      }
   }
}

static bool is_image_type(GLenum type)
{
   switch (type)
   {
   case GL_IMAGE_2D:
   case GL_IMAGE_3D:
   case GL_IMAGE_CUBE:
   case GL_IMAGE_2D_ARRAY:
   case GL_IMAGE_BUFFER:
   case GL_INT_IMAGE_2D:
   case GL_INT_IMAGE_3D:
   case GL_INT_IMAGE_CUBE:
   case GL_INT_IMAGE_2D_ARRAY:
   case GL_INT_IMAGE_BUFFER:
   case GL_UNSIGNED_INT_IMAGE_2D:
   case GL_UNSIGNED_INT_IMAGE_3D:
   case GL_UNSIGNED_INT_IMAGE_CUBE:
   case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
   case GL_UNSIGNED_INT_IMAGE_BUFFER:
      return true;
   default:
      return false;
   }
}

static bool decode_location(const GLSL_BLOCK_T *block, unsigned location, unsigned *index, unsigned *offset)
{
   for (unsigned i = 0; i < block->num_members; i++) {
      if (block->members[i].atomic_idx != -1) continue;

      if (block->members[i].offset <= location && block->members[i].offset + block->members[i].array_length > location) {
         *index = i;
         *offset = location - block->members[i].offset;
         return true;
      }
   }

   return false;
}

static void program_uniformv(GLXX_SERVER_STATE_T *state, GL20_PROGRAM_T *program,
                             GLint location, GLsizei num, const void *v,
                             GLenum from_type)
{
   unsigned int index, offset;

   /* Location -1 is special. Silently do nothing */
   if (location == -1) return;

   /* All negative location except -1 are invalid. */
   if (location < 0) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return;
   }

   /* All sizeis are invalid values if < 0 */
   if (num < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      return;
   }

   GLSL_BLOCK_MEMBER_T *info = program->common.linked_glsl_program->default_uniforms.members;

   if (!decode_location(&program->common.linked_glsl_program->default_uniforms, (unsigned)location, &index, &offset) ||
       offset >= info[index].array_length)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return;
   }

   if (is_image_type(info[index].type))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return;
   }

   if (glxx_get_element_count(from_type) != glxx_get_element_count(info[index].type))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return;
   }

   if (num > 1 && !info[index].is_array)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return;
   }

   if (offset + num > info[index].array_length)
      num = info[index].array_length - offset;

   unsigned int element_count = glxx_get_element_count(from_type);

   if (is_matrix_type(from_type)) {
      int num_rows = element_count / get_column_height(from_type);
      assert(element_count % num_rows == 0);
      element_count /= num_rows;
      num    *= num_rows;
      offset *= num_rows;
   }

   unsigned int array_stride;
   if (element_count == 3) array_stride = 4;
   else                    array_stride = element_count;

   for (int i=0; i<num; i++)
   {
      const int *datain = (const int *) v + i * element_count;
      uint32_t *data;
      unsigned j;

      data = program->common.uniform_data + program->common.linked_glsl_program->uniform_offsets[info[index].offset] + (offset+i) * array_stride;

      switch (get_coercion(from_type, info[index].type)) {
      case COERCE_ID:
         for (j = 0; j < element_count; j++)
            data[j] = datain[j];
         break;
      case COERCE_INT_TO_BOOL:
         for (j = 0; j < element_count; j++)
            data[j] = datain[j] ? 1 : 0;
         break;
      case COERCE_FLOAT_TO_BOOL:
         for (j = 0; j < element_count; j++)
            data[j] = (*(float *)&datain[j]) != 0.0f ? 1 : 0;
         break;
      case COERCE_FLOAT_TO_INT:
      case COERCE_INT_TO_FLOAT:
      case COERCE_SIGN:
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return;
      default:
         unreachable();
      }
   }
}

static void uniformv_internal(GLint location, GLsizei num, const void *v, GLenum from_type)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   GL20_PROGRAM_T      *active_program;
   if (!state) return;

   active_program = glxx_server_get_active_program(state);

   if (active_program == NULL) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   /* Use the current program. Current programs must have been linked */
   program_uniformv(state, active_program, location, num, v, from_type);

end:
   glxx_unlock_server_state();
}

static void get_uniform_internal(GLuint p, GLint location, GLsizei buf_size, void *v, GLenum to_type)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   GL20_PROGRAM_T *program = gl20_get_program(state, p);
   if (!program) goto end;    /* get_program sets its own error */

   if (buf_size < 0) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   if (!program->common.linked_glsl_program) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   GLSL_BLOCK_MEMBER_T *info = program->common.linked_glsl_program->default_uniforms.members;

   if (info == NULL) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   unsigned int index, offset;
   if (!decode_location(&program->common.linked_glsl_program->default_uniforms, (unsigned)location, &index, &offset) ||
       offset >= info[index].array_length)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   int element_count = glxx_get_element_count(info[index].type);
   int num = 1;

   if (is_matrix_type(info[index].type)) {
      int num_rows = element_count / get_column_height(info[index].type);
      assert(element_count % num_rows == 0);
      element_count /= num_rows;
      num    *= num_rows;
      offset *= num_rows;
   }

   int array_stride;
   if (element_count == 3) array_stride = 4;
   else                    array_stride = element_count;

   if ((num * element_count * sizeof(uint32_t)) > (unsigned)buf_size) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   for (int i=0; i<num; i++)
   {
      uint32_t *dataout = (uint32_t *)v + i*element_count;
      uint32_t *data = program->common.uniform_data + program->common.linked_glsl_program->uniform_offsets[info[index].offset] + (offset+i) * array_stride;

      for (int j = 0; j < element_count; j++)
      {
         switch (get_coercion(info[index].type, to_type))
         {
         case COERCE_ID:
         case COERCE_SIGN:
            dataout[j] = data[j];
            break;
         case COERCE_INT_TO_FLOAT:
            *(float*)&dataout[j] = (float) data[j];
            break;
         case COERCE_FLOAT_TO_INT:
            dataout[j] = (GLint) *(float*)&data[j];
            break;
         default:
            unreachable();
         }
      }
   }

end:
   glxx_unlock_server_state();
}

static void uniformv_transpose(GLint location, GLsizei num, const GLfloat *v, GLenum type) {
   GLfloat *v_t = malloc(16 * num * sizeof(float));
   int el_count = glxx_get_element_count(type);
   int i;
   for (i=0; i<num; i++) {
      transpose(v_t + i*el_count, v + i*el_count, type);
   }
   uniformv_internal(location, num, v_t, type);
   free(v_t);
}

GL_APICALL void GL_APIENTRY glUniform1i(GLint location, GLint x)
{
   uniformv_internal(location, 1, &x, GL_INT);
}

GL_APICALL void GL_APIENTRY glUniform2i(GLint location, GLint x, GLint y)
{
   GLint v[2] = { x, y };
   uniformv_internal(location, 1, v, GL_INT_VEC2);
}

GL_APICALL void GL_APIENTRY glUniform3i(GLint location, GLint x, GLint y, GLint z)
{
   GLint v[3] = {x, y, z};
   uniformv_internal(location, 1, v, GL_INT_VEC3);
}

GL_APICALL void GL_APIENTRY glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
   GLint v[4] = { x, y, z, w };
   uniformv_internal(location, 1, v, GL_INT_VEC4);
}

GL_APICALL void GL_APIENTRY glUniform1ui(GLint location, GLuint x)
{
   uniformv_internal(location, 1, &x, GL_UNSIGNED_INT);
}

GL_APICALL void GL_APIENTRY glUniform2ui(GLint location, GLuint x, GLuint y)
{
   GLuint v[2] = { x, y };
   uniformv_internal(location, 1, v, GL_UNSIGNED_INT_VEC2);
}

GL_APICALL void GL_APIENTRY glUniform3ui(GLint location, GLuint x, GLuint y, GLuint z)
{
   GLuint v[3] = {x, y, z};
   uniformv_internal(location, 1, v, GL_UNSIGNED_INT_VEC3);
}

GL_APICALL void GL_APIENTRY glUniform4ui(GLint location, GLuint x, GLuint y, GLuint z, GLuint w)
{
   GLuint v[4] = { x, y, z, w };
   uniformv_internal(location, 1, v, GL_UNSIGNED_INT_VEC4);
}

GL_APICALL void GL_APIENTRY glUniform1f(GLint location, GLfloat x)
{
   uniformv_internal(location, 1, &x, GL_FLOAT);
}

GL_APICALL void GL_APIENTRY glUniform2f(GLint location, GLfloat x, GLfloat y)
{
   GLfloat v[2] = { x, y };
   uniformv_internal(location, 1, v, GL_FLOAT_VEC2);
}

GL_APICALL void GL_APIENTRY glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
   GLfloat v[3] = { x, y, z };
   uniformv_internal(location, 1, v, GL_FLOAT_VEC3);
}

GL_APICALL void GL_APIENTRY glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GLfloat v[4] = { x, y, z, w };
   uniformv_internal(location, 1, v, GL_FLOAT_VEC4);
}

GL_APICALL void GL_APIENTRY glUniform1iv(GLint location, GLsizei num, const GLint *v)
{
   uniformv_internal(location, num, v, GL_INT);
}

GL_APICALL void GL_APIENTRY glUniform2iv(GLint location, GLsizei num, const GLint *v)
{
   uniformv_internal(location, num, v, GL_INT_VEC2);
}

GL_APICALL void GL_APIENTRY glUniform3iv(GLint location, GLsizei num, const GLint *v)
{
   uniformv_internal(location, num, v, GL_INT_VEC3);
}

GL_APICALL void GL_APIENTRY glUniform4iv(GLint location, GLsizei num, const GLint *v)
{
   uniformv_internal(location, num, v, GL_INT_VEC4);
}

GL_APICALL void GL_APIENTRY glUniform1uiv(GLint location, GLsizei num, const GLuint *v)
{
   uniformv_internal(location, num, v, GL_UNSIGNED_INT);
}

GL_APICALL void GL_APIENTRY glUniform2uiv(GLint location, GLsizei num, const GLuint *v)
{
   uniformv_internal(location, num, v, GL_UNSIGNED_INT_VEC2);
}

GL_APICALL void GL_APIENTRY glUniform3uiv(GLint location, GLsizei num, const GLuint *v)
{
   uniformv_internal(location, num, v, GL_UNSIGNED_INT_VEC3);
}

GL_APICALL void GL_APIENTRY glUniform4uiv(GLint location, GLsizei num, const GLuint *v)
{
   uniformv_internal(location, num, v, GL_UNSIGNED_INT_VEC4);
}

GL_APICALL void GL_APIENTRY glUniform1fv(GLint location, GLsizei num, const GLfloat *v)
{
   uniformv_internal(location, num, v, GL_FLOAT);
}

GL_APICALL void GL_APIENTRY glUniform2fv(GLint location, GLsizei num, const GLfloat *v)
{
   uniformv_internal(location, num, v, GL_FLOAT_VEC2);
}

GL_APICALL void GL_APIENTRY glUniform3fv(GLint location, GLsizei num, const GLfloat *v)
{
   uniformv_internal(location, num, v, GL_FLOAT_VEC3);
}

GL_APICALL void GL_APIENTRY glUniform4fv(GLint location, GLsizei num, const GLfloat *v)
{
   uniformv_internal(location, num, v, GL_FLOAT_VEC4);
}

GL_APICALL void GL_APIENTRY glUniformMatrix2fv(GLint location, GLsizei num, GLboolean transpose, const GLfloat *v)
{
   if (!transpose) {
      uniformv_internal(location, num, v, GL_FLOAT_MAT2);
   } else {
      uniformv_transpose(location, num, v, GL_FLOAT_MAT2);
   }
}

GL_APICALL void GL_APIENTRY glUniformMatrix2x3fv(GLint location, GLsizei num, GLboolean transpose, const GLfloat *v)
{
   if (!transpose) {
      uniformv_internal(location, num, v, GL_FLOAT_MAT2x3);
   } else {
      uniformv_transpose(location, num, v, GL_FLOAT_MAT2x3);
   }
}

GL_APICALL void GL_APIENTRY glUniformMatrix2x4fv(GLint location, GLsizei num, GLboolean transpose, const GLfloat *v)
{
   if (!transpose) {
      uniformv_internal(location, num, v, GL_FLOAT_MAT2x4);
   } else {
      uniformv_transpose(location, num, v, GL_FLOAT_MAT2x4);
   }
}

GL_APICALL void GL_APIENTRY glUniformMatrix3x2fv(GLint location, GLsizei num, GLboolean transpose, const GLfloat *v)
{
   if (!transpose) {
      uniformv_internal(location, num, v, GL_FLOAT_MAT3x2);
   } else {
      uniformv_transpose(location, num, v, GL_FLOAT_MAT3x2);
   }
}

GL_APICALL void GL_APIENTRY glUniformMatrix3fv(GLint location, GLsizei num, GLboolean transpose, const GLfloat *v)
{
   if (!transpose) {
      uniformv_internal(location, num, v, GL_FLOAT_MAT3);
   } else {
      uniformv_transpose(location, num, v, GL_FLOAT_MAT3);
   }
}

GL_APICALL void GL_APIENTRY glUniformMatrix3x4fv(GLint location, GLsizei num, GLboolean transpose, const GLfloat *v)
{
   if (!transpose) {
      uniformv_internal(location, num, v, GL_FLOAT_MAT3x4);
   } else {
      uniformv_transpose(location, num, v, GL_FLOAT_MAT3x4);
   }
}

GL_APICALL void GL_APIENTRY glUniformMatrix4x2fv(GLint location, GLsizei num, GLboolean transpose, const GLfloat *v)
{
   if (!transpose) {
      uniformv_internal(location, num, v, GL_FLOAT_MAT4x2);
   } else {
      uniformv_transpose(location, num, v, GL_FLOAT_MAT4x2);
   }
}

GL_APICALL void GL_APIENTRY glUniformMatrix4x3fv(GLint location, GLsizei num, GLboolean transpose, const GLfloat *v)
{
   if (!transpose) {
      uniformv_internal(location, num, v, GL_FLOAT_MAT4x3);
   } else {
      uniformv_transpose(location, num, v, GL_FLOAT_MAT4x3);
   }
}

GL_APICALL void GL_APIENTRY glUniformMatrix4fv(GLint location, GLsizei num, GLboolean transpose, const GLfloat *v)
{
   if (!transpose) {
      uniformv_internal(location, num, v, GL_FLOAT_MAT4);
   } else {
      uniformv_transpose(location, num, v, GL_FLOAT_MAT4);
   }
}

#if KHRN_GLES31_DRIVER

/* ES3.1 Program Pipeline uniform setting commands: */

static void program_uniformv_internal(GLuint p, GLint location,
                                      GLsizei num, const void *v,
                                      GLenum from_type)
{
   GLXX_SERVER_STATE_T *state;
   GL20_PROGRAM_T *program;

   state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   /* This sets its own errors */
   program = gl20_get_program(state, p);
   if (!program) goto end;

   if (!program->common.linked_glsl_program) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   program_uniformv(state, program, location, num, v, from_type);

end:
   glxx_unlock_server_state();
}

static void program_uniformv_transpose(GLuint p, GLint location,
                                       GLsizei num, const GLfloat *v, GLenum type)
{
   GLfloat *v_t = malloc(16 * num * sizeof(float));
   int el_count = glxx_get_element_count(type);
   int i;
   for (i=0; i<num; i++) {
      transpose(v_t + i*el_count, v + i*el_count, type);
   }
   program_uniformv_internal(p, location, num, v_t, type);
   free(v_t);
}

GL_APICALL void GL_APIENTRY glProgramUniform1i (GLuint program, GLint location, GLint v0)
{
   program_uniformv_internal(program, location, 1, &v0, GL_INT);
}

GL_APICALL void GL_APIENTRY glProgramUniform2i (GLuint program, GLint location, GLint v0, GLint v1)
{
   GLint v[2] = { v0, v1 };
   program_uniformv_internal(program, location, 1, v, GL_INT_VEC2);
}

GL_APICALL void GL_APIENTRY glProgramUniform3i (GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
   GLint v[3] = { v0, v1, v2 };
   program_uniformv_internal(program, location, 1, v, GL_INT_VEC3);
}

GL_APICALL void GL_APIENTRY glProgramUniform4i (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
   GLint v[4] = { v0, v1, v2, v3 };
   program_uniformv_internal(program, location, 1, v, GL_INT_VEC4);
}

GL_APICALL void GL_APIENTRY glProgramUniform1ui (GLuint program, GLint location, GLuint v0)
{
   program_uniformv_internal(program, location, 1, &v0, GL_UNSIGNED_INT);
}

GL_APICALL void GL_APIENTRY glProgramUniform2ui (GLuint program, GLint location, GLuint v0, GLuint v1)
{
   GLuint v[2] = { v0, v1 };
   program_uniformv_internal(program, location, 1, v, GL_UNSIGNED_INT_VEC2);
}

GL_APICALL void GL_APIENTRY glProgramUniform3ui (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
   GLuint v[3] = { v0, v1, v2 };
   program_uniformv_internal(program, location, 1, v, GL_UNSIGNED_INT_VEC3);
}

GL_APICALL void GL_APIENTRY glProgramUniform4ui (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
   GLuint v[4] = { v0, v1, v2, v3 };
   program_uniformv_internal(program, location, 1, v, GL_UNSIGNED_INT_VEC4);
}

GL_APICALL void GL_APIENTRY glProgramUniform1f (GLuint program, GLint location, GLfloat v0)
{
   program_uniformv_internal(program, location, 1, &v0, GL_FLOAT);
}

GL_APICALL void GL_APIENTRY glProgramUniform2f (GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
   GLfloat v[2] = { v0, v1 };
   program_uniformv_internal(program, location, 1, v, GL_FLOAT_VEC2);
}

GL_APICALL void GL_APIENTRY glProgramUniform3f (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
   GLfloat v[3] = { v0, v1, v2 };
   program_uniformv_internal(program, location, 1, v, GL_FLOAT_VEC3);
}

GL_APICALL void GL_APIENTRY glProgramUniform4f (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
   GLfloat v[4] = { v0, v1, v2, v3 };
   program_uniformv_internal(program, location, 1, v, GL_FLOAT_VEC4);
}

GL_APICALL void GL_APIENTRY glProgramUniform1iv (GLuint program, GLint location, GLsizei count, const GLint *value)
{
   program_uniformv_internal(program, location, count, value, GL_INT);
}

GL_APICALL void GL_APIENTRY glProgramUniform2iv (GLuint program, GLint location, GLsizei count, const GLint *value)
{
   program_uniformv_internal(program, location, count, value, GL_INT_VEC2);
}

GL_APICALL void GL_APIENTRY glProgramUniform3iv (GLuint program, GLint location, GLsizei count, const GLint *value)
{
   program_uniformv_internal(program, location, count, value, GL_INT_VEC3);
}

GL_APICALL void GL_APIENTRY glProgramUniform4iv (GLuint program, GLint location, GLsizei count, const GLint *value)
{
   program_uniformv_internal(program, location, count, value, GL_INT_VEC4);
}

GL_APICALL void GL_APIENTRY glProgramUniform1uiv (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
   program_uniformv_internal(program, location, count, value, GL_UNSIGNED_INT);
}

GL_APICALL void GL_APIENTRY glProgramUniform2uiv (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
   program_uniformv_internal(program, location, count, value, GL_UNSIGNED_INT_VEC2);
}

GL_APICALL void GL_APIENTRY glProgramUniform3uiv (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
   program_uniformv_internal(program, location, count, value, GL_UNSIGNED_INT_VEC3);
}

GL_APICALL void GL_APIENTRY glProgramUniform4uiv (GLuint program, GLint location, GLsizei count, const GLuint *value)
{
   program_uniformv_internal(program, location, count, value, GL_UNSIGNED_INT_VEC4);
}

GL_APICALL void GL_APIENTRY glProgramUniform1fv (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
   program_uniformv_internal(program, location, count, value, GL_FLOAT);
}

GL_APICALL void GL_APIENTRY glProgramUniform2fv (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
   program_uniformv_internal(program, location, count, value, GL_FLOAT_VEC2);
}

GL_APICALL void GL_APIENTRY glProgramUniform3fv (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
   program_uniformv_internal(program, location, count, value, GL_FLOAT_VEC3);
}

GL_APICALL void GL_APIENTRY glProgramUniform4fv (GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
   program_uniformv_internal(program, location, count, value, GL_FLOAT_VEC4);
}

GL_APICALL void GL_APIENTRY glProgramUniformMatrix2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!transpose) {
      program_uniformv_internal(program, location, count, value, GL_FLOAT_MAT2);
   } else {
      program_uniformv_transpose(program, location, count, value, GL_FLOAT_MAT2);
   }
}

GL_APICALL void GL_APIENTRY glProgramUniformMatrix3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!transpose) {
      program_uniformv_internal(program, location, count, value, GL_FLOAT_MAT3);
   } else {
      program_uniformv_transpose(program, location, count, value, GL_FLOAT_MAT3);
   }
}

GL_APICALL void GL_APIENTRY glProgramUniformMatrix4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!transpose) {
      program_uniformv_internal(program, location, count, value, GL_FLOAT_MAT4);
   } else {
      program_uniformv_transpose(program, location, count, value, GL_FLOAT_MAT4);
   }
}

GL_APICALL void GL_APIENTRY glProgramUniformMatrix2x3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!transpose) {
      program_uniformv_internal(program, location, count, value, GL_FLOAT_MAT2x3);
   } else {
      program_uniformv_transpose(program, location, count, value, GL_FLOAT_MAT2x3);
   }
}

GL_APICALL void GL_APIENTRY glProgramUniformMatrix3x2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!transpose) {
      program_uniformv_internal(program, location, count, value, GL_FLOAT_MAT3x2);
   } else {
      program_uniformv_transpose(program, location, count, value, GL_FLOAT_MAT3x2);
   }
}

GL_APICALL void GL_APIENTRY glProgramUniformMatrix2x4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!transpose) {
      program_uniformv_internal(program, location, count, value, GL_FLOAT_MAT2x4);
   } else {
      program_uniformv_transpose(program, location, count, value, GL_FLOAT_MAT2x4);
   }
}

GL_APICALL void GL_APIENTRY glProgramUniformMatrix4x2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!transpose) {
      program_uniformv_internal(program, location, count, value, GL_FLOAT_MAT4x2);
   } else {
      program_uniformv_transpose(program, location, count, value, GL_FLOAT_MAT4x2);
   }
}

GL_APICALL void GL_APIENTRY glProgramUniformMatrix3x4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!transpose) {
      program_uniformv_internal(program, location, count, value, GL_FLOAT_MAT3x4);
   } else {
      program_uniformv_transpose(program, location, count, value, GL_FLOAT_MAT3x4);
   }
}

GL_APICALL void GL_APIENTRY glProgramUniformMatrix4x3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!transpose) {
      program_uniformv_internal(program, location, count, value, GL_FLOAT_MAT4x3);
   } else {
      program_uniformv_transpose(program, location, count, value, GL_FLOAT_MAT4x3);
   }
}

#endif

GL_APICALL void GL_APIENTRY glUseProgram(GLuint p) // S
{
   GL20_PROGRAM_T *program;
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   //  The error INVALID_OPERATION is generated by UseProgram
   //  if the current transform feedback object is active and not paused
   if (state->transform_feedback.in_use) {
      log_info("glUseProgram() transform feedback object is active and not paused, setting INVALID_OPERATION");
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto unlock_out;
   }

   if (p != 0) {
      program = gl20_get_program(state, p);
      if (!program) goto unlock_out;

      if (!program->common.linked_glsl_program) {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         goto unlock_out;
      }

      gl20_program_acquire(program);

   } else
      program = NULL;

   release_program(state->shared, state->current_program);

   state->current_program = program;

unlock_out:
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glValidateProgram(GLuint p)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   GL20_PROGRAM_T *program;
   if (!state) return;

   program = gl20_get_program(state, p);

   if (program) {
      program->validated = gl20_validate_program(state, &program->common);

      KHRN_MEM_ASSIGN(program->mh_info, KHRN_MEM_HANDLE_EMPTY_STRING);
   }

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glCompileShader(GLuint s)
{
   GL20_SHADER_T *shader;
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   shader = get_shader(state, s);

   if (shader) {
      gl20_shader_compile(shader);
   }

   glxx_unlock_server_state();
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
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   GL20_SHADER_T *shader;
   if (!state) return;

   shader = get_shader(state, s);
   if (!shader) goto end;

   switch (pname) {
   case GL_SHADER_TYPE:
      params[0] = shader->type;
      break;
   case GL_DELETE_STATUS:
      params[0] = shader->deleted ? 1 : 0;
      break;
   case GL_COMPILE_STATUS:
      params[0] = (shader->binary != NULL) ? 1 : 0;
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
      int total = 0;

      for (int i = 0; i < shader->sourcec; i++)
         total += strlen(shader->source[i]);

      /* If there's any source at all, count 1 for the NULL terminator */
      if (shader->sourcec > 0) total++;

      params[0] = total;
      break;
   }
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

end:
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glGetShaderInfoLog(GLuint s, GLsizei bufsize, GLsizei *length, char *infolog)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   GL20_SHADER_T *shader;
   if (!state) return;

   shader = get_shader(state, s);
   if (shader == NULL) goto end;    /* get_shader will have set the error */

   if (bufsize < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   size_t chars = 0;
   if (shader->info_log != NULL)
      chars = strzncpy(infolog, shader->info_log, bufsize);

   if (length)
      *length = gfx_smax(0, (GLsizei)chars);

end:
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glGetShaderSource(GLuint s, GLsizei bufsize, GLsizei *length, GLchar *source)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   GL20_SHADER_T *shader;
   uint32_t charswritten = 0;
   if (!state) return;

   shader = get_shader(state, s);
   if (shader == NULL) goto end;       /* get_shader will have set error */

   if (bufsize < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (bufsize > 1) {//need 1 byte for NULL terminator below
      for (int i = 0; i < shader->sourcec; i++) {
         const char *str = shader->source[i];
         int32_t len = strlen(str);
         assert(len >= 0);
         assert(str[len] == 0);

         if (charswritten + len > (uint32_t)bufsize - 1)
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
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glShaderSource(GLuint s, GLsizei count, const GLchar *const *string, const GLint *length)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if (count < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   GL20_SHADER_T *shader = get_shader(state, s);
   if (!shader) goto end;

   if (!string) goto end;

   if (!gl20_shader_set_source(shader, count, string, length))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

end:
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glVertexAttribDivisor(GLuint index, GLuint divisor)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   GLXX_VAO_T *vao = state->vao.bound;

   if (index < GLXX_CONFIG_MAX_VERTEX_ATTRIBS)
   {
      /* This call also resets the attribute binding (see es3.1 spec, 10.3.2) */
      vao->attrib_config[index].vbo_index = index;
      vao->vbos[index].divisor = divisor;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glGetActiveUniformsiv(
   GLuint         p,
   GLsizei        uniformCount,
   const GLuint*  uniformIndices,
   GLenum         pname,
   GLint*         params)
{
   GLXX_SERVER_STATE_T  *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   GLenum               prop;
   GLenum               error = GL_NO_ERROR;

   if (!state)
      return;

   // Map pname to prop from table 7.6
   switch (pname)
   {
   case GL_UNIFORM_TYPE:            prop = GL_TYPE;            break;
   case GL_UNIFORM_SIZE:            prop = GL_ARRAY_SIZE;      break;
   case GL_UNIFORM_NAME_LENGTH:     prop = GL_NAME_LENGTH;     break;
   case GL_UNIFORM_BLOCK_INDEX:     prop = GL_BLOCK_INDEX;     break;
   case GL_UNIFORM_OFFSET:          prop = GL_OFFSET;          break;
   case GL_UNIFORM_ARRAY_STRIDE:    prop = GL_ARRAY_STRIDE;    break;
   case GL_UNIFORM_MATRIX_STRIDE:   prop = GL_MATRIX_STRIDE;   break;
   case GL_UNIFORM_IS_ROW_MAJOR:    prop = GL_IS_ROW_MAJOR;    break;
   default:
      error = GL_INVALID_ENUM;
      break;
   }

   if (uniformCount < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   // We need to see if any of the iterations would produce an error, as we must not write
   // to params in that case.
   for (int i = 0; i < uniformCount; i++)
   {
      if (!glxx_get_program_resourceiv(state, p, GL_UNIFORM, uniformIndices[i], 1, &prop, 0, NULL, NULL))
         goto end;
   }

   // Section 7.6 of the spec says this is equivalent to:
   for (int i = 0; i < uniformCount; i++)
      glxx_get_program_resourceiv(state, p, GL_UNIFORM, uniformIndices[i], 1, &prop, 1, NULL, &params[i]);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glGetActiveUniformBlockName(
   GLuint   p,
   GLuint   uniformBlockIndex,
   GLsizei  bufSize,
   GLsizei  *length,
   GLchar   *uniformBlockName)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state)
      return;

   // Section 7.6 of the spec says this is equivalent to :
   glxx_get_program_resource_name(state, p, GL_UNIFORM_BLOCK, uniformBlockIndex,
                                  bufSize, length, uniformBlockName);

   glxx_unlock_server_state();
}

GL_APICALL GLuint GL_APIENTRY glGetUniformBlockIndex(GLuint p, const GLchar* uniformBlockName)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   GLuint               ret = GL_INVALID_INDEX;
   if (!state)
      return ret;

   // Section 7.6 of the spec says this is equivalent to :
   ret = glxx_get_program_resource_index(state, p, GL_UNIFORM_BLOCK, uniformBlockName);

   glxx_unlock_server_state();

   return ret;
}

GL_APICALL void GL_APIENTRY glGetUniformIndices(
   GLuint               p,
   GLsizei              uniformCount,
   const GLchar* const  *uniformNames,
   GLuint               *uniformIndices)
{
   GLXX_SERVER_STATE_T  *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   GLenum               error = GL_NO_ERROR;

   if (uniformCount < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   // Section 7.6 of the spec says this is equivalent to:
   for (int i = 0; i < uniformCount; i++)
      uniformIndices[i] = glxx_get_program_resource_index(state, p, GL_UNIFORM, uniformNames[i]);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glUniformBlockBinding(
   GLuint p,
   GLuint uniformBlockIndex,
   GLuint uniformBlockBinding
)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if (uniformBlockBinding >= GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   GL20_PROGRAM_T *program = gl20_get_program(state, p); // takes care of setting correct error
   if (!program) goto end;

   /* If not linked then this index is definitely out of range */
   if (program->common.linked_glsl_program == NULL) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   const GLSL_BLOCK_T *block = gl20_get_ubo_from_index(program->common.linked_glsl_program, uniformBlockIndex);
   if (block != NULL) {
      program->common.ubo_binding_point[uniformBlockIndex] = uniformBlockBinding;
   } else {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   }

end:
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glGetActiveUniformBlockiv(
   GLuint   p,
   GLuint   uniformBlockIndex,
   GLenum   pname,
   GLint    *params
)
{
   GLXX_SERVER_STATE_T  *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);

   GLenum  prop;
   GLsizei bufSize = 1;

   switch (pname)
   {
   case GL_UNIFORM_BLOCK_BINDING:                       prop = GL_BUFFER_BINDING;                break;
   case GL_UNIFORM_BLOCK_DATA_SIZE:                     prop = GL_BUFFER_DATA_SIZE;              break;
   case GL_UNIFORM_BLOCK_NAME_LENGTH:                   prop = GL_NAME_LENGTH;                   break;
   case GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS:               prop = GL_NUM_ACTIVE_VARIABLES;          break;
   case GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES:        prop = GL_ACTIVE_VARIABLES;              break;
   case GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER:   prop = GL_REFERENCED_BY_VERTEX_SHADER;   break;
   case GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER: prop = GL_REFERENCED_BY_FRAGMENT_SHADER; break;
   default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         goto end;
   }

   if (prop == GL_ACTIVE_VARIABLES)
   {
      GLenum  numProp = GL_NUM_ACTIVE_VARIABLES;
      GLsizei numVars;
      glxx_get_program_resourceiv(state, p, GL_UNIFORM_BLOCK, uniformBlockIndex, 1, &numProp, bufSize, NULL, &numVars);
      bufSize = numVars;
   }

   glxx_get_program_resourceiv(state, p, GL_UNIFORM_BLOCK, uniformBlockIndex, 1, &prop, bufSize, NULL, params);

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetProgramBinary(GLuint p, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   GL20_PROGRAM_T      *program;
   if (!state) return;

   // No supported binary formats
   program = gl20_get_program(state, p);
   if (!program)
      goto end;

   if (!program->common.linked_glsl_program || bufSize == 0)
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);

   glxx_server_state_set_error(state, GL_INVALID_ENUM);

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glProgramBinary(GLuint p, GLenum binaryFormat, const GLvoid* binary, GLsizei length)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   // No supported binary formats
   glxx_server_state_set_error(state, GL_INVALID_OPERATION);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glProgramParameteri(GLuint p, GLenum pname, GLint value)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   GL20_PROGRAM_T *program = gl20_get_program(state, p);  // takes care of setting correct error
   if (!program)
      goto end;

   switch (pname)
   {
   case GL_PROGRAM_BINARY_RETRIEVABLE_HINT:
      if (value != GL_TRUE && value != GL_FALSE) {
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
      }
      program->binary_hint = !!value;
      break;

#if KHRN_GLES31_DRIVER
   case GL_PROGRAM_SEPARABLE:
      if (value != GL_TRUE && value != GL_FALSE) {
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
      }
      program->separable = !!value;
      break;
#endif

   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

end:
   glxx_unlock_server_state();
}

#if KHRN_GLES31_DRIVER

// TODO: Check this function for error behaviour.  It should not disturb the state.
GL_APICALL GLuint GL_APIENTRY glCreateShaderProgramv(GLenum type, GLsizei count, const GLchar *const*strings)
{
   GLenum               error   = GL_NO_ERROR;
   GLuint               program = 0;
   GLXX_SERVER_STATE_T *state   = glxx_lock_server_state(OPENGL_ES_3X);
   GL20_SHADER_T       *shader_object = NULL;

   if (state == NULL)
      return 0;

   if (!glxx_is_shader_type(type))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (count < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   // CreateShader
   GLuint shader = glxx_shared_create_shader(state->shared, type);

   if (shader == 0)
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }

   // ShaderSource
   shader_object = glxx_shared_get_pobject(state->shared, shader);

   if (shader_object == NULL || strings == NULL)
      goto end;

   if (!gl20_shader_set_source(shader_object, count, strings, NULL))
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }

   // CompileShader
   gl20_shader_compile(shader_object);

   // CreateProgram
   program = glxx_shared_create_program(state->shared);

   if (program == 0)
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }

   // GetShaderiv( , COMPILE_STATUS, )
   bool compiled = shader_object->binary != NULL;

   GL20_PROGRAM_T *program_object = glxx_shared_get_pobject(state->shared, program);

   // ProgramParameter( , PROGRAM_SEPARABLE, )
   program_object->separable = true;

   if (compiled)
   {
      // AttachShader
      gl20_program_attach_shader(program_object, shader_object);

      // LinkProgram
      gl20_program_link(program_object);

      // DetachShader
      gl20_program_detach_shader(program_object, shader_object);
   }

   // Concatenate the info_log
   const char   *old_log = khrn_mem_lock(program_object->mh_info);
   unsigned int  old_len = strlen(old_log);
   unsigned int  new_len = old_len + strlen(shader_object->info_log);
   char         *tmp     = calloc(sizeof(char), new_len + 1);

   if (tmp != NULL)
   {
      strncpy(tmp, old_log, old_len + 1);
      strcat(tmp, shader_object->info_log);
   }

   khrn_mem_unlock(program_object->mh_info);

   if (tmp != NULL)
      gl20_program_save_error(program_object, tmp);

   free(tmp);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   if (shader_object != NULL)
   {
      // DeleteShader
      shader_object->deleted = GL_TRUE;
      gl20_server_try_delete_shader(state->shared, shader_object);
   }

   glxx_unlock_server_state();

   return program;
}

#endif
