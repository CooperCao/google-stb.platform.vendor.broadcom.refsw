/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_HW_FRAMEBUFFER_H
#define GLXX_HW_FRAMEBUFFER_H

#include "../common/khrn_image_plane.h"
#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"
#include "libs/core/v3d/v3d_gen.h"
#include "glxx_int_config.h"
#include "../common/khrn_image_plane.h"

typedef struct
{
   bool ms;

   /* all the color images from the fb are stored here, irrespective of enabled
    * draw buffers */
   uint32_t rt_count; /* Index of highest used RT + 1 or 1 if no used RTs */
   khrn_image_plane color[GLXX_MAX_RENDER_TARGETS]; // Always downsampled
   khrn_image_plane color_ms[GLXX_MAX_RENDER_TARGETS]; // Always multisampled
   V3D_RT_FORMAT_T color_rt_format[GLXX_MAX_RENDER_TARGETS];

   /* if ms = true, depth and stencil are multisampled images */
   khrn_image_plane depth;
   khrn_image_plane stencil;

   unsigned width; /* min width of all images */
   unsigned height; /* min height of all images */

} GLXX_HW_FRAMEBUFFER_T;

#endif
