/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_backflow.h"

typedef struct {
   bool bin_mode;
   bool emit_point_size;
   bool z_only_active;

   const GLSL_VARY_MAP_T *vary_map;
} VertexBackendState;

void glsl_vertex_backend(SchedBlock *block, int block_id,
                         const IRShader *sh,
                         const LinkMap *link_map,
                         SchedShaderInputs *ins,
                         const VertexBackendState *s,
                         const bool *shader_outputs_used);
