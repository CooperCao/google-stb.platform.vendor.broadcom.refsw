/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "middleware/khronos/glsl/glsl_common.h"
#include "interface/khronos/common/khrn_options.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_optimize_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"
#include <stdio.h>

void Destroy(void *me)
{
   DFlowOptimizeVisitor *self = (DFlowOptimizeVisitor *)me;

   NodeSet_Destr(&self->m_removeList);
}

void Accept(void *me, DFlowNode *node)
{
   DFlowOptimizeVisitor *self = (DFlowOptimizeVisitor *)me;

   // Remove most mov nodes - our backend doesn't need them - with the exception of those under a 'pack to background reg', and
   // even then, only if the MOV's child has multiple parents.
   if (DFlowNode_Flavour(node) == DATAFLOW_MOV)
   {
      DFlowNode               *child;
      NodeList_const_iterator piter;
      const NodeList          *parents;

      assert(node->m_children.m_size == 1);
      child = node->m_children.m_head->m_node;

      if (DFlowNode_Parents(child)->m_size > 1)
      {
         parents = DFlowNode_Parents(node);

         for (piter = NodeList_const_begin(parents); piter != NodeList_const_end(parents); NodeList_const_next(&piter))
         {
            DFlowNode   *parent = NodeList_const_star(piter);

            switch (DFlowNode_Flavour(parent))
            {
            case DATAFLOW_PACK_COL_A :
            case DATAFLOW_PACK_COL_R :
            case DATAFLOW_PACK_COL_G :
            case DATAFLOW_PACK_COL_B :
               if (parent->m_args[DFlowNode_ARG_BACKGROUND] != NULL)
                  return;  // We need this MOV

            default:
               break;
            }
         }
      }

      NodeSet_insert(&self->m_removeList, node);
   }
}

void DFlowOptimizeVisitor_Constr(DFlowOptimizeVisitor *self, DFlowRecursionOptimizer *opt)
{
   DFlowVisitor_Constr(self, Destroy, Accept, opt);

   self->m_pass = 0;

   NodeSet_Constr(&self->m_removeList);
}

void DFlowOptimizeVisitor_Visit(DFlowOptimizeVisitor *self, DFlowNode *node)
{
   self->m_pass = 1;
   DFlowVisitor_VisitTopDown(self, node);

   {
      uint32_t    i;
      uint32_t    removeListSize = NodeSet_size(&self->m_removeList);

      NodeVector  removeList;
      NodeVector_Constr(&removeList, removeListSize);

      NodeSet_Flatten(&self->m_removeList, &removeList);

      // Unlink all nodes in remove list now

      for (i = 0; i < removeListSize; ++i)
      {
         DFlowNode               *node  = NodeVector_index(&removeList, i);
         DFlowNode               *child = NodeList_front(DFlowNode_Children(node));
         NodeList_const_iterator  piter;
         const NodeList          *parents = DFlowNode_Parents(node);
         const NodeList          *ioParents = DFlowNode_IoParents(node);

         for (piter = NodeList_const_begin(parents); piter != NodeList_const_end(parents); NodeList_const_next(&piter))
         {
            DFlowNode   *starPiter = NodeList_const_star(piter);

            DFlowNode_ReplaceChild(starPiter, node, child);
            DFlowNode_AddParent(child, starPiter);
         }

         DFlowNode_RemoveParent(child, node);

         for (piter = NodeList_const_begin(ioParents); piter != NodeList_const_end(ioParents); NodeList_const_next(&piter))
         {
            DFlowNode   *starPiter = NodeList_const_star(piter);

            DFlowNode_ReplaceChild(starPiter, node, child);
            DFlowNode_AddIoParent(child, starPiter);
         }

         DFlowNode_RemoveIoParent(child, node);
      }

      if (removeListSize > 0)
      {
         DFlowVisitor_InvalidateRecursionOptimizer(self); // We've changed the graph, we must invalidate the recursion optimizer

         if (khrn_options.glsl_debug_on)
            printf("Removed %d MOV nodes\n", removeListSize);
      }

      NodeVector_Destr(&removeList);
   }
}
