/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdio.h>
#include "glsl_dataflow.h"

EXTERN_C_BEGIN

void glsl_print_dataflow(FILE* f, Dataflow* dataflow);

// Prints "graphviz dot" representation of dataflow to f.
void glsl_print_dataflow_from_root(FILE* f, Dataflow* root);
void glsl_print_dataflow_from_roots(FILE* f, DataflowChain* roots, DataflowChain *order);

EXTERN_C_END
