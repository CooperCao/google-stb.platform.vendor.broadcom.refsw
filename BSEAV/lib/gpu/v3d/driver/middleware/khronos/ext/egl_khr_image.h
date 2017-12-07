/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"

#if KHRN_HW_TEX_SIZE_MAX <= 2048
typedef uint32_t tile_bits_t;
#elif KHRN_HW_TEX_SIZE_MAX <= 4096
typedef uint64_t tile_bits_t;
#else
#error Max texture size too big for this code
#endif

/* A structure with 1-bit per tile */
typedef struct {
   tile_bits_t  m_rowBits[KHRN_HW_TEX_SIZE_MAX / 64];
} EGL_IMAGE_TILE_DIRTY_BITS_T;

typedef struct EGL_IMAGE {
   MEM_HANDLE_T mh_image;

   /* intialized in install_uniforms and reset at the end of the respective draw call */
   /* prevents multiple conversions per draw call of the same buffer */
   MEM_HANDLE_T mh_tf_image;

   /* used for platforms which may have reference counting on mapping */
   EGLenum         target;
   void           *native_buffer;
   bool            platform_client_buffer;
} EGL_IMAGE_T;

extern void egl_image_term(MEM_HANDLE_T handle);
