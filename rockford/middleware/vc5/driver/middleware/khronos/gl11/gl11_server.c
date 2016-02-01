/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of OpenGL ES 1.1 state machine.
=============================================================================*/

#include "interface/khronos/glxx/gl_public_api.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_math.h"
#include "middleware/khronos/glxx/glxx_shared.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_server_internal.h"
#include "middleware/khronos/gl11/gl11_shadercache.h"

#include "middleware/khronos/glxx/glxx_texture.h"
#include "middleware/khronos/glxx/glxx_buffer.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "middleware/khronos/glxx/glxx_translate.h"

#include <string.h>
#include <math.h>
#include <limits.h>

#define LOG2E  1.442695f

#include "middleware/khronos/gl11/gl11_server_cr.c"

static void load_identity(float *mat) {
   int j;
   for (j=0; j<16; j++) {
      mat[j] = (float)((j/4) == (j&3));
   }
}

static bool gl11_state_init(GL11_STATE_T *gl11_state)
{
   int i;

   gl11_state->material.ambient[0] = 0.2f;
   gl11_state->material.ambient[1] = 0.2f;
   gl11_state->material.ambient[2] = 0.2f;
   gl11_state->material.ambient[3] = 1.0f;

   gl11_state->material.diffuse[0] = 0.8f;
   gl11_state->material.diffuse[1] = 0.8f;
   gl11_state->material.diffuse[2] = 0.8f;
   gl11_state->material.diffuse[3] = 1.0f;

   gl11_state->material.specular[0] = 0.0f;
   gl11_state->material.specular[1] = 0.0f;
   gl11_state->material.specular[2] = 0.0f;
   gl11_state->material.specular[3] = 1.0f;

   gl11_state->material.emission[0] = 0.0f;
   gl11_state->material.emission[1] = 0.0f;
   gl11_state->material.emission[2] = 0.0f;
   gl11_state->material.emission[3] = 1.0f;

   gl11_state->material.shininess = 0.0f;

   gl11_state->lightmodel.ambient[0] = 0.2f;
   gl11_state->lightmodel.ambient[1] = 0.2f;
   gl11_state->lightmodel.ambient[2] = 0.2f;
   gl11_state->lightmodel.ambient[3] = 1.0f;

   /* Set up GL1.1 specific parts of the statebits (the rest are glxx) */
   gl11_state->statebits.fragment = GL11_UCLIP_A | GL11_FOG_EXP | GL11_AFUNC_ALWAYS;
   gl11_state->statebits.f_enable = ~(GL11_UCLIP_M | GL11_FOG_M | GL11_LOGIC_M |
                                      GL11_AFUNC_M | GL11_POINTSMOOTH | GL11_LINESMOOTH);

   gl11_state->statebits.vertex = GL11_LIGHT_ENABLES_M | GL11_NO_NORMALIZE | GL11_LIGHTING;
   gl11_state->statebits.v_enable = ~(GL11_MPAL_M | GL11_LIGHTS_M);
   gl11_state->statebits.v_enable2 = ~GL11_LIGHTING_M;

   for (i = 0; i < GL11_CONFIG_MAX_LIGHTS; i++) {
      GL11_LIGHT_T *light = &gl11_state->lights[i];

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

   for (i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
      GL11_TEXUNIT_T *texunit = &gl11_state->texunits[i];

      texunit->target_enabled_2D = false;
      texunit->target_enabled_EXTERNAL_OES = false;

      gl11_state->statebits.texture[i] = GL11_TEX_STRING(MOD,S,C,P,C,K,A,MOD,S,A,P,A,K,A);

      texunit->mode = GL_MODULATE;

      texunit->color[0] = 0.0f;
      texunit->color[1] = 0.0f;
      texunit->color[2] = 0.0f;
      texunit->color[3] = 0.0f;

      texunit->rgb_scale = 1.0f;
      texunit->alpha_scale = 1.0f;

      gl11_matrix_stack_init(&texunit->stack);
      load_identity(texunit->current_matrix);
   }

   gl11_state->client_active_texture = GL_TEXTURE0;

   gl11_state->fog.color[0] = 0.0f;
   gl11_state->fog.color[1] = 0.0f;
   gl11_state->fog.color[2] = 0.0f;
   gl11_state->fog.color[3] = 0.0f;

   gl11_state->fog.density = 1.0f;
   gl11_state->fog.start = 0.0f;
   gl11_state->fog.end = 1.0f;

   gl11_state->fog.scale = 1.0f;
   gl11_state->fog.coeff_exp = -LOG2E;
   gl11_state->fog.coeff_exp2 = -LOG2E;

   gl11_state->hints_program.fog = GL_DONT_CARE;

   for (i = 0; i < GL11_CONFIG_MAX_PLANES; i++) {
      gl11_state->planes[i][0] = 0.0f;
      gl11_state->planes[i][1] = 0.0f;
      gl11_state->planes[i][2] = 0.0f;
      gl11_state->planes[i][3] = 0.0f;
   }

   gl11_state->shade_model = GL_SMOOTH;

   gl11_state->matrix_mode = GL_MODELVIEW;

   gl11_state->point_sprite = false;

   gl11_matrix_stack_init(&gl11_state->modelview);
   gl11_matrix_stack_init(&gl11_state->projection);
   /* Set current modelview and projection to the identity matrix */
   load_identity(gl11_state->current_modelview);
   load_identity(gl11_state->current_projection);

   gl11_state->alpha_func.ref = 0.0f;

   gl11_state->hints.perspective_correction = GL_DONT_CARE;
   gl11_state->hints.point_smooth = GL_DONT_CARE;
   gl11_state->hints.line_smooth = GL_DONT_CARE;

   gl11_state->point_params.size_min = 1.0f;
   gl11_state->point_params.size_min_clamped = 1.0f;
   gl11_state->point_params.size_max = 256.0f;
   gl11_state->point_params.fade_threshold = 1.0f;
   gl11_state->point_params.distance_attenuation[0] = 1.0f;
   gl11_state->point_params.distance_attenuation[1] = 0.0f;
   gl11_state->point_params.distance_attenuation[2] = 0.0f;

   gl11_state->changed.projection_modelview = true;
   gl11_state->changed.modelview_inv = true;

   // Allocate the shader cache
   gl11_state->shader_cache = calloc(GL11_CACHE_SIZE, sizeof(GL11_CACHE_ENTRY_T));
   if (gl11_state->shader_cache == NULL)
      return false;

   return true;
}

bool gl11_server_state_init(GLXX_SERVER_STATE_T *state, GLXX_SHARED_T *shared)
{
   GLXX_VAO_T *vao;

   //initialise common portions of state
   if(!glxx_server_state_init(state, shared))
      return false;

   if (!gl11_state_init(&state->gl11)) return false;

   /* Override some parts of the default attrib config with 1.1 values */
   state->generic_attrib[GL11_IX_COLOR].value[0] = 1.0f;
   state->generic_attrib[GL11_IX_COLOR].value[1] = 1.0f;
   state->generic_attrib[GL11_IX_COLOR].value[2] = 1.0f;
   state->generic_attrib[GL11_IX_NORMAL].value[2] = 1.0f;
   state->generic_attrib[GL11_IX_POINT_SIZE].value[0] = 1.0f;

   vao = state->vao.bound;
   vao->attrib_config[GL11_IX_NORMAL].size = 3;
   vao->attrib_config[GL11_IX_POINT_SIZE].size = 1;

   return true;
}

void gl11_server_state_destroy(GLXX_SERVER_STATE_T *state)
{
   if (state != NULL)
   {
      gl11_hw_shader_cache_reset(state->gl11.shader_cache);

      free(state->gl11.shader_cache);
      state->gl11.shader_cache = NULL;
   }
}

/*
void glPointSize_impl_11 (GLfloat size)
{
}
*/


////

GL_APICALL void GL_APIENTRY glGetTexParameterxv (GLenum target, GLenum pname, GLfixed *params)
{
   GLint temp[4];
   GLuint count = 0;
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();

   if (!state) return;

   if(glxx_is_int_texparam(state, target, pname))
      count = glxx_get_texparameter_internal(state, target, pname, temp);

   if (count) {
      unsigned int i;
      assert(count == 1 || count == 4);
      for(i=0;i<count;i++)
         params[i] = (GLfixed)temp[i];
   }
   GL11_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glTexParameterx (GLenum target, GLenum pname, GLfixed param)
{
   GLint iparams[4];
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   iparams[0] = (GLint)param;                /* no scaling for enum to fixed */

   if (pname == GL_TEXTURE_CROP_RECT_OES) {
      iparams[1] = iparams[2] = iparams[3] = 0;
   }

   glxx_texparameter_internal(state, target, pname, iparams);

   GL11_UNLOCK_SERVER_STATE();
}


GL_APICALL void GL_APIENTRY glTexParameterxv (GLenum target, GLenum pname, const GLfixed *params)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   if (params)
   {
      GLint iparams[4];
      iparams[0] = (GLint)params[0];         /* no scaling for enum to fixed */

      if(pname == GL_TEXTURE_CROP_RECT_OES) {
         int i;
         for(i=1;i<4;i++)                    /* fill in the remaining 3 */
            iparams[i] = (GLint)params[i];   /* no scaling for enum to fixed */
      }

      glxx_texparameter_internal(state, target, pname, iparams);
   }

   GL11_UNLOCK_SERVER_STATE();
}





/*
void glMultiTexCoord4f (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
}

void glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz)
{
}
*/
static void point_parameterv_internal(GLenum pname, const GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   switch (pname) {
   case GL_POINT_SIZE_MIN:
   {
      GLfloat m = clean_float(params[0]);

      if (m >= 0)
      {
         state->gl11.point_params.size_min = m;
         state->gl11.point_params.size_min_clamped = MAXF(1.0f, state->gl11.point_params.size_min);
      }
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);

      break;
   }
   case GL_POINT_SIZE_MAX:
   {
      GLfloat m = clean_float(params[0]);

      if (m >= 0)
         state->gl11.point_params.size_max = m;
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);

      break;
   }
   case GL_POINT_FADE_THRESHOLD_SIZE:
   {
      GLfloat m = clean_float(params[0]);

      if (m >= 0)
         state->gl11.point_params.fade_threshold = m;
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);

      break;
   }
   case GL_POINT_DISTANCE_ATTENUATION:
   {
      int i;
      for (i = 0; i < 3; i++)
         state->gl11.point_params.distance_attenuation[i] = clean_float(params[i]);

      break;
   }
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

   GL11_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glPointParameterf (GLenum pname, GLfloat param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f};

   params[0] = param;

   point_parameterv_internal(pname, params);
}

