/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_SERVER_H
#define GLXX_SERVER_H

#include "gl_public_api.h"

#include "../common/khrn_int_process.h"
#include "../common/khrn_image_plane.h"
#include "../common/khrn_mem.h"
#include "../common/khrn_map.h"
#include "../common/khrn_options.h"
#include "../common/khrn_timeline.h"

#include "glxx_client.h"
#include "glxx_draw.h"
#include "glxx_texture.h"
#include "glxx_shared.h"
#include "glxx_buffer.h"
#include "glxx_draw.h"
#include "glxx_int_attrib.h"
#include "glxx_pixel_store.h"
#include "glxx_framebuffer.h"
#include "glxx_textures.h"
#include "glxx_query.h"
#include "glxx_image_unit.h"
#include "glxx_hw_framebuffer.h"
#include "glxx_hw_render_state.h"
#include "glxx_debug.h"
#include "glxx_rect.h"

#include "../gl11/gl11_matrix.h"
#include "../gl11/gl11_texunit.h"
#include "../gl11/gl11_shader_cache.h"

#include "../glsl/glsl_backend_cfg.h"

#include "../egl/egl_context_gl.h"

#include "libs/util/profile/profile.h"
#include "glxx_tf.h"

enum glxx_default_fb_type
{
   GLXX_READ_SURFACE = 0,
   GLXX_DRAW_SURFACE,
   GLXX_NUM_SURFACES
};

typedef struct
{
   uint32_t fragment;   /* For 1.1 */
   uint32_t f_enable;   /* GL11_LOGIC_M, GL11_UCLIP_M, GL11_FOG_M, GL11_AFUNC_M, GL11_ALPHA_TO_ONE, (GL11_POINTSMOOTH, GL11_LINESMOOTH in triangles mode) */
   uint32_t vertex;     /* For 1.1 */
   uint32_t v_enable;   /* GL11_LIGHT_M, GL11_RESCALE_NORMAL */
   uint32_t v_enable2;  /* Lighting */
   uint32_t texture[4]; /* For 1.1 */
} GL11_STATE_BITS_T;

typedef struct glxx_compute_render_state glxx_compute_render_state;

#define SET_MASKED(x, value, masked) do { (x) = ((x) & ~(masked)) | value; } while(0)
#define SET_MULTI(x, bits, enabled) do { if (enabled) x |= (bits); else x &= ~(bits); } while(0)
#define SET_INDIVIDUAL SET_MULTI

/* Attrib stuff which doesn't affect shader compilation */
typedef struct
{
   GLenum   gl_type;
   v3d_attr_type_t v3d_type;
   bool     is_signed;
   bool     norm;
   uint32_t size;
   bool     enabled;
   /* Offset within each VBO stride at which this attribute starts */
   uint32_t relative_offset;
   bool     is_int;

   uint32_t total_size; /* Size of element * number of elements */

   /* Only used for client-side attributes, when binding state is ignored! */
   uint32_t stride;
   uint32_t original_stride;
   const void *pointer;

   /* VBO which contains the attribute data (within the parent VAO) */
   uint32_t vbo_index;
} GLXX_ATTRIB_CONFIG_T;

/* Vertex Buffer Object. Contains data for one or more vertex attributes. */
typedef struct
{
   uint32_t offset;
   uint32_t stride;
   uint32_t divisor;
   GLXX_BUFFER_T *buffer;
} GLXX_VBO_BINDING_T;

// Generic attributes, ref table 6.22 in gles 3.0 spec
typedef struct
{
   union
   {
      float f[4];
      int32_t i[4];
      uint32_t u[4];
   };
   v3d_attr_type_t type;
   bool is_signed;
} GLXX_GENERIC_ATTRIBUTE_T;

typedef struct GLXX_VAO_T_ {
   int32_t                 name;
   bool                    enabled;
   GLXX_ATTRIB_CONFIG_T    attrib_config[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   GLXX_VBO_BINDING_T      vbos[GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS];
   GLXX_BUFFER_BINDING_T   element_array_binding;
   char                    *debug_label;
} GLXX_VAO_T;

typedef struct {
   bool enable;
   bool invert;
   GLclampf value;
} GLXX_SAMPLE_COVERAGE_T;

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
} GL11_LIGHTMODEL_T;

