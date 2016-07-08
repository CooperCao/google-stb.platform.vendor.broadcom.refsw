/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Functions for driving the hardware for both GLES1.1 and GLES2.0.
=============================================================================*/

#ifndef GLXX_INNER_4_H
#define GLXX_INNER_4_H

#include "../common/khrn_image.h"
#include "../common/khrn_types.h"
#include "../common/khrn_fmem.h"
#include "glxx_server.h"
#include "gl_public_api.h"
#include "libs/core/v3d/v3d_shadrec.h"

#include "glxx_hw_render_state.h"

extern void glxx_hw_render_state_rw(GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *render_state);

extern void glxx_hw_render_state_flush(GLXX_HW_RENDER_STATE_T *render_state);

extern bool glxx_hw_start_frame_internal(GLXX_HW_RENDER_STATE_T *rs,
      GLXX_HW_FRAMEBUFFER_T *fb);
extern void glxx_hw_invalidate_internal(GLXX_HW_RENDER_STATE_T *rs, bool rt[GLXX_MAX_RENDER_TARGETS], bool color, bool multisample, bool depth, bool stencil);

extern void glxx_hw_discard_frame(GLXX_HW_RENDER_STATE_T *rs);

extern v3d_compare_func_t glxx_hw_convert_test_function(GLenum function);

extern bool glxx_hw_render_state_discard_and_restart(
   GLXX_SERVER_STATE_T *state,
   GLXX_HW_RENDER_STATE_T *rs);

#endif
