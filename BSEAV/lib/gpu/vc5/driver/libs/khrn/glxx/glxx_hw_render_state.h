/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef HW_RENDER_STATE_H
#define HW_RENDER_STATE_H

#include "gl_public_api.h"
#include "glxx_bufstate.h"
#include "glxx_render_state.h"
#include "glxx_hw_framebuffer.h"
#include "glxx_query.h"
#include "glxx_shader_cache.h"
#include "glxx_draw.h"
#include "glxx_ez.h"
#include "glxx_tf.h"

typedef struct {
   uint32_t color;
   float    depth;
   uint8_t  stencil;
   bool     colorClear;
   bool     depthClear;
   bool     stencilClear;
   uint32_t colorMask;
   uint8_t  stencilMask;
} GLXX_HW_CLEAR_T;

struct GLXX_QUERY_;

#define GLXX_RS_MAX_BLITS 8

typedef struct
{
   GLXX_HW_FRAMEBUFFER_T dst_fb;

   bool color;
   /* if color = true, for all set bits b in color_draw_to_buf, blit from
    * src_fb.color[color_read_buffer] to dst_fb.color[b].
    * src_fb is the GLXX_HW_FRAMEBUFFER_T where this struct is used.
    */
   unsigned color_read_buffer;
   uint32_t color_draw_to_buf;

   /* if depth = true, blit from src_fb.depth to dst_fb.depth */
   bool depth;
} GLXX_BLIT_T;

// enable multicore binning if we potentially have more than 1 core
#ifndef GLXX_MULTICORE_BIN_ENABLED
#define GLXX_MULTICORE_BIN_ENABLED (V3D_MAX_CORES > 1)
#endif

// max number of draw records to track for multicore binning
#define GLXX_MAX_CL_RECORDS (GLXX_MULTICORE_BIN_ENABLED?32:1)

// attempt to split bin-jobs with cost greater than this
// (currently measured in cycles)
#define GLXX_MULTICORE_BIN_SPLIT_THRESHOLD 32768

typedef enum
{
   // fixed sized states
#if V3D_VER_AT_LEAST(4,0,2,0)
   GLXX_CL_STATE_BLEND_ENABLES,
#else
   GLXX_CL_STATE_BLEND_CFG,
#endif
   GLXX_CL_STATE_BLEND_COLOR,
   GLXX_CL_STATE_COLOR_WRITE,
   GLXX_CL_STATE_CFG,
   GLXX_CL_STATE_LINE_WIDTH,
   GLXX_CL_STATE_POLYGON_OFFSET,
   GLXX_CL_STATE_STENCIL,
   GLXX_CL_STATE_SAMPLE_STATE,
   GLXX_CL_STATE_OCCLUSION_QUERY,

   GLXX_CL_STATE_NUM_FIXED_SIZE,

   // variable sized states
   GLXX_CL_STATE_VIEWPORT = GLXX_CL_STATE_NUM_FIXED_SIZE,
   GLXX_CL_STATE_FLAT_SHADING_FLAGS,
#if V3D_VER_AT_LEAST(4,1,34,0)
   GLXX_CL_STATE_NOPERSPECTIVE_FLAGS,
#endif
   GLXX_CL_STATE_CENTROID_FLAGS,
#if V3D_VER_AT_LEAST(4,0,2,0)
   GLXX_CL_STATE_BLEND_CFG,
#endif

   GLXX_CL_STATE_NUM
} glxx_cl_state_t;