/*
   structure representing OpenGL ES 1.1 light

   note that position and spot.direction are stored in eye coordinates,
   computed using the current modelview matrix
*/

typedef struct {
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

typedef struct {
   float color[4];   /* [0:1]. Default (0, 0, 0, 0) */
   float density;    /* > 0. Default 1 */
   /* End points for linear */
   float start;      /* Default 0 */
   float end;        /* Default 1 */

   /*
      Coefficients that are useful inside fragment shading to calculate
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

typedef struct {
   GL11_CACHE_KEY_T shaderkey;

   // The GL11 shader cache is allocated dynamically to reduce footprint when not using ES1
   GL11_CACHE_ENTRY_T  *shader_cache;

   GL11_STATE_BITS_T statebits;
   GL11_MATERIAL_T   material;
   GL11_LIGHTMODEL_T lightmodel;
   GL11_LIGHT_T      lights   [GL11_CONFIG_MAX_LIGHTS];
   GL11_TEXUNIT_T    texunits [GL11_CONFIG_MAX_TEXTURE_UNITS];

   GLenum client_active_texture;

   bool point_sprite;

   GL11_FOG_T fog;

   struct {
      float ref;     /* [0:1] Default 0 */
   } alpha_func;

   struct {
      GLenum fog;
   } hints_program;

   /*
      Current clip plane coefficients

      Note that the clip plane state variable name CLIP_PLANEi is used
      to refer to both whether a given clip plane is enabled and to the
      coefficients themselves. The former is returned by the standard
      get<type> functions and the latter by getClipPlane.
   */
   float planes[GL11_CONFIG_MAX_PLANES][4];  /* Default (0,0,0,0) x 4 */

   GLenum   shade_model;

   GL11_POINT_PARAMS_T point_params;

   /*
      Current matrix mode

      One of {MODELVIEW, PROJECTION, TEXTURE, MATRIX_PALETTE_OES}
      Default MODELVIEW.
   */
   GLenum matrix_mode;

   GL11_MATRIX_STACK_T modelview;
   GL11_MATRIX_STACK_T projection;

   struct {
      GLenum perspective_correction;
      GLenum point_smooth;
      GLenum line_smooth;
   } hints;

   /* 0 <= current_palette_matrix < GL11_CONFIG_MAX_PALETTE_MATRICES_OES */
   uint32_t current_palette_matrix;
   float palette_matrices[GL11_CONFIG_MAX_PALETTE_MATRICES_OES][16];
   uint32_t palette_matrices_base_ptr;          /* Direct alias to pass into shaders */

   /* Current matrices at the top of the stack. */
   float current_modelview[16];
   float current_projection[16];
   /* cached items for install_uniforms */
   float projection_modelview[16];
   float modelview_inv[16];
   float projected_clip_plane[4];

   float line_width;  /* Always > 0, default == 1 */

   struct {
      bool projection_modelview;
      bool modelview_inv;
   } changed;
} GL11_STATE_T;

typedef struct
{
   khrn_render_state_set_t blend_color;
#if V3D_VER_AT_LEAST(4,0,2,0)
   khrn_render_state_set_t blend_enables; // Per-RT enables
#endif
   khrn_render_state_set_t blend_cfg;
   khrn_render_state_set_t color_write;
   khrn_render_state_set_t cfg;             // clockwise endo oversample ztest zmask
   khrn_render_state_set_t linewidth;       // also when line_smooth changes
   khrn_render_state_set_t polygon_offset;
   khrn_render_state_set_t viewport;        // scissor viewport depthrange
   khrn_render_state_set_t sample_state;    // Sample coverage and (v4.1+) sample mask
   khrn_render_state_set_t stuff;           // uniforms, shader, config, attribs etc. - See fast_draw_path()
   khrn_render_state_set_t stencil;
} glxx_dirty_set_t;

struct glxx_context_fences
{
   khrn_fence *fence; /* fence describing at any point all the current render
                           states using a certain context and previous jobs that
                           were issued on that context; by flushing this fence
                           and waiting for it to be signaled we would wait for
                           all the jobs issued on that context to complete; */

   khrn_fence *fence_to_depend_on; /* any future job issued on a certain context
                                        will depend on this fence; */
};

