/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glxx
Module   :

FILE DESCRIPTION
=============================================================================*/

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
   KHRN_IMAGE_PLANE_T color[GLXX_MAX_RENDER_TARGETS]; // Always downsampled
   KHRN_IMAGE_PLANE_T color_ms[GLXX_MAX_RENDER_TARGETS]; // Always multisampled
   v3d_rt_bpp_t color_internal_bpp[GLXX_MAX_RENDER_TARGETS];
   v3d_rt_type_t color_internal_type[GLXX_MAX_RENDER_TARGETS];

   /* if ms = true, depth and stencil are multisampled images */
   KHRN_IMAGE_PLANE_T depth;
   KHRN_IMAGE_PLANE_T stencil;

   unsigned width; /* min width of all images */
   unsigned height; /* min height of all images */

} GLXX_HW_FRAMEBUFFER_T;

#endif
