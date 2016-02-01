/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of common OpenGL ES 1.1 and 2.0 state machine functions.
=============================================================================*/

#include "interface/khronos/glxx/gl_public_api.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/glxx/glxx_shared.h"

#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_server_internal.h"

#include "middleware/khronos/glxx/glxx_texture.h"
#include "middleware/khronos/glxx/glxx_buffer.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "middleware/khronos/common/khrn_interlock.h"

#include "interface/khronos/glxx/gl11_int_config.h"

#include "middleware/khronos/glxx/glxx_hw.h"
#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"
#include "middleware/khronos/glxx/glxx_translate.h"
#include "middleware/khronos/glxx/glxx_extensions.h"

#include "middleware/khronos/ext/gl_brcm_provoking_vertex.h"

#include <string.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "vcos.h"

#include "helpers/gfx/gfx_lfmt_translate_gl.h"

static void get_int64_internal(GLXX_SERVER_STATE_T *state, GLenum pname, GLint64 *params);
static int glxx_get_float_internal(GLXX_SERVER_STATE_T *state, GLenum pname, GLfloat *params);
static int glxx_get_signed_integer_internal(GLXX_SERVER_STATE_T *state, GLenum pname, int *params);
static int glxx_get_unsigned_integer_internal(GLXX_SERVER_STATE_T *state, GLenum pname, unsigned int *params);

#define MAX_BOOLEAN_ITEM_COUNT 16
#define MAX_INTEGER_ITEM_COUNT 64
#define MAX_FLOAT_ITEM_COUNT 64

GL_API GLenum GL_APIENTRY glGetError(void)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   GLenum result;
   if (!state) return 0;

   result = state->error;

   state->error = GL_NO_ERROR;

   GLXX_UNLOCK_SERVER_STATE();

   return result;
}

static bool glxx_is_boolean(GLXX_SERVER_STATE_T *state, GLenum pname)
{
   switch (pname) {

   // Common GLES 1.1 and 3.0 booleans
   case GL_SAMPLE_COVERAGE_INVERT:
   case GL_COLOR_WRITEMASK:
   case GL_DEPTH_WRITEMASK:
   case GL_CULL_FACE:
   case GL_POLYGON_OFFSET_FILL:
   case GL_SAMPLE_ALPHA_TO_COVERAGE:
   case GL_SAMPLE_COVERAGE:
   case GL_SCISSOR_TEST:
   case GL_STENCIL_TEST:
   case GL_DEPTH_TEST:
   case GL_BLEND:
   case GL_DITHER:
   case GL_DEBUG_OUTPUT_KHR:
   case GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR:
      return true;

   // GLES 1.1 specific booleans
   case GL_LIGHT_MODEL_TWO_SIDE:
   case GL_NORMALIZE:
   case GL_RESCALE_NORMAL:
   case GL_CLIP_PLANE0:
   case GL_FOG:
   case GL_LIGHTING:
   case GL_COLOR_MATERIAL:
   case GL_LIGHT0:
   case GL_LIGHT1:
   case GL_LIGHT2:
   case GL_LIGHT3:
   case GL_LIGHT4:
   case GL_LIGHT5:
   case GL_LIGHT6:
   case GL_LIGHT7:
   case GL_POINT_SMOOTH:
   case GL_POINT_SPRITE_OES:
   case GL_LINE_SMOOTH:
   case GL_MULTISAMPLE:
   case GL_SAMPLE_ALPHA_TO_ONE:
   case GL_TEXTURE_2D:
   case GL_ALPHA_TEST:
   case GL_COLOR_LOGIC_OP:
   case GL_VERTEX_ARRAY:
   case GL_NORMAL_ARRAY:
   case GL_COLOR_ARRAY:
   case GL_TEXTURE_COORD_ARRAY:
   case GL_POINT_SIZE_ARRAY_OES:
#if GL_OES_matrix_palette
   case GL_MATRIX_INDEX_ARRAY_OES:
   case GL_WEIGHT_ARRAY_OES:
#endif
      return IS_GL_11(state);

   // GLES 3.0 specific booleans
   case GL_SHADER_COMPILER:
   case GL_TRANSFORM_FEEDBACK_PAUSED:
   case GL_TRANSFORM_FEEDBACK_ACTIVE:
   case GL_PRIMITIVE_RESTART_FIXED_INDEX:
   case GL_RASTERIZER_DISCARD:
      return !IS_GL_11(state);
   case GL_SAMPLE_MASK:
      return KHRN_GLES31_DRIVER ? !IS_GL_11(state) : false;
#if GL_EXT_robustness
   case GL_CONTEXT_ROBUST_ACCESS_EXT:
      return true;
#endif
   default:
      return false;
   }
}

static bool glxx_is_unsigned_integer(GLXX_SERVER_STATE_T *state, GLenum pname)
{
   switch (pname) {
   // Common GLES 1.1 and 2.0 integers
   case GL_CULL_FACE_MODE:
   case GL_FRONT_FACE:
   case GL_TEXTURE_BINDING_2D:
   case GL_TEXTURE_BINDING_EXTERNAL_OES:
   case GL_ACTIVE_TEXTURE:
   case GL_STENCIL_FUNC:
   case GL_STENCIL_WRITEMASK:
   case GL_STENCIL_VALUE_MASK:
   case GL_STENCIL_FAIL:
   case GL_STENCIL_PASS_DEPTH_FAIL:
   case GL_STENCIL_PASS_DEPTH_PASS:
   case GL_DEPTH_FUNC:
   case GL_GENERATE_MIPMAP_HINT:
   case GL_SUBPIXEL_BITS:
   case GL_MAX_TEXTURE_SIZE:
   case GL_MAX_VIEWPORT_DIMS:
   case GL_SAMPLE_BUFFERS:
   case GL_SAMPLES:
   case GL_COMPRESSED_TEXTURE_FORMATS:
   case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
   case GL_RED_BITS:
   case GL_GREEN_BITS:
   case GL_BLUE_BITS:
   case GL_ALPHA_BITS:
   case GL_DEPTH_BITS:
   case GL_STENCIL_BITS:
   case GL_FRAMEBUFFER_BINDING:
   case GL_RENDERBUFFER_BINDING:
   case GL_MAX_RENDERBUFFER_SIZE:
   case GL_ARRAY_BUFFER_BINDING:
   case GL_ELEMENT_ARRAY_BUFFER_BINDING:
   case GL_IMPLEMENTATION_COLOR_READ_TYPE_OES:
   case GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES:
   case GL_UNPACK_ALIGNMENT:
   case GL_UNPACK_ROW_LENGTH:
   case GL_UNPACK_SKIP_ROWS:
   case GL_UNPACK_SKIP_PIXELS:
   case GL_UNPACK_SKIP_IMAGES:
   case GL_UNPACK_IMAGE_HEIGHT:
   case GL_PACK_ALIGNMENT:
   case GL_PACK_ROW_LENGTH:
   case GL_PACK_SKIP_ROWS:
   case GL_PACK_SKIP_PIXELS:
   case GL_MAX_DEBUG_MESSAGE_LENGTH_KHR:
   case GL_MAX_DEBUG_LOGGED_MESSAGES_KHR:
   case GL_MAX_DEBUG_GROUP_STACK_DEPTH_KHR:
   case GL_MAX_LABEL_LENGTH_KHR:
   case GL_DEBUG_LOGGED_MESSAGES_KHR:
   case GL_DEBUG_GROUP_STACK_DEPTH_KHR:
   case GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_KHR:
      return true;

   // GLES 1.1 specific integers
   case GL_MODELVIEW_STACK_DEPTH:
   case GL_PROJECTION_STACK_DEPTH:
   case GL_TEXTURE_STACK_DEPTH:
   case GL_MATRIX_MODE:
   case GL_FOG_MODE:
   case GL_SHADE_MODEL:
   case GL_ALPHA_TEST_FUNC:
   case GL_BLEND_SRC:
   case GL_BLEND_DST:
   case GL_LOGIC_OP_MODE:
   case GL_PERSPECTIVE_CORRECTION_HINT:
   case GL_POINT_SMOOTH_HINT:
   case GL_LINE_SMOOTH_HINT:
   case GL_FOG_HINT:
   case GL_MAX_LIGHTS:
   case GL_MAX_CLIP_PLANES:
   case GL_MAX_MODELVIEW_STACK_DEPTH:
   case GL_MAX_PROJECTION_STACK_DEPTH:
   case GL_MAX_TEXTURE_STACK_DEPTH:
   case GL_MAX_TEXTURE_UNITS:
   case GL_MAX_PALETTE_MATRICES_OES:
   case GL_MAX_VERTEX_UNITS_OES:
   case GL_CURRENT_PALETTE_MATRIX_OES:
   case GL_CLIENT_ACTIVE_TEXTURE:
   case GL_VERTEX_ARRAY_SIZE:
   case GL_VERTEX_ARRAY_TYPE:
   case GL_VERTEX_ARRAY_STRIDE:
   case GL_VERTEX_ARRAY_BUFFER_BINDING:
   case GL_NORMAL_ARRAY_TYPE:
   case GL_NORMAL_ARRAY_STRIDE:
   case GL_NORMAL_ARRAY_BUFFER_BINDING:
   case GL_COLOR_ARRAY_SIZE:
   case GL_COLOR_ARRAY_TYPE:
   case GL_COLOR_ARRAY_STRIDE:
   case GL_COLOR_ARRAY_BUFFER_BINDING:
   case GL_TEXTURE_COORD_ARRAY_SIZE:
   case GL_TEXTURE_COORD_ARRAY_TYPE:
   case GL_TEXTURE_COORD_ARRAY_STRIDE:
   case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
   case GL_POINT_SIZE_ARRAY_TYPE_OES:
   case GL_POINT_SIZE_ARRAY_STRIDE_OES:
   case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
#if GL_OES_matrix_palette
   case GL_MATRIX_INDEX_ARRAY_SIZE_OES:
   case GL_MATRIX_INDEX_ARRAY_TYPE_OES:
   case GL_MATRIX_INDEX_ARRAY_STRIDE_OES:
   case GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES:
   case GL_WEIGHT_ARRAY_SIZE_OES:
   case GL_WEIGHT_ARRAY_TYPE_OES:
   case GL_WEIGHT_ARRAY_STRIDE_OES:
   case GL_WEIGHT_ARRAY_BUFFER_BINDING_OES:
#endif
      return IS_GL_11(state);

   // GLES 2.0 / 3.0 integers
   case GL_TEXTURE_BINDING_CUBE_MAP:
   case GL_STENCIL_BACK_WRITEMASK:
   case GL_STENCIL_BACK_FUNC:
   case GL_STENCIL_BACK_VALUE_MASK:
   case GL_STENCIL_BACK_FAIL:
   case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
   case GL_STENCIL_BACK_PASS_DEPTH_PASS:
   case GL_BLEND_SRC_RGB:
   case GL_BLEND_SRC_ALPHA:
   case GL_BLEND_DST_RGB:
   case GL_BLEND_DST_ALPHA:
   case GL_BLEND_EQUATION_RGB:
   case GL_BLEND_EQUATION_ALPHA:
   case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
   case GL_SHADER_BINARY_FORMATS:
   case GL_NUM_SHADER_BINARY_FORMATS:
   case GL_MAX_VERTEX_ATTRIBS:
   case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
   case GL_MAX_VERTEX_UNIFORM_VECTORS:
   case GL_MAX_VARYING_VECTORS:
   case GL_MAX_VARYING_COMPONENTS:
   case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
   case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
   case GL_MAX_TEXTURE_IMAGE_UNITS:
   case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
   case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
   case GL_CURRENT_PROGRAM:
      return !IS_GL_11(state);

   // GLES 3.0 specific integers
   case GL_MAX_ELEMENTS_VERTICES:
   case GL_MAX_ELEMENTS_INDICES:
   case GL_MAX_ELEMENT_INDEX:
   case GL_MAX_DRAW_BUFFERS:
   case GL_TEXTURE_BINDING_3D:
   case GL_TEXTURE_BINDING_2D_ARRAY:
   case GL_MAX_COLOR_ATTACHMENTS:
   case GL_MAX_UNIFORM_BUFFER_BINDINGS:
   case GL_UNIFORM_BUFFER_BINDING:        /* Both an indexed and normal integer */
   case GL_TRANSFORM_FEEDBACK_BINDING:
   case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
   case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS:
   case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:
   case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:
   case GL_READ_BUFFER:
   case GL_READ_FRAMEBUFFER_BINDING:
   case GL_MAX_3D_TEXTURE_SIZE:
   case GL_MAX_ARRAY_TEXTURE_LAYERS:
   case GL_PIXEL_PACK_BUFFER_BINDING:
   case GL_PIXEL_UNPACK_BUFFER_BINDING:
   case GL_COPY_READ_BUFFER:
   case GL_COPY_WRITE_BUFFER:
   case GL_VERTEX_ARRAY_BINDING:
   case GL_SAMPLER_BINDING:
   case GL_MAX_SAMPLES:
   case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
   case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
   case GL_MAX_VERTEX_UNIFORM_BLOCKS:
   case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
   case GL_MAX_COMBINED_UNIFORM_BLOCKS:
   case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
   case GL_NUM_EXTENSIONS:
   case GL_MAJOR_VERSION:
   case GL_MINOR_VERSION:
   case GL_PROGRAM_BINARY_FORMATS:
   case GL_NUM_PROGRAM_BINARY_FORMATS:
   case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
      return !IS_GL_11(state);
   case GL_DRAW_BUFFER0:
   case GL_DRAW_BUFFER1:
   case GL_DRAW_BUFFER2:
   case GL_DRAW_BUFFER3:
      {
         unsigned index = pname - GL_DRAW_BUFFER0;
         return (index < GLXX_MAX_RENDER_TARGETS) && !IS_GL_11(state);
      }

   // ES3.1 integers
   case GL_TEXTURE_BINDING_2D_MULTISAMPLE:
   case GL_DRAW_INDIRECT_BUFFER_BINDING:
   case GL_DISPATCH_INDIRECT_BUFFER_BINDING:
   case GL_ATOMIC_COUNTER_BUFFER_BINDING:
   case GL_SHADER_STORAGE_BUFFER_BINDING:
   case GL_MAX_FRAMEBUFFER_WIDTH:
   case GL_MAX_FRAMEBUFFER_HEIGHT:
   case GL_MAX_FRAMEBUFFER_SAMPLES:
   case GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS:
   case GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT:
   case GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS:
   case GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS:
   case GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS:
   case GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS:
   case GL_MAX_SAMPLE_MASK_WORDS:
   case GL_MAX_UNIFORM_LOCATIONS:
   case GL_MAX_IMAGE_UNITS:
   case GL_MAX_VERTEX_IMAGE_UNIFORMS:
   case GL_MAX_FRAGMENT_IMAGE_UNIFORMS:
   case GL_MAX_COMPUTE_IMAGE_UNIFORMS:
   case GL_MAX_COMBINED_IMAGE_UNIFORMS:
   case GL_MAX_COMPUTE_UNIFORM_COMPONENTS:
   case GL_MAX_COMPUTE_UNIFORM_BLOCKS:
   case GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS:
   case GL_MAX_COMPUTE_ATOMIC_COUNTERS:
   case GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS:
   case GL_MAX_VERTEX_ATOMIC_COUNTERS:
   case GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS:
   case GL_MAX_FRAGMENT_ATOMIC_COUNTERS:
   case GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS:
   case GL_MAX_COMBINED_ATOMIC_COUNTERS:
   case GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS:
   case GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS:
   case GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE:
   case GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES:
   case GL_MAX_VERTEX_ATTRIB_BINDINGS:
   case GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET:
   case GL_MAX_VERTEX_ATTRIB_STRIDE:
   case GL_MAX_COLOR_TEXTURE_SAMPLES:
   case GL_MAX_DEPTH_TEXTURE_SAMPLES:
   case GL_MAX_INTEGER_SAMPLES:
   case GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS:
   case GL_MAX_COMPUTE_SHARED_MEMORY_SIZE:
      return KHRN_GLES31_DRIVER ? !IS_GL_11(state) : false;

   // ES3 extension integers
#if GL_BRCM_provoking_vertex
   case GL_PROVOKING_VERTEX_BRCM:
      return gl_brcm_provoking_vertex_supported() && !IS_GL_11(state);
#endif

#if GL_EXT_robustness
   case GL_RESET_NOTIFICATION_STRATEGY_EXT:
      return true;
#endif

   default:
      return false;
   }
}


