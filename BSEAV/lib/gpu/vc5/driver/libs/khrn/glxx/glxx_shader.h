/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "../glsl/glsl_ir_program.h"
#include "glxx_shader_cache.h"

bool glxx_hw_emit_shaders(GLXX_LINK_RESULT_DATA_T  *data,
                          const GLSL_BACKEND_CFG_T *key,
                          IR_PROGRAM_T             *ir);

void glxx_hw_cleanup_shaders(GLXX_LINK_RESULT_DATA_T *data);

void glxx_shader_process_ubo_loads(
   uint32_t* dst,
   uint32_t const* src,
   glxx_shader_ubo_load const* loads,
   unsigned num_loads);
