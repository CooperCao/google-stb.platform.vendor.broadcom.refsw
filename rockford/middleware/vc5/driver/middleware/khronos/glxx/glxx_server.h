/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 1.1 and 2.0 server-side state structure declaration.
=============================================================================*/

#ifndef GLXX_SERVER_H
#define GLXX_SERVER_H

#include "interface/khronos/glxx/gl_public_api.h"

#include "interface/khronos/common/khrn_int_process.h"
#include "interface/khronos/glxx/glxx_client.h"

#include "middleware/khronos/common/khrn_image_plane.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "middleware/khronos/common/khrn_map.h"
#include "middleware/khronos/glxx/glxx_draw.h"
#include "middleware/khronos/common/khrn_stats.h"
#include "middleware/khronos/glxx/glxx_texture.h"
#include "middleware/khronos/glxx/glxx_shared.h"
#include "middleware/khronos/glxx/glxx_buffer.h"
#include "middleware/khronos/glxx/glxx_shader_cache.h"
#include "middleware/khronos/glxx/glxx_draw.h"

#include "interface/khronos/glxx/glxx_int_attrib.h"
#include "middleware/khronos/gl11/gl11_matrix.h"
#include "middleware/khronos/gl11/gl11_texunit.h"
#include "middleware/khronos/gl11/gl11_shader_cache.h"

#include "interface/khronos/common/khrn_options.h"
#include "middleware/khronos/glxx/glxx_pixel_store.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"
#include "middleware/khronos/egl/egl_context_gl.h"
#include "middleware/khronos/glxx/glxx_textures.h"
#include "middleware/khronos/glxx/glxx_query.h"

#include "middleware/khronos/glxx/glxx_hw_framebuffer.h"
#include "middleware/khronos/common/khrn_timeline.h"
#include "middleware/khronos/glxx/glxx_hw_render_state.h"
#include "middleware/khronos/ext/gl_khr_debug.h"

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

typedef struct glxx_compute_render_state_s glxx_compute_render_state;

#define SET_MASKED(x, value, masked) do { (x) = ((x) & ~(masked)) | value; } while(0)
#define SET_MULTI(x, bits, enabled) do { if (enabled) x |= (bits); else x &= ~(bits); } while(0)
#define SET_INDIVIDUAL SET_MULTI

/* Attrib stuff which doesn't affect shader compilation */
typedef struct
{
   GLenum   type;
   bool     norm;
   uint32_t size;
   bool     enabled;
   /* Offset within each VBO stride at which this attribute starts */
   uint32_t relative_offset;
   bool is_int;

   uint32_t total_size; /* Size of element * number of elements */

   /* VBO which contains the attribute data (within the parent VAO) */
   uint32_t vbo_index;
} GLXX_ATTRIB_CONFIG_T;

/* Vertex Buffer Object. Contains data for one or more vertex attributes. */
typedef struct
{
   uint32_t offset;
   uint32_t stride;
   uint32_t divisor;

   /* stride actually passed into glVertexAttrib*Pointer (may be zero) */
   uint32_t original_stride;

   const void *pointer;
   GLXX_BUFFER_T *buffer;
} GLXX_VBO_BINDING_T;

