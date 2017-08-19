/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/v3d/v3d_gen.h"
#include "../common/khrn_fmem.h"
#include "glxx_rect.h"

#if V3D_VER_AT_LEAST(4,0,2,0)

#ifdef __cplusplus
extern "C" {
#endif

v3d_addr_t glxx_create_clear_gl_g_shader_record(khrn_fmem *fmem,
      const uint32_t *f_shader, const uint32_t *f_unif,
      const glxx_rect *rect, float clear_depth_val);

#ifdef __cplusplus
}
#endif

#endif
