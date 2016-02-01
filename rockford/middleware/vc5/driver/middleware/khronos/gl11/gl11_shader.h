/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Creates GLES1.1 shaders as dataflow graphs and passes them to the compiler
backend.
=============================================================================*/
#ifndef GL11_SHADER_4_H
#define GL11_SHADER_4_H

#include "middleware/khronos/gl11/gl11_shader_cache.h"
#include "middleware/khronos/glsl/glsl_ir_program.h"

extern uint32_t gl11_get_live_attr_set(const GL11_CACHE_KEY_T *shader);

extern IR_PROGRAM_T *gl11_shader_get_dataflow(const GL11_CACHE_KEY_T *v);

#endif
