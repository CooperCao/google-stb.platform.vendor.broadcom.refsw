/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  gl11
Module   :

FILE DESCRIPTION
=============================================================================*/

/*
   FIXMEFIXMEFIXMEFIX! change glFog and glLightModel and GlLight and GLMaterial to reject attempts to set
   vector valued variables with scalar variants


Commentary:

Many functions come in pairs - a fixed point version and a floating point version.
For instance:
   glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
   glClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha)

For these, we define an "internal" version of the function, which always deals in
floating point. We convert from fixed point to floating point before calling this
function if necessary, using fixed_to_float (or float_to_fixed for output parameters).


We assume enums in gl.h to be contiguous in various instances, as follows:
    GL_LIGHTn, GL_CLIP_PLANEn, GL_TEXTUREn.

We assume that GLfloat and float are the same size and representation.  We assume
that GLint and int are the same size and representation.  We take advantage of these
assumptions by merrily casting from a pointer to one to a pointer to the other.  This
is _not_ true for GLboolean and bool.  We do, however, assume that false and true are
represented by 0 and 1 respectively for both GLboolean and bool. We assume that int
and float are the same size.

We assume something about the valid ranges of signed and unsigned integers:
   If
      a is an int
      b is an int
      a >= 0
      b >= 0
   Then (uint32_t)a + (uint32_t)b does not overflow.

   SHADE_MODEL                            is incorrectly flagged as Z+ not Z2 in state tables
   IMPLEMENTATION_COLOR_READ_TYPE_OES     is not listed in the state tables
   IMPLEMENTATION_COLOR_READ_FORMAT_OES         "     "     "     "
   COMPRESSED TEXTURE FORMATS             is incorrectly flagged as 10xZ rather than 10*xZ in state tables

Various state table entries don't specify tuple orders.

The spec says that the env argument to GetTexEnv() must be TEXTURE ENV, but it's pretty
clear that it should be POINT_SPRITE_OES if asking for COORD_REPLACE_OES to agree with TexEnv()

It's not clear what to do if a vector-valued state variable is set using a scalar setter function. Generally
we have generated the 4-vector (x, x, x, 1) to allow opaque greyscale colors to be set using the scalar
version.

It's not clear what to do if glBufferData specifies a negative size
It's not clear what to do if a null pointer is passed in to glBufferSubData.

The line "s is masked to the number of bitplanes in the stencil buffer." doesn't make all that much sense,
since we can later change the stencil buffer without changing the stencil clear value. Also, what happens
if there is no stencil buffer?

Search for "deviation" to find possible deviations.


Boilerplate that we use

   If no current error, the following conditions cause error to assume the specified value

      GL_INVALID_VALUE              size < 0
      GL_INVALID_ENUM               usage invalid
      GL_INVALID_ENUM               target invalid
      GL_INVALID_OPERATION          no buffer bound to target
      GL_OUT_OF_MEMORY              failed to allocate buffer

   if more than one condition holds, the first error is generated.


   However it is perfectly legal to bind a buffer as the backing for the vertex color array,
   delete the buffer FROM ANOTHER EGL CONTEXT SHARING WITH THIS ONE, and then expect to
   continue using the buffer.

*/


/*
   uint32_t get_texenv_integer_internal(GLenum env, GLenum pname, int *params)

   Returns an integer texture unit state variable.  A utility
   function shared by GetTexEnviv() GetTexEnvfv() GetTexEnvxv()

   Native Integer Texture Unit State Variables

   COORD REPLACE OES GetTexEnviv (retrieved with GL_POINT_SPRITE_OES target)
   TEXTURE ENV MODE GetTexEnviv
   COMBINE RGB GetTexEnviv
   COMBINE ALPHA GetTexEnviv
   SRC0 RGB GetTexEnviv
   SRC1 RGB GetTexEnviv
   SRC2 RGB GetTexEnviv
   SRC0 ALPHA GetTexEnviv
   SRC1 ALPHA GetTexEnviv
   SRC2 ALPHA GetTexEnviv
   OPERAND0 RGB GetTexEnviv
   OPERAND1 RGB GetTexEnviv
   OPERAND2 RGB GetTexEnviv
   OPERAND0 ALPHA GetTexEnviv
   OPERAND1 ALPHA GetTexEnviv
   OPERAND2 ALPHA GetTexEnviv
*/

