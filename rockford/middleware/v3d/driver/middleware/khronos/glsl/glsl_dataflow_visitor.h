/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_DATAFLOW_VISITOR_H
#define GLSL_DATAFLOW_VISITOR_H

#include "middleware/khronos/glsl/glsl_dataflow.h"


// Compare with ast_visitor.h for usage instructions.

typedef Dataflow* (*DataflowPreVisitor)(Dataflow* dataflow, void* data);
typedef void (*DataflowPostVisitor)(Dataflow* dataflow, void* data);

// These functions visit all nodes that "dataflow" depends on.
void glsl_dataflow_accept_towards_leaves_prefix(Dataflow* dataflow, void* data, DataflowPreVisitor dprev, int pass);
void glsl_dataflow_accept_towards_leaves_postfix(Dataflow* dataflow, void* data, DataflowPostVisitor dpostv, int pass);
void glsl_dataflow_accept_towards_leaves(Dataflow* dataflow, void* data, DataflowPreVisitor dprev, DataflowPostVisitor dpostv, int pass);

// These functions visit all nodes dependent on the nodes in pool.
void glsl_dataflow_accept_from_leaves_prefix(DataflowChain* pool, void* data, DataflowPreVisitor dprev, int pass);
void glsl_dataflow_accept_from_leaves_postfix(DataflowChain* pool, void* data, DataflowPostVisitor dpostv, int pass);
void glsl_dataflow_accept_from_leaves(DataflowChain* pool, void* data, DataflowPreVisitor dprev, DataflowPostVisitor dpostv, int pass);


#endif // DATAFLOW_VISITOR_H