GL_APICALL void GL_APIENTRY glPointParameterfv (GLenum pname, const GLfloat *params)
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

static GLboolean is_scalef(GLfloat scale)
{
   return scale == 1.0f ||
          scale == 2.0f ||
          scale == 4.0f;
}

static void texenvfv_internal(GLenum target, GLenum pname, const GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   int t;
   if (!state) return;

   t = state->active_texture - GL_TEXTURE0;

   switch (target) {
   case GL_POINT_SPRITE_OES:
      switch (pname) {
      case GL_COORD_REPLACE_OES:
         SET_INDIVIDUAL(state->gl11.statebits.texture[t], GL11_TEX_COORDREPLACE, params[0] != 0.0f);
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
            state->gl11.texunits[t].mode = mode;
         }
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);

         break;
      }
      case GL_TEXTURE_ENV_COLOR:
         {
            int i;
            for (i = 0; i < 4; i++)
               state->gl11.texunits[t].color[i] = clampf(params[i], 0.0f, 1.0f);
         }
         break;
      case GL_COMBINE_RGB:
      {
         uint32_t combine = translate_combine_rgb((GLenum)params[0]);

         if (combine != ~0u)
            SET_MASKED(state->gl11.statebits.texture[t], combine<<GL11_TEX_CC_S, GL11_TEX_CC_M);
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);

         break;
      }
      case GL_RGB_SCALE:
      {
         if (is_scalef(params[0]))
         {
            SET_INDIVIDUAL(state->gl11.statebits.texture[t], GL11_TEX_CSCALE, params[0] != 1.0f);
            state->gl11.texunits[t].rgb_scale = params[0];
         }
         else
            glxx_server_state_set_error(state, GL_INVALID_VALUE);

         break;
      }
      case GL_SRC0_RGB:
      case GL_SRC1_RGB:
      case GL_SRC2_RGB:
      {
         uint32_t sh = GL11_TEX_CS_S + 2 * (pname - GL_SRC0_RGB);
         uint32_t source = translate_tex_source((GLenum)params[0]);

         if (source != ~0u)
            SET_MASKED(state->gl11.statebits.texture[t], source << sh, GL11_SRC_M << sh);
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);

         break;
      }
      case GL_OPERAND0_RGB:
      case GL_OPERAND1_RGB:
      case GL_OPERAND2_RGB:
      {
         uint32_t sh = GL11_TEX_CO_S + 2 * (pname - GL_OPERAND0_RGB);
         uint32_t operand = translate_tex_operand_rgb((GLenum)params[0]);

         if (operand != ~0u)
            SET_MASKED(state->gl11.statebits.texture[t], operand << sh, GL11_OPC_M << sh);
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);

         break;
      }
      case GL_COMBINE_ALPHA:
      {
         uint32_t combine = translate_combine_alpha((GLenum)params[0]);

         if (combine != ~0u)
            SET_MASKED(state->gl11.statebits.texture[t], combine<<GL11_TEX_AC_S, GL11_TEX_AC_M);
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);

         break;
      }
      case GL_ALPHA_SCALE:
      {
         if (is_scalef(params[0]))
         {
            SET_INDIVIDUAL(state->gl11.statebits.texture[t], GL11_TEX_ASCALE, params[0] != 1.0f);
            state->gl11.texunits[t].alpha_scale = params[0];
         }
         else
            glxx_server_state_set_error(state, GL_INVALID_VALUE);

         break;
      }
      case GL_SRC0_ALPHA:
      case GL_SRC1_ALPHA:
      case GL_SRC2_ALPHA:
      {
         uint32_t sh = GL11_TEX_AS_S + 2 * (pname - GL_SRC0_ALPHA);
         uint32_t source = translate_tex_source((GLenum)params[0]);

         if (source != ~0u)
            SET_MASKED(state->gl11.statebits.texture[t], source << sh, GL11_SRC_M << sh);
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);

         break;
      }
      case GL_OPERAND0_ALPHA:
      case GL_OPERAND1_ALPHA:
      case GL_OPERAND2_ALPHA:
      {
         uint32_t sh = GL11_TEX_AO_S + (pname - GL_OPERAND0_ALPHA);
         uint32_t operand = translate_tex_operand_alpha((GLenum)params[0]);

         if (operand != ~0u)
            SET_MASKED(state->gl11.statebits.texture[t], operand << sh, GL11_OPA_M << sh);
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

   GL11_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glTexEnvf (GLenum target, GLenum pname, GLfloat param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = param;

   texenvfv_internal(target, pname, params);
}

GL_APICALL void GL_APIENTRY glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params)
{
   texenvfv_internal(target, pname, params);
}






