/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/glxx/glxx_shared.h"

#include "middleware/khronos/gl11/gl11_server.h"
#include "middleware/khronos/glxx/glxx_server_internal.h"
#include "interface/khronos/include/GLES/glext.h"

#include "middleware/khronos/glxx/glxx_texture.h"
#include "middleware/khronos/glxx/glxx_buffer.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "middleware/khronos/common/khrn_interlock.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "middleware/khronos/glxx/glxx_tweaker.h"

#include <string.h>
#include <math.h>
#include <limits.h>

#define LOG2E  1.442695f

#ifdef GL11_SERVER_SINGLE
GLXX_SERVER_STATE_T gl11_server_state;
#endif

#define PI     3.14159265f

static int get_texenv_integer(GLXX_SERVER_STATE_T *state, GLenum env, GLenum pname, int *params)
{
   GL11_TEXUNIT_T *texunit = &state->texunits[state->active_texture - GL_TEXTURE0];
   GL11_CACHE_TEXUNIT_ABSTRACT_T *texabs = &state->shader.texunits[state->active_texture - GL_TEXTURE0];

   int result = 0;

   switch (env) {
   case GL_POINT_SPRITE_OES:
      switch (pname) {
      case GL_COORD_REPLACE_OES:
         params[0] = texabs->coord_replace;
         result = 1;
         break;
      default:
         UNREACHABLE();
         break;
      }
      break;
   case GL_TEXTURE_ENV:
      switch (pname) {
      case GL_TEXTURE_ENV_MODE:
         params[0] = texabs->mode;
         result = 1;
         break;
      case GL_COMBINE_RGB:
         params[0] = texunit->rgb.combine;
         result = 1;
         break;
      case GL_COMBINE_ALPHA:
         params[0] = texunit->alpha.combine;
         result = 1;
         break;
      case GL_SRC0_RGB:
      case GL_SRC1_RGB:
      case GL_SRC2_RGB:
         params[0] = texunit->rgb.source[pname - GL_SRC0_RGB];
         result = 1;
         break;
      case GL_SRC0_ALPHA:
      case GL_SRC1_ALPHA:
      case GL_SRC2_ALPHA:
         params[0] = texunit->alpha.source[pname - GL_SRC0_ALPHA];
         result = 1;
         break;
      case GL_OPERAND0_RGB:
      case GL_OPERAND1_RGB:
      case GL_OPERAND2_RGB:
         params[0] = texunit->rgb.operand[pname - GL_OPERAND0_RGB];
         result = 1;
         break;
      case GL_OPERAND0_ALPHA:
      case GL_OPERAND1_ALPHA:
      case GL_OPERAND2_ALPHA:
         params[0] = texunit->alpha.operand[pname - GL_OPERAND0_ALPHA];
         result = 1;
         break;
      default:
         UNREACHABLE();
         break;
      }
      break;
   default:
      UNREACHABLE();
      break;
   }

   return result;
}

static int get_texenv_float(GLXX_SERVER_STATE_T *state, GLenum env, GLenum pname, float *params)
{
   int t = state->active_texture - GL_TEXTURE0;
   int result = 0;

   switch (env) {
   case GL_TEXTURE_ENV:
      switch (pname) {
      case GL_TEXTURE_ENV_COLOR:
      {
         int i;
         for (i = 0; i < 4; i++)
            params[i] = state->texunits[t].color[i];
         result = 4;
      }
      break;
      case GL_RGB_SCALE:
         params[0] = state->texunits[t].rgb.scale;
         result = 1;
         break;
      case GL_ALPHA_SCALE:
         params[0] = state->texunits[t].alpha.scale;
         result = 1;
         break;
      default:
         UNREACHABLE();
         break;
      }
      break;
   default:
      UNREACHABLE();
      break;
   }

   return result;
}

static GL11_LIGHT_T *get_light(GLXX_SERVER_STATE_T *state, GLenum l)
{
   if (l >= GL_LIGHT0 && l < GL_LIGHT0 + GL11_CONFIG_MAX_LIGHTS)
      return &state->lights[l - GL_LIGHT0];
   else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return NULL;
   }
}

static GL11_MATRIX_STACK_T *get_stack(GLXX_SERVER_STATE_T *state)
{
   switch (state->matrix_mode) {
   case GL_MODELVIEW:
      return &state->modelview;
   case GL_PROJECTION:
      return &state->projection;
   case GL_TEXTURE:
      return &state->texunits[state->active_texture - GL_TEXTURE0].stack;
   default:
      UNREACHABLE();
      return NULL;
   }
}

static float *get_matrix(GLXX_SERVER_STATE_T *state)
{
   GL11_MATRIX_STACK_T *stack = get_stack(state);

   return stack->body[stack->pos];
}

static void load_matrix_internal(const float *m)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   float *c = get_matrix(state);

   gl11_matrix_load(c, m);

   glxx_unlock_server_state(OPENGL_ES_11);
}

static void mult_matrix(GLXX_SERVER_STATE_T *state, const float *m)
{
   GLfloat *c = get_matrix(state);
   gl11_matrix_mult(c, c, m);
}

static void mult_matrix_internal(const float *m)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   mult_matrix(state, m);

   glxx_unlock_server_state(OPENGL_ES_11);
}

static float *get_plane(GLXX_SERVER_STATE_T *state, GLenum p)
{
   if (p >= GL_CLIP_PLANE0 && p < GL_CLIP_PLANE0 + GL11_CONFIG_MAX_PLANES)
      return state->planes[p - GL_CLIP_PLANE0];
   else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return NULL;
   }
}

static bool is_alpha_func(GLenum func)
{
   return func == GL_NEVER ||
      func == GL_ALWAYS ||
      func == GL_LESS ||
      func == GL_LEQUAL ||
      func == GL_EQUAL ||
      func == GL_GREATER ||
      func == GL_GEQUAL ||
      func == GL_NOTEQUAL;
}

static void alpha_func_internal(GLenum func, float ref)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (is_alpha_func(func)) {
      state->changed_misc = true;
      state->alpha_func.func = func;
      state->alpha_func.ref = clampf(ref, 0.0f, 1.0f);
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glAlphaFunc(GLenum func, GLclampf ref)
{
   alpha_func_internal(func, ref);
}

GL_API void GL_APIENTRY glAlphaFuncx(GLenum func, GLclampx ref)
{
   alpha_func_internal(func, fixed_to_float(ref));
}

GL_API void GL_APIENTRY glClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha)
{
   glxx_clear_color_internal(fixed_to_float(red),
      fixed_to_float(green),
      fixed_to_float(blue),
      fixed_to_float(alpha));
}

GL_API void GL_APIENTRY glClearDepthx(GLclampx depth)
{
   glxx_clear_depth_internal(fixed_to_float(depth));
}

static void clip_plane_internal(GLenum p, const float *equation)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   float *plane = get_plane(state, p);

   if (plane) {
      float inv[16];

      assert(state->modelview.pos >= 0 && state->modelview.pos < GL11_CONFIG_MAX_STACK_DEPTH);

      state->changed_misc = true;

      gl11_matrix_invert_4x4(inv, state->modelview.body[state->modelview.pos]);
      gl11_matrix_mult_row(plane, equation, inv);
   }

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glClipPlanef(GLenum plane, const GLfloat *equation)
{
   clip_plane_internal(plane, equation);
}

GL_API void GL_APIENTRY glClipPlanex(GLenum plane, const GLfixed *equation)
{
   int i;
   float temp[4];

   for (i = 0; i < 4; i++)
      temp[i] = fixed_to_float(equation[i]);

   clip_plane_internal(plane, temp);
}

static bool is_fog_mode(GLenum mode)
{
   return mode == GL_EXP ||
      mode == GL_EXP2 ||
      mode == GL_LINEAR;
}