typedef enum
{
   GLXX_BLEND_EQN_ADD     = V3D_BLEND_EQN_ADD,
   GLXX_BLEND_EQN_SUB     = V3D_BLEND_EQN_SUB,
   GLXX_BLEND_EQN_RSUB    = V3D_BLEND_EQN_RSUB,
   GLXX_BLEND_EQN_MIN     = V3D_BLEND_EQN_MIN,
   GLXX_BLEND_EQN_MAX     = V3D_BLEND_EQN_MAX,
   GLXX_BLEND_EQN_MUL     = V3D_BLEND_EQN_MUL,
   GLXX_BLEND_EQN_SCREEN  = V3D_BLEND_EQN_SCREEN,
   GLXX_BLEND_EQN_DARKEN  = V3D_BLEND_EQN_DARKEN,
   GLXX_BLEND_EQN_LIGHTEN = V3D_BLEND_EQN_LIGHTEN,
   GLXX_BLEND_EQN_INVALID = V3D_BLEND_EQN_INVALID,

   GLXX_ADV_BLEND_EQN_BIT              = 0x20,
   GLXX_ADV_BLEND_EQN_MULTIPLY         = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_MULTIPLY,
   GLXX_ADV_BLEND_EQN_SCREEN           = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_SCREEN,
   GLXX_ADV_BLEND_EQN_OVERLAY          = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_OVERLAY,
   GLXX_ADV_BLEND_EQN_DARKEN           = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_DARKEN,
   GLXX_ADV_BLEND_EQN_LIGHTEN          = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_LIGHTEN,
   GLXX_ADV_BLEND_EQN_COLORDODGE       = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_COLORDODGE,
   GLXX_ADV_BLEND_EQN_COLORBURN        = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_COLORBURN,
   GLXX_ADV_BLEND_EQN_HARDLIGHT        = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_HARDLIGHT,
   GLXX_ADV_BLEND_EQN_SOFTLIGHT        = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_SOFTLIGHT,
   GLXX_ADV_BLEND_EQN_DIFFERENCE       = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_DIFFERENCE,
   GLXX_ADV_BLEND_EQN_EXCLUSION        = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_EXCLUSION,
   GLXX_ADV_BLEND_EQN_HSL_HUE          = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_HSL_HUE,
   GLXX_ADV_BLEND_EQN_HSL_SATURATION   = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_HSL_SATURATION,
   GLXX_ADV_BLEND_EQN_HSL_COLOR        = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_HSL_COLOR,
   GLXX_ADV_BLEND_EQN_HSL_LUMINOSITY   = GLXX_ADV_BLEND_EQN_BIT + GLSL_ADV_BLEND_HSL_LUMINOSITY
} glxx_blend_eqn_t;

typedef struct
{
   glxx_blend_eqn_t color_eqn;
   glxx_blend_eqn_t alpha_eqn;
   v3d_blend_mul_t  src_rgb;
   v3d_blend_mul_t  dst_rgb;
   v3d_blend_mul_t  src_alpha;
   v3d_blend_mul_t  dst_alpha;
} glxx_blend_cfg;

typedef struct
{
   unsigned int index; /* maximum index allowed across all enabled non-instance attributes */
   unsigned int instance; /* maximum instanced allowed across all enabled instance attributes */
   unsigned int base_instance; /* the base instance used when calculating max instance above */
} glxx_attribs_max;

struct GLXX_SERVER_STATE_T_
{
   /* The EGL Context this state is associated with. */
   const EGL_GL_CONTEXT_T *context;

   GLenum active_texture;

   GLenum error;

   struct {
      uint32_t backend;
   } statebits;

   GLXX_HW_RENDER_STATE_T    *current_render_state;   // If rs changes reissue frame-starting instructions
   glxx_compute_render_state *compute_render_state;

   struct {
#if V3D_VER_AT_LEAST(4,0,2,0)
      uint32_t             rt_enables; /* Bit i for RT i */
      glxx_blend_cfg       rt_cfgs[GLXX_MAX_RENDER_TARGETS];
#else
      bool enable;
      glxx_blend_cfg       cfg;
#endif
      bool advanced_coherent; // For state query only. No functional effect.
   } blend;

   /* RT i red/green/blue/alpha enable at bit 4*i + 0/1/2/3 */
   #define GLXX_SERVER_COLOR_WRITE_RED_BITS 0x11111111
   uint32_t color_write;