static bool glxx_is_signed_integer(GLXX_SERVER_STATE_T *state, GLenum pname)
{
   switch (pname) {
   // Common GLES 1.1 and 2.0 integers
   case GL_VIEWPORT:
   case GL_SCISSOR_BOX:
   case GL_STENCIL_CLEAR_VALUE:
   case GL_STENCIL_REF:
      return true;

   // GLES 1.1 specific integers
   case GL_MODELVIEW_MATRIX_FLOAT_AS_INT_BITS_OES:
   case GL_PROJECTION_MATRIX_FLOAT_AS_INT_BITS_OES:
   case GL_TEXTURE_MATRIX_FLOAT_AS_INT_BITS_OES:
      return IS_GL_11(state);

   // GLES 2.0 / 3.0 integers
   case GL_STENCIL_BACK_REF:
      return !IS_GL_11(state);

   // GLES 3.0 specific integers
   case GL_MIN_PROGRAM_TEXEL_OFFSET:
   case GL_MAX_PROGRAM_TEXEL_OFFSET:
   case GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET:
   case GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET:
      return !IS_GL_11(state);

   default:
      return false;
   }
}

static bool is_int64(GLXX_SERVER_STATE_T *state, GLenum pname)
{
   switch(pname) {
   case GL_MAX_ELEMENT_INDEX:
   case GL_MAX_SERVER_WAIT_TIMEOUT:
   case GL_MAX_UNIFORM_BLOCK_SIZE:
   case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
   case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
      return true;
   case GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS:
   case GL_MAX_SHADER_STORAGE_BLOCK_SIZE:
      return KHRN_GLES31_DRIVER;
   default: return false;
   }
}

static bool is_small_float_glxx(GLXX_SERVER_STATE_T *state, GLenum pname)
{
   switch (pname) {
   //common gl 1.1 and 2.0 floats in range 0-1
   case GL_DEPTH_RANGE:
   case GL_COLOR_CLEAR_VALUE:
   case GL_DEPTH_CLEAR_VALUE:
      return true;
   //gl 1.1 specific floats in range 0-1
   case GL_FOG_COLOR:
   case GL_LIGHT_MODEL_AMBIENT:
   case GL_ALPHA_TEST_REF:
   case GL_CURRENT_COLOR:
   case GL_CURRENT_NORMAL:
      return IS_GL_11(state);
   //gl 2.0 specific floats in range 0-1
   case GL_BLEND_COLOR:
      return !IS_GL_11(state);
   default:
      return false;
   }
}

static bool glxx_is_float(GLXX_SERVER_STATE_T *state, GLenum pname)
{
   switch (pname) {
   //common gl 1.1 and 2.0 floats
   case GL_DEPTH_RANGE:
   case GL_LINE_WIDTH:
   case GL_POLYGON_OFFSET_FACTOR:
   case GL_POLYGON_OFFSET_UNITS:
   case GL_SAMPLE_COVERAGE_VALUE:
   case GL_COLOR_CLEAR_VALUE:
   case GL_DEPTH_CLEAR_VALUE:
   case GL_ALIASED_POINT_SIZE_RANGE:
   case GL_ALIASED_LINE_WIDTH_RANGE:
#if GL_EXT_texture_filter_anisotropic
   case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
#endif
      return true;
   //gl 1.1 specific floats
   case GL_FOG_COLOR:
   case GL_LIGHT_MODEL_AMBIENT:
   case GL_MODELVIEW_MATRIX:
   case GL_PROJECTION_MATRIX:
   case GL_TEXTURE_MATRIX:
   case GL_FOG_DENSITY:
   case GL_FOG_START:
   case GL_FOG_END:
   case GL_POINT_SIZE_MIN:
   case GL_POINT_SIZE_MAX:
   case GL_POINT_FADE_THRESHOLD_SIZE:
   case GL_POINT_DISTANCE_ATTENUATION:
   case GL_ALPHA_TEST_REF:
   case GL_SMOOTH_POINT_SIZE_RANGE:
   case GL_SMOOTH_LINE_WIDTH_RANGE:
   case GL_CURRENT_TEXTURE_COORDS:
   case GL_CURRENT_COLOR:
   case GL_CURRENT_NORMAL:
   case GL_POINT_SIZE:
      return IS_GL_11(state);
   // GLES 2.0+ specific floats
   case GL_BLEND_COLOR:
   case GL_MAX_TEXTURE_LOD_BIAS:
      return !IS_GL_11(state);
   default:
      return false;
   }

}

/*
   glGetBooleanv()

   Gets the value(s) of a specified state variable into an array of
   booleans. Native integer and float variables return GL_FALSE if zero
   and GL_TRUE if non-zero. Gives GL_INVALID_ENUM if the state variable
   does not exist.
*/

GL_API void GL_APIENTRY glGetBooleanv(GLenum pname, GLboolean *params)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   if (!state) return;

   if (glxx_is_boolean(state, pname))
      (void)glxx_get_boolean_internal(state, pname, params);
   else if (is_int64(state, pname)) {
      int64_t ans;
      get_int64_internal(state, pname, &ans);
      params[0] = (ans != 0) ? GL_TRUE : GL_FALSE;
   } else if (glxx_is_signed_integer(state, pname)) {
      GLint temp[MAX_INTEGER_ITEM_COUNT];
      GLuint count = glxx_get_signed_integer_internal(state, pname, temp);
      GLuint i;

      assert(count <= MAX_INTEGER_ITEM_COUNT);

      for (i = 0; i < count; i++)
         params[i] = temp[i] != 0;

   } else if (glxx_is_unsigned_integer(state, pname)) {
      GLuint temp[MAX_INTEGER_ITEM_COUNT];
      GLuint count = glxx_get_unsigned_integer_internal(state, pname, temp);
      GLuint i;

      assert(count <= MAX_INTEGER_ITEM_COUNT);

      for (i = 0; i < count; i++)
         params[i] = temp[i] != 0;

   } else if (glxx_is_float(state, pname)) {
      GLfloat temp[MAX_FLOAT_ITEM_COUNT];
      GLuint count = glxx_get_float_internal(state, pname, temp);
      GLuint i;

      assert(count <= MAX_FLOAT_ITEM_COUNT);

      for (i = 0; i < count; i++)
         params[i] = temp[i] != 0.0f;

   } else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

   GLXX_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glGetIntegerv(GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   if (!state) return;

   if (glxx_is_boolean(state, pname)) {
      GLboolean temp[4];
      GLuint count = glxx_get_boolean_internal(state, pname, temp);
      GLuint i;

      assert(count <= 4);

      for (i = 0; i < count; i++)
         params[i] = temp[i] ? 1 : 0;

   } else if (glxx_is_signed_integer(state, pname)) {
      (void)glxx_get_signed_integer_internal(state, pname, params);
   } else if (glxx_is_unsigned_integer(state, pname)) {
      (void)glxx_get_unsigned_integer_internal(state, pname, (GLuint*)params);
   } else if (is_int64(state, pname)) {
      int64_t ans;
      get_int64_internal(state, pname, &ans);
      params[0] = (GLint)ans;
   } else if (is_small_float_glxx(state, pname)) {
      GLfloat temp[4];
      GLuint count = glxx_get_float_internal(state, pname, temp);
      GLuint i;

      assert(count <= 4);

      for (i = 0; i < count; i++) {
         params[i] = (GLint)floor((4294967295.0f * temp[i] - 1.0f) / 2.0f + 0.5f);

         if (params[i] < 0)
            params[i] = 0x7fffffff;
      }
   } else if (glxx_is_float(state, pname)) {
      GLfloat temp[MAX_FLOAT_ITEM_COUNT];
      GLuint count = glxx_get_float_internal(state, pname, temp);
      GLuint i;

      assert(count <= MAX_FLOAT_ITEM_COUNT);

      for (i = 0; i < count; i++)
         params[i] = float_to_int(temp[i]);
   } else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

   GLXX_UNLOCK_SERVER_STATE();
}

/*
   GetTexParameteriv

   TEXTURE MIN FILTER NEAREST MIPMAP LINEAR GetTexParameteriv
   TEXTURE MAG FILTER LINEAR GetTexParameteriv
   TEXTURE WRAP S REPEAT GetTexParameteriv
   TEXTURE WRAP T REPEAT GetTexParameteriv
   GENERATE MIPMAP GetTexParameter //gl 1.1 specific
*/

bool glxx_is_int_sampler_texparam(GLXX_SERVER_STATE_T *state, GLenum pname)
{
   switch(pname)
   {
      // Sampler parameters
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
         return true;

      case GL_TEXTURE_WRAP_R:
      case GL_TEXTURE_COMPARE_MODE:
      case GL_TEXTURE_COMPARE_FUNC:
         return !IS_GL_11(state);

#if GL_BRCM_texture_unnormalised_coords
      case GL_TEXTURE_UNNORMALISED_COORDS_BRCM:
         return !IS_GL_11(state) && (khrn_get_v3d_version() >= V3D_MAKE_VER(3,3));
#endif

      default:
         return false;
   }
}

bool glxx_is_int_texparam(GLXX_SERVER_STATE_T *state, GLenum target, GLenum pname)
{
   if (glxx_is_int_sampler_texparam(state, pname))
      return true;

   switch (pname)
   {
      case GL_GENERATE_MIPMAP:
         return IS_GL_11(state) && target != GL_TEXTURE_EXTERNAL_OES;

#if GL_OES_EGL_image_external
      case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
         return true;
#endif
      case GL_TEXTURE_SWIZZLE_R:
      case GL_TEXTURE_SWIZZLE_G:
      case GL_TEXTURE_SWIZZLE_B:
      case GL_TEXTURE_SWIZZLE_A:
      case GL_TEXTURE_BASE_LEVEL:
      case GL_TEXTURE_MAX_LEVEL:
#if GL_BRCM_texture_mirror_swap
      case GL_TEXTURE_FLIP_X:
      case GL_TEXTURE_FLIP_Y:
      case GL_TEXTURE_SWAP_ST:
#endif // GL_BRCM_texture_mirror_swap
      case GL_TEXTURE_IMMUTABLE_FORMAT:
      case GL_TEXTURE_IMMUTABLE_LEVELS:
         return !IS_GL_11(state);
      case GL_DEPTH_STENCIL_TEXTURE_MODE:
         return KHRN_GLES31_DRIVER ? !IS_GL_11(state) : false;
      default:
         return false;
   }
}

uint32_t glxx_get_texparameter_sampler_internal(GLXX_SERVER_STATE_T *state, GLXX_TEXTURE_SAMPLER_STATE_T *so, GLenum pname, GLint *params)
{
   uint32_t result;

   assert(glxx_is_int_sampler_texparam(state, pname));

   switch(pname)
   {
      case GL_TEXTURE_MIN_FILTER:
         params[0] = so->filter.min;
         result = 1;
         break;
      case GL_TEXTURE_MAG_FILTER:
         params[0] = so->filter.mag;
         result = 1;
         break;
      case GL_TEXTURE_WRAP_S:
         params[0] = so->wrap.s;
         result = 1;
         break;
      case GL_TEXTURE_WRAP_T:
         params[0] = so->wrap.t;
         result = 1;
         break;
      case GL_TEXTURE_WRAP_R:
         params[0] = so->wrap.r;
         result = 1;
         break;
      case GL_TEXTURE_COMPARE_MODE:
         params[0] = so->compare_mode;
         result = 1;
         break;
      case GL_TEXTURE_COMPARE_FUNC:
         params[0] = so->compare_func;
         result = 1;
         break;
#if GL_BRCM_texture_unnormalised_coords
      case GL_TEXTURE_UNNORMALISED_COORDS_BRCM:
         params[0] = so->unnormalised_coords ? GL_TRUE : GL_FALSE;
         result = 1;
         break;
#endif
      default:
         UNREACHABLE();
   }
   return result;
}