static void fogv_internal(GLenum pname, const GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   switch (pname) {
   case GL_FOG_MODE:
   {
      GLenum m = (GLenum)params[0];

      if (is_fog_mode(m))
      {
         state->changed_misc = true;
         state->fog.mode = m;
      }
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);

      break;
   }
   case GL_FOG_DENSITY:
   {
      GLfloat d = params[0];

      if (d >= 0.0f) {
         state->fog.density = d;

         state->fog.coeff_exp = -d * LOG2E;
         state->fog.coeff_exp2 = -d * d * LOG2E;
      }
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);

      break;
   }
   case GL_FOG_START:
      state->fog.start = params[0];

      state->fog.scale = 1.0f / (state->fog.end - state->fog.start);
      break;
   case GL_FOG_END:
      state->fog.end = params[0];

      state->fog.scale = 1.0f / (state->fog.end - state->fog.start);
      break;
   case GL_FOG_COLOR:
   {
      int i;
      for (i = 0; i < 4; i++)
         state->fog.color[i] = clampf(params[i], 0.0f, 1.0f);
   }
   break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glDepthRangex(GLclampx zNear, GLclampx zFar)
{
   glxx_depth_range_internal(fixed_to_float(zNear),
      fixed_to_float(zFar));
}

GL_API void GL_APIENTRY glFogf(GLenum pname, GLfloat param)
{
   GLfloat params[4];

   params[0] = param;
   params[1] = param;
   params[2] = param;
   params[3] = 1.0f;

   fogv_internal(pname, params);
}

GL_API void GL_APIENTRY glFogfv(GLenum pname, const GLfloat *params)
{
   fogv_internal(pname, params);
}

static GLboolean fog_requires_scaling(GLenum pname)
{
   return pname != GL_FOG_MODE;
}

GL_API void GL_APIENTRY glFogx(GLenum pname, GLfixed param)
{
   GLfloat floatParam = fog_requires_scaling(pname) ? fixed_to_float(param) : (GLfloat)param;
   GLfloat params[4];

   params[0] = floatParam;
   params[1] = floatParam;
   params[2] = floatParam;
   params[3] = 1.0f;

   fogv_internal(pname, params);
}

GL_API void GL_APIENTRY glFogxv(GLenum pname, const GLfixed *params)
{
   GLfloat temp[4];

   if (fog_requires_scaling(pname)) {
      int i;
      for (i = 0; i < 4; i++)
         temp[i] = fixed_to_float(params[i]);
   }
   else {
      int i;
      for (i = 0; i < 4; i++)
         temp[i] = (GLfloat)params[i];
   }

   fogv_internal(pname, temp);
}

static void frustum_internal(float l, float r, float b, float t, float n, float f)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (n > 0.0f && f > 0.0f && l != r && b != t && n != f) {
      float m[16];

      m[0] = 2.0f * n / (r - l);
      m[1] = 0.0f;
      m[2] = 0.0f;
      m[3] = 0.0f;

      m[4] = 0.0f;
      m[5] = 2.0f * n / (t - b);
      m[6] = 0.0f;
      m[7] = 0.0f;

      m[8] = (r + l) / (r - l);
      m[9] = (t + b) / (t - b);
      m[10] = -(f + n) / (f - n);
      m[11] = -1.0f;

      m[12] = 0.0f;
      m[13] = 0.0f;
      m[14] = -2.0f * f * n / (f - n);
      m[15] = 0.0f;

      mult_matrix(state, m);
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
   frustum_internal(left,
      right,
      bottom,
      top,
      zNear,
      zFar);
}

GL_API void GL_APIENTRY glFrustumx(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
   frustum_internal(fixed_to_float(left),
      fixed_to_float(right),
      fixed_to_float(bottom),
      fixed_to_float(top),
      fixed_to_float(zNear),
      fixed_to_float(zFar));
}

static void get_clip_plane_internal(GLenum pname, float eqn[4])
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   float *plane = get_plane(state, pname);

   if (plane) {
      int i;
      for (i = 0; i < 4; i++)
         eqn[i] = plane[i];
   }

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glGetClipPlanef(GLenum pname, GLfloat eqn[4])
{
   get_clip_plane_internal(pname, eqn);
}

GL_API void GL_APIENTRY glGetClipPlanex(GLenum pname, GLfixed eqn[4])
{
   float temp[4];

   int i;

   get_clip_plane_internal(pname, temp);

   for (i = 0; i < 4; i++)
      eqn[i] = float_to_fixed(temp[i]);
}

GL_API void GL_APIENTRY glGetFixedv(GLenum pname, GLfixed *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   GLfloat temp[16];
   int count = glxx_get_float_or_fixed(state, pname, temp);

   assert(count <= 16);

   for (int i = 0; i < count; i++)
      params[i] = float_to_fixed(temp[i]);

   glxx_unlock_server_state(OPENGL_ES_11);
}

static int get_lightv_internal(GLenum l, GLenum pname, float *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return 0;

   GL11_LIGHT_T *light = get_light(state, l);

   int result;

   if (light)
      switch (pname) {
      case GL_AMBIENT:
      {
         int i;
         for (i = 0; i < 4; i++)
            params[i] = light->ambient[i];
         result = 4;
         break;
      }
      case GL_DIFFUSE:
      {
         int i;
         for (i = 0; i < 4; i++)
            params[i] = light->diffuse[i];
         result = 4;
         break;
      }
      case GL_SPECULAR:
      {
         int i;
         for (i = 0; i < 4; i++)
            params[i] = light->specular[i];
         result = 4;
         break;
      }
      case GL_POSITION:
      {
         int i;
         for (i = 0; i < 4; i++)
            params[i] = light->position[i];
         result = 4;
         break;
      }
      case GL_CONSTANT_ATTENUATION:
         params[0] = light->attenuation.constant;
         result = 1;
         break;
      case GL_LINEAR_ATTENUATION:
         params[0] = light->attenuation.linear;
         result = 1;
         break;
      case GL_QUADRATIC_ATTENUATION:
         params[0] = light->attenuation.quadratic;
         result = 1;
         break;
      case GL_SPOT_DIRECTION:
      {
         int i;
         for (i = 0; i < 3; i++)
            params[i] = light->spot.direction[i];
         result = 3;
         break;
      }
      case GL_SPOT_EXPONENT:
         params[0] = light->spot.exponent;
         result = 1;
         break;
      case GL_SPOT_CUTOFF:
         params[0] = light->spot.cutoff;
         result = 1;
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
         break;
      }
   else
      result = 0;

   glxx_unlock_server_state(OPENGL_ES_11);

   return result;
}

GL_API void GL_APIENTRY glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
   get_lightv_internal(light, pname, params);
}

GL_API void GL_APIENTRY glGetLightxv(GLenum light, GLenum pname, GLfixed *params)
{
   float temp[4];
   int count = get_lightv_internal(light, pname, temp);

   assert(count <= 4);

   for (int i = 0; i < count; i++)
      params[i] = float_to_fixed(temp[i]);
}

static GLboolean is_single_face(GLenum face)
{
   return face == GL_FRONT ||
      face == GL_BACK;
}

static int get_materialv_internal(GLenum face, GLenum pname, float *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return 0;

   int result;

   if (is_single_face(face))
      switch (pname) {
      case GL_AMBIENT:
      {
         int i;
         for (i = 0; i < 4; i++)
            params[i] = state->material.ambient[i];
         result = 4;
      }
      break;
      case GL_DIFFUSE:
      {
         int i;
         for (i = 0; i < 4; i++)
            params[i] = state->material.diffuse[i];
         result = 4;
         break;
      }
      case GL_SPECULAR:
      {
         int i;
         for (i = 0; i < 4; i++)
            params[i] = state->material.specular[i];
         result = 4;
         break;
      }
      case GL_EMISSION:
      {
         int i;
         for (i = 0; i < 4; i++)
            params[i] = state->material.emission[i];
         result = 4;
         break;
      }
      case GL_SHININESS:
         params[0] = state->material.shininess;
         result = 1;
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
         break;
      }
   else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      result = 0;
   }

   glxx_unlock_server_state(OPENGL_ES_11);

   return result;
}

