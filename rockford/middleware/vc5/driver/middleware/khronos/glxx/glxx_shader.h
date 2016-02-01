/*=============================================================================
Copyright (c); 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
common GLES 1.1 and 2.0 dataflow graph functions
=============================================================================*/

#ifndef GLXX_SHADER_4_H
#define GLXX_SHADER_4_H

#include "middleware/khronos/glsl/glsl_ir_program.h"
#include "middleware/khronos/glxx/glxx_shader_cache.h"
#include "middleware/khronos/common/khrn_fmem.h"


bool glxx_hw_emit_shaders(GLXX_LINK_RESULT_DATA_T      *data,
                          const GLXX_LINK_RESULT_KEY_T *key,
                          IR_PROGRAM_T                 *ir);

void glxx_hw_cleanup_shaders(GLXX_LINK_RESULT_DATA_T *data);

#endif