uint32_t glxx_get_texparameter_internal(GLXX_SERVER_STATE_T *state, GLenum target, GLenum pname, GLint *params)
{
   int result = 0;
   GLXX_TEXTURE_T *texture;

   assert(glxx_is_int_texparam(state, target, pname));

   texture = glxx_server_state_get_texture(state, target, false);

   if (texture) {
      switch (pname) {
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_WRAP_R:
      case GL_TEXTURE_COMPARE_MODE:
      case GL_TEXTURE_COMPARE_FUNC:
#if GL_BRCM_texture_unnormalised_coords
      case GL_TEXTURE_UNNORMALISED_COORDS_BRCM:
#endif
         result = glxx_get_texparameter_sampler_internal(state, &texture->sampler, pname, params);
         break;
      case GL_GENERATE_MIPMAP:
         params[0] = texture->generate_mipmap;
         result = 1;
         break;
#if GL_OES_EGL_image_external
      case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
         params[0] = 1;
         result = 1;
         break;
#endif
      case GL_TEXTURE_SWIZZLE_R:
      case GL_TEXTURE_SWIZZLE_G:
      case GL_TEXTURE_SWIZZLE_B:
      case GL_TEXTURE_SWIZZLE_A:
         switch (texture->swizzle[pname - GL_TEXTURE_SWIZZLE_R]) {
            case 0: params[0] = GL_ZERO; break;
            case 1: params[0] = GL_ONE; break;
            case 2: params[0] = GL_RED; break;
            case 3: params[0] = GL_GREEN; break;
            case 4: params[0] = GL_BLUE; break;
            case 5: params[0] = GL_ALPHA; break;
            default:
               UNREACHABLE();
         }
         result = 1;
         break;
      case GL_TEXTURE_BASE_LEVEL:
         params[0] = texture->base_level;
         result = 1;
         break;
      case GL_TEXTURE_MAX_LEVEL:
         params[0] = texture->max_level;
         result = 1;
         break;
      case GL_TEXTURE_FLIP_X:
         params[0] = texture->flip_x;
         result = 1;
         break;
      case GL_TEXTURE_FLIP_Y:
         params[0] = texture->flip_y;
         result = 1;
         break;
      case GL_TEXTURE_SWAP_ST:
         params[0] = texture->swap_st;
         result = 1;
         break;
      case GL_TEXTURE_IMMUTABLE_FORMAT:
         params[0] = (texture->immutable_format == GFX_LFMT_NONE) ? GL_FALSE : GL_TRUE;
         result = 1;
         break;
      case GL_TEXTURE_IMMUTABLE_LEVELS:
         params[0] = texture->immutable_levels;
         result = 1;
         break;
      case GL_DEPTH_STENCIL_TEXTURE_MODE:
         params[0] = texture->ds_texture_mode;
         result = 1;
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
         break;
      }
   }

   return result;
}

uint32_t glxx_get_texparameterf_sampler_internal(GLXX_SERVER_STATE_T *state, GLXX_TEXTURE_SAMPLER_STATE_T *sampler, GLenum pname, GLfloat *params)
{
   uint32_t result;

   switch (pname)
   {
#if GL_EXT_texture_filter_anisotropic
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      if (!IS_GL_11(state)) {
         params[0] = sampler->anisotropy;
         result = 1;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
      }
      break;
#endif
   case GL_TEXTURE_MIN_LOD:
      if (!IS_GL_11(state)) {
         params[0] = sampler->min_lod;
         result = 1;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
      }
      break;
   case GL_TEXTURE_MAX_LOD:
      if (!IS_GL_11(state)) {
         params[0] = sampler->max_lod;
         result = 1;
      } else {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
      }
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      result = 0;
   }

   return result;
}

static uint32_t glxx_get_texparameterf_internal(GLXX_SERVER_STATE_T *state, GLenum target, GLenum pname, GLfloat *params)
{
   int result = 0;
   GLXX_TEXTURE_T *texture;

   texture = glxx_server_state_get_texture(state, target, false);

   if (texture) {
      switch (pname) {
#if GL_EXT_texture_filter_anisotropic
      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
#endif
      case GL_TEXTURE_MIN_LOD:
      case GL_TEXTURE_MAX_LOD:
         result = glxx_get_texparameterf_sampler_internal(state, &texture->sampler, pname, params);
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         result = 0;
         break;
      }
   }

   return result;
}

GL_API void GL_APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
   uint32_t count;
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   if (!state)
      return;

   if (glxx_is_float_texparam(pname)) {
      count = glxx_get_texparameterf_internal(state, target, pname, params);
   } else if (glxx_is_int_texparam(state, target, pname)) {
      GLint temp[4];
      count = glxx_get_texparameter_internal(state, target, pname, temp);

      if (count) {
         unsigned int i;
         assert(count == 1 || count == 4);
         for (i = 0; i < count; i++)
            params[i] = (GLfloat)temp[i];
      }
   } else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

   GLXX_UNLOCK_SERVER_STATE();

}

GL_API void GL_APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
   uint32_t count;
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   if (!state)
      return;

   if (glxx_is_float_texparam(pname)) {
      GLfloat temp[4];
      uint32_t i;
      count = glxx_get_texparameterf_internal(state, target, pname, temp);
      for (i = 0; i < count; i++)
         params[i] = float_to_int(temp[i]);
   } else if (glxx_is_int_texparam(state, target, pname)) {
      count = glxx_get_texparameter_internal(state, target, pname, params);
   } else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   }

   GLXX_UNLOCK_SERVER_STATE();
}

static int get_integer_vertex_attrib_internal(GLXX_SERVER_STATE_T *state,
                                              GLint index, GLenum pname)
{
   GLXX_ATTRIB_CONFIG_T *attr;
   GLXX_VBO_BINDING_T *vbo;
   assert(index < GLXX_CONFIG_MAX_VERTEX_ATTRIBS);

   attr = &state->vao.bound->attrib_config[index];
   vbo = &state->vao.bound->vbos[attr->vbo_index];
   switch (pname) {
   case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
      return attr->enabled ? GL_TRUE : GL_FALSE;
   case GL_VERTEX_ATTRIB_ARRAY_SIZE:
      return attr->size;
   case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
      return vbo->original_stride;
   case GL_VERTEX_ATTRIB_ARRAY_TYPE:
      return attr->type;
   case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
      return attr->norm ? GL_TRUE : GL_FALSE;
   case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
      if (vbo->buffer == NULL) return 0;
      else return vbo->buffer->name;
   case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
      return vbo->divisor;
   case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
      return attr->is_int;
   case GL_VERTEX_ATTRIB_BINDING:
      return attr->vbo_index;
   case GL_VERTEX_ATTRIB_RELATIVE_OFFSET:
      return attr->relative_offset;
   default:
      UNREACHABLE(); return 0;
   }
}

static bool is_integer_attrib_param(GLenum pname) {
   switch(pname) {
      case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
      case GL_VERTEX_ATTRIB_ARRAY_SIZE:
      case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
      case GL_VERTEX_ATTRIB_ARRAY_TYPE:
      case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
      case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
      case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
      case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
         return true;
      case GL_VERTEX_ATTRIB_BINDING:
      case GL_VERTEX_ATTRIB_RELATIVE_OFFSET:
         return KHRN_GLES31_DRIVER ? true : false;
      default:
         return false;
   }
}

GL_API void GL_APIENTRY glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   if (!state) return;

   if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (is_integer_attrib_param(pname)) {
      params[0] = (GLfloat)get_integer_vertex_attrib_internal(state, index, pname);
   } else {
      switch (pname) {
         case GL_CURRENT_VERTEX_ATTRIB:
            params[0] = state->generic_attrib[index].value[0];
            params[1] = state->generic_attrib[index].value[1];
            params[2] = state->generic_attrib[index].value[2];
            params[3] = state->generic_attrib[index].value[3];
            break;
         default:
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
            break;
      }
   }

end:
   GL20_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   if (!state) return;

   if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (is_integer_attrib_param(pname)) {
      params[0] = get_integer_vertex_attrib_internal(state, index, pname);
   } else {
      switch(pname) {
         case GL_CURRENT_VERTEX_ATTRIB:
            params[0] = (GLint)state->generic_attrib[index].value[0];
            params[1] = (GLint)state->generic_attrib[index].value[1];
            params[2] = (GLint)state->generic_attrib[index].value[2];
            params[3] = (GLint)state->generic_attrib[index].value[3];
            break;
         default:
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
            break;
      }
   }

end:
   GL20_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glGetVertexAttribIiv(GLuint index, GLenum pname, GLint* params)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   if (!state) return;

   if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (is_integer_attrib_param(pname)) {
      params[0] = get_integer_vertex_attrib_internal(state, index, pname);
   } else {
      switch (pname)
      {
         case GL_CURRENT_VERTEX_ATTRIB:
            params[0] = state->generic_attrib[index].value_int[0];
            params[1] = state->generic_attrib[index].value_int[1];
            params[2] = state->generic_attrib[index].value_int[2];
            params[3] = state->generic_attrib[index].value_int[3];
            break;

         default:
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
            break;
      }
   }

end:
   GL30_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   if (!state) return;

   if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (is_integer_attrib_param(pname)) {
      params[0] = get_integer_vertex_attrib_internal(state, index, pname);
   } else {
      switch (pname) {
      case GL_CURRENT_VERTEX_ATTRIB:
         params[0] = state->generic_attrib[index].value_int[0];
         params[1] = state->generic_attrib[index].value_int[1];
         params[2] = state->generic_attrib[index].value_int[2];
         params[3] = state->generic_attrib[index].value_int[3];
         break;

      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         break;
      }
   }

end:
   GL30_UNLOCK_SERVER_STATE();
}

//jeremyt 30/3/2010 get_boolean_internal moved back from server_cr.c
/*
   int glxx_get_boolean_internal(GLXX_SERVER_STATE_T *state, GLenum pname, GLboolean *params)

   Returns either a boolean state variable or the result of an isEnabled.  A utility
   function shared by GetBooleanv() GetIntegerv() GetFloatv() GetFixedv() IsEnabled(X)

   Implementation notes:

   stores the value(s) of the given state variable in the memory given by the pointer "params", and returns
   number of such values.  An UNREACHABLE() macro guards against preconditon on pname being unmet by the caller.

   Preconditions:

   Valid EGL server state exists
   EGL server state has a current OpenGL ES 1.1 or 2.0 context
   pname is one of the native boolean state variables listed below or an IsEnabled thing
   params points to sufficient GLbooleans for the pname (i.e. 1 value except where noted below)

   Postconditions:

   -

   Native Boolean State Variables

   COLOR WRITEMASK GetBooleanv (4 values returned)
   DEPTH WRITEMASK GetBooleanv
   SAMPLE COVERAGE INVERT GetBooleanv

   //gl 1.1 specific
   LIGHT MODEL TWO SIDE GetBooleanv

   //gl 2.0 specific
   SHADER COMPILER False GetBooleanv

   plus these

   CULL FACE IsEnabled
   POLYGON OFFSET FILL IsEnabled
   SAMPLE ALPHA TO COVERAGE IsEnabled
   SAMPLE COVERAGE IsEnabled
   SCISSOR TEST IsEnabled
   STENCIL TEST IsEnabled
   DEPTH TEST IsEnabled
   BLEND IsEnabled
   DITHER IsEnabled

   //gl 1.1 specific
   COLOR LOGIC OP IsEnabled
   NORMALIZE IsEnabled
   RESCALE NORMAL IsEnabled
   CLIP PLANE0 IsEnabled
   FOG IsEnabled
   LIGHTING IsEnabled
   COLOR MATERIAL IsEnabled
   LIGHT0 IsEnabled
   LIGHT1 IsEnabled
   LIGHT2 IsEnabled
   LIGHT3 IsEnabled
   LIGHT4 IsEnabled
   LIGHT5 IsEnabled
   LIGHT6 IsEnabled
   LIGHT7 IsEnabled
   POINT SMOOTH IsEnabled
   POINT SPRITE OES IsEnabled
   LINE SMOOTH IsEnabled
   TEXTURE 2D IsEnabled
   ALPHA TEST IsEnabled
   MULTISAMPLE IsEnabled
   SAMPLE ALPHA TO ONE IsEnabled

   ??GL_MATRIX_PALETTE_OES IsEnabled (GL_OES_matrix_palette)

   VERTEX ARRAY IsEnabled
   NORMAL ARRAY IsEnabled
   COLOR ARRAY IsEnabled
   TEXTURE COORD ARRAY IsEnabled
   POINT SIZE ARRAY OES IsEnabled
   GL_MATRIX_INDEX_ARRAY_OES IsEnabled (GL_OES_matrix_palette)
   GL_WEIGHT_ARRAY_OES IsEnabled (GL_OES_matrix_palette)

*/

