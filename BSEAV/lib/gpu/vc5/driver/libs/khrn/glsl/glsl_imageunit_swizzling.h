/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#if !V3D_VER_AT_LEAST(4,0,2,0)
typedef uint32_t glsl_imgunit_swizzling;
#define GLSL_IMGUNIT_SWIZZLING_NONE 0
#define GLSL_IMGUNIT_SWIZZLING_LT        1 << 0
#define GLSL_IMGUNIT_SWIZZLING_UBLINEAR  1 << 1
#define GLSL_IMGUNIT_SWIZZLING_UIF       1 << 2
#define GLSL_IMGUNIT_SWIZZLING_UIF_XOR   1 << 3
#define GLSL_IMGUNIT_SWIZZLING_RSO       1 << 4
#endif
