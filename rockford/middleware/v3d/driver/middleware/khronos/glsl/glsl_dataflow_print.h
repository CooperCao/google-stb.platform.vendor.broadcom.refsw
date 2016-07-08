/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_DATAFLOW_PRINT_H
#define GLSL_DATAFLOW_PRINT_H

// Debug only.
#ifndef NDEBUG

#include <stdio.h>
#include "middleware/khronos/glsl/glsl_dataflow.h"

// Prints "graphviz dot" representation of dataflow to f.
void glsl_print_dataflow_from_root(FILE* f, Dataflow* root, int pass);
void glsl_print_dataflow_from_roots(FILE* f, DataflowChain* roots, DataflowChain *order, int pass);


#endif // _DEBUG
#endif // DATAFLOW_PRINT_H