int glxx_get_boolean_internal(GLXX_SERVER_STATE_T *state, GLenum pname, GLboolean *params)
{
   int result = 0;
   GLXX_VAO_T *vao;
   vao = state->vao.bound;
   assert(vao);

   assert(glxx_is_boolean(state, pname));

   assert(GL11_CONFIG_MAX_PLANES == 1);
   assert(GL11_CONFIG_MAX_LIGHTS == 8);

   switch (pname) {
   case GL_LIGHT_MODEL_TWO_SIDE:
      params[0] = !!(state->gl11.statebits.vertex & GL11_TWOSIDE);
      result = 1;
      break;
   case GL_SAMPLE_COVERAGE_INVERT:
      params[0] = state->sample_coverage.invert;
      result = 1;
      break;
   case GL_COLOR_WRITEMASK:
      params[0] = state->color_write.r;
      params[1] = state->color_write.g;
      params[2] = state->color_write.b;
      params[3] = state->color_write.a;
      result = 4;
      break;
   case GL_DEPTH_WRITEMASK:
      params[0] = state->depth_mask;
      result = 1;
      break;
   case GL_NORMALIZE:
      params[0] = !(state->gl11.statebits.vertex & GL11_NO_NORMALIZE);
      result = 1;
      break;
   case GL_RESCALE_NORMAL:
      params[0] = !(state->gl11.statebits.v_enable & GL11_NO_NORMALIZE);
      result = 1;
      break;
   case GL_CLIP_PLANE0:
      params[0] = !!(state->gl11.statebits.f_enable & GL11_UCLIP_M);
      result = 1;
      break;
   case GL_FOG:
      params[0] = !!(state->gl11.statebits.f_enable & GL11_FOG_M);
      result = 1;
      break;
   case GL_LIGHTING:
      params[0] = !!(state->gl11.statebits.v_enable2 & GL11_LIGHTING_M);
      result = 1;
      break;
   case GL_COLOR_MATERIAL:
      params[0] = !!(state->gl11.statebits.vertex & GL11_COLORMAT);
      result = 1;
      break;
   case GL_LIGHT0:
   case GL_LIGHT1:
   case GL_LIGHT2:
   case GL_LIGHT3:
   case GL_LIGHT4:
   case GL_LIGHT5:
   case GL_LIGHT6:
   case GL_LIGHT7:
      params[0] = !!(state->gl11.statebits.v_enable & (GL11_LIGHT_M << (pname - GL_LIGHT0)));
      result = 1;
      break;
   case GL_POINT_SMOOTH:
      params[0] = !!(state->gl11.statebits.fragment & GL11_POINTSMOOTH);
      result = 1;
      break;
   case GL_POINT_SPRITE_OES:
      params[0] = !!(state->gl11.point_sprite);
      result = 1;
      break;
   case GL_LINE_SMOOTH:
      params[0] = !!(state->gl11.statebits.fragment & GL11_LINESMOOTH);
      result = 1;
      break;
   case GL_SHADER_COMPILER:
      params[0] = GL_TRUE;
      result = 1;
      break;
   case GL_CULL_FACE:
      params[0] = state->caps.cull_face;
      result = 1;
      break;
   case GL_POLYGON_OFFSET_FILL:
      params[0] = state->caps.polygon_offset_fill;
      result = 1;
      break;
   case GL_MULTISAMPLE:
      params[0] = state->caps.multisample;
      result = 1;
      break;
   case GL_SAMPLE_ALPHA_TO_COVERAGE:
      params[0] = !!(state->statebits.backend & GLXX_SAMPLE_ALPHA);
      result = 1;
      break;
   case GL_SAMPLE_ALPHA_TO_ONE:
      params[0] = !!(state->gl11.statebits.fragment & GL11_SAMPLE_ONE);
      result = 1;
      break;
   case GL_SAMPLE_COVERAGE:
      params[0] = state->sample_coverage.enable;
      result = 1;
      break;
   case GL_SAMPLE_MASK:
      params[0] = state->sample_mask.enable;
      result = 1;
      break;
   case GL_TEXTURE_2D:
      assert(state->active_texture - GL_TEXTURE0 < GL11_CONFIG_MAX_TEXTURE_UNITS);
      params[0] = state->gl11.texunits[state->active_texture - GL_TEXTURE0].target_enabled_2D;
      result = 1;
      break;
   case GL_TEXTURE_EXTERNAL_OES:
      assert(state->active_texture - GL_TEXTURE0 < GL11_CONFIG_MAX_TEXTURE_UNITS);
      params[0] = state->gl11.texunits[state->active_texture - GL_TEXTURE0].target_enabled_EXTERNAL_OES;
      result = 1;
      break;
   case GL_SCISSOR_TEST:
      params[0] = state->caps.scissor_test;
      result = 1;
      break;
   case GL_ALPHA_TEST:
      params[0] = !!(state->gl11.statebits.f_enable & GL11_AFUNC_M);
      result = 1;
      break;
   case GL_STENCIL_TEST:
      params[0] = state->caps.stencil_test;
      result = 1;
      break;
   case GL_DEPTH_TEST:
      params[0] = state->caps.depth_test;
      result = 1;
      break;
   case GL_BLEND:
      params[0] = !!(state->blend.enable);
      result = 1;
      break;
   case GL_DITHER:
      params[0] = state->caps.dither;
      result = 1;
      break;
   case GL_COLOR_LOGIC_OP:
      params[0] = !!(state->gl11.statebits.f_enable & GL11_LOGIC_M);
      result = 1;
      break;
#if GL_OES_matrix_palette
   case GL_MATRIX_PALETTE_OES:
      params[0] = !!(state->gl11.statebits.v_enable & GL11_MPAL_M);
      result = 1;
      break;
#endif
   case GL_PRIMITIVE_RESTART_FIXED_INDEX:
      params[0] = state->caps.primitive_restart;
      result = 1;
      break;
   case GL_TRANSFORM_FEEDBACK_ACTIVE:
   case GL_TRANSFORM_FEEDBACK_PAUSED:
      result = glxx_tf_get_boolean(state, pname, params);
      break;
   case GL_RASTERIZER_DISCARD:
      params[0] = state->caps.rasterizer_discard;
      result = 1;
      break;

   case GL_VERTEX_ARRAY:
      params[0] = vao->attrib_config[GL11_IX_VERTEX].enabled;
      result = 1;
      break;
   case GL_NORMAL_ARRAY:
      params[0] = vao->attrib_config[GL11_IX_NORMAL].enabled;
      result = 1;
      break;
   case GL_COLOR_ARRAY:
      params[0] = vao->attrib_config[GL11_IX_COLOR].enabled;
      result = 1;
      break;
   case GL_TEXTURE_COORD_ARRAY:
      params[0] = vao->attrib_config[state->gl11.client_active_texture - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].enabled;
      result = 1;
      break;
   case GL_POINT_SIZE_ARRAY_OES:
      params[0] = vao->attrib_config[GL11_IX_POINT_SIZE].enabled;
      result = 1;
      break;
   case GL_DEBUG_OUTPUT_KHR:
      params[0] = state->khr_debug.debug_output;
      result = 1;
      break;
   case GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR:
      params[0] = state->khr_debug.debug_output_synchronous;
      result = 1;
      break;
#if GL_OES_matrix_palette
   case GL_MATRIX_INDEX_ARRAY_OES:
      params[0] = vao->attrib_config[GL11_IX_MATRIX_INDEX].enabled;
      result = 1;
      break;
   case GL_WEIGHT_ARRAY_OES:
      params[0] = vao->attrib_config[GL11_IX_MATRIX_WEIGHT].enabled;
      result = 1;
      break;
#endif
#if GL_EXT_robustness
   case GL_CONTEXT_ROBUST_ACCESS_EXT:
      params[0] = egl_context_gl_robustness(state->context);
      result = 1;
      break;
#endif
   default:
      UNREACHABLE();
      result = 0;
      break;
   }

   return result;
}

//jeremyt 30/3/2010 get_float_internal moved back from server_cr.c
/*
   int glxx_get_float_internal(GLXX_SERVER_STATE_T *state, GLenum pname, float *params)

   Returns a float state variable.  A utility
   function shared by GetBooleanv() GetIntegerv() GetFloatv() GetFixedv()

   Implementation notes:

   stores the value(s) of the given state variable in the memory given by the pointer "params", and returns
   number of such values.  An UNREACHABLE() macro guards against preconditon on pname being unmet by the caller.

   ALPHA TEST REF and DEPTH CLEAR VALUE are stored and set as floats, but retrieved with GetIntegerv.  We treat
   them as floats.  This is a possible deviation from the specification.

   Non-overlapping precondition on gl11_matrix_load is satisfied as params can never point to part of the GL state.

   Preconditions:

   Valid EGL server state exists
   EGL server state has a current OpenGL ES 1.1 or 2.0 context
   pname is one of the native floating-point state variables listed below
   params points to sufficient float for the pname (i.e. 1 value except where noted below)

   Postconditions:

   -

   Native Floating-point State Variables

   LINE WIDTH GetFloatv
   POLYGON OFFSET FACTOR GetFloatv
   POLYGON OFFSET UNITS GetFloatv
   SAMPLE COVERAGE VALUE GetFloatv
   COLOR CLEAR VALUE GetFloatv (4 values returned)
   DEPTH CLEAR VALUE GetIntegerv                // this is set as a float, so moving to GetFloatv
   DEPTH RANGE GetFloatv (2 values returned)

   //gl 1.1 specific
   MODELVIEW MATRIX GetFloatv (16 values returned)
   PROJECTION MATRIX GetFloatv  (16 values returned)
   TEXTURE MATRIX GetFloatv (16 values returned)

   FOG COLOR GetFloatv (4 values returned)
   FOG DENSITY GetFloatv
   FOG START GetFloatv
   FOG END GetFloatv
   LIGHT MODEL AMBIENT GetFloatv (4 values returned)
   POINT SIZE MIN GetFloatv
   POINT SIZE MAX GetFloatv
   POINT FADE THRESHOLD SIZE GetFloatv
   POINT DISTANCE ATTENUATION GetFloatv (3 values returned)

   ALPHA TEST REF GetIntegerv                  // this is set as a float, so moving to GetFloatv

   ALIASED POINT SIZE RANGE GetFloatv (2 values returned)
   SMOOTH POINT SIZE RANGE GetFloatv (2 values returned)
   ALIASED LINE WIDTH RANGE GetFloatv (2 values returned)
   SMOOTH LINE WIDTH RANGE GetFloatv (2 values returned)

   //gl 2.0 specific
   BLEND COLOR 0,0,0,0 GetFloatv

   CURRENT_COLOR (4 values returned)
   CURRENT_TEXTURE_COORDS (4 values returned)
   CURRENT_NORMAL (3 values returned)
   POINT_SIZE
*/
static int glxx_get_float_internal(GLXX_SERVER_STATE_T *state, GLenum pname, GLfloat *params)
{
   int result = 0;

   assert(state);

   // This will return false if wrong GL version
   assert(glxx_is_float(state, pname));

   switch (pname) {
   case GL_MODELVIEW_MATRIX:
      gl11_matrix_load(params, state->gl11.current_modelview);
      result = 16;
      break;
   case GL_PROJECTION_MATRIX:
      gl11_matrix_load(params, state->gl11.current_projection);
      result = 16;
      break;
   case GL_TEXTURE_MATRIX:
      assert(state->active_texture - GL_TEXTURE0 < GL11_CONFIG_MAX_TEXTURE_UNITS);

      gl11_matrix_load(params, state->gl11.texunits[state->active_texture - GL_TEXTURE0].current_matrix);
      result = 16;
      break;
   case GL_DEPTH_RANGE:
      params[0] = state->viewport.vp_near;
      params[1] = state->viewport.vp_far;
      result = 2;
      break;
   case GL_FOG_COLOR:
   {
      int i;
      for (i = 0; i < 4; i++)
         params[i] = state->gl11.fog.color[i];
      result = 4;
      break;
   }
   case GL_FOG_DENSITY:
      params[0] = state->gl11.fog.density;
      result = 1;
      break;
   case GL_FOG_START:
      params[0] = state->gl11.fog.start;
      result = 1;
      break;
   case GL_FOG_END:
      params[0] = state->gl11.fog.end;
      result = 1;
      break;
   case GL_LIGHT_MODEL_AMBIENT:
      {
         int i;
         for (i = 0; i < 4; i++)
            params[i] = state->gl11.lightmodel.ambient[i];
         result = 4;
      }
      break;
   case GL_POINT_SIZE_MIN:
      params[0] = state->gl11.point_params.size_min;
      result = 1;
      break;
   case GL_POINT_SIZE_MAX:
      params[0] = state->gl11.point_params.size_max;
      result = 1;
      break;
   case GL_POINT_FADE_THRESHOLD_SIZE:
      params[0] = state->gl11.point_params.fade_threshold;
      result = 1;
      break;
   case GL_POINT_DISTANCE_ATTENUATION:
   {
      int i;
      for (i = 0; i < 3; i++)
         params[i] = state->gl11.point_params.distance_attenuation[i];
      result = 3;
      break;
   }
   case GL_LINE_WIDTH:
      params[0] = state->line_width;
      result = 1;
      break;
   case GL_POLYGON_OFFSET_FACTOR:
      params[0] = state->polygon_offset.factor;
      result = 1;
      break;
   case GL_POLYGON_OFFSET_UNITS:
      params[0] = state->polygon_offset.units;
      result = 1;
      break;
   case GL_SAMPLE_COVERAGE_VALUE:
      params[0] = state->sample_coverage.value;
      result = 1;
      break;
   case GL_ALPHA_TEST_REF:
      params[0] = state->gl11.alpha_func.ref;
      result = 1;
      break;
   case GL_COLOR_CLEAR_VALUE:
   {
      int i;
      for (i = 0; i < 4; i++)
         params[i] = state->clear.color_value[i];
      result = 4;
      break;
   }
   case GL_DEPTH_CLEAR_VALUE:
      params[0] = state->clear.depth_value;
      result = 1;
      break;
   case GL_BLEND_COLOR:
   {
      int i;
      for (i = 0; i < 4; i++)
         params[i] = state->blend_color[i];
      result = 4;
      break;
   }
   case GL_ALIASED_POINT_SIZE_RANGE:
      if (IS_GL_11(state)) {
         params[0] = GL11_CONFIG_MIN_ALIASED_POINT_SIZE;
         params[1] = GL11_CONFIG_MAX_ALIASED_POINT_SIZE;
         result = 2;
      } else {
         params[0] = GL20_CONFIG_MIN_ALIASED_POINT_SIZE;
         params[1] = GL20_CONFIG_MAX_ALIASED_POINT_SIZE;
         result = 2;
      }
      break;
   case GL_SMOOTH_POINT_SIZE_RANGE:
      params[0] = GL11_CONFIG_MIN_SMOOTH_POINT_SIZE;
      params[1] = GL11_CONFIG_MAX_SMOOTH_POINT_SIZE;
      result = 2;
      break;
   case GL_ALIASED_LINE_WIDTH_RANGE:
      if (IS_GL_11(state)) {
         params[0] = GL11_CONFIG_MIN_ALIASED_LINE_WIDTH;
         params[1] = GL11_CONFIG_MAX_ALIASED_LINE_WIDTH;
         result = 2;
      } else {
         params[0] = GL20_CONFIG_MIN_ALIASED_LINE_WIDTH;
         params[1] = GL20_CONFIG_MAX_ALIASED_LINE_WIDTH;
         result = 2;
      }
      break;
   case GL_SMOOTH_LINE_WIDTH_RANGE:
      params[0] = GL11_CONFIG_MIN_SMOOTH_LINE_WIDTH;
      params[1] = GL11_CONFIG_MAX_SMOOTH_LINE_WIDTH;
      result = 2;
      break;

   case GL_CURRENT_TEXTURE_COORDS:
   {
      /*
         apparently we need the current texture coordinates for the _server_ active texture unit
      */
      int i;
      for (i = 0; i < 4; i++)
         params[i] = state->generic_attrib[state->active_texture - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].value[i];
      result = 4;
      break;
   }
   case GL_CURRENT_COLOR:
   {
      int i;
      for (i = 0; i < 4; i++)
         params[i] = state->generic_attrib[GL11_IX_COLOR].value[i];
      result = 4;
      break;
   }
   case GL_CURRENT_NORMAL:
   {
      int i;
      for (i = 0; i < 3; i++)
         params[i] = state->generic_attrib[GL11_IX_NORMAL].value[i];
      result = 3;
      break;
   }
   case GL_POINT_SIZE:
   {
      params[0] = state->generic_attrib[GL11_IX_POINT_SIZE].value[0];
      result = 1;
      break;
   }
#if GL_EXT_texture_filter_anisotropic
   case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
      params[0] = 16.0f;
      result = 1;
      break;
#endif
   case GL_MAX_TEXTURE_LOD_BIAS:
      params[0] = GLXX_CONFIG_MAX_TEXTURE_LOD_BIAS;
      result = 1;
      break;
   default:
      UNREACHABLE();
      result = 0;
      break;
   }

   return result;
}

