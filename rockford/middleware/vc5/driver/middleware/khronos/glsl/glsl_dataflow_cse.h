/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_DATAFLOW_CSE_H_INCLUDED
#define GLSL_DATAFLOW_CSE_H_INCLUDED

#include "glsl_ir_shader.h"

// Merge duplicate dataflow trees. Modifications are done in-place.
void glsl_dataflow_cse(SSABlock *block, int n_blocks);


#endif