GL_API void GL_APIENTRY glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
   get_materialv_internal(face, pname, params);
}

GL_API void GL_APIENTRY glGetMaterialxv(GLenum face, GLenum pname, GLfixed *params)
{
   int i;
   float temp[4];

   int count = get_materialv_internal(face, pname, temp);

   assert(count <= 4);

   for (i = 0; i < count; i++)
      params[i] = float_to_fixed(temp[i]);
}

GL_API void GL_APIENTRY glGetTexEnviv(GLenum env, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   switch (env) {
   case GL_POINT_SPRITE_OES:
      switch (pname) {
      case GL_COORD_REPLACE_OES:
         get_texenv_integer(state, env, pname, params);
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
      break;

   case GL_TEXTURE_ENV:
      switch (pname) {
      case GL_TEXTURE_ENV_MODE:
      case GL_COMBINE_RGB:
      case GL_COMBINE_ALPHA:
      case GL_SRC0_RGB:
      case GL_SRC1_RGB:
      case GL_SRC2_RGB:
      case GL_SRC0_ALPHA:
      case GL_SRC1_ALPHA:
      case GL_SRC2_ALPHA:
      case GL_OPERAND0_RGB:
      case GL_OPERAND1_RGB:
      case GL_OPERAND2_RGB:
      case GL_OPERAND0_ALPHA:
      case GL_OPERAND1_ALPHA:
      case GL_OPERAND2_ALPHA:
         get_texenv_integer(state, env, pname, params);
         break;
      case GL_TEXTURE_ENV_COLOR:
      {
         float temp[4];
         int count = get_texenv_float(state, env, pname, temp);

         assert(count <= 4);

         for (int i = 0; i < count; i++) {
            params[i] = (GLint)floor((4294967295.0f * temp[i] - 1.0f) / 2.0f + 0.5f);

            if (params[i] < 0)
               params[i] = 0x7fffffff;
         }
         break;
      }
      case GL_RGB_SCALE:
      case GL_ALPHA_SCALE:
      {
         float temp;
         GLuint count = get_texenv_float(state, env, pname, &temp);
         assert(count == 1);
         params[0] = float_to_int(temp);
         break;
      }
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
      break;

   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }
}

static int get_texenv_float_or_fixed_internal(GLenum env, GLenum pname, float *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return 0;

   int result = 0;
   switch (env) {
   case GL_POINT_SPRITE_OES:
      switch (pname) {
      case GL_COORD_REPLACE_OES:
      {
         int temp;
         int count = get_texenv_integer(state, env, pname, &temp);
         assert(count == 1);
         params[0] = (float)temp;
         result = count;
         break;
      }
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
      break;

   case GL_TEXTURE_ENV:
      switch (pname) {
      case GL_TEXTURE_ENV_MODE:
      case GL_COMBINE_RGB:
      case GL_COMBINE_ALPHA:
      case GL_SRC0_RGB:
      case GL_SRC1_RGB:
      case GL_SRC2_RGB:
      case GL_SRC0_ALPHA:
      case GL_SRC1_ALPHA:
      case GL_SRC2_ALPHA:
      case GL_OPERAND0_RGB:
      case GL_OPERAND1_RGB:
      case GL_OPERAND2_RGB:
      case GL_OPERAND0_ALPHA:
      case GL_OPERAND1_ALPHA:
      case GL_OPERAND2_ALPHA:
      {
         int temp;
         int count = get_texenv_integer(state, env, pname, &temp);
         assert(count == 1);
         params[0] = (float)temp;
         result = count;
         break;
      }
      case GL_TEXTURE_ENV_COLOR:
      case GL_RGB_SCALE:
      case GL_ALPHA_SCALE:
         result = get_texenv_float(state, env, pname, params);
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
      break;

   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

   glxx_unlock_server_state(OPENGL_ES_11);

   return result;
}

GL_API void GL_APIENTRY glGetTexEnvfv(GLenum env, GLenum pname, GLfloat *params)
{
   get_texenv_float_or_fixed_internal(env, pname, params);
   return;
}

static void lightmodelv_internal(GLenum pname, const GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   switch (pname) {
   case GL_LIGHT_MODEL_AMBIENT:
   {
      int i;
      for (i = 0; i < 4; i++)
         state->lightmodel.ambient[i] = params[i];
      break;
   }
   case GL_LIGHT_MODEL_TWO_SIDE:
      state->changed_light = true;
      state->lightmodel.two_side = params[0] != 0.0f;
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glLightModelf(GLenum pname, GLfloat param)
{
   GLfloat params[4];

   params[0] = param;
   params[1] = param;
   params[2] = param;
   params[3] = 1.0f;

   lightmodelv_internal(pname, params);
}

GL_API void GL_APIENTRY glLightModelfv(GLenum pname, const GLfloat *params)
{
   lightmodelv_internal(pname, params);
}

GL_API void GL_APIENTRY glLightModelx(GLenum pname, GLfixed param)
{
   GLfloat params[4];

   params[0] = fixed_to_float(param);
   params[1] = fixed_to_float(param);
   params[2] = fixed_to_float(param);
   params[3] = 1.0f;

   lightmodelv_internal(pname, params);
}

GL_API void GL_APIENTRY glLightModelxv(GLenum pname, const GLfixed *params)
{
   int i;
   GLfloat temp[4];

   for (i = 0; i < 4; i++)
      temp[i] = fixed_to_float(params[i]);

   lightmodelv_internal(pname, temp);
}

GL_API void GL_APIENTRY glLineWidthx(GLfixed width)
{
   glxx_line_width_internal(fixed_to_float(width));
}

GL_API void GL_APIENTRY glLoadIdentity(void)
{
   float m[16];

   m[0] = 1.0f;
   m[1] = 0.0f;
   m[2] = 0.0f;
   m[3] = 0.0f;

   m[4] = 0.0f;
   m[5] = 1.0f;
   m[6] = 0.0f;
   m[7] = 0.0f;

   m[8] = 0.0f;
   m[9] = 0.0f;
   m[10] = 1.0f;
   m[11] = 0.0f;

   m[12] = 0.0f;
   m[13] = 0.0f;
   m[14] = 0.0f;
   m[15] = 1.0f;

   load_matrix_internal(m);
}

GL_API void GL_APIENTRY glLoadMatrixf(const GLfloat *m)
{
   load_matrix_internal(m);
}

GL_API void GL_APIENTRY glLoadMatrixx(const GLfixed *m)
{
   float f[16];

   for (int i = 0; i < 16; i++)
      f[i] = fixed_to_float(m[i]);

   load_matrix_internal(f);
}

GL_API void GL_APIENTRY glMultMatrixf(const GLfloat *m)
{
   mult_matrix_internal(m);
}

GL_API void GL_APIENTRY glMultMatrixx(const GLfixed *m)
{
   float f[16];

   for (int i = 0; i < 16; i++)
      f[i] = fixed_to_float(m[i]);

   mult_matrix_internal(f);
}

static void rotate_internal(float angle, float x, float y, float z)
{
   float s, c;
   float m[16];

   /*
   normalize axis vector
   */
   if (x != 0.0f || y != 0.0f || z != 0.0f)
   {
      float l = (float)sqrt(x * x + y * y + z * z);

      x = x / l;
      y = y / l;
      z = z / l;
   }
   /*
   convert angle to radians
   */

   angle = 2.0f * PI * angle / 360.0f;

   /*
   build matrix
   */

   s = (float)sin(angle);
   c = (float)cos(angle);

   m[0] = x * x * (1 - c) + c;
   m[1] = y * x * (1 - c) + z * s;
   m[2] = z * x * (1 - c) - y * s;
   m[3] = 0.0f;

   m[4] = x * y * (1 - c) - z * s;
   m[5] = y * y * (1 - c) + c;
   m[6] = z * y * (1 - c) + x * s;
   m[7] = 0.0f;

   m[8] = x * z * (1 - c) + y * s;
   m[9] = y * z * (1 - c) - x * s;
   m[10] = z * z * (1 - c) + c;
   m[11] = 0.0f;

   m[12] = 0.0f;
   m[13] = 0.0f;
   m[14] = 0.0f;
   m[15] = 1.0f;

   mult_matrix_internal(m);
}

GL_API void GL_APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
   rotate_internal(angle, x, y, z);
}

GL_API void GL_APIENTRY glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
   rotate_internal(fixed_to_float(angle),
      fixed_to_float(x),
      fixed_to_float(y),
      fixed_to_float(z));
}

static void scale_internal(float x, float y, float z)
{
   float m[16];

   m[0] = x;
   m[1] = 0.0f;
   m[2] = 0.0f;
   m[3] = 0.0f;

   m[4] = 0.0f;
   m[5] = y;
   m[6] = 0.0f;
   m[7] = 0.0f;

   m[8] = 0.0f;
   m[9] = 0.0f;
   m[10] = z;
   m[11] = 0.0f;

   m[12] = 0.0f;
   m[13] = 0.0f;
   m[14] = 0.0f;
   m[15] = 1.0f;

   mult_matrix_internal(m);
}

GL_API void GL_APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z)
{
   scale_internal(x, y, z);
}

GL_API void GL_APIENTRY glScalex(GLfixed x, GLfixed y, GLfixed z)
{
   scale_internal(fixed_to_float(x),
      fixed_to_float(y),
      fixed_to_float(z));
}

static void translate_internal(float x, float y, float z)
{
   float m[16];

   m[0] = 1.0f;
   m[1] = 0.0f;
   m[2] = 0.0f;
   m[3] = 0.0f;

   m[4] = 0.0f;
   m[5] = 1.0f;
   m[6] = 0.0f;
   m[7] = 0.0f;

   m[8] = 0.0f;
   m[9] = 0.0f;
   m[10] = 1.0f;
   m[11] = 0.0f;

   m[12] = x;
   m[13] = y;
   m[14] = z;
   m[15] = 1.0f;

   mult_matrix_internal(m);
}

GL_API void GL_APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
   translate_internal(x, y, z);
}

GL_API void GL_APIENTRY glTranslatex(GLfixed x, GLfixed y, GLfixed z)
{
   translate_internal(fixed_to_float(x),
      fixed_to_float(y),
      fixed_to_float(z));
}

static void ortho_internal(float l, float r, float b, float t, float n, float f)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (l != r && b != t && n != f) {
      float m[16];

      m[0] = 2.0f / (r - l);
      m[1] = 0.0f;
      m[2] = 0.0f;
      m[3] = 0.0f;

      m[4] = 0.0f;
      m[5] = 2.0f / (t - b);
      m[6] = 0.0f;
      m[7] = 0.0f;

      m[8] = 0.0f;
      m[9] = 0.0f;
      m[10] = -2.0f / (f - n);
      m[11] = 0.0f;

      m[12] = -(r + l) / (r - l);
      m[13] = -(t + b) / (t - b);
      m[14] = -(f + n) / (f - n);
      m[15] = 1.0f;

      mult_matrix(state, m);
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
   ortho_internal(left, right, bottom, top, zNear, zFar);
}

GL_API void GL_APIENTRY glOrthox(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
   ortho_internal(fixed_to_float(left),
      fixed_to_float(right),
      fixed_to_float(bottom),
      fixed_to_float(top),
      fixed_to_float(zNear),
      fixed_to_float(zFar));
}

GL_API void GL_APIENTRY glPolygonOffsetx(GLfixed factor, GLfixed units)
{
   glxx_polygon_offset_internal(fixed_to_float(factor),
      fixed_to_float(units));
}

GL_API void GL_APIENTRY glPopMatrix(void)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   GL11_MATRIX_STACK_T *stack = get_stack(state);

   if (stack->pos > 0)
      stack->pos--;
   else
      glxx_server_state_set_error(state, GL_STACK_UNDERFLOW);

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glPushMatrix(void)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   GL11_MATRIX_STACK_T *stack = get_stack(state);

   if (stack->pos + 1 < GL11_CONFIG_MAX_STACK_DEPTH) {
      gl11_matrix_load(stack->body[stack->pos + 1], stack->body[stack->pos]);

      stack->pos++;
   }
   else
      glxx_server_state_set_error(state, GL_STACK_OVERFLOW);

   glxx_unlock_server_state(OPENGL_ES_11);
}

bool gl11_server_state_init(GLXX_SERVER_STATE_T *state, GLXX_SHARED_T *shared, bool secure)
{
   state->type = OPENGL_ES_11;

   state->secure = secure;

   //initialise common portions of state
   if(!glxx_server_state_init(state, shared))
      return false;

   //gl 1.1 specific parts
   state->material.ambient[0] = 0.2f;
   state->material.ambient[1] = 0.2f;
   state->material.ambient[2] = 0.2f;
   state->material.ambient[3] = 1.0f;

   state->material.diffuse[0] = 0.8f;
   state->material.diffuse[1] = 0.8f;
   state->material.diffuse[2] = 0.8f;
   state->material.diffuse[3] = 1.0f;

   state->material.specular[0] = 0.0f;
   state->material.specular[1] = 0.0f;
   state->material.specular[2] = 0.0f;
   state->material.specular[3] = 1.0f;

   state->material.emission[0] = 0.0f;
   state->material.emission[1] = 0.0f;
   state->material.emission[2] = 0.0f;
   state->material.emission[3] = 1.0f;

   state->material.shininess = 0.0f;

   state->lightmodel.ambient[0] = 0.2f;
   state->lightmodel.ambient[1] = 0.2f;
   state->lightmodel.ambient[2] = 0.2f;
   state->lightmodel.ambient[3] = 1.0f;

   state->lightmodel.two_side = false;

   state->shader.two_side = false;

   for (int i = 0; i < GL11_CONFIG_MAX_LIGHTS; i++) {
      GL11_LIGHT_T *light = &state->lights[i];

      light->enabled = false;

      light->ambient[0] = 0.0f;
      light->ambient[1] = 0.0f;
      light->ambient[2] = 0.0f;
      light->ambient[3] = 1.0f;

      light->diffuse[0] = i ? 0.0f : 1.0f;
      light->diffuse[1] = i ? 0.0f : 1.0f;
      light->diffuse[2] = i ? 0.0f : 1.0f;
      light->diffuse[3] = 1.0f;

      light->specular[0] = i ? 0.0f : 1.0f;
      light->specular[1] = i ? 0.0f : 1.0f;
      light->specular[2] = i ? 0.0f : 1.0f;
      light->specular[3] = 1.0f;

      light->position[0] = 0.0f;
      light->position[1] = 0.0f;
      light->position[2] = 1.0f;
      light->position[3] = 0.0f;

      light->attenuation.constant = 1.0f;
      light->attenuation.linear = 0.0f;
      light->attenuation.quadratic = 0.0f;

      light->spot.direction[0] = 0.0f;
      light->spot.direction[1] = 0.0f;
      light->spot.direction[2] = -1.0f;
      light->spot.exponent = 0.0f;
      light->spot.cutoff = 180.0f;

      light->position3[0] = 0.0f;
      light->position3[1] = 0.0f;
      light->position3[2] = 1.0f;

      light->cos_cutoff = -1.0f;
   }

   for (int i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
      GL11_TEXUNIT_T *texunit = &state->texunits[i];
      GL11_CACHE_TEXUNIT_ABSTRACT_T *texabs = &state->shader.texunits[i];

      texunit->target_enabled = GL_NONE;

      texabs->mode = GL_MODULATE;

      texunit->color[0] = 0.0f;
      texunit->color[1] = 0.0f;
      texunit->color[2] = 0.0f;
      texunit->color[3] = 0.0f;

      texunit->rgb.combine = GL_MODULATE;
      texunit->rgb.source[0] = GL_TEXTURE;
      texunit->rgb.source[1] = GL_PREVIOUS;
      texunit->rgb.source[2] = GL_CONSTANT;
      texunit->rgb.operand[0] = GL_SRC_COLOR;
      texunit->rgb.operand[1] = GL_SRC_COLOR;
      texunit->rgb.operand[2] = GL_SRC_ALPHA;
      texunit->rgb.scale = 1.0f;

      texunit->alpha.combine = GL_MODULATE;
      texunit->alpha.source[0] = GL_TEXTURE;
      texunit->alpha.source[1] = GL_PREVIOUS;
      texunit->alpha.source[2] = GL_CONSTANT;
      texunit->alpha.operand[0] = GL_SRC_ALPHA;
      texunit->alpha.operand[1] = GL_SRC_ALPHA;
      texunit->alpha.operand[2] = GL_SRC_ALPHA;
      texunit->alpha.scale = 1.0f;

      texabs->coord_replace = false;

      gl11_matrix_stack_init(&texunit->stack);
   }

   state->fog.mode = GL_EXP;

   state->fog.color[0] = 0.0f;
   state->fog.color[1] = 0.0f;
   state->fog.color[2] = 0.0f;
   state->fog.color[3] = 0.0f;

   state->fog.density = 1.0f;
   state->fog.start = 0.0f;
   state->fog.end = 1.0f;

   state->fog.scale = 1.0f;
   state->fog.coeff_exp = -LOG2E;
   state->fog.coeff_exp2 = -LOG2E;

   state->shader.normalize = false;
   state->shader.rescale_normal = false;
   state->shader.lighting = false;
   state->shader.color_material = false;

   state->caps_fragment.fog = false;
   state->caps_fragment.alpha_test = false;
   state->caps_fragment.point_smooth = false;
   state->caps_fragment.point_sprite = false;
   state->shader.line_smooth = false;

   state->hints_program.fog = GL_DONT_CARE;

   for (int i = 0; i < GL11_CONFIG_MAX_PLANES; i++) {
      state->planes[i][0] = 0.0f;
      state->planes[i][1] = 0.0f;
      state->planes[i][2] = 0.0f;
      state->planes[i][3] = 0.0f;
   }

   for (int i = 0; i < GL11_CONFIG_MAX_PLANES; i++)
      state->caps.clip_plane[i] = false;

   state->shade_model = GL_SMOOTH;

   state->logic_op = GL_COPY;

   state->matrix_mode = GL_MODELVIEW;

   gl11_matrix_stack_init(&state->modelview);
   gl11_matrix_stack_init(&state->projection);

   state->alpha_func.func = GL_ALWAYS;
   state->alpha_func.ref = 0.0f;

   state->caps.sample_alpha_to_one = GL_FALSE;
   state->caps.color_logic_op = GL_FALSE;

   state->hints.perspective_correction = GL_DONT_CARE;
   state->hints.point_smooth = GL_DONT_CARE;
   state->hints.line_smooth = GL_DONT_CARE;

   state->point_params.size_min = 1.0f;
   state->point_params.size_min_clamped = 1.0f;
   state->point_params.size_max = 256.0f;
   state->point_params.fade_threshold = 1.0f;
   state->point_params.distance_attenuation[0] = 1.0f;
   state->point_params.distance_attenuation[1] = 0.0f;
   state->point_params.distance_attenuation[2] = 0.0f;

   state->client_active_texture = GL_TEXTURE0;
   state->active_texture = GL_TEXTURE0;

   //color
   state->attrib[GL11_IX_COLOR].size = 4;
   state->attrib[GL11_IX_COLOR].normalized = GL_TRUE;
   state->attrib[GL11_IX_COLOR].value[0] = 1.0f;
   state->attrib[GL11_IX_COLOR].value[1] = 1.0f;
   state->attrib[GL11_IX_COLOR].value[2] = 1.0f;
   state->attrib[GL11_IX_COLOR].value[3] = 1.0f;

   //normal
   state->attrib[GL11_IX_NORMAL].size = 3;
   state->attrib[GL11_IX_NORMAL].normalized = GL_TRUE;
   state->attrib[GL11_IX_NORMAL].value[0] = 0.0f;
   state->attrib[GL11_IX_NORMAL].value[1] = 0.0f;
   state->attrib[GL11_IX_NORMAL].value[2] = 1.0f;

   //vertex
   state->attrib[GL11_IX_VERTEX].size = 4;
   state->attrib[GL11_IX_VERTEX].normalized = GL_FALSE;
   state->attrib[GL11_IX_VERTEX].value[0] = 0.0f;
   state->attrib[GL11_IX_VERTEX].value[1] = 0.0f;
   state->attrib[GL11_IX_VERTEX].value[2] = 0.0f;
   state->attrib[GL11_IX_VERTEX].value[3] = 1.0f;

   for (int i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
      int indx = GL11_IX_TEXTURE_COORD + i;
      state->attrib[indx].size = 4;
      state->attrib[indx].normalized = GL_FALSE;
      state->attrib[indx].value[0] = 0.0f;
      state->attrib[indx].value[1] = 0.0f;
      state->attrib[indx].value[2] = 0.0f;
      state->attrib[indx].value[3] = 1.0f;
   }

   //point size
   state->attrib[GL11_IX_POINT_SIZE].size = 1;
   state->attrib[GL11_IX_POINT_SIZE].normalized = GL_FALSE;
   state->attrib[GL11_IX_POINT_SIZE].value[0] = 1.0f;

   glxx_tweaker_init(&state->tweak_state, false);

   return true;
}

GL_API void GL_APIENTRY glGetTexParameterxv(GLenum target, GLenum pname, GLfixed *params)
{
   GLint temp[4];
   GLuint count = glxx_get_texparameter_internal(target, pname, temp);

   if (count) {
      unsigned int i;
      assert(count == 1 || count == 4);
      for(i=0;i<count;i++)
         params[i] = (GLfixed)temp[i];
   }
}

static void point_parameterv_internal(GLenum pname, const GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   switch (pname) {
   case GL_POINT_SIZE_MIN:
   {
      GLfloat m = clean_float(params[0]);

      if (m >= 0)
      {
         state->point_params.size_min = m;
         state->point_params.size_min_clamped = _maxf(1.0f, state->point_params.size_min);
      }
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);

      break;
   }
   case GL_POINT_SIZE_MAX:
   {
      GLfloat m = clean_float(params[0]);

      if (m >= 0)
         state->point_params.size_max = m;
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);

      break;
   }
   case GL_POINT_FADE_THRESHOLD_SIZE:
   {
      GLfloat m = clean_float(params[0]);

      if (m >= 0)
         state->point_params.fade_threshold = m;
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);

      break;
   }
   case GL_POINT_DISTANCE_ATTENUATION:
   {
      int i;
      for (i = 0; i < 3; i++)
         state->point_params.distance_attenuation[i] = clean_float(params[i]);

      break;
   }
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glPointParameterf(GLenum pname, GLfloat param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f};

   params[0] = param;

   point_parameterv_internal(pname, params);
}