static int get_texture_binding(const GLXX_SERVER_STATE_T *state, GLenum binding)
{
   GLXX_TEXTURE_T *texture = NULL;
   enum glxx_tex_target target;

   switch (binding)
   {
      case GL_TEXTURE_BINDING_2D:
         target = GL_TEXTURE_2D;
         break;
      case GL_TEXTURE_BINDING_EXTERNAL_OES:
         target = GL_TEXTURE_EXTERNAL_OES;
         break;
      case GL_TEXTURE_BINDING_CUBE_MAP:
         target = GL_TEXTURE_CUBE_MAP;
         break;
      case GL_TEXTURE_BINDING_3D:
         target = GL_TEXTURE_3D;
         break;
      case GL_TEXTURE_BINDING_2D_ARRAY:
         target = GL_TEXTURE_2D_ARRAY;
         break;
      case GL_TEXTURE_BINDING_2D_MULTISAMPLE:
         target = GL_TEXTURE_2D_MULTISAMPLE;
         break;
      case GL_TEXTURE_BINDING_1D_BRCM:
         target = GL_TEXTURE_1D_BRCM;
         break;
      case GL_TEXTURE_BINDING_1D_ARRAY_BRCM:
         target = GL_TEXTURE_1D_ARRAY_BRCM;
         break;
      default:
         UNREACHABLE();
         return 0;
   }

   texture = glxx_server_get_active_texture(state, target);
   assert(texture);
   return texture->name;
}

static int gl11_binding_to_attr(GLenum binding) {
   switch (binding) {
   case GL_VERTEX_ARRAY_BUFFER_BINDING:            return GL11_IX_VERTEX;
   case GL_NORMAL_ARRAY_BUFFER_BINDING:            return GL11_IX_NORMAL;
   case GL_COLOR_ARRAY_BUFFER_BINDING:             return GL11_IX_COLOR;
   case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:     return GL11_IX_CLIENT_ACTIVE_TEXTURE;
   case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:    return GL11_IX_POINT_SIZE;
   case GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES:  return GL11_IX_MATRIX_INDEX;
   case GL_WEIGHT_ARRAY_BUFFER_BINDING_OES:        return GL11_IX_MATRIX_WEIGHT;
   default:                                        UNREACHABLE(); return -1;
   }
}

typedef uint32_t (*comp_size_fct_t)(GFX_LFMT_T lfmt);
uint32_t component_bits(GLXX_FRAMEBUFFER_T *fbo, GLenum component)
{
   glxx_attachment_point_t att_point;
   comp_size_fct_t fct = NULL;
   uint32_t bits = 0;

   switch(component)
   {
      case GL_RED_BITS:
         att_point = GL_COLOR_ATTACHMENT0;
         fct = gfx_lfmt_red_bits;
         break;
      case GL_GREEN_BITS:
         att_point = GL_COLOR_ATTACHMENT0;
         fct = gfx_lfmt_green_bits;
         break;
      case GL_BLUE_BITS:
         att_point = GL_COLOR_ATTACHMENT0;
         fct = gfx_lfmt_blue_bits;
         break;
      case GL_ALPHA_BITS:
         att_point = GL_COLOR_ATTACHMENT0;
         fct = gfx_lfmt_alpha_bits;
         break;
      case GL_DEPTH_BITS:
         att_point = GL_DEPTH_ATTACHMENT;
         fct = gfx_lfmt_depth_bits;
         break;
      case GL_STENCIL_BITS:
         att_point = GL_STENCIL_ATTACHMENT;
         fct = gfx_lfmt_stencil_bits;
         break;
      default:
         unreachable();
   }

   const GLXX_ATTACHMENT_T *att = glxx_fb_get_attachment(fbo, att_point);
   GFX_LFMT_T api_fmt = glxx_attachment_get_api_fmt(att);
   if (api_fmt != GFX_LFMT_NONE)
      bits = fct(api_fmt);

   return bits;
}

