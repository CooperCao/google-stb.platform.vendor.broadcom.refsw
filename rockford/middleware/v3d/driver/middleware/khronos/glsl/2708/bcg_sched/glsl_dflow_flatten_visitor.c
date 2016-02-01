/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/
#include "middleware/khronos/glsl/glsl_common.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_flatten_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"
#include <stdio.h>

static void Destroy(void *me)
{
}

static void Accept(void *me, DFlowNode *node)
{
   DFlowFlattenVisitor  *self = (DFlowFlattenVisitor *)me;

   NodeList_push_back(self->m_nodes, node);
}

void DFlowFlattenVisitor_Constr(DFlowFlattenVisitor *self, DFlowRecursionOptimizer *opt, NodeList *nodes)
{
   DFlowVisitor_Constr(self, Destroy, Accept, opt);
   self->m_nodes = nodes;
}

void DFlowFlattenVisitor_Visit(DFlowFlattenVisitor *self, DFlowNode *node)
{
   DFlowVisitor_VisitBottomUp(self, node);
}