/*
void glDisableClientState (GLenum array)
{
}
*/



/*
void glEnableClientState (GLenum array)
{
}
*/

/*
   glLightf (GLenum light, GLenum pname, GLfloat param)
   glLightfv (GLenum light, GLenum pname, const GLfloat *params)
   glLightx (GLenum light, GLenum pname, GLfixed param)
   glLightxv (GLenum light, GLenum pname, const GLfixed *params)

   Khronos documentation:

   Lighting parameters are divided into three categories: material parameters, light
   source parameters, and lighting model parameters (see Table 2.8). Sets of lighting
   parameters are specified with
            :  :  :  :
      void Light{xf}( enum light, enum pname, T param );
      void Light{xf}v( enum light, enum pname, T params );

   In the vector versions of the commands, params is a pointer to a group
   of values to which to set the indicated parameter. The number of values pointed to
   depends on the parameter being set. In the non-vector versions, param is a value
   to which to set a single-valued parameter. (If param corresponds to a multi-valued
   parameter, the error INVALID ENUM results.) In the case of Light, light is a symbolic
   constant of the form LIGHTi, indicating that light i is to have the specified parameter
   set. The constants obey LIGHTi = LIGHT0 + i.

   The current model-view matrix is applied to the position parameter indicated
   with Light for a particular light source when that position is specified. These
   transformed values are the values used in the lighting equation.

   The spotlight direction is transformed when it is specified using only the upper
   leftmost 3x3 portion of the model-view matrix

   Lighting Parameters

      AMBIENT 4
      DIFFUSE 4
      SPECULAR 4
      POSITION 4
      SPOT DIRECTION 3
      SPOT EXPONENT 1
      SPOT CUTOFF 1
      CONSTANT ATTENUATION 1
      LINEAR ATTENUATION 1
      QUADRATIC ATTENUATION 1

   Postconditions:

   If is a vector-valued light parameter and a scalar function has been used or
   light is invalid, set an GL_INVALID_ENUM error
   TODO: This differs from our behaviour in other places, as documented in server_cr
*/