static uint32_t get_texenv_integer_internal(GLenum env, GLenum pname, int *params)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE_UNCHANGED();
   int t;
   uint32_t tbits;
   uint32_t sh;
   int result = 0;
   if (!state) return 0;

   t = state->active_texture - GL_TEXTURE0;
   tbits = state->gl11.statebits.texture[t];

   switch (env) {
   case GL_POINT_SPRITE_OES:
      switch (pname) {
      case GL_COORD_REPLACE_OES:
         params[0] = !!(tbits & GL11_TEX_COORDREPLACE);
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
         params[0] = state->gl11.texunits[t].mode;
         result = 1;
         break;
      case GL_COMBINE_RGB:
         params[0] = untranslate_combine((tbits & GL11_TEX_CC_M) >> GL11_TEX_CC_S);
         result = 1;
         break;
      case GL_COMBINE_ALPHA:
         params[0] = untranslate_combine((tbits & GL11_TEX_AC_M) >> GL11_TEX_AC_S);
         result = 1;
         break;
      case GL_SRC0_RGB:
      case GL_SRC1_RGB:
      case GL_SRC2_RGB:
         sh = GL11_TEX_CS_S + 2 * (pname - GL_SRC0_RGB);
         params[0] = untranslate_tex_source((tbits >> sh) & GL11_SRC_M);
         result = 1;
         break;
      case GL_SRC0_ALPHA:
      case GL_SRC1_ALPHA:
      case GL_SRC2_ALPHA:
         sh = GL11_TEX_AS_S + 2 * (pname - GL_SRC0_ALPHA);
         params[0] = untranslate_tex_source((tbits >> sh) & GL11_SRC_M);
         result = 1;
         break;
      case GL_OPERAND0_RGB:
      case GL_OPERAND1_RGB:
      case GL_OPERAND2_RGB:
         sh = GL11_TEX_CO_S + 2 * (pname - GL_OPERAND0_RGB);
         params[0] = untranslate_tex_operand((tbits >> sh) & GL11_OPC_M);
         result = 1;
         break;
      case GL_OPERAND0_ALPHA:
      case GL_OPERAND1_ALPHA:
      case GL_OPERAND2_ALPHA:
         sh = GL11_TEX_AO_S + (pname - GL_OPERAND0_ALPHA);
         params[0] = untranslate_tex_operand((tbits >> sh) & GL11_OPA_M);
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

   GL11_UNLOCK_SERVER_STATE();

   return result;
}

/*
   int get_texenv_float_internal(GLenum env, GLenum pname, float *params)

   Called by GetTexEnv{iv,fv,xv}()

   pname must be one of the native floating-point texture unit state variables:
   TEXTURE ENV COLOR GetTexEnvfv (4) .
   RGB SCALE GetTexEnvfv .
   ALPHA SCALE GetTexEnvfv .
*/

static int get_texenv_float_internal(GLenum env, GLenum pname, float *params)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE_UNCHANGED();
   int t;
   int result = 0;
   if (!state) return 0;

   t = state->active_texture - GL_TEXTURE0;

   switch (env) {
   case GL_TEXTURE_ENV:
      switch (pname) {
      case GL_TEXTURE_ENV_COLOR:
         {
            int i;
            for (i = 0; i < 4; i++)
               params[i] = state->gl11.texunits[t].color[i];
            result = 4;
         }
         break;
      case GL_RGB_SCALE:
         params[0] = state->gl11.texunits[t].rgb_scale;
         result = 1;
         break;
      case GL_ALPHA_SCALE:
         params[0] = state->gl11.texunits[t].alpha_scale;
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

   GL11_UNLOCK_SERVER_STATE();

   return result;
}

static GL11_LIGHT_T *get_light(GLXX_SERVER_STATE_T *state, GLenum l)
{
   if (l >= GL_LIGHT0 && l < GL_LIGHT0 + GL11_CONFIG_MAX_LIGHTS)
      return &state->gl11.lights[l - GL_LIGHT0];
   else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return NULL;
   }
}

/* Return the active matrix stack according to matrix_mode */
static GL11_MATRIX_STACK_T *get_stack(GLXX_SERVER_STATE_T *state)
{
   switch (state->gl11.matrix_mode) {
   case GL_MODELVIEW:
      return &state->gl11.modelview;
   case GL_PROJECTION:
      return &state->gl11.projection;
   case GL_TEXTURE:
      return &state->gl11.texunits[state->active_texture - GL_TEXTURE0].stack;
   default:
      UNREACHABLE();
      return NULL;
   }
}

/*
   glAlphaFunc(GLenum func, GLclampf ref)
   glAlphaFuncx(GLenum func, GLclampx ref)

   ref is clamped to lie in [0, 1], and then converted to a fixed-point value according
   to the rules given for an A component in section 2.12.8.

   Possible deviation from spec in that we keep the value as floating point and compare with
   the floating point fragment alpha at fragment shading time.
*/

static void alpha_func_internal(GLenum func, float ref)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   uint32_t f;
   if (!state) return;

   f = translate_alpha_func(func);
   if (f != ~0u) {
      SET_MASKED(state->gl11.statebits.fragment, f, GL11_AFUNC_M);
      state->gl11.alpha_func.ref = clampf(ref, 0.0f, 1.0f);
   } else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   GL11_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glAlphaFunc (GLenum func, GLclampf ref)
{
   alpha_func_internal(func, ref);
}

GL_APICALL void GL_APIENTRY glAlphaFuncx (GLenum func, GLclampx ref)
{
   alpha_func_internal(func, fixed_to_float(ref));
}



/*
   glFogf (GLenum pname, GLfloat param)
   glFogfv (GLenum pname, const GLfloat *params)
   glFogx (GLenum pname, GLfixed param)
   glFogxv (GLenum pname, const GLfixed *params)

   Invariants preserved:

   state.fog.mode in {EXP, EXP2, LINEAR}
   0.0 <= state.fog.color[i] <= 1.0
   state.fog.density >= 0.0
   state.fog.scale is consistent with start and end
   state.fog.coeff_exp and state.fog.coeff_exp2 are consistent with density
*/

static void fogv_internal(GLenum pname, const GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   switch (pname) {
   case GL_FOG_MODE:
   {
      uint32_t m = translate_fog_mode((GLenum)params[0]);

      if (m != ~0u)
         SET_MASKED(state->gl11.statebits.fragment, m, GL11_FOG_M);
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);

      break;
   }
   case GL_FOG_DENSITY:
   {
      GLfloat d = params[0];

      if (d >= 0.0f) {
         state->gl11.fog.density = d;

         state->gl11.fog.coeff_exp = -d * LOG2E;
         state->gl11.fog.coeff_exp2 = -d * d * LOG2E;
      } else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);

      break;
   }
   case GL_FOG_START:
      state->gl11.fog.start = params[0];

      state->gl11.fog.scale = 1.0f / (state->gl11.fog.end - state->gl11.fog.start);
      break;
   case GL_FOG_END:
      state->gl11.fog.end = params[0];

      state->gl11.fog.scale = 1.0f / (state->gl11.fog.end - state->gl11.fog.start);
      break;
   case GL_FOG_COLOR:
      {
         int i;
         for (i = 0; i < 4; i++)
            state->gl11.fog.color[i] = clampf(params[i], 0.0f, 1.0f);
      }
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

   GL11_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glFogf (GLenum pname, GLfloat param)
{
   GLfloat params[4];

   params[0] = param;
   params[1] = param;
   params[2] = param;
   params[3] = 1.0f;

   fogv_internal(pname, params);
}

GL_APICALL void GL_APIENTRY glFogfv (GLenum pname, const GLfloat *params)
{
   fogv_internal(pname, params);
}

static GLboolean fog_requires_scaling(GLenum pname)
{
   return pname != GL_FOG_MODE;
}

GL_APICALL void GL_APIENTRY glFogx (GLenum pname, GLfixed param)
{
   GLfloat floatParam = fog_requires_scaling(pname) ? fixed_to_float(param) : (GLfloat)param;
   GLfloat params[4];

   params[0] = floatParam;
   params[1] = floatParam;
   params[2] = floatParam;
   params[3] = 1.0f;

   fogv_internal(pname, params);
}

GL_APICALL void GL_APIENTRY glFogxv (GLenum pname, const GLfixed *params)
{
   GLfloat temp[4];

   if (fog_requires_scaling(pname)) {
      int i;
      for (i = 0; i < 4; i++)
         temp[i] = fixed_to_float(params[i]);
   } else {
      int i;
      for (i = 0; i < 4; i++)
         temp[i] = (GLfloat)params[i];
   }

   fogv_internal(pname, temp);
}


GL_APICALL void GL_APIENTRY glGetFixedv (GLenum pname, GLfixed *params)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   if (!state) return;

   glxx_get_fixed_internal(state, pname, params);

   GLXX_UNLOCK_SERVER_STATE();
}