// Generic attributes, ref table 6.22 in gles 3.0 spec
typedef struct
{
   float     value[4];
   uint32_t  value_int[4];
   GLenum    type;
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



// Transform feedback is a private struct
struct GLXX_TRANSFORM_FEEDBACK_T_;

// TODO: Just include it here?
struct GL20_PROGRAM_T_;

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

#if GL_OES_matrix_palette
   /* 0 <= current_palette_matrix < GL11_CONFIG_MAX_PALETTE_MATRICES_OES */
   uint32_t current_palette_matrix;
   float palette_matrices[GL11_CONFIG_MAX_PALETTE_MATRICES_OES][16];
   uint32_t palette_matrices_base_ptr;          /* Direct alias to pass into shaders */
#endif

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

typedef struct GLXX_TRANSFORM_FEEDBACK_T_
{
   int32_t                       name;
   uint32_t                      flags;
   GLenum                        primitive_mode;
   GLXX_BUFFER_BINDING_T         generic_buffer_binding;
   GLXX_INDEXED_BINDING_POINT_T  binding_points[V3D_MAX_TF_BUFFERS]; // Table 6.24 buffer binding, start, size
   struct GL20_PROGRAM_T_        *program;                           // Program used by the tf
   uint32_t                      stream_position[V3D_MAX_TF_BUFFERS];
   char                          *debug_label;
} GLXX_TRANSFORM_FEEDBACK_T;

typedef struct{
   bool r;
   bool g;
   bool b;
   bool a;
} glxx_color_write_t;

typedef struct
{
   khrn_render_state_set_t blend_color;
   khrn_render_state_set_t blend_mode;
   khrn_render_state_set_t color_write;
   khrn_render_state_set_t cfg;             // clockwise endo oversample ztest zmask
   khrn_render_state_set_t linewidth;       // also when line_smooth changes
   khrn_render_state_set_t polygon_offset;
   khrn_render_state_set_t viewport;        // scissor viewport depthrange
   khrn_render_state_set_t sample_coverage;
   khrn_render_state_set_t stuff;           // uniforms, shader, config, attribs etc. - See fast_draw_path()
   khrn_render_state_set_t stencil;
} glxx_dirty_set_t;

struct glxx_context_fences
{
   KHRN_FENCE_T *fence; /* fence describing at any point all the current render
                           states using a certain context and previous jobs that
                           were issued on that context; by flushing this fence
                           and waiting for it to be signaled we would wait for
                           all the jobs issued on that context to complete; */

   KHRN_FENCE_T *fence_to_depend_on; /* any future job issued on a certain context
                                        will depend on this fence; */
};

struct GLXX_SERVER_STATE_T_
{
   /* The EGL Context this state is associated with. */
   const EGL_GL_CONTEXT_T *context;

   GLenum active_texture;

   GLenum error;

   GLXX_LINK_RESULT_KEY_T shaderkey_common;

   struct {
      uint32_t backend;
   } statebits;

   GLXX_HW_RENDER_STATE_T *current_render_state;   // If rs changes reissue frame-starting instructions
   glxx_compute_render_state* compute_render_state;

   struct {
      v3d_blend_eqn_t color_eqn;
      v3d_blend_eqn_t alpha_eqn;
      v3d_blend_mul_t src_rgb;
      v3d_blend_mul_t dst_rgb;
      v3d_blend_mul_t src_alpha;
      v3d_blend_mul_t dst_alpha;

      bool enable;
   } blend;


   glxx_color_write_t color_write;

   glxx_dirty_set_t dirty;

   // Contains attribs_max calculated in the previous draw call for server-side attributes
   // Valid if any bits in dirty.stuff are clear
   GLXX_ATTRIBS_MAX cached_server_attribs_max;

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

   GLXX_TEXTURES_T bound_texture[GLXX_CONFIG_MAX_TEXTURE_UNITS];

   /* OpenGL ES 3.0 specifies that framebuffers are not shared state.
      In later ES 3.0 it was implementation defined, and in early ES 2.0 it was shared state */
   uint32_t next_framebuffer;
   KHRN_MAP_T framebuffers;

   /*
      Shared object structure

      Invariants:

      shared is a valid GLXX_SHARED_T pointer
   */
   GLXX_SHARED_T *shared;

   GLXX_TEXTURES_T default_textures;

   GLXX_FRAMEBUFFER_T *default_framebuffer[GLXX_NUM_SURFACES];

   GLboolean made_current; // have we ever been made current

   uint32_t *temp_palette;     // Hack to get paletted textures to work across multiple calls

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

   GLfloat line_width;  /* Always > 0, default == 1 */

   GLenum front_face;

   struct {
      GLfloat factor;
      GLfloat units;
   } polygon_offset;

   GLXX_SAMPLE_COVERAGE_T sample_coverage;

   struct {
      bool     enable;
      uint32_t mask[GLXX_CONFIG_MAX_SAMPLE_WORDS];
   } sample_mask;

   struct {
      GLint x;
      GLint y;
      GLsizei width;
      GLsizei height;
   } scissor;

   /*
      Current viewport in GL and internal formats

      DEPTH_RANGE             (0, 1)
      VIEWPORT                (0, 0, 0, 0)   note set at first use of context with surface
   */
   struct {
      GLint x;
      GLint y;
      GLsizei width;        //       0 <= width <= GLXX_CONFIG_MAX_VIEWPORT_SIZE
      GLsizei height;       //       0 <= height <= GLXX_CONFIG_MAX_VIEWPORT_SIZE
      GLclampf vp_near;     //       0 <= near <= 1
      GLclampf vp_far;      //       0 <= far  <= 1

      /* A cache of internal derived values */
      float internal_dr_near;
      float internal_dr_far;
      float internal_dr_diff;
      float internal_xscale;
      float internal_yscale;
      float internal_zscale;
      float internal_wscale;
   } viewport;

   struct {
      bool cull_face;
      bool polygon_offset_fill;
      bool scissor_test;
      bool dither;
      bool stencil_test;
      bool depth_test;

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

   GLfloat blend_color[4];

   //gl 1.1 specific parts
   //  elements affecting programmable pipeline

   GL11_STATE_T gl11;

   // HW Uniform values associated with currently installed texture
   GLXX_TEXTURE_UNIF_T texture_unif[GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS];

   // Per-texture-unit sampler object, if any
   GLXX_TEXTURE_SAMPLER_STATE_T *bound_sampler[GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS];

   /*
      elements affecting fixed-function pipeline
   */

   //gl 2.0 specific
   struct GL20_PROGRAM_T_ *program;

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
         enum glxx_query_type type;
         GLXX_QUERY_T *active; /* this can be NULL or based on query->target,
                                 any_samples_passed or
                                 any_samples_passed_conservative */

         KHRN_TIMELINE_T timeline;
      } queries[GLXX_Q_COUNT];

      uint32_t next_name; /* next free name */
      KHRN_MAP_T  objects;

   } queries;

   // Transform feedback objects
   struct {
      struct GLXX_TRANSFORM_FEEDBACK_T_ *default_obj;
      uint32_t          next;
      struct GLXX_TRANSFORM_FEEDBACK_T_ *binding;    // TRANSFORM_FEEDBACK_BINDING Table 6.5
      KHRN_MAP_T        objects;
      bool              in_use;     // true when we have transform feedback active and not paused
   } transform_feedback;

   // Vertex array objects
   struct {
      GLXX_VAO_T        *default_vao; // Always valid
      GLXX_VAO_T        *bound;       // Always valid, ==default_vao if nothing explicitly bound
      uint32_t          next;
      KHRN_MAP_T        objects;
   } vao;

   struct
   {
      bool              counters_acquired;
   } perf_counters;

   GLXX_GENERIC_ATTRIBUTE_T generic_attrib[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];

   GLenum fill_mode;

#if GL_BRCM_provoking_vertex
   GLenum provoking_vtx;
#endif

   struct {
      unsigned draw_id;
   } debug;

   // State required for khr_debug
   GLXX_KHR_DEBUG_STATE_T khr_debug;

   struct glxx_context_fences fences;
};