GL_API void GL_APIENTRY glPointParameterfv(GLenum pname, const GLfloat *params)
{
   point_parameterv_internal(pname, params);
}

static GLboolean is_texture_function(GLenum mode)
{
   return mode == GL_REPLACE ||
          mode == GL_MODULATE ||
          mode == GL_DECAL ||
          mode == GL_BLEND ||
          mode == GL_ADD ||
          mode == GL_COMBINE;
}

static GLboolean is_combine_rgb(GLenum combine)
{
   return combine == GL_REPLACE ||
          combine == GL_MODULATE ||
          combine == GL_ADD ||
          combine == GL_ADD_SIGNED ||
          combine == GL_INTERPOLATE ||
          combine == GL_SUBTRACT ||
          combine == GL_DOT3_RGB ||
          combine == GL_DOT3_RGBA;
}

static GLboolean is_combine_alpha(GLenum combine)
{
   return combine == GL_REPLACE ||
          combine == GL_MODULATE ||
          combine == GL_ADD ||
          combine == GL_ADD_SIGNED ||
          combine == GL_INTERPOLATE ||
          combine == GL_SUBTRACT;
}

static GLboolean is_source(GLenum source)
{
   return source == GL_TEXTURE ||
          source == GL_CONSTANT ||
          source == GL_PRIMARY_COLOR ||
          source == GL_PREVIOUS;
}

