/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/egl/egl_server.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include "interface/khronos/egl/egl_client_context.h"

#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/glxx/glxx_texture.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"
#include "middleware/khronos/glxx/glxx_shared.h"
#include "middleware/khronos/glxx/glxx_buffer.h"

#include "interface/khronos/glxx/glxx_int_attrib.h"
#include "interface/khronos/glxx/glxx_int_config.h"
#include "middleware/khronos/gl11/gl11_matrix.h"
#include "middleware/khronos/gl11/gl11_texunit.h"

#include "interface/khronos/common/khrn_options.h"
#include "middleware/khronos/glxx/glxx_tweaker.h"

typedef struct {
   int uniform;
   int index;
   bool in_vshader; /* true if this is a sampler from the vertex shader */
} GL20_SAMPLER_INFO_T;

typedef struct {
   int offset;
   int size;
   unsigned type;
   bool is_array;
   char *name;
} GL20_UNIFORM_INFO_T;

typedef struct {
   GLclampf value;
   GLboolean invert;
} GLXX_SAMPLE_COVERAGE_T;

typedef struct
{
   GLenum equation;
   GLenum equation_alpha;
   GLenum src_function;
   GLenum src_function_alpha;
   GLenum dst_function;
   GLenum dst_function_alpha;
   GLenum sample_alpha_to_coverage;
   GLenum sample_coverage;
   GLXX_SAMPLE_COVERAGE_T sample_coverage_v;

   bool ms;
   GLenum logic_op;

   /*
      Current color mask

      Khronos state variable names:

      COLOR_WRITEMASK       (1,1,1,1)

      Invariant:

      color_mask is made up of:
         0x000000ff
         0x0000ff00
         0x00ff0000
         0xff000000
   */
   uint32_t color_mask;
} GLXX_HW_BLEND_T;

typedef struct {
   int size;
   GLenum type;
   bool norm;
} GLXX_ATTRIB_ABSTRACT_T;