/*
   glGetLightfv (GLenum light, GLenum pname, GLfloat *params)
   glGetLightxv (GLenum light, GLenum pname, GLfixed *params)

   Khronos documentation:

   GetLight places information about value (a symbolic constant) for light (also a
   symbolic constant) in data. POSITION or SPOT DIRECTION returns values in eye
   coordinates (again, these are the coordinates that were computed when the position
   or direction was specified).

   AMBIENT GetLightfv (4)
   DIFFUSE GetLightfv (4)
   SPECULAR GetLightfv (4)
   POSITION GetLightfv (4)
   CONSTANT ATTENUATION GetLightfv
   LINEAR ATTENUATION GetLightfv
   QUADRATIC ATTENUATION GetLightfv
   SPOT DIRECTION GetLightfv (3)
   SPOT EXPONENT GetLightfv
   SPOT CUTOFF GetLightfv

   Other commands exist to obtain state variables that are identified by a category
   (clip plane, light, material, etc.) as well as a symbolic constant. These are
      ...
      void GetLight{xf}v( enum light, enum value, T data );
      ...
*/

static uint32_t get_lightv_internal(GLenum l, GLenum pname, float *params)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE_UNCHANGED();
   GL11_LIGHT_T *light;
   uint32_t result;
   if (!state) return 0;

   light = get_light(state, l);

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

   GL11_UNLOCK_SERVER_STATE();

   return result;
}