   glxx_dirty_set_t dirty;

   // Note that ELEMENT_ARRAY bindings are stored in the vao.
   GLXX_BUFFER_BINDING_T bound_buffer[GLXX_BUFTGT_CTX_COUNT];

   struct {
      // Table 6.25 buffer binding, start, size
      GLXX_INDEXED_BINDING_POINT_T binding_points[GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS];
   } uniform_block;

   struct {
      GLXX_INDEXED_BINDING_POINT_T binding_points[GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS];
   } atomic_counter;

   struct {
      GLXX_INDEXED_BINDING_POINT_T binding_points[GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS];
   } ssbo;

   GLXX_TEXTURES_T bound_texture[GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS];

   glxx_image_unit image_unit[GLXX_CONFIG_MAX_IMAGE_UNITS];

   /* OpenGL ES 3.0 specifies that framebuffers are not shared state.
      In later ES 3.0 it was implementation defined, and in early ES 2.0 it was shared state */
   uint32_t next_framebuffer;
   khrn_map framebuffers;

   /*
      Shared object structure

      Invariants:

      shared is a valid GLXX_SHARED_T pointer
   */
   GLXX_SHARED_T *shared;

   GLXX_TEXTURES_T default_textures;

   GLXX_FRAMEBUFFER_T *default_framebuffer[GLXX_NUM_SURFACES];

   GLboolean made_current; // have we ever been made current

   uint32_t name;
   uint64_t pid;

   struct {
      GLfloat color_value[4];
      GLfloat depth_value;
      GLint stencil_value;
   } clear;

   GLenum cull_mode;

   GLenum depth_func;
   bool depth_mask;

   float line_width;  /* Always > 0, default == 1 */

   float sample_shading_fraction;

   GLenum front_face;

   struct {
#if V3D_HAS_POLY_OFFSET_CLAMP
      float limit;
#endif
      float factor;
      float units;
   } polygon_offset;

   GLXX_SAMPLE_COVERAGE_T sample_coverage;

   struct {
      bool     enable;
      uint32_t mask[GLXX_CONFIG_MAX_SAMPLE_WORDS];
   } sample_mask;

   glxx_rect scissor;
   glxx_rect viewport; // width/height <= GLXX_CONFIG_MAX_VIEWPORT_SIZE (clamped on set)

   struct {
      // It would be nice to just call these near and far but the Windows
      // headers #define near and far...
      float z_near; // 0 <= z_near <= 1
      float z_far;  // 0 <= z_far  <= 1
   } depth_range;

   /* A cache of internal values derived from viewport & depth_range */
   struct {
      float dr_diff;
      float xscale;
      float yscale;
      float zscale;
      float zoffset;
   } vp_internal;

   struct {
      bool cull_face;
      bool polygon_offset_fill;
      bool scissor_test;
      bool dither;
      bool stencil_test;
      bool depth_test;
      bool sample_shading;

      // ES 3.0 only
      bool primitive_restart;    // Table 6.3: PRIMITIVE_RESTART_FIXED_INDEX
      bool rasterizer_discard;

      // ES 1.x only
      bool multisample;
   } caps;

   struct {
      GLenum generate_mipmap;

      GLenum fshader_derivative;
   } hints;

   struct {
      struct {
         GLenum func;
         GLint ref;
         GLuint mask;
      } front;

      //gl 2.0 specific
      struct {
         GLenum func;
         GLint ref;
         GLuint mask;
      } back;
   } stencil_func;

   struct stencil_mask {
      GLuint front;
      GLuint back;
   } stencil_mask;

   struct {
      struct {
         GLenum fail;
         GLenum zfail;
         GLenum zpass;
      } front;

      //gl 2.0 specific
      struct {
         GLenum fail;
         GLenum zfail;
         GLenum zpass;
      } back;
   } stencil_op;

   float blend_color[4];

   //gl 1.1 specific parts
   //  elements affecting programmable pipeline

   GL11_STATE_T gl11;

   // Per-texture-unit sampler object, if any
   GLXX_TEXTURE_SAMPLER_STATE_T *bound_sampler[GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS];

   /*
      elements affecting fixed-function pipeline
   */

