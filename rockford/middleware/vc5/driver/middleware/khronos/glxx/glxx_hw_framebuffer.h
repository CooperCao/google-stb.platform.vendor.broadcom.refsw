/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glxx
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLXX_HW_FRAMEBUFFER_H
#define GLXX_HW_FRAMEBUFFER_H

#include "middleware/khronos/common/khrn_image_plane.h"
#include "helpers/gfx/gfx_buffer_translate_v3d.h"
#include "helpers/v3d/v3d_gen.h"
#include "interface/khronos/glxx/glxx_int_config.h"
#include "middleware/khronos/common/khrn_image_plane.h"

typedef struct
{
   bool  ms;

   /* all the color images from the fb are stored here, irrespective of enabled
    * draw buffers */
   KHRN_IMAGE_PLANE_T   color[GLXX_MAX_RENDER_TARGETS];
   KHRN_IMAGE_PLANE_T   color_ms[GLXX_MAX_RENDER_TARGETS];

   /* Max used color render targets */
   uint32_t    rt_count;

   /* if ms = true, depth and  stencil are multisampled images */
   KHRN_IMAGE_PLANE_T   depth;
   KHRN_IMAGE_PLANE_T   stencil;

   unsigned    width;               /* min width of all images */
   unsigned    height;              /* min height of all images */
   v3d_rt_bpp_t   max_bpp;          /* max bpp for for all color images */

} GLXX_HW_FRAMEBUFFER_T;

#endif