GL_APICALL void GL_APIENTRY glGetLightfv (GLenum light, GLenum pname, GLfloat *params)
{
   get_lightv_internal(light, pname, params);
}

GL_APICALL void GL_APIENTRY glGetLightxv (GLenum light, GLenum pname, GLfixed *params)
{
   float temp[4];

   uint32_t count = get_lightv_internal(light, pname, temp);
   unsigned int i;

   assert(count <= 4);

   for (i = 0; i < count; i++)
      params[i] = float_to_fixed(temp[i]);
}

/*
   glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params)
   glGetMaterialxv (GLenum face, GLenum pname, GLfixed *params)

   Khronos documentation:

   GetMaterial, GetTexEnv, GetTexParameter, and GetBufferParameter are
   similar to GetLight, placing information about value for the target indicated by
   their first argument into data. The face argument to GetMaterial must be either
   FRONT or BACK, indicating the front or back material, respectively.

   AMBIENT GetMaterialfv (4)
   DIFFUSE GetMaterialfv (4)
   SPECULAR GetMaterialfv (4)
   EMISSION GetMaterialfv (4)
   SHININESS GetMaterialfv

   Other commands exist to obtain state variables that are identified by a category
   (clip plane, light, material, etc.) as well as a symbolic constant. These are
      ...
      void GetMaterial{xf}v( enum face, enum value, T data );
      ...

   Implementation notes:

   From the specification of GL_MATERIAL we have

   "For the Material command, face must be FRONT AND BACK, indicating that the
   property name of both the front and back material, should be set."

   Under GL ES 1.1 there is no concept of separate front and back materials, so
   we merely check the face parameter for validity but otherwise ignore it.
*/

static GLboolean is_single_face(GLenum face)
{
   return face == GL_FRONT ||
          face == GL_BACK;
}

static uint32_t get_materialv_internal(GLenum face, GLenum pname, float *params)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE_UNCHANGED();
   uint32_t result;
   if (!state) return 0;

   /* TODO: This looks wrong according to the spec pasted above */
   if (is_single_face(face))
      switch (pname) {
      case GL_AMBIENT:
         {
            int i;
            for (i = 0; i < 4; i++)
               params[i] = state->gl11.material.ambient[i];
            result = 4;
         }
         break;
      case GL_DIFFUSE:
      {
         int i;
         for (i = 0; i < 4; i++)
            params[i] = state->gl11.material.diffuse[i];
         result = 4;
         break;
      }
      case GL_SPECULAR:
      {
         int i;
         for (i = 0; i < 4; i++)
            params[i] = state->gl11.material.specular[i];
         result = 4;
         break;
      }
      case GL_EMISSION:
      {
         int i;
         for (i = 0; i < 4; i++)
            params[i] = state->gl11.material.emission[i];
         result = 4;
         break;
      }
      case GL_SHININESS:
         params[0] = state->gl11.material.shininess;
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

   GL11_UNLOCK_SERVER_STATE();

   return result;
}

GL_APICALL void GL_APIENTRY glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params)
{
   get_materialv_internal(face, pname, params);
}