static void lightv_internal(GLenum l, GLenum pname, const GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   if (l >= GL_LIGHT0 && l < GL_LIGHT0 + GL11_CONFIG_MAX_LIGHTS)
   {
      uint32_t index = l - GL_LIGHT0;
      GL11_LIGHT_T *light = &state->gl11.lights[index];
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

         gl11_matrix_mult_col(light->position, state->gl11.current_modelview, clean);

         /*
            compute 3-vector position of the light for use by the HAL
         */

         SET_INDIVIDUAL(state->gl11.statebits.vertex, GL11_LIGHT_LOCAL << index, light->position[3] != 0.0f);
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

         gl11_matrix_mult_col(light->spot.direction, state->gl11.current_modelview, clean);
         break;
      }
      case GL_SPOT_EXPONENT:
         light->spot.exponent = clean_float(params[0]);
         break;
      case GL_SPOT_CUTOFF:
         light->spot.cutoff = clean_float(params[0]);
         SET_INDIVIDUAL(state->gl11.statebits.vertex, GL11_LIGHT_SPOT << index, light->spot.cutoff != 180.0f);

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
   }
   else
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

   GL11_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glLightf (GLenum light, GLenum pname, GLfloat param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = param;

   lightv_internal(light, pname, params);
}

GL_APICALL void GL_APIENTRY glLightfv (GLenum light, GLenum pname, const GLfloat *params)
{
   lightv_internal(light, pname, params);
}

GL_APICALL void GL_APIENTRY glLightx (GLenum light, GLenum pname, GLfixed param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = fixed_to_float(param);

   lightv_internal(light, pname, params);
}

GL_APICALL void GL_APIENTRY glLightxv (GLenum light, GLenum pname, const GLfixed *params)
{
   int i;
   GLfloat temp[4];

   for (i = 0; i < 4; i++)
      temp[i] = fixed_to_float(params[i]);

   lightv_internal(light, pname, temp);
}


GL_APICALL void GL_APIENTRY glLogicOp (GLenum opcode)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   uint32_t op;
   if (!state) return;

   op = translate_logic_op(opcode);
   if (op != ~0u)
   {
      SET_MASKED(state->gl11.statebits.fragment, op, GL11_LOGIC_M);
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   GL11_UNLOCK_SERVER_STATE();
}

//cr[ 2011-06-16 15:00

/*
   glMatrixMode

   Set the current matrix mode. See GL11 spec section X.X
*/
static bool is_matrix_mode(GLenum mode)
{
   return mode == GL_TEXTURE ||
          mode == GL_MODELVIEW ||
          mode == GL_PROJECTION ||
#if GL_OES_matrix_palette
          mode == GL_MATRIX_PALETTE_OES ||
#endif
          0;
}

GL_APICALL void GL_APIENTRY glMatrixMode (GLenum mode)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE_UNCHANGED();
   if (!state) return;

   if (is_matrix_mode(mode))
      state->gl11.matrix_mode = mode;
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   GL11_UNLOCK_SERVER_STATE();
}

//cr]

/*
   void glMaterialf (GLenum face, GLenum pname, GLfloat param)
   void glMaterialfv (GLenum face, GLenum pname, const GLfloat *params)
   void glMaterialx (GLenum face, GLenum pname, GLfixed param)
   void glMaterialxv (GLenum face, GLenum pname, const GLfixed *params)

   Khronos documentation:

   Lighting parameters are divided into three categories: material parameters, light
   source parameters, and lighting model parameters (see Table 2.8). Sets of lighting
   parameters are specified with
            :  :  :  :
      void Material{xf}( enum face, enum pname, T param );
      void Material{xf}v( enum face, enum pname, T params );

   Lighting Model Parameters

      AMBIENT 4
      DIFFUSE 4
      AMBIENT AND DIFFUSE 4
      SPECULAR 4
      EMISSION 4
      SHININESS 1

   Implementation notes:

   The range of light, lightmodel and material color parameters is -INF to INF (i.e. unclamped)
*/

