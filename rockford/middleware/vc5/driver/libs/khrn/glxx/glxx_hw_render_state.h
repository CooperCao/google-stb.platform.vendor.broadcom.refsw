/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/

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
   GLXX_HW_FRAMEBUFFER_T   dst_fb;

   bool color;
   /* if color = true, blit from from src_fb.color[color_read_buffer] to all
    * dst_fb.color[b] for which color_draw_to_buf[b] = true;
    * src_fb is the the HW_FRAMEBUFFER where this struct is used;
    * if color = false, no blitting to color buffers is required;
    */
   unsigned color_read_buffer;
   bool  color_draw_to_buf[GLXX_MAX_RENDER_TARGETS];

   /* if depth = true, blit from src_fb.depth to dst_fb.depth */
   bool                    depth;
   /* if depth = true, blit from src_fb.depth to dst_fb.depth */
   bool                    stencil;
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
   GLXX_CL_STATE_BLEND_MODE,
   GLXX_CL_STATE_BLEND_COLOR,
   GLXX_CL_STATE_COLOR_WRITE,
   GLXX_CL_STATE_CFG,
   GLXX_CL_STATE_LINE_WIDTH,
   GLXX_CL_STATE_POLYGON_OFFSET,
   GLXX_CL_STATE_VIEWPORT,
   GLXX_CL_STATE_STENCIL,
   GLXX_CL_STATE_SAMPLE_COVERAGE,
   GLXX_CL_STATE_OCCLUSION_QUERY,
   GLXX_CL_STATE_VCM_CACHE_SIZE,

   GLXX_CL_STATE_NUM_FIXED_SIZE,

   // variable sized states
   GLXX_CL_STATE_FLAT_SHADING_FLAGS = GLXX_CL_STATE_NUM_FIXED_SIZE,
   GLXX_CL_STATE_CENTROID_FLAGS,

   GLXX_CL_STATE_NUM
} glxx_cl_state_t;

static const uint8_t GLXX_CL_STATE_SIZE[GLXX_CL_STATE_NUM] =
{
    // fixed sized states
   [GLXX_CL_STATE_BLEND_MODE]       = V3D_CL_BLEND_CFG_SIZE,
   [GLXX_CL_STATE_BLEND_COLOR]      = V3D_CL_BLEND_CCOLOR_SIZE,
   [GLXX_CL_STATE_COLOR_WRITE]      = V3D_CL_COLOR_WMASKS_SIZE,
   [GLXX_CL_STATE_CFG]              = V3D_CL_CFG_BITS_SIZE,
   [GLXX_CL_STATE_LINE_WIDTH]       = V3D_CL_LINE_WIDTH_SIZE,
   [GLXX_CL_STATE_POLYGON_OFFSET]   = V3D_CL_DEPTH_OFFSET_SIZE,
   [GLXX_CL_STATE_VIEWPORT]         = (V3D_CL_CLIP_SIZE + V3D_CL_CLIPPER_XY_SIZE + V3D_CL_VIEWPORT_OFFSET_SIZE + V3D_CL_CLIPPER_Z_SIZE + V3D_CL_CLIPZ_SIZE),
   [GLXX_CL_STATE_STENCIL]          = (V3D_CL_STENCIL_CFG_SIZE * 2),
   [GLXX_CL_STATE_SAMPLE_COVERAGE]  = V3D_CL_SAMPLE_COVERAGE_SIZE,
   [GLXX_CL_STATE_OCCLUSION_QUERY]  = V3D_CL_OCCLUSION_QUERY_COUNTER_ENABLE_SIZE,
   [GLXX_CL_STATE_VCM_CACHE_SIZE]   = V3D_CL_VCM_CACHE_SIZE_SIZE,

   // variable sized states (these are maximums)
   [GLXX_CL_STATE_FLAT_SHADING_FLAGS]  = GLXX_NUM_SHADING_FLAG_WORDS * V3D_CL_VARY_FLAGS_SIZE,
   [GLXX_CL_STATE_CENTROID_FLAGS]      = GLXX_NUM_SHADING_FLAG_WORDS * V3D_CL_VARY_FLAGS_SIZE
};

// flag which states are optionally set
enum
{
   GLXX_CL_STATE_OPTIONAL = (1 << GLXX_CL_STATE_BLEND_COLOR)
                          | (1 << GLXX_CL_STATE_STENCIL)
                          | (1 << GLXX_CL_STATE_OCCLUSION_QUERY)  // occlusion queries are disabled in create_bin_cl, so this is optional
};

