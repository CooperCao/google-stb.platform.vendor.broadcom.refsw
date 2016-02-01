/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_BACKFLOW_VISITOR_H
#define GLSL_BACKFLOW_VISITOR_H

#include "glsl_backflow.h"

/* TODO: Template a DAG visiting thing, so we don't need to have 2 */

// Compare with ast_visitor.h for usage instructions.

typedef Backflow* (*BackflowPreVisitor)(Backflow* backflow, void* data);
typedef void (*BackflowPostVisitor)(Backflow* backflow, void* data);

// These numbers have no particular significance. TODO: use enum for all "pass" values */
#define BACKEND_PASS_SCHEDULE 123009
#define BACKEND_PASS_COMBINE  123011
#define BACKEND_PASS_PRINT    123012

// Same functions but operate on backend dataflow types
void glsl_backflow_accept_towards_leaves_prefix(Backflow* backflow, void* data, BackflowPreVisitor dprev, int pass);
void glsl_backflow_accept_towards_leaves_postfix(Backflow* backflow, void* data, BackflowPostVisitor dpostv, int pass);
void glsl_backflow_accept_towards_leaves(Backflow* backflow, void* data, BackflowPreVisitor dprev, BackflowPostVisitor dpostv, int pass);


#endif // BACKFLOW_VISITOR_H