static void materialv_internal (GLenum face, GLenum pname, const GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   if (face == GL_FRONT_AND_BACK) {
      switch (pname) {
      case GL_AMBIENT:
         if (!(state->gl11.statebits.vertex & GL11_COLORMAT)) {
            int i;
            for (i = 0; i < 4; i++)
               state->gl11.material.ambient[i] = clean_float(params[i]);
         }
         break;
      case GL_DIFFUSE:
      {
         if (!(state->gl11.statebits.vertex & GL11_COLORMAT)) {
            int i;
            for (i = 0; i < 4; i++)
               state->gl11.material.diffuse[i] = clean_float(params[i]);
         }
         break;
      }
      case GL_AMBIENT_AND_DIFFUSE:
      {
         if (!(state->gl11.statebits.vertex & GL11_COLORMAT)) {
            int i;
            for (i = 0; i < 4; i++) {
               GLfloat f = clean_float(params[i]);

               state->gl11.material.ambient[i] = f;
               state->gl11.material.diffuse[i] = f;
            }
         }
         break;
      }
      case GL_SPECULAR:
      {
         int i;
         for (i = 0; i < 4; i++)
            state->gl11.material.specular[i] = clean_float(params[i]);
         break;
      }
      case GL_EMISSION:
      {
         int i;
         for (i = 0; i < 4; i++)
            state->gl11.material.emission[i] = clean_float(params[i]);
         break;
      }
      case GL_SHININESS:
         state->gl11.material.shininess = clean_float(params[0]);
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   GL11_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glMaterialf (GLenum face, GLenum pname, GLfloat param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = param;

   materialv_internal(face, pname, params);
}

GL_APICALL void GL_APIENTRY glMaterialfv (GLenum face, GLenum pname, const GLfloat *params)
{
   materialv_internal(face, pname, params);
}

GL_APICALL void GL_APIENTRY glMaterialx (GLenum face, GLenum pname, GLfixed param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = fixed_to_float(param);

   materialv_internal(face, pname, params);
}

GL_APICALL void GL_APIENTRY glMaterialxv (GLenum face, GLenum pname, const GLfixed *params)
{
   int i;
   GLfloat temp[4];

   for (i = 0; i < 4; i++)
      temp[i] = fixed_to_float(params[i]);

   materialv_internal(face, pname, temp);
}

/*
void glMultiTexCoord4x (GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
}

void glNormal3x (GLfixed nx, GLfixed ny, GLfixed nz)
{
}
*/

GL_APICALL void GL_APIENTRY glPointParameterx (GLenum pname, GLfixed param)
{
   GLfloat params[] = {0.0f, 0.0f, 0.0f};

   params[0] = fixed_to_float(param);

   point_parameterv_internal(pname, params);
}

GL_APICALL void GL_APIENTRY glPointParameterxv (GLenum pname, const GLfixed *params)
{
   int i;
   GLfloat temp[3];

   for (i = 0; i < 3; i++)
      temp[i] = fixed_to_float(params[i]);

   point_parameterv_internal(pname, temp);
}
/*
void glPointSizex (GLfixed size)
{
}
*/




static GLboolean is_shade_model(GLenum model)
{
   return model == GL_SMOOTH ||
          model == GL_FLAT;
}

GL_APICALL void GL_APIENTRY glShadeModel (GLenum model)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   if (is_shade_model(model))
      state->gl11.shade_model = model;
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   GL11_UNLOCK_SERVER_STATE();
}

static GLboolean texenv_requires_scaling(GLenum pname)
{
   return pname == GL_TEXTURE_ENV_COLOR ||
          pname == GL_RGB_SCALE ||
          pname == GL_ALPHA_SCALE;
}

GL_APICALL void GL_APIENTRY glTexEnvi (GLenum target, GLenum pname, GLint param)
{
   float params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = (float)param;

   texenvfv_internal(target, pname, params);
}

GL_APICALL void GL_APIENTRY glTexEnvx (GLenum target, GLenum pname, GLfixed param)
{
   float params[] = {0.0f, 0.0f, 0.0f, 0.0f};

   params[0] = texenv_requires_scaling(pname) ? fixed_to_float(param) : (float)param;

   texenvfv_internal(target, pname, params);
}

/*
TODO: slightly annoying that we convert enum parameters to float and back.
But at least it avoids duplicated code.
*/
GL_APICALL void GL_APIENTRY glTexEnviv (GLenum target, GLenum pname, const GLint *params)
{
   int i;
   float temp[4];

   for (i = 0; i < 4; i++)
      temp[i] = (float)params[i];

   texenvfv_internal(target, pname, temp);
}

GL_APICALL void GL_APIENTRY glTexEnvxv (GLenum target, GLenum pname, const GLfixed *params)
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

GL_APICALL void GL_APIENTRY glGetTexEnvxv (GLenum env, GLenum pname, GLfixed *params)
{
   int i;
   float temp[4];

   int count = get_texenv_float_or_fixed_internal(env, pname, temp);

   assert(count <= 4);

   for (i = 0; i < count; i++)
      if (texenv_requires_scaling(pname))
         params[i] = float_to_fixed(temp[i]);
      else
         params[i] = (GLfixed)temp[i];
}

//cr[ 2011-06-16 15:00

/*
   get_matrix(GLXX_SERVER_STATE_T *state)

   Return the matrix on the top of the currently active stack
   as an array of 16 floats in column-major order.

   Implementation notes:

   The top of each stack is stored separately, so return that.
*/

static float *get_matrix(GLXX_SERVER_STATE_T *state)
{
   switch (state->gl11.matrix_mode) {
   case GL_MODELVIEW:
      state->gl11.changed.projection_modelview = true;
      state->gl11.changed.modelview_inv = true;
      return state->gl11.current_modelview;
   case GL_PROJECTION:
      state->gl11.changed.projection_modelview = true;
      return state->gl11.current_projection;
   case GL_TEXTURE:
      return state->gl11.texunits[state->active_texture - GL_TEXTURE0].current_matrix;
#if GL_OES_matrix_palette
   case GL_MATRIX_PALETTE_OES:
      return state->gl11.palette_matrices[state->gl11.current_palette_matrix];
#endif
   default:
      UNREACHABLE();
      return NULL;
   }
}

/*
   Copies the argument into the matrix at the top of the currently selected stack

   Preconditions:

   m is not overlapping the GL state
*/

static void load_matrix_internal(const float m[16])
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   float *c;
   if (!state) return;

   c = get_matrix(state);

   gl11_matrix_load(c, m);

   GL11_UNLOCK_SERVER_STATE();
}

/*
   Multiply the matrix at the top of the active stack on the right by the
   supplied matrix (supplied as an array of 16 floats in column-major order)
*/
static void mult_matrix_internal(const float m[16])
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   float *c;
   if (!state) return;

   c = get_matrix(state);

   gl11_matrix_mult(c, c, m);

   GL11_UNLOCK_SERVER_STATE();
}