typedef struct
{
   // cumulative binning cycle count estimate of all draw calls up to start_sub_addr in control list
   uint64_t bin_cost_cumulative;
   uint8_t *start_sub_addr;
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
extern bool glxx_cl_record_apply(GLXX_CL_RECORD_T *record, KHRN_FMEM_T *fmem);

/* If we have both a downsampled color buffer and a multisample color
   * buffer, we generally load from the multisample buffer and store to both.
   * This is because the rendering is done against the multisample buffer, but
   * we keep the downsampled buffer up-to-date "at all times". See
   * doc/multisample.md.
   *
   * This could be handled with just a single bufstate, except that we want to
   * take advantage of a special case with eglSwapBuffers(). eglSwapBuffers()
   * normally invalidates both buffers, but before doing so it puts the
   * downsampled one on the display. So when we flush in eglSwapBuffers(), we
   * normally only need to store to the downsampled buffer (both are
   * invalidated, but we need the downsampled one for display).
   *
   * We deal with this by having two bufstates: one tracking the downsampled
   * output and one tracking the multisample output. They should track each
   * other exactly, except in the eglSwapBuffers() case, where the multisample
   * bufstate will get invalidated before flushing, and so will report that we
   * don't need to store to the multisample buffer when we do the flush.
   *
   * Note that we always load from the multisample buffer, never from the
   * downsampled buffer. So when we flush the two bufstates, although the
   * store flags apply to the corresponding buffers, both load flags actually
   * mean load from the multisample buffer. TODO This is conceptually awkward.
   *
   * Of course, all of that only applies when there is both a downsampled
   * color buffer *and* a multisample color buffer, which is only the case for
   * multisample EGL window surfaces (see doc/multisample.md). When there is
   * only one color buffer, only one bufstate should be active and everything
   * should work as normal. */

/* Even if we have a packed depth/stencil buffer, we have completely
   * independent bufstates for depth and stencil. We can do this because the
   * hardware is flexible enough to be able to load/store depth & stencil
   * independently even if they are packed into the same physical buffer.
   *
   * TODO One slight nuisance with this at the moment is that we only have a
   * single interlock for a packed depth/stencil buffer and that single
   * interlock cannot express things like "depth is valid but stencil isn't",
   * so we currently have to merge the depth & stencil invalid flags when we
   * flush and update the interlock. */

static_assrt(offsetof(glxx_render_state, fmem) == 0);

typedef struct glxx_hw_render_state
{
   // Inherit from glxx_render_state.
   union
   {
      glxx_render_state base;
      KHRN_FMEM_T fmem;
   };

   // Renderstate needs a copy of the hw fb as it was at the time renderstate was prepared,
   // as the framebuffer can be changed after that before the renderstate gets flushed.
   GLXX_HW_FRAMEBUFFER_T   installed_fb;

   // Context that owns this render-state.
   GLXX_SERVER_STATE_T     *server_state;

   glxx_bufstate_t         color_buffer_state   [GLXX_MAX_RENDER_TARGETS];
   glxx_bufstate_t         ms_color_buffer_state[GLXX_MAX_RENDER_TARGETS];

   glxx_bufstate_t         depth_buffer_state;
   glxx_bufstate_t         stencil_buffer_state;

   /* Store the clear colors in raw 32-bit format */
   uint32_t                clear_colors[GLXX_MAX_RENDER_TARGETS][4];

   float                   depth_value;
   uint8_t                 stencil_value;
   bool                    dither;

   // flags previously set on this render-state
   uint32_t                prev_flat_shading_flags[GLXX_NUM_SHADING_FLAG_WORDS];
   uint32_t                prev_centroid_flags[GLXX_NUM_SHADING_FLAG_WORDS];
   bool                    flat_shading_flags_set;
   bool                    centroid_flags_set;

   GLXX_BLIT_T             tlb_blits[GLXX_RS_MAX_BLITS];
   unsigned                num_blits;

   bool                    workaround_gfxh_1313;

   bool                    z_prepass_allowed;         // can enable z-prepass for this control list
   bool                    z_prepass_started;         // z-prepass function is now set, using z-prepass for at least some of this control list
   bool                    z_prepass_stopped;         // z-prepass was started and stopped
   int8_t                  z_prepass_dir;
   unsigned                num_z_prepass_bins;

   /* last occlusion query that was enabled on this rs and the instance for
    * that query at the point when the query was enabled on this render state ;
    * if query = NULL, occlusion query was disabled  on this rs */
   struct
   {
      GLXX_QUERY_T *query;
      uint64_t instance;
   } last_occlusion_query;

   /* If transform feedback is used in renderstate, we can no longer
    * discard fmem on clears.
    */
   bool                    tf_used;
   unsigned                tf_started_count;  // Number of transform feedbacks started
   unsigned                tf_waited_count;   // Number of transform feedbacks waited for

   GLXX_EZ_STATE_T         ez;

   uint8_t vcm_cache_size_bin;
   uint8_t vcm_cache_size_render;

   uint64_t cl_record_threshold;   // bin-cycle count threshold to create a new entry
   uint64_t cl_record_remaining;   // bin-cycle count remaining before creating new record
   unsigned num_cl_records;        // num entries written to cl_records
   GLXX_CL_RECORD_T cl_records[GLXX_MAX_CL_RECORDS];
   bool do_multicore_bin;
} glxx_hw_render_state;

#include "glxx_hw_render_state.inl"

#endif // HW_RENDER_STATE_H