#define IS_GL_11(state) (!!egl_context_gl_api(state->context, OPENGL_ES_11))
#define IS_GL_30(state) (!!egl_context_gl_api(state->context, OPENGL_ES_30))
#define IS_GL_31(state) (!!egl_context_gl_api(state->context, OPENGL_ES_31))

static inline GLXX_SERVER_STATE_T *glxx_lock_server_state(uint32_t api,
      bool changed)
{
   GLXX_SERVER_STATE_T *state = NULL;
   bool locked = false;

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

end:
   if (!state && locked)
      egl_context_gl_unlock();
   return state;
}

static inline void glxx_unlock_server_state(void)
{
   egl_context_gl_unlock();
}

#define GLXX_LOCK_SERVER_STATE_UNCHANGED() glxx_lock_server_state(OPENGL_ES_ANY, false)
#define GLXX_LOCK_SERVER_STATE()           glxx_lock_server_state(OPENGL_ES_ANY, true)
#define GLXX_UNLOCK_SERVER_STATE()         glxx_unlock_server_state()

#define GLXX_FORCE_UNLOCK_SERVER_STATE()   khrn_process_glxx_force_unlock()

#define GLXX_LOCK_SERVER_STATE_API_UNCHANGED(api) glxx_lock_server_state(api, false)
#define GLXX_LOCK_SERVER_STATE_API(api)    glxx_lock_server_state(api, true)
#define GLXX_UNLOCK_SERVER_STATE_API(api)  glxx_unlock_server_state()

