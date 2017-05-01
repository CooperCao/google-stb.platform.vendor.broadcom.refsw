/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "middleware/khronos/glsl/glsl_common.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_validate_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"
#include <stdio.h>

static void Destroy(void *me)
{
   UNUSED(me);
}

static void Accept(void *me, DFlowNode *node)
{
   UNUSED(me);
   NodeList_const_iterator  iter;

   const NodeList *kids = DFlowNode_Children(node);
   const NodeList *ioKids = DFlowNode_IoChildren(node);

   assert(DFlowNode_TreeDepth(node) != -1);

   for (iter = NodeList_const_begin(kids); iter != NodeList_const_end(kids); NodeList_const_next(&iter))
   {
      DFlowNode   *starIter = NodeList_const_star(iter);

      // Our child nodes must have parent links back to us
      assert(DFlowNode_ListContains(DFlowNode_Parents(starIter), node));

      // All our children must have a bigger depth than us
      assert(DFlowNode_TreeDepth(starIter) > DFlowNode_TreeDepth(node));
   }

   for (iter = NodeList_const_begin(ioKids); iter != NodeList_const_end(ioKids); NodeList_const_next(&iter))
   {
      DFlowNode   *starIter = NodeList_const_star(iter);

      // Our child nodes must have parent links back to us
      assert(DFlowNode_ListContains(DFlowNode_IoParents(starIter), node));

      // All our children must have a bigger depth than us
      assert(DFlowNode_TreeDepth(starIter) > DFlowNode_TreeDepth(node));
   }
}

void DFlowValidateVisitor_Constr(DFlowValidateVisitor *self, DFlowRecursionOptimizer *opt)
{
   DFlowVisitor_Constr(self, Destroy, Accept, opt);
}

void DFlowValidateVisitor_Visit(DFlowValidateVisitor *self, DFlowNode *node)
{
#ifndef NDEBUG
   DFlowVisitor_VisitTopDown(self, node);
#else
   UNUSED(self);
   UNUSED(node);
#endif
}