static GLboolean is_operand_rgb(GLenum operand)
{
   return operand == GL_SRC_COLOR ||
          operand == GL_ONE_MINUS_SRC_COLOR ||
          operand == GL_SRC_ALPHA ||
          operand == GL_ONE_MINUS_SRC_ALPHA;
}

static GLboolean is_operand_alpha(GLenum operand)
{
   return operand == GL_SRC_ALPHA ||
          operand == GL_ONE_MINUS_SRC_ALPHA;
}

static GLboolean is_scalef(GLfloat scale)
{
   return scale == 1.0f ||
          scale == 2.0f ||
          scale == 4.0f;
}

static void texenvfv_internal(GLenum target, GLenum pname, const GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   int t = state->active_texture - GL_TEXTURE0;

   switch (target) {
   case GL_POINT_SPRITE_OES:
      switch (pname) {
      case GL_COORD_REPLACE_OES:
         state->changed_texunit = true;
         state->shader.texunits[t].coord_replace = (params[0] != 0.0f);
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
      break;

   case GL_TEXTURE_ENV:
      switch (pname) {
      case GL_TEXTURE_ENV_MODE:
      {
         GLenum mode = (GLenum)params[0];

         if (is_texture_function(mode))
         {
            state->changed_texunit = true;
            state->shader.texunits[t].mode = mode;
         }
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);

         break;
      }
      case GL_TEXTURE_ENV_COLOR:
         {
            int i;
            state->changed_texunit = true;
            for (i = 0; i < 4; i++)
               state->texunits[t].color[i] = clampf(params[i], 0.0f, 1.0f);
         }
         break;
      case GL_COMBINE_RGB:
      {
         GLenum combine = (GLenum)params[0];

         if (is_combine_rgb(combine))
         {
            state->changed_texunit = true;
            state->texunits[t].rgb.combine = combine;
         }
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);

         break;
      }
      case GL_RGB_SCALE:
      {
         if (is_scalef(params[0]))
         {
            state->changed_texunit = true;
            state->texunits[t].rgb.scale = params[0];
         }
         else
            glxx_server_state_set_error(state, GL_INVALID_VALUE);

         break;
      }
      case GL_SRC0_RGB:
      case GL_SRC1_RGB:
      case GL_SRC2_RGB:
      {
         GLenum source = (GLenum)params[0];

         if (is_source(source))
         {
            state->changed_texunit = true;
            state->texunits[t].rgb.source[pname - GL_SRC0_RGB] = source;
         }
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);

         break;
      }
      case GL_OPERAND0_RGB:
      case GL_OPERAND1_RGB:
      case GL_OPERAND2_RGB:
      {
         GLenum operand = (GLenum)params[0];

         if (is_operand_rgb(operand))
         {
            state->changed_texunit = true;
            state->texunits[t].rgb.operand[pname - GL_OPERAND0_RGB] = operand;
         }
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);

         break;
      }
      case GL_COMBINE_ALPHA:
      {
         GLenum combine = (GLenum)params[0];

         if (is_combine_alpha(combine))
         {
            state->changed_texunit = true;
            state->texunits[t].alpha.combine = combine;
         }
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);

         break;
      }
      case GL_ALPHA_SCALE:
      {
         if (is_scalef(params[0]))
         {
            state->changed_texunit = true;
            state->texunits[t].alpha.scale = params[0];
         }
         else
            glxx_server_state_set_error(state, GL_INVALID_VALUE);

         break;
      }
      case GL_SRC0_ALPHA:
      case GL_SRC1_ALPHA:
      case GL_SRC2_ALPHA:
      {
         GLenum source = (GLenum)params[0];

         if (is_source(source))
         {
            state->changed_texunit = true;
            state->texunits[t].alpha.source[pname - GL_SRC0_ALPHA] = source;
         }
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);

         break;
      }
      case GL_OPERAND0_ALPHA:
      case GL_OPERAND1_ALPHA:
      case GL_OPERAND2_ALPHA:
      {
         GLenum operand = (GLenum)params[0];

         if (is_operand_alpha(operand))
         {
            state->changed_texunit = true;
            state->texunits[t].alpha.operand[pname - GL_OPERAND0_ALPHA] = operand;
         }
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);

         break;
      }
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
      break;

   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = param;

   texenvfv_internal(target, pname, params);
}

