/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/v3d/v3d_gen.h"

#define V3D_CLEAR_SHADER_MAX_UNIFS 5

#ifdef __cplusplus
extern "C" {
#endif

void v3d_clear_shader_color(uint32_t **shader_code, uint32_t *shader_size, uint32_t *unif_ptr,
                            v3d_rt_type_t type, int rt, const uint32_t *clear_color);

void v3d_clear_shader_no_color(uint32_t **shader_code, uint32_t *shader_size, uint32_t *unif_ptr);


#ifdef __cplusplus
}
#endif
