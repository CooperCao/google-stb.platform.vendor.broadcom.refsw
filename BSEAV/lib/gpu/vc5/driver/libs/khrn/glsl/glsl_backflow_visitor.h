/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
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

typedef Backflow* (*BackflowPreVisitor) (Backflow *backflow, void *data);
typedef void      (*BackflowPostVisitor)(Backflow *backflow, void *data);

typedef struct backflow_visitor BackflowVisitor;

BackflowVisitor *glsl_backflow_visitor_begin(void *data, BackflowPreVisitor prev, BackflowPostVisitor post);
void             glsl_backflow_visitor_end(BackflowVisitor *visitor);

void glsl_backflow_visit(Backflow *backflow, BackflowVisitor *v);


#endif // BACKFLOW_VISITOR_H