static int glxx_get_unsigned_integer_internal(GLXX_SERVER_STATE_T *state, GLenum pname, unsigned int *params)
{
   int result = 0;
   GLXX_VAO_T *vao;
   vao = state->vao.bound;
   assert(vao);

   // This should have been checked by the caller
   assert(glxx_is_unsigned_integer(state, pname) && !glxx_is_signed_integer(state, pname));

   switch (pname) {
   case GL_ARRAY_BUFFER_BINDING:
      if (state->bound_buffer[GLXX_BUFTGT_ARRAY].obj != NULL) {
         params[0] = state->bound_buffer[GLXX_BUFTGT_ARRAY].obj->name;
      } else
         params[0] = 0;
      result = 1;
      break;
   case GL_ELEMENT_ARRAY_BUFFER_BINDING:
      if (vao->element_array_binding.obj != NULL) {
         params[0] = vao->element_array_binding.obj->name;
      } else
         params[0] = 0;
      result = 1;
      break;
   case GL_MODELVIEW_STACK_DEPTH:
      params[0] = state->gl11.modelview.pos + 1;
      result = 1;
      break;
   case GL_PROJECTION_STACK_DEPTH:
      params[0] = state->gl11.projection.pos + 1;
      result = 1;
      break;
   case GL_TEXTURE_STACK_DEPTH:
      params[0] = state->gl11.texunits[state->active_texture - GL_TEXTURE0].stack.pos + 1;
      result = 1;
      break;
   case GL_MATRIX_MODE:
      params[0] = state->gl11.matrix_mode;
      result = 1;
      break;
   case GL_FOG_MODE:
      params[0] = untranslate_fog_mode((uint32_t) state->gl11.statebits.fragment & GL11_FOG_M);
      result = 1;
      break;
   case GL_SHADE_MODEL:
      params[0] = state->gl11.shade_model;
      result = 1;
      break;
   case GL_CULL_FACE_MODE:
      params[0] = state->cull_mode;
      result = 1;
      break;
   case GL_FRONT_FACE:
      params[0] = state->front_face;
      result = 1;
      break;
   case GL_TEXTURE_BINDING_2D:
   case GL_TEXTURE_BINDING_EXTERNAL_OES:
   case GL_TEXTURE_BINDING_CUBE_MAP:
   case GL_TEXTURE_BINDING_3D:
   case GL_TEXTURE_BINDING_2D_ARRAY:
   case GL_TEXTURE_BINDING_2D_MULTISAMPLE:
   case GL_TEXTURE_BINDING_1D_BRCM:
   case GL_TEXTURE_BINDING_1D_ARRAY_BRCM:
      params[0] = get_texture_binding(state, pname);
      result = 1;
      break;
   case GL_ACTIVE_TEXTURE:
      params[0] = state->active_texture;
      result = 1;
      break;
   case GL_ALPHA_TEST_FUNC:
      params[0] = untranslate_alpha_func((uint32_t) state->gl11.statebits.fragment & GL11_AFUNC_M);
      result = 1;
      break;
   case GL_STENCIL_WRITEMASK:
      params[0] = state->stencil_mask.front;
      result = 1;
      break;
   case GL_STENCIL_BACK_WRITEMASK:
      assert(!IS_GL_11(state));
      params[0] = state->stencil_mask.back;
      result = 1;
      break;
   case GL_STENCIL_FUNC:
      params[0] = state->stencil_func.front.func;
      result = 1;
      break;
   case GL_STENCIL_VALUE_MASK:
      params[0] = state->stencil_func.front.mask;
      result = 1;
      break;
   case GL_STENCIL_FAIL:
      params[0] = state->stencil_op.front.fail;
      result = 1;
      break;
   case GL_STENCIL_PASS_DEPTH_FAIL:
      params[0] = state->stencil_op.front.zfail;
      result = 1;
      break;
   case GL_STENCIL_PASS_DEPTH_PASS:
      params[0] = state->stencil_op.front.zpass;
      result = 1;
      break;
   case GL_STENCIL_BACK_FUNC:
      params[0] = state->stencil_func.back.func;
      result = 1;
      break;
   case GL_STENCIL_BACK_VALUE_MASK:
      params[0] = state->stencil_func.back.mask;
      result = 1;
      break;

   case GL_STENCIL_BACK_FAIL:
      params[0] = state->stencil_op.back.fail;
      result = 1;
      break;
   case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
      params[0] = state->stencil_op.back.zfail;
      result = 1;
      break;
   case GL_STENCIL_BACK_PASS_DEPTH_PASS:
      params[0] = state->stencil_op.back.zpass;
      result = 1;
      break;
   case GL_DEPTH_FUNC:
      params[0] = state->depth_func;
      result = 1;
      break;
   case GL_BLEND_SRC:
      params[0] = untranslate_blend_func(state->blend.src_rgb);
      result = 1;
      break;
   case GL_BLEND_DST:
      params[0] = untranslate_blend_func(state->blend.dst_rgb);
      result = 1;
      break;
   case GL_BLEND_SRC_RGB:
      params[0] = untranslate_blend_func(state->blend.src_rgb);
      result = 1;
      break;
   case GL_BLEND_SRC_ALPHA:
      params[0] = untranslate_blend_func(state->blend.src_alpha);
      result = 1;
      break;
   case GL_BLEND_DST_RGB:
      params[0] = untranslate_blend_func(state->blend.dst_rgb);
      result = 1;
      break;
   case GL_BLEND_DST_ALPHA:
      params[0] = untranslate_blend_func(state->blend.dst_alpha);
      result = 1;
      break;
   case GL_BLEND_EQUATION_RGB:
      params[0] = untranslate_blend_equation(state->blend.color_eqn);
      result = 1;
      break;
   case GL_BLEND_EQUATION_ALPHA:
      params[0] = untranslate_blend_equation(state->blend.alpha_eqn);
      result = 1;
      break;
   case GL_LOGIC_OP_MODE:
      params[0] = untranslate_logic_op(state->gl11.statebits.fragment & GL11_LOGIC_M);
      result = 1;
      break;
   case GL_PERSPECTIVE_CORRECTION_HINT:
      params[0] = state->gl11.hints.perspective_correction;
      result = 1;
      break;
   case GL_POINT_SMOOTH_HINT:
      params[0] = state->gl11.hints.point_smooth;
      result = 1;
      break;
   case GL_LINE_SMOOTH_HINT:
      params[0] = state->gl11.hints.line_smooth;
      result = 1;
      break;
   case GL_FOG_HINT:
      params[0] = state->gl11.hints_program.fog;
      result = 1;
      break;
   case GL_GENERATE_MIPMAP_HINT:
      params[0] = state->hints.generate_mipmap;
      result = 1;
      break;
   case GL_MAX_LIGHTS:
      params[0] = GL11_CONFIG_MAX_LIGHTS;
      result = 1;
      break;
   case GL_MAX_CLIP_PLANES:
      params[0] = GL11_CONFIG_MAX_PLANES;
      result = 1;
      break;
   case GL_MAX_MODELVIEW_STACK_DEPTH:
   case GL_MAX_PROJECTION_STACK_DEPTH:
   case GL_MAX_TEXTURE_STACK_DEPTH:
      params[0] = GL11_CONFIG_MAX_STACK_DEPTH;
      result = 1;
      break;
   case GL_SUBPIXEL_BITS:
      params[0] = GLXX_CONFIG_SUBPIXEL_BITS;
      result = 1;
      break;
   case GL_MAX_TEXTURE_SIZE:
      params[0] = MAX_TEXTURE_SIZE;
      result = 1;
      break;
   case GL_MAX_3D_TEXTURE_SIZE:
      params[0] = MAX_3D_TEXTURE_SIZE;
      result = 1;
      break;
   case GL_MAX_ARRAY_TEXTURE_LAYERS:
      params[0] = MAX_ARRAY_TEXTURE_LAYERS;
      result = 1;
      break;
   case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
      params[0] = MAX_TEXTURE_SIZE;
      result = 1;
      break;
   case GL_MAX_VIEWPORT_DIMS:
      params[0] = GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE;
      params[1] = GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE;
      result = 2;
      break;
   case GL_MAX_TEXTURE_UNITS:
      params[0] = GL11_CONFIG_MAX_TEXTURE_UNITS;
      result = 1;
      break;
   case GL_SAMPLE_BUFFERS:
      params[0] = glxx_fb_get_ms_mode(state->bound_draw_framebuffer) == GLXX_NO_MS ? 0 : 1;
      result = 1;
      break;
   case GL_SAMPLES:
      params[0] = (int)(glxx_fb_get_ms_mode(state->bound_draw_framebuffer));
      result = 1;
      break;
   case GL_MAX_SAMPLES:
   case GL_MAX_COLOR_TEXTURE_SAMPLES:
   case GL_MAX_DEPTH_TEXTURE_SAMPLES:
      params[0] = GLXX_CONFIG_MAX_SAMPLES;
      result = 1;
      break;
   case GL_MAX_INTEGER_SAMPLES:
      params[0] = GLXX_CONFIG_MAX_INTEGER_SAMPLES;
      result = 1;
      break;
   case GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS:
      params[0] = GLXX_CONFIG_MAX_COMPUTE_WORK_GROUP_INVOCATIONS;
      result = 1;
      break;
   case GL_MAX_COMPUTE_SHARED_MEMORY_SIZE:
      params[0] = GLXX_CONFIG_MAX_COMPUTE_SHARED_MEM_SIZE;
      result = 1;
      break;
   case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
      params[0] = GL20_CONFIG_MAX_VARYING_SCALARS;
      result = 1;
      break;
   case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
      params[0] = GL20_CONFIG_MAX_VARYING_SCALARS;
      result = 1;
      break;
   case GL_MAX_VERTEX_UNIFORM_BLOCKS:
      params[0] = GLXX_CONFIG_MAX_VERTEX_UNIFORM_BLOCKS;
      result = 1;
      break;
   case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
      params[0] = GLXX_CONFIG_MAX_FRAGMENT_UNIFORM_BLOCKS;
      result = 1;
      break;
   case GL_MAX_COMBINED_UNIFORM_BLOCKS:
      params[0] = GLXX_CONFIG_MAX_COMBINED_UNIFORM_BLOCKS;
      result = 1;
      break;
   case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
      params[0] = GLXX_CONFIG_UNIFORM_BUFFER_OFFSET_ALIGNMENT;
      result = 1;
      break;
   case GL_COMPRESSED_TEXTURE_FORMATS:
      result = gfx_compressed_format_enumerate(params, khrn_get_lfmt_translate_exts());
      assert(result <= MAX_INTEGER_ITEM_COUNT);
      break;
   case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
      params[0] = gfx_compressed_format_enumerate(NULL, khrn_get_lfmt_translate_exts());
      result = 1;
      break;
   case GL_SHADER_BINARY_FORMATS:
      // No binary formats supported
      result = 0;
      break;
   case GL_NUM_SHADER_BINARY_FORMATS:
      params[0] = 0;
      result = 1;
      break;
   case GL_MAX_VERTEX_ATTRIBS:
      params[0] = GLXX_CONFIG_MAX_VERTEX_ATTRIBS;
      result = 1;
      break;
   case GL_MAX_VERTEX_ATTRIB_BINDINGS:
      params[0] = GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS;
      result = 1;
      break;
   case GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET:
      params[0] = GLXX_CONFIG_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET;
      result = 1;
      break;
   case GL_MAX_VERTEX_ATTRIB_STRIDE:
      params[0] = GLXX_CONFIG_MAX_VERTEX_ATTRIB_STRIDE;
      result = 1;
      break;
   case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
      params[0] = GL20_CONFIG_MAX_UNIFORM_VECTORS * 4;
      result = 1;
      break;
   case GL_MAX_VERTEX_UNIFORM_VECTORS:
      params[0] = GL20_CONFIG_MAX_UNIFORM_VECTORS;
      result = 1;
      break;
   case GL_MAX_VARYING_COMPONENTS:
      params[0] = GL20_CONFIG_MAX_VARYING_SCALARS;
      result = 1;
      break;
   case GL_MAX_VARYING_VECTORS:
      params[0] = GL20_CONFIG_MAX_VARYING_VECTORS;
      result = 1;
      break;
   case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
      params[0] = GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS;
      result = 1;
      break;
   case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
      params[0] = GL20_CONFIG_MAX_VERTEX_TEXTURE_UNITS;
      result = 1;
      break;
   case GL_MAX_TEXTURE_IMAGE_UNITS:
      params[0] = GL20_CONFIG_MAX_FRAGMENT_TEXTURE_UNITS;
      result = 1;
      break;
   case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
      assert(!IS_GL_11(state));
      params[0] = GL20_CONFIG_MAX_UNIFORM_VECTORS * 4;
      result = 1;
      break;
   case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
      assert(!IS_GL_11(state));
      params[0] = GL20_CONFIG_MAX_UNIFORM_VECTORS;
      result = 1;
      break;
   case GL_MAX_RENDERBUFFER_SIZE:
      params[0] = GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE;
      result = 1;
      break;
   case GL_MAX_DEBUG_MESSAGE_LENGTH_KHR:
      params[0] = GLXX_CONFIG_MAX_DEBUG_MESSAGE_LENGTH;
      result = 1;
      break;
   case GL_MAX_DEBUG_LOGGED_MESSAGES_KHR:
      params[0] = GLXX_CONFIG_MAX_DEBUG_LOGGED_MESSAGES;
      result = 1;
      break;
   case GL_MAX_DEBUG_GROUP_STACK_DEPTH_KHR:
      params[0] = GLXX_CONFIG_MAX_DEBUG_GROUP_STACK_DEPTH;
      result = 1;
      break;
   case GL_MAX_LABEL_LENGTH_KHR:
      params[0] = GLXX_CONFIG_MAX_LABEL_LENGTH;
      result = 1;
      break;
   case GL_DEBUG_LOGGED_MESSAGES_KHR:
      params[0] = state->khr_debug.message_log.count;
      result = 1;
      break;
   case GL_DEBUG_GROUP_STACK_DEPTH_KHR:
      params[0] = state->khr_debug.active_stack_level + 1;
      result = 1;
      break;
   case GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_KHR:
      params[0] = 0;
      if (state->khr_debug.message_log.count > 0 &&
          state->khr_debug.message_log.head->message != NULL)
      {
         // Includes NULL terminator
         params[0] = state->khr_debug.message_log.head->message_len + 1;
      }
      result = 1;
      break;

   case GL_RED_BITS:
   case GL_GREEN_BITS:
   case GL_BLUE_BITS:
   case GL_ALPHA_BITS:
   case GL_DEPTH_BITS:
   case GL_STENCIL_BITS:
      params[0] = component_bits(state->bound_draw_framebuffer, pname);
      result = 1;
      break;
   case GL_IMPLEMENTATION_COLOR_READ_TYPE:
   case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
      {
         GLenum format = GL_NONE;
         GLenum type = GL_NONE;

         const GLXX_ATTACHMENT_T *att = glxx_fb_get_read_buffer(state->bound_read_framebuffer);

         if (att)
         {
            GFX_LFMT_T api_fmt = glxx_attachment_get_api_fmt(att);

            if (api_fmt != GFX_LFMT_NONE)
            {
               GFX_LFMT_T dst_lfmt = GFX_LFMT_NONE;
               glxx_readpixels_impldefined_formats(&format, &type, &dst_lfmt, api_fmt);
            }
         }

         if (pname == GL_IMPLEMENTATION_COLOR_READ_TYPE)
            params[0] = type;
         else
            params[0] = format;
         result = 1;
         break;
      }
   case GL_MAX_ELEMENTS_VERTICES:
      params[0] = GLXX_CONFIG_RECOMMENDED_ELEMENTS_VERTICES;
      result = 1;
      break;
   case GL_MAX_ELEMENTS_INDICES:
      params[0] = GLXX_CONFIG_RECOMMENDED_ELEMENTS_INDICES;
      result = 1;
      break;
   case GL_MAX_ELEMENT_INDEX:
      params[0] = GLXX_CONFIG_MAX_ELEMENT_INDEX;
      result = 1;
      break;
   case GL_RENDERBUFFER_BINDING:
      if (state->bound_renderbuffer != NULL)
         params[0] = state->bound_renderbuffer->name;
      else
         params[0] = 0;
      result = 1;
      break;
   case GL_FRAMEBUFFER_BINDING:
   // GL_DRAW_FRAMEBUFFER_BINDING is defined to be GL_FRAMEBUFFER_BINDING
      params[0] = state->bound_draw_framebuffer->name;
      result = 1;
      break;
   case GL_MAX_DRAW_BUFFERS:
      params[0] = GLXX_MAX_RENDER_TARGETS;
      result = 1;
      break;
   case GL_READ_BUFFER:
      {
         GLXX_FRAMEBUFFER_T *fb = state->bound_read_framebuffer;
         params[0] = GL_NONE;
         if (fb->read_buffer != GLXX_INVALID_ATT)
         {
            if (fb->name == 0)
               params[0] = GL_BACK;
            else
               params[0] = GL_COLOR_ATTACHMENT0 +
                  (fb->read_buffer - GLXX_COLOR0_ATT);
         }
         result = 1;
      }
      break;
   case GL_DRAW_BUFFER0:
   case GL_DRAW_BUFFER1:
   case GL_DRAW_BUFFER2:
   case GL_DRAW_BUFFER3:
      {
         GLXX_FRAMEBUFFER_T *fb;
         unsigned index;

         index = (pname - GL_DRAW_BUFFER0);
         assert(index < GLXX_MAX_RENDER_TARGETS);

         params[0] = GL_NONE;
         result = 1;

         fb = state->bound_draw_framebuffer;
         if (fb->draw_buffer[index] == true)
         {
            if (fb->name == 0)
            {
               assert(index == 0);
               params[0] = GL_BACK;
            }
            else
               params[0] = GL_COLOR_ATTACHMENT0 + index;
         }
      }
      break;
   case GL_READ_FRAMEBUFFER_BINDING:
      params[0] = state->bound_read_framebuffer->name;
      result = 1;
      break;
   case GL_MAX_COLOR_ATTACHMENTS:
      params[0] = GLXX_MAX_RENDER_TARGETS;
      result = 1;
      break;
   case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS:
      params[0] = GLXX_CONFIG_MAX_TF_INTERLEAVED_COMPONENTS;
      result = 1;
      break;
   case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:
      params[0] = GLXX_CONFIG_MAX_TF_SEPARATE_ATTRIBS;
      result = 1;
      break;
   case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:
      params[0] = GLXX_CONFIG_MAX_TF_SEPARATE_COMPONENTS;
      result = 1;
      break;
   case GL_MAX_UNIFORM_BUFFER_BINDINGS:
      params[0] = GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS;
      result = 1;
      break;
   case GL_UNIFORM_BUFFER_BINDING:
      params[0] = state->bound_buffer[GLXX_BUFTGT_UNIFORM_BUFFER].buffer;
      result = 1;
      break;
   case GL_TRANSFORM_FEEDBACK_BINDING:
      params[0] = glxx_tf_get_bound(state);
      result = 1;
      break;
   case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
      params[0] = glxx_tf_get_bound_buffer(state);
      result = 1;
      break;
   case GL_PIXEL_PACK_BUFFER_BINDING:
      params[0] = state->bound_buffer[GLXX_BUFTGT_PIXEL_PACK].buffer;
      result = 1;
      break;
   case GL_PIXEL_UNPACK_BUFFER_BINDING:
      params[0] = state->bound_buffer[GLXX_BUFTGT_PIXEL_UNPACK].buffer;
      result = 1;
      break;
   case GL_DRAW_INDIRECT_BUFFER_BINDING:
      params[0] = state->bound_buffer[GLXX_BUFTGT_DRAW_INDIRECT].buffer;
      result = 1;
      break;
   case GL_ATOMIC_COUNTER_BUFFER_BINDING:
      params[0] = state->bound_buffer[GLXX_BUFTGT_ATOMIC_COUNTER_BUFFER].buffer;
      result = 1;
      break;
   case GL_SHADER_STORAGE_BUFFER_BINDING:
      params[0] = state->bound_buffer[GLXX_BUFTGT_SHADER_STORAGE_BUFFER].buffer;
      result = 1;
      break;
   case GL_DISPATCH_INDIRECT_BUFFER_BINDING:
      params[0] = state->bound_buffer[GLXX_BUFTGT_DISPATCH_INDIRECT].buffer;
      result = 1;
      break;
   case GL_COPY_READ_BUFFER:
      params[0] = state->bound_buffer[GLXX_BUFTGT_COPY_READ].buffer;
      result = 1;
      break;
   case GL_COPY_WRITE_BUFFER:
      params[0] = state->bound_buffer[GLXX_BUFTGT_COPY_WRITE].buffer;
      result = 1;
      break;
   case GL_NUM_EXTENSIONS:
      params[0] = glxx_get_num_gl30_extensions();
      result = 1;
      break;
   case GL_MAJOR_VERSION:
      params[0] = 3;
      result = 1;
      break;
   case GL_MINOR_VERSION:
      params[0] = KHRN_GLES31_DRIVER ? 1 : 0;
      result = 1;
      break;
   case GL_NUM_PROGRAM_BINARY_FORMATS:
      params[0] = 0;
      result = 1;
      break;
   case GL_PROGRAM_BINARY_FORMATS:
      result = 1;
      break;
   case GL_CURRENT_PROGRAM:
      params[0] = state->program ? state->program->name : 0;
      result = 1;
      break;
   case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
      params[0] = state->hints.fshader_derivative;
      result = 1;
      break;
#if GL_OES_matrix_palette
   case GL_MAX_PALETTE_MATRICES_OES:
      params[0] = GL11_CONFIG_MAX_PALETTE_MATRICES_OES;
      result = 1;
      break;
   case GL_MAX_VERTEX_UNITS_OES:
      params[0] = GL11_CONFIG_MAX_VERTEX_UNITS_OES;
      result = 1;
      break;
   case GL_CURRENT_PALETTE_MATRIX_OES:
      params[0] = state->gl11.current_palette_matrix;
      result = 1;
      break;
#endif   /* GL_OES_matrix_palette */
   case GL_CLIENT_ACTIVE_TEXTURE:
      params[0] = state->gl11.client_active_texture;
      result = 1;
      break;
   case GL_VERTEX_ARRAY_SIZE:
      params[0] = vao->attrib_config[GL11_IX_VERTEX].size;
      result = 1;
      break;
   case GL_VERTEX_ARRAY_TYPE:
      params[0] = vao->attrib_config[GL11_IX_VERTEX].type;
      result = 1;
      break;
   case GL_VERTEX_ARRAY_STRIDE:
      params[0] = vao->vbos[vao->attrib_config[GL11_IX_VERTEX].vbo_index].original_stride;
      result = 1;
      break;
   case GL_NORMAL_ARRAY_TYPE:
      params[0] = vao->attrib_config[GL11_IX_NORMAL].type;
      result = 1;
      break;
   case GL_NORMAL_ARRAY_STRIDE:
      params[0] = vao->vbos[vao->attrib_config[GL11_IX_NORMAL].vbo_index].original_stride;
      result = 1;
      break;
   case GL_COLOR_ARRAY_SIZE:
      params[0] = vao->attrib_config[GL11_IX_COLOR].size;
      result = 1;
      break;
   case GL_COLOR_ARRAY_TYPE:
      params[0] = vao->attrib_config[GL11_IX_COLOR].type;
      result = 1;
      break;
   case GL_COLOR_ARRAY_STRIDE:
      params[0] = vao->vbos[vao->attrib_config[GL11_IX_COLOR].vbo_index].original_stride;
      result = 1;
      break;
   case GL_TEXTURE_COORD_ARRAY_SIZE:
      params[0] = vao->attrib_config[state->gl11.client_active_texture - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].size;
      result = 1;
      break;
   case GL_TEXTURE_COORD_ARRAY_TYPE:
      params[0] = vao->attrib_config[state->gl11.client_active_texture - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].type;
      result = 1;
      break;
   case GL_TEXTURE_COORD_ARRAY_STRIDE:
      params[0] = vao->vbos[vao->attrib_config[state->gl11.client_active_texture - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD].vbo_index].original_stride;
      result = 1;
      break;
   case GL_POINT_SIZE_ARRAY_TYPE_OES:
      params[0] = vao->attrib_config[GL11_IX_POINT_SIZE].type;
      result = 1;
      break;
   case GL_POINT_SIZE_ARRAY_STRIDE_OES:
      params[0] = vao->vbos[vao->attrib_config[GL11_IX_POINT_SIZE].vbo_index].original_stride;
      result = 1;
      break;
#if GL_OES_matrix_palette
   case GL_MATRIX_INDEX_ARRAY_SIZE_OES:
      params[0] = vao->attrib_config[GL11_IX_MATRIX_INDEX].size;
      result = 1;
      break;
   case GL_MATRIX_INDEX_ARRAY_TYPE_OES:
      params[0] = vao->attrib_config[GL11_IX_MATRIX_INDEX].type;
      result = 1;
      break;
   case GL_MATRIX_INDEX_ARRAY_STRIDE_OES:
      params[0] = vao->vbos[vao->attrib_config[GL11_IX_MATRIX_INDEX].vbo_index].original_stride;
      result = 1;
      break;
   case GL_WEIGHT_ARRAY_SIZE_OES:
      params[0] = vao->attrib_config[GL11_IX_MATRIX_WEIGHT].size;
      result = 1;
      break;
   case GL_WEIGHT_ARRAY_TYPE_OES:
      params[0] = vao->attrib_config[GL11_IX_MATRIX_WEIGHT].type;
      result = 1;
      break;
   case GL_WEIGHT_ARRAY_STRIDE_OES:
      params[0] = vao->vbos[vao->attrib_config[GL11_IX_MATRIX_WEIGHT].vbo_index].original_stride;
      result = 1;
      break;
#endif //GL_OES_matrix_palette
   case GL_VERTEX_ARRAY_BUFFER_BINDING:
   case GL_NORMAL_ARRAY_BUFFER_BINDING:
   case GL_COLOR_ARRAY_BUFFER_BINDING:
   case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
   case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
   case GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES:
   case GL_WEIGHT_ARRAY_BUFFER_BINDING_OES:
   {
      int arr = gl11_binding_to_attr(pname);
      if (arr == (int)GL11_IX_CLIENT_ACTIVE_TEXTURE)
         arr = state->gl11.client_active_texture - GL_TEXTURE0 + GL11_IX_TEXTURE_COORD;
      GLXX_VBO_BINDING_T *vbo = &vao->vbos[vao->attrib_config[arr].vbo_index];

      if (vbo->buffer == NULL)
         params[0] = 0;
      else
         params[0] = vbo->buffer->name;
      result = 1;
      break;
   }
   case GL_UNPACK_ALIGNMENT:
      params[0] = state->pixel_store_state.unpack.alignment;
      result = 1;
      break;
   case GL_UNPACK_ROW_LENGTH:
      params[0] = state->pixel_store_state.unpack.row_length;
      result = 1;
      break;
   case GL_UNPACK_SKIP_ROWS:
      params[0] = state->pixel_store_state.unpack.skip_rows;
      result = 1;
      break;
   case GL_UNPACK_SKIP_PIXELS:
      params[0] = state->pixel_store_state.unpack.skip_pixels;
      result = 1;
      break;
   case GL_UNPACK_SKIP_IMAGES:
      params[0] = state->pixel_store_state.unpack_skip_images;
      result = 1;
      break;
   case GL_UNPACK_IMAGE_HEIGHT:
      params[0] = state->pixel_store_state.unpack_image_height;
      result = 1;
      break;
   case GL_PACK_ALIGNMENT:
      params[0] = state->pixel_store_state.pack.alignment;
      result = 1;
      break;
   case GL_PACK_ROW_LENGTH:
      params[0] = state->pixel_store_state.pack.row_length;
      result = 1;
      break;
   case GL_PACK_SKIP_ROWS:
      params[0] = state->pixel_store_state.pack.skip_rows;
      result = 1;
      break;
   case GL_PACK_SKIP_PIXELS:
      params[0] = state->pixel_store_state.pack.skip_pixels;
      result = 1;
      break;
   case GL_VERTEX_ARRAY_BINDING:
      params[0] = vao->name;
      result = 1;
      break;
   case GL_SAMPLER_BINDING:
   {
      GLXX_TEXTURE_SAMPLER_STATE_T *sampler = (GLXX_TEXTURE_SAMPLER_STATE_T *)
         state->bound_sampler[state->active_texture - GL_TEXTURE0];
      if (sampler != NULL)
         params[0] = sampler->id;
      else
         params[0] = 0;
      result = 1;
      break;
   }

   // OpenGL ES 3.1
   case GL_MAX_FRAMEBUFFER_WIDTH:
   case GL_MAX_FRAMEBUFFER_HEIGHT:
      params[0] = GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE;
      result = 1;
      break;
   case GL_MAX_FRAMEBUFFER_SAMPLES:
      params[0] = GLXX_CONFIG_MAX_SAMPLES;
      result = 1;
      break;
   case GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS:
      params[0] = GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS;
      result = 1;
      break;
   case GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT:
      params[0] = GLXX_CONFIG_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT;
      result = 1;
      break;
   case GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS:
      params[0] = GLXX_CONFIG_MAX_VERTEX_SSBOS;
      result = 1;
      break;
   case GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS:
      params[0] = GLXX_CONFIG_MAX_FRAGMENT_SSBOS;
      result = 1;
      break;
   case GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS:
      params[0] = GLXX_CONFIG_MAX_COMPUTE_SSBOS;
      result = 1;
      break;
   case GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS:
      params[0] = GLXX_CONFIG_MAX_COMBINED_SSBOS;
      result = 1;
      break;
   case GL_MAX_SAMPLE_MASK_WORDS:
      params[0] = GLXX_CONFIG_MAX_SAMPLE_WORDS;
      result = 1;
      break;
   case GL_MAX_UNIFORM_LOCATIONS:
      params[0] = GL20_CONFIG_MAX_UNIFORM_LOCATIONS;
      result = 1;
      break;
   case GL_MAX_IMAGE_UNITS:
      params[0] = GLXX_CONFIG_MAX_IMAGE_UNITS;
      result = 1;
      break;
   case GL_MAX_VERTEX_IMAGE_UNIFORMS:
      params[0] = GLXX_CONFIG_MAX_VERTEX_IMAGE_UNIFORMS;
      result = 1;
      break;
   case GL_MAX_FRAGMENT_IMAGE_UNIFORMS:
      params[0] = GLXX_CONFIG_MAX_FRAGMENT_IMAGE_UNIFORMS;
      result = 1;
      break;
   case GL_MAX_COMPUTE_IMAGE_UNIFORMS:
      params[0] = GLXX_CONFIG_MAX_COMPUTE_IMAGE_UNIFORMS;
      result = 1;
      break;
   case GL_MAX_COMBINED_IMAGE_UNIFORMS:
      params[0] = GLXX_CONFIG_MAX_COMBINED_IMAGE_UNIFORMS;
      result = 1;
      break;
   case GL_MAX_COMPUTE_UNIFORM_COMPONENTS:
      params[0] = GL20_CONFIG_MAX_UNIFORM_SCALARS;
      result = 1;
      break;
   case GL_MAX_COMPUTE_UNIFORM_BLOCKS:
      params[0] = GLXX_CONFIG_MAX_COMPUTE_UNIFORM_BLOCKS;
      result = 1;
      break;
   case GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS:
      params[0] = GLXX_CONFIG_MAX_COMPUTE_TEXTURE_IMAGE_UNITS;
      result = 1;
      break;
   case GL_MAX_COMPUTE_ATOMIC_COUNTERS:
      params[0] = GLXX_CONFIG_MAX_COMPUTE_ATOMIC_COUNTERS;
      result = 1;
      break;
   case GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS:
      params[0] = GLXX_CONFIG_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS;
      result = 1;
      break;
   case GL_MAX_VERTEX_ATOMIC_COUNTERS:
      params[0] = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTERS;
      result = 1;
      break;
   case GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS:
      params[0] = GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS;
      result = 1;
      break;
   case GL_MAX_FRAGMENT_ATOMIC_COUNTERS:
      params[0] = GLXX_CONFIG_MAX_FRAGMENT_ATOMIC_COUNTERS;
      result = 1;
      break;
   case GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS:
      params[0] = GLXX_CONFIG_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS;
      result = 1;
      break;
   case GL_MAX_COMBINED_ATOMIC_COUNTERS:
      params[0] = GLXX_CONFIG_MAX_COMBINED_ATOMIC_COUNTERS;
      result = 1;
      break;
   case GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS:
      params[0] = GLXX_CONFIG_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS;
      result = 1;
      break;
   case GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS:
      params[0] = GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;
      result = 1;
      break;
   case GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE:
      params[0] = GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_SIZE;
      result = 1;
      break;
   case GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES:
      params[0] = GLXX_CONFIG_MAX_COMBINED_SHADER_OUTPUTS;
      result = 1;
      break;

   // ES3 extension integers
#if GL_BRCM_provoking_vertex
   case GL_PROVOKING_VERTEX_BRCM:
      params[0] = state->provoking_vtx;
      result = 1;
      break;
#endif

#if GL_EXT_robustness
   case GL_RESET_NOTIFICATION_STRATEGY_EXT:
      params[0] = egl_context_gl_notification(state->context) ?
         GL_LOSE_CONTEXT_ON_RESET_EXT : GL_NO_RESET_NOTIFICATION_EXT;
      result = 1;
      break;
#endif

   default:
      UNREACHABLE();
      result = 0;
      break;
   }

   return result;
}

