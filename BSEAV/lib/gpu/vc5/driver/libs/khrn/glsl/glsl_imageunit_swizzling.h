/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#if !V3D_VER_AT_LEAST(4,1,34,0)

/* The ordering in this enum is important to the compiler code. It is assumed
 * that UIF || UBLinear is equivalent to < LT. Keeping ublinear == 0 also allows
 * slightly better QPU code generation */

typedef enum {
   GLSL_IMGUNIT_SWIZZLING_UBLINEAR  = 0,
   GLSL_IMGUNIT_SWIZZLING_UIF       = 1,
   GLSL_IMGUNIT_SWIZZLING_UIF_XOR   = 2,
   GLSL_IMGUNIT_SWIZZLING_LT        = 3,
   GLSL_IMGUNIT_SWIZZLING_RSO       = 4
} glsl_imgunit_swizzling;

#endif