static const uint8_t GLXX_CL_STATE_SIZE[GLXX_CL_STATE_NUM] =
{
    // fixed sized states
#if V3D_VER_AT_LEAST(4,0,2,0)
   [GLXX_CL_STATE_BLEND_ENABLES]    = V3D_CL_BLEND_ENABLES_SIZE,
#else
   [GLXX_CL_STATE_BLEND_CFG]        = V3D_CL_BLEND_CFG_SIZE,
#endif
   [GLXX_CL_STATE_BLEND_COLOR]      = V3D_CL_BLEND_CCOLOR_SIZE,
   [GLXX_CL_STATE_COLOR_WRITE]      = V3D_CL_COLOR_WMASKS_SIZE,
   [GLXX_CL_STATE_CFG]              = V3D_CL_CFG_BITS_SIZE,
   [GLXX_CL_STATE_LINE_WIDTH]       = V3D_CL_LINE_WIDTH_SIZE,
   [GLXX_CL_STATE_POLYGON_OFFSET]   = V3D_CL_DEPTH_OFFSET_SIZE,
   [GLXX_CL_STATE_STENCIL]          = (V3D_CL_STENCIL_CFG_SIZE * 2),
   [GLXX_CL_STATE_SAMPLE_STATE]     = V3D_CL_SAMPLE_STATE_SIZE,
   [GLXX_CL_STATE_OCCLUSION_QUERY]  = V3D_CL_OCCLUSION_QUERY_COUNTER_ENABLE_SIZE,

   // variable sized states (these are maximums)
   [GLXX_CL_STATE_VIEWPORT]            = (V3D_CL_CLIP_SIZE + V3D_CL_CLIPPER_XY_SIZE + V3D_CL_VIEWPORT_OFFSET_SIZE +
                                          V3D_CL_CLIPPER_Z_SIZE + V3D_CL_CLIPZ_SIZE),
   [GLXX_CL_STATE_FLAT_SHADING_FLAGS]  = V3D_MAX_VARY_FLAG_WORDS * V3D_CL_VARY_FLAGS_SIZE,
   [GLXX_CL_STATE_CENTROID_FLAGS]      = V3D_MAX_VARY_FLAG_WORDS * V3D_CL_VARY_FLAGS_SIZE,
#if V3D_VER_AT_LEAST(4,1,34,0)
   [GLXX_CL_STATE_NOPERSPECTIVE_FLAGS] = V3D_MAX_VARY_FLAG_WORDS * V3D_CL_VARY_FLAGS_SIZE,
#endif
#if V3D_VER_AT_LEAST(4,0,2,0)
   [GLXX_CL_STATE_BLEND_CFG]           = GLXX_MAX_RENDER_TARGETS * V3D_CL_BLEND_CFG_SIZE,
#endif
};

// flag which states are optionally set
enum
{
   GLXX_CL_STATE_OPTIONAL = (1 << GLXX_CL_STATE_BLEND_COLOR)
                          | (1 << GLXX_CL_STATE_BLEND_CFG)
                          | (1 << GLXX_CL_STATE_STENCIL)
                          | (1 << GLXX_CL_STATE_OCCLUSION_QUERY)  // occlusion queries are disabled in create_bin_cl, so this is optional
};

typedef struct
{
   // cumulative binning cycle count estimate of all draw calls up to start_sub_addr in control list
   uint64_t bin_cost_cumulative;
   uint8_t* ret_sub_ptr;
   v3d_addr_t start_sub_addr;
   bool z_prepass_stopped;

   // ptrs/sizes refer to state that should be applied before resuming at start_sub_addr
   uint8_t *ptrs[GLXX_CL_STATE_NUM];
   uint16_t sizes[GLXX_CL_STATE_NUM - GLXX_CL_STATE_NUM_FIXED_SIZE];

#ifndef NDEBUG
   glxx_cl_state_t in_begin;
#endif
} GLXX_CL_RECORD_T;

//! Check all expected states have been captured.
extern bool glxx_cl_record_validate(GLXX_CL_RECORD_T *record);

//! Apply cl record to current clist.
extern bool glxx_cl_record_apply(GLXX_CL_RECORD_T *record, khrn_fmem *fmem);

static_assrt(offsetof(glxx_render_state, fmem) == 0);