GL_API void GL_APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
   texenvfv_internal(target, pname, params);
}

static void lightv_internal(GLenum l, GLenum pname, const GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   GL11_LIGHT_T *light = get_light(state, l);

   if (light)
      switch (pname) {
      case GL_AMBIENT:
      {
         int i;
         for (i = 0; i < 4; i++)
            light->ambient[i] = clean_float(params[i]);
         break;
      }
      case GL_DIFFUSE:
      {
         int i;
         for (i = 0; i < 4; i++)
            light->diffuse[i] = clean_float(params[i]);
         break;
      }
      case GL_SPECULAR:
      {
         int i;
         for (i = 0; i < 4; i++)
            light->specular[i] = clean_float(params[i]);
         break;
      }
      case GL_POSITION:
      {
         GLfloat clean[4];

         int i;

         for(i = 0; i < 4; i++)
            clean[i] = clean_float(params[i]);

         gl11_matrix_mult_col(light->position, state->modelview.body[state->modelview.pos], clean);

         /*
            compute 3-vector position of the light for use by the HAL
         */

         state->changed_light = true;  /* Shader changes if position[3]==0 */
         if (light->position[3] != 0.0f)
            for (i = 0; i < 3; i++)
               light->position3[i] = light->position[i] / light->position[3];

         break;
      }
      case GL_SPOT_DIRECTION:
      {
         int i;
         GLfloat clean[4];

         for(i = 0; i < 3; i++)
            clean[i] = clean_float(params[i]);

         clean[3] = 0.0f;

         gl11_matrix_mult_col(light->spot.direction, state->modelview.body[state->modelview.pos], clean);
         break;
      }
      case GL_SPOT_EXPONENT:
         light->spot.exponent = clean_float(params[0]);
         break;
      case GL_SPOT_CUTOFF:
         state->changed_light = true;  /* Shader changes if cutoff==180 */
         light->spot.cutoff = clean_float(params[0]);

         light->cos_cutoff = (GLfloat)cos(2.0f * PI * light->spot.cutoff / 360.0f);        // derived value
         break;
      case GL_CONSTANT_ATTENUATION:
         light->attenuation.constant = clean_float(params[0]);
         break;
      case GL_LINEAR_ATTENUATION:
         light->attenuation.linear = clean_float(params[0]);
         break;
      case GL_QUADRATIC_ATTENUATION:
         light->attenuation.quadratic = clean_float(params[0]);
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = param;

   lightv_internal(light, pname, params);
}

GL_API void GL_APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
   lightv_internal(light, pname, params);
}

