/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_BINARY_PROGRAM_H_INCLUDED
#define GLSL_BINARY_PROGRAM_H_INCLUDED

#include "glsl_binary_shader.h"

VCOS_EXTERN_C_BEGIN

enum shader_mode {
   MODE_BIN    = 0,
   MODE_RENDER = 1,
   MODE_COUNT  = 2
};

typedef struct {
   BINARY_SHADER_T *fshader;
   BINARY_SHADER_T *vstages[SHADER_FLAVOUR_COUNT][MODE_COUNT];
   GLSL_VARY_MAP_T  vary_map;
} BINARY_PROGRAM_T;


BINARY_PROGRAM_T *glsl_binary_program_create(void);
BINARY_PROGRAM_T *glsl_binary_program_from_dataflow(IR_PROGRAM_T                 *ir,
                                                    const GLXX_LINK_RESULT_KEY_T *key);
void glsl_binary_program_free(BINARY_PROGRAM_T *prog);

VCOS_EXTERN_C_END

#endif
