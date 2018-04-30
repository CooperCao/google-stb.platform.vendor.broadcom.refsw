/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_binary_shader.h"

EXTERN_C_BEGIN

struct glsl_backend_cfg;

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
void              glsl_binary_program_free(BINARY_PROGRAM_T *prog);

BINARY_PROGRAM_T *glsl_binary_program_from_dataflow(IR_PROGRAM_T                  *ir,
                                                    const struct glsl_backend_cfg *key);

EXTERN_C_END
