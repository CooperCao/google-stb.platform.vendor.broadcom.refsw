/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#include "middleware/khronos/glsl/glsl_common.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_depth_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"
#include <stdio.h>

static void Destroy(void *me)
{
}

static int32_t maxi(int32_t l, int32_t r)
{
   return l > r ? l : r;
}

static void Accept(void *me, DFlowNode *node)
{
   DFlowDepthVisitor *self = (DFlowDepthVisitor *)me;

   // Find max parent depth
   NodeList_const_iterator iter;
   const NodeList *parents = DFlowNode_Parents(node);
   const NodeList *ioParents = DFlowNode_IoParents(node);

   int32_t maxDepth = -1;

   for (iter = NodeList_const_begin(parents); iter != NodeList_const_end(parents); NodeList_const_next(&iter))
      maxDepth = maxi(maxDepth, DFlowNode_TreeDepth(NodeList_const_star(iter)));

   for (iter = NodeList_const_begin(ioParents); iter != NodeList_const_end(ioParents); NodeList_const_next(&iter))
      maxDepth = maxi(maxDepth, DFlowNode_TreeDepth(NodeList_const_star(iter)));

   DFlowNode_SetTreeDepth(node, maxDepth + 1);
}


void DFlowDepthVisitor_Constr(DFlowDepthVisitor *self, DFlowRecursionOptimizer *opt)
{
   DFlowVisitor_Constr(self, Destroy, Accept, opt);
}

void DFlowDepthVisitor_Visit(DFlowDepthVisitor *self, DFlowNode *node)
{
   DFlowVisitor_VisitBreadthFirst(self, node);
}