/*
   Implementation notes:

   Non-overlapping precondition on load_matrix_internal() is satisfied as the
   stack never overlaps the GL state.
*/
GL_APICALL void GL_APIENTRY glLoadIdentity (void)
{
   float m[16];

   m[0]  = 1.0f;
   m[1]  = 0.0f;
   m[2]  = 0.0f;
   m[3]  = 0.0f;

   m[4]  = 0.0f;
   m[5]  = 1.0f;
   m[6]  = 0.0f;
   m[7]  = 0.0f;

   m[8]  = 0.0f;
   m[9]  = 0.0f;
   m[10] = 1.0f;
   m[11] = 0.0f;

   m[12] = 0.0f;
   m[13] = 0.0f;
   m[14] = 0.0f;
   m[15] = 1.0f;

   load_matrix_internal(m);
}

//cr]
//cr[ 2011-06-30 15:00
/*
   void glLoadMatrixf (const GLfloat *m)
   void glLoadMatrixx (const GLfixed *m)

   Implementation notes:

   Non-overlapping precondition on load_matrix_internal is satisfied as m must
   never point to part of the GL state and the stack never overlaps the GL state
   either.

   Preconditions:

   m is a valid pointer to 16 elements not overlapping the GL state

   Postconditions:

   Invariants preserved:
*/

GL_APICALL void GL_APIENTRY glLoadMatrixf (const GLfloat *m)
{
   load_matrix_internal(m);
}

GL_APICALL void GL_APIENTRY glLoadMatrixx (const GLfixed *m)
{
   int32_t i;
   float f[16];

   for (i = 0; i < 16; i++)
      f[i] = fixed_to_float(m[i]);

   load_matrix_internal(f);
}

/*
   void glMultMatrixf (const GLfloat *m)
   void glMultMatrixx (const GLfixed *m)

   Preconditions:

   m is a valid pointer to 16 elements

   Postconditions:

   -

*/

GL_APICALL void GL_APIENTRY glMultMatrixf (const GLfloat *m)
{
   mult_matrix_internal(m);
}

GL_APICALL void GL_APIENTRY glMultMatrixx (const GLfixed *m)
{
   int32_t i;
   float f[16];

   for (i = 0; i < 16; i++)
      f[i] = fixed_to_float(m[i]);

   mult_matrix_internal(f);
}

/*
   void glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
   void glRotatex (GLfixed angle, GLfixed x, GLfixed y, GLfixed z)

   Implementation notes:

   Division by zero may occur for very short axis vectors.

   Preconditions:

   -

   Postconditions:

   -

*/

static void rotate_internal(float angle, float x, float y, float z)
{
   float s, c;
   float m[16];

   /*
      normalize axis vector
   */
   if(x != 0.0f || y != 0.0f || z != 0.0f)
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

   m[0]  = x * x * (1 - c) + c;
   m[1]  = y * x * (1 - c) + z * s;
   m[2]  = z * x * (1 - c) - y * s;
   m[3]  = 0.0f;

   m[4]  = x * y * (1 - c) - z * s;
   m[5]  = y * y * (1 - c) + c;
   m[6]  = z * y * (1 - c) + x * s;
   m[7]  = 0.0f;

   m[8]  = x * z * (1 - c) + y * s;
   m[9]  = y * z * (1 - c) - x * s;
   m[10] = z * z * (1 - c) + c;
   m[11] = 0.0f;

   m[12] = 0.0f;
   m[13] = 0.0f;
   m[14] = 0.0f;
   m[15] = 1.0f;

   mult_matrix_internal(m);
}