typedef struct
{
   GLXX_ATTRIB_ABSTRACT_T attribs[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   uint32_t primitive_type;
   GLXX_HW_BLEND_T blend;
   unsigned int stencil_config;       /* bit 0 = use_stencil, bit 1 = fwd/rev masks are one of the special ones
                                         bit 2 = fwd/rev masks can be specified together */
   bool use_depth;
   bool render_alpha;
   bool rgb565;
#if GL_EXT_texture_format_BGRA8888
   bool texture_rb_swap[GLXX_CONFIG_MAX_TEXTURE_UNITS];
#endif
   bool fb_rb_swap;
   bool egl_output;
} GLXX_LINK_RESULT_KEY_T;

typedef struct {
   bool enabled;
   bool position_w_is_0;
   bool spot_cutoff_is_180;
} GL11_CACHE_LIGHT_ABSTRACT_T;

typedef struct {
   GLenum mode;

   GL11_COMBINER_T rgb;
   GL11_COMBINER_T alpha;

   GL11_TEXUNIT_PROPS_T props;
   bool coord_replace;
} GL11_CACHE_TEXUNIT_ABSTRACT_T;

typedef struct
{
   GLenum primitive_mode;

   /* GLES 2.0 only */
   GL20_SAMPLER_INFO_T *sampler_info;
   GL20_UNIFORM_INFO_T *uniform_info;
   uint32_t *uniform_data;
   uint32_t cattribs_live;
   uint32_t vattribs_live;
   int num_samplers;
} GLXX_DRAW_BATCH_T;

typedef struct {
   GLXX_LINK_RESULT_KEY_T common;

   GL11_CACHE_LIGHT_ABSTRACT_T lights[GL11_CONFIG_MAX_LIGHTS];
   GL11_CACHE_TEXUNIT_ABSTRACT_T texunits[GL11_CONFIG_MAX_TEXTURE_UNITS];
   //uint32_t uniform_index[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];

   /*
      variables from state
   */


   /*
      Current light model two-sidedness

      Khronos state variable names:

      LIGHT_MODEL_TWO_SIDE          FALSE

      Invariant:

      -
   */
   bool two_side;

   bool normalize;
   bool rescale_normal;
   bool lighting;
   bool color_material;

   bool point_smooth;
   bool line_smooth;

   GLenum alpha_func;
   GLenum fog_mode;        /* fog mode or 0 for no fog */
   int user_clip_plane;    /*: 0 = off. 1 = less. -1 = lequal */

   bool drawtex; /* OES_draw_texture */
   /*
      variables
   */

   uint32_t cattribs_live;
   uint32_t vattribs_live;

} GL11_CACHE_KEY_T;

/*
   structure representing OpenGL ES 1.1 material properties
*/

typedef struct {
   GLfloat ambient[4];
   GLfloat diffuse[4];
   GLfloat specular[4];
   GLfloat emission[4];

   GLfloat shininess;
} GL11_MATERIAL_T;

/*
   structure representing OpenGL ES 1.1 lighting model properties
*/

typedef struct {
   /*
      Current light model ambient color

      Khronos state variable names:

      LIGHT_MODEL_AMBIENT           (0.2,0.2,0.2,1.0)

      Invariant:

      -  (values are unclamped)
   */

   float ambient[4];
   bool two_side;
} GL11_LIGHTMODEL_T;

/*
   structure representing OpenGL ES 1.1 light

   note that position and spot.direction are stored in eye coordinates,
   computed using the current modelview matrix
*/

typedef struct {
   bool enabled;

   GLfloat ambient[4];
   GLfloat diffuse[4];
   GLfloat specular[4];

   GLfloat position[4];

   struct {
      GLfloat constant;
      GLfloat linear;
      GLfloat quadratic;
   } attenuation;

   struct {
      GLfloat direction[4];      // fourth component unused, but included so we can compute it directly using matrix_mult_col()
      GLfloat exponent;
      GLfloat cutoff;
   } spot;

   /*
      derived values for use by HAL
   */

   GLfloat position3[3];

   GLfloat cos_cutoff;
} GL11_LIGHT_T;

/*
   structure representing OpenGL ES 1.1 fog properties
*/

typedef struct {

   /*
      Current fog mode

      Khronos state variable names:

      FOG_MODE            EXP

      Invariant:

      mode in {EXP, EXP2, LINEAR}
   */
   GLenum mode;

   /*
      Current fog color

      Khronos state variable names:

      FOG_COLOR           (0,0,0,0)

      Invariant:

      0.0 <= color[i] <= 1.0
   */
   float color[4];

   /*
      Current fog density

      Khronos state variable names:

      FOG_DENSITY            1.0

      Invariant:

      density >= 0.0
   */

   float density;
   /*
      Current linear fog start/end

      Khronos state variable names:

      FOG_START            0.0
      FOG_END              1.0

      Invariant:

      -
   */

   float start;
   float end;

   /*
      Coefficients that are useful inside vertex shading to calculate
      fog.

      scale is consistent with start and end
      coeff_exp and coeff_exp2 are consistent with density

      ... according to the update rules in fogv_internal
   */

   float scale;
   float coeff_exp;
   float coeff_exp2;
} GL11_FOG_T;

typedef struct {
   GLfloat size_min;
   GLfloat size_min_clamped;
   GLfloat size_max;
   GLfloat fade_threshold;
   GLfloat distance_attenuation[3];
} GL11_POINT_PARAMS_T;

/*

   structure representing OpenGL ES 1.1 and 2.0 state

   General invariant - this applies to various fields in the structure below

   (GL11_SERVER_STATE_CLEAN_BOOL)
   All "bool" state variables are "clean" - i.e. 0 or 1
*/

typedef struct {

   /*
      Open GL version

      Invariants:

      OPENGL_ES_11 or OPENGL_ES_20
   */

   EGL_CONTEXT_TYPE_T type;

   /*
      context is marked secure
   */

   bool secure;


   /*
      Current (server-side) active texture unit selector

      Khronos state variable names:

      ACTIVE_TEXTURE        TEXTURE0

      Invariants:

      0 <= active_texture - TEXTURE0 < GL11_CONFIG_MAX_TEXTURE_UNITS
   */

   GLenum active_texture;

   /*
      Current server-side error

      Khronos state variable names:

      -

      Invariant:

      error is a valid GL error
   */

   GLenum error;

   GL11_CACHE_KEY_T shader;
   GLXX_DRAW_BATCH_T batch;  // Only valid for lifetime of glDrawElements_impl
   uint32_t current_render_state;   // If rs changes reissue frame-starting instructions
   bool changed_misc;   // alpha_func fog line_smooth user_clip
   bool changed_light;  // light material lightmodel normalize rescale_normal
   bool changed_texunit;// texunit
   bool changed_backend;// blend stencil logicop color_mask
   bool changed_vertex; // color_material
   bool changed_directly;// texture color/alpha. TODO others. (Things we write directly into shader structure, not via calculate_and_hide)

   bool changed_cfg;     // clockwise endo oversample ztest zmask
   bool changed_linewidth;
   bool changed_polygon_offset;
   bool changed_viewport;// scissor viewport depthrange
   uint32_t old_flat_shading_flags;

   struct {
      GLXX_BUFFER_T *array_buffer;
      GLXX_BUFFER_T *element_array_buffer;
      GLuint array_name;
      GLuint element_array_name;
   } bound_buffer;
   GLXX_ATTRIB_T attrib[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];

   /*
      Current texture bound to this unit

      Khronos name:

      TEXTURE_BINDING_2D      0 (i.e. default texture for context)

      Implementation Notes:

      We store the object rather than its integer name because in another
      shared context it may have been disassociated with its name.  The name
      can be obtained from the object

      gl 2.0 defines more texture units than gl 1.1 so make this big enough for
      both

      Invariant:

      mh_twod != MEM_HANDLE_INVALID
      mh_twod is a handle to a valid GLXX_TEXTURE_T object

      Same for mh_external

      mh_cube == MEM_HANDLE_INVALID
      mh_cube is gl 2.0 only
   */

   struct {
      GLXX_TEXTURE_T *twod;
      GLXX_TEXTURE_T *external;
      GLXX_TEXTURE_T *cube;
   } bound_texture[GLXX_CONFIG_MAX_TEXTURE_UNITS];

/*
      Shared object structure

      Invariants:

      mh_shared != MEM_INVALID_MHANDLE
      mh_shared is a handle to a valid GLXX_SHARED_T object
   */

   GLXX_SHARED_T *shared;

   /*
      default texture

      Khronos state variable names:

      -

      Invariants:

      mh_default_texture_twod != MEM_HANDLE_INVALID
      mh_default_texture_twod is a handle to a valid GLXX_TEXTURE_T object

      Same goes for mh_default_texture_external
   */

   GLXX_TEXTURE_T *default_texture_twod;
   GLXX_TEXTURE_T *default_texture_external;

   /*
      for compatibility with gl 2.0

      Khronos state variable names:

      -

      Invariants:

      mh_default_texture_cube == MEM_HANDLE_INVALID
   */
   GLXX_TEXTURE_T *default_texture_cube;

   /*
      target buffers
   */

   /*
      Current color buffer for EGL context draw and read surfaces

      Invariants:

      (GL11_SERVER_STATE_DRAW, GL11_SERVER_STATE_READ) For the "current" context, mh_draw and mh_read
      are handles to valid KHRN_IMAGE_T objects with format one of

         ABGR_8888_TF
         RGB_565_TF
         RGBA_5551_TF
         RGBA_4444_TF
   */

   KHRN_IMAGE_T *draw;
   KHRN_IMAGE_T *read;

   /*
      Current depth and stencil buffer for EGL draw surface (if any)

      Invariants:

      (GL11_SERVER_STATE_DEPTH) For the "current" context, if there is a depth or stencil buffer then
         mh_depth is a handle to a valid KHRN_IMAGE_T
         format is one of
            DEPTH_16_TF
            DEPTH_32_TF
      else
         mh_depth == MEM_HANDLE_INVALID

      (GL11_SERVER_STATE_DEPTH_DIMS)
      If valid
         If multisample
            must be at least as big as multisample color buffer
         Else
            must be at least as big as draw surface color buffer
   */

   KHRN_IMAGE_T *depth;

   /*
      Current multisample color buffer for EGL context draw surface (if any)

      Invariants:

      (GL11_SERVER_STATE_MULTI)
      For the "current" context, if there is a multisample buffer then
         mh_color_multi is a handle to a valid KHRN_IMAGE_T
         format is one of
            ABGR_8888_TF
            RGB_565_TF
            RGBA_5551_TF
            RGBA_4444_TF
      else
         mh_color_multi == MEM_HANDLE_INVALID

      (GL11_SERVER_STATE_MULTI_DIMS_FMT)
      If valid must be twice the width and height of, and have the same format as the draw surface color buffer
   */

   KHRN_IMAGE_T *color_multi;
   KHRN_IMAGE_T *ds_multi;

   uint32_t config_depth_bits;   /* Bit depth of depth buffer in chosen config (not the same as physical depth bits in buffer) */
   uint32_t config_stencil_bits; /* Bit depth of stencil buffer in chosen config (not the same as physical stencil bits in buffer) */

   /*
      have we ever been made current
   */

   GLboolean made_current;

   /*
      Current clear color

      Khronos state variable names:

      COLOR_CLEAR_VALUE       (0, 0, 0, 0)

      Invariant:

      0.0 <= clear_color[i] <= 1.0
   */

   GLclampf clear_color[4];

   /*
      Current clear depth

      Khronos state variable names:

      DEPTH_CLEAR_VALUE       1

      Invariant:

      0.0 <= clear_depth <= 1.0
   */

   GLclampf clear_depth;

   /*
      Current clear stencil

      Khronos state variable names:

      STENCIL_CLEAR_VALUE       0

      Invariant:

      -
   */

   GLint clear_stencil;


   /*
      Current cull mode

      Khronos state variable names:

      CULL_FACE_MODE       BACK

      Invariant:

      (GL11_SERVER_STATE_CULL_MODE)
      cull_mode in {FRONT, BACK, FRONT_AND_BACK}
   */
   GLenum cull_mode;

   GLenum depth_func;

   GLboolean depth_mask;

   /*
      Current line width

      Khronos state variable names:

      LINE_WIDTH       1

      Invariant:

      line_width > 0.0
   */

   GLfloat line_width;

   GLenum front_face;

   /*
      Current polygon depth offset state

      Khronos state variable names:

      POLYGON_OFFSET_FACTOR   1
      POLYGON_OFFSET_UNITS    1

      Invariant:

      -
   */

   struct {
      GLfloat factor;                                       // I
      GLfloat units;                                        // I
   } polygon_offset;

   GLXX_SAMPLE_COVERAGE_T sample_coverage;

   struct {
      GLint x;
      GLint y;
      GLsizei width;
      GLsizei height;
   } scissor;

   /*
      Current viewport in GL and internal formats

      Khronos state variable names:

      DEPTH_RANGE             (0, 1)
      VIEWPORT                (0, 0, 0, 0)   note set at first use of context with surface

      Invariants:

      0 <= width <= GLXX_CONFIG_MAX_VIEWPORT_SIZE
      0 <= height <= GLXX_CONFIG_MAX_VIEWPORT_SIZE
      0.0 <= near <= 1.0
      0.0 <= far  <= 1.0

      internal is consistent with other elements according to update_viewport_internal() docs
      elements of internal are valid
   */
   struct {
      GLint x;                                              // I
      GLint y;                                              // I
      GLsizei width;                                        // I
      GLsizei height;                                       // I
      GLclampf near;                                        // I
      GLclampf far;                                         // I

      reinterpret_cast_uf internal[12];
   } viewport;

   struct {

      bool cull_face;
      bool polygon_offset_fill;
      bool sample_alpha_to_coverage;
      bool sample_coverage;
      bool scissor_test;
      bool stencil_test;
      bool depth_test;
      bool blend;
      bool dither;

      //gl 1.1 specific
      bool clip_plane[GL11_CONFIG_MAX_PLANES];
      bool multisample;
      bool sample_alpha_to_one;
      bool color_logic_op;
   } caps;

   struct {
      GLenum generate_mipmap;

      //gl 1.1 specific
      GLenum perspective_correction;
      GLenum point_smooth;
      GLenum line_smooth;
   } hints;

   struct {
      struct {
         GLenum func;                                       // I
         GLint ref;                                         // I
         GLuint mask;                                       // I
      } front;

      //gl 2.0 specific
      struct {
         GLenum func;                                       // I
         GLint ref;                                         // I
         GLuint mask;                                       // I
      } back;
   } stencil_func;

   struct  {
      GLuint front;                                         // I
      GLuint back;                                          // I
   } stencil_mask;

   struct {
      struct {
         GLenum fail;                                       // I
         GLenum zfail;                                      // I
         GLenum zpass;                                      // I
      } front;

      //gl 2.0 specific
      struct {
         GLenum fail;                                       // I
         GLenum zfail;                                      // I
         GLenum zpass;                                      // I
      } back;
   } stencil_op;

   struct {
      //gl 1.1 specific documentation
      /*
         Current source function for blend operation

         Khronos state variable names:

         BLEND_SRC       GL_ONE

         Invariant:

         src in {
            GL_ZERO,GL_ONE,
            GL_DST_COLOR,GL_ONE_MINUS_DST_COLOR,
            GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,
            GL_DST_ALPHA,GL_ONE_MINUS_DST_ALPHA,
            GL_SRC_ALPHA_SATURATE}
      */
      /*
         Current dest function for blend operation

         Khronos state variable names:

         BLEND_DST       GL_ZERO

         Invariant:

         dst in {
            GL_ZERO,GL_ONE,
            GL_SRC_COLOR,GL_ONE_MINUS_SRC_COLOR,
            GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,
            GL_DST_ALPHA,GL_ONE_MINUS_DST_ALPHA}
      */

      GLenum src_rgb;                                       // I
      GLenum dst_rgb;                                       // I
      GLenum src_alpha;                                     // I
      GLenum dst_alpha;                                     // I
   } blend_func;

   GLclampf blend_color[4];                                 // I

   struct {
      GLenum rgb;                                           // I
      GLenum alpha;                                         // I
   } blend_equation;

   //gl 1.1 specific parts
   /*
      elements affecting programmable pipeline
   */

   GL11_MATERIAL_T material;

   GL11_LIGHTMODEL_T lightmodel;

   GL11_LIGHT_T lights[GL11_CONFIG_MAX_LIGHTS];

   /*
      Per texture unit state
   */

   GL11_TEXUNIT_T texunits[GL11_CONFIG_MAX_TEXTURE_UNITS];

   GL11_FOG_T fog;

   /*
      Current alpha function and reference

      Khronos state variable names:

      ALPHA_TEST_FUNC         ALWAYS
      ALPHA_TEST_REF          0

      Invariant:

      func in {NEVER, ALWAYS, LESS, LEQUAL, EQUAL, GEQUAL, GREATER, NOTEQUAL}
      0.0 <= ref <= 1.0
   */

   struct {
      GLenum func;
      float ref;
   } alpha_func;

   struct {
      bool fog;
      bool alpha_test;
      bool point_smooth;
      bool point_sprite;
      bool line_smooth;
   } caps_fragment;

   struct {
      GLenum fog;
   } hints_program;

   /*
      elements affecting fixed-function pipeline
   */

   /*
      Current clip plane coefficients

      Khronos state variable names:

      CLIP_PLANEi       (0, 0, 0, 0)

      Implementation notes:

      Note that the clip plane state variable name CLIP_PLANEi is used
      to refer to both whether a given clip plane is enabled and to the
      coefficients themselves. The former is returned by the standard
      get<type> functions and the latter by getClipPlane.

      Invariant:

      -
   */

   float planes[GL11_CONFIG_MAX_PLANES][4];

   GLenum shade_model;

   GLenum logic_op;

   GL11_POINT_PARAMS_T point_params;

   /*
      matrix stacks
   */


   /*
      Current matrix mode

      Khronos state variable names:

      MATRIX_MODE           MODELVIEW

      Invariants:

      matrix_mode in {MODELVIEW, PROJECTION, TEXTURE}
   */
   GLenum matrix_mode;

   GL11_MATRIX_STACK_T modelview;
   GL11_MATRIX_STACK_T projection;

   //cached items for instal_uniforms
   float current_modelview[16];
   float projection_modelview[16];
   float modelview_inv[16];
   float projected_clip_plane[4];

   //gl 2.0 specific
   GLXX_RENDERBUFFER_T *bound_renderbuffer;
   GLXX_FRAMEBUFFER_T *bound_framebuffer;
   void *program;

   GLfloat point_size;                                      // U

   GLenum client_active_texture;

   TWEAK_STATE_T  tweak_state;

   //client side stuff
   struct {
      GLint pack;
      GLint unpack;
   } alignment;

} GLXX_SERVER_STATE_T;

#define IS_GL_11(state) is_server_opengles_11(state)
#define IS_GL_20(state) is_server_opengles_20(state)

static inline bool is_server_opengles_11(GLXX_SERVER_STATE_T * state)
{
   return state && state->type == OPENGL_ES_11;
}

static inline bool is_server_opengles_20(GLXX_SERVER_STATE_T * state)
{
   return state && state->type == OPENGL_ES_20;
}

static inline bool glxx_check_gl_api(GLXX_SERVER_STATE_T *state, EGL_CONTEXT_TYPE_T type)
{
   switch (type)
   {
   case OPENGL_ES_11:
      return IS_GL_11(state);
   case OPENGL_ES_20:
      return IS_GL_20(state);
   case OPENGL_ES_ANY:
      return IS_GL_11(state) || IS_GL_20(state);
   default:
      UNREACHABLE();
      return false;
   }
}

extern void glxx_context_gl_lock(void);
extern void glxx_context_gl_unlock(void);

GLXX_SERVER_STATE_T *glxx_lock_server_state(EGL_CONTEXT_TYPE_T type);
void glxx_unlock_server_state(EGL_CONTEXT_TYPE_T type);

bool glxx_is_aligned(GLenum type, size_t value);

extern GLXX_TEXTURE_T *glxx_server_state_get_texture(GLXX_SERVER_STATE_T *state, GLenum target, GLboolean use_face);
extern void glxx_server_state_set_buffers(GLXX_SERVER_STATE_T *state, KHRN_IMAGE_T *draw, KHRN_IMAGE_T *read,
   KHRN_IMAGE_T *depth, KHRN_IMAGE_T *color_multi, KHRN_IMAGE_T *ds_multi,
   uint32_t config_depth_bits, uint32_t config_stencil_bits);

extern bool glxx_server_state_init(GLXX_SERVER_STATE_T *state, GLXX_SHARED_T *shared);
extern void glxx_server_state_term(void *p);
extern void glxx_server_state_flush(bool wait);

extern void glxx_server_set_stencil_func(GLXX_SERVER_STATE_T *state, GLenum face, GLenum func, GLint ref, GLuint mask);
extern void glxx_server_set_stencil_mask(GLXX_SERVER_STATE_T *state, GLenum face, GLuint mask);
extern void glxx_server_set_stencil_op(GLXX_SERVER_STATE_T *state, GLenum face, GLenum fail, GLenum zfail, GLenum zpass);

extern void glxx_server_set_blend_func(GLXX_SERVER_STATE_T *state, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

#ifdef DISABLE_OPTION_PARSING
extern void glxx_server_state_set_error(GLXX_SERVER_STATE_T *state, GLenum error);
#else
extern void glxx_server_state_set_error_ex(GLXX_SERVER_STATE_T *state, GLenum error, const char *func, unsigned int line);
#define glxx_server_state_set_error(a, b) glxx_server_state_set_error_ex(a, b, __func__, __LINE__)
#endif

extern GLboolean is_renderbuffer_internal(GLuint renderbuffer);
extern void bind_renderbuffer_internal(GLenum target, GLuint renderbuffer);
extern void delete_renderbuffers_internal(GLsizei n, const GLuint *renderbuffers);
extern void gen_renderbuffers_internal(GLsizei n, GLuint *renderbuffers);
extern void renderbuffer_storage_multisample_internal(GLenum target, GLsizei samples,
   GLenum internalformat, GLsizei width, GLsizei height);
extern void get_renderbuffer_parameteriv_internal(GLenum target, GLenum pname, GLint* params);
extern GLboolean is_framebuffer_internal(GLuint framebuffer);
extern void bind_framebuffer_internal(GLenum target, GLuint framebuffer);
extern void delete_framebuffers_internal(GLsizei n, const GLuint *framebuffers);
extern void gen_framebuffers_internal(GLsizei n, GLuint *framebuffers);
extern GLenum check_framebuffer_status_internal(GLenum target);
extern void framebuffer_texture2D_multisample_internal(GLenum target, GLenum attachment,
   GLenum textarget, GLuint texture, GLint level, GLsizei samples, bool only_attachment0);
extern void framebuffer_renderbuffer_internal(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern void get_framebuffer_attachment_parameteriv_internal(GLenum target, GLenum attachment, GLenum pname, GLint *params);
extern void generate_mipmap_internal(GLenum target);
extern void flush_internal(bool wait);
extern void get_core_revision_internal(GLsizei bufSize, GLubyte *revisionStr);

static inline KHRN_IMAGE_T *glxx_image_create_ms(KHRN_IMAGE_FORMAT_T format,
   uint32_t width, uint32_t height,
   KHRN_IMAGE_CREATE_FLAG_T flags,
   bool secure)
{
   const int ms_dim = (int)sqrt(GLXX_CONFIG_SAMPLES);
   return khrn_image_create(format,
      ms_dim * width, ms_dim * height,
      flags, secure);
}

extern void glxx_context_gl_create_lock(void);
