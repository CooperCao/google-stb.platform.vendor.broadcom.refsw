/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/
#ifndef __GLSL_DFLOW_FLATTEN_VISITOR_H__
#define __GLSL_DFLOW_FLATTEN_VISITOR_H__

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_containers.h"
#include "middleware/khronos/glsl/glsl_common.h"
#include <stdio.h>

typedef struct DFlowFlattenVisitor_s
{
   DFlowVisitor   m_base;
   NodeList       *m_nodes;
} DFlowFlattenVisitor;

void DFlowFlattenVisitor_Constr(DFlowFlattenVisitor *self, DFlowRecursionOptimizer *opt, NodeList *nodes);

static INLINE void DFlowFlattenVisitor_Destr(DFlowFlattenVisitor *self)
{
   DFlowVisitor_Destr(self);
}

void DFlowFlattenVisitor_Visit(DFlowFlattenVisitor *self, DFlowNode *node);

#endif /* __GLSL_DFLOW_FLATTEN_VISITOR_H__ */
