/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG client

FILE DESCRIPTION
Client-side VG state.
=============================================================================*/

#ifndef VG_CLIENT_H
#define VG_CLIENT_H

#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/common/khrn_client_pointermap.h"
#include "interface/khronos/common/khrn_client_vector.h"

#include "interface/khronos/egl/egl_client_context.h"

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/include/VG/openvg.h"

#include "interface/khronos/vg/vg_int_config.h"
#include "interface/khronos/vg/vg_int.h"
#include "interface/khronos/vg/vg_int_mat3x3.h"

/******************************************************************************
shared state
******************************************************************************/

#define VG_CLIENT_STEMS_COUNT_MAX 64

typedef struct {
   VG_CLIENT_OBJECT_TYPE_T object_type;
} VG_CLIENT_FONT_T;

typedef struct {
   VG_CLIENT_OBJECT_TYPE_T object_type;
   VGImageFormat format;
   VGint width;
   VGint height;
} VG_CLIENT_IMAGE_T;

typedef struct {
   VG_CLIENT_OBJECT_TYPE_T object_type;
   VGint width;
   VGint height;
} VG_CLIENT_MASK_LAYER_T;

typedef struct {
   VGfloat linear[4];
   VGfloat radial[5];
   VGColorRampSpreadMode ramp_spread_mode;
   bool ramp_pre;
   VGfloat *ramp_stops; /* NULL when count is 0 */
   VGuint ramp_stops_count;
} VG_CLIENT_PAINT_GRADIENT_T;

typedef struct {
   VG_CLIENT_OBJECT_TYPE_T object_type;
   VGPaintType type;
   VGfloat color[4];
   VG_CLIENT_PAINT_GRADIENT_T *gradient; /* allocated when required */
   VGTilingMode pattern_tiling_mode;
   VGImage pattern;
} VG_CLIENT_PAINT_T;

typedef struct {
   VG_CLIENT_OBJECT_TYPE_T object_type;
   VGint format;
   VGPathDatatype datatype;
   VGfloat scale;
   VGfloat bias;
   VGbitfield caps;
   KHRN_VECTOR_T segments;
} VG_CLIENT_PATH_T;

typedef struct {
   VGuint ref_count; /* we only read/write this when holding the client mutex */

   PLATFORM_MUTEX_T mutex; /* just one mutex for everything else in here */

   VGuint stems_count;
   VGHandle stems[VG_CLIENT_STEMS_COUNT_MAX];

   KHRN_POINTER_MAP_T objects;
} VG_CLIENT_SHARED_STATE_T;

extern VG_CLIENT_SHARED_STATE_T *vg_client_shared_state_alloc(void);
extern void vg_client_shared_state_free(VG_CLIENT_SHARED_STATE_T *shared_state);

static INLINE void vg_client_shared_state_acquire(VG_CLIENT_SHARED_STATE_T *shared_state)
{
   ++shared_state->ref_count;
}

static INLINE void vg_client_shared_state_release(VG_CLIENT_SHARED_STATE_T *shared_state)
{
   if (--shared_state->ref_count == 0) {
      vg_client_shared_state_free(shared_state);
   }
}

/******************************************************************************
state
******************************************************************************/

/*
   Called just before a rendering command (i.e. anything which could modify
   the draw surface) is executed
 */
typedef void (*VG_RENDER_CALLBACK_T)(void);

/*
   Called just after rendering has been compeleted (i.e. flush or finish).
   wait should be true for finish-like behaviour, false for flush-like
   behaviour
*/
typedef void (*VG_FLUSH_CALLBACK_T)(bool wait);

typedef struct {
   VG_MAT3X3_T client;
   VG_MAT3X3_T server;
} VG_MAT3X3_SYNC_T;

typedef struct {
   VG_CLIENT_SHARED_STATE_T *shared_state;

   VG_RENDER_CALLBACK_T render_callback;
   VG_FLUSH_CALLBACK_T flush_callback;

   /*
      matrices stored on client
      updates sent to server as necessary
   */

   VGMatrixMode matrix_mode;
   VG_MAT3X3_SYNC_T matrices[5];

   /*
      state cached on client-side to allow dropping of redundant calls
   */

   VGFillRule fill_rule;
   VGfloat stroke_line_width;
   VGCapStyle stroke_cap_style;
   VGJoinStyle stroke_join_style;
   VGfloat stroke_miter_limit;
   VGfloat stroke_dash_pattern[VG_CONFIG_MAX_DASH_COUNT];
   VGuint stroke_dash_pattern_count;
   VGfloat stroke_dash_phase;
   bool stroke_dash_phase_reset;
   VGImageQuality image_quality;
   VGImageMode image_mode;

   bool scissoring;
   VGint scissor_rects[VG_CONFIG_MAX_SCISSOR_RECTS * 4];
   VGuint scissor_rects_count;

   VGRenderingQuality rendering_quality;

   VGPaint fill_paint;
   VGPaint stroke_paint;
   VGfloat tile_fill_color[4];
   VGfloat clear_color[4];

   bool color_transform;
   VGfloat color_transform_values[8];

   VGBlendMode blend_mode;
   bool masking;

   bool filter_format_linear;
   bool filter_format_pre;
   VGuint filter_channel_mask;

   VGPixelLayout pixel_layout;
} VG_CLIENT_STATE_T;

extern VG_CLIENT_STATE_T *vg_client_state_alloc(VG_CLIENT_SHARED_STATE_T *shared_state);
extern void vg_client_state_free(VG_CLIENT_STATE_T *state);

static INLINE VG_CLIENT_STATE_T *vg_get_client_state(CLIENT_THREAD_STATE_T *thread)
{
   EGL_CONTEXT_T *context = thread->openvg.context;
   if (context) {
      vcos_assert(context->type == OPENVG);
      return (VG_CLIENT_STATE_T *)context->state;
   } else {
      return NULL;
   }
}

#define VG_GET_CLIENT_STATE(thread) vg_get_client_state(thread)

#endif