static int glxx_get_signed_integer_internal(GLXX_SERVER_STATE_T *state, GLenum pname, int *params)
{
   int result = 0;
   GLXX_VAO_T *vao;
   vao = state->vao.bound;
   assert(vao);

   // First condition should have been checked by the caller
   assert(glxx_is_signed_integer(state, pname) && !glxx_is_unsigned_integer(state, pname));

   switch (pname) {
   case GL_MODELVIEW_MATRIX_FLOAT_AS_INT_BITS_OES:
      gl11_matrix_load((float *)params, state->gl11.current_modelview);
      result = 16;
      break;
   case GL_PROJECTION_MATRIX_FLOAT_AS_INT_BITS_OES:
      gl11_matrix_load((float *)params, state->gl11.current_projection);
      result = 16;
      break;
   case GL_TEXTURE_MATRIX_FLOAT_AS_INT_BITS_OES:
      assert(state->active_texture - GL_TEXTURE0 < GL11_CONFIG_MAX_TEXTURE_UNITS);

      gl11_matrix_load((float *)params, state->gl11.texunits[state->active_texture - GL_TEXTURE0].current_matrix);
      result = 16;
      break;
   case GL_VIEWPORT:
      params[0] = state->viewport.x;
      params[1] = state->viewport.y;
      params[2] = state->viewport.width;
      params[3] = state->viewport.height;
      result = 4;
      break;
   case GL_SCISSOR_BOX:
      params[0] = state->scissor.x;
      params[1] = state->scissor.y;
      params[2] = state->scissor.width;
      params[3] = state->scissor.height;
      result = 4;
      break;
   case GL_STENCIL_CLEAR_VALUE:
      params[0] = state->clear.stencil_value;
      result = 1;
      break;
   case GL_STENCIL_REF:
      {
         int max = ((1 << glxx_get_stencil_size(state)) - 1);
         params[0] = gfx_sclamp(state->stencil_func.front.ref, 0, max);
         result = 1;
      }
      break;
   case GL_STENCIL_BACK_REF:
      {
         int max = ((1 << glxx_get_stencil_size(state)) - 1);
         params[0] = gfx_sclamp(state->stencil_func.back.ref, 0, max);
         result = 1;
      }
      result = 1;
      break;
   case GL_MIN_PROGRAM_TEXEL_OFFSET:
      params[0] = GLXX_CONFIG_MIN_TEXEL_OFFSET;
      result = 1;
      break;
   case GL_MAX_PROGRAM_TEXEL_OFFSET:
      params[0] = GLXX_CONFIG_MAX_TEXEL_OFFSET;
      result = 1;
      break;
   case GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET:
      params[0] = GLXX_CONFIG_MAX_TEXEL_OFFSET;
      result = 1;
      break;
   case GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET:
      params[0] = GLXX_CONFIG_MIN_TEXEL_OFFSET;
      result = 1;
      break;
   default:
      UNREACHABLE();
      result = 0;
      break;
   }

   return result;
}

/*
   glGetFloatv (GLenum pname, GLfloat *params)
   glGetFixedv (GLenum pname, GLfixed *params)

   We call glxx_is_* to classify the pname, fetch it using the correct function
   and then convert it to float.

   There is an internal assumption that no state variable has more than MAX_<TYPE>_ITEM_COUNT scalar elements.
*/

int glxx_get_float_or_fixed_internal(GLXX_SERVER_STATE_T *state, GLenum pname, float *params)
{
   if (glxx_is_boolean(state, pname)) {
      GLboolean temp[MAX_BOOLEAN_ITEM_COUNT];
      int count = glxx_get_boolean_internal(state, pname, temp);
      int i;

      assert(count <= MAX_BOOLEAN_ITEM_COUNT);

      for (i = 0; i < count; i++)
         params[i] = temp[i] ? 1.0f : 0.0f;

      return count;
   } else if (is_int64(state, pname)) {
      int64_t ans;
      get_int64_internal(state, pname, &ans);
      params[0] = (float)ans;
      return 1;
   } else if (glxx_is_signed_integer(state, pname)) {
      int temp[MAX_INTEGER_ITEM_COUNT];
      int count = glxx_get_signed_integer_internal(state, pname, temp);
      int i;

      assert(count <= MAX_INTEGER_ITEM_COUNT);

      for (i = 0; i < count; i++)
      {
         params[i] = (float)temp[i];
      }

      return count;
   } else if (glxx_is_unsigned_integer(state, pname)) {
      unsigned int temp[MAX_INTEGER_ITEM_COUNT];
      int count = glxx_get_unsigned_integer_internal(state, pname, temp);
      int i;

      assert(count <= MAX_INTEGER_ITEM_COUNT);

      for (i = 0; i < count; i++)
      {
         params[i] = (float)temp[i];
      }

      return count;
   } else if (glxx_is_float(state, pname))
      return glxx_get_float_internal(state, pname, params);
   else {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

      return 0;
   }
}

void glxx_get_fixed_internal (GLXX_SERVER_STATE_T *state, GLenum pname, GLfixed *params)
{
   int i;
   GLfloat temp[MAX_FLOAT_ITEM_COUNT];
   int count;

   count = glxx_get_float_or_fixed_internal(state, pname, temp);

   assert(count <= MAX_FLOAT_ITEM_COUNT);

   for (i = 0; i < count; i++)
      params[i] = float_to_fixed(temp[i]);
}

GL_API void GL_APIENTRY glGetFloatv(GLenum pname, GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   if (!state)
      return;

   (void)glxx_get_float_or_fixed_internal(state, pname, params);

   GLXX_UNLOCK_SERVER_STATE();
}

static void get_int64_internal(GLXX_SERVER_STATE_T *state, GLenum pname, GLint64 *params)
{
   switch (pname)
   {
   case GL_MAX_ELEMENT_INDEX:
      params[0] = 0x00ffffff;
      break;
   case GL_MAX_SERVER_WAIT_TIMEOUT:
      params[0] = 0; // TODO
      break;
   case GL_MAX_UNIFORM_BLOCK_SIZE:
      params[0] = GLXX_CONFIG_MAX_UNIFORM_BLOCK_SIZE;
      break;
   case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
      params[0] = GL20_CONFIG_MAX_UNIFORM_VECTORS * 4 +
                  GLXX_CONFIG_MAX_VERTEX_UNIFORM_BLOCKS * GLXX_CONFIG_MAX_UNIFORM_BLOCK_SIZE / 4;
      break;
   case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
      params[0] = GL20_CONFIG_MAX_UNIFORM_VECTORS * 4 +
                  GLXX_CONFIG_MAX_FRAGMENT_UNIFORM_BLOCKS * GLXX_CONFIG_MAX_UNIFORM_BLOCK_SIZE / 4;
      break;
   case GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS:
      params[0] = GL20_CONFIG_MAX_UNIFORM_VECTORS * 4 +
                  GLXX_CONFIG_MAX_COMPUTE_UNIFORM_BLOCKS * GLXX_CONFIG_MAX_UNIFORM_BLOCK_SIZE / 4;
      break;
   case GL_MAX_SHADER_STORAGE_BLOCK_SIZE:
      params[0] = GLXX_CONFIG_MAX_SHADER_STORAGE_BLOCK_SIZE;
      break;
   default:
      UNREACHABLE();
   }
}