GL_API void GL_APIENTRY glLightx(GLenum light, GLenum pname, GLfixed param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = fixed_to_float(param);

   lightv_internal(light, pname, params);
}

GL_API void GL_APIENTRY glLightxv(GLenum light, GLenum pname, const GLfixed *params)
{
   int i;
   GLfloat temp[4];

   for (i = 0; i < 4; i++)
      temp[i] = fixed_to_float(params[i]);

   lightv_internal(light, pname, temp);
}

static GLboolean is_logic_op(GLenum op)
{
   return op == GL_CLEAR ||
          op == GL_AND ||
          op == GL_AND_REVERSE ||
          op == GL_COPY ||
          op == GL_AND_INVERTED ||
          op == GL_NOOP ||
          op == GL_XOR ||
          op == GL_OR ||
          op == GL_NOR ||
          op == GL_EQUIV ||
          op == GL_INVERT ||
          op == GL_OR_REVERSE ||
          op == GL_COPY_INVERTED ||
          op == GL_OR_INVERTED ||
          op == GL_NAND ||
          op == GL_SET;
}

GL_API void GL_APIENTRY glLogicOp(GLenum opcode)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (is_logic_op(opcode))
   {
      state->changed_backend = true;
      state->logic_op = opcode;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_11);
}

static GLboolean is_matrix_mode(GLenum mode)
{
   return mode == GL_TEXTURE ||
          mode == GL_MODELVIEW ||
          mode == GL_PROJECTION;
}

GL_API void GL_APIENTRY glMatrixMode(GLenum mode)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (is_matrix_mode(mode))
      state->matrix_mode = mode;
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_11);
}

static void materialv_internal (GLenum face, GLenum pname, const GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (face == GL_FRONT_AND_BACK) {
      switch (pname) {
      case GL_AMBIENT:
         if (!state->shader.color_material) {
            int i;
            for (i = 0; i < 4; i++)
               state->material.ambient[i] = clean_float(params[i]);
         }
         break;
      case GL_DIFFUSE:
      {
         if (!state->shader.color_material) {
            int i;
            for (i = 0; i < 4; i++)
               state->material.diffuse[i] = clean_float(params[i]);
         }
         break;
      }
      case GL_AMBIENT_AND_DIFFUSE:
      {
         if (!state->shader.color_material) {
            int i;
            for (i = 0; i < 4; i++) {
               GLfloat f = clean_float(params[i]);

               state->material.ambient[i] = f;
               state->material.diffuse[i] = f;
            }
         }
         break;
      }
      case GL_SPECULAR:
      {
         int i;
         for (i = 0; i < 4; i++)
            state->material.specular[i] = clean_float(params[i]);
         break;
      }
      case GL_EMISSION:
      {
         int i;
         for (i = 0; i < 4; i++)
            state->material.emission[i] = clean_float(params[i]);
         break;
      }
      case GL_SHININESS:
         state->material.shininess = clean_float(params[0]);
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = param;

   materialv_internal(face, pname, params);
}

GL_API void GL_APIENTRY glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
   materialv_internal(face, pname, params);
}

GL_API void GL_APIENTRY glMaterialx(GLenum face, GLenum pname, GLfixed param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = fixed_to_float(param);

   materialv_internal(face, pname, params);
}

GL_API void GL_APIENTRY glMaterialxv(GLenum face, GLenum pname, const GLfixed *params)
{
   int i;
   GLfloat temp[4];

   for (i = 0; i < 4; i++)
      temp[i] = fixed_to_float(params[i]);

   materialv_internal(face, pname, temp);
}

GL_API void GL_APIENTRY glPointParameterx(GLenum pname, GLfixed param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f};

   params[0] = fixed_to_float(param);

   point_parameterv_internal(pname, params);
}

GL_API void GL_APIENTRY glPointParameterxv(GLenum pname, const GLfixed *params)
{
   int i;
   GLfloat temp[3];

   for (i = 0; i < 3; i++)
      temp[i] = fixed_to_float(params[i]);

   point_parameterv_internal(pname, temp);
}

GL_API void GL_APIENTRY glSampleCoveragex(GLclampx value, GLboolean invert)
{
   glxx_sample_coverage_internal(fixed_to_float(value), invert);
}

static GLboolean is_shade_model(GLenum model)
{
   return model == GL_SMOOTH ||
          model == GL_FLAT;
}

GL_API void GL_APIENTRY glShadeModel(GLenum model)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (is_shade_model(model))
      state->shade_model = model;
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_11);
}

static GLboolean texenv_requires_scaling(GLenum pname)
{
   return pname == GL_TEXTURE_ENV_COLOR ||
          pname == GL_RGB_SCALE ||
          pname == GL_ALPHA_SCALE;
}

GL_API void GL_APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param)
{
   float params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = (float)param;

   texenvfv_internal(target, pname, params);
}

GL_API void GL_APIENTRY glTexEnvx(GLenum target, GLenum pname, GLfixed param)
{
   float params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = texenv_requires_scaling(pname) ? fixed_to_float(param) : (float)param;

   texenvfv_internal(target, pname, params);
}

GL_API void GL_APIENTRY glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
   int i;
   float temp[4];

   for (i = 0; i < 4; i++)
      temp[i] = (float)params[i];

   texenvfv_internal(target, pname, temp);
}

GL_API void GL_APIENTRY glTexEnvxv(GLenum target, GLenum pname, const GLfixed *params)
{
   int i;
   float temp[4];

   for (i = 0; i < 4; i++)
      if (texenv_requires_scaling(pname))
         temp[i] = fixed_to_float(params[i]);
      else
         temp[i] = (float)params[i];

   texenvfv_internal(target, pname, temp);
}

GL_API void GL_APIENTRY glGetTexEnvxv(GLenum env, GLenum pname, GLfixed *params)
{
   float temp[4];
   int count = get_texenv_float_or_fixed_internal(env, pname, temp);

   assert(count <= 4);

   for (int i = 0; i < count; i++)
   {
      if (texenv_requires_scaling(pname))
         params[i] = float_to_fixed(temp[i]);
      else
         params[i] = (GLfixed)temp[i];
   }
}

static GLboolean is_color_size(GLint size)
{
   return size == 4;
}

static GLboolean is_color_type(GLenum type)
{
   return type == GL_UNSIGNED_BYTE ||
      type == GL_FIXED ||
      type == GL_FLOAT;
}

