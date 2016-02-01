/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  VG config

FILE DESCRIPTION
Implementation/platform-specific configuration stuff.
=============================================================================*/

#ifndef VG_INT_CONFIG_H
#define VG_INT_CONFIG_H

#include "interface/khronos/include/VG/vgext.h"
#include "middleware/khronos/vg/2708/vg_config_filler_4.h" /* should #define VG_CONFIG_RENDERER */

#define VG_CONFIG_VENDOR "Broadcom"
/* VG_CONFIG_RENDERER is platform-specific */
#if VG_KHR_EGL_image
   #define VG_CONFIG_EXTENSIONS_KHR_EGL_IMAGE "VG_KHR_EGL_image "
#else
   #define VG_CONFIG_EXTENSIONS_KHR_EGL_IMAGE ""
#endif

#if KHR_fence_sync
#define VG_CONFIG_EXTENSIONS_KHR_FENCE_SYNC "VG_KHR_EGL_sync "
#else
#define VG_CONFIG_EXTENSIONS_KHR_FENCE_SYNC ""
#endif

#define VG_CONFIG_EXTENSIONS \
   VG_CONFIG_EXTENSIONS_KHR_EGL_IMAGE \
   VG_CONFIG_EXTENSIONS_KHR_FENCE_SYNC

#define VG_CONFIG_SCREEN_LAYOUT              VG_PIXEL_LAYOUT_UNKNOWN
#define VG_CONFIG_MAX_SCISSOR_RECTS                               32 /* must be >= 32 */
#define VG_CONFIG_MAX_DASH_COUNT                                  16 /* must be >= 16 */
#define VG_CONFIG_MAX_KERNEL_SIZE                                 15 /* must be >= 7 */
#define VG_CONFIG_MAX_SEPARABLE_KERNEL_SIZE                       33 /* must be >= 15 */
#define VG_CONFIG_MAX_COLOR_RAMP_STOPS                            32 /* must be >= 32 */
#define VG_CONFIG_MAX_IMAGE_WIDTH                               2048 /* must be >= 256 */
#define VG_CONFIG_MAX_IMAGE_HEIGHT                              2048 /* must be >= 256 */
#define VG_CONFIG_MAX_IMAGE_PIXELS                           4194304 /* must be >= 65536 */
#define VG_CONFIG_MAX_IMAGE_BYTES                           16777216 /* must be >= 65536 */
#define VG_CONFIG_MAX_FLOAT                            3.4028234e38f /* must be >= 10.0e10f */
#define VG_CONFIG_MAX_GAUSSIAN_STD_DEVIATION                   16.0f /* must be >= 16.0f */

#endif