   //gl 2.0 specific
   struct GL20_PROGRAM_T_ *current_program;

   // Table 6.12. Framebuffer Control
   GLXX_FRAMEBUFFER_T *bound_read_framebuffer;
   GLXX_FRAMEBUFFER_T *bound_draw_framebuffer;
   GLXX_RENDERBUFFER_T *bound_renderbuffer;

   /* Need a copy of the data which gets shared between client and server */
   GLXX_PIXEL_STORE_STATE_T pixel_store_state;

   //gl3.0 specific
   struct glxx_queries
   {
      struct glxx_queries_of_type
      {
         GLXX_QUERY_T *active; /* active query, on this context, for this type of
                                  query; NULL if there is no active query for
                                  this type, on this context */

         khrn_timeline timeline;
      } queries[GLXX_Q_COUNT];

      uint32_t next_name; /* next free name */
      khrn_map  objects;

   } queries;

   // Transform feedback objects
   struct {
      bool in_use;
      GLXX_TRANSFORM_FEEDBACK_T *bound; /* bound transform feedback object,
                                                 == default_tf by default
                                                 or if the user calls BindTranformFeedback(0) */
      GLXX_TRANSFORM_FEEDBACK_T *default_tf;
      uint32_t          next;
      khrn_map        objects;
   } transform_feedback;

   // Vertex array objects
   struct {
      GLXX_VAO_T        *default_vao; // Always valid
      GLXX_VAO_T        *bound;       // Always valid, ==default_vao if nothing explicitly bound
      uint32_t          next;
      khrn_map        objects;
   } vao;

   // Pipeline objects ES3.1 specific
   struct glxx_pipelines {
      struct GLXX_PIPELINE_T_   *bound;
      uint32_t                   next;
      khrn_map                 objects;
   } pipelines;

   struct {
      bool              counters_acquired;
   } perf_counters;

   GLXX_GENERIC_ATTRIBUTE_T generic_attrib[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];

   GLenum fill_mode;

   GLenum provoking_vtx;

#if GLXX_HAS_TNG
   unsigned num_patch_vertices;
#endif

   struct {
      float min[4];
      float max[4];
   } primitive_bounding_box;

   struct {
      unsigned draw_id;
   } debug;

   // State required for khr_debug
   GLXX_KHR_DEBUG_STATE_T khr_debug;

   struct glxx_context_fences fences;
};

#if WANT_PROFILING
extern profile_timer* glxx_server_locked_timer;
extern uint32_t glxx_server_lock_time;
#endif

#define IS_GL_11(state) (!!egl_context_gl_api(state->context, OPENGL_ES_11))

static inline GLXX_SERVER_STATE_T *glxx_lock_server_state_with_changed(
   uint32_t api, bool changed)
{
   GLXX_SERVER_STATE_T *state = NULL;
   bool locked = false;
#if WANT_PROFILING
   uint32_t lock_time = profile_get_tick_count();
#endif

   if (!egl_context_gl_lock())
      goto end;
   locked = true;

   state = egl_context_gl_server_state(NULL);
   if (!state) goto end;

   if (!egl_context_gl_api(state->context, api))
   {
      state = NULL;
      goto end;
   }

   if (changed)
      state->dirty.stuff = KHRN_RENDER_STATE_SET_ALL;

#if WANT_PROFILING
   glxx_server_lock_time = lock_time; // store lock_time inside the lock
   if (!glxx_server_locked_timer)
      glxx_server_locked_timer = profile_timer_new("GL", __FILE__, "gl*", "");
#endif

end:
   if (!state && locked)
      egl_context_gl_unlock();
   return state;
}

static inline GLXX_SERVER_STATE_T *glxx_lock_server_state(uint32_t api)
{
   return glxx_lock_server_state_with_changed(api, /*changed=*/true);
}

static inline GLXX_SERVER_STATE_T *glxx_lock_server_state_unchanged(uint32_t api)
{
   return glxx_lock_server_state_with_changed(api, /*changed=*/false);
}

static inline void glxx_unlock_server_state(void)
{
#if WANT_PROFILING
   uint32_t lock_time = glxx_server_lock_time; // load lock_time inside the lock
#endif
   egl_context_gl_unlock();
#if WANT_PROFILING
   profile_timer_add_mt(glxx_server_locked_timer, profile_get_tick_count() - lock_time);
#endif
}