GL_API void GL_APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (!is_color_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (is_color_size(size) && glxx_is_aligned(type, (size_t)pointer) && glxx_is_aligned(type, (size_t)stride) && stride >= 0) {
      state->attrib[GL11_IX_COLOR].size = size;
      state->attrib[GL11_IX_COLOR].type = type;
      state->attrib[GL11_IX_COLOR].stride = stride;
      state->attrib[GL11_IX_COLOR].pointer = pointer;
      state->attrib[GL11_IX_COLOR].offset = (uintptr_t)pointer;
      state->attrib[GL11_IX_COLOR].buffer = state->bound_buffer.array_name;

      KHRN_MEM_ASSIGN(state->attrib[GL11_IX_COLOR].attrib, state->bound_buffer.array_buffer);
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   glxx_unlock_server_state(OPENGL_ES_11);
}

static GLboolean is_normal_type(GLenum type)
{
   return type == GL_BYTE ||
      type == GL_SHORT ||
      type == GL_FIXED ||
      type == GL_FLOAT;
}

GL_API void GL_APIENTRY glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (!is_normal_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (glxx_is_aligned(type, (size_t)pointer) && glxx_is_aligned(type, (size_t)stride) && stride >= 0) {

      state->attrib[GL11_IX_NORMAL].type = type;
      state->attrib[GL11_IX_NORMAL].stride = stride;
      state->attrib[GL11_IX_NORMAL].pointer = pointer;
      state->attrib[GL11_IX_NORMAL].offset = (uintptr_t)pointer;
      state->attrib[GL11_IX_NORMAL].buffer = state->bound_buffer.array_name;

      KHRN_MEM_ASSIGN(state->attrib[GL11_IX_NORMAL].attrib, state->bound_buffer.array_buffer);
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   glxx_unlock_server_state(OPENGL_ES_11);
}

static GLboolean is_vertex_size(GLint size)
{
   return size == 2 ||
      size == 3 ||
      size == 4;
}

static GLboolean is_vertex_type(GLenum type)
{
   return type == GL_BYTE ||
      type == GL_SHORT ||
      type == GL_FIXED ||
      type == GL_FLOAT;
}

GL_API void GL_APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (!is_vertex_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (is_vertex_size(size) && glxx_is_aligned(type, (size_t)pointer) && glxx_is_aligned(type, (size_t)stride) && stride >= 0) {

      state->attrib[GL11_IX_VERTEX].size = size;
      state->attrib[GL11_IX_VERTEX].type = type;
      state->attrib[GL11_IX_VERTEX].stride = stride;
      state->attrib[GL11_IX_VERTEX].pointer = pointer;
      state->attrib[GL11_IX_VERTEX].offset = (uintptr_t)pointer;
      state->attrib[GL11_IX_VERTEX].buffer = state->bound_buffer.array_name;

      KHRN_MEM_ASSIGN(state->attrib[GL11_IX_VERTEX].attrib, state->bound_buffer.array_buffer);
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   glxx_unlock_server_state(OPENGL_ES_11);
}

static GLboolean is_texture_coord_size(GLint size)
{
   return size == 2 ||
      size == 3 ||
      size == 4;
}

static GLboolean is_texture_coord_type(GLenum type)
{
   return type == GL_BYTE ||
      type == GL_SHORT ||
      type == GL_FIXED ||
      type == GL_FLOAT;
}

GL_API void GL_APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (!is_texture_coord_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (is_texture_coord_size(size) && glxx_is_aligned(type, (size_t)pointer) && glxx_is_aligned(type, (size_t)stride) && stride >= 0) {

      int indx = GL11_IX_TEXTURE_COORD + state->client_active_texture - GL_TEXTURE0;

      state->attrib[indx].size = size;
      state->attrib[indx].type = type;
      state->attrib[indx].stride = stride;
      state->attrib[indx].pointer = pointer;
      state->attrib[indx].offset = (uintptr_t)pointer;
      state->attrib[indx].buffer = state->bound_buffer.array_name;

      KHRN_MEM_ASSIGN(state->attrib[indx].attrib, state->bound_buffer.array_buffer);
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   glxx_unlock_server_state(OPENGL_ES_11);
}

static GLboolean is_point_size_type(GLenum type)
{
   return type == GL_FIXED ||
      type == GL_FLOAT;
}

GL_API void GL_APIENTRY glPointSizePointerOES(GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (!is_point_size_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (glxx_is_aligned(type, (size_t)pointer) && glxx_is_aligned(type, (size_t)stride) && stride >= 0) {

      state->attrib[GL11_IX_POINT_SIZE].type = type;
      state->attrib[GL11_IX_POINT_SIZE].stride = stride;
      state->attrib[GL11_IX_POINT_SIZE].pointer = pointer;
      state->attrib[GL11_IX_POINT_SIZE].offset = (uintptr_t)pointer;
      state->attrib[GL11_IX_POINT_SIZE].buffer = state->bound_buffer.array_name;

      KHRN_MEM_ASSIGN(state->attrib[GL11_IX_POINT_SIZE].attrib, state->bound_buffer.array_buffer);
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   glxx_unlock_server_state(OPENGL_ES_11);
}

static void color4f(float red, float green, float blue, float alpha)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   state->attrib[GL11_IX_COLOR].value[0] = red;
   state->attrib[GL11_IX_COLOR].value[1] = green;
   state->attrib[GL11_IX_COLOR].value[2] = blue;
   state->attrib[GL11_IX_COLOR].value[3] = alpha;

   glxx_update_color_material(state);

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
   color4f(
      clampf(red, 0.0f, 1.0f),
      clampf(green, 0.0f, 1.0f),
      clampf(blue, 0.0f, 1.0f),
      clampf(alpha, 0.0f, 1.0f));
}

GL_API void GL_APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
   color4f(
      (float)red / 255.0f,
      (float)green / 255.0f,
      (float)blue / 255.0f,
      (float)alpha / 255.0f);
}

GL_API void GL_APIENTRY glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
   color4f(
      clampf(fixed_to_float(red), 0.0f, 1.0f),
      clampf(fixed_to_float(green), 0.0f, 1.0f),
      clampf(fixed_to_float(blue), 0.0f, 1.0f),
      clampf(fixed_to_float(alpha), 0.0f, 1.0f));
}


GL_API void GL_APIENTRY glClientActiveTexture(GLenum texture)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (texture >= GL_TEXTURE0 && texture < GL_TEXTURE0 + GL11_CONFIG_MAX_TEXTURE_UNITS)
      state->client_active_texture = texture;
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state(OPENGL_ES_11);
}

static void enable_client_state(GLenum array, bool enabled)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   switch (array) {
   case GL_VERTEX_ARRAY:
      state->attrib[GL11_IX_VERTEX].enabled = enabled;
      break;
   case GL_NORMAL_ARRAY:
      state->attrib[GL11_IX_NORMAL].enabled = enabled;
      break;
   case GL_COLOR_ARRAY:
      state->attrib[GL11_IX_COLOR].enabled = enabled;
      break;
   case GL_POINT_SIZE_ARRAY_OES:
      state->attrib[GL11_IX_POINT_SIZE].enabled = enabled;
      break;
   case GL_TEXTURE_COORD_ARRAY:
      state->attrib[state->client_active_texture - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].enabled = enabled;
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glEnableClientState(GLenum array)
{
   enable_client_state(array, true);
}

GL_API void GL_APIENTRY glDisableClientState(GLenum array)
{
   enable_client_state(array, false);
}

static void multitexcoord4f_internal(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   if (target < GL_TEXTURE0 || target >= GL_TEXTURE0 + GL11_CONFIG_MAX_TEXTURE_UNITS) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   state->attrib[target - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].value[0] = clean_float(s);
   state->attrib[target - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].value[1] = clean_float(t);
   state->attrib[target - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].value[2] = clean_float(r);
   state->attrib[target - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].value[3] = clean_float(q);

end:
   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
   multitexcoord4f_internal(target, s, t, r, q);
}

GL_API void GL_APIENTRY glMultiTexCoord4x(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
   multitexcoord4f_internal(target, fixed_to_float(s), fixed_to_float(t), fixed_to_float(r), fixed_to_float(q));
}

static void normal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state)
      return;

   state->attrib[GL11_IX_NORMAL].value[0] = clean_float(nx);
   state->attrib[GL11_IX_NORMAL].value[1] = clean_float(ny);
   state->attrib[GL11_IX_NORMAL].value[2] = clean_float(nz);

   glxx_unlock_server_state(OPENGL_ES_11);
}

GL_API void GL_APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
   normal3f(nx, ny, nz);
}

GL_API void GL_APIENTRY glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz)
{
   normal3f(fixed_to_float(nx), fixed_to_float(ny), fixed_to_float(nz));
}