GL_APICALL void GL_APIENTRY glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
   rotate_internal(angle, x, y, z);
}

GL_APICALL void GL_APIENTRY glRotatex (GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
   rotate_internal(fixed_to_float(angle),
                   fixed_to_float(x),
                   fixed_to_float(y),
                   fixed_to_float(z));
}

/*
   void glScalef (GLfloat x, GLfloat y, GLfloat z)
   void glScalex (GLfixed x, GLfixed y, GLfixed z)

   Preconditions:

   -

   Postconditions:

   -

*/

static void scale_internal(float x, float y, float z)
{
   float m[16];

   m[0]  = x;
   m[1]  = 0.0f;
   m[2]  = 0.0f;
   m[3]  = 0.0f;

   m[4]  = 0.0f;
   m[5]  = y;
   m[6]  = 0.0f;
   m[7]  = 0.0f;

   m[8]  = 0.0f;
   m[9]  = 0.0f;
   m[10] = z;
   m[11] = 0.0f;

   m[12] = 0.0f;
   m[13] = 0.0f;
   m[14] = 0.0f;
   m[15] = 1.0f;

   mult_matrix_internal(m);
}

GL_APICALL void GL_APIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z)
{
   scale_internal(x, y, z);
}

GL_APICALL void GL_APIENTRY glScalex (GLfixed x, GLfixed y, GLfixed z)
{
   scale_internal(fixed_to_float(x),
                  fixed_to_float(y),
                  fixed_to_float(z));
}


/*
   void glTranslatef (GLfloat x, GLfloat y, GLfloat z)
   void glTranslatex (GLfixed x, GLfixed y, GLfixed z)

   Preconditions:

   -

   Postconditions:

   -
*/

static void translate_internal(float x, float y, float z)
{
   float m[16];

   m[0]  = 1.0f;
   m[1]  = 0.0f;
   m[2]  = 0.0f;
   m[3]  = 0.0f;

   m[4]  = 0.0f;
   m[5]  = 1.0f;
   m[6]  = 0.0f;
   m[7]  = 0.0f;

   m[8]  = 0.0f;
   m[9]  = 0.0f;
   m[10] = 1.0f;
   m[11] = 0.0f;

   m[12] = x;
   m[13] = y;
   m[14] = z;
   m[15] = 1.0f;

   mult_matrix_internal(m);
}

GL_APICALL void GL_APIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z)
{
   translate_internal(x, y, z);
}

GL_APICALL void GL_APIENTRY glTranslatex (GLfixed x, GLfixed y, GLfixed z)
{
   translate_internal(fixed_to_float(x),
                      fixed_to_float(y),
                      fixed_to_float(z));
}

/*
   glFrustumf (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
   glFrustumx (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)

   Khronos documentation:

   If either n or f is less than or equal to zero,
   l is equal to r, b is equal to t, or n is equal to f, the error INVALID VALUE results.

   Implementation Notes:

   The equality test is performed on the floating point values.

   Preconditions:

   -

   Postconditions:

   If constraints on l, r, b, t, n, f are not met and no current error, error becomes
   INVALID_VALUE

*/

static void frustum_internal(float l, float r, float b, float t, float n, float f)
{
   if (n > 0.0f && f > 0.0f && l != r && b != t && n != f) {
      float m[16];

      m[0]  = 2.0f * n / (r - l);
      m[1]  = 0.0f;
      m[2]  = 0.0f;
      m[3]  = 0.0f;

      m[4]  = 0.0f;
      m[5]  = 2.0f * n / (t - b);
      m[6]  = 0.0f;
      m[7]  = 0.0f;

      m[8]  = (r + l) / (r - l);
      m[9]  = (t + b) / (t - b);
      m[10] = -(f + n) / (f - n);
      m[11] = -1.0f;

      m[12] = 0.0f;
      m[13] = 0.0f;
      m[14] = -2.0f * f * n / (f - n);
      m[15] = 0.0f;

      mult_matrix_internal(m);
   } else {
      GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
      if (!state) return;

      glxx_server_state_set_error(state, GL_INVALID_VALUE);

      GL11_UNLOCK_SERVER_STATE();
   }
}

GL_APICALL void GL_APIENTRY glFrustumf (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
   frustum_internal(left,
                    right,
                    bottom,
                    top,
                    zNear,
                    zFar);
}

GL_APICALL void GL_APIENTRY glFrustumx (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
   frustum_internal(fixed_to_float(left),
                    fixed_to_float(right),
                    fixed_to_float(bottom),
                    fixed_to_float(top),
                    fixed_to_float(zNear),
                    fixed_to_float(zFar));
}

/*
   void glOrthof (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
   void glOrthox (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)

   If l is equal to r, b is equal to t, or n is equal to f, the error INVALID VALUE results.

   We perform the equality test on the floating point versions of the parameters, even for Orthox.
*/

