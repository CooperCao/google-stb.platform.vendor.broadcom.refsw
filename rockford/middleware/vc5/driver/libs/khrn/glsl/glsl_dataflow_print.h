/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_DATAFLOW_PRINT_H
#define GLSL_DATAFLOW_PRINT_H

#include <stdio.h>
#include "glsl_dataflow.h"

void glsl_print_dataflow(FILE* f, Dataflow* dataflow);

// Prints "graphviz dot" representation of dataflow to f.
void glsl_print_dataflow_from_root(FILE* f, Dataflow* root);
void glsl_print_dataflow_from_roots(FILE* f, DataflowChain* roots, DataflowChain *order);


#endif // DATAFLOW_PRINT_H