GL_APICALL void GL_APIENTRY glGetMaterialxv (GLenum face, GLenum pname, GLfixed *params)
{
   unsigned int i;
   float temp[4];

   uint32_t count = get_materialv_internal(face, pname, temp);

   assert(count <= 4);

   for (i = 0; i < count; i++)
      params[i] = float_to_fixed(temp[i]);
}

/*
   int glGetTexEnviv (GLenum env, GLenum pname, GLint *params)

   Khronos documentation:

   The command glTexEnv() sets parameters of the texture environment that specifies
   how texture values are interpreted when texturing a fragment; env must be TEXTURE ENV.

      :  :  :

   The point sprite texture coordinate replacement mode is set with the command glTexEnv()
   where env is POINT SPRITE OES and pname is COORD REPLACE OES

   State variables that can be obtained using any of GetBooleanv, GetIntegerv, GetFixedv, or
   GetFloatv are listed with just one of these commands... State variables for which any other
   command is listed as the query command can be obtained only by using that command.

   The env argument to GetTexEnv must be TEXTURE ENV*

   COORD REPLACE OES GetTexEnviv (retrieved with GL_POINT_SPRITE_OES target)
   TEXTURE ENV MODE GetTexEnviv
   COMBINE RGB GetTexEnviv
   COMBINE ALPHA GetTexEnviv
   SRC0 RGB GetTexEnviv
   SRC1 RGB GetTexEnviv
   SRC2 RGB GetTexEnviv
   SRC0 ALPHA GetTexEnviv
   SRC1 ALPHA GetTexEnviv
   SRC2 ALPHA GetTexEnviv
   OPERAND0 RGB GetTexEnviv
   OPERAND1 RGB GetTexEnviv
   OPERAND2 RGB GetTexEnviv
   OPERAND0 ALPHA GetTexEnviv
   OPERAND1 ALPHA GetTexEnviv
   OPERAND2 ALPHA GetTexEnviv

   TEXTURE ENV COLOR GetTexEnvfv (4) .
   RGB SCALE GetTexEnvfv .
   ALPHA SCALE GetTexEnvfv .

   * but note we don't strictly comply with this

   Other commands exist to obtain state variables that are identified by a category
   (clip plane, light, material, etc.) as well as a symbolic constant. These are
      ...
      void GetTexEnv{i}v( enum env, enum value, T data );
      ...

   Implementation notes:

   We don't believe that integer values can be retrieved from glGetTexEnv{xf}v and
   vice versa, but everyone else does. The spec says that the env argument to GetTexEnv()
   must be TEXTURE ENV, but it's pretty clear that it should be POINT_SPRITE_OES if asking
   for COORD_REPLACE_OES to agree with TexEnv()
*/

GL_APICALL void GL_APIENTRY glGetTexEnviv (GLenum env, GLenum pname, GLint *params)
{
   uint32_t count;
   switch (env) {
   case GL_POINT_SPRITE_OES:
      switch (pname) {
      case GL_COORD_REPLACE_OES:
         count = get_texenv_integer_internal(env, pname, params);
         return;
      default:
         {
         GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE_UNCHANGED();
         if (!state) return;

         glxx_server_state_set_error(state, GL_INVALID_ENUM);

         GL11_UNLOCK_SERVER_STATE();
         return;
         }
      }
      UNREACHABLE();
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
         count = get_texenv_integer_internal(env, pname, params);
         return;
      case GL_TEXTURE_ENV_COLOR:
      {
         GLfloat temp[4];
         GLuint i;

         count = get_texenv_float_internal(env, pname, temp);
         assert(count <= 4);

         for (i = 0; i < count; i++) {
            params[i] = (GLint)floor((4294967295.0f * temp[i] - 1.0f) / 2.0f + 0.5f);

            if (params[i] < 0)
               params[i] = 0x7fffffff;
         }

         return;
      }
      case GL_RGB_SCALE:
      case GL_ALPHA_SCALE:
      {
         GLfloat temp[1];

         count = get_texenv_float_internal(env, pname, temp);
         assert(count <= 1);

         if (count == 1)
         {
            params[0] = float_to_int(temp[0]);
         }

         return;
      }
      default:
         {
         GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE_UNCHANGED();
         if (!state) return;

         glxx_server_state_set_error(state, GL_INVALID_ENUM);

         GL11_UNLOCK_SERVER_STATE();
         return;
         }
      }
      UNREACHABLE();
      break;
   default:
      {
      GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE_UNCHANGED();
      if (!state) return;

      glxx_server_state_set_error(state, GL_INVALID_ENUM);

      GL11_UNLOCK_SERVER_STATE();
      return;
      }
   }
}