static void ortho_internal(float l, float r, float b, float t, float n, float f)
{
   if (l != r && b != t && n != f) {
      float m[16];

      m[0]  = 2.0f / (r - l);
      m[1]  = 0.0f;
      m[2]  = 0.0f;
      m[3]  = 0.0f;

      m[4]  = 0.0f;
      m[5]  = 2.0f / (t - b);
      m[6]  = 0.0f;
      m[7]  = 0.0f;

      m[8]  = 0.0f;
      m[9]  = 0.0f;
      m[10] = -2.0f / (f - n);
      m[11] = 0.0f;

      m[12] = -(r + l) / (r - l);
      m[13] = -(t + b) / (t - b);
      m[14] = -(f + n) / (f - n);
      m[15] = 1.0f;

      mult_matrix_internal(m);
   } else {
      GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
      if (!state) return;

      glxx_server_state_set_error(state, GL_INVALID_VALUE);

      GL11_UNLOCK_SERVER_STATE();
   }
}

GL_APICALL void GL_APIENTRY glOrthof (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
   ortho_internal(left, right, bottom, top, zNear, zFar);
}

GL_APICALL void GL_APIENTRY glOrthox (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
   ortho_internal(fixed_to_float(left),
                  fixed_to_float(right),
                  fixed_to_float(bottom),
                  fixed_to_float(top),
                  fixed_to_float(zNear),
                  fixed_to_float(zFar));
}



GL_APICALL void GL_APIENTRY glPopMatrix (void)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   GL11_MATRIX_STACK_T *stack;
   float *cur_matrix;
   if (!state) return;

#if GL_OES_matrix_palette
   if (state->gl11.matrix_mode == GL_MATRIX_PALETTE_OES) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto fail;
   }
#endif

   stack = get_stack(state);
   cur_matrix = get_matrix(state);

   if (stack->pos > 0) {
      stack->pos--;
      gl11_matrix_load(cur_matrix, stack->body[stack->pos]);
   } else
      glxx_server_state_set_error(state, GL_STACK_UNDERFLOW);

fail:
   GL11_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glPushMatrix (void)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   GL11_MATRIX_STACK_T *stack;
   float *cur_matrix;
   if (!state) return;

#if GL_OES_matrix_palette
   if (state->gl11.matrix_mode == GL_MATRIX_PALETTE_OES) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto fail;
   }
#endif

   stack = get_stack(state);
   cur_matrix = get_matrix(state);

   if (stack->pos + 1 < GL11_CONFIG_MAX_STACK_DEPTH) {
      gl11_matrix_load(stack->body[stack->pos], cur_matrix);
      stack->pos++;
   } else
      glxx_server_state_set_error(state, GL_STACK_OVERFLOW);

fail:
   GL11_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glClientActiveTexture (GLenum texture)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE_UNCHANGED();
   if (!state) return;

   if (texture >= GL_TEXTURE0 && texture < GL_TEXTURE0 + GL11_CONFIG_MAX_TEXTURE_UNITS)
      state->gl11.client_active_texture = texture;
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   GL11_UNLOCK_SERVER_STATE();
}


/* Return the specified clip plane from out of the server state.  */
static float *get_plane(GLXX_SERVER_STATE_T *state, GLenum p)
{
   if (p >= GL_CLIP_PLANE0 && p < GL_CLIP_PLANE0 + GL11_CONFIG_MAX_PLANES)
      return state->gl11.planes[p - GL_CLIP_PLANE0];
   else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return NULL;
   }
}

static void clip_plane_internal(GLenum p, const float equation[4])
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   float *plane;
   if (!state) return;

   plane = get_plane(state, p);

   if (plane) {
      float inv[16];

      /* Choose which test to use so that complementary planes don't produce gaps or overlap */
      if (
            equation[0] > 0 ||
            (equation[0] == 0 && (equation[1] > 0 ||
            (equation[1] == 0 && (equation[2] > 0 ||
            (equation[2] == 0 && (equation[3] >= 0)))) )))
      {
         SET_MASKED(state->gl11.statebits.fragment, GL11_UCLIP_A, GL11_UCLIP_M);
      }
      else SET_MASKED(state->gl11.statebits.fragment, GL11_UCLIP_B, GL11_UCLIP_M);

      gl11_matrix_invert_4x4(inv, state->gl11.current_modelview);
      gl11_matrix_mult_row(plane, equation, inv);
   }

   GL11_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glClipPlanef (GLenum plane, const GLfloat *equation)
{
   clip_plane_internal(plane, equation);
}

GL_APICALL void GL_APIENTRY glClipPlanex (GLenum plane, const GLfixed *equation)
{
   int32_t i;
   float temp[4];

   for (i = 0; i < 4; i++)
      temp[i] = fixed_to_float(equation[i]);

   clip_plane_internal(plane, temp);
}

//cr]

static void get_clip_plane_internal(GLenum pname, float eqn[4])
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE_UNCHANGED();
   float *plane;
   if (!state) return;

   plane = get_plane(state, pname);

   if (plane) {
      int i;
      for (i = 0; i < 4; i++)
         eqn[i] = plane[i];
   }

   GL11_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glGetClipPlanef (GLenum pname, GLfloat eqn[4])
{
   get_clip_plane_internal(pname, eqn);
}

GL_APICALL void GL_APIENTRY glGetClipPlanex (GLenum pname, GLfixed eqn[4])
{
   float temp[4];

   int i;

   get_clip_plane_internal(pname, temp);

   for (i = 0; i < 4; i++)
      eqn[i] = float_to_fixed(temp[i]);
}