typedef struct glxx_hw_render_state
{
   // Inherit from glxx_render_state.
   union
   {
      glxx_render_state base;
      khrn_fmem fmem;
   };

   v3d_addr_t clist_start;

   // Renderstate needs a copy of the hw fb as it was at the time renderstate was prepared,
   // as the framebuffer can be changed after that before the renderstate gets flushed.
   GLXX_HW_FRAMEBUFFER_T   installed_fb;

   // Context that owns this render-state.
   GLXX_SERVER_STATE_T     *server_state;

   /* If color_load_from_ms[i] then RT i will be loaded from
    * installed_fb.color_ms[i]. Otherwise RT i will be loaded from
    * installed_fb.color[i]. */
   bool                    color_load_from_ms[GLXX_MAX_RENDER_TARGETS];

   /* If color_discard_ms then we will only store to installed_fb.color[].
    * Otherwise we will store to both installed_fb.color[] *and*
    * installed_fb.color_ms[]. */
   bool                    color_discard_ms;

   glxx_bufstate_t         color_buffer_state[GLXX_MAX_RENDER_TARGETS];

   /* Even if we have a packed depth/stencil buffer, we have completely
    * independent bufstates for depth and stencil. We can do this because the
    * hardware is flexible enough to be able to load/store depth & stencil
    * independently even if they are packed into the same physical buffer. */
   glxx_bufstate_t         depth_buffer_state;
   glxx_bufstate_t         stencil_buffer_state;

   /* Store the clear colors in raw 32-bit format */
   uint32_t                clear_colors[GLXX_MAX_RENDER_TARGETS][4];

   float                   depth_value;
   uint8_t                 stencil_value;
   bool                    dither;

   // flags previously set on this render-state
   uint32_t                prev_flat_shading_flags[V3D_MAX_VARY_FLAG_WORDS];
   uint32_t                prev_centroid_flags[V3D_MAX_VARY_FLAG_WORDS];
   bool                    flat_shading_flags_set;
   bool                    centroid_flags_set;
#if V3D_VER_AT_LEAST(4,1,34,0)
   uint32_t                prev_noperspective_flags[V3D_MAX_VARY_FLAG_WORDS];
   bool                    noperspective_flags_set;
#endif

   GLXX_BLIT_T             tlb_blits[GLXX_RS_MAX_BLITS];
   unsigned                num_blits;

#if !V3D_VER_AT_LEAST(3,3,0,0)
   bool                    workaround_gfxh_1313;
#endif

   bool                    z_prepass_allowed;         // can enable z-prepass for this control list
   bool                    z_prepass_started;         // z-prepass function is now set, using z-prepass for at least some of this control list
   bool                    z_prepass_stopped;         // z-prepass was started and stopped
   int8_t                  z_prepass_dir;
   unsigned                num_z_prepass_bins;


   /* last query of this type that was started on this rs and the instance for
    * that query at the point when the query was started on this render state ;
    * if query = NULL, there is no query of this type in progress on this rs */
   glxx_instanced_query_t last_started_query[GLXX_Q_COUNT];

   /* If transform feedback is used in renderstate, we can no longer
    * discard fmem on clears.
    */
   struct
   {
      bool                    used;
      /* tf started_count = number of 'Draw Flush and Count' control items +
       * number of 'Prim Counts Feedback' control items.
       * 'Prim Counts Feedback' control items that don't do any feedback
       * do not count towards tf started count: i.e restore of TF buffer state
       * or PCF when feedback address is zero; */
      unsigned                started_count;
      unsigned                waited_count;   // Number of transform feedbacks waited for
      v3d_cache_ops           done_cache_ops; // Cache ops inserted in CL since last wait. Valid iff waited_count >0.

      GLXX_TRANSFORM_FEEDBACK_T *last_used;
      khrn_resource *res;
      bool enabled;
   }tf;

   GLXX_EZ_STATE_T         ez;

   uint64_t cl_record_threshold;   // bin-cycle count threshold to create a new entry
   uint64_t cl_record_remaining;   // bin-cycle count remaining before creating new record
   unsigned num_cl_records;        // num entries written to cl_records
   GLXX_CL_RECORD_T cl_records[GLXX_MAX_CL_RECORDS];

   unsigned num_used_layers;
} glxx_hw_render_state;

#include "glxx_hw_render_state.inl"

#endif // HW_RENDER_STATE_H
