/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_IMAGEUNIT_SWIZZLING_H
#define GLSL_IMAGEUNIT_SWIZZLING_H

#if !V3D_VER_AT_LEAST(4,0,2,0)
typedef uint32_t glsl_imgunit_swizzling;
#define GLSL_IMGUNIT_SWIZZLING_NONE 0
#define GLSL_IMGUNIT_SWIZZLING_LT        1 << 0
#define GLSL_IMGUNIT_SWIZZLING_UBLINEAR  1 << 1
#define GLSL_IMGUNIT_SWIZZLING_UIF       1 << 2
#define GLSL_IMGUNIT_SWIZZLING_UIF_XOR   1 << 3
#define GLSL_IMGUNIT_SWIZZLING_RSO       1 << 4
#endif

#endif
