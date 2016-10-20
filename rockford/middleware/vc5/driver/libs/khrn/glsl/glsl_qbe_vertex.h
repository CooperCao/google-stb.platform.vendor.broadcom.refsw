/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_QBE_VERTEX_H__
#define GLSL_QBE_VERTEX_H__

#include "glsl_backflow.h"

typedef struct {
   bool bin_mode;
   bool emit_point_size;
   bool z_only_active;
} VertexBackendState;

void glsl_vertex_backend(SchedBlock *block, int block_id,
                         const IRShader *sh,
                         const LinkMap *link_map,
                         SchedShaderInputs *ins,
                         const VertexBackendState *s,
                         const GLSL_VARY_MAP_T *vary_map);

#endif