/*
   void glGetTexEnvfv (GLenum env, GLenum pname, GLfloat *params)
   void glGetTexEnvxv (GLenum env, GLenum pname, GLfixed *params)

   We don't believe that integer values can be retrieved from glGetTexEnv{xf}v and
   vice versa, but everyone else does. See above for valid pnames and comments.
*/

static uint32_t get_texenv_float_or_fixed_internal (GLenum env, GLenum pname, float *params)
{
   switch (env) {
   case GL_POINT_SPRITE_OES:
      switch (pname) {
      case GL_COORD_REPLACE_OES:
      {
         int temp;
         uint32_t count = get_texenv_integer_internal(env, pname, &temp);

         assert(count <= 1);

         if (count == 1)
         {
            params[0] = (float)temp;
         }

         return count;
      }
      default:
         {
         GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE_UNCHANGED();
         if (!state) return 0;

         glxx_server_state_set_error(state, GL_INVALID_ENUM);

         GL11_UNLOCK_SERVER_STATE();
         return 0;
         }
      }
      UNREACHABLE();
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
         uint32_t count = get_texenv_integer_internal(env, pname, &temp);

         if (count == 1)
         {
            params[0] = (float)temp;
         }

         return count;
      }
      case GL_TEXTURE_ENV_COLOR:
      case GL_RGB_SCALE:
      case GL_ALPHA_SCALE:
         return get_texenv_float_internal(env, pname, params);
      default:
         {
         GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE_UNCHANGED();
         if (!state) return 0;

         glxx_server_state_set_error(state, GL_INVALID_ENUM);

         GL11_UNLOCK_SERVER_STATE();
         return 0;
         }
      }
      UNREACHABLE();
      break;
   default:
      {
      GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE_UNCHANGED();
      if (!state) return 0;

      glxx_server_state_set_error(state, GL_INVALID_ENUM);

      GL11_UNLOCK_SERVER_STATE();
      return 0;
      }
   }
}

GL_APICALL void GL_APIENTRY glGetTexEnvfv (GLenum env, GLenum pname, GLfloat *params)
{
   get_texenv_float_or_fixed_internal(env, pname, params);
}

/*
   glLightModelf (GLenum pname, GLfloat param)
   glLightModelfv (GLenum pname, const GLfloat *params)
   glLightModelx (GLenum pname, GLfixed param)
   glLightModelxv (GLenum pname, const GLfixed *params)

   Khronos documentation:

   Lighting parameters are divided into three categories: material parameters, light
   source parameters, and lighting model parameters (see Table 2.8). Sets of lighting
   parameters are specified with
            :  :  :  :
      void LightModel{xf}( enum pname, T param );
      void LightModel{xf}v( enum pname, T params );

   Lighting Model Parameters

      LIGHT MODEL AMBIENT 4
      LIGHT MODEL TWO SIDE 1

   The range of light, lightmodel and material color parameters is -INF to INF (i.e. unclamped)
*/

static void lightmodelv_internal (GLenum pname, const GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   switch (pname) {
   case GL_LIGHT_MODEL_AMBIENT:
      {
         int i;
         for (i = 0; i < 4; i++)
            state->gl11.lightmodel.ambient[i] = params[i];
         break;
      }
   case GL_LIGHT_MODEL_TWO_SIDE:
      SET_INDIVIDUAL(state->gl11.statebits.vertex, GL11_TWOSIDE, params[0] != 0.0f);
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

   GL11_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glLightModelf (GLenum pname, GLfloat param)
{
   GLfloat params[4];

   params[0] = param;
   params[1] = param;
   params[2] = param;
   params[3] = 1.0f;

   lightmodelv_internal(pname, params);
}

GL_APICALL void GL_APIENTRY glLightModelfv (GLenum pname, const GLfloat *params)
{
   lightmodelv_internal(pname, params);
}

GL_APICALL void GL_APIENTRY glLightModelx (GLenum pname, GLfixed param)
{
   GLfloat params[4];

   params[0] = fixed_to_float(param);
   params[1] = fixed_to_float(param);
   params[2] = fixed_to_float(param);
   params[3] = 1.0f;

   lightmodelv_internal(pname, params);
}

GL_APICALL void GL_APIENTRY glLightModelxv (GLenum pname, const GLfixed *params)
{
   int i;
   GLfloat temp[4];

   for (i = 0; i < 4; i++)
      temp[i] = fixed_to_float(params[i]);

   lightmodelv_internal(pname, temp);
}
