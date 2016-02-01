/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_BINARY_PROGRAM_H_INCLUDED
#define GLSL_BINARY_PROGRAM_H_INCLUDED

#include "glsl_binary_shader.h"

typedef struct {
   BINARY_SHADER_T *fshader;
   BINARY_SHADER_T *cshader;
   BINARY_SHADER_T *vshader;
   GLSL_VARY_MAP_T  vary_map;
   bool             has_point_size;
} BINARY_PROGRAM_T;


BINARY_PROGRAM_T *glsl_binary_program_create();
BINARY_PROGRAM_T *glsl_binary_program_from_dataflow(IR_PROGRAM_T                 *ir,
                                                    const GLXX_LINK_RESULT_KEY_T *key,
                                                    int v3d_version);
void glsl_binary_program_free(BINARY_PROGRAM_T *prog);

#endif