#define GL11_LOCK_SERVER_STATE_UNCHANGED() glxx_lock_server_state(OPENGL_ES_11, false)
#define GL11_LOCK_SERVER_STATE()           glxx_lock_server_state(OPENGL_ES_11, true)
#define GL11_UNLOCK_SERVER_STATE()         glxx_unlock_server_state()

#define GL20_LOCK_SERVER_STATE_UNCHANGED() glxx_lock_server_state(OPENGL_ES_30 | OPENGL_ES_31, false)
#define GL20_LOCK_SERVER_STATE()           glxx_lock_server_state(OPENGL_ES_30 | OPENGL_ES_31, true)
#define GL20_UNLOCK_SERVER_STATE()         glxx_unlock_server_state()

#define GL30_LOCK_SERVER_STATE_UNCHANGED() glxx_lock_server_state(OPENGL_ES_30 | OPENGL_ES_31, false)
#define GL30_LOCK_SERVER_STATE()           glxx_lock_server_state(OPENGL_ES_30 | OPENGL_ES_31, true)
#define GL30_UNLOCK_SERVER_STATE()         glxx_unlock_server_state()

#define GL31_LOCK_SERVER_STATE_UNCHANGED() glxx_lock_server_state(OPENGL_ES_31, false)
#define GL31_LOCK_SERVER_STATE()           glxx_lock_server_state(OPENGL_ES_31, true)
#define GL31_UNLOCK_SERVER_STATE()         glxx_unlock_server_state()

#define GLXX_STATE_OFFSET(x) (offsetof(GLXX_SERVER_STATE_T, x)/4)

/* returns the server active texture for that target;
 * sets error to GL_INVALID_VALUE if target is not a supported target */
extern GLXX_TEXTURE_T *glxx_server_state_get_texture(GLXX_SERVER_STATE_T *state,
      GLenum target, bool use_face);

/* returns the server active texture for a valid texture target (validate texture target) */
extern GLXX_TEXTURE_T* glxx_server_get_active_texture(const GLXX_SERVER_STATE_T
      *state, enum glxx_tex_target target);

typedef enum
{
   enumify(GL_READ_FRAMEBUFFER),
   enumify(GL_DRAW_FRAMEBUFFER),
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


#ifdef DISABLE_OPTION_PARSING
extern void glxx_server_state_set_error(GLXX_SERVER_STATE_T *state, GLenum error);
#else
extern void glxx_server_state_set_error_ex(GLXX_SERVER_STATE_T *state, GLenum error, const char *func, const char *file, int line);
#define glxx_server_state_set_error(a, b) glxx_server_state_set_error_ex(a, b, __func__, __FILE__, __LINE__)
#endif

static inline void glxx_set_error_api(uint32_t api, GLenum error)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_API(api);
   if (state != NULL)
   {
      glxx_server_state_set_error(state, error);
      GLXX_UNLOCK_SERVER_STATE();
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

extern bool glxx_server_active_queries_install(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs);

//! Set dirty flags for any state that needs to be flushed again to this render-state
extern void glxx_server_invalidate_for_render_state(GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs);

extern bool glxx_server_state_add_fence_to_depend_on(GLXX_SERVER_STATE_T *state,
      const KHRN_FENCE_T *fence);

extern void glxx_server_attach_surfaces(GLXX_SERVER_STATE_T *state,
      EGL_SURFACE_T *read, EGL_SURFACE_T *draw);
extern void glxx_server_detach_surfaces(GLXX_SERVER_STATE_T *state);

#endif
