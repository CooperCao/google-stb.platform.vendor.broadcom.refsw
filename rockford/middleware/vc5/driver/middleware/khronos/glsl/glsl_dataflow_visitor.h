/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_DATAFLOW_VISITOR_H
#define GLSL_DATAFLOW_VISITOR_H

#include "glsl_dataflow.h"

typedef struct {
   Dataflow** seen;
   int seen_array_size;
} DataflowVisitor;

// Compare with ast_visitor.h for usage instructions.
typedef Dataflow* (*DataflowPreVisitor)(Dataflow* dataflow, void* data);
typedef void (*DataflowPostVisitor)(Dataflow* dataflow, void* data);

// These functions visit all nodes that "dataflow" depends on.
void glsl_dataflow_visitor_begin(DataflowVisitor* visitor);
void glsl_dataflow_visitor_accept(DataflowVisitor* visitor, Dataflow* dataflow, void* data, DataflowPreVisitor dprev, DataflowPostVisitor dpostv);
void glsl_dataflow_visitor_end(DataflowVisitor* visitor);

void glsl_dataflow_visit_array(Dataflow** dataflow, int start, int end, void* data, DataflowPreVisitor dprev, DataflowPostVisitor dpostv);

typedef struct {
   Dataflow *df_arr;
   int df_count;
   bool *seen;
} DataflowRelocVisitor;

void glsl_dataflow_reloc_visitor_begin(DataflowRelocVisitor *visitor, Dataflow *df_arr, int df_count,
                                       bool *temp_array);
void glsl_dataflow_reloc_post_visitor(DataflowRelocVisitor *visitor, int id, void* data, DataflowPostVisitor dpostv);
void glsl_dataflow_reloc_visitor_end(DataflowRelocVisitor *visitor);

void glsl_dataflow_reloc_visit_array(Dataflow* dataflow, int dataflow_count, int *ids, int start, int end, void* data, DataflowPostVisitor dpostv, bool *temp);

#endif // DATAFLOW_VISITOR_H