GL_API void GL_APIENTRY glGetInteger64v(GLenum pname, GLint64* params)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   if (!state) return;

   if (glxx_is_boolean(state, pname)) {
      GLboolean temp[MAX_BOOLEAN_ITEM_COUNT];
      int count, i;
      count = glxx_get_boolean_internal(state, pname, temp);
      assert(count <= MAX_BOOLEAN_ITEM_COUNT);

      for (i=0; i<count; i++)
         params[i] = temp[i] ? 1 : 0;
   }
   else if (is_int64(state, pname))
   {
      get_int64_internal(state, pname, params);
   }
   else if (glxx_is_signed_integer(state, pname))
   {
      int param32[MAX_INTEGER_ITEM_COUNT] = { 0, };
      int count = 0;
      int i;
      count = glxx_get_signed_integer_internal(state, pname, param32);

      assert(count <= MAX_INTEGER_ITEM_COUNT);

      for (i=0; i<count; i++) {
         params[i] = param32[i];
      }
   }
   else if (glxx_is_unsigned_integer(state, pname))
   {
      unsigned int param32[MAX_INTEGER_ITEM_COUNT] = { 0, };
      int count = 0;
      int i;
      count = glxx_get_unsigned_integer_internal(state, pname, param32);

      assert(count <= MAX_INTEGER_ITEM_COUNT);

      for (i=0; i<count; i++) {
         params[i] = param32[i];
      }
   }
   else if (is_small_float_glxx(state, pname)) {
      GLfloat temp[4];
      GLuint count = glxx_get_float_internal(state, pname, temp);
      GLuint i;

      assert(count <= 4);

      for (i = 0; i < count; i++) {
         params[i] = (GLint)floor((4294967295.0f * temp[i] - 1.0f) / 2.0f + 0.5f);

         if (params[i] < 0)
            params[i] = 0x7fffffff;
      }
   } else if (glxx_is_float(state, pname)) {
      GLfloat temp[MAX_FLOAT_ITEM_COUNT];
      GLuint count = glxx_get_float_internal(state, pname, temp);
      GLuint i;

      assert(count <= MAX_FLOAT_ITEM_COUNT);

      for (i = 0; i < count; i++)
         params[i] = float_to_int(temp[i]);
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   GL30_UNLOCK_SERVER_STATE();
}

static bool is_indexed_boolean(GLenum target) {
   return false;     /* TODO: None of these are supported yet */
}

static bool is_indexed_integer(GLenum target) {
   switch (target) {
      case GL_UNIFORM_BUFFER_BINDING:
      case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
         return true;

      case GL_MAX_COMPUTE_WORK_GROUP_COUNT:
      case GL_MAX_COMPUTE_WORK_GROUP_SIZE:
      case GL_ATOMIC_COUNTER_BUFFER_BINDING:
      case GL_SHADER_STORAGE_BUFFER_BINDING:
      case GL_SAMPLE_MASK_VALUE:
      case GL_VERTEX_BINDING_DIVISOR:
      case GL_VERTEX_BINDING_STRIDE:
      case GL_VERTEX_BINDING_BUFFER:
         return KHRN_GLES31_DRIVER ? true : false;

      default: return false;
   }
}

static bool is_indexed_int64(GLenum target) {
   switch (target) {
   case GL_UNIFORM_BUFFER_START:
   case GL_UNIFORM_BUFFER_SIZE:
   case GL_TRANSFORM_FEEDBACK_BUFFER_START:
   case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
      return true;
   case GL_SHADER_STORAGE_BUFFER_START:
   case GL_SHADER_STORAGE_BUFFER_SIZE:
   case GL_ATOMIC_COUNTER_BUFFER_START:
   case GL_ATOMIC_COUNTER_BUFFER_SIZE:
   case GL_VERTEX_BINDING_OFFSET:
      return KHRN_GLES31_DRIVER ? true : false;

   default:
      return false;
   }
}

static unsigned indexed_get_max_index(GLenum target) {
   switch (target) {
      case GL_UNIFORM_BUFFER_BINDING:              return GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS;
      case GL_UNIFORM_BUFFER_START:                return GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS;
      case GL_UNIFORM_BUFFER_SIZE:                 return GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS;
      case GL_SHADER_STORAGE_BUFFER_BINDING:       return GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS;
      case GL_SHADER_STORAGE_BUFFER_START:         return GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS;
      case GL_SHADER_STORAGE_BUFFER_SIZE:          return GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS;
      case GL_ATOMIC_COUNTER_BUFFER_BINDING:       return GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;
      case GL_ATOMIC_COUNTER_BUFFER_START:         return GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;
      case GL_ATOMIC_COUNTER_BUFFER_SIZE:          return GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;
      case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:   return GLXX_CONFIG_MAX_TF_SEPARATE_ATTRIBS;
      case GL_TRANSFORM_FEEDBACK_BUFFER_START:     return GLXX_CONFIG_MAX_TF_SEPARATE_ATTRIBS;
      case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:      return GLXX_CONFIG_MAX_TF_SEPARATE_ATTRIBS;
      case GL_MAX_COMPUTE_WORK_GROUP_COUNT:        return 3;
      case GL_MAX_COMPUTE_WORK_GROUP_SIZE:         return 3;
      case GL_SAMPLE_MASK_VALUE:                   return GLXX_CONFIG_MAX_SAMPLE_WORDS;
      case GL_VERTEX_BINDING_OFFSET:               return GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS;
      case GL_VERTEX_BINDING_DIVISOR:              return GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS;
      case GL_VERTEX_BINDING_STRIDE:               return GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS;
      case GL_VERTEX_BINDING_BUFFER:               return GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS;
      default: unreachable();                      return 0;
   }
}

static bool get_indexed_bool(const GLXX_SERVER_STATE_T *state, GLenum target, unsigned index) {
   UNREACHABLE();
   return false;
}

static int get_indexed_int(const GLXX_SERVER_STATE_T *state, GLenum target, unsigned index) {
   switch (target)
   {
   case GL_UNIFORM_BUFFER_BINDING:
      return state->uniform_block.binding_points[index].buffer.buffer;
   case GL_SHADER_STORAGE_BUFFER_BINDING:
      return state->ssbo.binding_points[index].buffer.buffer;
   case GL_ATOMIC_COUNTER_BUFFER_BINDING:
      return state->atomic_counter.binding_points[index].buffer.buffer;
   case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
   {
      int data;
      glxx_tf_get_integeri(state, target, index, &data);
      return data;
   }
   case GL_MAX_COMPUTE_WORK_GROUP_COUNT:
      return GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT;
   case GL_MAX_COMPUTE_WORK_GROUP_SIZE:
   {
      const int v[3] = { GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_X,
                         GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_Y,
                         GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_Z };
      return v[index];
   }
   case GL_SAMPLE_MASK_VALUE:
      return state->sample_mask.mask[index];
   case GL_VERTEX_BINDING_DIVISOR:
      return state->vao.bound->vbos[index].divisor;
   case GL_VERTEX_BINDING_STRIDE:
      return state->vao.bound->vbos[index].original_stride;
   case GL_VERTEX_BINDING_BUFFER:
      return state->vao.bound->vbos[index].buffer ? state->vao.bound->vbos[index].buffer->name : 0;
   default:
      UNREACHABLE(); return 0;
   }
}

static int64_t get_indexed_int64(const GLXX_SERVER_STATE_T *state, GLenum target, unsigned index) {
   switch (target)
   {
   case GL_UNIFORM_BUFFER_START:
   case GL_UNIFORM_BUFFER_SIZE:
      if (state->uniform_block.binding_points[index].buffer.obj != NULL)
      {
         if (target == GL_UNIFORM_BUFFER_START)
            return state->uniform_block.binding_points[index].offset;
         else
            // Internally -1 means full size. In GL API 0 means full size
            return (state->uniform_block.binding_points[index].size == -1)
               ? 0
               : state->uniform_block.binding_points[index].size;
      }
      else
      {
         /* If no buffer is bound the return should be 0 */
         return 0;
      }
      break;
   case GL_SHADER_STORAGE_BUFFER_START:
   case GL_SHADER_STORAGE_BUFFER_SIZE:
      if (state->ssbo.binding_points[index].buffer.obj != NULL)
      {
         if (target == GL_SHADER_STORAGE_BUFFER_START)
            return state->ssbo.binding_points[index].offset;
         else
            // Internally -1 means full size. In GL API 0 means full size
            return (state->ssbo.binding_points[index].size == -1)
               ? 0
               : state->ssbo.binding_points[index].size;
      }
      else
      {
         /* If no buffer is bound the return should be 0 */
         return 0;
      }
      break;
   case GL_ATOMIC_COUNTER_BUFFER_START:
   case GL_ATOMIC_COUNTER_BUFFER_SIZE:
      if (state->atomic_counter.binding_points[index].buffer.obj != NULL)
      {
         if (target == GL_ATOMIC_COUNTER_BUFFER_START)
            return state->atomic_counter.binding_points[index].offset;
         else
            // Internally -1 means full size. In GL API 0 means full size
            return (state->atomic_counter.binding_points[index].size == -1)
               ? 0
               : state->atomic_counter.binding_points[index].size;
      }
      else
      {
         /* If no buffer is bound the return should be 0 */
         return 0;
      }
      break;
   case GL_TRANSFORM_FEEDBACK_BUFFER_START:
   case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
   {
      int64_t data;
      glxx_tf_get_integer64i(state, target, index, &data);
      return data;
   }
   case GL_VERTEX_BINDING_OFFSET:
      return state->vao.bound->vbos[index].offset;
   default:
      UNREACHABLE();
   }
}

GL_API void GL_APIENTRY glGetBooleani_v (GLenum target, GLuint index, GLboolean *data) {
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE();
   if (!state) return;

   if (!is_indexed_boolean(target) && !is_indexed_integer(target) && !is_indexed_int64(target)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (index >= indexed_get_max_index(target)) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (is_indexed_boolean(target)) {
      data[0] = get_indexed_bool(state, target, index);
   } else if (is_indexed_integer(target)) {
      int temp = get_indexed_int(state, target, index);
      data[0] = temp ? GL_TRUE : GL_FALSE;
   } else {
      int64_t temp = get_indexed_int64(state, target, index);
      data[0] = temp ? GL_TRUE : GL_FALSE;
   }

end:
   GL31_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glGetIntegeri_v(GLenum target, GLuint index, GLint* data)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   if (!state) return;

   if (!is_indexed_boolean(target) && !is_indexed_integer(target) && !is_indexed_int64(target)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (index >= indexed_get_max_index(target)) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (is_indexed_boolean(target)) {
      bool temp = get_indexed_bool(state, target, index);
      data[0] = temp ? 1 : 0;
   } else if (is_indexed_integer(target)) {
      data[0] = get_indexed_int(state, target, index);
   } else {
      int64_t temp = get_indexed_int64(state, target, index);
      data[0] = (int)temp;
   }

end:
   GL30_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glGetInteger64i_v(GLenum target, GLuint index, GLint64* data)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   if (!state) return;

   if (!is_indexed_boolean(target) && !is_indexed_integer(target) && !is_indexed_int64(target)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (index >= indexed_get_max_index(target)) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (is_indexed_boolean(target)) {
      bool temp = get_indexed_bool(state, target, index);
      data[0] = temp ? 1 : 0;
   } else if (is_indexed_integer(target)) {
      int temp = get_indexed_int(state, target, index);
      data[0] = temp;
   } else {
      data[0] = get_indexed_int64(state, target, index);
   }

end:
   GL30_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glGetInternalformativ (GLenum target,
                                               GLenum internalformat,
                                               GLenum pname,
                                               GLsizei bufSize,
                                               GLint* params)
{
   bool renderable;
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE_UNCHANGED();
   if (!state) return;

   renderable = glxx_is_color_renderable_internalformat(internalformat) ||
                glxx_is_depth_renderable_internalformat(internalformat) ||
                glxx_is_stencil_renderable_internalformat(internalformat);

   if (!renderable || (target != GL_RENDERBUFFER && (!KHRN_GLES31_DRIVER || target != GL_TEXTURE_2D_MULTISAMPLE)))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (bufSize < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (bufSize == 0)
   {
      goto end;   /* No space to write anything, so done. */
   }

   glxx_ms_mode max_ms_mode = glxx_max_ms_mode_for_internalformat(internalformat);

   switch (pname) {
      case GL_NUM_SAMPLE_COUNTS:
         if (max_ms_mode == GLXX_NO_MS) params[0] = 0;
         else             params[0] = 1;
         break;
      case GL_SAMPLES:
         params[0] = max_ms_mode;
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
         goto end;
   }

end:
   GL30_UNLOCK_SERVER_STATE();
}

/* the order and location of samples in TLB is (in 1/8s from bottom-left):
 *      1 2 3 4 5 6 7
 *     _______________
 *  7  |_|_|3|_|_|_|_|
 *  6  |_|_|_|_|_|_|_|
 *  5  |_|_|_|_|_|_|2|
 *  4  |_|_|_|_|_|_|_|
 *  3  |1| |_|_|_|_|_|
 *  2  |_|_|_|_|_|_|_|
 *  1  |_|_|_|_|0|_|_|
 *
 */
GL_APICALL void GL_APIENTRY glGetMultisamplefv (GLenum pname, GLuint index,
      GLfloat *val)
{
   GLenum error = GL_NO_ERROR;
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE();
   if (!state)
      return;

   if (pname != GL_SAMPLE_POSITION)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   /* if fb is not complete, samples(ms_mode) is undefined */
   glxx_ms_mode ms_mode = glxx_fb_get_ms_mode(state->bound_draw_framebuffer);
   if (index >= (unsigned) ms_mode)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }
   assert(ms_mode == GLXX_4X_MS);

   switch(index)
   {
      case 0:
         val[0] = 5; val[1] = 1;
         break;
      case 1:
         val[0] = 1; val[1] = 3;
         break;
      case 2:
         val[0] = 7; val[1] = 5;
         break;
      case 3:
         val[0] = 3; val[1] = 7;
         break;
      default:
         unreachable();
   }
   val[0] *= 1.0/8;
   val[1] *= 1.0/8;
end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   GL31_UNLOCK_SERVER_STATE();
}