#define GLXX_STATE_OFFSET(x) (offsetof(GLXX_SERVER_STATE_T, x)/4)

/* returns the server active texture for that target;
 * sets error to GL_INVALID_ENUM if target is not a supported target */
extern GLXX_TEXTURE_T *glxx_server_state_get_texture(GLXX_SERVER_STATE_T *state,
      GLenum target);

/* returns the server active texture for a valid texture target */
extern GLXX_TEXTURE_T* glxx_server_get_active_texture(const GLXX_SERVER_STATE_T
      *state, enum glxx_tex_target target);

typedef enum
{
   enumify(GL_READ_FRAMEBUFFER),
   enumify(GL_DRAW_FRAMEBUFFER),
   enumify(GL_FRAMEBUFFER),
}glxx_fb_target_t;

extern GLXX_FRAMEBUFFER_T *glxx_server_get_bound_fb(const GLXX_SERVER_STATE_T *state,
      glxx_fb_target_t fb_target);

extern bool glxx_server_state_init(GLXX_SERVER_STATE_T *state, GLXX_SHARED_T *shared);
extern void glxx_server_state_destroy(GLXX_SERVER_STATE_T *state);

/* flush the specified server state; if wait = true; wait for all the
 * operations on that context to finish (= glFinish) */
extern void glxx_server_state_flush(GLXX_SERVER_STATE_T *state, bool wait);

extern GLXX_FRAMEBUFFER_T* glxx_server_state_get_framebuffer(
      GLXX_SERVER_STATE_T *state, uint32_t fb_id, bool create);

extern void glxx_server_state_delete_framebuffer(GLXX_SERVER_STATE_T *state, uint32_t fb_id);


extern void glxx_server_state_set_error_ex(GLXX_SERVER_STATE_T *state, GLenum error, const char *func, const char *file, int line);
#define glxx_server_state_set_error(a, b) glxx_server_state_set_error_ex(a, b, __func__, __FILE__, __LINE__)

static inline void glxx_set_error_api(uint32_t api, GLenum error)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(api);
   if (state != NULL)
   {
      glxx_server_state_set_error(state, error);
      glxx_unlock_server_state();
   }
}

extern bool gl11_server_state_init(GLXX_SERVER_STATE_T *state, GLXX_SHARED_T *shared);
extern void gl11_server_state_destroy(GLXX_SERVER_STATE_T *state);

extern bool gl20_server_state_init(GLXX_SERVER_STATE_T *state, GLXX_SHARED_T *shared);

/* maps "size" bytes from buffer, starting from offset; if size == SIZE_MAX,*
 * the buffer is mapped starting from offset till the end of the buffer */
extern void* glxx_map_buffer_range(GLXX_SERVER_STATE_T *state, GLenum target,
      size_t offset, size_t size, GLbitfield access);

extern void glxx_get_buffer_pointerv(GLenum target, GLenum pname, GLvoid ** params);

extern GLboolean glxx_unmap_buffer(GLXX_SERVER_STATE_T *state, GLenum target);

extern bool glxx_server_queries_install(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs);

//! Set dirty flags for any state that needs to be flushed again to this render-state
extern void glxx_server_invalidate_for_render_state(GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs);

extern bool glxx_server_state_add_fence_to_depend_on(GLXX_SERVER_STATE_T *state,
      const khrn_fence *fence);

extern void glxx_server_attach_surfaces(GLXX_SERVER_STATE_T *state,
      EGL_SURFACE_T *read, EGL_SURFACE_T *draw);
extern void glxx_server_detach_surfaces(GLXX_SERVER_STATE_T *state);

extern bool gl11_get_pointerv(GLXX_SERVER_STATE_T *state, GLenum pname, void **params);
extern bool glxx_get_pointerv(GLXX_SERVER_STATE_T *state, GLenum pname, void **params);

// Program object
extern struct GL20_PROGRAM_T_ *glxx_server_get_active_program(GLXX_SERVER_STATE_T *state);

// Query blend propeties
extern bool glxx_advanced_blend_eqn_set(const GLXX_SERVER_STATE_T *state);
extern uint32_t glxx_advanced_blend_eqn(const GLXX_SERVER_STATE_T *state);

#endif
